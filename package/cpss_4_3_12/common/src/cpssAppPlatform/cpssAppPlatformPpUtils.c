/*******************************************************************************
*              (c), Copyright 2018, Marvell International Ltd.                 *
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
* @file cpssAppPlatformPpUtils.c
*
* @brief This file contains APIs for CPSS PP utility functions.
*
* @version   1
********************************************************************************
*/

#include <cpssAppPlatformSysConfig.h>
#include <cpssAppPlatformPpConfig.h>
#include <cpssAppPlatformPpUtils.h>
#include <cpssAppPlatformPortInit.h>

#include <cpssCommon/private/prvCpssSkipList.h>
#include <cpssDriver/pp/prvCpssDrvPpDefs.h>
#include <gtExtDrv/drivers/gtPciDrv.h>

#include <extUtils/common/cpssEnablerUtils.h>

#if defined CHX_FAMILY
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwInitLedCtrl.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwInit.h>
#include <cpss/dxCh/dxChxGen/private/lpm/tcam/prvCpssDxChLpmTcamCommonTypes.h>

#include <cpss/dxCh/dxChxGen/port/cpssDxChPortTxDba.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortManager.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgSecurityBreach.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgGen.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgStp.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgMc.h>
#include <cpss/dxCh/dxChxGen/nst/cpssDxChNstPortIsolation.h>
#include <cpss/dxCh/dxChxGen/nst/cpssDxChNst.h>
#include <cpss/dxCh/dxChxGen/ip/cpssDxChIp.h>
#include <cpss/dxCh/dxChxGen/ipLpmEngine/private/cpssDxChPrvIpLpm.h>
#include <cpss/dxCh/dxChxGen/ipLpmEngine/cpssDxChIpLpm.h>
#include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIf.h>
#include <cpss/dxCh/dxChxGen/trunk/cpssDxChTrunk.h>
#include <cpss/dxCh/dxChxGen/cos/cpssDxChCos.h>
#include <cpss/dxCh/dxChxGen/cscd/cpssDxChCscd.h>
#include <cpss/dxCh/dxChxGen/tti/cpssDxChTti.h>
#include <cpss/dxCh/dxChxGen/ptp/cpssDxChPtp.h>
#include <cpss/dxCh/dxChxGen/pha/cpssDxChPha.h>
#include <cpss/dxCh/dxChxGen/policer/cpssDxChPolicer.h>
#include <cpss/dxCh/dxChxGen/tcam/cpssDxChTcam.h>
#include <cpss/dxCh/dxChxGen/pcl/cpssDxChPcl.h>
#include <cpss/dxCh/dxChxGen/mirror/cpssDxChMirror.h>
#include <cpss/dxCh/dxChxGen/phy/cpssDxChPhySmi.h>

#endif/*CHX_FAMILY*/
#include <ezbringup/cpssAppPlatformEzBringupTools.h>

#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cpssAppUtilsCommon.h>

#define DXCH_HWINIT_GLOVAR(_x) PRV_SHARED_GLOBAL_VAR_GET(mainPpDrvMod.dxChCpssHwInit._x)

#define PLL4_FREQUENCY_1250000_KHZ_CNS  1250000
#define PLL4_FREQUENCY_1093750_KHZ_CNS  1093750
#define PLL4_FREQUENCY_1550000_KHZ_CNS  1550000

/*global variables macros*/
#define PRV_SHARED_PORT_DIR_IP_LPM_SRC_GLOBAL_VAR_GET(_var)\
    PRV_SHARED_GLOBAL_VAR_GET(mainPpDrvMod.ipLpmDir.ipLpmSrc._var)
#define HWINIT_GLOVAR(_var) \
    PRV_SHARED_GLOBAL_VAR_GET(commonMod.genericHwInitDir._var)



extern GT_STATUS   prvCpssDxChPhaFwVersionPrint(IN GT_U8    devNum);

/* default values for Aldrin2 Tail Drop DBA disable configuration */
#define PRV_CPSS_ALDRIN2_TAIL_DROP_DEFAULT_MAX_BUF_LIMIT_CNS      0x28
#define PRV_CPSS_ALDRIN2_TAIL_DROP_DEFAULT_MAX_DESCR_LIMIT_CNS    0x19
#define PRV_CPSS_ALDRIN2_TAIL_DROP_DEFAULT_DBA_ALPHA_CNS          CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_E
#define PRV_CPSS_ALDRIN2_TAIL_DROP_DEFAULT_DBA_AVAILABLE_BUFS_CNS 0x9060

/* next info not saved in CPSS , so can not be retrieved from CPSS. (lack of support)
   so we need to store the DMAs allocated from the first time
*/


#ifdef LINUX_NOKM
extern GT_STATUS extDrvGetDmaSize(OUT GT_U32 * dmaSize);
#endif

/* max number of LPM blocks on eArch architecture */
#define APP_CPSS_MAX_NUM_OF_LPM_BLOCKS_CNS    20



#ifndef GM_USED
/*
 * typedef: struct CPSS_FALCON_LED_STREAM_INDICATIONS_STC
 *
 * Description:
 *      Positions of LED bit indications in stream.
 *
 * Fields:
 *
 *      ledStart            - The first bit in the LED stream indication to be driven in current chiplet
 *                            (APPLICABLE RANGES: 0..255)
 *      ledEnd              - The last bit in the LED stream indication to be driven in the current chiplet
 *                            (APPLICABLE RANGES: 0..255)
 *      cpuPort             - CPU port is connected to the current chiplet
 */
typedef struct {
    GT_U32     ledStart;
    GT_U32     ledEnd;
    GT_BOOL    cpuPort;
} CPSS_IRONMAN_LED_STREAM_INDICATIONS_STC;

/** @struct CPSS_IRONMAN_LED_PORT_MAC_POSITION_STC
*
*   @brief Structure maps port MAC number into it's LED port
*          position in LED server.
*
*/
typedef struct {
/** MAC number of port */
    GT_U32      macPortNumber;

    /** LED Position of port */
    GT_U32      ledPortPosition;
}CPSS_IRONMAN_LED_PORT_MAC_POSITION_STC;


static const CPSS_IRONMAN_LED_STREAM_INDICATIONS_STC ironman_led_indications[CPSS_LED_UNITS_MAX_NUM_CNS] =
{
    {64, 66,    GT_FALSE},     /* LED ports (LED Unit 0 - Ports   0,  4, 52)      */
    {64, 66,    GT_FALSE},     /* LED ports (LED Unit 1 - Ports   8, 12, 53)      */
    {64, 66,    GT_FALSE},     /* LED ports (LED Unit 2 - Ports  16, 20, 54)      */
    {64, 67,    GT_FALSE}      /* LED ports (LED Unit 3 - CPU Ports 48-51)        */
};

static const CPSS_IRONMAN_LED_PORT_MAC_POSITION_STC ironman_single_port_per_lane_ports_map[] =
{
    { 0,  0},  { 4,  1}, {52,  2},          /* LED server 0 */
    { 8,  0},  {12,  1}, {53,  2},          /* LED server 1 */
    {16,  0},  {20,  1}, {54,  2},          /* LED server 2 */
    {48,  0},  {49,  1}, {50,  2}, {51, 3}  /* LED server 3 */
};

#define CPSS_APP_PLATFORM_LED_PORT_REORDER_MAP_SIZE_CNS  13
#endif /*GM_USED*/

/************** 100/400G WA ****************/

typedef struct{
    GT_U32  sdmaPort;
    GT_U32  macPort;
}MUX_DMA_STC;
/* MUXING info */

typedef struct{
    GT_U32   firstDma;
    GT_U32   numPorts;
}SDMA_RANGE_INFO_STC;

/************** 100/400G WA ****************/

/**
* @internal appPlatformDbEntryGet function
* @endinternal
*
* @brief   Get parameter value from App DataBase.
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
GT_STATUS appPlatformDbEntryGet
(
    IN  GT_CHAR *namePtr,
    OUT GT_U32  *valuePtr
)
{
    if (NULL != cpssAppUtilsDbEntryGetCb)
    {
        return cpssAppUtilsDbEntryGetCb(namePtr, valuePtr);
    }

    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
}

/**
* @internal appPlatformDbEntryAdd function
* @endinternal
*
* @brief   Set App DataBase value.This value will be considered during system
*         initialization process.
* @param[in] namePtr                  - points to parameter name
* @param[in] value                    - parameter value
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - if name is too long
* @retval GT_BAD_PTR               - if NULL pointer
* @retval GT_NO_RESOURCE           - if Database is full
*/
GT_STATUS appPlatformDbEntryAdd
(
    IN  GT_CHAR *namePtr,
    IN  GT_U32  value
)
{
    if (NULL != cpssAppUtilsDbEntryAddCb)
    {
        return cpssAppUtilsDbEntryAddCb(namePtr, value);
    }

    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
}


/**
* @internal appPlatformDbBlocksAllocationMethodGet function
* @endinternal
*
* @brief   Get the blocks allocation method configured in the Init System
*/
GT_STATUS appPlatformDbBlocksAllocationMethodGet
(
    IN  GT_U8                                               dev,
    OUT CPSS_DXCH_LPM_RAM_BLOCKS_ALLOCATION_METHOD_ENT  *blocksAllocationMethodGet
)
{
    GT_U32 value;
    (void)dev;
    if(appPlatformDbEntryGet("lpmRamMemoryBlocksCfg.lpmRamBlocksAllocationMethod", &value) == GT_OK)
        *blocksAllocationMethodGet = (CPSS_DXCH_LPM_RAM_BLOCKS_ALLOCATION_METHOD_ENT)value;
    else
        *blocksAllocationMethodGet = CPSS_DXCH_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITHOUT_BLOCK_SHARING_E;

    return GT_OK;
}

/**
* @internal appPlatformBc2IpLpmRamDefaultConfigCalc function
* @endinternal
*
* @brief   This function calculate the default RAM LPM DB configuration for LPM management.
*
* @param[in] devNum                   - The Pp device number to get the parameters for.
* @param[in] maxNumOfPbrEntries       - number of PBR entries to deduct from the LPM memory calculations
*
* @param[out] ramDbCfgPtr              - (pointer to) ramDbCfg structure to hold the defaults calculated
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS appPlatformBc2IpLpmRamDefaultConfigCalc
(
    IN  GT_U8                                    devNum,
    IN  GT_U32                                   maxNumOfPbrEntries,
    OUT CPSS_DXCH_LPM_RAM_CONFIG_STC             *ramDbCfgPtr
)
{
    GT_U32 i=0;
    GT_STATUS rc = GT_OK;
    GT_U32 blockSizeInBytes;
    GT_U32 blockSizeInLines;
    GT_U32 lastBlockSizeInLines;
    GT_U32 lastBlockSizeInBytes;
    GT_U32 lpmRamNumOfLines;
    GT_U32 numOfPbrBlocks;
    GT_U32 maxNumOfPbrEntriesToUse;
    GT_U32 value;

    if(appPlatformDbEntryGet("maxNumOfPbrEntries", &value) == GT_OK)
        maxNumOfPbrEntriesToUse = value;
    else
        maxNumOfPbrEntriesToUse = maxNumOfPbrEntries;

    /*relevant for BC3 only*/
    if(appPlatformDbEntryGet("lpmMemMode", &value) == GT_OK)
        ramDbCfgPtr->lpmMemMode = value?CPSS_DXCH_LPM_RAM_MEM_MODE_HALF_MEM_SIZE_E:CPSS_DXCH_LPM_RAM_MEM_MODE_FULL_MEM_SIZE_E;
    else
        ramDbCfgPtr->lpmMemMode = CPSS_DXCH_LPM_RAM_MEM_MODE_FULL_MEM_SIZE_E;

    lpmRamNumOfLines = PRV_CPSS_DXCH_LPM_RAM_GET_NUM_OF_LINES_MAC(PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.lpmRam);

    /*if we are working in half memory mode - then do all the calculations for half size,return to the real values later in the code*/
    if(ramDbCfgPtr->lpmMemMode == CPSS_DXCH_LPM_RAM_MEM_MODE_HALF_MEM_SIZE_E)
    {
        lpmRamNumOfLines/=2;
    }

    blockSizeInLines = (lpmRamNumOfLines/APP_CPSS_MAX_NUM_OF_LPM_BLOCKS_CNS);
    if (blockSizeInLines==0)
    {
        /* can not create a shadow with the current lpmRam size */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }
    blockSizeInBytes = blockSizeInLines * 4;

    if (maxNumOfPbrEntriesToUse >= lpmRamNumOfLines)
    {
        /* No memory for Ip LPM */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    if(maxNumOfPbrEntriesToUse > blockSizeInLines)
    {
        numOfPbrBlocks = (maxNumOfPbrEntriesToUse + blockSizeInLines - 1) / blockSizeInLines;
        lastBlockSizeInLines = (numOfPbrBlocks*blockSizeInLines)-maxNumOfPbrEntriesToUse;
        if (lastBlockSizeInLines==0)/* PBR will fit exactly in numOfPbrBlocks */
        {
            ramDbCfgPtr->numOfBlocks = APP_CPSS_MAX_NUM_OF_LPM_BLOCKS_CNS - numOfPbrBlocks;
            lastBlockSizeInLines = blockSizeInLines; /* all of last block for IP LPM */
        }
        else/* PBR will not fit exactly in numOfPbrBlocks and we will have in the last block LPM lines together with PBR lines*/
        {
            ramDbCfgPtr->numOfBlocks = APP_CPSS_MAX_NUM_OF_LPM_BLOCKS_CNS - numOfPbrBlocks + 1;
        }
    }
    else
    {
        if (maxNumOfPbrEntriesToUse == blockSizeInLines)
        {
            ramDbCfgPtr->numOfBlocks = APP_CPSS_MAX_NUM_OF_LPM_BLOCKS_CNS-1;
            lastBlockSizeInLines = blockSizeInLines;
        }
        else
        {
            ramDbCfgPtr->numOfBlocks = APP_CPSS_MAX_NUM_OF_LPM_BLOCKS_CNS;
            lastBlockSizeInLines = blockSizeInLines - maxNumOfPbrEntriesToUse;
        }
    }

    /* number of LPM bytes ONLY when last block is shared between LPM and PBR */
    lastBlockSizeInBytes = lastBlockSizeInLines * 4;

    /*ram configuration should contain physical block sizes,we divided block sizes for a calcultion earlier in the code,
            so now return to real values*/

    if(ramDbCfgPtr->lpmMemMode == CPSS_DXCH_LPM_RAM_MEM_MODE_HALF_MEM_SIZE_E)
    {
        blockSizeInBytes*=2;
        lastBlockSizeInBytes*=2;
    }

    for (i=0;i<ramDbCfgPtr->numOfBlocks-1;i++)
    {
        ramDbCfgPtr->blocksSizeArray[i] = blockSizeInBytes;
    }

    ramDbCfgPtr->blocksSizeArray[ramDbCfgPtr->numOfBlocks-1] =
        lastBlockSizeInBytes == 0 ?
            blockSizeInBytes :   /* last block is fully LPM (not PBR) */
            lastBlockSizeInBytes;/* last block uses 'x' for LPM , rest for PBR */

    /* reset other sections */
    i = ramDbCfgPtr->numOfBlocks;
    for (/*continue i*/;i<APP_CPSS_MAX_NUM_OF_LPM_BLOCKS_CNS;i++)
    {
        ramDbCfgPtr->blocksSizeArray[i] = 0;
    }

    rc = appPlatformDbBlocksAllocationMethodGet(devNum,&ramDbCfgPtr->blocksAllocationMethod);
    if (rc != GT_OK)
    {
        return rc;
    }
    return GT_OK;
}

/**
* @internal appPlatformLpmRamConfigSet function
* @endinternal
*
* @brief  Set given list of device type and Shared memory configuration mode.
*         Should be called before cpssInitSystem().
*
* @param[in] lpmRamConfigInfoArray          - array of pairs: devType+Shared memory configuration mode
* @param[in] lpmRamConfigInfoNumOfElements  - number of valid pairs

* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_BAD_PARAM             - on lpmRamConfigInfoNumOfElements bigger than array size
*/
GT_STATUS appPlatformLpmRamConfigSet
(
    IN CPSS_DXCH_LPM_RAM_CONFIG_INFO_STC    lpmRamConfigInfoArray[CPSS_DXCH_CFG_NUM_OF_DEV_TYPES_MANAGED_CNS],
    IN GT_U32                               lpmRamConfigInfoNumOfElements
)
{
    GT_U32 i=0;

    CPSS_NULL_PTR_CHECK_MAC(lpmRamConfigInfoArray);

    if(lpmRamConfigInfoNumOfElements > CPSS_DXCH_CFG_NUM_OF_DEV_TYPES_MANAGED_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    for (i=0;i<lpmRamConfigInfoNumOfElements;i++)
    {
        PRV_APP_REF_PP_UTILS_VAR(appPlatformLpmRamConfigInfo)[i].devType =  lpmRamConfigInfoArray[i].devType;
        PRV_APP_REF_PP_UTILS_VAR(appPlatformLpmRamConfigInfo)[i].sharedMemCnfg = lpmRamConfigInfoArray[i].sharedMemCnfg;
    }

    PRV_APP_REF_PP_UTILS_VAR(appPlatformLpmRamConfigInfoNumOfElements) = lpmRamConfigInfoNumOfElements;
    PRV_APP_REF_PP_UTILS_VAR(appPlatformLpmRamConfigInfoSet) = GT_TRUE;
    return GT_OK;
}

/**
* @internal appPlatformLpmRamConfigGet function
* @endinternal
*
* @brief  Get given list of device type and Shared memory configuration mode.
*
* @param[out] lpmRamConfigInfoArray             - array of pairs: devType+Shared memory configuration mode
* @param[out] lpmRamConfigInfoNumOfElementsPtr  - (pointer to) number of valid pairs
* @param[out] lpmRamConfigInfoSetFlagPtr        - (pointer to) Flag specifying that the configuration was set
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS appPlatformLpmRamConfigGet
(
    OUT CPSS_DXCH_LPM_RAM_CONFIG_INFO_STC    lpmRamConfigInfoArray[CPSS_DXCH_CFG_NUM_OF_DEV_TYPES_MANAGED_CNS],
    OUT GT_U32                               *lpmRamConfigInfoNumOfElementsPtr,
    OUT GT_BOOL                              *lpmRamConfigInfoSetFlagPtr
)
{
    GT_U32 i=0;

    CPSS_NULL_PTR_CHECK_MAC(lpmRamConfigInfoArray);
    CPSS_NULL_PTR_CHECK_MAC(lpmRamConfigInfoNumOfElementsPtr);
    CPSS_NULL_PTR_CHECK_MAC(lpmRamConfigInfoSetFlagPtr);

    *lpmRamConfigInfoSetFlagPtr=PRV_APP_REF_PP_UTILS_VAR(appPlatformLpmRamConfigInfoSet);

    if (PRV_APP_REF_PP_UTILS_VAR(appPlatformLpmRamConfigInfoSet)==GT_FALSE)
    {
        return GT_OK;
    }

    for (i = 0; i < PRV_APP_REF_PP_UTILS_VAR(appPlatformLpmRamConfigInfoNumOfElements); i++)
    {
        lpmRamConfigInfoArray[i].devType=PRV_APP_REF_PP_UTILS_VAR(appPlatformLpmRamConfigInfo)[i].devType;
        lpmRamConfigInfoArray[i].sharedMemCnfg=PRV_APP_REF_PP_UTILS_VAR(appPlatformLpmRamConfigInfo)[i].sharedMemCnfg;
    }

    *lpmRamConfigInfoNumOfElementsPtr = PRV_APP_REF_PP_UTILS_VAR(appPlatformLpmRamConfigInfoNumOfElements);

    return GT_OK;
}

/**
* @internal appPlatformFalconIpLpmRamDefaultConfigCalc function
* @endinternal
*
* @brief   This function calculate the default RAM LPM DB configuration for LPM management.
*
* @param[in] devNum                   - The Pp device number to get the parameters for.
* @param[in] sharedTableMode          - shared tables mode
* @param[in] maxNumOfPbrEntries       - number of PBR entries to deduct from the LPM memory calculations
*
* @param[out] ramDbCfgPtr              - (pointer to) ramDbCfg structure to hold the defaults calculated
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS appPlatformFalconIpLpmRamDefaultConfigCalc
(
    IN  GT_U8                                    devNum,
    IN  CPSS_DXCH_CFG_SHARED_TABLE_MODE_ENT      sharedTableMode,
    IN  GT_U32                                   maxNumOfPbrEntries,
    OUT CPSS_DXCH_LPM_RAM_CONFIG_STC             *ramDbCfgPtr
)
{
    GT_U32 value;
    GT_U32 i=0;
    CPSS_DXCH_CFG_DEV_INFO_STC   devInfo;
    GT_STATUS rc = GT_OK;

    rc = cpssDxChCfgDevInfoGet(devNum, &devInfo);
    if (rc!=GT_OK)
    {
        return rc;
    }

    if (PRV_APP_REF_PP_UTILS_VAR(appPlatformLpmRamConfigInfoSet)==GT_TRUE)
    {
        for (i=0;i<PRV_APP_REF_PP_UTILS_VAR(appPlatformLpmRamConfigInfoNumOfElements);i++)
        {
            ramDbCfgPtr->lpmRamConfigInfo[i].devType = PRV_APP_REF_PP_UTILS_VAR(appPlatformLpmRamConfigInfo)[i].devType;
            ramDbCfgPtr->lpmRamConfigInfo[i].sharedMemCnfg = PRV_APP_REF_PP_UTILS_VAR(appPlatformLpmRamConfigInfo)[i].sharedMemCnfg;
        }
        ramDbCfgPtr->lpmRamConfigInfoNumOfElements=PRV_APP_REF_PP_UTILS_VAR(appPlatformLpmRamConfigInfoNumOfElements);
    }
    else
    {
        /* set single configuration of current device */
        ramDbCfgPtr->lpmRamConfigInfo[0].devType = devInfo.genDevInfo.devType;
        ramDbCfgPtr->lpmRamConfigInfo[0].sharedMemCnfg=sharedTableMode;
        ramDbCfgPtr->lpmRamConfigInfoNumOfElements=1;
    }

    if(appPlatformDbEntryGet("maxNumOfPbrEntries", &value) == GT_OK)
        ramDbCfgPtr->maxNumOfPbrEntries = value;
    else
        ramDbCfgPtr->maxNumOfPbrEntries = maxNumOfPbrEntries;

    if(appPlatformDbEntryGet("sharedTableMode", &value) == GT_OK)
    {
        for (i=0;i<ramDbCfgPtr->lpmRamConfigInfoNumOfElements;i++)
        {
            ramDbCfgPtr->lpmRamConfigInfo[i].sharedMemCnfg = value;
        }
    }

    ramDbCfgPtr->lpmMemMode = CPSS_DXCH_LPM_RAM_MEM_MODE_FULL_MEM_SIZE_E;/*the only mode for Falcon*/
        if(appPlatformDbEntryGet("lpmRamMemoryBlocksCfg.lpmRamBlocksAllocationMethod", &value) == GT_OK)
        ramDbCfgPtr->blocksAllocationMethod  = (CPSS_DXCH_LPM_RAM_BLOCKS_ALLOCATION_METHOD_ENT)value;
    else
        ramDbCfgPtr->blocksAllocationMethod  = CPSS_DXCH_LPM_RAM_BLOCKS_ALLOCATION_METHOD_DYNAMIC_WITHOUT_BLOCK_SHARING_E;

    return GT_OK;
}

/**
* @internal falcon_initPortDelete_WA function
* @endinternal
*
*   purpose : state CPSS what reserved resources it should use for the 'port delete' WA.
*   actually the WA is activated during 'cpssDxChPortModeSpeedSet(...,powerUp = GT_FALSE)'
*   NOTE: the function gets the same parameters as the cpssDxChPortPhysicalPortMapSet(...)
*       because we need to find non-used 'physical port number' and to 'steal'
*       one SDMA queue number.
*/
static GT_STATUS falcon_initPortDelete_WA
(
    IN GT_SW_DEV_NUM       devNum,
    IN GT_U32                      portMapArraySize,
    IN CPSS_DXCH_PORT_MAP_STC      portMapArrayPtr[],
    OUT GT_U32                     falconDeleteWaReservedCpuSdmaGlobalQueueArr[]
)
{
    GT_STATUS   rc;
    GT_U32  ii,jj;
    CPSS_DXCH_PORT_MAP_STC      *currPtr = &portMapArrayPtr[0];
    CPSS_PORTS_BMP_STC          usedPhyPorts;
    MUX_DMA_STC                 *muxed_SDMAs_Ptr;
    SDMA_RANGE_INFO_STC         falcon_3_2_SDMAs_available  = { 64 +  4 ,  4};
    SDMA_RANGE_INFO_STC         falcon_6_4_SDMAs_available  = {128 +  8 ,  8};
    SDMA_RANGE_INFO_STC         falcon_12_8_SDMAs_available = {256 + 16 , 16};
    SDMA_RANGE_INFO_STC         *SDMAs_available_Ptr;
    CPSS_PORTS_BMP_STC          availableSDMAPorts;/* note : this is BMP of MAC/DMA ... not of physical ports */
    GT_U32                      maxPhyPorts;
    CPSS_DXCH_HW_PP_IMPLEMENT_WA_INIT_FALCON_PORT_DELETE_STC    waInfo;
    GT_U32                      numOfMappedSdma;
    GT_U32                      numCpuSdmas = 1;
    MUX_DMA_STC falcon_3_2_muxed_SDMAs[]  = {{ 68, 66},{ 70, 67},{GT_NA,GT_NA}};
    MUX_DMA_STC falcon_6_4_muxed_SDMAs[]  = {{136,130},{138,131},{140,132},{142,133},{GT_NA,GT_NA}};
    MUX_DMA_STC falcon_12_8_muxed_SDMAs[] = {{272,258},{274,259},{276,260},{278,261},{280,266},{282,267},{284,268},{286,269},{GT_NA,GT_NA}};

    maxPhyPorts = PRV_CPSS_DXCH_MAX_PHY_PORT_NUMBER_MAC(devNum);

    PRV_CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&usedPhyPorts);
    PRV_CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&availableSDMAPorts);

    muxed_SDMAs_Ptr = PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles == 4 ? falcon_12_8_muxed_SDMAs :
                      PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles == 2 ? falcon_6_4_muxed_SDMAs  :
                                                   /* single tile*/        falcon_3_2_muxed_SDMAs  ;

    SDMAs_available_Ptr = PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles == 4 ? &falcon_12_8_SDMAs_available :
                          PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles == 2 ? &falcon_6_4_SDMAs_available  :
                                                       /* single tile*/        &falcon_3_2_SDMAs_available  ;

    /**********************************************************************/
    /********** start by search for CPU SDMA to be used by the WA *********/
    /**********************************************************************/

    /* build BMP of available SDMA ports that can be used */
    for(ii = SDMAs_available_Ptr->firstDma ;
        ii < SDMAs_available_Ptr->firstDma + SDMAs_available_Ptr->numPorts ;
        ii++)
    {
        CPSS_PORTS_BMP_PORT_SET_MAC(&availableSDMAPorts, ii);
    }

    for(ii = 0 ; ii < portMapArraySize; ii++,currPtr++)
    {
        /* state that this physical port is used */
        CPSS_PORTS_BMP_PORT_SET_MAC(&usedPhyPorts, currPtr->physicalPortNumber);

        /* for CPU SDMA , find the highest 'physical port number' and still it's 'queue 7' */
        if(currPtr->mappingType == CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E)
        {
            /* state that this DMA port is used as CPU SDMA */
            /* note : this is BMP of DMAs ... not of physical ports */
            CPSS_PORTS_BMP_PORT_CLEAR_MAC(&availableSDMAPorts, currPtr->interfaceNum);
        }
        else
        {
            for(jj = 0 ; muxed_SDMAs_Ptr[jj].sdmaPort != GT_NA ; jj++)
            {
                if(currPtr->interfaceNum == muxed_SDMAs_Ptr[jj].macPort)
                {
                    /* since the application uses this MAC that is muxed with the SDMA ...
                       it is not free for the WA */
                    CPSS_PORTS_BMP_PORT_CLEAR_MAC(&availableSDMAPorts, muxed_SDMAs_Ptr[jj].sdmaPort);
                    break;
                }
            }
        }
    }

    if(PRV_CPSS_PORTS_BMP_IS_ZERO_MAC(&availableSDMAPorts))
    {
        /* the application uses ALL the SDMAs that the device can offer */
        /* so we will steal the 'queue 7,6' of the 'last one' */
        waInfo.reservedCpuSdmaGlobalQueue[0] = (numCpuSdmas * 8) - 1;
        waInfo.reservedCpuSdmaGlobalQueue[1] = (numCpuSdmas * 8) - 2;
    }
    else
    {
        rc = prvCpssDxChNetIfMultiNetIfNumberGet(devNum, &numOfMappedSdma);
        if (GT_OK != rc)
        {
            return rc;
        }

        /* the highest used SDMAs, not used SDMAs not supporte by WA implementation */
        waInfo.reservedCpuSdmaGlobalQueue[0] = (numOfMappedSdma * 8) - 1;
        waInfo.reservedCpuSdmaGlobalQueue[1] = (numOfMappedSdma * 8) - 2;
        /*  next not supported (yet) by the CPSS
            waInfo.reservedCpuSdmaGlobalQueue[0]         = 0xFFFFFFFF;
            waInfo.reservedCpuSdmaGlobalQueue[1]         = 0xFFFFFFFF;
        */
    }

    if(waInfo.reservedCpuSdmaGlobalQueue[0] == 7)/* single CPU port */
    {
        waInfo.reservedCpuSdmaGlobalQueue[0] = 6;/* the '7' is reserved for all 'from cpu' tests (LUA/enh-UT) */
        waInfo.reservedCpuSdmaGlobalQueue[1] = 5;
    }

    /* saved for later use */
    falconDeleteWaReservedCpuSdmaGlobalQueueArr[0] = waInfo.reservedCpuSdmaGlobalQueue[0];
    falconDeleteWaReservedCpuSdmaGlobalQueueArr[1] = waInfo.reservedCpuSdmaGlobalQueue[1];

    if(0 == CPSS_PORTS_BMP_IS_PORT_SET_MAC(&usedPhyPorts , 61))
    {
        /* check to use 61 as 'default' because it stated as 'special port' that
           should not be used by the application in the FS */
        /* but it is for the L2 traffic that is used by the WA */
        waInfo.reservedPortNum = 61;/* use 61 regardless to the port mode (64/128/512/1024) */
    }
    else
    {
        /**********************************************************************/
        /********** start the search for physical port to be used by the WA ***/
        /**********************************************************************/
        for(ii = maxPhyPorts-1 ; ii ; ii--)
        {
            if(ii == CPSS_CPU_PORT_NUM_CNS ||
               ii == CPSS_NULL_PORT_NUM_CNS)
            {
                /* not valid numbers to use ! */
                continue;
            }

            if(0 == CPSS_PORTS_BMP_IS_PORT_SET_MAC(&usedPhyPorts , ii))
            {
                /* not used */
                break;
            }
        }

        if(ii == 0)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FULL, LOG_ERROR_NO_MSG);
        }

        waInfo.reservedPortNum = ii;
    }

    /*************************************************/
    /********** State the tail drop profile to use ***/
    /*************************************************/
    waInfo.reservedTailDropProfile = 0;


    /**********************************/
    /* call the CPSS to have the info */
    /**********************************/
    rc = cpssDxChHwPpImplementWaInit_FalconPortDelete(CAST_SW_DEVNUM(devNum),&waInfo);
    if (GT_OK != rc)
    {
        return rc;
    }

    return GT_OK;
}

/**
* @internal cpssAppPlatformPhase2Init function
* @endinternal
*
* @brief   Phase2 PP configurations
*
* @param [in] devNum        - CPSS device number
* @param [in] *ppMapPtr     - PP_MAP board profile
* @param [in] *ppProfilePtr - PP profile
*
* @retval GT_OK                  - on success,
* @retval GT_FAIL                - otherwise.
*/
GT_STATUS cpssAppPlatformPhase2Init
(
    IN GT_U8                             devNum,
    IN CPSS_APP_PLATFORM_BOARD_PARAM_PP_MAP_STC *ppMapPtr,
    IN CPSS_APP_PLATFORM_PP_PROFILE_STC *ppProfilePtr
)
{
    GT_STATUS                  rc = GT_BAD_PARAM;
    CPSS_PP_DEVICE_TYPE        devType;
    CPSS_PP_FAMILY_TYPE_ENT    devFamily;
    CPSS_DXCH_CFG_DEV_INFO_STC devInfo;
    GT_HW_DEV_NUM              hwDevNum;
    GT_U32                    *tmpPtr;
    GT_U32                     rxBufSize = RX_BUFF_SIZE_DEF;
    GT_U32                     rxBufAllign = 1;
    GT_U32                     txQue,rxQue;
    GT_U32                     descSize,iii = 0,ii,i = 0,jj;
    GT_BOOL                    txGenMode = GT_FALSE;
    CPSS_DXCH_PP_PHASE2_INIT_INFO_STC cpssPpPhase2;
    CPSS_SYSTEM_RECOVERY_INFO_STC     system_recovery;
    GT_U32                     tmpData;
    GT_BOOL                    txGenModeEnabled = GT_TRUE;
    /* offset used during HW device ID calculation formula */
    GT_U8 appRefHwDevNumOffset = 0x10;
    CPSS_APP_PLATFORM_SDMA_QUEUE_STC falconWATxGenQueues[2] = {{0, 0, 0, 0}, {0, 0, 0, 0}};
    GT_U32 falconDeleteWaReservedCpuSdmaGlobalQueueArr[] = {0, 0};
    GT_U32                     mapArrLen = 0;
    CPSS_DXCH_PORT_MAP_STC     *mapArrPtr = NULL;\
    GT_BOOL                    useFalconPortDeleteWa = GT_FALSE;

    CPSS_APP_PLATFORM_LOG_ENTRY_MAC();

    if (NULL == ppProfilePtr)
    {
        CPSS_APP_PLATFORM_LOG_ERR_MAC("Input profile is NULL.\n");
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ERROR_NO_MSG);
    }

    cpssOsMemSet(&cpssPpPhase2, 0, sizeof(cpssPpPhase2));

    rc = cpssDxChCfgDevInfoGet(devNum, &devInfo);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChCfgDevInfoGet);

    devType = devInfo.genDevInfo.devType;
    devFamily = devInfo.genDevInfo.devFamily;

    if (ppProfilePtr->newDevNum >= 32)
    {
        cpssPpPhase2.newDevNum = devNum;
    }
    else
    {
        cpssPpPhase2.newDevNum = ppProfilePtr->newDevNum;
    }

    cpssPpPhase2.useDoubleAuq = ppProfilePtr->useDoubleAuq;
    cpssPpPhase2.useSecondaryAuq = ppProfilePtr->useSecondaryAuq;
    cpssPpPhase2.auMessageLength = ppProfilePtr->auMessageLength;
    cpssPpPhase2.fuqUseSeparate = ppProfilePtr->fuqUseSeparate;

    cpssPpPhase2.noTraffic2CPU = ppProfilePtr->noTraffic2CPU;
    cpssPpPhase2.useMultiNetIfSdma = ppProfilePtr->useMultiNetIfSdma;
    cpssPpPhase2.netifSdmaPortGroupId = ppProfilePtr->netifSdmaPortGroupId;

    if( appPlatformDbEntryGet("skipTxSdmaGenDefaultCfg", &tmpData) != GT_NO_SUCH )
    {
        txGenModeEnabled = GT_FALSE;
    }

    rc = cpssSystemRecoveryStateGet(&system_recovery);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssSystemRecoveryStateGet);

    if(ppProfilePtr->auDescNum == 0)
    {
        cpssPpPhase2.auqCfg.auDescBlock = 0;
        cpssPpPhase2.auqCfg.auDescBlockSize = 0;
    }
    else
    {
        /* Au block size calc & malloc  */
        cpssDxChHwAuDescSizeGet(devType,&descSize);
        cpssPpPhase2.auqCfg.auDescBlockSize = descSize * ppProfilePtr->auDescNum;
        cpssPpPhase2.auqCfg.auDescBlock =
#if defined(CONFIG_V2_DMA)
            osCacheDmaMallocByDevice(devNum, cpssPpPhase2.auqCfg.auDescBlockSize +
                            descSize);/*allocate space for one message more for alignment purposes
                                      NOTE: we not add it to the size , only to the buffer ! */
#else
            osCacheDmaMalloc(cpssPpPhase2.auqCfg.auDescBlockSize +
                            descSize);/*allocate space for one message more for alignment purposes
                                      NOTE: we not add it to the size , only to the buffer ! */
#endif
        if(cpssPpPhase2.auqCfg.auDescBlock == NULL)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
        }
        if(((GT_UINTPTR)cpssPpPhase2.auqCfg.auDescBlock) % descSize)
        {
            /* add to the size the extra value for alignment , to the actual size
               will be as needed , after the reduction in the cpss code */
            cpssPpPhase2.auqCfg.auDescBlockSize += descSize;
        }
    }

    if(ppProfilePtr->fuDescNum == 0)
    {
        cpssPpPhase2.fuqCfg.auDescBlock = 0;
        cpssPpPhase2.fuqCfg.auDescBlockSize = 0;
    }
    else
    {
        /* Fu block size calc & malloc  */
        cpssDxChHwAuDescSizeGet(devType,&descSize);
        cpssPpPhase2.fuqCfg.auDescBlockSize = descSize * ppProfilePtr->fuDescNum;
        cpssPpPhase2.fuqCfg.auDescBlock =
#if defined(CONFIG_V2_DMA)
            osCacheDmaMallocByDevice(devNum, cpssPpPhase2.fuqCfg.auDescBlockSize +
                            descSize);/*allocate space for one message more for alignment purposes
                                      NOTE: we not add it to the size , only to the buffer ! */
#else
            osCacheDmaMalloc(cpssPpPhase2.fuqCfg.auDescBlockSize +
                            descSize);/*allocate space for one message more for alignment purposes
                                      NOTE: we not add it to the size , only to the buffer ! */
#endif
        if(cpssPpPhase2.fuqCfg.auDescBlock == NULL)
        {
            osCacheDmaFree(cpssPpPhase2.auqCfg.auDescBlock);
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
        }

        if(((GT_UINTPTR)cpssPpPhase2.fuqCfg.auDescBlock) % descSize)
        {
            /* add to the size the extra value for alignment , to the actual size
               will be as needed , after the reduction in the cpss code */
            cpssPpPhase2.fuqCfg.auDescBlockSize += descSize;
        }
    }

    /* if the Falcon WA initialization is not disabled */
    useFalconPortDeleteWa = !DXCH_HWINIT_GLOVAR(prvFalconPortDeleteWa_disabled) &&
                            PRV_CPSS_PP_MAC(ppMapPtr->devNum)->devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E;

    if(useFalconPortDeleteWa)
    {
       if(ppMapPtr->portMap != NULL)
       {
           mapArrLen = ppMapPtr->portMapSize;
           mapArrPtr = ppMapPtr->portMap;
       }
       else
       {
           rc = cpssAppPlatformPpPortMapGet(devNum, &mapArrPtr, &mapArrLen);
           CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssAppPlatformPpPortMapGet);
       }

       /****************************************************************************/
       /* do init for the 'port delete' WA.                                        */
       /* with the same parameters as the cpssDxChPortPhysicalPortMapSet(...) used */
       /****************************************************************************/
       rc = falcon_initPortDelete_WA(devNum, mapArrLen, mapArrPtr, falconDeleteWaReservedCpuSdmaGlobalQueueArr);
       CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, falcon_initPortDelete_WA);
    }


    /* Tx block size calc & malloc  */
    if(ppProfilePtr->useMultiNetIfSdma == GT_FALSE)
    {
        cpssPpPhase2.netIfCfg.rxBufInfo.allocMethod = ppProfilePtr->rxAllocMethod;

        /*since cpssPpPhase2.netIfCfg.rxBufInfo.bufferPercentage is smaller then
        cpssPpPhase2.netIfCfg.rxBufInfo.bufferPercentage we only copy first  CPSS_MAX_RX_QUEUE_CNS*/

        cpssOsMemCpy(cpssPpPhase2.netIfCfg.rxBufInfo.bufferPercentage,
                     ppProfilePtr->rxBufferPercentage, sizeof(cpssPpPhase2.netIfCfg.rxBufInfo.bufferPercentage));

        cpssDxChHwTxDescSizeGet(devType,&descSize);
        cpssPpPhase2.netIfCfg.txDescBlockSize = descSize * ppProfilePtr->txDescNum;
        if (cpssPpPhase2.netIfCfg.txDescBlockSize != 0)
        {
            cpssPpPhase2.netIfCfg.txDescBlock =
#if defined(CONFIG_V2_DMA)
                osCacheDmaMallocByDevice(devNum, cpssPpPhase2.netIfCfg.txDescBlockSize);
#else
                osCacheDmaMalloc(cpssPpPhase2.netIfCfg.txDescBlockSize);
#endif
            if(cpssPpPhase2.netIfCfg.txDescBlock == NULL)
            {
                osCacheDmaFree(cpssPpPhase2.auqCfg.auDescBlock);
                osCacheDmaFree(cpssPpPhase2.fuqCfg.auDescBlock);
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
            }
        }
        if((system_recovery.systemRecoveryState == CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E)||
           ( (system_recovery.systemRecoveryState == CPSS_SYSTEM_RECOVERY_INIT_STATE_E) &&
              (system_recovery.systemRecoveryMode.continuousTx == GT_FALSE)) )
        {
            cpssOsMemSet(cpssPpPhase2.netIfCfg.txDescBlock,0,
                     cpssPpPhase2.netIfCfg.txDescBlockSize);
        }
        /* Rx block size calc & malloc  */
        cpssDxChHwRxDescSizeGet(devType,&descSize);

        cpssPpPhase2.netIfCfg.rxDescBlockSize = descSize * ppProfilePtr->rxDescNum;
        if (cpssPpPhase2.netIfCfg.rxDescBlockSize != 0)
        {
            cpssPpPhase2.netIfCfg.rxDescBlock =
#if defined(CONFIG_V2_DMA)
                osCacheDmaMallocByDevice(devNum, cpssPpPhase2.netIfCfg.rxDescBlockSize);
#else
                osCacheDmaMalloc(cpssPpPhase2.netIfCfg.rxDescBlockSize);
#endif
            if(cpssPpPhase2.netIfCfg.rxDescBlock == NULL)
            {
                osCacheDmaFree(cpssPpPhase2.auqCfg.auDescBlock);
                osCacheDmaFree(cpssPpPhase2.fuqCfg.auDescBlock);
                osCacheDmaFree(cpssPpPhase2.netIfCfg.txDescBlock);
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
            }
        }
        if((system_recovery.systemRecoveryState == CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E)||
           ( (system_recovery.systemRecoveryState == CPSS_SYSTEM_RECOVERY_INIT_STATE_E) &&
              (system_recovery.systemRecoveryMode.continuousRx == GT_FALSE)) )
        {
            cpssOsMemSet(cpssPpPhase2.netIfCfg.rxDescBlock,0,
                     cpssPpPhase2.netIfCfg.rxDescBlockSize);
        }
        /* init the Rx buffer allocation method */
        /* Set the system's Rx buffer size.     */
        if((rxBufSize % rxBufAllign) != 0)
        {
            rxBufSize = (rxBufSize + (rxBufAllign - (rxBufSize % rxBufAllign)));
        }

        if (cpssPpPhase2.netIfCfg.rxBufInfo.allocMethod == CPSS_RX_BUFF_STATIC_ALLOC_E)
        {
            cpssPpPhase2.netIfCfg.rxBufInfo.rxBufSize = rxBufSize;
            cpssPpPhase2.netIfCfg.rxBufInfo.buffData.staticAlloc.rxBufBlockSize = rxBufSize * ppProfilePtr->rxDescNum;

            /* set status of RX buffers - cacheable or not */
            cpssPpPhase2.netIfCfg.rxBufInfo.buffersInCachedMem = ppProfilePtr->rxBuffersInCachedMem;

            if ((rxBufSize * ppProfilePtr->rxDescNum) != 0)
            {
                /* If RX buffers should be cachable - allocate it from regular memory */
                if (GT_TRUE == cpssPpPhase2.netIfCfg.rxBufInfo.buffersInCachedMem)
                {
                    tmpPtr = osMalloc(((rxBufSize * ppProfilePtr->rxDescNum) + rxBufAllign-1));
                }
                else
                {
#if defined(CONFIG_V2_DMA)
                    tmpPtr = osCacheDmaMallocByDevice(devNum, ((rxBufSize * ppProfilePtr->rxDescNum) + rxBufAllign-1));
#else
                    tmpPtr = osCacheDmaMalloc(((rxBufSize * ppProfilePtr->rxDescNum) + rxBufAllign-1));
#endif
                }

                if(tmpPtr == NULL)
                {
                    osCacheDmaFree(cpssPpPhase2.auqCfg.auDescBlock);
                    osCacheDmaFree(cpssPpPhase2.fuqCfg.auDescBlock);
                    osCacheDmaFree(cpssPpPhase2.netIfCfg.txDescBlock);
                    osCacheDmaFree(cpssPpPhase2.netIfCfg.rxDescBlock);
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
                }
            }
            else
            {
                tmpPtr = NULL;
            }

            if((((GT_UINTPTR)tmpPtr) % rxBufAllign) != 0)
            {
                tmpPtr = (GT_U32*)(((GT_UINTPTR)tmpPtr) +
                                   (rxBufAllign - (((GT_UINTPTR)tmpPtr) % rxBufAllign)));
            }
            cpssPpPhase2.netIfCfg.rxBufInfo.buffData.staticAlloc.rxBufBlockPtr = tmpPtr;
        }
        else if (cpssPpPhase2.netIfCfg.rxBufInfo.allocMethod == CPSS_RX_BUFF_NO_ALLOC_E)
        {
            /* do not allocate rx buffers*/
        }
        else
        {
            /* dynamic RX buffer allocation currently is not supported by applicationPlatform*/
            osCacheDmaFree(cpssPpPhase2.auqCfg.auDescBlock);
            osCacheDmaFree(cpssPpPhase2.fuqCfg.auDescBlock);
            osCacheDmaFree(cpssPpPhase2.netIfCfg.txDescBlock);
            osCacheDmaFree(cpssPpPhase2.netIfCfg.rxDescBlock);
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, LOG_ERROR_NO_MSG);
        }
    }

    if(ppProfilePtr->useMultiNetIfSdma)
    {
        CPSS_MULTI_NET_IF_TX_SDMA_QUEUE_STC  *sdmaQueuesConfigPtr = NULL;
        CPSS_MULTI_NET_IF_RX_SDMA_QUEUE_STC  *sdmaRxQueuesConfigPtr = NULL;
        /* Tx block size calc & malloc  */
        cpssDxChHwTxDescSizeGet(devType,&descSize);

        if(useFalconPortDeleteWa)
        {
            /* force SDMA packet generator when the WA is needed in last queue */
            /* NOTE: ignoring "skipTxSdmaGenDefaultCfg" !!!                    */
            /* if this need to be bypassed ... need new 'flag' from appDemoDbEntryGet(...) */
            GT_U32  ii;

            for(ii = 0 ; ii < 2 ; ii ++)
            {
                osPrintf("falcon_initPortDelete_WA : using global SDMA queue [%d] as 'packet generator' \n",
                        falconDeleteWaReservedCpuSdmaGlobalQueueArr[ii]);

                falconWATxGenQueues[ii].sdmaPortNum = falconDeleteWaReservedCpuSdmaGlobalQueueArr[ii] / CPSS_MAX_TX_QUEUE_CNS;
                falconWATxGenQueues[ii].queueNum = falconDeleteWaReservedCpuSdmaGlobalQueueArr[ii] % CPSS_MAX_TX_QUEUE_CNS;

                /* set the second queue with 2 descriptors and (3000+16) buffer size */
                if(ii == 1)/* the queue for the long packets */
                {
                    falconWATxGenQueues[ii].numOfTxBuff = 2*1;
                    falconWATxGenQueues[ii].txBuffSize = (3000+16);
                }
                else
                {
                    falconWATxGenQueues[ii].numOfTxBuff = 256;
                    falconWATxGenQueues[ii].txBuffSize = 144;
                }
            }
        }

        ii=0;
        for(jj = 0; jj < CPSS_MAX_SDMA_CPU_PORTS_CNS; jj++)
        {
#ifdef CHX_FAMILY
            if(CPSS_IS_DXCH_FAMILY_MAC(devFamily))
            {
                PRV_CPSS_DXCH_NETIF_SKIP_NON_USED_NETIF_NUM_MAC(devNum, jj);
            }
#endif
            if(ii > 0 && (!PRV_CPSS_SIP_5_20_CHECK_MAC(devNum)))
            {
                break;
            }

            for(txQue = 0; txQue < CPSS_MAX_TX_QUEUE_CNS; txQue++)
            {
                txGenMode = GT_FALSE;
                sdmaQueuesConfigPtr = &cpssPpPhase2.multiNetIfCfg.txSdmaQueuesConfig[ii][txQue];

                if (txGenModeEnabled)
                {
                    for(i = 0; i < ppProfilePtr->txGenQueueNum; i++)
                    {
                        if((ppProfilePtr->txGenQueueList[i].sdmaPortNum == ii) &&
                           (ppProfilePtr->txGenQueueList[i].queueNum == txQue))
                        {
                            txGenMode = GT_TRUE;
                            break;
                        }
                    }

                    if (useFalconPortDeleteWa)
                    {
                        if (txGenMode == GT_FALSE)
                        {
                            for(iii = 0; iii < 2; iii++)
                            {
                                if((falconWATxGenQueues[iii].sdmaPortNum == ii) &&
                                   (falconWATxGenQueues[iii].queueNum == txQue))
                                {
                                    txGenMode = GT_TRUE;
                                    break;
                                }
                            }
                        }
                    }
                }

                if(txGenMode == GT_FALSE)
                {
                    /* Tx block size calc & malloc  */
                    sdmaQueuesConfigPtr->queueMode = CPSS_TX_SDMA_QUEUE_MODE_NORMAL_E;
                    if(GT_TRUE == ppProfilePtr->isEzbIsXmlLoaded)
                    {
                        sdmaQueuesConfigPtr->numOfTxDesc = (ppProfilePtr->txGenQueueList[(ii*8)+txQue].numOfTxBuff); /*EZB/XML uses multinetCfg which directly takes nufOfTxDesc per queue, txBufferPercentage array stores this instead of percentage(unlike in non-XML mode)*/
                    }
                    else
                    {
                        sdmaQueuesConfigPtr->numOfTxDesc = ppProfilePtr->txDescNum ? (ppProfilePtr->txDescNum / CPSS_MAX_TX_QUEUE_CNS ) : 0; /*125 */
                    }
                    sdmaQueuesConfigPtr->numOfTxBuff = 0; /*not relevant in non traffic generator mode*/
                    sdmaQueuesConfigPtr->buffSize = 0; /*not relevant in non traffic generator mode*/
                    sdmaQueuesConfigPtr->memData.staticAlloc.buffAndDescMemSize = (sdmaQueuesConfigPtr->numOfTxDesc * descSize);
                    if (sdmaQueuesConfigPtr->memData.staticAlloc.buffAndDescMemSize != 0)
                    {
                        sdmaQueuesConfigPtr->memData.staticAlloc.buffAndDescMemPtr =
#if defined(CONFIG_V2_DMA)
                            osCacheDmaMallocByDevice(devNum, sdmaQueuesConfigPtr->memData.staticAlloc.buffAndDescMemSize);
#else
                            osCacheDmaMalloc(sdmaQueuesConfigPtr->memData.staticAlloc.buffAndDescMemSize);
#endif
                        if(sdmaQueuesConfigPtr->memData.staticAlloc.buffAndDescMemPtr == NULL)
                        {
                            osCacheDmaFree(cpssPpPhase2.auqCfg.auDescBlock);
                            osCacheDmaFree(cpssPpPhase2.fuqCfg.auDescBlock);
                            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
                        }
                    }

                    sdmaQueuesConfigPtr->buffAndDescAllocMethod = CPSS_TX_BUFF_STATIC_ALLOC_E;

                    if((system_recovery.systemRecoveryState == CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E)||
                       ((system_recovery.systemRecoveryState == CPSS_SYSTEM_RECOVERY_INIT_STATE_E) &&
                       (system_recovery.systemRecoveryMode.continuousTx == GT_FALSE)) )
                    {
                        cpssOsMemSet(sdmaQueuesConfigPtr->memData.staticAlloc.buffAndDescMemPtr,0,
                                     sdmaQueuesConfigPtr->memData.staticAlloc.buffAndDescMemSize);
                    }
                }
                else
                {/* Generator mode */
                    sdmaQueuesConfigPtr->queueMode = CPSS_TX_SDMA_QUEUE_MODE_PACKET_GENERATOR_E;
                    sdmaQueuesConfigPtr->buffAndDescAllocMethod = CPSS_TX_BUFF_STATIC_ALLOC_E;
                    if (i < ppProfilePtr->txGenQueueNum)
                    {
                        /* Predefined txGenQueues in profile */
                        sdmaQueuesConfigPtr->numOfTxBuff = ppProfilePtr->txGenQueueList[i].numOfTxBuff;
                        sdmaQueuesConfigPtr->buffSize = ppProfilePtr->txGenQueueList[i].txBuffSize;
                    }
                    else
                    {
                        /* Falcon initPortDelete_WA additional txGenQueue */
                        sdmaQueuesConfigPtr->numOfTxBuff = falconWATxGenQueues[iii].numOfTxBuff;
                        sdmaQueuesConfigPtr->buffSize = falconWATxGenQueues[iii].txBuffSize;
                    }
                    sdmaQueuesConfigPtr->numOfTxDesc = sdmaQueuesConfigPtr->numOfTxBuff;
                    sdmaQueuesConfigPtr->memData.staticAlloc.buffAndDescMemSize =
                        (sdmaQueuesConfigPtr->numOfTxDesc + 1) * (descSize + sdmaQueuesConfigPtr->buffSize);
                    sdmaQueuesConfigPtr->memData.staticAlloc.buffAndDescMemPtr =
#if defined(CONFIG_V2_DMA)
                        osCacheDmaMallocByDevice(devNum, sdmaQueuesConfigPtr->memData.staticAlloc.buffAndDescMemSize);
#else
                        osCacheDmaMalloc(sdmaQueuesConfigPtr->memData.staticAlloc.buffAndDescMemSize);
#endif
                    if(sdmaQueuesConfigPtr->memData.staticAlloc.buffAndDescMemPtr == NULL)
                    {
                        osCacheDmaFree(cpssPpPhase2.auqCfg.auDescBlock);
                        osCacheDmaFree(cpssPpPhase2.fuqCfg.auDescBlock);
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
                    }
                }
            }
            ii++;
        }

        /* Rx block size calc & malloc  */
        cpssDxChHwRxDescSizeGet(devType,&descSize);
        ii=0;
        for(jj = 0; jj < CPSS_MAX_SDMA_CPU_PORTS_CNS; jj++)
        {
#ifdef CHX_FAMILY
           if(CPSS_IS_DXCH_FAMILY_MAC(devFamily))
            {
                PRV_CPSS_DXCH_NETIF_SKIP_NON_USED_NETIF_NUM_MAC(devNum, jj);
            }
#endif

            if( ii > 0 && (!PRV_CPSS_SIP_5_20_CHECK_MAC(devNum)) )
            {
                continue;
            }

            for(rxQue = 0; rxQue < CPSS_MAX_RX_QUEUE_CNS; rxQue++)
            {
                sdmaRxQueuesConfigPtr = &cpssPpPhase2.multiNetIfCfg.rxSdmaQueuesConfig[ii][rxQue];
                sdmaRxQueuesConfigPtr->buffAllocMethod =  ppProfilePtr->rxAllocMethod;
                sdmaRxQueuesConfigPtr->buffersInCachedMem = ppProfilePtr->rxBuffersInCachedMem;
                if(GT_TRUE == ppProfilePtr->isEzbIsXmlLoaded)
                {
                    sdmaRxQueuesConfigPtr->numOfRxDesc = ppProfilePtr->rxBufferPercentage[(ii*CPSS_MAX_RX_QUEUE_CNS)+rxQue]; /*EZB/XML uses multinetCfg which directly takes nufOfRxDesc per queue, rxBufferPercentage array stores this instead of percentage(unlike in non-XML mode)*/
                }
                else
                {
                    sdmaRxQueuesConfigPtr->numOfRxDesc = ppProfilePtr->rxDescNum * ppProfilePtr->rxBufferPercentage[(ii*CPSS_MAX_RX_QUEUE_CNS)+rxQue] / 100;
                }
                sdmaRxQueuesConfigPtr->descMemSize = sdmaRxQueuesConfigPtr->numOfRxDesc * descSize;
#if defined(CONFIG_V2_DMA)
                sdmaRxQueuesConfigPtr->descMemPtr = osCacheDmaMallocByDevice(devNum, sdmaRxQueuesConfigPtr->descMemSize);
#else
                sdmaRxQueuesConfigPtr->descMemPtr = osCacheDmaMalloc(sdmaRxQueuesConfigPtr->descMemSize);
#endif
                if(sdmaRxQueuesConfigPtr->descMemPtr == NULL)
                {
                    osCacheDmaFree(cpssPpPhase2.auqCfg.auDescBlock);
                    osCacheDmaFree(cpssPpPhase2.fuqCfg.auDescBlock);
                    for(txQue = 0; txQue < CPSS_MAX_TX_QUEUE_CNS; txQue++)
                    {
                        sdmaQueuesConfigPtr = &cpssPpPhase2.multiNetIfCfg.txSdmaQueuesConfig[ii][txQue];
                        if(sdmaQueuesConfigPtr->memData.staticAlloc.buffAndDescMemSize != 0)
                        {
                            osCacheDmaFree(sdmaQueuesConfigPtr->memData.staticAlloc.buffAndDescMemPtr);
                        }
                    }
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
                }
                /* init the Rx buffer allocation method */
                /* Set the system's Rx buffer size.     */
                if((rxBufSize % rxBufAllign) != 0)
                {
                    rxBufSize = (rxBufSize + (rxBufAllign - (rxBufSize % rxBufAllign)));
                }

                sdmaRxQueuesConfigPtr->buffHeaderOffset = ppProfilePtr->rxHeaderOffset; /* give the same offset to all queues*/
                sdmaRxQueuesConfigPtr->buffSize = rxBufSize;
                if(GT_TRUE == ppProfilePtr->isEzbIsXmlLoaded)
                {
                    sdmaRxQueuesConfigPtr->numOfRxBuff =  ppProfilePtr->rxBufferPercentage[(ii*CPSS_MAX_RX_QUEUE_CNS)+rxQue];
                }
                else
                {
                    sdmaRxQueuesConfigPtr->numOfRxBuff =  ppProfilePtr->rxDescNum * ppProfilePtr->rxBufferPercentage[(ii*CPSS_MAX_RX_QUEUE_CNS)+rxQue] / 100; /* by default the number of buffers equel the number of descriptors*/
                }
                sdmaRxQueuesConfigPtr->memData.staticAlloc.buffMemSize = (rxBufSize * sdmaRxQueuesConfigPtr->numOfRxBuff);
                if (sdmaRxQueuesConfigPtr->memData.staticAlloc.buffMemSize != 0)
                {
                    /* If RX buffers should be cachable - allocate it from regular memory */
                    if (GT_TRUE == sdmaRxQueuesConfigPtr->buffersInCachedMem)
                    {
                        tmpPtr = osMalloc((sdmaRxQueuesConfigPtr->memData.staticAlloc.buffMemSize + rxBufAllign-1));
                    }
                    else
                    {
#if defined(CONFIG_V2_DMA)
                        tmpPtr = osCacheDmaMallocByDevice(devNum, (sdmaRxQueuesConfigPtr->memData.staticAlloc.buffMemSize + rxBufAllign-1));
#else
                        tmpPtr = osCacheDmaMalloc((sdmaRxQueuesConfigPtr->memData.staticAlloc.buffMemSize + rxBufAllign-1));
#endif
                    }

                    if(tmpPtr == NULL)
                    {
                        osCacheDmaFree(cpssPpPhase2.auqCfg.auDescBlock);
                        osCacheDmaFree(cpssPpPhase2.fuqCfg.auDescBlock);
                        for(txQue = 0; txQue < CPSS_MAX_TX_QUEUE_CNS; txQue++)
                        {
                            sdmaQueuesConfigPtr = &cpssPpPhase2.multiNetIfCfg.txSdmaQueuesConfig[ii][txQue];
                            if(sdmaQueuesConfigPtr->memData.staticAlloc.buffAndDescMemSize != 0)
                            {
                                osCacheDmaFree(sdmaQueuesConfigPtr->memData.staticAlloc.buffAndDescMemPtr);
                            }
                        }
                        osCacheDmaFree(sdmaRxQueuesConfigPtr->descMemPtr);
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
                    }
                }
                else
                {
                    tmpPtr = NULL;
                }

                if((((GT_UINTPTR)tmpPtr) % rxBufAllign) != 0)
                {
                    tmpPtr = (GT_U32*)(((GT_UINTPTR)tmpPtr) +
                                       (rxBufAllign - (((GT_UINTPTR)tmpPtr) % rxBufAllign)));
                }
                sdmaRxQueuesConfigPtr->memData.staticAlloc.buffMemPtr = tmpPtr;

                if((system_recovery.systemRecoveryState == CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E)||
                   ( (system_recovery.systemRecoveryState == CPSS_SYSTEM_RECOVERY_INIT_STATE_E) &&
                     (system_recovery.systemRecoveryMode.continuousRx == GT_FALSE)) )
                {
                    cpssOsMemSet(sdmaRxQueuesConfigPtr->memData.staticAlloc.buffMemPtr,0,
                         sdmaRxQueuesConfigPtr->memData.staticAlloc.buffMemSize);
                }
            }
            ii++;
        }
    }

    rc = cpssDxChHwPpPhase2Init(devNum, &cpssPpPhase2);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChHwPpPhase2Init);

    cpssOsMemCpy(&(PRV_APP_REF_PP_UTILS_VAR(cpssAppPlatformPpPhase2Db)[devNum]), &cpssPpPhase2, sizeof(cpssPpPhase2));

    /* In order to configure HW device ID different from SW device ID the following */
    /* logic is used: HW_device_ID = (SW_device_ID + appDemoHwDevNumOffset) modulo 32 */
    /* (this insures different HW and SW device IDs since the HW device ID is 5 */
    /* bits length). */
    if (PRV_CPSS_DXCH_PP_HW_INFO_E_ARCH_ENABLED_MAC(devNum))
    {
        hwDevNum = ((devNum + appRefHwDevNumOffset) & 0x3FF);
    }
    else
    {
        hwDevNum = ((devNum + appRefHwDevNumOffset) & 0x1F);
    }

    /* set HWdevNum related values */
    rc = prvCpssDxChHwDevNumChange(devNum,hwDevNum);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, prvCpssDxChHwDevNumChange);

    return rc;
}

/*******************************************************************************
 * CPSS modules initialization routines
 ******************************************************************************/

static GT_STATUS prvPhyLibInit
(
    IN GT_U8 devNum
)
{
    GT_STATUS rc = GT_OK;

    rc = cpssDxChPhyPortSmiInit(devNum);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPhyPortSmiInit);

    return rc;
}

#define CHEETAH_CPU_PORT_PROFILE                   CPSS_PORT_RX_FC_PROFILE_1_E
#define CHEETAH_NET_GE_PORT_PROFILE                CPSS_PORT_RX_FC_PROFILE_2_E
#define CHEETAH_NET_10GE_PORT_PROFILE              CPSS_PORT_RX_FC_PROFILE_3_E
#define CHEETAH_CASCADING_PORT_PROFILE             CPSS_PORT_RX_FC_PROFILE_4_E

#define CHEETAH_GE_PORT_XON_DEFAULT                14 /* 28 Xon buffs per port   */
#define CHEETAH_GE_PORT_XOFF_DEFAULT               35 /* 70 Xoff buffs per port  */
#define CHEETAH_GE_PORT_RX_BUFF_LIMIT_DEFAULT      25 /* 100 buffers per port    */

#define CHEETAH_CPU_PORT_XON_DEFAULT               14 /* 28 Xon buffs per port   */
#define CHEETAH_CPU_PORT_XOFF_DEFAULT              35 /* 70 Xoff buffs per port  */
#define CHEETAH_CPU_PORT_RX_BUFF_LIMIT_DEFAULT     25 /* 100 buffers for CPU port */

#define CHEETAH_XG_PORT_XON_DEFAULT                25 /* 50 Xon buffs per port   */
#define CHEETAH_XG_PORT_XOFF_DEFAULT               85 /* 170 Xoff buffs per port */
#define CHEETAH_XG_PORT_RX_BUFF_LIMIT_DEFAULT      56 /* 224 buffers per port    */

static GT_STATUS prvPortLibInit
(
    IN GT_U8                             devNum,
    IN CPSS_APP_PLATFORM_PP_PROFILE_STC *ppProfilePtr
)
{
    GT_STATUS rc = GT_OK;
    CPSS_DXCH_PORT_HOL_FC_ENT modeFcHol = CPSS_DXCH_PORT_FC_E;

    rc = cpssDxChPortStatInit(devNum);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPortStatInit);

    /* Enable HOL system mode for revision 3 in DxCh2, DxCh3, XCAT. */
    if(ppProfilePtr->flowControlDisable)
    {
        modeFcHol = CPSS_DXCH_PORT_HOL_E;
    }

    if(ppProfilePtr->modeFcHol)
    {
        rc = cpssDxChPortFcHolSysModeSet(devNum, modeFcHol);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPortFcHolSysModeSet);
    }

    rc = cpssDxChPortTxInit(devNum);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPortTxInit);

    return rc;
}

static GT_STATUS dxChBrgFdbInit
(
    IN GT_U8 dev
)
{
    GT_STATUS                   rc = GT_OK;
    GT_HW_DEV_NUM               hwDev;    /* HW device number */
    CPSS_MAC_HASH_FUNC_MODE_ENT hashMode;
    GT_U32                      maxLookupLen;

    rc = cpssDxChBrgFdbInit(dev);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChBrgFdbInit);

    /* sip6 not supports any more the xor/crc , supports only the multi-hash */
    hashMode = (!PRV_CPSS_SIP_6_CHECK_MAC(dev)) ?
                CPSS_MAC_HASH_FUNC_XOR_E :
                CPSS_MAC_HASH_FUNC_CRC_MULTI_HASH_E;
    maxLookupLen = 4;

    /* Set lookUp mode and lookup length. */
    rc = cpssDxChBrgFdbHashModeSet(dev, hashMode);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChBrgFdbHashModeSet);

    /* NOTE : in sip6 calling this API in multi-hash mode is irrelevant
       as we not modify the value in the HW , and keep it 0 .

       the  API will fail if maxLookupLen != fdbHashParams.numOfBanks
    */
    if(!PRV_CPSS_SIP_6_CHECK_MAC(dev))
    {
        rc = cpssDxChBrgFdbMaxLookupLenSet(dev, maxLookupLen);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChBrgFdbMaxLookupLenSet);
    }

    /******************************/
    /* do specific cheetah coding */
    /******************************/

    /* the trunk entries registered according to : macEntryPtr->dstInterface.hwDevNum
       that is to support the "renumbering" feature , but the next configuration
       should not effect the behavior on other systems that not use a
       renumbering ..
    */
    /* age trunk entries on a device that registered from all devices
       since we registered the trunk entries on device macEntryPtr->dstInterface.hwDevNum
       that may differ from "own device"
       (and auto learn set it on "own device" */
    /* Set Action Active Device Mask and Action Active Device. This is needed
       in order to enable aging only on own device.  */
    /*
       BTW : the multicast entries are registered on "own device" (implicitly by the CPSS)
        (so will require renumber for systems that needs renumber)
    */

    rc = cpssDxChCfgHwDevNumGet(dev, &hwDev);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChCfgHwDevNumGet);

    rc = cpssDxChBrgFdbActionActiveDevSet(dev, hwDev, 0x1F);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChBrgFdbActionActiveDevSet);

    rc = cpssDxChBrgFdbAgeOutAllDevOnTrunkEnable(dev, GT_TRUE);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChBrgFdbAgeOutAllDevOnTrunkEnable);

    rc = cpssDxChBrgFdbAgeOutAllDevOnNonTrunkEnable(dev, GT_FALSE);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChBrgFdbAgeOutAllDevOnNonTrunkEnable);

    return rc;
}

static GT_STATUS prvBridgeLibInit
(
    IN GT_U8                             devNum,
    IN CPSS_APP_PLATFORM_PP_PROFILE_STC *ppProfilePtr
)
{
    GT_STATUS rc = GT_OK;
    GT_U32  numOfPhysicalPorts;
    GT_U32  numOfEports;
    GT_U32  portNum;
    GT_U32  i;
    GT_HW_DEV_NUM hwDevNum;
    GT_U32        stpEntry[CPSS_DXCH_STG_ENTRY_SIZE_CNS];
    GT_BOOL                             isCpu;
    /* allow processing of AA messages */
    /*
    applicationPlatformSysConfig.supportAaMessage = GT_TRUE;
    */

    /* Init VLAN */
    rc = cpssDxChBrgVlanInit(devNum);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChBrgVlanInit);

    /** STP **/
    rc = cpssDxChBrgStpInit(devNum);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChBrgStpInit);

    rc = dxChBrgFdbInit(devNum);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, dxChBrgFdbInit);

    rc = cpssDxChBrgMcInit(devNum);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChBrgMcInit);

    /* set first entry in STP like default entry */
    cpssOsMemSet(stpEntry, 0, sizeof(stpEntry));
    rc = cpssDxChBrgStpEntryWrite(devNum, 0, stpEntry);
    if( GT_OK != rc)
    {
        /* the device not support STP !!! --> it's ok ,we have those ... */
        CPSS_APP_PLATFORM_LOG_ERR_MAC("cpssDxChBrgStpEntryWrite : device[%d] not supported \n",devNum);
        rc = GT_OK;
    }

    if (! PRV_CPSS_SIP_5_CHECK_MAC(devNum))
    {
        return GT_OK;
    }

    if(PRV_CPSS_SIP_5_10_CHECK_MAC(devNum))
    {
        /* TBD: FE HA-3259 fix and removed from CPSS.
           Allow to the CPU to get the original vlan tag as payload after
           the DSA tag , so the info is not changed. */
        rc = cpssDxChBrgVlanForceNewDsaToCpuEnableSet(devNum, GT_TRUE);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChBrgVlanForceNewDsaToCpuEnableSet);

        /* set the command of 'SA static moved' to be 'forward' because this command
           applied also on non security breach event ! */
        rc = cpssDxChBrgSecurBreachEventPacketCommandSet(devNum,
            CPSS_BRG_SECUR_BREACH_EVENTS_MOVED_STATIC_E,
            CPSS_PACKET_CMD_FORWARD_E);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChBrgSecurBreachEventPacketCommandSet);
    }

    rc = cpssDxChCfgHwDevNumGet(devNum, &hwDevNum);
    if (rc != GT_OK)
        return rc;

    /* Enable configuration of drop for ARP MAC SA mismatch due to check per port */
    /* Loop on the first 256 (num of physical ports , and CPU port (63)) entries
       of the table */
    numOfPhysicalPorts = PRV_CPSS_DXCH_MAX_PHY_PORT_NUMBER_MAC(devNum);

    for(portNum=0; portNum < numOfPhysicalPorts; portNum++)
    {
        /* ARP MAC SA mismatch check per port configuration enabling */
        rc = cpssDxChBrgGenPortArpMacSaMismatchDropEnable(devNum,
                                                          portNum,
                                                          GT_TRUE);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChBrgGenPortArpMacSaMismatchDropEnable);

        /* this code can be restored after link up/ lind down EGF WA is implemented */
        if(PRV_CPSS_SIP_5_20_CHECK_MAC(devNum) && !cpssAppPlatformIsPortMgrPort(devNum))
        {
            /* set the EGF to filter traffic to ports that are 'link down'.
               state that all ports are currently 'link down' (except for 'CPU PORT')

               see function: sip5_20_linkChange , which handles runtime 'link change' event.
            */
            rc = cpssDxChPortPhysicalPortMapIsCpuGet(devNum, portNum, &isCpu);
            CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPortPhysicalPortMapIsCpuGet);
            rc = cpssDxChBrgEgrFltPortLinkEnableSet(devNum,
                                                    portNum,
                                                    (isCpu==GT_FALSE) ?
                                                    CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_FORCE_LINK_DOWN_E: /* FIlter non CPU port*/
                                                    CPSS_DXCH_BRG_EGR_FLT_PORT_LINK_STATUS_FORCE_LINK_UP_E  /*don't filter*/);
            CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChBrgEgrFltPortLinkEnableSet);
        }
    }

    /* Port Isolation is enabled if all three configurations are enabled:
       In the global TxQ registers, AND
       In the eVLAN egress entry, AND
       In the ePort entry

       For legacy purpose loop on all ePort and Trigger L2 & L3 Port
       Isolation filter for all ePorts */

    numOfEports = PRV_CPSS_DXCH_MAX_PORT_NUMBER_MAC(devNum);

    for(portNum=0; portNum < numOfEports; portNum++)
    {
        rc = cpssDxChNstPortIsolationModeSet(devNum,
                                             portNum,
                                             CPSS_DXCH_NST_PORT_ISOLATION_ALL_ENABLE_E);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChNstPortIsolationModeSet);

        /* for legacy : enable per eport <Egress eVLAN Filtering Enable>
           because Until today there was no enable bit, egress VLAN filtering is
           always performed, subject to the global <BridgedUcEgressFilterEn>. */
        rc = cpssDxChBrgEgrFltVlanPortFilteringEnableSet(devNum,
                                                          portNum,
                                                          GT_TRUE);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChBrgEgrFltVlanPortFilteringEnableSet);
    }

    /* Flow Control Initializations */
    for(portNum = 0; portNum < numOfPhysicalPorts; portNum++)
    {
        CPSS_APPLICATION_PORT_SKIP_CHECK(devNum, portNum);

        if(prvCpssDxChPortRemotePortCheck(devNum,portNum))
        {
            /* There is no supported way of setting MAC SA Lsb on remote ports */
            continue;
        }
        rc = cpssDxChPortMacSaLsbSet(devNum, portNum, (GT_U8)portNum);
        if (GT_OK != rc)
           CPSS_APP_PLATFORM_LOG_ERR_MAC("cpssDxChPortMacSaLsbSet rc=%d", rc);
    }

    if(ppProfilePtr->policerMruSupported)
    {
        /* PLR MRU : needed for bobk that hold default different then our tests expect */
        /* NOTE: for bobk it is not good value for packets > (10K/8) bytes */
        for(i = CPSS_DXCH_POLICER_STAGE_INGRESS_0_E;
            i <= CPSS_DXCH_POLICER_STAGE_EGRESS_E ;
            i++)
        {
            rc = cpssDxCh3PolicerMruSet(devNum,i,_10K);
            if (GT_OK != rc)
               CPSS_APP_PLATFORM_LOG_ERR_MAC("cpssDxCh3PolicerMruSet rc=%d", rc);
        }
    }

    if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        /* init Exact Match DB */
        rc = prvCpssDxChExactMatchDbInit(devNum);
        if (GT_OK != rc)
           CPSS_APP_PLATFORM_LOG_ERR_MAC("prvCpssDxChExactMatchDbInit rc=%d", rc);
    }

    return rc;
}

static GT_STATUS prvNetIfLibInit
(
    IN GT_U8                             devNum,
    IN CPSS_APP_PLATFORM_PP_PROFILE_STC *ppProfilePtr
)
{
    GT_STATUS rc = GT_OK;
    GT_U32 i;

    if (ppProfilePtr->cpuPortMode == CPSS_NET_CPU_PORT_MODE_SDMA_E)
    {
        rc = cpssDxChNetIfInit(devNum);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChNetIfInit);

        /* When CPSS_RX_BUFF_NO_ALLOC_E method is used application is responsible for
         * RX buffer allocation and attachment to descriptors.*/
    }
    else if (ppProfilePtr->cpuPortMode == CPSS_NET_CPU_PORT_MODE_MII_E)
    {
        CPSS_DXCH_NETIF_MII_INIT_STC miiInit;
        miiInit.numOfTxDesc = ppProfilePtr->miiTxDescNum;
        miiInit.txInternalBufBlockSize = ppProfilePtr->miiTxBufBlockSize;
        miiInit.txInternalBufBlockPtr = cpssOsCacheDmaMalloc(miiInit.txInternalBufBlockSize);
        if (miiInit.txInternalBufBlockPtr == NULL && miiInit.txInternalBufBlockSize != 0)
        {
            CPSS_APP_PLATFORM_LOG_ERR_MAC("cpssOsCacheDmaMalloc failed\r\n");
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }
        for (i = 0; i < CPSS_MAX_RX_QUEUE_CNS; i++)
        {
            miiInit.bufferPercentage[i] = ppProfilePtr->miiRxBufferPercentage[i];
        }
        miiInit.rxBufSize = ppProfilePtr->miiRxBufSize;
        miiInit.headerOffset = ppProfilePtr->miiRxHeaderOffset;
        miiInit.rxBufBlockSize = ppProfilePtr->miiRxBufBlockSize;
        miiInit.rxBufBlockPtr = cpssOsCacheDmaMalloc(miiInit.rxBufBlockSize);
        if (miiInit.rxBufBlockPtr == NULL && miiInit.rxBufBlockSize != 0)
        {
            CPSS_APP_PLATFORM_LOG_ERR_MAC("cpssOsCacheDmaMalloc failed\r\n");
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }
        if (miiInit.rxBufBlockPtr != NULL)
        {
            rc = cpssDxChNetIfMiiInit(devNum,&miiInit);
            CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChNetIfMiiInit);
        }
        else
        {
            rc = GT_OK;
        }
    }

    return rc;
}

static GT_STATUS prvMirrorLibInit
(
    IN GT_U8                             devNum,
    IN CPSS_APP_PLATFORM_PP_PROFILE_STC *ppProfilePtr
)
{
    GT_STATUS rc = GT_OK;
    GT_HW_DEV_NUM     hwDev; /* HW device number */
    CPSS_DXCH_MIRROR_ANALYZER_INTERFACE_STC   interface;

    /* The following mirroring settings are needed because several RDE tests */
    /* assume that that default HW values of analyzers is port 0 SW device ID 0. */
    if( 0 == devNum )
    {
        rc = cpssDxChCfgHwDevNumGet(0, &hwDev);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChCfgHwDevNumGet);
        interface.interface.type = CPSS_INTERFACE_PORT_E;
        interface.interface.devPort.hwDevNum = hwDev;
        interface.interface.devPort.portNum = ppProfilePtr->mirrorAnalyzerPortNum;

        rc = cpssDxChMirrorAnalyzerInterfaceSet(devNum, 0, &interface);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChMirrorAnalyzerInterfaceSet);

        rc = cpssDxChMirrorAnalyzerInterfaceSet(devNum, 1, &interface);
        if (GT_OK != rc)
           CPSS_APP_PLATFORM_LOG_ERR_MAC("cpssDxChMirrorAnalyzerInterfaceSet ret=%d\r\n", rc);
    }
    return rc;
}

static GT_STATUS prvPclLibInit
(
    IN GT_U8 devNum
)
{
    GT_STATUS rc = GT_OK;

    rc = cpssDxChPclInit(devNum);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPclInit);

    rc = cpssDxChPclIngressPolicyEnable(devNum, GT_TRUE);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPclIngressPolicyEnable);

    return rc;
}



#define GROUP_0             0
#define GROUP_1             1
#define GROUP_2             2
#define GROUP_3             3
#define GROUP_4             4

#define HIT_NUM_0           0
#define HIT_NUM_1           1
#define HIT_NUM_2           2
#define HIT_NUM_3           3
#define END_OF_LIST_MAC     0xFFFFFFFF

typedef struct{
    GT_U32  floorNum;/*if END_OF_LIST_MAC --> not valid */
    GT_U32  bankIndex;
    GT_U32  hitNum;
}BANK_PARTITION_INFO_STC;

/* save TCAM 'Sections' (base+size) for TTI and IPCL/EPLC */
GT_VOID applicationPlatformDxChTcamSectionsSave(GT_VOID)
{
     PRV_APP_REF_PP_UTILS_VAR(save_applicationPlatformTcamTtiHit0RuleBaseIndexOffset)  = PRV_APP_REF_PP_UTILS_VAR(applicationPlatformTcamTtiHit0RuleBaseIndexOffset);
     PRV_APP_REF_PP_UTILS_VAR(save_applicationPlatformTcamTtiHit1RuleBaseIndexOffset)  = PRV_APP_REF_PP_UTILS_VAR(applicationPlatformTcamTtiHit1RuleBaseIndexOffset);
     PRV_APP_REF_PP_UTILS_VAR(save_applicationPlatformTcamTtiHit2RuleBaseIndexOffset)  = PRV_APP_REF_PP_UTILS_VAR(applicationPlatformTcamTtiHit2RuleBaseIndexOffset);
     PRV_APP_REF_PP_UTILS_VAR(save_applicationPlatformTcamTtiHit3RuleBaseIndexOffset)  = PRV_APP_REF_PP_UTILS_VAR(applicationPlatformTcamTtiHit3RuleBaseIndexOffset);

     PRV_APP_REF_PP_UTILS_VAR(save_applicationPlatformTcamTtiHit0MaxNum)               = PRV_APP_REF_PP_UTILS_VAR(applicationPlatformTcamTtiHit0MaxNum);
     PRV_APP_REF_PP_UTILS_VAR(save_applicationPlatformTcamTtiHit1MaxNum)               = PRV_APP_REF_PP_UTILS_VAR(applicationPlatformTcamTtiHit1MaxNum);
     PRV_APP_REF_PP_UTILS_VAR(save_applicationPlatformTcamTtiHit2MaxNum)               = PRV_APP_REF_PP_UTILS_VAR(applicationPlatformTcamTtiHit2MaxNum);
     PRV_APP_REF_PP_UTILS_VAR(save_applicationPlatformTcamTtiHit3MaxNum)               = PRV_APP_REF_PP_UTILS_VAR(applicationPlatformTcamTtiHit3MaxNum);

     PRV_APP_REF_PP_UTILS_VAR(save_applicationPlatformTcamIpcl0RuleBaseIndexOffset)    = PRV_APP_REF_PP_UTILS_VAR(applicationPlatformTcamIpcl0RuleBaseIndexOffset);
     PRV_APP_REF_PP_UTILS_VAR(save_applicationPlatformTcamIpcl1RuleBaseIndexOffset)    = PRV_APP_REF_PP_UTILS_VAR(applicationPlatformTcamIpcl1RuleBaseIndexOffset);
     PRV_APP_REF_PP_UTILS_VAR(save_applicationPlatformTcamIpcl2RuleBaseIndexOffset)    = PRV_APP_REF_PP_UTILS_VAR(applicationPlatformTcamIpcl2RuleBaseIndexOffset);
     PRV_APP_REF_PP_UTILS_VAR(save_applicationPlatformTcamEpclRuleBaseIndexOffset)     = PRV_APP_REF_PP_UTILS_VAR(applicationPlatformTcamEpclRuleBaseIndexOffset);

     PRV_APP_REF_PP_UTILS_VAR(save_applicationPlatformTcamIpcl0MaxNum)                 = PRV_APP_REF_PP_UTILS_VAR(applicationPlatformTcamIpcl0MaxNum);
     PRV_APP_REF_PP_UTILS_VAR(save_applicationPlatformTcamIpcl1MaxNum)                 = PRV_APP_REF_PP_UTILS_VAR(applicationPlatformTcamIpcl1MaxNum);
     PRV_APP_REF_PP_UTILS_VAR(save_applicationPlatformTcamIpcl2MaxNum)                 = PRV_APP_REF_PP_UTILS_VAR(applicationPlatformTcamIpcl2MaxNum);
     PRV_APP_REF_PP_UTILS_VAR(save_applicationPlatformTcamEpclMaxNum )                 = PRV_APP_REF_PP_UTILS_VAR(applicationPlatformTcamEpclMaxNum);

    return;
}

static GT_STATUS prvTcamLibInit
(
    IN  GT_U8                       devNum,
    IN CPSS_APP_PLATFORM_PP_PROFILE_STC  *ppProfilePtr
)
{
    GT_STATUS rc;
    GT_U32 value;
    CPSS_DXCH_TCAM_BLOCK_INFO_STC          floorInfoArr[CPSS_DXCH_TCAM_MAX_NUM_BLOCKS_CNS];
    GT_U32 baseFloorForTtiLookup0 = 0;
    GT_U32 baseFloorForTtiLookup1 = 0;
    GT_U32 baseFloorForTtiLookup2 = 0;
    GT_U32 baseFloorForTtiLookup3 = 0;
    GT_U32 tcamFloorsNum;
    GT_U32  ii,jj,index;
    GT_U32 tcamEntriesNum;     /* number of entries for TTI in TCAM */
    GT_U32 tcamFloorEntriesNum;/* number of entries for TTI in TCAM floor */
    GT_BOOL ttiBasesfromArray = GT_FALSE;
    /* indication of bc2 b0 */
    GT_U32 isBobcat2B0 = 0;

    static const BANK_PARTITION_INFO_STC  bc2A0_ttiLookupArr[] = {
                    {6,0                                 ,HIT_NUM_0},
                    {6,CPSS_DXCH_TCAM_MAX_NUM_BANKS_CNS/2,HIT_NUM_0},

                    {7,0                                 ,HIT_NUM_0},
                    {7,CPSS_DXCH_TCAM_MAX_NUM_BANKS_CNS/2,HIT_NUM_0},

                    {8,0                                 ,HIT_NUM_0},
                    {8,CPSS_DXCH_TCAM_MAX_NUM_BANKS_CNS/2,HIT_NUM_0},

                    {9,0                                 ,HIT_NUM_0},
                    {9,CPSS_DXCH_TCAM_MAX_NUM_BANKS_CNS/2,HIT_NUM_0},

                    {10,0                                 ,HIT_NUM_1},
                    {10,CPSS_DXCH_TCAM_MAX_NUM_BANKS_CNS/2,HIT_NUM_1},

                    {11,0                                 ,HIT_NUM_1},
                    {11,CPSS_DXCH_TCAM_MAX_NUM_BANKS_CNS/2,HIT_NUM_1},

                    {END_OF_LIST_MAC,END_OF_LIST_MAC,END_OF_LIST_MAC}
                    };

    static const BANK_PARTITION_INFO_STC  bc2B0_ttiLookupArr[] = {
                    {6,0                                 ,HIT_NUM_0},
                    {6,CPSS_DXCH_TCAM_MAX_NUM_BANKS_CNS/2,HIT_NUM_0},

                    {7,0                                 ,HIT_NUM_0},
                    {7,CPSS_DXCH_TCAM_MAX_NUM_BANKS_CNS/2,HIT_NUM_0},

                    {8,0                                 ,HIT_NUM_3},
                    {8,CPSS_DXCH_TCAM_MAX_NUM_BANKS_CNS/2,HIT_NUM_3},

                    {9,0                                 ,HIT_NUM_2},
                    {9,CPSS_DXCH_TCAM_MAX_NUM_BANKS_CNS/2,HIT_NUM_2},

                    {10,0                                 ,HIT_NUM_1},
                    {10,CPSS_DXCH_TCAM_MAX_NUM_BANKS_CNS/2,HIT_NUM_1},

                    {11,0                                 ,HIT_NUM_1},
                    {11,CPSS_DXCH_TCAM_MAX_NUM_BANKS_CNS/2,HIT_NUM_1},

                    {END_OF_LIST_MAC,END_OF_LIST_MAC,END_OF_LIST_MAC}
                    };

    static const BANK_PARTITION_INFO_STC  bobk_ttiLookupArr[] = {
                    {3,0                                 ,HIT_NUM_0},
                    {3,CPSS_DXCH_TCAM_MAX_NUM_BANKS_CNS/2,HIT_NUM_0},

                    {4,0                                 ,HIT_NUM_2},
                    {4,CPSS_DXCH_TCAM_MAX_NUM_BANKS_CNS/2,HIT_NUM_3},

                    {5,0                                 ,HIT_NUM_1},
                    {5,CPSS_DXCH_TCAM_MAX_NUM_BANKS_CNS/2,HIT_NUM_1},

                    {END_OF_LIST_MAC,END_OF_LIST_MAC,END_OF_LIST_MAC}
                    };

    static const BANK_PARTITION_INFO_STC  hawk_ttiLookupArr[] = {
                    { 8,0                                 ,HIT_NUM_0},
                    { 8,CPSS_DXCH_TCAM_MAX_NUM_BANKS_CNS/2,HIT_NUM_0},

                    { 9,0                                 ,HIT_NUM_0},
                    { 9,CPSS_DXCH_TCAM_MAX_NUM_BANKS_CNS/2,HIT_NUM_0},

                    {10,0                                 ,HIT_NUM_0},
                    {10,CPSS_DXCH_TCAM_MAX_NUM_BANKS_CNS/2,HIT_NUM_0},

                    {11,0                                 ,HIT_NUM_0},
                    {11,CPSS_DXCH_TCAM_MAX_NUM_BANKS_CNS/2,HIT_NUM_0},

                    {12,0                                 ,HIT_NUM_3},
                    {12,CPSS_DXCH_TCAM_MAX_NUM_BANKS_CNS/2,HIT_NUM_3},

                    {13,0                                 ,HIT_NUM_2},
                    {13,CPSS_DXCH_TCAM_MAX_NUM_BANKS_CNS/2,HIT_NUM_2},

                    {14,0                                 ,HIT_NUM_1},
                    {14,CPSS_DXCH_TCAM_MAX_NUM_BANKS_CNS/2,HIT_NUM_1},

                    {15,0                                 ,HIT_NUM_1},
                    {15,CPSS_DXCH_TCAM_MAX_NUM_BANKS_CNS/2,HIT_NUM_1},

                    {END_OF_LIST_MAC,END_OF_LIST_MAC,END_OF_LIST_MAC}
                    };

    static const BANK_PARTITION_INFO_STC  hawk_12_floors_ttiLookupArr[] = {
                    { 6,0                                 ,HIT_NUM_0},
                    { 6,CPSS_DXCH_TCAM_MAX_NUM_BANKS_CNS/2,HIT_NUM_0},

                    { 7,0                                 ,HIT_NUM_0},
                    { 7,CPSS_DXCH_TCAM_MAX_NUM_BANKS_CNS/2,HIT_NUM_0},

                    {8,0                                 ,HIT_NUM_0},
                    {8,CPSS_DXCH_TCAM_MAX_NUM_BANKS_CNS/2,HIT_NUM_0},

                    {9,0                                 ,HIT_NUM_3},
                    {9,CPSS_DXCH_TCAM_MAX_NUM_BANKS_CNS/2,HIT_NUM_3},

                    {10,0                                 ,HIT_NUM_2},
                    {10,CPSS_DXCH_TCAM_MAX_NUM_BANKS_CNS/2,HIT_NUM_2},

                    {11,0                                 ,HIT_NUM_1},
                    {11,CPSS_DXCH_TCAM_MAX_NUM_BANKS_CNS/2,HIT_NUM_1},

                    {END_OF_LIST_MAC,END_OF_LIST_MAC,END_OF_LIST_MAC}
                    };

    const BANK_PARTITION_INFO_STC *ttiLookupInfoPtr = NULL;
    GT_BOOL ttiLookup3FromMidFloor = GT_FALSE;
    GT_BOOL ttiLookup0NonStandard = GT_FALSE;

    (void)ppProfilePtr;
    tcamFloorEntriesNum =
        CPSS_DXCH_TCAM_MAX_NUM_BANKS_CNS * CPSS_DXCH_TCAM_MAX_NUM_RULES_PER_BANKS_CNS;

    rc = cpssDxChCfgTableNumEntriesGet(
        devNum, CPSS_DXCH_CFG_TABLE_TTI_TCAM_E, &tcamEntriesNum);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    /* value must be a multiple of floor size */
    if (((tcamEntriesNum % tcamFloorEntriesNum) != 0) ||
        (tcamEntriesNum == 0) ||
        (tcamEntriesNum > (CPSS_DXCH_TCAM_MAX_NUM_FLOORS_CNS * tcamFloorEntriesNum)))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    tcamFloorsNum = tcamEntriesNum / tcamFloorEntriesNum;

    if(PRV_CPSS_SIP_5_10_CHECK_MAC(devNum))
    {
        isBobcat2B0 = 1;

        if(PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
        {
            ttiLookupInfoPtr = bobk_ttiLookupArr;
            if (tcamFloorsNum >= 16)
            {
                /* AC5P */
                ttiLookupInfoPtr = hawk_ttiLookupArr;
                ttiBasesfromArray = GT_TRUE;
            }
            else if(tcamFloorsNum >= 12)
            {
                /* AC5P/Aldrin3 with 12 floors */
                ttiLookupInfoPtr = hawk_12_floors_ttiLookupArr;
                ttiBasesfromArray = GT_TRUE;
            }
        }
        else
        {
            if (tcamFloorsNum >= 12)
            {
                ttiLookupInfoPtr = bc2B0_ttiLookupArr;
            }
            else
            {
                /* Drake with 6-floor TCAM */
                ttiLookupInfoPtr = bobk_ttiLookupArr;
            }
        }
    }
    else
    {
        ttiLookupInfoPtr = bc2A0_ttiLookupArr;
    }

    /* init TCAM - Divide the TCAM into 2 groups:
       ingress policy 0, ingress policy 1, ingress policy 2 and egress policy belong to group 0; using floors 0-5.
       client tunnel termination belong to group 1; using floor 6-11.
       applicationPlatformDbEntryAdd can change the division such that 6 will be replaced by a different value. */
    rc = cpssDxChTcamPortGroupClientGroupSet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,CPSS_DXCH_TCAM_IPCL_0_E,GROUP_0,GT_TRUE);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChTcamPortGroupClientGroupSet);

    rc = cpssDxChTcamPortGroupClientGroupSet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,CPSS_DXCH_TCAM_IPCL_1_E,GROUP_0,GT_TRUE);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChTcamPortGroupClientGroupSet);

    if (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.pcl.iPcl0Bypass == GT_FALSE)
    {
        rc = cpssDxChTcamPortGroupClientGroupSet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,CPSS_DXCH_TCAM_IPCL_2_E,GROUP_0,GT_TRUE);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChTcamPortGroupClientGroupSet);
    }

    rc = cpssDxChTcamPortGroupClientGroupSet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,CPSS_DXCH_TCAM_EPCL_E,GROUP_0,GT_TRUE);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChTcamPortGroupClientGroupSet);

    if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_IRONMAN_E) /* must bind to group 0 as this is the single group in the device */
    {
        rc = cpssDxChTcamPortGroupClientGroupSet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,CPSS_DXCH_TCAM_TTI_E,GROUP_0,GT_TRUE);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChTcamPortGroupClientGroupSet);
    }
    else
    {
        rc = cpssDxChTcamPortGroupClientGroupSet(devNum,CPSS_PORT_GROUP_UNAWARE_MODE_CNS,CPSS_DXCH_TCAM_TTI_E,GROUP_1,GT_TRUE);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChTcamPortGroupClientGroupSet);
    }

    if(tcamFloorsNum == 3 && PRV_CPSS_SIP_5_15_CHECK_MAC(devNum))
    {
        /* floor 0 for PCL clients
           All clients are connected to hit num 0 */
        for (ii = 0; ii < CPSS_DXCH_TCAM_MAX_NUM_BLOCKS_CNS; ii++)
        {
            floorInfoArr[ii].group  = GROUP_0;
            floorInfoArr[ii].hitNum = HIT_NUM_0;
        }

        rc = cpssDxChTcamIndexRangeHitNumAndGroupSet(devNum, 0,floorInfoArr);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChTcamIndexRangeHitNumAndGroupSet);

        /* floor 1: used by TTI_0 and TTI_1 */
        for (ii = 0; ii < CPSS_DXCH_TCAM_MAX_NUM_BLOCKS_CNS; ii++)
        {
            floorInfoArr[ii].group  = GROUP_1;
            floorInfoArr[ii].hitNum = (ii < 3) ? HIT_NUM_0 : HIT_NUM_1;
        }

        rc = cpssDxChTcamIndexRangeHitNumAndGroupSet(devNum, 1,floorInfoArr);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChTcamIndexRangeHitNumAndGroupSet);

        /* floor 2: used by TTI_2 and TTI_3 */
        for (ii = 0; ii < CPSS_DXCH_TCAM_MAX_NUM_BLOCKS_CNS; ii++)
        {
            floorInfoArr[ii].group  = GROUP_1;
            floorInfoArr[ii].hitNum = (ii < 3) ? HIT_NUM_2 : HIT_NUM_3;
        }

        rc = cpssDxChTcamIndexRangeHitNumAndGroupSet(devNum, 2,floorInfoArr);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChTcamIndexRangeHitNumAndGroupSet);

        PRV_APP_REF_PP_UTILS_VAR(applicationPlatformTcamTtiHit0RuleBaseIndexOffset) = tcamFloorEntriesNum;
        PRV_APP_REF_PP_UTILS_VAR(applicationPlatformTcamTtiHit1RuleBaseIndexOffset) = PRV_APP_REF_PP_UTILS_VAR(applicationPlatformTcamTtiHit0RuleBaseIndexOffset) + 6;
        PRV_APP_REF_PP_UTILS_VAR(applicationPlatformTcamTtiHit2RuleBaseIndexOffset) = tcamFloorEntriesNum * 2;
        PRV_APP_REF_PP_UTILS_VAR(applicationPlatformTcamTtiHit3RuleBaseIndexOffset) = PRV_APP_REF_PP_UTILS_VAR(applicationPlatformTcamTtiHit2RuleBaseIndexOffset) + 6;
        PRV_APP_REF_PP_UTILS_VAR(applicationPlatformTcamTtiHit0MaxNum) =
        PRV_APP_REF_PP_UTILS_VAR(applicationPlatformTcamTtiHit1MaxNum) =
        PRV_APP_REF_PP_UTILS_VAR(applicationPlatformTcamTtiHit2MaxNum) =
        PRV_APP_REF_PP_UTILS_VAR(applicationPlatformTcamTtiHit3MaxNum) = CPSS_DXCH_TCAM_MAX_NUM_RULES_PER_HALF_FLOOR_CNS;
    }
    else
    {
        if (ttiBasesfromArray == GT_FALSE)
        {
            baseFloorForTtiLookup0 = ((tcamFloorsNum + 1) / 2);
            baseFloorForTtiLookup1 = (tcamFloorsNum - 1);
            if (baseFloorForTtiLookup1 > 10)
            {
                baseFloorForTtiLookup1 = 10;
            }

            if(isBobcat2B0)
            {
                if ((tcamFloorsNum/2) < 4) /* support bobk and some bc2 flavors */
                {
                    /* we can not provide floor for each lookup TTI 0,1,2,3 */
                    switch (tcamFloorsNum/2)
                    {
                        case 2:
                            /* lookup 0,2 on floor 0 */
                            /* lookup 1,3 on floor 1 */
                            baseFloorForTtiLookup1 = baseFloorForTtiLookup0 + 1;
                            baseFloorForTtiLookup2 = baseFloorForTtiLookup0;
                            baseFloorForTtiLookup3 = baseFloorForTtiLookup1;
                            break;
                        case 1:
                            /* lookup 0,1,2,3 on floor 0 */
                            baseFloorForTtiLookup1 = baseFloorForTtiLookup0;
                            baseFloorForTtiLookup2 = baseFloorForTtiLookup0;
                            baseFloorForTtiLookup3 = baseFloorForTtiLookup0;
                            break;
                        case 3:
                            /* lookup 0   on floor 0 */
                            /* lookup 2,3 on floor 1 */
                            /* lookup 1   on floor 2 */
                            baseFloorForTtiLookup1 = baseFloorForTtiLookup0 + 2;
                            baseFloorForTtiLookup2 = baseFloorForTtiLookup0 + 1;
                            baseFloorForTtiLookup3 = baseFloorForTtiLookup2;
                            ttiLookup3FromMidFloor = GT_TRUE;/* lookup 3 from mid floor */
                            break;
                        default:  /*0*/
                            /* should not happen*/
                            break;
                    }
                }
                else
                {
                    baseFloorForTtiLookup2 = (baseFloorForTtiLookup1 - 1);
                    baseFloorForTtiLookup3 = (baseFloorForTtiLookup1 - 2);
                }

                PRV_APP_REF_PP_UTILS_VAR(applicationPlatformTcamTtiHit2RuleBaseIndexOffset) = (baseFloorForTtiLookup2 * tcamFloorEntriesNum);
                PRV_APP_REF_PP_UTILS_VAR(applicationPlatformTcamTtiHit3RuleBaseIndexOffset) = (baseFloorForTtiLookup3 * tcamFloorEntriesNum);
                if(ttiLookup3FromMidFloor == GT_TRUE)
                {
                    PRV_APP_REF_PP_UTILS_VAR(applicationPlatformTcamTtiHit3RuleBaseIndexOffset) += (CPSS_DXCH_TCAM_MAX_NUM_BANKS_CNS / 2);/*6*/
                }

            }

            PRV_APP_REF_PP_UTILS_VAR(applicationPlatformTcamTtiHit0RuleBaseIndexOffset) = (baseFloorForTtiLookup0 * tcamFloorEntriesNum);
            PRV_APP_REF_PP_UTILS_VAR(applicationPlatformTcamTtiHit1RuleBaseIndexOffset) = (baseFloorForTtiLookup1 * tcamFloorEntriesNum);
        }
        else /*ttiBasesfromArray == GT_TRUE*/
        {
            /* "invalid" stamps */
            PRV_APP_REF_PP_UTILS_VAR(applicationPlatformTcamTtiHit0RuleBaseIndexOffset) = 0xFFFFFFFF;
            PRV_APP_REF_PP_UTILS_VAR(applicationPlatformTcamTtiHit1RuleBaseIndexOffset) = 0xFFFFFFFF;
            PRV_APP_REF_PP_UTILS_VAR(applicationPlatformTcamTtiHit2RuleBaseIndexOffset) = 0xFFFFFFFF;
            PRV_APP_REF_PP_UTILS_VAR(applicationPlatformTcamTtiHit3RuleBaseIndexOffset) = 0xFFFFFFFF;

            /* positive "not found" stasmps */
            baseFloorForTtiLookup0 = 0xFFFF;
            baseFloorForTtiLookup1 = 0xFFFF;
            baseFloorForTtiLookup2 = 0xFFFF;
            baseFloorForTtiLookup3 = 0xFFFF;

            for (ii = 0;(ttiLookupInfoPtr[ii].floorNum != END_OF_LIST_MAC); ii++)
            {
                switch (ttiLookupInfoPtr[ii].hitNum)
                {
                    case HIT_NUM_0:
                        if (baseFloorForTtiLookup0 > ttiLookupInfoPtr[ii].floorNum)
                        {
                            baseFloorForTtiLookup0 = ttiLookupInfoPtr[ii].floorNum;
                        }
                        break;
                    case HIT_NUM_1:
                        if (baseFloorForTtiLookup1 > ttiLookupInfoPtr[ii].floorNum)
                        {
                            baseFloorForTtiLookup1 = ttiLookupInfoPtr[ii].floorNum;
                        }
                        break;
                    case HIT_NUM_2:
                        if (baseFloorForTtiLookup2 > ttiLookupInfoPtr[ii].floorNum)
                        {
                            baseFloorForTtiLookup2 = ttiLookupInfoPtr[ii].floorNum;
                        }
                        break;
                    case HIT_NUM_3:
                        if (baseFloorForTtiLookup3 > ttiLookupInfoPtr[ii].floorNum)
                        {
                            baseFloorForTtiLookup3 = ttiLookupInfoPtr[ii].floorNum;
                        }
                        break;
                    default: break;
                }
            }

            if (baseFloorForTtiLookup0 < 0xFFFF)
            {
                PRV_APP_REF_PP_UTILS_VAR(applicationPlatformTcamTtiHit0RuleBaseIndexOffset) = (baseFloorForTtiLookup0 * tcamFloorEntriesNum);
            }
            if (baseFloorForTtiLookup1 < 0xFFFF)
            {
                PRV_APP_REF_PP_UTILS_VAR(applicationPlatformTcamTtiHit1RuleBaseIndexOffset) = (baseFloorForTtiLookup1 * tcamFloorEntriesNum);
            }
            if (baseFloorForTtiLookup2 < 0xFFFF)
            {
                PRV_APP_REF_PP_UTILS_VAR(applicationPlatformTcamTtiHit2RuleBaseIndexOffset) = (baseFloorForTtiLookup2 * tcamFloorEntriesNum);
            }
            if (baseFloorForTtiLookup3 < 0xFFFF)
            {
                PRV_APP_REF_PP_UTILS_VAR(applicationPlatformTcamTtiHit3RuleBaseIndexOffset) = (baseFloorForTtiLookup3 * tcamFloorEntriesNum);
            }
        }

        if (appPlatformDbEntryGet("firstTtiTcamEntry", &value) == GT_OK)
        {
            /* value must be a multiple of floor size */
            if (((value % (CPSS_DXCH_TCAM_MAX_NUM_BANKS_CNS * CPSS_DXCH_TCAM_MAX_NUM_RULES_PER_BANKS_CNS)) == 0) &&
                (value > 0) &&
                (value < CPSS_DXCH_TCAM_MAX_NUM_FLOORS_CNS * CPSS_DXCH_TCAM_MAX_NUM_BANKS_CNS * CPSS_DXCH_TCAM_MAX_NUM_RULES_PER_BANKS_CNS))
            {
                baseFloorForTtiLookup0 = (value / (CPSS_DXCH_TCAM_MAX_NUM_BANKS_CNS * CPSS_DXCH_TCAM_MAX_NUM_RULES_PER_BANKS_CNS));
                PRV_APP_REF_PP_UTILS_VAR(applicationPlatformTcamTtiHit0RuleBaseIndexOffset) = value;

                if(baseFloorForTtiLookup0 != ttiLookupInfoPtr[0].floorNum /* 6 in bc2 , 3 in bobk */)
                {
                    ttiLookup0NonStandard = GT_TRUE;
                }
            }
            else
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
            }
        }

        for (ii = 0; ii < CPSS_DXCH_TCAM_MAX_NUM_BLOCKS_CNS; ii++)
        {
            floorInfoArr[ii].group  = GROUP_0;
            floorInfoArr[ii].hitNum = HIT_NUM_0;
        }

        /* PCL : All clients are connected to hit num 0 */
        for (value = 0; value < baseFloorForTtiLookup0; value++){
            rc = cpssDxChTcamIndexRangeHitNumAndGroupSet(devNum,value,floorInfoArr);
            CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChTcamIndexRangeHitNumAndGroupSet);
        }

        if (PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_IRONMAN_E)
        {
            for (ii = 0; ii < CPSS_DXCH_TCAM_MAX_NUM_BLOCKS_CNS; ii++)
            {
                floorInfoArr[ii].group = GROUP_1;
            }
        }

        PRV_APP_REF_PP_UTILS_VAR(applicationPlatformTcamTtiHit0MaxNum) = 0;
        PRV_APP_REF_PP_UTILS_VAR(applicationPlatformTcamTtiHit1MaxNum) = 0;
        PRV_APP_REF_PP_UTILS_VAR(applicationPlatformTcamTtiHit2MaxNum) = 0;
        PRV_APP_REF_PP_UTILS_VAR(applicationPlatformTcamTtiHit3MaxNum) = 0;

        /* TTI : All clients are connected to hit num 0..3 */
        for(ii = 0 ; ttiLookupInfoPtr[ii].floorNum != END_OF_LIST_MAC ; ii += 2)
        {
            value = ttiLookupInfoPtr[ii+0].floorNum;

            if(value >= tcamFloorsNum)
            {
                /* ignore */
                continue;
            }
            if (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.tcam.numBanksForHitNumGranularity == 2)
            {
                floorInfoArr[0].hitNum=ttiLookupInfoPtr[ii+0].hitNum;
                floorInfoArr[1].hitNum=ttiLookupInfoPtr[ii+1].hitNum;
            }
            else
            {
                floorInfoArr[0].hitNum=ttiLookupInfoPtr[ii+0].hitNum;
                floorInfoArr[1].hitNum=ttiLookupInfoPtr[ii+0].hitNum;
                floorInfoArr[2].hitNum=ttiLookupInfoPtr[ii+0].hitNum;
                floorInfoArr[3].hitNum=ttiLookupInfoPtr[ii+1].hitNum;
                floorInfoArr[4].hitNum=ttiLookupInfoPtr[ii+1].hitNum;
                floorInfoArr[5].hitNum=ttiLookupInfoPtr[ii+1].hitNum;
            }

            /* support case that the baseFloorForTtiLookup0 was set by 'applicationPlatformDbEntryGet'
               to value > 6 */
            if(baseFloorForTtiLookup0 > ttiLookupInfoPtr[0].floorNum)
            {
                if(baseFloorForTtiLookup0 > value/*current floor*/)
                {
                    /* this floor is part of the 'PCL' */
                    continue;
                }
                else if(baseFloorForTtiLookup0 == value)
                {
                    /* use it for lookup 0 */
                    for (jj=0; jj<CPSS_DXCH_TCAM_MAX_NUM_BLOCKS_CNS; jj++) {
                        floorInfoArr[jj].hitNum = HIT_NUM_0;
                    }
                }
            }

            /* calculate the number of entries that each lookup can use */
            for (jj = 0 ; jj < 2 ; jj++)
            {
                if (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.tcam.numBanksForHitNumGranularity == 2)
                {
                    index = jj;
                }
                else
                {
                    index = 3*jj;
                }
                if (floorInfoArr[index].hitNum == HIT_NUM_0)
                {
                    /* another half floor for lookup 0 */
                    PRV_APP_REF_PP_UTILS_VAR(applicationPlatformTcamTtiHit0MaxNum) += tcamFloorEntriesNum / 2;
                }
                else
                if(floorInfoArr[index].hitNum == HIT_NUM_1)
                {
                    /* another half floor for lookup 1 */
                    PRV_APP_REF_PP_UTILS_VAR(applicationPlatformTcamTtiHit1MaxNum) += tcamFloorEntriesNum / 2;
                }
                else
                if(floorInfoArr[index].hitNum == HIT_NUM_2)
                {
                    /* another half floor for lookup 2 */
                    PRV_APP_REF_PP_UTILS_VAR(applicationPlatformTcamTtiHit2MaxNum) += tcamFloorEntriesNum / 2;
                }
                else
                if(floorInfoArr[index].hitNum == HIT_NUM_3)
                {
                    /* another half floor for lookup 3 */
                    PRV_APP_REF_PP_UTILS_VAR(applicationPlatformTcamTtiHit3MaxNum) += tcamFloorEntriesNum / 2;
                }
            }

            rc = cpssDxChTcamIndexRangeHitNumAndGroupSet(devNum,value,floorInfoArr);
            CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChTcamIndexRangeHitNumAndGroupSet);
        }

        /* support case that the baseFloorForTtiLookup0 was set by 'applicationPlatformDbEntryGet'
           to value != 6 */
        if(ttiLookup0NonStandard == GT_TRUE)
        {
            /* dedicated floors for lookup 0 */
            for (ii = 0; ii < CPSS_DXCH_TCAM_MAX_NUM_BLOCKS_CNS; ii++)
            {
                floorInfoArr[ii].hitNum = HIT_NUM_0;
            }

            for(ii = baseFloorForTtiLookup0 ; ii < ttiLookupInfoPtr[0].floorNum ; ii ++)
            {
                value = ii;

                if(value >= tcamFloorsNum)
                {
                    /* ignore */
                    continue;
                }

                /* calculate the number of entries that each lookup can use */
                /* another floor for lookup 0 */
                PRV_APP_REF_PP_UTILS_VAR(applicationPlatformTcamTtiHit0MaxNum) += tcamFloorEntriesNum;

                rc = cpssDxChTcamIndexRangeHitNumAndGroupSet(devNum,value,floorInfoArr);
                CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChTcamIndexRangeHitNumAndGroupSet);
            }
        }
    }

    /* IPCL/EPCL get what the TTI not use */
    PRV_APP_REF_PP_UTILS_VAR(applicationPlatformTcamIpcl0MaxNum) = (tcamFloorsNum * tcamFloorEntriesNum) - /* full tcam size */
            (PRV_APP_REF_PP_UTILS_VAR(applicationPlatformTcamTtiHit0MaxNum) +   /* used by TTI hit 0*/
             PRV_APP_REF_PP_UTILS_VAR(applicationPlatformTcamTtiHit1MaxNum) +   /* used by TTI hit 1*/
             PRV_APP_REF_PP_UTILS_VAR(applicationPlatformTcamTtiHit2MaxNum) +   /* used by TTI hit 2*/
             PRV_APP_REF_PP_UTILS_VAR(applicationPlatformTcamTtiHit3MaxNum));   /* used by TTI hit 3*/
    PRV_APP_REF_PP_UTILS_VAR(applicationPlatformTcamIpcl1MaxNum) = PRV_APP_REF_PP_UTILS_VAR(applicationPlatformTcamIpcl0MaxNum);
    if (PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.pcl.iPcl0Bypass == GT_FALSE)
    {
        PRV_APP_REF_PP_UTILS_VAR(applicationPlatformTcamIpcl2MaxNum) = PRV_APP_REF_PP_UTILS_VAR(applicationPlatformTcamIpcl0MaxNum);
    }
    PRV_APP_REF_PP_UTILS_VAR(applicationPlatformTcamEpclMaxNum)  = PRV_APP_REF_PP_UTILS_VAR(applicationPlatformTcamIpcl0MaxNum);

    /* save TCAM 'Sections' (base+size) for TTI and IPCL/EPLC */
    applicationPlatformDxChTcamSectionsSave();
    return GT_OK;
}

static GT_STATUS prvPolicerLibInit
(
    IN GT_U8 devNum
)
{
    GT_STATUS rc = GT_OK;

    rc = cpssDxChPolicerInit(devNum);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPolicerInit);

    return rc;
}

static GT_STATUS prvIngressPolicerDisable
(
    IN GT_U8 devNum
)
{
    GT_STATUS rc = GT_OK;

    /* check if Ingress stage #1 exists */
    if (PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.featureInfo.iplrSecondStageSupported
        == GT_TRUE)
    {
        /* Disable Policer Metering on Ingress stage #1 */
        rc = cpssDxCh3PolicerMeteringEnableSet(devNum,
                                               CPSS_DXCH_POLICER_STAGE_INGRESS_1_E,
                                               GT_FALSE);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxCh3PolicerMeteringEnableSet);

        /* Disable Policer Counting on Ingress stage #1 */
        rc = cpssDxChPolicerCountingModeSet(devNum,
                                            CPSS_DXCH_POLICER_STAGE_INGRESS_1_E,
                                            CPSS_DXCH_POLICER_COUNTING_DISABLE_E);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPolicerCountingModeSet);
   }

   return rc;
}

static GT_STATUS prvTrunkLibInit
(
    IN GT_U8                             devNum,
    IN CPSS_APP_PLATFORM_PP_PROFILE_STC *ppProfilePtr
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    currMaxTrunks = 0;
    GT_U32    maxTrunksNeeded = 0;
    GT_BOOL   flexWithFixedSize = GT_FALSE;
    GT_U32    fixedNumOfMembersInTrunks = 0;
    GT_U32    value;
    CPSS_DXCH_TRUNK_MEMBERS_MODE_ENT membersMode = ppProfilePtr->trunkMembersMode;

    if (PRV_CPSS_SIP_7_CHECK_MAC(devNum))
    {
        /*sip7: unlike sip5 the trunk ECMP is dedicated and not need to use for trunks 1/2 the L2 ecmp table */
        maxTrunksNeeded = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.parametericTables.numEntriesTrunkEcmp;
        maxTrunksNeeded = (maxTrunksNeeded / PRV_CPSS_TRUNK_8_MAX_NUM_OF_MEMBERS_CNS) - 1;
    }
    else
    if (PRV_CPSS_SIP_5_CHECK_MAC(devNum))
    {
        /* at this stage the PRV_CPSS_DXCH_PP_MAC(dev)->fineTuning.tableSize.trunksNum
            hold the number '4K' ... which is NOT what we need !

            we need '8 members' trunks that take 1/2 of L2ECMP table.
        */
        maxTrunksNeeded = PRV_CPSS_DXCH_PP_MAC(devNum)->bridge.l2EcmpNumMembers;
        maxTrunksNeeded = ((maxTrunksNeeded / 2) / PRV_CPSS_TRUNK_8_MAX_NUM_OF_MEMBERS_CNS) - 1;
    }
    else
    {
        maxTrunksNeeded = PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.trunksNum;
    }

    if(ppProfilePtr->numOfTrunks > maxTrunksNeeded)
    {
        currMaxTrunks = maxTrunksNeeded;
    }
    else
    {
        currMaxTrunks = ppProfilePtr->numOfTrunks;
    }

    /* for falcon port mode 512 ( 512 trunks) and port 1024 (256 trunks) support maxTrunks according to table entries number */
    if((PRV_CPSS_SIP_6_CHECK_MAC(devNum)) &&
       (currMaxTrunks > PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.trunksNum))
    {
        currMaxTrunks =  PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.trunksNum;
    }

    if((appPlatformDbEntryGet("full_flex_trunks", &value) == GT_OK) && (value != 0))
    {
        osPrintf("appDemoDbEntryGet : set each trunk to be with own max num of members \n" ,
                value);
        membersMode = CPSS_DXCH_TRUNK_MEMBERS_MODE_FLEX_E;
        flexWithFixedSize = GT_FALSE;
        fixedNumOfMembersInTrunks = 0;
    }

    rc = cpssDxChTrunkInit(devNum, currMaxTrunks, membersMode);
    while(rc == GT_OUT_OF_RANGE && currMaxTrunks)
    {
        currMaxTrunks--;
        rc = cpssDxChTrunkInit(devNum, currMaxTrunks, membersMode);
    }

    if((rc != GT_OK) && (currMaxTrunks != ppProfilePtr->numOfTrunks))
    {
        CPSS_APP_PLATFORM_LOG_ERR_MAC("cpssDxChTrunkInit: device[%d] not support [%d] trunks\n",devNum,currMaxTrunks);
    }

    if(rc == GT_OK &&
       membersMode == CPSS_DXCH_TRUNK_MEMBERS_MODE_FLEX_E &&
       flexWithFixedSize == GT_TRUE)
    {
        /* set all our trunks to be with the same max size (the value that we want) */
        /* set the 'hybrid mode' of 'flex' and 'fixed size' */
        rc = cpssDxChTrunkFlexInfoSet(devNum,0xFFFF/*hybrid mode indication*/,
                                      0, fixedNumOfMembersInTrunks);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChTrunkFlexInfoSet);
    }

    return rc;
}

static GT_STATUS prvDxCh2Ch3IpLibInit
(
    IN GT_U8                             devNum,
    IN CPSS_APP_PLATFORM_PP_PROFILE_STC *ppProfilePtr
)
{
    GT_STATUS                                       rc = GT_OK;
    CPSS_DXCH_IP_TCAM_LPM_MANGER_CAPCITY_CFG_STC    cpssLpmDbCapacity;
    CPSS_DXCH_IP_TCAM_LPM_MANGER_INDEX_RANGE_STC    cpssLpmDbRange;
    CPSS_DXCH_IP_LPM_MEMORY_CONFIG_UNT              lpmMemoryConfig;
    CPSS_DXCH_IP_UC_ROUTE_ENTRY_STC                 ucRouteEntry;
    CPSS_DXCH_IP_MC_ROUTE_ENTRY_STC                 mcRouteEntry;
    CPSS_DXCH_IP_LPM_SHADOW_TYPE_ENT                shadowType;
    CPSS_IP_PROTOCOL_STACK_ENT                      protocolStack;
    CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT          defUcLttEntry;
    CPSS_DXCH_IP_LTT_ENTRY_STC                      defMcLttEntry;
    CPSS_DXCH_IP_LPM_VR_CONFIG_STC                  vrConfigInfo;
    GT_U32                                          lpmDbId = 0;
    CPSS_DXCH_CFG_DEV_INFO_STC                      devInfo;
    CPSS_PP_FAMILY_TYPE_ENT                         devFamily;
    /*CPSS_DXCH_LPM_RAM_CONFIG_STC                    lpmRamMemoryBlocksCfg;*/

    rc = cpssDxChCfgDevInfoGet(devNum, &devInfo);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChCfgDevInfoGet);

    devFamily = devInfo.genDevInfo.devFamily;

    switch (devFamily)
    {
        case CPSS_PP_FAMILY_DXCH_ALDRIN_E:
        case CPSS_PP_FAMILY_DXCH_ALDRIN2_E:
        case CPSS_PP_FAMILY_DXCH_AC3X_E:
            shadowType = CPSS_DXCH_IP_LPM_RAM_SIP5_SHADOW_E;
            break;
        case CPSS_PP_FAMILY_DXCH_FALCON_E:
        case CPSS_PP_FAMILY_DXCH_AC5X_E:
        case CPSS_PP_FAMILY_DXCH_AC5P_E:
        case CPSS_PP_FAMILY_DXCH_IRONMAN_E:
        case CPSS_PP_FAMILY_DXCH_HARRIER_E:
            shadowType = CPSS_DXCH_IP_LPM_RAM_SIP6_SHADOW_E;
            break;
        case CPSS_PP_FAMILY_DXCH_AAS_E:
            shadowType = CPSS_DXCH_IP_LPM_RAM_SIP7_SHADOW_E;
            break;
        case CPSS_PP_FAMILY_DXCH_AC5_E:
            shadowType = CPSS_DXCH_IP_LPM_TCAM_XCAT_SHADOW_E;
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_IMPLEMENTED, LOG_ERROR_NO_MSG);
    }

    /* init default UC and MC entries */
    cpssOsMemSet(&defUcLttEntry,0,sizeof(CPSS_DXCH_IP_TCAM_ROUTE_ENTRY_INFO_UNT));
    cpssOsMemSet(&defMcLttEntry,0,sizeof(CPSS_DXCH_IP_LTT_ENTRY_STC));

    defUcLttEntry.ipLttEntry.ipv6MCGroupScopeLevel    = CPSS_IPV6_PREFIX_SCOPE_GLOBAL_E;
    defUcLttEntry.ipLttEntry.numOfPaths               = 0;
    defUcLttEntry.ipLttEntry.routeEntryBaseIndex      = 0;
    if ((shadowType == CPSS_DXCH_IP_LPM_RAM_SIP5_SHADOW_E)||
        (shadowType == CPSS_DXCH_IP_LPM_RAM_SIP6_SHADOW_E)||
        (shadowType == CPSS_DXCH_IP_LPM_RAM_SIP7_SHADOW_E))
    {
        defUcLttEntry.ipLttEntry.routeType            = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_REGULAR_E;
    }
    else
    {
        defUcLttEntry.ipLttEntry.routeType            = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E;
    }

    defUcLttEntry.ipLttEntry.sipSaCheckMismatchEnable = GT_FALSE;
    defUcLttEntry.ipLttEntry.ucRPFCheckEnable         = GT_FALSE;

    defMcLttEntry.ipv6MCGroupScopeLevel    = CPSS_IPV6_PREFIX_SCOPE_GLOBAL_E;
    defMcLttEntry.numOfPaths               = 0;
    defMcLttEntry.routeEntryBaseIndex      = 1;
    if ((shadowType == CPSS_DXCH_IP_LPM_RAM_SIP5_SHADOW_E)||
        (shadowType == CPSS_DXCH_IP_LPM_RAM_SIP6_SHADOW_E)||
        (shadowType == CPSS_DXCH_IP_LPM_RAM_SIP7_SHADOW_E))
    {
        defMcLttEntry.routeType            = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_REGULAR_E;
    }
    else
    {
        defMcLttEntry.routeType            = CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E;
    }

    defMcLttEntry.sipSaCheckMismatchEnable = GT_FALSE;
    defMcLttEntry.ucRPFCheckEnable         = GT_FALSE;


    cpssOsMemSet(&vrConfigInfo,0,sizeof(CPSS_DXCH_IP_LPM_VR_CONFIG_STC));

    /* define protocolStack */
    if ((ppProfilePtr->lpmDbSupportIpv4 == GT_FALSE) && (ppProfilePtr->lpmDbSupportIpv6 == GT_FALSE))
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
     if ((ppProfilePtr->lpmDbSupportIpv4 == GT_TRUE) && (ppProfilePtr->lpmDbSupportIpv6 == GT_TRUE))
        protocolStack = CPSS_IP_PROTOCOL_IPV4V6_E;
    else
        protocolStack = (ppProfilePtr->lpmDbSupportIpv4 == GT_TRUE) ?
            CPSS_IP_PROTOCOL_IPV4_E : CPSS_IP_PROTOCOL_IPV6_E;

    /* fill vrConfigInfo structure */
    switch(protocolStack)
    {
    case CPSS_IP_PROTOCOL_IPV4_E:
        vrConfigInfo.supportIpv4Uc = GT_TRUE;
        cpssOsMemCpy(&vrConfigInfo.defIpv4UcNextHopInfo.ipLttEntry,
                     &defUcLttEntry.ipLttEntry,
                     sizeof(CPSS_DXCH_IP_LTT_ENTRY_STC));
        vrConfigInfo.supportIpv6Uc = GT_FALSE;
        vrConfigInfo.supportIpv6Mc = GT_FALSE;

        vrConfigInfo.supportIpv4Mc = GT_TRUE;
        cpssOsMemCpy(&vrConfigInfo.defIpv4McRouteLttEntry,
                     &defMcLttEntry,
                     sizeof(CPSS_DXCH_IP_LTT_ENTRY_STC));
        vrConfigInfo.treatIpv4ClassEasNonRegularUc = GT_FALSE;
        cpssOsMemCpy(&vrConfigInfo.defaultFcoeForwardingNextHopInfo.ipLttEntry,
                     &defUcLttEntry.ipLttEntry,
                     sizeof(CPSS_DXCH_IP_LTT_ENTRY_STC));
        break;
    case CPSS_IP_PROTOCOL_IPV6_E:
        vrConfigInfo.supportIpv4Uc = GT_FALSE;
        vrConfigInfo.supportIpv6Uc = GT_TRUE;
        cpssOsMemCpy(&vrConfigInfo.defIpv6UcNextHopInfo.ipLttEntry,
                     &defUcLttEntry.ipLttEntry,
                     sizeof(CPSS_DXCH_IP_LTT_ENTRY_STC));
        vrConfigInfo.supportIpv4Mc = GT_FALSE;
        vrConfigInfo.supportIpv6Mc = GT_TRUE;
        cpssOsMemCpy(&vrConfigInfo.defIpv6McRouteLttEntry,
                     &defMcLttEntry,
                     sizeof(CPSS_DXCH_IP_LTT_ENTRY_STC));
        break;
    case CPSS_IP_PROTOCOL_IPV4V6_E:
        vrConfigInfo.supportIpv4Uc = GT_TRUE;
        cpssOsMemCpy(&vrConfigInfo.defIpv4UcNextHopInfo.ipLttEntry,
                     &defUcLttEntry.ipLttEntry,
                     sizeof(CPSS_DXCH_IP_LTT_ENTRY_STC));
        vrConfigInfo.treatIpv4ClassEasNonRegularUc = GT_FALSE;
        cpssOsMemCpy(&vrConfigInfo.defaultFcoeForwardingNextHopInfo.ipLttEntry,
                     &defUcLttEntry.ipLttEntry,
                     sizeof(CPSS_DXCH_IP_LTT_ENTRY_STC));
        vrConfigInfo.supportIpv6Uc = GT_TRUE;
        cpssOsMemCpy(&vrConfigInfo.defIpv6UcNextHopInfo.ipLttEntry,
                     &defUcLttEntry.ipLttEntry,
                     sizeof(CPSS_DXCH_IP_LTT_ENTRY_STC));
        vrConfigInfo.supportIpv4Mc = GT_TRUE;
        vrConfigInfo.supportIpv6Mc = GT_TRUE;

        vrConfigInfo.supportIpv6Mc = GT_TRUE;
        vrConfigInfo.supportIpv4Mc = GT_TRUE;
        cpssOsMemCpy(&vrConfigInfo.defIpv4McRouteLttEntry,
                     &defMcLttEntry,
                     sizeof(CPSS_DXCH_IP_LTT_ENTRY_STC));
        cpssOsMemCpy(&vrConfigInfo.defIpv6McRouteLttEntry,
                     &defMcLttEntry,
                     sizeof(CPSS_DXCH_IP_LTT_ENTRY_STC));


        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }


    if( (vrConfigInfo.supportIpv4Uc == GT_TRUE) || (vrConfigInfo.supportIpv6Uc == GT_TRUE) )
    {
        /************************************************************/
        /* in this case ipv4/ipv6 unicast have common default route */
        /* set route entry 0 as default ipv4/6 unicast              */
        /************************************************************/
        cpssOsMemSet(&ucRouteEntry,0,sizeof(ucRouteEntry));
        ucRouteEntry.type = CPSS_DXCH_IP_UC_ROUTE_ENTRY_E;
        ucRouteEntry.entry.regularEntry.cmd = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
        rc = cpssDxChIpUcRouteEntriesWrite(devNum, 0, &ucRouteEntry, 1);
        if (rc != GT_OK)
        {
            if(rc == GT_OUT_OF_RANGE)
            {
                /* the device not support any IP (not router device)*/
                rc = GT_OK;

                CPSS_APP_PLATFORM_LOG_ERR_MAC("cpssDxChIpUcRouteEntriesWrite : device[%d] not supported \n",devNum);
            }

            return  rc;
        }
    }

    if( (vrConfigInfo.supportIpv4Mc == GT_TRUE) || (vrConfigInfo.supportIpv6Mc == GT_TRUE) )
    {
        /*******************************************************/
        /* in this case ipv4/ipv6 mc have common default route */
        /* set route entry 1 as default ipv4/6 multicast       */
        /*******************************************************/
        cpssOsMemSet(&mcRouteEntry,0,sizeof(mcRouteEntry));
        mcRouteEntry.cmd = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
        mcRouteEntry.RPFFailCommand = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
        rc = cpssDxChIpMcRouteEntriesWrite(devNum, 1, &mcRouteEntry);
        if (rc != GT_OK)
        {
            return  rc;
        }
    }

    /********************************************************************/
    /* if lpm db is already created, all that is needed to do is to add */
    /* the device to the lpm db                                         */
    /********************************************************************/
    if (PRV_APP_REF_PP_UTILS_VAR(lpmDbInitialized)== GT_TRUE)
    {
        rc = cpssDxChIpLpmDBDevListAdd(lpmDbId, &devNum, 1);
        if(rc == GT_BAD_PARAM)
        {
            CPSS_APP_PLATFORM_LOG_ERR_MAC("cpssDxChIpLpmDBDevListAdd : device[%d] not supported \n",devNum);
            /* the device not support the router tcam */
            rc = GT_OK;
        }

        return rc;
    }

    /*****************/
    /* create LPM DB */
    /*****************/

    /* set parameters */

    cpssLpmDbCapacity.numOfIpv4Prefixes         = ppProfilePtr->maxNumOfIpv4Prefixes;
    cpssLpmDbCapacity.numOfIpv6Prefixes         = ppProfilePtr->maxNumOfIpv6Prefixes;
    cpssLpmDbCapacity.numOfIpv4McSourcePrefixes = ppProfilePtr->maxNumOfIpv4McEntries;
    cpssLpmDbRange.firstIndex                   = ppProfilePtr->lpmDbFirstTcamLine;
    cpssLpmDbRange.lastIndex                    = ppProfilePtr->lpmDbLastTcamLine;

    if ((shadowType != CPSS_DXCH_IP_LPM_RAM_SIP5_SHADOW_E)&&
        (shadowType != CPSS_DXCH_IP_LPM_RAM_SIP6_SHADOW_E)&&
        (shadowType != CPSS_DXCH_IP_LPM_RAM_SIP7_SHADOW_E))
    {
        lpmMemoryConfig.tcamDbCfg.indexesRangePtr = &cpssLpmDbRange;
        lpmMemoryConfig.tcamDbCfg.partitionEnable = ppProfilePtr->lpmDbPartitionEnable;
        lpmMemoryConfig.tcamDbCfg.tcamLpmManagerCapcityCfgPtr = &cpssLpmDbCapacity;
        lpmMemoryConfig.tcamDbCfg.tcamManagerHandlerPtr = NULL;
    }
    else
    {
        if(PRV_CPSS_SIP_6_CHECK_MAC(devNum))
        {
            rc = appPlatformFalconIpLpmRamDefaultConfigCalc(devNum, ppProfilePtr->sharedTableMode, ppProfilePtr->maxNumOfPbrEntries, &lpmMemoryConfig.ramDbCfg);
            CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, appPlatformBc2IpLpmRamDefaultConfigCalc);
        }
        else
        {
            rc = appPlatformBc2IpLpmRamDefaultConfigCalc(devNum, ppProfilePtr->maxNumOfPbrEntries, &lpmMemoryConfig.ramDbCfg);
            CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, appPlatformBc2IpLpmRamDefaultConfigCalc);
        }
    }

    rc = cpssDxChIpLpmDBCreate(lpmDbId, shadowType,
                               protocolStack, &lpmMemoryConfig);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChIpLpmDBCreate);


    /* mark the lpm db as created */
    PRV_APP_REF_PP_UTILS_VAR(lpmDbInitialized) = GT_TRUE;

    /*******************************/
    /* add active device to LPM DB */
    /*******************************/
    rc = cpssDxChIpLpmDBDevListAdd(lpmDbId, &devNum, 1);
    if(rc == GT_BAD_PARAM)
    {
        /* the device not support the router tcam */
        CPSS_APP_PLATFORM_LOG_ERR_MAC("cpssDxChIpLpmDBDevListAdd : device[%d] not supported \n",devNum);
        return GT_OK;
    }
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChIpLpmDBDevListAdd);

    /*************************/
    /* create virtual router */
    /*************************/
    rc = cpssDxChIpLpmVirtualRouterAdd(lpmDbId, 0, &vrConfigInfo);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChIpLpmVirtualRouterAdd);

    return rc;
}

static GT_STATUS prvIpLibInit
(
    IN GT_U8                             devNum,
    IN CPSS_APP_PLATFORM_PP_PROFILE_STC *ppProfilePtr
)
{
    GT_STATUS rc = GT_OK;

    /* if policy based routing, use same initialization as Ch+,
       if not use same initialization as Ch2,Ch3 */
    if (ppProfilePtr->usePolicyBasedRouting == GT_TRUE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_IMPLEMENTED, LOG_ERROR_NO_MSG);
    }
    else
    {
        rc = prvDxCh2Ch3IpLibInit(devNum,ppProfilePtr);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, prvDxCh2Ch3IpLibInit);
    }

    return rc;
}


extern GT_VOID_PTR prvCpssSlSearch
(
    IN GT_VOID_PTR   ctrlPtr,
    IN GT_VOID_PTR   dataPtr
);

/**
* @internal prvCpssAppPlatformIpLpmLibReset function
* @endinternal
*
* @brief   IP LPM module reset
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS prvCpssAppPlatformIpLpmLibReset
(
   GT_VOID
)
{
    GT_STATUS       rc;                 /* return code */
    GT_U32          lpmDbId = 0;        /* LPM DB index */
    GT_UINTPTR      slIterator;         /* Skip List iterator */
    GT_U8           devListArray[PRV_CPSS_MAX_PP_DEVICES_CNS]; /* device list */
    GT_U32          numOfDevs;          /* device number (array size) */
    PRV_CPSS_DXCH_LPM_SHADOW_STC        *lpmDbPtr,tmpLpmDb; /* pointer to and temp instance of LMP DB entry */
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC    *ipShadowPtr; /* pointer to shadow entry */
    PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC*ipShadowsPtr; /* pointer to shadows DB */
    PRV_CPSS_DXCH_LPM_TCAM_SHADOW_STC    *tcam_ipShadowPtr;/* TCAM (xcat style): pointer to shadow entry */
    PRV_CPSS_DXCH_LPM_TCAM_VR_TBL_BLOCK_STC  *vrEntryPtr = NULL;/* TCAM (xcat style): iterator over virtual routers */
    GT_UINTPTR                          vrIterator;
    GT_U32                                 vrId;

    GT_U32          shadowIdx;          /* shadow index */
    GT_U32          i;                  /* loop iterator */
    CPSS_DXCH_IP_LPM_SHADOW_TYPE_ENT    shadowType;
    CPSS_IP_PROTOCOL_STACK_ENT          protocolStack;
    CPSS_DXCH_IP_LPM_MEMORY_CONFIG_UNT  memoryCfg;
    CPSS_DXCH_IP_TCAM_LPM_MANGER_INDEX_RANGE_STC indexesRange;
    CPSS_DXCH_IP_TCAM_LPM_MANGER_CAPCITY_CFG_STC capcityCfg;

    cpssOsMemSet(&memoryCfg,0,sizeof(memoryCfg));
    memoryCfg.tcamDbCfg.indexesRangePtr = &indexesRange;
    memoryCfg.tcamDbCfg.tcamLpmManagerCapcityCfgPtr = &capcityCfg;

    while (1)
    {
        slIterator = 0;
        rc =  prvCpssDxChIpLpmDbIdGetNext(&lpmDbId,&slIterator);
        if (rc == GT_NO_MORE)
        {
            break;
        }
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, prvCpssDxChIpLpmDbIdGetNext);

        rc = cpssDxChIpLpmDBConfigGet(lpmDbId,&shadowType,&protocolStack,&memoryCfg);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChIpLpmDBConfigGet);

       PRV_APP_REF_PP_UTILS_VAR(lpmDbSL) = PRV_SHARED_PORT_DIR_IP_LPM_SRC_GLOBAL_VAR_GET(lpmDbSL);

        /* check parameters */
        tmpLpmDb.lpmDbId = lpmDbId;
        lpmDbPtr = prvCpssSlSearch(PRV_APP_REF_PP_UTILS_VAR(lpmDbSL),&tmpLpmDb);
        if (lpmDbPtr == NULL)
        {
            cpssOsPrintSync("prvIpLpmLibReset: lpmDbId = %d\r\n", lpmDbId);
            /* can't find the lpm DB */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
        }

        if((shadowType == CPSS_DXCH_IP_LPM_RAM_SIP5_SHADOW_E) ||
           (shadowType == CPSS_DXCH_IP_LPM_RAM_SIP6_SHADOW_E) ||
           (shadowType == CPSS_DXCH_IP_LPM_RAM_SIP7_SHADOW_E) )
        {
            ipShadowsPtr = lpmDbPtr->shadow;

            for (shadowIdx = 0; shadowIdx < ipShadowsPtr->numOfShadowCfg; shadowIdx++)
            {
                ipShadowPtr = &ipShadowsPtr->shadowArray[shadowIdx];

                for(i = 0; i < ipShadowPtr->vrfTblSize; i++)
                {
                    if(ipShadowPtr->vrRootBucketArray[i].valid == GT_TRUE)
                    {
                        rc = cpssDxChIpLpmVirtualRouterDel(lpmDbId, i);
                        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChIpLpmVirtualRouterDel);
                    }
                }
            }
        }
        else
        {
            tcam_ipShadowPtr = (PRV_CPSS_DXCH_LPM_TCAM_SHADOW_STC*)(lpmDbPtr->shadow);
            vrIterator = 0;/* get first */
            vrEntryPtr = prvCpssSlGetFirst(tcam_ipShadowPtr->vrSl,NULL,&vrIterator);
            if(vrEntryPtr != NULL)
            {
                do
                {
                    vrId = vrEntryPtr->vrId;
                    /* hard coded ... tempo */
                    rc = cpssDxChIpLpmVirtualRouterDel(lpmDbId, vrId);
                    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChIpLpmVirtualRouterDel);
                }
                while ((vrEntryPtr = prvCpssSlGetNext(tcam_ipShadowPtr->vrSl,&vrIterator)) != NULL);
            }
        }

        numOfDevs = PRV_CPSS_MAX_PP_DEVICES_CNS;

        /* Get device list array from skip list for given LMP DB id */
        rc = cpssDxChIpLpmDBDevListGet(lpmDbId, &numOfDevs, devListArray);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChIpLpmDBDevListGet);

        /* Delete device list array for given LMP DB id */
        rc = cpssDxChIpLpmDBDevsListRemove(lpmDbId, devListArray , numOfDevs);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChIpLpmDBDevsListRemove);

        rc = cpssDxChIpLpmDBDelete(lpmDbId);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChIpLpmDBDelete);

    }

    PRV_APP_REF_PP_UTILS_VAR(lpmDbInitialized) = GT_FALSE;

    return GT_OK;
}

GT_STATUS prv_ptp_manager_get_init_param_from_xml
(
    IN     GT_U8                            devNum,
    IN     CPSS_APP_PLATFORM_PP_PROFILE_STC *ppProfilePtr,
    INOUT  CPSS_DXCH_PTP_MANAGER_TAI_OUTPUT_INTERFACE_STC *outputInterfaceConf,
    INOUT  CPSS_DXCH_PTP_MANAGER_PTP_OVER_ETHERNET_STC *ptpOverEthernetConf,
    INOUT  CPSS_DXCH_PTP_MANAGER_PTP_OVER_IP_UDP_STC *ptpOverIpUdpConf,
    INOUT  CPSS_DXCH_PTP_TS_TAG_GLOBAL_CFG_STC *ptpTsTagGlobalConf,
    INOUT  CPSS_DXCH_PTP_MANAGER_REF_CLOCK_CFG_STC *ptpRefClockConf,
    INOUT  CPSS_NET_RX_CPU_CODE_ENT *cpuCode
)
{
    GT_UNUSED_PARAM(devNum);
    GT_STATUS                rc              = GT_OK;

    /* initializing the output interface */
    outputInterfaceConf->taiNumber = ppProfilePtr->taiNumber;
    outputInterfaceConf->outputInterfaceMode = ppProfilePtr->outputInterfaceMode;
    outputInterfaceConf->nanoSeconds = ppProfilePtr->nanoSeconds;

    /* initializing the ptp over ethernet interface */
    ptpOverEthernetConf->ptpOverEthernetEnable = ppProfilePtr->ptpOverEthernetEnable;
    ptpOverEthernetConf->etherType0value = ppProfilePtr->etherType0value;
    ptpOverEthernetConf->etherType1value = ppProfilePtr->etherType1value;

    /* initializing the ptp over IP Udp interface */
    ptpOverIpUdpConf->ptpOverIpUdpEnable = ppProfilePtr->ptpOverIpUdpEnable;
    ptpOverIpUdpConf->udpPort0value = ppProfilePtr->udpPort0value;
    ptpOverIpUdpConf->udpPort1value = ppProfilePtr->udpPort1value;
    ptpOverIpUdpConf->udpCsUpdMode.ptpIpv4Mode = ppProfilePtr->ptpIpv4Mode;
    ptpOverIpUdpConf->udpCsUpdMode.ptpIpv6Mode = ppProfilePtr->ptpIpv6Mode;
    ptpOverIpUdpConf->udpCsUpdMode.ntpIpv4Mode = ppProfilePtr->ntpIpv4Mode ;
    ptpOverIpUdpConf->udpCsUpdMode.ntpIpv6Mode = ppProfilePtr->ntpIpv6Mode;
    ptpOverIpUdpConf->udpCsUpdMode.wampIpv4Mode = ppProfilePtr->wampIpv4Mode ;
    ptpOverIpUdpConf->udpCsUpdMode.wampIpv6Mode = ppProfilePtr->wampIpv6Mode;

    /* initializing the ptp TS tag config */
    ptpTsTagGlobalConf->tsTagParseEnable = ppProfilePtr->tsTagParseEnable;
    ptpTsTagGlobalConf->hybridTsTagParseEnable = ppProfilePtr->hybridTsTagParseEnable;
    ptpTsTagGlobalConf->tsTagEtherType = ppProfilePtr->tsTagEtherType;
    ptpTsTagGlobalConf->hybridTsTagEtherType = ppProfilePtr->hybridTsTagEtherType;

    /* initializing the ptp ref clock config */
    ptpRefClockConf->refClockSource = ppProfilePtr->refClockSource;
    ptpRefClockConf->refClockFrequency = ppProfilePtr->refClockFrequency;

    /* initializing the cpu code */
    *cpuCode = ppProfilePtr->ptpCpuCode;

    return rc;
}

static GT_STATUS prvPtpLibInit
(
    IN  GT_U8                            devNum,
    IN CPSS_APP_PLATFORM_PP_PROFILE_STC *ppProfilePtr
)
{
    GT_STATUS rc = GT_OK;
    CPSS_DXCH_PTP_MANAGER_TAI_OUTPUT_INTERFACE_STC outputInterfaceConf;
    CPSS_DXCH_PTP_MANAGER_PTP_OVER_ETHERNET_STC    ptpOverEthernetConf;
    CPSS_DXCH_PTP_MANAGER_PTP_OVER_IP_UDP_STC      ptpOverIpUdpConf;
    CPSS_DXCH_PTP_TS_TAG_GLOBAL_CFG_STC            ptpTsTagGlobalConf;
    CPSS_DXCH_PTP_MANAGER_REF_CLOCK_CFG_STC        ptpRefClockConf;
    CPSS_NET_RX_CPU_CODE_ENT                       cpuCode=0;

    cpssOsMemSet(&outputInterfaceConf, 0, sizeof(outputInterfaceConf));
    cpssOsMemSet(&ptpOverEthernetConf, 0, sizeof(ptpOverEthernetConf));
    cpssOsMemSet(&ptpOverIpUdpConf, 0, sizeof(ptpOverIpUdpConf));
    cpssOsMemSet(&ptpTsTagGlobalConf, 0, sizeof(ptpTsTagGlobalConf));
    cpssOsMemSet(&ptpRefClockConf, 0, sizeof(ptpRefClockConf));

    rc = prv_ptp_manager_get_init_param_from_xml(devNum, ppProfilePtr, &outputInterfaceConf,
            &ptpOverEthernetConf, &ptpOverIpUdpConf, &ptpTsTagGlobalConf,
            &ptpRefClockConf, &cpuCode);

    /* need to discuss with arch team on removing the hardcoding*/
    if(outputInterfaceConf.taiNumber == 255)
        outputInterfaceConf.taiNumber = CPSS_DXCH_PTP_TAI_NUMBER_ALL_E;

    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, prv_ac5x_ptp_manager_get_init_param_from_xml);

    rc = cpssDxChPtpManagerPtpInit(devNum, &outputInterfaceConf, &ptpOverEthernetConf, &ptpOverIpUdpConf,
            &ptpTsTagGlobalConf, &ptpRefClockConf, cpuCode);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPtpManagerPtpInit);

    return rc;
}

/* NOTE: should be used only for sip6 device */
static GT_STATUS prvPhaLibInit
(
    IN  GT_U8                            devNum,
    IN CPSS_APP_PLATFORM_PP_PROFILE_STC *ppProfilePtr
)
{
    GT_STATUS rc = GT_OK;
    CPSS_DXCH_PHA_FW_IMAGE_ID_ENT  phaFwImageId;

    /* Set PHA firmware image ID to default */
    phaFwImageId = CPSS_DXCH_PHA_FW_IMAGE_ID_DEFAULT_E;

    rc = cpssDxChPhaInit(devNum,ppProfilePtr->pha_packetOrderChangeEnable, phaFwImageId);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPhaInit);

    (GT_VOID)prvCpssDxChPhaFwVersionPrint(devNum);

    return rc;
}

/**
* @internal cpssAppPlatformPpLogicalInit function
* @endinternal
*
* @brief logical initialization for a specific Pp.
*
* @param[in] devNum                - Device Number.
* @param [in] *ppProfileInfo     - PP profile
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS cpssAppPlatformPpLogicalInit
(
    IN GT_U8                              devNum,
    IN CPSS_APP_PLATFORM_PP_PROFILE_STC  *ppProfilePtr
)
{
    GT_UNUSED_PARAM(devNum);
    GT_STATUS                        rc;
    GT_U32                           value;
    CPSS_DXCH_PP_CONFIG_INIT_STC     ppLogicalInfo;     /* CPSS format - DxCh info  */

    cpssOsMemSet(&ppLogicalInfo ,0, sizeof(ppLogicalInfo));

    ppLogicalInfo.routingMode        = ppProfilePtr->routingMode;

    if(appPlatformDbEntryGet("maxNumOfPbrEntries", &value) == GT_OK)
        ppLogicalInfo.maxNumOfPbrEntries = value;
    else
        ppLogicalInfo.maxNumOfPbrEntries = ppProfilePtr->maxNumOfPbrEntries;

    if(appPlatformDbEntryGet("lpmMemMode", &value) == GT_OK)
        ppLogicalInfo.lpmMemoryMode      = value?CPSS_DXCH_LPM_RAM_MEM_MODE_HALF_MEM_SIZE_E:CPSS_DXCH_LPM_RAM_MEM_MODE_FULL_MEM_SIZE_E;
    else
        ppLogicalInfo.lpmMemoryMode      = ppProfilePtr->lpmMemoryMode;

    if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        if(appPlatformDbEntryGet("sharedTableMode", &value) == GT_OK)
            ppLogicalInfo.sharedTableMode = value;
        else
            ppLogicalInfo.sharedTableMode = ppProfilePtr->sharedTableMode;
    }

    /* call CPSS to do Pp logical init */
    rc = cpssDxChCfgPpLogicalInit(devNum, &ppLogicalInfo);
    CPSS_APP_PLATFORM_LOG_INFO_MAC("Logical Init done...\n");
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChCfgPpLogicalInit);

    return GT_OK;

} /* cpssAppPlatformPpLogicalInit */

/**
* @internal cpssAppPlatformPpLibInit function
* @endinternal
*
* @brief   initialize CPSS LIBs in correct order.
*
* @param[in] devNum                - Device Number.
* @param [in] *ppProfileInfo       - PP profile.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS cpssAppPlatformPpLibInit
(
    IN GT_U8                             devNum,
    IN CPSS_APP_PLATFORM_PP_PROFILE_STC *ppProfilePtr
)
{
    GT_STATUS rc = GT_OK;

    if(ppProfilePtr->initPort == GT_TRUE)
    {
        rc = prvPortLibInit(devNum, ppProfilePtr);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, prvPortLibInit);
    }

    if(ppProfilePtr->initPhy)
    {
        rc = prvPhyLibInit(devNum);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, prvPhyLibInit);
    }

    if(ppProfilePtr->initBridge)
    {
        rc = prvBridgeLibInit(devNum, ppProfilePtr);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, prvBridgeLibInit);
    }

    if(ppProfilePtr->initNetIf)
    {
        rc = prvNetIfLibInit(devNum, ppProfilePtr);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, prvNetIfLibInit);
    }

    if(ppProfilePtr->initMirror)
    {
        rc = prvMirrorLibInit(devNum, ppProfilePtr);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, prvMirrorLibInit);
    }

    if(ppProfilePtr->initPcl)
    {
        rc = prvPclLibInit(devNum);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, prvPclLibInit);
    }

    if(ppProfilePtr->initTcam)
    {
        rc = prvTcamLibInit(devNum, ppProfilePtr);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, prvTcamLibInit);
    }

    if(ppProfilePtr->initPolicer)
    {
        rc = prvPolicerLibInit(devNum);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, prvPolicerLibInit);
    }
    else
    {
        rc = prvIngressPolicerDisable(devNum);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, prvIngressPolicerDisable);
    }

    if(ppProfilePtr->initTrunk)
    {
        rc = prvTrunkLibInit(devNum, ppProfilePtr);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, prvTrunkLibInit);
    }

    if(ppProfilePtr->initIp)
    {
        rc = prvIpLibInit(devNum, ppProfilePtr);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, prvIpLibInit);
    }

    if(ppProfilePtr->initPha)
    {
        rc = prvPhaLibInit(devNum, ppProfilePtr);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, prvPhaLibInit);
    }

    if(ppProfilePtr->initPtp)
    {
        rc = prvPtpLibInit(devNum, ppProfilePtr);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, prvPtpLibInit);
    }

    CPSS_APP_PLATFORM_LOG_INFO_MAC("Lib Init done...\n");

    return rc;

} /* cpssAppPlatformPpLibInit */

/**
* @internal prvFdbActionDelete function
* @endinternal
*
* @brief   Deletes all addresses from FDB table.
*
* @param[in] dev                      - physical device number
*
* @param[out] actDevPtr                - pointer to old action device number
* @param[out] actDevMaskPtr            - pointer to old action device mask
* @param[out] triggerModePtr           - pointer to old trigger mode
* @param[out] modePtr                  - pointer to old action mode
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
*/
static GT_STATUS prvFdbActionDelete
(
    IN  GT_U8                       dev,
    OUT GT_U32                      *actDevPtr,
    OUT GT_U32                      *actDevMaskPtr,
    OUT CPSS_MAC_ACTION_MODE_ENT    *triggerModePtr,
    OUT CPSS_FDB_ACTION_MODE_ENT    *modePtr
)
{
    GT_STATUS rc;

    /* save Mac Action action mode */
    rc = cpssDxChBrgFdbActionModeGet(dev, modePtr);
    if (rc != GT_OK)
    {
        CPSS_APP_PLATFORM_LOG_ERR_MAC("cpssDxChBrgFdbActionModeGet rc= %d ret=%d", rc, GT_HW_ERROR);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
    }

    /* save Mac Action trigger mode */
    rc = cpssDxChBrgFdbMacTriggerModeGet(dev, triggerModePtr);
    if (rc != GT_OK)
    {
        CPSS_APP_PLATFORM_LOG_ERR_MAC("cpssDxChBrgFdbMacTriggerModeGet rc= %d ret=%d", rc, GT_HW_ERROR);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
    }

    /* save mac action device parameters */
    rc = cpssDxChBrgFdbActionActiveDevGet(dev,actDevPtr,actDevMaskPtr);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChBrgFdbActionActiveDevGet);


    /* disable actions to be done on the entire MAC table
       before change active configuration */
    rc = cpssDxChBrgFdbActionsEnableSet(dev, GT_FALSE);
    if (rc != GT_OK)
    {
        CPSS_APP_PLATFORM_LOG_ERR_MAC("cpssDxChBrgFdbActionsEnableSet rc= %d ret=%d", rc, GT_HW_ERROR);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_HW_ERROR, LOG_ERROR_NO_MSG);
    }


    /* delete all entries regardless of device number */
    rc = cpssDxChBrgFdbActionActiveDevSet(dev,0, 0);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChBrgFdbActionActiveDevSet);

    /* enable actions, action mode, triggered mode, trigger the action */
    rc = cpssDxChBrgFdbTrigActionStart(dev, CPSS_FDB_ACTION_DELETING_E);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChBrgFdbTrigActionStart);

    return rc;
}

/**
* @internal appPlatformDxChFdbFlush function
* @endinternal
*
* @brief   Deletes all addresses from FDB table.
*
* @param[in] dev                      - physical device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS appPlatformDxChFdbFlush
(
    IN GT_U8   dev
)
{
    GT_STATUS                 rc;
    GT_BOOL                   actionCompleted;      /* Action Trigger Flag         */
    GT_U32                    loopCounter;          /* Counter for busy wait loops */
    /* old value of AA TA messages to CPU enabler */
    GT_BOOL                   aaTaMessegesToCpuEnable;
    GT_U32                    actDev;               /* old action device number */
    GT_U32                    actDevMask;           /* old action device mask */
    CPSS_MAC_ACTION_MODE_ENT  triggerMode;          /* old trigger mode */
    CPSS_FDB_ACTION_MODE_ENT  mode;                 /* old action mode */
    GT_U32 numFdbWaitIterations = 0;


    /* enable delete of static entries */
    rc = cpssDxChBrgFdbStaticDelEnable(dev, CPSS_DXCH_BRG_FDB_DEL_MODE_ALL_E);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChBrgFdbStaticDelEnable);

    /* save old value of AA TA messages to CPU enabler */
    rc = cpssDxChBrgFdbAAandTAToCpuGet(dev, &aaTaMessegesToCpuEnable);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChBrgFdbAAandTAToCpuGet);

    if(aaTaMessegesToCpuEnable == GT_TRUE)
    {


        /* disable AA and TA messages to CPU */
        rc = cpssDxChBrgFdbAAandTAToCpuSet(dev, GT_FALSE);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChBrgFdbAAandTAToCpuSet);

    }

    rc = prvFdbActionDelete(dev, &actDev, &actDevMask, &triggerMode, &mode);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, prvFdbActionDelete);


    actionCompleted = GT_FALSE;
    loopCounter = 0;

    /* busy wait for Action Trigger Status */
    while (!actionCompleted)
    {
        rc = cpssDxChBrgFdbTrigActionStatusGet(dev, &actionCompleted);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChBrgFdbTrigActionStatusGet);

        loopCounter ++;

        /* check if loop counter has reached 100000000 */
        if (loopCounter >= 100000000)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_TIMEOUT, LOG_ERROR_NO_MSG);
        }

        if(actionCompleted == GT_TRUE)
        {
            /* do nothing */
        }
        else if (cpssDeviceRunCheck_onEmulator())
        {
            osTimerWkAfter(500);
            CPSS_APP_PLATFORM_LOG_INFO_MAC(".");/* show progress ... */
            numFdbWaitIterations++;
        }
        else
        {
            osTimerWkAfter(1);
        }

    }

    if(numFdbWaitIterations)
    {
        CPSS_APP_PLATFORM_LOG_INFO_MAC("(trigger) FDB flush : numFdbWaitIterations[%d] equal [%d]ms wait \n",
            numFdbWaitIterations,numFdbWaitIterations*500);
    }

    if(aaTaMessegesToCpuEnable == GT_TRUE)
    {
        /* restore AA and TA messages to CPU */
        rc = cpssDxChBrgFdbAAandTAToCpuSet(dev, aaTaMessegesToCpuEnable);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChBrgFdbAAandTAToCpuSet);
    }


    /* restore active device parameters */
    rc = cpssDxChBrgFdbActionActiveDevSet(dev,
                                          actDev,
                                          actDevMask);

    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChBrgFdbActionActiveDevSet);

    /* disable delete of static entries */
    rc = cpssDxChBrgFdbStaticDelEnable(dev, CPSS_DXCH_BRG_FDB_DEL_MODE_DYNAMIC_ONLY_E);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChBrgFdbStaticDelEnable);


    /* restore Trigger mode - it should be last operation because
       it may start auto aging. All active config should be restored
       before start of auto aging */
    rc = cpssDxChBrgFdbMacTriggerModeSet(dev, triggerMode);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChBrgFdbMacTriggerModeSet);

    return rc;

}

/**
* @internal appPlatformDxChFdbInit function
* @endinternal
*
* @brief   Perform Fdb initialization for a Pp.
*
* @param[in] devNum                - Device Number.
* @param [in] *ppProfileInfo     - PP profile
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS appPlatformDxChFdbInit
(
    IN GT_U8 devNum,
    IN CPSS_APP_PLATFORM_PP_PROFILE_STC *ppProfilePtr
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    port;
    GT_BOOL   actFinished = GT_FALSE;
    GT_BOOL   autoLearn = GT_FALSE;
    GT_U32    fdbBmp;
    GT_U32    numPorts;
    GT_HW_DEV_NUM   hwDevNum;
    CPSS_FDB_ACTION_MODE_ENT    actionMode;

    /*********************************************************/
    /* Set FDB learning mode                                 */
    /*********************************************************/
    rc = cpssDxChBrgFdbMacVlanLookupModeSet(devNum, CPSS_IVL_E);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChBrgFdbMacVlanLookupModeSet);

    /* restore automatic action mode*/
    rc = cpssDxChBrgFdbMacTriggerModeSet(devNum, CPSS_ACT_AUTO_E);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChBrgFdbMacTriggerModeSet);

    /* default */
    actionMode = CPSS_FDB_ACTION_AGE_WITH_REMOVAL_E;

    switch(PRV_CPSS_DXCH_PP_MAC(devNum)->portGroupsExtraInfo.fdbMode)
    {
        case PRV_CPSS_DXCH_MULTI_PORT_GROUP_FDB_MODE_UNIFIED_E:
        case PRV_CPSS_DXCH_MULTI_PORT_GROUP_FDB_MODE_UNIFIED_LINKED_E:
            actionMode = CPSS_FDB_ACTION_AGE_WITHOUT_REMOVAL_E;
            break;
        case PRV_CPSS_DXCH_MULTI_PORT_GROUP_FDB_MODE_NON_MULTI_PORT_GROUP_DEVICE_E:
            if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_LION2_E)
            {
                actionMode = CPSS_FDB_ACTION_AGE_WITHOUT_REMOVAL_E;
            }
            break;
        default:
            break;
    }

    if (cpssDeviceRunCheck_onEmulator())
    {
        /* a waist of time to trigger operation on huge FDB table that is empty anyway */
        rc = cpssDxChBrgFdbActionModeSet(devNum, actionMode);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChBrgFdbActionModeSet);
    }
    else
    {
        /* restore age with/without removal */
        rc = cpssDxChBrgFdbTrigActionStart(devNum, actionMode);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChBrgFdbTrigActionStart);

        /* wait for action to be over */
        while(actFinished == GT_FALSE)
        {
            rc = cpssDxChBrgFdbTrigActionStatusGet(devNum,&actFinished);
            CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChBrgFdbTrigActionStatusGet);

            if(actFinished == GT_TRUE)
            {
                /* do nothing */
            }
            else
            {
                osTimerWkAfter(1);
            }
        }
    }

    /* enable the sending to CPU of AA/TA */
    rc = cpssDxChBrgFdbAAandTAToCpuSet(devNum, GT_TRUE);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChBrgFdbAAandTAToCpuSet);

    /* Setting aging timeout to default timeout (300 seconds) is needed because
       DX106 core clock is 144MHz and not 200MHz as in other PPs. */
    rc = cpssDxChBrgFdbAgingTimeoutSet(devNum, 300);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChBrgFdbAgingTimeoutSet);

    if(GT_FALSE == PRV_CPSS_DXCH_PP_HW_INFO_PORT_MAP_CHECK_SUPPORTED_MAC(devNum))
    {
        numPorts = PRV_APP_REF_PP_CONFIG_VAR(cpssCapMaxPortNum)[devNum]; /*Not PRV_CPSS_PP_MAC(devNum)->numOfPorts, but we need the max port number.*/
    }
    else
    {
        /* set range of physical ports although those are in eport range table */
        numPorts = MAX(256,PRV_CPSS_DXCH_MAX_PHY_PORT_NUMBER_MAC(devNum));
    }

    if(ppProfilePtr->ctrlMacLearn == GT_FALSE)
    {
        autoLearn = GT_TRUE;
    }

    for (port = 0; port < numPorts; port++)
    {
        if(GT_FALSE == PRV_CPSS_DXCH_PP_HW_INFO_PORT_MAP_CHECK_SUPPORTED_MAC(devNum))
        {
            CPSS_APPLICATION_PORT_SKIP_CHECK(devNum,port);
        }

        rc = cpssDxChBrgFdbPortLearnStatusSet(devNum, port, autoLearn, CPSS_LOCK_FRWRD_E);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChBrgFdbPortLearnStatusSet);

        /* note the cascade init will make sure to disable the sending from
           the cascade ports */
        rc = cpssDxChBrgFdbNaToCpuPerPortSet(devNum, port, GT_TRUE);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChBrgFdbNaToCpuPerPortSet);
    }

    fdbBmp = 0;
    /*Currently 1 device supported.will be updated later for more device*/
    /*
    for(i = 0; i < PRV_CPSS_MAX_PP_DEVICES_CNS; i++)
    {
        if((appDemoPpConfigList[i].valid == GT_TRUE) &&
           (PRV_CPSS_PP_MAC(appDemoPpConfigList[i].devNum)->devFamily !=
                            CPSS_PX_FAMILY_PIPE_E))
        {
            rc = cpssDxChCfgHwDevNumGet(appDemoPpConfigList[i].devNum, &hwDevNum);
            CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChCfgHwDevNumGet);
        }
    }*/
    rc = cpssDxChCfgHwDevNumGet(devNum, &hwDevNum);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChCfgHwDevNumGet);
    fdbBmp |= (1 << hwDevNum );

    if (! PRV_CPSS_SIP_5_CHECK_MAC(devNum))
    {
        if(fdbBmp != 0)
        {
            rc = cpssDxChBrgFdbDeviceTableSet(devNum, fdbBmp);
            CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChBrgFdbDeviceTableSet);
        }
    }

    if (cpssDeviceRunCheck_onEmulator())
    {
        /* a waist of time to trigger operation on huge FDB table that is empty anyway */
    }
    else
    {
        /* Deletes all addresses from FDB table */
        rc = appPlatformDxChFdbFlush(devNum);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, appPlatformDxChFdbFlush);
    }

    if(ppProfilePtr->ctrlMacLearn == GT_TRUE)
    {
        /* code from afterInitBoardConfig(...) */
        /* Set 'Controlled aging' because the port groups can't share refresh info
           the AA to CPU enabled from
           appDemoDxChFdbInit(...) --> call cpssDxChBrgFdbAAandTAToCpuSet(...) */
        rc = cpssDxChBrgFdbActionModeSet(devNum, CPSS_FDB_ACTION_AGE_WITHOUT_REMOVAL_E);
        if(rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

/**
* @internal appPlatformDxChIsLocalPort_or_RemotePort_with_MAC_PHY_OBJ function
* @endinternal
*
* @brief   the function allow to distinguish between ports that supports 'MAC'
*           related APIs on the 'local device' to those that not.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                - device number
* @param[in] portNum               - port number
*
* @retval 0         - the port is remote without 'MAC_PHY_OBJ' (not support 'MAC' APIs)
* @retval non-zero  - the port is local or remote with 'MAC_PHY_OBJ'
*/
GT_U32 appPlatformDxChIsLocalPort_or_RemotePort_with_MAC_PHY_OBJ
(
     GT_U8   devNum,
     GT_U32  portNum
)
{
    GT_STATUS             rc;
    CPSS_MACDRV_OBJ_STC * portMacObjPtr;
    GT_U32                  portMacNum;      /* MAC number */

    if(0 == PRV_CPSS_IS_DEV_EXISTS_MAC(devNum))
    {
        return 1;
    }

    if(!prvCpssDxChPortRemotePortCheck(devNum,portNum))
    {
        /* the port is local */
        return 1;
    }

    /* convert the remote port to it's LOCAL MAC number */
    rc = prvCpssDxChPortPhysicalPortMapCheckAndConvertForRemote(devNum, portNum, PRV_CPSS_DXCH_PORT_TYPE_MAC_E, &portMacNum);
    if(rc != GT_OK)
    {
        return 0;
    }

    /* Get PHY MAC object pointer */
    portMacObjPtr = PRV_CPSS_PHY_MAC_OBJ(devNum,portMacNum);

    return  portMacObjPtr ?
            1 :  /* the port is remote with 'MAC_PHY_OBJ' */
            0;   /* the port is remote without 'MAC_PHY_OBJ' (not support 'MAC' APIs) */
}

/**
* @internal appPlatformDxAldrin2TailDropDbaEnableConfigure function
* @endinternal
*
* @brief   Enables/disables Tail Drop DBA. Relevant for Aldrin2 only, does nothing for other devices.
*
* @param[in] dev                      -   device number
* @param[in] enable                   -   enable (GT_TRUE) or disable (GT_FALSE) Tail Drop DBA
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS appPlatformDxAldrin2TailDropDbaEnableConfigure
(
    IN  GT_U8     dev,
    IN  GT_BOOL   enable
)
{
    /* Aldrin2 defaults from JIRA CPSS-7422 */
    GT_STATUS                               rc;
    CPSS_PORT_TX_DROP_PROFILE_SET_ENT       profileSet;
    GT_U8                                   trafficClass;
    CPSS_PORT_TX_Q_TAIL_DROP_PROF_TC_PARAMS_STC tailDropProfileParams;
    CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT    profileTdDbaAlfa;
    GT_U32                                  portMaxBuffLimit;
    GT_U32                                  portMaxDescrLimit;
    GT_U32                                  dp0MaxBuffNum;
    CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT    dp0QueueAlpha;
    GT_U32                                  dp1MaxBuffNum;
    CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT    dp1QueueAlpha;
    GT_U32                                  dp2MaxBuffNum;
    CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT    dp2QueueAlpha;
    GT_U32                                  dp0MaxDescrNum;
    GT_U32                                  dp1MaxDescrNum;
    GT_U32                                  dp2MaxDescrNum;

    /* If not Aldrin2 do nothing */
    if (0 == PRV_CPSS_SIP_5_25_CHECK_MAC(dev)) return GT_OK;
    if (PRV_CPSS_SIP_6_CHECK_MAC(dev))         return GT_OK;

    if (enable == GT_FALSE)
    {
        /* HW defaults */
        profileTdDbaAlfa     = PRV_CPSS_ALDRIN2_TAIL_DROP_DEFAULT_DBA_ALPHA_CNS;
        portMaxBuffLimit     = PRV_CPSS_ALDRIN2_TAIL_DROP_DEFAULT_MAX_BUF_LIMIT_CNS;
        portMaxDescrLimit    = PRV_CPSS_ALDRIN2_TAIL_DROP_DEFAULT_MAX_DESCR_LIMIT_CNS;
        dp0MaxBuffNum        = PRV_CPSS_ALDRIN2_TAIL_DROP_DEFAULT_MAX_BUF_LIMIT_CNS;
        dp0QueueAlpha        = PRV_CPSS_ALDRIN2_TAIL_DROP_DEFAULT_DBA_ALPHA_CNS;
        dp1MaxBuffNum        = PRV_CPSS_ALDRIN2_TAIL_DROP_DEFAULT_MAX_BUF_LIMIT_CNS;
        dp1QueueAlpha        = PRV_CPSS_ALDRIN2_TAIL_DROP_DEFAULT_DBA_ALPHA_CNS;
        dp2MaxBuffNum        = PRV_CPSS_ALDRIN2_TAIL_DROP_DEFAULT_MAX_BUF_LIMIT_CNS;
        dp2QueueAlpha        = PRV_CPSS_ALDRIN2_TAIL_DROP_DEFAULT_DBA_ALPHA_CNS;
        dp0MaxDescrNum       = PRV_CPSS_ALDRIN2_TAIL_DROP_DEFAULT_MAX_DESCR_LIMIT_CNS;
        dp1MaxDescrNum       = PRV_CPSS_ALDRIN2_TAIL_DROP_DEFAULT_MAX_DESCR_LIMIT_CNS;
        dp2MaxDescrNum       = PRV_CPSS_ALDRIN2_TAIL_DROP_DEFAULT_MAX_DESCR_LIMIT_CNS;
    }
    else
    {
        profileTdDbaAlfa     = CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_E;
        portMaxBuffLimit     = 0xFFFF;
        portMaxDescrLimit    = 0xFFFF;
        dp0MaxBuffNum        = 0;
        dp0QueueAlpha        = CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_1_E;
        dp1MaxBuffNum        = 0;
        dp1QueueAlpha        = CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_1_E;
        dp2MaxBuffNum        = 0;
        dp2QueueAlpha        = CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_1_E;
        dp0MaxDescrNum       = 0xFFFF;
        dp1MaxDescrNum       = 0xFFFF;
        dp2MaxDescrNum       = 0xFFFF;
    }

    /* PFC DBA is not enabled by default */
    /* Aldrin2: /<CPFC_IP> CPFC_IP TLU/Units/CPFC_IP_SIP6.70 Units/Dynamic Buffer Allocation
       /Dynamic Buffer Allocation Disable bit0 default is 0 */

    /* QCN DBA is not enabled by default */
    /* Aldrin2: <TXQ_IP> TXQ_QCN/Units/TXQ_IP_qcn/CN Global Configuration bit7 default is 0 */

    /* TC Descriptor set to infinite */
    /* Buffer limit set to 0         */
    /* o Port thresholds set to infinity (disabled) */
    /* o Queue alpha\92s = 1 */
    /* o Available buffers = 38K   */
    for (profileSet = CPSS_PORT_TX_DROP_PROFILE_1_E;
         (profileSet <= CPSS_PORT_TX_DROP_PROFILE_16_E); profileSet++)
    {
        rc = cpssDxChPortTxTailDropProfileSet(
            dev, profileSet, profileTdDbaAlfa,
            portMaxBuffLimit, portMaxDescrLimit);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPortTxTailDropProfileSet);

        for (trafficClass = 0; (trafficClass < 8); trafficClass++)
        {
            rc = cpssDxChPortTx4TcTailDropProfileGet(
                dev, profileSet, trafficClass, &tailDropProfileParams);
            CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPortTx4TcTailDropProfileGet);

            tailDropProfileParams.dp0MaxBuffNum  = dp0MaxBuffNum;
            tailDropProfileParams.dp0QueueAlpha  = dp0QueueAlpha;
            tailDropProfileParams.dp1MaxBuffNum  = dp1MaxBuffNum;
            tailDropProfileParams.dp1QueueAlpha  = dp1QueueAlpha;
            tailDropProfileParams.dp2MaxBuffNum  = dp2MaxBuffNum;
            tailDropProfileParams.dp2QueueAlpha  = dp2QueueAlpha;
            tailDropProfileParams.dp0MaxDescrNum = dp0MaxDescrNum;
            tailDropProfileParams.dp1MaxDescrNum = dp1MaxDescrNum;
            tailDropProfileParams.dp2MaxDescrNum = dp2MaxDescrNum;
            rc = cpssDxChPortTx4TcTailDropProfileSet(
                dev, profileSet, trafficClass, &tailDropProfileParams);
            CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPortTx4TcTailDropProfileSet);
        }
    }

    /* Tail drop DBA enabled by default */
    rc =  cpssDxChPortTxDbaEnableSet(dev, enable);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPortTxDbaEnableSet);

    if (PRV_CPSS_DXCH_PP_MAC(dev)->cutThroughEnable != GT_FALSE)
    {
        /* DBA tail drop algorithm use number of buffers for management.       */
        /* Size of cut-through packets for DBA defined by configuration below  */
        /* but not real one. Jumbo 10K packet use 40 buffers.                  */
        /* Therefore configure this value to be ready for worst case scenario. */
        rc = cpssDxChPortTxTailDropBufferConsumptionModeSet(
            dev, CPSS_DXCH_PORT_TX_TAIL_DROP_BUFFER_CONSUMPTION_MODE_BYTE_E, 40);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPortTxTailDropBufferConsumptionModeSet);
    }
    return GT_OK;
}

/**
* @internal cpssAppPlatformPpGeneralInit function
* @endinternal
*
* @brief   General Pp configurations.
*
* @param[in] devNum                - Device Number.
* @param [in] *ppProfilePtr        - PP profile
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS cpssAppPlatformPpGeneralInit
(
    IN GT_U8                             devNum,
    IN CPSS_APP_PLATFORM_PP_PROFILE_STC *ppProfilePtr
)
{
    GT_STATUS           rc = GT_OK;
    GT_U32              maxPortNum;
    CPSS_PORTS_BMP_STC  portsMembers;
    CPSS_PORTS_BMP_STC  portsTagging;
    CPSS_DXCH_BRG_VLAN_INFO_STC  cpssVlanInfo;   /* cpss vlan info format    */
    CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC portsTaggingCmd; /* ports tagging command */
    GT_U32              port;           /* current port number      */
    GT_U16              vid;            /* default VLAN id          */
    GT_U32              mruSize, cpuMruSize;
    GT_BOOL             flowControlEnable;
    CPSS_PORT_MAC_TYPE_ENT  portMacType;
    CPSS_CSCD_PORT_TYPE_ENT cscdPortType;
    CPSS_SYSTEM_RECOVERY_INFO_STC system_recovery; /* holds system recovery information */
    GT_U32 isLocalPort_or_RemotePort_with_MAC_PHY_OBJ;/*
            0         - the port is remote without 'MAC_PHY_OBJ' (not support 'MAC' APIs)
            non-zero  - the port is local or remote with 'MAC_PHY_OBJ'
                    */

    rc = cpssSystemRecoveryStateGet(&system_recovery);
    if (rc != GT_OK)
        CPSS_APP_PLATFORM_LOG_ERR_MAC("cpssSystemRecoveryStateGet rc=%d", rc);
    /*
    if(appDemoOnDistributedSimAsicSide)
    {
        return GT_OK;
    }
    */
    /***********************/
    /* FDB configuration   */
    /***********************/
    rc = appPlatformDxChFdbInit(devNum,ppProfilePtr);
    if (GT_OK != rc)
        CPSS_APP_PLATFORM_LOG_ERR_MAC("appPlatformDxChFdbInit rc=%d", rc);

    /*********************************************************/
    /* Port Configuration                                    */
    /*********************************************************/
    /* Jumbo frame support */
    mruSize = 1522; /* default */
    cpuMruSize = CPSS_DXCH_PORT_MAX_MRU_CNS; /* maximum */

    /*
    rc = appDemoDevIdxGet(dev, &devIdx);
    if ( GT_OK != rc )
    {
        return rc;
    }
     */

    for (port = 0, maxPortNum = PRV_APP_REF_PP_CONFIG_VAR(cpssCapMaxPortNum)[devNum]; port < maxPortNum; port++)
    {
        CPSS_APPLICATION_PORT_SKIP_CHECK(devNum,port);

        isLocalPort_or_RemotePort_with_MAC_PHY_OBJ =
            appPlatformDxChIsLocalPort_or_RemotePort_with_MAC_PHY_OBJ(devNum,port);

        /*  The next 2 calls are to set Port-Override-Tc-Enable */
        /* Set port trust mode */
        rc = cpssDxChCosPortQosTrustModeSet(devNum, port, CPSS_QOS_PORT_TRUST_L2_L3_E);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChCosPortQosTrustModeSet);

        /* set: Packet's DSCP is not remapped */
        rc = cpssDxChCosPortReMapDSCPSet(devNum, port, GT_FALSE);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChCosPortReMapDSCPSet);

        rc = cpssDxChPortMacTypeGet(devNum, port, &portMacType);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPortMacTypeGet);

        if (system_recovery.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_NOT_ACTIVE_E)
        {

            if(portMacType < CPSS_PORT_MAC_TYPE_XG_E)
            {
                rc = cpssDxChPortDuplexModeSet(devNum, port, CPSS_PORT_FULL_DUPLEX_E);
                CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPortDuplexModeSet);

                if (PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_LION2_E)
                {/* in devices of Lion2 family half-duplex not supported in any port mode/speed */
                    rc = cpssDxChPortDuplexAutoNegEnableSet(devNum, port, GT_TRUE);
                    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPortDuplexAutoNegEnableSet);
                }

                rc = cpssDxChPortFlowCntrlAutoNegEnableSet(devNum, port, GT_FALSE, GT_TRUE);
                CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPortFlowCntrlAutoNegEnableSet);

                rc = cpssDxChPortSpeedAutoNegEnableSet(devNum, port, GT_TRUE);
                CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPortSpeedAutoNegEnableSet);
            }

            /* Disable Flow Control for revision 3 in DxCh2, DxCh3, XCAT. */
            if(ppProfilePtr->flowControlDisable)
            {
                flowControlEnable = GT_FALSE;
            }
            else
            {
                flowControlEnable = GT_TRUE;
            }

            if(isLocalPort_or_RemotePort_with_MAC_PHY_OBJ)
            {
                if(GT_FALSE ==PRV_CPSS_SIP_6_CHECK_MAC(devNum))
                {
                    rc = cpssDxChPortFlowControlEnableSet(devNum, port,
                            flowControlEnable == GT_TRUE ? CPSS_PORT_FLOW_CONTROL_RX_TX_E : CPSS_PORT_FLOW_CONTROL_DISABLE_E);
                    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPortFlowControlEnableSet);

                    rc = cpssDxChPortPeriodicFcEnableSet(devNum, port, CPSS_PORT_PERIODIC_FLOW_CONTROL_DISABLE_E);
                    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPortPeriodicFcEnableSet);
                }
            }
        }

        if(isLocalPort_or_RemotePort_with_MAC_PHY_OBJ)
        {
            rc = cpssDxChCscdPortTypeGet(devNum, port,CPSS_PORT_DIRECTION_BOTH_E, &cscdPortType);
            CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChCscdPortTypeGet);

            if((cscdPortType == CPSS_CSCD_PORT_NETWORK_E) &&
                (prvCpssDxChPortRemotePortCheck(devNum, port) == GT_FALSE))
            {
                rc = cpssDxChPortMruSet(devNum, port, mruSize);
                CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPortMruSet);
            }
            else
            {
                /* the MRU for cascade or remote ports are set outside this function */
                /* in general the MRU of cascade ports should not cause drops ! */
                /* so needed to be set to 'max' value of the 'system' (+ max DSA tag bytes)  */
            }
        }

        if(PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC3X_E)
        {
            if(cscdPortType == CPSS_CSCD_PORT_NETWORK_E)
            {
                rc = cpssDxChPortCrcCheckEnableSet(devNum, port, GT_TRUE);
                CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPortCrcCheckEnableSet);
            }
            else
            {
                /* Configure cascade port as 'pass-through" which means the CRC check is disabled by default -
                  letting the 88e1690 port configuration to decide */
                rc = cpssDxChPortCrcCheckEnableSet(devNum, port, GT_FALSE);
                CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPortCrcCheckEnableSet);
            }
        }
        else
        {
            /* falcon in port mode 1024 use remote ports that are not bind to any device*/
            if(isLocalPort_or_RemotePort_with_MAC_PHY_OBJ)
            {
                rc = cpssDxChPortCrcCheckEnableSet(devNum, port, GT_TRUE);
                CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPortCrcCheckEnableSet);
            }
        }

        /* call cpss api function with UP == 1 */
        rc = cpssDxChPortDefaultUPSet(devNum, port, 1);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPortDefaultUPSet);
    }

    /* No dedicated MAC for CPU port in E_ARCH */
    if(PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.dedicatedCpuMac.isNotSupported == GT_FALSE)
    {
        rc = cpssDxChPortMruSet(devNum, CPSS_CPU_PORT_NUM_CNS, cpuMruSize);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPortMruSet);
    }

    /****************************************************/
    /* XG ports additional board-specific configuration */
    /****************************************************/
    /* TODO in CPSS Application Platform PHASE2
    rc = prv10GPortsConfig(devNum);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, prv10GPortsConfig);
    */



    /*********************************************************/
    /* Default VLAN configuration: VLAN 1 contains all ports */
    /*********************************************************/

    /* default VLAN id is 1 */
    vid = 1;

    /* Fill Vlan info */
    cpssOsMemSet(&cpssVlanInfo, 0, sizeof(cpssVlanInfo));
    /* default IP MC VIDX */
    cpssVlanInfo.unregIpmEVidx = 0xFFF;

    cpssVlanInfo.naMsgToCpuEn           = GT_TRUE;

    if(ppProfilePtr->ctrlMacLearn == GT_TRUE)
    {
        cpssVlanInfo.autoLearnDisable       = GT_TRUE; /* Disable auto learn on VLAN */
    }

    cpssVlanInfo.unkUcastCmd            = CPSS_PACKET_CMD_FORWARD_E;
    cpssVlanInfo.unregIpv6McastCmd      = CPSS_PACKET_CMD_FORWARD_E;
    cpssVlanInfo.unregIpv4McastCmd      = CPSS_PACKET_CMD_FORWARD_E;
    cpssVlanInfo.unregNonIpMcastCmd     = CPSS_PACKET_CMD_FORWARD_E;
    cpssVlanInfo.unregNonIpv4BcastCmd   = CPSS_PACKET_CMD_FORWARD_E;
    cpssVlanInfo.unregIpv4BcastCmd      = CPSS_PACKET_CMD_FORWARD_E;
    cpssVlanInfo.floodVidx              = 0xFFF;

    cpssVlanInfo.mirrToRxAnalyzerIndex = 0;
    cpssVlanInfo.mirrToTxAnalyzerEn = GT_FALSE;
    cpssVlanInfo.mirrToTxAnalyzerIndex = 0;
    cpssVlanInfo.fidValue = vid;
    cpssVlanInfo.unknownMacSaCmd = CPSS_PACKET_CMD_FORWARD_E;
    cpssVlanInfo.ipv4McBcMirrToAnalyzerEn = GT_FALSE;
    cpssVlanInfo.ipv4McBcMirrToAnalyzerIndex = 0;
    cpssVlanInfo.ipv6McMirrToAnalyzerEn = GT_FALSE;
    cpssVlanInfo.ipv6McMirrToAnalyzerIndex = 0;

    /* Fill ports and tagging members */
    PRV_CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsMembers);
    PRV_CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsTagging);
    cpssOsMemSet(&portsTaggingCmd, 0, sizeof(CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC));

    /* set all ports as VLAN members */
    if (system_recovery.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_NOT_ACTIVE_E)
    {
        for (port = 0, maxPortNum = PRV_APP_REF_PP_CONFIG_VAR(cpssCapMaxPortNum)[devNum]; port < maxPortNum; port++)
        {
            CPSS_APPLICATION_PORT_SKIP_CHECK(devNum,port);

            /* set the port as member of vlan */
            CPSS_PORTS_BMP_PORT_SET_MAC(&portsMembers, port);

            /* Set port pvid */
            rc = cpssDxChBrgVlanPortVidSet(devNum, port, CPSS_DIRECTION_INGRESS_E,vid);
            CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChBrgVlanPortVidSet);

            portsTaggingCmd.portsCmd[port] = CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E;
        }

        /* Write default VLAN entry */
        rc = cpssDxChBrgVlanEntryWrite(devNum, vid,
                                       &portsMembers,
                                       &portsTagging,
                                       &cpssVlanInfo,
                                       &portsTaggingCmd);

        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChBrgVlanEntryWrite);
    }
    /*********************************************************/
    /* Default TTI configuration (xCat A1 and above only):   */
    /*   - Set TTI PCL ID for IPV4 lookup to 1               */
    /*   - Set TTI PCL ID for MPLS lookup to 2               */
    /*   - Set TTI PCL ID for ETH  lookup to 3               */
    /*   - Set TTI PCL ID for MIM  lookup to 4               */
    /*********************************************************/
    if (PRV_CPSS_DXCH_XCAT_FAMILY_CHECK_MAC(devNum))
    {
        rc = cpssDxChTtiPclIdSet(devNum,CPSS_DXCH_TTI_KEY_IPV4_E,1);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChTtiPclIdSet);

        rc = cpssDxChTtiPclIdSet(devNum,CPSS_DXCH_TTI_KEY_MPLS_E,2);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChTtiPclIdSet);

        rc = cpssDxChTtiPclIdSet(devNum,CPSS_DXCH_TTI_KEY_ETH_E,3);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChTtiPclIdSet);

        rc = cpssDxChTtiPclIdSet(devNum,CPSS_DXCH_TTI_KEY_MIM_E,4);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChTtiPclIdSet);
    }

    if(PRV_CPSS_DXCH_XCAT_FAMILY_CHECK_MAC(devNum))
    {
        /* Mirror initialization -
          init Rx and Tx Global Analyzer indexes to be 0 for Ingress
          and 1 for Egress */
        /* special setting for tastBoot test */

        if(ppProfilePtr->mirrorAnalyzerPortNum == 0)
        {
            rc = cpssDxChMirrorTxGlobalAnalyzerInterfaceIndexSet(devNum,
                                                                 GT_TRUE, 1);
            CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChMirrorTxGlobalAnalyzerInterfaceIndexSet);

            rc = cpssDxChMirrorRxGlobalAnalyzerInterfaceIndexSet(devNum,
                                                                 GT_TRUE, 0);
            CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChMirrorRxGlobalAnalyzerInterfaceIndexSet);
        }

        /* eArch device configuration */
        if (PRV_CPSS_SIP_5_CHECK_MAC(devNum) &&
            !PRV_CPSS_SIP_6_CHECK_MAC(devNum))
        {
                 /* enable Rx mirroring by FDB DA lookup and set analyser index 0 */
                 rc = cpssDxChBrgFdbDaLookupAnalyzerIndexSet(devNum, GT_TRUE, 0);
                 CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChBrgFdbDaLookupAnalyzerIndexSet);

                 /* enable Rx mirroring by FDB SA lookup and set analyser index 0 */
                 rc = cpssDxChBrgFdbSaLookupAnalyzerIndexSet(devNum, GT_TRUE, 0);
                 CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChBrgFdbSaLookupAnalyzerIndexSet);
        }
    }

    /* if not Aldrin2 the function does nothing */
    rc = appPlatformDxAldrin2TailDropDbaEnableConfigure(devNum, GT_TRUE);
    if (rc != GT_OK)
    {
        return rc;
    }

    CPSS_APP_PLATFORM_LOG_INFO_MAC("General Init done...\n");
    return GT_OK;

} /* cpssAppPlatformPpGeneralInit */

/**
* @internal cpssAppPlatformPIPEngineInit function
* @endinternal
*
* @brief   ASIC specific configurations : of PIP engine
*         initialization before port configuration.
* @param[in] dev                      - devNumber
*
* @retval GT_OK                    - on success,
*/
GT_STATUS cpssAppPlatformPIPEngineInit
(
    GT_U8 dev
)
{
    GT_STATUS rc;
    GT_PHYSICAL_PORT_NUM    portNum;
    GT_U32  numOfPhysicalPorts;
    /*-------------------------------------------------------
     * CPSS-5826  Oversubscription (PIP) Engine Init Configuration
     * JUST for BC3
     *  1. Enable PIP
     *      cpssDxChPortPipGlobalEnableSet(GT_TRUE); -- default , don't configure
     *
     *
     *  2. Define thresholds :
     *      reg -0 :  0-15 - index 0   PIP Very High Priority
     *      reg -0 : 16-31 - index 1   PIP High Priority
     *      reg -1 :  0-15 - index 2   PIP Med Priority
     *      reg -1 : 16-31 - index 3   PIP Low High Priority
     *
     *
     *
     *      cpssDxChPortPipGlobalBurstFifoThresholdsSet(0,1839), very high
     *      cpssDxChPortPipGlobalBurstFifoThresholdsSet(1,1380), high
     *      cpssDxChPortPipGlobalBurstFifoThresholdsSet(2,920),  medium
     *      cpssDxChPortPipGlobalBurstFifoThresholdsSet(2,460),  low
     *
     *  3. for each port
     *             cpssDxChPortPipProfileSet(0) -- High Priority  (default)
     *             cpssDxChPortPipTrustEnableSet(FALSE)   -- Not trusted (not default)
     *     end
     *-------------------------------------------------------------------------------*/
    if (PRV_CPSS_SIP_5_20_CHECK_MAC(dev)) /* BC3 , Aldrin2 */
    {
        typedef struct
        {
            GT_U32 priority;
            GT_U32 threshold;
        }BurstFifoThreshold_STC;

        GT_U32 i;

        #define BAD_VALUE (GT_U32)(~0)

        BurstFifoThreshold_STC threshArr[] =
        {
              {         0,       1839 /* "Very High Priority"*/}
             ,{         1,       1380 /* "High Priority"     */}
             ,{         2,        920 /* "Medium Priority"   */}
             ,{         3,        460 /* "Low Priority"      */}
             ,{ BAD_VALUE,  BAD_VALUE /*  NULL               */}
        };

        /*------------------------------------------------*
         * set thresholds
         *------------------------------------------------*/
        for (i = 0; threshArr[i].priority != BAD_VALUE; i++)
        {
            rc = cpssDxChPortPipGlobalBurstFifoThresholdsSet(dev,CPSS_DATA_PATH_UNAWARE_MODE_CNS,threshArr[i].priority,threshArr[i].threshold);
            if (rc != GT_OK)
            {
                CPSS_APP_PLATFORM_LOG_ERR_MAC("cpssDxChPortPipGlobalBurstFifoThresholdsSet i=[%d], rc=%d", i, rc);
            }
        }

        if(!PRV_CPSS_SIP_6_CHECK_MAC(dev))
        {
            /*---------------------------------------------------------*
             * pass over all physical ports and set Pip Untrusted mode *
             *---------------------------------------------------------*/
           numOfPhysicalPorts =
                PRV_CPSS_DXCH_MAX_PHY_PORT_NUMBER_MAC(dev);
           for (portNum = 0; portNum < numOfPhysicalPorts; portNum++)
           {
                CPSS_APPLICATION_PORT_SKIP_CHECK(dev, portNum);

                if(prvCpssDxChPortRemotePortCheck(dev,portNum))
                {
                    continue;
                }
                rc = cpssDxChPortPipTrustEnableSet(dev,portNum,GT_FALSE);
                if (rc != GT_OK)
                {
                    CPSS_APP_PLATFORM_LOG_ERR_MAC("cpssDxChPortPipTrustEnableSet() ret=%d", rc);
                }
            }
        }
        else
        {
            /* in sip6 the per port defaults are 'untrusted'  */
            /* so no need to disable the network ports        */
        }
    }

    return GT_OK;
}


#define FALCON_6_4T_MAC_TO_SERDES_MAP_ARR_SIZE   16

#define APPDEMO_FALCON_6_4T_MAC_2_SD_MAP_0 {{7,1,6,0,5,3,4,2}}
#define APPDEMO_FALCON_6_4T_MAC_2_SD_MAP_1 {{0,6,1,7,2,4,3,5}}


#define ARR_PTR_AND_SIZE_MAC(myArr , arrPtr , arrLen) \
    arrPtr = &myArr[0];                               \
    arrLen = sizeof(myArr)/sizeof(myArr[0])

GT_STATUS prvAppPlatformPortLaneMacToSerdesMuxSet
(
    IN  GT_U8 devNum,
    IN  CPSS_PORT_MAC_TO_SERDES_STC serdesMapArr[],
    IN  GT_U32 serdesMapArrSize
)
{
    GT_STATUS                   rc = GT_OK;
    GT_U32                      portNum, portMacNum;
    CPSS_DXCH_PORT_MAP_STC      portMap;
    CPSS_PORT_MAC_TO_SERDES_STC   macToSerdesMuxStc;
    GT_U32 maxPortNumber = PRV_APP_REF_PP_CONFIG_VAR(cpssCapMaxPortNum)[devNum];
    CPSS_PORT_MAC_TO_SERDES_STC         *phoenixMacToSerdesMapPtr;
    CPSS_PORT_MAC_TO_SERDES_STC         *ac5pMacToSerdesMapPtr;
    CPSS_PORT_MAC_TO_SERDES_STC         *harrierMacToSerdesMapPtr;
    GT_U32                              falcon_MacToSerdesMap_arrSize;
    CPSS_PORT_MAC_TO_SERDES_STC*        falcon_MacToSerdesMap = NULL;

    CPSS_PORT_MAC_TO_SERDES_STC  phoenix_DB_MacToSerdesMap[] =
    {
        {{2, 1, 3, 0}} /* port 50,51,52,53
                               10,9, 11, 8*/
    };

    CPSS_PORT_MAC_TO_SERDES_STC  phoenix_RD_MacToSerdesMap[] =
    {
        {{0, 1, 2, 3}} /* port 50,51,52,53
                                   8, 9, 10, 11*/
    };

    CPSS_PORT_MAC_TO_SERDES_STC  ac5p_DB_MacToSerdesMap[] =
    {
            {{0,6,1,7,2,4,3,5}},    /* SD: 0-7    MAC: 0...25  */
            {{3,2,0,1,7,6,4,5}},    /* SD: 8-15   MAC: 26..51  */
            {{7,1,6,0,5,3,4,2}},    /* SD: 16-23  MAC: 52..77  */
            {{0,6,1,7,2,4,3,5}},    /* SD: 24-31  MAC: 78..103 */
    };


    CPSS_PORT_MAC_TO_SERDES_STC  ac5p_RD_MacToSerdesMap[] =
    {
        {{0,1,2,3,4,5,6,7}},    /* SRDS: 0-7    MAC: 0...25  */
        {{0,1,2,3,4,5,6,7}},    /* SRDS: 8-15   MAC: 26..51  */
        {{5,0,4,1,6,7,3,2}},    /* SRDS: 16-23  MAC: 52..77  */
        {{1,6,2,5,0,7,3,4}},    /* SRDS: 24-31  MAC: 78..103 */
    };


    CPSS_PORT_MAC_TO_SERDES_STC  falcon_6_4T_DB_MacToSerdesMap[FALCON_6_4T_MAC_TO_SERDES_MAP_ARR_SIZE] =
    {
        APPDEMO_FALCON_6_4T_MAC_2_SD_MAP_0,            /* 0-7*/    /*Raven 0 */
        APPDEMO_FALCON_6_4T_MAC_2_SD_MAP_1,            /* 8-15*/   /*Raven 0 */
        APPDEMO_FALCON_6_4T_MAC_2_SD_MAP_0,            /*16-23*/   /*Raven 1 */
        APPDEMO_FALCON_6_4T_MAC_2_SD_MAP_1,            /*24-31*/   /*Raven 1 */
        APPDEMO_FALCON_6_4T_MAC_2_SD_MAP_0,            /*32-39*/   /*Raven 2 */
        APPDEMO_FALCON_6_4T_MAC_2_SD_MAP_1,            /*40-47*/   /*Raven 2 */
        APPDEMO_FALCON_6_4T_MAC_2_SD_MAP_0,            /*48-55*/   /*Raven 3 */
        APPDEMO_FALCON_6_4T_MAC_2_SD_MAP_1,            /*56-63*/   /*Raven 3 */
        APPDEMO_FALCON_6_4T_MAC_2_SD_MAP_0,            /*64-71*/   /*Raven 4 */
        APPDEMO_FALCON_6_4T_MAC_2_SD_MAP_1,            /*72-79*/   /*Raven 4 */
        APPDEMO_FALCON_6_4T_MAC_2_SD_MAP_0,            /*80-87*/   /*Raven 5 */
        APPDEMO_FALCON_6_4T_MAC_2_SD_MAP_1,            /*88-95*/   /*Raven 5 */
        APPDEMO_FALCON_6_4T_MAC_2_SD_MAP_0,            /*96-103*/  /*Raven 6 */
        APPDEMO_FALCON_6_4T_MAC_2_SD_MAP_1,            /*104-111*/ /*Raven 6 */
        APPDEMO_FALCON_6_4T_MAC_2_SD_MAP_0,            /*112-119*/ /*Raven 7 */
        APPDEMO_FALCON_6_4T_MAC_2_SD_MAP_1             /*120-127*/ /*Raven 7 */
    };




    CPSS_PORT_MAC_TO_SERDES_STC falcon_12_8T_DB_MacToSerdesMap[] =
    {
       {{6,7,2,5,1,3,0,4}},  /* 0-7*/    /*Raven 0 */
       {{0,5,1,7,4,3,2,6}},  /* 8-15*/   /*Raven 0 */
       {{3,7,0,6,4,5,1,2}},  /*16-23*/   /*Raven 1 */
       {{1,5,3,7,0,4,2,6}},  /*24-31*/   /*Raven 1 */
       {{3,7,1,5,2,6,0,4}},  /*32-39*/   /*Raven 2 */
       {{2,4,7,5,1,3,0,6}},  /*40-47*/   /*Raven 2 */
       {{5,7,2,4,1,6,0,3}},  /*48-55*/   /*Raven 3 */
       {{0,6,3,7,2,4,1,5}},  /*56-63*/   /*Raven 3 */
       {{3,5,2,4,1,6,0,7}},  /*64-71*/   /*Raven 4 */
       {{3,5,1,7,4,0,2,6}},  /*72-79*/   /*Raven 4 */
       {{1,7,0,4,6,5,3,2}},  /*80-87*/   /*Raven 5 */
       {{1,5,3,7,0,4,2,6}},  /*88-95*/   /*Raven 5 */
       {{3,7,1,5,2,6,0,4}},  /*96-103*/  /*Raven 6 */
       {{2,4,7,5,1,3,0,6}},  /*104-111*/ /*Raven 6 */
       {{5,7,2,4,1,6,0,3}},  /*112-119*/ /*Raven 7 */
       {{0,6,3,7,2,4,1,5}},  /*120-127*/ /*Raven 7 */
       {{3,7,2,5,1,6,0,4}},  /*128-135*/ /*Raven 8 */
       {{0,5,1,7,4,3,2,6}},  /*136-143*/ /*Raven 8 */
       {{1,7,0,4,6,5,3,2}},  /*144-151*/ /*Raven 9 */
       {{1,5,3,7,0,4,2,6}},  /*152-159*/ /*Raven 9 */
       {{3,7,1,5,2,6,0,4}},  /*160-167*/ /*Raven 10*/
       {{2,4,7,5,1,3,0,6}},  /*168-175*/ /*Raven 10*/
       {{5,7,2,4,1,6,0,3}},  /*176-183*/ /*Raven 11*/
       {{0,6,3,7,2,4,1,5}},  /*184-191*/ /*Raven 11*/
       {{6,7,2,5,1,3,0,4}},  /*192-199*/ /*Raven 12*/
       {{0,5,4,7,1,3,2,6}},  /*200-207*/ /*Raven 12*/
       {{1,7,0,4,6,5,3,2}},  /*208-215*/ /*Raven 13*/
       {{1,5,3,7,0,4,2,6}},  /*216-223*/ /*Raven 13*/
       {{3,7,1,5,2,6,0,4}},  /*224-231*/ /*Raven 14*/
       {{2,7,0,5,1,3,4,6}},  /*232-239*/ /*Raven 14*/
       {{1,6,0,4,5,7,2,3}},  /*240-247*/ /*Raven 15*/
       {{0,4,1,5,2,6,3,7}}   /*248-255*/ /*Raven 15*/
    };

    CPSS_PORT_MAC_TO_SERDES_STC  aldrin3M_DB_MacToSerdesMap[] =
    {
        {{4,7,5,6,3,0,2,1}},    /* SD: 0-7    MAC: 0...14  */
        {{4,7,5,6,3,0,2,1}},    /* SD: 8-15   MAC: 16..30  */
        {{3,2,1,0,4,5,6,7}},    /* SD: 16-19  MAC: 32..38  */
    };

    for(portNum = 0; portNum < maxPortNumber ; portNum++)
    {
        CPSS_ENABLER_PORT_SKIP_CHECK(devNum, portNum);
        if (prvCpssDxChPortRemotePortCheck(devNum,portNum))
        {
            continue;
        }

        rc = cpssDxChPortPhysicalPortMapGet(devNum, portNum, 1, /*OUT*/&portMap);
        if(rc != GT_OK)
        {
            continue;
        }

        portMacNum = portMap.interfaceNum;

        if(serdesMapArr !=NULL)
        {
            if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5X_E)
            {
                if(portMacNum != 50)
                {
                    continue;
                }
            }
            else if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5P_E)
            {
                if((portMacNum % 26) != 0)
                {
                    continue;
                }
            }
            else
            {
                if(serdesMapArrSize <= (portMacNum/8))
                {
                    break;
                }
                if((portMacNum % 8) != 0)
                {
                   continue;
                }
            }
            rc = cpssDxChPortLaneMacToSerdesMuxSet(devNum, portNum,
                                                   &serdesMapArr[portMacNum / 8]);
            if (rc != GT_OK)
            {
                CPSS_APP_PLATFORM_LOG_ERR_MAC("cpssDxChPortLaneMacToSerdesMuxSet ret=%d", rc);
            }
        }
        else
        {
            if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5X_E)
            {
                if(portMacNum != 50)
                {
                    continue;
                }

                if(PRV_CPSS_PP_MAC(devNum)->devType == CPSS_98DX3550M_CNS)
                {
                   phoenixMacToSerdesMapPtr = &phoenix_RD_MacToSerdesMap[0];
                }
                else
                {
                    phoenixMacToSerdesMapPtr = &phoenix_DB_MacToSerdesMap[0];
                }
                rc = cpssDxChPortLaneMacToSerdesMuxSet(devNum, portNum,
                                                       &phoenixMacToSerdesMapPtr[0]);
            }
            else if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5P_E)
            {
                if((portMacNum % 26) != 0)
                {
                    continue;
                }
                if(PRV_CPSS_PP_MAC(devNum)->devType == CPSS_98DX4590M_CNS)
                {
                    ac5pMacToSerdesMapPtr = &ac5p_RD_MacToSerdesMap[0];
                }
                else
                {
                    ac5pMacToSerdesMapPtr = &ac5p_DB_MacToSerdesMap[0];
                }

                rc = cpssDxChPortLaneMacToSerdesMuxSet(devNum, portNum,
                        &ac5pMacToSerdesMapPtr[portMacNum/26]);
            }
            else if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_HARRIER_E)
            {
                if((portMacNum % 16) != 0)
                {
                    continue;
                }
                harrierMacToSerdesMapPtr = &aldrin3M_DB_MacToSerdesMap[0];
                rc = cpssDxChPortLaneMacToSerdesMuxSet(devNum, portNum,
                        &harrierMacToSerdesMapPtr[portMacNum/16]);
            }
            else
            {
                if(PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles == 2) /*6.4T*/
                {
                    ARR_PTR_AND_SIZE_MAC(falcon_6_4T_DB_MacToSerdesMap, falcon_MacToSerdesMap, falcon_MacToSerdesMap_arrSize);
                }
                else if(PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles == 4) /*12.8T*/
                {
                    ARR_PTR_AND_SIZE_MAC(falcon_12_8T_DB_MacToSerdesMap, falcon_MacToSerdesMap, falcon_MacToSerdesMap_arrSize);
                }
                else
                {
                    CPSS_APP_PLATFORM_LOG_INFO_MAC("serdes mux not supported\n");
                    return GT_OK;
                }

                if(falcon_MacToSerdesMap_arrSize <= (portMacNum/8))
                {
                    break;
                }
                if((portMacNum % 8) != 0)
                {
                   continue;
                }

                rc = cpssDxChPortLaneMacToSerdesMuxGet(devNum,portNum,&macToSerdesMuxStc);
                if (rc != GT_OK)
                {
                    CPSS_APP_PLATFORM_LOG_ERR_MAC("cpssDxChPortLaneMacToSerdesMuxGet ret=%d", rc);
                }

                if(cpssOsMemCmp(&macToSerdesMuxStc, &(falcon_MacToSerdesMap[portMacNum/8]), sizeof(CPSS_PORT_MAC_TO_SERDES_STC)) != 0)
                {
                    rc = cpssDxChPortLaneMacToSerdesMuxSet(devNum,portNum,&falcon_MacToSerdesMap[portMacNum / 8]);
                    if (rc != GT_OK)
                    {
                        CPSS_APP_PLATFORM_LOG_ERR_MAC("cpssDxChPortLaneMacToSerdesMuxGet ret=%d", rc);
                   }
                }

            }
        }
    }

    return rc;
}



/**
* @internal prvAppPlatformSerdesPolarityConfigSet function
* @endinternal
*
* @brief   Per DB/RD board configure the Tx/Rx Polarity values on Serdeses
*
* @note   APPLICABLE DEVICES:      Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2.
*
* @param[in] devNum                   - device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
static GT_STATUS prvAppPlatformSerdesPolarityConfigSet
(
    IN  GT_U8 devNum,
    IN  CPSS_APP_PLATFORM_SERDES_LANE_POLARITY_STC  polarityArr[],
    IN  GT_U32  polarityArrSize
)
{
    GT_STATUS                               rc;
    GT_U32                                  i;
    CPSS_APP_PLATFORM_SERDES_LANE_POLARITY_STC  *currentPolarityArrayPtr;


    CPSS_APP_PLATFORM_SERDES_LANE_POLARITY_STC ac5p_DB_PolarityArray[] =
    {
        /* laneNum  invertTx    invertRx */
         { 0,   GT_TRUE ,   GT_TRUE   }
        ,{ 1,   GT_FALSE,   GT_TRUE   }
        ,{ 2,   GT_FALSE,   GT_FALSE  }
        ,{ 3,   GT_FALSE,   GT_TRUE   }
        ,{ 4,   GT_FALSE,   GT_TRUE   }
        ,{ 5,   GT_TRUE ,   GT_FALSE  }
        ,{ 6,   GT_TRUE ,   GT_FALSE  }
        ,{ 7,   GT_TRUE ,   GT_FALSE  }
        ,{ 8,   GT_TRUE ,   GT_TRUE   }
        ,{ 9,   GT_FALSE,   GT_FALSE  }
        ,{ 10,  GT_FALSE,   GT_FALSE  }
        ,{ 11,  GT_TRUE ,   GT_TRUE   }
        ,{ 12,  GT_TRUE ,   GT_TRUE   }
        ,{ 13,  GT_FALSE,   GT_FALSE  }
        ,{ 14,  GT_FALSE,   GT_FALSE  }
        ,{ 15,  GT_FALSE,   GT_TRUE   }
        ,{ 16,  GT_TRUE ,   GT_TRUE   }
        ,{ 17,  GT_FALSE,   GT_FALSE  }
        ,{ 18,  GT_FALSE,   GT_TRUE   }
        ,{ 19,  GT_FALSE,   GT_TRUE   }
        ,{ 20,  GT_FALSE,   GT_TRUE   }
        ,{ 21,  GT_TRUE ,   GT_FALSE  }
        ,{ 22,  GT_TRUE ,   GT_TRUE   }
        ,{ 23,  GT_TRUE ,   GT_FALSE  }
        ,{ 24,  GT_FALSE,   GT_FALSE  }
        ,{ 25,  GT_FALSE,   GT_TRUE   }
        ,{ 26,  GT_FALSE,   GT_FALSE  }
        ,{ 27,  GT_FALSE,   GT_TRUE   }
        ,{ 28,  GT_FALSE,   GT_TRUE   }
        ,{ 29,  GT_FALSE,   GT_TRUE   }
        ,{ 30,  GT_FALSE,   GT_TRUE   }
        ,{ 31,  GT_FALSE,   GT_FALSE  }

    };

    CPSS_APP_PLATFORM_SERDES_LANE_POLARITY_STC ac5p_RD_PolarityArray[] =
    {
        /* laneNum  invertTx    invertRx */
          { 0,   GT_FALSE,   GT_FALSE   }
         ,{ 1,   GT_FALSE,   GT_FALSE   }
         ,{ 2,   GT_FALSE,   GT_FALSE   }
         ,{ 3,   GT_FALSE,   GT_FALSE   }
         ,{ 4,   GT_FALSE,   GT_FALSE   }
         ,{ 5,   GT_FALSE,   GT_FALSE   }
         ,{ 6,   GT_FALSE,   GT_FALSE   }
         ,{ 7,   GT_FALSE,   GT_FALSE   }
         ,{ 8,   GT_FALSE,   GT_FALSE   }
         ,{ 9,   GT_FALSE,   GT_FALSE   }
         ,{ 10,  GT_FALSE,   GT_FALSE   }
         ,{ 11,  GT_FALSE,   GT_FALSE   }
         ,{ 12,  GT_FALSE,   GT_FALSE   }
         ,{ 13,  GT_FALSE,   GT_FALSE   }
         ,{ 14,  GT_FALSE,   GT_FALSE   }
         ,{ 15,  GT_FALSE,   GT_FALSE   }
         ,{ 16,  GT_FALSE,   GT_FALSE   }
         ,{ 17,  GT_TRUE ,   GT_TRUE    }
         ,{ 18,  GT_FALSE,   GT_TRUE    }
         ,{ 19,  GT_FALSE,   GT_FALSE   }
         ,{ 20,  GT_FALSE,   GT_TRUE    }
         ,{ 21,  GT_FALSE,   GT_FALSE   }
         ,{ 22,  GT_TRUE ,   GT_TRUE    }
         ,{ 23,  GT_TRUE ,   GT_TRUE    }
         ,{ 24,  GT_TRUE ,   GT_FALSE   }
         ,{ 25,  GT_FALSE,   GT_TRUE    }
         ,{ 26,  GT_FALSE,   GT_FALSE   }
         ,{ 27,  GT_FALSE,   GT_TRUE    }
         ,{ 28,  GT_FALSE,   GT_FALSE   }
         ,{ 29,  GT_FALSE,   GT_FALSE   }
         ,{ 30,  GT_TRUE ,   GT_FALSE   }
         ,{ 31,  GT_FALSE,   GT_TRUE    }
    };


    CPSS_APP_PLATFORM_SERDES_LANE_POLARITY_STC Falcon_6_4T_DB_PolarityArray[] =
    {
    /* laneNum  invertTx    invertRx */
        { 7,    GT_TRUE,    GT_FALSE  },
        { 1,    GT_TRUE,    GT_FALSE },
        { 6,    GT_TRUE,    GT_TRUE  },
        { 0,    GT_TRUE,    GT_FALSE },
        { 8,    GT_FALSE,   GT_TRUE  },
        { 14,   GT_FALSE,   GT_FALSE },
        { 9,    GT_FALSE,   GT_FALSE  },
        { 15,   GT_FALSE,   GT_TRUE },
        { 23,   GT_FALSE,   GT_FALSE },
        { 17,   GT_FALSE,   GT_FALSE },
        { 22,   GT_TRUE,    GT_FALSE },
        { 16,   GT_FALSE,   GT_TRUE },
        { 24,   GT_FALSE,   GT_FALSE },
        { 30,   GT_FALSE,   GT_FALSE },
        { 25,   GT_FALSE,   GT_TRUE },
        { 31,   GT_FALSE,   GT_FALSE },
    };

    currentPolarityArrayPtr = polarityArr;
    if ( PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5P_E )
    {
        if(PRV_CPSS_PP_MAC(devNum)->devType == CPSS_98DX4590M_CNS)
        {
            currentPolarityArrayPtr = ac5p_RD_PolarityArray;
            polarityArrSize         = sizeof(ac5p_RD_PolarityArray)/sizeof(CPSS_APP_PLATFORM_SERDES_LANE_POLARITY_STC);
        }
        else
        {
            currentPolarityArrayPtr = ac5p_DB_PolarityArray;
            polarityArrSize         = sizeof(ac5p_DB_PolarityArray)/sizeof(CPSS_APP_PLATFORM_SERDES_LANE_POLARITY_STC);
        }
    }

    if(currentPolarityArrayPtr == NULL)
    {
        if(PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles == 2) /*6.4T WA - default ASK profile is for 12.8T*/
        {
            currentPolarityArrayPtr = Falcon_6_4T_DB_PolarityArray;
            polarityArrSize         = sizeof(Falcon_6_4T_DB_PolarityArray)/sizeof(CPSS_APP_PLATFORM_SERDES_LANE_POLARITY_STC);
        }
        else if ( PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5P_E )
        {
            if(PRV_CPSS_PP_MAC(devNum)->devType == CPSS_98DX4590M_CNS)
            {
                currentPolarityArrayPtr = ac5p_RD_PolarityArray;
                polarityArrSize         = sizeof(ac5p_RD_PolarityArray)/sizeof(CPSS_APP_PLATFORM_SERDES_LANE_POLARITY_STC);
            }
            else
            {
                currentPolarityArrayPtr = ac5p_DB_PolarityArray;
                polarityArrSize         = sizeof(ac5p_DB_PolarityArray)/sizeof(CPSS_APP_PLATFORM_SERDES_LANE_POLARITY_STC);
            }
        }
        else
        {
            return GT_OK;
        }
    }

    for (i = 0; i < polarityArrSize; i++)
    {
        rc = cpssDxChPortSerdesLanePolaritySet(devNum, 0,
                                               currentPolarityArrayPtr[i].laneNum,
                                               currentPolarityArrayPtr[i].invertTx,
                                               currentPolarityArrayPtr[i].invertRx);

        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPortSerdesLanePolaritySet);
    }

    return GT_OK;
}

/**
* @internal cpssAppPlatformPortInterfaceInit function
* @endinternal
*
* @brief   Execute predefined ports configuration.
*
* @param[in] dev                      - device number
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS cpssAppPlatformPortInterfaceInit
(
    IN  GT_U8 dev,
    IN  CPSS_APP_PLATFORM_PP_PROFILE_STC  *ppProfilePtr
)
{
    GT_STATUS   rc;                     /* return code */

#ifndef ASIC_SIMULATION
    rc = prvAppPlatformPortLaneMacToSerdesMuxSet(dev, ppProfilePtr->serdesMapPtr, ppProfilePtr->serdesMapArrSize);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, prvAppPlatformPortLaneMacToSerdesMuxSet);
#endif

    rc = prvAppPlatformSerdesPolarityConfigSet(dev, ppProfilePtr->polarityPtr, ppProfilePtr->polarityArrSize);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, prvAppPlatformSerdesPolarityConfigSet);

    return GT_OK;
}

/**
* @internal cpssAppPlatformPtpConfig function
* @endinternal
*
* @brief   PTP and TAIs related configuration.
*
* @note   APPLICABLE DEVICES:      Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2.
*
* @param[in] devNum                   - physical device number
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong port number or device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS cpssAppPlatformPtpConfig
(
    IN GT_U8 devNum
)
{
    CPSS_DXCH_PTP_TAI_ID_STC        taiId;      /* TAI Units identification */
    CPSS_DXCH_PTP_TAI_TOD_STEP_STC  todStep;    /* TOD Step */
    GT_U32                          regAddr;    /* register address */
    GT_U32                          regData = 0;/* register value*/
    GT_U32                          pll4Frq = 1;/* PLL4 frequency (in khz) */
    GT_STATUS                       rc;         /* return code */
    GT_PHYSICAL_PORT_NUM            portNum;    /* port number */
    CPSS_DXCH_PTP_TSU_CONTROL_STC   control;    /* control structure */
    GT_U32                          ptpClkInKhz; /* PTP clock in KHz */
    GT_U32                          isPtpMgr=0;  /* whether system working mode is in PTP Manager mode or Legacy mode*/
    GT_U32                          taiNumber;
    GT_U32                          numOfPhysicalPorts;

    osMemSet(&control,0,sizeof(CPSS_DXCH_PTP_TSU_CONTROL_STC));

    if (PRV_CPSS_SIP_6_CHECK_MAC(devNum))
    {
        rc = appPlatformDbEntryGet("ptpMgr", &isPtpMgr);
        if(rc != GT_OK)
        {
            isPtpMgr = 0;
        }

        /* all the init sequence is done using PTP Manager */
        if (isPtpMgr == 1)
            return GT_OK;
    }

    if (PRV_CPSS_SIP_5_16_CHECK_MAC(devNum))
    {
        ptpClkInKhz = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.ptpInfo.ptpClkInKhz;
    }
    else
    {
        /* Get TAIs reference clock (its half of PLL4 frequency) */
        if(PRV_CPSS_DEV_DFX_SERVER_SUPPORTED_MAC(devNum) != GT_FALSE)
        {
            regAddr =
                PRV_CPSS_DEV_RESET_AND_INIT_CONTROLLER_REGS_MAC(devNum)->
                                        DFXServerUnitsDeviceSpecificRegs.deviceSAR2;
            rc = prvCpssDrvHwPpResetAndInitControllerGetRegField(devNum,
                                                             regAddr, 12, 2,
                                                             &regData);
            if(rc != GT_OK)
            {
                return rc;
            }
        }

        if(PRV_CPSS_DXCH_BOBCAT2_A0_CHECK_MAC(devNum))
        {
            switch(regData)
            {
                case 0: pll4Frq = PLL4_FREQUENCY_1250000_KHZ_CNS;
                        break;
                case 1: pll4Frq = PLL4_FREQUENCY_1093750_KHZ_CNS;
                        break;
                case 2: pll4Frq = PLL4_FREQUENCY_1550000_KHZ_CNS;
                        break;
                default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
            }
            ptpClkInKhz = pll4Frq / 2;
        }
        else
        {
            switch(regData)
            {
                case 0: ptpClkInKhz = 500000; /* 500 MHz */
                        break;
                case 1: ptpClkInKhz = 546875; /* 546.875MHz*/
                        break;
                default: CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
            }
        }
    }

    /* Configure TAIs nanosec step values */
    taiId.taiInstance = CPSS_DXCH_PTP_TAI_INSTANCE_ALL_E;
    taiId.taiNumber = CPSS_DXCH_PTP_TAI_NUMBER_ALL_E;
    todStep.nanoSeconds = 1000000/ptpClkInKhz;
    if (1000000.0/ptpClkInKhz - todStep.nanoSeconds > 0.5)
    {
        todStep.nanoSeconds++;
        /* NOTE: the casting to GT_32 is to solve known ARM issue: casting a negative float to an unsigned int returns '0' */
        todStep.fracNanoSeconds = (GT_32)((1000000.0/ptpClkInKhz - todStep.nanoSeconds) * 0x100000000);

        if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_IRONMAN_E)
        {
            /* need to adjust fracNanoSec from 0xAAAAAAAB as the 'Drift compensation mechanism' is calculated based on 0xAAAAAAAA */
            todStep.fracNanoSeconds = 0xAAAAAAAA;
        }
    }
    else
    {
        /* NOTE: the casting to GT_U32 is o.k. when casting a positive float */
        todStep.fracNanoSeconds = (GT_U32)(0xFFFFFFFF *
                (1000000.0/ptpClkInKhz - todStep.nanoSeconds) +
                (1000000.0/ptpClkInKhz - todStep.nanoSeconds));
    }

    rc = cpssDxChPtpTaiTodStepSet(devNum, &taiId, &todStep);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPtpTaiTodStepSet);

    numOfPhysicalPorts =
          PRV_CPSS_DXCH_MAX_PHY_PORT_NUMBER_MAC(devNum);

    for(portNum = 0; portNum < numOfPhysicalPorts; portNum++)
    {
        CPSS_APPLICATION_PORT_SKIP_CHECK(devNum, portNum);

        if(prvCpssDxChPortRemotePortCheck(devNum,portNum))
        {
            continue;
        }

        /*CPSS_TBD_BOOKMASK_AAS: kalex: remove after initial bring up*/
        if(cpssDeviceRunCheck_onEmulator() && (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AAS_E))
        {
            /*do nothing*/
        }
        else
        {
            rc = cpssDxChPtpTsuControlGet(devNum, portNum, &control);
            if(rc != GT_OK)
            {
                return rc;
            }
        }

        if(control.unitEnable == GT_FALSE)
        {
            control.unitEnable = GT_TRUE;
            /* Timestamping unit enable */
            /*CPSS_TBD_BOOKMASK_AAS: kalex: remove after initial bring up*/
            if(cpssDeviceRunCheck_onEmulator() && (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AAS_E))
            {
                /*do nothing*/
            }
            else
            {
                rc = cpssDxChPtpTsuControlSet(devNum, portNum, &control);
                if(rc != GT_OK)
                {
                    return rc;
                }
            }
        }
    }

    /* Hawk,Harrier : Configures input TAI clock's selection to PTP PLL;
       Others use Core Clock for TAIs */
    if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5P_E ||
        PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_HARRIER_E)
    {
        /* loop on TAIs */
        for (taiNumber = 0; taiNumber < PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.ptpInfo.maxNumOfTais ;taiNumber++)
        {
            rc = cpssDxChPtpTaiInputClockSelectSet(devNum,taiNumber,CPSS_DXCH_PTP_TAI_CLOCK_SELECT_PTP_PLL_E,CPSS_DXCH_PTP_25_FREQ_E);
            if(rc != GT_OK)
            {
                return rc;
            }
        }
    }

    /* TOD step resolution is not fine enough to give a presice TOD. Need to configure TAI Drift compensation patch. */
    if (PRV_CPSS_PP_MAC(devNum)->devFamily == CPSS_PP_FAMILY_DXCH_IRONMAN_E)
    {
        rc = prvCpssDxChPtpTaiDriftCompensationControlConfig(devNum);
        if(rc != GT_OK)
        {
            return rc;
        }
    }
    return GT_OK;
}


/**
* @internal cpssAppPlatformAfterInitDeviceConfig function
* @endinternal
*
* @brief   Device specific configurations that should be done after device
*         initialization.
* @param[in] boardRevId               - The board revision Id.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS cpssAppPlatformAfterInitDeviceConfig
(
    IN  GT_U8   devNum,
    IN  CPSS_APP_PLATFORM_PP_PROFILE_STC  *ppProfilePtr
)
{
    GT_STATUS rc = GT_OK, rc1 = GT_OK;
    CPSS_PORTS_BMP_STC  portsMembers; /* VLAN members */
    CPSS_PORTS_BMP_STC  portsTagging; /* VLAN tagging */
    CPSS_DXCH_BRG_VLAN_INFO_STC  cpssVlanInfo;   /* cpss vlan info format    */
    CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC portsTaggingCmd; /* ports tagging command */
    CPSS_PORTS_BMP_STC  portsAdditionalMembers; /* VLAN members to add */
    GT_BOOL             isValid; /* is Valid flag */
    CPSS_SYSTEM_RECOVERY_INFO_STC system_recovery; /* holds system recovery information */
    CPSS_SYSTEM_RECOVERY_INFO_STC saved_system_recovery;
    CPSS_SYSTEM_RECOVERY_INFO_STC tmp_system_recovery;
    cpssOsMemSet(&tmp_system_recovery,0,sizeof(CPSS_SYSTEM_RECOVERY_INFO_STC));
    cpssOsMemSet(&system_recovery,0,sizeof(CPSS_SYSTEM_RECOVERY_INFO_STC));
    cpssOsMemSet(&saved_system_recovery,0,sizeof(CPSS_SYSTEM_RECOVERY_INFO_STC));

    rc = cpssSystemRecoveryStateGet(&system_recovery);
    if (rc != GT_OK)
    {
        return rc;
    }

    if (CPSS_SYSTEM_RECOVERY_PROCESS_NOT_ACTIVE_E != system_recovery.systemRecoveryProcess)
    {
        tmp_system_recovery.systemRecoveryProcess = CPSS_SYSTEM_RECOVERY_PROCESS_HA_E;
        tmp_system_recovery.systemRecoveryState = CPSS_SYSTEM_RECOVERY_INIT_STATE_E;
        tmp_system_recovery.systemRecoveryMode = system_recovery.systemRecoveryMode;
        rc =  cpssSystemRecoveryStateSet(&tmp_system_recovery);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    /* PIP  not supported on Ironman*/
    if (PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_IRONMAN_E)
    {
        rc = cpssAppPlatformPIPEngineInit(devNum);
        if(rc != GT_OK)
        {
            rc =  cpssSystemRecoveryStateSet(&system_recovery);
            if (rc != GT_OK)
            {
                return rc;
            }
            return rc;
        }
    }

    /* After all CG MAC related configuration were done,
        it is needed to disable GC MAC UNIT in order to reduce power consumption.
        Only requested CG MACs will be enabled during appDemoBc2PortInterfaceInit */

    /* Bobcat3 CG MAC unit disable */
    rc = cpssSystemRecoveryStateGet(&tmp_system_recovery);
    if (rc != GT_OK)
    {
        return rc;
    }
    saved_system_recovery = tmp_system_recovery;

    tmp_system_recovery.systemRecoveryProcess = CPSS_SYSTEM_RECOVERY_PROCESS_NOT_ACTIVE_E;
    tmp_system_recovery.systemRecoveryState = CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E;
    rc =  cpssSystemRecoveryStateSet(&tmp_system_recovery);
    if (rc != GT_OK)
    {
        return rc;
    }

    rc = prvCpssDxChCgMacUnitDisable(devNum, GT_FALSE);

    rc1 =  cpssSystemRecoveryStateSet(&saved_system_recovery);
    if (rc1 != GT_OK)
    {
        return rc1;
    }
    if(GT_OK != rc)
    {
        return rc;
    }

    rc = cpssAppPlatformPortInterfaceInit(devNum, ppProfilePtr);
    if(rc != GT_OK)
    {
        rc1 =  cpssSystemRecoveryStateSet(&system_recovery);
        if (rc1 != GT_OK)
        {
            return rc1;
        }
        return rc;
    }

    if (CPSS_SYSTEM_RECOVERY_PROCESS_NOT_ACTIVE_E != system_recovery.systemRecoveryProcess)
    {
        rc =  cpssSystemRecoveryStateSet(&system_recovery);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    /* clear additional default VLAN members */
    PRV_CPSS_PORTS_BMP_PORT_CLEAR_ALL_MAC(&portsAdditionalMembers);

    if(PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.gop_ilkn.supported == GT_TRUE)
    {
        GT_PHYSICAL_PORT_NUM    chId;

        /* add ILKN channels to default VLAN, although they are not mapped yet */
        for(chId = 128; chId < 192; chId++)
        {
            /* set the port as member of vlan */
            CPSS_PORTS_BMP_PORT_SET_MAC(&portsAdditionalMembers, chId);
        }
    }

    rc = cpssSystemRecoveryStateGet(&system_recovery);
    if (rc != GT_OK)
    {
        return rc;
    }
    if (system_recovery.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_NOT_ACTIVE_E)
    {
        /* What is spl with port 83? */
        /* add port 83 to default VLAN, although it could be not mapped */
        CPSS_PORTS_BMP_PORT_SET_MAC(&portsAdditionalMembers, 83);

        /* read VLAN entry */
        rc = cpssDxChBrgVlanEntryRead(devNum, 1, &portsMembers, &portsTagging,
                                        &cpssVlanInfo, &isValid, &portsTaggingCmd);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChBrgVlanEntryRead);

        /* add new ports as members, portsTaggingCmd is default - untagged */
        CPSS_PORTS_BMP_BITWISE_OR_MAC(&portsMembers, &portsMembers, &portsAdditionalMembers);

        /* Write default VLAN entry */
        rc = cpssDxChBrgVlanEntryWrite(devNum, 1,
                                        &portsMembers,
                                        &portsTagging,
                                        &cpssVlanInfo,
                                        &portsTaggingCmd);

        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChBrgVlanEntryWrite);
    }

    /* PTP (and TAIs) configurations */
    if (system_recovery.systemRecoveryProcess != CPSS_SYSTEM_RECOVERY_PROCESS_FAST_BOOT_E &&
            (PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_AC5_E))
    {
        /* PTP (and TAIs) configurations */
        rc = cpssAppPlatformPtpConfig(devNum);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, prvBobcat2PtpConfig);
    }

    CPSS_APP_PLATFORM_LOG_INFO_MAC("After Device Init done...\n");

    return GT_OK;
}

/**
* @internal cpssAppPlatformAfterInitConfig function
* @endinternal
*
* @brief   After init configurations of PP device.
*
* @param[in] devNum                - Device Number.
* @param [in] *ppProfileInfo     - PP profile
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS cpssAppPlatformAfterInitConfig
(
    IN GT_U8                              devNum,
    IN CPSS_APP_PLATFORM_PP_PROFILE_STC  *ppProfilePtr
)
{
    GT_STATUS               rc                   = GT_OK;
    GT_U32                  portNum, maxPortNum;
    CPSS_PORT_MANAGER_STC   portEventStc;

    rc = cpssAppPlatformAfterInitDeviceConfig(devNum, ppProfilePtr);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssAppPlatformAfterInitDeviceConfig);

    rc = cpssDxChCfgDevEnable(devNum, GT_TRUE);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChCfgDevEnable);

    for (portNum = 0, maxPortNum = PRV_APP_REF_PP_CONFIG_VAR(cpssCapMaxPortNum)[devNum]; portNum < maxPortNum; portNum++)
    {
        if (PRV_CPSS_PP_MAC(devNum)->devFamily != CPSS_PP_FAMILY_DXCH_AC5_E)
        {
            CPSS_APPLICATION_PORT_SKIP_CHECK(devNum, portNum);
        }

        portEventStc.portEvent = CPSS_PORT_MANAGER_EVENT_UNFORCE_LINK_DOWN_E;
        rc = cpssDxChPortManagerEventSet(devNum, portNum, &portEventStc);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPortManagerEventSet);

        rc = cpssDxChNstPortEgressFrwFilterSet(devNum, portNum, CPSS_NST_EGRESS_FRW_FILTER_FROM_CPU_E, GT_FALSE);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChNstPortEgressFrwFilterSet);

        rc = cpssDxChNstPortIngressFrwFilterSet(devNum, portNum, CPSS_NST_INGRESS_FRW_FILTER_TO_CPU_E, GT_FALSE);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChNstPortIngressFrwFilterSet);

        rc =  cpssDxChBrgFdbNaToCpuPerPortSet(devNum,portNum,GT_TRUE);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChBrgFdbNaToCpuPerPortSet);

        rc =  cpssDxChBrgFdbNaStormPreventSet(devNum,portNum,GT_TRUE);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChBrgFdbNaStormPreventSet);
    }

    if(PRV_CPSS_SIP_5_CHECK_MAC(devNum) == GT_TRUE)
    {
        GT_U32  regAddr;
        regAddr = PRV_DXCH_REG1_UNIT_FDB_MAC(devNum).FDBCore.FDBGlobalConfig.FDBGlobalConfig;
        rc = prvCpssHwPpSetRegField(devNum, regAddr, 30, 1, 0);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    return rc;
}

static GT_VOID findMinMaxPositionByLedIf(GT_U32 ledIf,
                                            CPSS_APP_PLATFORM_MAC_LEDPOSITION_STC *ledPositionTbl,
                                            GT_U32 *minPositionPtr,
                                            GT_U32 *maxPositionPtr)
{
    GT_U32 i;
    GT_U32  minPosition = (GT_U32) (-1);
    GT_U32  maxPosition = 0;

    for (i = 0 ; ledPositionTbl[i].ledIf !=  APP_PLATFORM_BAD_VALUE; i++)
    {
        if (ledPositionTbl[i].ledIf == ledIf)
        {
            if (ledPositionTbl[i].ledPosition < minPosition)
            {
                minPosition = ledPositionTbl[i].ledPosition;
            }
            if (ledPositionTbl[i].ledPosition > maxPosition)
            {
                maxPosition = ledPositionTbl[i].ledPosition;
            }
        }
    }

    *minPositionPtr = minPosition;
    *maxPositionPtr = maxPosition;
}

static GT_U32 findLedPositionByMac(GT_U32 mac, CPSS_APP_PLATFORM_MAC_LEDPOSITION_STC *ledPositionTbl)
{
    GT_U32 i;
    for (i = 0 ; ledPositionTbl[i].portMac !=  APP_PLATFORM_BAD_VALUE; i++)
    {
        if (ledPositionTbl[i].portMac == mac)
        {
            return ledPositionTbl[i].ledPosition;
        }
    }
    return APP_PLATFORM_BAD_VALUE;
}

/************Falcon LED config*************/
#ifndef GM_USED
/*
 * typedef: struct CPSS_FALCON_LED_STREAM_INDICATIONS_STC
 *
 * Description:
 *      Positions of LED bit indications in stream.
 *
 * Fields:
 *
 *      ledStart            - The first bit in the LED stream indication to be driven in current chiplet
 *                            (APPLICABLE RANGES: 0..255)
 *      ledEnd              - The last bit in the LED stream indication to be driven in the current chiplet
 *                            (APPLICABLE RANGES: 0..255)
 *      cpuPort             - CPU port is connected to the current chiplet
 */
typedef struct {
    GT_U32     ledStart;
    GT_U32     ledEnd;
    GT_BOOL    cpuPort;
} CPSS_FALCON_LED_STREAM_INDICATIONS_STC;

typedef struct {
    /** The first bit in the LED stream indication to be driven in current LED unit. (APPLICABLE RANGES: 0..255) */
    GT_U32     ledStart;
    /** The last bit in the LED stream indication to be driven in the current LED unit. (APPLICABLE RANGES: 0..255) */
    GT_U32     ledEnd;
} PRV_CPSS_FALCON_LED_STREAM_INDICATIONS_STC;


/**
 * @internal led_port_position_get_mac_ironman
 * @endinternal
 *
 * @brief   Gets LED ports position for the current MAC port
 *
 * @param[in] portMacNum            - current MAC to check
 *
 * @retval GT_U32                   - the LED ports position for the current MAC port
 *
 */
static GT_U32 led_port_position_get_mac_ironman
(
 IN  GT_U32 portMacNum
)
{
    GT_U32 ledPosition = 0;
    const CPSS_IRONMAN_LED_PORT_MAC_POSITION_STC * ironmanPortsMapArrPtr;
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
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise
*
*/
static GT_STATUS ironman_LedInit
(
    IN  GT_U8    devNum
)
{
    GT_STATUS                       rc;
    GT_U32                          classNum;
    GT_PHYSICAL_PORT_NUM            portNum;
    CPSS_LED_CONF_STC               ledConfig;
    CPSS_LED_CLASS_MANIPULATION_STC ledClassManip;
    GT_U32                          position;
    GT_U32                          ledUnit;
    GT_U32                          numOfPhysicalPorts;
    CPSS_DXCH_PORT_MAP_STC          portMap;
    const CPSS_IRONMAN_LED_STREAM_INDICATIONS_STC * ledStreamIndication;
    GT_U32                          reorderMapArraySize;
    GT_U32                          ledSerialOutputBitMapArr[CPSS_APP_PLATFORM_LED_PORT_REORDER_MAP_SIZE_CNS] =
                                                              /* 0  1  2  3  4  5  6  7  8  9  10  11  12 */
                                                                {6, 3, 0, 1, 4, 7, 8, 2, 5, 9, 10, 11, 12};

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

    ledStreamIndication = ironman_led_indications;

    for(ledUnit = 0; ledUnit < PRV_CPSS_DXCH_PP_HW_INFO_LED_MAC(devNum).sip6LedUnitNum; ledUnit++)
    {
        /* Start of LED stream location - Class 2 */
        ledConfig.sip6LedConfig.ledStart[ledUnit] = ledStreamIndication[ledUnit].ledStart;    /* LedControl.LedStart[5:12] */
        /* End of LED stream location */
        ledConfig.sip6LedConfig.ledEnd[ledUnit] = ledStreamIndication[ledUnit].ledEnd;        /* LedControl.LedEnd[5:12] */
        /* LED unit may be bypassed in the LED chain */
        ledConfig.sip6LedConfig.ledChainBypass[ledUnit] = ledStreamIndication[ledUnit].cpuPort;
    }

    rc = cpssDxChLedStreamConfigSet(devNum, 0, &ledConfig);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChLedStreamConfigSet);

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
            CPSS_DXCH_LED_PORT_TYPE_XG_E /* don't care , not applicable for Ironman */
            ,classNum ,&ledClassManip);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChLedStreamClassManipulationSet);
    /*--------------------------------------------------------------------------------------------------------*
      * LED. classesAndGroupConfig uses default setting , don't configure   cpssDxChLedStreamGroupConfigSet()  *
      *--------------------------------------------------------------------------------------------------------*
      * now configure port dependent LED configurations                                                        *
      *       - port led position (stream is hard wired)                                                       *
      *           The port LED positions for full chip is as follows:                                          *
      *                                                                                                        *
      *           P0,    LED Server 0,  Position  0   Ports  0- 7                                              *
      *           P1,    LED Server 0,  Position  8   Ports 24-31                                              *
      *           P2,    LED Server 0,  Position 16   Port  52                                                 *
      *           P3,    LED Server 1,  Position  0   Ports  8-15                                              *
      *           P4,    LED Server 1,  Position  8   Ports 32-39                                              *
      *           P5,    LED Server 1,  Position 16   Port  53                                                 *
      *           P6,    LED Server 2,  Position  0   Ports 16-23                                              *
      *           P7,    LED Server 2,  Position  8   Ports 40-47                                              *
      *           P8,    LED Server 2,  Position 16   Port  54                                                 *
      *           P9,    LED Server 3,  Position  0   Port  48                                                 *
      *           P10,   LED Server 3,  Position  1   Port  49                                                 *
      *           P11,   LED Server 3,  Position  2   Port  50                                                 *
      *           P12,   LED Server 3,  Position  3   Port  51                                                 *
      *                                                                                                        *
      *--------------------------------------------------------------------------------------------------------*/
    numOfPhysicalPorts = PRV_CPSS_DXCH_MAX_PHY_PORT_NUMBER_MAC(devNum);

    for(portNum = 0; portNum < numOfPhysicalPorts; portNum++)
    {
        CPSS_ENABLER_PORT_SKIP_CHECK(devNum, portNum);

        if (prvCpssDxChPortRemotePortCheck(devNum,portNum))
        {
            continue;
        }

        rc = cpssDxChPortPhysicalPortMapGet(devNum, portNum, 1, /*OUT*/&portMap);
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChPortPhysicalPortMapGet);

        position = led_port_position_get_mac_ironman(portMap.interfaceNum);

        if (position != CAP_BAD_VALUE)
        {
            rc = cpssDxChLedStreamPortPositionSet(devNum, portNum, position);
            CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChLedStreamPortPositionSet);
        }
    }

    reorderMapArraySize = CPSS_APP_PLATFORM_LED_PORT_REORDER_MAP_SIZE_CNS;
    rc = cpssDxChLedStreamReorderMapSet(devNum, GT_TRUE, reorderMapArraySize, ledSerialOutputBitMapArr);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, cpssDxChLedStreamReorderMapSet);

    return rc;
}


/**
* @internal falconLedInit function
* @endinternal
*
* @brief   LED configurations
*
* @param[in] devNum                - The CPSS devNum.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*
*/
static GT_STATUS falcon_LedInit
(
    GT_U8    devNum,
    IN CPSS_APP_PLATFORM_PP_PROFILE_STC *ppProfilePtr
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
    GT_U32                          numOfPhysicalPorts;
    PRV_CPSS_FALCON_LED_STREAM_INDICATIONS_STC
        *ledStreamIndication;
    CPSS_APP_PLATFORM_MAC_LEDPOSITION_STC
        *ledPositionTbl;
    CPSS_SYSTEM_RECOVERY_INFO_STC   system_recovery; /* holds system recovery information */

    cpssOsMemSet(&ledConfig, 0, sizeof(CPSS_LED_CONF_STC));

    if(ppProfilePtr->ledPositionTable != NULL)
    {
        ledPositionTbl = ppProfilePtr->ledPositionTable;
    }
    else
    {
        CPSS_APP_PLATFORM_LOG_INFO_MAC("LED Init - Skipping: LED Table Not Found\n");
        return GT_OK;
    }

    ledConfig.ledOrganize                       = ppProfilePtr->ledOrganize;                       /* LedControl.OrganizeMode     [ 1: 1] =   1  by class       */
    ledConfig.sip6LedConfig.ledClockFrequency   = ppProfilePtr->sip6LedClockFrequency;                                                 /* LedChainClockControl.ledClkOutDiv  [11: 22] = 512 (app_clock - 833333/led_clock_out - 1627)  */
    ledConfig.pulseStretch                      = ppProfilePtr->pulseStretch;                           /* LedControl.Pulse-stretch-div[ 0: 29] =  100000 (ledClockFrequencyDivider - 10 * 10000  */

    ledConfig.blink0Duration                    = ppProfilePtr->blink0Duration;    /* 64 ms */                     /* BlinkDivision0.blink0Duration  [ 0: 29]  = 0x32DCD40 (64/app_clock in mSec)*/
    ledConfig.blink0DutyCycle                   = ppProfilePtr->blink0DutyCycle;  /* 50%   */                     /* blinkGlobalControl.Blink0-Duty [ 3: 4]  = 1 25%  */
    ledConfig.blink1Duration                    = ppProfilePtr->blink1Duration;    /* 64 ms */                     /* BlinkDivision1.blink0Duration  [ 0: 29]  = 0x32DCD40 (64/app_clock in mSec)*/
    ledConfig.blink1DutyCycle                   = ppProfilePtr->blink1DutyCycle;  /* 50%   */                     /* blinkGlobalControl.Blink1-Duty [ 8: 9]  = 1 25%  */

    ledConfig.sip6LedConfig.ledClockPosEdgeModeEnable  = ppProfilePtr->ledClockPosEdgeModeEnable;
    ledConfig.disableOnLinkDown                 = ppProfilePtr->disableOnLinkDown;  /* don't care , see led-class manipulation */
    ledConfig.clkInvert                         = ppProfilePtr->clkInvert;  /* don't care */
    ledConfig.class5select                      = ppProfilePtr->class5select;  /* don't care */
    ledConfig.class13select                     = ppProfilePtr->class13select; /* don't care */

    rc = cpssSystemRecoveryStateGet(&system_recovery);
    if (rc != GT_OK)
    {
        return rc;
    }

    ledStreamIndication = (PRV_CPSS_FALCON_LED_STREAM_INDICATIONS_STC *)PRV_APP_REF_PP_UTILS_VAR(sip6_led_indications_ezb);

    for(ledUnit = 0; ledUnit < PRV_CPSS_DXCH_PP_HW_INFO_LED_MAC(devNum).sip6LedUnitNum; ledUnit++)
    {
        /* Start of LED stream location - Class 2 */
        ledConfig.sip6LedConfig.ledStart[ledUnit] = ledStreamIndication[ledUnit].ledStart;    /* LedControl.LedStart[5:12] */
        /* End of LED stream location: LED ports 0, 1 or 0 */
        ledConfig.sip6LedConfig.ledEnd[ledUnit] = ledStreamIndication[ledUnit].ledEnd;        /* LedControl.LedEnd[5:12] */
        /* The LED unit is included into LED chain */
        ledConfig.sip6LedConfig.ledChainBypass[ledUnit] = GT_FALSE;
    }

    rc = cpssDxChLedStreamConfigSet(devNum, 0, &ledConfig);
    if(rc != GT_OK)
    {
        return rc;
    }

    ledClassManip.invertEnable            = ppProfilePtr->invertionEnable; /* not relevant for phoenix */
    ledClassManip.blinkSelect             = ppProfilePtr->blinkSelect;                            /* blinkGlobalControl.Blink Sel            [10-17] = 0 blink-0         */
    ledClassManip.forceEnable             = ppProfilePtr->forceEnable;                                             /* classesAndGroupConfig.Force En          [24-29] = pos = classNum = 0*/
    ledClassManip.forceData               = ppProfilePtr->forceData;                                                    /* classForcedData[class]                  [ 0:31] = 0                 */
    ledClassManip.pulseStretchEnable      = ppProfilePtr->pulseStretchEnable;                                             /* classGlobalControl.stretch class        [ 6-11] = pos = classNum = 0/1*/

    for (classNum = 0 ; classNum < 6; classNum++)
    {
        if(classNum == 2)
        {
            ledClassManip.disableOnLinkDown       = GT_TRUE;                                  /* classGlobalControl.disable on link down [ 0- 5] = pos = classNum = 0*/
            ledClassManip.blinkEnable             = GT_TRUE;                                  /* blinkGlobalControl.blinkEn     [18-25] = pos = classNum val = 1 */
        }                                                                                     /* blinkGlobalControl.Blink Select[10-17] = pos = classNum val = 0 */
        else
        {
            ledClassManip.disableOnLinkDown       = GT_FALSE;
            ledClassManip.blinkEnable             = GT_FALSE;
        }
        rc = cpssDxChLedStreamClassManipulationSet(devNum, 0,
                CPSS_DXCH_LED_PORT_TYPE_XG_E /* don't care , not applicable for phoenix */
                ,classNum ,&ledClassManip);
        if(rc != GT_OK)
        {
            return rc;
        }
    }
    /*--------------------------------------------------------------------------------------------------------*
     * LED. classesAndGroupConfig uses default setting , don't configure   cpssDxChLedStreamGroupConfigSet()  *
     *--------------------------------------------------------------------------------------------------------*
     * now configure port dependent LED configurations                                                        *
     *       - port led position (stream is hard wired)                                                       *
     *           The port LED positions for full chip is as follows:                                          *
     *                                                                                                        *
     *           P0,    LED Server 0,  Position 0   Ports  0.. 7                                              *
     *           P1,    LED Server 0,  Position 8   Ports  8..15                                              *
     *           P2,    LED Server 1,  Position 0   Ports 16..23                                              *
     *           P3,    LED Server 1,  Position 8   Ports 24..32                                              *
     *           P4,    LED Server 2,  Position 0   Ports 32..39                                              *
     *           P5,    LED Server 2,  Position 8   Ports 40..47                                              *
     *           P6,    LED Server 3,  Position 0   Port  48                                                  *
     *           P7,    LED Server 3,  Position 1   Port  49                                                  *
     *           P8,    LED Server 4,  Position 0   Ports 50..53                                              *
     *                                                                                                        *
     *--------------------------------------------------------------------------------------------------------*/
    numOfPhysicalPorts =
        PRV_CPSS_DXCH_MAX_PHY_PORT_NUMBER_MAC(devNum);
    for(portNum = 0; portNum < numOfPhysicalPorts; portNum++)
    {
        CPSS_APPLICATION_PORT_SKIP_CHECK(devNum, portNum);

        if (prvCpssDxChPortRemotePortCheck(devNum,portNum))
        {
            continue;
        }

        rc = cpssDxChPortPhysicalPortMapGet(devNum, portNum, 1, /*OUT*/&portMap);
        if(rc != GT_OK)
        {
            continue;
        }

        position = findLedPositionByMac(portMap.interfaceNum, ledPositionTbl);
        if (position != APP_PLATFORM_BAD_VALUE)
        {
            rc = cpssDxChLedStreamPortPositionSet(devNum, portNum, position);
            if(rc != GT_OK)
            {
                return rc;
            }
            /* Skip if in recovery mode HA */
            if (system_recovery.systemRecoveryProcess != CPSS_SYSTEM_RECOVERY_PROCESS_HA_E)
            {
                rc = cpssDxChLedStreamPortClassPolarityInvertEnableSet(devNum,portNum,/*classNum*/2,/*invertEnable*/1);
                if(rc != GT_OK)
                {
                    return rc;
                }
            }
        }
    }

    return GT_OK;
}
#endif
/************Falcon LED config*************/

/************AC5x LED config*************/
/** @struct PRV_CPSS_AC5X_LED_STREAM_INDICATIONS_STC
 *
 *   @brief Positions of LED bit indications in stream.
 *
 */
typedef struct {
    /** The first bit in the LED stream indication to be driven in current LED unit. (APPLICABLE RANGES: 0..255) */
    GT_U32     ledStart;
    /** The last bit in the LED stream indication to be driven in the current LED unit. (APPLICABLE RANGES: 0..255) */
    GT_U32     ledEnd;
} PRV_CPSS_AC5X_LED_STREAM_INDICATIONS_STC;



static GT_STATUS phoenix_LedInit
(
    IN GT_U8                             devNum,
    IN CPSS_APP_PLATFORM_PP_PROFILE_STC *ppProfilePtr
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
    GT_U32                          numOfPhysicalPorts;
    PRV_CPSS_AC5X_LED_STREAM_INDICATIONS_STC
        *ledStreamIndication;
    CPSS_APP_PLATFORM_MAC_LEDPOSITION_STC
        *ledPositionTbl;
    CPSS_SYSTEM_RECOVERY_INFO_STC   system_recovery; /* holds system recovery information */

    PRV_CPSS_AC5X_LED_STREAM_INDICATIONS_STC ac5x_led_indications[CPSS_LED_UNITS_MAX_NUM_CNS] =
    {
        {64, 79},   /* LED Unit 0 - Ports  0-15 */
        {64, 79},   /* LED Unit 1 - Ports 16-31 */
        {64, 79},   /* LED Unit 2 - Ports 32-47 */
        {64, 65},   /* LED Unit 3 - Ports 48-49 */
        {64, 67}    /* LED Unit 4 - Ports 50-53 */
    };

    PRV_CPSS_AC5X_LED_STREAM_INDICATIONS_STC ac5x_RD_led_indications[CPSS_LED_UNITS_MAX_NUM_CNS] =
    {
        { 0,  0},   /* LED Unit 0 - Excluded from LED stream */
        { 0,  0},   /* LED Unit 1 - Excluded from LED stream */
        { 0,  0},   /* LED Unit 2 - Excluded from LED stream */
        {64, 65},   /* LED Unit 3 - Ports 48-49 */
        {64, 67}    /* LED Unit 4 - Ports 50-53 */
    };

    cpssOsMemSet(&ledConfig, 0, sizeof(CPSS_LED_CONF_STC));

    if(ppProfilePtr->ledPositionTable != NULL)
    {
        ledPositionTbl = ppProfilePtr->ledPositionTable;
    }
    else
    {
        CPSS_APP_PLATFORM_LOG_INFO_MAC("LED Init - Skipping: LED Table Not Found\n");
        return GT_OK;
    }

    ledConfig.ledOrganize                       = ppProfilePtr->ledOrganize;                       /* LedControl.OrganizeMode     [ 1: 1] =   1  by class       */
    ledConfig.sip6LedConfig.ledClockFrequency   = ppProfilePtr->sip6LedClockFrequency;                                                 /* LedChainClockControl.ledClkOutDiv  [11: 22] = 512 (app_clock - 833333/led_clock_out - 1627)  */
    ledConfig.pulseStretch                      = ppProfilePtr->pulseStretch;                           /* LedControl.Pulse-stretch-div[ 0: 29] =  100000 (ledClockFrequencyDivider - 10 * 10000  */

    ledConfig.blink0Duration          = ppProfilePtr->blink0Duration;    /* 64 ms */                     /* BlinkDivision0.blink0Duration  [ 0: 29]  = 0x32DCD40 (64/app_clock in mSec)*/
    ledConfig.blink0DutyCycle         = ppProfilePtr->blink0DutyCycle;  /* 50%   */                     /* blinkGlobalControl.Blink0-Duty [ 3: 4]  = 1 25%  */
    ledConfig.blink1Duration          = ppProfilePtr->blink1Duration;    /* 64 ms */                     /* BlinkDivision1.blink0Duration  [ 0: 29]  = 0x32DCD40 (64/app_clock in mSec)*/
    ledConfig.blink1DutyCycle         = ppProfilePtr->blink1DutyCycle;  /* 50%   */                     /* blinkGlobalControl.Blink1-Duty [ 8: 9]  = 1 25%  */

    ledConfig.sip6LedConfig.ledClockPosEdgeModeEnable  = ppProfilePtr->ledClockPosEdgeModeEnable;
    ledConfig.disableOnLinkDown       = ppProfilePtr->disableOnLinkDown;  /* don't care , see led-class manipulation */
    ledConfig.clkInvert               = ppProfilePtr->clkInvert;  /* don't care */
    ledConfig.class5select            = ppProfilePtr->class5select;  /* don't care */
    ledConfig.class13select           = ppProfilePtr->class13select; /* don't care */

    rc = cpssSystemRecoveryStateGet(&system_recovery);
    if (rc != GT_OK)
    {
        return rc;
    }

    if(GT_TRUE == ppProfilePtr->isEzbIsXmlLoaded)
    {

        ledStreamIndication = (PRV_CPSS_AC5X_LED_STREAM_INDICATIONS_STC *)PRV_APP_REF_PP_UTILS_VAR(sip6_led_indications_ezb);
    }
    else
    {
        if(PRV_CPSS_PP_MAC(devNum)->devType == CPSS_98DX3550M_CNS)
        {
            ledStreamIndication = ac5x_RD_led_indications;
        }
        else
        {
            ledStreamIndication = ac5x_led_indications;
        }
    }

    for(ledUnit = 0; ledUnit < PRV_CPSS_DXCH_PP_HW_INFO_LED_MAC(devNum).sip6LedUnitNum; ledUnit++)
    {
        if(ledStreamIndication[ledUnit].ledStart == 0 && ledStreamIndication[ledUnit].ledEnd == 0)
        {
            /* The LED unit is exluded from the LED chain */
            ledConfig.sip6LedConfig.ledChainBypass[ledUnit] = GT_TRUE;
        }
        else
        {
            /* Start of LED stream location - Class 2 */
            ledConfig.sip6LedConfig.ledStart[ledUnit] = ledStreamIndication[ledUnit].ledStart;    /* LedControl.LedStart[5:12] */
            /* End of LED stream location: LED ports 0, 1 or 0 */
            ledConfig.sip6LedConfig.ledEnd[ledUnit] = ledStreamIndication[ledUnit].ledEnd;        /* LedControl.LedEnd[5:12] */
            /* The LED unit is included into LED chain */
            ledConfig.sip6LedConfig.ledChainBypass[ledUnit] = GT_FALSE;
        }
    }

    rc = cpssDxChLedStreamConfigSet(devNum, 0, &ledConfig);
    if(rc != GT_OK)
    {
        return rc;
    }

    ledClassManip.invertEnable            = ppProfilePtr->invertionEnable; /* not relevant for phoenix */
    ledClassManip.blinkSelect             = ppProfilePtr->blinkSelect;                            /* blinkGlobalControl.Blink Sel            [10-17] = 0 blink-0         */
    ledClassManip.forceEnable             = ppProfilePtr->forceEnable;                                             /* classesAndGroupConfig.Force En          [24-29] = pos = classNum = 0*/
    ledClassManip.forceData               = ppProfilePtr->forceData;                                                    /* classForcedData[class]                  [ 0:31] = 0                 */
    ledClassManip.pulseStretchEnable      = ppProfilePtr->pulseStretchEnable;                                             /* classGlobalControl.stretch class        [ 6-11] = pos = classNum = 0/1*/

    for (classNum = 0 ; classNum < 6; classNum++)
    {
        if(classNum == 2)
        {
            ledClassManip.disableOnLinkDown       = GT_TRUE;                                  /* classGlobalControl.disable on link down [ 0- 5] = pos = classNum = 0*/
            ledClassManip.blinkEnable             = GT_TRUE;                                  /* blinkGlobalControl.blinkEn     [18-25] = pos = classNum val = 1 */
        }                                                                                     /* blinkGlobalControl.Blink Select[10-17] = pos = classNum val = 0 */
        else
        {
            ledClassManip.disableOnLinkDown       = GT_FALSE;
            ledClassManip.blinkEnable             = GT_FALSE;
        }
        rc = cpssDxChLedStreamClassManipulationSet(devNum, 0,
                CPSS_DXCH_LED_PORT_TYPE_XG_E /* don't care , not applicable for phoenix */
                ,classNum ,&ledClassManip);
        if(rc != GT_OK)
        {
            return rc;
        }
    }
    /*--------------------------------------------------------------------------------------------------------*
     * LED. classesAndGroupConfig uses default setting , don't configure   cpssDxChLedStreamGroupConfigSet()  *
     *--------------------------------------------------------------------------------------------------------*
     * now configure port dependent LED configurations                                                        *
     *       - port led position (stream is hard wired)                                                       *
     *           The port LED positions for full chip is as follows:                                          *
     *                                                                                                        *
     *           P0,    LED Server 0,  Position 0   Ports  0.. 7                                              *
     *           P1,    LED Server 0,  Position 8   Ports  8..15                                              *
     *           P2,    LED Server 1,  Position 0   Ports 16..23                                              *
     *           P3,    LED Server 1,  Position 8   Ports 24..32                                              *
     *           P4,    LED Server 2,  Position 0   Ports 32..39                                              *
     *           P5,    LED Server 2,  Position 8   Ports 40..47                                              *
     *           P6,    LED Server 3,  Position 0   Port  48                                                  *
     *           P7,    LED Server 3,  Position 1   Port  49                                                  *
     *           P8,    LED Server 4,  Position 0   Ports 50..53                                              *
     *                                                                                                        *
     *--------------------------------------------------------------------------------------------------------*/
    numOfPhysicalPorts =
        PRV_CPSS_DXCH_MAX_PHY_PORT_NUMBER_MAC(devNum);
    for(portNum = 0; portNum < numOfPhysicalPorts; portNum++)
    {
        CPSS_APPLICATION_PORT_SKIP_CHECK(devNum, portNum);

        if (prvCpssDxChPortRemotePortCheck(devNum,portNum))
        {
            continue;
        }

        rc = cpssDxChPortPhysicalPortMapGet(devNum, portNum, 1, /*OUT*/&portMap);
        if(rc != GT_OK)
        {
            continue;
        }

        position = findLedPositionByMac(portMap.interfaceNum, ledPositionTbl);
        if (position != APP_PLATFORM_BAD_VALUE)
        {
            rc = cpssDxChLedStreamPortPositionSet(devNum, portNum, position);
            if(rc != GT_OK)
            {
                return rc;
            }
            /* Skip if in recovery mode HA */
            if (system_recovery.systemRecoveryProcess != CPSS_SYSTEM_RECOVERY_PROCESS_HA_E)
            {
            	rc = cpssDxChLedStreamPortClassPolarityInvertEnableSet(devNum,portNum,/*classNum*/2,/*invertEnable*/1);
            	if(rc != GT_OK)
            	{
            		return rc;
            	}
            }
        }
    }

    return GT_OK;
}
/************AC5x LED config*************/




/**
* @internal hawk_LedInit function
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
static GT_STATUS hawk_LedInit
(
    IN GT_U8                            devNum,
    IN CPSS_APP_PLATFORM_PP_PROFILE_STC *ppProfilePtr
)
{
    GT_STATUS                       rc;
    GT_U32                          classNum;
    GT_PHYSICAL_PORT_NUM            portNum;
    CPSS_LED_CONF_STC               ledConfig;
    CPSS_LED_CLASS_MANIPULATION_STC ledClassManip;
    GT_U32                          position;
    GT_U32                          ledUnit;
    GT_U32                          numOfPhysicalPorts;
    CPSS_DXCH_PORT_MAP_STC          portMap;
    PRV_CPSS_SIP6_LED_STREAM_INDICATIONS_STC *ledStreamIndication;
    CPSS_APP_PLATFORM_MAC_LEDPOSITION_STC    *ledPositionTbl;
    CPSS_SYSTEM_RECOVERY_INFO_STC   system_recovery; /* holds system recovery information */

    /************AC5p LED config*************/
    PRV_CPSS_SIP6_LED_STREAM_INDICATIONS_STC ac5p_XL_50G_led_indications[CPSS_LED_UNITS_MAX_NUM_CNS] =
    {
        {64, 71    },     /* 8 LED ports (LED Unit 0 - PIPE1 400G)     */
        { 0,  0    },     /* Not used    (LED Unit 1 - PIPE1 USX  )    */
        {64, 64+3  },     /* 1 LED port  (LED Unit 2 - PIPE1 CPU PORT) + 3 dummy indication */
        {64, 71    },     /* 8 LED ports (LED Unit 3 - PIPE0 400G)     */
        { 0,  0    },     /* Not used    (LED Unit 4 - PIPE0 USX)      */
        {64, 71    },     /* 8 LED ports (LED Unit 5 - PIPE3 400G)     */
        {64, 71+20 }      /* 8 LED ports  (LED Unit 6 - PIPE2 400G) +
                                        20 indications for dummy USX ports (LED stream alignment to 56 bits) */
    };

    PRV_CPSS_SIP6_LED_STREAM_INDICATIONS_STC ac5p_RD_led_indications[CPSS_LED_UNITS_MAX_NUM_CNS] =
    {
        {64, 65 },     /* 2 LED ports (LED Unit 0 - PIPE1 400G)     */
        { 0,  0 },     /* Not used    (LED Unit 1 - PIPE1 USX  )    */
        { 0,  0 },      /* Not used    (LED Unit 2 - PIPE1 CPU PORT) */
        {64, 65 },     /* 2 LED ports (LED Unit 3 - PIPE0 400G)     */
        { 0,  0 },     /* Not used    (LED Unit 4 - PIPE0 USX)      */
        {64, 64 },     /* 1 LED port  (LED Unit 5 - PIPE3 400G)     */
        {64, 64 }      /* 1 LED port  (LED Unit 6 - PIPE2 400G)     */
    };

    cpssOsMemSet(&ledConfig, 0, sizeof(CPSS_LED_CONF_STC));

    if(ppProfilePtr->ledPositionTable != NULL)
    {
        ledPositionTbl = ppProfilePtr->ledPositionTable;
    }
    else
    {
        CPSS_APP_PLATFORM_LOG_INFO_MAC("LED Init - Skipping: LED Table Not Found\n");
        return GT_OK;
    }

    ledConfig.ledOrganize                       = ppProfilePtr->ledOrganize;                       /* LedControl.OrganizeMode     [ 1: 1] =   1  by class       */
    ledConfig.sip6LedConfig.ledClockFrequency   = ppProfilePtr->sip6LedClockFrequency;                                                 /* LedChainClockControl.ledClkOutDiv  [11: 22] = 512 (app_clock - 833333/led_clock_out - 1627)  */
    ledConfig.pulseStretch                      = ppProfilePtr->pulseStretch;                           /* LedControl.Pulse-stretch-div[ 0: 29] =  100000 (ledClockFrequencyDivider - 10 * 10000  */

    ledConfig.blink0Duration          = ppProfilePtr->blink0Duration;    /* 64 ms */                     /* BlinkDivision0.blink0Duration  [ 0: 29]  = 0x32DCD40 (64/app_clock in mSec)*/
    ledConfig.blink0DutyCycle         = ppProfilePtr->blink0DutyCycle;  /* 50%   */                     /* blinkGlobalControl.Blink0-Duty [ 3: 4]  = 1 25%  */
    ledConfig.blink1Duration          = ppProfilePtr->blink1Duration;    /* 64 ms */                     /* BlinkDivision1.blink0Duration  [ 0: 29]  = 0x32DCD40 (64/app_clock in mSec)*/
    ledConfig.blink1DutyCycle         = ppProfilePtr->blink1DutyCycle;  /* 50%   */                     /* blinkGlobalControl.Blink1-Duty [ 8: 9]  = 1 25%  */

    ledConfig.sip6LedConfig.ledClockPosEdgeModeEnable  = ppProfilePtr->ledClockPosEdgeModeEnable;
    ledConfig.disableOnLinkDown       = ppProfilePtr->disableOnLinkDown;  /* don't care , see led-class manipulation */
    ledConfig.clkInvert               = ppProfilePtr->clkInvert;  /* don't care */
    ledConfig.class5select            = ppProfilePtr->class5select;  /* don't care */
    ledConfig.class13select           = ppProfilePtr->class13select; /* don't care */

    rc = cpssSystemRecoveryStateGet(&system_recovery);
    if (rc != GT_OK)
    {
      return rc;
    }

    if(GT_TRUE == ppProfilePtr->isEzbIsXmlLoaded)
    {

        ledStreamIndication = PRV_APP_REF_PP_UTILS_VAR(sip6_led_indications_ezb);
    }
    else
    {
        if(PRV_CPSS_PP_MAC(devNum)->devType == CPSS_98DX3550M_CNS)
        {
            ledStreamIndication = ac5p_RD_led_indications;
        }
        else
        {
            ledStreamIndication = ac5p_XL_50G_led_indications;
        }
    }

    for(ledUnit = 0; ledUnit < PRV_CPSS_DXCH_PP_HW_INFO_LED_MAC(devNum).sip6LedUnitNum; ledUnit++)
    {
        if(ledStreamIndication[ledUnit].ledStart == 0 && ledStreamIndication[ledUnit].ledEnd == 0)
        {
            /* The LED unit is exluded from the LED chain */
            ledConfig.sip6LedConfig.ledChainBypass[ledUnit] = GT_TRUE;
        }
        else
        {
            /* Start of LED stream location - Class 2 */
            ledConfig.sip6LedConfig.ledStart[ledUnit] = ledStreamIndication[ledUnit].ledStart;    /* LedControl.LedStart[5:12] */
            /* End of LED stream location: LED ports 0, 1 or 0 */
            ledConfig.sip6LedConfig.ledEnd[ledUnit] = ledStreamIndication[ledUnit].ledEnd;        /* LedControl.LedEnd[5:12] */
            /* The LED unit is included into LED chain */
            ledConfig.sip6LedConfig.ledChainBypass[ledUnit] = GT_FALSE;
        }
    }

    rc = cpssDxChLedStreamConfigSet(devNum, 0, &ledConfig);
    if(rc != GT_OK)
    {
        return rc;
    }

    ledClassManip.invertEnable            = ppProfilePtr->invertionEnable; /* not relevant for phoenix */
    ledClassManip.blinkSelect             = ppProfilePtr->blinkSelect;                            /* blinkGlobalControl.Blink Sel            [10-17] = 0 blink-0         */
    ledClassManip.forceEnable             = ppProfilePtr->forceEnable;                                             /* classesAndGroupConfig.Force En          [24-29] = pos = classNum = 0*/
    ledClassManip.forceData               = ppProfilePtr->forceData;                                                    /* classForcedData[class]                  [ 0:31] = 0                 */
    ledClassManip.pulseStretchEnable      = ppProfilePtr->pulseStretchEnable;                                             /* classGlobalControl.stretch class        [ 6-11] = pos = classNum = 0/1*/

    for (classNum = 0 ; classNum < 6; classNum++)
    {
        if(classNum == 2)
        {
            ledClassManip.disableOnLinkDown       = GT_TRUE;                                  /* classGlobalControl.disable on link down [ 0- 5] = pos = classNum = 0*/
            ledClassManip.blinkEnable             = GT_TRUE;                                  /* blinkGlobalControl.blinkEn     [18-25] = pos = classNum val = 1 */
        }                                                                                     /* blinkGlobalControl.Blink Select[10-17] = pos = classNum val = 0 */
        else
        {
            ledClassManip.disableOnLinkDown       = GT_FALSE;
            ledClassManip.blinkEnable             = GT_FALSE;
        }
        rc = cpssDxChLedStreamClassManipulationSet(devNum, 0,
                CPSS_DXCH_LED_PORT_TYPE_XG_E /* don't care , not applicable for hawk */
                ,classNum ,&ledClassManip);
        if(rc != GT_OK)
        {
            return rc;
        }
    }
    /*--------------------------------------------------------------------------------------------------------*
     * LED. classesAndGroupConfig uses default setting , don't configure   cpssDxChLedStreamGroupConfigSet()  *
     *--------------------------------------------------------------------------------------------------------*
     * now configure port dependent LED configurations                                                        *
     *       - port led position (stream is hard wired)                                                       *
     *           The port LED positions for full chip is as follows:                                          *
     *                                                                                                        *
     *           P0,    LED Server 1,  Positions 0-7   50G Ports  26, 27, 28, 32, 36, 40, 44, 48              *
     *                                                                                                        *
     *           P1,    LED Server 2   Positions 2     USX Ports  28, 29, 30, 31, 32, 33, 34, 35              *
     *           P1,    LED Server 2   Positions 4     USX Ports  36, 37, 38, 39, 40, 41, 42, 43              *
     *           P1,    LED Server 2   Positions 6     USX Ports  44, 45, 46, 47, 48, 49, 50, 51              *
     *                                                                                                        *
     *           P2,    LED Server 3,  Positions 0     CPU Ports  105                                         *
     *                                                                                                        *
     *           P3,    LED Server 4,  Positions 0-7   50G Ports  0,   1,  2,  6, 10, 14, 18, 22,             *
     *                                                                                                        *
     *           P4     LED Server 5   Positions 2     USX Ports   2,  3,  4,  5,  6,  7,  8,  9              *
     *           P4     LED Server 5   Positions 4     USX Ports  10, 11, 12, 13, 14, 15, 16, 17              *
     *           P4     LED Server 5   Positions 6     USX Ports  18, 19, 20, 21, 22, 23, 24, 25              *
     *                                                                                                        *
     *           P5,    LED Server 6,  Positions 0-7   50G Ports  78, 79, 80, 84, 88, 92, 96, 100             *
     *           P6,    LED Server 7,  Positions 0-7   50G Ports  52, 53, 54, 58, 62, 66, 70, 74              *
     *                                                                                                        *
     *--------------------------------------------------------------------------------------------------------*/
    numOfPhysicalPorts =
        PRV_CPSS_DXCH_MAX_PHY_PORT_NUMBER_MAC(devNum);
    for(portNum = 0; portNum < numOfPhysicalPorts; portNum++)
    {
        CPSS_APPLICATION_PORT_SKIP_CHECK(devNum, portNum);

        if (prvCpssDxChPortRemotePortCheck(devNum,portNum))
        {
            continue;
        }

        rc = cpssDxChPortPhysicalPortMapGet(devNum, portNum, 1, /*OUT*/&portMap);
        if(rc != GT_OK)
        {
            continue;
        }

        position = findLedPositionByMac(portMap.interfaceNum, ledPositionTbl);
        if (position != APP_PLATFORM_BAD_VALUE)
        {
            rc = cpssDxChLedStreamPortPositionSet(devNum, portNum, position);
            if(rc != GT_OK)
            {
                return rc;
            }
            /* Skip if in recovery mode HA */
            if (system_recovery.systemRecoveryProcess != CPSS_SYSTEM_RECOVERY_PROCESS_HA_E)
            {
                rc = cpssDxChLedStreamPortClassPolarityInvertEnableSet(devNum,portNum,/*classNum*/2,/*invertEnable*/1);
                if(rc != GT_OK)
                {
                    return rc;
                }
            }
        }
    }

    return GT_OK;
}
/************AC5p LED config*************/

/**
* @internal appPlatformLedInterfacesInit function
* @endinternal
*
* @brief   LED Init configurations
*
* @param [in] deviceNumber       - CPSS device number,
* @param [in] *ppProfileInfo     - PP profile
*
* @retval GT_OK                  - on success,
* @retval GT_FAIL                - otherwise.
*/

GT_STATUS appPlatformLedInterfacesInit
(
    IN GT_U8                             devNum,
    IN CPSS_APP_PLATFORM_PP_PROFILE_STC *ppProfilePtr
)
{
    GT_STATUS                       rc = GT_OK;
    GT_U32                          ledInterfaceNum;
    GT_U32                          classNum;
    GT_PHYSICAL_PORT_NUM            portNum;
    CPSS_LED_CONF_STC               ledConfig;
    CPSS_LED_CLASS_MANIPULATION_STC ledClassManip;
    GT_U32                          position;
    CPSS_DXCH_PORT_MAP_STC          portMap;
    CPSS_APP_PLATFORM_MAC_LEDPOSITION_STC
                                   *ledPositionTbl;
    GT_U32                          numOfPhysicalPorts;
    PRV_CPSS_DXCH_PP_CONFIG_STC *pDev;

    pDev = PRV_CPSS_DXCH_PP_MAC(devNum);

    if(ppProfilePtr->ledPositionTable != NULL)
    {
        CPSS_APP_PLATFORM_LOG_INFO_MAC("LED Init\n");
        ledPositionTbl = ppProfilePtr->ledPositionTable;
        if (pDev->genInfo.devFamily == CPSS_PP_FAMILY_DXCH_AC5X_E)
        {
#ifndef GM_USED
            rc = phoenix_LedInit(CAST_SW_DEVNUM(devNum), ppProfilePtr);
            CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, phoenix_LedInit);
#endif
            return GT_OK;
        }
        else if (pDev->genInfo.devFamily == CPSS_PP_FAMILY_DXCH_AC5P_E)
        {
#ifndef GM_USED
            rc = hawk_LedInit(CAST_SW_DEVNUM(devNum), ppProfilePtr);
            CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, hawk_LedInit);
#endif
            return GT_OK;
        }
        else if (pDev->genInfo.devFamily == CPSS_PP_FAMILY_DXCH_HARRIER_E)
        {
            return GT_OK;
        }
    }
    else
    {
        if (pDev->genInfo.devFamily == CPSS_PP_FAMILY_DXCH_FALCON_E)
        {
#ifndef GM_USED
            rc = falcon_LedInit(CAST_SW_DEVNUM(devNum), ppProfilePtr);
            CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, falcon_LedInit);
#endif
        }
        else if (pDev->genInfo.devFamily == CPSS_PP_FAMILY_DXCH_IRONMAN_E)
        {
#ifndef GM_USED
            rc = ironman_LedInit(CAST_SW_DEVNUM(devNum));
            CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, ironman_LedInit);
#endif
        }
        else
        {
            CPSS_APP_PLATFORM_LOG_INFO_MAC("LED Init - Skipping: LED Table Not Found\n");
        }
        return GT_OK;
    }

    ledConfig.ledOrganize             = ppProfilePtr->ledOrganize;
    ledConfig.ledClockFrequency       = ppProfilePtr->ledClockFrequency;
    ledConfig.invertEnable            = ppProfilePtr->invertEnable;
    ledConfig.pulseStretch            = ppProfilePtr->pulseStretch;
    ledConfig.blink0Duration          = ppProfilePtr->blink0Duration;
    ledConfig.blink0DutyCycle         = ppProfilePtr->blink0DutyCycle;
    ledConfig.blink1Duration          = ppProfilePtr->blink1Duration;
    ledConfig.blink1DutyCycle         = ppProfilePtr->blink1DutyCycle;
    ledConfig.disableOnLinkDown       = ppProfilePtr->disableOnLinkDown;
    ledConfig.clkInvert               = ppProfilePtr->clkInvert;
    ledConfig.class5select            = ppProfilePtr->class5select;
    ledConfig.class13select           = ppProfilePtr->class13select;

    if(pDev->genInfo.devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN_E || pDev->genInfo.devFamily == CPSS_PP_FAMILY_DXCH_AC3X_E )
    {
        ledConfig.ledStart                = ppProfilePtr->ledStart;
        ledConfig.ledEnd                  = ppProfilePtr->ledEnd;
        for(ledInterfaceNum = 0; ledInterfaceNum < PRV_CPSS_DXCH_ALDRIN_LED_IF_NUM_CNS; ledInterfaceNum++)
        {
            rc = cpssDxChLedStreamConfigSet(devNum, ledInterfaceNum, &ledConfig);
            if(rc != GT_OK)
            {
                return rc;
            }
        }
        ledClassManip.invertEnable            = ppProfilePtr->invertionEnable;
        ledClassManip.blinkSelect             = ppProfilePtr->blinkSelect;
        ledClassManip.forceEnable             = ppProfilePtr->forceEnable;
        ledClassManip.forceData               = ppProfilePtr->forceData;
        ledClassManip.pulseStretchEnable      = ppProfilePtr->pulseStretchEnable;
        ledClassManip.disableOnLinkDown       = ppProfilePtr->disableOnLinkDown;

        for(ledInterfaceNum = 0; ledInterfaceNum < PRV_CPSS_DXCH_ALDRIN_LED_IF_NUM_CNS; ledInterfaceNum++)
        {
            for (classNum = 0; classNum < 6; classNum++)
            {
                if(classNum == 2)
                {
                    ledClassManip.blinkEnable             = GT_TRUE;
                }
                else
                {
                    ledClassManip.blinkEnable             = GT_FALSE;
                }
                rc = cpssDxChLedStreamClassManipulationSet(devNum
                                                            ,ledInterfaceNum
                                                            ,CPSS_DXCH_LED_PORT_TYPE_XG_E /* don't care for Aldrin */
                                                            ,classNum
                                                            ,&ledClassManip);
                if(rc != GT_OK)
                {
                    return rc;
                }
            }
        }
    }
    else
    {
        for(ledInterfaceNum = 0; ledInterfaceNum < PRV_CPSS_DXCH_BOBCAT3_LED_IF_NUM_CNS; ledInterfaceNum++)
        {
            findMinMaxPositionByLedIf(ledInterfaceNum,
                                    ledPositionTbl,
                                    &ledConfig.ledStart,
                                    &ledConfig.ledEnd);

            ledConfig.ledStart += ppProfilePtr->ledStart;
            ledConfig.ledEnd += ppProfilePtr->ledEnd;

            rc = cpssDxChLedStreamConfigSet(devNum,ledInterfaceNum, &ledConfig);
            if(rc != GT_OK)
            {
                return rc;
            }
        }
        ledClassManip.invertEnable            = ppProfilePtr->invertionEnable;
        ledClassManip.blinkSelect             = ppProfilePtr->blinkSelect;
        ledClassManip.forceEnable             = ppProfilePtr->forceEnable;
        ledClassManip.forceData               = ppProfilePtr->forceData;
        ledClassManip.pulseStretchEnable      = ppProfilePtr->pulseStretchEnable;

        for(ledInterfaceNum = 0; ledInterfaceNum < PRV_CPSS_DXCH_BOBCAT3_LED_IF_NUM_CNS; ledInterfaceNum++)
        {
            for (classNum = 0 ; classNum < 6; classNum++)
            {
                if(classNum == 2)
                {
                    ledClassManip.disableOnLinkDown       = GT_TRUE;
                    ledClassManip.blinkEnable             = GT_TRUE;
                }
                else
                {
                    ledClassManip.disableOnLinkDown       = GT_FALSE;
                    ledClassManip.blinkEnable             = GT_FALSE;
                }
                rc = cpssDxChLedStreamClassManipulationSet(devNum
                                                            ,ledInterfaceNum
                                                            ,CPSS_DXCH_LED_PORT_TYPE_XG_E
                                                            ,classNum
                                                            ,&ledClassManip);
                if(rc != GT_OK)
                {
                    return rc;
                }
            }
        }
    }

    /*--------------------------------------------------------------------------------------------------------*
     * LED. classesAndGroupConfig uses default setting , don't configure   cpssDxChLedStreamGroupConfigSet()  *
     *--------------------------------------------------------------------------------------------------------*
     * now configure port dependent LED configurations                                                        *
     *       - port led position (stream is hard wired                                                        *
     *       - invert polarity                                                                                *
     *--------------------------------------------------------------------------------------------------------*/
    numOfPhysicalPorts =
        PRV_CPSS_DXCH_MAX_PHY_PORT_NUMBER_MAC(devNum);
    for(portNum = 0; portNum < numOfPhysicalPorts; portNum++)
    {
        CPSS_APPLICATION_PORT_SKIP_CHECK(devNum, portNum);

        if (prvCpssDxChPortRemotePortCheck(devNum,portNum))
        {
            continue;
        }

        rc = cpssDxChPortPhysicalPortMapGet(devNum, portNum, 1, /*OUT*/&portMap);
        if(rc != GT_OK)
        {
            continue;
        }

        position = findLedPositionByMac(portMap.interfaceNum, ledPositionTbl);
        if (position != APP_PLATFORM_BAD_VALUE)
        {
            rc = cpssDxChLedStreamPortPositionSet(devNum, portNum, position);
            if(rc != GT_OK)
            {
                return rc;
            }
            if(pDev->genInfo.devFamily == CPSS_PP_FAMILY_DXCH_ALDRIN2_E)
            {
                rc = cpssDxChLedStreamPortClassPolarityInvertEnableSet(devNum,portNum,/*classNum*/2,/*invertEnable*/1);
                if(rc != GT_OK)
                {
                    return rc;
                }
            }
        }
    }

    return rc;
}

#define PRV_MEM_FREE_NON_NULL_AND_SET_NULL_MAC(y, rc) \
if(y)                                             \
{                                                 \
  if(GT_OK == osCacheDmaFree(y))                  \
     y = NULL;                                    \
  else                                            \
  {                                               \
     rc = GT_FAIL;                                \
  }                                               \
}

/**
* @internal prvCpssAppPlatformDeAllocateDmaMem function
* @endinternal
*
* @param[in] devNum                - device number.
* @brief   This function deallocates memory of phase2 initialization stage, the
*         allocations include: Rx Descriptors / Buffer, Tx descriptors, Address
*         update descriptors.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS prvCpssAppPlatformDeAllocateDmaMem
(
    IN GT_U8 devNum
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    ii = 0;
    GT_U32    txQue = 0;
    GT_U32    rxQue = 0;
    PRV_CPSS_DXCH_WA_FALCON_PORT_DELETE_STC *prvInfoPtr;

    if(!PRV_CPSS_IS_DEV_EXISTS_MAC(devNum))
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

    if(PRV_APP_REF_PP_UTILS_VAR(cpssAppPlatformPpPhase2Db)[devNum].useMultiNetIfSdma == GT_TRUE)
    {
        for(ii = 0; ii < CPSS_MAX_SDMA_CPU_PORTS_CNS; ii++)
        {
            if( ii > 0 && (!PRV_CPSS_SIP_5_20_CHECK_MAC(devNum)) )
            {
                continue;
            }

            for(txQue = 0; txQue < CPSS_MAX_TX_QUEUE_CNS; txQue++)
            {
                PRV_MEM_FREE_NON_NULL_AND_SET_NULL_MAC(
                     PRV_APP_REF_PP_UTILS_VAR(cpssAppPlatformPpPhase2Db)[devNum].multiNetIfCfg.txSdmaQueuesConfig[ii][txQue].memData.staticAlloc.buffAndDescMemPtr, rc)
                CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, osCacheDmaFree);
            }

            for(rxQue = 0; rxQue < CPSS_MAX_RX_QUEUE_CNS; rxQue++)
            {

                PRV_MEM_FREE_NON_NULL_AND_SET_NULL_MAC(
                     PRV_APP_REF_PP_UTILS_VAR(cpssAppPlatformPpPhase2Db)[devNum].multiNetIfCfg.rxSdmaQueuesConfig[ii][rxQue].descMemPtr, rc)
                CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, osCacheDmaFree);

                PRV_MEM_FREE_NON_NULL_AND_SET_NULL_MAC(
                     PRV_APP_REF_PP_UTILS_VAR(cpssAppPlatformPpPhase2Db)[devNum].multiNetIfCfg.rxSdmaQueuesConfig[ii][rxQue].memData.staticAlloc.buffMemPtr, rc)
                CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, osCacheDmaFree);

            }
        }
    }

    PRV_MEM_FREE_NON_NULL_AND_SET_NULL_MAC(PRV_APP_REF_PP_UTILS_VAR(cpssAppPlatformPpPhase2Db)[devNum].auqCfg.auDescBlock, rc)
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, osCacheDmaFree);

    PRV_MEM_FREE_NON_NULL_AND_SET_NULL_MAC(PRV_APP_REF_PP_UTILS_VAR(cpssAppPlatformPpPhase2Db)[devNum].fuqCfg.auDescBlock, rc)
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, osCacheDmaFree);

    cpssOsMemSet(&(PRV_APP_REF_PP_UTILS_VAR(cpssAppPlatformPpPhase2Db)[devNum]), 0, sizeof(CPSS_DXCH_PP_PHASE2_INIT_INFO_STC));

    prvInfoPtr = PRV_CPSS_DXCH_PP_MAC(devNum)->port.falconPortDeleteInfoPtr;

    if (prvInfoPtr)
    {
        PRV_MEM_FREE_NON_NULL_AND_SET_NULL_MAC(prvInfoPtr->longBuffList[0], rc)
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, osCacheDmaFree);

        PRV_MEM_FREE_NON_NULL_AND_SET_NULL_MAC(prvInfoPtr->buffList[0], rc)
        CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, osCacheDmaFree);
    }

    return rc;
}

GT_STATUS prvCpssAppPlatformPpPhase1ConfigClear
(
    IN GT_U8 devNum
)
{
    GT_STATUS rc = GT_OK;
    (void)devNum;
#ifndef ASIC_SIMULATION
    rc = extDrvPexRemove (PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG(devNum)->hwInfo[0].hwAddr.busNo,
                          PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG(devNum)->hwInfo[0].hwAddr.devSel,
                          PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG(devNum)->hwInfo[0].hwAddr.funcNo);
    CPSS_APP_PLATFORM_LOG_AND_RET_ERR_MAC(rc, extDrvPexRemove);
#endif

    return rc;
}

GT_STATUS cpssAppPlatformTcamLibParamsGet
(
    OUT CPSS_APP_PLATFORM_TCAM_LIB_INFO_STC *tcamInfo
)
{
   tcamInfo->ttiTcamUseOffset                     = PRV_APP_REF_PP_UTILS_VAR(applicationPlatformTtiTcamUseOffset) ;
   tcamInfo->pclTcamUseIndexConversion            = PRV_APP_REF_PP_UTILS_VAR(applicationPlatformPclTcamUseIndexConversion) ;
   tcamInfo->tcamPclRuleBaseIndexOffset           = 0 ;
   tcamInfo->tcamIpcl0RuleBaseIndexOffset         = PRV_APP_REF_PP_UTILS_VAR(applicationPlatformTcamIpcl0RuleBaseIndexOffset) ;
   tcamInfo->tcamIpcl1RuleBaseIndexOffset         = PRV_APP_REF_PP_UTILS_VAR(applicationPlatformTcamIpcl1RuleBaseIndexOffset);
   tcamInfo->tcamIpcl2RuleBaseIndexOffset         = PRV_APP_REF_PP_UTILS_VAR(applicationPlatformTcamIpcl2RuleBaseIndexOffset) ;
   tcamInfo->tcamEpclRuleBaseIndexOffset          = PRV_APP_REF_PP_UTILS_VAR(applicationPlatformTcamEpclRuleBaseIndexOffset) ;
   tcamInfo->tcamIpcl0MaxNum                      = PRV_APP_REF_PP_UTILS_VAR(applicationPlatformTcamIpcl0MaxNum) ;
   tcamInfo->tcamIpcl1MaxNum                      = PRV_APP_REF_PP_UTILS_VAR(applicationPlatformTcamIpcl1MaxNum) ;
   tcamInfo->tcamIpcl2MaxNum                      = PRV_APP_REF_PP_UTILS_VAR(applicationPlatformTcamIpcl2MaxNum) ;
   tcamInfo->tcamEpclMaxNum                       = PRV_APP_REF_PP_UTILS_VAR(applicationPlatformTcamEpclMaxNum)  ;
   tcamInfo->tcamTtiHit0RuleBaseIndexOffset       = PRV_APP_REF_PP_UTILS_VAR(applicationPlatformTcamTtiHit0RuleBaseIndexOffset);
   tcamInfo->tcamTtiHit1RuleBaseIndexOffset       = PRV_APP_REF_PP_UTILS_VAR(applicationPlatformTcamTtiHit1RuleBaseIndexOffset);
   tcamInfo->tcamTtiHit2RuleBaseIndexOffset       = PRV_APP_REF_PP_UTILS_VAR(applicationPlatformTcamTtiHit2RuleBaseIndexOffset);
   tcamInfo->tcamTtiHit3RuleBaseIndexOffset       = PRV_APP_REF_PP_UTILS_VAR(applicationPlatformTcamTtiHit3RuleBaseIndexOffset);
   tcamInfo->tcamTtiHit0MaxNum                    = PRV_APP_REF_PP_UTILS_VAR(applicationPlatformTcamTtiHit0MaxNum );
   tcamInfo->tcamTtiHit1MaxNum                    = PRV_APP_REF_PP_UTILS_VAR(applicationPlatformTcamTtiHit1MaxNum );
   tcamInfo->tcamTtiHit2MaxNum                    = PRV_APP_REF_PP_UTILS_VAR(applicationPlatformTcamTtiHit2MaxNum );
   tcamInfo->tcamTtiHit3MaxNum                    = PRV_APP_REF_PP_UTILS_VAR(applicationPlatformTcamTtiHit3MaxNum );
   tcamInfo->save_tcamTtiHit0RuleBaseIndexOffset  = PRV_APP_REF_PP_UTILS_VAR(save_applicationPlatformTcamTtiHit0RuleBaseIndexOffset);
   tcamInfo->save_tcamTtiHit1RuleBaseIndexOffset  = PRV_APP_REF_PP_UTILS_VAR(save_applicationPlatformTcamTtiHit1RuleBaseIndexOffset);
   tcamInfo->save_tcamTtiHit2RuleBaseIndexOffset  = PRV_APP_REF_PP_UTILS_VAR(save_applicationPlatformTcamTtiHit2RuleBaseIndexOffset);
   tcamInfo->save_tcamTtiHit3RuleBaseIndexOffset  = PRV_APP_REF_PP_UTILS_VAR(save_applicationPlatformTcamTtiHit3RuleBaseIndexOffset);
   tcamInfo->save_tcamTtiHit0MaxNum               = PRV_APP_REF_PP_UTILS_VAR(save_applicationPlatformTcamTtiHit0MaxNum);
   tcamInfo->save_tcamTtiHit1MaxNum               = PRV_APP_REF_PP_UTILS_VAR(save_applicationPlatformTcamTtiHit1MaxNum);
   tcamInfo->save_tcamTtiHit2MaxNum               = PRV_APP_REF_PP_UTILS_VAR(save_applicationPlatformTcamTtiHit2MaxNum);
   tcamInfo->save_tcamTtiHit3MaxNum               = PRV_APP_REF_PP_UTILS_VAR(save_applicationPlatformTcamTtiHit3MaxNum);
   tcamInfo->save_tcamIpcl0RuleBaseIndexOffset    = PRV_APP_REF_PP_UTILS_VAR(save_applicationPlatformTcamIpcl0RuleBaseIndexOffset);
   tcamInfo->save_tcamIpcl1RuleBaseIndexOffset    = PRV_APP_REF_PP_UTILS_VAR(save_applicationPlatformTcamIpcl1RuleBaseIndexOffset);
   tcamInfo->save_tcamIpcl2RuleBaseIndexOffset    = PRV_APP_REF_PP_UTILS_VAR(save_applicationPlatformTcamIpcl2RuleBaseIndexOffset);
   tcamInfo->save_tcamEpclRuleBaseIndexOffset     = PRV_APP_REF_PP_UTILS_VAR(save_applicationPlatformTcamEpclRuleBaseIndexOffset;)
   tcamInfo->save_tcamIpcl0MaxNum                 = PRV_APP_REF_PP_UTILS_VAR(save_applicationPlatformTcamIpcl0MaxNum);
   tcamInfo->save_tcamIpcl1MaxNum                 = PRV_APP_REF_PP_UTILS_VAR(save_applicationPlatformTcamIpcl1MaxNum);
   tcamInfo->save_tcamIpcl2MaxNum                 = PRV_APP_REF_PP_UTILS_VAR(save_applicationPlatformTcamIpcl2MaxNum);
   tcamInfo->save_tcamEpclMaxNum                  = PRV_APP_REF_PP_UTILS_VAR(save_applicationPlatformTcamEpclMaxNum);
   return GT_OK;
}

