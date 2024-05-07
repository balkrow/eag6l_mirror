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
* @file prvCpssDxChTxqPds.c
*
* @brief CPSS SIP6 TXQ PDS low level configurations.
*
* @version   1
********************************************************************************
*/
#define CPSS_LOG_IN_MODULE_ENABLE
#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/dxCh/dxChxGen/txq/private/regFiles/falcon/pipe/prvCpssFalconTxqPdsRegFile.h>
#include <cpss/dxCh/dxChxGen/txq/private/regFiles/ac5p/pipe/prvCpssAc5pTxqPdsRegFile.h>
#include <cpss/dxCh/dxChxGen/txq/private/regFiles/ac5x/pipe/prvCpssAc5xTxqPdsRegFile.h>
#include <cpss/dxCh/dxChxGen/txq/private/regFiles/harrier/pipe/prvCpssHarrierTxqPdsRegFile.h>
#include <cpss/dxCh/dxChxGen/txq/private/regFiles/ironman/pipe/prvCpssIronmanTxqPdsRegFile.h>
#include <cpss/dxCh/dxChxGen/txq/private/regFiles/seahawk/pipe/prvCpssAasTxqPdsRegFile.h>

#include <cpss/dxCh/dxChxGen/txq/private/prvCpssDxChTxqMain.h>
#include <cpss/dxCh/dxChxGen/txq/private/units/prvCpssDxChTxqPds.h>
#include <cpss/dxCh/dxChxGen/txq/private/units/prvCpssDxChTxqSdq.h>


#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwInit.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>

#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwRegAddrVer1_ppDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#define PRV_PDS_SIP_6_30_DEV_DEP_FIELD_GET_MAC(_dev,_fieldName) \
    (PRV_CPSS_SIP_7_CHECK_MAC(_dev)?TXQ_AAS_PDS_##_fieldName:TXQ_IRONMAN_PDS_##_fieldName)

#define PRV_PDS_SIP_6_20_DEV_DEP_FIELD_GET_MAC(_dev,_fieldName) \
    (PRV_CPSS_SIP_6_30_CHECK_MAC(_dev)?PRV_PDS_SIP_6_30_DEV_DEP_FIELD_GET_MAC(_dev,_fieldName):TXQ_HARRIER_PDS_##_fieldName)

#define PRV_PDS_SIP_6_15_DEV_DEP_FIELD_GET_MAC(_dev,_fieldName) \
    (PRV_CPSS_SIP_6_20_CHECK_MAC(_dev)? \
     (PRV_PDS_SIP_6_20_DEV_DEP_FIELD_GET_MAC(_dev,_fieldName)):TXQ_PHOENIX_PDS_##_fieldName)


#define PRV_PDS_SIP_6_10_DEV_DEP_FIELD_GET_MAC(_dev,_fieldName) \
    (PRV_CPSS_SIP_6_15_CHECK_MAC(_dev)?\
     (PRV_PDS_SIP_6_15_DEV_DEP_FIELD_GET_MAC(_dev,_fieldName)):TXQ_HAWK_PDS_##_fieldName)

#define TXQ_PDS_FIELD_GET(_dev,_fieldName) (PRV_CPSS_SIP_6_10_CHECK_MAC(_dev)?\
    (PRV_PDS_SIP_6_10_DEV_DEP_FIELD_GET_MAC(_dev,_fieldName)):\
    (TXQ_PDS_##_fieldName))

/*Macro for fileds  not present at  SIP 6_30*/

#define PRV_PDS_EXCLUDE_6_30_START_6_20_END_7_DEV_FIELD_GET_MAC(_dev,_fieldName) \
    (PRV_CPSS_SIP_7_CHECK_MAC(_dev)?TXQ_AAS_PDS_##_fieldName:TXQ_HARRIER_PDS_##_fieldName)

#define PRV_PDS_EXCLUDE_6_30_START_6_15_END_7_DEV_DEP_FIELD_GET_MAC(_dev,_fieldName) \
    (PRV_CPSS_SIP_6_20_CHECK_MAC(_dev)? \
     (PRV_PDS_EXCLUDE_6_30_START_6_20_END_7_DEV_FIELD_GET_MAC(_dev,_fieldName)):TXQ_PHOENIX_PDS_##_fieldName)


#define PRV_PDS_EXCLUDE_6_30_START_6_10_END_7_DEV_DEP_FIELD_GET_MAC(_dev,_fieldName) \
    (PRV_CPSS_SIP_6_15_CHECK_MAC(_dev)?\
     (PRV_PDS_EXCLUDE_6_30_START_6_15_END_7_DEV_DEP_FIELD_GET_MAC(_dev,_fieldName)):TXQ_HAWK_PDS_##_fieldName)

#define TXQ_PDS_EXCLUDE_6_30_FIELD_GET(_dev,_fieldName) (PRV_CPSS_SIP_6_10_CHECK_MAC(_dev)?\
    (PRV_PDS_EXCLUDE_6_30_START_6_10_END_7_DEV_DEP_FIELD_GET_MAC(_dev,_fieldName)):\
    (TXQ_PDS_##_fieldName))


/*Macro for fileds  not present from  SIP 6_30*/


#define TXQ_PDS_UNTIL_6_20_START_6_15_FIELD_GET(_dev,_fieldName) \
    (PRV_CPSS_SIP_6_20_CHECK_MAC(_dev)? \
        TXQ_HARRIER_PDS_##_fieldName:TXQ_PHOENIX_PDS_##_fieldName)


#define TXQ_PDS_UNTIL_6_20_START_6_10_FIELD_GET(_dev,_fieldName) \
    (PRV_CPSS_SIP_6_15_CHECK_MAC(_dev)?\
     (TXQ_PDS_UNTIL_6_20_START_6_15_FIELD_GET(_dev,_fieldName)):TXQ_HAWK_PDS_##_fieldName)

#define TXQ_PDS_UNTIL_6_20_FIELD_GET(_dev,_fieldName) (PRV_CPSS_SIP_6_10_CHECK_MAC(_dev)?\
    (TXQ_PDS_UNTIL_6_20_START_6_10_FIELD_GET(_dev,_fieldName)):\
    (TXQ_PDS_##_fieldName))

/*Macro for fileds  not present from  SIP 7*/

#define TXQ_PDS_UNTIL_6_30_START_6_20_FIELD_GET(_dev,_fieldName) \
    (PRV_CPSS_SIP_6_20_CHECK_MAC(_dev)? \
        TXQ_HARRIER_PDS_##_fieldName:TXQ_IRONMAN_PDS_##_fieldName)


#define TXQ_PDS_UNTIL_6_30_START_6_15_FIELD_GET(_dev,_fieldName) \
    (PRV_CPSS_SIP_6_20_CHECK_MAC(_dev)? \
        TXQ_PDS_UNTIL_6_30_START_6_20_FIELD_GET(_dev,_fieldName):TXQ_PHOENIX_PDS_##_fieldName)


#define TXQ_PDS_UNTIL_6_30_START_6_10_FIELD_GET(_dev,_fieldName) \
    (PRV_CPSS_SIP_6_15_CHECK_MAC(_dev)?\
     (TXQ_PDS_UNTIL_6_30_START_6_15_FIELD_GET(_dev,_fieldName)):TXQ_HAWK_PDS_##_fieldName)

#define TXQ_PDS_UNTIL_6_30_FIELD_GET(_dev,_fieldName) (PRV_CPSS_SIP_6_10_CHECK_MAC(_dev)?\
    (TXQ_PDS_UNTIL_6_30_START_6_10_FIELD_GET(_dev,_fieldName)):\
    (TXQ_PDS_##_fieldName))



#define PRV_TXQ_UNIT_NAME "PDS"
#define PRV_TXQ_LOG_UNIT GT_FALSE
#define PRV_TXQ_LOG_TABLE GT_FALSE


#define PRV_TXQ_PDS_MAIL_BOX_MAX_NUM_CNS 32

#define TXQ_SIP_6_CHECK_MAIL_BOX_ID_MAC(_mailBoxId)\
    do\
      {\
        if(_mailBoxId>=PRV_TXQ_PDS_MAIL_BOX_MAX_NUM_CNS)\
        {\
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "mail box id  too big - [%d] must be less than [%d]",_mailBoxId,PRV_TXQ_PDS_MAIL_BOX_MAX_NUM_CNS);\
        }\
      }while(0)



GT_STATUS prvCpssSip6TxqPdsLogInit
(
   INOUT PRV_CPSS_DXCH_TXQ_SIP_6_REG_LOG_STC * logDataPtr,
   IN GT_U32                                   pdsNum,
   IN GT_CHAR_PTR                              regName
)
{
    CPSS_NULL_PTR_CHECK_MAC(logDataPtr);

    logDataPtr->log = PRV_TXQ_LOG_UNIT;
    logDataPtr->unitName = PRV_TXQ_UNIT_NAME;
    logDataPtr->unitIndex = pdsNum;
    logDataPtr->regName = regName;
    logDataPtr->regIndex = PRV_TXQ_LOG_NO_INDEX;

    return GT_OK;
}

static GT_STATUS prvCpssSip6TxqPdsTableLogInit
(
   INOUT PRV_CPSS_DXCH_TXQ_SIP_6_TABLE_LOG_STC * logDataPtr,
   IN GT_U32                                   tileNum,
   IN GT_U32                                   pdsNum,
   IN CPSS_DXCH_TABLE_ENT                      tableIdentity,
   IN GT_CHAR_PTR                              tableName,
   IN GT_U32                                   entryIndex
)
{
    CPSS_NULL_PTR_CHECK_MAC(logDataPtr);

    GT_UNUSED_PARAM(tableIdentity);

    logDataPtr->log = PRV_TXQ_LOG_TABLE;
    logDataPtr->unitName = PRV_TXQ_UNIT_NAME;
    logDataPtr->tileIndex = tileNum;
    logDataPtr->unitIndex = pdsNum;
    logDataPtr->tableName = tableName;
    logDataPtr->entryIndex = entryIndex;

    return GT_OK;
}


/**
* @internal prvCpssDxChTxqFalconPdsMaxPdsSizeLimitSet function
* @endinternal
*
* @brief  Set limit on PDS capacity .When this limit is reached, stop accepting descriptors into the PDS.
*         The motivation is to reduce risk of PDS getting FULL
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] tileNum               - The number of tile (Applicable range 0..3)
* @param[in] pdsNum                - the local number of PDS (APPLICABLE RANGES:0..7).
* @param[in] pdsSizeForPdx         - limit on PDS capacity for PDX
* @param[in] pdsSizeForPb          - limit on PDS capacity for PB
* @param[in] pbFullLimit           - limit on PB for PDS(used for long queues mechanism)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChTxqFalconPdsMaxPdsSizeLimitSet
(
    IN GT_U8 devNum,
    IN GT_U32 tileNum,
    IN GT_U32 pdsNum,
    IN GT_U32 pdsSizeForPdx,
    IN GT_U32 pdsSizeForPb,
    IN GT_U32 pbFullLimit
)
{
    GT_U32 regAddr;
    GT_U32 regValue = 0;
    GT_STATUS rc;
    PRV_CPSS_DXCH_TXQ_SIP_6_REG_LOG_STC log;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_PDS_NUM_MAC(pdsNum);

    if(pdsSizeForPdx>= (GT_U32)(1<<TXQ_PDS_FIELD_GET(devNum,MAX_PDS_SIZE_LIMIT_FOR_PDX_MAX_PDS_SIZE_LIMIT_FOR_PDX_FIELD_SIZE)))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssSip6TxqPdsLogInit(&log,pdsNum,"Max_PDS_size_limit_for_pdx");
    if (rc != GT_OK)
    {
      return rc;
    }


    regAddr = PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pds[pdsNum].Max_PDS_size_limit_for_pdx;

    U32_SET_FIELD_MASKED_MAC(regValue,TXQ_PDS_FIELD_GET(devNum,MAX_PDS_SIZE_LIMIT_FOR_PDX_MAX_PDS_SIZE_LIMIT_FOR_PDX_FIELD_OFFSET),
        TXQ_PDS_FIELD_GET(devNum,MAX_PDS_SIZE_LIMIT_FOR_PDX_MAX_PDS_SIZE_LIMIT_FOR_PDX_FIELD_SIZE),
        pdsSizeForPdx);

    rc = prvCpssSip6TxqRegisterFieldWrite(devNum,tileNum,&log,regAddr,0,32,regValue);
    if(rc!=GT_OK)
    {
        return rc;
    }


    if(PRV_CPSS_SIP_6_30_CHECK_MAC(devNum)==GT_FALSE)
    {
        if(pdsSizeForPdx>= (GT_U32)(1<<TXQ_PDS_EXCLUDE_6_30_FIELD_GET(devNum,MAX_PDS_SIZE_LIMIT_FOR_PB_MAX_PDS_SIZE_LIMIT_FOR_PB_FIELD_SIZE)))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        if(pbFullLimit>= (GT_U32)(1<<TXQ_PDS_EXCLUDE_6_30_FIELD_GET(devNum,PB_FULL_LIMIT_PB_FULL_LIMIT_FIELD_SIZE)))
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }



        regAddr = PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pds[pdsNum].Max_PDS_size_limit_for_PB;
        log.regName = "Max_PDS_size_limit_for_PB";

        regValue =0;

        U32_SET_FIELD_MASKED_MAC(regValue,TXQ_PDS_EXCLUDE_6_30_FIELD_GET(devNum,MAX_PDS_SIZE_LIMIT_FOR_PB_MAX_PDS_SIZE_LIMIT_FOR_PB_FIELD_OFFSET),
            TXQ_PDS_EXCLUDE_6_30_FIELD_GET(devNum,MAX_PDS_SIZE_LIMIT_FOR_PB_MAX_PDS_SIZE_LIMIT_FOR_PB_FIELD_SIZE),
            pdsSizeForPb);

        rc = prvCpssSip6TxqRegisterFieldWrite(devNum,tileNum,&log,regAddr,0,32,regValue);
        if(rc!=GT_OK)
        {
            return rc;
        }

        regAddr = PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pds[pdsNum].PB_Full_Limit;
        log.regName = "PB_Full_Limit";


        regValue =0;

        U32_SET_FIELD_MASKED_MAC(regValue,TXQ_PDS_EXCLUDE_6_30_FIELD_GET(devNum,PB_FULL_LIMIT_PB_FULL_LIMIT_FIELD_OFFSET),
            TXQ_PDS_EXCLUDE_6_30_FIELD_GET(devNum,PB_FULL_LIMIT_PB_FULL_LIMIT_FIELD_SIZE),
            pbFullLimit);

        rc = prvCpssSip6TxqRegisterFieldWrite(devNum,tileNum,&log,regAddr,0,32,regValue);

    }


    return rc;
}

/**
* @internal prvCpssDxChTxqFalconPdsProfileSet function
* @endinternal
*
* @brief   Profile :
*         Profile_%p_Long_Queue_Enable - Enables writing a queue to the PB when it is getting long
*         Profile_%p_Long_Queue_Limit [0..8K] - When the total queue counter reaches this limit, a long queue is opened
*         Profile_%p_Head_Empty_Limit [0..8K] - When the head counter is below this limit,
*         the head is considered empty and fragment read from PB
*         is triggered
*         Length_Adjust_Constant_byte_count_Profile_%p_ [0..64]
*         - Sets the value which will be decremented or
*         incremented from the packet's byte count
*         Length_Adjust_Sub_Profile_%p_ - Subtract enable bit -
*         for profile <%p> 0x0 = ADD; ADD; When ADD, the value
*         of constant byte count field is added to the
*         descriptor byte count 0x1 = SUB; SUB; When SUB, the
*         value of constant byte count field is subtracted from
*         the descriptor byte count
*         Length_Adjust_Enable_profile_%p - RW 0x0 Enable the
*         length adjust 0x0 = Disable Length Adjust;
*         Disable_Length_Adjust 0x1 = Enable Length Adjust;
*         Enable_Length_Adjust
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] tileNum               - The number of tile (Applicable range 0..3)
* @param[in] pdsNum                - the local number of PDS (APPLICABLE RANGES:0..7).
* @param[in] profileIndex          - index of the profile(APPLICABLE RANGES:0..15.)
* @param[in] profilePtr            - (pointer to)PDS  profile parameters
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChTxqFalconPdsProfileSet
(
    IN GT_U8    devNum,
    IN GT_U32   tileNum,
    IN GT_U32   pdsNum,
    IN GT_U32   profileIndex,
    IN PRV_CPSS_DXCH_SIP6_TXQ_PDS_QUEUE_PROFILE_STC *profilePtr
)
{
    GT_STATUS rc = GT_OK;
    GT_U32 regValue = 0;
    GT_U32 maxProfilesNum;
    PRV_CPSS_DXCH_TXQ_SIP_6_REG_LOG_STC log;
    GT_BOOL     queueProfilesPresent = PRV_CPSS_TXQ_PER_DEVICE_DB_MAC(devNum).pdsLongQueueProfilesSupported;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_PDS_NUM_MAC(pdsNum);


    rc = prvCpssSip6TxqPdsLogInit(&log,pdsNum,"Profile_Head_Empty_Limit");
    if (rc != GT_OK)
    {
      return rc;
    }

    log.regIndex = profileIndex;

    maxProfilesNum = PDS_PROFILE_MAX_MAC(devNum);

    if(profileIndex>=maxProfilesNum)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Profile index %d is bigger then max allowed %d\n",profileIndex,maxProfilesNum-1);
    }


    if(queueProfilesPresent==GT_TRUE)
    {
        if(profilePtr->longQueueParametersPtr)
        {
            if(profilePtr->longQueueParametersPtr->headEmptyLimit>=(GT_U32)(1<<TXQ_PDS_EXCLUDE_6_30_FIELD_GET(devNum,PROFILE_HEAD_EMPTY_LIMIT_PROFILE_HEAD_EMPTY_LIMIT_FIELD_SIZE)))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }

            if(profilePtr->longQueueParametersPtr->longQueueLimit>=(GT_U32)(1<<TXQ_PDS_EXCLUDE_6_30_FIELD_GET(devNum,PROFILE_LONG_QUEUE_LIMIT_PROFILE_LONG_QUEUE_LIMIT_FIELD_SIZE)))
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
            }

            U32_SET_FIELD_MASKED_MAC(regValue,TXQ_PDS_EXCLUDE_6_30_FIELD_GET(devNum,PROFILE_HEAD_EMPTY_LIMIT_PROFILE_HEAD_EMPTY_LIMIT_FIELD_OFFSET),
                TXQ_PDS_EXCLUDE_6_30_FIELD_GET(devNum,PROFILE_HEAD_EMPTY_LIMIT_PROFILE_HEAD_EMPTY_LIMIT_FIELD_SIZE),
                profilePtr->longQueueParametersPtr->headEmptyLimit);

           rc =prvCpssSip6TxqRegisterFieldWrite(devNum,tileNum,&log,
            PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pds[pdsNum].Profile_Head_Empty_Limit[profileIndex],
            0,32,regValue);

            if(rc!=GT_OK)
            {
                return rc;
            }

            regValue = 0;

            U32_SET_FIELD_MASKED_MAC(regValue,TXQ_PDS_EXCLUDE_6_30_FIELD_GET(devNum,PROFILE_LONG_QUEUE_LIMIT_PROFILE_LONG_QUEUE_LIMIT_FIELD_OFFSET),
                TXQ_PDS_EXCLUDE_6_30_FIELD_GET(devNum,PROFILE_LONG_QUEUE_LIMIT_PROFILE_LONG_QUEUE_LIMIT_FIELD_SIZE),
                profilePtr->longQueueParametersPtr->longQueueLimit);

            log.regName="Profile_Long_Queue_Limit";

            rc =prvCpssSip6TxqRegisterFieldWrite(devNum,tileNum,&log,
             PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pds[pdsNum].Profile_Long_Queue_Limit[profileIndex],
             0,32,regValue);

            if(rc!=GT_OK)
            {
                return rc;
            }

            regValue = 0;



            U32_SET_FIELD_MASKED_MAC(regValue,TXQ_PDS_EXCLUDE_6_30_FIELD_GET(devNum,PROFILE_LONG_QUEUE_ENABLE_PROFILE_LONG_QUEUE_ENABLE_FIELD_OFFSET),
                TXQ_PDS_EXCLUDE_6_30_FIELD_GET(devNum,PROFILE_LONG_QUEUE_ENABLE_PROFILE_LONG_QUEUE_ENABLE_FIELD_SIZE),
                profilePtr->longQueueParametersPtr->longQueueEnable?1:0);

            log.regName="Profile_Long_Queue_Enable";

            rc =prvCpssSip6TxqRegisterFieldWrite(devNum,tileNum,&log,
               PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pds[pdsNum].Profile_Long_Queue_Enable[profileIndex],
               0,32,regValue);

            if(rc!=GT_OK)
            {
                return rc;
            }
        }
    }

    if(profilePtr->lengthAdjustParametersPtr)
    {
        if(profilePtr->lengthAdjustParametersPtr->lengthAdjustByteCount>=(GT_U32)(1<<TXQ_PDS_FIELD_GET(devNum,PROFILE_DEQUEUE_LENGTH_ADJUST_LENGTH_ADJUST_CONSTANT_BYTE_COUNT_PROFILE_FIELD_SIZE)))
        {
                 CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
        }

        regValue = 0;

        U32_SET_FIELD_MASKED_MAC(regValue,TXQ_PDS_FIELD_GET(devNum,PROFILE_DEQUEUE_LENGTH_ADJUST_LENGTH_ADJUST_ENABLE_PROFILE_FIELD_OFFSET),
            TXQ_PDS_FIELD_GET(devNum,PROFILE_DEQUEUE_LENGTH_ADJUST_LENGTH_ADJUST_ENABLE_PROFILE_FIELD_SIZE),
            profilePtr->lengthAdjustParametersPtr->lengthAdjustEnable);



        U32_SET_FIELD_MASKED_MAC(regValue,TXQ_PDS_FIELD_GET(devNum,PROFILE_DEQUEUE_LENGTH_ADJUST_LENGTH_ADJUST_SUB_PROFILE_FIELD_OFFSET),
            TXQ_PDS_FIELD_GET(devNum,PROFILE_DEQUEUE_LENGTH_ADJUST_LENGTH_ADJUST_SUB_PROFILE_FIELD_SIZE),
            profilePtr->lengthAdjustParametersPtr->lengthAdjustSubstruct);


        U32_SET_FIELD_MASKED_MAC(regValue,TXQ_PDS_FIELD_GET(devNum,PROFILE_DEQUEUE_LENGTH_ADJUST_LENGTH_ADJUST_CONSTANT_BYTE_COUNT_PROFILE_FIELD_OFFSET),
            TXQ_PDS_FIELD_GET(devNum,PROFILE_DEQUEUE_LENGTH_ADJUST_LENGTH_ADJUST_CONSTANT_BYTE_COUNT_PROFILE_FIELD_SIZE),
            profilePtr->lengthAdjustParametersPtr->lengthAdjustByteCount);

       log.regName="Profile_Dequeue_Length_Adjust";

       rc =prvCpssSip6TxqRegisterFieldWrite(devNum,tileNum,&log,
           PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pds[pdsNum].Profile_Dequeue_Length_Adjust[profileIndex],
           0,32,regValue);
    }

    return rc;


}

/**
* @internal prvCpssDxChTxqFalconPdsProfileGet function
* @endinternal
*
* @brief   Profile :
*         Profile_%p_Long_Queue_Enable - Enables writing a queue to the PB when it is getting long
*         Profile_%p_Long_Queue_Limit [0..8K] - When the total queue counter reaches this limit, a long queue is opened
*         Profile_%p_Head_Empty_Limit [0..8K] - When the head counter is below this limit,
*         the head is considered empty and fragment read from PB
*         is triggered
*         Length_Adjust_Constant_byte_count_Profile_%p_ [0..64]
*         - Sets the value which will be decremented or
*         incremented from the packet's byte count
*         Length_Adjust_Sub_Profile_%p_ - Subtract enable bit -
*         for profile <%p> 0x0 = ADD; ADD; When ADD, the value
*         of constant byte count field is added to the
*         descriptor byte count 0x1 = SUB; SUB; When SUB, the
*         value of constant byte count field is subtracted from
*         the descriptor byte count
*         Length_Adjust_Enable_profile_%p - RW 0x0 Enable the
*         length adjust 0x0 = Disable Length Adjust;
*         Disable_Length_Adjust 0x1 = Enable Length Adjust;
*         Enable_Length_Adjust
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] tileNum               - The number of tile (Applicable range 0..3)
* @param[in] pdsNum                - the local number of PDS (APPLICABLE RANGES:0..7).
* @param[in] profileIndex          - index of the profile(APPLICABLE RANGES:0..15.)
* @param[in] lengthAdjustOnly      - GT_TRUE in order to get only length adjust attributes,GT_FALSE otherwise
* @param[out] profilePtr           - (pointer to)pds profile
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChTxqFalconPdsProfileGet
(
    IN GT_U8 devNum,
    IN GT_U32 tileNum,
    IN GT_U32 pdsNum,
    IN GT_U32 profileIndex,
    IN GT_BOOL   lengthAdjustOnly,
    OUT  PRV_CPSS_DXCH_SIP6_TXQ_PDS_QUEUE_PROFILE_STC * profilePtr
)
{
    GT_STATUS rc;
    GT_U32 regValue = 0;
    GT_BOOL queueProfilesPresent = PRV_CPSS_TXQ_PER_DEVICE_DB_MAC(devNum).pdsLongQueueProfilesSupported;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_PDS_NUM_MAC(pdsNum);
    CPSS_NULL_PTR_CHECK_MAC(profilePtr);

    if(profileIndex>=PDS_PROFILE_MAX_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }


    if(GT_TRUE==queueProfilesPresent)
    {
        if(lengthAdjustOnly==GT_FALSE)
        {
           CPSS_NULL_PTR_CHECK_MAC(profilePtr->longQueueParametersPtr);

           rc =prvCpssSip6TxqRegisterFieldRead(devNum,tileNum,
                PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pds[pdsNum].Profile_Head_Empty_Limit[profileIndex],
                TXQ_PDS_EXCLUDE_6_30_FIELD_GET(devNum,PROFILE_HEAD_EMPTY_LIMIT_PROFILE_HEAD_EMPTY_LIMIT_FIELD_OFFSET),
                 TXQ_PDS_EXCLUDE_6_30_FIELD_GET(devNum,PROFILE_HEAD_EMPTY_LIMIT_PROFILE_HEAD_EMPTY_LIMIT_FIELD_SIZE),
                 &(profilePtr->longQueueParametersPtr->headEmptyLimit));

            if(rc!=GT_OK)
            {
                return rc;
            }

           rc =prvCpssSip6TxqRegisterFieldRead(devNum,tileNum,
                PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pds[pdsNum].Profile_Long_Queue_Limit[profileIndex],
                TXQ_PDS_EXCLUDE_6_30_FIELD_GET(devNum,PROFILE_LONG_QUEUE_LIMIT_PROFILE_LONG_QUEUE_LIMIT_FIELD_OFFSET),
                TXQ_PDS_EXCLUDE_6_30_FIELD_GET(devNum,PROFILE_LONG_QUEUE_LIMIT_PROFILE_LONG_QUEUE_LIMIT_FIELD_SIZE),
                &(profilePtr->longQueueParametersPtr->longQueueLimit));

            if(rc!=GT_OK)
            {
                return rc;
            }

            rc =prvCpssSip6TxqRegisterFieldRead(devNum,tileNum,
                 PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pds[pdsNum].Profile_Long_Queue_Enable[profileIndex],
                 TXQ_PDS_EXCLUDE_6_30_FIELD_GET(devNum,PROFILE_LONG_QUEUE_ENABLE_PROFILE_LONG_QUEUE_ENABLE_FIELD_OFFSET),
                 TXQ_PDS_EXCLUDE_6_30_FIELD_GET(devNum,PROFILE_LONG_QUEUE_ENABLE_PROFILE_LONG_QUEUE_ENABLE_FIELD_SIZE),
                 &regValue);

            if(rc!=GT_OK)
            {
                return rc;
            }

            profilePtr->longQueueParametersPtr->longQueueEnable = (regValue==1)?GT_TRUE:GT_FALSE;

        }
    }

    CPSS_NULL_PTR_CHECK_MAC(profilePtr->lengthAdjustParametersPtr);

    rc =prvCpssSip6TxqRegisterFieldRead(devNum,tileNum,
           PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pds[pdsNum].Profile_Dequeue_Length_Adjust[profileIndex],
           0,32,&regValue);

    if(rc == GT_OK)
    {
        profilePtr->lengthAdjustParametersPtr->lengthAdjustEnable = (GT_BOOL)U32_GET_FIELD_MAC(regValue,
         TXQ_PDS_FIELD_GET(devNum,PROFILE_DEQUEUE_LENGTH_ADJUST_LENGTH_ADJUST_ENABLE_PROFILE_FIELD_OFFSET),
         TXQ_PDS_FIELD_GET(devNum,PROFILE_DEQUEUE_LENGTH_ADJUST_LENGTH_ADJUST_ENABLE_PROFILE_FIELD_SIZE));

        profilePtr->lengthAdjustParametersPtr->lengthAdjustSubstruct= (GT_BOOL)U32_GET_FIELD_MAC(regValue,
         TXQ_PDS_FIELD_GET(devNum,PROFILE_DEQUEUE_LENGTH_ADJUST_LENGTH_ADJUST_SUB_PROFILE_FIELD_OFFSET),
         TXQ_PDS_FIELD_GET(devNum,PROFILE_DEQUEUE_LENGTH_ADJUST_LENGTH_ADJUST_SUB_PROFILE_FIELD_SIZE));

        profilePtr->lengthAdjustParametersPtr->lengthAdjustByteCount= (GT_BOOL)U32_GET_FIELD_MAC(regValue,
         TXQ_PDS_FIELD_GET(devNum,PROFILE_DEQUEUE_LENGTH_ADJUST_LENGTH_ADJUST_CONSTANT_BYTE_COUNT_PROFILE_FIELD_OFFSET),
         TXQ_PDS_FIELD_GET(devNum,PROFILE_DEQUEUE_LENGTH_ADJUST_LENGTH_ADJUST_CONSTANT_BYTE_COUNT_PROFILE_FIELD_SIZE));
    }

    return rc;
}

/**
* @internal prvCpssDxChTxqFalconPdsQueueProfileMapSet function
* @endinternal
*
* @brief   Set queue profile to PDS queue.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] tileNum               - The number of tile (Applicable range 0..3)
* @param[in] pdsNum                - the local number of PDS (APPLICABLE RANGES:0..7).
* @param[in] queueNumber           - the number of the queue(APPLICABLE RANGES:0..399)
* @param[in] profileIndex          - index of the profile(APPLICABLE RANGES:0..15.)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChTxqFalconPdsQueueProfileMapSet
(
    IN GT_U8 devNum,
    IN GT_U32 tileNum,
    IN GT_U32 pdsNum,
    IN GT_U32 queueNumber,
    IN GT_U32 profileIndex
)
{
    GT_U32 value[2] = {0,0};
    GT_STATUS rc;
    PRV_CPSS_DXCH_TXQ_SIP_6_REG_LOG_STC log;
    CPSS_DXCH_TABLE_ENT table;
    PRV_CPSS_DXCH_TXQ_SIP_6_TABLE_LOG_STC tableLog;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_PDS_NUM_MAC(pdsNum);

    /*not supported*/
    if(PRV_CPSS_SIP_6_30_CHECK_MAC(devNum)==GT_TRUE)
    {
        if(PRV_CPSS_SIP_7_CHECK_MAC(devNum)==GT_FALSE)
        {
            return GT_OK;
        }
    }

    if(profileIndex>=PDS_PROFILE_MAX_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(queueNumber>=CPSS_DXCH_SIP_6_MAX_DATA_PATH_QUEUE_NUM_MAC)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(PRV_CPSS_SIP_7_CHECK_MAC(devNum)==GT_FALSE)
    {

        rc = prvCpssSip6TxqPdsLogInit(&log,pdsNum,"Queue_PDS_Profile");
        if (rc != GT_OK)
        {
          return rc;
        }

        log.regIndex = queueNumber;

        U32_SET_FIELD_MASKED_MAC(value[0],TXQ_PDS_UNTIL_6_20_FIELD_GET(devNum,QUEUE_PDS_PROFILE_QUEUE_PROFILE_3_0_FIELD_OFFSET),
            TXQ_PDS_UNTIL_6_20_FIELD_GET(devNum,QUEUE_PDS_PROFILE_QUEUE_PROFILE_3_0_FIELD_SIZE),
            profileIndex);


        rc =prvCpssSip6TxqRegisterFieldWrite(devNum,tileNum,&log,
            PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pds[pdsNum].Queue_PDS_Profile[queueNumber],
            0,32,value[0]);
     }
     else
     {
        if (GT_TRUE == PRV_CPSS_SIP_7_CHECK_MAC(devNum) && GT_TRUE == cpssDeviceRunCheck_onEmulator())
        {
            return GT_OK;
        }
        table= CPSS_DXCH_SIP7_TXQ_PDS_QUEUE_TO_PROFILE_MAP_E + pdsNum + tileNum * CPSS_DXCH_SIP_7_MAX_LOCAL_PDS_NUM_MAC;

        rc = prvCpssDxChReadTableEntry(devNum,table,
                                        queueNumber>>3/*each entry 8 queues*/,
                                        value);
        if(rc!=GT_OK)
        {
           return rc;
        }

        /*low 4 queues*/
        if((queueNumber&0x7)<4)
        {
               U32_SET_FIELD_MASKED_MAC(value[0],(queueNumber&0x7)*TXQ_AAS_PDS_QUEUE_TO_PROFILE_MAP_QUEUE_PROFILE_FIELD_SIZE,
                                 TXQ_AAS_PDS_QUEUE_TO_PROFILE_MAP_QUEUE_PROFILE_FIELD_SIZE,
                                 profileIndex);
        }
        else
        {
               U32_SET_FIELD_MASKED_MAC(value[1],((queueNumber&0x7)-4)*TXQ_AAS_PDS_QUEUE_TO_PROFILE_MAP_QUEUE_PROFILE_FIELD_SIZE,
                                 TXQ_AAS_PDS_QUEUE_TO_PROFILE_MAP_QUEUE_PROFILE_FIELD_SIZE,
                                 profileIndex);
        }

        rc = prvCpssSip6TxqPdsTableLogInit(&tableLog,tileNum,pdsNum,table,"TBL_Queue_To_Profile_Map",queueNumber);
        if (rc != GT_OK)
        {
            return rc;
        }

        rc = prvCpssSip6TxqWriteTableEntry(devNum,&tableLog,
                                        table,
                                        queueNumber>>3/*each entry 8 queues*/,
                                        value);
     }

    return rc;
}

/**
* @internal prvCpssDxChTxqFalconPdsQueueProfileMapGet function
* @endinternal
*
* @brief   Get queue profile that binded to  PDS queue.
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] tileNum               - The number of tile (Applicable range 0..3)
* @param[in] pdsNum                - the local number of PDS (APPLICABLE RANGES:0..7).
* @param[in] queueNumber           - the number of the queue(APPLICABLE RANGES:0..399)
* @param[in] profileIndexPtr       - (pointer to) index of the profile(APPLICABLE RANGES:0..15.)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChTxqFalconPdsQueueProfileMapGet
(
    IN GT_U8 devNum,
    IN GT_U32 tileNum,
    IN GT_U32 pdsNum,
    IN GT_U32 queueNumber,
    OUT  GT_U32 * profileIndexPtr
)
{
    GT_U32 value[2] = {0,0};
    GT_STATUS rc;
    CPSS_DXCH_TABLE_ENT table;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_PDS_NUM_MAC(pdsNum);

        /*not supported*/
     if(PRV_CPSS_SIP_6_30_CHECK_MAC(devNum)==GT_TRUE)
    {
        if(PRV_CPSS_SIP_7_CHECK_MAC(devNum)==GT_FALSE)
        {
            return GT_OK;
        }
    }

    if(queueNumber>=CPSS_DXCH_SIP_6_MAX_DATA_PATH_QUEUE_NUM_MAC)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }


    if(PRV_CPSS_SIP_7_CHECK_MAC(devNum)==GT_FALSE)
    {
        rc =prvCpssSip6TxqRegisterFieldRead(devNum,tileNum,
            PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pds[pdsNum].Queue_PDS_Profile[queueNumber],
            0,32,&value[0]);

        if(rc == GT_OK)
        {
            *profileIndexPtr  = (GT_BOOL)U32_GET_FIELD_MAC(value[0],
                             TXQ_PDS_UNTIL_6_20_FIELD_GET(devNum,QUEUE_PDS_PROFILE_QUEUE_PROFILE_3_0_FIELD_OFFSET),
                             TXQ_PDS_UNTIL_6_20_FIELD_GET(devNum,QUEUE_PDS_PROFILE_QUEUE_PROFILE_3_0_FIELD_SIZE));
        }
    }
    else
    {
        table= CPSS_DXCH_SIP7_TXQ_PDS_QUEUE_TO_PROFILE_MAP_E + pdsNum + tileNum * CPSS_DXCH_SIP_7_MAX_LOCAL_PDS_NUM_MAC;

        rc = prvCpssDxChReadTableEntry(devNum,table,
                                        queueNumber>>3/*each entry 8 queues*/,
                                        value);
        if(rc!=GT_OK)
        {
           return rc;
        }

        /*low 4 queues*/
        if((queueNumber&0x7)<4)
        {
               *profileIndexPtr  = (GT_BOOL)U32_GET_FIELD_MAC(value[0],
                             (queueNumber&0x7)*TXQ_AAS_PDS_QUEUE_TO_PROFILE_MAP_QUEUE_PROFILE_FIELD_SIZE,
                             TXQ_AAS_PDS_QUEUE_TO_PROFILE_MAP_QUEUE_PROFILE_FIELD_SIZE);
        }
        else
        {
               *profileIndexPtr  = (GT_BOOL)U32_GET_FIELD_MAC(value[1],
                             ((queueNumber&0x7)-4)*TXQ_AAS_PDS_QUEUE_TO_PROFILE_MAP_QUEUE_PROFILE_FIELD_SIZE,
                             TXQ_AAS_PDS_QUEUE_TO_PROFILE_MAP_QUEUE_PROFILE_FIELD_SIZE);
        }
    }

    return rc;
}

/**
* @internal prvCpssDxChTxqFalconPdsQueueLengthAdjustProfileMapSet function
* @endinternal
*
* @brief   Set lenght adjust profile to PDS queue.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                -  device number
* @param[in] tileNum               - The number of tile (Applicable range 0..3)
* @param[in] pdsNum                - the local number of PDS (APPLICABLE RANGES:0..7).
* @param[in] queueNumber           - the number of the queue(APPLICABLE RANGES:0..255)
* @param[in] profileIndex          - index of the profile(APPLICABLE RANGES:0..15.)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChTxqFalconPdsQueueLengthAdjustProfileMapSet
(
    IN GT_U8 devNum,
    IN GT_U32 tileNum,
    IN GT_U32 pdsNum,
    IN GT_U32 queueNumber,
    IN GT_U32 profileIndex
)
{
    GT_U32 regValue = 0;
    GT_STATUS rc;
    PRV_CPSS_DXCH_TXQ_SIP_6_REG_LOG_STC log;

    /*only supported from HAWK until  Ironman*/
    if(GT_TRUE==PRV_CPSS_TXQ_PER_DEVICE_DB_MAC(devNum).pdsProfilesCombined)
    {
      CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_APPLICABLE_DEVICE, LOG_ERROR_NO_MSG);
    }

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_PDS_NUM_MAC(pdsNum);

    if(profileIndex>=PDS_PROFILE_MAX_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(queueNumber>=CPSS_DXCH_SIP_6_MAX_DATA_PATH_QUEUE_NUM_MAC)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    U32_SET_FIELD_MASKED_MAC(regValue,
        TXQ_PDS_UNTIL_6_30_START_6_10_FIELD_GET(devNum,QUEUE_LENGTH_ADJUST_PROFILE_QUEUE_LENGTH_ADJUST_PROFILE_FIELD_OFFSET),
        TXQ_PDS_UNTIL_6_30_START_6_10_FIELD_GET(devNum,QUEUE_LENGTH_ADJUST_PROFILE_QUEUE_LENGTH_ADJUST_PROFILE_FIELD_SIZE),
        profileIndex);


   rc = prvCpssSip6TxqPdsLogInit(&log,pdsNum,"Queue_Length_Adjust_Profile");
   if (rc != GT_OK)
   {
     return rc;
   }

   log.regIndex = queueNumber;

   rc =prvCpssSip6TxqRegisterFieldWrite(devNum,tileNum,&log,
        PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pds[pdsNum].Queue_Length_Adjust_Profile[queueNumber],
        0,32,regValue);

    return rc;
}

/**
* @internal prvCpssDxChTxqFalconPdsQueueLengthAdjustProfileMapGet function
* @endinternal
*
* @brief   Get lenght adjust profile binded to PDS queue.
*
* @note   APPLICABLE DEVICES:      AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                - device number
* @param[in] tileNum               - The number of tile (Applicable range 0..3)
* @param[in] pdsNum                - the local number of PDS (APPLICABLE RANGES:0..7).
* @param[in] queueNumber           - the number of the queue(APPLICABLE RANGES:0..255)
* @param[out] profileIndexPtr      - (pointer to)index of the profile(APPLICABLE RANGES:0..15.)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChTxqFalconPdsQueueLengthAdjustProfileMapGet
(
    IN GT_U8 devNum,
    IN GT_U32 tileNum,
    IN GT_U32 pdsNum,
    IN GT_U32 queueNumber,
    OUT  GT_U32 * profileIndexPtr
)
{
    GT_U32 regValue = 0;
    GT_STATUS rc;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_PDS_NUM_MAC(pdsNum);

    if(queueNumber>=CPSS_DXCH_SIP_6_MAX_DATA_PATH_QUEUE_NUM_MAC)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc =prvCpssSip6TxqRegisterFieldRead(devNum,tileNum,
        PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pds[pdsNum].Queue_Length_Adjust_Profile[queueNumber],
        0,32,&regValue);

    if(rc == GT_OK)
     {

        *profileIndexPtr  = (GT_BOOL)U32_GET_FIELD_MAC(regValue,
                         TXQ_PDS_UNTIL_6_30_START_6_10_FIELD_GET(devNum,QUEUE_LENGTH_ADJUST_PROFILE_QUEUE_LENGTH_ADJUST_PROFILE_FIELD_OFFSET),
                         TXQ_PDS_UNTIL_6_30_START_6_10_FIELD_GET(devNum,QUEUE_LENGTH_ADJUST_PROFILE_QUEUE_LENGTH_ADJUST_PROFILE_FIELD_SIZE));
    }

    return rc;
}

/**
* @internal prvCpssDxChTxqFalconPdsTotalPdsDescCounterGet function
* @endinternal
*
* @brief   Counts the number of descriptors of all queues in both the cache and the PB
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] tileNum               - The number of tile (Applicable range 0..3)
* @param[in] pdsNum                - the local number of PDS (APPLICABLE RANGES:0..7).
*
* @param[out] pdsDescCountPtr      - the number of descriptors of all queues in both the cache and the PB
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChTxqFalconPdsTotalPdsDescCounterGet
(
    IN GT_U8 devNum,
    IN GT_U32 tileNum,
    IN GT_U32 pdsNum,
    OUT  GT_U32 * pdsDescCountPtr
)
{
    GT_U32 regAddr,tileOffset ;
    GT_U32 regValue = 0;
    GT_STATUS rc;

    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC * regAddrData;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_PDS_NUM_MAC(pdsNum);

    regAddrData = &(PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1);

    regAddr = regAddrData->TXQ.pds[pdsNum].Total_PDS_Counter;

    tileOffset = prvCpssSip6TileOffsetGet(devNum,tileNum);
    regAddr+=tileOffset;

    rc = prvCpssHwPpReadRegister(devNum,regAddr,&regValue);

    if(rc == GT_OK)
     {
        *pdsDescCountPtr = U32_GET_FIELD_MAC(regValue,TXQ_PDS_FIELD_GET(devNum,TOTAL_PDS_COUNTER_TOTAL_PDS_COUNTER_FIELD_OFFSET),
            TXQ_PDS_FIELD_GET(devNum,TOTAL_PDS_COUNTER_TOTAL_PDS_COUNTER_FIELD_SIZE));
     }

    return rc;
}


/**
* @internal prvCpssDxChTxqFalconPdsCacheDescCounterGet function
* @endinternal
*
* @brief   Counts the number of descriptors of all queues in the cache (not including the PB)
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] tileNum               - The number of tile (Applicable range 0..3)
* @param[in] pdsNum                - the local number of PDS (APPLICABLE RANGES:0..7).
*
* @param[out] pdsDescCountPtr      - the number of descriptors of all queues in the cache (not including the PB)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChTxqFalconPdsCacheDescCounterGet
(
    IN GT_U8 devNum,
    IN GT_U32 tileNum,
    IN GT_U32 pdsNum,
    OUT  GT_U32 * pdsDescCountPtr
)
{
    GT_U32 regAddr,tileOffset ;
    GT_U32 regValue = 0;
    GT_STATUS rc;

    PRV_CPSS_DXCH_PP_REGS_ADDR_VER1_STC * regAddrData;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_PDS_NUM_MAC(pdsNum);

     /*not supported*/
    if(PRV_CPSS_SIP_6_30_CHECK_MAC(devNum)==GT_TRUE)
    {
        return GT_OK;
    }

    regAddrData = &(PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1);

    regAddr = regAddrData->TXQ.pds[pdsNum].PDS_Cache_Counter;

     tileOffset = prvCpssSip6TileOffsetGet(devNum,tileNum);
     regAddr+=tileOffset;

    rc = prvCpssHwPpReadRegister(devNum,regAddr,&regValue);

    if(rc == GT_OK)
     {
        *pdsDescCountPtr = U32_GET_FIELD_MAC(regValue,TXQ_PDS_EXCLUDE_6_30_FIELD_GET(devNum,PDS_CACHE_COUNTER_PDS_CACHE_COUNTER_FIELD_OFFSET),
            TXQ_PDS_EXCLUDE_6_30_FIELD_GET(devNum,PDS_CACHE_COUNTER_PDS_CACHE_COUNTER_FIELD_SIZE));
     }

    return rc;
}

/**
* @internal prvCpssDxChTxqFalconPdsPerQueueCountersGet function
* @endinternal
*
* @brief   Counts the number of descriptors per queues in the cache and  the PB
*
* @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] tileNum               - The number of tile (Applicable range 0..3)
* @param[in] pdsNum                - the local number of PDS (APPLICABLE RANGES:0..7).
* @param[in] queueNumber           - the local queue number (APPLICABLE RANGES:0..399).
*
* @param[out] perQueueDescCountPtr - the number of descriptors at the queue
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChTxqFalconPdsPerQueueCountersGet
(
    IN GT_U8 devNum,
    IN GT_U32 tileNum,
    IN GT_U32 pdsNum,
    IN GT_U32 queueNumber,
    IN PRV_CPSS_DXCH_SIP6_TXQ_PDS_PER_QUEUE_COUNTER_STC * perQueueDescCountPtr
)
{
    GT_STATUS rc = GT_OK;
    GT_U32 regValue[2],size,value;
    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);
    TXQ_SIP_6_CHECK_LOCAL_PDS_NUM_MAC(pdsNum);
    CPSS_NULL_PTR_CHECK_MAC(perQueueDescCountPtr);

     /*not supported*/
    if(PRV_CPSS_SIP_6_30_CHECK_MAC(devNum)==GT_TRUE)
    {
        return GT_OK;
    }

    if(PRV_CPSS_PP_MAC(devNum)->isGmDevice)
    {
        queueNumber = queueNumber;/*avoid  warning*/
        perQueueDescCountPtr->headCounter =0;
        perQueueDescCountPtr->longQueue=GT_FALSE;
    }
    else
    {
        rc = prvCpssDxChReadTableEntry(devNum,
                                    CPSS_DXCH_SIP6_TXQ_PDS_PER_QUEUE_COUNTERS_E+pdsNum+tileNum*CPSS_DXCH_SIP_6_MAX_LOCAL_PDS_NUM_MAC,
                                    queueNumber,
                                    regValue);

        if(rc == GT_OK)
        {
            perQueueDescCountPtr->longQueue = (GT_BOOL)U32_GET_FIELD_MAC(regValue[0],
                     TXQ_PDS_EXCLUDE_6_30_FIELD_GET(devNum,PER_QUEUE_COUNTERS_LONG_FIELD_OFFSET),
                      TXQ_PDS_EXCLUDE_6_30_FIELD_GET(devNum,PER_QUEUE_COUNTERS_LONG_FIELD_SIZE));

            perQueueDescCountPtr->fragIndex = (GT_BOOL)U32_GET_FIELD_MAC(regValue[0],
                     TXQ_PDS_EXCLUDE_6_30_FIELD_GET(devNum,PER_QUEUE_COUNTERS_FIDX_FIELD_OFFSET),
                      TXQ_PDS_EXCLUDE_6_30_FIELD_GET(devNum,PER_QUEUE_COUNTERS_FIDX_FIELD_SIZE));

            perQueueDescCountPtr->headCounter = (GT_BOOL)U32_GET_FIELD_MAC(regValue[0],
                     TXQ_PDS_EXCLUDE_6_30_FIELD_GET(devNum,PER_QUEUE_COUNTERS_HEAD_COUNTER_FIELD_OFFSET),
                      TXQ_PDS_EXCLUDE_6_30_FIELD_GET(devNum,PER_QUEUE_COUNTERS_HEAD_COUNTER_FIELD_SIZE));

            perQueueDescCountPtr->fragCounter = (GT_BOOL)U32_GET_FIELD_MAC(regValue[0],
                     TXQ_PDS_EXCLUDE_6_30_FIELD_GET(devNum,PER_QUEUE_COUNTERS_FRAG_COUNTER_FIELD_OFFSET),
                      TXQ_PDS_EXCLUDE_6_30_FIELD_GET(devNum,PER_QUEUE_COUNTERS_FRAG_COUNTER_FIELD_SIZE));

                     size = 32 - TXQ_PDS_EXCLUDE_6_30_FIELD_GET(devNum,PER_QUEUE_COUNTERS_TAIL_COUNTER_FIELD_OFFSET);

            perQueueDescCountPtr->tailCounter = 0;

            perQueueDescCountPtr->tailCounter= U32_GET_FIELD_MAC(regValue[0],
                            TXQ_PDS_EXCLUDE_6_30_FIELD_GET(devNum,PER_QUEUE_COUNTERS_TAIL_COUNTER_FIELD_OFFSET),
                size);

            value =  U32_GET_FIELD_MAC(regValue[1],
                    0,
                    TXQ_PDS_EXCLUDE_6_30_FIELD_GET(devNum,PER_QUEUE_COUNTERS_TAIL_COUNTER_FIELD_SIZE)- size);

            perQueueDescCountPtr->tailCounter|= ((value)<<size);
        }
    }

    return rc;
}

GT_STATUS prvCpssDxChTxqFalconPdsDataStorageGet
(
    IN GT_U8 devNum,
    IN GT_U32 tileNum,
    IN GT_U32 pdsNum,
    IN GT_U32 descriptorInd,
    IN GT_U32 *descriptorPtr
)
{
    GT_STATUS rc = GT_OK;
    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);
    TXQ_SIP_6_CHECK_LOCAL_PDS_NUM_MAC(pdsNum);
    CPSS_NULL_PTR_CHECK_MAC(descriptorPtr);

    if(descriptorInd>TXQ_PDS_MAX_DESC_NUMBER_MAC)
    {
      CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "descriptorInd[%d] must be less than [%d]",descriptorInd,TXQ_PDS_MAX_DESC_NUMBER_MAC);
    }


    rc = prvCpssDxChReadTableEntry(devNum,
                                    CPSS_DXCH_SIP6_TXQ_PDS_DATA_STORAGE_E+pdsNum+tileNum*CPSS_DXCH_SIP_6_MAX_LOCAL_PDS_NUM_MAC,
                                    descriptorInd,
                                    descriptorPtr);
    return rc;
}

/**
* @internal prvCpssDxChTxqSip6PdsPbReadReqFifoLimitSet function
* @endinternal
*
* @brief  Configure read request Fifo limit
*
* @note   APPLICABLE DEVICES:AC5X;Harrier; AAS;
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;Falcon; AC5P;Ironman; .
*
* @param[in] devNum                - device number
* @param[in] pdsNum                - the local number of PDS (APPLICABLE RANGES:0..7).
* @param[in] pbReadReqFifoLimit    - fifo limit.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pdx number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChTxqSip6PdsPbReadReqFifoLimitSet
(
    IN GT_U8 devNum,
    IN GT_U32 tileNum,
    IN GT_U32 pdsNum,
    IN GT_U32 pbReadReqFifoLimit
)
{
    GT_U32 regAddr;
    GT_U32 regValue = 0;
    GT_STATUS rc;
    PRV_CPSS_DXCH_TXQ_SIP_6_REG_LOG_STC log;

    if(GT_FALSE == PRV_CPSS_SIP_6_15_CHECK_MAC(devNum))
    {
        return GT_OK;
    }

     /*not supported*/
    if(PRV_CPSS_SIP_6_30_CHECK_MAC(devNum)==GT_TRUE)
    {
        if(PRV_CPSS_SIP_7_CHECK_MAC(devNum)==GT_FALSE)
        {
            return GT_OK;
        }
    }

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_PDS_NUM_MAC(pdsNum);


    if(pbReadReqFifoLimit>= (GT_U32)(1<<PRV_PDS_EXCLUDE_6_30_START_6_15_END_7_DEV_DEP_FIELD_GET_MAC(devNum,PB_READ_REQ_FIFO_LIMIT_PB_RD_REQ_FIFO_THRESHOLD_FIELD_SIZE)))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssSip6TxqPdsLogInit(&log,pdsNum,"PB_Read_Req_FIFO_Limit");
    if (rc != GT_OK)
    {
      return rc;
    }

    regAddr = PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pds[pdsNum].PB_Read_Req_FIFO_Limit ;

    U32_SET_FIELD_MASKED_MAC(regValue,PRV_PDS_EXCLUDE_6_30_START_6_15_END_7_DEV_DEP_FIELD_GET_MAC(devNum,PB_READ_REQ_FIFO_LIMIT_PB_RD_REQ_FIFO_THRESHOLD_FIELD_OFFSET),
        PRV_PDS_EXCLUDE_6_30_START_6_15_END_7_DEV_DEP_FIELD_GET_MAC(devNum,PB_READ_REQ_FIFO_LIMIT_PB_RD_REQ_FIFO_THRESHOLD_FIELD_SIZE),
        pbReadReqFifoLimit);

    rc = prvCpssSip6TxqRegisterFieldWrite(devNum,tileNum,&log,regAddr,0,32,regValue);
    if(rc!=GT_OK)
    {
        return rc;
    }

    return rc;
}

/**
* @internal prvCpssSip6TxqPdsDebugInterruptDisableSet function
* @endinternal
*
* @brief   Debug function that disable iterrupt in PDS
*
* @note   APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] tileNum               - The number of tile (Applicable range 0..3)
* @param[in] pdsNum                - data path index
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong sdq number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssSip6TxqPdsDebugInterruptDisableSet
(
    IN GT_U8 devNum,
    IN GT_U32 tileNum,
    IN GT_U32 pdsNum
)
{
    PRV_CPSS_DXCH_TXQ_SIP_6_REG_LOG_STC log;
    GT_STATUS rc;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);
    TXQ_SIP_6_CHECK_LOCAL_PDS_NUM_MAC(pdsNum);


    rc = prvCpssSip6TxqPdsLogInit(&log,pdsNum,"PDS_Interrupt_Summary_Mask");
    if (rc != GT_OK)
    {
      return rc;
    }

    return prvCpssSip6TxqRegisterFieldWrite(devNum,
                 tileNum,&log,
                 PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pds[pdsNum].PDS_Interrupt_Summary_Mask,
                 0,32,0);
}

/**
* @internal prvCpssSip6TxqPdsDebugInterruptGet function
* @endinternal
*
* @brief   Get interrupt cause for PDS
*
* @note   APPLICABLE DEVICES:       Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
*
* @param[in] devNum                - device number
* @param[in] tileNum               - The number of tile (Applicable range 0..3)
* @param[in] sdqNum                - data path index
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong sdq number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssSip6TxqPdsDebugInterruptGet
(
    IN GT_U8  devNum,
    IN GT_U32 tileNum,
    IN GT_U32 pdsNum,
    OUT GT_U32 *functionalPtr,
    OUT GT_U32 *debugPtr
)
{
    GT_STATUS rc;

    CPSS_NULL_PTR_CHECK_MAC(functionalPtr);
    CPSS_NULL_PTR_CHECK_MAC(debugPtr);

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_PDS_NUM_MAC(pdsNum);

    rc = prvCpssSip6TxqRegisterFieldRead(devNum,
                 tileNum,
                 PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pds[pdsNum].PDS_Interrupt_Functional_Cause,
                 0,32,functionalPtr);
    if(rc!=GT_OK)
    {
      CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssSip6TxqRegisterFieldRead  failed \n");
    }

    rc = prvCpssSip6TxqRegisterFieldRead(devNum,
                 tileNum,
                 PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pds[pdsNum].PDS_Interrupt_Debug_Cause,
                 0,32,debugPtr);

    return rc;
}

/**
* @internal prvCpssSip7TxqDpdsModeSet function
* @endinternal
*
* @brief  Set all required configuration for PDS to work as DPDS.(Required for deep buffer feature)
*
* @note   APPLICABLE DEVICES      :  AAS
* @note   NOT APPLICABLE DEVICES  :  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                - device number
* @param[in] tileNum               - The number of tile (Applicable range 0..3)
* @param[in] pdsNum                - data path index
* @param[in] enable                - enable DPDS mode
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pds number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssSip7TxqDpdsModeSet
(
    IN GT_U8  devNum,
    IN GT_U32 tileNum,
    IN GT_U32  pdsNum,
    IN GT_BOOL enable
)
{
    PRV_CPSS_DXCH_TXQ_SIP_6_REG_LOG_STC log;
    GT_STATUS rc;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_PDS_NUM_MAC(pdsNum);


    rc = prvCpssSip6TxqPdsLogInit(&log,pdsNum,"Global_PDS_CFG");
    if (rc != GT_OK)
    {
      return rc;
    }

    rc = prvCpssSip6TxqRegisterFieldWrite(devNum,
                 tileNum,&log,
                 PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pds[pdsNum].Global_PDS_CFG,
                 TXQ_AAS_PDS_GLOBAL_PDS_CFG_FRAG_ATTR_FIFO_ENABLE_FIELD_OFFSET,
                 TXQ_AAS_PDS_GLOBAL_PDS_CFG_FRAG_ATTR_FIFO_ENABLE_FIELD_SIZE,
                 BOOL2BIT_MAC(enable));
    return rc;
}


/**
* @internal prvCpssSip7TxqDpdsModeGet function
* @endinternal
*
* @brief  Return GT_TRUE for DPDS,GT_FALSE for NPDS.
*
* @note   APPLICABLE DEVICES      :  AAS
* @note   NOT APPLICABLE DEVICES  :  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                - device number
* @param[in] tileNum               - The number of tile (Applicable range 0..3)
* @param[in] pdsNum                - data path index
* @param[in] enable                - enable DPDS mode
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pds number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssSip7TxqDpdsModeGet
(
    IN GT_U8  devNum,
    IN GT_U32 tileNum,
    IN GT_U32  pdsNum,
    IN GT_BOOL *enablePtr
)
{
    GT_STATUS rc;
    GT_U32    data = 0;

    CPSS_NULL_PTR_CHECK_MAC(enablePtr);

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_PDS_NUM_MAC(pdsNum);

    rc = prvCpssSip6TxqRegisterFieldRead(devNum,
                 tileNum,
                 PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pds[pdsNum].Global_PDS_CFG,
                 TXQ_AAS_PDS_GLOBAL_PDS_CFG_FRAG_ATTR_FIFO_ENABLE_FIELD_OFFSET,
                 TXQ_AAS_PDS_GLOBAL_PDS_CFG_FRAG_ATTR_FIFO_ENABLE_FIELD_SIZE,
                 &data);

    *enablePtr = BIT2BOOL_MAC(data);

    return rc;
}


/**
* @internal prvCpssSip7TxqPdsIdSet function
* @endinternal
*
* @brief  Set unique global PDS ID (Required for deep buffer feature).
*
* @note   APPLICABLE DEVICES      :  AAS
* @note   NOT APPLICABLE DEVICES  :  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                - device number
* @param[in] tileNum               - The number of tile (Applicable range 0..3)
* @param[in] pdsNum                - data path index
* @param[in] id                    - unique global PDS id
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pds number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssSip7TxqPdsIdSet
(
    IN GT_U8  devNum,
    IN GT_U32 tileNum,
    IN GT_U32 pdsNum,
    IN GT_U32 id
)
{
    PRV_CPSS_DXCH_TXQ_SIP_6_REG_LOG_STC log;
    GT_STATUS rc;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_PDS_NUM_MAC(pdsNum);


    rc = prvCpssSip6TxqPdsLogInit(&log,pdsNum,"PDS_ID");
    if (rc != GT_OK)
    {
      return rc;
    }

    rc = prvCpssSip6TxqRegisterFieldWrite(devNum,
                 tileNum,&log,
                 PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pds[pdsNum].PDS_ID,
                 TXQ_AAS_PDS_PDS_ID_PDS_ID_FIELD_OFFSET,
                 TXQ_AAS_PDS_PDS_ID_PDS_ID_FIELD_SIZE,
                 id);
    return rc;
}

/**
* @internal prvCpssSip7TxqPdsIdGet function
* @endinternal
*
* @brief  Get unique global PDS ID (Required for deep buffer feature).
*
* @note   APPLICABLE DEVICES      :  AAS
* @note   NOT APPLICABLE DEVICES  :  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                - device number
* @param[in] tileNum               - The number of tile (Applicable range 0..3)
* @param[in] pdsNum                - data path index
* @param[out] id                   - (pointer to)unique global PDS id
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pds number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssSip7TxqPdsIdGet
(
    IN GT_U8  devNum,
    IN GT_U32 tileNum,
    IN GT_U32 pdsNum,
    OUT GT_U32 *idPtr
)
{
    GT_STATUS rc;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_PDS_NUM_MAC(pdsNum);
    CPSS_NULL_PTR_CHECK_MAC(idPtr);

    rc = prvCpssSip6TxqRegisterFieldRead(devNum,
                 tileNum,
                 PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pds[pdsNum].PDS_ID,
                 TXQ_AAS_PDS_PDS_ID_PDS_ID_FIELD_OFFSET,
                 TXQ_AAS_PDS_PDS_ID_PDS_ID_FIELD_SIZE,
                 idPtr);
    return rc;
}

/**
* @internal prvCpssSip7TxqNpds2DpdsConnectionSet function
* @endinternal
*
* @brief  This function perform bi-directional configuration of PDS mailbox (Required for deep buffer feature).
*
* @note   APPLICABLE DEVICES      :  AAS
* @note   NOT APPLICABLE DEVICES  :  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                - device number
* @param[in] connectionPtr         - (pointer to) data structure that contain all relevant connection parameters
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pds number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssSip7TxqNpds2DpdsConnectionSet
(
    IN GT_U8   devNum,
    IN PRV_CPSS_TXQ_NPDS2DPDS_CONNECTION_STC  * connectionPtr
)
{
     PRV_CPSS_DXCH_TXQ_SIP_6_REG_LOG_STC log;
     GT_STATUS rc;
     GT_U32    regValue = 0;
     GT_U32    i,sourcePds,targetPds;
     GT_U32    sourceMailBoxId,targetMailBoxId;
     GT_U32    sourceTile,targetTile;

     CPSS_NULL_PTR_CHECK_MAC(connectionPtr);

     TXQ_SIP_6_CHECK_TILE_NUM_MAC(connectionPtr->tile[0]);
     TXQ_SIP_6_CHECK_TILE_NUM_MAC(connectionPtr->tile[1]);

     TXQ_SIP_6_CHECK_LOCAL_PDS_NUM_MAC(connectionPtr->pds[0]);
     TXQ_SIP_6_CHECK_LOCAL_PDS_NUM_MAC(connectionPtr->pds[1]);

     TXQ_SIP_6_CHECK_MAIL_BOX_ID_MAC(connectionPtr->mailBoxId[0]);
     TXQ_SIP_6_CHECK_MAIL_BOX_ID_MAC(connectionPtr->mailBoxId[1]);

     sourceTile = connectionPtr->tile[0];
     targetTile = connectionPtr->tile[1];
     sourcePds = connectionPtr->pds[0];
     targetPds = connectionPtr->pds[1];
     sourceMailBoxId = connectionPtr->mailBoxId[0];
     targetMailBoxId = connectionPtr->mailBoxId[1];
     rc = prvCpssSip6TxqPdsLogInit(&log,targetPds,"Mailbox_Dram_PDS_Queues");
     if (rc != GT_OK)
     {
       return rc;
     }
     log.regIndex = targetMailBoxId;

     /*At first iteration DPS is a target*/

     rc = prvCpssSip6TxqRegisterFieldWrite(devNum,
                 targetTile,&log,
                 PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pds[targetPds].Mailbox_Dram_PDS_Queues[targetMailBoxId],
                 TXQ_AAS_PDS_MAILBOX_DRAM_PDS_QUEUES_MBOX_DRAM_PDS_QUEUES_FIELD_OFFSET,
                 TXQ_AAS_PDS_MAILBOX_DRAM_PDS_QUEUES_MBOX_DRAM_PDS_QUEUES_FIELD_SIZE,
                 1);
     if (rc != GT_OK)
     {
       return rc;
     }


     /*Perform two iterations . [Source<->Target ]and also [Target<->Source]*/
     for(i=0;i<2;i++)
     {
         rc = prvCpssSip6TxqPdsLogInit(&log,sourcePds,"Mailbox_Target_Mailbox");
         if (rc != GT_OK)
         {
           return rc;
         }

         log.regIndex = sourceMailBoxId;

         U32_SET_FIELD_MASKED_MAC(regValue,TXQ_AAS_PDS_MAILBOX_TARGET_MAILBOX_MBOX_TARGET_MAILBOX_FIELD_OFFSET,
            TXQ_AAS_PDS_MAILBOX_TARGET_MAILBOX_MBOX_TARGET_MAILBOX_FIELD_SIZE,
            targetMailBoxId);

         U32_SET_FIELD_MASKED_MAC(regValue,TXQ_AAS_PDS_MAILBOX_TARGET_MAILBOX_MBOX_TARGET_PDS_ID_FIELD_OFFSET,
            TXQ_AAS_PDS_MAILBOX_TARGET_MAILBOX_MBOX_TARGET_PDS_ID_FIELD_SIZE,
            PRV_TXQ_PDS_GLOBAL_ID_MAC(devNum,targetTile,targetPds));

         rc = prvCpssSip6TxqRegisterFieldWrite(devNum,
                 sourceTile,&log,
                 PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pds[sourcePds].Mailbox_Target_Mailbox[sourceMailBoxId],
                 0,
                 32,
                 regValue);

         if(rc!=GT_OK)
         {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
         }

         regValue = 0;


         rc = prvCpssSip6TxqPdsLogInit(&log,sourcePds,"Mailbox_Priority");
         if (rc != GT_OK)
         {
           return rc;
         }

         log.regIndex = sourceMailBoxId;

         rc = prvCpssSip6TxqRegisterFieldWrite(devNum,
                     sourceTile,&log,
                     PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pds[sourcePds].Mailbox_Priority[sourceMailBoxId],
                     TXQ_AAS_PDS_MAILBOX_PRIORITY_MBOX_PRIORITY_FIELD_OFFSET,
                     TXQ_AAS_PDS_MAILBOX_PRIORITY_MBOX_PRIORITY_FIELD_SIZE,
                     BOOL2BIT_MAC(connectionPtr->hiPriority));

         if(rc!=GT_OK)
         {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
         }


         /*Swap for reverse direction - next iteration.
           Now update target mailbox.*/
         sourceTile = connectionPtr->tile[1];
         targetTile = connectionPtr->tile[0];
         sourcePds = connectionPtr->pds[1];
         targetPds = connectionPtr->pds[0];
         sourceMailBoxId = connectionPtr->mailBoxId[1];
         targetMailBoxId = connectionPtr->mailBoxId[0];
     }

     return GT_OK;
}
/**
* @internal prvCpssSip7TxqPdsMailBoxGet function
* @endinternal
*
* @brief  This function getonfiguration of PDS mailbox (Required for deep buffer feature).
*
* @note   APPLICABLE DEVICES      :  AAS
* @note   NOT APPLICABLE DEVICES  :  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                - device number
* @param[inout] connectionPtr      - (pointer to) data structure that contain all relevant connection parameters.
*                                    Note: For this function only first member of each array is used as input. Second member is the output.
*
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pds number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssSip7TxqPdsMailBoxGet
(
    IN GT_U8     devNum,
    IN GT_U32    tileInd,
    IN GT_U32    pdsId,
    IN GT_U32    mailBoxId,
    OUT GT_U32   *pdsIdOutPtr,
    OUT GT_U32   *mailBoxIdOutPtr,
    OUT GT_BOOL  *hiPriorityPtr
)
{
    GT_STATUS rc;
    GT_U32     regValue = 0;

    CPSS_NULL_PTR_CHECK_MAC(pdsIdOutPtr);
    CPSS_NULL_PTR_CHECK_MAC(mailBoxIdOutPtr);
    CPSS_NULL_PTR_CHECK_MAC(hiPriorityPtr);

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileInd);

    TXQ_SIP_6_CHECK_LOCAL_PDS_NUM_MAC(pdsId);
    TXQ_SIP_6_CHECK_MAIL_BOX_ID_MAC(mailBoxId);

    rc = prvCpssSip6TxqRegisterFieldRead(devNum,
                 tileInd,
                 PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pds[pdsId].Mailbox_Target_Mailbox[mailBoxId],
                 0,32,&regValue);

    if(rc!=GT_OK)
    {
      CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    *mailBoxIdOutPtr = U32_GET_FIELD_MAC(regValue,
                    TXQ_AAS_PDS_MAILBOX_TARGET_MAILBOX_MBOX_TARGET_MAILBOX_FIELD_OFFSET,
                    TXQ_AAS_PDS_MAILBOX_TARGET_MAILBOX_MBOX_TARGET_MAILBOX_FIELD_SIZE);

    *pdsIdOutPtr/*this is a global id*/= U32_GET_FIELD_MAC(regValue,
                    TXQ_AAS_PDS_MAILBOX_TARGET_MAILBOX_MBOX_TARGET_PDS_ID_FIELD_OFFSET,
                    TXQ_AAS_PDS_MAILBOX_TARGET_MAILBOX_MBOX_TARGET_PDS_ID_FIELD_SIZE);

    rc = prvCpssSip6TxqRegisterFieldRead(devNum,
                 tileInd,
                 PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pds[pdsId].Mailbox_Priority[mailBoxId],
                 TXQ_AAS_PDS_MAILBOX_PRIORITY_MBOX_PRIORITY_FIELD_OFFSET,
                 TXQ_AAS_PDS_MAILBOX_PRIORITY_MBOX_PRIORITY_FIELD_SIZE,
                 &regValue);

    if(rc!=GT_OK)
    {
      CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    *hiPriorityPtr = BIT2BOOL_MAC(regValue);

    return GT_OK;
}



/**
* @internal prvCpssSip7TxqPdsIdGet function
* @endinternal
*
* @brief  Get DRAM queue configuration for mailbox
*
* @note   APPLICABLE DEVICES      :  AAS
* @note   NOT APPLICABLE DEVICES  :  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                - device number
* @param[in] tileNum               - The number of tile (Applicable range 0..3)
* @param[in] pdsNum                - data path index
* @param[in] mailBoxNum            - mailbox index(Applicable range 0..31)
* @param[out] dramQueuePtr         - (pointer to)mailbox used for DRAM
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pds number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssSip7TxqDpdsMailBoxDramQueueGet
(
    IN  GT_U8    devNum,
    IN  GT_U32   tileNum,
    IN  GT_U32   pdsNum,
    IN  GT_U32   mailBoxNum,
    OUT GT_U32  *dramQueuePtr
)
{
    CPSS_NULL_PTR_CHECK_MAC(dramQueuePtr);
    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);
    TXQ_SIP_6_CHECK_LOCAL_PDS_NUM_MAC(pdsNum);
    TXQ_SIP_6_CHECK_MAIL_BOX_ID_MAC(mailBoxNum);


    return   prvCpssSip6TxqRegisterFieldRead(devNum,
                 tileNum,
                 PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pds[pdsNum].Mailbox_Dram_PDS_Queues[mailBoxNum],
                 TXQ_AAS_PDS_MAILBOX_DRAM_PDS_QUEUES_MBOX_DRAM_PDS_QUEUES_FIELD_OFFSET,
                 TXQ_AAS_PDS_MAILBOX_DRAM_PDS_QUEUES_MBOX_DRAM_PDS_QUEUES_FIELD_SIZE,
                 dramQueuePtr);


}

/**
* @internal prvCpssSip7TxqPdsEpbProfileSet function
* @endinternal
*
* @brief  Configure spillover profile (Required for deep buffer feature).
*
* @note   APPLICABLE DEVICES      :  AAS
* @note   NOT APPLICABLE DEVICES  :  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                - device number
* @param[in] tileNum               - The number of tile (Applicable range 0..3)
* @param[in] pdsNum                - data path index
* @param[in] profileInd            - PDS profile
* @param[in] epbProfilePtr         - (pointer to)EPB profile data
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pds number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssSip7TxqPdsEpbProfileSet
(
    IN  GT_U8    devNum,
    IN  GT_U32   tileNum,
    IN  GT_U32   pdsNum,
    IN  GT_U32   profileInd,
    IN  PRV_CPSS_DXCH_SIP7_TXQ_EPB_QUEUE_PROFILE_STC  *epbProfilePtr
)
{
    GT_U32 i;
    PRV_CPSS_DXCH_TXQ_SIP_6_REG_LOG_STC log;
    GT_STATUS rc;
    GT_U32    regValue = 0;
    GT_BOOL   configureMailBoxId = GT_FALSE;

    CPSS_NULL_PTR_CHECK_MAC(epbProfilePtr);
    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);
    TXQ_SIP_6_CHECK_LOCAL_PDS_NUM_MAC(pdsNum);


    if(profileInd>=PDS_PROFILE_MAX_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }


    rc = prvCpssSip6TxqPdsLogInit(&log,pdsNum,"profile_epb_spillover_control");
    if (rc != GT_OK)
    {
      return rc;
    }

    log.regIndex = profileInd;

    switch(epbProfilePtr->type)
    {
        case PRV_CPSS_DXCH_SIP7_TXQ_EPB_QUEUE_PROFILE_TYPE_NPDS_E:
            for(i=0;i<PRV_TXQ_SIP_7_MAX_DPDS_PER_DEVICE_NUM_CNS;i++)
            {
                TXQ_SIP_6_CHECK_MAIL_BOX_ID_MAC(epbProfilePtr->mailBoxId[i]);
            }
            U32_SET_FIELD_MASKED_MAC(regValue,
                TXQ_AAS_PDS_PROFILE_EPB_SPILLOVER_CONTROL_PROFILE_EPB_ENABLE_FIELD_OFFSET,
                TXQ_AAS_PDS_PROFILE_EPB_SPILLOVER_CONTROL_PROFILE_EPB_ENABLE_FIELD_SIZE,
                BOOL2BIT_MAC(epbProfilePtr->epbSpilloverEnable));
                configureMailBoxId = epbProfilePtr->epbSpilloverEnable;
            break;
        case PRV_CPSS_DXCH_SIP7_TXQ_EPB_QUEUE_PROFILE_TYPE_DPDS_RX_E:
            U32_SET_FIELD_MASKED_MAC(regValue,
                TXQ_AAS_PDS_PROFILE_EPB_SPILLOVER_CONTROL_PROFILE_RXQUEUE_FIELD_OFFSET,
                TXQ_AAS_PDS_PROFILE_EPB_SPILLOVER_CONTROL_PROFILE_RXQUEUE_FIELD_SIZE,
                1);
            break;
        case PRV_CPSS_DXCH_SIP7_TXQ_EPB_QUEUE_PROFILE_TYPE_DPDS_TX_E:
            U32_SET_FIELD_MASKED_MAC(regValue,
                TXQ_AAS_PDS_PROFILE_EPB_SPILLOVER_CONTROL_PROFILE_TXQUEUE_FIELD_OFFSET,
                TXQ_AAS_PDS_PROFILE_EPB_SPILLOVER_CONTROL_PROFILE_TXQUEUE_FIELD_SIZE,
                1);
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Invalid epbProfilePtr->type %d\n",epbProfilePtr->type);
            break;
    }


    rc = prvCpssSip6TxqRegisterFieldWrite(devNum,
            tileNum,&log,
            PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pds[pdsNum].profile_epb_spillover_control[profileInd],
            0,
            32,
            regValue);

    if(rc!=GT_OK)
    {
       CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    if(GT_TRUE==configureMailBoxId)
    {
        regValue = 0;

        rc = prvCpssSip6TxqPdsLogInit(&log,pdsNum,"Profile_Mailbox_ID");
        if (rc != GT_OK)
        {
          return rc;
        }

        U32_SET_FIELD_MASKED_MAC(regValue,
                TXQ_AAS_PDS_PROFILE_MAILBOX_ID_PROFILE_MAILBOX_ID_DRAM_0_FIELD_OFFSET,
                TXQ_AAS_PDS_PROFILE_MAILBOX_ID_PROFILE_MAILBOX_ID_DRAM_0_FIELD_SIZE,
                epbProfilePtr->mailBoxId[0]);
        U32_SET_FIELD_MASKED_MAC(regValue,
                TXQ_AAS_PDS_PROFILE_MAILBOX_ID_PROFILE_MAILBOX_ID_DRAM_1_FIELD_OFFSET,
                TXQ_AAS_PDS_PROFILE_MAILBOX_ID_PROFILE_MAILBOX_ID_DRAM_1_FIELD_SIZE,
                epbProfilePtr->mailBoxId[1]);

        rc = prvCpssSip6TxqRegisterFieldWrite(devNum,
            tileNum,&log,
            PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pds[pdsNum].Profile_Mailbox_ID[profileInd],
            0,
            32,
            regValue);

        if(rc!=GT_OK)
        {
           CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }
    }

    return GT_OK;
}

GT_STATUS prvCpssSip7TxqPdsEpbProfileRawGet
(
    IN  GT_U8    devNum,
    IN  GT_U32   tileNum,
    IN  GT_U32   pdsNum,
    IN  GT_U32   profileInd,
    OUT  GT_BOOL  *epbSpilloverEnablePtr,
    OUT  GT_U32   *mailBox0IdPtr,
    OUT  GT_BOOL  *mailBox1IdPtr,
    OUT  GT_U32   *typePtr
)
{
    GT_STATUS rc;
    GT_U32    regValue = 0;

    CPSS_NULL_PTR_CHECK_MAC(epbSpilloverEnablePtr);
    CPSS_NULL_PTR_CHECK_MAC(mailBox0IdPtr);
    CPSS_NULL_PTR_CHECK_MAC(mailBox1IdPtr);
    CPSS_NULL_PTR_CHECK_MAC(typePtr);

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);
    TXQ_SIP_6_CHECK_LOCAL_PDS_NUM_MAC(pdsNum);

    if(profileInd>=PDS_PROFILE_MAX_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

   rc = prvCpssSip6TxqRegisterFieldRead(devNum,
                 tileNum,
                 PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pds[pdsNum].profile_epb_spillover_control[profileInd],
                 0,32,&regValue);

    if(rc!=GT_OK)
    {
      CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    *epbSpilloverEnablePtr = U32_GET_FIELD_MAC(regValue,
                    TXQ_AAS_PDS_PROFILE_EPB_SPILLOVER_CONTROL_PROFILE_EPB_ENABLE_FIELD_OFFSET,
                    TXQ_AAS_PDS_PROFILE_EPB_SPILLOVER_CONTROL_PROFILE_EPB_ENABLE_FIELD_SIZE);

    *typePtr = U32_GET_FIELD_MAC(regValue,
                    TXQ_AAS_PDS_PROFILE_EPB_SPILLOVER_CONTROL_PROFILE_RXQUEUE_FIELD_OFFSET,
                    /*Read both at once*/
                    TXQ_AAS_PDS_PROFILE_EPB_SPILLOVER_CONTROL_PROFILE_RXQUEUE_FIELD_SIZE+
                    TXQ_AAS_PDS_PROFILE_EPB_SPILLOVER_CONTROL_PROFILE_TXQUEUE_FIELD_SIZE);



    rc = prvCpssSip6TxqRegisterFieldRead(devNum,
                 tileNum,
                 PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pds[pdsNum].Profile_Mailbox_ID[profileInd],
                 0,32,&regValue);

    if(rc!=GT_OK)
    {
      CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    *mailBox0IdPtr = U32_GET_FIELD_MAC(regValue,
                    TXQ_AAS_PDS_PROFILE_MAILBOX_ID_PROFILE_MAILBOX_ID_DRAM_0_FIELD_OFFSET,
                    TXQ_AAS_PDS_PROFILE_MAILBOX_ID_PROFILE_MAILBOX_ID_DRAM_0_FIELD_SIZE);
    *mailBox1IdPtr = U32_GET_FIELD_MAC(regValue,
                    TXQ_AAS_PDS_PROFILE_MAILBOX_ID_PROFILE_MAILBOX_ID_DRAM_1_FIELD_OFFSET,
                    TXQ_AAS_PDS_PROFILE_MAILBOX_ID_PROFILE_MAILBOX_ID_DRAM_1_FIELD_SIZE);

    return GT_OK;
}


/**
* @internal prvCpssSip7TxqPdsEpbProfileGet function
* @endinternal
*
* @brief  Get configuration of spillover profile (Required for deep buffer feature).
*
* @note   APPLICABLE DEVICES      :  AAS
* @note   NOT APPLICABLE DEVICES  :  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                - device number
* @param[in] tileNum               - The number of tile (Applicable range 0..3)
* @param[in] pdsNum                - data path index
* @param[in] profileInd            - PDS profile
* @param[out] epbProfilePtr        - (pointer to)EPB profile data
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pds number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssSip7TxqPdsEpbProfileGet
(
    IN  GT_U8    devNum,
    IN  GT_U32   tileNum,
    IN  GT_U32   pdsNum,
    IN  GT_U32   profileInd,
    OUT  PRV_CPSS_DXCH_SIP7_TXQ_EPB_QUEUE_PROFILE_STC  *epbProfilePtr
)
{
    GT_STATUS rc;
    GT_U32    type;

    CPSS_NULL_PTR_CHECK_MAC(epbProfilePtr);

    rc = prvCpssSip7TxqPdsEpbProfileRawGet(devNum,tileNum,pdsNum,profileInd,&(epbProfilePtr->epbSpilloverEnable),
        &(epbProfilePtr->mailBoxId[0]),&(epbProfilePtr->mailBoxId[1]),&type);

    if(rc!=GT_OK)
    {
      CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    switch(type)
    {
        case 0x0:
            epbProfilePtr->type = PRV_CPSS_DXCH_SIP7_TXQ_EPB_QUEUE_PROFILE_TYPE_NPDS_E;
            break;
        case 0x1:
            epbProfilePtr->type = PRV_CPSS_DXCH_SIP7_TXQ_EPB_QUEUE_PROFILE_TYPE_DPDS_RX_E;
            break;
        case 0x2:
            epbProfilePtr->type = PRV_CPSS_DXCH_SIP7_TXQ_EPB_QUEUE_PROFILE_TYPE_DPDS_TX_E;
            break;
        default:
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Invalid epbProfile->type 0x%x\n",type);
            break;
    }

    return GT_OK;
}

/**
* @internal prvCpssSip7TxqPdsMailBoxCopyBackDataGet function
* @endinternal
*
* @brief  Get copyback data (Required for deep buffer feature).
*
* @note   APPLICABLE DEVICES      :  AAS
* @note   NOT APPLICABLE DEVICES  :  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                - device number
* @param[in] tileNum               - The number of tile (Applicable range 0..3)
* @param[in] pdsNum                - data path index
* @param[in] profileInd            - PDS profile
* @param[out] reqCntPtr             - (pointer to)Counts the number of copyback requests that were sent to the EPB and not yet replied
* @param[out] reqCntPtr             - (pointer to)A limit for the max number of open copyback requests from this mailbox towards the EPB.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pds number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssSip7TxqPdsMailBoxCopyBackDataGet
(
    IN  GT_U8    devNum,
    IN  GT_U32   tileNum,
    IN  GT_U32   pdsNum,
    IN  GT_U32   mailBoxNum,
    OUT GT_U32  *reqCntPtr,
    OUT GT_U32  *reqLimitPtr
)
{
    GT_STATUS rc;

    CPSS_NULL_PTR_CHECK_MAC(reqCntPtr);
    CPSS_NULL_PTR_CHECK_MAC(reqLimitPtr);
    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);
    TXQ_SIP_6_CHECK_LOCAL_PDS_NUM_MAC(pdsNum);
    TXQ_SIP_6_CHECK_MAIL_BOX_ID_MAC(mailBoxNum);


    rc = prvCpssSip6TxqRegisterFieldRead(devNum,
                 tileNum,
                 PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pds[pdsNum].Mailbox_Copy_Back_Requests_Counter[mailBoxNum],
                 TXQ_AAS_PDS_MAILBOX_COPY_BACK_REQUESTS_COUNTER_MBOX_COPYBACK_REQ_CNT_FIELD_OFFSET,
                 TXQ_AAS_PDS_MAILBOX_COPY_BACK_REQUESTS_COUNTER_MBOX_COPYBACK_REQ_CNT_FIELD_SIZE,
                 reqCntPtr);


    if(rc!=GT_OK)
    {
      CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssSip6TxqRegisterFieldRead(devNum,
                 tileNum,
                 PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pds[pdsNum].Mailbox_Copy_Back_Requests_Limit[mailBoxNum],
                 TXQ_AAS_PDS_MAILBOX_COPY_BACK_REQUESTS_LIMIT_MBOX_COPYBACK_REQ_LIMIT_FIELD_OFFSET,
                 TXQ_AAS_PDS_MAILBOX_COPY_BACK_REQUESTS_LIMIT_MBOX_COPYBACK_REQ_LIMIT_FIELD_SIZE,
                 reqLimitPtr);


    if(rc!=GT_OK)
    {
      CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    return GT_OK;


}

/**
* @internal prvCpssSip7TxqPdsProfileDynamicThresholdEnableSet function
* @endinternal
*
* @brief  Enable/disable dynamic thresholds.
*
* @note   APPLICABLE DEVICES      :  AAS
* @note   NOT APPLICABLE DEVICES  :  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                - device number
* @param[in] tileNum               - The number of tile (Applicable range 0..3)
* @param[in] pdsNum                - data path index
* @param[in] profileInd            - PDS profile
* @param[in] enablePtr             - (pointer to)Dynamic thresholds enable.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pds number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssSip7TxqPdsProfileDynamicThresholdEnableSet
(
    IN  GT_U8    devNum,
    IN  GT_U32   tileNum,
    IN  GT_U32   pdsNum,
    IN  GT_U32   profileInd,
    IN  PRV_CPSS_DXCH_SIP7_TXQ_PDS_DYN_THR_ENABLE_STC  * enablePtr
)
{
    GT_U32 regValue = 0;
    PRV_CPSS_DXCH_TXQ_SIP_6_REG_LOG_STC log;
    GT_STATUS rc;

    CPSS_NULL_PTR_CHECK_MAC(enablePtr);
    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);
    TXQ_SIP_6_CHECK_LOCAL_PDS_NUM_MAC(pdsNum);


    if(profileInd>=PDS_PROFILE_MAX_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssSip6TxqPdsLogInit(&log,pdsNum,"Profile_Dynamic_Thresholds_Enable");
    if (rc != GT_OK)
    {
      return rc;
    }

    log.regIndex = profileInd;

    U32_SET_FIELD_MASKED_MAC(regValue,
            TXQ_AAS_PDS_PROFILE_DYNAMIC_THRESHOLDS_ENABLE_PROFILE_OPEN_DYNAMIC_ENABLE_FIELD_OFFSET,
            TXQ_AAS_PDS_PROFILE_DYNAMIC_THRESHOLDS_ENABLE_PROFILE_OPEN_DYNAMIC_ENABLE_FIELD_SIZE,
            BOOL2BIT_MAC(enablePtr->openLongEnable));
    U32_SET_FIELD_MASKED_MAC(regValue,
            TXQ_AAS_PDS_PROFILE_DYNAMIC_THRESHOLDS_ENABLE_PROFILE_SPILLOVER_DYNAMIC_ENABLE_FIELD_OFFSET,
            TXQ_AAS_PDS_PROFILE_DYNAMIC_THRESHOLDS_ENABLE_PROFILE_SPILLOVER_DYNAMIC_ENABLE_FIELD_SIZE,
            BOOL2BIT_MAC(enablePtr->spilloverEnable));

     U32_SET_FIELD_MASKED_MAC(regValue,
            TXQ_AAS_PDS_PROFILE_DYNAMIC_THRESHOLDS_ENABLE_PROFILE_COPYBACK_DYNAMIC_ENABLE_FIELD_OFFSET,
            TXQ_AAS_PDS_PROFILE_DYNAMIC_THRESHOLDS_ENABLE_PROFILE_COPYBACK_DYNAMIC_ENABLE_FIELD_SIZE,
            BOOL2BIT_MAC(enablePtr->copybackEnable));


    rc = prvCpssSip6TxqRegisterFieldWrite(devNum,
            tileNum,&log,
            PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pds[pdsNum].Profile_Dynamic_Thresholds_Enable[profileInd],
            0,
            32,
            regValue);

   if(rc!=GT_OK)
   {
      CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
   }

   return GT_OK;
}

/**
* @internal prvCpssSip7TxqPdsProfileDynamicThresholdEnableGet function
* @endinternal
*
* @brief  Get enable/disable dynamic thresholds.
*
* @note   APPLICABLE DEVICES      :  AAS
* @note   NOT APPLICABLE DEVICES  :  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                - device number
* @param[in] tileNum               - The number of tile (Applicable range 0..3)
* @param[in] pdsNum                - data path index
* @param[in] profileInd            - PDS profile
* @param[in] enablePtr             - (pointer to)Dynamic thresholds enable.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pds number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssSip7TxqPdsProfileDynamicThresholdEnableGet
(
    IN  GT_U8    devNum,
    IN  GT_U32   tileNum,
    IN  GT_U32   pdsNum,
    IN  GT_U32   profileInd,
    IN  PRV_CPSS_DXCH_SIP7_TXQ_PDS_DYN_THR_ENABLE_STC  * enablePtr
)
{
    GT_U32 regValue = 0;
    GT_STATUS rc;

    CPSS_NULL_PTR_CHECK_MAC(enablePtr);
    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);
    TXQ_SIP_6_CHECK_LOCAL_PDS_NUM_MAC(pdsNum);


    if(profileInd>=PDS_PROFILE_MAX_MAC(devNum))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssSip6TxqRegisterFieldRead(devNum,
            tileNum,
            PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pds[pdsNum].Profile_Dynamic_Thresholds_Enable[profileInd],
            0,
            32,
            &regValue);

   if(rc!=GT_OK)
   {
      CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
   }

   enablePtr->openLongEnable = (GT_BOOL)U32_GET_FIELD_MAC(regValue,
                     TXQ_AAS_PDS_PROFILE_DYNAMIC_THRESHOLDS_ENABLE_PROFILE_OPEN_DYNAMIC_ENABLE_FIELD_OFFSET,
                     TXQ_AAS_PDS_PROFILE_DYNAMIC_THRESHOLDS_ENABLE_PROFILE_OPEN_DYNAMIC_ENABLE_FIELD_SIZE);

   enablePtr->spilloverEnable = (GT_BOOL)U32_GET_FIELD_MAC(regValue,
                     TXQ_AAS_PDS_PROFILE_DYNAMIC_THRESHOLDS_ENABLE_PROFILE_SPILLOVER_DYNAMIC_ENABLE_FIELD_OFFSET,
                     TXQ_AAS_PDS_PROFILE_DYNAMIC_THRESHOLDS_ENABLE_PROFILE_SPILLOVER_DYNAMIC_ENABLE_FIELD_SIZE);

   enablePtr->copybackEnable = (GT_BOOL)U32_GET_FIELD_MAC(regValue,
                     TXQ_AAS_PDS_PROFILE_DYNAMIC_THRESHOLDS_ENABLE_PROFILE_COPYBACK_DYNAMIC_ENABLE_FIELD_OFFSET,
                     TXQ_AAS_PDS_PROFILE_DYNAMIC_THRESHOLDS_ENABLE_PROFILE_COPYBACK_DYNAMIC_ENABLE_FIELD_SIZE);

   return GT_OK;
}

/**
* @internal prvCpssSip7TxqPdsProfileDynamicThDebugEnableSet function
* @endinternal
*
* @brief  Set enable/disable dynamic specific thresholds . Required for debug configuration.
*
* @note   APPLICABLE DEVICES      :  AAS
* @note   NOT APPLICABLE DEVICES  :  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                - device number
* @param[in] tileNum               - The number of tile (Applicable range 0..3)
* @param[in] pdsNum                - data path index
* @param[in] profileInd            - PDS profile
* @param[in] thrInd                - (0 - copybackEnable ,1-spilloverEnable,2 -openLongEnable)
* @param[in] enablePtr             - (pointer to)Dynamic thresholds enable.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pds number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssSip7TxqPdsProfileDynamicThDebugEnableSet
(
    IN  GT_U8    devNum,
    IN  GT_U32   tileNum,
    IN  GT_U32   pdsNum,
    IN  GT_U32   profileInd,
    IN  GT_U32   thrInd,
    IN  GT_BOOL  enable
)
{
    PRV_CPSS_DXCH_SIP7_TXQ_PDS_DYN_THR_ENABLE_STC enableStc;
    GT_STATUS rc;

    rc = prvCpssSip7TxqPdsProfileDynamicThresholdEnableGet(devNum,tileNum,pdsNum,profileInd,&enableStc);
    if(rc!=GT_OK)
    {
      CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    switch (thrInd)
    {
        case 0:
         enableStc.copybackEnable = enable;
         break;

        case 1:
         enableStc.spilloverEnable = enable;
         break;

        default:
         enableStc.openLongEnable = enable;
         break;
    }

    rc = prvCpssSip7TxqPdsProfileDynamicThresholdEnableSet(devNum,tileNum,pdsNum,profileInd,&enableStc);
    if(rc!=GT_OK)
    {
      CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    return GT_OK;
}

/**
* @internal prvCpssSip7TxqPdsPortPdsAvailableSizeSet function
* @endinternal
*
* @brief  Set total PDS available size (In descriptors)
*  Used in dynamic calculation of Port Count Threshold
*
* @note   APPLICABLE DEVICES      :  AAS
* @note   NOT APPLICABLE DEVICES  :  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                - device number
* @param[in] tileNum               - The number of tile (Applicable range 0..3)
* @param[in] pdsNum                - data path index
* @param[in] localPort             - local port index(Applicable range 0..22)
* @param[in] pdsSize               - PDS available size (In descriptors)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pds number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssSip7TxqPdsPortPdsAvailableSizeSet
(
    IN GT_U8  devNum,
    IN GT_U32 tileNum,
    IN GT_U32 pdsNum,
    IN GT_U32 localPort,
    IN GT_U32 pdsSize
)
{
    PRV_CPSS_DXCH_TXQ_SIP_6_REG_LOG_STC log;
    GT_STATUS rc;
    GT_U32 offset,size;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_PDS_NUM_MAC(pdsNum);

    TXQ_SIP_6_CHECK_LOCAL_PORT_NUM_MAC(devNum,localPort);

    rc = prvCpssSip6TxqPdsLogInit(&log,pdsNum,"Port_PDS_Available_Threshold");
    if (rc != GT_OK)
    {
      return rc;
    }

    log.regIndex = localPort;

    offset = TXQ_AAS_PDS_PORT_PDS_AVAILABLE_THRESHOLD_PORT_PDS_AVAIL_SIZE_FIELD_OFFSET;
    size = TXQ_AAS_PDS_PORT_PDS_AVAILABLE_THRESHOLD_PORT_PDS_AVAIL_SIZE_FIELD_SIZE;

    if(pdsSize>=(GT_U32)1<<size)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return prvCpssSip6TxqRegisterFieldWrite(devNum,
            tileNum,&log,
            PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pds[pdsNum].Port_PDS_Available_Threshold[localPort],
            offset,
            size,
            pdsSize);
}

/**
* @internal prvCpssSip7TxqPdsPortPdsAvailableSizeGet function
* @endinternal
*
* @brief  Get total PDS available size (In descriptors)
*  Used in dynamic calculation of Port Count Threshold
*
* @note   APPLICABLE DEVICES      :  AAS
* @note   NOT APPLICABLE DEVICES  :  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                - device number
* @param[in] tileNum               - The number of tile (Applicable range 0..3)
* @param[in] pdsNum                - data path index
* @param[in] localPort             - local port index(Applicable range 0..22)
* @param[out] pdsSizePtr               - (pointer to)PDS available size (In descriptors)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pds number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssSip7TxqPdsPortPdsAvailableSizeGet
(
    IN GT_U8  devNum,
    IN GT_U32 tileNum,
    IN GT_U32 pdsNum,
    IN GT_U32 localPort,
    OUT GT_U32 *pdsSizePtr
)
{

    GT_U32 offset,size;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_PDS_NUM_MAC(pdsNum);

    CPSS_NULL_PTR_CHECK_MAC(pdsSizePtr);

    TXQ_SIP_6_CHECK_LOCAL_PORT_NUM_MAC(devNum,localPort);

    offset = TXQ_AAS_PDS_PORT_PDS_AVAILABLE_THRESHOLD_PORT_PDS_AVAIL_SIZE_FIELD_OFFSET;
    size = TXQ_AAS_PDS_PORT_PDS_AVAILABLE_THRESHOLD_PORT_PDS_AVAIL_SIZE_FIELD_SIZE;

    return prvCpssSip6TxqRegisterFieldRead(devNum,
            tileNum,
            PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pds[pdsNum].Port_PDS_Available_Threshold[localPort],
            offset,
            size,
            pdsSizePtr);
}
/**
* @internal prvCpssSip7TxqPdsPortPbAvailableSizeSet function
* @endinternal
*
* @brief  Set total PB available size (In units of 128 Bytes)
*         Used in dynamic calculation of Port BC Threshold .
*
* @note   APPLICABLE DEVICES      :  AAS
* @note   NOT APPLICABLE DEVICES  :  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                - device number
* @param[in] tileNum               - The number of tile (Applicable range 0..3)
* @param[in] pdsNum                - data path index
* @param[in] localPort             - local port index(Applicable range 0..22)
* @param[in] pbSize                - PB available size (In units of 128 Bytes)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pds number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssSip7TxqPdsPortPbAvailableSizeSet
(
    IN GT_U8  devNum,
    IN GT_U32 tileNum,
    IN GT_U32 pdsNum,
    IN GT_U32 localPort,
    IN GT_U32 pbSize
)
{
    PRV_CPSS_DXCH_TXQ_SIP_6_REG_LOG_STC log;
    GT_STATUS rc;
    GT_U32 offset,size;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_PDS_NUM_MAC(pdsNum);

    TXQ_SIP_6_CHECK_LOCAL_PORT_NUM_MAC(devNum,localPort);

    rc = prvCpssSip6TxqPdsLogInit(&log,pdsNum,"Port_PB_Available_Threshold");
    if (rc != GT_OK)
    {
      return rc;
    }

    log.regIndex = localPort;

    offset = TXQ_AAS_PDS_PORT_PB_AVAILABLE_THRESHOLD_PORT_PB_AVAIL_SIZE_FIELD_OFFSET;
    size = TXQ_AAS_PDS_PORT_PB_AVAILABLE_THRESHOLD_PORT_PB_AVAIL_SIZE_FIELD_SIZE;

    if(pbSize>=(GT_U32)1<<size)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return prvCpssSip6TxqRegisterFieldWrite(devNum,
            tileNum,&log,
            PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pds[pdsNum].Port_PB_Available_Threshold[localPort],
            offset,
            size,
            pbSize);
}
/**
* @internal prvCpssSip7TxqPdsPortPbAvailableSizeGet function
* @endinternal
*
* @brief  Get total PB available size (In units of 128 Bytes)
*         Used in dynamic calculation of Port BC Threshold .
*
* @note   APPLICABLE DEVICES      :  AAS
* @note   NOT APPLICABLE DEVICES  :  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                - device number
* @param[in] tileNum               - The number of tile (Applicable range 0..3)
* @param[in] pdsNum                - data path index
* @param[in] localPort             - local port index(Applicable range 0..22)
* @param[out] pbSizePtr            - (pointer to)PB available size (In units of 128 Bytes)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pds number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssSip7TxqPdsPortPbAvailableSizeGet
(
    IN GT_U8  devNum,
    IN GT_U32 tileNum,
    IN GT_U32 pdsNum,
    IN GT_U32 localPort,
    OUT GT_U32 *pbSizePtr
)
{

    GT_U32 offset,size;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_PDS_NUM_MAC(pdsNum);

    CPSS_NULL_PTR_CHECK_MAC(pbSizePtr);

    TXQ_SIP_6_CHECK_LOCAL_PORT_NUM_MAC(devNum,localPort);

    offset = TXQ_AAS_PDS_PORT_PB_AVAILABLE_THRESHOLD_PORT_PB_AVAIL_SIZE_FIELD_OFFSET;
    size = TXQ_AAS_PDS_PORT_PB_AVAILABLE_THRESHOLD_PORT_PB_AVAIL_SIZE_FIELD_SIZE;

    return prvCpssSip6TxqRegisterFieldRead(devNum,
            tileNum,
            PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pds[pdsNum].Port_PB_Available_Threshold[localPort],
            offset,
            size,
            pbSizePtr);
}

/**
* @internal prvCpssSip7TxqPdsPortPbDbaParamsSet function
* @endinternal
*
* @brief  Set Guaranteed BC Threshold (In units of 128 Bytes) and
*         Alpha configuration used to Calculate the congestion factor
*         Used in dynamic calculation of Port BC Final Threshold
*         Port BC Final Threshold calculated by: Gur_TH + congestion_factor x(PB Available - PB used)
*         Congestion_facor = (Alpha == 0) ? 0, else 2^(-8+Alpha).
*
* @note   APPLICABLE DEVICES      :  AAS
* @note   NOT APPLICABLE DEVICES  :  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                - device number
* @param[in] tileNum               - The number of tile (Applicable range 0..3)
* @param[in] pdsNum                - data path index
* @param[in] localPort             - local port index(Applicable range 0..22)
* @param[in] alfa                  - Congestion factor. (Alpha == 0) ? 0, else 2^(-8+Alpha)
* @param[in] garanteedTh           - Guaranteed BC Threshold (In units of 128 Bytes)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pds number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssSip7TxqPdsPortPbDbaParamsSet
(
    IN GT_U8  devNum,
    IN GT_U32 tileNum,
    IN GT_U32 pdsNum,
    IN GT_U32 localPort,
    IN GT_U32 alfa,
    IN GT_U32 garanteedTh

)
{
    PRV_CPSS_DXCH_TXQ_SIP_6_REG_LOG_STC log;
    GT_STATUS rc;
    GT_U32 offset,size,value = 0 ;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_PDS_NUM_MAC(pdsNum);

    TXQ_SIP_6_CHECK_LOCAL_PORT_NUM_MAC(devNum,localPort);

    rc = prvCpssSip6TxqPdsLogInit(&log,pdsNum,"Port_PB_Size_Thresholds");
    if (rc != GT_OK)
    {
      return rc;
    }

    log.regIndex = localPort;

    offset = TXQ_AAS_PDS_PORT_PB_SIZE_THRESHOLDS_PORT_PB_ALPHA_FIELD_OFFSET;
    size = TXQ_AAS_PDS_PORT_PB_SIZE_THRESHOLDS_PORT_PB_ALPHA_FIELD_SIZE;

    if(alfa>=(GT_U32)1<<size)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(garanteedTh>=(GT_U32)1<<TXQ_AAS_PDS_PORT_PB_SIZE_THRESHOLDS_PORT_BC_GUR_TH_FIELD_SIZE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    U32_SET_FIELD_MASKED_MAC(value,
            offset,
            size,
            alfa);

    offset = TXQ_AAS_PDS_PORT_PB_SIZE_THRESHOLDS_PORT_BC_GUR_TH_FIELD_OFFSET;
    size = TXQ_AAS_PDS_PORT_PB_SIZE_THRESHOLDS_PORT_BC_GUR_TH_FIELD_SIZE;

    U32_SET_FIELD_MASKED_MAC(value,
            offset,
            size,
            garanteedTh);

    return prvCpssSip6TxqRegisterFieldWrite(devNum,
            tileNum,&log,
            PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pds[pdsNum].Port_PB_Size_Thresholds[localPort],
            0,
            32,
            value);

}


/**
* @internal prvCpssSip7TxqPdsPortPbDbaParamsGet function
* @endinternal
*
* @brief  Get Guaranteed BC Threshold (In units of 128 Bytes) and
*         Alpha configuration used to Calculate the congestion factor
*         Used in dynamic calculation of Port BC Final Threshold
*         Port BC Final Threshold calculated by: Gur_TH + congestion_factor x(PB Available - PB used)
*         Congestion_facor = (Alpha == 0) ? 0, else 2^(-8+Alpha).
*
* @note   APPLICABLE DEVICES      :  AAS
* @note   NOT APPLICABLE DEVICES  :  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                - device number
* @param[in] tileNum               - The number of tile (Applicable range 0..3)
* @param[in] pdsNum                - data path index
* @param[in] localPort             - local port index(Applicable range 0..22)
* @param[out] alfaPtr              - (pointer to) Congestion factor. (Alpha == 0) ? 0, else 2^(-8+Alpha)
* @param[out] garanteedThPtr       - (pointer to) Guaranteed BC Threshold (In units of 128 Bytes)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pds number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssSip7TxqPdsPortPbDbaParamsGet
(
    IN GT_U8  devNum,
    IN GT_U32 tileNum,
    IN GT_U32 pdsNum,
    IN GT_U32 localPort,
    OUT GT_U32 *alfaPtr,
    OUT GT_U32 *garanteedThPtr
)
{
    GT_STATUS rc;
    GT_U32 offset,size,value;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_PDS_NUM_MAC(pdsNum);

    TXQ_SIP_6_CHECK_LOCAL_PORT_NUM_MAC(devNum,localPort);

    CPSS_NULL_PTR_CHECK_MAC(alfaPtr);
    CPSS_NULL_PTR_CHECK_MAC(garanteedThPtr);

    rc = prvCpssSip6TxqRegisterFieldRead(devNum,
            tileNum,
            PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pds[pdsNum].Port_PB_Size_Thresholds[localPort],
            0,
            32,
            &value);

    if(rc!=GT_OK)
    {
      CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    offset = TXQ_AAS_PDS_PORT_PB_SIZE_THRESHOLDS_PORT_PB_ALPHA_FIELD_OFFSET;
    size = TXQ_AAS_PDS_PORT_PB_SIZE_THRESHOLDS_PORT_PB_ALPHA_FIELD_SIZE;

    *alfaPtr = U32_GET_FIELD_MAC(value,
                     offset,
                     size);

    offset = TXQ_AAS_PDS_PORT_PB_SIZE_THRESHOLDS_PORT_BC_GUR_TH_FIELD_OFFSET;
    size = TXQ_AAS_PDS_PORT_PB_SIZE_THRESHOLDS_PORT_BC_GUR_TH_FIELD_SIZE;

    *garanteedThPtr = U32_GET_FIELD_MAC(value,
                     offset,
                     size);


    return GT_OK;

}


/**
* @internal prvCpssSip7TxqPdsPortPdsDbaParamsSet function
* @endinternal
*
* @brief  Set  Guaranteed Cnt Threshold (In descriptors) and
*         Alpha configuration used to Calculate the congestion factor
*         Used in dynamic calculation of Port Cnt Final  Threshold
*         Port Cnt Final Threshold calculated by: Gur_TH + congestion_factor x(PB Available - PB used)
*         Congestion_facor = (Alpha == 0) ? 0, else 2^(-8+Alpha).
*
* @note   APPLICABLE DEVICES      :  AAS
* @note   NOT APPLICABLE DEVICES  :  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                - device number
* @param[in] tileNum               - The number of tile (Applicable range 0..3)
* @param[in] pdsNum                - data path index
* @param[in] localPort             - local port index(Applicable range 0..22)
* @param[in] alfa                  - Congestion factor. (Alpha == 0) ? 0, else 2^(-8+Alpha)
* @param[in] garanteedTh           - Guaranteed Cnt Threshold (In descriptors)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pds number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssSip7TxqPdsPortPdsDbaParamsSet
(
    IN GT_U8  devNum,
    IN GT_U32 tileNum,
    IN GT_U32 pdsNum,
    IN GT_U32 localPort,
    IN GT_U32 alfa,
    IN GT_U32 garanteedTh

)
{
    PRV_CPSS_DXCH_TXQ_SIP_6_REG_LOG_STC log;
    GT_STATUS rc;
    GT_U32 offset,size,value = 0 ;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_PDS_NUM_MAC(pdsNum);

    TXQ_SIP_6_CHECK_LOCAL_PORT_NUM_MAC(devNum,localPort);

    rc = prvCpssSip6TxqPdsLogInit(&log,pdsNum,"Port_PDS_Size_Thresholds");
    if (rc != GT_OK)
    {
      return rc;
    }

    log.regIndex = localPort;

    offset = TXQ_AAS_PDS_PORT_PDS_SIZE_THRESHOLDS_PORT_PDS_ALPHA_FIELD_OFFSET;
    size = TXQ_AAS_PDS_PORT_PDS_SIZE_THRESHOLDS_PORT_PDS_ALPHA_FIELD_SIZE;

    if(alfa>=(GT_U32)1<<size)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    if(garanteedTh>=(GT_U32)1<<TXQ_AAS_PDS_PORT_PDS_SIZE_THRESHOLDS_PORT_CNT_GUR_TH_FIELD_SIZE)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    U32_SET_FIELD_MASKED_MAC(value,
            offset,
            size,
            alfa);

    offset = TXQ_AAS_PDS_PORT_PDS_SIZE_THRESHOLDS_PORT_CNT_GUR_TH_FIELD_OFFSET;
    size = TXQ_AAS_PDS_PORT_PDS_SIZE_THRESHOLDS_PORT_CNT_GUR_TH_FIELD_SIZE;

    U32_SET_FIELD_MASKED_MAC(value,
            offset,
            size,
            garanteedTh);

    return prvCpssSip6TxqRegisterFieldWrite(devNum,
            tileNum,&log,
            PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pds[pdsNum].Port_PDS_Size_Thresholds[localPort],
            0,
            32,
            value);

}


/**
* @internal prvCpssSip7TxqPdsPortPdsDbaParamsGet function
* @endinternal
*
* @brief  Get  Guaranteed Cnt Threshold (In descriptors) and
*         Alpha configuration used to Calculate the congestion factor
*         Used in dynamic calculation of Port Cnt Final  Threshold
*         Port Cnt Final Threshold calculated by: Gur_TH + congestion_factor x(PB Available - PB used)
*         Congestion_facor = (Alpha == 0) ? 0, else 2^(-8+Alpha).
*
*
* @note   APPLICABLE DEVICES      :  AAS
* @note   NOT APPLICABLE DEVICES  :  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                - device number
* @param[in] tileNum               - The number of tile (Applicable range 0..3)
* @param[in] pdsNum                - data path index
* @param[in] localPort             - local port index(Applicable range 0..22)
* @param[out] alfaPtr              - (pointer to) Congestion factor. (Alpha == 0) ? 0, else 2^(-8+Alpha)
* @param[out] garanteedThPtr       - (pointer to) Guaranteed Cnt Threshold (In descriptors)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pds number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssSip7TxqPdsPortPdsDbaParamsGet
(
    IN GT_U8  devNum,
    IN GT_U32 tileNum,
    IN GT_U32 pdsNum,
    IN GT_U32 localPort,
    OUT GT_U32 *alfaPtr,
    OUT GT_U32 *garanteedThPtr
)
{
    GT_STATUS rc;
    GT_U32 offset,size,value;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_PDS_NUM_MAC(pdsNum);

    TXQ_SIP_6_CHECK_LOCAL_PORT_NUM_MAC(devNum,localPort);

    CPSS_NULL_PTR_CHECK_MAC(alfaPtr);
    CPSS_NULL_PTR_CHECK_MAC(garanteedThPtr);

    rc = prvCpssSip6TxqRegisterFieldRead(devNum,
            tileNum,
            PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pds[pdsNum].Port_PDS_Size_Thresholds[localPort],
            0,
            32,
            &value);

    if(rc!=GT_OK)
    {
      CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    offset = TXQ_AAS_PDS_PORT_PB_SIZE_THRESHOLDS_PORT_PB_ALPHA_FIELD_OFFSET;
    size = TXQ_AAS_PDS_PORT_PB_SIZE_THRESHOLDS_PORT_PB_ALPHA_FIELD_SIZE;

    *alfaPtr = U32_GET_FIELD_MAC(value,
                     offset,
                     size);

    offset = TXQ_AAS_PDS_PORT_PB_SIZE_THRESHOLDS_PORT_BC_GUR_TH_FIELD_OFFSET;
    size = TXQ_AAS_PDS_PORT_PB_SIZE_THRESHOLDS_PORT_BC_GUR_TH_FIELD_SIZE;

    *garanteedThPtr = U32_GET_FIELD_MAC(value,
                     offset,
                     size);


    return GT_OK;

}
/**
* @internal prvCpssSip7TxqPdsPortFragSizeSet function
* @endinternal
*
* @brief  Set minimal number of frags in a queue to allow spillover.
*         Used in dynamic threshold mode.
*
* @note   APPLICABLE DEVICES      :  AAS
* @note   NOT APPLICABLE DEVICES  :  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                - device number
* @param[in] tileNum               - the number of tile (Applicable range 0..3)
* @param[in] pdsNum                - data path index
* @param[in] localPort             - local port index(Applicable range 0..22)
* @param[in] fragSize              - the minimal number of frags in a queue to allow spillover
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pds number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssSip7TxqPdsPortFragSizeSet
(
    IN GT_U8  devNum,
    IN GT_U32 tileNum,
    IN GT_U32 pdsNum,
    IN GT_U32 localPort,
    IN GT_U32 fragSize
)
{
    PRV_CPSS_DXCH_TXQ_SIP_6_REG_LOG_STC log;
    GT_STATUS rc;
    GT_U32 offset,size;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_PDS_NUM_MAC(pdsNum);

    TXQ_SIP_6_CHECK_LOCAL_PORT_NUM_MAC(devNum,localPort);

    rc = prvCpssSip6TxqPdsLogInit(&log,pdsNum,"Port_Frag_Spillover_Threshold");
    if (rc != GT_OK)
    {
      return rc;
    }

    log.regIndex = localPort;

    offset = TXQ_AAS_PDS_PORT_FRAG_SPILLOVER_THRESHOLD_PORT_FRAG_SPILLOVER_THRESHOLD_FIELD_OFFSET;
    size = TXQ_AAS_PDS_PORT_FRAG_SPILLOVER_THRESHOLD_PORT_FRAG_SPILLOVER_THRESHOLD_FIELD_SIZE;

    if(fragSize>=(GT_U32)1<<size)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return prvCpssSip6TxqRegisterFieldWrite(devNum,
            tileNum,&log,
            PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pds[pdsNum].Port_Frag_Spillover_Threshold[localPort],
            offset,
            size,
            fragSize);

}
/**
* @internal prvCpssSip7TxqPdsPortFragSizeGet function
* @endinternal
*
* @brief  Get minimal number of frags in a queue to allow spillover.
*         Used in dynamic threshold mode.
*
* @note   APPLICABLE DEVICES      :  AAS
* @note   NOT APPLICABLE DEVICES  :  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                - device number
* @param[in] tileNum               - the number of tile (Applicable range 0..3)
* @param[in] pdsNum                - data path index
* @param[in] localPort             - local port index(Applicable range 0..22)
* @param[out] fragSizePtr          - (pointer to) the minimal number of frags in a queue to allow spillover
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pds number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssSip7TxqPdsPortFragSizeGet
(
    IN  GT_U8  devNum,
    IN  GT_U32 tileNum,
    IN  GT_U32 pdsNum,
    IN  GT_U32 localPort,
    OUT GT_U32 *fragSizePtr
)
{
    GT_U32 offset,size;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_PDS_NUM_MAC(pdsNum);

    TXQ_SIP_6_CHECK_LOCAL_PORT_NUM_MAC(devNum,localPort);

    CPSS_NULL_PTR_CHECK_MAC(fragSizePtr);


    offset = TXQ_AAS_PDS_PORT_FRAG_SPILLOVER_THRESHOLD_PORT_FRAG_SPILLOVER_THRESHOLD_FIELD_OFFSET;
    size = TXQ_AAS_PDS_PORT_FRAG_SPILLOVER_THRESHOLD_PORT_FRAG_SPILLOVER_THRESHOLD_FIELD_SIZE;

    return prvCpssSip6TxqRegisterFieldRead(devNum,
            tileNum,
            PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pds[pdsNum].Port_Frag_Spillover_Threshold[localPort],
            offset,
            size,
            fragSizePtr);

}


/**
* @internal prvCpssSip7TxqPdsPortSelectFactorSet function
* @endinternal
*
* @brief  Set Margin factor used to scale the per queue select-in- window number
*         Used in dynamic threshold mode.
*
* @note   APPLICABLE DEVICES      :  AAS
* @note   NOT APPLICABLE DEVICES  :  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                - device number
* @param[in] tileNum               - the number of tile (Applicable range 0..3)
* @param[in] pdsNum                - data path index
* @param[in] localPort             - local port index(Applicable range 0..22)
* @param[in] factor                -  0x0 = x1; x1; Queue selection Factor by 1
*                                     0x1 = x1p25; x1p25; Queue selection Factor by 1.25
*                                     0x2 = x1p5; x1p5; Queue selection Factor by 1.5
*                                     0x3 = x2; x2; Queue selection Factor by 2
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pds number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssSip7TxqPdsPortSelectFactorSet
(
    IN GT_U8  devNum,
    IN GT_U32 tileNum,
    IN GT_U32 pdsNum,
    IN GT_U32 localPort,
    IN GT_U32 factor
)
{
    PRV_CPSS_DXCH_TXQ_SIP_6_REG_LOG_STC log;
    GT_STATUS rc;
    GT_U32 offset,size;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_PDS_NUM_MAC(pdsNum);

    TXQ_SIP_6_CHECK_LOCAL_PORT_NUM_MAC(devNum,localPort);

    rc = prvCpssSip6TxqPdsLogInit(&log,pdsNum,"Port_Head_Max_Factor");
    if (rc != GT_OK)
    {
      return rc;
    }

    log.regIndex = localPort;

    offset = TXQ_AAS_PDS_PORT_HEAD_MAX_FACTOR_PORT_SELECT_MAX_FACTOR_FIELD_OFFSET;
    size = TXQ_AAS_PDS_PORT_HEAD_MAX_FACTOR_PORT_SELECT_MAX_FACTOR_FIELD_SIZE;

    if(factor>=(GT_U32)1<<size)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    return prvCpssSip6TxqRegisterFieldWrite(devNum,
            tileNum,&log,
            PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pds[pdsNum].Port_Head_Max_Factor[localPort],
            offset,
            size,
            factor);

}



/**
* @internal prvCpssSip7TxqPdsPortSelectFactorGet function
* @endinternal
*
* @brief  Get Margin factor used to scale the per queue select-in- window number
*         Used in dynamic threshold mode.
*
* @note   APPLICABLE DEVICES      :  AAS
* @note   NOT APPLICABLE DEVICES  :  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                - device number
* @param[in] tileNum               - the number of tile (Applicable range 0..3)
* @param[in] pdsNum                - data path index
* @param[in] localPort             - local port index(Applicable range 0..22)
* @param[out] factorPtr            - (pointer to) 0x0 = x1; x1; Queue selection Factor by 1
*                                     0x1 = x1p25; x1p25; Queue selection Factor by 1.25
*                                     0x2 = x1p5; x1p5; Queue selection Factor by 1.5
*                                     0x3 = x2; x2; Queue selection Factor by 2
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong pds number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssSip7TxqPdsPortSelectFactorGet
(
    IN GT_U8  devNum,
    IN GT_U32 tileNum,
    IN GT_U32 pdsNum,
    IN GT_U32 localPort,
    OUT GT_U32 *factorPtr
)
{

    GT_U32 offset,size;

    TXQ_SIP_6_CHECK_TILE_NUM_MAC(tileNum);

    TXQ_SIP_6_CHECK_LOCAL_PDS_NUM_MAC(pdsNum);

    TXQ_SIP_6_CHECK_LOCAL_PORT_NUM_MAC(devNum,localPort);

    CPSS_NULL_PTR_CHECK_MAC(factorPtr);


    offset = TXQ_AAS_PDS_PORT_HEAD_MAX_FACTOR_PORT_SELECT_MAX_FACTOR_FIELD_OFFSET;
    size = TXQ_AAS_PDS_PORT_HEAD_MAX_FACTOR_PORT_SELECT_MAX_FACTOR_FIELD_SIZE;


    return prvCpssSip6TxqRegisterFieldRead(devNum,
            tileNum,
            PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pds[pdsNum].Port_Head_Max_Factor[localPort],
            offset,
            size,
            factorPtr);

}


GT_STATUS prvCpssSip7TxqPdsMarkDynamicProfileSet
(
    IN GT_U8  devNum,
    IN PRV_CPSS_DXCH_SIP6_TXQ_QUEUE_PROFILE_LONG_Q_STC *profilePtr
)
{
    CPSS_NULL_PTR_CHECK_MAC(profilePtr);

    profilePtr->headEmptyLimit = 0;
    profilePtr->longQueueLimit = (GT_U32)(1<<TXQ_PDS_EXCLUDE_6_30_FIELD_GET(devNum,PROFILE_LONG_QUEUE_LIMIT_PROFILE_LONG_QUEUE_LIMIT_FIELD_SIZE))-1;

    return GT_OK;
}


/**
* @internal prvCpssDxChTxqSip7PdsQueueRenamingEnableSet function
* @endinternal
*
* @brief   Enable/disable "queue renaming" feature
*
* @note   APPLICABLE DEVICES      :  AAS
* @note   NOT APPLICABLE DEVICES  :  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum               - device number
* @param[in] enable               - Enable/disable "queue renaming" feature
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong sdq number.
* @retval GT_HW_ERROR              - on writing to HW error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChTxqSip7PdsQueueRenamingEnableSet
(
    IN GT_U8   devNum,
    IN GT_BOOL enable
)
{
    GT_U32      regAddr,offset,size,i,j;
    GT_U32      numberOfTiles;
    PRV_CPSS_DXCH_TXQ_SIP_6_REG_LOG_STC log;
    GT_STATUS rc;

    PRV_CPSS_NUM_OF_TILE_GET_MAC(devNum,numberOfTiles)

    offset = TXQ_AAS_PDS_RENAMING_ENABLE_RENAMING_EN_FIELD_OFFSET;
    size   = TXQ_AAS_PDS_RENAMING_ENABLE_RENAMING_EN_FIELD_SIZE;


    for(i=0;i<numberOfTiles;i++)
    {
       for(j=0;j<MAX_DP_IN_TILE(devNum);j++)
       {
         rc = prvCpssSip6TxqPdsLogInit(&log,j,"Renaming_Enable");
         if (rc != GT_OK)
         {
           return rc;
         }

         regAddr =PRV_CPSS_DXCH_PP_MAC(devNum)->regsAddrVer1.TXQ.pds[j].Renaming_Enable;

         rc = prvCpssSip6TxqRegisterFieldWrite(devNum,i,&log,regAddr,offset,size,BOOL2BIT_MAC(enable));
         if(rc!=GT_OK)
         {
           CPSS_LOG_ERROR_AND_RETURN_MAC(rc, " prvCpssSip6TxqRegisterFieldWrite  failed \n");
         }
        }
    }

    return GT_OK;
}


