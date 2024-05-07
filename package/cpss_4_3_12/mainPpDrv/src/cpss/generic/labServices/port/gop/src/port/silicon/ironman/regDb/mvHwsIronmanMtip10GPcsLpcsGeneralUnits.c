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
* @file mvHwsIronmanMtip10GPcsLpcsGeneralUnits.c
*
* @brief IronmanL 10G PCS LPCS register DB
*
* @version   1
********************************************************************************
*/

#include <gtOs/gtGenTypes.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/siliconAddress.h>

const MV_HWS_REG_ADDR_FIELD_STC mvHwsIronmanLpcsGeneralUnitsDb[] = {
    /* GMODE */
  /*0*/  {/*baseAddr*/ 0x3e0,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*LPCS_ENABLE */
  /*1*/  {/*baseAddr*/ 0x3e0,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 1 }, /*QSGMII_0_ENABLE */
  /*2*/  {/*baseAddr*/ 0x3e0,  /*offsetFormula*/ 0x0, /*fieldStart*/  9, /*fieldLen*/ 1 }, /*QSGMII_1_ENABLE */
  /*3*/  {/*baseAddr*/ 0x3e0,  /*offsetFormula*/ 0x0, /*fieldStart*/ 10, /*fieldLen*/ 1 }, /*USGMII8_ENABLE */
  /*4*/  {/*baseAddr*/ 0x3e0,  /*offsetFormula*/ 0x0, /*fieldStart*/ 11, /*fieldLen*/ 1 }, /*USGMII_SCRAMBLE_ENABLE */

    /* GSTATUS */
  /*5*/  {/*baseAddr*/ 0x3e4,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*GSYNC_STATUS */
  /*6*/  {/*baseAddr*/ 0x3e4,  /*offsetFormula*/ 0x0, /*fieldStart*/  8, /*fieldLen*/ 8 }, /*GAN_DONE_STATUS */

    /* M0 */
  /*7*/  {/*baseAddr*/ 0x3e8,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 4 }, /*PORTSENABLE */

    /* M1 */
  /*8*/  {/*baseAddr*/ 0x3ec,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 4 }, /*PORTSENABLE */

    /* CFG_CLOCK_RATE */
  /*9*/  {/*baseAddr*/ 0x3f0,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 4 }, /*CFGCLOCKRATE */

    /* USXGMII_ENABLE_INDICATION */
  /*10*/  {/*baseAddr*/ 0x3f4,  /*offsetFormula*/ 0x0, /*fieldStart*/  0, /*fieldLen*/ 8 }, /*USXGMII_ENABLE_IND */
};

