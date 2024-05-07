/* Copyright (C) 2023 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */

/**
 * @file-docAll core\ez_bringup\h\interface\private\prvPdlSmiXsmi.h.
 *
 * @brief   Declares the prvPdlSmiXsmi class
 */

#ifndef __prvPdlSmiXsmih
#define __prvPdlSmiXsmih
/**
********************************************************************************
 * @file prvPdlSmiXsmi.h
 * @copyright
 *    (c), Copyright (C) 2023, Marvell International Ltd.
 *    THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.
 *    NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT
 *    OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE
 *    DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.
 *    THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,
 *    IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.
********************************************************************************
 *
 * @brief Platform driver layer - Private SMI/XSMI related API (internal)
 *
 * @version   1
********************************************************************************
*/

#include <pdl/common/pdlTypes.h>
#include <pdl/interface/pdlInterfaceDebug.h>
#include <pdl/interface/private/prvPdlInterface.h>
#include <pdlib/xml/private/prvXmlParser.h>

/**
* @addtogroup Interface
* @{
*/

/**
* @addtogroup SMI/XSMI
* @{
*/

#define CPSS_PORT_GROUP_UNAWARE_MODE_CNS    0

typedef struct {
    PDL_INTERFACE_SMI_XSMI_STC               publicInfo;
} PDL_INTERFACE_PRV_SMI_XSMI_DB_STC;

/**
 * @fn  PDL_STATUS prvPdlSmiXsmiInterfaceRegister ( IN UINT_8 dev, IN UINT_8 address, IN UINT_8 interfaceId, IN PDL_INTERFACE_EXTENDED_TYPE_ENT interfaceType, OUT PDL_INTERFACE_TYP * interfaceIdPtr )
 *
 * @brief   Register smi/xsmi interface usage
 *
 * @param [in]  dev                         device number
 * @param [in]  address                     address
 * @param [in]  interfaceId                 interface id
 * @param [in]  interfaceType               interface type (smi/xsmi)
 * @param [out] interfaceIdPtr              Identifier for the interface.
 *
 * @return  A PDL_STATUS.
 */
PDL_STATUS prvPdlSmiXsmiInterfaceRegister(
    IN UINT_8                           dev,
    IN UINT_8                           address,
    IN UINT_8                           interfaceId,
    IN PDL_INTERFACE_EXTENDED_TYPE_ENT  interfaceType,
    OUT PDL_INTERFACE_TYP             * interfaceIdPtr
);
/*$ END OF prvPdlSmiXsmiInterfaceRegister */

/**
 * @fn  PDL_STATUS prvPdlMpdSmiXsmiInterfaceRegister ( UINT_32 mpdLogicalPort, OUT PDL_INTERFACE_TYP * interfaceIdPtr )
 *
 * @brief   Register mpd interface usage
 *
 * @param [in]  mpdLogicalPort              mpd logical port number
 * @param [out] interfaceIdPtr              Identifier for the interface.
 *
 * @return  A PDL_STATUS.
 */
PDL_STATUS prvPdlMpdSmiXsmiInterfaceRegister(
    IN  UINT_32                         mpdLogicalPort,
    OUT PDL_INTERFACE_TYP             * interfaceIdPtr
);

/*$ END OF prvPdlMpdSmiXsmiInterfaceRegister */

/**
 * @fn  PDL_STATUS PdlSmiXsmiInit ( IN void )
 *
 * @brief   Pdl SMI/XSMI initialize
 *
 * @param   void    The void.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS pdlSmiXsmiInit(
    IN  void
);


/**
 * @fn  PDL_STATUS prvPdlSmiXsmiDestroy ()
 *
 * @brief   release all memory allocated by this module
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlSmiXsmiDestroy(
    void
);

/* @}*/
/* @}*/
/* @}*/

#endif
