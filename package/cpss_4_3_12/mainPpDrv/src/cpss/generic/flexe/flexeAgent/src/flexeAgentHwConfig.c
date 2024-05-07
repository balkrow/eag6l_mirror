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
* @file flexeAgentHwConfig.c
*
* @brief FLEXE AGENT hardware configuration API implementation
*
* @version   1
********************************************************************************
*/

#include <flexeAgentTypes.h>
#include <flexeAgentRegAddr.h>
#include <flexeAgentHwConfig.h>
#include <flexeAgentTransit.h>


static GT_STATUS prvFlexeAgentCoderInit
(
    IN  GT_U8    tileId
)
{
    GT_STATUS rc = GT_OK;

    /* [TBD] */
    (void) tileId;

    return rc;
}

GT_STATUS prvFlexeAgentCoderClientConfigSet
(
    IN  GT_U8    tileId,
    IN  FLEXE_AGENT_CLIENTS_INFO_STC *initParamsPtr
)
{
    GT_STATUS rc = GT_OK;
    GT_U32    clientIdx;
    GT_U32    slotIdx;
    GT_U32    numSlots;
    GT_U32    numPulses;
    GT_U32    mask = FLEXE_REG_MASK_NONE;
    FLEXE_AGENT_PIZZA_SELECT_ENT activePizza;
    GT_U32    rateCtrlAddress, encoderDecoderAddress;
    GT_U32    pizzaSlotAddress, pizzaControlAddress, xcPizzaControlAddress;
    GT_U32    rateCtrlData = 0, encoderDecoderData = 0;
    GT_U32    pizzaSlotData = 0, pizzaControlData = 0, xcPizzaControlData = 0;

    (void) pizzaControlData;

    for (clientIdx = 0; clientIdx < initParamsPtr->numClients; clientIdx++)
    {
        /* [TBD] Get clientType */
        if (initParamsPtr->clientCfg[clientIdx].clientType == FLEXE_AGENT_CLIENT_TYPE_L1_E)
        {
            continue;
        }
        numSlots  = initParamsPtr->clientCfg[clientIdx].clientSpeed/5;
        numPulses = 8*numSlots;
        rateCtrlData = numPulses & 0x7F; /* 7 bits */
        encoderDecoderData  =(initParamsPtr->clientCfg[clientIdx].channelId & 0x7F) << 1; /* Reg[7:1] 7 bits */
        encoderDecoderData |= 0x1; /* MAC_EN */

        rateCtrlAddress       = FLEXE_CODER_BASE_ADDRESS(tileId); /* base address */
        encoderDecoderAddress = FLEXE_CODER_BASE_ADDRESS(tileId); /* base address */
        switch (initParamsPtr->clientCfg[clientIdx].macType)
        {
        case FLEXE_AGENT_MAC_TYPE_25G_E:
            rateCtrlAddress       += FLEXE_CODER_MAC_25G_RATE_CTRL_ADDR(initParamsPtr->clientCfg[clientIdx].macNum);
            encoderDecoderAddress += FLEXE_CODER_MAC_25G_ENCODER_DECODER_ADDR(initParamsPtr->clientCfg[clientIdx].macNum);
            break;
        case FLEXE_AGENT_MAC_TYPE_50G_E:
            rateCtrlAddress       += FLEXE_CODER_MAC_50G_RATE_CTRL_ADDR(initParamsPtr->clientCfg[clientIdx].macNum);
            encoderDecoderAddress += FLEXE_CODER_MAC_50G_ENCODER_DECODER_ADDR(initParamsPtr->clientCfg[clientIdx].macNum);
            break;
        case FLEXE_AGENT_MAC_TYPE_100G_E:
            rateCtrlAddress       += FLEXE_CODER_MAC_100G_RATE_CTRL_ADDR(initParamsPtr->clientCfg[clientIdx].macNum);
            encoderDecoderAddress += FLEXE_CODER_MAC_100G_ENCODER_DECODER_ADDR(initParamsPtr->clientCfg[clientIdx].macNum);
            break;
        case FLEXE_AGENT_MAC_TYPE_200G_E:
            rateCtrlAddress       += FLEXE_CODER_MAC_200G_RATE_CTRL_ADDR(initParamsPtr->clientCfg[clientIdx].macNum);
            encoderDecoderAddress += FLEXE_CODER_MAC_200G_ENCODER_DECODER_ADDR(initParamsPtr->clientCfg[clientIdx].macNum);
            break;
        case FLEXE_AGENT_MAC_TYPE_400G_E:
            rateCtrlAddress       += FLEXE_CODER_MAC_400G_RATE_CTRL_ADDR(initParamsPtr->clientCfg[clientIdx].macNum);
            encoderDecoderAddress += FLEXE_CODER_MAC_400G_ENCODER_DECODER_ADDR(initParamsPtr->clientCfg[clientIdx].macNum);
            break;
        default:
            FLEXE_ERR_CHECK_AND_RETURN_MAC(GT_BAD_VALUE,"");
        }
        /* Write MAC 25G rate ctrl register */
        rc = flexeAgentHwRegWrite(rateCtrlAddress, rateCtrlData, mask);
        FLEXE_ERR_CHECK_AND_RETURN_MAC(rc, "address:0x%X, data:%d, mask:%d",
                                       rateCtrlAddress, rateCtrlData, mask);
        /* Write Encoder Decoder MAC 25G register */
        rc = flexeAgentHwRegWrite(encoderDecoderAddress, encoderDecoderData, mask);
        FLEXE_ERR_CHECK_AND_RETURN_MAC(rc, "address:0x%X, data:%d, mask:%d",
                                       encoderDecoderAddress, encoderDecoderData, mask);

        if (initParamsPtr->clientCfg[clientIdx].updateCal == GT_TRUE)
        {
            xcPizzaControlAddress = FLEXE_CODER_BASE_ADDRESS(tileId) +
                      FLEXE_CODER_XC_PIZZA_CONTROL_ADDR(initParamsPtr->clientCfg[clientIdx].shimId);
            mask    = 0x1;
            /* Read XC Pizza Control regiter */
            rc = flexeAgentHwRegRead(xcPizzaControlAddress, &xcPizzaControlData, mask);
            FLEXE_ERR_CHECK_AND_RETURN_MAC(rc, "address:0x%X, data:%d, mask:%d",
                                       xcPizzaControlAddress, xcPizzaControlData, mask);

            activePizza = (xcPizzaControlData & 0x1) ? FLEXE_AGENT_PIZZA_SELECT_B_E : FLEXE_AGENT_PIZZA_SELECT_A_E;

            /* Read Active Pizza */
            switch (activePizza)
            {
            case FLEXE_AGENT_PIZZA_SELECT_A_E:
                /* Get Pizza B addresses */
                pizzaSlotAddress = FLEXE_CODER_PIZZA_B_SLOT_BASE_ADDR(initParamsPtr->clientCfg[clientIdx].shimId);
                pizzaControlAddress = FLEXE_CODER_PIZZA_B_CONTROL_ADDR(initParamsPtr->clientCfg[clientIdx].shimId);
                xcPizzaControlData = (GT_U32)FLEXE_AGENT_PIZZA_SELECT_B_E;
                break;
            case FLEXE_AGENT_PIZZA_SELECT_B_E:
                /* Get Pizza A addresses */
                pizzaSlotAddress = FLEXE_CODER_PIZZA_A_SLOT_BASE_ADDR(initParamsPtr->clientCfg[clientIdx].shimId);
                pizzaControlAddress = FLEXE_CODER_PIZZA_A_CONTROL_ADDR(initParamsPtr->clientCfg[clientIdx].shimId);
                xcPizzaControlData = (GT_U32)FLEXE_AGENT_PIZZA_SELECT_A_E;
                break;
            default:
                break;
            }

            for (slotIdx = 0; slotIdx < FLEXE_NUM_SLOTS_MAX; slotIdx++)
            {
                pizzaSlotAddress += FLEXE_CODER_PIZZA_SLOT_OFFSET_ADDR(slotIdx);
                pizzaSlotData = initParamsPtr->clientCfg[clientIdx].pizza[slotIdx];
                /* Write Pizza Slots registers */
                rc = flexeAgentHwRegWrite(pizzaSlotAddress, pizzaSlotData, mask);
                FLEXE_ERR_CHECK_AND_RETURN_MAC(rc, "address:0x%X, data:%d, mask:%d",
                                       pizzaSlotAddress, pizzaSlotData, mask);
            }

            pizzaControlData = ((1<<7) | 80);

            /* Write Pizza Control regiter */
            rc = flexeAgentHwRegWrite(pizzaControlAddress, pizzaSlotData, mask);
            FLEXE_ERR_CHECK_AND_RETURN_MAC(rc, "address:0x%X, data:%d, mask:%d",
                                   pizzaControlAddress, pizzaSlotData, mask);

            /* Write XC Pizza Control regiter */
            rc = flexeAgentHwRegWrite(xcPizzaControlAddress, xcPizzaControlData, mask);
            FLEXE_ERR_CHECK_AND_RETURN_MAC(rc, "address:0x%X, data:%d, mask:%d",
                                   xcPizzaControlAddress, xcPizzaControlData, mask);
        }
    }

    return rc;
}

GT_STATUS prvFlexeAgentXcClientConfigSet
(
    IN  GT_U8    tileId,
    IN  FLEXE_AGENT_CLIENTS_INFO_STC *paramsPtr
)
{
    GT_STATUS   rc = GT_OK;
    GT_U32      i = 0, n = 0;
    GT_U32      channelId, shimId, address, data, mask;
    GT_U32      mcRateCompAddr, rxDataEnAddr;

    for (i=0; i<paramsPtr->numClients; i++)
    {
        shimId    = paramsPtr->clientCfg[i].shimId;
        channelId = paramsPtr->clientCfg[i].channelId;

        /* Write MC_FIFO_WRITE_MASK register */
        address   = FLEXE_XC_CORE_BASE_ADDRESS(tileId, shimId) +
                    FLEXE_XC_CORE_RX_SLAVE_MC_FIFO_WRITE_MASK((channelId/32));
        data      = (1 << (channelId%32));
        mask      = (1 << (channelId%32));
        rc = flexeAgentHwRegWrite(address, data, mask);
        FLEXE_ERR_CHECK_AND_RETURN_MAC(rc, "address:0x%X, data:%d, mask:%d",address, data, mask);

        /* Write XC_RX_CHID_MAP */
        address   = FLEXE_XC_CORE_BASE_ADDRESS(tileId, shimId) +
                    FLEXE_XC_CORE_RX_CHID_MAP_ADDR(channelId/2);
        data      = paramsPtr->clientCfg[i].clientNum;
        mask      = (0xFFFF << (16*(channelId%2)));
        rc = flexeAgentHwRegWrite(address, data, mask);
        FLEXE_ERR_CHECK_AND_RETURN_MAC(rc, "address:0x%X, data:%d, mask:%d",address, data, mask);

        /* Write RX_SEL_OUTPUT_MAP_RAM */
        address   = FLEXE_XC_CORE_BASE_ADDRESS(tileId, shimId) +
                    FLEXE_XC_CORE_RX_SLAVE_SEL_OUTPUT_MAP_RAM_ADDR(channelId/2);
        data      = paramsPtr->clientCfg[i].clientNum;
        mask      = (0xFFFF << (16*(channelId%2)));
        rc = flexeAgentHwRegWrite(address, data, mask);
        FLEXE_ERR_CHECK_AND_RETURN_MAC(rc, "address:0x%X, data:%d, mask:%d",address, data, mask);


        rxDataEnAddr = FLEXE_XC_CORE_BASE_ADDRESS(tileId, shimId);
        data      = (1 << (channelId%32));
        mask      = (1 << (channelId%32));

        switch (paramsPtr->clientCfg[i].clientType)
        {
        case FLEXE_AGENT_CLIENT_TYPE_L1_E:
            if (shimId != paramsPtr->clientCfg[i].peerClientShimId)
            {
                mcRateCompAddr += FLEXE_XC_CORE_TX_SLAVE_MC_RATE_COMP_WRITE_MASK_A(channelId/32);
                rxDataEnAddr += FLEXE_XC_CORE_RX_SLAVE_RX_DATA_EN_MASK_A(channelId/32);
            }
            else
            {
                mcRateCompAddr += FLEXE_XC_CORE_TX_SLAVE_MC_RATE_COMP_WRITE_MASK_C(channelId/32);
                rxDataEnAddr += FLEXE_XC_CORE_RX_SLAVE_RX_DATA_EN_MASK_C(channelId/32);
            }
            break;
        case FLEXE_AGENT_CLIENT_TYPE_L2_E:
            mcRateCompAddr += FLEXE_XC_CORE_TX_SLAVE_MC_RATE_COMP_WRITE_MASK_B(channelId/32);
            rxDataEnAddr += FLEXE_XC_CORE_RX_SLAVE_RX_DATA_EN_MASK_B(channelId/32);
            break;
        default:
            break;
        }

        /* Write MC_RATE_COMP_WRITE_MASK_A/B/C */
        rc = flexeAgentHwRegWrite(mcRateCompAddr, data, mask);
        FLEXE_ERR_CHECK_AND_RETURN_MAC(rc, "address:0x%X, data:%d, mask:%d",mcRateCompAddr, data, mask);

        /* Write RX_DATA_EN_MASK_A/B/C */
        rc = flexeAgentHwRegWrite(rxDataEnAddr, data, mask);
        FLEXE_ERR_CHECK_AND_RETURN_MAC(rc, "address:0x%X, data:%d, mask:%d",rxDataEnAddr, data, mask);

        if (paramsPtr->clientCfg[i].updateCal == GT_TRUE)
        {
            for (n=0; n<40; n++)
            {
                /* Write SEL_SCH_ENTRY_RAM*/
                address = FLEXE_XC_CORE_BASE_ADDRESS(tileId, shimId) +
                          FLEXE_XC_CORE_TX_SLAVE_SEL_SCH_ENTRY_RAM_ADDR(n);
                data  = (paramsPtr->clientCfg[i].xcCal[2*n].flowType & 0x3);
                data |= (((paramsPtr->clientCfg[i].xcCal[2*n].flowType == FLEXE_AGENT_XC_FLOW_TYPE_B_E) ?
                        paramsPtr->clientCfg[i].xcCal[2*n].channelId:
                        paramsPtr->clientCfg[i].xcCal[2*n].peerChannelId) << 2);
                data |= ((paramsPtr->clientCfg[i].xcCal[2*n+1].flowType & 0x3) << 16);
                data |= (((paramsPtr->clientCfg[i].xcCal[2*n+1].flowType == FLEXE_AGENT_XC_FLOW_TYPE_B_E) ?
                        paramsPtr->clientCfg[i].xcCal[2*n+1].channelId:
                        paramsPtr->clientCfg[i].xcCal[2*n+1].peerChannelId) << 18);
                mask = 0;
                rc = flexeAgentHwRegWrite(address, data, mask);
                FLEXE_ERR_CHECK_AND_RETURN_MAC(rc, "address:0x%X, data:%d, mask:%d",address, data, mask);

                /* Write TX_SEL_OUTPUT_MAP_RAM */
                address = FLEXE_XC_CORE_BASE_ADDRESS(tileId, shimId) +
                          FLEXE_XC_CORE_TX_SLAVE_SEL_OUTPUT_MAP_RAM_ADDR(n);
                data = ((paramsPtr->clientCfg[i].xcCal[2*n].flowType == FLEXE_AGENT_XC_FLOW_TYPE_B_E) ?
                        paramsPtr->clientCfg[i].xcCal[2*n].channelId:
                        paramsPtr->clientCfg[i].xcCal[2*n].peerChannelId);
                data |= (((paramsPtr->clientCfg[i].xcCal[2*n].flowType == FLEXE_AGENT_XC_FLOW_TYPE_B_E) ?
                        paramsPtr->clientCfg[i].xcCal[2*n].channelId:
                        paramsPtr->clientCfg[i].xcCal[2*n].peerChannelId) << 16);
                mask = 0;
                rc = flexeAgentHwRegWrite(address, data, mask);
                FLEXE_ERR_CHECK_AND_RETURN_MAC(rc, "address:0x%X, data:%d, mask:%d",address, data, mask);
            }
        }

        /* Write XC_TX_CHID_MAP */
        address   = FLEXE_XC_CORE_BASE_ADDRESS(tileId, shimId) +
                    FLEXE_XC_CORE_TX_CHID_MAP_ADDR(channelId/2);
        data      = paramsPtr->clientCfg[i].clientNum;
        mask      = (0xFFFF << (16*(channelId%2)));
        rc = flexeAgentHwRegWrite(address, data, mask);
        FLEXE_ERR_CHECK_AND_RETURN_MAC(rc, "address:0x%X, data:%d, mask:%d",address, data, mask);

    }

    return rc;
}

static GT_STATUS prvFlexeAgentXcInit
(
    IN  GT_U8    tileId
)
{
    GT_STATUS   rc = GT_OK;
    GT_U32      i = 0;
    GT_U32      address;
    GT_U32      mask = FLEXE_REG_MASK_NONE;
    GT_U32      data = 0;
    GT_U32      shimId = 0;

    for (shimId = 0; shimId < FLEXE_NUM_SHIMS_MAX; shimId++)
    {
        /*Set Active Bank to B */
        /* Write TX Main Control register */
        address = FLEXE_XC_CORE_BASE_ADDRESS(tileId, shimId) +
                  FLEXE_XC_CORE_TX_SLAVE_TX_MAIN_CTRL_ADDR;
        /*Set Bits BANK_SEL: Bit[1], FLOW A/B/C RATE COMP: BIT[8/9/10]*/
        data = 0x52;
        mask = 0x72;
        rc = flexeAgentHwRegWrite(address, data, mask);
        FLEXE_ERR_CHECK_AND_RETURN_MAC(rc, "address:0x%X, data:%d, mask:%d", address, data, mask);

        /* Write RX Main Control register */
        address = FLEXE_XC_CORE_BASE_ADDRESS(tileId, shimId) +
                  FLEXE_XC_CORE_RX_SLAVE_RX_MAIN_CTRL_ADDR;
        /*Set Bits BANK_SEL: Bit[1] */
        data = 0x2;
        mask = 0x2;
        rc = flexeAgentHwRegWrite(address, data, mask);
        FLEXE_ERR_CHECK_AND_RETURN_MAC(rc, "address:0x%X, data:%d, mask:%d", address, data, mask);

        /* Write XC common control register. configure reset Bits*/
        address = FLEXE_XC_CORE_BASE_ADDRESS(tileId, shimId) + FLEXE_XC_CORE_REG_CMN_CTRL_ADDR;
        /* Set Bits XC_RESET: Bit[0], TX_DP_RESET: Bit[1], RX_DP_RESET */
        mask = 0x7;
        rc = flexeAgentHwRegWrite(address, 0x7, mask);
        FLEXE_ERR_CHECK_AND_RETURN_MAC(rc, "address:0x%X, data:%d, mask:%d", address, 0x7, mask);

        /* Initialize the Flow Control SW RAM to zero */
        /* Initialize SEL_SCH_ENTRY_RAM to 0 */
        data = 0;
        mask = FLEXE_REG_MASK_NONE;
        for (i=0; i<FLEXE_XC_CORE_TX_SLAVE_SEL_SCH_ENTRY_RAM_SIZE; i++)
        {
            address = FLEXE_XC_CORE_BASE_ADDRESS(tileId, shimId) + FLEXE_XC_CORE_TX_SLAVE_SEL_SCH_ENTRY_RAM_ADDR(i);
            rc = flexeAgentHwRegWrite(address, data, mask);
            FLEXE_ERR_CHECK_AND_RETURN_MAC(rc, "address:0x%X, data:%d, mask:%d", address, data, mask);
        }

        /* Initialize TX_SEL_OUTPUT_MAP_RAM to 0 */
        for (i=0; i<FLEXE_XC_CORE_TX_SLAVE_SEL_OUTPUT_MAP_RAM_SIZE; i++)
        {
            address = FLEXE_XC_CORE_BASE_ADDRESS(tileId, shimId) + FLEXE_XC_CORE_TX_SLAVE_SEL_OUTPUT_MAP_RAM_ADDR(i);
            rc = flexeAgentHwRegWrite(address, data, mask);
            FLEXE_ERR_CHECK_AND_RETURN_MAC(rc, "address:0x%X, data:%d, mask:%d",
                                       address, data, mask);
        }

        /* Initialize the Re-map SW RAM (RX_SEL_OUTPUT_MAP_RAM) that remaps the
           Line channel to the MAC channel */
        for (i=0; i<FLEXE_XC_CORE_RX_SLAVE_SEL_OUTPUT_MAP_RAM_SIZE; i++)
        {
            address = FLEXE_XC_CORE_BASE_ADDRESS(tileId, shimId) + FLEXE_XC_CORE_RX_SLAVE_SEL_OUTPUT_MAP_RAM_ADDR(i);
            rc = flexeAgentHwRegWrite(address, data, mask);
            FLEXE_ERR_CHECK_AND_RETURN_MAC(rc, "address:0x%X, data:%d, mask:%d",
                                       address, data, mask);
        }

        /* Initialize TX_CHID_MAP RAM */
        for (i=0; i<FLEXE_XC_CORE_TX_CHID_SIZE; i++)
        {
            address = FLEXE_XC_CORE_BASE_ADDRESS(tileId, shimId) + FLEXE_XC_CORE_TX_CHID_MAP_ADDR(i);
            rc = flexeAgentHwRegWrite(address, data, mask);
            FLEXE_ERR_CHECK_AND_RETURN_MAC(rc, "address:0x%X, data:%d, mask:%d",address, data, mask);
        }

        /* Initialize RX_CHID_MAP RAM */
        for (i=0; i<FLEXE_XC_CORE_RX_CHID_SIZE; i++)
        {
            address = FLEXE_XC_CORE_BASE_ADDRESS(tileId, shimId) + FLEXE_XC_CORE_RX_CHID_MAP_ADDR(i);
            rc = flexeAgentHwRegWrite(address, data, mask);
            FLEXE_ERR_CHECK_AND_RETURN_MAC(rc, "address:0x%X, data:%d, mask:%d",address, data, mask);
        }

        for (i=0; i<3; i++)
        {
            address = FLEXE_XC_CORE_BASE_ADDRESS(tileId, shimId) + FLEXE_XC_CORE_RX_SLAVE_MC_FIFO_WRITE_MASK(i);
            rc = flexeAgentHwRegWrite(address, data, mask);
            FLEXE_ERR_CHECK_AND_RETURN_MAC(rc, "address:0x%X, data:%d, mask:%d",address, data, mask);

            address = FLEXE_XC_CORE_BASE_ADDRESS(tileId, shimId) + FLEXE_XC_CORE_TX_SLAVE_MC_RATE_COMP_WRITE_MASK_A(i);
            rc = flexeAgentHwRegWrite(address, data, mask);
            FLEXE_ERR_CHECK_AND_RETURN_MAC(rc, "address:0x%X, data:%d, mask:%d",address, data, mask);

            address = FLEXE_XC_CORE_BASE_ADDRESS(tileId, shimId) + FLEXE_XC_CORE_TX_SLAVE_MC_RATE_COMP_WRITE_MASK_B(i);
            rc = flexeAgentHwRegWrite(address, data, mask);
            FLEXE_ERR_CHECK_AND_RETURN_MAC(rc, "address:0x%X, data:%d, mask:%d",address, data, mask);

            address = FLEXE_XC_CORE_BASE_ADDRESS(tileId, shimId) + FLEXE_XC_CORE_TX_SLAVE_MC_RATE_COMP_WRITE_MASK_C(i);
            rc = flexeAgentHwRegWrite(address, data, mask);
            FLEXE_ERR_CHECK_AND_RETURN_MAC(rc, "address:0x%X, data:%d, mask:%d",address, data, mask);
        }

        /* Write XC common control register. configure reset Bits*/
        address = FLEXE_XC_CORE_BASE_ADDRESS(tileId, shimId) + FLEXE_XC_CORE_REG_CMN_CTRL_ADDR;
        /* Set Bits XC_RESET: Bit[0], TX_DP_RESET: Bit[1], RX_DP_RESET */
        mask = 0x7;
        rc = flexeAgentHwRegWrite(address, 0x0, mask);
        FLEXE_ERR_CHECK_AND_RETURN_MAC(rc, "address:0x%X, data:%d, mask:%d",
                                       address, 0x0, mask);

        /* Set Active Bank to A */
        /* Write TX Main Control register */
        address = FLEXE_XC_CORE_BASE_ADDRESS(tileId, shimId) +
                  FLEXE_XC_CORE_TX_SLAVE_TX_MAIN_CTRL_ADDR;
        /*Set Bits BANK_SEL: Bit[1]*/
        data = 0x0;
        mask = 0x2;
        rc = flexeAgentHwRegWrite(address, data, mask);
        FLEXE_ERR_CHECK_AND_RETURN_MAC(rc, "address:0x%X, data:%d, mask:%d", address, data, mask);

        /* Write RX Main Control register */
        address = FLEXE_XC_CORE_BASE_ADDRESS(tileId, shimId) +
                  FLEXE_XC_CORE_RX_SLAVE_RX_MAIN_CTRL_ADDR;
        /*Set Bits BANK_SEL: Bit[1] */
        data = 0x0;
        mask = 0x2;
        rc = flexeAgentHwRegWrite(address, data, mask);
        FLEXE_ERR_CHECK_AND_RETURN_MAC(rc, "address:0x%X, data:%d, mask:%d", address, data, mask);

    }

    return rc;
}

static GT_STATUS prvFlexeAgentShimInit
(
    IN  GT_U8    tileId
)
{
    GT_STATUS rc = GT_OK;
    GT_U32      address;
    GT_U32      mask = FLEXE_REG_MASK_NONE;
    GT_U32      data = 0;
    GT_U32      i    = 0;
    GT_U32      j    = 0;
    GT_U32      shimId = 0;

    for (shimId = 0; shimId < FLEXE_NUM_SHIMS_MAX; shimId++)
    {

        /* Write SOC_FLEXE_CTL register reset bits */
        address = FLEXE_SHIM_CORE_BASE_ADDRESS(tileId, shimId) + FLEXE_SHIM_CORE_SOC_FLEXE_CTRL_ADDR;
        /* Set Bits SOC_RESET: Bit[0], OH_BANK_SEL: Bit[25:18], TX_LOGIC_RESET: Bit[26], RX_LOGIC_RESET: Bit[27] */
        mask = 0x0FFC0001;
        rc = flexeAgentHwRegWrite(address, 0x0FFC0001, mask);
        FLEXE_ERR_CHECK_AND_RETURN_MAC(rc, "address:0x%X, data:%d, mask:%d",
                                       address, data, mask);

        /* At hard reset 8x50G PHY configuration is defined. Thus, PAD block
           insertion (TX) and removal (RX) is required. This is done by programming
           the TX_PAD_EN and RX_PAD_EN in the SOC_CTRL_FLEXE_50G register.
        */
        /* RX_PAD_EN = 0xff, TX_PAD_EN = 0xff, INST_50G_EN = 0x15 (default values)*/
        /* [TBD] adjust these values later */
        data = 0xFFFF15;
        address = FLEXE_SHIM_CORE_BASE_ADDRESS(tileId, shimId) + FLEXE_SHIM_CORE_SOC_CTRL_FLEXE_50G_ADDR;
        rc = flexeAgentHwRegWrite(address, data, mask);
        FLEXE_ERR_CHECK_AND_RETURN_MAC(rc, "address:0x%X, data:%d, mask:%d",
                                   address, data, mask);

        /* Furthermore, the PAD period is set in the VAL field of the
           TX_FLEXE_PAD_LOCK_PERIOD register and the RX_FLEXE_PAD_LOCK_PERIOD
           register respectively. Padding is optional */
        for (i=0; i<FLEXE_NUM_INSTANCES_MAX; i++)
        {
            data = 0x27ff6;
            address = FLEXE_SHIM_CORE_BASE_ADDRESS(tileId, shimId) + FLEXE_SHIM_CORE_TX_PAD_LOCK_PERIOD_ADDR(i);
            rc = flexeAgentHwRegWrite(address, data, mask);
            FLEXE_ERR_CHECK_AND_RETURN_MAC(rc, "address:0x%X, data:%d, mask:%d",
                                       address, data, mask);

            address = FLEXE_SHIM_CORE_BASE_ADDRESS(tileId, shimId) + FLEXE_SHIM_CORE_RX_PAD_LOCK_PERIOD_ADDR(i);
            rc = flexeAgentHwRegWrite(address, data, mask);
            FLEXE_ERR_CHECK_AND_RETURN_MAC(rc, "address:0x%X, data:%d, mask:%d",
                                       address, data, mask);
        }

        /* BOND_MASK in the TX_FLEXE_CTL register (one per a 50G instance) and
           RX_FLEXE_CTL register (one per a 50G instance). This setting is done
           according to the desired group configuration for the entire Shim.*/
        for (i=0; i<FLEXE_NUM_INSTANCES_MAX; i++)
        {
            /* [TBD] Get correct value */
            data = 0x0;
            mask = 0xFF; /* Bond mask [7:0] */
            address = FLEXE_SHIM_CORE_BASE_ADDRESS(tileId, shimId) + FLEXE_SHIM_CORE_TX_FLEXE_CTRL(i);
            rc = flexeAgentHwRegWrite(address, data, mask);
            FLEXE_ERR_CHECK_AND_RETURN_MAC(rc, "address:0x%X, data:%d, mask:%d",
                                       address, data, mask);

            address = FLEXE_SHIM_CORE_BASE_ADDRESS(tileId, shimId) + FLEXE_SHIM_CORE_RX_FLEXE_CTRL(i);
            rc = flexeAgentHwRegWrite(address, data, mask);
            FLEXE_ERR_CHECK_AND_RETURN_MAC(rc, "address:0x%X, data:%d, mask:%d",
                                       address, data, mask);
        }

        /* Initialize OH FRAME RAM */
        for (i=0; i<FLEXE_NUM_INSTANCES_MAX; i++)
        {
            for (j=0; j<FLEXE_SHIM_CORE_OH_FRAME_RAM_SIZE_IN_WORDS; j++)
            {
                data = 0x0;
                mask = FLEXE_REG_MASK_NONE;
                address = FLEXE_SHIM_CORE_BASE_ADDRESS(tileId, shimId) + FLEXE_SHIM_CORE_TX_OH_FRAME_RAM_ADDR(i) + 0x4*j;
                rc = flexeAgentHwRegWrite(address, data, mask);
                FLEXE_ERR_CHECK_AND_RETURN_MAC(rc, "address:0x%X, data:%d, mask:%d",
                                           address, data, mask);

                address = FLEXE_SHIM_CORE_BASE_ADDRESS(tileId, shimId) + FLEXE_SHIM_CORE_RX_OH_FRAME_RAM_ADDR(i) + 0x4*j;
                rc = flexeAgentHwRegWrite(address, data, mask);
                FLEXE_ERR_CHECK_AND_RETURN_MAC(rc, "address:0x%X, data:%d, mask:%d",
                                           address, data, mask);
            }
        }

        /* CAL_GEN_CTRL in the TX_FLEXE_OH_CTRL register (one per a 50G instance).*/
        for (i=0; i<FLEXE_NUM_INSTANCES_MAX; i++)
        {
            /* BANK_SEL: Bit[0], CAL_GEN_CTRL: Bit[1] */
            data = 0x3;
            mask = 0x3;
            address = FLEXE_SHIM_CORE_BASE_ADDRESS(tileId, shimId) + FLEXE_SHIM_CORE_TX_FLEXE_OH_CTRL(i);
            rc = flexeAgentHwRegWrite(address, data, mask);
            FLEXE_ERR_CHECK_AND_RETURN_MAC(rc, "address:0x%X, data:%d, mask:%d", address, data, mask);
        }

        /* Initialize client registers */
        for (i=0; i<FLEXE_NUM_INSTANCES_MAX; i++)
        {
            for(j=0; j<FLEXE_NUM_CLIENTS_MAX; j++)
            {
                data = 0;
                mask = 0;

                /* Write TX_INST_A_SCHEDULE_CYCLE */
                address = FLEXE_SHIM_CORE_BASE_ADDRESS(tileId, shimId) + FLEXE_SHIM_CORE_TX_INST_A_SCHEDULE_CYCLE(i,j);
                rc = flexeAgentHwRegWrite(address, data, mask);
                FLEXE_ERR_CHECK_AND_RETURN_MAC(rc, "address:0x%X, data:%d, mask:%d", address, data, mask);

                /* Write TX_INST_A_CHANNEL_ID */
                address = FLEXE_SHIM_CORE_BASE_ADDRESS(tileId, shimId) + FLEXE_SHIM_CORE_TX_INST_A_CHANNEL_ID(i,j);
                rc = flexeAgentHwRegWrite(address, data, mask);
                FLEXE_ERR_CHECK_AND_RETURN_MAC(rc, "address:0x%X, data:%d, mask:%d", address, data, mask);

                /* Write RX_INST_A_SCHEDULE_CYCLE */
                address = FLEXE_SHIM_CORE_BASE_ADDRESS(tileId, shimId) + FLEXE_SHIM_CORE_RX_INST_A_SCHEDULE_CYCLE(i,j);
                rc = flexeAgentHwRegWrite(address, data, mask);
                FLEXE_ERR_CHECK_AND_RETURN_MAC(rc, "address:0x%X, data:%d, mask:%d", address, data, mask);

                /* Write RX_INST_A_SCHEDULE_CYCLE */
                address = FLEXE_SHIM_CORE_BASE_ADDRESS(tileId, shimId) + FLEXE_SHIM_CORE_RX_INST_A_CHANNEL_ID(i,j);
                rc = flexeAgentHwRegWrite(address, data, mask);
                FLEXE_ERR_CHECK_AND_RETURN_MAC(rc, "address:0x%X, data:%d, mask:%d", address, data, mask);
            }
        }

        /* Write SOC_FLEXE_CTL register reset bits */
        address = FLEXE_SHIM_CORE_BASE_ADDRESS(tileId, shimId) + FLEXE_SHIM_CORE_SOC_FLEXE_CTRL_ADDR;
        /* Set Bits SOC_RESET: Bit[0], TX_LOGIC_RESET: Bit[26], RX_LOGIC_RESET: Bit[27] */
        mask = 0xC1;
        rc = flexeAgentHwRegWrite(address, 0x0, mask);
        FLEXE_ERR_CHECK_AND_RETURN_MAC(rc, "address:0x%X, data:%d, mask:%d",
                                       address, data, mask);

    }
    return rc;
}


/** [TBD] comments */
GT_STATUS   flexeAgentHwInit
(
    IN GT_U8       tileId
)
{
    GT_STATUS rc = GT_OK;

    /* Coder init failed */
    rc = prvFlexeAgentCoderInit(tileId);
    FLEXE_ERR_CHECK_AND_RETURN_MAC(rc, "Cross connet initialization failed");

    /* Initialize cross connect */
    rc = prvFlexeAgentXcInit(tileId);
    FLEXE_ERR_CHECK_AND_RETURN_MAC(rc, "Cross connet initialization failed");

    /* Initialize Shim */
    rc = prvFlexeAgentShimInit(tileId);
    FLEXE_ERR_CHECK_AND_RETURN_MAC(rc, "Shim initialization failed");

    return rc;
}

GT_STATUS flexeAgentGroupCreate
(
    IN  GT_U8                        tileId,
    IN  FLEXE_AGENT_GROUP_CONFIG_STC *paramsPtr
)
{
    GT_STATUS   rc = GT_OK;
    GT_U32      i;
    GT_U32      address;
    GT_U32      data = 0;
    GT_U32      mask = 0;
    GT_U32      instanceId, instanceId1;

    for (i=0; i<paramsPtr->numInstances; i++)
    {
        instanceId = paramsPtr->instance[i].id;
        instanceId1 = paramsPtr->instance[i].id1;
        /* Clear interleave Cfg reg for Tx and Rx before group create */
        /* Write TX_INTERLEAVE_CFG */
        address = FLEXE_SHIM_CORE_BASE_ADDRESS(tileId, paramsPtr->shimId) +
                  FLEXE_SHIM_CORE_TX_INTERLEAVE_CFG_ADDR(instanceId);
        mask = 0x3;
        data = 0;
        rc = flexeAgentHwRegWrite(address, data, mask);
        FLEXE_ERR_CHECK_AND_RETURN_MAC(rc, "address:0x%X, data:%d, mask:%d",
                                   address, data, mask);
        /* Write RX_INTERLEAVE_CFG */
        address = FLEXE_SHIM_CORE_BASE_ADDRESS(tileId, paramsPtr->shimId) +
                  FLEXE_SHIM_CORE_RX_INTERLEAVE_CFG_ADDR(instanceId);
        rc = flexeAgentHwRegWrite(address, data, mask);
        FLEXE_ERR_CHECK_AND_RETURN_MAC(rc, "address:0x%X, data:%d, mask:%d",
                                   address, data, mask);

        /* Write SOC_CTRL_FLEXE_50G Register */
        address = FLEXE_SHIM_CORE_BASE_ADDRESS(tileId, paramsPtr->shimId) +
                  FLEXE_SHIM_CORE_SOC_CTRL_FLEXE_50G_ADDR;
        if (paramsPtr->instance[i].type == FLEXE_AGENT_GROUP_INSTANCE_TYPE_50G_E)
        {
            /* RX_PAD_EN: Bit[19:12], TX_PAD_EN: Bit[11:4], INST_50G_EN[3:0]*/
            data = ((1<<instanceId) << 12) | ((1<<(instanceId) << 4) | (1<<(instanceId/2)));
            mask = data;
        }
        else
        {
            /* RX_PAD_EN: Bit[19:12], TX_PAD_EN: Bit[11:4], INST_50G_EN[3:0]*/
            data  = ((1<<instanceId) << 12) | ((1<<(instanceId) << 4) | (1<<(instanceId/2)));
            data |= ((1<<instanceId1) << 12) | ((1<<(instanceId1) << 4) | (1<<(instanceId1/2)));
            mask  = data;
        }
        rc = flexeAgentHwRegWrite(address, data, mask);
        FLEXE_ERR_CHECK_AND_RETURN_MAC(rc, "address:0x%X, data:%d, mask:%d", address, data, mask);


        /* WRITE TX_FLEXE_CTRL BOND_MASK configuration */
        address = FLEXE_SHIM_CORE_BASE_ADDRESS(tileId, paramsPtr->shimId) + FLEXE_SHIM_CORE_TX_FLEXE_CTRL(instanceId);
        mask = 0xFF;
        data = paramsPtr->instance[i].bondMask;
        rc = flexeAgentHwRegWrite(address, data, mask);
        FLEXE_ERR_CHECK_AND_RETURN_MAC(rc, "address:0x%X, data:%d, mask:%d",
                                   address, data, mask);

        /* Set OPTION_EN fields in INTERLEAVE_CFG register */
        if (paramsPtr->interleaveCfg)
        {
            /* Write TX_INTERLEAVE_CFG */
            address = FLEXE_SHIM_CORE_BASE_ADDRESS(tileId, paramsPtr->shimId) + FLEXE_SHIM_CORE_TX_INTERLEAVE_CFG_ADDR(instanceId);
            mask = 0x3;
            data = paramsPtr->interleaveCfg;
            rc = flexeAgentHwRegWrite(address, data, mask);
            FLEXE_ERR_CHECK_AND_RETURN_MAC(rc, "address:0x%X, data:%d, mask:%d",
                                       address, data, mask);

            /* Write RX_INTERLEAVE_CFG */
            address = FLEXE_SHIM_CORE_BASE_ADDRESS(tileId, paramsPtr->shimId) + FLEXE_SHIM_CORE_RX_INTERLEAVE_CFG_ADDR(i);
            rc = flexeAgentHwRegWrite(address, data, mask);
            FLEXE_ERR_CHECK_AND_RETURN_MAC(rc, "address:0x%X, data:%d, mask:%d",
                                       address, data, mask);
        }
    }

    /* [TBD] OH Frame write */
    return rc;
}

GT_STATUS flexeAgentShimClientConfigSet
(
    IN  GT_U8                        tileId,
    IN  FLEXE_AGENT_CLIENTS_INFO_STC *paramsPtr
)
{
    GT_STATUS   rc = GT_OK;

    GT_U32      i  = 0;
    GT_U32      numSlots;

    for (i=0; i<paramsPtr->numClients; i++)
    {
        numSlots = paramsPtr->clientCfg[i].clientSpeed;
    }
    (void) numSlots;
#if 0
        /* [TBD] Write Proper values */
        for (i=0; i<FlEXE_NUM_INSTANCES_MAX; i++)
        {
            for(j=0; j<FLEXE_NUM_CLIENTS_MAX; j++)
            {
                data = 0;
                mask = 0;

                /* Write TX_INST_A_SCHEDULE_CYCLE */
                address = FLEXE_SHIM_CORE_BASE_ADDRESS(tileId, shimId) + FLEXE_SHIM_CORE_TX_INST_A_SCHEDULE_CYCLE(i,j);
                rc = flexeAgentHwRegWrite(address, data, mask);
                FLEXE_ERR_CHECK_AND_RETURN_MAC(rc, "address:0x%X, data:%d, mask:%d", address, data, mask);

                /* Write TX_INST_A_CHANNEL_ID */
                address = FLEXE_SHIM_CORE_BASE_ADDRESS(tileId, shimId) + FLEXE_SHIM_CORE_TX_INST_A_CHANNEL_ID(i,j);
                rc = flexeAgentHwRegWrite(address, data, mask);
                FLEXE_ERR_CHECK_AND_RETURN_MAC(rc, "address:0x%X, data:%d, mask:%d", address, data, mask);

                /* Write RX_INST_A_SCHEDULE_CYCLE */
                address = FLEXE_SHIM_CORE_BASE_ADDRESS(tileId, shimId) + FLEXE_SHIM_CORE_RX_INST_A_SCHEDULE_CYCLE(i,j);
                rc = flexeAgentHwRegWrite(address, data, mask);
                FLEXE_ERR_CHECK_AND_RETURN_MAC(rc, "address:0x%X, data:%d, mask:%d", address, data, mask);

                /* Write RX_INST_A_SCHEDULE_CYCLE */
                address = FLEXE_SHIM_CORE_BASE_ADDRESS(tileId, shimId) + FLEXE_SHIM_CORE_RX_INST_A_CHANNEL_ID(i,j);
                rc = flexeAgentHwRegWrite(address, data, mask);
                FLEXE_ERR_CHECK_AND_RETURN_MAC(rc, "address:0x%X, data:%d, mask:%d", address, data, mask);
            }
        }
#endif
    return rc;
}

GT_STATUS flexeAgentGroupDelete
(
    IN  GT_U8       tileId,
    IN  GT_U8       instanceBmp,
    IN  GT_U8       *bondMaskArrPtr,
    IN  GT_U8       interleaveCfg
)
{
    GT_STATUS   rc = GT_OK;

    return rc;
}

GT_STATUS flexeAgentCalendarSwitch
(
    IN GT_U8        tileId
)
{
    GT_STATUS   rc = GT_OK;

    return rc;
}
