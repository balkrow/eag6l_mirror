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
* @file mvHwsAasAnUnits.c
*
* @brief AAS AN register DB
*
* @version   1
********************************************************************************
*/

#include <gtOs/gtGenTypes.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>

MV_HWS_REG_ADDR_FIELD_STC aasAnDb[] = {
    /* 802_3AP_AUTO_NEGOTIATION_CONTROL */
  /*0*/  {/*baseAddr*/ 0x20000,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 9 }, /*CONTROL_RESERVED */
  /*1*/  {/*baseAddr*/ 0x20000,  /*offsetFormula*/ 0x1000, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*RESTART_802_3AP_AUTO_NEGOTIATION */
  /*2*/  {/*baseAddr*/ 0x20000,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*FIELD_802_3AP_AUTO_NEGOTIATION_ENABLE */
  /*3*/  {/*baseAddr*/ 0x20000,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*EXTENDED_NEXT_PAGE_CONTROL */
  /*4*/  {/*baseAddr*/ 0x20000,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*MY_RESET */

    /* 802_3AP_AUTO_NEGOTIATION_STATUS */
  /*5*/  {/*baseAddr*/ 0x20004,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*LINK_PARTNER_AUTO_NEGOTIATION_ABILITY */
  /*6*/  {/*baseAddr*/ 0x20004,  /*offsetFormula*/ 0x1000, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*FIELD_802_3AP_LINK_STATUS */
  /*7*/  {/*baseAddr*/ 0x20004,  /*offsetFormula*/ 0x1000, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*FIELD_802_3AP_AUTO_NEGOTIATION_ABILITY */
  /*8*/  {/*baseAddr*/ 0x20004,  /*offsetFormula*/ 0x1000, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*STAT_REMOTE_FAULT */
  /*9*/  {/*baseAddr*/ 0x20004,  /*offsetFormula*/ 0x1000, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*FIELD_802_3AP_AUTO_NEGOTIATION_COMPLETE */
  /*10*/  {/*baseAddr*/ 0x20004,  /*offsetFormula*/ 0x1000, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*PAGE_RECEIVED */
  /*11*/  {/*baseAddr*/ 0x20004,  /*offsetFormula*/ 0x1000, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*EXTENDED_NEXT_PAGE_STATUS */
  /*12*/  {/*baseAddr*/ 0x20004,  /*offsetFormula*/ 0x1000, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*FIELD_802_3AP_PARALLEL_DETECTION_FAULT */

    /* AUTO_NEGOTIATION_DEVICE_IDENTIFIER_1 */
  /*13*/  {/*baseAddr*/ 0x20008,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*ORGANIZATIONALLY_UNIQUE_IDENTIFIER_BIT_318 */

    /* AUTO_NEGOTIATION_DEVICE_IDENTIFIER_2 */
  /*14*/  {/*baseAddr*/ 0x2000c,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 4 }, /*REVISION_NUMBER */
  /*15*/  {/*baseAddr*/ 0x2000c,  /*offsetFormula*/ 0x1000, /*fieldStart*/  4, /*fieldLen*/ 6 }, /*MODEL_NUMBER */
  /*16*/  {/*baseAddr*/ 0x2000c,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 10, /*fieldLen*/ 6 }, /*ORGANIZATIONALLY_UNIQUE_IDENTIFIER_BIT_1924 */

    /* AUTO_NEGOTIATION_DEVICES_IN_PACKAGE_1 */
  /*17*/  {/*baseAddr*/ 0x20014,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*CLAUSE_22_REGISTERS_PRESENT */
  /*18*/  {/*baseAddr*/ 0x20014,  /*offsetFormula*/ 0x1000, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*PMDPMA_PRESENT */
  /*19*/  {/*baseAddr*/ 0x20014,  /*offsetFormula*/ 0x1000, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*WIS_PRESENT */
  /*20*/  {/*baseAddr*/ 0x20014,  /*offsetFormula*/ 0x1000, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*PCS_PRESENT */
  /*21*/  {/*baseAddr*/ 0x20014,  /*offsetFormula*/ 0x1000, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*PHY_XS_PRESENT */
  /*22*/  {/*baseAddr*/ 0x20014,  /*offsetFormula*/ 0x1000, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*DTE_XS_PRESENT */
  /*23*/  {/*baseAddr*/ 0x20014,  /*offsetFormula*/ 0x1000, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*TC_PRESENT */
  /*24*/  {/*baseAddr*/ 0x20014,  /*offsetFormula*/ 0x1000, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*AUTO_NEGOTIATION_PRESENT */
  /*25*/  {/*baseAddr*/ 0x20014,  /*offsetFormula*/ 0x1000, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*SEPARATED_PMA_1 */
  /*26*/  {/*baseAddr*/ 0x20014,  /*offsetFormula*/ 0x1000, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*SEPARATED_PMA_2 */
  /*27*/  {/*baseAddr*/ 0x20014,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*SEPARATED_PMA_3 */
  /*28*/  {/*baseAddr*/ 0x20014,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*SEPARATED_PMA_4 */

    /* AUTO_NEGOTIATION_DEVICES_IN_PACKAGE_2 */
  /*29*/  {/*baseAddr*/ 0x20018,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*CLAUSE_22_EXTENSION_PRESENT */
  /*30*/  {/*baseAddr*/ 0x20018,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*MARVELL_SPECIFIC_DEVICE_1_PRESENT */
  /*31*/  {/*baseAddr*/ 0x20018,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*MARVELL_SPECIFIC_DEVICE_2_PRESENT */

    /* AUTO_NEGOTIATION_PACKAGE_IDENTIFIER_1 */
  /*32*/  {/*baseAddr*/ 0x20038,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*ORGANIZATIONALLY_UNIQUE_PKG_IDENTIFIER_BIT_318 */

    /* AUTO_NEGOTIATION_PACKAGE_IDENTIFIER_2 */
  /*33*/  {/*baseAddr*/ 0x2003c,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 4 }, /*PKG_REVISION_NUMBER */
  /*34*/  {/*baseAddr*/ 0x2003c,  /*offsetFormula*/ 0x1000, /*fieldStart*/  4, /*fieldLen*/ 6 }, /*PKG_MODEL_NUMBER */
  /*35*/  {/*baseAddr*/ 0x2003c,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 10, /*fieldLen*/ 6 }, /*PKG_ORGANIZATIONALLY_UNIQUE_IDENTIFIER_BIT_1924 */

    /* 802_3AP_LOCAL_DEVICE_BASE_PAGE_ABILITY_1 */
  /*36*/  {/*baseAddr*/ 0x20040,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 5 }, /*LD_SELECTOR_FIELD */
  /*37*/  {/*baseAddr*/ 0x20040,  /*offsetFormula*/ 0x1000, /*fieldStart*/  5, /*fieldLen*/ 5 }, /*LD_ECHOED_NONCE_FIELD */
  /*38*/  {/*baseAddr*/ 0x20040,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*LD_PAUSE_CAPABLE */
  /*39*/  {/*baseAddr*/ 0x20040,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*LD_ASYMMETRIC_PAUSE */
  /*40*/  {/*baseAddr*/ 0x20040,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*LD_REMOTE_FAULT */
  /*41*/  {/*baseAddr*/ 0x20040,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*LD_ACKNOWLEDGE */
  /*42*/  {/*baseAddr*/ 0x20040,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*LD_NEXT_PAGE */

    /* 802_3AP_LOCAL_DEVICE_BASE_PAGE_ABILITY_2 */
  /*43*/  {/*baseAddr*/ 0x20044,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 5 }, /*LD_LINK_PARTNER_TRANSMITTED_NONCE_FIELD */
  /*44*/  {/*baseAddr*/ 0x20044,  /*offsetFormula*/ 0x1000, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*LD_1000BASE_KX_ */
  /*45*/  {/*baseAddr*/ 0x20044,  /*offsetFormula*/ 0x1000, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*LD_10GBASE_KX4 */
  /*46*/  {/*baseAddr*/ 0x20044,  /*offsetFormula*/ 0x1000, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*LD_10GBASE_KR */
  /*47*/  {/*baseAddr*/ 0x20044,  /*offsetFormula*/ 0x1000, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*LD_40GBASE_KR4 */
  /*48*/  {/*baseAddr*/ 0x20044,  /*offsetFormula*/ 0x1000, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*LD_40GBASE_CR4 */
  /*49*/  {/*baseAddr*/ 0x20044,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*LD_100GBASE_CR10 */
  /*50*/  {/*baseAddr*/ 0x20044,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*LD_100GBASE_KP4 */
  /*51*/  {/*baseAddr*/ 0x20044,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*LD_100GBASE_KR4 */
  /*52*/  {/*baseAddr*/ 0x20044,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*LD_100GBASE_CR4 */
  /*53*/  {/*baseAddr*/ 0x20044,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*LD_25GBASE_KR_S_OR_25GBASE_CR_S */
  /*54*/  {/*baseAddr*/ 0x20044,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*LD_25GBASE_KR_OR_25GBASE_CR */

    /* 802_3AP_LOCAL_DEVICE_BASE_PAGE_ABILITY_3 */
  /*55*/  {/*baseAddr*/ 0x20048,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*LD_2_5G_KX */
  /*56*/  {/*baseAddr*/ 0x20048,  /*offsetFormula*/ 0x1000, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*LD_5G_KR */
  /*57*/  {/*baseAddr*/ 0x20048,  /*offsetFormula*/ 0x1000, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*LD_50G_KRCR */
  /*58*/  {/*baseAddr*/ 0x20048,  /*offsetFormula*/ 0x1000, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*LD_100G_KRCR2 */
  /*59*/  {/*baseAddr*/ 0x20048,  /*offsetFormula*/ 0x1000, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*LD_200G_KRCR4 */
  /*60*/  {/*baseAddr*/ 0x20048,  /*offsetFormula*/ 0x1000, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*LD_100G_KRCR */
  /*61*/  {/*baseAddr*/ 0x20048,  /*offsetFormula*/ 0x1000, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*LD_200G_KRCR2 */
  /*62*/  {/*baseAddr*/ 0x20048,  /*offsetFormula*/ 0x1000, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*LD_400G_KRCR4 */
  /*63*/  {/*baseAddr*/ 0x20048,  /*offsetFormula*/ 0x1000, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*LD_200G_KRCR8 */
  /*64*/  {/*baseAddr*/ 0x20048,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*LD_50G_R4_MARVELL_MODE */
  /*65*/  {/*baseAddr*/ 0x20048,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*LD_RS_FEC_INT_REQUESTEDF4 */
  /*66*/  {/*baseAddr*/ 0x20048,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*LD_25G_RS_FEC_REQUESTEDF2 */
  /*67*/  {/*baseAddr*/ 0x20048,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*LD_25G_BASE_R_REQUESTEDF3 */
  /*68*/  {/*baseAddr*/ 0x20048,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*LD_LINK_PARTNER_FEC_ABILITY */
  /*69*/  {/*baseAddr*/ 0x20048,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*LD_LINK_PARTNER_REQUESTING_FEC_ENABLE */

    /* 802_3AP_LINK_PARTNER_BASE_PAGE_ABILITY_1 */
  /*70*/  {/*baseAddr*/ 0x2004c,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 5 }, /*LP_SELECTOR_FIELD */
  /*71*/  {/*baseAddr*/ 0x2004c,  /*offsetFormula*/ 0x1000, /*fieldStart*/  5, /*fieldLen*/ 5 }, /*LP_ECHOED_NONCE_FIELD */
  /*72*/  {/*baseAddr*/ 0x2004c,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*LP_PAUSE_CAPABLE */
  /*73*/  {/*baseAddr*/ 0x2004c,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*LP_ASYMMETRIC_PAUSE */
  /*74*/  {/*baseAddr*/ 0x2004c,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*LP_REMOTE_FAULT */
  /*75*/  {/*baseAddr*/ 0x2004c,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*LP_ACKNOWLEDGE */
  /*76*/  {/*baseAddr*/ 0x2004c,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*LP_NEXT_PAGE */

    /* 802_3AP_LINK_PARTNER_BASE_PAGE_ABILITY_2 */
  /*77*/  {/*baseAddr*/ 0x20050,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 5 }, /*LP_LINK_PARTNER_TRANSMITTED_NONCE_FIELD */
  /*78*/  {/*baseAddr*/ 0x20050,  /*offsetFormula*/ 0x1000, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*LP_1000BASE_KX_ */
  /*79*/  {/*baseAddr*/ 0x20050,  /*offsetFormula*/ 0x1000, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*LP_10GBASE_KX4 */
  /*80*/  {/*baseAddr*/ 0x20050,  /*offsetFormula*/ 0x1000, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*LP_10GBASE_KR */
  /*81*/  {/*baseAddr*/ 0x20050,  /*offsetFormula*/ 0x1000, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*LP_40GBASE_KR4 */
  /*82*/  {/*baseAddr*/ 0x20050,  /*offsetFormula*/ 0x1000, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*LP_40GBASE_CR4 */
  /*83*/  {/*baseAddr*/ 0x20050,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*LP_100GBASE_CR10 */
  /*84*/  {/*baseAddr*/ 0x20050,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*LP_100GBASE_KP4 */
  /*85*/  {/*baseAddr*/ 0x20050,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*LP_100GBASE_KR4 */
  /*86*/  {/*baseAddr*/ 0x20050,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*LP_100GBASE_CR4 */
  /*87*/  {/*baseAddr*/ 0x20050,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*LP_25GBASE_KR_S_OR_25GBASE_CR_S */
  /*88*/  {/*baseAddr*/ 0x20050,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*LP_25GBASE_KR_OR_25GBASE_CR */

    /* 802_3AP_LINK_PARTNER_BASE_PAGE_ABILITY_3 */
  /*89*/  {/*baseAddr*/ 0x20054,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*LP_2_5G_KX */
  /*90*/  {/*baseAddr*/ 0x20054,  /*offsetFormula*/ 0x1000, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*LP_5G_KR */
  /*91*/  {/*baseAddr*/ 0x20054,  /*offsetFormula*/ 0x1000, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*LP_50G_KRCR */
  /*92*/  {/*baseAddr*/ 0x20054,  /*offsetFormula*/ 0x1000, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*LP_100G_KRCR2 */
  /*93*/  {/*baseAddr*/ 0x20054,  /*offsetFormula*/ 0x1000, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*LP_200G_KRCR4 */
  /*94*/  {/*baseAddr*/ 0x20054,  /*offsetFormula*/ 0x1000, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*LP_100G_KRCR */
  /*95*/  {/*baseAddr*/ 0x20054,  /*offsetFormula*/ 0x1000, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*LP_200G_KRCR2 */
  /*96*/  {/*baseAddr*/ 0x20054,  /*offsetFormula*/ 0x1000, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*LP_400G_KRCR4 */
  /*97*/  {/*baseAddr*/ 0x20054,  /*offsetFormula*/ 0x1000, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*LP_200G_KRCR8 */
  /*98*/  {/*baseAddr*/ 0x20054,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*LP_50G_R4_MARVELL_MODE */
  /*99*/  {/*baseAddr*/ 0x20054,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*LP_RS_FEC_INT_REQUESTEDF4 */
  /*100*/  {/*baseAddr*/ 0x20054,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*LP_25G_RS_FEC_REQUESTEDF2 */
  /*101*/  {/*baseAddr*/ 0x20054,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*LP_25G_BASE_R_REQUESTEDF3 */
  /*102*/  {/*baseAddr*/ 0x20054,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*LP_LINK_PARTNER_FEC_ABILITY */
  /*103*/  {/*baseAddr*/ 0x20054,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*LP_LINK_PARTNER_REQUESTING_FEC_ENABLE */

    /* 802_3AP_NEXT_PAGE_TRANSMIT_EXTENDED_NEXT_PAGE_TRANSMIT */
  /*104*/  {/*baseAddr*/ 0x20058,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 11 }, /*LD_NP_MESSAGEUNFORMATTED_FIELD */
  /*105*/  {/*baseAddr*/ 0x20058,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*LD_NP_TOGGLE */
  /*106*/  {/*baseAddr*/ 0x20058,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*LD_NP_ACKNOWLEDGE_2 */
  /*107*/  {/*baseAddr*/ 0x20058,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*LD_NP_MESSAGE_PAGE_MODE */
  /*108*/  {/*baseAddr*/ 0x20058,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*LD_NP_NEXT_PAGE */

    /* 802_3AP_EXTENDED_NEXT_PAGE_TRANSMIT_UNFORMATTED_CODE_FIELD_U0_TO_U15 */
  /*109*/  {/*baseAddr*/ 0x2005c,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*LD_UNFORMATTED_FIELD0_15 */

    /* 802_3AP_EXTENDED_NEXT_PAGE_TRANSMIT_UNFORMATTED_CODE_FIELD_U16_TO_U31 */
  /*110*/  {/*baseAddr*/ 0x20060,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*LD_UNFORMATTED_FIELD16_31 */

    /* 802_3AP_LINK_PARTNER_NEXT_PAGE_LINK_PARTNER_EXTENDED_NEXT_PAGE_ABILITY */
  /*111*/  {/*baseAddr*/ 0x20064,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 11 }, /*LP_NP_MESSAGEUNFORMATTED_FIELD */
  /*112*/  {/*baseAddr*/ 0x20064,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*LP_NP_TOGGLE */
  /*113*/  {/*baseAddr*/ 0x20064,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*LP_NP_ACKNOWLEDGE2 */
  /*114*/  {/*baseAddr*/ 0x20064,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*LP_NP_MESSAGE_PAGE */
  /*115*/  {/*baseAddr*/ 0x20064,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*LP_NP_ACKNOWLEDGE */
  /*116*/  {/*baseAddr*/ 0x20064,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*LP_NP_NEXT_PAGE */

    /* 802_3AP_LINK_PARTNER_EXTENDED_NEXT_PAGE_ABILITY_UNFORMATTED_CODE_FIELD_U0_TO_U15 */
  /*117*/  {/*baseAddr*/ 0x20068,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*LP_UNFORMATTED_FIELD0_15 */

    /* 802_3AP_LINK_PARTNER_EXTENDED_NEXT_PAGE_ABILITY_UNFORMATTED_CODE_FIELD_U16_TO_U31 */
  /*118*/  {/*baseAddr*/ 0x2006c,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*LP_UNFORMATTED_FIELD16_31 */

    /* BACKPLANE_ETHERNET_STATUS */
  /*119*/  {/*baseAddr*/ 0x200c0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*STAT_BP_AN_ABILITY */
  /*120*/  {/*baseAddr*/ 0x200c0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*STAT_FIELD_1000BASE_KX */
  /*121*/  {/*baseAddr*/ 0x200c0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*STAT_FIELD_10GBASE_KX4 */
  /*122*/  {/*baseAddr*/ 0x200c0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*STAT_FIELD_10GBASE_KR */
  /*123*/  {/*baseAddr*/ 0x200c0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*STAT_BASE_R_FEC_NEGOTIATED */
  /*124*/  {/*baseAddr*/ 0x200c0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*STAT_FIELD_40GBASE_KR4 */
  /*125*/  {/*baseAddr*/ 0x200c0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*STAT_FIELD_40GBASE_CR4 */
  /*126*/  {/*baseAddr*/ 0x200c0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*STAT_RS_FEC_NEGOTIATED */
  /*127*/  {/*baseAddr*/ 0x200c0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*STAT_FIELD_100GBASE_CR10 */
  /*128*/  {/*baseAddr*/ 0x200c0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*STAT_FIELD_100GBASE_KP4 */
  /*129*/  {/*baseAddr*/ 0x200c0,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*STAT_FIELD_100GBASE_KR4 */
  /*130*/  {/*baseAddr*/ 0x200c0,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*STAT_FIELD_100GBASE_CR4 */
  /*131*/  {/*baseAddr*/ 0x200c0,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*STAT_FIELD_25GBASE_KR_S_OR_25GBASE_CR_S */
  /*132*/  {/*baseAddr*/ 0x200c0,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*STAT_FIELD_25GBASE_KR_OR_25GBASE_CR */
  /*133*/  {/*baseAddr*/ 0x200c0,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*STAT_FIELD_2_5GBASE_KX */
  /*134*/  {/*baseAddr*/ 0x200c0,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*STAT_FIELD_5GBASE_KR */

    /* BACKPLANE_ETHERNET_STATUS2 */
  /*135*/  {/*baseAddr*/ 0x200c4,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*FIELD_50GBASE_KRCR */
  /*136*/  {/*baseAddr*/ 0x200c4,  /*offsetFormula*/ 0x1000, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*FIELD_100GBASE_KRCR2 */
  /*137*/  {/*baseAddr*/ 0x200c4,  /*offsetFormula*/ 0x1000, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*FIELD_200GBASE_KRCR4 */
  /*138*/  {/*baseAddr*/ 0x200c4,  /*offsetFormula*/ 0x1000, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*FIELD_100GBASE_KRCR */
  /*139*/  {/*baseAddr*/ 0x200c4,  /*offsetFormula*/ 0x1000, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*FIELD_200GBASE_KRCR2 */
  /*140*/  {/*baseAddr*/ 0x200c4,  /*offsetFormula*/ 0x1000, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*FIELD_400GBASE_KRCR4 */
  /*141*/  {/*baseAddr*/ 0x200c4,  /*offsetFormula*/ 0x1000, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*FIELD_RSFEC_INT_S */

    /* ANEG_LANE_0_CONTROL_0 */
  /*142*/  {/*baseAddr*/ 0x20800,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 2 }, /*OVERRIDE_CTRL_S */
  /*143*/  {/*baseAddr*/ 0x20800,  /*offsetFormula*/ 0x1000, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*AP_ANEG_AMDISAM_S */
  /*144*/  {/*baseAddr*/ 0x20800,  /*offsetFormula*/ 0x1000, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*AP_ANEG_BP_REACHED_S */
  /*145*/  {/*baseAddr*/ 0x20800,  /*offsetFormula*/ 0x1000, /*fieldStart*/  4, /*fieldLen*/ 11 }, /*AP_ANEG_STATE_S10_0 */
  /*146*/  {/*baseAddr*/ 0x20800,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*RG_EEE_XNP_SEL_S */

    /* ANEG_LANE_0_CONTROL_1 */
  /*147*/  {/*baseAddr*/ 0x20804,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 4 }, /*AG_LGTH_MATCH_CNT_S3_0 */
  /*148*/  {/*baseAddr*/ 0x20804,  /*offsetFormula*/ 0x1000, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*CG_ANEG_TEST0_S */
  /*149*/  {/*baseAddr*/ 0x20804,  /*offsetFormula*/ 0x1000, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*CG_ANEG_TEST3_S */
  /*150*/  {/*baseAddr*/ 0x20804,  /*offsetFormula*/ 0x1000, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*RG_NO_RINGOSC_S */
  /*151*/  {/*baseAddr*/ 0x20804,  /*offsetFormula*/ 0x1000, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*OW_AS_NONCE_MATCH_S */
  /*152*/  {/*baseAddr*/ 0x20804,  /*offsetFormula*/ 0x1000, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*RG_AS_NONCE_MATCH_S */
  /*153*/  {/*baseAddr*/ 0x20804,  /*offsetFormula*/ 0x1000, /*fieldStart*/  9, /*fieldLen*/ 2 }, /*RG_LINK_FAIL_TIMER_SEL1500_S */
  /*154*/  {/*baseAddr*/ 0x20804,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 11, /*fieldLen*/ 2 }, /*RG_LINK_FAIL_TIMER_SEL500_S */
  /*155*/  {/*baseAddr*/ 0x20804,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 13, /*fieldLen*/ 2 }, /*RG_LINK_FAIL_TIMER_SEL50_S */
  /*156*/  {/*baseAddr*/ 0x20804,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*RG_LINK_FAIL_TIMER_OFF_S */

    /* ANEG_LANE_0_CONTROL_2 */
  /*157*/  {/*baseAddr*/ 0x20808,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*CLEAR_HCD_OW_S */
  /*158*/  {/*baseAddr*/ 0x20808,  /*offsetFormula*/ 0x1000, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*RG_CLEAR_HCD_S */
  /*159*/  {/*baseAddr*/ 0x20808,  /*offsetFormula*/ 0x1000, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*ANEG_ENABLE_OW_S */
  /*160*/  {/*baseAddr*/ 0x20808,  /*offsetFormula*/ 0x1000, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*RG_ANEG_ENABLE_S */
  /*161*/  {/*baseAddr*/ 0x20808,  /*offsetFormula*/ 0x1000, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*RESTART_ANEG_OW_S */
  /*162*/  {/*baseAddr*/ 0x20808,  /*offsetFormula*/ 0x1000, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*RG_RESTART_ANEG_S */
  /*163*/  {/*baseAddr*/ 0x20808,  /*offsetFormula*/ 0x1000, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*LINK_OW_S */
  /*164*/  {/*baseAddr*/ 0x20808,  /*offsetFormula*/ 0x1000, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*RG_LINK_S */
  /*165*/  {/*baseAddr*/ 0x20808,  /*offsetFormula*/ 0x1000, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*HCD_RESOLVED_OW_S */
  /*166*/  {/*baseAddr*/ 0x20808,  /*offsetFormula*/ 0x1000, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*RG_HCD_RESOLVED_S */
  /*167*/  {/*baseAddr*/ 0x20808,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 10, /*fieldLen*/ 2 }, /*RG_LINK_FAIL_TIMER_SEL12P6_S */
  /*168*/  {/*baseAddr*/ 0x20808,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*DISABLE_CFG_CON40GR2_LOC_S */
  /*169*/  {/*baseAddr*/ 0x20808,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*DISABLE_CFG_800GR8_LOC_S */
  /*170*/  {/*baseAddr*/ 0x20808,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*DISABLE_CFG_200GR8_LOC_S */
  /*171*/  {/*baseAddr*/ 0x20808,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*DISABLE_CFG_50GR4_LOC_S */

    /* ANEG_LANE_0_CONTROL_3 */
  /*172*/  {/*baseAddr*/ 0x2080c,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 2 }, /*REG3_RESERVED */
  /*173*/  {/*baseAddr*/ 0x2080c,  /*offsetFormula*/ 0x1000, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*AA_SFEC_ENABLE_S */
  /*174*/  {/*baseAddr*/ 0x2080c,  /*offsetFormula*/ 0x1000, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*AA_FEC_ENABLE_S */
  /*175*/  {/*baseAddr*/ 0x2080c,  /*offsetFormula*/ 0x1000, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*AA_RSFEC_ENABLE_OVERWRITE_VALUE */
  /*176*/  {/*baseAddr*/ 0x2080c,  /*offsetFormula*/ 0x1000, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*AP_RSFEC_ENABLE_OVERWRITE */
  /*177*/  {/*baseAddr*/ 0x2080c,  /*offsetFormula*/ 0x1000, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*AA_FEC_ENABLE_OVERWRITE_VALUE */
  /*178*/  {/*baseAddr*/ 0x2080c,  /*offsetFormula*/ 0x1000, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*AP_FEC_ENABLE_OVERWRITE */
  /*179*/  {/*baseAddr*/ 0x2080c,  /*offsetFormula*/ 0x1000, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*TX_PAUSE_ENABLE_OW_S */
  /*180*/  {/*baseAddr*/ 0x2080c,  /*offsetFormula*/ 0x1000, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*RG_TX_PAUSE_ENABLE_S */
  /*181*/  {/*baseAddr*/ 0x2080c,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*RX_PAUSE_ENABLE_OW_S */
  /*182*/  {/*baseAddr*/ 0x2080c,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*RG_RX_PAUSE_ENABLE_S */
  /*183*/  {/*baseAddr*/ 0x2080c,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*PCS_LOCK_OW_S */
  /*184*/  {/*baseAddr*/ 0x2080c,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*RG_PCS_LOCK_S */
  /*185*/  {/*baseAddr*/ 0x2080c,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*DSP_LOCK_OW_S */
  /*186*/  {/*baseAddr*/ 0x2080c,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*RG_DSP_LOCK_S */

    /* ANEG_LANE_0_CONTROL_4 */
  /*187*/  {/*baseAddr*/ 0x20810,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*AA_PWRUP_G_S */
  /*188*/  {/*baseAddr*/ 0x20810,  /*offsetFormula*/ 0x1000, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*AA_PWRUP_2P5G_S */
  /*189*/  {/*baseAddr*/ 0x20810,  /*offsetFormula*/ 0x1000, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*AA_PWRUP_5GR_S */
  /*190*/  {/*baseAddr*/ 0x20810,  /*offsetFormula*/ 0x1000, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*AA_PWRUP_10GKR_S */
  /*191*/  {/*baseAddr*/ 0x20810,  /*offsetFormula*/ 0x1000, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*AA_PWRUP_CON25GKR_S */
  /*192*/  {/*baseAddr*/ 0x20810,  /*offsetFormula*/ 0x1000, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*AA_PWRUP_CON25GCR_S */
  /*193*/  {/*baseAddr*/ 0x20810,  /*offsetFormula*/ 0x1000, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*AA_PWRUP_IEEE25GS_S */
  /*194*/  {/*baseAddr*/ 0x20810,  /*offsetFormula*/ 0x1000, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*AA_PWRUP_IEEE25GRCR_S */
  /*195*/  {/*baseAddr*/ 0x20810,  /*offsetFormula*/ 0x1000, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*AA_PWRUP_40GKR4_S */
  /*196*/  {/*baseAddr*/ 0x20810,  /*offsetFormula*/ 0x1000, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*AA_PWRUP_40GCR4_S */
  /*197*/  {/*baseAddr*/ 0x20810,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*AA_PWRUP_CON50GKR2_S */
  /*198*/  {/*baseAddr*/ 0x20810,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*AA_PWRUP_CON50GCR2_S */
  /*199*/  {/*baseAddr*/ 0x20810,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*AA_PWRUP_100GKR4_S */
  /*200*/  {/*baseAddr*/ 0x20810,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*AA_PWRUP_100GCR4_S */
  /*201*/  {/*baseAddr*/ 0x20810,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*AA_PWRUP_50GRCR_S */
  /*202*/  {/*baseAddr*/ 0x20810,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*AA_PWRUP_100GRCR2_S */

    /* ANEG_LANE_0_CONTROL_5 */
  /*203*/  {/*baseAddr*/ 0x20814,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*AA_PWRUP_200GRCR4_S */
  /*204*/  {/*baseAddr*/ 0x20814,  /*offsetFormula*/ 0x1000, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*AA_PWRUP_100GRCR_S */
  /*205*/  {/*baseAddr*/ 0x20814,  /*offsetFormula*/ 0x1000, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*AA_PWRUP_200GRCR2_S */
  /*206*/  {/*baseAddr*/ 0x20814,  /*offsetFormula*/ 0x1000, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*AA_PWRUP_400GRCR4_S */
  /*207*/  {/*baseAddr*/ 0x20814,  /*offsetFormula*/ 0x1000, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*UNSUPPORTED_MODES */
  /*208*/  {/*baseAddr*/ 0x20814,  /*offsetFormula*/ 0x1000, /*fieldStart*/  5, /*fieldLen*/ 4 }, /*MY_RESERVED5 */
  /*209*/  {/*baseAddr*/ 0x20814,  /*offsetFormula*/ 0x1000, /*fieldStart*/  9, /*fieldLen*/ 6 }, /*SELECTED_MODES_TO_OVERWRITE */
  /*210*/  {/*baseAddr*/ 0x20814,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*PWRUP_MODES_OW_S */

    /* ANEG_LANE_0_CONTROL_6 */
  /*211*/  {/*baseAddr*/ 0x20818,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*OW_PM_NORM_X_STATE_S */
  /*212*/  {/*baseAddr*/ 0x20818,  /*offsetFormula*/ 0x1000, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*RG_PM_NORM_X_STATE_S */
  /*213*/  {/*baseAddr*/ 0x20818,  /*offsetFormula*/ 0x1000, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*OW_PM_LOOPBACK_S */
  /*214*/  {/*baseAddr*/ 0x20818,  /*offsetFormula*/ 0x1000, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*RG_PM_LOOPBACK_S */
  /*215*/  {/*baseAddr*/ 0x20818,  /*offsetFormula*/ 0x1000, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*OW_FAR_CLEAR_RESET_ALL_S */
  /*216*/  {/*baseAddr*/ 0x20818,  /*offsetFormula*/ 0x1000, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*RG_FAR_CLEAR_RESET_ALL_S */
  /*217*/  {/*baseAddr*/ 0x20818,  /*offsetFormula*/ 0x1000, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*OW_FAR_SET_RESTART_ALL_S */
  /*218*/  {/*baseAddr*/ 0x20818,  /*offsetFormula*/ 0x1000, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*RG_FAR_SET_RESTART_ALL_S */
  /*219*/  {/*baseAddr*/ 0x20818,  /*offsetFormula*/ 0x1000, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*OW_RING_OSC_S */
  /*220*/  {/*baseAddr*/ 0x20818,  /*offsetFormula*/ 0x1000, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*RG_RING_OSC_S */
  /*221*/  {/*baseAddr*/ 0x20818,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 10, /*fieldLen*/ 6 }, /*REG6_RESERVED10 */

    /* ANEG_LANE_0_CONTROL_7 */
  /*222*/  {/*baseAddr*/ 0x2081c,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*RG_LOSTLOCK_VALUE_S */
  /*223*/  {/*baseAddr*/ 0x2081c,  /*offsetFormula*/ 0x1000, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*RG_LOSTLOCK_OW_S */
  /*224*/  {/*baseAddr*/ 0x2081c,  /*offsetFormula*/ 0x1000, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*RG_LOSTLOCK_SEL_S */
  /*225*/  {/*baseAddr*/ 0x2081c,  /*offsetFormula*/ 0x1000, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*RG_TIMER2_LENGTH_S_OW_S */
  /*226*/  {/*baseAddr*/ 0x2081c,  /*offsetFormula*/ 0x1000, /*fieldStart*/  4, /*fieldLen*/ 2 }, /*RG_TIMER2_LENGTH_SEL_S */
  /*227*/  {/*baseAddr*/ 0x2081c,  /*offsetFormula*/ 0x1000, /*fieldStart*/  6, /*fieldLen*/ 10 }, /*MY_RESERVED6 */

    /* ANEG_LANE_0_CONTROL_8 */
  /*228*/  {/*baseAddr*/ 0x20820,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 3 }, /*DET_DATA_TIMER_WINDOW_SELECT_ */
  /*229*/  {/*baseAddr*/ 0x20820,  /*offsetFormula*/ 0x1000, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*RG_DET_DAT_TIMER_OW_S */
  /*230*/  {/*baseAddr*/ 0x20820,  /*offsetFormula*/ 0x1000, /*fieldStart*/  4, /*fieldLen*/ 3 }, /*DET_CLOCK_TIMER_WINDOW_SELECT_ */
  /*231*/  {/*baseAddr*/ 0x20820,  /*offsetFormula*/ 0x1000, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*RG_DET_CLK_TIMER_OW_S */
  /*232*/  {/*baseAddr*/ 0x20820,  /*offsetFormula*/ 0x1000, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*RG_IGNORE_PULSE_TOO_SHORT_S */
  /*233*/  {/*baseAddr*/ 0x20820,  /*offsetFormula*/ 0x1000, /*fieldStart*/  9, /*fieldLen*/ 2 }, /*BREAK_LINK_LENGTH_OVERRIDE_VALUE_SELECT */
  /*234*/  {/*baseAddr*/ 0x20820,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*RG_BREAK_LINK_LENGTH_OW_S */
  /*235*/  {/*baseAddr*/ 0x20820,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*RG_DISABLE_HW_NP_S */
  /*236*/  {/*baseAddr*/ 0x20820,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*RG_CHK_CLK_TRAN_S */
  /*237*/  {/*baseAddr*/ 0x20820,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 14, /*fieldLen*/ 2 }, /*MY_RESERVED14 */

    /* ANEG_LANE_0_CONTROL_9 */
  /*238*/  {/*baseAddr*/ 0x20824,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 5 }, /*SW_AG_MODE */
  /*239*/  {/*baseAddr*/ 0x20824,  /*offsetFormula*/ 0x1000, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*SW_FEC_ENABLE_S */
  /*240*/  {/*baseAddr*/ 0x20824,  /*offsetFormula*/ 0x1000, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*SW_RSFEC_ENABLE_S */
  /*241*/  {/*baseAddr*/ 0x20824,  /*offsetFormula*/ 0x1000, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*SW_RESOLVED_S */
  /*242*/  {/*baseAddr*/ 0x20824,  /*offsetFormula*/ 0x1000, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*SW_LLFEC_ENABLE_S */
  /*243*/  {/*baseAddr*/ 0x20824,  /*offsetFormula*/ 0x1000, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*SW_RSFEC_INT_S */
  /*244*/  {/*baseAddr*/ 0x20824,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 10, /*fieldLen*/ 3 }, /*MY_RESERVED10 */
  /*245*/  {/*baseAddr*/ 0x20824,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*SW_LP_SELECTOR_ENABLE_S */
  /*246*/  {/*baseAddr*/ 0x20824,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*Z80_IO_RESOLVED */
  /*247*/  {/*baseAddr*/ 0x20824,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*SW_AP_ENABLE */

    /* ANEG_LANE_0_CONTROL_10 */
  /*248*/  {/*baseAddr*/ 0x20828,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 5 }, /*ADVERTISED_CON25GKRCR_LOCATION */
  /*249*/  {/*baseAddr*/ 0x20828,  /*offsetFormula*/ 0x1000, /*fieldStart*/  5, /*fieldLen*/ 5 }, /*ADVERTISED_CON50GKRCR_LOCATION */
  /*250*/  {/*baseAddr*/ 0x20828,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 10, /*fieldLen*/ 3 }, /*ADVERTISED_CON400GRCR8_LOCATION */
  /*251*/  {/*baseAddr*/ 0x20828,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 13, /*fieldLen*/ 2 }, /*REG10_RESERVED13 */
  /*252*/  {/*baseAddr*/ 0x20828,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*RG_AP_LOCAL_RESET_S */

    /* ANEG_LANE_0_CONTROL_11 */
  /*253*/  {/*baseAddr*/ 0x2082c,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 3 }, /*REG11_RESERVED */
  /*254*/  {/*baseAddr*/ 0x2082c,  /*offsetFormula*/ 0x1000, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*SYMBOL_LOCK_OVERRIDE_VALUE */
  /*255*/  {/*baseAddr*/ 0x2082c,  /*offsetFormula*/ 0x1000, /*fieldStart*/  4, /*fieldLen*/ 3 }, /*REG11_RESERVED4 */
  /*256*/  {/*baseAddr*/ 0x2082c,  /*offsetFormula*/ 0x1000, /*fieldStart*/  7, /*fieldLen*/ 4 }, /*REG11_RESERVED7 */
  /*257*/  {/*baseAddr*/ 0x2082c,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*CHK_DSP_LOCK_S */
  /*258*/  {/*baseAddr*/ 0x2082c,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 12, /*fieldLen*/ 3 }, /*REG11_RESERVED12 */
  /*259*/  {/*baseAddr*/ 0x2082c,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*SYMBOL_LOCK_SELECT */

    /* ANEG_LANE_0_CONTROL_12 */
  /*260*/  {/*baseAddr*/ 0x20830,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 3 }, /*REG12_RESERVED */
  /*261*/  {/*baseAddr*/ 0x20830,  /*offsetFormula*/ 0x1000, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*SYMBOL_LOCK_OVERRIDE_ENABLE */
  /*262*/  {/*baseAddr*/ 0x20830,  /*offsetFormula*/ 0x1000, /*fieldStart*/  4, /*fieldLen*/ 4 }, /*REG12_MY_RESERVED4 */
  /*263*/  {/*baseAddr*/ 0x20830,  /*offsetFormula*/ 0x1000, /*fieldStart*/  8, /*fieldLen*/ 5 }, /*N_AG_MODE_S */
  /*264*/  {/*baseAddr*/ 0x20830,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*ADV_USE_N_AGMODE_S */
  /*265*/  {/*baseAddr*/ 0x20830,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*RE_AGSTART_USE_N_LINK_S */
  /*266*/  {/*baseAddr*/ 0x20830,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*INTEL_FEC_MODE */

    /* ANEG_LANE_0_CONTROL_13 */
  /*267*/  {/*baseAddr*/ 0x20834,  /*offsetFormula*/ 0x1000, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*CFG_CON800GKRCR8_LOC_S */

    /* ANEG_LANE_0_CONTROL_14 */
  /*268*/  {/*baseAddr*/ 0x20838,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 2 }, /*ADVERTISED_200G_R8_MARVELL_MODE_A15_TO_A22_BIT_LOCATION_SELECT */
  /*269*/  {/*baseAddr*/ 0x20838,  /*offsetFormula*/ 0x1000, /*fieldStart*/  2, /*fieldLen*/ 2 }, /*ADVERTISED_25R2_MODE_A15_TO_A22_BIT_LOCATION_SELECT */
  /*270*/  {/*baseAddr*/ 0x20838,  /*offsetFormula*/ 0x1000, /*fieldStart*/  4, /*fieldLen*/ 2 }, /*ADVERTISED_800G_R8_MARVELL_MODE_A15_TO_A22_BIT_LOCATION_SELECT */
  /*271*/  {/*baseAddr*/ 0x20838,  /*offsetFormula*/ 0x1000, /*fieldStart*/  6, /*fieldLen*/ 2 }, /*ADVERTISED_50G_R4_MARVELL_MODE_A15_TO_A22_BIT_LOCATION_SELECT */
  /*272*/  {/*baseAddr*/ 0x20838,  /*offsetFormula*/ 0x1000, /*fieldStart*/  9, /*fieldLen*/ 3 }, /*ADVERTISED_40G_R2_MODE_A15_TO_A22_BIT_LOCATION_SELECT */
  /*273*/  {/*baseAddr*/ 0x20838,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*CFG_40GR2_PRIO_HIGHER_THAN_40GR4_S */

    /* ANEG_LANE_0_CONTROL_15 */
  /*274*/  {/*baseAddr*/ 0x2083c,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*PM_NORM_X_STATE_S */
  /*275*/  {/*baseAddr*/ 0x2083c,  /*offsetFormula*/ 0x1000, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*FAR_SET_RESTART_ALL_S */
  /*276*/  {/*baseAddr*/ 0x2083c,  /*offsetFormula*/ 0x1000, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*FAR_CLEAR_RESET_ALL_S */
  /*277*/  {/*baseAddr*/ 0x2083c,  /*offsetFormula*/ 0x1000, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*PCS_LINK */
  /*278*/  {/*baseAddr*/ 0x2083c,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*SYMBOL_LOCK */

    /* ANEG_LANE_0_CONTROL_16_CONSORTIUM_MP5_0 */
  /*279*/  {/*baseAddr*/ 0x20840,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 11 }, /*REG16_MESSAGEUNFORMATTED_FIELD */
  /*280*/  {/*baseAddr*/ 0x20840,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*REG16_TOGGLE */
  /*281*/  {/*baseAddr*/ 0x20840,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*REG16_ACKNOWLEDGE2 */
  /*282*/  {/*baseAddr*/ 0x20840,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*REG16_MESSAGE_PAGE */
  /*283*/  {/*baseAddr*/ 0x20840,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*REG16_ACKNOWLEDGE */
  /*284*/  {/*baseAddr*/ 0x20840,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*REG16_NEXT_PAGE */

    /* ANEG_LANE_0_CONTROL_17_CONSORTIUM_MP5_1 */
  /*285*/  {/*baseAddr*/ 0x20844,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 11 }, /*REG17_OUI_23_13 */
  /*286*/  {/*baseAddr*/ 0x20844,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 11, /*fieldLen*/ 5 }, /*REG17_RESERVED11 */

    /* ANEG_LANE_0_CONTROL_18_CONSORTIUM_MP5_2 */
  /*287*/  {/*baseAddr*/ 0x20848,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 11 }, /*REG18_OUI_12_2 */
  /*288*/  {/*baseAddr*/ 0x20848,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 11, /*fieldLen*/ 5 }, /*REG18_RESERVED11 */

    /* ANEG_LANE_0_CONTROL_19_CONSORTIUM_ET_0 */
  /*289*/  {/*baseAddr*/ 0x2084c,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 9 }, /*REG19_MESSAGEUNFORMATTED_FIELD */
  /*290*/  {/*baseAddr*/ 0x2084c,  /*offsetFormula*/ 0x1000, /*fieldStart*/  9, /*fieldLen*/ 2 }, /*REG19_OUI_1_0 */
  /*291*/  {/*baseAddr*/ 0x2084c,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*REG19_TOGGLE */
  /*292*/  {/*baseAddr*/ 0x2084c,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*REG19_ACKNOWLEDGE2 */
  /*293*/  {/*baseAddr*/ 0x2084c,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*REG19_MESSAGE_PAGE */
  /*294*/  {/*baseAddr*/ 0x2084c,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*REG19_ACKNOWLEDGE */
  /*295*/  {/*baseAddr*/ 0x2084c,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*REG19_NEXT_PAGE */

    /* ANEG_LANE_0_CONTROL_20_CONSORTIUM_ET_1 */
  /*296*/  {/*baseAddr*/ 0x20850,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 4 }, /*REG20_RESERVED */
  /*297*/  {/*baseAddr*/ 0x20850,  /*offsetFormula*/ 0x1000, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*REG20_CONSORTIUM_25G_KR1 */
  /*298*/  {/*baseAddr*/ 0x20850,  /*offsetFormula*/ 0x1000, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*REG20_CONSORTIUM_25G_CR1 */
  /*299*/  {/*baseAddr*/ 0x20850,  /*offsetFormula*/ 0x1000, /*fieldStart*/  6, /*fieldLen*/ 2 }, /*REG20_CONSORTIUM_RESERVED */
  /*300*/  {/*baseAddr*/ 0x20850,  /*offsetFormula*/ 0x1000, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*REG20_CONSORTIUM_50G_KR2 */
  /*301*/  {/*baseAddr*/ 0x20850,  /*offsetFormula*/ 0x1000, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*REG20_CONSORTIUM_50G_CR2 */
  /*302*/  {/*baseAddr*/ 0x20850,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 10, /*fieldLen*/ 5 }, /*REG20_RESERVED10 */
  /*303*/  {/*baseAddr*/ 0x20850,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*PRE_AG_ADV_CON800GKRCR8_S */

    /* ANEG_LANE_0_CONTROL_21_CONSORTIUM_ET_2 */
  /*304*/  {/*baseAddr*/ 0x20854,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*REG21_MARVELL_CONSORTIUM_40GR2 */
  /*305*/  {/*baseAddr*/ 0x20854,  /*offsetFormula*/ 0x1000, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*REG21_RESERVED1 */
  /*306*/  {/*baseAddr*/ 0x20854,  /*offsetFormula*/ 0x1000, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*REG21_MARVELL_CONSORTIUM_400GR8 */
  /*307*/  {/*baseAddr*/ 0x20854,  /*offsetFormula*/ 0x1000, /*fieldStart*/  3, /*fieldLen*/ 2 }, /*REG21_RESERVED3 */
  /*308*/  {/*baseAddr*/ 0x20854,  /*offsetFormula*/ 0x1000, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*REG21_CONSORTIUM_LF1 */
  /*309*/  {/*baseAddr*/ 0x20854,  /*offsetFormula*/ 0x1000, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*REG21_CONSORTIUM_LF2 */
  /*310*/  {/*baseAddr*/ 0x20854,  /*offsetFormula*/ 0x1000, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*REG21_CONSORTIUM_LF3 */
  /*311*/  {/*baseAddr*/ 0x20854,  /*offsetFormula*/ 0x1000, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*REG21_CONSORTIUM_F1 */
  /*312*/  {/*baseAddr*/ 0x20854,  /*offsetFormula*/ 0x1000, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*REG21_CONSORTIUM_F2 */
  /*313*/  {/*baseAddr*/ 0x20854,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*REG21_CONSORTIUM_F3 */
  /*314*/  {/*baseAddr*/ 0x20854,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*REG21_CONSORTIUM_F4 */
  /*315*/  {/*baseAddr*/ 0x20854,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*REG21_CONSORTIUM_LFR */
  /*316*/  {/*baseAddr*/ 0x20854,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 13, /*fieldLen*/ 3 }, /*REG21_RESERVED13 */

    /* ANEG_LANE_0_CONTROL_22_LINK_PARTNER_CONSORTIUM_MP5_0 */
  /*317*/  {/*baseAddr*/ 0x20858,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 11 }, /*REG22_MESSAGEUNFORMATTED_FIELD */
  /*318*/  {/*baseAddr*/ 0x20858,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*REG22_TOGGLE */
  /*319*/  {/*baseAddr*/ 0x20858,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*REG22_ACKNOWLEDGE2 */
  /*320*/  {/*baseAddr*/ 0x20858,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*REG22_MESSAGE_PAGE */
  /*321*/  {/*baseAddr*/ 0x20858,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*REG22_ACKNOWLEDGE */
  /*322*/  {/*baseAddr*/ 0x20858,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*REG22_NEXT_PAGE */

    /* ANEG_LANE_0_CONTROL_23_LINK_PARTNER_CONSORTIUM_MP5_1 */
  /*323*/  {/*baseAddr*/ 0x2085c,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 11 }, /*REG23_OUI_23_13 */

    /* ANEG_LANE_0_CONTROL_24_LINK_PARTNER_CONSORTIUM_MP5_2 */
  /*324*/  {/*baseAddr*/ 0x20860,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 11 }, /*REG24_OUI_12_2 */
  /*325*/  {/*baseAddr*/ 0x20860,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 11, /*fieldLen*/ 5 }, /*REG24_RESERVED11 */

    /* ANEG_LANE_0_CONTROL_25_LINK_PARTNER_CONSORTIUM_ET_0 */
  /*326*/  {/*baseAddr*/ 0x20864,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 9 }, /*REG25_MESSAGEUNFORMATTED_FIELD */
  /*327*/  {/*baseAddr*/ 0x20864,  /*offsetFormula*/ 0x1000, /*fieldStart*/  9, /*fieldLen*/ 2 }, /*REG25_OUI_1_0 */
  /*328*/  {/*baseAddr*/ 0x20864,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*REG25_TOGGLE */
  /*329*/  {/*baseAddr*/ 0x20864,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*REG25_ACKNOWLEDGE2 */
  /*330*/  {/*baseAddr*/ 0x20864,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*REG25_MESSAGE_PAGE */
  /*331*/  {/*baseAddr*/ 0x20864,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*REG25_ACKNOWLEDGE */
  /*332*/  {/*baseAddr*/ 0x20864,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*REG25_NEXT_PAGE */

    /* ANEG_LANE_0_CONTROL_26_LINK_PARTNER_CONSORTIUM_ET_1 */
  /*333*/  {/*baseAddr*/ 0x20868,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 4 }, /*REG26_RESERVED */
  /*334*/  {/*baseAddr*/ 0x20868,  /*offsetFormula*/ 0x1000, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*REG26_CONSORTIUM_25G_KR1 */
  /*335*/  {/*baseAddr*/ 0x20868,  /*offsetFormula*/ 0x1000, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*REG26_CONSORTIUM_25G_CR1 */
  /*336*/  {/*baseAddr*/ 0x20868,  /*offsetFormula*/ 0x1000, /*fieldStart*/  6, /*fieldLen*/ 2 }, /*REG26_CONSORTIUM_RESERVED */
  /*337*/  {/*baseAddr*/ 0x20868,  /*offsetFormula*/ 0x1000, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*REG26_CONSORTIUM_50G_KR2 */
  /*338*/  {/*baseAddr*/ 0x20868,  /*offsetFormula*/ 0x1000, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*REG26_CONSORTIUM_50G_CR2 */
  /*339*/  {/*baseAddr*/ 0x20868,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 10, /*fieldLen*/ 5 }, /*REG26_RESERVED10 */
  /*340*/  {/*baseAddr*/ 0x20868,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*REG26_CONSORTIUM_800G_CRKR8 */

    /* ANEG_LANE_0_CONTROL_27_LINK_PARTNER_CONSORTIUM_ET_2 */
  /*341*/  {/*baseAddr*/ 0x2086c,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*REG27_MARVELL_CONSORTIUM_40GR2 */
  /*342*/  {/*baseAddr*/ 0x2086c,  /*offsetFormula*/ 0x1000, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*REG27_RESERVED1 */
  /*343*/  {/*baseAddr*/ 0x2086c,  /*offsetFormula*/ 0x1000, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*REG27_MARVELL_CONSORTIUM_400GR8 */
  /*344*/  {/*baseAddr*/ 0x2086c,  /*offsetFormula*/ 0x1000, /*fieldStart*/  3, /*fieldLen*/ 2 }, /*REG27_RESERVED3 */
  /*345*/  {/*baseAddr*/ 0x2086c,  /*offsetFormula*/ 0x1000, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*REG27_CONSORTIUM_LF1 */
  /*346*/  {/*baseAddr*/ 0x2086c,  /*offsetFormula*/ 0x1000, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*REG27_CONSORTIUM_LF2 */
  /*347*/  {/*baseAddr*/ 0x2086c,  /*offsetFormula*/ 0x1000, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*REG27_CONSORTIUM_LF3 */
  /*348*/  {/*baseAddr*/ 0x2086c,  /*offsetFormula*/ 0x1000, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*REG27_CONSORTIUM_F1 */
  /*349*/  {/*baseAddr*/ 0x2086c,  /*offsetFormula*/ 0x1000, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*REG27_CONSORTIUM_F2 */
  /*350*/  {/*baseAddr*/ 0x2086c,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*REG27_CONSORTIUM_F3 */
  /*351*/  {/*baseAddr*/ 0x2086c,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*REG27_CONSORTIUM_F4 */
  /*352*/  {/*baseAddr*/ 0x2086c,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*REG27_CONSORTIUM_LFR */
  /*353*/  {/*baseAddr*/ 0x2086c,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 13, /*fieldLen*/ 3 }, /*REG27_RESERVED13 */

    /* ANEG_Z80_MESSAGE_0 */
  /*354*/  {/*baseAddr*/ 0x20880,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*ANEG_Z80_MESSAGE0 */

    /* ANEG_Z80_MESSAGE_1 */
  /*355*/  {/*baseAddr*/ 0x20884,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*ANEG_Z80_MESSAGE1 */

    /* ANEG_Z80_MESSAGE_2 */
  /*356*/  {/*baseAddr*/ 0x20888,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*ANEG_Z80_MESSAGE2 */

    /* ANEG_Z80_MESSAGE_3 */
  /*357*/  {/*baseAddr*/ 0x2088c,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 16 }, /*ANEG_Z80_MESSAGE3 */

    /* ANEG_INTERRUPT_CONTROL_AUTONEG_COMPLETE */
  /*358*/  {/*baseAddr*/ 0x20890,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*REG24_AP_INTERRUPT_MASK */

    /* AUTONEG_COMPLETE_INT_STATUS */
  /*359*/  {/*baseAddr*/ 0x20894,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*REG25_AP_INTERRUPT_STATUS */

    /* ANEG_INTERRUPT_CONTROL_HCD_RESOLVE */
  /*360*/  {/*baseAddr*/ 0x20898,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*REG26_AP_INTERRUPT_MASK */

    /* HCD_RESOLVE_INT_STATUS */
  /*361*/  {/*baseAddr*/ 0x2089c,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*REG27_AP_INTERRUPT_STATUS */

    /* ANEG_INTERRUPT_CONTROL_COMPLETE_ACK */
  /*362*/  {/*baseAddr*/ 0x208a0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*REG28_AP_INTERRUPT_MASK */

    /* COMPETE_ACK_INT_STATUS */
  /*363*/  {/*baseAddr*/ 0x208a4,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*REG29_AP_INTERRUPT_STATUS */

    /* ANEG_TX_FSM_TRAP */
  /*364*/  {/*baseAddr*/ 0x208a8,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 2 }, /*REG2A_AP_TRAP_FUNCTION */
  /*365*/  {/*baseAddr*/ 0x208a8,  /*offsetFormula*/ 0x1000, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*REG2A_AP_FORCE_BREAKPOINT_EN */
  /*366*/  {/*baseAddr*/ 0x208a8,  /*offsetFormula*/ 0x1000, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*REG2A_AP_BREAKPOINT_REACHED */
  /*367*/  {/*baseAddr*/ 0x208a8,  /*offsetFormula*/ 0x1000, /*fieldStart*/  4, /*fieldLen*/ 4 }, /*REG2A_AP_OVERRIDE_STATE */

    /* ANEG_RX_FSM_TRAP */
  /*368*/  {/*baseAddr*/ 0x208ac,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 2 }, /*REG2B_AP_TRAP_FUNCTION */
  /*369*/  {/*baseAddr*/ 0x208ac,  /*offsetFormula*/ 0x1000, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*REG2B_AP_FORCE_BREAKPOINT_EN */
  /*370*/  {/*baseAddr*/ 0x208ac,  /*offsetFormula*/ 0x1000, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*REG2B_AP_BREAKPOINT_REACHED */
  /*371*/  {/*baseAddr*/ 0x208ac,  /*offsetFormula*/ 0x1000, /*fieldStart*/  4, /*fieldLen*/ 3 }, /*REG2B_AP_OVERRIDE_STATE */

    /* ANEG_LANE_0_MARVELL_PAGE_CONTROL_REG */
  /*372*/  {/*baseAddr*/ 0x208c0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*REG30_800GR8 */
  /*373*/  {/*baseAddr*/ 0x208c0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  1, /*fieldLen*/ 1 }, /*REG30_200GR8 */
  /*374*/  {/*baseAddr*/ 0x208c0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  2, /*fieldLen*/ 1 }, /*REG30_100GR4_RSFEC544 */
  /*375*/  {/*baseAddr*/ 0x208c0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  3, /*fieldLen*/ 1 }, /*REG30_50GR4 */
  /*376*/  {/*baseAddr*/ 0x208c0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*REG30_50GNRZ */
  /*377*/  {/*baseAddr*/ 0x208c0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*REG30_40GR1 */
  /*378*/  {/*baseAddr*/ 0x208c0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  6, /*fieldLen*/ 1 }, /*REG30_40GR2 */
  /*379*/  {/*baseAddr*/ 0x208c0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  7, /*fieldLen*/ 1 }, /*REG30_25GR2 */
  /*380*/  {/*baseAddr*/ 0x208c0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*REG30_BACKWARD_COMPATIBLE_EN */
  /*381*/  {/*baseAddr*/ 0x208c0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*REG30_RG_X9340PA0_CMN_MODE_40GR1 */
  /*382*/  {/*baseAddr*/ 0x208c0,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*REG30_RG_X9340PA0_CMN_MODE_25GR2 */
  /*383*/  {/*baseAddr*/ 0x208c0,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*REG30_RG_X9340PA0_CMN_MODE_50GNRZ */
  /*384*/  {/*baseAddr*/ 0x208c0,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*REG30_RG_X9340PA0_CMN_MODE_100GR4_RSFEC544 */

    /* ANEG_LANE_0_MARVELL_PAGE_MP5_REGISTER_0 */
  /*385*/  {/*baseAddr*/ 0x208c4,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 11 }, /*REG31_MESSAGEUNFORMATTED_FIELD */
  /*386*/  {/*baseAddr*/ 0x208c4,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*REG31_TOGGLE */
  /*387*/  {/*baseAddr*/ 0x208c4,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*REG31_ACKNOWLEDGE2 */
  /*388*/  {/*baseAddr*/ 0x208c4,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*REG31_MESSAGE_PAGE */
  /*389*/  {/*baseAddr*/ 0x208c4,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*REG31_ACKNOWLEDGE */
  /*390*/  {/*baseAddr*/ 0x208c4,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*REG31_NEXT_PAGE */

    /* ANEG_LANE_0_MARVELL_PAGE_MP5_REGISTER_1 */
  /*391*/  {/*baseAddr*/ 0x208c8,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 11 }, /*REG32_OUI_23_13 */

    /* ANEG_LANE_0_MARVELL_PAGE_MP5_REGISTER_2 */
  /*392*/  {/*baseAddr*/ 0x208cc,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 11 }, /*REG33_OUI_12_2 */

    /* ANEG_LANE_0_MARVELL_PAGE_ET_REGISTER_0 */
  /*393*/  {/*baseAddr*/ 0x208d0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 9 }, /*REG34_MESSAGEUNFORMATTED_FIELD */
  /*394*/  {/*baseAddr*/ 0x208d0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  9, /*fieldLen*/ 2 }, /*REG34_OUI_1_0 */
  /*395*/  {/*baseAddr*/ 0x208d0,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*REG34_TOGGLE */
  /*396*/  {/*baseAddr*/ 0x208d0,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*REG34_ACKNOWLEDGE2 */
  /*397*/  {/*baseAddr*/ 0x208d0,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*REG34_MESSAGE_PAGE */
  /*398*/  {/*baseAddr*/ 0x208d0,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*REG34_ACKNOWLEDGE */
  /*399*/  {/*baseAddr*/ 0x208d0,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*REG34_NEXT_PAGE */

    /* ANEG_LANE_0_MARVELL_PAGE_ET_REGISTER_1 */
  /*400*/  {/*baseAddr*/ 0x208d4,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*REG35_25GR2 */
  /*401*/  {/*baseAddr*/ 0x208d4,  /*offsetFormula*/ 0x1000, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*REG35_40GR2 */
  /*402*/  {/*baseAddr*/ 0x208d4,  /*offsetFormula*/ 0x1000, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*REG35_40GR1 */
  /*403*/  {/*baseAddr*/ 0x208d4,  /*offsetFormula*/ 0x1000, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*REG35_50GNRZ */
  /*404*/  {/*baseAddr*/ 0x208d4,  /*offsetFormula*/ 0x1000, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*REG35_50GR4 */
  /*405*/  {/*baseAddr*/ 0x208d4,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*REG35_100GR4_RSFEC544 */
  /*406*/  {/*baseAddr*/ 0x208d4,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*REG35_200GR8 */
  /*407*/  {/*baseAddr*/ 0x208d4,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*REG35_800GR8 */

    /* ANEG_LANE_0_MARVELL_PAGE_ET_REGISTER_2 */
  /*408*/  {/*baseAddr*/ 0x208d8,  /*offsetFormula*/ 0x1000, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*REG36_F1 */
  /*409*/  {/*baseAddr*/ 0x208d8,  /*offsetFormula*/ 0x1000, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*REG36_F2 */
  /*410*/  {/*baseAddr*/ 0x208d8,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*REG36_F3 */
  /*411*/  {/*baseAddr*/ 0x208d8,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*REG36_F4 */

    /* ANEG_LANE_0_LINK_PARTNER_MARVELL_PAGE_MP5_REGISTER_0 */
  /*412*/  {/*baseAddr*/ 0x208dc,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 11 }, /*REG37_MESSAGEUNFORMATTED_FIELD */
  /*413*/  {/*baseAddr*/ 0x208dc,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*REG37_TOGGLE */
  /*414*/  {/*baseAddr*/ 0x208dc,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*REG37_ACKNOWLEDGE2 */
  /*415*/  {/*baseAddr*/ 0x208dc,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*REG37_MESSAGE_PAGE */
  /*416*/  {/*baseAddr*/ 0x208dc,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*REG37_ACKNOWLEDGE */
  /*417*/  {/*baseAddr*/ 0x208dc,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*REG37_NEXT_PAGE */

    /* ANEG_LANE_0_LINK_PARTNER_MARVELL_PAGE_MP5_REGISTER_1 */
  /*418*/  {/*baseAddr*/ 0x208e0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 11 }, /*REG38_OUI_23_13 */

    /* ANEG_LANE_0_LINK_PARTNER_MARVELL_PAGE_MP5_REGISTER_2 */
  /*419*/  {/*baseAddr*/ 0x208e4,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 11 }, /*REG39_OUI_12_2 */

    /* ANEG_LANE_0_LINK_PARTNER_MARVELL_PAGE_ET_REGISTER_0 */
  /*420*/  {/*baseAddr*/ 0x208e8,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 9 }, /*REG3A_MESSAGEUNFORMATTED_FIELD */
  /*421*/  {/*baseAddr*/ 0x208e8,  /*offsetFormula*/ 0x1000, /*fieldStart*/  9, /*fieldLen*/ 2 }, /*REG3A_OUI_1_0 */
  /*422*/  {/*baseAddr*/ 0x208e8,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*REG3A_TOGGLE */
  /*423*/  {/*baseAddr*/ 0x208e8,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 12, /*fieldLen*/ 1 }, /*REG3A_ACKNOWLEDGE2 */
  /*424*/  {/*baseAddr*/ 0x208e8,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*REG3A_MESSAGE_PAGE */
  /*425*/  {/*baseAddr*/ 0x208e8,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 14, /*fieldLen*/ 1 }, /*REG3A_ACKNOWLEDGE */
  /*426*/  {/*baseAddr*/ 0x208e8,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*REG3A_NEXT_PAGE */

    /* ANEG_LANE_0_LINK_PARTNER_MARVELL_PAGE_ET_REGISTER_1 */
  /*427*/  {/*baseAddr*/ 0x208ec,  /*offsetFormula*/ 0x1000, /*fieldStart*/  0, /*fieldLen*/ 1 }, /*REG3B_25GR2 */
  /*428*/  {/*baseAddr*/ 0x208ec,  /*offsetFormula*/ 0x1000, /*fieldStart*/  4, /*fieldLen*/ 1 }, /*REG3B_40GR2 */
  /*429*/  {/*baseAddr*/ 0x208ec,  /*offsetFormula*/ 0x1000, /*fieldStart*/  5, /*fieldLen*/ 1 }, /*REG3B_40GR1 */
  /*430*/  {/*baseAddr*/ 0x208ec,  /*offsetFormula*/ 0x1000, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*REG3B_50GNRZ */
  /*431*/  {/*baseAddr*/ 0x208ec,  /*offsetFormula*/ 0x1000, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*REG3B_50GR4 */
  /*432*/  {/*baseAddr*/ 0x208ec,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*REG3B_100GR4_RSFEC544 */
  /*433*/  {/*baseAddr*/ 0x208ec,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 13, /*fieldLen*/ 1 }, /*REG3B_200GR8 */
  /*434*/  {/*baseAddr*/ 0x208ec,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 15, /*fieldLen*/ 1 }, /*REG3B_800GR8 */

    /* ANEG_LANE_0_LINK_PARTNER_MARVELL_PAGE_ET_REGISTER_2 */
  /*435*/  {/*baseAddr*/ 0x208f0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*REG3C_F1 */
  /*436*/  {/*baseAddr*/ 0x208f0,  /*offsetFormula*/ 0x1000, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*REG3C_F2 */
  /*437*/  {/*baseAddr*/ 0x208f0,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*REG3C_F3 */
  /*438*/  {/*baseAddr*/ 0x208f0,  /*offsetFormula*/ 0x1000, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*REG3C_F4 */
};

