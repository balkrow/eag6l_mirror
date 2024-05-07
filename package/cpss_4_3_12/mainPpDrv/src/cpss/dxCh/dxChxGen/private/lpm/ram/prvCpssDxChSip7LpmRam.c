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
* @file prvCpssDxChSip7LpmRam.c
*
* @brief the CPSS LPM Engine support.
*
* @version   1
********************************************************************************
*/

#include <cpss/dxCh/dxChxGen/private/lpm/ram/prvCpssDxChLpmRamTypes.h>
#include <cpss/dxCh/dxChxGen/private/lpm/ram/prvCpssDxChLpmRam.h>
#include <cpss/dxCh/dxChxGen/private/lpm/ram/prvCpssDxChSip7LpmRam.h>
#include <cpss/dxCh/dxChxGen/private/lpm/ram/prvCpssDxChLpmRamTrie.h>
#include <cpss/dxCh/dxChxGen/private/lpm/ram/prvCpssDxChSip7LpmRamTrie.h>
#include <cpssCommon/private/prvCpssSip7DevMemManager.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/dxCh/dxChxGen/private/lpm/ram/prvCpssDxChLpmRamUc.h>
#include <cpss/dxCh/dxChxGen/private/lpm/ram/prvCpssDxChSip7LpmRamUc.h>
#include <cpss/dxCh/dxChxGen/private/lpm/ram/prvCpssDxChLpmRamMc.h>
#include <cpss/dxCh/dxChxGen/private/lpm/ram/prvCpssDxChSip7LpmRamMc.h>
#include <cpssCommon/private/prvCpssSkipList.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/private/lpm/ram/prvCpssDxChLpmRamMng.h>
#include <cpss/dxCh/dxChxGen/private/lpm/ram/prvCpssDxChSip6LpmRamMng.h>
#include <cpss/dxCh/dxChxGen/private/lpm/ram/prvCpssDxChSip7LpmRamMng.h>
#include <cpss/dxCh/dxChxGen/private/lpm/ram/prvCpssDxChSip6LpmRamDeviceSpecific.h>
#include <cpss/dxCh/dxChxGen/systemRecovery/hsu/private/prvCpssDxChHsu.h>
#include <cpss/dxCh/dxChxGen/private/lpm/prvCpssDxChLpmUtils.h>
#include <cpss/dxCh/dxChxGen/lpm/cpssDxChLpm.h>
#include <cpss/dxCh/dxChxGen/ipLpmEngine/private/cpssDxChPrvIpLpm.h>
#include <cpss/dxCh/dxChxGen/ipLpmEngine/private/cpssDxChIpLpmDbg.h>
#include <cpss/dxCh/dxChxGen/private/lpm/hw/prvCpssDxChLpmHw.h>
#include <cpss/dxCh/dxChxGen/private/lpm/hw/prvCpssDxChLpmHwSip7.h>
#include <cpss/dxCh/dxChxGen/private/lpm/ram/prvCpssDxChLpmRamDbg.h>
#include <cpss/dxCh/dxChxGen/private/lpm/ram/prvCpssDxChSip7LpmRamDbg.h>
#include <cpss/common/systemRecovery/private/prvCpssCommonSystemRecoveryParallel.h>

#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

/*global variables macros*/
#define PRV_SHARED_PORT_DIR_IP_LPM_SRC_GLOBAL_VAR_GET(_var)\
    PRV_SHARED_GLOBAL_VAR_GET(mainPpDrvMod.ipLpmDir.ipLpmSrc._var)

#ifdef CPSS_DXCH_LPM_DEBUG_MEM
GT_U32 prvCpssDxChLpmRamMemSize = 0;
#endif

/* max number of PBR lines for SIP7 128K */
#define PRV_CPSS_SIP7_MAX_PBR_LINES_CNS 1310721

/* max number of pbr banks in hw  */
#define PRV_CPSS_SIP7_MAX_NUM_PBR_BANKS_CNS 32

#define PRV_CPSS_SIP7_MAX_NUM_OF_SMALL_BLOCKS 8
#define PRV_CPSS_SIP7_FIRST_SMALL_BLOCKS_POSITION 40

/*global variables macros*/
#define PRV_SHARED_IP_LPM_DIR_LPM_RAM_SRC_GLOBAL_VAR_GET(_var)\
    PRV_SHARED_GLOBAL_VAR_GET(mainPpDrvMod.ipLpmDir.lpmRamSrc._var)

extern GT_VOID * prvCpssSlSearch
(
    IN GT_VOID        *ctrlPtr,
    IN GT_VOID        *dataPtr
);

/*******************************************************************************
**                               local defines                                **
*******************************************************************************/

#define PRV_CPSS_DXCH_LPM_IPV4_MC_LINK_LOCAL_CHECK_MAC(_ipGroup, _ipGroupPrefixLen, _ipSrcPrefixLen)    \
        if(!((_ipGroup.arIP[0] == 224)  &&\
             (_ipGroup.arIP[1] == 0)    &&\
             (_ipGroup.arIP[2] == 0)    &&\
             (_ipGroupPrefixLen == 24)  &&\
             (_ipSrcPrefixLen == 0)))     \
        {                                 \
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Not allow 'ipv4 MC link local' (224.0.0.x) and src prefix length [0]");\
        }

#define PRV_CPSS_DXCH_LPM_IPV4_MC_LINK_LOCAL_CHECK_EXACT_MATCH_MAC(_ipGroup, _ipGroupPrefixLen)    \
        if((_ipGroup.arIP[0] == 224)  &&\
           (_ipGroup.arIP[1] == 0)    &&\
           (_ipGroup.arIP[2] == 0)    &&\
           (_ipGroupPrefixLen == 32))   \
        {                               \
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Not allow 'ipv4 MC link local' (224.0.0.x) with prefix length[32] ");\
        }

#define PRV_CPSS_DXCH_LPM_IPV6_MC_LINK_LOCAL_CHECK_MAC(_ipGroup, _ipGroupPrefixLen, _ipSrcPrefixLen)    \
        if(!((_ipGroup.arIP[0] == 0xff)  &&\
             (_ipGroup.arIP[1] == 0x02)   &&\
             (_ipGroupPrefixLen == 16 )  &&\
             (_ipSrcPrefixLen == 0)))     \
        {                                 \
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Not allow 'ipv6 MC link local' (ff:02:x...) and src prefix length [0]");\
        }

#define PRV_CPSS_DXCH_LPM_IPV6_MC_LINK_LOCAL_CHECK_EXACT_MATCH_MAC(_ipGroup, _ipGroupPrefixLen)    \
        if((_ipGroup.arIP[0] == 0xff)  &&\
           (_ipGroup.arIP[1] == 0x02)    &&\
           (_ipGroupPrefixLen == 128))   \
        {                               \
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Not allow 'ipv6 MC link local' (ff:02:x...) and prefix length [128]");\
        }

/**
* @enum PRV_CPSS_DXCH_LPM_RAM_SEARCH_MEM_UPDATE_TYPE_ENT
 *
 * @brief Indicates how the LPM search structures should be updated
*/
typedef enum{

    /** don't allocate memory in the HW. */
    PRV_CPSS_DXCH_LPM_RAM_SEARCH_MEM_UPDATE_E = 0,

    /** allocate memory in the HW. */
    PRV_CPSS_DXCH_LPM_RAM_SEARCH_MEM_UPDATE_AND_ALLOC_E,

    /** overwrite and allocate memory if needed. */
    PRV_CPSS_DXCH_LPM_RAM_SEARCH_MEM_OVERWRITE_E

} PRV_CPSS_DXCH_LPM_RAM_SEARCH_MEM_UPDATE_TYPE_ENT;

/*******************************************************************************
**                             forward declarations                           **
*******************************************************************************/

/* Check validity of values of route entry pointer */
static GT_STATUS prvCpssDxChLpmRouteEntryPointerCheckSip7
(
    IN  PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC  *routeEntryPointerPtr
);

/*******************************************************************************
**                               local functions                              **
*******************************************************************************/

/**
* @internal prvCpssDxChLpmRamSip7CalcBankNumberIndex function
* @endinternal
*
* @brief    This function calculates the bank number index after
*           taking into consideration the PBR offset and the holes between
*           big banks and small banks
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                  Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] shadowPtr      - points to the shadow
* @param[inout] blockIndexPtr  - (pointer to) the index of the block
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad param
*
*/
GT_STATUS prvCpssDxChLpmRamSip7CalcBankNumberIndex
(
    IN      PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC    *shadowPtr,
    INOUT   GT_U32                              *blockIndexPtr
)
{
    if(*blockIndexPtr < shadowPtr->memoryOffsetStartHoleValue)
    {
        /* shift big blocks over pbrBlocks*/
        *blockIndexPtr = *blockIndexPtr - shadowPtr->memoryOffsetValue;
    }
    else
    {
         if(*blockIndexPtr >= shadowPtr->memoryOffsetEndHoleValue)
         {
             /* shift small blocks over the holes */
            *blockIndexPtr = *blockIndexPtr -
                            (shadowPtr->memoryOffsetEndHoleValue-shadowPtr->memoryOffsetStartHoleValue)/* shift over the hole */
                            - shadowPtr->memoryOffsetValue;/* shift over pbrBlock */
         }
         else
         {
             /* illegal blockIndex - fall in holes */
             CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
         }
    }
    return GT_OK;
}

/**
* @internal prvCpssDxChLpmRamSip7CalcPbrBankNumber function
* @endinternal
*
* @brief   This function calculates pbr banks number
*
* @note   APPLICABLE DEVICES:      AAS
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                  Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] bankSize                 - size of bank in lines.
* @param[in] numberOfPbrLines         - number of pbr lines.
*
* @param[out] numberOfPbrBanksPtr     - (pointer to) number of banks intended for pbr.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad param
*
*/
GT_STATUS prvCpssDxChLpmRamSip7CalcPbrBankNumber
(
    IN GT_U32 bankSize,
    IN GT_U32 numberOfPbrLines,
    OUT GT_U32 *numberOfPbrBanksPtr
)
{
    GT_U32 pbrMaxBankNumber = 0;
    GT_U32 pbrBankSize = bankSize;
    GT_U32 lpmRamTotalBlocksSizeIncludingGap = PRV_CPSS_DXCH_LPM_RAM_SIP7_MAX_PBR_SIZE_IN_LPM_LINES_CNS;

    if ( (numberOfPbrLines > PRV_CPSS_SIP7_MAX_NUM_PBR_BANKS_CNS*bankSize) ||
         (numberOfPbrLines > PRV_CPSS_SIP7_MAX_PBR_LINES_CNS) )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "wrong PBR entries number");
    }

    if ((numberOfPbrLines > 16*bankSize) && (numberOfPbrLines <= 32*bankSize))
    {
        /* max addressable pbr banks is 32*/
        /* each bank can address 8k*/
        pbrMaxBankNumber = 32;
    }
    else
    if ((numberOfPbrLines > 8*bankSize) && (numberOfPbrLines <= 16*bankSize))
    {
        /* max addressable pbr banks is 16*/
        /* each bank can address 16k*/
        pbrMaxBankNumber = 16;
    }
    else
    if ((numberOfPbrLines > 4*bankSize) && (numberOfPbrLines <= 8*bankSize))
    {
        /* max addressable pbr banks is 8*/
        /* each bank can address 32k*/
        pbrMaxBankNumber = 8;
    }
    else
    if ((numberOfPbrLines > 2*bankSize) && (numberOfPbrLines <= 4*bankSize))
    {
        /* max addressable pbr banks is 4*/
        /* each bank can address 64k*/
        pbrMaxBankNumber = 4;
    }
    else
    if ((numberOfPbrLines > 1*bankSize) && (numberOfPbrLines <= 2*bankSize))
    {
        /* max addressable pbr banks is 2*/
        /* each bank can address 128k*/
        pbrMaxBankNumber = 2;
    }
    else
    if (numberOfPbrLines <= 1*bankSize)
    {
        /* max addressable pbr banks is 1*/
        /* each bank can address 256k*/
        pbrMaxBankNumber = 1;
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "wrong PBR configuration");
    }

    if (bankSize > lpmRamTotalBlocksSizeIncludingGap/pbrMaxBankNumber)
    {
        pbrBankSize = lpmRamTotalBlocksSizeIncludingGap/pbrMaxBankNumber;
    }
    *numberOfPbrBanksPtr = (numberOfPbrLines +(pbrBankSize-1))/(pbrBankSize);
    if (*numberOfPbrBanksPtr > pbrMaxBankNumber)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "wrong PBR configuration");
    }
    return GT_OK;
}

/**
* @internal prvCpssDxChLpmRamSip7FillCfg function
* @endinternal
*
* @brief   This function retrieve next LPM DB ID from LPM DBs Skip List
*
* @note   APPLICABLE DEVICES:       AAS.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                   Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] memoryCfgPtr        - provisioned LPM RAM configuration
*
* @param[out] ramMemoryPtr       - calculated in lines memory ram configuration
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_OUT_OF_RANGE          - on out of range parameter
* @note none.
*
*/
GT_STATUS prvCpssDxChLpmRamSip7FillCfg
(
    IN    CPSS_DXCH_LPM_RAM_CONFIG_STC      *memoryCfgPtr,
    OUT   PRV_CPSS_DXCH_LPM_RAM_CONFIG_STC  *ramMemoryPtr
)
{
    GT_U32 numberOfPbrLines;
    GT_U32 bigBanksNumber;
    GT_U32 smallBanksNumber;
    GT_U32 smallBankSize = _2K;
    GT_U32 numOfBigPbrBlocks = 0;
    GT_U32 numOfSmallPbrBlocks = 0;
    GT_U32 smallBankStartOffset = PRV_CPSS_SIP7_FIRST_SMALL_BLOCKS_POSITION;
    GT_U32 i,k;
    GT_U32 bigBankSize;
    GT_BOOL sharedMemoryUsed = GT_FALSE;
    GT_STATUS rc = GT_OK;
    CPSS_PP_FAMILY_TYPE_ENT devFamily;
    PRV_CPSS_DXCH_LPM_RAM_CONFIG_STC  *ramMemoryArray;/* a list of all devices configuration */
    GT_BOOL                 isAC5PsmallScale = GT_FALSE;

    CPSS_NULL_PTR_CHECK_MAC(memoryCfgPtr);
    CPSS_NULL_PTR_CHECK_MAC(ramMemoryPtr);

    cpssOsMemSet(ramMemoryPtr,0,sizeof(PRV_CPSS_DXCH_LPM_RAM_CONFIG_STC));

    ramMemoryPtr->maxNumOfPbrEntries = memoryCfgPtr->maxNumOfPbrEntries;
    numberOfPbrLines = memoryCfgPtr->maxNumOfPbrEntries/MAX_NUMBER_OF_LEAVES_IN_LPM_LINE_CNS;
    if (memoryCfgPtr->maxNumOfPbrEntries%MAX_NUMBER_OF_LEAVES_IN_LPM_LINE_CNS != 0)
    {
        numberOfPbrLines++;
    }
    if (numberOfPbrLines > PRV_CPSS_DXCH_LPM_RAM_SIP7_MAX_PBR_SIZE_IN_LPM_LINES_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "bad number of PBR");
    }

    /* allocate needed configuration arrays */
    ramMemoryArray = cpssOsMalloc(sizeof(PRV_CPSS_DXCH_LPM_RAM_CONFIG_STC)* memoryCfgPtr->lpmRamConfigInfoNumOfElements);
    if (ramMemoryArray==NULL)
    {
        /* fail to allocate space */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, "fail to allocate space for ramMemoryArray in prvCpssDxChLpmRamSip7FillCfg \n");
    }
    cpssOsMemSet(ramMemoryArray,0,sizeof(PRV_CPSS_DXCH_LPM_RAM_CONFIG_STC)* memoryCfgPtr->lpmRamConfigInfoNumOfElements);

    for (k=0;k<memoryCfgPtr->lpmRamConfigInfoNumOfElements;k++)
    {
        switch (memoryCfgPtr->lpmRamConfigInfo[k].devType)
        {
            case CPSS_AC5P_ALL_DEVICES_CASES_MAC:
                bigBankSize = _10K;
                devFamily=CPSS_PP_FAMILY_DXCH_AC5P_E;
                sharedMemoryUsed = GT_TRUE;
                switch(memoryCfgPtr->lpmRamConfigInfo[k].devType)
                {
                    case CPSS_AC5P_XL_DEVICES_CASES_MAC:
                        isAC5PsmallScale = GT_FALSE;
                        break;
                    default:
                        isAC5PsmallScale = GT_TRUE;
                        break;
                }
                break;
            case CPSS_HARRIER_ALL_DEVICES_CASES_MAC:
                bigBankSize = _1K;
                sharedMemoryUsed = GT_FALSE;
                devFamily=CPSS_PP_FAMILY_DXCH_HARRIER_E;
                break;
            case CPSS_IRONMAN_ALL_DEVICES_CASES_MAC:
                bigBankSize = 1536;
                sharedMemoryUsed = GT_FALSE;
                devFamily=CPSS_PP_FAMILY_DXCH_IRONMAN_E;
                break;
            case CPSS_PHOENIX_ALL_DEVICES_CASES_MAC:
                bigBankSize = _1K;
                sharedMemoryUsed = GT_FALSE;
                devFamily=CPSS_PP_FAMILY_DXCH_AC5X_E;
                break;
            case CPSS_FALCON_ALL_DEVICES_CASES_MAC:
                bigBankSize = 14 * _1K;
                sharedMemoryUsed = GT_TRUE;
                devFamily=CPSS_PP_FAMILY_DXCH_FALCON_E;
                break;
            case CPSS_AAS_ALL_DEVICES_CASES_MAC:
            case CPSS_AAS_2_TILES_ALL_DEVICES_CASES_MAC:
                #ifdef GM_USED
                bigBankSize = 16 * _1K;
                #else
                bigBankSize = 128 * _1K;
                #endif

                sharedMemoryUsed = GT_TRUE;
                devFamily=CPSS_PP_FAMILY_DXCH_AAS_E;
                break;
            default:
                cpssOsFree(ramMemoryArray);
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, "bad devType in lpmRamConfigInfo[0]\n");
        }

        if (sharedMemoryUsed == GT_FALSE)
        {
            /* For Ironman */
            if(devFamily == CPSS_PP_FAMILY_DXCH_IRONMAN_E)
            {
                ramMemoryPtr->lpmRamConfigInfoNumOfElements=memoryCfgPtr->lpmRamConfigInfoNumOfElements;
                ramMemoryPtr->lpmRamConfigInfo[k].devType = memoryCfgPtr->lpmRamConfigInfo[k].devType;
                ramMemoryPtr->lpmRamConfigInfo[k].sharedMemCnfg = PRV_CPSS_DXCH_CFG_SHARED_TABLE_MODE_MAX_L3_MIN_L2_NO_EM_E; /* not relevant */
                ramMemoryArray[k].octetsGettingSmallBanksPriorityBitMap = 0x0;   /* no octet try to get small banks first  */
                ramMemoryArray[k].octetsGettingBigBanksPriorityBitMap   = 0x1FFFF;/* 0-15 octets try to get big banks first
                                                                                 also 16(one extra level) */
                ramMemoryArray[k].bigBanksNumber = 9;
                ramMemoryArray[k].bigBankSize = bigBankSize;/* _1.5K */
                ramMemoryArray[k].numOfBlocks = 9;
            }
            else
            {
               /* no shared memory , treated as 'MAX_L3' */
                ramMemoryArray[k].bigBanksNumber = 28;
                ramMemoryArray[k].bigBankSize = bigBankSize;
                ramMemoryArray[k].numOfBlocks = 30;

                ramMemoryPtr->lpmRamConfigInfoNumOfElements=memoryCfgPtr->lpmRamConfigInfoNumOfElements;
                ramMemoryPtr->lpmRamConfigInfo[k].devType = memoryCfgPtr->lpmRamConfigInfo[k].devType;
                ramMemoryPtr->lpmRamConfigInfo[k].sharedMemCnfg = PRV_CPSS_DXCH_CFG_SHARED_TABLE_MODE_MAX_L3_MIN_L2_NO_EM_E; /* not relevant */
                ramMemoryArray[k].octetsGettingSmallBanksPriorityBitMap = 0x2;   /* 1 octet try to get small banks first  */
                ramMemoryArray[k].octetsGettingBigBanksPriorityBitMap   = 0x1FFFD;/* 0, 2-15 octets try to get big banks first also 16(one extra level) */
            }
        }
        else
        {
            if (CPSS_AC5P_98DX4504_DEVICES_CHECK_MAC(memoryCfgPtr->lpmRamConfigInfo[k].devType))
            {
                ramMemoryArray[k].bigBanksNumber = 8;
                ramMemoryArray[k].bigBankSize = bigBankSize/2;
                ramMemoryArray[k].numOfBlocks = 8+PRV_CPSS_SIP7_MAX_NUM_OF_SMALL_BLOCKS; /* 16 = 8 + max num of small banks 8 */
                ramMemoryPtr->lpmRamConfigInfoNumOfElements=memoryCfgPtr->lpmRamConfigInfoNumOfElements;
                ramMemoryPtr->lpmRamConfigInfo[k].devType = memoryCfgPtr->lpmRamConfigInfo[k].devType;
                ramMemoryPtr->lpmRamConfigInfo[k].sharedMemCnfg = PRV_CPSS_DXCH_CFG_SHARED_TABLE_MODE_MAX_L3_MIN_L2_NO_EM_E; /* not relevant */
                ramMemoryArray[k].octetsGettingSmallBanksPriorityBitMap = 0x2;   /* 1 octet try to get small banks first  */
                ramMemoryArray[k].octetsGettingBigBanksPriorityBitMap   = 0x1FFFD;/* 0, 2-15 octets try to get big banks first also 16(one extra level) */
            }
            else
            {
                switch (memoryCfgPtr->lpmRamConfigInfo[k].sharedMemCnfg)
                {
                case CPSS_DXCH_CFG_SHARED_TABLE_MODE_MAX_L3_MIN_L2_NO_EM_E:
                    if(isAC5PsmallScale)
                    {
                        ramMemoryArray[k].bigBanksNumber = 20;
                        ramMemoryArray[k].octetsGettingSmallBanksPriorityBitMap = 0x7D;
                        ramMemoryArray[k].octetsGettingBigBanksPriorityBitMap   = 0x1FF82;
                        ramMemoryArray[k].numOfBlocks = 20+PRV_CPSS_SIP7_MAX_NUM_OF_SMALL_BLOCKS; /* 28 = 20 + max num of small banks 8 */
                    }
                    else
                    {
                        ramMemoryArray[k].bigBanksNumber = 28;
                        ramMemoryArray[k].octetsGettingSmallBanksPriorityBitMap = 0x2;   /* 1 octet try to get small banks first  */
                        ramMemoryArray[k].octetsGettingBigBanksPriorityBitMap   = 0x1FFFD;/* 0, 2-15 octets try to get big banks first
                                                                                     also 16(one extra level) */
                        ramMemoryArray[k].numOfBlocks = 30;
                    }
                    ramMemoryArray[k].bigBankSize = bigBankSize;

                    ramMemoryPtr->lpmRamConfigInfoNumOfElements=memoryCfgPtr->lpmRamConfigInfoNumOfElements;
                    ramMemoryPtr->lpmRamConfigInfo[k].devType = memoryCfgPtr->lpmRamConfigInfo[k].devType;
                    ramMemoryPtr->lpmRamConfigInfo[k].sharedMemCnfg = PRV_CPSS_DXCH_CFG_SHARED_TABLE_MODE_MAX_L3_MIN_L2_NO_EM_E;

                    break;
                case CPSS_DXCH_CFG_SHARED_TABLE_MODE_MIN_L3_MAX_L2_NO_EM_E:
                    if(isAC5PsmallScale)
                    {
                        ramMemoryArray[k].bigBanksNumber = 8;
                        ramMemoryArray[k].bigBankSize = bigBankSize / 2;
                        ramMemoryArray[k].numOfBlocks = 8+PRV_CPSS_SIP7_MAX_NUM_OF_SMALL_BLOCKS; /* 16 = 8 + max num of small banks 8 */

                        ramMemoryArray[k].octetsGettingSmallBanksPriorityBitMap = 0x1FFFD;
                        ramMemoryArray[k].octetsGettingBigBanksPriorityBitMap   = 0x2;
                    }
                    else
                    {
                        ramMemoryArray[k].bigBanksNumber = 0;
                        ramMemoryArray[k].bigBankSize = 0;
                        ramMemoryArray[k].numOfBlocks = PRV_CPSS_SIP7_MAX_NUM_OF_SMALL_BLOCKS; /* max num of small banks 8 */
                        if (numberOfPbrLines > PRV_CPSS_DXCH_LPM_RAM_SIP7_MAX_PBR_SIZE_MIN_LPM_MODE_IN_LPM_LINES_CNS)
                        {
                            cpssOsFree(ramMemoryArray);
                            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "bad number of PBR");
                        }
                        ramMemoryArray[k].octetsGettingSmallBanksPriorityBitMap = 0x1FFFF;
                        ramMemoryArray[k].octetsGettingBigBanksPriorityBitMap   = 0x0;
                    }

                    ramMemoryPtr->lpmRamConfigInfoNumOfElements=memoryCfgPtr->lpmRamConfigInfoNumOfElements;
                    ramMemoryPtr->lpmRamConfigInfo[k].devType = memoryCfgPtr->lpmRamConfigInfo[k].devType;
                    ramMemoryPtr->lpmRamConfigInfo[k].sharedMemCnfg = PRV_CPSS_DXCH_CFG_SHARED_TABLE_MODE_MIN_L3_MAX_L2_NO_EM_E;

                    break;
                case CPSS_DXCH_CFG_SHARED_TABLE_MODE_MID_L3_MID_L2_MIN_EM_E:
                    if(isAC5PsmallScale)
                    {
                        ramMemoryArray[k].bigBanksNumber = 16;
                        ramMemoryArray[k].octetsGettingSmallBanksPriorityBitMap = 0x1FF82;
                        ramMemoryArray[k].octetsGettingBigBanksPriorityBitMap   = 0x7D;
                        ramMemoryArray[k].numOfBlocks = 16+PRV_CPSS_SIP7_MAX_NUM_OF_SMALL_BLOCKS; /* 24 = 16 + max num of small banks 8 */;
                    }
                    else
                    {
                        ramMemoryArray[k].bigBanksNumber = 24;
                        ramMemoryArray[k].octetsGettingSmallBanksPriorityBitMap = 0x1FE02;/* 1, 9-15 octets try to get small banks first
                                                                                     also 16(one extra level) */
                        ramMemoryArray[k].octetsGettingBigBanksPriorityBitMap   = 0x1FD;  /* 0, 2-8  octets try to get big banks first */
                        ramMemoryArray[k].numOfBlocks = 30;
                    }

                    ramMemoryArray[k].bigBankSize = bigBankSize; /* each bank is 128K lines - need to change logic to pair_dynamic_sharing */

                    ramMemoryPtr->lpmRamConfigInfoNumOfElements=memoryCfgPtr->lpmRamConfigInfoNumOfElements;
                    ramMemoryPtr->lpmRamConfigInfo[k].devType = memoryCfgPtr->lpmRamConfigInfo[k].devType;
                    ramMemoryPtr->lpmRamConfigInfo[k].sharedMemCnfg = PRV_CPSS_DXCH_CFG_SHARED_TABLE_MODE_MID_L3_MID_L2_MIN_EM_E;

                    break;

                case CPSS_DXCH_CFG_SHARED_TABLE_MODE_MID_L3_MIN_L2_MAX_EM_E:
                    if(isAC5PsmallScale)
                    {
                        ramMemoryArray[k].bigBanksNumber = 16;
                        ramMemoryArray[k].octetsGettingSmallBanksPriorityBitMap = 0x1FF82;
                        ramMemoryArray[k].octetsGettingBigBanksPriorityBitMap   = 0x7D;
                        ramMemoryArray[k].numOfBlocks = 16+PRV_CPSS_SIP7_MAX_NUM_OF_SMALL_BLOCKS; /* 24 = 16 + max num of small banks 8 */;
                    }
                    else
                    {
                        ramMemoryArray[k].bigBanksNumber = 24;
                        ramMemoryArray[k].octetsGettingSmallBanksPriorityBitMap = 0x1FE02;/* 1, 9-15 octets try to get small banks first
                                                                                     also 16(one extra level) */
                        ramMemoryArray[k].octetsGettingBigBanksPriorityBitMap   = 0x1FD;  /* 0, 2-8  octets try to get big banks first */
                        ramMemoryArray[k].numOfBlocks = 30;
                    }
                    ramMemoryArray[k].bigBankSize = bigBankSize / 2;

                    ramMemoryPtr->lpmRamConfigInfoNumOfElements=memoryCfgPtr->lpmRamConfigInfoNumOfElements;
                    ramMemoryPtr->lpmRamConfigInfo[k].devType = memoryCfgPtr->lpmRamConfigInfo[k].devType;
                    ramMemoryPtr->lpmRamConfigInfo[k].sharedMemCnfg = PRV_CPSS_DXCH_CFG_SHARED_TABLE_MODE_MID_L3_MIN_L2_MAX_EM_E;

                    break;
                case CPSS_DXCH_CFG_SHARED_TABLE_MODE_MID_LOW_L3_MID_LOW_L2_MAX_EM_E:
                    if(isAC5PsmallScale)
                    {
                        ramMemoryArray[k].bigBanksNumber = 8;
                        ramMemoryArray[k].numOfBlocks = 8+PRV_CPSS_SIP7_MAX_NUM_OF_SMALL_BLOCKS; /* 16 = 8 + max num of small banks 8 */;

                        ramMemoryArray[k].octetsGettingSmallBanksPriorityBitMap = 0x1FFFD;
                        ramMemoryArray[k].octetsGettingBigBanksPriorityBitMap   = 0x2;
                    }
                    else
                    {
                        ramMemoryArray[k].bigBanksNumber = 16;
                        ramMemoryArray[k].numOfBlocks = 16+PRV_CPSS_SIP7_MAX_NUM_OF_SMALL_BLOCKS; /* 24 = 16 + max num of small banks 8 */;

                        ramMemoryArray[k].octetsGettingSmallBanksPriorityBitMap = 0x1FF82;
                        ramMemoryArray[k].octetsGettingBigBanksPriorityBitMap   = 0x7D;
                    }

                    ramMemoryPtr->lpmRamConfigInfoNumOfElements=memoryCfgPtr->lpmRamConfigInfoNumOfElements;
                    ramMemoryPtr->lpmRamConfigInfo[k].devType = memoryCfgPtr->lpmRamConfigInfo[k].devType;

                    ramMemoryArray[k].bigBankSize = bigBankSize/2;
                    ramMemoryPtr->lpmRamConfigInfo[k].sharedMemCnfg = PRV_CPSS_DXCH_CFG_SHARED_TABLE_MODE_MID_LOW_L3_MID_LOW_L2_MAX_EM_E;

                    break;
                case CPSS_DXCH_CFG_SHARED_TABLE_MODE_MID_L3_MID_L2_NO_EM_E:
                    if(isAC5PsmallScale)
                    {
                        ramMemoryArray[k].bigBanksNumber = 24;
                        ramMemoryArray[k].bigBankSize = bigBankSize/2;

                        ramMemoryArray[k].octetsGettingSmallBanksPriorityBitMap = 0x1FE02;
                        ramMemoryArray[k].octetsGettingBigBanksPriorityBitMap   = 0x17D;
                        ramMemoryArray[k].numOfBlocks = 30;
                    }
                    else
                    {
                        ramMemoryArray[k].bigBanksNumber = 16;
                        ramMemoryArray[k].bigBankSize = bigBankSize;

                        ramMemoryArray[k].octetsGettingSmallBanksPriorityBitMap = 0x1FF82;/* 1, 7-15 octets try to get small banks first
                                                                                                also 16(one extra level) */
                        ramMemoryArray[k].octetsGettingBigBanksPriorityBitMap   = 0x7D;  /* 0, 2-6  octets try to get big banks first */
                        ramMemoryArray[k].numOfBlocks = 16+PRV_CPSS_SIP7_MAX_NUM_OF_SMALL_BLOCKS; /* 24 = 16 + max num of small banks 8 */;
                    }

                    ramMemoryPtr->lpmRamConfigInfoNumOfElements=memoryCfgPtr->lpmRamConfigInfoNumOfElements;
                    ramMemoryPtr->lpmRamConfigInfo[k].devType = memoryCfgPtr->lpmRamConfigInfo[k].devType;

                    ramMemoryPtr->lpmRamConfigInfo[k].sharedMemCnfg = PRV_CPSS_DXCH_CFG_SHARED_TABLE_MODE_MID_L3_MID_L2_NO_EM_E;

                    break;

                case CPSS_DXCH_CFG_SHARED_TABLE_MODE_MID_LOW_L3_MID_L2_MID_EM_E:
                    if(isAC5PsmallScale)
                    {
                        ramMemoryArray[k].bigBanksNumber = 8;
                        ramMemoryArray[k].numOfBlocks = 8+PRV_CPSS_SIP7_MAX_NUM_OF_SMALL_BLOCKS; /* 16 = 8 + max num of small banks 8 */;

                        ramMemoryArray[k].octetsGettingSmallBanksPriorityBitMap = 0x1FFFD;
                        ramMemoryArray[k].octetsGettingBigBanksPriorityBitMap   = 0x2;
                    }
                    else
                    {
                        ramMemoryArray[k].bigBanksNumber = 16;
                        ramMemoryArray[k].numOfBlocks = 16+PRV_CPSS_SIP7_MAX_NUM_OF_SMALL_BLOCKS; /* 24 = 16 + max num of small banks 8 */;

                        ramMemoryArray[k].octetsGettingSmallBanksPriorityBitMap = 0x1FF82;/* 1, 7-15 octets try to get small banks first
                                                                                    also 16(one extra level) */
                        ramMemoryArray[k].octetsGettingBigBanksPriorityBitMap   = 0x7D;  /* 0, 2-6  octets try to get big banks first */
                    }
                    ramMemoryArray[k].bigBankSize = bigBankSize/2;

                    ramMemoryPtr->lpmRamConfigInfoNumOfElements=memoryCfgPtr->lpmRamConfigInfoNumOfElements;
                    ramMemoryPtr->lpmRamConfigInfo[k].devType = memoryCfgPtr->lpmRamConfigInfo[0].devType;
                    ramMemoryPtr->lpmRamConfigInfo[k].sharedMemCnfg = PRV_CPSS_DXCH_CFG_SHARED_TABLE_MODE_MID_LOW_L3_MID_L2_MID_EM_E;

                    break;

                case CPSS_DXCH_CFG_SHARED_TABLE_MODE_MID_L3_MID_LOW_L2_MID_EM_MAX_ARP_E:

                    if(isAC5PsmallScale)
                    {
                        ramMemoryArray[k].bigBanksNumber = 16;
                        ramMemoryArray[k].numOfBlocks = 16+PRV_CPSS_SIP7_MAX_NUM_OF_SMALL_BLOCKS; /* 24 = 16 + max num of small banks 8 */;
                    }
                    else
                    {
                        ramMemoryArray[k].bigBanksNumber = 24;
                        ramMemoryArray[k].numOfBlocks = 30;
                    }
                    ramMemoryArray[k].bigBankSize = bigBankSize/2;

                    ramMemoryPtr->lpmRamConfigInfoNumOfElements=memoryCfgPtr->lpmRamConfigInfoNumOfElements;
                    ramMemoryPtr->lpmRamConfigInfo[k].devType = memoryCfgPtr->lpmRamConfigInfo[k].devType;
                    ramMemoryPtr->lpmRamConfigInfo[k].sharedMemCnfg = PRV_CPSS_DXCH_CFG_SHARED_TABLE_MODE_MID_L3_MID_LOW_L2_MID_EM_MAX_ARP_E;

                    ramMemoryArray[k].octetsGettingSmallBanksPriorityBitMap = 0x1FF82;/* 1, 7-15 octets try to get small banks first
                                                                                    also 16(one extra level) */
                    ramMemoryArray[k].octetsGettingBigBanksPriorityBitMap   = 0x7D;  /* 0, 2-6  octets try to get big banks first */
                    break;

                case CPSS_DXCH_CFG_SHARED_TABLE_MODE_MIN_L3_MID_L2_MAX_EM_E:
                    ramMemoryArray[k].bigBanksNumber = 0;
                    ramMemoryArray[k].bigBankSize = 0;
                    ramMemoryArray[k].numOfBlocks = PRV_CPSS_SIP7_MAX_NUM_OF_SMALL_BLOCKS;/* max num of small banks 8 */

                    ramMemoryPtr->lpmRamConfigInfoNumOfElements=memoryCfgPtr->lpmRamConfigInfoNumOfElements;
                    ramMemoryPtr->lpmRamConfigInfo[k].devType = memoryCfgPtr->lpmRamConfigInfo[k].devType;
                    ramMemoryPtr->lpmRamConfigInfo[k].sharedMemCnfg = PRV_CPSS_DXCH_CFG_SHARED_TABLE_MODE_MIN_L3_MID_L2_MAX_EM_E;

                    if (numberOfPbrLines > PRV_CPSS_DXCH_LPM_RAM_SIP7_MAX_PBR_SIZE_MIN_LPM_MODE_IN_LPM_LINES_CNS)
                    {
                        cpssOsFree(ramMemoryArray);
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "bad number of PBR");
                    }
                    ramMemoryArray[k].octetsGettingSmallBanksPriorityBitMap = 0x1FFFF;/* all octets get small banks */
                    ramMemoryArray[k].octetsGettingBigBanksPriorityBitMap   = 0;     /* no big banks */
                    break;

                case CPSS_DXCH_CFG_SHARED_TABLE_MODE_LOW_MAX_L3_MID_LOW_L2_NO_EM_E:
                    if(isAC5PsmallScale)
                    {
                        ramMemoryArray[k].bigBanksNumber = 16;
                        ramMemoryArray[k].octetsGettingSmallBanksPriorityBitMap = 0x1FF82;
                        ramMemoryArray[k].octetsGettingBigBanksPriorityBitMap   = 0x7D;
                        ramMemoryArray[k].numOfBlocks = 16+PRV_CPSS_SIP7_MAX_NUM_OF_SMALL_BLOCKS; /* 24 = 16 + max num of small banks 8 */;
                    }
                    else
                    {
                        ramMemoryArray[k].bigBanksNumber = 24;
                        ramMemoryArray[k].octetsGettingSmallBanksPriorityBitMap = 0x1FE02;/* 1, 9-15 octets try to get small banks first
                                                                                                also 16(one extra level) */
                        ramMemoryArray[k].octetsGettingBigBanksPriorityBitMap   = 0x1FD;  /* 0, 2-8  octets try to get big banks first */
                        ramMemoryArray[k].numOfBlocks = 30;
                    }

                    ramMemoryArray[k].bigBankSize = bigBankSize;

                    ramMemoryPtr->lpmRamConfigInfoNumOfElements=memoryCfgPtr->lpmRamConfigInfoNumOfElements;
                    ramMemoryPtr->lpmRamConfigInfo[k].devType = memoryCfgPtr->lpmRamConfigInfo[k].devType;
                    ramMemoryPtr->lpmRamConfigInfo[k].sharedMemCnfg = PRV_CPSS_DXCH_CFG_SHARED_TABLE_MODE_LOW_MAX_L3_MID_LOW_L2_NO_EM_E;


                    break;

                case CPSS_DXCH_CFG_SHARED_TABLE_MODE_MID_HIGH_L3_MID_LOW_L2_NO_EM_MAX_ARP_E:
                    if(isAC5PsmallScale)
                    {
                        ramMemoryArray[k].bigBanksNumber = 16;
                        ramMemoryArray[k].numOfBlocks = 16+PRV_CPSS_SIP7_MAX_NUM_OF_SMALL_BLOCKS; /* 24 = 16 + max num of small banks 8 */;
                    }
                    else
                    {
                        ramMemoryArray[k].bigBanksNumber = 20;
                        ramMemoryArray[k].numOfBlocks = 16+PRV_CPSS_SIP7_MAX_NUM_OF_SMALL_BLOCKS; /* 28 = 20 + max num of small banks 8 */;
                    }
                    ramMemoryArray[k].bigBankSize = bigBankSize;

                    ramMemoryPtr->lpmRamConfigInfoNumOfElements=memoryCfgPtr->lpmRamConfigInfoNumOfElements;
                    ramMemoryPtr->lpmRamConfigInfo[k].devType = memoryCfgPtr->lpmRamConfigInfo[k].devType;
                    ramMemoryPtr->lpmRamConfigInfo[k].sharedMemCnfg = PRV_CPSS_DXCH_CFG_SHARED_TABLE_MODE_MID_HIGH_L3_MID_LOW_L2_NO_EM_MAX_ARP_E;

                    ramMemoryArray[k].octetsGettingSmallBanksPriorityBitMap = 0x1FE02;/* 1, 9-15 octets try to get small banks first
                                                                                    also 16(one extra level) */
                    ramMemoryArray[k].octetsGettingBigBanksPriorityBitMap   = 0x1FD;  /* 0, 2-8  octets try to get big banks first */
                    break;

                default:
                     cpssOsFree(ramMemoryArray);
                     CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "bad share table mode");
                }
            }
        }
    }

    /* go over the array of configuration - and chose the minimal configuration
       that will fit all devices in the lpmRamConfigInfo list */
    for (k=0;k<memoryCfgPtr->lpmRamConfigInfoNumOfElements;k++)
    {
        if (k==0)
        {
            /* set first optional configuration */
            ramMemoryPtr->numOfBlocks = ramMemoryArray[k].numOfBlocks;
            ramMemoryPtr->bigBanksNumber = ramMemoryArray[k].bigBanksNumber;
            ramMemoryPtr->bigBankSize = ramMemoryArray[k].bigBankSize;
            ramMemoryPtr->octetsGettingSmallBanksPriorityBitMap = ramMemoryArray[k].octetsGettingSmallBanksPriorityBitMap;
            ramMemoryPtr->octetsGettingBigBanksPriorityBitMap = ramMemoryArray[k].octetsGettingBigBanksPriorityBitMap;
        }
        else
        {
            if (ramMemoryArray[k].numOfBlocks < ramMemoryPtr->numOfBlocks)
            {
                ramMemoryPtr->numOfBlocks = ramMemoryArray[k].numOfBlocks;
            }
            if (ramMemoryArray[k].bigBanksNumber < ramMemoryPtr->bigBanksNumber)
            {
                ramMemoryPtr->bigBanksNumber = ramMemoryArray[k].bigBanksNumber;
                ramMemoryPtr->octetsGettingSmallBanksPriorityBitMap = ramMemoryArray[k].octetsGettingSmallBanksPriorityBitMap;
                ramMemoryPtr->octetsGettingBigBanksPriorityBitMap = ramMemoryArray[k].octetsGettingBigBanksPriorityBitMap;
            }
            if (ramMemoryArray[k].bigBankSize < ramMemoryPtr->bigBankSize)
            {
                ramMemoryPtr->bigBankSize = ramMemoryArray[k].bigBankSize;
            }
        }
    }

    smallBanksNumber = ramMemoryPtr->numOfBlocks - ramMemoryPtr->bigBanksNumber;
    bigBanksNumber = ramMemoryPtr->bigBanksNumber;
    /* small bank always start from bank10 and above */
    if(bigBanksNumber > smallBankStartOffset)
        smallBankStartOffset = bigBanksNumber;

    if (numberOfPbrLines > 0)
    {
        if (numberOfPbrLines <= smallBankSize)
        {
            if (smallBanksNumber > 0)
            {
                /* take 1 bank from the end : it is only for PBR*/
                smallBanksNumber--;
                ramMemoryPtr->numOfBlocks--;
                numOfSmallPbrBlocks++;
            }
            else
            {
                /* num of small banks is 0*/
                ramMemoryPtr->numOfBlocks--;
                ramMemoryPtr->bigBanksNumber--;
                numOfBigPbrBlocks++;
            }
        }
        else
        {
            if (bigBanksNumber > 0)
            {
                /* The max pbr value in lines is 128K. There is a differnce in max PBR lines value
                   for shared memory configuration modes. It is limited by pbr bank size register*/
                /* pbr banks size reg config must be done correspondingly for each device */
                /* In this case pbrs will be allocated in big banks */
                rc = prvCpssDxChLpmRamSip7CalcPbrBankNumber(ramMemoryPtr->bigBankSize,numberOfPbrLines,&numOfBigPbrBlocks);
                if (rc != GT_OK)
                {
                    cpssOsFree(ramMemoryArray);
                    return rc;
                }
                ramMemoryPtr->bigBanksNumber = ramMemoryPtr->bigBanksNumber - numOfBigPbrBlocks;
                ramMemoryPtr->numOfBlocks = ramMemoryPtr->numOfBlocks - numOfBigPbrBlocks;
            }
            else
            {
                /* we have only small banks */
                /* calculate number of small banks needed for PBR */
                rc = prvCpssDxChLpmRamSip7CalcPbrBankNumber(smallBankSize,numberOfPbrLines,&numOfSmallPbrBlocks);
                if (rc != GT_OK)
                {
                    cpssOsFree(ramMemoryArray);
                    return rc;
                }
                smallBanksNumber = smallBanksNumber - numOfSmallPbrBlocks;
                ramMemoryPtr->numOfBlocks = ramMemoryPtr->numOfBlocks - numOfSmallPbrBlocks;
                /* in case of min lpm configuration we need max 8 small banks : pbr bank size 4K*/
            }
        }
    }
    ramMemoryPtr->numberOfBigPbrBanks = numOfBigPbrBlocks;

    /* fill big banks data: banks used for PBR filled with 0 size */
    for (i = numOfBigPbrBlocks; i < bigBanksNumber; i++)
    {
        ramMemoryPtr->blocksSizeArray[i] = ramMemoryPtr->bigBankSize;
    }
    /* if pbr resided in small block, it would be last */
    /* handle small blocks */
    for (i = 0; i < smallBanksNumber; i++)
    {
        if ((i+smallBankStartOffset) >= PRV_CPSS_DXCH_LPM_RAM_NUM_OF_MEMORIES_SIP7_CNS)
        {
            cpssOsFree(ramMemoryArray);
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "Souldn't happen: Exceeded bankSize array");
        }
        ramMemoryPtr->blocksSizeArray[i + smallBankStartOffset] = smallBankSize;
    }

    cpssOsFree(ramMemoryArray);
    return GT_OK;
}

/**
* @internal deleteSip7ShadowStruct function
* @endinternal
*
* @brief   This function deletes existing shadow struct.
*
* @param[in] shadowPtr                - points to the shadow to delete
*                                       GT_OK on success, or
*                                       GT_FAIL otherwise.
*/
static GT_STATUS deleteSip7ShadowStruct
(
    IN  PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC *shadowPtr
)
{
    GT_STATUS       rc;
    GT_U32          i,octet;
    PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT protocol;
    GT_U32          numOfOctetsPerProtocol[PRV_CPSS_DXCH_LPM_RAM_NUM_OF_UC_PROTOCOLS_CNS] = {PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_FOR_GON_IN_IPV4_PROTOCOL_CNS,
                                                                                             PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_FOR_GON_IN_IPV6_PROTOCOL_CNS,
                                                                                             PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_FOR_GON_IN_FCOE_PROTOCOL_CNS};
    PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC startOfOctetList;
    PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC *tempNextMemInfoPtr;

    /* first make sure all virtual routers are deleted */
    for (i = 0 ; i < shadowPtr->vrfTblSize ; i++)
    {
        if (shadowPtr->vrRootBucketArray[i].valid == GT_TRUE)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }
    }

    /* now free resources used by the shadow struct */

    /* free memory set to vr table array */
    cpssOsLpmFree(shadowPtr->vrRootBucketArray);

    /* free all memory pools allocated */
    for (i = 0 ; i < shadowPtr->numOfLpmMemories ; i++)
    {
        if(shadowPtr->lpmRamStructsMemPoolPtr[i]!=0)
        {
            rc = prvCpssDmmClosePartitionSip7(shadowPtr->lpmRamStructsMemPoolPtr[i]);
            if (rc != GT_OK)
            {
                 /* free alloctated arrays used for RAM configuration */
                cpssOsFree(shadowPtr->lpmRamBlocksSizeArrayPtr);
                cpssOsFree(shadowPtr->lpmRamStructsMemPoolPtr);
                cpssOsFree(shadowPtr->lpmRamOctetsToBlockMappingPtr);

                return  rc;
            }
        }
    }

    /* free alloctated arrays used for RAM configuration */
    cpssOsFree(shadowPtr->lpmRamBlocksSizeArrayPtr);
    cpssOsFree(shadowPtr->lpmRamStructsMemPoolPtr);
    cpssOsFree(shadowPtr->lpmRamOctetsToBlockMappingPtr);

    /* go over all linked list per protocol for all octets and free it */
    for (protocol = 0; protocol < PRV_CPSS_DXCH_LPM_PROTOCOL_LAST_E; protocol++)
    {
        for (octet = 0; octet < numOfOctetsPerProtocol[protocol]; octet++)
        {
            startOfOctetList = shadowPtr->lpmMemInfoArray[protocol][octet];
            while(startOfOctetList.nextMemInfoPtr != NULL)
            {
                /* keep a temp pointer to the element in the list we need to free */
                tempNextMemInfoPtr = startOfOctetList.nextMemInfoPtr;
                /* assign a new next element for the startOfOctetList */
                startOfOctetList.nextMemInfoPtr = tempNextMemInfoPtr->nextMemInfoPtr;
                /* free the memory kept in the temp pointer */
                cpssOsFree(tempNextMemInfoPtr);
            }
        }
    }

    /* free memory allocated for default head of trie */
    if (shadowPtr->defaultAddrForHeadOfTrie)
    {
        cpssOsLpmFree((void*)shadowPtr->defaultAddrForHeadOfTrie);
    }

    /* free memory allocated to swap */
    if (shadowPtr->swapMemoryAddr != 0)
    {
        cpssOsLpmFree((void*)shadowPtr->swapMemoryAddr);
    }

    /* free memory allocated to second swap */
    if (shadowPtr->secondSwapMemoryAddr != 0)
    {
        cpssOsLpmFree((void*)shadowPtr->secondSwapMemoryAddr);
    }

    /* free memory allocated to third swap */
    if (shadowPtr->thirdSwapMemoryAddr != 0)
    {
        cpssOsLpmFree((void*)shadowPtr->thirdSwapMemoryAddr);
    }

    /* remove add devices from device list */
    if (shadowPtr->shadowDevList.shareDevs != NULL)
    {
        cpssOsLpmFree(shadowPtr->shadowDevList.shareDevs);
    }

    return GT_OK;
}


/**
* @internal createSip7ShadowStruct function
* @endinternal
*
* @brief   This function returns a pointer to a new shadow struct.
*
* @param[in] shadowType               - the type of shadow to maintain
* @param[in] protocolBitmap           - the protocols this shodow supports
* @param[in] memoryCfgPtr             - (pointer to) the memory configuration of this LPM DB
*
* @param[out] shadowPtr                - points to the created shadow struct
*                                       GT_OK on success, or
*                                       GT_FAIL otherwise.
*/
static GT_STATUS createSip7ShadowStruct
(
    IN  PRV_CPSS_DXCH_LPM_SHADOW_TYPE_ENT   shadowType,
    IN  PRV_CPSS_DXCH_LPM_PROTOCOL_BMP      protocolBitmap,
    IN  PRV_CPSS_DXCH_LPM_RAM_CONFIG_STC    *memoryCfgPtr,
    OUT PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC    *shadowPtr
)
{
    GT_U8 numOfDevs;        /* Number of devices in system.                                     */
    GT_STATUS retVal = GT_OK;
    GT_U32 memSize;         /* size of the memory block to be inserted to                       */
                            /* the memory management unit.                                      */
    GT_U32 memSizeBetweenBlocksInBytes; /* the size between each memory block management unit   */
    GT_DMM_BLOCK_SIP7  *dmmBlockPtr = NULL;
    GT_DMM_BLOCK_SIP7  *secondDmmBlockPtr = NULL;
    GT_DMM_BLOCK_SIP7  *thirdDmmBlockPtr = NULL;
    GT_DMM_BLOCK_SIP7  *defaultAddrForHeadOfTriePtr = NULL;

    GT_U32      i,j,k = 0;
    GT_U32      maxAllocSizeInLpmLines;
    GT_U32      sizeLpmEntryInBytes;
    GT_U32      minAllocationSizeInBytes;
    /* when regular node is going to be converted to compressed all 6 GONs must be reunited*/
    /* to one big one. maxGonSizeUnderDeleteForSwap is the maximal possible allocation place for it*/
    GT_U32      maxGonSizeUnderDeleteForSwap;
    GT_BOOL     isProtocolInitialized[PRV_CPSS_DXCH_LPM_PROTOCOL_LAST_E];
    GT_U32      smallBankFirstPossiblePosition = PRV_CPSS_SIP7_FIRST_SMALL_BLOCKS_POSITION;
    GT_U32      offsetCompensation = 0; /* number of banks not used by the lpm */
    GT_U32      holeSize = 0; /* number of empty banks - not used = holes */
    GT_U32      swapFirstBlockOffset;
    /* number of memories in falcon is not constant, must be taken from configuration. */
    GT_UINTPTR  structsMemPool[PRV_CPSS_DXCH_LPM_RAM_NUM_OF_MEMORIES_SIP7_CNS];
    GT_U32      numOfOctetsPerProtocol[PRV_CPSS_DXCH_LPM_PROTOCOL_LAST_E] = {PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_FOR_GON_IN_IPV4_PROTOCOL_CNS,
                                                                             PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_FOR_GON_IN_IPV6_PROTOCOL_CNS,
                                                                             PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_FOR_GON_IN_FCOE_PROTOCOL_CNS};
    GT_U32      firstSmallBankIndex=0;
    GT_BOOL     firstSmallBankIndexFound=GT_FALSE;
    GT_U32      firstBigBankIndex=0;
    GT_BOOL     firstBigBankIndexFound=GT_FALSE;
    GT_U32      startAddressOffset = 0;
    GT_U32      numDefaultAddrLines = 1;
    GT_U32      firstBlockIndex=0;
#ifdef GM_USED
    GT_U32 gmUsed = 1;
#else
    GT_U32 gmUsed = 0;
#endif

    /* zero out the shadow */
    cpssOsMemSet(shadowPtr,0,sizeof(PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC));

    numOfDevs = PRV_CPSS_MAX_PP_DEVICES_CNS;
    maxAllocSizeInLpmLines = NUMBER_OF_RANGES_IN_SUBNODE*PRV_CPSS_DXCH_LPM_RAM_FALCON_MAX_SIZE_OF_BUCKET_IN_LPM_LINES_CNS;
    sizeLpmEntryInBytes = PRV_CPSS_DXCH_LPM_RAM_SIP7_SIZE_OF_LPM_ENTRY_IN_BYTES_CNS;
    minAllocationSizeInBytes = DMM_MIN_ALLOCATE_SIZE_IN_BYTE_SIP7_CNS;
    maxGonSizeUnderDeleteForSwap = maxAllocSizeInLpmLines;

    shadowPtr->shadowType = shadowType;

    shadowPtr->lpmRamTotalBlocksSizeIncludingGap = PRV_CPSS_DXCH_LPM_RAM_TOTAL_BLOCKS_SIZE_INCLUDING_GAP_SIP7_CNS;

    for (i=0;i<memoryCfgPtr->lpmRamConfigInfoNumOfElements;i++)
    {
        shadowPtr->lpmRamConfigInfo[i].devType = memoryCfgPtr->lpmRamConfigInfo[i].devType;
        shadowPtr->lpmRamConfigInfo[i].sharedMemCnfg = memoryCfgPtr->lpmRamConfigInfo[i].sharedMemCnfg;
    }

    shadowPtr->lpmRamConfigInfoNumOfElements = memoryCfgPtr->lpmRamConfigInfoNumOfElements;

    shadowPtr->maxNumOfPbrEntries = memoryCfgPtr->maxNumOfPbrEntries;
    /* update the protocols that the shadow needs to support */
    isProtocolInitialized[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E] =
        PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_STATUS_GET_MAC(protocolBitmap);
    isProtocolInitialized[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E] =
        PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_STATUS_GET_MAC(protocolBitmap);
    isProtocolInitialized[PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E] =
        PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_STATUS_GET_MAC(protocolBitmap);

    /* reset the to be freed memory list */
    shadowPtr->freeMemListDuringUpdate = NULL;
    shadowPtr->freeMemListEndOfUpdate = NULL;

    /* we keep the numOfBlocks value as the numOfLpmMemories.
       Block and Memory has the same meaning.
       We use numOfLpmMemories due to legacy code */
    shadowPtr->numOfLpmMemories = memoryCfgPtr->numOfBlocks;
    shadowPtr->bigBanksNumber = memoryCfgPtr->bigBanksNumber;
    shadowPtr->bigBankSize = memoryCfgPtr->bigBankSize;
    shadowPtr->smallBanksNumber = memoryCfgPtr->numOfBlocks - memoryCfgPtr->bigBanksNumber;

    /* allocate needed RAM configuration arrays */
    shadowPtr->lpmRamBlocksSizeArrayPtr = cpssOsMalloc(sizeof(GT_U32)*shadowPtr->numOfLpmMemories);
    if ( shadowPtr->lpmRamBlocksSizeArrayPtr == NULL)
    {
        /* fail to allocate space */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
    }
    shadowPtr->lpmRamStructsMemPoolPtr = cpssOsMalloc(sizeof(GT_UINTPTR) * shadowPtr->numOfLpmMemories);
    if ( shadowPtr->lpmRamStructsMemPoolPtr == NULL)
    {
        /* fail to allocate space */
        cpssOsFree(shadowPtr->lpmRamBlocksSizeArrayPtr);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
    }
    shadowPtr->lpmRamOctetsToBlockMappingPtr = cpssOsMalloc(sizeof(PRV_CPSS_DXCH_LPM_RAM_OCTETS_TO_BLOCK_MAPPING_STC)*shadowPtr->numOfLpmMemories);
    if ( shadowPtr->lpmRamOctetsToBlockMappingPtr == NULL)
    {
        /* fail to allocate space */
        cpssOsFree(shadowPtr->lpmRamBlocksSizeArrayPtr);
        cpssOsFree(shadowPtr->lpmRamStructsMemPoolPtr);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
    }
    /* reset needed RAM configuration arrays */
    cpssOsMemSet(shadowPtr->lpmRamBlocksSizeArrayPtr,0,sizeof(GT_U32)*shadowPtr->numOfLpmMemories);
    cpssOsMemSet(shadowPtr->lpmRamStructsMemPoolPtr,0,sizeof(GT_UINTPTR)*shadowPtr->numOfLpmMemories);
    cpssOsMemSet(shadowPtr->lpmRamOctetsToBlockMappingPtr,0,sizeof(PRV_CPSS_DXCH_LPM_RAM_OCTETS_TO_BLOCK_MAPPING_STC)*shadowPtr->numOfLpmMemories);
    cpssOsMemSet(shadowPtr->globalMemoryBlockTakenArr,0,sizeof(shadowPtr->globalMemoryBlockTakenArr));

    shadowPtr->lpmMemMode = memoryCfgPtr->lpmMemMode;

    shadowPtr->lpmRamBlocksAllocationMethod = memoryCfgPtr->blocksAllocationMethod;
    if ( ((memoryCfgPtr->bigBanksNumber + memoryCfgPtr->numberOfBigPbrBanks) < smallBankFirstPossiblePosition) &&
         ((memoryCfgPtr->numOfBlocks-memoryCfgPtr->bigBanksNumber) > 0)) /* if we have small banks */
    {
        /* this parameters are needed for bankIndex calculation in case of small banks */
        shadowPtr->memoryOffsetStartHoleValue = memoryCfgPtr->bigBanksNumber + memoryCfgPtr->numberOfBigPbrBanks;
        shadowPtr->memoryOffsetEndHoleValue   = smallBankFirstPossiblePosition;

        holeSize = shadowPtr->memoryOffsetEndHoleValue - shadowPtr->memoryOffsetStartHoleValue;

        if (((memoryCfgPtr->bigBanksNumber + memoryCfgPtr->numberOfBigPbrBanks)==0)||/* no big blocks - only small blocks */
            (memoryCfgPtr->bigBanksNumber==0))/* no free big blocks */
        {
            swapFirstBlockOffset=smallBankFirstPossiblePosition;/* block40 */
        }
        else
        {
            if (memoryCfgPtr->numberOfBigPbrBanks==0)
            {
                swapFirstBlockOffset = 0; /* block0 */
            }
            else
            {
                swapFirstBlockOffset = memoryCfgPtr->numberOfBigPbrBanks;
            }
        }
    }
    else
    {
        /* no holes */
        shadowPtr->memoryOffsetStartHoleValue=0;
        shadowPtr->memoryOffsetEndHoleValue=0;

        swapFirstBlockOffset = memoryCfgPtr->numberOfBigPbrBanks;/* swap will be located in the first free block after the pbr blocks */
    }
    offsetCompensation = holeSize + memoryCfgPtr->numberOfBigPbrBanks;/* this parameter is needed for going over all the memory blocks */
    shadowPtr->memoryOffsetValue = memoryCfgPtr->numberOfBigPbrBanks;/* this parameter is needed for bankIndex calculation in case of big banks */

    memoryCfgPtr->smallBanksIndexesBitMap.l[0] = 0;
    memoryCfgPtr->smallBanksIndexesBitMap.l[1] = 0;
    memoryCfgPtr->bigBanksIndexesBitMap.l[0]   = 0;
    memoryCfgPtr->bigBanksIndexesBitMap.l[1]   = 0;

    /* init the search memory pools */
    for (i = 0; i < shadowPtr->numOfLpmMemories + offsetCompensation; i++)
    {
        if (memoryCfgPtr->blocksSizeArray[i] == 0)
        {
            /* it is used for PBR or we have hole when number of big banks less than 40 */
            continue;
        }
        shadowPtr->lpmRamBlocksSizeArrayPtr[k] = memoryCfgPtr->blocksSizeArray[i];

        if (memoryCfgPtr->blocksSizeArray[i]==memoryCfgPtr->bigBankSize)
        {
            if (k<32) {
                /* update big banks locations 0-31 */
                memoryCfgPtr->bigBanksIndexesBitMap.l[0] |= 1 << k;
            }
            else
            {
                /* update big banks locations 32-47 */
                memoryCfgPtr->bigBanksIndexesBitMap.l[1] |= 1 << (k%32);
            }
        }
        else
        {
            if (k<32)
            {
                /* update small banks locations */
                memoryCfgPtr->smallBanksIndexesBitMap.l[0] |= 1<<k;
            }
            else
            {
                /* update small banks locations */
                memoryCfgPtr->smallBanksIndexesBitMap.l[1] |= 1<<(k%32);
            }
        }

        /* Swap area is in the first block. In order to leave space for it, we
           deduct the size of a max LPM bucket */
        if (k == 0)
        {
             /* in the first block there must be space for 3 swap areas in case the first block is a big block,
                otherwise we will set 3 swap areas in 3 different small banks */
            if (shadowPtr->lpmRamBlocksSizeArrayPtr[0] < (3*maxGonSizeUnderDeleteForSwap))
            {
                /* one swap area should fit into a small bank */
                if (shadowPtr->lpmRamBlocksSizeArrayPtr[0] < maxGonSizeUnderDeleteForSwap)
                {
                    cpssOsFree(shadowPtr->lpmRamBlocksSizeArrayPtr);
                    cpssOsFree(shadowPtr->lpmRamStructsMemPoolPtr);
                    cpssOsFree(shadowPtr->lpmRamOctetsToBlockMappingPtr);
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, LOG_ERROR_NO_MSG);
                }
                else
                {
                    memSize = shadowPtr->lpmRamBlocksSizeArrayPtr[0] - maxGonSizeUnderDeleteForSwap;

                    /* record the swap memory address (at the end of the structs memory)*/
                    dmmBlockPtr = (GT_DMM_BLOCK_SIP7*)cpssOsLpmMalloc(sizeof(GT_DMM_BLOCK_SIP7));
                    if (dmmBlockPtr == NULL)
                    {
                        cpssOsFree(shadowPtr->lpmRamBlocksSizeArrayPtr);
                        cpssOsFree(shadowPtr->lpmRamStructsMemPoolPtr);
                        cpssOsFree(shadowPtr->lpmRamOctetsToBlockMappingPtr);
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
                    }

                    /* memSize is in lines, each line hold 5 words */
                    SET_OFFSET_IN_WORDS_SIP7(dmmBlockPtr,(swapFirstBlockOffset*shadowPtr->lpmRamTotalBlocksSizeIncludingGap + memSize)*PRV_CPSS_DXCH_LPM_RAM_SIP7_SIZE_OF_LPM_ENTRY_IN_WORDS_CNS);

                    /* - mark swap dmm block by 0xFFFFFFFE partition Id.*/
            #if __WORDSIZE == 64
                    dmmBlockPtr->nextBySizeOrPartitionPtr.partitionPtr = (GT_DMM_PARTITION_SIP7*)0xFFFFFFFFFFFFFFFE;
            #else
                    dmmBlockPtr->nextBySizeOrPartitionPtr.partitionPtr = (GT_DMM_PARTITION_SIP7*)0xFFFFFFFE;
            #endif
                    /* set first swap area */
                    shadowPtr->swapMemoryAddr = (GT_UINTPTR)dmmBlockPtr;
                }
            }
            else
            {
                memSize = shadowPtr->lpmRamBlocksSizeArrayPtr[0] - maxGonSizeUnderDeleteForSwap;

                /* record the swap memory address (at the end of the structs memory)*/
                dmmBlockPtr = (GT_DMM_BLOCK_SIP7*)cpssOsLpmMalloc(sizeof(GT_DMM_BLOCK_SIP7));
                if (dmmBlockPtr == NULL)
                {
                    cpssOsFree(shadowPtr->lpmRamBlocksSizeArrayPtr);
                    cpssOsFree(shadowPtr->lpmRamStructsMemPoolPtr);
                    cpssOsFree(shadowPtr->lpmRamOctetsToBlockMappingPtr);
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
                }

                /* memSize is in lines, each line hold 5 words */
                SET_OFFSET_IN_WORDS_SIP7(dmmBlockPtr,(swapFirstBlockOffset*shadowPtr->lpmRamTotalBlocksSizeIncludingGap + memSize)*PRV_CPSS_DXCH_LPM_RAM_SIP7_SIZE_OF_LPM_ENTRY_IN_WORDS_CNS);


                /* - mark swap dmm block by 0xFFFFFFFE partition Id.*/
        #if __WORDSIZE == 64
                dmmBlockPtr->nextBySizeOrPartitionPtr.partitionPtr = (GT_DMM_PARTITION_SIP7*)0xFFFFFFFFFFFFFFFE;
        #else
                dmmBlockPtr->nextBySizeOrPartitionPtr.partitionPtr = (GT_DMM_PARTITION_SIP7*)0xFFFFFFFE;
        #endif
                /* set first swap area */
                shadowPtr->swapMemoryAddr = (GT_UINTPTR)dmmBlockPtr;

                memSize = shadowPtr->lpmRamBlocksSizeArrayPtr[0] - (2*maxGonSizeUnderDeleteForSwap);
                /* record the swap memory address (at the end of the structs memory)*/
                secondDmmBlockPtr = (GT_DMM_BLOCK_SIP7*)cpssOsLpmMalloc(sizeof(GT_DMM_BLOCK_SIP7));
                if (secondDmmBlockPtr == NULL)
                {
                    cpssOsFree(shadowPtr->lpmRamBlocksSizeArrayPtr);
                    cpssOsFree(shadowPtr->lpmRamStructsMemPoolPtr);
                    cpssOsFree(shadowPtr->lpmRamOctetsToBlockMappingPtr);
                    cpssOsLpmFree(dmmBlockPtr);/* free first swap area allocated */
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
                }

                /* memSize is in lines, each line hold 5 words */
                SET_OFFSET_IN_WORDS_SIP7(secondDmmBlockPtr, (swapFirstBlockOffset*shadowPtr->lpmRamTotalBlocksSizeIncludingGap + memSize)*PRV_CPSS_DXCH_LPM_RAM_SIP7_SIZE_OF_LPM_ENTRY_IN_WORDS_CNS);


                /* - mark swap dmm block by 0xFFFFFFFE partition Id.*/
        #if __WORDSIZE == 64
                secondDmmBlockPtr->nextBySizeOrPartitionPtr.partitionPtr = (GT_DMM_PARTITION_SIP7*)0xFFFFFFFFFFFFFFFE;
        #else
                secondDmmBlockPtr->nextBySizeOrPartitionPtr.partitionPtr = (GT_DMM_PARTITION_SIP7*)0xFFFFFFFE;
        #endif

                /* set second swap area */
                shadowPtr->secondSwapMemoryAddr = (GT_UINTPTR)secondDmmBlockPtr;

                memSize = shadowPtr->lpmRamBlocksSizeArrayPtr[0] - (3*maxGonSizeUnderDeleteForSwap);
                /* record the swap memory address (at the end of the structs memory)*/
                thirdDmmBlockPtr = (GT_DMM_BLOCK_SIP7*)cpssOsLpmMalloc(sizeof(GT_DMM_BLOCK_SIP7));
                if (thirdDmmBlockPtr == NULL)
                {
                    cpssOsFree(shadowPtr->lpmRamBlocksSizeArrayPtr);
                    cpssOsFree(shadowPtr->lpmRamStructsMemPoolPtr);
                    cpssOsFree(shadowPtr->lpmRamOctetsToBlockMappingPtr);
                    cpssOsLpmFree(dmmBlockPtr);/* free first swap area allocated */
                    cpssOsLpmFree(secondDmmBlockPtr);/* free second swap area allocated */
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
                }

                /* memSize is in lines, each line hold 5 words */
                SET_OFFSET_IN_WORDS_SIP7(thirdDmmBlockPtr, (swapFirstBlockOffset*shadowPtr->lpmRamTotalBlocksSizeIncludingGap + memSize)*PRV_CPSS_DXCH_LPM_RAM_SIP7_SIZE_OF_LPM_ENTRY_IN_WORDS_CNS);

                /* - mark swap dmm block by 0xFFFFFFFE partition Id.*/
        #if __WORDSIZE == 64
                thirdDmmBlockPtr->nextBySizeOrPartitionPtr.partitionPtr = (GT_DMM_PARTITION_SIP7*)0xFFFFFFFFFFFFFFFE;
        #else
                thirdDmmBlockPtr->nextBySizeOrPartitionPtr.partitionPtr = (GT_DMM_PARTITION_SIP7*)0xFFFFFFFE;
        #endif

                /* set third swap area */
                shadowPtr->thirdSwapMemoryAddr = (GT_UINTPTR)thirdDmmBlockPtr;
            }
        }
        else
        {   /* 3 swap areas need to be defined in 3 small banks */
            if ((k==1||k==2) && (shadowPtr->bigBanksNumber==0))
            {
                /* one swap area should fit into a small bank */
                if (shadowPtr->lpmRamBlocksSizeArrayPtr[k] < maxGonSizeUnderDeleteForSwap)
                {
                    cpssOsFree(shadowPtr->lpmRamBlocksSizeArrayPtr);
                    cpssOsFree(shadowPtr->lpmRamStructsMemPoolPtr);
                    cpssOsFree(shadowPtr->lpmRamOctetsToBlockMappingPtr);
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, LOG_ERROR_NO_MSG);
                }

                if (k==1)
                {
                    memSize = shadowPtr->lpmRamBlocksSizeArrayPtr[k] - maxGonSizeUnderDeleteForSwap;
                    /* record the swap memory address (at the end of the structs memory)*/
                    secondDmmBlockPtr = (GT_DMM_BLOCK_SIP7*)cpssOsLpmMalloc(sizeof(GT_DMM_BLOCK_SIP7));
                    if (secondDmmBlockPtr == NULL)
                    {
                        cpssOsFree(shadowPtr->lpmRamBlocksSizeArrayPtr);
                        cpssOsFree(shadowPtr->lpmRamStructsMemPoolPtr);
                        cpssOsFree(shadowPtr->lpmRamOctetsToBlockMappingPtr);
                        cpssOsLpmFree((GT_DMM_BLOCK_SIP7  *)shadowPtr->swapMemoryAddr);/* free first swap area allocated */
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
                    }

                    /* memSize is in lines, each line hold 5 words */
                    SET_OFFSET_IN_WORDS_SIP7(secondDmmBlockPtr, PRV_CPSS_DXCH_LPM_RAM_SIP7_SIZE_OF_LPM_ENTRY_IN_WORDS_CNS*((swapFirstBlockOffset+1)*shadowPtr->lpmRamTotalBlocksSizeIncludingGap + memSize));

                    /* - mark swap dmm block by 0xFFFFFFFE partition Id.*/
            #if __WORDSIZE == 64
                    secondDmmBlockPtr->nextBySizeOrPartitionPtr.partitionPtr = (GT_DMM_PARTITION_SIP7*)0xFFFFFFFFFFFFFFFE;
            #else
                    secondDmmBlockPtr->nextBySizeOrPartitionPtr.partitionPtr = (GT_DMM_PARTITION_SIP7*)0xFFFFFFFE;
            #endif

                    /* set second swap area */
                    shadowPtr->secondSwapMemoryAddr = (GT_UINTPTR)secondDmmBlockPtr;
                }
                else/* k==2 */
                {
                    memSize = shadowPtr->lpmRamBlocksSizeArrayPtr[k] - maxGonSizeUnderDeleteForSwap;
                    /* record the swap memory address (at the end of the structs memory)*/
                    thirdDmmBlockPtr = (GT_DMM_BLOCK_SIP7*)cpssOsLpmMalloc(sizeof(GT_DMM_BLOCK_SIP7));
                    if (thirdDmmBlockPtr == NULL)
                    {
                        cpssOsFree(shadowPtr->lpmRamBlocksSizeArrayPtr);
                        cpssOsFree(shadowPtr->lpmRamStructsMemPoolPtr);
                        cpssOsFree(shadowPtr->lpmRamOctetsToBlockMappingPtr);
                        cpssOsLpmFree((GT_DMM_BLOCK_SIP7  *)shadowPtr->swapMemoryAddr);/* free first swap area allocated */
                        cpssOsLpmFree((GT_DMM_BLOCK_SIP7  *)shadowPtr->secondSwapMemoryAddr);/* free second swap area allocated */
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
                    }

                    /* memSize is in lines, each line hold 5 words */
                    SET_OFFSET_IN_WORDS_SIP7(thirdDmmBlockPtr, PRV_CPSS_DXCH_LPM_RAM_SIP7_SIZE_OF_LPM_ENTRY_IN_WORDS_CNS*((swapFirstBlockOffset+2)*shadowPtr->lpmRamTotalBlocksSizeIncludingGap + memSize));

                    /* - mark swap dmm block by 0xFFFFFFFE partition Id.*/
            #if __WORDSIZE == 64
                    thirdDmmBlockPtr->nextBySizeOrPartitionPtr.partitionPtr = (GT_DMM_PARTITION_SIP7*)0xFFFFFFFFFFFFFFFE;
            #else
                    thirdDmmBlockPtr->nextBySizeOrPartitionPtr.partitionPtr = (GT_DMM_PARTITION_SIP7*)0xFFFFFFFE;
            #endif

                    /* set third swap area */
                    shadowPtr->thirdSwapMemoryAddr = (GT_UINTPTR)thirdDmmBlockPtr;
                }
            }
            else
            {
                memSize = shadowPtr->lpmRamBlocksSizeArrayPtr[k];
            }
        }

        /* start address offset in the partition */
        startAddressOffset = 0;
        if (k == 0)
        {
            /* Reserve one line in the first block, so that head of the trie of non-existant VRF can point to */
            memSize -= numDefaultAddrLines;
            defaultAddrForHeadOfTriePtr = (GT_DMM_BLOCK_SIP7*)cpssOsLpmMalloc(sizeof(GT_DMM_BLOCK_SIP7));
            if (defaultAddrForHeadOfTriePtr == NULL)
            {
                cpssOsFree(shadowPtr->lpmRamBlocksSizeArrayPtr);
                cpssOsFree(shadowPtr->lpmRamStructsMemPoolPtr);
                cpssOsFree(shadowPtr->lpmRamOctetsToBlockMappingPtr);
                cpssOsLpmFree(dmmBlockPtr);/* free first swap area allocated */
                cpssOsLpmFree(secondDmmBlockPtr); /* free second swap area allocated */
                cpssOsLpmFree(thirdDmmBlockPtr);/* free third swap area allocated */

                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
            }

            /* memSize is in lines, each line hold 5 words */
            SET_OFFSET_IN_WORDS_SIP7(defaultAddrForHeadOfTriePtr, (swapFirstBlockOffset*shadowPtr->lpmRamTotalBlocksSizeIncludingGap)*PRV_CPSS_DXCH_LPM_RAM_SIP7_SIZE_OF_LPM_ENTRY_IN_WORDS_CNS);
            startAddressOffset = numDefaultAddrLines * DMM_MIN_ALLOCATE_SIZE_IN_BYTE_SIP7_CNS;

            shadowPtr->defaultAddrForHeadOfTrie = (GT_UINTPTR) defaultAddrForHeadOfTriePtr;
        }

        memSizeBetweenBlocksInBytes  = shadowPtr->lpmRamTotalBlocksSizeIncludingGap * sizeLpmEntryInBytes;

        if (prvCpssDmmCreatePartitionSip7(minAllocationSizeInBytes * memSize,
                                      i * memSizeBetweenBlocksInBytes + startAddressOffset,
                                      minAllocationSizeInBytes,
                                      minAllocationSizeInBytes * maxAllocSizeInLpmLines,
                                      &structsMemPool[k]) != GT_OK)
        {
            cpssOsFree(shadowPtr->lpmRamBlocksSizeArrayPtr);
            cpssOsFree(shadowPtr->lpmRamStructsMemPoolPtr);
            cpssOsFree(shadowPtr->lpmRamOctetsToBlockMappingPtr);
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
        }

        /* keep all memory pool Id's for future use, when binding octets to the blocks */
        shadowPtr->lpmRamStructsMemPoolPtr[k] = structsMemPool[k];
        k++;
    }
    if (k != shadowPtr->numOfLpmMemories)
    {
        cpssOsPrintf(" wrong banks number!!!!!!!!!!! %d\n", shadowPtr->numOfLpmMemories);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    /* update allocation order of blocks according to octet index and memory mode
               Mode          /        Allocate small banks      /     Allocate big banks
      (total up to 30 banks)    (allocate big if no more small)  (allocate small if no more big)
      #Small banks  #Big banks           #Octets                       #Octets
            20          0                   All                           -         CPSS_DXCH_CFG_SHARED_TABLE_MODE_MIN_L3_MAX_L2_NO_EM_E
            20          8                   1, 7-15                   0,2-6         not supported yet
            18          12                  1, 7-15                   0,2-6         not supported yet
            14          16                  1, 7-15                   0,2-6         not supported yet
            10          20                  1, 9-15                   0,2-8         not supported yet
            6           24                  1, 9-15                   0,2-8         CPSS_DXCH_CFG_SHARED_TABLE_MODE_MID_L3_MID_L2_MIN_EM_E / MAX_EM_E
            2           28                  1                         0,2-16        CPSS_DXCH_CFG_SHARED_TABLE_MODE_MAX_L3_MIN_L2_NO_EM_E

      */

    shadowPtr->octetsGettingSmallBanksPriorityBitMap = memoryCfgPtr->octetsGettingSmallBanksPriorityBitMap;
    shadowPtr->octetsGettingBigBanksPriorityBitMap   = memoryCfgPtr->octetsGettingBigBanksPriorityBitMap;
    shadowPtr->smallBanksIndexesBitMap.l[0]          = memoryCfgPtr->smallBanksIndexesBitMap.l[0];
    shadowPtr->smallBanksIndexesBitMap.l[1]          = memoryCfgPtr->smallBanksIndexesBitMap.l[1];
    shadowPtr->bigBanksIndexesBitMap.l[0]            = memoryCfgPtr->bigBanksIndexesBitMap.l[0];
    shadowPtr->bigBanksIndexesBitMap.l[1]            = memoryCfgPtr->bigBanksIndexesBitMap.l[1];

    for (i = 0; i < PRV_CPSS_DXCH_LPM_PROTOCOL_LAST_E; i++)
    {
        firstSmallBankIndexFound=GT_FALSE;
        firstBigBankIndexFound=GT_FALSE;

        /* in case there is no big blocks we set 2 small blocks to octet0 and octet1,
           if there are big blocks we set a big block to octet0 and small block to octet1 */
        if (shadowPtr->bigBanksNumber==0)
        {
            /* for all the protocols octet0 is mapped to the first small bank
              octet1 is mapped to the second small bank */
            for (k=0;k<shadowPtr->numOfLpmMemories;k++)
            {
                if (((k<32)&&(((shadowPtr->smallBanksIndexesBitMap.l[0]) & (1<<k))!=0)) ||
                    ((k>=32)&&(((shadowPtr->smallBanksIndexesBitMap.l[1]) & (1<<(k%32)))!=0)))
                {
                    if (firstSmallBankIndexFound==GT_FALSE)
                    {
                        firstSmallBankIndex = k;
                        firstSmallBankIndexFound=GT_TRUE;
                        continue;
                    }
                }
            }

            firstBlockIndex=firstSmallBankIndex;
        }
        else
        {
            /* for all the protocols octet 0 is mapped to the first big bank
               for all the protocols octet 1 is mapped to the first small bank*/
            /* get the first free small bank index */
            for (k=0;k<shadowPtr->numOfLpmMemories;k++)
            {
                if ((firstBigBankIndexFound==GT_TRUE)&&(firstSmallBankIndexFound==GT_TRUE))
                {
                    break;
                }

                if (((k<32)&&(((shadowPtr->bigBanksIndexesBitMap.l[0]) & (1 << k)) != 0))||
                    ((k>=32)&&(((shadowPtr->bigBanksIndexesBitMap.l[1]) & (1 << (k%32))) != 0)))
                {
                    if (firstBigBankIndexFound==GT_FALSE)
                    {
                        firstBigBankIndex = k;
                        firstBigBankIndexFound=GT_TRUE;
                    }
                }
                if (((k<32)&&(((shadowPtr->smallBanksIndexesBitMap.l[0])&(1<<k))!=0))||
                    ((k>=32)&&(((shadowPtr->smallBanksIndexesBitMap.l[1])&(1<<(k%32)))!=0)))
                {
                    if (firstSmallBankIndexFound==GT_FALSE)
                    {
                        firstSmallBankIndex = k;
                        firstSmallBankIndexFound=GT_TRUE;
                    }
                }
            }

            firstBlockIndex=firstBigBankIndex;
        }

/*GM do not support new memory banks logic,
  need to stay with old logic that give:
  bank0 to octet0 and bank1 to octet1 */
        if(gmUsed)
        {
            firstBlockIndex=0;
        }

        /* for all the protocols octet0 mapping*/
        shadowPtr->lpmMemInfoArray[i][0].structsMemPool = shadowPtr->lpmRamStructsMemPoolPtr[firstBlockIndex];
        shadowPtr->lpmMemInfoArray[i][0].ramIndex = firstBlockIndex;
        shadowPtr->lpmMemInfoArray[i][0].structsBase = 0;
        shadowPtr->lpmMemInfoArray[i][0].nextMemInfoPtr = NULL;

        /* octet0 was mapped above */
        for (j = 1; j < numOfOctetsPerProtocol[i]; j++)
        {
            /* reset all blocks that are not mapped */
            shadowPtr->lpmMemInfoArray[i][j].structsMemPool = 0;
            shadowPtr->lpmMemInfoArray[i][j].ramIndex = 0;
            shadowPtr->lpmMemInfoArray[i][j].structsBase = 0;
            shadowPtr->lpmMemInfoArray[i][j].nextMemInfoPtr = NULL;
        }

        /* firstBlockIndex is used by all the protocols for octet0  - mark bit firstBlockIndex as 0x1*/
        PRV_CPSS_DXCH_LPM_RAM_OCTET_TO_BLOCK_MAPPING_SET_MAC(shadowPtr,i,0,firstBlockIndex);
    }

    /* firstBlockIndex is used by default entries */
    shadowPtr->lpmRamOctetsToBlockMappingPtr[firstBlockIndex].isBlockUsed=GT_TRUE;

    /* IPv4 init the lpm level's memory usage */
    /* meaning of i - octet in IP address used for level in LPM search .*/
    for (i = 0; i < PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_FOR_GON_IN_IPV4_PROTOCOL_CNS ; i++)
    {
        /* for the uc search */
        shadowPtr->ucSearchMemArrayPtr[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E][i] =
            &shadowPtr->lpmMemInfoArray[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E][i];

        /* for the mc grp search */
        shadowPtr->mcSearchMemArrayPtr[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E][i] =
            &shadowPtr->lpmMemInfoArray[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E][i];

        /* for the mc src search */
        shadowPtr->mcSearchMemArrayPtr[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E][i+PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_FOR_GON_IN_IPV4_PROTOCOL_CNS] =
            &shadowPtr->lpmMemInfoArray[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E][i];
    }

     /* IPv6 init the lpm level's memory usage */
     /* meaning of i - octet in IP address used for level in LPM search .*/
    for (i = 0; i < PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_FOR_GON_IN_IPV6_PROTOCOL_CNS ; i++)
    {
        /* for the uc search */
        shadowPtr->ucSearchMemArrayPtr[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E][i] =
            &shadowPtr->lpmMemInfoArray[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E][i];

        /* for the mc grp search */
        shadowPtr->mcSearchMemArrayPtr[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E][i] =
            &shadowPtr->lpmMemInfoArray[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E][i];

        /* for the mc src search */
        shadowPtr->mcSearchMemArrayPtr[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E][i+PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_FOR_GON_IN_IPV6_PROTOCOL_CNS] =
            &shadowPtr->lpmMemInfoArray[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E][i];
    }

    /* FCoE init the lpm level's memory usage */
    /* meaning of i - octet in FCoE address used for level in LPM search .*/
    for (i = 0; i < PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_FOR_GON_IN_FCOE_PROTOCOL_CNS ; i++)
    {
        /* for the uc search */
        shadowPtr->ucSearchMemArrayPtr[PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E][i] =
            &shadowPtr->lpmMemInfoArray[PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E][i];
    }

    /* Allocate the device list */
    shadowPtr->shadowDevList.shareDevs = (GT_U8*)cpssOsLpmMalloc(sizeof(GT_U8) * numOfDevs);

    if (shadowPtr->shadowDevList.shareDevs == NULL)
    {
        cpssOsFree(shadowPtr->lpmRamBlocksSizeArrayPtr);
        cpssOsFree(shadowPtr->lpmRamStructsMemPoolPtr);
        cpssOsFree(shadowPtr->lpmRamOctetsToBlockMappingPtr);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
    }

    /* set the working devlist (which is empty curretly)*/
    shadowPtr->workDevListPtr = &shadowPtr->shadowDevList;

    cpssOsMemCpy(shadowPtr->isProtocolInitialized, isProtocolInitialized, PRV_CPSS_DXCH_LPM_PROTOCOL_LAST_E * sizeof(GT_BOOL));

    shadowPtr->vrfTblSize = PRV_CPSS_DXCH_SIP7_LPM_RAM_NUM_OF_VIRTUAL_ROUTERS_CNS;
    shadowPtr->vrRootBucketArray =
        cpssOsLpmMalloc(sizeof(PRV_CPSS_DXCH_LPM_RAM_ROOT_BUCKET_STC) * shadowPtr->vrfTblSize);
    if (shadowPtr->vrRootBucketArray == NULL)
    {
        cpssOsFree(shadowPtr->lpmRamBlocksSizeArrayPtr);
        cpssOsFree(shadowPtr->lpmRamStructsMemPoolPtr);
        cpssOsFree(shadowPtr->lpmRamOctetsToBlockMappingPtr);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
    }
    for (i = 0; i < shadowPtr->vrfTblSize; i++)
    {
        shadowPtr->vrRootBucketArray[i].valid = GT_FALSE;
        shadowPtr->vrRootBucketArray[i].rootBucket[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E] = NULL;
        shadowPtr->vrRootBucketArray[i].rootBucket[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E] = NULL;
        shadowPtr->vrRootBucketArray[i].rootBucket[PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E] = NULL;
    }

    shadowPtr->defragSip6MergeEnable = GT_TRUE;

    return retVal;
}

/**
* @internal prvCpssDxChLpmRamDbCreateSip7 function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         LPM DB is a database that holds LPM shadow information that is shared
*         to one PP or more. The LPM DB manage adding, deleting and searching
*         This function creates LPM DB for a shared LPM managment.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                  Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] lpmDbPtrPtr              - the LPM DB
* @param[in] protocolBitmap           - the protocols that are used in this LPM DB
* @param[in] memoryCfgPtr             - (pointer to) the memory configuration of this LPM DB
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on illegal parameter
* @retval GT_OUT_OF_CPU_MEM        - on failure to allocate memory
* @retval GT_FAIL                  - on error
*/
GT_STATUS prvCpssDxChLpmRamDbCreateSip7
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC  **lpmDbPtrPtr,
    IN PRV_CPSS_DXCH_LPM_PROTOCOL_BMP        protocolBitmap,
    IN PRV_CPSS_DXCH_LPM_RAM_CONFIG_STC      *memoryCfgPtr
)
{
    GT_STATUS                            retVal = GT_OK;
    GT_U32                               shadowIdx;
    PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC *lpmDbPtr;
    GT_U32                               numOfShadowCfg = 1;

    *lpmDbPtrPtr = (PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC*)
                    cpssOsLpmMalloc(sizeof(PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC));
    if (*lpmDbPtrPtr == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
    }

    lpmDbPtr = *lpmDbPtrPtr;

    /* allocate memory */
    lpmDbPtr->shadowArray =
        cpssOsLpmMalloc(sizeof(PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC) * numOfShadowCfg);
    if (lpmDbPtr->shadowArray == NULL)
    {
        cpssOsLpmFree(lpmDbPtr);
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
    }

    /*copy out the lpm db info */
    lpmDbPtr->protocolBitmap = protocolBitmap;
    lpmDbPtr->numOfShadowCfg = numOfShadowCfg;

    /* now initlize the shadows */
    for (shadowIdx = 0; (shadowIdx < numOfShadowCfg) && (retVal == GT_OK);
          shadowIdx++)
    {
        retVal = createSip7ShadowStruct(PRV_CPSS_DXCH_LPM_RAM_SIP7_SHADOW_E,
                                        protocolBitmap,
                                        memoryCfgPtr,
                                        &lpmDbPtr->shadowArray[shadowIdx]);
        if (retVal!=GT_OK)
        {
           cpssOsLpmFree(lpmDbPtr);
           return retVal;
        }
    }

    return retVal;
}

/**
* @internal prvCpssDxChLpmRamDbDeleteSip7 function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         LPM DB is a database that holds LPM shadow information that is shared
*         to one PP or more. The LPM DB manage adding, deleting and searching
*         IPv4/6 UC/MC and FCoE prefixes.
*         This function deletes LPM DB for a shared LPM managment.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                  Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] lpmDbPtr                 - the LPM DB
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on illegal parameter
* @retval GT_OUT_OF_CPU_MEM        - on failure to allocate memory
* @retval GT_FAIL                  - on error
*
* @note Deleting LPM DB is allowed only if no VR present in the LPM DB (since
*       prefixes reside within VR it means that no prefixes present as well).
*
*/
GT_STATUS prvCpssDxChLpmRamDbDeleteSip7
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC  *lpmDbPtr
)
{
    GT_STATUS                           rc;         /* function return code */
    GT_U32                              i;

    /* make sure the lpm db id exists */
    /* delete all shadows */
    for (i = 0 ; i < lpmDbPtr->numOfShadowCfg ; i++)
    {
        rc = deleteSip7ShadowStruct(&lpmDbPtr->shadowArray[i]);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    /* free resources used by the lpm db */
    if (lpmDbPtr->shadowArray != NULL)
    {
        cpssOsLpmFree(lpmDbPtr->shadowArray);
    }

    cpssOsLpmFree(lpmDbPtr);

    return GT_OK;
}

/**
* @internal prvCpssDxChLpmRamDbConfigGetSip7 function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         LPM DB is a database that holds LPM shadow information that is shared
*         to one PP or more. The LPM DB manages adding, deleting and searching
*         IPv4/6 UC/MC and FCoE prefixes.
*         This function retrieves configuration of LPM DB.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                  Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] lpmDbPtr                 - (pointer to) the LPM DB
*
* @param[out] protocolBitmapPtr        - (pointer to) the protocols that are used in this LPM DB
* @param[out] memoryCfgPtr             - (pointer to) the memory configuration of this LPM DB
*
* @retval GT_OK                    - on success
*/
GT_STATUS prvCpssDxChLpmRamDbConfigGetSip7
(
    IN  PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC    *lpmDbPtr,
    OUT PRV_CPSS_DXCH_LPM_PROTOCOL_BMP          *protocolBitmapPtr,
    OUT PRV_CPSS_DXCH_LPM_RAM_CONFIG_STC        *memoryCfgPtr
)
{
    GT_U32 i=0;

    if (protocolBitmapPtr!=NULL)
    {
        *protocolBitmapPtr = lpmDbPtr->protocolBitmap;
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ERROR_NO_MSG);
    }

    if (memoryCfgPtr!=NULL)
    {
        memoryCfgPtr->numOfBlocks = lpmDbPtr->shadowArray[0].numOfLpmMemories;
        for (i=0;i<memoryCfgPtr->numOfBlocks;i++)
        {
            memoryCfgPtr->blocksSizeArray[i] = lpmDbPtr->shadowArray[0].lpmRamBlocksSizeArrayPtr[i];
        }
        memoryCfgPtr->blocksAllocationMethod = lpmDbPtr->shadowArray[0].lpmRamBlocksAllocationMethod;
        memoryCfgPtr->lpmMemMode = lpmDbPtr->shadowArray[0].lpmMemMode;
        memoryCfgPtr->bigBankSize = lpmDbPtr->shadowArray[0].bigBankSize;
        memoryCfgPtr->bigBanksNumber = lpmDbPtr->shadowArray[0].bigBanksNumber;
        for (i=0;i<lpmDbPtr->shadowArray[0].lpmRamConfigInfoNumOfElements;i++)
        {
             memoryCfgPtr->lpmRamConfigInfo[i].devType = lpmDbPtr->shadowArray[0].lpmRamConfigInfo[i].devType;
             memoryCfgPtr->lpmRamConfigInfo[i].sharedMemCnfg = lpmDbPtr->shadowArray[0].lpmRamConfigInfo[i].sharedMemCnfg;
        }
        memoryCfgPtr->lpmRamConfigInfoNumOfElements = lpmDbPtr->shadowArray[0].lpmRamConfigInfoNumOfElements;
        memoryCfgPtr->maxNumOfPbrEntries = lpmDbPtr->shadowArray[0].maxNumOfPbrEntries;
    }
    else
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PTR, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal isProtocolSetInBitmapSip7 function
* @endinternal
*
* @brief   Check whether the protocol is set in the protocols bitmap
*
* @param[in] protocol                 - bitmap of protocols
* @param[in] protocol                 - the protocol
*
* @retval GT_TRUE                  - the protocol bit is set in the bitmap, or
* @retval GT_FALSE                 - the protocol bit is not set in the bitmap
*/
static GT_BOOL isProtocolSetInBitmapSip7
(
    IN PRV_CPSS_DXCH_LPM_PROTOCOL_BMP          protocolBitmap,
    IN PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT    protocol
)
{
    if (protocol == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E)
    {
        return PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_STATUS_GET_MAC(protocolBitmap);
    }
    if (protocol == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E)
    {
        return PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_STATUS_GET_MAC(protocolBitmap);
    }
    if (protocol == PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E)
    {
        return PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_STATUS_GET_MAC(protocolBitmap);
    }
    return GT_FALSE;
}

/**
* @internal updateHwSearchStcSip7 function
* @endinternal
*
* @brief   updates the search structures accroding to the parameters passed
*
* @param[in] updateType               - indicates which and how entries will be updated
* @param[in] protocolBitmap           - bitmap of protocols to work on.
* @param[in] shadowPtr                - the shadow relevant for the devices asked to act on.
*
* @retval GT_OK                    - on success, or
* @retval GT_ERROR                 - If the vrId was not created yet, or
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS updateHwSearchStcSip7
(
    IN PRV_CPSS_DXCH_LPM_RAM_SEARCH_MEM_UPDATE_TYPE_ENT updateType,
    IN PRV_CPSS_DXCH_LPM_PROTOCOL_BMP                   protocolBitmap,
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC                 *shadowPtr
)
{
    GT_STATUS                                       retVal = GT_OK;
    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC          tmpRootRange;
    PRV_CPSS_DXCH_LPM_RAM_TRIE_UPDATE_TYPE_ENT      lpmTrieUpdateType;
    GT_U32                                          vrId;
    PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT            currProtocolStack;

    for (currProtocolStack = PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E;
          currProtocolStack < PRV_CPSS_DXCH_LPM_PROTOCOL_LAST_E;
          currProtocolStack++)
    {
        if ((isProtocolSetInBitmapSip7(protocolBitmap, currProtocolStack) == GT_FALSE) ||
            (shadowPtr->isProtocolInitialized[currProtocolStack] == GT_FALSE))
        {
            continue;
        }
        for (vrId = 0 ; vrId < shadowPtr->vrfTblSize; vrId++)
        {
            /* go over all valid VR , if it's initilized for this protocol */
            if ((shadowPtr->vrRootBucketArray[vrId].valid == GT_FALSE) ||
                (shadowPtr->vrRootBucketArray[vrId].rootBucket[currProtocolStack] == NULL))
            {
                continue;
            }

            /* if we are in update mode and the vr doesn't need an update , don't
               update */
            if ((updateType != PRV_CPSS_DXCH_LPM_RAM_SEARCH_MEM_OVERWRITE_E) &&
                (shadowPtr->vrRootBucketArray[vrId].needsHwUpdate == GT_FALSE))
            {
                continue;
            }

            retVal = prvCpssDxChLpmRamMemFreeListMngSip7(0,PRV_CPSS_DXCH_LPM_RAM_MEM_LIST_RESET_OP_E,
                                                     &shadowPtr->freeMemListDuringUpdate,shadowPtr);
            if (retVal != GT_OK)
                return retVal;

            if (shadowPtr->vrRootBucketArray[vrId].rootBucket[currProtocolStack])
            {
                switch (updateType)
                {
                case PRV_CPSS_DXCH_LPM_RAM_SEARCH_MEM_UPDATE_E:
                    lpmTrieUpdateType = PRV_CPSS_DXCH_LPM_RAM_TRIE_UPDATE_HW_ONLY_E;
                    break;
                case PRV_CPSS_DXCH_LPM_RAM_SEARCH_MEM_UPDATE_AND_ALLOC_E:
                    lpmTrieUpdateType = PRV_CPSS_DXCH_LPM_RAM_TRIE_UPDATE_HW_AND_ALLOC_MEM_E;
                    break;
                case PRV_CPSS_DXCH_LPM_RAM_SEARCH_MEM_OVERWRITE_E:
                    lpmTrieUpdateType = PRV_CPSS_DXCH_LPM_RAM_TRIE_WRITE_HW_AND_ALLOC_MEM_E;
                    break;
                CPSS_COVERITY_NON_ISSUE_BOOKMARK
                /* coverity[dead_error_begin] */
                default:
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
                }

                /* write the changes in the tree */
                tmpRootRange.lowerLpmPtr.nextBucket =
                    shadowPtr->vrRootBucketArray[vrId].rootBucket[currProtocolStack];
                tmpRootRange.pointerType =
                    (GT_U8)(shadowPtr->vrRootBucketArray[vrId].rootBucketType[currProtocolStack]);
                tmpRootRange.next = NULL;

                retVal =
                    prvCpssDxChLpmRamMngBucketTreeWriteSip7(&tmpRootRange,
                                                            shadowPtr->ucSearchMemArrayPtr[currProtocolStack],
                                                            shadowPtr,lpmTrieUpdateType,
                                                            currProtocolStack,vrId);
                if (retVal == GT_OK)
                {
                    /* now write the vr table */
                    retVal = prvCpssDxChLpmRamMngVrfEntryUpdateSip7(vrId, currProtocolStack, shadowPtr);
                    if (retVal == GT_OK)
                    {
                        /* indicate the update was done */
                        shadowPtr->vrRootBucketArray[vrId].needsHwUpdate = GT_FALSE;

                    }
                    else
                    {

                        return retVal;

                    }

                }
                else
                {
                    return retVal;
                }
            }

            /* now it's possible to free all the "end of update to be freed
               memory" memory , now that the HW is updated (and the "during
               update to be freed" memory if needed ) */
            retVal = prvCpssDxChLpmRamMemFreeListMngSip7(0,PRV_CPSS_DXCH_LPM_RAM_MEM_LIST_FREE_MEM_OP_E,
                                                     &shadowPtr->freeMemListDuringUpdate,shadowPtr);

            if (retVal != GT_OK)
            {
                return retVal;
            }

            retVal = prvCpssDxChLpmRamMemFreeListMngSip7(0,PRV_CPSS_DXCH_LPM_RAM_MEM_LIST_FREE_MEM_OP_E,
                                                     &shadowPtr->freeMemListEndOfUpdate,shadowPtr);

            if (retVal != GT_OK)
            {
                return retVal;
            }
        }
    }

    return (retVal);
}

/**
* @internal prvCpssDxChLpmRamDbDevListAddSip7 function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         This function adds list of devices to an existing LPM DB.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                  Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] lpmDbPtr                 - the LPM DB
* @param[in] devListArray[]           - array of device ids to add to the LPM DB
* @param[in] numOfDevs                - the number of device ids in the array
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on illegal parameter
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmRamDbDevListAddSip7
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC  *lpmDbPtr,
    IN GT_U8                                 devListArray[],
    IN GT_U32                                numOfDevs
)
{
    GT_STATUS                                   retVal = GT_OK;
    GT_U32                                      shadowIdx, devIdx, i;
    GT_U32                                      numOfShadowDevs=0;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_DEVS_LIST_STC  addedDevList;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_DEVS_LIST_STC  *shadowDevListPtr;
    PRV_CPSS_DXCH_MODULE_CONFIG_STC             *moduleCfgPtr;
    CPSS_SYSTEM_RECOVERY_INFO_STC               tempSystemRecovery_Info;
    GT_BOOL                                     managerHwWriteBlock;

    /* first check the devices we got fit the lpm DB shadows */
    for (devIdx = 0 ; devIdx < numOfDevs; devIdx++)
    {
        /*Lock the access to per device data base in order to avoid corruption*/
        PRV_CPSS_DXCH_DEV_CHECK_AND_CPSS_API_LOCK_MAC(devListArray[devIdx],PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

        retVal = cpssSystemRecoveryStateGet(&tempSystemRecovery_Info);
        if (retVal != GT_OK)
        {
            return retVal;
        }
        managerHwWriteBlock = SYSTEM_RECOVERY_CHECK_MANAGER_HW_WRITE_BLOCK_MAC(CPSS_SYSTEM_RECOVERY_LPM_MANAGER_E);
        if (((tempSystemRecovery_Info.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_NOT_ACTIVE_E) && (managerHwWriteBlock == GT_FALSE)) ||
            (tempSystemRecovery_Info.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_HITLESS_STARTUP_E))
        {
            retVal = prvCpssDxChLpmAccParamSetSip7(devListArray[devIdx]);
            if(retVal != GT_OK)
            {
                /* Unlock the access to per device data base */
                CPSS_API_UNLOCK_MAC(devListArray[devIdx],PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

                CPSS_LOG_ERROR_AND_RETURN_MAC(retVal, LOG_ERROR_NO_MSG);
            }
        }


        moduleCfgPtr = &(PRV_CPSS_DXCH_PP_MAC(devListArray[devIdx])->moduleCfg);
        for (shadowIdx = 0 ; shadowIdx < lpmDbPtr->numOfShadowCfg; shadowIdx++)
        {
            if ( (lpmDbPtr->shadowArray[shadowIdx].shadowType == PRV_CPSS_DXCH_LPM_RAM_SIP7_SHADOW_E) &&
                 (!PRV_CPSS_SIP_7_CHECK_MAC(devListArray[devIdx])))
            {
                /*Unlock the access to per device data base*/
                CPSS_API_UNLOCK_MAC(devListArray[devIdx],PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }

            /* SIP7 devices*/
            if(!((moduleCfgPtr->ip.numOfBigBanks >= lpmDbPtr->shadowArray[shadowIdx].bigBanksNumber) &&
                 (moduleCfgPtr->ip.numOfSmallBanks >= lpmDbPtr->shadowArray[shadowIdx].smallBanksNumber)&&
                 (moduleCfgPtr->ip.lpmBankSize >= lpmDbPtr->shadowArray[shadowIdx].bigBankSize) &&
                 (moduleCfgPtr->ip.numOfBigBanks+moduleCfgPtr->ip.numOfSmallBanks >= lpmDbPtr->shadowArray[shadowIdx].numOfLpmMemories) ))
            {
                /*Unlock the access to per device data base*/
                CPSS_API_UNLOCK_MAC(devListArray[devIdx],PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
                /* not enough memory in the device */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NO_RESOURCE, LOG_ERROR_NO_MSG);
            }

            shadowDevListPtr =
                &lpmDbPtr->shadowArray[shadowIdx].shadowDevList;

            /* ok the device fits, see that it doesn't exist already */
            for (i = 0 ; i < shadowDevListPtr->shareDevNum;i++)
            {
                if (shadowDevListPtr->shareDevs[i] == devListArray[devIdx])
                {
                   /*Unlock the access to per device data base*/
                    CPSS_API_UNLOCK_MAC(devListArray[devIdx],PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

                   /* the device exists, can't re-add it */
                   CPSS_LOG_ERROR_AND_RETURN_MAC(GT_ALREADY_EXIST, LOG_ERROR_NO_MSG);
                }
            }
        }

        /*Unlock the access to per device data base*/
        CPSS_API_UNLOCK_MAC(devListArray[devIdx],PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    }

    /* if we reached here, all the added devices are ok, add them */
    for (shadowIdx = 0 ; shadowIdx < lpmDbPtr->numOfShadowCfg; shadowIdx++)
    {
        numOfShadowDevs = 0;
        for (devIdx = 0 ; devIdx < numOfDevs; devIdx++)
        {
            PRV_SHARED_IP_LPM_DIR_LPM_RAM_SRC_GLOBAL_VAR_GET(shadowDevList)[numOfShadowDevs] = devListArray[devIdx];
            numOfShadowDevs++;
        }
        /* check if we have added device for the shadow , and if there are add
           them */
        if (numOfShadowDevs > 0)
        {
            addedDevList.shareDevNum = numOfShadowDevs;
            addedDevList.shareDevs   = PRV_SHARED_IP_LPM_DIR_LPM_RAM_SRC_GLOBAL_VAR_GET(shadowDevList);

            /* change the working devlist*/
            lpmDbPtr->shadowArray[shadowIdx].workDevListPtr = &addedDevList;
            retVal = cpssSystemRecoveryStateGet(&tempSystemRecovery_Info);
            if (retVal != GT_OK)
            {
                return retVal;
            }
            managerHwWriteBlock = SYSTEM_RECOVERY_CHECK_MANAGER_HW_WRITE_BLOCK_MAC(CPSS_SYSTEM_RECOVERY_LPM_MANAGER_E);
            if ( ((tempSystemRecovery_Info.systemRecoveryProcess != CPSS_SYSTEM_RECOVERY_PROCESS_HA_E) && (managerHwWriteBlock == GT_FALSE)) ||
                 ((tempSystemRecovery_Info.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_HA_E) &&
                 (tempSystemRecovery_Info.systemRecoveryState ==CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E)) )
            {
                retVal = updateHwSearchStcSip7(PRV_CPSS_DXCH_LPM_RAM_SEARCH_MEM_OVERWRITE_E,
                                           lpmDbPtr->protocolBitmap,
                                           &lpmDbPtr->shadowArray[shadowIdx]);
                if (retVal != GT_OK)
                {
                    return (retVal);
                }
            }

            /* change back the working devlist*/
            lpmDbPtr->shadowArray[shadowIdx].workDevListPtr =
                &lpmDbPtr->shadowArray[shadowIdx].shadowDevList;

            /* now register them */
            shadowDevListPtr = &lpmDbPtr->shadowArray[shadowIdx].shadowDevList;
            for (i = 0; i < numOfShadowDevs; i++)
            {
                shadowDevListPtr->shareDevs[shadowDevListPtr->shareDevNum] =
                    PRV_SHARED_IP_LPM_DIR_LPM_RAM_SRC_GLOBAL_VAR_GET(shadowDevList)[i];
                shadowDevListPtr->shareDevNum++;
            }
        }
    }
    return GT_OK;
}

/**
* @internal prvCpssDxChLpmRamDbDevListRemoveSip7 function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         This function removes devices from an existing LPM DB.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                  Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] lpmDbPtr                 - the LPM DB
* @param[in] devListArray[]           - array of device ids to remove from the LPM DB
* @param[in] numOfDevs                - the number of device ids in the array
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on illegal parameter
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note NONE
*
*/
GT_STATUS prvCpssDxChLpmRamDbDevListRemoveSip7
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC  *lpmDbPtr,
    IN GT_U8                                 devListArray[],
    IN GT_U32                                numOfDevs
)
{
    GT_U32                                      shadowIdx, devIdx, i, j;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_DEVS_LIST_STC  *shadowDevListPtr;
    GT_BOOL                                     devExists;

    /* first check the devices we got are ok */
    for (devIdx = 0 ; devIdx < numOfDevs; devIdx++)
    {
        /*Lock the access to per device data base in order to avoid corruption*/
        PRV_CPSS_DXCH_DEV_CHECK_AND_CPSS_API_LOCK_MAC(devListArray[devIdx],PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

        /*Unlock the access to per device data base*/
        CPSS_API_UNLOCK_MAC(devListArray[devIdx],PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);
    }

    /* first check that the devices exist in the shadows */
    for (devIdx = 0 ; devIdx < numOfDevs; devIdx++)
    {
        for (shadowIdx = 0 ; shadowIdx < lpmDbPtr->numOfShadowCfg; shadowIdx++)
        {
            shadowDevListPtr = &lpmDbPtr->shadowArray[shadowIdx].shadowDevList;

            devExists = GT_FALSE;
            for (i = 0 ; i < shadowDevListPtr->shareDevNum;i++)
            {
                if (shadowDevListPtr->shareDevs[i] == devListArray[devIdx])
                {
                    /* found it */
                    devExists = GT_TRUE;
                    break;
                }
            }
            if (devExists == GT_FALSE)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
            }
        }
    }

    /* now go and remove them from their shadow */
    for (devIdx = 0 ; devIdx < numOfDevs; devIdx++)
    {
        for (shadowIdx = 0 ; shadowIdx < lpmDbPtr->numOfShadowCfg; shadowIdx++)
        {
            shadowDevListPtr = &lpmDbPtr->shadowArray[shadowIdx].shadowDevList;

            /* ok the device fits, see that it doesn't exist already */
            for (i = 0 ; i < shadowDevListPtr->shareDevNum;i++)
            {
                if (shadowDevListPtr->shareDevs[i] == devListArray[devIdx])
                {
                    /* found it - remove it*/
                    for (j = i ; j < shadowDevListPtr->shareDevNum - 1; j++)
                    {
                        shadowDevListPtr->shareDevs[j] =
                            shadowDevListPtr->shareDevs[j+1];
                    }
                    shadowDevListPtr->shareDevNum--;
                }
            }
        }
    }
    return GT_OK;
}

/**
* @internal prvCpssDxChLpmRamDbDevListGetSip7 function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         This function retrieves the list of devices in an existing LPM DB.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                  Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] lpmDbPtr                 - the LPM DB
* @param[in,out] numOfDevsPtr             - points to the size of devListArray
* @param[in,out] numOfDevsPtr             - points to the number of devices retrieved
*
* @param[out] devListArray[]           - array of device ids in the LPM DB
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_BAD_SIZE              - in case not enough memory was allocated to device list
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The application should allocate memory for the device list array.
*       If the function returns GT_OK, then number of devices holds
*       the number of devices the function filled.
*       If the function returns GT_BAD_SIZE, then the memory allocated by the
*       application to the device list is not enough. In this case the
*       number of devices will hold the size of array needed.
*
*/
GT_STATUS prvCpssDxChLpmRamDbDevListGetSip7
(
    IN    PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC  *lpmDbPtr,
    INOUT GT_U32                                *numOfDevsPtr,
    OUT   GT_U8                                 devListArray[]
)
{
    GT_U32 i;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_DEVS_LIST_STC *shadowDevListPtr;

    shadowDevListPtr = &lpmDbPtr->shadowArray[0].shadowDevList;

    if (*numOfDevsPtr < shadowDevListPtr->shareDevNum)
    {
        *numOfDevsPtr = shadowDevListPtr->shareDevNum;
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_SIZE, LOG_ERROR_NO_MSG);
    }

    for (i = 0 ; i < shadowDevListPtr->shareDevNum ; i++)
    {
        devListArray[i] = shadowDevListPtr->shareDevs[i];
    }
    *numOfDevsPtr = shadowDevListPtr->shareDevNum;

    return GT_OK;
}



/**
* @internal removeSupportedProtocolsFromVirtualRouterSip7 function
* @endinternal
*
* @brief   This function removes all supported protocols from virtual router for specific shadow.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                  Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] vrId                     - The virtual's router ID.
* @param[in] shadowPtr                - the shadow to work on.
*                                       GT_OK on success, or
*
* @retval GT_OUT_OF_CPU_MEM        - if failed to allocate CPU memory, or
* @retval GT_OUT_OF_PP_MEM         - if failed to allocate PP memory.
* @retval GT_BAD_STATE             - if the existing VR is not empty.
*
* @note The VR must be empty from prefixes in order to be Removed!
*
*/
static GT_STATUS removeSupportedProtocolsFromVirtualRouterSip7
(
    IN GT_U32                                   vrId,
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC         *shadowPtr
)
{
    GT_STATUS                                 retVal = GT_OK;
    PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT      protStk;
    GT_UINTPTR                                handle;
    GT_U32                                    blockIndex;

    for (protStk = PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E; protStk < PRV_CPSS_DXCH_LPM_PROTOCOL_LAST_E; protStk++)
    {
        if (shadowPtr->vrRootBucketArray[vrId].rootBucket[protStk] == NULL)
        {
            continue;
        }

        handle = shadowPtr->vrRootBucketArray[vrId].rootBucket[protStk]->hwBucketOffsetHandle;

        if (0 == handle)
        {
            continue;
        }

        if (handle==0)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Error: unexpected handle=0\n");
        }
        /*need to free memory*/
        blockIndex = PRV_CPSS_DXCH_LPM_RAM_GET_SIP7_LPM_OFFSET_FROM_DMM_MAC(handle)/(shadowPtr->lpmRamTotalBlocksSizeIncludingGap);
        retVal = prvCpssDxChLpmRamSip7CalcBankNumberIndex(shadowPtr,&blockIndex);
        if (retVal != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "ERROR:illegal blockIndex - fall in holes \n");
        }
        /* set pending flag for future need */
        shadowPtr->pendingBlockToUpdateArr[blockIndex].updateDec=GT_TRUE;
        shadowPtr->pendingBlockToUpdateArr[blockIndex].numOfDecUpdates += PRV_CPSS_DXCH_LPM_RAM_GET_SIP7_LPM_SIZE_FROM_DMM_MAC(handle);

        prvCpssDmmFreeSip7(handle);
        retVal = prvCpssDxChLpmRamMngRootBucketDeleteSip7(shadowPtr, vrId, protStk);
        if (retVal != GT_OK)
        {
            /* we fail in allocation, reset pending array */
            cpssOsMemSet(shadowPtr->pendingBlockToUpdateArr,0,sizeof(shadowPtr->pendingBlockToUpdateArr));
            return retVal;
        }


        shadowPtr->vrRootBucketArray[vrId].rootBucket[protStk] = NULL;
        /* update counters for UC allocation */
        retVal = prvCpssDxChLpmRamUpdateBlockUsageCountersSip7(shadowPtr->lpmRamBlocksSizeArrayPtr,
                                                           shadowPtr->pendingBlockToUpdateArr,
                                                           shadowPtr->protocolCountersPerBlockArr,
                                                           shadowPtr->pendingBlockToUpdateArr,
                                                           protStk,
                                                           shadowPtr->numOfLpmMemories);
        if (retVal!=GT_OK)
        {
            /* reset pending array for future use */
            cpssOsMemSet(shadowPtr->pendingBlockToUpdateArr,0,sizeof(shadowPtr->pendingBlockToUpdateArr));

            return retVal;
        }
    }
    return retVal;
}

/**
* @internal lpmVirtualRouterSetSip7 function
* @endinternal
*
* @brief   This function sets a virtual router in system for specific shadow.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                  Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] vrId                       - The virtual's router ID.
* @param[in] defIpv4UcNextHopPointer    - the pointer info of the default IPv4 UC next
*                                         hop for this virtual   router.
* @param[in] defIpv6UcNextHopPointer    - the pointer info of the default IPv6 UC next
*                                         hop for this virtual router.
* @param[in] defFcoeNextHopPointer      - the pointer info of the default FCoE next
*                                         hop for this virtual router.
* @param[in] defIpv4McRoutePointer      - the pointer info of the default IPv4 MC route
*                                         for this virtual router.
* @param[in] defIpv6McRoutePointer      - the pointer info of the default IPv6 MC route
*                                         for this virtual router.
* @param[in] defIpv4ClassERoutePointer  - the pointer info of the default E class next
*                                          hop for this virtual router.
* @param[in] protocolBitmap             - types of protocol stack used in this virtual router.
* @param[in] ucSupportArr[PRV_CPSS_DXCH_LPM_RAM_NUM_OF_UC_PROTOCOLS_CNS] - Boolean array stating UC support for every protocol stack
* @param[in] mcSupportArr[PRV_CPSS_DXCH_LPM_RAM_NUM_OF_MC_PROTOCOLS_CNS] - Boolean array stating MC support for every protocol stack
* @param[in] vrEclassNonRegularUc     - GT_TRUE : e class is handling as separate range with its own default
*                                      -GT_FALSE: e class is handling as any regular UC with the same default
* @param[in] shadowPtr                - the shadow to work on.
* @param[in] updateHw                 - GT_TRUE : update the VR in the HW
*                                     - GT_FALSE : do not update the VR in the HW
*
* @retval GT_OK                    - on success
* @retval GT_OUT_OF_CPU_MEM        - if failed to allocate CPU memory, or
* @retval GT_OUT_OF_PP_MEM         - if failed to allocate PP memory.
* @retval GT_ALREADY_EXIST         - if the vr id is already used
*/
static GT_STATUS lpmVirtualRouterSetSip7
(
    IN GT_U32                                       vrId,
    IN PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC    defIpv4UcNextHopPointer,
    IN PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC    defIpv6UcNextHopPointer,
    IN PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC    defFcoeNextHopPointer,
    IN PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC    defIpv4McRoutePointer,
    IN PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC    defIpv6McRoutePointer,
    IN PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC    defIpv4ClassERoutePointer,
    IN PRV_CPSS_DXCH_LPM_PROTOCOL_BMP               protocolBitmap,
    IN GT_BOOL                                      ucSupportArr[PRV_CPSS_DXCH_LPM_RAM_NUM_OF_UC_PROTOCOLS_CNS],
    IN GT_BOOL                                      mcSupportArr[PRV_CPSS_DXCH_LPM_RAM_NUM_OF_MC_PROTOCOLS_CNS],
    IN GT_BOOL                                      vrEclassNonRegularUc,
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC             *shadowPtr,
    IN GT_BOOL                                      updateHw
)
{
    GT_STATUS                                       retVal;
    PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT            protStk;
    PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC       *defUcNextHopEntry;
    PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC       *defReservedNextHopEntry;
    PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC       *defMcNextHopEntry;
    PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC       defUcNextHopPointer;
    PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC       defMcNextHopPointer;
    PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC       defReservedNextHopPointer;

    cpssOsMemSet(&defUcNextHopPointer,0,sizeof(PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC));
    cpssOsMemSet(&defMcNextHopPointer,0,sizeof(PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC));

    if (shadowPtr->vrRootBucketArray[vrId].valid == GT_TRUE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_ALREADY_EXIST, LOG_ERROR_NO_MSG);
    }
    shadowPtr->vrRootBucketArray[vrId].treatIpv4ClassEasNonRegularUc = vrEclassNonRegularUc;
    for (protStk = PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E;
         protStk < PRV_CPSS_DXCH_LPM_PROTOCOL_LAST_E;
         protStk++)
    {
        defUcNextHopEntry = NULL;
        defReservedNextHopEntry = NULL;
        defMcNextHopEntry = NULL;

        if (isProtocolSetInBitmapSip7(protocolBitmap, protStk) == GT_FALSE)
        {
            if (protStk < PRV_CPSS_DXCH_LPM_RAM_NUM_OF_UC_PROTOCOLS_CNS)
            {
                shadowPtr->vrRootBucketArray[vrId].isUnicastSupported[protStk] = GT_FALSE;
            }
            if (protStk < PRV_CPSS_DXCH_LPM_RAM_NUM_OF_MC_PROTOCOLS_CNS)
            {
                shadowPtr->vrRootBucketArray[vrId].isMulticastSupported[protStk] = GT_FALSE;
            }
            continue;
        }

        if (protStk <= PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E)
        {
            if ((ucSupportArr[protStk] == GT_FALSE) && (mcSupportArr[protStk] == GT_FALSE))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
        }
        else
        {
            if (ucSupportArr[protStk] == GT_FALSE)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }
        }

        /* init UC structures */
        if (ucSupportArr[protStk] == GT_TRUE)
        {
            shadowPtr->vrRootBucketArray[vrId].isUnicastSupported[protStk] = GT_TRUE;
            if (protStk == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E)
            {
                defUcNextHopPointer = defIpv4UcNextHopPointer;
                if (vrEclassNonRegularUc == GT_FALSE)
                {
                    defReservedNextHopPointer = defIpv4UcNextHopPointer;
                }
                else
                {
                    defReservedNextHopPointer = defIpv4ClassERoutePointer;
                }
            }
            else if (protStk == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E)
            {
                defUcNextHopPointer = defIpv6UcNextHopPointer;
            }
            else    /* PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E */
            {
                defUcNextHopPointer = defFcoeNextHopPointer;
            }

            /* allocate space for the new default */
            defUcNextHopEntry = cpssOsLpmMalloc(sizeof(PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC));
            if (defUcNextHopEntry == NULL)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
            }
            cpssOsMemCpy(defUcNextHopEntry, &defUcNextHopPointer,
                         sizeof(PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC));
            if(protStk == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E)
            {
                if ((vrEclassNonRegularUc == GT_TRUE) || (shadowPtr->shadowType != PRV_CPSS_DXCH_LPM_RAM_SIP7_SHADOW_E))
                {
                    defReservedNextHopEntry = cpssOsLpmMalloc(sizeof(PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC));
                    if (defReservedNextHopEntry == NULL)
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
                    }
                    cpssOsMemCpy(defReservedNextHopEntry, &defReservedNextHopPointer,
                                 sizeof(PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC));
                }
                else
                {
                     defReservedNextHopEntry = defUcNextHopEntry;
                }
            }
        }
        else
        {
            shadowPtr->vrRootBucketArray[vrId].isUnicastSupported[protStk] = GT_FALSE;
        }

        /* now init mc stuctures */
        if (protStk <= PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E)
        {
            if (mcSupportArr[protStk] == GT_TRUE)
            {
                shadowPtr->vrRootBucketArray[vrId].isMulticastSupported[protStk] = GT_TRUE;
                if (protStk == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E)
                {
                    defMcNextHopPointer = defIpv4McRoutePointer;
                }
                else /* PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E */
                {
                    defMcNextHopPointer = defIpv6McRoutePointer;
                }

                defMcNextHopEntry = cpssOsLpmMalloc(sizeof(PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC));
                if (defMcNextHopEntry == NULL)
                {
                    if (ucSupportArr[protStk] == GT_TRUE)
                    {
                         cpssOsLpmFree(defUcNextHopEntry);
                    }
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
                }

                cpssOsMemCpy(defMcNextHopEntry, &defMcNextHopPointer,
                             sizeof(PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC));
            }
            else    /* mcSupportArr[protStk] == GT_FALSE */
            {
                shadowPtr->vrRootBucketArray[vrId].isMulticastSupported[protStk] = GT_FALSE;
            }
        }

        /* Create the root bucket */
        retVal = prvCpssDxChLpmRamMngRootBucketCreateSip7(shadowPtr, vrId, protStk,
                                                          defUcNextHopEntry,
                                                          defReservedNextHopEntry,
                                                          defMcNextHopEntry,
                                                          updateHw);
        if (retVal != GT_OK)
        {
            if (ucSupportArr[protStk] == GT_TRUE)
            {
                cpssOsLpmFree(defUcNextHopEntry);
            }
            if ((protStk <= PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E) && (mcSupportArr[protStk] == GT_TRUE))
            {
                if ((ucSupportArr[protStk] == GT_FALSE) ||
                    (defUcNextHopPointer.routeEntryBaseMemAddr != defMcNextHopPointer.routeEntryBaseMemAddr) ||
                    (defUcNextHopPointer.routeEntryMethod != defMcNextHopPointer.routeEntryMethod))
                {
                    cpssOsLpmFree(defMcNextHopEntry);
                }
            }
            return retVal;
        }

        /* Don't touch the hw in a HSU process and in recovery process after HA event */
        if(updateHw == GT_TRUE)
        {
            /* write the mc and uc vr router table */
            retVal = prvCpssDxChLpmRamMngVrfEntryUpdateSip7(vrId, protStk, shadowPtr);

            if (retVal != GT_OK)
            {
                if (ucSupportArr[protStk] == GT_TRUE)
                {
                    cpssOsLpmFree(defUcNextHopEntry);
                }
                if ((protStk <= PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E) && (mcSupportArr[protStk] == GT_TRUE))
                {
                    if ((ucSupportArr[protStk] == GT_FALSE) ||
                        (defUcNextHopPointer.routeEntryBaseMemAddr != defMcNextHopPointer.routeEntryBaseMemAddr) ||
                        (defUcNextHopPointer.routeEntryMethod != defMcNextHopPointer.routeEntryMethod))
                    {
                        cpssOsLpmFree(defMcNextHopEntry);
                    }
                }
                return retVal;
            }
        }
    }

    shadowPtr->vrRootBucketArray[vrId].valid = GT_TRUE;

    /* the data was written successfully to HW, we can reset the information regarding the new memoryPool allocations done.
    next call will set this array with new values of allocated/bound blocks */
    cpssOsMemSet(shadowPtr->allNewNextMemInfoAllocatedPerOctetArrayPtr,0,sizeof(shadowPtr->allNewNextMemInfoAllocatedPerOctetArrayPtr));

    cpssOsMemSet(shadowPtr->tempLpmRamOctetsToBlockMappingUsedForReconstractPtr,0,sizeof(shadowPtr->tempLpmRamOctetsToBlockMappingUsedForReconstractPtr));

    return GT_OK;
}

/**
* @internal lpmVirtualRouterRemoveSip7 function
* @endinternal
*
* @brief   This function removes a virtual router in system for specific shadow.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                  Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] vrId                     - The virtual's router ID.
* @param[in] shadowPtr                - the shadow to work on.
*                                       GT_OK on success, or
*
* @retval GT_OUT_OF_CPU_MEM        - if failed to allocate CPU memory, or
* @retval GT_OUT_OF_PP_MEM         - if failed to allocate PP memory.
* @retval GT_BAD_STATE             - if the existing VR is not empty.
*
* @note The VR must be empty from prefixes in order to be Removed!
*
*/
static GT_STATUS lpmVirtualRouterRemoveSip7
(
    IN GT_U32                                   vrId,
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC         *shadowPtr
)
{
    GT_STATUS                                 retVal = GT_OK;
    PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT      protocol;
    GT_U32                                    dev;
    GT_U32                                    headOfTrie = 0;
    CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT       nodeType = 0;

    if (shadowPtr->vrRootBucketArray[vrId].valid == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }
    /* this is an existing Vr - delete the allocated structures */
    retVal = removeSupportedProtocolsFromVirtualRouterSip7(vrId,shadowPtr);
    if (retVal != GT_OK)
    {
        return retVal;
    }

    shadowPtr->vrRootBucketArray[vrId].valid = GT_FALSE;

    nodeType   = CPSS_DXCH_LPM_COMPRESSED_NODE_PTR_TYPE_E;
    headOfTrie = PRV_CPSS_DXCH_LPM_RAM_GET_SIP7_LPM_OFFSET_FROM_DMM_MAC(shadowPtr->defaultAddrForHeadOfTrie);

    for (protocol = PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E; protocol < PRV_CPSS_DXCH_LPM_PROTOCOL_LAST_E;
          protocol++)
    {
        /* rewrite the VRF entry with default */
        for (dev = 0; dev < shadowPtr->workDevListPtr->shareDevNum; dev++)
        {
            retVal = prvCpssDxChLpmHwVrfEntryWriteSip7(shadowPtr->workDevListPtr->shareDevs[dev],
                                                   vrId,
                                                   protocol,
                                                   nodeType,
                                                   headOfTrie);
            if (retVal != GT_OK)
            {
                return retVal;
            }
        }

    }

    return retVal;
}

/**
* @internal prvCpssDxChLpmRamVirtualRouterAddSip7 function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         This function adds a virtual router for specific LPM DB.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                  Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] lpmDbPtr                 - the LPM DB
* @param[in] vrId                     - The virtual's router ID.
* @param[in] vrConfigPtr              - Configuration of the virtual router
*
* @retval GT_OK                    - on success, or
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_OUT_OF_CPU_MEM        - if failed to allocate CPU memory, or
* @retval GT_ALREADY_EXIST         - if the vr id is already used
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmRamVirtualRouterAddSip7
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC  *lpmDbPtr,
    IN GT_U32                                vrId,
    IN PRV_CPSS_DXCH_LPM_RAM_VR_CONFIG_STC   *vrConfigPtr
)
{
    GT_STATUS                              retVal = GT_OK;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC       *shadowPtr;
    GT_U32                                 shadowIdx;

    PRV_CPSS_DXCH_LPM_PROTOCOL_BMP         protocolBitmap = 0;
    PRV_CPSS_DXCH_LPM_PROTOCOL_BMP         protocolDifference;
    GT_BOOL                                vrUcSupport[PRV_CPSS_DXCH_LPM_RAM_NUM_OF_UC_PROTOCOLS_CNS];
    GT_BOOL                                vrMcSupport[PRV_CPSS_DXCH_LPM_RAM_NUM_OF_MC_PROTOCOLS_CNS];
    GT_BOOL                                vrEclassNonRegularUc;
    CPSS_SYSTEM_RECOVERY_INFO_STC          tempSystemRecovery_Info;
    GT_BOOL                                updateHw;
    GT_BOOL                                managerHwWriteBlock;


    vrUcSupport[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E] = (vrConfigPtr->supportUcIpv4) ? GT_TRUE : GT_FALSE;
    vrUcSupport[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E] = (vrConfigPtr->supportUcIpv6) ? GT_TRUE : GT_FALSE;
    vrUcSupport[PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E] = (vrConfigPtr->supportFcoe)   ? GT_TRUE : GT_FALSE;
    vrMcSupport[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E] = (vrConfigPtr->supportMcIpv4) ? GT_TRUE : GT_FALSE;
    vrMcSupport[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E] = (vrConfigPtr->supportMcIpv6) ? GT_TRUE : GT_FALSE;
    vrEclassNonRegularUc = vrConfigPtr->treatIpv4ClassEasNonRegularUc;

    /* determine the supported protocols */
    if ((vrConfigPtr->supportUcIpv4 == GT_TRUE) || (vrConfigPtr->supportMcIpv4 == GT_TRUE))
    {
        protocolBitmap |= PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_BIT_E;
    }
    if ((vrConfigPtr->supportUcIpv6 == GT_TRUE) || (vrConfigPtr->supportMcIpv6 == GT_TRUE))
    {
        protocolBitmap |= PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_BIT_E;
    }
    if (vrConfigPtr->supportFcoe == GT_TRUE)
    {
        protocolBitmap |= PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_BIT_E;
    }

    /* if initialization has not been done for the requested protocol stack -
    return error */
    protocolDifference = protocolBitmap ^ lpmDbPtr->protocolBitmap;
    if (protocolDifference)
    {
        if (protocolDifference & protocolBitmap)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
        }
    }

    for (shadowIdx = 0 ; shadowIdx < lpmDbPtr->numOfShadowCfg; shadowIdx++)
    {
        shadowPtr = &lpmDbPtr->shadowArray[shadowIdx];
        if (vrConfigPtr->supportUcIpv4 == GT_TRUE)
        {
            retVal = prvCpssDxChLpmRouteEntryPointerCheckSip7(&vrConfigPtr->defaultUcIpv4RouteEntry);
            if (retVal != GT_OK)
                return retVal;
        }
        if (vrConfigPtr->supportMcIpv4 == GT_TRUE)
        {
            retVal = prvCpssDxChLpmRouteEntryPointerCheckSip7(&vrConfigPtr->defaultMcIpv4RouteEntry);
            if (retVal != GT_OK)
                return retVal;
        }
        if (vrConfigPtr->supportUcIpv6 == GT_TRUE)
        {
            retVal = prvCpssDxChLpmRouteEntryPointerCheckSip7(&vrConfigPtr->defaultUcIpv6RouteEntry);
            if (retVal != GT_OK)
                return retVal;
        }
        if (vrConfigPtr->supportMcIpv6 == GT_TRUE)
        {
            retVal = prvCpssDxChLpmRouteEntryPointerCheckSip7(&vrConfigPtr->defaultMcIpv6RouteEntry);
            if (retVal != GT_OK)
                return retVal;
        }
        if (vrConfigPtr->supportFcoe == GT_TRUE)
        {
            retVal = prvCpssDxChLpmRouteEntryPointerCheckSip7(&vrConfigPtr->defaultFcoeForwardingEntry);
            if (retVal != GT_OK)
                return retVal;
        }
        if (vrConfigPtr->treatIpv4ClassEasNonRegularUc == GT_TRUE)
        {
            retVal = prvCpssDxChLpmRouteEntryPointerCheckSip7(&vrConfigPtr->defIpv4ClassERouteEntry);
            if (retVal != GT_OK)
                return retVal;
        }


        if (vrId >= shadowPtr->vrfTblSize)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

        retVal = cpssSystemRecoveryStateGet(&tempSystemRecovery_Info);
        if (retVal != GT_OK)
        {
            return retVal;
        }
        managerHwWriteBlock = SYSTEM_RECOVERY_CHECK_MANAGER_HW_WRITE_BLOCK_MAC(CPSS_SYSTEM_RECOVERY_LPM_MANAGER_E);
        if((tempSystemRecovery_Info.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_HA_E) ||
           (managerHwWriteBlock == GT_TRUE) )
        {
            updateHw=GT_FALSE;
        }
        else
        {
            updateHw=GT_TRUE;
        }

        retVal = lpmVirtualRouterSetSip7(vrId,
                                     vrConfigPtr->defaultUcIpv4RouteEntry,
                                     vrConfigPtr->defaultUcIpv6RouteEntry,
                                     vrConfigPtr->defaultFcoeForwardingEntry,
                                     vrConfigPtr->defaultMcIpv4RouteEntry,
                                     vrConfigPtr->defaultMcIpv6RouteEntry,
                                     vrConfigPtr->defIpv4ClassERouteEntry,
                                     protocolBitmap,
                                     vrUcSupport,
                                     vrMcSupport,
                                     vrEclassNonRegularUc,
                                     shadowPtr,
                                     updateHw);
        if (retVal != GT_OK)
        {
            if (retVal != GT_ALREADY_EXIST)
            {
                /* free all allocated resources  */
                removeSupportedProtocolsFromVirtualRouterSip7(vrId,shadowPtr);
            }
            return(retVal);
        }
    }

    return retVal;
}

/**
* @internal prvCpssDxChLpmRamVirtualRouterDelSip7 function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         This function deletes a virtual router for specific LPM DB.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                  Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] lpmDbPtr                 - the LPM DB
* @param[in] vrId                     - The virtual's router ID.
*
* @retval GT_OK                    - on success, or
* @retval GT_NOT_FOUND             - if the LPM DB id or vr id does not found
* @retval GT_OUT_OF_CPU_MEM        - if failed to allocate CPU memory, or
* @retval GT_BAD_STATE             - if the existing VR is not empty.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note All prefixes must be previously deleted.
*
*/
GT_STATUS prvCpssDxChLpmRamVirtualRouterDelSip7
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC  *lpmDbPtr,
    IN GT_U32                                vrId
)
{
    GT_STATUS                           retVal = GT_OK;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC    *shadowPtr;
    GT_U32                              shadowIdx;

    for (shadowIdx = 0 ; shadowIdx < lpmDbPtr->numOfShadowCfg; shadowIdx++)
    {
        shadowPtr = &lpmDbPtr->shadowArray[shadowIdx];

        /* delete all the IPv4 unicast prefixes for this virtual router */
        if (shadowPtr->vrRootBucketArray[vrId].isUnicastSupported[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E])
        {
                retVal = prvCpssDxChLpmRamIpv4UcPrefixesFlushSip7(lpmDbPtr,vrId);
                if (retVal != GT_OK)
                {
                    return (retVal);
                }
        }

        /* delete all the IPv4 multicast entries for this virtual router */
        if (shadowPtr->vrRootBucketArray[vrId].isMulticastSupported[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E])
        {
            retVal = prvCpssDxChLpmRamIpv4McEntriesFlushSip7(lpmDbPtr,vrId);
            if (retVal != GT_OK)
            {
                return (retVal);
            }
        }

        /* delete all the IPv6 unicast prefixes for this virtual router */
        if (shadowPtr->vrRootBucketArray[vrId].isUnicastSupported[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E])
        {
            retVal = prvCpssDxChLpmRamIpv6UcPrefixesFlushSip7(lpmDbPtr,vrId);
            if (retVal != GT_OK)
            {
                return (retVal);
            }
        }

        /* delete all the IPv6 multicast entries for this virtual router */
        if (shadowPtr->vrRootBucketArray[vrId].isMulticastSupported[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E])
        {
            retVal = prvCpssDxChLpmRamIpv6McEntriesFlushSip7(lpmDbPtr,vrId);
            if (retVal != GT_OK)
            {
                return (retVal);
            }
        }

        /* delete all the FCoE prefixes for this virtual router */
        if (shadowPtr->vrRootBucketArray[vrId].isUnicastSupported[PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E])
        {
            retVal = prvCpssDxChLpmRamFcoePrefixesFlushSip7(lpmDbPtr,vrId);
            if (retVal != GT_OK)
            {
                return (retVal);
            }
        }

        retVal = lpmVirtualRouterRemoveSip7(vrId, shadowPtr);
        if (retVal != GT_OK)
        {
            return (retVal);
        }
    }

    return retVal;
}

/**
* @internal prvCpssDxChLpmRamVirtualRouterGetSip7 function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         This function gets configuration of a virtual router for specific LPM DB.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                  Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] lpmDbPtr                 - the LPM DB.
* @param[in] vrId                     - The virtual's router ID.
*
* @param[out] vrConfigPtr              - Configuration of the virtual router
*
* @retval GT_OK                    - on success, or
* @retval GT_NOT_FOUND             - if the LPM DB id or vr id does not found
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_OUT_OF_CPU_MEM        - if failed to allocate CPU memory, or
* @retval GT_OUT_OF_PP_MEM         - if failed to allocate TCAM memory.
* @retval GT_BAD_STATE             - if the existing VR is not empty.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note none.
*
*/
GT_STATUS prvCpssDxChLpmRamVirtualRouterGetSip7
(
    IN  PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC *lpmDbPtr,
    IN  GT_U32                               vrId,
    OUT PRV_CPSS_DXCH_LPM_RAM_VR_CONFIG_STC  *vrConfigPtr
)
{
    GT_STATUS                                   retVal = GT_OK;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC            *shadowPtr;
    PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC   *defRoutePointerPtr = NULL;
    PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC   routeEntry;
    GT_IPV6ADDR                                 ipv6Addr;
    GT_IPADDR                                   ipv4Addr;
    GT_FCID                                     fcoeAddr;

    shadowPtr = &lpmDbPtr->shadowArray[0];

    if (shadowPtr->vrRootBucketArray[vrId].valid == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }

    /* IPv4 MC */
    if (shadowPtr->vrRootBucketArray[vrId].isMulticastSupported[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E] == GT_TRUE)
    {
        retVal = prvCpssDxChLpmRamMcDefRouteGetSip7(vrId ,&defRoutePointerPtr,
                                                shadowPtr ,PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E);
        if ((retVal != GT_OK) && (retVal != GT_NOT_FOUND) && (retVal != GT_NOT_INITIALIZED))
        {
            return retVal;
        }

        if ((defRoutePointerPtr == NULL) || (retVal == GT_NOT_FOUND) || (retVal == GT_NOT_INITIALIZED))
            vrConfigPtr->supportMcIpv4 = GT_FALSE;
        else
        {
            vrConfigPtr->defaultMcIpv4RouteEntry = *defRoutePointerPtr;
            vrConfigPtr->supportMcIpv4 = GT_TRUE;
        }
    }
    else
    {
        vrConfigPtr->supportMcIpv4 = GT_FALSE;
    }

    /* IPv4 UC */
    if (shadowPtr->vrRootBucketArray[vrId].isUnicastSupported[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E] == GT_TRUE)
    {
        cpssOsMemSet(&ipv4Addr, 0, sizeof(ipv4Addr));
        retVal = prvCpssDxChLpmRamIpv4UcPrefixSearchSip7(lpmDbPtr,vrId,ipv4Addr,0,&routeEntry);
        if ((retVal != GT_OK) && (retVal != GT_NOT_FOUND) && (retVal != GT_NOT_INITIALIZED))
        {
            return retVal;
        }

        if ((retVal == GT_NOT_FOUND)  || (retVal == GT_NOT_INITIALIZED))
            vrConfigPtr->supportUcIpv4 = GT_FALSE;
        else
        {
            vrConfigPtr->defaultUcIpv4RouteEntry = routeEntry;
            vrConfigPtr->supportUcIpv4 = GT_TRUE;
        }
        vrConfigPtr->treatIpv4ClassEasNonRegularUc = shadowPtr->vrRootBucketArray[vrId].treatIpv4ClassEasNonRegularUc;
        cpssOsMemSet(&ipv4Addr, 0, sizeof(ipv4Addr));
        ipv4Addr.arIP[0] = 240;
        retVal = prvCpssDxChLpmRamIpv4UcPrefixSearchSip7(lpmDbPtr,vrId,ipv4Addr,4,&routeEntry);
        if ((retVal != GT_OK) && (retVal != GT_NOT_FOUND) && (retVal != GT_NOT_INITIALIZED))
        {
            return retVal;
        }
        vrConfigPtr->defIpv4ClassERouteEntry = routeEntry;
    }
    else
    {
        vrConfigPtr->supportUcIpv4 = GT_FALSE;
    }

    /* IPv6 MC */
    if (shadowPtr->vrRootBucketArray[vrId].isMulticastSupported[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E] == GT_TRUE)
    {
        retVal = prvCpssDxChLpmRamMcDefRouteGetSip7(vrId, &defRoutePointerPtr,
                                                shadowPtr, PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E);
        if ((retVal != GT_OK) && (retVal != GT_NOT_FOUND) && (retVal != GT_NOT_INITIALIZED))
        {
            return retVal;
        }

        if ((defRoutePointerPtr == NULL) || (retVal == GT_NOT_FOUND) || (retVal == GT_NOT_INITIALIZED))
            vrConfigPtr->supportMcIpv6 = GT_FALSE;
        else
        {
            vrConfigPtr->defaultMcIpv6RouteEntry = *defRoutePointerPtr;
            vrConfigPtr->supportMcIpv6 = GT_TRUE;
        }
    }
    else
    {
        vrConfigPtr->supportMcIpv6 = GT_FALSE;
    }

    /* IPv6 UC */
    if (shadowPtr->vrRootBucketArray[vrId].isUnicastSupported[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E] == GT_TRUE)
    {
        cpssOsMemSet(&ipv6Addr, 0, sizeof(ipv6Addr));
        retVal = prvCpssDxChLpmRamIpv6UcPrefixSearchSip7(lpmDbPtr,vrId,ipv6Addr,0,&routeEntry);
        if ((retVal != GT_OK) && (retVal != GT_NOT_FOUND) && (retVal != GT_NOT_INITIALIZED))
        {
            return retVal;
        }

        if ((retVal == GT_NOT_FOUND)  || (retVal == GT_NOT_INITIALIZED))
            vrConfigPtr->supportUcIpv6 = GT_FALSE;
        else
        {
            vrConfigPtr->defaultUcIpv6RouteEntry = routeEntry;
            vrConfigPtr->supportUcIpv6 = GT_TRUE;
        }
    }
    else
    {
        vrConfigPtr->supportUcIpv6 = GT_FALSE;
    }

    /* FCoE */
    if (shadowPtr->vrRootBucketArray[vrId].isUnicastSupported[PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E] == GT_TRUE)
    {
        cpssOsMemSet(&fcoeAddr, 0, sizeof(fcoeAddr));
        retVal = prvCpssDxChLpmRamFcoePrefixSearchSip7(lpmDbPtr,vrId,fcoeAddr,0,&routeEntry);
        if ((retVal != GT_OK) && (retVal != GT_NOT_FOUND) && (retVal != GT_NOT_INITIALIZED))
        {
            return retVal;
        }

        if ((retVal == GT_NOT_FOUND)  || (retVal == GT_NOT_INITIALIZED))
            vrConfigPtr->supportFcoe = GT_FALSE;
        else
        {
            vrConfigPtr->defaultFcoeForwardingEntry = routeEntry;
            vrConfigPtr->supportFcoe = GT_TRUE;
        }
    }
    else
    {
        vrConfigPtr->supportFcoe = GT_FALSE;
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChLpmRamIpv4UcPrefixAddSip7 function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         This function adds a new IPv4 prefix to a Virtual Router in a
*         specific LPM DB or overrides an existing existing IPv4 prefix.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                  Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] lpmDbPtr                 - The LPM DB
* @param[in] vrId                     - The virtual router id.
* @param[in] ipAddr                   - The destination IP address of this prefix.
* @param[in] prefixLen                - The number of bits that are actual valid in the ipAddr.
* @param[in] nextHopInfoPtr           - (points to) The next hop pointer to set for this prefix.
* @param[in] override                 -  the existing entry if it already exists
* @param[in] defragmentationEnable    - whether to enable performance costing
*                                      de-fragmentation process in the case that there
*                                      is no place to insert the prefix. To point of the
*                                      process is just to make space for this prefix.
*
* @retval GT_OK                    - on success, or
* @retval GT_OUT_OF_RANGE          - If prefix length is too big, or
* @retval GT_ERROR                 - If the vrId was not created yet, or
* @retval GT_ALREADY_EXIST         - prefix already exist when override is GT_FALSE
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_OUT_OF_CPU_MEM        - If failed to allocate CPU memory, or
* @retval GT_OUT_OF_PP_MEM         - If failed to allocate PP memory, or
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note To change the default prefix for the VR use prefixLen = 0.
*
*/
GT_STATUS prvCpssDxChLpmRamIpv4UcPrefixAddSip7
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC         *lpmDbPtr,
    IN GT_U32                                       vrId,
    IN GT_IPADDR                                    ipAddr,
    IN GT_U32                                       prefixLen,
    IN PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC    *nextHopInfoPtr,
    IN GT_BOOL                                      override,
    IN GT_BOOL                                      defragmentationEnable
)
{
    GT_STATUS           retVal = GT_OK;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC *shadowPtr;
    GT_U32 shadowIdx;
    PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_MODE_ENT   insertMode;
    CPSS_SYSTEM_RECOVERY_INFO_STC tempSystemRecovery_Info;
    GT_BOOL                       managerHwWriteBlock;

    /* if initialization has not been done for the requested protocol stack -
    return error */
    if (PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_STATUS_GET_MAC(lpmDbPtr->protocolBitmap) == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    for (shadowIdx = 0 ; shadowIdx < lpmDbPtr->numOfShadowCfg; shadowIdx++)
    {
        shadowPtr = &lpmDbPtr->shadowArray[shadowIdx];

        retVal = prvCpssDxChLpmRouteEntryPointerCheckSip7(nextHopInfoPtr);
        if (retVal != GT_OK)
            return retVal;


        retVal = cpssSystemRecoveryStateGet(&tempSystemRecovery_Info);
        if (retVal != GT_OK)
        {
            return retVal;
        }
        managerHwWriteBlock = SYSTEM_RECOVERY_CHECK_MANAGER_HW_WRITE_BLOCK_MAC(CPSS_SYSTEM_RECOVERY_LPM_MANAGER_E);
        if((tempSystemRecovery_Info.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_HA_E) ||
           (managerHwWriteBlock == GT_TRUE))
        {
            insertMode=PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_SDW_ONLY_MODE_E;
        }
        else
        {
            insertMode=PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_SDW_MEM_HW_MODE_E;
        }

        retVal = prvCpssDxChLpmSip7RamUcEntryAdd(vrId,
                                                 ipAddr.arIP,
                                                 prefixLen,
                                                 nextHopInfoPtr,
                                                 insertMode,
                                                 override,
                                                 PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E,
                                                 shadowPtr,
                                                 defragmentationEnable);
        if (retVal != GT_OK)
        {
            /* reset the array for next use */
            cpssOsMemSet(shadowPtr->globalMemoryBlockTakenArr, 0, sizeof(shadowPtr->globalMemoryBlockTakenArr));
            break;
        }
        /* reset the array for next use */
        cpssOsMemSet(shadowPtr->globalMemoryBlockTakenArr, 0, sizeof(shadowPtr->globalMemoryBlockTakenArr));
    }
    return retVal;
}

/**
* @internal prvCpssDxChLpmRamAddBulkPendingBlockValuesToTotalPendingBlockListSip7 function
* @endinternal
*
* @brief   Update a summary list of all pending block values to be used at
*         the end of bulk operation
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                  Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] totalPendingBlockToUpdateArr[] - array holding all updates done until now
* @param[in] pendingBlockToUpdateArr[] - array of new updates to be kept in totalPendingBlockToUpdateArr
*
* @retval GT_OK                    - on succes
*/
GT_STATUS prvCpssDxChLpmRamAddBulkPendingBlockValuesToTotalPendingBlockListSip7
(
    IN PRV_CPSS_DXCH_LPM_RAM_PENDING_BLOCK_TO_UPDATE_STC totalPendingBlockToUpdateArr[],
    IN PRV_CPSS_DXCH_LPM_RAM_PENDING_BLOCK_TO_UPDATE_STC pendingBlockToUpdateArr[]
)
{
    GT_U32 blockIndex=0;

    for (blockIndex = 0 ; blockIndex < PRV_CPSS_DXCH_LPM_RAM_NUM_OF_MEMORIES_SIP7_CNS; blockIndex++)
    {   /* we only keep the values were update=GT_TRUE, since this is the sum of all updates done */
        if(pendingBlockToUpdateArr[blockIndex].updateInc==GT_TRUE)
        {
            totalPendingBlockToUpdateArr[blockIndex].updateInc=GT_TRUE;
            totalPendingBlockToUpdateArr[blockIndex].numOfIncUpdates +=
                                        pendingBlockToUpdateArr[blockIndex].numOfIncUpdates;
          /* reset pending array for next element */
            pendingBlockToUpdateArr[blockIndex].updateInc=GT_FALSE;
            pendingBlockToUpdateArr[blockIndex].numOfIncUpdates=0;
        }
        if(pendingBlockToUpdateArr[blockIndex].updateDec==GT_TRUE)
        {
            totalPendingBlockToUpdateArr[blockIndex].updateDec=GT_TRUE;
            totalPendingBlockToUpdateArr[blockIndex].numOfDecUpdates +=
                                            pendingBlockToUpdateArr[blockIndex].numOfDecUpdates;
            /* reset pending array for next element */
            pendingBlockToUpdateArr[blockIndex].updateDec=GT_FALSE;
            pendingBlockToUpdateArr[blockIndex].numOfDecUpdates=0;
        }
    }
    return GT_OK;
}

/**
* @internal prvCpssDxChLpmRamUpdateBlockUsageCountersSip7 function
* @endinternal
*
* @brief   Update block usage counters according to pending block values
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                  Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] lpmRamBlocksSizeArrayPtr - used for finding lpm lines per block
* @param[in] totalPendingBlockToUpdateArr[] - array holding all updates done until now
* @param[in] protocol                 - counters should be updated for given protocol
* @param[in] resetPendingBlockToUpdateArr[] - array need to be reset
* @param[in] protocol                 - counters should be updated for given protocol
* @param[in] numOfLpmMemories         -  number of LPM memories
*
* @retval GT_OK                    - on succes
* @retval GT_FAIL                  - on fail
*/
GT_STATUS prvCpssDxChLpmRamUpdateBlockUsageCountersSip7
(
    IN GT_U32                                            *lpmRamBlocksSizeArrayPtr,
    IN PRV_CPSS_DXCH_LPM_RAM_PENDING_BLOCK_TO_UPDATE_STC totalPendingBlockToUpdateArr[],
    IN PRV_CPSS_DXCH_LPM_ADDRESS_COUNTERS_INFO_STC       protocolCountersPerBlockArr[],
    IN PRV_CPSS_DXCH_LPM_RAM_PENDING_BLOCK_TO_UPDATE_STC resetPendingBlockToUpdateArr[],
    IN PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT              protocol,
    IN GT_U32                                            numOfLpmMemories
)
{
    GT_U32 blockIndex=0; /* calculated according to the memory offset devided by block size including gap */
    GT_U32 numOfIncUpdatesToCounter=0;/* counter to be used for incrementing "protocol to block" usage*/
    GT_U32 numOfDecUpdatesToCounter=0;/* counter to be used for decrementing "protocol to block" usage*/
    GT_U32 numOfLinesInBlock;

    /* update the block usage counters */
    for (blockIndex = 0; blockIndex < numOfLpmMemories; blockIndex++)
    {
        if (blockIndex >= PRV_CPSS_DXCH_LPM_RAM_NUM_OF_MEMORIES_SIP7_CNS)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "ERROR: illegal blockIndex - bigger then max number of blocks \n");
        }

        numOfLinesInBlock  = lpmRamBlocksSizeArrayPtr[blockIndex];

        if((totalPendingBlockToUpdateArr[blockIndex].updateInc==GT_TRUE)||
           (totalPendingBlockToUpdateArr[blockIndex].updateDec==GT_TRUE))
        {
            numOfIncUpdatesToCounter = totalPendingBlockToUpdateArr[blockIndex].numOfIncUpdates;
            numOfDecUpdatesToCounter = totalPendingBlockToUpdateArr[blockIndex].numOfDecUpdates;
            switch (protocol)
            {
                case PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E:
                if (((protocolCountersPerBlockArr[blockIndex].sumOfIpv4Counters +
                         numOfIncUpdatesToCounter)<numOfDecUpdatesToCounter) ||
                    (((protocolCountersPerBlockArr[blockIndex].sumOfIpv4Counters +
                         numOfIncUpdatesToCounter)-numOfDecUpdatesToCounter)>numOfLinesInBlock))
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
                }
                protocolCountersPerBlockArr[blockIndex].sumOfIpv4Counters +=
                        numOfIncUpdatesToCounter - numOfDecUpdatesToCounter ;
                break;
            case PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E:
                if (((protocolCountersPerBlockArr[blockIndex].sumOfIpv6Counters +
                         numOfIncUpdatesToCounter)<numOfDecUpdatesToCounter) ||
                    (((protocolCountersPerBlockArr[blockIndex].sumOfIpv6Counters +
                         numOfIncUpdatesToCounter)-numOfDecUpdatesToCounter)>numOfLinesInBlock))
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
                }
                protocolCountersPerBlockArr[blockIndex].sumOfIpv6Counters +=
                        numOfIncUpdatesToCounter - numOfDecUpdatesToCounter ;
                break;
            case PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E:
                if (((protocolCountersPerBlockArr[blockIndex].sumOfFcoeCounters +
                         numOfIncUpdatesToCounter)<numOfDecUpdatesToCounter) ||
                    (((protocolCountersPerBlockArr[blockIndex].sumOfFcoeCounters +
                         numOfIncUpdatesToCounter)-numOfDecUpdatesToCounter)>numOfLinesInBlock))
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
                }
                protocolCountersPerBlockArr[blockIndex].sumOfFcoeCounters +=
                        numOfIncUpdatesToCounter - numOfDecUpdatesToCounter ;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }

           /* reset pending for future use */
           resetPendingBlockToUpdateArr[blockIndex].updateInc=GT_FALSE;
           resetPendingBlockToUpdateArr[blockIndex].numOfIncUpdates=0;
           resetPendingBlockToUpdateArr[blockIndex].updateDec=GT_FALSE;
           resetPendingBlockToUpdateArr[blockIndex].numOfDecUpdates=0;
        }
    }
    return GT_OK;
}

/**
* @internal prvCpssDxChLpmRamIpv4UcPrefixBulkAddForHaSip7 function
* @endinternal
*
* @brief  Function Relevant mode : High Level API modes
*         Creates a new or override an existing bulk of IPv4 prefixes in a Virtual
*         Router for the specified LPM DB in case of HA and complition mode.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                  Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] shadowPtr                - (pointer to) the shadow relevant for the devices asked to act on.
* @param[in] ipv4PrefixArrayLen       - Length of UC prefix array.
* @param[in] ipv4PrefixArrayPtr       - The UC prefix array.
* @param[in] defragmentationEnable    - whether to enable performance costing
*                                      de-fragmentation process in the case that there
*                                      is no place to insert the prefix. To point of the
*                                      process is just to make space for this prefix.
*
* @retval GT_OK                    - if all prefixes were successfully added
* @retval GT_OUT_OF_RANGE          - if prefix length is too big
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_FAIL                  - if adding one or more prefixes failed; the
*                                       array will contain return status for each prefix
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmRamIpv4UcPrefixBulkAddForHaSip7
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC     *shadowPtr,
    IN GT_U32                               ipv4PrefixArrayLen,
    IN CPSS_DXCH_IP_LPM_IPV4_UC_PREFIX_STC  *ipv4PrefixArrayPtr,
    IN GT_BOOL                              defragmentationEnable
)
{
    GT_STATUS                              retVal = GT_OK;
    GT_U32                                 i=0;
    PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_INFO_UNT lpmRouteEntry;

    cpssOsMemSet(&lpmRouteEntry,0,sizeof(lpmRouteEntry));

    /* first go over the prefixes and update the shadow */
    for (i = 0 ; i < ipv4PrefixArrayLen ; i++)
    {
        retVal = prvCpssDxChLpmConvertIpRouteEntryToLpmRouteEntry(shadowPtr->shadowType,
                                                                  &(ipv4PrefixArrayPtr[i].nextHopInfo),
                                                                  &lpmRouteEntry);
        if (retVal != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(retVal, LOG_ERROR_NO_MSG);
        }
        retVal = prvCpssDxChLpmRouteEntryPointerCheckSip7(&(lpmRouteEntry.routeEntry));
        if (retVal != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(retVal, LOG_ERROR_NO_MSG);
        }

        ipv4PrefixArrayPtr[i].returnStatus = prvCpssDxChLpmSip7RamUcEntryAdd(ipv4PrefixArrayPtr[i].vrId,
                                                 ipv4PrefixArrayPtr[i].ipAddr.arIP,
                                                 ipv4PrefixArrayPtr[i].prefixLen,
                                                 &lpmRouteEntry.routeEntry,
                                                 PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_SDW_ONLY_MODE_E,
                                                 ipv4PrefixArrayPtr[i].override,
                                                 PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E,
                                                 shadowPtr,
                                                 defragmentationEnable);
        if (ipv4PrefixArrayPtr[i].returnStatus!=GT_OK)
        {
            return ipv4PrefixArrayPtr[i].returnStatus;
        }
    }

    return retVal;
}

/**
* @internal prvCpssDxChLpmRamIpv4UcPrefixBulkAddSip7 function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         Creates a new or override an existing bulk of IPv4 prefixes in a Virtual
*         Router for the specified LPM DB.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                  Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] lpmDbPtr                 - The LPM DB
* @param[in] ipv4PrefixArrayLen       - Length of UC prefix array.
* @param[in] ipv4PrefixArrayPtr       - The UC prefix array.
* @param[in] defragmentationEnable    - whether to enable performance costing
*                                      de-fragmentation process in the case that there
*                                      is no place to insert the prefix. To point of the
*                                      process is just to make space for this prefix.
*
* @retval GT_OK                    - if all prefixes were successfully added
* @retval GT_OUT_OF_RANGE          - if prefix length is too big
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_FAIL                  - if adding one or more prefixes failed; the
*                                       array will contain return status for each prefix
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmRamIpv4UcPrefixBulkAddSip7
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC *lpmDbPtr,
    IN GT_U32                               ipv4PrefixArrayLen,
    IN CPSS_DXCH_IP_LPM_IPV4_UC_PREFIX_STC  *ipv4PrefixArrayPtr,
    IN GT_BOOL                              defragmentationEnable
)
{
    GT_STATUS           retVal = GT_OK,retVal1=GT_OK,retVal2=GT_OK;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC *shadowPtr;
    GT_U32 shadowIdx;
    GT_U32 i;
    PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_INFO_UNT lpmRouteEntry;
    PRV_CPSS_DXCH_LPM_PROTOCOL_BMP     protocolBitmap=0;
    PRV_CPSS_DXCH_LPM_RAM_PENDING_BLOCK_TO_UPDATE_STC   tempPendingBlockToUpdateArr[PRV_CPSS_DXCH_LPM_RAM_NUM_OF_MEMORIES_SIP7_CNS];
    GT_U32                                              tempGlobalMemoryBlockTakenArr[PRV_CPSS_DXCH_LPM_RAM_NUM_OF_MEMORIES_SIP7_CNS];
    CPSS_SYSTEM_RECOVERY_INFO_STC                       tempSystemRecovery_Info;
    GT_BOOL                                             managerHwWriteBlock;

    cpssOsMemSet(&lpmRouteEntry,0,sizeof(lpmRouteEntry));
    cpssOsMemSet(tempPendingBlockToUpdateArr,0,sizeof(tempPendingBlockToUpdateArr));
    cpssOsMemSet(tempGlobalMemoryBlockTakenArr,0,sizeof(tempGlobalMemoryBlockTakenArr));

    for (i = 0 ; i < ipv4PrefixArrayLen; i++)
    {
        if (ipv4PrefixArrayPtr[i].prefixLen > 32)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }

        if ((ipv4PrefixArrayPtr[i].ipAddr.arIP[0] >= PRV_CPSS_DXCH_LPM_RAM_START_OF_IPV4_MC_ADDRESS_SPACE_CNS) &&
            (ipv4PrefixArrayPtr[i].ipAddr.arIP[0] <= PRV_CPSS_DXCH_LPM_RAM_END_OF_IPV4_MC_ADDRESS_SPACE_CNS))
        {
            /* Multicast range */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }

    /* if initialization has not been done for the requested protocol stack -
    return error */
    if (PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_STATUS_GET_MAC(lpmDbPtr->protocolBitmap) == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    retVal = cpssSystemRecoveryStateGet(&tempSystemRecovery_Info);
    if (retVal != GT_OK)
    {
        return retVal;
    }

    for (shadowIdx = 0 ; shadowIdx < lpmDbPtr->numOfShadowCfg; shadowIdx++)
    {
        shadowPtr = &lpmDbPtr->shadowArray[shadowIdx];

        /* keep value of prending array. update of counters should be done only after HW write is ok */
        cpssOsMemCpy(tempPendingBlockToUpdateArr,shadowPtr->pendingBlockToUpdateArr,sizeof(tempPendingBlockToUpdateArr));
        cpssOsMemCpy(tempGlobalMemoryBlockTakenArr,shadowPtr->globalMemoryBlockTakenArr,sizeof(tempGlobalMemoryBlockTakenArr));

        managerHwWriteBlock = SYSTEM_RECOVERY_CHECK_MANAGER_HW_WRITE_BLOCK_MAC(CPSS_SYSTEM_RECOVERY_LPM_MANAGER_E);
        if (((tempSystemRecovery_Info.systemRecoveryProcess != CPSS_SYSTEM_RECOVERY_PROCESS_HA_E)&&(managerHwWriteBlock == GT_TRUE))||
            ((tempSystemRecovery_Info.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_HA_E) &&
            (tempSystemRecovery_Info.systemRecoveryState == CPSS_SYSTEM_RECOVERY_HW_CATCH_UP_STATE_E)))
        {
            /* in HA process we want to add the prefixes to the shadow one by one and not using bulk */
            return prvCpssDxChLpmRamIpv4UcPrefixBulkAddForHaSip7(shadowPtr,
                                                             ipv4PrefixArrayLen,
                                                             ipv4PrefixArrayPtr,
                                                             defragmentationEnable);
        }

        /* first go over the prefixes and update the shadow */
        for (i = 0 ; i < ipv4PrefixArrayLen ; i++)
        {
            retVal = prvCpssDxChLpmConvertIpRouteEntryToLpmRouteEntry(shadowPtr->shadowType,
                                                                      &(ipv4PrefixArrayPtr[i].nextHopInfo),
                                                                      &lpmRouteEntry);

            if (retVal != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(retVal, LOG_ERROR_NO_MSG);
            }
            retVal = prvCpssDxChLpmRouteEntryPointerCheckSip7(&(lpmRouteEntry.routeEntry));
            if (retVal != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(retVal, LOG_ERROR_NO_MSG);
            }

            ipv4PrefixArrayPtr[i].returnStatus = prvCpssDxChLpmSip7RamUcEntryAdd(ipv4PrefixArrayPtr[i].vrId,
                                                                                 ipv4PrefixArrayPtr[i].ipAddr.arIP,
                                                                                 ipv4PrefixArrayPtr[i].prefixLen,
                                                                                 &lpmRouteEntry.routeEntry,
                                                                                 PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_SDW_MEM_MODE_E,
                                                                                 ipv4PrefixArrayPtr[i].override,
                                                                                 PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E,
                                                                                 shadowPtr,
                                                                                 defragmentationEnable);
            if (ipv4PrefixArrayPtr[i].returnStatus != GT_OK)
            {
                /* reset pending array for next use */
                cpssOsMemSet(shadowPtr->pendingBlockToUpdateArr,0,sizeof(shadowPtr->pendingBlockToUpdateArr));
                /* need to set the last valid state of globalMemoryBlockTakenArr for next add*/
                cpssOsMemCpy(shadowPtr->globalMemoryBlockTakenArr,tempGlobalMemoryBlockTakenArr,sizeof(tempGlobalMemoryBlockTakenArr));
                retVal2 = ipv4PrefixArrayPtr[i].returnStatus;
            }
            else
            {
                retVal1 = prvCpssDxChLpmRamAddBulkPendingBlockValuesToTotalPendingBlockListSip7(tempPendingBlockToUpdateArr,shadowPtr->pendingBlockToUpdateArr);
                if (retVal1!=GT_OK)
                {
                    /* need to reset the array */
                    cpssOsMemSet(shadowPtr->globalMemoryBlockTakenArr, 0, sizeof(shadowPtr->globalMemoryBlockTakenArr));
                    return retVal1;
                }
                else
                {
                    /* keep the last valid state of globalMemoryBlockTakenArr */
                    cpssOsMemCpy(tempGlobalMemoryBlockTakenArr,shadowPtr->globalMemoryBlockTakenArr,sizeof(tempGlobalMemoryBlockTakenArr));
                }
            }
        }

        prvCpssDxChLpmConvertIpProtocolStackToProtocolBitmap(CPSS_IP_PROTOCOL_IPV4_E, &protocolBitmap);
        /* now update the HW */
        retVal1 = updateHwSearchStcSip7(PRV_CPSS_DXCH_LPM_RAM_SEARCH_MEM_UPDATE_E,
                                    protocolBitmap,
                                    shadowPtr);
        if (retVal1 != GT_OK)
        {
            /* reset pending array for next use */
            cpssOsMemSet(shadowPtr->pendingBlockToUpdateArr,0,sizeof(shadowPtr->pendingBlockToUpdateArr));
            /* need to reset the array */
            cpssOsMemSet(shadowPtr->globalMemoryBlockTakenArr, 0, sizeof(shadowPtr->globalMemoryBlockTakenArr));
            return (retVal1);
        }
        else
        {
            retVal1 = prvCpssDxChLpmRamAddBulkPendingBlockValuesToTotalPendingBlockListSip7(tempPendingBlockToUpdateArr,shadowPtr->pendingBlockToUpdateArr);
            if (retVal1!=GT_OK)
            {
                /* need to reset the array */
                cpssOsMemSet(shadowPtr->globalMemoryBlockTakenArr, 0, sizeof(shadowPtr->globalMemoryBlockTakenArr));
                return retVal1;
            }

            retVal1 = prvCpssDxChLpmRamUpdateBlockUsageCountersSip7(shadowPtr->lpmRamBlocksSizeArrayPtr,
                                                                tempPendingBlockToUpdateArr,
                                                                shadowPtr->protocolCountersPerBlockArr,
                                                                shadowPtr->pendingBlockToUpdateArr,
                                                                PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E,
                                                                shadowPtr->numOfLpmMemories);
            if (retVal1!=GT_OK)
            {
                /* reset pending array for future use */
                cpssOsMemSet(shadowPtr->pendingBlockToUpdateArr,0,sizeof(shadowPtr->pendingBlockToUpdateArr));
                /* need to reset the array */
                cpssOsMemSet(shadowPtr->globalMemoryBlockTakenArr, 0, sizeof(shadowPtr->globalMemoryBlockTakenArr));
                return retVal1;
            }
        }
        /* after bulk end we need to reset the array */
        cpssOsMemSet(shadowPtr->globalMemoryBlockTakenArr, 0, sizeof(shadowPtr->globalMemoryBlockTakenArr));
    }
    return (retVal2);
}

/**
* @internal prvCpssDxChLpmRamIpv4UcPrefixDelSip7 function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         Deletes an existing IPv4 prefix in a Virtual Router for the specified
*         LPM DB.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                  Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] lpmDbPtr                 - The LPM DB.
* @param[in] vrId                     - The virtual router id.
* @param[in] ipAddr                   - The destination IP address of the prefix.
* @param[in] prefixLen                - The number of bits that are actual valid in the ipAddr.
*
* @retval GT_OK                    - on success, or
* @retval GT_OUT_OF_RANGE          - If prefix length is too big, or
* @retval GT_ERROR                 - If the vrId was not created yet, or
* @retval GT_NOT_FOUND             - If the given prefix doesn't exitst in the VR, or
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note the default prefix (prefixLen = 0) can't be deleted!
*
*/
GT_STATUS prvCpssDxChLpmRamIpv4UcPrefixDelSip7
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC  *lpmDbPtr,
    IN GT_U32                                vrId,
    IN GT_IPADDR                             ipAddr,
    IN GT_U32                                prefixLen
)
{
    GT_STATUS           retVal = GT_OK;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC *shadowPtr;
    GT_U32 shadowIdx;

    /* if initialization has not been done for the requested protocol stack -
    return error */
    if (PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_STATUS_GET_MAC(lpmDbPtr->protocolBitmap) == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    for (shadowIdx = 0 ; shadowIdx < lpmDbPtr->numOfShadowCfg; shadowIdx++)
    {
        shadowPtr = &lpmDbPtr->shadowArray[shadowIdx];

        if (shadowPtr->vrRootBucketArray[vrId].valid == GT_FALSE)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
        }

        retVal = prvCpssDxChLpmRamSip7UcEntryDel(vrId,
                                                 ipAddr.arIP,
                                                 prefixLen,
                                                 GT_TRUE,
                                                 PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E,
                                                 shadowPtr,
                                                 GT_FALSE);
        if (retVal != GT_OK)
        {
            break;
        }
    }
    return retVal;
}

/**
* @internal prvCpssDxChLpmRamIpv4UcPrefixBulkDelSip7 function
* @endinternal
*
* @brief   Function Relevant mode : High Level mode
*         Deletes an existing bulk of Ipv4 prefixes in a Virtual Router for the
*         specified LPM DB.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                  Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] lpmDbPtr                 - The LPM DB
* @param[in] ipv4PrefixArrayLen       - Length of UC prefix array.
* @param[in] ipv4PrefixArrayPtr       - The UC prefix array.
*
* @retval GT_OK                    - if all prefixes were successfully deleted
* @retval GT_OUT_OF_RANGE          - if prefix length is too big
* @retval GT_BAD_PTR               - if NULL pointer
* @retval GT_FAIL                  - if deleting one or more prefixes failed; the
*                                       array will contain return status for each prefix
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmRamIpv4UcPrefixBulkDelSip7
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC *lpmDbPtr,
    IN GT_U32                               ipv4PrefixArrayLen,
    IN CPSS_DXCH_IP_LPM_IPV4_UC_PREFIX_STC  *ipv4PrefixArrayPtr
)
{
    GT_STATUS           retVal = GT_OK;
    GT_U32      i;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC *shadowPtr;
    GT_U32 shadowIdx;
    PRV_CPSS_DXCH_LPM_RAM_PENDING_BLOCK_TO_UPDATE_STC   tempPendingBlockToUpdateArr[PRV_CPSS_DXCH_LPM_RAM_NUM_OF_MEMORIES_SIP7_CNS];

    cpssOsMemSet(tempPendingBlockToUpdateArr,0,sizeof(tempPendingBlockToUpdateArr));

    /* if initialization has not been done for the requested protocol stack -
    return error */
    if (PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_STATUS_GET_MAC(lpmDbPtr->protocolBitmap) == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    for (shadowIdx = 0 ; shadowIdx < lpmDbPtr->numOfShadowCfg; shadowIdx++)
    {
        shadowPtr = &lpmDbPtr->shadowArray[shadowIdx];

        /* keep value of prending array. update of counters should be done only after HW write is ok */
        cpssOsMemCpy(tempPendingBlockToUpdateArr,shadowPtr->pendingBlockToUpdateArr,
                     sizeof(tempPendingBlockToUpdateArr));

        for (i = 0 ; i < ipv4PrefixArrayLen ; i++)
        {
            if (ipv4PrefixArrayPtr[i].prefixLen > 32)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
            }

            if ((ipv4PrefixArrayPtr[i].ipAddr.arIP[0] >= PRV_CPSS_DXCH_LPM_RAM_START_OF_IPV4_MC_ADDRESS_SPACE_CNS) &&
                (ipv4PrefixArrayPtr[i].ipAddr.arIP[0] <= PRV_CPSS_DXCH_LPM_RAM_END_OF_IPV4_MC_ADDRESS_SPACE_CNS))
            {
                /* Multicast range */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }

            ipv4PrefixArrayPtr[i].returnStatus =
                 prvCpssDxChLpmRamSip7UcEntryDel(ipv4PrefixArrayPtr[i].vrId,
                                                 ipv4PrefixArrayPtr[i].ipAddr.arIP,
                                                 ipv4PrefixArrayPtr[i].prefixLen,
                                                 GT_TRUE,
                                                 PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E,
                                                 shadowPtr,
                                                 GT_FALSE);

            if (ipv4PrefixArrayPtr[i].returnStatus != GT_OK)
            {
               /* reset pending array for next use */
                cpssOsMemSet(shadowPtr->pendingBlockToUpdateArr,0,sizeof(shadowPtr->pendingBlockToUpdateArr));
                retVal = ipv4PrefixArrayPtr[i].returnStatus;
            }
        }
    }
    return (retVal);
}

/**
* @internal prvCpssDxChLpmRamIpv4UcPrefixesFlushSip7 function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         Flushes the unicast IPv4 Routing table and stays with the default prefix
*         only for a specific LPM DB.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                  Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] lpmDbPtr                 - The LPM DB
* @param[in] vrId                     - The virtual router identifier.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmRamIpv4UcPrefixesFlushSip7
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC  *lpmDbPtr,
    IN GT_U32                                vrId
)
{
    GT_STATUS           retVal = GT_OK;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC *shadowPtr;
    GT_U32 shadowIdx;

    /* if initialization has not been done for the requested protocol stack -
    return error */
    if (PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_STATUS_GET_MAC(lpmDbPtr->protocolBitmap) == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    for (shadowIdx = 0 ; shadowIdx < lpmDbPtr->numOfShadowCfg; shadowIdx++)
    {
        shadowPtr = &lpmDbPtr->shadowArray[shadowIdx];
        if (shadowPtr->vrRootBucketArray[vrId].valid == GT_FALSE)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
        }
        retVal = prvCpssDxChLpmRamUcEntriesFlushSip7(vrId,
                                                 PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E,
                                                 shadowPtr);
        if (retVal != GT_OK)
        {
            break;
        }
    }
    return (retVal);
}

/**
* @internal prvCpssDxChLpmRamIpv4UcPrefixSearchSip7 function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         This function searches for a given ip-uc address, and returns the next
*         hop pointer information bound to it if found.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                  Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] lpmDbPtr                 - The LPM DB
* @param[in] vrId                     - The virtual router id.
* @param[in] ipAddr                   - The destination IP address to look for.
* @param[in] prefixLen                - The number of bits that are actual valid in the
* @param[in] ipAddr
*
* @param[out] nextHopInfoPtr           - If found this is The next hop pointer to for this
*                                      prefix.
*
* @retval GT_OK                    - if the required entry was found, or
* @retval GT_OUT_OF_RANGE          - if prefix length is too big, or
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_FOUND             - if the given ip prefix was not found.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmRamIpv4UcPrefixSearchSip7
(
    IN  PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC        *lpmDbPtr,
    IN  GT_U32                                      vrId,
    IN  GT_IPADDR                                   ipAddr,
    IN  GT_U32                                      prefixLen,
    OUT PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC   *nextHopInfoPtr
)
{
    GT_STATUS           retVal = GT_OK;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC *shadowPtr;

    /* if initialization has not been done for the requested protocol stack -
    return error */
    if (PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_STATUS_GET_MAC(lpmDbPtr->protocolBitmap) == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    /* the search can be done on one shadow */
    shadowPtr = &lpmDbPtr->shadowArray[0];
    if (shadowPtr->vrRootBucketArray[vrId].valid == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }
    retVal = prvCpssDxChLpmRamUcEntrySearchSip7(vrId,ipAddr.arIP,prefixLen,
                                            nextHopInfoPtr,
                                            PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E,
                                            shadowPtr);

    return (retVal);
}

/**
* @internal prvCpssDxChLpmRamIpv4UcPrefixGetNextSip7 function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         This function returns an IP-Unicast prefix with larger (ipAddrPtr,prefixLenPtr)
*         than the given one, it used for iterating over the existing prefixes.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                  Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] lpmDbPtr                 - The LPM DB.
* @param[in] vrId                     - The virtual router Id to get the entry from.
* @param[in,out] ipAddrPtr                - The ip address to start the search from.
* @param[in,out] prefixLenPtr             - Prefix length of ipAddr.
* @param[in,out] ipAddrPtr                - The ip address of the found entry.
* @param[in,out] prefixLenPtr             - The prefix length of the found entry.
*
* @param[out] nextHopInfoPtr           - the next hop pointer associated with the found
*                                      ipAddr.
*
* @retval GT_OK                    - if the required entry was found, or
* @retval GT_OUT_OF_RANGE          - If prefix length is too big, or
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_FOUND             - if no more entries are left in the IP table.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. The values of (ipAddrPtr,prefixLenPtr) must be a valid values, it
*       means that they exist in the IP-UC Table, unless this is the first
*       call to this function, then the value of (ipAddrPtr,prefixLenPtr) is
*       (0,0).
*       2. In order to get route pointer information for (0,0) use the Ipv4
*       UC prefix get function.
*
*/
GT_STATUS prvCpssDxChLpmRamIpv4UcPrefixGetNextSip7
(
    IN    PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC          *lpmDbPtr,
    IN    GT_U32                                        vrId,
    INOUT GT_IPADDR                                     *ipAddrPtr,
    INOUT GT_U32                                        *prefixLenPtr,
    OUT   PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC     *nextHopInfoPtr
)
{
    GT_STATUS                                   retVal = GT_OK;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC            *shadowPtr;
    PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC   *tmpNextHopInfoPtr;

    /* if initialization has not been done for the requested protocol stack -
    return error */
    if (PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_STATUS_GET_MAC(lpmDbPtr->protocolBitmap) == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    /* the search can be done on one shadow */
    shadowPtr = &lpmDbPtr->shadowArray[0];
    if (shadowPtr->vrRootBucketArray[vrId].valid == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }
    retVal = prvCpssDxChLpmRamUcEntryGetSip7(vrId,ipAddrPtr->arIP,prefixLenPtr,
                                         &tmpNextHopInfoPtr,
                                         PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E,
                                         shadowPtr);
    if (retVal == GT_OK)
    {
        cpssOsMemCpy(nextHopInfoPtr,tmpNextHopInfoPtr,
                     sizeof(PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC));
    }

    return retVal;
}

/**
* @internal prvCpssDxChLpmRamIpv4UcPrefixGetSip7 function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         This function gets a given ip address, and returns the next
*         hop pointer information bounded to the longest prefix match.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                  Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] lpmDbPtr                 - The LPM DB
* @param[in] vrId                     - The virtual router id.
* @param[in] ipAddr                   - The destination IP address to look for.
*
* @param[out] prefixLenPtr             - Points to the number of bits that are actual valid
*                                      in the longest match
* @param[out] nextHopPointerPtr        - The next hop pointer bounded to the longest match
*
* @retval GT_OK                    - if the required entry was found, or
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_FOUND             - if the given ip prefix was not found.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmRamIpv4UcPrefixGetSip7
(
    IN  PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC        *lpmDbPtr,
    IN  GT_U32                                      vrId,
    IN  GT_IPADDR                                   ipAddr,
    OUT GT_U32                                      *prefixLenPtr,
    OUT PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC   *nextHopPointerPtr
)
{
    GT_STATUS                           retVal = GT_OK;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC    *shadowPtr;

    /* if initialization has not been done for the requested protocol stack -
       return error */
    if (PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_STATUS_GET_MAC(lpmDbPtr->protocolBitmap) == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    /* the search can be done on one shadow */
    shadowPtr = &lpmDbPtr->shadowArray[0];
    if (shadowPtr->vrRootBucketArray[vrId].valid == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }
    retVal = prvCpssDxChLpmRamUcEntryLpmSearchSip7(vrId,ipAddr.arIP,prefixLenPtr,
                                               nextHopPointerPtr,
                                               PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E,
                                               shadowPtr);
    return (retVal);
}

/**
* @internal prvCpssDxChLpmRamIpv4McEntryAddSip7 function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         To add the multicast routing information for IP datagrams from
*         a particular source and addressed to a particular IP multicast
*         group address for a specific LPM DB.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                  Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] lpmDbPtr                 - The LPM DB
* @param[in] vrId                     - The virtual private network identifier.
* @param[in] ipGroup                  - The IP multicast group address.
* @param[in] ipGroupPrefixLen         - The number of bits that are actual valid in,
*                                      the ipGroup.
* @param[in] ipSrc                    - the root address for source base multi tree protocol.
* @param[in] ipSrcPrefixLen           - The number of bits that are actual valid in,
*                                      the ipSrc.
* @param[in] mcRouteEntryPtr          - the mc Route pointer to set for the mc entry.
* @param[in] override                 -  the existing entry if it already exists
* @param[in] defragmentationEnable    - whether to enable performance costing
*                                      de-fragmentation process in the case that there
*                                      is no place to insert the prefix. To point of the
*                                      process is just to make space for this prefix.
*
* @retval GT_OK                    - on success, or
* @retval GT_OUT_OF_RANGE          - if prefix length is too big
* @retval GT_ERROR                 - if the virtual router does not exist.
* @retval GT_NOT_FOUND             - prefix was not found when override is GT_TRUE
* @retval GT_ALREADY_EXIST         - prefix already exist when override is GT_FALSE
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_OUT_OF_CPU_MEM        - if failed to allocate CPU memory, or
* @retval GT_OUT_OF_PP_MEM         - if failed to allocate PP memory, or
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. To override the default mc route use ipGroup = 0.
*       2. If (S,G) MC group is added when (,G) doesn't exists then implicit
*       (,G) is added pointing to (,) default. Application added (,G)
*       will override the implicit (,G).
*
*/
GT_STATUS prvCpssDxChLpmRamIpv4McEntryAddSip7
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC         *lpmDbPtr,
    IN GT_U32                                       vrId,
    IN GT_IPADDR                                    ipGroup,
    IN GT_U32                                       ipGroupPrefixLen,
    IN GT_IPADDR                                    ipSrc,
    IN GT_U32                                       ipSrcPrefixLen,
    IN PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC    *mcRouteEntryPtr,
    IN GT_BOOL                                      override,
    IN GT_BOOL                                      defragmentationEnable
)
{
    GT_STATUS           retVal = GT_OK;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC *shadowPtr;
    GT_U32 shadowIdx;
    PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_MODE_ENT   insertMode;
    CPSS_SYSTEM_RECOVERY_INFO_STC   tempSystemRecovery_Info;
    GT_BOOL                         managerHwWriteBlock;
    /* do special check for link local*/


    if ((ipGroupPrefixLen > 4) && (ipGroupPrefixLen < 32))
    {
        /* in this case all except link local must be rejected */
        PRV_CPSS_DXCH_LPM_IPV4_MC_LINK_LOCAL_CHECK_MAC(ipGroup, ipGroupPrefixLen, ipSrcPrefixLen);
    }

    if ((ipGroupPrefixLen <= 4) && (ipSrcPrefixLen > 0))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
    /* link local exact mach must be also rejected */
    PRV_CPSS_DXCH_LPM_IPV4_MC_LINK_LOCAL_CHECK_EXACT_MATCH_MAC(ipGroup, ipGroupPrefixLen);

    /* if initialization has not been done for the requested protocol stack - return error */
    if (PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_STATUS_GET_MAC(lpmDbPtr->protocolBitmap) == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    for (shadowIdx = 0 ; shadowIdx < lpmDbPtr->numOfShadowCfg; shadowIdx++)
    {
        shadowPtr = &lpmDbPtr->shadowArray[shadowIdx];
        retVal = prvCpssDxChLpmRouteEntryPointerCheckSip7(mcRouteEntryPtr);
        if (retVal != GT_OK)
            return retVal;

        if (shadowPtr->vrRootBucketArray[vrId].valid == GT_FALSE)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
        }

        retVal = cpssSystemRecoveryStateGet(&tempSystemRecovery_Info);
        if (retVal != GT_OK)
        {
            return retVal;
        }
        managerHwWriteBlock = SYSTEM_RECOVERY_CHECK_MANAGER_HW_WRITE_BLOCK_MAC(CPSS_SYSTEM_RECOVERY_LPM_MANAGER_E);
        if((tempSystemRecovery_Info.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_HA_E) ||
           (managerHwWriteBlock == GT_TRUE))
        {
            insertMode=PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_SDW_ONLY_MODE_E;
        }
        else
        {
            insertMode=PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_SDW_MEM_HW_MODE_E;
        }

        retVal = prvCpssDxChLpmSip7RamMcEntryAdd(vrId,ipGroup.arIP,ipGroupPrefixLen,
                                                 ipSrc.arIP,ipSrcPrefixLen,
                                                 mcRouteEntryPtr,
                                                 override,
                                                 insertMode,
                                                 PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E,
                                                 shadowPtr,
                                                 defragmentationEnable);
        if (retVal != GT_OK)
        {
            /* need to reset the array */
            cpssOsMemSet(shadowPtr->globalMemoryBlockTakenArr, 0, sizeof(shadowPtr->globalMemoryBlockTakenArr));
            break;
        }
        /* reset the array for next use */
        cpssOsMemSet(shadowPtr->globalMemoryBlockTakenArr, 0, sizeof(shadowPtr->globalMemoryBlockTakenArr));
    }
    return (retVal);
}

/**
* @internal prvCpssDxChLpmRamIpv4McEntryDelSip7 function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         To delete a particular mc route entry for a specific LPM DB.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                  Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] lpmDbPtr                 - The LPM DB
* @param[in] vrId                     - The virtual router identifier.
* @param[in] ipGroup                  - The IP multicast group address.
* @param[in] ipGroupPrefixLen         - The number of bits that are actual valid in,
*                                      the ipGroup.
* @param[in] ipSrc                    - the root address for source base multi tree protocol.
* @param[in] ipSrcPrefixLen           - The number of bits that are actual valid in,
*                                      the ipSrc.
*
* @retval GT_OK                    - on success, or
* @retval GT_OUT_OF_RANGE          - if prefix length is too big, or
* @retval GT_ERROR                 - if the virtual router does not exist, or
* @retval GT_NOT_FOUND             - if the (ipGroup,prefix) does not exist, or
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. In order to delete the multicast entry and all the src ip addresses
*       associated with it, call this function with ipSrc = ipSrcPrefixLen = 0.
*       2. If no (,G) was added but (S,G) were added, then implicit (,G)
*       that points to (,) is added. If (,G) is added later, it will
*       replace the implicit (,G).
*       When deleting (,G), then if there are still (S,G), an implicit (,G)
*       pointing to (,) will be added.
*       When deleting last (S,G) and the (,G) was implicitly added, then
*       the (,G) will be deleted as well.
*
*/
GT_STATUS prvCpssDxChLpmRamIpv4McEntryDelSip7
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC *lpmDbPtr,
    IN GT_U32                               vrId,
    IN GT_IPADDR                            ipGroup,
    IN GT_U32                               ipGroupPrefixLen,
    IN GT_IPADDR                            ipSrc,
    IN GT_U32                               ipSrcPrefixLen
)
{
    GT_STATUS           retVal = GT_OK;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC *shadowPtr;
    GT_U32 shadowIdx;

    /* if initialization has not been done for the requested protocol stack -
    return error */
    if (PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_STATUS_GET_MAC(lpmDbPtr->protocolBitmap) == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    for (shadowIdx = 0 ; shadowIdx < lpmDbPtr->numOfShadowCfg; shadowIdx++)
    {
        shadowPtr = &lpmDbPtr->shadowArray[shadowIdx];
        if (shadowPtr->vrRootBucketArray[vrId].valid == GT_FALSE)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
        }

        retVal = prvCpssDxChLpmSip7RamMcEntryDelete(vrId, ipGroup.arIP,ipGroupPrefixLen,
                                                    ipSrc.arIP,ipSrcPrefixLen,
                                                    PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E,
                                                    shadowPtr,GT_FALSE,NULL);
        if (retVal != GT_OK)
        {
            break;
        }

    }
    return (retVal);
}

/**
* @internal prvCpssDxChLpmRamIpv4McEntriesFlushSip7 function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         flushes the multicast IP Routing table and stays with the default entry
*         only for a specific LPM DB.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                  Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] lpmDbPtr                 - The LPM DB
* @param[in] vrId                     - The virtual router identifier.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmRamIpv4McEntriesFlushSip7
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC *lpmDbPtr,
    IN GT_U32                               vrId
)
{
    GT_STATUS           retVal = GT_OK;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC *shadowPtr;
    GT_U32 shadowIdx;

    /* if initialization has not been done for the requested protocol stack -
    return error */
    if (PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_STATUS_GET_MAC(lpmDbPtr->protocolBitmap) == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    for (shadowIdx = 0 ; shadowIdx < lpmDbPtr->numOfShadowCfg; shadowIdx++)
    {
        shadowPtr = &lpmDbPtr->shadowArray[shadowIdx];
        if (shadowPtr->vrRootBucketArray[vrId].valid == GT_FALSE)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
        }
        retVal = prvCpssDxChLpmRamMcEntriesFlushSip7(vrId,
                                                 PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E,
                                                 shadowPtr);
        if (retVal != GT_OK)
        {
            break;
        }
    }
    return (retVal);
}

/**
* @internal prvCpssDxChLpmRamIpv4McEntryGetSip7 function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         This function returns the muticast (ipSrc,ipGroup) entry, used
*         to find specific multicast adrress entry
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                  Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] lpmDbPtr                 - The LPM DB
* @param[in] vrId                     - The virtual router Id.
* @param[in] ipGroup                  - The ip Group address to get the next entry for.
* @param[in] ipGroupPrefixLen         - ipGroup prefix length.
* @param[in] ipSrc                    - The ip Source address to get the next entry for.
* @param[in] ipSrcPrefixLen           - ipSrc prefix length.
*
* @param[out] mcRouteEntryPtr          - the mc route entry ptr of the found mc mc route
*
* @retval GT_OK                    - if found, or
* @retval GT_OUT_OF_RANGE          - if prefix length is too big, or
* @retval GT_BAD_PTR               - if NULL pointer, or
* @retval GT_NOT_FOUND             - if the given address is the last one on the IP-Mc table, or
* @retval GT_NOT_APPLICABLE_DEVICE - if not applicable device.
*/
GT_STATUS prvCpssDxChLpmRamIpv4McEntryGetSip7
(
    IN  PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC        *lpmDbPtr,
    IN  GT_U32                                      vrId,
    IN  GT_IPADDR                                   ipGroup,
    IN  GT_U32                                      ipGroupPrefixLen,
    IN  GT_IPADDR                                   ipSrc,
    IN  GT_U32                                      ipSrcPrefixLen,
    OUT PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC   *mcRouteEntryPtr
)
{
    GT_STATUS           retVal = GT_OK;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC *shadowPtr;

    /* if initialization has not been done for the requested protocol stack -
    return error */
    if (PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_STATUS_GET_MAC(lpmDbPtr->protocolBitmap) == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    if (lpmDbPtr->shadowArray->vrRootBucketArray[vrId].valid == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }

    /* it's enough to look at the first shadow */
    shadowPtr = &lpmDbPtr->shadowArray[0];
    retVal = prvCpssDxChLpmRamMcEntrySearchSip7(vrId,ipGroup.arIP,ipGroupPrefixLen,
                                            ipSrc.arIP,ipSrcPrefixLen,
                                            mcRouteEntryPtr,
                                            PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E,
                                            shadowPtr);
    return (retVal);
}

/**
* @internal prvCpssDxChLpmRamIpv4McEntryGetNextSip7 function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         This function returns the next muticast (ipSrcPtr,ipGroupPtr) entry, used
*         to iterate over the existing multicast addresses for a specific LPM DB.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                  Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] lpmDbPtr                 - The LPM DB
* @param[in] vrId                     - The virtual router Id.
* @param[in,out] ipGroupPtr               - The ip Group address to get the next entry for.
* @param[in,out] ipGroupPrefixLenPtr      - ipGroupPtr prefix length.
* @param[in,out] ipSrcPtr                 - The ip Source address to get the next entry for.
* @param[in,out] ipSrcPrefixLenPtr        - ipSrcPtr prefix length.
* @param[in,out] ipGroupPtr               - The next ip Group address.
* @param[in,out] ipGroupPrefixLenPtr      - ipGroupPtr prefix length.
* @param[in,out] ipSrcPtr                 - The next ip Source address.
* @param[in,out] ipSrcPrefixLenPtr        - ipSrc prefix length.
*
* @param[out] mcRouteEntryPtr          - the mc route entry ptr of the found mc mc route
*
* @retval GT_OK                    - if found, or
* @retval GT_OUT_OF_RANGE          - if prefix length is too big, or
* @retval GT_BAD_PTR               - if NULL pointer, or
* @retval GT_NOT_FOUND             - if the given address is the last one on the IP-Mc table, or
* @retval GT_NOT_APPLICABLE_DEVICE - if not applicable device.
*
* @note 1. The values of (ipSrcPtr,ipGroupPtr) must be a valid values, it
*       means that they exist in the IP-Mc Table, unless this is the first
*       call to this function, then the value of (ipSrcPtr,ipGroupPtr) is
*       (0,0).
*       2. In order to get route pointer information for (0,0) use the Ipv4
*       MC get function.
*
*/
GT_STATUS prvCpssDxChLpmRamIpv4McEntryGetNextSip7
(
    IN    PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC      *lpmDbPtr,
    IN    GT_U32                                    vrId,
    INOUT GT_IPADDR                                 *ipGroupPtr,
    INOUT GT_U32                                    *ipGroupPrefixLenPtr,
    INOUT GT_IPADDR                                 *ipSrcPtr,
    INOUT GT_U32                                    *ipSrcPrefixLenPtr,
    OUT   PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC *mcRouteEntryPtr
)
{
    GT_STATUS           retVal = GT_OK;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC *shadowPtr;

    /* if initialization has not been done for the requested protocol stack -
    return error */
    if (PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_STATUS_GET_MAC(lpmDbPtr->protocolBitmap) == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    if (lpmDbPtr->shadowArray->vrRootBucketArray[vrId].valid == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }

    /* it's enough to look at the first shadow */
    shadowPtr = &lpmDbPtr->shadowArray[0];
    retVal = prvCpssDxChLpmRamMcEntryGetNextSip7(vrId,ipGroupPtr->arIP,ipGroupPrefixLenPtr,
                                             ipSrcPtr->arIP,
                                             ipSrcPrefixLenPtr,
                                             mcRouteEntryPtr,
                                             PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E,
                                             shadowPtr);
    /* group prefix 0 means this is the default entry (*,*) */
    if (*ipGroupPrefixLenPtr == 0)
    {
        ipGroupPtr->u32Ip = 0;
    }
    return (retVal);
}

/**
* @internal prvCpssDxChLpmRamIpv6UcPrefixAddSip7 function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         creates a new or override an existing Ipv6 prefix in a Virtual Router
*         for the specified LPM DB.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                  Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] lpmDbPtr                 - The LPM DB
* @param[in] vrId                     - The virtual router id.
* @param[in] ipAddr                   - Points to the destination IP address of this prefix.
* @param[in] prefixLen                - The number of bits that are actual valid in the ipAddr.
* @param[in] nextHopInfoPtr           - Points to the next hop pointer to set for this prefix.
* @param[in] override                 -  the existing entry if it already exists
* @param[in] defragmentationEnable    - whether to enable performance costing
*                                      de-fragmentation process in the case that there
*                                      is no place to insert the prefix. To point of the
*                                      process is just to make space for this prefix.
*
* @retval GT_OK                    - if success, or
* @retval GT_OUT_OF_RANGE          - if prefix length is too big, or
* @retval GT_ERROR                 - if the vrId was not created yet, or
* @retval GT_ALREADY_EXIST         - if prefix already exist when override is GT_FALSE, or
* @retval GT_BAD_PTR               - if NULL pointer, or
* @retval GT_OUT_OF_CPU_MEM        - if failed to allocate CPU memory, or
* @retval GT_OUT_OF_PP_MEM         - if failed to allocate PP memory, or
* @retval GT_NOT_APPLICABLE_DEVICE - if not applicable device, or
* @retval GT_FAIL                  - otherwise.
*
* @note To change the default prefix for the VR use prefixLen = 0.
*
*/
GT_STATUS prvCpssDxChLpmRamIpv6UcPrefixAddSip7
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC         *lpmDbPtr,
    IN GT_U32                                       vrId,
    IN GT_IPV6ADDR                                  ipAddr,
    IN GT_U32                                       prefixLen,
    IN PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC    *nextHopInfoPtr,
    IN GT_BOOL                                      override,
    IN GT_BOOL                                      defragmentationEnable
)
{
    GT_STATUS           retVal = GT_OK;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC *shadowPtr;
    GT_U32 shadowIdx;
    PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_MODE_ENT   insertMode;
    CPSS_SYSTEM_RECOVERY_INFO_STC   tempSystemRecovery_Info;
    GT_BOOL                         managerHwWriteBlock;

    /* if initialization has not been done for the requested protocol stack -
    return error */
    if (PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_STATUS_GET_MAC(lpmDbPtr->protocolBitmap) == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    for (shadowIdx = 0 ; shadowIdx < lpmDbPtr->numOfShadowCfg; shadowIdx++)
    {
        shadowPtr = &lpmDbPtr->shadowArray[shadowIdx];

        retVal = prvCpssDxChLpmRouteEntryPointerCheckSip7(nextHopInfoPtr);
        if (retVal != GT_OK)
            return retVal;

       retVal = cpssSystemRecoveryStateGet(&tempSystemRecovery_Info);
        if (retVal != GT_OK)
        {
            return retVal;
        }
        managerHwWriteBlock = SYSTEM_RECOVERY_CHECK_MANAGER_HW_WRITE_BLOCK_MAC(CPSS_SYSTEM_RECOVERY_LPM_MANAGER_E);
        if((tempSystemRecovery_Info.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_HA_E) ||
           (managerHwWriteBlock == GT_TRUE))
        {
            insertMode=PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_SDW_ONLY_MODE_E;
        }
        else
        {
            insertMode=PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_SDW_MEM_HW_MODE_E;
        }

        retVal = prvCpssDxChLpmSip7RamUcEntryAdd(vrId,
                                                 ipAddr.arIP,
                                                 prefixLen,
                                                 nextHopInfoPtr,
                                                 insertMode,
                                                 override,
                                                 PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E,
                                                 shadowPtr,
                                                 defragmentationEnable);
        if (retVal != GT_OK)
        {
            /* reset the array for next use */
            cpssOsMemSet(shadowPtr->globalMemoryBlockTakenArr, 0, sizeof(shadowPtr->globalMemoryBlockTakenArr));
            break;
        }
        /* reset the array for next use */
        cpssOsMemSet(shadowPtr->globalMemoryBlockTakenArr, 0, sizeof(shadowPtr->globalMemoryBlockTakenArr));
    }
    return retVal;
}

/**
* @internal prvCpssDxChLpmRamIpv6UcPrefixBulkAddForHaSip7 function
* @endinternal
*
* @brief  Function Relevant mode : High Level API modes
*         Creates a new or override an existing bulk of IPv6 prefixes in a Virtual
*         Router for the specified LPM DB in case of HA and complition mode.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                  Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] shadowPtr                - (pointer to) the shadow relevant for the devices asked to act on.
* @param[in] ipv6PrefixArrayLen       - Length of UC prefix array.
* @param[in] ipv6PrefixArrayPtr       - The UC prefix array.
* @param[in] defragmentationEnable    - whether to enable performance costing
*                                      de-fragmentation process in the case that there
*                                      is no place to insert the prefix. To point of the
*                                      process is just to make space for this prefix.
*
* @retval GT_OK                    - if all prefixes were successfully added
* @retval GT_OUT_OF_RANGE          - if prefix length is too big
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_FAIL                  - if adding one or more prefixes failed; the
*                                       array will contain return status for each prefix
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmRamIpv6UcPrefixBulkAddForHaSip7
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC     *shadowPtr,
    IN GT_U32                               ipv6PrefixArrayLen,
    IN CPSS_DXCH_IP_LPM_IPV6_UC_PREFIX_STC  *ipv6PrefixArrayPtr,
    IN GT_BOOL                              defragmentationEnable
)
{
    GT_STATUS                              retVal = GT_OK;
    GT_U32                                 i=0;
    PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_INFO_UNT lpmRouteEntry;

    cpssOsMemSet(&lpmRouteEntry,0,sizeof(lpmRouteEntry));

    /* first go over the prefixes and update the shadow */
    for (i = 0 ; i < ipv6PrefixArrayLen ; i++)
    {
        retVal = prvCpssDxChLpmConvertIpRouteEntryToLpmRouteEntry(shadowPtr->shadowType,
                                                                  &(ipv6PrefixArrayPtr[i].nextHopInfo),
                                                                  &lpmRouteEntry);
        if (retVal != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(retVal, LOG_ERROR_NO_MSG);
        }
        retVal = prvCpssDxChLpmRouteEntryPointerCheckSip7(&(lpmRouteEntry.routeEntry));
        if (retVal != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(retVal, LOG_ERROR_NO_MSG);
        }

        ipv6PrefixArrayPtr[i].returnStatus = prvCpssDxChLpmSip7RamUcEntryAdd(ipv6PrefixArrayPtr[i].vrId,
                                                 ipv6PrefixArrayPtr[i].ipAddr.arIP,
                                                 ipv6PrefixArrayPtr[i].prefixLen,
                                                 &lpmRouteEntry.routeEntry,
                                                 PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_SDW_ONLY_MODE_E,
                                                 ipv6PrefixArrayPtr[i].override,
                                                 PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E,
                                                 shadowPtr,
                                                 defragmentationEnable);

        if (ipv6PrefixArrayPtr[i].returnStatus!=GT_OK)
        {
            return ipv6PrefixArrayPtr[i].returnStatus;
        }
    }

    return retVal;
}
/**
* @internal prvCpssDxChLpmRamIpv6UcPrefixBulkAddSip7 function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         creates a new or override an existing bulk of Ipv6 prefixes in a Virtual
*         Router for the specified LPM DB.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                  Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] lpmDbPtr                 - The LPM DB
* @param[in] ipv6PrefixArrayLen       - Length of UC prefix array.
* @param[in] ipv6PrefixArrayPtr       - The UC prefix array.
* @param[in] defragmentationEnable    - whether to enable performance costing
*                                      de-fragmentation process in the case that there
*                                      is no place to insert the prefix. To point of the
*                                      process is just to make space for this prefix.
*
* @retval GT_OK                    - if all prefixes were successfully added
* @retval GT_OUT_OF_RANGE          - if prefix length is too big
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_FAIL                  - if adding one or more prefixes failed; the
*                                       array will contain return status for each prefix
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note none.
*
*/
GT_STATUS prvCpssDxChLpmRamIpv6UcPrefixBulkAddSip7
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC     *lpmDbPtr,
    IN GT_U32                                   ipv6PrefixArrayLen,
    IN CPSS_DXCH_IP_LPM_IPV6_UC_PREFIX_STC      *ipv6PrefixArrayPtr,
    IN GT_BOOL                                  defragmentationEnable
)
{
    GT_STATUS           retVal = GT_OK,retVal1=GT_OK,retVal2=GT_OK;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC *shadowPtr;
    GT_U32 shadowIdx;
    GT_U32 i;
    PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_INFO_UNT lpmRouteEntry;
    PRV_CPSS_DXCH_LPM_PROTOCOL_BMP     protocolBitmap=0;
    PRV_CPSS_DXCH_LPM_RAM_PENDING_BLOCK_TO_UPDATE_STC   tempPendingBlockToUpdateArr[PRV_CPSS_DXCH_LPM_RAM_NUM_OF_MEMORIES_SIP7_CNS];
    GT_U32                                              tempGlobalMemoryBlockTakenArr[PRV_CPSS_DXCH_LPM_RAM_NUM_OF_MEMORIES_SIP7_CNS];
    CPSS_SYSTEM_RECOVERY_INFO_STC                       tempSystemRecovery_Info;
    GT_BOOL                                             managerHwWriteBlock;


    cpssOsMemSet(tempPendingBlockToUpdateArr,0,sizeof(tempPendingBlockToUpdateArr));
    cpssOsMemSet(tempGlobalMemoryBlockTakenArr,0,sizeof(tempGlobalMemoryBlockTakenArr));

    cpssOsMemSet(&lpmRouteEntry,0,sizeof(lpmRouteEntry));

    for (i = 0 ; i < ipv6PrefixArrayLen ; i++)
    {
        if (ipv6PrefixArrayPtr[i].prefixLen > 128)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }

        if (ipv6PrefixArrayPtr[i].ipAddr.arIP[0] >= PRV_CPSS_DXCH_LPM_RAM_START_OF_IPV6_MC_ADDRESS_SPACE_CNS)
        {
            /* Multicast range */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }
    }

    /* if initialization has not been done for the requested protocol stack -
    return error */
    if (PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_STATUS_GET_MAC(lpmDbPtr->protocolBitmap) == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    retVal = cpssSystemRecoveryStateGet(&tempSystemRecovery_Info);
    if (retVal != GT_OK)
    {
        return retVal;
    }

    for (shadowIdx = 0 ; shadowIdx < lpmDbPtr->numOfShadowCfg; shadowIdx++)
    {
        shadowPtr = &lpmDbPtr->shadowArray[shadowIdx];

        /* keep value of prending array. update of counters should be done only after HW write is ok */
        cpssOsMemCpy(tempPendingBlockToUpdateArr,shadowPtr->pendingBlockToUpdateArr,sizeof(tempPendingBlockToUpdateArr));
        cpssOsMemCpy(tempGlobalMemoryBlockTakenArr,shadowPtr->globalMemoryBlockTakenArr,sizeof(tempGlobalMemoryBlockTakenArr));

        managerHwWriteBlock = SYSTEM_RECOVERY_CHECK_MANAGER_HW_WRITE_BLOCK_MAC(CPSS_SYSTEM_RECOVERY_LPM_MANAGER_E);
        if (((tempSystemRecovery_Info.systemRecoveryProcess != CPSS_SYSTEM_RECOVERY_PROCESS_HA_E)&&(managerHwWriteBlock == GT_TRUE))||
            ((tempSystemRecovery_Info.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_HA_E) &&
            (tempSystemRecovery_Info.systemRecoveryState == CPSS_SYSTEM_RECOVERY_HW_CATCH_UP_STATE_E)))
        {
            /* in HA process we want to add the prefixes to the shadow one by one and not using bulk */
            return prvCpssDxChLpmRamIpv6UcPrefixBulkAddForHaSip7(shadowPtr,
                                                                 ipv6PrefixArrayLen,
                                                                 ipv6PrefixArrayPtr,
                                                                 defragmentationEnable);
        }

        /* first go over the prefixes and update the shadow */
        for (i = 0 ; i < ipv6PrefixArrayLen ; i++)
        {
            retVal = prvCpssDxChLpmConvertIpRouteEntryToLpmRouteEntry(shadowPtr->shadowType,
                                                                      &(ipv6PrefixArrayPtr[i].nextHopInfo),
                                                                      &lpmRouteEntry);
            if (retVal != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(retVal, LOG_ERROR_NO_MSG);
            }
            retVal = prvCpssDxChLpmRouteEntryPointerCheckSip7(&(lpmRouteEntry.routeEntry));
            if (retVal != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(retVal, LOG_ERROR_NO_MSG);
            }

            ipv6PrefixArrayPtr[i].returnStatus =
                prvCpssDxChLpmSip7RamUcEntryAdd(ipv6PrefixArrayPtr[i].vrId,
                                                ipv6PrefixArrayPtr[i].ipAddr.arIP,
                                                ipv6PrefixArrayPtr[i].prefixLen,
                                                &lpmRouteEntry.routeEntry,
                                                PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_SDW_MEM_MODE_E,
                                                ipv6PrefixArrayPtr[i].override,
                                                PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E,
                                                shadowPtr,
                                                defragmentationEnable);

            if (ipv6PrefixArrayPtr[i].returnStatus != GT_OK)
            {
                /* reset pending array for next use */
                cpssOsMemSet(shadowPtr->pendingBlockToUpdateArr,0,sizeof(shadowPtr->pendingBlockToUpdateArr));
                /* need to set the last valid state of globalMemoryBlockTakenArr for next add*/
                cpssOsMemCpy(shadowPtr->globalMemoryBlockTakenArr,tempGlobalMemoryBlockTakenArr,sizeof(tempGlobalMemoryBlockTakenArr));
                retVal2 = ipv6PrefixArrayPtr[i].returnStatus;
            }
            else
            {
                retVal1 = prvCpssDxChLpmRamAddBulkPendingBlockValuesToTotalPendingBlockListSip7(tempPendingBlockToUpdateArr,
                                                                                            shadowPtr->pendingBlockToUpdateArr);
                if (retVal1!=GT_OK)
                {
                    return retVal1;
                }
                else
                {
                    /* keep the last valid state of globalMemoryBlockTakenArr */
                    cpssOsMemCpy(tempGlobalMemoryBlockTakenArr,shadowPtr->globalMemoryBlockTakenArr,sizeof(tempGlobalMemoryBlockTakenArr));
                }
            }
        }

        prvCpssDxChLpmConvertIpProtocolStackToProtocolBitmap(CPSS_IP_PROTOCOL_IPV6_E, &protocolBitmap);
        /* now update the HW */
        retVal1 = updateHwSearchStcSip7(PRV_CPSS_DXCH_LPM_RAM_SEARCH_MEM_UPDATE_E,
                                    protocolBitmap,
                                    shadowPtr);
        if (retVal1 != GT_OK)
        {
            /* reset pending array for next use */
            cpssOsMemSet(shadowPtr->pendingBlockToUpdateArr,0,sizeof(shadowPtr->pendingBlockToUpdateArr));
            /* need to reset the array */
            cpssOsMemSet(shadowPtr->globalMemoryBlockTakenArr, 0, sizeof(shadowPtr->globalMemoryBlockTakenArr));
            return (retVal1);
        }
        else
        {
            retVal1 = prvCpssDxChLpmRamAddBulkPendingBlockValuesToTotalPendingBlockListSip7(tempPendingBlockToUpdateArr,
                                                                                        shadowPtr->pendingBlockToUpdateArr);
            if (retVal1!=GT_OK)
            {
                /* need to reset the array */
                cpssOsMemSet(shadowPtr->globalMemoryBlockTakenArr, 0, sizeof(shadowPtr->globalMemoryBlockTakenArr));
                return retVal1;
            }
            retVal1 = prvCpssDxChLpmRamUpdateBlockUsageCountersSip7(shadowPtr->lpmRamBlocksSizeArrayPtr,
                                                                tempPendingBlockToUpdateArr,
                                                                shadowPtr->protocolCountersPerBlockArr,
                                                                shadowPtr->pendingBlockToUpdateArr,
                                                                PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E,
                                                                shadowPtr->numOfLpmMemories);
            if (retVal1!=GT_OK)
            {
                /* reset pending array for future use */
                cpssOsMemSet(shadowPtr->pendingBlockToUpdateArr,0,sizeof(shadowPtr->pendingBlockToUpdateArr));
                /* need to reset the array */
                cpssOsMemSet(shadowPtr->globalMemoryBlockTakenArr, 0, sizeof(shadowPtr->globalMemoryBlockTakenArr));
                return retVal1;
            }
        }
        /* after bulk end we need to reset the array */
        cpssOsMemSet(shadowPtr->globalMemoryBlockTakenArr, 0, sizeof(shadowPtr->globalMemoryBlockTakenArr));
    }
    return (retVal2);
}

/**
* @internal prvCpssDxChLpmRamIpv6UcPrefixDelSip7 function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         deletes an existing Ipv6 prefix in a Virtual Router for the specified
*         LPM DB.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                  Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] lpmDbPtr                 - The LPM DB
* @param[in] vrId                     - The virtual router id.
* @param[in] ipAddr                   - Points to the destination IP address of the prefix.
* @param[in] prefixLen                - The number of bits that are actual valid in the ipAddr.
*
* @retval GT_OK                    - if success, or
* @retval GT_OUT_OF_RANGE          - if prefix length is too big, or
* @retval GT_ERROR                 - if the vrId was not created yet, or
* @retval GT_BAD_PTR               - if NULL pointer, or
* @retval GT_NOT_FOUND             - if the given prefix doesn't exitst in the VR, or
* @retval GT_NOT_APPLICABLE_DEVICE - if not applicable device, or
* @retval GT_FAIL                  - otherwise.
*
* @note The default prefix (prefixLen = 0) can't be deleted!
*
*/
GT_STATUS prvCpssDxChLpmRamIpv6UcPrefixDelSip7
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC *lpmDbPtr,
    IN GT_U32                               vrId,
    IN GT_IPV6ADDR                          ipAddr,
    IN GT_U32                               prefixLen
)
{
    GT_STATUS           retVal = GT_OK;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC *shadowPtr;
    GT_U32 shadowIdx;

    /* if initialization has not been done for the requested protocol stack -
    return error */
    if (PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_STATUS_GET_MAC(lpmDbPtr->protocolBitmap) == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    for (shadowIdx = 0 ; shadowIdx < lpmDbPtr->numOfShadowCfg; shadowIdx++)
    {
        shadowPtr = &lpmDbPtr->shadowArray[shadowIdx];

        retVal = prvCpssDxChLpmRamSip7UcEntryDel(vrId,
                                                 ipAddr.arIP,
                                                 prefixLen,
                                                 GT_TRUE,
                                                 PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E,
                                                 shadowPtr,
                                                 GT_FALSE);
        if (retVal != GT_OK)
        {
            break;
        }
    }
    return retVal;
}

/**
* @internal prvCpssDxChLpmRamIpv6UcPrefixBulkDelSip7 function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         deletes an existing bulk of Ipv6 prefixes in a Virtual Router for the
*         specified LPM DB.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                  Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] lpmDbPtr                 - The LPM DB
* @param[in] ipv6PrefixArrayLen       - Length of UC prefix array.
* @param[in] ipv6PrefixArrayPtr       - The UC prefix array.
*
* @retval GT_OK                    - if all prefixes were successfully deleted
* @retval GT_OUT_OF_RANGE          - if prefix length is too big
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_FAIL                  - if deleting one or more prefixes failed; the
*                                       array will contain return status for each prefix
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note none.
*
*/
GT_STATUS prvCpssDxChLpmRamIpv6UcPrefixBulkDelSip7
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC     *lpmDbPtr,
    IN GT_U32                                   ipv6PrefixArrayLen,
    IN CPSS_DXCH_IP_LPM_IPV6_UC_PREFIX_STC      *ipv6PrefixArrayPtr
)
{
    GT_STATUS           retVal = GT_OK;
    GT_U32      i;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC *shadowPtr;
    GT_U32 shadowIdx;
    PRV_CPSS_DXCH_LPM_RAM_PENDING_BLOCK_TO_UPDATE_STC   tempPendingBlockToUpdateArr[PRV_CPSS_DXCH_LPM_RAM_NUM_OF_MEMORIES_SIP7_CNS];

    cpssOsMemSet(tempPendingBlockToUpdateArr,0,sizeof(tempPendingBlockToUpdateArr));

    if(ipv6PrefixArrayLen == 0)
        return GT_OK;

    /* if initialization has not been done for the requested protocol stack -
    return error */
    if (PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_STATUS_GET_MAC(lpmDbPtr->protocolBitmap) == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    for (shadowIdx = 0 ; shadowIdx < lpmDbPtr->numOfShadowCfg; shadowIdx++)
    {
        shadowPtr = &lpmDbPtr->shadowArray[shadowIdx];
        /* keep value of prending array. update of counters should be done only after HW write is ok */
        cpssOsMemCpy(tempPendingBlockToUpdateArr,shadowPtr->pendingBlockToUpdateArr,
                     sizeof(tempPendingBlockToUpdateArr));

        for (i = 0 ; i < ipv6PrefixArrayLen ; i++)
        {
            if (ipv6PrefixArrayPtr[i].prefixLen > 128)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
            }

            if (ipv6PrefixArrayPtr[i].ipAddr.arIP[0] >= PRV_CPSS_DXCH_LPM_RAM_START_OF_IPV6_MC_ADDRESS_SPACE_CNS)
            {
                /* Multicast range */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }

            ipv6PrefixArrayPtr[i].returnStatus =
                 prvCpssDxChLpmRamSip7UcEntryDel(ipv6PrefixArrayPtr[i].vrId,
                                                 ipv6PrefixArrayPtr[i].ipAddr.arIP,
                                                 ipv6PrefixArrayPtr[i].prefixLen,
                                                 GT_TRUE,
                                                 PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E,
                                                 shadowPtr,
                                                 GT_FALSE);

            if (ipv6PrefixArrayPtr[i].returnStatus != GT_OK)
            {
                /* reset pending array for next use */
                cpssOsMemSet(shadowPtr->pendingBlockToUpdateArr,0,sizeof(shadowPtr->pendingBlockToUpdateArr));
                retVal = ipv6PrefixArrayPtr[i].returnStatus;
            }
        }
    }
    return (retVal);
}

/**
* @internal prvCpssDxChLpmRamIpv6UcPrefixesFlushSip7 function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         flushes the unicast IPv6 Routing table and stays with the default prefix
*         only for a specific LPM DB.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                  Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] lpmDbPtr                 - The LPM DB
* @param[in] vrId                     - The virtual router identifier.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmRamIpv6UcPrefixesFlushSip7
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC *lpmDbPtr,
    IN GT_U32                               vrId
)
{
    GT_STATUS           retVal = GT_OK;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC *shadowPtr;
    GT_U32 shadowIdx;

    /* if initialization has not been done for the requested protocol stack -
    return error */
    if (PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_STATUS_GET_MAC(lpmDbPtr->protocolBitmap) == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    for (shadowIdx = 0 ; shadowIdx < lpmDbPtr->numOfShadowCfg; shadowIdx++)
    {
        shadowPtr = &lpmDbPtr->shadowArray[shadowIdx];
        if (shadowPtr->vrRootBucketArray[vrId].valid == GT_FALSE)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
        }
        retVal = prvCpssDxChLpmRamUcEntriesFlushSip7(vrId,
                                                 PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E,
                                                 shadowPtr);
        if (retVal != GT_OK)
        {
            break;
        }
    }
    return (retVal);
}

/**
* @internal prvCpssDxChLpmRamIpv6UcPrefixSearchSip7 function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         This function searches for a given ip-uc address, and returns the next
*         hop pointer information bound to it if found.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                  Falcon; AC5P; AC5X; Harrier; Ironman.
**
* @param[in] lpmDbPtr                 - The LPM DB
* @param[in] vrId                     - The virtual router id.
* @param[in] ipAddr                   - Points to the destination IP address to look for.
* @param[in] prefixLen                - The number of bits that are actual valid in the
* @param[in] ipAddr
*
* @param[out] nextHopInfoPtr           - If found this is The next hop pointer to for this
*                                      prefix.
*
* @retval GT_OK                    - if the required entry was found
* @retval GT_OUT_OF_RANGE          - if prefix length is too big
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_FOUND             - if the given ip prefix was not found.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmRamIpv6UcPrefixSearchSip7
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC         *lpmDbPtr,
    IN  GT_U32                                      vrId,
    IN  GT_IPV6ADDR                                 ipAddr,
    IN  GT_U32                                      prefixLen,
    OUT PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC   *nextHopInfoPtr
)
{
    GT_STATUS           retVal = GT_OK;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC *shadowPtr;

    /* if initialization has not been done for the requested protocol stack -
    return error */
    if (PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_STATUS_GET_MAC(lpmDbPtr->protocolBitmap) == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    /* the search can be done on one shadow */
    shadowPtr = &lpmDbPtr->shadowArray[0];
    retVal = prvCpssDxChLpmRamUcEntrySearchSip7(vrId,ipAddr.arIP,prefixLen,
                                            nextHopInfoPtr,
                                          PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E,
                                            shadowPtr);
    return (retVal);
}

/**
* @internal prvCpssDxChLpmRamIpv6UcPrefixGetNextSip7 function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         This function returns an IP-Unicast prefix with larger (ipAddrPtr,prefixLenPtr)
*         than the given one, it used for iterating over the existing prefixes.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                  Falcon; AC5P; AC5X; Harrier; Ironman.
**
* @param[in] lpmDbPtr                 - The LPM DB
* @param[in] vrId                     - The virtual router Id to get the entry from.
* @param[in,out] ipAddrPtr                - The ip address to start the search from.
* @param[in,out] prefixLenPtr             - Prefix length of ipAddr.
* @param[in,out] ipAddrPtr                - The ip address of the found entry.
* @param[in,out] prefixLenPtr             - The prefix length of the found entry.
*
* @param[out] nextHopInfoPtr           - the next hop pointer associated with the found
*                                      ipAddr.
*
* @retval GT_OK                    - if the required entry was found
* @retval GT_OUT_OF_RANGE          - if prefix length is too big
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_FOUND             - if no more entries are left in the IP table.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. The values of (ipAddrPtr,prefixLenPtr) must be a valid values, it
*       means that they exist in the IP-UC Table, unless this is the first
*       call to this function, then the value of (ipAddrPtr,prefixLenPtr) is
*       (0,0).
*       2. In order to get route pointer information for (0,0) use the Ipv6
*       UC prefix get function.
*
*/
GT_STATUS prvCpssDxChLpmRamIpv6UcPrefixGetNextSip7
(
    IN    PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC      *lpmDbPtr,
    IN    GT_U32                                    vrId,
    INOUT GT_IPV6ADDR                               *ipAddrPtr,
    INOUT GT_U32                                    *prefixLenPtr,
    OUT   PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC *nextHopInfoPtr
)
{
    GT_STATUS           retVal = GT_OK;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC *shadowPtr;
    PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC *tmpNextHopInfoPtr;

    /* if initialization has not been done for the requested protocol stack -
    return error */
    if (PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_STATUS_GET_MAC(lpmDbPtr->protocolBitmap) == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    /* the search can be done on one shadow */
    shadowPtr = &lpmDbPtr->shadowArray[0];
    if (shadowPtr->vrRootBucketArray[vrId].valid == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }
    retVal = prvCpssDxChLpmRamUcEntryGetSip7(vrId,ipAddrPtr->arIP,prefixLenPtr,
                                         &tmpNextHopInfoPtr,
                                         PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E,
                                         shadowPtr);
    if (retVal == GT_OK)
    {
        cpssOsMemCpy(nextHopInfoPtr,tmpNextHopInfoPtr,
                     sizeof(PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC));
    }

    return retVal;
}

/**
* @internal prvCpssDxChLpmRamIpv6UcPrefixGetSip7 function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         This function gets a given ip address, and returns the next
*         hop pointer information bounded to the longest prefix match.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                  Falcon; AC5P; AC5X; Harrier; Ironman.
**
* @param[in] lpmDbPtr                 - The LPM DB
* @param[in] vrId                     - The virtual router id.
* @param[in] ipAddr                   - The destination IP address to look for.
*
* @param[out] prefixLenPtr             - Points to the number of bits that are actual valid
*                                      in the longest match
* @param[out] nextHopInfoPtr           - The next hop pointer bounded to the longest match
*
* @retval GT_OK                    - if the required entry was found, or
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_FOUND             - if the given ip prefix was not found.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmRamIpv6UcPrefixGetSip7
(
    IN  PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC        *lpmDbPtr,
    IN  GT_U32                                      vrId,
    IN  GT_IPV6ADDR                                 ipAddr,
    OUT GT_U32                                      *prefixLenPtr,
    OUT PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC   *nextHopInfoPtr
)
{
    GT_STATUS                       retVal = GT_OK;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC   *shadowPtr;

    /* if initialization has not been done for the requested protocol stack -
       return error */
    if (PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_STATUS_GET_MAC(lpmDbPtr->protocolBitmap) == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    /* the search can be done on one shadow */
    shadowPtr = &lpmDbPtr->shadowArray[0];
    retVal = prvCpssDxChLpmRamUcEntryLpmSearchSip7(vrId,ipAddr.arIP,prefixLenPtr,
                                               nextHopInfoPtr,
                                               PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E,
                                               shadowPtr);
    return (retVal);
}

/**
* @internal prvCpssDxChLpmRamIpv6McEntryAddSip7 function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         To add the multicast routing information for IP datagrams from
*         a particular source and addressed to a particular IP multicast
*         group address for a specific LPM DB.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                  Falcon; AC5P; AC5X; Harrier; Ironman.
**
* @param[in] lpmDbPtr                 - The LPM DB
* @param[in] vrId                     - The virtual private network identifier.
* @param[in] ipGroup                  - The IP multicast group address.
* @param[in] ipGroupPrefixLen         - The number of bits that are actual valid in,
*                                      the ipGroup.
* @param[in] ipSrc                    - the root address for source base multi tree protocol.
* @param[in] ipSrcPrefixLen           - The number of bits that are actual valid in,
*                                      the ipSrc.
* @param[in] mcRouteEntryPtr          - (pointer to) the mc Route pointer to set for the mc entry.
* @param[in] override                 -  the existing entry if it already exists
* @param[in] defragmentationEnable    - whether to enable performance costing
*                                      de-fragmentation process in the case that there
*                                      is no place to insert the prefix. To point of the
*                                      process is just to make space for this prefix.
*
* @retval GT_OK                    - on success, or
* @retval GT_OUT_OF_RANGE          - if prefix length is too big, or
* @retval GT_ERROR                 - if the virtual router does not exist, or
* @retval GT_NOT_FOUND             - if prefix was not found when override is GT_TRUE, or
* @retval GT_ALREADY_EXIST         - if prefix already exist when override is GT_FALSE, or
* @retval GT_BAD_PTR               - if NULL pointer, or
* @retval GT_OUT_OF_CPU_MEM        - if failed to allocate CPU memory, or
* @retval GT_OUT_OF_PP_MEM         - if failed to allocate PP memory, or
* @retval GT_NOT_APPLICABLE_DEVICE - if not applicable device, or
* @retval GT_FAIL                  - otherwise.
*
* @note 1. To override the default mc route use ipGroup = 0.
*       2. If (S,G) MC group is added when (,G) doesn't exists then implicit
*       (,G) is added pointing to (,) default. Application added (,G)
*       will override the implicit (,G).
*
*/
GT_STATUS prvCpssDxChLpmRamIpv6McEntryAddSip7
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC         *lpmDbPtr,
    IN GT_U32                                       vrId,
    IN GT_IPV6ADDR                                  ipGroup,
    IN GT_U32                                       ipGroupPrefixLen,
    IN GT_IPV6ADDR                                  ipSrc,
    IN GT_U32                                       ipSrcPrefixLen,
    IN PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC    *mcRouteEntryPtr,
    IN GT_BOOL                                      override,
    IN GT_BOOL                                      defragmentationEnable
)
{
    GT_STATUS           retVal = GT_OK;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC *shadowPtr;
    GT_U32 shadowIdx;
    PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_MODE_ENT   insertMode;
    CPSS_SYSTEM_RECOVERY_INFO_STC   tempSystemRecovery_Info;
    GT_BOOL                         managerHwWriteBlock;

    if ((ipGroupPrefixLen > 8) && (ipGroupPrefixLen < 128))
    {
        /* in this case all except link local must be rejected */
        PRV_CPSS_DXCH_LPM_IPV6_MC_LINK_LOCAL_CHECK_MAC(ipGroup, ipGroupPrefixLen, ipSrcPrefixLen);
    }

     if ((ipGroupPrefixLen <= 8) && (ipSrcPrefixLen > 0))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }
     /* link local exact mach must be also rejected */
     PRV_CPSS_DXCH_LPM_IPV6_MC_LINK_LOCAL_CHECK_EXACT_MATCH_MAC(ipGroup, ipGroupPrefixLen);

    /* if initialization has not been done for the requested protocol stack -
    return error */
    if (PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_STATUS_GET_MAC(lpmDbPtr->protocolBitmap) == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    for (shadowIdx = 0 ; shadowIdx < lpmDbPtr->numOfShadowCfg; shadowIdx++)
    {
        shadowPtr = &lpmDbPtr->shadowArray[shadowIdx];
        retVal = prvCpssDxChLpmRouteEntryPointerCheckSip7(mcRouteEntryPtr);
        if (retVal != GT_OK)
            return retVal;

        retVal = cpssSystemRecoveryStateGet(&tempSystemRecovery_Info);
        if (retVal != GT_OK)
        {
            return retVal;
        }

        managerHwWriteBlock = SYSTEM_RECOVERY_CHECK_MANAGER_HW_WRITE_BLOCK_MAC(CPSS_SYSTEM_RECOVERY_LPM_MANAGER_E);
        if((tempSystemRecovery_Info.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_HA_E) ||
           (managerHwWriteBlock == GT_TRUE))
        {
            insertMode=PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_SDW_ONLY_MODE_E;
        }
        else
        {
            insertMode=PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_SDW_MEM_HW_MODE_E;
        }

        retVal = prvCpssDxChLpmSip7RamMcEntryAdd(vrId,ipGroup.arIP,ipGroupPrefixLen,
                                                 ipSrc.arIP,ipSrcPrefixLen,
                                                 mcRouteEntryPtr,
                                                 override,
                                                 insertMode,
                                                 PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E,
                                                 shadowPtr,
                                                 defragmentationEnable);
        /* reset the array for next use */
        cpssOsMemSet(shadowPtr->globalMemoryBlockTakenArr, 0, sizeof(shadowPtr->globalMemoryBlockTakenArr));
    }
    return (retVal);
}

/**
* @internal prvCpssDxChLpmRamIpv6McEntryDelSip7 function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         To delete a particular mc route entry for a specific LPM DB.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                  Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] lpmDbPtr                 - The LPM DB
* @param[in] vrId                     - The virtual router identifier.
* @param[in] ipGroup                  - The IP multicast group address.
* @param[in] ipGroupPrefixLen         - The number of bits that are actual valid in,
*                                      the ipGroup.
* @param[in] ipSrc                    - the root address for source base multi tree protocol.
* @param[in] ipSrcPrefixLen           - The number of bits that are actual valid in,
*                                      the ipSrc.
*
* @retval GT_OK                    - on success, or
* @retval GT_OUT_OF_RANGE          - if prefix length is too big, or
* @retval GT_ERROR                 - if the virtual router does not exist, or
* @retval GT_NOT_FOUND             - if the (ipGroup,prefix) does not exist, or
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device, or
* @retval GT_FAIL                  - otherwise.
*
* @note 1. Inorder to delete the multicast entry and all the src ip addresses
*       associated with it, call this function with ipSrc = ipSrcPrefixLen = 0.
*       2. If no (,G) was added but (S,G) were added, then implicit (,G)
*       that points to (,) is added. If (,G) is added later, it will
*       replace the implicit (,G).
*       When deleting (,G), then if there are still (S,G), an implicit (,G)
*       pointing to (,) will be added.
*       When deleting last (S,G) and the (,G) was implicitly added, then
*       the (,G) will be deleted as well.
*
*/
GT_STATUS prvCpssDxChLpmRamIpv6McEntryDelSip7
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC *lpmDbPtr,
    IN GT_U32                               vrId,
    IN GT_IPV6ADDR                          ipGroup,
    IN GT_U32                               ipGroupPrefixLen,
    IN GT_IPV6ADDR                          ipSrc,
    IN GT_U32                               ipSrcPrefixLen
)
{
    GT_STATUS           retVal = GT_OK;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC *shadowPtr;
    GT_U32 shadowIdx;

    /* if initialization has not been done for the requested protocol stack -
    return error */
    if (PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_STATUS_GET_MAC(lpmDbPtr->protocolBitmap) == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    for (shadowIdx = 0 ; shadowIdx < lpmDbPtr->numOfShadowCfg; shadowIdx++)
    {
        shadowPtr = &lpmDbPtr->shadowArray[shadowIdx];

        retVal = prvCpssDxChLpmSip7RamMcEntryDelete(vrId,ipGroup.arIP,ipGroupPrefixLen,
                                                    ipSrc.arIP,ipSrcPrefixLen,
                                                    PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E,
                                                    shadowPtr,GT_FALSE,NULL);
        if (retVal != GT_OK)
        {
            break;
        }
    }
    return (retVal);
}

/**
* @internal prvCpssDxChLpmRamIpv6McEntriesFlushSip7 function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         flushes the multicast IP Routing table and stays with the default entry
*         only for a specific LPM DB.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                  Falcon; AC5P; AC5X; Harrier; Ironman.
**
* @param[in] lpmDbPtr                 - The LPM DB
* @param[in] vrId                     - The virtual router identifier.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmRamIpv6McEntriesFlushSip7
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC *lpmDbPtr,
    IN GT_U32                               vrId
)
{
    GT_STATUS           retVal = GT_OK;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC *shadowPtr;
    GT_U32 shadowIdx;

    /* if initialization has not been done for the requested protocol stack -
    return error */
    if (PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_STATUS_GET_MAC(lpmDbPtr->protocolBitmap) == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    for (shadowIdx = 0 ; shadowIdx < lpmDbPtr->numOfShadowCfg; shadowIdx++)
    {
        shadowPtr = &lpmDbPtr->shadowArray[shadowIdx];
        retVal = prvCpssDxChLpmRamMcEntriesFlushSip7(vrId,
                                                 PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E,
                                                 shadowPtr);
        if (retVal != GT_OK)
        {
            break;
        }
    }
    return (retVal);
}

/**
* @internal prvCpssDxChLpmRamIpv6McEntryGetSip7 function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         This function returns the muticast (ipSrc,ipGroup) entry, used
*         to find specific multicast adrress entry
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                  Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] lpmDbPtr                 - The LPM DB
* @param[in] vrId                     - The virtual router Id.
* @param[in] ipGroup                  - The ip Group address to get the next entry for.
* @param[in] ipGroupPrefixLen         - ipGroup prefix length.
* @param[in] ipSrc                    - The ip Source address to get the next entry for.
* @param[in] ipSrcPrefixLen           - ipSrc prefix length.
*
* @param[out] mcRouteEntryPtr          - (pointer to) the mc route entry ptr of the found mc mc route
*
* @retval GT_OK                    - if found, or
* @retval GT_OUT_OF_RANGE          - if prefix length is too big, or
* @retval GT_BAD_PTR               - if NULL pointer, or
* @retval GT_NOT_FOUND             - if the given address is the last one on the IP-Mc table, or
* @retval GT_NOT_APPLICABLE_DEVICE - if not applicable device.
*/
GT_STATUS prvCpssDxChLpmRamIpv6McEntryGetSip7
(
    IN  PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC        *lpmDbPtr,
    IN  GT_U32                                      vrId,
    IN  GT_IPV6ADDR                                 ipGroup,
    IN  GT_U32                                      ipGroupPrefixLen,
    IN  GT_IPV6ADDR                                 ipSrc,
    IN  GT_U32                                      ipSrcPrefixLen,
    OUT PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC   *mcRouteEntryPtr
)
{
    GT_STATUS                               retVal = GT_OK;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC        *shadowPtr;

    /* if initialization has not been done for the requested protocol stack -
    return error */
    if (PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_STATUS_GET_MAC(lpmDbPtr->protocolBitmap) == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    if (lpmDbPtr->shadowArray->vrRootBucketArray[vrId].valid == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }

    /* it's enough to look at the first shadow */
    shadowPtr = &lpmDbPtr->shadowArray[0];
    retVal = prvCpssDxChLpmRamMcEntrySearchSip7(vrId,ipGroup.arIP,ipGroupPrefixLen,
                                            ipSrc.arIP,ipSrcPrefixLen,
                                            mcRouteEntryPtr,
                                            PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E,
                                            shadowPtr);
    return (retVal);
}

/**
* @internal prvCpssDxChLpmRamIpv6McEntryGetNextSip7 function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         This function returns the next muticast (ipSrcPtr,ipGroupPtr) entry, used
*         to iterate over the existing multicast addresses for a specific LPM DB.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                  Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] lpmDbPtr                 - The LPM DB
* @param[in] vrId                     - The virtual router Id.
* @param[in,out] ipGroupPtr               - (pointer to) The ip Group address to get the next entry for.
* @param[in,out] ipGroupPrefixLenPtr      - (pointer to) the ipGroupPtr prefix length.
* @param[in,out] ipSrcPtr                 - (pointer to) The ip Source address to get the next entry for.
* @param[in,out] ipSrcPrefixLenPtr        - (pointer to) ipSrcPtr prefix length.
* @param[in,out] ipGroupPtr               - (pointer to) The next ip Group address.
* @param[in,out] ipGroupPrefixLenPtr      - (pointer to) the ipGroupPtr prefix length.
* @param[in,out] ipSrcPtr                 - (pointer to) The next ip Source address.
* @param[in,out] ipSrcPrefixLenPtr        - (pointer to) ipSrcPtr prefix length.
*
* @param[out] mcRouteEntryPtr          - (pointer to) the mc route entry ptr of the found mc mc route
*
* @retval GT_OK                    - if found, or
* @retval GT_OUT_OF_RANGE          - if prefix length is too big, or
* @retval GT_BAD_PTR               - if NULL pointer, or
* @retval GT_NOT_FOUND             - if the given address is the last one on the IP-Mc table, or
* @retval GT_NOT_APPLICABLE_DEVICE - if not applicable device
*
* @note 1. The values of (ipSrcPtr,ipGroupPtr) must be a valid values, it
*       means that they exist in the IP-Mc Table, unless this is the first
*       call to this function, then the value of (ipSrcPtr,ipGroupPtr) is
*       (0,0).
*       2. In order to get route pointer information for (0,0) use the Ipv6
*       MC get function.
*
*/
GT_STATUS prvCpssDxChLpmRamIpv6McEntryGetNextSip7
(
    IN    PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC      *lpmDbPtr,
    IN    GT_U32                                    vrId,
    INOUT GT_IPV6ADDR                               *ipGroupPtr,
    INOUT GT_U32                                    *ipGroupPrefixLenPtr,
    INOUT GT_IPV6ADDR                               *ipSrcPtr,
    INOUT GT_U32                                    *ipSrcPrefixLenPtr,
    OUT   PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC *mcRouteEntryPtr
)
{
    GT_STATUS           retVal = GT_OK;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC *shadowPtr;

    /* if initialization has not been done for the requested protocol stack -
    return error */
    if (PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_STATUS_GET_MAC(lpmDbPtr->protocolBitmap) == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    if (lpmDbPtr->shadowArray->vrRootBucketArray[vrId].valid == GT_FALSE)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);

    /* it's enough to look at the first shadow */
    shadowPtr = &lpmDbPtr->shadowArray[0];
    retVal = prvCpssDxChLpmRamMcEntryGetNextSip7(vrId,ipGroupPtr->arIP,ipGroupPrefixLenPtr,
                                             ipSrcPtr->arIP,
                                             ipSrcPrefixLenPtr,
                                             mcRouteEntryPtr,
                                             PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E,
                                             shadowPtr);
    /* group prefix 0 means this is the default entry (*,*) */
    if (*ipGroupPrefixLenPtr == 0)
    {
        cpssOsMemSet(ipGroupPtr->u32Ip,0,sizeof(GT_U32)*4);
    }
    return (retVal);
}

/**
* @internal prvCpssDxChLpmRamFcoePrefixAddSip7 function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         This function adds a new FCoE prefix to a Virtual Router in a
*         specific LPM DB or overrides an existing existing FCoE prefix.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                  Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] lpmDbPtr                 - The LPM DB
* @param[in] vrId                     - The virtual router id.
* @param[in] fcoeAddr                 - The FC_ID of this prefix.
* @param[in] prefixLen                - The number of bits that are actual valid in the FC_ID.
* @param[in] nextHopInfoPtr           - (points to) The next hop pointer to set for this prefix.
* @param[in] override                 -  the existing entry if it already exists
* @param[in] defragmentationEnable    - whether to enable performance costing
*                                      de-fragmentation process in the case that there
*                                      is no place to insert the prefix. To point of the
*                                      process is just to make space for this prefix.
*
* @retval GT_OK                    - on success, or
* @retval GT_OUT_OF_RANGE          - If prefix length is too big, or
* @retval GT_ERROR                 - If the vrId was not created yet, or
* @retval GT_ALREADY_EXIST         - prefix already exist when override is GT_FALSE
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_OUT_OF_CPU_MEM        - If failed to allocate CPU memory, or
* @retval GT_OUT_OF_PP_MEM         - If failed to allocate PP memory, or
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note To change the default prefix for the VR use prefixLen = 0.
*
*/
GT_STATUS prvCpssDxChLpmRamFcoePrefixAddSip7
(
    IN  PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC        *lpmDbPtr,
    IN  GT_U32                                      vrId,
    IN  GT_FCID                                     fcoeAddr,
    IN  GT_U32                                      prefixLen,
    IN  PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC   *nextHopInfoPtr,
    IN  GT_BOOL                                     override,
    IN GT_BOOL                                      defragmentationEnable
)
{
    GT_STATUS                           retVal = GT_OK;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC    *shadowPtr;
    GT_U32                              shadowIdx;
    PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_MODE_ENT   insertMode;
    CPSS_SYSTEM_RECOVERY_INFO_STC       tempSystemRecovery_Info;
    GT_BOOL                             managerHwWriteBlock;

    /* if initialization has not been done for the requested protocol stack -
    return error */
    if (PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_STATUS_GET_MAC(lpmDbPtr->protocolBitmap) == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    for (shadowIdx = 0 ; shadowIdx < lpmDbPtr->numOfShadowCfg; shadowIdx++)
    {
        shadowPtr = &lpmDbPtr->shadowArray[shadowIdx];

        retVal = prvCpssDxChLpmRouteEntryPointerCheckSip7(nextHopInfoPtr);
        if (retVal != GT_OK)
            return retVal;

        retVal = cpssSystemRecoveryStateGet(&tempSystemRecovery_Info);
        if (retVal != GT_OK)
        {
            return retVal;
        }
        managerHwWriteBlock = SYSTEM_RECOVERY_CHECK_MANAGER_HW_WRITE_BLOCK_MAC(CPSS_SYSTEM_RECOVERY_LPM_MANAGER_E);
        if((tempSystemRecovery_Info.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_HA_E) ||
           (managerHwWriteBlock == GT_TRUE))
        {
            insertMode=PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_SDW_ONLY_MODE_E;
        }
        else
        {
            insertMode=PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_SDW_MEM_HW_MODE_E;
        }

        retVal = prvCpssDxChLpmSip7RamUcEntryAdd(vrId,
                                                 &fcoeAddr.fcid[0],
                                                 prefixLen,
                                                 nextHopInfoPtr,
                                                 insertMode,
                                                 override,
                                                 PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E,
                                                 shadowPtr,
                                                 defragmentationEnable);

        if (retVal != GT_OK)
        {
            /* reset the array for next use */
            cpssOsMemSet(shadowPtr->globalMemoryBlockTakenArr, 0, sizeof(shadowPtr->globalMemoryBlockTakenArr));
            break;
        }
        /* reset the array for next use */
        cpssOsMemSet(shadowPtr->globalMemoryBlockTakenArr, 0, sizeof(shadowPtr->globalMemoryBlockTakenArr));
    }
    return retVal;
}

/**
* @internal prvCpssDxChLpmRamFcoePrefixDelSip7 function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         Deletes an existing FCoE prefix in a Virtual Router for the specified
*         LPM DB.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                  Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] lpmDbPtr                 - The LPM DB.
* @param[in] vrId                     - The virtual router id.
* @param[in] fcoeAddr                 - The destination FC_ID address of the prefix.
* @param[in] prefixLen                - The number of bits that are actual valid in the FC_ID.
*
* @retval GT_OK                    - on success, or
* @retval GT_OUT_OF_RANGE          - If prefix length is too big, or
* @retval GT_ERROR                 - If the vrId was not created yet, or
* @retval GT_NOT_FOUND             - If the given prefix doesn't exitst in the VR, or
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note the default prefix (prefixLen = 0) can't be deleted!
*
*/
GT_STATUS prvCpssDxChLpmRamFcoePrefixDelSip7
(
    IN  PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC    *lpmDbPtr,
    IN  GT_U32                                  vrId,
    IN  GT_FCID                                 fcoeAddr,
    IN  GT_U32                                  prefixLen
)
{
    GT_STATUS           retVal = GT_OK;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC *shadowPtr;
    GT_U32 shadowIdx;

    /* if initialization has not been done for the requested protocol stack -
    return error */
    if (PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_STATUS_GET_MAC(lpmDbPtr->protocolBitmap) == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    for (shadowIdx = 0 ; shadowIdx < lpmDbPtr->numOfShadowCfg; shadowIdx++)
    {
        shadowPtr = &lpmDbPtr->shadowArray[shadowIdx];

        if (shadowPtr->vrRootBucketArray[vrId].valid == GT_FALSE)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
        }

        retVal = prvCpssDxChLpmRamSip7UcEntryDel(vrId,
                                                 fcoeAddr.fcid,
                                                 prefixLen,
                                                 GT_TRUE,
                                                 PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E,
                                                 shadowPtr,
                                                 GT_FALSE);
        if (retVal != GT_OK)
        {
            break;
        }
    }
    return retVal;
}

/**
* @internal prvCpssDxChLpmRamFcoePrefixAddBulkSip7 function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         Creates a new or override an existing bulk of FCoE prefixes in a Virtual
*         Router for the specified LPM DB.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                  Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] lpmDbPtr                 - The LPM DB
* @param[in] fcoeAddrPrefixArrayLen   - Length of FC_ID prefix array.
* @param[in] fcoeAddrPrefixArrayPtr   - The FC_ID prefix array.
* @param[in] defragmentationEnable    - whether to enable performance costing
*                                      de-fragmentation process in the case that there
*                                      is no place to insert the prefix. To point of the
*                                      process is just to make space for this prefix.
*
* @retval GT_OK                    - if all prefixes were successfully added
* @retval GT_OUT_OF_RANGE          - if prefix length is too big
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_FAIL                  - if adding one or more prefixes failed; the
*                                       array will contain return status for each prefix
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmRamFcoePrefixAddBulkSip7
(
    IN  PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC            *lpmDbPtr,
    IN  GT_U32                                          fcoeAddrPrefixArrayLen,
    IN  CPSS_DXCH_FCOE_LPM_PREFIX_BULK_OPERATION_STC    *fcoeAddrPrefixArrayPtr,
    IN  GT_BOOL                                         defragmentationEnable
)
{
    GT_STATUS           retVal = GT_OK,retVal1=GT_OK,retVal2=GT_OK;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC *shadowPtr;
    GT_U32 shadowIdx;
    GT_U32 i;
    PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_INFO_UNT lpmRouteEntry;
    PRV_CPSS_DXCH_LPM_PROTOCOL_BMP     protocolBitmap=0;
    PRV_CPSS_DXCH_LPM_RAM_PENDING_BLOCK_TO_UPDATE_STC   tempPendingBlockToUpdateArr[PRV_CPSS_DXCH_LPM_RAM_NUM_OF_MEMORIES_SIP7_CNS];
    GT_U32                                              tempGlobalMemoryBlockTakenArr[PRV_CPSS_DXCH_LPM_RAM_NUM_OF_MEMORIES_SIP7_CNS];

    cpssOsMemSet(&lpmRouteEntry,0,sizeof(lpmRouteEntry));
    cpssOsMemSet(tempPendingBlockToUpdateArr,0,sizeof(tempPendingBlockToUpdateArr));
    cpssOsMemSet(tempGlobalMemoryBlockTakenArr,0,sizeof(tempGlobalMemoryBlockTakenArr));

    for (i = 0 ; i < fcoeAddrPrefixArrayLen; i++)
    {
        if (fcoeAddrPrefixArrayPtr[i].prefixLen > 24)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
        }
    }

    /* if initialization has not been done for the requested protocol stack -
    return error */
    if (PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_STATUS_GET_MAC(lpmDbPtr->protocolBitmap) == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    for (shadowIdx = 0 ; shadowIdx < lpmDbPtr->numOfShadowCfg; shadowIdx++)
    {
        shadowPtr = &lpmDbPtr->shadowArray[shadowIdx];

        /* keep value of prending array. update of counters should be done only after HW write is ok */
        cpssOsMemCpy(tempPendingBlockToUpdateArr,shadowPtr->pendingBlockToUpdateArr,sizeof(tempPendingBlockToUpdateArr));
        cpssOsMemCpy(tempGlobalMemoryBlockTakenArr,shadowPtr->globalMemoryBlockTakenArr,sizeof(tempGlobalMemoryBlockTakenArr));

        /* first go over the prefixes and update the shadow */
        for (i = 0 ; i < fcoeAddrPrefixArrayLen ; i++)
        {
            retVal = prvCpssDxChLpmConvertIpRouteEntryToLpmRouteEntry(shadowPtr->shadowType,
                                                                      &(fcoeAddrPrefixArrayPtr[i].nextHopInfo),
                                                                      &lpmRouteEntry);
            if (retVal != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(retVal, LOG_ERROR_NO_MSG);
            }
            retVal = prvCpssDxChLpmRouteEntryPointerCheckSip7(&(lpmRouteEntry.routeEntry));
            if (retVal != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(retVal, LOG_ERROR_NO_MSG);
            }

            fcoeAddrPrefixArrayPtr[i].returnStatus =
                            prvCpssDxChLpmSip7RamUcEntryAdd(fcoeAddrPrefixArrayPtr[i].vrId,
                                                            fcoeAddrPrefixArrayPtr[i].fcoeAddr.fcid,
                                                            fcoeAddrPrefixArrayPtr[i].prefixLen,
                                                            &lpmRouteEntry.routeEntry,
                                                            PRV_CPSS_DXCH_LPM_RAM_TRIE_INSERT_SDW_MEM_MODE_E,
                                                            fcoeAddrPrefixArrayPtr[i].override,
                                                            PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E,
                                                            shadowPtr,
                                                            defragmentationEnable);

            if (fcoeAddrPrefixArrayPtr[i].returnStatus != GT_OK)
            {
                /* reset pending array for next use */
                cpssOsMemSet(shadowPtr->pendingBlockToUpdateArr,0,sizeof(shadowPtr->pendingBlockToUpdateArr));
                /* need to set the last valid state of globalMemoryBlockTakenArr for next add*/
                cpssOsMemCpy(shadowPtr->globalMemoryBlockTakenArr,tempGlobalMemoryBlockTakenArr,sizeof(tempGlobalMemoryBlockTakenArr));
                retVal2 = fcoeAddrPrefixArrayPtr[i].returnStatus;
            }
            else
            {
                retVal1 = prvCpssDxChLpmRamAddBulkPendingBlockValuesToTotalPendingBlockListSip7(tempPendingBlockToUpdateArr,
                                                                                            shadowPtr->pendingBlockToUpdateArr);
                if (retVal1!=GT_OK)
                {
                    /* need to reset the array */
                    cpssOsMemSet(shadowPtr->globalMemoryBlockTakenArr, 0, sizeof(shadowPtr->globalMemoryBlockTakenArr));
                    return retVal1;
                }
                else
                {
                    /* keep the last valid state of globalMemoryBlockTakenArr */
                    cpssOsMemCpy(tempGlobalMemoryBlockTakenArr,shadowPtr->globalMemoryBlockTakenArr,sizeof(tempGlobalMemoryBlockTakenArr));
                }
            }
        }

        prvCpssDxChLpmConvertIpProtocolStackToProtocolBitmap(CPSS_IP_PROTOCOL_FCOE_E, &protocolBitmap);
        /* now update the HW */
        retVal1 = updateHwSearchStcSip7(PRV_CPSS_DXCH_LPM_RAM_SEARCH_MEM_UPDATE_E,
                                    protocolBitmap,
                                    shadowPtr);
        if (retVal1 != GT_OK)
        {
            /* reset pending array for next use */
            cpssOsMemSet(shadowPtr->pendingBlockToUpdateArr,0,sizeof(shadowPtr->pendingBlockToUpdateArr));
            /* need to reset the array */
            cpssOsMemSet(shadowPtr->globalMemoryBlockTakenArr, 0, sizeof(shadowPtr->globalMemoryBlockTakenArr));
            return (retVal1);
        }
        else
        {
            retVal1 = prvCpssDxChLpmRamAddBulkPendingBlockValuesToTotalPendingBlockListSip7(tempPendingBlockToUpdateArr,
                                                                                        shadowPtr->pendingBlockToUpdateArr);
            if (retVal1!=GT_OK)
            {
                /* need to reset the array */
                cpssOsMemSet(shadowPtr->globalMemoryBlockTakenArr, 0, sizeof(shadowPtr->globalMemoryBlockTakenArr));
                return retVal1;
            }

            retVal1 = prvCpssDxChLpmRamUpdateBlockUsageCountersSip7(shadowPtr->lpmRamBlocksSizeArrayPtr,
                                                                tempPendingBlockToUpdateArr,
                                                                shadowPtr->protocolCountersPerBlockArr,
                                                                shadowPtr->pendingBlockToUpdateArr,
                                                                PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E,
                                                                shadowPtr->numOfLpmMemories);
            if (retVal1!=GT_OK)
            {
                /* reset pending array for future use */
                cpssOsMemSet(shadowPtr->pendingBlockToUpdateArr,0,sizeof(shadowPtr->pendingBlockToUpdateArr));
                /* need to reset the array */
                cpssOsMemSet(shadowPtr->globalMemoryBlockTakenArr, 0, sizeof(shadowPtr->globalMemoryBlockTakenArr));
                return retVal1;
            }
        }
        /* after bulk end we need to reset the array */
        cpssOsMemSet(shadowPtr->globalMemoryBlockTakenArr, 0, sizeof(shadowPtr->globalMemoryBlockTakenArr));
    }
    return (retVal2);
}

/**
* @internal prvCpssDxChLpmRamFcoePrefixDelBulkSip7 function
* @endinternal
*
* @brief   Function Relevant mode : High Level mode
*         Deletes an existing bulk of FCoE prefixes in a Virtual Router for the
*         specified LPM DB.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                  Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] lpmDbPtr                 - The LPM DB
* @param[in] fcoeAddrPrefixArrayLen   - Length of FC_ID prefix array.
* @param[in] fcoeAddrPrefixArrayPtr   - The FC_ID prefix array.
*
* @retval GT_OK                    - if all prefixes were successfully deleted
* @retval GT_OUT_OF_RANGE          - if prefix length is too big
* @retval GT_BAD_PTR               - if NULL pointer
* @retval GT_FAIL                  - if deleting one or more prefixes failed; the
*                                       array will contain return status for each prefix
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmRamFcoePrefixDelBulkSip7
(
    IN  PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC            *lpmDbPtr,
    IN  GT_U32                                          fcoeAddrPrefixArrayLen,
    IN  CPSS_DXCH_FCOE_LPM_PREFIX_BULK_OPERATION_STC    *fcoeAddrPrefixArrayPtr
)
{
    GT_STATUS           retVal = GT_OK;
    GT_U32      i;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC *shadowPtr;
    GT_U32 shadowIdx;
    PRV_CPSS_DXCH_LPM_RAM_PENDING_BLOCK_TO_UPDATE_STC   tempPendingBlockToUpdateArr[PRV_CPSS_DXCH_LPM_RAM_NUM_OF_MEMORIES_SIP7_CNS];

    cpssOsMemSet(tempPendingBlockToUpdateArr,0,sizeof(tempPendingBlockToUpdateArr));

    /* if initialization has not been done for the requested protocol stack -
    return error */
    if (PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_STATUS_GET_MAC(lpmDbPtr->protocolBitmap) == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    for (shadowIdx = 0 ; shadowIdx < lpmDbPtr->numOfShadowCfg; shadowIdx++)
    {
        shadowPtr = &lpmDbPtr->shadowArray[shadowIdx];

        /* keep value of prending array. update of counters should be done only after HW write is ok */
        cpssOsMemCpy(tempPendingBlockToUpdateArr,shadowPtr->pendingBlockToUpdateArr,sizeof(tempPendingBlockToUpdateArr));

        for (i = 0 ; i < fcoeAddrPrefixArrayLen ; i++)
        {
            if (fcoeAddrPrefixArrayPtr[i].prefixLen > 24)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
            }

            fcoeAddrPrefixArrayPtr[i].returnStatus =
                                prvCpssDxChLpmRamSip7UcEntryDel(fcoeAddrPrefixArrayPtr[i].vrId,
                                                                fcoeAddrPrefixArrayPtr[i].fcoeAddr.fcid,
                                                                fcoeAddrPrefixArrayPtr[i].prefixLen,
                                                                GT_FALSE,
                                                                PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E,
                                                                shadowPtr,
                                                                GT_FALSE);

            if (fcoeAddrPrefixArrayPtr[i].returnStatus != GT_OK)
            {
                /* reset pending array for next use */
                cpssOsMemSet(shadowPtr->pendingBlockToUpdateArr,0,sizeof(shadowPtr->pendingBlockToUpdateArr));
                retVal = fcoeAddrPrefixArrayPtr[i].returnStatus;
            }
        }
    }
    return (retVal);
}

/**
* @internal prvCpssDxChLpmRamFcoePrefixesFlushSip7 function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         Flushes the FCoE forwarding table and stays with the default prefix
*         only for a specific LPM DB.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                  Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] lpmDbPtr                 - The LPM DB
* @param[in] vrId                     - The virtual router identifier.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmRamFcoePrefixesFlushSip7
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC  *lpmDbPtr,
    IN GT_U32                                vrId
)
{
    GT_STATUS           retVal = GT_OK;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC *shadowPtr;
    GT_U32 shadowIdx;

    /* if initialization has not been done for the requested protocol stack -
    return error */
    if (PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_STATUS_GET_MAC(lpmDbPtr->protocolBitmap) == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    for (shadowIdx = 0 ; shadowIdx < lpmDbPtr->numOfShadowCfg; shadowIdx++)
    {
        shadowPtr = &lpmDbPtr->shadowArray[shadowIdx];
        if (shadowPtr->vrRootBucketArray[vrId].valid == GT_FALSE)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
        }
        retVal = prvCpssDxChLpmRamUcEntriesFlushSip7(vrId,
                                                 PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E,
                                                 shadowPtr);
        if (retVal != GT_OK)
        {
            break;
        }
    }
    return (retVal);
}

/**
* @internal prvCpssDxChLpmRamFcoePrefixSearchSip7 function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         This function searches for a given FC_ID, and returns the next
*         hop pointer information bound to it if found.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                  Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] lpmDbPtr                 - The LPM DB
* @param[in] vrId                     - The virtual router id.
* @param[in] fcoeAddr                 - The FC_ID to look for.
* @param[in] prefixLen                - The number of bits that are actual valid in the
*                                      FC_ID.
*
* @param[out] nextHopInfoPtr           - If found this is The next hop pointer to for this
*                                      prefix.
*
* @retval GT_OK                    - the required entry was found, or
* @retval GT_OUT_OF_RANGE          - the prefix length is too big, or
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_FOUND             - the given prefix was not found.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmRamFcoePrefixSearchSip7
(
    IN  PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC        *lpmDbPtr,
    IN  GT_U32                                      vrId,
    IN  GT_FCID                                     fcoeAddr,
    IN  GT_U32                                      prefixLen,
    OUT PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC   *nextHopInfoPtr
)
{
    GT_STATUS           retVal = GT_OK;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC *shadowPtr;

    /* if initialization has not been done for the requested protocol stack -
    return error */
    if (PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_STATUS_GET_MAC(lpmDbPtr->protocolBitmap) == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    /* the search can be done on one shadow */
    shadowPtr = &lpmDbPtr->shadowArray[0];
    if (shadowPtr->vrRootBucketArray[vrId].valid == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }
    retVal = prvCpssDxChLpmRamUcEntrySearchSip7(vrId,fcoeAddr.fcid,prefixLen,
                                            nextHopInfoPtr,
                                            PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E,
                                            shadowPtr);

    return (retVal);
}

/**
* @internal prvCpssDxChLpmRamFcoePrefixGetNextSip7 function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         This function returns an FCoE prefix with larger (FC_ID,prefix)
*         than the given one, it used for iterating over the existing prefixes.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                  Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] lpmDbPtr                 - The LPM DB.
* @param[in] vrId                     - The virtual router Id to get the entry from.
* @param[in,out] fcoeAddrPtr              - The FC_ID to start the search from.
* @param[in,out] prefixLenPtr             - The number of bits that are actual valid in the
*                                      FC_ID.
* @param[in,out] fcoeAddrPtr              - The FC_ID of the found entry.
* @param[in,out] prefixLenPtr             - The prefix length of the found entry.
*
* @param[out] nextHopInfoPtr           - the next hop pointer associated with the found FC_ID
*
* @retval GT_OK                    - the required entry was found, or
* @retval GT_OUT_OF_RANGE          - the prefix length is too big, or
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_FOUND             - no more entries are left in the FC_ID table.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note 1. The values of (FC_ID,prefix) must be a valid values, it
*       means that they exist in the forwarding Table, unless this is the
*       first call to this function, then the value of (FC_ID,prefix) is
*       (0,0).
*       2. In order to get route pointer information for (0,0) use the FC_ID
*       prefix get function.
*
*/
GT_STATUS prvCpssDxChLpmRamFcoePrefixGetNextSip7
(
    IN    PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC      *lpmDbPtr,
    IN    GT_U32                                    vrId,
    INOUT GT_FCID                                   *fcoeAddrPtr,
    INOUT GT_U32                                    *prefixLenPtr,
    OUT   PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC *nextHopInfoPtr
)
{
    GT_STATUS                                   retVal = GT_OK;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC            *shadowPtr;
    PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC   *tmpNextHopInfoPtr;

    /* if initialization has not been done for the requested protocol stack -
    return error */
    if (PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_STATUS_GET_MAC(lpmDbPtr->protocolBitmap) == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    /* the search can be done on one shadow */
    shadowPtr = &lpmDbPtr->shadowArray[0];
    if (shadowPtr->vrRootBucketArray[vrId].valid == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }
    retVal = prvCpssDxChLpmRamUcEntryGetSip7(vrId,fcoeAddrPtr->fcid,prefixLenPtr,
                                         &tmpNextHopInfoPtr,
                                         PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E,
                                         shadowPtr);
    if (retVal == GT_OK)
    {
        cpssOsMemCpy(nextHopInfoPtr,tmpNextHopInfoPtr,
                     sizeof(PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC));
    }

    return retVal;
}

/**
* @internal prvCpssDxChLpmRamFcoePrefixGetSip7 function
* @endinternal
*
* @brief   Function Relevant mode : High Level API modes
*         This function gets a given FC_ID address, and returns the next
*         hop pointer information bounded to the longest prefix match.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                  Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] lpmDbPtr                 - The LPM DB
* @param[in] vrId                     - The virtual router id.
* @param[in] fcoeAddr                 - The FC_ID to look for.
*
* @param[out] prefixLenPtr             - Points to the number of bits that are actual valid
*                                      in the longest match
* @param[out] nextHopInfoPtr           - The next hop pointer bound to the longest match
*
* @retval GT_OK                    - if the required entry was found, or
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_NOT_FOUND             - if the given prefix was not found.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmRamFcoePrefixGetSip7
(
    IN  PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC        *lpmDbPtr,
    IN  GT_U32                                      vrId,
    IN  GT_FCID                                     fcoeAddr,
    OUT GT_U32                                      *prefixLenPtr,
    OUT PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC   *nextHopInfoPtr
)
{
    GT_STATUS                           retVal = GT_OK;
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC    *shadowPtr;

    /* if initialization has not been done for the requested protocol stack -
       return error */
    if (PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_STATUS_GET_MAC(lpmDbPtr->protocolBitmap) == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    /* the search can be done on one shadow */
    shadowPtr = &lpmDbPtr->shadowArray[0];
    if (shadowPtr->vrRootBucketArray[vrId].valid == GT_FALSE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_FOUND, LOG_ERROR_NO_MSG);
    }
    retVal = prvCpssDxChLpmRamUcEntryLpmSearchSip7(vrId,fcoeAddr.fcid,prefixLenPtr,
                                               nextHopInfoPtr,
                                               PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E,
                                               shadowPtr);
    return (retVal);
}

/**
* @internal prvCpssDxChLpmRamMemFreeListMngSip7 function
* @endinternal
*
* @brief   This function is used to collect all Pp Narrow Sram memory free
*         operations inorder to be freed at the end of these operations.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                  Falcon; AC5P; AC5X; Harrier; Ironman.

* @param[in] ppMemAddr                - the address in the HW (the device memory pool) to record.
* @param[in] operation                - the  (see
*                                      PRV_CPSS_DXCH_LPM_RAM_MEM_LIST_RESET_OP_E,
*                                      PRV_CPSS_DXCH_LPM_RAM_MEM_LIST_ADD_MEM_OP_E,
*                                      PRV_CPSS_DXCH_LPM_RAM_MEM_LIST_FREE_MEM_OP_E
* @param[in] memBlockListPtrPtr       - (pointer to) the memory block list to act upon.
* @param[in] shadowPtr                - (pointer to) the shadow relevant for the devices asked to act on.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmRamMemFreeListMngSip7
(
    IN  GT_UINTPTR                                  ppMemAddr,
    IN  PRV_CPSS_DXCH_LPM_RAM_MEM_LIST_OP_ENT       operation,
    IN  PRV_CPSS_DXCH_LPM_RAM_MEM_LIST_ENTRY_STC    **memBlockListPtrPtr,
    IN  PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC            *shadowPtr
)
{
    PRV_CPSS_DXCH_LPM_RAM_MEM_LIST_ENTRY_STC  *ppMemEntry,*tmpPtr; /* Pp memory list entry.    */
    GT_STATUS                   retVal = GT_OK;     /* Functions return value.      */
    GT_U32                      blockIndex=0;/* calculated according to the ppMemAddr*/
    GT_U32                      sizeOfMemoryBlockInlines=0;/* calculated according to the memory handle */
    GT_BOOL                     oldUpdateDec=GT_FALSE;/* for reconstruct */
    GT_U32                      oldNumOfDecUpdates=0;/* for reconstruct */
    switch(operation)
    {
    case (PRV_CPSS_DXCH_LPM_RAM_MEM_LIST_RESET_OP_E):
        /* make sure it an empty list */
        if (*memBlockListPtrPtr != NULL)
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);

        break;

    case (PRV_CPSS_DXCH_LPM_RAM_MEM_LIST_ADD_MEM_OP_E):
        ppMemEntry = cpssOsLpmMalloc(sizeof(PRV_CPSS_DXCH_LPM_RAM_MEM_LIST_ENTRY_STC));
        if (ppMemEntry == NULL)
        {
            retVal = GT_OUT_OF_CPU_MEM;
        }
        else
        {
            ppMemEntry->memAddr  = ppMemAddr;
            ppMemEntry->next = *memBlockListPtrPtr;
            *memBlockListPtrPtr = ppMemEntry;

        }
        break;

    case (PRV_CPSS_DXCH_LPM_RAM_MEM_LIST_FREE_MEM_OP_E):
        ppMemEntry = *memBlockListPtrPtr;
        while (ppMemEntry != NULL)
        {
            /* create a pending list of blocks that will be used to
               update protocolCountersPerBlockArr */
            if (ppMemEntry->memAddr==0)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Error: unexpected ppMemEntry->memAddr=0\n");
            }
            /* the block index updated out of 20 blocks*/
            blockIndex = PRV_CPSS_DXCH_LPM_RAM_GET_SIP7_LPM_OFFSET_FROM_DMM_MAC(ppMemEntry->memAddr)/(shadowPtr->lpmRamTotalBlocksSizeIncludingGap);
            retVal = prvCpssDxChLpmRamSip7CalcBankNumberIndex(shadowPtr,&blockIndex);
            if (retVal != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "ERROR:illegal blockIndex - fall in holes \n");
            }
            sizeOfMemoryBlockInlines = PRV_CPSS_DXCH_LPM_RAM_GET_SIP7_LPM_SIZE_FROM_DMM_MAC(ppMemEntry->memAddr);

            /* keep values incase reconstruct is needed */
            oldUpdateDec = shadowPtr->pendingBlockToUpdateArr[blockIndex].updateDec;
            oldNumOfDecUpdates = shadowPtr->pendingBlockToUpdateArr[blockIndex].numOfDecUpdates;

            /* set pending flag for future need */
            shadowPtr->pendingBlockToUpdateArr[blockIndex].updateDec=GT_TRUE;
            shadowPtr->pendingBlockToUpdateArr[blockIndex].numOfDecUpdates += sizeOfMemoryBlockInlines;

            if (prvCpssDmmFreeSip7(ppMemEntry->memAddr) == 0)
            {
                /* reconstruct */
                shadowPtr->pendingBlockToUpdateArr[blockIndex].updateDec = oldUpdateDec;
                shadowPtr->pendingBlockToUpdateArr[blockIndex].numOfDecUpdates = oldNumOfDecUpdates;

                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
            }

            tmpPtr = ppMemEntry;
            ppMemEntry = ppMemEntry->next;

            cpssOsLpmFree(tmpPtr);
        }
        *memBlockListPtrPtr = NULL;
        break;
    }

    return retVal;
}

/**
* @internal prvCpssDxChLpmRamUpdateVrTableFuncWrapperSip7 function
* @endinternal
*
* @brief   This function is a wrapper to PRV_CPSS_DXCH_LPM_RAM_UPDATE_VRF_TABLE_FUNC_PTR
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                  Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] data                     - the parmeters for PRV_CPSS_DXCH_LPM_RAM_UPDATE_VRF_TABLE_FUNC_PTR
*                                       GT_OK on success, or
*
* @retval GT_OUT_OF_CPU_MEM        - if failed to allocate CPU memory, or
* @retval GT_OUT_OF_PP_MEM         - if failed to allocate PP memory.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmRamUpdateVrTableFuncWrapperSip7
(
    IN  GT_PTR                  data
)
{
    PRV_CPSS_DXCH_LPM_RAM_UPDATE_VRF_TABLE_FUNC_PARAMS_STC *params;

    params = (PRV_CPSS_DXCH_LPM_RAM_UPDATE_VRF_TABLE_FUNC_PARAMS_STC*)data;

    return prvCpssDxChLpmRamMngVrfEntryUpdateSip7(params->vrId, params->protocol,
                                              params->shadowPtr);
}

/**
* @internal prvCpssDxChLpmRouteEntryPointerCheckSip7 function
* @endinternal
*
* @brief   Check validity of values of route entry pointer
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                  Falcon; AC5P; AC5X; Harrier; Ironman.
* @param[in] routeEntryPointerPtr     - route entry pointer to check
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
static GT_STATUS prvCpssDxChLpmRouteEntryPointerCheckSip7
(
    IN  PRV_CPSS_DXCH_LPM_ROUTE_ENTRY_POINTER_STC  *routeEntryPointerPtr
)
{
    CPSS_NULL_PTR_CHECK_MAC(routeEntryPointerPtr);

    if ((routeEntryPointerPtr->routeEntryMethod != PRV_CPSS_DXCH_LPM_ENTRY_TYPE_ECMP_E) &&
        (routeEntryPointerPtr->routeEntryMethod != PRV_CPSS_DXCH_LPM_ENTRY_TYPE_QOS_E) &&
        (routeEntryPointerPtr->routeEntryMethod != PRV_CPSS_DXCH_LPM_ENTRY_TYPE_MULTIPATH_E) &&
        (routeEntryPointerPtr->routeEntryMethod != PRV_CPSS_DXCH_LPM_ENTRY_TYPE_REGULAR_E))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "routeEntryMethod: not supported type ");
    }

    if ((routeEntryPointerPtr->routeEntryMethod == PRV_CPSS_DXCH_LPM_ENTRY_TYPE_ECMP_E) ||
        (routeEntryPointerPtr->routeEntryMethod == PRV_CPSS_DXCH_LPM_ENTRY_TYPE_QOS_E))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "routeEntryMethod : Sip7 not supports types : 'ECMP' , 'QOS' ");
    }

    /* validate the ipv6 MC group scope level */
    switch (routeEntryPointerPtr->ipv6McGroupScopeLevel)
    {
        case CPSS_IPV6_PREFIX_SCOPE_LINK_LOCAL_E:
        case CPSS_IPV6_PREFIX_SCOPE_SITE_LOCAL_E:
        case CPSS_IPV6_PREFIX_SCOPE_UNIQUE_LOCAL_E:
        case CPSS_IPV6_PREFIX_SCOPE_GLOBAL_E:
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "groupScopeLevel : not supported type ");
    }

    switch (routeEntryPointerPtr->priority)
    {
        case PRV_CPSS_DXCH_LPM_LEAF_ENTRY_PRIORITY_FDB_E:
        case PRV_CPSS_DXCH_LPM_LEAF_ENTRY_PRIORITY_LPM_E:
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "priority : not supported type ");
    }

    /* note that since LPM DB can exist without any devices added to it, there
       is no way to know how many route entries or ECMP/QoS entries are
       available in specific device; therefore neither the route entries base
       address nor the size of the ECMP/QoS block value can't be checked */

    return GT_OK;
}

/**
* @internal prvCpssDxChIpLpmRamIpv4UcPrefixActivityStatusGetSip7 function
* @endinternal
*
* @brief   Get the activity bit status for specific IPv6 UC prefix for a specific LPM DB
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                  Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] vrId                     - The virtual router Id.(APPLICABLE RANGES: 0..4095)
* @param[in] ipPtr                    - Pointer to the ip address to look for.
* @param[in] prefixLen                - ipAddr prefix length.
* @param[in] clearActivity            - Indicates to clear activity status.
* @param[in] shadowPtr                - the shadow relevant for the devices asked to act on.
*
* @param[out] activityStatusPtr        - (pointer to) the activity status:
*                                      GT_TRUE  - Indicates that the entry is active
*                                      and should not be aged out.
*                                      GT_FALSE - Indicates that the entry is not active
*
* @retval GT_OK                    - on success.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_BAD_PTR               - if one of the parameters is NULL pointer.
* @retval GT_NOT_FOUND             - if entry not found.
* @retval GT_NOT_SUPPORTED         - on not supported request.
* @retval GT_FAIL                  - otherwise.
*
* @note none.
*
*/
GT_STATUS prvCpssDxChIpLpmRamIpv4UcPrefixActivityStatusGetSip7
(
    IN  GT_U32                              vrId,
    IN  GT_U8                               *ipPtr,
    IN  GT_U32                              prefixLen,
    IN  GT_BOOL                             clearActivity,
    IN  PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC    *shadowPtr,
    OUT GT_BOOL                             *activityStatusPtr
)
{
    if(shadowPtr->isProtocolInitialized[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E] != GT_TRUE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    return prvCpssDxChIpLpmSip7RamUcPrefixActivityStatusGet(
                vrId,
                CPSS_IP_PROTOCOL_IPV4_E,
                ipPtr,
                prefixLen,
                clearActivity,
                shadowPtr,
                activityStatusPtr);
}

/**
* @internal prvCpssDxChIpLpmRamIpv6UcPrefixActivityStatusGetSip7 function
* @endinternal
*
* @brief   Get the activity bit status for specific IPv6 UC prefix for a specific LPM DB
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                  Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] vrId                     - The virtual router Id.(APPLICABLE RANGES: 0..4095)
* @param[in] ipPtr                    - Pointer to the ip address to look for.
* @param[in] prefixLen                - ipAddr prefix length.
* @param[in] clearActivity            - Indicates to clear activity status.
* @param[in] shadowPtr                - the shadow relevant for the devices asked to act on.
*
* @param[out] activityStatusPtr        - (pointer to) the activity status:
*                                      GT_TRUE  - Indicates that the entry is active
*                                      and should not be aged out.
*                                      GT_FALSE - Indicates that the entry is not active
*
* @retval GT_OK                    - on success.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_BAD_PTR               - if one of the parameters is NULL pointer.
* @retval GT_NOT_FOUND             - if entry not found.
* @retval GT_NOT_SUPPORTED         - on not supported request.
* @retval GT_FAIL                  - otherwise.
*
* @note none.
*
*/
GT_STATUS prvCpssDxChIpLpmRamIpv6UcPrefixActivityStatusGetSip7
(
    IN  GT_U32                              vrId,
    IN  GT_U8                               *ipPtr,
    IN  GT_U32                              prefixLen,
    IN  GT_BOOL                             clearActivity,
    IN  PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC    *shadowPtr,
    OUT GT_BOOL                             *activityStatusPtr
)
{
    if(shadowPtr->isProtocolInitialized[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E] != GT_TRUE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    return prvCpssDxChIpLpmSip7RamUcPrefixActivityStatusGet(
                vrId,
                CPSS_IP_PROTOCOL_IPV6_E,
                ipPtr,
                prefixLen,
                clearActivity,
                shadowPtr,
                activityStatusPtr);
}

/**
* @internal prvCpssDxChIpLpmRamIpv4McEntryActivityStatusGetSip7 function
* @endinternal
*
* @brief   Get the activity bit status for specific IPv4 MC entry for a specific LPM DB
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                  Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] vrId                     - The virtual router Id.(APPLICABLE RANGES: 0..4095)
* @param[in] ipGroupPtr               - Pointer to the ip Group address to get the entry for.
* @param[in] ipGroupPrefixLen         - ipGroup prefix length.
* @param[in] ipSrcPtr                 - Pointer to the ip Source address to get the entry for.
* @param[in] ipSrcPrefixLen           - ipSrc prefix length.
* @param[in] clearActivity            - Indicates to clear activity status.
* @param[in] shadowPtr                - the shadow relevant for the devices asked to act on.
*
* @param[out] activityStatusPtr        - (pointer to) the activity status:
*                                      GT_TRUE  - Indicates that the entry is active
*                                      and should not be aged out.
*                                      GT_FALSE - Indicates that the entry is not active
*
* @retval GT_OK                    - on success.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_BAD_PTR               - if one of the parameters is NULL pointer.
* @retval GT_NOT_FOUND             - if entry not found.
* @retval GT_NOT_SUPPORTED         - on not supported request.
* @retval GT_FAIL                  - otherwise.
*
* @note none.
*
*/
GT_STATUS prvCpssDxChIpLpmRamIpv4McEntryActivityStatusGetSip7
(
    IN  GT_U32                              vrId,
    IN  GT_U8                               *ipGroupPtr,
    IN  GT_U32                              ipGroupPrefixLen,
    IN  GT_U8                               *ipSrcPtr,
    IN  GT_U32                              ipSrcPrefixLen,
    IN  GT_BOOL                             clearActivity,
    IN  PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC    *shadowPtr,
    OUT GT_BOOL                             *activityStatusPtr
)
{
    if(shadowPtr->isProtocolInitialized[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E] != GT_TRUE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    return prvCpssDxChIpLpmSip7RamMcEntryActivityStatusGet(
                vrId,
                CPSS_IP_PROTOCOL_IPV4_E,
                ipGroupPtr,
                ipGroupPrefixLen,
                ipSrcPtr,
                ipSrcPrefixLen,
                clearActivity,
                shadowPtr,
                activityStatusPtr);
}

/**
* @internal prvCpssDxChIpLpmRamIpv6McEntryActivityStatusGetSip7 function
* @endinternal
*
* @brief   Get the activity bit status for specific IPv6 MC entry for a specific LPM DB
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                  Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] vrId                     - The virtual router Id.(APPLICABLE RANGES: 0..4095)
* @param[in] ipGroupPtr               - Pointer to the ip Group address to get the entry for.
* @param[in] ipGroupPrefixLen         - ipGroup prefix length.
* @param[in] ipSrcPtr                 - Pointer to the ip Source address to get the entry for.
* @param[in] ipSrcPrefixLen           - ipSrc prefix length.
* @param[in] clearActivity            - Indicates to clear activity status.
* @param[in] shadowPtr                - the shadow relevant for the devices asked to act on.
*
* @param[out] activityStatusPtr        - (pointer to) the activity status:
*                                      GT_TRUE  - Indicates that the entry is active
*                                      and should not be aged out.
*                                      GT_FALSE - Indicates that the entry is not active
*
* @retval GT_OK                    - on success.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device.
* @retval GT_BAD_PTR               - if one of the parameters is NULL pointer.
* @retval GT_NOT_FOUND             - if entry not found.
* @retval GT_NOT_SUPPORTED         - on not supported request.
* @retval GT_FAIL                  - otherwise.
*
* @note none.
*
*/
GT_STATUS prvCpssDxChIpLpmRamIpv6McEntryActivityStatusGetSip7
(
    IN  GT_U32                              vrId,
    IN  GT_U8                               *ipGroupPtr,
    IN  GT_U32                              ipGroupPrefixLen,
    IN  GT_U8                               *ipSrcPtr,
    IN  GT_U32                              ipSrcPrefixLen,
    IN  GT_BOOL                             clearActivity,
    IN  PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC    *shadowPtr,
    OUT GT_BOOL                             *activityStatusPtr
)
{
    if(shadowPtr->isProtocolInitialized[PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E] != GT_TRUE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_INITIALIZED, LOG_ERROR_NO_MSG);
    }

    return prvCpssDxChIpLpmSip7RamMcEntryActivityStatusGet(
                vrId,
                CPSS_IP_PROTOCOL_IPV6_E,
                ipGroupPtr,
                ipGroupPrefixLen,
                ipSrcPtr,
                ipSrcPrefixLen,
                clearActivity,
                shadowPtr,
                activityStatusPtr);
}

/**
* @internal prvCpssDxChLpmRamSyncSwHwForHaCheckPerVrIdAndProtocol_SyncRangesSip7 function
* @endinternal
*
* @brief   Shadow and HW synchronization of bucket's ranges
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                  Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - The device number
* @param[in] shadowPtr                - pointer to shadow structure
* @param[in] vrId                     - the virtual router id
* @param[in] bucketPtr                - pointer to the bucket
*                                      maskForRangeInTrieArray   - array of the ranges masks as found in the
*                                      trie. The array index represents the range
*                                      start address
*                                      validRangeInTrieArray     - array to indicate if a range was found in
*                                      the trie. The array index represents the
*                                      range start address.
* @param[in]nodeTotalChildTypesArr[PRV_CPSS_DXCH_LPM_RAM_FALCON_MAX_SIZE_OF_BUCKET_IN_LPM_LINES_CNS][CPSS_DXCH_LPM_MAX_CHILD_TYPE_CNS];
*                                   6 elements in a regular node
*                                   3 types of child that can be
*                                   for each range
*                                   (leaf,regular,compress)
* @param[in] level                    - the  in the tree (first  is 0)
* @param[in] numOfMaxAllowedLevels    - the maximal number of levels that is allowed
*                                      for the relevant protocol and prefix type
* @param[in] prefixType               - unicast or multicast tree
* @param[in] protocol                 - protocol
*
* @retval GT_FAIL                  - on failure
* @retval GT_OK                    - on success
*/
GT_STATUS prvCpssDxChLpmRamSyncSwHwForHaCheckPerVrIdAndProtocol_SyncRangesSip7
(
    IN  GT_U8                                     devNum,
    IN  PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC          *shadowPtr,
    IN  GT_U32                                    vrId,
    IN  PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC   *bucketPtr,
    IN  GT_U32                                    nodeTotalChildTypesArr[PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS][PRV_CPSS_DXCH_LPM_MAX_CHILD_TYPE_CNS],
    IN  GT_U8                                     level,
    IN  GT_U8                                     numOfMaxAllowedLevels,
    IN  CPSS_UNICAST_MULTICAST_ENT                prefixType,
    IN  PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT      protocol
)
{
    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC  *rangePtr;
    GT_U8                                   prefixTypeFirstRange, prefixTypeLastRange, prefixMcTypeFirstRange=0;
    GT_U8                                   prefixTypeSecondRange=0, prefixTypeSecondLastRange=0;
    GT_U32                                  tmpPrefixTypeRange;
    GT_STATUS                               status, retVal = GT_OK;
    GT_U32    hwBucketDataArr[PRV_CPSS_DXCH_LPM_RAM_FALCON_MAX_SIZE_OF_BUCKET_IN_LPM_LINES_CNS*PRV_CPSS_DXCH_LPM_RAM_SIP7_SIZE_OF_LPM_ENTRY_IN_WORDS_CNS];
    GT_U32    gonNodeSize=0;/* the node size can be 6 for regular or 1 for compress */
    GT_U32    gonPointerIndex=0;/* can be 0 for compress or 0-5 for regular, this is the pointer to the GON */
    GT_U32    tempGonPointerIndex=0;/* can be 0 for compress or 0-5 for regular, this is the pointer to the GON */
    GT_U32    gonNodeAddr=0; /* Node address to read the HW data from */
    GT_U32    hwNodeOffset = 0;/* base node offset address to read the HW data from */
    GT_U32    rangeType;/* CPSS_DXCH_LPM_CHILD_TYPE_ENT */
    GT_U32    totalChildTypesUntilNow=0;
    GT_U32    totalCompressedChilds=0;
    GT_U32    totalRegularChilds=0;

    cpssOsMemSet(&hwBucketDataArr[0], 0, sizeof(hwBucketDataArr));

    if (level == 0)
    {
        if (prefixType == CPSS_UNICAST_E)
        {
            prefixTypeFirstRange = 0;
            tmpPrefixTypeRange = (protocol == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E) ?
                (PRV_CPSS_DXCH_LPM_RAM_START_OF_IPV4_MC_ADDRESS_SPACE_CNS - 1) :
                (PRV_CPSS_DXCH_LPM_RAM_START_OF_IPV6_MC_ADDRESS_SPACE_CNS - 1);
            prefixTypeLastRange = (GT_U8)tmpPrefixTypeRange;

            if (protocol == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E)
            {
                prefixTypeSecondRange = PRV_CPSS_DXCH_LPM_RAM_START_OF_IPV4_RESERVED_SPACE_ADDRESS_SPACE_CNS;
                prefixTypeSecondLastRange = PRV_CPSS_DXCH_LPM_RAM_END_OF_IPV4_RESERVED_SPACE_ADDRESS_SPACE_CNS;
            }
        }
        else /* CPSS_MULTICAST_E */
        {
            tmpPrefixTypeRange = (protocol == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E) ?
                PRV_CPSS_DXCH_LPM_RAM_START_OF_IPV4_MC_ADDRESS_SPACE_CNS :
                PRV_CPSS_DXCH_LPM_RAM_START_OF_IPV6_MC_ADDRESS_SPACE_CNS;
            prefixMcTypeFirstRange = (GT_U8)tmpPrefixTypeRange;

            /* Need to consider ranges of unicast also while calculating node offset in the GON */
            prefixTypeFirstRange = (bucketPtr->bucketType == CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E) ? 220 : 0;

            tmpPrefixTypeRange = (protocol == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E) ?
                PRV_CPSS_DXCH_LPM_RAM_END_OF_IPV4_MC_ADDRESS_SPACE_CNS :
                PRV_CPSS_DXCH_LPM_RAM_END_OF_IPV6_MC_ADDRESS_SPACE_CNS;
            prefixTypeLastRange = (GT_U8)tmpPrefixTypeRange;

            prefixTypeSecondLastRange = prefixTypeLastRange;
        }
    }
    else
    {
        prefixTypeFirstRange = 0;
        prefixTypeLastRange = 255;
        prefixTypeSecondRange = 0;
        prefixTypeSecondLastRange = 255;
    }

    /* Check next buckets */
    rangePtr = bucketPtr->rangeList;
    while (rangePtr)
    {
        /* Skip unicast ranges for multicast sync or multicast ranges for
           unicast sync */
        if (protocol == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E)
        {
            if ((rangePtr->startAddr < prefixTypeFirstRange) ||
                ((rangePtr->startAddr > prefixTypeLastRange) && (rangePtr->startAddr<prefixTypeSecondRange))||
                 (rangePtr->startAddr > prefixTypeSecondLastRange))
            {
                rangePtr = rangePtr->next;
                continue;
            }
        }
        else
        {
            if ((rangePtr->startAddr < prefixTypeFirstRange) ||
                (rangePtr->startAddr > prefixTypeLastRange))
            {
                rangePtr = rangePtr->next;
                continue;
            }
        }

        /* do not enter the if incase of pointer to NH or MC with type (G,*) */
        if ((rangePtr->pointerType != CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E) &&
            (rangePtr->pointerType != CPSS_DXCH_LPM_MULTIPATH_ENTRY_PTR_TYPE_E) &&
            (!(((GT_U32)rangePtr->pointerType==(GT_U32)PRV_CPSS_DXCH_LPM_RAM_TRIE_PTR_TYPE_E) &&
               ((rangePtr->lowerLpmPtr.nextBucket->isImplicitGroupDefault==GT_FALSE)))))/* when isImplicitGroupDefault==GT_FALSE we are dealing with (G,*) */
        {
            if (bucketPtr->bucketType==CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E)
            {
                /* find the correct GON pointer out of 6 possible pointers */
                gonPointerIndex = rangePtr->startAddr/NUMBER_OF_RANGES_IN_SUBNODE;
                /* in case we move from one gone index to the next one,
                   we need to reset the counter of the childs */
                if (gonPointerIndex!=tempGonPointerIndex)
                {
                    totalCompressedChilds = 0;
                    totalRegularChilds    = 0;
                    tempGonPointerIndex = gonPointerIndex;

                }
            }
            else
            {
                gonPointerIndex = 0;
            }

            if (rangePtr->pointerType==CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E)
            {
                rangeType = PRV_CPSS_DXCH_LPM_CHILD_REGULAR_TYPE_E;
                totalChildTypesUntilNow = totalRegularChilds;
                totalRegularChilds++;
            }
            else
            {
                rangeType = PRV_CPSS_DXCH_LPM_CHILD_COMPRESSED_TYPE_E;
                totalChildTypesUntilNow = totalCompressedChilds;
                totalCompressedChilds++;
            }

            if ((prefixType != CPSS_UNICAST_E) && (rangePtr->startAddr < prefixMcTypeFirstRange))
            {
                rangePtr = rangePtr->next;
                continue;
            }

            /* for MC case , need to calculate the address and size of a single GON
               according to the parameters we have in the Leaf */
            if((GT_U32)rangePtr->pointerType==(GT_U32)PRV_CPSS_DXCH_LPM_RAM_TRIE_PTR_TYPE_E)
            {
                /* root of SRC tree keep the hwNodeOffset in the hwBucketOffsetHandle
                   (same as root of MC GRP, and root of UC)*/
                /* at this point rangePtr->lowerLpmPtr.nextBucket->hwBucketOffsetHandle was already updated from the HW and can be used */
                if (rangePtr->lowerLpmPtr.nextBucket->hwBucketOffsetHandle==0)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Error: unexpected rangePtr->lowerLpmPtr.nextBucket->hwBucketOffsetHandle=0\n");
                }

                hwNodeOffset = PRV_CPSS_DXCH_LPM_RAM_GET_SIP7_LPM_OFFSET_FROM_DMM_MAC(rangePtr->lowerLpmPtr.nextBucket->hwBucketOffsetHandle);
                gonNodeAddr = hwNodeOffset;
                if(rangePtr->lowerLpmPtr.nextBucket->bucketType==CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E)
                {
                    gonNodeSize = 6;
                }
                else
                {
                    if((rangePtr->lowerLpmPtr.nextBucket->bucketType==CPSS_DXCH_LPM_COMPRESSED_NODE_PTR_TYPE_E)||
                       (rangePtr->lowerLpmPtr.nextBucket->bucketType==CPSS_DXCH_LPM_COMPRESSED_UP_TO_7_RANGES_1_LEAF_NODE_PTR_TYPE_E)||
                       (rangePtr->lowerLpmPtr.nextBucket->bucketType==CPSS_DXCH_LPM_COMPRESSED_UP_TO_5_RANGES_2_LEAVES_NODE_PTR_TYPE_E)||
                       (rangePtr->lowerLpmPtr.nextBucket->bucketType==CPSS_DXCH_LPM_COMPRESSED_3_RANGES_3_LEAVES_NODE_PTR_TYPE_E))

                    {
                        gonNodeSize = 1;
                    }
                    else
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
                    }
               }
                /* we check that that the address we calculated according to the HW is the same as the nodeMemAddr we keep in the shadow */
                if (gonNodeAddr!=rangePtr->lowerLpmPtr.nextBucket->nodeMemAddr)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "gonNodeAddr different from nodeMemAddr ");
                }
            }
            else
            {
              /* calculate the address and the size of a single node from the GON
                (6 lines for regular all the rest 1 line) according to the
                 parameters we got from getNumOfRangesFromHWSip7 */

                /* when we get here the value in bucketPtr->hwGroupOffsetHandle[gonPointerIndex] was already sync with HW
                   so we can use it*/
                if (bucketPtr->hwGroupOffsetHandle[gonPointerIndex]==0)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Error: unexpected rbucketPtr->hwGroupOffsetHandle[gonPointerIndex]=0\n");
                }

                hwNodeOffset = PRV_CPSS_DXCH_LPM_RAM_GET_SIP7_LPM_OFFSET_FROM_DMM_MAC(bucketPtr->hwGroupOffsetHandle[gonPointerIndex]);
                status = getFromTheGonOneNodeAddrAndSizeSip7(hwNodeOffset,
                                                     nodeTotalChildTypesArr[gonPointerIndex],
                                                     rangeType,
                                                     totalChildTypesUntilNow,
                                                     &gonNodeAddr,
                                                     &gonNodeSize);
                if (status != GT_OK)
                {
                    return status;
                }

                /* set the nodeMemAddr in SW with the HW value */
                rangePtr->lowerLpmPtr.nextBucket->nodeMemAddr = gonNodeAddr;

            }

            /* read the HW data for the specific range */
            status = prvCpssDxChLpmRamIndirectReadTableMultiEntrySip7(devNum,
                                                 CPSS_DXCH_SIP5_TABLE_LPM_MEM_E,
                                                 gonNodeAddr,
                                                 gonNodeSize,
                                                 &hwBucketDataArr[0]);
            if (status != GT_OK)
            {
                return status;
            }

            /* keep values in case reconstruct is needed */
            cpssOsMemCpy(shadowPtr->tempLpmRamOctetsToBlockMappingUsedForReconstractPtr,shadowPtr->lpmRamOctetsToBlockMappingPtr,
                 sizeof(PRV_CPSS_DXCH_LPM_RAM_OCTETS_TO_BLOCK_MAPPING_STC)*shadowPtr->numOfLpmMemories);

            status = prvCpssDxChLpmRamSyncSwHwForHaCheckPerVrIdAndProtocol_SyncBucketSip7(devNum,
                                                                 shadowPtr,
                                                                 vrId,
                                                                 rangePtr->lowerLpmPtr.nextBucket,
                                                                 gonNodeAddr,
                                                                 &hwBucketDataArr[0],
                                                                 (GT_U8)(level + 1),
                                                                 numOfMaxAllowedLevels,
                                                                 prefixType,
                                                                 protocol,
                                                                 GT_FALSE/*not the root bucket*/);
            if (status != GT_OK)
            {
                /* free the allocated/bound RAM memory */
                retVal = prvCpssDxChLpmRamMngAllocatedAndBoundMemFreeSip7(shadowPtr,protocol);
                if (retVal!=GT_OK)
                {
                    return retVal;
                }
                /* in case of fail we will need to recondtruct to the status we had before the call to prvCpssDxChLpmRamMngAllocAvailableMemCheck */
                cpssOsMemCpy(shadowPtr->lpmRamOctetsToBlockMappingPtr,shadowPtr->tempLpmRamOctetsToBlockMappingUsedForReconstractPtr,
                             sizeof(PRV_CPSS_DXCH_LPM_RAM_OCTETS_TO_BLOCK_MAPPING_STC)*shadowPtr->numOfLpmMemories);

               return status;
            }
            else
            {
                /* the data was written successfully to HW, we can reset the information regarding the new memoryPool allocations done.
                   next call to ADD will set this array with new values of allocated/bound blocks */
                cpssOsMemSet(shadowPtr->allNewNextMemInfoAllocatedPerOctetArrayPtr,0,sizeof(shadowPtr->allNewNextMemInfoAllocatedPerOctetArrayPtr));

                cpssOsMemSet(shadowPtr->tempLpmRamOctetsToBlockMappingUsedForReconstractPtr,0,sizeof(shadowPtr->tempLpmRamOctetsToBlockMappingUsedForReconstractPtr));
            }
        }
        else
        {
            /* in case of NextHop pointers no need to do anything since
               the SW and HW NextHop synchronization was done in
               prvCpssDxChLpmRamSyncSwHwForHaCheckPerVrIdAndProtocol_SyncBucketSip7 */
        }
        rangePtr = rangePtr->next;
    }

    return retVal;
}

/**
* @internal prvCpssDxChLpmRamSyncSwHwForHaCheckPerVrIdAndProtocol_SyncBucketSip7 function
* @endinternal
*
* @brief   Shadow and HW synchronization of a bucket
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                  Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - The device number
* @param[in] shadowPtr                - pointer to shadow structure
* @param[in] vrId                     - the virtual router id
* @param[in] bucketPtr                - pointer to the bucket
* @param[in] hwBucketGonAddr          - node address of the read HW
* @param[in] hwBucketDataArr          - array holding hw data.
*                                       in case of root this is
*                                       a null pointer
* @param[in] level                    - the  level in the tree
*                                       (first is 0)
* @param[in] numOfMaxAllowedLevels    - the maximal allowed number of levels
*                                      for the relevant protocol and prefix type
* @param[in] prefixType               - unicast or multicast tree
* @param[in] protocol                 - protocol
* @param[in] isRootBucket             - GT_TRUE:the bucketPtr is the root bucket
*                                      GT_FALSE:the bucketPtr is not the root bucket
*
* @retval GT_FAIL                  - on failure
* @retval GT_OK                    - on success
*/
GT_STATUS prvCpssDxChLpmRamSyncSwHwForHaCheckPerVrIdAndProtocol_SyncBucketSip7
(
    IN  GT_U8                                     devNum,
    IN  PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC          *shadowPtr,
    IN  GT_U32                                    vrId,
    IN  PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC   *bucketPtr,
    IN  GT_U32                                    hwBucketGonAddr,
    IN  GT_U32                                    hwBucketDataArr[],
    IN  GT_U8                                     level,
    IN  GT_U8                                     numOfMaxAllowedLevels,
    IN  CPSS_UNICAST_MULTICAST_ENT                prefixType,
    IN  PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT      protocol,
    IN  GT_BOOL                                   isRootBucket
)
{
    GT_STATUS status = GT_OK;
    GT_U32    nodeSize;
    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC *tempRangePtr=NULL;/* Current range pointer.   */
    GT_U32    hwNumOfRanges;            /* the number of ranges in the HW */
    GT_U32    *hwRangesArr;             /* the ranges values retrieved from the HW */
    GT_U32    *hwRangesTypesArr;        /* for each range keep its type (regular,compress,leaf) */
    GT_U32    *hwRangesBitLineArr;      /* for each range keep its bit vector line */
    GT_U32    *hwRangesTypeIndexArr;    /* what is the number of this range type
                                           from total number of lines with the same type -
                                           where it is located in the GON */
    GT_U32    nodeChildAddressesArr[PRV_CPSS_DXCH_LPM_RAM_FALCON_MAX_SIZE_OF_BUCKET_IN_LPM_LINES_CNS];/* the child_pointer located at the beginning of each line
                                                                                            for regular node we have 6 pointers, for compress node one pointer */
    GT_U32    nodeTotalChildTypesArr[PRV_CPSS_DXCH_LPM_RAM_FALCON_MAX_SIZE_OF_BUCKET_IN_LPM_LINES_CNS][PRV_CPSS_DXCH_LPM_MAX_CHILD_TYPE_CNS];/* 6 elements in a regular node
                                                                                                3 types of child that can be for each range (leaf,regular,compress) */

    GT_U32    nodeTotalLinesPerTypeArr[PRV_CPSS_DXCH_LPM_RAM_FALCON_MAX_SIZE_OF_BUCKET_IN_LPM_LINES_CNS][PRV_CPSS_DXCH_LPM_MAX_CHILD_TYPE_CNS];   /* sum the number of lines in the GON according to the Type */
    GT_U32    nodeTotalBucketPerTypesArr[PRV_CPSS_DXCH_LPM_RAM_FALCON_MAX_SIZE_OF_BUCKET_IN_LPM_LINES_CNS][PRV_CPSS_DXCH_LPM_MAX_CHILD_TYPE_CNS]; /* sum the number of buckets (copmress, regular or leaf) in the GON according to the Type */

    GT_U32    bankIndexsOfTheGonsArray[PRV_CPSS_DXCH_LPM_RAM_FALCON_MAX_SIZE_OF_BUCKET_IN_LPM_LINES_CNS];  /* the indexs of the banks were the GON is located */

    GT_U32    hwPointerType;   /* The type of the next entry this entry points to. can be Leaf/Trigger */
    GT_U32    i=0;
    GT_U32    nonRealRangeIndex=0;
    GT_U32                              hwNodeOffset[6];/* HW node address */
    GT_U32                              hwNodeSize=0; /* size of the node in lines
                                                         regular node have 6 lines, all the rest hold 1 line*/
    PRV_CPSS_DXCH_LPM_CHILD_TYPE_ENT    hwNodeType = 0;/* the bucketType retrieved from HW */
    GT_U32                              hwRootNodeAddr=0;/* HW Root node address */
    PRV_CPSS_DXCH_LPM_CHILD_TYPE_ENT    hwRootNodeType =0;/* the root bucketType retrieved from HW */

    GT_U32                              swTotalNumOfRanges=0;/* number of ranges from sw including the ranges that are hidden inside a regular node */
    GT_U32                              *swTotalRangesArr;/* the ranges values retrieved from the SW */
    GT_U32                              *swTotalRangesTypesArr;/* the ranges types values retrieved from the SW */
    GT_BOOL                             *swTotalRangesIsNonRealArr;/* Is the range a hidden one or not */

    GT_U32 nextNodeType; /* bit 22
                            Defines the next node entry type
                            0x0 = Regular
                            0x1 = Compressed
                            valid if EntryType="Trigger" */

    GT_U32 nhPointer;       /* bits 21-7
                            Pointer to the Next Hop Table or the ECMP Table,
                            based on the Leaf Type.
                            valid if EntryType="Leaf" */
    GT_U32 nhAdditionalData;/* bits 6-3
                            Contains a set of UC security check enablers
                            and IPv6 MC scope level:
                            1. [6..6] UC SIP SA Check Mismatch Enable
                            2. [5..5] UC RPF Check Enable
                            3. [4..3] IPv6 MC Group Scope Level[1..0]
                            valid if EntryType="Leaf" */
    GT_U32 nextBucketPointer;/* bits 21-2
                                Pointer to the next bucket on the (G,S) lookup
                                valid if EntryType="Trigger" */
    GT_U32 leafType; /* bit 2
                        The leaf entry type
                        0x0 = Regular Leaf
                        0x1 = Multipath Leaf
                        valid if EntryType="Leaf" */
    GT_U32 entryType;/* bit 1
                        In the process of (*,G) lookup.
                        When an entry has this bit set, the (*, G) lookup
                        terminates with a match at the current entry, and (S,
                        G) SIP based lookup is triggered.
                        Note that in such case, head of trie start address for
                        the (S,G) lookup is obtained from the the (*, G)
                        lookup stage.
                         0x0 = Leaf
                         0x1 = Trigger; Trigger IP MC S+G Lookup */
    GT_U32 lpmOverEmPriority;/* bit 0
                                Define the resolution priority between LPM and
                                Exact Match results
                                 0x0 = Low; Exact Match has priority over LPM result
                                 0x1 = High;LPM result has priority over Exact Match */

    GT_U32  leafLineHwAddr=0;
    GT_U32  offsetOfLeafInLine=0;/*the offset in HW were the 20 bits for
                                  the pointer to the next bucket on the (G,S) lookup
                                  starts this value can be:
                                  for a line leaf structure: 2 for Leaf0, 25 for Leaf1,
                                  48 for Leaf2, 71 for Leaf3, 94 for Leaf4 (92-23*leafOffsetInLine)+2
                                  for a line embedded leaf structure:
                                  91, 68, 45  (89 - leafOffsetInLine*23)+2*/

    GT_BOOL                      applyPbr=GT_FALSE;/* bit 23 Valid if EntryType ="Leaf"
                                                      0 = OFF, 1 = ON */
    GT_U32                       epgAssignedToLeafNode=0; /* bits 24-35 Valid if EntryType="Leaf"
                                                        EPG assigned to the leaf node
                                                        On SIP search - assigns source EPG
                                                        On DIP search - assigns target EPG */

    cpssOsMemSet(&hwNodeOffset[0], 0, sizeof(hwNodeOffset));
    if (isRootBucket==GT_TRUE)
    {
        /* read the HW data directly from the VRF table and update SW data in the shadow */
        status = prvCpssDxChLpmHwVrfEntryReadSip7(devNum, vrId, protocol,&hwRootNodeType, &hwRootNodeAddr);
        if (status != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(status, "Error on reading Vrf table for vrfId=%d, and protocol=%d\n");
        }
    }
    else
    {
        /* if this is not a call to the root bucket phase, but a call done from the ranges loop in
           prvCpssDxChLpmRamSyncSwHwForHaCheckPerVrIdAndProtocol_SyncRangesSip7,
           then we need to take the data address of the bucket from the shadow and to sync
           all missing data from HW data */
        switch (bucketPtr->bucketType)
        {
        case CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E:
            hwNodeOffset[i] = 0;/* need to read this data later on in the code */
            hwNodeSize=6;
            hwNodeType = PRV_CPSS_DXCH_LPM_CHILD_REGULAR_TYPE_E;
            break;
        case CPSS_DXCH_LPM_COMPRESSED_NODE_PTR_TYPE_E:
        case CPSS_DXCH_LPM_COMPRESSED_UP_TO_7_RANGES_1_LEAF_NODE_PTR_TYPE_E:
        case CPSS_DXCH_LPM_COMPRESSED_UP_TO_5_RANGES_2_LEAVES_NODE_PTR_TYPE_E:
        case CPSS_DXCH_LPM_COMPRESSED_3_RANGES_3_LEAVES_NODE_PTR_TYPE_E:
            /* in cases of last GON level the pointer will be null
               also in all other cases since the SW was not yet Sync with HW address */
            hwNodeOffset[0] = 0;/* need to read this data later on in the code */
            hwNodeSize=1;
            hwNodeType = PRV_CPSS_DXCH_LPM_CHILD_COMPRESSED_TYPE_E;
            break;
        case CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E:
        case CPSS_DXCH_LPM_MULTIPATH_ENTRY_PTR_TYPE_E:
        default:
            /* error - we should not get here in case of a leaf */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "Error - we should not get here in case of a leaf. vrfId=%d, and protocol=%d\n", vrId, protocol);
        }
    }
    if (isRootBucket==GT_TRUE)
    {
        /* Allocate Root SW Head Of Trie to be the same as HW Head Of Trie */
        if(bucketPtr->hwBucketOffsetHandle==0)
        {
            /* only the root uses hwBucketOffsetHandle pointer and not hwGroupOffsetHandle */
            nodeSize = (hwNodeType == PRV_CPSS_DXCH_LPM_CHILD_REGULAR_TYPE_E) ?
                            PRV_CPSS_DXCH_LPM_RAM_FALCON_BUCKET_SIZE_REGULAR_BV_CNS:
                            PRV_CPSS_DXCH_LPM_RAM_FALCON_BUCKET_SIZE_COMPRESSED_CNS;

            status = prvCpssDxChLpmRamMngHaSwPointerAllocateAccordingtoHwAddressSip7(shadowPtr,
                                                                                 hwRootNodeAddr,
                                                                                 nodeSize,
                                                                                 protocol,
                                                                                 isRootBucket,
                                                                                 0,/* relevant only if isRootBucket==GT_FALSE*/
                                                                                 0,/* Root is always octet0 */
                                                                                 prefixType,
                                                                                 bucketPtr);

            if (status!=GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(status, "error in swPointerAllocateAccordingtoHwAddress hwRootNodeAddr=%d",hwRootNodeAddr);
            }
        }
    }
    else
    {
        /* in case of non-root bucket the checking of the address was done in previous recursive call */
    }

    cpssOsMemSet(nodeChildAddressesArr,0,sizeof(nodeChildAddressesArr));
    cpssOsMemSet(nodeTotalChildTypesArr,0,sizeof(nodeTotalChildTypesArr));
    cpssOsMemSet(nodeTotalLinesPerTypeArr,0,sizeof(nodeTotalLinesPerTypeArr));
    cpssOsMemSet(nodeTotalBucketPerTypesArr,0,sizeof(nodeTotalBucketPerTypesArr));
    cpssOsMemSet(bankIndexsOfTheGonsArray,0,sizeof(bankIndexsOfTheGonsArray));

    hwRangesArr = cpssOsMalloc(PRV_CPSS_DXCH_LPM_RAM_MAX_NUMBER_OF_RANGES_CNS*sizeof(GT_U32));
    if (hwRangesArr == NULL)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
    cpssOsMemSet(hwRangesArr,0,PRV_CPSS_DXCH_LPM_RAM_MAX_NUMBER_OF_RANGES_CNS*sizeof(GT_U32));
    hwRangesTypesArr = cpssOsMalloc(PRV_CPSS_DXCH_LPM_RAM_MAX_NUMBER_OF_RANGES_CNS*sizeof(GT_U32));
    if (hwRangesTypesArr == NULL)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
    cpssOsMemSet(hwRangesTypesArr,0,PRV_CPSS_DXCH_LPM_RAM_MAX_NUMBER_OF_RANGES_CNS*sizeof(GT_U32));
    hwRangesBitLineArr = cpssOsMalloc(PRV_CPSS_DXCH_LPM_RAM_MAX_NUMBER_OF_RANGES_CNS*sizeof(GT_U32));
    if (hwRangesBitLineArr == NULL)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
    cpssOsMemSet(hwRangesBitLineArr,0,PRV_CPSS_DXCH_LPM_RAM_MAX_NUMBER_OF_RANGES_CNS*sizeof(GT_U32));
    hwRangesTypeIndexArr = cpssOsMalloc(PRV_CPSS_DXCH_LPM_RAM_MAX_NUMBER_OF_RANGES_CNS*sizeof(GT_U32));
    if (hwRangesTypeIndexArr == NULL)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
    cpssOsMemSet(hwRangesTypeIndexArr,0,PRV_CPSS_DXCH_LPM_RAM_MAX_NUMBER_OF_RANGES_CNS*sizeof(GT_U32));

    swTotalRangesArr = cpssOsMalloc(PRV_CPSS_DXCH_LPM_RAM_MAX_NUMBER_OF_RANGES_CNS*sizeof(GT_U32));
    if (swTotalRangesArr == NULL)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
    cpssOsMemSet(swTotalRangesArr,0,PRV_CPSS_DXCH_LPM_RAM_MAX_NUMBER_OF_RANGES_CNS*sizeof(GT_U32));
    swTotalRangesTypesArr = cpssOsMalloc(PRV_CPSS_DXCH_LPM_RAM_MAX_NUMBER_OF_RANGES_CNS*sizeof(GT_U32));
    if (swTotalRangesTypesArr == NULL)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
    cpssOsMemSet(swTotalRangesTypesArr,0,PRV_CPSS_DXCH_LPM_RAM_MAX_NUMBER_OF_RANGES_CNS*sizeof(GT_U32));
    swTotalRangesIsNonRealArr = cpssOsMalloc(PRV_CPSS_DXCH_LPM_RAM_MAX_NUMBER_OF_RANGES_CNS*sizeof(GT_U32));
    if (swTotalRangesIsNonRealArr == NULL)
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
    cpssOsMemSet(swTotalRangesIsNonRealArr,0,PRV_CPSS_DXCH_LPM_RAM_MAX_NUMBER_OF_RANGES_CNS*sizeof(GT_U32));

    if (isRootBucket==GT_TRUE)
    {
        /* hw pointer in LPM entry is in LPM lines*/
        hwNodeSize = (hwRootNodeType == PRV_CPSS_DXCH_LPM_CHILD_REGULAR_TYPE_E) ?
              PRV_CPSS_DXCH_LPM_RAM_FALCON_BUCKET_SIZE_REGULAR_BV_CNS:
              PRV_CPSS_DXCH_LPM_RAM_FALCON_BUCKET_SIZE_COMPRESSED_CNS;

        /* read the bit vector according to the address we got from the VRF entry*/
        status = prvCpssDxChLpmRamIndirectReadTableMultiEntrySip7(devNum,CPSS_DXCH_SIP5_TABLE_LPM_MEM_E,
                                                hwRootNodeAddr, hwNodeSize, &hwBucketDataArr[0]);

        if (status != GT_OK)
        {
            cpssOsFree(hwRangesArr);
            cpssOsFree(hwRangesTypesArr);
            cpssOsFree(hwRangesBitLineArr);
            cpssOsFree(hwRangesTypeIndexArr);

            cpssOsFree(swTotalRangesArr);
            cpssOsFree(swTotalRangesTypesArr);
            cpssOsFree(swTotalRangesIsNonRealArr);

            return status;
        }

       status = getNumOfRangesFromHWSip7(protocol,
                                          level,
                                          hwRootNodeType,       /* the Root node type */
                                          &hwBucketDataArr[0],  /* read from HW according to Root address */
                                          hwRootNodeAddr,       /* hwAddr is used in the fuction only for print incase of an error */
                                          nodeChildAddressesArr,
                                          nodeTotalChildTypesArr,
                                          nodeTotalLinesPerTypeArr,
                                          nodeTotalBucketPerTypesArr,
                                          &hwNumOfRanges,
                                          hwRangesArr,
                                          hwRangesTypesArr,
                                          hwRangesTypeIndexArr,
                                          hwRangesBitLineArr);
        if (status != GT_OK)
        {
            cpssOsFree(hwRangesArr);
            cpssOsFree(hwRangesTypesArr);
            cpssOsFree(hwRangesBitLineArr);
            cpssOsFree(hwRangesTypeIndexArr);

            cpssOsFree(swTotalRangesArr);
            cpssOsFree(swTotalRangesTypesArr);
            cpssOsFree(swTotalRangesIsNonRealArr);

            CPSS_LOG_ERROR_AND_RETURN_MAC(status, "fail in getNumOfRangesFromHWSip7\n");
        }

        /* set HW address of the GONs in SW shadow */
        for (i=0;i<PRV_CPSS_DXCH_LPM_RAM_FALCON_BUCKET_SIZE_REGULAR_BV_CNS;i++)
        {
            nodeSize = nodeTotalLinesPerTypeArr[i][0]+nodeTotalLinesPerTypeArr[i][1]+nodeTotalLinesPerTypeArr[i][2];
            if ((nodeSize!=0)&&(nodeChildAddressesArr[i]!=0)&&(bucketPtr->hwGroupOffsetHandle[i]==0))
            {
                status = prvCpssDxChLpmRamMngHaSwPointerAllocateAccordingtoHwAddressSip7(shadowPtr,
                                                                                     nodeChildAddressesArr[i],
                                                                                     nodeSize,/* the size of the gon */
                                                                                     protocol,
                                                                                     GT_FALSE, /* isRootBucket = GT_FALSE, we are now going over the GON pointers */
                                                                                     i,        /* gonIndex */
                                                                                     level,
                                                                                     prefixType,
                                                                                     bucketPtr);
                if (status!=GT_OK)
                {
                    cpssOsFree(hwRangesArr);
                    cpssOsFree(hwRangesTypesArr);
                    cpssOsFree(hwRangesBitLineArr);
                    cpssOsFree(hwRangesTypeIndexArr);

                    cpssOsFree(swTotalRangesArr);
                    cpssOsFree(swTotalRangesTypesArr);
                    cpssOsFree(swTotalRangesIsNonRealArr);
                    CPSS_LOG_ERROR_AND_RETURN_MAC(status, "error in swPointerAllocateAccordingtoHwAddress hwRootNodeAddr=%d",hwRootNodeAddr);
                }
            }
        }
    }
    else
    {
        /*  we are dealing with a non-root bucket */

        /* hwBucketDataArr contains the HW data of the node */
        status = getNumOfRangesFromHWSip7(protocol,
                                          level,
                                          hwNodeType,           /* the node type */
                                          &hwBucketDataArr[0],  /* given as a parameter to the function from the call to
                                                                   prvCpssDxChLpmRamDbgRangesHwShadowSyncValidityCheckSip7*/
                                          hwNodeOffset[0],      /* hwAddr is used in the fuction only for print incase of an error */
                                          nodeChildAddressesArr,
                                          nodeTotalChildTypesArr,
                                          nodeTotalLinesPerTypeArr,
                                          nodeTotalBucketPerTypesArr,
                                          &hwNumOfRanges,
                                          hwRangesArr,
                                          hwRangesTypesArr,
                                          hwRangesTypeIndexArr,
                                          hwRangesBitLineArr);
        if (status != GT_OK)
        {
            cpssOsFree(hwRangesArr);
            cpssOsFree(hwRangesTypesArr);
            cpssOsFree(hwRangesBitLineArr);
            cpssOsFree(hwRangesTypeIndexArr);

            cpssOsFree(swTotalRangesArr);
            cpssOsFree(swTotalRangesTypesArr);
            cpssOsFree(swTotalRangesIsNonRealArr);

            CPSS_LOG_ERROR_AND_RETURN_MAC(status, "fail in getNumOfRangesFromHWSip7\n");
        }
        /* set HW address of the GONs in SW shadow */
        for (i=0;i<PRV_CPSS_DXCH_LPM_RAM_FALCON_BUCKET_SIZE_REGULAR_BV_CNS;i++)
        {
            nodeSize = nodeTotalLinesPerTypeArr[i][0]+nodeTotalLinesPerTypeArr[i][1]+nodeTotalLinesPerTypeArr[i][2];
            if ((nodeSize!=0)&&(nodeChildAddressesArr[i]!=0)&&(bucketPtr->hwGroupOffsetHandle[i]==0))
            {
                status = prvCpssDxChLpmRamMngHaSwPointerAllocateAccordingtoHwAddressSip7(shadowPtr,
                                                                                     nodeChildAddressesArr[i],
                                                                                     nodeSize,/* the size of the gon */
                                                                                     protocol,
                                                                                     GT_FALSE, /* isRootBucket = GT_FALSE */
                                                                                     i,        /* gonIndex */
                                                                                     level,
                                                                                     prefixType,
                                                                                     bucketPtr);
                if (status!=GT_OK)
                {
                    cpssOsFree(hwRangesArr);
                    cpssOsFree(hwRangesTypesArr);
                    cpssOsFree(hwRangesBitLineArr);
                    cpssOsFree(hwRangesTypeIndexArr);

                    cpssOsFree(swTotalRangesArr);
                    cpssOsFree(swTotalRangesTypesArr);
                    cpssOsFree(swTotalRangesIsNonRealArr);
                    CPSS_LOG_ERROR_AND_RETURN_MAC(status, "error in swPointerAllocateAccordingtoHwAddress hwRootNodeAddr=%d",hwRootNodeAddr);
                }
            }
        }
    }

    /* 3. check that the SW ranges is the same as HW bitVector values
          get the SW number of ranges including the ones hidden incase of regular node */

    status = prvCpssDxChLpmRamDbgBucketShadowGetNumOfTotalRangesNonRealIncluddedSip7(bucketPtr,
                                                                                    &swTotalNumOfRanges,
                                                                                    swTotalRangesArr,
                                                                                    swTotalRangesTypesArr,
                                                                                    swTotalRangesIsNonRealArr);

    if (status!=GT_OK)
    {
        cpssOsFree(hwRangesArr);
        cpssOsFree(hwRangesTypesArr);
        cpssOsFree(hwRangesBitLineArr);
        cpssOsFree(hwRangesTypeIndexArr);

        cpssOsFree(swTotalRangesArr);
        cpssOsFree(swTotalRangesTypesArr);
        cpssOsFree(swTotalRangesIsNonRealArr);

        CPSS_LOG_ERROR_AND_RETURN_MAC(status, "error in call to prvCpssDxChLpmRamDbgBucketShadowGetNumOfTotalRangesNonRealIncluddedSip7\n");
    }

    /* 4. check that the SW ranges values is the same as HW ranges values */
    tempRangePtr = bucketPtr->rangeList;
    for (i=0; i<hwNumOfRanges; i++)
    {
        if (swTotalRangesIsNonRealArr[i]==GT_TRUE)
        {
            nonRealRangeIndex++;
        }

        /* compare the range pointer data */
        hwPointerType = hwRangesTypesArr[i];/* HW type can be empty=0/leaf=1/regular=2/compressed=3
                                               SW type can be regular=0/route=3/compressed=6/
                                               embedded1=7/embedded2=8/embedded3=9/multipath=9 */

        /* If the pointerType is to a MC source and the MC source bucket is a regular bucket then it means that
           this is a root of MC source tree that points directly to a nexthop or ECMP/QoS entry */
        if ((swTotalRangesIsNonRealArr[i]==GT_FALSE)&&
            ((GT_U32)swTotalRangesTypesArr[i]==(GT_U32)PRV_CPSS_DXCH_LPM_RAM_TRIE_PTR_TYPE_E) &&
            (tempRangePtr->lowerLpmPtr.nextBucket->isImplicitGroupDefault==GT_FALSE))/* when isImplicitGroupDefault==GT_FALSE we are dealing with (G,*) */
            {
                status = prvCpssDxChLpmRamDbgGetLeafDataSip7(devNum,
                                        bucketPtr,
                                        hwBucketGonAddr,
                                        hwBucketDataArr,
                                        hwRangesTypesArr[i],
                                        hwRangesTypeIndexArr[i],
                                        nodeChildAddressesArr[hwRangesBitLineArr[i]],
                                        nodeTotalChildTypesArr[hwRangesBitLineArr[i]],
                                        GT_TRUE,/*returnOnFailure*/
                                        &nextNodeType,
                                        &nhAdditionalData,
                                        &nextBucketPointer,
                                        &nhPointer,
                                        &leafType,
                                        &entryType,
                                        &lpmOverEmPriority,
                                        &leafLineHwAddr,
                                        &offsetOfLeafInLine,
                                        &applyPbr,
                                        &epgAssignedToLeafNode);
                if (status!=GT_OK)
                {
                    cpssOsFree(hwRangesArr);
                    cpssOsFree(hwRangesTypesArr);
                    cpssOsFree(hwRangesBitLineArr);
                    cpssOsFree(hwRangesTypeIndexArr);

                    cpssOsFree(swTotalRangesArr);
                    cpssOsFree(swTotalRangesTypesArr);
                    cpssOsFree(swTotalRangesIsNonRealArr);

                    CPSS_LOG_ERROR_AND_RETURN_MAC(status, "Error: no synchronization between HW and SW - prvCpssDxChLpmRamDbgGetLeafDataSip7\n");
                }
            }
            else
            {
                switch (hwPointerType)
                {
                case PRV_CPSS_DXCH_LPM_CHILD_LEAF_TYPE_E:

                     /* only incase we are dealing with a non-hidden range we should continue
                        checking this leaf */
                    if (swTotalRangesIsNonRealArr[i]==GT_FALSE)
                    {
                        status = prvCpssDxChLpmRamDbgGetLeafDataSip7(devNum,
                                                                    bucketPtr,
                                                                    hwBucketGonAddr,
                                                                    hwBucketDataArr,
                                                                    hwRangesTypesArr[i],
                                                                    hwRangesTypeIndexArr[i],
                                                                    nodeChildAddressesArr[hwRangesBitLineArr[i]],
                                                                    nodeTotalChildTypesArr[hwRangesBitLineArr[i]],
                                                                    GT_TRUE,/*returnOnFailure*/
                                                                    &nextNodeType,
                                                                    &nhAdditionalData,
                                                                    &nextBucketPointer,
                                                                    &nhPointer,
                                                                    &leafType,
                                                                    &entryType,
                                                                    &lpmOverEmPriority,
                                                                    &leafLineHwAddr,
                                                                    &offsetOfLeafInLine,
                                                                    &applyPbr,
                                                                    &epgAssignedToLeafNode);
                        if (status!=GT_OK)
                        {
                            cpssOsFree(hwRangesArr);
                            cpssOsFree(hwRangesTypesArr);
                            cpssOsFree(hwRangesBitLineArr);
                            cpssOsFree(hwRangesTypeIndexArr);

                            cpssOsFree(swTotalRangesArr);
                            cpssOsFree(swTotalRangesTypesArr);
                            cpssOsFree(swTotalRangesIsNonRealArr);

                            CPSS_LOG_ERROR_AND_RETURN_MAC(status, "Error: no synchronization between HW and SW - prvCpssDxChLpmRamDbgGetLeafDataSip7\n");
                        }

                        if((GT_U32)swTotalRangesTypesArr[i]==(GT_U32)PRV_CPSS_DXCH_LPM_RAM_TRIE_PTR_TYPE_E)
                        {
                            /* we are dealing with a range that points to a SRC tree that is not a NH (this was dealt with above) */

                            /* if entryType is trigger we need to set the hw value we got :
                               nextBucketPointer and HW offset of the Leaf */
                            tempRangePtr->lowerLpmPtr.nextBucket->nodeMemAddr=nextBucketPointer;
                            tempRangePtr->lowerLpmPtr.nextBucket->fifthAddress=(GT_U8)offsetOfLeafInLine;
                            tempRangePtr->lowerLpmPtr.nextBucket->pointingRangeMemAddr=leafLineHwAddr;

                            /* HW offset handle for MC is a special case that we use hwBucketOffsetHandle and not hwGroupOffsetHandle */
                            /* set HW address of the GONs in SW shadow */
                            if(nextNodeType==0)
                                nodeSize=PRV_CPSS_DXCH_LPM_RAM_FALCON_BUCKET_SIZE_REGULAR_BV_CNS;
                            else
                                nodeSize=PRV_CPSS_DXCH_LPM_RAM_FALCON_BUCKET_SIZE_COMPRESSED_CNS;

                            status = prvCpssDxChLpmRamMngHaSwPointerAllocateAccordingtoHwAddressSip7(shadowPtr,
                                                                                                 nextBucketPointer,
                                                                                                 nodeSize,/* the size of the gon */
                                                                                                 protocol,
                                                                                                 GT_TRUE,  /* isRootBucket = GT_TRUE - Root of SRC */
                                                                                                 0,        /* gonIndex */
                                                                                                 0,        /* srcRoot always in octetIndex=0*/
                                                                                                 prefixType,
                                                                                                 tempRangePtr->lowerLpmPtr.nextBucket);
                            if (status!=GT_OK)
                            {
                                cpssOsFree(hwRangesArr);
                                cpssOsFree(hwRangesTypesArr);
                                cpssOsFree(hwRangesBitLineArr);
                                cpssOsFree(hwRangesTypeIndexArr);

                                cpssOsFree(swTotalRangesArr);
                                cpssOsFree(swTotalRangesTypesArr);
                                cpssOsFree(swTotalRangesIsNonRealArr);
                                CPSS_LOG_ERROR_AND_RETURN_MAC(status, "error in swPointerAllocateAccordingtoHwAddress hwRootNodeAddr=%d",hwRootNodeAddr);
                            }
                        }
                    }
                    break;
                case PRV_CPSS_DXCH_LPM_CHILD_REGULAR_TYPE_E:
                case PRV_CPSS_DXCH_LPM_CHILD_COMPRESSED_TYPE_E:
                    /* nothing to check at this moment the node will be checked in the function
                       prvCpssDxChLpmRamSyncSwHwForHaCheckPerVrIdAndProtocol_SyncRangesSip7 */
                    break;
                default:
                    cpssOsFree(hwRangesArr);
                    cpssOsFree(hwRangesTypesArr);
                    cpssOsFree(hwRangesBitLineArr);
                    cpssOsFree(hwRangesTypeIndexArr);

                    cpssOsFree(swTotalRangesArr);
                    cpssOsFree(swTotalRangesTypesArr);
                    cpssOsFree(swTotalRangesIsNonRealArr);

                    CPSS_LOG_ERROR_AND_RETURN_MAC(status, "Error: no synchronization between HW and SW - - error in HW Pointer Type\n");
                    break;
                }
            }

            /* update the HW is not needed --> change the flag */
            tempRangePtr->updateRangeInHw = GT_FALSE;

            /* if this is a hidden range it will not apear in the tempRangePtr,
               so need to continue with the same tempRangePtr */
            if ((swTotalRangesIsNonRealArr[i]==GT_FALSE)&&(tempRangePtr->next != NULL))
            {
                tempRangePtr = tempRangePtr->next;
            }
    }
    if(i != hwNumOfRanges)
    {
        cpssOsFree(hwRangesArr);
        cpssOsFree(hwRangesTypesArr);
        cpssOsFree(hwRangesBitLineArr);
        cpssOsFree(hwRangesTypeIndexArr);

        cpssOsFree(swTotalRangesArr);
        cpssOsFree(swTotalRangesTypesArr);
        cpssOsFree(swTotalRangesIsNonRealArr);

        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Error: no synchronization between HW and SW - illegal ranges values\n");
    }
    /* 6. if the SW range point to a next bucket then recursively check the new bucket, stages 1-5  */
    if (bucketPtr->numOfRanges >= 2)
    {
        /* Ranges validity check */
        status = prvCpssDxChLpmRamSyncSwHwForHaCheckPerVrIdAndProtocol_SyncRangesSip7(devNum,
                                                             shadowPtr,
                                                             vrId,
                                                             bucketPtr,
                                                             nodeTotalChildTypesArr,
                                                             level,
                                                             numOfMaxAllowedLevels,
                                                             prefixType,
                                                             protocol);
        if (status != GT_OK)
        {
            cpssOsFree(hwRangesArr);
            cpssOsFree(hwRangesTypesArr);
            cpssOsFree(hwRangesBitLineArr);
            cpssOsFree(hwRangesTypeIndexArr);

            cpssOsFree(swTotalRangesArr);
            cpssOsFree(swTotalRangesTypesArr);
            cpssOsFree(swTotalRangesIsNonRealArr);
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "prvCpssDxChLpmRamSyncSwHwForHaCheckPerVrIdAndProtocol_SyncRangesSip7");
        }
    }
    else
    {
        cpssOsFree(hwRangesArr);
        cpssOsFree(hwRangesTypesArr);
        cpssOsFree(hwRangesBitLineArr);
        cpssOsFree(hwRangesTypeIndexArr);

        cpssOsFree(swTotalRangesArr);
        cpssOsFree(swTotalRangesTypesArr);
        cpssOsFree(swTotalRangesIsNonRealArr);
        /* error - a bucket can not have less then 2 ranges */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Error: a bucket can not have less then 2 ranges,"
                                               "fail in prvCpssDxChLpmRamSyncSwHwForHaCheckPerVrIdAndProtocol_SyncBucketSip7\n");
    }

    cpssOsFree(hwRangesArr);
    cpssOsFree(hwRangesTypesArr);
    cpssOsFree(hwRangesBitLineArr);
    cpssOsFree(hwRangesTypeIndexArr);

    cpssOsFree(swTotalRangesArr);
    cpssOsFree(swTotalRangesTypesArr);
    cpssOsFree(swTotalRangesIsNonRealArr);

    return status;
}

/**
* @internal prvCpssDxChLpmRamSyncSwHwForHaCheckPerVrIdAndProtocolSip7 function
* @endinternal
*
* @brief  Update SW Shadow with relevant data from HW, and
*         allocate DMM memory according to HW memory for a
*         specific vrId. Relevant for HA process
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                  Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] shadowPtr             - the shadow to work on.
* @param[in] vrId                  - The virtual's router ID.
* @param[in] protocolStack         - type of ip protocol stack to work on.
*
* @retval GT_OK                    - if success, or
* @retval GT_OUT_OF_CPU_MEM        - if failed to allocate CPU memory, or
* @retval GT_OUT_OF_PP_MEM         - if failed to allocate PP memory, or
* @retval GT_NOT_APPLICABLE_DEVICE - if not applicable device, or
* @retval GT_FAIL                  - otherwise.
*
*/
static GT_STATUS prvCpssDxChLpmRamSyncSwHwForHaCheckPerVrIdAndProtocolSip7
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC     *shadowPtr,
    IN GT_U32                               vrId,
    IN PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT protocolStack
)
{
    GT_STATUS                                rc=GT_OK;
    GT_U32                                   hwBucketDataArr[PRV_CPSS_DXCH_LPM_RAM_FALCON_MAX_SIZE_OF_BUCKET_IN_LPM_LINES_CNS*PRV_CPSS_DXCH_LPM_RAM_SIP7_SIZE_OF_LPM_ENTRY_IN_WORDS_CNS];
    GT_U32                                   gonNodeAddr=0; /* Node address to read the HW data from */
    PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC  *bucketPtr;
    GT_U8                                    numOfMaxAllowedLevels;

    /* read the HW data directly from the VRF table and compare it to the SW data in the shadow
       since LPM shadow has a shared device list - we will take the first device in the list
       to be used in calling prvCpssDxChLpmHwVrfEntryReadSip7 API */

    if(shadowPtr->vrRootBucketArray[vrId].isUnicastSupported[protocolStack]==GT_TRUE)
    {
        bucketPtr = shadowPtr->vrRootBucketArray[vrId].rootBucket[protocolStack];
        switch(protocolStack)
        {
        case PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E:
            numOfMaxAllowedLevels = PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS;
            break;
        case PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E:
            numOfMaxAllowedLevels = PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS;
            break;
        case PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E:
            numOfMaxAllowedLevels = PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_FCOE_PROTOCOL_CNS;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, "protocol type not supported for HA sync");

        }

        cpssOsMemSet(&hwBucketDataArr[0], 0, sizeof(hwBucketDataArr));

        /* keep values in case reconstruct is needed */
        cpssOsMemCpy(shadowPtr->tempLpmRamOctetsToBlockMappingUsedForReconstractPtr,shadowPtr->lpmRamOctetsToBlockMappingPtr,
                 sizeof(PRV_CPSS_DXCH_LPM_RAM_OCTETS_TO_BLOCK_MAPPING_STC)*shadowPtr->numOfLpmMemories);

        rc = prvCpssDxChLpmRamSyncSwHwForHaCheckPerVrIdAndProtocol_SyncBucketSip7(shadowPtr->shadowDevList.shareDevs[0],
                                                                            shadowPtr,
                                                                            vrId,
                                                                            bucketPtr,
                                                                            gonNodeAddr,    /* not used in the first call */
                                                                            &hwBucketDataArr[0],
                                                                            0,              /* level */
                                                                            numOfMaxAllowedLevels,
                                                                            CPSS_UNICAST_E,
                                                                            protocolStack,
                                                                            GT_TRUE);       /* isRootBucket */

        if (rc != GT_OK)
        {
            return rc;
        }
    }
    /* FCOE does not support Multicast */
    if ((protocolStack==PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E)||(protocolStack==PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E))
    {
        if (shadowPtr->vrRootBucketArray[vrId].isMulticastSupported[protocolStack] == GT_TRUE)
        {
            bucketPtr = shadowPtr->vrRootBucketArray[vrId].rootBucket[protocolStack];
            switch(protocolStack)
            {
            case PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E:
                numOfMaxAllowedLevels = 2*PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS;
                break;
            case PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E:
                numOfMaxAllowedLevels = 2*PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS;
                break;
            case PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E:
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_SUPPORTED, "protocol type not supported for HA sync");

            }
            /* keep values in case reconstruct is needed */
            cpssOsMemCpy(shadowPtr->tempLpmRamOctetsToBlockMappingUsedForReconstractPtr,shadowPtr->lpmRamOctetsToBlockMappingPtr,
                     sizeof(PRV_CPSS_DXCH_LPM_RAM_OCTETS_TO_BLOCK_MAPPING_STC)*shadowPtr->numOfLpmMemories);

            rc = prvCpssDxChLpmRamSyncSwHwForHaCheckPerVrIdAndProtocol_SyncBucketSip7(shadowPtr->shadowDevList.shareDevs[0],
                                                                                shadowPtr,
                                                                                vrId,
                                                                                bucketPtr,
                                                                                gonNodeAddr,    /* not used in the first call */
                                                                                &hwBucketDataArr[0],
                                                                                0,              /* level */
                                                                                numOfMaxAllowedLevels,
                                                                                CPSS_MULTICAST_E,
                                                                                protocolStack,
                                                                                GT_TRUE);       /* isRootBucket */
            if (rc != GT_OK)
            {
                return rc;
            }
        }
    }
    return rc;
}

/**
* @internal prvCpssDxChLpmRamSyncSwHwForHaSip7 function
* @endinternal
*
* @brief  Update SW Shadow with relevant data from HW, and
*         allocate DMM memory according to HW memory.
*         Relevant for HA process
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                  Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @retval GT_OK                    - if success, or
* @retval GT_OUT_OF_CPU_MEM        - if failed to allocate CPU memory, or
* @retval GT_OUT_OF_PP_MEM         - if failed to allocate PP memory, or
* @retval GT_NOT_APPLICABLE_DEVICE - if not applicable device, or
* @retval GT_FAIL                  - otherwise.
*
*
*/
GT_STATUS prvCpssDxChLpmRamSyncSwHwForHaSip7
(
    GT_VOID
)
{
    GT_STATUS       rc;                                             /* return code */
    GT_U32          lpmDbId = 0;                                    /* LPM DB index */
    GT_UINTPTR      slIterator;                                     /* Skip List iterator */
    PRV_CPSS_DXCH_LPM_SHADOW_STC            *lpmDbPtr,tmpLpmDb;     /* pointer to and temp instance of LMP DB entry */
    PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC        *ipShadowPtr;           /* pointer to shadow entry */
    PRV_CPSS_DXCH_LPM_RAM_SHADOWS_DB_STC    *ipShadowsPtr;          /* pointer to shadows DB */

    GT_U32                                  shadowIdx;              /* shadow index */
    GT_U32                                  i;                      /* loop iterator */
    CPSS_DXCH_IP_LPM_SHADOW_TYPE_ENT        shadowType;
    CPSS_IP_PROTOCOL_STACK_ENT              protocolStack;
    CPSS_DXCH_IP_LPM_MEMORY_CONFIG_UNT      memoryCfg;
    PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT    currProtocolStack;
    GT_U32                                  devNum=0;

    cpssOsMemSet(&memoryCfg,0,sizeof(memoryCfg));
    if (PRV_SHARED_PORT_DIR_IP_LPM_SRC_GLOBAL_VAR_GET(lpmDbSL) == NULL)
    {
        CPSS_LOG_INFORMATION_MAC("in prvCpssDxChLpmRamSyncSwHwForHa- prvCpssDxChIplpmDbSL not initialized");
        return GT_OK;
    }
    slIterator = 0;
    while (1)
    {
        rc =  prvCpssDxChIpLpmDbIdGetNext(&lpmDbId,&slIterator);
        if (rc == GT_NO_MORE)
        {
            break;
        }
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "error in prvCpssDxChLpmRamSyncSwHwForHa-prvCpssDxChIpLpmDbIdGetNext");
        }

        rc = cpssDxChIpLpmDBConfigGet(lpmDbId,&shadowType,&protocolStack,&memoryCfg);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "error in prvCpssDxChLpmRamSyncSwHwForHa-cpssDxChIpLpmDBConfigGet");
        }

        /* check parameters */
        tmpLpmDb.lpmDbId = lpmDbId;
        lpmDbPtr = prvCpssSlSearch(PRV_SHARED_PORT_DIR_IP_LPM_SRC_GLOBAL_VAR_GET(lpmDbSL),&tmpLpmDb);
        if (lpmDbPtr == NULL)
        {
            /* can't find the lpm DB */
            return GT_OK;
        }

        ipShadowsPtr = lpmDbPtr->shadow;

        for (shadowIdx = 0; shadowIdx < ipShadowsPtr->numOfShadowCfg; shadowIdx++)
        {
            ipShadowPtr = &ipShadowsPtr->shadowArray[shadowIdx];
            if (ipShadowPtr->shadowDevList.shareDevNum==0)
            {
                /* There is no added devices */
                CPSS_LOG_INFORMATION_MAC("in prvCpssDxChLpmRamSyncSwHwForHa- no added devices: shareDevNum=%d", ipShadowPtr->shadowDevList.shareDevNum);
                return GT_OK;
            }

            for (devNum=0;devNum<ipShadowPtr->shadowDevList.shareDevNum;devNum++)
            {
                /* default value mean: parameter was not configured yet
                   we want to force the setting of the lpmMemBankId to the HW in the HA process */
                PRV_CPSS_DXCH_PP_MAC(ipShadowPtr->shadowDevList.shareDevs[devNum])->moduleCfg.ip.lastBankIdConfigured = 0xFFF;
            }

            for(i = 0; i < ipShadowPtr->vrfTblSize; i++)
            {
                for (currProtocolStack = PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E;
                      currProtocolStack < PRV_CPSS_DXCH_LPM_PROTOCOL_LAST_E;
                      currProtocolStack++)
                {
                    if ((isProtocolSetInBitmapSip7(ipShadowsPtr->protocolBitmap, currProtocolStack) == GT_FALSE) ||
                        (ipShadowPtr->isProtocolInitialized[currProtocolStack] == GT_FALSE))
                    {
                        continue;
                    }
                    /* go over all valid VR , if it's initilized for this protocol */
                    if ((ipShadowPtr->vrRootBucketArray[i].valid == GT_TRUE) &&
                        (ipShadowPtr->vrRootBucketArray[i].rootBucket[currProtocolStack] != NULL))
                    {
                        rc = prvCpssDxChLpmRamSyncSwHwForHaCheckPerVrIdAndProtocolSip7(ipShadowPtr, i, currProtocolStack);

                        if (rc != GT_OK)
                        {
                            CPSS_LOG_ERROR_MAC("Error: got a fail in SW/HW sync in HA  - prvCpssDxChLpmRamSyncSwHwForHa \n");

                            /* for AAS we support logic to reconstruct HW according to existing SW */

                            /* if we got an error in sync operation,2 cases can cause the error
                               If the config exists on HW but didn't exist on replay data --> need to remove it from HW.
                               If the config doesn't exist on HW but exist on replay data --> need to add it to HW.

                               following will be done:
                               Clean HW data for all protocols and all vrfs
                               Clean all allocations done in the sync process
                               Allocate new DMM memory
                               Update the HW according to the existing shadow */

                            /* reset pending array - sync failed we will start from the
                               beggining the allocations and counters calculations */
                            cpssOsMemSet(ipShadowPtr->pendingBlockToUpdateArr,0,sizeof(ipShadowPtr->pendingBlockToUpdateArr));
                            cpssOsMemSet(ipShadowPtr->allNewNextMemInfoAllocatedPerOctetArrayPtr,0,sizeof(ipShadowPtr->allNewNextMemInfoAllocatedPerOctetArrayPtr));
                            cpssOsMemSet(ipShadowPtr->tempLpmRamOctetsToBlockMappingUsedForReconstractPtr,0,sizeof(ipShadowPtr->tempLpmRamOctetsToBlockMappingUsedForReconstractPtr));
                            rc = prvCpssDxChLpmRamCleanAndUpdateSwAndHwAfterHaSyncFailedSip7(ipShadowPtr);
                            if (rc!=GT_OK)
                            {
                                /* reset pending array for future use */
                                cpssOsMemSet(ipShadowPtr->pendingBlockToUpdateArr,0,sizeof(ipShadowPtr->pendingBlockToUpdateArr));
                                 /* reset the array for next use */
                                cpssOsMemSet(ipShadowPtr->globalMemoryBlockTakenArr, 0, sizeof(ipShadowPtr->globalMemoryBlockTakenArr));
                                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "error in prvCpssDxChLpmRamSyncSwHwForHa-prvCpssDxChLpmRamCleanAndUpdateSwAndHwAfterHaSyncFailedSip7");
                            }
                            return rc;
                        }

                        rc = prvCpssDxChLpmRamUpdateBlockUsageCountersSip7(ipShadowPtr->lpmRamBlocksSizeArrayPtr,
                                                                ipShadowPtr->pendingBlockToUpdateArr,
                                                                ipShadowPtr->protocolCountersPerBlockArr,
                                                                ipShadowPtr->pendingBlockToUpdateArr,
                                                                currProtocolStack,
                                                                ipShadowPtr->numOfLpmMemories);
                        if (rc!=GT_OK)
                        {
                            /* reset pending array for future use */
                            cpssOsMemSet(ipShadowPtr->pendingBlockToUpdateArr,0,sizeof(ipShadowPtr->pendingBlockToUpdateArr));

                            return rc;
                        }
                    }
                }
            }
        }
    }
    return GT_OK;
}


/**
* @internal prvCpssDxChLpmRamIndirectWriteTableMultiEntrySip7 function
* @endinternal
*
* @brief   Write number of entries to the table in consecutive indexes
*          LPM table support inderect write.
*
* @param[in] devNum                   - the device number
* @param[in] tableType                - the specific table name
* @param[in] startIndex               - index to the first table entry
* @param[in] numOfEntries             - the number of consecutive entries to write
* @param[in] entryValueArrayPtr       - (pointer to) the data that will be written to the table
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmRamIndirectWriteTableMultiEntrySip7
(
    IN GT_U8                            devNum,
    IN CPSS_DXCH_TABLE_ENT              tableType,
    IN GT_U32                           startIndex,
    IN GT_U32                           numOfEntries,
    IN GT_U32                           *entryValueArrayPtr
)
{
    GT_STATUS rc=GT_OK;
    GT_U32    bankNumber=0,offsetInBank=0;

    CPSS_NULL_PTR_CHECK_MAC(entryValueArrayPtr);

    if( 0 == numOfEntries )
        return GT_OK;

    /* extract the bankNumber and offset from the line index */
    offsetInBank = startIndex & PRV_CPSS_DXCH_LPM_RAM_OFFSET_MASK_CNS;
    bankNumber = (startIndex >> PRV_CPSS_DXCH_LPM_RAM_OFFSET_NUMBER_OF_BITS_CNS) & PRV_CPSS_DXCH_LPM_RAM_BANK_MASK_CNS;

    if (bankNumber>=PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.sip6_maxBlocksWithShared)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "FAIL in prvCpssDxChLpmRamIndirectWriteTableMultiEntrySip7, bank number bigger then max allowed banks \n");
    }

    /* if the last bank configured is the same as the one we want to set to hw
       no need to set it again */
    if(PRV_CPSS_DXCH_PP_MAC(devNum)->moduleCfg.ip.lastBankIdConfigured!=bankNumber)
    {
        /* write bankId to the register */
        rc = prvCpssHwPpSetRegField(devNum,
                    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->LPM.lpmMemBankId,
                    0, 6, bankNumber);
        if (rc != 0)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "TABLE_LPM_MEM bankId write FAIL \n");
        }

        /* save the last bankId configured to hw */
        PRV_CPSS_DXCH_PP_MAC(devNum)->moduleCfg.ip.lastBankIdConfigured = bankNumber;
    }

    /* write the LPM memory to the singleBank memory */
    rc = prvCpssDxChRamMngWriteMultiEntrySip7(devNum,
                                            tableType,
                                            offsetInBank,
                                            numOfEntries,
                                            entryValueArrayPtr);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "TABLE_LPM_MEM indirect write FAIL \n");
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChLpmRamIndirectReadTableMultiEntrySip7 function
* @endinternal
*
* @brief   Read number of entries to the table in consecutive indexes
*          LPM table support inderect write.
*
* @param[in] devNum                   - the device number
* @param[in] tableType                - the specific table name
* @param[in] startIndex               - index to the first table entry
* @param[in] numOfEntries             - the number of consecutive entries to read
*
* @param[out] entryValueArrayPtr       - (pointer to) the data that will be read from the table
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - on failure.
* @retval GT_OUT_OF_RANGE          - parameter not in valid range
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChLpmRamIndirectReadTableMultiEntrySip7
(
    IN  GT_U8                            devNum,
    IN  CPSS_DXCH_TABLE_ENT              tableType,
    IN  GT_U32                           startIndex,
    IN  GT_U32                           numOfEntries,
    OUT GT_U32                           *entryValueArrayPtr
)
{
    GT_STATUS rc=GT_OK;
    GT_U32    bankNumber=0,offsetInBank=0;

    CPSS_SYSTEM_RECOVERY_INFO_STC           oldSystemRecoveryInfo,newSystemRecoveryInfo;
    GT_BOOL                                 tempSystemRecoveryUsed=GT_FALSE;

    CPSS_NULL_PTR_CHECK_MAC(entryValueArrayPtr);

    if( 0 == numOfEntries )
        return GT_OK;

    /* extract the bankNumber and offset from the line index */
    offsetInBank = startIndex & PRV_CPSS_DXCH_LPM_RAM_OFFSET_MASK_CNS;
    bankNumber = (startIndex >> PRV_CPSS_DXCH_LPM_RAM_OFFSET_NUMBER_OF_BITS_CNS) & PRV_CPSS_DXCH_LPM_RAM_BANK_MASK_CNS;

    if (bankNumber>=PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.sip6_maxBlocksWithShared)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "FAIL in prvCpssDxChLpmRamIndirectReadTableMultiEntrySip7, bank number bigger then max allowed banks \n");
    }

    /* in case of HA we need to write the lpmMemBankId to the HW before we
       can read from the proper location, so temporarely change the flag */
    rc = cpssSystemRecoveryStateGet(&oldSystemRecoveryInfo);
    if (rc != GT_OK)
    {
        return rc;
    }
    newSystemRecoveryInfo = oldSystemRecoveryInfo;
    if ((oldSystemRecoveryInfo.systemRecoveryState == CPSS_SYSTEM_RECOVERY_HW_CATCH_UP_STATE_E) &&
        (oldSystemRecoveryInfo.systemRecoveryProcess == CPSS_SYSTEM_RECOVERY_PROCESS_HA_E))

    {
        newSystemRecoveryInfo.systemRecoveryState = CPSS_SYSTEM_RECOVERY_COMPLETION_STATE_E;
        newSystemRecoveryInfo.systemRecoveryProcess = CPSS_SYSTEM_RECOVERY_PROCESS_HA_E;
        prvCpssSystemRecoveryStateUpdate(&newSystemRecoveryInfo);
        /* set flag - need to reconstruct back to the original state after the HW write */
        tempSystemRecoveryUsed = GT_TRUE;
    }

    /* if the last bank configured is the same as the one we want to set to hw
       no need to set it again */
    if((PRV_CPSS_DXCH_PP_MAC(devNum)->moduleCfg.ip.lastBankIdConfigured!=bankNumber)||
       (tempSystemRecoveryUsed==GT_TRUE))
    {
        /* write bankId to the register */
        rc = prvCpssHwPpSetRegField(devNum,
                    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(devNum)->LPM.lpmMemBankId,
                    0, 6, bankNumber);
        if (rc != 0)
        {
            prvCpssSystemRecoveryStateUpdate(&oldSystemRecoveryInfo);

            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "TABLE_LPM_MEM bankId write FAIL \n");
        }

        /* save the last bankId configured to hw */
        PRV_CPSS_DXCH_PP_MAC(devNum)->moduleCfg.ip.lastBankIdConfigured = bankNumber;

        if (tempSystemRecoveryUsed==GT_TRUE)
        {
            /* restore back the HA recovery state */
            prvCpssSystemRecoveryStateUpdate(&oldSystemRecoveryInfo);
            tempSystemRecoveryUsed = GT_FALSE;
        }
    }

    rc = prvCpssDxChReadTableMultiEntry(devNum,
                                        tableType,
                                        offsetInBank,
                                        numOfEntries,
                                        entryValueArrayPtr);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "TABLE_LPM_MEM indirect read FAIL \n");
    }

    return GT_OK;
}


/**
* @internal prvCpssDxChLpmCleanHwAfterHaSyncFailedSip7 function
* @endinternal
*
* @brief  Clean LPM HW configuration for all protocols and all
*         VRFs on all devices
*         Relevant for HA process
*
* @note   APPLICABLE DEVICES:      AAS;
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                  Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] shadowPtr             - the shadow to work on.
*
* @retval GT_OK                    - if success, or
* @retval GT_OUT_OF_CPU_MEM        - if failed to allocate CPU memory, or
* @retval GT_OUT_OF_PP_MEM         - if failed to allocate PP memory, or
* @retval GT_NOT_APPLICABLE_DEVICE - if not applicable device, or
* @retval GT_FAIL                  - otherwise.
*
*
*/
GT_STATUS prvCpssDxChLpmCleanHwAfterHaSyncFailedSip7
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC     *shadowPtr
)
{
    GT_STATUS                                   rc = GT_OK;
    GT_U32                                      vrfId = 0;
    PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT        protocol;
    GT_U32                                      devNum;
    GT_U32                                      headOfTrie = 0;
    CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT         nodeType = 0;

    /* Clean all HW data related to all VR, by basically
       "cutting" the root nodes HW pointers.
       We cannot use the flush functions we have
       since flush cleans the SW and the HW */

    /* Make sure all 4K virtual routers are deleted from HW  */
    for (vrfId = 0 ; vrfId < BIT_12 ; vrfId++)
    {
        /* remove vrf for all protocola*/
        for (protocol = PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E;
             protocol < PRV_CPSS_DXCH_LPM_PROTOCOL_LAST_E;
             protocol++)
        {
            /* reset the VRF entry with default */
            for (devNum = 0; devNum < PRV_CPSS_MAX_PP_DEVICES_CNS; devNum++)
            {
                if(PRV_CPSS_IS_DEV_EXISTS_MAC(devNum))
                {
                    nodeType   = CPSS_DXCH_LPM_COMPRESSED_NODE_PTR_TYPE_E;
                    headOfTrie = PRV_CPSS_DXCH_LPM_RAM_GET_SIP7_LPM_OFFSET_FROM_DMM_MAC(shadowPtr->defaultAddrForHeadOfTrie);

                    rc = prvCpssDxChLpmHwVrfEntryWriteSip7(devNum,
                                                           vrfId,
                                                           protocol,
                                                           nodeType,
                                                           headOfTrie);
                    if (rc != GT_OK)
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "ERROR: fail in prvCpssDxChLpmCleanHwAfterHaSyncFailed, "
                                                          "fail in writing to VRF table");
                    }
                }
            }
        }
    }
    return rc;
}

/**
* @internal cleanSwTreeRangesFromDmmAllocDoneInHaSyncSip7 function
* @endinternal
*
* @brief   clean SW Shadow allocation done during LPM HA Sync - operation done on a range
*
* @param[in] shadowPtr                - pointer to shadow structure
* @param[in] vrId                     - the virtual router id
* @param[in] bucketPtr                - pointer to the bucket
* @param[in] level                    - the level in the tree (first  is 0)
* @param[in] numOfMaxAllowedLevels    - the maximal number of levels that is allowed
*                                      for the relevant protocol and prefix type
* @param[in] prefixType               - unicast or multicast tree
* @param[in] protocol                 - protocol
*
* @retval GT_FAIL                  - on failure
* @retval GT_OK                    - on success
*/
GT_STATUS cleanSwTreeRangesFromDmmAllocDoneInHaSyncSip7
(
    IN  PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC          *shadowPtr,
    IN  GT_U32                                    vrId,
    IN  PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC   *bucketPtr,
    IN  GT_U8                                     level,
    IN  GT_U8                                     numOfMaxAllowedLevels,
    IN  CPSS_UNICAST_MULTICAST_ENT                prefixType,
    IN  PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT      protocol
)
{
    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC  *rangePtr;
    GT_U8                                   prefixTypeFirstRange, prefixTypeLastRange;
    GT_U8                                   prefixTypeSecondRange=0, prefixTypeSecondLastRange=0;
    GT_U32                                  tmpPrefixTypeRange;
    GT_STATUS                               status = GT_OK;

    rangePtr = bucketPtr->rangeList;

    if (level == 0)
    {
        if (prefixType == CPSS_UNICAST_E)
        {
            prefixTypeFirstRange = 0;
            tmpPrefixTypeRange = (protocol == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E) ?
                (PRV_CPSS_DXCH_LPM_RAM_START_OF_IPV4_MC_ADDRESS_SPACE_CNS - 1) :
                (PRV_CPSS_DXCH_LPM_RAM_START_OF_IPV6_MC_ADDRESS_SPACE_CNS - 1);
            prefixTypeLastRange = (GT_U8)tmpPrefixTypeRange;

            if (protocol == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E)
            {
                prefixTypeSecondRange = PRV_CPSS_DXCH_LPM_RAM_START_OF_IPV4_RESERVED_SPACE_ADDRESS_SPACE_CNS;
                prefixTypeSecondLastRange = PRV_CPSS_DXCH_LPM_RAM_END_OF_IPV4_RESERVED_SPACE_ADDRESS_SPACE_CNS;
            }
        }
        else /* CPSS_MULTICAST_E */
        {
            tmpPrefixTypeRange = (protocol == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E) ?
                PRV_CPSS_DXCH_LPM_RAM_START_OF_IPV4_MC_ADDRESS_SPACE_CNS :
                PRV_CPSS_DXCH_LPM_RAM_START_OF_IPV6_MC_ADDRESS_SPACE_CNS;
            prefixTypeFirstRange = (GT_U8)tmpPrefixTypeRange;
            tmpPrefixTypeRange = (protocol == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E) ?
                PRV_CPSS_DXCH_LPM_RAM_END_OF_IPV4_MC_ADDRESS_SPACE_CNS :
                PRV_CPSS_DXCH_LPM_RAM_END_OF_IPV6_MC_ADDRESS_SPACE_CNS;
            prefixTypeLastRange = (GT_U8)tmpPrefixTypeRange;

            prefixTypeSecondLastRange = prefixTypeLastRange;
        }
    }
    else
    {
        prefixTypeFirstRange = 0;
        prefixTypeLastRange = 255;
        prefixTypeSecondRange = 0;
        prefixTypeSecondLastRange = 255;

    }

    /* Check next buckets */
    rangePtr = bucketPtr->rangeList;

    while (rangePtr)
    {
        if ((rangePtr->pointerType != CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E) &&
            (rangePtr->pointerType != CPSS_DXCH_LPM_MULTIPATH_ENTRY_PTR_TYPE_E))
        {
            if (protocol == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E)
            {
                if ((rangePtr->startAddr < prefixTypeFirstRange) ||
                    ((rangePtr->startAddr > prefixTypeLastRange) && (rangePtr->startAddr<prefixTypeSecondRange))||
                     (rangePtr->startAddr > prefixTypeSecondLastRange))
                {
                    rangePtr = rangePtr->next;
                    continue;
                }
            }
            else
            {
                if ((rangePtr->startAddr < prefixTypeFirstRange) ||
                    (rangePtr->startAddr > prefixTypeLastRange))
                {
                     rangePtr = rangePtr->next;
                     continue;
                }
            }
            status = cleanSwTreeFromDmmAllocDoneInHaSyncSip7(shadowPtr,
                                                           vrId,
                                                           rangePtr->lowerLpmPtr.nextBucket,
                                                           (GT_U8)(level + 1),
                                                           numOfMaxAllowedLevels,
                                                           prefixType,
                                                           protocol);
            if (status != GT_OK)
            {
                return status;
            }
        }
        rangePtr = rangePtr->next;
    }
    return status;
}

GT_STATUS cleanSwTreeFromDmmAllocDoneInHaSyncSip7
(
    IN  PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC          *shadowPtr,
    IN  GT_U32                                    vrId,
    IN  PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC   *bucketPtr,
    IN  GT_U8                                     level,
    IN  GT_U8                                     numOfMaxAllowedLevels,
    IN  CPSS_UNICAST_MULTICAST_ENT                prefixType,
    IN  PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT      protocol
)
{
    GT_U32    j;
    GT_STATUS status = GT_OK;
    GT_U32    blockIndex=0;

    /* Check that we don't exceed the number of allowed levels */
    if (level >= numOfMaxAllowedLevels)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Error: exceeded max number of levels\n");
    }

    /* deal with root bucket or in a root SRC case */
    if(bucketPtr->hwBucketOffsetHandle!=0)
    {
        if(DMM_BLOCK_STATUS_SIP7(bucketPtr->hwBucketOffsetHandle)==DMM_BLOCK_FREE_SIP7)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Error: unexpected bucketPtr->hwBucketOffsetHandle is set as free \n");
        }

        blockIndex = PRV_CPSS_DXCH_LPM_RAM_GET_SIP7_LPM_OFFSET_FROM_DMM_MAC(bucketPtr->hwBucketOffsetHandle) / shadowPtr->lpmRamTotalBlocksSizeIncludingGap;
        status = prvCpssDxChLpmRamSip7CalcBankNumberIndex(shadowPtr,&blockIndex);
        if (status != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "ERROR:illegal blockIndex - fall in holes \n");
        }

        /* free DMM that was set during HA sync operation that failed */
        shadowPtr->pendingBlockToUpdateArr[blockIndex].updateDec=GT_TRUE;
        shadowPtr->pendingBlockToUpdateArr[blockIndex].numOfDecUpdates += PRV_CPSS_DXCH_LPM_RAM_GET_SIP7_LPM_SIZE_FROM_DMM_MAC(bucketPtr->hwBucketOffsetHandle);

        prvCpssDmmFreeSip7(bucketPtr->hwBucketOffsetHandle);
        bucketPtr->hwBucketOffsetHandle = 0;
    }

    /* nodeMemAddr hold the node address - reset the value */
    bucketPtr->nodeMemAddr=0;

    /* reset pointingRangeMemAddr */
    bucketPtr->pointingRangeMemAddr = 0xffffffff;

    if (bucketPtr->bucketType != CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E)
    {
       for (j=0;j<PRV_CPSS_DXCH_LPM_RAM_FALCON_MAX_SIZE_OF_BUCKET_IN_LPM_LINES_CNS;j++)
        {
            /* in case of regular bit vector we will have 6 pointers (hwBucketOffsetHandle[6])*/
            if (bucketPtr->hwGroupOffsetHandle[j]!=0)
            {
                if(DMM_BLOCK_STATUS_SIP7(bucketPtr->hwGroupOffsetHandle[j])==DMM_BLOCK_FREE_SIP7)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Error: unexpected bucketPtr->hwGroupOffsetHandle[j] is set as free \n");
                }

                /* free DMM that was set during HA sync operation that failed */
                blockIndex = PRV_CPSS_DXCH_LPM_RAM_GET_SIP7_LPM_OFFSET_FROM_DMM_MAC(bucketPtr->hwGroupOffsetHandle[j])/(shadowPtr->lpmRamTotalBlocksSizeIncludingGap);
                status = prvCpssDxChLpmRamSip7CalcBankNumberIndex(shadowPtr,&blockIndex);
                if (status != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "ERROR:illegal blockIndex - fall in holes \n");
                }

                shadowPtr->pendingBlockToUpdateArr[blockIndex].updateDec=GT_TRUE;
                shadowPtr->pendingBlockToUpdateArr[blockIndex].numOfDecUpdates += PRV_CPSS_DXCH_LPM_RAM_GET_SIP7_LPM_SIZE_FROM_DMM_MAC(bucketPtr->hwGroupOffsetHandle[j]);

                prvCpssDmmFreeSip7(bucketPtr->hwGroupOffsetHandle[j]);
                bucketPtr->hwGroupOffsetHandle[j]=0;
            }
        }
    }

    if (bucketPtr->bucketType != CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E)
    {
        /* Ranges clean */
        status = cleanSwTreeRangesFromDmmAllocDoneInHaSyncSip7(shadowPtr,
                                                           vrId,
                                                           bucketPtr,
                                                           level,
                                                           numOfMaxAllowedLevels,
                                                           prefixType,
                                                           protocol);
    }

    return status;
}

/**
* @internal prvCpssDxChIpLpmCleanSwAllocationsPerProtocolAndPrefixTypeSip7 function
* @endinternal
*
* @brief   clean the SW search structures accroding to the parameters passed
*
* @param[in] shadowPtr                - the shadow relevant for the devices asked to act on.
* @param[in] vrId                     - virtual router ID to work on.
* @param[in] protocolStack            - protocol to work on.
* @param[in] prefixType               - prefix type to work on (MC or UC)
*
*
* @retval GT_OK                    - on success, or
* @retval GT_ERROR                 - If the vrId was not created yet, or
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS prvCpssDxChIpLpmCleanSwAllocationsPerProtocolAndPrefixTypeSip7
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC             *shadowPtr,
    IN GT_U32                                       vrId,
    IN PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT         protocolStack,
    IN CPSS_UNICAST_MULTICAST_ENT                   prefixType
)
{
    GT_STATUS                                       retVal = GT_OK;
    GT_U32                                          numOfMaxAllowedLevels;
    PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC         *bucketPtr;

    if (prefixType==CPSS_UNICAST_MULTICAST_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "ERROR:in prvCpssDxChIpLpmCleanSwAllocationsPerProtocolAndPrefixType, "
                                                    "prefixType can only be UC or MC \n");
    }

   /* go over valid VR , if it's initilized for this protocol */
    if ((shadowPtr->vrRootBucketArray[vrId].valid == GT_FALSE) ||
        (shadowPtr->vrRootBucketArray[vrId].rootBucket[protocolStack] == NULL))
    {
        /* nothing to clean -
           no SW allocations were done for this VRF and protocol */
        return GT_OK;
    }

    switch (protocolStack)
    {
        case PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E:
            numOfMaxAllowedLevels=PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_FOR_GON_IN_IPV4_PROTOCOL_CNS;
            break;
        case PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E:
            numOfMaxAllowedLevels=PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_FOR_GON_IN_IPV6_PROTOCOL_CNS;
            break;
        case PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E:
            numOfMaxAllowedLevels=PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_FOR_GON_IN_FCOE_PROTOCOL_CNS;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "ERROR:in prvCpssDxChIpLpmCleanSwAllocationsPerProtocolAndPrefixType, "
                                                        "not valid protocolStack \n");
    }
    if (shadowPtr->vrRootBucketArray[vrId].rootBucket[protocolStack])
    {
        bucketPtr = shadowPtr->vrRootBucketArray[vrId].rootBucket[protocolStack];

        if (prefixType==CPSS_MULTICAST_E)
        {
           numOfMaxAllowedLevels = 2*numOfMaxAllowedLevels;
        }

        retVal = cleanSwTreeFromDmmAllocDoneInHaSyncSip7(shadowPtr,
                                                       vrId,
                                                       bucketPtr,
                                                       0,
                                                       numOfMaxAllowedLevels,
                                                       prefixType,
                                                       protocolStack);
        if (retVal != GT_OK)
        {
           CPSS_LOG_ERROR_AND_RETURN_MAC(retVal, "ERROR:in prvCpssDxChIpLpmCleanSwAllocationsPerProtocolAndPrefixType, "
                                                        "fail cleanSwTreeFromDmmAllocDoneInHaSync \n");
        }
    }
    return retVal;
}

/**
* @internal prvCpssDxChIpLpmUpdateAndCleanCountersAfterHaSyncFailSip7 function
* @endinternal
*
* @brief   clean and update counters and blocks mapping after SW
*          was cleaned
*
* @param[in] shadowPtr                - the shadow relevant for the devices asked to act on.
*
* @retval GT_OK                    - on success, or
* @retval GT_ERROR                 - If the vrId was not created yet, or
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS prvCpssDxChIpLpmUpdateAndCleanCountersAfterHaSyncFailSip7
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC             *shadowPtr
)
{
    GT_STATUS                                   rc = GT_OK;
    PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT        protocolStack;
    GT_U32                                      firstBlockIndex=0;
    GT_U32                                      bankIndex=0;
    GT_U32                                      numOfMaxAllowedLevels;
    GT_U32                                      octetIndex=0;

    /* if the bank is marked as used we need to check if it is empty,
       if it is we need to unbind it from current "octet to bank mapping" and reuse it
       bank0 should never be released. For all the protocols octet 0 is mapped to bank 0.*/

    /* go over all protocols */
    for (protocolStack = PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E;
         protocolStack < PRV_CPSS_DXCH_LPM_PROTOCOL_LAST_E;
         protocolStack++)
    {
        switch (protocolStack)
        {
            case PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E:
                numOfMaxAllowedLevels=PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_FOR_GON_IN_IPV4_PROTOCOL_CNS;
                break;
            case PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E:
                numOfMaxAllowedLevels=PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_FOR_GON_IN_IPV6_PROTOCOL_CNS;
                break;
            case PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E:
                numOfMaxAllowedLevels=PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_FOR_GON_IN_FCOE_PROTOCOL_CNS;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "ERROR:in prvCpssDxChIpLpmUpdateAndCleanCountersAfterHaSyncFail, "
                                                            "not valid protocolStack \n");
        }

        /* octet0 mapping*/
        firstBlockIndex = shadowPtr->lpmMemInfoArray[protocolStack][0].ramIndex;

        for (bankIndex=0; bankIndex < shadowPtr->numOfLpmMemories; bankIndex++)
        {
            if (bankIndex==firstBlockIndex)
            {
                /* do not release the mapping */
                if (PRV_CPSS_DXCH_LPM_RAM_OCTET_TO_BLOCK_MAPPING_3_OCT_PER_BANK_MAC(shadowPtr))
                {
                    /* when multiple octets are binded to the block, unbind all octets except octet 0 */
                    rc = prvCpssDxChLpmRamMngUnbindFirstBlockSip7(shadowPtr, protocolStack, bankIndex);
                    if (rc!=GT_OK)
                    {
                        return rc;
                    }
                }
            }
            else if (PRV_CPSS_DXCH_LPM_RAM_IS_BLOCK_USED_BY_PROTOCOL_GET_MAC(shadowPtr,protocolStack,bankIndex)==GT_TRUE)
            {
                /* unbind the block from the list */
                rc = prvCpssDxChLpmRamMngUnbindBlockSip7(shadowPtr, bankIndex);
                if (rc!=GT_OK)
                {
                    return rc;
                }

                /* since in this case we can have a case that due to the fail we had in the HA sync we are left with
                   incorrect lpmRamOctetsToBlockMappingPtr, and tempLpmRamOctetsToBlockMappingUsedForReconstractPtr
                   we will clean all SW mapping */

                /* go over all octets and set them as not used by the bank */
                for (octetIndex=0;octetIndex<numOfMaxAllowedLevels;octetIndex++)
                {
                    /* set the block as not used */
                    PRV_CPSS_DXCH_LPM_RAM_OCTET_TO_BLOCK_MAPPING_CLEAR_MAC(shadowPtr, protocolStack, octetIndex, bankIndex);
                    /* the blocks that was freed should stay free even if we get an error in the next phases so we update
                       tempLpmRamOctetsToBlockMappingUsedForReconstractPtr to be used in case reconstruct is needed,*/
                    PRV_CPSS_DXCH_TEMP_LPM_RAM_OCTET_TO_BLOCK_MAPPING_CLEAR_MAC(shadowPtr, protocolStack, octetIndex, bankIndex);

                    /* if the block is not used by any protocol set it as not used */
                    if(((PRV_CPSS_DXCH_LPM_RAM_IS_BLOCK_USED_BY_PROTOCOL_GET_MAC(shadowPtr,PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E,bankIndex)==GT_FALSE))&&
                        ((PRV_CPSS_DXCH_LPM_RAM_IS_BLOCK_USED_BY_PROTOCOL_GET_MAC(shadowPtr,PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E,bankIndex)==GT_FALSE))&&
                        ((PRV_CPSS_DXCH_LPM_RAM_IS_BLOCK_USED_BY_PROTOCOL_GET_MAC(shadowPtr,PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E,bankIndex)==GT_FALSE)))
                    {
                        shadowPtr->lpmRamOctetsToBlockMappingPtr[bankIndex].isBlockUsed = GT_FALSE;
                        shadowPtr->tempLpmRamOctetsToBlockMappingUsedForReconstractPtr[bankIndex].isBlockUsed = GT_FALSE;
                    }
                }
            }
        }
        /* free the allocated/bound RAM memory */
        rc = prvCpssDxChLpmRamMngAllocatedAndBoundMemFreeSip7(shadowPtr,protocolStack);
        if (rc!=GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "ERROR:in prvCpssDxChIpLpmUpdateAndCleanCountersAfterHaSyncFail, "
                                                                "fail in prvCpssDxChLpmRamMngAllocatedAndBoundMemFree \n");
        }
    }
    return rc;
}

/**
* @internal prvCpssDxChLpmCleanSwAfterHaSyncFailedSip7 function
* @endinternal
*
* @brief  Clean all SW allocations done in the sync process
*         Relevant for HA process
*
* @note   APPLICABLE DEVICES:      AAS;
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                  Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] shadowPtr             - the shadow to work on.
*
* @retval GT_OK                    - if success, or
* @retval GT_OUT_OF_CPU_MEM        - if failed to allocate CPU memory, or
* @retval GT_OUT_OF_PP_MEM         - if failed to allocate PP memory, or
* @retval GT_NOT_APPLICABLE_DEVICE - if not applicable device, or
* @retval GT_FAIL                  - otherwise.
*
*/
GT_STATUS prvCpssDxChLpmCleanSwAfterHaSyncFailedSip7
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC     *shadowPtr
)
{
    GT_STATUS                                   rc = GT_OK,retVal=GT_OK;
    GT_U32                                      vrfId = 0;
    PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT        protocolStack;
    CPSS_UNICAST_MULTICAST_ENT                  prefixType;
    GT_U32                                      devNum=0;

    /* for all valid devices we need reset the lastBankIdConfigured */
    for (devNum = 0; devNum < PRV_CPSS_MAX_PP_DEVICES_CNS; devNum++)
    {
        if(PRV_CPSS_IS_DEV_EXISTS_MAC(devNum))
        {
            /* default value mean: parameter was not configured yet */
            PRV_CPSS_DXCH_PP_MAC(devNum)->moduleCfg.ip.lastBankIdConfigured = 0xFFF;
        }
    }

    /* Clean all allocations done in the sync process, those allocations were kept in the shadow,
       including: dmm allocations, Octet to bank mapping allocations, counters updates */

    /* go over all protocols */
    for (protocolStack = PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E;
         protocolStack < PRV_CPSS_DXCH_LPM_PROTOCOL_LAST_E;
         protocolStack++)
    {
        /*Go over all 4K virtual routers */
        for (vrfId = 0 ; vrfId < BIT_12 ; vrfId++)
        {
            if (shadowPtr->vrRootBucketArray[vrfId].valid == GT_TRUE)
            {
                if (shadowPtr->vrRootBucketArray[vrfId].rootBucket[protocolStack] != NULL)
                {
                    rc = prvCpssDxChLpmRamMemFreeListMngSip7(0,PRV_CPSS_DXCH_LPM_RAM_MEM_LIST_RESET_OP_E,
                                              &shadowPtr->freeMemListDuringUpdate,shadowPtr);
                    if (rc != GT_OK)
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "ERROR:in prvCpssDxChIpLpmCleanSwAllocationsPerProtocolAndPrefixType, "
                                                                 "fail in prvCpssDxChLpmRamMemFreeListMng \n");
                    }

                    for (prefixType=CPSS_UNICAST_E;prefixType<CPSS_UNICAST_MULTICAST_E;prefixType++)
                    {
                        if(((prefixType==CPSS_UNICAST_E)&&
                            (shadowPtr->vrRootBucketArray[vrfId].isUnicastSupported[protocolStack] == GT_TRUE))||
                           ((prefixType==CPSS_MULTICAST_E)&&(protocolStack!=PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E)&&
                            (shadowPtr->vrRootBucketArray[vrfId].isMulticastSupported[protocolStack] == GT_TRUE)))
                        {
                            /* go over all UC/MC ranges and free DMM allocations */
                            rc = prvCpssDxChIpLpmCleanSwAllocationsPerProtocolAndPrefixTypeSip7(shadowPtr,
                                                                                            vrfId,
                                                                                            protocolStack,
                                                                                            prefixType);
                            if (rc != GT_OK)
                            {
                                break;
                            }
                        }
                    }

                    /* DMM memory of the root handle should be already freed  */
                    if ((rc != GT_OK)||
                        (shadowPtr->vrRootBucketArray[vrfId].rootBucket[protocolStack]->hwBucketOffsetHandle!=0))
                    {
                        /* reset the array for next use */
                        cpssOsMemSet(shadowPtr->globalMemoryBlockTakenArr, 0, sizeof(shadowPtr->globalMemoryBlockTakenArr));
                         /* reset pending array for future use */
                        cpssOsMemSet(shadowPtr->pendingBlockToUpdateArr,0,sizeof(shadowPtr->pendingBlockToUpdateArr));

                        retVal = prvCpssDxChLpmRamMemFreeListMngSip7(0,PRV_CPSS_DXCH_LPM_RAM_MEM_LIST_FREE_MEM_OP_E,
                                         &shadowPtr->freeMemListDuringUpdate,shadowPtr);

                        if (retVal != GT_OK)
                        {
                            CPSS_LOG_ERROR_AND_RETURN_MAC(retVal, "ERROR:in prvCpssDxChIpLpmCleanSwAllocationsPerProtocolAndPrefixType, "
                                                                   "fail prvCpssDxChLpmRamMemFreeListMng \n");
                        }

                        retVal = prvCpssDxChLpmRamMemFreeListMngSip7(0,PRV_CPSS_DXCH_LPM_RAM_MEM_LIST_FREE_MEM_OP_E,
                                                                 &shadowPtr->freeMemListEndOfUpdate,shadowPtr);

                        if (retVal != GT_OK)
                        {
                            CPSS_LOG_ERROR_AND_RETURN_MAC(retVal, "ERROR:in prvCpssDxChIpLpmCleanSwAllocationsPerProtocolAndPrefixType, "
                                                                   "fail prvCpssDxChLpmRamMemFreeListMng \n");
                        }

                        /* DMM memory of the root handle should be already freed  */
                        if(shadowPtr->vrRootBucketArray[vrfId].rootBucket[protocolStack]->hwBucketOffsetHandle!=0)
                        {
                            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "ERROR:DMM memory of the root handle was already freed\n");
                        }
                        else
                        {
                            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "ERROR:in cleanSwAllocationsPerProtocolAndPrefixType \n");
                        }
                    }
                }
                /* now it's possible to free all the "end of update to be freed
                   memory" memory , now that the HW is updated (and the "during
                   update to be freed" memory if needed ) */
                rc = prvCpssDxChLpmRamMemFreeListMngSip7(0,PRV_CPSS_DXCH_LPM_RAM_MEM_LIST_FREE_MEM_OP_E,
                                                         &shadowPtr->freeMemListDuringUpdate,shadowPtr);

                if (rc != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "ERROR:in prvCpssDxChIpLpmCleanSwAllocationsPerProtocolAndPrefixType, "
                                                           "fail prvCpssDxChLpmRamMemFreeListMng \n");
                }

                rc = prvCpssDxChLpmRamMemFreeListMngSip7(0,PRV_CPSS_DXCH_LPM_RAM_MEM_LIST_FREE_MEM_OP_E,
                                                         &shadowPtr->freeMemListEndOfUpdate,shadowPtr);

                if (rc != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "ERROR:in prvCpssDxChIpLpmCleanSwAllocationsPerProtocolAndPrefixType, "
                                                           "fail prvCpssDxChLpmRamMemFreeListMng \n");
                }
            }
        }
    }

    /* reset pending array
       prvCpssDxChLpmRamSyncSwHwForHa failed so we will start from the
       beggining the allocations and counters calculations
       no need at the clean stage to update the counters decremented
       we should only update the counters after allocation done in next phase in the call to
       prvCpssDxChLpmAllocateSwAfterHaSyncFailed */
    cpssOsMemSet(shadowPtr->pendingBlockToUpdateArr,0,sizeof(shadowPtr->pendingBlockToUpdateArr));

    /* release banks from its mapping */
    rc = prvCpssDxChIpLpmUpdateAndCleanCountersAfterHaSyncFailSip7(shadowPtr);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "ERROR:in prvCpssDxChLpmCleanSwAfterHaSyncFailed, "
                                               "fail prvCpssDxChIpLpmUpdateAndCleanCountersAfterHaSyncFail \n");
    }

    return rc;
}

/**
* @internal allocateSwTreeRangesFromDmmAllocDoneInHaSyncSip7 function
* @endinternal
*
* @brief   allocate SW Shadow DMM memory
*          - operation done on a range
*
* @param[in] shadowPtr                - pointer to shadow structure
* @param[in] vrId                     - the virtual router id
* @param[in] bucketPtr                - pointer to the bucket
* @param[in] level                    - the level in the tree (first  is 0)
* @param[in] numOfMaxAllowedLevels    - the maximal number of levels that is allowed
*                                      for the relevant protocol and prefix type
* @param[in] prefixType               - unicast or multicast tree
* @param[in] protocol                 - protocol
*
* @retval GT_FAIL                  - on failure
* @retval GT_OK                    - on success
*/
GT_STATUS allocateSwTreeRangesFromDmmAllocDoneInHaSyncSip7
(
    IN  PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC          *shadowPtr,
    IN  GT_U32                                    vrId,
    IN  PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC   *bucketPtr,
    IN  GT_U8                                     level,
    IN  GT_U8                                     numOfMaxAllowedLevels,
    IN  CPSS_UNICAST_MULTICAST_ENT                prefixType,
    IN  PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT      protocol
)
{
    GT_STATUS                               status = GT_OK;
    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC  *rangePtr;
    GT_U8                                   prefixTypeFirstRange, prefixTypeLastRange;
    GT_U8                                   prefixTypeSecondRange=0, prefixTypeSecondLastRange=0;
    GT_U32                                  tmpPrefixTypeRange;

    rangePtr = bucketPtr->rangeList;

    if (level == 0)
    {
        if (prefixType == CPSS_UNICAST_E)
        {
            prefixTypeFirstRange = 0;
            tmpPrefixTypeRange = (protocol == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E) ?
                (PRV_CPSS_DXCH_LPM_RAM_START_OF_IPV4_MC_ADDRESS_SPACE_CNS - 1) :
                (PRV_CPSS_DXCH_LPM_RAM_START_OF_IPV6_MC_ADDRESS_SPACE_CNS - 1);
            prefixTypeLastRange = (GT_U8)tmpPrefixTypeRange;

            if (protocol == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E)
            {
                prefixTypeSecondRange = PRV_CPSS_DXCH_LPM_RAM_START_OF_IPV4_RESERVED_SPACE_ADDRESS_SPACE_CNS;
                prefixTypeSecondLastRange = PRV_CPSS_DXCH_LPM_RAM_END_OF_IPV4_RESERVED_SPACE_ADDRESS_SPACE_CNS;
            }
        }
        else /* CPSS_MULTICAST_E */
        {
            tmpPrefixTypeRange = (protocol == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E) ?
                PRV_CPSS_DXCH_LPM_RAM_START_OF_IPV4_MC_ADDRESS_SPACE_CNS :
                PRV_CPSS_DXCH_LPM_RAM_START_OF_IPV6_MC_ADDRESS_SPACE_CNS;
            prefixTypeFirstRange = (GT_U8)tmpPrefixTypeRange;
            tmpPrefixTypeRange = (protocol == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E) ?
                PRV_CPSS_DXCH_LPM_RAM_END_OF_IPV4_MC_ADDRESS_SPACE_CNS :
                PRV_CPSS_DXCH_LPM_RAM_END_OF_IPV6_MC_ADDRESS_SPACE_CNS;
            prefixTypeLastRange = (GT_U8)tmpPrefixTypeRange;

            prefixTypeSecondLastRange = prefixTypeLastRange;
        }
    }
    else
    {
        prefixTypeFirstRange = 0;
        prefixTypeLastRange = 255;
        prefixTypeSecondRange = 0;
        prefixTypeSecondLastRange = 255;

    }

    /* Check next buckets */
    rangePtr = bucketPtr->rangeList;

    while (rangePtr)
    {
        if ((rangePtr->pointerType != CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E) &&
            (rangePtr->pointerType != CPSS_DXCH_LPM_MULTIPATH_ENTRY_PTR_TYPE_E))
        {
            if (protocol == PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E)
            {
                if ((rangePtr->startAddr < prefixTypeFirstRange) ||
                    ((rangePtr->startAddr > prefixTypeLastRange) && (rangePtr->startAddr<prefixTypeSecondRange))||
                     (rangePtr->startAddr > prefixTypeSecondLastRange))
                {
                    rangePtr = rangePtr->next;
                    continue;
                }
            }
            else
            {
                if ((rangePtr->startAddr < prefixTypeFirstRange) ||
                    (rangePtr->startAddr > prefixTypeLastRange))
                {
                     rangePtr = rangePtr->next;
                     continue;
                }
            }
            status = allocateSwTreeFromDmmAllocDoneInHaSyncSip7(shadowPtr,
                                                           vrId,
                                                           rangePtr->lowerLpmPtr.nextBucket,
                                                           (GT_U8)(level + 1),
                                                           numOfMaxAllowedLevels,
                                                           prefixType,
                                                           protocol);
            if (status != GT_OK)
            {
                return status;
            }
        }
        rangePtr = rangePtr->next;
    }
    return status;
}

GT_STATUS allocateSwTreeFromDmmAllocDoneInHaSyncSip7
(
    IN  PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC          *shadowPtr,
    IN  GT_U32                                    vrId,
    IN  PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC   *bucketPtr,
    IN  GT_U8                                     level,
    IN  GT_U8                                     numOfMaxAllowedLevels,
    IN  CPSS_UNICAST_MULTICAST_ENT                prefixType,
    IN  PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT      protocol
)
{
    GT_U32    j;
    GT_STATUS status = GT_OK;
    GT_STATUS retVal2=GT_OK;
    GT_U32    blockIndex=0;
    GT_U32    subnodesSizes[PRV_CPSS_DXCH_LPM_RAM_BIT_VECTOR_LINES_NUMBER_CNS] = {0};
    PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC **lpmEngineMemPtrPtr;
    PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC *tempNextMemInfoPtr;
    GT_U32      newFreeBlockIndex; /* index of a new free block */
    GT_UINTPTR  tempNewStructsMemPool=0;
    PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC  **allNewNextMemInfoAllocatedPerOctetArrayPtr;/*(pointer to) an array that holds for each octet
                                                                                      the allocted new element that need to be freed.
                                                                                      Size of the array is 16 for case of IPV6 */
    PRV_CPSS_DXCH_LPM_RAM_MEM_INFO_STC  *firstMemInfoInListToFreePtr=NULL;/* use for going over the list of blocks per octet */

    GT_BOOL     justBindAndDontAllocateFreePoolMem = GT_FALSE;
    GT_U32      octetNum=0;
    GT_U32      gonOctetNum=0;
    GT_BOOL     isSrcRoot=GT_FALSE, isRoot=GT_FALSE;
    GT_U32      memSize=0;

    switch(protocol)
    {
        case PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E:
        case PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E:
        case PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E:
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Error: ilegal protocol \n");

    }

    /* find the octetNum according to level/protocol/type
       and set correct lpmEngineMemPtrPtr to work on */
    if (prefixType == CPSS_UNICAST_E)
    {
        /* Check that we don't exceed the number of allowed levels*/
        if (level >= numOfMaxAllowedLevels)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Error: exceeded max number of levels\n");
        }

        lpmEngineMemPtrPtr = shadowPtr->ucSearchMemArrayPtr[protocol];

        octetNum = level;
        if (level!=0)/* non root level */
        {
            octetNum = level+1;
            gonOctetNum = octetNum;

            isSrcRoot = GT_FALSE;
            isRoot    = GT_FALSE;
        }
        else
        {
            gonOctetNum = octetNum+1;
            isSrcRoot = GT_FALSE;
            isRoot    = GT_TRUE;
        }
    }
    else
    {
        switch(protocol)
        {
        case PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E:
            octetNum = level % PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS;
            if ((level!=0)&& (level != PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS))/* non root Grp/Src */
            {
                octetNum = octetNum+1;
                gonOctetNum = octetNum;
            }
            if (level == PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV4_PROTOCOL_CNS)
            {
                gonOctetNum = octetNum+1;
                isSrcRoot = GT_TRUE;
                isRoot    = GT_FALSE;
            }
            else
            {
                if(level==0)
                {
                    gonOctetNum = octetNum+1;
                    isSrcRoot = GT_FALSE;
                    isRoot    = GT_TRUE;
                }
                else
                {
                    gonOctetNum = octetNum;
                    isSrcRoot = GT_FALSE;
                    isRoot    = GT_FALSE;
                }
            }
            break;
        case PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E:
            octetNum = level % PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS;
            if ((level!=0)&& (level != PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS))/* non root Grp/Src */
            {
                octetNum = octetNum+1;
				gonOctetNum = octetNum;
            }
            if (level == PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_IN_IPV6_PROTOCOL_CNS)
            {
                gonOctetNum = octetNum+1;
                isSrcRoot = GT_TRUE;
                isRoot    = GT_FALSE;
            }
            else
            {
                if(level==0)
                {
                    gonOctetNum = octetNum+1;
                    isSrcRoot = GT_FALSE;
                    isRoot    = GT_TRUE;
                }
                else
                {
                    gonOctetNum = octetNum;
                    isSrcRoot = GT_FALSE;
                    isRoot    = GT_FALSE;
                }
            }
            break;
        case PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E:/* do not support mulicast*/
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);

        }
        lpmEngineMemPtrPtr = shadowPtr->mcSearchMemArrayPtr[protocol];
    }

    allNewNextMemInfoAllocatedPerOctetArrayPtr = shadowPtr->allNewNextMemInfoAllocatedPerOctetArrayPtr;

    if (bucketPtr->bucketType != CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E)
    {
        /* deal with root bucket or in a root SRC case */
        if((isSrcRoot == GT_TRUE) || (isRoot == GT_TRUE))
        {
            if(bucketPtr->hwBucketOffsetHandle==0)
            {
                if (bucketPtr->bucketType==CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E)
                {
                    memSize = PRV_CPSS_DXCH_LPM_RAM_FALCON_BUCKET_SIZE_REGULAR_BV_CNS;
                }
                else
                {
                    memSize = PRV_CPSS_DXCH_LPM_RAM_FALCON_BUCKET_SIZE_COMPRESSED_CNS;
                }

                 /* allocate memory for root bucket */
                status = prvCpssDxChLpmRamMngHandleRootNodeAllocationSip7(lpmEngineMemPtrPtr[octetNum],
                                                                      memSize,
                                                                      DMM_MIN_ALLOCATE_SIZE_IN_BYTE_SIP7_CNS,
                                                                      protocol,
                                                                      octetNum,
                                                                      shadowPtr,
                                                                      &bucketPtr->hwBucketOffsetHandle);
                if (status != GT_OK)
                {
                    /* free the allocated/bound RAM memory */
                    retVal2 = prvCpssDxChLpmRamMngAllocatedAndBoundMemFreeSip7(shadowPtr,protocol);
                    if (retVal2!=GT_OK)
                    {
                        return retVal2;
                    }

                    return status;
                }
                else
                {
                    if (bucketPtr->hwBucketOffsetHandle!=0)
                    {
                        SET_DMM_BLOCK_PROTOCOL_SIP7(bucketPtr->hwBucketOffsetHandle, protocol);
                        SET_DMM_BUCKET_SW_ADDRESS_SIP7(bucketPtr->hwBucketOffsetHandle, bucketPtr);
                    }
                    else
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "after prvCpssDxChLpmRamMngHandleRootNodeAllocationSip7 "
                                                               "pass ok we should have a valid bucketPtr->hwBucketOffsetHandle different from 0 ");
                    }

                    /* set root handle */
                    if (isRoot == GT_TRUE)
                    {
                        bucketPtr->nodeMemAddr = vrId;
                        bucketPtr->pointingRangeMemAddr = 0xffffffff;
                    }
                }
            }
        }

        /* Handle the GONs pointed by this bucket */

        /* need to find what is the size for each GON handle */
        status = lpmGetStaticGonsSizesSip7(bucketPtr,
                                         &subnodesSizes[0]);
        if (status != GT_OK)
        {
            return status;
        }

        for (j = 0; j < PRV_CPSS_DXCH_LPM_RAM_FALCON_MAX_SIZE_OF_BUCKET_IN_LPM_LINES_CNS; j++)
        {
            /* in AAS we are checking for all group of nodes */
            if (subnodesSizes[j] == 0)
            {
                continue;
            }
            if (bucketPtr->hwGroupOffsetHandle[j] == 0)
            {
                tempNextMemInfoPtr = lpmEngineMemPtrPtr[gonOctetNum]; /* the GONs are located in the next octet */
                if (lpmEngineMemPtrPtr[gonOctetNum]->structsMemPool != 0)
                {
                   bucketPtr->hwGroupOffsetHandle[j] =
                        prvCpssDmmAllocateSip7(lpmEngineMemPtrPtr[gonOctetNum]->structsMemPool,
                                           DMM_MIN_ALLOCATE_SIZE_IN_BYTE_SIP7_CNS *subnodesSizes[j],
                                           DMM_MIN_ALLOCATE_SIZE_IN_BYTE_SIP7_CNS);
                }
                else
                {
                    /* no memory pool is bound to the root of the octet list,
                       in this case no need to allocate a memory pool struct just
                       to bind it to a free pool */
                    justBindAndDontAllocateFreePoolMem = GT_TRUE;

                }

                while((tempNextMemInfoPtr->nextMemInfoPtr!= NULL)&&
                      ((bucketPtr->hwGroupOffsetHandle[j] == DMM_BLOCK_NOT_FOUND_SIP7)||
                       (bucketPtr->hwGroupOffsetHandle[j] == 0)))
                {
                    bucketPtr->hwGroupOffsetHandle[j] =
                        prvCpssDmmAllocateSip7(tempNextMemInfoPtr->nextMemInfoPtr->structsMemPool,
                                           DMM_MIN_ALLOCATE_SIZE_IN_BYTE_SIP7_CNS * subnodesSizes[j],
                                           DMM_MIN_ALLOCATE_SIZE_IN_BYTE_SIP7_CNS);
                    tempNextMemInfoPtr = tempNextMemInfoPtr->nextMemInfoPtr;
                }

                /* could not find an empty space in the current bound blocks - continue looking */
                if ((tempNextMemInfoPtr->nextMemInfoPtr==NULL)&&
                    ((bucketPtr->hwGroupOffsetHandle[j]==DMM_BLOCK_NOT_FOUND_SIP7)||
                     (bucketPtr->hwGroupOffsetHandle[j]==0)))
                {
                    status = findAndBindValidMemoryBlockSip7(shadowPtr, protocol, (gonOctetNum),
                                                         &bucketPtr->hwGroupOffsetHandle[j],
                                                         subnodesSizes[j],
                                                         tempNextMemInfoPtr,
                                                         firstMemInfoInListToFreePtr,
                                                         &justBindAndDontAllocateFreePoolMem,
                                                         allNewNextMemInfoAllocatedPerOctetArrayPtr,
                                                         tempNewStructsMemPool,
                                                         &newFreeBlockIndex,
                                                         shadowPtr->globalMemoryBlockTakenArr);
                    if (status!= GT_OK)
                    {
                        /* free the allocated/bound RAM memory */
                        retVal2 = prvCpssDxChLpmRamMngAllocatedAndBoundMemFreeSip7(shadowPtr,protocol);
                        if (retVal2!=GT_OK)
                        {
                            return retVal2;
                        }
                        status = GT_OUT_OF_PP_MEM;
                        break;
                    }
                    else
                    {
                        if (bucketPtr->hwGroupOffsetHandle[j]!=0)
                        {
                            /* mark the block as used */
                            SET_DMM_BLOCK_PROTOCOL_SIP7(bucketPtr->hwGroupOffsetHandle[j], protocol);
                            SET_DMM_BUCKET_SW_ADDRESS_SIP7(bucketPtr->hwGroupOffsetHandle[j], bucketPtr);
                        }
                        else
                        {
                            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "after findAndBindValidMemoryBlock "
                                                               "pass ok we should have a valid bucketPtr->hwGroupOffsetHandle[j] different from 0 ");
                        }
                    }
                }
                else
                {
                    /* we had a CPU Memory allocation error */
                    if(bucketPtr->hwGroupOffsetHandle[j] == DMM_MALLOC_FAIL_SIP7)
                    {
                        status = GT_OUT_OF_PP_MEM;
                        break;
                    }

                    /* mark the block as used */
                     SET_DMM_BLOCK_PROTOCOL_SIP7(bucketPtr->hwGroupOffsetHandle[j], protocol);
                     SET_DMM_BUCKET_SW_ADDRESS_SIP7(bucketPtr->hwGroupOffsetHandle[j], bucketPtr);

                    /* set pending flag for future need */
                    blockIndex = PRV_CPSS_DXCH_LPM_RAM_GET_SIP7_LPM_OFFSET_FROM_DMM_MAC(bucketPtr->hwGroupOffsetHandle[j])/(shadowPtr->lpmRamTotalBlocksSizeIncludingGap);
                    status = prvCpssDxChLpmRamSip7CalcBankNumberIndex(shadowPtr,&blockIndex);
                    if (status != GT_OK)
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "ERROR:illegal blockIndex - fall in holes \n");
                    }
                    shadowPtr->pendingBlockToUpdateArr[blockIndex].updateInc=GT_TRUE;
                    shadowPtr->pendingBlockToUpdateArr[blockIndex].numOfIncUpdates +=
                        PRV_CPSS_DXCH_LPM_RAM_GET_SIP7_LPM_SIZE_FROM_DMM_MAC(bucketPtr->hwGroupOffsetHandle[j]);
                }
            }
        }
        if (status!= GT_OK)
        {
            return status;
        }
    }

    if (bucketPtr->bucketType != CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E)
    {
        /* Ranges allocate */
        status = allocateSwTreeRangesFromDmmAllocDoneInHaSyncSip7(shadowPtr,
                                                           vrId,
                                                           bucketPtr,
                                                           level,
                                                           numOfMaxAllowedLevels,
                                                           prefixType,
                                                           protocol);
    }
    return status;
}

/**
* @internal prvCpssDxChIpLpmAllocateSwAllocationsPerProtocolAndPrefixTypeSip7 function
* @endinternal
*
* @brief   allocate the SW DMM memory accroding to the
*          parameters passed
*
* @param[in] shadowPtr                - the shadow relevant for the devices asked to act on.
* @param[in] vrId                     - virtual router ID to work on.
* @param[in] protocolStack            - protocol to work on.
* @param[in] prefixType               - prefix type to work on (MC or UC)
*
* @retval GT_OK                    - on success, or
* @retval GT_ERROR                 - If the vrId was not created yet, or
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS prvCpssDxChIpLpmAllocateSwAllocationsPerProtocolAndPrefixTypeSip7
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC             *shadowPtr,
    IN GT_U32                                       vrId,
    IN PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT         protocolStack,
    IN CPSS_UNICAST_MULTICAST_ENT                   prefixType
)
{
    GT_STATUS                                       retVal = GT_OK;
    GT_U32                                          numOfMaxAllowedLevels;
    PRV_CPSS_DXCH_LPM_RAM_BUCKET_SHADOW_STC         *bucketPtr;

    if (prefixType==CPSS_UNICAST_MULTICAST_E)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "ERROR:in prvCpssDxChIpLpmCleanSwAllocationsPerProtocolAndPrefixType, "
                                                    "prefixType can only be UC or MC \n");
    }

   /* go over valid VR , if it's initilized for this protocol */
    if ((shadowPtr->vrRootBucketArray[vrId].valid == GT_FALSE) ||
        (shadowPtr->vrRootBucketArray[vrId].rootBucket[protocolStack] == NULL))
    {
        /* nothing to do - no SW allocations done for this VRF and protocol */
        return GT_OK;
    }

    switch (protocolStack)
    {
        case PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E:
            numOfMaxAllowedLevels=PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_FOR_GON_IN_IPV4_PROTOCOL_CNS;
            break;
        case PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E:
            numOfMaxAllowedLevels=PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_FOR_GON_IN_IPV6_PROTOCOL_CNS;
            break;
        case PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E:
            numOfMaxAllowedLevels=PRV_CPSS_DXCH_LPM_NUM_OF_OCTETS_FOR_GON_IN_FCOE_PROTOCOL_CNS;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "ERROR:in prvCpssDxChIpLpmAllocateSwAllocationsPerProtocolAndPrefixType, "
                                                        "not valid protocolStack \n");
    }
    if (shadowPtr->vrRootBucketArray[vrId].rootBucket[protocolStack])
    {
        bucketPtr = shadowPtr->vrRootBucketArray[vrId].rootBucket[protocolStack];

        if (prefixType==CPSS_MULTICAST_E)
        {
           numOfMaxAllowedLevels = 2*numOfMaxAllowedLevels;
        }

        retVal = allocateSwTreeFromDmmAllocDoneInHaSyncSip7(shadowPtr,
                                                       vrId,
                                                       bucketPtr,
                                                       0,
                                                       numOfMaxAllowedLevels,
                                                       prefixType,
                                                       protocolStack);
        if (retVal != GT_OK)
        {
           CPSS_LOG_ERROR_AND_RETURN_MAC(retVal, "ERROR:in prvCpssDxChIpLpmAllocateSwAllocationsPerProtocolAndPrefixType, "
                                                        "fail allocateSwTreeFromDmmAllocDoneInHaSync \n");
        }
    }
    return retVal;
}

/**
* @internal prvCpssDxChLpmAllocateSwAfterHaSyncFailedSip7 function
* @endinternal
*
* @brief  allocate SW DMM memory
*         Relevant for HA process
*
* @note   APPLICABLE DEVICES:      AAS;
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                  Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] shadowPtr             - the shadow to work on.
*
* @retval GT_OK                    - if success, or
* @retval GT_OUT_OF_CPU_MEM        - if failed to allocate CPU memory, or
* @retval GT_OUT_OF_PP_MEM         - if failed to allocate PP memory, or
* @retval GT_NOT_APPLICABLE_DEVICE - if not applicable device, or
* @retval GT_FAIL                  - otherwise.
*
*/
GT_STATUS prvCpssDxChLpmAllocateSwAfterHaSyncFailedSip7
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC     *shadowPtr
)
{
    GT_STATUS                                   rc = GT_OK;
    GT_STATUS                                   retVal2 = GT_OK;
    GT_U32                                      vrfId = 0;
    PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT        protocolStack;
    GT_UINTPTR                                  vrfRootBucketHandle;
    CPSS_UNICAST_MULTICAST_ENT                  prefixType;

    /* do new allocations, those allocations is kept in the shadow,
       including: dmm allocations, Octet to bank mapping allocations, counters updates */

    /* go over all protocols */
    for (protocolStack = PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E;
         protocolStack < PRV_CPSS_DXCH_LPM_PROTOCOL_LAST_E;
         protocolStack++)
    {
        /*Go over all 4K virtual routers */
        for (vrfId = 0 ; vrfId < BIT_12 ; vrfId++)
        {
            if (shadowPtr->vrRootBucketArray[vrfId].valid == GT_TRUE)
            {
                if (shadowPtr->vrRootBucketArray[vrfId].rootBucket[protocolStack] != NULL)
                {
                    rc = prvCpssDxChLpmRamMemFreeListMngSip7(0,PRV_CPSS_DXCH_LPM_RAM_MEM_LIST_RESET_OP_E,
                                             &shadowPtr->freeMemListDuringUpdate,shadowPtr);
                    if (rc != GT_OK)
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "ERROR:in prvCpssDxChIpLpmAllocateSwAllocationsPerProtocolAndPrefixType, "
                                                                    "fail in prvCpssDxChLpmRamMemFreeListMng \n");
                    }

                    /* reset the array */
                    cpssOsMemSet(shadowPtr->globalMemoryBlockTakenArr, 0, sizeof(shadowPtr->globalMemoryBlockTakenArr));

                    vrfRootBucketHandle = shadowPtr->vrRootBucketArray[vrfId].rootBucket[protocolStack]->hwBucketOffsetHandle;

                    if (0 != vrfRootBucketHandle)
                    {
                        /* we expect the shadow to be clean */
                       CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "ERROR:in prvCpssDxChLpmAllocateSwAfterHaSyncFailed \n");
                    }

                    for (prefixType=CPSS_UNICAST_E;prefixType<CPSS_UNICAST_MULTICAST_E;prefixType++)
                    {
                        if(((prefixType==CPSS_UNICAST_E)&&
                            (shadowPtr->vrRootBucketArray[vrfId].isUnicastSupported[protocolStack] == GT_TRUE))||
                           ((prefixType==CPSS_MULTICAST_E)&&(protocolStack!=PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E)&&
                            (shadowPtr->vrRootBucketArray[vrfId].isMulticastSupported[protocolStack] == GT_TRUE)))
                        {
                            /* go over all UC/MC ranges and do DMM allocations */
                            rc = prvCpssDxChIpLpmAllocateSwAllocationsPerProtocolAndPrefixTypeSip7(shadowPtr,
                                                                                        vrfId,
                                                                                        protocolStack,
                                                                                        prefixType);
                            if (rc != GT_OK)
                            {
                                /* reset pending array for future use */
                                cpssOsMemSet(shadowPtr->pendingBlockToUpdateArr,0,sizeof(shadowPtr->pendingBlockToUpdateArr));

                                /* free the allocated/bound RAM memory */
                                retVal2 = prvCpssDxChLpmRamMngAllocatedAndBoundMemFreeSip7(shadowPtr,protocolStack);
                                if (retVal2!=GT_OK)
                                {
                                    return retVal2;
                                }
                                break;
                            }
                        }
                    }

                    /* reset array for future use */
                    cpssOsMemSet(shadowPtr->globalMemoryBlockTakenArr, 0, sizeof(shadowPtr->globalMemoryBlockTakenArr));

                    retVal2 = prvCpssDxChLpmRamMemFreeListMngSip7(0,PRV_CPSS_DXCH_LPM_RAM_MEM_LIST_FREE_MEM_OP_E,
                                                         &shadowPtr->freeMemListDuringUpdate,shadowPtr);

                    if (retVal2 != GT_OK)
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(retVal2, "ERROR:in prvCpssDxChLpmAllocateSwAfterHaSyncFailed, "
                                                               "fail prvCpssDxChLpmRamMemFreeListMng \n");
                    }

                    retVal2 = prvCpssDxChLpmRamMemFreeListMngSip7(0,PRV_CPSS_DXCH_LPM_RAM_MEM_LIST_FREE_MEM_OP_E,
                                                             &shadowPtr->freeMemListEndOfUpdate,shadowPtr);

                    if (retVal2 != GT_OK)
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(retVal2, "ERROR:in prvCpssDxChLpmAllocateSwAfterHaSyncFailed, "
                                                               "fail prvCpssDxChLpmRamMemFreeListMng \n");
                    }

                    if (rc != GT_OK)
                    {
                        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "ERROR:in prvCpssDxChLpmAllocateSwAfterHaSyncFailed \n");
                    }
                }
            }
        }

        /* update counters and reset pending blocks */
        rc = prvCpssDxChLpmRamUpdateBlockUsageCountersSip7(shadowPtr->lpmRamBlocksSizeArrayPtr,
                                                       shadowPtr->pendingBlockToUpdateArr,
                                                       shadowPtr->protocolCountersPerBlockArr,
                                                       shadowPtr->pendingBlockToUpdateArr,
                                                       protocolStack,
                                                       shadowPtr->numOfLpmMemories);
        if (rc!=GT_OK)
        {
            /* reset pending array for future use */
            cpssOsMemSet(shadowPtr->pendingBlockToUpdateArr,0,sizeof(shadowPtr->pendingBlockToUpdateArr));

            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "ERROR:in prvCpssDxChLpmAllocateSwAfterHaSyncFailed in prvCpssDxChLpmRamUpdateBlockUsageCounters \n");
        }
    }

    return rc;
}

/**
* @internal prvCpssDxChLpmUpdateHwAccordingToSwAfterHaSyncFailedSip7 function
* @endinternal
*
* @brief  Update the HW according to the existing SW shadow
*         Relevant for HA process
*
* @note   APPLICABLE DEVICES:      AAS;
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                  Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] shadowPtr             - the shadow to work on.
*
* @retval GT_OK                    - if success, or
* @retval GT_OUT_OF_CPU_MEM        - if failed to allocate CPU memory, or
* @retval GT_OUT_OF_PP_MEM         - if failed to allocate PP memory, or
* @retval GT_NOT_APPLICABLE_DEVICE - if not applicable device, or
* @retval GT_FAIL                  - otherwise.
*
*
*/
GT_STATUS prvCpssDxChLpmUpdateHwAccordingToSwAfterHaSyncFailedSip7
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC     *shadowPtr
)
{
    GT_STATUS                                       retVal = GT_OK;
    PRV_CPSS_DXCH_LPM_RAM_RANGE_SHADOW_STC          tmpRootRange;
    PRV_CPSS_DXCH_LPM_RAM_TRIE_UPDATE_TYPE_ENT      lpmTrieUpdateType;
    GT_U32                                          vrId;
    PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT            currProtocolStack;

    /* According to the existing shadow we will update the HW.
       The update will be done by going over the shadow and updating each bucket at a time.
       We need to check that allocations can be done.
       Only after DMM allocation pass ok, we update the HW and we need to bound the
       allocated space to the shadow, to update the octet to block mapping and the counters
       done in prvCpssDxChLpmAllocateSwAfterHaSyncFailed*/

    for (currProtocolStack = PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E;
          currProtocolStack < PRV_CPSS_DXCH_LPM_PROTOCOL_LAST_E;
          currProtocolStack++)
    {
        if (shadowPtr->isProtocolInitialized[currProtocolStack] == GT_FALSE)
        {
            continue;
        }
        for (vrId = 0 ; vrId < shadowPtr->vrfTblSize; vrId++)
        {
            /* go over all valid VR , if it's initilized for this protocol */
            if ((shadowPtr->vrRootBucketArray[vrId].valid == GT_FALSE) ||
                (shadowPtr->vrRootBucketArray[vrId].rootBucket[currProtocolStack] == NULL))
            {
                continue;
            }

            retVal = prvCpssDxChLpmRamMemFreeListMngSip7(0,PRV_CPSS_DXCH_LPM_RAM_MEM_LIST_RESET_OP_E,
                                                     &shadowPtr->freeMemListDuringUpdate,shadowPtr);
            if (retVal != GT_OK)
                return retVal;

            if (shadowPtr->vrRootBucketArray[vrId].rootBucket[currProtocolStack])
            {
                /* write the changes in the tree */
                lpmTrieUpdateType = PRV_CPSS_DXCH_LPM_RAM_TRIE_WRITE_HW_AND_ALLOC_MEM_E;

                tmpRootRange.lowerLpmPtr.nextBucket =
                    shadowPtr->vrRootBucketArray[vrId].rootBucket[currProtocolStack];
                tmpRootRange.pointerType =
                    (GT_U8)(shadowPtr->vrRootBucketArray[vrId].rootBucketType[currProtocolStack]);
                tmpRootRange.next = NULL;

                retVal =
                    prvCpssDxChLpmRamMngBucketTreeWriteSip7(&tmpRootRange,
                                                            shadowPtr->ucSearchMemArrayPtr[currProtocolStack],
                                                            shadowPtr,lpmTrieUpdateType,
                                                            currProtocolStack,vrId);
                if (retVal == GT_OK)
                {
                    /* now write the vr table */
                    retVal = prvCpssDxChLpmRamMngVrfEntryUpdateSip7(vrId, currProtocolStack, shadowPtr);
                    if (retVal == GT_OK)
                    {
                        /* indicate the update was done */
                        shadowPtr->vrRootBucketArray[vrId].needsHwUpdate = GT_FALSE;
                    }
                    else
                    {
                        return retVal;
                    }
                }
                else
                {
                    return retVal;
                }
            }

            retVal = prvCpssDxChLpmRamMemFreeListMngSip7(0,PRV_CPSS_DXCH_LPM_RAM_MEM_LIST_FREE_MEM_OP_E,
                                                     &shadowPtr->freeMemListDuringUpdate,shadowPtr);

            if (retVal != GT_OK)
            {
                return retVal;
            }

            retVal = prvCpssDxChLpmRamMemFreeListMngSip7(0,PRV_CPSS_DXCH_LPM_RAM_MEM_LIST_FREE_MEM_OP_E,
                                                     &shadowPtr->freeMemListEndOfUpdate,shadowPtr);

            if (retVal != GT_OK)
            {
                return retVal;
            }
        }
    }
    return (retVal);
}

/**
* @internal prvCpssDxChLpmRamCleanAndUpdateSwAndHwAfterHaSyncFailedSip7 function
* @endinternal
*
* @brief  Clean LPM HW configuration for all protocols and all VRFs
*         Clean all SW allocations done in the sync process
*         Allocate new DMM memory in the shadow, according to the current buckets
*         Update the HW according to the existing SW shadow
*         Relevant for HA process
*
* @note   APPLICABLE DEVICES:      AAS;
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;
*                                  Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] shadowPtr             - the shadow to work on.
*
* @retval GT_OK                    - if success, or
* @retval GT_OUT_OF_CPU_MEM        - if failed to allocate CPU memory, or
* @retval GT_OUT_OF_PP_MEM         - if failed to allocate PP memory, or
* @retval GT_NOT_APPLICABLE_DEVICE - if not applicable device, or
* @retval GT_FAIL                  - otherwise.
*
*
*/
GT_STATUS prvCpssDxChLpmRamCleanAndUpdateSwAndHwAfterHaSyncFailedSip7
(
    IN PRV_CPSS_DXCH_LPM_RAM_SHADOW_STC     *shadowPtr
)
{
    GT_STATUS rc = GT_OK;

    /* Clean all HW data related to all VR, by basically “cutting” the root nodes HW pointers.
       We cannot use the flush functions since flush cleans the SW and the HW */
    rc = prvCpssDxChLpmCleanHwAfterHaSyncFailedSip7(shadowPtr);
    if (rc !=0 )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "ERROR: fail in prvCpssDxChLpmRamCleanAndUpdateSwAndHwAfterHaSyncFailed, "
                                          "fail in cleaning the HW");
    }

    /* Clean all allocations done in the sync process, those allocations were kept in the shadow,
       including: dmm allocations, Octet to bank mapping allocations, counters updates */
    rc = prvCpssDxChLpmCleanSwAfterHaSyncFailedSip7(shadowPtr);
    if (rc !=0 )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "ERROR: fail in prvCpssDxChLpmRamCleanAndUpdateSwAndHwAfterHaSyncFailed, "
                                          "fail in cleaning the SW");
    }
    /* According to the existing shadow we will update the HW.
       The update will be done by going over the shadow and updating each bucket at the time.
       We need to check that allocations can be done.
       Only after DMM allocation pass ok:
       - update the HW
       - bound the allocated space to the shadow
       - update the octet to block mapping and the counters */

    /* Allocate new DMM memory in the shadow, according to the current buckets */
    rc = prvCpssDxChLpmAllocateSwAfterHaSyncFailedSip7(shadowPtr);
    if (rc !=0 )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "ERROR: fail in prvCpssDxChLpmRamCleanAndUpdateSwAndHwAfterHaSyncFailed, "
                                          "fail in allocating the SW");
    }

    /* update the HW according to current SW tree */
    rc = prvCpssDxChLpmUpdateHwAccordingToSwAfterHaSyncFailedSip7(shadowPtr);
    if (rc !=0 )
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "ERROR: fail in prvCpssDxChLpmRamCleanAndUpdateSwAndHwAfterHaSyncFailed, "
                                          "fail in updating HW according to SW");
    }

    /* return GT_OK but add a comment to the LOG */
    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "NOTE: success in overcoming a fail in the sync of HW and SW during LPM HA ");
}


