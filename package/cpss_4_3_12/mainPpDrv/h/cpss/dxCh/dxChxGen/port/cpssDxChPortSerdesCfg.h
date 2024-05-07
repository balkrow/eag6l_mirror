/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
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
* @file cpssDxChPortSerdesCfg.h
*
* @brief CPSS implementation for TX micro burst detection.
*
* @version  1
********************************************************************************

*/

#ifndef __cpssDxChPortSerdesCfgh
#define __cpssDxChPortSerdesCfgh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

GT_STATUS lion2PortSerdesPowerStatusSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  CPSS_PORT_DIRECTION_ENT direction,
    IN  GT_U32                  lanesBmp,
    IN  GT_BOOL                 powerUp
);

/**
* @struct CPSS_DXCH_PORT_SERDES_MAP_INFO_STC
 *
 * @brief A structure to hold physical port mapping information.
*/
typedef struct{

    /** @brief portNum  - Port number. */
    GT_U32 portNum;

    /** @brief macNum  - macNum number. */
    GT_U32 macNum;

    /** @brief serdesNum  - serdes number. */
    GT_U32 serdesNum;

    /** @brief serdesNum  - polarity of Tx serdes. */
    GT_BOOL polarityTx;

    /** @brief serdesNum  - polarity of Rx serdes. */
    GT_BOOL polarityRx;

} CPSS_DXCH_PORT_SERDES_MAP_INFO_STC;

/**
* @internal cpssDxChPortPhysicalSerdesMappingsGet function
* @endinternal
*
* @brief   Get all internal mappings: Port-Mac-SerDes
*
* @param[in] devNum                - The CPSS devNum.
* @param[in] portNum               - The Port number.
* @param[in] enablePrint           - Boolean value that enables or disables printing to stdout.
* @param[in] infoArraySizePtr      - The array size, variable's value will be updated based on the number of values actually written.
* @param[in] portMapInfoInfoArray  - The CPSS portMapInfoInfoArray.
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PARAM             - wrong *infoArraySizePtr,
* @retval GT_FAIL                  - otherwise.
*
*/
GT_STATUS cpssDxChPortPhysicalSerdesMappingsGet
(
    IN    GT_U8                     devNum,
    IN    GT_PHYSICAL_PORT_NUM      portNum,
    IN    GT_BOOL                   enablePrint,
    INOUT GT_U32                    *infoArraySizePtr,
    OUT CPSS_DXCH_PORT_SERDES_MAP_INFO_STC     portMapInfoInfoArray[]
);

/**
* @internal cpssDxChPortSerdesLaneSwappingModeSet function
* @endinternal
*
* @brief  Sets lane per port muxing mode.
*         2 modes are supported (0 and 1) for QSGMII/QXGMII port modes.
*         SerDeses 0-8 support those modes, configurable per GoP
*         There are 3 GoPs:
*         GoP0: SerdeSes 0-2, GoP1: SerdeSes 3-5, GoP2: SerdeSes 6-8
*         Must be configured before ports creation in same
*         cluster/GoP
*
* @note   APPLICABLE DEVICES:      Ironman
* @note   NOT APPLICABLE DEVICES:  All other
*
* @param[in] devNum                   - physical device number
* @param[in] portNum                  - physical port number
* @param[in] mode                     - selected mode (valid values: 0-1)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_FAIL                  - on error
* @retval GT_OUT_OF_RANGE          - out of range value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Mux mode is applied to entire GoP
*
*/
GT_STATUS cpssDxChPortSerdesLaneSwappingModeSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  GT_U8                   mode
);

/**
* @internal cpssDxChPortSerdesLaneSwappingModeGet function
* @endinternal
*
* @brief  Gets lane per port muxing mode.
*         2 modes are supported (0 and 1) for QSGMII/QXGMII port modes.
*         SerDeses 0-8 support those modes, configurable per GoP
*         There are 3 GoPs:
*         GoP0: SerdeSes 0-2, GoP1: SerdeSes 3-5, GoP2: SerdeSes 6-8
*
* @note   APPLICABLE DEVICES:      Ironman
* @note   NOT APPLICABLE DEVICES:  All other
*
* @param[in] devNum                - physical device number
* @param[in] portNum               - physical port number
* @param[out] modePtr              - pointer to muxing mode
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_FAIL                  - on error
* @retval GT_OUT_OF_RANGE          - out of range value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Mux mode is valid to entire GoP
*
*/
GT_STATUS cpssDxChPortSerdesLaneSwappingModeGet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    OUT  GT_U8                  *modePtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChPortSerdesCfgh */

