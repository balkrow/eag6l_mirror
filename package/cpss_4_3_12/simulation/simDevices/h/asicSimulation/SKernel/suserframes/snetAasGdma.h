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
* @file snetAasGdma.h
*
* @brief API declaration and data type definition for GDMA
*
* @version   1
********************************************************************************
*/
#ifndef __snetAasGdmah
#define __snetAasGdmah

#include <asicSimulation/SKernel/smain/smain.h>

/* types of rings (purpose of ring) */
enum{
    MEM2MEM_RING_TYPE             =0,
    TO_CPU_PKT_MODE_RING_TYPE     =1,
    TO_CPU_MSG_MODE_RING_TYPE     =2,
    PDI2PDI_MSG2PKT_RING_TYPE     =3,
    PDI2PDI_LPBK_RING_TYPE        =4,
    FROM_CPU_RING_TYPE            =5,
    FROM_CPU_PKT_GEN_RING_TYPE    =6,
};

/* SIP7 : GDMA unit : the fields of the SW RX descriptor */
typedef enum {                                                      /*start bit , num of bits*/
     SMEM_SIP7_GDMA_SW_GDMA_DESCRIPTOR_RX_FIELD_OWN                        /*0   ,   1  */
    ,SMEM_SIP7_GDMA_SW_GDMA_DESCRIPTOR_RX_FIELD_FAILURE_CODE               /*4   ,   2  */
    ,SMEM_SIP7_GDMA_SW_GDMA_DESCRIPTOR_RX_FIELD_F_BIT                      /*6   ,   1  */
    ,SMEM_SIP7_GDMA_SW_GDMA_DESCRIPTOR_RX_FIELD_L_BIT                      /*7   ,   1  */
    ,SMEM_SIP7_GDMA_SW_GDMA_DESCRIPTOR_RX_FIELD_AXI_ATTR_PROFILE           /*8   ,   5  */
    ,SMEM_SIP7_GDMA_SW_GDMA_DESCRIPTOR_RX_FIELD_OPCODE                     /*13  ,   1  */
    ,SMEM_SIP7_GDMA_SW_GDMA_DESCRIPTOR_RX_FIELD_BUFFER_BYTE_COUNT          /*14  ,   16 */
    ,SMEM_SIP7_GDMA_SW_GDMA_DESCRIPTOR_RX_FIELD_BUFFER_DST_ADDR_LOW_0      /*30  ,   32 */
    ,SMEM_SIP7_GDMA_SW_GDMA_DESCRIPTOR_RX_FIELD_BUFFER_DST_ADDR_LOW_1      /*62  ,    2 */
    ,SMEM_SIP7_GDMA_SW_GDMA_DESCRIPTOR_RX_FIELD_BUFFER_DST_ADDR_HIGH       /*96  ,   14 */

    ,SMEM_SIP7_GDMA_SW_GDMA_DESCRIPTOR_RX_FIELD___LAST_VALUE___E/* used for array size */
}SMEM_SIP7_GDMA_SW_GDMA_DESCRIPTOR_RX_FIELDS_ENT;

/* macro to access a field in SIP7 GDMA SW GDMA DESCRIPTOR RX - for GET field */
#define SMEM_SIP7_GDMA_SW_GDMA_DESCRIPTOR_RX_ENTRY_FIELD_GET(_devObjPtr,_memPtr,_index,fieldName)       \
    SNET_TABLE_ENTRY_FIELD_GET(_devObjPtr,(GT_U32*)(_memPtr),_index,fieldName,SKERNEL_TABLE_FORMAT_GDMA_SW_GDMA_DESCRIPTOR_RX_E)

/* macro to access a field in SIP7 GDMA SW GDMA DESCRIPTOR RX - for SET field */
#define SMEM_SIP7_GDMA_SW_GDMA_DESCRIPTOR_RX_ENTRY_FIELD_SET(_devObjPtr,_memPtr,_index,fieldName,_value)       \
    SNET_TABLE_ENTRY_FIELD_SET(_devObjPtr,(GT_U32*)(_memPtr),_index,fieldName,_value,SKERNEL_TABLE_FORMAT_GDMA_SW_GDMA_DESCRIPTOR_RX_E)

/* short name for SMEM_SIP7_GDMA_SW_GDMA_DESCRIPTOR_RX_ENTRY_FIELD_GET */
#define  GDMA_RX(a,b,c,d)   \
    SMEM_SIP7_GDMA_SW_GDMA_DESCRIPTOR_RX_ENTRY_FIELD_GET(a,b,c,d)

/* short name for SMEM_SIP7_GDMA_SW_GDMA_DESCRIPTOR_RX_ENTRY_FIELD_SET */
#define  GDMA_RX_SET(a,b,c,d,e)   \
    SMEM_SIP7_GDMA_SW_GDMA_DESCRIPTOR_RX_ENTRY_FIELD_SET(a,b,c,d,e)


/* SIP7 : GDMA unit : the fields of the SW TX descriptor */
typedef enum {                                                      /*start bit , num of bits*/
     SMEM_SIP7_GDMA_SW_GDMA_DESCRIPTOR_TX_FIELD_OWN                        /*0   ,   1  */
    ,SMEM_SIP7_GDMA_SW_GDMA_DESCRIPTOR_TX_FIELD_FAILURE_CODE               /*4   ,   1  */
    ,SMEM_SIP7_GDMA_SW_GDMA_DESCRIPTOR_TX_VALID                            /*5   ,   1  */
    ,SMEM_SIP7_GDMA_SW_GDMA_DESCRIPTOR_TX_FIELD_F_BIT                      /*6   ,   1  */
    ,SMEM_SIP7_GDMA_SW_GDMA_DESCRIPTOR_TX_FIELD_L_BIT                      /*7   ,   1  */
    ,SMEM_SIP7_GDMA_SW_GDMA_DESCRIPTOR_TX_FIELD_AXI_ATTR_PROFILE           /*8   ,   5  */
    ,SMEM_SIP7_GDMA_SW_GDMA_DESCRIPTOR_TX_FIELD_OPCODE                     /*13  ,   1  */
    ,SMEM_SIP7_GDMA_SW_GDMA_DESCRIPTOR_TX_FIELD_BUFFER_BYTE_COUNT          /*14  ,   16 */
    ,SMEM_SIP7_GDMA_SW_GDMA_DESCRIPTOR_TX_FIELD_BUFFER_DST_ADDR_LOW_0      /*30  ,   32 */
    ,SMEM_SIP7_GDMA_SW_GDMA_DESCRIPTOR_TX_FIELD_BUFFER_DST_ADDR_LOW_1      /*62  ,    2 */
    ,SMEM_SIP7_GDMA_SW_GDMA_DESCRIPTOR_TX_FIELD_BUFFER_DST_ADDR_HIGH       /*96  ,   14 */

    ,SMEM_SIP7_GDMA_SW_GDMA_DESCRIPTOR_TX_FIELD___LAST_VALUE___E/* used for array size */
}SMEM_SIP7_GDMA_SW_GDMA_DESCRIPTOR_TX_FIELDS_ENT;

/* macro to access a field in SIP7 GDMA SW GDMA DESCRIPTOR TX - for GET field */
#define SMEM_SIP7_GDMA_SW_GDMA_DESCRIPTOR_TX_ENTRY_FIELD_GET(_devObjPtr,_memPtr,_index,fieldName)       \
    SNET_TABLE_ENTRY_FIELD_GET(_devObjPtr,(GT_U32*)(_memPtr),_index,fieldName,SKERNEL_TABLE_FORMAT_GDMA_SW_GDMA_DESCRIPTOR_TX_E)

/* macro to access a field in SIP7 GDMA SW GDMA DESCRIPTOR TX - for SET field */
#define SMEM_SIP7_GDMA_SW_GDMA_DESCRIPTOR_TX_ENTRY_FIELD_SET(_devObjPtr,_memPtr,_index,fieldName,_value)       \
    SNET_TABLE_ENTRY_FIELD_SET(_devObjPtr,(GT_U32*)(_memPtr),_index,fieldName,_value,SKERNEL_TABLE_FORMAT_GDMA_SW_GDMA_DESCRIPTOR_TX_E)

/* short name for SMEM_SIP7_GDMA_SW_GDMA_DESCRIPTOR_TX_ENTRY_FIELD_GET */
#define  GDMA_TX(a,b,c,d)   \
    SMEM_SIP7_GDMA_SW_GDMA_DESCRIPTOR_TX_ENTRY_FIELD_GET(a,b,c,d)

/* short name for SMEM_SIP7_GDMA_SW_GDMA_DESCRIPTOR_TX_ENTRY_FIELD_SET */
#define  GDMA_TX_SET(a,b,c,d,e)   \
    SMEM_SIP7_GDMA_SW_GDMA_DESCRIPTOR_TX_ENTRY_FIELD_SET(a,b,c,d,e)

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
);

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
);


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
);

/**
* @internal snetAasGdmaFromCpuTriggerPktGen function
* @endinternal
*
* @brief   GDMA : sip7 : wakeup per-queue Packet Generator task upon queue-enable
*          (creates GDMA_PKT_GEN_STC object and task if not exist)
*/
GT_VOID snetAasGdmaFromCpuTriggerPktGen
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                  gdma_id,
    IN GT_U32                  ringId
);

/* SIP7 : GDMA clients for messages AUQ/FUQ/CNC/IPE */
typedef enum{
    SNET_SIP7_GDMA_MSG_CLIENT_FDB_AUQ_E,
    SNET_SIP7_GDMA_MSG_CLIENT_FDB_FUQ_E,
    SNET_SIP7_GDMA_MSG_CLIENT_CNC_E,
    SNET_SIP7_GDMA_MSG_CLIENT_IPE_E,

    SNET_SIP7_GDMA_MSG_CLIENT__LAST__E

}SNET_SIP7_GDMA_MSG_CLIENT_ENT;


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
);

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */



#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  /* __snetAasGdmah */



