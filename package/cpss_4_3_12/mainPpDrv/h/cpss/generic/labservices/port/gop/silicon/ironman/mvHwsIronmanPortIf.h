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
* @file mvHwsIornmanPortIf.h
*
* @brief Ironman port interface header file
*
* @version   1
********************************************************************************
*/

#ifndef __mvHwsIronmanPortIf_H
#define __mvHwsIronmanPortIf_H

#ifdef __cplusplus
extern "C" {
#endif

#include <gtOs/gtGenTypes.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortInitIf.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortTypes.h>
#include <cpss/common/labServices/port/gop/port/mvHwsPortMiscIf.h>
#include <cpss/common/labServices/port/gop/common/siliconIf/mvSiliconIf.h>


/**
* @internal hwsIronmanPortsElementsCfg function
* @endinternal
*
* @brief   Init Hawk port modes
*
* @param[in] devNum                   - system device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS hwsIronmanPortsElementsCfg
(
    IN GT_U8              devNum,
    IN GT_U32             portGroup
);

/**
* @internal mvHwsIronmanPortInit function
* @endinternal
*
* @brief   Hawk port init
*
* @param[in] devNum             - system device number
* @param[in] portGroup          - Port Group
* @param[in] phyPortNum         - Physical Port Number
* @param[in] portMode           - Port Mode
* @param[in] portInitInParamPtr - Input parameters
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS mvHwsIronmanPortInit
(
    IN GT_U8                           devNum,
    IN GT_U32                          portGroup,
    IN GT_U32                          phyPortNum,
    IN MV_HWS_PORT_STANDARD            portMode,
    IN MV_HWS_PORT_INIT_INPUT_PARAMS   *portInitInParamPtr
);

GT_STATUS hwsIronmanPortParamsSet
(
    IN GT_U8                   devNum,
    IN GT_U32                  portGroup,
    IN GT_U32                  phyPortNum,
    IN MV_HWS_PORT_STANDARD    portMode
);

GT_STATUS hwsIronmanPortParamsGetLanes
(
    GT_U8                       devNum,
    GT_U32                      portGroup,
    GT_U32                      phyPortNum,
    MV_HWS_PORT_INIT_PARAMS*    portParams
);

/**
* @internal mvHwsIronmanEthPortPowerUp function
* @endinternal
*
* @brief  port init seq
*
* @param[in] devNum                 - system device number
* @param[in] phyPortNum             - physical port number
* @param[in] portMode               - port mode
* @param[in] portInitInParamPtr     - port info struct
*
* @retval 0                         - on success
* @retval 1                         - on error
*/
GT_STATUS mvHwsIronmanEthPortPowerUp
(
    IN GT_U8                            devNum,
    IN GT_U32                           phyPortNum,
    IN MV_HWS_PORT_STANDARD             portMode,
    IN MV_HWS_PORT_INIT_INPUT_PARAMS    *portInitInParamPtr
);

/**
* @internal mvHwsIronmanEthPortPowerDown function
* @endinternal
*
* @brief  port power down seq
*
* @param[in] devNum                   - system device number
* @param[in] phyPortNum               - physical port number
* @param[in] portMode                 - port mode
* @param[in] stateInPortGroup         - first/existing/last int port group
* @param[in] stateInSerdesGroup       - first/existing/last int serdes group
*
* @retval 0                        - on success
* @retval 1                        - on error
*/
GT_STATUS mvHwsIronmanEthPortPowerDown
(
    GT_U8                       devNum,
    GT_U32                      phyPortNum,
    MV_HWS_PORT_STANDARD        portMode,
    MV_HWS_PORT_IN_GROUP_ENT    stateInPortGroup,
    MV_HWS_PORT_IN_GROUP_ENT    stateInSerdesGroup
);

/**
* @internal mvHwsIronmanExtMacClockEnableGet function
* @endinternal
*
* @brief  EXT unit clock enable get
*
*
* @param[in] devNum          - system device number
* @param[in] portNum         - physical port number
* @param[in] portMode        - port mode
* @param[out] enablePtr      - (pointer to) port channel enable /disable
*
* @retval 0                  - on success
* @retval 1                  - on error
*/
GT_STATUS mvHwsIronmanExtMacClockEnableGet
(
    IN GT_U8                        devNum,
    IN GT_U32                       portNum,
    IN MV_HWS_PORT_STANDARD         portMode,
    IN GT_BOOL                      *enablePtr
);

/**
* @internal mvHwsWindowSizeIndicationGet function
* @endinternal
*
* @brief  Get window size (indication of EOM width for 100FX only for Ironman only)
*
*
* @param[in] devNum          - system device number
* @param[in] portNum         - physical port number
* @param[out] regData        - (pointer to) EOM width 100FX
*
* @retval 0                  - on success
* @retval 1                  - on error
*/
GT_STATUS mvHwsWindowSizeIndicationGet
(
    IN  GT_U8  devNum,
    IN  GT_U32 portNum,
    OUT GT_U32 *regData
);

#ifdef __cplusplus
}
#endif

#endif /* __mvHwsIronmanPortIf_H */

