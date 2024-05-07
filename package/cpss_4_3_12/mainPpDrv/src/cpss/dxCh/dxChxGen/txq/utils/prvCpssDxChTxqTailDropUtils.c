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
* @file prvCpssDxChTxqTailDropUtils.c
*
* @brief CPSS SIP6 TXQ  tail drop high level functions
*
* @version   1
********************************************************************************
*/

#include <cpss/dxCh/dxChxGen/txq/private/prvCpssDxChTxqMain.h>
#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwInit.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/port/private/prvCpssDxChPortTxDba.h>
#include <cpss/dxCh/dxChxGen/cos/private/prvCpssDxChCoS.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

/*
Since there is some delay between the tail drop and the buffering,
we configure with some margin to prevent cases where we run out of buffer in a middle
of a packet and have to drop.
*/
#define PRV_CPSS_TD_GLOBAL_LIMIT_FACTOR_MAC (0.96)
#define PRV_CPSS_TD_AVAILABLE_BUF_FACTOR_MAC (0.96)
#define PRV_CPSS_SIP_6_15_TD_AVAILABLE_BUF_FACTOR_MAC (0.92)

#define PRV_CPSS_SIP_6_TD_PB_SIZE_PER_TILE_MAC     (98304)       /*Single tile = 12MB/128 = 96K cells*/
#define PRV_CPSS_SIP_6_10_TD_PB_SIZE_PER_TILE_MAC  (65536)      /*Single tile = 8MB/128 = 64K cells*/
#define PRV_CPSS_SIP_6_15_TD_PB_SIZE_PER_TILE_MAC  (24576)      /*Single tile = 3MB/128 = 24K cells*/
#define PRV_CPSS_SIP_6_20_TD_PB_SIZE_PER_TILE_MAC  (49152)      /*Single tile = 6MB/128 = 48K cells*/
#define PRV_CPSS_SIP_6_30_TD_PB_SIZE_PER_TILE_MAC  (24576)      /*Single tile = 3MB/128 = 24K cells*/
#define PRV_CPSS_SIP_7_TD_PB_SIZE_PER_TILE_MAC    (262144)      /*Single tile = 32MB/128 = 256K cells*/


#define PRV_CPSS_SIP7_TD_LP_FACTOR_MAC (0.8) /* SIP7 Low priority factor according to arch recommendation*/




/*1.Dynamic limits - DBA*/
#define PRV_CPSS_PORT_TX_TAIL_DROP_DBA_DEFAULT_QUEUE_DP_ALPHA CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_25_E
#define PRV_CPSS_PORT_TX_TAIL_DROP_DBA_DEFAULT_PORT_ALPHA     CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_E

#define PRV_CPSS_PORT_TX_TAIL_DROP_DBA_DEFAULT_PDX_ALPHA(_devNum) \
    (GT_FALSE == PRV_CPSS_SIP_6_10_CHECK_MAC(_devNum))?CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_1_E:CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_E

#define PRV_CPSS_PORT_TX_TAIL_DROP_DBA_DEFAULT_MC_LOW_ALPHA   CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_E
#define PRV_CPSS_PORT_TX_TAIL_DROP_DBA_DEFAULT_MC_HI_ALPHA  CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_E
#define PRV_CPSS_PORT_TX_TAIL_DROP_DBA_DEFAULT_QUEUE_DP_GURANTEED_LIMIT 0
#define PRV_CPSS_PORT_TX_TAIL_DROP_DBA_DEFAULT_PORT_GURANTEED_LIMIT     0
#define PRV_CPSS_PORT_TX_TAIL_DROP_DBA_DEFAULT_PDX_GURANTEED_LIMIT(_devNum)  (GT_FALSE == PRV_CPSS_SIP_6_10_CHECK_MAC(_devNum))?0 :0xFFFF
#define PRV_CPSS_PORT_TX_TAIL_DROP_DBA_DEFAULT_MC_LOW_GURANTEED_LIMIT   upperBound      /*set to max PB size */
#define PRV_CPSS_PORT_TX_TAIL_DROP_DBA_DEFAULT_MC_HI_GURANTEED_LIMIT    upperBound      /*set to max PB size */


/*2.Dynamic limits - WRED*/
#define PRV_CPSS_PORT_TX_TAIL_DROP_WRED_DEFAULT_QUEUE_DP_WRED_SIZE 0
#define PRV_CPSS_PORT_TX_TAIL_DROP_WRED_DEFAULT_PORT_WRED_SIZE     0
#define PRV_CPSS_PORT_TX_TAIL_DROP_WRED_DEFAULT_QUEUE_DP_DROP_PROB CPSS_PORT_TX_TAIL_DROP_PROBABILITY_100_E
#define PRV_CPSS_PORT_TX_TAIL_DROP_WRED_DEFAULT_PORT_DROP_PROB     CPSS_PORT_TX_TAIL_DROP_PROBABILITY_100_E
/*3.Static limits*/
#define PRV_CPSS_PORT_TX_TAIL_DROP_STATIC_PORT_LIMIT_DEFAULT  0
#define PRV_CPSS_PORT_TX_TAIL_DROP_STATIC_QUEUE_LIMIT_DEFAULT 0xfffff

#define PRV_CPSS_TD_DEFAULT_RESOURCE_MODE_MAC CPSS_PORT_TX_TAIL_DROP_RESOURCE_MODE_GLOBAL_E

/* start of register global_tail_drop_limit_high*/

#define      TXQ_AAS_PREQ_GLOBAL_TAIL_DROP_LIMIT_HIGH_GLOBAL_TAIL_DROP_LIMIT_H_FIELD_OFFSET         0


#define      TXQ_AAS_PREQ_GLOBAL_TAIL_DROP_LIMIT_HIGH_GLOBAL_TAIL_DROP_LIMIT_H_FIELD_SIZE           23

/* end of global_tail_drop_limit_high*/


/* start of register global_tail_drop_limit_low*/

#define      TXQ_AAS_PREQ_GLOBAL_TAIL_DROP_LIMIT_LOW_GLOBAL_TAIL_DROP_LIMIT_L_FIELD_OFFSET          0


#define      TXQ_AAS_PREQ_GLOBAL_TAIL_DROP_LIMIT_LOW_GLOBAL_TAIL_DROP_LIMIT_L_FIELD_SIZE            23

/* end of global_tail_drop_limit_low*/



/* start of register Global_PDX_Burst_Fifo_Limit_High*/

#define      TXQ_AAS_PREQ_GLOBAL_PDX_BURST_FIFO_LIMIT_HIGH_GLOBAL_PDX_BURST_FIFO_LIMIT_H_FIELD_OFFSET           0


#define      TXQ_AAS_PREQ_GLOBAL_PDX_BURST_FIFO_LIMIT_HIGH_GLOBAL_PDX_BURST_FIFO_LIMIT_H_FIELD_SIZE         18

/* end of Global_PDX_Burst_Fifo_Limit_High*/


/* start of register Global_PDX_Burst_Fifo_Limit_Low*/

#define      TXQ_AAS_PREQ_GLOBAL_PDX_BURST_FIFO_LIMIT_LOW_GLOBAL_PDX_BURST_FIFO_LIMIT_L_FIELD_OFFSET            0


#define      TXQ_AAS_PREQ_GLOBAL_PDX_BURST_FIFO_LIMIT_LOW_GLOBAL_PDX_BURST_FIFO_LIMIT_L_FIELD_SIZE          18

/* end of Global_PDX_Burst_Fifo_Limit_Low*/



GT_U32 prvCpssTxqUtilsPbSizeInBuffersGet
(
    IN GT_U8 devNum
)
{
    GT_U32 pbSize;
    if (GT_FALSE == PRV_CPSS_SIP_6_10_CHECK_MAC(devNum))
    {
        pbSize = PRV_CPSS_SIP_6_TD_PB_SIZE_PER_TILE_MAC;
    }
    else if (GT_FALSE == PRV_CPSS_SIP_6_15_CHECK_MAC(devNum))
    {
        pbSize = PRV_CPSS_SIP_6_10_TD_PB_SIZE_PER_TILE_MAC;
    }
    else if (GT_FALSE == PRV_CPSS_SIP_6_20_CHECK_MAC(devNum))
    {
        pbSize = PRV_CPSS_SIP_6_15_TD_PB_SIZE_PER_TILE_MAC;
    }
    else if (GT_FALSE == PRV_CPSS_SIP_6_30_CHECK_MAC(devNum))
    {
        pbSize = PRV_CPSS_SIP_6_20_TD_PB_SIZE_PER_TILE_MAC;
    }
    else if (GT_FALSE == PRV_CPSS_SIP_7_CHECK_MAC(devNum))
    {
        pbSize = PRV_CPSS_SIP_6_30_TD_PB_SIZE_PER_TILE_MAC;
    }
    else
    {
        pbSize = PRV_CPSS_SIP_7_TD_PB_SIZE_PER_TILE_MAC;
    }
    return pbSize;
}
/**
 * @internal prvCpssSip6TxqUtilsPbSizeGet function
 * @endinternal
 *
 * @brief  Get PB size.
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
 * @note   NOT APPLICABLE DEVICES:  Lion2;xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                   -                  physical device number
 * @param[out] maxPbSizePtr                   -     size of PB for device
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PARAM             - on wrong port number or device
 * @retval GT_BAD_PTR               - on NULL pointer
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
GT_STATUS  prvCpssSip6TxqUtilsPbSizeGet
(
    IN GT_U8 devNum,
    OUT GT_U32                              *maxPbSizePtr
)
{
    GT_U32 numberOfTiles, oneTileSize;
    PRV_CPSS_NUM_OF_TILE_GET_MAC(devNum, numberOfTiles);
    oneTileSize = prvCpssTxqUtilsPbSizeInBuffersGet(devNum);
    *maxPbSizePtr   = numberOfTiles * oneTileSize;
    return GT_OK;
}
/**
 * @internal prvDxChSip7PortTxGlobalPbLimitSet function
 * @endinternal
 *
 * @brief  Set Tail drop limit for global buffers usage. When the fill level exceeds the limit, packets will be dropped.
 *
 * @note   APPLICABLE DEVICES: AAS
 * @note   NOT APPLICABLE DEVICES:  Lion2;xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3;
 *                                  Aldrin2;Falcon; AC5P; AC5X; Harrier; Ironman..
 *
 * @param[in] devNum                - physical device number
 * @param[in] hiPriority            - differentiation between H/L priorities such that lower PB congestion
 *                                    results with early drops of low priority traffic.
 * @param[in] limit                 - total descriptor value
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PARAM             - on wrong port number or device
 * @retval GT_BAD_PTR               - on NULL pointer
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
GT_STATUS prvDxChSip7PortTxGlobalPbLimitSet
(
    IN  GT_U8   devNum,
    IN  GT_BOOL hiPriority,
    IN  GT_U32  limit
)
{
    GT_U32 regAddr,fieldLength,fieldOffset,pbSize;
    GT_STATUS rc;

    if(GT_TRUE==hiPriority)
    {
        regAddr = PRV_DXCH_REG1_UNIT_PREQ_MAC(devNum).global_tail_drop_limit_high;
        fieldOffset = TXQ_AAS_PREQ_GLOBAL_TAIL_DROP_LIMIT_HIGH_GLOBAL_TAIL_DROP_LIMIT_H_FIELD_OFFSET;
        fieldLength = TXQ_AAS_PREQ_GLOBAL_TAIL_DROP_LIMIT_HIGH_GLOBAL_TAIL_DROP_LIMIT_H_FIELD_SIZE;
    }
    else
    {
        regAddr = PRV_DXCH_REG1_UNIT_PREQ_MAC(devNum).global_tail_drop_limit_low;
        fieldOffset = TXQ_AAS_PREQ_GLOBAL_TAIL_DROP_LIMIT_LOW_GLOBAL_TAIL_DROP_LIMIT_L_FIELD_OFFSET;
        fieldLength = TXQ_AAS_PREQ_GLOBAL_TAIL_DROP_LIMIT_LOW_GLOBAL_TAIL_DROP_LIMIT_L_FIELD_SIZE;
    }

    if(limit>=(GT_U32)(1 << fieldLength))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssSip6TxqUtilsPbSizeGet(devNum,&pbSize);
    if(rc!=GT_OK)
    {
       CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    if(limit >pbSize)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "limit [%d]is out of range[%d].\n",
            limit,pbSize);
    }

    rc = prvCpssDrvHwPpSetRegField(devNum, regAddr, fieldOffset, fieldLength, limit);

    return rc;
}

/**
 * @internal prvDxChSip7PortTxGlobalPbLimitGet function
 * @endinternal
 *
 * @brief  Get Tail drop limit for global buffers usage. When the fill level exceeds the limit, packets will be dropped.
 *
 * @note   APPLICABLE DEVICES: AAS
 * @note   NOT APPLICABLE DEVICES:  Lion2;xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3;
 *                                  Aldrin2;Falcon; AC5P; AC5X; Harrier; Ironman..
 *
 * @param[in] devNum                - physical device number
 * @param[in] hiPriority            - differentiation between H/L priorities such that lower PB congestion
 *                                    results with early drops of low priority traffic.
 * @param[out] limitPtr                 - (pointer to) total descriptor value
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PARAM             - on wrong port number or device
 * @retval GT_BAD_PTR               - on NULL pointer
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
GT_STATUS prvDxChSip7PortTxGlobalPbLimitGet
(
    IN  GT_U8   devNum,
    IN  GT_BOOL hiPriority,
    OUT  GT_U32  *limitPtr
)
{
    GT_U32 regAddr,fieldLength,fieldOffset;
    GT_STATUS rc;

    CPSS_NULL_PTR_CHECK_MAC(limitPtr);

    if(GT_TRUE==hiPriority)
    {
        regAddr = PRV_DXCH_REG1_UNIT_PREQ_MAC(devNum).global_tail_drop_limit_high;
        fieldOffset = TXQ_AAS_PREQ_GLOBAL_TAIL_DROP_LIMIT_HIGH_GLOBAL_TAIL_DROP_LIMIT_H_FIELD_OFFSET;
        fieldLength = TXQ_AAS_PREQ_GLOBAL_TAIL_DROP_LIMIT_HIGH_GLOBAL_TAIL_DROP_LIMIT_H_FIELD_SIZE;
    }
    else
    {
        regAddr = PRV_DXCH_REG1_UNIT_PREQ_MAC(devNum).global_tail_drop_limit_low;
        fieldOffset = TXQ_AAS_PREQ_GLOBAL_TAIL_DROP_LIMIT_LOW_GLOBAL_TAIL_DROP_LIMIT_L_FIELD_OFFSET;
        fieldLength = TXQ_AAS_PREQ_GLOBAL_TAIL_DROP_LIMIT_LOW_GLOBAL_TAIL_DROP_LIMIT_L_FIELD_SIZE;
    }


    rc = prvCpssDrvHwPpGetRegField(devNum, regAddr, fieldOffset, fieldLength, limitPtr);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDrvHwPpGetRegField failed \n");
    }

    return rc;
}


/**
 * @internal prvDxChSip7PortTxGlobalPdxMinimalAvailebleLimitSet function
 * @endinternal
 *
 * @brief  Set minimal number of availeble PDX buffers .In case number of free buffers is less then packet is dropped
 * @note   APPLICABLE DEVICES: AAS
 * @note   NOT APPLICABLE DEVICES:  Lion2;xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3;
 *                                  Aldrin2;Falcon; AC5P; AC5X; Harrier; Ironman..
 *
 * @param[in] devNum                - physical device number
 * @param[in] hiPriority            - differentiation between H/L priorities such that lower PB congestion
 *                                    results with early drops of low priority traffic.
 * @param[in] limit                 - low limit of free  buffers
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PARAM             - on wrong port number or device
 * @retval GT_BAD_PTR               - on NULL pointer
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
GT_STATUS prvDxChSip7PortTxGlobalPdxMinimalAvailebleLimitSet
(
    IN  GT_U8   devNum,
    IN  GT_BOOL hiPriority,
    IN  GT_U32  limit
)
{
    GT_U32 regAddr,fieldLength,fieldOffset,pbSize;
    GT_STATUS rc;

    if(GT_TRUE==hiPriority)
    {
        regAddr = PRV_DXCH_REG1_UNIT_PREQ_MAC(devNum).Global_PDX_Burst_Fifo_Limit_High;
        fieldOffset = TXQ_AAS_PREQ_GLOBAL_PDX_BURST_FIFO_LIMIT_HIGH_GLOBAL_PDX_BURST_FIFO_LIMIT_H_FIELD_OFFSET;
        fieldLength = TXQ_AAS_PREQ_GLOBAL_PDX_BURST_FIFO_LIMIT_HIGH_GLOBAL_PDX_BURST_FIFO_LIMIT_H_FIELD_SIZE;
    }
    else
    {
        regAddr = PRV_DXCH_REG1_UNIT_PREQ_MAC(devNum).Global_PDX_Burst_Fifo_Limit_Low;
        fieldOffset = TXQ_AAS_PREQ_GLOBAL_PDX_BURST_FIFO_LIMIT_LOW_GLOBAL_PDX_BURST_FIFO_LIMIT_L_FIELD_OFFSET;
        fieldLength = TXQ_AAS_PREQ_GLOBAL_PDX_BURST_FIFO_LIMIT_LOW_GLOBAL_PDX_BURST_FIFO_LIMIT_L_FIELD_SIZE;
    }

    if(limit>=(GT_U32)(1<<fieldLength))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, LOG_ERROR_NO_MSG);
    }

    rc = prvCpssSip6TxqUtilsPbSizeGet(devNum,&pbSize);
    if(rc!=GT_OK)
    {
       CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    if(limit >pbSize)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "limit [%d]is out of range[%d].\n",
            limit,pbSize);
    }

    rc = prvCpssDrvHwPpSetRegField(devNum, regAddr, fieldOffset, fieldLength, limit);

    return rc;
}
/**
 * @internal prvDxChSip7PortTxGlobalPdxMinimalAvailebleLimitGet function
 * @endinternal
 *
 * @brief  Get minimal number of availeble PDX buffers .In case number of free buffers is less then packet is dropped
 *
 * @note   APPLICABLE DEVICES: AAS
 * @note   NOT APPLICABLE DEVICES:  Lion2;xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3;
 *                                  Aldrin2;Falcon; AC5P; AC5X; Harrier; Ironman..
 *
 * @param[in] devNum                - physical device number
 * @param[in] hiPriority            - differentiation between H/L priorities such that lower PB congestion
 *                                    results with early drops of low priority traffic.
 * @param[out] limitPtr                 - (pointer to) low limit of free  buffers
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PARAM             - on wrong port number or device
 * @retval GT_BAD_PTR               - on NULL pointer
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */

GT_STATUS prvDxChSip7PortTxGlobalPdxMinimalAvailebleLimitGet
(
    IN  GT_U8   devNum,
    IN  GT_BOOL hiPriority,
    IN  GT_U32  *limitPtr
)
{
    GT_U32 regAddr,fieldLength,fieldOffset;
    GT_STATUS rc;

    CPSS_NULL_PTR_CHECK_MAC(limitPtr);

    if(GT_TRUE==hiPriority)
    {
        regAddr = PRV_DXCH_REG1_UNIT_PREQ_MAC(devNum).Global_PDX_Burst_Fifo_Limit_High;
        fieldOffset = TXQ_AAS_PREQ_GLOBAL_PDX_BURST_FIFO_LIMIT_HIGH_GLOBAL_PDX_BURST_FIFO_LIMIT_H_FIELD_OFFSET;
        fieldLength = TXQ_AAS_PREQ_GLOBAL_PDX_BURST_FIFO_LIMIT_HIGH_GLOBAL_PDX_BURST_FIFO_LIMIT_H_FIELD_SIZE;
    }
    else
    {
        regAddr = PRV_DXCH_REG1_UNIT_PREQ_MAC(devNum).Global_PDX_Burst_Fifo_Limit_Low;
        fieldOffset = TXQ_AAS_PREQ_GLOBAL_PDX_BURST_FIFO_LIMIT_LOW_GLOBAL_PDX_BURST_FIFO_LIMIT_L_FIELD_OFFSET;
        fieldLength = TXQ_AAS_PREQ_GLOBAL_PDX_BURST_FIFO_LIMIT_LOW_GLOBAL_PDX_BURST_FIFO_LIMIT_L_FIELD_SIZE;
    }


    rc = prvCpssDrvHwPpGetRegField(devNum, regAddr, fieldOffset, fieldLength, limitPtr);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssDrvHwPpGetRegField failed \n");
    }

    return rc;
}



/**
 * @internal prvCpssFalconTxqUtilsITx4TcPdxTailDropProfileSet function
 * @endinternal
 *
 * @brief  Set profile PDX TD  parameters.This is interbnal configuration.
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
 * @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                   -         `                          physical device number
 * @param[in] profileSet               - the Profile Set in which the Traffic
 *                                      Class Drop Parameters is associated.
 * @param[in] trafficClass             - the Traffic Class associated with this set of
 *                                      Drop Parameters (0..7).
 * @param[in] guaranteedLimit - The guaranteed amount of buffers
 * @param[in] alfa                            The ratio of the free buffers that can be used for the queue
 *
 * @retval GT_OK                    -           on success
 * @retval GT_BAD_PARAM             - on wrong port number or device
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 *
 *
 */
static GT_STATUS prvCpssFalconTxqUtilsITx4TcPdxTailDropProfileSet
(
    IN GT_U8 devNum,
    IN CPSS_PORT_TX_DROP_PROFILE_SET_ENT profileSet,
    IN GT_U8 trafficClass,
    IN GT_U32 guaranteedLimit,
    IN CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_ENT alfa
)
{
    GT_U32      regData = 0;
    GT_STATUS   rc;
    GT_U32      tableIndex, i;
    PRV_CPSS_PORT_TX_TAIL_DROP_SIP6_ALPHA_TO_HW_CONVERT_MAC(alfa, regData);
    TXQ_SIP_6_CHECK_FIELD_LENGTH_VALID_MAC(guaranteedLimit, 16);

    if (GT_FALSE == PRV_CPSS_SIP_7_CHECK_MAC(devNum))
    {

        tableIndex = (profileSet << 3) + trafficClass;

        /*set same value for all DP*/
        for (i = 0; i < 3; i++, tableIndex += 256)
        {
            rc = prvCpssDxChWriteTableEntryField(devNum,
                                 CPSS_DXCH_SIP6_TABLE_PREQ_QUEUE_DP_CONFIGURATIONS_E,
                                 tableIndex,
                                 PRV_CPSS_DXCH_TABLE_WORD_INDICATE_GLOBAL_BIT_CNS, 68, 20,
                                 guaranteedLimit << 4 | regData);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "Updating TABLE_PREQ_QUEUE_CONFIGURATIONS  failed at index %d\n", tableIndex);
            }
        }
    }
    else
    {
        tableIndex = profileSet;

        rc = prvCpssDxChWriteTableEntryField(devNum,
                                 CPSS_DXCH_SIP6_TABLE_PREQ_QUEUE_CONFIGURATIONS_E,
                                 tableIndex,
                                 PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                 SIP7_PREQ_Q_PROFILES_TABLE_FIELDS_PDX_BURST_FIFO_GUARANTEED_LIMIT_E,
                                 PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                 guaranteedLimit);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "Updating TABLE_PREQ_QUEUE_CONFIGURATIONS  failed at index %d\n", tableIndex);
        }

        rc = prvCpssDxChWriteTableEntryField(devNum,
                                 CPSS_DXCH_SIP6_TABLE_PREQ_QUEUE_CONFIGURATIONS_E,
                                 tableIndex,
                                 PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                 SIP7_PREQ_Q_PROFILES_TABLE_FIELDS_PDX_BURST_FIFO_DYNAMIC_FACTOR_E,
                                 PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                 regData);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "Updating TABLE_PREQ_QUEUE_CONFIGURATIONS  failed at index %d\n", tableIndex);
        }

    }
    return GT_OK;
}

/**
 * @internal prvCpssFalconTxqUtilsInitTailDrop function
 * @endinternal
 *
 * @brief  Initialize PREQ tail drop parameters to default.
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P; AC5X; Harrier; Ironman.
 * @note   NOT APPLICABLE DEVICES:   xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                   -         `                          physical device number
 * @param[in] configureMulticast                   -         `        if equal GT_TRUE then default multicast threshold configured ,
 *                                                                                               else no multicast default limits configured
 *
 * @retval GT_OK                    -           on success
 * @retval GT_BAD_PARAM             - on wrong port number or device
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 *
 *
 */
GT_STATUS prvCpssFalconTxqUtilsInitTailDrop
(
    IN GT_U8 devNum,
    IN GT_BOOL configureMulticast
)
{
    GT_U32                      numberOfTiles,numberOfPools;
    GT_STATUS                   rc;
    CPSS_PORT_TX_Q_TAIL_DROP_WRED_TC_DP_PROF_PARAMS_STC tailDropWredProfileParams;
    CPSS_PORT_TX_DROP_PROFILE_SET_ENT       profileSet;
    GT_U8                       trafficClass;
    CPSS_PORT_TX_Q_TAIL_DROP_PROF_TC_PARAMS_STC     tailDropProfileParams;
    CPSS_PORT_TAIL_DROP_WRED_PARAMS_STC     portTailDropProfileParams;
    GT_U32                      pbTileSizeInBuffers = prvCpssTxqUtilsPbSizeInBuffersGet(devNum);
    GT_U32                      upperBound;/*maximal value equal to PB size*/
    GT_U32                      i,regAddr,fieldSize;
    CPSS_PORT_TX_DROP_PROFILE_SET_ENT   lastPortProfileToUpdate = CPSS_PORT_TX_DROP_PROFILE_16_E;

    cpssOsMemSet(&portTailDropProfileParams,0,sizeof(portTailDropProfileParams));
    cpssOsMemSet(&tailDropWredProfileParams,0,sizeof(tailDropWredProfileParams));
    cpssOsMemSet(&tailDropProfileParams,0,sizeof(tailDropProfileParams));

    switch (PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles)
    {
    case 0:
    case 1:
        numberOfTiles = 1;
        break;
    case 2:
    case 4:
        numberOfTiles = PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles;
        break;
    default:
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "unexpected number of tiles - %d ", PRV_CPSS_PP_MAC(devNum)->multiPipe.numOfTiles);
        break;
    }
    if (GT_FALSE == PRV_CPSS_SIP_6_15_CHECK_MAC(devNum))
    {
        upperBound = (GT_U32)(numberOfTiles * pbTileSizeInBuffers * PRV_CPSS_TD_AVAILABLE_BUF_FACTOR_MAC);
    }
    else
    {
        upperBound = (GT_U32)(numberOfTiles * pbTileSizeInBuffers * PRV_CPSS_SIP_6_15_TD_AVAILABLE_BUF_FACTOR_MAC);
    }
    rc = cpssDxChPortTxTailDropGlobalParamsSet(devNum, PRV_CPSS_TD_DEFAULT_RESOURCE_MODE_MAC,
                           upperBound,          /*globalAvailableBuffers*/
                           upperBound,          /*pool0AvailableBuffers*/
                           upperBound,          /*pool1AvailableBuffers*/
                           upperBound,          /*pool2AvailableBuffers*/
                           upperBound           /*pool3AvailableBuffers*/
                            );
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "cpssDxChPortTxTailDropGlobalParamsSet failed ");
    }

    if (GT_TRUE == PRV_CPSS_SIP_7_CHECK_MAC(devNum))
    {
        numberOfPools = SIP7_SHARED_TAIL_DROP_POOLS_NUM_CNS;
        fieldSize = 28;
    }
    else
    {
        numberOfPools = SIP6_SHARED_TAIL_DROP_POOLS_NUM_CNS;
        fieldSize = 20;
    }
    for(i=0;i<numberOfPools;i++)
    {

        regAddr = PRV_DXCH_REG1_UNIT_PREQ_MAC(devNum).poolTailDropLimit[i];

        rc = prvCpssHwPpSetRegField(devNum, regAddr, 0, fieldSize, i==0?(2*(GT_U32)(numberOfTiles * pbTileSizeInBuffers)):0);
        if(rc!=GT_OK)
        {
          CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }

    }


    if (configureMulticast == GT_TRUE)
    {
        rc = cpssDxChPortTxMcastAvailableBuffersSet(devNum, upperBound);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "cpssDxChPortTxMcastAvailableBuffersSet failed ");
        }
        rc = cpssDxChPortTxMcastBuffersPriorityLimitSet(devNum, CPSS_PORT_TX_DROP_MCAST_PRIORITY_LOW_E,
                                PRV_CPSS_PORT_TX_TAIL_DROP_DBA_DEFAULT_MC_LOW_GURANTEED_LIMIT,
                                PRV_CPSS_PORT_TX_TAIL_DROP_DBA_DEFAULT_MC_LOW_ALPHA);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "cpssDxChPortTxGlobalDescLimitSet failed ");
        }
        rc = cpssDxChPortTxMcastBuffersPriorityLimitSet(devNum, CPSS_PORT_TX_DROP_MCAST_PRIORITY_HI_E,
                                PRV_CPSS_PORT_TX_TAIL_DROP_DBA_DEFAULT_MC_HI_GURANTEED_LIMIT,
                                PRV_CPSS_PORT_TX_TAIL_DROP_DBA_DEFAULT_MC_HI_ALPHA);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "cpssDxChPortTxGlobalDescLimitSet failed ");
        }
    }

    rc = cpssDxChPortTxGlobalDescLimitSet(devNum, upperBound);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "cpssDxChPortTxGlobalDescLimitSet failed ");
    }


    if (GT_TRUE == PRV_CPSS_SIP_7_CHECK_MAC(devNum))
    {   /*PB Low Priority*/
        rc = prvDxChSip7PortTxGlobalPbLimitSet(devNum, GT_FALSE,(GT_U32)(upperBound*PRV_CPSS_SIP7_TD_LP_FACTOR_MAC));
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "cpssDxChPortTxGlobalDescLimitSet failed ");
        }
    }
    /*port*/
    portTailDropProfileParams.guaranteedLimit   = PRV_CPSS_PORT_TX_TAIL_DROP_DBA_DEFAULT_PORT_GURANTEED_LIMIT;
    portTailDropProfileParams.probability       = PRV_CPSS_PORT_TX_TAIL_DROP_WRED_DEFAULT_PORT_DROP_PROB;
    portTailDropProfileParams.wredSize      = PRV_CPSS_PORT_TX_TAIL_DROP_WRED_DEFAULT_PORT_WRED_SIZE;
    /*queue*/
    tailDropWredProfileParams.dp0WredAttributes.guaranteedLimit = PRV_CPSS_PORT_TX_TAIL_DROP_DBA_DEFAULT_QUEUE_DP_GURANTEED_LIMIT;
    tailDropWredProfileParams.dp0WredAttributes.probability     = PRV_CPSS_PORT_TX_TAIL_DROP_WRED_DEFAULT_QUEUE_DP_DROP_PROB;
    tailDropWredProfileParams.dp0WredAttributes.wredSize        = PRV_CPSS_PORT_TX_TAIL_DROP_WRED_DEFAULT_QUEUE_DP_WRED_SIZE;
    tailDropWredProfileParams.dp1WredAttributes         = tailDropWredProfileParams.dp0WredAttributes;
    tailDropWredProfileParams.dp2WredAttributes         = tailDropWredProfileParams.dp0WredAttributes;
    tailDropProfileParams.tcMaxBuffNum              =  PRV_CPSS_PORT_TX_TAIL_DROP_STATIC_QUEUE_LIMIT_DEFAULT;
    tailDropProfileParams.dp0QueueAlpha             = PRV_CPSS_PORT_TX_TAIL_DROP_DBA_DEFAULT_QUEUE_DP_ALPHA;
    tailDropProfileParams.dp1QueueAlpha             = tailDropProfileParams.dp0QueueAlpha;
    tailDropProfileParams.dp2QueueAlpha             = tailDropProfileParams.dp0QueueAlpha;
    if (GT_TRUE == PRV_CPSS_SIP_7_CHECK_MAC(devNum))
    {
        tailDropProfileParams.poolLimitEnable           = GT_TRUE;
    }
    else
    {   /*not relevant*/
        tailDropProfileParams.poolLimitEnable           = GT_FALSE;
    }
    if (GT_TRUE == PRV_CPSS_SIP_7_CHECK_MAC(devNum))
    {
        /*init configuration of profile-1 ( according to device) which is used by default as hardware.*/
        lastPortProfileToUpdate = CPSS_PORT_TX_DROP_PROFILE_1_E;
    }

    for (profileSet = CPSS_PORT_TX_DROP_PROFILE_1_E;
         profileSet <= lastPortProfileToUpdate; profileSet++)
    {
        /* OAM UP-MEP WA: if the tail-drop profile is reserved by the WA, skip it. */
        if (GT_TRUE == PRV_CPSS_DXCH_PP_MAC(devNum)->port.oamUpMepGlobalWa.oamUpMepWaEnable &&
            PRV_CPSS_DXCH_PP_MAC(devNum)->port.oamUpMepGlobalWa.oamUpMepWaReservedTailDropProfile == profileSet)
        {
            continue;
        }

        rc = cpssDxChPortTxTailDropWredProfileSet(devNum, profileSet, &(portTailDropProfileParams));
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "cpssDxChPortTxTailDropWredProfileSet failed ");
        }
        rc = cpssDxChPortTxTailDropProfileSet(devNum, profileSet,
                              PRV_CPSS_PORT_TX_TAIL_DROP_DBA_DEFAULT_PORT_ALPHA,
                              PRV_CPSS_PORT_TX_TAIL_DROP_STATIC_PORT_LIMIT_DEFAULT, 0 /*dont care*/);
        if (rc != GT_OK)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "cpssDxChPortTxTailDropProfileSet failed ");
        }
        for (trafficClass = 0; trafficClass < CPSS_16_TC_RANGE_CNS; trafficClass++)
        {
            rc = cpssDxChPortTx4TcTailDropWredProfileSet(devNum, profileSet, trafficClass, &tailDropWredProfileParams);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "cpssDxChPortTxTailDropWredProfileSet failed ");
            }
            rc = cpssDxChPortTx4TcTailDropProfileSet(devNum, profileSet, trafficClass, &tailDropProfileParams);
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "cpssDxChPortTxTailDropWredProfileSet failed ");
            }


            /*set internal PDX thresholds*/
            rc = prvCpssFalconTxqUtilsITx4TcPdxTailDropProfileSet(devNum, profileSet, trafficClass,
                                          PRV_CPSS_PORT_TX_TAIL_DROP_DBA_DEFAULT_PDX_GURANTEED_LIMIT(devNum),
                                          PRV_CPSS_PORT_TX_TAIL_DROP_DBA_DEFAULT_PDX_ALPHA(devNum));
            if (rc != GT_OK)
            {
                CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssFalconTxqUtilsITx4TcPdxTailDropProfileSet failed ");
            }
            if (GT_TRUE == PRV_CPSS_SIP_7_CHECK_MAC(devNum))
            {
                /*No trafficClass differentiation for SIP7*/
                break;
            }
        }
    }

     /*The 8b queue_offset logic added in Harrier shall  be removed as it is no longer needed in AAS .
        In AAS the queue_offset returns to 4b*/
     if (GT_FALSE == PRV_CPSS_SIP_7_CHECK_MAC(devNum))
     {
    /*initialize  queueOffsetProfileMapping to circular mapping
         0-15    to 0-15,
         16-31  to 0-15, ........*/
        if (GT_TRUE == PRV_CPSS_SIP_6_20_CHECK_MAC(devNum))
        {
            for(i=0;i<256;i++)
            {
                rc = cpssDxChPortTxQueueOffsetMapSet(devNum,i,i&0xF);
                if (rc != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssFalconTxqUtilsITx4TcPdxTailDropProfileSet failed ");
                }
            }
        }
    }
    return GT_OK;
}



/**
 * @internal prvCpssSip6TxQUtilsPoolValidateErrataConstraint function
 * @endinternal
 *
 * @brief  Validates that pool limit is within range and autoconfigure other pool if required
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P;AC5X;Harrier; Ironman.
 * @note   NOT APPLICABLE DEVICES:  Lion2;xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                                       -physical device number
 * @param[in] poolNum                                          index of the pool
  * @param[in] limit                                               pool limit
  * @param[in] autoComputeOtherPool               - The second shared pool size is automaticly configured to be   2PB size -[poolNum] size
*                                      NOT APPLICABLE DEVICES:
*                                      Lion2, Bobcat2, Caelum, Aldrin, AC3X; Bobcat3; Aldrin2;Ironman
*                                      APPLICABLE DEVICES:Falcon; AC5P; AC5X; Harrier;
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PARAM             - on wrong port number or device
 * @retval GT_BAD_PTR               - on NULL pointer
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
GT_STATUS prvCpssSip6TxQUtilsPoolValidateErrataConstraint
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      poolNum,
    IN  GT_U32                      limit,
    IN  GT_BOOL                     autoComputeOtherPool
)
{
    GT_STATUS rc;
    GT_U32    pbSize,maxVal,regAddr;
    GT_U32    otherPoolSize;

    if(poolNum>=SIP6_SHARED_TOTAL_POOLS_NUM_CNS(devNum))
    {
       CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "Pool index %d too big", poolNum);
    }

    rc = prvCpssSip6TxqUtilsPbSizeGet(devNum,&pbSize);
    if(rc!=GT_OK)
    {
       CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
    }

    /*due to multicast maximal value is twice PB size since counted in virtual buffers.*/

    maxVal=((2*pbSize) < BIT_20)?(2*pbSize):(BIT_20 - 1);

    if( limit > maxVal)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "Pool limit  %d too big should be smaller (or equal) than %d", limit,maxVal);
    }

    /*get the other*/
    if(poolNum == 1)
    {
        regAddr = PRV_DXCH_REG1_UNIT_PREQ_MAC(devNum).poolTailDropLimit[0];
    }
    else
    {
        regAddr = PRV_DXCH_REG1_UNIT_PREQ_MAC(devNum).poolTailDropLimit[1];
    }

    if(GT_TRUE==autoComputeOtherPool)
    {
          /*When the application uses other pool , reserved pool   =  2*PB -other pool */


         if(limit>2*pbSize)
         {
            limit = 2*pbSize;
         }

         rc = prvCpssHwPpSetRegField(devNum, regAddr, 0, 20, 2*pbSize - limit);

         if(rc!=GT_OK)
         {
           CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
         }

    }
    else
    {
        /*check that Pool0+Pool1 < 2PB size*/

        rc = prvCpssHwPpGetRegField(devNum, regAddr, 0, 20, &otherPoolSize);
        if(rc!=GT_OK)
        {
          CPSS_LOG_ERROR_AND_RETURN_MAC(rc, LOG_ERROR_NO_MSG);
        }

        if(otherPoolSize+limit>maxVal)
        {
          CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "Limit [%d] is too big,must be smaller then %d\n",limit,maxVal- otherPoolSize);
        }
    }

    return GT_OK;
}

/**
 * @internal prvCpssSip6TxqUtilsTailDropCreateStaticProfile function
 * @endinternal
 *
 * @brief  Configure tail drop profile to static mode(no DBA)
 *
 * @note   APPLICABLE DEVICES:      Falcon; AC5P;AC5X;Harrier; Ironman.
 * @note   NOT APPLICABLE DEVICES:  Lion2;xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2.
 *
 * @param[in] devNum                                       -physical device number
 * @param[in] profileSet               - the Profile Set in which the Traffic
 *                                      Class Drop Parameters is associated.
 * @param[in] guaranteedLimit - The guaranteed amount of buffers.Also configured as max limit.
 *
 * @retval GT_OK                    - on success
 * @retval GT_BAD_PARAM             - on wrong port number or device
 * @retval GT_BAD_PTR               - on NULL pointer
 * @retval GT_HW_ERROR              - on hardware error
 * @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
 */
GT_STATUS  prvCpssSip6TxqUtilsTailDropCreateStaticProfile
(
    IN GT_U8 devNum,
    IN CPSS_PORT_TX_DROP_PROFILE_SET_ENT     profileSet,
    IN GT_U32                                guaranteedLimit
)
{
     GT_STATUS rc;
     CPSS_PORT_TAIL_DROP_WRED_PARAMS_STC portTailDropProfileParams;
     CPSS_PORT_TX_Q_TAIL_DROP_WRED_TC_DP_PROF_PARAMS_STC tailDropWredProfileParams;
     CPSS_PORT_TX_Q_TAIL_DROP_PROF_TC_PARAMS_STC         tailDropProfileParams;
     GT_U32 trafficClass;

     PRV_CPSS_DXCH_DROP_PROFILE_CHECK_MAC(profileSet, devNum);

      /*port*/

     rc = cpssDxChPortTxTailDropWredProfileGet(devNum, profileSet, &(portTailDropProfileParams));
     if (rc != GT_OK)
     {
         CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "cpssDxChPortTxTailDropWredProfileSet failed ");
     }

     portTailDropProfileParams.guaranteedLimit = guaranteedLimit;

     rc = cpssDxChPortTxTailDropWredProfileSet(devNum, profileSet, &(portTailDropProfileParams));
     if (rc != GT_OK)
     {
         CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "cpssDxChPortTxTailDropWredProfileSet failed ");
     }

     rc = cpssDxChPortTxTailDropProfileSet(devNum, profileSet,
                              CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_E,
                              guaranteedLimit, 0 /*dont care*/);
     if (rc != GT_OK)
     {
         CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "cpssDxChPortTxTailDropProfileSet failed ");
     }

     /*queue*/

     for (trafficClass = 0; trafficClass < CPSS_16_TC_RANGE_CNS; trafficClass++)
     {
         rc = cpssDxChPortTx4TcTailDropWredProfileGet(devNum, profileSet, trafficClass, &tailDropWredProfileParams);
         if (rc != GT_OK)
         {
             CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "cpssDxChPortTxTailDropWredProfileSet failed ");
         }

         tailDropWredProfileParams.dp0WredAttributes.guaranteedLimit = guaranteedLimit;
         tailDropWredProfileParams.dp1WredAttributes.guaranteedLimit = guaranteedLimit;
         tailDropWredProfileParams.dp2WredAttributes.guaranteedLimit = guaranteedLimit;

         rc = cpssDxChPortTx4TcTailDropWredProfileSet(devNum, profileSet, trafficClass, &tailDropWredProfileParams);
         if (rc != GT_OK)
         {
             CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "cpssDxChPortTxTailDropWredProfileSet failed ");
         }

          rc = cpssDxChPortTx4TcTailDropProfileGet(devNum, profileSet, trafficClass, &tailDropProfileParams);
         if (rc != GT_OK)
         {
             CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "cpssDxChPortTxTailDropWredProfileSet failed ");
         }

         tailDropProfileParams.tcMaxBuffNum              =  guaranteedLimit;
         tailDropProfileParams.dp0QueueAlpha             = CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_E;
         tailDropProfileParams.dp1QueueAlpha             = tailDropProfileParams.dp0QueueAlpha;
         tailDropProfileParams.dp2QueueAlpha             = tailDropProfileParams.dp0QueueAlpha;

         rc = cpssDxChPortTx4TcTailDropProfileSet(devNum, profileSet, trafficClass, &tailDropProfileParams);
         if (rc != GT_OK)
         {
             CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "cpssDxChPortTxTailDropWredProfileSet failed ");
         }
         /*set internal PDX  to max*/
         rc = prvCpssFalconTxqUtilsITx4TcPdxTailDropProfileSet(devNum, profileSet, trafficClass,
                                       0xFFFF,
                                       CPSS_PORT_TX_TAIL_DROP_DBA_ALPHA_0_E);
         if (rc != GT_OK)
         {
             CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "prvCpssFalconTxqUtilsITx4TcPdxTailDropProfileSet failed ");
         }
     }

    return GT_OK;

}

GT_STATUS  prvCpssSip7TxqUtilsTailDropBindQueueToProfileSet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  GT_U32                              queueOffset,
    IN  GT_U32                              profileSet
)
{
    GT_STATUS rc;
    GT_U32    globalQIndex,queueGroupIndex;

    /*Find queue group index*/
    rc = prvCpssDxChPortMappingEGFTargetLocalPhys2TxQPortBaseMapGet(devNum,0/*unused*/,portNum,&queueGroupIndex);
    if (GT_OK != rc )
    {
        return rc;
    }

    globalQIndex = queueGroupIndex+queueOffset;

    rc = prvCpssDxChWriteTableEntryField(devNum,
                                        CPSS_DXCH_SIP7_TABLE_HA_QUEUE_MAPPING_E,
                                        globalQIndex,
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                        SIP7_HA_QUEUE_MAPPING_TABLE_FIELDS_TAIL_DROP_PROFILE_E,
                                        PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                        profileSet);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "Updating CPSS_DXCH_SIP7_TABLE_HA_QUEUE_MAPPING_E  failed at index %d\n", globalQIndex);
    }

    return rc;
}


GT_STATUS  prvCpssSip7TxqUtilsTailDropBindQueueToProfileGet
(
    IN  GT_U8                               devNum,
    IN  GT_PHYSICAL_PORT_NUM                portNum,
    IN  GT_U32                              queueOffset,
    IN  CPSS_PORT_TX_DROP_PROFILE_SET_ENT   *profileSetPtr
)
{
    GT_STATUS rc;
    GT_U32    globalQIndex,queueGroupIndex;

    /*Find queue group index*/
    rc = prvCpssDxChPortMappingEGFTargetLocalPhys2TxQPortBaseMapGet(devNum,0/*unused*/,portNum,&queueGroupIndex);
    if (GT_OK != rc )
    {
        return rc;
    }

    globalQIndex = queueGroupIndex+queueOffset;


    rc = prvCpssDxChReadTableEntryField(devNum,
                                        CPSS_DXCH_SIP7_TABLE_HA_QUEUE_MAPPING_E,
                                        globalQIndex,
                                        PRV_CPSS_DXCH_TABLE_WORD_INDICATE_FIELD_NAME_CNS,
                                        SIP7_HA_QUEUE_MAPPING_TABLE_FIELDS_TAIL_DROP_PROFILE_E,
                                        PRV_CPSS_DXCH_TABLES_WORD_INDICATE_AUTO_CALC_LENGTH_CNS,
                                        profileSetPtr);
    if (rc != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "Reading  CPSS_DXCH_SIP7_TABLE_HA_QUEUE_MAPPING_E  failed at index %d\n", globalQIndex);
    }

    return rc;
}


