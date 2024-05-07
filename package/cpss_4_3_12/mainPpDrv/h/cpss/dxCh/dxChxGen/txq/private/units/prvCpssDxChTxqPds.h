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
* @file prvCpssDxChTxqPds.h
*
* @brief CPSS SIP6 TXQ PDS low level configurations.
*
* @version   1
********************************************************************************
*/


#ifndef __prvCpssDxChTxqPds
#define __prvCpssDxChTxqPds

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/dxCh/dxChxGen/port/cpssDxChPortTx.h>


/**
* @struct PRV_CPSS_DXCH_SIP6_TXQ_PDS_PER_QUEUE_COUNTER_STC
 *
 * @brief Descriptor counter at PDS per queue.
 *  Total_queue_counter = Tail Counter + Head counter + (Frag Counter*16) - (15-FIdx);
*/
typedef struct
{
        /** @brief Tail counter indicates the tail size (in descriptor resolution) of a long queue.
        *This is relevant only when the queue is long
     */

    GT_U32     tailCounter;

    /** @brief Frag counter indicates the fragment size of long queue (in frgaments resolution).
    *This is relevant only when the queue is long
    */

    GT_U32     fragCounter;

     /** @brief Head counter indicates the head size (in descriptor resolution) of a long queue.
     *   When the queue is short, it indicates the total queue length
    */

    GT_U32     headCounter;

      /** @brief Fragment Index. Indicating the descriptor index within the 128B PB cell in case of noDeAlloc read from the PB.
     *This is relevant only when the queue is long
    */
    GT_U32     fragIndex;

     /** @brief  Queue is Long
      * GT_FALSE = short; short; queue is short
      * GT_TRUE  = long; long; queue is long
     */
    GT_BOOL longQueue;
} PRV_CPSS_DXCH_SIP6_TXQ_PDS_PER_QUEUE_COUNTER_STC;


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
);

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
);

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
);

/**
* @internal prvCpssDxChTxqFalconPdsProfileSet function
* @endinternal
*
* @brief   Profile :
*         Profile_%p_Long_Queue_Enable - Enables writing a queue to the PB when it is getting long
*         Profile_%p_Long_Queue_Limit [0..8K] - When the total queue counter reaches this limit, a long queue is opened
*         Profile_%p_Head_Empty_Limit [0..8K] - When the head counter is below this limit,
*         the head is considered empty and fragment read from PB is triggered
*         Length_Adjust_Constant_byte_count_Profile_%p_ [0..64] -
*         Sets the value which will be decremented or incremented from the packet's byte count
*         Length_Adjust_Sub_Profile_%p_ - Subtract enable bit - for profile <%p>
*         0x0 = ADD; ADD; When ADD, the value of constant byte count field is added to the descriptor byte count
*         0x1 = SUB; SUB; When SUB, the value of constant byte count field is subtracted from the descriptor byte count
*         Length_Adjust_Enable_profile_%p -
*         RW 0x0
*         Enable the length adjust
*         0x0 = Disable Length Adjust; Disable_Length_Adjust
*         0x1 = Enable Length Adjust; Enable_Length_Adjust
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
);

/**
* @internal prvCpssDxChTxqFalconPdsProfileGet function
* @endinternal
*
* @brief   Profile :
*         Profile_%p_Long_Queue_Enable - Enables writing a queue to the PB when it is getting long
*         Profile_%p_Long_Queue_Limit [0..8K] - When the total queue counter reaches this limit, a long queue is opened
*         Profile_%p_Head_Empty_Limit [0..8K] - When the head counter is below this limit,
*         the head is considered empty and fragment read from PB is triggered
*         Length_Adjust_Constant_byte_count_Profile_%p_ [0..64] -
*         Sets the value which will be decremented or incremented from the packet's byte count
*         Length_Adjust_Sub_Profile_%p_ - Subtract enable bit - for profile <%p>
*         0x0 = ADD; ADD; When ADD, the value of constant byte count field is added to the descriptor byte count
*         0x1 = SUB; SUB; When SUB, the value of constant byte count field is subtracted from the descriptor byte count
*         Length_Adjust_Enable_profile_%p -
*         RW 0x0
*         Enable the length adjust
*         0x0 = Disable Length Adjust; Disable_Length_Adjust
*         0x1 = Enable Length Adjust; Enable_Length_Adjust
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
    IN GT_U8                                        devNum,
    IN GT_U32                                       tileNum,
    IN GT_U32                                       pdsNum,
    IN GT_U32                                       profileIndex,
    IN GT_BOOL                                      lengthAdjustOnly,
    OUT  PRV_CPSS_DXCH_SIP6_TXQ_PDS_QUEUE_PROFILE_STC  *profilePtr
);

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
);

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
);

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
);

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
*/GT_STATUS prvCpssDxChTxqFalconPdsQueueLengthAdjustProfileMapSet
(
    IN GT_U8 devNum,
    IN GT_U32 tileNum,
    IN GT_U32 pdsNum,
    IN GT_U32 queueNumber,
    IN GT_U32 profileIndex
);

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
    IN  GT_U8  devNum,
    IN  GT_U32 tileNum,
    IN  GT_U32 pdsNum,
    IN  GT_U32 queueNumber,
    OUT GT_U32 *profileIndexPtr
);

GT_STATUS prvCpssDxChTxqFalconPdsDataStorageGet
(
    IN GT_U8 devNum,
    IN GT_U32 tileNum,
    IN GT_U32 pdsNum,
    IN GT_U32 descriptorInd,
    IN GT_U32 *descriptorPtr
);

/**
* @internal prvCpssDxChTxqSip6PdsPbReadReqFifoLimitSet function
* @endinternal
*
* @brief  Configure read request Fifo limit
*
* @note   APPLICABLE DEVICES:AC5X;Harrier; Ironman;
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;Falcon; AC5P .
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
);

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
);

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
* @param[in] pdsNum                - data path index
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
);

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
);
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
);

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
);

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
);
/**
* @internal prvCpssSip7TxqPdsMailBoxGet function
* @endinternal
*
* @brief  This function getonfiguration of PDS mailbox (Required for deep buffer feature).
*
* @note   APPLICABLE DEVICES      :  AAS
* @note   NOT APPLICABLE DEVICES  :  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2;Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum               - device number
* @param[inout] connectionPtr     - (pointer to) data structure that contain all relevant connection parameters.
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
);
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
    OUT GT_U32   *dramQueuePtr
);

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
);
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
);
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
);

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
);
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
);
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
);

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
);

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
);

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
);

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

);
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
);

/**
* @internal prvCpssSip7TxqPdsPortPdsDbaParamsSet function
* @endinternal
*
* @brief  Set  Guaranteed Cnt Threshold (In descriptors) and
*         Alpha configuration used to Calculate the congestion factor.
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

);


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
);

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
);

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
);

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
);
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
);

GT_STATUS prvCpssSip7TxqPdsMarkDynamicProfileSet
(
    IN GT_U8  devNum,
    IN PRV_CPSS_DXCH_SIP6_TXQ_QUEUE_PROFILE_LONG_Q_STC *profilePtr
);

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
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChTxqPds */

