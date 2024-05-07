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
* @file cpssAppPlatformReferenceMain.c
*
* @brief APIs/Entry point for Cpss application platform reference code
*
*
* @version   1
********************************************************************************
*/

#define CAP_LOG_MODULE  CPSS_APP_PLATFORM_LOG_MODULE_APPREF_E

#ifdef INCLUDE_MPD
#include <mpdPrefix.h>
#include <mpdPrv.h>
#endif

#include <profiles/cpssAppPlatformProfile.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cpssAppPlatformSysConfig.h>
#include <stdio.h>
#include <appReference/cpssAppRefUtils.h>
#include <cpssAppPlatformI2cUtils.h>

#include <cpss/dxCh/dxChxGen/port/cpssDxChPortCtrl.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortTx.h>
#include <cpss/dxCh/dxChxGen/port/PortMapping/prvCpssDxChPortMappingShadowDB.h>

#include <ezbringup/cpssAppPlatformEzBringupTools.h>
#include <cpssAppUtilsCommon.h>

#ifdef INCLUDE_MPD
#include <cpss/common/smi/cpssGenSmi.h>
#include <cpssAppPlatform/cpssAppPlatformPpConfig.h>
#include <cpssAppUtilsCommon.h>
#include <mpd.h>
#define __USE_POSIX199309
#include <sched.h>
#include <sys/types.h>
#include <time.h>

#if !defined MIXED_MODE
GT_U16 vendorId;
GT_U16 phyModelNum;
#endif
#endif

#if !defined MIXED_MODE
GT_BOOL disableAutoPortMgr;
#endif
#include <cpss/common/init/cpssInit.h>
#include <ezbringup/cpssAppPlatformMpdAndPp.h>



#ifndef ASIC_SIMULATION
extern GT_STATUS cpssAppRefSerdesTrainingTaskDelete
(
    GT_U8 devNum
);
#endif

GT_STATUS cpssAppPlatformShowProfile
(
   IN GT_CHAR *profileName
)
{
    GT_STATUS rc = GT_OK;

    CPSS_APP_PLATFORM_LOG_ENTRY_MAC();

    if (!profileName)
       return GT_BAD_PTR;

    rc = cpssAppPlatformPrintProfile(profileName);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssAppPlatformPrintProfile);

    return rc;
}

typedef GT_STATUS (*appDemoDbEntryGet_TYPE)
(
    IN  GT_CHAR *namePtr,
    OUT GT_U32  *valuePtr
);
extern appDemoDbEntryGet_TYPE appDemoDbEntryGet_func;

static GT_BOOL mainProfilesInitialized = GT_FALSE;

#ifdef INCLUDE_MPD
#define PHY_ID_REG_DEV_NUM_CNS                1
#define PHY_VENDOR_ID_REG_ADDR_CNS            2
#define PHY_ID_REG_ADDR_CNS                   3
#define ASK_XCAT5X_A0_BOARD_RD_CNS            0xB1
#define ASK_XCAT5P_A0_BOARD_RD_CNS            0xC1


/* Marvell PHYs unique vendor ID - register 2 */
#define MRVL_PHY_REV_A_UID_CNS 0x2B
#define MRVL_PHY_UID_CNS 0x141
#define MRVL_PHY_1780_MODEL_NUM_CNS 0x2D
#define MRVL_PHY_1781_MODEL_NUM_CNS (0x2D | (0x9 << 16))  /* the same model_num like 1780 + revision 0x9*/
#define MRVL_PHY_2540_MODEL_NUM_CNS 0x3C1 /* 4.2003=0xBC1, bit[9:4]=Model_Num */
#define MRVL_PHY_2580_MODEL_NUM_CNS 0x3C3
#define MRVL_PHY_3240_MODEL_NUM_CNS 0x18
#define MRVL_PHY_3340_MODEL_NUM_CNS 0x1A
#define MRVL_PHY_2180_MODEL_NUM_CNS 0x1B
#define MRVL_PHY_7120_MODEL_NUM_CNS 0x3B

#define ASK_IRONMAN_A0_DB_IM_L_PHY_ID 0xD1
#define ASK_XCAT5X_A0_BOARD_RD_CNS    0xB1
#define ASK_XCAT5P_A0_BOARD_RD_CNS    0xC1

#define MPD_SAMPLE_NUM_OF_DEBUG_FLAGS_CNS       (10)

extern GT_BOOL appDemoCheckEnable;

CPSS_APP_PLATFORM_XPHY_STATIC_INFO_STC portToPhyArrayAc5xRd[] = {
    {32,0x0 ,4, MPD_PHY_USX_TYPE_OXGMII, MPD_TRANSCEIVER_COPPER_E},
    {33,0x1 ,4, MPD_PHY_USX_TYPE_OXGMII, MPD_TRANSCEIVER_COPPER_E},
    {34,0x2 ,4, MPD_PHY_USX_TYPE_OXGMII, MPD_TRANSCEIVER_COPPER_E},
    {35,0x3 ,4, MPD_PHY_USX_TYPE_OXGMII, MPD_TRANSCEIVER_COPPER_E},
    {36,0x4 ,5, MPD_PHY_USX_TYPE_OXGMII, MPD_TRANSCEIVER_COPPER_E},
    {37,0x5 ,5, MPD_PHY_USX_TYPE_OXGMII, MPD_TRANSCEIVER_COPPER_E},
    {38,0x6 ,5, MPD_PHY_USX_TYPE_OXGMII, MPD_TRANSCEIVER_COPPER_E},
    {39,0x7 ,5, MPD_PHY_USX_TYPE_OXGMII, MPD_TRANSCEIVER_COPPER_E},
    {40,0x1c,6, MPD_PHY_USX_TYPE_OXGMII, MPD_TRANSCEIVER_COPPER_E},
    {41,0x1d,6, MPD_PHY_USX_TYPE_OXGMII, MPD_TRANSCEIVER_COPPER_E},
    {42,0x1e,6, MPD_PHY_USX_TYPE_OXGMII, MPD_TRANSCEIVER_COPPER_E},
    {43,0x1f,6, MPD_PHY_USX_TYPE_OXGMII, MPD_TRANSCEIVER_COPPER_E},
    {44,0xc ,7, MPD_PHY_USX_TYPE_OXGMII, MPD_TRANSCEIVER_COPPER_E},
    {45,0xd ,7, MPD_PHY_USX_TYPE_OXGMII, MPD_TRANSCEIVER_COPPER_E},
    {46,0xe ,7, MPD_PHY_USX_TYPE_OXGMII, MPD_TRANSCEIVER_COPPER_E},
    {47,0xf ,7, MPD_PHY_USX_TYPE_OXGMII, MPD_TRANSCEIVER_COPPER_E},
    {0xff, 0xff, 0xff, MPD_PHY_USX_TYPE_NONE, MPD_TRANSCEIVER_COPPER_E}
};


static struct{
    GT_U8       devNum;
    GT_U32      macNum;
}ifIndexInfoArr[MPD_MAX_PORT_NUMBER_CNS];
static GT_U32 nextFreeGlobalIfIndex = 1;
extern GT_U16 vendorId;
extern GT_U16 phyModelNum;
typedef struct {
        BOOLEAN   isFree;
        char    * flagName_PTR;
        char    * flagHelp_PTR;
        BOOLEAN   flagStatus; /* TRUE - On */
}MPD_SAMPLE_DEBUG_FLAG_INFO_STC;
static MPD_SAMPLE_DEBUG_FLAG_INFO_STC mpdSampleDebugFlags[MPD_SAMPLE_NUM_OF_DEBUG_FLAGS_CNS];
static UINT_32 MPD_SAMPLE_DEBUG_FREE_INDEX = 0;

/*********************************** Phy APIs ************************************/

BOOLEAN cpssAppPlatformPhyOsDelay
(
     IN UINT_32 delay
)
{
    struct timespec sleep, remain;
    if (1000000000 < delay) {
    /* sleep time to big */
        return 0;
    }

    sleep.tv_nsec = delay*1000000;
    sleep.tv_sec = 0;
    if (nanosleep(  &sleep, &remain)) {
        return TRUE;
    }

    return FALSE;
}

void * cpssAppPlatformPhyOsMalloc
(
     UINT_32  size
)
{
        return cpssOsMalloc(size);
}
void cpssAppPlatformPhyOsFree
(
     void* data
)
{
    return cpssOsFree(data);
}

BOOLEAN cpssAppPlatformPhyXsmiMdioWrite (
    IN UINT_32      rel_ifIndex,
    IN UINT_8       mdioAddress,
    IN UINT_16      deviceOrPage,   /* PRV_MPD_IGNORE_PAGE_CNS */
    IN UINT_16      address,
    IN UINT_16      value
)
{
    UINT_8          dev, port;
    UINT_8          xsmiInterface, smiInterface;
    UINT_32         portGroup, portGroupsBmp;
    PRV_MPD_PORT_HASH_ENTRY_STC * port_entry_PTR;
    GT_STATUS       status = GT_OK;


    port_entry_PTR =  prvMpdGetPortEntry(rel_ifIndex);
    if (port_entry_PTR == NULL) {
        return FALSE;
    }

    if ((UINT_32)port_entry_PTR->initData_PTR->phyType == MPD_TYPE_INVALID_E) {
        return FALSE;
    }

    port = port_entry_PTR->initData_PTR->port;
    dev = port_entry_PTR->initData_PTR->mdioInfo.mdioDev;
    xsmiInterface = port_entry_PTR->initData_PTR->mdioInfo.mdioBus;
    portGroup = MPD_PORT_NUM_TO_GROUP_MAC(port);
    portGroupsBmp = MPD_PORT_GROUP_TO_PORT_GROUP_BMP_MAC(portGroup);

    if (port_entry_PTR->initData_PTR->phyType != MPD_TYPE_88E1780_E)
    {
        xsmiInterface = port_entry_PTR->initData_PTR->mdioInfo.mdioBus;
        status = cpssXsmiPortGroupRegisterWrite( dev,
                                                 portGroupsBmp,
                                                 xsmiInterface,
                                                 mdioAddress,
                                                 address,
                                                 deviceOrPage,
                                                 value);
    }
    else
    {
        smiInterface = port_entry_PTR->initData_PTR->mdioInfo.mdioBus;
        status = cpssSmiRegisterWriteShort( dev,
                                        portGroupsBmp,
                                        smiInterface,
                                        mdioAddress,
                                        address,
                                        value);
    }

    if (status != GT_OK){
        return FALSE;
    }
    return TRUE;
}

BOOLEAN cpssAppPlatformPhyXsmiMdioRead (
        IN  UINT_32      rel_ifIndex,
        IN  UINT_8       mdioAddress,
        IN  UINT_16      deviceOrPage,
        IN  UINT_16      address,
        OUT UINT_16    * value_PTR
)
{
    UINT_8          dev, port;
    UINT_8          xsmiInterface, smiInterface;
    UINT_32         portGroup, portGroupsBmp;
    PRV_MPD_PORT_HASH_ENTRY_STC * port_entry_PTR;
    GT_STATUS       status = GT_OK;


    port_entry_PTR =  prvMpdGetPortEntry(rel_ifIndex);
    if (port_entry_PTR == NULL) {
        return FALSE;
    }

    if ((UINT_32)port_entry_PTR->initData_PTR->phyType == MPD_TYPE_INVALID_E) {
        return FALSE;
    }

    port = port_entry_PTR->initData_PTR->port;
    dev = port_entry_PTR->initData_PTR->mdioInfo.mdioDev;
    portGroup = MPD_PORT_NUM_TO_GROUP_MAC(port);
    portGroupsBmp = MPD_PORT_GROUP_TO_PORT_GROUP_BMP_MAC(portGroup);

    if (port_entry_PTR->initData_PTR->phyType != MPD_TYPE_88E1780_E)
    {
         xsmiInterface = port_entry_PTR->initData_PTR->mdioInfo.mdioBus;
         status = cpssXsmiPortGroupRegisterRead( dev,
                                            portGroupsBmp,
                                            xsmiInterface,
                                            mdioAddress,
                                            address,
                                            deviceOrPage,
                                            value_PTR);
    }
    else
    {
       smiInterface = port_entry_PTR->initData_PTR->mdioInfo.mdioBus;
       status = cpssSmiRegisterReadShort( dev,
                                            portGroupsBmp,
                                            smiInterface,
                                            mdioAddress,
                                            address,
                                            value_PTR);
    }

    if (status != GT_OK){
        return FALSE;
    }
    return TRUE;
}

/* mpdSampleGetFwFiles */
BOOLEAN cpssAppPlatformPhyGetFwFiles
(
    IN  MPD_TYPE_ENT          phyType,
    OUT MPD_FW_FILE_STC       * mainFile_PTR
)
{
    char * fwFileName_PTR = NULL;
    FILE * fp;
    int ret;
    UINT_32 fileSize;

    switch(phyType)
    {
        case MPD_TYPE_88E2540_E:
/*            slaveFileName_PTR = "x3240flashdlslave_0_6_6_0_10673.hdr"; */
            /*fwFileName_PTR = "v0A080000_11386_11374_e2540.hdr";*/
            fwFileName_PTR = "/usr/bin/samples/v0A0E0000_e2540.hdr";
            break;
        case MPD_TYPE_88E2580_E:
            /*fwFileName_PTR = "v0A0A0000_11508_11488_e2580.hdr";*/
            fwFileName_PTR = "/usr/bin/samples/v0A0E0000_e2580.hdr";
            break;
        case MPD_TYPE_88X3540_E:
            fwFileName_PTR = "/usr/bin/samples/v0A0E0000_e3540.hdr";
            break;
        case MPD_TYPE_88X32x0_E:
            fwFileName_PTR    = "/usr/bin/samples/88X32xx-FW.hdr";
            break;
        case MPD_TYPE_88X33x0_E:
             if(vendorId == MRVL_PHY_UID_CNS)
             {
                fwFileName_PTR    = "/usr/bin/samples/88X33xx-Z2-FW.hdr";
             }
             else if(vendorId == MRVL_PHY_REV_A_UID_CNS)
             {
                 fwFileName_PTR = "/usr/bin/samples/x3310fw_0_2_8_0_8850.hdr";
             }
             break;
        case MPD_TYPE_88X2180_E:
            if(vendorId == MRVL_PHY_REV_A_UID_CNS)
            {
                fwFileName_PTR = "/usr/bin/samples/e21x0fw_8_0_0_36_8923.hdr";
            }
            break;
        default:
            cpssOsPrintf("not supported PHY type\n");
            break;
    }
    if (fwFileName_PTR == NULL)
    {
            return FALSE;
    }

    fp = fopen(fwFileName_PTR,"rb");
    if (fp)
    {
        fseek(fp, 0 , SEEK_END);
        fileSize = ftell(fp);
        fseek(fp, 0 , SEEK_SET);
        mainFile_PTR->dataSize = fileSize;
        mainFile_PTR->data_PTR = cpssAppPlatformPhyOsMalloc(fileSize);

        if (mainFile_PTR->data_PTR == NULL)
        {
            fclose(fp);
            return FALSE;
        }
        ret = fread(mainFile_PTR->data_PTR, sizeof(char), fileSize, fp);
        fclose(fp);
        if (ret <= 0)
        {
            cpssOsPrintf("Read from file fail\n");
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_ERROR, LOG_ERROR_NO_MSG);
        }
    }
    return TRUE;
}

MPD_RESULT_ENT cpssAppPlatformPhyHandleFailure(
        IN UINT_32                              rel_ifIndex,
        IN MPD_ERROR_SEVERITY_ENT       severity,
        IN UINT_32                                      line,
        IN const char                           * calling_func_PTR,
        IN const char                           * error_text_PTR
)
{
     char * severityText[MPD_ERROR_SEVERITY_WARNING_E+1] = {"Fatal", "Error", "Minor", "Warning"};
     cpssOsPrintf("Failure Level [%s] on port [%d] line [%d] called by [%s].  %s\n",
                (severity<=MPD_ERROR_SEVERITY_WARNING_E)?severityText[severity]:"Unknown",
                 rel_ifIndex,
                 line,
                 calling_func_PTR,
                 error_text_PTR);

        if (severity == MPD_ERROR_SEVERITY_FATAL_E){
            cpssOsPrintf("Fatal\n");
        }

        return MPD_OK_E;
}

BOOLEAN cpssAppPlatformPhyDebugBind (
    IN const char   * comp_PTR,
        IN const char   * pkg_PTR,
        IN const char   * flag_PTR,
        IN const char   * help_PTR,
    OUT UINT_32         * flagId_PTR
)
{
    UINT_32 len = 0,index;

    if (MPD_SAMPLE_DEBUG_FREE_INDEX >= MPD_SAMPLE_NUM_OF_DEBUG_FLAGS_CNS) {
            /* no more debug flags */
            return FALSE;
    }
    index = MPD_SAMPLE_DEBUG_FREE_INDEX++;
    mpdSampleDebugFlags[index].isFree = FALSE;
    mpdSampleDebugFlags[index].flagStatus = FALSE;
#if DEBUG_MODE  /* Enable for logs */
    if(!strcmp(flag_PTR, "write") || !strcmp(flag_PTR, "read"))
    {
        mpdSampleDebugFlags[index].flagStatus = TRUE;
    }
#endif /* DEBUG_MODE */
    if(strcmp(flag_PTR,"fw") == 0)
    {
        /*cpssOsPrintf("Firmware logs enabled\n");*/
        mpdSampleDebugFlags[index].flagStatus = TRUE;
    }
    len += (comp_PTR != NULL)?strlen(comp_PTR):0;
    len += (pkg_PTR != NULL)?strlen(pkg_PTR):0;
    len += (flag_PTR != NULL)?strlen(flag_PTR):0;

    if (len)
    {
        len += 6;
        mpdSampleDebugFlags[index].flagName_PTR = cpssAppPlatformPhyOsMalloc(len+1/*room for '\0'*/);
        mpdSampleDebugFlags[index].flagName_PTR[0] = '\0';
        strcat(mpdSampleDebugFlags[index].flagName_PTR,comp_PTR);
        strcat(mpdSampleDebugFlags[index].flagName_PTR,"-->");
        strcat(mpdSampleDebugFlags[index].flagName_PTR,pkg_PTR);
        strcat(mpdSampleDebugFlags[index].flagName_PTR,"-->");
        strcat(mpdSampleDebugFlags[index].flagName_PTR,flag_PTR);

    }
    len = (help_PTR != NULL)?strlen(help_PTR):0;
    if (len)
    {
        mpdSampleDebugFlags[index].flagHelp_PTR = cpssAppPlatformPhyOsMalloc(len);
        mpdSampleDebugFlags[index].flagHelp_PTR[0] = '\0';
        strncpy(mpdSampleDebugFlags[index].flagHelp_PTR,help_PTR,len);
    }
    *flagId_PTR = index;
    return TRUE;
}

/**
 * @brief   example implementation of debug check is Flag active
 *
 * @return Active / InActive
 */
BOOLEAN cpssAppPlatformPhyDebugIsActive (
    IN UINT_32  flagId
)
{
    if (flagId >= MPD_SAMPLE_NUM_OF_DEBUG_FLAGS_CNS)
    {
        return FALSE;
    }
    if (mpdSampleDebugFlags[flagId].isFree == TRUE)
    {
        return FALSE;
    }
    return mpdSampleDebugFlags[flagId].flagStatus;
}

/**
 * @brief   print debug and fw loading info
 *
 */

MPD_RESULT_ENT cpssAppPlatformPhyLogging(
    const char              * log_text_PTR
)
{
    cpssOsPrintf(log_text_PTR);
    return MPD_OK_E;
}

/**
 * @brief   example implementation of debug log (print)
 *
 */

void cpssAppPlatformPhyDebugLog (
    IN const char    * funcName_PTR,
    IN const char    * format_PTR,
    IN ...
)
{

        va_list      argptr;

        printf("%s:",funcName_PTR);
        va_start(argptr, format_PTR);
        vprintf( format_PTR, argptr);
        va_end(argptr);
        printf("\n");
}


GT_U32 cpssAppPlatformPhyMpdIndexGet
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      portNum
)
{
   GT_U32 index;

   for(index = 1; index < nextFreeGlobalIfIndex; index++)
   {
       if(ifIndexInfoArr[index].devNum == devNum &&
          ifIndexInfoArr[index].macNum == portNum)
       {
           return index;
       }
   }
   ifIndexInfoArr[nextFreeGlobalIfIndex].devNum  = devNum;
   ifIndexInfoArr[nextFreeGlobalIfIndex].macNum = portNum;
   return nextFreeGlobalIfIndex++;
}

MPD_TYPE_ENT cpssAppPlatformMpdPhyTypeGet
(
    GT_U32 phyType
)
{
    switch(phyType)
    {
        case MRVL_PHY_2580_MODEL_NUM_CNS:
            return MPD_TYPE_88E2580_E;
        case MRVL_PHY_2540_MODEL_NUM_CNS:
            return MPD_TYPE_88E2540_E;
        case MRVL_PHY_3240_MODEL_NUM_CNS:
            return MPD_TYPE_88X32x0_E;
        case MRVL_PHY_1780_MODEL_NUM_CNS:
            return MPD_TYPE_88E1780_E;
        case MRVL_PHY_3340_MODEL_NUM_CNS:
            return MPD_TYPE_88X33x0_E;
        case MRVL_PHY_2180_MODEL_NUM_CNS:
            return MPD_TYPE_88X2180_E;
        case MRVL_PHY_7120_MODEL_NUM_CNS:
            return MPD_TYPE_88X7120_E;
        default:
            return MPD_TYPE_INVALID_E;
    }
}


static GT_STATUS cpssAppPlatformPhyPortNumGet
(
    GT_U32  currBoardType,
    IN  CPSS_APP_PLATFORM_XPHY_INFO_STC    phyInfo,
    OUT CPSS_APP_PLATFORM_XPHY_STATIC_INFO_STC *staticPhyInfo_PTR
)
{
    GT_U32  i;
    GT_U32  portStart = 0;
    GT_U32  portEnd = 0;
    GT_APPDEMO_XPHY_STATIC_INFO_STC  *staticPhyInfoArray = NULL;

    switch(currBoardType)
    {
       case ASK_XCAT5X_A0_BOARD_RD_CNS:
           if(phyInfo.phyType == MRVL_PHY_2540_MODEL_NUM_CNS)
             {
                 /*staticPhyInfoArray = portToPhyArrayAc5xRd;*/
                 staticPhyInfoArray = portToPhyArrayAc5xRd2540_SR2;
                 portStart = 40;
                 portEnd   = 47;
             }
             if(phyInfo.phyType == MRVL_PHY_2580_MODEL_NUM_CNS)
             {
                staticPhyInfoArray = portToPhyArrayAc5xRd2580_SR2;
                portStart = 32;
                portEnd   = 39;
             }
             if(phyInfo.phyType == MRVL_PHY_1780_MODEL_NUM_CNS)
             {
                 staticPhyInfoArray = portToPhyArrayAc5xRd1780;
                 portStart = 0;
                 portEnd   = 31;
             }
             break;
        case ASK_XCAT5P_A0_BOARD_RD_CNS:
            if(phyInfo.phyType == MRVL_PHY_2580_MODEL_NUM_CNS)
            {
                staticPhyInfoArray = portToPhyArrayAc5pRd2580_SR1;
                portStart = 2;
                portEnd   = 17;
            }
            if(phyInfo.phyType == MRVL_PHY_1780_MODEL_NUM_CNS)
            {
                staticPhyInfoArray = portToPhyArrayAc5pRd1780;
                portStart = 18;
                portEnd   = 51;
            }
            break;
        case ASK_IRONMAN_A0_DB_IM_L_PHY_ID:
            if(phyInfo.phyType == MRVL_PHY_1781_MODEL_NUM_CNS)
            {
                staticPhyInfoArray = portToPhyArrayIronmanL_Db1781;
                portStart = 0;
                portEnd   = 15;
            }
            break;
        default:
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }
    for(i = 0; staticPhyInfoArray[i].portNum != 0xff ; i++)
    {
        if(staticPhyInfoArray[i].portNum < portStart || staticPhyInfoArray[i].portNum  > portEnd)
        {
            continue;
        }
        if(staticPhyInfoArray[i].phyAddr == phyInfo.phyAddr)
        {
            osMemCpy(staticPhyInfo_PTR, &(staticPhyInfoArray[i]), sizeof(CPSS_APP_PLATFORM_XPHY_STATIC_INFO_STC));
        }
     }
    return GT_OK;
}

GT_STATUS cpssAppPlatformXPhyIdGet
(
    IN  GT_U8                       devNum,
    IN  GT_PORT_GROUPS_BMP          portGroupsBmp,
    IN  CPSS_PHY_XSMI_INTERFACE_ENT xsmiInterface,
    IN  GT_U32                      xsmiAddr,
    OUT GT_U16                     *phyIdPtr
)
{
    GT_STATUS rc;
    GT_U16    data;
    /*GT_U16    phyRev = 0;  future use for 2180 phy*/

    /* read PHY ID */
    rc = cpssXsmiPortGroupRegisterRead(devNum, portGroupsBmp, xsmiInterface,
                                        xsmiAddr, PHY_ID_REG_ADDR_CNS,
                                        PHY_ID_REG_DEV_NUM_CNS, &data);
    if (rc != GT_OK)
    {
        CPSS_APP_PLATFORM_LOG_DBG_MAC("cpssXsmiPortGroupRegisterRead %d", rc);
        return rc;
    }
    if (data != 0xFFFF)
    {
        switch(data){
            case 0xBC1: /* E2540, 0x3C1 */
            case 0xBC3: /* E2580, 0x3C3 */
                *phyIdPtr = (data & 0x3FF);
                phyModelNum = *phyIdPtr;
                break;
            default:
                *phyIdPtr = (data >> 4) & 0x3F;
                phyModelNum = *phyIdPtr;
                break;
        }
    }
    else
        *phyIdPtr = 0;

    return GT_OK;
}


GT_STATUS cpssAppPlatformXPhyVendorIdGet
(
    IN  GT_U8                       devNum,
    IN  GT_PORT_GROUPS_BMP          portGroupsBmp,
    IN  CPSS_PHY_XSMI_INTERFACE_ENT xsmiInterface,
    IN  GT_U32                      xsmiAddr,
    OUT  GT_U16                    *vendorIdPtr
)
{
    GT_STATUS rc;
    CPSS_SYSTEM_RECOVERY_INFO_STC saved_system_recovery;
    CPSS_SYSTEM_RECOVERY_INFO_STC system_recovery;

    rc = cpssSystemRecoveryStateGet(&system_recovery);
    if (rc != GT_OK)
    {
      return rc;
    }
    saved_system_recovery = system_recovery;

    system_recovery.systemRecoveryProcess = CPSS_SYSTEM_RECOVERY_PROCESS_NOT_ACTIVE_E;
    system_recovery.systemRecoveryState = CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E;
    rc =  cpssSystemRecoveryStateSet(&system_recovery);
    if (rc != GT_OK)
    {
        return rc;
    }
    /* read vendor ID */
    rc = cpssXsmiPortGroupRegisterRead(devNum, portGroupsBmp, xsmiInterface,
                                        xsmiAddr, PHY_VENDOR_ID_REG_ADDR_CNS,
                                        PHY_ID_REG_DEV_NUM_CNS,
                                        /*OUT*/vendorIdPtr);
    if (rc != GT_OK)
    {
        CPSS_APP_PLATFORM_LOG_DBG_MAC("cpssXsmiPortGroupRegisterRead %d", rc);
    }
    rc =  cpssSystemRecoveryStateSet(&saved_system_recovery);
    if (rc != GT_OK)
    {
        return rc;
    }
    return rc;
}

GT_VOID cpssAppPlatformXSmiPhyAddrArrayBuild
(
    IN  GT_U8                       devNum,
    OUT CPSS_APP_PLATFORM_XPHY_INFO_STC    *phyInfoArray,
    OUT GT_U32                      *phyInfoArrayLenPtr
)
{
    GT_STATUS   rc;                     /* return code */
    GT_U32      portGroupId;            /* local core number */
    GT_PORT_GROUPS_BMP  portGroupsBmp;  /* port groups bitmap */
    CPSS_PHY_XSMI_INTERFACE_ENT xsmiInterface;  /* SMI i/f iterator */
    GT_U32      xsmiAddr;               /* SMI Address iterator */
    GT_U16      data;                   /* register data */
    GT_U8       maxXSMIinterface=CPSS_PHY_XSMI_INTERFACE_0_E;
    GT_U8       minXSMIinterface=CPSS_PHY_XSMI_INTERFACE_0_E;

    /*XSMI phy is not needed for Ironman*/
    if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_IRONMAN_E)
    {
        return;
    }
    *phyInfoArrayLenPtr = 0;
    maxXSMIinterface = CPSS_PHY_XSMI_INTERFACE_0_E;
    PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(devNum,portGroupId)
    {
        portGroupsBmp = 1<<portGroupId;

        for(xsmiInterface = minXSMIinterface; xsmiInterface <= maxXSMIinterface; xsmiInterface++)
        {
            for(xsmiAddr = 0; xsmiAddr < BIT_5; xsmiAddr++)
            {
                rc = cpssAppPlatformXPhyVendorIdGet(devNum, portGroupsBmp, xsmiInterface, xsmiAddr, &data);
                if(rc != GT_OK)
                {
                    cpssOsPrintf("cpssAppPlatformXPhyVendorIdGet FAIL:devNum=%d,portGroupId=%d,xsmiInterface=%d,xsmiAddr=%d,\n",
                                devNum, portGroupId, xsmiInterface, xsmiAddr);
                    continue;
                }

                if ((data != MRVL_PHY_UID_CNS) && (data != MRVL_PHY_REV_A_UID_CNS))
                {
                    continue;
                }
                else if ((data == MRVL_PHY_UID_CNS || data == MRVL_PHY_REV_A_UID_CNS) &&
                        appDemoCheckEnable == GT_TRUE)
                {
                    vendorId = data;
                    appDemoCheckEnable = GT_FALSE;
                }
                rc = cpssAppPlatformXPhyIdGet(devNum, portGroupsBmp, xsmiInterface, xsmiAddr, &data);
                if(rc != GT_OK)
                {
                    cpssOsPrintf("cpssAppPlatformXPhyIdGet FAIL:devNum=%d,portGroupId=%d,xsmiInterface=%d,xsmiAddr=%d\n",
                                 devNum, portGroupId, xsmiInterface, xsmiAddr);
                    continue;
                }
                phyInfoArray[*phyInfoArrayLenPtr].hostDevNum = devNum;
                phyInfoArray[*phyInfoArrayLenPtr].portGroupId = portGroupId;
                phyInfoArray[*phyInfoArrayLenPtr].phyAddr = (GT_U16)xsmiAddr;
                phyInfoArray[*phyInfoArrayLenPtr].xsmiInterface = xsmiInterface;
                phyInfoArray[*phyInfoArrayLenPtr].phyType = data;
                phyInfoArray[*phyInfoArrayLenPtr].hostDevNum = devNum;
                cpssOsPrintf("%d)hostDevNum=%d,portGroupId=%d,phyAddr=0x%x,xsmiInterface=%d,phyType=0x%x\n",
                             *phyInfoArrayLenPtr,
                             phyInfoArray[*phyInfoArrayLenPtr].hostDevNum,
                             phyInfoArray[*phyInfoArrayLenPtr].portGroupId,
                             phyInfoArray[*phyInfoArrayLenPtr].phyAddr,
                             phyInfoArray[*phyInfoArrayLenPtr].xsmiInterface,
                             phyInfoArray[*phyInfoArrayLenPtr].phyType);
                (*phyInfoArrayLenPtr)++;
            }
        }
    }
    PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(devNum,portGroupId)

    return;
}

GT_STATUS cpssAppPlatformPhyIdGet
(
    IN  GT_U8                       devNum,
    IN  GT_PORT_GROUPS_BMP          portGroupsBmp,
    IN  CPSS_PHY_SMI_INTERFACE_ENT  smiInterface,
    IN  GT_U32                      smiAddr,
    OUT GT_U16                     *phyIdPtr
)
{
    GT_STATUS rc;
    GT_U16    data;

    /* read PHY ID */
    rc = cpssSmiRegisterReadShort(devNum, portGroupsBmp, smiInterface,
            smiAddr, PHY_ID_REG_ADDR_CNS, &data);
    if (rc != GT_OK)
    {
        cpssOsPrintf("cpssSmiRegisterReadShort : %d", rc);
        return rc;
    }

    if (data != 0xFFFF)
    {
        *phyIdPtr = (data >> 4) & 0x3F;
        phyModelNum = *phyIdPtr;
    }
    else
        *phyIdPtr = 0;

    return GT_OK;
}

GT_VOID cpssAppPlatformSmiPhyAddrArrayBuild
(
    IN  GT_U8                       devNum,
    OUT CPSS_APP_PLATFORM_XPHY_INFO_STC    *phyInfoArray,
    OUT GT_U32                      *phyInfoArrayLenPtr
)
{
    GT_STATUS   rc;                     /* return code */
    GT_U32      portGroupId;            /* local core number */
    GT_PORT_GROUPS_BMP  portGroupsBmp;  /* port groups bitmap */
    GT_U32      smiAddr;                /* SMI Address iterator */
    GT_U16      data1;                   /* register data */
    GT_U32      data;                   /* register data */
    CPSS_PHY_SMI_INTERFACE_ENT smiInterface = 0;
    GT_U32      smiMaxNum = BIT_5;
    *phyInfoArrayLenPtr = 0;

    if (    PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_IRONMAN_E
         || PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5P_E
       )
    {
        smiInterface = CPSS_PHY_SMI_INTERFACE_0_E;
        if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_IRONMAN_E)
        {
            smiMaxNum = BIT_4;
        }
    }
    else
    {
        smiInterface = CPSS_PHY_SMI_INTERFACE_1_E;
    }
    PRV_CPSS_GEN_PP_START_LOOP_PORT_GROUPS_MAC(devNum,portGroupId)
    {
        portGroupsBmp = 1<<portGroupId;

        for(smiAddr = 0; smiAddr < smiMaxNum; smiAddr++)
        {
            if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_IRONMAN_E)
            {
                data = (0x2D | (0x9 << 16));
            }
            else if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5P_E)
            {
                data = 0x2D;
            }
            else
            {
                rc = cpssAppPlatformPhyIdGet(devNum, portGroupsBmp, smiInterface, smiAddr, &data1);
                if(rc != GT_OK)
                {
                    cpssOsPrintf("cpssAppPlatformPhyIdGet FAIL:devNum=%d,portGroupId=%d,smiInterface=%d,smiAddr=%d\n",
                            devNum, portGroupId, CPSS_PHY_SMI_INTERFACE_1_E, smiAddr);
                    continue;
                }
                data = data1;
            }

            phyInfoArray[*phyInfoArrayLenPtr].hostDevNum = devNum;
            phyInfoArray[*phyInfoArrayLenPtr].portGroupId = portGroupId;
            phyInfoArray[*phyInfoArrayLenPtr].phyAddr = (GT_U16)smiAddr;
            phyInfoArray[*phyInfoArrayLenPtr].xsmiInterface = (CPSS_PHY_XSMI_INTERFACE_ENT)smiInterface;
            phyInfoArray[*phyInfoArrayLenPtr].phyType = data;
            cpssOsPrintf("%d)hostDevNum=%d,portGroupId=%d,phyAddr=0x%x,smiInterface=%d,phyType=0x%x\n",
                    *phyInfoArrayLenPtr,
                    phyInfoArray[*phyInfoArrayLenPtr].hostDevNum,
                    phyInfoArray[*phyInfoArrayLenPtr].portGroupId,
                    phyInfoArray[*phyInfoArrayLenPtr].phyAddr,
                    phyInfoArray[*phyInfoArrayLenPtr].xsmiInterface,
                    phyInfoArray[*phyInfoArrayLenPtr].phyType);
            (*phyInfoArrayLenPtr)++;
        }
    }
    PRV_CPSS_GEN_PP_END_LOOP_PORT_GROUPS_MAC(devNum,portGroupId)
        return;
}

GT_STATUS cpssAppPlatformPhyMpdInit
(
    GT_U8 devNum
)
{
    UINT_32                                     rel_ifIndex, i;
    MPD_RESULT_ENT                              rc = MPD_OK_E;
    MPD_CALLBACKS_STC                           phy_callbacks;
    MPD_PORT_INIT_DB_STC                        phy_entry;
    CPSS_APP_PLATFORM_XPHY_INFO_STC             phyInfoArray[256];
    CPSS_APP_PLATFORM_XPHY_INFO_STC             smiPhyInfoArray[256];
    GT_U16                                      data;
    GT_U32                                      phyInfoArrayLen = 0;
    GT_U32                                      smiPhyInfoArrayLen = 0;
    GT_U32                                      currBoardType = ASK_XCAT5X_A0_BOARD_RD_CNS;
    GT_U32                                      smiAddr;
    MPD_TYPE_ENT                                phyType;
    CPSS_APP_PLATFORM_XPHY_STATIC_INFO_STC      staticPhyInfo;
    MPD_OPERATIONS_PARAMS_UNT                   opParams;

    memset(phyInfoArray, 0, sizeof(phyInfoArray));
    memset(smiPhyInfoArray, 0, sizeof(smiPhyInfoArray));

    cpssAppPlatformXSmiPhyAddrArrayBuild(devNum, phyInfoArray, &phyInfoArrayLen);
    cpssAppPlatformSmiPhyAddrArrayBuild(devNum, smiPhyInfoArray, &smiPhyInfoArrayLen);

    /* bind basic callbacks needed from host application */
    memset(&phy_callbacks,0,sizeof(phy_callbacks));

    phy_callbacks.sleep_PTR               = cpssAppPlatformPhyOsDelay;
    phy_callbacks.alloc_PTR               = cpssAppPlatformPhyOsMalloc;
    phy_callbacks.free_PTR                = cpssAppPlatformPhyOsFree;
    phy_callbacks.debug_bind_PTR          = cpssAppPlatformPhyDebugBind;
    phy_callbacks.is_active_PTR          = cpssAppPlatformPhyDebugIsActive;
    phy_callbacks.debug_log_PTR           = cpssAppPlatformPhyDebugLog;
    phy_callbacks.txEnable_PTR            = NULL;
    phy_callbacks.handle_failure_PTR      = cpssAppPlatformPhyHandleFailure;
    phy_callbacks.getFwFiles_PTR          = cpssAppPlatformPhyGetFwFiles;
    phy_callbacks.mdioRead_PTR            = cpssAppPlatformPhyXsmiMdioRead;
    phy_callbacks.mdioWrite_PTR           = cpssAppPlatformPhyXsmiMdioWrite;
    phy_callbacks.logging_PTR             = cpssAppPlatformPhyLogging;


    if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_IRONMAN_E)
    {
        rc = genRegisterSet(devNum, 0, 0x80020110, 0x00002200, 0x0000FF00);
        if (rc != GT_OK)
        {
            cpssOsPrintf("genRegisterSet(devNum, 0, 0x80020110, 0x00002200, 0x0000FF00);: rc = %d\n", rc);
            return rc;
        }
        currBoardType = PRV_CPSS_PP_MAC(devNum)->devType;

        /* disable PCH header for PTP mode */
        for(smiAddr = 0; smiAddr < 16; smiAddr++)
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
        for(smiAddr = 0; smiAddr < 16; smiAddr++)
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
            /* LED Link/Activity */
            rc = cpssSmiRegisterWriteShort(devNum, CPSS_PORT_GROUP_UNAWARE_MODE_CNS, CPSS_PHY_SMI_INTERFACE_0_E, smiAddr, 16, 0x1181);
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
    }
    else if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5P_E)
    {
        rc = genRegisterSet(devNum, 0, 0x3c00010c, 0x01111220, 0xFFFFFFFF);
        if (rc != GT_OK)
        {
            cpssOsPrintf("genRegisterSet(0, 0, 0x3c00010c, 0x01111220, 0xFFFFFFFF): rc = %d\n", rc);
            return rc;
        }
        currBoardType = ASK_XCAT5P_A0_BOARD_RD_CNS;
    }
    else if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5X_E)
    {
        rc = genRegisterSet(devNum, 0, 0x8002010c, 0x22111133, 0xFFFFFFFF);
        if (rc != GT_OK)
        {
            cpssOsPrintf("genRegisterSet(0, 0, 0x8002010c, 0x22111133, 0xFFFFFFFF): rc = %d\n", rc);
            return rc;
        }
        currBoardType = ASK_XCAT5X_A0_BOARD_RD_CNS;
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
    }

    rc = mpdDriverInitDb(&phy_callbacks);
    if (rc != MPD_OK_E) {
        cpssOsPrintf("cpssAppPlatformPhyMpdInit failed in mpdDriverInitDbCb \n");
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    for(i = 0; i < phyInfoArrayLen ; i++)
    {
        rc = cpssAppPlatformPhyPortNumGet(currBoardType, phyInfoArray[i], &staticPhyInfo);
        if(rc != GT_OK)
        {
            cpssOsPrintf("cpssAppPlatformPhyMpdInit failed \n");
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }
        phyType = cpssAppPlatformMpdPhyTypeGet(phyInfoArray[i].phyType);

        memset(&phy_entry,0,sizeof(phy_entry));
        phy_entry.port                          = staticPhyInfo.portNum;
        phy_entry.phyNumber                     = staticPhyInfo.phyNumber;
        phy_entry.phyType                       = phyType;
        phy_entry.transceiverType               = staticPhyInfo.transceiverType;
        phy_entry.mdioInfo.mdioAddress          = phyInfoArray[i].phyAddr;
        phy_entry.mdioInfo.mdioDev              = phyInfoArray[i].hostDevNum;
        phy_entry.mdioInfo.mdioBus              = phyInfoArray[i].xsmiInterface;
        phy_entry.usxInfo.usxType               = staticPhyInfo.usxType;
        phy_entry.disableOnInit                 = GT_FALSE;

        rel_ifIndex = cpssAppPlatformPhyMpdIndexGet(devNum, staticPhyInfo.portNum);
        if (mpdPortDbUpdate(rel_ifIndex, &phy_entry) != MPD_OK_E) {
                cpssOsPrintf("cpssAppPlatformPhyMpdInit failed in mpdPortDbUpdateCb\n");
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }
    }

    for(i = 0; i < smiPhyInfoArrayLen; i++)
    {
        rc = cpssAppPlatformPhyPortNumGet(currBoardType, smiPhyInfoArray[i], &staticPhyInfo);
        if(rc != GT_OK)
        {
            cpssOsPrintf("gtAppDemoPhyMpdInit failed \n");
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }
        phyType = cpssAppPlatformMpdPhyTypeGet(smiPhyInfoArray[i].phyType);
        memset(&phy_entry,0,sizeof(phy_entry));
        phy_entry.port                          = staticPhyInfo.portNum;
        phy_entry.phyNumber                     = staticPhyInfo.phyNumber;
        phy_entry.phyType                       = phyType;
        phy_entry.transceiverType               = staticPhyInfo.transceiverType;
        phy_entry.mdioInfo.mdioAddress          = smiPhyInfoArray[i].phyAddr;
        phy_entry.mdioInfo.mdioDev              = smiPhyInfoArray[i].hostDevNum;
        phy_entry.mdioInfo.mdioBus              = smiPhyInfoArray[i].xsmiInterface;
        phy_entry.usxInfo.usxType               = staticPhyInfo.usxType;
        phy_entry.disableOnInit                 = GT_FALSE;

        rel_ifIndex = cpssAppPlatformPhyMpdIndexGet(devNum, staticPhyInfo.portNum);
        if (mpdPortDbUpdate(rel_ifIndex, &phy_entry) != MPD_OK_E) {
                cpssOsPrintf("gtAppDemoPhyMpdInit failed in mpdPortDbUpdateCb\n");
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }
    }


    for (i=0; i<MPD_TYPE_NUM_OF_TYPES_E; i++)
    {
         PRV_MPD_SHARED_GLBVAR_GLOBAL_DB_ACCESS_MAC()->fwDownloadType_ARR[i] = MPD_FW_DOWNLOAD_TYPE_RAM_E;
    }

    if (mpdDriverInitHw() != MPD_OK_E)
    {
        cpssOsPrintf("cpssAppPlatformPhyMpdInit failed in mpdDriverInitHwCb");
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    /* Advertise all supported speeds */
    opParams.phyAutoneg.enable       = MPD_AUTO_NEGOTIATION_ENABLE_E;
    opParams.phyAutoneg.capabilities = MPD_AUTONEG_CAPABILITIES_DEFAULT_CNS;
    opParams.phyAutoneg.masterSlave  = MPD_AUTONEGPREFERENCE_UNKNOWN_E;

    for(i = 0; i < phyInfoArrayLen ; i++)
    {
        rc = cpssAppPlatformPhyPortNumGet(currBoardType, phyInfoArray[i], &staticPhyInfo);
        if(rc != GT_OK)
        {
            cpssOsPrintf("cpssAppPlatformPhyMpdInit failed \n");
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }

        rel_ifIndex = cpssAppPlatformPhyMpdIndexGet(devNum, staticPhyInfo.portNum);
        /*cpssOsPrintf("mpdPerformPhyOperation for port %d\n", staticPhyInfo.portNum);*/

        rc = mpdPerformPhyOperation(rel_ifIndex, MPD_OP_CODE_SET_AUTONEG_E, &opParams);
        if ( MPD_OK_E != rc )
        {
            cpssOsPrintf("Error : devNum[%d]portNum[%d]rel_ifIndex[%d] : failed on [%s] status[%d]\n",
                    devNum,staticPhyInfo.portNum,rel_ifIndex,"MPD_OP_CODE_SET_AUTONEG_E",rc);
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }
    }

    cpssOsPrintf("MPD PHY initialized........ \n");

    return GT_OK;
 }
#endif /*INCLUDE_MPD*/


/*
* @internal cpssAppReferenceSystemInit function
* @endinternal
*
* @brief   configure board, PP and runtime modules based on input profiles.
*
* @param[in] inputProfileListPtr - Input profile list containing board profile,
*                                  Pp profile and runTime profile.
*                                  Note: Pp and runTime profiles are optional.
* @param[in] systemRecovery      - system recovery mode.
*
* @retval GT_OK                  - on success.
* @retval GT_BAD_PARAM           - if input profile is not board profile.
* @retval GT_FAIL                - otherwise.
*/

GT_STATUS cpssAppReferenceSystemInit
(
    IN CPSS_APP_PLATFORM_PROFILE_STC *inputProfileListPtr,
    IN CPSS_SYSTEM_RECOVERY_INFO_STC *systemRecovery
)
{
    GT_STATUS rc;

    CPSS_APP_PLATFORM_PP_PROFILE_STC  *ppProfilePtr   = NULL;
    CPSS_APP_PLATFORM_BOARD_PARAM_PP_MAP_STC *boardPpMapPtr  = NULL;
    CPSS_APP_PLATFORM_PROFILE_STC     *profileListPtr = inputProfileListPtr;


    appDemoDbEntryGet_func = appPlatformDbEntryGet;

#if (defined EZ_BRINGUP) && (defined CHX_FAMILY)

    if(GT_TRUE == ezbIsXmlLoaded())
    {
        PRV_CPSS_APP_START_LOOP_PROFILE_MAC(profileListPtr, CPSS_APP_PLATFORM_PROFILE_TYPE_PP_E)
        {

            if(profileListPtr->profileValue.ppInfoPtr == NULL)
            {
                /* PP profile is Empty. proceed to next profile */
                continue;
            }
            ppProfilePtr = profileListPtr->profileValue.ppInfoPtr;

            /* For each valid PP profile, next profile should be Board PP map profile. */
            profileListPtr++;
            if((profileListPtr->profileType != CPSS_APP_PLATFORM_PROFILE_TYPE_BOARD_E) ||
                    (profileListPtr->profileValue.boardInfoPtr == NULL))
            {
                CPSS_APP_PLATFORM_LOG_ERR_MAC("Board PP_MAP profile is not found after PP profile.\n \
                        So moving on to next PP profile on the list\n");
                continue;
            }
            if((profileListPtr->profileValue.boardInfoPtr->boardInfoType != CPSS_APP_PLATFORM_BOARD_PARAM_PP_MAP_E) ||
                    (profileListPtr->profileValue.boardInfoPtr->boardParam.ppMapPtr == NULL))
            {
                CPSS_APP_PLATFORM_LOG_ERR_MAC("Board PP_MAP profile is not found after PP profile.\n \
                        So moving on to next PP profile on the list\n");
                continue;
            }

#ifdef INCLUDE_MPD

            /* check if we need to init the MPD to support phys */
            rc = cpssAppPlatformEzbMpdPpPortInit(profileListPtr->profileValue.runTimeInfoPtr->runtimeInfoValue.trafficEnablePtr);
            CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssAppPlatformEzbMpdPpPortInit);

            rc = cpssAppPlatformEzbPortSpeedSet(profileListPtr->profileValue.runTimeInfoPtr->runtimeInfoValue.trafficEnablePtr);
            CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssAppPlatformEzbPortSpeedSet);
#endif /*INCLUDE_MPD*/
            boardPpMapPtr = profileListPtr->profileValue.boardInfoPtr->boardParam.ppMapPtr;

            rc = cpssAppPlatformEzbDevInfoSerdesGet(boardPpMapPtr->devNum, boardPpMapPtr);
            CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssAppPlatformEzbDevInfoSerdesGet);


            rc = cpssAppPlatformEzbSerdesPolarityGet(boardPpMapPtr->devNum, ppProfilePtr);
            CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssAppPlatformEzbSerdesPolarityGet);

        }
        PRV_CPSS_APP_END_LOOP_PROFILE_MAC(profileListPtr, CPSS_APP_PLATFORM_PROFILE_TYPE_PP_E)


    }
#endif /*#if (defined EZ_BRINGUP) && (defined CHX_FAMILY)*/


    rc =  cpssAppPlatformSystemInit(inputProfileListPtr, systemRecovery);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssAppPlatformSystemInit);

    profileListPtr = inputProfileListPtr;

    PRV_CPSS_APP_START_LOOP_PROFILE_MAC(profileListPtr, CPSS_APP_PLATFORM_PROFILE_TYPE_PP_E)
    {

        if(profileListPtr->profileValue.ppInfoPtr == NULL)
        {
            /* PP profile is Empty. proceed to next profile */
            continue;
        }
        ppProfilePtr = profileListPtr->profileValue.ppInfoPtr;

        /* For each valid PP profile, next profile should be Board PP map profile. */
        profileListPtr++;
        if((profileListPtr->profileType != CPSS_APP_PLATFORM_PROFILE_TYPE_BOARD_E) ||
                (profileListPtr->profileValue.boardInfoPtr == NULL))
        {
            CPSS_APP_PLATFORM_LOG_ERR_MAC("Board PP_MAP profile is not found after PP profile.\n \
                    So moving on to next PP profile on the list\n");
            continue;
        }
        if((profileListPtr->profileValue.boardInfoPtr->boardInfoType != CPSS_APP_PLATFORM_BOARD_PARAM_PP_MAP_E) ||
                (profileListPtr->profileValue.boardInfoPtr->boardParam.ppMapPtr == NULL))
        {
            CPSS_APP_PLATFORM_LOG_ERR_MAC("Board PP_MAP profile is not found after PP profile.\n \
                    So moving on to next PP profile on the list\n");
            continue;
        }
        boardPpMapPtr = profileListPtr->profileValue.boardInfoPtr->boardParam.ppMapPtr;

#ifdef INCLUDE_MPD
        if (ppProfilePtr->initMpd)
        {
            rc = cpssAppPlatformPhyMpdInit(boardPpMapPtr->devNum);
            CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssAppPlatformPhyMpdInit);
        }
#endif

        if ((PRV_CPSS_SIP_6_CHECK_MAC(boardPpMapPtr->devNum) && PRV_CPSS_PP_MAC(boardPpMapPtr->devNum)->devFamily != CPSS_PP_FAMILY_DXCH_IRONMAN_E) ||
            (PRV_CPSS_PP_MAC(boardPpMapPtr->devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5_E && ! ppProfilePtr->internalCpu))
        {
            rc = cpssDevSupportSystemResetSet(CAST_SW_DEVNUM(boardPpMapPtr->devNum), GT_TRUE);
            CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDevSupportSystemResetSet);
        }

    }
    PRV_CPSS_APP_END_LOOP_PROFILE_MAC(profileListPtr, CPSS_APP_PLATFORM_PROFILE_TYPE_PP_E)

        return GT_OK;
}



#define PRV_APP_REF_ADD_PROFILE_TO_DB(_profile_name) \
    extern void _profile_name##_add_profile (void);\
    _profile_name##_add_profile();

    GT_VOID appRefDefaultProfilesInit
(
 void
 )
{
    if(GT_TRUE == mainProfilesInitialized)
    {
        return;
    }

    appRefLogInit();

    /*AC5X*/

    PRV_APP_REF_ADD_PROFILE_TO_DB(WM_AC5X_all)
        PRV_APP_REF_ADD_PROFILE_TO_DB( WM_AC5X_B2B_all )
        PRV_APP_REF_ADD_PROFILE_TO_DB( WM_AC5X_B2B_noPorts_all )
        PRV_APP_REF_ADD_PROFILE_TO_DB( DB_AC5X_B2B_all )
        PRV_APP_REF_ADD_PROFILE_TO_DB( DB_AC5X_B2B_noPorts_all )
        PRV_APP_REF_ADD_PROFILE_TO_DB( DB_AC5X_B2B_int_cpu_all )
        PRV_APP_REF_ADD_PROFILE_TO_DB( DB_AC5X_B2B_int_cpu_noPorts_all )
        PRV_APP_REF_ADD_PROFILE_TO_DB( WM_AC5X_int_cpu_all )
        PRV_APP_REF_ADD_PROFILE_TO_DB( WM_AC5X_board_only )
        PRV_APP_REF_ADD_PROFILE_TO_DB( WM_AC5X_PP_only)
        PRV_APP_REF_ADD_PROFILE_TO_DB( WM_AC5X_RT)
        PRV_APP_REF_ADD_PROFILE_TO_DB( WM_AC5X_all_noports )
        PRV_APP_REF_ADD_PROFILE_TO_DB( WM_AC5X_int_cpu_all_noports )
        PRV_APP_REF_ADD_PROFILE_TO_DB( WM_AC5X_RT_noports)
        PRV_APP_REF_ADD_PROFILE_TO_DB( WM_AC5X_PP_RT )
        PRV_APP_REF_ADD_PROFILE_TO_DB( WM_AC5X_PP_RT_int_cpu )
        PRV_APP_REF_ADD_PROFILE_TO_DB( WM_AC5X_PP_RT_noports )
        PRV_APP_REF_ADD_PROFILE_TO_DB( WM_AC5X_PP_RT_int_cpu_noports )

        PRV_APP_REF_ADD_PROFILE_TO_DB( DB_AC5X_8SFP_all )
        PRV_APP_REF_ADD_PROFILE_TO_DB( DB_AC5X_8SFP_int_cpu_all )
        PRV_APP_REF_ADD_PROFILE_TO_DB( RD_AC5X_32SFP_int_cpu_all )
        PRV_APP_REF_ADD_PROFILE_TO_DB( RD_AC5X_32SFP_int_cpu_all_noports )
        PRV_APP_REF_ADD_PROFILE_TO_DB( DB_AC5X_8SFP_board_only )
        PRV_APP_REF_ADD_PROFILE_TO_DB( DB_AC5X_8SFP_PP_only)
        PRV_APP_REF_ADD_PROFILE_TO_DB( DB_AC5X_8SFP_RT)
        PRV_APP_REF_ADD_PROFILE_TO_DB( DB_AC5X_8SFP_all_noports )
        PRV_APP_REF_ADD_PROFILE_TO_DB( DB_AC5X_8SFP_int_cpu_all_noports )
        PRV_APP_REF_ADD_PROFILE_TO_DB( DB_AC5X_8SFP_RT_noports)

        /*AC5P*/

        PRV_APP_REF_ADD_PROFILE_TO_DB( WM_AC5P_all )
        PRV_APP_REF_ADD_PROFILE_TO_DB( WM_AC5P_all_noports )
        PRV_APP_REF_ADD_PROFILE_TO_DB( DB_AC5P_8SFP_all )
        PRV_APP_REF_ADD_PROFILE_TO_DB( DB_AC5P_8SFP_all_noports )
        PRV_APP_REF_ADD_PROFILE_TO_DB( RD_AC5P_32SFP_all )
        PRV_APP_REF_ADD_PROFILE_TO_DB( RD_AC5P_32SFP_all_noports )
        PRV_APP_REF_ADD_PROFILE_TO_DB( WM_AC5P_int_cpu_all )
        PRV_APP_REF_ADD_PROFILE_TO_DB( WM_AC5P_RT)
        PRV_APP_REF_ADD_PROFILE_TO_DB( WM_AC5P_Link)
        PRV_APP_REF_ADD_PROFILE_TO_DB( WM_AC5P_Rx)
        PRV_APP_REF_ADD_PROFILE_TO_DB ( WM_AC5P_board_only )
        PRV_APP_REF_ADD_PROFILE_TO_DB (WM_AC5P_PP_only)

        /*Falcon*/

        PRV_APP_REF_ADD_PROFILE_TO_DB( WM_Falcon_12_8_R0_1_all )
        PRV_APP_REF_ADD_PROFILE_TO_DB( WM_Falcon_12_8_R0_1_all_no_ports )
        PRV_APP_REF_ADD_PROFILE_TO_DB( WM_Falcon_12_8_Z2_all )
        PRV_APP_REF_ADD_PROFILE_TO_DB( Falcon_12_8_Belly2Belly_all )
        PRV_APP_REF_ADD_PROFILE_TO_DB( Falcon_12_8_Belly2Belly_all_no_ports )
        PRV_APP_REF_ADD_PROFILE_TO_DB( Falcon_6_4_board_only )
        PRV_APP_REF_ADD_PROFILE_TO_DB( Falcon_6_4_PP_only)
        PRV_APP_REF_ADD_PROFILE_TO_DB( Falcon_6_4_RT)
        PRV_APP_REF_ADD_PROFILE_TO_DB( Falcon_6_4_Link)
        PRV_APP_REF_ADD_PROFILE_TO_DB( Falcon_6_4_Rx)

        /*IRONMAN*/
        PRV_APP_REF_ADD_PROFILE_TO_DB( WM_IRONMAN_all )
        PRV_APP_REF_ADD_PROFILE_TO_DB( DB_IRONMAN_intCPU_all )
        PRV_APP_REF_ADD_PROFILE_TO_DB( DB_IRONMAN_intCPU_all_noPorts )
        PRV_APP_REF_ADD_PROFILE_TO_DB( DB_IRONMAN_all )
        PRV_APP_REF_ADD_PROFILE_TO_DB( DB_IRONMAN_all_noPorts )
        PRV_APP_REF_ADD_PROFILE_TO_DB( DB_IRONMAN_M_all )
        PRV_APP_REF_ADD_PROFILE_TO_DB( DB_IRONMAN_M_all_noPorts )
        PRV_APP_REF_ADD_PROFILE_TO_DB( DB_IRONMAN_L_intCPU_all_phy )
        PRV_APP_REF_ADD_PROFILE_TO_DB( DB_IRONMAN_L_intCPU_all_noPorts_phy )
        PRV_APP_REF_ADD_PROFILE_TO_DB( DB_IRONMAN_L_all_phy )
        PRV_APP_REF_ADD_PROFILE_TO_DB( DB_IRONMAN_L_all_noPorts_phy )

        /*AC5*/
        PRV_APP_REF_ADD_PROFILE_TO_DB( WM_AC5_all )
        PRV_APP_REF_ADD_PROFILE_TO_DB( DB_AC5_all )
        PRV_APP_REF_ADD_PROFILE_TO_DB( DB_AC5_all_noPorts )
        PRV_APP_REF_ADD_PROFILE_TO_DB( DB_AC5_all_noPorts_externalCpu )
        PRV_APP_REF_ADD_PROFILE_TO_DB( DB_AC5_all_externalCpu )

        /*ALDRIN*/
        PRV_APP_REF_ADD_PROFILE_TO_DB( DB_ALD_all )
        PRV_APP_REF_ADD_PROFILE_TO_DB( DB_ALDB2B_all )
        PRV_APP_REF_ADD_PROFILE_TO_DB( DB_ALD_runtime )
        PRV_APP_REF_ADD_PROFILE_TO_DB( DB_ALD_all_legacy )

        /*ALDRIN2*/
        PRV_APP_REF_ADD_PROFILE_TO_DB( DB_ALD2_48MG6CG_all )
        PRV_APP_REF_ADD_PROFILE_TO_DB( DB_ALD2_48MG6CG_all )
        PRV_APP_REF_ADD_PROFILE_TO_DB( DB_ALD2_48MG6CG_all_ap )
        PRV_APP_REF_ADD_PROFILE_TO_DB( DB_ALD2_48MG6CG_all_legacy )
        PRV_APP_REF_ADD_PROFILE_TO_DB( DB_ALD2_48MG6CG_all_legacy_ap )
        PRV_APP_REF_ADD_PROFILE_TO_DB( DB_ALD2_48MG6CG_board )
        PRV_APP_REF_ADD_PROFILE_TO_DB( DB_ALD2_48MG6CG_pp )
        PRV_APP_REF_ADD_PROFILE_TO_DB( DB_ALD2_48MG6CG_runtime )

        /*HARRIER*/
        PRV_APP_REF_ADD_PROFILE_TO_DB( WM_HARRIER_all )
        PRV_APP_REF_ADD_PROFILE_TO_DB( WM_HARRIER_all_noports )
        PRV_APP_REF_ADD_PROFILE_TO_DB( WM_HARRIER_RT )
        PRV_APP_REF_ADD_PROFILE_TO_DB( WM_HARRIER_Link )
        PRV_APP_REF_ADD_PROFILE_TO_DB( WM_HARRIER_Rx )
        PRV_APP_REF_ADD_PROFILE_TO_DB( WM_HARRIER_board_only )
        PRV_APP_REF_ADD_PROFILE_TO_DB( WM_HARRIER_PP_only )

        mainProfilesInitialized = GT_TRUE;
}

/**
 * @internal cpssAppPlatformSysInit function
 * @endinternal
 *
 * @brief   Init Board, PP, runtime based on input profile.
 *
 * @param[in] profileName           - Profile name.
 *
 * @retval GT_OK                    - on success,
 * @retval GT_FAIL                  - otherwise.
 *
 */
    GT_STATUS cpssAppPlatformSysInit
(
 IN GT_CHAR  *profileName
 )
{
    GT_STATUS                      rc         = GT_OK;
    CPSS_APP_PLATFORM_PROFILE_STC *profilePtr = NULL;
    GT_U8                          devNum     = 0;

    CPSS_APP_PLATFORM_LOG_ENTRY_MAC();

    cpssAppPlatformInitialized = GT_TRUE;

    if (!profileName)
        return GT_BAD_PTR;

    CPSS_APP_PLATFORM_LOG_INFO_MAC("Handle cpssAppPlatformSysInit ProfileName=[%s] \n", profileName);

#ifdef LINUX_NOKM
    rc = prvUtilsHostI2cInit();
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, prvUtilsHostI2cInit);
#endif

    appRefDefaultProfilesInit();


    rc = cpssAppPlatformProfileGet(profileName, &profilePtr);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssAppPlatformProfileGet);

    if(GT_TRUE == ezbIsXmlLoaded())
    {
        rc = cpssAppPlatformEzbUpdateProfile(profilePtr);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssAppPlatformEzbUpdateProfile);

        profilePtr->profileValue.ppInfoPtr->isEzbIsXmlLoaded = GT_TRUE;
    }
    else
    {
        profilePtr->profileValue.ppInfoPtr->isEzbIsXmlLoaded = GT_FALSE;
    }

    rc = cpssAppReferenceSystemInit(profilePtr, GT_FALSE);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssAppReferenceSystemInit);

    for(devNum = 0; devNum < CPSS_APP_PLATFORM_MAX_PP_CNS; devNum++)
    {
        if(PRV_CPSS_IS_DEV_EXISTS_MAC(devNum))
        {
            rc = cpssAppRefEventFuncInit(devNum);
            CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssAppRefEventFuncInit);

            rc = localUtfInitRef(devNum);
            CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, localUtfInitRef);
        }
    }

    return GT_OK;
}

/**
 * @internal cpssSysInit function
 * @endinternal
 *
 * @brief   Init Board, PP, runtime based on input profile.
 *          This function is to allow cpssSysInit() call from old cmd shell (without Lua cpssSysInit wrapper)
 *
 * @param[in] profileName           - Profile name.
 *
 * @retval GT_OK                    - on success,
 * @retval GT_FAIL                  - otherwise.
 *
 */

    GT_STATUS cpssSysInit
(
 IN GT_CHAR  *profileName
 )
{
    return cpssAppPlatformSysInit(profileName);
}

/**
 * @internal cpssInitSystem function
 * @endinternal
 *
 * @brief   mimicing appDemo's initSystem to enable use in CI
 *
 * @param[in] profileName           - Profile name.
 *
 * @retval GT_OK                    - on success,
 * @retval GT_FAIL                  - otherwise.
 *
 */
#ifndef MIXED_MODE
GT_STATUS cpssInitSystem
#else
GT_STATUS cpssAppPlatformInitSystem
#endif /* MIXED_MODE */
(
 IN GT_CHAR  *profileName
 )
{
    return (cpssAppPlatformSysInit(profileName));
}

/**
 * @internal cpssAppPlatformSysPpAdd function
 * @endinternal
 *
 * @brief   initilize PP device based on input profile .
 *
 * @param[in] profileName           - Profile name.
 *
 * @retval GT_OK                    - on success,
 * @retval GT_FAIL                  - otherwise.
 *
 */
    GT_STATUS cpssAppPlatformSysPpAdd
(
 IN GT_CHAR *profileName
 )
{
    GT_STATUS                      rc         = GT_OK;
    CPSS_APP_PLATFORM_PROFILE_STC *profilePtr = NULL;
    GT_U8                          devNum     = 0;

    CPSS_APP_PLATFORM_LOG_ENTRY_MAC();

    if (!profileName)
        return GT_BAD_PTR;

    CPSS_APP_PLATFORM_LOG_INFO_MAC("Handle cpssAppPlatformSysPpAdd ProfileName=[%s] \n",  profileName);

    rc = cpssAppPlatformProfileGet(profileName, &profilePtr);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssAppPlatformProfileGet);

    appDemoDbEntryGet_func = appPlatformDbEntryGet;
    rc = cpssAppPlatformSystemPpAdd(profilePtr, NULL);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssAppPlatformSystemPpAdd);

    for(devNum = 0; devNum < CPSS_APP_PLATFORM_MAX_PP_CNS; devNum++)
    {
        if(PRV_CPSS_IS_DEV_EXISTS_MAC(devNum))
        {
            rc = cpssAppRefEventFuncInit(devNum);
            CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssAppRefEventFuncInit);

            rc = localUtfInitRef(devNum);
            CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, localUtfInitRef);
        }
    }

    return GT_OK;
}

/**
 * @internal cpssAppPlatformSysPpRemove function
 * @endinternal
 *
 * @brief   remove a specific PP.
 *
 * @param[in] devNum          - PP device number.
 * @param[in] removalType     - Removal type: 0 - Managed Removal,
 *                                            1 - Unmanaged Removal,
 *                                            2 - Managed Reset
 *
 * @retval GT_OK              - on success,
 * @retval GT_FAIL            - otherwise.
 *
 */
    GT_STATUS cpssAppPlatformSysPpRemove
(
 IN GT_U8 devNum,
 IN GT_U8 removalType
 )
{
    GT_STATUS   rc = GT_OK;

    CPSS_APP_PLATFORM_LOG_ENTRY_MAC();

    rc = cpssAppPlatformSystemPpRemove(devNum, removalType);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssAppPlatformSystemPpRemove);


    if(GT_TRUE == ezbIsXmlLoaded())
    {
        cpssAppPlatformEzbFree(devNum);
    }

#ifndef ASIC_SIMULATION
    rc = cpssAppRefSerdesTrainingTaskDelete(devNum);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssAppRefSerdesTrainingTaskDelete);
#endif

    CPSS_APP_PLATFORM_LOG_EXIT_MAC(rc);
    return rc;
}

/**
 * @internal cpssAppPlatformSysRun function
 * @endinternal
 *
 * @brief   initialize run time modules based on input Profile.
 *
 * @param[in] profileName           - Profile name.
 *
 * @retval GT_OK                    - on success,
 * @retval GT_FAIL                  - otherwise.
 *
 *
 */
    GT_STATUS cpssAppPlatformSysRun
(
 IN GT_CHAR *profileName
 )
{
    GT_STATUS rc;
    CPSS_APP_PLATFORM_PROFILE_STC *profilePtr = NULL;

    CPSS_APP_PLATFORM_LOG_ENTRY_MAC();

    if (!profileName)
        return GT_BAD_PTR;

    CPSS_APP_PLATFORM_LOG_INFO_MAC("Handle cpssAppPlatformSysRun ProfileName=[%s] \n",  profileName);

    rc = cpssAppPlatformProfileGet(profileName, &profilePtr);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssAppPlatformProfileGet);

    rc = cpssAppPlatformSystemRun(profilePtr);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssAppPlatformSystemRun);

    return GT_OK;
}

/**
 * @internal cpssAppPlatformLsProfile function
 * @endinternal
 *
 * @brief   This is the function to list all available Profile .
 *
 * @retval GT_OK                    - on success,
 * @retval GT_FAIL                  - otherwise.
 *
 */
    GT_STATUS cpssAppPlatformLsProfile
(
 GT_VOID
 )
{
    GT_STATUS rc;

    CPSS_APP_PLATFORM_LOG_ENTRY_MAC();

    appRefDefaultProfilesInit();

    rc = cpssAppPlatformListProfiles();
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssAppPlatformListProfiles);

    return GT_OK;
}

/**
 * @internal cpssAppPlatformLsTestProfile function
 * @endinternal
 *
 * @brief   This is the function to list all available Profile .
 *
 * @retval GT_OK                    - on success,
 * @retval GT_FAIL                  - otherwise.
 *
 */
    GT_STATUS cpssAppPlatformLsTestProfile
(
 GT_VOID
 )
{
    GT_STATUS rc;

    CPSS_APP_PLATFORM_LOG_ENTRY_MAC();

    rc = cpssAppPlatformListTestProfiles();
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssAppPlatformListProfiles);

    return GT_OK;
}

