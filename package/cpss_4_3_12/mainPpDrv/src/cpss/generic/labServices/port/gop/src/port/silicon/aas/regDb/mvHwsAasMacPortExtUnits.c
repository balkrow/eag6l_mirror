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
* @file mvHwsAasMacPortExtUnits.c
*
* @brief AAS MTI MAC Port EXT register DB
*
* @version   1
********************************************************************************
*/

#include <gtOs/gtGenTypes.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>

const MV_HWS_REG_ADDR_FIELD_STC aasMacPortExtDb[] = {
    /* Port Control */
  /*0*/  {/*baseAddr*/ 0x100,  /*offsetFormula*/ 0x100, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*TX_LOC_FAULT */
  /*1*/  {/*baseAddr*/ 0x100,  /*offsetFormula*/ 0x100, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*TX_REM_FAULT */
  /*2*/  {/*baseAddr*/ 0x100,  /*offsetFormula*/ 0x100, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*TX_LI_FAULT */
  /*3*/  {/*baseAddr*/ 0x100,  /*offsetFormula*/ 0x100, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*TOD_SELECT */
  /*4*/  {/*baseAddr*/ 0x100,  /*offsetFormula*/ 0x100, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*RX_PAUSE_CONTROL */
  /*5*/  {/*baseAddr*/ 0x100,  /*offsetFormula*/ 0x100, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*RX_PAUSE_OW_VAL */
  /*6*/  {/*baseAddr*/ 0x100,  /*offsetFormula*/ 0x100, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*PAUSE_802_3_REFLECT */
  /*7*/  {/*baseAddr*/ 0x100,  /*offsetFormula*/ 0x100, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*LOOP_ENA */
  /*8*/  {/*baseAddr*/ 0x100,  /*offsetFormula*/ 0x100, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*FF_TX_CRC */
  /*9*/  {/*baseAddr*/ 0x100,  /*offsetFormula*/ 0x100, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*MAC_TIME_CAPTURE_EN */
  /*10*/  {/*baseAddr*/ 0x100,  /*offsetFormula*/ 0x100, /*fieldStart*/ 10, /*fieldLen*/ 6 }, /*LED_PORT_NUM */
  /*11*/  {/*baseAddr*/ 0x100,  /*offsetFormula*/ 0x100, /*fieldStart*/ 16, /*fieldLen*/ 1 }, /*LED_PORT_EN */
  /*12*/  {/*baseAddr*/ 0x100,  /*offsetFormula*/ 0x100, /*fieldStart*/ 17, /*fieldLen*/ 1 }, /*LOOP_RX_BLOCK_OUT */
  /*13*/  {/*baseAddr*/ 0x100,  /*offsetFormula*/ 0x100, /*fieldStart*/ 18, /*fieldLen*/ 1 }, /*LOOP_TX_RDY_OUT */
  /*14*/  {/*baseAddr*/ 0x100,  /*offsetFormula*/ 0x100, /*fieldStart*/ 19, /*fieldLen*/ 2 }, /*BACK_PRESSURE_MODE_SELECT */
  /*15*/  {/*baseAddr*/ 0x100,  /*offsetFormula*/ 0x100, /*fieldStart*/ 21, /*fieldLen*/ 1 }, /*TX_LOC_FAULT_SEL */
  /*16*/  {/*baseAddr*/ 0x100,  /*offsetFormula*/ 0x100, /*fieldStart*/ 22, /*fieldLen*/ 1 }, /*TX_REM_FAULT_SEL */

    /* Port Status */
  /*17*/  {/*baseAddr*/ 0x104,  /*offsetFormula*/ 0x100, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*LINK_OK */
  /*18*/  {/*baseAddr*/ 0x104,  /*offsetFormula*/ 0x100, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*RX_TRAFFIC_IND */
  /*19*/  {/*baseAddr*/ 0x104,  /*offsetFormula*/ 0x100, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*TX_TRAFFIC_IND */
  /*20*/  {/*baseAddr*/ 0x104,  /*offsetFormula*/ 0x100, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*MAC_TX_EMPTY */
  /*21*/  {/*baseAddr*/ 0x104,  /*offsetFormula*/ 0x100, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*MAC_TX_ISIDLE */
  /*22*/  {/*baseAddr*/ 0x104,  /*offsetFormula*/ 0x100, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*PFC_MODE */
  /*23*/  {/*baseAddr*/ 0x104,  /*offsetFormula*/ 0x100, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*FF_TX_SEPTY */
  /*24*/  {/*baseAddr*/ 0x104,  /*offsetFormula*/ 0x100, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*FF_RX_EMPTY */
  /*25*/  {/*baseAddr*/ 0x104,  /*offsetFormula*/ 0x100, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*FF_RX_DSAV */
  /*26*/  {/*baseAddr*/ 0x104,  /*offsetFormula*/ 0x100, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*LINK_OK_CLEAN */
  /*27*/  {/*baseAddr*/ 0x104,  /*offsetFormula*/ 0x100, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*LINK_STATUS_CLEAN */
  /*28*/  {/*baseAddr*/ 0x104,  /*offsetFormula*/ 0x100, /*fieldStart*/ 20, /*fieldLen*/ 1 }, /*TSU_RX_READY */
  /*29*/  {/*baseAddr*/ 0x104,  /*offsetFormula*/ 0x100, /*fieldStart*/ 21, /*fieldLen*/ 1 }, /*TSU_TX_READY */
  /*30*/  {/*baseAddr*/ 0x104,  /*offsetFormula*/ 0x100, /*fieldStart*/ 22, /*fieldLen*/ 1 }, /*FF_TX_RDY */
  /*31*/  {/*baseAddr*/ 0x104,  /*offsetFormula*/ 0x100, /*fieldStart*/ 23, /*fieldLen*/ 1 }, /*FF_RX_RDY */

    /* Port Pause and Error Status */
  /*32*/  {/*baseAddr*/ 0x108,  /*offsetFormula*/ 0x100, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*PAUSE_ON */
  /*33*/  {/*baseAddr*/ 0x108,  /*offsetFormula*/ 0x100, /*fieldStart*/  8, /*fieldLen*/ 8 }, /*FF_RX_ERR_STAT */

    /* Port Interrupt Cause */
  /*34*/  {/*baseAddr*/ 0x10c,  /*offsetFormula*/ 0x100, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*PORT_INTERRUPT_CAUSE_INT_SUM */
  /*35*/  {/*baseAddr*/ 0x10c,  /*offsetFormula*/ 0x100, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*ILLEGAL_ADDRESS_ACCESS */
  /*36*/  {/*baseAddr*/ 0x10c,  /*offsetFormula*/ 0x100, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*LINK_OK_CHANGE */
  /*37*/  {/*baseAddr*/ 0x10c,  /*offsetFormula*/ 0x100, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*MAC_LOC_FAULT */
  /*38*/  {/*baseAddr*/ 0x10c,  /*offsetFormula*/ 0x100, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*MAC_REM_FAULT */
  /*39*/  {/*baseAddr*/ 0x10c,  /*offsetFormula*/ 0x100, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*MAC_LI_FAULT */
  /*40*/  {/*baseAddr*/ 0x10c,  /*offsetFormula*/ 0x100, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*MAC_TX_UNDERFLOW */
  /*41*/  {/*baseAddr*/ 0x10c,  /*offsetFormula*/ 0x100, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*EMAC_TX_OVR_ERR */
  /*42*/  {/*baseAddr*/ 0x10c,  /*offsetFormula*/ 0x100, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*PMAC_TX_OVR_ERR */
  /*43*/  {/*baseAddr*/ 0x10c,  /*offsetFormula*/ 0x100, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*EMAC_RX_OVERRUN */
  /*44*/  {/*baseAddr*/ 0x10c,  /*offsetFormula*/ 0x100, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*PMAC_RX_OVERRUN */
  /*45*/  {/*baseAddr*/ 0x10c,  /*offsetFormula*/ 0x100, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*FFE_RX_RDY */
  /*46*/  {/*baseAddr*/ 0x10c,  /*offsetFormula*/ 0x100, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*FFP_RX_RDY */
  /*47*/  {/*baseAddr*/ 0x10c,  /*offsetFormula*/ 0x100, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*LINK_OK_CLEAN_CHANGE */
  /*48*/  {/*baseAddr*/ 0x10c,  /*offsetFormula*/ 0x100, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*FFP_RX_NOT_RDY */
  /*49*/  {/*baseAddr*/ 0x10c,  /*offsetFormula*/ 0x100, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*FFE_RX_NOT_RDY */
  /*50*/  {/*baseAddr*/ 0x10c,  /*offsetFormula*/ 0x100, /*fieldStart*/ 16, /*fieldLen*/ 1 }, /*FFE_RDY2LOAD_BRG_FIFO_OVERRUN */
  /*51*/  {/*baseAddr*/ 0x10c,  /*offsetFormula*/ 0x100, /*fieldStart*/ 17, /*fieldLen*/ 1 }, /*FFP_RDY2LOAD_BRG_FIFO_OVERRUN */
  /*52*/  {/*baseAddr*/ 0x10c,  /*offsetFormula*/ 0x100, /*fieldStart*/ 18, /*fieldLen*/ 1 }, /*TSU_RX_DFF_ERR */
  /*53*/  {/*baseAddr*/ 0x10c,  /*offsetFormula*/ 0x100, /*fieldStart*/ 19, /*fieldLen*/ 1 }, /*TSU_RX_AM_ERR */
  /*54*/  {/*baseAddr*/ 0x10c,  /*offsetFormula*/ 0x100, /*fieldStart*/ 20, /*fieldLen*/ 1 }, /*TSU_TX_SYNC_ERR */
  /*55*/  {/*baseAddr*/ 0x10c,  /*offsetFormula*/ 0x100, /*fieldStart*/ 21, /*fieldLen*/ 1 }, /*TSQ_TS_FIFO_WR */
  /*56*/  {/*baseAddr*/ 0x10c,  /*offsetFormula*/ 0x100, /*fieldStart*/ 22, /*fieldLen*/ 1 }, /*TSQ_TS_FIFO_OVERWRITE */
  /*57*/  {/*baseAddr*/ 0x10c,  /*offsetFormula*/ 0x100, /*fieldStart*/ 23, /*fieldLen*/ 1 }, /*TX_VERIFY_RESPONSE_DONE */
  /*58*/  {/*baseAddr*/ 0x10c,  /*offsetFormula*/ 0x100, /*fieldStart*/ 24, /*fieldLen*/ 1 }, /*FRAME_DROP */

    /* Port Interrupt Mask */
  /*59*/  {/*baseAddr*/ 0x110,  /*offsetFormula*/ 0x100, /*fieldStart*/  1, /*fieldLen*/ 24 }, /*INTERRUPT_MASK */

    /* Port Peer Delay */
  /*60*/  {/*baseAddr*/ 0x114,  /*offsetFormula*/ 0x100, /*fieldStart*/  0, /*fieldLen*/ 30 }, /*PEER_DELAY */
  /*61*/  {/*baseAddr*/ 0x114,  /*offsetFormula*/ 0x100, /*fieldStart*/ 31, /*fieldLen*/ 1 }, /*PEER_DELAY_VALID */

    /* Port Xoff Status */
  /*62*/  {/*baseAddr*/ 0x118,  /*offsetFormula*/ 0x100, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*XOFF_STATUS */

    /* Port Pause Override */
  /*63*/  {/*baseAddr*/ 0x11c,  /*offsetFormula*/ 0x100, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*PAUSE_OVERRIDE_CTRL */
  /*64*/  {/*baseAddr*/ 0x11c,  /*offsetFormula*/ 0x100, /*fieldStart*/  8, /*fieldLen*/ 8 }, /*PAUSE_OVERRIDE_VAL */

    /* Port Xoff Override */
  /*65*/  {/*baseAddr*/ 0x120,  /*offsetFormula*/ 0x100, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*XOFF_OVERRIDE_CTRL */
  /*66*/  {/*baseAddr*/ 0x120,  /*offsetFormula*/ 0x100, /*fieldStart*/  8, /*fieldLen*/ 8 }, /*XOFF_OVERRIDE_VAL */

    /* Port Control1 */
  /*67*/  {/*baseAddr*/ 0x124,  /*offsetFormula*/ 0x100, /*fieldStart*/  0, /*fieldLen*/ 4 }, /*PORT_RES_SPEED */
  /*68*/  {/*baseAddr*/ 0x124,  /*offsetFormula*/ 0x100, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*FORCE_LINK_OK_EN */
  /*69*/  {/*baseAddr*/ 0x124,  /*offsetFormula*/ 0x100, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*FORCE_LINK_OK_DIS */
  /*70*/  {/*baseAddr*/ 0x124,  /*offsetFormula*/ 0x100, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*FORCE_TS_IF_PCH_EN */
  /*71*/  {/*baseAddr*/ 0x124,  /*offsetFormula*/ 0x100, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*FORCE_TS_IF_PCH_DIS */
  /*72*/  {/*baseAddr*/ 0x124,  /*offsetFormula*/ 0x100, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*FORCE_TS_IF_CF_EN */
  /*73*/  {/*baseAddr*/ 0x124,  /*offsetFormula*/ 0x100, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*FORCE_TS_IF_CF_DIS */
  /*74*/  {/*baseAddr*/ 0x124,  /*offsetFormula*/ 0x100, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*FORCE_TS_IF_TSTF_EN */
  /*75*/  {/*baseAddr*/ 0x124,  /*offsetFormula*/ 0x100, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*FORCE_TS_IF_TSTF_DIS */
  /*76*/  {/*baseAddr*/ 0x124,  /*offsetFormula*/ 0x100, /*fieldStart*/ 16, /*fieldLen*/ 1 }, /*LINK_UP_MUX */
  /*77*/  {/*baseAddr*/ 0x124,  /*offsetFormula*/ 0x100, /*fieldStart*/ 17, /*fieldLen*/ 1 }, /*FORCE_TS_IF_2STEP_EN */
  /*78*/  {/*baseAddr*/ 0x124,  /*offsetFormula*/ 0x100, /*fieldStart*/ 18, /*fieldLen*/ 1 }, /*FORCE_TS_IF_2STEP_DIS */
  /*79*/  {/*baseAddr*/ 0x124,  /*offsetFormula*/ 0x100, /*fieldStart*/ 19, /*fieldLen*/ 1 }, /*MAC_OPEN_DRAIN_DISABLED */
  /*80*/  {/*baseAddr*/ 0x124,  /*offsetFormula*/ 0x100, /*fieldStart*/ 20, /*fieldLen*/ 1 }, /*MAC_OPEN_DRAIN_ENABLED */

    /* Port Control2 */
  /*81*/  {/*baseAddr*/ 0x128,  /*offsetFormula*/ 0x100, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*FEC91_ENA */

    /* External Port Control */
  /*82*/  {/*baseAddr*/ 0x130,  /*offsetFormula*/ 0x100, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*MAC_MUX_CTRL */

    /* Per Port Last Violation */
  /*83*/  {/*baseAddr*/ 0x134,  /*offsetFormula*/ 0x100, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*PER_PORT_LAST_VIOLATION */

    /* Port L1 Interrupt Cause */
  /*84*/  {/*baseAddr*/ 0x138,  /*offsetFormula*/ 0x100, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*PORT_L1_INTERRUPT_CAUSE_INT_SUM */
  /*85*/  {/*baseAddr*/ 0x138,  /*offsetFormula*/ 0x100, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*L1_LINK_OK_CHANGE */
  /*86*/  {/*baseAddr*/ 0x138,  /*offsetFormula*/ 0x100, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*L1_MAC_LOC_FAULT */
  /*87*/  {/*baseAddr*/ 0x138,  /*offsetFormula*/ 0x100, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*L1_MAC_REM_FAULT */
  /*88*/  {/*baseAddr*/ 0x138,  /*offsetFormula*/ 0x100, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*L1_MAC_LI_FAULT */
  /*89*/  {/*baseAddr*/ 0x138,  /*offsetFormula*/ 0x100, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*L1_MAC_TX_UNDERFLOW */
  /*90*/  {/*baseAddr*/ 0x138,  /*offsetFormula*/ 0x100, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*L1_EMAC_TX_OVR_ERR */
  /*91*/  {/*baseAddr*/ 0x138,  /*offsetFormula*/ 0x100, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*L1_PMAC_TX_OVR_ERR */
  /*92*/  {/*baseAddr*/ 0x138,  /*offsetFormula*/ 0x100, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*L1_EMAC_RX_OVERRUN */
  /*93*/  {/*baseAddr*/ 0x138,  /*offsetFormula*/ 0x100, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*L1_PMAC_RX_OVERRUN */
  /*94*/  {/*baseAddr*/ 0x138,  /*offsetFormula*/ 0x100, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*L1_FFE_RX_RDY */
  /*95*/  {/*baseAddr*/ 0x138,  /*offsetFormula*/ 0x100, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*L1_FFP_RX_RDY */
  /*96*/  {/*baseAddr*/ 0x138,  /*offsetFormula*/ 0x100, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*L1_LINK_OK_CLEAN_CHANGE */
  /*97*/  {/*baseAddr*/ 0x138,  /*offsetFormula*/ 0x100, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*L1_FFP_RX_NOT_RDY */
  /*98*/  {/*baseAddr*/ 0x138,  /*offsetFormula*/ 0x100, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*L1_FFE_RX_NOT_RDY */
  /*99*/  {/*baseAddr*/ 0x138,  /*offsetFormula*/ 0x100, /*fieldStart*/ 16, /*fieldLen*/ 1 }, /*L1_FFE_RDY2LOAD_BRG_FIFO_OVERRUN */
  /*100*/  {/*baseAddr*/ 0x138,  /*offsetFormula*/ 0x100, /*fieldStart*/ 17, /*fieldLen*/ 1 }, /*L1_FFP_RDY2LOAD_BRG_FIFO_OVERRUN */
  /*101*/  {/*baseAddr*/ 0x138,  /*offsetFormula*/ 0x100, /*fieldStart*/ 18, /*fieldLen*/ 1 }, /*L1_TSU_RX_DFF_ERR */
  /*102*/  {/*baseAddr*/ 0x138,  /*offsetFormula*/ 0x100, /*fieldStart*/ 19, /*fieldLen*/ 1 }, /*L1_TSU_RX_AM_ERR */
  /*103*/  {/*baseAddr*/ 0x138,  /*offsetFormula*/ 0x100, /*fieldStart*/ 20, /*fieldLen*/ 1 }, /*L1_TSU_TX_SYNC_ERR */
  /*104*/  {/*baseAddr*/ 0x138,  /*offsetFormula*/ 0x100, /*fieldStart*/ 21, /*fieldLen*/ 1 }, /*L1_TSQ_TS_FIFO_WR */
  /*105*/  {/*baseAddr*/ 0x138,  /*offsetFormula*/ 0x100, /*fieldStart*/ 22, /*fieldLen*/ 1 }, /*L1_TSQ_TS_FIFO_OVERWRITE */
  /*106*/  {/*baseAddr*/ 0x138,  /*offsetFormula*/ 0x100, /*fieldStart*/ 23, /*fieldLen*/ 1 }, /*L1_TX_VERIFY_RESPONSE_DONE */
  /*107*/  {/*baseAddr*/ 0x138,  /*offsetFormula*/ 0x100, /*fieldStart*/ 24, /*fieldLen*/ 1 }, /*L1_FRAME_DROP */

    /* Port L1 Interrupt Mask */
  /*108*/  {/*baseAddr*/ 0x13c,  /*offsetFormula*/ 0x100, /*fieldStart*/  1, /*fieldLen*/ 24 }, /*INTERRUPT_MASK */

    /* Port Summary L1 Interrupt Cause */
  /*109*/  {/*baseAddr*/ 0x140,  /*offsetFormula*/ 0x100, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*MAC_PORT_EXT_UNITS_INTERRUPTS_SUMMARY */
  /*110*/  {/*baseAddr*/ 0x140,  /*offsetFormula*/ 0x100, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*PORT_L1_INTERRUPT */

    /* Port Summary L1 Interrupt Mask */
  /*111*/  {/*baseAddr*/ 0x144,  /*offsetFormula*/ 0x100, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*PORT_SUMMARY_L1_INTERRUPT_MASK */

    /* Port TSX Control */
  /*112*/  {/*baseAddr*/ 0x150,  /*offsetFormula*/ 0x100, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*C_MODULO_RX */

    /* Port TSX Control1 */
  /*113*/  {/*baseAddr*/ 0x154,  /*offsetFormula*/ 0x100, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*C_MODULO_TX */

    /* Port TSX Control2 */
  /*114*/  {/*baseAddr*/ 0x158,  /*offsetFormula*/ 0x100, /*fieldStart*/  0, /*fieldLen*/ 5 }, /*C_BLOCKTIME */
  /*115*/  {/*baseAddr*/ 0x158,  /*offsetFormula*/ 0x100, /*fieldStart*/  5, /*fieldLen*/ 9 }, /*C_BLOCKTIME_DEC */
  /*116*/  {/*baseAddr*/ 0x158,  /*offsetFormula*/ 0x100, /*fieldStart*/ 14, /*fieldLen*/ 5 }, /*C_MARKERTIME */
  /*117*/  {/*baseAddr*/ 0x158,  /*offsetFormula*/ 0x100, /*fieldStart*/ 19, /*fieldLen*/ 9 }, /*C_MARKERTIME_DEC */
  /*118*/  {/*baseAddr*/ 0x158,  /*offsetFormula*/ 0x100, /*fieldStart*/ 28, /*fieldLen*/ 3 }, /*C_TX_MODE */

    /* Port TSX Control3 */
  /*119*/  {/*baseAddr*/ 0x15c,  /*offsetFormula*/ 0x100, /*fieldStart*/  0, /*fieldLen*/ 3 }, /*C_RX_MODE */
  /*120*/  {/*baseAddr*/ 0x15c,  /*offsetFormula*/ 0x100, /*fieldStart*/  3, /*fieldLen*/ 3 }, /*C_MII_CW_DLY */
  /*121*/  {/*baseAddr*/ 0x15c,  /*offsetFormula*/ 0x100, /*fieldStart*/  6, /*fieldLen*/ 3 }, /*C_MII_MK_DLY */
  /*122*/  {/*baseAddr*/ 0x15c,  /*offsetFormula*/ 0x100, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*C_BLKS_PER_CLK */
  /*123*/  {/*baseAddr*/ 0x15c,  /*offsetFormula*/ 0x100, /*fieldStart*/ 10, /*fieldLen*/ 10 }, /*C_DESKEW */
  /*124*/  {/*baseAddr*/ 0x15c,  /*offsetFormula*/ 0x100, /*fieldStart*/ 20, /*fieldLen*/ 5 }, /*C_MII_TX_MK_CYC_DLY */
  /*125*/  {/*baseAddr*/ 0x15c,  /*offsetFormula*/ 0x100, /*fieldStart*/ 25, /*fieldLen*/ 5 }, /*C_MII_TX_CW_CYC_DLY */

    /* Port TSX Control4 */
  /*126*/  {/*baseAddr*/ 0x160,  /*offsetFormula*/ 0x100, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*C_MODULO_RX_OW */
  /*127*/  {/*baseAddr*/ 0x160,  /*offsetFormula*/ 0x100, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*C_MODULO_TX_OW */
  /*128*/  {/*baseAddr*/ 0x160,  /*offsetFormula*/ 0x100, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*C_RX_MODE_OW */
  /*129*/  {/*baseAddr*/ 0x160,  /*offsetFormula*/ 0x100, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*C_MII_CW_DLY_OW */
  /*130*/  {/*baseAddr*/ 0x160,  /*offsetFormula*/ 0x100, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*C_MII_MK_DLY_OW */
  /*131*/  {/*baseAddr*/ 0x160,  /*offsetFormula*/ 0x100, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*C_DESKEW_OW */
  /*132*/  {/*baseAddr*/ 0x160,  /*offsetFormula*/ 0x100, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*C_BLOCKTIME_OW */
  /*133*/  {/*baseAddr*/ 0x160,  /*offsetFormula*/ 0x100, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*C_BLOCKTIME_DEC_OW */
  /*134*/  {/*baseAddr*/ 0x160,  /*offsetFormula*/ 0x100, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*C_MARKERTIME_OW */
  /*135*/  {/*baseAddr*/ 0x160,  /*offsetFormula*/ 0x100, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*C_MARKERTIME_DEC_OW */
  /*136*/  {/*baseAddr*/ 0x160,  /*offsetFormula*/ 0x100, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*C_BLKS_PER_CLK_OW */
  /*137*/  {/*baseAddr*/ 0x160,  /*offsetFormula*/ 0x100, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*C_TX_MODE_OW */
  /*138*/  {/*baseAddr*/ 0x160,  /*offsetFormula*/ 0x100, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*C_MII_TX_MK_CYC_DLY_OW */
  /*139*/  {/*baseAddr*/ 0x160,  /*offsetFormula*/ 0x100, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*C_MII_TX_CW_CYC_DLY_OW */
  /*140*/  {/*baseAddr*/ 0x160,  /*offsetFormula*/ 0x100, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*C_TSU_TX_SD_PERIOD_OW */
  /*141*/  {/*baseAddr*/ 0x160,  /*offsetFormula*/ 0x100, /*fieldStart*/ 15, /*fieldLen*/ 15 }, /*C_TSU_TX_SD_PERIOD */

    /* Port Summary Interrupt Cause */
  /*142*/  {/*baseAddr*/ 0x168,  /*offsetFormula*/ 0x100, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*MAC_PORT_EXT_UNITS_INTERRUPTS_SUMMARY */
  /*143*/  {/*baseAddr*/ 0x168,  /*offsetFormula*/ 0x100, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*PORT_INTERRUPT */

    /* Port Summary Interrupt Mask */
  /*144*/  {/*baseAddr*/ 0x16c,  /*offsetFormula*/ 0x100, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*PORT_SUMMARY_INTERRUPT_MASK */

    /* Port PMAC Control */
  /*145*/  {/*baseAddr*/ 0x170,  /*offsetFormula*/ 0x100, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*PREEMPTION_LOOP_ENA */
  /*146*/  {/*baseAddr*/ 0x170,  /*offsetFormula*/ 0x100, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*FFP_TX_CRC */
  /*147*/  {/*baseAddr*/ 0x170,  /*offsetFormula*/ 0x100, /*fieldStart*/ 22, /*fieldLen*/ 2 }, /*PREEMPTION_BACK_PRESSURE_MODE_SELECT */
  /*148*/  {/*baseAddr*/ 0x170,  /*offsetFormula*/ 0x100, /*fieldStart*/ 24, /*fieldLen*/ 1 }, /*PREEMPTION_LOOP_RX_BLOCK_OUT */
  /*149*/  {/*baseAddr*/ 0x170,  /*offsetFormula*/ 0x100, /*fieldStart*/ 25, /*fieldLen*/ 1 }, /*PREEMPTION_LOOP_TX_RDY_OUT */
  /*150*/  {/*baseAddr*/ 0x170,  /*offsetFormula*/ 0x100, /*fieldStart*/ 26, /*fieldLen*/ 2 }, /*LED_INDICATION_CTRL */
  /*151*/  {/*baseAddr*/ 0x170,  /*offsetFormula*/ 0x100, /*fieldStart*/ 28, /*fieldLen*/ 1 }, /*TX_HOLD_REQ */

    /* Port PMAC Status */
  /*152*/  {/*baseAddr*/ 0x174,  /*offsetFormula*/ 0x100, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*PREEMPTION_ENABLED */
  /*153*/  {/*baseAddr*/ 0x174,  /*offsetFormula*/ 0x100, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*EXPRESS_MAC_ACTIVE */
  /*154*/  {/*baseAddr*/ 0x174,  /*offsetFormula*/ 0x100, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*PREEMPTION_MAC_ACTIVE */
  /*155*/  {/*baseAddr*/ 0x174,  /*offsetFormula*/ 0x100, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*PREEMPTION_MAC_ACTIVE_OR_FRAME_PREEMPTED */
  /*156*/  {/*baseAddr*/ 0x174,  /*offsetFormula*/ 0x100, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*PREEMPTION_MAC_TX_EMPTY */
  /*157*/  {/*baseAddr*/ 0x174,  /*offsetFormula*/ 0x100, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*FFP_TX_SEPTY */
  /*158*/  {/*baseAddr*/ 0x174,  /*offsetFormula*/ 0x100, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*FFP_RX_EMPTY */
  /*159*/  {/*baseAddr*/ 0x174,  /*offsetFormula*/ 0x100, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*FFP_RX_DSAV */
  /*160*/  {/*baseAddr*/ 0x174,  /*offsetFormula*/ 0x100, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*FF_PMAC_TX_RDY */
  /*161*/  {/*baseAddr*/ 0x174,  /*offsetFormula*/ 0x100, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*FF_PMAC_RX_RDY */

    /* Port PMAC Error Status */
  /*162*/  {/*baseAddr*/ 0x178,  /*offsetFormula*/ 0x100, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*FFP_RX_ERR_STAT */

    /* Port MAC Clock and Reset Control */
  /*163*/  {/*baseAddr*/ 0x17c,  /*offsetFormula*/ 0x100, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*MAC_RESET_ */
  /*164*/  {/*baseAddr*/ 0x17c,  /*offsetFormula*/ 0x100, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*MAC_CLK_EN */
  /*165*/  {/*baseAddr*/ 0x17c,  /*offsetFormula*/ 0x100, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*MAC_DOUBLE_FREQUENCY */

    /* Port GPIO Status */
  /*166*/  {/*baseAddr*/ 0x180,  /*offsetFormula*/ 0x100, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*GPIO_STATUS */

    /* Port GPIO Control */
  /*167*/  {/*baseAddr*/ 0x184,  /*offsetFormula*/ 0x100, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*GPIO_CONTROL */

    /* Port FC and PFC Control */
  /*168*/  {/*baseAddr*/ 0x188,  /*offsetFormula*/ 0x100, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*XOFF_ONLY_FROM_EXP */
  /*169*/  {/*baseAddr*/ 0x188,  /*offsetFormula*/ 0x100, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*MAC_XOFF_GEN_SAMP */
  /*170*/  {/*baseAddr*/ 0x188,  /*offsetFormula*/ 0x100, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*DUPLICATE_XOFF_E_TO_P */
  /*171*/  {/*baseAddr*/ 0x188,  /*offsetFormula*/ 0x100, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*FC_MODE_MSK_OUT */
  /*172*/  {/*baseAddr*/ 0x188,  /*offsetFormula*/ 0x100, /*fieldStart*/  4, /*fieldLen*/ 2 }, /*TX_HOLD_SRC */
  /*173*/  {/*baseAddr*/ 0x188,  /*offsetFormula*/ 0x100, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*PMAC_STOP_BY_PAUSE_802_3 */
  /*174*/  {/*baseAddr*/ 0x188,  /*offsetFormula*/ 0x100, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*PMAC_STOP_VAL */

    /* TSQ Timestamp */
  /*175*/  {/*baseAddr*/ 0x190,  /*offsetFormula*/ 0x100, /*fieldStart*/  0, /*fieldLen*/ 32 }, /*TSQ_TIMESTAMP */

    /* TSQ Signature */
  /*176*/  {/*baseAddr*/ 0x194,  /*offsetFormula*/ 0x100, /*fieldStart*/  0, /*fieldLen*/ 14 }, /*TSQ_SIGNATURE_VALUE */
  /*177*/  {/*baseAddr*/ 0x194,  /*offsetFormula*/ 0x100, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*TSQ_REQUEST_SOURCE_PORT */
  /*178*/  {/*baseAddr*/ 0x194,  /*offsetFormula*/ 0x100, /*fieldStart*/ 16, /*fieldLen*/ 1 }, /*TSQ_VALID */

    /* TSQ FIFO Status */
  /*179*/  {/*baseAddr*/ 0x198,  /*offsetFormula*/ 0x100, /*fieldStart*/  0, /*fieldLen*/ 4 }, /*TSQ_FIFO_FILL_LEVEL */
  /*180*/  {/*baseAddr*/ 0x198,  /*offsetFormula*/ 0x100, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*TSQ_FIFO_FULL */

    /* TSQ Clear Control */
  /*181*/  {/*baseAddr*/ 0x19c,  /*offsetFormula*/ 0x100, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*TSQ_CLEAR_CONTROL */
};

