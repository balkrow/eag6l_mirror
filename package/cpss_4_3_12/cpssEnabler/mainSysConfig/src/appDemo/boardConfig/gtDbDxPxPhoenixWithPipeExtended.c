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
* @file gtDbDxPxPhoenixWithPipeExtended.c
*
* @brief Initialization functions for the the AC5X and PIPE.
*       NOTE: this file compiled only when both CHX_FAMILY and PX_FAMILY defined.
*
* @version   1
********************************************************************************
*/
#include <appDemo/boardConfig/appDemoBoardConfig.h>
#include <appDemo/boardConfig/gtDbDxBoardTypeConfig.h>
#include <appDemo/boardConfig/appDemoCfgMisc.h>
#include <appDemo/sysHwConfig/gtAppDemoSysConfigDefaults.h>
#include <appDemo/utils/mainUT/appDemoUtils.h>
#include <appDemo/sysHwConfig/appDemoDb.h>
#include <appDemo/boardConfig/gtDbDxPhoenixWithPipeExtended.h>
#include <appDemo/boardConfig/gtBoardsConfigFuncs.h>

/* DXCH includes */
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortMapping.h>
#include <cpss/dxCh/dxChxGen/cscd/cpssDxChCscd.h>
#include <cpss/dxCh/dxChxGen/trunk/cpssDxChTrunk.h>
#include <cpss/dxCh/dxChxGen/nst/cpssDxChNstPortIsolation.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortManager.h>

/* PX includes */
#include <cpss/px/ingress/cpssPxIngress.h>
#include <cpss/px/port/cpssPxPortMapping.h>
#include <cpss/px/config/private/prvCpssPxInfo.h>
#include <cpss/px/ingress/cpssPxIngressHash.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

extern GT_STATUS px_force_Ports_0_3_10G_12_13_25G(void);
extern GT_BOOL     px_forwardingTablesStage_enabled;

#define PORT_SKIP_CHECK(dev,port)                                             \
    {                                                                         \
        GT_BOOL     isValid;                                                  \
        rc = cpssPxPortPhysicalPortMapIsValidGet(devNum, portNum , &isValid); \
        if(rc != GT_OK || isValid == GT_FALSE)                                \
        {                                                                     \
            continue;                                                         \
        }                                                                     \
    }

#define NUM_CASCADE_PORTS   2
#define NUM_REMOTE_PORTS    4

AC5X_PORT_MAP_STC ac5x_with_rpp_for_pipe_port_mode[] =
{
    /* ports from USX mac */
     {{ 0,  1,  2,  3,  4,  5,  6,  7,  GT_NA} , { 0,  1,  2,  3,  4,  5,  6,  7,  GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E}
    ,{{ 8,  9,  10, 11, 12, 13, 14, 15, GT_NA} , { 8,  9,  10, 11, 12, 13, 14, 15, GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E}
    ,{{ 16, 17, 18, 19, 20, 21, 22, 23, GT_NA} , { 16, 17, 18, 19, 20, 21, 22, 23, GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E}
    ,{{ 24, 25, 26, 27, 28, 29, 30, 31, GT_NA} , { 24, 25, 26, 27, 28, 29, 30, 31, GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E}
    ,{{ 32, 33, 34, 35, 36, 37, 38, 39, GT_NA} , { 32, 33, 34, 35, 36, 37, 38, 39, GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E}
    ,{{ 40, 41, 42, 43, 44, 45, 46, 47, GT_NA} , { 40, 41, 42, 43, 44, 45, 46, 47, GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E}
    /* cascde ports from 25G mac */
    ,{{48,49,                   GT_NA} , {48,49,                    GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E}
    /* remote ports */
    ,{{100,101,102,103,         GT_NA} , {48,48,49,49,              GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E}
    /* ports from 50G mac */
    ,{{50,                      GT_NA} , {50,                       GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E}
    ,{{52,                      GT_NA} , {52,                       GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E}
     /* skip physical 62 */
    ,{{CPU_PORT,/*63*/          GT_NA} , {54,/*DMA - bound to MG0*/ GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    }
     /* map second 'CPU SDMA port' */
    ,{{60,                      GT_NA} , {55,/*DMA - bound to MG1*/ GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    }
};
/* value set in runtime with ac5x_with_rpp_for_pipe_port_mode */
GT_U32   actualNum_ac5x_with_rpp_for_pipe_port_mode = sizeof(ac5x_with_rpp_for_pipe_port_mode)/sizeof(ac5x_with_rpp_for_pipe_port_mode[0]);

PortInitList_STC ac5x_with_rpp_for_pipe_portInitlist[] =
{
    /* ports in 1G */
     { PORT_LIST_TYPE_INTERVAL, { 0,31,1,               APP_INV_PORT_CNS },  CPSS_PORT_SPEED_1000_E,    CPSS_PORT_INTERFACE_MODE_USX_OUSGMII_E  }
    /* ports in 2.5G */
    ,{ PORT_LIST_TYPE_INTERVAL, { 32,47,1,              APP_INV_PORT_CNS },  CPSS_PORT_SPEED_2500_E,    CPSS_PORT_INTERFACE_MODE_USX_20G_OXGMII_E  }
    /* ports in 25G */
    ,{ PORT_LIST_TYPE_LIST,     { 48,49,                APP_INV_PORT_CNS }, CPSS_PORT_SPEED_25000_E,    CPSS_PORT_INTERFACE_MODE_KR_E}
    /* remote ports */
    ,{ PORT_LIST_TYPE_LIST,     { 100,101,102,103,      APP_INV_PORT_CNS }, CPSS_PORT_SPEED_REMOTE_E,   CPSS_PORT_INTERFACE_MODE_REMOTE_E}
    /* ports in 50G  */
    ,{ PORT_LIST_TYPE_LIST,     { 50,52,                APP_INV_PORT_CNS }, CPSS_PORT_SPEED_50000_E,    CPSS_PORT_INTERFACE_MODE_KR2_E}

    ,{ PORT_LIST_TYPE_EMPTY,    {APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,   CPSS_PORT_INTERFACE_MODE_QSGMII_E    }
};

/* Defines ports mapping between DX cascade and remote ports */
typedef struct 
{
    GT_U32  dxCascadePort;          /* cascade port */
    GT_U32  dxRemotePhysicalPort;   /* remote physical port */
    GT_U32  dxRemoteMacNumber;      /* remote MAC ports */
}PHOENIX_PIPE_CASCADE_PORTS_STC;

/* Physical ports represent cascade ports on AC5X device */
#define DX_PHYSICAL_CSCD_PORT_CASCADE_0             48
#define DX_PHYSICAL_CSCD_PORT_CASCADE_1             49

/* MAC ports represent cascade ports on AC5X device */
#define DX_MAC_CSCD_PORT_CASCADE_0                  48
#define DX_MAC_CSCD_PORT_CASCADE_1                  49

/* Physical ports on remote (PIPE) device */
#define DX_REMOTE_PHYSICAL_PORT_0                   100
#define DX_REMOTE_PHYSICAL_PORT_1                   102

/* MAC ports on remote (PIPE) device */
#define DX_REMOTE_MAC_NUMBER_0                      0
#define DX_REMOTE_MAC_NUMBER_1                      2


/* Used by pipeFrontPanelPortsSet(...)  with the 'defaults' of 0x8100 */
static GT_U16   tpidTable[8/* TPID index 0..7 */] = {
     0x8100
    ,0x8100
    ,0x8100
    ,0x8100
    ,0x8100
    ,0x8100
    ,0x8100
    ,0x8100
};

/* per port info */
typedef struct{
    GT_BOOL     isPortMappedAsRemote;/* indication that the port is used as remote on DX device */
    GT_U32      remotePortNumberOnDx;/* valid when isPortMappedAsRemote == GT_TRUE */
    GT_U32      tpidProfile[2];/* index into tpidProfileBmp[0 or 1][x] */
} PIPE_PORT_INFO_STC;

/* (based on) similar to CPSS_DXCH_CFG_REMOTE_PHY_MAC_INFO_STC */
typedef struct{
    GT_U32 numOfRemotePorts;
    GT_U32 startPhysicalPortNumber;
    GT_U32 startRemoteMacPortNum;
    GT_SW_DEV_NUM  cpssDevNum;/* NOTE: set by default like appDemoDevIndex , but may be changed after 'removel of device' and re-insert to new deviceNumber */
    GT_U32 dxCascadePortNum;/* physical port number of the cascade port from the DX device side */
    GT_U8  appDemoDevIndex;/* index in appDemoPpConfigList */
    PIPE_PORT_INFO_STC  portInfo[16];
} PIPE_REMOTE_DEV_INFO_STC;

#define PIPE_NETWORK_PORTS_NUMBER_CNS             4

static PIPE_REMOTE_DEV_INFO_STC pipe_Info =
{
     PIPE_NETWORK_PORTS_NUMBER_CNS/*numOfRemotePorts*/
    ,DX_PHYSICAL_CSCD_PORT_CASCADE_0 /*startPhysicalPortNumber*/
    ,0/*startRemoteMacPortNum*/
    ,1/* cpssDevNum */
    ,DX_MAC_CSCD_PORT_CASCADE_0/*dxCascadePortNum*/
    ,1/*appDemoDevIndex*/
    ,{{GT_FALSE, 0, {0,0}}}
};

static PHOENIX_PIPE_CASCADE_PORTS_STC  cascadePorts[] = 
{
    {DX_PHYSICAL_CSCD_PORT_CASCADE_0, DX_REMOTE_PHYSICAL_PORT_0, DX_REMOTE_MAC_NUMBER_0},
    {DX_PHYSICAL_CSCD_PORT_CASCADE_1, DX_REMOTE_PHYSICAL_PORT_1, DX_REMOTE_MAC_NUMBER_1}
};

typedef enum{
    SRC_PORT_PROFILE_FRONT_UPLINK_E = 0,
    SRC_PORT_PROFILE_FRONT_PANEL_E
}SRC_PORT_PROFILE_E;

typedef enum{
    /************************************/
    /* profiles for 'front panel' ports */
    /************************************/
    /* BIT 0 */
    SRC_PORT_PROFILE_BMP_FRONT_PANEL_E                                  = 1 << 0   ,
    /* BIT 1 */
    SRC_PORT_PROFILE_BMP_FRONT_PANEL_USE_TPID_0_AS_TAG0_E               = 1 << 1   ,
    /* BIT 2 */
    SRC_PORT_PROFILE_BMP_FRONT_PANEL_USE_TPID_1_AS_TAG0_E               = 1 << 2   ,
    /* BIT 3 */
    SRC_PORT_PROFILE_BMP_FRONT_PANEL_USE_TPID_0_AS_TAG0_WITH_TAG1_E     = 1 << 3   ,
    /* BIT 4 */
    SRC_PORT_PROFILE_BMP_FRONT_PANEL_USE_TPID_1_AS_TAG0_WITH_TAG1_E     = 1 << 4   ,
    /* BIT 5 */
    SRC_PORT_PROFILE_BMP_FRONT_PANEL_USE_TPID_2_AS_TAG1_E               = 1 << 5   ,
    /* BIT 6 */
    SRC_PORT_PROFILE_BMP_FRONT_PANEL_USE_TPID_3_AS_TAG1_E               = 1 << 6   ,
}SRC_PORT_PROFILE_BMP_E;

typedef enum{
            /* uplink */
/* 0*/    PIPE_INGRESS_TRAFFIC_TYPE_UPLINK_E,
/* 1*/    PIPE_INGRESS_TRAFFIC_TYPE_UPLINK_INVALID_E,        /* 'fallback' if not matched in 'uplink' */

            /* front panel */
/* 2*/    PIPE_INGRESS_TRAFFIC_TYPE_FRONT_PANEL_TPID_0_TAG0_TPID_2_TAG1_E,
/* 3*/    PIPE_INGRESS_TRAFFIC_TYPE_FRONT_PANEL_TPID_1_TAG0_TPID_2_TAG1_E,

/* 4*/    PIPE_INGRESS_TRAFFIC_TYPE_FRONT_PANEL_TPID_0_TAG0_TPID_3_TAG1_E,
/* 5*/    PIPE_INGRESS_TRAFFIC_TYPE_FRONT_PANEL_TPID_1_TAG0_TPID_3_TAG1_E,

/* 6*/    PIPE_INGRESS_TRAFFIC_TYPE_FRONT_PANEL_TPID_0_TAG0_E,
/* 7*/    PIPE_INGRESS_TRAFFIC_TYPE_FRONT_PANEL_TPID_1_TAG0_E,

/* 8*/    PIPE_INGRESS_TRAFFIC_TYPE_FRONT_PANEL_TPID_2_TAG1_E,
/* 9*/    PIPE_INGRESS_TRAFFIC_TYPE_FRONT_PANEL_TPID_3_TAG1_E,

/*10*/    PIPE_INGRESS_TRAFFIC_TYPE_FRONT_PANEL_UNTAGGED_E, /* 'fallback' if not matched in any of the tag0/1 */

}PIPE_INGRESS_TRAFFIC_TYPE_ENT;

typedef struct{
    GT_U32                                      tpidIndex;
    GT_U32                                      tpid1Index;  /* relevant for tag1 or tag0+tag1*/
    PIPE_INGRESS_TRAFFIC_TYPE_ENT               appDemoType;
    CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_ENT   cpssType;
    GT_U32                                      srcProfileId;/* combination of values from SRC_PORT_PROFILE_E */
}APPDEMO_TO_CPSS_TRAFFIC_TYPE_STC;

static APPDEMO_TO_CPSS_TRAFFIC_TYPE_STC frontPanel_appDemoToCpssTrafficType[] =
{    /*tpidIndex*//*tpid1Index*/     /*appDemoType*/                                                  /*cpssType*/                                                                    /*srcProfileId*/
     {0          ,    2          ,PIPE_INGRESS_TRAFFIC_TYPE_FRONT_PANEL_TPID_0_TAG0_TPID_2_TAG1_E , CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EDSA_EXTENDED_PORT_DTAGGED_TO_UPSTREAM_PORT_E       ,SRC_PORT_PROFILE_BMP_FRONT_PANEL_USE_TPID_0_AS_TAG0_WITH_TAG1_E}
    ,{1          ,    2          ,PIPE_INGRESS_TRAFFIC_TYPE_FRONT_PANEL_TPID_1_TAG0_TPID_2_TAG1_E , CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EDSA_EXTENDED_PORT_DTAGGED_TO_UPSTREAM_PORT_E       ,SRC_PORT_PROFILE_BMP_FRONT_PANEL_USE_TPID_1_AS_TAG0_WITH_TAG1_E}

    ,{0          ,    3          ,PIPE_INGRESS_TRAFFIC_TYPE_FRONT_PANEL_TPID_0_TAG0_TPID_3_TAG1_E , CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EDSA_EXTENDED_PORT_DTAGGED_TO_UPSTREAM_PORT_E       ,SRC_PORT_PROFILE_BMP_FRONT_PANEL_USE_TPID_0_AS_TAG0_WITH_TAG1_E}
    ,{1          ,    3          ,PIPE_INGRESS_TRAFFIC_TYPE_FRONT_PANEL_TPID_1_TAG0_TPID_3_TAG1_E , CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EDSA_EXTENDED_PORT_DTAGGED_TO_UPSTREAM_PORT_E       ,SRC_PORT_PROFILE_BMP_FRONT_PANEL_USE_TPID_1_AS_TAG0_WITH_TAG1_E}

    ,{0          ,    0/*NA*/    ,PIPE_INGRESS_TRAFFIC_TYPE_FRONT_PANEL_TPID_0_TAG0_E             , CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EDSA_EXTENDED_PORT_TAGGED_TO_UPSTREAM_PORT_E        ,SRC_PORT_PROFILE_BMP_FRONT_PANEL_USE_TPID_0_AS_TAG0_E}
    ,{1          ,    0/*NA*/    ,PIPE_INGRESS_TRAFFIC_TYPE_FRONT_PANEL_TPID_1_TAG0_E             , CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EDSA_EXTENDED_PORT_TAGGED_TO_UPSTREAM_PORT_E        ,SRC_PORT_PROFILE_BMP_FRONT_PANEL_USE_TPID_1_AS_TAG0_E}

    ,{2          ,    2          ,PIPE_INGRESS_TRAFFIC_TYPE_FRONT_PANEL_TPID_2_TAG1_E             , CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EDSA_EXTENDED_PORT_TAG1_TAGGED_TO_UPSTREAM_PORT_E   ,SRC_PORT_PROFILE_BMP_FRONT_PANEL_USE_TPID_2_AS_TAG1_E}
    ,{3          ,    3          ,PIPE_INGRESS_TRAFFIC_TYPE_FRONT_PANEL_TPID_3_TAG1_E             , CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EDSA_EXTENDED_PORT_TAG1_TAGGED_TO_UPSTREAM_PORT_E   ,SRC_PORT_PROFILE_BMP_FRONT_PANEL_USE_TPID_3_AS_TAG1_E}

    ,{0          ,    0/*NA*/    ,PIPE_INGRESS_TRAFFIC_TYPE_FRONT_PANEL_UNTAGGED_E                , CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EDSA_EXTENDED_PORT_UNTAGGED_TO_UPSTREAM_PORT_E      ,0/* no extra*/}

    /* must be last */
    ,{GT_NA,0,0,0,0}
};

static GT_BOARD_CONFIG_FUNCS pipe_genericBoardConfigFun;

#define PIPE_REMOTE_PHYSICAL_PORT_BOARD_ID   0xFF /*dummy*/
/* base index in the table of 'destination' table for the traffic ingress from the 'front panel' */
#define PIPE_FRONT_PANEL_DESTINATION_BASE_INDEX 0
/* base index in the table of 'destination' table for the traffic ingress from the 'uplink' */
#define PIPE_UPLINK_DESTINATION_BASE_INDEX      0x100

/*******************************************************************************
* pipe_treatLinkStatusChange
*
* DESCRIPTION:
*       function called for 'link status change'
*       the PIPE device need to notify the DX device that the MAC of the port
*       hold link status change.
*
* INPUTS:
*       devNum     - physical device number
*       portNum    - physical port
*       linkUp     - indication that the port is currently 'up'/'down'
*                   relevant only to 'APP_DEMO_CALL_BACK_ON_LINK_STATUS_CHANGE_AFTER_MAIN_LOGIC_E'
*       partIndex  - indication of calling before main logic or after main logic.
*
* OUTPUTS:
*       stopLogicPtr - (pointer to) indication to stop the logic after CB ended.
*                      NOTE: relevant to 'APP_DEMO_CALL_BACK_ON_LINK_STATUS_CHANGE_BEFORE_MAIN_LOGIC_E'
*                       GT_TRUE - stop the logic after CB ended.
*                            meaning caller will not call the main logic
*                       GT_TRUE - continue the logic after CB ended.
*                            meaning caller call the main logic
*
* RETURNS:
*       GT_OK   - on success,
*       GT_FAIL - otherwise.
*
* COMMENTS:
*
*
*******************************************************************************/
static GT_STATUS pipe_treatLinkStatusChange
(
IN GT_U8     devNum,
IN GT_PHYSICAL_PORT_NUM  portNum,
IN GT_BOOL  linkUp,
IN APP_DEMO_CALL_BACK_ON_LINK_STATUS_CHANGE_ENT partIndex,
OUT GT_BOOL     *stopLogicPtr
)
{
    GT_STATUS   rc;
    PIPE_REMOTE_DEV_INFO_STC * internal_infoPtr = &pipe_Info;
    GT_U32                  dxPortNum;
    CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_ENT portLinkStatusState;
    CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_ENT old_portLinkStatusState;
    GT_U8 ac5xDevNum;

    *stopLogicPtr = GT_FALSE;
    if (!PRV_CPSS_PP_MAC(devNum)) {
        /* support calling from gtDbDxPxBc3PipesSmiBoardReg_removedDeviceFromDb(...)
           after the PX device was removed ! */

        /* the function getPipeDevInfo_byCpssDevNum() will check that this was valid PX device */
    } else
        if (!PRV_CPSS_PX_FAMILY_CHECK_MAC(devNum)) {
        return GT_OK;
    }

    if (partIndex == APP_DEMO_CALL_BACK_ON_LINK_STATUS_CHANGE_BEFORE_MAIN_LOGIC_E) {
        return GT_OK;
    }

    if (portNum >= 16) {
        rc = GT_NO_SUCH;
        CPSS_ENABLER_DBG_TRACE_RC_MAC("unexpected portNumber", rc);
        return rc;
    }

    if (GT_FALSE == internal_infoPtr->portInfo[portNum].isPortMappedAsRemote) {
        /* the port is not mapped (like the cascade port #12..15) */
        return GT_OK;
    }
    ac5xDevNum = appDemoPpConfigList[0].devNum;
    dxPortNum = internal_infoPtr->portInfo[portNum].remotePortNumberOnDx;

    if (portMgr) {
        CPSS_PORT_MANAGER_STATUS_STC portStage;
        CPSS_PORT_MANAGER_STC portEventStc;

        rc = cpssDxChPortManagerStatusGet(ac5xDevNum, dxPortNum, &portStage);

        if (rc != GT_OK) {
            cpssOsPrintf("calling PortManagerStatusGet returned=%d, portNum=%d", rc, dxPortNum);
            return rc;
        }

        if (portStage.portState == CPSS_PORT_MANAGER_STATE_FAILURE_E ||
            portStage.portUnderOperDisable == GT_TRUE) {
            cpssOsPrintf("port is in HW failure state or in disable so abort link change notification");
            return GT_OK;
        }

        if (portStage.portState == CPSS_PORT_MANAGER_STATE_RESET_E ) {
            return GT_OK;
        }

        portEventStc.portEvent = linkUp? CPSS_PORT_MANAGER_EVENT_ENABLE_E:CPSS_PORT_MANAGER_EVENT_DISABLE_E;

        rc = cpssDxChPortManagerEventSet(ac5xDevNum, dxPortNum, &portEventStc);
        if (rc != GT_OK) {
            cpssOsPrintf("calling PortManagerEventSet event %d returned=%d, portNum=%d", portEventStc.portEvent, rc, portNum);
            return rc;
        }
        return GT_OK;
    }

    /* the remote physical port number on the DX that represents the PIPE port */
    rc = cpssDxChBrgEgrFltPortLinkEnableGet(ac5xDevNum, dxPortNum, &old_portLinkStatusState);
    if (rc != GT_OK) {
        return rc;
    }

    portLinkStatusState =  linkUp ?
                           CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_FORCE_LINK_UP_E :
                           CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_FORCE_LINK_DOWN_E;

    if (old_portLinkStatusState == portLinkStatusState) {
        /* avoid 'printings' of ports when no change in value */
        return GT_OK;
    }

    /* the port on the PIPE changed status , we need to notify the relevant DX device */
    rc = cpssDxChBrgEgrFltPortLinkEnableSet(ac5xDevNum, dxPortNum, portLinkStatusState);
    if (rc != GT_OK) {
        return rc;
    }

    if (appDemoPrintLinkChangeFlagGet()) {
        cpssOsPrintSync("LINK %s : devNum[%ld] (remote physical port[%2ld]) \n",
                        (linkUp == GT_FALSE) ? "DOWN" : "UP  ",
                        (GT_U32)ac5xDevNum, dxPortNum);
    }
    return GT_OK;
}

/* based on : internal_initConnectionTo88e1690_dxch_remote_port(...) */
static GT_STATUS internal_initConnectionToPipe_dxch_remote_port(
    IN GT_U8   devNum,
    IN GT_U32  cascadePortNum,
    IN GT_U32  physicalPortNumber,
    IN GT_U32  remoteMacNumber
)
{
    GT_STATUS rc;
    GT_U32  portNum = cascadePortNum;
    GT_U32  dsa_HwDevNum , dsa_PortNum = remoteMacNumber;

    pipe_Info.portInfo[remoteMacNumber].isPortMappedAsRemote = GT_TRUE;
    pipe_Info.portInfo[remoteMacNumber].remotePortNumberOnDx = physicalPortNumber;

    dsa_HwDevNum = 0;/* the bits from the DSA are ignored anyway ! by the PIPE */

    /* state that the port egress packets with DSA (single word) */
    rc = cpssDxChCscdPortTypeSet(devNum, portNum ,
        CPSS_PORT_DIRECTION_TX_E , CPSS_CSCD_PORT_DSA_MODE_1_WORD_E);
    if(rc != GT_OK)
    {
        return rc;
    }

    /*
         The following configuration should be applied in the Header Alteration Physical Port Table 2:
             <Transmit FORWARD packets with Forced 4B FROM_CPU bit> = 1
             <Transmit TO_ANALZER packets with Forced 4B FROM_CPU bit> = 1
             <Transmit FROM_CPU packets with Forced 4B FROM_CPU bit> = 1
    */
    rc = cpssDxChCscdPortForce4BfromCpuDsaEnableSet(devNum, portNum, GT_TRUE);
    if(rc!=GT_OK)
    {
        return rc;
    }

    rc = cpssDxChCscdCentralizedChassisRemotePhysicalPortMapEnableSet (devNum, portNum, GT_TRUE);
    if(rc != GT_OK)
    {
        return rc;
    }
    /* set info what the egress DSA should hold for target port , target device */
    rc = cpssDxChCscdCentralizedChassisRemotePhysicalPortMapSet (devNum, portNum,
        dsa_HwDevNum , dsa_PortNum);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* force the 'localDevSrcPort' to do 'briging' , because by default the bridge
       is bypassed for 'eDSA' packets , that comes from the PIPE , but the PIPE
       not aware to the bridging */
    return cpssDxChCscdPortBridgeBypassEnableSet(devNum, portNum, GT_FALSE);
}

/* based on : internal_initConnectionTo88e1690_dxch(...) */
static GT_STATUS internal_initConnectionToPipe_dxch
(
    IN GT_U8   devNum,
    IN PHOENIX_PIPE_CASCADE_PORTS_STC  *cascadePortsPtr
)
{
    GT_STATUS rc;
    GT_U32    ii;
    GT_U32  srcDevLsbAmount = 0, srcPortLsbAmount = 4;
    GT_U32  cascadePortNum = cascadePortsPtr->dxCascadePort;

    /***********************/
    /* global - per device */
    /***********************/
    rc = cpssDxChCscdCentralizedChassisMyPhysicalPortMappingDsaSrcLsbAmountSet(
        devNum, srcDevLsbAmount,srcPortLsbAmount);
    if(rc != GT_OK)
    {
        return rc;
    }

    /***********************/
    /* per cascade port    */
    /***********************/
    /* to limit errors by application that added it to vlan */
    rc = cpssDxChBrgEgrFltPortLinkEnableSet(devNum, cascadePortNum  ,
        CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_FORCE_LINK_DOWN_E);
    if(rc != GT_OK)
    {
        return rc;
    }
    /* actually AC5X gets (from PIPE) eDSA , but AC5X sends (to PIPE) '1 word 'from cpu' DSA' */
    rc = cpssDxChCscdPortTypeSet(devNum, cascadePortNum ,
        CPSS_PORT_DIRECTION_BOTH_E , CPSS_CSCD_PORT_DSA_MODE_1_WORD_E);
    if(rc != GT_OK)
    {
        return rc;
    }

    rc = cpssDxChCscdCentralizedChassisMyPhysicalPortAssignmentModeSet(devNum, cascadePortNum ,
         CPSS_DXCH_CSCD_INGRESS_PHYSICAL_PORT_ASSIGNMENT_MODE_DSA_E);
    if(rc != GT_OK)
    {
        return rc;
    }

    rc = cpssDxChCscdCentralizedChassisMyPhysicalPortBaseSet(devNum, cascadePortNum, cascadePortNum - 0);
    if(rc != GT_OK)
    {
        return rc;
    }

    rc = cpssDxChCscdCentralizedChassisMyPhysicalPortAssignSrcDevEnableSet(devNum,
        cascadePortNum, GT_TRUE);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* the 88e1690 supports Tx for max 10K , we add 16 ... to not be bottleneck in any case */
    rc = cpssDxChPortMruSet(devNum, cascadePortNum , _10K + 16);
    if(rc != GT_OK)
    {
        return rc;
    }

    /*******************************/
    /* per remote physical port    */
    /*******************************/

    for(ii = 0; ii < NUM_REMOTE_PORTS/2; ii++)
    {
        rc = internal_initConnectionToPipe_dxch_remote_port(devNum, cascadePortNum,
                cascadePortsPtr->dxRemotePhysicalPort + ii, cascadePortsPtr->dxRemoteMacNumber + ii);
        if(rc != GT_OK)
        {
            CPSS_ENABLER_DBG_TRACE_RC_MAC("internal_initConnectionToPipe_dxch_remote_port", rc);
            return rc;
        }
    }

    return GT_OK;
}

static CPSS_PX_PORTS_BMP    uplinkPortsBmp       = 0x3000;  /* 12, 13   - 25G uplink port */
static CPSS_PX_PORTS_BMP    frontPannelPortsBmp  = 0xF;     /* 0..3     - network ports */

/* set the ingress and PHA tables to forward packets from 'front panel' to 'uplink'
   + adding 'FRW' eDSA (4 words) with 'src port' info.*/
static GT_STATUS pipeFrontPanelPortsSet
(
    IN GT_SW_DEV_NUM   devNum
)
{
    GT_STATUS   rc;
    CPSS_PX_INGRESS_PORT_PACKET_TYPE_KEY_STC    portKey;
    CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC  keyData;
    CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC  keyMask;
    CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_ENT     tableType;
    CPSS_PX_INGRESS_PORT_MAP_PACKET_TYPE_FORMAT_STC packetTypeFormat;
    GT_PHYSICAL_PORT_NUM  portNum;
    GT_U32                entryIndex;
    CPSS_PX_EGRESS_HEADER_ALTERATION_ENTRY_UNT  operationInfo;
    CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_UNT        srcPortInfo;
    GT_U32  dsaPortNum;
    GT_U32  tpidIndex;
    GT_U16  tpid;
    CPSS_PX_PACKET_TYPE     packetType;
    CPSS_PX_INGRESS_TPID_ENTRY_STC  tpidEntry;

    tpidEntry.size = 2; /* BUG in CPSS (CPSS-8765 : PIPE : cpssPxIngressTpidEntrySet use 'size' as '2 bytes' resolution instead of '1 byte')
                           TPID size resolution is 2B. */
    tpidEntry.valid = GT_TRUE;

    cpssOsMemSet(&portKey, 0, sizeof(CPSS_PX_INGRESS_PORT_PACKET_TYPE_KEY_STC));
    portKey.srcPortProfile = SRC_PORT_PROFILE_FRONT_PANEL_E;
    portKey.portUdbPairArr[0].udbAnchorType = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L2_E;
    portKey.portUdbPairArr[0].udbByteOffset = 16;/* start of Tag1 after Tag0 (relevant to double tagging) */

    for (portNum = 0; portNum < 12; portNum++)
    {
        PORT_SKIP_CHECK(devNum,portNum);

        rc = cpssPxIngressPortPacketTypeKeySet(devNum, portNum, &portKey);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssPxIngressPortPacketTypeKeySet", rc);
        if (rc != GT_OK)
            return rc;
    }

    /**************************************/
    /* define and enable the traffic type */
    /**************************************/

    /* define 'traffic type' will match the src port profile */
    cpssOsMemSet(&keyData, 0, sizeof(CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC));
    cpssOsMemSet(&keyMask, 0, sizeof(CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC));
    keyMask.profileIndex = SRC_PORT_PROFILE_FRONT_PANEL_E;/* bit 0 */
    keyData.profileIndex = SRC_PORT_PROFILE_FRONT_PANEL_E;

    for(entryIndex = 0 ;
        frontPanel_appDemoToCpssTrafficType[entryIndex].tpidIndex != GT_NA;
        entryIndex++)
    {
        tpidIndex = frontPanel_appDemoToCpssTrafficType[entryIndex].tpidIndex;
        tpid = tpidTable[tpidIndex];

        if(tpidIndex < 4)
        {
            tpidEntry.val = tpid;
            rc = cpssPxIngressTpidEntrySet(devNum,tpidIndex,&tpidEntry);
            if(GT_OK != rc)
            {
                return rc;
            }
        }

        keyMask.etherType = 0xFFFF;
        keyData.etherType = tpid;

        keyMask.udbPairsArr[0].udb[0] = 0;
        keyMask.udbPairsArr[0].udb[1] = 0;

        keyData.udbPairsArr[0].udb[0] = 0;
        keyData.udbPairsArr[0].udb[1] = 0;

        switch(frontPanel_appDemoToCpssTrafficType[entryIndex].cpssType)
        {
            case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EDSA_EXTENDED_PORT_DTAGGED_TO_UPSTREAM_PORT_E:
                keyMask.udbPairsArr[0].udb[0] = 0xFF;
                keyMask.udbPairsArr[0].udb[1] = 0xFF;

                keyData.udbPairsArr[0].udb[0] = (GT_U8)(tpid>>8);
                keyData.udbPairsArr[0].udb[1] = (GT_U8)(tpid>>0);
                break;

            case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EDSA_EXTENDED_PORT_UNTAGGED_TO_UPSTREAM_PORT_E:
                keyMask.etherType = 0;
                keyData.etherType = 0;
                break;

            default:
                break;
        }

        packetType = frontPanel_appDemoToCpssTrafficType[entryIndex].appDemoType;
        rc = cpssPxIngressPacketTypeKeyEntrySet(devNum, packetType, &keyData, &keyMask);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssPxIngressPacketTypeKeyEntrySet", rc);
        if (rc != GT_OK)
            return rc;


        /* validate the entry to allow match ... otherwise we get DROP */
        rc = cpssPxIngressPacketTypeKeyEntryEnableSet(devNum, packetType, GT_TRUE);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssPxIngressPacketTypeKeyEntryEnableSet", rc);
        if (rc != GT_OK)
            return rc;
    }

    /*************************************************************/

    cpssOsMemSet(&packetTypeFormat, 0, sizeof(CPSS_PX_INGRESS_PORT_MAP_PACKET_TYPE_FORMAT_STC));

    /* state that 'front panel' can forward packet only to the uplink ports */
    tableType = CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E;
    entryIndex = PIPE_FRONT_PANEL_DESTINATION_BASE_INDEX;

    rc = cpssPxIngressPortMapEntrySet(devNum, tableType, entryIndex, uplinkPortsBmp, GT_FALSE);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssPxIngressPortMapEntrySet(Destination table)", rc);
    if (rc != GT_OK)
        return rc;

    packetTypeFormat.indexMax = 1;/* 0 bits */
    packetTypeFormat.indexConst = PIPE_FRONT_PANEL_DESTINATION_BASE_INDEX;

    for(entryIndex = 0 ;
        frontPanel_appDemoToCpssTrafficType[entryIndex].tpidIndex != GT_NA;
        entryIndex++)
    {
        packetType = frontPanel_appDemoToCpssTrafficType[entryIndex].appDemoType;
        rc = cpssPxIngressPortMapPacketTypeFormatEntrySet(devNum, tableType,packetType,&packetTypeFormat);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssPxIngressPortMapPacketTypeFormatEntrySet(Destination table)", rc);
        if (rc != GT_OK)
            return rc;
    }

    for(entryIndex = 0 ;
        frontPanel_appDemoToCpssTrafficType[entryIndex].tpidIndex != GT_NA;
        entryIndex++)
    {
        packetType = frontPanel_appDemoToCpssTrafficType[entryIndex].appDemoType;
        /* disable SRC filtering ! ... not needed */
        rc = cpssPxIngressFilteringEnableSet(devNum, CPSS_DIRECTION_EGRESS_E,
            packetType, GT_FALSE);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssPxIngressFilteringEnableSet", rc);
        if (rc != GT_OK)
            return rc;
    }

    /*********************************/
    /* set the egress PHA processing */
    /*********************************/

    /* the traffic ingress from the 'front panel' ports :
       state that this traffic that egress the uplink ports treated by the :
       CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EDSA_EXTENDED_PORT_DTAGGED_TO_UPSTREAM_PORT_E
       CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EDSA_EXTENDED_PORT_TAGGED_TO_UPSTREAM_PORT_E
       CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EDSA_EXTENDED_PORT_TAG1_TAGGED_TO_UPSTREAM_PORT_E
       CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EDSA_EXTENDED_PORT_UNTAGGED_TO_UPSTREAM_PORT_E
    */

    cpssOsMemSet(&operationInfo,0,sizeof(operationInfo));
    operationInfo.info_edsa_E2U.eDsaForward.srcTagged     = GT_FALSE;/* will be set in next logic */
    operationInfo.info_edsa_E2U.eDsaForward.hwSrcDev = 0;/* ignored any way by the DX device */
    operationInfo.info_edsa_E2U.eDsaForward.tpIdIndex = 0;/* TBD */
    operationInfo.info_edsa_E2U.eDsaForward.tag1SrcTagged = GT_FALSE;/* will be set in next logic */

    for(entryIndex = 0 ;
        frontPanel_appDemoToCpssTrafficType[entryIndex].tpidIndex != GT_NA;
        entryIndex++)
    {
        operationInfo.info_edsa_E2U.eDsaForward.tpIdIndex = frontPanel_appDemoToCpssTrafficType[entryIndex].tpidIndex;

        switch(frontPanel_appDemoToCpssTrafficType[entryIndex].cpssType)
        {
            case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EDSA_EXTENDED_PORT_DTAGGED_TO_UPSTREAM_PORT_E:
                operationInfo.info_edsa_E2U.eDsaForward.srcTagged     = GT_TRUE;  /*tag0*/
                operationInfo.info_edsa_E2U.eDsaForward.tag1SrcTagged = GT_TRUE;  /*tag1*/
                break;
            case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EDSA_EXTENDED_PORT_TAGGED_TO_UPSTREAM_PORT_E:
                operationInfo.info_edsa_E2U.eDsaForward.srcTagged     = GT_TRUE;  /*tag0*/
                operationInfo.info_edsa_E2U.eDsaForward.tag1SrcTagged = GT_FALSE; /*no tag1*/
                break;
            case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EDSA_EXTENDED_PORT_TAG1_TAGGED_TO_UPSTREAM_PORT_E:
                operationInfo.info_edsa_E2U.eDsaForward.srcTagged     = GT_FALSE; /*no tag0*/
                operationInfo.info_edsa_E2U.eDsaForward.tag1SrcTagged = GT_TRUE;  /*tag1*/
                break;
            case CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_EDSA_EXTENDED_PORT_UNTAGGED_TO_UPSTREAM_PORT_E:
            default:
                operationInfo.info_edsa_E2U.eDsaForward.srcTagged     = GT_FALSE; /*no tag0*/
                operationInfo.info_edsa_E2U.eDsaForward.tag1SrcTagged = GT_FALSE; /*no tag1*/
                break;
        }

        for (portNum = 12; portNum < 16; portNum++)
        {
            PORT_SKIP_CHECK(devNum,portNum);
            rc = cpssPxEgressHeaderAlterationEntrySet(devNum,
                portNum,
                frontPanel_appDemoToCpssTrafficType[entryIndex].appDemoType,/*packetType*/
                frontPanel_appDemoToCpssTrafficType[entryIndex].cpssType,   /*operationType*/
                &operationInfo);
            CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssPxEgressHeaderAlterationEntrySet", rc);
            if (rc != GT_OK)
                return rc;
        }
    }

    cpssOsMemSet(&srcPortInfo,0,sizeof(srcPortInfo));

    dsaPortNum = 0;
    /* state that traffic ingress from the 'front pannel' , need next src info :
       The source ePort number is configured for each Extended Port.
       It is used to assign the source port number into the eDSA tag */
    for (portNum = 0; portNum < 12; portNum++)
    {
        PORT_SKIP_CHECK(devNum,portNum);

        srcPortInfo.info_dsa.srcPortNum = dsaPortNum;

        rc = cpssPxEgressSourcePortEntrySet(devNum,
            portNum,
            CPSS_PX_EGRESS_SOURCE_PORT_ENTRY_TYPE_EDSA_E,
            &srcPortInfo
        );
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssPxEgressSourcePortEntrySet", rc);
        if (rc != GT_OK)
            return rc;

        /* the DXCH device not like 'holes' in the port numbers */
        dsaPortNum++;
    }

    return GT_OK;
}

/* get the 4 bits trg_port from the DSA tag bits 23:19 <Trg Phy Port[4:0]> */
static CPSS_PX_INGRESS_PACKET_BIT_FIELD_OFFSET_STC bitFieldArr[] = {
    /* byteOffset , startBit , numBits */
    {13           , 3        ,  4     }
};

/* set the ingress and PHA tables to forward packets from uplink to specific port
    in 'front panel' and removing the 'FROM_CPU' dsa (single word) */
static GT_STATUS pipeUplinksPortsSet
(
    IN GT_SW_DEV_NUM   devNum
)
{
    GT_STATUS   rc;
    CPSS_PX_INGRESS_PORT_PACKET_TYPE_KEY_STC    portKey;
    CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC  keyData;
    CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC  keyMask;
    CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_ENT     tableType;
    CPSS_PX_INGRESS_PORT_MAP_PACKET_TYPE_FORMAT_STC packetTypeFormat;
    GT_PHYSICAL_PORT_NUM  portNum;
    CPSS_PX_PORTS_BMP     trgPortsBmp;
    CPSS_PX_EGRESS_HEADER_ALTERATION_ENTRY_UNT  operationInfo;
    CPSS_PX_PACKET_TYPE     packetType;

    cpssOsMemSet(&portKey, 0, sizeof(CPSS_PX_INGRESS_PORT_PACKET_TYPE_KEY_STC));

    portKey.srcPortProfile = SRC_PORT_PROFILE_FRONT_UPLINK_E;
    portKey.portUdbPairArr[0].udbAnchorType = CPSS_PX_INGRESS_UDB_ANCHOR_TYPE_L2_E;
    portKey.portUdbPairArr[0].udbByteOffset = 12;/* start of the DSA */

    for (portNum = 12; portNum < 16; portNum++)
    {
        PORT_SKIP_CHECK(devNum,portNum);

        rc = cpssPxIngressPortPacketTypeKeySet(devNum, portNum, &portKey);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssPxIngressPortPacketTypeKeySet", rc);
        if (rc != GT_OK)
            return rc;

        /* state that the port is 'cascade port' that ingress with DSA tag */
        rc = cpssPxIngressPortDsaTagEnableSet(devNum,portNum,GT_TRUE);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssPxIngressPortDsaTagEnableSet", rc);
        if (rc != GT_OK)
            return rc;
    }

    /**************************************/
    /* define and enable the traffic type */
    /**************************************/

    /* define 'traffic type' will match the src port profile */
    cpssOsMemSet(&keyData, 0, sizeof(CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC));
    cpssOsMemSet(&keyMask, 0, sizeof(CPSS_PX_INGRESS_PACKET_TYPE_KEY_FORMAT_STC));
    keyMask.profileIndex = 0x7F;
    keyMask.udbPairsArr[0].udb[0] = 0xc0;/* the 2 bits of '31:30 Tag Command' */

    packetType = PIPE_INGRESS_TRAFFIC_TYPE_UPLINK_E;
    keyData.profileIndex = PIPE_INGRESS_TRAFFIC_TYPE_UPLINK_E;
    keyData.udbPairsArr[0].udb[0] = 0x40;/* the 2 bits of '31:30 Tag Command' = 'FROM_CPU' */

    rc = cpssPxIngressPacketTypeKeyEntrySet(devNum, packetType, &keyData, &keyMask);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssPxIngressPacketTypeKeyEntrySet", rc);
    if (rc != GT_OK)
        return rc;


    /* validate the entry to allow match ... otherwise we get DROP */
    rc = cpssPxIngressPacketTypeKeyEntryEnableSet(devNum, packetType, GT_TRUE);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssPxIngressPacketTypeKeyEntryEnableSet", rc);
    if (rc != GT_OK)
        return rc;

    /* if the traffic from the uplink will not be matched by the above 'PIPE_INGRESS_TRAFFIC_TYPE_UPLINK_E' */
    /* cause it to be dropped by the next index */
    packetType           = PIPE_INGRESS_TRAFFIC_TYPE_UPLINK_INVALID_E;
    keyData.profileIndex = BIT_0;

    keyMask.udbPairsArr[0].udb[0] = 0;/* dont care */
    keyData.udbPairsArr[0].udb[0] = 0;/* dont care */

    rc = cpssPxIngressPacketTypeKeyEntrySet(devNum, packetType, &keyData, &keyMask);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssPxIngressPacketTypeKeyEntrySet", rc);
    if (rc != GT_OK)
        return rc;

    /* invalidate the entry to cause DROP */
    rc = cpssPxIngressPacketTypeKeyEntryEnableSet(devNum, packetType, GT_FALSE);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssPxIngressPacketTypeKeyEntryEnableSet", rc);
    if (rc != GT_OK)
        return rc;

    /*************************************************************/
    /* state that 'uplink' can forward packet to any of the 'front panel' ports */
    tableType = CPSS_PX_INGRESS_PORT_MAP_TABLE_TYPE_DESTINATION_E;

    for(portNum = 0; portNum < 12 ; portNum++)
    {
        trgPortsBmp = 1 << portNum;

        rc = cpssPxIngressPortMapEntrySet(devNum, tableType, portNum + PIPE_UPLINK_DESTINATION_BASE_INDEX, trgPortsBmp, GT_FALSE);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssPxIngressPortMapEntrySet(Destination table)", rc);
        if (rc != GT_OK)
            return rc;
    }

    cpssOsMemSet(&packetTypeFormat, 0, sizeof(CPSS_PX_INGRESS_PORT_MAP_PACKET_TYPE_FORMAT_STC));
    packetTypeFormat.indexMax   = PIPE_UPLINK_DESTINATION_BASE_INDEX + 12;/*12 ports*/
    packetTypeFormat.indexConst = PIPE_UPLINK_DESTINATION_BASE_INDEX;
    packetTypeFormat.bitFieldArr[0] = bitFieldArr[0];

    packetType = PIPE_INGRESS_TRAFFIC_TYPE_UPLINK_E;
    rc = cpssPxIngressPortMapPacketTypeFormatEntrySet(devNum, tableType, packetType, &packetTypeFormat);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssPxIngressPortMapPacketTypeFormatEntrySet(Destination table)", rc);
    if (rc != GT_OK)
        return rc;

    /*************************************************************/
    {
        /* disable SRC filtering ! ... not needed */
        rc = cpssPxIngressFilteringEnableSet(devNum, CPSS_DIRECTION_EGRESS_E,
            packetType, GT_FALSE);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssPxIngressFilteringEnableSet", rc);
        if (rc != GT_OK)
            return rc;
    }

    /*********************************/
    /* set the egress PHA processing */
    /*********************************/

    /* the traffic ingress from the uplink ports :
       state that this traffic that egress the 'front panel' ports treated by the :
       CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_UPSTREAM_PORT_MIRRORING_TO_EXTENDED_PORT_E */

    cpssOsMemSet(&operationInfo,0,sizeof(operationInfo));
    for (portNum = 0; portNum < 12; portNum++)
    {
        PORT_SKIP_CHECK(devNum,portNum);
        rc = cpssPxEgressHeaderAlterationEntrySet(devNum,
            portNum,
            PIPE_INGRESS_TRAFFIC_TYPE_UPLINK_E,
            CPSS_PX_EGRESS_HEADER_ALTERATION_TYPE_DSA_UPSTREAM_PORT_MIRRORING_TO_EXTENDED_PORT_E,
            &operationInfo);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssPxEgressHeaderAlterationEntrySet", rc);
        if (rc != GT_OK)
            return rc;

    }

    return GT_OK;
}

/* set the ingress and PHA tables to forward packets from 'front panel' to 'uplink'
   + adding 'FRW' eDSA (4 words) with 'src port' info.
   and from uplink to specific port in 'front panel' and removing the 'FROM_CPU' dsa (single word) */
static GT_STATUS pipeDsaSystemSet
(
    IN GT_SW_DEV_NUM   devNum
)
{
    GT_STATUS   rc;
    GT_PHYSICAL_PORT_NUM portNum;
    CPSS_PX_PORTS_BMP   fullPortsBmp;
    GT_U32    entryIndex;

    /* set the ingress ports into 2 categories:
       1. 'front panel'
       2. 'uplink'
    */

    rc = pipeFrontPanelPortsSet(devNum);
    if(rc != GT_OK)
    {
        CPSS_ENABLER_DBG_TRACE_RC_MAC("pipeFrontPanelPortsSet", rc);
        return rc;
    }

    rc = pipeUplinksPortsSet(devNum);
    if(rc != GT_OK)
    {
        CPSS_ENABLER_DBG_TRACE_RC_MAC("pipeUplinksPortsSet", rc);
        return rc;
    }

    fullPortsBmp = uplinkPortsBmp | frontPannelPortsBmp;

    /* designated port : no trunk members */
    for(entryIndex = 0 ; entryIndex < 128 ; entryIndex++)
    {
        rc = cpssPxIngressHashDesignatedPortsEntrySet(devNum, entryIndex, fullPortsBmp);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssPxIngressHashDesignatedPortsEntrySet", rc);
        if (rc != GT_OK)
            return rc;
    }

    /* disable forwarding to all ports. Event handler will enable forwarding for
       connected ports with link UP. */
    for (portNum = 0; portNum < PRV_CPSS_PX_CPU_DMA_NUM_CNS; portNum++)
    {
        rc = cpssPxIngressPortTargetEnableSet(devNum, portNum, GT_FALSE);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssPxIngressPortTargetEnableSet", rc);
        if (rc != GT_OK)
            return rc;
    }

    return GT_OK;
}

/* do init PIPE configurations */
static GT_STATUS internal_initConnectionToPipe_px
(
    IN PIPE_REMOTE_DEV_INFO_STC *pipe_info_ptr
)
{
    GT_STATUS rc;
    GT_SW_DEV_NUM  devNum;
    GT_U8   devIndex;

    devNum = pipe_info_ptr->cpssDevNum;
    /* sanity check */
    devIndex = pipe_info_ptr->appDemoDevIndex;

    /* state the single device to work on */
    appDemoSysConfig.appDemoActiveDeviceBmp = 1 << devIndex;
    /* now indicate that the second device valid in the appDemo so we can
       create it in the cpss
       rest of 'pci scan' info was set during : phoenix_getBoardInfo()
    */
    appDemoPpConfigList[devIndex].valid = GT_TRUE;
    appDemoPpConfigList[devIndex].fdbNotUsed = GT_TRUE;/*The device not need to learn NA messages from the AC5X */
    /* allow the cpssInitSystem to use the '1' or '2' parameter for the PIPE board */
    userForceBoardType(GT_NA);

    if(GT_FALSE == appDemoPpConfigList[devIndex].valid)
    {
        rc = GT_NOT_FOUND;
        CPSS_ENABLER_DBG_TRACE_RC_MAC("PIPE device not found in AppDemo DB", rc);
        return rc;
    }

    /* skip calling to px_forwardingTablesStage(...)
       from gtDbPxPipeBoardReg_DeviceSimpleInit(...) */
    px_forwardingTablesStage_enabled = GT_FALSE;

    /* must update the 'to be used' devNum in the PIPE initialization. */
    appDemoPpConfigList[devIndex].devNum = CAST_SW_DEVNUM(devNum);

    rc = gtDbPxPipeBoardReg(PIPE_REMOTE_PHYSICAL_PORT_BOARD_ID, &pipe_genericBoardConfigFun);
    if(rc != GT_OK)
    {
        CPSS_ENABLER_DBG_TRACE_RC_MAC("gtDbPxPipeBoardReg", rc);
        return rc;
    }

    rc = pipe_genericBoardConfigFun.deviceSimpleInit(devIndex, PIPE_REMOTE_PHYSICAL_PORT_BOARD_ID);
    if(rc != GT_OK)
    {
        CPSS_ENABLER_DBG_TRACE_RC_MAC("deviceSimpleInit on PIPE", rc);
        return rc;
    }

    if(!PRV_CPSS_PX_A1_AND_ABOVE_CHECK_MAC(devNum))
    {
        /* some configurations not hold the A0 options.
           see use of PRV_CPSS_PX_A1_AND_ABOVE_CHECK_MAC() in file gtDbPxPipe.c
           that not copied to current file */
        return GT_NOT_SUPPORTED;
    }

    rc = pipeDsaSystemSet(devNum);
    if(rc != GT_OK)
    {
        CPSS_ENABLER_DBG_TRACE_RC_MAC("pipeDsaSystemSet", rc);
        return rc;
    }

    rc = appDemoDeviceEventHandlerPreInit(CAST_SW_DEVNUM(devNum));
    CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoDeviceEventHandlerPreInit", rc);
    if (rc != GT_OK)
    {
        return rc;
    }
    /* spawn the user event handler processes */
    rc = appDemoDeviceEventRequestDrvnModeInit(devIndex);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoDeviceEventRequestDrvnModeInit", rc);
    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/* based on : initConnectionTo88e1690(...) */
static GT_STATUS initConnectionToPipe
(
    IN GT_U8   devNum,
    IN PHOENIX_PIPE_CASCADE_PORTS_STC  *cascadePortsPtr
)
{
    GT_STATUS rc;

    rc = internal_initConnectionToPipe_dxch(devNum, cascadePortsPtr);
    if(rc != GT_OK)
    {
        CPSS_ENABLER_DBG_TRACE_RC_MAC("internal_initConnectionToPipe_dxch", rc);
        return rc;
    }

    return GT_OK;
}

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
GT_STATUS   appDemoAc5xWithPipe_fillAc5xWithPortMappingAndSpeed(void)
{
    GT_STATUS rc;

    /* extra/override info needed */
    rc = px_force_Ports_0_3_10G_12_13_25G();
    if(rc != GT_OK)
    {
        CPSS_ENABLER_DBG_TRACE_RC_MAC("px_force_Ports_0_3_10G_12_13_25G", rc);
        return rc;
    }

    return GT_OK;
}

/*******************************************************************************
* appDemoAc5xWithPipe_initAfterBasicAc5xInit
*
* DESCRIPTION:
*       The function is called after the AC5X did the basic
*       (including port mapping and speeds that was done during appDemoAc5xWithPipe_fillAc5xWithPortMappingAndSpeed(...))
*       The function :
*       1. sets the AC5X with cascade config and remote physical config .
*       2. sets the PIPE with cascade config and PVE to those cascade.
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
)
{
    GT_STATUS   rc;
    GT_U32      ii;
    GT_U8 devNum = appDemoPpConfigList[ac5x_devIndex].devNum;


    /* do the cascade ports before the remote ports */
    for(ii = 0; ii < NUM_CASCADE_PORTS; ii++)
    {
        /* based on : initConnectionTo88e1690(...) */
        rc = initConnectionToPipe(devNum, &cascadePorts[ii]);
        if(rc != GT_OK)
        {
            CPSS_ENABLER_DBG_TRACE_RC_MAC("initConnectionToPipe", rc);
            return rc;
        }
    }

    rc = internal_initConnectionToPipe_px(&pipe_Info);
    if(rc != GT_OK)
    {
        CPSS_ENABLER_DBG_TRACE_RC_MAC("internal_initConnectionToPipe_dxch", rc);
        return rc;
    }

    /* enable port isolation for the src-filter for trunks in stacking WA */
    rc = cpssDxChNstPortIsolationEnableSet(devNum, GT_TRUE);
    if(rc != GT_OK)
    {
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChNstPortIsolationEnableSet", rc);
        return rc;
    }


    /* state that port isolation work according to eport/trunkId from DSA/local
        and not according to src port/trunk on the local device */
    rc = cpssDxChNstPortIsolationOnEportsEnableSet(devNum, GT_TRUE);
    if(rc != GT_OK)
    {
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChNstPortIsolationEnableSet", rc);
        return rc;
    }


    /* disable the <OrigSrcPortFilter> due to wrong <Orig Src Phy Port Or Trunk ID> */
    /* this adds no limitation on the application/system !  */
    rc = cpssDxChCscdOrigSrcPortFilterEnableSet(devNum, GT_FALSE);
    if(rc != GT_OK)
    {
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChCscdOrigSrcPortFilterEnableSet", rc);
        return rc;
    }

    /* special link status event from PX hold impact on the DX device */
    appDemoCallBackOnLinkStatusChaneFunc = pipe_treatLinkStatusChange;

    return GT_OK;
}

