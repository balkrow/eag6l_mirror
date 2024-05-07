/* Copyright (C) 2023 Marvell International Ltd.
   This program is provided "as is" without any warranty of any kind, and is distributed under the applicable Marvell limited use license agreement. */

/**
 * @file-docAll core\ez_bringup\h\interface\private\prvPdlPpReg.h.
 *
 * @brief   Declares the prvPdlSmiXsmi class
 */

#ifndef __prvPdlPpRegh
#define __prvPdlPpRegh
/**
********************************************************************************
 * @file prvPdlPpReg.h
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
 * @brief Platform driver layer - Private packet processor register related API (internal)
 *
 * @version   1
********************************************************************************
*/

#include <pdl/common/pdlTypes.h>
#include <pdl/interface/pdlInterfaceDebug.h>
#include <pdl/interface/private/prvPdlInterface.h>
#include <pdlib/xml/private/prvXmlParser.h>

/**
* @addtogroup PpReg
* @{
*/

/**
 * @fn  PDL_STATUS prvPdlPpRegHwGetValue ( IN PDL_INTERFACE_TYP interfaceId, OUT UINT_32 * dataPtr )
 *
 * @brief   PDL packet processor register hardware get value
 *
 * @param           interfaceId Identifier for the interface.
 * @param [in,out]  dataPtr     If non-null, the data pointer.
 *
 * @return  A PDL_STATUS.
 */
PDL_STATUS prvPdlPpRegHwGetValue(
    IN  PDL_INTERFACE_TYP                       interfaceId,
    OUT UINT_32                               * dataPtr
);

/*$ END OF prvPdlPpRegHwGetValue */

/**
 * @fn  PDL_STATUS prvPdlPpRegHwSetValue ( IN PDL_INTERFACE_TYP interfaceId, IN UINT_32 data )
 *
 * @brief   PDL packet processor register hardware set value
 *
 * @param   interfaceId Identifier for the interface.
 * @param   data        The data.
 *
 * @return  A PDL_STATUS.
 */
PDL_STATUS prvPdlPpRegHwSetValue(
    IN  PDL_INTERFACE_TYP                       interfaceId,
    IN  UINT_32                                 data
);

/*$ END OF prvPdlPpRegHwSetValue */

/**
 * @fn  PDL_STATUS prvPdlPpRegInterfaceRegister ( IN UINT_8 devNum, IN UINT_32 regAddr, IN UINT_32 mask, OUT PDL_INTERFACE_TYP * interfaceIdPtr )
 *
 * @brief   Register packet processor register interface
 *
 * @param [in]  devNum          device number
 * @param [in]  regAddr         PP register address
 * @param [in]  mask            mask for register operation
 * @param [out] interfaceIdPtr  Identifier for the interface.
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlPpRegInterfaceRegister(
    IN UINT_8               devNum,
    IN UINT_32              regAddr,
    IN UINT_32              mask,
    OUT PDL_INTERFACE_TYP * interfaceIdPtr
);

/*$ END OF prvPdlPpRegInterfaceRegister */

/**
 * @fn  PDL_STATUS prvPdlPpRegInit ( IN PDL_OS_INIT_TYPE_ENT initType )
 *
 * @brief   Init packet processor register  module
 *
 * @return  A PDL_STATUS.
 */
PDL_STATUS prvPdlPpRegInit(
    IN  void
);

/*$ END OF prvPdlPpRegInit */

/**
 * @fn  PDL_STATUS prvPdlPpRegDestroy ()
 *
 * @brief   release all memory allocated by this module
 *
 * @return  A PDL_STATUS.
 */

PDL_STATUS prvPdlPpRegDestroy(
    void
);

/*$ END OF prvPdlPpRegDestroy */

/* @}*/
/* @}*/
/* @}*/

#endif
