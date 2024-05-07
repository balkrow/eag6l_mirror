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
* @file gtDbDxPhoenixWithPipeExtended.h
*
* @brief Extended info for the AC5X + PIPE board.
*
* @version   1
********************************************************************************
*/
#ifndef __gtDbDxPhoenixWithPipeExtended_H
#define __gtDbDxPhoenixWithPipeExtended_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <appDemo/boardConfig/gtDbDxBobcat2Mappings.h>
#include <appDemo/boardConfig/gtDbDxPhoenixWithCaelumExtended.h>

/* next 3 filled by : appDemoAc5xWithPipe_fillAc5xWithPortMappingAndSpeed() */
extern AC5X_PORT_MAP_STC ac5x_with_rpp_for_pipe_port_mode[];
extern GT_U32  actualNum_ac5x_with_rpp_for_pipe_port_mode;
extern PortInitList_STC ac5x_with_rpp_for_pipe_portInitlist[];

/*******************************************************************************
* appDemoAc5xWithPipe_fillAc5xWithPortMappingAndSpeed
*
* DESCRIPTION:
*       The function is called after we recognize that PIPE is in the system with the AC5X.
*       The function sets arrays needed for : port mapping and speeds
*       The function fill port mapping and speed in :
*       extern AC5X_PORT_MAP_STC ac5x_with_rpp_for_pipe_port_mode[];
*       extern GT_U32  actualNum_ac5x_with_rpp_for_pipe_port_mode;
*       extern PortInitList_STC ac5x_with_rpp_for_pipe_portInitlist[];
*       static APP_DEMO_PORT_MAP_VER1_STC appDemoInfo_ac5xCascadePorts[];
*       static GT_U32  ac5x_cascadePorts[];
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success,
*       GT_FAIL - otherwise.
*
* COMMENTS:
*
*
*******************************************************************************/
GT_STATUS   appDemoAc5xWithPipe_fillAc5xWithPortMappingAndSpeed(void);

/*******************************************************************************
* appDemoAc5xWithPipe_initAfterBasicAc5xInit
*
* DESCRIPTION:
*       The function is called after the AC5X did the basic
*       (including port mapping and speeds that was done during appDemoAc5xWithPipe_fillAc5xWithPortMappingAndSpeed(...))
*       The function sets the AC5X with cascade config and remote physical config.
*
* INPUTS:
*       ac5x_devIndex  - device index for the AC5X (index in appDemoPpConfigList[] DB)
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success,
*       GT_FAIL - otherwise.
*
* COMMENTS:
*
*
*******************************************************************************/
GT_STATUS   appDemoAc5xWithPipe_initAfterBasicAc5xInit(
    IN GT_U32  ac5x_devIndex
);

/*******************************************************************************
* appDemoAc5xWithPipe_fillAc5xWithPortMappingAndSpeed
*
* DESCRIPTION:
*       The function is called after we recognize that PIPE is in the system with the AC5X.
*       The function sets arrays needed for : port mapping and speeds
*       The function fill port mapping and speed in :
*       extern AC5X_PORT_MAP_STC ac5x_with_rpp_for_pipe_port_mode[];
*       extern GT_U32  actualNum_ac5x_with_rpp_for_pipe_port_mode;
*       extern PortInitList_STC ac5x_with_rpp_for_pipe_portInitlist[];
*       static APP_DEMO_PORT_MAP_VER1_STC appDemoInfo_ac5xCascadePorts[];
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success,
*       GT_FAIL - otherwise.
*
* COMMENTS:
*
*
*******************************************************************************/
GT_STATUS   appDemoAc5xWithPipe_fillAc5xWithPortMappingAndSpeed(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /*__gtDbDxPhoenixWithPipeExtended_H*/


