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
* @file cpssCommonUtils.c
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
#if defined(SHARED_MEMORY)
#include <gtOs/gtOsSharedFunctionPointers.h>
#endif /*defined(SHARED_MEMORY)*/

GT_CHAR * CPSS_SPEED_2_STR
(
    CPSS_PORT_SPEED_ENT speed
)
{

    typedef struct
    {
        CPSS_PORT_SPEED_ENT speedEnm;
        GT_CHAR            *speedStr;
    } APPDEMO_SPEED_2_STR_STC;

    APPDEMO_SPEED_2_STR_STC prv_speed2str[] =
    {
         {  CPSS_PORT_SPEED_10_E,     "10M  "}
        ,{  CPSS_PORT_SPEED_100_E,    "100M "}
        ,{  CPSS_PORT_SPEED_1000_E,   "1G   "}
        ,{  CPSS_PORT_SPEED_2500_E,   "2.5G "}
        ,{  CPSS_PORT_SPEED_5000_E,   "5G   "}
        ,{  CPSS_PORT_SPEED_10000_E,  "10G  "}
        ,{  CPSS_PORT_SPEED_11800_E,  "11.8G"}
        ,{  CPSS_PORT_SPEED_12000_E,  "12G  "}
        ,{  CPSS_PORT_SPEED_12500_E,  "12.5G"}
        ,{  CPSS_PORT_SPEED_13600_E,  "13.6G"}
        ,{  CPSS_PORT_SPEED_15000_E,  "15G  "}
        ,{  CPSS_PORT_SPEED_16000_E,  "16G  "}
        ,{  CPSS_PORT_SPEED_20000_E,  "20G  "}
        ,{  CPSS_PORT_SPEED_22000_E,  "22G  "}
        ,{  CPSS_PORT_SPEED_23600_E,  "23.6G"}
        ,{  CPSS_PORT_SPEED_25000_E,  "25G  "}
        ,{  CPSS_PORT_SPEED_29090_E,  "29.1G"}
        ,{  CPSS_PORT_SPEED_40000_E,  "40G  "}
        ,{  CPSS_PORT_SPEED_47200_E,  "47.2G"}
        ,{  CPSS_PORT_SPEED_50000_E,  "50G  "}
        ,{  CPSS_PORT_SPEED_200G_E,   "200G "}
        ,{  CPSS_PORT_SPEED_75000_E,  "75G  "}
        ,{  CPSS_PORT_SPEED_100G_E,   "100G "}
        ,{  CPSS_PORT_SPEED_107G_E,   "107G "}
        ,{  CPSS_PORT_SPEED_140G_E,   "140G "}
        ,{  CPSS_PORT_SPEED_102G_E,   "102G "}
        ,{  CPSS_PORT_SPEED_52500_E,  "52.5G"}
        ,{  CPSS_PORT_SPEED_26700_E,  "26.7G"}
        ,{  CPSS_PORT_SPEED_400G_E,   "400G "}
        ,{  CPSS_PORT_SPEED_800G_E,   "800G "}
        ,{  CPSS_PORT_SPEED_NA_E,     "NA   "}
    };

    GT_U32 i;
    for (i = 0 ; prv_speed2str[i].speedEnm != CPSS_PORT_SPEED_NA_E; i++)
    {
        if (prv_speed2str[i].speedEnm == speed)
        {
            return prv_speed2str[i].speedStr;
        }
    }
    return "-----";
}

GT_CHAR * CPSS_IF_2_STR
(
    CPSS_PORT_INTERFACE_MODE_ENT ifEnm
)
{


    typedef struct
    {
        CPSS_PORT_INTERFACE_MODE_ENT ifEnm;
        GT_CHAR                     *ifStr;
    } APPDEMO_IF_2_STR_STC;



    APPDEMO_IF_2_STR_STC prv_prvif2str[] =
    {
         {  CPSS_PORT_INTERFACE_MODE_REDUCED_10BIT_E, "REDUCED_10BIT"  }
        ,{  CPSS_PORT_INTERFACE_MODE_REDUCED_GMII_E,  "REDUCED_GMII"   }
        ,{  CPSS_PORT_INTERFACE_MODE_MII_PHY_E,       "MII_PHY     "   }
        ,{  CPSS_PORT_INTERFACE_MODE_MII_E,           "MII         "   }
        ,{  CPSS_PORT_INTERFACE_MODE_GMII_E,          "GMII        "   }
        ,{  CPSS_PORT_INTERFACE_MODE_SGMII_E,         "SGMII       "   }
        ,{  CPSS_PORT_INTERFACE_MODE_XGMII_E,         "XGMII       "   }
        ,{  CPSS_PORT_INTERFACE_MODE_LOCAL_XGMII_E,   "LOCAL_XGMII "   }
        ,{  CPSS_PORT_INTERFACE_MODE_MGMII_E,         "MGMII       "   }
        ,{  CPSS_PORT_INTERFACE_MODE_QSGMII_E,        "QSGMII      "   }
        ,{  CPSS_PORT_INTERFACE_MODE_QX_E,            "QX          "   }
        ,{  CPSS_PORT_INTERFACE_MODE_HX_E,            "HX          "   }
        ,{  CPSS_PORT_INTERFACE_MODE_RXAUI_E,         "RXAUI       "   }
        ,{  CPSS_PORT_INTERFACE_MODE_100BASE_FX_E,    "100BASE_FX  "   }
        ,{  CPSS_PORT_INTERFACE_MODE_1000BASE_X_E,    "1000BASE_X  "   }
        ,{  CPSS_PORT_INTERFACE_MODE_XLG_E,           "XLG         "   }
        ,{  CPSS_PORT_INTERFACE_MODE_HGL_E,           "HGL         "   }
        ,{  CPSS_PORT_INTERFACE_MODE_CHGL_12_E,       "CHGL_12     "   }
        ,{  CPSS_PORT_INTERFACE_MODE_KR_E,            "KR          "   }
        ,{  CPSS_PORT_INTERFACE_MODE_KR2_E,           "KR2         "   }
        ,{  CPSS_PORT_INTERFACE_MODE_KR4_E,           "KR4         "   }
        ,{  CPSS_PORT_INTERFACE_MODE_KR8_E,           "KR8         "   }
        ,{  CPSS_PORT_INTERFACE_MODE_SR_LR_E,         "SR_LR       "   }
        ,{  CPSS_PORT_INTERFACE_MODE_SR_LR2_E,        "SL_LR2      "   }
        ,{  CPSS_PORT_INTERFACE_MODE_SR_LR4_E,        "SL_LR4      "   }
        ,{  CPSS_PORT_INTERFACE_MODE_SR_LR8_E,        "SL_LR8      "   }
        ,{  CPSS_PORT_INTERFACE_MODE_ILKN4_E,         "ILKN4       "   }
        ,{  CPSS_PORT_INTERFACE_MODE_ILKN8_E,         "ILKN8       "   }
        ,{  CPSS_PORT_INTERFACE_MODE_ILKN12_E,        "ILKN12      "   }
        ,{  CPSS_PORT_INTERFACE_MODE_ILKN16_E,        "ILKN16      "   }
        ,{  CPSS_PORT_INTERFACE_MODE_ILKN24_E,        "ILKN24      "   }
        ,{  CPSS_PORT_INTERFACE_MODE_XHGS_E,          "XHGS        "   }
        ,{  CPSS_PORT_INTERFACE_MODE_XHGS_SR_E ,      "XHGS-SR     "   }
        ,{  CPSS_PORT_INTERFACE_MODE_MLG_40G_10G_40G_10G_E,     "MLG         "   }
        ,{  CPSS_PORT_INTERFACE_MODE_KR_C_E,          "KR_C        "   }
        ,{  CPSS_PORT_INTERFACE_MODE_CR_C_E,          "CR_C        "   }
        ,{  CPSS_PORT_INTERFACE_MODE_KR2_C_E,         "KR2_C       "   }
        ,{  CPSS_PORT_INTERFACE_MODE_CR2_C_E,         "CR2_C       "   }
        ,{  CPSS_PORT_INTERFACE_MODE_CR_E,            "CR          "   }
        ,{  CPSS_PORT_INTERFACE_MODE_CR_E,            "CR2         "   }
        ,{  CPSS_PORT_INTERFACE_MODE_CR4_E,           "CR4         "   }
        ,{  CPSS_PORT_INTERFACE_MODE_CR8_E,           "CR8         "   }
        ,{  CPSS_PORT_INTERFACE_MODE_KR_S_E,          "KR_S        "   }
        ,{  CPSS_PORT_INTERFACE_MODE_CR_S_E,          "CR_S        "   }
        ,{  CPSS_PORT_INTERFACE_MODE_USX_OUSGMII_E,   "USX_OUSGMII "   }
        ,{  CPSS_PORT_INTERFACE_MODE_USX_20G_OXGMII_E,"USX_20G_OXGMII" }
        ,{  CPSS_PORT_INTERFACE_MODE_KR8_E,           "KR8         "   }
        ,{  CPSS_PORT_INTERFACE_MODE_CR8_E,           "CR8         "   }
        ,{  CPSS_PORT_INTERFACE_MODE_SR_LR8_E,        "SR_LR8      "   }
        ,{  CPSS_PORT_INTERFACE_MODE_USX_2_5G_SXGMII_E, "USX_2_5G_SXGMII"}
        ,{  CPSS_PORT_INTERFACE_MODE_USX_5G_SXGMII_E,   "USX_5G_SXGMII  "}
        ,{  CPSS_PORT_INTERFACE_MODE_USX_10G_SXGMII_E,  "USX_10G_SXGMII "}
        ,{  CPSS_PORT_INTERFACE_MODE_USX_5G_DXGMII_E,   "USX_5G_DXGMII  "}
        ,{  CPSS_PORT_INTERFACE_MODE_USX_10G_DXGMII_E,  "USX_10G_DXGMII "}
        ,{  CPSS_PORT_INTERFACE_MODE_USX_20G_DXGMII_E,  "USX_20G_DXGMII "}
        ,{  CPSS_PORT_INTERFACE_MODE_USX_QUSGMII_E,     "USX_QUSGMII_   "}
        ,{  CPSS_PORT_INTERFACE_MODE_USX_10G_QXGMII_E,  "USX_10G_QXGMII "}
        ,{  CPSS_PORT_INTERFACE_MODE_USX_20G_QXGMII_E,  "USX_20G_QXGMII "}
        ,{  CPSS_PORT_INTERFACE_MODE_USX_OUSGMII_E,     "USX_OUSGMII    "}
        ,{  CPSS_PORT_INTERFACE_MODE_USX_20G_OXGMII_E,  "USX_20G_OXGMII "}
        ,{  CPSS_PORT_INTERFACE_MODE_2500BASE_X_E,      "2500BASE_X     "}
        ,{  CPSS_PORT_INTERFACE_MODE_1000BASE_SXLX_E,   "1000BASE_SX    "}
        ,{  CPSS_PORT_INTERFACE_MODE_REMOTE_E,          "REMOTE         "}
        ,{  CPSS_PORT_INTERFACE_MODE_NA_E,            "NA          "   }
    };

    GT_U32 i;
    for (i = 0 ; prv_prvif2str[i].ifEnm != CPSS_PORT_INTERFACE_MODE_NA_E; i++)
    {
        if (prv_prvif2str[i].ifEnm == ifEnm)
        {
            return prv_prvif2str[i].ifStr;
        }
    }
    return "------------";
}

#ifdef CHX_FAMILY
GT_CHAR * CPSS_MAPPING_2_STR
(
    CPSS_DXCH_PORT_MAPPING_TYPE_ENT mapEnm
)
{

    typedef struct
    {
        CPSS_DXCH_PORT_MAPPING_TYPE_ENT mapEnm;
        GT_CHAR                        *mapStr;
    } APPDEMO_MAPPING_2_STR_STC;


    APPDEMO_MAPPING_2_STR_STC prv_mappingTypeStr[] =
    {
             { CPSS_DXCH_PORT_MAPPING_TYPE_ETHERNET_MAC_E,           "ETHERNET" }
            ,{ CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E,               "CPU-SDMA" }
            ,{ CPSS_DXCH_PORT_MAPPING_TYPE_ILKN_CHANNEL_E,           "ILKN-CHL" }
            ,{ CPSS_DXCH_PORT_MAPPING_TYPE_REMOTE_PHYSICAL_PORT_E,   "REMOTE-P" }
            ,{ CPSS_DXCH_PORT_MAPPING_TYPE_INVALID_E,                "--------" }
    };
    GT_U32 i;
    for (i = 0 ; prv_mappingTypeStr[i].mapEnm != CPSS_DXCH_PORT_MAPPING_TYPE_INVALID_E; i++)
    {
        if (prv_mappingTypeStr[i].mapEnm == mapEnm)
        {
            return prv_mappingTypeStr[i].mapStr;
        }
    }
    return "--------";
}





#endif /*CHX_FAMILY*/
