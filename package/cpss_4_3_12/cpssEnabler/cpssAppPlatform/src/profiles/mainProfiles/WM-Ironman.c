/*******************************************************************************
*              (c), Copyright 2018, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/
/**
********************************************************************************
* @file WM-Ironman.c
*
* @brief Ironman CAP main profiles
*
* @version   1
********************************************************************************
*/

#include <profiles/cpssAppPlatformProfile.h>
#include <profiles/cpssAppPlatformProfileDecl.h>

/**** IRONMAN full profile - 1 PP ****/
CPSS_CAP_PROFILE_START( WM_IRONMAN_all )
  CPSS_CAP_ADD_BOARD_PROFILE( &WM_IRONMAN_BOARD_INFO )
  CPSS_CAP_ADD_PP_PROFILE( &WM_IRONMAN_PP_INFO, &WM_IRONMAN_PP_MAP_INFO )
  CPSS_CAP_ADD_RUNTIME_PROFILE( &RUNTIME_WM_IRONMAN_INFO )
  CPSS_CAP_ADD_RUNTIME_PROFILE( &RUNTIME_PORT_MANAGER_INFO_IRONMAN )
  CPSS_CAP_ADD_RUNTIME_PROFILE( &RUNTIME_WM_IRONMAN_GEN_EVENT_INFO )
CPSS_CAP_PROFILE_END( WM_IRONMAN_all, "IRONMAN - External Cpu" )

/**** IRONMAN DB board profile - 1 PP ****/
CPSS_CAP_PROFILE_START( DB_IRONMAN_intCPU_all )
  CPSS_CAP_ADD_BOARD_PROFILE( &WM_IRONMAN_BOARD_INFO )
  CPSS_CAP_ADD_PP_PROFILE( &DB_IRONMAN_PP_INTCPU_INFO, &WM_IRONMAN_PP_MAP_INFO )
  CPSS_CAP_ADD_RUNTIME_PROFILE( &RUNTIME_WM_IRONMAN_INFO )
  CPSS_CAP_ADD_RUNTIME_PROFILE( &RUNTIME_PORT_MANAGER_INFO_IRONMAN )
  CPSS_CAP_ADD_RUNTIME_PROFILE( &RUNTIME_WM_IRONMAN_GEN_EVENT_INFO )
CPSS_CAP_PROFILE_END( DB_IRONMAN_intCPU_all, "IRONMAN DB board - Internal Cpu" )

/**** IRONMAN DB board profile - No Ports ****/
CPSS_CAP_PROFILE_START( DB_IRONMAN_intCPU_all_noPorts )
  CPSS_CAP_ADD_BOARD_PROFILE( &WM_IRONMAN_BOARD_INFO )
  CPSS_CAP_ADD_PP_PROFILE( &DB_IRONMAN_PP_INTCPU_INFO, &WM_IRONMAN_PP_MAP_INFO )
  CPSS_CAP_ADD_RUNTIME_PROFILE( &RUNTIME_WM_IRONMAN_NOPORTS_INFO )
  CPSS_CAP_ADD_RUNTIME_PROFILE( &RUNTIME_PORT_MANAGER_INFO_IRONMAN )
  CPSS_CAP_ADD_RUNTIME_PROFILE( &RUNTIME_WM_IRONMAN_GEN_EVENT_INFO )
CPSS_CAP_PROFILE_END( DB_IRONMAN_intCPU_all_noPorts, "IRONMAN DB board - Internal Cpu - No ports" )

/**** IRONMAN DB board profile - 1 PP ****/
CPSS_CAP_PROFILE_START( DB_IRONMAN_all )
  CPSS_CAP_ADD_BOARD_PROFILE( &WM_IRONMAN_BOARD_INFO )
  CPSS_CAP_ADD_PP_PROFILE( &DB_IRONMAN_PP_EXTCPU_INFO, &WM_IRONMAN_PP_MAP_INFO )
  CPSS_CAP_ADD_RUNTIME_PROFILE( &RUNTIME_WM_IRONMAN_INFO )
  CPSS_CAP_ADD_RUNTIME_PROFILE( &RUNTIME_PORT_MANAGER_INFO_IRONMAN )
  CPSS_CAP_ADD_RUNTIME_PROFILE( &RUNTIME_WM_IRONMAN_GEN_EVENT_INFO )
CPSS_CAP_PROFILE_END( DB_IRONMAN_all, "IRONMAN DB board - External Cpu" )

/**** IRONMAN DB board profile - No Ports ****/
CPSS_CAP_PROFILE_START( DB_IRONMAN_all_noPorts )
  CPSS_CAP_ADD_BOARD_PROFILE( &WM_IRONMAN_BOARD_INFO )
  CPSS_CAP_ADD_PP_PROFILE( &DB_IRONMAN_PP_EXTCPU_INFO, &WM_IRONMAN_PP_MAP_INFO )
  CPSS_CAP_ADD_RUNTIME_PROFILE( &RUNTIME_WM_IRONMAN_NOPORTS_INFO )
  CPSS_CAP_ADD_RUNTIME_PROFILE( &RUNTIME_PORT_MANAGER_INFO_IRONMAN )
  CPSS_CAP_ADD_RUNTIME_PROFILE( &RUNTIME_WM_IRONMAN_GEN_EVENT_INFO )
CPSS_CAP_PROFILE_END( DB_IRONMAN_all_noPorts, "IRONMAN DB board - External Cpu - No ports" )

/**** IRONMAN DB board profile - 1 PP ****/
CPSS_CAP_PROFILE_START( DB_IRONMAN_M_all )
  CPSS_CAP_ADD_BOARD_PROFILE( &WM_IRONMAN_BOARD_INFO )
  CPSS_CAP_ADD_PP_PROFILE( &DB_IRONMAN_M_PP_INTCPU_INFO, &WM_IRONMAN_PP_MAP_INFO )
  CPSS_CAP_ADD_RUNTIME_PROFILE( &RUNTIME_WM_IRONMAN_M_INFO )
  CPSS_CAP_ADD_RUNTIME_PROFILE( &RUNTIME_PORT_MANAGER_INFO_IRONMAN )
  CPSS_CAP_ADD_RUNTIME_PROFILE( &RUNTIME_WM_IRONMAN_GEN_EVENT_INFO )
CPSS_CAP_PROFILE_END( DB_IRONMAN_M_all, "IRONMAN-M DB board - Internal Cpu" )

/**** IRONMAN DB board profile - 1 PP ****/
CPSS_CAP_PROFILE_START( DB_IRONMAN_M_all_noPorts )
  CPSS_CAP_ADD_BOARD_PROFILE( &WM_IRONMAN_BOARD_INFO )
  CPSS_CAP_ADD_PP_PROFILE( &DB_IRONMAN_M_PP_INTCPU_INFO, &WM_IRONMAN_PP_MAP_INFO )
  CPSS_CAP_ADD_RUNTIME_PROFILE( &RUNTIME_WM_IRONMAN_M_INFO_NOPORTS )
  CPSS_CAP_ADD_RUNTIME_PROFILE( &RUNTIME_PORT_MANAGER_INFO_IRONMAN )
  CPSS_CAP_ADD_RUNTIME_PROFILE( &RUNTIME_WM_IRONMAN_GEN_EVENT_INFO )
CPSS_CAP_PROFILE_END( DB_IRONMAN_M_all_noPorts, "IRONMAN-M DB board - Internal Cpu - No ports" )

/**** IRONMAN DB board profile - 1 PP ****/
CPSS_CAP_PROFILE_START( DB_IRONMAN_L_intCPU_all_phy )
  CPSS_CAP_ADD_BOARD_PROFILE( &WM_IRONMAN_BOARD_INFO )
  CPSS_CAP_ADD_PP_PROFILE( &DB_IRONMAN_L_PP_INTCPU_INFO_PHY, &WM_IRONMAN_PP_MAP_INFO )
  CPSS_CAP_ADD_RUNTIME_PROFILE( &RUNTIME_WM_IRONMAN_INFO )
  CPSS_CAP_ADD_RUNTIME_PROFILE( &RUNTIME_PORT_MANAGER_INFO_IRONMAN )
  CPSS_CAP_ADD_RUNTIME_PROFILE( &RUNTIME_WM_IRONMAN_GEN_EVENT_INFO )
CPSS_CAP_PROFILE_END( DB_IRONMAN_L_intCPU_all_phy, "IRONMAN DB board - Phy - Internal Cpu" )

/**** IRONMAN DB board profile - No Ports ****/
CPSS_CAP_PROFILE_START( DB_IRONMAN_L_intCPU_all_noPorts_phy )
  CPSS_CAP_ADD_BOARD_PROFILE( &WM_IRONMAN_BOARD_INFO )
  CPSS_CAP_ADD_PP_PROFILE( &DB_IRONMAN_L_PP_INTCPU_INFO_PHY, &WM_IRONMAN_PP_MAP_INFO )
  CPSS_CAP_ADD_RUNTIME_PROFILE( &RUNTIME_WM_IRONMAN_NOPORTS_INFO )
  CPSS_CAP_ADD_RUNTIME_PROFILE( &RUNTIME_PORT_MANAGER_INFO_IRONMAN )
  CPSS_CAP_ADD_RUNTIME_PROFILE( &RUNTIME_WM_IRONMAN_GEN_EVENT_INFO )
CPSS_CAP_PROFILE_END( DB_IRONMAN_L_intCPU_all_noPorts_phy, "IRONMAN DB board - Phy - Internal Cpu - No ports" )

/**** IRONMAN DB board profile - 1 PP ****/
CPSS_CAP_PROFILE_START( DB_IRONMAN_L_all_phy )
  CPSS_CAP_ADD_BOARD_PROFILE( &WM_IRONMAN_BOARD_INFO )
  CPSS_CAP_ADD_PP_PROFILE( &DB_IRONMAN_L_PP_EXTCPU_INFO_PHY, &WM_IRONMAN_PP_MAP_INFO )
  CPSS_CAP_ADD_RUNTIME_PROFILE( &RUNTIME_WM_IRONMAN_INFO )
  CPSS_CAP_ADD_RUNTIME_PROFILE( &RUNTIME_PORT_MANAGER_INFO_IRONMAN )
  CPSS_CAP_ADD_RUNTIME_PROFILE( &RUNTIME_WM_IRONMAN_GEN_EVENT_INFO )
CPSS_CAP_PROFILE_END( DB_IRONMAN_L_all_phy, "IRONMAN DB board - Phy - External Cpu" )

/**** IRONMAN DB board profile - No Ports ****/
CPSS_CAP_PROFILE_START( DB_IRONMAN_L_all_noPorts_phy )
  CPSS_CAP_ADD_BOARD_PROFILE( &WM_IRONMAN_BOARD_INFO )
  CPSS_CAP_ADD_PP_PROFILE( &DB_IRONMAN_L_PP_EXTCPU_INFO_PHY, &WM_IRONMAN_PP_MAP_INFO )
  CPSS_CAP_ADD_RUNTIME_PROFILE( &RUNTIME_WM_IRONMAN_NOPORTS_INFO )
  CPSS_CAP_ADD_RUNTIME_PROFILE( &RUNTIME_PORT_MANAGER_INFO_IRONMAN )
  CPSS_CAP_ADD_RUNTIME_PROFILE( &RUNTIME_WM_IRONMAN_GEN_EVENT_INFO )
CPSS_CAP_PROFILE_END( DB_IRONMAN_L_all_noPorts_phy, "IRONMAN DB board - Phy - External Cpu - No ports" )


