/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/
/**
********************************************************************************
* @file cpssAppPlatformI2cUtils.h
*
* @brief Host I2C device driver module, for nokm mode
*
* @version   3
********************************************************************************
*/

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <gtExtDrv/drivers/gtI2cDrv.h>

/**
* @internal prvUtilsHostI2cInit function
* @endinternal
*
* @brief   This routine init host I2C device
*
* @retval GT_OK                    - on success.
* @retval GT_NO_RESOURCE           - in case some resource failed to init
*
*/
GT_STATUS prvUtilsHostI2cInit(void);

/**
* @internal prvUtilsHostI2cDetect function
* @endinternal
*
* @brief   This routine detects all I2C slaves on a given bus, by writing '0' to
*          the the slave at offset 0.
* Note that calling this func, will modify slaves which have configurable
* register at offset 0 - such as MUXes.
*
* @param[in] bus_id        - the I2C bus id.
*
* @retval GT_OK
*/

GT_STATUS prvUtilsHostI2cDetect(
    IN GT_U8    bus_id
);

/**
 * @internal prvUtilsHostI2cDump function
 * @endinternal
 *
 * @param bus_id
 * @param slave_address
 * @param offset_type
 * @param offset
 * @param size
 *
 * @return GT_STATUS
 */
GT_STATUS prvUtilsHostI2cDump(
    IN GT_U8    bus_id,
    IN GT_U8    slave_address,
    IN EXT_DRV_I2C_OFFSET_TYPE_ENT   offset_type,
    IN GT_U32   offset,
    IN GT_U32   size
);

/**
 * @internal prvUtilsHostI2cRead functionh
 * @endinternal
 *
 * @param bus_id
 * @param slave_address
 * @param offset_type
 * @param offset
 * @param size
 * @param buffer
 *
 * @return GT_STATUS
 */
GT_STATUS prvUtilsHostI2cRead(
    IN GT_U8    bus_id,
    IN GT_U8    slave_address,
    IN EXT_DRV_I2C_OFFSET_TYPE_ENT   offset_type,
    IN GT_U32   offset,
    IN GT_U32   size,
    GT_U8 * buffer
);

/**
 *
 * @internal prvUtilsHostI2cWrite function
 * @endinternal
 *
 * @param bus_id
 * @param slave_address
 * @param offset_type
 * @param offset
 * @param nof_args
 * @param arg1
 * @param arg2
 * @param arg3
 * @param arg4
 *
 * @return GT_STATUS
 */
GT_STATUS prvUtilsHostI2cWrite(
    IN GT_U8    bus_id,
    IN GT_U8    slave_address,
    IN EXT_DRV_I2C_OFFSET_TYPE_ENT   offset_type,
    IN GT_U32   offset,
    IN GT_U8   nof_args,
    IN GT_U8   arg1,
    IN GT_U8   arg2,
    IN GT_U8   arg3,
    IN GT_U8   arg4
);

/**
 *
 * @internal prvUtilsHostI2cPrintErrorEnableSet function
 * @endinternal
 *
 * @param param[in] en - GT_TRUE - enable printing I2C functions errors
 *                       GT_FALSE - disable printing I2C functions errors
 *
 * @return GT_STATUS
 */
GT_STATUS prvUtilsHostI2cPrintErrorEnableSet(
    IN GT_BOOL en
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

