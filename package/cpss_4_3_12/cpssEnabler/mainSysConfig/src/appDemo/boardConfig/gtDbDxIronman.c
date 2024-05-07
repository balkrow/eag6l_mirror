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
* @file gtDbDxIronman.c
*
* @brief Initialization functions for the Ironman - SIP6.30 - board.
*
* @version   1
********************************************************************************
*/

#include <appDemo/boardConfig/appDemoBoardConfig.h>
#include <appDemo/boardConfig/gtDbDxBoardTypeConfig.h>
#include <appDemo/boardConfig/appDemoCfgMisc.h>
#include <appDemo/sysHwConfig/gtAppDemoSysConfigDefaults.h>
#include <appDemo/sysHwConfig/appDemoDb.h>
#include <appDemo/boardConfig/gtDbDxBobcat2Mappings.h>
#include <appDemo/userExit/userEventHandler.h>
#include <appDemo/boardConfig/appDemoBoardConfig_ezBringupTools.h>
#include <appDemo/boardConfig/appDemoBoardConfig_mpdTools.h>
#include <appDemo/boardConfig/appDemoBoardConfig_mpdAndPp.h>

#include <gtExtDrv/drivers/gtUartDrv.h>
#ifdef INCLUDE_UTF
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#endif /* INCLUDE_UTF */
#include <gtOs/gtOsExc.h>

#include <cpss/dxCh/dxChxGen/port/cpssDxChPortManager.h>
#include <cpssDriver/pp/config/prvCpssDrvPpCfgInit.h>
#include <appDemo/utils/mainUT/appDemoUtils.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgL2Ecmp.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwInit.h>
#include <cpss/dxCh/dxChxGen/phy/cpssDxChPhySmiPreInit.h>
#include <cpss/common/smi/cpssGenSmi.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#ifdef INCLUDE_MPD
#include <appDemo/phy/gtAppDemoPhyConfig.h>
#endif

/* offset used during HW device ID calculation formula */
extern GT_U8 appDemoHwDevNumOffset;
/* cpssInitSystem has been run */
extern GT_BOOL systemInitialized;
/* port manager boolean variable */
extern GT_BOOL portMgr;

/* indication that during initialization the call to appDemoBc2PortListInit(...)
   was already done so no need to call it for second time. */
extern GT_U32  appDemo_PortsInitList_already_done;

extern GT_STATUS userForceBoardType(IN GT_U32 boardType);

extern CPSS_DXCH_PORT_MAP_STC  *appDemoPortMapPtr;
/* number of elements in appDemoPortMapPtr */
extern GT_U32                  appDemoPortMapSize;

extern GT_STATUS prvCpssDrvHwPpPrePhase1NextDevFamilySet(IN CPSS_PP_FAMILY_TYPE_ENT devFamily);
extern GT_STATUS appDemoDxLibrariesInit(
    IN  GT_U8                       dev,
    IN  APP_DEMO_LIB_INIT_PARAMS    *libInitParamsPtr,
    IN  CPSS_PP_CONFIG_INIT_STC     *sysConfigParamsPtr
);
extern GT_STATUS appDemoDxPpGeneralInit(GT_U8 dev);
extern GT_STATUS appDemoDxTrafficEnable(GT_U8 dev);
extern GT_STATUS boardAutoPollingConfigure(IN  GT_U8   dev,IN  GT_U32  boardType);
extern GT_STATUS appDemoBc2IpLpmRamDefaultConfigCalc(
    IN  GT_U8                                    devNum,
    IN  GT_U32                                   maxNumOfPbrEntries,
    OUT CPSS_DXCH_LPM_RAM_CONFIG_STC             *ramDbCfgPtr
);
extern GT_STATUS appDemoBc2PortListInit
(
    IN GT_U8 dev,
    IN PortInitList_STC * portInitList,
    IN GT_BOOL            skipCheckEnable
);
extern GT_STATUS appDemoDxHwDevNumChange
(
    IN  GT_U8                       dev,
    IN  GT_HW_DEV_NUM               hwDevNum
);
GT_STATUS appDemoBc2PIPEngineInit
(
    IN  GT_U8       boardRevId
);
extern GT_STATUS appDemoDxHwPpPhase1DbEntryInit
(
    IN  GT_U8                        dev,
    INOUT  CPSS_DXCH_PP_PHASE1_INIT_INFO_STC   *cpssPpPhase1ParamsPtr,
    IN  CPSS_PP_DEVICE_TYPE         deviceId,
    OUT GT_U32                      *waIndexPtr,
    OUT CPSS_DXCH_IMPLEMENT_WA_ENT  *waFromCpss,
    OUT GT_BOOL                     *trunkToCpuWaEnabledPtr
);

#define PRINT_SKIP_DUE_TO_DB_FLAG(reasonPtr , flagNamePtr) \
        cpssOsPrintf("NOTE: '%s' skipped ! (due to flag '%s') \n",  \
            reasonPtr , flagNamePtr)

#define ARR_PTR_AND_SIZE_MAC(myArr , arrPtr , arrLen) \
    arrPtr = &myArr[0];                               \
    arrLen = sizeof(myArr)/sizeof(myArr[0])

#define UNUSED_PARAM_MAC(x) x = x

#define PORT_NUMBER_ARR_SIZE    9
typedef struct
{
    GT_PHYSICAL_PORT_NUM                portNumberArr  [PORT_NUMBER_ARR_SIZE];
    GT_U32                              globalDmaNumber[PORT_NUMBER_ARR_SIZE];
    CPSS_DXCH_PORT_MAPPING_TYPE_ENT     mappingType;
    GT_BOOL                             reservePreemptiveChannel;
}IRONMAN_PORT_MAP_STC;

/* the physical port numbers are restricted to 0..74 due to 'my physical ports' table (TTI unit) */
/* mapping of Ironman physical ports to MAC ports (and TXQ ports). */
static IRONMAN_PORT_MAP_STC ironman_l_port_mode[] =
{
    /* ports from USX mac */
/*SD_6*/  {{ 0,  1,  2,  3,  4,  5,  6,  7,  GT_NA} , { 16, 17, 18, 19, 20, 21, 22, 23, GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, GT_TRUE}
/*SD_3*/ ,{{ 8,  9,  10, 11, 12, 13, 14, 15, GT_NA} , { 8,  9,  10, 11, 12, 13, 14, 15, GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, GT_TRUE}
/*SD_0*/ ,{{ 16, 17, 18, 19, 20, 21, 22, 23, GT_NA} , { 0,  1,  2,  3,  4,  5,  6,  7,  GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, GT_TRUE}
/*SD_1*/ ,{{ 24, 25, 26, 27, 28, 29, 30, 31, GT_NA} , { 24, 25, 26, 27, 28, 29, 30, 31, GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, GT_FALSE}
/*SD_4*/ ,{{ 32, 33, 34, 35, 36, 37, 38, 39, GT_NA} , { 32, 33, 34, 35, 36, 37, 38, 39, GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, GT_FALSE}
/*SD_7*/ ,{{ 40, 41, 42, 43, 44, 45, 46, 47, GT_NA} , { 40, 41, 42, 43, 44, 45, 46, 47, GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, GT_FALSE}
/*SD_8,2,5*/                 ,{{ 48, 49, 50, GT_NA} , { 54, 52, 53,                     GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, GT_FALSE}
/*SD_9,10,11,12*/        ,{{ 51, 52, 53, 54, GT_NA} , { 48, 49, 50, 51,                 GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, GT_TRUE}

    ,{{CPU_PORT,/*63*/          GT_NA}         , {56,/*DMA - bound to MG0*/GT_NA}         , CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,     GT_FALSE    }
};
static const GT_U32   actualNum_ironman_l_port_mode = sizeof(ironman_l_port_mode)/sizeof(ironman_l_port_mode[0]);

/* IM-L RD Board */
static IRONMAN_PORT_MAP_STC ironman_rd_dx1518m_24g6xg_port_mode[] =
{
    /* ports from USX mac */
	/*SD_6*/  {{ 7,  6,  5,  4,  3,  2,  1,  0,  GT_NA} , { 16, 17, 18, 19, 20, 21, 22, 23, GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, GT_TRUE}
	/*SD_3*/ ,{{ 15,  14,  13, 12, 11, 10, 9, 8, GT_NA} , { 8,  9,  10, 11, 12, 13, 14, 15, GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, GT_TRUE}
	/*SD_0*/ ,{{ 23, 22, 21, 20, 19, 18, 17, 16, GT_NA} , { 0,  1,  2,  3,  4,  5,  6,  7,  GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, GT_TRUE}
	/*SD_1*/ ,{{ 24, 25, 26, 27, 28, 29, 30, 31, GT_NA} , { 24, 25, 26, 27, 28, 29, 30, 31, GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, GT_FALSE}
	/*SD_4*/ ,{{ 32, 33, 34, 35, 36, 37, 38, 39, GT_NA} , { 32, 33, 34, 35, 36, 37, 38, 39, GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, GT_FALSE}
	/*SD_7*/ ,{{ 40, 41, 42, 43, 44, 45, 46, 47, GT_NA} , { 40, 41, 42, 43, 44, 45, 46, 47, GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, GT_FALSE}
	/*SD_8,2,5*/                 ,{{ 48, 49, 50, GT_NA} , { 54, 52, 53,                     GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, GT_FALSE}
	/*SD_9,10,11,12*/        ,{{ 51, 52, 53, 54, GT_NA} , { 48, 49, 50, 51,                 GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, GT_TRUE}

    ,{{CPU_PORT,/*63*/          GT_NA}         , {56,/*DMA - bound to MG0*/GT_NA}         , CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,     GT_FALSE    }
};

static const GT_U32   actualNum_ironman_rd_dx1518m_24g6xg_port_mode = sizeof(ironman_rd_dx1518m_24g6xg_port_mode)/sizeof(ironman_rd_dx1518m_24g6xg_port_mode[0]);

static IRONMAN_PORT_MAP_STC ironman_m_port_mode[] =
{
    /* ports from USX mac */
     {{ 0, 1, 2, 3, 4, 5, 6, 7, GT_NA} , { 16, 17, 18, 19, 20, 21, 22, 23, GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, GT_TRUE}
    ,{{ 8, 9, 10, 11, GT_NA}           , { 48, 49, 50, 51, GT_NA}                 , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, GT_TRUE}
    ,{{CPU_PORT,/*63*/          GT_NA}         , {56,/*DMA - bound to MG0*/GT_NA}         , CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,     GT_FALSE    }
};
static const GT_U32   actualNum_ironman_m_port_mode = sizeof(ironman_m_port_mode)/sizeof(ironman_m_port_mode[0]);

/* use same map as IM-M DB board */
static IRONMAN_PORT_MAP_STC ironman_rd_dx1508m_8g4xg_port_mode[] =
{
    /* ports from USX mac */
     {{ 0, 1, 2, 3, 4, 5, 6, 7, GT_NA} , { 16, 17, 18, 19, 20, 21, 22, 23, GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, GT_TRUE}
    ,{{ 8, 9, 10, 11, GT_NA}           , { 48, 49, 50, 51, GT_NA}                 , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, GT_TRUE}
    ,{{CPU_PORT,/*63*/          GT_NA}         , {56,/*DMA - bound to MG0*/GT_NA}         , CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,     GT_FALSE    }
};
static const GT_U32   actualNum_ironman_rd_dx1508m_8g4xg_port_mode = sizeof(ironman_rd_dx1508m_8g4xg_port_mode)/sizeof(ironman_rd_dx1508m_8g4xg_port_mode[0]);

static IRONMAN_PORT_MAP_STC ironman_l_RPP_port_mode[] =
{
    /* ports from USX mac */
/*SD_6*/  {{ 0,  GT_NA} , { 16, GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, GT_FALSE}
/*SD_3*/ ,{{ 8,  9,  10, 11, 12, 13, 14, 15, GT_NA} , { 16,  16,  16, 16, 16, 16, 16, 16, GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E, GT_FALSE}
/*SD_0*/ ,{{ 16, 17, 18, 19, 20, 21, 22, 23, GT_NA} , { 0,  1,  2,  3,  4,  5,  6,  7,  GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, GT_TRUE}
/*SD_1*/ ,{{ 24, 25, 26, 27, 28, 29, 30, 31, GT_NA} , { 24, 25, 26, 27, 28, 29, 30, 31, GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, GT_FALSE}
/*SD_4*/ ,{{ 32, 33, 34, 35, 36, 37, 38, 39, GT_NA} , { 32, 33, 34, 35, 36, 37, 38, 39, GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, GT_FALSE}
/*SD_7*/ ,{{ 40, 41, 42, 43, 44, 45, 46, 47, GT_NA} , { 40, 41, 42, 43, 44, 45, 46, 47, GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, GT_FALSE}
/*SD_8,2,5*/                 ,{{ 48, 49, 50, GT_NA} , { 54, 52, 53,                     GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, GT_FALSE}
/*SD_9,10,11,12*/        ,{{ 51, 52, 53, 54, GT_NA} , { 48, 49, 50, 51,                 GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E, GT_TRUE}

    ,{{CPU_PORT,/*63*/          GT_NA}         , {56,/*DMA - bound to MG0*/GT_NA}         , CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,     GT_FALSE    }
};

static const GT_U32   actualNum_ironman_l_RPP_port_mode = sizeof(ironman_l_RPP_port_mode)/sizeof(ironman_l_RPP_port_mode[0]);

/***************************************************************/
/* flag to use the MAX number of macs that the device supports */
/* AND max SDMAs                                               */
/***************************************************************/

#define MAX_MAPPED_PORTS_CNS    128
static  GT_U32  actualNum_cpssApi_ironman_defaultMap = 0;/*actual number of valid entries in cpssApi_ironman_defaultMap[] */
static CPSS_DXCH_PORT_MAP_STC   cpssApi_ironman_defaultMap[MAX_MAPPED_PORTS_CNS];
    /* filled at run time , by info from falcon_3_2_defaultMap[] */

static PortInitList_STC *ironman_force_PortsInitListPtr = NULL;

/* Ironman-L ports mode */
static PortInitList_STC ironman_l_portInitlist_sfp[] =
{    /* ports in 10G */
     /*{ PORT_LIST_TYPE_INTERVAL,  {0 ,47, 1, APP_INV_PORT_CNS },  CPSS_PORT_SPEED_1000_E,  CPSS_PORT_INTERFACE_MODE_USX_OUSGMII_E}*/
     { PORT_LIST_TYPE_LIST,      {0,4,8,12,16,20,48,49,50,51,52,53,54, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_KR_E      }

/*  not supported ports yet !
    ,{ PORT_LIST_TYPE_INTERVAL,  {48,54, 1, APP_INV_PORT_CNS },  CPSS_PORT_SPEED_1000_E,  CPSS_PORT_INTERFACE_MODE_USX_OUSGMII_E}*/

    ,{ PORT_LIST_TYPE_EMPTY,     {APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,   CPSS_PORT_INTERFACE_MODE_QSGMII_E    }
};

/* Ironman-L ports mode */
static PortInitList_STC ironman_l_portInitlist_phy[] =
{    /* ports in 10G */
     { PORT_LIST_TYPE_INTERVAL,  {0 ,15, 1, APP_INV_PORT_CNS },  CPSS_PORT_SPEED_1000_E,  CPSS_PORT_INTERFACE_MODE_USX_OUSGMII_E}
    ,{ PORT_LIST_TYPE_LIST,      {16,20,48,49,50,51,52,53,54, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_KR_E      }
    ,{ PORT_LIST_TYPE_EMPTY,     {APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,   CPSS_PORT_INTERFACE_MODE_QSGMII_E    }
};

/* Ironman-rd-dx1518m-24g6xg ports mode */
static PortInitList_STC ironman_rd_dx1518m_24g6xg_portInitlist[] =
{    /* ports in 1G */
     { PORT_LIST_TYPE_INTERVAL,  {0 ,23, 1, APP_INV_PORT_CNS },  CPSS_PORT_SPEED_1000_E,  CPSS_PORT_INTERFACE_MODE_USX_OUSGMII_E}
     /* ports in 10G */
    ,{ PORT_LIST_TYPE_LIST,      {49,50,51,52,53,54, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_KR_E      }
    ,{ PORT_LIST_TYPE_EMPTY,     {APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,   CPSS_PORT_INTERFACE_MODE_QSGMII_E    }
};

/* Ironman M ports mode */
static PortInitList_STC ironman_m_portInitlist[] =
{    /* ports in 1G */
     { PORT_LIST_TYPE_INTERVAL,  {0,7,1, APP_INV_PORT_CNS }       , CPSS_PORT_SPEED_1000_E,   CPSS_PORT_INTERFACE_MODE_USX_OUSGMII_E  }
     /* ports in 10G */
    ,{ PORT_LIST_TYPE_LIST,      {8,9,10,11, APP_INV_PORT_CNS }   , CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_KR_E           }

    ,{ PORT_LIST_TYPE_EMPTY,     {APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,   CPSS_PORT_INTERFACE_MODE_QSGMII_E                         }
};

static PortInitList_STC ironman_l_portInitlist_rpp_sfp[] =
{    /* ports in 10G */
     /*{ PORT_LIST_TYPE_INTERVAL,  {0 ,47, 1, APP_INV_PORT_CNS },  CPSS_PORT_SPEED_1000_E,  CPSS_PORT_INTERFACE_MODE_USX_OUSGMII_E}*/
     { PORT_LIST_TYPE_LIST,      {0,16,20,48,49,50,51,52,53,54, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_10000_E,  CPSS_PORT_INTERFACE_MODE_KR_E      }
     ,{ PORT_LIST_TYPE_LIST,      {8,  9,  10, 11, 12, 13, 14, 15, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_REMOTE_E,  CPSS_PORT_INTERFACE_MODE_REMOTE_E      }
/*  not supported ports yet !
    ,{ PORT_LIST_TYPE_INTERVAL,  {48,54, 1, APP_INV_PORT_CNS },  CPSS_PORT_SPEED_1000_E,  CPSS_PORT_INTERFACE_MODE_USX_OUSGMII_E}*/

    ,{ PORT_LIST_TYPE_EMPTY,     {APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,   CPSS_PORT_INTERFACE_MODE_QSGMII_E    }
};


/* fill array of cpssApi_ironman_defaultMap[] and update actualNum_cpssApi_ironman_defaultMap and numCpuSdmas */
static GT_STATUS fillDbForCpssPortMappingInfo(
    IN GT_U32 devNum,
    IN GT_U8  boardRevId
)
{
    GT_STATUS   rc;
    IRONMAN_PORT_MAP_STC *currAppDemoInfoPtr;
    CPSS_DXCH_PORT_MAP_STC *currCpssInfoPtr;
    GT_U32  ii,jj;
    GT_U32 numEntries;
    GT_U32 maxPhyPorts;
    GT_U32 interfaceNum;
    CPSS_PP_DEVICE_TYPE  devType;
    GT_U32      value;

    if(actualNum_cpssApi_ironman_defaultMap)
    {
        /* already initialized */
        return GT_OK;
    }

    devType = PRV_CPSS_PP_MAC(devNum)->devType;
    maxPhyPorts = PRV_CPSS_DXCH_MAX_PHY_PORT_NUMBER_MAC(devNum);
    switch (devType)
    {
        case CPSS_98DX1507_CNS  :
        case CPSS_98DX1507M_CNS :
        case CPSS_98DX1508_CNS  :
        case CPSS_98DX1508M_CNS :
        case CPSS_98DX2548_CNS  :
        case CPSS_98DX2548M_CNS :
            if (boardRevId == 4)
            {
                currAppDemoInfoPtr = &ironman_rd_dx1508m_8g4xg_port_mode[0];
                numEntries = actualNum_ironman_rd_dx1508m_8g4xg_port_mode;
            }
            else
            {
                currAppDemoInfoPtr = &ironman_m_port_mode[0];
                numEntries = actualNum_ironman_m_port_mode;
            }
            break;
        default:
            if((appDemoDbEntryGet("hqos", &value) == GT_OK) && (value != 0))
            {
                currAppDemoInfoPtr = &ironman_l_RPP_port_mode[0];
                numEntries = actualNum_ironman_l_RPP_port_mode;
            }
            else if (boardRevId == 5)
            {
                /* RD board */
                currAppDemoInfoPtr = &ironman_rd_dx1518m_24g6xg_port_mode[0];
                numEntries = actualNum_ironman_rd_dx1518m_24g6xg_port_mode;
            }
            else
            {
               /* DB boards */
               currAppDemoInfoPtr = &ironman_l_port_mode[0];
               numEntries = actualNum_ironman_l_port_mode;
            }
    }

    cpssOsMemSet(cpssApi_ironman_defaultMap,0,sizeof(cpssApi_ironman_defaultMap));

    currCpssInfoPtr    = &cpssApi_ironman_defaultMap[0];

    for (ii = 0 ; ii < numEntries; ii++ , currAppDemoInfoPtr++)
    {
        for(jj = 0 ; currAppDemoInfoPtr->portNumberArr[jj] != GT_NA ; jj++ , currCpssInfoPtr++)
        {
            if(actualNum_cpssApi_ironman_defaultMap >= maxPhyPorts)
            {
                rc = GT_FULL;
                CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssApi_ironman_defaultMap is FULL, maxPhy port ", rc);
                return rc;
            }

            interfaceNum = currAppDemoInfoPtr->globalDmaNumber[jj];

            currCpssInfoPtr->physicalPortNumber = currAppDemoInfoPtr->portNumberArr[jj];
            currCpssInfoPtr->mappingType        = currAppDemoInfoPtr->mappingType;
            currCpssInfoPtr->interfaceNum       = interfaceNum;
            currCpssInfoPtr->txqPortNumber      = 0;/* CPSS will give us 8 TC queues per port */
            currCpssInfoPtr->reservePreemptiveChannel = currAppDemoInfoPtr->reservePreemptiveChannel;

            if(currAppDemoInfoPtr->mappingType == CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E)
            {
                if(PRV_CPSS_PP_MAC(devNum)->isGmDevice && currCpssInfoPtr->physicalPortNumber != CPU_PORT)
                {
                    /* skip this SDMA - the GM currently supports single SDMA */
                    currCpssInfoPtr--;
                    continue;
                }
            }

            actualNum_cpssApi_ironman_defaultMap++;
        }
    }

    return GT_OK;
}

/**
* @internal ironman_getBoardInfo function
* @endinternal
*
* @brief   Return the board configuration info.
*
* @param[in] firstDev                 - GT_TRUE - first device, GT_FALSE - next device
*
* @param[out] hwInfoPtr                - HW info of found device.
* @param[out] pciInfo                  - PCI info of found device.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*
* @note This is a simplified version of getBoardInfo.
*
*/
static GT_STATUS ironman_getBoardInfo
(
    IN  GT_BOOL          firstDev,
    OUT CPSS_HW_INFO_STC *hwInfoPtr,
    OUT GT_PCI_INFO      *pciInfo
)
{
    GT_STATUS   rc;

    /* In our case we want to find just one prestera device on PCI bus*/
    rc = gtPresteraGetPciDev(firstDev, pciInfo);
    if (rc != GT_OK)
    {
        if (firstDev != GT_FALSE)
        {
            osPrintf("Could not recognize any device on PCI bus!\n");
        }
        return GT_NOT_FOUND;
    }

    /*copy info for later use*/
    appDemoPpConfigList[SYSTEM_DEV_NUM_MAC(0)].pciInfo = *pciInfo;

    rc = extDrvPexConfigure(
            pciInfo->pciBusNum,
            pciInfo->pciIdSel,
            pciInfo->funcNo,
            MV_EXT_DRV_CFG_FLAG_EAGLE_E,
            hwInfoPtr);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("extDrvPexConfigure", rc);

    appDemoPpConfigList[SYSTEM_DEV_NUM_MAC(0)].hwInfo = *hwInfoPtr;

    return rc;
}

/**
* @internal ironman_getBoardInfoSimple function
* @endinternal
*
* @brief   Return the board configuration info.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*
* @note This is a simplified version of getBoardInfo.
*
*/
static GT_STATUS ironman_getBoardInfoSimple
(
    OUT CPSS_HW_INFO_STC *hwInfoPtr,
    OUT GT_PCI_INFO *pciInfo
)
{
    GT_STATUS   rc;

    rc = ironman_getBoardInfo(GT_TRUE/*firstDev*/, hwInfoPtr, pciInfo);
    if (rc != GT_OK) {return rc;}

    /* In our case we want to find just one prestera device on PCI bus*/
    rc = gtPresteraGetPciDev(GT_FALSE, pciInfo);
    if(rc == GT_OK)
    {
        osPrintf("More than one recognized device found on PCI bus!\n");
        return GT_INIT_ERROR;
    }

    return GT_OK;
}

#ifdef ASIC_SIMULATION
extern GT_U32 simCoreClockOverwrittenGet(GT_VOID);
#endif

/**
* @internal ironman_boardTypePrint function
* @endinternal
*
* @brief   This function prints type of Ironman board and Cider version.
*
* @param[in] boardName                - board name
* @param[in] devName                  - device name
*                                       none
*/
static GT_VOID ironman_boardTypePrint
(
    IN GT_CHAR  *boardName,
    IN GT_CHAR *devName
)
{
    GT_CHAR *environment;

#ifdef GM_USED
    environment = "GM (Golden Model) - simulation";
#elif defined ASIC_SIMULATION
    if(cpssDeviceRunCheck_onEmulator())
    {
        environment = "WM (White Model) - simulation (special EMULATOR Mode)";
    }
    else
    {
        environment = "WM (White Model) - simulation";
    }
#else
    if(cpssDeviceRunCheck_onEmulator())
    {
        environment = "HW (on EMULATOR)";
    }
    else
    {
        environment = "HW (Hardware)";
    }
#endif
    cpssOsPrintf("%s Board Type: %s [%s]\n", devName , boardName , environment);
/*    cpssOsPrintf("Using Cider Version [%s] \n", ciderVersionName);*/

}

/**
* @internal getPpPhase1ConfigSimple function
* @endinternal
*
* @brief   Returns the configuration parameters for cpssDxChHwPpPhase1Init().
*
* @param[in] devNum                   - The CPSS devNum.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*
* @note This is a simplified version of configBoardAfterPhase1.
*
*/
static GT_STATUS getPpPhase1ConfigSimple
(
    IN  GT_SW_DEV_NUM                       devNum,
    IN  GT_U8                               boardRevId,
    OUT CPSS_DXCH_PP_PHASE1_INIT_INFO_STC  *phase1Params
)
{
    GT_STATUS rc;

    UNUSED_PARAM_MAC(boardRevId);

    phase1Params->devNum = CAST_SW_DEVNUM(devNum);

/* retrieve PP Core Clock from HW */
    phase1Params->coreClock             = CPSS_DXCH_AUTO_DETECT_CORE_CLOCK_CNS;

    phase1Params->ppHAState             = CPSS_SYS_HA_MODE_ACTIVE_E;


    phase1Params->numOfPortGroups       = 1;

    /* Eagle access mode */
    phase1Params->mngInterfaceType      = CPSS_CHANNEL_PEX_EAGLE_E;

    phase1Params->maxNumOfPhyPortsToUse = 0; /* single mode of 64 physical ports ,
                                            so supports value 64 or 0 */

    phase1Params->serdesRefClock        = CPSS_DXCH_PP_SERDES_REF_CLOCK_EXTERNAL_25_SINGLE_ENDED_E;

    /* add Db Entry */
    rc = appDemoDxHwPpPhase1DbEntryInit(CAST_SW_DEVNUM(devNum),phase1Params,
        CPSS_98DXA000_CNS ,/* dummy to represent 'ironman' family (one of falcon values) */
        NULL,NULL,NULL);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoDxHwPpPhase1DbEntryInit", rc);
    if(rc != GT_OK)
        return rc;

    return GT_OK;
}

/**
* @internal getPpPhase2ConfigSimple function
* @endinternal
*
* @brief   Returns the configuration parameters for cpssDxChHwPpPhase2Init().
*
* @param[in] devNum                   - The device number to get the parameters for.
* @param[in] devType                  - The Pp device type
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS getPpPhase2ConfigSimple
(
    IN GT_SW_DEV_NUM       devNum,
    IN  CPSS_PP_DEVICE_TYPE         devType,
    OUT CPSS_DXCH_PP_PHASE2_INIT_INFO_STC  *phase2Params
)
{
    GT_STATUS                   rc;
    CPSS_PP_PHASE2_INIT_PARAMS  appDemo_ppPhase2Params;
    GT_U32                      tmpData;
    GT_U32                      auDescNum,fuDescNum;
    GT_U32 rxDescNum = RX_DESC_NUM_DEF;
    GT_U32 txDescNum = TX_DESC_NUM_DEF;

    appDemoPhase2DefaultInit(&appDemo_ppPhase2Params);
    osMemSet(phase2Params,0,sizeof(CPSS_DXCH_PP_PHASE2_INIT_INFO_STC));

    /* Enable double AUQ for SIP6 devices */
    appDemo_ppPhase2Params.useDoubleAuq = GT_TRUE;

    if( appDemoDbEntryGet("useMultiNetIfSdma", &tmpData) == GT_NO_SUCH )
    {
        appDemo_ppPhase2Params.useMultiNetIfSdma = GT_TRUE;
        if( appDemoDbEntryGet("skipTxSdmaGenDefaultCfg", &tmpData) == GT_NO_SUCH )
        {

            /* Enable Tx queue 3 to work in Tx queue generator mode */
            rc = appDemoDbEntryAdd("txGenQueue_3", GT_TRUE);
            if(rc != GT_OK)
            {
                return rc;
            }

            /* Enable Tx queue 6 to work in Tx queue generator mode */
            rc = appDemoDbEntryAdd("txGenQueue_6", GT_TRUE);
            if(rc != GT_OK)
            {
                 return rc;
            }
         }
    }

    /* allow each tile with it's own number of AUQ size */
    auDescNum = AU_DESC_NUM_SIP6_DEF;
    /* allow each pipe with it's own number of FUQ size (for CNC upload) */
    /* NOTE: Fdb Upload still happen only on 'per tile' base (like AUQ) ...
       it is not per pipe triggered */
    fuDescNum = AU_DESC_NUM_SIP6_DEF;

    phase2Params->newDevNum                  = CAST_SW_DEVNUM(devNum);
    phase2Params->noTraffic2CPU = GT_FALSE;
    phase2Params->netifSdmaPortGroupId = 0;
    phase2Params->auMessageLength = CPSS_AU_MESSAGE_LENGTH_8_WORDS_E;

    rc = appDemoDbEntryAdd("fuqUseSeparate", GT_TRUE);
    if(rc != GT_OK)
    {
        return rc;
    }

    if(appDemoDbEntryGet("fuDescNum", &tmpData) == GT_OK)
    {
        /* Someone already stated the number of fuDescNum, so we NOT override it ! */
    }
    else
    {
        /* Add the fuDescNum to the DB, to allow appDemoAllocateDmaMem(...) to use it ! */
        rc = appDemoDbEntryAdd("fuDescNum", fuDescNum);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    /*
     * Enable use of CnM SRAM for TX SDMA Queue 4 and allocate space for 20 desc
     */
    appDemoDbEntryAdd("txSramQueue_4", 1);

    if(appDemoDbEntryGet("txSramNumOfDesc_4", &tmpData) != GT_OK)
    {
        appDemoDbEntryAdd("txSramNumOfDesc_4", 20);
    }

    appDemoCpssCurrentDevIndex = devNum;

    rc = appDemoAllocateDmaMem(devType,
                               /* Rx */
                               rxDescNum,RX_BUFF_SIZE_DEF,RX_BUFF_ALLIGN_DEF,
                               /* Tx */
                               txDescNum,
                               /* AUQ */
                               auDescNum,
                               &appDemo_ppPhase2Params);

    CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoAllocateDmaMem", rc);
    if(rc != GT_OK)
        return rc;

    /* convert CPSS_PP_PHASE2_INIT_PARAMS to CPSS_DXCH_PP_PHASE2_INIT_INFO_STC */
    phase2Params->netIfCfg          = appDemo_ppPhase2Params.netIfCfg         ;
    phase2Params->auqCfg            = appDemo_ppPhase2Params.auqCfg           ;
    phase2Params->fuqUseSeparate    = appDemo_ppPhase2Params.fuqUseSeparate   ;
    phase2Params->fuqCfg            = appDemo_ppPhase2Params.fuqCfg           ;
    phase2Params->useSecondaryAuq   = appDemo_ppPhase2Params.useSecondaryAuq  ;
    phase2Params->useDoubleAuq      = appDemo_ppPhase2Params.useDoubleAuq     ;
    phase2Params->useMultiNetIfSdma = appDemo_ppPhase2Params.useMultiNetIfSdma;
    phase2Params->multiNetIfCfg     = appDemo_ppPhase2Params.multiNetIfCfg    ;

    return rc;
}

/**
* @internal ironman_initPortMappingStage_fromEzbXml function
* @endinternal
*
*/
static GT_STATUS ironman_initPortMappingStage_fromEzbXml
(
    IN GT_U32                   devIndex,
    OUT CPSS_DXCH_PORT_MAP_STC **mapArrPtrPtr,
    OUT GT_U32                 *mapArrLenPtr
)
{
    GT_STATUS   rc;
    #define PORT_MAP_MAX_PORTS_CNS  CPSS_MAX_PORTS_NUM_CNS/* max physical ports */
    static CPSS_DXCH_PORT_MAP_STC portMapArr[PORT_MAP_MAX_PORTS_CNS];
    CPSS_DXCH_PORT_MAP_STC *currCpuSdmaPortInfoPtr;
    GT_U32 numPorts,numCpuSdmaPorts;
    CPSS_DXCH_PORT_MAP_STC cpuSdmaArr[2];

    cpssOsBzero((GT_CHAR *)cpuSdmaArr,sizeof(cpuSdmaArr));

    numCpuSdmaPorts = 0;
    /* the EZ_BRINGUP not supports the CPU SDMA ports that need to be added */
    currCpuSdmaPortInfoPtr = &cpuSdmaArr[numCpuSdmaPorts++];
    currCpuSdmaPortInfoPtr->physicalPortNumber = CPU_PORT;
    currCpuSdmaPortInfoPtr->mappingType = CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E;
    currCpuSdmaPortInfoPtr->interfaceNum = 56; /*DMA - bound to MG0*/

    rc = appDemoEzbSip6PortMappingGet(devIndex,
            PORT_MAP_MAX_PORTS_CNS,
            portMapArr,
            &numPorts,
            numCpuSdmaPorts,
            cpuSdmaArr);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoEzbSip6PortMappingGet", rc);
    if (GT_OK != rc)
    {
        return rc;
    }

    cpssOsPrintf(" -- EZ_BRINGUP : The port mapping from XML file [%s] \n",
        ezbXmlName());

    *mapArrPtrPtr = portMapArr;
    *mapArrLenPtr = numPorts;
    return GT_OK;
}

/**
* @internal ironman_initPortMappingStage function
* @endinternal
*
*/
static GT_STATUS ironman_initPortMappingStage
(
    IN GT_U32                   devIndex,
    IN GT_SW_DEV_NUM            devNum,
    IN CPSS_PP_DEVICE_TYPE      devType
)
{
    GT_STATUS   rc;
    CPSS_DXCH_PORT_MAP_STC *mapArrPtr;
    GT_U32 mapArrLen;
    CPSS_DXCH_PORT_MAP_STC *mapUpdatedArrPtr;
    GT_U32 mapArrUpdatedLen;

    switch(devType)
    {
        case CPSS_IRONMAN_ALL_DEVICES_CASES_MAC:
        default:
            ARR_PTR_AND_SIZE_MAC(cpssApi_ironman_defaultMap, mapArrPtr, mapArrLen);
            mapArrLen = actualNum_cpssApi_ironman_defaultMap;
            break;
    }

    if(appDemoPortMapPtr && appDemoPortMapSize)
    {
        /* allow to 'FORCE' specific array by other logic */
        mapArrPtr = appDemoPortMapPtr;
        mapArrLen = appDemoPortMapSize;
    }
    else
    if(GT_TRUE == appDemoEzbIsXmlWithDevNum(devNum))
    {
        /* get information from the XML */
        rc = ironman_initPortMappingStage_fromEzbXml(devIndex,&mapArrPtr,&mapArrLen);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("ironman_initPortMappingStage_fromEzbXml", rc);
        if (GT_OK != rc)
        {
            return rc;
        }
    }

    rc = appDemoDxChPortPhysicalPortUpdatedMapCreate(
        mapArrLen, mapArrPtr, &mapArrUpdatedLen, &mapUpdatedArrPtr);
    if (GT_OK != rc)
    {
        return rc;
    }
    if (mapUpdatedArrPtr != NULL)
    {
        mapArrPtr = mapUpdatedArrPtr;
        mapArrLen = mapArrUpdatedLen;
    }

    rc = cpssDxChPortPhysicalPortMapSet(CAST_SW_DEVNUM(devNum), mapArrLen, mapArrPtr);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPortPhysicalPortMapSet", rc);
    if (GT_OK != rc)
    {
        if (mapUpdatedArrPtr != NULL)
        {
            cpssOsFree(mapUpdatedArrPtr);
        }
        return rc;
    }

    /* it's for appdemo DB only */
    rc = appDemoDxChMaxMappedPortSet(CAST_SW_DEVNUM(devNum), mapArrLen, mapArrPtr);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoDxChMaxMappedPortSet", rc);

    if (mapUpdatedArrPtr != NULL)
    {
        cpssOsFree(mapUpdatedArrPtr);
    }
    return rc;
}

/**
* @internal ironman_PortModeConfiguration_fromEzbXml function
* @endinternal
*
*/
static GT_STATUS ironman_PortModeConfiguration_fromEzbXml
(
    IN GT_U32                   devIndex,
    OUT PortInitList_STC        **portInitListPtrPtr
)
{
    GT_STATUS   rc;
    /* each port will get it's own line according to info from the XML */
    /* function appDemoEzbPortsSpeedGet will fill it                   */
    static PortInitList_STC ironman_portInitlist__from_xml[PORT_MAP_MAX_PORTS_CNS];
    GT_U32  actualNumEntries;

    rc = appDemoEzbPortsSpeedGet(devIndex,
            PORT_MAP_MAX_PORTS_CNS,
            ironman_portInitlist__from_xml,
            &actualNumEntries);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoEzbPortsSpeedGet", rc);
    if (GT_OK != rc)
    {
        return rc;
    }

    /* the XML not supports and also 'PDL' not support next: CPSS_PORT_INTERFACE_MODE_USX_20G_OXGMII_E */
    /* so convert CPSS_PORT_INTERFACE_MODE_XHGS_E to CPSS_PORT_INTERFACE_MODE_USX_20G_OXGMII_E         */
    {
        static EZB_XML_SPEED_IF_MODE_REPLACE_STC replaceInfo[] =
        {
            /* ability to state 'no speed' and 'no interface' by using '10M' speed from the XML,
                , as PDL not supports 'no speed' and 'no interface'  */
            {
                /*old*/{GT_NA                                    ,CPSS_PORT_SPEED_10_E},
                /*new*/{CPSS_PORT_INTERFACE_MODE_NA_E            ,CPSS_PORT_SPEED_NA_E}
            },
            /* allow to set {USX_20G_OXGMII , *} , as PDL not supports USX_20G_OXGMII */
            {
                /*old*/{CPSS_PORT_INTERFACE_MODE_XHGS_E          ,GT_NA/*all speeds*/},
                /*new*/{CPSS_PORT_INTERFACE_MODE_USX_20G_OXGMII_E,GT_NA}
            },
            /* allow to set {KR , 100G} to become {KR4 , 100G} , as PDL not supports KR4 */
            {
                /*old*/{CPSS_PORT_INTERFACE_MODE_KR_E            ,CPSS_PORT_SPEED_100G_E},
                /*new*/{CPSS_PORT_INTERFACE_MODE_KR4_E           ,CPSS_PORT_SPEED_100G_E}
            },

            /* must be last */
            EZB_XML_SPEED_IF_MODE_REPLACE___MUAST_BE_LAST__CNS
        };
        rc = appDemoEzbPortsSpeedAndModeUpdate(ironman_portInitlist__from_xml,replaceInfo);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoEzbPortsSpeedGet", rc);
        if (GT_OK != rc)
        {
            return rc;
        }
    }

    cpssOsPrintf(" -- EZ_BRINGUP : The default port speed from XML file [%s] \n",
        ezbXmlName());
    *portInitListPtrPtr = ironman_portInitlist__from_xml;
    return GT_OK;
}

/**
* @internal ironman_initSerdesMuxingAndPolaritySwap function
* @endinternal
*
*/
static GT_STATUS ironman_initSerdesMuxingAndPolaritySwap_fromEzbXml
(
    IN  GT_U32                              devIndex,
    OUT EZB_GEN_MAC_SERDES_INFO_STC        **macToSerdesArrPtr,
    OUT GT_U32                              *macToSerdesArr_arrSizePtr,
    OUT EZB_GEN_SERDES_INFO_STC            **serdesArrPtr,
    OUT GT_U32                              *serdesArr_arrSizePtr
)
{
    static EZB_GEN_MAC_SERDES_INFO_STC macToSerdesArr_fromEzbXml[PORT_MAP_MAX_PORTS_CNS];
    EZB_PP_CONFIG *ezbPpConfigPtr;
    EZB_GEN_MAC_INFO_STC   *macGenInfoPtr;
    GT_U32  ii,jj,numOfMuxingSerdeses;
#if EZ_BRINGUP
    ezbPpConfigPtr = &ezbPpConfigList[devIndex];
#else
    (void)devIndex;
    ezbPpConfigPtr = NULL;
#endif
    CPSS_NULL_PTR_CHECK_MAC(ezbPpConfigPtr);
    macGenInfoPtr = ezbPpConfigPtr->macGenInfoPtr;

    cpssOsPrintf("XML : mac-to-serdes info : \n");
    cpssOsPrintf("      mac      |      local SERDESes  \n");
    cpssOsPrintf("======================================\n");

    numOfMuxingSerdeses = 0;
    for(ii = 0 ; ii < ezbPpConfigPtr->numOfMacPorts; ii++,macGenInfoPtr++)
    {
        if(macGenInfoPtr->isMacUsed == GT_FALSE ||
           macGenInfoPtr->isSerdesInfoValid == GT_FALSE ||
           macGenInfoPtr->macSerdesInfo.holdSerdesMuxingInfo == GT_FALSE)
        {
            continue;
        }

        cpssOsPrintf("      [%3.3d]   |   [%d]",
            macGenInfoPtr->macNum,
            macGenInfoPtr->macSerdesInfo.macToSerdesMap.serdesLanes[0]);

        for(jj = 1 ; jj < macGenInfoPtr->macSerdesInfo.numOfSerdeses; jj++)
        {
            cpssOsPrintf(",[%d]",
                macGenInfoPtr->macSerdesInfo.macToSerdesMap.serdesLanes[jj]);
        }
        cpssOsPrintf("\n");

        if(macGenInfoPtr->macSerdesInfo.firstMacPtr == NULL)/* should not be NULL ... but just in case */
        {
            macGenInfoPtr->macSerdesInfo.firstMacPtr = macGenInfoPtr;
        }

        cpssOsMemCpy(&macToSerdesArr_fromEzbXml[numOfMuxingSerdeses],
            &macGenInfoPtr->macSerdesInfo,
            sizeof(EZB_GEN_MAC_SERDES_INFO_STC));

        numOfMuxingSerdeses++;
    }

    *macToSerdesArr_arrSizePtr =  numOfMuxingSerdeses;
    *macToSerdesArrPtr         =  macToSerdesArr_fromEzbXml;

    if(numOfMuxingSerdeses)
    {
        cpssOsPrintf(" -- EZ_BRINGUP : The 'MAC-SERDES muxing' from XML file [%s] \n",
            ezbXmlName());
    }
    else
    {
        cpssOsPrintf(" -- EZ_BRINGUP : no MAC-SERDES muxing defined \n");
    }

    cpssOsPrintf("XML : serdes polarity info : \n");
    cpssOsPrintf("      SERDES   |  TX polarity |   RX polarity \n");
    cpssOsPrintf("==============================================\n");
    for(jj = 0 ; jj < ezbPpConfigPtr->numOfSerdeses ; jj++)
    {
        if(ezbPpConfigPtr->serdesGenInfoPtr[jj].polarityValid == GT_FALSE)
        {
            continue;
        }

        cpssOsPrintf("        [%3.3d]   |   %s      |   %s  \n"
            ,ezbPpConfigPtr->serdesGenInfoPtr[jj].polarity.laneNum
            ,ezbPpConfigPtr->serdesGenInfoPtr[jj].polarity.invertTx == GT_TRUE ? "true ":"false"
            ,ezbPpConfigPtr->serdesGenInfoPtr[jj].polarity.invertRx == GT_TRUE ? "true ":"false"
            );
    }

    *serdesArr_arrSizePtr      =  ezbPpConfigPtr->numOfSerdeses;   /* can be ZERO */
    *serdesArrPtr              =  ezbPpConfigPtr->serdesGenInfoPtr;/* can be NULL */

    if(ezbPpConfigPtr->numOfSerdeses)
    {
        cpssOsPrintf(" -- EZ_BRINGUP : The 'SERDES lane swaping' from XML file [%s] \n",
            ezbXmlName());
    }
    else
    {
        cpssOsPrintf(" -- EZ_BRINGUP : no 'SERDES lane swaping' (polarity invert) defined \n");
    }

    return GT_OK;
}

/**
* @internal ironman_phyInit function
* @endinternal
*
* @brief   PHY related configurations
*
* @param[in] devNum                - device number
* @param[in] boardRevId            - Board revision Id.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise
*
*/
GT_STATUS ironman_phyInit
(
    IN  GT_U8  devNum,
    IN  GT_U8  boardRevId
)
{
    GT_STATUS   rc;
    GT_U32      smiAddr;
    GT_U16      data;
    CPSS_SYSTEM_RECOVERY_INFO_STC tempSystemRecovery_Info;
    GT_U32      maxSmiAddr;

    rc = cpssSystemRecoveryStateGet(&tempSystemRecovery_Info);
    if( rc != GT_OK)
    {
        return rc;
    }

    if ((tempSystemRecovery_Info.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_HA_E) &&
        (tempSystemRecovery_Info.systemRecoveryState == CPSS_SYSTEM_RECOVERY_INIT_STATE_E) )
    {
        return GT_OK;
    }

    maxSmiAddr = (boardRevId == 2) ? 16 : 24;

    /* disable PCH header for PTP mode */
    for(smiAddr = 0; smiAddr < maxSmiAddr; smiAddr++)
    {
        /* Page 26 */
        rc = cpssSmiRegisterWriteShort(devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, CPSS_PHY_SMI_INTERFACE_0_E, smiAddr, 22, 26);
        if( rc != GT_OK)
        {
            return rc;
        }

        /* read register 0 from page 26 (PCH Common Control) */
        rc = cpssSmiRegisterReadShort(devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, CPSS_PHY_SMI_INTERFACE_0_E, smiAddr, 0, &data);
        if( rc != GT_OK)
        {
            return rc;
        }

        /* disable PCH - field 'pch_mch_en' (bit_0) */
        data = (data >> 1) << 1;


        /* write register 0 from page 26 (PCH Common Control) */
        rc = cpssSmiRegisterWriteShort(devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, CPSS_PHY_SMI_INTERFACE_0_E, smiAddr, 0, data);
        if( rc != GT_OK)
        {
            return rc;
        }

        /* Change to Page 0 */
        rc = cpssSmiRegisterWriteShort(devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, CPSS_PHY_SMI_INTERFACE_0_E, smiAddr, 22, 0);
        if( rc != GT_OK)
        {
            return rc;
        }
    }

    /* LED configurations */
    for(smiAddr = 0; smiAddr < maxSmiAddr; smiAddr++)
    {
        /* Page 28 */
        rc = cpssSmiRegisterWriteShort(devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, CPSS_PHY_SMI_INTERFACE_0_E, smiAddr, 22, 28);
        if( rc != GT_OK)
        {
            return rc;
        }
        /* LED matrix mode */
        rc = cpssSmiRegisterWriteShort(devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, CPSS_PHY_SMI_INTERFACE_0_E, smiAddr, 1, 0x400);
        if( rc != GT_OK)
        {
            return rc;
        }

        /* Page 3 */
        rc = cpssSmiRegisterWriteShort(devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, CPSS_PHY_SMI_INTERFACE_0_E, smiAddr, 22, 3);
        if( rc != GT_OK)
        {
            return rc;
        }

        if (boardRevId == 2)
        {
            /* LED Link/Activity */
            rc = cpssSmiRegisterWriteShort(devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, CPSS_PHY_SMI_INTERFACE_0_E, smiAddr, 16, 0x1181);
            if( rc != GT_OK)
            {
                return rc;
            }
        }
        else
        {
            /* LED Link/Activity */
            rc = cpssSmiRegisterWriteShort(devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, CPSS_PHY_SMI_INTERFACE_0_E, smiAddr, 16, 0x1771);
            if( rc != GT_OK)
            {
                return rc;
            }

            rc = cpssSmiRegisterWriteShort(devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, CPSS_PHY_SMI_INTERFACE_0_E, smiAddr, 17, 0x8800);
            if( rc != GT_OK)
            {
                return rc;
            }
        }

        /* Change to Page 0 */
        rc = cpssSmiRegisterWriteShort(devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, CPSS_PHY_SMI_INTERFACE_0_E, smiAddr, 22, 0);
        if( rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

static GT_U32                              ironman_MacToSerdesMap_arrSize = 0;
static EZB_GEN_MAC_SERDES_INFO_STC*   ironman_MacToSerdesMap = NULL;
static GT_U32                              ironman_SerdesArray_arrSize = 0;
static EZB_GEN_SERDES_INFO_STC*       ironman_SerdesArray = NULL;

/* IM-L DB board SERDES polarity settings */
static const APPDEMO_SERDES_LANE_POLARITY_STC  ironman_L_DB_PolarityArray[] =
{
    /* laneNum  invertTx    invertRx */
    { 3,   GT_TRUE, GT_TRUE },
    { 6,   GT_TRUE, GT_TRUE }
};

/* IM-M DB board SERDES polarity settings */
static const APPDEMO_SERDES_LANE_POLARITY_STC  ironman_M_DB_PolarityArray[] =
{
    /* laneNum  invertTx    invertRx */
    { 9,   GT_TRUE, GT_FALSE },
    { 10,   GT_TRUE, GT_TRUE },
    { 11,   GT_TRUE, GT_TRUE },
    { 12,   GT_TRUE, GT_FALSE }
};

/* IM-M RD board SERDES polarity settings */
static const APPDEMO_SERDES_LANE_POLARITY_STC  ironman_M_RD_PolarityArray[] =
{
    /* laneNum  invertTx    invertRx */
    { 12,  GT_FALSE, GT_TRUE  }
};

/* IM-L RD board SERDES polarity settings */
static const APPDEMO_SERDES_LANE_POLARITY_STC  ironman_L_RD_PolarityArray[] =
{
    /* laneNum  invertTx    invertRx */
    {  0,   GT_TRUE,  GT_FALSE },
    {  2,   GT_TRUE,  GT_FALSE },
    {  5,   GT_TRUE,  GT_FALSE },
    {  9,   GT_TRUE,  GT_FALSE },
    { 10,   GT_TRUE,  GT_TRUE  },
    { 11,   GT_TRUE,  GT_TRUE  },
    { 12,   GT_TRUE,  GT_FALSE },
};

/**
* @internal ironman_initSerdesMuxingAndPolaritySwap function
* @endinternal
*
*/
static GT_STATUS ironman_initSerdesMuxingAndPolaritySwap
(
    IN GT_U32                   devIndex,
    IN GT_SW_DEV_NUM            devNum,
    IN GT_U8                    boardRevId
)
{
    GT_STATUS   rc;
    EZB_GEN_MAC_SERDES_INFO_STC *macSerdesPtr;
    EZB_GEN_SERDES_INFO_STC     *serdesPtr;
    GT_U32  ii;

    if(GT_TRUE == appDemoEzbIsXmlWithDevNum(devNum))
    {
        /* get information from the XML */
        rc = ironman_initSerdesMuxingAndPolaritySwap_fromEzbXml(devIndex,
            &ironman_MacToSerdesMap,
            &ironman_MacToSerdesMap_arrSize,
            &ironman_SerdesArray,
            &ironman_SerdesArray_arrSize);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("ironman_initSerdesMuxingAndPolaritySwap_fromEzbXml`", rc);
        if (GT_OK != rc)
        {
            return rc;
        }
    }

    /**************************/
    /* handle 'Serdes muxing' */
    /**************************/
    if(ironman_MacToSerdesMap)
    {
        macSerdesPtr = &ironman_MacToSerdesMap[0];

        for(ii = 0 ; ii < ironman_MacToSerdesMap_arrSize ; ii++,macSerdesPtr++)
        {
            if(macSerdesPtr->holdSerdesMuxingInfo == GT_FALSE)
            {
                continue;
            }

            if(macSerdesPtr->firstMacPtr == NULL)
            {
                continue;
            }

            rc = cpssDxChPortLaneMacToSerdesMuxSet(CAST_SW_DEVNUM(devNum),
                    macSerdesPtr->firstMacPtr->physicalPort,/* The physical port that represent the 'group' */
                    &macSerdesPtr->macToSerdesMap);         /* The array of the lane swaps */
            if (rc != GT_OK &&
                rc != GT_NOT_SUPPORTED)/* HWS not supports the device yet 'mvHwsPortLaneMacToSerdesMuxSet' */
            {
                CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPortLaneMacToSerdesMuxSet", rc);
                return rc;
            }
            else if(rc == GT_NOT_SUPPORTED)
            {
                cpssOsPrintf("WARNING : cpssDxChPortLaneMacToSerdesMuxSet not supported yet \n");
                /* no need for all ports to fail */
                break;
            }
        }
    }
    else
    {
        cpssOsPrintf("no MAC-SERDES muxing defined \n");
    }

    /**************************/
    /* handle 'lane swaping'  */
    /* ('polarity invert')    */
    /**************************/
    if(ironman_SerdesArray)
    {
        GT_BOOL wasCpssLaneInitDone = PRV_CPSS_DXCH_PP_MAC(devNum)->serdesPolarityPtr ? GT_TRUE : GT_FALSE;

        serdesPtr = &ironman_SerdesArray[0];

        for(ii = 0 ; ii < ironman_SerdesArray_arrSize ; ii++,serdesPtr++)
        {
            if(serdesPtr->polarityValid != GT_TRUE)
            {
                /* not hold info from the XML */
                continue;
            }

           rc = cpssDxChPortSerdesLanePolaritySet(CAST_SW_DEVNUM(devNum), 0/*portGroupNum*/,
                       serdesPtr->polarity.laneNum,
                       serdesPtr->polarity.invertTx,
                       serdesPtr->polarity.invertRx);
            if (rc != GT_OK &&
                rc != GT_NOT_IMPLEMENTED)/* HWS not supports the device yet: in function hwsHawkIfInit :
                    hwsDeviceSpecInfo[devNum].serdesType = SERDES_LAST;             CPSS_TBD_BOOKMARK_AC5P */
            {
                CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPortSerdesLanePolaritySet", rc);
                return rc;
            }
            else if(rc == GT_NOT_IMPLEMENTED)
            {
                cpssOsPrintf("WARNING : cpssDxChPortSerdesLanePolaritySet not supported yet \n");
                if(wasCpssLaneInitDone == GT_FALSE)
                {
                    /* free the DB if not created before , as the DB still exists
                       even though the support for polarity not exists
                       this to prevent fails during cpssInitSystem
                    */
                    FREE_PTR_MAC(PRV_CPSS_DXCH_PP_MAC(devNum)->serdesPolarityPtr);
                }
                /* no need for all ports to fail */
                break;
            }
        }
    }
    else
    {
        GT_U32          doPhyCfg = GT_FALSE; /* need or not PHY configuration */
        GT_U32          polarityArraySize;
        APPDEMO_SERDES_LANE_POLARITY_STC const * polarityArrayPtr = NULL; /* some boards need not polarity settings */

        switch (boardRevId)
        {
            case 2:
                /* IM-L DB board with two 1780 PHYs */
                polarityArrayPtr = ironman_L_DB_PolarityArray;
                polarityArraySize = sizeof(ironman_L_DB_PolarityArray) / sizeof(APPDEMO_SERDES_LANE_POLARITY_STC);
                doPhyCfg = GT_TRUE;
                break;
            case 3:
                /* IM-M DB board */
                polarityArrayPtr = ironman_M_DB_PolarityArray;
                polarityArraySize = sizeof(ironman_M_DB_PolarityArray) / sizeof(APPDEMO_SERDES_LANE_POLARITY_STC);
                break;
            case 4:
                /* IM-M RD board */
                polarityArrayPtr = ironman_M_RD_PolarityArray;
                polarityArraySize = sizeof(ironman_M_RD_PolarityArray) / sizeof(APPDEMO_SERDES_LANE_POLARITY_STC);
                break;
            case 5:
                /* IM-L RD board with three 1780 PHYs */
                polarityArrayPtr = ironman_L_RD_PolarityArray;
                polarityArraySize = sizeof(ironman_L_RD_PolarityArray) / sizeof(APPDEMO_SERDES_LANE_POLARITY_STC);
                doPhyCfg = GT_TRUE;
                break;
            default:
                break;
        }

        if (polarityArrayPtr)
        {
            GT_U32 i;

            for (i = 0; i < polarityArraySize; i++)
            {
                rc = cpssDxChPortSerdesLanePolaritySet(devNum, 0,
                        polarityArrayPtr[i].laneNum,
                        polarityArrayPtr[i].invertTx,
                        polarityArrayPtr[i].invertRx);

                CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPortSerdesLanePolaritySet", rc);
                if (rc != GT_OK)
                {
                    return rc;
                }
            }
        }

#if !defined(ASIC_SIMULATION) && defined(INCLUDE_MPD)
        if (doPhyCfg)
        {
            rc = gtAppDemoPhyMpdInit(devNum,boardRevId);
            CPSS_ENABLER_DBG_TRACE_RC_MAC("gtAppDemoPhyMpdInit", rc);

            rc = ironman_phyInit(devNum,boardRevId);
            CPSS_ENABLER_DBG_TRACE_RC_MAC("phoenix_phyInit", rc);
            if (rc != GT_OK)
            {
                return rc;
            }
        }
#else
        GT_UNUSED_PARAM(doPhyCfg);
#endif
    }

    return GT_OK;
}

/**
* @internal ironman_PortModeConfiguration function
* @endinternal
*
*/
static GT_STATUS ironman_PortModeConfiguration
(
    IN GT_U32                   devIndex,
    IN GT_SW_DEV_NUM            devNum,
    IN CPSS_PP_DEVICE_TYPE      devType,
    IN  GT_U8                   boardRevId
)
{
    GT_STATUS   rc;
    PortInitList_STC *portInitListPtr;
    CPSS_SYSTEM_RECOVERY_INFO_STC   system_recovery;
    GT_U32           value;

    UNUSED_PARAM_MAC(devType);

    if(ironman_force_PortsInitListPtr)
    {
        /* allow to 'FORCE' specific array by other logic */
        portInitListPtr = ironman_force_PortsInitListPtr;
    }
    else
    if(GT_TRUE == appDemoEzbIsXmlWithDevNum(devNum))
    {
        /* get information from the XML */
        rc = ironman_PortModeConfiguration_fromEzbXml(devIndex,&portInitListPtr);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("ironman_PortModeConfiguration_fromEzbXml", rc);
        if (GT_OK != rc)
        {
            return rc;
        }
    }
    else
    {
        switch (devType)
        {
            case CPSS_98DX1507_CNS  :
            case CPSS_98DX1507M_CNS :
            case CPSS_98DX1508_CNS  :
            case CPSS_98DX1508M_CNS :
            case CPSS_98DX2548_CNS  :
            case CPSS_98DX2548M_CNS :
                portInitListPtr = ironman_m_portInitlist;
                break;
            default:
                if(boardRevId == 2)
                {
                    portInitListPtr = ironman_l_portInitlist_phy;
                }
                else if (boardRevId == 5) /* ironman_rd_dx1518m_24g6xg */
                {
	                portInitListPtr = ironman_rd_dx1518m_24g6xg_portInitlist;
                }
                else
                {   if((appDemoDbEntryGet("hqos", &value) == GT_OK) && (value != 0))
                    {
                        portInitListPtr = ironman_l_portInitlist_rpp_sfp;
                    }
                    else
                    {
                        portInitListPtr = ironman_l_portInitlist_sfp;
                    }
                }
                break;
        }
    }

    rc = cpssSystemRecoveryStateGet(&system_recovery);
    if (rc != GT_OK)
    {
      return rc;
    }

    if (CPSS_SYSTEM_RECOVERY_PROCESS_NOT_ACTIVE_E == system_recovery.systemRecoveryProcess)
    {
        rc = appDemoBc2PortListInit(CAST_SW_DEVNUM(devNum),portInitListPtr,GT_TRUE);
        if (rc != GT_OK)
        {
            return rc;
        }

        /* indication for function appDemoBc2PortInterfaceInit(...) to
           not call appDemoBc2PortListInit(...) */
        appDemo_PortsInitList_already_done = 1;
    }

    return GT_OK;
}

#define HIT_NUM_0           0
#define HIT_NUM_1           1
static BANK_PARTITION_INFO_STC  ironman_ttiLookupArr[] = {
{ 2,0                                 ,HIT_NUM_0},
{ 2,CPSS_DXCH_TCAM_MAX_NUM_BANKS_CNS/2,HIT_NUM_0},

{ 3,0                                 ,HIT_NUM_1},
{ 3,CPSS_DXCH_TCAM_MAX_NUM_BANKS_CNS/2,HIT_NUM_1},
/* must be LAST*/
{GT_NA,GT_NA,GT_NA}
};

/**
* @internal ironman_appDemoDbUpdate function
* @endinternal
*
* @brief   Update the AppDemo DB (appDemoPpConfigList[devIndex]) about the device
*
* @param[in] devIndex                 - the device index
* @param[in] devNum                   - the SW devNum
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS ironman_appDemoDbUpdate
(
    IN GT_U32             devIndex,
    IN GT_SW_DEV_NUM      devNum,
    IN CPSS_PP_DEVICE_TYPE     devType
)
{
    appDemoPpConfigList[devIndex].valid = GT_TRUE;
    appDemoPpConfigList[devIndex].devNum = CAST_SW_DEVNUM(devNum);
    appDemoPpConfigList[devIndex].deviceId = devType;
    appDemoPpConfigList[devIndex].devFamily = CPSS_PP_FAMILY_DXCH_IRONMAN_E;
    CPSS_TBD_BOOKMARK_IRONMAN
    appDemoPpConfigList[devIndex].apiSupportedBmp = APP_DEMO_DXCH_FUNCTIONS_SUPPORT_CNS;

    appDemoPpConfigList[devIndex].ttiTcamPartitionInfoPtr = ironman_ttiLookupArr;

    return GT_OK;
}

/**
* @internal ironman_mppConfig function
* @endinternal
*
* @brief   Set MPP control registers for serial LED mode
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; AC5; Falcon; AC5P; AC5X; Harrier.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS ironman_mppConfig
(
    IN  GT_U8    devNum,
    IN GT_U8     boardRevId
)
{
    GT_STATUS   rc;         /* return code */
    GT_U32      ii;

    GT_U32 const ironman_L_mppListArr[][2] =
    {
        /* MPP 30 - LED_CLK, MPP 31 - LED_STB, MPP 32 - LED_DATA */
         {30,1}, {31,1}, {32,1}
    };

    GT_U32 const ironman_M_mppListArr[][2] =
    {
        /* MPP 30 - LED_P0, MPP 31 - LED_P1, MPP 32 - LED_P2, MPP 33 - LED_P3 */
        {30,2}, {31,2}, {32,2}, {33,2}
    };

    GT_U32 const * mppListArrPtr;

    GT_U32 mppListSize;

    /* Ironman-M DB use parallel LED, but IM-M RD use serial LED like IM-L */
    if(boardRevId == 3)
    {
        mppListArrPtr = ironman_M_mppListArr[0];
        mppListSize = 2 * (sizeof(ironman_M_mppListArr) / sizeof(ironman_M_mppListArr[0]));
    }
    else
    {
        mppListArrPtr = ironman_L_mppListArr[0];
        mppListSize = 2 * (sizeof(ironman_L_mppListArr) / sizeof(ironman_L_mppListArr[0]));
    }

    for (ii = 0; ii < mppListSize; ii+=2)
    {
        rc = cpssDxChHwMppSelectSet(devNum, mppListArrPtr[ii], mppListArrPtr[ii+1]);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    /* IM-M RD does not use SyncE */
    if(boardRevId != 4)
    {
        /* Set MPP_45 to work as RCVR_CLK_OUT[0] with SyncE */
        rc = cpssDxChHwMppSelectSet(devNum, 45, 0x1);
        if(rc != GT_OK)
        {
            return rc;
        }
        /* Set MPP_46 to work as RCVR_CLK_OUT[1] with SyncE */
        rc = cpssDxChHwMppSelectSet(devNum, 46, 0x1);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal ironman_initPhase1AndPhase2 function
* @endinternal
*
*/
static GT_STATUS ironman_initPhase1AndPhase2
(
    IN GT_SW_DEV_NUM       devNum,
    IN GT_U8              boardRevId,
    IN CPSS_HW_INFO_STC   *hwInfoPtr,
    IN GT_U32             devIndex,
    OUT CPSS_PP_DEVICE_TYPE     *devTypePtr
)
{
    GT_STATUS   rc;
    CPSS_DXCH_PP_PHASE1_INIT_INFO_STC       cpssPpPhase1Info;     /* CPSS phase 1 PP params */
    CPSS_DXCH_PP_PHASE2_INIT_INFO_STC       cpssPpPhase2Info;     /* CPSS phase 2 PP params */
    GT_HW_DEV_NUM           hwDevNum;
    CPSS_PP_DEVICE_TYPE     devType;
    GT_U32                  waIndex;
    static CPSS_DXCH_IMPLEMENT_WA_ENT falcon_WaList[] =
    {
        /* no current WA that need to force CPSS */
        CPSS_DXCH_IMPLEMENT_WA_LAST_E
    };
    GT_U32    secondsStart;
    GT_U32    nanoSecondsStart;

    osMemSet(&cpssPpPhase1Info, 0, sizeof(cpssPpPhase1Info));
    osMemSet(&cpssPpPhase2Info, 0, sizeof(cpssPpPhase2Info));


    rc = getPpPhase1ConfigSimple(devNum, boardRevId, &cpssPpPhase1Info);
    if (rc != GT_OK)
        return rc;

    cpssPpPhase1Info.hwInfo[0] = *hwInfoPtr;

    /* devType is retrieved in hwPpPhase1Part1*/
    rc = appDemoDxChHwPpPhase1ResetAndInit(&cpssPpPhase1Info, &devType);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChHwPpPhase1Init", rc);
    if (rc != GT_OK)
        return rc;

    *devTypePtr = devType;

    /* update the appDemo DB */
    rc = ironman_appDemoDbUpdate(devIndex, devNum , devType);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("ironman_appDemoDbUpdate", rc);
    if (rc != GT_OK)
        return rc;

    if (appDemoInitRegDefaults != GT_FALSE)
    {
        return GT_OK;
    }

    rc = ironman_mppConfig(CAST_SW_DEVNUM(devNum), boardRevId);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* Enable use of CnM SRAM for TX SDMA */
    appDemoDbEntryAdd("txSramQueue", 1);

    rc = appDemo_configure_dma_per_devNum(CAST_SW_DEVNUM(devNum));
    if (rc != GT_OK)
    {
        return rc;
    }

    waIndex = 0;
    while(falcon_WaList[waIndex] != CPSS_DXCH_IMPLEMENT_WA_LAST_E)
    {
        waIndex++;
    }

    if(waIndex)
    {
        cpssOsTimeRT(&secondsStart, &nanoSecondsStart);
        rc = cpssDxChHwPpImplementWaInit(CAST_SW_DEVNUM(devNum),waIndex,falcon_WaList, NULL);
        if (prvAppDemoFirstInitTimes.initSystemTimePpImplementWaInit == 0)
        {
            prvAppDemoFirstInitTimes.initSystemTimePpImplementWaInit =
                prvAppDemoGetDurationTimeMilliSec(secondsStart, nanoSecondsStart);
        }
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChHwPpImplementWaInit", rc);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    /*************************************************/
    /* fill once the DB of 'port mapping' to be used */
    /*************************************************/
    rc = fillDbForCpssPortMappingInfo(devNum,boardRevId);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("fillDbForCpssPortMappingInfo", rc);
    if (rc != GT_OK)
        return rc;

    rc = ironman_initPortMappingStage(devIndex,devNum,devType);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("ironman_initPortMappingStage", rc);
    if (GT_OK != rc)
        return rc;

    /* memory related data, such as addresses and block lenghts, are set in this funtion*/
    rc = getPpPhase2ConfigSimple(devNum, devType, &cpssPpPhase2Info);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("getPpPhase2ConfigSimple", rc);
    if (rc != GT_OK)
        return rc;

    cpssOsTimeRT(&secondsStart, &nanoSecondsStart);
    rc = cpssDxChHwPpPhase2Init(CAST_SW_DEVNUM(devNum),  &cpssPpPhase2Info);
    if (prvAppDemoFirstInitTimes.initSystemTimePpPhase2Init == 0)
    {
        prvAppDemoFirstInitTimes.initSystemTimePpPhase2Init =
            prvAppDemoGetDurationTimeMilliSec(secondsStart, nanoSecondsStart);
    }
    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChHwPpPhase2Init", rc);
    if (rc != GT_OK)
        return rc;

    hwDevNum = (devNum + appDemoHwDevNumOffset) & 0x3FF;

    /* set HWdevNum related values */
    rc = appDemoDxHwDevNumChange(CAST_SW_DEVNUM(devNum), hwDevNum);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoDxHwDevNumChange", rc);
    if (GT_OK != rc)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal ironman_getNeededLibs function
* @endinternal
*
*/
static GT_STATUS ironman_getNeededLibs(
    OUT  APP_DEMO_LIB_INIT_PARAMS    *libInitParamsPtr,
    OUT  CPSS_PP_CONFIG_INIT_STC     *sysConfigParamsPtr
)
{
    osMemSet(libInitParamsPtr,0,sizeof(*libInitParamsPtr));
    osMemSet(sysConfigParamsPtr,0,sizeof(*sysConfigParamsPtr));

    libInitParamsPtr->initBridge = GT_TRUE;/*prvBridgeLibInit*/
    libInitParamsPtr->initIpv4   = GT_TRUE;/*prvIpLibInit*/
    libInitParamsPtr->initIpv6   = GT_TRUE;/*prvIpLibInit*/
    libInitParamsPtr->initPcl    = GT_TRUE;/*prvPclLibInit*/
    libInitParamsPtr->initTcam   = GT_TRUE;/*prvTcamLibInit*/
    libInitParamsPtr->initPort   = GT_TRUE;/*prvPortLibInit*/
    libInitParamsPtr->initPhy    = GT_TRUE;/*prvPhyLibInit*/
    libInitParamsPtr->initNetworkIf = GT_TRUE;/*prvNetIfLibInit*/
    libInitParamsPtr->initMirror = GT_TRUE;/*prvMirrorLibInit*/
    libInitParamsPtr->initTrunk  = GT_TRUE;/*prvTrunkLibInit*/
    libInitParamsPtr->initPha    = GT_FALSE;/*prvPhaLibInit --> NO PHA in Ironman */

    sysConfigParamsPtr->numOfTrunks         = _4K;    /*used by prvTrunkLibInit*/
    sysConfigParamsPtr->lpmDbSupportIpv4    = GT_TRUE;/*used by prvIpLibInit*/
    sysConfigParamsPtr->lpmDbSupportIpv6    = GT_TRUE;/*used by prvIpLibInit*/
    /*NOTYE: info sysConfigParamsPtr->lpmRamMemoryBlocksCfg is set later : used by prvIpLibInit*/

    sysConfigParamsPtr->pha_packetOrderChangeEnable = GT_FALSE;/*used by prvPhaLibInit*/

    return GT_OK;
}

/* number of SIP6 PBR entries in lpm line */
#define PRV_CPSS_SIP6_NUM_PBR_ENTRIES_IN_LPM_LINE_CNS 5

/**
* @internal ironman_initPpLogicalInit function
* @endinternal
*
*/
static GT_STATUS ironman_initPpLogicalInit
(
    IN GT_SW_DEV_NUM       devNum,
    OUT CPSS_DXCH_LPM_RAM_CONFIG_STC    *lpmRamMemoryBlocksCfgPtr
)
{
    GT_STATUS   rc;
    CPSS_DXCH_PP_CONFIG_INIT_STC    ppLogInitParams;
    GT_U32 value = 0;
    GT_U32    secondsStart;
    GT_U32    nanoSecondsStart;

    osMemSet(&ppLogInitParams ,0, sizeof(ppLogInitParams));

    /* wanted to use small block for PBR  = 3200 (640*5) instead of 8K , but this cause bad behavior */
    /* so wanted to use 8K but this cause to use more than single big bank (5k in bank = 1K lines)
       8K fatal error on UTF_TEST_CASE_MAC(cpssDxChLpmLeafEntryWrite) */
    /* so using 5K */
    /* from more info
       see JIRA : CPSS-10305 : Ironman : using 8K PBR cause cpss to write to non-existing memory */
    ppLogInitParams.maxNumOfPbrEntries = _1K;
    ppLogInitParams.lpmMemoryMode = CPSS_DXCH_LPM_RAM_MEM_MODE_FULL_MEM_SIZE_E;
    ppLogInitParams.sharedTableMode = CPSS_DXCH_CFG_SHARED_TABLE_MODE_MID_L3_MID_L2_MIN_EM_E;/* don't care value for this device as it have no shared tables ! */

    if(appDemoDbEntryGet("maxNumOfPbrEntries", &value) == GT_OK)
    {
        ppLogInitParams.maxNumOfPbrEntries = value;
    }

    if(appDemoDbEntryGet("sharedTableMode", &value) == GT_OK)
    {
        ppLogInitParams.sharedTableMode = value;
    }

    cpssOsTimeRT(&secondsStart, &nanoSecondsStart);
    rc = cpssDxChCfgPpLogicalInit(CAST_SW_DEVNUM(devNum), &ppLogInitParams);
    if (prvAppDemoFirstInitTimes.initSystemTimePpLogicalInit == 0)
    {
        prvAppDemoFirstInitTimes.initSystemTimePpLogicalInit =
            prvAppDemoGetDurationTimeMilliSec(secondsStart, nanoSecondsStart);
    }
    if(rc != GT_OK)
        return rc;

    /* function must be called after cpssDxChCfgPpLogicalInit that update the value of :
        PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.lpmRam */
    /* calculate info needed by 'prvDxCh2Ch3IpLibInit' (init of LPM manager) */
    rc = appDemoFalconIpLpmRamDefaultConfigCalc(CAST_SW_DEVNUM(devNum),
                                                ppLogInitParams.sharedTableMode,
                                                ppLogInitParams.maxNumOfPbrEntries,
                                                lpmRamMemoryBlocksCfgPtr);
    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

extern void pexAccessTracePreWriteCallbackBind(void (*pCb)(GT_U32, GT_U32));

static void appDemoHwPpDrvRegTracingIronmanDpTracingRegAddressesBoundsSet
(
    IN GT_U8  devNum,
    IN GT_U32 unitsBitmap
)
{
    GT_STATUS                rc;
    GT_U32                   baseArr[20];
    GT_U32                   sizeArr[20];
    GT_U32                   i;
    GT_U32                   unitsNum;
    PRV_CPSS_DXCH_UNIT_ENT   unitId;
    GT_BOOL                  error;

    cpssOsMemSet(baseArr, 0, sizeof(baseArr));
    cpssOsMemSet(sizeArr, 0, sizeof(sizeArr));

    unitsNum = 0;
    for (i = 0; (i < 20); i++)
    {
        if ((unitsBitmap & (1 << i)) == 0) continue;
        switch (i)
        {
            case  0: unitId = PRV_CPSS_DXCH_UNIT_MIF_USX_0_E; break;
            case  1: unitId = PRV_CPSS_DXCH_UNIT_MIF_USX_1_E; break;
            case  2: unitId = PRV_CPSS_DXCH_UNIT_MIF_USX_2_E; break;
            case  3: unitId = PRV_CPSS_DXCH_UNIT_MIF_USX_3_E; break;
            case  4: unitId = PRV_CPSS_DXCH_UNIT_PCA_SFF_0_E; break;
            case  5: unitId = PRV_CPSS_DXCH_UNIT_PCA_PZ_ARBITER_I_0_E; break;
            case  6: unitId = PRV_CPSS_DXCH_UNIT_TX_FIFO_E; break;
            case  7: unitId = PRV_CPSS_DXCH_UNIT_TXDMA_E; break;
            case  8: unitId = PRV_CPSS_DXCH_UNIT_RXDMA_E; break;
            case  9: unitId = PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_E_163_0_E; break;
            case 10: unitId = PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_I_163_0_E; break;
            case 11: unitId = PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_E_164_0_E; break;
            case 12: unitId = PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_I_164_0_E; break;
            case 13: unitId = PRV_CPSS_DXCH_UNIT_PB_CENTER_BLK_E; break;
            case 14: unitId = PRV_CPSS_DXCH_UNIT_PB_COUNTER_E; break;
            case 15: unitId = PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_WRITE_0_E; break;
            case 16: unitId = PRV_CPSS_DXCH_UNIT_PB_GPC_GRP_PACKET_READ_0_E; break;
            case 17: unitId = PRV_CPSS_DXCH_UNIT_PB_SHARED_MEMO_BUF_0_0_E; break;
            default:
                /* unused bits */
                continue;
        }
        error = GT_FALSE;
        baseArr[unitsNum] = prvCpssDxChHwUnitBaseAddrGet(
            devNum, unitId, &error);
        if (error != GT_FALSE)
        {
            cpssOsPrintf("prvCpssDxChHwUnitBaseAddrGet error\n");
            return;
        }
        rc = prvCpssSip6UnitIdSizeInByteGet(
            devNum, unitId, &(sizeArr[unitsNum]));
        if (rc != GT_OK)
        {
            cpssOsPrintf("prvCpssSip6UnitIdSizeInByteGet error\n");
            return;
        }
        unitsNum ++;
    }

    /* pairs of base and size */
    appDemoHwPpDrvRegTracingRegAddressesBoundsSet(
        baseArr[0], sizeArr[0], baseArr[1], sizeArr[1],
        baseArr[2], sizeArr[2], baseArr[3], sizeArr[3],
        baseArr[4], sizeArr[4], baseArr[5], sizeArr[5],
        baseArr[6], sizeArr[6], baseArr[7], sizeArr[7],
        baseArr[8], sizeArr[8], baseArr[9], sizeArr[9],
        baseArr[10], sizeArr[10], baseArr[11], sizeArr[11],
        baseArr[12], sizeArr[12], baseArr[13], sizeArr[13],
        baseArr[14], sizeArr[14], baseArr[15], sizeArr[15],
        baseArr[16], sizeArr[16], baseArr[17], sizeArr[17],
        baseArr[18], sizeArr[18], baseArr[19], sizeArr[19],
        0, 0);
}

static GT_BOOL ironmanPreWriteTrace_FirstCall = GT_TRUE;
static GT_U8   ironmanPreWriteTrace_devNum = 0;
static GT_U32  ironmanPreWriteTrace_UnitsBitmap = 0;

static void ironmanPreWriteTrace(GT_U32 address, GT_U32 data)
{
    if (ironmanPreWriteTrace_UnitsBitmap == 0)
    {
        /* no units bound - ready to new bind */
        ironmanPreWriteTrace_FirstCall = GT_TRUE;
        return;
    }
    if (ironmanPreWriteTrace_FirstCall != GT_FALSE)
    {
        if (PRV_CPSS_PP_MAC(ironmanPreWriteTrace_devNum) == 0)
        {
            /* no DB yet */
            return;
        }
        if (PRV_CPSS_DXCH_PP_MAC(ironmanPreWriteTrace_devNum)->accessTableInfoSize == 0)
        {
            /* no table DB yet */
            return;
        }
        /* bind at first call when unit addresses DB already configured */
        ironmanPreWriteTrace_FirstCall = GT_FALSE;
        appDemoHwPpDrvRegTracingIronmanDpTracingRegAddressesBoundsSet(
            ironmanPreWriteTrace_devNum,
            ironmanPreWriteTrace_UnitsBitmap);
    }
    if (appDemoHwPpDrvRegTracingIsTraced(address) == 0) return;

    osPrintf("0x%08X 0x%08X\n", address, data);
}

void appDemoHwPpDrvRegTracingIronmanDpPreWriteEnable
(
    IN GT_U8  devNum,
    IN GT_U32 unitsBitmap
)
{
    ironmanPreWriteTrace_devNum      = devNum;
    ironmanPreWriteTrace_UnitsBitmap = unitsBitmap;
    pexAccessTracePreWriteCallbackBind(&ironmanPreWriteTrace);
}


/**
* @internal ironman_appDemoInitSequence function
* @endinternal
*
* @brief   This is the 'simple' board initialization function for Falcon device.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*
* @note This is a simplified version of cpssInitSystem.
*
*/
static GT_STATUS ironman_appDemoInitSequence
(
    IN  GT_U8             boardRevId,
    IN GT_U32             devIndex,
    IN GT_SW_DEV_NUM      devNum,
    IN CPSS_HW_INFO_STC   *hwInfoPtr
)
{
    GT_STATUS   rc;
    CPSS_PP_DEVICE_TYPE     devType;
    APP_DEMO_LIB_INIT_PARAMS    libInitParams;
    CPSS_PP_CONFIG_INIT_STC     sysConfigParams;
    GT_BOOL supportSystemReset = GT_TRUE;
    GT_U32      tmpData;
    GT_U32    secondsStart;
    GT_U32    nanoSecondsStart;

    if (appDemoDbEntryGet("portMgr", &tmpData) != GT_OK)
    {
        portMgr = GT_FALSE;
    }
    else
    {
        portMgr = (tmpData == 1) ? GT_TRUE : GT_FALSE;
    }

    /* Set debug devType if needed */
    rc = appDemoDebugDeviceIdGet(CAST_SW_DEVNUM(devNum), &devType);
    if (GT_OK == rc)
    {
        rc = prvCpssDrvDebugDeviceIdSet(CAST_SW_DEVNUM(devNum), devType);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("prvCpssDrvDebugDeviceIdSet", rc);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    rc = ironman_initPhase1AndPhase2(devNum,boardRevId,hwInfoPtr,devIndex,&devType);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("ironman_initPhase1AndPhase2", rc);
    if (GT_OK != rc)
        return rc;


    if (appDemoInitRegDefaults != GT_FALSE)
    {
        return GT_OK;
    }


    if(!cpssDeviceRunCheck_onEmulator())
    {
        rc = ironman_initSerdesMuxingAndPolaritySwap(devIndex,devNum,boardRevId);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("ironman_initStaticSerdesMuxing", rc);
        if (rc != GT_OK)
            return rc;
    }

    if (portMgr)
    {
        rc = cpssDxChPortManagerInit(CAST_SW_DEVNUM(devNum));
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPortMgrInit", rc);
        if(rc != GT_OK)
        {
            return rc;
        }

        rc = appDemoPortManagerTaskCreate(CAST_SW_DEVNUM(devNum));
        CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoPortManagerTaskCreate", rc);
        if (rc != GT_OK)
            return rc;
    }

    rc = ironman_PortModeConfiguration(devIndex,devNum,devType,boardRevId);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("ironman_PortModeConfiguration", rc);
    if (rc != GT_OK)
        return rc;

    rc = ironman_getNeededLibs(&libInitParams,&sysConfigParams);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("ironman_getNeededLibs", rc);
    if(rc != GT_OK)
        return rc;

    rc = ironman_initPpLogicalInit(devNum,&sysConfigParams.lpmRamMemoryBlocksCfg);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("ironman_initPpLogicalInit", rc);
    if(rc != GT_OK)
        return rc;

    cpssOsTimeRT(&secondsStart, &nanoSecondsStart);
    rc = appDemoDxLibrariesInit(CAST_SW_DEVNUM(devNum),&libInitParams,&sysConfigParams);
    if (prvAppDemoFirstInitTimes.initSystemTimeCpssLibInit == 0)
    {
        prvAppDemoFirstInitTimes.initSystemTimeCpssLibInit =
            prvAppDemoGetDurationTimeMilliSec(secondsStart, nanoSecondsStart);
    }
    CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoDxLibrariesInit", rc);
    if(rc != GT_OK)
        return rc;

    rc = appDemoDxPpGeneralInit(CAST_SW_DEVNUM(devNum));
    CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoDxPpGeneralInit", rc);
    if(rc != GT_OK)
        return rc;

#if 0 /* The Ironman not supports the PIP feature ! */
    rc = appDemoBc2PIPEngineInit(CAST_SW_DEVNUM(devNum));
    CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoBc2PIPEngineInit", rc);
    if(rc != GT_OK)
        return rc;
#endif /*0*/

    rc = appDemoDxTrafficEnable(CAST_SW_DEVNUM(devNum));
    CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoDxTrafficEnable", rc);
    if(rc != GT_OK)
        return rc;

    #ifdef GM_USED
    /* the GM not supports the 'soft reset' so we can not support system with it */
    supportSystemReset = GT_FALSE;
    #endif /*GM_USED*/
    if(cpssDeviceRunCheck_onEmulator())
    {
        CPSS_TBD_BOOKMARK_FALCON_EMULATOR
        /* until we make it happen ... do not try it on emulator ! */
        supportSystemReset = GT_FALSE;
    }

#ifndef ASIC_SIMULATION
    /* the HW not supports the 'soft reset' so we can not support system with it */
    supportSystemReset = GT_FALSE;
    cpssOsPrintf("NOTE: the Ironman HW crash the prvTgfResetAndInitSystem(...) \n");
#endif /* ASIC_SIMULATION */

    rc = appDemoB2PtpConfig(CAST_SW_DEVNUM(devNum));
    CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoB2PtpConfig", rc);
    if(rc != GT_OK)
        return rc;

    /* not support 'system reset' */
    rc = cpssDevSupportSystemResetSet(CAST_SW_DEVNUM(devNum), supportSystemReset);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDevSupportSystemResetSet", rc);
    if (GT_OK != rc)
        return rc;

    return GT_OK;
}

/**
* @internal ironman_EventHandlerInit function
* @endinternal
*
*/
static GT_STATUS ironman_EventHandlerInit
(
    IN GT_SW_DEV_NUM      devNum
)
{
    GT_STATUS   rc;
    GT_U32 value;

    UNUSED_PARAM_MAC(devNum);

    /* appDemoEventHandlerPreInit:
        needed to be called before:
        boardCfgFuncs.boardAfterInitConfig(...)
        and before:
        appDemoEventRequestDrvnModeInit()
        */
    rc = appDemoEventHandlerPreInit();
    CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoEventHandlerPreInit", rc);
    if (rc != GT_OK)
    {
        return rc;
    }


    if (GT_OK == appDemoDbEntryGet("initSystemWithoutInterrupts", &value) && value)
    {
        /* we not want interrupts handling */
        PRINT_SKIP_DUE_TO_DB_FLAG("event handling","initSystemWithoutInterrupts");
    }
    else
    {
        rc = appDemoEventRequestDrvnModeInit();
        CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoEventRequestDrvnModeInit", rc);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

extern GT_STATUS   appDemoFalconMainUtForbidenTests(void);
/*
    MainUT tests:
    list of tests/suites that forbidden because crash the CPSS or the device
   (fatal error in simulation on unknown address)
*/
static GT_STATUS   appDemoIronmanMainUtForbidenTests(void)
{
#ifdef IMPL_TGF
    static FORBIDEN_TESTS mainUt_serdes [] =
    {
         TEST_NAME(cpssDxChPortSerdesGroupGet)
        ,TEST_NAME(cpssDxChPortSerdesPowerStatusSet)
        ,TEST_NAME(cpssDxChPortSerdesResetStateSet)
        ,TEST_NAME(cpssDxChPortSerdesAutoTune)
        ,TEST_NAME(cpssDxChPortSerdesLoopbackModeSet)
        ,TEST_NAME(cpssDxChPortSerdesLoopbackModeGet)
        ,TEST_NAME(cpssDxChPortSerdesPolaritySet)
        ,TEST_NAME(cpssDxChPortSerdesPolarityGet)
        ,TEST_NAME(cpssDxChPortSerdesTuningSet)
        ,TEST_NAME(cpssDxChPortSerdesTuningGet)
        ,TEST_NAME(cpssDxChPortSerdesTxEnableSet)
        ,TEST_NAME(cpssDxChPortSerdesTxEnableGet)
        ,TEST_NAME(cpssDxChPortSerdesManualTxConfigSet)
        ,TEST_NAME(cpssDxChPortSerdesManualTxConfigGet)
        ,TEST_NAME(cpssDxChPortEomBaudRateGet)
        ,TEST_NAME(cpssDxChPortEomDfeResGet)
        ,TEST_NAME(cpssDxChPortEomMatrixGet)
        ,TEST_NAME(cpssDxChPortSerdesAutoTuneStatusGet)
        ,TEST_NAME(cpssDxChPortSerdesPpmGet)
        ,TEST_NAME(cpssDxChPortSerdesPpmSet)
        ,TEST_NAME(cpssDxChPortSerdesManualRxConfigSet)
        ,TEST_NAME(cpssDxChPortSerdesManualRxConfigGet)
        ,TEST_NAME(cpssDxChPortSerdesEyeMatrixGet)
        ,TEST_NAME(cpssDxChPortSerdesLaneTuningSet)
        ,TEST_NAME(cpssDxChPortSerdesLaneTuningGet)

        ,TEST_NAME(cpssDxChPortSyncEtherRecoveredClkAutomaticMaskingEnableSet)/* also read SERDES non supported register */
        ,TEST_NAME(cpssDxChCatchUpValidityCheckTest)                          /* also read SERDES non supported register - for recovery */

        /* other non-serdes tests : */
        ,TEST_NAME(cpssDxChPhyPortSmiRegisterRead) /*SMI is not supported as we not init the PHY lib ... see appDemoDbEntryAdd ("initPhy",0); */
        ,TEST_NAME(cpssDxChPhyPortSmiRegisterWrite)/*SMI is not supported as we not init the PHY lib ... see appDemoDbEntryAdd ("initPhy",0); */

        /* PLL tests */
        ,TEST_NAME(cpssDxChPtpPLLBypassEnableGet)
        ,TEST_NAME(cpssDxChPtpPLLBypassEnableSet)

        ,{NULL}/* must be last */
    };

    static FORBIDEN_TESTS mainUt_forbidenTests_FatalError [] =
    {
         TEST_NAME(/*cpssDxChPhySmi.*/cpssDxChPhyXsmiMdcDivisionFactorSet )
        ,{NULL}/* must be last */
    };

    /* suites that most or all tests CRASH or FATAL ERROR or Fail ! */
    static FORBIDEN_SUITES mainUt_forbidenSuites_CRASH_FAIL [] =
    {
        SUITE_NAME(cpssDxChPortSyncEther      )/* lib is not aligned yet */

        ,{NULL}/* must be last */
    };

    static FORBIDEN_SUITES mainUt_forbidenSuites_Emulator_too_long_and_PASS [] =
    {
         SUITE_NAME(cpssDxChTrunkHighLevel_1) /* cpssDxChTrunkHighLevel is enough to run */
        ,SUITE_NAME(cpssDxChTrunkLowLevel_1)  /* cpssDxChTrunkLowLevel  is enough to run */

        ,{NULL}/* must be last */
    };

    static FORBIDEN_SUITES mainUt_forbidenSuites_Emulator_skipped [] =
    {
        SUITE_NAME(cpssDxChDiagBist)
    };

    static FORBIDEN_TESTS enhUt_forbidenTests_FatalError [] =
    {
         TEST_NAME(/*tgfPtp.*/tgfPtpEgressTimestampQueueinterrupt   )
        ,TEST_NAME(/*tgfCutThrough.*/prvTgfCutThroughIpv4CtTermination_SlowToFast         ) /* port modes not supported yet */

        ,{NULL}/* must be last */
    };

    static FORBIDEN_TESTS mainUt_forbidenTests_Emulator_too_long_and_PASS [] =
    {
         TEST_NAME(cpssDxChDiagAllMemTest)                  /* : takes [313] sec */
        ,TEST_NAME(cpssDxChLatencyMonitoringPortCfgSet)     /* : takes [219] sec */
        ,TEST_NAME(cpssDxChPtpTaiTodStepSet)                /* : takes [119] sec */
        ,TEST_NAME(cpssDxChPtpTsuTSFrameCounterControlSet)  /* : takes [1322] sec*/

        ,TEST_NAME(prvCpssDxChTestFdbHash_8K)               /* : takes [92] sec */
        ,TEST_NAME(prvCpssDxChTestFdbHash_16K)              /* : takes [92] sec */
        ,TEST_NAME(prvCpssDxChTestFdbHash_with_DDE_partitions) /* : takes [101] sec */
        /*,TEST_NAME(prvCpssDxChTestFdbHash_32K) allow one of those to run */  /* : takes [92] sec */
        ,TEST_NAME(cpssDxChTrunkDbMembersSortingEnableSet_membersManipulations)/* : takes [167] sec*/
        ,TEST_NAME(cpssDxChExactMatchRulesDump)             /* : takes [209] sec */
        ,TEST_NAME(cpssDxChPortLoopbackPktTypeToCpuSet)     /* : takes [295] sec */
        ,TEST_NAME(cpssDxChHsrPrpDdeTimeFieldSet)           /* : takes [343] sec */
        ,TEST_NAME(cpssDxChHsrPrpDdeEntryWrite)             /* : takes [228] sec */
        ,TEST_NAME(cpssDxChHsrPrpDdeAgingApply)             /* : takes [2157] sec */

        /* enhUT : */
        ,TEST_NAME(tgfIOamKeepAliveFlowHashVerificationBitSelection) /* : takes [252] sec */
        ,TEST_NAME(prvTgfIpv4NonExactMatchMcRouting    ) /*takes [147] sec*/
        ,TEST_NAME(tgfIpv4UcRoutingHiddenPrefixes      ) /*takes [215] sec*/
        ,TEST_NAME(tgfBasicIpv4UcEcmpRoutingHashCheck  ) /*takes [371] sec*/
        ,TEST_NAME(prvTgfPclMetadataPacketType         ) /*takes [804] sec*/

        ,{NULL}/* must be last */
    };

    static FORBIDEN_TESTS mainUt_forbidenTests_Emulator_Stuck [] =
    {
         TEST_NAME(/*cpssDxChCscd.*/cpssDxChCscdQosTcDpRemapTableAccessModeGet)

        ,TEST_NAME(/*cpssDxChPtp.*/cpssDxChPtpTsuTSFrameCounterControlSet)
        ,TEST_NAME(/*cpssDxChPtp.*/cpssDxChPtpTsuTSFrameCounterControlGet)
        ,TEST_NAME(/*cpssDxChPtp.*/cpssDxChPtpTsuTsFrameCounterIndexSet)
        ,TEST_NAME(/*cpssDxChDiag.*/cpssDxChDiagMemoryBistsRun)
        ,{NULL}/* must be last */
    };

    static FORBIDEN_TESTS enhancedUt_forbidenTests_Emulator_Stuck [] =
    {
         TEST_NAME(/*tgfMpls.*/tgfMplsSrEntropyLabel) /* segmentation fail */
        ,{NULL}/* must be last */
    };

    static FORBIDEN_SUITES enhancedUt_forbidenSuites_Emulator_Killer [] =
    {

        {NULL}/* must be last */
    };


    static FORBIDEN_TESTS enhancedUt_forbidenTests_Emulator_LongerThanOneMinute [] =
    {
         TEST_NAME(tgfIOamKeepAliveFlowHashVerificationBitSelection)
        ,TEST_NAME(tgfPortTxSchedulerSp)
        ,TEST_NAME(tgfPortTxTailDropDbaPortResources)
        ,TEST_NAME(prvTgfBrgVplsBasicTest1)
        ,TEST_NAME(tgfBasicTrafficSanity)
        ,TEST_NAME(tgfBasicDynamicLearning_fromCascadePortDsaTagForward)
        ,TEST_NAME(tgfVlanManipulationEthernetOverMpls)
        ,TEST_NAME(prvTgfFdbIpv4UcPointerRouteByIndex)
        ,TEST_NAME(prvTgfFdbIpv4UcPointerRouteByMsg)
        ,TEST_NAME(prvTgfFdbIpv4UcRoutingLookupMask)
        ,TEST_NAME(prvTgfFdbIpv4UcRoutingAgingEnable)
        ,TEST_NAME(prvTgfFdbIpv6UcRoutingLookupMask)
        ,TEST_NAME(prvTgfFdbIpv6UcRoutingAgingEnable)
        ,TEST_NAME(prvTgfBrgGenMtuCheck)
        ,TEST_NAME(tgfBasicIpv4UcEcmpRouting)
        ,TEST_NAME(tgfBasicIpv4UcEcmpRoutingRandomEnable)
        ,TEST_NAME(tgfBasicIpv4UcEcmpRoutingIndirectAccess)
        ,TEST_NAME(prvTgfIpv4NonExactMatchUcRouting)
        ,TEST_NAME(prvTgfIpv4NonExactMatchMcRouting)
        ,TEST_NAME(tgfIpv4UcRoutingHiddenPrefixes)
        ,TEST_NAME(tgfBasicIpv4UcEcmpRoutingHashCheck)
        ,TEST_NAME(prvTgfPclMetadataPacketType)
        ,TEST_NAME(prvTgfPclMetadataPacketTypeIpOverMpls)
        ,TEST_NAME(prvTgfIngressMirrorEportVsPhysicalPort)
        ,TEST_NAME(prvTgfEgressMirrorEportVsPhysicalPort)
        ,TEST_NAME(tgfTrunkSaLearning)
        ,TEST_NAME(tgfTrunkSaLearning_globalEPortRepresentTrunk)
        ,TEST_NAME(tgfTrunkSimpleHashMacSaDa)
        ,TEST_NAME(tgfTrunkSimpleHashMacSaDa_globalEPortRepresentTrunk)
        ,TEST_NAME(tgfTrunkWithRemoteMembers)
        ,TEST_NAME(tgfTrunkCrcHashMaskPriority)
        ,TEST_NAME(tgfTrunkCascadeTrunk)
        ,TEST_NAME(tgfTrunkDesignatedTableModes)
        ,TEST_NAME(tgfTrunkCascadeTrunkWithWeights)
        ,TEST_NAME(tgfTrunkSortMode)
        ,TEST_NAME(tgfTrunkCrcHash_liteMode_CRC32_cpssPredictEgressPort)
        ,TEST_NAME(tgfTrunk_80_members_flood_EPCL)
        ,TEST_NAME(tgfTrunkPacketTypeHashModeTest)
        ,TEST_NAME(tgfIpfixSamplingToCpuTest)
        ,TEST_NAME(tgfIpfixAlarmEventsTest)
        ,TEST_NAME(prvTgfUnucUnregmcBcFiltering)
        ,TEST_NAME(prvTgfEgressInterface)
        ,TEST_NAME(tgfTunnelTermEtherOverMplsPwLabelParallelLookup)
        ,TEST_NAME(prvTgfTunnelTermPbrDualLookup)
        ,{NULL}/* must be last */
    };

    /* suites that not supported by the device */
    static FORBIDEN_SUITES enhUt_suites_not_supported [] =
    {
         SUITE_NAME(tgfPpu           )/* the PPU not supported */

        ,{NULL}/* must be last */
    };

    static FORBIDEN_TESTS enhUt_tests_not_supported [] =
    {
         TEST_NAME(prvTgfCutThroughBypassOam)/* the IOAM supported but the EOAM  is not (tests combine settings) */
        ,TEST_NAME(prvTgfPclBc2MapMplsChannelToOamOpcode)/* the IOAM supported but the EOAM  is not (tests combine settings) */
        ,TEST_NAME(prvTgfPclIngressQuadLookup                 )/* 'Quad' lookup is not supported : only 2 parallel lookups */
        ,TEST_NAME(prvTgfPclIngressQuadLookup_virtTcam        )/* 'Quad' lookup is not supported : only 2 parallel lookups */
        ,TEST_NAME(prvTgfPclIngressQuadLookup_virtTcamPriority)/* 'Quad' lookup is not supported : only 2 parallel lookups */
        ,TEST_NAME(prvTgfPclEgressQuadLookup                  )/* 'Quad' lookup is not supported : only 2 parallel lookups */
        ,TEST_NAME(prvTgfPclIngressQuadLookupWithInvalid      )/* 'Quad' lookup is not supported : only 2 parallel lookups */
        ,TEST_NAME(prvTgfPclEgressQuadLookupWithInvalid       )/* 'Quad' lookup is not supported : only 2 parallel lookups */

        ,{NULL}/* must be last */
    };


    /* suites that not implemented for the device */
    static FORBIDEN_SUITES mainUt_suites_not_implmented [] =
    {
         SUITE_NAME(cpssDxChPpu                     )/* PPU unit not supported , and test not skip it yet */
        ,SUITE_NAME(prvCpssDxChPortTxPizzaResDpHawk )/* DP units for Ironman not implemented */

        ,{NULL}/* must be last */
    };


#endif /*IMPL_TGF*/


    /* tests that defined forbidden by Ironman */

#ifdef IMPL_TGF
    appDemoForbidenSuitesAdd(enhUt_suites_not_supported);
    appDemoForbidenTestsAdd (enhUt_tests_not_supported);
    appDemoForbidenSuitesAdd(mainUt_suites_not_implmented);

    appDemoForbidenTestsAdd (mainUt_forbidenTests_FatalError);
    appDemoForbidenSuitesAdd(mainUt_forbidenSuites_CRASH_FAIL);
    appDemoForbidenTestsAdd (enhUt_forbidenTests_FatalError);

    if (cpssDeviceRunCheck_onEmulator())
    {
        /* very slow suites */
        utfAddPreSkippedRule("cpssDxChBridgeFdbManager","*","","");
        utfAddPreSkippedRule("cpssDxChExactMatchManager","*","","");
        utfAddPreSkippedRule("tgfBridgeFdbManager","*","","");

        appDemoForbidenTestsAdd (mainUt_forbidenTests_Emulator_Stuck);
        appDemoForbidenTestsAdd (enhancedUt_forbidenTests_Emulator_Stuck);
        appDemoForbidenTestsAdd (mainUt_serdes);
        appDemoForbidenTestsAdd (mainUt_forbidenTests_Emulator_too_long_and_PASS);
        appDemoForbidenSuitesAdd(mainUt_forbidenSuites_Emulator_too_long_and_PASS);
        appDemoForbidenSuitesAdd(enhancedUt_forbidenSuites_Emulator_Killer);
        appDemoForbidenSuitesAdd(mainUt_forbidenSuites_Emulator_skipped);

        {
            GT_U32 skip = 0;
            appDemoDbEntryGet("onEmulatorSkipSlowEnhancedUt", &skip);
            if (skip)
            {
                appDemoForbidenTestsAdd(
                    enhancedUt_forbidenTests_Emulator_LongerThanOneMinute);
            }
        }
    }
#endif /*IMPL_TGF*/

    /* add tests that defined forbidden by Falcon */
    return appDemoFalconMainUtForbidenTests();
}

/**
* @internal ironman_localUtfInit function
* @endinternal
*
*/
static GT_STATUS ironman_localUtfInit
(
    IN GT_SW_DEV_NUM      devNum
)
{
    GT_STATUS rc = GT_OK;

#ifdef INCLUDE_UTF

    /* Initialize unit tests for CPSS */
    rc = utfPreInitPhase();
    if (rc != GT_OK)
    {
        utfPostInitPhase(rc);
        return rc;
    }

    rc = utfInit(CAST_SW_DEVNUM(devNum));
    if (rc != GT_OK)
    {
        CPSS_ENABLER_DBG_TRACE_RC_MAC("utfInit", rc);
        utfPostInitPhase(rc);
        return rc;
    }

    utfPostInitPhase(rc);
#else
    devNum = devNum;
#endif /* INCLUDE_UTF */

    appDemoIronmanMainUtForbidenTests();

    return rc;
}

extern GT_STATUS prvCpssHwInitSip5IsInitTcamDefaultsDone(IN GT_U8   devNum, GT_BOOL tcamDaemonEnable);
extern GT_U32 prvCpssDxChHwIsUnitUsed_Ironman_Emulator_Bypass_TCAM_get(void);

static const APP_DEMO_LED_UNIT_CONFIG_STC ironman_led_indications[CPSS_LED_UNITS_MAX_NUM_CNS] =
{
    {64, 66,    GT_FALSE},     /* LED ports (LED Unit 0 - Ports   0,  4, 52)      */
    {64, 66,    GT_FALSE},     /* LED ports (LED Unit 1 - Ports   8, 12, 53)      */
    {64, 66,    GT_FALSE},     /* LED ports (LED Unit 2 - Ports  16, 20, 54)      */
    {64, 67,    GT_FALSE}      /* LED ports (LED Unit 3 - CPU Ports 48-51)        */
};

/* IM-M RD board use serial LED for 4 XG ports from last LED server */
static const APP_DEMO_LED_UNIT_CONFIG_STC ironman_M_RD_led_indications[CPSS_LED_UNITS_MAX_NUM_CNS] =
{
    { 0,  0, GT_TRUE},     /* LED ports (LED Unit 0 - bypass)      */
    { 0,  0, GT_TRUE},     /* LED ports (LED Unit 1 - bypass)      */
    { 0,  0, GT_TRUE},     /* LED ports (LED Unit 2 - bypass)      */
    {64, 67, GT_FALSE}     /* LED ports (LED Unit 3 - XG Ports 48-51)        */
};

/* IM-L RD board use serial LED for 6 XG ports from LED servers */
static const APP_DEMO_LED_UNIT_CONFIG_STC ironman_L_RD_led_indications[CPSS_LED_UNITS_MAX_NUM_CNS] =
{
    {65, 66,    GT_FALSE},     /* LED ports (LED Unit 0 - Ports   0,  4, 52)      */
    {66, 66,    GT_FALSE},     /* LED ports (LED Unit 1 - Ports   8, 12, 53)      */
    { 0,  0,    GT_TRUE},      /* LED ports (LED Unit 2 - Ports  16, 20, 54)      */
    {64, 67,    GT_FALSE}      /* LED ports (LED Unit 3 - CPU Ports 48-51)        */
};

static const APP_DEMO_LED_PORT_MAC_POSITION_STC ironman_single_port_per_lane_ports_map[] =
{
    { 0,  0},  { 4,  1}, {52,  2},          /* LED server 0 */
    { 8,  0},  {12,  1}, {53,  2},          /* LED server 1 */
    {16,  0},  {20,  1}, {54,  2},          /* LED server 2 */
    {48,  0},  {49,  1}, {50,  2}, {51, 3}  /* LED server 3 */
};

#define APP_DEMO_LED_PORT_REORDER_MAP_SIZE_CNS  13

/**
* @internal led_port_position_get_mac
* @endinternal
*
* @brief   Gets LED ports position for the current MAC port
*
* @param[in] portMacNum            - current MAC to check
*
* @retval GT_U32                   - the LED ports position for the current MAC port
*
*/
static GT_U32 led_port_position_get_mac
(
    IN  GT_U32 portMacNum
)
{
    GT_U32 ledPosition = APPDEMO_BAD_VALUE;
    const APP_DEMO_LED_PORT_MAC_POSITION_STC * ironmanPortsMapArrPtr;
    GT_U32 ironmanPortsMapArrLength;
    GT_U32 ii;

    ironmanPortsMapArrPtr = &ironman_single_port_per_lane_ports_map[0];
    ironmanPortsMapArrLength = sizeof(ironman_single_port_per_lane_ports_map) / sizeof(ironman_single_port_per_lane_ports_map[0]);

    for (ii = 0; ii < ironmanPortsMapArrLength; ii++)
    {
        if (ironmanPortsMapArrPtr[ii].macPortNumber == portMacNum)
        {
            ledPosition = ironmanPortsMapArrPtr[ii].ledPortPosition;
            break;
        }
    }

    return ledPosition;
}

/**
* @internal ironman_LedInit function
* @endinternal
*
* @brief   LED configurations
*
* @param[in] devNum                - device number
* @param[in] boardRevId            - board revision ID
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise
*
*/
static GT_STATUS ironman_LedInit
(
    IN  GT_U8    devNum,
    IN  GT_U8    boardRevId
)
{
    GT_STATUS                       rc;
    GT_U32                          classNum;
    GT_PHYSICAL_PORT_NUM            portNum;
    CPSS_LED_CONF_STC               ledConfig;
    CPSS_LED_CLASS_MANIPULATION_STC ledClassManip;
    GT_U32                          position;
    GT_U32                          ledUnit;
    CPSS_DXCH_PORT_MAP_STC          portMap;
    const APP_DEMO_LED_UNIT_CONFIG_STC * ledStreamIndication;
    GT_U32                          reorderMapArraySize;
    GT_U32                          ledSerialOutputBitMapArr[APP_DEMO_LED_PORT_REORDER_MAP_SIZE_CNS] =
                                                              /* 0  1  2  3  4  5  6  7  8  9  10  11  12 */
                                                                {8, 1, 6, 3, 4, 2, 5, 0, 7, 9, 10, 11, 12};
    CPSS_LED_PARALLEL_CONFIG_STC  ledParallelConfig =
    {
        GT_TRUE,
        {{0x11, 0}, {0x12, 0}, {0x13, 0}, {0x14, 0},
         { CPSS_LED_BIT_SELECTOR_NOT_USED_CNS, 0},
         { CPSS_LED_BIT_SELECTOR_NOT_USED_CNS, 0},
         { CPSS_LED_BIT_SELECTOR_NOT_USED_CNS, 0},
         { CPSS_LED_BIT_SELECTOR_NOT_USED_CNS, 0}}
    };

    cpssOsMemSet(&ledConfig, 0, sizeof(CPSS_LED_CONF_STC));

    ledConfig.ledOrganize                       = CPSS_LED_ORDER_MODE_BY_CLASS_E;
    ledConfig.sip6LedConfig.ledClockFrequency   = 1000;
    ledConfig.pulseStretch                      = CPSS_LED_PULSE_STRETCH_1_E;

    ledConfig.blink0Duration          = CPSS_LED_BLINK_DURATION_1_E;    /* 64 ms */
    ledConfig.blink0DutyCycle         = CPSS_LED_BLINK_DUTY_CYCLE_1_E;  /* 50%   */
    ledConfig.blink1Duration          = CPSS_LED_BLINK_DURATION_1_E;    /* 64 ms */
    ledConfig.blink1DutyCycle         = CPSS_LED_BLINK_DUTY_CYCLE_1_E;  /* 50%   */

    ledConfig.disableOnLinkDown       = GT_FALSE;  /* don't care , see led-class manipulation */
    ledConfig.clkInvert               = GT_FALSE;  /* don't care */
    ledConfig.class5select            = CPSS_LED_CLASS_5_SELECT_FIBER_LINK_UP_E;   /* don't care */
    ledConfig.class13select           = CPSS_LED_CLASS_13_SELECT_COPPER_LINK_UP_E; /* don't care */

    if (boardRevId == 5)
    {
        ledStreamIndication = ironman_L_RD_led_indications;
    }
    else if (boardRevId == 4)
    {
        ledStreamIndication = ironman_M_RD_led_indications;
    }
    else
    {
        ledStreamIndication = ironman_led_indications;
    }

    for(ledUnit = 0; ledUnit < PRV_CPSS_DXCH_PP_HW_INFO_LED_MAC(devNum).sip6LedUnitNum; ledUnit++)
    {
        /* Start of LED stream location - Class 2 */
        ledConfig.sip6LedConfig.ledStart[ledUnit] = ledStreamIndication[ledUnit].ledStart;    /* LedControl.LedStart[5:12] */
        /* End of LED stream location */
        ledConfig.sip6LedConfig.ledEnd[ledUnit] = ledStreamIndication[ledUnit].ledEnd;        /* LedControl.LedEnd[5:12] */
        /* LED unit may be bypassed in the LED chain */
        ledConfig.sip6LedConfig.ledChainBypass[ledUnit] = ledStreamIndication[ledUnit].bypassLedUnit;
    }

    rc = cpssDxChLedStreamConfigSet(devNum, 0, &ledConfig);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChLedStreamConfigSet", rc);
    if(rc != GT_OK)
    {
        return rc;
    }

    classNum = 2;
    ledClassManip.invertEnable            = GT_FALSE; /* not relevant for SIP_5/6 */
    ledClassManip.blinkEnable             = GT_TRUE;
    ledClassManip.blinkSelect             = CPSS_LED_BLINK_SELECT_0_E;
    ledClassManip.forceEnable             = GT_FALSE;
    ledClassManip.forceData               = 0;
    ledClassManip.pulseStretchEnable      = GT_TRUE;

    /* activate disableOnLinkDown as part of "Link and Activity" indication */
    ledClassManip.disableOnLinkDown       = GT_TRUE;

    rc = cpssDxChLedStreamClassManipulationSet(devNum, 0,
                                               CPSS_DXCH_LED_PORT_TYPE_XG_E /* don't care , not applicable for Ironman */
                                               ,classNum ,&ledClassManip);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChLedStreamClassManipulationSet", rc);
    if(rc != GT_OK)
    {
        return rc;
    }
    /*--------------------------------------------------------------------------------------------------------*
     * LED. classesAndGroupConfig uses default setting , don't configure   cpssDxChLedStreamGroupConfigSet()  *
     *--------------------------------------------------------------------------------------------------------*
     * now configure port dependent LED configurations                                                        *
     *       - port led position (stream is hard wired)                                                       *
     *           The port LED positions for full chip is as follows:                                          *
     *                                                                                                        *
     *           P0,    LED Server 0,  Position  0   Ports  0- 7                                              *
     *           P1,    LED Server 0,  Position  1   Ports 24-31                                              *
     *           P2,    LED Server 0,  Position  2   Port  52                                                 *
     *           P3,    LED Server 1,  Position  0   Ports  8-15                                              *
     *           P4,    LED Server 1,  Position  1   Ports 32-39                                              *
     *           P5,    LED Server 1,  Position  2   Port  53                                                 *
     *           P6,    LED Server 2,  Position  0   Ports 16-23                                              *
     *           P7,    LED Server 2,  Position  1   Ports 40-47                                              *
     *           P8,    LED Server 2,  Position  2   Port  54                                                 *
     *           P9,    LED Server 3,  Position  0   Port  48                                                 *
     *           P10,   LED Server 3,  Position  1   Port  49                                                 *
     *           P11,   LED Server 3,  Position  2   Port  50                                                 *
     *           P12,   LED Server 3,  Position  3   Port  51                                                 *
     *                                                                                                        *
     *--------------------------------------------------------------------------------------------------------*/
    for(portNum = 0; portNum < (appDemoPpConfigList[devNum].maxPortNumber); portNum++)
    {
        CPSS_ENABLER_PORT_SKIP_CHECK(devNum, portNum);

        if (prvCpssDxChPortRemotePortCheck(devNum,portNum))
        {
            continue;
        }

        rc = cpssDxChPortPhysicalPortMapGet(devNum, portNum, 1, /*OUT*/&portMap);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChPortPhysicalPortMapGet", rc);
        if(rc != GT_OK)
        {
            continue;
        }

        position = led_port_position_get_mac(portMap.interfaceNum);

        if (position != APPDEMO_BAD_VALUE)
        {
            rc = cpssDxChLedStreamPortPositionSet(devNum, portNum, position);
            CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChLedStreamPortPositionSet", rc);
            if(rc != GT_OK)
            {
                return rc;
            }
        }
    }

    reorderMapArraySize = APP_DEMO_LED_PORT_REORDER_MAP_SIZE_CNS;
    rc = cpssDxChLedStreamReorderMapSet(devNum, GT_TRUE, reorderMapArraySize, ledSerialOutputBitMapArr);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChLedStreamPortPositionSet", rc);

    if(boardRevId == 3)
    {
        /* Parallel LED interface configurations */
        rc = cpssDxChLedParallelConfigSet(devNum, &ledParallelConfig);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChLedParallelConfigSet", rc);
    }

    return rc;
}

/**
* @internal gtDbIronmanBoardReg_SimpleInit function
* @endinternal
*
* @brief   This is the 'simple' board initialization function for Ironman device.
*
* @param[in] boardRevId               - Board revision Id.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*
* @note This is a simplified version of cpssInitSystem.
*
*/
static GT_STATUS gtDbIronmanBoardReg_SimpleInit
(
    IN  GT_U8  boardRevId
)
{
    GT_STATUS   rc;
    GT_U32      start_sec  = 0;
    GT_U32      start_nsec = 0;
    GT_U32      end_sec  = 0;
    GT_U32      end_nsec = 0;
    GT_U32      diff_sec;
    GT_U32      diff_nsec;
    CPSS_HW_INFO_STC   hwInfo[APP_DEMO_CPSS_MAX_NUM_PORT_GROUPS_CNS];
    GT_SW_DEV_NUM           devNum;
    GT_U32      devIndex;/* device index in the array of appDemoPpConfigList[devIndex] */
    GT_PCI_INFO pciInfo;
    GT_U32      boardType;

    GT_U32  secondsStart, secondsEnd,
            nanoSecondsStart, nanoSecondsEnd,
            seconds, nanoSec; /* time of init */

    rc = cpssOsTimeRT(&secondsStart,&nanoSecondsStart);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssOsTimeRT", rc);
    if(rc != GT_OK)
        return rc;

#if (defined ASIC_SIMULATION) &&  (defined RTOS_ON_SIM)
    /*simulation initialization*/
    appDemoRtosOnSimulationInit();

#endif /*(defined ASIC_SIMULATION) &&  (defined RTOS_ON_SIM)*/

#ifdef ASIC_SIMULATION
    /* allow setting clock after the SOFT-RESET during the 'system reset'
       and 'system init' */
    if(simCoreClockOverwrittenGet())
    {
        simCoreClockOverwrite(simCoreClockOverwrittenGet());
    }
#endif

    /* Enable printing inside interrupt routine - supplied by extrernal drive */
    extDrvUartInit();

    /* Call to fatal_error initialization, use default fatal error call_back - supplied by mainOs */
    rc = osFatalErrorInit((FATAL_FUNC_PTR)NULL);
    if (rc != GT_OK)
        return rc;

    /* this function finds all Prestera devices on PCI bus */
    rc = ironman_getBoardInfoSimple(
            &hwInfo[0],
            &pciInfo);
    if (rc != GT_OK)
        return rc;

    /* give 'hint' to the CPSS that the device is of Ironman type .
       so it will know to access the MG registers in proper offset ...
       to read register 0x4c to get the 'deviceId'
    */
    prvCpssDrvHwPpPrePhase1NextDevFamilySet(CPSS_PP_FAMILY_DXCH_IRONMAN_E);

    appDemoPpConfigDevAmount = 1;
    devIndex = SYSTEM_DEV_NUM_MAC(0);
    devNum =   devIndex;

    /* take time from the 'phase1 init' stage (not including the 'PCI scan' operations) */
    cpssOsTimeRT(&start_sec, &start_nsec);

    switch(boardRevId)
    {
        case 1:
            boardType = APP_DEMO_IRONMAN_A0_DB_IM_L_SFP_ID;
            ironman_boardTypePrint("DB" /*boardName*/, "IronmanL_SFP_MODE" /*devName*/);
            break;
        case 2:
            boardType = APP_DEMO_IRONMAN_A0_DB_IM_L_PHY_ID;
            ironman_boardTypePrint("DB" /*boardName*/, "IronmanL_PHY_MODE" /*devName*/);
            break;
        case 3:
            boardType = APP_DEMO_IRONMAN_A0_DB_IM_M_ID;
            ironman_boardTypePrint("DB" /*boardName*/, "IronmanM" /*devName*/);
            break;
        case 4:
            boardType = APP_DEMO_IRONMAN_A0_RD_IM_DX1508M_8G4XG_ID;
            ironman_boardTypePrint("RD 8G 4XG" /*boardName*/, "IronmanM" /*devName*/);
            break;
        case 5:
            boardType = APP_DEMO_IRONMAN_A0_RD_IM_DX1518M_24G6XG_ID;
            ironman_boardTypePrint("RD 24G 6XG" /*boardName*/, "IronmanL" /*devName*/);
            break;
        default:
            return GT_BAD_PARAM;
    }

    userForceBoardType(boardType);
    rc = appDemoDxChBoardTypeGet(CAST_SW_DEVNUM(devNum), boardRevId, &bc2BoardType);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = ironman_appDemoInitSequence(boardRevId, devIndex, devNum, hwInfo);
    if (rc != GT_OK)
        return rc;

    rc = ironman_LedInit(devNum, boardRevId);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("ironman_LedInit", rc);
    if (rc != GT_OK)
        return rc;

    if (appDemoInitRegDefaults != GT_FALSE)
    {
        return GT_OK;
    }

    if(ezbIsXmlLoaded())
    {
        /* check if we need to init the MPD to support phys */
        rc = appDemoEzbMpdPpPortInit();
        CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoEzbMpdPpPortInit", rc);
        if (rc != GT_OK)
            return rc;
    }

    /* init the event handlers */
    rc = ironman_EventHandlerInit(devNum);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("ironman_EventHandlerInit", rc);
    if (rc != GT_OK)
        return rc;

    /* Data Integrity module initialization. It should be done after events init. */
    rc = appDemoDataIntegrityInit(devNum);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoDataIntegrityInit", rc);
    if(GT_OK != rc)
    {
        return rc;
    }

    rc = ironman_localUtfInit(devNum);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("ironman_localUtfInit", rc);
    if (rc != GT_OK)
        return rc;

    /* CPSSinit time measurement */
    rc = cpssOsTimeRT(&secondsEnd,&nanoSecondsEnd);
    if(rc != GT_OK)
    {
        return rc;
    }

    seconds = secondsEnd-secondsStart;
    if(nanoSecondsEnd >= nanoSecondsStart)
    {
        nanoSec = nanoSecondsEnd-nanoSecondsStart;
    }
    else
    {
        nanoSec = (1000000000 - nanoSecondsStart) + nanoSecondsEnd;
        seconds--;
    }
    cpssOsPrintf("cpssInitSystem time: %d sec., %d nanosec.\n", seconds, nanoSec);


    cpssOsTimeRT(&end_sec, &end_nsec);
    if(end_nsec < start_nsec)
    {
        end_nsec += 1000000000;
        end_sec  -= 1;
    }
    diff_sec  = end_sec  - start_sec;
    diff_nsec = end_nsec - start_nsec;

    cpssOsPrintf("Time processing the cpssInitSimple (from 'phase1 init') is [%d] seconds + [%d] nanoseconds \n" , diff_sec , diff_nsec);

    systemInitialized = GT_TRUE;


    /* allow interrupts / appDemo tasks to stable */
    osTimerWkAfter(500);

    return rc;
}

/**
* @internal gtDbIronmanBoardReg_BoardCleanDbDuringSystemReset function
* @endinternal
*
* @brief   clear the DB of the specific board config file , as part of the 'system rest'
*         to allow the 'cpssInitSystem' to run again as if it is the first time it runs
* @param[in] boardRevId               - The board revision Id.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS gtDbIronmanBoardReg_BoardCleanDbDuringSystemReset
(
    IN  GT_U8   boardRevId
)
{
    UNUSED_PARAM_MAC(boardRevId);

    appDemo_PortsInitList_already_done = 0;
    actualNum_cpssApi_ironman_defaultMap = 0;

    return GT_OK;
}

/**
* @internal gtDbDxIronmanBoardReg function
* @endinternal
*
* @brief   Registration function for the Ironman board .
*
* @param[in] boardRevId               - The board revision Id.
*
* @param[out] boardCfgFuncs            - Board configuration functions.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS gtDbDxIronmanBoardReg
(
    IN  GT_U8                   boardRevId,
    OUT GT_BOARD_CONFIG_FUNCS   *boardCfgFuncs
)
{
    UNUSED_PARAM_MAC(boardRevId);

    /* set the actual pointer to use for this board */
    boardCfgFuncs->boardSimpleInit               = gtDbIronmanBoardReg_SimpleInit;
    boardCfgFuncs->boardCleanDbDuringSystemReset = gtDbIronmanBoardReg_BoardCleanDbDuringSystemReset;

    return GT_OK;
}

extern void trace_ADDRESS_NOT_SUPPORTED_MAC_set(GT_U32 enable);
extern GT_STATUS internal_onEmulator(GT_BOOL doInitSystem);
static GT_U32 ironman_emulator_deviceId = CPSS_98DXA000_CNS;/* use value recognized by the CPSS ! */
/* function to be called before ironman_onEmulator_prepare to allow other/ZERO the 'ironman_emulator_deviceId' */
GT_STATUS ironman_emulator_deviceId_set(GT_U32   deviceId)
{
    ironman_emulator_deviceId = deviceId;
    return GT_OK;
}
extern void trace_ADDRESS_NOT_SUPPORTED_MAC_set(GT_U32 enable);
extern GT_STATUS appDemoTrace_skipTrace_onAddress(
    IN GT_U32   address,
    IN GT_U32   index /* up to 16 indexes supported 0..15 */
);

extern GT_STATUS debugEmulatorTimeOutSet
(
    GT_U32 timeOutType,
    GT_U32 timeOutValue
);

extern GT_STATUS cnc_onEmulator_extremely_slow_emulator_set(GT_U32   timeToSleep);
#ifdef NOKM_DRV_EMULATE_INTERRUPTS
extern void NOKM_DRV_EMULATE_INTERRUPTS_pollingSleepTime_Set(GT_U32 new_milisec_pollingSleepTime);
#endif


GT_STATUS ironman_onEmulator_prepare(GT_VOID)
{
    GT_STATUS rc;
    rc =  internal_onEmulator(GT_FALSE/*without cpssInitSystem*/);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* override 200 millisec set by internal_onEmulator */
#ifdef NOKM_DRV_EMULATE_INTERRUPTS
    NOKM_DRV_EMULATE_INTERRUPTS_pollingSleepTime_Set(160/*160 mills*/);/*do 6 iterations in second*/
#endif /*NOKM_DRV_EMULATE_INTERRUPTS*/

    /* the 'NO KN' is doing polling in this interrupt global cause register */
    /* but this polling should not be seen during 'trace'                   */
    appDemoTrace_skipTrace_onAddress(PRV_CPSS_IRONMAN_MG0_BASE_ADDRESS_CNS + 0x30 /*0x7F900030*//*address*/,0/*index*/);

    /*
        allow to see Addresses that CPSS not access because forbidden in the Emulator
    */
    trace_ADDRESS_NOT_SUPPORTED_MAC_set(1);

    /* CPSS for emulator is stable. LOG is not needed for common testing. */
#ifdef CPSS_LOG_ENABLE
    /* disable CPSS LOG */
    cpssLogEnableSet (0);
    /* disable LOG for ERRORs */
    cpssLogLibEnableSet (CPSS_LOG_LIB_ALL_E,CPSS_LOG_TYPE_ERROR_E,0);
#endif /*CPSS_LOG_ENABLE*/

    appDemoPrintLinkChangeFlagSet(GT_TRUE);

    if(ironman_emulator_deviceId)/* assign only if not ZERO */
    {
        appDemoDebugDeviceIdSet(0, ironman_emulator_deviceId);
    }

#ifdef IMPL_TGF
#ifdef INCLUDE_UTF
    /* in the cnc test with fdb upload :
       1. the device get to the fdb entry after some time. so we need to help it ... to wait more than usual
       2. the CNC upload takes longer than usual.
       influence the next 2 tests :
       do shell-execute utfTestsRun "cpssDxChCnc.cpssDxChCncBlockUploadTrigger_cncOverflowAndCombineWithFu",1,1
       do shell-execute utfTestsRun "cpssDxChCnc.cpssDxChCncBlockUploadTrigger_fullQueue",1,1
    */
    cnc_onEmulator_extremely_slow_emulator_set(3000);
#endif /* INCLUDE_UTF */
#endif /* IMPL_TGF */

    {
        GT_U32 debug_sip6_TO_txq_polling_interval = 10;/*orig 200*/
        GT_U32 debug_sip6_TO_after_txq_drain      = 10;/*orig 500*/
        GT_U32 debug_sip6_TO_after_mac_disabled   = 10;/*orig 500*/

        /* use values that used before Phoenix/Hawk added USX ports */
        debugEmulatorTimeOutSet(1,debug_sip6_TO_txq_polling_interval);
        debugEmulatorTimeOutSet(3,debug_sip6_TO_after_txq_drain);
        debugEmulatorTimeOutSet(4,debug_sip6_TO_after_mac_disabled);
    }

    return GT_OK;
}


GT_STATUS appDemoIronManTempGet
(
    IN GT_U8 devNum
)
{
#define  TSEN_SAMPLES_PER_SENSOR_CNS 3
    CPSS_DXCH_DIAG_TEMPERATURE_SENSOR_ENT sensor;
    GT_U32 sample;
    GT_32 maxTemp = -1000;
    GT_32 currTempArr[CPSS_DXCH_DIAG_TEMPERATURE_SENSOR_3_E][TSEN_SAMPLES_PER_SENSOR_CNS];
    GT_STATUS rc;
    GT_32 maxTempArr[CPSS_DXCH_DIAG_TEMPERATURE_SENSOR_3_E]={-1000,-1000,-1000};
    GT_32 averTempArr[CPSS_DXCH_DIAG_TEMPERATURE_SENSOR_3_E]={0,0,0};

    for (sample = 0; sample < TSEN_SAMPLES_PER_SENSOR_CNS; sample++)
    {
        /* temperature of each sensor */
        for (sensor = 0 ; sensor < CPSS_DXCH_DIAG_TEMPERATURE_SENSOR_3_E ; sensor++)
        {
            rc = cpssDxChDiagDeviceTemperatureSensorsSelectSet(devNum, sensor);
            if(rc != GT_OK)
            {
                cpssOsPrintf("cpssDxChDiagDeviceTemperatureSensorsSelectSet failed\n");
                return rc;
            }

            osTimerWkAfter(2);

            rc = cpssDxChDiagDeviceTemperatureGet(devNum, &currTempArr[sensor][sample]);
            if(rc != GT_OK)
            {
                cpssOsPrintf("cpssDxChDiagDeviceTemperatureGet failed \n");
                return rc;
            }

            if (currTempArr[sensor][sample] > maxTemp)
            {
                maxTemp = currTempArr[sensor][sample];
            }


            if (currTempArr[sensor][sample] > maxTempArr[sensor])
            {
                maxTempArr[sensor] = currTempArr[sensor][sample];
            }

            averTempArr[sensor] += currTempArr[sensor][sample];
        }
    }

    cpssOsPrintf(" Sample | Sen 0 | Sen 1 | Sen 2 |\n");
    cpssOsPrintf("--------|-------|-------|-------|\n");
    for (sample = 0; sample < TSEN_SAMPLES_PER_SENSOR_CNS; sample++)
    {
        cpssOsPrintf("   %d    | %4d  | %4d  | %4d  |\n",sample, currTempArr[0][sample],currTempArr[1][sample],currTempArr[2][sample]);
    }

    cpssOsPrintf("  Max   | %4d  | %4d  | %4d  |\n",maxTempArr[0],maxTempArr[1],maxTempArr[2]);
    cpssOsPrintf("  Aver  | %4d  | %4d  | %4d  |\n",averTempArr[0]/TSEN_SAMPLES_PER_SENSOR_CNS,averTempArr[1]/TSEN_SAMPLES_PER_SENSOR_CNS,averTempArr[2]/TSEN_SAMPLES_PER_SENSOR_CNS);

    cpssOsPrintf("Maximal temperature %d C\n",maxTemp);
    return GT_OK;
}

