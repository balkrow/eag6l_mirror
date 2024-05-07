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
* @file gtDbDxPhoenixWithCaelumExtended.h
*
* @brief Extended info for the AC5X + Caelum board.
*
* @version   1
********************************************************************************
*/
#ifndef __gtDbDxPhoenixWithCaelumExtended_H
#define __gtDbDxPhoenixWithCaelumExtended_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define ARR_PTR_AND_SIZE_MAC(myArr , arrPtr , arrLen) \
    arrPtr = &myArr[0];                               \
    arrLen = sizeof(myArr)/sizeof(myArr[0])

#define CPU_PORT    CPSS_CPU_PORT_NUM_CNS

#define AC5X_PORT_NUMBER_ARR_SIZE    9
typedef struct
{
    GT_PHYSICAL_PORT_NUM                portNumberArr  [AC5X_PORT_NUMBER_ARR_SIZE];
    GT_U32                              globalDmaNumber[AC5X_PORT_NUMBER_ARR_SIZE];
    CPSS_DXCH_PORT_MAPPING_TYPE_ENT     mappingType;
}AC5X_PORT_MAP_STC;

/* next 3 filled by : appDemoAc5xWithCaelum_fillAc5xWithPortMappingAndSpeed() */
extern AC5X_PORT_MAP_STC ac5x_with_rpp_for_caelum_port_mode[];
extern GT_U32  actualNum_ac5x_with_rpp_for_caelum_port_mode;
extern PortInitList_STC ac5x_with_rpp_for_caelum_portInitlist[];

/*next 4 filled by appDemoAc5xWithCaelum_initAfterBasicAc5xInit */
extern CPSS_DXCH_PORT_MAP_STC  *appDemoPortMapPtr;
/* number of elements in appDemoPortMapPtr */
extern GT_U32                  appDemoPortMapSize;

extern boardRevId2PortsInitList_STC *appDemo_boardRevId2PortsInitListPtr;
/* number of elements in appDemo_boardRevId2PortsInitListPtr */
extern GT_U32                  appDemo_boardRevId2PortsSize;

extern GT_STATUS userForceBoardType(IN GT_U32 boardType);
extern GT_STATUS appDemoDxTrafficEnable(GT_U8 dev);
extern void DRV_EMULATE_INTERRUPTS_pollingSleepTime_Set(GT_U32 new_milisec_pollingSleepTime);
extern GT_STATUS appDemoTrunkCrcHashDefaultsSet(IN GT_U8 devNum);
/*******************************************************************************
* appDemoAc5xWithCaelum_fillAc5xWithPortMappingAndSpeed
*
* DESCRIPTION:
*       The function is called after we recognize that Caelum is in the system with the AC5X.
*       The function sets arrays needed for : port mapping and speeds
*       The function fill port mapping and speed in :
*       extern AC5X_PORT_MAP_STC ac5x_with_rpp_for_caelum_port_mode[];
*       extern GT_U32  actualNum_ac5x_with_rpp_for_caelum_port_mode;
*       extern PortInitList_STC ac5x_with_rpp_for_caelum_portInitlist[];
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
GT_STATUS   appDemoAc5xWithCaelum_fillAc5xWithPortMappingAndSpeed(void);

/*******************************************************************************
* appDemoAc5xWithCaelum_initAfterBasicAc5xInit
*
* DESCRIPTION:
*       The function is called after the AC5X did the basic
*       (including port mapping and speeds that was done during appDemoAc5xWithCaelum_fillAc5xWithPortMappingAndSpeed(...))
*       The function :
*       1. sets the AC5X with cascade config and remote physical config .
*       2. init the Caelum device using TM (calling cpssInitSystem 29,2,0)
*           with port mapping and speeds of this system
*       3. sets the Caelum with cascade config and PVE to those cascade.
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
GT_STATUS   appDemoAc5xWithCaelum_initAfterBasicAc5xInit(
    IN GT_U32  ac5x_devIndex
);

/*******************************************************************************
* appDemoAc5xWithCaelum_HaPostCompletion
*
* DESCRIPTION:
*       trigers appDemoAc5xWithCaelum_SystemReady at the end of HA process , needs to be called manually
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
*******************************************************************************/
GT_STATUS appDemoAc5xWithCaelum_HaPostCompletion(
    GT_VOID
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /*__gtDbDxPhoenixWithCaelumExtended_H*/


