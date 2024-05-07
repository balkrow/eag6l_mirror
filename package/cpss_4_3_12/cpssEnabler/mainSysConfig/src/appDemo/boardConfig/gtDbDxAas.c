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
* @file gtDbDxAas.c
*
* @brief Initialization functions for the Aas - SIP7.0- board.
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
#include <gtExtDrv/drivers/gtUartDrv.h>
#ifdef INCLUDE_UTF
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#endif /* INCLUDE_UTF */
#include <gtOs/gtOsExc.h>

#include <cpss/dxCh/dxChxGen/port/cpssDxChPortManager.h>
#include <cpssDriver/pp/config/prvCpssDrvPpCfgInit.h>
#include <appDemo/utils/mainUT/appDemoUtils.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwInit.h>
#include <cpss/dxCh/dxChxGen/cscd/cpssDxChCscd.h>
#include <cpss/dxCh/dxChxGen/cos/cpssDxChCos.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cpss/common/init/cpssInit.h>/* needed for : cpssPpWmDeviceInAsimEnvSet() */
#include <gtOs/globalDb/gtOsGlobalDb.h>
#include <extUtils/trafficEngine/tgfTrafficEngine.h>
#include <gtOs/gtOsSharedPp.h>
#include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIfGdma.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwAasAddrSpace.h>

/* indication for the ASIM project */
static GT_U8       last_boardRevId = 0;

/* offset used during HW device ID calculation formula */
extern GT_U8 appDemoHwDevNumOffset;
/* cpssInitSystem has been run */
extern GT_BOOL systemInitialized;
/* port manager boolean variable */
extern GT_BOOL portMgr;

/* indication that during initialization the call to appDemoBc2PortListInit(...)
   was already done so no need to call it for second time. */
extern GT_U32  appDemo_PortsInitList_already_done;

extern CPSS_DXCH_PORT_MAP_STC  *appDemoPortMapPtr;
/* number of elements in appDemoPortMapPtr */
extern GT_U32                  appDemoPortMapSize;

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
    arrLen = NUM_ELEMENTS_IN_ARR_MAC(myArr)

#define UNUSED_PARAM_MAC(x) x = x

#define SPECIAL_PORT_GLOBAL_DMA_BASE    80  /* CPU and LOOPBACK - Special-ports' BASE global(~SW) dma number */

#define CPU_PORT                    CPSS_CPU_PORT_NUM_CNS /*63*/
#define FROM_CPU_PORT_NUM           127
#define CPU_GDMA_NUM                4
#define CPU_GDMA_NUM_2TILES         3

#define LOOPBACK_PHYSICAL_PORT(_dp_)    (128 + _dp_) /* number chosen for loopback-port (one per DataPath) */
#define LOOPBACK_DMA(_dp_)              (SPECIAL_PORT_GLOBAL_DMA_BASE + CPU_GDMA_NUM + _dp_)

#define DEFAULT_QUEUE_NUM         8
#define DEFAULT_LB_QUEUE_NUM       64



#define DEFAULT_PREEMPTION_ENABLE         GT_FALSE




#define HQOS_ARR_UNAWARE NULL


extern GT_STATUS appDemoTrace_skipTrace_onAddress(
    IN GT_U32   address,
    IN GT_U32   index /* up to 16 indexes supported 0..15 */
);

#define DEV_TYPE_TO_2_FIELDS_MAC(devType)    {(GT_U16)(devType)/*vendorId*/ , (GT_U16)((devType) >> 16) /*devId*/}

/* define the ability to use limited list and not query the BUS for ALL historical / future devices */
/* if this pointer need to be used the caller must fill the vendorId with 0x11AB (GT_PCI_VENDOR_ID) */
/* and LAST entry MUST be 'termination' by vendorId = 0  !!*/
extern GT_PCI_DEV_VENDOR_ID    *limitedDevicesListPtr;

#define PORT_NUMBER_ARR_SIZE    22
typedef struct
{
    GT_PHYSICAL_PORT_NUM                portNumberArr  [PORT_NUMBER_ARR_SIZE];
    GT_U32                              globalDmaNumber[PORT_NUMBER_ARR_SIZE];
    CPSS_DXCH_PORT_MAPPING_TYPE_ENT     mappingType;
    GT_U32                              numberOfQueuesPerPort;
    GT_BOOL                             reservePreemptiveChannel;
    /*hqosId[port0,level0],hqosId[port0,level1],
            hqosId[port1,level0],hqosId[port1,level1],....and etc*/
    GT_U32                              *hqosIdArr;

}AAS_PORT_MAP_STC;

GT_BOOL appDemoUseVPortMode = GT_FALSE;
GT_U32  learnPriority_minimalVPortNumber = 0;
GT_U32  learnPriority_numberOfVPorts = 0;

/* mapping of AAS physical ports to MAC ports (and TXQ ports). */
static AAS_PORT_MAP_STC aas_port_mode[] =
{
    /* DP[0] ports */
     {{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, /*16, 17, 18, 19,*/ GT_NA} ,
      { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, /*16, 17, 18, 19,*/ GT_NA} ,
        CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DEFAULT_QUEUE_NUM,GT_TRUE,HQOS_ARR_UNAWARE
      }

    /* DP[1] ports */
    ,{{ 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, /*36, 37, 38, 39,*/ GT_NA} ,
      { 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, /*36, 37, 38, 39,*/ GT_NA} ,
        CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DEFAULT_QUEUE_NUM,GT_TRUE,
        HQOS_ARR_UNAWARE
      }

    /* DP[2] ports */
    ,{{ 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, /*56, 57, 58, 59,*/ GT_NA} ,
      { 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, /*56, 57, 58, 59,*/ GT_NA} ,
      CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DEFAULT_QUEUE_NUM,GT_TRUE,
      HQOS_ARR_UNAWARE}

    /* DP[3] ports */
    ,{{ 60, 61, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, /*76, 77, 78, 79,*/ GT_NA} ,
      { 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, /*76, 77, 78, 79,*/ GT_NA} ,
      CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DEFAULT_QUEUE_NUM,GT_TRUE,
      HQOS_ARR_UNAWARE}

     /* skip physical 62 */
    ,{{CPU_PORT/*63*/          ,GT_NA} , { 81/*DMA-MG0*/          ,GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,DEFAULT_QUEUE_NUM,GT_FALSE,
      HQOS_ARR_UNAWARE}

    /* Internal Loopback Port for DP[0],..,DP[n] */
    ,{{LOOPBACK_PHYSICAL_PORT(0), LOOPBACK_PHYSICAL_PORT(1), LOOPBACK_PHYSICAL_PORT(2), LOOPBACK_PHYSICAL_PORT(3), GT_NA} ,
      {LOOPBACK_DMA(0), LOOPBACK_DMA(1), LOOPBACK_DMA(2), LOOPBACK_DMA(3), GT_NA} ,
      CPSS_DXCH_PORT_MAPPING_TYPE_LOOPBACK_E, DEFAULT_QUEUE_NUM, GT_FALSE,
      HQOS_ARR_UNAWARE}
};
static GT_U32   actualNum_aas_port_mode = NUM_ELEMENTS_IN_ARR_MAC(aas_port_mode);


/* mapping of AAS physical ports to MAC ports (and TXQ ports). */
static AAS_PORT_MAP_STC aas_port_mode_3[] =
{
    /* DP[0] ports */
     {{ 0,  2,  4,  6,  8,  10,  12,  14,  GT_NA} ,
      { 0,  2,  4,  6,  8,  10,  12,  14,  GT_NA} ,
        CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DEFAULT_QUEUE_NUM,GT_TRUE,HQOS_ARR_UNAWARE
      }

    /* DP[1] ports */
    ,{{ 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, /*36, 37, 38, 39,*/ GT_NA} ,
      { 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, /*36, 37, 38, 39,*/ GT_NA} ,
        CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DEFAULT_QUEUE_NUM,GT_TRUE,
        HQOS_ARR_UNAWARE
      }

    /* DP[2] ports */
    ,{{ 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, /*56, 57, 58, 59,*/ GT_NA} ,
      { 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, /*56, 57, 58, 59,*/ GT_NA} ,
      CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DEFAULT_QUEUE_NUM,GT_TRUE,
      HQOS_ARR_UNAWARE}

    /* DP[3] ports */
    ,{{ 60, 61, /*62, 63,*/ 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, /*76, 77, 78, 79,*/ GT_NA} ,
      { 60, 61, /*62, 63,*/ 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, /*76, 77, 78, 79,*/ GT_NA} ,
      CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DEFAULT_QUEUE_NUM,GT_TRUE,
      HQOS_ARR_UNAWARE}

     /* skip physical 62 */
    ,{{CPU_PORT/*63*/          ,GT_NA} , { 82/*DP1 : TO_CPU:GDMA1,2  */  ,GT_NA} ,
        CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,DEFAULT_QUEUE_NUM,GT_FALSE,HQOS_ARR_UNAWARE}
    ,{{FROM_CPU_PORT_NUM/*127*/,GT_NA} , { 83/*DP2 : FROM_CPU:GDMA3 */ ,GT_NA} ,
        CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,DEFAULT_QUEUE_NUM,GT_FALSE,HQOS_ARR_UNAWARE}

    /* Internal Loopback Port for DP[0],..,DP[n] */
    ,{{LOOPBACK_PHYSICAL_PORT(0), LOOPBACK_PHYSICAL_PORT(1), LOOPBACK_PHYSICAL_PORT(2), LOOPBACK_PHYSICAL_PORT(3), GT_NA} ,
      {LOOPBACK_DMA(0), LOOPBACK_DMA(1), LOOPBACK_DMA(2), LOOPBACK_DMA(3), GT_NA} ,
      CPSS_DXCH_PORT_MAPPING_TYPE_LOOPBACK_E, DEFAULT_QUEUE_NUM, GT_FALSE,
      HQOS_ARR_UNAWARE}
};
static GT_U32   actualNum_aas_port_mode_3 = NUM_ELEMENTS_IN_ARR_MAC(aas_port_mode_3);



/* mapping of AAS physical ports to MAC ports (and TXQ ports). */
static AAS_PORT_MAP_STC GDMA_aas_port_mode[] =
{
    /* DP[0] ports */
     {{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, /*16, 17, 18, 19,*/ GT_NA} ,
      { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, /*16, 17, 18, 19,*/ GT_NA} ,
        CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DEFAULT_QUEUE_NUM,DEFAULT_PREEMPTION_ENABLE,HQOS_ARR_UNAWARE
      }

    /* DP[1] ports */
    ,{{ 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, /*36, 37, 38, 39,*/ GT_NA} ,
      { 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, /*36, 37, 38, 39,*/ GT_NA} ,
        CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DEFAULT_QUEUE_NUM,DEFAULT_PREEMPTION_ENABLE,HQOS_ARR_UNAWARE
      }

    /* DP[2] ports */
    ,{{ 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, /*56, 57, 58, 59,*/ GT_NA} ,
      { 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, /*56, 57, 58, 59,*/ GT_NA} ,
      CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DEFAULT_QUEUE_NUM,DEFAULT_PREEMPTION_ENABLE,HQOS_ARR_UNAWARE}

    /* DP[3] ports */
    ,{{ 60, 61, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, /*76, 77, 78, 79,*/ GT_NA} ,
      { 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, /*76, 77, 78, 79,*/ GT_NA} ,
      CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DEFAULT_QUEUE_NUM,DEFAULT_PREEMPTION_ENABLE,HQOS_ARR_UNAWARE}

     /* skip physical 62 */
    ,{{CPU_PORT/*63*/          ,GT_NA} , { 82/*DP1 : TO_CPU:GDMA1,2  */  ,GT_NA} ,
        CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,DEFAULT_QUEUE_NUM,GT_FALSE,HQOS_ARR_UNAWARE}
    ,{{FROM_CPU_PORT_NUM/*127*/,GT_NA} , { 83/*DP2 : FROM_CPU:GDMA3 */ ,GT_NA} ,
        CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,DEFAULT_QUEUE_NUM,GT_FALSE,HQOS_ARR_UNAWARE}

    /* Internal Loopback Port for DP[0],..,DP[n] */
    ,{{LOOPBACK_PHYSICAL_PORT(0), LOOPBACK_PHYSICAL_PORT(1), LOOPBACK_PHYSICAL_PORT(2), LOOPBACK_PHYSICAL_PORT(3), GT_NA} ,
      {LOOPBACK_DMA(0), LOOPBACK_DMA(1), LOOPBACK_DMA(2), LOOPBACK_DMA(3), GT_NA} ,
      CPSS_DXCH_PORT_MAPPING_TYPE_LOOPBACK_E, DEFAULT_QUEUE_NUM, GT_FALSE,
      HQOS_ARR_UNAWARE}
};
static GT_U32   GDMA_actualNum_aas_port_mode = NUM_ELEMENTS_IN_ARR_MAC(GDMA_aas_port_mode);


/* mapping of AAS physical ports to MAC ports (and TXQ ports). */
static AAS_PORT_MAP_STC aas_2_tiles_port_mode[] =
{
/* tile 0 */
    /* DP[0] ports */
     {{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, GT_NA} ,
      { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, GT_NA} ,
        CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DEFAULT_QUEUE_NUM,DEFAULT_PREEMPTION_ENABLE,HQOS_ARR_UNAWARE}
    /* DP[1] ports */
    ,{{ 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, GT_NA} ,
      { 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, GT_NA} ,
        CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DEFAULT_QUEUE_NUM,DEFAULT_PREEMPTION_ENABLE,HQOS_ARR_UNAWARE}

    /* DP[2] ports */
    ,{{ 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, GT_NA} ,
      { 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, GT_NA} ,
      CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DEFAULT_QUEUE_NUM,DEFAULT_PREEMPTION_ENABLE,HQOS_ARR_UNAWARE}

    /* DP[3] ports */
    ,{{ 60, 61, /*62, 63,*/ 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, GT_NA} ,
      { 60, 61, /*62, 63,*/ 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, GT_NA} ,
      CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DEFAULT_QUEUE_NUM,DEFAULT_PREEMPTION_ENABLE,HQOS_ARR_UNAWARE}

/* tile 1 */
    /* DP[4] ports */
    ,{{ 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, GT_NA} ,
      { 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, GT_NA} ,
        CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DEFAULT_QUEUE_NUM,DEFAULT_PREEMPTION_ENABLE,HQOS_ARR_UNAWARE}
    /* DP[5] ports */
    ,{{ 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, GT_NA} ,
      { 100, 101, 102, 103, 104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, GT_NA} ,
        CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DEFAULT_QUEUE_NUM,DEFAULT_PREEMPTION_ENABLE,HQOS_ARR_UNAWARE}

    /* DP[6] ports */
    ,{{ 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, GT_NA} ,
      { 120, 121, 122, 123, 124, 125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, GT_NA} ,
      CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DEFAULT_QUEUE_NUM,DEFAULT_PREEMPTION_ENABLE,HQOS_ARR_UNAWARE}

    /* DP[7] ports */
    ,{{ 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, GT_NA} ,
      { 140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, GT_NA} ,
      CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DEFAULT_QUEUE_NUM,DEFAULT_PREEMPTION_ENABLE,HQOS_ARR_UNAWARE}

     /* skip physical 62 */
    ,{{CPU_PORT/*63*/          ,GT_NA} , {162/*DMA-MG0*/          ,GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,
        DEFAULT_QUEUE_NUM,GT_FALSE,HQOS_ARR_UNAWARE}
};

static GT_U32   actualNum_aas_2_tiles_port_mode = NUM_ELEMENTS_IN_ARR_MAC(aas_2_tiles_port_mode);

static GT_U32 hqosMappingDma0[]= {/*1*/0,0,/*2*/0,1,/*3*/0,2,/*4*/1,0,/*5*/1,1};
static GT_U32 hqosMappingDma1[]= {/*7*/0,0,/*8*/0,0,/*9*/0,2,/*10*/1,1,/*11*/1,1};


/* mapping of AAS physical ports to MAC ports (and TXQ ports). */
static AAS_PORT_MAP_STC aas_port_mode_hqos[] =
{
    /* DP[0] ports */
     {{ 0, GT_NA} ,
      { 0, GT_NA} ,
        CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DEFAULT_QUEUE_NUM,GT_FALSE,
        HQOS_ARR_UNAWARE
        },
     {{ 1,2,3,4,5,GT_NA} ,
      { 0,0,0,0,0,GT_NA} ,
        CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,DEFAULT_QUEUE_NUM,GT_FALSE,
        hqosMappingDma0
      },

    {{ 6, GT_NA} ,
      { 1, GT_NA} ,
        CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DEFAULT_QUEUE_NUM,GT_FALSE,
        HQOS_ARR_UNAWARE
     },
     {{ 7,8,9,10,11,GT_NA} ,
      { 1,1,1,1,1,GT_NA} ,
        CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,DEFAULT_QUEUE_NUM,GT_FALSE,
       hqosMappingDma1
      }
    /* DP[1] ports */
    ,{{ 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, GT_NA} ,
      { 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, GT_NA} ,
        CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DEFAULT_QUEUE_NUM,GT_TRUE,HQOS_ARR_UNAWARE
      }

    /* DP[2] ports */
    ,{{ 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, GT_NA} ,
      { 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, GT_NA} ,
      CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DEFAULT_QUEUE_NUM,GT_TRUE,HQOS_ARR_UNAWARE}

    /* DP[3] ports */
    ,{{ 60, 61, /*62, 63,*/ 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, GT_NA} ,
      { 60, 61, /*62, 63,*/ 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, GT_NA} ,
      CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DEFAULT_QUEUE_NUM,GT_TRUE,HQOS_ARR_UNAWARE
      }

     /* skip physical 62 */
    ,{{CPU_PORT/*63*/          ,GT_NA} , { 81/*DMA-MG0*/          ,GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,DEFAULT_QUEUE_NUM,GT_FALSE,
      HQOS_ARR_UNAWARE}

    /* Internal Loopback Port for DP[0],..,DP[n] */
    ,{{LOOPBACK_PHYSICAL_PORT(0), LOOPBACK_PHYSICAL_PORT(1), LOOPBACK_PHYSICAL_PORT(2), LOOPBACK_PHYSICAL_PORT(3), GT_NA} ,
      {LOOPBACK_DMA(0), LOOPBACK_DMA(1), LOOPBACK_DMA(2), LOOPBACK_DMA(3), GT_NA} ,
      CPSS_DXCH_PORT_MAPPING_TYPE_LOOPBACK_E, DEFAULT_QUEUE_NUM, GT_FALSE,
      HQOS_ARR_UNAWARE}
};

static GT_U32   actualNum_aas_port_mode_hqos = NUM_ELEMENTS_IN_ARR_MAC(aas_port_mode_hqos);

/*shell-execute appDemoDbEntryAdd "portModeExcelAllignedMapping",11*/

/* mapping of AAS physical ports to MAC ports (and TXQ ports). */
static AAS_PORT_MAP_STC aas_port_mode_11[] =
{
    /* DP[0] ports */
    {{ 0, GT_NA} , { 0, GT_NA} ,CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DEFAULT_QUEUE_NUM,GT_FALSE,
        HQOS_ARR_UNAWARE},

    {{ 1, GT_NA} , { 8, GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DEFAULT_QUEUE_NUM,GT_FALSE,
        HQOS_ARR_UNAWARE},

    {{ 2, GT_NA} , { 20, GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DEFAULT_QUEUE_NUM,GT_FALSE,
        HQOS_ARR_UNAWARE},

    {{ 3, GT_NA} , { 28, GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DEFAULT_QUEUE_NUM,GT_FALSE,
        HQOS_ARR_UNAWARE},

    {{ 4, GT_NA} , { 40, GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DEFAULT_QUEUE_NUM,GT_FALSE,
        HQOS_ARR_UNAWARE},

    {{ 5, GT_NA} , { 48, GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DEFAULT_QUEUE_NUM,GT_FALSE,
        HQOS_ARR_UNAWARE},

    {{ 6, GT_NA} , { 60, GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DEFAULT_QUEUE_NUM,GT_FALSE,
        HQOS_ARR_UNAWARE},

    {{ 7, GT_NA} , { 68, GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DEFAULT_QUEUE_NUM,GT_FALSE,
        HQOS_ARR_UNAWARE},

    /*special ports*/
    {{ 52, GT_NA} , { 80, GT_NA} , CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,DEFAULT_LB_QUEUE_NUM,GT_FALSE,
        HQOS_ARR_UNAWARE}

    ,{{CPU_PORT/*63*/          ,GT_NA} , { 82/*DP1 : TO_CPU:GDMA1,2  */  ,GT_NA} ,
        CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,DEFAULT_QUEUE_NUM,GT_FALSE,HQOS_ARR_UNAWARE}
    ,{{FROM_CPU_PORT_NUM/*127*/,GT_NA} , { 83/*DP2 : FROM_CPU:GDMA3 */ ,GT_NA} ,
        CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,DEFAULT_QUEUE_NUM,GT_FALSE,HQOS_ARR_UNAWARE}

    /* Internal Loopback Port for DP[0],..,DP[n] */
    ,{{LOOPBACK_PHYSICAL_PORT(0), LOOPBACK_PHYSICAL_PORT(1), LOOPBACK_PHYSICAL_PORT(2), LOOPBACK_PHYSICAL_PORT(3), GT_NA} ,
      {LOOPBACK_DMA(0), LOOPBACK_DMA(1), LOOPBACK_DMA(2), LOOPBACK_DMA(3), GT_NA} ,
      CPSS_DXCH_PORT_MAPPING_TYPE_LOOPBACK_E, DEFAULT_QUEUE_NUM, GT_FALSE,
      HQOS_ARR_UNAWARE}
};

static GT_U32   actualNum_aas_port_mode_11 = NUM_ELEMENTS_IN_ARR_MAC(aas_port_mode_11);

/* the number of CPU SDMAs that we need resources for */
static GT_U32   numCpuSdmas = 0;

#define MAX_MAPPED_PORTS_CNS    1024
static  GT_U32  actualNum_cpssApi_aas_defaultMap = 0;/*actual number of valid entries in cpssApi_aas_defaultMap[] */
static CPSS_DXCH_PORT_MAP_STC   cpssApi_aas_defaultMap[MAX_MAPPED_PORTS_CNS];
/* filled at run time , by info from aas_defaultMap[] */

static PortInitList_STC *aas_force_PortsInitListPtr = NULL;
static AAS_PORT_MAP_STC *aas_forcePortMapPtr = NULL;
static GT_U32 * force_actualNum_aas_port_modePtr = NULL;

#define AAS_BOARD_TYPE_DB_AAS               0
#define AAS_BOARD_TYPE_INVALID              1

static GT_U32 aasBoardType = AAS_BOARD_TYPE_INVALID;

/* Aas ports in 50G mode */
/* Each DP has BW 850G including CPU and Loopback ports */
/* default port list holds 16 * 50G                     */
static PortInitList_STC aas_portInitlist[] =
{
#if 0
     { PORT_LIST_TYPE_INTERVAL,  { 0,15,1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_50000_E,  CPSS_PORT_INTERFACE_MODE_KR_E}
    ,{ PORT_LIST_TYPE_INTERVAL,  {20,35,1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_50000_E,  CPSS_PORT_INTERFACE_MODE_KR_E}
    ,{ PORT_LIST_TYPE_INTERVAL,  {40,55,1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_50000_E,  CPSS_PORT_INTERFACE_MODE_KR_E}
    ,{ PORT_LIST_TYPE_INTERVAL,  {60,75,1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_50000_E,  CPSS_PORT_INTERFACE_MODE_KR_E}
#else /* set 400G ports as emulator need it */
     { PORT_LIST_TYPE_LIST,  {0,8,20,28,40,48,60,70, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_400G_E,  CPSS_PORT_INTERFACE_MODE_KR8_E}
#endif

    ,{ PORT_LIST_TYPE_EMPTY,     {APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,   CPSS_PORT_INTERFACE_MODE_QSGMII_E    }
};

/* Aas 2 tiles ports in 50G mode */
static PortInitList_STC aas_2_tiles_portInitlist[] =
{
     { PORT_LIST_TYPE_INTERVAL,  {0,159,1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_50000_E,  CPSS_PORT_INTERFACE_MODE_KR_E}

    ,{ PORT_LIST_TYPE_EMPTY,     {APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,   CPSS_PORT_INTERFACE_MODE_QSGMII_E    }
};

/* Aas HQOS mapping - ports in 50G mode */

static PortInitList_STC aas_hqosPortInitlist[] =
{
     { PORT_LIST_TYPE_LIST,  {0,6, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_50000_E,  CPSS_PORT_INTERFACE_MODE_KR_E},
     { PORT_LIST_TYPE_LIST,  {1,2,3,4,5,7,8,9,10,11, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_REMOTE_E,  CPSS_PORT_INTERFACE_MODE_REMOTE_E},
     { PORT_LIST_TYPE_INTERVAL,  {12,79,1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_50000_E,  CPSS_PORT_INTERFACE_MODE_KR_E}
    ,{ PORT_LIST_TYPE_EMPTY,     {APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,   CPSS_PORT_INTERFACE_MODE_QSGMII_E    }
};

static PortInitList_STC aas_portMode_11_Initlist[] =
{
     { PORT_LIST_TYPE_INTERVAL,  {0,6,1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_400G_E,  CPSS_PORT_INTERFACE_MODE_KR8_E},
     { PORT_LIST_TYPE_LIST,  {7, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_50000_E,  CPSS_PORT_INTERFACE_MODE_KR_E},
     { PORT_LIST_TYPE_EMPTY,     {APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,   CPSS_PORT_INTERFACE_MODE_QSGMII_E    }
};


static PortInitList_STC aas_portMode_3_Initlist[] =
{
     { PORT_LIST_TYPE_INTERVAL,  {0,14,1, APP_INV_PORT_CNS }, CPSS_PORT_SPEED_25000_E,  CPSS_PORT_INTERFACE_MODE_KR_E},
     { PORT_LIST_TYPE_EMPTY,     {APP_INV_PORT_CNS }, CPSS_PORT_SPEED_NA_E,   CPSS_PORT_INTERFACE_MODE_QSGMII_E    }
};


/**
* @internal aas_SpecialPortsInit function
* @endinternal
*
* @brief   Default config speed 25G and linkUp for all special Loopback ports
*/
static GT_STATUS aas_SpecialPortsInit
(
    IN  GT_U8   devNum
);

/**
* @internal aas_BindAllPortsToDefaultTdProfile function
* @endinternal
*
* @brief   Bind all ports and port queues to default tail drop profile.
*
* @param[in] devNum                - device number.
* @param[in] portDefaultProfile    - Port default tail drop profile.
* @param[in] queueDefaultProfile   - Queue default tail drop profile.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS aas_BindAllPortsToDefaultTdProfile
(
    IN  GT_U8   devNum,
    IN  CPSS_PORT_TX_DROP_PROFILE_SET_ENT portDefaultProfile,
    IN  GT_U32   queueDefaultProfile
);


/**
* @internal prvAppDemoAasSerdesConfigSet function
* @endinternal
*
* @brief   Per DB/RD board configure the Tx/Rx Polarity values on Serdeses
*
* @note   APPLICABLE DEVICES: Aas.
* @note   NOT APPLICABLE DEVICES:  Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5X; Ironman; xCat3; AC5; AC5P.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
static GT_STATUS prvAppDemoAasSerdesConfigSet
(
    IN GT_U8 devNum,
    IN GT_U8 boardRevId
)
{
    devNum = devNum;
    boardRevId = boardRevId;

    return GT_OK;
}

/* fill array of cpssApi_aas_defaultMap[] and update actualNum_cpssApi_aas_defaultMap and numCpuSdmas */
static GT_STATUS fillDbForCpssPortMappingInfo(IN GT_U32 devNum,IN GT_U8              boardRevId)
{
    GT_STATUS   rc;
    AAS_PORT_MAP_STC *currAppDemoInfoPtr;
    CPSS_DXCH_PORT_MAP_STC *currCpssInfoPtr;
    GT_U32  ii,jj;
    GT_U32 numEntries;
    GT_U32 maxPhyPorts;
    GT_U32 interfaceNum;
    GT_BOOL hqosMapping=GT_FALSE;
    GT_U32  value;
    GT_U32  singleQueuePerPort;

    if(2==boardRevId)
    {
        hqosMapping = GT_TRUE;
    }

    if(actualNum_cpssApi_aas_defaultMap)
    {
        /* already initialized */
        return GT_OK;
    }

    if((appDemoDbEntryGet("singleQueuePerPort", &value) == GT_OK))
    {
        singleQueuePerPort = value; /* allow value 0..max */
    }
    else
    {
        singleQueuePerPort = 0;
    }

    maxPhyPorts = PRV_CPSS_DXCH_MAX_PHY_PORT_NUMBER_MAC(devNum);
    if(NULL == aas_force_PortsInitListPtr || NULL == force_actualNum_aas_port_modePtr)
    {
        if(PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles == 1)
        {
            if((appDemoDbEntryGet("portModeExcelAllignedMapping", &value) == GT_OK) && (value != 0))
            {
                switch (value)
                {
                    case 11:
                      currAppDemoInfoPtr = &aas_port_mode_11[0];
                      numEntries = actualNum_aas_port_mode_11;
                      break;
                   case 3:
                      currAppDemoInfoPtr = &aas_port_mode_3[0];
                      numEntries = actualNum_aas_port_mode_3;
                      break;
                    default:
                     rc = GT_BAD_PARAM;
                     CPSS_ENABLER_DBG_TRACE_RC_MAC("unsupported pot mode %d", value);
                     return rc;
                     break;
                }

            }
            else if(PRV_CPSS_DXCH_GDMA_SUPPORTED_MAC(devNum))
            {
                if(GT_TRUE==hqosMapping)
                {
                    osPrintf("port mapping for HQOS and GDMA not implemented yet \n");
                    return GT_NOT_IMPLEMENTED;
                }

                currAppDemoInfoPtr = &GDMA_aas_port_mode[0];
                numEntries         = GDMA_actualNum_aas_port_mode;
            }
            else if(GT_FALSE==hqosMapping)
            {
                currAppDemoInfoPtr = &aas_port_mode[0];
                numEntries = actualNum_aas_port_mode;
            }
            else
            {
                currAppDemoInfoPtr = &aas_port_mode_hqos[0];
                numEntries = actualNum_aas_port_mode_hqos;
            }
        }
        else
        {
            currAppDemoInfoPtr = &aas_2_tiles_port_mode[0];
            numEntries = actualNum_aas_2_tiles_port_mode;
        }
    }
    else
    {
        currAppDemoInfoPtr = aas_forcePortMapPtr;
        numEntries = *force_actualNum_aas_port_modePtr;
    }

    cpssOsMemSet(cpssApi_aas_defaultMap,0,sizeof(cpssApi_aas_defaultMap));

    currCpssInfoPtr    = &cpssApi_aas_defaultMap[0];

    for (ii = 0 ; ii < numEntries; ii++ , currAppDemoInfoPtr++)
    {
        for(jj = 0 ; currAppDemoInfoPtr->portNumberArr[jj] != GT_NA ; jj++ , currCpssInfoPtr++)
        {
            if(actualNum_cpssApi_aas_defaultMap >= maxPhyPorts)
            {
                rc = GT_FULL;
                CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssApi_aas_defaultMap is FULL, maxPhy port ", rc);
                return rc;
            }

            interfaceNum = currAppDemoInfoPtr->globalDmaNumber[jj];


            currCpssInfoPtr->physicalPortNumber = currAppDemoInfoPtr->portNumberArr[jj];
            currCpssInfoPtr->mappingType        = currAppDemoInfoPtr->mappingType;
            currCpssInfoPtr->interfaceNum       = interfaceNum;

            if(singleQueuePerPort)
            {
                currCpssInfoPtr->txqPortNumber      = singleQueuePerPort;/* allow value 1..max for all the ports */
            }
            else
            {
                currCpssInfoPtr->txqPortNumber      = currAppDemoInfoPtr->numberOfQueuesPerPort;
            }
            currCpssInfoPtr->reservePreemptiveChannel = currAppDemoInfoPtr->reservePreemptiveChannel;
            if(NULL !=currAppDemoInfoPtr->hqosIdArr)
            {
                currCpssInfoPtr->hqosId[0]= currAppDemoInfoPtr->hqosIdArr[jj*2];
                currCpssInfoPtr->hqosId[1]= currAppDemoInfoPtr->hqosIdArr[jj*2+1];
            }
            else
            {
                currCpssInfoPtr->hqosId[0]= 0;
                currCpssInfoPtr->hqosId[1]= 0;
            }

            if(currAppDemoInfoPtr->mappingType == CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E)
            {
                if(PRV_CPSS_PP_MAC(devNum)->isGmDevice && currCpssInfoPtr->physicalPortNumber != CPU_PORT)
                {
                    /* skip this SDMA - the GM currently supports single SDMA */
                    currCpssInfoPtr--;
                    continue;
                }

                numCpuSdmas++;
            }

            actualNum_cpssApi_aas_defaultMap++;
        }
    }

    return GT_OK;
}

/**
* @internal aas_getBoardInfo function
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
static GT_STATUS aas_getBoardInfo
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
    appDemoPpConfigList[SYSTEM_DEV_NUM_MAC(appDemoPpConfigDevAmount)].pciInfo = *pciInfo;

    rc = extDrvPexConfigure(
            pciInfo->pciBusNum,
            pciInfo->pciIdSel,
            pciInfo->funcNo,
            MV_EXT_DRV_CFG_FLAG_EAGLE_E,
            hwInfoPtr);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("extDrvPexConfigure", rc);

    appDemoPpConfigList[SYSTEM_DEV_NUM_MAC(appDemoPpConfigDevAmount)].hwInfo = *hwInfoPtr;

    return rc;
}

/**
* @internal aas_getBoardInfoSimple function
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
static GT_STATUS aas_getBoardInfoSimple
(
    OUT CPSS_HW_INFO_STC *hwInfoPtr,
    OUT GT_PCI_INFO *pciInfo,
    IN GT_BOOL  firstDev
)
{
    GT_STATUS   rc;

    rc = aas_getBoardInfo(firstDev, hwInfoPtr, pciInfo);
    return rc;
}

#ifdef ASIC_SIMULATION
extern GT_U32 simCoreClockOverwrittenGet(GT_VOID);
#endif

/**
* @internal aas_boardTypePrint function
* @endinternal
*
* @brief   This function prints type of aas board and Cider version.
*
* @param[in] boardName                - board name
* @param[in] devName                  - device name
*                                       none
*/
static GT_VOID aas_boardTypePrint
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

    if(appDemoPpConfigDevAmount > 1)
    {
        cpssOsPrintf("%s Board Type: %s [%s] of [%d] devices \n", devName , boardName , environment, appDemoPpConfigDevAmount);
    }
    else
    {
        cpssOsPrintf("%s Board Type: %s [%s]\n", devName , boardName , environment);
    }
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
    GT_U32    tmpData;

    UNUSED_PARAM_MAC(boardRevId);

    rc = appDemoDbEntryGet("use_vPort_mode", &tmpData);
    if(rc == GT_OK && tmpData != 0)
    {
        appDemoUseVPortMode = GT_TRUE;

        rc = appDemoDbEntryGet("learnPriority_minimalVPortNumber", &learnPriority_minimalVPortNumber);
        if(rc != GT_OK)/* not defined by the caller , so we need 'default' */
        {
            learnPriority_minimalVPortNumber = GT_NA;/* use num of physical ports */
        }

        rc = appDemoDbEntryGet("learnPriority_numberOfVPorts", &learnPriority_numberOfVPorts);
        if(rc != GT_OK)/* not defined by the caller , so we need 'default' */
        {
            learnPriority_numberOfVPorts = GT_NA;/* use : (16K - num of physical ports) */
        }
    }
    else
    {
        appDemoUseVPortMode = GT_FALSE;
    }

    phase1Params->devNum = CAST_SW_DEVNUM(devNum);

/* retrieve PP Core Clock from HW */
    phase1Params->coreClock             = CPSS_DXCH_AUTO_DETECT_CORE_CLOCK_CNS;

    phase1Params->serdesRefClock        = CPSS_DXCH_PP_SERDES_REF_CLOCK_EXTERNAL_156_25_DIFF_E;

    phase1Params->ppHAState             = CPSS_SYS_HA_MODE_ACTIVE_E;


    phase1Params->numOfPortGroups       = 1;

    /* Eagle access mode */
    phase1Params->mngInterfaceType      = CPSS_CHANNEL_PEX_EAGLE_E;

    phase1Params->maxNumOfPhyPortsToUse = appDemoUseVPortMode == GT_TRUE ?
                                            (2==boardRevId /* hqosMapping with remote ports */?
                                                _1K : /* reserved range for physical ports and remotes ('non-vPorts') */
                                                512) :/* reserved range for physical ports ('non-vPorts')             */
                                            0/*not care - any value 0..1023 allowed */;
    phase1Params->vPortModeInfo.enableVPortMode = appDemoUseVPortMode;
    phase1Params->vPortModeInfo.learnPriority.minimalVPortNumber =
    learnPriority_minimalVPortNumber == GT_NA ?
        phase1Params->maxNumOfPhyPortsToUse :
        learnPriority_minimalVPortNumber;
    phase1Params->vPortModeInfo.learnPriority.numberOfVPorts     =
        learnPriority_numberOfVPorts == GT_NA ?
        (_16K-phase1Params->maxNumOfPhyPortsToUse):
        learnPriority_numberOfVPorts;
    if(phase1Params->vPortModeInfo.enableVPortMode == GT_TRUE)
    {
        osPrintf("work in 'vPort mode' : maxNumOfPhyPortsToUse[%d],learnPriority.minimalVPortNumber[%d],learnPriority.numberOfVPorts[%d] \n",
            phase1Params->maxNumOfPhyPortsToUse,
            phase1Params->vPortModeInfo.learnPriority.minimalVPortNumber,
            phase1Params->vPortModeInfo.learnPriority.numberOfVPorts);
    }
    else
    {
        osPrintf("work in legacy mode : non-'vPort mode' \n");
        osPrintf("for new 'vPort mode' see DB flags: use_vPort_mode,learnPriority_minimalVPortNumber,learnPriority_numberOfVPorts \n");
    }

    /* add Db Entry */
    rc = appDemoDxHwPpPhase1DbEntryInit(CAST_SW_DEVNUM(devNum),phase1Params,
        CPSS_98DX4180_CNS ,/* dummy to represent 'aas' family (one of AAS values) */
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
    IN GT_U32               devIndex,
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

    /* Disable double AUQ for SIP7 devices ,as :
       the CPSS will decide how many 'sub-queues' will be for be AUQ
    */
    appDemo_ppPhase2Params.useDoubleAuq = GT_FALSE;

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

    appDemoCpssCurrentDevIndex = devIndex;

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
* @internal aas_initPortMappingStage function
* @endinternal
*
*/
static GT_STATUS aas_initPortMappingStage
(
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
        case CPSS_AAS_ALL_DEVICES_CASES_MAC:
        case CPSS_AAS_2_TILES_ALL_DEVICES_CASES_MAC:
        default:
            ARR_PTR_AND_SIZE_MAC(cpssApi_aas_defaultMap, mapArrPtr, mapArrLen);
            mapArrLen = actualNum_cpssApi_aas_defaultMap;
            break;
    }

    if(appDemoPortMapPtr && appDemoPortMapSize)
    {
        /* allow to 'FORCE' specific array by other logic */
        mapArrPtr = appDemoPortMapPtr;
        mapArrLen = appDemoPortMapSize;
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
            goto exitCleanly_lbl;
        }
        return rc;
    }

    /* it's for appdemo DB only */
    rc = appDemoDxChMaxMappedPortSet(CAST_SW_DEVNUM(devNum), mapArrLen, mapArrPtr);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoDxChMaxMappedPortSet", rc);

    exitCleanly_lbl:
    if (mapUpdatedArrPtr != NULL)
    {
        cpssOsFree(mapUpdatedArrPtr);
    }
    return rc;
}

extern GT_STATUS appDemoBc2PortListInitForceShowOnlySet(IN GT_BOOL force);

/**
* @internal aas_PortModeConfiguration function
* @endinternal
*
*/
static GT_STATUS aas_PortModeConfiguration
(
    IN GT_U32                   devIndex,
    IN GT_SW_DEV_NUM            devNum,
    IN CPSS_PP_DEVICE_TYPE      devType,
    IN GT_U8                    boardRevId
)
{
    GT_STATUS   rc;
    PortInitList_STC *portInitListPtr;
    CPSS_SYSTEM_RECOVERY_INFO_STC   system_recovery;
    GT_U32      value;

    UNUSED_PARAM_MAC(devType);
    UNUSED_PARAM_MAC(devIndex);

    if(aas_force_PortsInitListPtr)
    {
        /* allow to 'FORCE' specific array by other logic */
        portInitListPtr = aas_force_PortsInitListPtr;
    }
    else
    if(PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles == 1)
    {
        portInitListPtr = aas_portInitlist;
        if(2==boardRevId)
        {
             portInitListPtr = aas_hqosPortInitlist;
        }
        else if((appDemoDbEntryGet("portModeExcelAllignedMapping", &value) == GT_OK) && (value != 0))
        {
            switch (value)
            {
                case 11:
                  portInitListPtr =aas_portMode_11_Initlist;
                  break;
                case 3:
                  portInitListPtr =aas_portMode_3_Initlist;
                  break;
                default:
                 rc = GT_BAD_PARAM;
                 CPSS_ENABLER_DBG_TRACE_RC_MAC("aas_PortModeConfiguration",rc);
                 return rc;
                 break;
            }

        }

    }
    else/*PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles==2*/
    {
        portInitListPtr = aas_2_tiles_portInitlist;
    }

    rc = cpssSystemRecoveryStateGet(&system_recovery);
    if (rc != GT_OK)
    {
      return rc;
    }

    if (CPSS_SYSTEM_RECOVERY_PROCESS_NOT_ACTIVE_E != system_recovery.systemRecoveryProcess)
    {
        /* force appDemoBc2PortListInit(...) to just print info , without any config */
        appDemoBc2PortListInitForceShowOnlySet(GT_TRUE);
    }

    rc = appDemoBc2PortListInit(CAST_SW_DEVNUM(devNum),portInitListPtr,GT_TRUE);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* indication for function appDemoBc2PortInterfaceInit(...) to
       not call appDemoBc2PortListInit(...) */
    appDemo_PortsInitList_already_done = 1;

    if (CPSS_SYSTEM_RECOVERY_PROCESS_NOT_ACTIVE_E != system_recovery.systemRecoveryProcess)
    {
        appDemoBc2PortListInitForceShowOnlySet(GT_FALSE);
    }

    return GT_OK;
}

/**
* @internal aas_appDemoDbUpdate function
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
static GT_STATUS aas_appDemoDbUpdate
(
    IN GT_U32             devIndex,
    IN GT_SW_DEV_NUM      devNum,
    IN CPSS_PP_DEVICE_TYPE     devType
)
{
    appDemoPpConfigList[devIndex].valid = GT_TRUE;
    appDemoPpConfigList[devIndex].devNum = CAST_SW_DEVNUM(devNum);
    appDemoPpConfigList[devIndex].deviceId = devType;
    appDemoPpConfigList[devIndex].devFamily = CPSS_PP_FAMILY_DXCH_AAS_E;
    appDemoPpConfigList[devIndex].apiSupportedBmp = APP_DEMO_DXCH_FUNCTIONS_SUPPORT_CNS;

    return GT_OK;
}

/**
* @internal aas_initPhase1AndPhase2 function
* @endinternal
*
*/
static GT_STATUS aas_initPhase1AndPhase2
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
    GT_HW_DEV_NUM           hwDevNum,hsHwDevNum;
    CPSS_PP_DEVICE_TYPE     devType;
    GT_U32                  waIndex;
    GT_U32    secondsStart;
    GT_U32    nanoSecondsStart;
    static CPSS_DXCH_IMPLEMENT_WA_ENT falcon_WaList[] =
    {
        /* no current WA that need to force CPSS */
        CPSS_DXCH_IMPLEMENT_WA_LAST_E
    };

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
    rc = aas_appDemoDbUpdate(devIndex, devNum , devType);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("hawk_appDemoDbUpdate", rc);
    if (rc != GT_OK)
        return rc;

    if (appDemoInitRegDefaults != GT_FALSE)
    {
        return GT_OK;
    }

    osPrintf("devNum[%d] : set DMA windows (MG , oATU) \n",devNum);

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

    rc = aas_initPortMappingStage(devNum,devType);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("aas_initPortMappingStage", rc);
    if (GT_OK != rc)
        return rc;

    /* memory related data, such as addresses and block lenghts, are set in this funtion*/
    rc = getPpPhase2ConfigSimple(devIndex , devNum, devType, &cpssPpPhase2Info);
    if (rc != GT_OK)
        return rc;

    cpssOsTimeRT(&secondsStart, &nanoSecondsStart);
    rc = cpssDxChHwPpPhase2Init(CAST_SW_DEVNUM(devNum),  &cpssPpPhase2Info);
    if (prvAppDemoFirstInitTimes.initSystemTimePpPhase2Init == 0)
    {
        prvAppDemoFirstInitTimes.initSystemTimePpPhase2Init =
            prvAppDemoGetDurationTimeMilliSec(secondsStart, nanoSecondsStart);
    }
    if (rc != GT_OK)
        return rc;

    hwDevNum = (devNum + appDemoHwDevNumOffset) & 0x3FF;

    /*in case of CPSS_SYSTEM_RECOVERY_PROCESS_HITLESS_STARTUP_E hwDevNum should be set by application */
    if (appDemoDbEntryGet("hitless_startup_HwDevNum",&hsHwDevNum) == GT_OK)
    {
        hwDevNum = (devNum + hsHwDevNum) & 0x3FF;
    }

    /* set HWdevNum related values */
    rc = appDemoDxHwDevNumChange(CAST_SW_DEVNUM(devNum), hwDevNum);
    if (GT_OK != rc)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal aas_getNeededLibs function
* @endinternal
*
*/
static GT_STATUS aas_getNeededLibs(
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
    libInitParamsPtr->initPha    = GT_TRUE;/*prvPhaLibInit*/
    libInitParamsPtr->initCncDump= GT_TRUE;/*prvCncLibInit*/

    sysConfigParamsPtr->numOfTrunks         = _4K;    /*used by prvTrunkLibInit*/
    sysConfigParamsPtr->lpmDbSupportIpv4    = GT_TRUE;/*used by prvIpLibInit*/
    sysConfigParamsPtr->lpmDbSupportIpv6    = GT_TRUE;/*used by prvIpLibInit*/
    /*NOTYE: info sysConfigParamsPtr->lpmRamMemoryBlocksCfg is set later : used by prvIpLibInit*/

    sysConfigParamsPtr->pha_packetOrderChangeEnable = GT_FALSE;/*used by prvPhaLibInit*/

    return GT_OK;
}

/**
* @internal aas_initPpLogicalInit function
* @endinternal
*
*/
static GT_STATUS aas_initPpLogicalInit
(
    IN GT_SW_DEV_NUM       devNum,
    OUT CPSS_DXCH_LPM_RAM_CONFIG_STC    *lpmRamMemoryBlocksCfgPtr
)
{
    GT_STATUS   rc;
    CPSS_DXCH_PP_CONFIG_INIT_STC    ppLogInitParams;
    CPSS_DXCH_PP_CONFIG_SHARED_TABLES_SIZE_STC sharedTablesSizes;
    GT_U32      ii;
    GT_U32 value = 0;
    GT_U32    secondsStart;
    GT_U32    nanoSecondsStart;

    osMemSet(&ppLogInitParams ,0, sizeof(ppLogInitParams));

    osMemSet(&sharedTablesSizes,0, sizeof(sharedTablesSizes));

    /*not using _512K that that use 25 SBMs because the LPM code fail on it */
    sharedTablesSizes.numPrefixesForLPM = 491520;/* this use 24 SMBs */
    sharedTablesSizes.numEntriesForFDB = _256K;
    sharedTablesSizes.numEmUnitsNeeded = 2;
    ii = 0;
    sharedTablesSizes.emUnits[ii].singleTableUsedForThe2Clients = GT_FALSE;
    sharedTablesSizes.emUnits[ii].autoLearnMode = CPSS_DXCH_PP_CONFIG_EM_AUTO_LEARN_MODE_CLIENT1_E;
    sharedTablesSizes.emUnits[ii].clientInfoArr[0].clientType = CPSS_DXCH_PP_CONFIG_EM_CLIENT_TYPE_TTI_FIRST_E;
    sharedTablesSizes.emUnits[ii].clientInfoArr[0].numEntriesForClient = _32K;
    sharedTablesSizes.emUnits[ii].clientInfoArr[1].clientType = CPSS_DXCH_PP_CONFIG_EM_CLIENT_TYPE_IPCL0_SECOND_E;
    sharedTablesSizes.emUnits[ii].clientInfoArr[1].numEntriesForClient = _32K;

    ii = 1;
    sharedTablesSizes.emUnits[ii].singleTableUsedForThe2Clients = GT_TRUE;
    sharedTablesSizes.emUnits[ii].autoLearnMode = CPSS_DXCH_PP_CONFIG_EM_AUTO_LEARN_MODE__NOT_USED__E;
    sharedTablesSizes.emUnits[ii].clientInfoArr[0].clientType = CPSS_DXCH_PP_CONFIG_EM_CLIENT_TYPE_TTI_ILM2_E;
    sharedTablesSizes.emUnits[ii].clientInfoArr[0].numEntriesForClient = _16K;
    sharedTablesSizes.emUnits[ii].clientInfoArr[1].clientType = CPSS_DXCH_PP_CONFIG_EM_CLIENT_TYPE_TTI_ILM3_E;
    sharedTablesSizes.emUnits[ii].clientInfoArr[1].numEntriesForClient = _32K;/*ignores*/

    sharedTablesSizes.numEntriesForARP = _192K;
    sharedTablesSizes.numEntriesForTS  = _64K;
    sharedTablesSizes.numEntriesForHF0 = _32K;
    sharedTablesSizes.numEntriesForHF1 = _16K;

    sharedTablesSizes.numEntriesForIpfixOrBilling = _32K;
    sharedTablesSizes.numEntriesForMetering       = _16K;

    sharedTablesSizes.numOfCncBlocks              = 16;

    sharedTablesSizes.numEntriesForIpeState0 = _16K-_4K;/*12K*/
    sharedTablesSizes.numEntriesForIpeState1 = _16K+_4K;/*20K*/

    /* wanted to use small block for PBR  = 3200 (640*5) instead of 8K , but this cause bad behavior */
    /* so wanted to use 8K but this cause to use more than single big bank (5k in bank = 1K lines)
       8K fatal error on UTF_TEST_CASE_MAC(cpssDxChLpmLeafEntryWrite) */
    /* so using 5K */
    /* from more info
       see JIRA : CPSS-10305 : Phoenix : using 8K PBR cause cpss to write to non-existing memory */
    ppLogInitParams.maxNumOfPbrEntries = _5K;
    ppLogInitParams.lpmMemoryMode = CPSS_DXCH_LPM_RAM_MEM_MODE_FULL_MEM_SIZE_E;
    ppLogInitParams.sharedTableMode = CPSS_DXCH_CFG_SHARED_TABLE_MODE_MID_L3_MID_L2_MIN_EM_E;
    ppLogInitParams.sharedTablesSizeInfo = sharedTablesSizes;


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

#define AAS_LED_UNITS_MAX_NUM_CNS 3
#define AAS_DB_LED_INDICATIONS_NUM_CNS 20

/* Aas DB board has following LEDs:
   QSFP cages have 4 LEDs per cage - one per SERDES
   SFP cages have 1 LED per cage
   First two LED Units should have 8 indications. Last one should have 4 indications.
   Use Class 2 indication in mode "Link and Activity" */
static const APP_DEMO_LED_UNIT_CONFIG_STC aas_DB_led_indications[AAS_LED_UNITS_MAX_NUM_CNS] =
{
    {64, 71,    GT_FALSE}, /* 8 LED ports (LED Unit 0 - DP0)     */
    {64, 71,    GT_FALSE}, /* 8 LED ports (LED Unit 1 - DP1)     */
    {64, 67,    GT_FALSE}  /* 4 LED ports (LED Unit 2 - DP2)     */
};

/* MAC to LED Position mapping */
static const APP_DEMO_LED_PORT_MAC_POSITION_STC aas_DB_led_port_map[AAS_DB_LED_INDICATIONS_NUM_CNS] =
{
    { 2, 6}, { 4, 5}, { 0, 7}, { 6, 4}, {12, 1}, {10, 2}, {14, 0}, { 8, 3},
    {18, 6}, {20, 5}, {16, 7}, {22, 4}, {28, 1}, {26, 2}, {30, 0}, {24, 3},
    {32, 3}, {34, 2}, {36, 1}, {38, 0}
};

/**
* @internal aasLedPortPositionGet
* @endinternal
*
* @brief   Gets LED ports position for the current MAC port
*
* @param[in] boardRevId            - board revision ID
* @param[in] portMacNum            - current MAC to check
*
* @retval GT_U32                   - the LED ports position for the current MAC port
*
*/
static GT_U32 aasLedPortPositionGet
(
    IN  GT_U8  boardRevId,
    IN  GT_U32 portMacNum
)
{
    GT_U32 ledPosition = APPDEMO_BAD_VALUE;
    const APP_DEMO_LED_PORT_MAC_POSITION_STC * portsMapArrPtr;
    GT_U32 portsMapArrLength;
    GT_U32 ii;

    GT_UNUSED_PARAM(boardRevId);

    portsMapArrPtr = aas_DB_led_port_map;
    portsMapArrLength = AAS_DB_LED_INDICATIONS_NUM_CNS;

    for (ii = 0; ii < portsMapArrLength; ii++)
    {
        if (portsMapArrPtr[ii].macPortNumber == portMacNum)
        {
            ledPosition = portsMapArrPtr[ii].ledPortPosition;
            break;
        }
    }

    return ledPosition;
}

/**
* @internal aas_LedInit function
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
static GT_STATUS aas_LedInit
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
    CPSS_SYSTEM_RECOVERY_INFO_STC   system_recovery; /* holds system recovery information */

    /*CPSS_TBD_BOOKMASK_AAS: kalex: remove after initial bring up*/
    if(cpssDeviceRunCheck_onEmulator())
    {
        return GT_OK;
    }

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

    rc = cpssSystemRecoveryStateGet(&system_recovery);
    if (rc != GT_OK)
    {
      return rc;
    }

    ledStreamIndication = aas_DB_led_indications;

    for(ledUnit = 0; ledUnit < AAS_LED_UNITS_MAX_NUM_CNS; ledUnit++)
    {
        /* Start of LED stream location - Class 2 */
        ledConfig.sip6LedConfig.ledStart[ledUnit] = ledStreamIndication[ledUnit].ledStart;
        /* End of LED stream location: LED ports 0..7 */
        ledConfig.sip6LedConfig.ledEnd[ledUnit] = ledStreamIndication[ledUnit].ledEnd;
        /* LED unit may be bypassed in the LED chain */
        ledConfig.sip6LedConfig.ledChainBypass[ledUnit] = ledStreamIndication[ledUnit].bypassLedUnit;
    }

    /*CPSS_TBD_BOOKMASK_AAS: kalex: remove after initial bring up*/
    if(cpssDeviceRunCheck_onEmulator())
    {
        /*do nothing */
    }
    else
    {
        rc = cpssDxChLedStreamConfigSet(devNum, 0, &ledConfig);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDxChLedStreamConfigSet", rc);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    classNum = 2;
    ledClassManip.invertEnable            = GT_FALSE; /* not relevant for SIP_5/6 */
    ledClassManip.blinkEnable             = GT_TRUE;
    ledClassManip.blinkSelect             = CPSS_LED_BLINK_SELECT_0_E;
    ledClassManip.forceEnable             = GT_FALSE;
    ledClassManip.forceData               = 0;
    ledClassManip.pulseStretchEnable      = GT_FALSE;

    /* activate disableOnLinkDown as part of "Link and Activity" indication */
    ledClassManip.disableOnLinkDown       = GT_TRUE;

    rc = cpssDxChLedStreamClassManipulationSet(devNum, 0,
                                               CPSS_DXCH_LED_PORT_TYPE_XG_E /* don't care , not applicable for Hawk */
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

        position = aasLedPortPositionGet(boardRevId, portMap.interfaceNum);

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

    return GT_OK;
}

/**
* @internal aas_appDemoInitSequence_part1 function
* @endinternal
*
* @brief   Part 1 : This is the 'simple' board initialization function for the device.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*
* @note This is a simplified version of cpssInitSystem.
*
*/
static GT_STATUS aas_appDemoInitSequence_part1
(
    IN  GT_U8             boardRevId,
    IN GT_U32             devIndex,
    IN GT_SW_DEV_NUM      devNum,
    IN CPSS_HW_INFO_STC   *hwInfoPtr
)
{
    GT_STATUS   rc;
    CPSS_PP_DEVICE_TYPE     devType;

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

    rc = aas_initPhase1AndPhase2(devNum,boardRevId,hwInfoPtr,devIndex,&devType);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("aas_initPhase1AndPhase2", rc);
    if (GT_OK != rc)
        return rc;




    return GT_OK;
}

extern GT_STATUS appDemoTrunkCrcHashDefaultsSet(IN GT_U8 devNum);
/**
* @internal aas_appDemoInitSequence_part2 function
* @endinternal
*
* @brief   part2 : This is the 'simple' board initialization function for the device.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*
* @note This is a simplified version of cpssInitSystem.
*
*/
static GT_STATUS aas_appDemoInitSequence_part2
(
    IN  GT_U8             boardRevId,
    IN GT_U32             devIndex,
    IN GT_SW_DEV_NUM      devNum
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


    if (appDemoInitRegDefaults != GT_FALSE)
    {
        return GT_OK;
    }

    devType = appDemoPpConfigList[devIndex].deviceId;

    if (appDemoDbEntryGet("portMgr", &tmpData) != GT_OK)
    {
        portMgr = GT_FALSE;
    }
    else
    {
        portMgr = (tmpData == 1) ? GT_TRUE : GT_FALSE;
    }

    if(!cpssDeviceRunCheck_onEmulator())
    {
        rc = prvAppDemoAasSerdesConfigSet(CAST_SW_DEVNUM(devNum),boardRevId);
        CPSS_ENABLER_DBG_TRACE_RC_MAC("prvAppDemoAasSerdesConfigSet", rc);
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
    rc = aas_PortModeConfiguration(devIndex,devNum,devType,boardRevId);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("aas_PortModeConfiguration", rc);
    if (rc != GT_OK)
        return rc;

    rc = aas_getNeededLibs(&libInitParams,&sysConfigParams);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("aas_getNeededLibs", rc);
    if(rc != GT_OK)
        return rc;

    rc = aas_initPpLogicalInit(devNum,&sysConfigParams.lpmRamMemoryBlocksCfg);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("aas_initPpLogicalInit", rc);
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

    rc = appDemoBc2PIPEngineInit(CAST_SW_DEVNUM(devNum));
    CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoBc2PIPEngineInit", rc);
    if(rc != GT_OK)
        return rc;

    rc = aas_LedInit(CAST_SW_DEVNUM(devNum), boardRevId);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("aas_LedInit", rc);
    if(rc != GT_OK)
        return rc;

    rc = appDemoB2PtpConfig(CAST_SW_DEVNUM(devNum));
    CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoB2PtpConfig", rc);
    if(rc != GT_OK)
        return rc;

    rc = appDemoDxTrafficEnable(CAST_SW_DEVNUM(devNum));
    CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoDxTrafficEnable", rc);
    if(rc != GT_OK)
        return rc;

    rc = appDemoTrunkCrcHashDefaultsSet(CAST_SW_DEVNUM(devNum));
    CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoDxTrafficEnable", rc);
    if(rc != GT_OK)
        return rc;

    rc = aas_BindAllPortsToDefaultTdProfile(CAST_SW_DEVNUM(devNum),CPSS_PORT_TX_DROP_PROFILE_1_E,0);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("aas_BindAllPortsToDefaultTdProfile", rc);
    if(rc != GT_OK)
        return rc;

    rc = aas_SpecialPortsInit(devNum);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("aas_SpecialPortsInit", rc);
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

    /* not support 'system reset' */
    rc = cpssDevSupportSystemResetSet(CAST_SW_DEVNUM(devNum), supportSystemReset);
    CPSS_ENABLER_DBG_TRACE_RC_MAC("cpssDevSupportSystemResetSet", rc);
    if (GT_OK != rc)
        return rc;

    return GT_OK;
}

/**
* @internal aas_EventHandlerInit function
* @endinternal
*
*/
static GT_STATUS aas_EventHandlerInit
(
    GT_VOID
)
{
    GT_STATUS   rc;
    GT_U32 value;

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
#ifdef IMPL_TGF
GT_STATUS  alternative_CPU_port_expected_interface_set(
    IN GT_U32   alternativeCpuPort
);
#endif /*IMPL_TGF*/
/*
    MainUT tests:
    list of tests/suites that forbidden because crash the CPSS or the device
   (fatal error in simulation on unknown address)
*/
static GT_STATUS   appDemoAasMainUtForbidenTests(void)
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
         TEST_NAME(/*cpssDxChDiag.*/cpssDxChDiagPortGroupTablePrint       )
        ,TEST_NAME(/*cpssDxChPhySmi.*/cpssDxChPhyXsmiMdcDivisionFactorSet )
        ,TEST_NAME(/*cpssDxChExactMatch.*/cpssDxChExactMatchExpandedActionSpecificScenarioSet )
        ,TEST_NAME(/*cpssDxChExactMatch.*/cpssDxChExactMatchRulesDump )
        ,TEST_NAME(prvCpssDxChPorttTxPizzaResHawkDpSpeedCfg)
        ,{NULL}/* must be last */
    };

    /* suites that most or all tests CRASH or FATAL ERROR or Fail ! */
    static FORBIDEN_SUITES mainUt_forbidenSuites_CRASH_FAIL [] =
    {
         SUITE_NAME(cpssDxChHwInitDeviceMatrix )/* lib is not aligned yet */
        ,SUITE_NAME(cpssDxChPortSyncEther      )/* lib is not aligned yet */

        ,SUITE_NAME(cpssDxChPortTx    )
        ,SUITE_NAME(cpssDxChPtpManager)
        ,SUITE_NAME(cpssDxChMacSec)
        ,SUITE_NAME(cpssDxChPpu)               /* the code yet not ready */
        ,SUITE_NAME(cpssDxChPtp)               /* the code yet not ready */
        ,SUITE_NAME(cpssDxChExactMatchManager) /* the code yet not ready - registers moved from TCAM unut to clients */
        ,SUITE_NAME(cpssDxChBobcat2PortPizzaArbiter)/*last test with system reset not pass cpssInitSystem*/

        ,{NULL}/* must be last */
    };

    static FORBIDEN_TESTS mainUt_forbidenTests_Emulator_too_long_and_PASS [] =
    {
         TEST_NAME(cpssDxChDiagAllMemTest)                  /* : takes [313] sec */
        ,TEST_NAME(cpssDxChLatencyMonitoringPortCfgSet)     /* : takes [219] sec */
        ,TEST_NAME(cpssDxChPtpTaiTodStepSet)                /* : takes [119] sec */
        ,TEST_NAME(cpssDxChPtpTsuTSFrameCounterControlSet)  /* : takes [422] sec*/
        ,TEST_NAME(cpssDxChPtpTsuTSFrameCounterControlGet)  /* : takes [187] sec*/

        ,TEST_NAME(prvCpssDxChTestFdbHash_8K)               /* : takes [92] sec */
        ,TEST_NAME(prvCpssDxChTestFdbHash_16K)              /* : takes [92] sec */
        /*,TEST_NAME(prvCpssDxChTestFdbHash_32K) allow one of those to run */  /* : takes [92] sec */
        ,TEST_NAME(cpssDxChTrunkDbMembersSortingEnableSet_membersManipulations)/* : takes [167] sec*/
        ,TEST_NAME(cpssDxChExactMatchRulesDump)             /* : takes [209] sec */
        ,TEST_NAME(cpssDxChPortLoopbackPktTypeToCpuSet)     /* : takes [295] sec */

        /* enhUT : */
        ,TEST_NAME(tgfIpv4UcRoutingHiddenPrefixes      ) /*takes [215] sec*/
        ,TEST_NAME(tgfBasicIpv4UcEcmpRoutingHashCheck  ) /*takes [371] sec*/
        ,TEST_NAME(prvTgfPclMetadataPacketType         ) /*takes [578] sec*/
        ,TEST_NAME(tgfExactMatchManagerTtiPclFullPathHaCheckSingleEntry) /* take [170] seconds  */
        ,TEST_NAME(tgfExactMatchManagerHaFullCapacity)                   /* take [1014] seconds */
        ,TEST_NAME(tgfExactMatchManagerDeleteScanFullCapacity)           /* take [754] seconds  */
        ,TEST_NAME(tgfExactMatchManagerAgingScanFullCapacity)            /* take [914] seconds  */
        ,TEST_NAME(tgfTrunkCascadeTrunk)                                 /* takes [264] sec  */
        ,TEST_NAME(tgfTrunkCascadeTrunkWithWeights)                      /* takes [271] sec  */
        ,TEST_NAME(tgfTrunkCrcHash_liteMode_CRC32_cpssPredictEgressPort) /* takes [1808] sec */
        ,TEST_NAME(tgfTrunkPacketTypeHashModeTest)                       /* takes [446] sec  */
        ,TEST_NAME(prvTgfPclFwsMove)                                     /* takes [320] sec  */
        ,TEST_NAME(prvTgfPclFwsMove_virtTcam_80B)                        /* takes [486] sec  */

        ,{NULL}/* must be last */
    };

    static FORBIDEN_SUITES mainUt_forbidenSuites_Emulator_too_long_and_PASS [] =
    {
         SUITE_NAME(cpssDxChTrunkHighLevel_1) /* cpssDxChTrunkHighLevel is enough to run */
        ,SUITE_NAME(cpssDxChTrunkLowLevel_1)  /* cpssDxChTrunkLowLevel  is enough to run */

        ,{NULL}/* must be last */
    };

    static FORBIDEN_TESTS enhUt_forbidenTests_FatalError [] =
    {
         /* tgfBridge */
         TEST_NAME(tgfVlanManipulationEthernetOverMpls)
        ,TEST_NAME(prvTgfBrgGenBypassMode)
        ,TEST_NAME(prvTgfBrgSrcIdPortForce_bypassIngressPipe)
        ,TEST_NAME(prvTgfBrgSrcIdScalableSgt)
        ,TEST_NAME(prvTgfBrgGenPortBypassMode)
        ,TEST_NAME(prvTgfBrgPVEMirroring)
        ,TEST_NAME(prvTgfFdbIpv4UcPointerRouteByIndex)
        ,TEST_NAME(prvTgfFdbIpv4UcPointerRouteByMsg)
        ,TEST_NAME(prvTgfFdbIpv4UcRoutingAgingEnable)
        /* tgfVnt */
        ,TEST_NAME(tgfVntCfmEtherTypeIdentification)
        ,TEST_NAME(tgfVntCfmOpcodeIdentification)
        /* tgfCos */
        ,TEST_NAME(tgfCosTcDpRemappingOfStackPortTrafficOnStackPorts) /* pcl done, egress cntrs problem */
        /* tgfL2Mll */
        ,TEST_NAME(prvTgfUnucUnregmcBcFiltering_with_PCL_mirror_to_cpu)
        ,TEST_NAME(prvTgfEgressInterface)
        ,TEST_NAME(prvTgfEgressInterfaceEntrySelector)
        /* tgfIp */
        ,TEST_NAME(tgfTrunkBasicIpv4UcRoutingTrunkAToTrunkA)
        ,TEST_NAME(tgfVidxBasicIpv4UcRoutingTrunkAToVidxWithTrunkA)
        ,TEST_NAME(tgfBasicIpv4McRoutingSourceId)
        ,TEST_NAME(tgfIpv4UcNatTcpRouting)
        ,TEST_NAME(tgfIpv4UcNatUdpWithOptions)
        ,TEST_NAME(tgfIpv4UcPbrRoutingPriority)
        ,TEST_NAME(tgfIpv4UcPbrLpmRoutingPriorityFdbEcmp)
        ,TEST_NAME(tgfIpv4McRoutingWithFdb)
        ,TEST_NAME(tgfIpv4McRoutingWithFdbPclActionVrfIdAssign)
        ,TEST_NAME(tgfIpv4UcRoutingNonExistedVrf)
        ,TEST_NAME(tgfIpv4UcPbrLpmRoutingPriority)
        /* tgfFcoe */
        ,TEST_NAME(tgfFcoeBaseForwarding)
        /* tgfStream */
        ,TEST_NAME(prvTgfStreamSgcMaxSduSizeProfileCheckIngress)
        ,TEST_NAME(prvTgfStreamSgcMaxSduSizeProfileCheckEgress)
        /* tgfTrunk */
        ,TEST_NAME(tgfTrunkSaLearning)
        ,TEST_NAME(tgfTrunkSaLearning_globalEPortRepresentTrunk)
        ,TEST_NAME(tgfTrunkSimpleHashMacSaDa)
        ,TEST_NAME(tgfTrunkSimpleHashMacSaDa_globalEPortRepresentTrunk)
        ,TEST_NAME(tgfTrunkCrcHash)
        ,TEST_NAME(tgfTrunkCrcHashL3L4OverMplsParseEnable)
        ,TEST_NAME(tgfTrunkCrcHashMaskPriority)
        ,TEST_NAME(tgfTrunkCascadeTrunkWithWeights)
        ,TEST_NAME(tgfTrunkSortMode)
        ,TEST_NAME(tgfTrunkCrcHash_liteMode_CRC32_cpssPredictEgressPort)
        ,TEST_NAME(tgfTrunk_80_members_flood_EPCL)
        ,TEST_NAME(tgfTrunk_80_members_flood_EPCL_resilient_mode)
        ,TEST_NAME(tgfTrunkIpclLoadBalancingHash)

        ,TEST_NAME(prvTgfCutThroughIpv4CtTermination_SlowToFast) /* fails due to not ready AAS support */
        ,TEST_NAME(prvTgfCutThroughBypassPolicer)                /* fails due to not ready AAS support */

        ,{NULL}/* must be last */
    };

    static FORBIDEN_SUITES enhUt_forbidenSuites_FatalError [] =
    {
        /* suits are not aligned yet for AAS and have a lot of
           failures and some of them have crashes */
         SUITE_NAME(tgfExactMatch   )
        ,SUITE_NAME(tgfConfig       )
        ,SUITE_NAME(tgfCnc          )
        ,SUITE_NAME(tgfTunnel       )
        ,SUITE_NAME(tgfPolicer      ) /* needed something instead of management counters */
        ,SUITE_NAME(tgfIpfix        )
        ,SUITE_NAME(tgfPtpManager   )
        ,SUITE_NAME(tgfPtp          )
        ,SUITE_NAME(tgfPpu          )
        ,SUITE_NAME(tgfSgt          )
        ,SUITE_NAME(tgfVirtualTcam  )
        ,SUITE_NAME(tgfEvent  )
        ,SUITE_NAME(tgfBridgeFdbManager )
        ,SUITE_NAME(tgfExactMatchManager)
        ,{NULL}/* must be last */
    };

    static FORBIDEN_TESTS enhUt_forbidenTests_too_long_and_fail [] =
    {
         TEST_NAME(tgfPortTxSip6QcnQueueGlobalResourcesWithoutIngressHdr) /* failed [20] times ,takes [618] sec  */
        ,TEST_NAME(tgfPortTxSip6QcnQueueGlobalResourcesWithIngressHdr)    /* failed [20] times ,takes [534] sec  */

        ,TEST_NAME(tgfPtpAddCorrectionTimeTest)
        ,TEST_NAME(prvTgfGenInterruptCoalescingForLinkChangeEvent)

        ,{NULL}/* must be last */
    };

    static FORBIDEN_TESTS mainUt_forbidenTests_Emulator_Stuck [] =
    {
        {NULL}/* must be last */
    };

    static FORBIDEN_TESTS enhancedUt_forbidenTests_Emulator_Stuck [] =
    {
        TEST_NAME(/*tgfEvent.*/prvTgfGenInterruptCoalescing)/* critical ERROR .. cause 'fatal error' */

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
        ,TEST_NAME(tgfPortTxSip6QcnQueuePool0ResourcesWithoutIngressHdr)
        ,TEST_NAME(tgfPortTxSip6QcnQueuePool1ResourcesWithoutIngressHdr)
        ,TEST_NAME(tgfPortTxSip6QcnQueuePool0ResourcesWithIngressHdr)
        ,TEST_NAME(tgfPortTxSip6QcnQueuePool1ResourcesWithIngressHdr)
        ,TEST_NAME(tgfTunnelTermEtherOverMplsPwLabelParallelLookup)
        ,TEST_NAME(prvTgfTunnelTermPbrDualLookup)
        ,{NULL}/* must be last */
    };

#endif /*IMPL_TGF*/


    /* tests that defined forbidden by Hawk */

#ifdef IMPL_TGF
    appDemoForbidenTestsAdd (mainUt_forbidenTests_FatalError);
    appDemoForbidenSuitesAdd(mainUt_forbidenSuites_CRASH_FAIL);
    appDemoForbidenSuitesAdd(enhUt_forbidenSuites_FatalError);
    appDemoForbidenTestsAdd (enhUt_forbidenTests_FatalError);
    appDemoForbidenTestsAdd (enhUt_forbidenTests_too_long_and_fail);

    if (cpssDeviceRunCheck_onEmulator())
    {
        /* very slow suites */
        utfAddPreSkippedRule("cpssDxChBridgeFdbManager","*","","");
        utfAddPreSkippedRule("cpssDxChExactMatchManager","*","","");
        utfAddPreSkippedRule("tgfBridgeFdbManager","*","","");
        /*utfAddPreSkippedRule("tgfExactMatchManager","*","","");*/

        appDemoForbidenTestsAdd (mainUt_forbidenTests_Emulator_Stuck);
        appDemoForbidenTestsAdd (enhancedUt_forbidenTests_Emulator_Stuck);
        appDemoForbidenTestsAdd (mainUt_serdes);
        appDemoForbidenTestsAdd (mainUt_forbidenTests_Emulator_too_long_and_PASS);
        appDemoForbidenSuitesAdd(mainUt_forbidenSuites_Emulator_too_long_and_PASS);
        appDemoForbidenSuitesAdd (enhancedUt_forbidenSuites_Emulator_Killer);

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
* @internal aas_localUtfInit function
* @endinternal
*
*/
static GT_STATUS aas_localUtfInit
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;

#ifdef INCLUDE_UTF
    GT_SW_DEV_NUM           devNum;
    GT_U32      devIndex;/* device index in the array of appDemoPpConfigList[devIndex] */

    /* Initialize unit tests for CPSS */
    rc = utfPreInitPhase();
    if (rc != GT_OK)
    {
        utfPostInitPhase(rc);
        return rc;
    }

    START_LOOP_ALL_DEVICES(devIndex,devNum)
    {
        rc = utfInit(CAST_SW_DEVNUM(devNum));
        if (rc != GT_OK)
        {
            CPSS_ENABLER_DBG_TRACE_RC_MAC("utfInit", rc);
            utfPostInitPhase(rc);
            return rc;
        }
    }
    END_LOOP_ALL_DEVICES

    utfPostInitPhase(rc);
#endif /* INCLUDE_UTF */

    appDemoAasMainUtForbidenTests();

#ifdef IMPL_TGF
    alternative_CPU_port_expected_interface_set(FROM_CPU_PORT_NUM);
#endif /* IMPL_TGF */

    return rc;
}

extern GT_STATUS prvCpssHwInitSip5IsInitTcamDefaultsDone(IN GT_U8   devNum, GT_BOOL tcamDaemonEnable);

extern void appDemoLinkChngEventSimulate
(
    GT_U32 devNum,
    GT_U32 portNum
);

/**
* @internal gtDbAasBoardReg_SimpleInit function
* @endinternal
*
* @brief   This is the 'simple' board initialization function for Aas device.
*
* @param[in] boardRevId               - Board revision Id.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*
* @note This is a simplified version of cpssInitSystem.
*
*/
static GT_STATUS gtDbAasBoardReg_SimpleInit
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
    GT_BOOL     firstDev;
    char*       namePtr = NULL;
    CPSS_SYSTEM_RECOVERY_INFO_STC   system_recovery;

    GT_U32  secondsStart, secondsEnd,
            nanoSecondsStart, nanoSecondsEnd,
            seconds, nanoSec; /* time of init */
    GT_U32  value;
    GT_BOOL resilientInit = GT_FALSE; /*in case of error during device init , loop doesn't stop just skip to next dev */

    last_boardRevId = boardRevId;

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

    appDemoPpConfigDevAmount = 0;

    firstDev = GT_TRUE;

    while(1)
    {
        /* this function finds a Prestera devices on PCI bus */
        rc = aas_getBoardInfoSimple(
                &hwInfo[0],
                &pciInfo,
                firstDev);
        if (rc != GT_OK)
        {
            break;/* no more devices */
        }

        appDemoPpConfigDevAmount++;
        firstDev = GT_FALSE;
    }

    namePtr = "AAS";
    aasBoardType = AAS_BOARD_TYPE_DB_AAS;

    /* print board type before phase 1,2 */
    aas_boardTypePrint("DB" /*boardName*/, namePtr /*devName*/);

    if ( (appDemoDbEntryGet("resilientInit", &value) == GT_OK) && (value != 0) )
    {
        cpssOsMemSet(appDemoSysConfig.resilientInitDevErrorArr, 0, sizeof(appDemoSysConfig.resilientInitDevErrorArr));
        resilientInit = GT_TRUE;
    }

    for (devIndex = SYSTEM_DEV_NUM_MAC(0);
          (devIndex < SYSTEM_DEV_NUM_MAC(appDemoPpConfigDevAmount)); devIndex++)
    {
        devNum =   devIndex;

        SYSTEM_SKIP_NON_ACTIVE_DEV((GT_U8)devIndex);

        rc = aas_appDemoInitSequence_part1(boardRevId, devIndex, devNum, &appDemoPpConfigList[devIndex].hwInfo);
        if (rc != GT_OK)
        {
            RESILIENT_INIT_SET_OR_RETURN(rc,resilientInit,devIndex);
        }
    }

    /* take time from the 'phase1 init' stage (not including the 'PCI scan' operations) */
    cpssOsTimeRT(&start_sec, &start_nsec);

    START_LOOP_ALL_DEVICES(devIndex,devNum)
    {
        RESILIENT_INIT_CHECK_AND_SKIP(devIndex);

        rc = aas_appDemoInitSequence_part2(boardRevId, devIndex , devNum);
        if (rc != GT_OK)
        {
            RESILIENT_INIT_SET_OR_RETURN(rc,resilientInit,devIndex);
        }
    }
    END_LOOP_ALL_DEVICES

    /* init the event handlers */
    rc = aas_EventHandlerInit();
    CPSS_ENABLER_DBG_TRACE_RC_MAC("hawk_EventHandlerInit", rc);
    if (rc != GT_OK)
        return rc;

    rc = cpssSystemRecoveryStateGet(&system_recovery);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (CPSS_SYSTEM_RECOVERY_PROCESS_NOT_ACTIVE_E == system_recovery.systemRecoveryProcess)
    {
        START_LOOP_ALL_DEVICES(devIndex,devNum)
        {
            /* Data Integrity module initialization. It should be done after events init. */
            RESILIENT_INIT_CHECK_AND_SKIP(devIndex);

            rc = appDemoDataIntegrityInit((GT_U8)devNum);
            CPSS_ENABLER_DBG_TRACE_RC_MAC("appDemoDataIntegrityInit", rc);
            if(GT_OK != rc)
            {
                RESILIENT_INIT_SET_OR_RETURN(rc,resilientInit,devIndex);
            }
        }
        END_LOOP_ALL_DEVICES
    }

    rc = aas_localUtfInit();
    CPSS_ENABLER_DBG_TRACE_RC_MAC("hawk_localUtfInit", rc);
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

    cpssOsPrintf("Time processing the cpssInitSystem (from 'phase1 init') is [%d] seconds + [%d] nanoseconds \n" , diff_sec , diff_nsec);

    systemInitialized = GT_TRUE;


    /* allow interrupts / appDemo tasks to stable */
    osTimerWkAfter(500);

    START_LOOP_ALL_DEVICES(devIndex,devNum)
    {
        GT_U32  portNum;
        GT_BOOL isLinkUp;

        /* the WM interrupt tree and the CPSS interrupt tree not support the ports from port 20 and above */
        /* this is needed for ports that got created and connected to */
        for(portNum = 20 ; portNum < (PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles*80)+1 ; portNum ++)
        {
            isLinkUp = GT_FALSE;
            rc = cpssDxChPortLinkStatusGet(CAST_SW_DEVNUM(devNum),portNum,&isLinkUp);
            if(rc != GT_OK)
            {
                continue;
            }

            if(GT_TRUE == isLinkUp)
            {
                appDemoLinkChngEventSimulate(CAST_SW_DEVNUM(devNum),portNum);
            }
        }
    }
    END_LOOP_ALL_DEVICES


    return GT_OK;
}

/**
* @internal gtDbAasBoardReg_BoardCleanDbDuringSystemReset function
* @endinternal
*
* @brief   clear the DB of the specific board config file , as part of the 'system rest'
*         to allow the 'cpssInitSystem' to run again as if it is the first time it runs
* @param[in] boardRevId               - The board revision Id.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS gtDbAasBoardReg_BoardCleanDbDuringSystemReset
(
    IN  GT_U8   boardRevId
)
{
    UNUSED_PARAM_MAC(boardRevId);

    appDemo_PortsInitList_already_done = 0;

    return GT_OK;
}
/**
* @internal gtDbDxAasBoardReg function
* @endinternal
*
* @brief   Registration function for the AAS board .
*
* @param[in] boardRevId               - The board revision Id.
*
* @param[out] boardCfgFuncs            - Board configuration functions.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS gtDbDxAasBoardReg
(
    IN  GT_U8                   boardRevId,
    OUT GT_BOARD_CONFIG_FUNCS   *boardCfgFuncs
)
{
    UNUSED_PARAM_MAC(boardRevId);

    /* set the actual pointer to use for this board */
    boardCfgFuncs->boardSimpleInit               = gtDbAasBoardReg_SimpleInit;
    boardCfgFuncs->boardCleanDbDuringSystemReset = gtDbAasBoardReg_BoardCleanDbDuringSystemReset;

    return GT_OK;
}

/**
* @internal aas_BindAllPortsToDefaultTdProfile function
* @endinternal
*
* @brief   Bind all ports and port queues to default tail drop profile.
*
* @param[in] devNum                - device number.
* @param[in] portDefaultProfile    - Port default tail drop profile.
* @param[in] queueDefaultProfile   - Queue default tail drop profile.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS aas_BindAllPortsToDefaultTdProfile
(
    IN  GT_U8   devNum,
    IN  CPSS_PORT_TX_DROP_PROFILE_SET_ENT portDefaultProfile,
    IN  GT_U32   queueDefaultProfile
)
{
    GT_PHYSICAL_PORT_NUM                    portNum;
    GT_U32                                  portMaxIter;/* maximal port number */
    GT_STATUS                               rc;
    GT_U32                                  queue;
    CPSS_DXCH_DETAILED_PORT_MAP_STC         portMapShadow;

    portMaxIter = PRV_CPSS_DXCH_MAX_PHY_PORT_NUMBER_MAC(devNum);

    for (portNum = 0; portNum < portMaxIter; portNum++)
    {
        rc =cpssDxChPortPhysicalPortDetailedMapGet(devNum,portNum,&portMapShadow);
        if (rc != GT_OK)
        {
            return rc;
        }

        if(portMapShadow.valid == GT_TRUE &&
                portMapShadow.portMap.mappingType != CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E)
        {
            rc = cpssDxChPortTxBindPortToDpSet(devNum,portNum,portDefaultProfile);
            if (rc != GT_OK)
                return rc;

            for(queue=0;queue<portMapShadow.extPortMap.txqInfo.numberOfQueues;queue++)
            {
                rc = cpssDxChPortTxBindPortQueueToDpSet(devNum,portNum,queue,queueDefaultProfile);
                if (rc != GT_OK)
                    return rc;
            }
        }
    }

    return GT_OK;
}


/**
* @internal aas_PortLoopbackShow function
* @endinternal
*
* @brief    Show Loopback Ports config/status used on Init but also in run-time.
*           Show-output is like:
*    |----+------+-------+--------------+------------+----+-----+-----+-----+----|
*    | dev| Port | Speed | MAC IF MODE  |ResourcePort| mac| chnl| pipe| tile| dp |
*    |----+------+-------+--------------+------------+----+-----+-----+-----+----|
*    |  0 |  128 |   NA  |  NA (LoopBk) |    128     | 84 |  20 |   0 |   0 |  0 |
*    |  0 |  129 |  25G  |  NA (LoopBk) |    129     | 85 |  20 |   0 |   0 |  1 |
*    |  0 |  130 |  400G |  NA (LoopBk) |     28     | 86 |  20 |   0 |   0 |  2 |
*    |  0 |  131 |  50G  |  NA (LoopBk) |    131     | 87 |  20 |   0 |   0 |  3 |
*    |----+------+-------+--------------+------------+----+-----+-----+-----+----|
*
* @note   APPLICABLE DEVICES:     AAS.
*
* @param[in] devNum                 - device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameter value
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS aas_PortLoopbackShow
(
    IN  GT_U8                       devNum
)
{
    GT_STATUS                       rc;
    GT_U32                          ii; /*port*/
    GT_U32                          resourcePairPort;
    GT_U32                          dp;
    GT_U32                          localDma;
    GT_U32                          ifNum;
    CPSS_DXCH_DETAILED_PORT_MAP_STC portMapDetailed;
    CPSS_DXCH_PORT_MAP_STC          portMap;
    PRV_CPSS_PORT_INFO_ARRAY_STC    *portInfo;
    CPSS_PORT_SPEED_ENT             speed;

    PRV_CPSS_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_AAS_E);

    cpssOsPrintf("|----+------+-------+----------+------------+--------+-----+-----+-----+----|\n");
    cpssOsPrintf("| dev| Port | Speed | MAP TYPE |ResourcePort| globDMA| chnl| pipe| tile| dp |\n");
    cpssOsPrintf("|----+------+-------+----------+------------+--------+-----+-----+-----+----|\n");

    for (ii = 0; ii < PRV_CPSS_MAX_PP_PORTS_NUM_CNS; ii++)
    {
        rc = cpssDxChPortPhysicalPortDetailedMapGet(devNum, ii, &portMapDetailed);
        if (rc != GT_OK || portMapDetailed.valid != GT_TRUE ||
            portMapDetailed.portMap.mappingType != CPSS_DXCH_PORT_MAPPING_TYPE_LOOPBACK_E)
            continue;

        rc = cpssDxChPortPhysicalPortMapGet(devNum, ii, 1, &portMap);
        if (rc != GT_OK)
            continue;

        rc = cpssDxChPortSpeedGet(devNum, ii, &speed);
        if (rc != GT_OK)
            continue;

        dp = portMapDetailed.extPortMap.localDpInTile;
        localDma = portMapDetailed.extPortMap.localPortInDp;
        ifNum = portMap.interfaceNum; /* global-DMA, NOT the portMapDetailed.portMap.macNum=-1 */
        resourcePairPort = ii;

        if (speed != CPSS_PORT_SPEED_NA_E)
        {
            /* Get paired resource port */
            portInfo = &PRV_CPSS_PP_MAC(devNum)->phyPortInfoArray[ifNum];
            resourcePairPort = portInfo->pcaLoopbackResourcePort;
        }
        /*            | dev| Port | Speed | MAP TYPE |ResourcePort| globDMA| chnl| pipe| tile| dp | */
        /*              /      /     /       /          /            /       /     /     /     /    */
        cpssOsPrintf("|%3d |  %3d | %s | LOOPBACK |    %4d    |    %2d  | %3d | %3d | %3d | %2d |\n",
                    devNum, ii, CPSS_SPEED_2_STR(speed),
                                             resourcePairPort, ifNum, localDma, 0,0, dp);
    }
    cpssOsPrintf("|----+------+-------+----------+------------+--------+-----+-----+-----+----|\n");
    return GT_OK;
}

/**
* @internal aas_SpecialPortsInit function
* @endinternal
*
* @brief   Default config speed 25G and linkUp for all special Loopback ports
*
* @param[in] devNum                - device number.
*
* @retval GT_OK                    - on success
* @retval GT_NOT_INITIALIZED       - if port was not initialized
*                                     (does not exist in port DB)
* @retval GT_NO_RESOURCE           - not enought resources for requested speed
* @retval GT_BAD_STATE             - requested port is not free
*
* @note   Should be called after appDemoDxLibrariesInit making LinkForceDown
*/
static GT_STATUS aas_SpecialPortsInit
(
    IN  GT_U8   devNum
)
{
    GT_U32                                  portMaxIter;/* maximal port number */
    GT_PHYSICAL_PORT_NUM                    portNum;
    CPSS_DXCH_DETAILED_PORT_MAP_STC         portMapShadow;
    GT_STATUS                               rc;

    portMaxIter = PRV_CPSS_DXCH_MAX_PHY_PORT_NUMBER_MAC(devNum);

    for (portNum = 0; portNum < portMaxIter; portNum++)
    {
        rc = cpssDxChPortPhysicalPortDetailedMapGet(devNum, portNum, &portMapShadow);
        if (rc != GT_OK)
        {
            return rc;
        }
        if(portMapShadow.valid != GT_TRUE ||
           portMapShadow.portMap.mappingType != CPSS_DXCH_PORT_MAPPING_TYPE_LOOPBACK_E)
        {
            continue;
        }
        /* Loopback Port found (refer LOOPBACK_PHYSICAL_PORT(_dp) in this file */
        rc = cpssDxChPortEgressLoopbackSpeedSet(devNum, portNum, portNum, CPSS_PORT_SPEED_25000_E);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    aas_PortLoopbackShow(devNum);

    return GT_OK;
}
