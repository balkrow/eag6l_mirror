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
* @file snetAasExactMatch.c
*
* @brief SIP7 AAS Exact Match
*
* @version   1
********************************************************************************
*/

#include <asicSimulation/SLog/simLog.h>
#include <asicSimulation/SLog/simLogInfoTypePacket.h>
#include <asicSimulation/SLog/simLogInfoTypeDevice.h>
#include <asicSimulation/SKernel/suserframes/snetCheetahPcl.h>
#include <asicSimulation/SKernel/suserframes/snetFalconExactMatch.h>
#include <asicSimulation/SKernel/suserframes/snetHawkExactMatch.h>
#include <asicSimulation/SKernel/cheetahCommon/sregFalcon.h>
#include <asicSimulation/SKernel/suserframes/snetLion3Tcam.h>
#include <common/Utils/PresteraHash/smacHashExactMatch.h>
#include <asicSimulation/SKernel/cheetahCommon/sregHawk.h>
#include <asicSimulation/SKernel/suserframes/snetAasExactMatch.h>
#include <asicSimulation/SKernel/cheetahCommon/sregSeahawk.h>

#define CHECK_FIELD_INTEGRITY(startBit,endBit) \
    if(startBit > endBit) skernelFatalError(" CHECK_FIELD_INTEGRITY: start bit[%d] > end bit[%d] \n");\
    if((startBit + 32) < endBit) skernelFatalError(" CHECK_FIELD_INTEGRITY: start bit[%d] + 32 < end bit[%d] \n")

#define CONVERT_EM_UNIT_TO_EM_TABLE_ID(emUnitNum, channelNum) \
    ((2*emUnitNum) + channelNum);

static GT_CHAR* emxClientName[SIP7_EMX_CLIENT_LAST_E + 1] =
{
    STR(SIP7_EMX_CLIENT_TTI_E),
    STR(SIP7_EMX_CLIENT_IPCL1_E),
    STR(SIP7_EMX_CLIENT_IPCL2_E),
    STR(SIP7_EMX_CLIENT_MPCL_E),
    STR(SIP7_EMX_CLIENT_EPCL_E),
    STR(SIP7_EMX_CLIENT_IPE_E),
    STR(SIP7_EMX_CLIENT_TTI_SERIAL_EM_E),
    NULL
};

#define SMEM_SIP7_EXACT_MATCH_TABLE_FIELDS_NAME                       \
     STR(SMEM_SIP7_EXACT_MATCH_TABLE_FIELDS_VALID                    )\
    ,STR(SMEM_SIP7_EXACT_MATCH_TABLE_FIELDS_ENTRY_TYPE               )\
     /*EM ACTION*/                                                    \
    ,STR(SMEM_SIP7_EXACT_MATCH_TABLE_FIELDS_EM_ACTION_AGE            )\
    ,STR(SMEM_SIP7_EXACT_MATCH_TABLE_FIELDS_EM_ACTION_KEY_SIZE       )\
    ,STR(SMEM_SIP7_EXACT_MATCH_TABLE_FIELDS_EM_ACTION_LOOKUP_NUMBER  )\
    ,STR(SMEM_SIP7_EXACT_MATCH_TABLE_FIELDS_EM_ACTION_KEY_31_0       )\
    ,STR(SMEM_SIP7_EXACT_MATCH_TABLE_FIELDS_EM_ACTION_KEY_39_32      )\
    ,STR(SMEM_SIP7_EXACT_MATCH_TABLE_FIELDS_EM_ACTION_ACTION_31_0    )\
    ,STR(SMEM_SIP7_EXACT_MATCH_TABLE_FIELDS_EM_ACTION_ACTION_63_32   )\
    ,STR(SMEM_SIP7_EXACT_MATCH_TABLE_FIELDS_EM_ACTION_ACTION_83_64   )\
     /*EM KEY*/                                                       \
    ,STR(SMEM_SIP7_EXACT_MATCH_TABLE_FIELDS_EM_KEY_KEY_31_0          )\
    ,STR(SMEM_SIP7_EXACT_MATCH_TABLE_FIELDS_EM_KEY_KEY_63_32         )\
    ,STR(SMEM_SIP7_EXACT_MATCH_TABLE_FIELDS_EM_KEY_KEY_95_64         )\
    ,STR(SMEM_SIP7_EXACT_MATCH_TABLE_FIELDS_EM_KEY_KEY_127_96        )

static char * sip7ExactMatchFieldsTableNames[SMEM_SIP7_EXACT_MATCH_TABLE_FIELDS___LAST_VALUE___E] =
    {SMEM_SIP7_EXACT_MATCH_TABLE_FIELDS_NAME};

static SNET_ENTRY_FORMAT_TABLE_STC sip7ExactMatchTableFieldsFormat[SMEM_SIP7_EXACT_MATCH_TABLE_FIELDS___LAST_VALUE___E] =
{
    /*SMEM_SIP7_EXACT_MATCH_TABLE_FIELDS_VALID*/
    STANDARD_FIELD_MAC(1)
    /*SMEM_SIP7_EXACT_MATCH_TABLE_FIELDS_ENTRY_TYPE*/
    ,STANDARD_FIELD_MAC(1)
    /*SMEM_SIP7_EXACT_MATCH_TABLE_FIELDS_EM_ACTION_AGE*/
        ,{FIELD_SET_IN_RUNTIME_CNS,
         1,
         SMEM_SIP7_EXACT_MATCH_TABLE_FIELDS_ENTRY_TYPE}
    /*SMEM_SIP7_EXACT_MATCH_TABLE_FIELDS_EM_ACTION_KEY_SIZE*/
    ,STANDARD_FIELD_MAC(2)
   /*   SMEM_SIP7_EXACT_MATCH_TABLE_FIELDS_EM_ACTION_LOOKUP_NUMBER*/
        ,STANDARD_FIELD_MAC(1)
    /*    SMEM_SIP7_EXACT_MATCH_TABLE_FIELDS_EM_ACTION_KEY_31_0*/
        ,STANDARD_FIELD_MAC(32)
    /*    SMEM_SIP7_EXACT_MATCH_TABLE_FIELDS_EM_ACTION_KEY_39_32*/
        ,STANDARD_FIELD_MAC(8)
    /*    SMEM_SIP7_EXACT_MATCH_TABLE_FIELDS_EM_ACTION_ACTION_31_0*/
        ,STANDARD_FIELD_MAC(32)
    /*    SMEM_SIP7_EXACT_MATCH_TABLE_FIELDS_EM_ACTION_ACTION_63_32*/
        ,STANDARD_FIELD_MAC(32)
    /*    SMEM_SIP7_EXACT_MATCH_TABLE_FIELDS_EM_ACTION_ACTION_83_64*/
        ,STANDARD_FIELD_MAC(20)

    /*    SMEM_SIP7_EXACT_MATCH_TABLE_FIELDS_EM_KEY_KEY_31_0*/
        ,{FIELD_SET_IN_RUNTIME_CNS,
         32,
         SMEM_SIP7_EXACT_MATCH_TABLE_FIELDS_ENTRY_TYPE}
    /*    SMEM_SIP7_EXACT_MATCH_TABLE_FIELDS_EM_KEY_KEY_63_32*/
        ,STANDARD_FIELD_MAC(32)
    /*    SMEM_SIP7_EXACT_MATCH_TABLE_FIELDS_EM_KEY_KEY_95_64*/
        ,STANDARD_FIELD_MAC(32)
    /*    SMEM_SIP7_EXACT_MATCH_TABLE_FIELDS_EM_KEY_KEY_127_96*/
        ,STANDARD_FIELD_MAC(32)

};

/* array that holds the info about the Serial EM VLAN Tag fields */
static EXACT_MATCH_SERIAL_EM_KEY_FIELDS_INFO_STC serialEmVlanTagKeyFieldsData[SIP7_EXACT_MATCH_SERIAL_EM_VLAN_TAG_KEY_FIELDS_ID_LAST_E+1] =
{
    {0 , 8 , " SIP7_EXACT_MATCH_SERIAL_EM_VLAN_TAG_KEY_FIELDS_ID_UDB28_E "  },
    {0 , 11, " SIP7_EXACT_MATCH_SERIAL_EM_VLAN_TAG_KEY_FIELDS_ID_TAG1_E "  },
    {12, 12, " SIP7_EXACT_MATCH_SERIAL_EM_VLAN_TAG_KEY_FIELDS_ID_TAG1_IS_RANGE_E "  },
    {13, 13, " SIP7_EXACT_MATCH_SERIAL_EM_VLAN_TAG_KEY_FIELDS_ID_TAG1_FOUND_E "  },
    {14, 21, " SIP7_EXACT_MATCH_SERIAL_EM_VLAN_TAG_KEY_FIELDS_ID_UDB29_E "  },
    {14, 25, " SIP7_EXACT_MATCH_SERIAL_EM_VLAN_TAG_KEY_FIELDS_ID_TAG0_E "  },
    {26, 26, " SIP7_EXACT_MATCH_SERIAL_EM_VLAN_TAG_KEY_FIELDS_ID_TAG0_IS_RANGE_E "  },
    {27, 27, " SIP7_EXACT_MATCH_SERIAL_EM_VLAN_TAG_KEY_FIELDS_ID_TAG0_FOUND_E "  },
    {28, 37, " SIP7_EXACT_MATCH_SERIAL_EM_VLAN_TAG_KEY_FIELDS_ID_SRC_PORT_PROFILE_E "  },
    {38, 39, " SIP7_EXACT_MATCH_SERIAL_EM_VLAN_TAG_KEY_FIELDS_ID_PCL_ID_E "  },
    /* dummy */
    {0 ,  0,  " ------ "}
};

/* array that holds the info about the Serial EM MPLS fields */
static EXACT_MATCH_SERIAL_EM_KEY_FIELDS_INFO_STC serialEmMplsKeyFieldsData[SIP7_EXACT_MATCH_SERIAL_EM_MPLS_KEY_FIELDS_ID_LAST_E+1] =
{
    {0 , 15, " SIP7_EXACT_MATCH_SERIAL_EM_MPLS_KEY_FIELDS_ID_GENERIC_CLASSIFICATION_E "  },
    {16, 16, " SIP7_EXACT_MATCH_SERIAL_EM_MPLS_KEY_FIELDS_ID_LABEL_VALID_E "  },
    {17, 36, " SIP7_EXACT_MATCH_SERIAL_EM_MPLS_KEY_FIELDS_ID_LABEL_E "  },
    {37, 37, " SIP7_EXACT_MATCH_SERIAL_EM_MPLS_KEY_FIELDS_ID_MAC2ME_E "  },
    {38, 39, " SIP7_EXACT_MATCH_SERIAL_EM_MPLS_KEY_FIELDS_ID_PCL_ID_E "  },
    /* dummy */
    {0 ,  0,  " ------ "}
};

/**
* @internal snetSip7ExactMatchTablesFormatInit function
* @endinternal
*
* @brief init the format of SIP7 Exact Match tables.
*
* @param[in] devObjPtr      - pointer to device object.
*/
void snetSip7ExactMatchTablesFormatInit(
    IN SKERNEL_DEVICE_OBJECT            * devObjPtr
)
{
    if(SMEM_CHT_IS_SIP7_GET(devObjPtr))
    {
        LION3_TABLES_FORMAT_INIT_MAC(
            devObjPtr, SKERNEL_TABLE_FORMAT_EXACT_MATCH_ENTRY_E,
            sip7ExactMatchTableFieldsFormat, sip7ExactMatchFieldsTableNames);
    }
}

/**
* @internal exactMatchKeyComparison function
* @endinternal
*
* @brief   return GT_TRUE incase the exact match key (part or all of it )
*          match the key in the exact match bank
*
* @param[in] devObjPtr                       - (pointer to) the device object
* @param[in] exactMatchNewKeyDataPtr         - (pointer to) EM-Key data
* @param[in] exactMatchNewKeySize            -  EM-Key size
* @param[in] exactMatchParsedEntryPtr        - (pointer to) Exact match Parsed Entry
* @param[in] hitNum                          -  exact matchhit number (0,1)
* @param[in] relativeBankOffset              -  the relative bank Offset from the first bank (0..3)
*
*/
static GT_BOOL exactMatchKeyComparison
(
    IN  SKERNEL_DEVICE_OBJECT                 *devObjPtr,
    IN  GT_U32                                *exactMatchNewKeyDataPtr,
    IN  GT_U32                                 exactMatchNewKeySize,
    IN  SNET_SIP7_EXACT_MATCH_ENTRY_INFO      *exactMatchParsedEntryPtr,
    IN  GT_U32                                 hitNum,
    IN  GT_U32                                 relativeBankOffset
)
{
    DECLARE_FUNC_NAME(exactMatchKeyComparison);
    GT_U32 originalKey[4]; /* exact matchkey (per bank ) is max 128 bits */
    GT_U32 globleBitOffset=0;
    GT_U32 parsedKeySize;

    __LOG(("Start comparison of Exact Match entry and Exact Match key \n"));

    if (!exactMatchParsedEntryPtr->valid) /* not valid */
    {
        if (relativeBankOffset == 0)
        {
            __LOG(("No hit , entry is not valid \n"));
        }
        else
            skernelFatalError("Illegal state ,relativeBankOffset [%d] have valid bit 0 \n",
                              relativeBankOffset);
        return GT_FALSE;
    }

    if(relativeBankOffset == 0) /*we are in the first bank */
    {
        parsedKeySize = (exactMatchParsedEntryPtr->key_size > 3)?
                         (exactMatchParsedEntryPtr->key_size - 3):
                         exactMatchParsedEntryPtr->key_size;
        /*EM key and action can be only in relativeBankOffset 0 */
        if ( (exactMatchParsedEntryPtr->entry_type!=0)           ||
             (exactMatchParsedEntryPtr->lookup_number != hitNum) ||
             (parsedKeySize != exactMatchNewKeySize) )
        {
            __LOG(("No hit. One or more of these parameters does not match search data\n"
                   "\tParsed Val: entryType[%u], lookup_num[%u], key_size[%u]\n"
                   "\tExpected Val: entryType[0] hitNum [%u] keySize [%u] \n",
                   exactMatchParsedEntryPtr->entry_type, exactMatchParsedEntryPtr->lookup_number,
                   parsedKeySize, hitNum, exactMatchNewKeySize));
            return GT_FALSE;
        }
        originalKey[0] = snetFieldValueGet(exactMatchNewKeyDataPtr,0, 32);
        originalKey[1] = snetFieldValueGet(exactMatchNewKeyDataPtr,32, 8);

        if ((originalKey[0]== exactMatchParsedEntryPtr->key_31_0  ) &&
            (originalKey[1]== exactMatchParsedEntryPtr->key_39_32 )    )
        {
            __LOG(("Match in relative bank [%d],hitNum [%d] \n",
                   relativeBankOffset,hitNum));
            return GT_TRUE;
        }
        __LOG(("No match in relative bank [%d] \n",
               relativeBankOffset));
    }
    else
    {
        /*EM key can be only in relativeBankOffset > 0 */
        if (exactMatchParsedEntryPtr->entry_type !=1 )
        {
            skernelFatalError("Entry_type [%d] was not expected when relativeBankOffset [%d] \n",
                               exactMatchParsedEntryPtr->entry_type,relativeBankOffset);
            return GT_FALSE;
        }
        /*40 for the first bank and 130 per bank */
        globleBitOffset= 40+((relativeBankOffset-1)* SIP7_EXACT_MATCH_KEY_ONLY_FIELD_SIZE_CNS);

        originalKey[0] = snetFieldValueGet(exactMatchNewKeyDataPtr, globleBitOffset+0  ,32);
        originalKey[1] = snetFieldValueGet(exactMatchNewKeyDataPtr, globleBitOffset+32 ,32);
        originalKey[2] = snetFieldValueGet(exactMatchNewKeyDataPtr, globleBitOffset+64 ,32);
        originalKey[3] = snetFieldValueGet(exactMatchNewKeyDataPtr, globleBitOffset+96 ,32);
        if ((originalKey[0] == exactMatchParsedEntryPtr->keyOnly_31_0  )&&
            (originalKey[1] == exactMatchParsedEntryPtr->keyOnly_63_32 )&&
            (originalKey[2] == exactMatchParsedEntryPtr->keyOnly_95_64 )&&
            (originalKey[3] == exactMatchParsedEntryPtr->keyOnly_127_96))
        {
            __LOG(("Match in relative bank [%d],hitNum [%d] \n",
            relativeBankOffset,hitNum));
            return GT_TRUE;
        }
        __LOG(("No match in relative bank [%d] \n",
               relativeBankOffset));
    }
    return GT_FALSE;
}

/**
* @internal sip7ExactMatchIsHit function
* @endinternal
*
* @brief   return GT_TRUE incase there was a hit in the Exact match table
*
* @param[in] devObjPtr                  - (pointer to) the device object
* @param[in] hitNum                     - hit number (0,1)
* @param[in] exactMatchNewKeyDataPtr    - EM-Key data
* @param[in] exactMatchNewKeySizePtr    - EM-Key size
* @param[out] matchIndexPtr             - pointer to match Index
*/
static GT_BOOL sip7ExactMatchIsHit
(
    IN  SKERNEL_DEVICE_OBJECT                *devObjPtr,
    IN  SIP7_EXACT_MATCH_UNIT_ENT            emUnitNum,
    IN  GT_U32                               hitNum,
    IN  GT_U32                               *exactMatchNewKeyDataPtr,
    IN  SIP7_EXACT_MATCH_KEY_SIZE_ENT        exactMatchNewKeySize,
    OUT GT_U32                               *matchIndexPtr
)
{
    GT_U32 numOfValidBanks = 0;
    GT_U32 multiHashIndexArr[SIP7_EXACT_MATCH_MAX_NUM_BANKS_CNS] = {0};
    GT_U32 bank=0 ,relativeBank=0;
    GT_U32 regAddr , ageEn =0 ;
    GT_BOOL hit = GT_FALSE;
    GT_U32 * entryPtr , * firstBankEntryPtr=NULL ; /* Exact Match table entry pointer */
    SNET_SIP7_EXACT_MATCH_ENTRY_INFO exactMatchEntryInfo ;
    GT_U32 emTableId;

    DECLARE_FUNC_NAME(sip7ExactMatchIsHit);
    __LOG(("Started Exact Match hit on EM unit[%u] on Channel[%u]\n", emUnitNum, hitNum));

    emTableId = CONVERT_EM_UNIT_TO_EM_TABLE_ID(emUnitNum, hitNum)
    __LOG(("Converted EM table ID [%u]\n", emTableId));

    smacHashAasExactMatchMultiHashResultsCalc(devObjPtr, emUnitNum, exactMatchNewKeyDataPtr, exactMatchNewKeySize,
                                              &multiHashIndexArr[0], &numOfValidBanks);

    regAddr = SMEM_SIP7_EXACT_MATCH_GLOBAL_CONFIGURATION_REG(devObjPtr, emTableId, emGlobalConfiguration1);
    smemRegFldGet(devObjPtr, regAddr, 5, 1, &ageEn);

    for (bank = 0; bank < numOfValidBanks; bank+=(exactMatchNewKeySize+1))
    {
        for ( relativeBank = 0 ;relativeBank <= (GT_U32)exactMatchNewKeySize ;relativeBank ++ )
        {
            memset(&exactMatchEntryInfo, 0, sizeof(SNET_SIP7_EXACT_MATCH_ENTRY_INFO));
            /* Get entryPtr according to entry index */

            if (emUnitNum == SIP7_EXACT_MATCH_UNIT_REDUCED_E)
            {
                entryPtr = smemMemGet(devObjPtr, SMEM_SIP7_REDUCED_EM_TBL_MEM(devObjPtr, multiHashIndexArr[bank+relativeBank]));
            }
            else
            {
                entryPtr = SMEM_SIP7_HIDDEN_EXACT_MATCH_PTR(devObjPtr, emTableId, multiHashIndexArr[bank+relativeBank]);
            }
            __LOG(("EntryPtr for EM table ID [%u] is [0x%X]\n", emTableId, entryPtr));
            snetAasExactMatchParseEntry(devObjPtr, entryPtr, multiHashIndexArr[bank+relativeBank], &exactMatchEntryInfo);

            if (relativeBank == 0)
            {
                firstBankEntryPtr = entryPtr ;
            }

            /* TBD: Update the Channel mask field in the key comparison */
            hit = exactMatchKeyComparison(devObjPtr,exactMatchNewKeyDataPtr,(GT_U32) exactMatchNewKeySize,
                                          &exactMatchEntryInfo, hitNum,
                                          relativeBank /*relative bank ofset from the "n" bank*/);

            if (!hit)
            {
                __LOG(("No hit bank index [0x%x] \n",
                       multiHashIndexArr[bank+relativeBank]));
                break;
            }

            if (relativeBank ==(GT_U32) exactMatchNewKeySize) /*hit is contuinuous GT_TRUE in all checked bank */
            {
                *matchIndexPtr=multiHashIndexArr[bank];
                if (ageEn)
                {
                    if(firstBankEntryPtr == NULL)
                    {
                        skernelFatalError("firstBankEntryPtr == NULL \n");
                        return GT_FALSE;
                    }

                    SMEM_SIP7_EXACT_MATCH_ENTRY_FIELD_SET(devObjPtr, firstBankEntryPtr, multiHashIndexArr[bank],
                                 SMEM_SIP7_EXACT_MATCH_TABLE_FIELDS_EM_ACTION_AGE,1);
                }
                __LOG(("Hit bank index [0x%x] \n",
                       multiHashIndexArr[bank]));
                return GT_TRUE;
            }
        }/*end for relativeBank loop*/
    }/*end for bank loop*/

    return GT_FALSE;
}

/**
* @internal sip7ExactMatchProfileTableMaskDataGet function
* @endinternal
*
* @brief   return Profile Table Mask Data according to the ProfileID
*
* @param[in] devObjPtr                - (pointer to) the device object
* @param[in] emUnitNum                - Exact Match unit number
* @param[in] exactMatchProfileId      - index to the exact match
*                                       profiles table
* @param[out]maskDataPtr              - pointer to Exact
*                                       Match Profile table mask
*
*/
static GT_U32 sip7ExactMatchProfileTableMaskDataGet
(
    IN  SKERNEL_DEVICE_OBJECT       *devObjPtr,
    IN  GT_U32                      emUnitNum,
    IN  GT_U32                      emProfileId,
    OUT GT_U32                      *maskDataPtr
)
{
    GT_U32  regAddr = 0;
    GT_U32  wordNum = 0;

    /* 384 bits of data for each entry mask */
    for (wordNum = 0; wordNum < SIP7_EXACT_MATCH_MAX_KEY_SIZE_WORD_CNS; wordNum++)
    {
        regAddr = SMEM_SIP7_EXACT_MATCH_PROFILE_TABLE_MASK_DATA_REG(devObjPtr, emUnitNum, emProfileId, wordNum);
        smemRegFldGet(devObjPtr, regAddr, 0, 32, &maskDataPtr[wordNum]);
    }

    return GT_OK;
}

/**
* @internal sip7ExactMatchBuildNewKey function
* @endinternal
*
* @brief   return the new Exact match Key build from the original
*          TCAM-Key, according to the ProfileID
*
* @param[in] devObjPtr                   - (pointer to) the device object
* @param[in] exactMatchProfileId         - index to the exact
*                                          match profiles table
* @param[in] keyDataPtr                  - TCAM-Key data
* @param[in] keySize                     - TCAM-Key size
* @param[in] exactMatchNewKeyDataPtr     - EM-Key data
* @param[in] exactMatchNewKeySizePtr     - EM-Key size
*
*/
static GT_U32 sip7ExactMatchBuildNewKey
(
    IN  SKERNEL_DEVICE_OBJECT            *devObjPtr,
    IN  GT_U32                           emUnitNum,
    IN  GT_U32                           emProfileId,
    IN  GT_U32                           *superKeyDataPtr,
    OUT GT_U32                           *exactMatchNewKeyDataPtr,
    OUT SIP7_EXACT_MATCH_KEY_SIZE_ENT    *exactMatchNewKeySizePtr
)
{
    GT_U32 exactMatchProfileTableMaskDataArray[SIP7_EXACT_MATCH_MAX_KEY_SIZE_WORD_CNS];/* 448 bits for the mask */
    GT_U32 word, select, regVal, regAddr;
    GT_U32 muxBytePairMap, muxBytePairIndex, muxBytePairVal;
    GT_U32 exactMatchKeysize = 0;
    GT_U32 bytesSelected = 0;
    DECLARE_FUNC_NAME(sip7ExactMatchBuildNewKey);
    __LOG(("Makes new Exact Match key from TCAM super key \n"));

    /* Get the configured EM key size */
    regAddr = SMEM_SIP7_EXACT_MATCH_PROFILE_CONFIG_REG(devObjPtr, emUnitNum, emProfileId);
    smemRegFldGet(devObjPtr, regAddr, 1, 2, &regVal);
    *exactMatchNewKeySizePtr = SMEM_SIP7_EXACT_MATCH_HW_KEY_SIZE_TO_MAC(regVal);

    /* Convert EM key size to bytes */
    exactMatchKeysize = SMEM_SIP7_EXACT_MATCH_KEY_SIZE_TO_BYTE_MAC(*exactMatchNewKeySizePtr);
    __LOG(("Exact Match key size in bytes = [%u]\n", exactMatchKeysize));
    /* Get the key mask from EM profile table */
    sip7ExactMatchProfileTableMaskDataGet(devObjPtr, emUnitNum, emProfileId, &exactMatchProfileTableMaskDataArray[0]);

    /* superKeyDataPtr and exactMatchNewKeyDataPtr are in words
     * exactMatchKeysize is in Bytes, convert it to Words and loop it */
    for (word = 0; word < (exactMatchKeysize/4); word++)
    {
        /* read the 10 byte select register */
        smemRegGet(devObjPtr,
                   SMEM_SIP7_EXACT_MATCH_PROFILE_10_BYTE_SELECT_REG(devObjPtr, emUnitNum, emProfileId, word),
                   &regVal);
        /* Each 10 Byte select word is in 2 Byte resolution
         * For each EM Profile entry
	     * Each line represents 10Bytes (each byte represents byte pairs)
	     *      0 - 00 000000 000000 000000 000000 000000    (30b)
	     *      1 - ...
	     *      2 - ...
	     *      3 - ...
	     *      4 - ...
         *      5 - ...
         * Each 6 bits represents pair of byte selection from TCAM superkey
         * If 0 then bytes 0 and 1 selected
         * If 1 then bytes 2 and 3 selected
         * 	…
         * If x then bytes 2*x and (2*x)+1 are selected
         * So max of x=59, which selects bytes 118 and 119, max superkey size is 120 Bytes
         */
        for (select = 0; (select < 5); select++)
        {
            if (bytesSelected >= exactMatchKeysize)
            {
                break;
            }
            /* extract the 6 bits */
            muxBytePairMap   = (regVal >> (6 * select)) & 0x3F;
            /* calculate the byte pair index */
            muxBytePairIndex = (word * 5) + select;
            /* Extract the byte pair value from superkey (in words) */
            muxBytePairVal   = (superKeyDataPtr[muxBytePairMap / 2] >> ((muxBytePairMap % 2) * 16)) & 0xFFFF;
            /* Copy the extracted value in the new EM key (in words) */
            exactMatchNewKeyDataPtr[muxBytePairIndex / 2] |= muxBytePairVal << ((muxBytePairIndex % 2) * 16);
            /* each slection will select 2 bytes from superKey */
            bytesSelected += 2;
        }
        __LOG(("Exact Match Key Data before mask word[%u] = [0x%x], Mask Array word[%u] = [0x%x], bytesSelected = [%u]\n",
              word, exactMatchNewKeyDataPtr[word], word, exactMatchProfileTableMaskDataArray[word], bytesSelected));
        /*Mask the key*/
        exactMatchNewKeyDataPtr[word] &= exactMatchProfileTableMaskDataArray[word];
        __LOG(("Exact Match Key Data after mask word[%u] = [0x%x]\n",
               word, exactMatchNewKeyDataPtr[word]));
    }
    return GT_OK;
}

/**
* @internal sip7ExactMatchDoLookup function
* @endinternal
*
* @brief   sip6 function that does sequential lookup on all banks
*
* @param[in] devObjPtr                  - (pointer to) the device object
* @param[in] tcamClient                 - Exact match client
* @param[in] keyArrayPtr                - key data array
* @param[in] keySize                    - size of the key
* @param[in] exactMatchProfileIdArr     - Exact Match lookup profile ID array
* @param[out] exactMatchClientMatchArr  - if client is configured as the client of the Exact match lookup
* @param[out] resultArr                 - Exact Match hits number of results
*/
static GT_U32 sip7ExactMatchDoLookup
(
    IN  SKERNEL_DEVICE_OBJECT        *devObjPtr,
    IN  SIP7_EMX_CLIENT_ENT          emxClient,
    IN  GT_U32                       *keyArrayPtr,
    IN  SIP5_TCAM_KEY_SIZE_ENT       keySize,
    IN  GT_U32                       exactMatchProfileIdArr[],
    OUT SIP7_EXACT_MATCH_UNIT_ENT     emUnitNumArr[],
    OUT GT_BOOL                      exactMatchClientMatchArr[],
    OUT GT_U32                       resultArr[]
)
{
    DECLARE_FUNC_NAME(sip7ExactMatchDoLookup);
    GT_BOOL clientFound = GT_FALSE;
    SIP7_EXACT_MATCH_KEY_SIZE_ENT  exactMatchNewKeySize;
    GT_U32  matchIndex=0;
    GT_U32  hitsCounter = 0;
    GT_BOOL gotHit = GT_FALSE;
    GT_U32  hitNum;
    /* the new key that will be used for Exact match table - 448 bits*/
    GT_U32  exactMatchNewKeyArray[SIP7_EXACT_MATCH_MAX_KEY_SIZE_WORD_CNS] = {0};
    GT_CHAR* clientNamePtr = ((emxClient < SIP7_EMX_CLIENT_LAST_E) ? emxClientName[emxClient] : "unknown");

    for(hitNum = 0; hitNum < SIP6_EXACT_MATCH_MAX_NUM_OF_HITS_CNS; hitNum++)
    {
        __LOG(("Start Exact match lookup for EMX Client [%s] with hitNum [%u] \n", clientNamePtr, hitNum));

        /* need to reset the New Key calculated between each lookup */
        memset(exactMatchNewKeyArray, 0, sizeof(exactMatchNewKeyArray));

        /* Do not trigger Exact match lookup if the profile ID is 0 */
        if (exactMatchProfileIdArr[hitNum] == 0)
        {
            /*init for the caller that no match for this lookup*/
            resultArr[hitNum] = SNET_CHT_POLICY_NO_MATCH_INDEX_CNS;
            __LOG(("for Exact match hitNum [%d]: NO hit, The EMX client[%s] do not trigger the EM lookup with 'EM Profile ID' != 0. \n",
                  hitNum, clientNamePtr));
            continue;
        }

        /* Get EM unit number from EMX Client */
        clientFound = snetAasExactMatchUnitNumGet(devObjPtr, emxClient, hitNum, &emUnitNumArr[hitNum]);
        if (clientFound == GT_FALSE)
        {
            /*No match for this lookup*/
            resultArr[hitNum] = SNET_CHT_POLICY_NO_MATCH_INDEX_CNS;
            __LOG(("EM unit not configured for EMX Client [%s] with lookupNum [%u]\n",
                  clientNamePtr, hitNum));
            exactMatchClientMatchArr[hitNum]= GT_FALSE ;
            continue;
        }

        /* If we get valid EM unit the mark this entry as GT_TRUE */
        exactMatchClientMatchArr[hitNum]= GT_TRUE ;
        __LOG(("Generating the EM lookup key from SuperKey for EMX Client [%s] with hitNum [%u] \n", clientNamePtr, hitNum));
        /* Generate the EM key from TCAM Super key */
        sip7ExactMatchBuildNewKey(devObjPtr, emUnitNumArr[hitNum], exactMatchProfileIdArr[hitNum], keyArrayPtr,
                                  &exactMatchNewKeyArray[0], &exactMatchNewKeySize);

        __LOG(("exactMatchNewKeySize [%d] \n", exactMatchNewKeySize));
        __LOG(("exactMatchNewKeyArray_0 [0x%X],exactMatchNewKeyArray_1 [0x%X],exactMatchNewKeyArray_2 [0x%X] \n",
              exactMatchNewKeyArray[0],exactMatchNewKeyArray[1],exactMatchNewKeyArray[2]));
        __LOG(("exactMatchNewKeyArray_3 [0x%X],exactMatchNewKeyArray_4 [0x%X],exactMatchNewKeyArray_5 [0x%X] \n",
              exactMatchNewKeyArray[3],exactMatchNewKeyArray[4],exactMatchNewKeyArray[5]));
        __LOG(("exactMatchNewKeyArray_6 [0x%X],exactMatchNewKeyArray_7 [0x%X],exactMatchNewKeyArray_8 [0x%X] \n",
              exactMatchNewKeyArray[6],exactMatchNewKeyArray[7],exactMatchNewKeyArray[8]));
        __LOG(("exactMatchNewKeyArray_9 [0x%X],exactMatchNewKeyArray_10 [0x%X],exactMatchNewKeyArray_11 [0x%X] \n",
              exactMatchNewKeyArray[9],exactMatchNewKeyArray[10],exactMatchNewKeyArray[11]));
        __LOG(("exactMatchNewKeyArray_12 [0x%X],exactMatchNewKeyArray_13 [0x%X]\n",
              exactMatchNewKeyArray[12],exactMatchNewKeyArray[13]));

        /* use the new key to get a match in the Exact match entry */
        gotHit = sip7ExactMatchIsHit(devObjPtr, emUnitNumArr[hitNum], hitNum, &exactMatchNewKeyArray[0],
                                     exactMatchNewKeySize, &matchIndex);

        if(gotHit)
        {
            resultArr[hitNum] = matchIndex;
            hitsCounter++;
            __LOG(("for Exact match hitNum [%d]: got hit: matchIndex [%d] \n", hitNum, matchIndex));
        }
        else
        {
            /* TBD : EM Auto learn code */

            /* No match for this lookup*/
            resultArr[hitNum] = SNET_CHT_POLICY_NO_MATCH_INDEX_CNS;
            __LOG(("for Exact match hitNum [%d]: NO hit \n", hitNum));
        }
    }
    return hitsCounter;
}

/**
* @internal snetAasExactMatchLookup function
* @endinternal
*
* @brief   For a given key, perform the lookup in Exact match table and
*          fill the results array
*
* @param[in]  devObjPtr                      - pointer to device object.
* @param[in]  emxClient                      - EMX Client
* @param[in]  keyArrayPtr                    - superkey array (size up to 120 bytes)
* @param[in]  keySize                        - size of the EM key
* @param[in]  exactMatchProfileIdArr         - Exact Match lookup profile ID array
* @param[in]  emUnitNumArr                   - Array of EM unit numbers
* @param[out] exactMatchClientMatchArr       - if client is configured as the client of the Exact match lookup
* @param[out] resultArr                      - Exact Match hits number of results
*
*/
GT_U32 snetAasExactMatchLookup
(
    IN  SKERNEL_DEVICE_OBJECT             *devObjPtr,
    IN  SIP7_EMX_CLIENT_ENT               emxClient,
    IN  GT_U32                            *keyArrayPtr,
    IN  SIP5_TCAM_KEY_SIZE_ENT            keySize,
    IN  GT_U32                            exactMatchProfileIdArr[],
    OUT SIP7_EXACT_MATCH_UNIT_ENT         *emUnitNumPtr,
    OUT GT_BOOL                           exactMatchClientMatchArr[],
    OUT GT_U32                            resultArr[]
)
{
    DECLARE_FUNC_NAME(snetAasExactMatchLookup);

    GT_U32      numOfHits;
    GT_CHAR* clientNamePtr = ((emxClient < SIP7_EMX_CLIENT_LAST_E) ? emxClientName[emxClient] : "unknown");

    __LOG(("Start Exact Match check for [%s] \n", clientNamePtr));

    if((exactMatchProfileIdArr[0] == 0 ) && (exactMatchProfileIdArr[1] == 0))
    {
        __LOG(("Both the EM Profile IDs are 0\n"));
        numOfHits = 0;
        resultArr[0] = SNET_CHT_POLICY_NO_MATCH_INDEX_CNS;
        resultArr[1] = SNET_CHT_POLICY_NO_MATCH_INDEX_CNS;
        return numOfHits;
    }

        /* sequential lookup on all segments */
    numOfHits = sip7ExactMatchDoLookup(devObjPtr, emxClient,
                                       keyArrayPtr, keySize,
                                       exactMatchProfileIdArr,
                                       emUnitNumPtr,
                                       exactMatchClientMatchArr,
                                       resultArr);

   __LOG_PARAM(emUnitNumPtr[0]);
   __LOG_PARAM(emUnitNumPtr[1]);
   __LOG_PARAM(resultArr[0]);
   __LOG_PARAM(resultArr[1]);
   __LOG_PARAM(exactMatchClientMatchArr[0]);
   __LOG_PARAM(exactMatchClientMatchArr[1]);

    return numOfHits;
}

/**
* @internal snetAasExactMatchLookupInEmx function
* @endinternal
*
* @brief   For a given key, perform the lookup in Exact match table and
*          fill the results array
*
* @param[in] devObjPtr                      - pointer to device object.
* @param[in] descrPtr                       - pointer to frame descriptor.
* @param[in] emxClient                      - EMX Client
* @param[in] exactMatchProfileIdTableIndex  - index to the EM Profile table
* @param[in] keyArrayPtr                    - superkey array (size up to 120 bytes)
* @param[in] keySize                        - size of the TCAM key
* @param[in] matchIndexArr                  - Matched lookup index array filled by TCAM
* @param[in] isMatchDoneInTcamOrEmArr       - Array to denote hit by TCAM or EM
* @param[in] tcamOverExactMatchPriorityArr  - Array to denote priority is TCAM or EM
*/
GT_U32 snetAasExactMatchLookupInEmx
(
    IN  SKERNEL_DEVICE_OBJECT             *devObjPtr,
    IN  SKERNEL_FRAME_CHEETAH_DESCR_STC   *descrPtr,
    IN  SIP7_EMX_CLIENT_ENT               emxClient,
    IN  GT_U32                            exactMatchProfileIdTableIndex,
    IN  GT_U32                            *keyArrayPtr,
    IN  SIP5_TCAM_KEY_SIZE_ENT            tcamKeySize,
    IN  GT_U32                            matchIndexArr[],
    IN  GT_BOOL                           isMatchDoneInTcamOrEmArr[],
    IN  GT_BOOL                           tcamOverExactMatchPriorityArr[]
)
{
    DECLARE_FUNC_NAME(snetAasExactMatchLookupInEmx);
    GT_U32                    exactMatchIndexArr[SIP6_EXACT_MATCH_MAX_NUM_OF_HITS_CNS];
    GT_BOOL                   exactMatchClientMatchArr[SIP6_EXACT_MATCH_MAX_NUM_OF_HITS_CNS]={GT_FALSE,GT_FALSE};
    GT_U32                    exactMatchProfileIdArr[SIP6_EXACT_MATCH_MAX_NUM_OF_HITS_CNS];
    GT_U32                    exactMatchNumOfHits=0;
    GT_U32                    regAddr, regVal;
    GT_U32                    exactMatchProfileTableEnableDefault;
    GT_U32                    hitNum;
    SIP7_EXACT_MATCH_UNIT_ENT  emUnitNumArr[SIP6_EXACT_MATCH_MAX_NUM_OF_HITS_CNS];

    simLogPacketFrameUnitSet(SIM_LOG_FRAME_UNIT_EMX_E);

    /*Get exact match profile id */
    snetAasExactMatchProfileIdGet(devObjPtr, descrPtr, SIP7_EMX_CLIENT_TTI_E,
                                  exactMatchProfileIdTableIndex, &exactMatchProfileIdArr[0]);

    __LOG_PARAM(exactMatchProfileIdArr[0]);
    __LOG_PARAM(exactMatchProfileIdArr[1]);

    if ((exactMatchProfileIdArr[0] != 0) || (exactMatchProfileIdArr[1] != 0))
    {
        /* EM lookup for TTI Client */
        exactMatchNumOfHits = snetAasExactMatchLookup(devObjPtr, emxClient,
                                                      keyArrayPtr, tcamKeySize,
                                                      exactMatchProfileIdArr, emUnitNumArr,
                                                      exactMatchClientMatchArr, exactMatchIndexArr);
        for (hitNum = 0; hitNum < SIP6_EXACT_MATCH_MAX_NUM_OF_HITS_CNS; hitNum++)
        {
            __LOG(("For HitNum[%u], exactMatchClientMatchArr[%u] = [0x%x], matchIndexArr[%u] = [0x%x]\n",
                   hitNum, hitNum, exactMatchClientMatchArr[hitNum], hitNum, matchIndexArr[hitNum]));
            if (exactMatchClientMatchArr[hitNum]==GT_TRUE)
            {
                if (matchIndexArr[hitNum] != SNET_CHT_POLICY_NO_MATCH_INDEX_CNS) /* Hit in TCAM case */
                {
                    __LOG(("For hitNum[%u], exactMatchIndexArr[%u] = [0x%x], tcamOverExactMatchPriorityArr[%u] = [0x%x]\n",
                           hitNum, hitNum, exactMatchIndexArr[hitNum], hitNum, tcamOverExactMatchPriorityArr[hitNum]));
                    if((exactMatchIndexArr[hitNum] != SNET_CHT_POLICY_NO_MATCH_INDEX_CNS) && /* there is hit in EM */
                        (tcamOverExactMatchPriorityArr[hitNum] == GT_FALSE))/* EM have priority */
                    {
                        __LOG(("take exact match index - reason : got priority \n "));
                        matchIndexArr[hitNum]=exactMatchIndexArr[hitNum];
                        isMatchDoneInTcamOrEmArr[hitNum]=GT_FALSE;
                    }
                }
                else /* No hit in TCAM case */
                {
                    __LOG(("No hit in TCAM\n"));
                    if(exactMatchIndexArr[hitNum]!=SNET_CHT_POLICY_NO_MATCH_INDEX_CNS ) /* there is hit in em*/
                    {
                        __LOG(("take exact match index - reason : no Tcam hit  \n "));
                        matchIndexArr[hitNum]=exactMatchIndexArr[hitNum];
                        isMatchDoneInTcamOrEmArr[hitNum]=GT_FALSE;
                    }
                    else /*no hit in EM , check for default action*/
                    {
                        __LOG(("No hit in EM , check for default action\n"));
                        if (exactMatchProfileIdArr[hitNum]!= 0)
                        {
                            /* Get the configured Default action enable flag */
                            regAddr = SMEM_SIP7_EXACT_MATCH_PROFILE_CONFIG_REG(devObjPtr, emUnitNumArr[hitNum],
                                                                               exactMatchProfileIdArr[hitNum]);
                            smemRegFldGet(devObjPtr, regAddr, 0, 1, &regVal);
                            exactMatchProfileTableEnableDefault = (regVal & 0x1);

                            if (exactMatchProfileTableEnableDefault==GT_TRUE)
                            {
                                __LOG(("take default action \n "));
                                isMatchDoneInTcamOrEmArr[hitNum] = GT_FALSE;
                                matchIndexArr[hitNum] = (hitNum == 0) ?  SNET_SIP6_EXACT_MATCH_DEFAULT_INDEX_LOOKUP0_CNS :
                                                        SNET_SIP6_EXACT_MATCH_DEFAULT_INDEX_LOOKUP1_CNS ;
                                exactMatchNumOfHits++;
                            }
                            else
                            {
                                __LOG(("Default Action is not enabled for EM profile ID [%u] in EM unit [%u]\n",
                                       exactMatchProfileIdArr[hitNum], emUnitNumArr[hitNum]));
                            }
                        }
                    }
                }
            } /* end if exactMatchClientMatchArr 0 or 1 == GT_TRUE*/
        } /* end for loop */
    } /* if profileId 0 or 1 != 0*/
    else
    {
        __LOG(("profileId1 and profileId2 set to 0 - Skipping Exact Match engine  \n "));
    }
    return exactMatchNumOfHits;
}

/**
* @internal snetAasSerialEmProfileParamsGet function
* @endinternal
*
* @brief   Gets the Serial EM Lookup Profile EM0/EM1/EM2 for EMX Clients
*
* @param[in] devObjPtr                      - pointer to device object.
* @param[in] descrPtr                       - pointer to frame descriptor.
* @param[in] emxClient                      - EMX client
* @param[in] serialEmPortProfileId          - Serial EM Port Profile Id
* @param[out] serialEmProfileParamsPtr      - Pointer to SIP7_SERIAL_EM_ENTRY_STC
*
*/
GT_VOID snetAasSerialEmProfileParamsGet
(
    IN  SKERNEL_DEVICE_OBJECT             *devObjPtr,
    IN  SKERNEL_FRAME_CHEETAH_DESCR_STC   *descrPtr,
    IN  SIP7_EMX_CLIENT_ENT               emxClient,
    IN  SIP7_SERIAL_EM_TYPE_ENT           serialEm,
    IN  GT_U32                            serialEmPortProfileId,
    OUT SIP7_SERIAL_EM_ENTRY_STC          *serialEmProfileParamsPtr
)
{
    DECLARE_FUNC_NAME(snetAasSerialEmProfileParamsGet);
    GT_U32 packetType;
    GT_U32 serialEmProfileIdIndex;
    GT_U32 regVal;

    /* Get the packet type from the Descriptor */
    if (emxClient == SIP7_EMX_CLIENT_TTI_SERIAL_EM_E)
    {
        packetType = (descrPtr->tti_pcktType_sip5 & 0xF);
        __LOG_PARAM(packetType);
    }
    else
    {
        __LOG(("Serial EM lookup is supported only for TTI ILM Client\n"));
        return;
    }

    /* Construct the serialEmProfile index */
    /* Bits [6:3]= Packet Type */
    /* Bits [2:0]= Default EPort table <Port Profile ID>[2:0] */
    serialEmProfileIdIndex = (packetType << 3) | (serialEmPortProfileId & 0x7);
    __LOG_PARAM(serialEmProfileIdIndex);

    /* Read the entry from Serial EM Profile Id Table */
    smemRegGet(devObjPtr, SMEM_SIP7_TTI_LU_SERIAL_EM_PROFILE_REG(devObjPtr, serialEm, serialEmProfileIdIndex), &regVal);
    __LOG_PARAM(regVal);

    /* Extract the Profile Id,Key Type, PCL ID, UDB28/29 Vlan Tag flags from the word
     * Profile Id            bits 0...3
     * Key Type              bits 4...5
     * Enable UDB29 VLAN Tag bit  6
     * Enable UDB28 VLAN Tag bit  7
     * PCL ID                bits 8...9
     */
    serialEmProfileParamsPtr->serialEmProfileId             = (regVal & 0xF);
    serialEmProfileParamsPtr->serialEmKeyType               = (regVal >> 4) & 0x3;
    serialEmProfileParamsPtr->serialEmEnableUDB29VlanTagKey = (regVal >> 6) & 0x1;
    serialEmProfileParamsPtr->serialEmEnableUDB28VlanTagKey = (regVal >> 7) & 0x1;
    serialEmProfileParamsPtr->serialEmPclId                 = (regVal >> 8) & 0x3;

    if (serialEmProfileParamsPtr->serialEmProfileId == 0)
    {
        /* Skipping the inactive params dump */
        return;
    }

    __LOG_PARAM(serialEmProfileParamsPtr->serialEmProfileId);
    __LOG_PARAM(serialEmProfileParamsPtr->serialEmKeyType);
    __LOG_PARAM(serialEmProfileParamsPtr->serialEmEnableUDB29VlanTagKey);
    __LOG_PARAM(serialEmProfileParamsPtr->serialEmEnableUDB28VlanTagKey);
    __LOG_PARAM(serialEmProfileParamsPtr->serialEmPclId);
}

/**
* @internal snetAasExactMatchProfileIdGet function
* @endinternal
*
* @brief   Gets Exact Match Lookup Profile ID1/ID2 for EMX Clients
*
* @param[in] devObjPtr                      - pointer to device object.
* @param[in] descrPtr                       - pointer to frame descriptor.
* @param[in] emxClient                      - EMX client
* @param[in] index                          - keyType in case of TTI
*                                             or index for Exact match Profile-ID mapping table
*                                             in case of PCL/EPCL
* @param[out] exactMatchProfileIdArr        - Exact Match Lookup
*                                             Profile ID Array
*/
GT_VOID snetAasExactMatchProfileIdGet
(
    IN  SKERNEL_DEVICE_OBJECT             *devObjPtr,
    IN  SKERNEL_FRAME_CHEETAH_DESCR_STC   *descrPtr ,
    IN  SIP7_EMX_CLIENT_ENT               emxClient,
    IN  GT_U32                            index,
    OUT GT_U32                            exactMatchProfileIdArr[]
)
{
    DECLARE_FUNC_NAME(snetAasExactMatchProfileIdGet);
    GT_U32  regAddr,profileId1=0,profileId2=0;
    GT_U32 * regPtr;/* register entry pointer */
    GT_CHAR* clientNamePtr = ((emxClient < SIP7_EMX_CLIENT_LAST_E) ? emxClientName[emxClient] : "unknown");

    if(SMEM_CHT_IS_SIP7_GET(devObjPtr))
    {
        switch (emxClient)
        {
            case SIP7_EMX_CLIENT_TTI_E:
                /* keyIndex: 0-19 */
                if(index <= SKERNEL_LION3_TTI_KEY_UDB_PKT_TYPE_15_E)
                {
                    GT_U32 regVal;
                    /* get profileId1, profileId2 */
                    regAddr = SMEM_SIP6_TTI_PACKET_TYPE_EXACT_MATCH_PROFILE_ID_REG(devObjPtr, (index)/2);
                    smemRegGet(devObjPtr,regAddr , &regVal);

                    /* 4 bits for profileId1 (bits 0-3 for even index, bits 8-11 for odd index)
                    4 bits for profileId2 (bits 4-7 for even index, bits 12-15 for odd index)*/
                    profileId1 = SMEM_U32_GET_FIELD(regVal,8*(index%2), 4);
                    profileId2 = SMEM_U32_GET_FIELD(regVal,4 + 8*(index%2), 4);

                }
                break;
            case SIP7_EMX_CLIENT_IPCL1_E:
            case SIP7_EMX_CLIENT_IPCL2_E:
                /* keyIndex: 0-255 */
                if(index < devObjPtr->numofIPclProfileId)
                {
                    GT_U32 regVal;
                    /* get profileId1, profileId2 */
                    regAddr = SMEM_SIP6_PCL_EXACT_MATCH_PROFILE_ID_MAP_REG(devObjPtr, index);

                    smemRegGet(devObjPtr, regAddr, &regVal);

                /* 4 bits for profileId1 (bits 0-3 )
                   4 bits for profileId2 (bits 4-7 )
                */
                    profileId1 = SMEM_U32_GET_FIELD(regVal,0, 4);
                    profileId2 = SMEM_U32_GET_FIELD(regVal,4, 4);
                }
                break;
            case SIP7_EMX_CLIENT_EPCL_E:
                /* sip6    : keyIndex: 0-127 */
                /* sip6_10 : keyIndex: 0-255 */
                if(index < devObjPtr->numofEPclProfileId)
                {
                    /* get profileId1, profileId2 */
                    regAddr = SMEM_SIP6_EPCL_EXACT_MATCH_PROFILE_ID_MAP_TBL_MEM(devObjPtr, index);
                    regPtr = smemMemGet(devObjPtr, regAddr);

                /* 4 bits for profileId1 (bits 0-3 )
                    4 bits for profileId2 (bits 4-7 )
                */
                    profileId1 = snetFieldValueGet(regPtr, 0, 4);
                    profileId2 = snetFieldValueGet(regPtr, 4, 4);
                }
                break;
            case SIP7_EMX_CLIENT_MPCL_E:
                /* sip6    : keyIndex: 0-127 */
                /* sip6_10 : keyIndex: 0-255 */
                if(index < devObjPtr->numofMPclProfileId)
                {
                    /* get profileId1, profileId2 */
                    regAddr = SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->MPCL.exactMatchProfileIdMapEntry[index];
                    regPtr = smemMemGet(devObjPtr, regAddr);

                /* 4 bits for profileId1 (bits 0-3 )
                    4 bits for profileId2 (bits 4-7 )
                */
                    profileId1 = snetFieldValueGet(regPtr, 0, 4);
                    profileId2 = snetFieldValueGet(regPtr, 4, 4);
                }
                break;
            default:
                __LOG(("invalid client\n"));
                return;
        }
    }

    __LOG_PARAM(profileId1);
    __LOG_PARAM(profileId2);

    if(emxClient == SIP7_EMX_CLIENT_TTI_E && descrPtr)
    {
        GT_U32  port_profileId1,port_profileId2;
        GT_U32  index_profileId1,index_profileId2;

        port_profileId1 = SMEM_LION3_TTI_PHYSICAL_PORT_ENTRY_FIELD_GET(devObjPtr, descrPtr,
            SMEM_SIP6_10_TTI_PHYSICAL_PORT_TABLE_FIELDS_EM_LOOKUP_PROFILE_ID1);
        port_profileId2 = SMEM_LION3_TTI_PHYSICAL_PORT_ENTRY_FIELD_GET(devObjPtr, descrPtr,
            SMEM_SIP6_10_TTI_PHYSICAL_PORT_TABLE_FIELDS_EM_LOOKUP_PROFILE_ID2);

        __LOG_PARAM(port_profileId1);
        __LOG_PARAM(port_profileId2);

        switch(SMEM_LION3_TTI_PHYSICAL_PORT_ENTRY_FIELD_GET(devObjPtr, descrPtr,
            SMEM_SIP6_10_TTI_PHYSICAL_PORT_TABLE_FIELDS_EM_PROFILE_ID_MODE))
        {
            case 1:  /*Per Port Assignment*/
                profileId1 = port_profileId1;
                profileId2 = port_profileId2;
                __LOG(("EM TTI : profiles per Port Assignment :profileId1[%d],profileId2[%d] \n",
                    profileId1,profileId2));
                break;
            case 2: /*  Per <Port, Packet type> Assignment */
                /* Bits [6:3]= Packet Type */
                /* Bits [2:0]= Source Physical Port table<Exact Match Lookup Profile-ID1>[2:0] */
                index_profileId1 = ((index & 0xF) << 3) | (port_profileId1 & 0x7);
                index_profileId2 = ((index & 0xF) << 3) | (port_profileId2 & 0x7);
                __LOG_PARAM(index_profileId1);
                __LOG_PARAM(index_profileId2);

                smemRegGet(devObjPtr,
                    SMEM_SIP6_10_TTI_EM_PROFILE_ID_1_MAPPING_TBL_MEM(devObjPtr,index_profileId1),
                    &profileId1);
                smemRegGet(devObjPtr,
                    SMEM_SIP6_10_TTI_EM_PROFILE_ID_2_MAPPING_TBL_MEM(devObjPtr,index_profileId2),
                    &profileId2);

                __LOG(("EM TTI : profiles per <Port, Packet type> Assignment :profileId1[%d],profileId2[%d] \n",
                    profileId1,profileId2));
                break;
            default:/*0/3 no change*/
                __LOG(("EM TTI : profiles per packet type :profileId1[%d],profileId2[%d] \n",
                    profileId1,profileId2));
        }
    }

    exactMatchProfileIdArr[0] = profileId1;
    exactMatchProfileIdArr[1] = profileId2;

    __LOG(("exactMatchProfileId client  = %s \n", clientNamePtr));
    __LOG(("exactMatchProfileId table index = %d \n", index));

    if (exactMatchProfileIdArr[0] == 0)
    {
        __LOG(("exactMatchProfileId1 set to 0 , lookup0 is disabled for client [%s]\n", clientNamePtr));
    }
    else
    {
        __LOG(("exactMatchProfileId1 = %d \n", exactMatchProfileIdArr[0]));
    }

    if (exactMatchProfileIdArr[1] == 0)
    {
        __LOG(("exactMatchProfileId2 set to 0 , lookup1 is disabled for client [%s]\n", clientNamePtr));
    }
    else
    {
        __LOG(("exactMatchProfileId2 = %d \n", exactMatchProfileIdArr[1]));
    }
    return;
}

/**
* @internal snetAasExactMatchProfileTableDefaultActionEntryGet function
* @endinternal
*
* @brief   return the Profile Table default action according to the ProfileID
*
* @param[in]  devObjPtr                - (pointer to) the device object
* @param[in]  emUnitNum                - Exact Match unit number
* @param[in]  exactMatchProfileId      - index to the exact match
*                                        profiles table
* @param[out] defaultActionPtr         - pointer to Exact Match
*                                        Default action
*/
static GT_U32 snetAasExactMatchProfileTableDefaultActionEntryGet
(
    IN  SKERNEL_DEVICE_OBJECT       *devObjPtr,
    IN  SIP7_EXACT_MATCH_UNIT_ENT    emUnitNum,
    IN  GT_U32                      exactMatchProfileId,
    OUT GT_U32                      *defaultActionPtr
)
{
    GT_U32  regAddr=0,wordNum=0;

    /* 384 bits of data for each default entry */
    for (wordNum = 0;wordNum < SIP7_EXACT_MATCH_FULL_ACTION_SIZE_WORD_CNS ;wordNum++)
    {
        regAddr = SMEM_SIP7_EXACT_MATCH_PROFILE_TABLE_DEFAULT_ACTION_DATA_REG(devObjPtr, emUnitNum,
                                                                              exactMatchProfileId, wordNum);
        smemRegFldGet(devObjPtr, regAddr, 0, 32, &defaultActionPtr[wordNum]);
    }

    return GT_OK;
}


/**
* @internal sip7ExactMatchActionAssignmentGet function
* @endinternal
*
* @brief   return origin bit and data from the exact match table
*
* @param[in]  devObjPtr                        - (pointer to) the device object
* @param[in]  expandedActionIndex              - index to action assignment table row (0..15)
* @param[in]  byte                             - the byte offset in the row  (0..30)
* @param[out] originPtr                        - pointer to origin bit :
*                                                0- take from dataptr
*                                                1- take value from exact matchaction
* @param[out] dataPtr                          - pointer to data Depending on origin value
*                                               origin==0 - default value
*                                               origin==1 - byte offset in the exact match action
*/
static GT_VOID sip7ExactMatchActionAssignmentGet
(
    IN  SKERNEL_DEVICE_OBJECT       *devObjPtr,
    IN  SIP7_EXACT_MATCH_UNIT_ENT   emUnitNum,
    IN  GT_U32                      expandedActionIndex,
    IN  GT_U32                      byte,
    OUT GT_U32                      *originPtr,
    OUT GT_U32                      *dataPtr
)
{
    GT_U32  regAddr=0;

    regAddr = SMEM_SIP7_EXACT_MATCH_ACTION_ASSIGNMENT_REG(devObjPtr, emUnitNum, expandedActionIndex, byte);
    smemRegFldGet(devObjPtr, regAddr, 8, 1, originPtr);
    if (*originPtr==0)
    {
        smemRegFldGet(devObjPtr, regAddr, 0, 8, dataPtr);
    }
    else
    {
        smemRegFldGet(devObjPtr, regAddr, 0, 3, dataPtr);
    }
}

/**
* @internal snetAasExactMatchActionExpander function
* @endinternal
*
* @brief   Receives reduced action from the Exact match engine
*          and expands them fully-sized TCAM Action
*
* @param[in]  devObjPtr                  - (pointer to) device object.
* @param[in]  reducedActionPtr           - (pointer to) reduced Exact match action
* @param[in]  expandedActionIndex        -  index to Exact match expander action row (0..15)
* @param[out] fullActionPtr              - (pointer to) full Action after expanding
*/
static GT_VOID snetAasExactMatchActionExpander
(
    IN  SKERNEL_DEVICE_OBJECT              *devObjPtr,
    IN  SIP7_EXACT_MATCH_UNIT_ENT          emUnitNum,
    IN  GT_U32                             *reducedActionPtr,
    IN  GT_U32                             expandedActionIndex,
    OUT GT_U32                             *fullActionPtr
)
{

   GT_U32 byte=0, origin=0, dataOrOffset=0;
   GT_U32 writeOffset=0, byteFromeReducedAction=0;

   for (byte = 0; byte < SIP7_EXACT_MATCH_EXPANDED_ACTION_SIZE_CNS; byte++)
   {
       sip7ExactMatchActionAssignmentGet(devObjPtr, emUnitNum, expandedActionIndex,
                                         byte, &origin, &dataOrOffset);
       if (origin == 0)
       {
           /*write the value from dataOrOffset into fullActionPtr */
           snetFieldValueSet(fullActionPtr, writeOffset, 8, dataOrOffset);
       }
       else
       {   /*write the value from reducedActionPtr with the offset dataOrOffset into fullActionPtr */
           byteFromeReducedAction = snetFieldValueGet(reducedActionPtr,dataOrOffset*8,8);
           snetFieldValueSet(fullActionPtr,writeOffset,8,byteFromeReducedAction);
       }
       writeOffset+=8;
   }
}

/**
* @internal snetAasExactMatchParseEntry function
* @endinternal
*
* @brief   Parses Exact match entry to the struct
*
* @param[in]  devObjPtr                    - (pointer to) the device object
* @param[in]  exactMatchEntryPtr           - (pointer to) Exact match entry
* @param[in]  entryIndex                   -  entry index (hashed index)
* @param[out] exactMatchEntryInfoPtr       - (pointer to) Exact match entry parsed into fields
*
*/
GT_VOID snetAasExactMatchParseEntry
(
    IN    SKERNEL_DEVICE_OBJECT                *devObjPtr,
    IN    GT_U32                               *exactMatchEntryPtr,
    IN    GT_U32                                entryIndex,
    OUT   SNET_SIP7_EXACT_MATCH_ENTRY_INFO     *exactMatchEntryInfoPtr
)
{
    DECLARE_FUNC_NAME(snetAasExactMatchParseEntry);

#define SIP7_GET_VALUE(structFieldName, tableFieldName)\
    exactMatchEntryInfoPtr->structFieldName = SMEM_SIP7_EXACT_MATCH_ENTRY_FIELD_GET(devObjPtr,exactMatchEntryPtr, entryIndex, tableFieldName)

    SIP7_GET_VALUE(valid                  , SMEM_SIP7_EXACT_MATCH_TABLE_FIELDS_VALID                     );
    if(exactMatchEntryInfoPtr->valid == 0)
    {
        /* do not parse the rest of the entry ... not relevant ... save some time in Exact match scanning */
        return;
    }
    SIP7_GET_VALUE(entry_type             , SMEM_SIP7_EXACT_MATCH_TABLE_FIELDS_ENTRY_TYPE            );
    __LOG(("snetAasExactMatchParseEntry found valid entry type : %d \n", exactMatchEntryInfoPtr->entry_type));

    if(exactMatchEntryInfoPtr->entry_type == 0)  /*entry is key and action*/
    {
        SIP7_GET_VALUE(key_size           , SMEM_SIP7_EXACT_MATCH_TABLE_FIELDS_EM_ACTION_KEY_SIZE               );
        SIP7_GET_VALUE(age                , SMEM_SIP7_EXACT_MATCH_TABLE_FIELDS_EM_ACTION_AGE                    );
        SIP7_GET_VALUE(lookup_number      , SMEM_SIP7_EXACT_MATCH_TABLE_FIELDS_EM_ACTION_LOOKUP_NUMBER          );
        SIP7_GET_VALUE(key_31_0           , SMEM_SIP7_EXACT_MATCH_TABLE_FIELDS_EM_ACTION_KEY_31_0               );
        SIP7_GET_VALUE(key_39_32          , SMEM_SIP7_EXACT_MATCH_TABLE_FIELDS_EM_ACTION_KEY_39_32              );
        SIP7_GET_VALUE(action_31_0        , SMEM_SIP7_EXACT_MATCH_TABLE_FIELDS_EM_ACTION_ACTION_31_0            );
        SIP7_GET_VALUE(action_63_32       , SMEM_SIP7_EXACT_MATCH_TABLE_FIELDS_EM_ACTION_ACTION_63_32           );
        SIP7_GET_VALUE(action_83_64       , SMEM_SIP7_EXACT_MATCH_TABLE_FIELDS_EM_ACTION_ACTION_83_64           );
    }
    else /*entry is key only */
    {
        SIP7_GET_VALUE(keyOnly_31_0       , SMEM_SIP7_EXACT_MATCH_TABLE_FIELDS_EM_KEY_KEY_31_0           );
        SIP7_GET_VALUE(keyOnly_63_32      , SMEM_SIP7_EXACT_MATCH_TABLE_FIELDS_EM_KEY_KEY_63_32          );
        SIP7_GET_VALUE(keyOnly_95_64      , SMEM_SIP7_EXACT_MATCH_TABLE_FIELDS_EM_KEY_KEY_95_64          );
        SIP7_GET_VALUE(keyOnly_127_96     , SMEM_SIP7_EXACT_MATCH_TABLE_FIELDS_EM_KEY_KEY_127_96         );
    }
}

/**
* @internal snetAasExactMatchUnitNumGet function
* @endinternal
*
* @brief   Get the action entry from the Exact Match table.
*
* @param[in]  devObjPtr             - pointer to device object.
* @param[in]  emxClient             - EMX client
* @param[in]  lookupNum             - EM lookup number
* @param[out] emUnitNumPtr          - (pointer to) emUnit number
*/
GT_BOOL snetAasExactMatchUnitNumGet
(
    IN SKERNEL_DEVICE_OBJECT               *devObjPtr,
    IN SIP7_EMX_CLIENT_ENT                 emxClient,
    IN GT_U32                              lookupNum,
    OUT SIP7_EXACT_MATCH_UNIT_ENT           *emUnitNumPtr
)
{
    DECLARE_FUNC_NAME(snetAasExactMatchUnitNumGet);
    GT_U32 emChannel, regVal;
    GT_BOOL clientFound    = GT_FALSE;
    GT_CHAR* clientNamePtr = ((emxClient < SIP7_EMX_CLIENT_LAST_E) ? emxClientName[emxClient] : "unknown");

    __LOG(("Get the registered EM unit for EMX Client [%s] and lookupNum [%u]\n", clientNamePtr, lookupNum));

    /* Get EM channel from emxClient */
    for (emChannel = 0; emChannel < SIP7_EMX_MAX_CHANNEL_CNS; emChannel++)
    {
        smemRegGet(devObjPtr, SMEM_SIP7_EMX_CLIENT_CHANNEL_MAPPING_REG(devObjPtr, emChannel), &regVal);
        if (regVal == ((emxClient<<1)+lookupNum))
        {
            clientFound = GT_TRUE;
            break;
        }
    }

    if (clientFound == GT_FALSE)
    {
        __LOG(("EM unit not found for EMX Client [%s]\n", clientNamePtr));
        return GT_FALSE;
    }

    __LOG_PARAM(regVal);
    __LOG_PARAM(emChannel);
    /* Derive the EM unit number */
    *emUnitNumPtr = (emChannel/2);
    emChannel     = (emChannel%2);

    __LOG(("EMX Client [%s] is registered to EM unit [%u] with EM channel [%u]\n", clientNamePtr, *emUnitNumPtr, emChannel));

    return GT_TRUE;
}

/**
* @internal snetAasExactMatchActionGet function
* @endinternal
*
* @brief   Get the action entry from the Exact Match table.
*
* @param[in]  devObjPtr                       - pointer to device object.
* @param[in]  descrPtr                        - pointer to frame descriptor.
* @param[in]  matchIndex                      - index to the action table .
* @param[in]  emxClient                       - EMX client
* @param[in]  exactMatchProfileIdTableIndex   - index for Exact match Profile-ID mapping table
* @param[out] actionDataPtr                   - (pointer to) action data
*/
GT_VOID snetAasExactMatchActionGet
(
    IN SKERNEL_DEVICE_OBJECT               *devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC     *descrPtr  ,
    IN GT_U32                              matchIndex,
    IN SIP7_EMX_CLIENT_ENT                 emxClient,
    IN GT_U32                              lookupNum,
    IN GT_U32                              exactMatchProfileIdTableIndex,
    OUT GT_U32                             *actionDataPtr
)
{
    DECLARE_FUNC_NAME(snetAasExactMatchActionGet);
    /*for the reduced action needed 64 bit */
    GT_U32  reducedAction[3]={0} ;
    GT_U32  exactMatchProfileIdArr[SIP6_EXACT_MATCH_MAX_NUM_OF_HITS_CNS]={0} ;
    GT_U32  expandedActionIndex ;
    GT_U32  *entryPtr ;  /* MAC table entry pointer */
    GT_U32  profileId = 0;
    SNET_SIP7_EXACT_MATCH_ENTRY_INFO exactMatchEntryInfo ;
    SIP7_EXACT_MATCH_UNIT_ENT emUnitNum;
    GT_U32 retVal;
    GT_CHAR* clientNamePtr = ((emxClient < SIP7_EMX_CLIENT_LAST_E) ? emxClientName[emxClient] : "unknown");

    /* Get EM unit number from EMX Client */
    retVal = snetAasExactMatchUnitNumGet(devObjPtr, emxClient, lookupNum, &emUnitNum);
    if (retVal == GT_FALSE)
    {
        __LOG(("snetAasExactMatchActionGet : EM unit not found for EMX Client [%s]\n",
               clientNamePtr));
        return;
    }

    if ((matchIndex == SNET_SIP6_EXACT_MATCH_DEFAULT_INDEX_LOOKUP0_CNS) ||
        (matchIndex == SNET_SIP6_EXACT_MATCH_DEFAULT_INDEX_LOOKUP1_CNS))
    {
        snetAasExactMatchProfileIdGet(devObjPtr, descrPtr, emxClient,
                                      exactMatchProfileIdTableIndex, &exactMatchProfileIdArr[0]);
        profileId = (matchIndex == SNET_SIP6_EXACT_MATCH_DEFAULT_INDEX_LOOKUP0_CNS ) ? exactMatchProfileIdArr[0] : exactMatchProfileIdArr[1];

        if (profileId != 0)
        {
            __LOG(("snetAasExactMatchActionGet : action taken from exact match default action entry [%d] \n" ,
                   profileId));
            snetAasExactMatchProfileTableDefaultActionEntryGet(devObjPtr, emUnitNum, profileId, &actionDataPtr[0]);
        }
        else
        {
             __LOG(("profileId [%s] not legal - profile is disabled", profileId));
             simLogPacketFrameUnitSet(SIM_LOG_FRAME_UNIT___RESTORE_PREVIOUS_UNIT___E);
        }
    }
    else
    {
        __LOG(("snetAasExactMatchActionGet : action taken from exact match reduced action , matchIndex entry [%d] \n" ,
               matchIndex));
        if (emUnitNum == SIP7_EXACT_MATCH_UNIT_REDUCED_E)
        {
            entryPtr = smemMemGet(devObjPtr, SMEM_SIP7_REDUCED_EM_TBL_MEM(devObjPtr, matchIndex));
        }
        else
        {
            entryPtr = SMEM_SIP7_HIDDEN_EXACT_MATCH_PTR(devObjPtr, ((2*emUnitNum)+lookupNum), matchIndex);
        }
        snetAasExactMatchParseEntry(devObjPtr, entryPtr, matchIndex, &exactMatchEntryInfo);
        reducedAction[0]    = exactMatchEntryInfo.action_31_0;
        reducedAction[1]    = exactMatchEntryInfo.action_63_32;
        reducedAction[2]    = (exactMatchEntryInfo.action_83_64) & 0xFFFF; /* extract the 16 bits */
        expandedActionIndex = ((exactMatchEntryInfo.action_83_64)&0xF0000)>>16; /* extract last 4 bits*/

        __LOG(("ExpandedActionIndex[%u] \n", expandedActionIndex));

        snetAasExactMatchActionExpander(devObjPtr, emUnitNum, &reducedAction[0], expandedActionIndex, &actionDataPtr[0]);
    }
}

/**
* @internal snetAasExactMatchSerialEmActionGet function
* @endinternal
*
* @brief   Get the action entry from the Exact Match table.
*
* @param[in]  devObjPtr                       - pointer to device object.
* @param[in]  descrPtr                        - pointer to frame descriptor.
* @param[in]  matchIndex                      - index to the action table .
* @param[in]  emUnitNum                       - Exact Match unit number
* @param[in]  lookupNum                       - EM lookup number
* @param[in]  serialEmProfileId               - index for Exact match Serial EM Profile-ID mapping table
* @param[out] actionDataPtr                   - (pointer to) action data
*/
GT_VOID snetAasExactMatchSerialEmActionGet
(
    IN SKERNEL_DEVICE_OBJECT               *devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC     *descrPtr  ,
    IN GT_U32                              matchIndex,
    IN SIP7_EXACT_MATCH_UNIT_ENT           emUnitNum,
    IN GT_U32                              lookupNum,
    IN GT_U32                              serialEmProfileId,
    OUT GT_U32                             *actionDataPtr
)
{
    DECLARE_FUNC_NAME(snetAasExactMatchSerialEmActionGet);
    /*for the reduced action needed 64 bit */
    GT_U32  reducedAction[3]={0} ;
    GT_U32  expandedActionIndex ;
    GT_U32  *entryPtr ;  /* MAC table entry pointer */
    SNET_SIP7_EXACT_MATCH_ENTRY_INFO exactMatchEntryInfo ;

    if (matchIndex == SNET_SIP7_EXACT_MATCH_SERIAL_EM_DEFAULT_INDEX_CNS)
    {
        __LOG(("Action taken from exact match default action entry [0x%x] on EM unit [%u]\n" ,
               serialEmProfileId, emUnitNum));
        snetAasExactMatchProfileTableDefaultActionEntryGet(devObjPtr, emUnitNum, serialEmProfileId, &actionDataPtr[0]);
    }
    else
    {
        __LOG(("Action taken from exact match reduced action , matchIndex entry [0x%x] on EM unit [%u]\n" ,
               matchIndex, emUnitNum));
        if (emUnitNum == SIP7_EXACT_MATCH_UNIT_REDUCED_E)
        {
            entryPtr = smemMemGet(devObjPtr, SMEM_SIP7_REDUCED_EM_TBL_MEM(devObjPtr, matchIndex));
        }
        else
        {
            entryPtr = SMEM_SIP7_HIDDEN_EXACT_MATCH_PTR(devObjPtr, ((2*emUnitNum)+lookupNum), matchIndex);
        }
        snetAasExactMatchParseEntry(devObjPtr, entryPtr, matchIndex, &exactMatchEntryInfo);
        reducedAction[0]    = exactMatchEntryInfo.action_31_0;
        reducedAction[1]    = exactMatchEntryInfo.action_63_32;
        reducedAction[2]    = (exactMatchEntryInfo.action_83_64) & 0xFFFF; /* extract the 16 bits */
        expandedActionIndex = ((exactMatchEntryInfo.action_83_64)&0xF0000)>>16; /* extract last 4 bits*/

        __LOG(("ExpandedActionIndex[%u] \n", expandedActionIndex));

        snetAasExactMatchActionExpander(devObjPtr, emUnitNum, &reducedAction[0], expandedActionIndex, &actionDataPtr[0]);
    }
}

/**
* @internal snetAasExactMatchSerialEmKeyFieldBuildByPointer function
* @endinternal
*
* @brief   function inserts data of the field to the search key
*         in specified place in key
*         used for "numeric" fields - up to GT_U32
* @param[in,out] keyPtr              - pointer to Serial EM key
* @param[in]     endBit              - field MSB position
* @param[in]     startBit            - field LSB position
* @param[in]     fieldVal            - data of field to insert to key
*/
static GT_VOID snetAasExactMatchSerialEmKeyFieldBuildByU32Pointer
(
    INOUT GT_U32                   *keyPtr,
    IN    GT_U32                   startBit,
    IN    GT_U32                   endBit,
    IN    GT_U32                   fieldVal
)
{
    GT_U32  wordIndex;
    GT_U32  bitIndex;/*index of start bit in the start byte*/
    GT_U32  length;/* length of bits to set */
    GT_U32  numBitsInField;/* number of bits in the field */

    CHECK_FIELD_INTEGRITY(startBit,endBit);

    numBitsInField = endBit - startBit + 1;

    wordIndex = startBit / 32;
    bitIndex  = startBit % 32;

    while (numBitsInField)
    {
        length = (numBitsInField < (32 - bitIndex))
                 ? numBitsInField : (32 - bitIndex);

        SMEM_U32_SET_FIELD(keyPtr[wordIndex],/*data*/
                           bitIndex, /*offset*/
                           length, /*length*/
                           fieldVal);/*val*/

        wordIndex++;
        bitIndex = 0;
        numBitsInField -= length;
        fieldVal >>= length;
    }
}

/**
* @internal snetAasExactMatchSerialEmVlanTagKeyFieldBuild function
* @endinternal
*
* @brief   function insert data of field to the search key in specific place in VLAN Tag key
*
* @param[in,out] keyPtr                   - (pointer to) current serial EM key
* @param[in]     fieldVal                 - (pointer to) data of field to insert to key
* @param[in]     fieldId                  - field id
*/
static GT_VOID snetAasExactMatchSerialEmVlanTagKeyFieldBuild
(
    INOUT GT_U32                                                *keyPtr,
    IN    GT_U32                                                fieldVal,
    IN    SIP7_EXACT_MATCH_SERIAL_EM_VLAN_TAG_KEY_FIELDS_ID_ENT fieldId
)
{
    EXACT_MATCH_SERIAL_EM_KEY_FIELDS_INFO_STC *fieldInfoPtr = &serialEmVlanTagKeyFieldsData[fieldId];

    snetAasExactMatchSerialEmKeyFieldBuildByU32Pointer(keyPtr, fieldInfoPtr->startBitInKey,
                                                       fieldInfoPtr->endBitInKey, fieldVal);

    return;
}

/**
* @internal snetAasExactMatchSerialEmMplsKeyFieldBuild function
* @endinternal
*
* @brief   Insert data of field to the search key in specific place in MPLS key
*
* @param[in,out] keyPtr                   - (pointer to) current serial EM key
* @param[in]     fieldVal                 - data of field to insert to key
* @param[in]     fieldId                  - field id
*/
static GT_VOID snetAasExactMatchSerialEmMplsKeyFieldBuild
(
    INOUT GT_U32                                             *keyPtr,
    IN    GT_U32                                             fieldVal,
    IN    SIP7_EXACT_MATCH_SERIAL_EM_MPLS_KEY_FIELDS_ID_ENT  fieldId
)
{
    EXACT_MATCH_SERIAL_EM_KEY_FIELDS_INFO_STC *fieldInfoPtr = &serialEmMplsKeyFieldsData[fieldId];

    snetAasExactMatchSerialEmKeyFieldBuildByU32Pointer(keyPtr, fieldInfoPtr->startBitInKey,
                                                       fieldInfoPtr->endBitInKey, fieldVal);

    return;
}

/**
* @internal snetAasExactMatchSerialEmBuildKey function
* @endinternal
*
* @brief   For a given key, perform the lookup in Exact match table and
*          fill the results array
*
* @param[in]  devObjPtr                - pointer to device object.
* @param[in]  descrPtr                 - pointer to frame descriptor.
* @param[in]  emUnitNum                - Exact Match unit number
* @param[in]  serialEM                 - Serial EM lookup number
* @param[in]  serialEmProfileParamsPtr - Pointer to SIP7_SERIAL_EM_ENTRY_STC
* @param[out] keyPtr                   - Serial EM key pointer
*
*/
static GT_VOID snetAasExactMatchSerialEmBuildKey
(
    IN  SKERNEL_DEVICE_OBJECT             *devObjPtr,
    IN  SKERNEL_FRAME_CHEETAH_DESCR_STC   *descrPtr,
    IN  SIP7_EXACT_MATCH_UNIT_ENT         emUnitNum,
    IN  SIP7_SERIAL_EM_TYPE_ENT           serialEm,
    IN  SIP7_SERIAL_EM_ENTRY_STC          *serialEmProfileParamsPtr,
    OUT GT_U32                            *keyPtr
)
{
    DECLARE_FUNC_NAME(snetAasExactMatchSerialEmBuildKey);
    GT_U32  exactMatchProfileTableMaskDataArray[SIP7_EXACT_MATCH_MAX_KEY_SIZE_WORD_CNS];
    GT_U32  word;
    GT_U32  currFwdLabelIndex;
    GT_BOOL tag0IsRange, tag1IsRange;
    GT_U32  sourcePortProfile, udb28, udb29;

    __LOG(("Building Serial EM key for EM unit[%u]\n", emUnitNum));

    __LOG(("For Serial EM[%u], the profileId = [%u], keyType = [%u], enableUDB28VlanTag=[%u], enableUDB29VlanTag=[%u], pclId=[%u]\n",
           serialEm, serialEmProfileParamsPtr->serialEmProfileId, serialEmProfileParamsPtr->serialEmKeyType,
           serialEmProfileParamsPtr->serialEmEnableUDB28VlanTagKey, serialEmProfileParamsPtr->serialEmEnableUDB29VlanTagKey,
           serialEmProfileParamsPtr->serialEmPclId));

    switch (serialEmProfileParamsPtr->serialEmKeyType)
    {
        case SIP7_EXACT_MATCH_SERIAL_EM_KEY_TYPE_VLAN_TAG_E:
            sourcePortProfile = SMEM_LION3_TTI_DEFAULT_EPORT_ENTRY_FIELD_GET(devObjPtr, descrPtr,
                                SMEM_SIP7_TTI_DEFAULT_EPORT_TABLE_FIELDS_GENERIC_RANGE_SOURCE_PORT_PROFILE_E);
            /* TBD : update the fileds from Range Classification */
            tag0IsRange = 0;
            tag1IsRange = 0;
            /* TBD : update the UDB fileds */
            udb28       = 0;
            udb29       = 0;
            if (serialEmProfileParamsPtr->serialEmEnableUDB28VlanTagKey)
            {
                __LOG(("UDB28 Flag enabled: Setting UDB28[%u] in EM key\n", udb28));
                snetAasExactMatchSerialEmVlanTagKeyFieldBuild(keyPtr, udb28,
                                                              SIP7_EXACT_MATCH_SERIAL_EM_VLAN_TAG_KEY_FIELDS_ID_UDB28_E);
            }
            else
            {
                __LOG(("Setting VLAN TAG1 [%u] in EM key\n", descrPtr->vid1));
                snetAasExactMatchSerialEmVlanTagKeyFieldBuild(keyPtr, descrPtr->vid1,
                                                              SIP7_EXACT_MATCH_SERIAL_EM_VLAN_TAG_KEY_FIELDS_ID_TAG1_E);
            }
            __LOG(("Setting TAG1 Range flag [%u] in EM key\n", tag1IsRange));
            snetAasExactMatchSerialEmVlanTagKeyFieldBuild(keyPtr, tag1IsRange,
                                                          SIP7_EXACT_MATCH_SERIAL_EM_VLAN_TAG_KEY_FIELDS_ID_TAG1_IS_RANGE_E);
            __LOG(("Setting TAG1 Exist flag [%u] in EM key\n", TAG1_EXIST_MAC(descrPtr)));
            snetAasExactMatchSerialEmVlanTagKeyFieldBuild(keyPtr, TAG1_EXIST_MAC(descrPtr),
                                                          SIP7_EXACT_MATCH_SERIAL_EM_VLAN_TAG_KEY_FIELDS_ID_TAG1_FOUND_E);
            if (serialEmProfileParamsPtr->serialEmEnableUDB29VlanTagKey)
            {
                __LOG(("UDB29 Flag enabled: Setting UDB29[%u] in EM key\n", udb29));
                snetAasExactMatchSerialEmVlanTagKeyFieldBuild(keyPtr, udb29, SIP7_EXACT_MATCH_SERIAL_EM_VLAN_TAG_KEY_FIELDS_ID_UDB29_E);
            }
            else
            {
                __LOG(("Setting VLAN TAG0 [%u] in EM key\n", descrPtr->eVid));
                snetAasExactMatchSerialEmVlanTagKeyFieldBuild(keyPtr, descrPtr->eVid, SIP7_EXACT_MATCH_SERIAL_EM_VLAN_TAG_KEY_FIELDS_ID_TAG0_E);
            }
            __LOG(("Setting TAG0 Range flag [%u] in EM key\n", tag0IsRange));
            snetAasExactMatchSerialEmVlanTagKeyFieldBuild(keyPtr, tag0IsRange,
                                                          SIP7_EXACT_MATCH_SERIAL_EM_VLAN_TAG_KEY_FIELDS_ID_TAG0_IS_RANGE_E);
            __LOG(("Setting TAG0 Exist flag [%u] in EM key\n", TAG0_EXIST_MAC(descrPtr)));
            snetAasExactMatchSerialEmVlanTagKeyFieldBuild(keyPtr, TAG0_EXIST_MAC(descrPtr),
                                                          SIP7_EXACT_MATCH_SERIAL_EM_VLAN_TAG_KEY_FIELDS_ID_TAG0_FOUND_E);
            __LOG(("Setting SourcePortProfile [%u] in EM key\n", sourcePortProfile));
            snetAasExactMatchSerialEmVlanTagKeyFieldBuild(keyPtr, sourcePortProfile,
                                                          SIP7_EXACT_MATCH_SERIAL_EM_VLAN_TAG_KEY_FIELDS_ID_SRC_PORT_PROFILE_E);
            __LOG(("Setting PCL ID [%u] in EM key\n", serialEmProfileParamsPtr->serialEmPclId));
            snetAasExactMatchSerialEmVlanTagKeyFieldBuild(keyPtr, serialEmProfileParamsPtr->serialEmPclId,
                                                          SIP7_EXACT_MATCH_SERIAL_EM_VLAN_TAG_KEY_FIELDS_ID_PCL_ID_E);
            break;
        case SIP7_EXACT_MATCH_SERIAL_EM_KEY_TYPE_MPLS_E:
            /* Get the current forwarding Label Index */
            currFwdLabelIndex = descrPtr->currFwdLabelIndex;
            snetAasExactMatchSerialEmMplsKeyFieldBuild(keyPtr, descrPtr->genericClassification[0],
                                                       SIP7_EXACT_MATCH_SERIAL_EM_MPLS_KEY_FIELDS_ID_GENERIC_CLASSIFICATION_E);
            snetAasExactMatchSerialEmMplsKeyFieldBuild(keyPtr, descrPtr->isLabelValid[currFwdLabelIndex],
                                                       SIP7_EXACT_MATCH_SERIAL_EM_MPLS_KEY_FIELDS_ID_LABEL_VALID_E);
            snetAasExactMatchSerialEmMplsKeyFieldBuild(keyPtr, descrPtr->label[currFwdLabelIndex],
                                                       SIP7_EXACT_MATCH_SERIAL_EM_MPLS_KEY_FIELDS_ID_LABEL_E);
            snetAasExactMatchSerialEmMplsKeyFieldBuild(keyPtr, descrPtr->mac2me,
                                                       SIP7_EXACT_MATCH_SERIAL_EM_MPLS_KEY_FIELDS_ID_MAC2ME_E);
            snetAasExactMatchSerialEmMplsKeyFieldBuild(keyPtr, serialEmProfileParamsPtr->serialEmPclId,
                                                       SIP7_EXACT_MATCH_SERIAL_EM_MPLS_KEY_FIELDS_ID_PCL_ID_E);
            break;
        case SIP7_EXACT_MATCH_SERIAL_EM_KEY_TYPE_UDB_20_24_E:
            /* TBD*/
            break;
        case SIP7_EXACT_MATCH_SERIAL_EM_KEY_TYPE_UDB_25_29_E:
            /* TBD */
            break;
        default:
            __LOG(("Invalid KeyType"));
    }

    __LOG(("Before Masking : Serial EM[%u] KeyArray_0 [0x%X], keyArray_1 [0x%X]\n", serialEm, keyPtr[0], keyPtr[1]));

    /* Get the key mask from EM profile table */
    sip7ExactMatchProfileTableMaskDataGet(devObjPtr, emUnitNum,
                                          serialEmProfileParamsPtr->serialEmProfileId,
                                          &exactMatchProfileTableMaskDataArray[0]);

    /* Mask the Key */
    for (word=0; word < SIP7_EXACT_MATCH_SERIAL_EM_MAX_KEY_SIZE_WORD_CNS; word++)
    {
        __LOG(("Mask for word[%u] = [0x%X]\n", word, exactMatchProfileTableMaskDataArray[word]));
        keyPtr[word] &= exactMatchProfileTableMaskDataArray[word];
    }

    __LOG(("After Masking : Serial EM[%u] KeyArray_0 [0x%X], keyArray_1 [0x%X]\n", serialEm, keyPtr[0], keyPtr[1]));

    __LOG(("End of Building Serial EM key"));
}

/**
* @internal snetAasExactMatchSerialEmLookup function
* @endinternal
*
* @brief   For a given Profile ID, perform the Serial EM lookups and
*          update the Action pointer
*
* @param[in]  devObjPtr                - pointer to device object.
* @param[in]  descrPtr                 - pointer to frame descriptor.
* @param[in]  emxClient                - EMX Client
* @param[out] internalTtiInfoPtr       - (pointer to) internal TTI info
*/
GT_VOID snetAasExactMatchSerialEmLookup
(
    IN  SKERNEL_DEVICE_OBJECT             *devObjPtr,
    IN  SKERNEL_FRAME_CHEETAH_DESCR_STC   *descrPtr,
    IN  SIP7_EMX_CLIENT_ENT               emxClient,
    OUT INTERNAL_TTI_DESC_INFO_STC        *internalTtiInfoPtr
)
{
    DECLARE_FUNC_NAME(snetAasExactMatchSerialEmLookup);
    GT_U32                      serialEm;
    GT_U32                      lookupNum;
    SIP7_EXACT_MATCH_UNIT_ENT   emUnitNum;
    GT_U32                      exactMatchIndex;
    GT_U32                      serialEmKey[SIP7_EXACT_MATCH_MAX_KEY_SIZE_WORD_CNS] = {0};
    GT_U32                      retVal;
    GT_BOOL                     gotHit;
    GT_U32                      regAddr;
    GT_U32                      regVal;
    GT_U32                      exactMatchProfileTableEnableDefault;
    GT_U32                      serialEmPortProfileId;
    SIP7_SERIAL_EM_ENTRY_STC    serialEmProfileParams = {0};
    GT_CHAR* clientNamePtr = ((emxClient < SIP7_EMX_CLIENT_LAST_E) ? emxClientName[emxClient] : "unknown");

    __LOG(("Starting Serial EM lookups for EMX Client [%s]\n", clientNamePtr));

    /* Get Serial EM port profile ID from egress EPort */
    serialEmPortProfileId = SMEM_LION3_TTI_DEFAULT_EPORT_ENTRY_FIELD_GET(devObjPtr, descrPtr,
                               SMEM_SIP7_TTI_DEFAULT_EPORT_TABLE_FIELDS_SERIAL_EM_PORT_PROFILE_ID_E);
    __LOG_PARAM(serialEmPortProfileId);

    /* Serial EM Lookup1 & Lookup2 */
    for (serialEm = SIP7_SERIAL_EM0_E; serialEm < SIP7_EXACT_MATCH_SERIAL_EM_MAX_CNS; serialEm++)
    {
        /* TBD : Generic Range Check Lookup sections */

        /* Get the Serial EM Profile Params */
        snetAasSerialEmProfileParamsGet(devObjPtr, descrPtr, emxClient, serialEm,
                                        serialEmPortProfileId, &serialEmProfileParams);
        __LOG(("Serial EM Profile ID is [%u] for EM Lookup[%d]\n",
               serialEmProfileParams.serialEmProfileId, serialEm));

        /* If Serial EM Profile ID is 0 then skip that lookup */
        if (serialEmProfileParams.serialEmProfileId == 0)
        {
            __LOG(("Serial EM Lookup[%u] is disabled\n", serialEm));
            continue;
        }
        __LOG(("Started Serial EM Lookup[%d]\n", serialEm));

        if (serialEm == SIP7_SERIAL_EM0_E)
        {
            emUnitNum = SIP7_EXACT_MATCH_UNIT_REDUCED_E;
            lookupNum = 0;
        }
        else
        {
            lookupNum = serialEm - 1;
            /* Get EM unit number from EMX Client */
            retVal = snetAasExactMatchUnitNumGet(devObjPtr, emxClient, lookupNum, &emUnitNum);
            if (retVal == GT_FALSE)
            {
                __LOG(("EM unit not found for EMX Client[%u] with lookupNum[%u]\n", emxClient, lookupNum));
                return;
            }
        }

        /* Build the Serial EM key based on key type */
        snetAasExactMatchSerialEmBuildKey(devObjPtr, descrPtr, emUnitNum, serialEm,
                                          &serialEmProfileParams, &serialEmKey[0]);

        __LOG(("Serial EM Lookup[%u]: Perform the EM lookup on EM unit[%u] with Channel[%u]\n",
               serialEm, emUnitNum, lookupNum));
        /* Do the lookup */
        /* use the new key to get a match in the Exact match entry */
        gotHit = sip7ExactMatchIsHit(devObjPtr, emUnitNum,
                                     lookupNum, &serialEmKey[0],
                                     SIP7_EXACT_MATCH_KEY_SIZE_5B_E,
                                     &exactMatchIndex);

        if (gotHit)
        {
            __LOG(("Got hit: matchIndex [%d] \n", exactMatchIndex));
        }
        else
        {
            __LOG(("EM entry not found for Serial EM Lookup[%u] on EM unit[%u] with Channel[%u]\n",
                   serialEm, emUnitNum, lookupNum));
            /* Get the configured Default action enable flag */
            regAddr = SMEM_SIP7_EXACT_MATCH_PROFILE_CONFIG_REG(devObjPtr, emUnitNum, serialEmProfileParams.serialEmProfileId);
            smemRegFldGet(devObjPtr, regAddr, 0, 1, &regVal);
            exactMatchProfileTableEnableDefault = (regVal & 0x1);

            if (exactMatchProfileTableEnableDefault == GT_TRUE)
            {
                __LOG(("EM Default Action is Enabled for Serial EM Lookup[%u] on EM unit[%u] with Channel[%u]\n",
                        serialEm, emUnitNum, lookupNum));
                __LOG(("Setting default Index [0x%x]\n",
                        SNET_SIP7_EXACT_MATCH_SERIAL_EM_DEFAULT_INDEX_CNS));
                /* Index not found. Set Default index to choose default action */
                exactMatchIndex = SNET_SIP7_EXACT_MATCH_SERIAL_EM_DEFAULT_INDEX_CNS;
            }
            else
            {
                __LOG(("EM Default Action NOT Enabled for Serial EM Lookup[%u] on EM unit[%u] with Channel[%u]\n",
                       serialEm, emUnitNum, lookupNum));
                continue;
            }
        }

        __LOG(("Serial EM Lookup[%u]: Start processing the Expanded Action on EM unit[%u] with Channel[%u]\n",
               serialEm, emUnitNum, lookupNum));
        snetAasExactMatchSerialEmTTProcessAction(devObjPtr, descrPtr, emUnitNum,
                                                 lookupNum,
                                                 exactMatchIndex,
                                                 serialEmProfileParams.serialEmProfileId,
                                                 internalTtiInfoPtr);
    } /* Serial EM loop */
}
