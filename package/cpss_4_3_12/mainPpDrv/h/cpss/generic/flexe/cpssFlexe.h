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

#ifndef _cpssFlexeh
#define _cpssFlexeh

#ifdef _cplusplus
extern "C" {
#endif /* _cplusplus */

#include <cpss/generic/flexe/cpssFlexeTypes.h>

/**
 * @internal cpssFlexeHwInit function
 * @endinternal
 *
 * @brief initialize an FlexE Hardware.
 *
 * @param[in] devNum        - device number
 * @param[in] portGroupsBmp - Bitmap of Port Groups
 *
 * @retval GT_OK                    - on success
 * @retval GT_FAIL                  - on IPC error
 * @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
 */
GT_STATUS cpssFlexeHwInit
(
    IN  GT_U8               devNum,
    IN  GT_PORT_GROUPS_BMP  portGroupsBmp
);

/**
 * @internal cpssFlexeHwSoftReset function
 * @endinternal
 *
 * @brief Reset an FlexE Hardware
 *
 * @note Before calling this API, all the groups should be deleted.
 *
 * @param[in] devNum        - device number
 * @param[in] portGroupsBmp - Bitmap of Port Groups
 *
 * @retval GT_OK                    - on success
 * @retval GT_FAIL                  - on IPC error
 * @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
 */
GT_STATUS cpssFlexeHwSoftReset
(
    IN  GT_U8               devNum,
    IN  GT_PORT_GROUPS_BMP  portGroupsBmp
);

/**
 * @internal cpssFlexePhyConfigGet function
 * @endinternal
 *
 * @brief Get Phy configuration
 *
 * @param[in] devNum    - device number
 * @param[in] phyId     - PHY instance number in the flexe domain.
 * @param[in] phyCfgPtr - (pointer to) phy configuration struture
 *
 * @retval GT_OK                    - on success
 * @retval GT_FAIL                  - on IPC error
 * @retval GT_OUT_OF_RANGE          - on out of range error
 * @retval GT_BAD_PARAM             - on illegal parameter.
 * @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
 */
GT_STATUS cpssFlexePhyConfigGet
(
    IN  GT_U8                      devNum,
    IN  GT_U32                     phyId,
    OUT CPSS_FLEXE_PHY_CONFIG_STC  *phyCfgPtr
);

/**
 * @internal cpssFlexeGroupCreate function
 * @endinternal
 *
 * @brief Create an FlexE group and add related PHYs
 *
 * @param[in] devNum       - device number
 * @param[in] groupNum      - FlexE group number
 * @param[in] groupInfoPtr - (pointer to) FlexE group information structure
 *
 * @retval GT_OK                    - on success
 * @retval GT_FAIL                  - on IPC error
 * @retval GT_OUT_OF_RANGE          - on out of range error
 * @retval GT_BAD_PARAM             - on illegal parameter.
 * @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
 */
GT_STATUS cpssFlexeGroupCreate
(
    IN  GT_U8                          devNum,
    IN  GT_U32                         groupNum,
    IN  CPSS_FLEXE_GROUP_INFO_STC      *groupInfoPtr
);

/**
 * @internal cpssFlexeGroupGet function
 * @endinternal
 *
 * @brief Get FlexE group information of a specific group number
 *
 * @param[in] devNum       - device number
 * @param[in] groupNum     - FlexE group number
 * @param[out] groupInfoPtr - (pointer to) FlexE group information structure 
 * @param[out] firstPhyIdPtr- (pointer to) First PHY ID. All the PHYs are sequential.
 *                            If the number for PHYs is 4, PHY IDs will be firstPhyId,
 *                            firstPhyId+1, firstPhyId+2 and firstPhyId+3.
 *
 * @retval GT_OK                    - on success
 * @retval GT_FAIL                  - on IPC error
 * @retval GT_OUT_OF_RANGE          - on out of range error
 * @retval GT_BAD_PARAM             - on illegal parameter.
 * @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
 */
GT_STATUS cpssFlexeGroupGet
(
    IN   GT_U8                      devNum,
    IN   GT_U32                     groupNum,
    OUT  CPSS_FLEXE_GROUP_INFO_STC  *groupInfoPtr,
    OUT  GT_U32                     *firstPhyIdPtr
);

/**
 * @internal cpssFlexeGroupDelete function
 * @endinternal
 *
 * @brief   Delete one FlexE group.
 *
 * @param[in] devNum      - device number
 * @param[in] groupNum    - FlexE group number
 *
 * @retval GT_OK                    - on success
 * @retval GT_OUT_OF_RANGE          - on out of range error
 * @retval GT_BAD_PARAM             - on illegal parameter.
 * @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
 */
GT_STATUS cpssFlexeGroupDelete
(
    IN  GT_U8           devNum,
    IN  GT_U8           groupNum
);

/**
 * @internal cpssFlexeGroupCalendarSwitch function
 * @endinternal
 *
 * @brief   Initialize a calendar switch on a given group
 *
 * @param[in] devNum      - device number
 * @param[in] groupNum    - FlexE group number
 * @param[in] paramsPtr   - calendar switch parameters
 *
 * @retval GT_OK                    - on success
 * @retval GT_OUT_OF_RANGE          - on out of range error
 * @retval GT_BAD_PARAM             - on illegal parameter.
 * @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
 */
GT_STATUS cpssFlexeGroupCalendarSwitch
(
    IN  GT_U8                                      devNum,
    IN  GT_U32                                     groupNum,
    IN  CPSS_FLEXE_GROUP_CALENDAR_SWITCH_PARAMS_STC *paramsPtr
);

/**
 * @internal cpssFlexeGroupCalendarGet function
 * @endinternal
 *
 * @brief   Get calendar settings of a specific group
 *
 * @param[in] devNum         - device number
 * @param[in] groupNum       - FlexE group number
 * @param[in] instanceNum    - instance number
 * @param[out] calTypePtr - (pointer to) Calendar type
 *
 * @retval GT_OK                    - on success
 * @retval GT_OUT_OF_RANGE          - on out of range error
 * @retval GT_BAD_PARAM             - on illegal parameter.
 * @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
 */
GT_STATUS cpssFlexeGroupCalendarGet
(
    IN  GT_U8                             devNum,
    IN  GT_U32                            groupNum,
    IN  CPSS_FLEXE_TRANSMIT_DIRECTION_ENT direction,
    OUT CPSS_FLEXE_CALENDAR_INFO_STC      *calInfoPtr
);

/**
 * @internal cpssFlexeGroupActiveCalendarGet function
 * @endinternal
 *
 * @brief   Get active calendar of a specific group
 *
 * @param[in] devNum         - device number
 * @param[in] groupNum       - FlexE group number
 * @param[in] instanceNum    - instance number
 * @param[out] calTypePtr - (pointer to) Calendar type
 *
 * @retval GT_OK                    - on success
 * @retval GT_OUT_OF_RANGE          - on out of range error
 * @retval GT_BAD_PARAM             - on illegal parameter.
 * @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
 */
GT_STATUS cpssFlexeGroupActiveCalendarGet
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          groupNum,
    IN  GT_U32                          instanceNum,
    OUT CPSS_FLEXE_CALENDAR_TYPE_ENT    *calTypePtr
);

/**
 * @internal cpssFlexeClientAdd function
 * @endinternal
 *
 * @brief Add a client to active group
 *
 * @param[in] devNum       - device number
 * @param[in] groupNum     - Flexe Group number
 * @param[in] clientInfoPtr - (pointer to) client info structure
 *
 * @retval GT_OK                    - on success
 * @retval GT_FAIL                  - on IPC error
 * @retval GT_OUT_OF_RANGE          - on out of range error
 * @retval GT_BAD_PARAM             - on illegal parameter.
 * @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
 */
GT_STATUS cpssFlexeClientAdd
(
    IN  GT_U8                             devNum,
    IN  GT_U32                            clientNum,
    IN  CPSS_FLEXE_CLIENT_INFO_STC        *clientInfoPtr
);

/**
 * @internal cpssFlexeClientDelete function
 * @endinternal
 *
 * @brief Delete a client from active group
 *
 * @param[in] devNum       - device number
 * @param[in] clientNum    - Client Number
 *
 * @retval GT_OK                    - on success
 * @retval GT_OUT_OF_RANGE          - on out of range error
 * @retval GT_BAD_PARAM             - on illegal parameter.
 * @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
 */
GT_STATUS cpssFlexeClientDelete
(
    IN  GT_U8                             devNum,
    IN  GT_U32                            clientNum
);

/**
 * @internal cpssFlexeClientGet function
 * @endinternal
 *
 * @brief Get a client allocation for a given group
 *
 * @param[in]  devNum        - device number
 * @param[in]  clientNum     - client number
 * @param[out] clientInfoPtr - (pointre to) client Info structure
 *
 * @retval GT_OK                    - on success
 * @retval GT_OUT_OF_RANGE          - on out of range error
 * @retval GT_BAD_PARAM             - on illegal parameter.
 * @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
 */
GT_STATUS cpssFlexeClientGet
(
    IN  GT_U8                             devNum,
    IN  GT_U32                            clientNum,
    OUT CPSS_FLEXE_CLIENT_INFO_STC        *clientInfoPtr
);

/**
 * @internal cpssFlexeClientTrafficModeSet function
 *
 * @brief Enable/disable traffic for a given client
 *
 * @param[in] devNum    -  device number
 * @param[in] clientId  -  client ID
 * @param[in] enable    -  GT_TRUE - enable
 *                         GT_FALSE - disable
 *
 * @retval GT_OK                    - on success
 * @retval GT_OUT_OF_RANGE          - on out of range error
 * @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
 */
GT_STATUS cpssFlexeClientTrafficModeSet
(
    IN  GT_U8      devNum,
    IN  GT_U32     clientNum,
    IN  GT_BOOL    enable
);

/**
 * @internal cpssFlexeClientTrafficModeGet function
 *
 * @brief Get Enable/disable staus of traffic for a given client
 *
 * @param[in] devNum    -  device number
 * @param[in] clientId  -  client ID
 * @param[out] enablePtr - (pointer to) enable status.
 *                          GT_TRUE - enable
 *                          GT_FALSE - disable
 *
 * @retval GT_OK                    - on success
 * @retval GT_OUT_OF_RANGE          - on out of range error
 * @retval GT_NOT_APPLICABLE_DEVICE - on devNum of not applicable device.
 */
GT_STATUS cpssFlexeClientTrafficModeGet
(
    IN  GT_U8      devNum,
    IN  GT_U32     clientNum,
    OUT GT_BOOL    *enablePtr
);

#ifdef _cplusplus
}
#endif /* _cplusplus */

#endif /* _cpssFlexeh */

