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
* @file cpssDrvPpIntDefDxChIronman.c
*
* @brief This file includes the definition and initialization of the interrupts
* init. parameters tables. -- DxCh Ironman devices
*
* @version   1
********************************************************************************
*/

#include <cpssDriver/pp/interrupts/dxExMx/prvCpssDrvDxExMxInterrupts.h>
#include <cpssDriver/pp/interrupts/dxExMx/prvCpssDrvDxChEventsIronman.h>
#include <cpssDriver/pp/interrupts/dxExMx/prvCpssDrvExMxUtilLion.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwRegAddr.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/private/prvCpssDxChHwRegAddrVer1_ppDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

/* First macsec register id as per Ironman interrupts enum */
#define  PRV_CPSS_DXCH_IRONMAN_MACSEC_REGID_FIRST_CNS (PRV_CPSS_IRONMAN_MSEC_EGR_163_INST_0_GLOBAL_MAC_SEC_INTERRUPT_CAUSE_STAT_TCAM_THR_E >> 5)

#define  CPSS_PP_RX_ERROR_QUEUE0_E      CPSS_PP_RX_ERR_QUEUE0_E
#define  CPSS_PP_RX_ERROR_QUEUE1_E      CPSS_PP_RX_ERR_QUEUE1_E
#define  CPSS_PP_RX_ERROR_QUEUE2_E      CPSS_PP_RX_ERR_QUEUE2_E
#define  CPSS_PP_RX_ERROR_QUEUE3_E      CPSS_PP_RX_ERR_QUEUE3_E
#define  CPSS_PP_RX_ERROR_QUEUE4_E      CPSS_PP_RX_ERR_QUEUE4_E
#define  CPSS_PP_RX_ERROR_QUEUE5_E      CPSS_PP_RX_ERR_QUEUE5_E
#define  CPSS_PP_RX_ERROR_QUEUE6_E      CPSS_PP_RX_ERR_QUEUE6_E
#define  CPSS_PP_RX_ERROR_QUEUE7_E      CPSS_PP_RX_ERR_QUEUE7_E

/* convert bit in the enum of the huge interrupts of the device into BIT_0..BIT_31 in a register */
#define DEVICE_INT_TO_BIT_MAC(deviceInterruptId)    \
            (1 << ((deviceInterruptId) & 0x1f))

#define FDB_REG_MASK_CNS        \
    (0xFFFFFFFF & ~                                                             \
        (DEVICE_INT_TO_BIT_MAC(PRV_CPSS_IRONMAN_FDB_IP_FDB_INTERRUPT_CAUSE_REGISTER_AGE_VIA_TRIGGER_ENDED_E) |     \
         DEVICE_INT_TO_BIT_MAC(PRV_CPSS_IRONMAN_FDB_IP_FDB_INTERRUPT_CAUSE_REGISTER_AU_PROC_COMPLETED_INT_E)))

#define PRV_CPSS_IRONMAN_MIF_3_MIF_GLOBAL_INTERRUPT_SUMMARY_CAUSE_MIF_GLOBAL_CHANNEL_INTERRUPT_E        PRV_CPSS_IRONMAN_CPU_3_MIF_GLOBAL_INTERRUPT_SUMMARY_CAUSE_MIF_GLOBAL_CHANNEL_INTERRUPT_E
#define PRV_CPSS_IRONMAN_MIF_3_MIF_GLOBAL_INTERRUPT_SUMMARY_CAUSE_MIF_GLOBAL_BAD_ADDRESS_INTERRUPT_E    PRV_CPSS_IRONMAN_CPU_3_MIF_GLOBAL_INTERRUPT_SUMMARY_CAUSE_MIF_GLOBAL_BAD_ADDRESS_INTERRUPT_E
#define PRV_CPSS_IRONMAN_LED_3_LED_INTERRUPT_CAUSE_ILLEGAL_ADDRESS_ACCESS_E                             PRV_CPSS_IRONMAN_LED_CPU_3_LED_INTERRUPT_CAUSE_ILLEGAL_ADDRESS_ACCESS_E

/* unit : PRV_CPSS_DXCH_UNIT_MG_E */
#define MG_UNIT(unit,_mgIndex) \
    (_mgIndex ? (PRV_CPSS_DXCH_UNIT_MG_0_1_E + (_mgIndex-1)) : PRV_CPSS_DXCH_UNIT_MG_E)

#define UNIT_ID_GET(unit) ((unit) & 0xFFFF)

/* MTI 10G PCS x8 MAC - cause */
#define PORT_MTI_10G_PCS_X8_MAC_INTERRUPT_CAUSE_MAC(port)               \
        (0x50 + ((port) * 0x4))
/* MTI 10G PCS x8 MAC - mask */
#define PORT_MTI_10G_PCS_X8_MAC_INTERRUPT_MASK_MAC(port)                \
        (0x70 + ((port) * 0x4))

/* CSTU Channel group - cause */
#define CSTU_CHANNEL_GROUP_INTERRUPT_CAUSE_MAC(group)                   \
        (0x3800 + ((group) * 0x4))

/* CSTU Channel - mask */
#define CSTU_CHANNEL_GROUP_INTERRUPT_MASK_MAC(group)                    \
        (0x3820 + ((group) * 0x4))

/* USX ports - cause */
#define PORT_MTI_USX_X8_MAC_INTERRUPT_CAUSE_MAC(port)                   \
        (0x10C + ((port) * 0x100))

/* USX ports - cause */
#define PORT_MTI_USX_X8_MAC_INTERRUPT_MASK_MAC(port)                    \
        (0x110 + ((port) * 0x100))

/* CSTU Channel - cause */
#define CSTU_CHANNEL_INTERRUPT_CAUSE_MAC(channel)                       \
        (0x3000 + ((channel) * 0x4))

/* CSTU Channel - mask */
#define CSTU_CHANNEL_INTERRUPT_MASK_MAC(channel)                        \
        (0x3400 + ((channel) * 0x4))

/* USX ports - summary cause */
#define PORT_MTI_USX_X8_MAC_INTERRUPT_SUMMARY_CAUSE_MAC(port)           \
        (0x168 + ((port) * 0x100))

/* USX ports - summary mask */
#define PORT_MTI_USX_X8_MAC_INTERRUPT_SUMMARY_MASK_MAC(port)            \
        (0x16c + ((port) * 0x100))

GT_STATUS prvCpssGenericSrvCpuRegisterRead
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId,
    IN GT_U32   regAddr,
    IN GT_U32   *dataPtr
);
GT_STATUS prvCpssGenericSrvCpuRegisterWrite
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId,
    IN GT_U32   regAddr,
    IN GT_U32   value
);

#define   AP_Doorbell_SUB_TREE_MAC(bit, _mgIndex)                                                   \
    /* AP_DOORBELL_MAIN */                                                                          \
    {bit,GT_FALSE,MG_UNIT(PRV_CPSS_DXCH_UNIT_MG_E,_mgIndex), NULL, 0x00000010, 0x00000011,          \
        prvCpssGenericSrvCpuRegisterClearOnRead,                                                           \
        prvCpssGenericSrvCpuRegisterWrite,                                                          \
        PRV_CPSS_IRONMAN_MG_##_mgIndex##_AP_DOORBELL_MAIN_IPC_E,                                    \
        PRV_CPSS_IRONMAN_MG_##_mgIndex##_AP_DOORBELL_MAIN_RESERVED31_E,                             \
        FILLED_IN_RUNTIME_CNS, 0, 0xFFFFF880, 4,NULL, NULL},                               \
        /* AP_DOORBELL_PORT0_31_LINK_STATUS_CHANGE */                                               \
        {4,GT_FALSE,MG_UNIT(PRV_CPSS_DXCH_UNIT_MG_E,_mgIndex), NULL, 0x00000012, 0x00000013,        \
            prvCpssGenericSrvCpuRegisterClearOnRead,                                                       \
            prvCpssGenericSrvCpuRegisterWrite,                                                      \
            PRV_CPSS_IRONMAN_MG_##_mgIndex##_AP_DOORBELL_PORT_0_LINK_STATUS_CHANGE_E,               \
            PRV_CPSS_IRONMAN_MG_##_mgIndex##_AP_DOORBELL_PORT_31_LINK_STATUS_CHANGE_E,              \
            FILLED_IN_RUNTIME_CNS, 0, 0x0, 0,NULL, NULL},                                  \
        /* AP_DOORBELL_PORT32_63_LINK_STATUS_CHANGE */                                              \
        {5,GT_FALSE,MG_UNIT(PRV_CPSS_DXCH_UNIT_MG_E,_mgIndex), NULL, 0x00000014, 0x00000015,        \
            prvCpssGenericSrvCpuRegisterClearOnRead,                                                       \
            prvCpssGenericSrvCpuRegisterWrite,                                                      \
            PRV_CPSS_IRONMAN_MG_##_mgIndex##_AP_DOORBELL_PORT_32_LINK_STATUS_CHANGE_E,              \
            PRV_CPSS_IRONMAN_MG_##_mgIndex##_AP_DOORBELL_PORT_63_LINK_STATUS_CHANGE_E,              \
            FILLED_IN_RUNTIME_CNS, 0, 0x0, 0,NULL, NULL},                                  \
        /* AP_DOORBELL_PORT0_31_802_3_AP */                                                         \
        {8,GT_FALSE,MG_UNIT(PRV_CPSS_DXCH_UNIT_MG_E,_mgIndex), NULL, 0x00000018, 0x00000019,        \
            prvCpssGenericSrvCpuRegisterClearOnRead,                                                       \
            prvCpssGenericSrvCpuRegisterWrite,                                                      \
            PRV_CPSS_IRONMAN_MG_##_mgIndex##_AP_DOORBELL_PORT_0_802_3_AP_E,                         \
            PRV_CPSS_IRONMAN_MG_##_mgIndex##_AP_DOORBELL_PORT_31_802_3_AP_E,                        \
            FILLED_IN_RUNTIME_CNS, 0, 0x0, 0,NULL, NULL},                                  \
        /* AP_DOORBELL_PORT32_63_802_3_AP */                                                        \
        {9,GT_FALSE,MG_UNIT(PRV_CPSS_DXCH_UNIT_MG_E,_mgIndex), NULL, 0x0000001a, 0x0000001b,        \
            prvCpssGenericSrvCpuRegisterClearOnRead,                                                       \
            prvCpssGenericSrvCpuRegisterWrite,                                                      \
            PRV_CPSS_IRONMAN_MG_##_mgIndex##_AP_DOORBELL_PORT_32_802_3_AP_E,                        \
            PRV_CPSS_IRONMAN_MG_##_mgIndex##_AP_DOORBELL_PORT_63_802_3_AP_E,                        \
            FILLED_IN_RUNTIME_CNS, 0, 0x0, 0,NULL, NULL}

#define   mg0InternalIntsSum_SUB_TREE_MAC(bit, _mgIndex)                                                \
    /* Miscellaneous */                                                                                 \
    {bit, GT_FALSE, MG_UNIT(PRV_CPSS_DXCH_UNIT_MG_E,_mgIndex), NULL,                                    \
        0x00000038, 0x0000003C,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_IRONMAN_MG_##_mgIndex##_MG_INTERNAL_INTERRUPT_CAUSE_TWSI_TIME_OUT_INT_E,               \
        PRV_CPSS_IRONMAN_MG_##_mgIndex##_MG_INTERNAL_INTERRUPT_CAUSE_SERDES_OUT_OF_RANGE_VIOLATION_E,   \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 1, NULL, NULL},                                         \
        /* HostCpuDoorbellIntSum - single register per MG accessed from MG Internal Cause0/1/2/3 */     \
        {24, GT_FALSE, MG_UNIT(PRV_CPSS_DXCH_UNIT_MG_E, _mgIndex), NULL,                                \
            0x00000518, 0x0000051c,                                                                     \
            prvCpssDrvHwPpPortGroupIsrRead,                                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
            PRV_CPSS_IRONMAN_MG##_mgIndex##_HOST_CPU_DOORBELL_SUM_DATA_TO_HOST_CPU_0_E,                 \
            PRV_CPSS_IRONMAN_MG##_mgIndex##_HOST_CPU_DOORBELL_SUM_DATA_TO_HOST_CPU_30_E,                \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFEF, 1, NULL, NULL},                                     \
            AP_Doorbell_SUB_TREE_MAC(4,_mgIndex)

#define   mgInternalIntsSum_SUB_TREE_MAC(bit, _mgIndex)                                                \
    /* Miscellaneous */                                                                                 \
    {bit, GT_FALSE, MG_UNIT(PRV_CPSS_DXCH_UNIT_MG_E,_mgIndex), NULL,                                    \
        0x00000038, 0x0000003C,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_IRONMAN_MG_##_mgIndex##_MG_INTERNAL_INTERRUPT_CAUSE_TWSI_TIME_OUT_INT_E,               \
        PRV_CPSS_IRONMAN_MG_##_mgIndex##_MG_INTERNAL_INTERRUPT_CAUSE_SERDES_OUT_OF_RANGE_VIOLATION_E,   \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 1, NULL, NULL},                                         \
        /* HostCpuDoorbellIntSum - single register per MG accessed from MG Internal Cause0/1/2/3 */     \
        {24, GT_FALSE, MG_UNIT(PRV_CPSS_DXCH_UNIT_MG_E, _mgIndex), NULL,                                \
            0x00000518, 0x0000051c,                                                                     \
            prvCpssDrvHwPpPortGroupIsrRead,                                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
            PRV_CPSS_IRONMAN_MG##_mgIndex##_HOST_CPU_DOORBELL_SUM_DATA_TO_HOST_CPU_0_E,                 \
            PRV_CPSS_IRONMAN_MG##_mgIndex##_HOST_CPU_DOORBELL_SUM_DATA_TO_HOST_CPU_30_E,                \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFEF, 0, NULL, NULL}

#define   mt_NODE_MAC(bit)                                                                              \
    /* MT FDBIntSum - FDB Interrupt Cause */                                                            \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_FDB_E, NULL,                                                     \
        0x000001B0, 0x000001B4,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_IRONMAN_FDB_IP_FDB_INTERRUPT_CAUSE_REGISTER_NUM_OF_HOP_EX_P_E,                         \
        PRV_CPSS_IRONMAN_FDB_IP_FDB_INTERRUPT_CAUSE_REGISTER_ADDRESS_OUT_OF_RANGE_E,                    \
        FILLED_IN_RUNTIME_CNS, 0x0, FDB_REG_MASK_CNS, 0, NULL, NULL}

#define   bma_NODE_MAC(bit)                                                     \
        /* bma  */                                                                  \
        {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_BMA_E, NULL,                             \
            0x00059000, 0x00059004,                                                 \
            prvCpssDrvHwPpPortGroupIsrRead,                                         \
            prvCpssDrvHwPpPortGroupIsrWrite,                                        \
            PRV_CPSS_IRONMAN_BMA_MC_CNT_RMW_CLEAR_FIFO_FULL_E,                         \
            PRV_CPSS_IRONMAN_BMA_FINAL_CLEAR_UDB_FIFO0_FULL_E,                         \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define   cnc_SUB_TREE_MAC(bit)                                                                         \
    /* CNC-0  */                                                                                        \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_CNC_0_E, NULL,                                                   \
        0x00000100, 0x00000104,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_IRONMAN_CNC_CNC_INTERRUPT_SUMMARY_CAUSE_WRAPAROUND_FUNC_INTERRUPT_SUM_E,               \
        PRV_CPSS_IRONMAN_CNC_CNC_INTERRUPT_SUMMARY_CAUSE_BLOCK_INTERRUPTS_SUM_E,                        \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 3, NULL, NULL},                                         \
                                                                                                        \
        /* WraparoundFuncInterruptSum  */                                                               \
        {1, GT_FALSE, PRV_CPSS_DXCH_UNIT_CNC_0_E, NULL,                                                 \
            0x00000190, 0x000001A4,                                                                     \
            prvCpssDrvHwPpPortGroupIsrRead,                                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
            PRV_CPSS_IRONMAN_CNC_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_BLOCK_0_WRAPAROUND_E,            \
            PRV_CPSS_IRONMAN_CNC_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_BLOCK_7_WRAPAROUND_E,            \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},                                      \
        /* RateLimitFuncInterruptSum  */                                                                \
        {2, GT_FALSE, PRV_CPSS_DXCH_UNIT_CNC_0_E, NULL,                                                 \
            0x000001B8, 0x000001CC,                                                                     \
            prvCpssDrvHwPpPortGroupIsrRead,                                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
            PRV_CPSS_IRONMAN_CNC_RATE_LIMIT_FUNCTIONAL_INTERRUPT_CAUSE_BLOCK_NUMBER_0_RATE_LIMIT_FIFO_DROP_E,  \
            PRV_CPSS_IRONMAN_CNC_RATE_LIMIT_FUNCTIONAL_INTERRUPT_CAUSE_BLOCK_NUMBER_7_RATE_LIMIT_FIFO_DROP_E,  \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},                                      \
        /* MiscFuncInterruptSum  */                                                                     \
        {3, GT_FALSE, PRV_CPSS_DXCH_UNIT_CNC_0_E, NULL,                                                 \
            0x000001E0, 0x000001E4,                                                                     \
            prvCpssDrvHwPpPortGroupIsrRead,                                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
            PRV_CPSS_IRONMAN_CNC_MISC_FUNCTIONAL_INTERRUPT_CAUSE_DUMP_FINISHED_E,                       \
            PRV_CPSS_IRONMAN_CNC_MISC_FUNCTIONAL_INTERRUPT_CAUSE_CNC_TABLE_OVERLAPPING_E,               \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

#define pdx_NODE_MAC(bit)                                                                               \
        /* PDX Interrupt Summary */                                                                     \
        {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_TXQ_PDX_E, NULL,                                             \
            0x00001408, 0x0000140c,                                                                     \
            prvCpssDrvHwPpPortGroupIsrRead,                                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
            PRV_CPSS_IRONMAN_TXQ_PR_PDX_FUNCTIONAL_INTERRUPT_CAUSE_ILLEGAL_ADDRESS_ACCESS_E,            \
            PRV_CPSS_IRONMAN_TXQ_PR_PDX_FUNCTIONAL_INTERRUPT_CAUSE_CHANNELS_RANGE_0_DX_DESC_DROP_E,     \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define pfcc_NODE_MAC(bit)                                                                              \
        /* PFCC Interrupt Summary */                                                                    \
        {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_TXQ_PFCC_E, NULL,                                            \
            0x00000100, 0x00000104,                                                                     \
            prvCpssDrvHwPpPortGroupIsrRead,                                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
            PRV_CPSS_IRONMAN_TXQ_PR_PFCC_INTERRUPT_CAUSE_ILLEGAL_ADDRESS_ACCESS_E,                      \
            PRV_CPSS_IRONMAN_TXQ_PR_PFCC_INTERRUPT_CAUSE_TOTAL_VIRTUAL_BUFFER_COUNTER_OVERFLOW_E,       \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define psi_NODE_MAC(bit)                                                                               \
        /* PSI Interrupt Summary */                                                                     \
        {bit, GT_FALSE,PRV_CPSS_DXCH_UNIT_TXQ_PSI_E, NULL,                                              \
            0x00000020c, 0x00000210,                                                                    \
            prvCpssDrvHwPpPortGroupIsrRead,                                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
            PRV_CPSS_IRONMAN_PSI_REGS_PSI_INTERRUPT_CAUSE_SCHEDULER_INTERRUPT_E,                        \
            PRV_CPSS_IRONMAN_PSI_REGS_PSI_INTERRUPT_CAUSE_PDQ_ACCESS_MAP_ERROR_E,                       \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define eft_NODE_MAC(bit)                                                                               \
        /* eft  */                                                                                      \
        {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_EGF_EFT_E, NULL,                                             \
            0x000010A0, 0x000010B0,                                                                     \
            prvCpssDrvHwPpPortGroupIsrRead,                                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
            PRV_CPSS_IRONMAN_EGF_EFT_INTERRUPTS_CAUSE_RESERVED_1_E,                                     \
            PRV_CPSS_IRONMAN_EGF_EFT_INTERRUPTS_CAUSE_INC_BUS_IS_TOO_SMALL_INT_E,                       \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define epcl_NODE_MAC(bit)                                                                              \
        /* EPCL  */                                                                                     \
        {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_EPCL_E, NULL,                                                \
            0x00000010, 0x00000014,                                                                     \
            prvCpssDrvHwPpPortGroupIsrRead,                                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
            PRV_CPSS_IRONMAN_EPCL_EPCL_INTERRUPT_CAUSE_REGFILE_ADDRESS_ERROR_E,                         \
            PRV_CPSS_IRONMAN_EPCL_EPCL_INTERRUPT_CAUSE_TABLE_OVERLAP_ACCESS_ERROR_E,                    \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

#define eplr_NODE_MAC(bit)                                                                              \
        /* eplr  */                                                                                     \
        {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_EPLR_E, NULL,                                                \
            0x00000200, 0x00000204,                                                                     \
            prvCpssDrvHwPpPortGroupIsrRead,                                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
            PRV_CPSS_IRONMAN_EPLR_POLICER_INTERRUPT_CAUSE_ILLEGAL_OVERLAPPED_ACCESS_E,                  \
            PRV_CPSS_IRONMAN_EPLR_POLICER_INTERRUPT_CAUSE_QOS_ATTRIBUTES_TABLE_WRONG_ADDRESS_INTERRUPT_E, \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

#define ermrk_NODE_MAC(bit)                                                                             \
    /* ERMRK  */                                                                                        \
        {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_ERMRK_E, NULL,                                               \
            0x00000004, 0x00000008,                                                                     \
            prvCpssDrvHwPpPortGroupIsrRead,                                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
            PRV_CPSS_IRONMAN_ERMRK_ERMRK_INTERRUPT_CAUSE_REGFILE_ADDRESS_ERROR_E,                       \
            PRV_CPSS_IRONMAN_ERMRK_ERMRK_INTERRUPT_CAUSE_TABLE_OVERLAP_ACCESS_ERROR_E,                  \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define l2i_NODE_MAC(bit)                                                                               \
        /* L2 Bridge  */                                                                                \
        {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_L2I_E,   NULL,                                               \
            0x00002100, 0x00002104,                                                                     \
            prvCpssDrvHwPpPortGroupIsrRead,                                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
            PRV_CPSS_IRONMAN_L2I_IP_BRIDGE_INTERRUPT_CAUSE_ADDRESS_OUT_OF_RANGE_E,                      \
            PRV_CPSS_IRONMAN_L2I_IP_BRIDGE_INTERRUPT_CAUSE_UPDATE_SECURITY_BREACH_REGISTER_INT_E, \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

/* node for ingress SCT rate limiters interrupts leaf */
#define SCT_RATE_LIMITERS_NODE_FOR_16_PORTS_MAC(index)                                                  \
        {(index+1), GT_FALSE, PRV_CPSS_DXCH_UNIT_EQ_E, NULL,                                            \
            0x00010020 + ((index) * 0x10), 0x00010024 + ((index) * 0x10),                               \
            prvCpssDrvHwPpPortGroupIsrRead,                                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
            PRV_CPSS_IRONMAN_EQ_REG_1_PORT_0_PKT_DROPED_INT_E   + ((index) * 32),                       \
            PRV_CPSS_IRONMAN_EQ_REG_1_PORT_15_PKT_DROPED_INT_E  + ((index) * 32),                       \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define eqSctRateLimiters_SUB_TREE_MAC(bit)                                                             \
        /* SCT Rate Limiters */                                                                         \
        {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_EQ_E, NULL,                                                  \
            0x00010000, 0x00010004,                                                                     \
            prvCpssDrvHwPpPortGroupIsrRead,                                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
            PRV_CPSS_IRONMAN_EQ_CPU_CODE_RATE_LIMITERS_INTERRUPT_SUMMARY_CAUSE_REG_1_CPU_CODE_RATE_LIMITER_INT_E,         \
            PRV_CPSS_IRONMAN_EQ_CPU_CODE_RATE_LIMITERS_INTERRUPT_SUMMARY_CAUSE_REG_16_CPU_CODE_RATE_LIMITER_INT_E,        \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 16, NULL, NULL},                                    \
                SCT_RATE_LIMITERS_NODE_FOR_16_PORTS_MAC(0),                                             \
                SCT_RATE_LIMITERS_NODE_FOR_16_PORTS_MAC(1),                                             \
                SCT_RATE_LIMITERS_NODE_FOR_16_PORTS_MAC(2),                                             \
                SCT_RATE_LIMITERS_NODE_FOR_16_PORTS_MAC(3),                                             \
                SCT_RATE_LIMITERS_NODE_FOR_16_PORTS_MAC(4),                                             \
                SCT_RATE_LIMITERS_NODE_FOR_16_PORTS_MAC(5),                                             \
                SCT_RATE_LIMITERS_NODE_FOR_16_PORTS_MAC(6),                                             \
                SCT_RATE_LIMITERS_NODE_FOR_16_PORTS_MAC(7),                                             \
                SCT_RATE_LIMITERS_NODE_FOR_16_PORTS_MAC(8),                                             \
                SCT_RATE_LIMITERS_NODE_FOR_16_PORTS_MAC(9),                                             \
                SCT_RATE_LIMITERS_NODE_FOR_16_PORTS_MAC(10),                                            \
                SCT_RATE_LIMITERS_NODE_FOR_16_PORTS_MAC(11),                                            \
                SCT_RATE_LIMITERS_NODE_FOR_16_PORTS_MAC(12),                                            \
                SCT_RATE_LIMITERS_NODE_FOR_16_PORTS_MAC(13),                                            \
                SCT_RATE_LIMITERS_NODE_FOR_16_PORTS_MAC(14),                                            \
                SCT_RATE_LIMITERS_NODE_FOR_16_PORTS_MAC(15)

#define eq_SUB_TREE_MAC(bit)                                                                            \
        /* Pre-Egress Interrupt Summary Cause */                                                        \
        {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_EQ_E, NULL,                                                  \
            0x00000058, 0x0000005C,                                                                     \
            prvCpssDrvHwPpPortGroupIsrRead,                                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
            PRV_CPSS_IRONMAN_EQ_PRE_EGRESS_INTERRUPT_SUMMARY_CAUSE_INGRESS_STC_INT_E,                   \
            PRV_CPSS_IRONMAN_EQ_PRE_EGRESS_INTERRUPT_SUMMARY_CAUSE_PORT_DLB_INTERRUPT_E,                \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 1, NULL, NULL},                                     \
                eqSctRateLimiters_SUB_TREE_MAC(2)

#define  ha_NODE_MAC(bit)                                                                               \
        /* HA Rate Limiters */                                                                          \
        {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_HA_E, NULL,                                                  \
            0x00000300, 0x00000304,                                                                     \
            prvCpssDrvHwPpPortGroupIsrRead,                                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
            PRV_CPSS_IRONMAN_HA_HA_INTERRUPT_CAUSE_TABLE_OVERLAP_ACCESS_ERROR_E,                        \
            PRV_CPSS_IRONMAN_HA_HA_INTERRUPT_CAUSE_OVERSIZE_TUNNEL_HEADER_E,                            \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define ioam_NODE_MAC(bit)                                                                              \
        /* ingress OAM  */                                                                              \
        {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_IOAM_E, NULL,                                                \
            0x000000F0, 0x000000F4,                                                                     \
            prvCpssDrvHwPpPortGroupIsrRead,                                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
            PRV_CPSS_IRONMAN_IOAM_OAM_UNIT_INTERRUPT_CAUSE_KEEP_ALIVE_AGING_E,                          \
            PRV_CPSS_IRONMAN_IOAM_OAM_UNIT_INTERRUPT_CAUSE_TX_PERIOD_E,                                 \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define iplr0_NODE_MAC(bit)                                                                             \
        /* iplr0 */                                                                                     \
        {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_IPLR_E, NULL,                                                \
            0x00000200, 0x00000204,                                                                     \
            prvCpssDrvHwPpPortGroupIsrRead,                                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
            PRV_CPSS_IRONMAN_IPLR0_POLICER_INTERRUPT_CAUSE_ILLEGAL_OVERLAPPED_ACCESS_E,                     \
            PRV_CPSS_IRONMAN_IPLR0_POLICER_INTERRUPT_CAUSE_QOS_ATTRIBUTES_TABLE_WRONG_ADDRESS_INTERRUPT_E,  \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define iplr1_NODE_MAC(bit)                                                                             \
        /* iplr1 */                                                                                     \
        {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_IPLR_1_E, NULL,                                              \
            0x00000200, 0x00000204,                                                                     \
            prvCpssDrvHwPpPortGroupIsrRead,                                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
            PRV_CPSS_IRONMAN_IPLR1_POLICER_INTERRUPT_CAUSE_ILLEGAL_OVERLAPPED_ACCESS_E,                 \
            PRV_CPSS_IRONMAN_IPLR1_POLICER_INTERRUPT_CAUSE_QOS_ATTRIBUTES_TABLE_WRONG_ADDRESS_INTERRUPT_E,  \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define router_NODE_MAC(bit)                                                                            \
        /* router */                                                                                    \
        {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_IPVX_E, NULL,                                                \
            0x00000970, 0x00000974,                                                                     \
            prvCpssDrvHwPpPortGroupIsrRead,                                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
            PRV_CPSS_IRONMAN_IPVX_ROUTER_INTERRUPT_CAUSE_I_PV_X_BAD_ADDR_E,                             \
            PRV_CPSS_IRONMAN_IPVX_ROUTER_INTERRUPT_CAUSE_ILLEGAL_OVERLAP_ACCESS_E,                      \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define lpm_NODE_MAC(bit)                                                                               \
        /* LPM */                                                                                       \
        {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_LPM_E, NULL,                                                 \
            0x00F00120, 0x00F00130,                                                                     \
            prvCpssDrvHwPpPortGroupIsrRead,                                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
            PRV_CPSS_IRONMAN_LPM_LPM_GENERAL_INT_CAUSE_LPM_WRONG_ADDRESS_E,                             \
            PRV_CPSS_IRONMAN_LPM_LPM_GENERAL_INT_CAUSE_TABLE_OVERLAPING_E,                              \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define mll_NODE_MAC(bit)                                                                               \
        /* MLL */                                                                                       \
        {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_MLL_E, NULL,                                                 \
            0x00000030, 0x00000034,                                                                     \
            prvCpssDrvHwPpPortGroupIsrRead,                                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
            PRV_CPSS_IRONMAN_MLL_MLL_INTERRUPT_CAUSE_INVALID_IP_MLL_ACCESS_INTERRUPT_E,                 \
            PRV_CPSS_IRONMAN_MLL_MLL_INTERRUPT_CAUSE_ILLEGAL_TABLE_OVERLAP_E,                           \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define pcl_NODE_MAC(bit)                                                                               \
        /* PCL */                                                                                       \
        {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCL_E, NULL,                                                 \
            0x00000004, 0x00000008,                                                                     \
            prvCpssDrvHwPpPortGroupIsrRead,                                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
            PRV_CPSS_IRONMAN_PCL_PCL_UNIT_INTERRUPT_CAUSE_MG_ADDR_OUTOF_RANGE_E,                        \
            PRV_CPSS_IRONMAN_PCL_PCL_UNIT_INTERRUPT_CAUSE_PCL_ILLEGAL_TABLE_OVERLAP_E,                  \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define preq_NODE_MAC(bit)                                                                              \
        /* PREQ */                                                                                      \
        {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PREQ_E, NULL,                                                \
            0x00000600, 0x00000604,                                                                     \
            prvCpssDrvHwPpPortGroupIsrRead,                                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
            PRV_CPSS_IRONMAN_PREQ_IP_PREQ_INTERRUPT_CAUSE_PREQ_REGFILE_ADDRESS_ERROR_E,                 \
            PRV_CPSS_IRONMAN_PREQ_IP_PREQ_INTERRUPT_CAUSE_PREQ_LATENT_ERROR_DETECTED_E,                 \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define qag_NODE_MAC(bit)                                                                               \
        /* qag */                                                                                       \
        {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_EGF_QAG_E, NULL,                                             \
            0x000B0000, 0x000B0010,                                                                     \
            prvCpssDrvHwPpPortGroupIsrRead,                                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
            PRV_CPSS_IRONMAN_EGF_QAG_INTERRUPTS_CAUSE_WRONG_ADDRESS_INTERRUPT_E,                        \
            PRV_CPSS_IRONMAN_EGF_QAG_INTERRUPTS_CAUSE_TABLE_OVERLAPING_INTERRUPT_E,                     \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define sht_NODE_MAC(bit)                                                                               \
        /* SHT */                                                                                       \
        {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_EGF_SHT_E, NULL,                                             \
            0x00100010, 0x00100020,                                                                     \
            prvCpssDrvHwPpPortGroupIsrRead,                                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
            PRV_CPSS_IRONMAN_EGF_SHT_INTERRUPTS_CAUSE_WRONG_ADDRESS_INTERRUPT_E,                        \
            PRV_CPSS_IRONMAN_EGF_SHT_INTERRUPTS_CAUSE_TABLE_OVERLAPPING_INTERRUPT_E,                    \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define tti_NODE_MAC(bit)                                                                               \
        /* TTI */                                                                                       \
        {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_TTI_E, NULL,                                                 \
            0x00000004, 0x00000008,                                                                     \
            prvCpssDrvHwPpPortGroupIsrRead,                                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
            PRV_CPSS_IRONMAN_TTI_TTI_ENGINE_INTERRUPT_CAUSE_CPU_ADDRESS_OUT_OF_RANGE_E,                 \
            PRV_CPSS_IRONMAN_TTI_TTI_ENGINE_INTERRUPT_CAUSE_TABLE_OVERLAP_ERROR_E,                      \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define hbu_NODE_MAC(bit)                                                                               \
        /* HBU */                                                                                       \
        {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_HBU_E, NULL,                                                 \
            0x00000000, 0x00000004,                                                                     \
            prvCpssDrvHwPpPortGroupIsrRead,                                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
            PRV_CPSS_IRONMAN_HBU_HBU_INTERRUPT_CAUSE_ERROR_FHF_MEM_E,                                   \
            PRV_CPSS_IRONMAN_HBU_HBU_INTERRUPT_CAUSE_HEADER_FIFO_EMPTY_E,                               \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}


#define erep_NODE_MAC(bit)                                                                              \
        /* EREP Rate Limiters */                                                                        \
        {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_EREP_E, NULL,                                                \
            0x00003000, 0x00003004,                                                                     \
            prvCpssDrvHwPpPortGroupIsrRead,                                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
            PRV_CPSS_IRONMAN_EREP_IP_EREP_INTERRUPTS_CAUSE_EREP_CPU_ADDRESS_OUT_OF_RANGE_E,             \
            PRV_CPSS_IRONMAN_EREP_IP_EREP_INTERRUPTS_CAUSE_MIRROR_REPLICATION_NOT_PERFORMED_E,          \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define tai_NODE_MAC(bit, index)                                                                        \
        {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_TAI_E + index, NULL,                                         \
            0x00000000, 0x00000004,                                                                     \
            prvCpssDrvHwPpPortGroupIsrRead,                                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
            PRV_CPSS_IRONMAN_TAI_0_TAI_INTERRUPT_CAUSE_GENERATION_INT_E + (index * 32),                 \
            PRV_CPSS_IRONMAN_TAI_0_TAI_INTERRUPT_CAUSE_FSM_STATE_DETECTED_INT_E + (index * 32),         \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define gop_mif_usx_SUB_TREE_MAC(bit, _index)                                                           \
        /* MIF */                                                                                       \
        {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_MIF_USX_##_index##_E, NULL,                                  \
            0x00000010, 0x00000014,                                                                     \
            prvCpssDrvHwPpPortGroupIsrRead,                                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
            PRV_CPSS_IRONMAN_MIF_##_index##_MIF_GLOBAL_INTERRUPT_SUMMARY_CAUSE_MIF_GLOBAL_CHANNEL_INTERRUPT_E,       \
            PRV_CPSS_IRONMAN_MIF_##_index##_MIF_GLOBAL_INTERRUPT_SUMMARY_CAUSE_MIF_GLOBAL_BAD_ADDRESS_INTERRUPT_E,   \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define gop_led_NODE_MAC(bit, _index)                                                                   \
        /* LED */                                                                                       \
        {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_LED_##_index##_E, NULL,                                      \
            0x00000200, 0x00000204,                                                                     \
            prvCpssDrvHwPpPortGroupIsrRead,                                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
            PRV_CPSS_IRONMAN_LED_##_index##_LED_INTERRUPT_CAUSE_ILLEGAL_ADDRESS_ACCESS_E,               \
            PRV_CPSS_IRONMAN_LED_##_index##_LED_INTERRUPT_CAUSE_ILLEGAL_ADDRESS_ACCESS_E,               \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define cpu_led_NODE_MAC(bit, _index)                                                                   \
        /* CPU LED */                                                                                   \
        {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_LED_##_index##_E, NULL,                                      \
            0x00000200, 0x00000204,                                                                     \
            prvCpssDrvHwPpPortGroupIsrRead,                                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
            PRV_CPSS_IRONMAN_LED_CPU_3_LED_INTERRUPT_CAUSE_ILLEGAL_ADDRESS_ACCESS_E,                    \
            PRV_CPSS_IRONMAN_LED_CPU_3_LED_INTERRUPT_CAUSE_ILLEGAL_ADDRESS_ACCESS_E,                    \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define port_mti_10G_pcs_x8_0_NODE_MAC(bit, port)                                                       \
        {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_MTI_10G_PCS_x8_0_E, NULL,                                    \
            PORT_MTI_10G_PCS_X8_MAC_INTERRUPT_CAUSE_MAC(port),                                          \
            PORT_MTI_10G_PCS_X8_MAC_INTERRUPT_MASK_MAC(port),                                           \
            prvCpssDrvHwPpPortGroupIsrRead,                                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
            PRV_CPSS_IRONMAN_INST_0_0_PCS8_0_TO_1_0_PORT_NUM_##port##_LINK_STATUS_CHANGE_E,             \
            PRV_CPSS_IRONMAN_INST_0_0_PCS8_0_TO_1_0_PORT_NUM_##port##_LPCS_AN_DONE_INT_E,               \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define port_mti_10G_pcs_x8_1_NODE_MAC(bit, port)                                                       \
        {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_MTI_10G_PCS_x8_1_E, NULL,                                    \
            PORT_MTI_10G_PCS_X8_MAC_INTERRUPT_CAUSE_MAC(port),                                          \
            PORT_MTI_10G_PCS_X8_MAC_INTERRUPT_MASK_MAC(port),                                           \
            prvCpssDrvHwPpPortGroupIsrRead,                                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
            PRV_CPSS_IRONMAN_INST_0_0_PCS8_0_TO_1_1_PORT_NUM_##port##_LINK_STATUS_CHANGE_E,             \
            PRV_CPSS_IRONMAN_INST_0_0_PCS8_0_TO_1_1_PORT_NUM_##port##_LPCS_AN_DONE_INT_E,               \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define port_mti_10G_pcs_x8_2_NODE_MAC(bit, port)                                                       \
        {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_MTI_10G_PCS_x8_2_E, NULL,                                    \
            PORT_MTI_10G_PCS_X8_MAC_INTERRUPT_CAUSE_MAC(port),                                          \
            PORT_MTI_10G_PCS_X8_MAC_INTERRUPT_MASK_MAC(port),                                           \
            prvCpssDrvHwPpPortGroupIsrRead,                                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
            PRV_CPSS_IRONMAN_INST_1_1_PCS8_0_TO_1_0_PORT_NUM_##port##_LINK_STATUS_CHANGE_E,             \
            PRV_CPSS_IRONMAN_INST_1_1_PCS8_0_TO_1_0_PORT_NUM_##port##_LPCS_AN_DONE_INT_E,               \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define port_mti_10G_pcs_x8_3_NODE_MAC(bit, port)                                                       \
        {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_MTI_10G_PCS_x8_3_E, NULL,                                    \
            PORT_MTI_10G_PCS_X8_MAC_INTERRUPT_CAUSE_MAC(port),                                          \
            PORT_MTI_10G_PCS_X8_MAC_INTERRUPT_MASK_MAC(port),                                           \
            prvCpssDrvHwPpPortGroupIsrRead,                                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
            PRV_CPSS_IRONMAN_INST_1_1_PCS8_0_TO_1_1_PORT_NUM_##port##_LINK_STATUS_CHANGE_E,             \
            PRV_CPSS_IRONMAN_INST_1_1_PCS8_0_TO_1_1_PORT_NUM_##port##_LPCS_AN_DONE_INT_E,               \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define port_mti_10G_pcs_x8_4_NODE_MAC(bit, port)                                                       \
        {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_MTI_10G_PCS_x8_4_E, NULL,                                    \
            PORT_MTI_10G_PCS_X8_MAC_INTERRUPT_CAUSE_MAC(port),                                          \
            PORT_MTI_10G_PCS_X8_MAC_INTERRUPT_MASK_MAC(port),                                           \
            prvCpssDrvHwPpPortGroupIsrRead,                                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
            PRV_CPSS_IRONMAN_INST_2_2_PCS8_0_TO_1_0_PORT_NUM_##port##_LINK_STATUS_CHANGE_E,             \
            PRV_CPSS_IRONMAN_INST_2_2_PCS8_0_TO_1_0_PORT_NUM_##port##_LPCS_AN_DONE_INT_E,               \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define port_mti_10G_pcs_x8_5_NODE_MAC(bit, port)                                                       \
        {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_MTI_10G_PCS_x8_5_E, NULL,                                    \
            PORT_MTI_10G_PCS_X8_MAC_INTERRUPT_CAUSE_MAC(port),                                          \
            PORT_MTI_10G_PCS_X8_MAC_INTERRUPT_MASK_MAC(port),                                           \
            prvCpssDrvHwPpPortGroupIsrRead,                                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
            PRV_CPSS_IRONMAN_INST_2_2_PCS8_0_TO_1_1_PORT_NUM_##port##_LINK_STATUS_CHANGE_E,             \
            PRV_CPSS_IRONMAN_INST_2_2_PCS8_0_TO_1_1_PORT_NUM_##port##_LPCS_AN_DONE_INT_E,               \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define gop_mti_10G_pcs_x8_0_SUB_TREE_MAC(bit)                                                          \
    /*  MTI 10G x8 PCS */                                                                               \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_MTI_10G_PCS_x8_0_E, NULL,                                        \
        0x00000014, 0x00000018,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_IRONMAN_INST_0_0_PCS8_0_TO_1_0_GLOBAL_INTERRUPT_SUMMARY_CAUSE_PORT_NUM_0_P_INT_SUM_E,  \
        PRV_CPSS_IRONMAN_INST_0_0_PCS8_0_TO_1_0_GLOBAL_INTERRUPT_SUMMARY_CAUSE_PORT_NUM_7_P_INT_SUM_E,  \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 8, NULL, NULL},                                         \
            port_mti_10G_pcs_x8_0_NODE_MAC(1,   0),                                                     \
            port_mti_10G_pcs_x8_0_NODE_MAC(2,   1),                                                     \
            port_mti_10G_pcs_x8_0_NODE_MAC(3,   2),                                                     \
            port_mti_10G_pcs_x8_0_NODE_MAC(4,   3),                                                     \
            port_mti_10G_pcs_x8_0_NODE_MAC(5,   4),                                                     \
            port_mti_10G_pcs_x8_0_NODE_MAC(6,   5),                                                     \
            port_mti_10G_pcs_x8_0_NODE_MAC(7,   6),                                                     \
            port_mti_10G_pcs_x8_0_NODE_MAC(8,   7)

#define gop_mti_10G_pcs_x8_1_SUB_TREE_MAC(bit)                                                          \
    /*  MTI 10G x8 PCS */                                                                               \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_MTI_10G_PCS_x8_1_E, NULL,                                        \
        0x00000014, 0x00000018,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_IRONMAN_INST_0_0_PCS8_0_TO_1_1_GLOBAL_INTERRUPT_SUMMARY_CAUSE_PORT_NUM_0_P_INT_SUM_E,  \
        PRV_CPSS_IRONMAN_INST_0_0_PCS8_0_TO_1_1_GLOBAL_INTERRUPT_SUMMARY_CAUSE_PORT_NUM_7_P_INT_SUM_E,  \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 8, NULL, NULL},                                         \
            port_mti_10G_pcs_x8_1_NODE_MAC(1,   0),                                                     \
            port_mti_10G_pcs_x8_1_NODE_MAC(2,   1),                                                     \
            port_mti_10G_pcs_x8_1_NODE_MAC(3,   2),                                                     \
            port_mti_10G_pcs_x8_1_NODE_MAC(4,   3),                                                     \
            port_mti_10G_pcs_x8_1_NODE_MAC(5,   4),                                                     \
            port_mti_10G_pcs_x8_1_NODE_MAC(6,   5),                                                     \
            port_mti_10G_pcs_x8_1_NODE_MAC(7,   6),                                                     \
            port_mti_10G_pcs_x8_1_NODE_MAC(8,   7)

#define gop_mti_10G_pcs_x8_2_SUB_TREE_MAC(bit)                                                          \
    /*  MTI 10G x8 PCS */                                                                               \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_MTI_10G_PCS_x8_2_E, NULL,                                        \
        0x00000014, 0x00000018,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_IRONMAN_INST_1_1_PCS8_0_TO_1_0_GLOBAL_INTERRUPT_SUMMARY_CAUSE_PORT_NUM_0_P_INT_SUM_E,  \
        PRV_CPSS_IRONMAN_INST_1_1_PCS8_0_TO_1_0_GLOBAL_INTERRUPT_SUMMARY_CAUSE_PORT_NUM_7_P_INT_SUM_E,  \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 8, NULL, NULL},                                         \
            port_mti_10G_pcs_x8_2_NODE_MAC(1,   0),                                                     \
            port_mti_10G_pcs_x8_2_NODE_MAC(2,   1),                                                     \
            port_mti_10G_pcs_x8_2_NODE_MAC(3,   2),                                                     \
            port_mti_10G_pcs_x8_2_NODE_MAC(4,   3),                                                     \
            port_mti_10G_pcs_x8_2_NODE_MAC(5,   4),                                                     \
            port_mti_10G_pcs_x8_2_NODE_MAC(6,   5),                                                     \
            port_mti_10G_pcs_x8_2_NODE_MAC(7,   6),                                                     \
            port_mti_10G_pcs_x8_2_NODE_MAC(8,   7)

#define gop_mti_10G_pcs_x8_3_SUB_TREE_MAC(bit)                                                          \
    /*  MTI 10G x8 PCS */                                                                               \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_MTI_10G_PCS_x8_3_E, NULL,                                        \
        0x00000014, 0x00000018,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_IRONMAN_INST_1_1_PCS8_0_TO_1_1_GLOBAL_INTERRUPT_SUMMARY_CAUSE_PORT_NUM_0_P_INT_SUM_E,  \
        PRV_CPSS_IRONMAN_INST_1_1_PCS8_0_TO_1_1_GLOBAL_INTERRUPT_SUMMARY_CAUSE_PORT_NUM_7_P_INT_SUM_E,  \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 8, NULL, NULL},                                         \
            port_mti_10G_pcs_x8_3_NODE_MAC(1,   0),                                                     \
            port_mti_10G_pcs_x8_3_NODE_MAC(2,   1),                                                     \
            port_mti_10G_pcs_x8_3_NODE_MAC(3,   2),                                                     \
            port_mti_10G_pcs_x8_3_NODE_MAC(4,   3),                                                     \
            port_mti_10G_pcs_x8_3_NODE_MAC(5,   4),                                                     \
            port_mti_10G_pcs_x8_3_NODE_MAC(6,   5),                                                     \
            port_mti_10G_pcs_x8_3_NODE_MAC(7,   6),                                                     \
            port_mti_10G_pcs_x8_3_NODE_MAC(8,   7)

#define gop_mti_10G_pcs_x8_4_SUB_TREE_MAC(bit)                                                          \
    /*  MTI 10G x8 PCS */                                                                               \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_MTI_10G_PCS_x8_4_E, NULL,                                        \
        0x00000014, 0x00000018,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_IRONMAN_INST_2_2_PCS8_0_TO_1_0_GLOBAL_INTERRUPT_SUMMARY_CAUSE_PORT_NUM_0_P_INT_SUM_E,  \
        PRV_CPSS_IRONMAN_INST_2_2_PCS8_0_TO_1_0_GLOBAL_INTERRUPT_SUMMARY_CAUSE_PORT_NUM_7_P_INT_SUM_E,  \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 8, NULL, NULL},                                         \
            port_mti_10G_pcs_x8_4_NODE_MAC(1,   0),                                                     \
            port_mti_10G_pcs_x8_4_NODE_MAC(2,   1),                                                     \
            port_mti_10G_pcs_x8_4_NODE_MAC(3,   2),                                                     \
            port_mti_10G_pcs_x8_4_NODE_MAC(4,   3),                                                     \
            port_mti_10G_pcs_x8_4_NODE_MAC(5,   4),                                                     \
            port_mti_10G_pcs_x8_4_NODE_MAC(6,   5),                                                     \
            port_mti_10G_pcs_x8_4_NODE_MAC(7,   6),                                                     \
            port_mti_10G_pcs_x8_4_NODE_MAC(8,   7)

#define gop_mti_10G_pcs_x8_5_SUB_TREE_MAC(bit)                                                          \
    /*  MTI 10G x8 PCS */                                                                               \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_MTI_10G_PCS_x8_5_E, NULL,                                        \
        0x00000014, 0x00000018,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_IRONMAN_INST_2_2_PCS8_0_TO_1_1_GLOBAL_INTERRUPT_SUMMARY_CAUSE_PORT_NUM_0_P_INT_SUM_E,  \
        PRV_CPSS_IRONMAN_INST_2_2_PCS8_0_TO_1_1_GLOBAL_INTERRUPT_SUMMARY_CAUSE_PORT_NUM_7_P_INT_SUM_E,  \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 8, NULL, NULL},                                         \
            port_mti_10G_pcs_x8_5_NODE_MAC(1,   0),                                                     \
            port_mti_10G_pcs_x8_5_NODE_MAC(2,   1),                                                     \
            port_mti_10G_pcs_x8_5_NODE_MAC(3,   2),                                                     \
            port_mti_10G_pcs_x8_5_NODE_MAC(4,   3),                                                     \
            port_mti_10G_pcs_x8_5_NODE_MAC(5,   4),                                                     \
            port_mti_10G_pcs_x8_5_NODE_MAC(6,   5),                                                     \
            port_mti_10G_pcs_x8_5_NODE_MAC(7,   6),                                                     \
            port_mti_10G_pcs_x8_5_NODE_MAC(8,   7)

#define global_port_anp_NODE_MAC(bit, unit)                                                             \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_ANP_CPU_4_E + unit, NULL,                                        \
        0x00000088, 0x0000008C,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_IRONMAN_INST_##unit##_##unit##_ANP_4_GLOBAL_INTERRUPT_CAUSE_ILLEGAL_ADDRESS_ACCESS_E,  \
        PRV_CPSS_IRONMAN_INST_##unit##_##unit##_ANP_4_GLOBAL_INTERRUPT_CAUSE_ILLEGAL_ADDRESS_ACCESS_E,  \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define global_port_anp_0_NODE_MAC(bit, unit, index)                                                    \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_ANP_USX_0_E + unit, NULL,                                        \
        0x00000088, 0x0000008C,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_IRONMAN_INST_0_0_ANP_##index##_GLOBAL_INTERRUPT_CAUSE_ILLEGAL_ADDRESS_ACCESS_E,        \
        PRV_CPSS_IRONMAN_INST_0_0_ANP_##index##_GLOBAL_INTERRUPT_CAUSE_ILLEGAL_ADDRESS_ACCESS_E,        \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define global_port_anp_1_NODE_MAC(bit, unit, index)                                                    \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_ANP_USX_0_E + unit, NULL,                                        \
        0x00000088, 0x0000008C,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_IRONMAN_INST_1_1_ANP_##index##_GLOBAL_INTERRUPT_CAUSE_ILLEGAL_ADDRESS_ACCESS_E,        \
        PRV_CPSS_IRONMAN_INST_1_1_ANP_##index##_GLOBAL_INTERRUPT_CAUSE_ILLEGAL_ADDRESS_ACCESS_E,        \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define global_port_anp_2_NODE_MAC(bit, unit, index)                                                    \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_ANP_USX_0_E + unit, NULL,                                        \
        0x00000088, 0x0000008C,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_IRONMAN_INST_2_2_ANP_##index##_GLOBAL_INTERRUPT_CAUSE_ILLEGAL_ADDRESS_ACCESS_E,        \
        PRV_CPSS_IRONMAN_INST_2_2_ANP_##index##_GLOBAL_INTERRUPT_CAUSE_ILLEGAL_ADDRESS_ACCESS_E,        \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define global_cpu_anp_NODE_MAC(bit, index)                                                             \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_ANP_CPU_0_E + index, NULL,                                       \
        0x00000088, 0x0000008C,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_IRONMAN_CPU_ANP_##index##_CPU_5_GLOBAL_INTERRUPT_CAUSE_ILLEGAL_ADDRESS_ACCESS_E,       \
        PRV_CPSS_IRONMAN_CPU_ANP_##index##_CPU_5_GLOBAL_INTERRUPT_CAUSE_ILLEGAL_ADDRESS_ACCESS_E,       \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define port_usx_anp_0_NODE_MAC(bit, unit, index)                                                       \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_ANP_USX_0_E + unit, NULL,                                        \
        0x00000100, 0x00000104,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_IRONMAN_INST_0_0_ANP_##index##_PORT_NUM_0_AN_RESTART_E,                                \
        PRV_CPSS_IRONMAN_INST_0_0_ANP_##index##_PORT_NUM_0_INT_WAIT_PWRDN_TIME_OUT_E,                   \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define port_usx_anp_1_NODE_MAC(bit, unit, index)                                                       \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_ANP_USX_0_E + unit, NULL,                                        \
        0x00000100, 0x00000104,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_IRONMAN_INST_1_1_ANP_##index##_PORT_NUM_0_AN_RESTART_E,                                \
        PRV_CPSS_IRONMAN_INST_1_1_ANP_##index##_PORT_NUM_0_INT_WAIT_PWRDN_TIME_OUT_E,                   \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define port_usx_anp_2_NODE_MAC(bit, unit, index)                                                       \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_ANP_USX_0_E + unit, NULL,                                        \
        0x00000100, 0x00000104,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_IRONMAN_INST_2_2_ANP_##index##_PORT_NUM_0_AN_RESTART_E,                                \
        PRV_CPSS_IRONMAN_INST_2_2_ANP_##index##_PORT_NUM_0_INT_WAIT_PWRDN_TIME_OUT_E,                   \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define port_usx_anp_NODE_MAC(bit, unit)                                                                \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_ANP_CPU_4_E + unit, NULL,                                        \
        0x00000100, 0x00000104,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_IRONMAN_INST_##unit##_##unit##_ANP_4_PORT_NUM_0_AN_RESTART_E,                          \
        PRV_CPSS_IRONMAN_INST_##unit##_##unit##_ANP_4_PORT_NUM_0_INT_WAIT_PWRDN_TIME_OUT_E,             \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define port_cpu_anp_NODE_MAC(bit, index)                                                               \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_ANP_CPU_0_E + index, NULL,                                       \
        0x00000100, 0x00000104,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_IRONMAN_CPU_ANP_##index##_CPU_5_PORT_NUM_0_AN_RESTART_E,                               \
        PRV_CPSS_IRONMAN_CPU_ANP_##index##_CPU_5_PORT_NUM_0_INT_WAIT_PWRDN_TIME_OUT_E,                  \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define gop_anp_0_summary_SUB_TREE_MAC(bit, unit, index)                                                \
    /* ANP0 */                                                                                          \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_ANP_USX_0_E + unit, NULL,                                        \
        0x00000094, 0x00000098,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_IRONMAN_INST_0_0_ANP_##index##_INTERRUPT_SUMMARY_CAUSE_GLOBAL_INT_E,                   \
        PRV_CPSS_IRONMAN_INST_0_0_ANP_##index##_INTERRUPT_SUMMARY_CAUSE_PORT_NUM_0_P_INT2_SUM_E,        \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 2, NULL, NULL},                                         \
            global_port_anp_0_NODE_MAC(1, unit, index),                                                 \
            port_usx_anp_0_NODE_MAC(2, unit, index)

#define gop_anp_1_summary_SUB_TREE_MAC(bit, unit, index)                                                \
    /* ANP1 */                                                                                          \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_ANP_USX_0_E + unit, NULL,                                        \
        0x00000094, 0x00000098,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_IRONMAN_INST_1_1_ANP_##index##_INTERRUPT_SUMMARY_CAUSE_GLOBAL_INT_E,                   \
        PRV_CPSS_IRONMAN_INST_1_1_ANP_##index##_INTERRUPT_SUMMARY_CAUSE_PORT_NUM_0_P_INT2_SUM_E,        \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 2, NULL, NULL},                                         \
            global_port_anp_1_NODE_MAC(1, unit, index),                                                 \
            port_usx_anp_1_NODE_MAC(2, unit, index)

#define gop_anp_2_summary_SUB_TREE_MAC(bit, unit, index)                                                \
    /* ANP2 */                                                                                          \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_ANP_USX_0_E + unit, NULL,                                        \
        0x00000094, 0x00000098,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_IRONMAN_INST_2_2_ANP_##index##_INTERRUPT_SUMMARY_CAUSE_GLOBAL_INT_E,                   \
        PRV_CPSS_IRONMAN_INST_2_2_ANP_##index##_INTERRUPT_SUMMARY_CAUSE_PORT_NUM_0_P_INT2_SUM_E,        \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 2, NULL, NULL},                                         \
            global_port_anp_2_NODE_MAC(1, unit, index),                                                 \
            port_usx_anp_2_NODE_MAC(2, unit, index)

#define gop_anp_summary_SUB_TREE_MAC(bit, unit)                                                         \
    /* ANP1 */                                                                                          \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_ANP_CPU_4_E + unit, NULL,                                        \
        0x00000094, 0x00000098,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_IRONMAN_INST_##unit##_##unit##_ANP_4_INTERRUPT_SUMMARY_CAUSE_GLOBAL_INT_E,             \
        PRV_CPSS_IRONMAN_INST_##unit##_##unit##_ANP_4_INTERRUPT_SUMMARY_CAUSE_PORT_NUM_0_P_INT2_SUM_E,  \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 2, NULL, NULL},                                         \
            global_port_anp_NODE_MAC(1, unit),                                                          \
            port_usx_anp_NODE_MAC(2, unit)

#define gop_anp_cpu_summary_SUB_TREE_MAC(bit, index)                                                    \
    /* ANP1 */                                                                                          \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_ANP_CPU_0_E + index, NULL,                                       \
        0x00000094, 0x00000098,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_IRONMAN_CPU_ANP_##index##_CPU_5_INTERRUPT_SUMMARY_CAUSE_GLOBAL_INT_E,                  \
        PRV_CPSS_IRONMAN_CPU_ANP_##index##_CPU_5_INTERRUPT_SUMMARY_CAUSE_PORT_NUM_0_P_INT2_SUM_E,       \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 2, NULL, NULL},                                         \
            global_cpu_anp_NODE_MAC(1, index),                                                          \
            port_cpu_anp_NODE_MAC(2, index)

#define gop_sdw_interrupt_cause_NODE_MAC(bit, unit)                                                     \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_SDW0_E + unit, NULL,                                             \
        0x00000200, 0x00000204,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_IRONMAN_SDW_INSTANCES_##unit##_SDW_INTERRUPT_CAUSE_SER_DES_INTERRUPT_E,                \
        PRV_CPSS_IRONMAN_SDW_INSTANCES_##unit##_SDW_INTERRUPT_CAUSE_SQ_DETECT_0_E,                      \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define lmu_latency_over_threshold_cause_NODE_MAC(bit, index)                                           \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_LMU_0_E, NULL,                                               \
        (0x00008100 + (0x4 * (index))), (0x00008180 + (0x4 * (index))),                                 \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_IRONMAN_LMU_N_##index##_I_0_LMU_LATENCY_OVER_THRESHOLD_E,                              \
        PRV_CPSS_IRONMAN_LMU_N_##index##_I_30_LMU_LATENCY_OVER_THRESHOLD_E,                             \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define lmu_cause_summary_SUB_TREE_MAC(bit)                                                             \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_LMU_0_E, NULL,                                               \
        0x00008230, 0x00008234,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_IRONMAN_LMU_LMU_CAUSE_SUMMARY_N_0_LMU_LATENCY_OVER_THRESHOLD_SUM_E,                    \
        PRV_CPSS_IRONMAN_LMU_LMU_CAUSE_SUMMARY_N_0_LMU_LATENCY_FIFO_FULL_SUM_E,                         \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 17, NULL, NULL},                                        \
            lmu_latency_over_threshold_cause_NODE_MAC( 1,  0),                                          \
            lmu_latency_over_threshold_cause_NODE_MAC( 2,  1),                                          \
            lmu_latency_over_threshold_cause_NODE_MAC( 3,  2),                                          \
            lmu_latency_over_threshold_cause_NODE_MAC( 4,  3),                                          \
            lmu_latency_over_threshold_cause_NODE_MAC( 5,  4),                                          \
            lmu_latency_over_threshold_cause_NODE_MAC( 6,  5),                                          \
            lmu_latency_over_threshold_cause_NODE_MAC( 7,  6),                                          \
            lmu_latency_over_threshold_cause_NODE_MAC( 8,  7),                                          \
            lmu_latency_over_threshold_cause_NODE_MAC( 9,  8),                                          \
            lmu_latency_over_threshold_cause_NODE_MAC(10,  9),                                          \
            lmu_latency_over_threshold_cause_NODE_MAC(11, 10),                                          \
            lmu_latency_over_threshold_cause_NODE_MAC(12, 11),                                          \
            lmu_latency_over_threshold_cause_NODE_MAC(13, 12),                                          \
            lmu_latency_over_threshold_cause_NODE_MAC(14, 13),                                          \
            lmu_latency_over_threshold_cause_NODE_MAC(15, 14),                                          \
            lmu_latency_over_threshold_cause_NODE_MAC(16, 15),                                          \
            lmu_latency_over_threshold_cause_NODE_MAC(17, 16)

#define pizarb_bad_address_interrupt_cause_NODE_MAC(bit)                                                \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_PZ_ARBITER_I_0_E, NULL,                                      \
        0x00000b04, 0x00000b08,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_IRONMAN_PIZARB_PIZZA_ARBITER_BAD_ADDRESS_INTERRUPT_CAUSE_PZARB_BAD_ADDRESS_INTERRUPT_E,\
        PRV_CPSS_IRONMAN_PIZARB_PIZZA_ARBITER_BAD_ADDRESS_INTERRUPT_CAUSE_PZARB_BAD_ADDRESS_INTERRUPT_E,\
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define  tcam_SUB_TREE_MAC(bit)                                                                         \
    /* TCAM Interrupt Summary */                                                                        \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_TCAM_E, NULL,                                                    \
        0x00501004, 0x00501000,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_IRONMAN_TCAM_TCAM_INTERRUPTS_SUMMARY_CAUSE_TCAM_LOGIC_INTERRUPT_SUMMARY_E,             \
        PRV_CPSS_IRONMAN_TCAM_TCAM_INTERRUPTS_SUMMARY_CAUSE_TCAM_LOGIC_INTERRUPT_SUMMARY_E,             \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 1, NULL, NULL},                                         \
        /* TCAM Interrupt */                                                                            \
        {1, GT_FALSE, PRV_CPSS_DXCH_UNIT_TCAM_E, NULL,                                                  \
            0x0050100C, 0x00501008,                                                                     \
            prvCpssDrvHwPpPortGroupIsrRead,                                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
            PRV_CPSS_IRONMAN_TCAM_TCAM_INTERRUPT_CAUSE_CPU_ADDRESS_OUT_OF_RANGE_E,                      \
            PRV_CPSS_IRONMAN_TCAM_TCAM_INTERRUPT_CAUSE_TABLE_OVERLAPPING_E,                             \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define smb_SUB_TREE_MAC(bit)                                                                           \
    /* SMB-MC */                                                                                        \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PB_SHARED_MEMO_BUF_0_0_E, NULL,                                  \
        0x00000150, 0x00000154,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_IRONMAN_SMB_MC_INTERRUPT_SUM_CAUSE_INTERRUPT_SUM_MISC_E,                               \
        PRV_CPSS_IRONMAN_SMB_MC_INTERRUPT_SUM_CAUSE_INTERRUPT_SUM_FIFO_E,                               \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 6,NULL, NULL},                                          \
                                                                                                        \
        /* MiscInterruptSum  */                                                                         \
        {1, GT_FALSE, PRV_CPSS_DXCH_UNIT_PB_SHARED_MEMO_BUF_0_0_E, NULL,                                \
            0x00000100, 0x00000104,                                                                     \
            prvCpssDrvHwPpPortGroupIsrRead,                                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
            PRV_CPSS_IRONMAN_SMB_MC_INTERRUPT_MISC_CAUSE_INTERRUPT_RBW_NO_TRUST_E,                      \
            PRV_CPSS_IRONMAN_SMB_MC_INTERRUPT_MISC_CAUSE_INTERRUPT_ALIGN_WR_CMDS_FIFOS_CONGESTION_E,    \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},                                      \
        /* AgeInterruptSum  */                                                                          \
        {2, GT_FALSE, PRV_CPSS_DXCH_UNIT_PB_SHARED_MEMO_BUF_0_0_E, NULL,                                \
            0x00000110, 0x00000114,                                                                     \
            prvCpssDrvHwPpPortGroupIsrRead,                                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
            PRV_CPSS_IRONMAN_SMB_MC_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_0_INTERRUPT_AGE_ERR_E,           \
            PRV_CPSS_IRONMAN_SMB_MC_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_7_INTERRUPT_AGE_ERR_E,           \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},                                      \
        /* RbwInterruptSum  */                                                                          \
        {3, GT_FALSE, PRV_CPSS_DXCH_UNIT_PB_SHARED_MEMO_BUF_0_0_E, NULL,                                \
            0x00000120, 0x00000124,                                                                     \
            prvCpssDrvHwPpPortGroupIsrRead,                                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
            PRV_CPSS_IRONMAN_SMB_MC_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_0_INTERRUPT_RBW_ERR_E,           \
            PRV_CPSS_IRONMAN_SMB_MC_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_7_INTERRUPT_RBW_ERR_E,           \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},                                      \
        /* MemoriesInterruptSum  */                                                                     \
        {4, GT_FALSE, PRV_CPSS_DXCH_UNIT_PB_SHARED_MEMO_BUF_0_0_E, NULL,                                \
            0x00000130, 0x00000134,                                                                     \
            prvCpssDrvHwPpPortGroupIsrRead,                                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
            PRV_CPSS_IRONMAN_SMB_MC_INTERRUPT_MEMORIES_CAUSE_LOGICAL_MEMORY_0_PAYLOAD_SER_ERROR_E,      \
            PRV_CPSS_IRONMAN_SMB_MC_INTERRUPT_MEMORIES_CAUSE_LOGICAL_MEMORY_7_PAYLOAD_SER_ERROR_E,      \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},                                      \
        /* Memories2InterruptSum  */                                                                    \
        {5, GT_FALSE, PRV_CPSS_DXCH_UNIT_PB_SHARED_MEMO_BUF_0_0_E, NULL,                                \
            0x00000140, 0x00000144,                                                                     \
            prvCpssDrvHwPpPortGroupIsrRead,                                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
            PRV_CPSS_IRONMAN_SMB_MC_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_0_REFCNT_SER_ERROR_E,      \
            PRV_CPSS_IRONMAN_SMB_MC_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_1_LIST_SER_ERROR_E,        \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL},                                      \
        /* FifoInterruptSum  */                                                                         \
        {6, GT_FALSE, PRV_CPSS_DXCH_UNIT_PB_SHARED_MEMO_BUF_0_0_E, NULL,                                \
            0x00000118, 0x0000011C,                                                                     \
            prvCpssDrvHwPpPortGroupIsrRead,                                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
            PRV_CPSS_IRONMAN_SMB_MC_INTERRUPT_FIFO_CAUSE_LOGICAL_MEMORY_0_INTERRUPT_FIFO_ERR_E,         \
            PRV_CPSS_IRONMAN_SMB_MC_INTERRUPT_FIFO_CAUSE_LOGICAL_MEMORY_7_INTERRUPT_FIFO_ERR_E,         \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}

#define pb_SUB_TREE_MAC(bit)                                                                            \
    /* PB */                                                                                            \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PB_CENTER_BLK_E, NULL,                                           \
        0x00001128, 0x0000112C,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_IRONMAN_PB_CENTER_INTERRUPT_SUMMARY_PACKET_BUFFER_CAUSE_INTERRUPT_SUMMARY_NPM_MEMORY_CLUSTERS_E,\
        PRV_CPSS_IRONMAN_PB_CENTER_INTERRUPT_SUMMARY_PACKET_BUFFER_CAUSE_INTERRUPT_SUMMARY_PB_CENTER_E,          \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 6, NULL, NULL},                                         \
            {1, GT_FALSE, PRV_CPSS_DXCH_UNIT_PB_CENTER_BLK_E, NULL,                                     \
                0x00001108, 0x0000110C,                                                                 \
                prvCpssDrvHwPpPortGroupIsrRead,                                                         \
                prvCpssDrvHwPpPortGroupIsrWrite,                                                        \
                PRV_CPSS_IRONMAN_PB_CENTER_INTERRUPT_SUMMARY_NPM_MEMORY_CLUSTER_CAUSE_INSTANCE_0_INTERRUPT_SUMMARY_NPM_MEMORY_CLUSTER_E,\
                PRV_CPSS_IRONMAN_PB_CENTER_INTERRUPT_SUMMARY_NPM_MEMORY_CLUSTER_CAUSE_INSTANCE_0_INTERRUPT_SUMMARY_NPM_MEMORY_CLUSTER_E,\
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},                                 \
            {2, GT_FALSE, PRV_CPSS_DXCH_UNIT_PB_CENTER_BLK_E, NULL,                                     \
                0x00001110, 0x00001114,                                                                 \
                prvCpssDrvHwPpPortGroupIsrRead,                                                         \
                prvCpssDrvHwPpPortGroupIsrWrite,                                                        \
                PRV_CPSS_IRONMAN_PB_CENTER_INTERRUPT_SUMMARY_GPC_CAUSE_INSTANCE_0_INTERRUPT_SUMMARY_PACKET_WRITE_GPC_E,                                   \
                PRV_CPSS_IRONMAN_PB_CENTER_INTERRUPT_SUMMARY_GPC_CAUSE_INSTANCE_0_INTERRUPT_SUMMARY_CELL_READ_GPC_E,                                      \
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},                                 \
            {3, GT_FALSE, PRV_CPSS_DXCH_UNIT_PB_CENTER_BLK_E, NULL,                                     \
                0x00001118, 0x0000111C,                                                                 \
                prvCpssDrvHwPpPortGroupIsrRead,                                                         \
                prvCpssDrvHwPpPortGroupIsrWrite,                                                        \
                PRV_CPSS_IRONMAN_PB_CENTER_INTERRUPT_SUMMARY_SMB_MEMORY_CLUSTER_CAUSE_INSTANCE_0_INTERRUPT_SUMMARY_SMB_MEMORY_CLUSTER_E,\
                PRV_CPSS_IRONMAN_PB_CENTER_INTERRUPT_SUMMARY_SMB_MEMORY_CLUSTER_CAUSE_INSTANCE_0_INTERRUPT_SUMMARY_SMB_MEMORY_CLUSTER_E,\
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},                                 \
            {4, GT_FALSE, PRV_CPSS_DXCH_UNIT_PB_CENTER_BLK_E, NULL,                                     \
                0x00001120, 0x00001124,                                                                 \
                prvCpssDrvHwPpPortGroupIsrRead,                                                         \
                prvCpssDrvHwPpPortGroupIsrWrite,                                                        \
                PRV_CPSS_IRONMAN_PB_CENTER_INTERRUPT_SUMMARY_SMB_WRITE_ARBITER_CAUSE_INSTANCE_0_INTERRUPT_SUMMARY_SMB_WRITE_ARBITER_E,\
                PRV_CPSS_IRONMAN_PB_CENTER_INTERRUPT_SUMMARY_SMB_WRITE_ARBITER_CAUSE_INSTANCE_0_INTERRUPT_SUMMARY_SMB_WRITE_ARBITER_E,\
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},                                 \
            {5, GT_FALSE, PRV_CPSS_DXCH_UNIT_PB_COUNTER_E, NULL,                                        \
                0x00000190, 0x00000194,                                                                 \
                prvCpssDrvHwPpPortGroupIsrRead,                                                         \
                prvCpssDrvHwPpPortGroupIsrWrite,                                                        \
                PRV_CPSS_IRONMAN_PB_COUNTER_INTERRUPT_CAUSE_ILLEGAL_ADDRESS_ACCESS_E,                   \
                PRV_CPSS_IRONMAN_PB_COUNTER_INTERRUPT_CAUSE_ILLEGAL_ADDRESS_ACCESS_E,                   \
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},                                 \
            {6, GT_FALSE, PRV_CPSS_DXCH_UNIT_PB_CENTER_BLK_E, NULL,                                     \
                0x00001100, 0x00001104,                                                                 \
                prvCpssDrvHwPpPortGroupIsrRead,                                                         \
                prvCpssDrvHwPpPortGroupIsrWrite,                                                        \
                PRV_CPSS_IRONMAN_PB_CENTER_INTERRUPT_PB_CENTER_CAUSE_ILLEGAL_ADDRESS_ACCESS_E,          \
                PRV_CPSS_IRONMAN_PB_CENTER_INTERRUPT_PB_CENTER_CAUSE_ILLEGAL_ADDRESS_ACCESS_E,          \
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define  rxDma0_NODE_MAC(bit)                                                                           \
    /* RXD0 */                                                                                          \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_RXDMA_E, NULL,                                                   \
        0x00001C80, 0x00001C84,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_IRONMAN_RXDMA_RX_DMA_INTERRUPT_0_CAUSE_RX_DMA_RF_ERR_E,                                \
        PRV_CPSS_IRONMAN_RXDMA_RX_DMA_INTERRUPT_0_CAUSE_HIGH_PRIO_DESC_DROP_E,                          \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define  rxDma1_NODE_MAC(bit)                                                                           \
    /* RXD1 */                                                                                          \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_RXDMA_E, NULL,                                                   \
        0x00001C88, 0x00001C8C,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_IRONMAN_RXDMA_RX_DMA_INTERRUPT_1_CAUSE_DESC_ENQUEUE_FIFO_FULL_INT_E,                   \
        PRV_CPSS_IRONMAN_RXDMA_RX_DMA_INTERRUPT_1_CAUSE_DESC_ENQUEUE_FIFO_FULL_INT_E,                   \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define  rxDma2_NODE_MAC(bit)                                                                           \
    /* RXD2 */                                                                                          \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_RXDMA_E, NULL,                                                   \
        0x00001C90, 0x00001C94,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_IRONMAN_RXDMA_RX_DMA_INTERRUPT_2_CAUSE_PB_TAIL_ID_MEM_SER_INT_E,                       \
        PRV_CPSS_IRONMAN_RXDMA_RX_DMA_INTERRUPT_2_CAUSE_RX_LOCAL_IDS_FIFO_MEM_RD_SER_INT_E,             \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define  rxDma_SUB_TREE_MAC(bit)                                                                        \
    /* RXD */                                                                                           \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_RXDMA_E, NULL,                                                   \
        0x00001CA0, 0x00001CA4,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_IRONMAN_RXDMA_RX_DMA_INTERRUPT_SUMMARY_CAUSE_RX_DMA_INTERRUPT_0_INT_E,                 \
        PRV_CPSS_IRONMAN_RXDMA_RX_DMA_INTERRUPT_SUMMARY_CAUSE_RX_DMA_INTERRUPT_2_INT_E,                 \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 3, NULL, NULL},                                         \
            rxDma0_NODE_MAC(1),                                                                         \
            rxDma1_NODE_MAC(2),                                                                         \
            rxDma2_NODE_MAC(3)

#define   txDma_NODE_MAC(bit)                                                                           \
    /* TXD */                                                                                           \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_TXDMA_E, NULL,                                                   \
        0x00006000, 0x00006004,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_IRONMAN_TXDMA_TXD_INTERRUPT_CAUSE_TXD_RF_ERR_E,                                        \
        PRV_CPSS_IRONMAN_TXDMA_TXD_INTERRUPT_CAUSE_DWIP_FIFO_OVER_FLOW_E,                               \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define   txFifo_NODE_MAC(bit)                                                                          \
    /* TXF */                                                                                           \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_TX_FIFO_E, NULL,                                                 \
        0x00004000, 0x00004004,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_IRONMAN_TXFIFO_TXF_INTERRUPT_CAUSE_TX_FIFO_RF_ERR_E,                                   \
        PRV_CPSS_IRONMAN_TXFIFO_TXF_INTERRUPT_CAUSE_TXF_INIT_PROCESS_FINISH_E,                          \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define global_cstu_NODE_MAC(bit)                                                                       \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_CTSU_0_E, NULL,                                              \
        0x00000080, 0x00000084,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_IRONMAN_CTSU_CTSU_GLOBAL_INTERRUPT_CAUSE_ILLEGAL_ADDRESS_ACCESS_E,                     \
        PRV_CPSS_IRONMAN_CTSU_CTSU_GLOBAL_INTERRUPT_CAUSE_TIME_STAMPING_COMMAND_PARSING_ERROR_E,        \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define channel_cstu_NODE_MAC(bit, channel)                                                             \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_CTSU_0_E, NULL,                                              \
        CSTU_CHANNEL_INTERRUPT_CAUSE_MAC(channel),                                                      \
        CSTU_CHANNEL_INTERRUPT_MASK_MAC(channel),                                                       \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_IRONMAN_CTSU_CHANNEL_##channel##_CHANNEL_EGRESS_TIMESTAMP_QUEUE_0_NEW_ENTRY_INTERRUPT_E, \
        PRV_CPSS_IRONMAN_CTSU_CHANNEL_##channel##_CHANNEL_EGRESS_TIMESTAMP_QUEUE_1_OVERFLOW_INTERRUPT_E,  \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define GROUP_0_CHANNEL_CSTU_16_NODES_MAC                                                               \
        channel_cstu_NODE_MAC( 1,  0),                                                                  \
        channel_cstu_NODE_MAC( 2,  1),                                                                  \
        channel_cstu_NODE_MAC( 3,  2),                                                                  \
        channel_cstu_NODE_MAC( 4,  3),                                                                  \
        channel_cstu_NODE_MAC( 5,  4),                                                                  \
        channel_cstu_NODE_MAC( 6,  5),                                                                  \
        channel_cstu_NODE_MAC( 7,  6),                                                                  \
        channel_cstu_NODE_MAC( 8,  7),                                                                  \
        channel_cstu_NODE_MAC( 9,  8),                                                                  \
        channel_cstu_NODE_MAC(10,  9),                                                                  \
        channel_cstu_NODE_MAC(11, 10),                                                                  \
        channel_cstu_NODE_MAC(12, 11),                                                                  \
        channel_cstu_NODE_MAC(13, 12),                                                                  \
        channel_cstu_NODE_MAC(14, 13),                                                                  \
        channel_cstu_NODE_MAC(15, 14),                                                                  \
        channel_cstu_NODE_MAC(16, 15)

#define GROUP_1_CHANNEL_CSTU_16_NODES_MAC                                                               \
        channel_cstu_NODE_MAC( 1, 16),                                                                  \
        channel_cstu_NODE_MAC( 2, 17),                                                                  \
        channel_cstu_NODE_MAC( 3, 18),                                                                  \
        channel_cstu_NODE_MAC( 4, 19),                                                                  \
        channel_cstu_NODE_MAC( 5, 20),                                                                  \
        channel_cstu_NODE_MAC( 6, 21),                                                                  \
        channel_cstu_NODE_MAC( 7, 22),                                                                  \
        channel_cstu_NODE_MAC( 8, 23),                                                                  \
        channel_cstu_NODE_MAC( 9, 24),                                                                  \
        channel_cstu_NODE_MAC(10, 25),                                                                  \
        channel_cstu_NODE_MAC(11, 26),                                                                  \
        channel_cstu_NODE_MAC(12, 27),                                                                  \
        channel_cstu_NODE_MAC(13, 28),                                                                  \
        channel_cstu_NODE_MAC(14, 29),                                                                  \
        channel_cstu_NODE_MAC(15, 30),                                                                  \
        channel_cstu_NODE_MAC(16, 31)

#define GROUP_2_CHANNEL_CSTU_16_NODES_MAC                                                               \
        channel_cstu_NODE_MAC( 1, 32),                                                                  \
        channel_cstu_NODE_MAC( 2, 33),                                                                  \
        channel_cstu_NODE_MAC( 3, 34),                                                                  \
        channel_cstu_NODE_MAC( 4, 35),                                                                  \
        channel_cstu_NODE_MAC( 5, 36),                                                                  \
        channel_cstu_NODE_MAC( 6, 37),                                                                  \
        channel_cstu_NODE_MAC( 7, 38),                                                                  \
        channel_cstu_NODE_MAC( 8, 39),                                                                  \
        channel_cstu_NODE_MAC( 9, 40),                                                                  \
        channel_cstu_NODE_MAC(10, 41),                                                                  \
        channel_cstu_NODE_MAC(11, 42),                                                                  \
        channel_cstu_NODE_MAC(12, 43),                                                                  \
        channel_cstu_NODE_MAC(13, 44),                                                                  \
        channel_cstu_NODE_MAC(14, 45),                                                                  \
        channel_cstu_NODE_MAC(15, 46),                                                                  \
        channel_cstu_NODE_MAC(16, 47)

#define GROUP_3_CHANNEL_CSTU_16_NODES_MAC                                                               \
        channel_cstu_NODE_MAC( 1, 48),                                                                  \
        channel_cstu_NODE_MAC( 2, 49),                                                                  \
        channel_cstu_NODE_MAC( 3, 50),                                                                  \
        channel_cstu_NODE_MAC( 4, 51),                                                                  \
        channel_cstu_NODE_MAC( 5, 52),                                                                  \
        channel_cstu_NODE_MAC( 6, 53),                                                                  \
        channel_cstu_NODE_MAC( 7, 54),                                                                  \
        channel_cstu_NODE_MAC( 8, 55),                                                                  \
        channel_cstu_NODE_MAC( 9, 56),                                                                  \
        channel_cstu_NODE_MAC(10, 57),                                                                  \
        channel_cstu_NODE_MAC(11, 58),                                                                  \
        channel_cstu_NODE_MAC(12, 59),                                                                  \
        channel_cstu_NODE_MAC(13, 60),                                                                  \
        channel_cstu_NODE_MAC(14, 61),                                                                  \
        channel_cstu_NODE_MAC(15, 62),                                                                  \
        channel_cstu_NODE_MAC(16, 63)

#define channel_group_cstu_SUB_TREE_MAC(bit, group)                                                     \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_CTSU_0_E, NULL,                                              \
        CSTU_CHANNEL_GROUP_INTERRUPT_CAUSE_MAC(group),                                                  \
        CSTU_CHANNEL_GROUP_INTERRUPT_MASK_MAC(group),                                                   \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_IRONMAN_CTSU_CH_GROUP_##group##_INTERRUPT_CAUSE_SUMMARY_CH_GROUP_##group##_INTERRUPT_SUMMARY_E + 1, \
        PRV_CPSS_IRONMAN_CTSU_CH_GROUP_##group##_INTERRUPT_CAUSE_SUMMARY_RESERVED_17_E - 1,             \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 16, NULL, NULL},                                    \
            GROUP_##group##_CHANNEL_CSTU_16_NODES_MAC

#define msec_ING_CH0_NODE_MAC(bit, _unit, _index)                                                                \
    /* MacSec Channel 0 Interrupts Cause */                                                                      \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_I_##_unit##_0_E,  NULL,                                    \
        0x00000508, 0x0000050c,                                                                                  \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                          \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                         \
        PRV_CPSS_IRONMAN_163_AND_164_##_index##_I_0_MAC_SEC_CHANNEL_INTERRUPTS_0_CAUSE_CHANNEL_0_CHANNEL_INTERRUPT_E, \
        PRV_CPSS_IRONMAN_163_AND_164_##_index##_I_0_MAC_SEC_CHANNEL_INTERRUPTS_0_CAUSE_CHANNEL_15_CHANNEL_INTERRUPT_E,\
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define msec_ING_CH1_NODE_MAC(bit, _unit, _index)                                                                \
    /* MacSec Channel 1 Interrupts Cause */                                                                      \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_I_##_unit##_0_E,  NULL,                                    \
        0x00000510, 0x00000514,                                                                                  \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                          \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                         \
        PRV_CPSS_IRONMAN_163_AND_164_##_index##_I_0_MAC_SEC_CHANNEL_INTERRUPTS_1_CAUSE_CHANNEL_16_CHANNEL_INTERRUPT_E,\
        PRV_CPSS_IRONMAN_163_AND_164_##_index##_I_0_MAC_SEC_CHANNEL_INTERRUPTS_1_CAUSE_CHANNEL_31_CHANNEL_INTERRUPT_E,\
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define msec_ING_CH2_NODE_MAC(bit, _unit, _index)                                                                \
    /* MacSec Channel 2 Interrupts Cause */                                                                      \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_I_##_unit##_0_E,  NULL,                                    \
        0x00000518, 0x0000051c,                                                                                  \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                          \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                         \
        PRV_CPSS_IRONMAN_163_AND_164_##_index##_I_0_MAC_SEC_CHANNEL_INTERRUPTS_2_CAUSE_CHANNEL_32_CHANNEL_INTERRUPT_E,\
        PRV_CPSS_IRONMAN_163_AND_164_##_index##_I_0_MAC_SEC_CHANNEL_INTERRUPTS_2_CAUSE_CHANNEL_47_CHANNEL_INTERRUPT_E,\
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define msec_ING_CH3_NODE_MAC(bit, _unit, _index)                                                                \
    /* MacSec Channel 3 Interrupts Cause */                                                                      \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_I_##_unit##_0_E,  NULL,                                    \
        0x00000520, 0x00000524,                                                                                  \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                          \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                         \
        PRV_CPSS_IRONMAN_163_AND_164_##_index##_I_0_MAC_SEC_CHANNEL_INTERRUPTS_3_CAUSE_CHANNEL_48_CHANNEL_INTERRUPT_E,\
        PRV_CPSS_IRONMAN_163_AND_164_##_index##_I_0_MAC_SEC_CHANNEL_INTERRUPTS_3_CAUSE_CHANNEL_63_CHANNEL_INTERRUPT_E,\
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define msec_EGR_CH0_NODE_MAC(bit, _unit, _index)                                                                \
    /* MacSec Channel 0 Interrupts Cause */                                                                      \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_E_##_unit##_0_E,  NULL,                                    \
        0x00000508, 0x0000050c,                                                                                  \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                          \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                         \
        PRV_CPSS_IRONMAN_163_AND_164_##_index##_E_1_MAC_SEC_CHANNEL_INTERRUPTS_0_CAUSE_CHANNEL_0_CHANNEL_INTERRUPT_E, \
        PRV_CPSS_IRONMAN_163_AND_164_##_index##_E_1_MAC_SEC_CHANNEL_INTERRUPTS_0_CAUSE_CHANNEL_15_CHANNEL_INTERRUPT_E,\
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define msec_EGR_CH1_NODE_MAC(bit, _unit, _index)                                                                \
    /* MacSec Channel 1 Interrupts Cause */                                                                      \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_E_##_unit##_0_E,  NULL,                                    \
        0x00000510, 0x00000514,                                                                                  \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                          \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                         \
        PRV_CPSS_IRONMAN_163_AND_164_##_index##_E_1_MAC_SEC_CHANNEL_INTERRUPTS_1_CAUSE_CHANNEL_16_CHANNEL_INTERRUPT_E,\
        PRV_CPSS_IRONMAN_163_AND_164_##_index##_E_1_MAC_SEC_CHANNEL_INTERRUPTS_1_CAUSE_CHANNEL_31_CHANNEL_INTERRUPT_E,\
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define msec_EGR_CH2_NODE_MAC(bit, _unit, _index)                                                                \
    /* MacSec Channel 2 Interrupts Cause */                                                                      \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_E_##_unit##_0_E,  NULL,                                    \
        0x00000518, 0x0000051c,                                                                                  \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                          \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                         \
        PRV_CPSS_IRONMAN_163_AND_164_##_index##_E_1_MAC_SEC_CHANNEL_INTERRUPTS_2_CAUSE_CHANNEL_32_CHANNEL_INTERRUPT_E,\
        PRV_CPSS_IRONMAN_163_AND_164_##_index##_E_1_MAC_SEC_CHANNEL_INTERRUPTS_2_CAUSE_CHANNEL_47_CHANNEL_INTERRUPT_E,\
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define msec_EGR_CH3_NODE_MAC(bit, _unit, _index)                                                                \
    /* MacSec Channel 3 Interrupts Cause */                                                                      \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_E_##_unit##_0_E,  NULL,                                    \
        0x00000520, 0x00000524,                                                                                  \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                          \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                         \
        PRV_CPSS_IRONMAN_163_AND_164_##_index##_E_1_MAC_SEC_CHANNEL_INTERRUPTS_3_CAUSE_CHANNEL_48_CHANNEL_INTERRUPT_E,\
        PRV_CPSS_IRONMAN_163_AND_164_##_index##_E_1_MAC_SEC_CHANNEL_INTERRUPTS_3_CAUSE_CHANNEL_63_CHANNEL_INTERRUPT_E,\
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define msec_EGR_EIP_163_GLOBAL_NODE_MAC(bit, _unit, _index)                                            \
    /* Classifier Engine(EIP-163) Egress Interrupts Cause */                                            \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_##_unit##_E_0_E,  NULL,                           \
        0x0000FC2C, 0x0000FC08,                                                                         \
        prvCpssDrvHwPpPortGroupAicIsrRead,                                                              \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_IRONMAN_MSEC_EGR_163_INST_##_index##_GLOBAL_MAC_SEC_INTERRUPT_CAUSE_STAT_TCAM_THR_E,   \
        PRV_CPSS_IRONMAN_MSEC_EGR_163_INST_##_index##_GLOBAL_MAC_SEC_INTERRUPT_CAUSE_CHANNEL_IRQ_15_31_E,  \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define msec_ING_EIP_163_GLOBAL_NODE_MAC(bit, _unit, _index)                                            \
    /* Classifier Engine(EIP-163) Ingress Interrupts Cause */                                           \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_##_unit##_I_0_E,  NULL,                           \
        0x0000FC2C, 0x0000FC08,                                                                         \
        prvCpssDrvHwPpPortGroupAicIsrRead,                                                              \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_IRONMAN_MSEC_ING_163_INST_##_index##_GLOBAL_MAC_SEC_INTERRUPT_CAUSE_STAT_TCAM_THR_E,   \
        PRV_CPSS_IRONMAN_MSEC_ING_163_INST_##_index##_GLOBAL_MAC_SEC_INTERRUPT_CAUSE_CHANNEL_IRQ_15_31_E,\
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define msec_EGR_EIP_66_NODE_MAC(bit, _unit, _index)                                                    \
    /* MACSec Transform Crypto Engine(EIP-66) Egress Interrupts Cause */                                \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_##_unit##_E_0_E,  NULL,                           \
        0x00005C2C, 0x00005C08,                                                                         \
        prvCpssDrvHwPpPortGroupAicIsrRead,                                                              \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_IRONMAN_MSEC_EGR_66_INST_##_index##_GLOBAL_MAC_SEC_INTERRUPT_CAUSE_ERROR_NOTIFICATION_NUMBER_0_E, \
        PRV_CPSS_IRONMAN_MSEC_EGR_66_INST_##_index##_GLOBAL_MAC_SEC_INTERRUPT_CAUSE_SEQ_NUM_ROLLOVER_E, \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define msec_ING_EIP_66_NODE_MAC(bit, _unit, _index)                                                    \
    /* MACSec Transform Crypto Engine(EIP-66) Ingress Interrupts Cause */                               \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_##_unit##_I_0_E,  NULL,                           \
        0x00005C2C, 0x00005C08,                                                                         \
        prvCpssDrvHwPpPortGroupAicIsrRead,                                                              \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_IRONMAN_MSEC_ING_66_INST_##_index##_GLOBAL_MAC_SEC_INTERRUPT_CAUSE_ERROR_NOTIFICATION_NUMBER_0_E, \
        PRV_CPSS_IRONMAN_MSEC_ING_66_INST_##_index##_GLOBAL_MAC_SEC_INTERRUPT_CAUSE_ERROR_NOTIFICATION_NUMBER_14_E,\
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define msec_EGR_EIP_164_GLOBAL_NODE_MAC(bit, _unit, _index)                                            \
    /* Transformer Engine(EIP-164) Egress Interrupts Cause */                                           \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_##_unit##_E_0_E,  NULL,                           \
        0x0000FC2C, 0x0000FC08,                                                                         \
        prvCpssDrvHwPpPortGroupAicIsrRead,                                                              \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_IRONMAN_MSEC_EGR_164_INST_##_index##_GLOBAL_MAC_SEC_INTERRUPT_CAUSE_DROP_CLASS_E,      \
        PRV_CPSS_IRONMAN_MSEC_EGR_164_INST_##_index##_GLOBAL_MAC_SEC_INTERRUPT_CAUSE_CHANNEL_IRQ_15_31_E,\
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 1, NULL, NULL},                                         \
        msec_EGR_EIP_66_NODE_MAC(3, _unit, _index)

#define msec_ING_EIP_164_GLOBAL_NODE_MAC(bit, _unit, _index)                                            \
    /* Transformer Engine(EIP-164) Ingress Interrupts Cause */                                          \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EIP_##_unit##_I_0_E,  NULL,                           \
        0x0000FC2C, 0x0000FC08,                                                                         \
        prvCpssDrvHwPpPortGroupAicIsrRead,                                                              \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_IRONMAN_MSEC_ING_164_INST_##_index##_GLOBAL_MAC_SEC_INTERRUPT_CAUSE_DROP_CLASS_E, \
        PRV_CPSS_IRONMAN_MSEC_ING_164_INST_##_index##_GLOBAL_MAC_SEC_INTERRUPT_CAUSE_CHANNEL_IRQ_15_31_E,\
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 1, NULL, NULL},                                         \
        msec_ING_EIP_66_NODE_MAC(3, _unit, _index)

#define gop_eip_163i_SUB_TREE_MAC(bit)                                                                  \
    /* MSEC Ingress 163 */                                                                              \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_I_163_0_E,  NULL,                                 \
        0x00000500, 0x00000504,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_IRONMAN_163_AND_164_0_I_0_MAC_SEC_INTERRUPT_CAUSE_MAC_SEC_IP_GLOBAL_IRQ_E,             \
        PRV_CPSS_IRONMAN_163_AND_164_0_I_0_MAC_SEC_INTERRUPT_CAUSE_ILLEGAL_ADDRESS_ACCESS_E,            \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 1, NULL, NULL},                                         \
        msec_ING_EIP_163_GLOBAL_NODE_MAC(1, 163, 1)

#define gop_eip_164i_SUB_TREE_MAC(bit)                                                                  \
    /* MSEC Ingress 164 */                                                                              \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_I_164_0_E,  NULL,                                 \
        0x00000500, 0x00000504,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_IRONMAN_163_AND_164_1_I_0_MAC_SEC_INTERRUPT_CAUSE_MAC_SEC_IP_GLOBAL_IRQ_E,             \
        PRV_CPSS_IRONMAN_163_AND_164_1_I_0_MAC_SEC_INTERRUPT_CAUSE_ILLEGAL_ADDRESS_ACCESS_E,            \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 1, NULL, NULL},                                         \
        msec_ING_EIP_164_GLOBAL_NODE_MAC(1, 164, 2)

#define gop_eip_163e_SUB_TREE_MAC(bit)                                                                  \
    /* MSEC Egress 163 */                                                                               \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_E_163_0_E,  NULL,                                 \
        0x00000500, 0x00000504,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_IRONMAN_163_AND_164_0_E_1_MAC_SEC_INTERRUPT_CAUSE_MAC_SEC_IP_GLOBAL_IRQ_E,             \
        PRV_CPSS_IRONMAN_163_AND_164_0_E_1_MAC_SEC_INTERRUPT_CAUSE_ILLEGAL_ADDRESS_ACCESS_E,            \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 1, NULL, NULL},                                         \
        msec_EGR_EIP_163_GLOBAL_NODE_MAC(1, 163, 0)

#define gop_eip_164e_SUB_TREE_MAC(bit)                                                                  \
    /* MSEC Egress 164 */                                                                               \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_E_164_0_E,  NULL,                                 \
        0x00000500, 0x00000504,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_IRONMAN_163_AND_164_1_E_1_MAC_SEC_INTERRUPT_CAUSE_MAC_SEC_IP_GLOBAL_IRQ_E,             \
        PRV_CPSS_IRONMAN_163_AND_164_1_E_1_MAC_SEC_INTERRUPT_CAUSE_ILLEGAL_ADDRESS_ACCESS_E,            \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 1, NULL, NULL},                                         \
        msec_EGR_EIP_164_GLOBAL_NODE_MAC(1, 164, 1)

#define gop_163_e_mac_sec_summary_SUB_TREE_MAC(bit)                                     \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_E_163_0_E, NULL,                                  \
        0x00000548, 0x0000054c,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_IRONMAN_163_AND_164_0_E_1_MAC_SEC_INTERRUPT_CAUSE_SUMMARY_MAC_SEC_CHANNELS_0_INTERRUPT_E,   \
        PRV_CPSS_IRONMAN_163_AND_164_0_E_1_MAC_SEC_INTERRUPT_CAUSE_SUMMARY_MAC_SEC_GLOBAL_INTERRUPT_CAUSE_E, \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 5, NULL, NULL},                                         \
        msec_EGR_CH0_NODE_MAC(1, 163, 0),                                                               \
        msec_EGR_CH1_NODE_MAC(2, 163, 0),                                                               \
        msec_EGR_CH2_NODE_MAC(3, 163, 0),                                                               \
        msec_EGR_CH3_NODE_MAC(4, 163, 0),                                                               \
        gop_eip_163e_SUB_TREE_MAC(9)

#define gop_164_e_mac_sec_summary_SUB_TREE_MAC(bit)                                     \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_E_164_0_E, NULL,                                  \
        0x00000548, 0x0000054c,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_IRONMAN_163_AND_164_1_E_1_MAC_SEC_INTERRUPT_CAUSE_SUMMARY_MAC_SEC_CHANNELS_0_INTERRUPT_E,   \
        PRV_CPSS_IRONMAN_163_AND_164_1_E_1_MAC_SEC_INTERRUPT_CAUSE_SUMMARY_MAC_SEC_GLOBAL_INTERRUPT_CAUSE_E, \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 5, NULL, NULL},                                         \
        msec_EGR_CH0_NODE_MAC(1, 164, 1),                                                               \
        msec_EGR_CH1_NODE_MAC(2, 164, 1),                                                               \
        msec_EGR_CH2_NODE_MAC(3, 164, 1),                                                               \
        msec_EGR_CH3_NODE_MAC(4, 164, 1),                                                               \
        gop_eip_164e_SUB_TREE_MAC(9)

#define gop_163_i_mac_sec_summary_SUB_TREE_MAC(bit)                                     \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_I_163_0_E, NULL,                                  \
        0x00000548, 0x0000054c,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_IRONMAN_163_AND_164_0_I_0_MAC_SEC_INTERRUPT_CAUSE_SUMMARY_MAC_SEC_CHANNELS_0_INTERRUPT_E,   \
        PRV_CPSS_IRONMAN_163_AND_164_0_I_0_MAC_SEC_INTERRUPT_CAUSE_SUMMARY_MAC_SEC_GLOBAL_INTERRUPT_CAUSE_E, \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 5, NULL, NULL},                                         \
        msec_ING_CH0_NODE_MAC(1, 163, 0),                                                               \
        msec_ING_CH1_NODE_MAC(2, 163, 0),                                                               \
        msec_ING_CH2_NODE_MAC(3, 163, 0),                                                               \
        msec_ING_CH3_NODE_MAC(4, 163, 0),                                                               \
        gop_eip_163i_SUB_TREE_MAC(9)


#define gop_164_i_mac_sec_summary_SUB_TREE_MAC(bit)                                     \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_MACSEC_EXT_I_164_0_E, NULL,                                  \
        0x00000548, 0x0000054c,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_IRONMAN_163_AND_164_1_I_0_MAC_SEC_INTERRUPT_CAUSE_SUMMARY_MAC_SEC_CHANNELS_0_INTERRUPT_E,   \
        PRV_CPSS_IRONMAN_163_AND_164_1_I_0_MAC_SEC_INTERRUPT_CAUSE_SUMMARY_MAC_SEC_GLOBAL_INTERRUPT_CAUSE_E, \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 5, NULL, NULL},                                         \
        msec_ING_CH0_NODE_MAC(1, 164, 1),                                                               \
        msec_ING_CH1_NODE_MAC(2, 164, 1),                                                               \
        msec_ING_CH2_NODE_MAC(3, 164, 1),                                                               \
        msec_ING_CH3_NODE_MAC(4, 164, 1),                                                               \
        gop_eip_164i_SUB_TREE_MAC(9)


#define gop_ctsu_SUB_TREE_MAC(bit)                                                                      \
    /* CTSU */                                                                                          \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_PCA_CTSU_0_E, NULL,                                              \
        0x00000088, 0x0000008C,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_IRONMAN_CTSU_CTSU_INTERRUPT_CAUSE_SUMMARY_GLOBAL_INTERRUPT_CAUSE_SUMMARY_E,            \
        PRV_CPSS_IRONMAN_CTSU_CTSU_INTERRUPT_CAUSE_SUMMARY_CHANNEL_GROUP_3_INTERRUPT_CAUSE_SUMMARY_E,   \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 5, NULL, NULL},                                         \
            global_cstu_NODE_MAC(1),                                                                    \
            channel_group_cstu_SUB_TREE_MAC(2, 0),                                                      \
            channel_group_cstu_SUB_TREE_MAC(3, 1),                                                      \
            channel_group_cstu_SUB_TREE_MAC(4, 2),                                                      \
            channel_group_cstu_SUB_TREE_MAC(5, 3)

#define mac8br_port_summary_cause_SUB_TREE_MAC(bit, unit, port)                                         \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_MTI_10G_MAC_BR_##unit##_E, NULL,                                 \
        PORT_MTI_USX_X8_MAC_INTERRUPT_SUMMARY_CAUSE_MAC(port),                                          \
        PORT_MTI_USX_X8_MAC_INTERRUPT_SUMMARY_MASK_MAC(port),                                           \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_IRONMAN_MAC8BR_##unit##_PORT_NUM_##port##_PORT_SUMMARY_INTERRUPT_CAUSE_PORT_INTERRUPT_E,               \
        PRV_CPSS_IRONMAN_MAC8BR_##unit##_PORT_NUM_##port##_PORT_SUMMARY_INTERRUPT_CAUSE_PORT_PMAC_USX_PCH_INTERRUPT_E,  \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 1, NULL, NULL},                                         \
        {1, GT_FALSE, PRV_CPSS_DXCH_UNIT_MTI_10G_MAC_BR_##unit##_E, NULL,                               \
            PORT_MTI_USX_X8_MAC_INTERRUPT_CAUSE_MAC(port),                                              \
            PORT_MTI_USX_X8_MAC_INTERRUPT_MASK_MAC(port),                                               \
            prvCpssDrvHwPpPortGroupIsrRead,                                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
            PRV_CPSS_IRONMAN_MAC8BR_##unit##_PORT_NUM_##port##_PORT_INTERRUPT_CAUSE_ILLEGAL_ADDRESS_ACCESS_E,  \
            PRV_CPSS_IRONMAN_MAC8BR_##unit##_PORT_NUM_##port##_PORT_INTERRUPT_CAUSE_TSQ_TS_FIFO_OVERWRITE_E,   \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define mac8br_summary_SUB_TREE_MAC(bit, unit)                                                          \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_MTI_10G_MAC_BR_##unit##_E, NULL,                                 \
        0x00000018, 0x0000001c,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_IRONMAN_MAC8BR_##unit##_GLOBAL_INTERRUPT_SUMMARY_CAUSE_GLOBAL_INT_SUM_E,               \
        PRV_CPSS_IRONMAN_MAC8BR_##unit##_GLOBAL_INTERRUPT_SUMMARY_CAUSE_PORT_NUM_7_P_INT_SUM_E,         \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 8, NULL, NULL},                                         \
            mac8br_port_summary_cause_SUB_TREE_MAC(2, unit, 0),                                         \
            mac8br_port_summary_cause_SUB_TREE_MAC(3, unit, 1),                                         \
            mac8br_port_summary_cause_SUB_TREE_MAC(4, unit, 2),                                         \
            mac8br_port_summary_cause_SUB_TREE_MAC(5, unit, 3),                                         \
            mac8br_port_summary_cause_SUB_TREE_MAC(6, unit, 4),                                         \
            mac8br_port_summary_cause_SUB_TREE_MAC(7, unit, 5),                                         \
            mac8br_port_summary_cause_SUB_TREE_MAC(8, unit, 6),                                         \
            mac8br_port_summary_cause_SUB_TREE_MAC(9, unit, 7)

#define mac8_port_summary_cause_SUB_TREE_MAC(bit, unit, port)                                           \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_MTI_10G_MAC_NON_BR_##unit##_E, NULL,                             \
        PORT_MTI_USX_X8_MAC_INTERRUPT_SUMMARY_CAUSE_MAC(port),                                          \
        PORT_MTI_USX_X8_MAC_INTERRUPT_SUMMARY_MASK_MAC(port),                                           \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_IRONMAN_MAC8_##unit##_PORT_NUM_##port##_PORT_SUMMARY_INTERRUPT_CAUSE_PORT_INTERRUPT_E, \
        PRV_CPSS_IRONMAN_MAC8_##unit##_PORT_NUM_##port##_PORT_SUMMARY_INTERRUPT_CAUSE_PORT_PMAC_USX_PCH_INTERRUPT_E,  \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 1, NULL, NULL},                                         \
        {1, GT_FALSE, PRV_CPSS_DXCH_UNIT_MTI_10G_MAC_NON_BR_##unit##_E, NULL,                           \
            PORT_MTI_USX_X8_MAC_INTERRUPT_CAUSE_MAC(port),                                              \
            PORT_MTI_USX_X8_MAC_INTERRUPT_MASK_MAC(port),                                               \
            prvCpssDrvHwPpPortGroupIsrRead,                                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
            PRV_CPSS_IRONMAN_MAC8_##unit##_PORT_NUM_##port##_PORT_INTERRUPT_CAUSE_ILLEGAL_ADDRESS_ACCESS_E, \
            PRV_CPSS_IRONMAN_MAC8_##unit##_PORT_NUM_##port##_PORT_INTERRUPT_CAUSE_TSQ_TS_FIFO_OVERWRITE_E,  \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define mac8_summary_SUB_TREE_MAC(bit, unit)                                                            \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_MTI_10G_MAC_NON_BR_##unit##_E , NULL,                            \
        0x00000018, 0x0000001c,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_IRONMAN_MAC8_##unit##_GLOBAL_INTERRUPT_SUMMARY_CAUSE_GLOBAL_INT_SUM_E,                 \
        PRV_CPSS_IRONMAN_MAC8_##unit##_GLOBAL_INTERRUPT_SUMMARY_CAUSE_PORT_NUM_7_P_INT_SUM_E,           \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 8, NULL, NULL},                                         \
            mac8_port_summary_cause_SUB_TREE_MAC(2, unit, 0),                                           \
            mac8_port_summary_cause_SUB_TREE_MAC(3, unit, 1),                                           \
            mac8_port_summary_cause_SUB_TREE_MAC(4, unit, 2),                                           \
            mac8_port_summary_cause_SUB_TREE_MAC(5, unit, 3),                                           \
            mac8_port_summary_cause_SUB_TREE_MAC(6, unit, 4),                                           \
            mac8_port_summary_cause_SUB_TREE_MAC(7, unit, 5),                                           \
            mac8_port_summary_cause_SUB_TREE_MAC(8, unit, 6),                                           \
            mac8_port_summary_cause_SUB_TREE_MAC(9, unit, 7)

#define mac_port_summary_cause_SUB_TREE_MAC(bit, unit)                                                  \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_MTI_10G_MAC_NON_BR_3_E + unit, NULL,                             \
        0x00000168, 0x0000016c,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_IRONMAN_MAC_##unit##_PORT_NUM_0_PORT_SUMMARY_INTERRUPT_CAUSE_PORT_INTERRUPT_E,         \
        PRV_CPSS_IRONMAN_MAC_##unit##_PORT_NUM_0_PORT_SUMMARY_INTERRUPT_CAUSE_PORT_PMAC_USX_PCH_INTERRUPT_E,      \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 1, NULL, NULL},                                         \
        {1, GT_FALSE, PRV_CPSS_DXCH_UNIT_MTI_10G_MAC_NON_BR_3_E + unit, NULL,                           \
            0x0000010c, 0x00000110,                                                                     \
            prvCpssDrvHwPpPortGroupIsrRead,                                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
            PRV_CPSS_IRONMAN_MAC_##unit##_PORT_NUM_0_PORT_INTERRUPT_CAUSE_ILLEGAL_ADDRESS_ACCESS_E,     \
            PRV_CPSS_IRONMAN_MAC_##unit##_PORT_NUM_0_PORT_INTERRUPT_CAUSE_TSQ_TS_FIFO_OVERWRITE_E,      \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define mac_summary_SUB_TREE_MAC(bit, unit)                                                             \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_MTI_10G_MAC_NON_BR_3_E + unit, NULL,                             \
        0x00000018, 0x0000001c,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_IRONMAN_MAC_##unit##_GLOBAL_INTERRUPT_SUMMARY_CAUSE_GLOBAL_INT_SUM_E,                  \
        PRV_CPSS_IRONMAN_MAC_##unit##_GLOBAL_INTERRUPT_SUMMARY_CAUSE_PORT_NUM_7_P_INT_SUM_E,            \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 1, NULL, NULL},                                         \
            mac_port_summary_cause_SUB_TREE_MAC(2, unit)

#define cpu_mac_summary_cause_SUB_TREE_MAC(bit, index)                                                  \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_MTI_10G_MAC_BR_3_E + index, NULL,                                \
        0x00000168, 0x0000016c,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_IRONMAN_CPU_MAC_BR_##index##_PORT_NUM_0_PORT_SUMMARY_INTERRUPT_CAUSE_PORT_INTERRUPT_E, \
        PRV_CPSS_IRONMAN_CPU_MAC_BR_##index##_PORT_NUM_0_PORT_SUMMARY_INTERRUPT_CAUSE_PORT_PMAC_USX_PCH_INTERRUPT_E,  \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 1, NULL, NULL},                                         \
        {1, GT_FALSE, PRV_CPSS_DXCH_UNIT_MTI_10G_MAC_BR_3_E + index, NULL,                              \
            0x0000010c, 0x00000110,                                                                     \
            prvCpssDrvHwPpPortGroupIsrRead,                                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
            PRV_CPSS_IRONMAN_CPU_MAC_BR_##index##_PORT_NUM_0_PORT_INTERRUPT_CAUSE_ILLEGAL_ADDRESS_ACCESS_E, \
            PRV_CPSS_IRONMAN_CPU_MAC_BR_##index##_PORT_NUM_0_PORT_INTERRUPT_CAUSE_TSQ_TS_FIFO_OVERWRITE_E,  \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define cpu_mac_summary_SUB_TREE_MAC(bit, index)                                                        \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_MTI_10G_MAC_BR_3_E + index, NULL,                                \
        0x00000018, 0x0000001c,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_IRONMAN_CPU_MAC_BR_##index##_GLOBAL_INTERRUPT_SUMMARY_CAUSE_GLOBAL_INT_SUM_E,          \
        PRV_CPSS_IRONMAN_CPU_MAC_BR_##index##_GLOBAL_INTERRUPT_SUMMARY_CAUSE_PORT_NUM_7_P_INT_SUM_E,    \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 1, NULL, NULL},                                         \
            cpu_mac_summary_cause_SUB_TREE_MAC(2, index)

#define pcs_port_cause_NODE_MAC(bit, unit)                                                              \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_MTI_10G_PCS_x1_4_E + unit , NULL,                                \
        0x00000050, 0x00000070,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_IRONMAN_PCS_##unit##_NO_CPU_0_PORT_NUM_0_LINK_STATUS_CHANGE_E,                         \
        PRV_CPSS_IRONMAN_PCS_##unit##_NO_CPU_0_PORT_NUM_0_LPCS_AN_DONE_INT_E,                           \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define pcs_cpu_cause_NODE_MAC(bit, unit)                                                               \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_MTI_10G_PCS_x1_0_E + unit, NULL,                                 \
        0x00000050, 0x00000054,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_IRONMAN_CPU_PCS_0_TO_3_0_CPU_INSTANCES_1_PORT_NUM_LINK_STATUS_CHANGE_E,                \
        PRV_CPSS_IRONMAN_CPU_PCS_0_TO_3_0_CPU_INSTANCES_1_PORT_NUM_LPCS_AN_DONE_INT_E,                  \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define pcs_summary_SUB_TREE_MAC(bit, unit)                                                             \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_MTI_10G_PCS_x1_4_E + unit, NULL,                                 \
        0x00000014, 0x00000018,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_IRONMAN_PCS_##unit##_NO_CPU_0_GLOBAL_INTERRUPT_SUMMARY_CAUSE_PORT_NUM_0_P_INT_SUM_E,   \
        PRV_CPSS_IRONMAN_PCS_##unit##_NO_CPU_0_GLOBAL_INTERRUPT_SUMMARY_CAUSE_SD_NUM_1_SD_100FX_INT_SUM_E,\
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 1, NULL, NULL},                                         \
            pcs_port_cause_NODE_MAC(1, unit)

#define pcs_cpu_port_cause_NODE_MAC(bit, unit)                                                          \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_MTI_10G_PCS_x1_0_E + unit, NULL,                                 \
        0x00000050, 0x00000070,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_IRONMAN_CPU_PCS_0_TO_3_##unit##_CPU_INSTANCES_1_PORT_NUM_0_LINK_STATUS_CHANGE_E,       \
        PRV_CPSS_IRONMAN_CPU_PCS_0_TO_3_##unit##_CPU_INSTANCES_1_PORT_NUM_0_LPCS_AN_DONE_INT_E,         \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define pcs_cpu_summary_SUB_TREE_MAC(bit, unit)                                                         \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_MTI_10G_PCS_x1_0_E + unit, NULL,                                 \
        0x00000014, 0x00000018,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_IRONMAN_CPU_PCS_0_TO_3_##unit##_CPU_INSTANCES_1_GLOBAL_INTERRUPT_SUMMARY_CAUSE_PORT_NUM_0_P_INT_SUM_E,       \
        PRV_CPSS_IRONMAN_CPU_PCS_0_TO_3_##unit##_CPU_INSTANCES_1_GLOBAL_INTERRUPT_SUMMARY_CAUSE_SD_NUM_1_SD_100FX_INT_SUM_E,  \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 1, NULL, NULL},                                         \
            pcs_cpu_port_cause_NODE_MAC(1, unit)

#define pds_SUB_TREE_MAC(bit)                                                                           \
    /* PDS Interrupt Summary */                                                                         \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_PDS0_E, NULL,                                          \
        0x00066000, 0x00066004,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_IRONMAN_TXQ_SCN_PDS_INTERRUPT_SUMMARY_CAUSE_PDS_FUNCTIONAL_INT_SUM_E,                  \
        PRV_CPSS_IRONMAN_TXQ_SCN_PDS_INTERRUPT_SUMMARY_CAUSE_PDS_DEBUG_INT_SUM_E,                       \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 2, NULL, NULL},                                         \
        /* PDS Functional Interrupt Summary */                                                          \
        {1, GT_FALSE, PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_PDS0_E, NULL,                                        \
            0x00066008, 0x0006600C,                                                                     \
            prvCpssDrvHwPpPortGroupIsrRead,                                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
            PRV_CPSS_IRONMAN_TXQ_SCN_PDS_INTERRUPT_FUNCTIONAL_CAUSE_PDS_BAD_ADDRESS_INT_E,              \
            PRV_CPSS_IRONMAN_TXQ_SCN_PDS_INTERRUPT_FUNCTIONAL_CAUSE_PDS_MEM_DOUBLE_ERROR_INT_E,         \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},                                     \
        /* PDS Debug Interrupt Summary */                                                               \
        {2, GT_FALSE, PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_PDS0_E, NULL,                                        \
            0x00066010, 0x00066014,                                                                     \
            prvCpssDrvHwPpPortGroupIsrRead,                                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
            PRV_CPSS_IRONMAN_TXQ_SCN_PDS_INTERRUPT_DEBUG_CAUSE_RESERVED_1_E,                            \
            PRV_CPSS_IRONMAN_TXQ_SCN_PDS_INTERRUPT_DEBUG_CAUSE_QUEUE_READ_RATE_EXCEEDED_INT_E,          \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define qfc_SUB_TREE_MAC(bit)                                                                           \
    /* QFC Interrupt Summary */                                                                         \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_QFC0_E, NULL,                                          \
        0x00000210, 0x00000214,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_IRONMAN_TXQ_SCN_QFC_INTERRUPT_SUMMARY_CAUSE_QFC_FUNCTIONAL_INT_SUM_E,                  \
        PRV_CPSS_IRONMAN_TXQ_SCN_QFC_INTERRUPT_SUMMARY_CAUSE_REGISTER_INDEX_18_INT_SUM_E,               \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 1, NULL, NULL},                                         \
        /* QFC Interrupt Summary */                                                                     \
        {1, GT_FALSE, PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_QFC0_E, NULL,                                        \
            0x00000200, 0x00000204,                                                                     \
            prvCpssDrvHwPpPortGroupIsrRead,                                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
            PRV_CPSS_IRONMAN_TXQ_SCN_QFC_INTERRUPT_FUNCTIONAL_CAUSE_UBURST_EVENT_INT_E,                 \
            PRV_CPSS_IRONMAN_TXQ_SCN_QFC_INTERRUPT_FUNCTIONAL_CAUSE_GLOBAL_TC_AVAILABLE_BUFFERS_LESS_THAN_USED_E, \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define sdq_SUB_TREE_MAC(bit)                                                                           \
    /* SDQ Interrupt Summary */                                                                         \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_SDQ0_E, NULL,                                          \
        0x00000200, 0x00000204,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_IRONMAN_TXQ_SCN_SDQ_INTERRUPT_SUMMARY_CAUSE_SDQ_FUNCTIONAL_INTERRUPT_SUMMARY_E,        \
        PRV_CPSS_IRONMAN_TXQ_SCN_SDQ_INTERRUPT_SUMMARY_CAUSE_SDQ_DEBUG_INTERRUPT_SUMMARY_E,             \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 2, NULL, NULL},                                         \
        /* SDQ Functional Interrupt Summary */                                                          \
        {1, GT_FALSE, PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_SDQ0_E, NULL,                                        \
            0x00000208, 0x0000020C,                                                                     \
            prvCpssDrvHwPpPortGroupIsrRead,                                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
            PRV_CPSS_IRONMAN_TXQ_SCN_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_ILLEGAL_ADDRESS_ACCESS_E,           \
            PRV_CPSS_IRONMAN_TXQ_SCN_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_SEL_LIST_PTRS_CONFIG_PORT_NOT_EMPTY_E, \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},                                     \
        /* SDQ Debug Interrupt Summary */                                                               \
        {2, GT_FALSE, PRV_CPSS_DXCH_UNIT_TXQ_PIPE0_SDQ0_E, NULL,                                        \
            0x00000210, 0x00000214,                                                                     \
            prvCpssDrvHwPpPortGroupIsrRead,                                                             \
            prvCpssDrvHwPpPortGroupIsrWrite,                                                            \
            PRV_CPSS_IRONMAN_TXQ_SCN_SDQ_INTERRUPT_DEBUG_CAUSE_QCN_FIFO_OVERRUN_E,                      \
            PRV_CPSS_IRONMAN_TXQ_SCN_SDQ_INTERRUPT_DEBUG_CAUSE_QCN_FIFO_OVERRUN_E,                      \
            FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define dfx_server_NODE_MAC(bit)                                                                        \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_DFX_SERVER_E, NULL,                                              \
        0x000F8108, 0x000F810C,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_IRONMAN_DFX_SERVER_INTERRUPT_WRONG_ADDRESS_FROM_PIPE_0_E,                              \
        PRV_CPSS_IRONMAN_DFX_SERVER_INTERRUPT_EXTERNAL_LOW_TEMPERATURE_THRESHOLD_E,                     \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define dfx_server_sum_SUB_TREE_MAC(bit)                                                                \
    {bit, GT_FALSE, PRV_CPSS_DXCH_UNIT_DFX_SERVER_E, NULL,                                              \
        0x000F8100, 0x000F8104,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_IRONMAN_DFX_SERVER_INTERRUPT_SUMMARY_SERVER_INTERRUPT_SUM_E,                           \
        PRV_CPSS_IRONMAN_DFX_SERVER_INTERRUPT_SUMMARY_PIPE_7_INTERRUPT_SUM_E,                           \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 1, NULL, NULL},                                         \
            dfx_server_NODE_MAC(1)

#define ironman_cnm_grp_0_0_IntsSum_SUB_TREE_MAC(bit, _mgIndex)                                         \
    /* FuncUnitsIntsSum Interrupt Cause */                                                              \
    {bit, GT_FALSE,MG_UNIT(PRV_CPSS_DXCH_UNIT_MG_E,_mgIndex), NULL,                                     \
        0x000003F8, 0x000003FC,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_IRONMAN_MG_##_mgIndex##_FUNCTIONAL_INTERRUPTS_SUMMARY_CAUSE_FUNC_UNITS_INT_SUM_1_E,    \
        PRV_CPSS_IRONMAN_MG_##_mgIndex##_FUNCTIONAL_INTERRUPTS_SUMMARY_CAUSE_FUNC_UNITS_INT_SUM_31_E,   \
        FILLED_IN_RUNTIME_CNS, 0, 0x0, 28, NULL, NULL},                                                 \
            mt_NODE_MAC(2),                                                                             \
            bma_NODE_MAC(3),                                                                            \
            cnc_SUB_TREE_MAC(4),                                                                        \
            eft_NODE_MAC(5),                                                                            \
            epcl_NODE_MAC(7),                                                                           \
            eplr_NODE_MAC(8),                                                                           \
            eq_SUB_TREE_MAC(9),                                                                         \
            erep_NODE_MAC(10),                                                                          \
            ermrk_NODE_MAC(11),                                                                         \
            ha_NODE_MAC(12),                                                                            \
            hbu_NODE_MAC(13),                                                                           \
            ioam_NODE_MAC(14),                                                                          \
            iplr0_NODE_MAC(15),                                                                         \
            iplr1_NODE_MAC(16),                                                                         \
            router_NODE_MAC(17),                                                                        \
            l2i_NODE_MAC(18),                                                                           \
            lpm_NODE_MAC(19),                                                                           \
            mll_NODE_MAC(20),                                                                           \
            pcl_NODE_MAC(21),                                                                           \
            preq_NODE_MAC(22),                                                                          \
            qag_NODE_MAC(23),                                                                           \
            sht_NODE_MAC(24),                                                                           \
            tti_NODE_MAC(26),                                                                           \
            tai_NODE_MAC(27, 0),                                                                        \
            tai_NODE_MAC(28, 1),                                                                        \
            tai_NODE_MAC(29, 2),                                                                        \
            tai_NODE_MAC(30, 3),                                                                        \
            tai_NODE_MAC(31, 4)

#define ironman_cnm_grp_0_1_IntsSum_SUB_TREE_MAC(bit, _mgIndex)                                         \
    /* DataPathIntsSum Interrupt Cause */                                                               \
    {bit, GT_FALSE, MG_UNIT(PRV_CPSS_DXCH_UNIT_MG_E,_mgIndex), NULL,                                    \
        0x000000A4, 0x000000A8,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_IRONMAN_MG_##_mgIndex##_DATA_PATH_INTERRUPTS_SUMMARY_CAUSE_DATA_PATH_INT_SUM1_E,       \
        PRV_CPSS_IRONMAN_MG_##_mgIndex##_DATA_PATH_INTERRUPTS_SUMMARY_CAUSE_DATA_PATH_INT_SUM31_E,      \
        FILLED_IN_RUNTIME_CNS, 0, 0x0, 25, NULL, NULL},                                                 \
            tcam_SUB_TREE_MAC(1),                                                                       \
            pb_SUB_TREE_MAC(2),                                                                         \
            rxDma_SUB_TREE_MAC(3),                                                                      \
            txDma_NODE_MAC(4),                                                                          \
            txFifo_NODE_MAC(5),                                                                         \
            gop_ctsu_SUB_TREE_MAC(6),                                                                   \
            gop_163_e_mac_sec_summary_SUB_TREE_MAC(7),                                                  \
            gop_163_i_mac_sec_summary_SUB_TREE_MAC(8),                                                  \
            gop_164_e_mac_sec_summary_SUB_TREE_MAC(9),                                                  \
            gop_164_i_mac_sec_summary_SUB_TREE_MAC(10),                                                 \
            gop_anp_0_summary_SUB_TREE_MAC(14, 0, 0),                                                   \
            gop_anp_0_summary_SUB_TREE_MAC(15, 1, 1),                                                   \
            gop_anp_0_summary_SUB_TREE_MAC(16, 6, 2),                                                   \
            gop_anp_0_summary_SUB_TREE_MAC(17, 7, 3),                                                   \
            gop_anp_summary_SUB_TREE_MAC(18, 0),                                                        \
            gop_led_NODE_MAC(19, 0),                                                                    \
            mac8br_summary_SUB_TREE_MAC (20, 0),                                                        \
            mac8_summary_SUB_TREE_MAC   (21, 0),                                                        \
            mac_summary_SUB_TREE_MAC    (22, 0),                                                        \
            gop_mti_10G_pcs_x8_0_SUB_TREE_MAC(26),                                                      \
            gop_mti_10G_pcs_x8_3_SUB_TREE_MAC(27),                                                      \
            pcs_summary_SUB_TREE_MAC(28, 0),                                                            \
            gop_sdw_interrupt_cause_NODE_MAC(29, 1),                                                    \
            gop_sdw_interrupt_cause_NODE_MAC(30, 2),                                                    \
            gop_sdw_interrupt_cause_NODE_MAC(31, 0)

#define ironman_cnm_grp_0_2_IntsSum_SUB_TREE_MAC(bit, _mgIndex)                                         \
    /* PortsIntsSum Interrupt Cause */                                                                  \
    {bit, GT_FALSE, MG_UNIT(PRV_CPSS_DXCH_UNIT_MG_E, _mgIndex), NULL,                                   \
        0x00000080, 0x00000084,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_IRONMAN_MG_##_mgIndex##_PORTS_INTERRUPTS_SUMMARY_CAUSE_PORT_INT_SUM_1_E,               \
        PRV_CPSS_IRONMAN_MG_##_mgIndex##_PORTS_INTERRUPTS_SUMMARY_CAUSE_PORT_INT_SUM_31_E,              \
        FILLED_IN_RUNTIME_CNS, 0, 0x0, 17, NULL, NULL},                                                 \
            gop_anp_1_summary_SUB_TREE_MAC(4, 2, 0),                                                    \
            gop_anp_1_summary_SUB_TREE_MAC(5, 3, 1),                                                    \
            gop_anp_1_summary_SUB_TREE_MAC(6, 8, 2),                                                    \
            gop_anp_1_summary_SUB_TREE_MAC(7, 9, 3),                                                    \
            gop_anp_summary_SUB_TREE_MAC(8, 1),                                                         \
            gop_led_NODE_MAC(9, 1),                                                                     \
            mac8br_summary_SUB_TREE_MAC (10, 1),                                                        \
            mac8_summary_SUB_TREE_MAC   (11, 1),                                                        \
            mac_summary_SUB_TREE_MAC    (12, 1),                                                        \
            gop_mti_10G_pcs_x8_1_SUB_TREE_MAC(16),                                                      \
            gop_mti_10G_pcs_x8_4_SUB_TREE_MAC(17),                                                      \
            pcs_summary_SUB_TREE_MAC(18, 1),                                                            \
            gop_sdw_interrupt_cause_NODE_MAC(19, 4),                                                    \
            gop_sdw_interrupt_cause_NODE_MAC(20, 5),                                                    \
            gop_sdw_interrupt_cause_NODE_MAC(21, 3),                                                    \
            lmu_cause_summary_SUB_TREE_MAC(27),                                                         \
            pizarb_bad_address_interrupt_cause_NODE_MAC(30)

#define ironman_cnm_grp_0_3_IntsSum_SUB_TREE_MAC(bit, _mgIndex)                                         \
    /* DFX */                                                                                           \
    {bit, GT_FALSE, MG_UNIT(PRV_CPSS_DXCH_UNIT_MG_E, _mgIndex), NULL,                                   \
        0x000000AC, 0x000000B0,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_IRONMAN_MG_##_mgIndex##_DFX_INTERRUPTS_SUMMARY_CAUSE_DFX_INTERRUPT_SUM_1_E,            \
        PRV_CPSS_IRONMAN_MG_##_mgIndex##_DFX_INTERRUPTS_SUMMARY_CAUSE_DFX_INTERRUPT_SUM_31_E,           \
        FILLED_IN_RUNTIME_CNS, 0, 0x0, 22, NULL, NULL},                                                 \
            gop_anp_2_summary_SUB_TREE_MAC(4,  4, 0),                                                   \
            gop_anp_2_summary_SUB_TREE_MAC(5,  5, 1),                                                   \
            gop_anp_2_summary_SUB_TREE_MAC(6, 10, 2),                                                   \
            gop_anp_2_summary_SUB_TREE_MAC(7, 11, 3),                                                   \
            gop_anp_summary_SUB_TREE_MAC(8, 2),                                                         \
            gop_led_NODE_MAC(9, 2),                                                                     \
            mac8br_summary_SUB_TREE_MAC (10, 2),                                                        \
            mac8_summary_SUB_TREE_MAC   (11, 2),                                                        \
            mac_summary_SUB_TREE_MAC    (12, 2),                                                        \
            gop_mti_10G_pcs_x8_2_SUB_TREE_MAC(16),                                                      \
            gop_mti_10G_pcs_x8_5_SUB_TREE_MAC(17),                                                      \
            pcs_summary_SUB_TREE_MAC(18, 2),                                                            \
            gop_sdw_interrupt_cause_NODE_MAC(19, 7),                                                    \
            gop_sdw_interrupt_cause_NODE_MAC(20, 8),                                                    \
            gop_sdw_interrupt_cause_NODE_MAC(21, 6),                                                    \
            dfx_server_sum_SUB_TREE_MAC(25),                                                            \
            pds_SUB_TREE_MAC(26),                                                                       \
            pdx_NODE_MAC(27),                                                                           \
            pfcc_NODE_MAC(28),                                                                          \
            psi_NODE_MAC(29),                                                                           \
            qfc_SUB_TREE_MAC(30),                                                                       \
            sdq_SUB_TREE_MAC(31)

#define ironman_cnm_grp_0_4_IntsSum_SUB_TREE_MAC(bit, _mgIndex)                                         \
    /* DFX1 */                                                                                          \
    {bit, GT_FALSE, MG_UNIT(PRV_CPSS_DXCH_UNIT_MG_E, _mgIndex), NULL,                                   \
        0x000000b8, 0x000000bc,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_IRONMAN_MG_##_mgIndex##_DFX_1_INTERRUPTS_SUMMARY_CAUSE_DFX_1_INTERRUPT_SUM_1_E,        \
        PRV_CPSS_IRONMAN_MG_##_mgIndex##_DFX_1_INTERRUPTS_SUMMARY_CAUSE_DFX_1_INTERRUPT_SUM_31_E,       \
        FILLED_IN_RUNTIME_CNS, 0, 0x0, 16, NULL, NULL},                                                 \
            gop_anp_cpu_summary_SUB_TREE_MAC(5, 0),                                                     \
            gop_anp_cpu_summary_SUB_TREE_MAC(6, 1),                                                     \
            gop_anp_cpu_summary_SUB_TREE_MAC(7, 2),                                                     \
            gop_anp_cpu_summary_SUB_TREE_MAC(8, 3),                                                     \
/*            gop_led_NODE_MAC(9, 3), */                                                                \
            cpu_mac_summary_SUB_TREE_MAC(10, 0),                                                        \
            cpu_mac_summary_SUB_TREE_MAC(11, 1),                                                        \
            cpu_mac_summary_SUB_TREE_MAC(12, 2),                                                        \
            cpu_mac_summary_SUB_TREE_MAC(13, 3),                                                        \
            pcs_cpu_summary_SUB_TREE_MAC(18, 0),                                                        \
            pcs_cpu_summary_SUB_TREE_MAC(19, 1),                                                        \
            pcs_cpu_summary_SUB_TREE_MAC(20, 2),                                                        \
            pcs_cpu_summary_SUB_TREE_MAC(21, 3),                                                        \
            gop_sdw_interrupt_cause_NODE_MAC(22, 9),                                                    \
            gop_sdw_interrupt_cause_NODE_MAC(23, 10),                                                   \
            gop_sdw_interrupt_cause_NODE_MAC(24, 11),                                                   \
            gop_sdw_interrupt_cause_NODE_MAC(25, 12)

#define ironman_cnm_grp_0_5_IntsSum_SUB_TREE_MAC(bitIndexInCaller, _mgIndex)                                     \
     /* FuncUnits1IntsSum */                                                                                     \
    {bitIndexInCaller, GT_FALSE, MG_UNIT(PRV_CPSS_DXCH_UNIT_MG_E,_mgIndex), NULL,                                \
        0x000003f4, 0x000003f0,                                                                                  \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                          \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                         \
        PRV_CPSS_IRONMAN_MG_##_mgIndex##_FUNCTIONAL_1_INTERRUPTS_SUMMARY_CAUSE_FUNCTIONAL_1_INTERRUPT_SUMMARY_E, \
        PRV_CPSS_IRONMAN_MG_##_mgIndex##_FUNCTIONAL_1_INTERRUPTS_SUMMARY_CAUSE_FUNC_UNITS_1_INT_SUM_31_E,        \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xffffffff, 2, NULL, NULL},                                                  \
        /*MG1*/                                                                                                  \
        IRONMAN_MG_INTERRUPTS_MG_1_MAC(ironman_MG1_int_sum),                                                     \
        /*MG2*/                                                                                                  \
        IRONMAN_MG_INTERRUPTS_MG_2_MAC(ironman_MG2_int_sum)

#define ironman_cnm_grp_0_8_IntsSum_SUB_TREE_MAC(bit, _mgIndex)                                                  \
/* MG Internal Interrupt Cause register */                                                                      \
    {bit, GT_FALSE,MG_UNIT(PRV_CPSS_DXCH_UNIT_MG_E, _mgIndex), NULL,                                             \
        0x0000009c, 0x000000a0,                                                                                  \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                          \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                         \
        PRV_CPSS_IRONMAN_MG_##_mgIndex##_MG1_INTERNAL_INTERRUPT_CAUSE_MISC_INT_SUM1_E,                              \
        PRV_CPSS_IRONMAN_MG_##_mgIndex##_MG1_INTERNAL_INTERRUPT_CAUSE_CM3_SRAM_OOR_INT_E,                        \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

#define  IRONMAN_MG_INTERRUPTS_MG_1_MAC(bitIndexInCaller)                           \
    /* Global Interrupt Cause */                                                    \
    MG_GLOBAL_SUMMARY_MAC(bitIndexInCaller, 2 /*numChild*/, 1/*mgId*/),             \
        /* MG internal Interrupt Cause */                                           \
        mgInternalIntsSum_SUB_TREE_MAC(7, 1 /*_mgIndex*/),                         \
        /* Misc1 Interrupt SUM bit */                                               \
        ironman_cnm_grp_0_8_IntsSum_SUB_TREE_MAC(18, 1 /*_mgIndex*/)

#define  IRONMAN_MG_INTERRUPTS_MG_2_MAC(bitIndexInCaller)                           \
    /* Global Interrupt Cause */                                                    \
    MG_GLOBAL_SUMMARY_MAC(bitIndexInCaller, 2 /*numChild*/, 2/*mgId*/),             \
        /* MG internal Interrupt Cause */                                           \
        mgInternalIntsSum_SUB_TREE_MAC(7, 2 /*_mgIndex*/),                         \
        /* Misc1 Interrupt SUM bit */                                               \
        ironman_cnm_grp_0_8_IntsSum_SUB_TREE_MAC(18, 2 /*_mgIndex*/)

#define ironman_MG1_int_sum 4
#define ironman_MG2_int_sum 7

/*  cnm_grp_0_6_IntsSum_SUB_TREE_MAC */
#define ironman_cnm_grp_0_6_IntsSum_SUB_TREE_MAC(bit, _mgIndex)                                         \
    /* Ports1IntSum */                                                                                  \
    {bit, GT_FALSE, MG_UNIT(PRV_CPSS_DXCH_UNIT_MG_E, _mgIndex), NULL,                                   \
        0x00000150, 0x00000154,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_IRONMAN_MG_##_mgIndex##_PORTS_1_INTERRUPTS_SUMMARY_CAUSE_PORT_1_INT_SUM_1_E,           \
        PRV_CPSS_IRONMAN_MG_##_mgIndex##_PORTS_1_INTERRUPTS_SUMMARY_CAUSE_PORT_1_INT_SUM_31_E,          \
        FILLED_IN_RUNTIME_CNS, 0, 0x0, 0, NULL, NULL}

/*  cnm_grp_0_7_IntsSum_single_tile_SUB_TREE_MAC */
#define ironman_cnm_grp_0_7_IntsSum_SUB_TREE_MAC(bit, _mgIndex)                                         \
    /* Ports2IntSum */                                                                                  \
    {bit, GT_FALSE, MG_UNIT(PRV_CPSS_DXCH_UNIT_MG_E, _mgIndex), NULL,                                   \
        0x00000158, 0x0000015C,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_IRONMAN_MG_##_mgIndex##_PORTS_2_INTERRUPTS_SUMMARY_CAUSE_PORT_2_INT_SUM_1_E,           \
        PRV_CPSS_IRONMAN_MG_##_mgIndex##_PORTS_2_INTERRUPTS_SUMMARY_CAUSE_PORT_2_INT_SUM_31_E,          \
        FILLED_IN_RUNTIME_CNS, 0, 0x0, 8, NULL, NULL},                                                  \
        /* 0..7, GPIO_0_31_Interrupt Cause 0 */                                                         \
        {1, GT_TRUE, 0, prvCpssDrvDxExMxRunitGppIsrCall,                                                \
                0x00018114,                                                                             \
                PRV_CPSS_DRV_PEX_INDICATION_IN_ADDR_MAC(0x00018118),                                    \
                prvCpssDrvHwPpPortGroupReadInternalPciReg,                                              \
                prvCpssDrvHwPpPortGroupWriteInternalPciReg,                                             \
                PRV_CPSS_IRONMAN_GPIO_REGFILE_GPIO_INTERRUPT_CAUSE_0_RESERVED_1_E,                      \
                PRV_CPSS_IRONMAN_GPIO_REGFILE_GPIO_INTERRUPT_CAUSE_0_RESERVED_7_E,                      \
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},                                 \
                                                                                                        \
        /* 8..15, GPIO_0_31_Interrupt Cause 0 */                                                        \
        {2, GT_TRUE, 8, prvCpssDrvDxExMxRunitGppIsrCall,                                                \
                0x00018114,                                                                             \
                PRV_CPSS_DRV_PEX_INDICATION_IN_ADDR_MAC(0x00018118),                                    \
                prvCpssDrvHwPpPortGroupReadInternalPciReg,                                              \
                prvCpssDrvHwPpPortGroupWriteInternalPciReg,                                             \
                PRV_CPSS_IRONMAN_GPIO_REGFILE_GPIO_INTERRUPT_CAUSE_0_RESERVED_8_E,                      \
                PRV_CPSS_IRONMAN_GPIO_REGFILE_GPIO_INTERRUPT_CAUSE_0_RESERVED_15_E,                     \
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},                                 \
                                                                                                        \
        /* 16..23, GPIO_0_31_Interrupt Cause 0 */                                                       \
        {3, GT_TRUE, 16, prvCpssDrvDxExMxRunitGppIsrCall,                                               \
                0x00018114,                                                                             \
                PRV_CPSS_DRV_PEX_INDICATION_IN_ADDR_MAC(0x00018118),                                    \
                prvCpssDrvHwPpPortGroupReadInternalPciReg,                                              \
                prvCpssDrvHwPpPortGroupWriteInternalPciReg,                                             \
                PRV_CPSS_IRONMAN_GPIO_REGFILE_GPIO_INTERRUPT_CAUSE_0_RESERVED_16_E,                     \
                PRV_CPSS_IRONMAN_GPIO_REGFILE_GPIO_INTERRUPT_CAUSE_0_RESERVED_23_E,                     \
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},                                 \
                                                                                                        \
        /* 24..31, GPIO_0_31_Interrupt Cause 0 */                                                       \
        {4, GT_TRUE, 24, prvCpssDrvDxExMxRunitGppIsrCall,                                               \
                0x00018114,                                                                             \
                PRV_CPSS_DRV_PEX_INDICATION_IN_ADDR_MAC(0x00018118),                                    \
                prvCpssDrvHwPpPortGroupReadInternalPciReg,                                              \
                prvCpssDrvHwPpPortGroupWriteInternalPciReg,                                             \
                PRV_CPSS_IRONMAN_GPIO_REGFILE_GPIO_INTERRUPT_CAUSE_0_RESERVED_24_E,                     \
                PRV_CPSS_IRONMAN_GPIO_REGFILE_GPIO_INTERRUPT_CAUSE_0_RESERVED_31_E,                     \
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},                                 \
                                                                                                        \
        /* 32..39, GPIO_0_31_Interrupt Cause 1 */                                                       \
        {5, GT_TRUE, 32, prvCpssDrvDxExMxRunitGppIsrCall,                                               \
                0x00018154,                                                                             \
                PRV_CPSS_DRV_PEX_INDICATION_IN_ADDR_MAC(0x00018158),                                    \
                prvCpssDrvHwPpPortGroupReadInternalPciReg,                                              \
                prvCpssDrvHwPpPortGroupWriteInternalPciReg,                                             \
                PRV_CPSS_IRONMAN_GPIO_REGFILE_GPIO_INTERRUPT_CAUSE_1_RESERVED_1_E,                      \
                PRV_CPSS_IRONMAN_GPIO_REGFILE_GPIO_INTERRUPT_CAUSE_1_RESERVED_7_E,                      \
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},                                 \
                                                                                                        \
        /* 40..47, GPIO_0_31_Interrupt Cause 1 */                                                       \
        {6, GT_TRUE, 40, prvCpssDrvDxExMxRunitGppIsrCall,                                               \
                0x00018154,                                                                             \
                PRV_CPSS_DRV_PEX_INDICATION_IN_ADDR_MAC(0x00018158),                                    \
                prvCpssDrvHwPpPortGroupReadInternalPciReg,                                              \
                prvCpssDrvHwPpPortGroupWriteInternalPciReg,                                             \
                PRV_CPSS_IRONMAN_GPIO_REGFILE_GPIO_INTERRUPT_CAUSE_1_RESERVED_8_E,                      \
                PRV_CPSS_IRONMAN_GPIO_REGFILE_GPIO_INTERRUPT_CAUSE_1_RESERVED_15_E,                     \
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},                                 \
                                                                                                        \
        /* 55..48, GPIO_0_31_Interrupt Cause 1 */                                                       \
        {7, GT_TRUE, 55, prvCpssDrvDxExMxRunitGppIsrCall,                                               \
                0x00018114,                                                                             \
                PRV_CPSS_DRV_PEX_INDICATION_IN_ADDR_MAC(0x00018158),                                    \
                prvCpssDrvHwPpPortGroupReadInternalPciReg,                                              \
                prvCpssDrvHwPpPortGroupWriteInternalPciReg,                                             \
                PRV_CPSS_IRONMAN_GPIO_REGFILE_GPIO_INTERRUPT_CAUSE_1_RESERVED_16_E,                     \
                PRV_CPSS_IRONMAN_GPIO_REGFILE_GPIO_INTERRUPT_CAUSE_1_RESERVED_23_E,                     \
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL},                                 \
                                                                                                        \
        /* 56..63, GPIO_0_31_Interrupt Cause 1 */                                                       \
        {8, GT_TRUE, 56, prvCpssDrvDxExMxRunitGppIsrCall,                                               \
                0x00018114,                                                                             \
                PRV_CPSS_DRV_PEX_INDICATION_IN_ADDR_MAC(0x00018158),                                    \
                prvCpssDrvHwPpPortGroupReadInternalPciReg,                                              \
                prvCpssDrvHwPpPortGroupWriteInternalPciReg,                                             \
                PRV_CPSS_IRONMAN_GPIO_REGFILE_GPIO_INTERRUPT_CAUSE_1_RESERVED_24_E,                     \
                PRV_CPSS_IRONMAN_GPIO_REGFILE_GPIO_INTERRUPT_CAUSE_1_RESERVED_31_E,                     \
                FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}


#define MG_GLOBAL_SUMMARY_MAC(bitIndexInCaller, numChild, _mgIndex)                                     \
    /* Ironman Global Interrupt Cause */                                                                \
    {bitIndexInCaller, GT_FALSE, MG_UNIT(PRV_CPSS_DXCH_UNIT_MG_E,_mgIndex), NULL,                       \
        0x00000030, 0x00000034,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_IRONMAN_MG_##_mgIndex##_GLOBAL_INTERRUPTS_SUMMARY_CAUSE_PEX_INT_SUM_E,                 \
        PRV_CPSS_IRONMAN_MG_##_mgIndex##_GLOBAL_INTERRUPTS_SUMMARY_CAUSE_MG1_INTERNAL_INT_SUM_E,        \
        FILLED_IN_RUNTIME_CNS, 0, 0xffffffff, numChild, NULL, NULL}


#define IRONMAN_MG_Tx_SDMA_SUMMARY_MAC(bitIndexInCaller, _mgIndex)                                      \
    /* Tx SDMA  */                                                                                      \
    {bitIndexInCaller, GT_FALSE,MG_UNIT(PRV_CPSS_DXCH_UNIT_MG_E,_mgIndex), NULL,                        \
        0x00002810, 0x00002818,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_IRONMAN_MG_##_mgIndex##_TRANSMIT_SDMA_INTERRUPT_CAUSE0_TX_BUFFER_QUEUE_0_E,            \
        PRV_CPSS_IRONMAN_MG_##_mgIndex##_TRANSMIT_SDMA_INTERRUPT_CAUSE0_TX_REJECT_0_E,                  \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0,NULL, NULL}


#define IRONMAN_MG_Rx_SDMA_SUMMARY_MAC(bitIndexInCaller, _mgIndex)                                      \
    /* Rx SDMA  */                                                                                      \
    {bitIndexInCaller, GT_FALSE,MG_UNIT(PRV_CPSS_DXCH_UNIT_MG_E,_mgIndex), NULL,                        \
        0x0000280C, 0x00002814,                                                                         \
        prvCpssDrvHwPpPortGroupIsrRead,                                                                 \
        prvCpssDrvHwPpPortGroupIsrWrite,                                                                \
        PRV_CPSS_IRONMAN_MG_##_mgIndex##_RECEIVE_SDMA_INTERRUPT_CAUSE0_RX_BUFFER_QUEUE_0_E,             \
        PRV_CPSS_IRONMAN_MG_##_mgIndex##_RECEIVE_SDMA_INTERRUPT_CAUSE0_PACKET_CNT_OF_E,                 \
        FILLED_IN_RUNTIME_CNS, 0x0, 0xFFFFFFFF, 0, NULL, NULL}

static const PRV_CPSS_DRV_INTERRUPT_SCAN_STC ironmanIntrScanArr[] =
{
    /* Global Interrupt Cause */
    MG_GLOBAL_SUMMARY_MAC(0, 10/*numChild*/, 0 /*_mgIndex*/),
        /* FuncUnitsIntsSum Interrupt Cause */
        ironman_cnm_grp_0_0_IntsSum_SUB_TREE_MAC(3, 0 /*_mgIndex*/),
        /* Data Path Interrupt Summay */
        ironman_cnm_grp_0_1_IntsSum_SUB_TREE_MAC(4, 0/*_mgIndex*/),
        /* Ports Summary Interrupt Cause */
        ironman_cnm_grp_0_2_IntsSum_SUB_TREE_MAC(5, 0 /*_mgIndex */),
        /* DFX interrupts summary cause */
        ironman_cnm_grp_0_3_IntsSum_SUB_TREE_MAC(6, 0 /*_mgIndex */),
        /* MG internal Interrupt Cause */
        mg0InternalIntsSum_SUB_TREE_MAC(7, 0 /*_mgIndex */),
        /* Tx SDMA  */
        IRONMAN_MG_Tx_SDMA_SUMMARY_MAC(8, 0 /*_mgIndex*/),
        /* Rx SDMA  */
        IRONMAN_MG_Rx_SDMA_SUMMARY_MAC(9, 0 /*_mgIndex*/),
        /* DFX1 Interrupt Summary */
        ironman_cnm_grp_0_4_IntsSum_SUB_TREE_MAC(10, 0 /*_mgIndex*/),
        /* FuncUnits1IntsSum */
        ironman_cnm_grp_0_5_IntsSum_SUB_TREE_MAC(11, 0 /*_mgIndex*/),
        /* MG1 Internal Interrupt Cause register summary bit */
        ironman_cnm_grp_0_8_IntsSum_SUB_TREE_MAC(18, 0 /*_mgIndex*/)
};

#define IRONMAN_NUM_MASK_REGISTERS_CNS  (PRV_CPSS_IRONMAN_LAST_INT_E / 32)

/* _postFix - must include the "_E" */
#define SET_EVENT_PER_REG_INDEX_PER_PORT_MAC(_preFix, _postFix, _regIndex, _port)     \
    PRV_CPSS_IRONMAN_##_preFix##_##_regIndex##_PORT_##_port##_##_postFix, (((_regIndex) * 16) + _port)

/* _postFix - must include the "_E" */
#define SET_EVENT_PER_REG_PORTS_MAC(_preFix, _regIndex, _postFix)                 \
    SET_EVENT_PER_REG_INDEX_PER_PORT_MAC(_preFix, _postFix, _regIndex, 0),        \
    SET_EVENT_PER_REG_INDEX_PER_PORT_MAC(_preFix, _postFix, _regIndex, 1),        \
    SET_EVENT_PER_REG_INDEX_PER_PORT_MAC(_preFix, _postFix, _regIndex, 2),        \
    SET_EVENT_PER_REG_INDEX_PER_PORT_MAC(_preFix, _postFix, _regIndex, 3),        \
    SET_EVENT_PER_REG_INDEX_PER_PORT_MAC(_preFix, _postFix, _regIndex, 4),        \
    SET_EVENT_PER_REG_INDEX_PER_PORT_MAC(_preFix, _postFix, _regIndex, 5),        \
    SET_EVENT_PER_REG_INDEX_PER_PORT_MAC(_preFix, _postFix, _regIndex, 6),        \
    SET_EVENT_PER_REG_INDEX_PER_PORT_MAC(_preFix, _postFix, _regIndex, 7),        \
    SET_EVENT_PER_REG_INDEX_PER_PORT_MAC(_preFix, _postFix, _regIndex, 8),        \
    SET_EVENT_PER_REG_INDEX_PER_PORT_MAC(_preFix, _postFix, _regIndex, 9),        \
    SET_EVENT_PER_REG_INDEX_PER_PORT_MAC(_preFix, _postFix, _regIndex, 10),       \
    SET_EVENT_PER_REG_INDEX_PER_PORT_MAC(_preFix, _postFix, _regIndex, 11),       \
    SET_EVENT_PER_REG_INDEX_PER_PORT_MAC(_preFix, _postFix, _regIndex, 12),       \
    SET_EVENT_PER_REG_INDEX_PER_PORT_MAC(_preFix, _postFix, _regIndex, 13),       \
    SET_EVENT_PER_REG_INDEX_PER_PORT_MAC(_preFix, _postFix, _regIndex, 14),       \
    SET_EVENT_PER_REG_INDEX_PER_PORT_MAC(_preFix, _postFix, _regIndex, 15)


#define SET_EVENT_PER_ALL_PORTS_MAC(_preFix, _postFix)                            \
    SET_EVENT_PER_REG_PORTS_MAC(_preFix,     1,  _postFix),                       \
    SET_EVENT_PER_REG_PORTS_MAC(_preFix,     2,  _postFix),                       \
    SET_EVENT_PER_REG_PORTS_MAC(_preFix,     3,  _postFix),                       \
    SET_EVENT_PER_REG_PORTS_MAC(_preFix,     4,  _postFix),                       \
    SET_EVENT_PER_REG_PORTS_MAC(_preFix,     5,  _postFix),                       \
    SET_EVENT_PER_REG_PORTS_MAC(_preFix,     6,  _postFix),                       \
    SET_EVENT_PER_REG_PORTS_MAC(_preFix,     7,  _postFix),                       \
    SET_EVENT_PER_REG_PORTS_MAC(_preFix,     8,  _postFix),                       \
    SET_EVENT_PER_REG_PORTS_MAC(_preFix,     9,  _postFix),                       \
    SET_EVENT_PER_REG_PORTS_MAC(_preFix,    10,  _postFix),                       \
    SET_EVENT_PER_REG_PORTS_MAC(_preFix,    11,  _postFix),                       \
    SET_EVENT_PER_REG_PORTS_MAC(_preFix,    12,  _postFix),                       \
    SET_EVENT_PER_REG_PORTS_MAC(_preFix,    13,  _postFix),                       \
    SET_EVENT_PER_REG_PORTS_MAC(_preFix,    14,  _postFix),                       \
    SET_EVENT_PER_REG_PORTS_MAC(_preFix,    15,  _postFix),                       \
    SET_EVENT_PER_REG_PORTS_MAC(_preFix,    16,  _postFix)

/* _postFix - must include the "_E" */
#define SET_TAI_UNIT_EVENT_WITH_INDEX_MAC(_postFix, unit)                        \
    PRV_CPSS_IRONMAN_TAI_##unit##_##_postFix,  unit

/* _postFix - must include the "_E" */
#define SET_EQUAL_EVENT_WITH_POSTFIX_MAC(_postFix)                                \
    PRV_CPSS_IRONMAN_##_postFix, PRV_CPSS_IRONMAN_##_postFix


/* _postFix - must include the "_E" */
#define SET_EVENT_WITH_INDEX_MAC(_postFix, _index)                                \
    PRV_CPSS_IRONMAN_##_postFix,    _index

/* _postFix - must include the "_E" */
#define SET_EVENT_SOURCE_PER_DIRECTION_ALL_DPS_MAC(_postFix, _prefix, _direction, _source, _inst)           \
    PRV_CPSS_IRONMAN_MSEC_##_prefix##_##_inst##_##_postFix,            (_source << 8 |(_direction))

/* _postFix - must include the "_E" */
#define IRONMAN_SET_EIP_164_EVENT_SOURCE_PER_ALL_DIRECTIONS_ALL_DP_MAC(_postFix, _source)  \
    SET_EVENT_SOURCE_PER_DIRECTION_ALL_DPS_MAC(_postFix, EGR_164_INST, 0, _source, 1),     \
    SET_EVENT_SOURCE_PER_DIRECTION_ALL_DPS_MAC(_postFix, ING_164_INST, 1, _source, 2)

/* _postFix - must include the "_E" */
#define IRONMAN_SET_EIP_163_EVENT_SOURCE_PER_ALL_DIRECTIONS_ALL_DP_MAC(_postFix, _source)  \
    SET_EVENT_SOURCE_PER_DIRECTION_ALL_DPS_MAC(_postFix, EGR_163_INST, 0, _source, 0),     \
    SET_EVENT_SOURCE_PER_DIRECTION_ALL_DPS_MAC(_postFix, ING_163_INST, 1, _source, 1)

/* _postFix - must include the "_E" */
#define SET_EVENT_PER_DIRECTION_ALL_DPS_MAC(_postFix, _prefix, _direction, _inst)       \
    PRV_CPSS_IRONMAN_MSEC_##_prefix##_##_inst##_##_postFix,           (_direction)

/* _postFix - must include the "_E" */
#define IRONMAN_SET_EIP_164_EVENT_PER_EGRESS_DIRECTION_ALL_DP_MAC(_postFix)  \
    SET_EVENT_PER_DIRECTION_ALL_DPS_MAC(_postFix, EGR_164_INST, 0, 1)

/* _postFix - must include the "_E" */
#define IRONMAN_SET_EIP_164_EVENT_SOURCE_PER_INGRESS_DIRECTION_ALL_DP_MAC(_postFix, _source)  \
    SET_EVENT_SOURCE_PER_DIRECTION_ALL_DPS_MAC(_postFix, ING_164_INST, 1, _source, 2)

/* _postFix - must include the "_E" */
#define IRONMAN_SET_EIP_66_EVENT_PER_EGRESS_DIRECTION_ALL_DP_MAC(_postFix)  \
    SET_EVENT_PER_DIRECTION_ALL_DPS_MAC(_postFix, EGR_66_INST, 0, 1)

/* _postFix - must include the "_E" */
#define IRONMAN_SET_EIP_164_EVENT_PER_ALL_DIRECTIONS_ALL_DP_MAC(_postFix)  \
    SET_EVENT_PER_DIRECTION_ALL_DPS_MAC(_postFix, EGR_164_INST, 0, 1),      \
    SET_EVENT_PER_DIRECTION_ALL_DPS_MAC(_postFix, ING_164_INST, 1, 2)

#define SET_EVENT_ALL_DIRECTIONS_ALL_DP_MAC(_prefix, _index, _direction, _inst)            \
    PRV_CPSS_IRONMAN_MSEC_##_prefix##_##_inst##_GLOBAL_MAC_SEC_INTERRUPT_CAUSE_ERROR_NOTIFICATION_NUMBER_##_index##_E,  (_index << 8 |(_direction))

#define IRONMAN_SET_EIP_66_ERROR_EVENT_PER_ALL_DIRECTIONS_ALL_DP_MAC(_index)    \
    SET_EVENT_ALL_DIRECTIONS_ALL_DP_MAC(EGR_66_INST, _index, 0, 1),             \
    SET_EVENT_ALL_DIRECTIONS_ALL_DP_MAC(ING_66_INST, _index, 1, 2)

/* _postFix - must include the "_E" */
#define SET_INDEX_EVENT_PER_8_PCS_8_PORTS_MAC(_postFix, _inst, _index)             \
    PRV_CPSS_IRONMAN_INST_##_inst##_##_inst##_PCS8_0_TO_1_##_index##_PORT_NUM_0_##_postFix, (((_inst) * 16) + ((_index) * 8)) +  0, \
    PRV_CPSS_IRONMAN_INST_##_inst##_##_inst##_PCS8_0_TO_1_##_index##_PORT_NUM_1_##_postFix, (((_inst) * 16) + ((_index) * 8)) +  1, \
    PRV_CPSS_IRONMAN_INST_##_inst##_##_inst##_PCS8_0_TO_1_##_index##_PORT_NUM_2_##_postFix, (((_inst) * 16) + ((_index) * 8)) +  2, \
    PRV_CPSS_IRONMAN_INST_##_inst##_##_inst##_PCS8_0_TO_1_##_index##_PORT_NUM_3_##_postFix, (((_inst) * 16) + ((_index) * 8)) +  3, \
    PRV_CPSS_IRONMAN_INST_##_inst##_##_inst##_PCS8_0_TO_1_##_index##_PORT_NUM_4_##_postFix, (((_inst) * 16) + ((_index) * 8)) +  4, \
    PRV_CPSS_IRONMAN_INST_##_inst##_##_inst##_PCS8_0_TO_1_##_index##_PORT_NUM_5_##_postFix, (((_inst) * 16) + ((_index) * 8)) +  5, \
    PRV_CPSS_IRONMAN_INST_##_inst##_##_inst##_PCS8_0_TO_1_##_index##_PORT_NUM_6_##_postFix, (((_inst) * 16) + ((_index) * 8)) +  6, \
    PRV_CPSS_IRONMAN_INST_##_inst##_##_inst##_PCS8_0_TO_1_##_index##_PORT_NUM_7_##_postFix, (((_inst) * 16) + ((_index) * 8)) +  7

/* state for ports 0..47 that event hold extra port number */
#define SET_INDEX_EQUAL_EVENT_PER_48_PCS8_PORTS_MAC(_postFix)               \
    SET_INDEX_EVENT_PER_8_PCS_8_PORTS_MAC(_postFix, 0, 0),                  \
    SET_INDEX_EVENT_PER_8_PCS_8_PORTS_MAC(_postFix, 0, 1),                  \
    SET_INDEX_EVENT_PER_8_PCS_8_PORTS_MAC(_postFix, 1, 0),                  \
    SET_INDEX_EVENT_PER_8_PCS_8_PORTS_MAC(_postFix, 1, 1),                  \
    SET_INDEX_EVENT_PER_8_PCS_8_PORTS_MAC(_postFix, 2, 0),                  \
    SET_INDEX_EVENT_PER_8_PCS_8_PORTS_MAC(_postFix, 2, 1)

/* state for PCS 0..2 that event hold extra port number */
#define SET_INDEX_EQUAL_EVENT_PER_3_PCS_PORTS_MAC(_postFix)                 \
    PRV_CPSS_IRONMAN_PCS_0_NO_CPU_0_PORT_NUM_0_##_postFix,    52,            \
    PRV_CPSS_IRONMAN_PCS_1_NO_CPU_0_PORT_NUM_0_##_postFix,    53,            \
    PRV_CPSS_IRONMAN_PCS_2_NO_CPU_0_PORT_NUM_0_##_postFix,    54

/* state for CPU PCS 0..3 that event hold extra port number */
#define SET_INDEX_EQUAL_EVENT_PER_4_CPU_PCS_PORTS_MAC(_postFix)                             \
    PRV_CPSS_IRONMAN_CPU_PCS_0_TO_3_0_CPU_INSTANCES_1_PORT_NUM_0_##_postFix,    48,          \
    PRV_CPSS_IRONMAN_CPU_PCS_0_TO_3_1_CPU_INSTANCES_1_PORT_NUM_0_##_postFix,    49,          \
    PRV_CPSS_IRONMAN_CPU_PCS_0_TO_3_2_CPU_INSTANCES_1_PORT_NUM_0_##_postFix,    50,          \
    PRV_CPSS_IRONMAN_CPU_PCS_0_TO_3_3_CPU_INSTANCES_1_PORT_NUM_0_##_postFix,    51

/* _postFix - must include the "_E" */
#define SET_INDEX_EVENT_PER_PER_8_MAC_8BR_PORTS_MAC(_postFix, _inst)                \
    PRV_CPSS_IRONMAN_MAC8BR_##_inst##_PORT_NUM_0_PORT_INTERRUPT_CAUSE_##_postFix, ((_inst) * 8)  + 0, \
    PRV_CPSS_IRONMAN_MAC8BR_##_inst##_PORT_NUM_1_PORT_INTERRUPT_CAUSE_##_postFix, ((_inst) * 8)  + 1, \
    PRV_CPSS_IRONMAN_MAC8BR_##_inst##_PORT_NUM_2_PORT_INTERRUPT_CAUSE_##_postFix, ((_inst) * 8)  + 2, \
    PRV_CPSS_IRONMAN_MAC8BR_##_inst##_PORT_NUM_3_PORT_INTERRUPT_CAUSE_##_postFix, ((_inst) * 8)  + 3, \
    PRV_CPSS_IRONMAN_MAC8BR_##_inst##_PORT_NUM_4_PORT_INTERRUPT_CAUSE_##_postFix, ((_inst) * 8)  + 4, \
    PRV_CPSS_IRONMAN_MAC8BR_##_inst##_PORT_NUM_5_PORT_INTERRUPT_CAUSE_##_postFix, ((_inst) * 8)  + 5, \
    PRV_CPSS_IRONMAN_MAC8BR_##_inst##_PORT_NUM_6_PORT_INTERRUPT_CAUSE_##_postFix, ((_inst) * 8)  + 6, \
    PRV_CPSS_IRONMAN_MAC8BR_##_inst##_PORT_NUM_7_PORT_INTERRUPT_CAUSE_##_postFix, ((_inst) * 8)  + 7

/* state for mac 0..2 that event hold extra port number */
#define SET_INDEX_EQUAL_EVENT_PER_3_MAC_PORTS_MAC(_postFix)                         \
    PRV_CPSS_IRONMAN_MAC_0_PORT_NUM_0_PORT_INTERRUPT_CAUSE_##_postFix,    52,       \
    PRV_CPSS_IRONMAN_MAC_1_PORT_NUM_0_PORT_INTERRUPT_CAUSE_##_postFix,    53,       \
    PRV_CPSS_IRONMAN_MAC_2_PORT_NUM_0_PORT_INTERRUPT_CAUSE_##_postFix,    54

/* state for ports 0..23 that event hold extra port number */
#define SET_INDEX_EQUAL_EVENT_PER_24_MAC_8BR_PORTS_MAC(_postFix)                    \
    SET_INDEX_EVENT_PER_PER_8_MAC_8BR_PORTS_MAC(_postFix, 0),                       \
    SET_INDEX_EVENT_PER_PER_8_MAC_8BR_PORTS_MAC(_postFix, 1),                       \
    SET_INDEX_EVENT_PER_PER_8_MAC_8BR_PORTS_MAC(_postFix, 2)

/* _postFix - must include the "_E" */
#define SET_INDEX_EVENT_PER_PER_8_MAC_8_PORTS_MAC(_postFix, _inst)                \
    PRV_CPSS_IRONMAN_MAC8_##_inst##_PORT_NUM_0_PORT_INTERRUPT_CAUSE_##_postFix, ((_inst) * 8)  +  24, \
    PRV_CPSS_IRONMAN_MAC8_##_inst##_PORT_NUM_1_PORT_INTERRUPT_CAUSE_##_postFix, ((_inst) * 8)  +  25, \
    PRV_CPSS_IRONMAN_MAC8_##_inst##_PORT_NUM_2_PORT_INTERRUPT_CAUSE_##_postFix, ((_inst) * 8)  +  26, \
    PRV_CPSS_IRONMAN_MAC8_##_inst##_PORT_NUM_3_PORT_INTERRUPT_CAUSE_##_postFix, ((_inst) * 8)  +  27, \
    PRV_CPSS_IRONMAN_MAC8_##_inst##_PORT_NUM_4_PORT_INTERRUPT_CAUSE_##_postFix, ((_inst) * 8)  +  28, \
    PRV_CPSS_IRONMAN_MAC8_##_inst##_PORT_NUM_5_PORT_INTERRUPT_CAUSE_##_postFix, ((_inst) * 8)  +  29, \
    PRV_CPSS_IRONMAN_MAC8_##_inst##_PORT_NUM_6_PORT_INTERRUPT_CAUSE_##_postFix, ((_inst) * 8)  +  30, \
    PRV_CPSS_IRONMAN_MAC8_##_inst##_PORT_NUM_7_PORT_INTERRUPT_CAUSE_##_postFix, ((_inst) * 8)  +  31

/* state for ports 0..23 that event hold extra port number */
#define SET_INDEX_EQUAL_EVENT_PER_24_MAC_8_PORTS_MAC(_postFix)              \
    SET_INDEX_EVENT_PER_PER_8_MAC_8_PORTS_MAC(_postFix, 0),                 \
    SET_INDEX_EVENT_PER_PER_8_MAC_8_PORTS_MAC(_postFix, 1),                 \
    SET_INDEX_EVENT_PER_PER_8_MAC_8_PORTS_MAC(_postFix, 2)

/* state for mac 0..2 that event hold extra port number */
#define SET_INDEX_EQUAL_EVENT_PER_4_MAC_BR_CPU_PORTS_MAC(_postFix)                          \
    PRV_CPSS_IRONMAN_CPU_MAC_BR_0_PORT_NUM_0_PORT_INTERRUPT_CAUSE_##_postFix,    48,        \
    PRV_CPSS_IRONMAN_CPU_MAC_BR_1_PORT_NUM_0_PORT_INTERRUPT_CAUSE_##_postFix,    49,        \
    PRV_CPSS_IRONMAN_CPU_MAC_BR_2_PORT_NUM_0_PORT_INTERRUPT_CAUSE_##_postFix,    50,        \
    PRV_CPSS_IRONMAN_CPU_MAC_BR_3_PORT_NUM_0_PORT_INTERRUPT_CAUSE_##_postFix,    51

#define PORTS_LINK_STATUS_CHANGE_EVENTS \
    /* USX total ports 0..47 */                                                             \
    SET_INDEX_EQUAL_EVENT_PER_48_PCS8_PORTS_MAC(LINK_STATUS_CHANGE_E),                      \
    SET_INDEX_EQUAL_EVENT_PER_48_PCS8_PORTS_MAC(LPCS_LINK_STATUS_CHANGE_E),                 \
    SET_INDEX_EQUAL_EVENT_PER_3_PCS_PORTS_MAC(LINK_STATUS_CHANGE_E),                        \
    SET_INDEX_EQUAL_EVENT_PER_3_PCS_PORTS_MAC(LPCS_LINK_STATUS_CHANGE_E),                   \
    SET_INDEX_EQUAL_EVENT_PER_4_CPU_PCS_PORTS_MAC(LINK_STATUS_CHANGE_E),                    \
    SET_INDEX_EQUAL_EVENT_PER_4_CPU_PCS_PORTS_MAC(LPCS_LINK_STATUS_CHANGE_E)

#define PORTS_LINK_OK_CHANGE_EVENTS \
    SET_INDEX_EQUAL_EVENT_PER_3_MAC_PORTS_MAC(LINK_OK_CHANGE_E),                            \
    SET_INDEX_EQUAL_EVENT_PER_24_MAC_8BR_PORTS_MAC(LINK_OK_CHANGE_E),                       \
    SET_INDEX_EQUAL_EVENT_PER_24_MAC_8_PORTS_MAC(LINK_OK_CHANGE_E),                         \
    SET_INDEX_EQUAL_EVENT_PER_4_MAC_BR_CPU_PORTS_MAC(LINK_OK_CHANGE_E)

#define IRONMAN_PORT_ANP_EVENTS(_inst, _postfix)                                                        \
    PRV_CPSS_IRONMAN_INST_##_inst##_##_inst##_ANP_0_PORT_NUM_0_##_postfix,     ((_inst) * 16) +  0,     \
    PRV_CPSS_IRONMAN_INST_##_inst##_##_inst##_ANP_1_PORT_NUM_0_##_postfix,     ((_inst) * 16) +  4,     \
    PRV_CPSS_IRONMAN_INST_##_inst##_##_inst##_ANP_2_PORT_NUM_0_##_postfix,     ((_inst) * 16) +  8,     \
    PRV_CPSS_IRONMAN_INST_##_inst##_##_inst##_ANP_3_PORT_NUM_0_##_postfix,     ((_inst) * 16) + 12,     \
    PRV_CPSS_IRONMAN_INST_##_inst##_##_inst##_ANP_4_PORT_NUM_0_##_postfix,     ((_inst) + 52)

#define IRONMAN_CPU_PORT_ANP_EVENTS(_index, _postfix)                                           \
    PRV_CPSS_IRONMAN_CPU_ANP_##_index##_CPU_5_PORT_NUM_0_##_postfix,           (48 + _index)

#define IRONMAN_PORT_AN_HCD_RESOLUTION_DONE_EVENTS                                              \
    IRONMAN_PORT_ANP_EVENTS(0, HCD_FOUND_E),                                                    \
    IRONMAN_PORT_ANP_EVENTS(1, HCD_FOUND_E),                                                    \
    IRONMAN_PORT_ANP_EVENTS(2, HCD_FOUND_E),                                                    \
    IRONMAN_CPU_PORT_ANP_EVENTS(0, HCD_FOUND_E),                                                \
    IRONMAN_CPU_PORT_ANP_EVENTS(1, HCD_FOUND_E),                                                \
    IRONMAN_CPU_PORT_ANP_EVENTS(2, HCD_FOUND_E),                                                \
    IRONMAN_CPU_PORT_ANP_EVENTS(3, HCD_FOUND_E)

#define IRONAMN_PORT_AN_RESTART_DONE_EVENTS                                                     \
    IRONMAN_PORT_ANP_EVENTS(0, AN_RESTART_E),                                                   \
    IRONMAN_PORT_ANP_EVENTS(1, AN_RESTART_E),                                                   \
    IRONMAN_PORT_ANP_EVENTS(2, AN_RESTART_E),                                                   \
    IRONMAN_CPU_PORT_ANP_EVENTS(0, AN_RESTART_E),                                               \
    IRONMAN_CPU_PORT_ANP_EVENTS(1, AN_RESTART_E),                                               \
    IRONMAN_CPU_PORT_ANP_EVENTS(2, AN_RESTART_E),                                               \
    IRONMAN_CPU_PORT_ANP_EVENTS(3, AN_RESTART_E)

#define IRONMAN_PORT_INT_PM_PCS_LINK_TIMER_OUT_EVENTS                                           \
    IRONMAN_PORT_ANP_EVENTS(0, INT_PM_PCS_LINK_TIMER_OUT_E),                                    \
    IRONMAN_PORT_ANP_EVENTS(1, INT_PM_PCS_LINK_TIMER_OUT_E),                                    \
    IRONMAN_PORT_ANP_EVENTS(2, INT_PM_PCS_LINK_TIMER_OUT_E),                                    \
    IRONMAN_CPU_PORT_ANP_EVENTS(0, INT_PM_PCS_LINK_TIMER_OUT_E),                                \
    IRONMAN_CPU_PORT_ANP_EVENTS(1, INT_PM_PCS_LINK_TIMER_OUT_E),                                \
    IRONMAN_CPU_PORT_ANP_EVENTS(2, INT_PM_PCS_LINK_TIMER_OUT_E),                                \
    IRONMAN_CPU_PORT_ANP_EVENTS(3, INT_PM_PCS_LINK_TIMER_OUT_E)

#define SINGLE_MG_EVENT_MAC(_postFix, _mgUnitId, _index)                                        \
    PRV_CPSS_IRONMAN_MG_##_mgUnitId##_MG_INTERNAL_INTERRUPT_CAUSE_##_postFix, (((_mgUnitId)*8) + _index)

#define ALL_MG_EVENTS_MAC(_postFix, _index)                                                   \
    SINGLE_MG_EVENT_MAC(_postFix, 0, _index),                                                 \
    SINGLE_MG_EVENT_MAC(_postFix, 1, _index),                                                 \
    SINGLE_MG_EVENT_MAC(_postFix, 2, _index)

#define SINGLE_TX_SDMA_QUEUE_MAC(type, _mgUnitId, _localQueue)  \
    PRV_CPSS_IRONMAN_MG_##_mgUnitId##_TRANSMIT_SDMA_INTERRUPT_CAUSE0_##type##_##_localQueue##_E,  (_mgUnitId*8 + _localQueue)

#define SINGLE_MG_TX_SDMA_QUEUES_MAC(type, _mgUnitId)                  \
    SINGLE_TX_SDMA_QUEUE_MAC(type, _mgUnitId, 0/*local queue 0*/),     \
    SINGLE_TX_SDMA_QUEUE_MAC(type, _mgUnitId, 1/*local queue 1*/),     \
    SINGLE_TX_SDMA_QUEUE_MAC(type, _mgUnitId, 2/*local queue 2*/),     \
    SINGLE_TX_SDMA_QUEUE_MAC(type, _mgUnitId, 3/*local queue 3*/),     \
    SINGLE_TX_SDMA_QUEUE_MAC(type, _mgUnitId, 4/*local queue 4*/),     \
    SINGLE_TX_SDMA_QUEUE_MAC(type, _mgUnitId, 5/*local queue 5*/),     \
    SINGLE_TX_SDMA_QUEUE_MAC(type, _mgUnitId, 6/*local queue 6*/),     \
    SINGLE_TX_SDMA_QUEUE_MAC(type, _mgUnitId, 7/*local queue 7*/)

#define ALL_MG_TX_SDMA_QUEUES(type)         \
    SINGLE_MG_TX_SDMA_QUEUES_MAC(type, 0/*mg0*/),  \
    SINGLE_MG_TX_SDMA_QUEUES_MAC(type, 2/*mg2*/)


/* connect unified event of 'RX SDMA' to their interrupt */
/* specific unified event to specific interrupt */
#define SINGLE_RX_SDMA_INT_MAC(type, local_mgUnitId, globalQueue, localQueue) \
    CPSS_PP_##type##_QUEUE##globalQueue##_E,                              \
        PRV_CPSS_IRONMAN_MG_##local_mgUnitId##_RECEIVE_SDMA_INTERRUPT_CAUSE0_##type##_QUEUE_##localQueue##_E, globalQueue, \
    MARK_END_OF_UNI_EV_CNS



#define SINGLE_MG_RX_SDMA_MAC(mgIndex) \
    PRV_CPSS_IRONMAN_MG_##mgIndex##_RECEIVE_SDMA_INTERRUPT_CAUSE0_RESOURCE_ERROR_CNT_OF_E,     ((mgIndex)*6)+0, \
    PRV_CPSS_IRONMAN_MG_##mgIndex##_RECEIVE_SDMA_INTERRUPT_CAUSE0_BYTE_CNT_OF_E,               ((mgIndex)*6)+1, \
    PRV_CPSS_IRONMAN_MG_##mgIndex##_RECEIVE_SDMA_INTERRUPT_CAUSE0_PACKET_CNT_OF_E,             ((mgIndex)*6)+2


/* connect unified event of 'RX SDMA' to their interrupt */
/* global MG 0 */
#define MG0_RX_SDMA_INT_MAC(type) \
    SINGLE_RX_SDMA_INT_MAC(type,  0/*local mg*/,  0/*globalQueue*/,0/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,  0/*local mg*/,  1/*globalQueue*/,1/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,  0/*local mg*/,  2/*globalQueue*/,2/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,  0/*local mg*/,  3/*globalQueue*/,3/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,  0/*local mg*/,  4/*globalQueue*/,4/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,  0/*local mg*/,  5/*globalQueue*/,5/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,  0/*local mg*/,  6/*globalQueue*/,6/*localQueue*/),\
    SINGLE_RX_SDMA_INT_MAC(type,  0/*local mg*/,  7/*globalQueue*/,7/*localQueue*/)

/* _postFix - must include the "_E" */
#define SET_LMU_EVENT_WITH_INDEX_MAC(_postFix, _threshold, _index)             \
    PRV_CPSS_IRONMAN_LMU_N_##_threshold##_I_##_index##_##_postFix,  ((31*(_threshold)) +_index)

#define LMU_LATENCY_OVER_THRESHOLD_MAC(_threshold)   \
    SET_LMU_EVENT_WITH_INDEX_MAC(LMU_LATENCY_OVER_THRESHOLD_E,  _threshold,   0),                                                                        \
    SET_LMU_EVENT_WITH_INDEX_MAC(LMU_LATENCY_OVER_THRESHOLD_E,  _threshold,   1),                                                                        \
    SET_LMU_EVENT_WITH_INDEX_MAC(LMU_LATENCY_OVER_THRESHOLD_E,  _threshold,   2),                                                                        \
    SET_LMU_EVENT_WITH_INDEX_MAC(LMU_LATENCY_OVER_THRESHOLD_E,  _threshold,   3),                                                                        \
    SET_LMU_EVENT_WITH_INDEX_MAC(LMU_LATENCY_OVER_THRESHOLD_E,  _threshold,   4),                                                                        \
    SET_LMU_EVENT_WITH_INDEX_MAC(LMU_LATENCY_OVER_THRESHOLD_E,  _threshold,   5),                                                                        \
    SET_LMU_EVENT_WITH_INDEX_MAC(LMU_LATENCY_OVER_THRESHOLD_E,  _threshold,   6),                                                                        \
    SET_LMU_EVENT_WITH_INDEX_MAC(LMU_LATENCY_OVER_THRESHOLD_E,  _threshold,   7),                                                                        \
    SET_LMU_EVENT_WITH_INDEX_MAC(LMU_LATENCY_OVER_THRESHOLD_E,  _threshold,   8),                                                                        \
    SET_LMU_EVENT_WITH_INDEX_MAC(LMU_LATENCY_OVER_THRESHOLD_E,  _threshold,   9),                                                                        \
    SET_LMU_EVENT_WITH_INDEX_MAC(LMU_LATENCY_OVER_THRESHOLD_E,  _threshold,  10),                                                                        \
    SET_LMU_EVENT_WITH_INDEX_MAC(LMU_LATENCY_OVER_THRESHOLD_E,  _threshold,  11),                                                                        \
    SET_LMU_EVENT_WITH_INDEX_MAC(LMU_LATENCY_OVER_THRESHOLD_E,  _threshold,  12),                                                                        \
    SET_LMU_EVENT_WITH_INDEX_MAC(LMU_LATENCY_OVER_THRESHOLD_E,  _threshold,  13),                                                                        \
    SET_LMU_EVENT_WITH_INDEX_MAC(LMU_LATENCY_OVER_THRESHOLD_E,  _threshold,  14),                                                                        \
    SET_LMU_EVENT_WITH_INDEX_MAC(LMU_LATENCY_OVER_THRESHOLD_E,  _threshold,  15),                                                                        \
    SET_LMU_EVENT_WITH_INDEX_MAC(LMU_LATENCY_OVER_THRESHOLD_E,  _threshold,  16),                                                                        \
    SET_LMU_EVENT_WITH_INDEX_MAC(LMU_LATENCY_OVER_THRESHOLD_E,  _threshold,  17),                                                                        \
    SET_LMU_EVENT_WITH_INDEX_MAC(LMU_LATENCY_OVER_THRESHOLD_E,  _threshold,  18),                                                                        \
    SET_LMU_EVENT_WITH_INDEX_MAC(LMU_LATENCY_OVER_THRESHOLD_E,  _threshold,  19),                                                                        \
    SET_LMU_EVENT_WITH_INDEX_MAC(LMU_LATENCY_OVER_THRESHOLD_E,  _threshold,  20),                                                                        \
    SET_LMU_EVENT_WITH_INDEX_MAC(LMU_LATENCY_OVER_THRESHOLD_E,  _threshold,  21),                                                                        \
    SET_LMU_EVENT_WITH_INDEX_MAC(LMU_LATENCY_OVER_THRESHOLD_E,  _threshold,  22),                                                                        \
    SET_LMU_EVENT_WITH_INDEX_MAC(LMU_LATENCY_OVER_THRESHOLD_E,  _threshold,  23),                                                                        \
    SET_LMU_EVENT_WITH_INDEX_MAC(LMU_LATENCY_OVER_THRESHOLD_E,  _threshold,  24),                                                                        \
    SET_LMU_EVENT_WITH_INDEX_MAC(LMU_LATENCY_OVER_THRESHOLD_E,  _threshold,  25),                                                                        \
    SET_LMU_EVENT_WITH_INDEX_MAC(LMU_LATENCY_OVER_THRESHOLD_E,  _threshold,  26),                                                                        \
    SET_LMU_EVENT_WITH_INDEX_MAC(LMU_LATENCY_OVER_THRESHOLD_E,  _threshold,  27),                                                                        \
    SET_LMU_EVENT_WITH_INDEX_MAC(LMU_LATENCY_OVER_THRESHOLD_E,  _threshold,  28),                                                                        \
    SET_LMU_EVENT_WITH_INDEX_MAC(LMU_LATENCY_OVER_THRESHOLD_E,  _threshold,  29),                                                                        \
    SET_LMU_EVENT_WITH_INDEX_MAC(LMU_LATENCY_OVER_THRESHOLD_E,  _threshold,  30)


#define CTSU_INSTANCE_CHANNEL_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, _channel,  _extData, _port)                        \
        PRV_CPSS_IRONMAN_CTSU_CHANNEL_##_channel##_CHANNEL_EGRESS_TIMESTAMP_##_postfix, (((_port) << 8) | (_extData))

#define SET_EVENT_GROUP_CHANNEL_CTSU_64_PCA_MAC(_postfix, _extData)                                               \
        CTSU_INSTANCE_CHANNEL_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix,  0, _extData,  0),               \
        CTSU_INSTANCE_CHANNEL_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix,  1, _extData,  1),               \
        CTSU_INSTANCE_CHANNEL_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix,  2, _extData,  2),               \
        CTSU_INSTANCE_CHANNEL_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix,  3, _extData,  3),               \
        CTSU_INSTANCE_CHANNEL_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix,  4, _extData,  4),               \
        CTSU_INSTANCE_CHANNEL_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix,  5, _extData,  5),               \
        CTSU_INSTANCE_CHANNEL_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix,  6, _extData,  6),               \
        CTSU_INSTANCE_CHANNEL_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix,  7, _extData,  7),               \
        CTSU_INSTANCE_CHANNEL_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix,  8, _extData,  8),               \
        CTSU_INSTANCE_CHANNEL_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix,  9, _extData,  9),               \
        CTSU_INSTANCE_CHANNEL_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, 10, _extData, 10),               \
        CTSU_INSTANCE_CHANNEL_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, 11, _extData, 11),               \
        CTSU_INSTANCE_CHANNEL_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, 12, _extData, 12),               \
        CTSU_INSTANCE_CHANNEL_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, 13, _extData, 13),               \
        CTSU_INSTANCE_CHANNEL_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, 14, _extData, 14),               \
        CTSU_INSTANCE_CHANNEL_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, 15, _extData, 15),               \
        CTSU_INSTANCE_CHANNEL_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, 16, _extData, 16),               \
        CTSU_INSTANCE_CHANNEL_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, 17, _extData, 17),               \
        CTSU_INSTANCE_CHANNEL_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, 18, _extData, 18),               \
        CTSU_INSTANCE_CHANNEL_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, 19, _extData, 19),               \
        CTSU_INSTANCE_CHANNEL_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, 20, _extData, 20),               \
        CTSU_INSTANCE_CHANNEL_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, 21, _extData, 21),               \
        CTSU_INSTANCE_CHANNEL_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, 22, _extData, 22),               \
        CTSU_INSTANCE_CHANNEL_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, 23, _extData, 23),               \
        CTSU_INSTANCE_CHANNEL_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, 24, _extData, 24),               \
        CTSU_INSTANCE_CHANNEL_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, 25, _extData, 25),               \
        CTSU_INSTANCE_CHANNEL_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, 26, _extData, 26),               \
        CTSU_INSTANCE_CHANNEL_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, 27, _extData, 27),               \
        CTSU_INSTANCE_CHANNEL_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, 28, _extData, 28),               \
        CTSU_INSTANCE_CHANNEL_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, 29, _extData, 29),               \
        CTSU_INSTANCE_CHANNEL_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, 30, _extData, 30),               \
        CTSU_INSTANCE_CHANNEL_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, 31, _extData, 31),               \
        CTSU_INSTANCE_CHANNEL_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, 32, _extData, 32),               \
        CTSU_INSTANCE_CHANNEL_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, 33, _extData, 33),               \
        CTSU_INSTANCE_CHANNEL_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, 34, _extData, 34),               \
        CTSU_INSTANCE_CHANNEL_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, 35, _extData, 35),               \
        CTSU_INSTANCE_CHANNEL_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, 36, _extData, 36),               \
        CTSU_INSTANCE_CHANNEL_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, 37, _extData, 37),               \
        CTSU_INSTANCE_CHANNEL_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, 38, _extData, 38),               \
        CTSU_INSTANCE_CHANNEL_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, 39, _extData, 39),               \
        CTSU_INSTANCE_CHANNEL_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, 40, _extData, 40),               \
        CTSU_INSTANCE_CHANNEL_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, 41, _extData, 41),               \
        CTSU_INSTANCE_CHANNEL_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, 42, _extData, 42),               \
        CTSU_INSTANCE_CHANNEL_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, 43, _extData, 43),               \
        CTSU_INSTANCE_CHANNEL_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, 44, _extData, 44),               \
        CTSU_INSTANCE_CHANNEL_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, 45, _extData, 45),               \
        CTSU_INSTANCE_CHANNEL_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, 46, _extData, 46),               \
        CTSU_INSTANCE_CHANNEL_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, 47, _extData, 47),               \
        CTSU_INSTANCE_CHANNEL_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, 48, _extData, 48),               \
        CTSU_INSTANCE_CHANNEL_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, 49, _extData, 49),               \
        CTSU_INSTANCE_CHANNEL_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, 50, _extData, 50),               \
        CTSU_INSTANCE_CHANNEL_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, 51, _extData, 51),               \
        CTSU_INSTANCE_CHANNEL_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, 52, _extData, 52),               \
        CTSU_INSTANCE_CHANNEL_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, 53, _extData, 53),               \
        CTSU_INSTANCE_CHANNEL_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, 54, _extData, 54),               \
        CTSU_INSTANCE_CHANNEL_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, 55, _extData, 55),               \
        CTSU_INSTANCE_CHANNEL_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, 56, _extData, 56),               \
        CTSU_INSTANCE_CHANNEL_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, 57, _extData, 57),               \
        CTSU_INSTANCE_CHANNEL_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, 58, _extData, 58),               \
        CTSU_INSTANCE_CHANNEL_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, 59, _extData, 59),               \
        CTSU_INSTANCE_CHANNEL_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, 60, _extData, 60),               \
        CTSU_INSTANCE_CHANNEL_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, 61, _extData, 61),               \
        CTSU_INSTANCE_CHANNEL_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, 62, _extData, 62),               \
        CTSU_INSTANCE_CHANNEL_PCA_EGRESS_TS_INTERRUPT_EVENT_MAC(_postfix, 63, _extData, 63)


#define IRONMAN_COMMON_EVENTS                                                                                           \
 /* single uniEvent for all 128 queues , extData = queueId */                                                           \
 CPSS_PP_TX_BUFFER_QUEUE_E,                                                                                             \
     ALL_MG_TX_SDMA_QUEUES(TX_BUFFER_QUEUE),                                                                            \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 /* single uniEvent for all 128 queues , extData = queueId */                                                           \
 CPSS_PP_TX_ERR_QUEUE_E,                                                                                                \
     ALL_MG_TX_SDMA_QUEUES(TX_ERROR_QUEUE),                                                                             \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 /* single uniEvent for all 128 queues , extData = queueId */                                                           \
 CPSS_PP_TX_END_E,                                                                                                      \
     ALL_MG_TX_SDMA_QUEUES(TX_END_QUEUE),                                                                               \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 MG0_RX_SDMA_INT_MAC(RX_BUFFER),                                                                                        \
 MG0_RX_SDMA_INT_MAC(RX_ERROR),                                                                                         \
                                                                                                                        \
 CPSS_PP_PORT_TX_FIFO_UNDERRUN_E,                                                                                       \
    /* USX total ports 0..47 */                                                                                         \
    SET_INDEX_EQUAL_EVENT_PER_3_MAC_PORTS_MAC(MAC_TX_UNDERFLOW_E),                                                      \
    SET_INDEX_EQUAL_EVENT_PER_24_MAC_8BR_PORTS_MAC(MAC_TX_UNDERFLOW_E),                                                 \
    SET_INDEX_EQUAL_EVENT_PER_24_MAC_8_PORTS_MAC(MAC_TX_UNDERFLOW_E),                                                   \
    SET_INDEX_EQUAL_EVENT_PER_4_MAC_BR_CPU_PORTS_MAC(MAC_TX_UNDERFLOW_E),                                               \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_PORT_TX_FIFO_OVERRUN_E,                                                                                        \
    /* USX total ports 0..47 */                                                                                         \
    SET_INDEX_EQUAL_EVENT_PER_3_MAC_PORTS_MAC(EMAC_TX_OVR_ERR_E),                                                       \
    SET_INDEX_EQUAL_EVENT_PER_3_MAC_PORTS_MAC(PMAC_TX_OVR_ERR_E),                                                       \
    SET_INDEX_EQUAL_EVENT_PER_24_MAC_8BR_PORTS_MAC(EMAC_TX_OVR_ERR_E),                                                  \
    SET_INDEX_EQUAL_EVENT_PER_24_MAC_8BR_PORTS_MAC(PMAC_TX_OVR_ERR_E),                                                  \
    SET_INDEX_EQUAL_EVENT_PER_24_MAC_8_PORTS_MAC(EMAC_TX_OVR_ERR_E),                                                    \
    SET_INDEX_EQUAL_EVENT_PER_24_MAC_8_PORTS_MAC(PMAC_TX_OVR_ERR_E),                                                    \
    SET_INDEX_EQUAL_EVENT_PER_4_MAC_BR_CPU_PORTS_MAC(EMAC_TX_OVR_ERR_E),                                                \
    SET_INDEX_EQUAL_EVENT_PER_4_MAC_BR_CPU_PORTS_MAC(PMAC_TX_OVR_ERR_E),                                                \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_PORT_RX_FIFO_OVERRUN_E,                                                                                        \
    /* USX total ports 0..47 */                                                                                         \
    SET_INDEX_EQUAL_EVENT_PER_3_MAC_PORTS_MAC(EMAC_RX_OVERRUN_E),                                                       \
    SET_INDEX_EQUAL_EVENT_PER_3_MAC_PORTS_MAC(PMAC_RX_OVERRUN_E),                                                       \
    SET_INDEX_EQUAL_EVENT_PER_24_MAC_8BR_PORTS_MAC(EMAC_RX_OVERRUN_E),                                                  \
    SET_INDEX_EQUAL_EVENT_PER_24_MAC_8BR_PORTS_MAC(PMAC_RX_OVERRUN_E),                                                  \
    SET_INDEX_EQUAL_EVENT_PER_24_MAC_8_PORTS_MAC(EMAC_RX_OVERRUN_E),                                                    \
    SET_INDEX_EQUAL_EVENT_PER_24_MAC_8_PORTS_MAC(PMAC_RX_OVERRUN_E),                                                    \
    SET_INDEX_EQUAL_EVENT_PER_4_MAC_BR_CPU_PORTS_MAC(EMAC_RX_OVERRUN_E),                                                \
    SET_INDEX_EQUAL_EVENT_PER_4_MAC_BR_CPU_PORTS_MAC(PMAC_RX_OVERRUN_E),                                                \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
  /* Per Index events */                                                                                                \
 CPSS_PP_GPP_E,                                                                                                         \
    ALL_MG_EVENTS_MAC(GPP_INT_CAUSE_0_E, 0),                                                                            \
    ALL_MG_EVENTS_MAC(GPP_INT_CAUSE_1_E, 1),                                                                            \
    ALL_MG_EVENTS_MAC(GPP_INT_CAUSE_2_E, 2),                                                                            \
    ALL_MG_EVENTS_MAC(GPP_INT_CAUSE_3_E, 3),                                                                            \
    ALL_MG_EVENTS_MAC(GPP_INT_CAUSE_4_E, 4),                                                                            \
    ALL_MG_EVENTS_MAC(GPP_INT_CAUSE_5_E, 5),                                                                            \
    ALL_MG_EVENTS_MAC(GPP_INT_CAUSE_6_E, 6),                                                                            \
    ALL_MG_EVENTS_MAC(GPP_INT_CAUSE_7_E, 7),                                                                            \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_LMU_LATENCY_EXCEED_THRESHOLD_E,                                                                                \
    LMU_LATENCY_OVER_THRESHOLD_MAC(0),                                                                                  \
    LMU_LATENCY_OVER_THRESHOLD_MAC(1),                                                                                  \
    LMU_LATENCY_OVER_THRESHOLD_MAC(2),                                                                                  \
    LMU_LATENCY_OVER_THRESHOLD_MAC(3),                                                                                  \
    LMU_LATENCY_OVER_THRESHOLD_MAC(4),                                                                                  \
    LMU_LATENCY_OVER_THRESHOLD_MAC(5),                                                                                  \
    LMU_LATENCY_OVER_THRESHOLD_MAC(6),                                                                                  \
    LMU_LATENCY_OVER_THRESHOLD_MAC(7),                                                                                  \
    LMU_LATENCY_OVER_THRESHOLD_MAC(8),                                                                                  \
    LMU_LATENCY_OVER_THRESHOLD_MAC(9),                                                                                  \
    LMU_LATENCY_OVER_THRESHOLD_MAC(10),                                                                                 \
    LMU_LATENCY_OVER_THRESHOLD_MAC(11),                                                                                 \
    LMU_LATENCY_OVER_THRESHOLD_MAC(12),                                                                                 \
    LMU_LATENCY_OVER_THRESHOLD_MAC(13),                                                                                 \
    LMU_LATENCY_OVER_THRESHOLD_MAC(14),                                                                                 \
    LMU_LATENCY_OVER_THRESHOLD_MAC(15),                                                                                 \
    LMU_LATENCY_OVER_THRESHOLD_MAC(16),                                                                                 \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
                                                                                                                        \
 CPSS_PP_CNC_WRAPAROUND_BLOCK_E,                                                                                        \
    SET_EVENT_WITH_INDEX_MAC(CNC_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_BLOCK_0_WRAPAROUND_E,  0),                       \
    SET_EVENT_WITH_INDEX_MAC(CNC_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_BLOCK_1_WRAPAROUND_E,  1),                       \
    SET_EVENT_WITH_INDEX_MAC(CNC_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_BLOCK_2_WRAPAROUND_E,  2),                       \
    SET_EVENT_WITH_INDEX_MAC(CNC_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_BLOCK_3_WRAPAROUND_E,  3),                       \
    SET_EVENT_WITH_INDEX_MAC(CNC_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_BLOCK_4_WRAPAROUND_E,  4),                       \
    SET_EVENT_WITH_INDEX_MAC(CNC_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_BLOCK_5_WRAPAROUND_E,  5),                       \
    SET_EVENT_WITH_INDEX_MAC(CNC_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_BLOCK_6_WRAPAROUND_E,  6),                       \
    SET_EVENT_WITH_INDEX_MAC(CNC_WRAPAROUND_FUNCTIONAL_INTERRUPT_CAUSE_BLOCK_7_WRAPAROUND_E,  7),                       \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_RX_CNTR_OVERFLOW_E,                                                                                            \
    SINGLE_MG_RX_SDMA_MAC(0),                                                                                           \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_SCT_RATE_LIMITER_E,                                                                                            \
    SET_EVENT_PER_ALL_PORTS_MAC(EQ_REG, PKT_DROPED_INT_E),                                                              \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_POLICER_ADDR_OUT_OF_MEMORY_E,                                                                                  \
    SET_EVENT_WITH_INDEX_MAC(IPLR0_POLICER_INTERRUPT_CAUSE_ILLEGAL_ACCESS_E,       0),                                  \
    SET_EVENT_WITH_INDEX_MAC(IPLR1_POLICER_INTERRUPT_CAUSE_ILLEGAL_ACCESS_E,       1),                                  \
    SET_EVENT_WITH_INDEX_MAC(EPLR_POLICER_INTERRUPT_CAUSE_ILLEGAL_ACCESS_E,        2),                                  \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_POLICER_IPFIX_WRAP_AROUND_E,                                                                                   \
    SET_EVENT_WITH_INDEX_MAC(IPLR0_POLICER_INTERRUPT_CAUSE_IPFIX_WRAPAROUND_E,            0),                           \
    SET_EVENT_WITH_INDEX_MAC(IPLR1_POLICER_INTERRUPT_CAUSE_IPFIX_WRAPAROUND_E,            1),                           \
    SET_EVENT_WITH_INDEX_MAC(EPLR_POLICER_INTERRUPT_CAUSE_IPFIX_WRAPAROUND_E,             2),                           \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_POLICER_IPFIX_ALARM_E,                                                                                         \
    SET_EVENT_WITH_INDEX_MAC(IPLR0_POLICER_INTERRUPT_CAUSE_IPFIX_ALARM_E,                 0),                           \
    SET_EVENT_WITH_INDEX_MAC(IPLR1_POLICER_INTERRUPT_CAUSE_IPFIX_ALARM_E,                 1),                           \
    SET_EVENT_WITH_INDEX_MAC(EPLR_POLICER_INTERRUPT_CAUSE_IPFIX_ALARM_E,                  2),                           \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_POLICER_IPFIX_ALARMED_ENTRIES_FIFO_WRAP_AROUND_E,                                                              \
    SET_EVENT_WITH_INDEX_MAC(IPLR0_POLICER_INTERRUPT_CAUSE_IPFIX_ALARMED_ENTRIES_FIFO_WRAPAROUND_E,    0),              \
    SET_EVENT_WITH_INDEX_MAC(IPLR1_POLICER_INTERRUPT_CAUSE_IPFIX_ALARMED_ENTRIES_FIFO_WRAPAROUND_E,    1),              \
    SET_EVENT_WITH_INDEX_MAC(EPLR_POLICER_INTERRUPT_CAUSE_IPFIX_ALARMED_ENTRIES_FIFO_WRAPAROUND_E,     2),              \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_PCL_LOOKUP_DATA_ERROR_E,                                                                                       \
    SET_EVENT_WITH_INDEX_MAC(PCL_PCL_UNIT_INTERRUPT_CAUSE_PCL0_ACTION0_TCAM_ACCESS_DATA_ERROR_E,    0),                 \
    SET_EVENT_WITH_INDEX_MAC(PCL_PCL_UNIT_INTERRUPT_CAUSE_PCL0_ACTION1_TCAM_ACCESS_DATA_ERROR_E,    0),                 \
    SET_EVENT_WITH_INDEX_MAC(PCL_PCL_UNIT_INTERRUPT_CAUSE_PCL0_ACTION2_TCAM_ACCESS_DATA_ERROR_E,    0),                 \
    SET_EVENT_WITH_INDEX_MAC(PCL_PCL_UNIT_INTERRUPT_CAUSE_PCL0_ACTION3_TCAM_ACCESS_DATA_ERROR_E,    0),                 \
                                                                                                                        \
    SET_EVENT_WITH_INDEX_MAC(PCL_PCL_UNIT_INTERRUPT_CAUSE_PCL1_ACTION0_TCAM_ACCESS_DATA_ERROR_E,    1),                 \
    SET_EVENT_WITH_INDEX_MAC(PCL_PCL_UNIT_INTERRUPT_CAUSE_PCL1_ACTION1_TCAM_ACCESS_DATA_ERROR_E,    1),                 \
    SET_EVENT_WITH_INDEX_MAC(PCL_PCL_UNIT_INTERRUPT_CAUSE_PCL1_ACTION2_TCAM_ACCESS_DATA_ERROR_E,    1),                 \
    SET_EVENT_WITH_INDEX_MAC(PCL_PCL_UNIT_INTERRUPT_CAUSE_PCL1_ACTION3_TCAM_ACCESS_DATA_ERROR_E,    1),                 \
                                                                                                                        \
    SET_EVENT_WITH_INDEX_MAC(PCL_PCL_UNIT_INTERRUPT_CAUSE_PCL2_ACTION0_TCAM_ACCESS_DATA_ERROR_E,    2),                 \
    SET_EVENT_WITH_INDEX_MAC(PCL_PCL_UNIT_INTERRUPT_CAUSE_PCL2_ACTION1_TCAM_ACCESS_DATA_ERROR_E,    2),                 \
    SET_EVENT_WITH_INDEX_MAC(PCL_PCL_UNIT_INTERRUPT_CAUSE_PCL2_ACTION2_TCAM_ACCESS_DATA_ERROR_E,    2),                 \
    SET_EVENT_WITH_INDEX_MAC(PCL_PCL_UNIT_INTERRUPT_CAUSE_PCL2_ACTION3_TCAM_ACCESS_DATA_ERROR_E,    2),                 \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_PCL_LOOKUP_FIFO_FULL_E,                                                                                        \
    SET_EVENT_WITH_INDEX_MAC(PCL_PCL_UNIT_INTERRUPT_CAUSE_LOOKUP0_FIFO_FULL_E,                      0),                 \
    SET_EVENT_WITH_INDEX_MAC(PCL_PCL_UNIT_INTERRUPT_CAUSE_LOOKUP1_FIFO_FULL_E,                      1),                 \
    SET_EVENT_WITH_INDEX_MAC(PCL_PCL_UNIT_INTERRUPT_CAUSE_LOOKUP2_FIFO_FULL_E,                      2),                 \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_PCL_ACTION_TRIGGERED_E,                                                                                        \
    SET_EVENT_WITH_INDEX_MAC(PCL_PCL_UNIT_INTERRUPT_CAUSE_IPCL0_TCAM_TRIGGERED_INTERRUPT_E,         0),                 \
    SET_EVENT_WITH_INDEX_MAC(PCL_PCL_UNIT_INTERRUPT_CAUSE_IPCL1_TCAM_TRIGGERED_INTERRUPT_E,         1),                 \
    SET_EVENT_WITH_INDEX_MAC(PCL_PCL_UNIT_INTERRUPT_CAUSE_IPCL2_TCAM_TRIGGERED_INTERRUPT_E,         2),                 \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_EB_NA_FIFO_FULL_E,                                                                                             \
    SET_EVENT_WITH_INDEX_MAC(FDB_IP_FDB_INTERRUPT_CAUSE_REGISTER_PIPE_NUMBER_0_NA_FIFO_FULL_E,         0),              \
    SET_EVENT_WITH_INDEX_MAC(FDB_IP_FDB_INTERRUPT_CAUSE_REGISTER_PIPE_NUMBER_1_NA_FIFO_FULL_E,         1),              \
    SET_EVENT_WITH_INDEX_MAC(FDB_IP_FDB_INTERRUPT_CAUSE_REGISTER_PIPE_NUMBER_2_NA_FIFO_FULL_E,         2),              \
    SET_EVENT_WITH_INDEX_MAC(FDB_IP_FDB_INTERRUPT_CAUSE_REGISTER_PIPE_NUMBER_3_NA_FIFO_FULL_E,         3),              \
    SET_EVENT_WITH_INDEX_MAC(FDB_IP_FDB_INTERRUPT_CAUSE_REGISTER_PIPE_NUMBER_4_NA_FIFO_FULL_E,         4),              \
    SET_EVENT_WITH_INDEX_MAC(FDB_IP_FDB_INTERRUPT_CAUSE_REGISTER_PIPE_NUMBER_5_NA_FIFO_FULL_E,         5),              \
    SET_EVENT_WITH_INDEX_MAC(FDB_IP_FDB_INTERRUPT_CAUSE_REGISTER_PIPE_NUMBER_6_NA_FIFO_FULL_E,         6),              \
    SET_EVENT_WITH_INDEX_MAC(FDB_IP_FDB_INTERRUPT_CAUSE_REGISTER_PIPE_NUMBER_7_NA_FIFO_FULL_E,         7),              \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_TTI_ACCESS_DATA_ERROR_E,                                                                                       \
    SET_EVENT_WITH_INDEX_MAC(TTI_TTI_ENGINE_INTERRUPT_CAUSE_TTIACTION_0_ACCESS_DATA_ERROR_E, 0),                        \
    SET_EVENT_WITH_INDEX_MAC(TTI_TTI_ENGINE_INTERRUPT_CAUSE_TTIACTION_1_ACCESS_DATA_ERROR_E, 1),                        \
    SET_EVENT_WITH_INDEX_MAC(TTI_TTI_ENGINE_INTERRUPT_CAUSE_TTIACTION_2_ACCESS_DATA_ERROR_E, 2),                        \
    SET_EVENT_WITH_INDEX_MAC(TTI_TTI_ENGINE_INTERRUPT_CAUSE_TTIACTION_3_ACCESS_DATA_ERROR_E, 3),                        \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
CPSS_SRVCPU_SDK_WATCHDOG_E,                                                                                             \
    PRV_CPSS_IRONMAN_MG_0_MG1_INTERNAL_INTERRUPT_CAUSE_CM3_SECOND_TIMER_INT_E,  0,                                      \
    PRV_CPSS_IRONMAN_MG_1_MG1_INTERNAL_INTERRUPT_CAUSE_CM3_SECOND_TIMER_INT_E,  1,                                      \
    PRV_CPSS_IRONMAN_MG_2_MG1_INTERNAL_INTERRUPT_CAUSE_CM3_SECOND_TIMER_INT_E,  2,                                      \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_SRVCPU_SDK_DOORBELL_E,                                                                                            \
    PRV_CPSS_IRONMAN_MG_0_MG_INTERNAL_INTERRUPT_CAUSE_SERVICE_CP_U2_HOST_DOORBELL_INTERRUPT_E,  0,                      \
    PRV_CPSS_IRONMAN_MG_1_MG_INTERNAL_INTERRUPT_CAUSE_SERVICE_CP_U2_HOST_DOORBELL_INTERRUPT_E,  1,                      \
    PRV_CPSS_IRONMAN_MG_2_MG_INTERNAL_INTERRUPT_CAUSE_SERVICE_CP_U2_HOST_DOORBELL_INTERRUPT_E,  2,                      \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
  /* next HW interrupt value will have same index (evExtData) value in CPSS_PP_CRITICAL_HW_ERROR_E */                   \
 CPSS_PP_CRITICAL_HW_ERROR_E,                                                                                           \
                                                                                                                        \
    /*PSI_REG*/                                                                                                         \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PSI_REGS_PSI_INTERRUPT_CAUSE_SCHEDULER_INTERRUPT_E),                               \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(PSI_REGS_PSI_INTERRUPT_CAUSE_APB2SNAKE_INTERRUPT_E),                               \
                                                                                                                        \
    /*TXQ_QFC*/                                                                                                         \
                                                                                                                        \
    /*TXQ_PFCC*/                                                                                                        \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(TXQ_PR_PFCC_INTERRUPT_CAUSE_POOL_0_COUNTER_OVERFLOW_E),                            \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(TXQ_PR_PFCC_INTERRUPT_CAUSE_SOURCE_PORT_COUNTER_OVERFLOW_E),                       \
                                                                                                                        \
    /*HBU*/                                                                                                             \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(HBU_HBU_INTERRUPT_CAUSE_ERROR_FHF_MEM_E),                                          \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(HBU_HBU_INTERRUPT_CAUSE_ERROR_CMT1_MEM_E),                                         \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(HBU_HBU_INTERRUPT_CAUSE_ERROR_CMT2_MEM_E),                                         \
                                                                                                                        \
    /*TXQ_SDQ*/                                                                                                         \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(TXQ_SCN_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UN_MAP_QUEUE_CREDIT_E),                     \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(TXQ_SCN_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UN_MAP_QUEUE_ENQ_E),                        \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(TXQ_SCN_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UN_MAP_QUEUE_DEQ_E),                        \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(TXQ_SCN_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UN_MAP_QUEUE_QCN_E),                        \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(TXQ_SCN_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UN_MAP_PORT_CREDIT_E),                      \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(TXQ_SCN_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UN_MAP_PORT_ENQ_E),                         \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(TXQ_SCN_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UN_MAP_PORT_DEQ_E),                         \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(TXQ_SCN_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UN_MAP_PORT_QCN_E),                         \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(TXQ_SCN_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_UN_MAP_PORT_PFC_E),                         \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(TXQ_SCN_SDQ_INTERRUPT_FUNCTIONAL_CAUSE_SEL_PORT_OUTOFRANGE_E),                     \
                                                                                                                        \
    /*TXQ_PDS*/                                                                                                         \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(TXQ_SCN_PDS_INTERRUPT_DEBUG_CAUSE_TOTAL_PDS_CNTR_OVRRUN_INT_E),                    \
                                                                                                                        \
    /*SMB_MC*/                                                                                                          \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INTERRUPT_MISC_CAUSE_INTERRUPT_ALIGN_WR_CMDS_FIFOS_OVERFLOW_E),             \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_0_INTERRUPT_RBW_ERR_E),                  \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_1_INTERRUPT_RBW_ERR_E),                  \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_2_INTERRUPT_RBW_ERR_E),                  \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_3_INTERRUPT_RBW_ERR_E),                  \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_4_INTERRUPT_RBW_ERR_E),                  \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_5_INTERRUPT_RBW_ERR_E),                  \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_6_INTERRUPT_RBW_ERR_E),                  \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INTERRUPT_RBW_CAUSE_LOGICAL_MEMORY_7_INTERRUPT_RBW_ERR_E),                  \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_0_REFCNT_SER_ERROR_E),             \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_1_REFCNT_SER_ERROR_E),             \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_2_REFCNT_SER_ERROR_E),             \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_3_REFCNT_SER_ERROR_E),             \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_4_REFCNT_SER_ERROR_E),             \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_5_REFCNT_SER_ERROR_E),             \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_6_REFCNT_SER_ERROR_E),             \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INTERRUPT_MEMORIES2_CAUSE_LOGICAL_MEMORY_7_REFCNT_SER_ERROR_E),             \
                                                                                                                        \
    /*SMB_WRITE_ARBITER*/                                                                                               \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(SMB_MC_INTERRUPT_MISC_CAUSE_INTERRUPT_ALIGN_WR_CMDS_FIFOS_OVERFLOW_E),             \
                                                                                                                        \
    /*GPC_PACKET_WRITE*/                                                                                                \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_WRITE_INTERRUPT_CAUSE_INT_CAUSE_CA_FIFO_UNDRFL_E),                      \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_WRITE_INTERRUPT_CAUSE_INT_CAUSE_CA_FIFO_OVFL_E),                        \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_WRITE_INTERRUPT_CAUSE_INT_CAUSE_NPM_WR_FIFO_UNDRFL_E),                  \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_WRITE_INTERRUPT_CAUSE_INT_CAUSE_NPM_WR_FIFO_OVFL_E),                    \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(GPC_PACKET_WRITE_INTERRUPT_CAUSE_INT_CAUSE_LENGTH_ERR_TAIL_E),                     \
                                                                                                                        \
    /*DFX_SERVER_INTERRUPT_CAUSE*/                                                                                      \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(DFX_SERVER_INTERRUPT_BIST_FAIL_E),                                                 \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(DFX_SERVER_INTERRUPT_EXTERNAL_TEMPERATURE_THRESHOLD_E),                            \
                                                                                                                        \
    /*RXDMA_INTERRUPT2_CAUSE*/                                                                                          \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(RXDMA_RX_DMA_INTERRUPT_2_CAUSE_PB_TAIL_ID_MEM_SER_INT_E),                          \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(RXDMA_RX_DMA_INTERRUPT_2_CAUSE_FINAL_TAIL_BC_0_MEM_SER_INT_E),                     \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(RXDMA_RX_DMA_INTERRUPT_2_CAUSE_FINAL_TAIL_BC_1_MEM_SER_INT_E),                     \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(RXDMA_RX_DMA_INTERRUPT_2_CAUSE_REPLY_TAIL_PB_ID_MEM_SER_INT_E),                    \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(RXDMA_RX_DMA_INTERRUPT_2_CAUSE_PACM_MEM_0_PB_RD_DATA_ERR_INT_E),                   \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(RXDMA_RX_DMA_INTERRUPT_2_CAUSE_PACM_MEM_1_PB_RD_DATA_ERR_INT_E),                   \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(RXDMA_RX_DMA_INTERRUPT_2_CAUSE_PCM_MEM_0_PB_RD_DATA_ERR_INT_E),                    \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(RXDMA_RX_DMA_INTERRUPT_2_CAUSE_PCM_MEM_1_PB_RD_DATA_ERR_INT_E),                    \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(RXDMA_RX_DMA_INTERRUPT_2_CAUSE_PACKET_HEAD_FIFO_0_RD_SER_INT_E),                   \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(RXDMA_RX_DMA_INTERRUPT_2_CAUSE_PACKET_HEAD_FIFO_1_RD_SER_INT_E),                   \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(RXDMA_RX_DMA_INTERRUPT_2_CAUSE_ENQUEUE_REQUESTS_DESCRIPTOR_FIFO_RD_SER_INT_E),     \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(RXDMA_RX_DMA_INTERRUPT_2_CAUSE_HEAD_PACKET_COUNT_DATA_FIFO_MEM_RD_SER_INT_E),      \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(RXDMA_RX_DMA_INTERRUPT_2_CAUSE_RX_LOCAL_IDS_FIFO_MEM_RD_SER_INT_E),                \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_PTP_TAI_INCOMING_TRIGGER_E,                                                                                    \
    SET_TAI_UNIT_EVENT_WITH_INDEX_MAC(TAI_INTERRUPT_CAUSE_INCOMING_TRIGGER_INT_E, 0),                                   \
    SET_TAI_UNIT_EVENT_WITH_INDEX_MAC(TAI_INTERRUPT_CAUSE_INCOMING_TRIGGER_INT_E, 1),                                   \
    SET_TAI_UNIT_EVENT_WITH_INDEX_MAC(TAI_INTERRUPT_CAUSE_INCOMING_TRIGGER_INT_E, 2),                                   \
    SET_TAI_UNIT_EVENT_WITH_INDEX_MAC(TAI_INTERRUPT_CAUSE_INCOMING_TRIGGER_INT_E, 3),                                   \
    SET_TAI_UNIT_EVENT_WITH_INDEX_MAC(TAI_INTERRUPT_CAUSE_INCOMING_TRIGGER_INT_E, 4),                                   \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_PTP_TAI_GENERATION_E,                                                                                          \
    SET_TAI_UNIT_EVENT_WITH_INDEX_MAC(TAI_INTERRUPT_CAUSE_GENERATION_INT_E, 0),                                         \
    SET_TAI_UNIT_EVENT_WITH_INDEX_MAC(TAI_INTERRUPT_CAUSE_GENERATION_INT_E, 1),                                         \
    SET_TAI_UNIT_EVENT_WITH_INDEX_MAC(TAI_INTERRUPT_CAUSE_GENERATION_INT_E, 2),                                         \
    SET_TAI_UNIT_EVENT_WITH_INDEX_MAC(TAI_INTERRUPT_CAUSE_GENERATION_INT_E, 3),                                         \
    SET_TAI_UNIT_EVENT_WITH_INDEX_MAC(TAI_INTERRUPT_CAUSE_GENERATION_INT_E, 4),                                         \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_GTS_GLOBAL_FIFO_FULL_E,                                                                                        \
    PRV_CPSS_IRONMAN_ERMRK_ERMRK_INTERRUPT_CAUSE_INGRESS_TIMESTAMP_QUEUE0_FULL_E,  0,                                   \
    PRV_CPSS_IRONMAN_ERMRK_ERMRK_INTERRUPT_CAUSE_EGRESS_TIMESTAMP_QUEUE0_FULL_E,   1,                                   \
    PRV_CPSS_IRONMAN_ERMRK_ERMRK_INTERRUPT_CAUSE_INGRESS_TIMESTAMP_QUEUE1_FULL_E,  2,                                   \
    PRV_CPSS_IRONMAN_ERMRK_ERMRK_INTERRUPT_CAUSE_EGRESS_TIMESTAMP_QUEUE1_FULL_E,   3,                                   \
    SET_EVENT_GROUP_CHANNEL_CTSU_64_PCA_MAC(QUEUE_0_OVERFLOW_INTERRUPT_E, GTS_INT_EXT_PARAM_MAC(1,0,1)),                \
    SET_EVENT_GROUP_CHANNEL_CTSU_64_PCA_MAC(QUEUE_1_OVERFLOW_INTERRUPT_E, GTS_INT_EXT_PARAM_MAC(1,1,1)),                \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_GTS_VALID_TIME_SAMPLE_MESSAGE_E,                                                                               \
    PRV_CPSS_IRONMAN_ERMRK_ERMRK_INTERRUPT_CAUSE_NEW_INGRESS_TIMESTAMP_Q0_E,  0,                                        \
    PRV_CPSS_IRONMAN_ERMRK_ERMRK_INTERRUPT_CAUSE_NEW_EGRESS_TIMESTAMP_Q0_E,   1,                                        \
    PRV_CPSS_IRONMAN_ERMRK_ERMRK_INTERRUPT_CAUSE_NEW_INGRESS_TIMESTAMP_Q1_E,  2,                                        \
    PRV_CPSS_IRONMAN_ERMRK_ERMRK_INTERRUPT_CAUSE_NEW_EGRESS_TIMESTAMP_Q1_E,   3,                                        \
    SET_EVENT_GROUP_CHANNEL_CTSU_64_PCA_MAC(QUEUE_0_NEW_ENTRY_INTERRUPT_E, GTS_INT_EXT_PARAM_MAC(1,0,1)),               \
    SET_EVENT_GROUP_CHANNEL_CTSU_64_PCA_MAC(QUEUE_1_NEW_ENTRY_INTERRUPT_E, GTS_INT_EXT_PARAM_MAC(1,1,1)),               \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_DATA_INTEGRITY_ERROR_E,                                                                                        \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(HA_HA_INTERRUPT_CAUSE_ECC_SINGLE_ERROR_E),                                         \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(HA_HA_INTERRUPT_CAUSE_ECC_DOUBLE_ERROR_E),                                         \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(TCAM_TCAM_INTERRUPT_CAUSE_TCAM_ARRAY_PARITY_ERROR_E),                              \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(BMA_MC_CNT_PARITY_ERROR_E),                                                        \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(DFX_SERVER_INTERRUPT_SUMMARY_PIPE_0_INTERRUPT_SUM_E),                              \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(DFX_SERVER_INTERRUPT_SUMMARY_PIPE_1_INTERRUPT_SUM_E),                              \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(DFX_SERVER_INTERRUPT_SUMMARY_PIPE_2_INTERRUPT_SUM_E),                              \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_OAM_EXCEPTION_KEEPALIVE_AGING_E,                                                                               \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(IOAM_OAM_UNIT_INTERRUPT_CAUSE_KEEP_ALIVE_AGING_E),                                 \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_OAM_EXCEPTION_EXCESS_KEEPALIVE_E,                                                                              \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(IOAM_OAM_UNIT_INTERRUPT_CAUSE_EXCESS_KEEPALIVE_E),                                 \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_OAM_EXCEPTION_INVALID_KEEPALIVE_E,                                                                             \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(IOAM_OAM_UNIT_INTERRUPT_CAUSE_INVALID_KEEPALIVE_HASH_E),                           \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_OAM_EXCEPTION_RDI_STATUS_E,                                                                                    \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(IOAM_OAM_UNIT_INTERRUPT_CAUSE_RDI_STATUS_E),                                       \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_OAM_EXCEPTION_TX_PERIOD_E,                                                                                     \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(IOAM_OAM_UNIT_INTERRUPT_CAUSE_TX_PERIOD_E),                                        \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_OAM_EXCEPTION_MEG_LEVEL_E,                                                                                     \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(IOAM_OAM_UNIT_INTERRUPT_CAUSE_MEG_LEVEL_EXCEPTION_E),                              \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_OAM_EXCEPTION_SOURCE_INTERFACE_E,                                                                              \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(IOAM_OAM_UNIT_INTERRUPT_CAUSE_SOURCE_INTERFACE_EXCEPTION_E),                       \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
 CPSS_PP_OAM_ILLEGAL_ENTRY_INDEX_E,                                                                                     \
    SET_EQUAL_EVENT_WITH_POSTFIX_MAC(IOAM_OAM_UNIT_INTERRUPT_CAUSE_ILLEGAL_OAM_ENTRY_INDEX_E),                          \
 MARK_END_OF_UNI_EV_CNS,                                                                                                \
                                                                                                                        \
CPSS_PP_MACSEC_SA_EXPIRED_E,                                                                                            \
    IRONMAN_SET_EIP_164_EVENT_PER_ALL_DIRECTIONS_ALL_DP_MAC(GLOBAL_MAC_SEC_INTERRUPT_CAUSE_SA_EXPIRED_E),               \
MARK_END_OF_UNI_EV_CNS,                                                                                                 \
                                                                                                                        \
CPSS_PP_MACSEC_SA_PN_FULL_E,                                                                                            \
    IRONMAN_SET_EIP_164_EVENT_PER_EGRESS_DIRECTION_ALL_DP_MAC(GLOBAL_MAC_SEC_INTERRUPT_CAUSE_SA_PN_THR_E),              \
MARK_END_OF_UNI_EV_CNS,                                                                                                 \
                                                                                                                        \
CPSS_PP_MACSEC_STATISTICS_SUMMARY_E,                                                                                    \
    IRONMAN_SET_EIP_164_EVENT_SOURCE_PER_ALL_DIRECTIONS_ALL_DP_MAC(GLOBAL_MAC_SEC_INTERRUPT_CAUSE_STAT_SA_THR_E,       0),  \
    IRONMAN_SET_EIP_164_EVENT_SOURCE_PER_ALL_DIRECTIONS_ALL_DP_MAC(GLOBAL_MAC_SEC_INTERRUPT_CAUSE_STAT_SECY_THR_E,     1),  \
    IRONMAN_SET_EIP_164_EVENT_SOURCE_PER_ALL_DIRECTIONS_ALL_DP_MAC(GLOBAL_MAC_SEC_INTERRUPT_CAUSE_STAT_IFC0_THR_E,     2),  \
    IRONMAN_SET_EIP_164_EVENT_SOURCE_PER_INGRESS_DIRECTION_ALL_DP_MAC(GLOBAL_MAC_SEC_INTERRUPT_CAUSE_STAT_IFC1_THR_E,  3),  \
    IRONMAN_SET_EIP_164_EVENT_SOURCE_PER_INGRESS_DIRECTION_ALL_DP_MAC(GLOBAL_MAC_SEC_INTERRUPT_CAUSE_STAT_RXCAM_THR_E, 4),  \
    IRONMAN_SET_EIP_163_EVENT_SOURCE_PER_ALL_DIRECTIONS_ALL_DP_MAC(GLOBAL_MAC_SEC_INTERRUPT_CAUSE_STAT_TCAM_THR_E,     5),  \
    IRONMAN_SET_EIP_163_EVENT_SOURCE_PER_ALL_DIRECTIONS_ALL_DP_MAC(GLOBAL_MAC_SEC_INTERRUPT_CAUSE_STAT_CHANNEL_THR_E,  6),  \
MARK_END_OF_UNI_EV_CNS,                                                                                                  \
                                                                                                                         \
CPSS_PP_MACSEC_TRANSFORM_ENGINE_ERR_E,                                                                                   \
    IRONMAN_SET_EIP_66_ERROR_EVENT_PER_ALL_DIRECTIONS_ALL_DP_MAC(0),                                                     \
    IRONMAN_SET_EIP_66_ERROR_EVENT_PER_ALL_DIRECTIONS_ALL_DP_MAC(1),                                                     \
    IRONMAN_SET_EIP_66_ERROR_EVENT_PER_ALL_DIRECTIONS_ALL_DP_MAC(2),                                                     \
    IRONMAN_SET_EIP_66_ERROR_EVENT_PER_ALL_DIRECTIONS_ALL_DP_MAC(3),                                                     \
    IRONMAN_SET_EIP_66_ERROR_EVENT_PER_ALL_DIRECTIONS_ALL_DP_MAC(4),                                                     \
    IRONMAN_SET_EIP_66_ERROR_EVENT_PER_ALL_DIRECTIONS_ALL_DP_MAC(5),                                                     \
    IRONMAN_SET_EIP_66_ERROR_EVENT_PER_ALL_DIRECTIONS_ALL_DP_MAC(6),                                                     \
    IRONMAN_SET_EIP_66_ERROR_EVENT_PER_ALL_DIRECTIONS_ALL_DP_MAC(7),                                                     \
    IRONMAN_SET_EIP_66_ERROR_EVENT_PER_ALL_DIRECTIONS_ALL_DP_MAC(8),                                                     \
    IRONMAN_SET_EIP_66_ERROR_EVENT_PER_ALL_DIRECTIONS_ALL_DP_MAC(9),                                                     \
    IRONMAN_SET_EIP_66_ERROR_EVENT_PER_ALL_DIRECTIONS_ALL_DP_MAC(10),                                                    \
    IRONMAN_SET_EIP_66_ERROR_EVENT_PER_ALL_DIRECTIONS_ALL_DP_MAC(11),                                                    \
    IRONMAN_SET_EIP_66_ERROR_EVENT_PER_ALL_DIRECTIONS_ALL_DP_MAC(12),                                                    \
    IRONMAN_SET_EIP_66_ERROR_EVENT_PER_ALL_DIRECTIONS_ALL_DP_MAC(13),                                                    \
    IRONMAN_SET_EIP_66_ERROR_EVENT_PER_ALL_DIRECTIONS_ALL_DP_MAC(14),                                                    \
MARK_END_OF_UNI_EV_CNS,                                                                                                  \
                                                                                                                         \
CPSS_PP_MACSEC_EGRESS_SEQ_NUM_ROLLOVER_E,                                                                                \
    IRONMAN_SET_EIP_66_EVENT_PER_EGRESS_DIRECTION_ALL_DP_MAC(GLOBAL_MAC_SEC_INTERRUPT_CAUSE_SEQ_NUM_ROLLOVER_E),         \
MARK_END_OF_UNI_EV_CNS

#define SINGLE_AP_DOORBELL_EVENT_MAC(_postFix, _mgUnitId, _portNum)                                        \
    PRV_CPSS_IRONMAN_MG_##_mgUnitId##_AP_DOORBELL_PORT_##_portNum##_##_postFix,    MARK_PER_PORT_INT_MAC(_portNum)

#define SINGLE_MG_PER_54_PORTS_AP_DOORBELL_EVENT_MAC(_postFix,_mgUnitId)        \
    SINGLE_AP_DOORBELL_EVENT_MAC(_postFix, _mgUnitId,        0    ), \
    SINGLE_AP_DOORBELL_EVENT_MAC(_postFix, _mgUnitId,        1    ), \
    SINGLE_AP_DOORBELL_EVENT_MAC(_postFix, _mgUnitId,        2    ), \
    SINGLE_AP_DOORBELL_EVENT_MAC(_postFix, _mgUnitId,        3    ), \
    SINGLE_AP_DOORBELL_EVENT_MAC(_postFix, _mgUnitId,        4    ), \
    SINGLE_AP_DOORBELL_EVENT_MAC(_postFix, _mgUnitId,        5    ), \
    SINGLE_AP_DOORBELL_EVENT_MAC(_postFix, _mgUnitId,        6    ), \
    SINGLE_AP_DOORBELL_EVENT_MAC(_postFix, _mgUnitId,        7    ), \
    SINGLE_AP_DOORBELL_EVENT_MAC(_postFix, _mgUnitId,        8    ), \
    SINGLE_AP_DOORBELL_EVENT_MAC(_postFix, _mgUnitId,        9    ), \
    SINGLE_AP_DOORBELL_EVENT_MAC(_postFix, _mgUnitId,       10    ), \
    SINGLE_AP_DOORBELL_EVENT_MAC(_postFix, _mgUnitId,       11    ), \
    SINGLE_AP_DOORBELL_EVENT_MAC(_postFix, _mgUnitId,       12    ), \
    SINGLE_AP_DOORBELL_EVENT_MAC(_postFix, _mgUnitId,       13    ), \
    SINGLE_AP_DOORBELL_EVENT_MAC(_postFix, _mgUnitId,       14    ), \
    SINGLE_AP_DOORBELL_EVENT_MAC(_postFix, _mgUnitId,       15    ), \
    SINGLE_AP_DOORBELL_EVENT_MAC(_postFix, _mgUnitId,       16    ), \
    SINGLE_AP_DOORBELL_EVENT_MAC(_postFix, _mgUnitId,       17    ), \
    SINGLE_AP_DOORBELL_EVENT_MAC(_postFix, _mgUnitId,       18    ), \
    SINGLE_AP_DOORBELL_EVENT_MAC(_postFix, _mgUnitId,       19    ), \
    SINGLE_AP_DOORBELL_EVENT_MAC(_postFix, _mgUnitId,       20    ), \
    SINGLE_AP_DOORBELL_EVENT_MAC(_postFix, _mgUnitId,       21    ), \
    SINGLE_AP_DOORBELL_EVENT_MAC(_postFix, _mgUnitId,       22    ), \
    SINGLE_AP_DOORBELL_EVENT_MAC(_postFix, _mgUnitId,       23    ), \
    SINGLE_AP_DOORBELL_EVENT_MAC(_postFix, _mgUnitId,       24    ), \
    SINGLE_AP_DOORBELL_EVENT_MAC(_postFix, _mgUnitId,       25    ), \
    SINGLE_AP_DOORBELL_EVENT_MAC(_postFix, _mgUnitId,       26    ), \
    SINGLE_AP_DOORBELL_EVENT_MAC(_postFix, _mgUnitId,       27    ), \
    SINGLE_AP_DOORBELL_EVENT_MAC(_postFix, _mgUnitId,       28    ), \
    SINGLE_AP_DOORBELL_EVENT_MAC(_postFix, _mgUnitId,       29    ), \
    SINGLE_AP_DOORBELL_EVENT_MAC(_postFix, _mgUnitId,       30    ), \
    SINGLE_AP_DOORBELL_EVENT_MAC(_postFix, _mgUnitId,       31    ), \
    SINGLE_AP_DOORBELL_EVENT_MAC(_postFix, _mgUnitId,       32    ), \
    SINGLE_AP_DOORBELL_EVENT_MAC(_postFix, _mgUnitId,       33    ), \
    SINGLE_AP_DOORBELL_EVENT_MAC(_postFix, _mgUnitId,       34    ), \
    SINGLE_AP_DOORBELL_EVENT_MAC(_postFix, _mgUnitId,       35    ), \
    SINGLE_AP_DOORBELL_EVENT_MAC(_postFix, _mgUnitId,       36    ), \
    SINGLE_AP_DOORBELL_EVENT_MAC(_postFix, _mgUnitId,       37    ), \
    SINGLE_AP_DOORBELL_EVENT_MAC(_postFix, _mgUnitId,       38    ), \
    SINGLE_AP_DOORBELL_EVENT_MAC(_postFix, _mgUnitId,       39    ), \
    SINGLE_AP_DOORBELL_EVENT_MAC(_postFix, _mgUnitId,       40    ), \
    SINGLE_AP_DOORBELL_EVENT_MAC(_postFix, _mgUnitId,       41    ), \
    SINGLE_AP_DOORBELL_EVENT_MAC(_postFix, _mgUnitId,       42    ), \
    SINGLE_AP_DOORBELL_EVENT_MAC(_postFix, _mgUnitId,       43    ), \
    SINGLE_AP_DOORBELL_EVENT_MAC(_postFix, _mgUnitId,       44    ), \
    SINGLE_AP_DOORBELL_EVENT_MAC(_postFix, _mgUnitId,       45    ), \
    SINGLE_AP_DOORBELL_EVENT_MAC(_postFix, _mgUnitId,       46    ), \
    SINGLE_AP_DOORBELL_EVENT_MAC(_postFix, _mgUnitId,       47    ), \
    SINGLE_AP_DOORBELL_EVENT_MAC(_postFix, _mgUnitId,       48    ), \
    SINGLE_AP_DOORBELL_EVENT_MAC(_postFix, _mgUnitId,       49    ), \
    SINGLE_AP_DOORBELL_EVENT_MAC(_postFix, _mgUnitId,       50    ), \
    SINGLE_AP_DOORBELL_EVENT_MAC(_postFix, _mgUnitId,       51    ), \
    SINGLE_AP_DOORBELL_EVENT_MAC(_postFix, _mgUnitId,       52    ), \
    SINGLE_AP_DOORBELL_EVENT_MAC(_postFix, _mgUnitId,       53    ), \
    SINGLE_AP_DOORBELL_EVENT_MAC(_postFix, _mgUnitId,       54    )

#define SET_AP_DOORBELL_EVENT_MAC(_postFix)                         \
    SINGLE_MG_PER_54_PORTS_AP_DOORBELL_EVENT_MAC(_postFix, 0),      \
    SINGLE_MG_PER_54_PORTS_AP_DOORBELL_EVENT_MAC(_postFix, 1),      \
    SINGLE_MG_PER_54_PORTS_AP_DOORBELL_EVENT_MAC(_postFix, 2)


#define IRONMAN_DOORBELL_EVENTS                         \
CPSS_SRVCPU_PORT_LINK_STATUS_CHANGED_E,                 \
    SET_AP_DOORBELL_EVENT_MAC(LINK_STATUS_CHANGE_E),    \
MARK_END_OF_UNI_EV_CNS,                                 \
                                                        \
CPSS_SRVCPU_PORT_802_3_AP_E,                            \
    SET_AP_DOORBELL_EVENT_MAC(802_3_AP_E),              \
MARK_END_OF_UNI_EV_CNS

/* Interrupt cause to unified event map for Ironman
 This Table is for unified event with extended data

  Array structure:
  1. The first element of the array and the elements after MARK_END_OF_UNI_EV_CNS are unified event types.
  2. Elements after unified event type are pairs of interrupt cause and event extended data,
     until MARK_END_OF_UNI_EV_CNS.
*/
static const GT_U32 ironman_UniEvMapTableWithExtData[] = {
    CPSS_PP_PORT_PCS_ALIGN_LOCK_LOST_E,
        PORTS_LINK_STATUS_CHANGE_EVENTS,
    MARK_END_OF_UNI_EV_CNS,

    CPSS_PP_PORT_LINK_STATUS_CHANGED_E,
        PORTS_LINK_OK_CHANGE_EVENTS,
    MARK_END_OF_UNI_EV_CNS,

    CPSS_PP_PORT_AN_HCD_FOUND_E,
        IRONMAN_PORT_AN_HCD_RESOLUTION_DONE_EVENTS,
    MARK_END_OF_UNI_EV_CNS,

    CPSS_PP_PORT_AN_RESTART_E,
        IRONAMN_PORT_AN_RESTART_DONE_EVENTS,
    MARK_END_OF_UNI_EV_CNS,

    CPSS_PP_PORT_AN_PARALLEL_DETECT_E,
        IRONMAN_PORT_INT_PM_PCS_LINK_TIMER_OUT_EVENTS,
    MARK_END_OF_UNI_EV_CNS,

    IRONMAN_DOORBELL_EVENTS,

    IRONMAN_COMMON_EVENTS,
};


#define ironmanUniEvMapTable_MG_n(_mgIndex)  \
    {CPSS_PP_EB_AUQ_PENDING_E,              PRV_CPSS_IRONMAN_MG_##_mgIndex##_MG_INTERNAL_INTERRUPT_CAUSE_AUQ_PENDING_E},                  \
    {CPSS_PP_EB_AUQ_FULL_E,                 PRV_CPSS_IRONMAN_MG_##_mgIndex##_MG_INTERNAL_INTERRUPT_CAUSE_AU_QUEUE_FULL_E},                \
    {CPSS_PP_EB_AUQ_OVER_E,                 PRV_CPSS_IRONMAN_MG_##_mgIndex##_MG_INTERNAL_INTERRUPT_CAUSE_AUQ_OVER_RUN_E},                 \
    {CPSS_PP_EB_AUQ_ALMOST_FULL_E,          PRV_CPSS_IRONMAN_MG_##_mgIndex##_MG_INTERNAL_INTERRUPT_CAUSE_AUQ_ALMOST_FULL_E},              \
    {CPSS_PP_EB_FUQ_PENDING_E,              PRV_CPSS_IRONMAN_MG_##_mgIndex##_MG_INTERNAL_INTERRUPT_CAUSE_FUQ_PENDING_E},                  \
    {CPSS_PP_EB_FUQ_FULL_E,                 PRV_CPSS_IRONMAN_MG_##_mgIndex##_MG_INTERNAL_INTERRUPT_CAUSE_FU_QUEUE_FULL_E},                \
    {CPSS_PP_MISC_GENXS_READ_DMA_DONE_E,    PRV_CPSS_IRONMAN_MG_##_mgIndex##_MG_INTERNAL_INTERRUPT_CAUSE_GENXS_READ_DMA_DONE_E},          \
    {CPSS_PP_MISC_PEX_ADDR_UNMAPPED_E,      PRV_CPSS_IRONMAN_MG_##_mgIndex##_MG_INTERNAL_INTERRUPT_CAUSE_PEX_ADDR_UNMAPPED_E},            \
    {CPSS_PP_MISC_TWSI_TIME_OUT_E,          PRV_CPSS_IRONMAN_MG_##_mgIndex##_MG_INTERNAL_INTERRUPT_CAUSE_TWSI_TIME_OUT_INT_E},            \
    {CPSS_PP_MISC_TWSI_STATUS_E,            PRV_CPSS_IRONMAN_MG_##_mgIndex##_MG_INTERNAL_INTERRUPT_CAUSE_TWSI_STATUS_INT_E},              \
    {CPSS_PP_MISC_ILLEGAL_ADDR_E,           PRV_CPSS_IRONMAN_MG_##_mgIndex##_MG_INTERNAL_INTERRUPT_CAUSE_ILEGAL_ADDR_INT_E},              \
    {CPSS_PP_PORT_802_3_AP_E,               PRV_CPSS_IRONMAN_MG_##_mgIndex##_MG_INTERNAL_INTERRUPT_CAUSE_Z80_INTERRUPT_E},                \
    {CPSS_SRVCPU_IPC_E,                     PRV_CPSS_IRONMAN_MG_##_mgIndex##_AP_DOORBELL_MAIN_IPC_E}

/* Interrupt cause to unified event map for Ironman
 This Table is for unified event without extended data
*/
static const GT_U32 ironmanUniEvMapTable[][2] =
{
    {CPSS_PP_EB_SECURITY_BREACH_UPDATE_E,    PRV_CPSS_IRONMAN_L2I_IP_BRIDGE_INTERRUPT_CAUSE_UPDATE_SECURITY_BREACH_REGISTER_INT_E         },
    {CPSS_PP_MAC_NUM_OF_HOP_EXP_E,           PRV_CPSS_IRONMAN_FDB_IP_FDB_INTERRUPT_CAUSE_REGISTER_NUM_OF_HOP_EX_P_E                       },
    {CPSS_PP_MAC_NA_LEARNED_E,               PRV_CPSS_IRONMAN_FDB_IP_FDB_INTERRUPT_CAUSE_REGISTER_NA_LEARNT_E                             },
    {CPSS_PP_MAC_NA_NOT_LEARNED_E,           PRV_CPSS_IRONMAN_FDB_IP_FDB_INTERRUPT_CAUSE_REGISTER_NA_NOT_LEARNT_E                         },
    {CPSS_PP_MAC_AGE_VIA_TRIGGER_ENDED_E,    PRV_CPSS_IRONMAN_FDB_IP_FDB_INTERRUPT_CAUSE_REGISTER_AGE_VIA_TRIGGER_ENDED_E                 },
    {CPSS_PP_MAC_UPDATE_FROM_CPU_DONE_E,     PRV_CPSS_IRONMAN_FDB_IP_FDB_INTERRUPT_CAUSE_REGISTER_AU_PROC_COMPLETED_INT_E                 },
    {CPSS_PP_MAC_MESSAGE_TO_CPU_READY_E,     PRV_CPSS_IRONMAN_FDB_IP_FDB_INTERRUPT_CAUSE_REGISTER_AU_MSG_TOCPU_READY_E                    },
    {CPSS_PP_MAC_NA_SELF_LEARNED_E,          PRV_CPSS_IRONMAN_FDB_IP_FDB_INTERRUPT_CAUSE_REGISTER_NA_SELF_LEARNED_E                       },
    {CPSS_PP_MAC_NA_FROM_CPU_LEARNED_E,      PRV_CPSS_IRONMAN_FDB_IP_FDB_INTERRUPT_CAUSE_REGISTER_N_AFROM_CPU_LEARNED_E                   },
    {CPSS_PP_MAC_NA_FROM_CPU_DROPPED_E,      PRV_CPSS_IRONMAN_FDB_IP_FDB_INTERRUPT_CAUSE_REGISTER_N_AFROM_CPU_DROPPED_E                   },
    {CPSS_PP_MAC_AGED_OUT_E,                 PRV_CPSS_IRONMAN_FDB_IP_FDB_INTERRUPT_CAUSE_REGISTER_AGED_OUT_E                              },
    {CPSS_PP_MAC_FIFO_2_CPU_EXCEEDED_E,      PRV_CPSS_IRONMAN_FDB_IP_FDB_INTERRUPT_CAUSE_REGISTER_AU_FIFO_TO_CPU_IS_FULL_E                },
    {CPSS_PP_EB_MG_ADDR_OUT_OF_RANGE_E,      PRV_CPSS_IRONMAN_FDB_IP_FDB_INTERRUPT_CAUSE_REGISTER_ADDRESS_OUT_OF_RANGE_E                  },
    {CPSS_PP_TTI_CPU_ADDRESS_OUT_OF_RANGE_E, PRV_CPSS_IRONMAN_TTI_TTI_ENGINE_INTERRUPT_CAUSE_CPU_ADDRESS_OUT_OF_RANGE_E                   },
    {CPSS_PP_PCL_MG_ADDR_OUT_OF_RANGE_E,     PRV_CPSS_IRONMAN_PCL_PCL_UNIT_INTERRUPT_CAUSE_MG_ADDR_OUTOF_RANGE_E                          },
    {CPSS_PP_MAC_BANK_LEARN_COUNTERS_OVERFLOW_E, PRV_CPSS_IRONMAN_FDB_IP_FDB_INTERRUPT_CAUSE_REGISTER_BLC_OVERFLOW_E                      },
    {CPSS_PP_TQ_PORT_MICRO_BURST_E,          PRV_CPSS_IRONMAN_TXQ_SCN_QFC_INTERRUPT_FUNCTIONAL_CAUSE_UBURST_EVENT_INT_E                   },
    {CPSS_PP_HSR_PRP_PNT_NUM_OF_READY_ENTRIES_E,PRV_CPSS_IRONMAN_L2I_IP_BRIDGE_INTERRUPT_CAUSE_PNT_NUMBER_OF_READY_ENTRIES_INTERRUPT_E    },
    {CPSS_PP_STREAM_LATENT_ERROR_E,          PRV_CPSS_IRONMAN_PREQ_IP_PREQ_INTERRUPT_CAUSE_PREQ_LATENT_ERROR_DETECTED_E                   },
    {CPSS_PP_BM_AGED_PACKET_E,               PRV_CPSS_IRONMAN_SMB_MC_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_0_INTERRUPT_AGE_ERR_E             },
    {CPSS_PP_BM_AGED_PACKET_E,               PRV_CPSS_IRONMAN_SMB_MC_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_1_INTERRUPT_AGE_ERR_E             },
    {CPSS_PP_BM_AGED_PACKET_E,               PRV_CPSS_IRONMAN_SMB_MC_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_2_INTERRUPT_AGE_ERR_E             },
    {CPSS_PP_BM_AGED_PACKET_E,               PRV_CPSS_IRONMAN_SMB_MC_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_3_INTERRUPT_AGE_ERR_E             },
    {CPSS_PP_BM_AGED_PACKET_E,               PRV_CPSS_IRONMAN_SMB_MC_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_4_INTERRUPT_AGE_ERR_E             },
    {CPSS_PP_BM_AGED_PACKET_E,               PRV_CPSS_IRONMAN_SMB_MC_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_5_INTERRUPT_AGE_ERR_E             },
    {CPSS_PP_BM_AGED_PACKET_E,               PRV_CPSS_IRONMAN_SMB_MC_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_6_INTERRUPT_AGE_ERR_E             },
    {CPSS_PP_BM_AGED_PACKET_E,               PRV_CPSS_IRONMAN_SMB_MC_INTERRUPT_AGE_CAUSE_LOGICAL_MEMORY_7_INTERRUPT_AGE_ERR_E             },
    ironmanUniEvMapTable_MG_n(0)                                                                                                           ,

};

/* Interrupt cause to unified event map for Ironman without extended data size */
static const GT_U32 ironmanUniEvMapTableSize = (sizeof(ironmanUniEvMapTable)/(sizeof(GT_U32)*2));

/**
* @internal setIronmanDedicatedEventsConvertInfo function
* @endinternal
*
* @brief   set info needed by chIntCauseToUniEvConvertDedicatedTables(...) for the
*          Ironman devices.
*/
static void setIronmanDedicatedEventsConvertInfo(IN GT_U8   devNum)
{
    /* Interrupt cause to unified event map for Ironman with extended data size */
    GT_U32 ironmanUniEvMapTableWithExtDataSize;

    ironmanUniEvMapTableWithExtDataSize = (sizeof(ironman_UniEvMapTableWithExtData)/(sizeof(GT_U32)));
    PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG(devNum)->intCtrl.uniEvMapTableWithExtDataArr   = ironman_UniEvMapTableWithExtData;

    PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG(devNum)->intCtrl.uniEvMapTableWithExtDataSize  = ironmanUniEvMapTableWithExtDataSize;
    PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG(devNum)->intCtrl.uniEvMapTableArr              = ironmanUniEvMapTable;
    PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG(devNum)->intCtrl.uniEvMapTableSize             = ironmanUniEvMapTableSize;
    PRV_SHARED_GLOBAL_VAR_CPSS_DRIVER_PP_CONFIG(devNum)->intCtrl.numReservedPorts              = 0;

    return;
}

static void ironmanUpdateIntScanTreeInfo(
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
        unitId       = UNIT_ID_GET ((PRV_CPSS_DXCH_UNIT_ENT)intrScanArr[ii].gppId);

        didError = GT_FALSE;

        baseAddr = prvCpssDxChHwUnitBaseAddrGet(devNum,unitId,&didError);

        if(didError == GT_TRUE)
        {

            /*support MG units */
            intrScanArr[ii].maskRegAddr  = CPSS_EVENT_SKIP_MASK_REG_ADDR_CNS;

            /* NOTE: this macro not 'return' */
            CPSS_LOG_ERROR_MAC("not supported unit [%d] in Ironman",
                unitId);
        }
        else
        {
            if(baseAddr > 0 && intrScanArr[ii].causeRegAddr > baseAddr)
            {
                /* ERROR : all addresses MUST be 0 based !!! */
                /* ERROR : all addresses MUST be 0 based !!! */
                /* ERROR : all addresses MUST be 0 based !!! */

               /* because we can't mask the address !!! */
               cpssOsPrintf("ERROR : Ironman interrupts tree : NODE hold causeRegAddr [0x%8.8x] but must be relative to the start of the unit [0x%8.8x] \n",
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
* @internal prvCpssDrvPpIntDefDxChIronmanMacSecRegIdInit function
* @endinternal
*
* @brief   Init macsec register ids array.
*
* */
static GT_STATUS prvCpssDrvPpIntDefDxChIronmanMacSecRegIdInit
(
    GT_VOID
)
{
    GT_U32 iter;

    /* check that Global DB size is in synch with required one */
    if (PRV_CPSS_GLOBAL_DB_AC5X_MACSEC_REGID_ARRAY_SIZE_CNS != PRV_CPSS_DXCH_IRONMAN_MACSEC_REGID_ARRAY_SIZE_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    for(iter = 0; iter < PRV_CPSS_DXCH_IRONMAN_MACSEC_REGID_ARRAY_SIZE_CNS - 1; iter++)
    {
        PRV_SHARED_COMMON_INT_DIR_DXEXMX_GLOBAL_VAR_SET(macSecRegIdNotAccessibleBeforeMacSecInitIronMan[iter],
                                                                               PRV_CPSS_DXCH_IRONMAN_MACSEC_REGID_FIRST_CNS + iter);
    }
    /* end identifier for the array */
    PRV_SHARED_COMMON_INT_DIR_DXEXMX_GLOBAL_VAR_SET(macSecRegIdNotAccessibleBeforeMacSecInitIronMan[iter],
                                                                           PRV_CPSS_MACSEC_REGID_LAST_CNS);
    return GT_OK;
}

/**
* @internal prvCpssDrvPpIntDefDxChIronmanInit function
* @endinternal
*
* @brief   Interrupts initialization for the Ironman devices.
*
* @param[in] devNum                   - the device number
* @param[in] ppRevision               - the revision of the device
*
* @retval GT_OK                    - on success,
* @retval GT_OUT_OF_CPU_MEM        - fail to allocate cpu memory (osMalloc)
* @retval GT_BAD_PARAM             - the scan tree information has error
*/
GT_STATUS prvCpssDrvPpIntDefDxChIronmanInit
(
    IN GT_U8   devNum ,
    IN GT_U32   ppRevision
)
{
    GT_STATUS   rc;         /* return code */
    PRV_CPSS_DRV_DEVICE_INTERRUPTS_INFO_STC * devInterruptInfoPtr;
                            /* pointer to device interrupt info */
    PRV_CPSS_DRV_INTERRUPTS_INFO_STC devFamilyInterrupstInfo =
    {
        NULL,
        PRV_CPSS_DRV_FAMILY_DXCH_IRONMAN_L_E,
        NUM_ELEMENTS_IN_ARR_MAC(ironmanIntrScanArr),
        ironmanIntrScanArr,
        PRV_CPSS_IRONMAN_LAST_INT_E/32,
        NULL, NULL, NULL
    };

    ppRevision = ppRevision;

    /*set info needed by chIntCauseToUniEvConvertDedicatedTables(...) */
    setIronmanDedicatedEventsConvertInfo(devNum);

    /* init macsec register ids */
    rc = prvCpssDrvPpIntDefDxChIronmanMacSecRegIdInit();
    if(rc != GT_OK)
    {
        /* error */
        return rc;
    }

    if(PRV_SHARED_COMMON_INT_DIR_DXEXMX_GLOBAL_VAR_GET(ironmanInitDone) == GT_TRUE)
    {
        return GT_OK;
    }

    rc = prvCpssDrvDxExMxInterruptsMemoryInit(&devFamilyInterrupstInfo, 1);
    if(rc != GT_OK)
    {
        /* error */
        return rc;
    }

    (void)prvCpssDxChUnitBaseTableInit(CPSS_PP_FAMILY_DXCH_IRONMAN_E);/*needed for calling prvCpssDxChHwUnitBaseAddrGet(...) */

    /* 1. Align base addresses for mask registers */
    ironmanUpdateIntScanTreeInfo(devNum,
        devFamilyInterrupstInfo.intrScanOutArr ,
        devFamilyInterrupstInfo.numScanElements);

    /*
       1. fill the array of mask registers addresses
       2. fill the array of default values for the mask registers
       3. update the bits of nonSumBitMask in the scan tree
    */
    rc = prvCpssDrvExMxDxHwPpMaskRegInfoGet(
            CPSS_PP_FAMILY_DXCH_IRONMAN_E        ,
            devFamilyInterrupstInfo.numScanElements          ,
            devFamilyInterrupstInfo.intrScanOutArr              ,
            devFamilyInterrupstInfo.numMaskRegisters         ,
            devFamilyInterrupstInfo.maskRegDefaultSummaryArr ,
            devFamilyInterrupstInfo.maskRegMapArr            );
    if(rc != GT_OK)
    {
        /* error */
        return rc;
    }

    rc = prvCpssDrvPpIntUnMappedMgSetSkip(devNum, devFamilyInterrupstInfo.maskRegMapArr, devFamilyInterrupstInfo.numMaskRegisters);
    if(rc != GT_OK)
    {
        /* error */
        return rc;
    }

    devInterruptInfoPtr = &PRV_SHARED_COMMON_INT_DIR_DXEXMX_GLOBAL_VAR_GET(prvCpssDrvDevFamilyInterruptInfoArray[devFamilyInterrupstInfo.intFamily]);
    devInterruptInfoPtr->numOfScanElements               = devFamilyInterrupstInfo.numScanElements         ;
    devInterruptInfoPtr->interruptsScanArray             = devFamilyInterrupstInfo.intrScanOutArr             ;
    devInterruptInfoPtr->maskRegistersDefaultValuesArray = devFamilyInterrupstInfo.maskRegDefaultSummaryArr;
    devInterruptInfoPtr->maskRegistersAddressesArray     = devFamilyInterrupstInfo.maskRegMapArr           ;

    devInterruptInfoPtr->numOfInterrupts = devFamilyInterrupstInfo.numMaskRegisters * 32;
    devInterruptInfoPtr->hasFakeInterrupts = GT_FALSE;
    devInterruptInfoPtr->firstFakeInterruptId = 0;/* don't care */
    devInterruptInfoPtr->drvIntCauseToUniEvConvertFunc = &prvCpssDrvPpPortGroupIntCheetahIntCauseToUniEvConvert;
    devInterruptInfoPtr->numOfInterruptRegistersNotAccessibleBeforeStartInit = 0;
    devInterruptInfoPtr->notAccessibleBeforeStartInitPtr = NULL;
    PRV_INTERRUPT_CTRL_GET(devNum).intMaskSetFptr = NULL;
    devInterruptInfoPtr->macSecRegIdArray =
        PRV_SHARED_COMMON_INT_DIR_DXEXMX_GLOBAL_VAR_GET(macSecRegIdNotAccessibleBeforeMacSecInitIronMan);

    PRV_SHARED_COMMON_INT_DIR_DXEXMX_GLOBAL_VAR_SET(ironmanInitDone, GT_TRUE);

    return GT_OK;
}

#define DUMP_DEFAULT_INFO
#ifdef DUMP_DEFAULT_INFO

/**
* @internal prvCpssDrvPpIntDefIronmanPrint function
* @endinternal
*
* @brief   print the interrupts arrays info of Ironman devices
*/
void  prvCpssDrvPpIntDefIronmanPrint(
    void
)
{
    PRV_CPSS_DRV_DEVICE_INTERRUPTS_INFO_STC * devInterruptInfoPtr;
    PRV_CPSS_DRV_INTERRUPT_SCAN_STC *  interruptsScanArray;

    cpssOsPrintf("Ironman - start : \n");
    devInterruptInfoPtr = &PRV_SHARED_COMMON_INT_DIR_DXEXMX_GLOBAL_VAR_GET(prvCpssDrvDevFamilyInterruptInfoArray[PRV_CPSS_DRV_FAMILY_DXCH_IRONMAN_L_E]);

    /* remove the 'const' from the array */
    interruptsScanArray = devInterruptInfoPtr->interruptsScanArray;
    /* port group 0 */
    prvCpssDrvPpIntDefPrint(devInterruptInfoPtr->numOfScanElements,
                            interruptsScanArray,
                            IRONMAN_NUM_MASK_REGISTERS_CNS,
                            devInterruptInfoPtr->maskRegistersAddressesArray,
                            devInterruptInfoPtr->maskRegistersDefaultValuesArray);
    cpssOsPrintf("Ironman - End : \n");

}
#endif/*DUMP_DEFAULT_INFO*/

/**
* @internal prvCpssDrvPpIntDefIronmanPrint_regInfoByInterruptIndex function
* @endinternal
*
* @brief   print for Ironman devices the register info according to value in
*         PRV_CPSS_Ironman_LAST_INT_E (assuming that this register is part of
*         the interrupts tree)
*/
void  prvCpssDrvPpIntDefIronmanPrint_regInfoByInterruptIndex(
    IN PRV_CPSS_IRONMAN_INT_CAUSE_ENT   interruptId
)
{
    PRV_CPSS_DRV_DEVICE_INTERRUPTS_INFO_STC * devInterruptInfoPtr;
    GT_CHAR*    deviceNamePtr = "Ironman interrupts";
    GT_U32      numMaskRegisters = PRV_CPSS_IRONMAN_LAST_INT_E/32;

    cpssOsPrintf("Ironman - start regInfoByInterruptIndex : \n");

    if(interruptId >= PRV_CPSS_IRONMAN_LAST_INT_E)
    {
        cpssOsPrintf("interruptId[%d] >= maximum(%d) \n" ,
            interruptId, PRV_CPSS_IRONMAN_LAST_INT_E);
    }
    else
    {
        if(PRV_SHARED_COMMON_INT_DIR_DXEXMX_GLOBAL_VAR_GET(ironmanInitDone) == GT_FALSE)
        {
            cpssOsPrintf("Ironman - ERROR : the 'IntrScanArr' was not initialized with 'base addresses' yet End \n");
            return;
        }

        if((GT_U32)interruptId >= (numMaskRegisters << 5))
        {
            cpssOsPrintf("[%s] skipped : interruptId[%d] >= maximum(%d) \n" ,
                deviceNamePtr, interruptId, (numMaskRegisters << 5));
        }

        devInterruptInfoPtr = &PRV_SHARED_COMMON_INT_DIR_DXEXMX_GLOBAL_VAR_GET(prvCpssDrvDevFamilyInterruptInfoArray[PRV_CPSS_DRV_FAMILY_DXCH_IRONMAN_L_E]);

        cpssOsPrintf("start [%s]: \n", deviceNamePtr);
        prvCpssDrvPpIntDefPrint_regInfoByInterruptIndex(devInterruptInfoPtr->numOfScanElements, devInterruptInfoPtr->interruptsScanArray, interruptId, GT_FALSE);
        cpssOsPrintf("ended [%s] \n\n", deviceNamePtr);
    }

    cpssOsPrintf("Ironman - End regInfoByInterruptIndex : \n");
}


