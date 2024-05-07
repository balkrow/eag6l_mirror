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
* @file common/h/cpss/common/cpssHwInit/cpssHwInit.h
*
* @brief Includes CPSS basic Hw initialization functions, and data structures.
*
* @version   1
********************************************************************************
*/
#ifndef __cpssHwInit_h
#define __cpssHwInit_h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/common/cpssTypes.h>

/* Define the maximum number of RX queues */
#define CPSS_MAX_RX_QUEUE_CNS     8

/* Define the maximum number of TX queues */
#define CPSS_MAX_TX_QUEUE_CNS     8

/* constant used for field of :
    CPSS_DXCH_PP_PHASE1_INIT_INFO_STC:: hwInfo[ggg].irq[iii] --> Nofity the cpss NOT capture
    the interrupt line (for this device) with it's ISR
*/
#define CPSS_PP_INTERRUPT_VECTOR_NOT_USED_CNS                 0xFFFFFFFF
/* constant used for field of :
    CPSS_DXCH_PP_PHASE1_INIT_INFO_STC:: hwInfo[ggg].irq[iii] --> Nofity the cpss
    will emulate interrupt by polling.
*/
#define CPSS_PP_INTERRUPT_VECTOR_USE_POLLING_CNS              0xFFFFFFFE

/**
 * @enum CPSS_SYS_HA_MODE_ENT
 *
 * @brief define the High Availability mode of the CPU
 *
*/
typedef enum
{
    /** unit HA mode is active   */
    CPSS_SYS_HA_MODE_ACTIVE_E,
    /** unit HA mode is standby   */
    CPSS_SYS_HA_MODE_STANDBY_E
}CPSS_SYS_HA_MODE_ENT;

/**
* @enum CPSS_PP_INTERFACE_CHANNEL_ENT
 *
 * @brief Packet Processor interface channel to the CPU
 * Used to select driver which performs low-level access to
 * Switching registers
*/
typedef enum
{
    /** @brief channel to the CPU with PCI
     *  Selects pre-Sip5 PCI/PEX driver with 4-region address completion,
     *  64M should be mapped to CPSS address space
    */
    CPSS_CHANNEL_PCI_E    = 0x0,

    /** @brief channel to the CPU with SMI */
    CPSS_CHANNEL_SMI_E,

    /** @brief channel to the CPU with TWSI */
    CPSS_CHANNEL_TWSI_E,

    /** @brief channel to the CPU with PCI express (PEX)
     *  Selects pre-Sip5 PCI/PEX driver with 4-region address completion,
     *  64M should be mapped to CPSS address space
    */
    CPSS_CHANNEL_PEX_E,

    /** @brief channel to the CPU with PCI express (PEX or MBUS)
     *  with 8 Address Region Completion Mechanism.
     *  (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3, Aldrin2)
     *  Selects Sip5+ driver with 8-region address completion,
     *  4M should be mapped to CPSS address space
    */
    CPSS_CHANNEL_PEX_MBUS_E,

    /** @brief channel to the CPU with PCI express or internal embedded CPU bus
     *  (PEX or MBUS) for SIP6 generation devices.
     *  (APPLICABLE DEVICES: Falcon, AC5P, AC5X, Harrier, Ironman)
     *  Selects Sip5+ driver with 8-region address completion,
     *  4M should be mapped to CPSS address space
    */
    CPSS_CHANNEL_PEX_EAGLE_E,

    /** @brief channel to the CPU with SMI.
     *  Used for CPSS internal debug only
    */
    CPSS_CHANNEL_PEX_FALCON_Z_E,

    /** @brief kernel PEX driver (linux only, legacy prestera driver)
     *  The low-level driver is implemented in kernel,
     *  no need Switching registers to be mapped to user-space.
     *  For each Lion2 saves (64M+1M)*8 == 520M of address space
     *  appAddrCompletionRegionsBmp must be set to one of
     *  0x0f to use 4 regions addres completion
     *       (64M virtual addresses)
     *  0x03 to use 2 regions addres completion
     *       (32M virtual addresses)
    */
    CPSS_CHANNEL_PEX_KERNEL_E,
#ifdef DEVELOPER_NEW_DRIVERS
    CPSS_CHANNEL_PEX_NEWDRV_E,
    CPSS_CHANNEL_PEX_MBUS_NEWDRV_E,
    CPSS_CHANNEL_PEX_MBUS_KERNEL_E,
#endif

    /** for internal use , must be last */
    CPSS_CHANNEL_LAST_E
}CPSS_PP_INTERFACE_CHANNEL_ENT;

#ifndef DEVELOPER_NEW_DRIVERS
/* EYALO question - to which defines add CPSS_CHANNEL_PEX_EAGLE_E */
#define CPSS_CHANNEL_IS_PCI_COMPATIBLE_MAC(_channel) \
    ((_channel) == CPSS_CHANNEL_PCI_E || (_channel) == CPSS_CHANNEL_PEX_E \
        || (_channel) == CPSS_CHANNEL_PEX_MBUS_E \
        || (_channel) == CPSS_CHANNEL_PEX_KERNEL_E \
        || (_channel) == CPSS_CHANNEL_PEX_EAGLE_E \
        || (_channel) == CPSS_CHANNEL_PEX_FALCON_Z_E \
    )

#define CPSS_CHANNEL_IS_PCI_PEX_MAC(_channel) \
    ((_channel) == CPSS_CHANNEL_PCI_E || (_channel) == CPSS_CHANNEL_PEX_E \
        || (_channel) == CPSS_CHANNEL_PEX_KERNEL_E \
        || (_channel) == CPSS_CHANNEL_PEX_EAGLE_E \
        || (_channel) == CPSS_CHANNEL_PEX_FALCON_Z_E \
    )

#define CPSS_CHANNEL_IS_PEX_MAC(_channel) \
    ((_channel) == CPSS_CHANNEL_PEX_E \
        || (_channel) == CPSS_CHANNEL_PEX_KERNEL_E \
        || (_channel) == CPSS_CHANNEL_PEX_EAGLE_E \
        || (_channel) == CPSS_CHANNEL_PEX_FALCON_Z_E \
    )

#define CPSS_CHANNEL_IS_PEX_MBUS_MAC(_channel) \
    ((_channel) == CPSS_CHANNEL_PEX_MBUS_E )
#else /* DEVELOPER_NEW_DRIVERS */
/* to be enabled later:
 *    CPSS_CHANNEL_PEX_NEWDRV_E         - new PEX driver
 *    CPSS_CHANNEL_PEX_MBUS_NEWDRV_E    - new PEX_MBUS driver
 *    CPSS_CHANNEL_PEX_MBUS_KERNEL_E    - kernel PEX MBUS driver
 */
#define CPSS_CHANNEL_IS_PCI_COMPATIBLE_MAC(_channel) \
    ((_channel) == CPSS_CHANNEL_PCI_E || (_channel) == CPSS_CHANNEL_PEX_E \
        || (_channel) == CPSS_CHANNEL_PEX_MBUS_E \
        || (_channel) == CPSS_CHANNEL_PEX_NEWDRV_E \
        || (_channel) == CPSS_CHANNEL_PEX_MBUS_NEWDRV_E \
        || (_channel) == CPSS_CHANNEL_PEX_KERNEL_E \
        || (_channel) == CPSS_CHANNEL_PEX_MBUS_KERNEL_E
    )

#define CPSS_CHANNEL_IS_PCI_PEX_MAC(_channel) \
    ((_channel) == CPSS_CHANNEL_PCI_E || (_channel) == CPSS_CHANNEL_PEX_E \
        || (_channel) == CPSS_CHANNEL_PEX_NEWDRV_E \
        || (_channel) == CPSS_CHANNEL_PEX_KERNEL_E \
    )

#define CPSS_CHANNEL_IS_PEX_MAC(_channel) \
    ((_channel) == CPSS_CHANNEL_PEX_E \
        || (_channel) == CPSS_CHANNEL_PEX_NEWDRV_E \
        || (_channel) == CPSS_CHANNEL_PEX_KERNEL_E \
    )

#define CPSS_CHANNEL_IS_PEX_MBUS_MAC(_channel) \
    ((_channel) == CPSS_CHANNEL_PEX_MBUS_E \
        || (_channel) == CPSS_CHANNEL_PEX_MBUS_NEWDRV_E \
        || (_channel) == CPSS_CHANNEL_PEX_MBUS_KERNEL_E \
    )
#endif /* DEVELOPER_NEW_DRIVERS */



/**
* @enum CPSS_RX_BUFF_ALLOC_METHOD_ENT
 *
 * @brief Defines the different allocation methods for the Rx / Tx
 * descriptors, and Rx buffers.
*/
typedef enum{

    /** @brief The user provides a function pointer which
     *  dynamically allocates a single Rx buffer.
    */
    CPSS_RX_BUFF_DYNAMIC_ALLOC_E = 0,

    /** @brief The user provides a block of memory in which
     *  the driver then partitions into Rx buffers.
    */
    CPSS_RX_BUFF_STATIC_ALLOC_E,

    /** @brief CPSS doesn't manage the RX buffers allocation;
     *  application must provide buffers for RX queues.
     *  Supported in SDMA CPU port mode and for DXCH devices only.
     */
    CPSS_RX_BUFF_NO_ALLOC_E

} CPSS_RX_BUFF_ALLOC_METHOD_ENT;

/**
* @enum CPSS_TX_BUFF_ALLOC_METHOD_ENT
 *
 * @brief Defines the different allocation methods for the Tx
 * descriptors and Tx buffers.
*/
typedef enum{

    /** @brief The user provides a function pointer which
     *  dynamically allocates descriptors & buffers memory.
    */
    CPSS_TX_BUFF_DYNAMIC_ALLOC_E = 0,

    /** @brief The user provides a block of memory in which
     *  the driver then partitions into Tx descriptors & buffers.
    */
    CPSS_TX_BUFF_STATIC_ALLOC_E

} CPSS_TX_BUFF_ALLOC_METHOD_ENT;


/**
* @enum CPSS_PHY_REF_CLK_PU_OE_CFG_ENT
 *
 * @brief Defines the different configurations methods for the
 * phy ref clk tx Pu and Oe in Device General Control 1 47
 * (bits 9 and 10 on Ironman)
 *
 *  @note (APPLICABLE DEVICES: Ironman)
 *
*/
typedef enum{

    /** @brief Phy ref clk tx Pu set to disabled, Oe is set to disabled
    */
    CPSS_PHY_REF_CLK_PU_DISABLE_OE_DISABLE_E = 0x0,
    /** @brief Phy ref clk tx Pu set to disabled, Oe is set to enabled
    */
    CPSS_PHY_REF_CLK_PU_DISABLE_OE_ENABLE_E  = 0x01,
    /** @brief Phy ref clk tx Pu set to enabled, Oe is set to disabled
    */
    CPSS_PHY_REF_CLK_PU_ENABLE_OE_DISABLE_E  = 0x10,
    /** @brief Phy ref clk tx Pu set to enabled, Oe is set to enabled
    */
    CPSS_PHY_REF_CLK_PU_ENABLE_OE_ENABLE_E   = 0x11

} CPSS_PHY_REF_CLK_PU_OE_CFG_ENT;


/**
* @enum CPSS_TX_SDMA_QUEUE_MODE_ENT
 *
 * @brief Defines the different working mode of Tx SDMA queue.
*/
typedef enum{

    /** @brief SDMA updates the descriptor ownership
     *  bit to mark CPU ownership.
     */
    CPSS_TX_SDMA_QUEUE_MODE_NORMAL_E,

    /** @brief SDMA will not return the
     *  descriptor ownership to the CPU.
     */
    CPSS_TX_SDMA_QUEUE_MODE_PACKET_GENERATOR_E,

    /** @brief TX SDMA performed from SRAM & SDMA updates the descriptor
     *  ownership bit to mark CPU ownership.
     *
     *  @note (APPLICABLE DEVICES: Falcon; AC5X; Ironman)
     */
    CPSS_TX_SDMA_QUEUE_MODE_SRAM_NORMAL_E,

} CPSS_TX_SDMA_QUEUE_MODE_ENT;


/*
 * Typedef: CPSS_TX_BUFF_MALLOC_FUNC
 *
 * Description: Prototype of the function pointer to be provided by the
 *              user to perform Rx descriptors & buffers allocation.
 *              The CPSS invokes this user function when allocating
 *              Rx buffers. This is relevant only if  the DYNAMIC_ALLOC
 *              allocation method is used.
 *
 * Fields:
 *      size   - the size  to allocate.
 *      align - The required byte-alignment for the allocation.
 *
 */
typedef GT_U8* (*CPSS_TX_BUFF_MALLOC_FUNC)
(
    IN GT_U32 size,
    IN GT_U32 align
);

/*
 * Typedef: CPSS_RX_BUFF_MALLOC_FUNC
 *
 * Description: Prototype of the function pointer to be provided by the
 *              user to perform Rx buffer allocation.
 *              The CPSS invokes this user function when allocating
 *              Rx buffers. This is relevant only if  the DYNAMIC_ALLOC
 *              allocation method is used.
 *
 * Fields:
 *      size   - the size of the Rx buffer to allocate.  This value reflects
 *               the Rx buffer size that the user defines in the
 *               TAPI gtSysConfig.h structure GT_SYS_CONFIG
 *      align - The required byte-alignment for the Rx buffer
 *
 */
typedef GT_U8* (*CPSS_RX_BUFF_MALLOC_FUNC)
(
    IN GT_U32 size,
    IN GT_U32 align
);


/**
* @struct CPSS_RX_BUF_INFO_STC
 *
 *  @brief Rx buffer Information
 *
 *  If DYNAMIC_ALLOC method is used, then the function provided by user
 *  should have the following prototype defined by CPSS_RX_BUFF_MALLOC_FUNC
*/
typedef struct
{
    /** @brief There are choices for providing Rx Buffers to the CPSS:
     *  1) The user passes a block of memory which the
     *     CPSS will internally partition into Rx Buffers.
     *  2) The user passes a function pointer to a
     *     user supplied Rx Buffer allocator
    */
    CPSS_RX_BUFF_ALLOC_METHOD_ENT   allocMethod;

    /** @brief A table (entry per queue) that describes the buffer
     *  dispersal among all Rx queues. (values 0..100)
     *  NOTE: The accumulation of all queues should not
     *        be more than 100% !
    */
    GT_U32                  bufferPercentage[CPSS_MAX_RX_QUEUE_CNS];

    /** @brief The size of the SDMA Rx data buffer. If the Data
     *  buffer size is smaller than received packet size, the
     *  packet is "chained" over multiple buffers. This value is
     *  used by the CPSS when initializing the RX descriptor rings.
    */
    GT_U32                  rxBufSize;

    /** @brief The number of bytes before the start of the Rx buffer
     *  to reserve for the application use. This value is used by the
     *  CPSS when initializing the RX descriptor rings.
     *  NOTE: This parameter must be synchronized among all system CPUs
    */
    GT_U32                  headerOffset;

    /** @brief GT_TRUE if buffers allocated in cached CPU memory,
     *         GT_FALSE otherwise.
     *  When using cached RX buffers on Linux/FreeBSD please take
     *  in mind high impact on system performance due to IOCTL from
     *  user to kernel space during the invalidate action.
    */
    GT_BOOL                 buffersInCachedMem;

    union
    {
        /** (relevant when CPSS_RX_BUFF_STATIC_ALLOC_E) */
        struct
        {
            /** @brief (Pointer to) a block of memory to be used for
             *  allocating Rx packet data buffers
            */
            GT_U32  *rxBufBlockPtr;

            /** The raw size in byte of rxBufBlock.*/
            GT_U32  rxBufBlockSize;
        }staticAlloc;

        /** (relevant when CPSS_RX_BUFF_DYNAMIC_ALLOC_E) */
        struct
        {
            /** Function pointer to be invoked for allocating buffers.*/
            CPSS_RX_BUFF_MALLOC_FUNC    mallocFunc;

            /** @brief The number of Buffers that need to be allocated.
             *  This field is used by Eth port NetworkInterface.
            */
            GT_U32                      numOfRxBuffers;
        }dynamicAlloc;
    }buffData;
}CPSS_RX_BUF_INFO_STC;


/**
* @struct CPSS_NET_IF_CFG_STC
 *
 * @brief Network interface configuration parameters
*/
typedef struct{

    /** @brief Pointer to a block of host memory to be used
     *  for allocating Tx packet descriptor structures.
    */
    GT_U32 *txDescBlock;

    /** @brief The raw size in bytes of txDescBlock memory.
     *  rxDescBlock   - Pointer to a block memory to be used for
     *  allocating Rx description structures.(Rx descriptor
     *  structures to cpu)
     */
    GT_U32 txDescBlockSize;

    /** @brief Pointer to a block memory to be used for
     *  allocating Rx description structures.(Rx descriptor
     *  structures to cpu)
    */
    GT_U32 *rxDescBlock;

    /** The raw size in byte of rxDescBlock. */
    GT_U32 rxDescBlockSize;

    /** Rx buffers allocation information. */
    CPSS_RX_BUF_INFO_STC rxBufInfo;

} CPSS_NET_IF_CFG_STC;

/**
* @struct CPSS_DMA_QUEUE_CFG_STC
 *
 * @brief DMA Queue configuration parameters
*/
typedef struct{

    /** @brief The block of memory used for the DMA Queue.
    */
    GT_U8 *dmaDescBlock;

    /** Size of dmaDescBlock in bytes. */
    GT_U32 dmaDescBlockSize;

} CPSS_DMA_QUEUE_CFG_STC;


/**
* @struct CPSS_MULTI_NET_IF_RX_SDMA_QUEUE_STC
 *
 * @brief Network Multi-Group interface configuration parameters for
 * Rx SDMA.
*/
typedef struct
{
    /** Memory allocation method for buffers: static or dynamic */
    CPSS_RX_BUFF_ALLOC_METHOD_ENT   buffAllocMethod;

    /** Number of descriptors to allocate. */
    GT_U32                          numOfRxDesc;

    /** Number of buffers to allocate.*/
    GT_U32                          numOfRxBuff;

    /** @brief The size of the SDMA Rx data buffer. If the Data
     * buffer size is smaller than received packet size,
     * the packet is "chained" over multiple buffers.
     * This value is used by the CPSS when initializing the
     * RX descriptor rings.
    */
    GT_U32                          buffSize;

    /** @brief The number of bytes before the start of the Rx buffer
     *  to reserve for the application use. This value is used by the
     *  CPSS when initializing the RX descriptor rings.
     *  NOTE: This parameter must be synchronized among all system CPUs.
    */
    GT_U32                          buffHeaderOffset;

    /** @brief GT_TRUE if buffers allocated in cached CPU memory,
     *                          GT_FALSE otherwise.
    */
    GT_BOOL                         buffersInCachedMem;

    /** (pointer to) memory block allocation.*/
    GT_U32                          *descMemPtr;

    /** The raw size in bytes of descMemPtr memory.*/
    GT_U32                          descMemSize;

    union
    {
        struct
            {
            /** (pointer to) memory block static allocation.*/
            GT_U32  *buffMemPtr;

            /** size of allocated block. */
            GT_U32   buffMemSize;
        } staticAlloc;

        struct
        {
            /** function pointer for memory allocation.*/
            CPSS_RX_BUFF_MALLOC_FUNC buffMallocFunc;
        } dynamicAlloc;
   } memData;
} CPSS_MULTI_NET_IF_RX_SDMA_QUEUE_STC;


/**
* @struct CPSS_MULTI_NET_IF_TX_SDMA_QUEUE_STC
 *
 * @brief Network Multi-Group interface configuration parameters for
 * Tx SDMA.
*/
typedef struct
{
    /** Queue working mode.*/
    CPSS_TX_SDMA_QUEUE_MODE_ENT     queueMode;

    /** @brief Memory allocation method for buffers
     *  and descriptors: static or dynamic
    */
    CPSS_TX_BUFF_ALLOC_METHOD_ENT   buffAndDescAllocMethod;

    /** Number of descriptors to allocate.*/
    GT_U32                          numOfTxDesc;

    /** @brief Number of buffers to allocate.
     *  Relevant only to queue working as generator.
    */
    GT_U32                          numOfTxBuff;

    /** @brief Allocated buffer size (internal packet additions,
     *  e.g. DSA tag, should be taken into consideration when
     *  defining this length).
     *  Relevant only to queue working as generator.
    */
    GT_U32                          buffSize;

    /** @brief Memory allocation definitions. Descriptors related.
     *  For queue working as generator also relevant for buffers.
    */
    union
    {
        struct
            {
            /** (pointer to) memory block static allocation.*/
            GT_U32  *buffAndDescMemPtr;

            /** size of allocated block.*/
            GT_U32   buffAndDescMemSize;
        } staticAlloc;

        struct
        {
            /** function pointer for memory allocation.*/
            CPSS_TX_BUFF_MALLOC_FUNC buffAndDescMallocFunc;
        } dynamicAlloc;
     }memData;
} CPSS_MULTI_NET_IF_TX_SDMA_QUEUE_STC;


/**
* @struct CPSS_MULTI_NET_IF_CFG_STC
 *
 * @brief Multi-Group Network interface configuration parameters.
 *
 *  the indexes into rxSdmaQueuesConfig[x][y] and txSdmaQueuesConfig[x][y] are
 *      according to 'global queue index' :
 *      x = 'global queue index' / 8
 *      y = 'global queue index' % 8
 *
 *  NOTE: in Lion2 : x = PRV_CPSS_NETIF_SDMA_PORT_GROUP_ID_MAC(devNum)
*/
typedef struct{

    /** Rx queues related configurations.*/
    CPSS_MULTI_NET_IF_RX_SDMA_QUEUE_STC rxSdmaQueuesConfig[CPSS_MAX_SDMA_CPU_PORTS_CNS][CPSS_MAX_RX_QUEUE_CNS];

    /** Tx queues related configurations.*/
    CPSS_MULTI_NET_IF_TX_SDMA_QUEUE_STC txSdmaQueuesConfig[CPSS_MAX_SDMA_CPU_PORTS_CNS][CPSS_MAX_TX_QUEUE_CNS];

} CPSS_MULTI_NET_IF_CFG_STC;

/**
* @struct CPSS_AUQ_CFG_STC
 *
 * @brief Address Update Queue configuration parameters
*/
typedef struct{

    /** @brief The block of memory used for the Address Update Queue.
     *  The packet processor writes AU messages to this queue.
    */
    GT_U8 *auDescBlock;

    /** Size of auDescBlock (in Bytes). */
    GT_U32 auDescBlockSize;

} CPSS_AUQ_CFG_STC;

/**
* @enum CPSS_PP_PHASE2_UPLOAD_QUEUE_TYPE_ENT
 *
 * @brief Type of FDB/CNC upload queue.
*/
typedef enum{

    /** @brief UPLOAD QUEUE shared between FDB AU messages and FDB upload, CNC upload not supported.
     */
    CPSS_PP_PHASE2_UPLOAD_QUEUE_SHARED_WITH_AUQ_E = GT_FALSE, /*0*/

    /** @brief UPLOAD QUEUE separated from FDB AU messages QUEUE, used for FDB upload and CNC upload.
     *  For AC5P implemented as 2 queues (each - half of memory specified by application):
     *  1. FDB and CNC blocks 0-15 to MG0 unit.
     *  2. CNC blocks 16-31 to MG1 unit.
     */
    CPSS_PP_PHASE2_UPLOAD_QUEUE_SEPARATE_E = GT_TRUE, /*1*/

    /** @brief UPLOAD QUEUE separated from FDB AU messages QUEUE, used for FDB upload and CNC upload.
     *  APPLICABLE DEVICES: AC5P.
     *  1. All the memory specified by application given to
     *     Upload of FDB and CNC blocks 0-15 .
     *  2. Upload of CNC blocks 16-31 not supported.
     */
    CPSS_PP_PHASE2_UPLOAD_QUEUE_SEPARATE_AND_LIMITED_CNC_E  /*2*/


} CPSS_PP_PHASE2_UPLOAD_QUEUE_TYPE_ENT;


/**
* @enum CPSS_HW_PP_RESET_SKIP_TYPE_ENT
 *
 * @brief define the skip type parameter to set. setting this parameters are
 *      relevant when setting <SoftResetTrigger> to 1.
 */
typedef enum {
    /** @brief Skip the registers initialization at soft reset.
     *  Disable = Set: At software reset, registers are set to their
     *            default value.
     *  Enable  = Not Set: At software reset, registers are not set to
     *            their default value. Their value remains as it was
     *            before the software reset.
    */
    CPSS_HW_PP_RESET_SKIP_TYPE_REGISTER_E = 0,

    /** @brief Skip the tables initialization at soft reset.
     *  Disable = Set: At software reset, tables are set to their
     *            default value.
     *  Enable = Not Set: At software reset, tables are not set to
     *           their default value. Their value remains as it was
     *           before the software reset.
    */
    CPSS_HW_PP_RESET_SKIP_TYPE_TABLE_E,

    /** @brief Skip the EEPROM initialization at soft reset.
     *  Disable  = Performed: At software reset, EEPROM initialization
     *             is performed.
     *  Enable   = Not Performed: At software reset, EEPROM initialization
     *             is not performed
    */
    CPSS_HW_PP_RESET_SKIP_TYPE_EEPROM_E,

    /** @brief Skip the PEX initialization at soft reset.
     *  Disable = Do Not Skip: Reset the PEX on SW Reset.
     *  Enable  = Skip: Do not reset the PEX on SW Reset.
    */
    CPSS_HW_PP_RESET_SKIP_TYPE_PEX_E,

    /** @brief Soft reset done without link loss.
     *  Disable = Do Not Skip: Link Loss on SW Reset.
     *  Enable  = Skip: No Link Loss on SW Reset.
    */
    CPSS_HW_PP_RESET_SKIP_TYPE_LINK_LOSS_E,

    /** @brief Skip the Chiplets initialization at soft reset.
     *  Disable = Do Not Skip: Reset the Chiplets on SW Reset.
     *  Enable  = Skip: Do not reset the Chiplets on SW Reset.
     *  (APPLICABLE DEVICES: Falcon)
     *  This feature cofigured on Main Dies only.
    */
    CPSS_HW_PP_RESET_SKIP_TYPE_CHIPLETS_E,

    /** @brief Skips initialization of the POE unit
     *  Disable = Do Not Skip:  Initialize the POE unit on SW reset
     *  Enable  = Skip: Skips initialization of the POE unit on SW reset
     *  (APPLICABLE DEVICES: AC5, AC5X)
    */
    CPSS_HW_PP_RESET_SKIP_TYPE_POE_E,

    /** @brief All options above
     *  Disable = Do Not Skip on SW Reset.
     *  Enable  = Skip on SW Reset.
    */
    CPSS_HW_PP_RESET_SKIP_TYPE_ALL_E,

    /** @brief like CPSS_HW_PP_RESET_SKIP_TYPE_ALL_E but
     *  exclude CPSS_HW_PP_RESET_SKIP_TYPE_PEX_E
     *  this case is useful when wanting the device to do soft reset
     *  without the need to reconfigure the PEX again.
    */
    CPSS_HW_PP_RESET_SKIP_TYPE_ALL_EXCLUDE_PEX_E
} CPSS_HW_PP_RESET_SKIP_TYPE_ENT;


/**
* @enum CPSS_HW_PP_INIT_STAGE_ENT
 *
 * @brief Indicates the initialization stage of the device.
 *
 */
typedef enum {
    /** Initialization is during reset*/
    CPSS_HW_PP_INIT_STAGE_INIT_DURING_RESET_E = 0,

    /** @brief Initialize after reset.
     *  EEPROM initialization is done and Internal memory
     *  initialization is not done.
    */
    CPSS_HW_PP_INIT_STAGE_EEPROM_DONE_INT_MEM_DONE_E,

    /** @brief Initialize after reset.
     *  Internal memory initialization is done and EEPROM
     *  initialization is not done.
    */
    CPSS_HW_PP_INIT_STAGE_EEPROM_NOT_DONE_INT_MEM_DONE_E,

    /** Initialization is fully functional.*/
    CPSS_HW_PP_INIT_STAGE_FULLY_FUNC_E
} CPSS_HW_PP_INIT_STAGE_ENT;

/**
 * @enum CPSS_HW_PP_SOFT_RESET_OPTIONS_ENT
 *
 *  @brief Defines a set of skip options, each supporting specific functional use case.
 *
 *  All activities between CnM and CPU should be halted before the CPU reset:
 *   - Traffic to CPU
 *   - Traffic from CPU and traffic generator
 *   - AUQ, FUQ, CNC Upload
 *   - Service CPU transactions (only in case of Ac5 and AC5X internal cpu)
 */
typedef enum {
    /** @brief Reset all components without skipping any.
    */
    CPSS_HW_PP_RESET_ALL_SKIP_NONE_E = 0,

    /** @brief Reset CnM and PCIe, skip Switch and POE (POE is skipped for applicable devices).
    */
    CPSS_HW_PP_RESET_CNM_PCIE_SKIP_SWITCH_POE_E,

    /** @brief Reset CnM, skip Switch, PCIe and POE (POE is skipped for applicable devices).
    */
    CPSS_HW_PP_RESET_CNM_SKIP_SWITCH_PCIE_POE_E,

    /** @brief Reset Switch, skip PCIe and CnM
    *
    *  POE is automatically skipped for applicable devices as it's a part of CnM, which is skipped.
    */
    CPSS_HW_PP_RESET_SWITCH_SKIP_PCIE_CNM_E,

    /** @brief Reset internal CPUs, skip Switch and PoE
    *
    *  POE is automatically skipped for applicable devices as it's a part of CnM, which is skipped.
    *
    *  @note APPLICABLE DEVICES: AC5; AC5X; Ironman.
    */
    CPSS_HW_PP_RESET_INTERNAL_CPU_SKIP_SWITCH_POE_E,

    /* Should be last */
    CPSS_HW_PP_SOFT_RESET_OPTIONS_LAST_E

} CPSS_HW_PP_SOFT_RESET_OPTIONS_ENT;

/**
* @internal cpssPpInterruptsDisable function
* @endinternal
*
* @brief   This API disable PP interrupts
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Puma2; Puma3; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device number.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on hardware error
* @retval GT_NOT_INITIALIZED       - if the driver was not initialized
* @retval GT_HW_ERROR              - on hardware error
*/
GT_STATUS cpssPpInterruptsDisable
(
    IN GT_U8     devNum
);


/**
* @internal cpssPpConfigDevDataImport function
* @endinternal
*
* @brief   Imports data for device specific information. Used in catch up process
*         during initialization as the first step in the catch up process for
*         configuring the standby CPU.
*         NOTE: for a device that will be initialized in "pp phase 1" with
*         High availability mode of "standby" , this function MUST be called prior
*         to the "pp phase 1"
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Puma2; Puma3; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device number.
* @param[in] configDevDataBufferPtr   - pointer to a pre allocated buffer for holding
*                                      information on devices in the system
* @param[in] configDevDataBufferSize  - size of the pre allocated buffer
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - wrong device Number or configDevDataBufferSize
*/
GT_STATUS cpssPpConfigDevDataImport
(
    IN GT_U8     devNum,
    IN void      *configDevDataBufferPtr,
    IN GT_U32    configDevDataBufferSize
);

/**
* @internal cpssPpConfigDevDataExport function
* @endinternal
*
* @brief   Imports data for device specific information. Used in catch up process
*         during initialization as the first step in the catch up process for
*         configuring the standby CPU.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Puma2; Puma3; ExMx.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - The device number.
* @param[in,out] configDevDataBufferPtr   - pointer to a pre allocated buffer for
*                                      holding information on devices in the system
* @param[in,out] configDevDataBufferSizePtr - size of the pre allocated buffer
* @param[in,out] configDevDataBufferPtr   - pointer to a pre allocated buffer for
*                                      holding information on devices in the system
*                                      filled by the function.
* @param[in,out] configDevDataBufferSizePtr - size of the filled data in the pre
*                                      allocated buffer, in case of failure
*                                      (GT_BAD_SIZE), needed size of the block
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_SIZE              - block is not big enough to hold all the data
* @retval GT_BAD_PARAM             - wrong device Number
*/
GT_STATUS cpssPpConfigDevDataExport
(
    IN    GT_U8     devNum,
    INOUT void      *configDevDataBufferPtr,
    INOUT GT_U32    *configDevDataBufferSizePtr
);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssHwInit_h */


