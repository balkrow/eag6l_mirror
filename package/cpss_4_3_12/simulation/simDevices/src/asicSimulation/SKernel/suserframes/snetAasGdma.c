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
* @file snetAasGdma.c
*
* @brief Gdma processing
*
* @version   1
********************************************************************************
*/

#include <asicSimulation/SKernel/cheetahCommon/sregCheetah.h>
#include <asicSimulation/SKernel/smem/smemCheetah.h>
#include <asicSimulation/SKernel/suserframes/snetAasGdma.h>
#include <asicSimulation/SKernel/suserframes/snetCheetahEgress.h>
#include <asicSimulation/SLog/simLog.h>
#include <asicSimulation/SLog/simLogInfoTypePacket.h>
#include <asicSimulation/SKernel/suserframes/snetCheetahEq.h>

#define GDMA_PKT_GEN_MAGIC  0xcebab007

/* Shared data between MSG-creator and hadler
* Currently used by FromCpu for TX packet
*/
typedef struct {
    GT_U32                  gdma_id;
    GT_U32                  ringId;

    /* Performance optimization for HW parameters not changed on-the-fly
     * and could be saved once per queue
     */

    /* Packet Generator only */
    GT_U32                  sgdIdx;      /* per-msg copy of wru_sgd_pointer_value */
    GT_U32                  pktGenMagic; /* GDMA_PKT_GEN_MAGIC */
} GDMA_SHARED_DATA_STC;

/* SIP7 : GDMA unit : the fields of the SW RX descriptor */
#define SMEM_SIP7_GDMA_SW_GDMA_DESCRIPTOR_RX_FIELDS_NAME                    \
     STR(SMEM_SIP7_GDMA_SW_GDMA_DESCRIPTOR_RX_FIELD_OWN                    )\
    ,STR(SMEM_SIP7_GDMA_SW_GDMA_DESCRIPTOR_RX_FIELD_FAILURE_CODE           )\
    ,STR(SMEM_SIP7_GDMA_SW_GDMA_DESCRIPTOR_RX_FIELD_F_BIT                  )\
    ,STR(SMEM_SIP7_GDMA_SW_GDMA_DESCRIPTOR_RX_FIELD_L_BIT                  )\
    ,STR(SMEM_SIP7_GDMA_SW_GDMA_DESCRIPTOR_RX_FIELD_AXI_ATTR_PROFILE       )\
    ,STR(SMEM_SIP7_GDMA_SW_GDMA_DESCRIPTOR_RX_FIELD_OPCODE                 )\
    ,STR(SMEM_SIP7_GDMA_SW_GDMA_DESCRIPTOR_RX_FIELD_BUFFER_BYTE_COUNT      )\
    ,STR(SMEM_SIP7_GDMA_SW_GDMA_DESCRIPTOR_RX_FIELD_BUFFER_DST_ADDR_LOW_0  )\
    ,STR(SMEM_SIP7_GDMA_SW_GDMA_DESCRIPTOR_RX_FIELD_BUFFER_DST_ADDR_LOW_1  )\
    ,STR(SMEM_SIP7_GDMA_SW_GDMA_DESCRIPTOR_RX_FIELD_BUFFER_DST_ADDR_HIGH   )

static char * sip7GdmaSwGdmaDescriptorRxFieldsNames[
    SMEM_SIP7_GDMA_SW_GDMA_DESCRIPTOR_RX_FIELD___LAST_VALUE___E] =
    {SMEM_SIP7_GDMA_SW_GDMA_DESCRIPTOR_RX_FIELDS_NAME};

static SNET_ENTRY_FORMAT_TABLE_STC sip7GdmaSwGdmaDescriptorRxFieldsFormat[
    SMEM_SIP7_GDMA_SW_GDMA_DESCRIPTOR_RX_FIELD___LAST_VALUE___E] =
{
/*SMEM_SIP7_GDMA_SW_GDMA_DESCRIPTOR_RX_FIELD_OWN                  */
     EXPLICIT_FIELD_MAC(0 ,1 )
/*SMEM_SIP7_GDMA_SW_GDMA_DESCRIPTOR_RX_FIELD_FAILURE_CODE         */
    ,EXPLICIT_FIELD_MAC(4 ,2 )
/*SMEM_SIP7_GDMA_SW_GDMA_DESCRIPTOR_RX_FIELD_F_BIT                */
    ,EXPLICIT_FIELD_MAC(6 ,1 )
/*SMEM_SIP7_GDMA_SW_GDMA_DESCRIPTOR_RX_FIELD_L_BIT                */
    ,EXPLICIT_FIELD_MAC(7 ,1 )
/*SMEM_SIP7_GDMA_SW_GDMA_DESCRIPTOR_RX_FIELD_AXI_ATTR_PROFILE     */
    ,EXPLICIT_FIELD_MAC(8 ,5 )
/*SMEM_SIP7_GDMA_SW_GDMA_DESCRIPTOR_RX_FIELD_OPCODE               */
    ,EXPLICIT_FIELD_MAC(13,1 )
/*SMEM_SIP7_GDMA_SW_GDMA_DESCRIPTOR_RX_FIELD_BUFFER_BYTE_COUNT    */
    ,EXPLICIT_FIELD_MAC(14,16)
/*SMEM_SIP7_GDMA_SW_GDMA_DESCRIPTOR_RX_FIELD_BUFFER_DST_ADDR_LOW_0 */
    ,EXPLICIT_FIELD_MAC(30,32)
/*SMEM_SIP7_GDMA_SW_GDMA_DESCRIPTOR_RX_FIELD_BUFFER_DST_ADDR_LOW_1 */
    ,EXPLICIT_FIELD_MAC(62,2)
/*SMEM_SIP7_GDMA_SW_GDMA_DESCRIPTOR_RX_FIELD_BUFFER_DST_ADDR_HIGH */
    ,EXPLICIT_FIELD_MAC(96,14)
};

/* SIP7 : GDMA unit : the fields of the SW TX descriptor */
#define SMEM_SIP7_GDMA_SW_GDMA_DESCRIPTOR_TX_FIELDS_NAME                    \
     STR(SMEM_SIP7_GDMA_SW_GDMA_DESCRIPTOR_TX_FIELD_OWN                    )\
    ,STR(SMEM_SIP7_GDMA_SW_GDMA_DESCRIPTOR_TX_FIELD_FAILURE_CODE           )\
    ,STR(SMEM_SIP7_GDMA_SW_GDMA_DESCRIPTOR_TX_VALID                        )\
    ,STR(SMEM_SIP7_GDMA_SW_GDMA_DESCRIPTOR_TX_FIELD_F_BIT                  )\
    ,STR(SMEM_SIP7_GDMA_SW_GDMA_DESCRIPTOR_TX_FIELD_L_BIT                  )\
    ,STR(SMEM_SIP7_GDMA_SW_GDMA_DESCRIPTOR_TX_FIELD_AXI_ATTR_PROFILE       )\
    ,STR(SMEM_SIP7_GDMA_SW_GDMA_DESCRIPTOR_TX_FIELD_OPCODE                 )\
    ,STR(SMEM_SIP7_GDMA_SW_GDMA_DESCRIPTOR_TX_FIELD_BUFFER_BYTE_COUNT      )\
    ,STR(SMEM_SIP7_GDMA_SW_GDMA_DESCRIPTOR_TX_FIELD_BUFFER_DST_ADDR_LOW_0  )\
    ,STR(SMEM_SIP7_GDMA_SW_GDMA_DESCRIPTOR_TX_FIELD_BUFFER_DST_ADDR_LOW_1  )\
    ,STR(SMEM_SIP7_GDMA_SW_GDMA_DESCRIPTOR_TX_FIELD_BUFFER_DST_ADDR_HIGH   )

static char * sip7GdmaSwGdmaDescriptorTxFieldsNames[
    SMEM_SIP7_GDMA_SW_GDMA_DESCRIPTOR_TX_FIELD___LAST_VALUE___E] =
    {SMEM_SIP7_GDMA_SW_GDMA_DESCRIPTOR_TX_FIELDS_NAME};

static SNET_ENTRY_FORMAT_TABLE_STC sip7GdmaSwGdmaDescriptorTxFieldsFormat[
    SMEM_SIP7_GDMA_SW_GDMA_DESCRIPTOR_TX_FIELD___LAST_VALUE___E] =
{
/*SMEM_SIP7_GDMA_SW_GDMA_DESCRIPTOR_TX_FIELD_OWN                  */
     EXPLICIT_FIELD_MAC(0 ,1 )
/*SMEM_SIP7_GDMA_SW_GDMA_DESCRIPTOR_TX_FIELD_FAILURE_CODE         */
    ,EXPLICIT_FIELD_MAC(4 ,1 )
/*SMEM_SIP7_GDMA_SW_GDMA_DESCRIPTOR_TX_VALID                      */
    ,EXPLICIT_FIELD_MAC(5 ,1 )
/*SMEM_SIP7_GDMA_SW_GDMA_DESCRIPTOR_TX_FIELD_F_BIT                */
    ,EXPLICIT_FIELD_MAC(6 ,1 )
/*SMEM_SIP7_GDMA_SW_GDMA_DESCRIPTOR_TX_FIELD_L_BIT                */
    ,EXPLICIT_FIELD_MAC(7 ,1 )
/*SMEM_SIP7_GDMA_SW_GDMA_DESCRIPTOR_TX_FIELD_AXI_ATTR_PROFILE     */
    ,EXPLICIT_FIELD_MAC(8 ,5 )
/*SMEM_SIP7_GDMA_SW_GDMA_DESCRIPTOR_TX_FIELD_OPCODE               */
    ,EXPLICIT_FIELD_MAC(13,1 )
/*SMEM_SIP7_GDMA_SW_GDMA_DESCRIPTOR_TX_FIELD_BUFFER_BYTE_COUNT    */
    ,EXPLICIT_FIELD_MAC(14,16)
/*SMEM_SIP7_GDMA_SW_GDMA_DESCRIPTOR_TX_FIELD_BUFFER_DST_ADDR_LOW_0 */
    ,EXPLICIT_FIELD_MAC(30,32)
/*SMEM_SIP7_GDMA_SW_GDMA_DESCRIPTOR_TX_FIELD_BUFFER_DST_ADDR_LOW_1 */
    ,EXPLICIT_FIELD_MAC(62,2)
/*SMEM_SIP7_GDMA_SW_GDMA_DESCRIPTOR_TX_FIELD_BUFFER_DST_ADDR_HIGH */
    ,EXPLICIT_FIELD_MAC(96,14)
};

/**
* @internal snetGdmaTablesFormatInit function
* @endinternal
*
* @brief   init the format of GDMA tables.
*
* @param[in] devObjPtr                - pointer to device object.
*/
void snetGdmaTablesFormatInit(
    IN SKERNEL_DEVICE_OBJECT            * devObjPtr
)
{
    LION3_TABLES_FORMAT_INIT_MAC(devObjPtr, SKERNEL_TABLE_FORMAT_GDMA_SW_GDMA_DESCRIPTOR_RX_E,
        sip7GdmaSwGdmaDescriptorRxFieldsFormat, sip7GdmaSwGdmaDescriptorRxFieldsNames);

    LION3_TABLES_FORMAT_INIT_MAC(devObjPtr, SKERNEL_TABLE_FORMAT_GDMA_SW_GDMA_DESCRIPTOR_TX_E,
        sip7GdmaSwGdmaDescriptorTxFieldsFormat, sip7GdmaSwGdmaDescriptorTxFieldsNames);
}


/* Array to store the current descriptor position for GDMA rings
   Need to scale to required number of GDMA's and interfaces*/
GT_U32 currentDescPos[32] = {0};

enum {REGULAR=0,EXTENDED=1};
enum {DROP=0,WAIT=1};
enum {OWNER_CPU=0,OWNER_HW=1};
enum {OPCODE_regular_SGD = 0 , opcode_SGD_write_reg = 1};
enum {RING_BUSY=0,RING_IDLE=1};

typedef struct{
    GT_U32  words[2];
} GDMA_DESC_REGULARE;
typedef struct{
    GDMA_DESC_REGULARE  regulare;
    GT_U32  words[2];
} GDMA_DESC_EXTENDED;

enum {NO_FAILURE = 0 , LOR_FAILURE = 1 , SER_FAILURE = 2 , NO_EOP_FAILURE = 3};

typedef enum{
    /*GDMA_Ring_cause_sum = 1<<0,*/
    m2m_sgd_done_int            = 1<<1,
    lor_int                     = 1<<2,
    no_sgd_available_int        = 1<<3,
    pkt_done_int                = 1<<4,
    pkt_drop_int                = 1<<5,
    msg_done_int                = 1<<6,
    msg_drop_int                = 1<<7,
    tx_sgd_done_int             = 1<<8,
    rx_sgd_done_int             = 1<<9,
    r_chain_done_int            = 1<<10,
    no_eop_pkt_frag_int         = 1<<11,

}INTERRUPT_BMP_TYPE_ENT;

/**
* @internal gdmaTimestampTrace function
* @endinternal
*
* @brief   __LOG trace with timestamp
*
* @ note   By default Timestamp is disabled since osTimeRT latency,
*          and log with no timestamp used.
*          To enable timestamp set the GDMA_PKTGEN_TIMESTAMP_TRACE_ENA > 0
*/
static void gdmaTimestampTrace
(
    IN SKERNEL_DEVICE_OBJECT   *devObjPtr,
    IN char                    *idStr,
    IN GT_U32                  queue,
    IN GT_U32                  sgdIdx
)
{
#define GDMA_PKTGEN_TIMESTAMP_TRACE_ENA   0 /* set 1 or 2 to enable */
#if (GDMA_PKTGEN_TIMESTAMP_TRACE_ENA == 0)
    GT_UNUSED_PARAM(devObjPtr);
    GT_UNUSED_PARAM(idStr);
    GT_UNUSED_PARAM(queue);
    GT_UNUSED_PARAM(sgdIdx);
#else
    DECLARE_FUNC_NAME(gdmaTimestampTrace);

    extern GT_STATUS osTimeRT(OUT GT_U32 *seconds, OUT GT_U32 *nanoSeconds);
    GT_U32  seconds, nanoSeconds;

    osTimeRT(&seconds, &nanoSeconds);
    __LOG(("%s: q[%d]/%-3d  ts: %2u.%06u sec\n",
           idStr, queue, sgdIdx, seconds & 63, nanoSeconds/1000));
#if (GDMA_PKTGEN_TIMESTAMP_TRACE_ENA == 2)
    simulationPrintf("%s: q[%d]/%-3d  ts: %2u.%06u sec\n",
           idStr, queue, sgdIdx, seconds & 63, nanoSeconds/1000);
#endif
#endif
}

/**
* @internal gdmaGenInterrupt function
* @endinternal
*
* @brief   Generate interrupt for GDMA buffer .
*
* @param[in] devObjPtr                - pointer to device.
* @param[in] gdma_id                  - the GDMA instance id.
*                                       relative to the CnM instance
*                                       in multi CnM instances , this is not 'global' GDMA instance id.
* @param[in] ringId                   - the GDMA ring-id to use.
* @param[in] intType_bmp              - bmp of needed interrupts to assert
*/
static void gdmaGenInterrupt
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                  gdma_id,
    IN GT_U32                  ringId,
    IN INTERRUPT_BMP_TYPE_ENT  intType_bmp
)
{
    /* this interrupt in MG should go to all interrupt trees of the device */
    snetChetahDoInterrupt(devObjPtr,
                           SMEM_SIP7_GDMA_RING_INT_CAUSE_REG(devObjPtr,gdma_id,ringId),
                           SMEM_SIP7_GDMA_RING_INT_MASK_REG(devObjPtr,gdma_id,ringId),
                           (GT_U32)intType_bmp,
                           0/*not care*/);
}

/**
* @internal gdmaInc64BitCounter function
* @endinternal
*
* @brief   increment 64 bits counter by needed value
*
* @param[in] devObjPtr                - pointer to device.
* @param[in] incValue                 - number to increase the counter
*/
static void gdmaInc64BitCounter
(
    IN GT_U32                  *memPtr,
    IN GT_U32                  incValue
)
{
    memPtr[0] += incValue;
    if(memPtr[0] < incValue)
    {
        memPtr[1] ++;
    }
}

/**
* @internal gdmaSendPacketToCpu function
* @endinternal
*
* @brief   GDMA : Send the packet to the CPU
*
* @param[in] devObjPtr                - pointer to device.
* @param[in] gdma_id                  - the GDMA instance id.
*                                       relative to the CnM instance
*                                       in multi CnM instances , this is not 'global' GDMA instance id.
* @param[in] ringId                   - the GDMA ring-id to use.
* @param[in] cpuPortDma               - the DMA number of the CPU port (for __LOG info)
* @param[in] dataPtr                  - pointer to data
* @param[in] dataSize                 - size of data (dataPtr)
*                                       void
*/
static void gdmaSendPacketToCpu
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                  gdma_id,
    IN GT_U32                  ringId,
    IN GT_U32                  cpuPortDma,
    IN GT_U8                 * dataPtr,
    IN GT_U32                  dataSize
)
{
    DECLARE_FUNC_NAME(gdmaSendPacketToCpu);

    GT_U32  buffLen;                    /* Length of the current buffer */
    GT_U32  sentBytes = 0;              /* Number of bytes already sent */
    GT_U32  regAddress;                 /* Register address */
    GT_U32  *counterPtr;                /* counter pointer*/
    GT_U32  fldValue;                   /* Register field value */
    INTERRUPT_BMP_TYPE_ENT  intType_bmp = 0;
    GDMA_DESC_EXTENDED curDescrInfo;    /* RX descriptor */
    GT_U32      *rxDesc_Ptr;            /* Pointer to RX descriptor */
    GT_U32      currentDmaAddr[2];      /* Current RX descriptor address */
    GT_U32      startDmaAddr[2];
    GT_U32      ringEnable;
    GT_U32      q_type,sgd_recycle_chain,sgd_extend_mode,q_wrr_weight;
    GT_U32      wru_sgd_pointer_value;
    GT_U32      SGD_Chain_Size;
    GT_U32      free_sgd_num;
    GT_U32      rx_buf_bc_mode,rx_buf_bc;
    GT_U32      dest_addr_high;
    GT_U32      temp_address[2];
    GT_U32      ownBit;
    GT_U32      lor_error = 0;
    GT_U32      sizeof_sgd; /* size of the SGD in bytes */
    GT_U32      opcode;
    GT_U32      lor_mode;
    GT_U32      numWaitIterations = 0;

    SCIB_SEM_TAKE;
    smemRegFldGet(devObjPtr,SMEM_SIP7_GDMA_QUEUE_EN_REG(devObjPtr,gdma_id,ringId), 0,1,&ringEnable);
    if (ringEnable == 0)
    {
        SCIB_SEM_SIGNAL;

        __LOG(("GDMA to CPU : ringId[%d] is disabled (so not send packet to CPU) \n",
            ringId));
        return;
    }

    /* state that <Ring is busy> */
    smemRegFldSet(devObjPtr,SMEM_SIP7_GDMA_QUEUE_EN_REG(devObjPtr,gdma_id,ringId), 1,1,RING_BUSY);
    SCIB_SEM_SIGNAL;

    regAddress = SMEM_SIP7_GDMA_CHAIN_ADDRESS_LOW_REG(devObjPtr,gdma_id, ringId);
    smemRegGet(devObjPtr, regAddress,  &startDmaAddr[0]);
    regAddress = SMEM_SIP7_GDMA_CHAIN_ADDRESS_HIGH_REG(devObjPtr,gdma_id, ringId);
    smemRegGet(devObjPtr, regAddress,  &startDmaAddr[1]);

    /* Get the descriptor size regular mode is 8B and extended mode is 16B*/
    regAddress = SMEM_SIP7_GDMA_QUEUE_CONTROL_REG(devObjPtr,gdma_id, ringId);
    smemRegGet(devObjPtr, regAddress , &fldValue );
    sgd_recycle_chain = SMEM_U32_GET_FIELD(fldValue, 4, 1);
    sgd_extend_mode = SMEM_U32_GET_FIELD(fldValue, 5, 1);
    q_type = SMEM_U32_GET_FIELD(fldValue, 0, 4);
    q_wrr_weight = SMEM_U32_GET_FIELD(fldValue, 20, 4);

    if(q_wrr_weight == 0)
    {
        /* behave just as 'ring disable' */
        __LOG(("ERROR : GDMA to CPU : q_wrr_weight == 0 --> act as ring disabled (so not send packet to CPU) \n"));
        return;
    }

    lor_mode = SMEM_U32_GET_FIELD(fldValue, 6, 1);
    if(lor_mode == 1)
    {
        __LOG(("LOR mode is 'WAIT' \n"));
    }
    else
    {
        __LOG(("LOR mode is 'DROP' \n"));
    }

    sizeof_sgd = (sgd_extend_mode == EXTENDED) ? 16 : 8;

    if(TO_CPU_PKT_MODE_RING_TYPE != q_type)
    {
        __LOG(("ERROR : GDMA to CPU : ringId[%d] q_type[%d] is NOT for 'TO_CPU_PKT_MODE' (so not send packet to CPU) \n",
            ringId,q_type));
        return;
    }

    /* this is NOT counter , but config for 'counting' */
    regAddress = SMEM_SIP7_GDMA_RX_BUF_BYTE_CNT_REG(devObjPtr,gdma_id, ringId);
    smemRegGet(devObjPtr, regAddress , &fldValue );
    rx_buf_bc_mode = SMEM_U32_GET_FIELD(fldValue, 16,  1);
    rx_buf_bc      = SMEM_U32_GET_FIELD(fldValue,  0, 16);

    __LOG_PARAM(rx_buf_bc_mode);
    __LOG_PARAM(rx_buf_bc);

    /* Start from first descriptor address */
    regAddress = SMEM_SIP7_GDMA_BASE_DESC_POINTER_REG(devObjPtr,gdma_id, ringId);
    smemRegGet(devObjPtr, regAddress , &fldValue );
    wru_sgd_pointer_value = SMEM_U32_GET_FIELD(fldValue, 0, 16);

    __LOG_PARAM(wru_sgd_pointer_value);



    /* Calculate the currentDescp address based on current Descriptor position*/
    currentDmaAddr[0] = startDmaAddr[0] + wru_sgd_pointer_value * sizeof_sgd;
    currentDmaAddr[1] = startDmaAddr[1];
    if(currentDmaAddr[0] < startDmaAddr[0])
    {
        /* wrap around of the address 32LSBits */
        currentDmaAddr[1]++;
    }

    if(sgd_extend_mode == REGULAR)
    {
        regAddress = SMEM_SIP7_GDMA_PAYLOAD_BUFFER_ADDRESS_HIGH_REG(devObjPtr,gdma_id, ringId);
        smemRegGet(devObjPtr, regAddress , &fldValue );
        dest_addr_high = SMEM_U32_GET_FIELD(fldValue, 16, 14);/* for bits 34..47 of the address */
    }
    else
    {
        dest_addr_high = 0;/* the info is in the SGD */
    }

    rxDesc_Ptr = &curDescrInfo.regulare.words[0];

    regAddress = SMEM_SIP7_GDMA_CHAIN_SIZE_REG(devObjPtr,gdma_id, ringId);
    smemRegGet(devObjPtr, regAddress , &fldValue );
    SGD_Chain_Size = SMEM_U32_GET_FIELD(fldValue,  0, 16);

    if(wru_sgd_pointer_value >= SGD_Chain_Size)
    {
        __LOG(("ERROR : GDMA to CPU : ringId[%d] wru_sgd_pointer_value[%d] >= SGD_Chain_Size[%d] (so not send packet to CPU) \n",
            ringId,q_type,
            wru_sgd_pointer_value,SGD_Chain_Size));
        return;
    }

    regAddress = SMEM_SIP7_GDMA_FREE_SGDS_STATUS_REG(devObjPtr,gdma_id, ringId);
    smemRegGet(devObjPtr, regAddress , &fldValue );
    free_sgd_num = SMEM_U32_GET_FIELD(fldValue,  0, 16);

    __LOG_PARAM(free_sgd_num);

    /* Fill descriptors if there is a data to send */
    while (1)
    {
        if(free_sgd_num == 0)
        {
            if(lor_mode)
            {
                while(free_sgd_num == 0)
                {
                    /* WAIT */
                    /* let the Application time to process the packets , so the list will be free */
                    SIM_OS_MAC(simOsSleep)(1);
                    regAddress = SMEM_SIP7_GDMA_FREE_SGDS_STATUS_REG(devObjPtr,gdma_id, ringId);
                    smemRegGet(devObjPtr, regAddress , &fldValue );
                    free_sgd_num = SMEM_U32_GET_FIELD(fldValue,  0, 16);

                    numWaitIterations++;

                    if((numWaitIterations % 1000) == 999)
                    {
                        printf("iteration[%d] : waited for additional 1000 millisec for a new free SGD (why CPSS/application not free descriptors)\n",
                            (numWaitIterations+1));
                    }
                }
            }
            else
            {
                /* we not update any descriptor */
                lor_error = 1;
                __LOG(("we are done with this frame , as no more free descriptors left in the ring \n"));
                break;
            }
        }

        /* Read the DMA for needed value, put data into current descriptor */
        __LOG(("Read the DMA for needed value, put data into current descriptor"));
        snetChtPerformScibDmaRead_64BitAddr(SNET_CHT_DMA_CLIENT_PACKET_TO_CPU_E,gdma_id,
                    devObjPtr->deviceId,currentDmaAddr[0],currentDmaAddr[1],
                    NUM_BYTES_TO_WORDS(sizeof_sgd),
                    rxDesc_Ptr, SCIB_DMA_WORDS);

        ownBit =
            GDMA_RX(devObjPtr,rxDesc_Ptr,wru_sgd_pointer_value,
                SMEM_SIP7_GDMA_SW_GDMA_DESCRIPTOR_RX_FIELD_OWN);

        if(ownBit == OWNER_CPU)/* for the TO_CPU the HW ignore the read value of ownBit */
        {
            __LOG(("NOTE : TO_CPU : the ownBit is 'OWNER_CPU' but the HW will treat it anyway (as owned by HW) \n"));
        }

        opcode =
            GDMA_RX(devObjPtr,rxDesc_Ptr,wru_sgd_pointer_value,
                SMEM_SIP7_GDMA_SW_GDMA_DESCRIPTOR_RX_FIELD_OPCODE);

        if(opcode != 0)
        {
            /* was not implemented on the design (on real HW) */
            __LOG(("opcode == 1 was not implemented (so the value is ignored) \n"));
        }

        if(rx_buf_bc_mode == 1)
        {
            buffLen = rx_buf_bc;
        }
        else
        {
            buffLen =
                GDMA_RX(devObjPtr,rxDesc_Ptr,wru_sgd_pointer_value,
                    SMEM_SIP7_GDMA_SW_GDMA_DESCRIPTOR_RX_FIELD_BUFFER_BYTE_COUNT);
        }
        buffLen += 1;

        /* Trim buffer size if data size less than descriptor buffer size */
        buffLen = (buffLen > (dataSize - sentBytes)) ?
                            (dataSize - sentBytes) : buffLen;

        temp_address[0] =
            GDMA_RX(devObjPtr,rxDesc_Ptr,wru_sgd_pointer_value,
                SMEM_SIP7_GDMA_SW_GDMA_DESCRIPTOR_RX_FIELD_BUFFER_DST_ADDR_LOW_0);
        temp_address[1] =
            GDMA_RX(devObjPtr,rxDesc_Ptr,wru_sgd_pointer_value,
                SMEM_SIP7_GDMA_SW_GDMA_DESCRIPTOR_RX_FIELD_BUFFER_DST_ADDR_LOW_1);

        if(sgd_extend_mode == EXTENDED)
        {
            dest_addr_high = GDMA_RX(devObjPtr,rxDesc_Ptr,wru_sgd_pointer_value,
                SMEM_SIP7_GDMA_SW_GDMA_DESCRIPTOR_RX_FIELD_BUFFER_DST_ADDR_HIGH);
        }
        temp_address[1] |= dest_addr_high << 2;

        if(temp_address[0] & 0xF)
        {
            /* Since TO_CPU works in 16B alignment */
            __LOG(("config ERROR : the buffer address [0x%8.8x] is not aligned on 16 bytes , so will be considered as address [0x%8.8x] \n",
                temp_address[0], temp_address[0] & 0xFFFFFFF0));

            temp_address[0] &= 0xFFFFFFF0;
        }

        /* Write data into the DMA buffer */
        snetChtPerformScibDmaWrite_64BitAddr(SNET_CHT_DMA_CLIENT_PACKET_TO_CPU_E,gdma_id,
                    devObjPtr->deviceId,temp_address[0/*low*/],temp_address[1/*high*/],
                     NUM_BYTES_TO_WORDS(buffLen),
                     (GT_U32*)&(dataPtr[sentBytes]), TX_BYTE_ORDER_MAC(devObjPtr));

        if (sentBytes == 0)
        {
            /* set First bit */
            GDMA_RX_SET(devObjPtr,rxDesc_Ptr,wru_sgd_pointer_value,
                SMEM_SIP7_GDMA_SW_GDMA_DESCRIPTOR_RX_FIELD_F_BIT,
                1);
        }

        /* update the actual number of bytes in the buffer */
        GDMA_RX_SET(devObjPtr,rxDesc_Ptr,wru_sgd_pointer_value,
            SMEM_SIP7_GDMA_SW_GDMA_DESCRIPTOR_RX_FIELD_BUFFER_BYTE_COUNT,
            buffLen - 1);

        /* Increment send bytes counter */
        sentBytes += buffLen;

        if(wru_sgd_pointer_value >= SGD_Chain_Size &&
            sentBytes != dataSize)
        {
            GDMA_RX_SET(devObjPtr,rxDesc_Ptr,wru_sgd_pointer_value,
                SMEM_SIP7_GDMA_SW_GDMA_DESCRIPTOR_RX_FIELD_FAILURE_CODE,
                LOR_FAILURE);

            lor_error = 1;
        }

        if (sentBytes == dataSize)
        {
            /* set Last bit */
            GDMA_RX_SET(devObjPtr,rxDesc_Ptr,wru_sgd_pointer_value,
                SMEM_SIP7_GDMA_SW_GDMA_DESCRIPTOR_RX_FIELD_L_BIT,
                1);
        }

        GDMA_RX_SET(devObjPtr,rxDesc_Ptr,wru_sgd_pointer_value,
            SMEM_SIP7_GDMA_SW_GDMA_DESCRIPTOR_RX_FIELD_OWN,
            OWNER_CPU);

        /* only word 0 hold info that the device may have changed */
        __LOG(("Write back the the first word of current descriptor \n"));
        snetChtPerformScibDmaWrite_64BitAddr(SNET_CHT_DMA_CLIENT_PACKET_TO_CPU_E,gdma_id,
                    devObjPtr->deviceId,currentDmaAddr[0],currentDmaAddr[1],
                    1/*single word*/,
                    rxDesc_Ptr, SCIB_DMA_WORDS);
        /* decrement the 'free_sgd_num' (but make sure to SCIB lock the operation)
           as the active memory may want to update it */
        SCIB_SEM_TAKE;
        regAddress = SMEM_SIP7_GDMA_FREE_SGDS_STATUS_REG(devObjPtr,gdma_id, ringId);
        /* we read it under LOCK , as maybe changed by active memory after we unlocked the SCIB*/
        smemRegGet(devObjPtr, regAddress , &fldValue );
        free_sgd_num = SMEM_U32_GET_FIELD(fldValue,  0, 16);
        if(free_sgd_num == 0)
        {
            /* should not happen */
            skernelFatalError("gdmaSendPacketToCpu: free_sgd_num == 0 when need to decrement --> bad management \n");
        }
        else
        {
            free_sgd_num --;
            __LOG_PARAM(free_sgd_num);
        }

        smemRegFldSet(devObjPtr, regAddress , 0, 16, free_sgd_num);
        SCIB_SEM_SIGNAL;

        __LOG(("invoke interrupt about SGD Done (rx_sgd_done_int) \n"));
        intType_bmp |= rx_sgd_done_int;

        /* move the address of descriptor to the next descriptor */
        currentDmaAddr[0] += sizeof_sgd;
        if(currentDmaAddr[0] < sizeof_sgd)
        {
            /* wrap around of the address 32LSBits */
            currentDmaAddr[1]++;
        }

        /* update the current pointer of the ring */
        wru_sgd_pointer_value++;
        if(wru_sgd_pointer_value >= SGD_Chain_Size)
        {
            /* restart for next time */
            wru_sgd_pointer_value = 0;
            __LOG_PARAM(wru_sgd_pointer_value);

            /* wrap also the DMA of the start of the queue */
            currentDmaAddr[0] = startDmaAddr[0];
            currentDmaAddr[1] = startDmaAddr[1];
        }
        __LOG_PARAM(wru_sgd_pointer_value);

        if(lor_error)
        {
            /* we are done with this frame , as we got LOR */
            __LOG(("we are done with this frame , as we got LOR \n"));
            break;
        }

        if (sentBytes == dataSize)
        {
            /* we are done with this frame bytes */
            __LOG(("we are done with this frame bytes \n"));
            break;
        }
        else if(sentBytes > dataSize)
        {
            /* should not happen */
            skernelFatalError("gdmaSendPacketToCpu: bad management on byte send \n");
        }
        if(wru_sgd_pointer_value == 0 &&
           sgd_recycle_chain == 0)
        {
            /* not allow new SGD iteration */
            break;
        }
    }

    if(wru_sgd_pointer_value == 0 &&
       sgd_recycle_chain == 0)
    {
        __LOG(("invoke interrupt about 'Packet TO CPU' Ring Chain Done (r_chain_done_int) \n"));
        intType_bmp |= r_chain_done_int;
        __LOG(("'open chain' mode (non-cyclic) : chain ended , so disabling the Ring \n"));
        ringEnable = 0;
        smemRegFldSet(devObjPtr,SMEM_SIP7_GDMA_QUEUE_EN_REG(devObjPtr,gdma_id,ringId), 0,1,ringEnable);
    }

    /* Update the next descriptor to handle */
    regAddress = SMEM_SIP7_GDMA_BASE_DESC_POINTER_REG(devObjPtr,gdma_id, ringId);
    SMEM_U32_SET_FIELD(fldValue,  0, 16 , wru_sgd_pointer_value);
    smemRegSet(devObjPtr, regAddress , fldValue);

    /* Upon every packet 'done' (regardless to drop) */
    __LOG(("invoke interrupt about 'Packet to CPU' (pkt_done_int) \n"));
    intType_bmp = pkt_done_int;

    if(free_sgd_num == 0)
    {
        /* LOR (internal/debug) Interrupt shall be asserted whenever a PDI2MEM R
        doesn't have any SGDs (independent of whether there is incoming traffic
        or not)*/
        __LOG(("invoke interrupt no SGDs (lor_int) \n"));
        intType_bmp |= lor_int;

        if(sgd_recycle_chain == 0)
        {
            /* Whenever the SGD chain is an open chain (Recycle = Disable) and
               the last SGD in the chain has been served, the R will be automatically
               disabled by HW*/
            __LOG(("'open chain' mode (non-cyclic) : used last free SGD , so disabling the Ring \n"));
            ringEnable = 0;
            smemRegFldSet(devObjPtr,SMEM_SIP7_GDMA_QUEUE_EN_REG(devObjPtr,gdma_id,ringId), 0,1,ringEnable);
        }
    }

    if(lor_error)
    {
        __LOG(("DROP interrupt (pkt_drop_int) \n"));
        intType_bmp |= pkt_drop_int;
    }

    /* generate the interrupt(s) */
    gdmaGenInterrupt(devObjPtr, gdma_id, ringId , intType_bmp);

    /* update the counters */
    if(! lor_error)
    {
        /* increment the packet counter */
        regAddress = SMEM_SIP7_GDMA_STAT_COUNTER_REG(devObjPtr,gdma_id, ringId);
        counterPtr = smemMemGet(devObjPtr, regAddress);
        __LOG(("GDMA ringId[%d] : Increment GDMA packet OK count counter by 1 from[%d] \n",
            ringId,counterPtr[0]));
        gdmaInc64BitCounter(counterPtr , 1);

        /* increment the byte counter */
        regAddress = SMEM_SIP7_GDMA_STAT_BYTE_COUNTER_REG(devObjPtr,gdma_id, ringId);
        counterPtr = smemMemGet(devObjPtr, regAddress);
        __LOG(("GDMA ringId[%d] : Increment GDMA bytes OK count counter by [%d] from [%d] \n",
            ringId,dataSize,counterPtr[0]));
        gdmaInc64BitCounter(counterPtr , dataSize);
    }
    else
    {
        /* increment the drop counter */
        regAddress = SMEM_SIP7_GDMA_RX_DROP_COUNTER_REG(devObjPtr,gdma_id, ringId);
        counterPtr = smemMemGet(devObjPtr, regAddress);
        __LOG(("GDMA ringId[%d] : Increment GDMA packet DROP count counter by 1 from[%d] \n",
            ringId,counterPtr[0]));
        gdmaInc64BitCounter(counterPtr , 1);
    }

    if(sentBytes != dataSize)
    {
        __LOG(("ERROR : Final Egress packet (to CPU DMA) ONLY [%d] bytes out of[%d] bytes \n",
            sentBytes,dataSize));
    }
    else
    {
        __LOG(("Final Egress packet (to CPU DMA) [%d] bytes \n",
            sentBytes));
    }

    /* dump to log the packet that egress the port*/
    simLogPacketDump(devObjPtr,
        GT_FALSE ,
        cpuPortDma,
        dataPtr,
        sentBytes);

    return;
}

/**
* @internal snetSip7GdmaSendPacketToCpu function
* @endinternal
*
* @brief   GDMA : Send the packet to the CPU
*
* @param[in] devObjPtr                - pointer to device.
* @param[in] cpuPortDma               - the DMA number of the CPU port (for __LOG info)
* @param[in] tc                       - traffic class
* @param[in] dataPtr                  - pointer to data
* @param[in] dataSize                 - size of data (dataPtr)
*                                       void
*/
void snetSip7GdmaSendPacketToCpu
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                  cpuPortDma,
    IN GT_U32                  tc,
    IN GT_U8                 * dataPtr,
    IN GT_U32                  dataSize
)
{
    DECLARE_FUNC_NAME(snetSip7GdmaSendPacketToCpu);

    GT_U32  regAddress,regValue;
    GT_U32  gdmaPdiEnable,suff_append_en,dispatcherSlavePortAssignMode;
    GT_U32  dsa_slv_type_offset;
    GT_U32  pdi_slv_cpu_code_offset;
    GT_U32  dsa_type;
    GT_U32  ringId = SMEM_U32_GET_FIELD(tc,0,5);
    GT_U32  gdma_id = SMEM_U32_GET_FIELD(tc,5,2);
    GT_U32  gdma_unit_id;

    SIM_OS_MAC(simOsTaskOwnTaskPurposeSet)(
        SIM_OS_TASK_PURPOSE_TYPE_PP_PIPE_PROCESSING_DAEMON_E,NULL);

    simLogPacketFrameUnitSet(SIM_LOG_FRAME_UNIT_GDMA_TO_CPU_E);

    __LOG(("GDMA dispatcher for packet to CPU , started for TC[%d] \n",
        tc));

    __LOG_PARAM(gdma_id);

    regAddress = SMEM_SIP7_GDMA_DISPATCHER_CONTROL_REG(devObjPtr);
    smemRegGet(devObjPtr, regAddress, &regValue);
    gdmaPdiEnable                   = SMEM_U32_GET_FIELD(regValue,0,1);

    if(gdmaPdiEnable == 0)
    {
        /* PDI slave interface disable.
           Until this bit is set, dispatcher is not ready to receive any data
           from DP and applies back pressure towards DP
        */
        __LOG(("Potential configuration ERROR : GDMA dispatcher : PDI slave : globally disabled  (so packet not sent to CPU port) \n"));
        return;
    }

    dispatcherSlavePortAssignMode   = SMEM_U32_GET_FIELD(regValue,1,1);
    suff_append_en                  = SMEM_U32_GET_FIELD(regValue,2,1);


    if (suff_append_en)
    {
        regAddress = SMEM_SIP7_GDMA_PACKET_DEBUG_SUFFIX_REG(devObjPtr);
        smemRegGet(devObjPtr, regAddress,  &regValue);

        /* update 4 bytes to packet header for CRC (used in RTG/RDE tests) */
        __LOG(("GDMA to CPU : Override last 4 bytes of packet as 'fixed' CRC [0x%8.8x] \n",regValue));

        dataPtr[dataSize-4] = (GT_U8)regValue;
        dataPtr[dataSize-3] = (GT_U8)(regValue >>  8);
        dataPtr[dataSize-2] = (GT_U8)(regValue >> 16);
        dataPtr[dataSize-1] = (GT_U8)(regValue >> 24);
    }
    else
    {
        __LOG(("GDMA to CPU : DO NOT update last 4 bytes of packet as 'fixed' \n"));
    }

    if(dispatcherSlavePortAssignMode == 0 /*PDI_Q_MODE*/)
    {
        /*Legacy assignment mode. The GDMA Q is assigned according to the Q field received on the PDI Interface (limited to 8 when working with DP).*/
        __LOG(("GDMA to CPU : GDMA Q assigned according to the Q [%d] field (not use DSA 'CPU CODE') \n",
            tc));
    }
    else
    {
        regAddress = SMEM_SIP7_GDMA_DISPATCHER_DSA_FIELD_OFFSETS_REG(devObjPtr);
        smemRegGet(devObjPtr, regAddress,  &regValue);

        /*DMA_Q is assigned according to the CPU CODE defined inside the header of the packet (eDSA). In case there is no
        CPU_CODE inside packet's header, theGDMA_Q will be assigned according to configuration Notice that in order to
        generate the actual Q, the GDMAQ will get the value according to the mentioned logic plus the Q offset of the specific PDI interface;
        */
        /*Byte Offset of the DSA<CPU_CODE> from beginning of packet. Legal values are 0x10-0x1F*/
        dsa_slv_type_offset     =  SMEM_U32_GET_FIELD(regValue,0,5);
        /*Byte Offset of the DSA<TYPE> from beginning of packet. Legal values are 0x0-0xF*/
        pdi_slv_cpu_code_offset =  SMEM_U32_GET_FIELD(regValue,5,5);

        __LOG_PARAM(pdi_slv_cpu_code_offset);
        __LOG_PARAM(dsa_slv_type_offset);

        __LOG_PARAM(dataPtr[pdi_slv_cpu_code_offset]);
        __LOG_PARAM(dataPtr[dsa_slv_type_offset]);

        dsa_type = dataPtr[dsa_slv_type_offset] >> 6;

        __LOG_PARAM(dsa_type);

        if(SKERNEL_MTAG_CMD_TO_CPU_E != dsa_type)
        {
            __LOG(("GDMA to CPU : DSA is not 'TO_CPU' , so the GDMA Q not map CPU code from DSA (keep TC[%d]) \n",
                tc));
        }
        else
        {
            regAddress = SMEM_SIP7_GDMA_CPU_CODE_TO_GDMA_Q_TBL_MEM(devObjPtr,
                dataPtr[pdi_slv_cpu_code_offset]);
            smemRegGet(devObjPtr, regAddress,  &regValue);
            /*gdmaq*/
            ringId = SMEM_U32_GET_FIELD(regValue,0,5);
            gdma_id = SMEM_U32_GET_FIELD(regValue,5,2);

            __LOG_PARAM(ringId);
            __LOG_PARAM(gdma_id);

            __LOG(("GDMA to CPU : GDMA Q assigned according to the CPU code[0x%2.2x] in DSA , mapped to ringId[%d] \n",
                dataPtr[pdi_slv_cpu_code_offset],
                ringId));
        }
    }

    /* map the <gdma_id> to project GDMA unit ID */
    if(gdma_id > 3)
    {
        gdma_id = 0;
        __LOG_PARAM(gdma_id);
    }

    gdma_unit_id = devObjPtr->gdmaInfo.gdmaDispatcherToUnitmap[gdma_id];

    if(gdma_unit_id == SMAIN_NOT_VALID_CNS)
    {
        __LOG(("configuration error : gdma_id [%d] is NOT mapped to any GDMA unit (so packet not sent to CPU port) \n",
            gdma_id));
        return;
    }

    __LOG(("GDMA to CPU : on ringId[%d] started , in GDMA unit[%d] \n",
        ringId,
        gdma_unit_id));

    /* start the operation in the proper ring */
    gdmaSendPacketToCpu(devObjPtr,gdma_unit_id,ringId,cpuPortDma,dataPtr,dataSize);
    /* state that <Ring is idle> */
    smemRegFldSet(devObjPtr,SMEM_SIP7_GDMA_QUEUE_EN_REG(devObjPtr,gdma_unit_id,ringId), 1,1,RING_IDLE);

    __LOG(("GDMA to CPU : on ringId[%d] Ended , in GDMA unit[%d] \n",
        ringId,
        gdma_unit_id));
}

/**
* @internal gdmaTxSgdValidGet function
* @endinternal
*
* @brief   GDMA : get 'From_cpu' SGD descriptor worg0 and Valid status
*
* @param[in] devObjPtr           - pointer to device.
* @param[in] gdma_id             - the GDMA instance id.
*                                  relative to the CnM instance
*                                  in multi CnM instances , this is not 'global' GDMA instance id.
* @param[in] ringId              - the GDMA ring-id to use.
* @param[out] txDesc_Ptr         - Pointer to full TX descriptor
* @param[out] currentDmaAddr     - TX descriptor address - [0=low][1=high] (Optional param)
* @param[out] dest_addr_high_ptr - Packet destination address High         (Optional param)
*
* @return void
*/
GT_U32 gdmaTxSgdValidGet
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                  gdma_id,
    IN GT_U32                  ringId,
    IN GT_U32                  wru_sgd_pointer_value,
    OUT GT_U32                 *txDesc_Ptr,
    OUT GT_U32                 *currentDmaAddr2,
    OUT GT_U32                 *dest_addr_high_ptr
)
{
    DECLARE_FUNC_NAME(gdmaTxSgdValidGet);

    GT_U32  regAddress;                 /* Register address */
    GT_U32  fldValue;                   /* Register field value */
    GT_U32      startDmaAddr[2];        /* const-config */
    GT_U32      sgd_extend_mode;        /* const-config */
    GT_U32      sizeof_sgd; /* size of the SGD in bytes - const-config */
    GT_U32      isValid;

    GT_U32      currentDmaAddr[2];
    GT_U32      dest_addr_high;


    regAddress = SMEM_SIP7_GDMA_CHAIN_ADDRESS_LOW_REG(devObjPtr,gdma_id, ringId);
    smemRegGet(devObjPtr, regAddress,  &startDmaAddr[0]);
    regAddress = SMEM_SIP7_GDMA_CHAIN_ADDRESS_HIGH_REG(devObjPtr,gdma_id, ringId);
    smemRegGet(devObjPtr, regAddress,  &startDmaAddr[1]);

    /* Get the descriptor size regular mode is 8B and extended mode is 16B*/
    regAddress = SMEM_SIP7_GDMA_QUEUE_CONTROL_REG(devObjPtr,gdma_id, ringId);
    smemRegGet(devObjPtr, regAddress , &fldValue );

    sgd_extend_mode = SMEM_U32_GET_FIELD(fldValue, 5, 1);
    sizeof_sgd = (sgd_extend_mode == EXTENDED) ? 16 : 8;

    if (SMEM_U32_GET_FIELD(fldValue, 0, 4) != FROM_CPU_PKT_GEN_RING_TYPE) /* q_type */
    {
        __LOG(("PktGen: ERROR [%u:%d]sgdIdx=0x%x: wrong q_type, control reg=0x%08x\n",
                                gdma_id, ringId, wru_sgd_pointer_value, fldValue));
        return 0;
    }

    /* Calculate the currentDescp address based on current Descriptor position*/
    currentDmaAddr[0] = startDmaAddr[0] + wru_sgd_pointer_value * sizeof_sgd;
    currentDmaAddr[1] = startDmaAddr[1];
    if(currentDmaAddr[0] < startDmaAddr[0])
    {
        /* wrap around of the address 32LSBits */
        currentDmaAddr[1]++;
    }

    /* Read the DMA for needed value, put data into current descriptor 'txDesc_Ptr~curDescrInfo' */
    snetChtPerformScibDmaRead_64BitAddr(SNET_CHT_DMA_CLIENT_PACKET_FROM_CPU_E, gdma_id,
                devObjPtr->deviceId,
                currentDmaAddr[0], currentDmaAddr[1],
                NUM_BYTES_TO_WORDS(sizeof_sgd),
                txDesc_Ptr,
                SCIB_DMA_WORDS);

    if(sgd_extend_mode == REGULAR)
    {
        regAddress = SMEM_SIP7_GDMA_PAYLOAD_BUFFER_ADDRESS_HIGH_REG(devObjPtr,gdma_id, ringId);
        smemRegGet(devObjPtr, regAddress , &fldValue );
        dest_addr_high = SMEM_U32_GET_FIELD(fldValue, 16, 14);/* for bits 34..47 of the address */
    }
    else /*(sgd_extend_mode == EXTENDED)*/
    {
        dest_addr_high = GDMA_TX(devObjPtr, txDesc_Ptr, wru_sgd_pointer_value,
                    SMEM_SIP7_GDMA_SW_GDMA_DESCRIPTOR_TX_FIELD_BUFFER_DST_ADDR_HIGH);
    }

    /* --- Setting output parameters --- */
    isValid = GDMA_TX(devObjPtr, txDesc_Ptr, wru_sgd_pointer_value,
                      SMEM_SIP7_GDMA_SW_GDMA_DESCRIPTOR_TX_VALID);

    if (currentDmaAddr2 != NULL)
    {
        currentDmaAddr2[0] = currentDmaAddr[0];
        currentDmaAddr2[1] = currentDmaAddr[1];
    }
    if (dest_addr_high_ptr != NULL)
        *dest_addr_high_ptr = dest_addr_high;

    return isValid;
}

/**
* @internal gdmaTreatFromCpuPacketsPktGen function
* @endinternal
*
* @brief   GDMA : treat 'From_cpu' packets that ready on the queue (ring)
*
* @param[in] devObjPtr                - pointer to device.
* @param[in] gdma_id                  - the GDMA instance id.
*                                       relative to the CnM instance
*                                       in multi CnM instances , this is not 'global' GDMA instance id.
* @param[in] ringId                   - the GDMA ring-id to use.
*
* @return void
*/
static void gdmaTreatFromCpuPacketsPktGen
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32                  gdma_id,
    IN GT_U32                  ringId,
    IN GT_U32                  sgdIdx,
    IN GT_U8                   *placeForPacketPtr,
    IN GT_U32                  placeForPacketSize,
    OUT GT_BOOL                *queueEmptyPtr,
    OUT GT_BOOL                *isLastPacketPtr
)
{
    DECLARE_FUNC_NAME(gdmaTreatFromCpuPacketsPktGen);

    GT_U32  buffLen;                    /* Length of the current buffer */
    GT_U32  totalPacketSize = 0;        /* Number of bytes for the packet */
    GT_U32  regAddress;                 /* Register address */
    GT_U32  fldValue;                   /* Register field value */
    GT_U32  *counterPtr;                /* STATS counter pointer*/
    INTERRUPT_BMP_TYPE_ENT  intType_bmp = 0;
    GDMA_DESC_EXTENDED curDescrInfo;    /* TX descriptor */
    GT_U32      *txDesc_Ptr;            /* Pointer to TX descriptor */
    GT_U32      currentDmaAddr[2];      /* Current TX descriptor address [0=low][1=high]*/
    GT_U32      wru_sgd_pointer_value;  /* SGD index in array of descriptors */
    GT_U32      dest_addr_high;
    GT_U32      temp_address[2];        /* PACKET address [0=low][1=high] */
    GT_U32      ownBit;
    GT_U32      opcode;                 /* const-config */
    GT_U32      isValid;
    GT_U32      isFirst, isLast;

    *queueEmptyPtr   = GT_TRUE;
    *isLastPacketPtr = GT_TRUE;

    /* Send enqueued packets even if ring/queue has been disabled by PktGen task.
     * RING_BUSY/RING_IDLE is set by the PktGen task, and not per-packet but per whole chain
     */

    wru_sgd_pointer_value = sgdIdx;
    txDesc_Ptr = &curDescrInfo.regulare.words[0];

    isValid = gdmaTxSgdValidGet(devObjPtr, gdma_id, ringId, wru_sgd_pointer_value,
                                txDesc_Ptr, currentDmaAddr, &dest_addr_high);

    /* DO treat 1 txDesc_Ptr that holds info to send from the GDMA to the DP */
    do
    {
        if (!isValid)
        {
            __LOG(("PktGen: ERROR [%u:%d]sgdIdx=0x%x: Invalid descriptor, control reg=0x%08x\n",
                                    gdma_id, ringId, sgdIdx, fldValue));
            break;
        }

        ownBit =
            GDMA_TX(devObjPtr,txDesc_Ptr,wru_sgd_pointer_value,
                SMEM_SIP7_GDMA_SW_GDMA_DESCRIPTOR_TX_FIELD_OWN);

        if(ownBit == OWNER_HW) /* Cleared by set OWNER_CPU later on */
        {
            __LOG(("PktGen: NOTE : the ownBit=1, HW process SGD with clear ownBit\n"));
        }

        isFirst =
            GDMA_TX(devObjPtr,txDesc_Ptr,wru_sgd_pointer_value,
                SMEM_SIP7_GDMA_SW_GDMA_DESCRIPTOR_TX_FIELD_F_BIT);

        /* PktGen sends OnePkt-per-OneDescriptor => should be Firs&Last */
        if(!isFirst)
        {
            __LOG(("PktGen: ERROR : <F>=0 on first SGD (send the packet as if <F>=1) \n"));
        }

        opcode =
            GDMA_TX(devObjPtr,txDesc_Ptr,wru_sgd_pointer_value,
                SMEM_SIP7_GDMA_SW_GDMA_DESCRIPTOR_TX_FIELD_OPCODE);

        if(opcode != 0)
        {
            /* was not implemented on the design (on real HW) */
            __LOG(("PktGen: opcode == 1 was not implemented (so the value is ignored) \n"));
        }

        temp_address[0] =
            GDMA_TX(devObjPtr,txDesc_Ptr,wru_sgd_pointer_value,
                SMEM_SIP7_GDMA_SW_GDMA_DESCRIPTOR_TX_FIELD_BUFFER_DST_ADDR_LOW_0);
        temp_address[1] =
            GDMA_TX(devObjPtr,txDesc_Ptr,wru_sgd_pointer_value,
                SMEM_SIP7_GDMA_SW_GDMA_DESCRIPTOR_TX_FIELD_BUFFER_DST_ADDR_LOW_1);
        temp_address[1] |= dest_addr_high << 2;

        buffLen =
            GDMA_TX(devObjPtr,txDesc_Ptr,wru_sgd_pointer_value,
                SMEM_SIP7_GDMA_SW_GDMA_DESCRIPTOR_TX_FIELD_BUFFER_BYTE_COUNT);

        buffLen += 1;
        if(buffLen == 1)
        {
            __LOG(("PktGen: ERROR : '1' (One) byte in the 'from_cpu' descriptor\n"));
            isValid = 0;
            break;
        }

        if(buffLen > placeForPacketSize)
        {
            __LOG(("PktGen: ERROR : packet size[%d] > WM supported [%d] bytes \n",
                                                buffLen, placeForPacketSize));
            isValid = 0;
            break;
        }

        totalPacketSize = 0;
        /* Read packet part from the DMA buffer (sharedData is destroyed after this copy) */
        snetChtPerformScibDmaRead_64BitAddr(SNET_CHT_DMA_CLIENT_PACKET_FROM_CPU_E,gdma_id,
                    devObjPtr->deviceId,temp_address[0/*low*/],temp_address[1/*high*/],
                        NUM_BYTES_TO_WORDS(buffLen),
                        (GT_U32*)&(placeForPacketPtr[totalPacketSize]),
                        TX_BYTE_ORDER_MAC(devObjPtr));
        totalPacketSize += buffLen;

        isLast =
            GDMA_TX(devObjPtr,txDesc_Ptr,wru_sgd_pointer_value,
                SMEM_SIP7_GDMA_SW_GDMA_DESCRIPTOR_TX_FIELD_L_BIT);

        GDMA_TX_SET(devObjPtr,txDesc_Ptr,wru_sgd_pointer_value,
            SMEM_SIP7_GDMA_SW_GDMA_DESCRIPTOR_TX_FIELD_OWN,
            OWNER_CPU);

        /* PktGen sends OnePkt-per-OneDescriptor => should be Firs&Last */
        if(!isLast)
        {
            __LOG(("PktGen: ERROR : <L>=0, send the packet as if <L>=1)\n"));
        }

        /* only word 0 hold info that the device may have changed */
        snetChtPerformScibDmaWrite_64BitAddr(SNET_CHT_DMA_CLIENT_PACKET_FROM_CPU_E,gdma_id,
                    devObjPtr->deviceId,currentDmaAddr[0],currentDmaAddr[1],
                    1/*single word*/,
                    txDesc_Ptr, SCIB_DMA_WORDS);

    } while(0);

    intType_bmp |= tx_sgd_done_int;
    if (!isValid)
    {
        intType_bmp |= pkt_drop_int;
        gdmaGenInterrupt(devObjPtr, gdma_id, ringId , intType_bmp);
    }
    intType_bmp |= pkt_done_int;
    gdmaGenInterrupt(devObjPtr, gdma_id, ringId , intType_bmp);

    if (!isValid)
        return; /* ERROR, drop */

    /* update the counters */
    /* increment the packet counter */
    regAddress = SMEM_SIP7_GDMA_STAT_COUNTER_REG(devObjPtr,gdma_id, ringId);
    counterPtr = smemMemGet(devObjPtr, regAddress);
    gdmaInc64BitCounter(counterPtr , 1);

    /* increment the byte counter */
    regAddress = SMEM_SIP7_GDMA_STAT_BYTE_COUNTER_REG(devObjPtr,gdma_id, ringId);
    counterPtr = smemMemGet(devObjPtr, regAddress);
    gdmaInc64BitCounter(counterPtr , totalPacketSize);

    regAddress = SMEM_SIP7_GDMA_PDI_MASTER_CONTROL_REG(devObjPtr,gdma_id);
    smemRegGet(devObjPtr, regAddress , &fldValue);
    if(0 == (fldValue & 0x1))
    {
        __LOG(("PktGen: ERROR : The <pdi_mstr_enable> is DISABLED , so no ring of GDMA[%d] can send packet --> DROP the packet \n",
            gdma_id));
        return; /*ERROR*/
    }

    *queueEmptyPtr = GT_FALSE;

    /* Sip6.30 (Ironman) Tx Handle sends packet from CPU without CRC ,
        so for the WM we need to add 'fake' 4 bytes to allow WM logic to keep going ... */
    placeForPacketPtr[totalPacketSize + 0 ] = 0x66;/*for WM only*/
    placeForPacketPtr[totalPacketSize + 1 ] = 0x66;/*for WM only*/
    placeForPacketPtr[totalPacketSize + 2 ] = 0x66;/*for WM only*/
    placeForPacketPtr[totalPacketSize + 3 ] = 0x66;/*for WM only*/
    totalPacketSize += 4;                          /*for WM only*/

    descrPtr->frameBuf->actualDataSize = totalPacketSize;
    /* call the proper DP[] to treat the packet */

    /* the packet came from the DMA of the CPU */
    /* convert the GDMA unit to the DP index */
    descrPtr->ingressRxDmaPortNumber = devObjPtr->gdmaInfo.gdmaFromCpuMapTo_ingressRxDmaPortNumber[gdma_id];
    descrPtr->localDevSrcPort = descrPtr->ingressRxDmaPortNumber;
    if(devObjPtr->numOfPipes)
    {
        GT_U32  dummyPort;
        /* Update 'currentPipeId' and get new (local) port */
        smemConvertGlobalPortToCurrentPipeId(devObjPtr,descrPtr->ingressRxDmaPortNumber,&dummyPort);
    }

    /* the GDMA CPU port does not hold 'GOP' !!! */
    descrPtr->ingressGopPortNumber = SMAIN_NOT_VALID_CNS;/* value to cause fatal error for using 'GOP' when should not */

    /* Indicate frame received through DMA mechanism */
    descrPtr->isFromSdma = GT_TRUE;

    __LOG(("PktGen-sentOut: ring=%u SGD-index=%u - sending is passed\n", ringId, wru_sgd_pointer_value));
    gdmaTimestampTrace(devObjPtr, "PktGen-sentOut", ringId, wru_sgd_pointer_value);

    return; /*OK*/
}

/**
* @internal gdmaTreatFromCpuPackets function
* @endinternal
*
* @brief   GDMA : treat 'From_cpu' packets that ready on the queue (ring)
*
* @param[in] devObjPtr                - pointer to device.
* @param[in] gdma_id                  - the GDMA instance id.
*                                       relative to the CnM instance
*                                       in multi CnM instances , this is not 'global' GDMA instance id.
* @param[in] ringId                   - the GDMA ring-id to use.
*
* @return void
*/
static void gdmaTreatFromCpuPackets
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32                  gdma_id,
    IN GT_U32                  ringId,
    IN GT_U8                   *placeForPacketPtr,
    IN GT_U32                  placeForPacketSize,
    OUT GT_BOOL                *queueEmptyPtr,
    OUT GT_BOOL                *isLastPacketPtr
)
{
    DECLARE_FUNC_NAME(gdmaTreatFromCpuPackets);

    GT_U32  buffLen;                    /* Length of the current buffer */
    GT_U32  totalPacketSize = 0;        /* Number of bytes for the packet */
    GT_U32  regAddress;                 /* Register address */
    GT_U32  *counterPtr;                /* counter pointer*/
    GT_U32  fldValue;                   /* Register field value */
    INTERRUPT_BMP_TYPE_ENT  intType_bmp = 0;
    GDMA_DESC_EXTENDED curDescrInfo;    /* TX descriptor */
    GT_U32      *txDesc_Ptr;            /* Pointer to TX descriptor */
    GT_U32      currentDmaAddr[2];      /* Current TX descriptor address */
    GT_U32      startDmaAddr[2];
    GT_U32      ringEnable;
    GT_U32      q_type,sgd_recycle_chain,sgd_extend_mode,q_wrr_weight;
    GT_U32      wru_sgd_pointer_value;
    GT_U32      SGD_Chain_Size;
    GT_U32      free_sgd_num;
    GT_U32      dest_addr_high;
    GT_U32      temp_address[2];
    GT_U32      ownBit;
    GT_U32      sizeof_sgd; /* size of the SGD in bytes */
    GT_U32      opcode;
    GT_U32      isValid;
    GT_U32      no_more_sgd = 0;
    GT_U32      oversize = 0;
    GT_U32      numOfParts = 0;
    GT_U32      isFirst,isLast;

    *queueEmptyPtr   = GT_TRUE;
    *isLastPacketPtr = GT_TRUE;

    SCIB_SEM_TAKE;
    smemRegFldGet(devObjPtr,SMEM_SIP7_GDMA_QUEUE_EN_REG(devObjPtr,gdma_id,ringId), 0,1,&ringEnable);
    if (ringEnable == 0)
    {
        SCIB_SEM_SIGNAL;
        __LOG(("GDMA from CPU : ringId[%d] is disabled (so not send packet to CPU) \n",
            ringId));
        return;
    }

    /* state that <Ring is busy> */
    smemRegFldSet(devObjPtr,SMEM_SIP7_GDMA_QUEUE_EN_REG(devObjPtr,gdma_id,ringId), 1,1,RING_BUSY);
    SCIB_SEM_SIGNAL;

    regAddress = SMEM_SIP7_GDMA_CHAIN_ADDRESS_LOW_REG(devObjPtr,gdma_id, ringId);
    smemRegGet(devObjPtr, regAddress,  &startDmaAddr[0]);
    regAddress = SMEM_SIP7_GDMA_CHAIN_ADDRESS_HIGH_REG(devObjPtr,gdma_id, ringId);
    smemRegGet(devObjPtr, regAddress,  &startDmaAddr[1]);

    /* Get the descriptor size regular mode is 8B and extended mode is 16B*/
    regAddress = SMEM_SIP7_GDMA_QUEUE_CONTROL_REG(devObjPtr,gdma_id, ringId);
    smemRegGet(devObjPtr, regAddress , &fldValue );
    sgd_recycle_chain = SMEM_U32_GET_FIELD(fldValue, 4, 1);
    sgd_extend_mode = SMEM_U32_GET_FIELD(fldValue, 5, 1);
    q_type = SMEM_U32_GET_FIELD(fldValue, 0, 4);
    q_wrr_weight = SMEM_U32_GET_FIELD(fldValue, 20, 4);

    if(q_wrr_weight == 0)
    {
        /* behave just as 'ring disable' */
        __LOG(("ERROR : GDMA from CPU : q_wrr_weight == 0 --> act as ring disabled (so not treat packet from CPU) \n"));
        return;
    }

    sizeof_sgd = (sgd_extend_mode == EXTENDED) ? 16 : 8;

    if(FROM_CPU_RING_TYPE != q_type)
    {
        __LOG(("ERROR : GDMA from CPU : ringId[%d] q_type[%d] is NOT for 'FROM_CPU' (so not treat packet from CPU) \n",
            ringId,q_type));
        return;
    }

    /* Start from first descriptor address */
    regAddress = SMEM_SIP7_GDMA_BASE_DESC_POINTER_REG(devObjPtr,gdma_id, ringId);
    smemRegGet(devObjPtr, regAddress , &fldValue );
    wru_sgd_pointer_value = SMEM_U32_GET_FIELD(fldValue, 0, 16);

    __LOG_PARAM(wru_sgd_pointer_value);

    /* Calculate the currentDescp address based on current Descriptor position*/
    currentDmaAddr[0] = startDmaAddr[0] + wru_sgd_pointer_value * sizeof_sgd;
    currentDmaAddr[1] = startDmaAddr[1];
    if(currentDmaAddr[0] < startDmaAddr[0])
    {
        /* wrap around of the address 32LSBits */
        currentDmaAddr[1]++;
    }

    __LOG_PARAM(currentDmaAddr[0]);
    __LOG_PARAM(currentDmaAddr[1]);

    if(sgd_extend_mode == REGULAR)
    {
        regAddress = SMEM_SIP7_GDMA_PAYLOAD_BUFFER_ADDRESS_HIGH_REG(devObjPtr,gdma_id, ringId);
        smemRegGet(devObjPtr, regAddress , &fldValue );
        dest_addr_high = SMEM_U32_GET_FIELD(fldValue, 16, 14);/* for bits 34..47 of the address */
    }
    else
    {
        dest_addr_high = 0;/* the info is in the SGD */
    }

    txDesc_Ptr = &curDescrInfo.regulare.words[0];

    regAddress = SMEM_SIP7_GDMA_CHAIN_SIZE_REG(devObjPtr,gdma_id, ringId);
    smemRegGet(devObjPtr, regAddress , &fldValue );
    SGD_Chain_Size = SMEM_U32_GET_FIELD(fldValue,  0, 16);

    if(wru_sgd_pointer_value >= SGD_Chain_Size)
    {
        __LOG(("ERROR : GDMA from CPU : ringId[%d] wru_sgd_pointer_value[%d] >= SGD_Chain_Size[%d] (so not handle packet from CPU) \n",
            ringId,
            wru_sgd_pointer_value,SGD_Chain_Size));
        return;
    }

    /* CIDER: this register is for TO_CPU only */
    SCIB_SEM_TAKE;
    regAddress = SMEM_SIP7_GDMA_FREE_SGDS_STATUS_REG(devObjPtr,gdma_id, ringId);
    smemRegGet(devObjPtr, regAddress , &fldValue );
    free_sgd_num = SMEM_U32_GET_FIELD(fldValue,  0, 16);

    __LOG_PARAM(free_sgd_num);
    if(free_sgd_num == 0)
    {
        no_more_sgd = 1;
    }
    SCIB_SEM_SIGNAL;

    /* treat descriptors , that hold info to sent from the GDMA to the DP */
    while (1)
    {
        if(no_more_sgd)
        {
            if(numOfParts)
            {
                __LOG(("from cpu : no more waiting descriptors , stop processing new SGDs ,"
                    "loosing [%d] parts of the packet \n",
                    numOfParts));
            }
            else
            {
                __LOG(("from cpu : no more waiting descriptors , stop processing new SGDs\n",
                    numOfParts));
            }

            /* we stop the processing and we wait for active memory to trigger a
               message to skernel to let this function called again when there are more descriptors
            */
            return;
        }

        /* Read the DMA for needed value, put data into current descriptor */
        __LOG(("from cpu : Read the DMA for needed value, put data into current descriptor"));
        snetChtPerformScibDmaRead_64BitAddr(SNET_CHT_DMA_CLIENT_PACKET_FROM_CPU_E,gdma_id,
                    devObjPtr->deviceId,currentDmaAddr[0],currentDmaAddr[1],
                    NUM_BYTES_TO_WORDS(sizeof_sgd),
                    txDesc_Ptr, SCIB_DMA_WORDS);

        ownBit =
            GDMA_TX(devObjPtr,txDesc_Ptr,wru_sgd_pointer_value,
                SMEM_SIP7_GDMA_SW_GDMA_DESCRIPTOR_TX_FIELD_OWN);

        if(ownBit == OWNER_CPU)/* for the FROM_CPU the HW ignore the read value of ownBit */
        {
            __LOG(("NOTE : FROM_CPU : the ownBit is 'OWNER_CPU' but the HW will treat it anyway (as owned by HW) \n"));
        }

        isValid =
            GDMA_TX(devObjPtr,txDesc_Ptr,wru_sgd_pointer_value,
                SMEM_SIP7_GDMA_SW_GDMA_DESCRIPTOR_TX_VALID);
        if(!isValid)
        {
            __LOG(("NOTE : FROM_CPU : current descriptor is 'not valid' , so skip it \n"));

            isLast = 0;
            goto continueAfterSgdUpdate_lbl;
        }

        /* get First bit */
        isFirst =
            GDMA_TX(devObjPtr,txDesc_Ptr,wru_sgd_pointer_value,
                SMEM_SIP7_GDMA_SW_GDMA_DESCRIPTOR_TX_FIELD_F_BIT);

        if(isFirst && totalPacketSize != 0)
        {
            __LOG(("configuration ERROR : <F>=1 (second on this packet) , before we ended the packet \n"));
            __LOG(("starting over the packet (ignoring the previous bytes) \n"));

            __LOG(("invoke interrupt about 'NO_EOP packet fragment interrupt' \n"));
            intType_bmp |= no_eop_pkt_frag_int;

            totalPacketSize = 0;
        }
        else
        if(!isFirst && totalPacketSize == 0)
        {
            __LOG(("configuration ERROR : <F>=0 on first SGD (send the packet as if <F>=1) \n"));
        }

        opcode =
            GDMA_TX(devObjPtr,txDesc_Ptr,wru_sgd_pointer_value,
                SMEM_SIP7_GDMA_SW_GDMA_DESCRIPTOR_TX_FIELD_OPCODE);

        if(opcode != 0)
        {
            /* was not implemented on the design (on real HW) */
            __LOG(("opcode == 1 was not implemented (so the value is ignored) \n"));
        }

        temp_address[0] =
            GDMA_TX(devObjPtr,txDesc_Ptr,wru_sgd_pointer_value,
                SMEM_SIP7_GDMA_SW_GDMA_DESCRIPTOR_TX_FIELD_BUFFER_DST_ADDR_LOW_0);
        temp_address[1] =
            GDMA_TX(devObjPtr,txDesc_Ptr,wru_sgd_pointer_value,
                SMEM_SIP7_GDMA_SW_GDMA_DESCRIPTOR_TX_FIELD_BUFFER_DST_ADDR_LOW_1);

        if(sgd_extend_mode == EXTENDED)
        {
            dest_addr_high = GDMA_TX(devObjPtr,txDesc_Ptr,wru_sgd_pointer_value,
                SMEM_SIP7_GDMA_SW_GDMA_DESCRIPTOR_TX_FIELD_BUFFER_DST_ADDR_HIGH);
        }
        temp_address[1] |= dest_addr_high << 2;

        buffLen =
            GDMA_TX(devObjPtr,txDesc_Ptr,wru_sgd_pointer_value,
                SMEM_SIP7_GDMA_SW_GDMA_DESCRIPTOR_TX_FIELD_BUFFER_BYTE_COUNT);

        buffLen += 1;
        if(buffLen == 1)
        {
            __LOG(("potential configuration ERROR : '1' (One) byte in the 'from_cpu' descriptor , is that intended ?! \n"));
        }

        if(oversize)
        {
            /* already limited the size */
        }
        else
        if((totalPacketSize + buffLen) > placeForPacketSize)
        {
            __LOG(("potential configuration ERROR : total size of packet is already [%d] but WM supporting only [%d] bytes \n",
                (totalPacketSize + buffLen),
                placeForPacketSize));

            buffLen = placeForPacketSize - totalPacketSize;

            oversize = 1;
        }

        if(!oversize)
        {
            /* Read packet part from the DMA buffer */
            snetChtPerformScibDmaRead_64BitAddr(SNET_CHT_DMA_CLIENT_PACKET_FROM_CPU_E,gdma_id,
                        devObjPtr->deviceId,temp_address[0/*low*/],temp_address[1/*high*/],
                         NUM_BYTES_TO_WORDS(buffLen),
                         (GT_U32*)&(placeForPacketPtr[totalPacketSize]),
                         TX_BYTE_ORDER_MAC(devObjPtr));

            totalPacketSize += buffLen;
        }
        else
        {
            __LOG(("oversized packet , we ignore those [%d] bytes  \n",
                buffLen));
        }

        /* Last bit */
        isLast =
            GDMA_TX(devObjPtr,txDesc_Ptr,wru_sgd_pointer_value,
                SMEM_SIP7_GDMA_SW_GDMA_DESCRIPTOR_TX_FIELD_L_BIT);

        GDMA_TX_SET(devObjPtr,txDesc_Ptr,wru_sgd_pointer_value,
            SMEM_SIP7_GDMA_SW_GDMA_DESCRIPTOR_TX_FIELD_OWN,
            OWNER_CPU);

        /* only word 0 hold info that the device may have changed */
        __LOG(("Write back the the first word of current descriptor \n"));
        snetChtPerformScibDmaWrite_64BitAddr(SNET_CHT_DMA_CLIENT_PACKET_FROM_CPU_E,gdma_id,
                    devObjPtr->deviceId,currentDmaAddr[0],currentDmaAddr[1],
                    1/*single word*/,
                    txDesc_Ptr, SCIB_DMA_WORDS);

        continueAfterSgdUpdate_lbl:

        /* decrement the 'free_sgd_num' (but make sure to SCIB lock the operation)
           as the active memory may want to update it */
        SCIB_SEM_TAKE;
        regAddress = SMEM_SIP7_GDMA_FREE_SGDS_STATUS_REG(devObjPtr,gdma_id, ringId);
        /* we read it under LOCK , as maybe changed by active memory after we unlocked the SCIB*/
        smemRegGet(devObjPtr, regAddress , &fldValue );
        free_sgd_num = SMEM_U32_GET_FIELD(fldValue,  0, 16);
        if(free_sgd_num == 0)
        {
            /* should not happen */
            skernelFatalError("gdmaTreatFromCpuPackets: free_sgd_num == 0 when need to decrement --> bad management \n");
        }
        else
        {
            free_sgd_num --;
            __LOG_PARAM(free_sgd_num);

            if(free_sgd_num == 0)
            {
                no_more_sgd = 1;
            }
        }

        smemRegFldSet(devObjPtr, regAddress , 0, 16, free_sgd_num);
        SCIB_SEM_SIGNAL;

        __LOG(("invoke interrupt about SGD Done (tx_sgd_done_int) \n"));
        intType_bmp |= tx_sgd_done_int;

        /* move the address of descriptor to the next descriptor */
        currentDmaAddr[0] += sizeof_sgd;
        if(currentDmaAddr[0] < sizeof_sgd)
        {
            /* wrap around of the address 32LSBits */
            currentDmaAddr[1]++;
        }

        /* update the current pointer of the ring */
        wru_sgd_pointer_value++;
        if(wru_sgd_pointer_value >= SGD_Chain_Size)
        {
            /* restart for next time */
            wru_sgd_pointer_value = 0;
            __LOG_PARAM(wru_sgd_pointer_value);

            /* wrap also the DMA of the start of the queue */
            currentDmaAddr[0] = startDmaAddr[0];
            currentDmaAddr[1] = startDmaAddr[1];
        }
        __LOG_PARAM(wru_sgd_pointer_value);
        __LOG_PARAM(currentDmaAddr[0]);
        __LOG_PARAM(currentDmaAddr[1]);


        numOfParts ++;

        if (isLast == 1)
        {
            /* we are done with this frame bytes */
            __LOG(("From CPU : we got all the frame [%d] part%s (got <L> = 1) \n",
                numOfParts,
                numOfParts > 1 ?"(s)":""));
            break;
        }

        if(wru_sgd_pointer_value == 0 &&
           sgd_recycle_chain == 0)
        {
            /* not allow new SGD iteration */
            break;
        }
    }

    if(wru_sgd_pointer_value == 0 &&
       sgd_recycle_chain == 0)
    {
        __LOG(("invoke interrupt about 'Packet from CPU' Ring Chain Done (r_chain_done_int) \n"));
        intType_bmp |= r_chain_done_int;
        __LOG(("'open chain' mode (non-cyclic) : chain ended , so disabling the Ring \n"));
        ringEnable = 0;
        smemRegFldSet(devObjPtr,SMEM_SIP7_GDMA_QUEUE_EN_REG(devObjPtr,gdma_id,ringId), 0,1,ringEnable);
    }

    /* Update the next descriptor to handle */
    regAddress = SMEM_SIP7_GDMA_BASE_DESC_POINTER_REG(devObjPtr,gdma_id, ringId);
    SMEM_U32_SET_FIELD(fldValue,  0, 16 , wru_sgd_pointer_value);
    smemRegSet(devObjPtr, regAddress , fldValue);

    /* Upon every packet 'done' (regardless to drop) */
    __LOG(("invoke interrupt about 'Packet from CPU' (pkt_done_int) \n"));
    intType_bmp |= pkt_done_int;

    if(free_sgd_num == 0)
    {
        /* LOR (internal/debug) Interrupt shall be asserted whenever a PDI2MEM R
        doesn't have any SGDs (independent of whether there is incoming traffic
        or not)*/
        __LOG(("invoke interrupt no SGDs (lor_int) \n"));
        intType_bmp |= lor_int;

        if(sgd_recycle_chain == 0)
        {
            /* Whenever the SGD chain is an open chain (Recycle = Disable) and
               the last SGD in the chain has been served, the R will be automatically
               disabled by HW*/
            __LOG(("'open chain' mode (non-cyclic) : used last free SGD , so disabling the Ring \n"));
            ringEnable = 0;
            smemRegFldSet(devObjPtr,SMEM_SIP7_GDMA_QUEUE_EN_REG(devObjPtr,gdma_id,ringId), 0,1,ringEnable);
        }
    }

    /* generate the interrupt(s) */
    gdmaGenInterrupt(devObjPtr, gdma_id, ringId , intType_bmp);

    /* update the counters */
    /* increment the packet counter */
    regAddress = SMEM_SIP7_GDMA_STAT_COUNTER_REG(devObjPtr,gdma_id, ringId);
    counterPtr = smemMemGet(devObjPtr, regAddress);
    __LOG(("GDMA ringId[%d] : Increment GDMA packet OK count counter by 1 from[%d] \n",
        ringId,counterPtr[0]));
    gdmaInc64BitCounter(counterPtr , 1);

    /* increment the byte counter */
    regAddress = SMEM_SIP7_GDMA_STAT_BYTE_COUNTER_REG(devObjPtr,gdma_id, ringId);
    counterPtr = smemMemGet(devObjPtr, regAddress);
    __LOG(("GDMA ringId[%d] : Increment GDMA bytes OK count counter by [%d] from [%d] \n",
        ringId,totalPacketSize,counterPtr[0]));
    gdmaInc64BitCounter(counterPtr , totalPacketSize);

    __LOG(("Final packet (from CPU DMA) [%d] bytes \n",
        totalPacketSize));

    regAddress = SMEM_SIP7_GDMA_PDI_MASTER_CONTROL_REG(devObjPtr,gdma_id);
    smemRegGet(devObjPtr, regAddress , &fldValue);
    if(0 == (fldValue & 0x1))
    {
        __LOG(("GDMA from CPU : ERROR : The <pdi_mstr_enable> is DISABLED , so no ring of GDMA[%d] can send packet --> DROP the packet \n",
            gdma_id));
        return;
    }


    *queueEmptyPtr   = GT_FALSE;
    *isLastPacketPtr = (free_sgd_num == 0) ? GT_TRUE : GT_FALSE;

    /* Sip6.30 (Ironman) Tx Handle sends packet from CPU without CRC ,
        so for the WM we need to add 'fake' 4 bytes to allow WM logic to keep going ... */
    placeForPacketPtr[totalPacketSize + 0 ] = 0x66;/*for WM only*/
    placeForPacketPtr[totalPacketSize + 1 ] = 0x66;/*for WM only*/
    placeForPacketPtr[totalPacketSize + 2 ] = 0x66;/*for WM only*/
    placeForPacketPtr[totalPacketSize + 3 ] = 0x66;/*for WM only*/
    totalPacketSize += 4;                          /*for WM only*/

    descrPtr->frameBuf->actualDataSize = totalPacketSize;
    /* call the proper DP[] to treat the packet */

    /* the packet came from the DMA of the CPU */
    /* convert the GDMA unit to the DP index */
    descrPtr->ingressRxDmaPortNumber = devObjPtr->gdmaInfo.gdmaFromCpuMapTo_ingressRxDmaPortNumber[gdma_id];
    descrPtr->localDevSrcPort = descrPtr->ingressRxDmaPortNumber;
    if(devObjPtr->numOfPipes)
    {
        GT_U32  dummyPort;
        /* Update 'currentPipeId' and get new (local) port */
        smemConvertGlobalPortToCurrentPipeId(devObjPtr,descrPtr->ingressRxDmaPortNumber,&dummyPort);
    }

    /* the GDMA CPU port does not hold 'GOP' !!! */
    descrPtr->ingressGopPortNumber = SMAIN_NOT_VALID_CNS;/* value to cause fatal error for using 'GOP' when should not */

    __LOG_PARAM_NO_LOCATION_META_DATA(descrPtr->localDevSrcPort);
    __LOG_PARAM_NO_LOCATION_META_DATA(descrPtr->ingressRxDmaPortNumber);
    __LOG_PARAM_NO_LOCATION_META_DATA(descrPtr->ingressGopPortNumber);

    /* Indicate frame received through DMA mechanism */
    descrPtr->isFromSdma = GT_TRUE;
    return;
}

/**
* @internal gdmaTreatFromCpuQueuePackets function
* @endinternal
*
* @brief   get from the GDMA queue 'from cpu' packets and send them to the
*           ingress pipe
*          continue till no more packets
*/
GT_VOID gdmaTreatFromCpuQueuePackets
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32                  gdma_id,
    IN GT_U32                  ringId,
    IN GDMA_SHARED_DATA_STC    *sharedData,
    OUT GT_BOOL                *queueEmptyPtr,
    OUT GT_BOOL                *isLastPacketPtr
)
{
    DECLARE_FUNC_NAME(gdmaTreatFromCpuQueuePackets);

    GT_U32  mutexUsed;
    GT_U32  sgdIdx;

    /* packet from cpu (DMA)*/
    __LOG((SIM_LOG_INGRESS_PACKET_FROM_CPU_DMA_STR " \n"));

    /* increment the number of packets in the system */
    skernelNumOfPacketsInTheSystemSet(GT_TRUE);

    simLogPacketFrameUnitSet(SIM_LOG_FRAME_UNIT_GDMA_TO_CPU_E);

    SIM_LOG_PACKET_DESCR_SAVE


    descrPtr->frameBuf->actualDataSize = SBUF_DATA_SIZE_CNS;

    /* Fill frame descriptor from Tx GDMA descriptor */
    if (sharedData->pktGenMagic == GDMA_PKT_GEN_MAGIC)
    {
        sgdIdx = sharedData->sgdIdx;
        sharedData->pktGenMagic = 0;

        gdmaTreatFromCpuPacketsPktGen(devObjPtr,descrPtr,
            gdma_id, ringId, sgdIdx,
            descrPtr->frameBuf->actualDataPtr,
            descrPtr->frameBuf->actualDataSize,
            queueEmptyPtr,
            isLastPacketPtr);
        /* <Ring is idle> done by PktGen task according to PktGen logic */
    }
    else
    {
        gdmaTreatFromCpuPackets(devObjPtr,descrPtr,
            gdma_id,ringId,
            descrPtr->frameBuf->actualDataPtr,
            descrPtr->frameBuf->actualDataSize,
            queueEmptyPtr,
            isLastPacketPtr);
            /* state that <Ring is idle> */
            smemRegFldSet(devObjPtr,SMEM_SIP7_GDMA_QUEUE_EN_REG(devObjPtr,gdma_id,ringId), 1,1,RING_IDLE);
    }

    SIM_LOG_PACKET_DESCR_COMPARE("gdmaTreatFromCpuQueuePackets : prepare packet from CPU GDMA");

    if((*queueEmptyPtr) == GT_FALSE)
    {
        if(devObjPtr->portGroupSharedDevObjPtr || simLogIsOpenFlag)
        {
            mutexUsed = 1;
            /* protect the full processing */
            SIM_OS_MAC(simOsMutexLock)(LOG_fullPacketWalkThroughProtectMutex);
        }
        else
        {
            mutexUsed = 0;
        }

        simLogPacketFrameUnitSet(SIM_LOG_FRAME_UNIT_RXDMA_E);

        snetChtIngress(devObjPtr, descrPtr);

        if(mutexUsed)
        {
            /* protect the egress processing */
            SIM_OS_MAC(simOsMutexUnlock)(LOG_fullPacketWalkThroughProtectMutex);
        }
    }

    /* decrement the number of packets in the system */
    skernelNumOfPacketsInTheSystemSet(GT_FALSE);

    /* packet from cpu (DMA) - Ended */
    __LOG((SIM_LOG_INGRESS_PACKET_FROM_CPU_DMA_ENDED_STR " \n"));
}

/**
* @internal snetAasGdmaFromCpuProcess function
* @endinternal
*
* @brief   GDMA : sip7 : Process GDMA queue (ring) for 'from_cpu' processing
*   called by message from snetAasGdmaFromCpuTrigger(...) to the 'skernel task'
*   that was triggered by active memory of 'queue enable' and
*   from smemAasActiveWriteCnmGdmaAddGdmaSgdOwn(...) when free_sgd_num was 0 and
*   queue was not in cyclic mode.
*
*/
static GT_VOID snetAasGdmaFromCpuProcess
(
    IN SKERNEL_DEVICE_OBJECT *devObjPtr,
    IN SBUF_BUF_ID            bufferId,
    IN GT_U8                 *dataPtr,
    IN GT_U32                 dataLength
)
{
    GT_U32  gdma_id;
    GT_U32  ringId;
    GT_BOOL queueEmpty;     /* queue is empty */
    GT_BOOL isLastPacket;   /* is last packet */
    SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr; /* frame descriptor pointer */
    GDMA_SHARED_DATA_STC  *sharedData = (void*)dataPtr;

    SIM_OS_MAC(simOsTaskOwnTaskPurposeSet)(
        SIM_OS_TASK_PURPOSE_TYPE_PP_PIPE_PROCESSING_DAEMON_E,NULL);
    /*state from cpu command ()*/
    simLogPacketFrameCommandSet(SIM_LOG_FRAME_COMMAND_TYPE_FROM_CPU_E);

    descrPtr = snetChtEqDuplicateDescr(devObjPtr, NULL);

    descrPtr->frameBuf = bufferId;
    descrPtr->ingressDevObjPtr = devObjPtr;

    /* parse the data from the buffer of the message */
    gdma_id = sharedData->gdma_id;
    ringId = sharedData->ringId;

    do
    {
        gdmaTreatFromCpuQueuePackets(devObjPtr,
            descrPtr,gdma_id,ringId,
            sharedData,
            &queueEmpty,
            &isLastPacket);
    }while(queueEmpty == GT_FALSE &&
         isLastPacket == GT_FALSE);

    return;
}

/**
* @internal snetAasGdmaFromCpuTrigger function
* @endinternal
*
* @brief   GDMA : sip7 : trigger the queue (ring) to send message of
*   SMAIN_MSG_TYPE_GENERIC_FUNCTION_E to the 'skernel task' , so it will call
*   snetAasGdmaFromCpuProcess .
*   this is called from active memories of :
*   1. 'queue enable'
*   2. smemAasActiveWriteCnmGdmaAddGdmaSgdOwn(...) when free_sgd_num was 0 and
*       queue was not in cyclic mode.
*
*/
GT_VOID snetAasGdmaFromCpuTrigger
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                  gdma_id,
    IN GT_U32                  ringId
)
{
    SBUF_BUF_ID bufferId;               /* buffer */
    GT_U8  *dataPtr;                   /* pointer to the data in the buffer */
    GT_U32 dataSize;                    /* data size */
    GENERIC_MSG_FUNC genFunc = snetAasGdmaFromCpuProcess;/* generic function */
    GT_U32  retry_allowed    = 10;

    do
    {
        /* Get buffer -- allocate size for max supported frame size */
        bufferId = sbufAlloc(devObjPtr->bufPool, SBUF_DATA_SIZE_CNS);/*12000*/
        if(bufferId == NULL)
        {
            /* free the lock of the SCIB , to allow the sKernel thread to take it
               as the releasing of the buffer involve this lock */
            SCIB_SEM_SIGNAL;
            /* let the sKernel thread time to free buffers */
            SIM_OS_MAC(simOsSleep)(50);
            /* restore the lock */
            SCIB_SEM_TAKE;
        }
    }
    while((retry_allowed--) && bufferId == NULL);
    if (bufferId == NULL)
    {
        simWarningPrintf(" snetAasGdmaFromCpuTrigger: no buffers to start TX GDMA\n");

        return;
    }

    /* Get actual data pointer */
    sbufDataGet(bufferId, (GT_U8 **)&dataPtr, &dataSize);

    /* put the name of the function into the message */
    memcpy(dataPtr,&genFunc,sizeof(GENERIC_MSG_FUNC));
    dataPtr+=sizeof(GENERIC_MSG_FUNC);

    /* save gdma_id */
    memcpy(dataPtr,&gdma_id,sizeof(GT_U32));
    dataPtr+=sizeof(GT_U32);

    /* save ringId */
    memcpy(dataPtr,&ringId,sizeof(GT_U32));
    dataPtr+=sizeof(GT_U32);

    /* set source type of buffer  */
    bufferId->srcType = SMAIN_SRC_TYPE_CPU_E;

    /* set message type of buffer */
    bufferId->dataType = SMAIN_MSG_TYPE_GENERIC_FUNCTION_E;

    /* put buffer to queue        */
    squeBufPut(devObjPtr->queueId, SIM_CAST_BUFF(bufferId));
}

/* 4 words (16 bytes) to allow padded ZEROs (actually 12 bytes is enough) */
static  GT_U32  zeroPaddedBuffer[4] = {0,0,0,0};
/* the FIRST,LAST bits in the SGD for MSG not set as 1 , but set at 0 ! */
static const GT_U32 MSG_firstBit_to_set = 0;
static const GT_U32 MSG_lastBit_to_set  = 0;

/**
* @internal gdmaSendMsgToCpu function
* @endinternal
*
* @brief   GDMA : Send A message to the CPU from a client (AUQ/FUQ/CNC/IPE)
*          NOTE: the client not know to which GDMA unit it is connected to
*
* @param[in] devObjPtr                - pointer to device.
* @param[in] client                   - the client AUQ/FUQ/CNC/IPE that send the message
* @param[in] gdma_id                   - the gdma unit id that serve the client
* @param[in] ringId                   - the ring that the client uses
* @param[in] wordsDataPtr             - pointer to data (array of words - each word 32 bits)
* @param[in] numOfWords               - number of words in wordsDataPtr
* @param[in] minSizeNeededInBuffer    - minimal number of bytes in the buffer needed
*                                       for the 'next' message (not current) , so
*                                       if after putting current message there is
*                                       less than this 'min' in the buffer , the SGD is closed
*                                       with current message , and next message will use next SGD
* @param[out] needRetryPtr            - (pointer to) the need for retry for this message
*                                       GT_TRUE  - the caller need to retry to send the message due to 'wait' mode
*                                       GT_FALSE - the caller NOT need to retry to send the message
*                                       (msg sent or not allowed , but without retry)
*/
static void  gdmaSendMsgToCpu
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SNET_CHT_DMA_CLIENT_ENT dmaClient,
    IN GT_U32                  gdma_id,
    IN GT_U32                  ringId,
    IN GT_U32                  *wordsDataPtr,
    IN GT_U32                  numOfWords,
    IN GT_U32                  minSizeNeededInBuffer,
    IN GT_BOOL                 *needRetryPtr,
    IN const char*              clientName
)
{
    DECLARE_FUNC_NAME(gdmaSendMsgToCpu);

    GT_U32  alignToNext16Bytes;         /* number of bytes to align the message to next 16 bytes */
    GT_U32  buffLen;                    /* Length of the current buffer */
    GT_U32  sentBytes;                  /* Number of bytes already sent */
    GT_U32  sgd_remaining_bc;           /* This field reflects the value of the remaining space in the current RX buffer (in bytes) */
    GT_U32  regAddress;                 /* Register address */
    GT_U32  *counterPtr;                /* counter pointer*/
    GT_U32  fldValue;                   /* Register field value */
    INTERRUPT_BMP_TYPE_ENT  intType_bmp = 0;
    GDMA_DESC_EXTENDED curDescrInfo;    /* RX descriptor */
    GT_U32      *rxDesc_Ptr;            /* Pointer to RX descriptor */
    GT_U32      currentDmaAddr[2];      /* Current RX descriptor address */
    GT_U32      startDmaAddr[2];
    GT_U32      ringEnable;
    GT_U32      q_type,sgd_recycle_chain,sgd_extend_mode,q_wrr_weight;
    GT_U32      wru_sgd_pointer_value;
    GT_U32      SGD_Chain_Size;
    GT_U32      free_sgd_num;
    GT_U32      rx_buf_bc_mode,rx_buf_bc;
    GT_U32      dest_addr_high;
    GT_U32      temp_address[2];
    GT_U32      ownBit;
    GT_U32      lor_error = 0;
    GT_U32      sizeof_sgd; /* size of the SGD in bytes */
    GT_U32      opcode;
    GT_U32      lor_mode;
    GT_U32      numWaitIterations = 0;
    GT_U32      dataSize = 4 * numOfWords;
    GT_BIT      closeTheSgd = 0;/* indication to close the current SGD */

    *needRetryPtr = GT_FALSE;

    SCIB_SEM_TAKE;
    smemRegFldGet(devObjPtr,SMEM_SIP7_GDMA_QUEUE_EN_REG(devObjPtr,gdma_id,ringId), 0,1,&ringEnable);
    if (ringEnable == 0)
    {
        SCIB_SEM_SIGNAL;

        __LOG(("GDMA to MSG CPU : ringId[%d] is disabled (so not send MSG to CPU) \n",
            ringId));
        return;
    }

    /* state that <Ring is busy> */
    smemRegFldSet(devObjPtr,SMEM_SIP7_GDMA_QUEUE_EN_REG(devObjPtr,gdma_id,ringId), 1,1,RING_BUSY);
    SCIB_SEM_SIGNAL;

    regAddress = SMEM_SIP7_GDMA_CHAIN_ADDRESS_LOW_REG(devObjPtr,gdma_id, ringId);
    smemRegGet(devObjPtr, regAddress,  &startDmaAddr[0]);
    regAddress = SMEM_SIP7_GDMA_CHAIN_ADDRESS_HIGH_REG(devObjPtr,gdma_id, ringId);
    smemRegGet(devObjPtr, regAddress,  &startDmaAddr[1]);

    /* Get the descriptor size regular mode is 8B and extended mode is 16B*/
    regAddress = SMEM_SIP7_GDMA_QUEUE_CONTROL_REG(devObjPtr,gdma_id, ringId);
    smemRegGet(devObjPtr, regAddress , &fldValue );
    sgd_recycle_chain = SMEM_U32_GET_FIELD(fldValue, 4, 1);
    sgd_extend_mode = SMEM_U32_GET_FIELD(fldValue, 5, 1);
    q_type = SMEM_U32_GET_FIELD(fldValue, 0, 4);
    q_wrr_weight = SMEM_U32_GET_FIELD(fldValue, 20, 4);

    if(q_wrr_weight == 0)
    {
        /* behave just as 'ring disable' */
        __LOG(("ERROR : GDMA MSG to CPU : q_wrr_weight == 0 --> act as ring disabled (so not send MSG to CPU) \n"));
        return;
    }

    lor_mode = SMEM_U32_GET_FIELD(fldValue, 6, 1);
    if(lor_mode == 1)
    {
        __LOG(("LOR mode is 'WAIT' \n"));
    }
    else
    {
        __LOG(("LOR mode is 'DROP' \n"));
    }

    sizeof_sgd = (sgd_extend_mode == EXTENDED) ? 16 : 8;

    if(TO_CPU_MSG_MODE_RING_TYPE != q_type)
    {
        __LOG(("ERROR : GDMA MSG to CPU : ringId[%d] q_type[%d] is NOT for 'TO_CPU_MSG_MODE' (so not send MSG to CPU) \n",
            ringId,q_type));
        return;
    }

    /* this is NOT counter , but config for 'counting' */
    regAddress = SMEM_SIP7_GDMA_RX_BUF_BYTE_CNT_REG(devObjPtr,gdma_id, ringId);
    smemRegGet(devObjPtr, regAddress , &fldValue );
    rx_buf_bc_mode = SMEM_U32_GET_FIELD(fldValue, 16,  1);
    rx_buf_bc      = SMEM_U32_GET_FIELD(fldValue,  0, 16);

    __LOG_PARAM(rx_buf_bc_mode);
    __LOG_PARAM(rx_buf_bc);

    if(simLogIsOpenFlag)
    {
        GT_U32  ii;
        __LOG_PARAM(numOfWords);
        __LOG(("MSG:"));
        for (ii = 0 ; ii < numOfWords; ii++)
        {
            __LOG(("[%8.8x],",
                wordsDataPtr[ii]));
        }
        __LOG(("\n"));
    }

    /* Start from first descriptor address */
    regAddress = SMEM_SIP7_GDMA_BASE_DESC_POINTER_REG(devObjPtr,gdma_id, ringId);
    smemRegGet(devObjPtr, regAddress , &fldValue );
    wru_sgd_pointer_value = SMEM_U32_GET_FIELD(fldValue, 0, 16);
    sgd_remaining_bc      = SMEM_U32_GET_FIELD(fldValue,16, 16);

    __LOG_PARAM(wru_sgd_pointer_value);
    __LOG_PARAM(sgd_remaining_bc);

    /* Calculate the currentDescp address based on current Descriptor position*/
    currentDmaAddr[0] = startDmaAddr[0] + wru_sgd_pointer_value * sizeof_sgd;
    currentDmaAddr[1] = startDmaAddr[1];
    if(currentDmaAddr[0] < startDmaAddr[0])
    {
        /* wrap around of the address 32LSBits */
        currentDmaAddr[1]++;
    }

    if(sgd_extend_mode == REGULAR)
    {
        regAddress = SMEM_SIP7_GDMA_PAYLOAD_BUFFER_ADDRESS_HIGH_REG(devObjPtr,gdma_id, ringId);
        smemRegGet(devObjPtr, regAddress , &fldValue );
        dest_addr_high = SMEM_U32_GET_FIELD(fldValue, 16, 14);/* for bits 34..47 of the address */
    }
    else
    {
        dest_addr_high = 0;/* the info is in the SGD */
    }

    rxDesc_Ptr = &curDescrInfo.regulare.words[0];

    regAddress = SMEM_SIP7_GDMA_CHAIN_SIZE_REG(devObjPtr,gdma_id, ringId);
    smemRegGet(devObjPtr, regAddress , &fldValue );
    SGD_Chain_Size = SMEM_U32_GET_FIELD(fldValue,  0, 16);

    if(wru_sgd_pointer_value >= SGD_Chain_Size)
    {
        __LOG(("ERROR : GDMA MSG to CPU : ringId[%d] wru_sgd_pointer_value[%d] >= SGD_Chain_Size[%d] (so not send MSG to CPU) \n",
            ringId,q_type,
            wru_sgd_pointer_value,SGD_Chain_Size));
        return;
    }

    regAddress = SMEM_SIP7_GDMA_FREE_SGDS_STATUS_REG(devObjPtr,gdma_id, ringId);
    smemRegGet(devObjPtr, regAddress , &fldValue );
    free_sgd_num = SMEM_U32_GET_FIELD(fldValue,  0, 16);

    __LOG_PARAM(free_sgd_num);

    /* work with single descriptor , as data never split between descriptors */
    if(free_sgd_num == 0)
    {
        if(lor_mode)
        {
            __LOG(("we are in WAIT mode with this MSG , as no more free descriptors left in the ring need to try again later \n"));
            /* in WM the unit need to send a new message on the 'skernel'
               to handle the retry */
            *needRetryPtr = GT_TRUE;
            return;/* need retry */
        }

        while(free_sgd_num == 0)
        {
            /* allow 10 retries also for 'DROP mode' (lor_mode==1)
               as the CPU may a desc by this time
            */

            /* let the Application time to process the messages , so the list will be free */
            SIM_OS_MAC(simOsSleep)(1);
            regAddress = SMEM_SIP7_GDMA_FREE_SGDS_STATUS_REG(devObjPtr,gdma_id, ringId);
            smemRegGet(devObjPtr, regAddress , &fldValue );
            free_sgd_num = SMEM_U32_GET_FIELD(fldValue,  0, 16);

            numWaitIterations++;

            if(numWaitIterations >= 10)
            {
                __LOG(("WARNING : we are done with this MSG , as no more free descriptors left in the ring \n"));
                /* message not sent BUT we need to loose it , as the mode is 'drop'
                    so state that we not need retry on it */
                __LOG(("DROP interrupt (msg_drop_int) \n"));
                intType_bmp |= msg_drop_int;

                /* generate the interrupt(s) */
                gdmaGenInterrupt(devObjPtr, gdma_id, ringId , intType_bmp);

                goto handleDrop_lbl;
            }
        }
    }

    /* Read the DMA for needed value, put data into current descriptor */
    __LOG(("Read the DMA for needed value, put data into current descriptor"));
    snetChtPerformScibDmaRead_64BitAddr(dmaClient,gdma_id,
                devObjPtr->deviceId,currentDmaAddr[0],currentDmaAddr[1],
                NUM_BYTES_TO_WORDS(sizeof_sgd),
                rxDesc_Ptr, SCIB_DMA_WORDS);

    ownBit =
        GDMA_RX(devObjPtr,rxDesc_Ptr,wru_sgd_pointer_value,
            SMEM_SIP7_GDMA_SW_GDMA_DESCRIPTOR_RX_FIELD_OWN);

    if(ownBit == OWNER_CPU)/* for the TO_CPU the HW ignore the read value of ownBit */
    {
        __LOG(("NOTE : TO_CPU : the ownBit is 'OWNER_CPU' but the HW will treat it anyway (as owned by HW) \n"));
    }

    opcode =
        GDMA_RX(devObjPtr,rxDesc_Ptr,wru_sgd_pointer_value,
            SMEM_SIP7_GDMA_SW_GDMA_DESCRIPTOR_RX_FIELD_OPCODE);

    if(opcode != 0)
    {
        /* was not implemented on the design (on real HW) */
        __LOG(("opcode == 1 was not implemented (so the value is ignored) \n"));
    }

    if(rx_buf_bc_mode == 1)
    {
        buffLen = rx_buf_bc;
    }
    else
    {
        buffLen =
            GDMA_RX(devObjPtr,rxDesc_Ptr,wru_sgd_pointer_value,
                SMEM_SIP7_GDMA_SW_GDMA_DESCRIPTOR_RX_FIELD_BUFFER_BYTE_COUNT);
    }
    buffLen += 1;

    if (buffLen  < minSizeNeededInBuffer)
    {
        __LOG(("configuration error : the buffer byte count[%d] is less than [%d] (minimal size) \n",
            buffLen,minSizeNeededInBuffer));
        return;
    }

    if(sgd_remaining_bc == 0)
    {
        sentBytes = 0;
        /* we start putting info in the buffer */
        sgd_remaining_bc = buffLen - dataSize;
    }
    else
    if (sgd_remaining_bc >= dataSize)
    {
        /* this is how much we already put on the buffer */
        sentBytes = buffLen - sgd_remaining_bc;
        sgd_remaining_bc -= dataSize;
    }
    else
    {
        sentBytes = buffLen - sgd_remaining_bc;
        __LOG(("ERROR : management error on sgd_remaining_bc[%d] is less than dataSize[%d] \n",
            sgd_remaining_bc, dataSize));
    }

    __LOG(("Already treated [%d] bytes on current SGD \n",
        sentBytes));

    /* The 'next' MSG can't be split to 2 descriptors */
    if ((buffLen - (sentBytes+dataSize)) < minSizeNeededInBuffer)
    {
        closeTheSgd = 1;
    }

    __LOG_PARAM(closeTheSgd);

    /* and simulation NOT know (yet) to accumulate several messages on the same buffer */
    /* as simulation not implemented yet the timer that controls that */
    buffLen = dataSize;

    temp_address[0] =
        GDMA_RX(devObjPtr,rxDesc_Ptr,wru_sgd_pointer_value,
            SMEM_SIP7_GDMA_SW_GDMA_DESCRIPTOR_RX_FIELD_BUFFER_DST_ADDR_LOW_0);
    temp_address[1] =
        GDMA_RX(devObjPtr,rxDesc_Ptr,wru_sgd_pointer_value,
            SMEM_SIP7_GDMA_SW_GDMA_DESCRIPTOR_RX_FIELD_BUFFER_DST_ADDR_LOW_1);

    if(sgd_extend_mode == EXTENDED)
    {
        dest_addr_high = GDMA_RX(devObjPtr,rxDesc_Ptr,wru_sgd_pointer_value,
            SMEM_SIP7_GDMA_SW_GDMA_DESCRIPTOR_RX_FIELD_BUFFER_DST_ADDR_HIGH);
    }
    temp_address[1] |= dest_addr_high << 2;

    if(temp_address[0] & 0xF)
    {
        /* Since TO_CPU works in 16B alignment */
        __LOG(("config ERROR : the buffer address [0x%8.8x] is not aligned on 16 bytes , so will be considered as address [0x%8.8x] \n",
            temp_address[0], temp_address[0] & 0xFFFFFFF0));

        temp_address[0] &= 0xFFFFFFF0;
    }

    /* step into the buffer according to what already put on the buffer */
    if((temp_address[0] + sentBytes) < temp_address[0])
    {
        /* wrap around of the address 32LSBits */
        temp_address[0] = temp_address[0] + sentBytes;
        temp_address[1]++;
    }
    else
    {
        temp_address[0] = temp_address[0] + sentBytes;
    }

    /* Write data into the DMA buffer */
    snetChtPerformScibDmaWrite_64BitAddr(dmaClient,gdma_id,
                devObjPtr->deviceId,temp_address[0/*low*/],temp_address[1/*high*/],
                 NUM_BYTES_TO_WORDS(buffLen),
                 &wordsDataPtr[0], TX_BYTE_ORDER_MAC(devObjPtr));

    if(buffLen & 0xF)/*%16*/
    {
        /* we need to align the massage to 16 bytes */
        alignToNext16Bytes = 16 - (buffLen & 0xF);

        /* Write ZERO PADDED data into the DMA buffer */
        snetChtPerformScibDmaWrite_64BitAddr(dmaClient,gdma_id,
                    devObjPtr->deviceId,temp_address[0/*low*/]+buffLen,temp_address[1/*high*/],
                     NUM_BYTES_TO_WORDS(alignToNext16Bytes),
                     &zeroPaddedBuffer[0], TX_BYTE_ORDER_MAC(devObjPtr));

        buffLen += alignToNext16Bytes;
        sgd_remaining_bc -= alignToNext16Bytes;
    }

    if (sentBytes == 0)
    {
        /* set First bit */
        GDMA_RX_SET(devObjPtr,rxDesc_Ptr,wru_sgd_pointer_value,
            SMEM_SIP7_GDMA_SW_GDMA_DESCRIPTOR_RX_FIELD_F_BIT,
            MSG_firstBit_to_set);
    }

    /* Increment send bytes counter */
    sentBytes += buffLen;

    if(!closeTheSgd)
    {
        /* save value for next time as we continue with current SGD */
        regAddress = SMEM_SIP7_GDMA_BASE_DESC_POINTER_REG(devObjPtr,gdma_id, ringId);
        smemRegFldSet(devObjPtr, regAddress , 16,16 , sgd_remaining_bc );
    }
    else
    {

        __LOG(("[%s] : Closing SGD[%d] : for GDMA MSG : on ringId[%d] in GDMA unit[%d] \n",
            clientName,
            wru_sgd_pointer_value,
            ringId,
            gdma_id));

        /* update the actual number of bytes in the buffer */
        GDMA_RX_SET(devObjPtr,rxDesc_Ptr,wru_sgd_pointer_value,
            SMEM_SIP7_GDMA_SW_GDMA_DESCRIPTOR_RX_FIELD_BUFFER_BYTE_COUNT,
            sentBytes - 1);

        /* set Last bit */
        GDMA_RX_SET(devObjPtr,rxDesc_Ptr,wru_sgd_pointer_value,
            SMEM_SIP7_GDMA_SW_GDMA_DESCRIPTOR_RX_FIELD_L_BIT,
            MSG_lastBit_to_set);

        GDMA_RX_SET(devObjPtr,rxDesc_Ptr,wru_sgd_pointer_value,
            SMEM_SIP7_GDMA_SW_GDMA_DESCRIPTOR_RX_FIELD_OWN,
            OWNER_CPU);

        /* only word 0 hold info that the device may have changed */
        __LOG(("Write back the the first word of current descriptor \n"));
        snetChtPerformScibDmaWrite_64BitAddr(dmaClient,gdma_id,
                    devObjPtr->deviceId,currentDmaAddr[0],currentDmaAddr[1],
                    1/*single word*/,
                    rxDesc_Ptr, SCIB_DMA_WORDS);
        /* decrement the 'free_sgd_num' (but make sure to SCIB lock the operation)
           as the active memory may want to update it */
        SCIB_SEM_TAKE;
        regAddress = SMEM_SIP7_GDMA_FREE_SGDS_STATUS_REG(devObjPtr,gdma_id, ringId);
        /* we read it under LOCK , as maybe changed by active memory after we unlocked the SCIB*/
        smemRegGet(devObjPtr, regAddress , &fldValue );
        free_sgd_num = SMEM_U32_GET_FIELD(fldValue,  0, 16);
        if(free_sgd_num == 0)
        {
            /* should not happen */
            skernelFatalError("gdmaSendMsgToCpu: free_sgd_num == 0 when need to decrement --> bad management \n");
        }
        else
        {
            free_sgd_num --;
            __LOG_PARAM(free_sgd_num);
        }

        smemRegFldSet(devObjPtr, regAddress , 0, 16, free_sgd_num);
        SCIB_SEM_SIGNAL;

        __LOG(("invoke interrupt about SGD Done (rx_sgd_done_int) \n"));
        intType_bmp |= rx_sgd_done_int;

        /* update the current pointer of the ring */
        wru_sgd_pointer_value++;
        if(wru_sgd_pointer_value >= SGD_Chain_Size)
        {
            /* restart for next time */
            wru_sgd_pointer_value = 0;
            __LOG_PARAM(wru_sgd_pointer_value);
        }
        __LOG_PARAM(wru_sgd_pointer_value);

        if(wru_sgd_pointer_value == 0 &&
           sgd_recycle_chain == 0)
        {
            ringEnable = 0;
            smemRegFldSet(devObjPtr,SMEM_SIP7_GDMA_QUEUE_EN_REG(devObjPtr,gdma_id,ringId), 0,1,ringEnable);
        }

        /* Update the next descriptor to handle */
        regAddress = SMEM_SIP7_GDMA_BASE_DESC_POINTER_REG(devObjPtr,gdma_id, ringId);
        fldValue = 0;
        SMEM_U32_SET_FIELD(fldValue,  0, 16 , wru_sgd_pointer_value);
        SMEM_U32_SET_FIELD(fldValue, 16, 16 , 0/*sgd_remaining_bc*/);
        smemRegSet(devObjPtr, regAddress , fldValue);
    }

    /* Upon every MSG 'done' (regardless to drop) */
    __LOG(("invoke interrupt about 'MSG to CPU' (msg_done_int) \n"));
    intType_bmp = msg_done_int;

    if(free_sgd_num == 0)
    {
        /* LOR (internal/debug) Interrupt shall be asserted whenever a PDI2MEM R
        doesn't have any SGDs (independent of whether there is incoming traffic
        or not)*/
        __LOG(("invoke interrupt no SGDs (lor_int) \n"));
        intType_bmp |= lor_int;

        if(sgd_recycle_chain == 0)
        {
            /* Whenever the SGD chain is an open chain (Recycle = Disable) and
               the last SGD in the chain has been served, the R will be automatically
               disabled by HW*/
            __LOG(("'open chain' mode (non-cyclic) : used last free SGD , so disabling the Ring \n"));
            ringEnable = 0;
            smemRegFldSet(devObjPtr,SMEM_SIP7_GDMA_QUEUE_EN_REG(devObjPtr,gdma_id,ringId), 0,1,ringEnable);
        }
    }

    /* generate the interrupt(s) */
    gdmaGenInterrupt(devObjPtr, gdma_id, ringId , intType_bmp);

    /* update the counters */
    if(! lor_error)
    {
        /* increment the msg counter */
        regAddress = SMEM_SIP7_GDMA_STAT_COUNTER_REG(devObjPtr,gdma_id, ringId);
        counterPtr = smemMemGet(devObjPtr, regAddress);
        __LOG(("GDMA ringId[%d] : Increment GDMA MSG OK count counter by 1 from[%d] \n",
            ringId,counterPtr[0]));
        gdmaInc64BitCounter(counterPtr , 1);

        /* increment the msg byte counter */
        regAddress = SMEM_SIP7_GDMA_STAT_BYTE_COUNTER_REG(devObjPtr,gdma_id, ringId);
        counterPtr = smemMemGet(devObjPtr, regAddress);
        __LOG(("GDMA ringId[%d] : Increment GDMA bytes OK count counter by [%d] from [%d] \n",
            ringId,buffLen,counterPtr[0]));
        gdmaInc64BitCounter(counterPtr , buffLen);

        if(closeTheSgd)
        {
            __LOG(("'MSG to CPU' of [%d] bytes send to CPU (SGD was closed now) \n",
                buffLen));
            __LOG(("Final Egress total messages info on the buffer is [%d] bytes  \n",
                sentBytes));
        }
        else
        {
            __LOG(("'MSG to CPU' of [%d] bytes send to CPU (SGD not closed yet) \n",
                buffLen));
        }
    }
    else
    {
        handleDrop_lbl:

        /* increment the drop counter */
        regAddress = SMEM_SIP7_GDMA_RX_DROP_COUNTER_REG(devObjPtr,gdma_id, ringId);
        counterPtr = smemMemGet(devObjPtr, regAddress);
        __LOG(("GDMA ringId[%d] : Increment GDMA MSG DROP count counter by 1 from[%d] \n",
            ringId,counterPtr[0]));
        gdmaInc64BitCounter(counterPtr , 1);
    }

    return;
}


/**
* @internal snetSip7GdmaSendMsgToCpu function
* @endinternal
*
* @brief   GDMA : Send A message to the CPU from a client (AUQ/FUQ/CNC/IPE)
*          NOTE: the client not know to which GDMA unit it is connected to
*
* @param[in] devObjPtr                - pointer to device.
* @param[in] client                   - the client AUQ/FUQ/CNC/IPE that send the message
* @param[in] ringId                   - the ring that the client uses
* @param[in] wordsDataPtr             - pointer to data (array of words - each word 32 bits)
* @param[in] numOfWords               - number of words in wordsDataPtr
* @param[out] needRetryPtr            - (pointer to) the need for retry for this message
*                                       GT_TRUE  - the caller need to retry to send the message due to 'wait' mode
*                                       GT_FALSE - the caller NOT need to retry to send the message
*                                       (msg sent or not allowed , but without retry)
*
*/
void snetSip7GdmaSendMsgToCpu
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SNET_SIP7_GDMA_MSG_CLIENT_ENT client,
    IN GT_U32                  ringId,
    IN GT_U32                  *wordsDataPtr,
    IN GT_U32                  numOfWords,
    IN GT_BOOL                 *needRetryPtr
)
{
    DECLARE_FUNC_NAME(snetSip7GdmaSendMsgToCpu);

    GT_U32 gdma_id;
    GT_U32 regAddress,fldValue;
    GT_U32 ipe_msg_size,ipeNumWords;
    SNET_CHT_DMA_CLIENT_ENT dmaClient;
    GT_U32 minSizeNeededInBuffer;/*size in bytes*/

    const static char*  clientsName[] = {
    STR(SNET_SIP7_GDMA_MSG_CLIENT_FDB_AUQ_E),
    STR(SNET_SIP7_GDMA_MSG_CLIENT_FDB_FUQ_E),
    STR(SNET_SIP7_GDMA_MSG_CLIENT_CNC_E),
    STR(SNET_SIP7_GDMA_MSG_CLIENT_IPE_E)
    };
    const char*  clientName;

    *needRetryPtr = GT_FALSE;

    switch(client)
    {
        case SNET_SIP7_GDMA_MSG_CLIENT_FDB_AUQ_E:
        case SNET_SIP7_GDMA_MSG_CLIENT_FDB_FUQ_E:
            minSizeNeededInBuffer = 64;/* 'next' message can be 24/56 bytes */
            dmaClient = (client == SNET_SIP7_GDMA_MSG_CLIENT_FDB_AUQ_E) ? SNET_CHT_DMA_CLIENT_AUQ_E : SNET_CHT_DMA_CLIENT_FUQ_E;
            gdma_id = devObjPtr->gdmaInfo.gdmaUnitForFdbMsg;
            break;
        case SNET_SIP7_GDMA_MSG_CLIENT_CNC_E:
            minSizeNeededInBuffer = 16;/* all messages are 16 bytes (so also the 'next' )*/
            dmaClient = SNET_CHT_DMA_CLIENT_CNC_E;
            gdma_id = devObjPtr->gdmaInfo.gdmaUnitForCncMsg;
            break;
        case SNET_SIP7_GDMA_MSG_CLIENT_IPE_E:
            minSizeNeededInBuffer = 16;/* 'next' message can be 4/8/16 bytes */
            dmaClient = SNET_CHT_DMA_CLIENT_IPE_E;
            gdma_id = devObjPtr->gdmaInfo.gdmaUnitForIpeMsg;

            /* Get the descriptor size regular mode is 8B and extended mode is 16B*/
            regAddress = SMEM_SIP7_GDMA_QUEUE_CONTROL_REG(devObjPtr,gdma_id, ringId);
            smemRegGet(devObjPtr, regAddress , &fldValue );
            ipe_msg_size = SMEM_U32_GET_FIELD(fldValue, 26, 2);
            /*
                0x0 = msg_size_4; msg_size_4; IPE message size is 4 bytes; IPE message size is 4 bytes
                0x1 = msg_size_8; msg_size_8; IPE message size is 8 bytes; IPE message size is 8 bytes
                0x2 = msg_size_16; msg_size_16; IPE message
            */
            __LOG_PARAM(ipe_msg_size);

            ipeNumWords = 4 << ipe_msg_size;

            if(ipeNumWords/*from config*/ != numOfWords/*from client*/)
            {
                __LOG(("ERROR : config of 'ipe_msg_size' (ipeNumWords[%d]) not match the number of words [%d] from the IPE client \n",
                    ipeNumWords,
                    numOfWords));

                /* The HW not have such a check , and rely on the 'config' value */
                __LOG(("So consider the numOfWords to be [%d] (instead of [%d]) \n",
                    ipeNumWords,
                    numOfWords));

                numOfWords = ipeNumWords;
            }

            break;
        default:
            /* should not happen */
            skernelFatalError("snetSip7GdmaSendMsgToCpu: unknown client[%d] \n",
                client);
            return ;
    }

    if((numOfWords*4) > minSizeNeededInBuffer)
    {
        __LOG(("WM ERROR : the MSG size [%d] bytes > minSizeNeededInBuffer[%d] \n",
            (numOfWords*4),
            minSizeNeededInBuffer));

        minSizeNeededInBuffer = (numOfWords*4);

        __LOG(("The WM will treat minSizeNeededInBuffer as [%d] \n",
            minSizeNeededInBuffer));
    }

    clientName = clientsName[client];

    __LOG(("GDMA MSG to CPU : on ringId[%d] Started , in GDMA unit[%d] [%s] \n",
        ringId,
        gdma_id,
        clientName));

    simLogPacketFrameUnitSet(SIM_LOG_FRAME_UNIT_GDMA_TO_CPU_E);

    __LOG_PARAM(gdma_id);

    /* start the operation in the proper ring */
    gdmaSendMsgToCpu(devObjPtr,dmaClient,gdma_id,ringId,wordsDataPtr,numOfWords,
        minSizeNeededInBuffer,needRetryPtr,clientName);

    /* state that <Ring is idle> */
    smemRegFldSet(devObjPtr,SMEM_SIP7_GDMA_QUEUE_EN_REG(devObjPtr,gdma_id,ringId), 1,1,RING_IDLE);

    __LOG(("GDMA MSG to CPU : on ringId[%d] Ended , in GDMA unit[%d] [%s] \n",
        ringId,
        gdma_id,
        clientName));

    simLogPacketFrameUnitSet(SIM_LOG_FRAME_UNIT___RESTORE_PREVIOUS_UNIT___E);

    return;
}


/**** GDMA 'Packet Generator' (PktGen) or the same 'Traffic Generator' (TG) *************
*/

/* SIMULATOR WA for "slow" Msg-handling task overloading
*    by Frequent PktGen enqueueing (on PktGen Timer disable or low timer value).
*  On a steady state sleep 1ms is enough, but on an overloading
*  (the escalating factor tgfTrafficGeneratorPortTxEthCaptureSet(DISABLE))
*  add extra sleep.
*/
#define PKT_GEN_MIN_INTERVAL_MSEC   1
#define WAIT_BUFF_BUSY_MSEC         3
#define WAIT_BUFF_BUSY_RETRIES      1

typedef struct {
    /* Common for PktGen and for other FromCpu */
    GENERIC_MSG_FUNC        genFunc;/* generic function */
    GDMA_SHARED_DATA_STC    shared;

    /* Private PktGen only used by Task */
    GT_SEM                  eventId; /* Sync-event */
    void                    *taskHandle; /* PktGen task per queue */
    SKERNEL_DEVICE_OBJECT   *devObjPtr;
    GDMA_SHARED_DATA_STC    *prevSharedData; /* shared-data - ptr in previous BufferId */

    /* Debug counters */
    GT_U32                  bufferAllocationCounter;
    GT_U32                  noBufferAllocationCounter;
    GT_U32                  bufferBusyCounter;
} GDMA_PKT_GEN_TASK_DATA_STC;

/**
* @internal pktGenBufferSetAndEnqueue function
* @endinternal
* @brief  Send MSG_TYPE_GENERIC to skernel task with BufferId for processing by
*        generic function  snetAasGdmaFromCpuProcess
*                             gdmaTreatFromCpuQueuePackets
*                                gdmaTreatFromCpuPackets
*
* @note  Refer also snetLion3SdmaTaskPerQueue, skernelCreatePacketGenerator
*/
static GT_VOID pktGenBufferSetAndEnqueue
(
    IN  SKERNEL_DEVICE_OBJECT       *devObjPtr,
    IN  GDMA_PKT_GEN_TASK_DATA_STC  *pktGenData
)
{
    SBUF_BUF_ID bufferId;   /* Buffer from bufPool */
    GT_U8       *dataPtr;   /* pointer to the data in the buffer */
    GT_U32      dataSize;   /* data size */
    GT_U32      retry = WAIT_BUFF_BUSY_RETRIES;
    GDMA_SHARED_DATA_STC  *sharedData;

    /* Get buffer -- allocate size for max supported frame size */
    pktGenData->bufferAllocationCounter++;
    bufferId = sbufAlloc(devObjPtr->bufPool, SBUF_DATA_SIZE_CNS);/*12000*/
    if (bufferId == NULL)
    {
        pktGenData->noBufferAllocationCounter++;
        simWarningPrintf("PktGen[%u:%d] no-buffer-cnt=%u (total=%u); pktGap=%u msec\n",
                    pktGenData->shared.gdma_id, pktGenData->shared.ringId,
                    pktGenData->noBufferAllocationCounter,
                    pktGenData->bufferAllocationCounter);
        devObjPtr = skernelSleep(devObjPtr, WAIT_BUFF_BUSY_MSEC);
        /* GT_NO_RESOURCE but nothing to do with that - drop this SGD and go ahead */
        return;
    }

    /* Get actual data pointer */
    sbufDataGet(bufferId, (GT_U8 **)&dataPtr, &dataSize);

    /* put all genFunc/unit/ring into the message */
    memcpy(dataPtr, pktGenData, sizeof(GENERIC_MSG_FUNC) + sizeof(GDMA_SHARED_DATA_STC));

    /* set source type of buffer */
    bufferId->srcType = SMAIN_SRC_TYPE_CPU_E;

    /* set message type of buffer */
    bufferId->dataType = SMAIN_MSG_TYPE_GENERIC_FUNCTION_E;

    /* Check simulator is not overloaded: when buffer is handled the MAGIC is cleaned */
    sharedData = &((GDMA_PKT_GEN_TASK_DATA_STC *)dataPtr)->shared;
    if (pktGenData->prevSharedData != NULL)
    {
        while (retry-- && (pktGenData->prevSharedData->pktGenMagic == GDMA_PKT_GEN_MAGIC))
        {
            pktGenData->bufferBusyCounter++; /* no any warn-print here */
            devObjPtr = skernelSleep(devObjPtr, WAIT_BUFF_BUSY_MSEC);
        }
    }
    pktGenData->prevSharedData = sharedData; /* save new buffer */

    /* put buffer to message queue */
    squeBufPut(devObjPtr->queueId, SIM_CAST_BUFF(bufferId));
    return;
}

/**
* @internal snetAasGdmaPktGenTask function
* @endinternal
*
* @brief    GDMA Packet Generator task transmitting packets placed into ring in SGD.
*           For each PktGen(unit:queue) has own handling Task(unit:queue).
*  Main register fields and logic are:
*   ring_idle           - cleared whilst task running
*   sgd_chain_size      - number of SGDs in ring processed whilst running
*   sgd_recycle_chain   - if set task goes over all SGDs till sgd_chain_size and starts again
*                         when cleared the task passes until sgd_chain_size, sets ring_idle
*                         and goes to self-suspend
*   timer_en, timer_val - a transition from SGD to next SGD is upon timer
*             ...       - send/process packet buffer/len described in SGD
*   q_en                - 'set' is the queue is enabled and PktGen is active,
*                         'clear' the task is self-suspend and waiting for wakeup
*
* @param[in] pktGenData  - Pointer to Tx GDMA PktGen object data
*                          (created  by snetAasGdmaFromCpuTriggerPktGen)
*/
static GT_VOID snetAasGdmaPktGenTask
(
    IN  GDMA_PKT_GEN_TASK_DATA_STC *pktGenData
)
{
    DECLARE_FUNC_NAME(snetAasGdmaPktGenTask);

    SKERNEL_DEVICE_OBJECT   *devObjPtr;
    GT_U32      gdmaUnit;
    GT_U32      txQueue;
    GT_U32      regAddr;    /* register address for read or write */
    GT_U32      regVal;     /* register's value - 32bits or a field */
    GT_U32      sgdIdx;     /* Index of GDMA SGD descriptor */
    GT_U32      queueEnableRegVal;
    GT_U32      queueControlRegVal;
    GT_U32      queueEnabled, queueEnabledPrev;
    GT_U32      isValid;
    GT_U32      txDesc[4];
    GT_U32      pktIntervalMSec; /* Packet Interval mSec configured in timer reg */
    GT_U32      sgdChainSize;
    GT_BOOL     chainEndedWithStopReq; /*STOP task if sgd_recycle_chain=0 and whole sgd_chain_size passed*/

#ifdef _WIN32
    /* call SHOST to register the application task in the asic task table*/
    SHOSTG_psos_reg_asic_task();
#endif /*_WIN32*/

    devObjPtr = pktGenData->devObjPtr;
    gdmaUnit = pktGenData->shared.gdma_id;
    txQueue = pktGenData->shared.ringId;
    chainEndedWithStopReq = GT_FALSE;
    sgdIdx = 0;

    skernelSleep(devObjPtr, 1); /* yield back to caller */

    while(1)
    {
        SCIB_SEM_TAKE;
        /* Get whole 'queueEnable' register info, to be used later */
        regAddr = SIP_7_REG(devObjPtr, CNM.GDMA[gdmaUnit].
                      queueStatusAndControlRegs[txQueue].queueEnable);
        smemRegGet(devObjPtr, regAddr, &queueEnableRegVal);

        if (chainEndedWithStopReq == GT_TRUE)
        {
            if (!!(queueEnableRegVal & BIT_1) == RING_BUSY)
            {
                /* busy in msg-Task handler - yield a bit but then force down */
                devObjPtr = skernelSleep(devObjPtr, 2);
            }
            /* Disable on the end of chain */
            queueEnableRegVal = 0;
        }

        queueEnabled = SMEM_U32_GET_FIELD(queueEnableRegVal, 0, 1);
        queueEnabledPrev = queueEnabled;

        /* {{---- Wait for event Queue Enable --------------------------*/
waitForEnable:
        if (queueEnabled == 0)
        {
            queueEnableRegVal |= BIT_1; /* set RING_IDLE */
            smemRegSet(devObjPtr, regAddr, queueEnableRegVal);
            SCIB_SEM_SIGNAL;
            SIM_OS_MAC(simOsEventWait)(pktGenData->eventId, 0/*forever*/);
        }
        else
        {
            SCIB_SEM_SIGNAL;
        }
        /* }}------------------------------------------------------------*/

        SCIB_SEM_TAKE;
        /* Re-Get and check 'queueEnable' after wait-for-event */
        regAddr = SIP_7_REG(devObjPtr, CNM.GDMA[gdmaUnit].
                      queueStatusAndControlRegs[txQueue].queueEnable);
        smemRegGet(devObjPtr, regAddr, &queueEnableRegVal);
        queueEnabled = SMEM_U32_GET_FIELD(queueEnableRegVal, 0, 1);
        if (queueEnabled == GT_FALSE)
            goto waitForEnable;

        smemRegFldSet(devObjPtr, regAddr, 1,1, RING_BUSY);
        SCIB_SEM_SIGNAL;

        /* gdmaTimestampTrace(devObjPtr, "PktGen-task", txQueue, sgdIdx); */

        chainEndedWithStopReq = GT_FALSE;

        /* "Rare changes" to be taken on disable->enable transition only */
        if (queueEnabledPrev != queueEnabled)
        {
            ;
        }

        /* Get whole 'queueControl' register info */
        regAddr = SIP_7_REG(devObjPtr, CNM.GDMA[gdmaUnit].
                      queueStatusAndControlRegs[txQueue].queueControl);
        smemRegGet(devObjPtr, regAddr, &queueControlRegVal);

        if (SMEM_U32_GET_FIELD(queueControlRegVal, 20, 4) == 0)
        {
            __LOG(("PktGen-task: ERROR: q_wrr_weight == 0 -> ring disabled\n"));
            chainEndedWithStopReq = GT_TRUE;
            continue;
        }

        /* Check FIFO Flush ~ reset sgdIdx=0 */
        if (queueEnableRegVal & BIT_5)
        {
            sgdIdx = 0;
            regAddr = SIP_7_REG(devObjPtr, CNM.GDMA[gdmaUnit].
                          queueStatusAndControlRegs[txQueue].queueEnable);
            smemRegFldSet(devObjPtr, regAddr, 5,1, 0); /* clear Flush */
        }

        /* Publish SGD index */
        pktGenData->shared.sgdIdx = sgdIdx;
        /* Update the next descriptor to handle */
        regAddr = SIP_7_REG(devObjPtr, CNM.GDMA[gdmaUnit].
                      queueStatusAndControlRegs[txQueue].wruSGDPointer);
        smemRegFldSet(devObjPtr, regAddr, 0,16, sgdIdx);

        /* Get Chain size on chain starting */
        if (sgdIdx == 0)
        {
            regAddr = SIP_7_REG(devObjPtr, CNM.GDMA[gdmaUnit].
                                  sgdAxiControlRegs[txQueue].sgdChainSize);
            smemRegFldGet(devObjPtr, regAddr, 0,16, &sgdChainSize);
            if (sgdChainSize == 0)
            {
                __LOG(("PktGen-task: ERROR: queue[%u] start with ZERO chain size\n", txQueue));
                chainEndedWithStopReq = GT_TRUE;
                continue;
            }
        }

        /* {{-- Send message with SGD chain's descriptor in BufferId ----*/
        isValid = gdmaTxSgdValidGet(devObjPtr, gdmaUnit, txQueue, sgdIdx, txDesc,
                                    NULL, NULL); /* ptr=NULL: no OUT required */
        if (isValid)
        {
            pktGenBufferSetAndEnqueue(devObjPtr, pktGenData);
        }
        /* }}------------------------------------------------------------*/

        /* Increment SGD-index and check wrap vs Ring Chain Size */
        if (++sgdIdx >= sgdChainSize)
        {
            sgdIdx = 0;
            /* Check 'sgd_recycle_chain' configuration. If(0) -> the Queue goes to */
            /*  Disabled by the HW -> stop loop and wait for queue disable/enable */
           chainEndedWithStopReq = ((queueControlRegVal & BIT_4) == 0);
        }

        /* Single SGD handling is finished => check-&-apply inter-packet timer */
        /* To yield/reschedule SIMULATOR's tasks - always go over sleep (with min 1 msec) */
        if ((queueControlRegVal & BIT_7) == 0)
        {
            pktIntervalMSec = PKT_GEN_MIN_INTERVAL_MSEC; /* timer disabled. Use min */
        }
        else
        {
            /* Get/Calculate Packet Interval mSec */
            regAddr = SIP_7_REG(devObjPtr, CNM.GDMA[gdmaUnit].gdmaTimerRegs[txQueue].timerValue);
            smemRegGet(devObjPtr, regAddr, &regVal);

            /* The GDMA timer clock is 12.5 MHz -> 1clock is 80 nSec */
            /* Convert nanoseconds in granularity of 80 ns to mSec   */
            if (regVal > 0xffffffff/80)
            {
                pktIntervalMSec = ((regVal / 100) * 80) / 10000;
            }
            else
            {
                pktIntervalMSec = (regVal * 80) / 1000000;
            }
            /* Set min sleep 1 ms */
            if (pktIntervalMSec < PKT_GEN_MIN_INTERVAL_MSEC)
                pktIntervalMSec = PKT_GEN_MIN_INTERVAL_MSEC;
            /* This task cycle takes ~700 ms => adjust by -1 */
            if (pktIntervalMSec > PKT_GEN_MIN_INTERVAL_MSEC)
                pktIntervalMSec -= 1;
        }
        devObjPtr = skernelSleep(devObjPtr, pktIntervalMSec);

    } /* while(1) */
}

/**
* @internal snetAasGdmaFromCpuTriggerPktGen function
* @endinternal
*
* @brief   GDMA : sip7 : wakeup per-queue Packet Generator task upon queue-enable
*          (creates GDMA_PKT_GEN_TASK_DATA_STC object and task if not exist)
*/
GT_VOID snetAasGdmaFromCpuTriggerPktGen
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                  gdma_id,
    IN GT_U32                  ringId
)
{
    GDMA_PKT_GEN_TASK_DATA_STC  *pktGenData, *ptr;
    GT_U32                      ii;

    /* {{ First entry: allocate storage for all PktGen objects */
    if (devObjPtr->trafficGeneratorSupport.trafficGenData[0].dataPatternPtr == NULL)
    {
        pktGenData = calloc(SIM_MAX_GDMA_QUEUE_NUM_CNS, sizeof(GDMA_PKT_GEN_TASK_DATA_STC));
        if (pktGenData == NULL)
        {
            skernelFatalError("GdmaFromCpuPktGen(%u,%d): cannot allocate GDMA_PKT_GEN_TASK_DATA_STC",
                              gdma_id, ringId);
        }
        devObjPtr->trafficGeneratorSupport.trafficGenData[0].dataPatternPtr = (void *)pktGenData;
    }
    else
    {
        pktGenData = (void *)devObjPtr->trafficGeneratorSupport.trafficGenData[0].dataPatternPtr;
    }
    /* }} First entry */

    /* {{ Look for Task(unit:ring). Create if not exist */
    ptr = pktGenData;
    for (ii = 0; ii < SIM_MAX_GDMA_QUEUE_NUM_CNS; ii++, ptr++)
    {
        if (ptr->shared.ringId == ringId && ptr->shared.gdma_id == gdma_id)
            break;
    }
    if (ii < SIM_MAX_GDMA_QUEUE_NUM_CNS)
    {
        /* Task found. Go to wake it up */
        pktGenData = ptr;
    }
    else
    {
        /* Look for first 'free' entry space to create an object */
        ptr = pktGenData;
        for (ii = 0; ii < SIM_MAX_GDMA_QUEUE_NUM_CNS; ii++, ptr++)
        {
            if (ptr->taskHandle == NULL)
                break;
        }
        if (ii == SIM_MAX_GDMA_QUEUE_NUM_CNS)
        {
            skernelFatalError("GdmaFromCpuPktGen(%u,%d): "
                              "no free entry for %d PktGenerators",
                              gdma_id, ringId, SIM_MAX_GDMA_QUEUE_NUM_CNS+1);
        }

        pktGenData = ptr;

        /* Fill Traf-Gen Data */
        pktGenData->genFunc = snetAasGdmaFromCpuProcess; /* never changed */
        pktGenData->shared.gdma_id = gdma_id;
        pktGenData->shared.ringId = ringId;
        pktGenData->shared.sgdIdx = 0;
        pktGenData->shared.pktGenMagic = GDMA_PKT_GEN_MAGIC;
        pktGenData->devObjPtr = devObjPtr;
        pktGenData->eventId = SIM_OS_MAC(simOsEventCreate)();

        /* Create SKernel task */
        pktGenData->taskHandle = SIM_OS_MAC(simOsTaskCreate)(
                        GT_TASK_PRIORITY_NORMAL,
                        (unsigned (__TASKCONV *)(void*))snetAasGdmaPktGenTask,
                        (void *)pktGenData);

        if (pktGenData->taskHandle == NULL)
        {
            skernelFatalError("GdmaFromCpuPktGen(%u,%d): cannot create task",
                              gdma_id, ringId);
        }
        /* Task creat succeed / finished */
        devObjPtr->numThreadsOnMe++; /* already under SCIB_SEM_TAKE */
    }
    /* }} Look for Task(unit:ring) */

    if (pktGenData->devObjPtr != devObjPtr)
    {
        skernelFatalError("GdmaFromCpuPktGen(%u,%d): "
                          "'soft reset' currently not supported",
                          gdma_id, ringId);
    }

    /* Signal the PktGen task to wakeup (if it sleeps) */
    SIM_OS_MAC(simOsEventSet)(pktGenData->eventId);
}
