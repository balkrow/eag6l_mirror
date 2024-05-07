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
* @file prvCpssDxChHwGdmaMsg.h
*
* @brief Private definition for GDMA-MSG (FDB/CNC/IPE) functions for the DXCH devices.
*
*
* @version   1
********************************************************************************
*/

#ifndef __prvCpssDxChHwGdmaMsgh
#define __prvCpssDxChHwGdmaMsgh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/config/private/prvCpssGenIntDefs.h>
#include <cpss/generic/bridge/cpssGenBrgFdb.h>
#include <cpss/dxCh/dxChxGen/bridge/private/prvCpssDxChBrgFdbAu.h>
#include <cpss/dxCh/dxChxGen/cnc/cpssDxChCnc.h>

/**
* @internal prvCpssDxChGdmaFreeDb function
* @endinternal
*
* @brief   GDMA : free all dynamic allocation that the GDMA did during init and in runtime.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Ironman.
*
* @param[in] devNum                - The PP's device number .
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum
*/
GT_STATUS prvCpssDxChGdmaFreeDb
(
    IN  GT_U8                  devNum
);

/**
* @internal prvCpssDxChGdmaFdbMsgInit function
* @endinternal
*
* @brief   This function initializes the GDMA for FDB AUQ/FUQ , by allocating the array
*         of AUQs,FUQs , implemented by , per AUQ/FUQ we hold :
*          1. SGD - to state the size of the AUQ/FUQ (number of bytes , aligned on 'needed' bytes)
*          2. MSG buffer - the buffer that 'act' as AUQ/FUQ , that get the messages
*
*           NOTE:
*           1. for AUQ/FUQ : all AUQ messages have additional 8 bytes 'padded ZERO' to align to 32 bytes
*
*           2. ALL tiles will get the same size of those queues .
*              example :
*                  tile 0 : 1000 , 1000 , 1000 , 300
*                  tile 1 : 1000 , 1000 , 1000 , 300
*           3. the SGDs (for all tiles) that needed for those queues , are
*              'stolen' from the start of the memory
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Ironman.
*
* @param[in] devNum                - The device number to init the FDB AUQ/FUQ for.
* @param[in] maxNum64BMsgPerAuq    - The max number of 64B messages that the each AUQ should hold
*                                    this will decide how many AUQs to hold for the device
* @param[in] maxNum32BMsgPerFuq    - The max number of 32B messages that the each FUQ should hold
*                                    this will decide how many FUQs to hold for the device
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device Number
*
*/
GT_STATUS prvCpssDxChGdmaFdbMsgInit
(
    IN GT_U8                    devNum,
    IN GT_U32                   maxNum64BMsgPerAuq,
    IN GT_U32                   maxNum32BMsgPerFuq
);

/**
* @internal prvCpssDxChGdmaFdbAuFuMsgBlockGet function
* @endinternal
*
* @brief   GDMA : The function return a block (array) of AU / FU messages ,
*           the max number of elements defined by the caller
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Ironman.
*
* @param[in] devNum                   - the device number from which AU / FU are taken
* @param[in] portGroupId              - the portGroupId - for multi-port-groups support
* @param[in] queueType                - type of message queue to retrieve entries from
* @param[in] numOfAuFuPtr             - (pointer to) max number of AU / FU messages to receive
*
* @param[out] numOfAuFuPtr            - (pointer to)actual number of AU / FU messages that
*                                      were received
* @param[out] auFuMessagesPtr         - array that holds received AU / FU messages
*                                      pointer is allocated by the caller , with enough memory
*                                      for numOfAuPtr entries that was given as INPUT
*                                      parameter.
*                                      !!! NOT used when NULL !!!
* @param[out] sip7HwAuMessagesPtr    - sip7 : array that holds received AU messages in HW format
*                                      pointer is allocated by the caller , with enough memory
*                                      for numOfAuPtr entries that was given as INPUT
*                                      parameter.
*                                      !!! NOT used when NULL !!!
*
* @retval GT_OK                    - on success
* @retval GT_NO_MORE               - the action succeeded and there are no more
*                                    waiting messages
* @retval GT_FAIL                  - on failure
* @retval GT_BAD_STATE             - on bad state
*/
GT_STATUS prvCpssDxChGdmaFdbAuFuMsgBlockGet
(
    IN     GT_U8                       devNum,
    IN     GT_U32                      portGroupId,
    IN     MESSAGE_QUEUE_ENT           queueType,
    INOUT  GT_U32                      *numOfAuFuPtr,
    OUT    CPSS_MAC_UPDATE_MSG_EXT_STC *auFuMessagesPtr,
    OUT    PRV_CPSS_AU_FU_MSG_SIP7_STC *sip7HwAuMessagesPtr
);

/**
* @internal prvCpssDxChGdmaFdbAuqFuqMessagesNumberGet function
* @endinternal
*
* @brief  GDMA : The function scan the AU/FU queues and returns the number of
*         AU/FU messages in the queue.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Ironman.
*
* @param[in] devNum                   - the device number on which AU are counted
* @param[in] portGroupId              - the portGroupId - for multi-port-groups support
* @param[in] queueType                - AUQ or FUQ.
*
* @param[out] numOfMsgPtr             - (pointer to) number of AU messages in the specified queue.
* @param[out] endOfQueueReachedPtr    - (pointer to) GT_TRUE: The queue reached to the end.
*                                      GT_FALSE: else
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, queueType.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_NOT_INITIALIZED       - on not initialized queue
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note : none
*
*/
GT_STATUS prvCpssDxChGdmaFdbAuqFuqMessagesNumberGet
(
    IN  GT_U8                         devNum,
    IN  GT_U32                        portGroupId,
    IN  MESSAGE_QUEUE_ENT             queueType,
    OUT GT_U32                       *numOfMsgPtr,
    OUT GT_BOOL                      *endOfQueueReachedPtr
);

/**
* @internal prvCpssDxChGdmaCncMsgInit function
* @endinternal
*
* @brief   This function initializes the GDMA for CNC , by allocating the array
*         of CNCs , implemented by , per CNC unit queue we hold :
*          1. SGD - to state the size of the CNC unit queue (number of bytes , aligned on 'needed' bytes)
*          2. MSG buffer - the buffer that 'act' as CNC unit queue , that get the messages
*
*           NOTE:
*           1. ALL port groups will get the same size of those queues .
*              example :
*                  pipe 0 : CNC unit 0 : 1000 , 1000 , 1000 , 300
*                  pipe 0 : CNC unit 1 :  500 ,  500 ,  500 , 200
*                   ....
*                  pipe 1 : CNC unit 0 : 1000 , 1000 , 1000 , 300
*                  pipe 1 : CNC unit 1 :  500 ,  500 ,  500 , 200
*                   ....
*           3. the SGDs (for all CNC units) that needed for those queues , are
*              'stolen' from the start of the memory
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Ironman.
*
* @param[in] devNum                - The device number to init the CNC unit for.
* @param[in] cncUnitId             - The CNC unit id (local in the port group)
* @param[in] maxNum16BMsgPerCncQueue - The max number of 16B messages that the each CNC unit queue
*                                    should hold , this will decide how many CNC unit queues to hold for the device
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device Number
*
*/
GT_STATUS prvCpssDxChGdmaCncMsgInit
(
    IN GT_U8                    devNum,
    IN GT_U32                   cncUnitId,
    IN CPSS_DMA_QUEUE_CFG_STC   *cncCfgPtr,
    IN GT_U32                   maxNum16BMsgPerCncQueue
);

/**
* @internal prvCpssDxChGdmaCncBeforeDumpTrigger function
* @endinternal
*
* @brief  The function check is the CNC dump is allowed , and will add the number
*         of expected CNC counters to 'unreadCncCounters'.
*         The function expected to be called BEFORE the caller trigger the CNC upload in the unit.
*         The function checks that there is no unfinished previous CNC dump, that
*         not allow new trigger to start.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES: AAS)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      Note: Bobcat3 has CNC unit per port group.
*                                      Triggering upload from both CNC ports groups in parallel is not supported.
*                                      Application must choose a single Port-Group at a time.
*                                      meaning that only value 'BIT_0' or 'BIT_1' are supported.
* @param[in] inProcessBlocksBmpArr[]- (array of) bitmap of in-process blocks
*                                      value 1 of the bit#n means that CNC upload
*                                      is not finished yet for block n
*
* @param[in] format                - CNC counter HW format
* @param[in] unitNum               - The CNC unit (0..3)
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_READY             - Upload started after upload trigger or
*                                    continued after queue rewind but yet
*                                    not paused due to queue full and yet not finished.
*                                    The part of queue memory does not contain uploaded counters yet.
*                                    No counters retrieved.
* @retval GT_NOT_SUPPORTED         - block upload operation not supported.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note
*       1. The device support parallel dump operations on all 4 CNC units.
*       2. The caller expected to trigger the CNC upload in the unit
*
*/
GT_STATUS prvCpssDxChGdmaCncBeforeDumpTrigger
(
    IN  GT_U8                       devNum,
    IN  GT_PORT_GROUPS_BMP          portGroupsBmp,
    IN  GT_U32                      inProcessBlocksBmpArr[/*2*/],
    IN  CPSS_DXCH_CNC_COUNTER_FORMAT_ENT  format,
    IN  GT_U32                      unitNum
);

/**
* @internal prvCpssDxChGdmaCncUploadedBlockGet function
* @endinternal
*
* @brief   The function return a block (array) of CNC counter values,
*         the maximal number of elements defined by the caller.
*         The CNC upload may triggered by cpssDxChCncPortGroupBlockUploadTrigger.
*         The CNC upload transfers whole CNC block (2K CNC counters)
*         to FDB Upload queue. An application must get all transferred counters.
*         An application may sequentially upload several CNC blocks before start
*         to retrieve uploaded counters.
*         The device may transfer only part of CNC block because of FUQ full. In
*         this case the cpssDxChCncPortGroupUploadedBlockGet may return only part of the
*         CNC block with return GT_OK. An application must to call
*         cpssDxChCncPortGroupUploadedBlockGet one more time to get rest of the block.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Ironman.
*
* @param[in] devNum                   - the device number from which FU are taken
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES: AAS)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
*                                      Note: Bobcat3 has CNC unit per port group.
*                                      Getting messages from both CNC ports groups in parallel is not supported.
*                                      Application must choose a single Port-Group at a time.
*                                      meaning that only value 'BIT_0' or 'BIT_1' are supported.
* @param[in,out] numOfCounterValuesPtr    - (pointer to) maximal number of CNC counters
*                                      values to get.This is the size of
* @param[in] counterValuesPtr         array allocated by caller.
* @param[in] format                   - CNC counter HW format
* @param[in,out] numOfCounterValuesPtr    - (pointer to) actual number of CNC
*                                      counters values in counterValuesPtr.
*
* @param[out] counterValuesPtr         - (pointer to) array that holds received CNC
*                                      counters values. Array must be allocated by
*                                      caller. For multi port group devices
*                                      the result counters contain the sum of
*                                      counters read from all port groups.
*                                      It contains more bits than in HW.
*                                      For example the sum of 4 35-bit values may be
*                                      37-bit value.
*
* @retval GT_OK                    - on success
* @retval GT_NO_MORE               - the action succeeded and there are no more waiting
*                                       CNC counter value
* @retval GT_NOT_READY             - Upload started after upload trigger or
*                                       continued after queue rewind but yet
*                                       not paused due to queue full and yet not finished.
*                                       The part of queue memory does not contain uploaded counters yet.
*                                       No counters retrieved.
* @retval GT_FAIL                  - on failure
* @retval GT_BAD_PARAM             - on wrong devNum
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_STATE             - if the previous FU messages were not
*                                       retrieved yet by cpssDxChBrgFdbFuMsgBlockGet.
* @retval GT_NOT_SUPPORTED         - block upload operation not supported.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The device use DMA memory that set by cpssDxChCncUploadInit.
*
*/
GT_STATUS prvCpssDxChGdmaCncUploadedBlockGet
(
    IN     GT_U8                             devNum,
    IN     GT_PORT_GROUPS_BMP                portGroupsBmp,
    INOUT  GT_U32                            *numOfCounterValuesPtr,
    IN     CPSS_DXCH_CNC_COUNTER_FORMAT_ENT  format,
    OUT    CPSS_DXCH_CNC_COUNTER_STC         counterValuesPtr[]
);

/**
* @internal prvCpssDxChGdmaRingFullGet function
* @endinternal
*
* @brief   get indication if the ring (in wait mode) is full (busy waiting).
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; xCat3; AC5; Lion2.
*
* @param[in] devNum                 - The device number .
* @param[in] portGroupsBmp            - bitmap of Port Groups.
*                                      NOTEs:
*                                      1. for non multi-port groups device this parameter is IGNORED.
*                                      2. for multi-port groups device :
*                                      (APPLICABLE DEVICES : AAS)
*                                      bitmap must be set with at least one bit representing
*                                      valid port group(s). If a bit of non valid port group
*                                      is set then function returns GT_BAD_PARAM.
*                                      Value CPSS_PORT_GROUP_UNAWARE_MODE_CNS is supported.
* @param[in] clientMsgType          - The client type
* @param[in] subClientIndex         - The sub client index
*                                    Relevant to CNC and IPE.
* @param[out] isFullPortGroupsBmpPtr - (pointer to) BMP of per port group bit if the queue full.
*                                       a bit is 1 meaning that port group ring is full.
*                                       a bit is 0 meaning that port group ring is not full.
*
* @retval GT_OK                    - on success.
* @retval GT_HW_ERROR              - HW error
*/
GT_STATUS prvCpssDxChGdmaRingFullGet
(
    IN GT_U8                                    devNum,
    IN GT_U32                                   portGroupsBmp,
    IN PRV_CPSS_DXCH_GDMA_CLIENT_TYPE_MSG_ENT   clientMsgType,
    IN GT_U32                                   subClientIndex,
    OUT GT_PORT_GROUPS_BMP                      *isFullPortGroupsBmpPtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssDxChHwGdmaMsgh */

