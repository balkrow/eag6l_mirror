/*******************************************************************************
*              (c), Copyright 2023, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/
/**
********************************************************************************
* @file WM-AC5P.c
*
* @brief AC5P CAP main profiles
*
* @version   1
********************************************************************************
*/

#include <profiles/cpssAppPlatformProfile.h>
#include <profiles/cpssAppPlatformProfileDecl.h>

CPSS_CAP_PROFILE_START( WM_AC5P_all )
  CPSS_CAP_ADD_BOARD_PROFILE( &WM_AC5P_BOARD_INFO )
  CPSS_CAP_ADD_PP_PROFILE( &WM_AC5P_PP_INFO, &WM_AC5P_PP_MAP_INFO )
  CPSS_CAP_ADD_RUNTIME_PROFILE( &RUNTIME_WM_AC5P_INFO )
  CPSS_CAP_ADD_RUNTIME_PROFILE( &RUNTIME_PORT_MANAGER_INFO_AC5P )
  CPSS_CAP_ADD_RUNTIME_PROFILE( &RUNTIME_WM_AC5P_GEN_EVENT_INFO )
CPSS_CAP_PROFILE_END( WM_AC5P_all, "AC5P - External Cpu" )

CPSS_CAP_PROFILE_START( WM_AC5P_all_noports )
  CPSS_CAP_ADD_BOARD_PROFILE( &WM_AC5P_BOARD_INFO )
  CPSS_CAP_ADD_PP_PROFILE( &WM_AC5P_PP_INFO, &WM_AC5P_PP_MAP_INFO )
  CPSS_CAP_ADD_RUNTIME_PROFILE( &RUNTIME_WM_NOPORTS_AC5P_INFO )
  CPSS_CAP_ADD_RUNTIME_PROFILE( &RUNTIME_PORT_MANAGER_INFO_AC5P )
  CPSS_CAP_ADD_RUNTIME_PROFILE( &RUNTIME_WM_AC5P_GEN_EVENT_INFO )
CPSS_CAP_PROFILE_END( WM_AC5P_all_noports, "AC5P - External Cpu with no ports" )

CPSS_CAP_PROFILE_START( DB_AC5P_8SFP_all )
  CPSS_CAP_ADD_BOARD_PROFILE( &DB_AC5P_8SFP_BOARD_INFO )
  CPSS_CAP_ADD_PP_PROFILE( &WM_AC5P_PP_INFO, &WM_AC5P_PP_MAP_INFO )
  CPSS_CAP_ADD_RUNTIME_PROFILE( &RUNTIME_DB_AC5P_8SFP_INFO )
  CPSS_CAP_ADD_RUNTIME_PROFILE( &RUNTIME_PORT_MANAGER_INFO_AC5P )
  CPSS_CAP_ADD_RUNTIME_PROFILE( &RUNTIME_WM_AC5P_GEN_EVENT_INFO )
CPSS_CAP_PROFILE_END( DB_AC5P_8SFP_all, "AC5P DB - External Cpu" )

CPSS_CAP_PROFILE_START( DB_AC5P_8SFP_all_noports )
  CPSS_CAP_ADD_BOARD_PROFILE( &DB_AC5P_8SFP_BOARD_INFO )
  CPSS_CAP_ADD_PP_PROFILE( &WM_AC5P_PP_INFO, &WM_AC5P_PP_MAP_INFO )
  CPSS_CAP_ADD_RUNTIME_PROFILE( &RUNTIME_DB_AC5P_8SFP_NOPORTS_INFO )
  CPSS_CAP_ADD_RUNTIME_PROFILE( &RUNTIME_PORT_MANAGER_INFO_AC5P )
  CPSS_CAP_ADD_RUNTIME_PROFILE( &RUNTIME_WM_AC5P_GEN_EVENT_INFO )
CPSS_CAP_PROFILE_END( DB_AC5P_8SFP_all_noports, "AC5P DB - External Cpu  with no ports" )

CPSS_CAP_PROFILE_START( RD_AC5P_32SFP_all )
  CPSS_CAP_ADD_BOARD_PROFILE( &RD_AC5P_32SFP_BOARD_INFO )
  CPSS_CAP_ADD_PP_PROFILE( &WM_AC5P_RD_PP_INFO, &WM_AC5P_PP_MAP_INFO )
  CPSS_CAP_ADD_RUNTIME_PROFILE( &RUNTIME_RD_AC5P_32SFP_INFO )
  CPSS_CAP_ADD_RUNTIME_PROFILE( &RUNTIME_PORT_MANAGER_INFO_AC5P )
  CPSS_CAP_ADD_RUNTIME_PROFILE( &RUNTIME_WM_AC5P_GEN_EVENT_INFO )
CPSS_CAP_PROFILE_END( RD_AC5P_32SFP_all, "AC5P RD - External Cpu" )

CPSS_CAP_PROFILE_START( RD_AC5P_32SFP_all_noports )
  CPSS_CAP_ADD_BOARD_PROFILE( &RD_AC5P_32SFP_BOARD_INFO )
  CPSS_CAP_ADD_PP_PROFILE( &WM_AC5P_RD_PP_INFO, &WM_AC5P_PP_MAP_INFO )
  CPSS_CAP_ADD_RUNTIME_PROFILE( &RUNTIME_RD_AC5P_32SFP_NOPORTS_INFO )
  CPSS_CAP_ADD_RUNTIME_PROFILE( &RUNTIME_PORT_MANAGER_INFO_AC5P )
  CPSS_CAP_ADD_RUNTIME_PROFILE( &RUNTIME_WM_AC5P_GEN_EVENT_INFO )
CPSS_CAP_PROFILE_END( RD_AC5P_32SFP_all_noports, "AC5P RD - External Cpu with no ports" )

CPSS_CAP_PROFILE_START( WM_AC5P_int_cpu_all )
  CPSS_CAP_ADD_BOARD_PROFILE( &WM_AC5P_BOARD_INFO )
  CPSS_CAP_ADD_PP_PROFILE( &WM_AC5P_INT_CPU_PP_INFO, &WM_AC5P_PP_MAP_INFO )
  CPSS_CAP_ADD_RUNTIME_PROFILE( &RUNTIME_WM_AC5P_INFO )
  CPSS_CAP_ADD_RUNTIME_PROFILE( &RUNTIME_PORT_MANAGER_INFO_AC5P )
  CPSS_CAP_ADD_RUNTIME_PROFILE( &RUNTIME_WM_AC5P_GEN_EVENT_INFO )
CPSS_CAP_PROFILE_END( WM_AC5P_int_cpu_all, "AC5P - Internal Cpu" )

/* AC5P RT Profile */
CPSS_CAP_PROFILE_START( WM_AC5P_RT)
  CPSS_CAP_ADD_RUNTIME_PROFILE( &RUNTIME_WM_AC5P_INFO )
  CPSS_CAP_ADD_RUNTIME_PROFILE( &RUNTIME_PORT_MANAGER_INFO_AC5P )
  CPSS_CAP_ADD_RUNTIME_PROFILE( &RUNTIME_WM_AC5P_GEN_EVENT_INFO )
CPSS_CAP_PROFILE_END( WM_AC5P_RT, "AC5P WM - runtime profile" )

/* AC5P Link Profile */
CPSS_CAP_PROFILE_START( WM_AC5P_Link)
  CPSS_CAP_ADD_RUNTIME_PROFILE( &LINK_WM_AC5P_INFO )
CPSS_CAP_PROFILE_END( WM_AC5P_Link, "AC5P WM - Link profile" )

/* AC5P Rx Profile */
CPSS_CAP_PROFILE_START( WM_AC5P_Rx)
  CPSS_CAP_ADD_RUNTIME_PROFILE( &RX_WM_AC5P_INFO )
CPSS_CAP_PROFILE_END( WM_AC5P_Rx, "AC5P WM - Rx profile" )

/* AC5P - board profile */
CPSS_CAP_PROFILE_START ( WM_AC5P_board_only )
   CPSS_CAP_ADD_BOARD_PROFILE ( &WM_AC5P_BOARD_INFO )
CPSS_CAP_PROFILE_END ( WM_AC5P_board_only, "AC5P - Board only" )

/* AC5P with one PP */
CPSS_CAP_PROFILE_START (WM_AC5P_PP_only)
  CPSS_CAP_ADD_PP_PROFILE (&WM_AC5P_PP_INFO, &WM_AC5P_PP_MAP_INFO)
CPSS_CAP_PROFILE_END (WM_AC5P_PP_only, "AC5P - PP only")


