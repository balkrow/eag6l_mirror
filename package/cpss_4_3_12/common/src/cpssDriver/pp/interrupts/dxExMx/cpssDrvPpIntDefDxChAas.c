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
* @file cpssDrvPpIntDefDxChAas.c
*
* @brief This file includes the definition and initialization of the interrupts
* init. parameters tables. -- DxCh aas devices
*
* @version   1
********************************************************************************
*/

#include <cpssDriver/pp/interrupts/dxExMx/prvCpssDrvDxExMxInterrupts.h>
#include <cpssDriver/pp/interrupts/dxExMx/prvCpssDrvDxChEventsAas.h>
#include <cpssDriver/pp/interrupts/dxExMx/prvCpssDrvExMxUtilLion.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwRegAddr.h>
#include <cpss/dxCh/dxChxGen/config/private/prvCpssDxChInfo.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwRegAddrVer1_ppDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <cpssDriver/pp/config/prvCpssDrvPpCfgInit.h>/*prvCpssDrvEventUpdate(...)*/

#define INT_TILE_OFFSET(_tile)     ((_tile)*PRV_CPSS_AAS_INT_TILE_OFFSET_E)

/* First macsec register id as per Aas interrupts enum */
#define  PRV_CPSS_DXCH_AAS_MACSEC_REGID_FIRST_CNS (PRV_CPSS_AAS_EIP163E_MACSEC_TYPE_1_EIP163E_MACSEC_INSTANCE_0_GLOBAL_MAC_SEC_INTERRUPT_CAUSE_STAT_TCAM_THR_E >> 5)
#define  PRV_CPSS_DXCH_UNIT_RXDMA0_E   PRV_CPSS_DXCH_UNIT_RXDMA_E
#define  PRV_CPSS_DXCH_UNIT_TXDMA0_E   PRV_CPSS_DXCH_UNIT_TXDMA_E
#define  PRV_CPSS_DXCH_UNIT_TX_FIFO0_E PRV_CPSS_DXCH_UNIT_TX_FIFO_E

#define GDMA_RX_CONVERT(_tileId,_gdmaId,_queueId,_type_old,_type_new)              \
    PRV_CPSS_AAS_CNM_CNM_TILE_##_tileId##_GDMA_##_gdmaId##_QUEUE_##_queueId##_##_type_old##_INTERRUPT_E = \
    PRV_CPSS_AAS_CNM_CNM_TILE_##_tileId##_GDMA_##_gdmaId##_QUEUE_##_queueId##_##_type_new##_INTERRUPT_E

#define AAS_GDMA0_RINGS_RX_CONVERT(_tileId,_type_old,_type_new)         \
     GDMA_RX_CONVERT(_tileId, 0, 0,_type_old,_type_new)             \
    ,GDMA_RX_CONVERT(_tileId, 0, 1,_type_old,_type_new)             \
    ,GDMA_RX_CONVERT(_tileId, 0, 2,_type_old,_type_new)             \
    ,GDMA_RX_CONVERT(_tileId, 0, 3,_type_old,_type_new)             \
    ,GDMA_RX_CONVERT(_tileId, 0, 4,_type_old,_type_new)             \
    ,GDMA_RX_CONVERT(_tileId, 0, 5,_type_old,_type_new)             \
    ,GDMA_RX_CONVERT(_tileId, 0, 6,_type_old,_type_new)             \
    ,GDMA_RX_CONVERT(_tileId, 0, 7,_type_old,_type_new)             \
    ,GDMA_RX_CONVERT(_tileId, 0, 8,_type_old,_type_new)             \
    ,GDMA_RX_CONVERT(_tileId, 0, 9,_type_old,_type_new)             \
    ,GDMA_RX_CONVERT(_tileId, 0,10,_type_old,_type_new)             \
    ,GDMA_RX_CONVERT(_tileId, 0,11,_type_old,_type_new)             \
    ,GDMA_RX_CONVERT(_tileId, 0,12,_type_old,_type_new)             \
    ,GDMA_RX_CONVERT(_tileId, 0,13,_type_old,_type_new)             \
    ,GDMA_RX_CONVERT(_tileId, 0,14,_type_old,_type_new)             \
    ,GDMA_RX_CONVERT(_tileId, 0,15,_type_old,_type_new)             \
    ,GDMA_RX_CONVERT(_tileId, 0,16,_type_old,_type_new)             \
    ,GDMA_RX_CONVERT(_tileId, 0,17,_type_old,_type_new)             \
    ,GDMA_RX_CONVERT(_tileId, 0,18,_type_old,_type_new)             \
    ,GDMA_RX_CONVERT(_tileId, 0,19,_type_old,_type_new)             \
    ,GDMA_RX_CONVERT(_tileId, 0,20,_type_old,_type_new)             \
    ,GDMA_RX_CONVERT(_tileId, 0,21,_type_old,_type_new)             \
    ,GDMA_RX_CONVERT(_tileId, 0,22,_type_old,_type_new)             \
    ,GDMA_RX_CONVERT(_tileId, 0,23,_type_old,_type_new)             \
    ,GDMA_RX_CONVERT(_tileId, 0,24,_type_old,_type_new)             \
    ,GDMA_RX_CONVERT(_tileId, 0,25,_type_old,_type_new)             \
    ,GDMA_RX_CONVERT(_tileId, 0,26,_type_old,_type_new)             \
    ,GDMA_RX_CONVERT(_tileId, 0,27,_type_old,_type_new)             \
    ,GDMA_RX_CONVERT(_tileId, 0,28,_type_old,_type_new)             \
    ,GDMA_RX_CONVERT(_tileId, 0,29,_type_old,_type_new)             \
    ,GDMA_RX_CONVERT(_tileId, 0,30,_type_old,_type_new)

#define AAS_GDMA1_RINGS_RX_CONVERT(_tileId,_type_old,_type_new)         \
     GDMA_RX_CONVERT(_tileId, 0,31,_type_old,_type_new)             \
    ,GDMA_RX_CONVERT(_tileId, 1, 0,_type_old,_type_new)             \
    ,GDMA_RX_CONVERT(_tileId, 1, 1,_type_old,_type_new)             \
    ,GDMA_RX_CONVERT(_tileId, 1, 2,_type_old,_type_new)             \
    ,GDMA_RX_CONVERT(_tileId, 1, 3,_type_old,_type_new)             \
    ,GDMA_RX_CONVERT(_tileId, 1, 4,_type_old,_type_new)             \
    ,GDMA_RX_CONVERT(_tileId, 1, 5,_type_old,_type_new)             \
    ,GDMA_RX_CONVERT(_tileId, 1, 6,_type_old,_type_new)             \
    ,GDMA_RX_CONVERT(_tileId, 1, 7,_type_old,_type_new)             \
    ,GDMA_RX_CONVERT(_tileId, 1, 8,_type_old,_type_new)             \
    ,GDMA_RX_CONVERT(_tileId, 1, 9,_type_old,_type_new)             \
    ,GDMA_RX_CONVERT(_tileId, 1,10,_type_old,_type_new)             \
    ,GDMA_RX_CONVERT(_tileId, 1,11,_type_old,_type_new)             \
    ,GDMA_RX_CONVERT(_tileId, 1,12,_type_old,_type_new)             \
    ,GDMA_RX_CONVERT(_tileId, 1,13,_type_old,_type_new)             \
    ,GDMA_RX_CONVERT(_tileId, 1,14,_type_old,_type_new)             \
    ,GDMA_RX_CONVERT(_tileId, 1,15,_type_old,_type_new)             \
    ,GDMA_RX_CONVERT(_tileId, 1,16,_type_old,_type_new)             \
    ,GDMA_RX_CONVERT(_tileId, 1,17,_type_old,_type_new)             \
    ,GDMA_RX_CONVERT(_tileId, 1,18,_type_old,_type_new)             \
    ,GDMA_RX_CONVERT(_tileId, 1,19,_type_old,_type_new)             \
    ,GDMA_RX_CONVERT(_tileId, 1,20,_type_old,_type_new)             \
    ,GDMA_RX_CONVERT(_tileId, 1,21,_type_old,_type_new)             \
    ,GDMA_RX_CONVERT(_tileId, 1,22,_type_old,_type_new)             \
    ,GDMA_RX_CONVERT(_tileId, 1,23,_type_old,_type_new)             \
    ,GDMA_RX_CONVERT(_tileId, 1,24,_type_old,_type_new)             \
    ,GDMA_RX_CONVERT(_tileId, 1,25,_type_old,_type_new)             \
    ,GDMA_RX_CONVERT(_tileId, 1,26,_type_old,_type_new)             \
    ,GDMA_RX_CONVERT(_tileId, 1,27,_type_old,_type_new)             \
    ,GDMA_RX_CONVERT(_tileId, 1,28,_type_old,_type_new)             \
    ,GDMA_RX_CONVERT(_tileId, 1,29,_type_old,_type_new)

#define AAS_GDMA2_RINGS_RX_CONVERT(_tileId,_type_old,_type_new)         \
     GDMA_RX_CONVERT(_tileId, 1,30,_type_old,_type_new)             \
    ,GDMA_RX_CONVERT(_tileId, 1,31,_type_old,_type_new)             \
    ,GDMA_RX_CONVERT(_tileId, 2, 0,_type_old,_type_new)             \
    ,GDMA_RX_CONVERT(_tileId, 2, 1,_type_old,_type_new)             \
    ,GDMA_RX_CONVERT(_tileId, 2, 2,_type_old,_type_new)             \
    ,GDMA_RX_CONVERT(_tileId, 2, 3,_type_old,_type_new)             \
    ,GDMA_RX_CONVERT(_tileId, 2, 4,_type_old,_type_new)             \
    ,GDMA_RX_CONVERT(_tileId, 2, 5,_type_old,_type_new)             \
    ,GDMA_RX_CONVERT(_tileId, 2, 6,_type_old,_type_new)             \
    ,GDMA_RX_CONVERT(_tileId, 2, 7,_type_old,_type_new)             \
    ,GDMA_RX_CONVERT(_tileId, 2, 8,_type_old,_type_new)             \
    ,GDMA_RX_CONVERT(_tileId, 2, 9,_type_old,_type_new)             \
    ,GDMA_RX_CONVERT(_tileId, 2,10,_type_old,_type_new)             \
    ,GDMA_RX_CONVERT(_tileId, 2,11,_type_old,_type_new)             \
    ,GDMA_RX_CONVERT(_tileId, 2,12,_type_old,_type_new)             \
    ,GDMA_RX_CONVERT(_tileId, 2,13,_type_old,_type_new)             \
    ,GDMA_RX_CONVERT(_tileId, 2,14,_type_old,_type_new)             \
    ,GDMA_RX_CONVERT(_tileId, 2,15,_type_old,_type_new)             \
    ,GDMA_RX_CONVERT(_tileId, 2,16,_type_old,_type_new)             \
    ,GDMA_RX_CONVERT(_tileId, 2,17,_type_old,_type_new)             \
    ,GDMA_RX_CONVERT(_tileId, 2,18,_type_old,_type_new)             \
    ,GDMA_RX_CONVERT(_tileId, 2,19,_type_old,_type_new)             \
    ,GDMA_RX_CONVERT(_tileId, 2,20,_type_old,_type_new)             \
    ,GDMA_RX_CONVERT(_tileId, 2,21,_type_old,_type_new)             \
    ,GDMA_RX_CONVERT(_tileId, 2,22,_type_old,_type_new)             \
    ,GDMA_RX_CONVERT(_tileId, 2,23,_type_old,_type_new)             \
    ,GDMA_RX_CONVERT(_tileId, 2,24,_type_old,_type_new)             \
    ,GDMA_RX_CONVERT(_tileId, 2,25,_type_old,_type_new)             \
    ,GDMA_RX_CONVERT(_tileId, 2,26,_type_old,_type_new)             \
    ,GDMA_RX_CONVERT(_tileId, 2,27,_type_old,_type_new)             \
    ,GDMA_RX_CONVERT(_tileId, 2,28,_type_old,_type_new)

#define AAS_GDMA3_RINGS_RX_CONVERT(_tileId,_type_old,_type_new)         \
     GDMA_RX_CONVERT(_tileId, 2,29,_type_old,_type_new)             \
    ,GDMA_RX_CONVERT(_tileId, 2,30,_type_old,_type_new)             \
    ,GDMA_RX_CONVERT(_tileId, 2,31,_type_old,_type_new)             \
    ,GDMA_RX_CONVERT(_tileId, 3, 0,_type_old,_type_new)             \
    ,GDMA_RX_CONVERT(_tileId, 3, 1,_type_old,_type_new)             \
    ,GDMA_RX_CONVERT(_tileId, 3, 2,_type_old,_type_new)             \
    ,GDMA_RX_CONVERT(_tileId, 3, 3,_type_old,_type_new)             \
    ,GDMA_RX_CONVERT(_tileId, 3, 4,_type_old,_type_new)             \
    ,GDMA_RX_CONVERT(_tileId, 3, 5,_type_old,_type_new)             \
    ,GDMA_RX_CONVERT(_tileId, 3, 6,_type_old,_type_new)             \
    ,GDMA_RX_CONVERT(_tileId, 3, 7,_type_old,_type_new)             \
    ,GDMA_RX_CONVERT(_tileId, 3, 8,_type_old,_type_new)             \
    ,GDMA_RX_CONVERT(_tileId, 3, 9,_type_old,_type_new)             \
    ,GDMA_RX_CONVERT(_tileId, 3,10,_type_old,_type_new)             \
    ,GDMA_RX_CONVERT(_tileId, 3,11,_type_old,_type_new)             \
    ,GDMA_RX_CONVERT(_tileId, 3,12,_type_old,_type_new)             \
    ,GDMA_RX_CONVERT(_tileId, 3,13,_type_old,_type_new)             \
    ,GDMA_RX_CONVERT(_tileId, 3,14,_type_old,_type_new)             \
    ,GDMA_RX_CONVERT(_tileId, 3,15,_type_old,_type_new)             \
    ,GDMA_RX_CONVERT(_tileId, 3,16,_type_old,_type_new)             \
    ,GDMA_RX_CONVERT(_tileId, 3,17,_type_old,_type_new)             \
    ,GDMA_RX_CONVERT(_tileId, 3,18,_type_old,_type_new)             \
    ,GDMA_RX_CONVERT(_tileId, 3,19,_type_old,_type_new)             \
    ,GDMA_RX_CONVERT(_tileId, 3,20,_type_old,_type_new)             \
    ,GDMA_RX_CONVERT(_tileId, 3,21,_type_old,_type_new)             \
    ,GDMA_RX_CONVERT(_tileId, 3,22,_type_old,_type_new)             \
    ,GDMA_RX_CONVERT(_tileId, 3,23,_type_old,_type_new)             \
    ,GDMA_RX_CONVERT(_tileId, 3,24,_type_old,_type_new)             \
    ,GDMA_RX_CONVERT(_tileId, 3,25,_type_old,_type_new)             \
    ,GDMA_RX_CONVERT(_tileId, 3,26,_type_old,_type_new)             \
    ,GDMA_RX_CONVERT(_tileId, 3,27,_type_old,_type_new)

#define AAS_GDMA4_RINGS_RX_CONVERT(_tileId,_type_old,_type_new)         \
     GDMA_RX_CONVERT(_tileId, 3,28,_type_old,_type_new)             \
    ,GDMA_RX_CONVERT(_tileId, 3,29,_type_old,_type_new)             \
    ,GDMA_RX_CONVERT(_tileId, 3,30,_type_old,_type_new)             \
    ,GDMA_RX_CONVERT(_tileId, 3,31,_type_old,_type_new)

#define AAS_GDMA_RX_CONVERT_ALL_32_RINGS(_tileId,_type_old,_type_new)   \
    AAS_GDMA0_RINGS_RX_CONVERT(_tileId,_type_old,_type_new)             \
   ,AAS_GDMA1_RINGS_RX_CONVERT(_tileId,_type_old,_type_new)             \
   ,AAS_GDMA2_RINGS_RX_CONVERT(_tileId,_type_old,_type_new)             \
   ,AAS_GDMA3_RINGS_RX_CONVERT(_tileId,_type_old,_type_new)             \
   ,AAS_GDMA4_RINGS_RX_CONVERT(_tileId,_type_old,_type_new)

/* needed by AAS_ALL_GDMA_RX_INTERRUPTS_MAC */
#define   AAS_GDMA_PER_UNIT_RX_CONVERT_ALL_OLD_TYPES(_tileId)           \
     AAS_GDMA_RX_CONVERT_ALL_32_RINGS(_tileId, RX_BUFFER,   PACKET_DONE)\
    ,AAS_GDMA_RX_CONVERT_ALL_32_RINGS(_tileId, RX_ERR,      PACKET_DROP)

enum{
    AAS_GDMA_PER_UNIT_RX_CONVERT_ALL_OLD_TYPES(0),
    AAS_GDMA_PER_UNIT_RX_CONVERT_ALL_OLD_TYPES(1)
};

/* needed by : AAS_ALL_GDMA_TX_INTERRUPTS_MAC */
#define AAS_GDMA_PER_UNIT_TX_CONVERT_ALL_OLD_TYPES(_tileId)                         \
     AAS_GDMA_RX_CONVERT_ALL_32_RINGS(_tileId, TX_BUFFER_QUEUE   ,TX_SGD_DONE)   \
    ,AAS_GDMA_RX_CONVERT_ALL_32_RINGS(_tileId, TX_ERR_QUEUE      ,PACKET_DROP)   \
    ,AAS_GDMA_RX_CONVERT_ALL_32_RINGS(_tileId, TX_END            ,PACKET_DONE)

enum{
    AAS_GDMA_PER_UNIT_TX_CONVERT_ALL_OLD_TYPES(0),
    AAS_GDMA_PER_UNIT_TX_CONVERT_ALL_OLD_TYPES(1)
};

/* convert bit in the enum of the huge interrupts of the device into BIT_0..BIT_31 in a register */
#define DEVICE_INT_TO_BIT_MAC(deviceInterruptId)    \
            (1 << ((deviceInterruptId) & 0x1f))

#define FDB_REG_MASK_CNS    \
    (0xFFFFFFFF & ~                                                             \
        (DEVICE_INT_TO_BIT_MAC(PRV_CPSS_AAS_FDB_FDB_INTERRUPT_CAUSE_REGISTER_AGE_VIA_TRIGGER_ENDED_E) |     \
         DEVICE_INT_TO_BIT_MAC(PRV_CPSS_AAS_FDB_FDB_INTERRUPT_CAUSE_REGISTER_AU_PROC_COMPLETED_INT_E)))

/* unit : PRV_CPSS_DXCH_UNIT_CNM_INTERRUPTS_E */
#define CNM_INTERRUPTS_UNIT(_tile) \
    (PRV_CPSS_DXCH_UNIT_DISTANCE_BETWEEN_TILES_CNS*(_tile) +  PRV_CPSS_DXCH_UNIT_CNM_INTERRUPTS_E)

/* unit of type PRV_CPSS_DXCH_UNIT_ENT */
#define UNIT_IN_TILE(_tile,unit)                                           \
    ((unit) + (PRV_CPSS_DXCH_UNIT_DISTANCE_BETWEEN_TILES_CNS*(_tile)))

#define CNM_RFU_INT_ADDR_MAC(_cause, _mask, _treeId)                       \
     ((_cause) + (0x100 * (_treeId))), ((_mask) + (0x100 * (_treeId)))

#define GDMA_INT_ADDR_MAC(_cause, _mask, _index)                           \
     ((_cause) + (0x4 * (_index))), ((_mask) + (0x4 * (_index)))

#define UNIT_ID_GET(unit) ((unit) & 0xFFFF)

#define AAS_LMU_LATENCY_INTERRUPT_CAUSE_MAC(_threshold)                    \
        ((0x8100 ) + (0x4 * (_threshold)))

#define AAS_LMU_LATENCY_INTERRUPT_MASK_MAC(_threshold)                     \
        ((0x8180 ) + (0x4 * (_threshold)))


/* MAC - cause */
#define PORT_MAC_INTERRUPT_CAUSE_MAC(port)                             \
        (0x94 + ((port) * 0x18))
/* MAC - mask */
#define PORT_MAC_INTERRUPT_MASK_MAC(port)                              \
        (0x98 + ((port) * 0x18))

/* SEG MAC - cause */
#define PORT_SEG_MAC_INTERRUPT_CAUSE_MAC(port)                              \
        (0x68 + ((port) * 0x14))
/* SEG - mask */
#define PORT_SEG_MAC_INTERRUPT_MASK_MAC(port)                                 \
        (0x6c + ((port) * 0x14))

/* AAS device tile offset */
#define AAS_TILE_OFFSET_CNS    0x80000000

/* in sip7 the registers in fdb unit starts at 0x02000000*/
#define AAS_FDB_REG_RELATIVE_BASE   0x02000000

#define   mt_NODE_MAC(_tile,bit)                                                                          \
    /* MT FDBIntSum - FDB Interrupt Cause */                                                        \
    {bit, GT_FALSE, UNIT_IN_TILE(_tile,PRV_CPSS_DXCH_UNIT_FDB_E), NULL,                                                 \
        AAS_FDB_REG_RELATIVE_BASE + 0x000001b0, AAS_FDB_REG_RELATIVE_BASE + 0x000001b4,             \
        prvCpssDrvHwPpPortGroupIsrRead,                                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_FDB_FDB_INTERRUPT_CAUSE_REGISTER_NUM_OF_HOP_EX_P_E,                           \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_FDB_FDB_INTERRUPT_CAUSE_REGISTER_ADDRESS_OUT_OF_RANGE_E,                      \
        FILLED_IN_RUNTIME_CNS, 0x0, FDB_REG_MASK_CNS,                                               \
             0, NULL, NULL}

#define   em_NODE_MAC(_tile,bit)                                                                          \
    /* EM FDBIntSum - FDB Interrupt Cause */                                                        \
    {bit, GT_FALSE, UNIT_IN_TILE(_tile,PRV_CPSS_DXCH_UNIT_EM_E), NULL,                                                  \
        0x00004000, 0x00004004,                                                                     \
        prvCpssDrvHwPpPortGroupIsrRead,                                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_EM_EM_INTERRUPT_CAUSE_REGISTER_INDIRECT_ADDRESS_OUT_OF_RANGE_E,               \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_EM_EM_INTERRUPT_CAUSE_REGISTER_CHANNEL_1_SHM_ANSWER_FIFO_OVERFLOW_E,          \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFEF,                                                     \
             0, NULL, NULL}

#define AAS_LMU_PCA_LATENCY_OVER_THRESHOLD_INT_MAC(_tile,bit, _instance, _threshold)                     \
    {bit, GT_FALSE, UNIT_IN_TILE(_tile,PRV_CPSS_DXCH_UNIT_PCA_LMU_##_instance##_E), NULL,                               \
        AAS_LMU_LATENCY_INTERRUPT_CAUSE_MAC(_threshold),                                           \
        AAS_LMU_LATENCY_INTERRUPT_MASK_MAC(_threshold),                                            \
        prvCpssDrvHwPpPortGroupIsrRead,                                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_LMU_INSTANCE_##_instance##_N_##_threshold##_I_0_LMU_LATENCY_OVER_THRESHOLD_E, \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_LMU_INSTANCE_##_instance##_N_##_threshold##_I_30_LMU_LATENCY_OVER_THRESHOLD_E,\
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define lmu_PCA_SUB_TREE_MAC(_tile,bit, _instance)                                                        \
    /* LMU Cause Summary */                                                                         \
    {bit, GT_FALSE, UNIT_IN_TILE(_tile,PRV_CPSS_DXCH_UNIT_PCA_LMU_##_instance##_E), NULL,                               \
        0x00008230, 0x00008234,                                                                     \
        prvCpssDrvHwPpPortGroupIsrRead,                                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_LMU_INSTANCE_##_instance##_LMU_CAUSE_SUMMARY_N_0_LMU_LATENCY_OVER_THRESHOLD_SUM_E,  \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_LMU_INSTANCE_##_instance##_LMU_CAUSE_SUMMARY_N_16_LMU_LATENCY_OVER_THRESHOLD_SUM_E,       \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 17, NULL, NULL},                                         \
        AAS_LMU_PCA_LATENCY_OVER_THRESHOLD_INT_MAC(_tile, 1, _instance,  0),  \
        AAS_LMU_PCA_LATENCY_OVER_THRESHOLD_INT_MAC(_tile, 2, _instance,  1),  \
        AAS_LMU_PCA_LATENCY_OVER_THRESHOLD_INT_MAC(_tile, 3, _instance,  2),  \
        AAS_LMU_PCA_LATENCY_OVER_THRESHOLD_INT_MAC(_tile, 4, _instance,  3),  \
        AAS_LMU_PCA_LATENCY_OVER_THRESHOLD_INT_MAC(_tile, 5, _instance,  4),  \
        AAS_LMU_PCA_LATENCY_OVER_THRESHOLD_INT_MAC(_tile, 6, _instance,  5),  \
        AAS_LMU_PCA_LATENCY_OVER_THRESHOLD_INT_MAC(_tile, 7, _instance,  6),  \
        AAS_LMU_PCA_LATENCY_OVER_THRESHOLD_INT_MAC(_tile, 8, _instance,  7),  \
        AAS_LMU_PCA_LATENCY_OVER_THRESHOLD_INT_MAC(_tile, 9, _instance,  8),  \
        AAS_LMU_PCA_LATENCY_OVER_THRESHOLD_INT_MAC(_tile,10, _instance,  9),  \
        AAS_LMU_PCA_LATENCY_OVER_THRESHOLD_INT_MAC(_tile,11, _instance, 10),  \
        AAS_LMU_PCA_LATENCY_OVER_THRESHOLD_INT_MAC(_tile,12, _instance, 11),  \
        AAS_LMU_PCA_LATENCY_OVER_THRESHOLD_INT_MAC(_tile,13, _instance, 12),  \
        AAS_LMU_PCA_LATENCY_OVER_THRESHOLD_INT_MAC(_tile,14, _instance, 13),  \
        AAS_LMU_PCA_LATENCY_OVER_THRESHOLD_INT_MAC(_tile,15, _instance, 14),  \
        AAS_LMU_PCA_LATENCY_OVER_THRESHOLD_INT_MAC(_tile,16, _instance, 15),  \
        AAS_LMU_PCA_LATENCY_OVER_THRESHOLD_INT_MAC(_tile,17, _instance, 16)

#define anp_global_NODE_MAC(_tile,bit, _instance)                                              \
    /* ANP */                                                                                      \
    {bit, GT_FALSE, UNIT_IN_TILE(_tile,PRV_CPSS_DXCH_UNIT_ANP_400G_##_instance##_E), NULL,                                  \
        0x00000088, 0x0000008c,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_ANP_INSTANCE_##_instance##_GLOBAL_INTERRUPT_CAUSE_ILLEGAL_ADDRESS_ACCESS_E,      \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_ANP_INSTANCE_##_instance##_GLOBAL_INTERRUPT_CAUSE_ILLEGAL_ADDRESS_ACCESS_E,      \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}


#define anp_port_NODE_MAC(_tile,bit, _instance, _port)                                                      \
    /* ANP */                                                                                       \
    {bit, GT_FALSE, UNIT_IN_TILE(_tile,PRV_CPSS_DXCH_UNIT_ANP_400G_##_instance##_E), NULL,                                  \
        0x00000100 + (_port * 0x2a4), 0x00000104 + (_port * 0x2a4),                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_ANP_INSTANCE_##_instance##_PORT_NUM_##_port##_AN_RESTART_E,                             \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_ANP_INSTANCE_##_instance##_PORT_NUM_##_port##_INT_WAIT_PWRDN_TIME_OUT_E,               \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define anp_port2_NODE_MAC(_tile,bit, _instance, _port)                                                  \
    /*  ANP */                                                                                      \
    {bit, GT_FALSE, UNIT_IN_TILE(_tile,PRV_CPSS_DXCH_UNIT_ANP_400G_##_instance##_E), NULL,                                  \
        0x00000108 + (_port * 0x2a4), 0x0000010c + (_port * 0x2a4),                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_ANP_INSTANCE_##_instance##_PORT_NUM_##_port##_TX_TFIFO_W_ERR_E,                \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_ANP_INSTANCE_##_instance##_PORT_NUM_##_port##_RX_TFIFO_EMPTY_E,                \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define anp_SUB_TREE_MAC(_tile,bit, _instance)                                                       \
    /* ANP */                                                                                  \
    {bit, GT_FALSE, UNIT_IN_TILE(_tile,PRV_CPSS_DXCH_UNIT_ANP_400G_##_instance##_E), NULL,                         \
        0x00000094, 0x00000098,                                                                \
        prvCpssDrvHwPpPortGroupIsrRead,                                                        \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                       \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_ANP_INSTANCE_##_instance##_INTERRUPT_SUMMARY_CAUSE_GLOBAL_INT_E,      \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_ANP_INSTANCE_##_instance##_INTERRUPT_SUMMARY_CAUSE_PORT_NUM_7_P_INT2_SUM_E,      \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 17, NULL, NULL},                               \
        anp_global_NODE_MAC(_tile,1, _instance),                                                     \
        anp_port_NODE_MAC(_tile,2, _instance, 0),                                                    \
        anp_port2_NODE_MAC(_tile,3, _instance, 0),                                                   \
        anp_port_NODE_MAC(_tile,4, _instance, 1),                                                    \
        anp_port2_NODE_MAC(_tile,5, _instance, 1),                                                   \
        anp_port_NODE_MAC(_tile,6, _instance, 2),                                                    \
        anp_port2_NODE_MAC(_tile,7, _instance, 2),                                                   \
        anp_port_NODE_MAC(_tile,8, _instance, 3),                                                    \
        anp_port2_NODE_MAC(_tile,9, _instance, 3),                                                   \
        anp_port_NODE_MAC(_tile,10, _instance, 4),                                                   \
        anp_port2_NODE_MAC(_tile,11, _instance, 4),                                                  \
        anp_port_NODE_MAC(_tile,12, _instance, 5),                                                   \
        anp_port2_NODE_MAC(_tile,13, _instance, 5),                                                  \
        anp_port_NODE_MAC(_tile,14, _instance, 6),                                                   \
        anp_port2_NODE_MAC(_tile,15, _instance, 6),                                                  \
        anp_port_NODE_MAC(_tile,16, _instance, 7),                                                   \
        anp_port2_NODE_MAC(_tile,17, _instance, 7)

#define anp_half_SUB_TREE_MAC(_tile,bit, _instance)                                                       \
    /* ANP */                                                                                  \
    {bit, GT_FALSE, UNIT_IN_TILE(_tile,PRV_CPSS_DXCH_UNIT_ANP_400G_##_instance##_E), NULL,                         \
        0x00000094, 0x00000098,                                                                \
        prvCpssDrvHwPpPortGroupIsrRead,                                                        \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                       \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_ANP_INSTANCE_##_instance##_INTERRUPT_SUMMARY_CAUSE_GLOBAL_INT_E,      \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_ANP_INSTANCE_##_instance##_INTERRUPT_SUMMARY_CAUSE_PORT_NUM_7_P_INT2_SUM_E,      \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 9, NULL, NULL},                                \
        anp_global_NODE_MAC(_tile,1, _instance),                                                     \
        anp_port_NODE_MAC(_tile,2, _instance, 0),                                                    \
        anp_port2_NODE_MAC(_tile,3, _instance, 0),                                                   \
        anp_port_NODE_MAC(_tile,4, _instance, 1),                                                    \
        anp_port2_NODE_MAC(_tile,5, _instance, 1),                                                   \
        anp_port_NODE_MAC(_tile,6, _instance, 2),                                                    \
        anp_port2_NODE_MAC(_tile,7, _instance, 2),                                                   \
        anp_port_NODE_MAC(_tile,8, _instance, 3),                                                    \
        anp_port2_NODE_MAC(_tile,9, _instance, 3)


#define pdx_NODE_MAC(_tile,bit)                                                                           \
    /* PDX Interrupt Summary */                                                                     \
    {bit, GT_FALSE, UNIT_IN_TILE(_tile,PRV_CPSS_DXCH_UNIT_TXQ_PDX_E), NULL,                                             \
        0x00001408, 0x0000140c,                                                                     \
        prvCpssDrvHwPpPortGroupIsrRead,                                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_TXQ_PR_PDX_FUNCTIONAL_INTERRUPT_CAUSE_ILLEGAL_ADDRESS_ACCESS_E,            \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_TXQ_PR_PDX_FUNCTIONAL_INTERRUPT_CAUSE_CHANNELS_RANGE_2_DX_DESC_DROP_E,     \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define pfcc_NODE_MAC(_tile,bit)                                                                          \
    /* PFCC Interrupt Summary */                                                                    \
    {bit, GT_FALSE, UNIT_IN_TILE(_tile,PRV_CPSS_DXCH_UNIT_TXQ_PFCC_E), NULL,                                            \
        0x00000100, 0x00000104,                                                                     \
        prvCpssDrvHwPpPortGroupIsrRead,                                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_TXQ_PR_PFCC_INTERRUPT_CAUSE_ILLEGAL_ADDRESS_ACCESS_E,                         \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_TXQ_PR_PFCC_INTERRUPT_CAUSE_SOURCE_PORT_COUNTER_OVERFLOW_E,                   \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define psi_NODE_MAC(_tile,bit)                                                                           \
    /* PSI Interrupt Summary */                                                                     \
    {bit, GT_FALSE, UNIT_IN_TILE(_tile,PRV_CPSS_DXCH_UNIT_TXQ_PSI_E), NULL,                                              \
        0x0000020c, 0x00000210,                                                                     \
        prvCpssDrvHwPpPortGroupIsrRead,                                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_PSI_REGS_PSI_INTERRUPT_CAUSE_SCHEDULER_INTERRUPT_E,                           \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_PSI_REGS_PSI_INTERRUPT_CAUSE_PDQ_ACCESS_MAP_ERROR_E,                          \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define tcam_SUB_TREE_MAC(_tile,bit)                                                                      \
    /* TCAM Interrupt Summary */                                                                    \
    {bit, GT_FALSE, UNIT_IN_TILE(_tile,PRV_CPSS_DXCH_UNIT_TCAM_E), NULL,                                                \
        0x00701004, 0x00701000,                                                                     \
        prvCpssDrvHwPpPortGroupIsrRead,                                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_TCAM_TCAM_INTERRUPTS_SUMMARY_CAUSE_TCAM_LOGIC_INTERRUPT_SUMMARY_E,         \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_TCAM_TCAM_INTERRUPTS_SUMMARY_CAUSE_TCAM_LOGIC_INTERRUPT_SUMMARY_E,         \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 1, NULL, NULL},                                     \
        /* TCAM Interrupt */                                                                        \
        {1, GT_FALSE, UNIT_IN_TILE(_tile,PRV_CPSS_DXCH_UNIT_TCAM_E), NULL,                                              \
            0x0070100C, 0x00701008,                                                                 \
            prvCpssDrvHwPpPortGroupIsrRead,                                                         \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                        \
            INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_TCAM_TCAM_INTERRUPT_CAUSE_CPU_ADDRESS_OUT_OF_RANGE_E,                  \
            INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_TCAM_TCAM_INTERRUPT_CAUSE_TABLE_OVERLAPPING_E,                         \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define   eft_NODE_MAC(_tile,bit)                                                                         \
    /* eft  */                                                                                      \
    {bit, GT_FALSE, UNIT_IN_TILE(_tile,PRV_CPSS_DXCH_UNIT_EGF_EFT_E), NULL,                                             \
        0x000010a0, 0x000010b0,                                                                     \
        prvCpssDrvHwPpPortGroupIsrRead,                                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_EGF_EFT_INTERRUPTS_CAUSE_EGRESS_WRONG_ADDR_INT_E,                             \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_EGF_EFT_INTERRUPTS_CAUSE_INC_BUS_IS_TOO_SMALL_INT_E,                          \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define   eoam_NODE_MAC(_tile,bit)                                                                        \
    /* egress OAM  */                                                                               \
    {bit, GT_FALSE, UNIT_IN_TILE(_tile,PRV_CPSS_DXCH_UNIT_EOAM_E), NULL,                                                \
        0x000000f0, 0x000000f4,                                                                     \
        prvCpssDrvHwPpPortGroupIsrRead,                                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_EOAM_OAM_UNIT_INTERRUPT_CAUSE_KEEP_ALIVE_AGING_E,                             \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_EOAM_OAM_UNIT_INTERRUPT_CAUSE_TX_PERIOD_E,                             \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

#define   epcl_NODE_MAC(_tile,bit)                                                                        \
    /* EPCL  */                                                                                 \
    {bit, GT_FALSE, UNIT_IN_TILE(_tile,PRV_CPSS_DXCH_UNIT_EPCL_E), NULL,                                            \
        0x00000010, 0x00000014,                                                                 \
        prvCpssDrvHwPpPortGroupIsrRead,                                                         \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                        \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_EPCL_EPCL_INTERRUPT_CAUSE_REGFILE_ADDRESS_ERROR_E,                        \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_EPCL_EPCL_INTERRUPT_CAUSE_TABLE_OVERLAP_ACCESS_ERROR_E,                   \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

#define   eplr_NODE_MAC(_tile,bit)                                                                        \
    /* eplr  */                                                                                 \
    {bit, GT_FALSE, UNIT_IN_TILE(_tile,PRV_CPSS_DXCH_UNIT_EPLR_E), NULL,                                            \
        0x00000200, 0x00000204,                                                                 \
        prvCpssDrvHwPpPortGroupIsrRead,                                                         \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                        \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_EPLR_POLICER_INTERRUPT_CAUSE_ILLEGAL_OVERLAPPED_ACCESS_E,                 \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_EPLR_POLICER_INTERRUPT_CAUSE_QOS_ATTRIBUTES_TABLE_WRONG_ADDRESS_INTERRUPT_E,    \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

#define   ermrk_NODE_MAC(_tile,bit)                                                                       \
    /* ERMRK  */                                                                                    \
    {bit, GT_FALSE, UNIT_IN_TILE(_tile,PRV_CPSS_DXCH_UNIT_ERMRK_E), NULL,                                           \
        0x00000004, 0x00000008,                                                                 \
        prvCpssDrvHwPpPortGroupIsrRead,                                                         \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                        \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_ERMRK_ERMRK_INTERRUPT_CAUSE_REGFILE_ADDRESS_ERROR_E,                      \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_ERMRK_ERMRK_INTERRUPT_CAUSE_TABLE_OVERLAP_ACCESS_ERROR_E,                 \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

#define   l2i_NODE_MAC(_tile,bit)                                                                         \
    /* L2 Bridge  */                                                                            \
    {bit, GT_FALSE, UNIT_IN_TILE(_tile,PRV_CPSS_DXCH_UNIT_L2I_E), NULL,                                            \
        0x00002300, 0x00002304,                                                                 \
        prvCpssDrvHwPpPortGroupIsrRead,                                                         \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                        \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_L2I_BRIDGE_INTERRUPT_CAUSE_ADDRESS_OUT_OF_RANGE_E,                     \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_L2I_BRIDGE_INTERRUPT_CAUSE_UPDATE_SECURITY_BREACH_REGISTER_INT_E,      \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

/* node for ingress SCT rate limiters interrupts leaf */
#define SCT_RATE_LIMITERS_NODE_FOR_16_PORTS_MAC(_tile,index)                                          \
    {(index+1), GT_FALSE, UNIT_IN_TILE(_tile,PRV_CPSS_DXCH_UNIT_EQ_E), NULL,                                        \
        (0x00010020 + ((index) * 0x10)), (0x00010024 + ((index) * 0x10)),                       \
        prvCpssDrvHwPpPortGroupIsrRead,                                                         \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                        \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_EQ_REG_1_PORT_0_PKT_DROPED_INT_E   + ((index) * 32),                   \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_EQ_REG_1_PORT_15_PKT_DROPED_INT_E  + ((index) * 32),                   \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define   eqSctRateLimiters_SUB_TREE_MAC(_tile,bit)                                                   \
    /* SCT Rate Limiters */                                                                     \
    {bit, GT_FALSE, UNIT_IN_TILE(_tile,PRV_CPSS_DXCH_UNIT_EQ_E), NULL,                                              \
        0x00010000, 0x00010004,                                                                 \
        prvCpssDrvHwPpPortGroupIsrRead,                                                         \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                        \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_EQ_CPU_CODE_RATE_LIMITERS_INTERRUPT_SUMMARY_CAUSE_REG_1_CPU_CODE_RATE_LIMITER_INT_E,         \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_EQ_CPU_CODE_RATE_LIMITERS_INTERRUPT_SUMMARY_CAUSE_REG_16_CPU_CODE_RATE_LIMITER_INT_E,        \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 16, NULL, NULL},                                \
        SCT_RATE_LIMITERS_NODE_FOR_16_PORTS_MAC(_tile,0),                                             \
        SCT_RATE_LIMITERS_NODE_FOR_16_PORTS_MAC(_tile,1),                                             \
        SCT_RATE_LIMITERS_NODE_FOR_16_PORTS_MAC(_tile,2),                                             \
        SCT_RATE_LIMITERS_NODE_FOR_16_PORTS_MAC(_tile,3),                                             \
        SCT_RATE_LIMITERS_NODE_FOR_16_PORTS_MAC(_tile,4),                                             \
        SCT_RATE_LIMITERS_NODE_FOR_16_PORTS_MAC(_tile,5),                                             \
        SCT_RATE_LIMITERS_NODE_FOR_16_PORTS_MAC(_tile,6),                                             \
        SCT_RATE_LIMITERS_NODE_FOR_16_PORTS_MAC(_tile,7),                                             \
        SCT_RATE_LIMITERS_NODE_FOR_16_PORTS_MAC(_tile,8),                                             \
        SCT_RATE_LIMITERS_NODE_FOR_16_PORTS_MAC(_tile,9),                                             \
        SCT_RATE_LIMITERS_NODE_FOR_16_PORTS_MAC(_tile,10),                                            \
        SCT_RATE_LIMITERS_NODE_FOR_16_PORTS_MAC(_tile,11),                                            \
        SCT_RATE_LIMITERS_NODE_FOR_16_PORTS_MAC(_tile,12),                                            \
        SCT_RATE_LIMITERS_NODE_FOR_16_PORTS_MAC(_tile,13),                                            \
        SCT_RATE_LIMITERS_NODE_FOR_16_PORTS_MAC(_tile,14),                                            \
        SCT_RATE_LIMITERS_NODE_FOR_16_PORTS_MAC(_tile,15)

#define   eq_SUB_TREE_MAC(_tile,bit)                                                                          \
    /* Pre-Egress Interrupt Summary Cause */                                                    \
    {bit, GT_FALSE, UNIT_IN_TILE(_tile,PRV_CPSS_DXCH_UNIT_EQ_E), NULL,                                              \
        0x00000058, 0x0000005C,                                                                 \
        prvCpssDrvHwPpPortGroupIsrRead,                                                         \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                        \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_EQ_PRE_EGRESS_INTERRUPT_SUMMARY_CAUSE_INGRESS_STC_INT_E,                  \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_EQ_PRE_EGRESS_INTERRUPT_SUMMARY_CAUSE_PORT_DLB_INTERRUPT_E,               \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 1, NULL, NULL},                                 \
        eqSctRateLimiters_SUB_TREE_MAC(_tile,2)

#define   ia_NODE_MAC(_tile,bit)                                                          \
    /* IA */                                                                        \
    {bit, GT_FALSE, UNIT_IN_TILE(_tile,PRV_CPSS_DXCH_UNIT_IA_E),  NULL,                                 \
        0x00000500, 0x00000504,                                                     \
        prvCpssDrvHwPpPortGroupIsrRead,                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                            \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_IA_IA_INTERRUPT_0_CAUSE_IA_RF_ERR_E,                          \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_IA_IA_INTERRUPT_0_CAUSE_IA_LS_FIFO_READ_ERROR_E,              \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define   ioam_NODE_MAC(_tile,bit)                                                        \
    /* ingress OAM  */                                                              \
    {bit, GT_FALSE, UNIT_IN_TILE(_tile,PRV_CPSS_DXCH_UNIT_IOAM_E),  NULL,                               \
        0x000000F0, 0x000000F4,                                                     \
        prvCpssDrvHwPpPortGroupIsrRead,                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                            \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_IOAM_OAM_UNIT_INTERRUPT_CAUSE_KEEP_ALIVE_AGING_E,             \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_IOAM_OAM_UNIT_INTERRUPT_CAUSE_TX_PERIOD_E,                   \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define   iplr0_NODE_MAC(_tile,bit)                                                       \
    /* iplr0 */                                                                     \
    {bit, GT_FALSE, UNIT_IN_TILE(_tile,PRV_CPSS_DXCH_UNIT_IPLR_E),  NULL,                               \
        0x00000200, 0x00000204,                                                     \
        prvCpssDrvHwPpPortGroupIsrRead,                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                            \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_IPLR0_POLICER_INTERRUPT_CAUSE_ILLEGAL_OVERLAPPED_ACCESS_E,    \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_IPLR0_POLICER_INTERRUPT_CAUSE_QOS_ATTRIBUTES_TABLE_WRONG_ADDRESS_INTERRUPT_E,            \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define   iplr1_NODE_MAC(_tile,bit)                                                       \
    /* iplr1 */                                                                     \
    {bit, GT_FALSE, UNIT_IN_TILE(_tile,PRV_CPSS_DXCH_UNIT_IPLR_1_E), NULL,                             \
        0x00000200, 0x00000204,                                                     \
        prvCpssDrvHwPpPortGroupIsrRead,                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                            \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_IPLR1_POLICER_INTERRUPT_CAUSE_ILLEGAL_OVERLAPPED_ACCESS_E,    \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_IPLR1_POLICER_INTERRUPT_CAUSE_QOS_ATTRIBUTES_TABLE_WRONG_ADDRESS_INTERRUPT_E,            \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define   router_NODE_MAC(_tile,bit)                                                          \
    /* router */                                                                    \
    {bit, GT_FALSE, UNIT_IN_TILE(_tile,PRV_CPSS_DXCH_UNIT_IPVX_E), NULL,                               \
        0x00000970, 0x00000974,                                                     \
        prvCpssDrvHwPpPortGroupIsrRead,                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                            \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_IPVX_ROUTER_INTERRUPT_CAUSE_I_PV_X_BAD_ADDR_E,                \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_IPVX_ROUTER_INTERRUPT_CAUSE_ILLEGAL_OVERLAP_ACCESS_E,         \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define   lpm_NODE_MAC(_tile,bit)                                                             \
    /* LPM */                                                                       \
    {bit, GT_FALSE, UNIT_IN_TILE(_tile,PRV_CPSS_DXCH_UNIT_LPM_E), NULL,                                \
        0x00F00120, 0x00F00130,         \
        prvCpssDrvHwPpPortGroupIsrRead,                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                            \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_LPM_LPM_GENERAL_INT_CAUSE_LPM_WRONG_ADDRESS_E,                \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_LPM_LPM_GENERAL_INT_CAUSE_TABLE_OVERLAPING_E,                 \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define   mll_NODE_MAC(_tile,bit)                                                             \
    /* MLL */                                                                       \
    {bit, GT_FALSE, UNIT_IN_TILE(_tile,PRV_CPSS_DXCH_UNIT_MLL_E), NULL,                                \
        0x00000030, 0x00000034,                                                     \
        prvCpssDrvHwPpPortGroupIsrRead,                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                            \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_MLL_MLL_INTERRUPT_CAUSE_REGISTER_INVALID_IP_MLL_ACCESS_INTERRUPT_E,         \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_MLL_MLL_INTERRUPT_CAUSE_REGISTER_ILLEGAL_TABLE_OVERLAP_E,     \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define   pcl_NODE_MAC(_tile,bit)                                                             \
    /* PCL */                                                                       \
    {bit, GT_FALSE, UNIT_IN_TILE(_tile,PRV_CPSS_DXCH_UNIT_PCL_E), NULL,                                \
        0x00000004, 0x00000008,                                                     \
        prvCpssDrvHwPpPortGroupIsrRead,                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                            \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_PCL_PCL_UNIT_INTERRUPT_CAUSE_MG_ADDR_OUTOF_RANGE_E,           \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_PCL_PCL_UNIT_INTERRUPT_CAUSE_PCL_ILLEGAL_TABLE_OVERLAP_E,     \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define   preq_NODE_MAC(_tile,bit)                                                        \
    /* PREQ */                                                                      \
    {bit, GT_FALSE, UNIT_IN_TILE(_tile,PRV_CPSS_DXCH_UNIT_PREQ_E), NULL,                               \
        0x00000600, 0x00000604,                                                     \
        prvCpssDrvHwPpPortGroupIsrRead,                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                            \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_PREQ_PREQ_INTERRUPT_CAUSE_PREQ_REGFILE_ADDRESS_ERROR_E,    \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_PREQ_PREQ_INTERRUPT_CAUSE_PREQ_TABLE_ACCESS_OVERLAP_ERROR_E, \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define   ppu_NODE_MAC(_tile,bit)                                                         \
    /* PPU */                                                                       \
    {bit, GT_FALSE, UNIT_IN_TILE(_tile,PRV_CPSS_DXCH_UNIT_PPU_E), NULL,                                 \
        0x00000004, 0x00000008,                                                     \
        prvCpssDrvHwPpPortGroupIsrRead,                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                            \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_PPU_PPU_INTERRUPT_CAUSE_ILLEGAL_ADDRESS_ACCESS_E,          \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_PPU_PPU_INTERRUPT_CAUSE_INT_INDEX_7_PPU_ACTION_INT_E,      \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define   qag_NODE_MAC(_tile,bit)                                                         \
    /* qag */                                                                       \
    {bit, GT_FALSE, UNIT_IN_TILE(_tile,PRV_CPSS_DXCH_UNIT_EGF_QAG_E), NULL,                            \
        0x000B0000, 0x000B0010,                                                     \
        prvCpssDrvHwPpPortGroupIsrRead,                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                            \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_EGF_QAG_INTERRUPTS_CAUSE_WRONG_ADDRESS_INTERRUPT_E,        \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_EGF_QAG_INTERRUPTS_CAUSE_TABLE_OVERLAPING_INTERRUPT_E,     \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define   sht_NODE_MAC(_tile,bit)                                                         \
    /* SHT */                                                                       \
    {bit, GT_FALSE, UNIT_IN_TILE(_tile,PRV_CPSS_DXCH_UNIT_EGF_SHT_E), NULL,                            \
        0x00100010, 0x00100020,                                                     \
        prvCpssDrvHwPpPortGroupIsrRead,                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                            \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_EGF_SHT_INTERRUPTS_CAUSE_WRONG_ADDRESS_INTERRUPT_E,           \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_EGF_SHT_INTERRUPTS_CAUSE_TABLE_OVERLAPPING_INTERRUPT_E,       \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}


#define PPG_OFFSET(ppg)            (0x80000 * (ppg))
#define PHA_PPN_OFFSET(ppg, ppn)   (PPG_OFFSET(ppg) + 0x4000 * (ppn))

#define PHA_PPN_NODE_MAC(_tile,bit, ppg, ppn)                                         \
    /* PPN Internal Error Cause */                                              \
    {bit, GT_FALSE, UNIT_IN_TILE(_tile,PRV_CPSS_DXCH_UNIT_PHA_E), NULL,                            \
       (0x00003040 + PHA_PPN_OFFSET(ppg, ppn)), (0x00003044 + PHA_PPN_OFFSET(ppg, ppn)),\
        prvCpssDrvHwPpPortGroupIsrRead,                                         \
        prvCpssDrvHwPpPortGroupIsrWrite,                                        \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_PPN_##ppn##_PPG_##ppg##_PPN_INTERNAL_ERROR_CAUSE_HOST_UNMAPPED_ACCESS_E, \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_PPN_##ppn##_PPG_##ppg##_PPN_INTERNAL_ERROR_CAUSE_DOORBELL_INTERRUPT_E,   \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define phaPpg_SUB_TREE_MAC(_tile,bit, ppg)                                           \
    /* PPG Internal Error Cause */                                              \
    {bit, GT_FALSE, UNIT_IN_TILE(_tile,PRV_CPSS_DXCH_UNIT_PHA_E), NULL,                            \
        (0x0007ff00 + PPG_OFFSET(ppg)), (0x0007ff04 + PPG_OFFSET(ppg)),         \
        prvCpssDrvHwPpPortGroupIsrRead,                                         \
        prvCpssDrvHwPpPortGroupIsrWrite,                                        \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_PPG_##ppg##_PPG_INTERRUPT_SUM_CAUSE_PPG_FUNCTIONAL_SUMMARY_INTERRUPT_E, \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_PPG_##ppg##_PPG_INTERRUPT_SUM_CAUSE_PPN_7_SUMMARY_INTERRUPT_E,      \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 9, NULL, NULL},                \
        /*  PPG Internal Error Cause */                                         \
        {1, GT_FALSE, UNIT_IN_TILE(_tile,PRV_CPSS_DXCH_UNIT_PHA_E), NULL,                          \
            (0x0007ff10 + PPG_OFFSET(ppg)), (0x0007ff14 + PPG_OFFSET(ppg)),     \
            prvCpssDrvHwPpPortGroupIsrRead,                                     \
            prvCpssDrvHwPpPortGroupIsrWrite,                                    \
            INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_PPG_##ppg##_PPG_INTERNAL_ERROR_CAUSE_PPG_BAD_ADDRESS_ERROR_E,          \
            INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_PPG_##ppg##_PPG_INTERNAL_ERROR_CAUSE_PPG_TABLE_ACCESS_OVERLAP_ERROR_E, \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},             \
        PHA_PPN_NODE_MAC(_tile,2, ppg, 0),                                            \
        PHA_PPN_NODE_MAC(_tile,3, ppg, 1),                                            \
        PHA_PPN_NODE_MAC(_tile,4, ppg, 2),                                            \
        PHA_PPN_NODE_MAC(_tile,5, ppg, 3),                                            \
        PHA_PPN_NODE_MAC(_tile,6, ppg, 4),                                            \
        PHA_PPN_NODE_MAC(_tile,7, ppg, 5),                                            \
        PHA_PPN_NODE_MAC(_tile,8, ppg, 6),                                            \
        PHA_PPN_NODE_MAC(_tile,9, ppg, 7)

#define   phaPpa_SUB_TREE_MAC(_tile,bit)                                              \
    /* PHA PPA */                                                               \
    {bit, GT_FALSE, UNIT_IN_TILE(_tile,PRV_CPSS_DXCH_UNIT_PHA_E), NULL,                            \
        0x007eff10, 0x007eff14,                                                 \
        prvCpssDrvHwPpPortGroupIsrRead,                                         \
        prvCpssDrvHwPpPortGroupIsrWrite,                                        \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_PPA_PPA_INTERRUPT_SUM_CAUSE_PPA_INTERNAL_FUNCTIONAL_SUMMARY_INTERRUPT_E,    \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_PPA_PPA_INTERRUPT_SUM_CAUSE_PPG_1_SUMMARY_INTERRUPT_E,    \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 3, NULL, NULL},                 \
        /* PPA Internal Error Cause */                                          \
        {1, GT_FALSE, UNIT_IN_TILE(_tile,PRV_CPSS_DXCH_UNIT_PHA_E), NULL,                          \
            0x007eff20, 0x007eff24,                                             \
            prvCpssDrvHwPpPortGroupIsrRead,                                     \
            prvCpssDrvHwPpPortGroupIsrWrite,                                    \
            INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_PPA_PPA_INTERNAL_ERROR_CAUSE_PPA_BAD_ADDRESS_ERRROR_E,\
            INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_PPA_PPA_INTERNAL_ERROR_CAUSE_PPA_BAD_ADDRESS_ERRROR_E,\
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},             \
        phaPpg_SUB_TREE_MAC(_tile,2, 0),                                              \
        phaPpg_SUB_TREE_MAC(_tile,3, 1)


#define   phaInternal_NODE_MAC(_tile,bit)                                             \
    /* PHA Internal Error Cause */                                              \
    {bit, GT_FALSE, UNIT_IN_TILE(_tile,PRV_CPSS_DXCH_UNIT_PHA_E), NULL,                            \
        0x007fff80, 0x007fff84,                                                 \
        prvCpssDrvHwPpPortGroupIsrRead,                                         \
        prvCpssDrvHwPpPortGroupIsrWrite,                                        \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_PHA_PHA_INTERNAL_ERROR_CAUSE_PHA_BAD_ADDRESS_ERROR_E,     \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_PHA_PHA_INTERNAL_ERROR_CAUSE_PPA_CLOCK_DOWN_VIOLATION_E,  \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define  ha_NODE_MAC(_tile,bit)                                                           \
        /* HA Rate Limiters */                                                      \
        {bit, GT_FALSE, UNIT_IN_TILE(_tile,PRV_CPSS_DXCH_UNIT_HA_E), NULL,                              \
            0x00000300, 0x00000304,                                                 \
            prvCpssDrvHwPpPortGroupIsrRead,                                         \
            prvCpssDrvHwPpPortGroupIsrWrite,                                        \
            INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_HA_HA_INTERRUPT_CAUSE_TABLE_OVERLAP_ACCESS_ERROR_E,    \
            INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_HA_HA_INTERRUPT_CAUSE_OVERSIZE_TUNNEL_HEADER_E,        \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define   pha_SUB_TREE_MAC(_tile,bit)                                                                     \
    /* PHA */                                                                                       \
    {bit, GT_FALSE, UNIT_IN_TILE(_tile,PRV_CPSS_DXCH_UNIT_PHA_E), NULL,                                                \
        0x007fff88, 0x007fff8c,                                                                     \
        prvCpssDrvHwPpPortGroupIsrRead,                                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_PHA_PHA_INTERRUPT_SUM_CAUSE_PHA_INTERNAL_FUNCTIONAL_SUMMARY_INTERRUPT_E,      \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_PHA_PHA_INTERRUPT_SUM_CAUSE_PPA_SUMMARY_INTERRUPT_E,                          \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 2, NULL, NULL},                                     \
        phaInternal_NODE_MAC(_tile,1),                                                                    \
        phaPpa_SUB_TREE_MAC(_tile,2)


#define   tti_NODE_MAC(_tile,bit)                                                                         \
    /* TTI */                                                                                       \
    {bit, GT_FALSE, UNIT_IN_TILE(_tile,PRV_CPSS_DXCH_UNIT_TTI_E), NULL,                                                \
        0x00000004, 0x00000008,                                                                     \
        prvCpssDrvHwPpPortGroupIsrRead,                                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_TTI_TTI_ENGINE_INTERRUPT_CAUSE_CPU_ADDRESS_OUT_OF_RANGE_E,                    \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_TTI_TTI_ENGINE_INTERRUPT_CAUSE_TABLE_OVERLAP_ERROR_E,                         \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define   txDma_NODE_MAC(_tile,bit, _index)                                                               \
    /* TXD */                                                                                       \
    {bit, GT_FALSE, UNIT_IN_TILE(_tile,PRV_CPSS_DXCH_UNIT_TXDMA##_index##_E), NULL,                                    \
        0x00007000, 0x00007004,                                                                     \
        prvCpssDrvHwPpPortGroupIsrRead,                                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_TXD_INSTANCE_##_index##_TXD_INTERRUPT_CAUSE_TXD_RF_ERR_E,                  \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_TXD_INSTANCE_##_index##_TXD_INTERRUPT_CAUSE_SDQ_BYTES_UNDERFLOW_E,         \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define   txFifo_NODE_MAC(_tile,bit, _index)                                                              \
    /* TXF */                                                                                       \
    {bit, GT_FALSE, UNIT_IN_TILE(_tile,PRV_CPSS_DXCH_UNIT_TX_FIFO##_index##_E), NULL,                                  \
        0x00004000, 0x00004004,                                                                     \
        prvCpssDrvHwPpPortGroupIsrRead,                                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_TXF_INSTANCE_##_index##_TXF_INTERRUPT_CAUSE_TX_FIFO_RF_ERR_E,              \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_TXF_INSTANCE_##_index##_TXF_INTERRUPT_CAUSE_DESCRIPTOR_RD_SER_ERROR_E,     \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define   rxDma_int0_SUB_TREE_MAC(_tile,bit, _index)                                                            \
        /* RXD INT0 */                                                                                    \
        {bit, GT_FALSE, UNIT_IN_TILE(_tile,PRV_CPSS_DXCH_UNIT_RXDMA##_index##_E), NULL,                                       \
            0x00001C80, 0x00001C84,                                                                       \
            prvCpssDrvHwPpPortGroupIsrRead,                                                               \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                              \
            INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_RX_INSTANCE_##_index##_RX_DMA_INTERRUPT_0_CAUSE_RX_DMA_RF_ERR_E,             \
            INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_RX_INSTANCE_##_index##_RX_DMA_INTERRUPT_0_CAUSE_HIGH_PRIO_DESC_DROP_E,       \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define   rxDma_int1_SUB_TREE_MAC(_tile,bit, _index)                                                               \
        /* RXD INT1 */                                                                                       \
        {bit, GT_FALSE, UNIT_IN_TILE(_tile,PRV_CPSS_DXCH_UNIT_RXDMA##_index##_E), NULL,                                          \
            0x00001C88, 0x00001C8C,                                                                          \
            prvCpssDrvHwPpPortGroupIsrRead,                                                                  \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                                 \
            INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_RX_INSTANCE_##_index##_RX_DMA_INTERRUPT_1_CAUSE_DESC_ENQUEUE_FIFO_FULL_INT_E,   \
            INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_RX_INSTANCE_##_index##_RX_DMA_INTERRUPT_1_CAUSE_DESC_ENQUEUE_FIFO_FULL_INT_E,   \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define   rxDma_int2_SUB_TREE_MAC(_tile,bit, _index)                                                                     \
        /* RXD INT2 */                                                                                             \
        {bit, GT_FALSE, UNIT_IN_TILE(_tile,PRV_CPSS_DXCH_UNIT_RXDMA##_index##_E), NULL,                                                \
            0x00001C90, 0x00001C94,                                                                                \
            prvCpssDrvHwPpPortGroupIsrRead,                                                                        \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                                       \
            INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_RX_INSTANCE_##_index##_RX_DMA_INTERRUPT_2_CAUSE_PB_TAIL_ID_MEM_SER_INT_E,             \
            INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_RX_INSTANCE_##_index##_RX_DMA_INTERRUPT_2_CAUSE_RX_LOCAL_IDS_FIFO_MEM_RD_SER_INT_E,   \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define   rxDma_NODE_MAC(_tile,bit, _index)                                                                        \
    /* RXD */                                                                                                \
    {bit, GT_FALSE, UNIT_IN_TILE(_tile,PRV_CPSS_DXCH_UNIT_RXDMA##_index##_E), NULL,                                             \
        0x00001CA0, 0x00001CA4,                                                                              \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                      \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                     \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_RX_INSTANCE_##_index##_RX_DMA_INTERRUPT_SUMMARY_CAUSE_RX_DMA_INTERRUPT_0_INT_E,     \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_RX_INSTANCE_##_index##_RX_DMA_INTERRUPT_SUMMARY_CAUSE_RX_DMA_INTERRUPT_2_INT_E,     \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 3, NULL, NULL},                                              \
        rxDma_int0_SUB_TREE_MAC(_tile,1, _index),                                                                  \
        rxDma_int1_SUB_TREE_MAC(_tile,2, _index),                                                                  \
        rxDma_int2_SUB_TREE_MAC(_tile,3, _index)


#define smb_SUB_TREE_MAC(_tile,bit, _index)                                                                                   \
    /* SMB-MC */                                                                                                        \
    {bit, GT_FALSE, UNIT_IN_TILE(_tile,PRV_CPSS_DXCH_UNIT_PB_SHARED_MEMO_BUF_##_index##_0_E), NULL,                                         \
        0x00000150, 0x00000154,                                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                                \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_SMB_MC_INSTANCE_##_index##_INTERRUPT_SUM_CAUSE_INTERRUPT_SUM_MISC_E,                           \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_SMB_MC_INSTANCE_##_index##_INTERRUPT_SUM_CAUSE_INTERRUPT_SUM_FIFO_E,                           \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 6,NULL, NULL},                                                          \
        /* MiscInterruptSum  */                                                                                         \
        {1, GT_FALSE, UNIT_IN_TILE(_tile,PRV_CPSS_DXCH_UNIT_PB_SHARED_MEMO_BUF_##_index##_0_E), NULL,                                       \
            0x00000100, 0x00000104,                                                                                     \
            prvCpssDrvHwPpPortGroupIsrRead,                                                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                                            \
            INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_SMB_MC_INSTANCE_##_index##_INTERRUPT_MISC_CAUSE_INTERRUPT_RPW_NO_TRUST_E,                  \
            INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_SMB_MC_INSTANCE_##_index##_INTERRUPT_MISC_CAUSE_INTERRUPT_ALIGN_WR_CMDS_FIFOS_CONGESTION_E,      \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},                                                      \
        /* AgeInterruptSum  */                                                                                          \
        {2, GT_FALSE, UNIT_IN_TILE(_tile,PRV_CPSS_DXCH_UNIT_PB_SHARED_MEMO_BUF_##_index##_0_E), NULL,                                       \
            0x00000110, 0x00000114,                                                                                     \
            prvCpssDrvHwPpPortGroupIsrRead,                                                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                                            \
            INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_SMB_MC_INSTANCE_##_index##_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_0_INTERRUPT_AGE_ERR_E,       \
            INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_SMB_MC_INSTANCE_##_index##_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_15_INTERRUPT_AGE_ERR_E,      \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},                                                      \
        /* RbwInterruptSum  */                                                                                          \
        {3, GT_FALSE, UNIT_IN_TILE(_tile,PRV_CPSS_DXCH_UNIT_PB_SHARED_MEMO_BUF_##_index##_0_E), NULL,                                       \
            0x00000120, 0x00000124,                                                                                     \
            prvCpssDrvHwPpPortGroupIsrRead,                                                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                                            \
            INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_SMB_MC_INSTANCE_##_index##_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_0_INTERRUPT_RBW_ERR_E,       \
            INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_SMB_MC_INSTANCE_##_index##_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_15_INTERRUPT_RBW_ERR_E,      \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},                                                      \
        /* MemoriesInterruptSum  */                                                                                     \
        {4, GT_FALSE, UNIT_IN_TILE(_tile,PRV_CPSS_DXCH_UNIT_PB_SHARED_MEMO_BUF_##_index##_0_E), NULL,                                       \
            0x00000130, 0x00000134,                                                                                     \
            prvCpssDrvHwPpPortGroupIsrRead,                                                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                                            \
            INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_SMB_MC_INSTANCE_##_index##_INTERRUPT_MEMORIES_CAUSE_LOGICAL_MEMORY_0_PAYLOAD_SER_ERROR_E,  \
            INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_SMB_MC_INSTANCE_##_index##_INTERRUPT_MEMORIES_CAUSE_LOGICAL_MEMORY_15_PAYLOAD_SER_ERROR_E, \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},                                                      \
        /* Memories2InterruptSum  */                                                                                    \
        {5, GT_FALSE, UNIT_IN_TILE(_tile,PRV_CPSS_DXCH_UNIT_PB_SHARED_MEMO_BUF_##_index##_0_E), NULL,                                       \
            0x00000140, 0x00000144,                                                                                     \
            prvCpssDrvHwPpPortGroupIsrRead,                                                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                                            \
            INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_SMB_MC_INSTANCE_##_index##_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_0_REFCNT_SER_ERROR_E,  \
            INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_SMB_MC_INSTANCE_##_index##_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_15_REFCNT_SER_ERROR_E,    \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},                                                      \
        /* FifoInterruptSum  */                                                                                         \
        {6, GT_FALSE, UNIT_IN_TILE(_tile,PRV_CPSS_DXCH_UNIT_PB_SHARED_MEMO_BUF_##_index##_0_E), NULL,                                       \
            0x00000118, 0x0000011C,                                                                                     \
            prvCpssDrvHwPpPortGroupIsrRead,                                                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                                            \
            INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_SMB_MC_INSTANCE_##_index##_INTERRUPT_FIFO_CAUSE_LOGICAL_MEMORY_0_INTERRUPT_FIFO_ERR_E,     \
            INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_SMB_MC_INSTANCE_##_index##_INTERRUPT_FIFO_CAUSE_LOGICAL_MEMORY_15_INTERRUPT_FIFO_ERR_E,    \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}


#define smb_write_arb_SUB_TREE_MAC(_tile,bit, _unit)                                                                          \
    {bit, GT_FALSE, UNIT_IN_TILE(_tile,PRV_CPSS_DXCH_UNIT_PB_SMB_WRITE_ARBITER_##_unit##_E), NULL,                                          \
        0x00000110, 0x0000114,                                                                                          \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                                \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_SMB_WRITE_ARBITER_INSTANCE_##_unit##_INTERRUPT_SUM_CAUSE_INTERRUPT_SUM_MISC_E,                 \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_SMB_WRITE_ARBITER_INSTANCE_##_unit##_INTERRUPT_SUM_CAUSE_INTERRUPT_SUM_MISC_E,                 \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}


#define   erep_NODE_MAC(_tile,bit)                                                                            \
    /* EREP Rate Limiters */                                                                            \
    {bit, GT_FALSE, UNIT_IN_TILE(_tile,PRV_CPSS_DXCH_UNIT_EREP_E), NULL,                                                   \
        0x00003000, 0x00003004,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_EREP_EREP_INTERRUPTS_CAUSE_EREP_CPU_ADDRESS_OUT_OF_RANGE_E,                       \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_EREP_EREP_INTERRUPTS_CAUSE_MIRROR_REPLICATION_NOT_PERFORMED_E,                    \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define   cnc_SUB_TREE_MAC(_tile,bit,_index)                                                                         \
    /* CNC  */                                                                                          \
    {bit, GT_FALSE, UNIT_IN_TILE(_tile,PRV_CPSS_DXCH_UNIT_CNC_0_E+_index),  NULL,                                                  \
        0x00000100, 0x00000104,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_CNC_CNC_INTERRUPT_SUMMARY_CAUSE_REGISTER_WRAPAROUND_FUNC_INTERRUPT_SUM_E,         \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_CNC_CNC_INTERRUPT_SUMMARY_CAUSE_REGISTER_MISC_FUNC_INTERRUPT_SUM_E,               \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 3,NULL, NULL},                                          \
                                                                                                        \
        /* WraparoundFuncInterruptSum  */                                                               \
        {1, GT_FALSE, UNIT_IN_TILE(_tile,PRV_CPSS_DXCH_UNIT_CNC_0_E+_index),  NULL,                                     \
            0x00000190, 0x000001a4,                                                                     \
            prvCpssDrvHwPpPortGroupIsrRead,                                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
            INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_CNC_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_BLOCK_0_WRAPAROUND_E,   \
            INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_CNC_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_BLOCK_15_WRAPAROUND_E,  \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},                                      \
        /* RateLimitFuncInterruptSum  */                                                                \
        {2, GT_FALSE, UNIT_IN_TILE(_tile,PRV_CPSS_DXCH_UNIT_CNC_0_E+_index),  NULL,                                     \
            0x000001b8, 0x000001cc,                                                                     \
            prvCpssDrvHwPpPortGroupIsrRead,                                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
            INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_CNC_RATE_LIMIT_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_BLOCK_NUMBER_0_RATE_LIMIT_FIFO_DROP_E,  \
            INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_CNC_RATE_LIMIT_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_BLOCK_NUMBER_15_RATE_LIMIT_FIFO_DROP_E, \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},                                      \
        /* MiscFuncInterruptSum  */                                                                     \
        {3, GT_FALSE, UNIT_IN_TILE(_tile,PRV_CPSS_DXCH_UNIT_CNC_0_E+_index),  NULL,                                     \
            0x000001e0, 0x000001e4,                                                                     \
            prvCpssDrvHwPpPortGroupIsrRead,                                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
            INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_CNC_MISC_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_DUMP_FINISHED_E,           \
            INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_CNC_MISC_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_CNC_TABLE_OVERLAPPING_E,   \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}


#define pds_SUB_TREE_MAC(_tile,bit, _pds)                                                                                 \
    /* PDS Interrupt Summary */                                                                                     \
    {bit, GT_FALSE, UNIT_IN_TILE(_tile,PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_PDS##_pds##_E), NULL,                                               \
        0x00042000, 0x00042004,                                                                                     \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                            \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_PDS_UNIT_##_pds##_PDS_INTERRUPT_SUMMARY_CAUSE_PDS_FUNCTIONAL_INT_SUM_E,                    \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_PDS_UNIT_##_pds##_PDS_INTERRUPT_SUMMARY_CAUSE_PDS_DEBUG_INT_SUM_E,                         \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 2, NULL, NULL},                                                     \
        /* PDS Functional Interrupt Summary */                                                                      \
        {1, GT_FALSE, UNIT_IN_TILE(_tile,PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_PDS##_pds##_E), NULL,                                             \
            0x00042008, 0x0004200C,                                                                                 \
            prvCpssDrvHwPpPortGroupIsrRead,                                                                         \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                                        \
            INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_PDS_UNIT_##_pds##_PDS_INTERRUPT_FUNCTIONAL_CAUSE_PDS_BAD_ADDRESS_INT_E,                \
            INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_PDS_UNIT_##_pds##_PDS_INTERRUPT_FUNCTIONAL_CAUSE_PB_ECC_DOUBLE_ERROR_INT_E,            \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},                                                 \
        /* PDS Debug Interrupt Summary */                                                                           \
        {2, GT_FALSE, UNIT_IN_TILE(_tile,PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_PDS##_pds##_E), NULL,                                             \
            0x00042010, 0x00042014,                                                                                 \
            prvCpssDrvHwPpPortGroupIsrRead,                                                                         \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                                        \
            INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_PDS_UNIT_##_pds##_PDS_INTERRUPT_DEBUG_CAUSE_PB_WRITE_REPLY_FIFO_FULL_E, \
            INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_PDS_UNIT_##_pds##_PDS_INTERRUPT_DEBUG_CAUSE_PB_RD_NOT_PERFORMED_INT_E,  \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define qfc_FUNC_INT_NODE_MAC(_tile,bit, _unit)                                                        \
    /* QFC Interrupt Summary */                                                         \
    {bit, GT_FALSE, UNIT_IN_TILE(_tile,PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_QFC##_unit##_E), NULL,                   \
        0x00000200, 0x00000204,                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_QFC_UNIT_##_unit##_QFC_INTERRUPT_FUNCTIONAL_CAUSE_UBURST_EVENT_INT_E, \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_QFC_UNIT_##_unit##_QFC_INTERRUPT_FUNCTIONAL_CAUSE_GLOBAL_TC_AVAILABLE_BUFFERS_LESS_THAN_USED_E,      \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define qfc_DBG_INT_NODE_MAC(_tile,bit, _unit)                                                                            \
    /* QFC Debug Interrupt Summary */                                                                               \
    {bit, GT_FALSE, UNIT_IN_TILE(_tile,PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_QFC##_unit##_E), NULL,                                              \
        0x00000208, 0x0000020C,                                                                                     \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                            \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_QFC_UNIT_##_unit##_QFC_INTERRUPT_DEBUG_CAUSE_PFC_MESSAGES_DROP_COUNTER_WRAPAROUND_INT_E,   \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_QFC_UNIT_##_unit##_QFC_INTERRUPT_DEBUG_CAUSE_AGGREGATION_BUFFER_COUNT_OVERFLOW_E,          \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define QFC_HR_INT_OFFSET(_unit, _regIndex)                                                                         \
        (((_unit) * 9 * 32) + ((_regIndex) * 32))

#define qfc_HR_INT_NODE_MAC(_tile,bit, _unit, _regIndex)                                                                  \
    /* QFC HR Crossed Threshold Interrupt Summary */                                                                \
    {bit, GT_FALSE, UNIT_IN_TILE(_tile,PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_QFC##_unit##_E), NULL,                                              \
        (0x0000021C+(_regIndex*0x8)), (0x00000220+(_regIndex*0x8)),                                                 \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                            \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_QFC_UNIT_0_REGISTER_INDEX_0_PORT_0_TC_0_CROSSED_HR_THRESHOLD_E + QFC_HR_INT_OFFSET(_unit, _regIndex),   \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_QFC_UNIT_0_REGISTER_INDEX_0_PORT_2_TC_7_CROSSED_HR_THRESHOLD_E + QFC_HR_INT_OFFSET(_unit, _regIndex),   \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define qfc_SUB_TREE_MAC(_tile,bit, _unit)                                                    \
    /* QFC Interrupt Summary */                                                         \
    {bit, GT_FALSE, UNIT_IN_TILE(_tile,PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_QFC##_unit##_E), NULL,                  \
        0x00000210, 0x00000214,                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_QFC_UNIT_##_unit##_QFC_INTERRUPT_SUMMARY_CAUSE_QFC_FUNCTIONAL_INT_SUM_E,    \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_QFC_UNIT_##_unit##_QFC_INTERRUPT_SUMMARY_CAUSE_REGISTER_INDEX_8_INT_SUM_E,  \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 11, NULL, NULL},                        \
        qfc_FUNC_INT_NODE_MAC(_tile, 1, _unit),                                               \
        qfc_DBG_INT_NODE_MAC (_tile, 2, _unit),                                               \
        qfc_HR_INT_NODE_MAC  (_tile, 3, _unit, 0),                                            \
        qfc_HR_INT_NODE_MAC  (_tile, 4, _unit, 1),                                            \
        qfc_HR_INT_NODE_MAC  (_tile, 5, _unit, 2),                                            \
        qfc_HR_INT_NODE_MAC  (_tile, 6, _unit, 3),                                            \
        qfc_HR_INT_NODE_MAC  (_tile, 7, _unit, 4),                                            \
        qfc_HR_INT_NODE_MAC  (_tile, 8, _unit, 5),                                            \
        qfc_HR_INT_NODE_MAC  (_tile, 9, _unit, 6),                                            \
        qfc_HR_INT_NODE_MAC  (_tile,10, _unit, 7),                                            \
        qfc_HR_INT_NODE_MAC  (_tile,11, _unit, 8)

#define sdq_NODE_MAC(_tile,bit, _sdq)                                                                     \
    /* SDQ Interrupt Summary */                                                                     \
    {bit, GT_FALSE, UNIT_IN_TILE(_tile,PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_SDQ##_sdq##_E), NULL,                               \
        0x00000200, 0x00000204,                                                                     \
        prvCpssDrvHwPpPortGroupIsrRead,                                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_SDQ_INST_##_sdq##_SDQ_INTERRUPT_SUMMARY_CAUSE_SDQ_FUNCTIONAL_INTERRUPT_SUMMARY_E, \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_SDQ_INST_##_sdq##_SDQ_INTERRUPT_SUMMARY_CAUSE_SDQ_DEBUG_INTERRUPT_SUMMARY_E,      \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define tai_master_NODE_MAC(_tile,bit, _instance)                                                    \
    /* TAI */                                                                                  \
    {bit, GT_FALSE, UNIT_IN_TILE(_tile,PRV_CPSS_DXCH_UNIT_CPU_PORT_TAI##_instance##_E), NULL,                      \
        0x00000000, 0x00000004,                                                                     \
        prvCpssDrvHwPpPortGroupIsrRead,                                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_TAI_MASTER_##_instance##_TAI_INTERRUPT_CAUSE_GENERATION_INT_E,      \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_TAI_MASTER_##_instance##_TAI_INTERRUPT_CAUSE_SER_RX_LINK_FAULT_E,   \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define led_NODE_MAC(_tile,bit, _instance)                                                  \
    /*  LED */                                                                         \
    {bit, GT_FALSE, UNIT_IN_TILE(_tile,PRV_CPSS_DXCH_UNIT_LED_##_instance##_E), NULL,                          \
        0x00000200, 0x00000204,                                                            \
        prvCpssDrvHwPpPortGroupIsrRead,                                                    \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                   \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_LED_INSTANCE_##_instance##_LED_INTERRUPT_CAUSE_ILLEGAL_ADDRESS_ACCESS_E,             \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_LED_INSTANCE_##_instance##_LED_INTERRUPT_CAUSE_ILLEGAL_ADDRESS_ACCESS_E,             \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}


#define mac_pcs_NODE_MAC(_tile,bit, _instance, _port)                                                 \
    {bit, GT_FALSE, UNIT_IN_TILE(_tile,PRV_CPSS_DXCH_UNIT_MAC_400G_##_instance##_E), NULL,                          \
        PORT_MAC_INTERRUPT_CAUSE_MAC(_port),                                                    \
        PORT_MAC_INTERRUPT_MASK_MAC(_port),                                                     \
        prvCpssDrvHwPpPortGroupIsrRead,                                                         \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                        \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_MAC_INSTANCE_##_instance##_PORT_NUM_##_port##_LINK_OK_CHANGE_E,        \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_MAC_INSTANCE_##_instance##_PORT_NUM_##_port##_TSD_NON_ACCURATE_PTP_E,  \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define mac_pcs_SEG_NODE_MAC(_tile,bit, _instance, _port)                                              \
    {bit, GT_FALSE, UNIT_IN_TILE(_tile,PRV_CPSS_DXCH_UNIT_MAC_400G_##_instance##_E), NULL,                           \
        PORT_SEG_MAC_INTERRUPT_CAUSE_MAC(_port),                                                 \
        PORT_SEG_MAC_INTERRUPT_MASK_MAC(_port),                                                  \
        prvCpssDrvHwPpPortGroupIsrRead,                                                          \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                         \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_MAC_INSTANCE_##_instance##_SEG_PORT_NUM_##_port##_LINK_OK_CHANGE_E,             \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_MAC_INSTANCE_##_instance##_SEG_PORT_NUM_##_port##_FRAME_DROP_E,                 \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define mac_pcs_SUB_TREE_MAC(_tile,bit, _instance)                                                    \
    /*  400 MAC */                                                                                   \
    {bit, GT_FALSE, UNIT_IN_TILE(_tile,PRV_CPSS_DXCH_UNIT_MAC_400G_##_instance##_E), NULL,                               \
        0x0000004C, 0x00000050,                                                                      \
        prvCpssDrvHwPpPortGroupIsrRead,                                                              \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                             \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_MAC_INSTANCE_##_instance##_GLOBAL_INTERRUPT_SUMMARY_CAUSE_PORT_NUM_0_P_INT_SUM_E,         \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_MAC_INSTANCE_##_instance##_GLOBAL_INTERRUPT_SUMMARY_CAUSE_FEC_NCE_INT_SUM_E,  \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 10, NULL, NULL},                                     \
            mac_pcs_NODE_MAC(_tile,1, _instance,  0),                                                 \
            mac_pcs_NODE_MAC(_tile,2, _instance,  1),                                                 \
            mac_pcs_NODE_MAC(_tile,3, _instance,  2),                                                 \
            mac_pcs_NODE_MAC(_tile,4, _instance,  3),                                                 \
            mac_pcs_NODE_MAC(_tile,5, _instance,  4),                                                 \
            mac_pcs_NODE_MAC(_tile,6, _instance,  5),                                                 \
            mac_pcs_NODE_MAC(_tile,7, _instance,  6),                                                 \
            mac_pcs_NODE_MAC(_tile,8, _instance,  7),                                                 \
            mac_pcs_SEG_NODE_MAC (_tile,9, _instance,  0),                                            \
            mac_pcs_SEG_NODE_MAC (_tile,10, _instance,  1)

#define mac_pcs_GOP2_SUB_TREE_MAC(_tile,bit)                                                          \
    /* 400G MAC */                                                                              \
    {bit, GT_FALSE, UNIT_IN_TILE(_tile,PRV_CPSS_DXCH_UNIT_MAC_400G_2_E), NULL,                                      \
        0x0000004C, 0x00000050,                                                                 \
        prvCpssDrvHwPpPortGroupIsrRead,                                                         \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                        \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_MAC_INSTANCE_2_GLOBAL_INTERRUPT_SUMMARY_CAUSE_PORT_NUM_0_P_INT_SUM_E,  \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_MAC_INSTANCE_2_GLOBAL_INTERRUPT_SUMMARY_CAUSE_FEC_NCE_INT_SUM_E,       \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 5, NULL, NULL},                                 \
            mac_pcs_NODE_MAC(_tile,1, 2,  0),                                                         \
            mac_pcs_NODE_MAC(_tile,2, 2,  1),                                                         \
            mac_pcs_NODE_MAC(_tile,3, 2,  2),                                                         \
            mac_pcs_NODE_MAC(_tile,4, 2,  3),                                                         \
            mac_pcs_SEG_NODE_MAC (_tile,9, 2,  0)

#define mif_NODE_MAC(_tile,bit, _instance)                                                                \
    /*  MIF */                                                                                      \
    {bit, GT_FALSE, UNIT_IN_TILE(_tile,PRV_CPSS_DXCH_UNIT_MIF_400G_##_instance##_E), NULL,                              \
        0x00000120, 0x00000124,                                                                     \
        prvCpssDrvHwPpPortGroupIsrRead,                                                             \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_MIF_INSTACE_##_instance##_MIF_GLOBAL_INTERRUPT_SUMMARY_CAUSE_MIF_TYPE_8_TX_PROTOCOL_VIOLATION_INTERRUPT_E, \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_MIF_INSTACE_##_instance##_MIF_GLOBAL_INTERRUPT_SUMMARY_CAUSE_MIF_BAD_ACCESS_SUM_E,       \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

/* CTSU RX Channel - cause */
#define CTSU_CHANNEL_PCA_INTERRUPT_CAUSE_MAC(channel)                                                   \
        (0x3000 + ((channel) * 0x4))

/* CTSU RX Channel - mask */
#define CTSU_CHANNEL_PCA_INTERRUPT_MASK_MAC(channel)                        \
        (0x3400 + ((channel) * 0x4))

#define ctsu_PCA_GLOBAL_NODE_MAC(_tile,bit, _instance)                                                        \
    {bit, GT_FALSE, UNIT_IN_TILE(_tile,PRV_CPSS_DXCH_UNIT_PCA_CTSU_##_instance##_E), NULL,                                  \
        0x00000080, 0x00000084,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_CTSU_INSTANCE_##_instance##_CTSU_GLOBAL_INTERRUPT_CAUSE_ILLEGAL_ADDRESS_ACCESS_E,     \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_CTSU_INSTANCE_##_instance##_CTSU_GLOBAL_INTERRUPT_CAUSE_TIME_STAMPING_COMMAND_PARSING_ERROR_E, \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define ctsu_channel_group0_PCA_SUB_TREE_MAC(_tile,bit, _instance)                                            \
    {bit, GT_FALSE, UNIT_IN_TILE(_tile,PRV_CPSS_DXCH_UNIT_PCA_CTSU_##_instance##_E), NULL,                                  \
        0x00003800, 0x00003820,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_CTSU_INSTANCE_##_instance##_CH_GROUP_0_INTERRUPT_CAUSE_SUMMARY_CHANNEL_0_INTERRUPT_CAUSE_SUMMARY_E,  \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_CTSU_INSTANCE_##_instance##_CH_GROUP_0_INTERRUPT_CAUSE_SUMMARY_CHANNEL_15_INTERRUPT_CAUSE_SUMMARY_E, \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 16, NULL, NULL},                                        \
        GROUP_0_CHANNEL_CTSU_16_PCA_NODES_MAC(_tile,_instance)

#define ctsu_channel_group1_PCA_SUB_TREE_MAC(_tile,bit, _instance)                                            \
    {bit, GT_FALSE, UNIT_IN_TILE(_tile,PRV_CPSS_DXCH_UNIT_PCA_CTSU_##_instance##_E), NULL,                                  \
        0x00003804, 0x00003824,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_CTSU_INSTANCE_##_instance##_CH_GROUP_1_INTERRUPT_CAUSE_SUMMARY_CHANNEL_16_INTERRUPT_CAUSE_SUMMARY_E,  \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_CTSU_INSTANCE_##_instance##_CH_GROUP_1_INTERRUPT_CAUSE_SUMMARY_CHANNEL_31_INTERRUPT_CAUSE_SUMMARY_E, \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 16, NULL, NULL},                                        \
        GROUP_1_CHANNEL_CTSU_16_PCA_NODES_MAC(_tile,_instance)

#define ctsu_channel_PCA_NODE_MAC(_tile,bit, _instance, channel)                                              \
    {bit, GT_FALSE, UNIT_IN_TILE(_tile,PRV_CPSS_DXCH_UNIT_PCA_CTSU_##_instance##_E), NULL,                                  \
        CTSU_CHANNEL_PCA_INTERRUPT_CAUSE_MAC(channel),                                                  \
        CTSU_CHANNEL_PCA_INTERRUPT_MASK_MAC(channel),                                                   \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_CTSU_INSTANCE_##_instance##_CHANNEL_##channel##_CHANNEL_EGRESS_TIMESTAMP_QUEUE_0_NEW_ENTRY_INTERRUPT_E,     \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_CTSU_INSTANCE_##_instance##_CHANNEL_##channel##_CHANNEL_EGRESS_TIMESTAMP_QUEUE_1_OVERFLOW_INTERRUPT_E,      \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define GROUP_0_CHANNEL_CTSU_16_PCA_NODES_MAC(_tile,_instance)                                                \
        ctsu_channel_PCA_NODE_MAC(_tile, 1, _instance,  0),                                                   \
        ctsu_channel_PCA_NODE_MAC(_tile, 2, _instance,  1),                                                   \
        ctsu_channel_PCA_NODE_MAC(_tile, 3, _instance,  2),                                                   \
        ctsu_channel_PCA_NODE_MAC(_tile, 4, _instance,  3),                                                   \
        ctsu_channel_PCA_NODE_MAC(_tile, 5, _instance,  4),                                                   \
        ctsu_channel_PCA_NODE_MAC(_tile, 6, _instance,  5),                                                   \
        ctsu_channel_PCA_NODE_MAC(_tile, 7, _instance,  6),                                                   \
        ctsu_channel_PCA_NODE_MAC(_tile, 8, _instance,  7),                                                   \
        ctsu_channel_PCA_NODE_MAC(_tile, 9, _instance,  8),                                                   \
        ctsu_channel_PCA_NODE_MAC(_tile,10, _instance,  9),                                                   \
        ctsu_channel_PCA_NODE_MAC(_tile,11, _instance, 10),                                                   \
        ctsu_channel_PCA_NODE_MAC(_tile,12, _instance, 11),                                                   \
        ctsu_channel_PCA_NODE_MAC(_tile,13, _instance, 12),                                                   \
        ctsu_channel_PCA_NODE_MAC(_tile,14, _instance, 13),                                                   \
        ctsu_channel_PCA_NODE_MAC(_tile,15, _instance, 14),                                                   \
        ctsu_channel_PCA_NODE_MAC(_tile,16, _instance, 15)

#define GROUP_1_CHANNEL_CTSU_16_PCA_NODES_MAC(_tile,_instance)                                                \
        ctsu_channel_PCA_NODE_MAC(_tile, 1, _instance, 16),                                                   \
        ctsu_channel_PCA_NODE_MAC(_tile, 2, _instance, 17),                                                   \
        ctsu_channel_PCA_NODE_MAC(_tile, 3, _instance, 18),                                                   \
        ctsu_channel_PCA_NODE_MAC(_tile, 4, _instance, 19),                                                   \
        ctsu_channel_PCA_NODE_MAC(_tile, 5, _instance, 20),                                                   \
        ctsu_channel_PCA_NODE_MAC(_tile, 6, _instance, 21),                                                   \
        ctsu_channel_PCA_NODE_MAC(_tile, 7, _instance, 22),                                                   \
        ctsu_channel_PCA_NODE_MAC(_tile, 8, _instance, 23),                                                   \
        ctsu_channel_PCA_NODE_MAC(_tile, 9, _instance, 24),                                                   \
        ctsu_channel_PCA_NODE_MAC(_tile,10, _instance, 25),                                                   \
        ctsu_channel_PCA_NODE_MAC(_tile,11, _instance, 26),                                                   \
        ctsu_channel_PCA_NODE_MAC(_tile,12, _instance, 27),                                                   \
        ctsu_channel_PCA_NODE_MAC(_tile,13, _instance, 28),                                                   \
        ctsu_channel_PCA_NODE_MAC(_tile,14, _instance, 29),                                                   \
        ctsu_channel_PCA_NODE_MAC(_tile,15, _instance, 30),                                                   \
        ctsu_channel_PCA_NODE_MAC(_tile,16, _instance, 31)

#define ctsu_PCA_SUB_TREE_MAC(_tile,bit, _instance)                                                           \
    {bit, GT_FALSE, UNIT_IN_TILE(_tile,PRV_CPSS_DXCH_UNIT_PCA_CTSU_##_instance##_E), NULL,                                  \
        0x00000088, 0x0000008c,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_CTSU_INSTANCE_##_instance##_CTSU_INTERRUPT_CAUSE_SUMMARY_GLOBAL_INTERRUPT_CAUSE_SUMMARY_E, \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_CTSU_INSTANCE_##_instance##_CTSU_INTERRUPT_CAUSE_SUMMARY_CHANNEL_GROUP_1_INTERRUPT_CAUSE_SUMMARY_E, \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 3, NULL, NULL},                                         \
        ctsu_PCA_GLOBAL_NODE_MAC(_tile,1, _instance),                                                         \
        ctsu_channel_group0_PCA_SUB_TREE_MAC(_tile,2, _instance),                                             \
        ctsu_channel_group1_PCA_SUB_TREE_MAC(_tile,3, _instance)

/* CTSU RX Channel - cause */
#define CTSU_CHANNEL_MSEC_INTERRUPT_CAUSE_MAC(channel)                                                   \
        (0x3000 + ((channel) * 0x4))

/* CTSU RX Channel - mask */
#define CTSU_CHANNEL_MSEC_INTERRUPT_MASK_MAC(channel)                        \
        (0x3400 + ((channel) * 0x4))

#define ctsu_MSEC_GLOBAL_NODE_MAC(_tile,bit, _instance)                                                        \
    {bit, GT_FALSE, UNIT_IN_TILE(_tile,PRV_CPSS_DXCH_UNIT_PCA_CTSU_##_instance##_E), NULL,                                  \
        0x00000080, 0x00000084,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_MSEC_INST_##_instance##_CTSU_GLOBAL_INTERRUPT_CAUSE_ILLEGAL_ADDRESS_ACCESS_E,     \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_MSEC_INST_##_instance##_CTSU_GLOBAL_INTERRUPT_CAUSE_TIME_STAMPING_COMMAND_PARSING_ERROR_E, \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define ctsu_channel_group0_MSEC_SUB_TREE_MAC(_tile,bit, _instance)                                            \
    {bit, GT_FALSE, UNIT_IN_TILE(_tile,PRV_CPSS_DXCH_UNIT_PCA_CTSU_##_instance##_E), NULL,                                  \
        0x00003800, 0x00003820,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_MSEC_INST_##_instance##_CH_GROUP_0_INTERRUPT_CAUSE_SUMMARY_CHANNEL_0_INTERRUPT_CAUSE_SUMMARY_E,  \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_MSEC_INST_##_instance##_CH_GROUP_0_INTERRUPT_CAUSE_SUMMARY_CHANNEL_15_INTERRUPT_CAUSE_SUMMARY_E, \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 16, NULL, NULL},                                        \
        GROUP_0_CHANNEL_CTSU_16_MSEC_NODES_MAC(_instance)

#define ctsu_channel_group1_MSEC_SUB_TREE_MAC(_tile,bit, _instance)                                            \
    {bit, GT_FALSE, UNIT_IN_TILE(_tile,PRV_CPSS_DXCH_UNIT_PCA_CTSU_##_instance##_E), NULL,                                  \
        0x00003804, 0x00003824,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_MSEC_INST_##_instance##_CH_GROUP_1_INTERRUPT_CAUSE_SUMMARY_CHANNEL_16_INTERRUPT_CAUSE_SUMMARY_E,  \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_MSEC_INST_##_instance##_CH_GROUP_1_INTERRUPT_CAUSE_SUMMARY_CHANNEL_31_INTERRUPT_CAUSE_SUMMARY_E, \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 16, NULL, NULL},                                        \
        GROUP_1_CHANNEL_CTSU_16_MSEC_NODES_MAC(_instance)

#define ctsu_channel_MSEC_NODE_MAC(_tile,bit, _instance, channel)                                              \
    {bit, GT_FALSE, UNIT_IN_TILE(_tile,PRV_CPSS_DXCH_UNIT_PCA_CTSU_##_instance##_E), NULL,                                  \
        CTSU_CHANNEL_MSEC_INTERRUPT_CAUSE_MAC(channel),                                                  \
        CTSU_CHANNEL_MSEC_INTERRUPT_MASK_MAC(channel),                                                   \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_CTSU_INSTANCE__##_instance##_CHANNEL_##channel##_CHANNEL_EGRESS_TIMESTAMP_QUEUE_0_NEW_ENTRY_INTERRUPT_E,     \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_CTSU_INSTANCE__##_instance##_CHANNEL_##channel##_CHANNEL_EGRESS_TIMESTAMP_QUEUE_1_OVERFLOW_INTERRUPT_E,      \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define GROUP_0_CHANNEL_CTSU_16_MSEC_NODES_MAC(_tile,_instance)                                                \
        ctsu_channel_MSEC_NODE_MAC(_tile, 1, _instance,  0),                                                   \
        ctsu_channel_MSEC_NODE_MAC(_tile, 2, _instance,  1),                                                   \
        ctsu_channel_MSEC_NODE_MAC(_tile, 3, _instance,  2),                                                   \
        ctsu_channel_MSEC_NODE_MAC(_tile, 4, _instance,  3),                                                   \
        ctsu_channel_MSEC_NODE_MAC(_tile, 5, _instance,  4),                                                   \
        ctsu_channel_MSEC_NODE_MAC(_tile, 6, _instance,  5),                                                   \
        ctsu_channel_MSEC_NODE_MAC(_tile, 7, _instance,  6),                                                   \
        ctsu_channel_MSEC_NODE_MAC(_tile, 8, _instance,  7),                                                   \
        ctsu_channel_MSEC_NODE_MAC(_tile, 9, _instance,  8),                                                   \
        ctsu_channel_MSEC_NODE_MAC(_tile,10, _instance,  9),                                                   \
        ctsu_channel_MSEC_NODE_MAC(_tile,11, _instance, 10),                                                   \
        ctsu_channel_MSEC_NODE_MAC(_tile,12, _instance, 11),                                                   \
        ctsu_channel_MSEC_NODE_MAC(_tile,13, _instance, 12),                                                   \
        ctsu_channel_MSEC_NODE_MAC(_tile,14, _instance, 13),                                                   \
        ctsu_channel_MSEC_NODE_MAC(_tile,15, _instance, 14),                                                   \
        ctsu_channel_MSEC_NODE_MAC(_tile,16, _instance, 15)

#define GROUP_1_CHANNEL_CTSU_16_MSEC_NODES_MAC(_tile,_instance)                                                \
        ctsu_channel_MSEC_NODE_MAC(_tile, 1, _instance, 16),                                                   \
        ctsu_channel_MSEC_NODE_MAC(_tile, 2, _instance, 17),                                                   \
        ctsu_channel_MSEC_NODE_MAC(_tile, 3, _instance, 18),                                                   \
        ctsu_channel_MSEC_NODE_MAC(_tile, 4, _instance, 19),                                                   \
        ctsu_channel_MSEC_NODE_MAC(_tile, 5, _instance, 20),                                                   \
        ctsu_channel_MSEC_NODE_MAC(_tile, 6, _instance, 21),                                                   \
        ctsu_channel_MSEC_NODE_MAC(_tile, 7, _instance, 22),                                                   \
        ctsu_channel_MSEC_NODE_MAC(_tile, 8, _instance, 23),                                                   \
        ctsu_channel_MSEC_NODE_MAC(_tile, 9, _instance, 24),                                                   \
        ctsu_channel_MSEC_NODE_MAC(_tile,10, _instance, 25),                                                   \
        ctsu_channel_MSEC_NODE_MAC(_tile,11, _instance, 26),                                                   \
        ctsu_channel_MSEC_NODE_MAC(_tile,12, _instance, 27),                                                   \
        ctsu_channel_MSEC_NODE_MAC(_tile,13, _instance, 28),                                                   \
        ctsu_channel_MSEC_NODE_MAC(_tile,14, _instance, 29),                                                   \
        ctsu_channel_MSEC_NODE_MAC(_tile,15, _instance, 30),                                                   \
        ctsu_channel_MSEC_NODE_MAC(_tile,16, _instance, 31)

#define ctsu_MSEC_SUB_TREE_MAC(_tile,bit, _instance)                                                           \
    {bit, GT_FALSE, UNIT_IN_TILE(_tile,PRV_CPSS_DXCH_UNIT_PCA_CTSU_##_instance##_E), NULL,                                  \
        0x00000088, 0x0000008c,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_MSEC_INST_##_instance##_CTSU_INTERRUPT_CAUSE_SUMMARY_GLOBAL_INTERRUPT_CAUSE_SUMMARY_E, \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_MSEC_INST_##_instance##_CTSU_INTERRUPT_CAUSE_SUMMARY_CHANNEL_GROUP_1_INTERRUPT_CAUSE_SUMMARY_E, \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 3, NULL, NULL},                                         \
        ctsu_MSEC_GLOBAL_NODE_MAC(1, _instance),                                                         \
        ctsu_channel_group0_MSEC_SUB_TREE_MAC(2, _instance),                                             \
        ctsu_channel_group1_MSEC_SUB_TREE_MAC(3, _instance)

/* BRG TX Channel - cause */
#define BRG_TX_CHANNEL_INTERRUPT_CAUSE_MAC(_channel, _group)                                     \
        (0x1500 + ((_group) * 0x40) + ((_channel) * 0x4))

/* BRG TX Channel - mask */
#define BRG_TX_CHANNEL_INTERRUPT_MASK_MAC(_channel, _group)                                      \
        (0x1700 + ((_group) * 0x40) + ((_channel) * 0x4))


/* BRG RX Channel - cause */
#define BRG_RX_CHANNEL_INTERRUPT_CAUSE_MAC(_channel, _group)                                     \
        (0x200 + ((_group) * 0x80) + ((_channel) * 0x4))

/* BRG RX Channel - mask */
#define BRG_RX_CHANNEL_INTERRUPT_MASK_MAC(_channel, _group)                                      \
        (0x400 + ((_group) * 0x80) + ((_channel) * 0x4))

#define BRG_RX_CHANNEL_GROUP_0_MSEC_16_NODES_MAC(_tile,_instance)                                             \
        brg_rx_channel_MSEC_NODE_MAC(_tile, 1, _instance,  0, 0),                                                \
        brg_rx_channel_MSEC_NODE_MAC(_tile, 2, _instance,  1, 0),                                                \
        brg_rx_channel_MSEC_NODE_MAC(_tile, 3, _instance,  2, 0),                                                \
        brg_rx_channel_MSEC_NODE_MAC(_tile, 4, _instance,  3, 0),                                                \
        brg_rx_channel_MSEC_NODE_MAC(_tile, 5, _instance,  4, 0),                                                \
        brg_rx_channel_MSEC_NODE_MAC(_tile, 6, _instance,  5, 0),                                                \
        brg_rx_channel_MSEC_NODE_MAC(_tile, 7, _instance,  6, 0),                                                \
        brg_rx_channel_MSEC_NODE_MAC(_tile, 8, _instance,  7, 0),                                                \
        brg_rx_channel_MSEC_NODE_MAC(_tile, 9, _instance,  8, 0),                                                \
        brg_rx_channel_MSEC_NODE_MAC(_tile,10, _instance,  9, 0),                                                \
        brg_rx_channel_MSEC_NODE_MAC(_tile,11, _instance, 10, 0),                                                \
        brg_rx_channel_MSEC_NODE_MAC(_tile,12, _instance, 11, 0),                                                \
        brg_rx_channel_MSEC_NODE_MAC(_tile,13, _instance, 12, 0),                                                \
        brg_rx_channel_MSEC_NODE_MAC(_tile,14, _instance, 13, 0),                                                \
        brg_rx_channel_MSEC_NODE_MAC(_tile,15, _instance, 14, 0),                                                \
        brg_rx_channel_MSEC_NODE_MAC(_tile,16, _instance, 15, 0)

#define BRG_RX_CHANNEL_GROUP_1_MSEC_16_NODES_MAC(_tile,_instance)                                             \
        brg_rx_channel_MSEC_NODE_MAC(_tile, 1, _instance, 16, 1),                                                \
        brg_rx_channel_MSEC_NODE_MAC(_tile, 2, _instance, 17, 1),                                                \
        brg_rx_channel_MSEC_NODE_MAC(_tile, 3, _instance, 18, 1),                                                \
        brg_rx_channel_MSEC_NODE_MAC(_tile, 4, _instance, 19, 1),                                                \
        brg_rx_channel_MSEC_NODE_MAC(_tile, 5, _instance, 20, 1),                                                \
        brg_rx_channel_MSEC_NODE_MAC(_tile, 6, _instance, 21, 1),                                                \
        brg_rx_channel_MSEC_NODE_MAC(_tile, 7, _instance, 22, 1),                                                \
        brg_rx_channel_MSEC_NODE_MAC(_tile, 8, _instance, 23, 1),                                                \
        brg_rx_channel_MSEC_NODE_MAC(_tile, 9, _instance, 24, 1),                                                \
        brg_rx_channel_MSEC_NODE_MAC(_tile,10, _instance, 25, 1),                                                \
        brg_rx_channel_MSEC_NODE_MAC(_tile,11, _instance, 26, 1),                                                \
        brg_rx_channel_MSEC_NODE_MAC(_tile,12, _instance, 27, 1),                                                \
        brg_rx_channel_MSEC_NODE_MAC(_tile,13, _instance, 28, 1),                                                \
        brg_rx_channel_MSEC_NODE_MAC(_tile,14, _instance, 29, 1),                                                \
        brg_rx_channel_MSEC_NODE_MAC(_tile,15, _instance, 30, 1),                                                \
        brg_rx_channel_MSEC_NODE_MAC(_tile,16, _instance, 31, 1)

#define brg_rx_channel_MSEC_NODE_MAC(_tile,bit, _instance, _channel, _group)                                          \
    {bit, GT_FALSE, UNIT_IN_TILE(_tile,PRV_CPSS_DXCH_UNIT_PCA_BRG_##_instance##_E), NULL,                                   \
        BRG_RX_CHANNEL_INTERRUPT_CAUSE_MAC(_channel, _group),                                              \
        BRG_RX_CHANNEL_INTERRUPT_MASK_MAC(_channel, _group),                                               \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_MSEC_INST_##_instance##_CHANNEL_##_channel##_RX_SDB_PACKET_ON_DISABLED_CHANNEL_INTERRUPT_E,     \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_MSEC_INST_##_instance##_CHANNEL_##_channel##_RX_SDB_PACKET_ON_DISABLED_CHANNEL_INTERRUPT_E,     \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define brg_rx_global_MSEC_NODE_MAC(_tile,bit, _instance)                                                      \
    {bit, GT_FALSE, UNIT_IN_TILE(_tile,PRV_CPSS_DXCH_UNIT_PCA_BRG_##_instance##_E), NULL,                                   \
        0x00000750, 0x00000754,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_MSEC_INST_##_instance##_RX_GLOBAL_INTERRUPT_CAUSE_RX_ROTATED_FIFO_OVERRUN_INTERRUPT_E,\
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_MSEC_INST_##_instance##_RX_GLOBAL_INTERRUPT_CAUSE_RX_SDB_CHANNEL_ID_ERROR_INTERRUPT_E, \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define brg_rx_channel_group0_MSEC_SUB_TREE_MAC(_tile,bit, _instance)                                         \
    {bit, GT_FALSE, UNIT_IN_TILE(_tile,PRV_CPSS_DXCH_UNIT_PCA_BRG_##_instance##_E), NULL,                                   \
        0x00000600, 0x00000620,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_MSEC_INST_##_instance##_RX_CH_GROUP_0_INTERRUPT_CAUSE_SUMMARY_CHANNEL_0_INTERRUPT_CAUSE_SUMMARY_E,\
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_MSEC_INST_##_instance##_RX_CH_GROUP_0_INTERRUPT_CAUSE_SUMMARY_CHANNEL_15_INTERRUPT_CAUSE_SUMMARY_E, \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 16, NULL, NULL},                                        \
        BRG_RX_CHANNEL_GROUP_0_MSEC_16_NODES_MAC(_instance)

#define brg_rx_channel_group1_MSEC_SUB_TREE_MAC(_tile,bit, _instance)                                          \
    {bit, GT_FALSE, UNIT_IN_TILE(_tile,PRV_CPSS_DXCH_UNIT_PCA_BRG_##_instance##_E), NULL,                                   \
        0x00000604, 0x00000624,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_MSEC_INST_##_instance##_RX_CH_GROUP_1_INTERRUPT_CAUSE_SUMMARY_CHANNEL_16_INTERRUPT_CAUSE_SUMMARY_E,\
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_MSEC_INST_##_instance##_RX_CH_GROUP_1_INTERRUPT_CAUSE_SUMMARY_CHANNEL_31_INTERRUPT_CAUSE_SUMMARY_E, \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 16, NULL, NULL},                                        \
        BRG_RX_CHANNEL_GROUP_1_MSEC_16_NODES_MAC(_instance)

#define BRG_TX_CHANNEL_GROUP_0_MSEC_16_NODES_MAC(_tile,_instance)                                             \
        brg_tx_channel_MSEC_NODE_MAC(_tile, 1, _instance,  0, 0),                                                \
        brg_tx_channel_MSEC_NODE_MAC(_tile, 2, _instance,  1, 0),                                                \
        brg_tx_channel_MSEC_NODE_MAC(_tile, 3, _instance,  2, 0),                                                \
        brg_tx_channel_MSEC_NODE_MAC(_tile, 4, _instance,  3, 0),                                                \
        brg_tx_channel_MSEC_NODE_MAC(_tile, 5, _instance,  4, 0),                                                \
        brg_tx_channel_MSEC_NODE_MAC(_tile, 6, _instance,  5, 0),                                                \
        brg_tx_channel_MSEC_NODE_MAC(_tile, 7, _instance,  6, 0),                                                \
        brg_tx_channel_MSEC_NODE_MAC(_tile, 8, _instance,  7, 0),                                                \
        brg_tx_channel_MSEC_NODE_MAC(_tile, 9, _instance,  8, 0),                                                \
        brg_tx_channel_MSEC_NODE_MAC(_tile,10, _instance,  9, 0),                                                \
        brg_tx_channel_MSEC_NODE_MAC(_tile,11, _instance, 10, 0),                                                \
        brg_tx_channel_MSEC_NODE_MAC(_tile,12, _instance, 11, 0),                                                \
        brg_tx_channel_MSEC_NODE_MAC(_tile,13, _instance, 12, 0),                                                \
        brg_tx_channel_MSEC_NODE_MAC(_tile,14, _instance, 13, 0),                                                \
        brg_tx_channel_MSEC_NODE_MAC(_tile,15, _instance, 14, 0),                                                \
        brg_tx_channel_MSEC_NODE_MAC(_tile,16, _instance, 15, 0)

#define BRG_TX_CHANNEL_GROUP_1_MSEC_16_NODES_MAC(_tile,_instance)                                             \
        brg_tx_channel_MSEC_NODE_MAC(_tile, 1, _instance, 16, 1),                                                \
        brg_tx_channel_MSEC_NODE_MAC(_tile, 2, _instance, 17, 1),                                                \
        brg_tx_channel_MSEC_NODE_MAC(_tile, 3, _instance, 18, 1),                                                \
        brg_tx_channel_MSEC_NODE_MAC(_tile, 4, _instance, 19, 1),                                                \
        brg_tx_channel_MSEC_NODE_MAC(_tile, 5, _instance, 20, 1),                                                \
        brg_tx_channel_MSEC_NODE_MAC(_tile, 6, _instance, 21, 1),                                                \
        brg_tx_channel_MSEC_NODE_MAC(_tile, 7, _instance, 22, 1),                                                \
        brg_tx_channel_MSEC_NODE_MAC(_tile, 8, _instance, 23, 1),                                                \
        brg_tx_channel_MSEC_NODE_MAC(_tile, 9, _instance, 24, 1),                                                \
        brg_tx_channel_MSEC_NODE_MAC(_tile,10, _instance, 25, 1),                                                \
        brg_tx_channel_MSEC_NODE_MAC(_tile,11, _instance, 26, 1),                                                \
        brg_tx_channel_MSEC_NODE_MAC(_tile,12, _instance, 27, 1),                                                \
        brg_tx_channel_MSEC_NODE_MAC(_tile,13, _instance, 28, 1),                                                \
        brg_tx_channel_MSEC_NODE_MAC(_tile,14, _instance, 29, 1),                                                \
        brg_tx_channel_MSEC_NODE_MAC(_tile,15, _instance, 30, 1),                                                \
        brg_tx_channel_MSEC_NODE_MAC(_tile,16, _instance, 31, 1)

#define brg_tx_channel_MSEC_NODE_MAC(_tile,bit, _instance, _channel, _group)                                           \
    {bit, GT_FALSE, UNIT_IN_TILE(_tile,PRV_CPSS_DXCH_UNIT_PCA_BRG_##_instance##_E), NULL,                                   \
        BRG_TX_CHANNEL_INTERRUPT_CAUSE_MAC(_channel, _group),                                               \
        BRG_TX_CHANNEL_INTERRUPT_MASK_MAC(_channel, _group),                                                \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_MSEC_INST_##_instance##_CHANNEL_##_channel##_TX_SLICE_PACKET_ON_DISABLED_CHANNEL_INTERRUPT_E,   \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_MSEC_INST_##_instance##_CHANNEL_##_channel##_TX_SDB_CREDIT_ON_DISABLED_CHANNEL_INTERRUPT_E,     \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define brg_tx_global_MSEC_NODE_MAC(_tile,bit, _instance)                                                     \
    {bit, GT_FALSE, UNIT_IN_TILE(_tile,PRV_CPSS_DXCH_UNIT_PCA_BRG_##_instance##_E), NULL,                                   \
        0x00001a64, 0x00001a68,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_MSEC_INST_##_instance##_TX_GLOBAL_INTERRUPT_CAUSE_ACCUM_MEMORY_FULL_E,            \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_MSEC_INST_##_instance##_TX_GLOBAL_INTERRUPT_CAUSE_TX_CREDIT_FIFO_OVERRUN_E,       \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define brg_tx_channel_group0_MSEC_SUB_TREE_MAC(_tile,bit, _instance)                                         \
    {bit, GT_FALSE, UNIT_IN_TILE(_tile,PRV_CPSS_DXCH_UNIT_PCA_BRG_##_instance##_E), NULL,                                   \
        0x00001900, 0x00001920,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_MSEC_INST_##_instance##_TX_CH_GROUP_0_INTERRUPT_CAUSE_SUMMARY_CHANNEL_0_INTERRUPT_CAUSE_SUMMARY_E,\
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_MSEC_INST_##_instance##_TX_CH_GROUP_0_INTERRUPT_CAUSE_SUMMARY_CHANNEL_15_INTERRUPT_CAUSE_SUMMARY_E, \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 16, NULL, NULL},                                        \
        BRG_TX_CHANNEL_GROUP_0_MSEC_16_NODES_MAC(_instance)

#define brg_tx_channel_group1_MSEC_SUB_TREE_MAC(_tile,bit, _instance)                                         \
    {bit, GT_FALSE, UNIT_IN_TILE(_tile,PRV_CPSS_DXCH_UNIT_PCA_BRG_##_instance##_E), NULL,                                   \
        0x00001904, 0x00001924,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_MSEC_INST_##_instance##_TX_CH_GROUP_1_INTERRUPT_CAUSE_SUMMARY_CHANNEL_16_INTERRUPT_CAUSE_SUMMARY_E,\
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_MSEC_INST_##_instance##_TX_CH_GROUP_1_INTERRUPT_CAUSE_SUMMARY_CHANNEL_31_INTERRUPT_CAUSE_SUMMARY_E, \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 16, NULL, NULL},                                        \
        BRG_TX_CHANNEL_GROUP_1_MSEC_16_NODES_MAC(_instance)

#define brg_MSEC_NODE_MAC(_tile,bit, _instance)                                                               \
    {bit, GT_FALSE, UNIT_IN_TILE(_tile,PRV_CPSS_DXCH_UNIT_PCA_BRG_##_instance##_E), NULL,                                   \
        0x00000004, 0x00000008,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_MSEC_INST_##_instance##_BRG_INTERRUPT_CAUSE_ILLEGAL_ADDRESS_ACCESS_E,             \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_MSEC_INST_##_instance##_BRG_INTERRUPT_CAUSE_ILLEGAL_ADDRESS_ACCESS_E,             \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define brg_MSEC_SUB_TREE_MAC(_tile,bit, _instance)                                                           \
    {bit, GT_FALSE, UNIT_IN_TILE(_tile,PRV_CPSS_DXCH_UNIT_PCA_BRG_##_instance##_E), NULL,                                   \
        0x0000000c, 0x00000010,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_MSEC_INST_##_instance##_BRG_INTERRUPT_CAUSE_SUMMARY_RX_GLOBAL_INTERRUPT_SUMMARY_E,\
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_MSEC_INST_##_instance##_BRG_INTERRUPT_CAUSE_SUMMARY_BRIDGE_CAUSE_INTERRUPT_SUMMARY_E, \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 7, NULL, NULL},                                         \
        brg_rx_global_MSEC_NODE_MAC(_tile,1, _instance),                                                      \
        brg_rx_channel_group0_MSEC_SUB_TREE_MAC(_tile,2, _instance),                                          \
        brg_rx_channel_group1_MSEC_SUB_TREE_MAC(_tile,3, _instance),                                          \
        brg_tx_global_MSEC_NODE_MAC(_tile,6, _instance),                                                      \
        brg_tx_channel_group0_MSEC_SUB_TREE_MAC(_tile,7, _instance),                                          \
        brg_tx_channel_group1_MSEC_SUB_TREE_MAC(_tile,8, _instance),                                          \
        brg_MSEC_NODE_MAC(_tile,9, _instance)

#define BRG_RX_CHANNEL_GROUP_0_PCA_16_NODES_MAC(_tile,_instance)                                             \
        brg_rx_channel_PCA_NODE_MAC(_tile, 1, _instance,  0, 0),                                                \
        brg_rx_channel_PCA_NODE_MAC(_tile, 2, _instance,  1, 0),                                                \
        brg_rx_channel_PCA_NODE_MAC(_tile, 3, _instance,  2, 0),                                                \
        brg_rx_channel_PCA_NODE_MAC(_tile, 4, _instance,  3, 0),                                                \
        brg_rx_channel_PCA_NODE_MAC(_tile, 5, _instance,  4, 0),                                                \
        brg_rx_channel_PCA_NODE_MAC(_tile, 6, _instance,  5, 0),                                                \
        brg_rx_channel_PCA_NODE_MAC(_tile, 7, _instance,  6, 0),                                                \
        brg_rx_channel_PCA_NODE_MAC(_tile, 8, _instance,  7, 0),                                                \
        brg_rx_channel_PCA_NODE_MAC(_tile, 9, _instance,  8, 0),                                                \
        brg_rx_channel_PCA_NODE_MAC(_tile,10, _instance,  9, 0),                                                \
        brg_rx_channel_PCA_NODE_MAC(_tile,11, _instance, 10, 0),                                                \
        brg_rx_channel_PCA_NODE_MAC(_tile,12, _instance, 11, 0),                                                \
        brg_rx_channel_PCA_NODE_MAC(_tile,13, _instance, 12, 0),                                                \
        brg_rx_channel_PCA_NODE_MAC(_tile,14, _instance, 13, 0),                                                \
        brg_rx_channel_PCA_NODE_MAC(_tile,15, _instance, 14, 0),                                                \
        brg_rx_channel_PCA_NODE_MAC(_tile,16, _instance, 15, 0)

#define BRG_RX_CHANNEL_GROUP_1_PCA_16_NODES_MAC(_tile,_instance)                                             \
        brg_rx_channel_PCA_NODE_MAC(_tile, 1, _instance, 16, 1),                                                \
        brg_rx_channel_PCA_NODE_MAC(_tile, 2, _instance, 17, 1),                                                \
        brg_rx_channel_PCA_NODE_MAC(_tile, 3, _instance, 18, 1),                                                \
        brg_rx_channel_PCA_NODE_MAC(_tile, 4, _instance, 19, 1),                                                \
        brg_rx_channel_PCA_NODE_MAC(_tile, 5, _instance, 20, 1),                                                \
        brg_rx_channel_PCA_NODE_MAC(_tile, 6, _instance, 21, 1),                                                \
        brg_rx_channel_PCA_NODE_MAC(_tile, 7, _instance, 22, 1),                                                \
        brg_rx_channel_PCA_NODE_MAC(_tile, 8, _instance, 23, 1),                                                \
        brg_rx_channel_PCA_NODE_MAC(_tile, 9, _instance, 24, 1),                                                \
        brg_rx_channel_PCA_NODE_MAC(_tile,10, _instance, 25, 1),                                                \
        brg_rx_channel_PCA_NODE_MAC(_tile,11, _instance, 26, 1),                                                \
        brg_rx_channel_PCA_NODE_MAC(_tile,12, _instance, 27, 1),                                                \
        brg_rx_channel_PCA_NODE_MAC(_tile,13, _instance, 28, 1),                                                \
        brg_rx_channel_PCA_NODE_MAC(_tile,14, _instance, 29, 1),                                                \
        brg_rx_channel_PCA_NODE_MAC(_tile,15, _instance, 30, 1),                                                \
        brg_rx_channel_PCA_NODE_MAC(_tile,16, _instance, 31, 1)

#define brg_rx_channel_PCA_NODE_MAC(_tile,bit, _instance, _channel, _group)                                           \
    {bit, GT_FALSE, UNIT_IN_TILE(_tile,PRV_CPSS_DXCH_UNIT_PCA_BRG_##_instance##_E), NULL,                                   \
        BRG_RX_CHANNEL_INTERRUPT_CAUSE_MAC(_channel, _group),                                    \
        BRG_RX_CHANNEL_INTERRUPT_MASK_MAC(_channel, _group),                                     \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_PCA_INST_##_instance##_CHANNEL_##_channel##_RX_SDB_PACKET_ON_DISABLED_CHANNEL_INTERRUPT_E,     \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_PCA_INST_##_instance##_CHANNEL_##_channel##_RX_SDB_PACKET_ON_DISABLED_CHANNEL_INTERRUPT_E,     \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define brg_rx_global_PCA_NODE_MAC(_tile,bit, _instance)                                                      \
    {bit, GT_FALSE, UNIT_IN_TILE(_tile,PRV_CPSS_DXCH_UNIT_PCA_BRG_##_instance##_E), NULL,                                   \
        0x00000750, 0x00000754,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_PCA_INST_##_instance##_RX_GLOBAL_INTERRUPT_CAUSE_RX_ROTATED_FIFO_OVERRUN_INTERRUPT_E,\
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_PCA_INST_##_instance##_RX_GLOBAL_INTERRUPT_CAUSE_RX_SDB_CHANNEL_ID_ERROR_INTERRUPT_E, \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define brg_rx_channel_group0_PCA_SUB_TREE_MAC(_tile,bit, _instance)                                          \
    {bit, GT_FALSE, UNIT_IN_TILE(_tile,PRV_CPSS_DXCH_UNIT_PCA_BRG_##_instance##_E), NULL,                                   \
        0x00000600, 0x00000620,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_PCA_INST_##_instance##_RX_CH_GROUP_0_INTERRUPT_CAUSE_SUMMARY_CHANNEL_0_INTERRUPT_CAUSE_SUMMARY_E,\
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_PCA_INST_##_instance##_RX_CH_GROUP_0_INTERRUPT_CAUSE_SUMMARY_CHANNEL_15_INTERRUPT_CAUSE_SUMMARY_E, \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 16, NULL, NULL},                                        \
        BRG_RX_CHANNEL_GROUP_0_PCA_16_NODES_MAC(_instance)

#define brg_rx_channel_group1_PCA_SUB_TREE_MAC(_tile,bit, _instance)                                          \
    {bit, GT_FALSE, UNIT_IN_TILE(_tile,PRV_CPSS_DXCH_UNIT_PCA_BRG_##_instance##_E), NULL,                                   \
        0x00000604, 0x00000624,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_PCA_INST_##_instance##_RX_CH_GROUP_1_INTERRUPT_CAUSE_SUMMARY_CHANNEL_16_INTERRUPT_CAUSE_SUMMARY_E,\
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_PCA_INST_##_instance##_RX_CH_GROUP_1_INTERRUPT_CAUSE_SUMMARY_CHANNEL_31_INTERRUPT_CAUSE_SUMMARY_E, \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 16, NULL, NULL},                                        \
        BRG_RX_CHANNEL_GROUP_1_PCA_16_NODES_MAC(_instance)

#define BRG_TX_CHANNEL_GROUP_0_PCA_16_NODES_MAC(_tile,_instance)                                             \
        brg_tx_channel_PCA_NODE_MAC(_tile, 1, _instance,  0, 0),                                                \
        brg_tx_channel_PCA_NODE_MAC(_tile, 2, _instance,  1, 0),                                                \
        brg_tx_channel_PCA_NODE_MAC(_tile, 3, _instance,  2, 0),                                                \
        brg_tx_channel_PCA_NODE_MAC(_tile, 4, _instance,  3, 0),                                                \
        brg_tx_channel_PCA_NODE_MAC(_tile, 5, _instance,  4, 0),                                                \
        brg_tx_channel_PCA_NODE_MAC(_tile, 6, _instance,  5, 0),                                                \
        brg_tx_channel_PCA_NODE_MAC(_tile, 7, _instance,  6, 0),                                                \
        brg_tx_channel_PCA_NODE_MAC(_tile, 8, _instance,  7, 0),                                                \
        brg_tx_channel_PCA_NODE_MAC(_tile, 9, _instance,  8, 0),                                                \
        brg_tx_channel_PCA_NODE_MAC(_tile,10, _instance,  9, 0),                                                \
        brg_tx_channel_PCA_NODE_MAC(_tile,11, _instance, 10, 0),                                                \
        brg_tx_channel_PCA_NODE_MAC(_tile,12, _instance, 11, 0),                                                \
        brg_tx_channel_PCA_NODE_MAC(_tile,13, _instance, 12, 0),                                                \
        brg_tx_channel_PCA_NODE_MAC(_tile,14, _instance, 13, 0),                                                \
        brg_tx_channel_PCA_NODE_MAC(_tile,15, _instance, 14, 0),                                                \
        brg_tx_channel_PCA_NODE_MAC(_tile,16, _instance, 15, 0)

#define BRG_TX_CHANNEL_GROUP_1_PCA_16_NODES_MAC(_tile,_instance)                                             \
        brg_tx_channel_PCA_NODE_MAC(_tile, 1, _instance, 16, 1),                                                \
        brg_tx_channel_PCA_NODE_MAC(_tile, 2, _instance, 17, 1),                                                \
        brg_tx_channel_PCA_NODE_MAC(_tile, 3, _instance, 18, 1),                                                \
        brg_tx_channel_PCA_NODE_MAC(_tile, 4, _instance, 19, 1),                                                \
        brg_tx_channel_PCA_NODE_MAC(_tile, 5, _instance, 20, 1),                                                \
        brg_tx_channel_PCA_NODE_MAC(_tile, 6, _instance, 21, 1),                                                \
        brg_tx_channel_PCA_NODE_MAC(_tile, 7, _instance, 22, 1),                                                \
        brg_tx_channel_PCA_NODE_MAC(_tile, 8, _instance, 23, 1),                                                \
        brg_tx_channel_PCA_NODE_MAC(_tile, 9, _instance, 24, 1),                                                \
        brg_tx_channel_PCA_NODE_MAC(_tile,10, _instance, 25, 1),                                                \
        brg_tx_channel_PCA_NODE_MAC(_tile,11, _instance, 26, 1),                                                \
        brg_tx_channel_PCA_NODE_MAC(_tile,12, _instance, 27, 1),                                                \
        brg_tx_channel_PCA_NODE_MAC(_tile,13, _instance, 28, 1),                                                \
        brg_tx_channel_PCA_NODE_MAC(_tile,14, _instance, 29, 1),                                                \
        brg_tx_channel_PCA_NODE_MAC(_tile,15, _instance, 30, 1),                                                \
        brg_tx_channel_PCA_NODE_MAC(_tile,16, _instance, 31, 1)

#define brg_tx_channel_PCA_NODE_MAC(_tile,bit, _instance, _channel, _group)                                          \
    {bit, GT_FALSE, UNIT_IN_TILE(_tile,PRV_CPSS_DXCH_UNIT_PCA_BRG_##_instance##_E), NULL,                                  \
        BRG_TX_CHANNEL_INTERRUPT_CAUSE_MAC(_channel, _group),                                   \
        BRG_TX_CHANNEL_INTERRUPT_MASK_MAC(_channel, _group),                                    \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                               \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_PCA_INST_##_instance##_CHANNEL_##_channel##_TX_SLICE_PACKET_ON_DISABLED_CHANNEL_INTERRUPT_E,   \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_PCA_INST_##_instance##_CHANNEL_##_channel##_TX_SDB_CREDIT_ON_DISABLED_CHANNEL_INTERRUPT_E,     \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define brg_tx_global_PCA_NODE_MAC(_tile,bit, _instance)                                                      \
    {bit, GT_FALSE, UNIT_IN_TILE(_tile,PRV_CPSS_DXCH_UNIT_PCA_BRG_##_instance##_E), NULL,                                   \
        0x00001a64, 0x00001a68,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_PCA_INST_##_instance##_TX_GLOBAL_INTERRUPT_CAUSE_ACCUM_MEMORY_FULL_E,             \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_PCA_INST_##_instance##_TX_GLOBAL_INTERRUPT_CAUSE_TX_CREDIT_FIFO_OVERRUN_E,        \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define brg_tx_channel_group0_PCA_SUB_TREE_MAC(_tile,bit, _instance)                                          \
    {bit, GT_FALSE, UNIT_IN_TILE(_tile,PRV_CPSS_DXCH_UNIT_PCA_BRG_##_instance##_E), NULL,                                   \
        0x00001900, 0x00001920,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_PCA_INST_##_instance##_TX_CH_GROUP_0_INTERRUPT_CAUSE_SUMMARY_CHANNEL_0_INTERRUPT_CAUSE_SUMMARY_E,\
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_PCA_INST_##_instance##_TX_CH_GROUP_0_INTERRUPT_CAUSE_SUMMARY_CHANNEL_15_INTERRUPT_CAUSE_SUMMARY_E, \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 16, NULL, NULL},                                        \
        BRG_TX_CHANNEL_GROUP_0_PCA_16_NODES_MAC(_instance)

#define brg_tx_channel_group1_PCA_SUB_TREE_MAC(_tile,bit, _instance)                                          \
    {bit, GT_FALSE, UNIT_IN_TILE(_tile,PRV_CPSS_DXCH_UNIT_PCA_BRG_##_instance##_E), NULL,                                   \
        0x00001904, 0x00001924,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_PCA_INST_##_instance##_TX_CH_GROUP_1_INTERRUPT_CAUSE_SUMMARY_CHANNEL_16_INTERRUPT_CAUSE_SUMMARY_E,\
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_PCA_INST_##_instance##_TX_CH_GROUP_1_INTERRUPT_CAUSE_SUMMARY_CHANNEL_31_INTERRUPT_CAUSE_SUMMARY_E, \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 16, NULL, NULL},                                        \
        BRG_TX_CHANNEL_GROUP_1_PCA_16_NODES_MAC(_instance)

#define brg_PCA_NODE_MAC(_tile,bit, _instance)                                                                \
    {bit, GT_FALSE, UNIT_IN_TILE(_tile,PRV_CPSS_DXCH_UNIT_PCA_BRG_##_instance##_E), NULL,                                   \
        0x00000004, 0x00000008,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_PCA_INST_##_instance##_BRG_INTERRUPT_CAUSE_ILLEGAL_ADDRESS_ACCESS_E,              \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_PCA_INST_##_instance##_BRG_INTERRUPT_CAUSE_ILLEGAL_ADDRESS_ACCESS_E,              \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define brg_PCA_SUB_TREE_MAC(_tile,bit, _instance)                                                            \
    {bit, GT_FALSE, UNIT_IN_TILE(_tile,PRV_CPSS_DXCH_UNIT_PCA_BRG_##_instance##_E), NULL,                                   \
        0x0000000c, 0x00000010,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_PCA_INST_##_instance##_BRG_INTERRUPT_CAUSE_SUMMARY_RX_GLOBAL_INTERRUPT_SUMMARY_E, \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_PCA_INST_##_instance##_BRG_INTERRUPT_CAUSE_SUMMARY_BRIDGE_CAUSE_INTERRUPT_SUMMARY_E, \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 7, NULL, NULL},                                         \
        brg_rx_global_PCA_NODE_MAC(_tile,1, _instance),                                                       \
        brg_rx_channel_group0_PCA_SUB_TREE_MAC(_tile,2, _instance),                                           \
        brg_rx_channel_group1_PCA_SUB_TREE_MAC(_tile,3, _instance),                                           \
        brg_tx_global_PCA_NODE_MAC(_tile,6, _instance),                                                       \
        brg_tx_channel_group0_PCA_SUB_TREE_MAC(_tile,7, _instance),                                           \
        brg_tx_channel_group1_PCA_SUB_TREE_MAC(_tile,8, _instance),                                           \
        brg_PCA_NODE_MAC(_tile,9, _instance)


#define AAS_CNM_RFU_GLOBAL_INTERRUPT_SUMMARY_MAC(_tileId, bitIndexInCaller, numChild, _treeId)          \
    /* Global Interrupt Cause */                                                                        \
    {bitIndexInCaller, GT_FALSE, CNM_INTERRUPTS_UNIT(_tileId), NULL,                                           \
        0x00000100, 0x00000104,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AAS_CNM_CNM_TILE_##_tileId##_TREE_##_treeId##_GLOBAL_INTERRUPTS_SUMMARY_CAUSE_GLOBAL_CN_M_AGGREGATION_INT_SUM_E,        \
        PRV_CPSS_AAS_CNM_CNM_TILE_##_tileId##_TREE_##_treeId##_GLOBAL_INTERRUPTS_SUMMARY_CAUSE_GLOBAL_UNITS_14_INT_SUM_E, \
        FILLED_IN_RUNTIME_CNS, 0, 0x0, numChild, NULL, NULL}


#define xsmi_IntsSum_NODE_MAC(bit, _tileId, _instId) \
        {bit, GT_FALSE, UNIT_IN_TILE(_tileId, PRV_CPSS_DXCH_UNIT_CNM_PERIPHERALS_E), NULL, \
            0x00000010, 0x00000014, \
            prvCpssDrvHwPpPortGroupIsrRead, \
            prvCpssDrvHwPpPortGroupIsrWrite, \
            PRV_CPSS_AAS_CNM_CNM_TILE_##_tileId##_XSMI_INST_##_instId##_XSMI_INTERRUPT_CAUSE_ILLEGAL_ADDRESS_ACCESS_INT_E,\
            PRV_CPSS_AAS_CNM_CNM_TILE_##_tileId##_XSMI_INST_##_instId##_XSMI_INTERRUPT_CAUSE_XG_SMI_WRITE_INT_E,\
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define cnm_0_IntsSum_SUB_TREE_MAC(_tileId, bit, _treeId)                                               \
    /*         CnM 0 Interrupt Summary Cause */                                                                \
    {bit, GT_FALSE, CNM_INTERRUPTS_UNIT(_tileId), NULL,                                                        \
        CNM_RFU_INT_ADDR_MAC(0x00000108, 0x0000010c, _treeId),                                          \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AAS_CNM_CNM_TILE_##_tileId##_TREE_##_treeId##_CN_M_0_INTERRUPT_SUMMARY_CAUSE_CNM_0_INT_SUM_1_E,    \
        PRV_CPSS_AAS_CNM_CNM_TILE_##_tileId##_TREE_##_treeId##_CN_M_0_INTERRUPT_SUMMARY_CAUSE_CNM_0_INT_SUM_31_E,   \
        FILLED_IN_RUNTIME_CNS, 0, 0x0, 11, NULL, NULL},                                                 \
        /* 0..7, GPIO_0_31_Interrupt Cause 0 */                                                         \
        {1, GT_TRUE, 0, prvCpssDrvDxExMxRunitGppIsrCall,                                                \
                0x00018114,                                                                             \
                PRV_CPSS_DRV_PEX_INDICATION_IN_ADDR_MAC(0x00018118),                                    \
                prvCpssDrvHwPpPortGroupReadInternalPciReg,                                              \
                prvCpssDrvHwPpPortGroupWriteInternalPciReg,                                             \
                PRV_CPSS_AAS_CNM_CNM_TILE_##_tileId##_GPIO_INTERRUPT_CAUSE_0_RESERVED_1_E,              \
                PRV_CPSS_AAS_CNM_CNM_TILE_##_tileId##_GPIO_INTERRUPT_CAUSE_0_RESERVED_7_E,              \
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},                                 \
                                                                                                        \
        /* 8..15, GPIO_0_31_Interrupt Cause 0 */                                                        \
        {2, GT_TRUE, 8, prvCpssDrvDxExMxRunitGppIsrCall,                                                \
                0x00018114,                                                                             \
                PRV_CPSS_DRV_PEX_INDICATION_IN_ADDR_MAC(0x00018118),                                    \
                prvCpssDrvHwPpPortGroupReadInternalPciReg,                                              \
                prvCpssDrvHwPpPortGroupWriteInternalPciReg,                                             \
                PRV_CPSS_AAS_CNM_CNM_TILE_##_tileId##_GPIO_INTERRUPT_CAUSE_0_RESERVED_8_E,              \
                PRV_CPSS_AAS_CNM_CNM_TILE_##_tileId##_GPIO_INTERRUPT_CAUSE_0_RESERVED_15_E,             \
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},                                 \
                                                                                                        \
        /* 16..23, GPIO_0_31_Interrupt Cause 0 */                                                       \
        {3, GT_TRUE, 16, prvCpssDrvDxExMxRunitGppIsrCall,                                               \
                0x00018114,                                                                             \
                PRV_CPSS_DRV_PEX_INDICATION_IN_ADDR_MAC(0x00018118),                                    \
                prvCpssDrvHwPpPortGroupReadInternalPciReg,                                              \
                prvCpssDrvHwPpPortGroupWriteInternalPciReg,                                             \
                PRV_CPSS_AAS_CNM_CNM_TILE_##_tileId##_GPIO_INTERRUPT_CAUSE_0_RESERVED_16_E,             \
                PRV_CPSS_AAS_CNM_CNM_TILE_##_tileId##_GPIO_INTERRUPT_CAUSE_0_RESERVED_23_E,             \
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},                                 \
                                                                                                        \
        /* 24..31, GPIO_0_31_Interrupt Cause 0 */                                                       \
        {4, GT_TRUE, 24, prvCpssDrvDxExMxRunitGppIsrCall,                                               \
                0x00018114,                                                                             \
                PRV_CPSS_DRV_PEX_INDICATION_IN_ADDR_MAC(0x00018118),                                    \
                prvCpssDrvHwPpPortGroupReadInternalPciReg,                                              \
                prvCpssDrvHwPpPortGroupWriteInternalPciReg,                                             \
                PRV_CPSS_AAS_CNM_CNM_TILE_##_tileId##_GPIO_INTERRUPT_CAUSE_0_RESERVED_24_E,             \
                PRV_CPSS_AAS_CNM_CNM_TILE_##_tileId##_GPIO_INTERRUPT_CAUSE_0_RESERVED_31_E,             \
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},                                 \
                                                                                                        \
        /* 32..39, GPIO_0_31_Interrupt Cause 1 */                                                       \
        {5, GT_TRUE, 32, prvCpssDrvDxExMxRunitGppIsrCall,                                               \
                0x00018154,                                                                             \
                PRV_CPSS_DRV_PEX_INDICATION_IN_ADDR_MAC(0x00018158),                                    \
                prvCpssDrvHwPpPortGroupReadInternalPciReg,                                              \
                prvCpssDrvHwPpPortGroupWriteInternalPciReg,                                             \
                PRV_CPSS_AAS_CNM_CNM_TILE_##_tileId##_GPIO_INTERRUPT_CAUSE_1_RESERVED_1_E,              \
                PRV_CPSS_AAS_CNM_CNM_TILE_##_tileId##_GPIO_INTERRUPT_CAUSE_1_RESERVED_7_E,              \
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},                                 \
                                                                                                        \
        /* 40..47, GPIO_0_31_Interrupt Cause 1 */                                                       \
        {6, GT_TRUE, 40, prvCpssDrvDxExMxRunitGppIsrCall,                                               \
                0x00018154,                                                                             \
                PRV_CPSS_DRV_PEX_INDICATION_IN_ADDR_MAC(0x00018158),                                    \
                prvCpssDrvHwPpPortGroupReadInternalPciReg,                                              \
                prvCpssDrvHwPpPortGroupWriteInternalPciReg,                                             \
                PRV_CPSS_AAS_CNM_CNM_TILE_##_tileId##_GPIO_INTERRUPT_CAUSE_1_RESERVED_8_E,              \
                PRV_CPSS_AAS_CNM_CNM_TILE_##_tileId##_GPIO_INTERRUPT_CAUSE_1_RESERVED_15_E,             \
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},                                 \
                                                                                                        \
        /* 48..55, GPIO_0_31_Interrupt Cause 1 */                                                       \
        {7, GT_TRUE, 48, prvCpssDrvDxExMxRunitGppIsrCall,                                               \
                0x00018154,                                                                             \
                PRV_CPSS_DRV_PEX_INDICATION_IN_ADDR_MAC(0x00018158),                                    \
                prvCpssDrvHwPpPortGroupReadInternalPciReg,                                              \
                prvCpssDrvHwPpPortGroupWriteInternalPciReg,                                             \
                PRV_CPSS_AAS_CNM_CNM_TILE_##_tileId##_GPIO_INTERRUPT_CAUSE_1_RESERVED_16_E,             \
                PRV_CPSS_AAS_CNM_CNM_TILE_##_tileId##_GPIO_INTERRUPT_CAUSE_1_RESERVED_23_E,             \
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},                                 \
                                                                                                        \
        /* 56..63, GPIO_0_31_Interrupt Cause 1 */                                                       \
        {8, GT_TRUE, 56, prvCpssDrvDxExMxRunitGppIsrCall,                                               \
                0x00018154,                                                                             \
                PRV_CPSS_DRV_PEX_INDICATION_IN_ADDR_MAC(0x00018158),                                    \
                prvCpssDrvHwPpPortGroupReadInternalPciReg,                                              \
                prvCpssDrvHwPpPortGroupWriteInternalPciReg,                                             \
                PRV_CPSS_AAS_CNM_CNM_TILE_##_tileId##_GPIO_INTERRUPT_CAUSE_1_RESERVED_24_E,             \
                PRV_CPSS_AAS_CNM_CNM_TILE_##_tileId##_GPIO_INTERRUPT_CAUSE_1_RESERVED_31_E,             \
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},                                 \
                                                                                                        \
        /* XSMI Interrupt Cause */                                                                      \
        xsmi_IntsSum_NODE_MAC(9, _tileId, 0),                                                           \
        xsmi_IntsSum_NODE_MAC(9, _tileId, 1),                                                           \
        xsmi_IntsSum_NODE_MAC(9, _tileId, 2)

#define cnm_1_IntsSum_SUB_TREE_MAC(_tileId, bit, _treeId)                                               \
    /*         CnM 1 Interrupt Summary Cause */                                                                \
    {bit, GT_FALSE, CNM_INTERRUPTS_UNIT(_tileId), NULL,                                                        \
        CNM_RFU_INT_ADDR_MAC(0x00000110, 0x0000011c, _treeId),                                          \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AAS_CNM_CNM_TILE_##_tileId##_TREE_##_treeId##_CN_M_1_INTERRUPT_SUMMARY_CAUSE_CNM_1_INT_SUM_1_E,    \
        PRV_CPSS_AAS_CNM_CNM_TILE_##_tileId##_TREE_##_treeId##_CN_M_1_INTERRUPT_SUMMARY_CAUSE_CNM_1_INT_SUM_31_E,   \
        FILLED_IN_RUNTIME_CNS, 0, 0x0, 0, NULL, NULL}

#define cnm_2_IntsSum_SUB_TREE_MAC(_tileId, bit, _treeId)                                     \
    /*         CnM 2 Interrupt Summary Cause */                                                \
    {bit, GT_FALSE, CNM_INTERRUPTS_UNIT(_tileId), NULL,                                        \
        CNM_RFU_INT_ADDR_MAC(0x00000118, 0x0000018c, _treeId),                          \
        prvCpssDrvHwPpPortGroupIsrRead,                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                \
        PRV_CPSS_AAS_CNM_CNM_TILE_##_tileId##_TREE_##_treeId##_CN_M_2_INTERRUPT_SUMMARY_CAUSE_CNM_2_INT_SUM_1_E,    \
        PRV_CPSS_AAS_CNM_CNM_TILE_##_tileId##_TREE_##_treeId##_CN_M_2_INTERRUPT_SUMMARY_CAUSE_CNM_2_INT_SUM_31_E,   \
        FILLED_IN_RUNTIME_CNS, 0, 0x0, 0, NULL, NULL}

#define cnm_3_IntsSum_SUB_TREE_MAC(_tileId, bit, _treeId)                                     \
    /*         CnM 3 Interrupt Summary Cause */                                                \
    {bit, GT_FALSE, CNM_INTERRUPTS_UNIT(_tileId), NULL,                                        \
        CNM_RFU_INT_ADDR_MAC(0x00000120, 0x00000124, _treeId),                          \
        prvCpssDrvHwPpPortGroupIsrRead,                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                \
        PRV_CPSS_AAS_CNM_CNM_TILE_##_tileId##_TREE_##_treeId##_CN_M_3_INTERRUPT_SUMMARY_CAUSE_CNM_3_INT_SUM_1_E,    \
        PRV_CPSS_AAS_CNM_CNM_TILE_##_tileId##_TREE_##_treeId##_CN_M_3_INTERRUPT_SUMMARY_CAUSE_CNM_3_INT_SUM_31_E,   \
        FILLED_IN_RUNTIME_CNS, 0, 0x0, 0, NULL, NULL}

#define units_0_IntsSum_SUB_TREE_MAC(_tileId, bit, _treeId)                                     \
    /* Units 0 Interrupts Summary Cause */                                              \
    {bit, GT_FALSE, CNM_INTERRUPTS_UNIT(_tileId), NULL,                                        \
        CNM_RFU_INT_ADDR_MAC(0x00000128, 0x0000012c, _treeId),                          \
        prvCpssDrvHwPpPortGroupIsrRead,                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                \
        PRV_CPSS_AAS_CNM_CNM_TILE_##_tileId##_TREE_##_treeId##_UNITS_0_INTERRUPTS_SUMMARY_CAUSE_UNITS_0_INT_SUM_1_E,    \
        PRV_CPSS_AAS_CNM_CNM_TILE_##_tileId##_TREE_##_treeId##_UNITS_0_INTERRUPTS_SUMMARY_CAUSE_UNITS_0_INT_SUM_31_E,   \
        FILLED_IN_RUNTIME_CNS, 0, 0x0, 0, NULL, NULL}

#define units_1_IntsSum_SUB_TREE_MAC(_tileId, bit, _treeId)                                     \
    /* Units 1 Interrupts Summary Cause */                                              \
    {bit, GT_FALSE, CNM_INTERRUPTS_UNIT(_tileId), NULL,                                        \
        CNM_RFU_INT_ADDR_MAC(0x00000130, 0x00000134, _treeId),                          \
        prvCpssDrvHwPpPortGroupIsrRead,                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                \
        PRV_CPSS_AAS_CNM_CNM_TILE_##_tileId##_TREE_##_treeId##_UNITS_1_INTERRUPTS_SUMMARY_CAUSE_UNITS_1_INT_SUM_1_E,    \
        PRV_CPSS_AAS_CNM_CNM_TILE_##_tileId##_TREE_##_treeId##_UNITS_1_INTERRUPTS_SUMMARY_CAUSE_UNITS_1_INT_SUM_31_E,   \
        FILLED_IN_RUNTIME_CNS, 0, 0x0, 0, NULL, NULL}

#define units_2_IntsSum_SUB_TREE_MAC(_tileId, bit, _treeId)                                     \
    /* Units 2 Interrupts Summary Cause */                                              \
    {bit, GT_FALSE, CNM_INTERRUPTS_UNIT(_tileId), NULL,                                        \
        CNM_RFU_INT_ADDR_MAC(0x00000138, 0x0000013c, _treeId),                          \
        prvCpssDrvHwPpPortGroupIsrRead,                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                \
        PRV_CPSS_AAS_CNM_CNM_TILE_##_tileId##_TREE_##_treeId##_UNITS_2_INTERRUPTS_SUMMARY_CAUSE_UNITS_2_INT_SUM_1_E,    \
        PRV_CPSS_AAS_CNM_CNM_TILE_##_tileId##_TREE_##_treeId##_UNITS_2_INTERRUPTS_SUMMARY_CAUSE_UNITS_2_INT_SUM_31_E,   \
        FILLED_IN_RUNTIME_CNS, 0, 0x0, 0, NULL, NULL}

#define units_3_IntsSum_SUB_TREE_MAC(_tileId, bit, _treeId)                                     \
    /* Units 3 Interrupts Summary Cause */                                              \
    {bit, GT_FALSE, CNM_INTERRUPTS_UNIT(_tileId), NULL,                                        \
        CNM_RFU_INT_ADDR_MAC(0x00000140, 0x00000144, _treeId),                          \
        prvCpssDrvHwPpPortGroupIsrRead,                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                \
        PRV_CPSS_AAS_CNM_CNM_TILE_##_tileId##_TREE_##_treeId##_UNITS_3_INTERRUPTS_SUMMARY_CAUSE_UNITS_3_INT_SUM_1_E,    \
        PRV_CPSS_AAS_CNM_CNM_TILE_##_tileId##_TREE_##_treeId##_UNITS_3_INTERRUPTS_SUMMARY_CAUSE_UNITS_3_INT_SUM_31_E,   \
        FILLED_IN_RUNTIME_CNS, 0, 0x0, 0, NULL, NULL}

#define units_4_IntsSum_SUB_TREE_MAC(_tileId, bit, _treeId)                                     \
    /* Units 4 Interrupts Summary Cause */                                              \
    {bit, GT_FALSE, CNM_INTERRUPTS_UNIT(_tileId), NULL,                                        \
        CNM_RFU_INT_ADDR_MAC(0x00000148, 0x0000014c, _treeId),                          \
        prvCpssDrvHwPpPortGroupIsrRead,                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                \
        PRV_CPSS_AAS_CNM_CNM_TILE_##_tileId##_TREE_##_treeId##_UNITS_4_INTERRUPTS_SUMMARY_CAUSE_UNITS_4_INTERRUPT_SUM_1_E,    \
        PRV_CPSS_AAS_CNM_CNM_TILE_##_tileId##_TREE_##_treeId##_UNITS_4_INTERRUPTS_SUMMARY_CAUSE_UNITS_4_INTERRUPT_SUM_31_E,   \
        FILLED_IN_RUNTIME_CNS, 0, 0x0, 0, NULL, NULL}

#define units_5_IntsSum_SUB_TREE_MAC(_tileId, bit, _treeId)                                     \
    /* Units 5 Interrupts Summary Cause */                                              \
    {bit, GT_FALSE, CNM_INTERRUPTS_UNIT(_tileId), NULL,                                        \
        CNM_RFU_INT_ADDR_MAC(0x00000150, 0x00000154, _treeId),                          \
        prvCpssDrvHwPpPortGroupIsrRead,                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                \
        PRV_CPSS_AAS_CNM_CNM_TILE_##_tileId##_TREE_##_treeId##_UNITS_5_INTERRUPTS_SUMMARY_CAUSE_UNITS_5_INTERRUPT_SUM_1_E,    \
        PRV_CPSS_AAS_CNM_CNM_TILE_##_tileId##_TREE_##_treeId##_UNITS_5_INTERRUPTS_SUMMARY_CAUSE_UNITS_5_INTERRUPT_SUM_31_E,   \
        FILLED_IN_RUNTIME_CNS, 0, 0x0, 0, NULL, NULL}

#define units_6_IntsSum_SUB_TREE_MAC(_tileId, bit, _treeId)                                     \
    /* Units 6 Interrupts Summary Cause */                                              \
    {bit, GT_FALSE, CNM_INTERRUPTS_UNIT(_tileId), NULL,                                        \
        CNM_RFU_INT_ADDR_MAC(0x00000158, 0x0000015c, _treeId),                          \
        prvCpssDrvHwPpPortGroupIsrRead,                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                \
        PRV_CPSS_AAS_CNM_CNM_TILE_##_tileId##_TREE_##_treeId##_UNITS_6_INTERRUPTS_SUMMARY_CAUSE_UNITS_6_INT_SUM_1_E,    \
        PRV_CPSS_AAS_CNM_CNM_TILE_##_tileId##_TREE_##_treeId##_UNITS_6_INTERRUPTS_SUMMARY_CAUSE_UNITS_6_INT_SUM_31_E,   \
        FILLED_IN_RUNTIME_CNS, 0, 0x0, 0, NULL, NULL}

#define units_7_IntsSum_SUB_TREE_MAC(_tileId, bit, _treeId)                                     \
    /* Units 7 Interrupts Summary Cause */                                              \
    {bit, GT_FALSE, CNM_INTERRUPTS_UNIT(_tileId), NULL,                                        \
        CNM_RFU_INT_ADDR_MAC(0x00000160, 0x00000164, _treeId),                          \
        prvCpssDrvHwPpPortGroupIsrRead,                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                \
        PRV_CPSS_AAS_CNM_CNM_TILE_##_tileId##_TREE_##_treeId##_UNITS_7_INTERRUPTS_SUMMARY_CAUSE_UNITS_7_INTERRUPT_SUM_1_E,    \
        PRV_CPSS_AAS_CNM_CNM_TILE_##_tileId##_TREE_##_treeId##_UNITS_7_INTERRUPTS_SUMMARY_CAUSE_UNITS_7_INTERRUPT_SUM_31_E,   \
        FILLED_IN_RUNTIME_CNS, 0, 0x0, 0, NULL, NULL}

#define units_8_IntsSum_SUB_TREE_MAC(_tileId, bit, _treeId)                                     \
    /* Units 8 Interrupts Summary Cause */                                              \
    {bit, GT_FALSE, CNM_INTERRUPTS_UNIT(_tileId), NULL,                                        \
        CNM_RFU_INT_ADDR_MAC(0x00000168, 0x0000016c, _treeId),                          \
        prvCpssDrvHwPpPortGroupIsrRead,                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                \
        PRV_CPSS_AAS_CNM_CNM_TILE_##_tileId##_TREE_##_treeId##_UNITS_8_INTERRUPTS_SUMMARY_CAUSE_UNITS_8_INTERRUPT_SUM_1_E,    \
        PRV_CPSS_AAS_CNM_CNM_TILE_##_tileId##_TREE_##_treeId##_UNITS_8_INTERRUPTS_SUMMARY_CAUSE_UNITS_8_INTERRUPT_SUM_31_E,   \
        FILLED_IN_RUNTIME_CNS, 0, 0x0, 0, NULL, NULL}

#define units_9_IntsSum_SUB_TREE_MAC(_tileId, bit, _treeId)                                     \
    /* Units 9 Interrupts Summary Cause */                                              \
    {bit, GT_FALSE, CNM_INTERRUPTS_UNIT(_tileId), NULL,                                        \
        CNM_RFU_INT_ADDR_MAC(0x00000170, 0x00000174, _treeId),                          \
        prvCpssDrvHwPpPortGroupIsrRead,                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                \
        PRV_CPSS_AAS_CNM_CNM_TILE_##_tileId##_TREE_##_treeId##_UNITS_9_INTERRUPTS_SUMMARY_CAUSE_UNITS_9_INTERRUPT_SUM_1_E,    \
        PRV_CPSS_AAS_CNM_CNM_TILE_##_tileId##_TREE_##_treeId##_UNITS_9_INTERRUPTS_SUMMARY_CAUSE_UNITS_9_INTERRUPT_SUM_31_E,   \
        FILLED_IN_RUNTIME_CNS, 0, 0x0, 0, NULL, NULL}

#define high_priority_coalescing_bypass_0_IntsSum_SUB_TREE_MAC(_tileId, bit, _treeId)                                     \
    /* High Priority Coalescing Bypass 0 Interrupts Summary Cause */                    \
    {bit, GT_FALSE, CNM_INTERRUPTS_UNIT(_tileId), NULL,                                        \
        CNM_RFU_INT_ADDR_MAC(0x000001a8, 0x000001ac, _treeId),                          \
        prvCpssDrvHwPpPortGroupIsrRead,                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                \
        PRV_CPSS_AAS_CNM_CNM_TILE_##_tileId##_TREE_##_treeId##_HIGH_PRIORITY_COALESCING_BYPASS_0_INTERRUPTS_SUMMARY_CAUSE_HIGH_PRIORITY_COALESCING_BYPASS_0_INT_SUM_1_E,    \
        PRV_CPSS_AAS_CNM_CNM_TILE_##_tileId##_TREE_##_treeId##_HIGH_PRIORITY_COALESCING_BYPASS_0_INTERRUPTS_SUMMARY_CAUSE_HIGH_PRIORITY_COALESCING_BYPASS_0_INT_SUM_31_E,   \
        FILLED_IN_RUNTIME_CNS, 0, 0x0, 0, NULL, NULL}

#define high_priority_coalescing_bypass_1_IntsSum_SUB_TREE_MAC(_tileId, bit, _treeId)                                     \
    /* High Priority Coalescing Bypass 1 Interrupts Summary Cause */                    \
    {bit, GT_FALSE, CNM_INTERRUPTS_UNIT(_tileId), NULL,                                        \
        CNM_RFU_INT_ADDR_MAC(0x000001b0, 0x000001b4, _treeId),                          \
        prvCpssDrvHwPpPortGroupIsrRead,                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                \
        PRV_CPSS_AAS_CNM_CNM_TILE_##_tileId##_TREE_##_treeId##_HIGH_PRIORITY_COALESCING_BYPASS_1_INTERRUPTS_SUMMARY_CAUSE_HIGH_PRIORITY_COALESCING_BYPASS_1_INT_SUM_1_E,    \
        PRV_CPSS_AAS_CNM_CNM_TILE_##_tileId##_TREE_##_treeId##_HIGH_PRIORITY_COALESCING_BYPASS_1_INTERRUPTS_SUMMARY_CAUSE_HIGH_PRIORITY_COALESCING_BYPASS_1_INT_SUM_31_E,   \
        FILLED_IN_RUNTIME_CNS, 0, 0x0, 0, NULL, NULL}

#define gdma_ring_interrupt_cause_NODE_MAC(bit, _tileId, _gdmaId, _index)                   \
    {bit, GT_FALSE, UNIT_IN_TILE(_tileId, PRV_CPSS_DXCH_UNIT_CNM_GDMA_0_E +_gdmaId), NULL,  \
        GDMA_INT_ADDR_MAC(0x00004300, 0x00004380, _index),                                  \
        prvCpssDrvHwPpPortGroupIsrRead,     \
        prvCpssDrvHwPpPortGroupIsrWrite,    \
        PRV_CPSS_AAS_CNM_CNM_TILE_##_tileId##_GDMA_##_gdmaId##_QUEUE_0_MEM2MEM_TRANSFER_DONE_INTERRUPT_E + (_index * 32),\
        PRV_CPSS_AAS_CNM_CNM_TILE_##_tileId##_GDMA_##_gdmaId##_QUEUE_0_RX_SGD_DONE_INTERRUPT_E + (_index * 32),\
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}


#define gdma_0_IntsSum_SUB_TREE_MAC(_tileId, bit, _treeId)                                              \
    /* GDMA 0 Interrupts Summary Cause */                                                               \
    {bit, GT_FALSE, CNM_INTERRUPTS_UNIT(_tileId), NULL,                                                        \
        CNM_RFU_INT_ADDR_MAC(0x000001b8,0x000001bc, _treeId),                                           \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AAS_CNM_CNM_TILE_##_tileId##_TREE_##_treeId##_GDMA_0_INTERRUPTS_SUMMARY_CAUSE_GDMA_0_INT_SUM_0_E,  \
        PRV_CPSS_AAS_CNM_CNM_TILE_##_tileId##_TREE_##_treeId##_GDMA_0_INTERRUPTS_SUMMARY_CAUSE_GDMA_0_INT_SUM_16_E, \
        FILLED_IN_RUNTIME_CNS, 0, 0x0, 16, NULL, NULL},                                   \
        gdma_ring_interrupt_cause_NODE_MAC( 1, _tileId, 0, 0),                          \
        gdma_ring_interrupt_cause_NODE_MAC( 2, _tileId, 0, 1),                          \
        gdma_ring_interrupt_cause_NODE_MAC( 3, _tileId, 0, 2),                          \
        gdma_ring_interrupt_cause_NODE_MAC( 4, _tileId, 0, 3),                          \
        gdma_ring_interrupt_cause_NODE_MAC( 5, _tileId, 0, 4),                          \
        gdma_ring_interrupt_cause_NODE_MAC( 6, _tileId, 0, 5),                          \
        gdma_ring_interrupt_cause_NODE_MAC( 7, _tileId, 0, 6),                          \
        gdma_ring_interrupt_cause_NODE_MAC( 8, _tileId, 0, 7),                          \
        gdma_ring_interrupt_cause_NODE_MAC( 9, _tileId, 0, 8),                          \
        gdma_ring_interrupt_cause_NODE_MAC(10, _tileId, 0, 9),                          \
        gdma_ring_interrupt_cause_NODE_MAC(11, _tileId, 0, 10),                         \
        gdma_ring_interrupt_cause_NODE_MAC(12, _tileId, 0, 11),                         \
        gdma_ring_interrupt_cause_NODE_MAC(13, _tileId, 0, 12),                         \
        gdma_ring_interrupt_cause_NODE_MAC(14, _tileId, 0, 13),                         \
        gdma_ring_interrupt_cause_NODE_MAC(15, _tileId, 0, 14),                         \
        gdma_ring_interrupt_cause_NODE_MAC(16, _tileId, 0, 15)

#define gdma_1_IntsSum_SUB_TREE_MAC(_tileId, bit, _treeId)                                              \
    /* GDMA 1 Interrupts Summary Cause */                                                               \
    {bit, GT_FALSE, CNM_INTERRUPTS_UNIT(_tileId), NULL,                                                        \
        CNM_RFU_INT_ADDR_MAC(0x000001c0,0x000001c4, _treeId),                                           \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AAS_CNM_CNM_TILE_##_tileId##_TREE_##_treeId##_GDMA_1_INTERRUPTS_SUMMARY_CAUSE_GDMA_1_INT_SUM_0_E,         \
        PRV_CPSS_AAS_CNM_CNM_TILE_##_tileId##_TREE_##_treeId##_GDMA_1_INTERRUPTS_SUMMARY_CAUSE_GDMA_1_INT_SUM_30_E,        \
        FILLED_IN_RUNTIME_CNS, 0, 0x0, 16, NULL, NULL},                                  \
        gdma_ring_interrupt_cause_NODE_MAC( 1, _tileId, 0, 16),                          \
        gdma_ring_interrupt_cause_NODE_MAC( 2, _tileId, 0, 17),                          \
        gdma_ring_interrupt_cause_NODE_MAC( 3, _tileId, 0, 18),                          \
        gdma_ring_interrupt_cause_NODE_MAC( 4, _tileId, 0, 19),                          \
        gdma_ring_interrupt_cause_NODE_MAC( 5, _tileId, 0, 20),                          \
        gdma_ring_interrupt_cause_NODE_MAC( 6, _tileId, 0, 21),                          \
        gdma_ring_interrupt_cause_NODE_MAC( 7, _tileId, 0, 22),                          \
        gdma_ring_interrupt_cause_NODE_MAC( 8, _tileId, 0, 23),                          \
        gdma_ring_interrupt_cause_NODE_MAC( 9, _tileId, 0, 24),                          \
        gdma_ring_interrupt_cause_NODE_MAC(10, _tileId, 0, 25),                          \
        gdma_ring_interrupt_cause_NODE_MAC(11, _tileId, 0, 26),                         \
        gdma_ring_interrupt_cause_NODE_MAC(12, _tileId, 0, 27),                         \
        gdma_ring_interrupt_cause_NODE_MAC(13, _tileId, 0, 28),                         \
        gdma_ring_interrupt_cause_NODE_MAC(14, _tileId, 0, 29),                         \
        gdma_ring_interrupt_cause_NODE_MAC(15, _tileId, 0, 30),                         \
        gdma_ring_interrupt_cause_NODE_MAC(16, _tileId, 0, 31)

#define gdma_2_IntsSum_SUB_TREE_MAC(_tileId, bit, _treeId)                              \
    /* GDMA 2 Interrupts Summary Cause */                                               \
    {bit, GT_FALSE, CNM_INTERRUPTS_UNIT(_tileId), NULL,                                        \
        CNM_RFU_INT_ADDR_MAC(0x000001c8,0x000001cc, _treeId),                           \
        prvCpssDrvHwPpPortGroupIsrRead,                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                \
        PRV_CPSS_AAS_CNM_CNM_TILE_##_tileId##_TREE_##_treeId##_GDMA_2_INTERRUPTS_SUMMARY_CAUSE_GDMA_2_INT_SUM_0_E,         \
        PRV_CPSS_AAS_CNM_CNM_TILE_##_tileId##_TREE_##_treeId##_GDMA_2_INTERRUPTS_SUMMARY_CAUSE_GDMA_2_INT_SUM_16_E,        \
        FILLED_IN_RUNTIME_CNS, 0, 0x0, 16, NULL, NULL},                                  \
        gdma_ring_interrupt_cause_NODE_MAC( 1, _tileId, 1, 0),                          \
        gdma_ring_interrupt_cause_NODE_MAC( 2, _tileId, 1, 1),                          \
        gdma_ring_interrupt_cause_NODE_MAC( 3, _tileId, 1, 2),                          \
        gdma_ring_interrupt_cause_NODE_MAC( 4, _tileId, 1, 3),                          \
        gdma_ring_interrupt_cause_NODE_MAC( 5, _tileId, 1, 4),                          \
        gdma_ring_interrupt_cause_NODE_MAC( 6, _tileId, 1, 5),                          \
        gdma_ring_interrupt_cause_NODE_MAC( 7, _tileId, 1, 6),                          \
        gdma_ring_interrupt_cause_NODE_MAC( 8, _tileId, 1, 7),                          \
        gdma_ring_interrupt_cause_NODE_MAC( 9, _tileId, 1, 8),                          \
        gdma_ring_interrupt_cause_NODE_MAC(10, _tileId, 1, 9),                          \
        gdma_ring_interrupt_cause_NODE_MAC(11, _tileId, 1, 10),                         \
        gdma_ring_interrupt_cause_NODE_MAC(12, _tileId, 1, 11),                         \
        gdma_ring_interrupt_cause_NODE_MAC(13, _tileId, 1, 12),                         \
        gdma_ring_interrupt_cause_NODE_MAC(14, _tileId, 1, 13),                         \
        gdma_ring_interrupt_cause_NODE_MAC(15, _tileId, 1, 14),                         \
        gdma_ring_interrupt_cause_NODE_MAC(16, _tileId, 1, 15)

#define gdma_3_IntsSum_SUB_TREE_MAC(_tileId, bit, _treeId)                              \
    /* GDMA 3 Interrupts Summary Cause */                                               \
    {bit, GT_FALSE, CNM_INTERRUPTS_UNIT(_tileId), NULL,                                        \
        CNM_RFU_INT_ADDR_MAC(0x000001d0,0x000001d4, _treeId),                           \
        prvCpssDrvHwPpPortGroupIsrRead,                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                \
        PRV_CPSS_AAS_CNM_CNM_TILE_##_tileId##_TREE_##_treeId##_GDMA_3_INTERRUPTS_SUMMARY_CAUSE_GDMA_3_INT_SUM_0_E,         \
        PRV_CPSS_AAS_CNM_CNM_TILE_##_tileId##_TREE_##_treeId##_GDMA_3_INTERRUPTS_SUMMARY_CAUSE_GDMA_3_INT_SUM_16_E,        \
        FILLED_IN_RUNTIME_CNS, 0, 0x0, 16, NULL, NULL},                                 \
        gdma_ring_interrupt_cause_NODE_MAC( 1, _tileId, 1, 16),                          \
        gdma_ring_interrupt_cause_NODE_MAC( 2, _tileId, 1, 17),                          \
        gdma_ring_interrupt_cause_NODE_MAC( 3, _tileId, 1, 18),                          \
        gdma_ring_interrupt_cause_NODE_MAC( 4, _tileId, 1, 19),                          \
        gdma_ring_interrupt_cause_NODE_MAC( 5, _tileId, 1, 20),                          \
        gdma_ring_interrupt_cause_NODE_MAC( 6, _tileId, 1, 21),                          \
        gdma_ring_interrupt_cause_NODE_MAC( 7, _tileId, 1, 22),                          \
        gdma_ring_interrupt_cause_NODE_MAC( 8, _tileId, 1, 23),                          \
        gdma_ring_interrupt_cause_NODE_MAC( 9, _tileId, 1, 24),                          \
        gdma_ring_interrupt_cause_NODE_MAC(10, _tileId, 1, 25),                          \
        gdma_ring_interrupt_cause_NODE_MAC(11, _tileId, 1, 26),                         \
        gdma_ring_interrupt_cause_NODE_MAC(12, _tileId, 1, 27),                         \
        gdma_ring_interrupt_cause_NODE_MAC(13, _tileId, 1, 28),                         \
        gdma_ring_interrupt_cause_NODE_MAC(14, _tileId, 1, 29),                         \
        gdma_ring_interrupt_cause_NODE_MAC(15, _tileId, 1, 30),                         \
        gdma_ring_interrupt_cause_NODE_MAC(16, _tileId, 1, 31)

#define gdma_4_IntsSum_SUB_TREE_MAC(_tileId, bit, _treeId)                              \
    /* GDMA 4 Interrupts Summary Cause */                                               \
    {bit, GT_FALSE, CNM_INTERRUPTS_UNIT(_tileId), NULL,                                        \
        CNM_RFU_INT_ADDR_MAC(0x000001d8, 0x000001dc, _treeId),                          \
        prvCpssDrvHwPpPortGroupIsrRead,                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                \
        PRV_CPSS_AAS_CNM_CNM_TILE_##_tileId##_TREE_##_treeId##_GDMA_4_INTERRUPTS_SUMMARY_CAUSE_GDMA_4_INT_SUM_0_E,         \
        PRV_CPSS_AAS_CNM_CNM_TILE_##_tileId##_TREE_##_treeId##_GDMA_4_INTERRUPTS_SUMMARY_CAUSE_GDMA_4_INT_SUM_16_E,        \
        FILLED_IN_RUNTIME_CNS, 0, 0x0, 16, NULL, NULL},                                  \
        gdma_ring_interrupt_cause_NODE_MAC( 1, _tileId, 2, 0),                          \
        gdma_ring_interrupt_cause_NODE_MAC( 2, _tileId, 2, 1),                          \
        gdma_ring_interrupt_cause_NODE_MAC( 3, _tileId, 2, 2),                          \
        gdma_ring_interrupt_cause_NODE_MAC( 4, _tileId, 2, 3),                          \
        gdma_ring_interrupt_cause_NODE_MAC( 5, _tileId, 2, 4),                          \
        gdma_ring_interrupt_cause_NODE_MAC( 6, _tileId, 2, 5),                          \
        gdma_ring_interrupt_cause_NODE_MAC( 7, _tileId, 2, 6),                          \
        gdma_ring_interrupt_cause_NODE_MAC( 8, _tileId, 2, 7),                          \
        gdma_ring_interrupt_cause_NODE_MAC( 9, _tileId, 2, 8),                          \
        gdma_ring_interrupt_cause_NODE_MAC(10, _tileId, 2, 9),                          \
        gdma_ring_interrupt_cause_NODE_MAC(11, _tileId, 2, 10),                         \
        gdma_ring_interrupt_cause_NODE_MAC(12, _tileId, 2, 11),                         \
        gdma_ring_interrupt_cause_NODE_MAC(13, _tileId, 2, 12),                         \
        gdma_ring_interrupt_cause_NODE_MAC(14, _tileId, 2, 13),                         \
        gdma_ring_interrupt_cause_NODE_MAC(15, _tileId, 2, 14),                         \
        gdma_ring_interrupt_cause_NODE_MAC(16, _tileId, 2, 15)

#define gdma_5_IntsSum_SUB_TREE_MAC(_tileId, bit, _treeId)                              \
    /* GDMA 5 Interrupts Summary Cause */                                               \
    {bit, GT_FALSE, CNM_INTERRUPTS_UNIT(_tileId), NULL,                                        \
        CNM_RFU_INT_ADDR_MAC(0x000001e0, 0x000001e4, _treeId),                          \
        prvCpssDrvHwPpPortGroupIsrRead,                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                \
        PRV_CPSS_AAS_CNM_CNM_TILE_##_tileId##_TREE_##_treeId##_GDMA_5_INTERRUPTS_SUMMARY_CAUSE_GDMA_5_INT_SUM_0_E,         \
        PRV_CPSS_AAS_CNM_CNM_TILE_##_tileId##_TREE_##_treeId##_GDMA_5_INTERRUPTS_SUMMARY_CAUSE_GDMA_5_INT_SUM_16_E,        \
        FILLED_IN_RUNTIME_CNS, 0, 0x0, 16, NULL, NULL},                                                                     \
        gdma_ring_interrupt_cause_NODE_MAC( 1, _tileId, 2, 16),                          \
        gdma_ring_interrupt_cause_NODE_MAC( 2, _tileId, 2, 17),                          \
        gdma_ring_interrupt_cause_NODE_MAC( 3, _tileId, 2, 18),                          \
        gdma_ring_interrupt_cause_NODE_MAC( 4, _tileId, 2, 19),                          \
        gdma_ring_interrupt_cause_NODE_MAC( 5, _tileId, 2, 20),                          \
        gdma_ring_interrupt_cause_NODE_MAC( 6, _tileId, 2, 21),                          \
        gdma_ring_interrupt_cause_NODE_MAC( 7, _tileId, 2, 22),                          \
        gdma_ring_interrupt_cause_NODE_MAC( 8, _tileId, 2, 23),                          \
        gdma_ring_interrupt_cause_NODE_MAC( 9, _tileId, 2, 24),                          \
        gdma_ring_interrupt_cause_NODE_MAC(10, _tileId, 2, 25),                          \
        gdma_ring_interrupt_cause_NODE_MAC(11, _tileId, 2, 26),                         \
        gdma_ring_interrupt_cause_NODE_MAC(12, _tileId, 2, 27),                         \
        gdma_ring_interrupt_cause_NODE_MAC(13, _tileId, 2, 28),                         \
        gdma_ring_interrupt_cause_NODE_MAC(14, _tileId, 2, 29),                         \
        gdma_ring_interrupt_cause_NODE_MAC(15, _tileId, 2, 30),                         \
        gdma_ring_interrupt_cause_NODE_MAC(16, _tileId, 2, 31)

#define gdma_6_IntsSum_SUB_TREE_MAC(_tileId, bit, _treeId)                              \
    /* GDMA 6 Interrupts Summary Cause */                                               \
    {bit, GT_FALSE, CNM_INTERRUPTS_UNIT(_tileId), NULL,                                        \
        CNM_RFU_INT_ADDR_MAC(0x000001E8, 0x000001EC, _treeId),                          \
        prvCpssDrvHwPpPortGroupIsrRead,                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                \
        PRV_CPSS_AAS_CNM_CNM_TILE_##_tileId##_TREE_##_treeId##_GDMA_6_INTERRUPTS_SUMMARY_CAUSE_GDMA_6_INT_SUM_0_E,         \
        PRV_CPSS_AAS_CNM_CNM_TILE_##_tileId##_TREE_##_treeId##_GDMA_6_INTERRUPTS_SUMMARY_CAUSE_GDMA_6_INT_SUM_16_E,        \
        FILLED_IN_RUNTIME_CNS, 0, 0x0, 16, NULL, NULL},                                  \
        gdma_ring_interrupt_cause_NODE_MAC( 1, _tileId, 3, 0),                          \
        gdma_ring_interrupt_cause_NODE_MAC( 2, _tileId, 3, 1),                          \
        gdma_ring_interrupt_cause_NODE_MAC( 3, _tileId, 3, 2),                          \
        gdma_ring_interrupt_cause_NODE_MAC( 4, _tileId, 3, 3),                          \
        gdma_ring_interrupt_cause_NODE_MAC( 5, _tileId, 3, 4),                          \
        gdma_ring_interrupt_cause_NODE_MAC( 6, _tileId, 3, 5),                          \
        gdma_ring_interrupt_cause_NODE_MAC( 7, _tileId, 3, 6),                          \
        gdma_ring_interrupt_cause_NODE_MAC( 8, _tileId, 3, 7),                          \
        gdma_ring_interrupt_cause_NODE_MAC( 9, _tileId, 3, 8),                          \
        gdma_ring_interrupt_cause_NODE_MAC(10, _tileId, 3, 9),                          \
        gdma_ring_interrupt_cause_NODE_MAC(11, _tileId, 3, 10),                         \
        gdma_ring_interrupt_cause_NODE_MAC(12, _tileId, 3, 11),                         \
        gdma_ring_interrupt_cause_NODE_MAC(13, _tileId, 3, 12),                         \
        gdma_ring_interrupt_cause_NODE_MAC(14, _tileId, 3, 13),                         \
        gdma_ring_interrupt_cause_NODE_MAC(15, _tileId, 3, 14),                         \
        gdma_ring_interrupt_cause_NODE_MAC(16, _tileId, 3, 15)

#define gdma_7_IntsSum_SUB_TREE_MAC(_tileId, bit, _treeId)                              \
    /* GDMA 7 Interrupts Summary Cause */                                               \
    {bit, GT_FALSE, CNM_INTERRUPTS_UNIT(_tileId), NULL,                                        \
        CNM_RFU_INT_ADDR_MAC(0x000001f0, 0x000001f4, _treeId),                          \
        prvCpssDrvHwPpPortGroupIsrRead,                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                \
        PRV_CPSS_AAS_CNM_CNM_TILE_##_tileId##_TREE_##_treeId##_GDMA_7_INTERRUPTS_SUMMARY_CAUSE_GDMA_7_INT_SUM_0_E,         \
        PRV_CPSS_AAS_CNM_CNM_TILE_##_tileId##_TREE_##_treeId##_GDMA_7_INTERRUPTS_SUMMARY_CAUSE_GDMA_7_INT_SUM_16_E,        \
        FILLED_IN_RUNTIME_CNS, 0, 0x0, 16, NULL, NULL},                                                                     \
        gdma_ring_interrupt_cause_NODE_MAC( 1, _tileId, 3, 16),                          \
        gdma_ring_interrupt_cause_NODE_MAC( 2, _tileId, 3, 17),                          \
        gdma_ring_interrupt_cause_NODE_MAC( 3, _tileId, 3, 18),                          \
        gdma_ring_interrupt_cause_NODE_MAC( 4, _tileId, 3, 19),                          \
        gdma_ring_interrupt_cause_NODE_MAC( 5, _tileId, 3, 20),                          \
        gdma_ring_interrupt_cause_NODE_MAC( 6, _tileId, 3, 21),                          \
        gdma_ring_interrupt_cause_NODE_MAC( 7, _tileId, 3, 22),                          \
        gdma_ring_interrupt_cause_NODE_MAC( 8, _tileId, 3, 23),                          \
        gdma_ring_interrupt_cause_NODE_MAC( 9, _tileId, 3, 24),                          \
        gdma_ring_interrupt_cause_NODE_MAC(10, _tileId, 3, 25),                          \
        gdma_ring_interrupt_cause_NODE_MAC(11, _tileId, 3, 26),                         \
        gdma_ring_interrupt_cause_NODE_MAC(12, _tileId, 3, 27),                         \
        gdma_ring_interrupt_cause_NODE_MAC(13, _tileId, 3, 28),                         \
        gdma_ring_interrupt_cause_NODE_MAC(14, _tileId, 3, 29),                         \
        gdma_ring_interrupt_cause_NODE_MAC(15, _tileId, 3, 30),                         \
        gdma_ring_interrupt_cause_NODE_MAC(16, _tileId, 3, 31)

#define cnm_aggregation_IntsSum_SUB_TREE_MAC(_tileId, bit, _treeId)                                     \
    /*         CnM Aggregation Interrupts Summary Cause */                                                     \
    {bit, GT_FALSE, CNM_INTERRUPTS_UNIT(_tileId), NULL,                                                        \
        CNM_RFU_INT_ADDR_MAC(0x000001a0, 0x000001a4, _treeId),                                          \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_AAS_CNM_CNM_TILE_##_tileId##_TREE_##_treeId##_CN_M_AGGREGATION_INTERRUPTS_SUMMARY_CAUSE_GLOBAL_SUM_CN_M_0_INT_SUM_E,    \
        PRV_CPSS_AAS_CNM_CNM_TILE_##_tileId##_TREE_##_treeId##_CN_M_AGGREGATION_INTERRUPTS_SUMMARY_CAUSE_GLOBAL_CM7_6_INT_SUM_E,   \
        FILLED_IN_RUNTIME_CNS, 0, 0x0, 11, NULL, NULL},                                                  \
        cnm_1_IntsSum_SUB_TREE_MAC(0/*tile*/,2/*bit*/, 0 /*_tree*/),                                    \
        cnm_2_IntsSum_SUB_TREE_MAC(0/*tile*/,3/*bit*/, 0 /*_tree*/),                                    \
        cnm_3_IntsSum_SUB_TREE_MAC(0/*tile*/,4/*bit*/, 0 /*_tree*/),                                    \
        /* GDMA Interrupts Summary Cause */                                                             \
        gdma_0_IntsSum_SUB_TREE_MAC(0/*tile*/, 5/*bit*/, 0 /*_tree*/),                                  \
        gdma_1_IntsSum_SUB_TREE_MAC(0/*tile*/, 6/*bit*/, 0 /*_tree*/),                                  \
        gdma_2_IntsSum_SUB_TREE_MAC(0/*tile*/, 7/*bit*/, 0 /*_tree*/),                                  \
        gdma_3_IntsSum_SUB_TREE_MAC(0/*tile*/, 8/*bit*/, 0 /*_tree*/),                                  \
        gdma_4_IntsSum_SUB_TREE_MAC(0/*tile*/, 9/*bit*/, 0 /*_tree*/),                                  \
        gdma_5_IntsSum_SUB_TREE_MAC(0/*tile*/,10/*bit*/, 0 /*_tree*/),                                  \
        gdma_6_IntsSum_SUB_TREE_MAC(0/*tile*/,11/*bit*/, 0 /*_tree*/),                                  \
        gdma_7_IntsSum_SUB_TREE_MAC(0/*tile*/,12/*bit*/, 0 /*_tree*/)

static const PRV_CPSS_DRV_INTERRUPT_SCAN_STC aas1IntrScanArr[] =
{
    /* Global Interrupt Cause */
    AAS_CNM_RFU_GLOBAL_INTERRUPT_SUMMARY_MAC(0/*tile*/,0/*bit*/,  13 /*numChild*/, 0 /*_tree*/),
        cnm_aggregation_IntsSum_SUB_TREE_MAC(0/*tile*/,1/*bit*/, 0 /*_tree*/),
        /* High Priority Coalescing Bypass Interrupts Summary Cause  */
        high_priority_coalescing_bypass_0_IntsSum_SUB_TREE_MAC(0/*tile*/,2/*bit*/, 0 /*_tree*/),
        high_priority_coalescing_bypass_1_IntsSum_SUB_TREE_MAC(0/*tile*/,3/*bit*/, 0 /*_tree*/),
        /* Units Interrupts Summary Cause */
        units_0_IntsSum_SUB_TREE_MAC(0/*tile*/, 4/*bit*/, 0 /*_tree*/),
        units_1_IntsSum_SUB_TREE_MAC(0/*tile*/, 5/*bit*/, 0 /*_tree*/),
        units_2_IntsSum_SUB_TREE_MAC(0/*tile*/, 6/*bit*/, 0 /*_tree*/),
        units_3_IntsSum_SUB_TREE_MAC(0/*tile*/, 7/*bit*/, 0 /*_tree*/),
        units_4_IntsSum_SUB_TREE_MAC(0/*tile*/, 8/*bit*/, 0 /*_tree*/),
        units_5_IntsSum_SUB_TREE_MAC(0/*tile*/, 9/*bit*/, 0 /*_tree*/),
        units_6_IntsSum_SUB_TREE_MAC(0/*tile*/,10/*bit*/, 0 /*_tree*/),
        units_7_IntsSum_SUB_TREE_MAC(0/*tile*/,11/*bit*/, 0 /*_tree*/),
        units_8_IntsSum_SUB_TREE_MAC(0/*tile*/,12/*bit*/, 0 /*_tree*/),
        units_9_IntsSum_SUB_TREE_MAC(0/*tile*/,13/*bit*/, 0 /*_tree*/),
};

/* number of elements in the array of aas1IntrScanArr[] */
#define AAS1_NUM_ELEMENTS_IN_SCAN_TREE_CNS \
    NUM_ELEMENTS_IN_ARR_MAC(aas1IntrScanArr)

#define AAS_NUM_MASK_REGISTERS_CNS  (PRV_CPSS_AAS_LAST_INT_E / 32)

/* mark extData as per pipe per tile port interrupt */
#define AAS_SET_EVENT_PER_PORT_MAC(_tile,_instance, _postFix, _portNum)   \
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_MAC_INSTANCE_##_instance##_PORT_NUM_##_portNum##_##_postFix,      ((_tile)*80 + (_instance) * 8 + (_portNum))

#define AAS_SET_EVENT_PER_SEG_PORT_MAC(_tile,_instance, _postFix, _portNum)   \
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_MAC_INSTANCE_##_instance##_SEG_PORT_NUM_##_portNum##_##_postFix,  ((_tile)*80 + (_instance) * 8 + (_portNum*4))

/* _postFix - must include the "_E" */
#define AAS_SET_EVENT_WITH_INDEX_MAC(_tile,_postFix, _index)             \
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_##_postFix,                   _index

/* _postFix - must include the "_E" */
#define AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,_postFix)                         \
    SET_INDEX_EQUAL_EVENT_MAC(INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_##_postFix)

/* _postFix - must include the "_E" */
#define AAS_SET_EVENT_PER_INDEX_PER_REGISTER_PER_PORT_MAC(_tile,_preFix, _index, _port, _postFix)     \
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_##_preFix##_REG_##_index##_PORT_##_port##_##_postFix,    (_index * 16) + _port

/* _postFix - must include the "_E" */
#define SET_EVENT_INDEX_PER_SOURCE_PER_DIRECTION_PER_DP_MAC(_tile,_postFix, _type, _prefix, _direction, _dp, _source)   \
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_##_type##_##_prefix##_##_dp##_##_postFix,            (_source << 8 |(((_dp) * 2) + _direction))

#define SET_EVENT_SOURCE_PER_DIRECTION_ALL_DPS_MAC(_tile,_postFix, _type, _prefix, _direction, _source)           \
    SET_EVENT_INDEX_PER_SOURCE_PER_DIRECTION_PER_DP_MAC(_tile,_postFix, _type, _prefix, _direction, 0, _source), \
    SET_EVENT_INDEX_PER_SOURCE_PER_DIRECTION_PER_DP_MAC(_tile,_postFix, _type, _prefix, _direction, 1, _source)

/* _postFix - must include the "_E" */
#define AAS_SET_EIP_164_EVENT_SOURCE_PER_ALL_DIRECTIONS_ALL_DP_MAC(_tile,_postFix, _source)  \
    SET_EVENT_SOURCE_PER_DIRECTION_ALL_DPS_MAC(_tile,_postFix, EIP164E_MACSEC_TYPE_3, EIP164E_MACSEC_INSTANCE, 0, _source),     \
    SET_EVENT_SOURCE_PER_DIRECTION_ALL_DPS_MAC(_tile,_postFix, EIP164I_MACSEC_TYPE_2, EIP164I_MACSEC_INSTANCE, 1, _source)

/* _postFix - must include the "_E" */
#define AAS_SET_EIP_163_EVENT_SOURCE_PER_ALL_DIRECTIONS_ALL_DP_MAC(_tile,_postFix, _source)  \
    SET_EVENT_SOURCE_PER_DIRECTION_ALL_DPS_MAC(_tile,_postFix, EIP163E_MACSEC_TYPE_1, EIP163E_MACSEC_INSTANCE, 0, _source),     \
    SET_EVENT_SOURCE_PER_DIRECTION_ALL_DPS_MAC(_tile,_postFix, EIP163I_MACSEC_TYPE_0, EIP163I_MACSEC_INSTANCE, 1, _source)

/* _postFix - must include the "_E" */
#define SET_EVENT_INDEX_PER_DIRECTION_PER_DP_MAC(_tile,_postFix, _type, _prefix, _direction, _dp)   \
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_##_type##_##_prefix##_##_dp##_##_postFix,           (((_dp) << 1) + _direction)

#define SET_EVENT_PER_DIRECTION_ALL_DPS_MAC(_tile,_postFix, _type, _prefix, _direction)       \
    SET_EVENT_INDEX_PER_DIRECTION_PER_DP_MAC(_tile,_postFix, _type, _prefix, _direction, 0), \
    SET_EVENT_INDEX_PER_DIRECTION_PER_DP_MAC(_tile,_postFix, _type, _prefix, _direction, 1)

/* _postFix - must include the "_E" */
#define AAS_SET_EIP_164_EVENT_PER_EGRESS_DIRECTION_ALL_DP_MAC(_tile,_postFix)  \
    SET_EVENT_PER_DIRECTION_ALL_DPS_MAC(_tile,_postFix, EIP164E_MACSEC_TYPE_3, EIP164E_MACSEC_INSTANCE, 0)

/* _postFix - must include the "_E" */
#define AAS_SET_EIP_164_EVENT_SOURCE_PER_INGRESS_DIRECTION_ALL_DP_MAC(_tile,_postFix, _source)  \
    SET_EVENT_SOURCE_PER_DIRECTION_ALL_DPS_MAC(_tile,_postFix, EIP164I_MACSEC_TYPE_2, EIP164I_MACSEC_INSTANCE, 1, _source)

/* _postFix - must include the "_E" */
#define AAS_SET_EIP_66_EVENT_PER_EGRESS_DIRECTION_ALL_DP_MAC(_tile,_postFix)  \
    SET_EVENT_PER_DIRECTION_ALL_DPS_MAC(_tile,_postFix, EIP66E_MACSEC_TYPE_3, EIP66E_MACSEC_INSTANCE, 0)

/* _postFix - must include the "_E" */
#define AAS_SET_EIP_164_EVENT_PER_ALL_DIRECTIONS_ALL_DP_MAC(_tile,_postFix)  \
    SET_EVENT_PER_DIRECTION_ALL_DPS_MAC(_tile,_postFix, EIP164E_MACSEC_TYPE_3, EIP164E_MACSEC_INSTANCE, 0),      \
    SET_EVENT_PER_DIRECTION_ALL_DPS_MAC(_tile,_postFix, EIP164I_MACSEC_TYPE_2, EIP164I_MACSEC_INSTANCE, 1)

#define SET_EVENT_ALL_DIRECTIONS_PER_DP_MAC(_tile,_type, _prefix, _index, _direction, _dp)   \
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_##_type##_##_prefix##_##_dp##_GLOBAL_MAC_SEC_INTERRUPT_CAUSE_ERROR_NOTIFICATION_NUMBER_##_index##_E,  (_index << 8 |(((_dp) * 2) + _direction))

#define SET_EVENT_ALL_DIRECTIONS_ALL_DP_MAC(_tile,_type, _prefix, _index, _direction)            \
    SET_EVENT_ALL_DIRECTIONS_PER_DP_MAC(_tile,_type, _prefix, _index, _direction, 0),             \
    SET_EVENT_ALL_DIRECTIONS_PER_DP_MAC(_tile,_type, _prefix, _index, _direction, 1)

#define AAS_SET_EIP_66_ERROR_EVENT_PER_ALL_DIRECTIONS_ALL_DP_MAC(_tile,_index)    \
    SET_EVENT_ALL_DIRECTIONS_ALL_DP_MAC(_tile,EIP66E_MACSEC_TYPE_3, EIP66E_MACSEC_INSTANCE, _index, 0),             \
    SET_EVENT_ALL_DIRECTIONS_ALL_DP_MAC(_tile,EIP66I_MACSEC_TYPE_2, EIP66I_MACSEC_INSTANCE, _index, 1)

#define AAS_SET_EQ_EVENT_PER_16_PORTS_PER_REGISTER_MAC(_tile,_index, _postFix)          \
    AAS_SET_EVENT_PER_INDEX_PER_REGISTER_PER_PORT_MAC(_tile,EQ, _index,  0, _postFix),            \
    AAS_SET_EVENT_PER_INDEX_PER_REGISTER_PER_PORT_MAC(_tile,EQ, _index,  1, _postFix),            \
    AAS_SET_EVENT_PER_INDEX_PER_REGISTER_PER_PORT_MAC(_tile,EQ, _index,  2, _postFix),            \
    AAS_SET_EVENT_PER_INDEX_PER_REGISTER_PER_PORT_MAC(_tile,EQ, _index,  3, _postFix),            \
    AAS_SET_EVENT_PER_INDEX_PER_REGISTER_PER_PORT_MAC(_tile,EQ, _index,  4, _postFix),            \
    AAS_SET_EVENT_PER_INDEX_PER_REGISTER_PER_PORT_MAC(_tile,EQ, _index,  5, _postFix),            \
    AAS_SET_EVENT_PER_INDEX_PER_REGISTER_PER_PORT_MAC(_tile,EQ, _index,  6, _postFix),            \
    AAS_SET_EVENT_PER_INDEX_PER_REGISTER_PER_PORT_MAC(_tile,EQ, _index,  7, _postFix),            \
    AAS_SET_EVENT_PER_INDEX_PER_REGISTER_PER_PORT_MAC(_tile,EQ, _index,  8, _postFix),            \
    AAS_SET_EVENT_PER_INDEX_PER_REGISTER_PER_PORT_MAC(_tile,EQ, _index,  9, _postFix),            \
    AAS_SET_EVENT_PER_INDEX_PER_REGISTER_PER_PORT_MAC(_tile,EQ, _index, 10, _postFix),            \
    AAS_SET_EVENT_PER_INDEX_PER_REGISTER_PER_PORT_MAC(_tile,EQ, _index, 11, _postFix),            \
    AAS_SET_EVENT_PER_INDEX_PER_REGISTER_PER_PORT_MAC(_tile,EQ, _index, 12, _postFix),            \
    AAS_SET_EVENT_PER_INDEX_PER_REGISTER_PER_PORT_MAC(_tile,EQ, _index, 13, _postFix),            \
    AAS_SET_EVENT_PER_INDEX_PER_REGISTER_PER_PORT_MAC(_tile,EQ, _index, 14, _postFix),            \
    AAS_SET_EVENT_PER_INDEX_PER_REGISTER_PER_PORT_MAC(_tile,EQ, _index, 15, _postFix)


#define AAS_SET_EVENT_PER_ALL_PORTS_ALL_REGISTERS_MAC(_tile,_postFix)                             \
    AAS_SET_EQ_EVENT_PER_16_PORTS_PER_REGISTER_MAC(_tile, 1, _postFix),                           \
    AAS_SET_EQ_EVENT_PER_16_PORTS_PER_REGISTER_MAC(_tile, 2, _postFix),                           \
    AAS_SET_EQ_EVENT_PER_16_PORTS_PER_REGISTER_MAC(_tile, 3, _postFix),                           \
    AAS_SET_EQ_EVENT_PER_16_PORTS_PER_REGISTER_MAC(_tile, 4, _postFix),                           \
    AAS_SET_EQ_EVENT_PER_16_PORTS_PER_REGISTER_MAC(_tile, 5, _postFix),                           \
    AAS_SET_EQ_EVENT_PER_16_PORTS_PER_REGISTER_MAC(_tile, 6, _postFix),                           \
    AAS_SET_EQ_EVENT_PER_16_PORTS_PER_REGISTER_MAC(_tile, 7, _postFix),                           \
    AAS_SET_EQ_EVENT_PER_16_PORTS_PER_REGISTER_MAC(_tile, 8, _postFix),                           \
    AAS_SET_EQ_EVENT_PER_16_PORTS_PER_REGISTER_MAC(_tile, 9, _postFix),                           \
    AAS_SET_EQ_EVENT_PER_16_PORTS_PER_REGISTER_MAC(_tile,10, _postFix),                           \
    AAS_SET_EQ_EVENT_PER_16_PORTS_PER_REGISTER_MAC(_tile,11, _postFix),                           \
    AAS_SET_EQ_EVENT_PER_16_PORTS_PER_REGISTER_MAC(_tile,12, _postFix),                           \
    AAS_SET_EQ_EVENT_PER_16_PORTS_PER_REGISTER_MAC(_tile,13, _postFix),                           \
    AAS_SET_EQ_EVENT_PER_16_PORTS_PER_REGISTER_MAC(_tile,14, _postFix),                           \
    AAS_SET_EQ_EVENT_PER_16_PORTS_PER_REGISTER_MAC(_tile,15, _postFix),                           \
    AAS_SET_EQ_EVENT_PER_16_PORTS_PER_REGISTER_MAC(_tile,16, _postFix)


#define AAS_SET_INDEX_EQUAL_EVENT_PER_MTI_PORTS(_tile,index,_postFix)                             \
    AAS_SET_EVENT_PER_PORT_MAC(_tile,index, _postFix,      0),                                    \
    AAS_SET_EVENT_PER_PORT_MAC(_tile,index, _postFix,      1),                                    \
    AAS_SET_EVENT_PER_PORT_MAC(_tile,index, _postFix,      2),                                    \
    AAS_SET_EVENT_PER_PORT_MAC(_tile,index, _postFix,      3),                                    \
    AAS_SET_EVENT_PER_PORT_MAC(_tile,index, _postFix,      4),                                    \
    AAS_SET_EVENT_PER_PORT_MAC(_tile,index, _postFix,      5),                                    \
    AAS_SET_EVENT_PER_PORT_MAC(_tile,index, _postFix,      6),                                    \
    AAS_SET_EVENT_PER_PORT_MAC(_tile,index, _postFix,      7)

#define AAS_HALF_DP_SET_INDEX_EQUAL_EVENT_PER_MTI_PORTS(_tile,index,_postFix)                     \
    AAS_SET_EVENT_PER_PORT_MAC(_tile,index, _postFix,      0),                                    \
    AAS_SET_EVENT_PER_PORT_MAC(_tile,index, _postFix,      1),                                    \
    AAS_SET_EVENT_PER_PORT_MAC(_tile,index, _postFix,      2),                                    \
    AAS_SET_EVENT_PER_PORT_MAC(_tile,index, _postFix,      3)

#define AAS_SET_INDEX_EQUAL_EVENT_PER_SEG_PORTS(_tile,index,_postFix)                             \
    AAS_SET_EVENT_PER_SEG_PORT_MAC(_tile,index,_postFix,  0),                                     \
    AAS_SET_EVENT_PER_SEG_PORT_MAC(_tile,index,_postFix,  1)

#define AAS_HALF_DP_SET_INDEX_EQUAL_EVENT_PER_SEG_PORTS(_tile,index,_postFix)                     \
    AAS_SET_EVENT_PER_SEG_PORT_MAC(_tile,index,_postFix,  0)

/* The GDMA[1,2] support the GDMA for 'TO_CPU' */
#define AAS_ALL_GDMA_RX_INTERRUPTS_MAC(_tile, _type)    \
    SINGLE_GDMA_RX_INT_BIND_MAC(_tile, _type,  1/*local GDMA*/,   0/*globalQueue*/, 0/*localQueue*/),\
    SINGLE_GDMA_RX_INT_BIND_MAC(_tile, _type,  1/*local GDMA*/,   1/*globalQueue*/, 1/*localQueue*/),\
    SINGLE_GDMA_RX_INT_BIND_MAC(_tile, _type,  1/*local GDMA*/,   2/*globalQueue*/, 2/*localQueue*/),\
    SINGLE_GDMA_RX_INT_BIND_MAC(_tile, _type,  1/*local GDMA*/,   3/*globalQueue*/, 3/*localQueue*/),\
    SINGLE_GDMA_RX_INT_BIND_MAC(_tile, _type,  1/*local GDMA*/,   4/*globalQueue*/, 4/*localQueue*/),\
    SINGLE_GDMA_RX_INT_BIND_MAC(_tile, _type,  1/*local GDMA*/,   5/*globalQueue*/, 5/*localQueue*/),\
    SINGLE_GDMA_RX_INT_BIND_MAC(_tile, _type,  1/*local GDMA*/,   6/*globalQueue*/, 6/*localQueue*/),\
    SINGLE_GDMA_RX_INT_BIND_MAC(_tile, _type,  1/*local GDMA*/,   7/*globalQueue*/, 7/*localQueue*/),\
    SINGLE_GDMA_RX_INT_BIND_MAC(_tile, _type,  1/*local GDMA*/,   8/*globalQueue*/, 8/*localQueue*/),\
    SINGLE_GDMA_RX_INT_BIND_MAC(_tile, _type,  1/*local GDMA*/,   9/*globalQueue*/, 9/*localQueue*/),\
    SINGLE_GDMA_RX_INT_BIND_MAC(_tile, _type,  1/*local GDMA*/,  10/*globalQueue*/,10/*localQueue*/),\
    SINGLE_GDMA_RX_INT_BIND_MAC(_tile, _type,  1/*local GDMA*/,  11/*globalQueue*/,11/*localQueue*/),\
    SINGLE_GDMA_RX_INT_BIND_MAC(_tile, _type,  1/*local GDMA*/,  12/*globalQueue*/,12/*localQueue*/),\
    SINGLE_GDMA_RX_INT_BIND_MAC(_tile, _type,  1/*local GDMA*/,  13/*globalQueue*/,13/*localQueue*/),\
    SINGLE_GDMA_RX_INT_BIND_MAC(_tile, _type,  1/*local GDMA*/,  14/*globalQueue*/,14/*localQueue*/),\
    SINGLE_GDMA_RX_INT_BIND_MAC(_tile, _type,  1/*local GDMA*/,  15/*globalQueue*/,15/*localQueue*/),\
    SINGLE_GDMA_RX_INT_BIND_MAC(_tile, _type,  1/*local GDMA*/,  16/*globalQueue*/,16/*localQueue*/),\
    SINGLE_GDMA_RX_INT_BIND_MAC(_tile, _type,  1/*local GDMA*/,  17/*globalQueue*/,17/*localQueue*/),\
    SINGLE_GDMA_RX_INT_BIND_MAC(_tile, _type,  1/*local GDMA*/,  18/*globalQueue*/,18/*localQueue*/),\
    SINGLE_GDMA_RX_INT_BIND_MAC(_tile, _type,  1/*local GDMA*/,  19/*globalQueue*/,19/*localQueue*/),\
    SINGLE_GDMA_RX_INT_BIND_MAC(_tile, _type,  1/*local GDMA*/,  20/*globalQueue*/,20/*localQueue*/),\
    SINGLE_GDMA_RX_INT_BIND_MAC(_tile, _type,  1/*local GDMA*/,  21/*globalQueue*/,21/*localQueue*/),\
    SINGLE_GDMA_RX_INT_BIND_MAC(_tile, _type,  1/*local GDMA*/,  22/*globalQueue*/,22/*localQueue*/),\
    SINGLE_GDMA_RX_INT_BIND_MAC(_tile, _type,  1/*local GDMA*/,  23/*globalQueue*/,23/*localQueue*/),\
    SINGLE_GDMA_RX_INT_BIND_MAC(_tile, _type,  1/*local GDMA*/,  24/*globalQueue*/,24/*localQueue*/),\
    SINGLE_GDMA_RX_INT_BIND_MAC(_tile, _type,  1/*local GDMA*/,  25/*globalQueue*/,25/*localQueue*/),\
    SINGLE_GDMA_RX_INT_BIND_MAC(_tile, _type,  1/*local GDMA*/,  26/*globalQueue*/,26/*localQueue*/),\
    SINGLE_GDMA_RX_INT_BIND_MAC(_tile, _type,  1/*local GDMA*/,  27/*globalQueue*/,27/*localQueue*/),\
    SINGLE_GDMA_RX_INT_BIND_MAC(_tile, _type,  1/*local GDMA*/,  28/*globalQueue*/,28/*localQueue*/),\
    SINGLE_GDMA_RX_INT_BIND_MAC(_tile, _type,  1/*local GDMA*/,  29/*globalQueue*/,29/*localQueue*/),\
    SINGLE_GDMA_RX_INT_BIND_MAC(_tile, _type,  1/*local GDMA*/,  30/*globalQueue*/,30/*localQueue*/),\
    SINGLE_GDMA_RX_INT_BIND_MAC(_tile, _type,  1/*local GDMA*/,  31/*globalQueue*/,31/*localQueue*/),\
    \
    SINGLE_GDMA_RX_INT_BIND_MAC(_tile, _type,  2/*local GDMA*/,  32/*globalQueue*/, 0/*localQueue*/),\
    SINGLE_GDMA_RX_INT_BIND_MAC(_tile, _type,  2/*local GDMA*/,  33/*globalQueue*/, 1/*localQueue*/),\
    SINGLE_GDMA_RX_INT_BIND_MAC(_tile, _type,  2/*local GDMA*/,  34/*globalQueue*/, 2/*localQueue*/),\
    SINGLE_GDMA_RX_INT_BIND_MAC(_tile, _type,  2/*local GDMA*/,  35/*globalQueue*/, 3/*localQueue*/),\
    SINGLE_GDMA_RX_INT_BIND_MAC(_tile, _type,  2/*local GDMA*/,  36/*globalQueue*/, 4/*localQueue*/),\
    SINGLE_GDMA_RX_INT_BIND_MAC(_tile, _type,  2/*local GDMA*/,  37/*globalQueue*/, 5/*localQueue*/),\
    SINGLE_GDMA_RX_INT_BIND_MAC(_tile, _type,  2/*local GDMA*/,  38/*globalQueue*/, 6/*localQueue*/),\
    SINGLE_GDMA_RX_INT_BIND_MAC(_tile, _type,  2/*local GDMA*/,  39/*globalQueue*/, 7/*localQueue*/),\
    SINGLE_GDMA_RX_INT_BIND_MAC(_tile, _type,  2/*local GDMA*/,  40/*globalQueue*/, 8/*localQueue*/),\
    SINGLE_GDMA_RX_INT_BIND_MAC(_tile, _type,  2/*local GDMA*/,  41/*globalQueue*/, 9/*localQueue*/),\
    SINGLE_GDMA_RX_INT_BIND_MAC(_tile, _type,  2/*local GDMA*/,  42/*globalQueue*/,10/*localQueue*/),\
    SINGLE_GDMA_RX_INT_BIND_MAC(_tile, _type,  2/*local GDMA*/,  43/*globalQueue*/,11/*localQueue*/),\
    SINGLE_GDMA_RX_INT_BIND_MAC(_tile, _type,  2/*local GDMA*/,  44/*globalQueue*/,12/*localQueue*/),\
    SINGLE_GDMA_RX_INT_BIND_MAC(_tile, _type,  2/*local GDMA*/,  45/*globalQueue*/,13/*localQueue*/),\
    SINGLE_GDMA_RX_INT_BIND_MAC(_tile, _type,  2/*local GDMA*/,  46/*globalQueue*/,14/*localQueue*/),\
    SINGLE_GDMA_RX_INT_BIND_MAC(_tile, _type,  2/*local GDMA*/,  47/*globalQueue*/,15/*localQueue*/),\
    SINGLE_GDMA_RX_INT_BIND_MAC(_tile, _type,  2/*local GDMA*/,  48/*globalQueue*/,16/*localQueue*/),\
    SINGLE_GDMA_RX_INT_BIND_MAC(_tile, _type,  2/*local GDMA*/,  49/*globalQueue*/,17/*localQueue*/),\
    SINGLE_GDMA_RX_INT_BIND_MAC(_tile, _type,  2/*local GDMA*/,  50/*globalQueue*/,18/*localQueue*/),\
    SINGLE_GDMA_RX_INT_BIND_MAC(_tile, _type,  2/*local GDMA*/,  51/*globalQueue*/,19/*localQueue*/),\
    SINGLE_GDMA_RX_INT_BIND_MAC(_tile, _type,  2/*local GDMA*/,  52/*globalQueue*/,20/*localQueue*/),\
    SINGLE_GDMA_RX_INT_BIND_MAC(_tile, _type,  2/*local GDMA*/,  53/*globalQueue*/,21/*localQueue*/),\
    SINGLE_GDMA_RX_INT_BIND_MAC(_tile, _type,  2/*local GDMA*/,  54/*globalQueue*/,22/*localQueue*/),\
    SINGLE_GDMA_RX_INT_BIND_MAC(_tile, _type,  2/*local GDMA*/,  55/*globalQueue*/,23/*localQueue*/),\
    SINGLE_GDMA_RX_INT_BIND_MAC(_tile, _type,  2/*local GDMA*/,  56/*globalQueue*/,24/*localQueue*/),\
    SINGLE_GDMA_RX_INT_BIND_MAC(_tile, _type,  2/*local GDMA*/,  57/*globalQueue*/,25/*localQueue*/),\
    SINGLE_GDMA_RX_INT_BIND_MAC(_tile, _type,  2/*local GDMA*/,  58/*globalQueue*/,26/*localQueue*/),\
    SINGLE_GDMA_RX_INT_BIND_MAC(_tile, _type,  2/*local GDMA*/,  59/*globalQueue*/,27/*localQueue*/),\
    SINGLE_GDMA_RX_INT_BIND_MAC(_tile, _type,  2/*local GDMA*/,  60/*globalQueue*/,28/*localQueue*/),\
    SINGLE_GDMA_RX_INT_BIND_MAC(_tile, _type,  2/*local GDMA*/,  61/*globalQueue*/,29/*localQueue*/),\
    SINGLE_GDMA_RX_INT_BIND_MAC(_tile, _type,  2/*local GDMA*/,  62/*globalQueue*/,30/*localQueue*/),\
    SINGLE_GDMA_RX_INT_BIND_MAC(_tile, _type,  2/*local GDMA*/,  63/*globalQueue*/,31/*localQueue*/)


#define SINGLE_GDMA_RX_INT_BIND_MAC(_tileId, _type, _gdmaId, _globalQueue, _queueId) \
    CPSS_PP_##_type##_QUEUE##_globalQueue##_E,                              \
        PRV_CPSS_AAS_CNM_CNM_TILE_##_tileId##_GDMA_##_gdmaId##_QUEUE_##_queueId##_##_type##_INTERRUPT_E, _globalQueue,\
    MARK_END_OF_UNI_EV_CNS

/* The GDMA[3] support the GDMA for 'FROM_CPU' */
#define AAS_ALL_GDMA_TX_INTERRUPTS_MAC(_tile, _type)    \
    SINGLE_GDMA_TX_INT_NAME(_tile,     _type,  3/*local GDMA*/,  0/*globalQueue*/, 0/*localQueue*/),\
    SINGLE_GDMA_TX_INT_NAME(_tile,     _type,  3/*local GDMA*/,  1/*globalQueue*/, 1/*localQueue*/),\
    SINGLE_GDMA_TX_INT_NAME(_tile,     _type,  3/*local GDMA*/,  2/*globalQueue*/, 2/*localQueue*/),\
    SINGLE_GDMA_TX_INT_NAME(_tile,     _type,  3/*local GDMA*/,  3/*globalQueue*/, 3/*localQueue*/),\
    SINGLE_GDMA_TX_INT_NAME(_tile,     _type,  3/*local GDMA*/,  4/*globalQueue*/, 4/*localQueue*/),\
    SINGLE_GDMA_TX_INT_NAME(_tile,     _type,  3/*local GDMA*/,  5/*globalQueue*/, 5/*localQueue*/),\
    SINGLE_GDMA_TX_INT_NAME(_tile,     _type,  3/*local GDMA*/,  6/*globalQueue*/, 6/*localQueue*/),\
    SINGLE_GDMA_TX_INT_NAME(_tile,     _type,  3/*local GDMA*/,  7/*globalQueue*/, 7/*localQueue*/),\
    SINGLE_GDMA_TX_INT_NAME(_tile,     _type,  3/*local GDMA*/,  8/*globalQueue*/, 8/*localQueue*/),\
    SINGLE_GDMA_TX_INT_NAME(_tile,     _type,  3/*local GDMA*/,  9/*globalQueue*/, 9/*localQueue*/),\
    SINGLE_GDMA_TX_INT_NAME(_tile,     _type,  3/*local GDMA*/, 10/*globalQueue*/,10/*localQueue*/),\
    SINGLE_GDMA_TX_INT_NAME(_tile,     _type,  3/*local GDMA*/, 11/*globalQueue*/,11/*localQueue*/),\
    SINGLE_GDMA_TX_INT_NAME(_tile,     _type,  3/*local GDMA*/, 12/*globalQueue*/,12/*localQueue*/),\
    SINGLE_GDMA_TX_INT_NAME(_tile,     _type,  3/*local GDMA*/, 13/*globalQueue*/,13/*localQueue*/),\
    SINGLE_GDMA_TX_INT_NAME(_tile,     _type,  3/*local GDMA*/, 14/*globalQueue*/,14/*localQueue*/),\
    SINGLE_GDMA_TX_INT_NAME(_tile,     _type,  3/*local GDMA*/, 15/*globalQueue*/,15/*localQueue*/),\
    SINGLE_GDMA_TX_INT_NAME(_tile,     _type,  3/*local GDMA*/, 16/*globalQueue*/,16/*localQueue*/),\
    SINGLE_GDMA_TX_INT_NAME(_tile,     _type,  3/*local GDMA*/, 17/*globalQueue*/,17/*localQueue*/),\
    SINGLE_GDMA_TX_INT_NAME(_tile,     _type,  3/*local GDMA*/, 18/*globalQueue*/,18/*localQueue*/),\
    SINGLE_GDMA_TX_INT_NAME(_tile,     _type,  3/*local GDMA*/, 19/*globalQueue*/,19/*localQueue*/),\
    SINGLE_GDMA_TX_INT_NAME(_tile,     _type,  3/*local GDMA*/, 20/*globalQueue*/,20/*localQueue*/),\
    SINGLE_GDMA_TX_INT_NAME(_tile,     _type,  3/*local GDMA*/, 21/*globalQueue*/,21/*localQueue*/),\
    SINGLE_GDMA_TX_INT_NAME(_tile,     _type,  3/*local GDMA*/, 22/*globalQueue*/,22/*localQueue*/),\
    SINGLE_GDMA_TX_INT_NAME(_tile,     _type,  3/*local GDMA*/, 23/*globalQueue*/,23/*localQueue*/),\
    SINGLE_GDMA_TX_INT_NAME(_tile,     _type,  3/*local GDMA*/, 24/*globalQueue*/,24/*localQueue*/),\
    SINGLE_GDMA_TX_INT_NAME(_tile,     _type,  3/*local GDMA*/, 25/*globalQueue*/,25/*localQueue*/),\
    SINGLE_GDMA_TX_INT_NAME(_tile,     _type,  3/*local GDMA*/, 26/*globalQueue*/,26/*localQueue*/),\
    SINGLE_GDMA_TX_INT_NAME(_tile,     _type,  3/*local GDMA*/, 27/*globalQueue*/,27/*localQueue*/),\
    SINGLE_GDMA_TX_INT_NAME(_tile,     _type,  3/*local GDMA*/, 28/*globalQueue*/,28/*localQueue*/),\
    SINGLE_GDMA_TX_INT_NAME(_tile,     _type,  3/*local GDMA*/, 29/*globalQueue*/,29/*localQueue*/),\
    SINGLE_GDMA_TX_INT_NAME(_tile,     _type,  3/*local GDMA*/, 30/*globalQueue*/,30/*localQueue*/),\
    SINGLE_GDMA_TX_INT_NAME(_tile,     _type,  3/*local GDMA*/, 31/*globalQueue*/,31/*localQueue*/)

#define SINGLE_GDMA_TX_INT_NAME(_tileId, _type, _gdmaId, _globalQueue,_queueId) \
        PRV_CPSS_AAS_CNM_CNM_TILE_##_tileId##_GDMA_##_gdmaId##_QUEUE_##_queueId##_##_type##_INTERRUPT_E, _globalQueue

/* Maximal PPN events */
#define PPN_EVENT_TOTAL    16

#define SET_PPN_EVENT_MAC(_tile,_ppg, _ppn)   \
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_PPN_##_ppn##_PPG_##_ppg##_PPN_INTERNAL_ERROR_CAUSE_HOST_UNMAPPED_ACCESS_E             , ((8 + (PPN_EVENT_TOTAL * 0) + (_ppg * 8)) + (_ppn)),  \
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_PPN_##_ppn##_PPG_##_ppg##_PPN_INTERNAL_ERROR_CAUSE_CORE_UNMAPPED_ACCESS_E             , ((8 + (PPN_EVENT_TOTAL * 1) + (_ppg * 8)) + (_ppn)),  \
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_PPN_##_ppn##_PPG_##_ppg##_PPN_INTERNAL_ERROR_CAUSE_NEAR_EDGE_IMEM_ACCESS_E            , ((8 + (PPN_EVENT_TOTAL * 2) + (_ppg * 8)) + (_ppn)),  \
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_PPN_##_ppn##_PPG_##_ppg##_PPN_INTERNAL_ERROR_CAUSE_DOORBELL_INTERRUPT_E               , ((8 + (PPN_EVENT_TOTAL * 3) + (_ppg * 8)) + (_ppn))

#define SET_PPG_EVENT_PER_8_PPN_MAC(_tile,_ppg)\
    SET_PPN_EVENT_MAC(_tile,_ppg, 0),          \
    SET_PPN_EVENT_MAC(_tile,_ppg, 1),          \
    SET_PPN_EVENT_MAC(_tile,_ppg, 2),          \
    SET_PPN_EVENT_MAC(_tile,_ppg, 3),          \
    SET_PPN_EVENT_MAC(_tile,_ppg, 4),          \
    SET_PPN_EVENT_MAC(_tile,_ppg, 5),          \
    SET_PPN_EVENT_MAC(_tile,_ppg, 6),          \
    SET_PPN_EVENT_MAC(_tile,_ppg, 7)


#define SET_PHA_EVENTS_MAC(_tile)  \
   /* PHA/PPA */                         \
   INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_PHA_PHA_INTERNAL_ERROR_CAUSE_PHA_BAD_ADDRESS_ERROR_E,                                      0,   \
   INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_PHA_PHA_INTERNAL_ERROR_CAUSE_PHA_TABLE_ACCESS_OVERLAP_ERROR_E,                             512, \
   INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_PHA_PHA_INTERNAL_ERROR_CAUSE_HEADER_SIZE_VIOLATION_E,                                      513, \
   INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_PHA_PHA_INTERNAL_ERROR_CAUSE_PPA_CLOCK_DOWN_VIOLATION_E,                                   514, \
   /* PPGs */                                                                                                    \
   INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_PPG_0_PPG_INTERNAL_ERROR_CAUSE_PPG_BAD_ADDRESS_ERROR_E,                                    4,   \
   INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_PPG_1_PPG_INTERNAL_ERROR_CAUSE_PPG_BAD_ADDRESS_ERROR_E,                                    5,   \
                                                                                                                 \
   INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_PPG_0_PPG_INTERNAL_ERROR_CAUSE_PPG_TABLE_ACCESS_OVERLAP_ERROR_E,                           515, \
   INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_PPG_1_PPG_INTERNAL_ERROR_CAUSE_PPG_TABLE_ACCESS_OVERLAP_ERROR_E,                           516, \
   /* PPNs */                                                                                                    \
   SET_PPG_EVENT_PER_8_PPN_MAC(_tile,0),                                                                              \
   SET_PPG_EVENT_PER_8_PPN_MAC(_tile,1)

/* _postFix - must include the "_E" */
#define SET_LMU_EVENT_WITH_INDEX_MAC(_tile,_postFix, _instance, _threshold, _index)                                   \
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_LMU_INSTANCE_##_instance##_N_##_threshold##_I_##_index##_##_postFix, ((_instance << 16) | ((31*_threshold) +(_index)))

#define LMU_LATENCY_OVER_THRESHOLD_MAC(_tile,_instance, _threshold)                                   \
    SET_LMU_EVENT_WITH_INDEX_MAC(_tile,LMU_LATENCY_OVER_THRESHOLD_E, _instance,  _threshold,   0),    \
    SET_LMU_EVENT_WITH_INDEX_MAC(_tile,LMU_LATENCY_OVER_THRESHOLD_E, _instance,  _threshold,   1),    \
    SET_LMU_EVENT_WITH_INDEX_MAC(_tile,LMU_LATENCY_OVER_THRESHOLD_E, _instance,  _threshold,   2),    \
    SET_LMU_EVENT_WITH_INDEX_MAC(_tile,LMU_LATENCY_OVER_THRESHOLD_E, _instance,  _threshold,   3),    \
    SET_LMU_EVENT_WITH_INDEX_MAC(_tile,LMU_LATENCY_OVER_THRESHOLD_E, _instance,  _threshold,   4),    \
    SET_LMU_EVENT_WITH_INDEX_MAC(_tile,LMU_LATENCY_OVER_THRESHOLD_E, _instance,  _threshold,   5),    \
    SET_LMU_EVENT_WITH_INDEX_MAC(_tile,LMU_LATENCY_OVER_THRESHOLD_E, _instance,  _threshold,   6),    \
    SET_LMU_EVENT_WITH_INDEX_MAC(_tile,LMU_LATENCY_OVER_THRESHOLD_E, _instance,  _threshold,   7),    \
    SET_LMU_EVENT_WITH_INDEX_MAC(_tile,LMU_LATENCY_OVER_THRESHOLD_E, _instance,  _threshold,   8),    \
    SET_LMU_EVENT_WITH_INDEX_MAC(_tile,LMU_LATENCY_OVER_THRESHOLD_E, _instance,  _threshold,   9),    \
    SET_LMU_EVENT_WITH_INDEX_MAC(_tile,LMU_LATENCY_OVER_THRESHOLD_E, _instance,  _threshold,  10),    \
    SET_LMU_EVENT_WITH_INDEX_MAC(_tile,LMU_LATENCY_OVER_THRESHOLD_E, _instance,  _threshold,  11),    \
    SET_LMU_EVENT_WITH_INDEX_MAC(_tile,LMU_LATENCY_OVER_THRESHOLD_E, _instance,  _threshold,  12),    \
    SET_LMU_EVENT_WITH_INDEX_MAC(_tile,LMU_LATENCY_OVER_THRESHOLD_E, _instance,  _threshold,  13),    \
    SET_LMU_EVENT_WITH_INDEX_MAC(_tile,LMU_LATENCY_OVER_THRESHOLD_E, _instance,  _threshold,  14),    \
    SET_LMU_EVENT_WITH_INDEX_MAC(_tile,LMU_LATENCY_OVER_THRESHOLD_E, _instance,  _threshold,  15),    \
    SET_LMU_EVENT_WITH_INDEX_MAC(_tile,LMU_LATENCY_OVER_THRESHOLD_E, _instance,  _threshold,  16),    \
    SET_LMU_EVENT_WITH_INDEX_MAC(_tile,LMU_LATENCY_OVER_THRESHOLD_E, _instance,  _threshold,  17),    \
    SET_LMU_EVENT_WITH_INDEX_MAC(_tile,LMU_LATENCY_OVER_THRESHOLD_E, _instance,  _threshold,  18),    \
    SET_LMU_EVENT_WITH_INDEX_MAC(_tile,LMU_LATENCY_OVER_THRESHOLD_E, _instance,  _threshold,  19),    \
    SET_LMU_EVENT_WITH_INDEX_MAC(_tile,LMU_LATENCY_OVER_THRESHOLD_E, _instance,  _threshold,  20),    \
    SET_LMU_EVENT_WITH_INDEX_MAC(_tile,LMU_LATENCY_OVER_THRESHOLD_E, _instance,  _threshold,  21),    \
    SET_LMU_EVENT_WITH_INDEX_MAC(_tile,LMU_LATENCY_OVER_THRESHOLD_E, _instance,  _threshold,  22),    \
    SET_LMU_EVENT_WITH_INDEX_MAC(_tile,LMU_LATENCY_OVER_THRESHOLD_E, _instance,  _threshold,  23),    \
    SET_LMU_EVENT_WITH_INDEX_MAC(_tile,LMU_LATENCY_OVER_THRESHOLD_E, _instance,  _threshold,  24),    \
    SET_LMU_EVENT_WITH_INDEX_MAC(_tile,LMU_LATENCY_OVER_THRESHOLD_E, _instance,  _threshold,  25),    \
    SET_LMU_EVENT_WITH_INDEX_MAC(_tile,LMU_LATENCY_OVER_THRESHOLD_E, _instance,  _threshold,  26),    \
    SET_LMU_EVENT_WITH_INDEX_MAC(_tile,LMU_LATENCY_OVER_THRESHOLD_E, _instance,  _threshold,  27),    \
    SET_LMU_EVENT_WITH_INDEX_MAC(_tile,LMU_LATENCY_OVER_THRESHOLD_E, _instance,  _threshold,  28),    \
    SET_LMU_EVENT_WITH_INDEX_MAC(_tile,LMU_LATENCY_OVER_THRESHOLD_E, _instance,  _threshold,  29),    \
    SET_LMU_EVENT_WITH_INDEX_MAC(_tile,LMU_LATENCY_OVER_THRESHOLD_E, _instance,  _threshold,  30)

#define AAS_PER_INSTANCE_LMU_LATENCY_OVER_THRESHOLD_MAC(_tile,_instance)                          \
    LMU_LATENCY_OVER_THRESHOLD_MAC(_tile,_instance,  0),                                              \
    LMU_LATENCY_OVER_THRESHOLD_MAC(_tile,_instance,  1),                                              \
    LMU_LATENCY_OVER_THRESHOLD_MAC(_tile,_instance,  2),                                              \
    LMU_LATENCY_OVER_THRESHOLD_MAC(_tile,_instance,  3),                                              \
    LMU_LATENCY_OVER_THRESHOLD_MAC(_tile,_instance,  4),                                              \
    LMU_LATENCY_OVER_THRESHOLD_MAC(_tile,_instance,  5),                                              \
    LMU_LATENCY_OVER_THRESHOLD_MAC(_tile,_instance,  6),                                              \
    LMU_LATENCY_OVER_THRESHOLD_MAC(_tile,_instance,  7),                                              \
    LMU_LATENCY_OVER_THRESHOLD_MAC(_tile,_instance,  8),                                              \
    LMU_LATENCY_OVER_THRESHOLD_MAC(_tile,_instance,  9),                                              \
    LMU_LATENCY_OVER_THRESHOLD_MAC(_tile,_instance, 10),                                              \
    LMU_LATENCY_OVER_THRESHOLD_MAC(_tile,_instance, 11),                                              \
    LMU_LATENCY_OVER_THRESHOLD_MAC(_tile,_instance, 12),                                              \
    LMU_LATENCY_OVER_THRESHOLD_MAC(_tile,_instance, 13),                                              \
    LMU_LATENCY_OVER_THRESHOLD_MAC(_tile,_instance, 14),                                              \
    LMU_LATENCY_OVER_THRESHOLD_MAC(_tile,_instance, 15),                                              \
    LMU_LATENCY_OVER_THRESHOLD_MAC(_tile,_instance, 16)

#define SET_HR_TRSH_EVENT_MAC(_tile,_unit, _regIndex, _port, _tc)             \
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_QFC_UNIT_##_unit##_REGISTER_INDEX_##_regIndex##_PORT_##_port##_TC_##_tc##_CROSSED_HR_THRESHOLD_E, (_port * 8) + _tc

#define SET_HR_TRSH_EVENT_ALL_TC_MAC(_tile,_unit, _regIndex, _port)           \
    SET_HR_TRSH_EVENT_MAC(_tile,_unit, _regIndex, _port, 0),                  \
    SET_HR_TRSH_EVENT_MAC(_tile,_unit, _regIndex, _port, 1),                  \
    SET_HR_TRSH_EVENT_MAC(_tile,_unit, _regIndex, _port, 2),                  \
    SET_HR_TRSH_EVENT_MAC(_tile,_unit, _regIndex, _port, 3),                  \
    SET_HR_TRSH_EVENT_MAC(_tile,_unit, _regIndex, _port, 4),                  \
    SET_HR_TRSH_EVENT_MAC(_tile,_unit, _regIndex, _port, 5),                  \
    SET_HR_TRSH_EVENT_MAC(_tile,_unit, _regIndex, _port, 6),                  \
    SET_HR_TRSH_EVENT_MAC(_tile,_unit, _regIndex, _port, 7)

#define AAS_SET_HR_TRSH_EVENT_ALL_PORTS_MAC(_tile)                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(_tile,0, 0,   0),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(_tile,0, 0,   1),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(_tile,0, 0,   2),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(_tile,0, 1,   3),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(_tile,0, 1,   4),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(_tile,0, 1,   5),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(_tile,0, 2,   6),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(_tile,0, 2,   7),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(_tile,0, 2,   8),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(_tile,0, 3,   9),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(_tile,0, 3,  10),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(_tile,0, 3,  11),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(_tile,0, 4,  12),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(_tile,0, 4,  13),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(_tile,0, 4,  14),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(_tile,0, 5,  15),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(_tile,0, 5,  16),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(_tile,0, 5,  17),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(_tile,0, 6,  18),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(_tile,0, 6,  19),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(_tile,0, 6,  20),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(_tile,0, 7,  21),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(_tile,0, 7,  22),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(_tile,0, 7,  23),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(_tile,0, 8,  24),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(_tile,0, 8,  25),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(_tile,0, 8,  26),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(_tile,1, 0,  27),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(_tile,1, 0,  28),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(_tile,1, 0,  29),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(_tile,1, 1,  30),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(_tile,1, 1,  31),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(_tile,1, 1,  32),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(_tile,1, 2,  33),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(_tile,1, 2,  34),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(_tile,1, 2,  35),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(_tile,1, 3,  36),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(_tile,1, 3,  37),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(_tile,1, 3,  38),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(_tile,1, 4,  39),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(_tile,1, 4,  40),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(_tile,1, 4,  41),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(_tile,1, 5,  42),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(_tile,1, 5,  43),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(_tile,1, 5,  44),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(_tile,1, 6,  45),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(_tile,1, 6,  46),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(_tile,1, 6,  47),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(_tile,1, 7,  48),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(_tile,1, 7,  49),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(_tile,1, 7,  50),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(_tile,1, 8,  51),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(_tile,1, 8,  52),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(_tile,1, 8,  53),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(_tile,2, 0,  54),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(_tile,2, 0,  55),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(_tile,2, 0,  56),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(_tile,2, 1,  57),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(_tile,2, 1,  58),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(_tile,2, 1,  59),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(_tile,2, 2,  60),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(_tile,2, 2,  61),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(_tile,2, 2,  62),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(_tile,2, 3,  63),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(_tile,2, 3,  64),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(_tile,2, 3,  65),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(_tile,2, 4,  66),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(_tile,2, 4,  67),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(_tile,2, 4,  68),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(_tile,2, 5,  69),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(_tile,2, 5,  70),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(_tile,2, 5,  71),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(_tile,2, 6,  72),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(_tile,2, 6,  73),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(_tile,2, 6,  74),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(_tile,2, 7,  75),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(_tile,2, 7,  76),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(_tile,2, 7,  77),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(_tile,2, 8,  78),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(_tile,2, 8,  79),                                 \
    SET_HR_TRSH_EVENT_ALL_TC_MAC(_tile,2, 8,  80)

#define CTSU_INSTANCE_CHANNEL_QUEUE0_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_tile,_postfix, _instance, _channel, _extData, _port)                          \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_CTSU_INSTANCE_##_instance##_CHANNEL_##_channel##_CHANNEL_EGRESS_TIMESTAMP_QUEUE_0_##_postfix, (((_port) << 8) | (_extData))

#define CTSU_INSTANCE_CHANNEL_QUEUE1_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_tile,_postfix, _instance, _channel, _extData, _port)                          \
        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_CTSU_INSTANCE_##_instance##_CHANNEL_##_channel##_CHANNEL_EGRESS_TIMESTAMP_QUEUE_1_##_postfix, (((_port) << 8) | (_extData))

#define SET_EVENT_GROUP_0_CHANNEL_CTSU_16_PCA_MAC(_tile,_postfix, _instance, _extData)                                                                 \
        CTSU_INSTANCE_CHANNEL_QUEUE0_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_tile,_postfix, _instance,  0, _extData, ( 0 + (_instance * 16))),               \
        CTSU_INSTANCE_CHANNEL_QUEUE0_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_tile,_postfix, _instance,  1, _extData, ( 1 + (_instance * 16))),               \
        CTSU_INSTANCE_CHANNEL_QUEUE0_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_tile,_postfix, _instance,  2, _extData, ( 2 + (_instance * 16))),               \
        CTSU_INSTANCE_CHANNEL_QUEUE0_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_tile,_postfix, _instance,  3, _extData, ( 3 + (_instance * 16))),               \
        CTSU_INSTANCE_CHANNEL_QUEUE0_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_tile,_postfix, _instance,  4, _extData, ( 4 + (_instance * 16))),               \
        CTSU_INSTANCE_CHANNEL_QUEUE0_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_tile,_postfix, _instance,  5, _extData, ( 5 + (_instance * 16))),               \
        CTSU_INSTANCE_CHANNEL_QUEUE0_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_tile,_postfix, _instance,  6, _extData, ( 6 + (_instance * 16))),               \
        CTSU_INSTANCE_CHANNEL_QUEUE0_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_tile,_postfix, _instance,  7, _extData, ( 7 + (_instance * 16))),               \
        CTSU_INSTANCE_CHANNEL_QUEUE0_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_tile,_postfix, _instance,  8, _extData, ( 8 + (_instance * 16))),               \
        CTSU_INSTANCE_CHANNEL_QUEUE0_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_tile,_postfix, _instance,  9, _extData, ( 9 + (_instance * 16))),               \
        CTSU_INSTANCE_CHANNEL_QUEUE0_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_tile,_postfix, _instance, 10, _extData, (10 + (_instance * 16))),               \
        CTSU_INSTANCE_CHANNEL_QUEUE0_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_tile,_postfix, _instance, 11, _extData, (11 + (_instance * 16))),               \
        CTSU_INSTANCE_CHANNEL_QUEUE0_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_tile,_postfix, _instance, 12, _extData, (12 + (_instance * 16))),               \
        CTSU_INSTANCE_CHANNEL_QUEUE0_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_tile,_postfix, _instance, 13, _extData, (13 + (_instance * 16))),               \
        CTSU_INSTANCE_CHANNEL_QUEUE0_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_tile,_postfix, _instance, 14, _extData, (14 + (_instance * 16))),               \
        CTSU_INSTANCE_CHANNEL_QUEUE0_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_tile,_postfix, _instance, 15, _extData, (15 + (_instance * 16)))

#define SET_EVENT_GROUP_0_INSTANCE_CTSU_16_PCA_MAC(_tile,_postfix, _extData)                                                                          \
        SET_EVENT_GROUP_0_CHANNEL_CTSU_16_PCA_MAC(_tile,_postfix, 0, _extData),                                                                       \
        SET_EVENT_GROUP_0_CHANNEL_CTSU_16_PCA_MAC(_tile,_postfix, 1, _extData),                                                                       \
        SET_EVENT_GROUP_0_CHANNEL_CTSU_16_PCA_MAC(_tile,_postfix, 2, _extData)

#define SET_EVENT_GROUP_1_CHANNEL_CTSU_16_PCA_MAC(_tile,_postfix, _instance, _extData)                                                                 \
        CTSU_INSTANCE_CHANNEL_QUEUE1_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_tile,_postfix,_instance,  0, _extData, (16 + (_instance * 16))),               \
        CTSU_INSTANCE_CHANNEL_QUEUE1_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_tile,_postfix,_instance,  1, _extData, (17 + (_instance * 16))),               \
        CTSU_INSTANCE_CHANNEL_QUEUE1_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_tile,_postfix,_instance,  2, _extData, (18 + (_instance * 16))),               \
        CTSU_INSTANCE_CHANNEL_QUEUE1_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_tile,_postfix,_instance,  3, _extData, (19 + (_instance * 16))),               \
        CTSU_INSTANCE_CHANNEL_QUEUE1_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_tile,_postfix,_instance,  4, _extData, (20 + (_instance * 16))),               \
        CTSU_INSTANCE_CHANNEL_QUEUE1_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_tile,_postfix,_instance,  5, _extData, (21 + (_instance * 16))),               \
        CTSU_INSTANCE_CHANNEL_QUEUE1_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_tile,_postfix,_instance,  6, _extData, (22 + (_instance * 16))),               \
        CTSU_INSTANCE_CHANNEL_QUEUE1_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_tile,_postfix,_instance,  7, _extData, (23 + (_instance * 16))),               \
        CTSU_INSTANCE_CHANNEL_QUEUE1_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_tile,_postfix,_instance,  8, _extData, (24 + (_instance * 16))),               \
        CTSU_INSTANCE_CHANNEL_QUEUE1_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_tile,_postfix,_instance,  9, _extData, (25 + (_instance * 16))),               \
        CTSU_INSTANCE_CHANNEL_QUEUE1_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_tile,_postfix,_instance, 10, _extData, (26 + (_instance * 16))),               \
        CTSU_INSTANCE_CHANNEL_QUEUE1_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_tile,_postfix,_instance, 11, _extData, (27 + (_instance * 16))),               \
        CTSU_INSTANCE_CHANNEL_QUEUE1_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_tile,_postfix,_instance, 12, _extData, (28 + (_instance * 16))),               \
        CTSU_INSTANCE_CHANNEL_QUEUE1_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_tile,_postfix,_instance, 13, _extData, (29 + (_instance * 16))),               \
        CTSU_INSTANCE_CHANNEL_QUEUE1_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_tile,_postfix,_instance, 14, _extData, (30 + (_instance * 16))),               \
        CTSU_INSTANCE_CHANNEL_QUEUE1_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_tile,_postfix,_instance, 15, _extData, (31 + (_instance * 16)))

#define SET_EVENT_GROUP_1_INSTANCE_CTSU_16_PCA_MAC(_tile,_postfix, _extData)                                                                          \
        SET_EVENT_GROUP_1_CHANNEL_CTSU_16_PCA_MAC(_tile,_postfix, 0, _extData),                                                                       \
        SET_EVENT_GROUP_1_CHANNEL_CTSU_16_PCA_MAC(_tile,_postfix, 1, _extData),                                                                       \
        SET_EVENT_GROUP_1_CHANNEL_CTSU_16_PCA_MAC(_tile,_postfix, 2, _extData)

#define AAS_COMMON_EVENTS(_tile)                                                                                        \
                                                                                                                        \
 /* single uniEvent for all 128 queues , extData = queueId */                                                           \
 CPSS_PP_TX_BUFFER_QUEUE_E,                                                                                             \
     AAS_ALL_GDMA_TX_INTERRUPTS_MAC(_tile,TX_BUFFER_QUEUE),                                                                 \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 /* single uniEvent for all 128 queues , extData = queueId */                                                           \
 CPSS_PP_TX_ERR_QUEUE_E,                                                                                                \
     AAS_ALL_GDMA_TX_INTERRUPTS_MAC(_tile,TX_ERR_QUEUE),                                                                    \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 /* single uniEvent for all 128 queues , extData = queueId */                                                           \
/* the CPSS_PP_TX_END_E have no bit in HW */                                                                            \
CPSS_PP_TX_END_E,                                                                                                       \
    AAS_ALL_GDMA_TX_INTERRUPTS_MAC(_tile,TX_END),                                                                           \
MARK_END_OF_UNI_EV_CNS,                                                                                                 \
                                                                                                                        \
                                                                                                                        \
 CPSS_PP_GTS_GLOBAL_FIFO_FULL_E,                                                                                        \
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_ERMRK_ERMRK_INTERRUPT_CAUSE_INGRESS_TIMESTAMP_QUEUE0_FULL_E,  0,                                   \
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_ERMRK_ERMRK_INTERRUPT_CAUSE_EGRESS_TIMESTAMP_QUEUE0_FULL_E,   1,                                   \
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_ERMRK_ERMRK_INTERRUPT_CAUSE_INGRESS_TIMESTAMP_QUEUE1_FULL_E,  2,                                   \
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_ERMRK_ERMRK_INTERRUPT_CAUSE_EGRESS_TIMESTAMP_QUEUE1_FULL_E,   3,                                   \
    SET_EVENT_GROUP_0_INSTANCE_CTSU_16_PCA_MAC(_tile,OVERFLOW_INTERRUPT_E, GTS_INT_EXT_PARAM_MAC(1,0,1)),                     \
    SET_EVENT_GROUP_0_INSTANCE_CTSU_16_PCA_MAC(_tile,OVERFLOW_INTERRUPT_E, GTS_INT_EXT_PARAM_MAC(1,1,1)),                     \
    SET_EVENT_GROUP_1_INSTANCE_CTSU_16_PCA_MAC(_tile,OVERFLOW_INTERRUPT_E, GTS_INT_EXT_PARAM_MAC(1,0,1)),                     \
    SET_EVENT_GROUP_1_INSTANCE_CTSU_16_PCA_MAC(_tile,OVERFLOW_INTERRUPT_E, GTS_INT_EXT_PARAM_MAC(1,1,1)),                     \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_GTS_VALID_TIME_SAMPLE_MESSAGE_E,                                                                               \
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_ERMRK_ERMRK_INTERRUPT_CAUSE_NEW_INGRESS_TIMESTAMP_Q0_E,  0,                                        \
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_ERMRK_ERMRK_INTERRUPT_CAUSE_NEW_EGRESS_TIMESTAMP_Q0_E,   1,                                        \
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_ERMRK_ERMRK_INTERRUPT_CAUSE_NEW_INGRESS_TIMESTAMP_Q1_E,  2,                                        \
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_ERMRK_ERMRK_INTERRUPT_CAUSE_NEW_EGRESS_TIMESTAMP_Q1_E,   3,                                        \
    SET_EVENT_GROUP_0_INSTANCE_CTSU_16_PCA_MAC(_tile,NEW_ENTRY_INTERRUPT_E, GTS_INT_EXT_PARAM_MAC(1,0,1)),                    \
    SET_EVENT_GROUP_0_INSTANCE_CTSU_16_PCA_MAC(_tile,NEW_ENTRY_INTERRUPT_E, GTS_INT_EXT_PARAM_MAC(1,1,1)),                    \
    SET_EVENT_GROUP_1_INSTANCE_CTSU_16_PCA_MAC(_tile,NEW_ENTRY_INTERRUPT_E, GTS_INT_EXT_PARAM_MAC(1,0,1)),                    \
    SET_EVENT_GROUP_1_INSTANCE_CTSU_16_PCA_MAC(_tile,NEW_ENTRY_INTERRUPT_E, GTS_INT_EXT_PARAM_MAC(1,1,1)),                    \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_PTP_TAI_INCOMING_TRIGGER_E,                                                                                    \
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_TAI_MASTER_0_TAI_INTERRUPT_CAUSE_INCOMING_TRIGGER_INT_E,       0,                                  \
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_TAI_MASTER_1_TAI_INTERRUPT_CAUSE_INCOMING_TRIGGER_INT_E,       1,                                  \
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_MAC_TAI_SLAVE_0_TAI_INTERRUPT_CAUSE_INCOMING_TRIGGER_INT_E,    2,                                  \
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_MAC_TAI_SLAVE_1_TAI_INTERRUPT_CAUSE_INCOMING_TRIGGER_INT_E,    3,                                  \
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_MAC_TAI_SLAVE_2_TAI_INTERRUPT_CAUSE_INCOMING_TRIGGER_INT_E,    4,                                  \
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_MAC_TAI_SLAVE_3_TAI_INTERRUPT_CAUSE_INCOMING_TRIGGER_INT_E,    5,                                  \
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_MAC_TAI_SLAVE_4_TAI_INTERRUPT_CAUSE_INCOMING_TRIGGER_INT_E,    6,                                  \
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_MAC_TAI_SLAVE_5_TAI_INTERRUPT_CAUSE_INCOMING_TRIGGER_INT_E,    7,                                  \
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_QFC_TAI_SLAVE_0_TAI_INTERRUPT_CAUSE_INCOMING_TRIGGER_INT_E,    8,                                  \
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_QFC_TAI_SLAVE_1_TAI_INTERRUPT_CAUSE_INCOMING_TRIGGER_INT_E,    9,                                  \
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_QFC_TAI_SLAVE_2_TAI_INTERRUPT_CAUSE_INCOMING_TRIGGER_INT_E,    10,                                 \
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_ERMRK_TAI_SLAVE_0_TAI_INTERRUPT_CAUSE_INCOMING_TRIGGER_INT_E,  11,                                 \
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_ERMRK_TAI_SLAVE_1_TAI_INTERRUPT_CAUSE_INCOMING_TRIGGER_INT_E,  12,                                 \
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_PHA_TAI_SLAVE_0_TAI_INTERRUPT_CAUSE_INCOMING_TRIGGER_INT_E,    13,                                 \
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_PHA_TAI_SLAVE_1_TAI_INTERRUPT_CAUSE_INCOMING_TRIGGER_INT_E,    14,                                 \
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_TTI_TAI_SLAVE_0_TAI_INTERRUPT_CAUSE_INCOMING_TRIGGER_INT_E,    15,                                 \
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_TTI_TAI_SLAVE_1_TAI_INTERRUPT_CAUSE_INCOMING_TRIGGER_INT_E,    16,                                 \
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_PCA_TAI_SLAVE_0_TAI_INTERRUPT_CAUSE_INCOMING_TRIGGER_INT_E,    17,                                 \
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_PCA_TAI_SLAVE_1_TAI_INTERRUPT_CAUSE_INCOMING_TRIGGER_INT_E,    18,                                 \
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_PCA_TAI_SLAVE_2_TAI_INTERRUPT_CAUSE_INCOMING_TRIGGER_INT_E,    19,                                 \
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_PCA_TAI_SLAVE_3_TAI_INTERRUPT_CAUSE_INCOMING_TRIGGER_INT_E,    20,                                 \
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_PCA_TAI_SLAVE_4_TAI_INTERRUPT_CAUSE_INCOMING_TRIGGER_INT_E,    21,                                 \
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_PCA_TAI_SLAVE_5_TAI_INTERRUPT_CAUSE_INCOMING_TRIGGER_INT_E,    22,                                 \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_PTP_TAI_GENERATION_E,                                                                                          \
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_TAI_MASTER_0_TAI_INTERRUPT_CAUSE_GENERATION_INT_E,             0,                                  \
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_TAI_MASTER_1_TAI_INTERRUPT_CAUSE_GENERATION_INT_E,             1,                                  \
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_MAC_TAI_SLAVE_0_TAI_INTERRUPT_CAUSE_GENERATION_INT_E,          2,                                  \
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_MAC_TAI_SLAVE_1_TAI_INTERRUPT_CAUSE_GENERATION_INT_E,          3,                                  \
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_MAC_TAI_SLAVE_2_TAI_INTERRUPT_CAUSE_GENERATION_INT_E,          4,                                  \
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_MAC_TAI_SLAVE_3_TAI_INTERRUPT_CAUSE_GENERATION_INT_E,          5,                                  \
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_MAC_TAI_SLAVE_4_TAI_INTERRUPT_CAUSE_GENERATION_INT_E,          6,                                  \
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_MAC_TAI_SLAVE_5_TAI_INTERRUPT_CAUSE_GENERATION_INT_E,          7,                                  \
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_QFC_TAI_SLAVE_0_TAI_INTERRUPT_CAUSE_GENERATION_INT_E,          8,                                  \
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_QFC_TAI_SLAVE_1_TAI_INTERRUPT_CAUSE_GENERATION_INT_E,          9,                                  \
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_QFC_TAI_SLAVE_2_TAI_INTERRUPT_CAUSE_GENERATION_INT_E,          10,                                 \
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_ERMRK_TAI_SLAVE_0_TAI_INTERRUPT_CAUSE_GENERATION_INT_E,        11,                                 \
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_ERMRK_TAI_SLAVE_1_TAI_INTERRUPT_CAUSE_GENERATION_INT_E,        12,                                 \
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_PHA_TAI_SLAVE_0_TAI_INTERRUPT_CAUSE_GENERATION_INT_E,          13,                                 \
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_PHA_TAI_SLAVE_1_TAI_INTERRUPT_CAUSE_GENERATION_INT_E,          14,                                 \
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_TTI_TAI_SLAVE_0_TAI_INTERRUPT_CAUSE_GENERATION_INT_E,          15,                                 \
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_TTI_TAI_SLAVE_1_TAI_INTERRUPT_CAUSE_GENERATION_INT_E,          16,                                 \
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_PCA_TAI_SLAVE_0_TAI_INTERRUPT_CAUSE_GENERATION_INT_E,          17,                                 \
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_PCA_TAI_SLAVE_1_TAI_INTERRUPT_CAUSE_GENERATION_INT_E,          18,                                 \
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_PCA_TAI_SLAVE_2_TAI_INTERRUPT_CAUSE_GENERATION_INT_E,          19,                                 \
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_PCA_TAI_SLAVE_3_TAI_INTERRUPT_CAUSE_GENERATION_INT_E,          20,                                 \
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_PCA_TAI_SLAVE_4_TAI_INTERRUPT_CAUSE_GENERATION_INT_E,          21,                                 \
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_PCA_TAI_SLAVE_5_TAI_INTERRUPT_CAUSE_GENERATION_INT_E,          22,                                 \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 /* 64 uniEvents for 64 queues , extData = queueId */                                                                 \
 AAS_ALL_GDMA_RX_INTERRUPTS_MAC(_tile, RX_BUFFER),                                                                         \
 /* 64 uniEvents for 64 queues , extData = queueId */                                                                 \
 AAS_ALL_GDMA_RX_INTERRUPTS_MAC(_tile, RX_ERR),                                                                         \
                                                                                                                        \
 CPSS_PP_PORT_TX_FIFO_UNDERRUN_E,                                                                                       \
    /* MTI total ports 0..255 */                                                                                        \
    AAS_SET_INDEX_EQUAL_EVENT_PER_MTI_PORTS(_tile,0, MAC_TX_UNDERFLOW_E),                                                 \
    AAS_SET_INDEX_EQUAL_EVENT_PER_MTI_PORTS(_tile,1, MAC_TX_UNDERFLOW_E),                                                 \
    AAS_HALF_DP_SET_INDEX_EQUAL_EVENT_PER_MTI_PORTS(_tile,2, MAC_TX_UNDERFLOW_E),                                         \
    /* seg ports*/                                                                                                      \
    AAS_SET_INDEX_EQUAL_EVENT_PER_SEG_PORTS(_tile,0, MAC_TX_UNDERFLOW_E),                                                 \
    AAS_SET_INDEX_EQUAL_EVENT_PER_SEG_PORTS(_tile,1, MAC_TX_UNDERFLOW_E),                                                 \
    AAS_HALF_DP_SET_INDEX_EQUAL_EVENT_PER_SEG_PORTS(_tile,2, MAC_TX_UNDERFLOW_E),                                         \
                                                                                                                        \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_PORT_TX_FIFO_OVERRUN_E,                                                                                        \
    /* MTI total ports 0..255 */                                                                                        \
    AAS_SET_INDEX_EQUAL_EVENT_PER_MTI_PORTS(_tile,0, EMAC_TX_OVR_ERR_E),                                                  \
    AAS_SET_INDEX_EQUAL_EVENT_PER_MTI_PORTS(_tile,1, EMAC_TX_OVR_ERR_E),                                                  \
    AAS_HALF_DP_SET_INDEX_EQUAL_EVENT_PER_MTI_PORTS(_tile,2, EMAC_TX_OVR_ERR_E),                                          \
    /* seg ports*/                                                                                                      \
    AAS_SET_INDEX_EQUAL_EVENT_PER_SEG_PORTS(_tile,0, MAC_TX_OVR_ERR_E),                                                   \
    AAS_SET_INDEX_EQUAL_EVENT_PER_SEG_PORTS(_tile,1, MAC_TX_OVR_ERR_E),                                                 \
    AAS_HALF_DP_SET_INDEX_EQUAL_EVENT_PER_SEG_PORTS(_tile,2, MAC_TX_OVR_ERR_E),                                           \
                                                                                                                        \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_PORT_RX_FIFO_OVERRUN_E,                                                                                        \
    /* MTI total ports 0..255 */                                                                                        \
    AAS_SET_INDEX_EQUAL_EVENT_PER_MTI_PORTS(_tile,0, EMAC_RX_OVERRUN_E),                                                  \
    AAS_SET_INDEX_EQUAL_EVENT_PER_MTI_PORTS(_tile,1, EMAC_RX_OVERRUN_E),                                                  \
    AAS_HALF_DP_SET_INDEX_EQUAL_EVENT_PER_MTI_PORTS(_tile,2, EMAC_RX_OVERRUN_E),                                          \
    /* seg ports*/                                                                                                      \
    AAS_SET_INDEX_EQUAL_EVENT_PER_SEG_PORTS(_tile,0, RX_OVERRUN_E),                                                       \
    AAS_SET_INDEX_EQUAL_EVENT_PER_SEG_PORTS(_tile,1, RX_OVERRUN_E),                                                 \
    AAS_HALF_DP_SET_INDEX_EQUAL_EVENT_PER_SEG_PORTS(_tile,2, RX_OVERRUN_E),                                               \
                                                                                                                        \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_CNC_WRAPAROUND_BLOCK_E,                                                                                        \
    AAS_SET_EVENT_WITH_INDEX_MAC(_tile,CNC_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_BLOCK_0_WRAPAROUND_E,  (16*0 +  0)),  \
    AAS_SET_EVENT_WITH_INDEX_MAC(_tile,CNC_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_BLOCK_1_WRAPAROUND_E,  (16*0 +  1)),  \
    AAS_SET_EVENT_WITH_INDEX_MAC(_tile,CNC_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_BLOCK_2_WRAPAROUND_E,  (16*0 +  2)),  \
    AAS_SET_EVENT_WITH_INDEX_MAC(_tile,CNC_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_BLOCK_3_WRAPAROUND_E,  (16*0 +  3)),  \
    AAS_SET_EVENT_WITH_INDEX_MAC(_tile,CNC_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_BLOCK_4_WRAPAROUND_E,  (16*0 +  4)),  \
    AAS_SET_EVENT_WITH_INDEX_MAC(_tile,CNC_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_BLOCK_5_WRAPAROUND_E,  (16*0 +  5)),  \
    AAS_SET_EVENT_WITH_INDEX_MAC(_tile,CNC_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_BLOCK_6_WRAPAROUND_E,  (16*0 +  6)),  \
    AAS_SET_EVENT_WITH_INDEX_MAC(_tile,CNC_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_BLOCK_7_WRAPAROUND_E,  (16*0 +  7)),  \
    AAS_SET_EVENT_WITH_INDEX_MAC(_tile,CNC_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_BLOCK_8_WRAPAROUND_E,  (16*0 +  8)),  \
    AAS_SET_EVENT_WITH_INDEX_MAC(_tile,CNC_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_BLOCK_9_WRAPAROUND_E,  (16*0 +  9)),  \
    AAS_SET_EVENT_WITH_INDEX_MAC(_tile,CNC_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_BLOCK_10_WRAPAROUND_E, (16*0 + 10)),  \
    AAS_SET_EVENT_WITH_INDEX_MAC(_tile,CNC_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_BLOCK_11_WRAPAROUND_E, (16*0 + 11)),  \
    AAS_SET_EVENT_WITH_INDEX_MAC(_tile,CNC_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_BLOCK_12_WRAPAROUND_E, (16*0 + 12)),  \
    AAS_SET_EVENT_WITH_INDEX_MAC(_tile,CNC_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_BLOCK_13_WRAPAROUND_E, (16*0 + 13)),  \
    AAS_SET_EVENT_WITH_INDEX_MAC(_tile,CNC_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_BLOCK_14_WRAPAROUND_E, (16*0 + 14)),  \
    AAS_SET_EVENT_WITH_INDEX_MAC(_tile,CNC_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_REGISTER_BLOCK_15_WRAPAROUND_E, (16*0 + 15)),  \
                                                                                                                                  \
 MARK_END_OF_UNI_EV_CNS,                                                                                          \
                                                                                                                  \
 CPSS_PP_SCT_RATE_LIMITER_E,                                                                                      \
    AAS_SET_EVENT_PER_ALL_PORTS_ALL_REGISTERS_MAC(_tile,PKT_DROPED_INT_E),                                          \
                                                                                                                  \
 MARK_END_OF_UNI_EV_CNS,                                                                                          \
                                                                                                                  \
 CPSS_PP_POLICER_ADDR_OUT_OF_MEMORY_E,                                                                            \
    AAS_SET_EVENT_WITH_INDEX_MAC(_tile,IPLR0_POLICER_INTERRUPT_CAUSE_ILLEGAL_ACCESS_E,       0),                    \
    AAS_SET_EVENT_WITH_INDEX_MAC(_tile,IPLR1_POLICER_INTERRUPT_CAUSE_ILLEGAL_ACCESS_E,       1),                    \
    AAS_SET_EVENT_WITH_INDEX_MAC(_tile,EPLR_POLICER_INTERRUPT_CAUSE_ILLEGAL_ACCESS_E,        2),                    \
                                                                                                                  \
 MARK_END_OF_UNI_EV_CNS,                                                                                          \
                                                                                                                  \
 CPSS_PP_POLICER_IPFIX_WRAP_AROUND_E,                                                                             \
    AAS_SET_EVENT_WITH_INDEX_MAC(_tile,IPLR0_POLICER_INTERRUPT_CAUSE_IPFIX_WRAPAROUND_E,            0 ),            \
    AAS_SET_EVENT_WITH_INDEX_MAC(_tile,IPLR1_POLICER_INTERRUPT_CAUSE_IPFIX_WRAPAROUND_E,            1 ),            \
    AAS_SET_EVENT_WITH_INDEX_MAC(_tile,EPLR_POLICER_INTERRUPT_CAUSE_IPFIX_WRAPAROUND_E,             2 ),            \
                                                                                                                  \
 MARK_END_OF_UNI_EV_CNS,                                                                                          \
                                                                                                                  \
 CPSS_PP_POLICER_IPFIX_ALARM_E,                                                                                   \
    AAS_SET_EVENT_WITH_INDEX_MAC(_tile,IPLR0_POLICER_INTERRUPT_CAUSE_IPFIX_ALARM_E,                 0 ),            \
    AAS_SET_EVENT_WITH_INDEX_MAC(_tile,IPLR1_POLICER_INTERRUPT_CAUSE_IPFIX_ALARM_E,                 1 ),            \
    AAS_SET_EVENT_WITH_INDEX_MAC(_tile,EPLR_POLICER_INTERRUPT_CAUSE_IPFIX_ALARM_E,                  2 ),            \
                                                                                                                  \
 MARK_END_OF_UNI_EV_CNS,                                                                                          \
                                                                                                                  \
 CPSS_PP_POLICER_IPFIX_ALARMED_ENTRIES_FIFO_WRAP_AROUND_E,                                                        \
    AAS_SET_EVENT_WITH_INDEX_MAC(_tile,IPLR0_POLICER_INTERRUPT_CAUSE_IPFIX_ALARMED_ENTRIES_FIFO_WRAPAROUND_E,    0 ),  \
    AAS_SET_EVENT_WITH_INDEX_MAC(_tile,IPLR1_POLICER_INTERRUPT_CAUSE_IPFIX_ALARMED_ENTRIES_FIFO_WRAPAROUND_E,    1 ),  \
    AAS_SET_EVENT_WITH_INDEX_MAC(_tile,EPLR_POLICER_INTERRUPT_CAUSE_IPFIX_ALARMED_ENTRIES_FIFO_WRAPAROUND_E,     2 ),  \
                                                                                                                  \
 MARK_END_OF_UNI_EV_CNS,                                                                                          \
                                                                                                                  \
 CPSS_PP_PCL_LOOKUP_DATA_ERROR_E,                                                                                 \
    AAS_SET_EVENT_WITH_INDEX_MAC(_tile,PCL_PCL_UNIT_INTERRUPT_CAUSE_PCL0_ACTION0_TCAM_ACCESS_DATA_ERROR_E,    0),   \
    AAS_SET_EVENT_WITH_INDEX_MAC(_tile,PCL_PCL_UNIT_INTERRUPT_CAUSE_PCL0_ACTION1_TCAM_ACCESS_DATA_ERROR_E,    0),   \
    AAS_SET_EVENT_WITH_INDEX_MAC(_tile,PCL_PCL_UNIT_INTERRUPT_CAUSE_PCL0_ACTION2_TCAM_ACCESS_DATA_ERROR_E,    0),   \
    AAS_SET_EVENT_WITH_INDEX_MAC(_tile,PCL_PCL_UNIT_INTERRUPT_CAUSE_PCL0_ACTION3_TCAM_ACCESS_DATA_ERROR_E,    0),   \
                                                                                                                  \
    AAS_SET_EVENT_WITH_INDEX_MAC(_tile,PCL_PCL_UNIT_INTERRUPT_CAUSE_PCL1_ACTION0_TCAM_ACCESS_DATA_ERROR_E,    1),   \
    AAS_SET_EVENT_WITH_INDEX_MAC(_tile,PCL_PCL_UNIT_INTERRUPT_CAUSE_PCL1_ACTION1_TCAM_ACCESS_DATA_ERROR_E,    1),   \
    AAS_SET_EVENT_WITH_INDEX_MAC(_tile,PCL_PCL_UNIT_INTERRUPT_CAUSE_PCL1_ACTION2_TCAM_ACCESS_DATA_ERROR_E,    1),   \
    AAS_SET_EVENT_WITH_INDEX_MAC(_tile,PCL_PCL_UNIT_INTERRUPT_CAUSE_PCL1_ACTION3_TCAM_ACCESS_DATA_ERROR_E,    1),   \
                                                                                                                  \
    AAS_SET_EVENT_WITH_INDEX_MAC(_tile,PCL_PCL_UNIT_INTERRUPT_CAUSE_PCL2_ACTION0_TCAM_ACCESS_DATA_ERROR_E,    2),   \
    AAS_SET_EVENT_WITH_INDEX_MAC(_tile,PCL_PCL_UNIT_INTERRUPT_CAUSE_PCL2_ACTION1_TCAM_ACCESS_DATA_ERROR_E,    2),   \
    AAS_SET_EVENT_WITH_INDEX_MAC(_tile,PCL_PCL_UNIT_INTERRUPT_CAUSE_PCL2_ACTION2_TCAM_ACCESS_DATA_ERROR_E,    2),   \
    AAS_SET_EVENT_WITH_INDEX_MAC(_tile,PCL_PCL_UNIT_INTERRUPT_CAUSE_PCL2_ACTION3_TCAM_ACCESS_DATA_ERROR_E,    2),   \
                                                                                                                  \
 MARK_END_OF_UNI_EV_CNS,                                                                                          \
                                                                                                                  \
 CPSS_PP_PCL_LOOKUP_FIFO_FULL_E,                                                                                  \
    AAS_SET_EVENT_WITH_INDEX_MAC(_tile,PCL_PCL_UNIT_INTERRUPT_CAUSE_LOOKUP0_FIFO_FULL_E,                      0),   \
    AAS_SET_EVENT_WITH_INDEX_MAC(_tile,PCL_PCL_UNIT_INTERRUPT_CAUSE_LOOKUP1_FIFO_FULL_E,                      1),   \
    AAS_SET_EVENT_WITH_INDEX_MAC(_tile,PCL_PCL_UNIT_INTERRUPT_CAUSE_LOOKUP2_FIFO_FULL_E,                      2),   \
                                                                                                                  \
 MARK_END_OF_UNI_EV_CNS,                                                                                          \
                                                                                                                  \
 CPSS_PP_PCL_ACTION_TRIGGERED_E,                                                                                  \
    AAS_SET_EVENT_WITH_INDEX_MAC(_tile,PCL_PCL_UNIT_INTERRUPT_CAUSE_IPCL0_TCAM_TRIGGERED_INTERRUPT_E,         0),   \
    AAS_SET_EVENT_WITH_INDEX_MAC(_tile,PCL_PCL_UNIT_INTERRUPT_CAUSE_IPCL1_TCAM_TRIGGERED_INTERRUPT_E,         1),   \
    AAS_SET_EVENT_WITH_INDEX_MAC(_tile,PCL_PCL_UNIT_INTERRUPT_CAUSE_IPCL2_TCAM_TRIGGERED_INTERRUPT_E,         2),   \
 MARK_END_OF_UNI_EV_CNS,                                                                                          \
                                                                                                                  \
 CPSS_PP_EB_NA_FIFO_FULL_E,                                                                                       \
    AAS_SET_EVENT_WITH_INDEX_MAC(_tile,FDB_FDB_INTERRUPT_CAUSE_REGISTER_PIPE_NUMBER_0_NA_FIFO_FULL_E,         0),   \
    AAS_SET_EVENT_WITH_INDEX_MAC(_tile,FDB_FDB_INTERRUPT_CAUSE_REGISTER_PIPE_NUMBER_1_NA_FIFO_FULL_E,         1),   \
    AAS_SET_EVENT_WITH_INDEX_MAC(_tile,FDB_FDB_INTERRUPT_CAUSE_REGISTER_PIPE_NUMBER_2_NA_FIFO_FULL_E,         2),   \
    AAS_SET_EVENT_WITH_INDEX_MAC(_tile,FDB_FDB_INTERRUPT_CAUSE_REGISTER_PIPE_NUMBER_3_NA_FIFO_FULL_E,         3),   \
    AAS_SET_EVENT_WITH_INDEX_MAC(_tile,FDB_FDB_INTERRUPT_CAUSE_REGISTER_PIPE_NUMBER_4_NA_FIFO_FULL_E,         4),   \
    AAS_SET_EVENT_WITH_INDEX_MAC(_tile,FDB_FDB_INTERRUPT_CAUSE_REGISTER_PIPE_NUMBER_5_NA_FIFO_FULL_E,         5),   \
    AAS_SET_EVENT_WITH_INDEX_MAC(_tile,FDB_FDB_INTERRUPT_CAUSE_REGISTER_PIPE_NUMBER_6_NA_FIFO_FULL_E,         6),   \
    AAS_SET_EVENT_WITH_INDEX_MAC(_tile,FDB_FDB_INTERRUPT_CAUSE_REGISTER_PIPE_NUMBER_7_NA_FIFO_FULL_E,         7),   \
 MARK_END_OF_UNI_EV_CNS,                                                                                          \
                                                                                                                  \
 CPSS_PP_TTI_ACCESS_DATA_ERROR_E,                                                                                 \
    AAS_SET_EVENT_WITH_INDEX_MAC(_tile,TTI_TTI_ENGINE_INTERRUPT_CAUSE_TTIACTION_0_ACCESS_DATA_ERROR_E, 0),          \
    AAS_SET_EVENT_WITH_INDEX_MAC(_tile,TTI_TTI_ENGINE_INTERRUPT_CAUSE_TTIACTION_1_ACCESS_DATA_ERROR_E, 1),          \
    AAS_SET_EVENT_WITH_INDEX_MAC(_tile,TTI_TTI_ENGINE_INTERRUPT_CAUSE_TTIACTION_2_ACCESS_DATA_ERROR_E, 2),          \
    AAS_SET_EVENT_WITH_INDEX_MAC(_tile,TTI_TTI_ENGINE_INTERRUPT_CAUSE_TTIACTION_3_ACCESS_DATA_ERROR_E, 3),          \
                                                                                                                        \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 /* next HW interrupt value will have same index (evExtData) value in CPSS_PP_CRITICAL_HW_ERROR_E */                    \
 CPSS_PP_CRITICAL_HW_ERROR_E,                                                                                           \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_INTERRUPT_MISC_CAUSE_CELL_READ_CELL_CREDIT_COUNTER_OVERFLOW_E), \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_INTERRUPT_MISC_CAUSE_CELL_READ_CELL_CREDIT_COUNTER_OVERFLOW_E), \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_INTERRUPT_MISC_CAUSE_CELL_READ_CELL_CREDIT_COUNTER_OVERFLOW_E), \
     /*PSI_REG*/                                                                                                         \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,PSI_REGS_PSI_INTERRUPT_CAUSE_SCHEDULER_INTERRUPT_E),                          \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,PSI_REGS_PSI_INTERRUPT_CAUSE_MG2SNAKE_INTERRUPT_E),                           \
\
    /*TXQ_QFC*/                                                                                                         \
    /*TXQ_PFCC*/                                                                                                        \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,TXQ_PR_PFCC_INTERRUPT_CAUSE_POOL_0_COUNTER_OVERFLOW_E),                       \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,TXQ_PR_PFCC_INTERRUPT_CAUSE_SOURCE_PORT_COUNTER_OVERFLOW_E),                  \
\
    /*HBU*/                                                                                                             \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,HBU_HBU_INTERRUPT_CAUSE_REGISTER_ERROR_FHF_MEM_E),                            \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,HBU_HBU_INTERRUPT_CAUSE_REGISTER_ERROR_CMT1_MEM_E),                           \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,HBU_HBU_INTERRUPT_CAUSE_REGISTER_ERROR_CMT2_MEM_E),                           \
\
    /*TXQ_SDQ*/                                                                                                         \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SDQ_INST_0_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UN_MAP_QUEUE_CREDIT_E),\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SDQ_INST_0_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UN_MAP_QUEUE_ENQ_E),   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SDQ_INST_0_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UN_MAP_QUEUE_DEQ_E),   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SDQ_INST_0_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UN_MAP_QUEUE_QCN_E),   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SDQ_INST_0_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UN_MAP_PORT_CREDIT_E), \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SDQ_INST_0_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UN_MAP_PORT_ENQ_E),    \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SDQ_INST_0_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UN_MAP_PORT_DEQ_E),    \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SDQ_INST_0_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UN_MAP_PORT_QCN_E),    \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SDQ_INST_0_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UN_MAP_PORT_PFC_E),    \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SDQ_INST_0_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_SEL_PORT_OUTOFRANGE_E),\
\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SDQ_INST_1_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UN_MAP_QUEUE_CREDIT_E),\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SDQ_INST_1_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UN_MAP_QUEUE_ENQ_E),   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SDQ_INST_1_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UN_MAP_QUEUE_DEQ_E),   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SDQ_INST_1_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UN_MAP_QUEUE_QCN_E),   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SDQ_INST_1_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UN_MAP_PORT_CREDIT_E), \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SDQ_INST_1_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UN_MAP_PORT_ENQ_E),    \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SDQ_INST_1_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UN_MAP_PORT_DEQ_E),    \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SDQ_INST_1_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UN_MAP_PORT_QCN_E),    \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SDQ_INST_1_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UN_MAP_PORT_PFC_E),    \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SDQ_INST_1_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_SEL_PORT_OUTOFRANGE_E),\
\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SDQ_INST_2_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UN_MAP_QUEUE_CREDIT_E),\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SDQ_INST_2_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UN_MAP_QUEUE_ENQ_E),   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SDQ_INST_2_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UN_MAP_QUEUE_DEQ_E),   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SDQ_INST_2_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UN_MAP_QUEUE_QCN_E),   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SDQ_INST_2_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UN_MAP_PORT_CREDIT_E), \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SDQ_INST_2_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UN_MAP_PORT_ENQ_E),    \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SDQ_INST_2_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UN_MAP_PORT_DEQ_E),    \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SDQ_INST_2_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UN_MAP_PORT_QCN_E),    \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SDQ_INST_2_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UN_MAP_PORT_PFC_E),    \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SDQ_INST_2_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_SEL_PORT_OUTOFRANGE_E),\
\
    /*TXQ_PDS*/                                                                                                                              \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,PDS_UNIT_0_PDS_INTERRUPT_DEBUG_CAUSE_PB_WRITE_REPLY_FIFO_FULL_E),                     \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,PDS_UNIT_0_PDS_INTERRUPT_DEBUG_CAUSE_PB_FIFO_STATE_ORDER_FIFO_FULL_E),                \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,PDS_UNIT_0_PDS_INTERRUPT_DEBUG_CAUSE_PB_FIFO_DATA_ORDER_FIFO_FULL_E),                 \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,PDS_UNIT_0_PDS_INTERRUPT_DEBUG_CAUSE_FRAG_ON_THE_AIR_CNTR_FULL_INT_E),                \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,PDS_UNIT_0_PDS_INTERRUPT_DEBUG_CAUSE_DATAS_BMX_ADDR_OUT_OF_RANGE_INT_E),              \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,PDS_UNIT_0_PDS_INTERRUPT_DEBUG_CAUSE_DATAS_BMX_SAME_WR_ADDR_INT_E),                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,PDS_UNIT_0_PDS_INTERRUPT_DEBUG_CAUSE_TOTAL_PDS_CNTR_OVRRUN_INT_E),                    \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,PDS_UNIT_0_PDS_INTERRUPT_DEBUG_CAUSE_PDS_CACHE_CNTR_OVRRUN_INT_E),                    \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,PDS_UNIT_0_PDS_INTERRUPT_FUNCTIONAL_CAUSE_PB_ECC_DOUBLE_ERROR_INT_E),                 \
\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,PDS_UNIT_1_PDS_INTERRUPT_DEBUG_CAUSE_PB_WRITE_REPLY_FIFO_FULL_E),                     \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,PDS_UNIT_1_PDS_INTERRUPT_DEBUG_CAUSE_PB_FIFO_STATE_ORDER_FIFO_FULL_E),                \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,PDS_UNIT_1_PDS_INTERRUPT_DEBUG_CAUSE_PB_FIFO_DATA_ORDER_FIFO_FULL_E),                 \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,PDS_UNIT_1_PDS_INTERRUPT_DEBUG_CAUSE_FRAG_ON_THE_AIR_CNTR_FULL_INT_E),                \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,PDS_UNIT_1_PDS_INTERRUPT_DEBUG_CAUSE_DATAS_BMX_ADDR_OUT_OF_RANGE_INT_E),              \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,PDS_UNIT_1_PDS_INTERRUPT_DEBUG_CAUSE_DATAS_BMX_SAME_WR_ADDR_INT_E),                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,PDS_UNIT_1_PDS_INTERRUPT_DEBUG_CAUSE_TOTAL_PDS_CNTR_OVRRUN_INT_E),                    \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,PDS_UNIT_1_PDS_INTERRUPT_DEBUG_CAUSE_PDS_CACHE_CNTR_OVRRUN_INT_E),                    \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,PDS_UNIT_1_PDS_INTERRUPT_FUNCTIONAL_CAUSE_PB_ECC_DOUBLE_ERROR_INT_E),                 \
\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,PDS_UNIT_2_PDS_INTERRUPT_DEBUG_CAUSE_PB_WRITE_REPLY_FIFO_FULL_E),                     \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,PDS_UNIT_2_PDS_INTERRUPT_DEBUG_CAUSE_PB_FIFO_STATE_ORDER_FIFO_FULL_E),                \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,PDS_UNIT_2_PDS_INTERRUPT_DEBUG_CAUSE_PB_FIFO_DATA_ORDER_FIFO_FULL_E),                 \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,PDS_UNIT_2_PDS_INTERRUPT_DEBUG_CAUSE_FRAG_ON_THE_AIR_CNTR_FULL_INT_E),                \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,PDS_UNIT_2_PDS_INTERRUPT_DEBUG_CAUSE_DATAS_BMX_ADDR_OUT_OF_RANGE_INT_E),              \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,PDS_UNIT_2_PDS_INTERRUPT_DEBUG_CAUSE_DATAS_BMX_SAME_WR_ADDR_INT_E),                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,PDS_UNIT_2_PDS_INTERRUPT_DEBUG_CAUSE_TOTAL_PDS_CNTR_OVRRUN_INT_E),                    \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,PDS_UNIT_2_PDS_INTERRUPT_DEBUG_CAUSE_PDS_CACHE_CNTR_OVRRUN_INT_E),                    \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,PDS_UNIT_2_PDS_INTERRUPT_FUNCTIONAL_CAUSE_PB_ECC_DOUBLE_ERROR_INT_E),                 \
\
    /*GPC_CELL_READ*/                                                                                                                    \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_CELL_READ_INSTANCE_0_INTERRUPT_CAUSE_INTERFACE_0_CELL_READ_REQUEST_FIFO_UNDERFLOW_INTERFACE_E),\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_CELL_READ_INSTANCE_0_INTERRUPT_CAUSE_PACKET_COUNT_REQUEST_FIFO_UNDERFLOW_E),                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_CELL_READ_INSTANCE_0_INTERRUPT_CAUSE_PACKET_COUNT_REQUEST_FIFO_OVERFLOW_E),                    \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_CELL_READ_INSTANCE_0_INTERRUPT_CAUSE_CAM_MULTIPLE_MATCHES_E),                                  \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_CELL_READ_INSTANCE_0_INTERRUPT_CAUSE_CELL_REFERENCE_FREE_LIST_UNDERFLOW_E),                    \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_CELL_READ_INSTANCE_0_INTERRUPT_CAUSE_CELL_REFERENCE_FREE_LIST_OVERFLOW_E),                     \
\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_CELL_READ_INSTANCE_1_INTERRUPT_CAUSE_INTERFACE_0_CELL_READ_REQUEST_FIFO_UNDERFLOW_INTERFACE_E),\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_CELL_READ_INSTANCE_1_INTERRUPT_CAUSE_PACKET_COUNT_REQUEST_FIFO_UNDERFLOW_E),                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_CELL_READ_INSTANCE_1_INTERRUPT_CAUSE_PACKET_COUNT_REQUEST_FIFO_OVERFLOW_E),                    \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_CELL_READ_INSTANCE_1_INTERRUPT_CAUSE_CAM_MULTIPLE_MATCHES_E),                                  \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_CELL_READ_INSTANCE_1_INTERRUPT_CAUSE_CELL_REFERENCE_FREE_LIST_UNDERFLOW_E),                    \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_CELL_READ_INSTANCE_1_INTERRUPT_CAUSE_CELL_REFERENCE_FREE_LIST_OVERFLOW_E),                     \
\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_CELL_READ_INSTANCE_2_INTERRUPT_CAUSE_INTERFACE_0_CELL_READ_REQUEST_FIFO_UNDERFLOW_INTERFACE_E),\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_CELL_READ_INSTANCE_2_INTERRUPT_CAUSE_PACKET_COUNT_REQUEST_FIFO_UNDERFLOW_E),                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_CELL_READ_INSTANCE_2_INTERRUPT_CAUSE_PACKET_COUNT_REQUEST_FIFO_OVERFLOW_E),                    \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_CELL_READ_INSTANCE_2_INTERRUPT_CAUSE_CAM_MULTIPLE_MATCHES_E),                                  \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_CELL_READ_INSTANCE_2_INTERRUPT_CAUSE_CELL_REFERENCE_FREE_LIST_UNDERFLOW_E),                    \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_CELL_READ_INSTANCE_2_INTERRUPT_CAUSE_CELL_REFERENCE_FREE_LIST_OVERFLOW_E),                     \
\
    /*NPM_MC*/                                                                                                                           \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_0_0_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_0_1_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_0_2_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_0_3_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_0_4_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_1_0_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_1_1_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_1_2_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_1_3_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_1_4_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_2_0_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_2_1_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_2_2_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_2_3_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_2_4_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_3_0_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_3_1_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_3_2_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_3_3_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_3_4_E),                      \
    \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_0_0_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_0_1_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_0_2_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_0_3_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_0_4_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_1_0_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_1_1_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_1_2_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_1_3_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_1_4_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_2_0_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_2_1_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_2_2_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_2_3_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_2_4_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_3_0_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_3_1_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_3_2_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_3_3_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_UNIT_ERROR_CAUSE_INT_UNIT_ERR_CAUSE_3_4_E),                      \
\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_0_0_E),                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_0_1_E),                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_0_2_E),                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_0_3_E),                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_0_4_E),                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_1_0_E),                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_1_1_E),                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_1_2_E),                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_1_3_E),                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_1_4_E),                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_2_0_E),                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_2_1_E),                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_2_2_E),                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_2_3_E),                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_2_4_E),                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_3_0_E),                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_3_1_E),                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_3_2_E),                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_3_3_E),                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_3_4_E),                   \
\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_0_0_E),                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_0_1_E),                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_0_2_E),                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_0_3_E),                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_0_4_E),                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_1_0_E),                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_1_1_E),                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_1_2_E),                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_1_3_E),                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_1_4_E),                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_2_0_E),                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_2_1_E),                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_2_2_E),                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_2_3_E),                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_2_4_E),                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_3_0_E),                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_3_1_E),                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_3_2_E),                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_3_3_E),                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_REINSERT_FAIL_CAUSE_INT_REINFAIL_CAUSE_3_4_E),                   \
\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_0_0_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_0_1_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_0_2_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_0_3_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_0_4_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_1_0_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_1_1_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_1_2_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_1_3_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_1_4_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_2_0_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_2_1_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_2_2_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_2_3_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_2_4_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_3_0_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_3_1_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_3_2_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_3_3_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_3_4_E),                      \
\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_0_0_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_0_1_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_0_2_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_0_3_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_0_4_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_1_0_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_1_1_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_1_2_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_1_3_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_1_4_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_2_0_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_2_1_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_2_2_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_2_3_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_2_4_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_3_0_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_3_1_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_3_2_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_3_3_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_UNIT_REFS_CAUSE_INT_UNIT_REFS_CAUSE_3_4_E),                      \
\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_0_0_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_0_1_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_0_2_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_0_3_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_0_4_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_1_0_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_1_1_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_1_2_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_1_3_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_1_4_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_2_0_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_2_1_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_2_2_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_2_3_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_2_4_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_3_0_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_3_1_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_3_2_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_3_3_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_3_4_E),                      \
\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_REFS_ERROR_CAUSE_INT_REFS_ERR_CAUSE_0_0_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_REFS_ERROR_CAUSE_INT_REFS_ERR_CAUSE_0_1_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_REFS_ERROR_CAUSE_INT_REFS_ERR_CAUSE_0_2_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_REFS_ERROR_CAUSE_INT_REFS_ERR_CAUSE_0_3_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_REFS_ERROR_CAUSE_INT_REFS_ERR_CAUSE_1_0_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_REFS_ERROR_CAUSE_INT_REFS_ERR_CAUSE_1_1_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_REFS_ERROR_CAUSE_INT_REFS_ERR_CAUSE_1_2_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_REFS_ERROR_CAUSE_INT_REFS_ERR_CAUSE_1_3_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_REFS_ERROR_CAUSE_INT_REFS_ERR_CAUSE_2_0_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_REFS_ERROR_CAUSE_INT_REFS_ERR_CAUSE_2_1_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_REFS_ERROR_CAUSE_INT_REFS_ERR_CAUSE_2_2_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_REFS_ERROR_CAUSE_INT_REFS_ERR_CAUSE_2_3_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_REFS_ERROR_CAUSE_INT_REFS_ERR_CAUSE_3_0_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_REFS_ERROR_CAUSE_INT_REFS_ERR_CAUSE_3_1_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_REFS_ERROR_CAUSE_INT_REFS_ERR_CAUSE_3_2_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_REFS_ERROR_CAUSE_INT_REFS_ERR_CAUSE_3_3_E),                      \
\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_AGING_ERROR_CAUSE_INT_AGING_ERR_CAUSE_0_0_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_AGING_ERROR_CAUSE_INT_AGING_ERR_CAUSE_0_1_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_AGING_ERROR_CAUSE_INT_AGING_ERR_CAUSE_0_2_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_AGING_ERROR_CAUSE_INT_AGING_ERR_CAUSE_0_3_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_AGING_ERROR_CAUSE_INT_AGING_ERR_CAUSE_1_0_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_AGING_ERROR_CAUSE_INT_AGING_ERR_CAUSE_1_1_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_AGING_ERROR_CAUSE_INT_AGING_ERR_CAUSE_1_2_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_AGING_ERROR_CAUSE_INT_AGING_ERR_CAUSE_1_3_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_AGING_ERROR_CAUSE_INT_AGING_ERR_CAUSE_2_0_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_AGING_ERROR_CAUSE_INT_AGING_ERR_CAUSE_2_1_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_AGING_ERROR_CAUSE_INT_AGING_ERR_CAUSE_2_2_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_AGING_ERROR_CAUSE_INT_AGING_ERR_CAUSE_2_3_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_AGING_ERROR_CAUSE_INT_AGING_ERR_CAUSE_3_0_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_AGING_ERROR_CAUSE_INT_AGING_ERR_CAUSE_3_1_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_AGING_ERROR_CAUSE_INT_AGING_ERR_CAUSE_3_2_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_AGING_ERROR_CAUSE_INT_AGING_ERR_CAUSE_3_3_E),                      \
\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_REFS_FIFO_CAUSE_INT_REFS_FIFO_CAUSE_0_0_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_REFS_FIFO_CAUSE_INT_REFS_FIFO_CAUSE_0_1_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_REFS_FIFO_CAUSE_INT_REFS_FIFO_CAUSE_0_2_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_REFS_FIFO_CAUSE_INT_REFS_FIFO_CAUSE_0_3_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_REFS_FIFO_CAUSE_INT_REFS_FIFO_CAUSE_1_0_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_REFS_FIFO_CAUSE_INT_REFS_FIFO_CAUSE_1_1_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_REFS_FIFO_CAUSE_INT_REFS_FIFO_CAUSE_1_2_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_REFS_FIFO_CAUSE_INT_REFS_FIFO_CAUSE_1_3_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_REFS_FIFO_CAUSE_INT_REFS_FIFO_CAUSE_2_0_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_REFS_FIFO_CAUSE_INT_REFS_FIFO_CAUSE_2_1_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_REFS_FIFO_CAUSE_INT_REFS_FIFO_CAUSE_2_2_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_REFS_FIFO_CAUSE_INT_REFS_FIFO_CAUSE_2_3_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_REFS_FIFO_CAUSE_INT_REFS_FIFO_CAUSE_3_0_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_REFS_FIFO_CAUSE_INT_REFS_FIFO_CAUSE_3_1_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_REFS_FIFO_CAUSE_INT_REFS_FIFO_CAUSE_3_2_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_REFS_FIFO_CAUSE_INT_REFS_FIFO_CAUSE_3_3_E),                      \
\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_LIST_ERROR_CAUSE_INT_LIST_ERR_CAUSE_0_E),                           \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_LIST_ERROR_CAUSE_INT_LIST_ERR_CAUSE_1_E),                           \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_LIST_ERROR_CAUSE_INT_LIST_ERR_CAUSE_2_E),                           \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_0_INTERRUPT_LIST_ERROR_CAUSE_INT_LIST_ERR_CAUSE_3_E),                           \
\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_0_0_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_0_1_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_0_2_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_0_3_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_0_4_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_1_0_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_1_1_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_1_2_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_1_3_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_1_4_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_2_0_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_2_1_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_2_2_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_2_3_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_2_4_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_3_0_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_3_1_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_3_2_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_3_3_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_UNIT_DATA_CAUSE_INT_UNIT_DATA_CAUSE_3_4_E),                      \
\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_REFS_ERROR_CAUSE_INT_REFS_ERR_CAUSE_0_0_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_REFS_ERROR_CAUSE_INT_REFS_ERR_CAUSE_0_1_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_REFS_ERROR_CAUSE_INT_REFS_ERR_CAUSE_0_2_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_REFS_ERROR_CAUSE_INT_REFS_ERR_CAUSE_0_3_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_REFS_ERROR_CAUSE_INT_REFS_ERR_CAUSE_1_0_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_REFS_ERROR_CAUSE_INT_REFS_ERR_CAUSE_1_1_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_REFS_ERROR_CAUSE_INT_REFS_ERR_CAUSE_1_2_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_REFS_ERROR_CAUSE_INT_REFS_ERR_CAUSE_1_3_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_REFS_ERROR_CAUSE_INT_REFS_ERR_CAUSE_2_0_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_REFS_ERROR_CAUSE_INT_REFS_ERR_CAUSE_2_1_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_REFS_ERROR_CAUSE_INT_REFS_ERR_CAUSE_2_2_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_REFS_ERROR_CAUSE_INT_REFS_ERR_CAUSE_2_3_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_REFS_ERROR_CAUSE_INT_REFS_ERR_CAUSE_3_0_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_REFS_ERROR_CAUSE_INT_REFS_ERR_CAUSE_3_1_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_REFS_ERROR_CAUSE_INT_REFS_ERR_CAUSE_3_2_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_REFS_ERROR_CAUSE_INT_REFS_ERR_CAUSE_3_3_E),                      \
\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_AGING_ERROR_CAUSE_INT_AGING_ERR_CAUSE_0_0_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_AGING_ERROR_CAUSE_INT_AGING_ERR_CAUSE_0_1_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_AGING_ERROR_CAUSE_INT_AGING_ERR_CAUSE_0_2_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_AGING_ERROR_CAUSE_INT_AGING_ERR_CAUSE_0_3_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_AGING_ERROR_CAUSE_INT_AGING_ERR_CAUSE_1_0_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_AGING_ERROR_CAUSE_INT_AGING_ERR_CAUSE_1_1_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_AGING_ERROR_CAUSE_INT_AGING_ERR_CAUSE_1_2_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_AGING_ERROR_CAUSE_INT_AGING_ERR_CAUSE_1_3_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_AGING_ERROR_CAUSE_INT_AGING_ERR_CAUSE_2_0_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_AGING_ERROR_CAUSE_INT_AGING_ERR_CAUSE_2_1_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_AGING_ERROR_CAUSE_INT_AGING_ERR_CAUSE_2_2_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_AGING_ERROR_CAUSE_INT_AGING_ERR_CAUSE_2_3_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_AGING_ERROR_CAUSE_INT_AGING_ERR_CAUSE_3_0_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_AGING_ERROR_CAUSE_INT_AGING_ERR_CAUSE_3_1_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_AGING_ERROR_CAUSE_INT_AGING_ERR_CAUSE_3_2_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_AGING_ERROR_CAUSE_INT_AGING_ERR_CAUSE_3_3_E),                      \
\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_REFS_FIFO_CAUSE_INT_REFS_FIFO_CAUSE_0_0_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_REFS_FIFO_CAUSE_INT_REFS_FIFO_CAUSE_0_1_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_REFS_FIFO_CAUSE_INT_REFS_FIFO_CAUSE_0_2_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_REFS_FIFO_CAUSE_INT_REFS_FIFO_CAUSE_0_3_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_REFS_FIFO_CAUSE_INT_REFS_FIFO_CAUSE_1_0_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_REFS_FIFO_CAUSE_INT_REFS_FIFO_CAUSE_1_1_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_REFS_FIFO_CAUSE_INT_REFS_FIFO_CAUSE_1_2_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_REFS_FIFO_CAUSE_INT_REFS_FIFO_CAUSE_1_3_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_REFS_FIFO_CAUSE_INT_REFS_FIFO_CAUSE_2_0_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_REFS_FIFO_CAUSE_INT_REFS_FIFO_CAUSE_2_1_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_REFS_FIFO_CAUSE_INT_REFS_FIFO_CAUSE_2_2_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_REFS_FIFO_CAUSE_INT_REFS_FIFO_CAUSE_2_3_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_REFS_FIFO_CAUSE_INT_REFS_FIFO_CAUSE_3_0_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_REFS_FIFO_CAUSE_INT_REFS_FIFO_CAUSE_3_1_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_REFS_FIFO_CAUSE_INT_REFS_FIFO_CAUSE_3_2_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_REFS_FIFO_CAUSE_INT_REFS_FIFO_CAUSE_3_3_E),                      \
\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_LIST_ERROR_CAUSE_INT_LIST_ERR_CAUSE_0_E),                           \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_LIST_ERROR_CAUSE_INT_LIST_ERR_CAUSE_1_E),                           \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_LIST_ERROR_CAUSE_INT_LIST_ERR_CAUSE_2_E),                           \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,NPM_MC_INSTANCE_1_INTERRUPT_LIST_ERROR_CAUSE_INT_LIST_ERR_CAUSE_3_E),                           \
\
    /*SMB_MC*/                                                                                                                           \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SMB_MC_INSTANCE_0_INTERRUPT_MISC_CAUSE_INTERRUPT_ALIGN_WR_CMDS_FIFOS_OVERFLOW_E),                  \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SMB_MC_INSTANCE_0_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_0_INTERRUPT_RBW_ERR_E),                                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SMB_MC_INSTANCE_0_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_1_INTERRUPT_RBW_ERR_E),                                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SMB_MC_INSTANCE_0_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_2_INTERRUPT_RBW_ERR_E),                                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SMB_MC_INSTANCE_0_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_3_INTERRUPT_RBW_ERR_E),                                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SMB_MC_INSTANCE_0_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_4_INTERRUPT_RBW_ERR_E),                                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SMB_MC_INSTANCE_0_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_5_INTERRUPT_RBW_ERR_E),                                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SMB_MC_INSTANCE_0_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_6_INTERRUPT_RBW_ERR_E),                                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SMB_MC_INSTANCE_0_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_7_INTERRUPT_RBW_ERR_E),                                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SMB_MC_INSTANCE_0_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_8_INTERRUPT_RBW_ERR_E),                                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SMB_MC_INSTANCE_0_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_9_INTERRUPT_RBW_ERR_E),                                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SMB_MC_INSTANCE_0_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_10_INTERRUPT_RBW_ERR_E),                                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SMB_MC_INSTANCE_0_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_11_INTERRUPT_RBW_ERR_E),                                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SMB_MC_INSTANCE_0_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_12_INTERRUPT_RBW_ERR_E),                                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SMB_MC_INSTANCE_0_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_13_INTERRUPT_RBW_ERR_E),                                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SMB_MC_INSTANCE_0_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_14_INTERRUPT_RBW_ERR_E),                                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SMB_MC_INSTANCE_0_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_15_INTERRUPT_RBW_ERR_E),                                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SMB_MC_INSTANCE_0_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_0_REFCNT_SER_ERROR_E),                                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SMB_MC_INSTANCE_0_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_1_REFCNT_SER_ERROR_E),                                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SMB_MC_INSTANCE_0_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_2_REFCNT_SER_ERROR_E),                                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SMB_MC_INSTANCE_0_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_3_REFCNT_SER_ERROR_E),                                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SMB_MC_INSTANCE_0_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_4_REFCNT_SER_ERROR_E),                                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SMB_MC_INSTANCE_0_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_5_REFCNT_SER_ERROR_E),                                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SMB_MC_INSTANCE_0_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_6_REFCNT_SER_ERROR_E),                                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SMB_MC_INSTANCE_0_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_7_REFCNT_SER_ERROR_E),                                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SMB_MC_INSTANCE_0_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_8_REFCNT_SER_ERROR_E),                                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SMB_MC_INSTANCE_0_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_9_REFCNT_SER_ERROR_E),                                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SMB_MC_INSTANCE_0_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_10_REFCNT_SER_ERROR_E),                                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SMB_MC_INSTANCE_0_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_11_REFCNT_SER_ERROR_E),                                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SMB_MC_INSTANCE_0_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_12_REFCNT_SER_ERROR_E),                                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SMB_MC_INSTANCE_0_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_13_REFCNT_SER_ERROR_E),                                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SMB_MC_INSTANCE_0_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_14_REFCNT_SER_ERROR_E),                                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SMB_MC_INSTANCE_0_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_15_REFCNT_SER_ERROR_E),                                   \
\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SMB_MC_INSTANCE_1_INTERRUPT_MISC_CAUSE_INTERRUPT_ALIGN_WR_CMDS_FIFOS_OVERFLOW_E),                  \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SMB_MC_INSTANCE_1_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_0_INTERRUPT_RBW_ERR_E),                                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SMB_MC_INSTANCE_1_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_1_INTERRUPT_RBW_ERR_E),                                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SMB_MC_INSTANCE_1_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_2_INTERRUPT_RBW_ERR_E),                                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SMB_MC_INSTANCE_1_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_3_INTERRUPT_RBW_ERR_E),                                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SMB_MC_INSTANCE_1_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_4_INTERRUPT_RBW_ERR_E),                                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SMB_MC_INSTANCE_1_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_5_INTERRUPT_RBW_ERR_E),                                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SMB_MC_INSTANCE_1_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_6_INTERRUPT_RBW_ERR_E),                                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SMB_MC_INSTANCE_1_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_7_INTERRUPT_RBW_ERR_E),                                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SMB_MC_INSTANCE_1_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_8_INTERRUPT_RBW_ERR_E),                                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SMB_MC_INSTANCE_1_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_9_INTERRUPT_RBW_ERR_E),                                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SMB_MC_INSTANCE_1_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_10_INTERRUPT_RBW_ERR_E),                                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SMB_MC_INSTANCE_1_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_11_INTERRUPT_RBW_ERR_E),                                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SMB_MC_INSTANCE_1_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_12_INTERRUPT_RBW_ERR_E),                                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SMB_MC_INSTANCE_1_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_13_INTERRUPT_RBW_ERR_E),                                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SMB_MC_INSTANCE_1_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_14_INTERRUPT_RBW_ERR_E),                                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SMB_MC_INSTANCE_1_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_15_INTERRUPT_RBW_ERR_E),                                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SMB_MC_INSTANCE_1_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_0_REFCNT_SER_ERROR_E),                                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SMB_MC_INSTANCE_1_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_1_REFCNT_SER_ERROR_E),                                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SMB_MC_INSTANCE_1_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_2_REFCNT_SER_ERROR_E),                                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SMB_MC_INSTANCE_1_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_3_REFCNT_SER_ERROR_E),                                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SMB_MC_INSTANCE_1_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_4_REFCNT_SER_ERROR_E),                                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SMB_MC_INSTANCE_1_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_5_REFCNT_SER_ERROR_E),                                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SMB_MC_INSTANCE_1_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_6_REFCNT_SER_ERROR_E),                                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SMB_MC_INSTANCE_1_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_7_REFCNT_SER_ERROR_E),                                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SMB_MC_INSTANCE_1_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_8_REFCNT_SER_ERROR_E),                                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SMB_MC_INSTANCE_1_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_9_REFCNT_SER_ERROR_E),                                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SMB_MC_INSTANCE_1_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_10_REFCNT_SER_ERROR_E),                                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SMB_MC_INSTANCE_1_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_11_REFCNT_SER_ERROR_E),                                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SMB_MC_INSTANCE_1_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_12_REFCNT_SER_ERROR_E),                                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SMB_MC_INSTANCE_1_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_13_REFCNT_SER_ERROR_E),                                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SMB_MC_INSTANCE_1_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_14_REFCNT_SER_ERROR_E),                                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SMB_MC_INSTANCE_1_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_15_REFCNT_SER_ERROR_E),                                   \
\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SMB_MC_INSTANCE_2_INTERRUPT_MISC_CAUSE_INTERRUPT_ALIGN_WR_CMDS_FIFOS_OVERFLOW_E),                  \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SMB_MC_INSTANCE_2_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_0_INTERRUPT_RBW_ERR_E),                                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SMB_MC_INSTANCE_2_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_1_INTERRUPT_RBW_ERR_E),                                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SMB_MC_INSTANCE_2_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_2_INTERRUPT_RBW_ERR_E),                                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SMB_MC_INSTANCE_2_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_3_INTERRUPT_RBW_ERR_E),                                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SMB_MC_INSTANCE_2_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_4_INTERRUPT_RBW_ERR_E),                                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SMB_MC_INSTANCE_2_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_5_INTERRUPT_RBW_ERR_E),                                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SMB_MC_INSTANCE_2_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_6_INTERRUPT_RBW_ERR_E),                                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SMB_MC_INSTANCE_2_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_7_INTERRUPT_RBW_ERR_E),                                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SMB_MC_INSTANCE_2_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_8_INTERRUPT_RBW_ERR_E),                                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SMB_MC_INSTANCE_2_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_9_INTERRUPT_RBW_ERR_E),                                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SMB_MC_INSTANCE_2_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_10_INTERRUPT_RBW_ERR_E),                                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SMB_MC_INSTANCE_2_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_11_INTERRUPT_RBW_ERR_E),                                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SMB_MC_INSTANCE_2_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_12_INTERRUPT_RBW_ERR_E),                                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SMB_MC_INSTANCE_2_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_13_INTERRUPT_RBW_ERR_E),                                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SMB_MC_INSTANCE_2_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_14_INTERRUPT_RBW_ERR_E),                                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SMB_MC_INSTANCE_2_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_15_INTERRUPT_RBW_ERR_E),                                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SMB_MC_INSTANCE_2_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_0_REFCNT_SER_ERROR_E),                                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SMB_MC_INSTANCE_2_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_1_REFCNT_SER_ERROR_E),                                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SMB_MC_INSTANCE_2_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_2_REFCNT_SER_ERROR_E),                                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SMB_MC_INSTANCE_2_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_3_REFCNT_SER_ERROR_E),                                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SMB_MC_INSTANCE_2_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_4_REFCNT_SER_ERROR_E),                                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SMB_MC_INSTANCE_2_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_5_REFCNT_SER_ERROR_E),                                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SMB_MC_INSTANCE_2_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_6_REFCNT_SER_ERROR_E),                                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SMB_MC_INSTANCE_2_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_7_REFCNT_SER_ERROR_E),                                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SMB_MC_INSTANCE_2_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_8_REFCNT_SER_ERROR_E),                                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SMB_MC_INSTANCE_2_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_9_REFCNT_SER_ERROR_E),                                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SMB_MC_INSTANCE_2_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_10_REFCNT_SER_ERROR_E),                                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SMB_MC_INSTANCE_2_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_11_REFCNT_SER_ERROR_E),                                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SMB_MC_INSTANCE_2_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_12_REFCNT_SER_ERROR_E),                                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SMB_MC_INSTANCE_2_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_13_REFCNT_SER_ERROR_E),                                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SMB_MC_INSTANCE_2_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_14_REFCNT_SER_ERROR_E),                                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SMB_MC_INSTANCE_2_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_15_REFCNT_SER_ERROR_E),                                   \
\
    /*SMB_WRITE_ARBITER*/                                                                                                                       \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SMB_WRITE_ARBITER_INSTANCE_0_INTERRUPT_MISC_CAUSE_INTERRUPT_MC_CA_FIFO_OVERFLOW_E),                \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SMB_WRITE_ARBITER_INSTANCE_1_INTERRUPT_MISC_CAUSE_INTERRUPT_MC_CA_FIFO_OVERFLOW_E),                \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SMB_WRITE_ARBITER_INSTANCE_2_INTERRUPT_MISC_CAUSE_INTERRUPT_MC_CA_FIFO_OVERFLOW_E),                \
\
    /*GPC_PACKET_WRITE[0x0B400200]*/                                                                                                     \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_WRITE_INSTANCE_0_INTERRUPT_CAUSE_INT_CAUSE_FIFO_0_E),                                       \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_WRITE_INSTANCE_0_INTERRUPT_CAUSE_INT_CAUSE_FIFO_1_E),                                       \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_WRITE_INSTANCE_0_INTERRUPT_CAUSE_INT_CAUSE_FIFO_2_E),                                       \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_WRITE_INSTANCE_0_INTERRUPT_CAUSE_INT_CAUSE_FIFO_3_E),                                       \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_WRITE_INSTANCE_0_INTERRUPT_CAUSE_INT_CAUSE_LENGTH_0_E),                                     \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_WRITE_INSTANCE_0_INTERRUPT_CAUSE_INT_CAUSE_LENGTH_0_E),                                     \
\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_WRITE_INSTANCE_1_INTERRUPT_CAUSE_INT_CAUSE_FIFO_0_E),                                       \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_WRITE_INSTANCE_1_INTERRUPT_CAUSE_INT_CAUSE_FIFO_1_E),                                       \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_WRITE_INSTANCE_1_INTERRUPT_CAUSE_INT_CAUSE_FIFO_2_E),                                       \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_WRITE_INSTANCE_1_INTERRUPT_CAUSE_INT_CAUSE_FIFO_3_E),                                       \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_WRITE_INSTANCE_1_INTERRUPT_CAUSE_INT_CAUSE_LENGTH_0_E),                                     \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_WRITE_INSTANCE_1_INTERRUPT_CAUSE_INT_CAUSE_LENGTH_0_E),                                     \
\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_WRITE_INSTANCE_2_INTERRUPT_CAUSE_INT_CAUSE_FIFO_0_E),                                       \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_WRITE_INSTANCE_2_INTERRUPT_CAUSE_INT_CAUSE_FIFO_1_E),                                       \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_WRITE_INSTANCE_2_INTERRUPT_CAUSE_INT_CAUSE_FIFO_2_E),                                       \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_WRITE_INSTANCE_2_INTERRUPT_CAUSE_INT_CAUSE_FIFO_3_E),                                       \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_WRITE_INSTANCE_2_INTERRUPT_CAUSE_INT_CAUSE_LENGTH_0_E),                                     \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_WRITE_INSTANCE_2_INTERRUPT_CAUSE_INT_CAUSE_LENGTH_0_E),                                     \
\
    /*GPC_PACKET_READ[0x0AD01300]*/                                                                                                                 \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_0_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),                 \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_0_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),                       \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_0_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),                \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_0_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),                          \
\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_0_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E), \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_0_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),       \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_0_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_0_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),          \
\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_0_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E), \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_0_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),       \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_0_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_0_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),          \
\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_1_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E), \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_1_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),       \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_1_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_1_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),          \
\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_1_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E), \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_1_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),       \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_1_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_1_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),          \
\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_1_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),  \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_1_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),        \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_1_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E), \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_1_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),           \
\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_2_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),  \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_2_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),        \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_2_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E), \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_2_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),           \
\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_2_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),  \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_2_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),        \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_2_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E), \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_2_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),           \
\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_2_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),  \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_2_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),        \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_2_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E), \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_2_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),           \
\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_3_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),  \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_3_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),        \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_3_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E), \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_3_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),           \
\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_3_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),  \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_3_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),        \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_3_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E), \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_3_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),           \
\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_3_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),  \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_3_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),        \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_3_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E), \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_3_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),           \
\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_4_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),  \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_4_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),        \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_4_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E), \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_4_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),           \
\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_4_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),  \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_4_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),        \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_4_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E), \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_4_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),           \
\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_4_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),  \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_4_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),        \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_4_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E), \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_4_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),           \
\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_5_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),  \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_5_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),        \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_5_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E), \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_5_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),           \
\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_5_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),  \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_5_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),        \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_5_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E), \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_5_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),           \
\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_5_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),  \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_5_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),        \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_5_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E), \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_5_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),           \
\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_6_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),  \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_6_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),        \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_6_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E), \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_6_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),           \
\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_6_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),  \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_6_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),        \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_6_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E), \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_6_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),           \
\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_6_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),  \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_6_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),        \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_6_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E), \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_6_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),           \
\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_7_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),  \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_7_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),        \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_7_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E), \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_7_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),           \
\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_7_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),  \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_7_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),         \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_7_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E), \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_7_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),           \
\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_7_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),  \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_7_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),        \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_7_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E), \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_7_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),           \
\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_8_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),  \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_8_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),        \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_8_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E), \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_8_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),           \
\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_8_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),  \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_8_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),        \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_8_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E), \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_8_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),           \
\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_8_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),  \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_8_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),        \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_8_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E), \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_8_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),           \
\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_9_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),  \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_9_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),        \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_9_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E), \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_9_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),           \
\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_9_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),  \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_9_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),        \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_9_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E), \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_9_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),           \
\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_9_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E),  \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_9_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),        \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_9_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E), \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_9_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),           \
\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_10_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E), \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_10_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),       \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_10_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_10_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),          \
\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_10_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E), \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_10_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),       \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_10_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_10_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),          \
\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_10_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E), \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_10_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),       \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_10_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_10_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),          \
\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_11_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E), \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_11_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),       \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_11_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_11_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),          \
\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_11_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E), \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_11_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),       \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_11_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_11_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),          \
\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_11_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E), \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_11_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),       \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_11_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_11_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),          \
\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_12_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E), \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_12_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),       \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_12_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_12_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),          \
\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_12_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E), \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_12_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),       \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_12_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_12_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),          \
\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_12_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E), \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_12_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),       \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_12_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_12_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),          \
\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_13_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E), \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_13_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),       \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_13_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_13_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),          \
\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_13_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E), \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_13_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),       \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_13_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_13_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),          \
\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_13_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E), \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_13_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),       \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_13_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_13_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),          \
\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_14_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E), \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_14_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),       \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_14_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_14_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),          \
\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_14_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E), \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_14_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),       \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_14_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_14_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),          \
\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_14_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E), \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_14_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),       \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_14_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_14_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),          \
\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_15_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E), \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_15_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),       \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_15_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_15_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),          \
\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_15_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E), \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_15_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),       \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_15_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_15_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),          \
\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_15_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E), \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_15_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),       \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_15_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_15_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),          \
\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_16_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E), \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_16_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),       \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_16_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_16_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),          \
\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_16_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E), \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_16_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),       \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_16_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_16_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),          \
\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_16_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E), \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_16_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),       \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_16_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_16_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),          \
\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_17_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E), \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_17_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),       \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_17_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_17_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),          \
\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_17_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E), \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_17_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),       \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_17_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_17_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),          \
\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_17_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E), \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_17_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),       \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_17_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_17_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),          \
\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_18_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E), \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_18_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),       \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_18_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_18_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),          \
\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_18_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E), \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_18_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),       \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_18_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_18_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),          \
\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_18_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E), \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_18_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),       \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_18_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_18_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),          \
\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_19_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E), \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_19_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),       \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_19_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_19_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),          \
\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_19_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E), \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_19_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),       \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_19_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_19_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),          \
\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_19_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E), \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_19_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),       \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_19_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_19_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),          \
\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_20_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E), \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_20_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),       \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_20_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_20_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),          \
\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_20_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E), \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_20_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),       \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_20_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_20_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),          \
\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_20_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E), \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_20_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),       \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_20_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_20_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),          \
\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_21_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E), \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_21_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),       \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_21_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_21_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),          \
\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_21_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E), \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_21_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),       \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_21_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_21_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),          \
\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_21_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E), \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_21_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),       \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_21_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_21_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),          \
\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_22_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E), \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_22_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),       \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_22_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_22_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),          \
\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_22_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E), \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_22_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),       \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_22_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_22_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),          \
\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_22_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E), \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_22_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),       \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_22_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_22_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),          \
\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_23_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E), \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_23_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),       \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_23_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_23_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),          \
\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_23_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E), \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_23_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),       \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_23_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_23_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),          \
\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_23_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E), \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_23_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),       \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_23_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_23_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),          \
\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_24_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E), \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_24_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),       \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_24_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_24_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),          \
\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_24_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E), \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_24_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),       \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_24_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_24_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),          \
\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_24_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E), \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_24_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),       \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_24_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_24_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),          \
\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_25_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E), \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_25_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),       \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_25_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_25_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),          \
\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_25_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E), \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_25_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),       \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_25_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_25_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),          \
\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_25_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E), \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_25_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),       \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_25_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_25_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),          \
\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_26_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E), \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_26_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),       \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_26_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_26_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),          \
\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_26_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E), \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_26_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),       \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_26_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_26_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),          \
\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_26_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E), \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_26_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),       \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_26_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_26_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),          \
\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_27_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E), \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_27_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),       \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_27_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_CHANNEL_INDEX_27_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),          \
\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_27_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E), \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_27_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),       \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_27_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_CHANNEL_INDEX_27_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),          \
\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_27_PACKET_DESCRIPTOR_FIFO_OVERFLOW_CHANNEL_E), \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_27_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_E),       \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_27_TX_READ_REQUEST_COUNTER_OVERFLOW_CHANNEL_E),\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_CHANNEL_INDEX_27_CELL_TAG_FIFO_OVERFLOW_CHANNEL_E),          \
\
    /*GPC_PACKET_READ_MISC[0x01C01508]*/                                                                                                     \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_INTERRUPT_MISC_CAUSE_CELL_READ_CELL_CREDIT_COUNTER_OVERFLOW_E),\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_INTERRUPT_MISC_CAUSE_MULTICAST_COUNTER_OVERFLOW_E),            \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_INTERRUPT_MISC_CAUSE_MULTICAST_COUNTER_UNDERFLOW_E),           \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_INTERRUPT_MISC_CAUSE_CAM_MULTIPLE_MATCHES_E),                  \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_INTERRUPT_MISC_CAUSE_DESCRIPTOR_HEADER_SIZE_ERROR_E),          \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_INTERRUPT_MISC_CAUSE_DESCRIPTOR_TAIL_SIZE_ERROR_E),            \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_INTERRUPT_MISC_CAUSE_PACKET_COUNT_PACKET_SIZE_ERROR_E),        \
\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_INTERRUPT_MISC_CAUSE_CELL_READ_CELL_CREDIT_COUNTER_OVERFLOW_E),\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_INTERRUPT_MISC_CAUSE_MULTICAST_COUNTER_OVERFLOW_E),            \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_INTERRUPT_MISC_CAUSE_MULTICAST_COUNTER_UNDERFLOW_E),           \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_INTERRUPT_MISC_CAUSE_CAM_MULTIPLE_MATCHES_E),                  \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_INTERRUPT_MISC_CAUSE_DESCRIPTOR_HEADER_SIZE_ERROR_E),          \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_INTERRUPT_MISC_CAUSE_DESCRIPTOR_TAIL_SIZE_ERROR_E),            \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_INTERRUPT_MISC_CAUSE_PACKET_COUNT_PACKET_SIZE_ERROR_E),        \
\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_INTERRUPT_MISC_CAUSE_CELL_READ_CELL_CREDIT_COUNTER_OVERFLOW_E),\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_INTERRUPT_MISC_CAUSE_MULTICAST_COUNTER_OVERFLOW_E),            \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_INTERRUPT_MISC_CAUSE_MULTICAST_COUNTER_UNDERFLOW_E),           \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_INTERRUPT_MISC_CAUSE_CAM_MULTIPLE_MATCHES_E),                  \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_INTERRUPT_MISC_CAUSE_DESCRIPTOR_HEADER_SIZE_ERROR_E),          \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_INTERRUPT_MISC_CAUSE_DESCRIPTOR_TAIL_SIZE_ERROR_E),            \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_INTERRUPT_MISC_CAUSE_PACKET_COUNT_PACKET_SIZE_ERROR_E),        \
\
    /*GPC_PACKET_READ_PACKET_COUNT[0x0AD01518]*/                                                                                          \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_INTERRUPT_PACKET_COUNT_CAUSE_PACKET_DESCRIPTOR_FIFO_OVERFLOW_PACKET_COUNT_E),  \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_INTERRUPT_PACKET_COUNT_CAUSE_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_PACKET_COUNT_E),\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_INTERRUPT_PACKET_COUNT_CAUSE_PACKET_DESCRIPTOR_FIFO_OVERFLOW_PACKET_COUNT_E),  \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_INTERRUPT_PACKET_COUNT_CAUSE_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_PACKET_COUNT_E),\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_INTERRUPT_PACKET_COUNT_CAUSE_PACKET_DESCRIPTOR_FIFO_OVERFLOW_PACKET_COUNT_E),  \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_INTERRUPT_PACKET_COUNT_CAUSE_NPM_ENDOF_PACKET_MISMATCH_CHANNEL_PACKET_COUNT_E),\
\
    /*GPC_PACKET_READ_PACKET_CREDIT_COUNTER[0x0AD01510]*/                                                                                 \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_REORDER_CREDIT_COUNTER_CELL_OVERFLOW_E),         \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_REORDER_CREDIT_COUNTER_LVL2_OVERFLOW_E),         \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_REORDER_CREDIT_COUNTER_LVL1_OVERFLOW_E),         \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_REORDER_CREDIT_COUNTER_LVL0_OVERFLOW_E),         \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_REORDER_CREDIT_COUNTER_HT_OVERFLOW_E),           \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_REORDER_COMMAND_CREDIT_COUNTER_CELL_OVERFLOW_E), \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_REORDER_COMMAND_CREDIT_COUNTER_LVL2_OVERFLOW_E), \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_REORDER_COMMAND_CREDIT_COUNTER_LVL1_OVERFLOW_E), \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_REORDER_COMMAND_CREDIT_COUNTER_LVL0_OVERFLOW_E), \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_REORDER_COMMAND_CREDIT_COUNTER_HT_OVERFLOW_E),   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_COMMAND_QUEUE_CREDIT_COUNTER_CELL_OVERFLOW_E),   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_COMMAND_QUEUE_CREDIT_COUNTER_LVL2_OVERFLOW_E),   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_COMMAND_QUEUE_CREDIT_COUNTER_LVL1_OVERFLOW_E),   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_COMMAND_QUEUE_CREDIT_COUNTER_LVL0_OVERFLOW_E),   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_COMMAND_QUEUE_CREDIT_COUNTER_HT_OVERFLOW_E),     \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_MSW_CREDIT_COUNTER_OVERFLOW_E),                  \
\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_REORDER_CREDIT_COUNTER_CELL_OVERFLOW_E),         \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_REORDER_CREDIT_COUNTER_LVL2_OVERFLOW_E),         \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_REORDER_CREDIT_COUNTER_LVL1_OVERFLOW_E),         \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_REORDER_CREDIT_COUNTER_LVL0_OVERFLOW_E),         \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_REORDER_CREDIT_COUNTER_HT_OVERFLOW_E),           \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_REORDER_COMMAND_CREDIT_COUNTER_CELL_OVERFLOW_E), \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_REORDER_COMMAND_CREDIT_COUNTER_LVL2_OVERFLOW_E), \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_REORDER_COMMAND_CREDIT_COUNTER_LVL1_OVERFLOW_E), \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_REORDER_COMMAND_CREDIT_COUNTER_LVL0_OVERFLOW_E), \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_REORDER_COMMAND_CREDIT_COUNTER_HT_OVERFLOW_E),   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_COMMAND_QUEUE_CREDIT_COUNTER_CELL_OVERFLOW_E),   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_COMMAND_QUEUE_CREDIT_COUNTER_LVL2_OVERFLOW_E),   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_COMMAND_QUEUE_CREDIT_COUNTER_LVL1_OVERFLOW_E),   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_COMMAND_QUEUE_CREDIT_COUNTER_LVL0_OVERFLOW_E),   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_COMMAND_QUEUE_CREDIT_COUNTER_HT_OVERFLOW_E),     \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_MSW_CREDIT_COUNTER_OVERFLOW_E),                  \
\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_REORDER_CREDIT_COUNTER_CELL_OVERFLOW_E),         \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_REORDER_CREDIT_COUNTER_LVL2_OVERFLOW_E),         \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_REORDER_CREDIT_COUNTER_LVL1_OVERFLOW_E),         \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_REORDER_CREDIT_COUNTER_LVL0_OVERFLOW_E),         \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_REORDER_CREDIT_COUNTER_HT_OVERFLOW_E),           \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_REORDER_COMMAND_CREDIT_COUNTER_CELL_OVERFLOW_E), \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_REORDER_COMMAND_CREDIT_COUNTER_LVL2_OVERFLOW_E), \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_REORDER_COMMAND_CREDIT_COUNTER_LVL1_OVERFLOW_E), \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_REORDER_COMMAND_CREDIT_COUNTER_LVL0_OVERFLOW_E), \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_REORDER_COMMAND_CREDIT_COUNTER_HT_OVERFLOW_E),   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_COMMAND_QUEUE_CREDIT_COUNTER_CELL_OVERFLOW_E),   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_COMMAND_QUEUE_CREDIT_COUNTER_LVL2_OVERFLOW_E),   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_COMMAND_QUEUE_CREDIT_COUNTER_LVL1_OVERFLOW_E),   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_COMMAND_QUEUE_CREDIT_COUNTER_LVL0_OVERFLOW_E),   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_COMMAND_QUEUE_CREDIT_COUNTER_HT_OVERFLOW_E),     \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_INTERRUPT_PACKET_READ_CREDIT_COUNTER_CAUSE_PACKET_READ_MSW_CREDIT_COUNTER_OVERFLOW_E),                  \
\
    /*GPC_PACKET_READ_PACKET_COUNT_CREDIT_COUNTER[0x01C01518]*/                                                         \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_REORDER_CREDIT_COUNTER_CELL_OVERFLOW_E),         \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_REORDER_CREDIT_COUNTER_LVL2_OVERFLOW_E),         \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_REORDER_CREDIT_COUNTER_LVL1_OVERFLOW_E),         \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_REORDER_CREDIT_COUNTER_LVL0_OVERFLOW_E),         \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_REORDER_COMMAND_CREDIT_COUNTER_CELL_OVERFLOW_E), \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_REORDER_COMMAND_CREDIT_COUNTER_LVL2_OVERFLOW_E), \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_REORDER_COMMAND_CREDIT_COUNTER_LVL1_OVERFLOW_E), \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_REORDER_COMMAND_CREDIT_COUNTER_LVL0_OVERFLOW_E), \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_COMMAND_QUEUE_CREDIT_COUNTER_CELL_OVERFLOW_E),   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_COMMAND_QUEUE_CREDIT_COUNTER_LVL2_OVERFLOW_E),   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_COMMAND_QUEUE_CREDIT_COUNTER_LVL1_OVERFLOW_E),   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_COMMAND_QUEUE_CREDIT_COUNTER_LVL0_OVERFLOW_E),   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_0_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_NPM_REQUEST_CREDIT_COUNTER_OVERFLOW_E),          \
\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_REORDER_CREDIT_COUNTER_CELL_OVERFLOW_E),         \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_REORDER_CREDIT_COUNTER_LVL2_OVERFLOW_E),         \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_REORDER_CREDIT_COUNTER_LVL1_OVERFLOW_E),         \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_REORDER_CREDIT_COUNTER_LVL0_OVERFLOW_E),         \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_REORDER_COMMAND_CREDIT_COUNTER_CELL_OVERFLOW_E), \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_REORDER_COMMAND_CREDIT_COUNTER_LVL2_OVERFLOW_E), \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_REORDER_COMMAND_CREDIT_COUNTER_LVL1_OVERFLOW_E), \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_REORDER_COMMAND_CREDIT_COUNTER_LVL0_OVERFLOW_E), \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_COMMAND_QUEUE_CREDIT_COUNTER_CELL_OVERFLOW_E),   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_COMMAND_QUEUE_CREDIT_COUNTER_LVL2_OVERFLOW_E),   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_COMMAND_QUEUE_CREDIT_COUNTER_LVL1_OVERFLOW_E),   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_COMMAND_QUEUE_CREDIT_COUNTER_LVL0_OVERFLOW_E),   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_1_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_NPM_REQUEST_CREDIT_COUNTER_OVERFLOW_E),          \
\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_REORDER_CREDIT_COUNTER_CELL_OVERFLOW_E),         \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_REORDER_CREDIT_COUNTER_LVL2_OVERFLOW_E),         \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_REORDER_CREDIT_COUNTER_LVL1_OVERFLOW_E),         \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_REORDER_CREDIT_COUNTER_LVL0_OVERFLOW_E),         \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_REORDER_COMMAND_CREDIT_COUNTER_CELL_OVERFLOW_E), \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_REORDER_COMMAND_CREDIT_COUNTER_LVL2_OVERFLOW_E), \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_REORDER_COMMAND_CREDIT_COUNTER_LVL1_OVERFLOW_E), \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_REORDER_COMMAND_CREDIT_COUNTER_LVL0_OVERFLOW_E), \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_COMMAND_QUEUE_CREDIT_COUNTER_CELL_OVERFLOW_E),   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_COMMAND_QUEUE_CREDIT_COUNTER_LVL2_OVERFLOW_E),   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_COMMAND_QUEUE_CREDIT_COUNTER_LVL1_OVERFLOW_E),   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_COMMAND_QUEUE_CREDIT_COUNTER_LVL0_OVERFLOW_E),   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,GPC_PACKET_READ_INSTANCE_2_INTERRUPT_PACKET_COUNT_CREDIT_COUNTER_CAUSE_PACKET_COUNT_NPM_REQUEST_CREDIT_COUNTER_OVERFLOW_E),          \
\
    /*DFX_SERVER_INTERRUPT_CAUSE[0x0B8F8108]*/                                                                                                                    \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,DFX_SERVER_INTERRUPT_BIST_FAIL_E),                                                    \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,DFX_SERVER_INTERRUPT_EXTERNAL_TEMPERATURE_THRESHOLD_E),                               \
\
    /*RXDMA_INTERRUPT2_CAUSE[0x08001C90]*/                                                                                         \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,RX_INSTANCE_0_RX_DMA_INTERRUPT_2_CAUSE_PB_TAIL_ID_MEM_SER_INT_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,RX_INSTANCE_0_RX_DMA_INTERRUPT_2_CAUSE_FINAL_TAIL_BC_0_MEM_SER_INT_E),                 \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,RX_INSTANCE_0_RX_DMA_INTERRUPT_2_CAUSE_FINAL_TAIL_BC_1_MEM_SER_INT_E),                 \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,RX_INSTANCE_0_RX_DMA_INTERRUPT_2_CAUSE_REPLY_TAIL_PB_ID_MEM_SER_INT_E),                \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,RX_INSTANCE_0_RX_DMA_INTERRUPT_2_CAUSE_PACM_MEM_0_PB_RD_DATA_ERR_INT_E),               \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,RX_INSTANCE_0_RX_DMA_INTERRUPT_2_CAUSE_PACM_MEM_1_PB_RD_DATA_ERR_INT_E),               \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,RX_INSTANCE_0_RX_DMA_INTERRUPT_2_CAUSE_PCM_MEM_0_PB_RD_DATA_ERR_INT_E),                \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,RX_INSTANCE_0_RX_DMA_INTERRUPT_2_CAUSE_PCM_MEM_1_PB_RD_DATA_ERR_INT_E),                \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,RX_INSTANCE_0_RX_DMA_INTERRUPT_2_CAUSE_PACKET_HEAD_FIFO_0_RD_SER_INT_E),               \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,RX_INSTANCE_0_RX_DMA_INTERRUPT_2_CAUSE_PACKET_HEAD_FIFO_1_RD_SER_INT_E),               \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,RX_INSTANCE_0_RX_DMA_INTERRUPT_2_CAUSE_ENQUEUE_REQUESTS_DESCRIPTOR_FIFO_RD_SER_INT_E), \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,RX_INSTANCE_0_RX_DMA_INTERRUPT_2_CAUSE_HEAD_PACKET_COUNT_DATA_FIFO_MEM_RD_SER_INT_E),  \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,RX_INSTANCE_0_RX_DMA_INTERRUPT_2_CAUSE_RX_LOCAL_IDS_FIFO_MEM_RD_SER_INT_E),            \
\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,RX_INSTANCE_1_RX_DMA_INTERRUPT_2_CAUSE_PB_TAIL_ID_MEM_SER_INT_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,RX_INSTANCE_1_RX_DMA_INTERRUPT_2_CAUSE_FINAL_TAIL_BC_0_MEM_SER_INT_E),                 \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,RX_INSTANCE_1_RX_DMA_INTERRUPT_2_CAUSE_FINAL_TAIL_BC_1_MEM_SER_INT_E),                 \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,RX_INSTANCE_1_RX_DMA_INTERRUPT_2_CAUSE_REPLY_TAIL_PB_ID_MEM_SER_INT_E),                \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,RX_INSTANCE_1_RX_DMA_INTERRUPT_2_CAUSE_PACM_MEM_0_PB_RD_DATA_ERR_INT_E),               \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,RX_INSTANCE_1_RX_DMA_INTERRUPT_2_CAUSE_PACM_MEM_1_PB_RD_DATA_ERR_INT_E),               \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,RX_INSTANCE_1_RX_DMA_INTERRUPT_2_CAUSE_PCM_MEM_0_PB_RD_DATA_ERR_INT_E),                \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,RX_INSTANCE_1_RX_DMA_INTERRUPT_2_CAUSE_PCM_MEM_1_PB_RD_DATA_ERR_INT_E),                \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,RX_INSTANCE_1_RX_DMA_INTERRUPT_2_CAUSE_PACKET_HEAD_FIFO_0_RD_SER_INT_E),               \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,RX_INSTANCE_1_RX_DMA_INTERRUPT_2_CAUSE_PACKET_HEAD_FIFO_1_RD_SER_INT_E),               \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,RX_INSTANCE_1_RX_DMA_INTERRUPT_2_CAUSE_ENQUEUE_REQUESTS_DESCRIPTOR_FIFO_RD_SER_INT_E), \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,RX_INSTANCE_1_RX_DMA_INTERRUPT_2_CAUSE_HEAD_PACKET_COUNT_DATA_FIFO_MEM_RD_SER_INT_E),  \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,RX_INSTANCE_1_RX_DMA_INTERRUPT_2_CAUSE_RX_LOCAL_IDS_FIFO_MEM_RD_SER_INT_E),            \
\
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,RX_INSTANCE_2_RX_DMA_INTERRUPT_2_CAUSE_PB_TAIL_ID_MEM_SER_INT_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,RX_INSTANCE_2_RX_DMA_INTERRUPT_2_CAUSE_FINAL_TAIL_BC_0_MEM_SER_INT_E),                 \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,RX_INSTANCE_2_RX_DMA_INTERRUPT_2_CAUSE_FINAL_TAIL_BC_1_MEM_SER_INT_E),                 \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,RX_INSTANCE_2_RX_DMA_INTERRUPT_2_CAUSE_REPLY_TAIL_PB_ID_MEM_SER_INT_E),                \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,RX_INSTANCE_2_RX_DMA_INTERRUPT_2_CAUSE_PACM_MEM_0_PB_RD_DATA_ERR_INT_E),               \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,RX_INSTANCE_2_RX_DMA_INTERRUPT_2_CAUSE_PACM_MEM_1_PB_RD_DATA_ERR_INT_E),               \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,RX_INSTANCE_2_RX_DMA_INTERRUPT_2_CAUSE_PCM_MEM_0_PB_RD_DATA_ERR_INT_E),                \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,RX_INSTANCE_2_RX_DMA_INTERRUPT_2_CAUSE_PCM_MEM_1_PB_RD_DATA_ERR_INT_E),                \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,RX_INSTANCE_2_RX_DMA_INTERRUPT_2_CAUSE_PACKET_HEAD_FIFO_0_RD_SER_INT_E),               \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,RX_INSTANCE_2_RX_DMA_INTERRUPT_2_CAUSE_PACKET_HEAD_FIFO_1_RD_SER_INT_E),               \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,RX_INSTANCE_2_RX_DMA_INTERRUPT_2_CAUSE_ENQUEUE_REQUESTS_DESCRIPTOR_FIFO_RD_SER_INT_E), \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,RX_INSTANCE_2_RX_DMA_INTERRUPT_2_CAUSE_HEAD_PACKET_COUNT_DATA_FIFO_MEM_RD_SER_INT_E),  \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,RX_INSTANCE_2_RX_DMA_INTERRUPT_2_CAUSE_RX_LOCAL_IDS_FIFO_MEM_RD_SER_INT_E),            \
\
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_DATA_INTEGRITY_ERROR_E,                                                                                        \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,HA_HA_INTERRUPT_CAUSE_ECC_SINGLE_ERROR_E),                                 \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,HA_HA_INTERRUPT_CAUSE_ECC_DOUBLE_ERROR_E),                                 \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,TCAM_TCAM_INTERRUPT_CAUSE_TCAM_ARRAY_PARITY_ERROR_E),                      \
    \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SDW_INSTANCE_0_SDW_INTERRUPT_CAUSE_SRAM_SINGLE_ERR_E),                     \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SDW_INSTANCE_0_SDW_INTERRUPT_CAUSE_SRAM_DOUBLE_ERR_E),                     \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SDW_INSTANCE_1_SDW_INTERRUPT_CAUSE_SRAM_SINGLE_ERR_E),                     \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SDW_INSTANCE_1_SDW_INTERRUPT_CAUSE_SRAM_DOUBLE_ERR_E),                     \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SDW_INSTANCE_2_SDW_INTERRUPT_CAUSE_SRAM_SINGLE_ERR_E),                     \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SDW_INSTANCE_2_SDW_INTERRUPT_CAUSE_SRAM_DOUBLE_ERR_E),                     \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SDW_INSTANCE_3_SDW_INTERRUPT_CAUSE_SRAM_SINGLE_ERR_E),                     \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SDW_INSTANCE_3_SDW_INTERRUPT_CAUSE_SRAM_DOUBLE_ERR_E),                     \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SDW_INSTANCE_4_SDW_INTERRUPT_CAUSE_SRAM_SINGLE_ERR_E),                     \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,SDW_INSTANCE_4_SDW_INTERRUPT_CAUSE_SRAM_DOUBLE_ERR_E),                     \
    \
    /* DFX Interrupts. Aas has 6 DFX Pipes. Need to connect to all. */                                              \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,DFX_SERVER_INTERRUPT_SUMMARY_PIPE_0_INTERRUPT_SUM_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,DFX_SERVER_INTERRUPT_SUMMARY_PIPE_1_INTERRUPT_SUM_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,DFX_SERVER_INTERRUPT_SUMMARY_PIPE_2_INTERRUPT_SUM_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,DFX_SERVER_INTERRUPT_SUMMARY_PIPE_3_INTERRUPT_SUM_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,DFX_SERVER_INTERRUPT_SUMMARY_PIPE_4_INTERRUPT_SUM_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,DFX_SERVER_INTERRUPT_SUMMARY_PIPE_5_INTERRUPT_SUM_E),                      \
                                                                                                                        \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_OAM_EXCEPTION_KEEPALIVE_AGING_E,                                                                               \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,IOAM_OAM_UNIT_INTERRUPT_CAUSE_KEEP_ALIVE_AGING_E),                         \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,EOAM_OAM_UNIT_INTERRUPT_CAUSE_KEEP_ALIVE_AGING_E),                         \
                                                                                                                        \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_OAM_EXCEPTION_EXCESS_KEEPALIVE_E,                                                                              \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,IOAM_OAM_UNIT_INTERRUPT_CAUSE_EXCESS_KEEPALIVE_E),                            \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,EOAM_OAM_UNIT_INTERRUPT_CAUSE_EXCESS_KEEPALIVE_E),                            \
                                                                                                                        \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_OAM_EXCEPTION_INVALID_KEEPALIVE_E,                                                                             \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,IOAM_OAM_UNIT_INTERRUPT_CAUSE_INVALID_KEEPALIVE_HASH_E),                      \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,EOAM_OAM_UNIT_INTERRUPT_CAUSE_INVALID_KEEPALIVE_HASH_E),                      \
                                                                                                                        \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_OAM_EXCEPTION_RDI_STATUS_E,                                                                                    \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,IOAM_OAM_UNIT_INTERRUPT_CAUSE_RDI_STATUS_E),                                  \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,EOAM_OAM_UNIT_INTERRUPT_CAUSE_RDI_STATUS_E),                                  \
                                                                                                                        \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_OAM_EXCEPTION_TX_PERIOD_E,                                                                                     \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,IOAM_OAM_UNIT_INTERRUPT_CAUSE_TX_PERIOD_E),                                   \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,EOAM_OAM_UNIT_INTERRUPT_CAUSE_TX_PERIOD_E),                                   \
                                                                                                                        \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_OAM_EXCEPTION_MEG_LEVEL_E,                                                                                     \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,IOAM_OAM_UNIT_INTERRUPT_CAUSE_MEG_LEVEL_EXCEPTION_E),                         \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,EOAM_OAM_UNIT_INTERRUPT_CAUSE_MEG_LEVEL_EXCEPTION_E),                         \
                                                                                                                        \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_OAM_EXCEPTION_SOURCE_INTERFACE_E,                                                                              \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,IOAM_OAM_UNIT_INTERRUPT_CAUSE_SOURCE_INTERFACE_EXCEPTION_E),               \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,EOAM_OAM_UNIT_INTERRUPT_CAUSE_SOURCE_INTERFACE_EXCEPTION_E),               \
                                                                                                                        \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_OAM_ILLEGAL_ENTRY_INDEX_E,                                                                                     \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,IOAM_OAM_UNIT_INTERRUPT_CAUSE_ILLEGAL_OAM_ENTRY_INDEX_E),                  \
    AAS_SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_tile,EOAM_OAM_UNIT_INTERRUPT_CAUSE_ILLEGAL_OAM_ENTRY_INDEX_E),                  \
                                                                                                                        \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
                                                                                                                        \
 CPSS_PP_PHA_E,                                                                                                         \
    SET_PHA_EVENTS_MAC(_tile),                                                                                                 \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_TQ_PORT_HR_CROSSED_THRESHOLD_E,                                                                                \
    AAS_SET_HR_TRSH_EVENT_ALL_PORTS_MAC(_tile),                                                                            \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
CPSS_PP_MACSEC_SA_EXPIRED_E,\
    AAS_SET_EIP_164_EVENT_PER_ALL_DIRECTIONS_ALL_DP_MAC(_tile,GLOBAL_MAC_SEC_INTERRUPT_CAUSE_SA_EXPIRED_E),               \
MARK_END_OF_UNI_EV_CNS,                                                                                                 \
                                                                                                                        \
CPSS_PP_MACSEC_SA_PN_FULL_E,                                                                                            \
    AAS_SET_EIP_164_EVENT_PER_EGRESS_DIRECTION_ALL_DP_MAC(_tile,GLOBAL_MAC_SEC_INTERRUPT_CAUSE_SA_PN_THR_E),              \
MARK_END_OF_UNI_EV_CNS,                                                                                                 \
                                                                                                                        \
CPSS_PP_MACSEC_STATISTICS_SUMMARY_E,                                                                                    \
    AAS_SET_EIP_164_EVENT_SOURCE_PER_ALL_DIRECTIONS_ALL_DP_MAC(_tile,GLOBAL_MAC_SEC_INTERRUPT_CAUSE_STAT_SA_THR_E,       0),  \
    AAS_SET_EIP_164_EVENT_SOURCE_PER_ALL_DIRECTIONS_ALL_DP_MAC(_tile,GLOBAL_MAC_SEC_INTERRUPT_CAUSE_STAT_SECY_THR_E,     1),  \
    AAS_SET_EIP_164_EVENT_SOURCE_PER_ALL_DIRECTIONS_ALL_DP_MAC(_tile,GLOBAL_MAC_SEC_INTERRUPT_CAUSE_STAT_IFC0_THR_E,     2),  \
    AAS_SET_EIP_164_EVENT_SOURCE_PER_INGRESS_DIRECTION_ALL_DP_MAC(_tile,GLOBAL_MAC_SEC_INTERRUPT_CAUSE_STAT_IFC1_THR_E,  3),  \
    AAS_SET_EIP_164_EVENT_SOURCE_PER_INGRESS_DIRECTION_ALL_DP_MAC(_tile,GLOBAL_MAC_SEC_INTERRUPT_CAUSE_STAT_RXCAM_THR_E, 4),  \
                                                                                                                         \
    AAS_SET_EIP_163_EVENT_SOURCE_PER_ALL_DIRECTIONS_ALL_DP_MAC(_tile,GLOBAL_MAC_SEC_INTERRUPT_CAUSE_STAT_TCAM_THR_E,     5),  \
    AAS_SET_EIP_163_EVENT_SOURCE_PER_ALL_DIRECTIONS_ALL_DP_MAC(_tile,GLOBAL_MAC_SEC_INTERRUPT_CAUSE_STAT_CHANNEL_THR_E,  6),  \
MARK_END_OF_UNI_EV_CNS,                                                                                                  \
                                                                                                                         \
CPSS_PP_MACSEC_TRANSFORM_ENGINE_ERR_E,                                                                                   \
    AAS_SET_EIP_66_ERROR_EVENT_PER_ALL_DIRECTIONS_ALL_DP_MAC(_tile,0),       \
    AAS_SET_EIP_66_ERROR_EVENT_PER_ALL_DIRECTIONS_ALL_DP_MAC(_tile,1),       \
    AAS_SET_EIP_66_ERROR_EVENT_PER_ALL_DIRECTIONS_ALL_DP_MAC(_tile,2),       \
    AAS_SET_EIP_66_ERROR_EVENT_PER_ALL_DIRECTIONS_ALL_DP_MAC(_tile,3),       \
    AAS_SET_EIP_66_ERROR_EVENT_PER_ALL_DIRECTIONS_ALL_DP_MAC(_tile,4),       \
    AAS_SET_EIP_66_ERROR_EVENT_PER_ALL_DIRECTIONS_ALL_DP_MAC(_tile,5),       \
    AAS_SET_EIP_66_ERROR_EVENT_PER_ALL_DIRECTIONS_ALL_DP_MAC(_tile,6),       \
    AAS_SET_EIP_66_ERROR_EVENT_PER_ALL_DIRECTIONS_ALL_DP_MAC(_tile,7),       \
    AAS_SET_EIP_66_ERROR_EVENT_PER_ALL_DIRECTIONS_ALL_DP_MAC(_tile,8),       \
    AAS_SET_EIP_66_ERROR_EVENT_PER_ALL_DIRECTIONS_ALL_DP_MAC(_tile,9),       \
    AAS_SET_EIP_66_ERROR_EVENT_PER_ALL_DIRECTIONS_ALL_DP_MAC(_tile,10),      \
    AAS_SET_EIP_66_ERROR_EVENT_PER_ALL_DIRECTIONS_ALL_DP_MAC(_tile,11),      \
    AAS_SET_EIP_66_ERROR_EVENT_PER_ALL_DIRECTIONS_ALL_DP_MAC(_tile,12),      \
    AAS_SET_EIP_66_ERROR_EVENT_PER_ALL_DIRECTIONS_ALL_DP_MAC(_tile,13),      \
    AAS_SET_EIP_66_ERROR_EVENT_PER_ALL_DIRECTIONS_ALL_DP_MAC(_tile,14),      \
MARK_END_OF_UNI_EV_CNS,                                                                                                            \
                                                                                                                                   \
CPSS_PP_MACSEC_EGRESS_SEQ_NUM_ROLLOVER_E,                                                                                          \
    AAS_SET_EIP_66_EVENT_PER_EGRESS_DIRECTION_ALL_DP_MAC(_tile,GLOBAL_MAC_SEC_INTERRUPT_CAUSE_SEQ_NUM_ROLLOVER_E),                      \
MARK_END_OF_UNI_EV_CNS,                                                     \
                                                                            \
CPSS_PP_LMU_LATENCY_EXCEED_THRESHOLD_E,                                     \
    AAS_PER_INSTANCE_LMU_LATENCY_OVER_THRESHOLD_MAC(_tile,0),                 \
    AAS_PER_INSTANCE_LMU_LATENCY_OVER_THRESHOLD_MAC(_tile,1),                 \
    AAS_PER_INSTANCE_LMU_LATENCY_OVER_THRESHOLD_MAC(_tile,2),                 \
MARK_END_OF_UNI_EV_CNS

#define PORTS_LINK_STATUS_CHANGE_EVENTS(_tile) \
    /* MTI total ports 0..255 */                                                                \
    AAS_SET_INDEX_EQUAL_EVENT_PER_MTI_PORTS(_tile,0,LINK_STATUS_CHANGE_E),                        \
    AAS_SET_INDEX_EQUAL_EVENT_PER_MTI_PORTS(_tile,1,LINK_STATUS_CHANGE_E),                        \
    AAS_HALF_DP_SET_INDEX_EQUAL_EVENT_PER_MTI_PORTS(_tile,2,LINK_STATUS_CHANGE_E),                \
    AAS_SET_INDEX_EQUAL_EVENT_PER_MTI_PORTS(_tile,0,LPCS_LINK_STATUS_CHANGE_E),                   \
    AAS_SET_INDEX_EQUAL_EVENT_PER_MTI_PORTS(_tile,1,LPCS_LINK_STATUS_CHANGE_E),                   \
    AAS_HALF_DP_SET_INDEX_EQUAL_EVENT_PER_MTI_PORTS(_tile,2,LPCS_LINK_STATUS_CHANGE_E),           \
/* seg ports*/                                                                                  \
    AAS_SET_INDEX_EQUAL_EVENT_PER_SEG_PORTS(_tile,0,LINK_STATUS_CHANGE_E),                        \
    AAS_SET_INDEX_EQUAL_EVENT_PER_SEG_PORTS(_tile,1,LINK_STATUS_CHANGE_E),                        \
    AAS_HALF_DP_SET_INDEX_EQUAL_EVENT_PER_SEG_PORTS(_tile,2,LINK_STATUS_CHANGE_E)

#define PORTS_LINK_OK_CHANGE_EVENTS(_tile) \
                                                                                                \
    /* MTI total ports 0..255 */                                                                \
    AAS_SET_INDEX_EQUAL_EVENT_PER_MTI_PORTS(_tile,0, LINK_OK_CHANGE_E),                           \
    AAS_SET_INDEX_EQUAL_EVENT_PER_MTI_PORTS(_tile,1, LINK_OK_CHANGE_E),                           \
    AAS_HALF_DP_SET_INDEX_EQUAL_EVENT_PER_MTI_PORTS(_tile,2, LINK_OK_CHANGE_E),                   \
/* seg ports*/                                                                                  \
    AAS_SET_INDEX_EQUAL_EVENT_PER_SEG_PORTS(_tile,0, LINK_OK_CHANGE_E),                           \
    AAS_SET_INDEX_EQUAL_EVENT_PER_SEG_PORTS(_tile,1, LINK_OK_CHANGE_E),                           \
    AAS_HALF_DP_SET_INDEX_EQUAL_EVENT_PER_SEG_PORTS(_tile,2, LINK_OK_CHANGE_E)

#define AAS_PORT_AN_HCD_RESOLUTION_DONE_EVENTS(_tile)                \
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_ANP_INSTANCE_0_PORT_NUM_0_HCD_FOUND_E,     0,\
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_ANP_INSTANCE_0_PORT_NUM_1_HCD_FOUND_E,     2,\
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_ANP_INSTANCE_0_PORT_NUM_2_HCD_FOUND_E,     4,\
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_ANP_INSTANCE_0_PORT_NUM_3_HCD_FOUND_E,     6,\
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_ANP_INSTANCE_0_PORT_NUM_4_HCD_FOUND_E,     8,\
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_ANP_INSTANCE_0_PORT_NUM_5_HCD_FOUND_E,    10,\
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_ANP_INSTANCE_0_PORT_NUM_6_HCD_FOUND_E,    12,\
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_ANP_INSTANCE_0_PORT_NUM_7_HCD_FOUND_E,    14,\
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_ANP_INSTANCE_1_PORT_NUM_0_HCD_FOUND_E,    16,\
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_ANP_INSTANCE_1_PORT_NUM_1_HCD_FOUND_E,    18,\
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_ANP_INSTANCE_1_PORT_NUM_2_HCD_FOUND_E,    20,\
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_ANP_INSTANCE_1_PORT_NUM_3_HCD_FOUND_E,    22,\
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_ANP_INSTANCE_1_PORT_NUM_4_HCD_FOUND_E,    24,\
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_ANP_INSTANCE_1_PORT_NUM_5_HCD_FOUND_E,    26,\
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_ANP_INSTANCE_1_PORT_NUM_6_HCD_FOUND_E,    28,\
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_ANP_INSTANCE_1_PORT_NUM_7_HCD_FOUND_E,    30,\
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_ANP_INSTANCE_2_PORT_NUM_0_HCD_FOUND_E,    32,\
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_ANP_INSTANCE_2_PORT_NUM_1_HCD_FOUND_E,    34,\
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_ANP_INSTANCE_2_PORT_NUM_2_HCD_FOUND_E,    36,\
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_ANP_INSTANCE_2_PORT_NUM_3_HCD_FOUND_E,    38


#define AAS_PORT_AN_RESTART_DONE_EVENTS(_tile)                \
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_ANP_INSTANCE_0_PORT_NUM_0_AN_RESTART_E,     0,\
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_ANP_INSTANCE_0_PORT_NUM_1_AN_RESTART_E,     2,\
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_ANP_INSTANCE_0_PORT_NUM_2_AN_RESTART_E,     4,\
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_ANP_INSTANCE_0_PORT_NUM_3_AN_RESTART_E,     6,\
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_ANP_INSTANCE_0_PORT_NUM_4_AN_RESTART_E,     8,\
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_ANP_INSTANCE_0_PORT_NUM_5_AN_RESTART_E,    10,\
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_ANP_INSTANCE_0_PORT_NUM_6_AN_RESTART_E,    12,\
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_ANP_INSTANCE_0_PORT_NUM_7_AN_RESTART_E,    14,\
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_ANP_INSTANCE_1_PORT_NUM_0_AN_RESTART_E,    16,\
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_ANP_INSTANCE_1_PORT_NUM_1_AN_RESTART_E,    18,\
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_ANP_INSTANCE_1_PORT_NUM_2_AN_RESTART_E,    20,\
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_ANP_INSTANCE_1_PORT_NUM_3_AN_RESTART_E,    22,\
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_ANP_INSTANCE_1_PORT_NUM_4_AN_RESTART_E,    24,\
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_ANP_INSTANCE_1_PORT_NUM_5_AN_RESTART_E,    26,\
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_ANP_INSTANCE_1_PORT_NUM_6_AN_RESTART_E,    28,\
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_ANP_INSTANCE_1_PORT_NUM_7_AN_RESTART_E,    30,\
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_ANP_INSTANCE_2_PORT_NUM_0_AN_RESTART_E,    32,\
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_ANP_INSTANCE_2_PORT_NUM_1_AN_RESTART_E,    34,\
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_ANP_INSTANCE_2_PORT_NUM_2_AN_RESTART_E,    36,\
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_ANP_INSTANCE_2_PORT_NUM_3_AN_RESTART_E,    38

#define AAS_PORT_INT_PM_PCS_LINK_TIMER_OUT_EVENTS(_tile)                \
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_ANP_INSTANCE_0_PORT_NUM_0_INT_PM_PCS_LINK_TIMER_OUT_E,     0,\
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_ANP_INSTANCE_0_PORT_NUM_1_INT_PM_PCS_LINK_TIMER_OUT_E,     2,\
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_ANP_INSTANCE_0_PORT_NUM_2_INT_PM_PCS_LINK_TIMER_OUT_E,     4,\
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_ANP_INSTANCE_0_PORT_NUM_3_INT_PM_PCS_LINK_TIMER_OUT_E,     6,\
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_ANP_INSTANCE_0_PORT_NUM_4_INT_PM_PCS_LINK_TIMER_OUT_E,     8,\
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_ANP_INSTANCE_0_PORT_NUM_5_INT_PM_PCS_LINK_TIMER_OUT_E,    10,\
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_ANP_INSTANCE_0_PORT_NUM_6_INT_PM_PCS_LINK_TIMER_OUT_E,    12,\
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_ANP_INSTANCE_0_PORT_NUM_7_INT_PM_PCS_LINK_TIMER_OUT_E,    14,\
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_ANP_INSTANCE_1_PORT_NUM_0_INT_PM_PCS_LINK_TIMER_OUT_E,    16,\
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_ANP_INSTANCE_1_PORT_NUM_1_INT_PM_PCS_LINK_TIMER_OUT_E,    18,\
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_ANP_INSTANCE_1_PORT_NUM_2_INT_PM_PCS_LINK_TIMER_OUT_E,    20,\
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_ANP_INSTANCE_1_PORT_NUM_3_INT_PM_PCS_LINK_TIMER_OUT_E,    22,\
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_ANP_INSTANCE_1_PORT_NUM_4_INT_PM_PCS_LINK_TIMER_OUT_E,    24,\
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_ANP_INSTANCE_1_PORT_NUM_5_INT_PM_PCS_LINK_TIMER_OUT_E,    26,\
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_ANP_INSTANCE_1_PORT_NUM_6_INT_PM_PCS_LINK_TIMER_OUT_E,    28,\
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_ANP_INSTANCE_1_PORT_NUM_7_INT_PM_PCS_LINK_TIMER_OUT_E,    30,\
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_ANP_INSTANCE_2_PORT_NUM_0_INT_PM_PCS_LINK_TIMER_OUT_E,    32,\
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_ANP_INSTANCE_2_PORT_NUM_1_INT_PM_PCS_LINK_TIMER_OUT_E,    34,\
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_ANP_INSTANCE_2_PORT_NUM_2_INT_PM_PCS_LINK_TIMER_OUT_E,    36,\
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_ANP_INSTANCE_2_PORT_NUM_3_INT_PM_PCS_LINK_TIMER_OUT_E,    38

#define AAS_PORT_LANE_SQ_EVENTS(_tile)                                 \
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_SDW_INSTANCE_0_SDW_INTERRUPT_CAUSE_SQ_DETECT_0_E,                 0,\
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_SDW_INSTANCE_0_SDW_INTERRUPT_CAUSE_SQ_DETECT_1_E,                 1,\
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_SDW_INSTANCE_0_SDW_INTERRUPT_CAUSE_SQ_DETECT_2_E,                 2,\
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_SDW_INSTANCE_0_SDW_INTERRUPT_CAUSE_SQ_DETECT_3_E,                 3,\
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_SDW_INSTANCE_1_SDW_INTERRUPT_CAUSE_SQ_DETECT_0_E,                 4,\
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_SDW_INSTANCE_1_SDW_INTERRUPT_CAUSE_SQ_DETECT_1_E,                 5,\
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_SDW_INSTANCE_1_SDW_INTERRUPT_CAUSE_SQ_DETECT_2_E,                 6,\
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_SDW_INSTANCE_1_SDW_INTERRUPT_CAUSE_SQ_DETECT_3_E,                 7,\
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_SDW_INSTANCE_2_SDW_INTERRUPT_CAUSE_SQ_DETECT_0_E,                 8,\
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_SDW_INSTANCE_2_SDW_INTERRUPT_CAUSE_SQ_DETECT_1_E,                 9,\
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_SDW_INSTANCE_2_SDW_INTERRUPT_CAUSE_SQ_DETECT_2_E,                 10,\
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_SDW_INSTANCE_2_SDW_INTERRUPT_CAUSE_SQ_DETECT_3_E,                 11,\
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_SDW_INSTANCE_3_SDW_INTERRUPT_CAUSE_SQ_DETECT_0_E,                 12,\
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_SDW_INSTANCE_3_SDW_INTERRUPT_CAUSE_SQ_DETECT_1_E,                 13,\
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_SDW_INSTANCE_3_SDW_INTERRUPT_CAUSE_SQ_DETECT_2_E,                 14,\
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_SDW_INSTANCE_3_SDW_INTERRUPT_CAUSE_SQ_DETECT_3_E,                 15,\
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_SDW_INSTANCE_4_SDW_INTERRUPT_CAUSE_SQ_DETECT_0_E,                 16,\
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_SDW_INSTANCE_4_SDW_INTERRUPT_CAUSE_SQ_DETECT_1_E,                 17,\
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_SDW_INSTANCE_4_SDW_INTERRUPT_CAUSE_SQ_DETECT_2_E,                 18,\
    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_SDW_INSTANCE_4_SDW_INTERRUPT_CAUSE_SQ_DETECT_3_E,                 19

/* Interrupt cause to unified event map for aas
 This Table is for unified event with extended data

  Array structure:
  1. The first element of the array and the elements after MARK_END_OF_UNI_EV_CNS are unified event types.
  2. Elements after unified event type are pairs of interrupt cause and event extended data,
     until MARK_END_OF_UNI_EV_CNS.
*/
#define aasUniEvMapTableWithExtData_tile(_tile)             \
/*start with{*/                                             \
    CPSS_PP_PORT_PCS_ALIGN_LOCK_LOST_E,                     \
        PORTS_LINK_STATUS_CHANGE_EVENTS(_tile),                    \
    MARK_END_OF_UNI_EV_CNS,                                 \
                                                            \
    CPSS_PP_PORT_LINK_STATUS_CHANGED_E,                     \
        PORTS_LINK_OK_CHANGE_EVENTS(_tile),                        \
    MARK_END_OF_UNI_EV_CNS,                                 \
                                                            \
    CPSS_PP_PORT_AN_HCD_FOUND_E,                            \
        AAS_PORT_AN_HCD_RESOLUTION_DONE_EVENTS(_tile),             \
    MARK_END_OF_UNI_EV_CNS,                                 \
                                                            \
    CPSS_PP_PORT_AN_RESTART_E,                              \
        AAS_PORT_AN_RESTART_DONE_EVENTS(_tile),                    \
    MARK_END_OF_UNI_EV_CNS,                                 \
                                                            \
    CPSS_PP_PORT_AN_PARALLEL_DETECT_E,                      \
        AAS_PORT_INT_PM_PCS_LINK_TIMER_OUT_EVENTS(_tile),          \
    MARK_END_OF_UNI_EV_CNS,                                 \
                                                            \
    CPSS_PP_PORT_LANE_SQ_EVENT_E,                           \
        AAS_PORT_LANE_SQ_EVENTS(_tile),                            \
    MARK_END_OF_UNI_EV_CNS,                                 \
                                                            \
    AAS_COMMON_EVENTS(_tile)
/*end with};*/

static const GT_U32 aasUniEvMapTableWithExtData[] = {
    aasUniEvMapTableWithExtData_tile(0)
};
static const GT_U32 aas2TilesUniEvMapTableWithExtData[] = {
    aasUniEvMapTableWithExtData_tile(0),
    aasUniEvMapTableWithExtData_tile(1)
};


#define aasUniEvMapTableUburst(_tile)  \
    {CPSS_PP_TQ_PORT_MICRO_BURST_E ,        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_QFC_UNIT_0_QFC_INTERRUPT_FUNCTIONAL_CAUSE_UBURST_EVENT_INT_E},    \
    {CPSS_PP_TQ_PORT_MICRO_BURST_E ,        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_QFC_UNIT_1_QFC_INTERRUPT_FUNCTIONAL_CAUSE_UBURST_EVENT_INT_E},    \
    {CPSS_PP_TQ_PORT_MICRO_BURST_E ,        INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_QFC_UNIT_2_QFC_INTERRUPT_FUNCTIONAL_CAUSE_UBURST_EVENT_INT_E}

#define GDMA_MSG(_gdmaId,_ring,_type_new)    \
    PRV_CPSS_AAS_GDMA_##_gdmaId##_RING_##_ring##_INTERRUPT_CAUSE_##_type_new##_E

/* Interrupt cause to unified event map for aas
 This Table is for unified event without extended data
*/
#define aasUniEvMapTable_tile(_tile)   /*[][2] =*/ \
    {CPSS_PP_EB_SECURITY_BREACH_UPDATE_E,    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_L2I_BRIDGE_INTERRUPT_CAUSE_UPDATE_SECURITY_BREACH_REGISTER_INT_E         },                         \
    {CPSS_PP_MAC_NUM_OF_HOP_EXP_E,           INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_FDB_FDB_INTERRUPT_CAUSE_REGISTER_NUM_OF_HOP_EX_P_E                       },                         \
    {CPSS_PP_MAC_NA_LEARNED_E,               INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_FDB_FDB_INTERRUPT_CAUSE_REGISTER_NA_LEARNT_E                             },                         \
    {CPSS_PP_MAC_NA_NOT_LEARNED_E,           INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_FDB_FDB_INTERRUPT_CAUSE_REGISTER_NA_NOT_LEARNT_E                         },                         \
    {CPSS_PP_MAC_AGE_VIA_TRIGGER_ENDED_E,    INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_FDB_FDB_INTERRUPT_CAUSE_REGISTER_AGE_VIA_TRIGGER_ENDED_E                 },                         \
    {CPSS_PP_MAC_UPDATE_FROM_CPU_DONE_E,     INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_FDB_FDB_INTERRUPT_CAUSE_REGISTER_AU_PROC_COMPLETED_INT_E                 },                         \
    {CPSS_PP_MAC_MESSAGE_TO_CPU_READY_E,     INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_FDB_FDB_INTERRUPT_CAUSE_REGISTER_AU_MSG_TOCPU_READY_E                    },                         \
    {CPSS_PP_MAC_NA_SELF_LEARNED_E,          INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_FDB_FDB_INTERRUPT_CAUSE_REGISTER_NA_SELF_LEARNED_E                       },                         \
    {CPSS_PP_MAC_NA_FROM_CPU_LEARNED_E,      INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_FDB_FDB_INTERRUPT_CAUSE_REGISTER_N_AFROM_CPU_LEARNED_E                   },                         \
    {CPSS_PP_MAC_NA_FROM_CPU_DROPPED_E,      INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_FDB_FDB_INTERRUPT_CAUSE_REGISTER_N_AFROM_CPU_DROPPED_E                   },                         \
    {CPSS_PP_MAC_AGED_OUT_E,                 INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_FDB_FDB_INTERRUPT_CAUSE_REGISTER_AGED_OUT_E                              },                         \
    {CPSS_PP_MAC_FIFO_2_CPU_EXCEEDED_E,      INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_FDB_FDB_INTERRUPT_CAUSE_REGISTER_AU_FIFO_TO_CPU_IS_FULL_E                },                         \
    {CPSS_PP_EB_MG_ADDR_OUT_OF_RANGE_E,      INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_FDB_FDB_INTERRUPT_CAUSE_REGISTER_ADDRESS_OUT_OF_RANGE_E                  },                         \
    {CPSS_PP_TTI_CPU_ADDRESS_OUT_OF_RANGE_E, INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_TTI_TTI_ENGINE_INTERRUPT_CAUSE_CPU_ADDRESS_OUT_OF_RANGE_E                },                         \
    {CPSS_PP_PCL_MG_ADDR_OUT_OF_RANGE_E,     INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_PCL_PCL_UNIT_INTERRUPT_CAUSE_MG_ADDR_OUTOF_RANGE_E                       },                         \
    {CPSS_PP_BM_AGED_PACKET_E,               INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_SMB_MC_INSTANCE_0_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_0_INTERRUPT_AGE_ERR_E},                        \
    {CPSS_PP_BM_AGED_PACKET_E,               INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_SMB_MC_INSTANCE_0_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_1_INTERRUPT_AGE_ERR_E},                        \
    {CPSS_PP_BM_AGED_PACKET_E,               INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_SMB_MC_INSTANCE_0_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_2_INTERRUPT_AGE_ERR_E},                        \
    {CPSS_PP_BM_AGED_PACKET_E,               INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_SMB_MC_INSTANCE_0_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_3_INTERRUPT_AGE_ERR_E},                        \
    {CPSS_PP_BM_AGED_PACKET_E,               INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_SMB_MC_INSTANCE_0_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_4_INTERRUPT_AGE_ERR_E},                        \
    {CPSS_PP_BM_AGED_PACKET_E,               INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_SMB_MC_INSTANCE_0_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_5_INTERRUPT_AGE_ERR_E},                        \
    {CPSS_PP_BM_AGED_PACKET_E,               INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_SMB_MC_INSTANCE_0_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_6_INTERRUPT_AGE_ERR_E},                        \
    {CPSS_PP_BM_AGED_PACKET_E,               INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_SMB_MC_INSTANCE_0_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_7_INTERRUPT_AGE_ERR_E},                        \
    {CPSS_PP_BM_AGED_PACKET_E,               INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_SMB_MC_INSTANCE_0_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_8_INTERRUPT_AGE_ERR_E},                        \
    {CPSS_PP_BM_AGED_PACKET_E,               INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_SMB_MC_INSTANCE_0_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_9_INTERRUPT_AGE_ERR_E},                        \
    {CPSS_PP_BM_AGED_PACKET_E,               INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_SMB_MC_INSTANCE_0_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_10_INTERRUPT_AGE_ERR_E},                       \
    {CPSS_PP_BM_AGED_PACKET_E,               INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_SMB_MC_INSTANCE_0_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_11_INTERRUPT_AGE_ERR_E},                       \
    {CPSS_PP_BM_AGED_PACKET_E,               INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_SMB_MC_INSTANCE_0_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_12_INTERRUPT_AGE_ERR_E},                       \
    {CPSS_PP_BM_AGED_PACKET_E,               INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_SMB_MC_INSTANCE_0_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_13_INTERRUPT_AGE_ERR_E},                       \
    {CPSS_PP_BM_AGED_PACKET_E,               INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_SMB_MC_INSTANCE_0_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_14_INTERRUPT_AGE_ERR_E},                       \
    {CPSS_PP_BM_AGED_PACKET_E,               INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_SMB_MC_INSTANCE_0_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_15_INTERRUPT_AGE_ERR_E},                       \
    {CPSS_PP_BM_AGED_PACKET_E,               INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_SMB_MC_INSTANCE_1_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_0_INTERRUPT_AGE_ERR_E},                        \
    {CPSS_PP_BM_AGED_PACKET_E,               INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_SMB_MC_INSTANCE_1_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_1_INTERRUPT_AGE_ERR_E},                        \
    {CPSS_PP_BM_AGED_PACKET_E,               INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_SMB_MC_INSTANCE_1_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_2_INTERRUPT_AGE_ERR_E},                        \
    {CPSS_PP_BM_AGED_PACKET_E,               INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_SMB_MC_INSTANCE_1_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_3_INTERRUPT_AGE_ERR_E},                        \
    {CPSS_PP_BM_AGED_PACKET_E,               INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_SMB_MC_INSTANCE_1_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_4_INTERRUPT_AGE_ERR_E},                        \
    {CPSS_PP_BM_AGED_PACKET_E,               INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_SMB_MC_INSTANCE_1_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_5_INTERRUPT_AGE_ERR_E},                        \
    {CPSS_PP_BM_AGED_PACKET_E,               INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_SMB_MC_INSTANCE_1_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_6_INTERRUPT_AGE_ERR_E},                        \
    {CPSS_PP_BM_AGED_PACKET_E,               INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_SMB_MC_INSTANCE_1_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_7_INTERRUPT_AGE_ERR_E},                        \
    {CPSS_PP_BM_AGED_PACKET_E,               INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_SMB_MC_INSTANCE_1_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_8_INTERRUPT_AGE_ERR_E},                        \
    {CPSS_PP_BM_AGED_PACKET_E,               INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_SMB_MC_INSTANCE_1_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_9_INTERRUPT_AGE_ERR_E},                        \
    {CPSS_PP_BM_AGED_PACKET_E,               INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_SMB_MC_INSTANCE_1_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_10_INTERRUPT_AGE_ERR_E},                       \
    {CPSS_PP_BM_AGED_PACKET_E,               INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_SMB_MC_INSTANCE_1_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_11_INTERRUPT_AGE_ERR_E},                       \
    {CPSS_PP_BM_AGED_PACKET_E,               INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_SMB_MC_INSTANCE_1_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_12_INTERRUPT_AGE_ERR_E},                       \
    {CPSS_PP_BM_AGED_PACKET_E,               INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_SMB_MC_INSTANCE_1_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_13_INTERRUPT_AGE_ERR_E},                       \
    {CPSS_PP_BM_AGED_PACKET_E,               INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_SMB_MC_INSTANCE_1_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_14_INTERRUPT_AGE_ERR_E},                       \
    {CPSS_PP_BM_AGED_PACKET_E,               INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_SMB_MC_INSTANCE_1_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_15_INTERRUPT_AGE_ERR_E},                       \
    {CPSS_PP_BM_AGED_PACKET_E,               INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_SMB_MC_INSTANCE_2_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_0_INTERRUPT_AGE_ERR_E},                        \
    {CPSS_PP_BM_AGED_PACKET_E,               INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_SMB_MC_INSTANCE_2_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_1_INTERRUPT_AGE_ERR_E},                        \
    {CPSS_PP_BM_AGED_PACKET_E,               INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_SMB_MC_INSTANCE_2_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_2_INTERRUPT_AGE_ERR_E},                        \
    {CPSS_PP_BM_AGED_PACKET_E,               INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_SMB_MC_INSTANCE_2_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_3_INTERRUPT_AGE_ERR_E},                        \
    {CPSS_PP_BM_AGED_PACKET_E,               INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_SMB_MC_INSTANCE_2_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_4_INTERRUPT_AGE_ERR_E},                        \
    {CPSS_PP_BM_AGED_PACKET_E,               INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_SMB_MC_INSTANCE_2_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_5_INTERRUPT_AGE_ERR_E},                        \
    {CPSS_PP_BM_AGED_PACKET_E,               INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_SMB_MC_INSTANCE_2_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_6_INTERRUPT_AGE_ERR_E},                        \
    {CPSS_PP_BM_AGED_PACKET_E,               INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_SMB_MC_INSTANCE_2_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_7_INTERRUPT_AGE_ERR_E},                        \
    {CPSS_PP_BM_AGED_PACKET_E,               INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_SMB_MC_INSTANCE_2_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_8_INTERRUPT_AGE_ERR_E},                        \
    {CPSS_PP_BM_AGED_PACKET_E,               INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_SMB_MC_INSTANCE_2_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_9_INTERRUPT_AGE_ERR_E},                        \
    {CPSS_PP_BM_AGED_PACKET_E,               INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_SMB_MC_INSTANCE_2_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_10_INTERRUPT_AGE_ERR_E},                       \
    {CPSS_PP_BM_AGED_PACKET_E,               INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_SMB_MC_INSTANCE_2_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_11_INTERRUPT_AGE_ERR_E},                       \
    {CPSS_PP_BM_AGED_PACKET_E,               INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_SMB_MC_INSTANCE_2_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_12_INTERRUPT_AGE_ERR_E},                       \
    {CPSS_PP_BM_AGED_PACKET_E,               INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_SMB_MC_INSTANCE_2_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_13_INTERRUPT_AGE_ERR_E},                       \
    {CPSS_PP_BM_AGED_PACKET_E,               INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_SMB_MC_INSTANCE_2_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_14_INTERRUPT_AGE_ERR_E},                       \
    {CPSS_PP_BM_AGED_PACKET_E,               INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_SMB_MC_INSTANCE_2_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_15_INTERRUPT_AGE_ERR_E},                       \
    {CPSS_PP_MAC_BANK_LEARN_COUNTERS_OVERFLOW_E, INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_FDB_FDB_INTERRUPT_CAUSE_REGISTER_BLC_OVERFLOW_E                      },                         \
    {CPSS_PP_EM_AUTO_LEARN_INDIRECT_ADDRESS_OUT_OF_RANGE_EVENT_E,       INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_EM_EM_INTERRUPT_CAUSE_REGISTER_INDIRECT_ADDRESS_OUT_OF_RANGE_E        }, \
    {CPSS_PP_EM_AUTO_LEARN_FLOW_ID_ALLOCATION_FAILED_EVENT_E,           INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_EM_EM_INTERRUPT_CAUSE_REGISTER_FLOW_ID_ALLOCATION_FAILED_E            }, \
    {CPSS_PP_EM_AUTO_LEARN_FLOW_ID_RANGE_FULL_EVENT_E,                  INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_EM_EM_INTERRUPT_CAUSE_REGISTER_FLOW_ID_RANGE_FULL_E                   }, \
    {CPSS_PP_EM_AUTO_LEARN_FLOW_ID_THRESHOLD_CROSSED_EVENT_E,           INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_EM_EM_INTERRUPT_CAUSE_REGISTER_FLOW_ID_THRESHOLD_CROSSED_E            }, \
    {CPSS_PP_EM_AUTO_LEARN_FLOW_ID_EMPTY_EVENT_E,                       INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_EM_EM_INTERRUPT_CAUSE_REGISTER_FLOW_ID_EMPTY_E                        }, \
    {CPSS_PP_EM_AUTO_LEARN_COLLISION_EVENT_E,                           INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_EM_EM_INTERRUPT_CAUSE_REGISTER_AUTO_LEARNING_COLLISION_E              }, \
    {CPSS_PP_EM_AUTO_LEARN_ILLEGAL_ADDRESS_ACCESS_EVENT_E,              INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_EM_EM_INTERRUPT_CAUSE_REGISTER_ILLEGAL_ADDRESS_ACCESS_E               }, \
    {CPSS_PP_EM_AUTO_LEARN_ILLEGAL_TABLE_OVERLAP_EVENT_E,               INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_EM_EM_INTERRUPT_CAUSE_REGISTER_ILLEGAL_TABLE_OVERLAP_E                }, \
    {CPSS_PP_EM_AUTO_LEARN_CHANNEL_0_SHM_ANSWER_FIFO_OVERFLOW_EVENT_E,  INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_EM_EM_INTERRUPT_CAUSE_REGISTER_CHANNEL_0_SHM_ANSWER_FIFO_OVERFLOW_E   }, \
    {CPSS_PP_EM_AUTO_LEARN_CHANNEL_1_SHM_ANSWER_FIFO_OVERFLOW_EVENT_E,  INT_TILE_OFFSET(_tile)+PRV_CPSS_AAS_EM_EM_INTERRUPT_CAUSE_REGISTER_CHANNEL_1_SHM_ANSWER_FIFO_OVERFLOW_E   }, \
    {CPSS_PP_EB_AUQ_PENDING_E                                        ,  PRV_CPSS_AAS_CNM_CNM_TILE_##_tile##_GDMA_0_QUEUE_0_MESSAGE_DONE_INTERRUPT_E                               }, \
    {CPSS_PP_EB_FUQ_PENDING_E                                        ,  PRV_CPSS_AAS_CNM_CNM_TILE_##_tile##_GDMA_0_QUEUE_1_MESSAGE_DONE_INTERRUPT_E                               }, \
    aasUniEvMapTableUburst(_tile)

static const GT_U32 aasUniEvMapTable[][2] =
{
    aasUniEvMapTable_tile(0)
};

/* Interrupt cause to unified event map for Aas without extended data size */
static const GT_U32 aasUniEvMapTableSize = NUM_ELEMENTS_IN_ARR_MAC(aasUniEvMapTable);

static const GT_U32 aas2TilesUniEvMapTable[][2] =
{
    aasUniEvMapTable_tile(0),
    aasUniEvMapTable_tile(1)
};

/* Interrupt cause to unified event map for Aas without extended data size */
static const GT_U32 aas2TilesUniEvMapTableSize = NUM_ELEMENTS_IN_ARR_MAC(aas2TilesUniEvMapTable);

/**
* @internal prvCpssDxChAasPortEventPortMapConvert function
* @endinternal
*
* @brief   Converts port unified event port from extended data to MAC/PHY port.
* @param[in] evConvertType          - event convert type
* @param[inout] portNumPtr          - (pointer to) port from event extended data
*
* @retval GT_BAD_PTR                - port parameter is NULL pointer
* @retval GT_BAD_PARAM              - wrong port type
* @retval GT_OUT_OF_RANGE           - port index is out of range
*/
static GT_STATUS prvCpssDxChAasPortEventPortMapConvert
(
    IN GT_SW_DEV_NUM                    devNum,
    IN PRV_CPSS_EV_CONVERT_DIRECTION_ENT evConvertType,
    INOUT GT_U32                *portNumPtr
)
{
    CPSS_NULL_PTR_CHECK_MAC(portNumPtr);

    /* currently no convert needed */
    devNum = devNum;
    evConvertType = evConvertType;
    *portNumPtr = *portNumPtr;

    return GT_OK;
}
/**
* @internal prvCpssAasDedicatedEventsNonSharedConvertInfoSet function
* @endinternal
*
* @brief   set non-shared callbacks  needed by chIntCauseToUniEvConvertDedicatedTables(...) for the
*          Aas  devices.
*/

void prvCpssAasDedicatedEventsNonSharedConvertInfoSet(IN GT_U8   devNum)
{
    PRV_INTERRUPT_CTRL_GET(devNum).eventHwIndexToMacNumConvertFunc = prvCpssDxChAasPortEventPortMapConvert;
    return;
}

/**
* @internal aasDedicatedEventsConvertInfoSet function
* @endinternal
*
* @brief   set info needed by chIntCauseToUniEvConvertDedicatedTables(...) for the
*          Aas devices.
*/
static void aasDedicatedEventsConvertInfoSet(IN GT_U8   devNum)
{
    /* Interrupt cause to unified event map for Aas with extended data size */
    GT_U32 aasUniEvMapTableWithExtDataSize,aas2TilesUniEvMapTableWithExtDataSize;

    if(PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG(devNum)->numOfTiles == 1)
    {
        aasUniEvMapTableWithExtDataSize = NUM_ELEMENTS_IN_ARR_MAC(aasUniEvMapTableWithExtData);
        PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG(devNum)->intCtrl.uniEvMapTableWithExtDataArr   = aasUniEvMapTableWithExtData;

        PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG(devNum)->intCtrl.uniEvMapTableWithExtDataSize  = aasUniEvMapTableWithExtDataSize;
        PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG(devNum)->intCtrl.uniEvMapTableArr              = aasUniEvMapTable;
        PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG(devNum)->intCtrl.uniEvMapTableSize             = aasUniEvMapTableSize;
    }
    else
    {
        aas2TilesUniEvMapTableWithExtDataSize = NUM_ELEMENTS_IN_ARR_MAC(aas2TilesUniEvMapTableWithExtData);
        PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG(devNum)->intCtrl.uniEvMapTableWithExtDataArr   = aas2TilesUniEvMapTableWithExtData;

        PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG(devNum)->intCtrl.uniEvMapTableWithExtDataSize  = aas2TilesUniEvMapTableWithExtDataSize;
        PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG(devNum)->intCtrl.uniEvMapTableArr              = aas2TilesUniEvMapTable;
        PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG(devNum)->intCtrl.uniEvMapTableSize             = aas2TilesUniEvMapTableSize;
    }

    PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG(devNum)->intCtrl.numReservedPorts              = 0;
    prvCpssAasDedicatedEventsNonSharedConvertInfoSet(devNum);

    return;
}

static void aasUpdateIntScanTreeInfo(
    IN GT_U8   devNum,
    IN PRV_CPSS_DRV_INTERRUPT_SCAN_STC *intrScanArr,
    IN GT_U32  numOfElements
)
{
    GT_U32  ii;
    GT_U32  baseAddr;
    PRV_CPSS_DXCH_UNIT_ENT  unitId;
    GT_BOOL didError;

    for(ii = 0 ; ii < numOfElements ; ii++)
    {
        unitId = UNIT_ID_GET ((PRV_CPSS_DXCH_UNIT_ENT)intrScanArr[ii].gppId);

        didError = GT_FALSE;

        baseAddr = prvCpssDxChHwUnitBaseAddrGet(devNum, unitId, &didError);

        if(didError == GT_TRUE)
        {

            /*support MG units */
            intrScanArr[ii].maskRegAddr  = CPSS_EVENT_SKIP_MASK_REG_ADDR_CNS;

            /* NOTE: this macro not 'return' */
            CPSS_LOG_ERROR_MAC("Not supported unit [%d] in Aas", unitId);
        }
        else
        {
            if(baseAddr > 0 && intrScanArr[ii].causeRegAddr > baseAddr)
            {
                /* ERROR : all addresses MUST be 0 based !!! */
                /* ERROR : all addresses MUST be 0 based !!! */
                /* ERROR : all addresses MUST be 0 based !!! */

               /* because we can't mask the address !!! */
               cpssOsPrintf("ERROR : Aas interrupts tree : NODE hold causeRegAddr [0x%8.8x] but must be relative to the start of the unit [0x%8.8x] \n",
                intrScanArr[ii].causeRegAddr,
                baseAddr);

                continue;/* do not ADD the base --- ERROR */
            }

            intrScanArr[ii].causeRegAddr += baseAddr;
            intrScanArr[ii].maskRegAddr  += baseAddr;
        }
    }
}

extern GT_STATUS prvCpssDxChUnitBaseTableInit(IN CPSS_PP_FAMILY_TYPE_ENT devFamily);

/**
* @internal prvCpssDrvPpIntDefDxChAasMacSecRegIdInit function
* @endinternal
*
* @brief   Init macsec register ids array.
*
* */
static GT_STATUS prvCpssDrvPpIntDefDxChAasMacSecRegIdInit
(
    GT_VOID
)
{
    GT_U32 iter;

    /* check that Global DB size is in synch with required one */
    if (PRV_CPSS_GLOBAL_DB_AAS_MACSEC_REGID_ARRAY_SIZE_CNS != PRV_CPSS_DXCH_AAS_MACSEC_REGID_ARRAY_SIZE_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    for(iter = 0; iter < PRV_CPSS_DXCH_AAS_MACSEC_REGID_ARRAY_SIZE_CNS - 1; iter++)
    {
        PRV_SHARED_COMMON_INT_DIR_DXEXMX_GLOBAL_VAR_SET(macSecRegIdNotAccessibleBeforeMacSecInitAas[iter],
            PRV_CPSS_DXCH_AAS_MACSEC_REGID_FIRST_CNS + iter);
    }
    /* end identifier for the array */
    PRV_SHARED_COMMON_INT_DIR_DXEXMX_GLOBAL_VAR_SET(macSecRegIdNotAccessibleBeforeMacSecInitAas[iter],
        PRV_CPSS_MACSEC_REGID_LAST_CNS);
    return GT_OK;
}

/**
* @internal prvCpssDrvPpIntDefDxChAasInit function
* @endinternal
*
* @brief   Interrupts initialization for the Aas devices.
*
* @param[in] devNum                   - the device number
* @param[in] ppRevision               - the revision of the device
*
* @retval GT_OK                    - on success,
* @retval GT_OUT_OF_CPU_MEM        - fail to allocate cpu memory (osMalloc)
* @retval GT_BAD_PARAM             - the scan tree information has error
*/
GT_STATUS prvCpssDrvPpIntDefDxChAasInit
(
    IN GT_U8   devNum ,
    IN GT_U32   ppRevision
)
{
    GT_STATUS   rc;         /* return code */
    GT_U32      index;
    GT_U32      ii;
    PRV_CPSS_DRV_DEVICE_INTERRUPTS_INFO_STC * devInterruptInfoPtr;
                            /* pointer to device interrupt info */
    PRV_CPSS_DRV_INTERRUPTS_INFO_STC devFamilyInterrupstInfo[] =
    {
        {
            NULL,
            PRV_CPSS_DRV_FAMILY_DXCH_AAS_1_TILE_E,
            AAS1_NUM_ELEMENTS_IN_SCAN_TREE_CNS ,
            aas1IntrScanArr,
            PRV_CPSS_AAS_LAST_INT_E/32,
            NULL, NULL, NULL
        }
    };

    PRV_CPSS_DRV_INTERRUPTS_INFO_STC    *devFamilyInterrupstInfoPtr;

    if(PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG(devNum)->numOfTiles > 1)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_IMPLEMENTED, LOG_ERROR_NO_MSG);
    }

    ii = 0;
    devFamilyInterrupstInfoPtr = &devFamilyInterrupstInfo[ii];

    index = PRV_CPSS_DRV_FAMILY_DXCH_AAS_1_TILE_E;

    ppRevision = ppRevision;

    /*set info needed by chIntCauseToUniEvConvertDedicatedTables(...) */
    aasDedicatedEventsConvertInfoSet(devNum);

    /* init macsec register ids */
    rc = prvCpssDrvPpIntDefDxChAasMacSecRegIdInit();
    if(rc != GT_OK)
    {
        /* error */
        return rc;
    }

    if(PRV_SHARED_COMMON_INT_DIR_DXEXMX_GLOBAL_VAR_GET(aasInitDone[ii]) == GT_TRUE)
    {
        return GT_OK;
    }

    rc = prvCpssDrvDxExMxInterruptsMemoryInit(&devFamilyInterrupstInfo[0],
        NUM_ELEMENTS_IN_ARR_MAC(devFamilyInterrupstInfo));
    if(rc != GT_OK)
    {
        /* error */
        return rc;
    }

    (void)prvCpssDxChUnitBaseTableInit(PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG(devNum)->devFamily);/*needed for calling prvCpssDxChHwUnitBaseAddrGet(...) */


    /* 1. Align base addresses for mask registers */
    aasUpdateIntScanTreeInfo(devNum,
        devFamilyInterrupstInfoPtr->intrScanOutArr ,
        devFamilyInterrupstInfoPtr->numScanElements);

    /*
       1. fill the array of mask registers addresses
       2. fill the array of default values for the mask registers
       3. update the bits of nonSumBitMask in the scan tree
    */
    rc = prvCpssDrvExMxDxHwPpMaskRegInfoGet(
            PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG(devNum)->devFamily,
            devFamilyInterrupstInfoPtr->numScanElements          ,
            devFamilyInterrupstInfoPtr->intrScanOutArr              ,
            devFamilyInterrupstInfoPtr->numMaskRegisters         ,
            devFamilyInterrupstInfoPtr->maskRegDefaultSummaryArr ,
            devFamilyInterrupstInfoPtr->maskRegMapArr            );
    if(rc != GT_OK)
    {
        /* error */
        return rc;
    }

    rc = prvCpssDrvPpIntUnMappedMgSetSkip(devNum, devFamilyInterrupstInfoPtr->maskRegMapArr, devFamilyInterrupstInfoPtr->numMaskRegisters);
    if(rc != GT_OK)
    {
        /* error */
        return rc;
    }

    devInterruptInfoPtr = &PRV_SHARED_COMMON_INT_DIR_DXEXMX_GLOBAL_VAR_GET(prvCpssDrvDevFamilyInterruptInfoArray[index]);
    devInterruptInfoPtr->numOfScanElements               = devFamilyInterrupstInfoPtr->numScanElements         ;
    devInterruptInfoPtr->interruptsScanArray             = devFamilyInterrupstInfoPtr->intrScanOutArr             ;
    devInterruptInfoPtr->maskRegistersDefaultValuesArray = devFamilyInterrupstInfoPtr->maskRegDefaultSummaryArr;
    devInterruptInfoPtr->maskRegistersAddressesArray     = devFamilyInterrupstInfoPtr->maskRegMapArr           ;

    devInterruptInfoPtr->numOfInterrupts = devFamilyInterrupstInfoPtr->numMaskRegisters * 32;
    devInterruptInfoPtr->hasFakeInterrupts = GT_FALSE;
    devInterruptInfoPtr->firstFakeInterruptId = 0;/* don't care */
    devInterruptInfoPtr->drvIntCauseToUniEvConvertFunc = &prvCpssDrvPpPortGroupIntCheetahIntCauseToUniEvConvert;
    devInterruptInfoPtr->numOfInterruptRegistersNotAccessibleBeforeStartInit = 0;
    devInterruptInfoPtr->notAccessibleBeforeStartInitPtr = NULL;
    PRV_INTERRUPT_CTRL_GET(devNum).intMaskSetFptr = NULL;
    devInterruptInfoPtr->macSecRegIdArray =
        PRV_SHARED_COMMON_INT_DIR_DXEXMX_GLOBAL_VAR_GET(macSecRegIdNotAccessibleBeforeMacSecInitAas);

    PRV_SHARED_COMMON_INT_DIR_DXEXMX_GLOBAL_VAR_SET(aasInitDone[ii], GT_TRUE);

    return GT_OK;
}




#ifdef DUMP_DEFAULT_INFO

/**
* @internal prvCpssDrvPpIntDefAasPrint function
* @endinternal
*
* @brief   print the interrupts arrays info of Aas devices
*/
void  prvCpssDrvPpIntDefAasPrint(
    IN GT_U32                       numOfTiles
)
{
    GT_U32  index;
    PRV_CPSS_DRV_DEVICE_INTERRUPTS_INFO_STC * devInterruptInfoPtr;
    PRV_CPSS_DRV_INTERRUPT_SCAN_STC *  interruptsScanArray;

    if(numOfTiles > 1)
    {
        cpssOsPrintf("prvCpssDrvPpIntDefAasPrint : not implemented for 2 tiles device \n");
        return ;
    }

    index = PRV_CPSS_DRV_FAMILY_DXCH_AAS_1_TILE_E;

    cpssOsPrintf("Aas - start : \n");
    devInterruptInfoPtr = &PRV_SHARED_COMMON_INT_DIR_DXEXMX_GLOBAL_VAR_GET(prvCpssDrvDevFamilyInterruptInfoArray[index]);

    /* remove the 'const' from the array */
    interruptsScanArray = devInterruptInfoPtr->interruptsScanArray;
    /* port group 0 */
    prvCpssDrvPpIntDefPrint(devInterruptInfoPtr->numOfScanElements,
                            interruptsScanArray,
                            devInterruptInfoPtr->numOfInterrupts,
                            devInterruptInfoPtr->maskRegistersAddressesArray,
                            devInterruptInfoPtr->maskRegistersDefaultValuesArray);
    cpssOsPrintf("Aas - End : \n");

}
#endif/*DUMP_DEFAULT_INFO*/

/**
* @internal prvCpssDrvPpIntDefAasPrint_regInfoByInterruptIndex function
* @endinternal
*
* @brief   print for Aas devices the register info according to value in
*         PRV_CPSS_AAS_LAST_INT_E (assuming that this register is part of
*         the interrupts tree)
*/
void  prvCpssDrvPpIntDefAasPrint_regInfoByInterruptIndex(
    IN PRV_CPSS_AAS_INT_CAUSE_ENT   interruptId
)
{
    GT_U32  ii;
    PRV_CPSS_DRV_DEVICE_INTERRUPTS_INFO_STC * devInterruptInfoPtr;
    GT_CHAR*    deviceNamePtr = "Aas interrupts";
    GT_U32      numMaskRegisters = PRV_CPSS_AAS_LAST_INT_E/32;

    cpssOsPrintf("Aas - start regInfoByInterruptIndex : \n");

    if(interruptId >= PRV_CPSS_AAS_LAST_INT_E)
    {
        cpssOsPrintf("interruptId[%d] >= maximum(%d) \n" ,
            interruptId, PRV_CPSS_AAS_LAST_INT_E);
    }
    else
    {
        for(ii = 0 ; ii < 2 ; ii++)
        {
            if(PRV_SHARED_COMMON_INT_DIR_DXEXMX_GLOBAL_VAR_GET(aasInitDone[ii]) == GT_FALSE)
            {
                continue;
            }

            if((GT_U32)interruptId >= (numMaskRegisters << 5))
            {
                cpssOsPrintf("[%s] skipped : interruptId[%d] >= maximum(%d) \n" ,
                    deviceNamePtr , interruptId, (numMaskRegisters << 5));
            }

            devInterruptInfoPtr = &PRV_SHARED_COMMON_INT_DIR_DXEXMX_GLOBAL_VAR_GET(prvCpssDrvDevFamilyInterruptInfoArray[ii+PRV_CPSS_DRV_FAMILY_DXCH_AAS_1_TILE_E]);

            cpssOsPrintf("start [%s]: \n", deviceNamePtr);
            prvCpssDrvPpIntDefPrint_regInfoByInterruptIndex(devInterruptInfoPtr->numOfScanElements, devInterruptInfoPtr->interruptsScanArray, interruptId, GT_FALSE);
            cpssOsPrintf("ended [%s] \n\n", deviceNamePtr);
        }
    }
    cpssOsPrintf("Aas - End regInfoByInterruptIndex : \n");
}

#define LIST_4(_n1, _n2, _i0, _i1, _i2, _i3) \
    _n1##_i0##_n2, _n1##_i1##_n2, _n1##_i2##_n2, _n1##_i3##_n2
#define LIST_8(_n1, _n2, _i0, _i1, _i2, _i3, _i4, _i5, _i6, _i7) \
    LIST_4(_n1, _n2, _i0, _i1, _i2, _i3), LIST_4(_n1, _n2, _i4, _i5, _i6, _i7)
#define LIST_0_64(_n1,_n2) \
    LIST_8(_n1, _n2, 0, 1, 2, 3, 4, 5, 6, 7), \
    LIST_8(_n1, _n2, 8, 9, 10, 11, 12, 13, 14, 15), \
    LIST_8(_n1, _n2, 16, 17, 18, 19, 20, 21, 22, 23), \
    LIST_8(_n1, _n2, 24, 25, 26, 27, 28, 29, 30, 31), \
    LIST_8(_n1, _n2, 32, 33, 34, 35, 36, 37, 38, 39), \
    LIST_8(_n1, _n2, 40, 41, 42, 43, 44, 45, 46, 47), \
    LIST_8(_n1, _n2, 48, 49, 50, 51, 52, 53, 54, 55), \
    LIST_8(_n1, _n2, 56, 57, 58, 59, 60, 61, 62, 63),

#define AAS_INT_NAME_RX_SDMA_QUEUE(_type_old, _tileId, _gdmaId, _queueId) \
    PRV_CPSS_AAS_CNM_CNM_TILE_##_tileId##_GDMA_##_gdmaId##_QUEUE_##_queueId##_##_type_old##_INTERRUPT_E

#define AAS_INT_NAME_RX_SDMA_MG(type, _tileId, _gdmaId)      \
    AAS_INT_NAME_RX_SDMA_QUEUE(type, _tileId, _gdmaId, 0),   \
    AAS_INT_NAME_RX_SDMA_QUEUE(type, _tileId, _gdmaId, 1),   \
    AAS_INT_NAME_RX_SDMA_QUEUE(type, _tileId, _gdmaId, 2),   \
    AAS_INT_NAME_RX_SDMA_QUEUE(type, _tileId, _gdmaId, 3),   \
    AAS_INT_NAME_RX_SDMA_QUEUE(type, _tileId, _gdmaId, 4),   \
    AAS_INT_NAME_RX_SDMA_QUEUE(type, _tileId, _gdmaId, 5),   \
    AAS_INT_NAME_RX_SDMA_QUEUE(type, _tileId, _gdmaId, 6),   \
    AAS_INT_NAME_RX_SDMA_QUEUE(type, _tileId, _gdmaId, 7),   \
    AAS_INT_NAME_RX_SDMA_QUEUE(type, _tileId, _gdmaId, 8),   \
    AAS_INT_NAME_RX_SDMA_QUEUE(type, _tileId, _gdmaId, 9),   \
    AAS_INT_NAME_RX_SDMA_QUEUE(type, _tileId, _gdmaId, 10),   \
    AAS_INT_NAME_RX_SDMA_QUEUE(type, _tileId, _gdmaId, 11),   \
    AAS_INT_NAME_RX_SDMA_QUEUE(type, _tileId, _gdmaId, 12),   \
    AAS_INT_NAME_RX_SDMA_QUEUE(type, _tileId, _gdmaId, 13),   \
    AAS_INT_NAME_RX_SDMA_QUEUE(type, _tileId, _gdmaId, 14),   \
    AAS_INT_NAME_RX_SDMA_QUEUE(type, _tileId, _gdmaId, 15),   \
    AAS_INT_NAME_RX_SDMA_QUEUE(type, _tileId, _gdmaId, 16),   \
    AAS_INT_NAME_RX_SDMA_QUEUE(type, _tileId, _gdmaId, 17),   \
    AAS_INT_NAME_RX_SDMA_QUEUE(type, _tileId, _gdmaId, 18),   \
    AAS_INT_NAME_RX_SDMA_QUEUE(type, _tileId, _gdmaId, 19),   \
    AAS_INT_NAME_RX_SDMA_QUEUE(type, _tileId, _gdmaId, 20),   \
    AAS_INT_NAME_RX_SDMA_QUEUE(type, _tileId, _gdmaId, 21),   \
    AAS_INT_NAME_RX_SDMA_QUEUE(type, _tileId, _gdmaId, 22),   \
    AAS_INT_NAME_RX_SDMA_QUEUE(type, _tileId, _gdmaId, 23),   \
    AAS_INT_NAME_RX_SDMA_QUEUE(type, _tileId, _gdmaId, 24),   \
    AAS_INT_NAME_RX_SDMA_QUEUE(type, _tileId, _gdmaId, 25),   \
    AAS_INT_NAME_RX_SDMA_QUEUE(type, _tileId, _gdmaId, 26),   \
    AAS_INT_NAME_RX_SDMA_QUEUE(type, _tileId, _gdmaId, 27),   \
    AAS_INT_NAME_RX_SDMA_QUEUE(type, _tileId, _gdmaId, 28),   \
    AAS_INT_NAME_RX_SDMA_QUEUE(type, _tileId, _gdmaId, 29),   \
    AAS_INT_NAME_RX_SDMA_QUEUE(type, _tileId, _gdmaId, 30),   \
    AAS_INT_NAME_RX_SDMA_QUEUE(type, _tileId, _gdmaId, 31)

#define AAS_INT_NAME_RX_SDMA(type)                         \
    AAS_INT_NAME_RX_SDMA_MG(type, 0/*tile*/,0/*GDMA*/),    \
    AAS_INT_NAME_RX_SDMA_MG(type, 0/*tile*/,1/*GDMA*/),    \
    AAS_INT_NAME_RX_SDMA_MG(type, 0/*tile*/,2/*GDMA*/),    \
    AAS_INT_NAME_RX_SDMA_MG(type, 0/*tile*/,3/*GDMA*/)

/* uniEvent queue IDs - 0-127 */
static const CPSS_UNI_EV_CAUSE_ENT         uniEventRxBufQueueArr[64] =
{
    LIST_0_64(CPSS_PP_RX_BUFFER_QUEUE, _E)
};

/* uniEvent error IDs - 0-127 */
static const CPSS_UNI_EV_CAUSE_ENT         uniEventRxErrQueueArr[64] =
{
    LIST_0_64(CPSS_PP_RX_ERR_QUEUE, _E)
};

/* HW interrupt IDs RX BUF_QUEUE tile.mg.queue ordered */
static const GT_U32 aasInterruptRxBufQueArr[] =
{
    AAS_INT_NAME_RX_SDMA(RX_BUFFER)
};

/* HW interrupt IDs RX ERR tile.mg.queue ordered */
static const GT_U32 aasInterruptRxErrQueArr[] =
{
    AAS_INT_NAME_RX_SDMA(RX_ERR)
};

/**
* @internal prvCpssPpDrvIntDefDxChAasBindGdmaInterruptsToPortMappingDependedEvents function
* @endinternal
*
* @brief   update the RX TO_CPU interrupts unified event to connect the queues to rings
*          with the proper offset
*          similar to falcon function :
*          prvCpssPpDrvIntDefDxChFalconBindSdmaInterruptsToPortMappingDependedEvents
*
* @note   APPLICABLE DEVICES:      AAS.
* @note   NOT APPLICABLE DEVICES:  xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.

* @param[in] devNum             - device number.
* @param[in] gdmaUnit           - the GDMA unit to work on.
* @param[in] rxQueue0StartRing  - the actual offset the 'queue 0' should be connected to 'ring x'
* @param[in] numOfQueues        - number of queues (from queue 0) that need this offset
*
*/
GT_STATUS prvCpssPpDrvIntDefDxChAasBindGdmaInterruptsToPortMappingDependedEvents
(
    IN  GT_U8  devNum,
    IN  GT_U32 gdmaUnit,
    IN  GT_U32 rxQueue0StartRing,
    IN  GT_U32 numOfQueues
)
{
    GT_STATUS             rc;
    GT_U32                ii;
    GT_U32                interruptIndex;
    CPSS_UNI_EV_CAUSE_ENT uniEvent;
    GT_U32                uniEventExt;
    GT_U32                interruptOffset;
    GT_U32                index_to_uniEventArr;
    GT_U32                index_to_intArr;

    if((rxQueue0StartRing >= 64) ||
       ((rxQueue0StartRing + numOfQueues) > 64))
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, "rxQueue0StartRing[%d]+numOfQueues[%d] > 64",
            rxQueue0StartRing , numOfQueues);
    }

    interruptOffset = gdmaUnit * 32;

    for(ii = 0 ; ii < numOfQueues ; ii++)
    {
        index_to_intArr      = interruptOffset + rxQueue0StartRing + ii;
        index_to_uniEventArr = ii;

        /*CPSS_PP_RX_BUFFER_QUEUE0_E - CPSS_PP_RX_BUFFER_QUEUE63_E*/
        interruptIndex = aasInterruptRxBufQueArr[index_to_intArr];
        uniEvent       = uniEventRxBufQueueArr[index_to_uniEventArr];

        /* queue number according to updated uniEvent */
        uniEventExt = ii;
        rc = prvCpssDrvEventUpdate(
            devNum, interruptIndex,
            GT_TRUE /*invalidateOtherNodes*/,
            uniEvent, uniEventExt/*evExtData*/);
        if (rc != GT_OK)
        {
            return rc;
        }

        index_to_intArr      = interruptOffset + rxQueue0StartRing + ii;
        index_to_uniEventArr = ii;

        /*CPSS_PP_RX_ERR_QUEUE0_E - CPSS_PP_RX_ERR_QUEUE63_E*/
        interruptIndex = aasInterruptRxErrQueArr[index_to_intArr];
        uniEvent       = uniEventRxErrQueueArr[index_to_uniEventArr];

        /* queue number according to updated uniEvent */
        uniEventExt = ii;
        rc = prvCpssDrvEventUpdate(
            devNum, interruptIndex,
            GT_TRUE /*invalidateOtherNodes*/,
            uniEvent, uniEventExt/*evExtData*/);
        if (rc != GT_OK)
        {
            return rc;
        }
    }

    return GT_OK;
}

