/*******************************************************************************
Copyright (C) 2014 - 2022, Marvell International Ltd. and its affiliates
If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.
*******************************************************************************/

/********************************************************************
This file contains functions for enabling and configuring the MACSec block,
reading the statistics, enabling interrupts and various operations for
the Marvell CUX3610/CUE2610 MACSec enable Ethernet PHYs.
********************************************************************/
#include "mtdFeatures.h"
#include "mtdApiTypes.h"
#include "mtdHwCntl.h"
#include "mtdAPI.h"
#include "mtdIntr.h"
#include "mtdRSMACSecHwCntl.h"
#include "macsec_api.h"
#include "mtdRSMACSecTypes.h"
#include "mtdRSMACSecAPI.h"

#ifdef MACSEC_API_IEEE
#include "mcs_internals_ieee.h"
#include "macsec_ieee_api.h"
#else
#include "mcs_internals_CT.h"
#include "macsec_CT_api.h"
#endif

MTD_VOID mtdRSMACSecGetAPIVersion
(
    OUT MTD_U8* major,
    OUT MTD_U8* minor,
    OUT MTD_U8* buildID
)
{
    *major = MACSEC_API_MAJOR_VERSION;
    *minor = MACSEC_API_MINOR_VERSION;
    *buildID = MACSEC_API_BUILD_ID;
}

MTD_STATUS mtdRSMACSecConfigEnable
(
    IN MTD_DEV_PTR pDev,
    IN MTD_U16 mdioPort,
    IN MTD_U32 configOptions
)
{
#ifndef MTD_RS_MACSEC
    MTD_DBG_ERROR("mtdRSMACSecConfigEnable: MACSec not supported\n");
    return MTD_FAIL;
#endif

    pDev->rsMACSecCntl.rsMACSecPort = mdioPort;

    /* Enable MMAC blocks */
    MTD_ATTEMPT(mtdHwXmdioWrite(pDev, mdioPort, MTD_MACSEC_DEV_3, MTD_MPM_CONFIG_REG, 0x3002));

    MTD_ATTEMPT(mtdHwSetPhyRegField(pDev, mdioPort, MTD_MACSEC_DEV_3, MTD_BLOCK_RESET_REGISTER, 6, 1, 0x1));

#ifdef MACSEC_API_IEEE
    switch (pDev->deviceId)
    {
        case MTD_REV_X3610M_A1:
        case MTD_REV_X3610M_A2:
        case MTD_REV_X3610M_B0:
        case MTD_REV_X3610T_A1:
        case MTD_REV_X3610T_A2:
        case MTD_REV_X3610T_B0:
        case MTD_REV_E2610M_A1:
        case MTD_REV_E2610M_A2:
        case MTD_REV_E2610M_B0:
        case MTD_REV_E2610T_A1:
        case MTD_REV_E2610T_A2:
        case MTD_REV_E2610T_B0:
            MTD_ATTEMPT(mtdHwXmdioWrite(pDev, mdioPort, MTD_MACSEC_DEV_3, MTD_MMAC_CONTROL_REGISTER, 0x8000));
            break;

        default:
            MTD_DBG_ERROR("mtdRSMACSecConfigEnable: MACSec not supported on this device.\n");
            return MTD_FAIL; 
            break;
    }
#else
    switch (pDev->deviceId)
    {
        case MTD_REV_X3610T_A1:
        case MTD_REV_X3610T_A2:
        case MTD_REV_X3610T_B0:
        case MTD_REV_E2610T_A1:
        case MTD_REV_E2610T_A2:
        case MTD_REV_E2610T_B0:
            MTD_ATTEMPT(mtdHwXmdioWrite(pDev, mdioPort, MTD_MACSEC_DEV_3, MTD_MMAC_CONTROL_REGISTER, 0x0000));
            break;

        default:
            MTD_DBG_ERROR("mtdRSMACSecConfigEnable: MACSec not supported on this device.\n");
            return MTD_FAIL;
            break;
    }
#endif

    MTD_ATTEMPT(mtdHwXmdioWrite(pDev, mdioPort, MTD_MACSEC_DEV_3, MTD_MPM_MISC_REG, 0xe00));
    MTD_ATTEMPT(mtdHwXmdioWrite(pDev, mdioPort, MTD_MACSEC_DEV_3, MTD_MPM_SOFT_RESET_REG, 0x0));
    MTD_ATTEMPT(mtdRegWrite32(pDev, mdioPort, MTD_MACSEC_UNIT, MTD_SLC_CFG_GEN, 0x7ed00f0f));
    MTD_ATTEMPT(mtdRegWrite32(pDev, mdioPort, MTD_MACSEC_UNIT, MTD_PORT_MAC_SA0, 0x2bec));
    MTD_ATTEMPT(mtdRegWrite32(pDev, mdioPort, MTD_MACSEC_UNIT, MTD_PORT_MAC_SA1, 0x70f4b234));
    MTD_ATTEMPT(mtdRegWrite32(pDev, mdioPort, MTD_MACSEC_UNIT, MTD_PAUSE_CTL, 0x304));
    MTD_ATTEMPT(mtdRegWrite32(pDev, mdioPort, MTD_MACSEC_UNIT, MTD_EDB_THRESH, 0x42085));
    MTD_ATTEMPT(mtdRegWrite32(pDev, mdioPort, MTD_MACSEC_UNIT, MTD_IDB_THRESH, 0x4d09a));
    MTD_ATTEMPT(mtdRegWrite32(pDev, mdioPort, MTD_MACSEC_UNIT, MTD_CFG_VLAN_ET, 0x7529));
    MTD_ATTEMPT(mtdRegWrite32(pDev, mdioPort, MTD_MACSEC_UNIT, MTD_PTP_CFG1, 0x130e));
    MTD_ATTEMPT(mtdRegWrite32(pDev, mdioPort, MTD_MACSEC_UNIT, MTD_PTP_CFG3, 0x10c03));
    MTD_ATTEMPT(mtdRegWrite32(pDev, mdioPort, MTD_MACSEC_UNIT, MTD_QOS_FC, 0x4));
    MTD_ATTEMPT(mtdRegWrite32(pDev, mdioPort, MTD_MACSEC_UNIT, MTD_QOS_ETHERTYPE, 0x88a88100));
    MTD_ATTEMPT(mtdRegWrite32(pDev, mdioPort, MTD_MACSEC_UNIT, MTD_SMC_MAC_PREEMPT_CTRL, 0x400));
    MTD_ATTEMPT(mtdRegWrite32(pDev, mdioPort, MTD_MACSEC_UNIT, MTD_SMC_MAC_PREEMPT_STAT, 0x400));

    /* Tx/Rx DELAY_CFG_0 */ 
    MTD_ATTEMPT(mtdRegWrite32(pDev, mdioPort, MTD_MACSEC_UNIT, MTD_MCS_RX_DELAY_CFG_0, 0x1000004B));
    MTD_ATTEMPT(mtdRegWrite32(pDev, mdioPort, MTD_MACSEC_UNIT, MTD_MCS_TX_DELAY_CFG_0, 0x10000047));

    if (MTD_IS_X3610_DEVICE(pDev) && (pDev->deviceId & MTD_X36X0BASE_SINGLE_PORTA1))
    {
        /* enhancement for A1 device only */
        MTD_ATTEMPT(mtdHwSetPhyRegField(pDev, mdioPort, MTD_MACSEC_DEV_3, MTD_MMAC_CONTROL_REGISTER, 8, 1, 0x1));
        MTD_ATTEMPT(mtdRegWrite32(pDev, mdioPort, MTD_MACSEC_UNIT, MTD_EDB_THRESH, 0x200600));
        MTD_ATTEMPT(mtdRegWrite32(pDev, mdioPort, MTD_MACSEC_UNIT, MTD_IDB_THRESH, 0x200600));
    }

    /* LINK_RESET_CFG */ 
    MTD_ATTEMPT(mtdRegWrite32(pDev, mdioPort, MTD_MACSEC_UNIT, MTD_MAC_LINK_RESET_CFG, 0x03000000));

    /* disable MPM bypass */ 
    MTD_ATTEMPT(mtdHwXmdioWrite(pDev, mdioPort, MTD_MACSEC_DEV_3, 0xD81D, 0x8000));

    /* soft reset mmac and ptp */ 
    MTD_ATTEMPT(mtdHwXmdioWrite(pDev, mdioPort, MTD_MACSEC_DEV_3, MTD_MPM_SOFT_RESET_REG, 0x500));
    MTD_ATTEMPT(mtdHwXmdioWrite(pDev, mdioPort, MTD_MACSEC_DEV_3, MTD_MPM_SOFT_RESET_REG, 0x0));

    return MTD_OK;
}


MTD_STATUS mtdRSMACSecUnloadDriver
(
    IN MTD_DEV_PTR pDev,
    IN MTD_U16 mdioPort
)
{
/*    MTD_DBG_INFO("mtdRSMACSecUnloadDriver Called.\n"); */

    pDev->fmtdReadMdio =  NULL;
    pDev->fmtdWriteMdio = NULL;
    pDev->fmtdWaitFunc = NULL;

    return MTD_OK;
}

/************************* MIB Statistics API ********************************/
MTD_STATUS mtdGetRxSMCStats
(
    IN MTD_DEV_PTR pDev, 
    IN MTD_U16 mdioPort,
    IN MTD_RX_SMC_STATS *stats
)
{
    MTD_U64 counterMask = 0xFFFFFFFFFFF;
    MTD_U16 regAddr = MTD_SMAC_RX_STATS_REG;
    MTD_U64 regVal;

    MTD_DBG_INFO("mtdGetRxSMCStats:\n");

    MTD_ATTEMPT(mtdIndirectRegRead64(pDev, mdioPort, regAddr, &regVal));
    stats->goodOctets = regVal & counterMask;
    MTD_DBG_INFO("Good frame octets goodOctets: %llu\n", stats->goodOctets);

    regAddr += 4;
    MTD_ATTEMPT(mtdIndirectRegRead64(pDev, mdioPort, regAddr, &regVal));
    stats->errorOctets = regVal & counterMask;
    MTD_DBG_INFO("Bad frame octets errorOctets: %llu\n", stats->errorOctets);

    regAddr += 4;
    MTD_ATTEMPT(mtdIndirectRegRead64(pDev, mdioPort, regAddr, &regVal));
    stats->jabber = regVal & counterMask;
    MTD_DBG_INFO("Frame of size  MTU and bad CRC jabber: %llu\n", stats->jabber);

    regAddr += 4;
    MTD_ATTEMPT(mtdIndirectRegRead64(pDev, mdioPort, regAddr, &regVal));
    stats->fragment = regVal & counterMask;
    MTD_DBG_INFO("Frame 64B with bad CRC fragment: %llu\n", stats->fragment);

    regAddr += 4;
    MTD_ATTEMPT(mtdIndirectRegRead64(pDev, mdioPort, regAddr, &regVal));
    stats->undersize = regVal & counterMask;
    MTD_DBG_INFO("Frame 64B with good CRC undersize: %llu\n", stats->undersize);

    regAddr += 4;
    MTD_ATTEMPT(mtdIndirectRegRead64(pDev, mdioPort, regAddr, &regVal));
    stats->oversize = regVal & counterMask;
    MTD_DBG_INFO("Frame of size MTU and good CRC oversize: %llu\n", stats->oversize);

    regAddr += 4;
    MTD_ATTEMPT(mtdIndirectRegRead64(pDev, mdioPort, regAddr, &regVal));
    stats->rxError = regVal & counterMask;
    MTD_DBG_INFO("Sequence, code, symbol errors rxError: %llu\n", stats->rxError);

    regAddr += 4;
    MTD_ATTEMPT(mtdIndirectRegRead64(pDev, mdioPort, regAddr, &regVal));
    stats->crcErrTxUnderrun = regVal & counterMask;
    MTD_DBG_INFO("CRC error crcErrTxUnderrun: %llu\n", stats->crcErrTxUnderrun);

    regAddr += 4;
    MTD_ATTEMPT(mtdIndirectRegRead64(pDev, mdioPort, regAddr, &regVal));
    stats->rxOverrunBadFC = regVal & counterMask;
    MTD_DBG_INFO("Overrun or bad flow control packet rxOverrunBadFC: %llu\n", stats->rxOverrunBadFC);

    regAddr += 4;
    MTD_ATTEMPT(mtdIndirectRegRead64(pDev, mdioPort, regAddr, &regVal));
    stats->goodPkts = regVal & counterMask;
    MTD_DBG_INFO("Number of good packets goodPkts: %llu\n", stats->goodPkts);

    regAddr += 4;
    MTD_ATTEMPT(mtdIndirectRegRead64(pDev, mdioPort, regAddr, &regVal));
    stats->fcPkts = regVal & counterMask;
    MTD_DBG_INFO("Flow control packet fcPkts: %llu\n", stats->fcPkts);

    regAddr += 4;
    MTD_ATTEMPT(mtdIndirectRegRead64(pDev, mdioPort, regAddr, &regVal));
    stats->broadcast = regVal & counterMask;
    MTD_DBG_INFO("Number of broadcast packets broadcast: %llu\n", stats->broadcast);

    regAddr += 4;
    MTD_ATTEMPT(mtdIndirectRegRead64(pDev, mdioPort, regAddr, &regVal));
    stats->multicast = regVal & counterMask;
    MTD_DBG_INFO("Number of multicast packets multicast: %llu\n", stats->multicast);

    regAddr += 4;
    MTD_ATTEMPT(mtdIndirectRegRead64(pDev, mdioPort, regAddr, &regVal));
    stats->inPAssemblyErr = regVal & counterMask;
    MTD_DBG_INFO("inPAssemblyErr: %llu\n", stats->inPAssemblyErr);

    regAddr += 4;
    MTD_ATTEMPT(mtdIndirectRegRead64(pDev, mdioPort, regAddr, &regVal));
    stats->inPFrames = regVal & counterMask;
    MTD_DBG_INFO("inPFrames: %llu\n", stats->inPFrames);

    regAddr += 4;
    MTD_ATTEMPT(mtdIndirectRegRead64(pDev, mdioPort, regAddr, &regVal));
    stats->inPFrags = regVal & counterMask;
    MTD_DBG_INFO("inPFrags: %llu\n", stats->inPFrags);

    regAddr += 4;
    MTD_ATTEMPT(mtdIndirectRegRead64(pDev, mdioPort, regAddr, &regVal));
    stats->inPBadFrags = regVal & counterMask;
    MTD_DBG_INFO("inPBadFrags: %llu\n", stats->inPBadFrags);

    return MTD_OK;
}

MTD_STATUS mtdGetTxSMCStats
(
    IN MTD_DEV_PTR pDev,
    IN MTD_U16 mdioPort,
    IN MTD_TX_SMC_STATS *stats
)
{
    MTD_U64 counterMask = 0xFFFFFFFFFFF;
    MTD_U16 regAddr = MTD_SMAC_TX_STATS_REG;
    MTD_U64 regVal;

    MTD_DBG_INFO("mtdGetTxSMCStats:\n");

    MTD_ATTEMPT(mtdIndirectRegRead64(pDev, mdioPort, regAddr, &regVal));
    stats->goodOctets = regVal & 0xFFFFFFFF;
    MTD_DBG_INFO("Good frame octets goodOctets: %llu\n", stats->goodOctets);

    regAddr += 4;
    MTD_ATTEMPT(mtdIndirectRegRead64(pDev, mdioPort, regAddr, &regVal));
    stats->errorOctets = regVal & counterMask;
    MTD_DBG_INFO("Bad frame octets errorOctets: %llu\n", stats->errorOctets);

    regAddr += 4;
    MTD_ATTEMPT(mtdIndirectRegRead64(pDev, mdioPort, regAddr, &regVal));
    stats->jabber = regVal & counterMask;
    MTD_DBG_INFO("Frame of size  MTU and bad CRC jabber: %llu\n", stats->jabber);

    regAddr += 4;
    MTD_ATTEMPT(mtdIndirectRegRead64(pDev, mdioPort, regAddr, &regVal));
    stats->fragment = regVal & counterMask;
    MTD_DBG_INFO("Frame 64B with bad CRC fragment: %llu\n", stats->fragment);

    regAddr += 4;
    MTD_ATTEMPT(mtdIndirectRegRead64(pDev, mdioPort, regAddr, &regVal));
    stats->undersize = regVal & counterMask;
    MTD_DBG_INFO("Frame 64B with good CRC undersize: %llu\n", stats->undersize);

    regAddr += 4;
    MTD_ATTEMPT(mtdIndirectRegRead64(pDev, mdioPort, regAddr, &regVal));
    stats->oversize = regVal & counterMask;
    MTD_DBG_INFO("Frame of size MTU and good CRC oversize: %llu\n", stats->oversize);

    regAddr += 4;
    MTD_ATTEMPT(mtdIndirectRegRead64(pDev, mdioPort, regAddr, &regVal));
    stats->rxError = regVal & counterMask;
    MTD_DBG_INFO("Sequence, code, symbol errors rxError: %llu\n", stats->rxError);

    regAddr += 4;
    MTD_ATTEMPT(mtdIndirectRegRead64(pDev, mdioPort, regAddr, &regVal));
    stats->crcErrTxUnderrun = regVal & counterMask;
    MTD_DBG_INFO("CRC error crcErrTxUnderrun: %llu\n", stats->crcErrTxUnderrun);

    regAddr += 4;
    MTD_ATTEMPT(mtdIndirectRegRead64(pDev, mdioPort, regAddr, &regVal));
    stats->rxOverrunBadFC = regVal & counterMask;
    MTD_DBG_INFO("Overrun or bad flow control packet rxOverrunBadFC: %llu\n", stats->rxOverrunBadFC);

    regAddr += 4;
    MTD_ATTEMPT(mtdIndirectRegRead64(pDev, mdioPort, regAddr, &regVal));
    stats->goodPkts = regVal & counterMask;
    MTD_DBG_INFO("Number of good packets goodPkts: %llu\n", stats->goodPkts);

    regAddr += 4;
    MTD_ATTEMPT(mtdIndirectRegRead64(pDev, mdioPort, regAddr, &regVal));
    stats->fcPkts = regVal & counterMask;
    MTD_DBG_INFO("Flow control packet fcPkts: %llu\n", stats->fcPkts);

    regAddr += 4;
    MTD_ATTEMPT(mtdIndirectRegRead64(pDev, mdioPort, regAddr, &regVal));
    stats->broadcast = regVal & counterMask;
    MTD_DBG_INFO("Number of broadcast packets broadcast: %llu\n", stats->broadcast);

    regAddr += 4;
    MTD_ATTEMPT(mtdIndirectRegRead64(pDev, mdioPort, regAddr, &regVal));
    stats->multicast = regVal & counterMask;
    MTD_DBG_INFO("Number of multicast packets multicast: %llu\n", stats->multicast);

    regAddr += 4;
    MTD_ATTEMPT(mtdIndirectRegRead64(pDev, mdioPort, regAddr, &regVal));
    stats->outPFrags = regVal & counterMask;
    MTD_DBG_INFO("outPFrags: %llu\n", stats->outPFrags);

    regAddr += 4;
    MTD_ATTEMPT(mtdIndirectRegRead64(pDev, mdioPort, regAddr, &regVal));
    stats->outPFrames = regVal & counterMask;
    MTD_DBG_INFO("outPFrames: %llu\n", stats->outPFrames);

    return MTD_OK;
}

MTD_STATUS mtdGetRxWMCStats
(
    IN MTD_DEV_PTR pDev,
    IN MTD_U16 mdioPort,
    IN MTD_RX_WMC_STATS *stats
)
{
    MTD_U64 counterMask = 0xFFFFFFFFFFF;
    MTD_U16 regAddr = MTD_WMAC_RX_STATS_REG + 0x80;
    MTD_U64 regVal;

    MTD_DBG_INFO("mtdGetRxWMCStats:\n");

    MTD_ATTEMPT(mtdIndirectRegRead64(pDev, mdioPort, regAddr, &regVal));
    stats->pkts = regVal & counterMask;
    MTD_DBG_INFO("All packets count pkts: %llu\n", stats->pkts);

    regAddr += 4;
    MTD_ATTEMPT(mtdIndirectRegRead64(pDev, mdioPort, regAddr, &regVal));
    stats->droppedPkts = regVal & counterMask;
    MTD_DBG_INFO("Dropped packets droppedPkts: %llu\n", stats->droppedPkts);

    regAddr += 4;
    MTD_ATTEMPT(mtdIndirectRegRead64(pDev, mdioPort, regAddr, &regVal));
    stats->octets = regVal & counterMask;
    MTD_DBG_INFO("All packets octets octets: %llu\n", stats->octets);

    regAddr += 4;
    MTD_ATTEMPT(mtdIndirectRegRead64(pDev, mdioPort, regAddr, &regVal));
    stats->jabber = regVal & counterMask;
    MTD_DBG_INFO("Packet MTU and bad CRC jabber: %llu\n", stats->jabber);

    regAddr += 4;
    MTD_ATTEMPT(mtdIndirectRegRead64(pDev, mdioPort, regAddr, &regVal));
    stats->fragment = regVal & counterMask;
    MTD_DBG_INFO("Packet 64B and bad CRC fragment: %llu\n", stats->fragment);

    regAddr += 4;
    MTD_ATTEMPT(mtdIndirectRegRead64(pDev, mdioPort, regAddr, &regVal));
    stats->undersize = regVal & counterMask;
    MTD_DBG_INFO("Packet 64B and good CRC undersize: %llu\n", stats->undersize);

    regAddr += 4;
    MTD_ATTEMPT(mtdIndirectRegRead64(pDev, mdioPort, regAddr, &regVal));
    stats->rxError = regVal & counterMask;
    MTD_DBG_INFO("Sequence, code, symbol errors rxError: %llu\n", stats->rxError);

    regAddr += 4;
    MTD_ATTEMPT(mtdIndirectRegRead64(pDev, mdioPort, regAddr, &regVal));
    stats->crcError = regVal & counterMask;
    MTD_DBG_INFO("CRC error packet crcError: %llu\n", stats->crcError);

    regAddr += 4;
    MTD_ATTEMPT(mtdIndirectRegRead64(pDev, mdioPort, regAddr, &regVal));
    stats->fcPkts = regVal & counterMask;
    MTD_DBG_INFO("Flow control packet fcPkts: %llu\n", stats->fcPkts);

    regAddr += 4;
    MTD_ATTEMPT(mtdIndirectRegRead64(pDev, mdioPort, regAddr, &regVal));
    stats->broadcast = regVal & counterMask;
    MTD_DBG_INFO("Number of broadcast packets broadcast: %llu\n", stats->broadcast);

    regAddr += 4;
    MTD_ATTEMPT(mtdIndirectRegRead64(pDev, mdioPort, regAddr, &regVal));
    stats->multicast = regVal & counterMask;
    MTD_DBG_INFO("Number of multicast packets multicast: %llu\n", stats->multicast);

    regAddr = MTD_WMAC_RX_STATS_REG + 0xDC;
    MTD_ATTEMPT(mtdIndirectRegRead64(pDev, mdioPort, regAddr, &regVal));
    stats->inPAssemblyErr = regVal & counterMask;
    MTD_DBG_INFO("inPAssemblyErr: %llu\n", stats->inPAssemblyErr);

    regAddr += 4;
    MTD_ATTEMPT(mtdIndirectRegRead64(pDev, mdioPort, regAddr, &regVal));
    stats->inPFrames = regVal & counterMask;
    MTD_DBG_INFO("inPFrames: %llu\n", stats->inPFrames);

    regAddr += 4;
    MTD_ATTEMPT(mtdIndirectRegRead64(pDev, mdioPort, regAddr, &regVal));
    stats->inPFrags = regVal & counterMask;
    MTD_DBG_INFO("inPFrags: %llu\n", stats->inPFrags);

    regAddr += 4;
    MTD_ATTEMPT(mtdIndirectRegRead64(pDev, mdioPort, regAddr, &regVal));
    stats->inPBadFrags = regVal & counterMask;
    MTD_DBG_INFO("inPBadFrags: %llu\n", stats->inPBadFrags);

    return MTD_OK;

}


MTD_STATUS mtdGetTxWMCStats
(
    IN MTD_DEV_PTR pDev,
    IN MTD_U16 mdioPort,
    IN MTD_TX_WMC_STATS *stats
)
{
    MTD_U64 counterMask = 0xFFFFFFFFFFF;
    MTD_U16 regAddr = MTD_WMAC_TX_STATS_REG + 0x80;
    MTD_U64 regVal;

    MTD_DBG_INFO("mtdGetTxWMCStats:\n");

    MTD_ATTEMPT(mtdIndirectRegRead64(pDev, mdioPort, regAddr, &regVal));
    stats->pkts = regVal & counterMask;
    MTD_DBG_INFO("All packets count pkts: %llu\n", stats->pkts);

    regAddr += 4;
    MTD_ATTEMPT(mtdIndirectRegRead64(pDev, mdioPort, regAddr, &regVal));
    stats->droppedPkts = regVal & counterMask;
    MTD_DBG_INFO("Dropped packets droppedPkts: %llu\n", stats->droppedPkts);

    regAddr += 4;
    MTD_ATTEMPT(mtdIndirectRegRead64(pDev, mdioPort, regAddr, &regVal));
    stats->octets = regVal & counterMask;
    MTD_DBG_INFO("All packets octets octets: %llu\n", stats->octets);

    regAddr += 4;
    MTD_ATTEMPT(mtdIndirectRegRead64(pDev, mdioPort, regAddr, &regVal));
    stats->jabber = regVal & counterMask;
    MTD_DBG_INFO("Packet MTU and bad CRC jabber: %llu\n", stats->jabber);

    regAddr += 4;
    MTD_ATTEMPT(mtdIndirectRegRead64(pDev, mdioPort, regAddr, &regVal));
    stats->fragment = regVal & counterMask;
    MTD_DBG_INFO("Packet 64B and bad CRC fragment: %llu\n", stats->fragment);

    regAddr += 4;
    MTD_ATTEMPT(mtdIndirectRegRead64(pDev, mdioPort, regAddr, &regVal));
    stats->undersize = regVal & counterMask;
    MTD_DBG_INFO("Packet 64B and good CRC undersize: %llu\n", stats->undersize);

    regAddr += 4;
    MTD_ATTEMPT(mtdIndirectRegRead64(pDev, mdioPort, regAddr, &regVal));
    stats->rxError = regVal & counterMask;
    MTD_DBG_INFO("Sequence, code, symbol errors rxError: %llu\n", stats->rxError);

    regAddr += 4;
    MTD_ATTEMPT(mtdIndirectRegRead64(pDev, mdioPort, regAddr, &regVal));
    stats->crcError = regVal & counterMask;
    MTD_DBG_INFO("CRC error packet crcError: %llu\n", stats->crcError);

    regAddr += 4;
    MTD_ATTEMPT(mtdIndirectRegRead64(pDev, mdioPort, regAddr, &regVal));
    stats->fcPkts = regVal & counterMask;
    MTD_DBG_INFO("Flow control packet fcPkts: %llu\n", stats->fcPkts);

    regAddr += 4;
    MTD_ATTEMPT(mtdIndirectRegRead64(pDev, mdioPort, regAddr, &regVal));
    stats->broadcast = regVal & counterMask;
    MTD_DBG_INFO("Number of broadcast packets broadcast: %llu\n", stats->broadcast);

    regAddr += 4;
    MTD_ATTEMPT(mtdIndirectRegRead64(pDev, mdioPort, regAddr, &regVal));
    stats->multicast = regVal & counterMask;
    MTD_DBG_INFO("Number of multicast packets multicast: %llu\n", stats->multicast);

    regAddr = MTD_WMAC_TX_STATS_REG + 0xDC;
    MTD_ATTEMPT(mtdIndirectRegRead64(pDev, mdioPort, regAddr, &regVal));
    stats->outPFrags = regVal & counterMask;
    MTD_DBG_INFO("outPFrags: %llu\n", stats->outPFrags);

    regAddr += 4;
    MTD_ATTEMPT(mtdIndirectRegRead64(pDev, mdioPort, regAddr, &regVal));
    stats->outPFrames = regVal & counterMask;
    MTD_DBG_INFO("outPFrames: %llu\n", stats->outPFrames);

    return MTD_OK;
}

MTD_STATUS mtdClearMIBStats
(
    IN MTD_DEV_PTR pDev,
    IN MTD_U16  mdioPort
)
{
    MTD_U32 regVal;
    MTD_RX_SMC_STATS rxSMCstats;
    MTD_TX_SMC_STATS txSMCstats;
    MTD_RX_WMC_STATS rxWMCstats;
    MTD_TX_WMC_STATS txWMCstats;

    /* set clear-on-read MIBS counters */
    MTD_ATTEMPT(mtdIndirectRegRead32(pDev, mdioPort, MTD_SLC_CFG_GEN, &regVal));
    MTD_ATTEMPT(mtdRegWrite32(pDev, mdioPort, MTD_MACSEC_UNIT, MTD_SLC_CFG_GEN, regVal | 0x3C000000));

    /* read to clear MIBS counters */
    MTD_ATTEMPT(mtdGetRxSMCStats(pDev, mdioPort, &rxSMCstats));
    MTD_ATTEMPT(mtdGetTxSMCStats(pDev, mdioPort, &txSMCstats));
    MTD_ATTEMPT(mtdGetRxWMCStats(pDev, mdioPort, &rxWMCstats));
    MTD_ATTEMPT(mtdGetTxWMCStats(pDev, mdioPort, &txWMCstats));

    /* set back MIBS counters */
    MTD_ATTEMPT(mtdRegWrite32(pDev, mdioPort, MTD_MACSEC_UNIT, MTD_SLC_CFG_GEN, regVal));

    return MTD_OK;
}

MTD_STATUS mtdClearOnReadMIBStats
(
    IN MTD_DEV_PTR pDev,
    IN MTD_U16  mdioPort,
    IN MTD_BOOL clearOnReadFlag
)
{
    MTD_U32 regVal;

    MTD_ATTEMPT(mtdRSMACSecRegRead32(pDev, MTD_SLC_CFG_GEN, &regVal));
    if (clearOnReadFlag)
    {
        /* set clear-on-read MIBS counters */
        MTD_ATTEMPT(mtdRegWrite32(pDev, mdioPort, MTD_MACSEC_UNIT, MTD_SLC_CFG_GEN, regVal | 0x3C000000));
    }
    else
    {
        /* clear clear-on-read MIBS counters */
        MTD_ATTEMPT(mtdRegWrite32(pDev, mdioPort, MTD_MACSEC_UNIT, MTD_SLC_CFG_GEN, regVal & ~(0x3C000000)));
    }

    return MTD_OK;
}
/************************* End of MIB Statistics API ********************************/

MTD_STATUS mtdRSMACSecClearStatisticCounters
(
    IN MTD_DEV_PTR pDev,
    IN MTD_U16 mdioPort,
    IN MTD_BOOL clearOnReadFlag
)
{
    RmsDev_t *mcsDev_p;

    mcsDev_p = &pDev->rsMACSecCntl.rmsDev;

    Ra01RsMcsCseRxCtrl_t cseRxCtrl;
    Ra01RsMcsCseTxCtrl_t cseTxCtrl;
    Ra01RsMcsCseRxStatsClear_t cseRxStatsClear;
    Ra01RsMcsCseTxStatsClear_t cseTxStatsClear;

    MTD_DBG_INFO("mtdRSMACSecClearStatisticCounters.\n");

    cseRxCtrl.cpuCseClrOnRd = clearOnReadFlag;
    Ra01AccRsMcsCseRxCtrl(mcsDev_p, &cseRxCtrl, RA01_READ_OP);
    cseTxCtrl.cpuCseClrOnRd = clearOnReadFlag;
    Ra01AccRsMcsCseTxCtrl(mcsDev_p, &cseTxCtrl, RA01_READ_OP);

    cseRxStatsClear.rxFlowidGo = 1;
    cseRxStatsClear.rxPortGo = 1;
    cseRxStatsClear.rxSaGo = 1;
    cseRxStatsClear.rxScGo = 1;
    cseRxStatsClear.rxSecyGo = 1;
    Ra01AccRsMcsCseRxStatsClear(mcsDev_p, &cseRxStatsClear, RA01_WRITE_OP);

    cseTxStatsClear.txFlowidGo = 1;
    cseTxStatsClear.txPortGo = 1;
    cseTxStatsClear.txSaGo = 1;
    cseTxStatsClear.txScGo = 1;
    cseTxStatsClear.txSecyGo = 1;
    Ra01AccRsMcsCseTxStatsClear(mcsDev_p, &cseTxStatsClear, RA01_WRITE_OP);

    return MTD_OK;
}

MTD_STATUS mtdRSMACRxSecYStatisticCounters
(
    IN MTD_DEV_PTR pDev,
    IN MTD_U16 mdioPort,
    IN MTD_U16 index,
    IN MacsecRxSecyCounters_t *rxSecYCounters
)
{
    RmsDev_t *mcsDev_p;

    mcsDev_p = &pDev->rsMACSecCntl.rmsDev;

    MTD_DBG_INFO("mtdRSMACRxSecYStatisticCounters:\n");

    MTD_ATTEMPT(MacsecGetRxSecyStatisticCounters(mcsDev_p, index, rxSecYCounters));

    MTD_DBG_INFO("Ifinunctloctets: %llu.\n", rxSecYCounters->Ifinunctloctets);
    MTD_DBG_INFO("Ifinctloctets: %llu.\n", rxSecYCounters->Ifinctloctets);
    MTD_DBG_INFO("Ifinunctlucpkts: %llu.\n", rxSecYCounters->Ifinunctlucpkts);
    MTD_DBG_INFO("Ifinunctlmcpkts: %llu.\n", rxSecYCounters->Ifinunctlmcpkts);
    MTD_DBG_INFO("Ifinunctlbcpkts: %llu.\n", rxSecYCounters->Ifinunctlbcpkts);
    MTD_DBG_INFO("Ifinctlucpkts: %llu.\n", rxSecYCounters->Ifinctlucpkts);
    MTD_DBG_INFO("Ifinctlmcpkts: %llu.\n", rxSecYCounters->Ifinctlmcpkts);
    MTD_DBG_INFO("Ifinctlbcpkts: %llu.\n", rxSecYCounters->Ifinctlbcpkts);
    MTD_DBG_INFO("Inpktssecyuntaggedornotag: %llu.\n", rxSecYCounters->Inpktssecyuntaggedornotag);
    MTD_DBG_INFO("Inpktssecybadtag: %llu.\n", rxSecYCounters->Inpktssecybadtag);
    MTD_DBG_INFO("Inpktssecyctl: %llu.\n", rxSecYCounters->Inpktssecyctl);
    MTD_DBG_INFO("Inpktssecytaggedctl: %llu.\n", rxSecYCounters->Inpktssecytaggedctl);
    MTD_DBG_INFO("Inpktssecyunknownsci: %llu.\n", rxSecYCounters->Inpktssecyunknownsci);
    MTD_DBG_INFO("Inpktssecynosci: %llu.\n", rxSecYCounters->Inpktssecynosci);
    MTD_DBG_INFO("Inpktsctrlportdisabled: %llu.\n", rxSecYCounters->Inpktsctrlportdisabled);

    return MTD_OK;
}


MTD_STATUS mtdRSMACRxSCStatisticCounters
(
    IN MTD_DEV_PTR pDev,
    IN MTD_U16 mdioPort,
    IN MTD_U16 index,
    IN MacsecRxScCounters_t *rxSCCounters
)
{
    RmsDev_t *mcsDev_p;

    mcsDev_p = &pDev->rsMACSecCntl.rmsDev;

    MTD_DBG_INFO("mtdRSMACRxSCStatisticCounters:\n");

    MTD_ATTEMPT(MacsecGetRxSCStatisticCounters(mcsDev_p, index, rxSCCounters));

    MTD_DBG_INFO("Inoctetsscvalidate: %llu.\n", rxSCCounters->Inoctetsscvalidate);
    MTD_DBG_INFO("Inoctetsscdecrypted: %llu.\n", rxSCCounters->Inoctetsscdecrypted);
    MTD_DBG_INFO("Inpktsscunchecked: %llu.\n", rxSCCounters->Inpktsscunchecked);
    MTD_DBG_INFO("Inpktssclateordelayed: %llu.\n", rxSCCounters->Inpktssclateordelayed);
    MTD_DBG_INFO("Inpktssccamhit: %llu.\n", rxSCCounters->Inpktssccamhit);

    return MTD_OK;
}

MTD_STATUS mtdRSMACRxSAStatisticCounters
(
    IN MTD_DEV_PTR pDev,
    IN MTD_U16 mdioPort,
    IN MTD_U16 index,
    IN MacsecRxSaCounters_t *rxSACounters
)
{
    RmsDev_t *mcsDev_p;

    mcsDev_p = &pDev->rsMACSecCntl.rmsDev;

    MTD_DBG_INFO("mtdRSMACRxSAStatisticCounters:\n");

    MTD_ATTEMPT(MacsecGetRxSAStatisticCounters(mcsDev_p, index, rxSACounters));

    MTD_DBG_INFO("Inpktssaok: %llu.\n", rxSACounters->Inpktssaok);
    MTD_DBG_INFO("Inpktssainvalid: %llu.\n", rxSACounters->Inpktssainvalid);
    MTD_DBG_INFO("Inpktssanotvalid: %llu.\n", rxSACounters->Inpktssanotvalid);
    MTD_DBG_INFO("Inpktssaunusedsa: %llu.\n", rxSACounters->Inpktssaunusedsa);
    MTD_DBG_INFO("Inpktssanotusingsaerror: %llu.\n", rxSACounters->Inpktssanotusingsaerror);

    return MTD_OK;
}

MTD_STATUS mtdRSMACRxPortStatisticCounters
(
    IN MTD_DEV_PTR pDev,
    IN MTD_U16 mdioPort,
    IN MTD_U16 index,
    IN MacsecRxPortCounters_t *rxPortCounters
)
{
    RmsDev_t *mcsDev_p;

    mcsDev_p = &pDev->rsMACSecCntl.rmsDev;

    MTD_DBG_INFO("mtdRSMACRxPortStatisticCounters:\n");

    MTD_ATTEMPT(MacsecGetRxPortStatisticCounters(mcsDev_p, index, rxPortCounters));

    MTD_DBG_INFO("Inpktsflowidtcammiss: %llu.\n", rxPortCounters->Inpktsflowidtcammiss);
    MTD_DBG_INFO("Inpktsparseerr: %llu.\n", rxPortCounters->Inpktsparseerr);
    MTD_DBG_INFO("Inpktsearlypreempterr: %llu.\n", rxPortCounters->Inpktsearlypreempterr);

    return MTD_OK;
}

MTD_STATUS mtdRSMACRxFlowStatisticCounters
(
    IN MTD_DEV_PTR pDev,
    IN MTD_U16 mdioPort,
    IN MTD_U16 index,
    IN MacsecRxFlowCounters_t *rxFlowCounters
)
{
    RmsDev_t *mcsDev_p;

    mcsDev_p = &pDev->rsMACSecCntl.rmsDev;

    MTD_DBG_INFO("mtdRSMACRxFlowStatisticCounters:\n");

    MTD_ATTEMPT(MacsecGetRxFlowStatisticCounters(mcsDev_p, index, rxFlowCounters));

    MTD_DBG_INFO("Inpktsflowidtcamhit: %llu.\n", rxFlowCounters->Inpktsflowidtcamhit);

    return MTD_OK;
}


MTD_STATUS mtdRSMACTxSecYStatisticCounters
(
    IN MTD_DEV_PTR pDev,
    IN MTD_U16 mdioPort,
    IN MTD_U16 index,
    IN MacsecTxSecyCounters_t *txSecYCounters
)
{
    RmsDev_t *mcsDev_p;

    mcsDev_p = &pDev->rsMACSecCntl.rmsDev;

    MTD_DBG_INFO("mtdRSMACTxSecYStatisticCounters:\n");

    MTD_ATTEMPT(MacsecGetTxSecyStatisticCounters(mcsDev_p, index, txSecYCounters));

    MTD_DBG_INFO("Ifoutcommonoctets: %llu.\n", txSecYCounters->Ifoutcommonoctets);
    MTD_DBG_INFO("Ifoutunctloctets: %llu.\n", txSecYCounters->Ifoutunctloctets);
    MTD_DBG_INFO("Ifoutctloctets: %llu.\n", txSecYCounters->Ifoutctloctets);
    MTD_DBG_INFO("Ifoutunctlucpkts: %llu.\n", txSecYCounters->Ifoutunctlucpkts);
    MTD_DBG_INFO("Ifoutunctlmcpkts: %llu.\n", txSecYCounters->Ifoutunctlmcpkts);
    MTD_DBG_INFO("Ifoutunctlbcpkts: %llu.\n", txSecYCounters->Ifoutunctlbcpkts);
    MTD_DBG_INFO("Ifoutctlucpkts: %llu.\n", txSecYCounters->Ifoutctlucpkts);
    MTD_DBG_INFO("Ifoutctlmcpkts: %llu.\n", txSecYCounters->Ifoutctlmcpkts);
    MTD_DBG_INFO("Ifoutctlbcpkts: %llu.\n", txSecYCounters->Ifoutctlbcpkts);
    MTD_DBG_INFO("Outpktssecyuntagged: %llu.\n", txSecYCounters->Outpktssecyuntagged);
    MTD_DBG_INFO("Outpktssecytoolong: %llu.\n", txSecYCounters->Outpktssecytoolong);
    MTD_DBG_INFO("Outpktssecynoactivesa: %llu.\n", txSecYCounters->Outpktssecynoactivesa);
    MTD_DBG_INFO("Outpktsctrlportdisabled: %llu.\n", txSecYCounters->Outpktsctrlportdisabled);

    return MTD_OK;
}

MTD_STATUS mtdRSMACTxSCStatisticCounters
(
    IN MTD_DEV_PTR pDev,
    IN MTD_U16 mdioPort,
    IN MTD_U16 index,
    IN MacsecTxScCounters_t *txSCCounters
)
{
    RmsDev_t *mcsDev_p;

    mcsDev_p = &pDev->rsMACSecCntl.rmsDev;

    MTD_DBG_INFO("mtdRSMACTxSCStatisticCounters:\n");

    MTD_ATTEMPT(MacsecGetTxSCStatisticCounters(mcsDev_p, index, txSCCounters));

    MTD_DBG_INFO("Outoctetsscprotected: %llu.\n", txSCCounters->Outoctetsscprotected);
    MTD_DBG_INFO("Outoctetsscencrypted: %llu.\n", txSCCounters->Outoctetsscencrypted);

    return MTD_OK;
}

MTD_STATUS mtdRSMACTxSAStatisticCounters
(
    IN MTD_DEV_PTR pDev,
    IN MTD_U16 mdioPort,
    IN MTD_U16 index,
    IN MacsecTxSaCounters_t *txSaCounters
)
{
    RmsDev_t *mcsDev_p;

    mcsDev_p = &pDev->rsMACSecCntl.rmsDev;

    MTD_DBG_INFO("mtdRSMACTxSAStatisticCounters:\n");

    MTD_ATTEMPT(MacsecGetTxSAStatisticCounters(mcsDev_p, index, txSaCounters));

    MTD_DBG_INFO("Outpktssaprotected: %llu.\n", txSaCounters->Outpktssaprotected);
    MTD_DBG_INFO("Outpktssaencrypted: %llu.\n", txSaCounters->Outpktssaencrypted);

    return MTD_OK;
}

MTD_STATUS mtdRSMACTxPortStatisticCounters
(
    IN MTD_DEV_PTR pDev,
    IN MTD_U16 mdioPort,
    IN MTD_U16 index,
    IN MacsecTxPortCounters_t *txPortCounters
)
{
    RmsDev_t *mcsDev_p;

    mcsDev_p = &pDev->rsMACSecCntl.rmsDev;

    MTD_DBG_INFO("mtdRSMACTxPortStatisticCounters:\n");

    MTD_ATTEMPT(MacsecGetTxPortStatisticCounters(mcsDev_p, index, txPortCounters));

    MTD_DBG_INFO("Outpktsflowidtcammiss: %llu.\n", txPortCounters->Outpktsflowidtcammiss);
    MTD_DBG_INFO("Outpktsparseerr: %llu.\n", txPortCounters->Outpktsparseerr);
    MTD_DBG_INFO("Outpktssectaginsertionerr: %llu.\n", txPortCounters->Outpktssectaginsertionerr);
    MTD_DBG_INFO("Outpktsearlypreempterr: %llu.\n", txPortCounters->Outpktsearlypreempterr);

    return MTD_OK;
}

MTD_STATUS mtdRSMACTxFlowStatisticCounters
(
    IN MTD_DEV_PTR pDev,
    IN MTD_U16 mdioPort,
    IN MTD_U16 index,
    IN MacsecTxFlowCounters_t *txFlowCounters
)
{
    RmsDev_t *mcsDev_p;

    mcsDev_p = &pDev->rsMACSecCntl.rmsDev;

    MTD_DBG_INFO("mtdRSMACTxFlowStatisticCounters:\n");

    MTD_ATTEMPT(MacsecGetTxFlowStatisticCounters(mcsDev_p, index, txFlowCounters));

    MTD_DBG_INFO("Outpktsflowidtcamhit: %llu.\n", txFlowCounters->Outpktsflowidtcamhit);

    return MTD_OK;
}

MTD_STATUS mtdRSMACInterruptEnable
(
    IN MTD_DEV_PTR pDev,
    IN MTD_U16 mdioPort,
    IN MTD_BOOL interruptEnable 
)
{
    RmsDev_t* mcsDev_p;
    MTD_U32 regVal32;

    mcsDev_p = &pDev->rsMACSecCntl.rmsDev;

    if (interruptEnable)
    {
        MTD_ATTEMPT(mtdRegRead32(pDev, mdioPort, MTD_MACSEC_UNIT, MTD_SLC_CFG_GEN, &regVal32));
        regVal32 |= 0x200000;
        MTD_ATTEMPT(mtdRegWrite32(pDev, mdioPort, MTD_MACSEC_UNIT, MTD_SLC_CFG_GEN, regVal32));
    
        /* enable PHY device MACSec block MTD_PTP_MPM_INTR interrupt; this will propagate the MACSec interrupt to
           the top level */
        MTD_ATTEMPT(mtdEnableDisableTopInterrupts(pDev, mdioPort, MTD_PTP_MPM_INTR, MTD_TRUE));

        /* to enable selected MACSec IP block interrupts, call MacsecEnableBlockInterrupts() with
           the select block in BlockInterruptEnables_t structure */
           /* MacsecEnableInterrupts() to enable all MACSec IP block interrupts */
        MTD_ATTEMPT(MacsecEnableInterrupts(mcsDev_p));
    }
    else
    {
        MTD_ATTEMPT(mtdRegRead32(pDev, mdioPort, MTD_MACSEC_UNIT, MTD_SLC_CFG_GEN, &regVal32));
        regVal32 &= ~(0x200000);
        MTD_ATTEMPT(mtdRegWrite32(pDev, mdioPort, MTD_MACSEC_UNIT, MTD_SLC_CFG_GEN, regVal32));

        /* enable PHY device MACSec block MTD_PTP_MPM_INTR interrupt; this will propagate the MACSec interrupt to
           the top level */
        MTD_ATTEMPT(mtdEnableDisableTopInterrupts(pDev, mdioPort, MTD_PTP_MPM_INTR, MTD_FALSE));
    }

    return MTD_OK;
}



