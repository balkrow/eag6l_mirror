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
* @file prvCpssDxChHwGdma.h
*
* @brief Private definition for GDMA for the DXCH devices.
*
*
* @version   1
********************************************************************************
*/

#ifndef __prvCpssDxChHwGdmah
#define __prvCpssDxChHwGdmah

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIfGdma.h>

#define GDMA_UNIT_SIZE  (32 * _1K)

/* maximal number of GDMA units per Tile */
#define PRV_CPSS_MAX_GDMA_UNITS_PER_TILE_CNS                4

/* number of GDMA rings per GDMA unit */
#define PRV_CPSS_NUM_OF_GDMA_RINGS_CNS  32

#define PRV_CPSS_MAX_GDMA_MC_PROFILE_NUM_CNS                 16
#define PRV_CPSS_MAX_GDMA_PDI_SLAVE_NUM_CNS                   8
#define PRV_CPSS_MAX_GDMA_PDI_QUEUE_PROFILE_NUM_CNS           4
#define PRV_CPSS_MAX_GDMA_PDI_QUEUE_PROFILE_HEADER_NUM_CNS    8

/* short name to RXDMA unit registers */
#define PRV_DXCH_REG1_UNIT_GDMA_MAC(_devNum)   \
    PRV_CPSS_DXCH_DEV_REGS_VER1_MAC(_devNum)->CNM.GDMA


#if __WORDSIZE == 64  /* 64 bits compilation */
    /* check that address not over lower 48 bits */
    #define PRV_CPSS_GDMA_CHECK_DMA_ADDR_48_BITS_MAC(phyAddr)   \
    if (0 != (phyAddr & 0xffff000000000000L))                   \
    {                                                           \
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_RANGE, "dmaAddr[0x%lx] is over lower 48 bits"); \
    }

    /* split the phyAddr into lowAddr32Bits,highAddr32Bits */
    #define PRV_CPSS_GDMA_SPLIT_48_BITS_ADDR_MAC(/*IN*/phyAddr,/*OUT*/lowAddr32Bits,/*OUT*/highAddr32Bits) \
        lowAddr32Bits  = (GT_U32)phyAddr;                                              \
        highAddr32Bits = (GT_U32)(phyAddr>>32)


    /* split the 48 bits of : lowAddr32Bits,highAddr32Bits
       into 3 : addr_2_LSBits (0..1) , addr_32_NSBits (2..33) , addr_14_MSBits (34..47)*/
    #define PRV_CPSS_GDMA_SPLIT_2_WORDS_48_BITS_TO_3_WORDS_ADDR_MAC(/*IN*/lowAddr32Bits,/*IN*/highAddr32Bits , \
        /*OUT*/addr_2_LSBits,/*OUT*/addr_32_NSBits,/*OUT*/addr_14_MSBits)                   \
        addr_2_LSBits  = (GT_U32)(lowAddr32Bits & 0x3);                                     \
        addr_32_NSBits = (GT_U32)((lowAddr32Bits>>2) | (highAddr32Bits << 30));             \
        addr_14_MSBits = (GT_U32)((highAddr32Bits>>2) & 0x3FFF)

    /* Get the GDMA 48 bits Address field in gdmaDesc->word0,1,3.*/
    #define GDMA_DESC_GET_BUFF_ADDR_48_BITS_FIELD(gdmaDesc/*IN*/,_phyAddr/*OUT*/) \
        {                                                                 \
            GT_UINTPTR  value1 =  (gdmaDesc->word0 >> 30) & 0x3;          \
            GT_UINTPTR  value2 =   gdmaDesc->word1;                       \
            GT_UINTPTR  value3 =  (gdmaDesc->word3 >> 2) & 0x3FFF;        \
                                                                          \
            _phyAddr = value1 | value2 << 2 | value3 << 34;               \
        }

#else /* 32 bits compilation */
    /* check that address not over lower 48 bits */
    #define PRV_CPSS_GDMA_CHECK_DMA_ADDR_48_BITS_MAC(phyAddr)   phyAddr=phyAddr/*can't be more than 32 bits*/

    /* split the phyAddr into lowAddr32Bits,highAddr32Bits */
    #define PRV_CPSS_GDMA_SPLIT_48_BITS_ADDR_MAC(/*IN*/phyAddr,/*OUT*/lowAddr32Bits,/*OUT*/highAddr32Bits) \
        lowAddr32Bits  = phyAddr;                                                      \
        highAddr32Bits = 0;                                                            \
        highAddr32Bits = highAddr32Bits/* avoid compilation errors : unused-but-set-variable */

    /* split the 48 bits of : lowAddr32Bits,highAddr32Bits
       into 3 : addr_2_LSBits (0..1) , addr_32_NSBits (2..33) , addr_14_MSBits (34..47)*/
    #define PRV_CPSS_GDMA_SPLIT_2_WORDS_48_BITS_TO_3_WORDS_ADDR_MAC(/*IN*/lowAddr32Bits,/*IN*/highAddr32Bits , \
        /*OUT*/addr_2_LSBits,/*OUT*/addr_32_NSBits,/*OUT*/addr_14_MSBits)                   \
        addr_2_LSBits  = (GT_U32)(lowAddr32Bits & 0x3);                                     \
        addr_32_NSBits = (GT_U32)((lowAddr32Bits>>2) | (highAddr32Bits << 30));             \
        addr_14_MSBits = (GT_U32)((highAddr32Bits>>2) & 0x3FFF)

    /* Get the GDMA 48 bits Address field in gdmaDesc->word0,1,3.*/
    #define GDMA_DESC_GET_BUFF_ADDR_48_BITS_FIELD(gdmaDesc/*IN*/,_phyAddr/*OUT*/) \
        {                                                                 \
            GT_UINTPTR  value1 =  (gdmaDesc->word0 >> 30) & 0x3;          \
            GT_UINTPTR  value2 =   gdmaDesc->word1 & 0x3FFFFFFF;          \
                                                                          \
            _phyAddr = value1 | value2 << 2 ;                             \
        }

#endif



/* macro to validate the value of GDMA queue parameter
    isToCpu - GT_TRUE : TO CPU , GT_FALSE : FROM CPU
    globalQueue - global queue number
*/
#define PRV_CPSS_DXCH_GDMA_CHECK_SDMA_Q_MAC(_devNum,isToCpu,globalQueue)    \
    {                                                                       \
        GT_STATUS   rc;                                                     \
        GT_U32  toCpuNumQueues,fromCpuNumQueues;                            \
        rc = prvCpssDxChGdmaNetIfNumQueuesGet(devNum,&toCpuNumQueues,&fromCpuNumQueues);\
        if (rc != GT_OK)                                                    \
        {                                                                   \
            return rc;                                                      \
        }                                                                   \
        if(isToCpu == GT_TRUE && globalQueue >= toCpuNumQueues)             \
        {                                                                   \
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "globalQueue[%d] >= 'max'[%d] for TO_CPU queues", \
                globalQueue,toCpuNumQueues);                                \
        }                                                                   \
        if(isToCpu == GT_FALSE && globalQueue >= fromCpuNumQueues)          \
        {                                                                   \
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "globalQueue[%d] >= 'max'[%d] for FROM_CPU queues", \
                globalQueue,fromCpuNumQueues);                              \
        }                                                                   \
    }


/* convert globalRxQueue to : gdmaUnitId,localRxQueue */
#define PRV_CPSS_GDMA_CONVERT_RX_GLOBAL_QUEUE_TO_GDMA_UNIT_AND_LOCAL_QUEUE_MAC(/*IN*/_devNum,/*IN*/_globalRxQueue,\
        /*OUT*/_gdmaUnitId,/*OUT*/_localRxQueue)                                                                  \
    {                                                                                                             \
        PRV_CPSS_DXCH_GDMA_DB_PER_QUEUE_TO_CPU_STC  *gdmaPerQueue_toCpu_Ptr;                                      \
        gdmaPerQueue_toCpu_Ptr = &PRV_CPSS_DXCH_PP_MAC(_devNum)->gdmaInfo.gdmaPerQueue_toCpu_Arr[_globalRxQueue]; \
        _gdmaUnitId   =  gdmaPerQueue_toCpu_Ptr->gdmaUnitId;                                                      \
        _localRxQueue =  gdmaPerQueue_toCpu_Ptr->localRxQueue;                                                    \
    }

/* convert globalTxQueue to : gdmaUnitId,localTxQueue */
#define PRV_CPSS_GDMA_CONVERT_TX_GLOBAL_QUEUE_TO_GDMA_UNIT_AND_LOCAL_QUEUE_MAC(/*IN*/_devNum,/*IN*/_globalTxQueue,\
        /*OUT*/_gdmaUnitId,/*OUT*/_localTxQueue)                                                                  \
    {                                                                                                             \
        PRV_CPSS_DXCH_GDMA_DB_PER_QUEUE_FROM_CPU_STC  *gdmaPerQueue_fromCpu_Ptr;                                      \
        gdmaPerQueue_fromCpu_Ptr = &PRV_CPSS_DXCH_PP_MAC(_devNum)->gdmaInfo.gdmaPerQueue_fromCpu_Arr[_globalTxQueue]; \
        _gdmaUnitId   =  gdmaPerQueue_fromCpu_Ptr->gdmaUnitId;                                                      \
        _localTxQueue =  gdmaPerQueue_fromCpu_Ptr->localTxQueue;                                                    \
    }

/* Gets bits from word0 of the descriptor.           */
#define GDMA_GET_FROM_WORD0_MAC(gdmaDesc,startBit,mask)  \
    ((gdmaDesc->word0 >> startBit) & mask)

/* Sets bits in word0 of the descriptor.           */
#define GDMA_SET_TO_WORD0_MAC(gdmaDesc,startBit,mask,_value)  \
    /* clear the bits */                                      \
    gdmaDesc->word0 &= ~((mask) << startBit);                 \
    /* set the bits with the needed value*/                   \
    gdmaDesc->word0 |= (val & mask) << startBit


/* Gets the GDMA Own bit field of the descriptor.           */
#define GDMA_DESC_GET_OWN_BIT(gdmaDesc)                 GDMA_GET_FROM_WORD0_MAC(gdmaDesc,0/*startBit*/,0x1/*mask*/)
/* Returns the GDMA SOP/First bit field of the descriptor.  */
#define GDMA_DESC_GET_FIRST_BIT(gdmaDesc)               GDMA_GET_FROM_WORD0_MAC(gdmaDesc,6/*startBit*/,0x1/*mask*/)
/* Returns the EOP/Last bit field of the descriptor.          */
#define GDMA_DESC_GET_LAST_BIT(gdmaDesc)                GDMA_GET_FROM_WORD0_MAC(gdmaDesc,7/*startBit*/,0x1/*mask*/)
/* Get GDMA Buffer Byte Count/Size field*/
#define GDMA_DESC_GET_BYTE_COUNT_FIELD_MAC(gdmaDesc)    GDMA_GET_FROM_WORD0_MAC(gdmaDesc,14/*startBit*/,0xFFFF/*mask*/)
/* Get GDMA RX FAILURE_CODE field from the first word of an desc.
    2 bits value */
#define GDMA_DESC_RX_GET_FAILURE_CODE_FIELD(gdmaDesc)   GDMA_GET_FROM_WORD0_MAC(gdmaDesc,4/*startBit*/,0x3/*mask*/)

/**
* @struct PRV_CPSS_GDMA_DESC_STC
 *
 * @brief describe the GDMA descriptor : 4 words. (extended mode)
 *      each descriptor supports 48 bits of buffer address , in order to remove any
 *      dependency on buffer location in the memory
 *  NOTE: it is generic for PDI2MEM (TO_CPU)  ,MEM2PDI (FROM-CPU)
 *  word   start bit | num of bits  | field name
 *   0        0,         1             OWN
 *   0        1,         3             RESERVED_0
 *   0        4,         2             FAILURE_CODE (     for FROM-CPU , single bit)
 *   0        5,         1             VALID        (only for FROM-CPU)
 *   0        6,         1             F
 *   0        7,         1             L
 *   0        8,         5             AXI_ATTR_PROFILE
 *   0       13,         1             OPCODE
 *   0       14,        16             BUFFER_BYTE_COUNT
 *   0,      30,         2             BUFFER_DST_ADDR_LOW_0_1
 *   1        0,        32             BUFFER_DST_ADDR_LOW_2_33
 *   2       64,        16             RESERVED_1
 *   2       80,        16             RESERVED_2
 *   3       96,        14             BUFFER_DST_ADDR_HIGH_34_47
 *   3      110,        28             RSVD
*/
typedef struct{
    volatile GT_U32         word0;
    volatile GT_U32         word1;
    volatile GT_U32         word2;/* !!! not in use !!! hold only reserved fields */
    volatile GT_U32         word3;
} PRV_CPSS_GDMA_DESC_STC;

typedef struct{
    /* 'attach' each Tx descriptor with the 16 bytes that eDSA may hold
       CPSS for legacy SDMA did similar allocation but different management on it

       we keep it in bytes because function cpssDxChNetIfDsaTagBuild(...) work
       with bytes
    */
    GT_U8                   edsa_bytes[16];

} PRV_CPSS_GDMA_TX_EDSA_STC;


/* number of bytes in PRV_CPSS_GDMA_DESC_STC */
#define PRV_CPSS_GDMA_DESC_ALIGN    16

typedef struct{
    GT_U32  gdmaLocalFirstQueue;
    GT_U32  gdmaGlobalQueue;
    GT_U32  numOfQueues;
}PRV_CPSS_DXCH_GDMA_QUEUE_INFO_STC;

/**
* @struct PRV_CPSS_DXCH_GDMA_CPU_PORT_NETIF_INFO_STC
 *
 * @brief A Structure with info per cpu port - GDMA info
 * Notes: index by 'netIfNum'
 *
 * partial init during 'phase 1 init' : prvCpssAasInitParamsSet().
 * rest of it during cpssDxChGdmaCpuPortBindToQueuesSet(...) , when application
 *  state what queues are used by each cpu port.
*/
typedef struct{

    /** The TO_CPU may hold more than 32 queues that each GDMA unit hold
     * this value state hiw many indexes valid in
    */
    GT_U32  numGdmaUnitsUsed;

    /** The FROM_CPU may hold single or 0 GDMA units , there for we need index to
     *  gdmaUnitInfo[]
     *  GT_NA : the CPU port not used for FROM_CPU
    */
    GT_U32  index_gdmaUnitInfo_FROM_CPU;

    /** The TO_CPU may have more than single GDMA unit that serve the CPU port.
     * if numGdmaUnitsUsed > 1 and application not use more queues than served
     * by single GDMA unit , this value is the index to gdmaUnitInfo[]
     *********
     * GT_NA    : means that application not using this CPU port for TO_CPU
     * (GT_NA-1): means that application using this CPU port for TO_CPU with multiple GDMA units
     * number(0,1,2,3) : means the index in gdmaUnitInfo[]
    */
    GT_U32  index_gdmaUnitInfo_TO_CPU;

    struct{
        /* the GDMA unit that serve this CPU port */
        GT_U32  gdmaUnitId;

        PRV_CPSS_DXCH_GDMA_QUEUE_INFO_STC gdmaQueueInfo_TO_CPU;

        PRV_CPSS_DXCH_GDMA_QUEUE_INFO_STC gdmaQueueInfo_FROM_CPU;
    }gdmaUnitInfo[PRV_CPSS_MAX_GDMA_UNITS_PER_TILE_CNS];

}PRV_CPSS_DXCH_GDMA_CPU_PORT_NETIF_INFO_STC;

/* bmp of actual used rings/queues in the GDMA unit */
typedef struct{
    struct{
        GT_U32 usedQueuesBmp;
    }globalGdmaUnits[MAX_TILE_CNS*PRV_CPSS_MAX_GDMA_UNITS_PER_TILE_CNS];
}PRV_CPSS_DXCH_GDMA_ACTUAL_USED_RINGS_PER_CLIENT_STC;


typedef struct{

    /* @brief : The Virtual address (address in the DRAM/DDR) of the memory that
     *   hold the descriptors for this queue.
     *   that CPSS did 'casting' to (PRV_CPSS_GDMA_DESC_STC  *)
     ************************
     *  This value set during init from : CPSS_MULTI_NET_IF_RX_SDMA_QUEUE_STC::descMemPtr
     *  (with alignment on descMemPtr to sizeof(PRV_CPSS_GDMA_DESC_STC))
     *
     ************************
     *  In run time the manager will access :
     *  toCpu_descQueueStartVirtualAddress[toCpu_currDescriptorIndex]
    */
    PRV_CPSS_GDMA_DESC_STC  *toCpu_descQueueStartVirtualAddress;

    /* @brief : number of descriptors in the queue.
     ************************
     *  This value set during init from : CPSS_MULTI_NET_IF_RX_SDMA_QUEUE_STC::descMemSize/sizeof(PRV_CPSS_GDMA_DESC_STC)
     *  (with alignment on descMemPtr to sizeof(PRV_CPSS_GDMA_DESC_STC))
     *
     *  for mode : CPSS_RX_BUFF_NO_ALLOC_E --> see also : toCpu_numOfBuffers_for_CPSS_RX_BUFF_NO_ALLOC_E
     *
    */
    GT_U32  toCpu_numOfDescriptors;

    /* @brief : The current descriptor index in toCpu_descQueueStartVirtualAddress[].
     ************************
     *  The value of it is : 0.. (toCpu_numOfDescriptors-1) in cyclic manner.
     *  *********************
     *  The value doing ++ (with modulo 'toCpu_numOfDescriptors' meaning :
     *  x = ((x+1) % toCpu_numOfDescriptors);
     *  The '++' is done after the function 'packet get' finished handling the
     *  current descriptor at toCpu_descQueueStartVirtualAddress[toCpu_currDescriptorIndex]
     *  Regardless to 'error' or 'ok' on this descriptor handling
    */
    GT_U32  toCpu_currDescriptorIndex;

    /* @brief : number of buffers in the queue , that CPSS_RX_BUFF_NO_ALLOC_E mode
     *  to serve the descriptors.
     *  as those buffers not exists during prvCpssDxChGdmaNetworkIfToCpuInit(...)
     *
     *  as long as this value is less than toCpu_numOfDescriptors , function 'Rx packet free'
     *      will add the buffer to the next descriptor that wait for it
    */
    GT_U32  toCpu_numOfBuffers_for_CPSS_RX_BUFF_NO_ALLOC_E;

}PRV_CPSS_DXCH_GDMA_DB_PER_QUEUE_TO_CPU_DESCRIPTORS_STC;

typedef struct PRV_CPSS_DXCH_GDMA_DB_PER_QUEUE_TO_CPU_BUFFER_NODE_STCT {

    /* @brief : The physical address of the buffer (not virtual address) .
     *  this value need to be written 'as is into the SGD <BUFFER_DST_ADDR_LOW>
     *
     *  The SGD <BUFFER_DST_ADDR_LOW> hold 34 bits , so 32 bits from :
     *  bufferDmaAddr_low and 2 more bits from bufferDmaAddr_high
    */
    GT_U32 node_bufferDmaAddr_low;

    /* @brief : The additional 16 bits to support 48 bits address , as supported by
     *  addition 14 bits in SGD <BUFFER_DST_ADDR_LOW> in the 'extended' mode.
     *****************
     *  CPSS currently not supports the 'extended' mode.
     *  so using only 2 bits from this field into the 34 of <BUFFER_DST_ADDR_LOW>
    */
    GT_U32 node_bufferDmaAddr_high;

    /* @brief : number of bytes in the buffer.
       used ONLY in mode : CPSS_RX_BUFF_NO_ALLOC_E
       As for other modes all buffers are of the same size.
        (see PRV_CPSS_DXCH_GDMA_DB_PER_QUEUE_TO_CPU_BUFFERS_STC::bufferNumOfBytes)
    */
    GT_U32  node_bufferNumOfBytes;

    /* pointer to the 'next' node (of same type) , to allow 'linked list' */
    struct PRV_CPSS_DXCH_GDMA_DB_PER_QUEUE_TO_CPU_BUFFER_NODE_STCT *node_nextPtr;

}PRV_CPSS_DXCH_GDMA_DB_PER_QUEUE_TO_CPU_BUFFER_NODE_STC;


typedef struct{

    /* @brief : array of nodes that serves the 2 lists of buffers :
     *  NOTE: NOT allocated in mode : CPSS_RX_BUFF_NO_ALLOC_E
     *      as in this mode we not know it's size .
     *      and the needed nodes are allocated in run time.
     ***********
     * 1. list of 'free buffers' :
     *      a. that application released by 'free buffer' API --> added as 'last' (removed from second list from 'first')
     *         the node gets the address and size of buffer.
     *      b. that CPSS will copy info from it into a descriptor that finished
     *         handling in the 'Rx packet get' API --> removed from 'first' (move to second list as 'last')
     *         the node clear the address and size.
     * 2. list of 'free nodes'
     *      a. that the CPSS have free node after this buffer given to application
     *         by 'Rx packet get' API --> added as last (removed from first list as 'last')
     *         the node clear the address and size.
     *      b. that application released by 'free buffer' API -->
     *         removed from 'first' (move to first list as 'last')
     *         the node gets the address and size of buffer.
     *
    */
    PRV_CPSS_DXCH_GDMA_DB_PER_QUEUE_TO_CPU_BUFFER_NODE_STC  *buffersNodesArray;

    /* @brief : number of elements in the allocation of buffersNodesArray
     *  NOTE: NOT allocated in mode : CPSS_RX_BUFF_NO_ALLOC_E
     *      as in this mode we not know it's size .
     *      and the needed nodes are allocated in run time.
    */
    GT_U32 buffersNodesNumElements;

    /* @brief : start of : list of 'free buffers' (first list)
    */
    PRV_CPSS_DXCH_GDMA_DB_PER_QUEUE_TO_CPU_BUFFER_NODE_STC  *freeBuffers_startLinkdedList;
    /* @brief : end of : list of 'free buffers' (first list)
    */
    PRV_CPSS_DXCH_GDMA_DB_PER_QUEUE_TO_CPU_BUFFER_NODE_STC  *freeBuffers_lastLinkdedList;

    /* @brief : start of : list of 'free nodex' (second list)
    */
    PRV_CPSS_DXCH_GDMA_DB_PER_QUEUE_TO_CPU_BUFFER_NODE_STC  *freeNodes_startLinkdedList;
    /* @brief : end of : list of 'free nodex' (second list)
    */
    PRV_CPSS_DXCH_GDMA_DB_PER_QUEUE_TO_CPU_BUFFER_NODE_STC  *freeNodes_lastLinkdedList;

    /** @brief : The number of bytes for all buffers in this queue.
        value GT_NA means that we work in mode : CPSS_RX_BUFF_NO_ALLOC_E
        where each packet with it's own size.
            see : PRV_CPSS_DXCH_GDMA_DB_PER_QUEUE_TO_CPU_BUFFER_NODE_STC::node_bufferNumOfBytes
     */
    GT_U32  perQueue_bufferNumOfBytes;

    /** @brief Number of reserved bytes before each buffer, to be
     *  kept for application and internal use.
     *   value GT_NA means that we work in mode : CPSS_RX_BUFF_NO_ALLOC_E
     *   where the application is the one to return buffer of 'rx packet free'
     *   without manipulations by the CPSS
     *   see function internal_cpssDxChNetIfRxBufFreeWithSize
     */
    GT_U32 headerOffset;

    /** @brief : The number of bytes for LAST buffer in this queue.
        this is needed for management of 'buffers for MSG' , that when CPSS divide
        the DMA from application , to SGDs and sub-buffers , the last buffer may
        be smaller than the others.
        NOTE : value 0 means that last buffer, also uses perQueue_bufferNumOfBytes
     */
    GT_U32  perQueue_lastBufferNumOfBytes;

}PRV_CPSS_DXCH_GDMA_DB_PER_QUEUE_TO_CPU_BUFFERS_STC;

typedef struct{

    /* @brief: TO_CPU serving GDMA unit */
    GT_U32  gdmaUnitId;

    /* @brief: TO_CPU local queue in the GDMA unit */
    GT_U32  localRxQueue;

    /** @brief GT_TRUE if buffers allocated in cached CPU memory,
     *         GT_FALSE otherwise.
    */
    GT_BOOL                         buffersInCachedMem;

    /* @brief: TO_CPU info per queue about the descriptors */
    PRV_CPSS_DXCH_GDMA_DB_PER_QUEUE_TO_CPU_DESCRIPTORS_STC  toCpu_descriptors;

    /* @brief: TO_CPU info per queue about the buffers */
    PRV_CPSS_DXCH_GDMA_DB_PER_QUEUE_TO_CPU_BUFFERS_STC      toCpu_buffers;

}PRV_CPSS_DXCH_GDMA_DB_PER_QUEUE_TO_CPU_STC;

/* @brief : info per TX descriptor that we get from application during 'Tx packet send'.
 *  and CPSS save it in 'buffer from pool' (buffer from txEndFifo_poolId)
 *  during 'ISR - Tx Ended' (prvCpssDxChNetIfSdmaTxBuffQueueEvIsrHandle(...))
 *  and application can get it by calling : cpssDxChNetIfTxBufferQueueGet
 ******************
 *      userData        - A 'cookie' given by the application , so it can know what
 *                        packet did 'Tx done' .
 *      evReqHndl       - The user event handle that application got from
 *                        cpssEventBind.
 *      wasFreeByIsr    - relevant ONLY to 'last descriptor' of a packet.
 *                        indication that the ISR context free this descriptor
 *                        this is needed for case that the ISR 'see' descriptor
 *                        that was set by 'sync send' , and should tell the 'sync send'
 *                        to not free this descriptor , as it already handled it.
*/
typedef struct{
    GT_PTR              userData;
    GT_UINTPTR          evReqHndl;
    GT_BOOL             wasFreeByIsr;
}PRV_CPSS_DXCH_GDMA_DB_FROM_CPU_DESC_EXT_INFO_STC;

typedef struct{

    /* @brief : The Virtual address (address in the DRAM/DDR) of the memory that
     *   hold the descriptors for this queue.
     *   that CPSS did 'casting' to (PRV_CPSS_GDMA_DESC_STC  *)
     ************************
     *  This value set during init from : CPSS_MULTI_NET_IF_TX_SDMA_QUEUE_STC::..buffAndDescMemPtr/buffAndDescMallocFunc
     *  (with alignment on descMemPtr to sizeof(PRV_CPSS_GDMA_DESC_STC))
     *
     ************************
     *  In run time the manager will access :
     *  fromCpu_descQueueStartVirtualAddress[fromCpu_currDescriptorIndex]
     *
     ************************
     *  after fromCpu_numOfDescriptors elements in this memory , starting the 'eDSA'
     *  in format of PRV_CPSS_GDMA_TX_EDSA_STC , see fromCpu_offset_toEdsa
     *
    */
    PRV_CPSS_GDMA_DESC_STC  *fromCpu_descQueueStartVirtualAddress;

    /* @brief : the 'offset' between fromCpu_descQueueStartVirtualAddress and
     *      the 'eDsa buffers' for those descriptors , is :
     *      fromCpu_numOfDescriptors * sizeof(PRV_CPSS_GDMA_DESC_STC).
     **********
     *      the format of 'element' in this offset is : PRV_CPSS_GDMA_TX_EDSA_STC
    */
    GT_U32  fromCpu_offset_toEdsa;

    /* @brief : number of descriptors in the queue.
     ************************
     *  This value set during init from : CPSS_MULTI_NET_IF_TX_SDMA_QUEUE_STC::..buffAndDescMemPtr/buffAndDescMallocFunc
     *  (with alignment on descMemPtr to sizeof(PRV_CPSS_GDMA_DESC_STC))
     *
     *********************
     *  it is also the number of indexes in fromCpu_descExtInfoArr[]
    */
    GT_U32  fromCpu_numOfDescriptors;

    /* @brief : The current descriptor index in fromCpu_descQueueStartVirtualAddress[].
     ************************
     *  The value of it is : 0.. (fromCpu_numOfDescriptors-1) in cyclic manner.
     *  *********************
     *  The value doing ++ (with modulo 'fromCpu_numOfDescriptors' meaning :
     *  x = ((x+1) % fromCpu_numOfDescriptors);
     *  The '++' is done after the function 'packet send' finished handling the
     *  current descriptor at fromCpu_descQueueStartVirtualAddress[fromCpu_currDescriptorIndex]
     *  Regardless to 'error' or 'ok' on this descriptor handling
     *
     *********************
     *  it is also the index to fromCpu_descExtInfoArr[]
     *
     *  NOTE: must be managed only under 'ISR LOCK' , as also used (read) by the ISR context !!!
     *      see prvCpssDxChGdmaNetIfSdmaTxBuffQueueEvIsrHandle(...)
     *      so need to use : PRV_CPSS_INT_SCAN_LOCK , PRV_CPSS_INT_SCAN_UNLOCK
     *
    */
    GT_U32  fromCpu_currDescriptorIndex;


    /* @brief : The array of extra info 'per descriptor' , indexed by fromCpu_currDescriptorIndex
     *      this is 'regular' malloc memory (non-DMA memory)
    */
    PRV_CPSS_DXCH_GDMA_DB_FROM_CPU_DESC_EXT_INFO_STC *fromCpu_descExtInfoArr;



    /* @brief : number of free consecutive descriptors in the queue.
     ************************
     *  This value set during init from : fromCpu_numOfDescriptors
     *      and decremented by 'tx packet' , and incremented by 'Tx ended' ISR or
     *      by the 'sync tx packet' .
     *********************
     *
     *  NOTE: must be managed only under 'ISR LOCK' , as also updated by the ISR context !!!
     *      see prvCpssDxChGdmaNetIfSdmaTxBuffQueueEvIsrHandle(...)
     *      so need to use : PRV_CPSS_INT_SCAN_LOCK , PRV_CPSS_INT_SCAN_UNLOCK
    */
    GT_U32  fromCpu_free_numOfDescriptors;


}PRV_CPSS_DXCH_GDMA_DB_PER_QUEUE_FROM_CPU_DESCRIPTORS_STC;

typedef struct{

    /** @brief: FROM_CPU serving GDMA unit */
    GT_U32  gdmaUnitId;

    /** @brief: FROM_CPU local queue in the GDMA unit */
    GT_U32  localTxQueue;

    /** @brief:  Queue working mode. ('phase 2' info)*/
    CPSS_TX_SDMA_QUEUE_MODE_ENT     queueMode;

    /* @brief: FROM_CPU info per queue about the descriptors (and eDSA 'buffers') */
    PRV_CPSS_DXCH_GDMA_DB_PER_QUEUE_FROM_CPU_DESCRIPTORS_STC  fromCpu_descriptors;

    /** @brief : copied from : PRV_CPSS_TX_DESC_LIST_STC::poolId
     *  The Tx End FIFO pool.
     *  the pool needed for TX descriptors that the ISR context free , and their
     *  'event handle' info.
     *
     *  the nodes in this pool are of type : PRV_CPSS_SW_TX_FREE_DATA_STC
     */
    CPSS_BM_POOL_ID txEndFifo_poolId;

}PRV_CPSS_DXCH_GDMA_DB_PER_QUEUE_FROM_CPU_STC;

/** @brief the types of messages clients */
typedef enum{
    PRV_CPSS_DXCH_GDMA_CLIENT_MSG_TYPE_NOT_USED_E,

    PRV_CPSS_DXCH_GDMA_CLIENT_MSG_TYPE_FDB_AUQ_E,
    PRV_CPSS_DXCH_GDMA_CLIENT_MSG_TYPE_FDB_FUQ_E,
    PRV_CPSS_DXCH_GDMA_CLIENT_MSG_TYPE_CNC_E,
    PRV_CPSS_DXCH_GDMA_CLIENT_MSG_TYPE_IPE_4_E,
    PRV_CPSS_DXCH_GDMA_CLIENT_MSG_TYPE_IPE_8_E,
    PRV_CPSS_DXCH_GDMA_CLIENT_MSG_TYPE_IPE_16_E

}PRV_CPSS_DXCH_GDMA_CLIENT_TYPE_MSG_ENT;

typedef struct{
    /* @brief: offset from the address of descriptor to the address of it's buffer
     *  needed to save time to not do converting phy2Virt of the buffer
     *      NOTE : the offset in bytes
    */
    GT_UINTPTR  offsetFromDescToBuff;

    /* @brief: offset from the start of buffer , to the next message to serve
     *      NOTE : the offset in bytes
    */
    GT_U32      currOffsetInBuffer;

    /* @brief: the number of bytes that the buffer can hold
     *      this value is redundant with
     *          PRV_CPSS_DXCH_GDMA_DB_PER_QUEUE_TO_CPU_BUFFER_NODE_STC::node_bufferNumOfBytes
     *          but needed for MSG management
    */
    GT_U32      numBytesInBuffer;

}PRV_CPSS_DXCH_GDMA_DB_PER_MSG_DESCRIPTORS_INFO_STC;

/** @brief: sip7 max num of port groups in the device */
#define SIP7_MAX_PORT_GROUPS_UNITS   2
/** @brief: sip7 max num of CNC units per port group in the device */
#define SIP7_MAX_CNC_UNITS_PER_PORT_GROUP   4

/** @brief: sip7 max num of CNC units in the device */
#define SIP7_MAX_CNC_UNITS   (SIP7_MAX_CNC_UNITS_PER_PORT_GROUP * SIP7_MAX_PORT_GROUPS_UNITS)

typedef struct PRV_CPSS_DXCH_GDMA_DB_CNC_DUMP_STCT{
    /** @brief: info per port group to allow the 'trigger of dump' to be with
     *  some port groups , but the 'get counters' to be with other port groups
     *  once the unread counters of port group goes to zero , we remove the matching
     *  cnc unit of this port group from <node_cncUnitsBmp>.
     *
    */
    struct{
        /** @brief: The number of CNC counters that this dump still hold , and not read yet */
        GT_U32  node_unreadCncCounters;

        /** @brief: The format of the CNC counters that relate to this dump */
        GT_U32  node_cncFormat;/* one of CPSS_DXCH_CNC_COUNTER_FORMAT_ENT */
    }portGroupInfo[SIP7_MAX_PORT_GROUPS_UNITS];


    /** @brief: The bmp of CNC units that relate to this dump
     *      as triggered by cpssDxChCncPortGroupBlockUploadTrigger
     *      that called to prvCpssDxChGdmaCncBeforeDumpTrigger
    */
    GT_CNC_UNITS_BMP    node_cncUnitsBmp;

    /* @brief: info for debug and statistics */
    struct{
        GT_U32  unitNum;/* local unitNum to the port group (not global) */
    }debug_and_statistics;

    /** @brief: pointer to the next node of the same type , to allow link list
     *   those nodes allocated in run time when doing 'trigger of dump' ,
     *   and released (freed) when the 'get counters' , got the <node_unreadCncCounters>
     *   of all port groups to ZERO.
    */
    struct PRV_CPSS_DXCH_GDMA_DB_CNC_DUMP_STCT  *nextCncDumpNodePtr;
}PRV_CPSS_DXCH_GDMA_DB_CNC_DUMP_STC;


typedef struct{
    /* @brief: client MSG type */
    PRV_CPSS_DXCH_GDMA_CLIENT_TYPE_MSG_ENT clientMsgType;

    /* @brief: MSG serving GDMA unit */
    GT_U32  gdmaUnitId;

    /* @brief: MSG ring in the GDMA unit */
    GT_U32  localRing;

    /* @brief: PDI slave interface in the GDMA unit */
    GT_U32  pdiSlaveInterface;

    /* @brief: MSG (max) size in byte */
    GT_U32  msgSizeInBytes;

    /* @brief: array of ext info per descriptor
        the array allocated in runtime according to number of descriptors/buffers
        index to this array is :  toCpu_currDescriptorIndex
    */
    PRV_CPSS_DXCH_GDMA_DB_PER_MSG_DESCRIPTORS_INFO_STC  *msg_extInfoArr;

    /* @brief: MSG info per ring about the descriptors */
    PRV_CPSS_DXCH_GDMA_DB_PER_QUEUE_TO_CPU_DESCRIPTORS_STC  msg_descriptors;

    /* @brief: MSG info per queue about the buffers */
    PRV_CPSS_DXCH_GDMA_DB_PER_QUEUE_TO_CPU_BUFFERS_STC      msg_buffers;

    /* @brief: info for debug and statistics */
    struct{
        /* @brief: the number of descriptors that we finished handling
        *   (got OWNER_CPU on them and returned them to OWNER_HW)
        */
        GT_U32  numOfDescTreated;

        /* @brief: allow to trace content of messages
        */

        GT_U32  enableTraceMsg;
        /* @brief: number of messages treated
        */
        GT_U32  numOfMsgTreated;
        /* @brief: number of messages skipped
        */
        GT_U32  numOfMsgSkipped;
    }debug_and_statistics;

}PRV_CPSS_DXCH_GDMA_DB_CLIENT_MSG_STC;

typedef struct{
    /** @brief: info about the CNC manager of it's messages. */
    PRV_CPSS_DXCH_GDMA_DB_CLIENT_MSG_STC  cncInfo[SIP7_MAX_CNC_UNITS];

    /** @brief: head of the linked list , for CNC dumps (maintained for the device (not per tile/pipe)) */
    PRV_CPSS_DXCH_GDMA_DB_CNC_DUMP_STC  *cncDumpListHeadPtr;
    /** @brief: tail of the linked list , for CNC dumps (maintained for the device (not per tile/pipe)) */
    PRV_CPSS_DXCH_GDMA_DB_CNC_DUMP_STC  *cncDumpListTailPtr;

}PRV_CPSS_DXCH_GDMA_DB_CNC_STC;

/**
* @struct PRV_CPSS_DXCH_GDMA_MSG_INFO_STC
 *
 * @brief describe the GDMA metadata needed to work with the GDMAs
 *
 *
*/
typedef struct{
    /** @brief: what GDMA unit may serve the AUQ */
    GT_U32  gdmaUnitIdForAuq;
    /** @brief: what GDMA unit may serve the FUQ */
    GT_U32  gdmaUnitIdForFuq;
    /** @brief: what GDMA unit may serve the 4 CNC units */
    GT_U32  gdmaUnitIdForCnc;
    /** @brief: what GDMA unit may serve the IPE */
    GT_U32  gdmaUnitIdForIpe;

    /** @brief: info about the AUQ manager of it's messages */
    PRV_CPSS_DXCH_GDMA_DB_CLIENT_MSG_STC  gdmaAuqInfo[MAX_TILE_CNS];

    /** info about the FUQ manager of it's messages */
    PRV_CPSS_DXCH_GDMA_DB_CLIENT_MSG_STC  gdmaFuqInfo[MAX_TILE_CNS];

    /** @brief: info about the CNC manager of it's messages.
      * this client need bit more complex info than the auq/fuq ,
      * therefore with different info
    */
    PRV_CPSS_DXCH_GDMA_DB_CNC_STC         gdmaCncInfo;

    /** @brief: info about the IPE manager of it's messages.
        still need to allow use of single ring for ALL 4 CNC units (per tile) ,
            when no need to allow parallel dump between units
    */
    PRV_CPSS_DXCH_GDMA_DB_CLIENT_MSG_STC  gdmaIpeInfo[4/*IPE interfaces per unit*/ * MAX_TILE_CNS];
}PRV_CPSS_DXCH_GDMA_CLIENTS_MSG_STC;


/**
* @struct PRV_CPSS_DXCH_GDMA_INFO_STC
 *
 * @brief describe the GDMA metadata needed to work with the GDMAs
 *
 *
*/
typedef struct{
    GT_BOOL supported;/* The GM not supports the GDMA even for sip7 devices
            see macro : PRV_CPSS_DXCH_GDMA_SUPPORTED_MAC(dev) */

    /**@brief: flags to allow steps in development of the move from MG SDMA to GDMA
     *  NOTE: for GM_USED we keep using the SDMA
    */
    /**@brief: if GT_FALSE : still using the MG SDMA*/
    GT_BOOL     supportGdmaFor_AUQ;
    /**@brief: if GT_FALSE : still using the MG SDMA*/
    GT_BOOL     supportGdmaFor_FUQ_CNC;
    /**@brief: info about all the CPU ports of this device
      *     which queues used for TO_CPU/FROM_CPU
    */
    CPSS_DXCH_NETIF_GDMA_CPU_PORTS_INFO_STC    allCpuPortsInfo;

    /**@brief : array of global TO_CPU queues , indexed by global queue , allocated in
     *  runtime during init
    */
    PRV_CPSS_DXCH_GDMA_DB_PER_QUEUE_TO_CPU_STC  *gdmaPerQueue_toCpu_Arr;
    /**@brief : total number of global TO_CPU queues (number of elements in gdmaPerQueue_toCpu_Arr)
    */
    GT_U32                                      totalNumQueues_toCpu;


    /**@brief : array of global FROM_CPU queues , indexed by global queue , allocated in
     *  runtime during init
    */
    PRV_CPSS_DXCH_GDMA_DB_PER_QUEUE_FROM_CPU_STC  *gdmaPerQueue_fromCpu_Arr;
    /**@brief : total number of global FROM_CPU queues (number of elements in gdmaPerQueue_fromCpu_Arr)
    */
    GT_U32                                      totalNumQueues_fromCpu;


    GT_U32  gdmaDispatcherToUnitmap[PRV_CPSS_MAX_GDMA_UNITS_PER_TILE_CNS];/* convert the 4 values : 0..3 in <gdma_id> in :
        /Cider/EBU-IP/CNM/GDMA/GDMA DISPATCHER/GDMA Dispatcher {Current}/GDMA Dispatcher/Tables/CPU Code To GDMAQ/GDMAQ
        to the unit ID of the GDMA that supports them.

        NOTE: in AAS only 2 GDMA units connected to the 'gdma Dispatcher' (that used for packets TO CPU)
            those are GDMA1,GDMA2
        */
    /* num of queues for TO_CPU that the device have */
    GT_U32  gdmaNumQueues_TO_CPU[PRV_CPSS_MAX_GDMA_UNITS_PER_TILE_CNS];
    /* num of queues for FROM_CPU that the device have */
    GT_U32  gdmaNumQueues_FROM_CPU[PRV_CPSS_MAX_GDMA_UNITS_PER_TILE_CNS];

    /* bmp of actual used rings/queues in the GDMA unit : FROM_CPU*/
    PRV_CPSS_DXCH_GDMA_ACTUAL_USED_RINGS_PER_CLIENT_STC  gdmaUsedBmp_FROM_CPU;
    /* bmp of actual used rings/queues in the GDMA unit : TO_CPU*/
    PRV_CPSS_DXCH_GDMA_ACTUAL_USED_RINGS_PER_CLIENT_STC  gdmaUsedBmp_TO_CPU;
    /* bmp of actual used rings/queues in the GDMA unit : FDB*/
    PRV_CPSS_DXCH_GDMA_ACTUAL_USED_RINGS_PER_CLIENT_STC  gdmaUsedBmp_FDB;
    /* bmp of actual used rings/queues in the GDMA unit : CNC*/
    PRV_CPSS_DXCH_GDMA_ACTUAL_USED_RINGS_PER_CLIENT_STC  gdmaUsedBmp_CNC;
    /* bmp of actual used rings/queues in the GDMA unit : IPE*/
    PRV_CPSS_DXCH_GDMA_ACTUAL_USED_RINGS_PER_CLIENT_STC  gdmaUsedBmp_IPE;

    /** info about the GDMA(s) that serve this CPU port
      * sip7 and above
      *
      * this is extra info to PRV_CPSS_DXCH_PP_HW_INFO_CPU_PORT_INFO_STC
    */
    PRV_CPSS_DXCH_GDMA_CPU_PORT_NETIF_INFO_STC cpuPortsArr[CPSS_MAX_SDMA_CPU_PORTS_CNS];

    /**@brief : info about the clients of the MSG rings (AUQ/FUQ/CNC/IPE)
    */
    PRV_CPSS_DXCH_GDMA_CLIENTS_MSG_STC         gdmaClientsMsgInfo;

}PRV_CPSS_DXCH_GDMA_INFO_STC;


/**
* @internal prvCpssHwPpGdmaReadReg function
* @endinternal
*
* @brief   GDMA register : Read a register value from the GDMA unit.
*           - generic (non-driver) API
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; xCat3; AC5; Lion2.
*
* @param[in] devNum                   - The PP to read from.
* @param[in] gdmaNum                  - The GDMA unit to access .
* @param[in] regAddr_gdma0            - The register address in GDMA 0 !!!
*
* @param[out] data                     - Includes the register value.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssHwPpGdmaReadReg /*like prvCpssHwPpMgReadReg*/
(
    IN  GT_U8   devNum,
    IN  GT_U32  gdmaNum,
    IN  GT_U32  regAddr_gdma0,
    OUT GT_U32  *data
);
/**
* @internal prvCpssHwPpGdmaWriteReg function
* @endinternal
*
* @brief   GDMA register : Write to the GDMA unit given register.
*           - generic (non-driver) API
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; xCat3; AC5; Lion2.
*
* @param[in] devNum                   - The PP to write to.
* @param[in] gdmaNum                  - The GDMA unit to access .
* @param[in] regAddr_gdma0            - The register address in GDMA 0 !!!
* @param[in] data                     - The value to be written.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssHwPpGdmaWriteReg /*like prvCpssHwPpMgWriteReg*/
(
    IN  GT_U8   devNum,
    IN  GT_U32  gdmaNum,
    IN  GT_U32  regAddr_gdma0,
    IN  GT_U32  data
);

/**
* @internal prvCpssHwPpGdmaGetRegField function
* @endinternal
*
* @brief   GDMA register : Read a selected register field from the GDMA Unit.
*           - generic (non-driver) API
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; xCat3; AC5; Lion2.
*
* @param[in] devNum                   - The PP device number to read from.
* @param[in] gdmaNum                  - The GDMA unit to access .
* @param[in] regAddr_gdma0            - The register address in GDMA 0 !!!
* @param[in] fieldOffset              - The start bit number in the register.
* @param[in] fieldLength              - The number of bits to be read.
*
* @param[out] fieldData                - Data to read from the register.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssHwPpGdmaGetRegField
(
    IN  GT_U8   devNum,
    IN  GT_U32  gdmaNum,
    IN  GT_U32  regAddr_gdma0,
    IN  GT_U32  fieldOffset,
    IN  GT_U32  fieldLength,
    OUT GT_U32  *fieldData
);

/**
* @internal prvCpssHwPpGdmaSetRegField function
* @endinternal
*
* @brief   GDMA register : Write value to selected register field of the GDMA Unit.
*           - generic (non-driver) API
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; xCat3; AC5; Lion2.
*
* @param[in] devNum                   - The PP device number to write to.
* @param[in] gdmaNum                  - The GDMA unit to access .
* @param[in] regAddr_gdma0            - The register address in GDMA 0 !!!
* @param[in] fieldOffset              - The start bit number in the register.
* @param[in] fieldLength              - The number of bits to be written to register.
* @param[in] fieldData                - Data to be written into the register.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*
* @note this function actually read the register modifies the requested field
*       and writes the new value back to the HW.
*
*/
GT_STATUS prvCpssHwPpGdmaSetRegField
(
    IN  GT_U8   devNum,
    IN  GT_U32  gdmaNum,
    IN  GT_U32  regAddr_gdma0,
    IN  GT_U32  fieldOffset,
    IN  GT_U32  fieldLength,
    IN  GT_U32  fieldData
);

/**
* @internal prvCpssHwPpGdmaRegBusyWaitByMask function
* @endinternal
*
* @brief   GDMA register : function do 'Busy wait' on specific mask of the register.
*           - generic (non-driver) API
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; xCat3; AC5; Lion2.
*
* @param[in] devNum                   - the device number
* @param[in] gdmaNum                  - The GDMA unit to access .
* @param[in] regAddr_gdma0            - The register address in GDMA 0 !!!
* @param[in] regMask                  - the mask of the register that we wait for to clear
* @param[in] isRevert                 - indication that we wait for 'set' or wait for 'clear'
*                                       GT_TRUE  : wait for 'set'   (value 1 in the bits)
*                                       GT_FALSE : wait for 'clear' (value 0 in the bits)
* @param[in] onlyCheck                - do we want only to check the current status , or to wait
*                                      until ready
*                                      GT_TRUE - check status , without busy wait
*                                      GT_FALSE - loop on the value until value reached
*
* @retval GT_OK                    - on success.
* @retval GT_TIMEOUT               - when onlyCheck == GT_FALSE and the busy wait time expired
* @retval GT_BAD_STATE             - when onlyCheck == GT_TRUE and the value was not reached
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS prvCpssHwPpGdmaRegBusyWaitByMask
(
    IN GT_U8                            devNum,
    IN GT_U32                           gdmaNum,
    IN GT_U32                           regAddr_gdma0,
    IN GT_U32                           regMask,
    IN GT_BOOL                          isRevert,
    IN GT_BOOL                          onlyCheck
);

/**
* @internal prvCpssHwPpGdmaWriteRegBitMask function
* @endinternal
*
* @brief  GDMA register :  Writes the unmasked bits of a register.
*         - generic (non-driver) API
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; xCat3; AC5; Lion2.
*
* @param[in] devNum                   - PP device number to write to.
* @param[in] gdmaNum                  - The GDMA unit to access .
* @param[in] regAddr_gdma0            - The register address in GDMA 0 !!!
* @param[in] mask                     - Mask for selecting the written bits.
* @param[in] value                    - Data to be written to register.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
* @note The bits in value to be written are the masked bit of 'mask'.
*
*/
GT_STATUS prvCpssHwPpGdmaWriteRegBitMask
(
    IN GT_U8    devNum,
    IN GT_U32   gdmaNum,
    IN GT_U32   regAddr_gdma0,
    IN GT_U32   mask,
    IN GT_U32   value
);

/**
* @internal prvCpssDxChGdmaNetIfNumQueuesGet function
* @endinternal
*
* @brief  GDMA : the function return the number of queue for FROM_CPU,TO_CPU
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
*
* @param[in] devNum                   - The device number.
*
* @param[out] toCpu_numOfQueuesPtr    - (pointer to) the number of queues that bound for TO_CPU
*                                       ignored if NULL
* @param[out] fromCpu_numOfQueuesPtr  - (pointer to) the number of queues that bound for FROM_CPU
*                                       ignored if NULL
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device Number
* @retval GT_NOT_FOUND             - device not found in the DB
* @retval GT_NOT_APPLICABLE_DEVICE - on non-applicable device
*
*/
GT_STATUS prvCpssDxChGdmaNetIfNumQueuesGet
(
    IN GT_U8                        devNum,
    OUT GT_U32                      *toCpu_numOfQueuesPtr,
    OUT GT_U32                      *fromCpu_numOfQueuesPtr
);

/**
* @internal prvCpssDxChGdmaGlobalInit function
* @endinternal
*
* @brief  init the GDMA unit for global purposes
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Ironman.
*
* @param[in] devNum                - The device number.
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device Number
*
*/
GT_STATUS prvCpssDxChGdmaGlobalInit
(
    IN GT_U8                    devNum
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssDxChHwGdmah */

