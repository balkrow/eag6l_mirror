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
* @file prvCpssDxChHwGdmaNetIfFunc.h
*
* @brief Private definition for GDMA-netIf functions for the DXCH devices.
*
*
* @version   1
********************************************************************************
*/

#ifndef __prvCpssDxChHwGdmaNetIfFunch
#define __prvCpssDxChHwGdmaNetIfFunch

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

typedef struct{
    /**@brief : the first global queue for TO_CPU for this port.
     *  relevant when numOfQueuesToCpu != 0
    */
    GT_U32                   firstQueueToCpu;

    /**@brief : number of queues for TO_CPU for this port.
     *  can be 0 , when only FROM_CPU queues exists/needed for this port
    */
    GT_U32                   numOfQueuesToCpu;

    /**@brief : the first global queue for FROM_CPU for this port.
     *  relevant when numOfQueuesFromCpu != 0
    */
    GT_U32                   firstQueueFromCpu;

    /**@brief : number of queues for FROM_CPU for this port.
     *  can be 0 , when only TO_CPU queues exists/needed for this port
    */
    GT_U32                   numOfQueuesFromCpu;
}PRV_CPSS_DXCH_NETIF_GDMA_BIND_CPU_PORT_INFO_STC;

/**
* @internal prvCpssDxChGdmaNetIfRxInit function
* @endinternal
*
* @brief   This function initializes the GDMA for TO_CPU , by allocating the array
*         of TO_CPU descriptors , and the TO_CPU buffers
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Ironman.
*
* @param[in] devNum                   - The device number to init the Rx unit for.
*    GT_OK on success, or
*    GT_FAIL otherwise.
*/
GT_STATUS prvCpssDxChGdmaNetIfRxInit
(
    IN GT_U8                    devNum
);

/**
* @internal prvCpssDxChGdmaNetIfTxInit function
* @endinternal
*
* @brief   This function initializes the GDMA for FROM_CPU , by allocating the array
*         of FROM_CPU descriptors , and the 'eDSA' buffers
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Ironman.
*
* @param[in] devNum                   - The device number to init the Rx unit for.
*    GT_OK on success, or
*    GT_FAIL otherwise.
*/
GT_STATUS prvCpssDxChGdmaNetIfTxInit
(
    IN GT_U8                    devNum
);


/**
* @internal prvCpssDxChGdmaNetIfRxBufFreeWithSize function
* @endinternal
*
* @brief  GDMA : Free a list of buffers, that where previously passed to the application
*         by the 'Rx Packet Get' API. -- GDMA relate
*
*         NOTE: in CPSS_RX_BUFF_DYNAMIC_ALLOC_E/CPSS_RX_BUFF_STATIC_ALLOC_E mode :
*               the function NOT do any read/write to HW
*               the function NOT do any read/write to descriptors/buffers memory in DRAM
*               the function only save the info about the released buffer into the link list
*                   that wait for descriptor(s) to need to bind to buffer.
*               in CPSS_RX_BUFF_NO_ALLOC_E mode:
*                   if the amount of buffers returned by this function (in total by all calls to it)
*                   is less than the number of descriptors that allocated for this queue then
*                       the function will update 'empty' descriptor , with this buffer info
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; xCat3; AC5; Lion2.
*
* @param[in] devNum                   - The device number through which these buffers where received.
* @param[in] globalRxQueue            - The Rx queue number through which these buffers where received.
* @param[in] rxBuffList[]             - List of Rx buffers to be freed.
* @param[in] rxBuffSizeList[]         - List of Rx Buffers sizes , to set to the free descriptor
*                                      if this parameter is NULL --> we ignore it.
* @param[in] buffListLen              - Length of rxBufList.
*
* @retval GT_OK on success, or
* @retval GT_FAIL otherwise.
*/
GT_STATUS prvCpssDxChGdmaNetIfRxBufFreeWithSize
(
    IN GT_U8    devNum,
    IN GT_U8    globalRxQueue,
    IN GT_U8    *rxBuffList[],
    IN GT_U32   rxBuffSizeList[],
    IN GT_U32   buffListLen
);

/**
* @internal prvCpssDxChGdmaNetIfRxCountersGet function
* @endinternal
*
* @brief  GDMA : Get the Rx packet counters for the specific queue.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; xCat3; AC5; Lion2.
*
* @param[in] devNum                   - device number
* @param[in] globalRxQueue            - The Rx queue number through which these packets received.
*
* @param[out] rxCountersPtr           - (pointer to) rx counters on this queue
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note Counters are reset on every read.
*
*/
GT_STATUS prvCpssDxChGdmaNetIfRxCountersGet
(
    IN    GT_U8      devNum,
    IN    GT_U8      globalRxQueue,
    OUT   CPSS_DXCH_NET_SDMA_RX_COUNTERS_STC *rxCountersPtr
);

/**
* @internal prvCpssDxChGdmaNetIfRxErrorCountersGet function
* @endinternal
*
* @brief  GDMA : Get the Rx Error packet counters for all the queues.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; xCat3; AC5; Lion2.
*
* @param[in] devNum                - device number
*
* @param[out] rxErrCountPtr        - (pointer to) The total number of Rx resource errors on
*                                    the device for all the queues.
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_FAIL                  - otherwise.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The counters returned by this function reflects the number of Rx errors
*       that occurred since the last call to this function.
*
*/
GT_STATUS prvCpssDxChGdmaNetIfRxErrorCountersGet
(
    IN GT_U8    devNum,
    OUT CPSS_NET_SDMA_RX_ERROR_COUNTERS_STC  *rxErrCountPtr
);

/**
* @internal prvCpssDxChGdmaNetIfRxQueueEnableSet function
* @endinternal
*
* @brief  GDMA :  Set (enable/disable) the specified traffic class queue for RX
*         packets in CPU.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Bobcat3; Aldrin; AC3X; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                - device number
* @param[in] globalRxQueue         - The Rx queue number through which these packets received.
*
* @param[in] enable                - GT_TRUE, enable queue
*                                    GT_FALSE, disable queue
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChGdmaNetIfRxQueueEnableSet
(
    IN  GT_U8                           devNum,
    IN  GT_U8                           globalRxQueue,
    IN  GT_BOOL                         enable
);

/**
* @internal prvCpssDxChGdmaNetIfRxQueueEnableGet function
* @endinternal
*
* @brief  GDMA :  Get status of the specified traffic class queue for RX
*         packets in CPU.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Bobcat3; Aldrin; AC3X; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] globalRxQueue            - The Rx queue number through which these packets received.
*
* @param[out] enablePtr                - GT_TRUE, enable queue
*                                      GT_FALSE, disable queue
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL ptr
*/
GT_STATUS prvCpssDxChGdmaNetIfRxQueueEnableGet
(
    IN  GT_U8                           devNum,
    IN  GT_U8                           globalRxQueue,
    OUT GT_BOOL                         *enablePtr
);

/**
* @internal prvCpssDxChGdmaNetIfPrePendTwoBytesHeaderSet function
* @endinternal
*
* @brief  GDMA : Enables/Disable pre-pending a two-byte header to all packets forwarded
*         to the CPU (via the CPU port or the PCI interface).
*         This two-byte header is used to align the IPv4 header to 32 bits.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Bobcat3; Aldrin; AC3X; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] enable                   - GT_TRUE  - Two-byte header is pre-pended to packets
*                                      forwarded to the CPU.
*                                      GT_FALSE - Two-byte header is not pre-pended to packets
*                                      forward to the CPU.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChGdmaNetIfPrePendTwoBytesHeaderSet
(
    IN  GT_U8                           devNum,
    IN  GT_BOOL                         enable
);

/**
* @internal prvCpssDxChGdmaNetIfRxPacketGet function
* @endinternal
*
* @brief  GDMA :  This function returns packets from PP destined to the CPU port.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Bobcat3; Aldrin; AC3X; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                - Device number.
* @param[in] globalRxQueue         - The Rx queue number through which these packets received.
* @param[in] numOfBuffPtr          - (pointer to) Num of buffs in packetBuffsArrPtr that application want to get (at max).
*
* @param[in] numOfBuffPtr          - (pointer to) Num of buffs in packetBuffsArrPtr that CPSS bind for the packet.
* @param[out] packetBuffsArrPtr[]  - (pointer to)The received packet buffers list.
* @param[out] buffLenArr[]         - List of buffer lengths for packetBuffsArrPtr.
*
* @retval GT_NO_MORE               - no more packets on the device/queue
* @retval GT_OK                    - packet got with no error.
* @retval GT_FAIL                  - failed to get the packet
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
* @retval GT_BAD_PARAM             - wrong devNum,portNum,queueIdx.
* @retval GT_NOT_INITIALIZED       - the library was not initialized
* @retval GT_DSA_PARSING_ERROR     - DSA tag parsing error.
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS prvCpssDxChGdmaNetIfRxPacketGet
(
    IN GT_U8                                devNum,
    IN GT_U8                                globalRxQueue,
    INOUT GT_U32                            *numOfBuffPtr,
    OUT GT_U8                               *packetBuffsArrPtr[],
    OUT GT_U32                              buffLenArr[]
);

/**
* @internal prvCpssDxChGdmaNetIfTxPacketSend function
* @endinternal
*
* @brief   GDMA : This function receives packet buffers & parameters from the different
*         core Tx functions. Prepares them for the transmit operation, and
*         enqueues the prepared descriptors to the PP's tx queues. -- GDMA relate
*         function activates Tx GDMA , function doesn't wait for event of
*         "Tx buffer queue" from PP
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Ironman.
*
* @param[in] devNum                   - Device number.
* @param[in] pcktParamsPtr            - The internal packet params to be set into the packet descriptors.
* @param[in] buffList[]               - The packet data buffers list.
* @param[in] buffLenList[]            - A list of the buffers len in buffList.
* @param[in] numOfBufs                - Length of buffList.
* @param[in] synchronicSend           - sent the packets synchronic (not return until the
*                                      buffers can be free)
*                                      GT_TRUE - send synchronic
*                                           the free of buffers done from this function
*                                      GT_FALSE - send asynchronous (not wait for buffers to be free)
*                                           the free of buffers done from 'Tx ended' ISR context
*
* @retval GT_OK                    - on success, or
* @retval GT_NO_RESOURCE           - if there is not enough free elements in the fifo
*                                       associated with the Event Request Handle.
* @retval GT_EMPTY                 - if there are not enough descriptors to do the sending.
* @retval GT_ABORTED               - if command aborted (during shutDown operation)
* @retval GT_HW_ERROR              - when synchronicSend = GT_TRUE and after transmission
*                                       the last descriptor own bit wasn't changed for long time.
* @retval GT_BAD_PARAM             - the data buffer is longer than allowed.
*                                       buffer data can occupied up to the maximum
*                                       number of descriptors defined or queue assigned for
*                                       Packet Generator, unavailable for regular transmission.
* @retval GT_BAD_PTR               - one of the parameters is NULL pointer
*                                       GT_FAIL otherwise.
*
* @note 1. Each buffer should be at least 8 bytes long.
*       2. first buffer must be at least 24 bytes for tagged packet,
*          20 for untagged packet.
*       3. support ALL DSA tag formats regulate / extended / eDSA
*       4. Packet's length should include 4 bytes for CRC. (legacy from previous devices)
*
*       logic based on dxChNetIfSdmaTxPacketSend(...)
*
*/
GT_STATUS prvCpssDxChGdmaNetIfTxPacketSend
(
    IN GT_U8                             devNum,
    IN CPSS_DXCH_NET_TX_PARAMS_STC      *pcktParamsPtr,
    IN GT_U8                            *buffList[],
    IN GT_U32                            buffLenList[],
    OUT GT_U32                           numOfBufs,
    IN GT_BOOL                           synchronicSend
);

/**
* @internal prvCpssDxChNetIfSdmaTxDsaLengthAndDataOffsetGet function
* @endinternal
*
* @brief   This function calculated DSA tag bytes length and the packet data offset
*         based on tagging existence.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] packetParamsPtr          - (pointer to) The internal packet params to be set into
*                                      the packet descriptors.
*
* @param[out] dsaTagNumOfBytesPtr      - (pointer to) DSA tag length in bytes.
* @param[out] dataOffsetPtr            - (pointer to) packet data offset.
*
* @retval GT_OK                    - on success, or
* @retval GT_BAD_PARAM             - on wrong DSA tag type
*/
GT_STATUS prvCpssDxChNetIfSdmaTxDsaLengthAndDataOffsetGet
(
    IN  CPSS_DXCH_NET_TX_PARAMS_STC *packetParamsPtr,
    OUT GT_U32                      *dsaTagNumOfBytesPtr,
    OUT GT_U32                      *dataOffsetPtr
);

/**
* @internal prvCpssDxChNetIfSdmaTxBuffQueueEvIsrHandle function
* @endinternal
*
* @brief   GDMA : This routine frees all transmitted packets descriptors.
*          In addition, all user relevant data in Tx End FIFO. -- GDMA relate.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Ironman.
*
* @param[in] devNum                - The device number the packet was transmitted from
* @param[in] globalTxQueue         - The Tx queue number that need to free descriptors.
*
* @retval GT_OK on success, or
*
* @retval GT_NOT_INITIALIZED       - the library was not initialized
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_STATE             - the CPSS does not handle the ISR for the device
*                                       so function must not be called
*                                       GT_FAIL otherwise.
*
* @note Invoked by ISR routine !!
*       the function is called internally in the CPSS from the driver as a call
*       back , see bind in function hwPpPhase1Part1(...)
*
*       may also be called also from non ISR context , but the function need to
*       be called under 'ISR LOCK/UNLOCK'
*
*   logic taken from : prvCpssDxChNetIfSdmaTxBuffQueueEvIsrHandle
*
*/
GT_STATUS prvCpssDxChGdmaNetIfTxBuffQueueEvIsrHandle
(
    IN GT_U8                devNum,
    IN GT_U8                globalTxQueue
);

/**
* @internal prvCpssDxChGdmaNetIfTxQueueEnableSet function
* @endinternal
*
* @brief  GDMA :  Set (enable/disable) the specified traffic class queue for TX
*         packets FROM CPU.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Bobcat3; Aldrin; AC3X; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                - device number
* @param[in] globalTxQueue         - The Tx queue number through which these packets received.
*
* @param[in] enable                - GT_TRUE, enable queue
*                                    GT_FALSE, disable queue
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChGdmaNetIfTxQueueEnableSet
(
    IN  GT_U8                           devNum,
    IN  GT_U8                           globalTxQueue,
    IN  GT_BOOL                         enable
);

/**
* @internal prvCpssDxChGdmaNetIfTxQueueEnableGet function
* @endinternal
*
* @brief  GDMA :  Get status of the specified traffic class queue for TX
*         packets FROM CPU.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Bobcat2; Caelum; Bobcat3; Aldrin; AC3X; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
* @param[in] globalTxQueue            - The Tx queue number through which these packets received.
*
* @param[out] enablePtr                - GT_TRUE, enable queue
*                                      GT_FALSE, disable queue
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on bad parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_BAD_PTR               - on NULL ptr
*/
GT_STATUS prvCpssDxChGdmaNetIfTxQueueEnableGet
(
    IN  GT_U8                           devNum,
    IN  GT_U8                           globalTxQueue,
    OUT GT_BOOL                         *enablePtr
);

/**
* @internal prvCpssDxChGdmaNetIfEventCpuPortConvert function
* @endinternal
*
* @brief   GDMA : Function checks and gets valid mapping for port per event type
*          that relate to TO_CPU/FROM_CPU traffic
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - device number
* @param[in]  uniEvent                - the unified event , that relate to TO_CPU/FROM_CPU traffic.
* @param[in] evConvertType            - event convert type
* @param[in] IN_queue                 - queue to convert
*
* @param[out] OUT_queuePtr            - (pointer to) converted queue, according to convert event type
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - wrong devNum, portNumFrom
* @retval GT_BAD_PTR               - on bad pointer
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChGdmaNetIfEventCpuPortConvert
(
    IN GT_SW_DEV_NUM                    devNum,
    IN  CPSS_UNI_EV_CAUSE_ENT           uniEvent,
    IN  PRV_CPSS_EV_CONVERT_DIRECTION_ENT    evConvertType,
    IN  GT_U32                          IN_queue,
    OUT GT_U32                          *OUT_queuePtr
);

/**
* @internal prvCpssDxChNetIfGdmaCpuPortBindToQueuesSet function
* @endinternal
*
* @brief   Bind CPU physical port to the number of GDMA queues that it need to use.
*           number of queues for 'TO_CPU'   (Rx direction)
*           number of queues for 'FROM_CPU' (Tx direction)
*           NOTE:
*               1. Those are NOT the 'TXQ queues' that the port give in port mapping <txqPortNumber>
*                  Those are GDMA queues.
*               2. must be called after 'port mapping' and before 'phase 2 init'
*               so can be considered as need to be called right after 'port mapping'
*               3. after calling this API for all ports of type CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E
*                  and giving the info to CPSS about number of GDMA queues needed
*                  the user can call to cpssDxChNetIfGdmaCpuPortsInfoGet , to get the 'global GDMA queues' that each port get.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Ironman.
*
* @param[in] devNum                - Device number.
* @param[in] cpuPortNumber         - The CPU physical port number.
*                                    A physical port that used CPSS_DXCH_PORT_MAPPING_TYPE_CPU_SDMA_E
*                                    in the 'port mapping' API
* @param[in] cpuPortInfoPtr        - (pointer to) The CPU port info
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - on bad device number or bad physical port
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_OUT_OF_RANGE          - the numOfQueuesToCpu or numOfQueuesFromCpu more than the port supports
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*/
GT_STATUS prvCpssDxChNetIfGdmaCpuPortBindToQueuesSet
(
    IN GT_U8                    devNum,
    IN GT_PHYSICAL_PORT_NUM     cpuPortNumber,
    IN PRV_CPSS_DXCH_NETIF_GDMA_BIND_CPU_PORT_INFO_STC    *cpuPortInfoPtr
);

/**
* @internal prvCpssDxChNetIfGdmaInitAfterPortMapping function
* @endinternal
*
* @brief   GDMA : set the information about all the network interfaces according
*           to 'port mapping' params
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - The device number.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device Number
* @retval GT_NOT_FOUND             - device not found in the DB
*/
GT_STATUS prvCpssDxChNetIfGdmaInitAfterPortMapping
(
    IN GT_U8                        devNum,
    IN GT_PHYSICAL_PORT_NUM         portNum,
    IN GT_U32                       globalDma,
    IN GT_U32                       netIfNum
);

/**
* @internal prvCpssDxChNetIfGdmaSgdChainSizeGet function
* @endinternal
*
* @brief   Get Ring/Queue SDG chain size current HW configuration
*          Reperesents number of packets-in-queue to be sent by PktGen if queue enabled
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Ironman.
*
* @param[in] devNum             PP's device number
* @param[in] gdmaUnit           gdma unit number
* @param[in] queueNum           Queue number (0..32)
* @param[out] sgdChainSizePtr   sgd_chain_size value in a HW-register
*
* @retval GT_OK
* @retval GT_BAD_PARAM        - The requested absolute or incremental value causes for
*                                 overload or underload
* @retval GT_NOT_INITIALIZED  - if the REG driver was not initialized
*
* @note This API is for TxGenerator queue but could be used for other queues as well
*/
GT_U32 prvCpssDxChNetIfGdmaSgdChainSizeGet
(
    IN  GT_U8       devNum,
    IN  GT_U32      gdmaUnit,
    IN  GT_U32      queueNum,
    IN  GT_U32      *sgdChainSizePtr
);

/**
* @internal prvCpssDxChNetIfGdmaSgdChainSizeSet function
* @endinternal
*
* @brief   Set Ring/Queue SDG chain size
*          The setting could be 'absolute set Num' or incremental +Num / -Num
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Ironman.
*
* @param[in] devNum             PP's device number
* @param[in] gdmaUnit           gdma unit number
* @param[in] queueNum           Queue number (0..32)
* @param[in] modeIsIncremental  GT_FALSE set 'sgdChainSize' as absolute value
*                               GT_TRUE incremental +sgdChainSize/-sgdChainSize
* @param[in] sgdChainSize       signed, value to be set into sgd_chain_size
*                               or to increment/decrement current register's value
*
* @retval GT_OK
* @retval GT_BAD_PARAM        - The requested absolute or incremental value causes for
*                                 overload or underload
* @retval GT_NOT_INITIALIZED  - if the REG driver was not initialized
*
* @note This API is for TxGenerator queue but could be used for other queues as well
*/
GT_U32 prvCpssDxChNetIfGdmaSgdChainSizeSet
(
    IN  GT_U8       devNum,
    IN  GT_U32      gdmaUnit,
    IN  GT_U32      queueNum,
    IN  GT_BOOL     modeIsIncremental,
    IN  GT_32       sgdChainSize
);

/**
* @internal prvCpssDxChNetIfGdmaTxGeneratorEnable function
* @endinternal
*
* @brief   This function enables selected Tx GDMA Generator.
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Ironman.
*
* @param[in] devNum                   - Device number.
* @param[in] unitId                   - GDMA unit id/number.
* @param[in] txQueue                  - Tx GDMA queue to enable.
* @param[in] burstEnable              - GT_TRUE for packets burst generation,
*                                     - GT_FALSE for continuous packets generation
* @param[in] burstPacketsNumber       - Number of packets in burst.
*                                      Relevant only if burstEnable == GT_TRUE.
*                                      (APPLICABLE RANGES: 0..512M-1)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - Tx SDMA is not in Generator mode or burst size.
* @retval GT_NOT_INITIALIZED       - the library was not initialized.
*
* @note If burstEnable and queue is empty, configuration is done beside the enabling which will
*       be implicitly performed after the first packet will be added to the
*       queue.
*       On each HW Generator enabling the Tx GDMA Current Descriptor Pointer
*       register will set to point to the first descriptor in chain list.
*/
GT_STATUS prvCpssDxChNetIfGdmaTxGeneratorEnable
(
    IN  GT_U8                           devNum,
    IN  GT_U32                          unitId,
    IN  GT_U32                          txQueue,
    IN  GT_BOOL                         burstEnable,
    IN  GT_U32                          burstPacketsNumber
);

/**
* @internal prvCpssDxChNetIfGdmaTxGeneratorDescriptorGet function
* @endinternal
*
* @brief   Get GDMA descriptor's parced information
*
* @note   APPLICABLE DEVICES:      AAS
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum           - Device number.
* @param[in] descPtr          - GDMA descriptor used for that packet
* @param[out] buffAddr        - packet buffer virtual address
* @param[out] buffLen         - packet data length
* @param[out] isValid         - VALID bit is set in descriptor
* @param[out] handledByHw     - GT_TRUE - OWN bit0 is cleared by HW (handled by HW)
*                               GT_FALSE - bit was set by SW and still not handled by HW
*
* @retval GT_OK               - on success
* @retval GT_FAIL             - cannot convert Physical descriptor's address to Virtual
*
* @note  Output 'handledByHw' is optional - may have NULL storage
*/
GT_STATUS prvCpssDxChNetIfGdmaTxGeneratorDescriptorGet
(
    IN  GT_U8                   devNum,
    IN  PRV_CPSS_GDMA_DESC_STC  *descPtr,
    OUT GT_U8                   **buffAddr,
    OUT GT_U32                  *buffLen,
    OUT GT_BOOL                 *isValid,
    OUT GT_BOOL                 *handledByHw
);

/**
* @internal prvCpssDxChNetIfGdmaTxGeneratorDescriptorSet function
* @endinternal
*
* @brief   Set GDMA descriptor (SGD) with packet buffer for TxGenerator
*          or only update the VALID bit (if packet lenght or buffAddr are NULL)
*
* @note   APPLICABLE DEVICES:      AAS
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - Device number.
* @param[in] descPtr                  - GDMA descriptor used for that packet
* @param[in] buffAddr                 - packet buffer virtual address
* @param[in] buffLen                  - packet data length
* @param[in] validSet                 - set or no(clear) Descriptor's bit VALID
*
* @retval GT_OK               - on success
* @retval GT_FAIL             - cannot convert Virtual to Physical address
*/
GT_STATUS prvCpssDxChNetIfGdmaTxGeneratorDescriptorSet
(
    IN  GT_U8                   devNum,
    IN  PRV_CPSS_GDMA_DESC_STC  *descPtr,
    IN  GT_U8                   *buffAddr,
    IN  GT_U32                  buffLen,
    IN  GT_BOOL                 validSet
);

/**
* @internal prvCpssDxChNetIfGdmaTxGeneratorDescriptorsSwap function
* @endinternal
*
* @brief   Swap an information between 2 descriptors and set/clear Valid bit
*
* @note   APPLICABLE DEVICES:      AAS
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] descPtr1        - 1st descriptor to be swapped with 2nd
* @param[in] descValid1      - GT_TRUE/GT_FALSE set/clear VALID bit on 1st after swap
* @param[in] descPtr2        - 2nd descriptor to be swapped with 1st
* @param[in] descValid2      - GT_TRUE/GT_FALSE set/clear VALID bit on 2nd after swap
*
* @retval GT_VOID
*/
GT_VOID prvCpssDxChNetIfGdmaTxGeneratorDescriptorsSwap
(
    IN  PRV_CPSS_GDMA_DESC_STC  *descPtr1,
    IN  GT_BOOL                 descValid1,
    IN  PRV_CPSS_GDMA_DESC_STC  *descPtr2,
    IN  GT_BOOL                 descValid2
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssDxChHwGdmaNetIfFunch */

