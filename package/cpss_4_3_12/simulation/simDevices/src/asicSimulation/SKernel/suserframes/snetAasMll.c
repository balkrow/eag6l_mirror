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
* @file snetAasMll.c
*
* @brief This is a external API definition for MLL unit
*
* @version   1
********************************************************************************
*/
#include <asicSimulation/SKernel/smain/smain.h>
#include <asicSimulation/SKernel/smem/smemCheetah.h>
#include <asicSimulation/SKernel/smem/smemAas.h>
#include <asicSimulation/SKernel/suserframes/snet.h>
#include <asicSimulation/SKernel/skernel.h>
#include <asicSimulation/SKernel/cheetahCommon/sregCheetah.h>
#include <asicSimulation/SKernel/cheetahCommon/sregBobcat2.h>
#include <asicSimulation/SKernel/cheetahCommon/sregFalcon.h>
#include <asicSimulation/SKernel/suserframes/snetCheetahEgress.h>
#include <asicSimulation/SKernel/suserframes/snetCheetah2TStart.h>
#include <asicSimulation/SKernel/suserframes/snetCheetah3.h>
#include <asicSimulation/SKernel/suserframes/snetXCat.h>
#include <asicSimulation/SKernel/suserframes/snetCheetah3CentralizedCnt.h>
#include <asicSimulation/SKernel/suserframes/snetXCatPolicer.h>
#include <asicSimulation/SKernel/suserframes/snetCheetahEq.h>
#include <asicSimulation/SKernel/suserframes/snetLion.h>
#include <asicSimulation/SKernel/suserframes/snetAasHa.h>
#include <asicSimulation/SKernel/suserframes/snetAasMll.h>
#include <asicSimulation/SLog/simLog.h>
#include <asicSimulation/SLog/simLogInfoTypePacket.h>

#define SMEM_SIP7_IP_MLL_BIER_TABLE_FIELDS_NAME                                    \
     STR(SMEM_SIP7_IP_MLL_BIER_TABLE_FIELDS_LAST_0                                  )\
    ,STR(SMEM_SIP7_IP_MLL_BIER_TABLE_FIELDS_START_OF_TUNNEL_0                       )\
    ,STR(SMEM_SIP7_IP_MLL_BIER_TABLE_FIELDS_MLL_RPF_FAIL_CMD_0                      )\
    ,STR(SMEM_SIP7_IP_MLL_BIER_TABLE_FIELDS_MLL_EVID_0                              )\
    ,STR(SMEM_SIP7_IP_MLL_BIER_TABLE_FIELDS_USE_VIDX_0                              )\
    ,STR(SMEM_SIP7_IP_MLL_BIER_TABLE_FIELDS_TARGET_IS_TRUNK_0                       )\
    ,STR(SMEM_SIP7_IP_MLL_BIER_TABLE_FIELDS_USE_VPORT_0                              )\
    ,STR(SMEM_SIP7_IP_MLL_BIER_TABLE_FIELDS_EVIDX_0                                 )\
    ,STR(SMEM_SIP7_IP_MLL_BIER_TABLE_FIELDS_TRG_EPORT_0                             )\
    ,STR(SMEM_SIP7_IP_MLL_BIER_TABLE_FIELDS_TRG_TRUNK_ID_0                          )\
    ,STR(SMEM_SIP7_IP_MLL_BIER_TABLE_FIELDS_VPORT_0                                  )\
    ,STR(SMEM_SIP7_IP_MLL_BIER_TABLE_FIELDS_DISABLE_MAC_SA_MODIFICATION_0            )\
    ,STR(SMEM_SIP7_IP_MLL_BIER_TABLE_FIELDS_RESERVED_0                              )\
    ,STR(SMEM_SIP7_IP_MLL_BIER_TABLE_FIELDS_TRG_DEV_0                               )\
    ,STR(SMEM_SIP7_IP_MLL_BIER_TABLE_FIELDS_EXCLUDE_SRC_VLAN_0                      )\
    ,STR(SMEM_SIP7_IP_MLL_BIER_TABLE_FIELDS_TTL_THRESHOLD_0_OR_HOP_LIMIT_THRESHOLD_0)\
    ,STR(SMEM_SIP7_IP_MLL_BIER_TABLE_FIELDS_TUNNEL_PTR_0                            )\
    ,STR(SMEM_SIP7_IP_MLL_BIER_TABLE_FIELDS_TUNNEL_TYPE_0                           )\
    ,STR(SMEM_SIP7_IP_MLL_BIER_TABLE_FIELDS_LAST_1                                  )\
    ,STR(SMEM_SIP7_IP_MLL_BIER_TABLE_FIELDS_START_OF_TUNNEL_1                       )\
    ,STR(SMEM_SIP7_IP_MLL_BIER_TABLE_FIELDS_MLL_RPF_FAIL_CMD_1                      )\
    ,STR(SMEM_SIP7_IP_MLL_BIER_TABLE_FIELDS_MLL_EVID_1                              )\
    ,STR(SMEM_SIP7_IP_MLL_BIER_TABLE_FIELDS_USE_VIDX_1                              )\
    ,STR(SMEM_SIP7_IP_MLL_BIER_TABLE_FIELDS_TARGET_IS_TRUNK_1                       )\
    ,STR(SMEM_SIP7_IP_MLL_BIER_TABLE_FIELDS_USE_VPORT_1                              )\
    ,STR(SMEM_SIP7_IP_MLL_BIER_TABLE_FIELDS_EVIDX_1                                 )\
    ,STR(SMEM_SIP7_IP_MLL_BIER_TABLE_FIELDS_TRG_EPORT_1                             )\
    ,STR(SMEM_SIP7_IP_MLL_BIER_TABLE_FIELDS_TRG_TRUNK_ID_1                          )\
    ,STR(SMEM_SIP7_IP_MLL_BIER_TABLE_FIELDS_VPORT_1                                  )\
    ,STR(SMEM_SIP7_IP_MLL_BIER_TABLE_FIELDS_DISABLE_MAC_SA_MODIFICATION_1            )\
    ,STR(SMEM_SIP7_IP_MLL_BIER_TABLE_FIELDS_RESERVED_1                              )\
    ,STR(SMEM_SIP7_IP_MLL_BIER_TABLE_FIELDS_TRG_DEV_1                               )\
    ,STR(SMEM_SIP7_IP_MLL_BIER_TABLE_FIELDS_EXCLUDE_SRC_VLAN_1                      )\
    ,STR(SMEM_SIP7_IP_MLL_BIER_TABLE_FIELDS_TTL_THRESHOLD_1_OR_HOP_LIMIT_THRESHOLD_1)\
    ,STR(SMEM_SIP7_IP_MLL_BIER_TABLE_FIELDS_TUNNEL_PTR_1                            )\
    ,STR(SMEM_SIP7_IP_MLL_BIER_TABLE_FIELDS_TUNNEL_TYPE_1                           )\
    ,STR(SMEM_SIP7_IP_MLL_BIER_TABLE_FIELDS_NEXT_MLL_PTR                            )\
    ,STR(SMEM_SIP7_IP_MLL_BIER_TABLE_FIELDS_RESERVED_2                              )

static SNET_ENTRY_FORMAT_TABLE_STC sip7IpMllBierTableFieldsFormat[
    SMEM_SIP7_IP_MLL_BIER_TABLE_FIELDS___LAST_VALUE___E] =
{
/*SMEM_SIP7_IP_MLL_BIER_TABLE_FIELDS_LAST_0                                      */
    STANDARD_FIELD_MAC(1),
/*SMEM_SIP7_IP_MLL_BIER_TABLE_FIELDS_START_OF_TUNNEL_0                           */
    STANDARD_FIELD_MAC(1),
/*SMEM_SIP7_IP_MLL_BIER_TABLE_FIELDS_MLL_RPF_FAIL_CMD_0                          */
    STANDARD_FIELD_MAC(3),
/*SMEM_SIP7_IP_MLL_BIER_TABLE_FIELDS_MLL_EVID_0                                  */
    STANDARD_FIELD_MAC(16),/*was 13*/
/*SMEM_SIP7_IP_MLL_BIER_TABLE_FIELDS_USE_VIDX_0                                  */
    STANDARD_FIELD_MAC(1),
/*SMEM_SIP7_IP_MLL_BIER_TABLE_FIELDS_TARGET_IS_TRUNK_0                           */
    STANDARD_FIELD_MAC(1),
/*SMEM_SIP7_IP_MLL_BIER_TABLE_FIELDS_USE_VPORT_0*/
    STANDARD_FIELD_MAC(1),

    /*UseVIDX0 = "True"*/
    /*SMEM_SIP7_IP_MLL_BIER_TABLE_FIELDS_EVIDX_0                                     */
        {FIELD_SET_IN_RUNTIME_CNS,
         17,
         SMEM_SIP7_IP_MLL_TABLE_FIELDS_USE_VPORT_0},

    /*UseVIDX0 = "False") and (TargetIsTrunk0 = "False")*/
    /*SMEM_SIP7_IP_MLL_BIER_TABLE_FIELDS_TRG_EPORT_0                                 */
        {FIELD_SET_IN_RUNTIME_CNS,
         15,/*was 14 */
         SMEM_SIP7_IP_MLL_TABLE_FIELDS_USE_VPORT_0},

    /*(UseVIDX0 = "False") and (TargetIsTrunk0 = "True")*/
    /*SMEM_SIP7_IP_MLL_BIER_TABLE_FIELDS_TRG_TRUNK_ID_0                              */
        {FIELD_SET_IN_RUNTIME_CNS,
         12,
         SMEM_SIP7_IP_MLL_TABLE_FIELDS_USE_VPORT_0},

    /*SMEM_SIP7_IP_MLL_BIER_TABLE_FIELDS_VPORT_0*/
        {FIELD_SET_IN_RUNTIME_CNS,
         18,
         SMEM_SIP7_IP_MLL_TABLE_FIELDS_USE_VPORT_0},

    /*SMEM_SIP7_IP_MLL_BIER_TABLE_FIELDS_DISABLE_MAC_SA_MODIFICATION_0*/
    STANDARD_FIELD_MAC(1),

    /*SMEM_SIP7_IP_MLL_BIER_TABLE_FIELDS_RESERVED_0*/
    STANDARD_FIELD_MAC(2),

/*(UseVIDX0 = "False") and (TargetIsTrunk0 = "False")*/
/*SMEM_SIP7_IP_MLL_BIER_TABLE_FIELDS_TRG_DEV_0                                   */
    STANDARD_FIELD_MAC(10),

/*SMEM_SIP7_IP_MLL_BIER_TABLE_FIELDS_EXCLUDE_SRC_VLAN_0                          */
    EXPLICIT_FIELD_MAC(63,1),
/*SMEM_SIP7_IP_MLL_BIER_TABLE_FIELDS_TTL_THRESHOLD_0_OR_HOP_LIMIT_THRESHOLD_0    */
    EXPLICIT_FIELD_MAC(55,8),
/*SMEM_SIP7_IP_MLL_BIER_TABLE_FIELDS_TUNNEL_PTR_0                                */
    {FIELD_SET_IN_RUNTIME_CNS,
     19,
     SMEM_SIP7_IP_MLL_BIER_TABLE_FIELDS_EXCLUDE_SRC_VLAN_0},
/*SMEM_SIP7_IP_MLL_BIER_TABLE_FIELDS_TUNNEL_TYPE_0                               */
    STANDARD_FIELD_MAC(1),

/*SMEM_SIP7_IP_MLL_BIER_TABLE_FIELDS_LAST_1                                      */
    STANDARD_FIELD_MAC(1),
/*SMEM_SIP7_IP_MLL_BIER_TABLE_FIELDS_START_OF_TUNNEL_1                           */
    STANDARD_FIELD_MAC(1),
/*SMEM_SIP7_IP_MLL_BIER_TABLE_FIELDS_MLL_RPF_FAIL_CMD_1                          */
    STANDARD_FIELD_MAC(3),
/*SMEM_SIP7_IP_MLL_BIER_TABLE_FIELDS_MLL_EVID_1                                  */
    STANDARD_FIELD_MAC(16),/*was 13*/
/*SMEM_SIP7_IP_MLL_BIER_TABLE_FIELDS_USE_VIDX_1                                  */
    STANDARD_FIELD_MAC(1),
/*SMEM_SIP7_IP_MLL_BIER_TABLE_FIELDS_TARGET_IS_TRUNK_1                           */
    STANDARD_FIELD_MAC(1),
/*SMEM_SIP7_IP_MLL_TABLE_FIELDS_USE_VPORT_1*/
    STANDARD_FIELD_MAC(1),/* not exist */

    /*UseVIDX0 = "True"*/
    /*SMEM_SIP7_IP_MLL_BIER_TABLE_FIELDS_EVIDX_1                                     */
        {FIELD_SET_IN_RUNTIME_CNS,
         17,/*was 16*/
         SMEM_SIP7_IP_MLL_TABLE_FIELDS_USE_VPORT_1},

    /*UseVIDX0 = "False") and (TargetIsTrunk0 = "False")*/
    /*SMEM_SIP7_IP_MLL_BIER_TABLE_FIELDS_TRG_EPORT_1                                 */
        {FIELD_SET_IN_RUNTIME_CNS,
         15,/*was 14 */
         SMEM_SIP7_IP_MLL_TABLE_FIELDS_USE_VPORT_1},

    /*(UseVIDX0 = "False") and (TargetIsTrunk0 = "True")*/
    /*SMEM_SIP7_IP_MLL_BIER_TABLE_FIELDS_TRG_TRUNK_ID_1                              */
        {FIELD_SET_IN_RUNTIME_CNS,
         12,
         SMEM_SIP7_IP_MLL_TABLE_FIELDS_USE_VPORT_1},

/*SMEM_SIP7_IP_MLL_BIER_TABLE_FIELDS_VPORT_1*/
        {FIELD_SET_IN_RUNTIME_CNS,
         18,
         SMEM_SIP7_IP_MLL_TABLE_FIELDS_USE_VPORT_1},
/*SMEM_SIP7_IP_MLL_BIER_TABLE_FIELDS_DISABLE_MAC_SA_MODIFICATION_1*/
    STANDARD_FIELD_MAC(1),

    /*SMEM_SIP7_IP_MLL_BIER_TABLE_FIELDS_RESERVED_1*/
    STANDARD_FIELD_MAC(2),

/*(UseVIDX0 = "False") and (TargetIsTrunk0 = "False")*/
/*SMEM_SIP7_IP_MLL_BIER_TABLE_FIELDS_TRG_DEV_1                                   */
    STANDARD_FIELD_MAC(10),

/*SMEM_SIP7_IP_MLL_BIER_TABLE_FIELDS_EXCLUDE_SRC_VLAN_1                          */
    EXPLICIT_FIELD_MAC(147,1),
/*SMEM_SIP7_IP_MLL_BIER_TABLE_FIELDS_TTL_THRESHOLD_1_OR_HOP_LIMIT_THRESHOLD_1    */
    EXPLICIT_FIELD_MAC(139,8),
/*SMEM_SIP7_IP_MLL_BIER_TABLE_FIELDS_TUNNEL_PTR_1                                */
    {FIELD_SET_IN_RUNTIME_CNS,
     19,/*was 16*/
     SMEM_SIP7_IP_MLL_BIER_TABLE_FIELDS_EXCLUDE_SRC_VLAN_1},
/*SMEM_SIP7_IP_MLL_BIER_TABLE_FIELDS_TUNNEL_TYPE_1                               */
    STANDARD_FIELD_MAC(1),
/*SMEM_SIP7_IP_MLL_BIER_TABLE_FIELDS_NEXT_MLL_PTR                                */
    STANDARD_FIELD_MAC(17),/*was 16*/
/*SMEM_SIP7_IP_MLL_BIER_TABLE_FIELDS_RESERVED_2*/
    STANDARD_FIELD_MAC(555)
};

static char * sip7IpMllBierFieldsTableNames[
    SMEM_SIP7_IP_MLL_BIER_TABLE_FIELDS___LAST_VALUE___E] =
    {SMEM_SIP7_IP_MLL_BIER_TABLE_FIELDS_NAME};


#define SMEM_SIP7_L2_MLL_BIER_TABLE_FIELDS_NAME                                    \
     STR(SMEM_SIP7_L2_MLL_BIER_TABLE_FIELDS_LAST_0                                 )\
    ,STR(SMEM_SIP7_L2_MLL_BIER_TABLE_FIELDS_BIND_TO_MLL_COUNTER_0                  )\
    ,STR(SMEM_SIP7_L2_MLL_BIER_TABLE_FIELDS_MASK_BITMAP_0                          )\
    ,STR(SMEM_SIP7_L2_MLL_BIER_TABLE_FIELDS_E_TREE_LEAF_0                          )\
    ,STR(SMEM_SIP7_L2_MLL_BIER_TABLE_FIELDS_BIER_MASK_PACKET_BIT_STRING_0          )\
    ,STR(SMEM_SIP7_L2_MLL_BIER_TABLE_FIELDS_RESERVED_0                             )\
    ,STR(SMEM_SIP7_L2_MLL_BIER_TABLE_FIELDS_USE_VIDX_0                             )\
    ,STR(SMEM_SIP7_L2_MLL_BIER_TABLE_FIELDS_TARGET_IS_TRUNK_0                      )\
    ,STR(SMEM_SIP7_L2_MLL_BIER_TABLE_FIELDS_USE_VPORT_0                            )\
    ,STR(SMEM_SIP7_L2_MLL_BIER_TABLE_FIELDS_TRG_EPORT_0                            )\
    ,STR(SMEM_SIP7_L2_MLL_BIER_TABLE_FIELDS_TRG_TRUNK_ID_0                         )\
    ,STR(SMEM_SIP7_L2_MLL_BIER_TABLE_FIELDS_VIDX_0                                 )\
    ,STR(SMEM_SIP7_L2_MLL_BIER_TABLE_FIELDS_VPORT_0                                )\
    ,STR(SMEM_SIP7_L2_MLL_BIER_TABLE_FIELDS_UNKNOWN_UC_FILTER_ENABLE_0             )\
    ,STR(SMEM_SIP7_L2_MLL_BIER_TABLE_FIELDS_UNREGISTERED_MC_FILTER_ENABLE_0        )\
    ,STR(SMEM_SIP7_L2_MLL_BIER_TABLE_FIELDS_BC_FILTER_ENABLE_0                     )\
    ,STR(SMEM_SIP7_L2_MLL_BIER_TABLE_FIELDS_TRG_DEV_0                              )\
    ,STR(SMEM_SIP7_L2_MLL_BIER_TABLE_FIELDS_ONE_PLUS_ONE_FILTERING_ENABLE_0        )\
    ,STR(SMEM_SIP7_L2_MLL_BIER_TABLE_FIELDS_TTL_THRESHOLD_0                        )\
    ,STR(SMEM_SIP7_L2_MLL_BIER_TABLE_FIELDS_MESH_ID_0                              )\
    ,STR(SMEM_SIP7_L2_MLL_BIER_TABLE_FIELDS_MC_LOCAL_SWITCHING_ENABLE_0            )\
    ,STR(SMEM_SIP7_L2_MLL_BIER_TABLE_FIELDS_MAX_HOP_COUNT_ENABLE_0                 )\
    ,STR(SMEM_SIP7_L2_MLL_BIER_TABLE_FIELDS_MAX_OUTGOING_HOP_COUNT_0               )\
    ,STR(SMEM_SIP7_L2_MLL_BIER_TABLE_FIELDS_RESERVED_1                             )\
    ,STR(SMEM_SIP7_L2_MLL_BIER_TABLE_FIELDS_LAST_1                                 )\
    ,STR(SMEM_SIP7_L2_MLL_BIER_TABLE_FIELDS_BIND_TO_MLL_COUNTER_1                  )\
    ,STR(SMEM_SIP7_L2_MLL_BIER_TABLE_FIELDS_MASK_BITMAP_1                          )\
    ,STR(SMEM_SIP7_L2_MLL_BIER_TABLE_FIELDS_E_TREE_LEAF_1                          )\
    ,STR(SMEM_SIP7_L2_MLL_BIER_TABLE_FIELDS_BIER_MASK_PACKET_BIT_STRING_1          )\
    ,STR(SMEM_SIP7_L2_MLL_BIER_TABLE_FIELDS_RESERVED_2                             )\
    ,STR(SMEM_SIP7_L2_MLL_BIER_TABLE_FIELDS_USE_VIDX_1                             )\
    ,STR(SMEM_SIP7_L2_MLL_BIER_TABLE_FIELDS_TARGET_IS_TRUNK_1                      )\
    ,STR(SMEM_SIP7_L2_MLL_BIER_TABLE_FIELDS_USE_VPORT_1                            )\
    ,STR(SMEM_SIP7_L2_MLL_BIER_TABLE_FIELDS_TRG_EPORT_1                            )\
    ,STR(SMEM_SIP7_L2_MLL_BIER_TABLE_FIELDS_TRG_TRUNK_ID_1                         )\
    ,STR(SMEM_SIP7_L2_MLL_BIER_TABLE_FIELDS_VIDX_1                                 )\
    ,STR(SMEM_SIP7_L2_MLL_BIER_TABLE_FIELDS_VPORT_1                                 )\
    ,STR(SMEM_SIP7_L2_MLL_BIER_TABLE_FIELDS_UNKNOWN_UC_FILTER_ENABLE_1             )\
    ,STR(SMEM_SIP7_L2_MLL_BIER_TABLE_FIELDS_UNREGISTERED_MC_FILTER_ENABLE_1        )\
    ,STR(SMEM_SIP7_L2_MLL_BIER_TABLE_FIELDS_BC_FILTER_ENABLE_1                     )\
    ,STR(SMEM_SIP7_L2_MLL_BIER_TABLE_FIELDS_TRG_DEV_1                              )\
    ,STR(SMEM_SIP7_L2_MLL_BIER_TABLE_FIELDS_ONE_PLUS_ONE_FILTERING_ENABLE_1        )\
    ,STR(SMEM_SIP7_L2_MLL_BIER_TABLE_FIELDS_TTL_THRESHOLD_1                        )\
    ,STR(SMEM_SIP7_L2_MLL_BIER_TABLE_FIELDS_MESH_ID_1                              )\
    ,STR(SMEM_SIP7_L2_MLL_BIER_TABLE_FIELDS_MC_LOCAL_SWITCHING_ENABLE_1            )\
    ,STR(SMEM_SIP7_L2_MLL_BIER_TABLE_FIELDS_MAX_HOP_COUNT_ENABLE_1                 )\
    ,STR(SMEM_SIP7_L2_MLL_BIER_TABLE_FIELDS_MAX_OUTGOING_HOP_COUNT_1               )\
    ,STR(SMEM_SIP7_L2_MLL_BIER_TABLE_FIELDS_RESERVED_3                             )\
    ,STR(SMEM_SIP7_L2_MLL_BIER_TABLE_FIELDS_ENTRY_SELECTOR                         )\
    ,STR(SMEM_SIP7_L2_MLL_BIER_TABLE_FIELDS_L2_NEXT_MLL_PTR                        )\
    ,STR(SMEM_SIP7_L2_MLL_BIER_TABLE_FIELDS_BIER_F_BM_ENTRY_0                      )\
    ,STR(SMEM_SIP7_L2_MLL_BIER_TABLE_FIELDS_BIER_F_BM_ENTRY_1                      )\
    ,STR(SMEM_SIP7_L2_MLL_BIER_TABLE_FIELDS_RESERVED_4                             )


static char * sip7L2MllBierFieldsTableNames[
         SMEM_SIP7_L2_MLL_BIER_TABLE_FIELDS___LAST_VALUE___E] =
         {SMEM_SIP7_L2_MLL_BIER_TABLE_FIELDS_NAME};

static SNET_ENTRY_FORMAT_TABLE_STC sip7L2MllBierTableFieldsFormat[
        SMEM_SIP7_L2_MLL_BIER_TABLE_FIELDS___LAST_VALUE___E] =
{
    /*SMEM_SIP7_L2_MLL_BIER_TABLE_FIELDS_LAST_0                                    */
        STANDARD_FIELD_MAC(1),
    /*SMEM_SIP7_L2_MLL_BIER_TABLE_FIELDS_BIND_TO_MLL_COUNTER_0                     */
        STANDARD_FIELD_MAC(2),
    /*SMEM_SIP7_L2_MLL_BIER_TABLE_FIELDS_MASK_BITMAP_0                             */
        STANDARD_FIELD_MAC(15),
    /*SMEM_SIP7_L2_MLL_BIER_TABLE_FIELDS_E_TREE_LEAF_0                             */
        STANDARD_FIELD_MAC(1),
    /*SMEM_SIP7_L2_MLL_BIER_TABLE_FIELDS_BIER_MASK_PACKET_BIT_STRING_0             */
        STANDARD_FIELD_MAC(1),
    /*SMEM_SIP7_L2_MLL_BIER_TABLE_FIELDS_RESERVED_0                                */
        STANDARD_FIELD_MAC(1),
    /*SMEM_SIP7_L2_MLL_BIER_TABLE_FIELDS_USE_VIDX_0                                */
        EXPLICIT_FIELD_MAC(21,1),

        /*UseVIDX0 = "False"*/
        /*SMEM_SIP7_L2_MLL_BIER_TABLE_FIELDS_TARGET_IS_TRUNK_0                         */
            {FIELD_SET_IN_RUNTIME_CNS,
             1,
             SMEM_SIP7_L2_MLL_BIER_TABLE_FIELDS_USE_VIDX_0},

        /*SMEM_SIP7_L2_MLL_BIER_TABLE_FIELDS_USE_VPORT_0*/
            STANDARD_FIELD_MAC(1),


            /*(UseVIDX0 = "False") and (TargetIsTrunk0 = "False")*/
            /*SMEM_SIP7_L2_MLL_BIER_TABLE_FIELDS_TRG_EPORT_0                               */
            {FIELD_SET_IN_RUNTIME_CNS,
             15,
             SMEM_SIP7_L2_MLL_BIER_TABLE_FIELDS_USE_VPORT_0},

            /*(UseVIDX0 = "False") and (TargetIsTrunk0 = "True")*/
            /*SMEM_SIP7_L2_MLL_BIER_TABLE_FIELDS_TRG_TRUNK_ID_0                             */
            {FIELD_SET_IN_RUNTIME_CNS,
             12,
             SMEM_SIP7_L2_MLL_BIER_TABLE_FIELDS_USE_VPORT_0},

        /*UseVIDX0 = "True"*/
        /*SMEM_SIP7_L2_MLL_BIER_TABLE_FIELDS_VIDX_0                                    */
            {FIELD_SET_IN_RUNTIME_CNS,
             14,
             SMEM_SIP7_L2_MLL_BIER_TABLE_FIELDS_USE_VPORT_0},

    /*SMEM_SIP7_L2_MLL_BIER_TABLE_FIELDS_VPORT_0*/
            {FIELD_SET_IN_RUNTIME_CNS,
             18,
             SMEM_SIP7_L2_MLL_BIER_TABLE_FIELDS_USE_VPORT_0},


    /*SMEM_SIP7_L2_MLL_BIER_TABLE_FIELDS_UNKNOWN_UC_FILTER_ENABLE_0                */
        STANDARD_FIELD_MAC(1),
    /*SMEM_SIP7_L2_MLL_BIER_TABLE_FIELDS_UNREGISTERED_MC_FILTER_ENABLE_0           */
        STANDARD_FIELD_MAC(1),
    /*SMEM_SIP7_L2_MLL_BIER_TABLE_FIELDS_BC_FILTER_ENABLE_0                        */
        STANDARD_FIELD_MAC(1),

    /*(UseVIDX0 = "False") and (TargetIsTrunk0 = "False")*/
    /*SMEM_SIP7_L2_MLL_BIER_TABLE_FIELDS_TRG_DEV_0                                 */
        STANDARD_FIELD_MAC(10),

    /*SMEM_SIP7_L2_MLL_BIER_TABLE_FIELDS_ONE_PLUS_ONE_FILTERING_ENABLE_0           */
        EXPLICIT_FIELD_MAC(63,1),
    /*SMEM_SIP7_L2_MLL_BIER_TABLE_FIELDS_TTL_THRESHOLD_0                           */
        {FIELD_SET_IN_RUNTIME_CNS,
         8,
         SMEM_SIP7_L2_MLL_BIER_TABLE_FIELDS_TRG_DEV_0},
    /*SMEM_SIP7_L2_MLL_BIER_TABLE_FIELDS_MESH_ID_0                                 */
         {FIELD_SET_IN_RUNTIME_CNS,
         8,
         SMEM_SIP7_L2_MLL_BIER_TABLE_FIELDS_ONE_PLUS_ONE_FILTERING_ENABLE_0},
    /*SMEM_SIP7_L2_MLL_BIER_TABLE_FIELDS_MC_LOCAL_SWITCHING_ENABLE_0               */
        STANDARD_FIELD_MAC(1),
    /*SMEM_SIP7_L2_MLL_BIER_TABLE_FIELDS_MAX_HOP_COUNT_ENABLE_0                    */
        STANDARD_FIELD_MAC(1),
    /*SMEM_SIP7_L2_MLL_BIER_TABLE_FIELDS_MAX_OUTGOING_HOP_COUNT_0                  */
        STANDARD_FIELD_MAC(6),
    /*SMEM_SIP7_L2_MLL_BIER_TABLE_FIELDS_RESERVED_1*/
        STANDARD_FIELD_MAC(4),


    /*SMEM_SIP7_L2_MLL_BIER_TABLE_FIELDS_LAST_1                                    */
        STANDARD_FIELD_MAC(1),
    /*SMEM_SIP7_L2_MLL_BIER_TABLE_FIELDS_BIND_TO_MLL_COUNTER_1                     */
        STANDARD_FIELD_MAC(2),
    /*SMEM_SIP7_L2_MLL_BIER_TABLE_FIELDS_MASK_BITMAP_1                             */
        STANDARD_FIELD_MAC(15),
    /*SMEM_SIP7_L2_MLL_BIER_TABLE_FIELDS_E_TREE_LEAF_1_E                             */
        STANDARD_FIELD_MAC(1),
    /*SMEM_SIP7_L2_MLL_BIER_TABLE_FIELDS_BIER_MASK_PACKET_BIT_STRING_1_E                             */
        STANDARD_FIELD_MAC(1),
    /*SMEM_SIP7_L2_MLL_BIER_TABLE_FIELDS_RESERVED_2                               */
        STANDARD_FIELD_MAC(1),
    /*SMEM_SIP7_L2_MLL_BIER_TABLE_FIELDS_USE_VIDX_1                                */
        EXPLICIT_FIELD_MAC(105,1),

        /*UseVIDX0 = "False"*/
        /*SMEM_SIP7_L2_MLL_BIER_TABLE_FIELDS_TARGET_IS_TRUNK_1                         */
            {FIELD_SET_IN_RUNTIME_CNS,
             1,
             SMEM_SIP7_L2_MLL_BIER_TABLE_FIELDS_USE_VIDX_1},

        /*SMEM_SIP7_L2_MLL_BIER_TABLE_FIELDS_USE_VPORT_1*/
            STANDARD_FIELD_MAC(1),

            /*(UseVIDX0 = "False") and (TargetIsTrunk0 = "False")*/
            /*SMEM_SIP7_L2_MLL_BIER_TABLE_FIELDS_TRG_EPORT_1                               */
            {FIELD_SET_IN_RUNTIME_CNS,
             15,
             SMEM_SIP7_L2_MLL_BIER_TABLE_FIELDS_USE_VPORT_1},

            /*(UseVIDX0 = "False") and (TargetIsTrunk0 = "True")*/
            /*SMEM_SIP7_L2_MLL_BIER_TABLE_FIELDS_TRG_TRUNK_ID_1                             */
            {FIELD_SET_IN_RUNTIME_CNS,
             12,
             SMEM_SIP7_L2_MLL_BIER_TABLE_FIELDS_USE_VPORT_1},

        /*UseVIDX0 = "True"*/
        /*SMEM_SIP7_L2_MLL_BIER_TABLE_FIELDS_VIDX_1                                    */
            {FIELD_SET_IN_RUNTIME_CNS,
             14,
             SMEM_SIP7_L2_MLL_BIER_TABLE_FIELDS_USE_VPORT_1},

    /*SMEM_SIP7_L2_MLL_BIER_TABLE_FIELDS_VPORT_1*/
            {FIELD_SET_IN_RUNTIME_CNS,
             18,
             SMEM_SIP7_L2_MLL_BIER_TABLE_FIELDS_USE_VPORT_1},

    /*SMEM_SIP7_L2_MLL_BIER_TABLE_FIELDS_UNKNOWN_UC_FILTER_ENABLE_1                */
        STANDARD_FIELD_MAC(1),
    /*SMEM_SIP7_L2_MLL_BIER_TABLE_FIELDS_UNREGISTERED_MC_FILTER_ENABLE_1           */
        STANDARD_FIELD_MAC(1),
    /*SMEM_SIP7_L2_MLL_BIER_TABLE_FIELDS_BC_FILTER_ENABLE_1                        */
        STANDARD_FIELD_MAC(1),

    /*(UseVIDX0 = "False") and (TargetIsTrunk0 = "False")*/
    /*SMEM_SIP7_L2_MLL_BIER_TABLE_FIELDS_TRG_DEV_1                                 */
        STANDARD_FIELD_MAC(10),

    /*SMEM_SIP7_L2_MLL_BIER_TABLE_FIELDS_ONE_PLUS_ONE_FILTERING_ENABLE_1           */
       EXPLICIT_FIELD_MAC(147,1),
    /*SMEM_SIP7_L2_MLL_BIER_TABLE_FIELDS_TTL_THRESHOLD_1                           */
        {FIELD_SET_IN_RUNTIME_CNS,
         8,
         SMEM_SIP7_L2_MLL_BIER_TABLE_FIELDS_TRG_DEV_1},
    /*SMEM_SIP7_L2_MLL_BIER_TABLE_FIELDS_MESH_ID_1                                 */
         {FIELD_SET_IN_RUNTIME_CNS,
         8,
         SMEM_SIP7_L2_MLL_BIER_TABLE_FIELDS_ONE_PLUS_ONE_FILTERING_ENABLE_1},
    /*SMEM_SIP7_L2_MLL_BIER_TABLE_FIELDS_MC_LOCAL_SWITCHING_ENABLE_1               */
        STANDARD_FIELD_MAC(1),
    /*SMEM_SIP7_L2_MLL_BIER_TABLE_FIELDS_MAX_HOP_COUNT_ENABLE_1                    */
        STANDARD_FIELD_MAC(1),
    /*SMEM_SIP7_L2_MLL_BIER_TABLE_FIELDS_MAX_OUTGOING_HOP_COUNT_1                  */
        STANDARD_FIELD_MAC(6),
    /*SMEM_SIP7_L2_MLL_BIER_TABLE_FIELDS_RESERVED_3*/
        STANDARD_FIELD_MAC(3),

    /*SMEM_SIP7_L2_MLL_BIER_TABLE_FIELDS_ENTRY_SELECTOR                            */
        STANDARD_FIELD_MAC(1),
    /*SMEM_SIP7_L2_MLL_BIER_TABLE_FIELDS_L2_NEXT_MLL_PTR                           */
        STANDARD_FIELD_MAC(17),

    /*SMEM_SIP7_L2_MLL_BIER_TABLE_FIELDS_BIER_F_BM_ENTRY_0                         */
        STANDARD_FIELD_MAC(256),
    /*SMEM_SIP7_L2_MLL_BIER_TABLE_FIELDS_BIER_F_BM_ENTRY_1                        */
        STANDARD_FIELD_MAC(256),
    /*SMEM_SIP7_L2_MLL_BIER_TABLE_FIELDS_RESERVED_4                               */
        STANDARD_FIELD_MAC(45)
};

/**
* @internal snetAasMllTablesFormatInit function
* @endinternal
*
* @brief   init the format of IPVX tables.
*
* @param[in] devObjPtr                - pointer to device object.
*/
void snetAasMllTablesFormatInit(
    IN SKERNEL_DEVICE_OBJECT            * devObjPtr
)
{
    LION3_TABLES_FORMAT_INIT_MAC(
         devObjPtr, SKERNEL_TABLE_FORMAT_IP_MLL_BIER_E,
         sip7IpMllBierTableFieldsFormat, sip7IpMllBierFieldsTableNames);

    LION3_TABLES_FORMAT_INIT_MAC(
         devObjPtr, SKERNEL_TABLE_FORMAT_L2_MLL_BIER_E,
         sip7L2MllBierTableFieldsFormat, sip7L2MllBierFieldsTableNames);
}
