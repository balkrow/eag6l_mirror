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
* @file gtDbDxPhoenixWithCaelumExtended.c
*
* @brief Extended info for the AC5X + Caelum board.
*
* @version   1
*
*   See high level description doc in SharePoint : Ac5x_and_caelum_appDemo.docx
*
********************************************************************************
*/

#include <appDemo/boardConfig/appDemoBoardConfig.h>
#include <appDemo/boardConfig/gtDbDxBoardTypeConfig.h>
#include <appDemo/boardConfig/appDemoCfgMisc.h>
#include <appDemo/sysHwConfig/gtAppDemoSysConfigDefaults.h>
#include <appDemo/utils/mainUT/appDemoUtils.h>
#include <appDemo/sysHwConfig/appDemoDb.h>

#include <appDemo/boardConfig/gtDbDxBobcat2Mappings.h>
#include <appDemo/boardConfig/gtDbDxPhoenixWithCaelumExtended.h>

#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cpss/dxCh/dxChxGen/cscd/cpssDxChCscd.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgPrvEdgeVlan.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortManager.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgSecurityBreach.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgMc.h>

#define DEFAULT_VLANID  1
static CPSS_CSCD_PORT_TYPE_ENT DSA_SIZE_AC5X_TO_CAELUM = CPSS_CSCD_PORT_DSA_MODE_4_WORDS_E;
static CPSS_CSCD_PORT_TYPE_ENT DSA_SIZE_CAELUM_TO_AC5X = CPSS_CSCD_PORT_DSA_MODE_4_WORDS_E;

#define MAX_CAELUM_PORTS  30
#define MAX_AC5X_PORTS  50

/* useTmPorts options :
    0 - don't use
    1 - use with TM port number equal to MAC      port number
    2 - use with TM port number equal to physical port number (this mode asked by CPSS CV team)
*/
enum{
    TM_PORT_NOT_USED_E                      = 0,
    TM_PORT_NUM_EQUAL_MAC_NUM_E             = 1,
    TM_PORT_NUM_EQUAL_PHYSICAL_PORT_NUM_E   = 2,
};
#ifdef ASIC_SIMULATION
static GT_U32  useTmPorts = TM_PORT_NOT_USED_E;
#else /*ASIC_SIMULATION*/
static GT_U32  useTmPorts = TM_PORT_NOT_USED_E;/* modify default for HW to not use TM, till TM works */
#endif /*!ASIC_SIMULATION*/

static GT_BOOL  useInterruptPollingOnCaelum = GT_TRUE;
GT_STATUS appDemoAc5xWithCaelum_useInterruptPollingOnCaelum(IN GT_U32 usePolling)
{
    useInterruptPollingOnCaelum = usePolling ? GT_TRUE : GT_FALSE;
    return GT_OK;
}

/* (from CLI) debug option for the developer before doing autoInitSystem , to state use/not use TM */
/* useTm options :
    0 - don't use
    1 - use with TM port number equal to MAC      port number
    2 - use with TM port number equal to physical port number (this mode asked by CPSS CV team)
*/
GT_STATUS   appDemoAc5xWithCaelum_useTmPorts(IN GT_U32  useTm)
{
    useTmPorts = useTm;

    return GT_OK;
}

static CPSS_CSCD_PORT_TYPE_ENT  getDsaSize(IN GT_U32  numWords)
{
    return numWords == 1 ? CPSS_CSCD_PORT_DSA_MODE_1_WORD_E  :
           numWords == 2 ? CPSS_CSCD_PORT_DSA_MODE_2_WORDS_E :
           numWords == 4 ? CPSS_CSCD_PORT_DSA_MODE_4_WORDS_E :
          CPSS_CSCD_PORT_NETWORK_E;

}
/* (from CLI) debug option for the developer before doing autoInitSystem , to set DSA_SIZE_AC5X_TO_CAELUM */
GT_STATUS   appDemoAc5xWithCaelum_DSA_SIZE_AC5X_TO_CAELUM(IN GT_U32  numWords)
{
    CPSS_CSCD_PORT_TYPE_ENT tmp = getDsaSize(numWords);
    if(tmp == CPSS_CSCD_PORT_NETWORK_E)
    {
        osPrintf("only values : 1,2,4 supported \n");
        return GT_BAD_PARAM;
    }

    DSA_SIZE_AC5X_TO_CAELUM = tmp;

    return GT_OK;
}
/* (from CLI) debug option for the developer before doing autoInitSystem , to set DSA_SIZE_CAELUM_TO_AC5X */
GT_STATUS   appDemoAc5xWithCaelum_DSA_SIZE_CAELUM_TO_AC5X(IN GT_U32  numWords)
{
    CPSS_CSCD_PORT_TYPE_ENT tmp = getDsaSize(numWords);
    if(tmp == CPSS_CSCD_PORT_NETWORK_E)
    {
        osPrintf("only values : 1,2,4 supported \n");
        return GT_BAD_PARAM;
    }

    DSA_SIZE_CAELUM_TO_AC5X = tmp;

    return GT_OK;
}

static GT_STATUS ac5xTreatLinkStatusChange
(
    IN GT_U8     devNum,
    IN GT_PHYSICAL_PORT_NUM  portNum,
    IN GT_BOOL  linkUp
);


#ifdef PX_FAMILY
extern
#endif
APP_DEMO_CALL_BACK_ON_LINK_STATUS_CHANGE_FUNC appDemoCallBackOnLinkStatusChaneFunc;
static GT_U8 ac5x_devNum = 0;
static GT_U8 caelum_devNum = 0;

/* TPID profile-Id to hold no VLAN tag recognition (empty profile)*/
static GT_U32   caelum_tpid_empty_profile = 7;

static PortInitList_STC portInitlist_caelumPorts[MAX_CAELUM_PORTS];
    /* filled in runtime from ac5xCaelumBoardInfo*/

static CPSS_DXCH_PORT_MAP_STC caelumPorts[MAX_CAELUM_PORTS];
    /* filled in runtime from ac5xCaelumBoardInfo*/
static GT_U32   numOf_caelumPorts = 0;

static APP_DEMO_STR_STC     caelumPorts_notes[MAX_CAELUM_PORTS+1];
    /* filled in runtime from ac5xCaelumBoardInfo*/

#define NUM_CASCADE_PORTS  4

static GT_U32  ac5x_cascadePorts[NUM_CASCADE_PORTS+1];/* physical port numbers of the cascade ports */
    /* filled in runtime from ac5xCaelumBoardInfo */

/* the remote ports of caelum set in this bmp , at runtime */
static GT_U32   caelumRemotePortsBmp[4/*support 128 ports*/]={0,0,0,0};

typedef struct{
    GT_U32  portNum;    /*physical number*/
    GT_U32  macNum;     /*mac number*/
    GT_U32  tmPort;     /*TM port number*/
    CPSS_PORT_SPEED_ENT          speed;
    CPSS_PORT_INTERFACE_MODE_ENT mode;
    GT_CHAR*    notes;
}PORT_MAPPING_AND_SPEED_STC;

typedef struct{
    GT_U32                      cascadeIndex;/* index in caelumCascadePorts_speedMode */
    PORT_MAPPING_AND_SPEED_STC  portInfo;
}PVE_INFO_STC;
#define SAME_AS_MAC 0xFFFF

#define PRE_INIT_NUM_ENTRIES_IN_AC5X_ARR  4
AC5X_PORT_MAP_STC ac5x_with_rpp_for_caelum_port_mode[MAX_AC5X_PORTS] =
{
    /*  1G/2.5G/10G/25G */
     {{0,   8, 16, 24,  GT_NA}    , { 0,   8, 16, 24, GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E}
     /* QSGMII ports */
    ,{{40, 41, 42, 43, GT_NA}     , { 40, 41, 42, 43, GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E}

    /* CPU SDMA ports :*/
    ,{{CPU_PORT,/*63*/          GT_NA} , {54,/*DMA - bound to MG0*/GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    }
     /* map second 'CPU SDMA port' */
    ,{{60,                      GT_NA} , {55,/*DMA - bound to MG1*/GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E    }

    /* the rest filled in runtime from ac5xCaelumBoardInfo*/
};
/* value set in runtime with ac5x_with_rpp_for_caelum_port_mode */
GT_U32  actualNum_ac5x_with_rpp_for_caelum_port_mode = PRE_INIT_NUM_ENTRIES_IN_AC5X_ARR;

#define PRE_INIT_NUM_ENTRIES_IN_AC5X_PORT_MODES_ARR  2
PortInitList_STC ac5x_with_rpp_for_caelum_portInitlist[MAX_AC5X_PORTS] =
{
    /*  ports 0,   8, 16, 24 : 1G/2.5G/10G/25G */
     { PORT_LIST_TYPE_LIST,  {0,   8, 16, 24, APP_INV_PORT_CNS },  CPSS_PORT_SPEED_10000_E,   CPSS_PORT_INTERFACE_MODE_KR_E  }

    /*  ports 40, 41, 42, 43 : 1G QSGMII */
    ,{ PORT_LIST_TYPE_LIST,  {40, 41, 42, 43, APP_INV_PORT_CNS },  CPSS_PORT_SPEED_1000_E,   CPSS_PORT_INTERFACE_MODE_QSGMII_E  }

    /* the rest filled in runtime from ac5xCaelumBoardInfo*/
};

static PVE_INFO_STC caelumPvePorts[] = {
    /*cscd*//*port*//*mac*/ /*TM port*/     /* speed */                 /*mode*/                        /*notes*/
     {0,    {64  ,   64  ,   SAME_AS_MAC  , CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_KR_E    , "slot A0(Remote:117)"}}
    ,{1,    {44  ,   44  ,   SAME_AS_MAC  , CPSS_PORT_SPEED_2500_E,   CPSS_PORT_INTERFACE_MODE_SGMII_E , "slot A0(Remote: 97)"}}

    ,{0,    {59  ,   59  ,   SAME_AS_MAC  , CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_KR_E    , "slot A1(Remote:112)"}}
    ,{1,    {32  ,   32  ,   SAME_AS_MAC  , CPSS_PORT_SPEED_2500_E,   CPSS_PORT_INTERFACE_MODE_SGMII_E , "slot A1(Remote: 85)"}}
    ,{1,    {36  ,   36  ,   SAME_AS_MAC  , CPSS_PORT_SPEED_2500_E,   CPSS_PORT_INTERFACE_MODE_SGMII_E , "slot A1(Remote: 89)"}}
    ,{1,    {40  ,   40  ,   SAME_AS_MAC  , CPSS_PORT_SPEED_2500_E,   CPSS_PORT_INTERFACE_MODE_SGMII_E , "slot A1(Remote: 93)"}}

    ,{0,    {57  ,   57  ,   SAME_AS_MAC  , CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_KR_E    , "slot A2(Remote:110)"}}
    ,{2,    {28  ,   28  ,   SAME_AS_MAC  , CPSS_PORT_SPEED_2500_E,   CPSS_PORT_INTERFACE_MODE_SGMII_E , "slot A2(Remote: 81)"}}

    ,{0,    {58  ,   58  ,   SAME_AS_MAC  , CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_KR_E    , "slot A3(Remote:111)"}}
    ,{2,    {24  ,   24  ,   SAME_AS_MAC  , CPSS_PORT_SPEED_2500_E,   CPSS_PORT_INTERFACE_MODE_SGMII_E , "slot A3(Remote: 77)"}}

    ,{2,    {20  ,   20  ,   SAME_AS_MAC  , CPSS_PORT_SPEED_2500_E,   CPSS_PORT_INTERFACE_MODE_SGMII_E , "slot A4(Remote: 73)"}}

    ,{2,    {56  ,   56  ,   SAME_AS_MAC  , CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_KR_E    , "slot A5(Remote:109)"}}
    ,{2,    {16  ,   16  ,   SAME_AS_MAC  , CPSS_PORT_SPEED_2500_E,   CPSS_PORT_INTERFACE_MODE_SGMII_E , "slot A5(Remote: 69)"}}

    ,{3,    { 8  ,    8  ,   SAME_AS_MAC  , CPSS_PORT_SPEED_2500_E,   CPSS_PORT_INTERFACE_MODE_SGMII_E , "slot A6(Remote: 61)"}}
    ,{3,    {12  ,   12  ,   SAME_AS_MAC  , CPSS_PORT_SPEED_2500_E,   CPSS_PORT_INTERFACE_MODE_SGMII_E , "slot A6(Remote: 65)"}}

    ,{3,    { 0  ,    0  ,   SAME_AS_MAC  , CPSS_PORT_SPEED_2500_E,   CPSS_PORT_INTERFACE_MODE_SGMII_E , "slot A7(Remote: 53)"}}
    ,{3,    { 4  ,    4  ,   SAME_AS_MAC  , CPSS_PORT_SPEED_2500_E,   CPSS_PORT_INTERFACE_MODE_SGMII_E , "slot A7(Remote: 57)"}}

    /* must be last */
    ,{GT_NA, {GT_NA,0,0,0,0,NULL}}
};

static PORT_MAPPING_AND_SPEED_STC   caelumCascadePorts_speedMode[] =
{
    /*port*//*mac*/ /*TM port*/     /* speed */                 /*mode*/                        /*notes*/
      {68  ,   68  ,   GT_NA  , CPSS_PORT_SPEED_40000_E,   CPSS_PORT_INTERFACE_MODE_KR4_E , "cascade : hold : 57,58,59,64"}

     ,{67  ,   67  ,   GT_NA  , CPSS_PORT_SPEED_10000_E,   CPSS_PORT_INTERFACE_MODE_KR_E  , "cascade : hold : 32,36,40,44"}
     ,{66  ,   66  ,   GT_NA  , CPSS_PORT_SPEED_10000_E,   CPSS_PORT_INTERFACE_MODE_KR_E  , "cascade : hold : 16,20,24,28"}
     ,{65  ,   65  ,   GT_NA  , CPSS_PORT_SPEED_10000_E,   CPSS_PORT_INTERFACE_MODE_KR_E  , "cascade : hold :  0, 4, 8,12"}

    /* must be last */
    ,{GT_NA,0,0,0,0,"no more"}
};

static PORT_MAPPING_AND_SPEED_STC   ac5xCascadePorts_speedMode[] =
{
    /*port*//*mac*/ /*TM port*/     /* speed */                 /*mode*/                   /*notes*/
      {50  ,   50  ,   GT_NA  , CPSS_PORT_SPEED_40000_E,   CPSS_PORT_INTERFACE_MODE_KR4_E , NULL}

     ,{32  ,   32  ,   GT_NA  , CPSS_PORT_SPEED_10000_E,   CPSS_PORT_INTERFACE_MODE_KR_E  , NULL}
     ,{48  ,   48  ,   GT_NA  , CPSS_PORT_SPEED_10000_E,   CPSS_PORT_INTERFACE_MODE_KR_E  , NULL}
     ,{49  ,   49  ,   GT_NA  , CPSS_PORT_SPEED_10000_E,   CPSS_PORT_INTERFACE_MODE_KR_E  , NULL}

    /* must be last */
    ,{GT_NA,0,0,0,0,NULL}
};


typedef struct{
    PORT_MAPPING_AND_SPEED_STC  *cascade_portInfoPtr;/* pointer to info of single cascade port */

    /* the ports that do PVE to this cascade port */
    PVE_INFO_STC    *pveInfoArr;

}CASCADE_INFO_STC;

static CASCADE_INFO_STC   caelumCascadeInfo[4] =
{
    {
         caelumCascadePorts_speedMode /*cascade_portInfoPtr -- pointer to info of single cascade port */
        ,caelumPvePorts
    }
    ,
    {
         caelumCascadePorts_speedMode /*cascade_portInfoPtr -- pointer to info of single cascade port */
        ,caelumPvePorts
    }
    ,
    {
         caelumCascadePorts_speedMode /*cascade_portInfoPtr -- pointer to info of single cascade port */
        ,caelumPvePorts
    }
    ,
    {
         caelumCascadePorts_speedMode /*cascade_portInfoPtr -- pointer to info of single cascade port */
        ,caelumPvePorts
    }
};


static CASCADE_INFO_STC   ac5xCascadeInfo[4] =
{
    {
         ac5xCascadePorts_speedMode /*cascade_portInfoPtr -- pointer to info of single cascade port */
        ,NULL
    }
    ,
    {
         ac5xCascadePorts_speedMode /*cascade_portInfoPtr -- pointer to info of single cascade port */
        ,NULL
    }
    ,
    {
         ac5xCascadePorts_speedMode /*cascade_portInfoPtr -- pointer to info of single cascade port */
        ,NULL
    }
    ,
    {
         ac5xCascadePorts_speedMode /*cascade_portInfoPtr -- pointer to info of single cascade port */
        ,NULL
    }
};
/* set in runtime during ac5xInitRemotePorts(...)
   used by appDemoAc5xWithCaelum_treatLinkStatusChange(...) */
static GT_U32   caelumPortToAc5xPortMap[128] = {0,0};
#define REMOTE_PHYSICAL_PORT_BASE_CASCADE_0     53
#define REMOTE_PHYSICAL_PORT_BASE_CASCADE_1     53
#define REMOTE_PHYSICAL_PORT_BASE_CASCADE_2     53
#define REMOTE_PHYSICAL_PORT_BASE_CASCADE_3     53

typedef struct{
    CASCADE_INFO_STC    *devAPtr;/* The info in AC5X   Device */
    CASCADE_INFO_STC    *devBPtr;/* The info in Caelum Device */
    GT_U32              remotePhysicalPortBase;/* the offset per cascade port for the
        mapping from DSA tag info to remote physical port */
}CASCADE_B2B_INFO_STC;

typedef struct{
    CASCADE_B2B_INFO_STC    cascade[NUM_CASCADE_PORTS];
}BOARD_CONNECTIONS;

static BOARD_CONNECTIONS    ac5xCaelumBoardInfo =
{                                                 /*remotePhysicalPortBase*/
     {{&ac5xCascadeInfo[0],&caelumCascadeInfo[0],REMOTE_PHYSICAL_PORT_BASE_CASCADE_0}
     ,{&ac5xCascadeInfo[1],&caelumCascadeInfo[1],REMOTE_PHYSICAL_PORT_BASE_CASCADE_1}
     ,{&ac5xCascadeInfo[2],&caelumCascadeInfo[2],REMOTE_PHYSICAL_PORT_BASE_CASCADE_2}
     ,{&ac5xCascadeInfo[3],&caelumCascadeInfo[3],REMOTE_PHYSICAL_PORT_BASE_CASCADE_3}
     }
};


static boardRevId2PortsInitList_STC  portInitlist_caelumPortsInitList[]=
{
    /* clock, revid,  port list */
     {  167,   1,  CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E,     0x3,  &portInitlist_caelumPorts[0]}
    ,{  200,   1,  CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E,     0x3,  &portInitlist_caelumPorts[0]}
    ,{  250,   1,  CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E,     0x3,  &portInitlist_caelumPorts[0]}
    ,{  365,   1,  CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E,     0x3,  &portInitlist_caelumPorts[0]}
    ,{  167,   2,  CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E,     0x3,  &portInitlist_caelumPorts[0]}
    ,{  200,   2,  CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E,     0x3,  &portInitlist_caelumPorts[0]}
    ,{  250,   2,  CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E,     0x3,  &portInitlist_caelumPorts[0]}
    ,{  365,   2,  CPSS_PP_SUB_FAMILY_BOBCAT2_BOBK_E,     0x3,  &portInitlist_caelumPorts[0]}
    ,{    0,   0,        CPSS_BAD_SUB_FAMILY,             0x0,  NULL}   /* default port init */
};

static GT_STATUS appDemoAc5xWithCaelum_treatLinkStatusChange
(
    IN GT_U8     devNum,
    IN GT_PHYSICAL_PORT_NUM  portNum,
    IN GT_BOOL  linkUp,
    IN APP_DEMO_CALL_BACK_ON_LINK_STATUS_CHANGE_ENT partIndex,
    OUT GT_BOOL     *stopLogicPtr
);

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
GT_STATUS   appDemoAc5xWithCaelum_fillAc5xWithPortMappingAndSpeed(void)
{
    GT_U32  ii,jj;
    CASCADE_B2B_INFO_STC    *b2bPtr;
    PORT_MAPPING_AND_SPEED_STC  *cascade_portInfoPtr;
    AC5X_PORT_MAP_STC        *currAc5xPortMappingPtr;
    PortInitList_STC            *currAc5xPortModesPtr;
    CASCADE_INFO_STC    *devAPtr;
    CASCADE_INFO_STC    *devBPtr;
    PVE_INFO_STC        *pveInfoArr;
    PORT_MAPPING_AND_SPEED_STC  *portInfoPtr;
    GT_U32  remotePhysicalPortBase;

    /* state the system should not allow traffic until the 2 devices finished proper init !!! */
    appDemoDbEntryAdd("appDemoDxTrafficEnable = disable", 1);
    /* state the system should not allow events until the 2 devices finished proper init !!! */
    appDemoDbEntryAdd("appDemoEventRequestDrvnModeInit = disable" ,1);


    if(actualNum_ac5x_with_rpp_for_caelum_port_mode != PRE_INIT_NUM_ENTRIES_IN_AC5X_ARR)
    {
        /* already done init*/
        return GT_OK;
    }

    /* special link status event from Caelum hold impact on the AC5X device */
    appDemoCallBackOnLinkStatusChaneFunc = appDemoAc5xWithCaelum_treatLinkStatusChange;

    currAc5xPortMappingPtr = &ac5x_with_rpp_for_caelum_port_mode[PRE_INIT_NUM_ENTRIES_IN_AC5X_ARR];
    currAc5xPortModesPtr   = &ac5x_with_rpp_for_caelum_portInitlist[PRE_INIT_NUM_ENTRIES_IN_AC5X_PORT_MODES_ARR];

    /* do the cascade ports before the remote ports */
    for(ii = 0 ; ii < NUM_CASCADE_PORTS ; ii++)
    {
        b2bPtr = &ac5xCaelumBoardInfo.cascade[ii];

        /******************************************/
        /* operate on the info from the AC5X side */
        /******************************************/
        devAPtr = b2bPtr->devAPtr;
        cascade_portInfoPtr = &devAPtr->cascade_portInfoPtr[ii];

        /* cascade port mapping */
        currAc5xPortMappingPtr->portNumberArr[0] = cascade_portInfoPtr->portNum;
        currAc5xPortMappingPtr->portNumberArr[1] = GT_NA;

        currAc5xPortMappingPtr->globalDmaNumber[0] = cascade_portInfoPtr->macNum;
        currAc5xPortMappingPtr->globalDmaNumber[1] = GT_NA;

        currAc5xPortMappingPtr->mappingType = CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E;

        /* cascade mode and speed */
        currAc5xPortModesPtr->entryType     = PORT_LIST_TYPE_LIST;
        currAc5xPortModesPtr->portList[0]   = cascade_portInfoPtr->portNum;
        currAc5xPortModesPtr->portList[1]   = GT_NA;
        currAc5xPortModesPtr->speed         = cascade_portInfoPtr->speed;
        currAc5xPortModesPtr->interfaceMode = cascade_portInfoPtr->mode;

        currAc5xPortMappingPtr++;
        actualNum_ac5x_with_rpp_for_caelum_port_mode++;
        currAc5xPortModesPtr++;
    }


    for(ii = 0 ; ii < NUM_CASCADE_PORTS ; ii++)
    {
        b2bPtr = &ac5xCaelumBoardInfo.cascade[ii];

        /******************************************/
        /* operate on the info from the AC5X side */
        /******************************************/
        devAPtr = b2bPtr->devAPtr;
        cascade_portInfoPtr = &devAPtr->cascade_portInfoPtr[ii];
        /********************************************/
        /* operate on the info from the Caelum side */
        /********************************************/
        devBPtr = b2bPtr->devBPtr;
        pveInfoArr = devBPtr->pveInfoArr;

        /* we need to define the remove ports mapping */
        for(jj = 0 ; pveInfoArr[jj].cascadeIndex != GT_NA; jj++)
        {
            if(pveInfoArr[jj].cascadeIndex != ii/*cascade index*/)
            {
                /* this port is not bound to current cascade */
                continue;
            }
            portInfoPtr = &pveInfoArr[jj].portInfo;
            remotePhysicalPortBase = ac5xCaelumBoardInfo.cascade[ii].remotePhysicalPortBase;

            /* remote port mapping */
            currAc5xPortMappingPtr->portNumberArr[0] = portInfoPtr->portNum + remotePhysicalPortBase;/* from the remote device (Caelum) */
            currAc5xPortMappingPtr->portNumberArr[1] = GT_NA;

            currAc5xPortMappingPtr->globalDmaNumber[0] = cascade_portInfoPtr->macNum;/* from the local AC5X */
            currAc5xPortMappingPtr->globalDmaNumber[1] = GT_NA;

            currAc5xPortMappingPtr->mappingType = CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E;

            /* remote port mode and speed */
            currAc5xPortModesPtr->entryType     = PORT_LIST_TYPE_LIST;
            currAc5xPortModesPtr->portList[0]   = portInfoPtr->portNum + remotePhysicalPortBase;
            currAc5xPortModesPtr->portList[1]   = GT_NA;
            currAc5xPortModesPtr->speed         = CPSS_PORT_SPEED_REMOTE_E;
            currAc5xPortModesPtr->interfaceMode = CPSS_PORT_INTERFACE_MODE_REMOTE_E;

            currAc5xPortMappingPtr++;
            actualNum_ac5x_with_rpp_for_caelum_port_mode++;
            currAc5xPortModesPtr++;
        }

        ac5x_cascadePorts[ii]                                         = cascade_portInfoPtr->portNum;
    }
    /* terminate the array */
    currAc5xPortModesPtr->entryType = PORT_LIST_TYPE_EMPTY;
    /* terminate the array */
    ac5x_cascadePorts[ii] = GT_NA;

    return GT_OK;
}

/*******************************************************************************
* fillCaelumWithPortMappingAndSpeed
*
* DESCRIPTION:
*       The function is called before doing cpssInitSystem for the Caelum.
*       called from : caelumBefore_cpssInitSystem()
*       The function sets arrays needed for : port mapping and speeds for the Caelum :
*       The function fill port mapping and speed in :
*       static CPSS_DXCH_PORT_MAP_STC   caelumPorts[];
*       static GT_U32                   numOf_caelumPorts;
*       static PortInitList_STC         portInitlist_caelumPorts[];
*       static APP_DEMO_STR_STC         caelumPorts_notes;
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
static GT_STATUS   fillCaelumWithPortMappingAndSpeed(void)
{
    GT_U32  ii,jj;
    CASCADE_B2B_INFO_STC    *b2bPtr;
    PORT_MAPPING_AND_SPEED_STC  *cascade_portInfoPtr;
    CPSS_DXCH_PORT_MAP_STC      *currCaelumPortMappingPtr;
    PortInitList_STC            *currCaelumPortModesPtr;
    CASCADE_INFO_STC    *devBPtr;
    PVE_INFO_STC        *pveInfoArr;
    PORT_MAPPING_AND_SPEED_STC  *portInfoPtr;

    if(numOf_caelumPorts != 0)
    {
        /* already done init*/
        return GT_OK;
    }

    currCaelumPortMappingPtr = &caelumPorts[0];
    currCaelumPortModesPtr   = &portInitlist_caelumPorts[0];

    /* no need to loop as all the ports are those of caelumPvePorts that given to
       all 4 cascade lists */
    ii = 0;
    /*for(ii = 0 ; ii < NUM_CASCADE_PORTS ; ii++)*/
    {
        b2bPtr = &ac5xCaelumBoardInfo.cascade[ii];

        /********************************************/
        /* operate on the info from the Caelum side */
        /********************************************/
        devBPtr = b2bPtr->devBPtr;
        pveInfoArr = devBPtr->pveInfoArr;

        /* we need to define the remove ports mapping */
        for(jj = 0 ; pveInfoArr[jj].cascadeIndex != GT_NA; jj++)
        {
            portInfoPtr = &pveInfoArr[jj].portInfo;

            caelumPorts_notes[numOf_caelumPorts].str     = portInfoPtr->notes;

            caelumRemotePortsBmp[portInfoPtr->portNum >> 5] |= 1 << (portInfoPtr->portNum & 0x1f);

            /* remote port mapping */
            currCaelumPortMappingPtr->physicalPortNumber = portInfoPtr->portNum;/* from pve port */
            currCaelumPortMappingPtr->mappingType = CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E;
            currCaelumPortMappingPtr->portGroup = 0;
            currCaelumPortMappingPtr->interfaceNum  = portInfoPtr->macNum;/* from pve port */
            if(useTmPorts == TM_PORT_NUM_EQUAL_MAC_NUM_E)
            {
                currCaelumPortMappingPtr->txqPortNumber = GT_NA;
                currCaelumPortMappingPtr->tmEnable      = GT_TRUE;
                currCaelumPortMappingPtr->tmPortInd     = portInfoPtr->macNum;/* from pve port */
            }
            else
            if(useTmPorts == TM_PORT_NUM_EQUAL_PHYSICAL_PORT_NUM_E)
            {
                currCaelumPortMappingPtr->txqPortNumber = GT_NA;
                currCaelumPortMappingPtr->tmEnable      = GT_TRUE;
                currCaelumPortMappingPtr->tmPortInd     = portInfoPtr->portNum;
            }
            else
            {
                /* The WM not hold Traffic manager logic , so need to use regular TXQ */
                currCaelumPortMappingPtr->txqPortNumber = portInfoPtr->macNum;/* from pve port */
                currCaelumPortMappingPtr->tmEnable      = GT_FALSE;
                currCaelumPortMappingPtr->tmPortInd     = GT_NA;
            }
            currCaelumPortMappingPtr->reservePreemptiveChannel = GT_FALSE;

            /* remote port mode and speed */
            currCaelumPortModesPtr->entryType     = PORT_LIST_TYPE_LIST;
            currCaelumPortModesPtr->portList[0]   = portInfoPtr->portNum;
            currCaelumPortModesPtr->portList[1]   = GT_NA;
            currCaelumPortModesPtr->speed         = portInfoPtr->speed;
            currCaelumPortModesPtr->interfaceMode = portInfoPtr->mode;

            currCaelumPortMappingPtr++;
            numOf_caelumPorts++;
            currCaelumPortModesPtr++;
        }
    }

    /* do cascade ports after the 'remote ports' */
    for(ii = 0 ; ii < NUM_CASCADE_PORTS ; ii++)
    {
        b2bPtr = &ac5xCaelumBoardInfo.cascade[ii];

        /********************************************/
        /* operate on the info from the Caelum side */
        /********************************************/
        devBPtr = b2bPtr->devBPtr;
        cascade_portInfoPtr = &devBPtr->cascade_portInfoPtr[ii];

        /* cascade port mapping */
        caelumPorts_notes[numOf_caelumPorts].str     = cascade_portInfoPtr->notes;

        currCaelumPortMappingPtr->physicalPortNumber = cascade_portInfoPtr->portNum;/* from cascade port */
        currCaelumPortMappingPtr->mappingType = CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E;
        currCaelumPortMappingPtr->portGroup = 0;
        currCaelumPortMappingPtr->interfaceNum  = cascade_portInfoPtr->macNum;/* from cascade port */
        currCaelumPortMappingPtr->txqPortNumber = cascade_portInfoPtr->macNum;/* from cascade port */
        currCaelumPortMappingPtr->tmEnable      = GT_FALSE;
        currCaelumPortMappingPtr->tmPortInd     = GT_NA;
        currCaelumPortMappingPtr->reservePreemptiveChannel = GT_FALSE;


        /* cascade mode and speed */
        currCaelumPortModesPtr->entryType     = PORT_LIST_TYPE_LIST;
        currCaelumPortModesPtr->portList[0]   = cascade_portInfoPtr->portNum;
        currCaelumPortModesPtr->portList[1]   = GT_NA;
        currCaelumPortModesPtr->speed         = cascade_portInfoPtr->speed;
        currCaelumPortModesPtr->interfaceMode = cascade_portInfoPtr->mode;

        currCaelumPortMappingPtr++;
        numOf_caelumPorts++;
        currCaelumPortModesPtr++;
    }

    /* allow sending direct traffic from CPU to Caelum (debug purposes) */
    currCaelumPortMappingPtr->physicalPortNumber = CPU_PORT;
    currCaelumPortMappingPtr->mappingType = CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E;
    currCaelumPortMappingPtr->portGroup = 0;
    currCaelumPortMappingPtr->interfaceNum  = GT_NA;
    currCaelumPortMappingPtr->txqPortNumber = CPU_PORT;
    currCaelumPortMappingPtr->tmEnable      = GT_FALSE;
    currCaelumPortMappingPtr->tmPortInd     = GT_NA;
    currCaelumPortMappingPtr->reservePreemptiveChannel = GT_FALSE;

    caelumPorts_notes[numOf_caelumPorts].str = "---";

    numOf_caelumPorts++;

    /* terminate the array */
    currCaelumPortModesPtr->entryType = PORT_LIST_TYPE_EMPTY;
    caelumPorts_notes[numOf_caelumPorts].str = "no more";

    return GT_OK;
}

/*******************************************************************************
* caelumBefore_cpssInitSystem
*
* DESCRIPTION:
*       called before cpssInitSystem 29,2,0
*       pre-Caelum init , bind of:
*       appDemoPortMapPtr,appDemoPortMapSize to caelum ports (port mapping)
*       appDemo_boardRevId2PortsInitListPtr , appDemo_boardRevId2PortsSize  to caelum ports (speed,mode)
*
*       NOTE: the device was NOT created yet in the CPSS
*
* INPUTS:
*       caelum_devIndex  - device index for the Caelum (index in appDemoPpConfigList[] DB)
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
static GT_STATUS caelumBefore_cpssInitSystem(IN GT_U32 caelum_devIndex)
{
    GT_STATUS   rc;

    /*  call to fill info in :
    *       static CPSS_DXCH_PORT_MAP_STC   caelumPorts[];
    *       static GT_U32                   numOf_caelumPorts;
    *       static PortInitList_STC         portInitlist_caelumPorts[];
    *       static APP_DEMO_STR_STC         caelumPorts_notes;
    */
    rc = fillCaelumWithPortMappingAndSpeed();
    CPSS_ENABLER_DBG_TRACE_RC_MAC("fillCaelumWithPortMappingAndSpeed", rc);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* bind the info , with variables that used during cpssInitSystem 29,2
       of the Caelum device */
    appDemoPortMapPtr = caelumPorts;
    appDemoPortMapSize = numOf_caelumPorts;

    ARR_PTR_AND_SIZE_MAC(portInitlist_caelumPortsInitList , appDemo_boardRevId2PortsInitListPtr , appDemo_boardRevId2PortsSize);

    appDemoPpConfigList[caelum_devIndex].portsNotesArray = caelumPorts_notes;


    return GT_OK;
}

/*******************************************************************************
* caelumPveToCascade
*
* DESCRIPTION:
*       init the on the Caelum device after calling cpssInitSystem 29,2,0
*       per remote port:
*           1. set PVE on the caelum ports toward the cascade
*           2. set pvid = 0 so untagged packets will be sent to AC5X with vlanId=0 in the eDSA
*              so the pvid will be configured only on the AC5X
*       per cascade port:
*           1. 'port type' = 'cascade' of 1 or 4 words
*           2. set 10K+16 as MRU , to not limit traffic on cascade
*
* INPUTS:
*       devNum  - device number of the Caelum
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
/*   */
static GT_STATUS caelumPveToCascade(IN GT_U8 devNum)
{
    GT_STATUS   rc;
    GT_U32  ii,jj;
    CASCADE_B2B_INFO_STC    *b2bPtr;
    PORT_MAPPING_AND_SPEED_STC  *cascade_portInfoPtr;
    CASCADE_INFO_STC    *devBPtr;
    PVE_INFO_STC        *pveInfoArr;
    PORT_MAPPING_AND_SPEED_STC  *portInfoPtr;
    GT_HW_DEV_NUM   hwDevNum;

    hwDevNum = PRV_CPSS_HW_DEV_NUM_MAC(devNum);

    /* NOTE: due to :
        GT_TRUE == appDemoPpConfigList[devIdx].fdbNotUsed
        the cpssInitSystem for the Caelum , not enabled NA on the default vlan
        and did not enabled NA on the eports
        but the auto learning is disabled on the ports and default vlan
    */

    /* loop on cascade ports and their PVE ports */
    for(ii = 0 ; ii < NUM_CASCADE_PORTS ; ii++)
    {
        b2bPtr = &ac5xCaelumBoardInfo.cascade[ii];

        /******************************************/
        /* operate on the info from the Caelum side */
        /******************************************/
        devBPtr = b2bPtr->devBPtr;
        cascade_portInfoPtr = &devBPtr->cascade_portInfoPtr[ii];
        pveInfoArr = devBPtr->pveInfoArr;

        for(jj = 0 ; pveInfoArr[jj].cascadeIndex != GT_NA; jj++)
        {
            portInfoPtr = &pveInfoArr[jj].portInfo;

            if(pveInfoArr[jj].cascadeIndex != ii)
            {
                /* this port is not bound to current cascade */
                continue;
            }
            /*set PVE from the back plane to the cascade ports */
            rc = cpssDxChBrgPrvEdgeVlanPortEnable(devNum,portInfoPtr->portNum,GT_TRUE,
                cascade_portInfoPtr->portNum,
                hwDevNum,GT_FALSE/*not trunk*/);
            if(rc != GT_OK)
            {
                return rc;
            }

            /* set pvid = 0 so untagged packets will be sent to AC5X with vlanId=0 in the eDSA
               so the pvid will be configured only on the AC5X */
            rc = cpssDxChBrgVlanPortVidSet(devNum,portInfoPtr->portNum,CPSS_DIRECTION_INGRESS_E,0);
            if(rc != GT_OK)
            {
                return rc;
            }
        }

        /* set the cascade port to use 4 words DSA on egress packets */
        /* NOTE: only 5 bits (0..31) needed from 'srcPort/trgPort' (could use single word DSA) */
        rc = cpssDxChCscdPortTypeSet(devNum, cascade_portInfoPtr->portNum ,
            CPSS_PORT_DIRECTION_BOTH_E , DSA_SIZE_CAELUM_TO_AC5X);
        if(rc != GT_OK)
        {
            return rc;
        }

        /* set 10K+16 as MRU , to not limit traffic on cascade */
        rc = cpssDxChPortMruSet(devNum, cascade_portInfoPtr->portNum , _10K + 16);
        if(rc != GT_OK)
        {
            return rc;
        }


        if(DSA_SIZE_AC5X_TO_CAELUM != CPSS_CSCD_PORT_DSA_MODE_1_WORD_E)
        {
            /* we don't want the cascade port traffic to recognize vlan tags ,
               so those will not be removed when the packet egress the ports
               (as all vlans not hold any vlan member , and all are 'untagged')

               see this section in the 'Caelum' per cascade port configurations.
            */
            rc = cpssDxChBrgVlanPortIngressTpidProfileSet(devNum, cascade_portInfoPtr->portNum,
                CPSS_VLAN_ETHERTYPE0_E, GT_TRUE, caelum_tpid_empty_profile);
            if(rc != GT_OK)
            {
                return rc;
            }

            rc = cpssDxChBrgVlanPortIngressTpidProfileSet(devNum, cascade_portInfoPtr->portNum,
                CPSS_VLAN_ETHERTYPE1_E, GT_TRUE, caelum_tpid_empty_profile);
            if(rc != GT_OK)
            {
                return rc;
            }
        }


    }

    return GT_OK;
}

/*******************************************************************************
* caelumAfter_cpssInitSystem
*
* DESCRIPTION:
*       init the on the Caelum device after calling cpssInitSystem 29,2,0
*       set the device:
*           1. set 'Invalid Vlan' command to be forward (instead of drop)
*           2. disable the egress filter (for 4 words DSA)
*
*       per remote port:
*           1. set PVE on the caelum ports toward the cascade
*           2. set pvid = 0 so untagged packets will be sent to AC5X with vlanId=0 in the eDSA
*              so the pvid will be configured only on the AC5X
*           3. invalidate vlan 1 , that was created by the 'unaware' init of appDemo for the Caelum.
*       per cascade port:
*           1. 'port type' = 'cascade' of 1 or 4 words
*           2. set 10K+16 as MRU , to not limit traffic on cascade
*
* INPUTS:
*       devNum  - device number of the Caelum
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
static GT_STATUS caelumAfter_cpssInitSystem(IN GT_U8 devNum)
{
    GT_STATUS   rc;

    /* remove the drop of 'Invalid Vlan' */
    rc = cpssDxChBrgSecurBreachEventPacketCommandSet(devNum,
        CPSS_BRG_SECUR_BREACH_EVENTS_INVALID_VLAN_E,
        CPSS_PACKET_CMD_FORWARD_E);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(DSA_SIZE_AC5X_TO_CAELUM != CPSS_CSCD_PORT_DSA_MODE_1_WORD_E)
    {
        /* we not want to define all the vlans and members in the Caelum */
        /* so we disable the egress filter */
        rc = cpssDxChBrgVlanEgressFilteringEnable(devNum,GT_FALSE);
        if(rc != GT_OK)
        {
            return rc;
        }


        /* we don't want the cascade port traffic to recognize vlan tags ,
           so those will not be removed when the packet egress the ports
           (as all vlans not hold any vlan member , and all are 'untagged')

           see this section in the 'Caelum' per cascade port configurations.
        */
        rc = cpssDxChBrgVlanIngressTpidProfileSet(devNum,caelum_tpid_empty_profile,CPSS_VLAN_ETHERTYPE0_E,0);
        if(rc != GT_OK)
        {
            return rc;
        }
        rc = cpssDxChBrgVlanIngressTpidProfileSet(devNum,caelum_tpid_empty_profile,CPSS_VLAN_ETHERTYPE1_E,0);
        if(rc != GT_OK)
        {
            return rc;
        }

    }
    else
    {
        /* we send 'from_cpu' from the AC5X , so no egress filter anyway in the Caelum */
    }

    /* the appDemo created Vlan 1 on the Caelum device */
    /* although the device will not use this vlan (or any Vlan) ,
       let's remove just to 'show' , (make sure) the device not use it */
    /* so invalidate vlan 1 */
    rc = cpssDxChBrgVlanEntryInvalidate(devNum,DEFAULT_VLANID);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* do per cascade port and per remote port configuration */
    rc = caelumPveToCascade(devNum);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("caelumPveToCascade", rc);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* restore to NULL after the cpssInitSystem(29,2,0) done */
    appDemoPortMapPtr  = NULL;
    appDemoPortMapSize = 0;
    appDemo_boardRevId2PortsInitListPtr = NULL;
    appDemo_boardRevId2PortsSize = 0;

    return GT_OK;
}

/*******************************************************************************
* ac5xInitRemotePorts_perPort
*
* DESCRIPTION:
*       The function init the remote physical port settings on the AC5X device.
*
*       1. state that the remote port egress packets with eDSA (4 words)
*       2. Enable mapping of target physical port to a remote physical port that resides over
*          a DSA-tagged interface
*       3. set info what the egress DSA should hold for target port , target device
*       4. force the 'localDevSrcPort' to do 'briging' , because by default the bridge
*           is bypassed for 'eDSA' packets
*
* INPUTS:
*       devNum          - AC5X device Num
*       cascadePortNum  - the cascade port
*       ac5x_physicalPortNumber - the physical port number as called in the AC5X
*       caelum_physicalPortNumber - the physical port number as called in the Caelum
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success,
*       GT_FAIL - otherwise.
*
* COMMENTS:
*  based on : internal_initConnectionToPipe_dxch_remote_port(...)
*
*******************************************************************************/
static GT_STATUS ac5xInitRemotePorts_perPort(
    IN GT_U8   devNum,
    IN GT_U32  cascadePortNum,
    IN GT_U32  ac5x_physicalPortNumber,
    IN GT_U32  caelum_physicalPortNumber
)
{
    GT_STATUS rc;
    GT_U32  portNum = ac5x_physicalPortNumber;
    GT_U32  dsa_HwDevNum , dsa_PortNum = caelum_physicalPortNumber;

    cascadePortNum = cascadePortNum;

    dsa_HwDevNum = PRV_CPSS_HW_DEV_NUM_MAC(caelum_devNum);/* get HwDevNum from devNum of Caelum */

    /* state that the port egress packets with eDSA (4 words) */
    rc = cpssDxChCscdPortTypeSet(devNum, portNum ,
        CPSS_PORT_DIRECTION_TX_E , DSA_SIZE_AC5X_TO_CAELUM);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(DSA_SIZE_AC5X_TO_CAELUM == CPSS_CSCD_PORT_DSA_MODE_1_WORD_E)
    {
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
    }
    else
    {
        /* not using 'from cpu' single word DSA , as we need 4 words eDSA to come to the Caelum , for QoS purpose */
        /* but we need the eDSa to be in addition to the vlan tags , so the
           Caelum will only pop the eDSA and can egress the packet as 'un-tagged'
           on all vlans on all ports.
        */
        rc = cpssDxChCscdPortForceNewForwardOrFromCpuDsaEnableSet(devNum, portNum, GT_TRUE);
        if(rc!=GT_OK)
        {
            return rc;
        }
    }

    /* Enable mapping of target physical port to a remote physical port that resides over
       a DSA-tagged interface */
    rc = cpssDxChCscdCentralizedChassisRemotePhysicalPortMapEnableSet (devNum, portNum,GT_TRUE);
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
    rc = cpssDxChCscdPortBridgeBypassEnableSet(devNum,portNum,GT_FALSE);
    if(rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/*******************************************************************************
* ac5xInRemotePhysicalPortSystem
*
* DESCRIPTION:
*       The function init the cascade port settings on the AC5X device.
*       1. cascade port to not get floods (unreg uc,bc,mc)
*       2. remove cascade port from vlan 1
*       3. the cascade port used only for ingress , to parse the incoming DSA
*       4. set 'remote physical port' mapping on ingress : to get info from the DSA from Caelum
*       5. set base for the cascade port (as input to the function)
*       6. enable setting the dsa mapping to use 'local SRC device' of the AC5X
*       7. set 10K+16 as MRU , to not limit traffic on cascade
*
* INPUTS:
*       devNum          - AC5X device Num
*       cascadePortNum  - the cascade port
*       remotePhysicalPortBase - the remote ports base for this cascade
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success,
*       GT_FAIL - otherwise.
*
* COMMENTS:
*  based on : internal_initConnectionToPipe_dxch(...)
*
*******************************************************************************/
static GT_STATUS ac5xInitCascadeToCaelum(
    IN GT_U8   devNum,
    IN GT_U32   cascadePortNum,
    IN GT_U32   remotePhysicalPortBase
)
{
    GT_STATUS rc;
    GT_U32  physicalPortBase = remotePhysicalPortBase; /* mapping between ports in Caelum and those of AC5X */
    GT_BOOL linkUp;

    /***********************/
    /* per cascade port    */
    /***********************/
    /* to limit errors by application that added it to vlan */
    /* disable 'flooding' of unknown unicast */
    rc = cpssDxChBrgPortEgrFltUnkEnable(devNum,cascadePortNum,GT_TRUE);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* disable 'flooding' of unregistered multicast */
    rc = cpssDxChBrgPortEgrFltUregMcastEnable(devNum,cascadePortNum,GT_TRUE);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* disable 'flooding' of unregistered broadcast */
    rc = cpssDxChBrgPortEgrFltUregBcEnable(devNum,cascadePortNum,GT_TRUE);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* the cascade ports should not be part of the Vlan , as there should not be
       'flooding' to them .
       the 'flooding' is done to remote ports that need to be in the vlan.

       so as the AppDemo already added those ports to the vlan 1 , we can remove them.

       NOTE: we also set above as 'disable' flooding reasons to the cascade ports :
       cpssDxChBrgPortEgrFltUnkEnable
       cpssDxChBrgPortEgrFltUregMcastEnable
       cpssDxChBrgPortEgrFltUregBcEnable
    */
    rc = cpssDxChBrgVlanPortDelete(devNum,DEFAULT_VLANID,cascadePortNum);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* the cascade port used only for ingress , to parse the incoming DSA , so the size
       of DSA is according to actual ingress packet (not from this config)
       and for egress the DSA tag size taken per 'remote port' and not from the cascade.
    */
    rc = cpssDxChCscdPortTypeSet(devNum, cascadePortNum ,
        CPSS_PORT_DIRECTION_RX_E , CPSS_CSCD_PORT_DSA_MODE_1_WORD_E/*not relevant to 'rx' direction*/);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* set 'remote physical port' mapping on ingress : to get info from the DSA from Caelum */
    rc = cpssDxChCscdCentralizedChassisMyPhysicalPortAssignmentModeSet(devNum, cascadePortNum ,
         CPSS_DXCH_CSCD_INGRESS_PHYSICAL_PORT_ASSIGNMENT_MODE_DSA_E);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* set base for the cascade port */
    rc = cpssDxChCscdCentralizedChassisMyPhysicalPortBaseSet(devNum, cascadePortNum ,
        physicalPortBase);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* enable setting the dsa mapping to use 'local SRC device' of the AC5X */
    rc = cpssDxChCscdCentralizedChassisMyPhysicalPortAssignSrcDevEnableSet(devNum,
        cascadePortNum, GT_TRUE);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* set 10K+16 as MRU , to not limit traffic on cascade */
    rc = cpssDxChPortMruSet(devNum, cascadePortNum , _10K + 16);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(portMgr)
    {
        rc = cpssDxChPortLinkStatusGet(devNum, cascadePortNum, &linkUp);
        if(GT_TRUE == linkUp)
        {
            /* do catch-up in case that the link up happened before the ports of the Caelum defined */
            /* see logic of : */
                    /* The port not exists (yet) in the CPSS (too early) */
                    /* we will trigger 'catch-up' at the end of the init */
            /* in  ac5xTreatLinkStatusChange */
            rc = ac5xTreatLinkStatusChange(devNum,cascadePortNum,linkUp);
            if(rc != GT_OK)
            {
                return rc;
            }
        }
    }

    return GT_OK;
}

/*******************************************************************************
* ac5xInitRemotePorts
*
* DESCRIPTION:
*       The function init the remote physical port settings on the AC5X device.
*
*       see per remote port in : ac5xInitRemotePorts_perPort
*
* INPUTS:
*       devNum          - AC5X device Num
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
*******************************************************************************/
static GT_STATUS ac5xInitRemotePorts(IN GT_U8 devNum)
{
    GT_STATUS   rc;
    GT_U32  ii,jj;
    CASCADE_B2B_INFO_STC    *b2bPtr;
    PORT_MAPPING_AND_SPEED_STC  *cascade_portInfoPtr;
    CASCADE_INFO_STC    *devAPtr;
    CASCADE_INFO_STC    *devBPtr;
    PVE_INFO_STC        *pveInfoArr;
    PORT_MAPPING_AND_SPEED_STC  *portInfoPtr;
    GT_U32  ac5x_physicalPortNumber,caelum_physicalPortNumber;
    GT_U32  remotePhysicalPortBase;

    for(ii = 0 ; ii < NUM_CASCADE_PORTS ; ii++)
    {
        b2bPtr = &ac5xCaelumBoardInfo.cascade[ii];

        /******************************************/
        /* operate on the info from the Caelum side */
        /******************************************/
        devAPtr = b2bPtr->devAPtr;
        devBPtr = b2bPtr->devBPtr;
        cascade_portInfoPtr = &devAPtr->cascade_portInfoPtr[ii];
        pveInfoArr = devBPtr->pveInfoArr;
        remotePhysicalPortBase = b2bPtr->remotePhysicalPortBase;

        for(jj = 0 ; pveInfoArr[jj].cascadeIndex != GT_NA; jj++)
        {
            portInfoPtr = &pveInfoArr[jj].portInfo;

            if(pveInfoArr[jj].cascadeIndex != ii)
            {
                /* this port is not bound to current cascade */
                continue;
            }

            /* mapping between the 'remote port' on the AC5X and the port number on the Caelum */
            ac5x_physicalPortNumber   = portInfoPtr->portNum + remotePhysicalPortBase;
            caelum_physicalPortNumber = portInfoPtr->portNum;

            if(caelum_physicalPortNumber >= 128)
            {
                rc = GT_OUT_OF_RANGE;
                CPSS_ENABLER_DBG_TRACE_RC_MAC("caelum_physicalPortNumber >= 128", rc);
                return rc;
            }

            if(ac5x_physicalPortNumber >= 128)
            {
                rc = GT_OUT_OF_RANGE;
                CPSS_ENABLER_DBG_TRACE_RC_MAC("ac5x_physicalPortNumber >= 128", rc);
                return rc;
            }

            /* save the mapping , for appDemoAc5xWithCaelum_treatLinkStatusChange(...)*/
            caelumPortToAc5xPortMap[caelum_physicalPortNumber] = ac5x_physicalPortNumber;

            rc = ac5xInitRemotePorts_perPort(devNum,
                    cascade_portInfoPtr->portNum,/* the cascade port */
                    ac5x_physicalPortNumber,
                    caelum_physicalPortNumber);
            if(rc != GT_OK)
            {
                CPSS_ENABLER_DBG_TRACE_RC_MAC("ac5xInitRemotePorts_perPort", rc);
                return rc;
            }
        }
    }

    return GT_OK;
}

/*******************************************************************************
* ac5xInRemotePhysicalPortSystem
*
* DESCRIPTION:
*       The function init the remote physical port settings on the AC5X device.
*       and on the cascade ports
*       1. Centralized Chassis to use 5 bits 'src  port' from the DSA , 0 bits from hwDevNum
*
*       per cascade port :
*           1. cascade port to not get floods (unreg uc,bc,mc)
*           2. remove cascade port from vlan 1
*           3. the cascade port used only for ingress , to parse the incoming DSA
*           4. set 'remote physical port' mapping on ingress : to get info from the DSA from Caelum
*           5. set base for the cascade port
*           6. enable setting the dsa mapping to use 'local SRC device' of the AC5X
*           7. set 10K+16 as MRU , to not limit traffic on cascade
*
*       per remote physical port :
*           1. state that the remote port egress packets with eDSA (4 words)
*           2. Enable mapping of target physical port to a remote physical port that resides over
*              a DSA-tagged interface
*           3. set info what the egress DSA should hold for target port , target device
*           4. force the 'localDevSrcPort' to do 'briging' , because by default the bridge
*               is bypassed for 'eDSA' packets
*
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
static GT_STATUS ac5xInRemotePhysicalPortSystem(IN GT_U8 devNum/*ac5x_devNum*/)
{
    GT_STATUS   rc;
    GT_U32      ii;
    GT_U32  srcDevLsbAmount = 0 , srcPortLsbAmount = 7;/* support all 128 values */

    /***********************/
    /* global - per device */
    /***********************/
    /* Centralized Chassis to use 6 bits 'src  port' from the DSA , 0 bits from hwDevNum */
    rc = cpssDxChCscdCentralizedChassisMyPhysicalPortMappingDsaSrcLsbAmountSet(
        devNum, srcDevLsbAmount,srcPortLsbAmount);
    if(rc != GT_OK)
    {
        return rc;
    }

    /********************************/
    /* do per cascade port settings */
    /********************************/
    for(ii = 0 ; ac5x_cascadePorts[ii] != GT_NA ; ii++)
    {
        /* based on : initConnectionTo88e1690(...) */
        rc = ac5xInitCascadeToCaelum(devNum,ac5x_cascadePorts[ii],
            ac5xCaelumBoardInfo.cascade[ii].remotePhysicalPortBase);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("ac5xInitCascadeToCaelum", rc);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    /****************************************/
    /* do per remote physical port settings */
    /****************************************/
    rc = ac5xInitRemotePorts(devNum);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("ac5xInitRemotePorts", rc);

    return rc;
}

/*******************************************************************************
* ac5xTreatLinkStatusChange
*
* DESCRIPTION:
*       The function do 'extra' config in response to link change event for
*       the AC5X cascade port connects to Caelum
*
*       The function is for AC5X device
*       The function do nothing for non-cascade port
*       The function logic is:
*           if the cascade port goes DOWN , take down the remote ports to it
*           if the cascade port goes UP   , take UP only the remote ports that have link UP on the Caelum
*
* INPUTS:
*       devNum  - the device number of the AC5X
*       portNum - physical port number
*       linkUp  - indication that link is UP/DOWN
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
static GT_STATUS ac5xTreatLinkStatusChange
(
    IN GT_U8     devNum,
    IN GT_PHYSICAL_PORT_NUM  portNum,
    IN GT_BOOL  linkUp
)
{
    GT_STATUS   rc;
    GT_U32  ii,jj;
    CASCADE_B2B_INFO_STC    *b2bPtr;
    CASCADE_INFO_STC    *devBPtr;
    PVE_INFO_STC        *pveInfoArr;
    PORT_MAPPING_AND_SPEED_STC  *portInfoPtr;
    GT_BOOL isCascade = GT_FALSE;
    GT_BOOL caelum_linkUp;
    CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_ENT  remotePortLinkState;

    for(ii = 0 ; ac5x_cascadePorts[ii] != GT_NA ; ii++)
    {
        if(portNum == ac5x_cascadePorts[ii])
        {
            isCascade = GT_TRUE;
            break;
        }
    }

    if(isCascade == GT_FALSE)
    {
        return GT_OK;
    }

    /* if the cascade port goes DOWN , take down the remote ports to it */
    /* if the cascade port goes UP   , take UP only the remote ports that have link UP on the Caelum */

    {
        b2bPtr = &ac5xCaelumBoardInfo.cascade[ii];

        /******************************************/
        /* operate on the info from the Caelum side */
        /******************************************/
        devBPtr = b2bPtr->devBPtr;
        pveInfoArr = devBPtr->pveInfoArr;

        for(jj = 0 ; pveInfoArr[jj].cascadeIndex != GT_NA; jj++)
        {
            portInfoPtr = &pveInfoArr[jj].portInfo;

            if(pveInfoArr[jj].cascadeIndex != ii)
            {
                /* this port is not bound to current cascade */
                continue;
            }

            if(linkUp == GT_FALSE)
            {
                /* if the cascade port goes DOWN , take down the remote ports to it */
                remotePortLinkState = CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_FORCE_LINK_DOWN_E;
            }
            else
            {
                /* if the cascade port goes UP   , take UP only the remote ports that have link UP on the Caelum */
                rc = cpssDxChPortLinkStatusGet(caelum_devNum, portInfoPtr->portNum, &caelum_linkUp);
                if (rc != GT_OK)
                {
                    /* The port not exists (yet) in the CPSS (too early) */
                    /* we will trigger 'catch-up' at the end of the init */
                    remotePortLinkState = CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_FORCE_LINK_DOWN_E;
                }

                if(caelum_linkUp == GT_TRUE)
                {
                    remotePortLinkState = CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_FORCE_LINK_UP_E;
                }
                else
                {
                    remotePortLinkState = CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_FORCE_LINK_DOWN_E;
                }
            }

            rc = cpssDxChBrgEgrFltPortLinkEnableSet(devNum, portInfoPtr->portNum,
                    remotePortLinkState);
            if(rc != GT_OK)
            {
                CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChBrgEgrFltPortLinkEnableSet", rc);
                return rc;
            }

        }
    }

    return GT_OK;
}


/*******************************************************************************
* appDemoAc5xWithCaelum_treatLinkStatusChange
*
* DESCRIPTION:
*       function called for 'link status change' (a callback function)
*       the 2 devices need to do extra config depend on the port that changed link.
*       for AC5X see : ac5xTreatLinkStatusChange
*       The Caelum device :
*       1. if the port is not one of the 'remotes' --> do nothing.
*       2. need to notify the AC5X device that the MAC of the port hold link status change.
*       based on : pipe_treatLinkStatusChange
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
static GT_STATUS appDemoAc5xWithCaelum_treatLinkStatusChange
(
    IN GT_U8     devNum,
    IN GT_PHYSICAL_PORT_NUM  portNum,
    IN GT_BOOL  linkUp,
    IN APP_DEMO_CALL_BACK_ON_LINK_STATUS_CHANGE_ENT partIndex,
    OUT GT_BOOL     *stopLogicPtr
)
{
    GT_STATUS   rc;
    GT_U32                  ac5xPortNum;
    CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_ENT portLinkStatusState;
    CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_ENT old_portLinkStatusState;
    CPSS_PORT_MANAGER_STC portEventStc;

    *stopLogicPtr = GT_FALSE;

    if(partIndex == APP_DEMO_CALL_BACK_ON_LINK_STATUS_CHANGE_BEFORE_MAIN_LOGIC_E)
    {
        return GT_OK;
    }

    if(caelum_devNum == 0)
    {
        /* not initialized yet , so ignored , for now */
        return GT_OK;
    }

    if(ac5x_devNum == devNum)
    {
        /* call the AC5X to treat it's ports */
        return ac5xTreatLinkStatusChange(devNum,portNum,linkUp);
    }

    /* caelumRemotePortsBmp array boundary check */
    if (portNum >= 128)
    {
        return GT_FAIL;
    }

    if(0 == (caelumRemotePortsBmp[portNum >> 5] & (1 << (portNum & 0x1f))))
    {
        /* this port of Caelum is not 'remote' on the AC5X (like the Caelum cascade ports) */
        return GT_OK;
    }

    /* convert the Caelum port numbers to remote port in the AC5X */
    ac5xPortNum = caelumPortToAc5xPortMap[portNum];

    if(portMgr)
    {
        portEventStc.portEvent = linkUp ? CPSS_PORT_MANAGER_EVENT_ENABLE_E : CPSS_PORT_MANAGER_EVENT_DISABLE_E;

        /* the port manager will set the EGF link filer to up/down */
        rc = cpssDxChPortManagerEventSet(ac5x_devNum, ac5xPortNum, &portEventStc);
        if(rc != GT_OK)
        {
            osPrintf("calling PortManagerEventSet event %d returned=%d, portNum=%d \n", portEventStc.portEvent, rc, ac5xPortNum);
            return rc;
        }
    }
    else
    {
        /* the remote physical port number on the DX that represents the PIPE port */

        rc = cpssDxChBrgEgrFltPortLinkEnableGet(ac5x_devNum, ac5xPortNum, &old_portLinkStatusState);
        if(rc != GT_OK)
        {
            return rc;
        }

        portLinkStatusState =  linkUp ?
            CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_FORCE_LINK_UP_E :
            CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_FORCE_LINK_DOWN_E;

        if(old_portLinkStatusState == portLinkStatusState)
        {
            /* avoid 'printings' of ports when no change in value */
            return GT_OK;
        }

        /* the port on the Caelum changed status , we need to notify the relevant DX device */
        rc = cpssDxChBrgEgrFltPortLinkEnableSet(ac5x_devNum, ac5xPortNum, portLinkStatusState);
        if(rc != GT_OK)
        {
            return rc;
        }

        if(appDemoPrintLinkChangeFlagGet())
        {
            cpssOsPrintSync("LINK %s : devNum[%ld] (remote physical port[%2ld]) \n",
                                (linkUp == GT_FALSE) ? "DOWN" : "UP  ",
                                            (GT_U32)ac5x_devNum, ac5xPortNum);
        }
    }

    return GT_OK;
}

/*******************************************************************************
* appDemoAc5xWithCaelum_SystemReady
*
* DESCRIPTION:
*       state the system now ready to allow :
*       1. traffic.
*       2. events.
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
static GT_STATUS appDemoAc5xWithCaelum_SystemReady(void)
{
    GT_STATUS rc;
    CPSS_SYSTEM_RECOVERY_INFO_STC   system_recovery;

    rc = cpssSystemRecoveryStateGet(&system_recovery);
    if (rc != GT_OK)
    {
      return rc;
    }

    /*
      skip appDemoAc5xWithCaelum_SystemReady during system recovery.
      needs to be call later by appDemoAc5xWithCaelum_HaPostCompletion after recovery is done
    */
    if (CPSS_SYSTEM_RECOVERY_PROCESS_NOT_ACTIVE_E != system_recovery.systemRecoveryProcess &&
        CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E != system_recovery.systemRecoveryState)
    {
        return GT_OK;
    }


    /* state the system now ready to allow traffic */
    rc = appDemoDbEntryAdd("appDemoDxTrafficEnable = disable", 0);
    if(rc != GT_OK)
    {
        CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoDbEntryAdd : appDemoDxTrafficEnable = disable --> 0", rc);
    }

    rc = appDemoDbEntryAdd("appDemoEventRequestDrvnModeInit = disable", 0);
    if(rc != GT_OK)
    {
        CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoDbEntryAdd : appDemoEventRequestDrvnModeInit = disable --> 0", rc);
    }

    rc = appDemoEventRequestDrvnModeInit();
    CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoEventRequestDrvnModeInit ", rc);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* should be done after appDemoEventRequestDrvnModeInit() */
    rc = appDemoDataIntegrityInit(ac5x_devNum);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoDataIntegrityInit ", rc);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(useInterruptPollingOnCaelum == GT_TRUE)
    {
        /* set normal polling time .
        0 means default of 20 milisec */
        DRV_EMULATE_INTERRUPTS_pollingSleepTime_Set(0);
    }

    /* allow interrupts / appDemo tasks to stable */
    osTimerWkAfter(500);

    rc = appDemoDxTrafficEnable(ac5x_devNum);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoDxTrafficEnable ac5x ", rc);
    if(rc != GT_OK)
    {
        return rc;
    }
    rc = appDemoDxTrafficEnable(caelum_devNum);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoDxTrafficEnable caelum", rc);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* only the AC5X need to calc hash properly */
    rc = appDemoTrunkCrcHashDefaultsSet(ac5x_devNum);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoTrunkCrcHashDefaultsSet ac5x", rc);
    if(rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

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
)
{
    GT_STATUS   rc;
    GT_U32  caelum_devIndex;

    ac5x_devNum = appDemoPpConfigList[ac5x_devIndex].devNum;

    caelum_devNum   = ac5x_devNum   + 1;
    caelum_devIndex = ac5x_devIndex + 1;

    /* state the single device to work on */
    appDemoSysConfig.appDemoActiveDeviceBmp = 1 << caelum_devIndex ;
    appDemoDbEntryAdd("appDemoActiveDeviceBmp", appDemoSysConfig.appDemoActiveDeviceBmp);

    /* now indicate that the second device valid in the appDemo so we can
       create it in the cpss
       rest of 'pci scan' info was set during : phoenix_getBoardInfo()
    */
    appDemoPpConfigList[caelum_devIndex].valid = GT_TRUE;
    appDemoPpConfigList[caelum_devIndex].fdbNotUsed = GT_TRUE;/*The device not need to learn NA messages from the AC5X */
    /* allow the cpssInitSystem to use the '1' or '2' parameter for the Caelum board */
    userForceBoardType(GT_NA);

    /* bind port mapping and speed,modes to Caelum device , to be used during cpssInitSystem(29,1/2) */
    rc = caelumBefore_cpssInitSystem(caelum_devIndex);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("caelumBefore_cpssInitSystem", rc);
    if(rc != GT_OK)
    {
        osPrintf("ERROR : initializing the Caelum (as first device) failed\n");
        return rc;
    }

    /* needed on linux that calls : extDrvBoardIdGet from bobkBoardTypeGet()
        so to avoid it we need to give it type in advance
    */
    appDemoDbEntryAdd("bc2BoardType", APP_DEMO_CAELUM_BOARD_DB_CNS);

    /*
        needed in order not to save the corrent cpssInitParams in storedBoardIdx,storedBoardRevId and storedMultiProcessApp ,
        orignal value allready set
    */
    appDemoDbEntryAdd("secondCpssInitConfig", 1);
    /* lpm DB was init for sip 6 by AC5x and won't support caelum device */
    appDemoDbEntryAdd("appDemoSkipLpmDeviceAdd", 1);

    if(useInterruptPollingOnCaelum == GT_TRUE)
    {
        /* we not get interrupts on HW from the Caelum */
        /* so hint it to use 'polling mode' from task in the appDemo (not in no_KM extDrv) */
        appDemoDbEntryAdd("hwInfo.irq.switching", CPSS_PP_INTERRUPT_VECTOR_USE_POLLING_CNS);
        /* set very slow polling time , to not impact the init time of Caelum */
        /* it will be set back to normal just after events will be initialized
           it have great impact on WM
        */
        DRV_EMULATE_INTERRUPTS_pollingSleepTime_Set(500);
    }

    /* do not allow the cpssInitSystem of the Caelum to set the 'systemInitialized = GT_TRUE' */
    /* this will be done only by the caller of appDemoAc5xWithCaelum_initAfterBasicAc5xInit() */
    appDemoDbEntryAdd("systemInitialized = not allow", 1);

    if(useTmPorts != TM_PORT_NOT_USED_E)
    {
        rc = cpssInitSystem(29,2,0);/* version 2 is with 'TM enabled' = GT_TRUE */
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssInitSystem 29,2,0", rc);
    }
    else
    {
        rc = cpssInitSystem(29,1,0);/* version 1 is with 'TM enabled' = GT_FALSE */
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssInitSystem 29,1,0", rc);
    }

    appDemoSysConfig.appDemoActiveDeviceBmp = 0x0;/* reset the indications */
    appDemoDbEntryAdd("appDemoActiveDeviceBmp", appDemoSysConfig.appDemoActiveDeviceBmp);

    appDemoDbEntryAdd("systemInitialized = not allow", 0); /* not needed any more */


    if(rc != GT_OK)
    {
        osPrintf("ERROR : initializing the Caelum (as first device) failed\n");
        return rc;
    }

    /* init the remote physical port settings on the AC5X device */
    rc = ac5xInRemotePhysicalPortSystem(ac5x_devNum);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("ac5xInRemotePhysicalPortSystem", rc);
    if (rc != GT_OK)
        return rc;

    /* init the Caelum device about the cascade ports and the remote ports */
    rc = caelumAfter_cpssInitSystem(caelum_devNum);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("caelumAfter_cpssInitSystem", rc);
    if (rc != GT_OK)
        return rc;


    /* state the system now ready to allow : events , traffic */
    rc = appDemoAc5xWithCaelum_SystemReady();
    CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoAc5xWithCaelum_SystemReady", rc);
    if (rc != GT_OK)
        return rc;

    return GT_OK;
}

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
)
{
    GT_STATUS  rc = GT_OK ;

    /* state the system now ready to allow : events , traffic */
    rc = appDemoAc5xWithCaelum_SystemReady();
    CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoAc5xWithCaelum_SystemReady", rc);
    return rc;
}
