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
* @file cpssAppUtilsCommon.c
*
* @brief Includes board specific initialization definitions and data-structures.
*
* @version   1
********************************************************************************
*/

#ifdef LINUX
#define _BSD_SOURCE
#define __USE_XOPEN2K
#define _DEFAULT_SOURCE
#include <unistd.h>
#include <stdlib.h>
#endif /*LINUX*/

#include <cpssAppUtilsCommon.h>
#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <galtisAgent/wrapUtil/cmdCpssPresteraUtils.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <gtOs/gtOsGen.h>
#include <gtOs/gtOsSem.h>
#include <cpss/extServices/os/gtOs/gtOs.h>
#include <cmdShell/common/cmdExtServices.h>
#include <gtOs/gtOsMem.h>
#include <gtStack/gtOsSocket.h>
#ifdef ASIC_SIMULATION
#include <os/simTypesBind.h>
#define EXPLICIT_INCLUDE_TO_SIM_OS_H_FILES
#include <os/simOsBindOwn.h>
#undef EXPLICIT_INCLUDE_TO_SIM_OS_H_FILES
#endif /*ASIC_SIMULATION*/
#include <gtOs/gtOsIo.h>
#include <cmdShell/shell/cmdMain.h>
#include <gtExtDrv/drivers/gtCacheMng.h>
#include <gtExtDrv/drivers/gtSmiHwCtrl.h>
#include <gtExtDrv/drivers/gtDmaDrv.h>
#include <gtExtDrv/drivers/gtEthPortCtrl.h>
#include <gtExtDrv/drivers/gtIntDrv.h>
#include <gtExtDrv/drivers/gtPciDrv.h>
#include <gtExtDrv/drivers/gtTwsiHwCtrl.h>
#include <gtExtDrv/drivers/gtDmaDrv.h>
#include <gtExtDrv/drivers/gtHsuDrv.h>
#include <gtExtDrv/drivers/gtI2cDrv.h>
#include <gtExtDrv/drivers/gtHwIfDrv.h>
#include <gtExtDrv/drivers/gtDragoniteDrv.h>
#include <cpss/common/init/cpssInit.h>
#include <cpss/extServices/cpssExtServices.h>
#include <gtStack/gtOsSocket.h>
#include <gtOs/gtOsInet.h>
#include <gtOs/gtOsMsgQ.h>
#include <gtOs/gtOsStdLib.h>
#include <gtOs/gtOsExc.h>
#include <gtOs/gtOsTask.h>
#include <gtOs/gtOsTimer.h>
#include <gtUtil/gtBmPool.h>
#include <cpss/generic/events/cpssGenEventRequests.h>
#if !defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)
#include <appDemo/boardConfig/appDemoBoardConfig.h>
#include <appDemo/sysHwConfig/appDemoDb.h>
#include <appDemo/utils/mainUT/appDemoUtils.h>
#include <appDemo/confi/confi2.h>
#ifdef CHX_FAMILY
#include <appDemo/utils/hsu/appDemoDxChHsuUtils.h>
#endif
#else
#include <profiles/cpssAppPlatformProfile.h>
#include <cpssAppPlatformBoardConfig.h>
#endif /*!defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)*/
#if defined(SHARED_MEMORY)
#include <gtOs/gtOsSharedFunctionPointers.h>
#endif /*defined(SHARED_MEMORY)*/
#include <utfExtHelpers.h>

#ifdef LINUX
#ifndef ASIC_SIMULATION
/*Linux*/
#define __NO_STRING_INLINES
#include <gtOs/gtOsSharedMemory.h>
#include <gtOs/gtOsInit.h>
#include <appDemo/sysHwConfig/gtAppDemoSysConfig.h>
#include <cmdShell/shell/cmdMain.h>
#define CMDOS_NO_CMDCOMMON
#include <cmdShell/os/cmdOs.h>
#ifdef CMD_LUA_CLI
#include <termios.h>
#endif
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <cpss/generic/version/cpssGenStream.h>
#ifdef CHX_FAMILY
#include <cpss/dxCh/dxChxGen/version/cpssDxChVersion.h>
#endif
#ifdef PX_FAMILY
#include <cpss/px/version/cpssPxVersion.h>
#endif

extern GT_STATUS userAppInit(void);

#else /*ASIC_SIMULATION*/

/*LinuxSim*/
#include <ctype.h>
#ifdef __USE_BSD
#undef __USE_BSD
#endif
#define __USE_BSD
#ifdef __USE_MISC
#undef __USE_MISC
#endif
#define __USE_MISC
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#ifdef CMD_LUA_CLI
#include <termios.h>
#endif
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <gtOs/gtOsGen.h>
#include <gtOs/gtOsMem.h>
#include <gtOs/gtOsSem.h>
#include <gtOs/gtOsInit.h>
#include <gtOs/gtOsSharedMemory.h>
#include <gtStack/gtOsSocket.h>
#define CMDOS_NO_CMDCOMMON
#include <cmdShell/os/cmdOs.h>
#include <cpss/generic/version/cpssGenStream.h>
#include <asicSimulation/SInit/sinit.h>
#include <os/simTypesBind.h>
#ifdef CHX_FAMILY
#include <cpss/dxCh/dxChxGen/version/cpssDxChVersion.h>
#endif
#ifdef PX_FAMILY
#include <cpss/px/version/cpssPxVersion.h>
#endif
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <gtOs/gtOsSharedPp.h>
#define EXPLICIT_INCLUDE_TO_SIM_OS_H_FILES
#include <os/simOsBindOwn.h>
#include <cpssAppUtilsCommon.h>

/* commandLine should belong to simulation (simulation can be used by another client withot this var) */
extern char commandLine[1280];

#endif /*ASIC_SIMULATION*/
#endif /*LINUX*/


CPSS_ENABLER_DB_ENTRY_STC    prvCpssDb[CPSS_ENABLER_DB_MAX_SIZE_CNS];
GT_U32                       prvCpssDbSize = 0;


GT_BOOL appDemoCheckEnable;

#ifdef GM_USED
GT_U32  useSinglePipeSingleDp = 1;
#else /*!GM_USED*/
GT_U32  useSinglePipeSingleDp = 0;
#endif /*!GM_USED*/


GT_BOOL appDemoCheckEnableGet()
{
    return appDemoCheckEnable;
}

void appDemoCheckEnableSet(GT_BOOL checkEnable)
{
    appDemoCheckEnable = checkEnable;
}


/**********************************************************************************
        if multiProcessAppDemo = GT_FALSE the appDemo run as regular (one process)
        if multiProcessAppDemo = GT_TRUE the appDemo run as multiprocess application
                the multiprocess application includes:
                - appDemo process
                - fdb learning process
                - RxTx process
        the variable multiProcessAppDemo used in file userEventHandler.c to define
        the event handler configuration
*********************************************************************************/
GT_BOOL multiProcessAppDemo = GT_FALSE;

GT_U32 appDemoDbSizeGet()
{
    return prvCpssDbSize;
}

CPSS_ENABLER_DB_ENTRY_STC appDemoDbIndexedEntryGet(GT_U32 index)
{
    return prvCpssDb[index];
}


#ifdef INCLUDE_MPD

GT_APPDEMO_XPHY_STATIC_INFO_STC portToPhyArrayAc5xRd2580_SR2[] = {
    {32,0x0,6, MPD_PHY_USX_TYPE_OXGMII, MPD_TRANSCEIVER_COPPER_E},
    {33,0x1,6, MPD_PHY_USX_TYPE_OXGMII, MPD_TRANSCEIVER_COPPER_E},
    {34,0x2,6, MPD_PHY_USX_TYPE_OXGMII, MPD_TRANSCEIVER_COPPER_E},
    {35,0x3,6, MPD_PHY_USX_TYPE_OXGMII, MPD_TRANSCEIVER_COPPER_E},
    {36,0x4,6, MPD_PHY_USX_TYPE_OXGMII, MPD_TRANSCEIVER_COPPER_E},
    {37,0x5,6, MPD_PHY_USX_TYPE_OXGMII, MPD_TRANSCEIVER_COPPER_E},
    {38,0x6,6, MPD_PHY_USX_TYPE_OXGMII, MPD_TRANSCEIVER_COPPER_E},
    {39,0x7,6, MPD_PHY_USX_TYPE_OXGMII, MPD_TRANSCEIVER_COPPER_E},
    {0xff, 0xff, 0xff, MPD_PHY_USX_TYPE_NONE, MPD_TRANSCEIVER_COPPER_E}
};

GT_APPDEMO_XPHY_STATIC_INFO_STC portToPhyArrayAc5xRd2540_SR2[] = {
    {40,0x8,4, MPD_PHY_USX_TYPE_OXGMII, MPD_TRANSCEIVER_COPPER_E},
    {41,0x9,4, MPD_PHY_USX_TYPE_OXGMII, MPD_TRANSCEIVER_COPPER_E},
    {42,0xa,4, MPD_PHY_USX_TYPE_OXGMII, MPD_TRANSCEIVER_COPPER_E},
    {43,0xb,4, MPD_PHY_USX_TYPE_OXGMII, MPD_TRANSCEIVER_COPPER_E},
    {44,0xc,5, MPD_PHY_USX_TYPE_OXGMII, MPD_TRANSCEIVER_COPPER_E},
    {45,0xd,5, MPD_PHY_USX_TYPE_OXGMII, MPD_TRANSCEIVER_COPPER_E},
    {46,0xe,5, MPD_PHY_USX_TYPE_OXGMII, MPD_TRANSCEIVER_COPPER_E},
    {47,0xf,5, MPD_PHY_USX_TYPE_OXGMII, MPD_TRANSCEIVER_COPPER_E},
    {0xff, 0xff, 0xff, MPD_PHY_USX_TYPE_NONE, MPD_TRANSCEIVER_COPPER_E}
};

GT_APPDEMO_XPHY_STATIC_INFO_STC portToPhyArrayAc5pRd2580_SR1[] = {
    {16, 0x1, 4, MPD_PHY_USX_TYPE_20G_QXGMII, MPD_TRANSCEIVER_COPPER_E}, /* PanelPort[33] E2580_#1 SRDS[5] 20G_QXGMII */
    {14, 0x0, 4, MPD_PHY_USX_TYPE_20G_QXGMII, MPD_TRANSCEIVER_COPPER_E}, /* PanelPort[34] E2580_#1 SRDS[5] 20G_QXGMII */
    {17, 0x3, 4, MPD_PHY_USX_TYPE_20G_QXGMII, MPD_TRANSCEIVER_COPPER_E}, /* PanelPort[35] E2580_#1 SRDS[5] 20G_QXGMII */
    {15, 0x2, 4, MPD_PHY_USX_TYPE_20G_QXGMII, MPD_TRANSCEIVER_COPPER_E}, /* PanelPort[36] E2580_#1 SRDS[5] 20G_QXGMII */
    {12, 0x5, 4, MPD_PHY_USX_TYPE_20G_QXGMII, MPD_TRANSCEIVER_COPPER_E}, /* PanelPort[37] E2580_#1 SRDS[4] 20G_QXGMII */
    {10, 0x4, 4, MPD_PHY_USX_TYPE_20G_QXGMII, MPD_TRANSCEIVER_COPPER_E}, /* PanelPort[38] E2580_#1 SRDS[4] 20G_QXGMII */
    {13, 0x7, 4, MPD_PHY_USX_TYPE_20G_QXGMII, MPD_TRANSCEIVER_COPPER_E}, /* PanelPort[39] E2580_#1 SRDS[4] 20G_QXGMII */
    {11, 0x6, 4, MPD_PHY_USX_TYPE_20G_QXGMII, MPD_TRANSCEIVER_COPPER_E}, /* PanelPort[40] E2580_#1 SRDS[4] 20G_QXGMII */

    { 8, 0x9, 5, MPD_PHY_USX_TYPE_20G_QXGMII, MPD_TRANSCEIVER_COPPER_E}, /* PanelPort[41] E2580_#2 SRDS[3] 20G_QXGMII */
    { 6, 0x8, 5, MPD_PHY_USX_TYPE_20G_QXGMII, MPD_TRANSCEIVER_COPPER_E}, /* PanelPort[42] E2580_#2 SRDS[3] 20G_QXGMII */
    { 9, 0xb, 5, MPD_PHY_USX_TYPE_20G_QXGMII, MPD_TRANSCEIVER_COPPER_E}, /* PanelPort[43] E2580_#2 SRDS[3] 20G_QXGMII */
    { 7, 0xa, 5, MPD_PHY_USX_TYPE_20G_QXGMII, MPD_TRANSCEIVER_COPPER_E}, /* PanelPort[44] E2580_#2 SRDS[3] 20G_QXGMII */
    { 4, 0xd, 5, MPD_PHY_USX_TYPE_20G_QXGMII, MPD_TRANSCEIVER_COPPER_E}, /* PanelPort[45] E2580_#2 SRDS[2] 20G_QXGMII */
    { 2, 0xc, 5, MPD_PHY_USX_TYPE_20G_QXGMII, MPD_TRANSCEIVER_COPPER_E}, /* PanelPort[46] E2580_#2 SRDS[2] 20G_QXGMII */
    { 5, 0xf, 5, MPD_PHY_USX_TYPE_20G_QXGMII, MPD_TRANSCEIVER_COPPER_E}, /* PanelPort[47] E2580_#2 SRDS[2] 20G_QXGMII */
    { 3, 0xe, 5, MPD_PHY_USX_TYPE_20G_QXGMII, MPD_TRANSCEIVER_COPPER_E}, /* PanelPort[47] E2580_#2 SRDS[2] 20G_QXGMII */
    {0xff, 0xff, 0xff, MPD_PHY_USX_TYPE_NONE, MPD_TRANSCEIVER_COPPER_E}
};

GT_APPDEMO_XPHY_STATIC_INFO_STC portToPhyArrayAc5pRd1780[] = {
    {30, 0x0, 0, MPD_PHY_USX_TYPE_NONE, MPD_TRANSCEIVER_COPPER_E},
    {28, 0x1, 0, MPD_PHY_USX_TYPE_NONE, MPD_TRANSCEIVER_COPPER_E},
    {31, 0x2, 0, MPD_PHY_USX_TYPE_NONE, MPD_TRANSCEIVER_COPPER_E},
    {29, 0x3, 0, MPD_PHY_USX_TYPE_NONE, MPD_TRANSCEIVER_COPPER_E},
    {34, 0x4, 0, MPD_PHY_USX_TYPE_NONE, MPD_TRANSCEIVER_COPPER_E},
    {32, 0x5, 0, MPD_PHY_USX_TYPE_NONE, MPD_TRANSCEIVER_COPPER_E},
    {35, 0x6, 0, MPD_PHY_USX_TYPE_NONE, MPD_TRANSCEIVER_COPPER_E},
    {33, 0x7, 0, MPD_PHY_USX_TYPE_NONE, MPD_TRANSCEIVER_COPPER_E},
    {38, 0x8, 1, MPD_PHY_USX_TYPE_NONE, MPD_TRANSCEIVER_COPPER_E},
    {36, 0x9, 1, MPD_PHY_USX_TYPE_NONE, MPD_TRANSCEIVER_COPPER_E},
    {39,0xa , 1, MPD_PHY_USX_TYPE_NONE, MPD_TRANSCEIVER_COPPER_E},
    {37,0xb , 1, MPD_PHY_USX_TYPE_NONE, MPD_TRANSCEIVER_COPPER_E},
    {42,0xc , 1, MPD_PHY_USX_TYPE_NONE, MPD_TRANSCEIVER_COPPER_E},
    {40,0xd , 1, MPD_PHY_USX_TYPE_NONE, MPD_TRANSCEIVER_COPPER_E},
    {43,0xe , 1, MPD_PHY_USX_TYPE_NONE, MPD_TRANSCEIVER_COPPER_E},
    {41,0xf , 1, MPD_PHY_USX_TYPE_NONE, MPD_TRANSCEIVER_COPPER_E},
    {46,0x10, 2, MPD_PHY_USX_TYPE_NONE, MPD_TRANSCEIVER_COPPER_E},
    {44,0x11, 2, MPD_PHY_USX_TYPE_NONE, MPD_TRANSCEIVER_COPPER_E},
    {47,0x12, 2, MPD_PHY_USX_TYPE_NONE, MPD_TRANSCEIVER_COPPER_E},
    {45,0x13, 2, MPD_PHY_USX_TYPE_NONE, MPD_TRANSCEIVER_COPPER_E},
    {50,0x14, 2, MPD_PHY_USX_TYPE_NONE, MPD_TRANSCEIVER_COPPER_E},
    {48,0x15, 2, MPD_PHY_USX_TYPE_NONE, MPD_TRANSCEIVER_COPPER_E},
    {51,0x16, 2, MPD_PHY_USX_TYPE_NONE, MPD_TRANSCEIVER_COPPER_E},
    {49,0x17, 2, MPD_PHY_USX_TYPE_NONE, MPD_TRANSCEIVER_COPPER_E},
    {20,0x18, 3, MPD_PHY_USX_TYPE_NONE, MPD_TRANSCEIVER_COPPER_E},
    {18,0x19, 3, MPD_PHY_USX_TYPE_NONE, MPD_TRANSCEIVER_COPPER_E},
    {21,0x1a, 3, MPD_PHY_USX_TYPE_NONE, MPD_TRANSCEIVER_COPPER_E},
    {19,0x1b, 3, MPD_PHY_USX_TYPE_NONE, MPD_TRANSCEIVER_COPPER_E},
    {24,0x1c, 3, MPD_PHY_USX_TYPE_NONE, MPD_TRANSCEIVER_COPPER_E},
    {22,0x1d, 3, MPD_PHY_USX_TYPE_NONE, MPD_TRANSCEIVER_COPPER_E},
    {25,0x1e, 3, MPD_PHY_USX_TYPE_NONE, MPD_TRANSCEIVER_COPPER_E},
    {23,0x1f, 3, MPD_PHY_USX_TYPE_NONE, MPD_TRANSCEIVER_COPPER_E},
    {0xff, 0xff, 0xff, MPD_PHY_USX_TYPE_NONE, MPD_TRANSCEIVER_COPPER_E}
};

GT_APPDEMO_XPHY_STATIC_INFO_STC portToPhyArrayAc5xRd1780[] = {
    {0, 0x0, 0, MPD_PHY_USX_TYPE_NONE, MPD_TRANSCEIVER_COPPER_E},
    {1, 0x1, 0, MPD_PHY_USX_TYPE_NONE, MPD_TRANSCEIVER_COPPER_E},
    {2, 0x2, 0, MPD_PHY_USX_TYPE_NONE, MPD_TRANSCEIVER_COPPER_E},
    {3, 0x3, 0, MPD_PHY_USX_TYPE_NONE, MPD_TRANSCEIVER_COPPER_E},
    {4, 0x4, 0, MPD_PHY_USX_TYPE_NONE, MPD_TRANSCEIVER_COPPER_E},
    {5, 0x5, 0, MPD_PHY_USX_TYPE_NONE, MPD_TRANSCEIVER_COPPER_E},
    {6, 0x6, 0, MPD_PHY_USX_TYPE_NONE, MPD_TRANSCEIVER_COPPER_E},
    {7, 0x7, 0, MPD_PHY_USX_TYPE_NONE, MPD_TRANSCEIVER_COPPER_E},
    {8, 0x8, 1, MPD_PHY_USX_TYPE_NONE, MPD_TRANSCEIVER_COPPER_E},
    {9, 0x9, 1, MPD_PHY_USX_TYPE_NONE, MPD_TRANSCEIVER_COPPER_E},
    {10,0xa, 1, MPD_PHY_USX_TYPE_NONE, MPD_TRANSCEIVER_COPPER_E},
    {11,0xb, 1, MPD_PHY_USX_TYPE_NONE, MPD_TRANSCEIVER_COPPER_E},
    {12,0xc, 1, MPD_PHY_USX_TYPE_NONE, MPD_TRANSCEIVER_COPPER_E},
    {13,0xd, 1, MPD_PHY_USX_TYPE_NONE, MPD_TRANSCEIVER_COPPER_E},
    {14,0xe, 1, MPD_PHY_USX_TYPE_NONE, MPD_TRANSCEIVER_COPPER_E},
    {15,0xf, 1, MPD_PHY_USX_TYPE_NONE, MPD_TRANSCEIVER_COPPER_E},
    {16,0x10,2, MPD_PHY_USX_TYPE_NONE, MPD_TRANSCEIVER_COPPER_E},
    {17,0x11,2, MPD_PHY_USX_TYPE_NONE, MPD_TRANSCEIVER_COPPER_E},
    {18,0x12,2, MPD_PHY_USX_TYPE_NONE, MPD_TRANSCEIVER_COPPER_E},
    {19,0x13,2, MPD_PHY_USX_TYPE_NONE, MPD_TRANSCEIVER_COPPER_E},
    {20,0x14,2, MPD_PHY_USX_TYPE_NONE, MPD_TRANSCEIVER_COPPER_E},
    {21,0x15,2, MPD_PHY_USX_TYPE_NONE, MPD_TRANSCEIVER_COPPER_E},
    {22,0x16,2, MPD_PHY_USX_TYPE_NONE, MPD_TRANSCEIVER_COPPER_E},
    {23,0x17,2, MPD_PHY_USX_TYPE_NONE, MPD_TRANSCEIVER_COPPER_E},
    {24,0x18,3, MPD_PHY_USX_TYPE_NONE, MPD_TRANSCEIVER_COPPER_E},
    {25,0x19,3, MPD_PHY_USX_TYPE_NONE, MPD_TRANSCEIVER_COPPER_E},
    {26,0x1a,3, MPD_PHY_USX_TYPE_NONE, MPD_TRANSCEIVER_COPPER_E},
    {27,0x1b,3, MPD_PHY_USX_TYPE_NONE, MPD_TRANSCEIVER_COPPER_E},
    {28,0x1c,3, MPD_PHY_USX_TYPE_NONE, MPD_TRANSCEIVER_COPPER_E},
    {29,0x1d,3, MPD_PHY_USX_TYPE_NONE, MPD_TRANSCEIVER_COPPER_E},
    {30,0x1e,3, MPD_PHY_USX_TYPE_NONE, MPD_TRANSCEIVER_COPPER_E},
    {31,0x1f,3, MPD_PHY_USX_TYPE_NONE, MPD_TRANSCEIVER_COPPER_E},
    {0xff, 0xff, 0xff, MPD_PHY_USX_TYPE_NONE, MPD_TRANSCEIVER_COPPER_E}
};

GT_APPDEMO_XPHY_STATIC_INFO_STC portToPhyArrayIronmanL_Db1781[] = {
    {0, 0x0, 0, MPD_PHY_USX_TYPE_NONE, MPD_TRANSCEIVER_COPPER_E},
    {1, 0x1, 0, MPD_PHY_USX_TYPE_NONE, MPD_TRANSCEIVER_COPPER_E},
    {2, 0x2, 0, MPD_PHY_USX_TYPE_NONE, MPD_TRANSCEIVER_COPPER_E},
    {3, 0x3, 0, MPD_PHY_USX_TYPE_NONE, MPD_TRANSCEIVER_COPPER_E},
    {4, 0x4, 0, MPD_PHY_USX_TYPE_NONE, MPD_TRANSCEIVER_COPPER_E},
    {5, 0x5, 0, MPD_PHY_USX_TYPE_NONE, MPD_TRANSCEIVER_COPPER_E},
    {6, 0x6, 0, MPD_PHY_USX_TYPE_NONE, MPD_TRANSCEIVER_COPPER_E},
    {7, 0x7, 0, MPD_PHY_USX_TYPE_NONE, MPD_TRANSCEIVER_COPPER_E},
    {8, 0x8, 1, MPD_PHY_USX_TYPE_NONE, MPD_TRANSCEIVER_COPPER_E},
    {9, 0x9, 1, MPD_PHY_USX_TYPE_NONE, MPD_TRANSCEIVER_COPPER_E},
    {10,0xa , 1, MPD_PHY_USX_TYPE_NONE, MPD_TRANSCEIVER_COPPER_E},
    {11,0xb , 1, MPD_PHY_USX_TYPE_NONE, MPD_TRANSCEIVER_COPPER_E},
    {12,0xc , 1, MPD_PHY_USX_TYPE_NONE, MPD_TRANSCEIVER_COPPER_E},
    {13,0xd , 1, MPD_PHY_USX_TYPE_NONE, MPD_TRANSCEIVER_COPPER_E},
    {14,0xe , 1, MPD_PHY_USX_TYPE_NONE, MPD_TRANSCEIVER_COPPER_E},
    {15,0xf , 1, MPD_PHY_USX_TYPE_NONE, MPD_TRANSCEIVER_COPPER_E},
    {0xff, 0xff, 0xff, MPD_PHY_USX_TYPE_NONE, MPD_TRANSCEIVER_COPPER_E}
};

GT_APPDEMO_XPHY_STATIC_INFO_STC portToPhyArrayIronmanRd24g6xg1781[] = {
    {0, 0x7, 0, MPD_PHY_USX_TYPE_NONE, MPD_TRANSCEIVER_COPPER_E},
    {1, 0x6, 0, MPD_PHY_USX_TYPE_NONE, MPD_TRANSCEIVER_COPPER_E},
    {2, 0x5, 0, MPD_PHY_USX_TYPE_NONE, MPD_TRANSCEIVER_COPPER_E},
    {3, 0x4, 0, MPD_PHY_USX_TYPE_NONE, MPD_TRANSCEIVER_COPPER_E},
    {4, 0x3, 0, MPD_PHY_USX_TYPE_NONE, MPD_TRANSCEIVER_COPPER_E},
    {5, 0x2, 0, MPD_PHY_USX_TYPE_NONE, MPD_TRANSCEIVER_COPPER_E},
    {6, 0x1, 0, MPD_PHY_USX_TYPE_NONE, MPD_TRANSCEIVER_COPPER_E},
    {7, 0x0, 0, MPD_PHY_USX_TYPE_NONE, MPD_TRANSCEIVER_COPPER_E},
    {8, 0xF, 1, MPD_PHY_USX_TYPE_NONE, MPD_TRANSCEIVER_COPPER_E},
    {9, 0xE, 1, MPD_PHY_USX_TYPE_NONE, MPD_TRANSCEIVER_COPPER_E},
    {10,0xD , 1, MPD_PHY_USX_TYPE_NONE, MPD_TRANSCEIVER_COPPER_E},
    {11,0xC , 1, MPD_PHY_USX_TYPE_NONE, MPD_TRANSCEIVER_COPPER_E},
    {12,0xB , 1, MPD_PHY_USX_TYPE_NONE, MPD_TRANSCEIVER_COPPER_E},
    {13,0xA , 1, MPD_PHY_USX_TYPE_NONE, MPD_TRANSCEIVER_COPPER_E},
    {14,0x9 , 1, MPD_PHY_USX_TYPE_NONE, MPD_TRANSCEIVER_COPPER_E},
    {15,0x8 , 1, MPD_PHY_USX_TYPE_NONE, MPD_TRANSCEIVER_COPPER_E},
    {16,0x17 , 2, MPD_PHY_USX_TYPE_NONE, MPD_TRANSCEIVER_COPPER_E},
    {17,0x16 , 2, MPD_PHY_USX_TYPE_NONE, MPD_TRANSCEIVER_COPPER_E},
    {18,0x15 , 2, MPD_PHY_USX_TYPE_NONE, MPD_TRANSCEIVER_COPPER_E},
    {19,0x14 , 2, MPD_PHY_USX_TYPE_NONE, MPD_TRANSCEIVER_COPPER_E},
    {20,0x13 , 2, MPD_PHY_USX_TYPE_NONE, MPD_TRANSCEIVER_COPPER_E},
    {21,0x12 , 2, MPD_PHY_USX_TYPE_NONE, MPD_TRANSCEIVER_COPPER_E},
    {22,0x11 , 2, MPD_PHY_USX_TYPE_NONE, MPD_TRANSCEIVER_COPPER_E},
    {23,0x10 , 2, MPD_PHY_USX_TYPE_NONE, MPD_TRANSCEIVER_COPPER_E},
    {0xff, 0xff, 0xff, MPD_PHY_USX_TYPE_NONE, MPD_TRANSCEIVER_COPPER_E}
};

#endif/* INCLUDE_MPD */

GT_BOOL appDemoCheckEnable = GT_TRUE;


/* app demo database */
CPSS_ENABLER_DB_ENTRY_STC    appDemoDb[CPSS_ENABLER_DB_MAX_SIZE_CNS];
GT_U32                   appDemoDbSize = 0;
GT_U32                   initDone = 0;



#ifdef IMPL_GALTIS
/**
* @internal cmdLibResetAppDemoDb function
* @endinternal
*
* @brief   Library database reset function.
*
* @note none
*
*/
static GT_VOID cmdLibResetAppDemoDb
(
    GT_VOID
)
{
    /* must not reset the DB ... to keep runtime parameters for next run */
}
#endif /*IMPL_GALTIS*/

/**
* @internal appDemoDbEntryAdd function
* @endinternal
*
* @brief   Set AppDemo DataBase value.This value will be considered during system
*         initialization process.
* @param[in] namePtr                  - points to parameter name
* @param[in] value                    - parameter value
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - if name is too long
* @retval GT_BAD_PTR               - if NULL pointer
* @retval GT_NO_RESOURCE           - if Database is full
*/
GT_STATUS appDemoDbEntryAdd
(
    IN  GT_CHAR *namePtr,
    IN  GT_U32  value
)
{
#ifdef IMPL_GALTIS
    GT_STATUS   rc;
#endif /*IMPL_GALTIS*/
    GT_U32 i;

    if(initDone == 0)
    {
#ifdef IMPL_GALTIS
        /* register function to reset DB during system reset */
        rc = wrCpssRegisterResetCb(cmdLibResetAppDemoDb);
        if(rc != GT_OK)
        {
            return rc;
        }
#endif /*IMPL_GALTIS*/
        initDone = 1;
    }

    /* check parameters */
    if (namePtr == NULL)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ERROR_NO_MSG);

    /* check database is not full */
    if (prvCpssDbSize >= CPSS_ENABLER_DB_MAX_SIZE_CNS)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, LOG_ERROR_NO_MSG);

    /* check name is not too long */
    if (cpssOsStrlen(namePtr) > CPSS_ENABLER_DB_NAME_MAX_LEN_CNS)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    /* search if the name already exists, if so override the value */
    for (i = 0 ; i < prvCpssDbSize ; i++)
    {
        if (cpssOsStrCmp(namePtr, prvCpssDb[i].name) == 0)
        {
            prvCpssDb[i].value = value;
            return GT_OK;
        }
    }

    /* the entry wasn't already in database, add it */
    cpssOsStrCpy(prvCpssDb[prvCpssDbSize].name, namePtr);
    prvCpssDb[prvCpssDbSize].value = value;
    prvCpssDbSize++;

    return GT_OK;
}

/**
* @internal appDemoDbEntryGet function
* @endinternal
*
* @brief   Get parameter value from AppDemo DataBase.
*
* @param[in] namePtr                  - points to parameter name
*
* @param[out] valuePtr                 - points to parameter value
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - if name is too long
* @retval GT_BAD_PTR               - if NULL pointer
* @retval GT_NO_SUCH               - there is no such parameter in Database
*/
GT_STATUS appDemoDbEntryGet
(
    IN  GT_CHAR *namePtr,
    OUT GT_U32  *valuePtr
)
{
    GT_U32 i;

    /* check parameters */
    if ((namePtr == NULL) || (valuePtr == NULL))
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ERROR_NO_MSG);

    /* check name is not too long */
    if (cpssOsStrlen(namePtr) > CPSS_ENABLER_DB_NAME_MAX_LEN_CNS)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    /* search the name in the databse */
    for (i = 0 ; i < prvCpssDbSize ; i++)
    {
        if (cpssOsStrCmp(namePtr, prvCpssDb[i].name) == 0)
        {
            *valuePtr = prvCpssDb[i].value;
            return GT_OK;
        }
    }

    /* the entry wasn't found */
    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_SUCH, LOG_ERROR_NO_MSG);
}

extern GT_STATUS prvAppDemoEventRequestDrvnModeInit
(
    IN GT_VOID
);
extern GT_STATUS appPlatformEventRequestDrvnModeInit
(
    GT_VOID
);
extern GT_STATUS cpssAppDemoReInitSystem
(
    GT_VOID
);
extern GT_STATUS cpssAppPlatformReInitSystem
(
    GT_VOID
);
extern GT_STATUS cpssAppPlatformDevSupportSystemResetSet(

    IN  GT_U8       devNum,
    IN  GT_BOOL     enable
);
extern GT_STATUS cpssAppDemoDevSupportSystemResetSet(

    IN  GT_U8       devNum,
    IN  GT_BOOL     enable
);
extern GT_STATUS cpssAppPlatformResetSystem (

    IN  GT_BOOL     doHwReset
);
extern GT_STATUS cpssAppDemoResetSystem(
    IN GT_BOOL doHwReset
);
extern const char *VERSION_DATE;
extern const char *VERSION_FLAGS;

#if defined(LINUX)

extern GT_U32 osGetPid
(
    GT_VOID
);

#endif




GT_U32   phoenix_only_ports_of_100G_mac = 0; /* the default is 0 as the CPSS supports USX ports */
GT_STATUS phoenix_only_ports_of_100G_mac_set(IN GT_U32   use)
{
    phoenix_only_ports_of_100G_mac = use;
    return GT_OK;
}

GT_U32   phoenix_only_ports_of_25G_mac = 1; /* the default is 1 as the CPSS not supports ALL USX ports , but 'first port' of each 8 USX ports */
GT_STATUS phoenix_only_ports_of_25G_mac_set(IN GT_U32   use)
{
    phoenix_only_ports_of_25G_mac = use;
    return GT_OK;
}

/* allow to modify the flag from the terminal */
GT_STATUS falcon_useSinglePipeSingleDp_Set(IN GT_BOOL useSingleDp)
{
    useSinglePipeSingleDp = useSingleDp;

    return GT_OK;
}

GT_STATUS
cpssEnablerIsAppPlat
(
)
{
#ifdef CPSS_APP_PLATFORM_REFERENCE
    return GT_OK;
#else
    return GT_FAIL;
#endif
}

GT_STATUS
cpssEnablerIsMixedModePlat
(
)
{
#ifdef MIXED_MODE
    return GT_OK;
#else
    return GT_FAIL;
#endif
}

/* Notification callback for catching events */
EVENT_NOTIFY_FUNC *notifyEventArrivedFunc = NULL;

#ifdef CHX_FAMILY
GT_U32   trainingTrace=0;/* option to disable the print in runtime*/
/**
* @internal waTriggerInterruptsMaskNoLinkStatusChangeInterrupt function
* @endinternal
*
* @brief   Mask/unmask specific interrupts on port that can trigger WA's restart
*         or other not needed for now actions.
*         This function is almost the same as waTriggerInterruptsMask but uses
*         When it is no signal to not unmask CPSS_PP_PORT_LINK_STATUS_CHANGED_E interrupt
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] ifMode                   - current interface on port
* @param[in] operation                - mask/unmask
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*
* @note Affected interrupts: CPSS_PP_PORT_LANE_SIGNAL_DETECT_CHANGED_E,
*       CPSS_PP_PORT_MMPCS_SIGNAL_DETECT_CHANGE_E.
*
*/

GT_STATUS waTriggerInterruptsMaskNoLinkStatusChangeInterrupt
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_EVENT_MASK_SET_ENT     operation
)
{
    GT_STATUS                       rc;
    GT_U32                          laneNum,
                                    lanesCount;
    CPSS_PORT_SPEED_ENT             speed;

    if((CPSS_PP_FAMILY_DXCH_LION2_E == PRV_CPSS_PP_MAC(devNum)->devFamily) || ((CPSS_PORT_INTERFACE_MODE_RXAUI_E != ifMode) &&
        (CPSS_PORT_INTERFACE_MODE_XGMII_E != ifMode)))
    {
        if((ifMode != CPSS_PORT_INTERFACE_MODE_XGMII_E)
           || (operation != CPSS_EVENT_UNMASK_E))
        {
            rc = cpssDxChPortSpeedGet(devNum, portNum, &speed);
            if (rc != GT_OK)
            {
                cpssOsPrintSync("cpssDxChPortSpeedGet(portNum=%d):rc=%d\n",
                                    portNum, rc);
                return rc;
            }

            if((CPSS_PORT_INTERFACE_MODE_RXAUI_E == ifMode)
               || (CPSS_PORT_INTERFACE_MODE_XGMII_E == ifMode))
            {
                GT_BOOL supported;

                rc = cpssDxChPortInterfaceSpeedSupportGet(devNum, portNum, ifMode,
                                                          CPSS_PORT_SPEED_10000_E,
                                                          &supported);
                if(rc != GT_OK)
                {
                    cpssOsPrintSync("cpssDxChPortInterfaceSpeedSupportGet(portNum=%d,ifMode=%d):rc=%d\n",
                                    portNum, ifMode, rc);
                    return rc;
                }

                if(supported)
                {
                    lanesCount = (CPSS_PORT_INTERFACE_MODE_RXAUI_E == ifMode) ? 2 : 6;
                    for(laneNum = 0; laneNum < lanesCount; laneNum++)
                    {
                        if(trainingTrace)
                        {
                            cpssOsPrintf("cpssEventDeviceMaskWithEvExtDataSet(LANE_SIGNAL_DETECT_CHANGED,((portNum(%d)<<8)|laneNum(%d))=0x%x,op=%d\n",
                                portNum, laneNum, ((portNum<<8)|laneNum), operation);
                        }
                        rc = cpssEventDeviceMaskWithEvExtDataSet(devNum,
                                                                 CPSS_PP_PORT_LANE_SYNC_STATUS_CHANGED_E,
                                                                 ((portNum<<8)|laneNum),
                                                                 operation);
                        if(rc != GT_OK)
                        {
                            cpssOsPrintSync("2.cpssEventDeviceMaskWithEvExtDataSet(CPSS_PP_PORT_LANE_SIGNAL_DETECT_CHANGED_E,extData=0x%x):rc=%d\n",
                                            ((portNum<<8)|laneNum), rc);
                            return rc;
                        }
                    }

                    rc = cpssEventDeviceMaskWithEvExtDataSet(devNum,
                                                             CPSS_PP_PORT_MMPCS_SIGNAL_DETECT_CHANGE_E,
                                                             portNum,
                                                             CPSS_EVENT_MASK_E);
                    if(rc != GT_OK)
                    {
                        cpssOsPrintSync("cpssEventDeviceMaskWithEvExtDataSet(CPSS_PP_PORT_MMPCS_SIGNAL_DETECT_CHANGE_E,extData=%d):rc=%d\n",
                                        portNum, rc);
                        return rc;
                    }
                }
            }
            else if(   (CPSS_PORT_INTERFACE_MODE_KR_E == ifMode && CPSS_PORT_SPEED_5000_E != speed)
                    || (CPSS_PORT_INTERFACE_MODE_KR2_E == ifMode)
                    || (CPSS_PORT_INTERFACE_MODE_KR4_E == ifMode)
                    || (CPSS_PORT_INTERFACE_MODE_CR_E == ifMode)
                    || (CPSS_PORT_INTERFACE_MODE_CR2_E == ifMode)
                    || (CPSS_PORT_INTERFACE_MODE_CR4_E == ifMode)
                    || (CPSS_PORT_INTERFACE_MODE_SR_LR_E == ifMode)
                    || (CPSS_PORT_INTERFACE_MODE_XHGS_E == ifMode)
                    || (CPSS_PORT_INTERFACE_MODE_XHGS_SR_E == ifMode))

            {
                rc = cpssEventDeviceMaskWithEvExtDataSet(devNum,
                                                         CPSS_PP_PORT_MMPCS_SIGNAL_DETECT_CHANGE_E,
                                                         portNum,
                                                         operation);
                if(rc != GT_OK)
                {
                    cpssOsPrintSync("cpssEventDeviceMaskWithEvExtDataSet(CPSS_PP_PORT_MMPCS_SIGNAL_DETECT_CHANGE_E,extData=%d):rc=%d\n",
                                    portNum, rc);
                    return rc;
                }
            }
        }
    }

    return GT_OK;
}

/**
* @internal waTriggerInterruptsMask function
* @endinternal
*
* @brief   Mask/unmask specific interrupts on port that can trigger WA's restart
*         or other not needed for now actions.
* @param[in] devNum                   - device number
* @param[in] portNum                  - physical port number
* @param[in] ifMode                   - current interface on port
* @param[in] operation                - mask/unmask
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*
* @note Affected interrupts: CPSS_PP_PORT_LANE_SIGNAL_DETECT_CHANGED_E,
*       CPSS_PP_PORT_LINK_STATUS_CHANGED_E,
*       CPSS_PP_PORT_MMPCS_SIGNAL_DETECT_CHANGE_E.
*
*/
GT_STATUS waTriggerInterruptsMask
(
    IN  GT_U8                       devNum,
    IN  GT_PHYSICAL_PORT_NUM        portNum,
    CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
    IN  CPSS_EVENT_MASK_SET_ENT     operation
)
{
    GT_STATUS   rc;

    rc = waTriggerInterruptsMaskNoLinkStatusChangeInterrupt(devNum,  portNum, ifMode, operation);
    if(rc != GT_OK)
    {
        return rc;
    }

    rc = cpssEventDeviceMaskWithEvExtDataSet(devNum,
                                             CPSS_PP_PORT_LINK_STATUS_CHANGED_E,
                                             portNum,
                                             operation);
    if(rc != GT_OK)
    {
        cpssOsPrintSync("cpssEventDeviceMaskWithEvExtDataSet(CPSS_PP_PORT_LINK_STATUS_CHANGED_E,extData=%d):rc=%d\n",
                        portNum, rc);
        return rc;
    }

    return GT_OK;
}

GT_STATUS appDemoFdbManagerControlSet
(
    IN APP_DEMO_AU_MESSAGE_HANDLER_METHORD_ENT  auMsgHandler,
    IN GT_BOOL                                  autoAgingEn,
    IN GT_U32                                   autoAgingInterval
)
{
    return prvWrAppDemoFdbManagerControlSet(auMsgHandler, autoAgingEn, autoAgingInterval);
}

GT_STATUS appDemoFdbManagerGlobalIDSet
(
    IN GT_U32 fdbManagerId
)
{
    return prvWrAppDemoFdbManagerGlobalIDSet(fdbManagerId);
}

#endif /*CHX_FAMILY*/

GT_BOOL cpssAppPlatformInitialized = GT_FALSE;

/**
* @internal cpssDevSupportSystemResetSet function
* @endinternal
*
* @brief   API controls if SYSTEM reset is supported, it is used to disable the
*         support on devices that do support. vice versa is not possible
*
* @retval GT_OK                    - on success else if failed
*/

GT_STATUS cpssDevSupportSystemResetSet(
    IN  GT_U8       devNum,
    IN  GT_BOOL     enable
)
{
#ifdef CPSS_APP_PLATFORM_REFERENCE
    if (cpssAppPlatformInitialized == GT_TRUE)
    {
        return cpssAppPlatformDevSupportSystemResetSet(devNum, enable);
    }
#endif /*CPSS_APP_PLATFORM_REFERENCE*/

#if !defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)
    if (cpssAppPlatformInitialized == GT_FALSE)
    {
        return cpssAppDemoDevSupportSystemResetSet(devNum, enable);
    }
#endif /*!defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)*/

    return GT_OK;
}

/**
* @internal cpssResetSystem function
* @endinternal
*
* @brief   API performs CPSS reset that includes device remove, mask interrupts,
*         queues and tasks destroy, all static variables/arrays clean.
* @param[in] doHwReset                - indication to do HW reset
*
* @retval GT_OK                    - on success else if failed
*
* For HA to simulate Crash on WM use sequence
*    appDemoHaEmulateSwCrash()
*    cpssResetSystem(GT_FALSE)
*
*/

GT_STATUS cpssResetSystem(
    IN  GT_BOOL     doHwReset
)
{
#ifdef CPSS_APP_PLATFORM_REFERENCE
    if (cpssAppPlatformInitialized == GT_TRUE)
    {
        return cpssAppPlatformResetSystem(doHwReset);
    }
#endif /*CPSS_APP_PLATFORM_REFERENCE*/

#if !defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)
    if (cpssAppPlatformInitialized == GT_FALSE)
    {
        return cpssAppDemoResetSystem(doHwReset);
    }
#endif /*!defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)*/

    return GT_OK;
}

/*support 'makefile' ability to define the size of the allocation */
#ifndef APP_DEMO_OSMEM_DEFAULT_MEM_INIT_SIZE
    /* Default memory size */
    #define APP_DEMO_OSMEM_DEFAULT_MEM_INIT_SIZE (2048*1024)
#endif /* ! APP_DEMO_OSMEM_DEFAULT_MEM_INIT_SIZE */

#if !defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)
/* Cpss initialization */
GT_STATUS appDemoCpssInit
(
    GT_VOID
);

#endif /*!defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)*/

#ifdef CPSS_APP_PLATFORM_REFERENCE
extern GT_STATUS cpssAppRefEventFuncInit
(
    GT_U8  devNum
);

#ifdef IMPL_GALTIS
extern GT_U32 *enhUtUseCaptureToCpuPtr;
extern GT_U32 *tgfCmdCpssNetEnhUtUseCaptureToCpuPtr;
#endif /* IMPL_GALTIS */

static GT_STATUS cpssAppRefCommonFuncBind
(
    GT_VOID
)
{

    cpssAppUtilsEventHandlerPreInitCb = cpssAppUtilsEventHandlerPreInit;
    cpssAppUtilsUniEventsFreeCb = cpssAppUtilsUniEventsFree;
    cpssAppUtilsGenEventCounterIncrementCb = cpssAppUtilsGenEventCounterIncrement;
    cpssAppUtilsGenEventFuncInitCb = cpssAppRefEventFuncInit;
    cpssAppUtilsDbEntryAddCb = appDemoDbEntryAdd;
    cpssAppUtilsDbEntryGetCb = appDemoDbEntryGet;
#ifdef IMPL_GALTIS
    cpssAppUtilsEnhUtUseCaptureToCpuPtrPtr = & enhUtUseCaptureToCpuPtr;
    cpssAppUtilsTgfCmdCpssNetEnhUtUseCaptureToCpuPtrPtr = & tgfCmdCpssNetEnhUtUseCaptureToCpuPtr;
#endif /* IMPL_GALTIS */

    return GT_OK;;
} /* cpssAppRefEventFuncBind */

#endif /* CPSS_APP_PLATFORM_REFERENCE */


/**
* @internal userAppInitialize function
* @endinternal
*
* @brief   This routine is the starting point of the Driver.
*         Called from userAppInit() or from win32:main()
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS userAppInitialize(
    IN GT_U32 devNum
)
{
    GT_STATUS rc;

#ifdef CPSS_APP_PLATFORM_REFERENCE
    extern GT_STATUS cpssPpInit(void);
    extern GT_STATUS cpssAppRefOsFuncBind(GT_VOID);
#endif

    /* must be called before any Os function */
    rc = osWrapperOpen(NULL);
    if(rc != GT_OK)
    {
        osPrintf("osWrapper initialization failure!\n");
        return rc;
    }

    /* Initialize memory pool. It must be done before any memory allocations */
    /* must be before osWrapperOpen(...) that calls osStaticMalloc(...) */
    rc = osMemInit(APP_DEMO_OSMEM_DEFAULT_MEM_INIT_SIZE, GT_TRUE);
    if (rc != GT_OK)
    {
        osPrintf("osMemInit() failed, rc=%d\n", rc);
        return rc;
    }

#if defined(CPSS_APP_PLATFORM_REFERENCE) && !defined(MIXED_MODE)
    rc = cpssAppRefOsFuncBind();
#else
    /* run appDemoCpssInit() directly, not from console => show board list => show sw version
     * this allow application to work without console task */
    rc = appDemoCpssInit();
#endif /*defined(CPSS_APP_PLATFORM_REFERENCE) && !defined(MIXED_MODE)*/
    if (rc != GT_OK)
    {
        osPrintf("appDemoCpssInit() failed, rc=%d\n", rc);
    }

#ifdef CPSS_APP_PLATFORM_REFERENCE
    rc = cpssAppRefCommonFuncBind();
    if (rc != GT_OK)
    {
        osPrintf("cpssAppRefEventFuncBind() failed, rc=%d\n", rc);
        return rc;
    }

#endif /* CPSS_APP_PLATFORM_REFERENCE */

#ifdef CPSS_APP_PLATFORM_REFERENCE
    rc = cpssPpInit();
#endif
    /* Set gtInitSystem to be the init function */
    /*cpssInitSystemFuncPtr = (GT_INTFUNCPTR)cpssInitSystem;*/

#ifdef INIT_BEFORE_SHELL
    osPrintf("cpssInitSystemCallTask will be called\n");
    rc = cpssInitSystemCallTask();
    osPrintf("cpssInitSystemCallTask was called, rc=%d\n", rc);
#endif /*INIT_BEFORE_SHELL*/
    /* Start the command shell */



    return cmdInit(devNum);
} /* userAppInitialize */

/**
* @internal userAppInit function
* @endinternal
*
* @brief   This routine is the starting point of the Driver.
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - otherwise.
*/

GT_STATUS userAppInit(GT_VOID)
{
    return userAppInitialize(0);
} /* userAppInit */

/* macro to force casting between 2 functions prototypes */
#define FORCE_FUNC_CAST     (void *)

#if defined(CPSS_APP_PLATFORM_REFERENCE) && !defined(MIXED_MODE)
GT_VOID cpssAppOsLog
(
    IN    CPSS_LOG_LIB_ENT      lib,
    IN    CPSS_LOG_TYPE_ENT     type,
    IN    const char*           format,
    ...
);
GT_STATUS cpssAppPlatRefWelcome(GT_VOID);
#else
GT_VOID appDemoOsLog
(
    IN    CPSS_LOG_LIB_ENT      lib,
    IN    CPSS_LOG_TYPE_ENT     type,
    IN    const char*           format,
    ...
);

/* if not used shared memory or it's unix-like simulation do compile this */
#if !defined(SHARED_MEMORY)
GT_STATUS appDemoGetDefaultExtDrvFuncs(
    OUT CPSS_EXT_DRV_FUNC_BIND_STC  *extDrvFuncBindInfoPtr
);

#endif /* !defined(SHARED_MEMORY) */

#if (!defined(SHARED_MEMORY) || (defined(LINUX) && defined(ASIC_SIMULATION)))
GT_STATUS appDemoGetDefaultTraceFuncs(
    OUT CPSS_TRACE_FUNC_BIND_STC  *traceFuncBindInfoPtr
);
#endif

GT_BOOL appDemoCpuEtherPortUsed(void);

GT_STATUS appDemoEventBind
(
    IN  CPSS_UNI_EV_CAUSE_ENT uniEventArr[],
    IN  GT_U32               arrLength,
    OUT GT_UINTPTR           *hndlPtr
);

GT_STATUS appDemoEventSelect
(
    IN  GT_UINTPTR           hndl,
    IN  GT_U32              *timeoutPtr,
    OUT GT_U32              evBitmapArr[],
    IN  GT_U32              evBitmapArrLength
);
GT_STATUS appDemoEventRecv
(
    IN  GT_UINTPTR            hndl,
    IN  CPSS_UNI_EV_CAUSE_ENT evCause,
    OUT GT_U32              *evExtDataPtr,
    OUT GT_U8               *evDevPtr
);
GT_STATUS appDemoEventDeviceMaskSet
(
    IN GT_U8                    devNum,
    IN CPSS_UNI_EV_CAUSE_ENT    uniEvent,
    IN CPSS_EVENT_MASK_SET_ENT  operation
);

GT_STATUS appDemoPpConfigGet
(
    IN GT_U8    devIndex,
    OUT CMD_APP_PP_CONFIG* ppConfigPtr
);
GT_BOOL appDemoIsSystemInitialized
(
    void
);
GT_STATUS appDemoPpConfigPrint
(
    IN  GT_U8  devNum
);
GT_STATUS prvAppDemoTraceHwAccessEnable
(
    IN GT_U8                devNum,
    IN GT_U32               accessType,
    IN GT_BOOL              enable
);
GT_STATUS prvAppDemoTraceHwAccessOutputModeSet
(
    IN GT_U32   mode
);
GT_STATUS prvAppDemoEventFatalErrorEnable
(
    IN GT_32 fatalErrorType
);
GT_STATUS util_appDemoDxChPortFWSRatesTableGet
(
        IN  GT_U8 devNum,
        IN  GT_U32 arrSize,
        OUT CMD_PORT_FWS_RATE_STC *rateDbPtr
);

#if (defined CHX_FAMILY)
    GT_STATUS bc2AppDemoPortsConfig
    (
            IN  GT_U8                           devNumber,
            IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
            IN  CPSS_PORT_SPEED_ENT             speed,
            IN  GT_BOOL                         powerUp,
            IN  GT_U32                          numOfPorts,
            ...
    );
    GT_STATUS bobkAppDemoPortsConfig
    (
            IN  GT_U8                           devNumber,
            IN  CPSS_PORT_INTERFACE_MODE_ENT    ifMode,
            IN  CPSS_PORT_SPEED_ENT             speed,
            IN  GT_BOOL                         powerUp,
            IN  GT_U32                          squelch,
            IN  GT_U32                          numOfPorts,
            ...
    );
    GT_STATUS gtAppDemoXcat2StackPortsModeSpeedSet
    (
            IN  GT_U8     devNum,
            IN  GT_U8     portNum,
            IN  CPSS_PORT_INTERFACE_MODE_ENT   ifMode,
            IN  CPSS_PORT_SPEED_ENT  speed
    );
    GT_STATUS gtAppDemoXcat2SfpPortEnable
    (
            IN  GT_U8     devNum,
            IN  GT_U8     portNum,
            IN  GT_BOOL   enable
    );
    GT_STATUS util_appDemoDxChPortFWSRatesTableGet
    (
            IN  GT_U8 devNum,
            IN  GT_U32 arrSize,
            OUT CMD_PORT_FWS_RATE_STC *rateDbPtr
    );

#endif

#if !defined(SHARED_MEMORY)
    GT_STATUS appDemoGetDefaultOsBindFuncs(
        OUT CPSS_OS_FUNC_BIND_STC *osFuncBindPtr
    );
#endif /* !defined(SHARED_MEMORY) */
#endif /*defined(CPSS_APP_PLATFORM_REFERENCE) && !defined(MIXED_MODE)*/


#define BIND_APP_LEVEL_FUNC(infoPtr,level,funcName)     \
        infoPtr->level.funcName = funcName

#define BIND_APP2_LEVEL_FUNC(infoPtr,level,funcInLevel,funcName)     \
        infoPtr->level.funcInLevel = funcName

#if (defined PX_FAMILY)
    extern GT_STATUS util_appDemoPxPortFWSRatesTableGet
    (
            IN  GT_U8 devNum,
            IN  GT_U32 arrSize,
            OUT CMD_PORT_FWS_RATE_STC *rateDbPtr
    );
#endif /*CHX_FAMILY*/

/**
* @internal applicationExtServicesBind function
* @endinternal
*
* @brief   the mainCmd calls this function , so the application (that implement
*         this function) will bind the mainCmd (and GaltisWrapper) with OS ,
*         external driver functions and other application functions
*
* @param[out] extDrvFuncBindInfoPtr    - (pointer to) set of external driver call back functions
* @param[out] osFuncBindPtr            - (pointer to) set of OS call back functions
* @param[out] osExtraFuncBindPtr       - (pointer to) set of extra OS call back functions (that CPSS not use)
* @param[out] extraFuncBindPtr         - (pointer to) set of extra call back functions (that CPSS not use) (non OS functions)
* @param[out] traceFuncBindPtr         - (pointer to) set of Trace call back functions
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
* @note This function must be implemented by the Application !!!
*
*/
GT_STATUS   applicationExtServicesBind(
    OUT CPSS_EXT_DRV_FUNC_BIND_STC   *extDrvFuncBindInfoPtr,
    OUT CPSS_OS_FUNC_BIND_STC        *osFuncBindPtr,
    OUT CMD_OS_FUNC_BIND_EXTRA_STC   *osExtraFuncBindPtr,
    OUT CMD_FUNC_BIND_EXTRA_STC      *extraFuncBindPtr,
    OUT CPSS_TRACE_FUNC_BIND_STC     *traceFuncBindPtr
)
{
    GT_STATUS   rc;


    if(extDrvFuncBindInfoPtr == NULL ||
       osFuncBindPtr == NULL ||
       osExtraFuncBindPtr == NULL ||
       extraFuncBindPtr == NULL ||
       traceFuncBindPtr == NULL)
    {
        return GT_BAD_PTR;
    }

#if defined(SHARED_MEMORY)
    /* Shared memory approach on ARM should use dlsym-based workaround for pointers to functions */
#if !defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)
    rc = shrMemGetDefaultExtDrvFuncs(extDrvFuncBindInfoPtr);
    if(rc != GT_OK)
    {
        return rc;
    }

    rc = shrMemGetDefaultOsBindFuncsThruDynamicLoader(osFuncBindPtr);
    if(rc != GT_OK)
    {
        return rc;
    }

    /* Shared memory for Trace should be implemented later */
    CPSS_TBD_BOOKMARK
#else
    rc = cpssAppPlatformOsCbGet(CPSS_APP_PLATFORM_OS_CALL_TYPE_SHARED_E, osFuncBindPtr);
    if(rc != GT_OK)
    {
        return rc;
    }
#endif /*!defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)*/
#else
#if !defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)
    rc = appDemoGetDefaultExtDrvFuncs(extDrvFuncBindInfoPtr);
    if(rc != GT_OK)
    {
        return rc;
    }

    rc = appDemoGetDefaultOsBindFuncs(osFuncBindPtr);
    if(rc != GT_OK)
    {
        return rc;
    }

    rc = appDemoGetDefaultTraceFuncs(traceFuncBindPtr);
    if(rc != GT_OK)
    {
        return rc;
    }
#else
    rc = cpssAppPlatformOsCbGet(CPSS_APP_PLATFORM_OS_CALL_TYPE_STATIC_E, osFuncBindPtr);
    if(rc != GT_OK)
    {
        return rc;
    }
#endif /*!defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE) */
#endif

#if defined(CPSS_APP_PLATFORM_REFERENCE) && !defined(MIXED_MODE)
    osMemSet(extDrvFuncBindInfoPtr,0,sizeof(*extDrvFuncBindInfoPtr));
    osMemSet(traceFuncBindPtr,0,sizeof(*traceFuncBindPtr));
#endif

#if ! defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)
    osFuncBindPtr->osTaskBindInfo.osTaskCreateFunc = appDemoWrap_osTaskCreate;
    osFuncBindPtr->osTaskBindInfo.osTaskDeleteFunc = appDemoWrap_osTaskDelete;
#endif /*! defined(CPSS_APP_PLATFORM_REFERENCE) && defined(MIXED_MODE)*/

/*    osFuncBindPtr->osTaskBindInfo.osTaskGetSelfFunc = osTaskGetSelf; */

    BIND_APP_LEVEL_FUNC(osExtraFuncBindPtr,osSocketsBindInfo,osSocketTcpCreate                       );
    BIND_APP_LEVEL_FUNC(osExtraFuncBindPtr,osSocketsBindInfo,osSocketTcpCreate                       );
#ifndef _WIN32
    BIND_APP_LEVEL_FUNC(osExtraFuncBindPtr,osSocketsBindInfo,osSocketUdsCreate                       );
#endif
    BIND_APP_LEVEL_FUNC(osExtraFuncBindPtr,osSocketsBindInfo,osSocketUdpCreate                       );
    BIND_APP_LEVEL_FUNC(osExtraFuncBindPtr,osSocketsBindInfo,osSocketTcpDestroy                      );
    BIND_APP_LEVEL_FUNC(osExtraFuncBindPtr,osSocketsBindInfo,osSocketUdpDestroy                      );
    BIND_APP_LEVEL_FUNC(osExtraFuncBindPtr,osSocketsBindInfo,osSocketCreateAddr                      );
    BIND_APP_LEVEL_FUNC(osExtraFuncBindPtr,osSocketsBindInfo,osSocketDestroyAddr                     );
    BIND_APP_LEVEL_FUNC(osExtraFuncBindPtr,osSocketsBindInfo,osSocketBind                            );
    BIND_APP_LEVEL_FUNC(osExtraFuncBindPtr,osSocketsBindInfo,osSocketListen                          );
    BIND_APP_LEVEL_FUNC(osExtraFuncBindPtr,osSocketsBindInfo,osSocketAccept                          );
    BIND_APP_LEVEL_FUNC(osExtraFuncBindPtr,osSocketsBindInfo,osSocketConnect                         );
    BIND_APP_LEVEL_FUNC(osExtraFuncBindPtr,osSocketsBindInfo,osSocketSetNonBlock                     );
    BIND_APP_LEVEL_FUNC(osExtraFuncBindPtr,osSocketsBindInfo,osSocketSetBlock                        );
    BIND_APP_LEVEL_FUNC(osExtraFuncBindPtr,osSocketsBindInfo,osSocketSend                            );
    BIND_APP_LEVEL_FUNC(osExtraFuncBindPtr,osSocketsBindInfo,osSocketSendTo                          );
    BIND_APP_LEVEL_FUNC(osExtraFuncBindPtr,osSocketsBindInfo,osSocketRecv                            );
    BIND_APP_LEVEL_FUNC(osExtraFuncBindPtr,osSocketsBindInfo,osSocketRecvFrom                        );
    BIND_APP_LEVEL_FUNC(osExtraFuncBindPtr,osSocketsBindInfo,osSocketSetSocketNoLinger               );
    BIND_APP_LEVEL_FUNC(osExtraFuncBindPtr,osSocketsBindInfo,osSocketExtractIpAddrFromSocketAddr     );
    BIND_APP_LEVEL_FUNC(osExtraFuncBindPtr,osSocketsBindInfo,osSocketGetSocketAddrSize               );
    osExtraFuncBindPtr->osSocketsBindInfo.osSocketShutDown = (CPSS_SOCKET_SHUTDOWN_FUNC)osSocketShutDown;
    BIND_APP_LEVEL_FUNC(osExtraFuncBindPtr,osSocketsBindInfo,osSelectCreateSet                       );
    BIND_APP_LEVEL_FUNC(osExtraFuncBindPtr,osSocketsBindInfo,osSelectEraseSet                        );
    BIND_APP_LEVEL_FUNC(osExtraFuncBindPtr,osSocketsBindInfo,osSelectZeroSet                         );
    BIND_APP_LEVEL_FUNC(osExtraFuncBindPtr,osSocketsBindInfo,osSelectAddFdToSet                      );
    BIND_APP_LEVEL_FUNC(osExtraFuncBindPtr,osSocketsBindInfo,osSelectClearFdFromSet                  );
    BIND_APP_LEVEL_FUNC(osExtraFuncBindPtr,osSocketsBindInfo,osSelectIsFdSet                         );
    BIND_APP_LEVEL_FUNC(osExtraFuncBindPtr,osSocketsBindInfo,osSelectCopySet                         );
    BIND_APP_LEVEL_FUNC(osExtraFuncBindPtr,osSocketsBindInfo,osSelect                                );
    BIND_APP_LEVEL_FUNC(osExtraFuncBindPtr,osSocketsBindInfo,osSocketGetSocketFdSetSize              );

    BIND_APP_LEVEL_FUNC(osExtraFuncBindPtr,osTasksBindInfo,osTaskGetSelf                            );
    BIND_APP_LEVEL_FUNC(osExtraFuncBindPtr,osTasksBindInfo,osSetTaskPrior                           );

#if ! defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)
    BIND_APP_LEVEL_FUNC(osExtraFuncBindPtr,osPoolsBindInfo,gtPoolCreatePool        );
    BIND_APP_LEVEL_FUNC(osExtraFuncBindPtr,osPoolsBindInfo,gtPoolCreateDmaPool     );
    BIND_APP_LEVEL_FUNC(osExtraFuncBindPtr,osPoolsBindInfo,gtPoolReCreatePool      );
    BIND_APP_LEVEL_FUNC(osExtraFuncBindPtr,osPoolsBindInfo,gtPoolDeletePool        );
    BIND_APP_LEVEL_FUNC(osExtraFuncBindPtr,osPoolsBindInfo,gtPoolGetBuf            );
    BIND_APP_LEVEL_FUNC(osExtraFuncBindPtr,osPoolsBindInfo,gtPoolFreeBuf           );
    BIND_APP_LEVEL_FUNC(osExtraFuncBindPtr,osPoolsBindInfo,gtPoolGetBufSize        );
    BIND_APP_LEVEL_FUNC(osExtraFuncBindPtr,osPoolsBindInfo,gtPoolExpandPool        );
    BIND_APP_LEVEL_FUNC(osExtraFuncBindPtr,osPoolsBindInfo,gtPoolGetBufFreeCnt     );
    BIND_APP_LEVEL_FUNC(osExtraFuncBindPtr,osPoolsBindInfo,gtPoolPrintStats        );

    BIND_APP2_LEVEL_FUNC(extraFuncBindPtr,cpuEthernetPortBindInfo,cmdIsCpuEtherPortUsed        ,appDemoCpuEtherPortUsed             );
    BIND_APP2_LEVEL_FUNC(extraFuncBindPtr,eventsBindInfo,cmdCpssEventBind         ,appDemoEventBind             );
    BIND_APP2_LEVEL_FUNC(extraFuncBindPtr,eventsBindInfo,cmdCpssEventSelect       ,appDemoEventSelect           );
    BIND_APP2_LEVEL_FUNC(extraFuncBindPtr,eventsBindInfo,cmdCpssEventRecv         ,appDemoEventRecv             );
    BIND_APP2_LEVEL_FUNC(extraFuncBindPtr,eventsBindInfo,cmdCpssEventDeviceMaskSet,appDemoEventDeviceMaskSet    );

    BIND_APP2_LEVEL_FUNC(extraFuncBindPtr,appDbBindInfo,cmdAppDbEntryAdd ,appDemoDbEntryAdd );
    BIND_APP2_LEVEL_FUNC(extraFuncBindPtr,appDbBindInfo,cmdAppDbEntryGet ,appDemoDbEntryGet );
    BIND_APP2_LEVEL_FUNC(extraFuncBindPtr,appDbBindInfo,cmdAppDbDump     ,appDemoDbDump     );
    BIND_APP2_LEVEL_FUNC(extraFuncBindPtr,appDbBindInfo,cmdAppPpConfigGet,appDemoPpConfigGet);
    BIND_APP2_LEVEL_FUNC(extraFuncBindPtr,appDbBindInfo,cmdInitSystem    ,cpssInitSystem    );
    BIND_APP2_LEVEL_FUNC(extraFuncBindPtr,appDbBindInfo,cmdInitSystemGet ,cpssInitSystemGet );
    BIND_APP2_LEVEL_FUNC(extraFuncBindPtr,appDbBindInfo,cmdResetSystem   ,cpssResetSystem   );
    BIND_APP2_LEVEL_FUNC(extraFuncBindPtr,appDbBindInfo,cmdInitConfi     ,confi2InitSystem    );
    BIND_APP2_LEVEL_FUNC(extraFuncBindPtr,appDbBindInfo,cmdAppShowBoardsList         ,appDemoShowBoardsList    );
    BIND_APP2_LEVEL_FUNC(extraFuncBindPtr,appDbBindInfo,cmdAppIsSystemInitialized    ,appDemoIsSystemInitialized);
    BIND_APP2_LEVEL_FUNC(extraFuncBindPtr,appDbBindInfo,cmdAppPpConfigPrint,appDemoPpConfigPrint);
    BIND_APP2_LEVEL_FUNC(extraFuncBindPtr,appDbBindInfo,cmdAppAllowProcessingOfAuqMessages ,
                         appDemoAllowProcessingOfAuqMessages);
    BIND_APP2_LEVEL_FUNC(extraFuncBindPtr,appDbBindInfo,cmdAppTraceHwAccessEnable ,
                         prvAppDemoTraceHwAccessEnable);
    BIND_APP2_LEVEL_FUNC(extraFuncBindPtr,appDbBindInfo,cmdAppTraceHwAccessOutputModeSet ,
                         prvAppDemoTraceHwAccessOutputModeSet);
    BIND_APP2_LEVEL_FUNC(extraFuncBindPtr,appDbBindInfo,cmdAppStartHeapAllocCounter  ,osMemStartHeapAllocationCounter );
    BIND_APP2_LEVEL_FUNC(extraFuncBindPtr,appDbBindInfo,cmdAppGetHeapAllocCounter    ,osMemGetHeapAllocationCounter   );

    BIND_APP2_LEVEL_FUNC(extraFuncBindPtr,appDbBindInfo,cmdAppEventFatalErrorEnable ,
                         prvAppDemoEventFatalErrorEnable);
    BIND_APP2_LEVEL_FUNC(extraFuncBindPtr,appDbBindInfo,cmdAppDemoEventsDataBaseGet ,
                         appDemoEventsDataBaseGet);

#if (defined CHX_FAMILY)
    BIND_APP2_LEVEL_FUNC(extraFuncBindPtr,appDbBindInfo,cmdPortRateTableGet, util_appDemoDxChPortFWSRatesTableGet);
    BIND_APP2_LEVEL_FUNC(extraFuncBindPtr,appDbBindInfo,cmdAppBc2PortsConfig,bc2AppDemoPortsConfig);
    BIND_APP2_LEVEL_FUNC(extraFuncBindPtr,appDbBindInfo,cmdAppBobkPortsConfig,bobkAppDemoPortsConfig);
    BIND_APP2_LEVEL_FUNC(extraFuncBindPtr,appDbBindInfo,cmdAppXcat2StackPortsModeSpeedSet,gtAppDemoXcat2StackPortsModeSpeedSet);
    BIND_APP2_LEVEL_FUNC(extraFuncBindPtr,appDbBindInfo,cmdAppXcat2SfpPortEnable,gtAppDemoXcat2SfpPortEnable);
    BIND_APP2_LEVEL_FUNC(extraFuncBindPtr,appDbBindInfo,cmdAppDxChHsuOldImagePreUpdatePreparation       ,appDemoDxChHsuOldImagePreUpdatePreparation       );
    BIND_APP2_LEVEL_FUNC(extraFuncBindPtr,appDbBindInfo,cmdAppDxChHsuNewImageUpdateAndSynch             ,appDemoDxChHsuNewImageUpdateAndSynch             );

#endif

#if (defined PX_FAMILY)
    BIND_APP2_LEVEL_FUNC(extraFuncBindPtr,appDbBindInfo,cmdPortRateTableGet, util_appDemoPxPortFWSRatesTableGet);
#endif
#endif /*! defined(CPSS_APP_PLATFORM_REFERENCE) && defined(MIXED_MODE)*/

#if defined(CPSS_APP_PLATFORM_REFERENCE) && !defined(MIXED_MODE)
    extraFuncBindPtr->appDbBindInfo.cmdAppShowBoardsList = cpssAppPlatRefWelcome;
#endif /*defined(CPSS_APP_PLATFORM_REFERENCE) && !defined(MIXED_MODE)*/

    return GT_OK;
}

#ifdef ASIC_SIMULATION

#define BIND_FUNC(funcName)        \
            simOsBindInfo.funcName = funcName
#define BIND_LEVEL_FUNC(level,funcName) \
            simOsBindInfo.level.funcName = funcName
SIM_OS_FUNC_BIND_STC simOsBindInfo;

void simulationBindOsFuncs(void)
{
        /* reset all fields of simOsBindInfo */
        osMemSet(&simOsBindInfo,0,sizeof(simOsBindInfo));

        /* let the simulation OS bind it's own functions to the simulation */
        simOsFuncBindOwnSimOs();

        BIND_FUNC(osWrapperOpen);

        BIND_LEVEL_FUNC(sockets,osSocketTcpCreate);
        BIND_LEVEL_FUNC(sockets,osSocketUdpCreate);
        BIND_LEVEL_FUNC(sockets,osSocketTcpDestroy);
        BIND_LEVEL_FUNC(sockets,osSocketUdpDestroy);
        BIND_LEVEL_FUNC(sockets,osSocketCreateAddr);
        BIND_LEVEL_FUNC(sockets,osSocketDestroyAddr);
        BIND_LEVEL_FUNC(sockets,osSocketBind);
        BIND_LEVEL_FUNC(sockets,osSocketListen);
        BIND_LEVEL_FUNC(sockets,osSocketAccept);
        BIND_LEVEL_FUNC(sockets,osSocketConnect);
        BIND_LEVEL_FUNC(sockets,osSocketSetNonBlock);
        BIND_LEVEL_FUNC(sockets,osSocketSetBlock);
        BIND_LEVEL_FUNC(sockets,osSocketSend);
        BIND_LEVEL_FUNC(sockets,osSocketSendTo);
        BIND_LEVEL_FUNC(sockets,osSocketRecv);
        BIND_LEVEL_FUNC(sockets,osSocketRecvFrom);
        BIND_LEVEL_FUNC(sockets,osSocketSetSocketNoLinger);
        BIND_LEVEL_FUNC(sockets,osSocketExtractIpAddrFromSocketAddr);
        BIND_LEVEL_FUNC(sockets,osSocketExtractPortFromSocketAddr);
        BIND_LEVEL_FUNC(sockets,osSocketGetSocketAddrSize);
        BIND_LEVEL_FUNC(sockets,osSocketShutDown);

        BIND_LEVEL_FUNC(sockets,osSelectCreateSet);
        BIND_LEVEL_FUNC(sockets,osSelectEraseSet);
        BIND_LEVEL_FUNC(sockets,osSelectZeroSet);
        BIND_LEVEL_FUNC(sockets,osSelectAddFdToSet);
        BIND_LEVEL_FUNC(sockets,osSelectClearFdFromSet);
        BIND_LEVEL_FUNC(sockets,osSelectIsFdSet);
        BIND_LEVEL_FUNC(sockets,osSelectCopySet);
        BIND_LEVEL_FUNC(sockets,osSelect);
        BIND_LEVEL_FUNC(sockets,osSocketGetSocketFdSetSize);

        /* this needed for binding the OS of simulation with our OS functions */
        simOsFuncBind(&simOsBindInfo);

        return;
}


#ifdef LINUX
/* IntLockmutex should belong to simulation (simulation can be used without Enabler)! */
extern GT_MUTEX IntLockmutex;
extern void simulationLibInit(void);

void simulationStart (void)
{
    extern GT_STATUS userAppInit(void);

    char   semName[50];

    /* Must be called before use any of OS functions. */
    osWrapperOpen(NULL);
    /* create semaphores */

    osSprintf(semName, "IntLockmutex");

    /* create 'reentrant' mutex ! */
    if (osMutexCreate(semName, &IntLockmutex) != 0)
        exit (0);

    simulationBindOsFuncs();

#ifndef RTOS_ON_SIM
    /* call simulation init */
    simulationLibInit();
#endif
    /* init CMD */
    userAppInit();
}
#endif /*LINUX*/

#endif /*ASIC_SIMULATION*/

/**
* @internal cpssReInitSystem function
* @endinternal
*
* @brief   Run cpssInitSystem according to previously initialized parameters
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS cpssReInitSystem
(
    GT_VOID
)
{
#ifdef CPSS_APP_PLATFORM_REFERENCE
    if (cpssAppPlatformInitialized == GT_TRUE)
    {
        return cpssAppPlatformReInitSystem();
    }
#endif /*CPSS_APP_PLATFORM_REFERENCE*/
#if !defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)
    if (cpssAppPlatformInitialized == GT_FALSE)
    {
        return cpssAppDemoReInitSystem();
    }
#endif /*!defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)*/

    return GT_OK;
}

/**
* @internal appDemoEventRequestDrvnModeInit function
* @endinternal
*
* @brief   This routine spawns the App Demo event handlers.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS appDemoEventRequestDrvnModeInit
(
    GT_VOID
)
{
#ifdef CPSS_APP_PLATFORM_REFERENCE
    if (cpssAppPlatformInitialized == GT_TRUE)
    {
       return appPlatformEventRequestDrvnModeInit();
    }
#endif /*CPSS_APP_PLATFORM_REFERENCE*/

#if !defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)
    if (cpssAppPlatformInitialized == GT_FALSE)
    {
       return prvAppDemoEventRequestDrvnModeInit();
    }
#endif /*!defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)*/

    return GT_OK;
}


/* linux/LinuxSim MAIN */

#ifdef CPSS_APP_PLATFORM_REFERENCE
GT_32 isWarmboot = 0;
#endif /* CPSS_APP_PLATFORM_REFERENCE*/

#if !defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)
/* flag to indicate wthether this is normal or hir app */
GT_BOOL isHirApp;
#endif /*!defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)*/


#ifdef LINUX

#ifdef CMD_LUA_CLI
static GT_BOOL cmdOsWaitForGdbConnection;
static int wgdbDummy;

static void main_gdb(void)
{
    printf("Wait-loop for GDB connection... Set breakpoint on sleep() in the main_gdb\n");
    while (cmdOsWaitForGdbConnection == GT_TRUE) /* -wgdb */
    {
        sleep(1);
    }
    wgdbDummy++; /* This dummy just to set-istruct-pointer here to exit the loop */
}
#endif /* CMD_LUA_CLI GDB */

#ifndef ASIC_SIMULATION
/*Linux*/

void preUserAppInit(void)
{
    /* here we will map the pss memory */

    userAppInit();
}

static void Usage(const char *argv0)
{
    printf("Usage: %s [options]\n", argv0);
    printf("Options:\n");
    printf("    -help          - This help\n");
    printf("    -tty           - Use current tty for console\n");
    printf("                     Enabled by default\n");
#ifdef CMD_LUA_CLI
    printf("    -cmdshell      - Force cmdshell (Galtis shell) at startup\n");
    printf("    -luacli        - Force luaCLI at startup\n");
    printf("    -config _file_ - Execute luaCLI script\n");
    printf("    -noconfig      - Ignore luaCLI startup script\n");
#if !defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)
    printf("    -hir           - Run in Hot Insert Removal Mode\n");
#endif /*!defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)*/
    printf("    -wgdb          - Wait-loop for GDB connection\n");
#endif

#if !defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)
    printf("    -dev_num       - Device number to manage (all by default)\n");
    printf("    -port          - Alternative telnet port (default is 12345)\n");
#endif /*!defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)*/
    printf("    -daemon        - daemonize, redirect stdout\n");
    printf("                     to /tmp/appDemo_stdout\n");
    printf("    -redir_stdout _filename_\n");
    printf("                   - Redirect stdout to file\n");
    printf("                     To be used with -daemon\n");
    exit(0);
}

static void appDemoPrintVersion()
{
    printf("CPSS Version Stream: %s\n",CPSS_STREAM_NAME_CNS);
#ifdef CHX_FAMILY
    printf("CPSS DXCH Version: %s\n",CPSS_DXCH_VERSION_CNS);
#endif
#ifdef PX_FAMILY
    printf("CPSS PX Version: %s\n",CPSS_PX_VERSION_CNS);
#endif
#ifndef PTP_PLUGIN_SUPPORT
    printf("Build date: %s\n",VERSION_DATE);
    printf("Compilation flags: %s\n",VERSION_FLAGS);
#endif
    exit(0);
}

extern GT_BOOL cmdStreamGrabSystemOutput;
#ifdef CMD_LUA_CLI
static struct termios tc_saved;
static void restore_tty(void)
{
    tcsetattr(0, TCSAFLUSH, &tc_saved);
    printf("tty settings restored\n");
}
#endif

#ifdef PTP_PLUGIN_SUPPORT
int main_cpss(int argc, const char * argv[])
#else
int main(int argc, const char * argv[])
#endif
{
    int i, fd;
#ifdef CMD_LUA_CLI
    GT_BOOL cmdOsConfigFileNameAlloc = GT_FALSE;
#endif
    const char *redir_stdout = NULL;
#if !defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)
    /*hir feature is disabled by default */
    isHirApp=GT_FALSE;
    unsigned int devNum;
#endif /*!defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)*/

    for (i=1; i<argc; i++)
    {

        if ((strcmp(argv[i], "--version") == 0 ))
        {
           appDemoPrintVersion();
        }

        if (   (strcmp(argv[i], "-help") == 0)
            || (strcmp(argv[i], "--help") == 0)
            || (strcmp(argv[i], "-h") == 0)
            || (strcmp(argv[i], "-?") == 0)
            || (strcmp(argv[i], "?") == 0)
            || (strcmp(argv[i], "/h") == 0)
            || (strcmp(argv[i], "/?") == 0))
        {
            Usage(argv[0]);
        }
        if (strcmp(argv[i], "-daemon") == 0)
        {
            if (daemon(1, 1) < 0)
            {
                perror("Failed to daemonize");
             }
#ifdef SHARED_MEMORY
            else /*Add new PID to db,
            old one will be erased automatically*/
            {
                (GT_VOID)osGlobalSharedDbAddProcess();
            }
#endif

            setenv("PSEUDO", "notty", 1);
            redir_stdout = "/tmp/appDemo_stdout";
        }
        if ((i+1 < argc) && (strcmp(argv[i],"-redir_stdout") == 0))
        {
            redir_stdout = argv[++i];
        }

#if !defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)
        /* Check and validate dev_num command line option */
        if ((strcmp("-dev_num", argv[i]) == 0) && (i + 1 < argc))
        {
            i++;
            if (sscanf(argv[i], "%u", &devNum) != 1) {
                errno = EINVAL;
                perror("-dev_num expects U8");
            }
            appDemoSysOneDeviceSet((GT_U8)devNum);
        }

        /* Telnet port number to bind to */
        if ((strcmp("-port", argv[i]) == 0) && (i + 1 < argc))
        {
            i++;
            cmdTelnetPortSet(atol(argv[i]));
        }
#endif /*!defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)*/
    }

    if (redir_stdout != NULL) /* Only when daemon or forced */
    {
        cmdStreamGrabSystemOutput = GT_FALSE;
        fd = open(redir_stdout, O_WRONLY|O_CREAT|O_APPEND, 0644);
        if (fd < 0)
        {
            perror("open(redir_stdout_file) failed");
        }
        else
        {
            if (fd != 1)
            {
                close(1);
                dup2(fd, 1);
            }
            if (fd != 2)
            {
                close(2);
                dup2(fd, 2);
            }
            close(fd);
        }
    }
/*
#ifdef SHARED_MEMORY
    if(shrMemInitSharedLibrary_FirstClient() != GT_OK)
        return -1;
#endif
*/

#ifdef CMD_LUA_CLI
    cmdDefaultShellType = CMD_STARTUP_SHELL_LUACLI_E;
#endif

    for (i=1; i < argc; i++)
    {
#ifdef CMD_LUA_CLI
        if (strcmp("-cmdshell", argv[i]) == 0)
        {
            cmdDefaultShellType = CMD_STARTUP_SHELL_CMDSHELL_E;
        }
        if (strcmp("-luacli", argv[i]) == 0)
        {
            cmdDefaultShellType = CMD_STARTUP_SHELL_LUACLI_E;
        }
#if !defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)
        if (strcmp("-hir", argv[i]) == 0)
        {
            isHirApp=GT_TRUE;
        }
#endif /*!defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)*/

        /* support "-config" key to enter the name of the configuration file */
        if ((strcmp("-config", argv[i]) == 0) && (i + 1 < argc))
        {
            i++;
            if( GT_TRUE == cmdOsConfigFileNameAlloc )
            {
                free(cmdOsConfigFileName);
            }
            cmdOsConfigFileName = malloc(strlen(argv[i])+1);
            if( NULL == cmdOsConfigFileName )
            {
                return -1;
            }
            cmdOsConfigFileNameAlloc = GT_TRUE;
            strcpy(cmdOsConfigFileName, argv[i]);
            continue;
        }
        if (strcmp("-noconfig", argv[i]) == 0)
        {
            if( GT_TRUE == cmdOsConfigFileNameAlloc )
            {
                free(cmdOsConfigFileName);
                cmdOsConfigFileNameAlloc = GT_FALSE;
            }
            cmdOsConfigFileName = NULL;
        }
        if (strcmp("-wgdb", argv[i]) == 0)
        {
            cmdOsWaitForGdbConnection = GT_TRUE;
        }
#endif
        /* no need to define environment variable PSEUDO to cause simulation
             * print output on local console
            */
         /* new key "-tty" causes all input/output be on local console */
        if (!strcmp(argv[i], "-tty"))
        {
            setenv("PSEUDO", "/dev/tty", 1);
        }

#ifdef CPSS_APP_PLATFORM_REFERENCE
        if (strcmp("-ha", argv[i]) == 0) {
           isWarmboot = GT_TRUE;
        }
#endif /*CPSS_APP_PLATFORM_REFERENCE*/
    }

    /* all input/output be on local console if PSEUDO is not set yet */
    if (getenv("PSEUDO") == NULL)
        setenv("PSEUDO", "/dev/tty", 1);


#ifdef CMD_LUA_CLI
    if (cmdOsWaitForGdbConnection == GT_TRUE)
    {
        main_gdb();
    }
    tcgetattr(0, &tc_saved);
    atexit(restore_tty);
#endif

   if (osStartEngine(argc, argv, "appDemo", preUserAppInit) != GT_OK)
        return 1;

    return 0;
}

#else /*ASIC_SIMULATION*/

/*LinuxSim*/

char **cmdArgv;
char **cmdEnv;

GT_STATUS  simulationDistributedClientSet
(
    GT_U32 index,
    GT_U32 tcpPortSync,
    GT_U32 tcpPortAsync
);

/* IntLockmutex should belong to simulation (simulation can be used without Enabler)! */
extern GT_MUTEX IntLockmutex;

extern int osSprintf(char * buffer, const char* format, ...);

static void Usage(const char *argv0)
{
    printf("Usage: %s [options]\n", argv0);
    printf("Options:\n");
    printf("    -help          - This help\n");
    printf("    -i _inifile_   - Use this .ini file for simulation\n");
    printf("    -e _inifile_   - Use built-in .ini file for simulation\n");
    printf("    -tty           - Use current tty for console\n");
#if !defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)
    printf("    -hir           - Run in Hot Insert Removal Mode\n");
#endif /*!defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)*/
    printf("                     Enabled by default\n");
#ifdef CMD_LUA_CLI
    printf("    -cmdshell      - Force cmdshell (Galtis shell) at startup\n");
    printf("    -luacli        - Force luaCLI at startup\n");
    printf("    -config _file_ - Execute luaCLI script\n");
    printf("    -noconfig      - Ignore luaCLI startup script\n");
    printf("    -wgdb          - Wait-loop for GDB connection\n");
#endif
    printf("    -daemon        - daemonize, redirect stdout\n");
    printf("                     to /tmp/appDemo_stdout\n");
    printf("    -redir_stdout _filename_\n");
    printf("                   - Redirect stdout to file\n");
    printf("                     To be used with -daemon\n");
    exit(0);
}

static void appDemoPrintVersion()
{
    printf("CPSS Version Stream: %s\n",CPSS_STREAM_NAME_CNS);
#ifdef CHX_FAMILY
    printf("CPSS DXCH Version: %s\n",CPSS_DXCH_VERSION_CNS);
#endif
#ifdef PX_FAMILY
    printf("CPSS PX Version: %s\n",CPSS_PX_VERSION_CNS);
#endif
    printf("Build date: %s\n",VERSION_DATE);
    printf("Compilation flags: %s\n",VERSION_FLAGS);
    exit(0);
}

#ifdef CMD_LUA_CLI
static struct termios tc_saved;
static void restore_tty(void)
{
    tcsetattr(0, TCSAFLUSH, &tc_saved);
    printf("tty settings restored\n");
}
#endif
extern GT_BOOL cmdStreamGrabSystemOutput;
#ifdef PTP_PLUGIN_SUPPORT
int main_cpss(int argc, char * argv[], char *envp[])
#else
int main(int argc, char * argv[], char *envp[])
#endif
{
    int i, len=0;
    const char *redir_stdout = NULL;
#ifdef SHARED_MEMORY
    GT_BOOL aslrSupport= GT_FALSE;
    aslrSupport = OS_ASLR_SUPPORT_DEFAULT;
    GT_U32 tcpPortSync,tcpPortAsync,clientIndex =0xFFFF,result;
    GT_STATUS rc;
#endif

#if !defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)
    isHirApp=GT_FALSE;
#endif /*!defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)*/
    for (i=1; i<argc; i++)
    {
        if ((strcmp(argv[i], "--version") == 0 ))
        {
           appDemoPrintVersion();
        }
        if (   (strcmp(argv[i], "-help") == 0)
            || (strcmp(argv[i], "--help") == 0)
            || (strcmp(argv[i], "-h") == 0)
            || (strcmp(argv[i], "-?") == 0)
            || (strcmp(argv[i], "?") == 0)
            || (strcmp(argv[i], "/h") == 0)
            || (strcmp(argv[i], "/?") == 0))
        {
            Usage(argv[0]);
        }
        if (strcmp(argv[i], "-daemon") == 0)
        {
            if (daemon(1, 1) < 0)
                perror("Failed to daemonize");
            setenv("PSEUDO", "notty", 1);
            redir_stdout = "/tmp/appDemo_stdout";
        }
        if ((i+1 < argc) && (strcmp(argv[i],"-redir_stdout") == 0))
        {
            redir_stdout = argv[++i];
        }

#ifdef SHARED_MEMORY
        if (strcmp("-noaslr", argv[i]) == 0)
        {
            aslrSupport = GT_FALSE;
        }


        if(0==strcmp(argv[i],CLIENT_TOKEN0))
        {
            if(i+1<argc)
            {
                result = sscanf(argv[i+1],"%d",&clientIndex);
                if(result==0)
                {
                    clientIndex=0;
                }
             }
        }
        else if(0==strcmp(argv[i],CLIENT_TOKEN1))
        {
            if(i+1<argc)
            {
                result = sscanf(argv[i+1],"%d",&tcpPortSync);
                if(result==0)
                {
                    tcpPortSync=0;
                }
             }
        }
        else if(0==strcmp(argv[i],CLIENT_TOKEN2))
        {
            if(i+1<argc)
            {
                result = sscanf(argv[i+1],"%d",&tcpPortAsync);
                if(result==0)
                {
                    tcpPortAsync=0;
                }
             }
        }
#endif
    }

    if (redir_stdout != NULL) /* Only when daemon or forced */
    {
        i = open(redir_stdout, O_WRONLY|O_CREAT|O_APPEND, 0644);
        if (i < 0)
        {
            perror("open(redir_stdout_file) failed");
        }
        else
        {
            if (i != 1)
            {
                close(1);
                dup2(i, 1);
            }
            if (i != 2)
            {
                close(2);
                dup2(i, 2);
            }
        }
    }

#ifdef SHARED_MEMORY
    if(GT_TRUE== aslrSupport)
    {
       rc = simulationDistributedClientSet(clientIndex,tcpPortSync,tcpPortAsync);
       if(rc!=GT_OK)
       {
          printf ("simulationDistributedClientSet  failed with code %d\n",rc);
          return 1;
       }
    }
#endif

    cmdStreamGrabSystemOutput = GT_FALSE;
#ifdef CMD_LUA_CLI
    cmdDefaultShellType = CMD_STARTUP_SHELL_LUACLI_E;
    /* set cmdOsConfigFilePath to current directory */
    {
        char tmppath[4096];
        if (getcwd(tmppath, sizeof(tmppath)) == 0)
        {
            fprintf(stderr, "getcwd() : %s\n", strerror(errno));
        }
        strcat(tmppath, "/");
        cmdOsConfigFilePath = malloc(strlen(tmppath)+1);
        strcpy(cmdOsConfigFilePath, tmppath);
    }
#endif /* CMD_LUA_CLI */

    cmdArgv = calloc(argc+1, sizeof(char*));
    if (cmdArgv == NULL)
    {
        fprintf(stderr, "calloc(%d, %d) failed: %s\n",
                argc+1, sizeof(char*), strerror(errno));
        return 1;
    }
    cmdEnv = envp;
    commandLine[0] = 0;
    for (i=0; i<argc; i++)
    {
        len += strlen(argv[i]);
        cmdArgv[i] = argv[i];

        if ((unsigned)len < sizeof(commandLine)-2) /* leave place for ' ' and '\0' */
        {
            strcat(commandLine,argv[i]);

            if (i<argc-1)
            {
                strcat(commandLine," ");
                len++;
            }
        }

        /* no need to define environment variable PSEUDO to cause simulation
         * print output on local console
         */
        /* new key "-tty" causes all input/output be on local console */
#if 0
        /* just ignore this option, always act as if it set */
        if (i > 0 && !strcmp(argv[i], "-tty"))
        {
        }
#endif

        /* new key equale to '-stdout comPort' */
        if (i > 0 && !strcmp(argv[i], "-grabstd"))
        {
            cmdStreamGrabSystemOutput = GT_TRUE;
        }

#if !defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)
        /* to enable hot insert remove feature  */
        if (i > 0 && !strcmp(argv[i], "-hir"))
        {
            isHirApp = GT_TRUE;
        }
#endif /*!defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)*/

        /* support "-stdout" key like in window's simulation */
        if (i > 0 && (i+1 < argc) && !strcmp(argv[i], "-stdout"))
        {
            if (strcmp(argv[i+1], "comPort") == 0 ||
                    strcmp(argv[i+1], "comport") == 0)
            {
                cmdStreamGrabSystemOutput = GT_TRUE;
            }
        }

#ifdef CPSS_APP_PLATFORM_REFERENCE
        if (strcmp("-ha", argv[i]) == 0) {
            isWarmboot = GT_TRUE;
            continue;
        }
#endif /*CPSS_APP_PLATFORM_REFERENCE*/

#ifdef CMD_LUA_CLI
        /* support "-cmdshell" key to set cmdShell starttup shell */
        if (strcmp("-cmdshell", argv[i]) == 0) {
            cmdDefaultShellType = CMD_STARTUP_SHELL_CMDSHELL_E;
            continue;
        }
        if (strcmp("-luacli", argv[i]) == 0) {
            cmdDefaultShellType = CMD_STARTUP_SHELL_LUACLI_E;
            continue;
        }
        /* support "-config" key to enter the name of the configuration file */
        if (strcmp("-config", argv[i]) == 0) {
            i++;
            cmdOsConfigFileName = malloc(strlen(argv[i])+1);
            strcpy(cmdOsConfigFileName, argv[i]);
            continue;
        }
        if (strcmp("-noconfig", argv[i]) == 0)
        {
            cmdOsConfigFileName = NULL;
        }
        if (strcmp("-wgdb", argv[i]) == 0)
        {
            cmdOsWaitForGdbConnection = GT_TRUE;
        }
#endif /* CMD_LUA_CLI */

    }
    /* all input/output be on local console if PSEUDO is not set yet */
    if (getenv("PSEUDO") == NULL)
        setenv("PSEUDO", "/dev/tty", 1);

    cmdArgv[argc] = 0;
    /* extend cmdArgv[0] to full path if it is relative */
    if (cmdArgv[0][0] != '/')
    {
        char argv0[4096];
        if (getcwd(argv0, sizeof(argv0)) == 0)
        {
            fprintf(stderr, "getcwd() : %s\n", strerror(errno));
        }
        strcat(argv0, "/");
        strcat(argv0, cmdArgv[0]);
        cmdArgv[0] = malloc(strlen(argv0)+1);
        strcpy(cmdArgv[0], argv0);
    }

#ifdef CMD_LUA_CLI
    if (cmdOsWaitForGdbConnection == GT_TRUE)
    {
        main_gdb();
    }
    tcgetattr(0, &tc_saved);
    atexit(restore_tty);
#endif
    if (osStartEngine(argc, (const char**)argv, "appDemoSim", simulationStart) != GT_OK)
        return 1;

    return 0;
}

#endif /*!ASIC_SIMULATION*/

#endif /*LINUX*/
