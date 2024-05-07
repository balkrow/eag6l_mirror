/*******************************************************************************
*              (c), Copyright 2023, Marvell International Ltd.                 *
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
* @file flexeAgentRegAddr.h
*
* @brief FLEXE AGENT registers address definitions
*
* @version   1
********************************************************************************
*/
#ifndef FLEXE_AGENT_REG_ADDR_H
#define FLEXE_AGENT_REG_ADDR_H

#include <flexeAgent.h>


#define SDMA_BASE_ADDRESS(n)        (MG_BASE_ADDRESS(n) + 0x20C)
#define SDMA_SIZE_ADDRESS(n)        (MG_BASE_ADDRESS(n) + 0x210)
#define SDMA_WINDOW_ADDRESS(n)      (MG_BASE_ADDRESS(n) + 0x254)

#define FLEXE_REG_MASK_NONE 0


/* [TBD] find correct base address */
#define FLEXE_BASE_ADDRESS(tileId)                  0x0

/* Coder registers */
/* [TBD] find correct base address */
#define FLEXE_CODER_BASE_ADDRESS(tileId)                       (0x30000000 + 0x10000000*tileId)
#define FLEXE_CODER_MAC_25G_ENCODER_DECODER_ADDR(mac25GNum)    (0x00000100 + mac25GNum*8)
#define FLEXE_CODER_MAC_25G_RATE_CTRL_ADDR(mac25GNum)          (0x00000104 + mac25GNum*8)
#define FLEXE_CODER_MAC_50G_ENCODER_DECODER_ADDR(mac50GNum)    (0x00000400 + mac50GNum*8)
#define FLEXE_CODER_MAC_50G_RATE_CTRL_ADDR(mac50GNum)          (0x00000404 + mac50GNum*8)
#define FLEXE_CODER_MAC_100G_ENCODER_DECODER_ADDR(mac100GNum)  (0x00000500 + mac100GNum*8)
#define FLEXE_CODER_MAC_100G_RATE_CTRL_ADDR(mac100GNum)        (0x00000504 + mac100GNum*8)
#define FLEXE_CODER_MAC_200G_ENCODER_DECODER_ADDR(mac200GNum)  (0x00000600 + mac200GNum*8)
#define FLEXE_CODER_MAC_200G_RATE_CTRL_ADDR(mac200GNum)        (0x00000604 + mac200GNum*8)
#define FLEXE_CODER_MAC_400G_ENCODER_DECODER_ADDR(mac400GNum)  (0x00000700 + mac400GNum*8)
#define FLEXE_CODER_MAC_400G_RATE_CTRL_ADDR(mac400GNum)        (0x00000704 + mac400GNum*8)
#define FLEXE_CODER_XC_PIZZA_CONTROL_ADDR(shimId)              (0x00000800 + 0x600*shimId)
#define FLEXE_CODER_PIZZA_A_CONTROL_ADDR(shimId)               (0x00000804 + 0x600*shimId)
#define FLEXE_CODER_PIZZA_B_CONTROL_ADDR(shimId)               (0x00000808 + 0x600*shimId)
#define FLEXE_CODER_PIZZA_A_SLOT_BASE_ADDR(shimId)             (0x00000A00 + 0x600*shimId)
#define FLEXE_CODER_PIZZA_B_SLOT_BASE_ADDR(shimId)             (0x00000C00 + 0x600*shimId)
#define FLEXE_CODER_PIZZA_SLOT_OFFSET_ADDR(slotNum)            (4*slotNum)


/* SHIM Core registers */
/* [TBD] find correct base address */
#define FLEXE_SHIM_CORE_BASE_ADDRESS(tileId, shimId) (0x10000000*tileId + 0x1000000*shimId + 0x100000)
#define FLEXE_SHIM_CORE_SOC_FLEXE_CTRL_ADDR         0x0C000001
#define FLEXE_SHIM_CORE_SOC_CTRL_FLEXE_50G_ADDR      0x00000014

#define FLEXE_SHIM_CORE_TX_FLEXE_CTRL(instNum)           (0x00001000 + 0x4000*instNum)
#define FLEXE_SHIM_CORE_TX_PAD_LOCK_PERIOD_ADDR(instNum) (0x00001038 + 0x4000*instNum)
#define FLEXE_SHIM_CORE_TX_INTERLEAVE_CFG_ADDR(instNum)  (0x0000103C + 0x4000*instNum)
#define FLEXE_SHIM_CORE_TX_OH_FRAME_RAM_ADDR(instNum)    (0x00003800 + 0x4000*instNum)
#define FLEXE_SHIM_CORE_TX_FLEXE_OH_CTRL(instNum)        (0x00002800 + 0x4000*instNum)
#define FLEXE_SHIM_CORE_TX_INST_A_SCHEDULE_CYCLE(instNum,clientId) (0x00001200 + 0x4000*instNum + clientId*4)
#define FLEXE_SHIM_CORE_TX_INST_A_CHANNEL_ID(instNum,clientId)     (0x00001344 + 0x4000*instNum + clientId*4)

#define FLEXE_SHIM_CORE_RX_FLEXE_CTRL(instNum)           (0x00001100 + 0x4000*instNum)
#define FLEXE_SHIM_CORE_RX_PAD_LOCK_PERIOD_ADDR(instNum) (0x00001154 + 0x4000*instNum)
#define FLEXE_SHIM_CORE_RX_INTERLEAVE_CFG_ADDR(instNum)  (0x00001160 + 0x4000*instNum)
#define FLEXE_SHIM_CORE_RX_OH_FRAME_RAM_ADDR(instNum)    (0x00004800 + 0x4000*instNum)
#define FLEXE_SHIM_CORE_RX_INST_A_SCHEDULE_CYCLE(instNum,clientId) (0x00001600 + 0x4000*instNum + clientId*4)
#define FLEXE_SHIM_CORE_RX_INST_A_CHANNEL_ID(instNum,clientId)     (0x00001744 + 0x4000*instNum + clientId*4)

#define FLEXE_SHIM_CORE_OH_FRAME_RAM_SIZE_IN_WORDS  512

/* XC Core registers */
/* [TBD] find correct base address */
#define FLEXE_XC_CORE_BASE_ADDRESS(tileId, shimId) (0x10000000*tileId + 0x1000000*shimId + 0x200000)
#define FLEXE_XC_CORE_REG_CMN_CTRL_ADDR             0x0
#define FLEXE_XC_CORE_TX_CHID_MAP_ADDR(entryNum)         (0x10 + 0x4*entryNum)
#define FLEXE_XC_CORE_TX_CHID_SIZE                  40
#define FLEXE_XC_CORE_RX_CHID_MAP_ADDR(entryNum)         (0xB0 + 0x4*entryNum)
#define FLEXE_XC_CORE_RX_CHID_SIZE                  40

/* TX Slave */
#define FLEXE_XC_CORE_TX_SLAVE_TX_MAIN_CTRL_ADDR            0x00001000
#define FLEXE_XC_CORE_TX_SLAVE_SEL_SCH_ENTRY_RAM_ADDR(i)    (0x00001500 + i*0x4)
#define FLEXE_XC_CORE_TX_SLAVE_SEL_SCH_ENTRY_RAM_SIZE       40
#define FLEXE_XC_CORE_TX_SLAVE_SEL_OUTPUT_MAP_RAM_ADDR(i)   (0x00001600 + i*0x4)
#define FLEXE_XC_CORE_TX_SLAVE_SEL_OUTPUT_MAP_RAM_SIZE      40
#define FLEXE_XC_CORE_TX_SLAVE_MC_RATE_COMP_WRITE_MASK_A(i) (0x00001080 + i*0x4)
#define FLEXE_XC_CORE_TX_SLAVE_MC_RATE_COMP_WRITE_MASK_B(i) (0x0000108C + i*0x4)
#define FLEXE_XC_CORE_TX_SLAVE_MC_RATE_COMP_WRITE_MASK_C(i) (0x00001098 + i*0x4)

/* RX Slave */
#define FLEXE_XC_CORE_RX_SLAVE_RX_MAIN_CTRL_ADDR            0x00002000
#define FLEXE_XC_CORE_RX_SLAVE_SEL_OUTPUT_MAP_RAM_ADDR(i)   (0x00002200 + i*0x4)
#define FLEXE_XC_CORE_RX_SLAVE_SEL_OUTPUT_MAP_RAM_SIZE      40
#define FLEXE_XC_CORE_RX_SLAVE_MC_FIFO_WRITE_MASK(i)        (0x00002024 + i*0x4)
#define FLEXE_XC_CORE_RX_SLAVE_RX_DATA_EN_MASK_A(i)         (0x00002080 + i*0x4)
#define FLEXE_XC_CORE_RX_SLAVE_RX_DATA_EN_MASK_B(i)         (0x0000208C + i*0x4)
#define FLEXE_XC_CORE_RX_SLAVE_RX_DATA_EN_MASK_C(i)         (0x00002098 + i*0x4)

GT_VOID flexeAgentBaseAddressGet
(
    IN  GT_U8       tileId,
    OUT GT_U32      *baseAddress
);

#ifndef FLEXE_AGENT_IN_CPSS
GT_VOID flexeAgentHwAccessLock
(
    IN GT_U8 hwSem
);

GT_VOID flexeAgentHwAccessUnLock
(
    IN GT_U8 hwSem
);
#endif

GT_STATUS flexeAgentRegisterFieldSet
(
    GT_U32  regAddr,
    GT_U32  fieldOffset,
    GT_U32  fieldLength,
    GT_U32  fieldData
);

GT_STATUS flexeAgentRegisterFieldGet
(
    GT_U32  regAddr,
    GT_U32  fieldOffset,
    GT_U32  fieldLength,
    GT_U32  *fieldDataPtr
);

#if 0
MV_U32 prvPpReadRegister(MV_U32 addr);
void prvPpWriteRegister(MV_U32 addr, MV_U32 val);

/**
* @internal unitsRegRead function
* @endinternal
*
* @brief   Read a switch address space register using 2nd window, previous window base is preserved
*
* @param offset           - absolute offset of regiser
*
* @retval MV_U32          - the value of the wanted register
*/
MV_U32 unitsRegRead(MV_U32 offset);

/**
* @internal unitsRegWrite function
* @endinternal
*
* @brief   Write a switch address space register using 2nd window, previous window base is preserved
*
* @param offset           - absolute offset of regiser
* @param value            - value to write
*
* @retval none
*/
MV_U32 unitsRegWrite(MV_U32 offset, MV_U32 value);
#endif

#endif

