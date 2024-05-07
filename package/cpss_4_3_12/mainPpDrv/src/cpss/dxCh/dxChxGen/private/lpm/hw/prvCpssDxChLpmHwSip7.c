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
* @file prvCpssDxChLpmHwSip7.c
*
* @brief LPM HW Sip7 releated internal functions
*
* @version   1
********************************************************************************
*/

#include <cpss/dxCh/dxChxGen/private/lpm/hw/prvCpssDxChLpmHwSip7.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwTables.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/private/lpm/ram/prvCpssDxChLpmRamTypes.h>
#include <cpss/dxCh/dxChxGen/private/lpm/ram/prvCpssDxChLpmRam.h>
#include <cpss/dxCh/dxChxGen/ip/private/prvCpssDxChIp.h>

#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

/*global variables macros*/

#define PRV_SHARED_PORT_DIR_IP_LPM_HW_SRC_GLOBAL_VAR_SET(_var,_value)\
    PRV_SHARED_GLOBAL_VAR_SET(mainPpDrvMod.ipLpmDir.ipLpmHwSrc._var,_value)

#define PRV_SHARED_PORT_DIR_IP_LPM_HW_SRC_GLOBAL_VAR_GET(_var)\
    PRV_SHARED_GLOBAL_VAR_GET(mainPpDrvMod.ipLpmDir.ipLpmHwSrc._var)

/* LPM bucket - fields offsets in bits */
#define PRV_CPSS_DXCH_LPM_RAM_POINTER_TYPE_OFFSET_CNS                       0
#define PRV_CPSS_DXCH_LPM_RAM_POINT_TO_LOOKUP_1_OFFSET_CNS                  2
#define PRV_CPSS_DXCH_LPM_RAM_IPV6_MC_GROUP_SCOPE_LEVEL_OFFSET_CNS          2
#define PRV_CPSS_DXCH_LPM_RAM_UC_RPF_CHECK_OFFSET_CNS                       4
#define PRV_CPSS_DXCH_LPM_RAM_UC_SIP_SA_CHECK_MISMATCH_OFFSET_CNS           5
#define PRV_CPSS_DXCH_LPM_RAM_ACTIVITY_STATE_OFFSET_CNS                     6
#define PRV_CPSS_DXCH_LPM_RAM_NEXT_BUCKET_TYPE_OFFSET_CNS                   3
#define PRV_CPSS_DXCH_LPM_RAM_FIFTH_RANGE_OFFSET_CNS                        5
#define PRV_CPSS_DXCH_LPM_RAM_NEXT_BUCKET_POINTER_OFFSET_CNS                5
#define PRV_CPSS_DXCH_LPM_RAM_NEXTHOP_OR_ECMP_POINTER_OFFSET_CNS            7
#define PRV_CPSS_DXCH_LPM_RAM_COMPRESSED_2_NEXT_BUCKET_POINTER_OFFSET_CNS   13

/* LPM bucket - fields length in bits */
#define PRV_CPSS_DXCH_LPM_RAM_POINTER_TYPE_LENGTH_CNS                       2
#define PRV_CPSS_DXCH_LPM_RAM_POINT_TO_LOOKUP_1_LENGTH_CNS                  1
#define PRV_CPSS_DXCH_LPM_RAM_IPV6_MC_GROUP_SCOPE_LEVEL_LENGTH_CNS          2
#define PRV_CPSS_DXCH_LPM_RAM_UC_RPF_CHECK_LENGTH_CNS                       1
#define PRV_CPSS_DXCH_LPM_RAM_UC_SIP_SA_CHECK_MISMATCH_LENGTH_CNS           1
#define PRV_CPSS_DXCH_LPM_RAM_ACTIVITY_STATE_LENGTH_CNS                     1
#define PRV_CPSS_DXCH_LPM_RAM_NEXT_BUCKET_TYPE_LENGTH_CNS                   2
#define PRV_CPSS_DXCH_LPM_RAM_FIFTH_RANGE_LENGTH_CNS                        8
#define PRV_CPSS_DXCH_LPM_RAM_NEXT_BUCKET_POINTER_LENGTH_CNS                24
#define PRV_CPSS_DXCH_LPM_RAM_NEXTHOP_OR_ECMP_POINTER_LENGTH_CNS            15
#define PRV_CPSS_DXCH_LPM_RAM_COMPRESSED_2_NEXT_BUCKET_POINTER_LENGTH_CNS   19

/**
* @internal prvCpssDxChLpmHwVrfEntryWriteSip7 function
* @endinternal
*
* @brief   Write a VRF table entry
*
* @note   APPLICABLE DEVICES:      AAS
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3;
*                                  Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - the device number
* @param[in] shadowType               - the shadow type
* @param[in] vrId                     - the virtual router id
* @param[in] protocol                 - the protocol
* @param[in] nodeType                 - the type of the node
* @param[in] rootNodeAddr             - the line number of the root node
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - one of the parameters with bad value
* @retval GT_OUT_OF_RANGE          - rootNodeAddr is out of range
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on non applicable device
*
*/
GT_STATUS prvCpssDxChLpmHwVrfEntryWriteSip7
(
    IN  GT_U8                                   devNum,
    IN  GT_U32                                  vrId,
    IN  PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT    protocol,
    IN  CPSS_DXCH_LPM_NEXT_POINTER_TYPE_ENT     nodeType,
    IN  GT_U32                                  rootNodeAddr
)
{
    /* in sip7 we have one table that holds ipv4/ipv6/fcoe
       instae of 3 tables we had in sip6 */

    CPSS_DXCH_TABLE_ENT         tableType;
    GT_U32                      data[3];
    GT_STATUS                   rc;

    GT_U32                      headOfTrieOffset;
    GT_U32                      bucketTypeOffset;
    GT_U32                      bucketTypeValue;

    tableType = CPSS_DXCH_SIP7_TABLE_VRF_COMMON_E;

    switch (protocol)
    {
        case PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E:
            headOfTrieOffset = 25; /* bits 25-48 */
            bucketTypeOffset = 49;
            break;

        case PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E:
            headOfTrieOffset = 50; /* bits 50-73 */
            bucketTypeOffset = 74;
            break;

        case PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E:
            headOfTrieOffset = 0; /* bits 0-23 */
            bucketTypeOffset = 24;
            break;

        default:

            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    switch (nodeType)
    {
        case CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E:
            bucketTypeValue = 0;

            break;

        case CPSS_DXCH_LPM_COMPRESSED_NODE_PTR_TYPE_E:
        case CPSS_DXCH_LPM_COMPRESSED_UP_TO_7_RANGES_1_LEAF_NODE_PTR_TYPE_E:
        case CPSS_DXCH_LPM_COMPRESSED_UP_TO_5_RANGES_2_LEAVES_NODE_PTR_TYPE_E:
        case CPSS_DXCH_LPM_COMPRESSED_3_RANGES_3_LEAVES_NODE_PTR_TYPE_E:
            bucketTypeValue = 1;
            break;

        default:

            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    cpssOsMemSet(data,0,sizeof(data));
    rc = prvCpssDxChReadTableEntry(devNum,tableType,vrId,data);
    if (rc != GT_OK)
    {
        return rc;
    }
    U32_SET_FIELD_IN_ENTRY_MAC(data, bucketTypeOffset, 1, bucketTypeValue);
    U32_SET_FIELD_IN_ENTRY_MAC(data, headOfTrieOffset, 24, rootNodeAddr);

    /*Lock the access to per device data base in order to avoid corruption*/
    CPSS_API_LOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    rc = prvCpssDxChWriteTableEntry(devNum,
                                    tableType,
                                    vrId,
                                    data);

    /*Unlock the access to per device data base*/
    CPSS_API_UNLOCK_MAC(devNum,PRV_CPSS_FUNCTIONALITY_CONFIGURATION_CNS);

    return rc;
}

/**
* @internal prvCpssDxChLpmHwVrfEntryReadSip7 function
* @endinternal
 *
* @brief   Read a VRF table entry
 *
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3;
*                                  Falcon; AC5P; AC5X; Harrier; Ironman.
 *
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - one of the parameters with bad value
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on non applicable device
*
* @note VRF root nodeTypePtr can be only CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E
*       CPSS_DXCH_LPM_LEAFE_NODE_PTR_TYPE_E,
*       or CPSS_DXCH_LPM_ONE_COMPRESSED_NODE_PTR_TYPE_E.
*
* @param[in] devNum                   - the device number
* @param[in] vrId                     - the virtual router id
* @param[in] protocol                 - the
* @param[out] nodeTypePtr              - the type of the node
* @param[out] rootNodeAddrPtr          - the line number of the root node
*/
GT_STATUS prvCpssDxChLpmHwVrfEntryReadSip7
(
    IN  GT_U8                                   devNum,
    IN  GT_U32                                  vrId,
    IN  PRV_CPSS_DXCH_LPM_PROTOCOL_STACK_ENT    protocol,
    OUT PRV_CPSS_DXCH_LPM_CHILD_TYPE_ENT        *nodeTypePtr,
    OUT GT_U32                                  *rootNodeAddrPtr
)
{
    /* in sip7 we have one table that holds ipv4/ipv6/fcoe
       instae of 3 tables we had in sip6 */

    CPSS_DXCH_TABLE_ENT tableType;
    GT_U32                  data[3];
    GT_STATUS               rc;
    GT_U32                  headOfTrieOffset;
    GT_U32                  headOfTrieValue;
    GT_U32                  bucketTypeOffset;
    GT_U32                  bucketTypeValue;

    PRV_CPSS_DXCH_DEV_CHECK_MAC(devNum);
    PRV_CPSS_NOT_APPLICABLE_DEV_CHECK_MAC(devNum, CPSS_XCAT3_E | CPSS_AC5_E | CPSS_LION2_E | CPSS_BOBCAT2_E | CPSS_CAELUM_E | CPSS_ALDRIN_E | CPSS_BOBCAT3_E | CPSS_ALDRIN2_E);

    CPSS_NULL_PTR_CHECK_MAC(nodeTypePtr);
    CPSS_NULL_PTR_CHECK_MAC(rootNodeAddrPtr);

    cpssOsMemSet(data,0,sizeof(data));

    tableType = CPSS_DXCH_SIP7_TABLE_VRF_COMMON_E;
    switch (protocol)
    {
        case PRV_CPSS_DXCH_LPM_PROTOCOL_IPV4_E:
            headOfTrieOffset = 25; /* bits 25-48 */
            bucketTypeOffset = 49;
            break;

        case PRV_CPSS_DXCH_LPM_PROTOCOL_IPV6_E:
            headOfTrieOffset = 50; /* bits 50-73 */
            bucketTypeOffset = 74;
            break;

        case PRV_CPSS_DXCH_LPM_PROTOCOL_FCOE_E:
            headOfTrieOffset = 0; /* bits 0-23 */
            bucketTypeOffset = 24;
            break;

        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssDxChReadTableEntry(devNum,
                                   tableType,
                                   vrId,
                                   data);

    U32_GET_FIELD_IN_ENTRY_MAC(data,bucketTypeOffset,1,bucketTypeValue);

    /* the bucket type */
    switch (bucketTypeValue)
    {
        case 0:/* regular */
            *nodeTypePtr = PRV_CPSS_DXCH_LPM_CHILD_REGULAR_TYPE_E;
            break;
        case 1:/* one compress  */
            *nodeTypePtr = PRV_CPSS_DXCH_LPM_CHILD_COMPRESSED_TYPE_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_VALUE, LOG_ERROR_NO_MSG);
    }

    U32_GET_FIELD_IN_ENTRY_MAC(data,headOfTrieOffset,24,headOfTrieValue);
    *rootNodeAddrPtr = headOfTrieValue;/* 24 bits */

    return rc;
}

/**
* @internal prvCpssDxChLpmHwEcmpEntryWriteSip7 function
* @endinternal
*
* @brief   Write an ECMP entry
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3;
*                                  Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - the device number
* @param[in] ecmpEntryIndex           - the index of the entry in the ECMP table
* @param[in] ecmpEntryPtr             - (pointer to) the ECMP entry
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
*/
GT_STATUS prvCpssDxChLpmHwEcmpEntryWriteSip7
(
    IN GT_U8                                devNum,
    IN GT_U32                               ecmpEntryIndex,
    IN PRV_CPSS_DXCH_LPM_ECMP_ENTRY_STC     *ecmpEntryPtr
)
{
/* SIP7_LPM_TBD update logic*/
    GT_U32  data = 0;
    GT_U32  numOfNextHopEntries;
    GT_U32  maximalIndex;
    GT_U32  value;
    CPSS_DXCH_TABLE_ENT         tableType;

    if(PRV_CPSS_SIP_5_25_CHECK_MAC(devNum) == GT_TRUE)
    {
        /*ECMP table point to indirect access table*/
        maximalIndex = prvCpssDxchTableIpvxRouterEcmpPointerNumEntriesGet(devNum);
    }
    else
    {
        maximalIndex = PRV_CPSS_DXCH_PP_MAC(devNum)->fineTuning.tableSize.routerNextHop;
    }

    /* for sip6 there are only 15 bits the field
       sip7 was increased to 18 bits */
    if(PRV_CPSS_SIP_7_CHECK_MAC(devNum) == GT_TRUE)
    {
        numOfNextHopEntries = MIN(BIT_18, maximalIndex);
    }
    else
    {
        numOfNextHopEntries = MIN(BIT_15, maximalIndex);
    }

    if ((ecmpEntryPtr->numOfPaths == 0) || (ecmpEntryPtr->numOfPaths > BIT_12))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (ecmpEntryPtr->nexthopBaseIndex >= numOfNextHopEntries)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if ((ecmpEntryPtr->nexthopBaseIndex + ecmpEntryPtr->numOfPaths) > numOfNextHopEntries)
    {
        /* last entry must not be out of the NH table */
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    tableType = CPSS_DXCH_SIP6_TABLE_IPVX_ECMP_E;
    switch (ecmpEntryPtr->multiPathMode) {
    case PRV_CPSS_DXCH_LPM_ENTRY_TYPE_ECMP_E:
        value = 0;
        break;
    case PRV_CPSS_DXCH_LPM_ENTRY_TYPE_QOS_E:
        value = 1;
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    /* in SIP7 the NH pointer was increased from 15 to 18 bits
       all fields moved accordingly */
    data = ((BOOL2BIT_MAC(ecmpEntryPtr->randomEnable) << 18)|
            ((ecmpEntryPtr->numOfPaths - 1) << 19) |
            (value  << 31) |
            (ecmpEntryPtr->nexthopBaseIndex));


    return prvCpssDxChWriteTableEntry(devNum,
                                      tableType,
                                      ecmpEntryIndex,
                                      &data);
}

/**
* @internal prvCpssDxChLpmHwEcmpEntryReadSip7 function
* @endinternal
*
* @brief   Read an ECMP entry
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3;
*                                  Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - the device number
* @param[in] ecmpEntryIndex           - the index of the entry in the ECMP table
*
* @param[out] ecmpEntryPtr             - (pointer to) the ECMP entry
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on wrong devNum
*/
GT_STATUS prvCpssDxChLpmHwEcmpEntryReadSip7
(
    IN  GT_U8                               devNum,
    IN  GT_U32                              ecmpEntryIndex,
    OUT PRV_CPSS_DXCH_LPM_ECMP_ENTRY_STC    *ecmpEntryPtr
)
{
/* SIP7_LPM_TBD update logic*/
    GT_U32      data = 0;
    GT_STATUS   rc;
    GT_U32      value = 0;
    CPSS_DXCH_TABLE_ENT         tableType;

    tableType = CPSS_DXCH_SIP6_TABLE_IPVX_ECMP_E;

    rc = prvCpssDxChReadTableEntry(devNum,
                                   tableType,
                                   ecmpEntryIndex,
                                   &data);

    /* in SIP7 the NH pointer was increased from 15 to 18 bits
       all fields moved accordingly */

    if (rc == GT_OK)
    {
        ecmpEntryPtr->randomEnable = BIT2BOOL_MAC((data >> 18) & 0x1);
        ecmpEntryPtr->numOfPaths = ((data >> 19) & 0xfff) + 1;
        ecmpEntryPtr->nexthopBaseIndex = (data & 0x3FFFF);

        if (PRV_CPSS_SIP_6_CHECK_MAC(devNum) == GT_TRUE)
        {
            value = (data >> 31) & 0x1;
            switch (value) {
            case 0:
                ecmpEntryPtr->multiPathMode = PRV_CPSS_DXCH_LPM_ENTRY_TYPE_ECMP_E;
                break;
            case 1:
                ecmpEntryPtr->multiPathMode = PRV_CPSS_DXCH_LPM_ENTRY_TYPE_QOS_E;
                break;
            default:
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, LOG_ERROR_NO_MSG);
            }
        }
    }

    return rc;
}


/**
* @internal prvCpssDxChLpmLeafEntryCalcLpmHwIndexFromSwIndexSip7 function
* @endinternal
*
* @brief   Calculate HW Index for an LPM leaf entry
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3;
*                                  Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - the device number
* @param[in] leafIndex                - leaf index within the LPM PBR block
*
* @param[out] lpmIndexPtr              - (pointer to) leaf index within the LPM PBR block in HW representation
*
* @retval GT_OK                    - on success
*/
GT_STATUS prvCpssDxChLpmLeafEntryCalcLpmHwIndexFromSwIndexSip7
(
    IN GT_U8            devNum,
    IN GT_U32           leafIndex,
    OUT GT_U32          *lpmIndexPtr
)
{
    GT_U32              pbrBase_k;                   /* the base address of PBR - SW address */
    GT_U32              numOfLinesInFirstPbrBlock_m; /* number of lines occupied by PBR in the first PBR block,
                                                        rest of the lines are used for IP LPM*/
    GT_U32              numOfLinesInBlock_n;         /* number of legal lines in the memory block*/
    GT_U32              totalNumOfLinesInBlockIncludingGap_h;/* number of lines in the memory block
                                                                including gap of lines that cannot be used */
    GT_U32              blockIdFromStartPbr_e;      /* block id from the start of PBR blocks.
                                                       if we have 3 blocks for example then the
                                                       blocks ID will be 0, 1, 2 */
    GT_U32              addrOfLastPbrBlock;         /* address of the last PBR found that
                                                       is suitable for adding the needed PBR */
    GT_U32              offsetInLastPbrBlock;       /* offset of the new PBR in the last PBR block found */


    /* check that leafIndex is a legal PBR index: 0...(maxNumOfPbrEntries-1) */
    if(leafIndex>=PRV_CPSS_DXCH_PP_MAC(devNum)->moduleCfg.ip.maxNumOfPbrEntries)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "leafIndex[%d] must be less than maxNumOfPbrEntries[%d]",
            leafIndex,
            PRV_CPSS_DXCH_PP_MAC(devNum)->moduleCfg.ip.maxNumOfPbrEntries);
    }
    pbrBase_k=PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.indexForPbr;
    numOfLinesInBlock_n = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.numOfLinesInBlock;

    if(PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.lpmMemMode == PRV_CPSS_DXCH_LPM_RAM_MEM_MODE_HALF_MEM_SIZE_E)
    {
       numOfLinesInBlock_n/=2;
    }
    totalNumOfLinesInBlockIncludingGap_h = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.totalNumOfLinesInBlockIncludingGap;
    numOfLinesInFirstPbrBlock_m = numOfLinesInBlock_n-(pbrBase_k%totalNumOfLinesInBlockIncludingGap_h);

    if (leafIndex<numOfLinesInFirstPbrBlock_m)
    {
        *lpmIndexPtr = pbrBase_k+leafIndex;
    }
    else
    {

        /* LPM with PBR blocks in case numOfLinesInBlock_n < totalNumOfLinesInBlockIncludingGap_h

           blocks:  0       1               17          18          19 --> total 20 blocks
                    -----   -----   .....   -----  e--> ----- e-->  -----  --
                    |   |   |   |           |n-m|       |   |       |   |  ^
                    | n |   | n |      k--> |---|       | n |       | n |  ^
                    |   |   |   |           | m |       |   |       |   |  ^
                    |---|   |---|           |---|       |---|       |---|  ^ h
                    |   |   |   |           |   |       |   |       |   |  ^
                    |   |   |   |           |   |       |   |       |   |  ^
                    -----   -----           -----       -----       -----  --
        */
        /* the formula for finding the lpmIndex (hw index)calculated from leafIndex (sw index ) is:
           lpmIndex = (e+(k/h))*h + ((x-m)%n)
           e = (leafIndex - m)/n + 1 --> when the leafIndex is bigger the size allocated for the
           PBR in the first PBR block (m), we need to deduct the size in the first PBR block (m) from the leafIndex,
           then we need to divide it by the size of the block (n) in order to get the block index where it should be set.
           since we get values from 0... we need to add + 1 in order to get to the correct block index that can be
           used in the next formula calculation */
        blockIdFromStartPbr_e = ((leafIndex-numOfLinesInFirstPbrBlock_m)/numOfLinesInBlock_n)+1;
        /* addrOfLastPbrBlock = (e+(k/h))* h --> k is the base address of PBR (SW address),
           h is the total size of a block including the gap, k/h gives the block index where the PBR starts,
           then we need to progress to the block calculated above (e) where our index will reside.
           in the end we have the PBR block index from the beginning of the LPM blocks where we will set the data.
           now we want to translate it to an address so we multiply it by the total size of the block including the gap (h)*/
        addrOfLastPbrBlock = (blockIdFromStartPbr_e+(pbrBase_k/totalNumOfLinesInBlockIncludingGap_h))*totalNumOfLinesInBlockIncludingGap_h;
        /* offsetInLastPbrBlock = (leafIndex - m) % n --> in order to get the offset of the PBR in the
           last suitable block we found, we need to deduct from leafIndex the size in the first PBR block,
           and to do modulo n in order to get the remaining that will fit the last suitable PBR block */
        offsetInLastPbrBlock = ((leafIndex-numOfLinesInFirstPbrBlock_m)%numOfLinesInBlock_n);
        *lpmIndexPtr = addrOfLastPbrBlock + offsetInLastPbrBlock;
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChLpmLeafEntryCalcLpmSwIndexFromHwIndexSip7 function
* @endinternal
*
* @brief   Calculate SW Index for an LPM leaf HW index
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2;Bobcat2; Caelum; Aldrin; AC3X; Bobcat3;
*                                  Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - the device number
* @param[in] leafHwIndex              - leaf HW index within the LPM PBR block in HW representation
*
* @param[out] leafSwIndexPtr           - (pointer to)leaf index within the LPM PBR block SW representation
*
* @retval GT_OK                    - on success
*/
GT_VOID prvCpssDxChLpmLeafEntryCalcLpmSwIndexFromHwIndexSip7
(
    IN GT_U8            devNum,
    IN GT_U32           leafHwIndex,
    OUT GT_U32          *leafSwIndexPtr
)
{
    GT_U32              pbrBase_k;                   /* the base address of PBR - SW address */
    GT_U32              numOfLinesInFirstPbrBlock_m; /* number of lines occupied by PBR in the first PBR block,
                                                        rest of the lines are used for IP LPM*/
    GT_U32              numOfLinesInBlock_n;         /* number of legal lines in the memory block*/
    GT_U32              totalNumOfLinesInBlockIncludingGap_h;/* number of lines in the memory block
                                                                including gap of lines that cannot be used */
    GT_U32              numberOfGapsFromBeginnigOfPbrBase; /* number of "holes" from the beggining of the
                                                              PBR to the checked HW Index */
    GT_U32              gapSize_w;                         /* the size of the "hole" in each LPM block */

    pbrBase_k=PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.indexForPbr;
    numOfLinesInBlock_n = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.numOfLinesInBlock;

    if(PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.lpmMemMode == PRV_CPSS_DXCH_LPM_RAM_MEM_MODE_HALF_MEM_SIZE_E)
    {
       numOfLinesInBlock_n/=2;
    }

    totalNumOfLinesInBlockIncludingGap_h = PRV_CPSS_DXCH_PP_MAC(devNum)->hwInfo.lpm.totalNumOfLinesInBlockIncludingGap;
    numOfLinesInFirstPbrBlock_m = numOfLinesInBlock_n-(pbrBase_k%totalNumOfLinesInBlockIncludingGap_h);

    if (leafHwIndex<numOfLinesInFirstPbrBlock_m)
    {
        *leafSwIndexPtr = leafHwIndex;
    }
    else
    {

        /* LPM with PBR blocks in case numOfLinesInBlock_n < totalNumOfLinesInBlockIncludingGap_h

           blocks:  0       1               17          18          19 --> total 20 blocks
                    -----   -----   .....   -----  e--> ----- e-->  -----  --
                    |   |   |   |           |n-m|       |   |       |   |  ^
                    | n |   | n |      k--> |---|       | n |       | n |  ^
                    |   |   |   |           | m |       |   |       |   |  ^
                    |---|   |---|           |---|       |---|       |---|  ^ h
                    |   |   |   |           |   |       |   |       |   |  ^
                    | w |   | w |           | w |       | w |       | w |  ^
                    -----   -----           -----       -----       -----  --
        */

        numberOfGapsFromBeginnigOfPbrBase = (leafHwIndex/totalNumOfLinesInBlockIncludingGap_h);
        if (numberOfGapsFromBeginnigOfPbrBase==0)
        {
            /* in case the HW Index value is in bigger the numOfLinesInFirstPbrBlock_m
               but smaller then the size of one block (h)--> the index resize in the first
               full block after the block partialy occupied by m */
            numberOfGapsFromBeginnigOfPbrBase=1;
        }
        gapSize_w = totalNumOfLinesInBlockIncludingGap_h - numOfLinesInBlock_n;
        *leafSwIndexPtr = (leafHwIndex - (numberOfGapsFromBeginnigOfPbrBase * gapSize_w));
    }

    return;
}

/**
* @internal prvCpssLpmFillPrvLeafValueSip7 function
* @endinternal
*
* @brief   translate CPSS_DXCH_LPM_LEAF_ENTRY_STC leaf into PRV_CPSS_DXCH_LPM_LEAF_ENTRY_STC leaf entry
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] leafNodePtr            - leaf data
*
* @param[out] valuePtr              - pointer to leaf data in hw format.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on any parameter is wrong
*/
GT_VOID prvCpssLpmFillPrvLeafValueSip7
(
    IN  CPSS_DXCH_LPM_LEAF_ENTRY_STC      *leafEntryPtr,
    OUT PRV_CPSS_DXCH_LPM_LEAF_ENTRY_STC  *prvLeafEntryPtr
)
{
    cpssOsMemSet(prvLeafEntryPtr,0,sizeof(PRV_CPSS_DXCH_LPM_LEAF_ENTRY_STC));
    prvLeafEntryPtr->entryType = leafEntryPtr->entryType;
    prvLeafEntryPtr->index = leafEntryPtr->index;
    prvLeafEntryPtr->ipv6MCGroupScopeLevel = leafEntryPtr->ipv6MCGroupScopeLevel;
    prvLeafEntryPtr->sipSaCheckMismatchEnable = leafEntryPtr->sipSaCheckMismatchEnable;
    prvLeafEntryPtr->ucRPFCheckEnable = leafEntryPtr->ucRPFCheckEnable;
    prvLeafEntryPtr->priority = leafEntryPtr->priority;
    /* Applicable for AC5P */
    prvLeafEntryPtr->applyPbr = leafEntryPtr->applyPbr;
    prvLeafEntryPtr->isIpv6Mc = leafEntryPtr->isIpv6Mc;
    prvLeafEntryPtr->epgAssignedToLeafNode = leafEntryPtr->epgAssignedToLeafNode;
}


/**
* @internal prvCpssLpmBuildLeafValueSip7 function
* @endinternal
*
* @brief   Build leaf data for single leaf
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                 - device number
* @param[in] leafNodePtr            - leaf data
*
* @param[out] valuePtr              - pointer to leaf data in hw format.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on any parameter is wrong
*/
GT_STATUS prvCpssLpmBuildLeafValueSip7
(
    IN  GT_U8                             devNum,
    IN  PRV_CPSS_DXCH_LPM_LEAF_ENTRY_STC  *leafNodePtr,
    OUT GT_U32                            *valuePtr
)
{
    GT_U32  value1 = 0;
    GT_U32  value2 = 0;

   /* Set Leaf priority */
    switch (leafNodePtr->priority)
    {
        case CPSS_DXCH_LPM_LEAF_ENTRY_PRIORITY_FDB_E:
            value1 = 0;
            break;
        case CPSS_DXCH_LPM_LEAF_ENTRY_PRIORITY_LPM_E:
            value1 = 1;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "bad priority value");
    }

    U32_SET_FIELD_MASKED_MAC(valuePtr[0], 0, 1, value1); /* LPM priority */
    if (leafNodePtr->pointToSip == GT_TRUE)
    {
        /* fill fields that relevant for pointing to src tree*/
        U32_SET_FIELD_MASKED_MAC(valuePtr[0], 1, 1, 1); /* entry type: 1:1 "trigger" */
        /* pointer to next bucket : 2...25 */
        U32_SET_FIELD_MASKED_MAC(valuePtr[0], 2,
                                 PRV_CPSS_DXCH_LPM_RAM_CHILD_POINTER_NUMBER_OF_BITS_CNS,
                                 leafNodePtr->pointerToSip);
        switch (leafNodePtr->nextNodeType)
        {
        case CPSS_DXCH_LPM_REGULAR_NODE_PTR_TYPE_E:
            value1 = 0;
            break;
        case CPSS_DXCH_LPM_COMPRESSED_NODE_PTR_TYPE_E:
        case CPSS_DXCH_LPM_COMPRESSED_UP_TO_7_RANGES_1_LEAF_NODE_PTR_TYPE_E:
        case CPSS_DXCH_LPM_COMPRESSED_UP_TO_5_RANGES_2_LEAVES_NODE_PTR_TYPE_E:
        case CPSS_DXCH_LPM_COMPRESSED_3_RANGES_3_LEAVES_NODE_PTR_TYPE_E:
            value1 = 1;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_STATE, "BAD next node type");
        }
        U32_SET_FIELD_MASKED_MAC(valuePtr[0], 0, 1, value1);

        return GT_OK;
    }
    else if (PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        /* fill fields that relevant for pointing to leaf*/
        U32_SET_FIELD_MASKED_MAC(valuePtr[0], 1, 1, 0); /*  entry type: "leaf" */

        /* set pbr/lpm match priority */
        U32_SET_FIELD_MASKED_MAC(valuePtr[0], 23, 1, leafNodePtr->applyPbr);
    }

    switch (leafNodePtr->entryType)
    {
    case CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E:
        value1 = 0;
        break;
    case CPSS_DXCH_LPM_MULTIPATH_ENTRY_PTR_TYPE_E:
        value1 = 1;
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    U32_SET_FIELD_MASKED_MAC(valuePtr[0], 1, 1, 0); /* entry type: "leaf" */

    U32_SET_FIELD_MASKED_MAC(valuePtr[0], 2, 1, value1); /* regular leaf */

    if (leafNodePtr->isIpv6Mc==GT_TRUE)
    {
        U32_SET_FIELD_MASKED_MAC(valuePtr[0], 3, 2, leafNodePtr->ipv6MCGroupScopeLevel);
    }
    else
    {
        U32_SET_FIELD_MASKED_MAC(valuePtr[0], 3, 1, BOOL2BIT_MAC(leafNodePtr->ucRPFCheckEnable));
        U32_SET_FIELD_MASKED_MAC(valuePtr[0], 4, 1,BOOL2BIT_MAC(leafNodePtr->sipSaCheckMismatchEnable));
    }

    /* in SIP7 the pointer was increased from 16 bits (supports 48K entries L2NHE)
       to 18 bits(to support 192K entries – 4 in a line for the case of new L3NHE).
       Bits[0:1] , is the number of the entry in a line
       Bits [2:17]>>2 , is the line number.
       if the leaf is of type L2NHE we need to set the index in hw to be (leafNodePtr->index<<2)
       legal values for the index are 0...48K-1
       if the leaf is of type L3NHE we just set the index as it is
       legal values for the index are 0...192K-1
    */
    if ((leafNodePtr->pointToSip == GT_FALSE) &&
        (leafNodePtr->entryType  == CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E)) /* regular leaf */
    {
        /* for now all entries are of type L2NHE */
        U32_SET_FIELD_MASKED_MAC(valuePtr[0], 5, 18, (leafNodePtr->index << 2)); /* nextHop pointer */
    }
    else
    {
        U32_SET_FIELD_MASKED_MAC(valuePtr[0], 5, 18, leafNodePtr->index); /* ecmp/qos pointer */
    }

    /* EPG bits 24:35 - leaf size is 36 bits */
    if (leafNodePtr->pointToSip == GT_FALSE) /* EntryType="Leaf" */
    {
        /* need to add a new field to leafNodePtr name epgAssignedToLeafNode
           EPG assigned to the leaf node
           - On SIP search - assigns source_EPG
           - On DIP search - assigns target EPG
        */
        value1 = leafNodePtr->epgAssignedToLeafNode & 0xFF;
        value2 = (leafNodePtr->epgAssignedToLeafNode>>8) & 0xF;
        U32_SET_FIELD_MASKED_MAC(valuePtr[0], 24, 8, value1);
        U32_SET_FIELD_MASKED_MAC(valuePtr[1], 0, 4, value2);
    }

    return GT_OK;
}

/**
* @internal prvCpssDxChLpmLeafNodeDataBuildSip7 function
* @endinternal
*
* @brief   Build hw format for LPM leaves nodes
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                     - device number
* @param[in] leafNodePtr                - node data
* @param[in] leavesCounter              - number of leaves so far
* @param[in] startNewGonLeaves          - leaves from the new GON started to process
* @param[in] groupOfNodesArrayPtrPtr    - pointer to adress of current data in hw format
*
* @param[out] groupOfNodesArrayPtrPtr   - (pointer to pointer) to data in hw format.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on any parameter is wrong
*/
GT_STATUS prvCpssDxChLpmLeafNodeDataBuildSip7
(
    IN  GT_U8                                       devNum,
    IN  PRV_CPSS_DXCH_LPM_LEAF_ENTRY_STC            *leafNodePtr,
    IN  GT_U32                                      leavesCounter,
    IN  GT_BOOL                                     startNewGonLeaves,
    INOUT GT_U32                                    **groupOfNodesArrayPtrPtr
)
{
    GT_U32      value[2]={0,0};
    GT_U32      bitMask = 0;
    GT_U32      firstPartFieldBitsNum = 0;
    GT_U32      restFieldBitsNum = 0;
    GT_STATUS   rc = GT_OK;

    rc = prvCpssLpmBuildLeafValueSip7(devNum, leafNodePtr,value);
    if (rc != GT_OK)
    {
        return rc;
    }

    /* one lpm line contain 5 leaves: 26*5 = 130 bits */
    /* for each group of nodes leavesCounter is started from 0*/
    if ((leavesCounter % MAX_NUMBER_OF_LEAVES_IN_LPM_LINE_CNS) == 0)
    {
        PRV_SHARED_PORT_DIR_IP_LPM_HW_SRC_GLOBAL_VAR_SET(k, 2); /* first leaf in entry started from bit 129 -> second bit in the word */
        if (startNewGonLeaves == GT_TRUE)
        {
            PRV_SHARED_PORT_DIR_IP_LPM_HW_SRC_GLOBAL_VAR_SET(tempGonArrayPtr,
                                                             *groupOfNodesArrayPtrPtr + PRV_CPSS_DXCH_LPM_RAM_SIP7_SIZE_OF_LPM_ENTRY_DATA_IN_WORDS_CNS); /* points to next LPM line */
            *groupOfNodesArrayPtrPtr = *groupOfNodesArrayPtrPtr+(PRV_CPSS_DXCH_LPM_RAM_SIP7_SIZE_OF_LPM_ENTRY_DATA_IN_WORDS_CNS-1); /* start from the last word , 5 words of data but 8 word for allignment */
        }
        else
        {
            /* another LPM entry */
            *groupOfNodesArrayPtrPtr = PRV_SHARED_PORT_DIR_IP_LPM_HW_SRC_GLOBAL_VAR_GET(tempGonArrayPtr)+(PRV_CPSS_DXCH_LPM_RAM_SIP7_SIZE_OF_LPM_ENTRY_DATA_IN_WORDS_CNS-1);
            PRV_SHARED_PORT_DIR_IP_LPM_HW_SRC_GLOBAL_VAR_SET(tempGonArrayPtr,
                                                             PRV_SHARED_PORT_DIR_IP_LPM_HW_SRC_GLOBAL_VAR_GET(tempGonArrayPtr) +
                                                             PRV_CPSS_DXCH_LPM_RAM_SIP7_SIZE_OF_LPM_ENTRY_DATA_IN_WORDS_CNS); /* points to next LPM line */
        }
    }

    if ((PRV_SHARED_PORT_DIR_IP_LPM_HW_SRC_GLOBAL_VAR_GET(k) - 26) >= 0)
    {
        /* there is enough place to set full leaf in word */
        PRV_SHARED_PORT_DIR_IP_LPM_HW_SRC_GLOBAL_VAR_SET(k, (PRV_SHARED_PORT_DIR_IP_LPM_HW_SRC_GLOBAL_VAR_GET(k) - 26));

        U32_SET_FIELD_MASKED_MAC(**groupOfNodesArrayPtrPtr, PRV_SHARED_PORT_DIR_IP_LPM_HW_SRC_GLOBAL_VAR_GET(k), 26, value[0] & 0x3FFFFFF);

    }
    else
    {
        firstPartFieldBitsNum = PRV_SHARED_PORT_DIR_IP_LPM_HW_SRC_GLOBAL_VAR_GET(k); /* how much bits free till word bound */
        restFieldBitsNum = 26 - firstPartFieldBitsNum; /* number of bits for next word */
        if (firstPartFieldBitsNum != 0)
        {
            PRV_SHARED_PORT_DIR_IP_LPM_HW_SRC_GLOBAL_VAR_SET(k, 0);
            bitMask = BIT_MASK_MAC(firstPartFieldBitsNum);
            U32_SET_FIELD_MASKED_MAC(**groupOfNodesArrayPtrPtr, PRV_SHARED_PORT_DIR_IP_LPM_HW_SRC_GLOBAL_VAR_GET(k), firstPartFieldBitsNum,
                                      (value[0]>>restFieldBitsNum) & bitMask);
        }
        (*groupOfNodesArrayPtrPtr)--;
        /* new word in use */
        PRV_SHARED_PORT_DIR_IP_LPM_HW_SRC_GLOBAL_VAR_SET(k, 32 - restFieldBitsNum);
        if (restFieldBitsNum !=0)
        {
            bitMask = BIT_MASK_MAC(restFieldBitsNum);
            U32_SET_FIELD_MASKED_MAC(**groupOfNodesArrayPtrPtr, PRV_SHARED_PORT_DIR_IP_LPM_HW_SRC_GLOBAL_VAR_GET(k), restFieldBitsNum, value[0] & bitMask);
        }

    }

    return GT_OK;
}

/**
* @internal prvCpssDxChLpmCompressedNodeDataBuildSip7 function
* @endinternal
*
* @brief   Build hw format for LPM compressed nodes
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                     - the device number
* @param[in] compressedNodePtr          - node data
* @param[in] groupOfNodesArrayPtrPtr    - pointer to adress of current data in hw format
*
* @param[out] groupOfNodesArrayPtrPtr   - (pointer to pointer) to data in hw format.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on any parameter is wrong
*/
GT_STATUS prvCpssDxChLpmCompressedNodeDataBuildSip7
(
    IN    GT_U8                                     devNum,
    IN    PRV_CPSS_DXCH_LPM_COMPRESSED_STC          *compressedNodePtr,
    INOUT GT_U32                                    **groupOfNodesArrayPtrPtr
)
{
    int k;
    GT_U32 i;
    GT_U32      childType = 0;
    GT_U32      offset = 0;
    GT_U32      value[2] = {0,0};
    GT_U32      bitMask = 0;
    GT_U32      firstPartFieldBitsNum = 0;
    GT_U32      restFieldBitsNum = 0;
    GT_U32      *tempGonArrayPtr = NULL;
    GT_U32      bankNumber = 0;
    GT_U32      offsetInBank = 0;


    cpssOsMemSet(value,0,sizeof(value));
    tempGonArrayPtr = *groupOfNodesArrayPtrPtr;
    *groupOfNodesArrayPtrPtr = *groupOfNodesArrayPtrPtr+(PRV_CPSS_DXCH_LPM_RAM_SIP7_SIZE_OF_LPM_ENTRY_DATA_IN_WORDS_CNS-1); /* entry have 5 words of data bits 0-131, go to the last word with data: word4*/
    if (compressedNodePtr->compressedType == CPSS_DXCH_LPM_COMPRESSED_NODE_PTR_TYPE_E)
    {
        /* the number of leaves for standart compress is 0 but for SIP7 we need to set the value to be 4 */
        U32_SET_FIELD_MASKED_MAC(**groupOfNodesArrayPtrPtr, 1, 3, 4); /* bits 129-131 */
        (*groupOfNodesArrayPtrPtr)--; /* go to word3: bits 96-127 */
        k = 28;/* first child type is located at bit 124 = 96+28 */
        for (i = 0; i < SIP7_MAX_NUMBER_OF_COMPRESSED_RANGES_CNS; i++)
        {
            childType = compressedNodePtr->childNodeTypes0_9[i];
            offset = compressedNodePtr->ranges1_9[i];
            if (i == 0)
            {
                U32_SET_FIELD_MASKED_MAC(**groupOfNodesArrayPtrPtr, k, 2,childType);
                continue;
            }
            value[0] = ((offset << 2) | childType)& 0x3ff;
            if ((k - 10) > 0)
            {
                k = k -10;
                U32_SET_FIELD_MASKED_MAC(**groupOfNodesArrayPtrPtr, k, 10, value[0]);
            }
            else
            {
                firstPartFieldBitsNum = k; /* how much bits free till word bound */
                restFieldBitsNum = 10 - firstPartFieldBitsNum; /* number of bits in next word */
                if (firstPartFieldBitsNum != 0)
                {
                    k = 0;
                    bitMask = BIT_MASK_MAC(firstPartFieldBitsNum);
                    U32_SET_FIELD_MASKED_MAC(**groupOfNodesArrayPtrPtr, k, firstPartFieldBitsNum,
                                             (value[0]>>restFieldBitsNum) & bitMask);
                }
                (*groupOfNodesArrayPtrPtr)--;
                k = 32-restFieldBitsNum;
                if (restFieldBitsNum != 0)
                {
                    bitMask = BIT_MASK_MAC(restFieldBitsNum);
                    U32_SET_FIELD_MASKED_MAC(**groupOfNodesArrayPtrPtr, k, restFieldBitsNum, value[0] & bitMask);
                }
            }
        }
    }
    else
        if (compressedNodePtr->compressedType == CPSS_DXCH_LPM_COMPRESSED_UP_TO_7_RANGES_1_LEAF_NODE_PTR_TYPE_E)
        {
            /* the number of leaves for this type of compress is 1 */
            U32_SET_FIELD_MASKED_MAC(**groupOfNodesArrayPtrPtr, 1, 3, 1); /* bits 129-131 */
            (*groupOfNodesArrayPtrPtr)--; /* go to word3: bits 96-127 */

            /* leaf occupies bits 92-127 */
            cpssOsMemSet(value,0,sizeof(value));
            prvCpssLpmBuildLeafValueSip7(devNum, &compressedNodePtr->embLeavesArray[0], value);
            /* In current word there is place for 32 bits: from 96 - 127 */
            U32_SET_FIELD_MASKED_MAC(**groupOfNodesArrayPtrPtr, 28, 4, (value[1]&0xF));
            U32_SET_FIELD_MASKED_MAC(**groupOfNodesArrayPtrPtr, 0, 28, value[0]>>4);
            (*groupOfNodesArrayPtrPtr)--; /* go to word2 */
            /* The last 4 bits from 92 - 95*/
            U32_SET_FIELD_MASKED_MAC(**groupOfNodesArrayPtrPtr, 28, 4, (value[0]&0xF));
            /* 86 - 91 are reserved */
            k = 20;/* first child type is located at bit 84 = 64+20 - word2 */
            cpssOsMemSet(value,0,sizeof(value));
            for (i = 0; i < 7; i++)
            {
                childType = compressedNodePtr->childNodeTypes0_9[i];
                offset = compressedNodePtr->ranges1_9[i];
                if (i == 0)
                {
                    U32_SET_FIELD_MASKED_MAC(**groupOfNodesArrayPtrPtr, k, 2,childType);
                    continue;
                }
                value[0] = ((offset << 2) | childType)& 0x3ff;
                if ((k - 10) > 0)
                {
                    k = k -10;
                    U32_SET_FIELD_MASKED_MAC(**groupOfNodesArrayPtrPtr, k, 10, value[0]);
                }
                else
                {
                    firstPartFieldBitsNum = k; /* how much bits free till word bound */
                    restFieldBitsNum = 10 - firstPartFieldBitsNum; /* number of bits in next word */
                    if (firstPartFieldBitsNum != 0)
                    {
                        k = 0;
                        bitMask = BIT_MASK_MAC(firstPartFieldBitsNum);
                        U32_SET_FIELD_MASKED_MAC(**groupOfNodesArrayPtrPtr, k, firstPartFieldBitsNum,
                                                 (value[0]>>restFieldBitsNum) & bitMask);
                    }
                    (*groupOfNodesArrayPtrPtr)--;
                    k = 32-restFieldBitsNum;
                    bitMask = BIT_MASK_MAC(restFieldBitsNum);
                    U32_SET_FIELD_MASKED_MAC(**groupOfNodesArrayPtrPtr, k, restFieldBitsNum, value[0] & bitMask);
                }
            }
        }
        else
            if (compressedNodePtr->compressedType == CPSS_DXCH_LPM_COMPRESSED_UP_TO_5_RANGES_2_LEAVES_NODE_PTR_TYPE_E)
            {
                /* the number of leaves for this type of compress is 2 */
                U32_SET_FIELD_MASKED_MAC(**groupOfNodesArrayPtrPtr, 1, 3, 2); /* bits 129-131 */
                (*groupOfNodesArrayPtrPtr)--; /* go to word3: bits 96-127 */
                cpssOsMemSet(value,0,sizeof(value));
                prvCpssLpmBuildLeafValueSip7(devNum, &compressedNodePtr->embLeavesArray[0], value);
                /* leaf occupies bits 92-127 */
                /* In current word there is place for 32 bits: from 96 - 127*/
                U32_SET_FIELD_MASKED_MAC(**groupOfNodesArrayPtrPtr, 28, 4, (value[1]&0xF));
                U32_SET_FIELD_MASKED_MAC(**groupOfNodesArrayPtrPtr, 0, 28, value[0]>>4);
                (*groupOfNodesArrayPtrPtr)--;
                /* The last 4 bits from 92 - 95*/
                U32_SET_FIELD_MASKED_MAC(**groupOfNodesArrayPtrPtr, 28, 4, (value[0]&0xF));
                /* The second leaf is 66-91 */
                cpssOsMemSet(value,0,sizeof(value));
                prvCpssLpmBuildLeafValueSip7(devNum, &compressedNodePtr->embLeavesArray[1], value);
                U32_SET_FIELD_MASKED_MAC(**groupOfNodesArrayPtrPtr, 2, 26, value[0] & 0x3FFFFFF);

                /* the next 2 bits - bits 65,64 in this word are childType0 */
                childType = compressedNodePtr->childNodeTypes0_9[0];
                U32_SET_FIELD_MASKED_MAC(**groupOfNodesArrayPtrPtr, 0, 2,childType);

                (*groupOfNodesArrayPtrPtr)--;
                /* starting from bit32 */

                k = 32;
                cpssOsMemSet(value,0,sizeof(value));
                for (i = 1; i < 5; i++)
                {
                    childType = compressedNodePtr->childNodeTypes0_9[i];
                    offset = compressedNodePtr->ranges1_9[i];

                    value[0] = ((offset << 2) | childType)& 0x3ff;
                    if ((k - 10) > 0)
                    {
                        k = k -10;
                        U32_SET_FIELD_MASKED_MAC(**groupOfNodesArrayPtrPtr, k, 10, value[0]);
                    }
                    else
                    {
                        firstPartFieldBitsNum = k; /* how much bits free till word bound */
                        restFieldBitsNum = 10 - firstPartFieldBitsNum; /* number of bits in next word */
                        if (firstPartFieldBitsNum != 0)
                        {
                            k = 0;
                            bitMask = BIT_MASK_MAC(firstPartFieldBitsNum);
                            U32_SET_FIELD_MASKED_MAC(**groupOfNodesArrayPtrPtr, k, firstPartFieldBitsNum,
                                                     (value[0]>>restFieldBitsNum) & bitMask);
                        }
                        (*groupOfNodesArrayPtrPtr)--;
                        k = 32-restFieldBitsNum;
                        bitMask = BIT_MASK_MAC(restFieldBitsNum);
                        U32_SET_FIELD_MASKED_MAC(**groupOfNodesArrayPtrPtr, k, restFieldBitsNum, value[0] & bitMask);
                    }
                }

            }
            else

                if (compressedNodePtr->compressedType == CPSS_DXCH_LPM_COMPRESSED_3_RANGES_3_LEAVES_NODE_PTR_TYPE_E)
                {
                    /* the number of leaves for this type of compress is 3 */
                    U32_SET_FIELD_MASKED_MAC(**groupOfNodesArrayPtrPtr, 1, 3, 3); /* bits 129-131 */
                    (*groupOfNodesArrayPtrPtr)--; /* go to word3: bits 96-127 */
                    /* bits 124-128 are reserved */
                    /* so fill ranges starting from bit 116*/

                    /* offset1 bits 116 - 123*/
                    offset = compressedNodePtr->ranges1_9[1];
                    U32_SET_FIELD_MASKED_MAC(**groupOfNodesArrayPtrPtr, 20, 8, offset);
                    /* offset2 bits 108 - 115*/
                    offset = compressedNodePtr->ranges1_9[2];
                    U32_SET_FIELD_MASKED_MAC(**groupOfNodesArrayPtrPtr, 12, 8, offset);

                    /* leaf0 occupies bits 72-107 */
                    cpssOsMemSet(value,0,sizeof(value));
                    prvCpssLpmBuildLeafValueSip7(devNum, &compressedNodePtr->embLeavesArray[0], value);
                    /* In current word there is place for 12 bits: from 96 - 107*/
                    U32_SET_FIELD_MASKED_MAC(**groupOfNodesArrayPtrPtr, 8, 4, (value[1]&0xF));
                    U32_SET_FIELD_MASKED_MAC(**groupOfNodesArrayPtrPtr, 0, 8, (value[0]>>24)&0xFF);
                    (*groupOfNodesArrayPtrPtr)--;
                    /* The last 24 bits from 72 - 95*/
                    U32_SET_FIELD_MASKED_MAC(**groupOfNodesArrayPtrPtr, 8, 24, (value[0]&0xFFFFFF));

                    /* The second leaf1 is 36-71 */
                    cpssOsMemSet(value,0,sizeof(value));
                    prvCpssLpmBuildLeafValueSip7(devNum, &compressedNodePtr->embLeavesArray[1], value);
                    /* In current word there is place for 8 bits: from 64 - 71 */
                    U32_SET_FIELD_MASKED_MAC(**groupOfNodesArrayPtrPtr, 4, 4, (value[1]&0xF));
                    U32_SET_FIELD_MASKED_MAC(**groupOfNodesArrayPtrPtr, 0, 4, (value[0]>>28)&0xF);
                     (*groupOfNodesArrayPtrPtr)--;
                    /* The last 28 bits from 36 - 63*/
                    U32_SET_FIELD_MASKED_MAC(**groupOfNodesArrayPtrPtr, 4, 28, value[0]);

                    /* The 3 leaf is 0-35 */
                    cpssOsMemSet(value,0,sizeof(value));
                    prvCpssLpmBuildLeafValueSip7(devNum, &compressedNodePtr->embLeavesArray[2], value);
                    /* In current word there is place for 4 bits: from 32-35 */
                    U32_SET_FIELD_MASKED_MAC(**groupOfNodesArrayPtrPtr, 0, 4, (value[1]&0xF));
                    (*groupOfNodesArrayPtrPtr)--;
                    /* In this word 32 bits 0-31  ( we can not use U32_SET_FIELD_MASKED_MAC to set 32 bits) */
                    **groupOfNodesArrayPtrPtr=value[0];
                }
                else
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
                }
    if (*groupOfNodesArrayPtrPtr != tempGonArrayPtr)
    {
        cpssOsPrintf("prvCpssDxChLpmGroupOfNodesWrite: wrong compr GON ptr\n ");
    }
    if (compressedNodePtr->compressedType == CPSS_DXCH_LPM_COMPRESSED_3_RANGES_3_LEAVES_NODE_PTR_TYPE_E)
    {
        /* child pointer do not exist in 3 embedded structure */
        *groupOfNodesArrayPtrPtr = *groupOfNodesArrayPtrPtr+PRV_CPSS_DXCH_LPM_RAM_SIP7_SIZE_OF_LPM_ENTRY_DATA_IN_WORDS_CNS;
    }
    else
    {
        offsetInBank = compressedNodePtr->lpmOffset & PRV_CPSS_DXCH_LPM_RAM_OFFSET_MASK_CNS;
        bankNumber = (compressedNodePtr->lpmOffset >> PRV_CPSS_DXCH_LPM_RAM_OFFSET_NUMBER_OF_BITS_CNS) & PRV_CPSS_DXCH_LPM_RAM_BANK_MASK_CNS;
        U32_SET_FIELD_MASKED_MAC(**groupOfNodesArrayPtrPtr,
                                 PRV_CPSS_DXCH_LPM_RAM_OFFSET_NUMBER_OF_BITS_CNS,
                                 PRV_CPSS_DXCH_LPM_RAM_BANK_NUMBER_OF_BITS_CNS,
                                 bankNumber & PRV_CPSS_DXCH_LPM_RAM_BANK_MASK_CNS);

        U32_SET_FIELD_MASKED_MAC(**groupOfNodesArrayPtrPtr,
                                 0,
                                 PRV_CPSS_DXCH_LPM_RAM_OFFSET_NUMBER_OF_BITS_CNS,
                                 offsetInBank & PRV_CPSS_DXCH_LPM_RAM_OFFSET_MASK_CNS);

        *groupOfNodesArrayPtrPtr = *groupOfNodesArrayPtrPtr+PRV_CPSS_DXCH_LPM_RAM_SIP7_SIZE_OF_LPM_ENTRY_DATA_IN_WORDS_CNS;
    }

    return GT_OK;

}

/**
* @internal prvCpssLpmDecodeLeafValueSip7 function
* @endinternal
*
* @brief   Decode leaf entry from HW value
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                 - device number
* @param[in] value                  - leaf data in HW format
*
* @param[out] leafNodePtr           - pointer to leaf entry.
*
* @retval GT_OK                     - on success
* @retval GT_BAD_PARAM              - on any parameter is wrong
*/
GT_STATUS prvCpssLpmDecodeLeafValueSip7
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          *value,
    OUT CPSS_DXCH_LPM_LEAF_ENTRY_STC    *leafNodePtr
)
{
    GT_U32 value1 = 0;
    GT_BOOL mcLeafTrigerMcSgLookup = GT_FALSE;

    leafNodePtr->priority = U32_GET_FIELD_MAC(value[0], 0, 1);
    mcLeafTrigerMcSgLookup = BIT2BOOL_MAC(U32_GET_FIELD_MAC(value[0] , 1, 1));
    value1 = U32_GET_FIELD_MAC(value[0], 2, 1);
    switch(value1)
    {
        case 0:
            leafNodePtr->entryType = CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E;
            break;
        case 1:
            leafNodePtr->entryType = CPSS_DXCH_LPM_MULTIPATH_ENTRY_PTR_TYPE_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if (mcLeafTrigerMcSgLookup == GT_TRUE)
    {
        /* not implemented yet */
        /* in bits 2...21 is intended for pointer to the next bucket on hte S,G lookup */
        /* it will be source tree root bucket */
    }

    if (leafNodePtr->isIpv6Mc==GT_TRUE)
    {
         leafNodePtr->ipv6MCGroupScopeLevel = U32_GET_FIELD_MAC(value[0], 3, 2);
    }
    else
    {
        leafNodePtr->ucRPFCheckEnable = BIT2BOOL_MAC(U32_GET_FIELD_MAC(value[0] , 3, 1));
        leafNodePtr->sipSaCheckMismatchEnable = BIT2BOOL_MAC(U32_GET_FIELD_MAC(value[0], 4, 1));
    }

     /* in SIP7 the pointer was increased from 16 bits (supports 48K entries L2NHE)
       to 18 bits(to support 192K entries – 4 in a line for the case of new L3NHE).
       Bits[0:1] , is the number of the entry in a line
       Bits [2:17]>>2 , is the line number.
       if the leaf is of type L2NHE we need to get the index from hw (leafNodePtr->index>>2)
       legal values for the index are 0...48K-1
       if the leaf is of type L3NHE we just get the index as it is
       legal values for the index are 0...192K-1
    */
    leafNodePtr->index = U32_GET_FIELD_MAC(value[0], 5, 18);

    if ((mcLeafTrigerMcSgLookup == GT_FALSE) &&
        (leafNodePtr->entryType == CPSS_DXCH_LPM_ROUTE_ENTRY_NEXT_PTR_TYPE_E))/* regular leaf */
    {
        /* for now all entries are of type L2NHE */
        leafNodePtr->index = leafNodePtr->index>>2;
    }

    if (mcLeafTrigerMcSgLookup == GT_TRUE)
    {
        /* next node type : regular or compressed : not implemented yet */
    }
    else
    {
        if(PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
        {
            /* pbr/lpm match priority */
            leafNodePtr->applyPbr = BIT2BOOL_MAC(U32_GET_FIELD_MAC(value[0], 23, 1));
        }
    }

    if((PRV_CPSS_SIP_7_CHECK_MAC(devNum)) && (mcLeafTrigerMcSgLookup == GT_FALSE))/* entryType="Leaf" */
    {
        leafNodePtr->epgAssignedToLeafNode = U32_GET_FIELD_MAC(value[1], 0, 4);
        leafNodePtr->epgAssignedToLeafNode = (leafNodePtr->epgAssignedToLeafNode<<8) | U32_GET_FIELD_MAC(value[0], 24, 8);
    }

    return GT_OK;
}


/**
* @internal prvCpssLpmPbrIndexConvertSip7 function
* @endinternal
*
* @brief   Convert leafIndex to address space index
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                  - device number
* @param[in] leafIndexPtr            - pointer to leaf index.
*
* @param[out] leafIndexPtr           - pointer to leaf index in address space format.
*
* @retval GT_OK                     - on success
*/
GT_STATUS prvCpssLpmPbrIndexConvertSip7
(
    IN    GT_U8     devNum,
    INOUT GT_U32    *leafIndexPtr
)
{
    GT_U32 lineNumber;
    GT_U32 leafOffsetInLine;

    PRV_CPSS_DXCH_MODULE_CONFIG_STC *moduleCfgPtr;
    moduleCfgPtr = &(PRV_CPSS_DXCH_PP_MAC(devNum)->moduleCfg);
    lineNumber = (*leafIndexPtr)/MAX_NUMBER_OF_LEAVES_IN_LPM_LINE_CNS;
    leafOffsetInLine = (*leafIndexPtr)%MAX_NUMBER_OF_LEAVES_IN_LPM_LINE_CNS;

    /* for sip7 we need to calculate the offset in a given bank
       leafIndex is the offset in the PBR blocks (offset in the sum of all banks assigned to the PBR)
       we need the offset in a specific PBR bank, offset should be 0 to pbrSize */
    *leafIndexPtr =  ((lineNumber%(moduleCfgPtr->ip.pbrBankSize))*MAX_NUMBER_OF_LEAVES_IN_LPM_LINE_CNS)+leafOffsetInLine;
    return GT_OK;
}
