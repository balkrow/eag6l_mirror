/*******************************************************************************
*              (c), Copyright 2022, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*/
/**
********************************************************************************
* @file cpssFlexe.h
*
* @brief CPSS FlexE API definitions
*
* @version   1
********************************************************************************
*/

#ifndef _cpssFlexePhyh
#define _cpssFlexePhyh

#ifdef _cplusplus
extern "C" {
#endif /* _cplusplus */

#include <cpss/generic/flexe/cpssFlexeTypes.h>

/**
 * @internal prvCpssFlexePhyCreate function
 * @endinternal
 *
 * @brief Create a FlexE PHY
 *
 * @param[in] devNum    - device number
 * @param[in] phyCfgPtr - (pointer to) phy configuration struture
 *
 * @retval GT_OK                    - on success
 * @retval GT_FAIL                  - on IPC error
 * @retval GT_OUT_OF_RANGE          - on out of range error
 * @retval GT_BAD_PARAM             - on illegal parameter.
 * @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
 */
GT_STATUS prvCpssFlexePhyCreate
(
    IN  GT_U8                          devNum,
    IN  GT_U32                         phyId,
    IN  CPSS_FLEXE_PHY_CONFIG_STC      *phyCfgPtr
);

/**
 * @internal prvCpssFlexePhyDelete function
 * @endinternal
 *
 * @brief Delete a specific PHY
 *
 * @param[in] devNum    - device number
 * @param[in] phyId     - PHY instance number in the flexe domain.
 *
 * @retval GT_OK                    - on success
 * @retval GT_FAIL                  - on IPC error
 * @retval GT_OUT_OF_RANGE          - on out of range error
 * @retval GT_BAD_PARAM             - on illegal parameter.
 * @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
 */
GT_STATUS prvCpssFlexePhyDelete
(
    IN  GT_U8                      devNum,
    IN  GT_U32                     phyId
);

/**
 * @internal prvCpssFlexePortModeSpeedPhySet function
 * @endinternal
 *
 * @brief Create/Delete a FlexE PHY
 *
 * @param[in] devNum    - device number
 * @param[in] macNum    - mac number
 * @param[in] ifMode    - interface Mode
 * @param[in] speed     - PHY speed
 * @param[in] powerUp   - GT_TRUE  - Initialize
 *                        GT_FALSE - DeInitialize
 *
 * @retval GT_OK                    - on success
 * @retval GT_OUT_OF_RANGE          - on out of range error
 * @retval GT_BAD_PARAM             - on illegal parameter.
 * @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
 */
GT_STATUS prvCpssFlexePortModeSpeedPhySet
(
    IN  GT_U8                          devNum,
    IN  GT_U32                         macNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT   ifMode,
    IN  CPSS_PORT_SPEED_ENT            speed,
    IN  GT_BOOL                        powerUp
);

/**
 * @internal prvCpssFlexePortModeSpeedMacSet function
 * @endinternal
 *
 * @brief Initialize/deInitialize FlexE MAC
 *
 * @param[in] devNum    - device number
 * @param[in] macNum    - mac number
 * @param[in] ifMode    - interface Mode
 * @param[in] speed     - MAC speed
 * @param[in] powerUp   - GT_TRUE  - Initialize
 *                        GT_FALSE - DeInitialize
 *
 * @retval GT_OK                    - on success
 * @retval GT_OUT_OF_RANGE          - on out of range error
 * @retval GT_BAD_PARAM             - on illegal parameter.
 * @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
 */
GT_STATUS prvCpssFlexePortModeSpeedMacSet
(
    IN  GT_U8                          devNum,
    IN  GT_U32                         macNum,
    IN  CPSS_PORT_INTERFACE_MODE_ENT   ifMode,
    IN  CPSS_PORT_SPEED_ENT            speed,
    IN  GT_BOOL                        powerUp
);


#ifdef _cplusplus
}
#endif /* _cplusplus */

#endif /* _cpssFlexePorth */

