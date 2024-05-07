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
* @file gtAppDemoTmConfig.h
*
* @brief Initialization functions for the TM.
*
* @version   2.
********************************************************************************
*/
#ifndef __gtAppDemoTmCsRefConfigh
#define __gtAppDemoTmCsRefConfigh

/**
 * @internal appDemoTmPrintOutEnable function
 * @endinternal
 *
 * @brief  Eanble/Disable debug prints
 *
 * @note   APPLICABLE DEVICES:      Caelum.
 *
 * @param[in] enable - GT_TRUE/GT_FALSE to enable/disable
 *                     debug prints
 *
 * @retval GT_OK   -   on success
 */
GT_STATUS appDemoTmPrintOutEnable
(
    IN GT_BOOL enable
);

/**
* @internal appDemoTmNumOfTmPortSet function
* @endinternal
*
* @brief   This API set the total number of ports enabled for TM
*
* @note   APPLICABLE DEVICES:      Caelum.
* @note   NOT APPLICABLE DEVICES:  All The rest.
*
* @param[in] numOfPorts - Number of ports used by TM
*
* @retval GT_OK                    - on success
*/
GT_STATUS appDemoTmNumOfTmPortSet
(
    IN GT_U32      numOfPorts
);

/**
* @internal appDemoTmPortIndexSet function
* @endinternal
*
* @brief   This API set a port to be enable for TM in the port list array
*
* @note   APPLICABLE DEVICES:      Caelum.
* @note   NOT APPLICABLE DEVICES:  All The rest.
*
* @param[in] portIndex    - port index
* @param[in] portNum      - port number
*
* @retval GT_OK                    - on success
*/

GT_STATUS appDemoTmPortIndexSet
(
    IN  GT_U32      portIndex   ,
    IN  GT_PORT_NUM portNum
);

/**
 * @internal  appDemoTmScenario17PortInit function
 * @endinternal
 *
 * @brief Initialize Traffic Manager for 17 ports scenario
 *
 * @note  APPLICALBE_DEVICES - Caelum
 *
 * @param[in] devNum - device number
 *
 * @retval GT_OK   -  on success
 */
GT_STATUS appDemoTmScenario17PortInit
(
    GT_U8   devNum
);

/**
 * @internal  appDemoTmScenario17PortConfigSet function
 * @endinternal
 *
 * @brief  Configure Traffic Manager for 17 port scenario
 *
 * @note  APPLICALBE_DEVICES - Caelum
 *
 * @param[in] devNum  - device number
 *
 * @retval   GT_OK  - on success
 */
GT_STATUS appDemoTmScenario17PortConfigSet
(
    GT_U8   devNum
);

/**
* @internal appDemoEgressPortDbDump function
* @endinternal
*
* @brief   This API clear the ports DB
*
*
* @note   APPLICABLE DEVICES:      Caelum.
* @note   NOT APPLICABLE DEVICES:  All The rest.
*
* @param[in] portNum        - a port Num (not the index )
*
* @retval    GT_OK      - on success
*/
GT_STATUS appDemoEgressPortDbDump
(
    GT_PORT_NUM portNum
);

/**
 * @internal appDemoTmFlowAdd function
 * @endinternal
 *
 * @brief   This API map Tag0 and Tag1 and UP0 to TM queues
 *
 * @note   APPLICABLE DEVICES     -  Caelum
 *
 * @param[in]    devNum        - device number
 * @param[in]    sTag          - service Vlan tag vlan ID
 * @param[in]    cTag          - customer Vlan tag vlan ID
 * @param[in]    bNodeInd      - B node index
 * @param[inout] aNodeIndPtr   - (pointer to) A node index
 * @param[in]    egressPortNum - egress port number
 *
 * @retval  GT_OK - on success
 */
GT_STATUS appDemoTmFlowAdd
(
    IN      GT_U8       devNum          ,
    IN      GT_U16      sTag            ,
    IN      GT_U16      cTag            ,
    IN      GT_U32      bNodeInd        ,
    INOUT   GT_U32      *aNodeIndPtr    , /* This is the global aNode index in the TM */
    IN      GT_PORT_NUM egressPortNum /* jus to avoid confusion */
);

/**
* @internal appDemoTmFlowRemove function
* @endinternal
*
* @brief   This API unmap Tag0 and Tag1 and UP0 to TM queues
*
*
* @note   APPLICABLE DEVICES:      Caelum.
* @note   NOT APPLICABLE DEVICES:  All The rest.
*
 * @param[in]    devNum        - device number
 * @param[in]    sTag          - service Vlan tag vlan ID
 * @param[in]    cTag          - customer Vlan tag vlan ID
 * @param[in]    bNodeInd      - B node index
 * @param[inout] aNodeIndPtr   - (pointer to) A node index
 * @param[in]    egressPortNum - egress port number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS appDemoTmFlowRemove
(
    IN      GT_U8       devNum          ,
    IN      GT_U16      sTag            ,
    IN      GT_U16      cTag            ,
    IN      GT_U32      bNodeInd        ,
    IN      GT_U32      aNodeInd       , /* This is the global aNode index in the TM */
    IN      GT_PORT_NUM egressPortNum /* jus to avoid confusion */
);

/**
* @internal appDemoTcam2CncBlockGet function
* @endinternal
*
* @brief   This API gets the CNC counters
*
*
* @note   APPLICABLE DEVICES:      Caelum.
* @note   NOT APPLICABLE DEVICES:  All The rest.
*
* @param[in] devNum       - device number
*
* @retval GT_OK                    - on success
*/
GT_STATUS appDemoTcam2CncBlockGet
(
    IN GT_U8 devNum
);

/**
 * @internal  appDemoTmScenario17PortInit function
 * @endinternal
 *
 * @brief Initialize Traffic Manager for 17 ports scenario
 *
 * @note  APPLICALBE_DEVICES - Caelum
 *
 * @param[in] devNum - device number
 *
 * @retval GT_OK   -  on success
 */
GT_STATUS appDemoTmScenario17PortInitTmAndPcl
(
    IN GT_U8   devNum,
    IN GT_BOOL onlyTmInit
);

/**
 * @internal  appDemoTmScenario17PortSchedulerDelete function
 * @endinternal
 *
 * @brief Delete Scheduler nodes for 17 ports scenario
 *
 * @note  APPLICALBE_DEVICES - Caelum
 *
 * @param[in] devNum - device number
 *
 * @retval GT_OK   -  on success
 */
GT_STATUS appDemoTmScenario17PortSchedulerDelete
(
    IN GT_U8   devNum
);

/**
 * @internal  appDemoTmScenario17PortVTcamDelete function
 * @endinternal
 *
 * @brief Delete VTCAM Manager for 17 ports scenario
 *
 * @note  APPLICALBE_DEVICES - Caelum
 *
 * @param[in] devNum - device number
 *
 * @retval GT_OK   -  on success
 */
GT_STATUS appDemoTmScenario17PortVTcamDelete
(
    IN GT_U8   devNum
);

/**
 * @internal appDemoTmDbReset function
 * @endinternal
 *
 * @brief Reset appDemo Database
 *
 * @param[in] devNum  - device Number
 *
 * @retval GT_OK    - on success
 */
GT_STATUS appDemoTmDbReset
(
    IN GT_U8 devNum
);
#endif /*  __gtAppDemoTmCsRefConfigh */
