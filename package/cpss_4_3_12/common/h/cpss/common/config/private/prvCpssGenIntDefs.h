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
* @file prvCpssGenIntDefs.h
*
* @brief This file includes general structures definitions for interrupts
* handling, Packet Reception, and Address Update Messages
*
*
* @version   16
********************************************************************************
*/
#ifndef __prvCpssGenIntDefsh
#define __prvCpssGenIntDefsh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* support running from the PSS */
#ifndef __gtCoreIntDefsh

/* Defines  */
#define NUM_OF_RX_QUEUES    (8)
#define NUM_OF_TX_QUEUES    (8)
#define NUM_OF_SDMA_QUEUES  (8*CPSS_MAX_SDMA_CPU_PORTS_CNS)

#endif /* #ifndef __gtCoreIntDefsh */

#include <cpssCommon/cpssBuffManagerPool.h>
#include <cpss/common/cpssTypes.h>

/* support running from the PSS */
#ifndef __gtCoreIntDefsh

/************************   RX Related Definitions  ***************************/

/* macro that define the alignment needed for rx descriptor that given to the PP
relate to packets that the PP send to the CPU*/
#define RX_DESC_ALIGN       (16)  /* In bytes */
/* macro that define the alignment needed for rx buffers that given to the PP ,
relate to packets that the PP send to the CPU*/
#define RX_BUFF_ALIGN       (8)     /* In bytes */

/* macro that define the multiple size of rx buffers that given to the PP ,
relate to packets that the PP send to the CPU*/
#define RX_BUFF_SIZE_MULT   (8)   /* In bytes */
/* macro that define the rx descriptor size that given to the PP ,
relate to packets that the PP send to the CPU*/
#define RX_DESC_SIZE        (16)  /* In Bytes */

/* macro that define that CPU own the Rx descriptor --
the value in field "ownership bit" in the Rx descriptor in the PP
relate to packets that the PP send to the CPU */
#define RX_DESC_CPU_OWN     (0)
/* macro that define that DMA (PP) own the Rx descriptor --
the value in field "ownership bit" in the Rx descriptor in the PP
relate to packets that the PP send to the CPU */
#define RX_DESC_DMA_OWN     (1)

/* Resets the Rx descriptor's word1 */
#define RX_DESC_RESET_MAC(devNum,rxDesc) ((rxDesc)->word1 = CPSS_32BIT_LE(0xA0000000))

/* Resets the Rx descriptor's word2 */
#define RX_DESC_RESET_WORD2_MAC(devNum,rxDesc) ((rxDesc)->word2 = 0)

/* Returns / Sets the Own bit field of the rx descriptor.           */
#define RX_DESC_GET_OWN_BIT(rxDesc) (((rxDesc)->word1) >> 31)

/* Returns the First bit field of the rx descriptor.                */
#define RX_DESC_GET_FIRST_BIT(rxDesc) ((((rxDesc)->word1) >> 27) & 0x1)

/* Returns the Last bit field of the rx descriptor.                 */
#define RX_DESC_GET_LAST_BIT(rxDesc) ((((rxDesc)->word1) >> 26) & 0x1)

/* Returns the Resource error bit field of the rx descriptor.       */
#define RX_DESC_GET_REC_ERR_BIT(rxDesc) ((((rxDesc)->word1) >> 28) & 0x1)

/* Return the buffer size field from the second word of an Rx desc. */
/* Make sure to set the lower 3 bits to 0.                          */
#define RX_DESC_GET_BUFF_SIZE_FIELD_MAC(rxDesc)             \
            (((((rxDesc)->word2) >> 3) & 0x7FF) << 3)
#define RX_DESC_SET_BUFF_SIZE_FIELD_MAC(devNum,rxDesc,val)         \
            (rxDesc)->word2 = CPSS_32BIT_LE((rxDesc)->word2);  \
            U32_SET_FIELD_MAC((rxDesc)->word2,0,14,(val));      \
            (rxDesc)->word2 = CPSS_32BIT_LE((rxDesc)->word2)

/* Return the byte count field from the second word of an Rx desc.  */
/* Make sure to set the lower 3 bits to 0.                          */
#define RX_DESC_GET_BYTE_COUNT_FIELD(rxDesc)        \
            ((((rxDesc)->word2) >> 16) & 0x3FFF)


#endif /* #ifndef __gtCoreIntDefsh */

/**
* @struct PRV_CPSS_RX_DESC_STC
 *
 * @brief Includes the PP Rx descriptor fields, to be used for handling
 * received packets.
*/
typedef struct{

    volatile GT_U32         word1;

    volatile GT_U32         word2;

    volatile GT_U32         buffPointer;

    volatile GT_U32         nextDescPointer;

} PRV_CPSS_RX_DESC_STC;



/*
 * typedef: struct PRV_CPSS_SW_RX_DESC_STC
 *
 * Description: Sw management Rx descriptor.
 *
 * Fields:
 *      rxDesc          - Points to the Rx descriptor representing this Sw Rx
 *                        desc.
 *      swNextDesc      - A pointer to the next descriptor in the linked-list.
 *      shadowRxDesc    - A shadow struct to hold the real descriptor data
 *                        after byte swapping to save non-cacheable memory
 *                        access.
 *
 */
typedef struct PRV_CPSS_SW_RX_DESC_STCT
{
    PRV_CPSS_RX_DESC_STC      *rxDesc;
    struct PRV_CPSS_SW_RX_DESC_STCT    *swNextDesc;

    PRV_CPSS_RX_DESC_STC      shadowRxDesc;
}PRV_CPSS_SW_RX_DESC_STC;


/**
* @struct PRV_CPSS_RX_DESC_LIST_STC
 *
 * @brief The control block of a single Rx descriptors list.
*/
typedef struct{

    void *swRxDescBlock;

    PRV_CPSS_SW_RX_DESC_STC *next2Return;

    PRV_CPSS_SW_RX_DESC_STC *next2Receive;

    /** Number of free descriptors in list. */
    GT_U32 freeDescNum;

    /** Maximal number descriptors in the list */
    GT_U32 maxDescNum;

    /** @brief Number of reserved bytes before each buffer, to be
     *  kept for application and internal use.
     */
    GT_U32 headerOffset;

    /** @brief When set to GT_TRUE enabling the Rx SDMA on buffer
     *  release is forbidden.
     */
    GT_BOOL forbidQEn;

} PRV_CPSS_RX_DESC_LIST_STC;


/************************   AU Related Definitions  ***************************/

/* support running from the PSS */
#ifndef __gtCoreIntDefsh

#define AU_BLOCK_ALIGN      (16)  /* In Bytes */
#define AU_DESC_SIZE        (16)  /* In Bytes */

/* ExMxPm devices uses 32 byte alignment between AU/FU messages.
  Also baseaddresses for AUQ and FUQ must be 32 byte alignment. */
#define AU_DESC_ALIGN_EXT_5       (32)  /* In Bytes */
#define AU_DESC_SIZE_EXT_5        (32)  /* In Bytes */

/* AU descriptor related macros.    */

/* Checks if the given desc. is not valid.       */
#define AU_DESC_IS_NOT_VALID(auMsgPtr)                                     \
            (((auMsgPtr)->word0 == 0) && ((auMsgPtr)->word1 == 0) &&   \
             ((auMsgPtr)->word2 == 0) && ((auMsgPtr)->word3 == 0))


/* Invalidates the given AU desc.               */
#define AU_DESC_RESET_MAC(auMsgPtr)        \
            (auMsgPtr)->word0 = 0;         \
            (auMsgPtr)->word1 = 0;         \
            (auMsgPtr)->word2 = 0;         \
            (auMsgPtr)->word3 = 0

/* swap the 4 words of au message */
#define AU_DESC_SWAP_MAC(origAuMsgPtr,swappedAuMsg)        \
            swappedAuMsg.word0 = CPSS_32BIT_LE((origAuMsgPtr)->word0); \
            swappedAuMsg.word1 = CPSS_32BIT_LE((origAuMsgPtr)->word1); \
            swappedAuMsg.word2 = CPSS_32BIT_LE((origAuMsgPtr)->word2); \
            swappedAuMsg.word3 = CPSS_32BIT_LE((origAuMsgPtr)->word3)



/* Checks if the given desc. is not valid.
  Used for Puma devices. The Puma device has AU message with bit#0 always == 0.
  In the init of AU queue and after reading of AU the CPSS writes 0xFFFF FFFF
  to the first word of AU. The PP changes bit#0 of the first to 0 when transfers
  AU to the queue. So the first word of valid AU is NOT equal to 0xFFFF FFFF */
#define AU_DESC_IS_NOT_VALID_EXT_5(auMsgPtr)  ((auMsgPtr)->word0 == 0xFFFFFFFF)

/* Checks if the given desc. is valid. */
#define AU_DESC_IS_VALID_EXT_5(auMsgPtr)  ((auMsgPtr)->word0 != 0xFFFFFFFF)

/* Invalidates the given AU desc. Extended for 5 words. */
#define AU_DESC_RESET_EXT_5(auMsgPtr)   (auMsgPtr)->word0 = 0xFFFFFFFF;


#endif /* #ifndef __gtCoreIntDefsh */

/**
* @struct PRV_CPSS_AU_DESC_STC
 *
 * @brief Includes fields definitions of the Address Update messages sent
 * to the CPU.
*/
typedef struct{

    /** word#0 of AU descriptor */
    GT_U32 word0;

    /** word#1 of AU descriptor */
    GT_U32 word1;

    /** word#2 of AU descriptor */
    GT_U32 word2;

    /** word#3 of AU descriptor */
    GT_U32 word3;

} PRV_CPSS_AU_DESC_STC;

/**
* @struct PRV_CPSS_AU_DESC_EXT_8_STC
 *
 * @brief Includes fields definitions of the Address Update messages sent
 * to the CPU. Extended for 8 words. Relevant to Lion2 devices.
*/
typedef struct{

    /** AU/FU message data elem0 (word0 */
    PRV_CPSS_AU_DESC_STC elem0;

    /** AU/FU message data elem1 (word4 */
    PRV_CPSS_AU_DESC_STC elem1;

} PRV_CPSS_AU_DESC_EXT_8_STC;

/**
* @struct PRV_CPSS_AU_FU_MSG_SIP7_STC
 *
 * @brief Define the raw words format of the AU/FU messages sent
 * to the CPU. Extended for 16 words.
 *     the message size is one of :
 *     a.  6 words + 2 words ZERO padded
 *     b. 14 words + 2 words ZERO padded
 *
 * APPLICABLE DEVICES : AAS.
*/
typedef struct{
    /** @brief : SIP7 : AU/FU message data words 0..15
    */
    GT_U32  fdbMsgWords[16];
} PRV_CPSS_AU_FU_MSG_SIP7_STC;


/**
* @struct PRV_CPSS_AU_DESC_CTRL_STC
 *
 * @brief Address Update descriptors block control struct.
*/
typedef struct{

    /** Address of the descs. block (Virtual Address) */
    GT_UINTPTR blockAddr;

    /** Size (in descs.) of the descs. block. */
    GT_U32 blockSize;

    /** @brief Index of the next desc. to handle.
     *  auCtrlSem     - Semaphore for mutual exclusion.
     */
    GT_U32 currDescIdx;

    /** @brief relevant for DxCh3 and above devices.
     *  The CNC block uploaded
     *  to the same memory as FDB uploaded. The same 4 words
     *  record that contains FDB FU record can contain two
     *  CNC counters. The CNC block contains 2048 CNC counters
     *  and uploaded to 1024 records.
     *  This field is set to 2048 by an API triggering the CNC
     *  block upload and decremented by API retrieves the CNC
     *  counters from this Queue.
     */
    GT_U32 unreadCncCounters;

    /** @brief the mg unit that connected to this descriptor block */
    GT_U32 mgUnitId;

} PRV_CPSS_AU_DESC_CTRL_STC;

/**
* @struct PRV_CPSS_SHARED_CNC_IN_PORT_GROUPS_CTRL_STC
 *
 * @brief info about the shared CNC units between port groups control struct.
*/
typedef struct
{
    GT_U32      mgPortGroupId;
    GT_U32      currCncDescIdxInTrigger;
    GT_U32      *isMyCncUploadTriggeredBmpArr;
    GT_U32      numBitsInBmp;
    GT_PORT_GROUPS_BMP  otherSharedPortGroupsOnMyMgUnit;

    GT_U32      savedCncDescEntriesBeforeRewind;

    /* valid only on 'port group 0' --- serves as 'per device info' */
    struct{
        GT_U32      triggeredNumOfCncUploads;
        GT_U32      servedNumOfCncUploads;
        GT_BOOL     endOfFuqReached;
        GT_U32      cncPortGroupId_endOfFuqReached;
    }perMgUnit;
}PRV_CPSS_SHARED_CNC_IN_PORT_GROUPS_CTRL_STC;

#define PRV_CPSS_SHARED_CNC_IN_PORT_GROUPS_GET_MAC(/*IN*/devNum,/*IN*/cnc_portGroupId) \
    (&PRV_CPSS_PP_MAC(devNum)->intCtrl.sharedCncDescCtrl[cnc_portGroupId])
/*get bit in BMP*/
#define PRV_CPSS_IS_IN_BMP_MAC(bmpArr,bitIndex) \
    ((bmpArr[(bitIndex)>>5] & (1<<((bitIndex) & 0x1f))) ? 1:0)  /*get the bit*/

/*set/clear bit in BMP*/
#define PRV_CPSS_SET_IN_BMP_MAC(bmpArr,bitIndex,value) \
    bmpArr[(bitIndex)>>5] &= ~(1<<((bitIndex) & 0x1f));/*clear the bit*/ \
    bmpArr[(bitIndex)>>5] |= (value ? (1<<((bitIndex) & 0x1f)) : 0)/*set the bit if needed*/

/* check <triggerId> 'bit_index' into isMyCncUploadTriggeredBmpArr[] of PRV_CPSS_SHARED_CNC_IN_PORT_GROUPS_CTRL_STC */
#define PRV_CPSS_SHARED_CNC_IN_PORT_GROUPS_MY_TRIGGER_VALIDITY_CHECK_MAC(/*IN*/devNum,/*IN*/cnc_portGroupId,   \
    /*IN*/triggerId)                                                               \
    if(PRV_CPSS_SHARED_CNC_IN_PORT_GROUPS_GET_MAC(devNum,cnc_portGroupId)->numBitsInBmp <= triggerId)   \
    {                                                                                                   \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "[%s] bit index[%s] is out of range [0..%d]" ,      \
            #triggerId,                                                                                 \
            triggerId,                                                                                  \
            PRV_CPSS_SHARED_CNC_IN_PORT_GROUPS_GET_MAC(devNum,cnc_portGroupId)->numBitsInBmp);          \
    }


/* check if the bit is set in isMyCncUploadTriggeredBmpArr[] of PRV_CPSS_SHARED_CNC_IN_PORT_GROUPS_CTRL_STC */
#define PRV_CPSS_SHARED_CNC_IN_PORT_GROUPS_MY_TRIGGER_GET_MAC(/*IN*/devNum,/*IN*/cnc_portGroupId,   \
    /*IN*/triggerId,/*OUT*/isMyTrigger)                                                              \
    PRV_CPSS_SHARED_CNC_IN_PORT_GROUPS_MY_TRIGGER_VALIDITY_CHECK_MAC(devNum,cnc_portGroupId,triggerId); \
    isMyTrigger = PRV_CPSS_IS_IN_BMP_MAC(                                                           \
        PRV_CPSS_SHARED_CNC_IN_PORT_GROUPS_GET_MAC(devNum,cnc_portGroupId)->isMyCncUploadTriggeredBmpArr, \
        triggerId)

/* set the bit in isMyCncUploadTriggeredBmpArr[] of PRV_CPSS_SHARED_CNC_IN_PORT_GROUPS_CTRL_STC */
#define PRV_CPSS_SHARED_CNC_IN_PORT_GROUPS_MY_TRIGGER_SET_MAC(/*IN*/devNum,/*IN*/cnc_portGroupId,   \
    /*IN*/triggerId,/*IN*/isMyTrigger)                                                               \
    PRV_CPSS_SHARED_CNC_IN_PORT_GROUPS_MY_TRIGGER_VALIDITY_CHECK_MAC(devNum,cnc_portGroupId,triggerId); \
    PRV_CPSS_SET_IN_BMP_MAC(                                                                        \
        PRV_CPSS_SHARED_CNC_IN_PORT_GROUPS_GET_MAC(devNum,cnc_portGroupId)->isMyCncUploadTriggeredBmpArr,  \
        triggerId , isMyTrigger)

/* support running from the PSS */
#ifndef __gtCoreIntDefsh

/* macro that define the alignment needed for Tx descriptor that given to the PP
relate to packets that the CPU send to the PP*/
#define TX_DESC_ALIGN       (16)  /* In bytes */
/* macro that define the Tx "short" buffer size that given to the PP ,
this buffer needed in cases that the PP need to insert data to the packet that
the application put it the Tx buffers
relate to packets that the CPU send to the PP*/
#define TX_SHORT_BUFF_SIZE  (8)   /* Bytes    */

/* Define the Tx "short" buffer size 16 bytes,
   for CPSS to insert up to 4 words DSA tag into packets sent from CPU.
   Used for eArch devices Bobcat2, Caelum, Aldrin, AC3X, Bobcat3, Aldrin2, Falcon, AC5P, AC5X, Harrier, Ironman */
#define TX_SHORT_BUFF_SIZE_16_CNS  (16)   /* Bytes    */

/* macro that define the Tx descriptor size that given to the PP ,
relate to packets that the CPU send to the PP*/
#define TX_DESC_SIZE        (16)  /* In Bytes */

/************************************/
/* Tx descriptor related macros.    */
/************************************/

/* macro that define that CPU own the Tx descriptor --
the value in field "ownership bit" in the Tx descriptor in the PP
relate to packets that the CPU send to the PP */
#define TX_DESC_CPU_OWN     (0)
/* macro that define that DMA (PP) own the Tx descriptor --
the value in field "ownership bit" in the Tx descriptor in the PP
relate to packets that the CPU send to the PP */
#define TX_DESC_DMA_OWN     (1)


/* Resets the Tx descriptor's word1 & word2.                        */
#define TX_DESC_RESET_MAC(txDesc)                                           \
            (txDesc)->word1 = 0x0;                                      \
            (txDesc)->word2 = 0x0

/*  Make sure that all previous operations where
 *  executed before changing the own bit of the
 *  (own bit & other controls are present in word1)
 *  first descriptor.
 */
#define TX_FIRST_DESC_COPY_MAC(dstDesc,srcDesc)                         \
            (dstDesc)->buffPointer      = (srcDesc)->buffPointer;       \
            (dstDesc)->word2            = (srcDesc)->word2;             \
             GT_SYNC;                                                   \
            (dstDesc)->word1            = (srcDesc)->word1;

/* Copy a tx descriptor from one struct to another      */
#define TX_DESC_COPY_MAC(dstDesc,srcDesc)                               \
            (dstDesc)->buffPointer      = (srcDesc)->buffPointer;       \
            (dstDesc)->word2            = (srcDesc)->word2;             \
            (dstDesc)->word1            = (srcDesc)->word1;

/* Get / Set the Own bit field of the tx descriptor.    */
#define TX_DESC_GET_OWN_BIT_MAC(devNum,txDesc)                         \
            (CPSS_32BIT_LE((txDesc)->word1) >> 31)
#define TX_DESC_SET_OWN_BIT_MAC(txDesc,val)                     \
            (U32_SET_FIELD_MAC((txDesc)->word1,31,1,val))

/* Get / Set the First bit field of the tx descriptor.  */
#define TX_DESC_GET_FIRST_BIT_MAC(devNum,txDesc)                       \
            ((CPSS_32BIT_LE((txDesc)->word1) >> 21) &0x1)
#define TX_DESC_SET_FIRST_BIT_MAC(txDesc,val)                   \
            (U32_SET_FIELD_MAC((txDesc)->word1,21,1,val))


/* Get / Set the Last bit field of the tx descriptor.   */
#define TX_DESC_GET_LAST_BIT_MAC(devNum,txDesc)                        \
            ((CPSS_32BIT_LE((txDesc)->word1)>> 20) & 0x1)
#define TX_DESC_SET_LAST_BIT_MAC(txDesc,val)                    \
            (U32_SET_FIELD_MAC((txDesc)->word1,20,1,val))


/* Get / Set the Reject indication bit field of the tx descriptor.  */
#define TX_DESC_GET_REJECT_BIT_MAC(txDesc)                  \
            (((txDesc)->word1 >> 0) & 0x1)
#define TX_DESC_SET_REJECT_BIT_MAC(txDesc,val)              \
            (U32_SET_FIELD_MAC((txDesc)->word1,0,1,val))

/* Get / Set the Byte Count field in the tx descriptor.       */
#define TX_DESC_GET_BYTE_CNT_MAC(txDesc)            \
            (U32_GET_FIELD_MAC((CPSS_32BIT_LE((txDesc)->word2)),16,14))

#define TX_DESC_SET_BYTE_CNT_MAC(txDesc,val)            \
            (U32_SET_FIELD_MAC((txDesc)->word2,16,14,val))

/* Set the Vid field in the tx descriptor.              */
#define TX_DESC_SET_VID_MAC(txDesc,val)            \
            (U32_SET_FIELD_MAC((txDesc)->word1,0,12,val))

/* Get / Set the Int bit field of the tx descriptor.    */
#define TX_DESC_GET_INT_BIT(txDesc)     (((txDesc)->word1 >> 23) & 0x1)
#define TX_DESC_SET_INT_BIT_MAC(txDesc,val) \
            (U32_SET_FIELD_MAC((txDesc)->word1,23,1,val))

#endif /* #ifndef __gtCoreIntDefsh */

/**
* @struct PRV_CPSS_TX_DESC_STC
 *
 * @brief Includes the PP Tx descriptor fields, to be used for handling
 * packet transmits.
*/
typedef struct{

    volatile GT_U32         word1;

    volatile GT_U32         word2;

    volatile GT_U32         buffPointer;

    volatile GT_U32         nextDescPointer;

} PRV_CPSS_TX_DESC_STC;




/**
 * @struct PRV_CPSS_SW_TX_DESC_STC
 *
 * @brief Sw management Tx descriptor.
 *
 */
typedef struct PRV_CPSS_SW_TX_DESC_STCT
{
    /** @brief Points to the Tx descriptor representing this Sw Tx desc
     */
    PRV_CPSS_TX_DESC_STC                *txDesc;

    /** @brief A pointer to the next descriptor in the linked-list.
     */
    struct PRV_CPSS_SW_TX_DESC_STCT     *swNextDesc;

    /** @brief A pointer to a 8 or 16 bytes buffer to be used when
     * transmitting buffers of size <= 8 or size <= 16.
     */
    GT_U8                               *shortBuffer;

    /** @brief A data to be stored in gtBuf on packet transmit
     * request, and returned to user on TxBufferQueue.
     */
    GT_PTR                              userData;

    /** @brief The user event handle got from gtEventBind when
     * working in Event Request Driven mode.
     */
    GT_UINTPTR                          evReqHndl;

    /** @brief SRAM offset at which this TX descriptor is located at
     */
    GT_U32                              sramTxDescOffset;
}PRV_CPSS_SW_TX_DESC_STC;



/**
* @struct PRV_CPSS_TX_DESC_LIST_STC
 *
 * @brief The control block of a single Tx descriptors list.
*/
typedef struct{

    void *swTxDescBlock;

    /** @brief DXCH : The Tx End FIFO pool used when working in Event
     *  -Driven mode.
     *  PX : Since we only supports 'tx synchronic' we NOT need this pool.
     *  next2Free    - Points to the descriptor from which the next
     *  transmitted buffer should be freed, When receiving
     *  a TxBufferQueue interrupt.
     *  next2Feed    - Points to the descriptor to which the next transmitted
     *  packet should be attached.
     *  (This actually points to the first desc. of the packet
     *  in case of a multi desc. packet).
     */
    CPSS_BM_POOL_ID poolId;

    PRV_CPSS_SW_TX_DESC_STC *next2Free;

    PRV_CPSS_SW_TX_DESC_STC *next2Feed;

    /** Number of free descriptors in list. */
    GT_U32 freeDescNum;

    /** @brief maximum number of descriptors in list.
     *  txListSem    - Semaphore for mutual exclusion on the access to the Tx
     *  descriptors list.
     */
    GT_U32 maxDescNum;

    /** @brief Whether to free the tx core buffer, this is true
     *  whenever a packet is transferred to the CPSS with the
     *  bufCopy on (parameter of coreGetTxPacket()).
     */
    GT_BOOL freeCpssBuff;

    /** @brief the real buffer size due to descriptor alignment
     *  limitations.
     *  Relevant only to queues working as Packet Generator.
     *  firstDescPtr  - (pointer to) the first descriptor place in the chain.
     *  Relevant only to queues working as Packet Generator.
     *  freeDescPtr   - (pointer to) the next to use descriptor in the chain.
     *  When the chain is empty, the first and free point to
     *  the same place.
     *  Relevant only to queues working as Packet Generator.
     *  scratchPadPtr  - (pointer to) the scratch pad descriptor (used during
     *  descriptors and buffers manipulations for update and
     *  removal).
     *  Relevant only to queues working as Packet Generator.
     *  revPacketIdDb  - (pointer to) per queue DB to allocate packet ID during
     *  removal operation.
     *  Relevant only to queues working as Packet Generator.
     */
    GT_U32 actualBuffSize;

    PRV_CPSS_TX_DESC_STC *firstDescPtr;

    PRV_CPSS_TX_DESC_STC *freeDescPtr;

    PRV_CPSS_TX_DESC_STC *scratchPadPtr;

    GT_U32 *revPacketIdDb;

    /** @brief Indication for the last queue enable or disable
     *  operation requested by the application.
     */
    GT_BOOL userQueueEnabled;

    /** @brief number of short buffers in (array of) shortBuffers_arr
     *  shortBuffers_arr  - DXCH : not used.
     *  PX : pointer to array of short buffers , that needed
     *  for sending buffers
     *  in size of 1..8 that must be aligned to 8 bytes
     *  physical address
     */
    GT_U32 shortBuffers_number;

    GT_U8 *shortBuffers_arr;

    /** @brief SRAM address at which TX descriptors starts
     */
    GT_U32 sramTxDescStart;

    /** @brief SRAM address at which TX buffers start
     */
    GT_U32 sramTxBuffStart;

    /** @brief SRAM address at which next TX buffer can start
     */
    GT_U32 sramTxBuffOffset;

    /** @brief SRAM address at which TX buffers must end
     */
    GT_U32 sramTxBuffEnd;

    /** @brief Pointer to the current SW descriptor in use
     */
    PRV_CPSS_SW_TX_DESC_STC *currSwDesc;

} PRV_CPSS_TX_DESC_LIST_STC;



/**
* @enum PRV_CPSS_AUQ_STATE_ENT
 *
 * @brief enumeration for the states of AU queues.
 * Relevant for WA:
 * PRV_CPSS_DXCH_FER_FDB_AUQ_LEARNING_AND_SCANING_DEADLOCK_WA_E
*/
typedef enum{

    /** all AU queues are full */
    PRV_CPSS_AUQ_STATE_ALL_FULL_E,

    /** one of two AU queues is empty */
    PRV_CPSS_AUQ_STATE_ONE_EMPTY_E,

    /** all AU queues are empty */
    PRV_CPSS_AUQ_STATE_ALL_EMPTY_E

} PRV_CPSS_AUQ_STATE_ENT;

/**
* @struct PRV_CPSS_AU_DESC_CTRL_FOR_DEAD_LOCK_STC
 *
 * @brief stuct to hold parameters relate to the WA for
 * PRV_CPSS_DXCH_FER_FDB_AUQ_LEARNING_AND_SCANING_DEADLOCK_WA_E
*/
typedef struct{

    /** @brief indication that we need/no need to get
     *  AU messages from the secondary AUQ.
     */
    GT_BOOL needToCheckSecondaryAuQueue;

    /** @brief indication that the primary AUQ was rewind since last
     *  time that it was queried
     *  GT_TRUE - AUQ was rewind since last call
     *  GT_FALSE - AUQ wasn't rewind since last call
     */
    GT_BOOL auqRewind;

    /** was this port group activated for the last WA. */
    GT_BOOL activated;

    /** @brief (this flag ignored for non-multi port group device ,
     *  -- ExMxPm code not need it and DxCh ignor for non multi-port groups device)
     *  indication that this port group started the WA in function
     *  cpssDxChBrgFdbTriggerAuQueueWa and already cleared the
     *  'trigger action' bit in HW , so the functions that re-trigger
     *  the action (cpssDxChBrgFdbMacTriggerToggle , cpssDxChBrgFdbTrigActionStart)
     *  will know if current port group need to re-trigger the action or not.
     *  (this flag ignored for non-multi port group device ,
     *  -- ExMxPm code not need it and DxCh ignor for non multi-port groups device)
     */
    GT_BOOL needWaWaitForReTriggering;

    /** @brief this port group not finished the trigger action but it
     *  is not ready for for WA.
     *  when the port group triggered action but not finished
     *  when the AUQ is not full , the WA not need to start.
     *  so this is indication to skip the port group when checking
     *  if new trigger can start after starting the WA (due to
     *  other port groups)
     *  this flag is set/cleared in function cpssDxChBrgFdbTriggerAuQueueWa
     *  this flag is used by cpssDxChBrgFdbTrigActionStart
     *  this flag is MAYBE cleared by cpssDxChBrgFdbTrigActionStart
     *  (ExMxPm code -- this flag ignored)
     */
    GT_BOOL notReady;

    /** - 0 - secondary AUQ is currently active queue; 1 - secondary AUQ1 is currently active queue; */
    GT_U32 activeSecondaryAuqIndex;

    /** - enumeration for the states of secondary AU queues. */
    PRV_CPSS_AUQ_STATE_ENT secondaryState;

    /** - enumeration for the states of primary AU queues. */
    PRV_CPSS_AUQ_STATE_ENT primaryState;

} PRV_CPSS_AU_DESC_CTRL_FOR_DEAD_LOCK_STC;

/****************   General Interrupts Control Definitions  *******************/

/**
* @struct PRV_CPSS_INTERRUPT_CTRL_STC
 *
 * @brief Includes all needed definitions for interrupts handling
 * in port group level. (Rx, Tx, Address Updates,...).
*/
typedef struct
{
    PRV_CPSS_RX_DESC_LIST_STC    rxDescList[NUM_OF_SDMA_QUEUES];
    PRV_CPSS_TX_DESC_LIST_STC    txDescList[NUM_OF_SDMA_QUEUES];
    PRV_CPSS_AU_DESC_CTRL_STC    auDescCtrl[CPSS_MAX_PORT_GROUPS_CNS];
    /** @brief used for FDB upload and for CNC0,1 for CNC upload */
    PRV_CPSS_AU_DESC_CTRL_STC    fuDescCtrl[CPSS_MAX_PORT_GROUPS_CNS];
    /** @brief CNC2,3 for CNC upload connected to other MG from the 'main CNC0,1'
        that  relate to fuDescCtrl[] .
        Needed by devices with CNC2,3 (per pipe!)
        NOTE: NOT relevant to Falcon as it hold only CNC0,1 per pipe !
        APPLICABLE DEVICES: AC5P.
    */
    PRV_CPSS_AU_DESC_CTRL_STC    cnc23_fuDescCtrl[CPSS_MAX_PORT_GROUPS_CNS];
    PRV_CPSS_AU_DESC_CTRL_STC    secondaryAuDescCtrl[CPSS_MAX_PORT_GROUPS_CNS];
    PRV_CPSS_AU_DESC_CTRL_FOR_DEAD_LOCK_STC auqDeadLockWa[CPSS_MAX_PORT_GROUPS_CNS];
    GT_U32                       activeAuqIndex[CPSS_MAX_PORT_GROUPS_CNS];
    PRV_CPSS_AU_DESC_CTRL_STC    au1DescCtrl[CPSS_MAX_PORT_GROUPS_CNS];
    PRV_CPSS_AU_DESC_CTRL_STC    secondaryAu1DescCtrl[CPSS_MAX_PORT_GROUPS_CNS];
    struct{
        GT_U32      *cookie;
        GT_U32      *freeLinkedList;
        GT_U32      firstFree;
        GT_U32      generatorsTotalDesc;
    }txGeneratorPacketIdDb;

    GT_BOOL                      use_sharedCncDescCtrl;/* only BC3 */
    PRV_CPSS_SHARED_CNC_IN_PORT_GROUPS_CTRL_STC sharedCncDescCtrl[CPSS_MAX_PORT_GROUPS_CNS];

}PRV_CPSS_INTERRUPT_CTRL_STC;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssGenIntDefsh */



