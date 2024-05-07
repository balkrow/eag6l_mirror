/*! 
 * @file macsec_api.c 
 */

// *******************************************************************************
// *
// * MARVELL CONFIDENTIAL
// * ___________________
// *
// * Copyright (C) 2022 Marvell.
// * All Rights Reserved.
// *
// * NOTICE: All information contained herein is, and remains the property of
// * Marvell Asia Pte Ltd and its suppliers, if any. The intellectual and
// * technical concepts contained herein are proprietary to Marvell Asia Pte Ltd
// * and its suppliers and may be covered by U.S. and Foreign Patents, patents
// * in process, and are protected by trade secret or copyright law.
// * Dissemination of this information or reproduction of this material is strictly
// * forbidden unless prior written permission is obtained from Marvell Asia Pte Ltd.
// *
// ******************************************************************************
// * $Rev: 
// * $Date: 
// ******************************************************************************

#include <errno.h>
#include <string.h>
#include "i_macsec.h"

#include "macsec_api.h"


/*!
 * @brief Set the debug verbosity
 */
#define SIM_PRINT_LEVEL SIM_DEBUG
#include "sim_log.h"

//------------------------------------------------------------------------------
// Register read/write functions
//------------------------------------------------------------------------------
static int autoRekey( RmsDev_t * rmsDev_p, unsigned secyIndex, bool enable )
{
    int                       ret = 0;
    Ra01IfRsMcsCpmTxSaMap_t rekey;
    unsigned                  reg_index = secyIndex / (sizeof(ra01_register_t) * 8);

    ret = Ra01WriteRsMcsCpmTxSaMap(rmsDev_p, reg_index, &rekey );
    DBGRETURN(ret);

    rekey.autoRekeyEnable = enable;

    ret = Ra01WriteRsMcsCpmTxSaMap(rmsDev_p, reg_index, &rekey );

    return ret;
}

int setRxSecyMap( RmsDev_t * rmsDev_p, int tableIndex, uint32_t secy, bool controlPacket )
{
    int                 ret = 0;
    Ra01IfRsMcsCpmRxSecyMap_t map;

    map.ctrlPkt = controlPacket;
#if RA01_RS_MCS_CPM_RX_SECY_PLCY_MEM_SIZE <= 256
    map.secy	= (uint8_t)secy;
#else
    map.secy	= (uint16_t)secy;
#endif

    ret = Ra01WriteRsMcsCpmRxSecyMap( rmsDev_p, tableIndex, &map ) ;

    return ret;
}

int getRxSecyMap( RmsDev_t * rmsDev_p, int tableIndex, uint32_t *secy, bool *controlPacket )
{
    int                 ret = 0;
    Ra01IfRsMcsCpmRxSecyMap_t map;

    ret = Ra01ReadRsMcsCpmRxSecyMap( rmsDev_p, tableIndex, &map ) ;

    (*controlPacket) = map.ctrlPkt;
    (*secy)          = (uint32_t)map.secy;

    return ret;
}

int setTxSecyMap( RmsDev_t * rmsDev_p,
		  int        tableIndex,
		  uint32_t   secy,
		  uint32_t   sc,
		  bool       controlPacket,
		  bool       auxiliaryPolicy )
{
    int                 ret = 0;
    Ra01IfRsMcsCpmTxSecyMap_t map;

#if RA01_RS_MCS_CPM_TX_SECY_PLCY_MEM_SIZE <= 256
    map.secy	      = (uint8_t)secy;
#else
    map.secy	      = (uint16_t)secy;
#endif
    map.ctrlPkt	      = controlPacket;
#if RA01_RS_MCS_CPM_TX_SA_MAP_MEM_SIZE <= 256
    map.sc	      = (uint8_t)sc;
#else
    map.sc	      = (uint16_t)sc;
#endif
    map.auxiliaryPlcy = auxiliaryPolicy;

    ret = Ra01WriteRsMcsCpmTxSecyMap( rmsDev_p, tableIndex, &map );

    return ret;
}

int getTxSecyMap( RmsDev_t * rmsDev_p,
		  int        tableIndex,
		  uint32_t * secy,
		  uint32_t * sc,
		  bool *     controlPacket,
		  bool *     auxiliaryPolicy )
{
    int				ret = 0;
    Ra01IfRsMcsCpmTxSecyMap_t	map;

    ret = Ra01ReadRsMcsCpmTxSecyMap( rmsDev_p, tableIndex, &map );

    (*secy)             = (uint32_t)map.secy;
    (*controlPacket)    = map.ctrlPkt;
    (*sc)               = (uint32_t)map.sc;
    (*auxiliaryPolicy)  = map.auxiliaryPlcy;

    return ret;
}

static int updateSaKeyLockout( RmsDev_t * rmsDev_p, MacsecDirection_t dir, unsigned saIndex, bool lock )
{
    int      ret = 0;
    unsigned reg_index = saIndex / (8 * sizeof(ra01_register_t));
    unsigned bit_index = saIndex % (8 * sizeof(ra01_register_t));

    if (dir == MACSEC_INGRESS) {
        Ra01RsMcsCpmRxSaKeyLockout_t value;
        ret = Ra01AccRsMcsCpmRxSaKeyLockout( rmsDev_p, reg_index, &value, RA01_READ_OP);
        if( ret < 0 ) {
            return ret;
        }

        if( lock ) {
            value.sa |= ((uint64_t)1 << bit_index);
        } else {
            value.sa &= ~((uint64_t)1 << bit_index);
        }
        ret = Ra01AccRsMcsCpmRxSaKeyLockout( rmsDev_p, reg_index, &value, RA01_WRITE_OP);
        if( ret < 0 ) {
            return ret;
        }
    } else {
        Ra01RsMcsCpmTxSaKeyLockout_t value;
        ret = Ra01AccRsMcsCpmTxSaKeyLockout( rmsDev_p, reg_index, &value, RA01_READ_OP);
        if( ret < 0 ) {
            return ret;
        }

        if( lock ) {
            value.sa |= ((uint64_t)1 << bit_index);
        } else {
            value.sa &= ~((uint64_t)1 << bit_index);
        }
        ret = Ra01AccRsMcsCpmTxSaKeyLockout( rmsDev_p, reg_index, &value, RA01_WRITE_OP);
        if( ret < 0 ) {
            return ret;
        }
    }

    return ret;
}

//------------------------------------------------------------------------------
// Structure processing functions
//------------------------------------------------------------------------------
static int setIngressParameters(RmsDev_t * rmsDev_p, MacsecConfig_t * cfg )
{
    int                     ret = 0;
    int                     i;
    MacsecRxParam_t * ingress = &(cfg->ingress);

    if( ingress->opt.setOptParameters ) {
        ret = Ra01AccRsMcsBbePaddingCtl( rmsDev_p, RA01_RS_MCS_BBE_RX_SLAVE, &(ingress->opt.padding) , RA01_WRITE_OP);
        DBGRETURN(ret);

        ret = Ra01AccRsMcsPabIncludeTagCnt( rmsDev_p, RA01_RS_MCS_PAB_RX_SLAVE, &(ingress->opt.includeTagCount) , RA01_WRITE_OP);
        DBGRETURN(ret);

        ret = Ra01AccRsMcsCpmRxRxDefaultSci( rmsDev_p, &(ingress->opt.rxDefaultSci) , RA01_WRITE_OP);
        DBGRETURN(ret);

        ret = Ra01AccRsMcsCpmRxRxEtype( rmsDev_p, &(ingress->opt.rxSectagEtype), RA01_WRITE_OP);
        DBGRETURN(ret);

        ret = Ra01AccRsMcsCpmRxRxMcsBasePort( rmsDev_p, &(ingress->opt.rxMcsBasePort) , RA01_WRITE_OP);
        DBGRETURN(ret);

        ret = Ra01AccRsMcsCpmRxCpuPuntEnable( rmsDev_p, &(ingress->opt.cpuPuntEnable) , RA01_WRITE_OP);
        DBGRETURN(ret);
    }

    Ra01RsMcsCpmRxSectagRuleChkEnable_t sectagRuleEnb;
    sectagRuleEnb.sectagVEq1Ena        = ingress->sectagRuleChkEnable.sectagVEq1Ena;
    sectagRuleEnb.sectagEsEq1ScEq1Ena  = ingress->sectagRuleChkEnable.sectagEsEq1ScEq1Ena;
    sectagRuleEnb.sectagScEq1ScbEq1Ena = ingress->sectagRuleChkEnable.sectagScEq1ScbEq1Ena;
    sectagRuleEnb.sectagPnEq0Ena       = ingress->sectagRuleChkEnable.sectagPnEq0Ena;
    ret = Ra01AccRsMcsCpmRxSectagRuleChkEnable(rmsDev_p, &sectagRuleEnb, RA01_WRITE_OP);
    DBGRETURN(ret);

    for( i = 0; i < MACSEC_PORT_COUNT; i++ ) {
        if( ingress->rxmcsInsertion[i].valid ) {
            ret = Ra01AccRsMcsCpmRxEnableRxmcsInsertion( rmsDev_p, i, &(ingress->rxmcsInsertion[i].insertion) , RA01_WRITE_OP);
        }
        DBGRETURN(ret);
    }
    for( i = 0; i < MACSEC_CHANNELS_PER_PORT; i++ ) {
        Ra01RsMcsTopChannelConfig_t chCfg;
        chCfg.chBypass = ingress->chBypass[i];
        ret = Ra01AccRsMcsTopChannelConfig( rmsDev_p, i, &chCfg , RA01_WRITE_OP);
        DBGRETURN(ret);
    }

    Ra01RsMcsCpmRxPnThreshold_t pn;
    pn.pnThreshold = ingress->pnThreshold;
    ret = Ra01AccRsMcsCpmRxPnThreshold( rmsDev_p, &pn , RA01_WRITE_OP);
    DBGRETURN(ret);
    Ra01RsMcsCpmRxXpnThreshold_t xpn;
    xpn.xpnThresholdMsb = (ingress->xpnThreshold >> 32) & 0xffffffff;
    xpn.xpnThresholdLsb = ingress->xpnThreshold & 0xffffffff;
    ret = Ra01AccRsMcsCpmRxXpnThreshold( rmsDev_p, &xpn , RA01_WRITE_OP);
    DBGRETURN(ret);

    Ra01RsMcsCpmRxSaPnTableUpdate_t pnUp;
    pnUp.maxUpdate = ingress->pnTableUpdate;
    ret = Ra01AccRsMcsCpmRxSaPnTableUpdate( rmsDev_p, &pnUp , RA01_WRITE_OP);
    DBGRETURN(ret);

    Ra01RsMcsCseRxCtrl_t cseRxCtrl;
    cseRxCtrl.cpuCseClrOnRd = ingress->statsClearOnRead;
    ret = Ra01AccRsMcsCseRxCtrl(rmsDev_p, &cseRxCtrl, RA01_WRITE_OP);
    DBGRETURN(ret);

    Ra01RsMcsPexEarlyPreemptFilterCtrl_t earlyPreemptFilterCtrl;
    earlyPreemptFilterCtrl.earlyPreemptFilterEnable = ingress->earlyPreemptFilterEnable;
    earlyPreemptFilterCtrl.earlyPreemptMinNumWords = 9;
    ret = Ra01AccRsMcsPexEarlyPreemptFilterCtrl(rmsDev_p, RA01_RS_MCS_PEX_RX_SLAVE, 0, &earlyPreemptFilterCtrl, RA01_WRITE_OP);
    DBGRETURN(ret);

    return ret;
}

static int getIngressParameters( RmsDev_t * rmsDev_p, MacsecRxParam_t * ingress )
{
    int                ret = 0;
    int                i;

    ret = Ra01AccRsMcsBbePaddingCtl( rmsDev_p, RA01_RS_MCS_BBE_RX_SLAVE, &(ingress->opt.padding), RA01_READ_OP);
    DBGRETURN(ret);

    ret = Ra01AccRsMcsCpmRxRxDefaultSci( rmsDev_p, &(ingress->opt.rxDefaultSci) , RA01_READ_OP);
    DBGRETURN(ret);

    ret = Ra01AccRsMcsCpmRxRxEtype( rmsDev_p, &(ingress->opt.rxSectagEtype), RA01_READ_OP);
    DBGRETURN(ret);

    ret = Ra01AccRsMcsCpmRxRxMcsBasePort( rmsDev_p, &(ingress->opt.rxMcsBasePort) , RA01_READ_OP);
    DBGRETURN(ret);

    ret = Ra01AccRsMcsCpmRxCpuPuntEnable( rmsDev_p, &(ingress->opt.cpuPuntEnable) , RA01_READ_OP);
    DBGRETURN(ret);

    ingress->opt.setOptParameters = true;

    Ra01RsMcsCpmRxSaPnTableUpdate_t pnUp;
    ret = Ra01AccRsMcsCpmRxSaPnTableUpdate( rmsDev_p, &pnUp , RA01_READ_OP);
    DBGRETURN(ret);
    ingress->pnTableUpdate = pnUp.maxUpdate;

    Ra01RsMcsCpmRxSectagRuleChkEnable_t sectagRuleEnb;
    ret = Ra01AccRsMcsCpmRxSectagRuleChkEnable(rmsDev_p, &sectagRuleEnb, RA01_READ_OP);
    ingress->sectagRuleChkEnable.sectagVEq1Ena        = sectagRuleEnb.sectagVEq1Ena;
    ingress->sectagRuleChkEnable.sectagEsEq1ScEq1Ena  = sectagRuleEnb.sectagEsEq1ScEq1Ena;
    ingress->sectagRuleChkEnable.sectagScEq1ScbEq1Ena = sectagRuleEnb.sectagScEq1ScbEq1Ena;
    ingress->sectagRuleChkEnable.sectagPnEq0Ena       = sectagRuleEnb.sectagPnEq0Ena;
    DBGRETURN(ret);

    for( i = 0; i < MACSEC_PORT_COUNT; i++ ) {

        ret = Ra01AccRsMcsCpmRxEnableRxmcsInsertion( rmsDev_p, i, &(ingress->rxmcsInsertion[i].insertion) , RA01_READ_OP);
        DBGRETURN(ret);
        ingress->rxmcsInsertion[i].valid = true;
    }
    for( i = 0; i < MACSEC_CHANNELS_PER_PORT; i++ ) {
        Ra01RsMcsTopChannelConfig_t chCfg;
        ret = Ra01AccRsMcsTopChannelConfig( rmsDev_p, i, &chCfg , RA01_READ_OP);
        DBGRETURN(ret);
        ingress->chBypass[i] = chCfg.chBypass;
    }

    Ra01RsMcsCpmRxPnThreshold_t pnThreshold;
    ret = Ra01AccRsMcsCpmRxPnThreshold(rmsDev_p, &pnThreshold, RA01_READ_OP);
    DBGRETURN(ret);
    ingress->pnThreshold = pnThreshold.pnThreshold;

    Ra01RsMcsCpmRxXpnThreshold_t xpnThreshold;
    ret = Ra01AccRsMcsCpmRxXpnThreshold(rmsDev_p, &xpnThreshold, RA01_READ_OP);
    DBGRETURN(ret);
    uint64_t xpnThresholdMsbShifted = (uint64_t)xpnThreshold.xpnThresholdMsb;
    xpnThresholdMsbShifted <<= 32;
    ingress->xpnThreshold = xpnThresholdMsbShifted | (uint64_t)xpnThreshold.xpnThresholdLsb;

    Ra01RsMcsCseRxCtrl_t cseRxCtrl;
    ret = Ra01AccRsMcsCseRxCtrl(rmsDev_p, &cseRxCtrl, RA01_READ_OP);
    DBGRETURN(ret);
    ingress->statsClearOnRead = cseRxCtrl.cpuCseClrOnRd;

    Ra01RsMcsPexEarlyPreemptFilterCtrl_t earlyPreemptFilterCtrl;
    ret = Ra01AccRsMcsPexEarlyPreemptFilterCtrl(rmsDev_p, RA01_RS_MCS_PEX_RX_SLAVE, 0, &earlyPreemptFilterCtrl, RA01_READ_OP);
    DBGRETURN(ret);
    ingress->earlyPreemptFilterEnable = earlyPreemptFilterCtrl.earlyPreemptFilterEnable;
    
    return ret;
}

static int setEgressParameters( RmsDev_t * rmsDev_p, MacsecConfig_t * cfg )
{
    int                       ret = 0;
    int                       i;
    MacsecTxParam_t * egress = &(cfg->egress);

    if( egress->opt.setOptParameters ) {
        ret = Ra01AccRsMcsBbePaddingCtl( rmsDev_p, RA01_RS_MCS_BBE_TX_SLAVE, &(egress->opt.padding) , RA01_WRITE_OP);
        DBGRETURN(ret);

        ret =  Ra01AccRsMcsPabIncludeTagCnt( rmsDev_p, RA01_RS_MCS_PAB_TX_SLAVE, &(egress->opt.includeTagCount) , RA01_WRITE_OP);
        DBGRETURN(ret);

        Ra01RsMcsCpmTxFixedOffsetAdjust_t foa;
        foa.fixedOffsetAdjust = egress->opt.fixedOffsetAdjust;
        ret = Ra01AccRsMcsCpmTxFixedOffsetAdjust( rmsDev_p, &foa , RA01_WRITE_OP);
        DBGRETURN(ret);
    }

    Ra01RsMcsCpmTxPnThreshold_t pn;
    pn.pnThreshold = egress->pnThreshold;
    ret = Ra01AccRsMcsCpmTxPnThreshold( rmsDev_p, &pn, RA01_WRITE_OP);
    DBGRETURN(ret);

    Ra01RsMcsCpmTxXpnThreshold_t xpn;
    xpn.xpnThresholdMsb = (egress->xpnThreshold >> 32) & 0xffffffff;
    xpn.xpnThresholdLsb = egress->xpnThreshold & 0xffffffff;
    ret = Ra01AccRsMcsCpmTxXpnThreshold( rmsDev_p, &xpn, RA01_WRITE_OP);
    DBGRETURN(ret);

    for( i = 0; i < MACSEC_PORT_COUNT; i++ ) {
        if( egress->opt.setOptParameters ) {
            Ra01RsMcsCpmTxTxPortCfg_t pc;
            pc.sectagEtype = egress->opt.sectagEtype[i];
            ret = Ra01AccRsMcsCpmTxTxPortCfg( rmsDev_p, i, &pc , RA01_WRITE_OP);
            DBGRETURN(ret);
        }
    }

    for( i = 0; i < MACSEC_CHANNELS_PER_PORT; i++ ) {
        Ra01RsMcsTopChannelConfig_t chCfg;
        chCfg.chBypass = egress->chBypass[i];
        ret = Ra01AccRsMcsTopChannelConfig( rmsDev_p, i, &chCfg , RA01_WRITE_OP);
        DBGRETURN(ret);
    }

    Ra01RsMcsPabTxTrunc_t te;
    te.enableTrunc = egress->enableTxTrunc;
    ret = Ra01AccRsMcsPabTxTrunc( rmsDev_p, RA01_RS_MCS_PAB_TX_SLAVE, &te , RA01_WRITE_OP);
    DBGRETURN(ret);

    Ra01RsMcsCseTxCtrl_t cseTxCtrl;
    cseTxCtrl.cpuCseClrOnRd = egress->statsClearOnRead;
    ret = Ra01AccRsMcsCseTxCtrl(rmsDev_p, &cseTxCtrl, RA01_WRITE_OP);
    DBGRETURN(ret);

    Ra01RsMcsPexEarlyPreemptFilterCtrl_t earlyPreemptFilterCtrl;
    earlyPreemptFilterCtrl.earlyPreemptFilterEnable = egress->earlyPreemptFilterEnable;
    earlyPreemptFilterCtrl.earlyPreemptMinNumWords = 11;
    ret = Ra01AccRsMcsPexEarlyPreemptFilterCtrl(rmsDev_p, RA01_RS_MCS_PEX_TX_SLAVE, 0, &earlyPreemptFilterCtrl, RA01_WRITE_OP);
    DBGRETURN(ret);

    return ret;
}

static int getEgressParameters( RmsDev_t * rmsDev_p, MacsecTxParam_t * egress )
{
    int                ret = 0;
    int                i;

    ret = Ra01AccRsMcsBbePaddingCtl( rmsDev_p, RA01_RS_MCS_BBE_TX_SLAVE, &(egress->opt.padding) , RA01_READ_OP);
    DBGRETURN(ret);

    Ra01RsMcsCpmTxFixedOffsetAdjust_t foa;
    ret = Ra01AccRsMcsCpmTxFixedOffsetAdjust( rmsDev_p, &foa , RA01_READ_OP);
    DBGRETURN(ret);
    egress->opt.fixedOffsetAdjust = foa.fixedOffsetAdjust;

    for( i = 0; i < MACSEC_PORT_COUNT; i++ ) {
        Ra01RsMcsCpmTxTxPortCfg_t pc;
        ret = Ra01AccRsMcsCpmTxTxPortCfg( rmsDev_p, i, &pc , RA01_READ_OP);
        egress->opt.sectagEtype[i] = pc.sectagEtype;
        DBGRETURN(ret);
    }
    for( i = 0; i < MACSEC_CHANNELS_PER_PORT; i++ ) {
        Ra01RsMcsTopChannelConfig_t chCfg;
        ret = Ra01AccRsMcsTopChannelConfig( rmsDev_p, i, &chCfg , RA01_READ_OP);
        DBGRETURN(ret);
        egress->chBypass[i] = chCfg.chBypass;
    }
    egress->opt.setOptParameters = true;

    Ra01RsMcsCpmTxPnThreshold_t pnThreshold;
    ret = Ra01AccRsMcsCpmTxPnThreshold(rmsDev_p, &pnThreshold, RA01_READ_OP);
    DBGRETURN(ret);
    egress->pnThreshold = pnThreshold.pnThreshold;

    Ra01RsMcsCpmTxXpnThreshold_t xpnThreshold;
    ret = Ra01AccRsMcsCpmTxXpnThreshold(rmsDev_p, &xpnThreshold, RA01_READ_OP);
    DBGRETURN(ret);
    uint64_t xpnThresholdMsbShifted = (uint64_t)xpnThreshold.xpnThresholdMsb;
    xpnThresholdMsbShifted <<= 32;
    egress->xpnThreshold = xpnThresholdMsbShifted | (uint64_t)xpnThreshold.xpnThresholdLsb;

    Ra01RsMcsPabTxTrunc_t te;
    ret = Ra01AccRsMcsPabTxTrunc( rmsDev_p, RA01_RS_MCS_PAB_TX_SLAVE, &te , RA01_READ_OP);
    DBGRETURN(ret);
    egress->enableTxTrunc = te.enableTrunc;

    Ra01RsMcsCseTxCtrl_t cseTxCtrl;
    ret = Ra01AccRsMcsCseTxCtrl(rmsDev_p, &cseTxCtrl, RA01_READ_OP);
    DBGRETURN(ret);
    egress->statsClearOnRead = cseTxCtrl.cpuCseClrOnRd;

    Ra01RsMcsPexEarlyPreemptFilterCtrl_t earlyPreemptFilterCtrl;
    ret = Ra01AccRsMcsPexEarlyPreemptFilterCtrl(rmsDev_p, RA01_RS_MCS_PEX_TX_SLAVE, 0, &earlyPreemptFilterCtrl, RA01_READ_OP);
    DBGRETURN(ret);
    egress->earlyPreemptFilterEnable = earlyPreemptFilterCtrl.earlyPreemptFilterEnable;

    return ret;
}

//------------------------------------------------------------------------------
// Misc section
//------------------------------------------------------------------------------
int MacsecInitialize( RmsDev_t * rmsDev_p)
{
    int ret = 0;

    if( rmsDev_p == NULL ) {
        return -EINVAL;
    }

    return ret;
}

int MacsecUninitialize( RmsDev_t * rmsDev_p )
{
    int ret = 0;

    if( rmsDev_p == NULL ) {
        return -EINVAL;
    }

    return ret;
}

int MacsecSetFixedLatency(RmsDev_t * rmsDev_p, MacsecPortRate_t portRate, bool enableTxFixedLatency, bool enableRxFixedLatency, bool enableTxPreemptFixedLatency, bool enableRxPreemptFixedLatency)
{
  int ret = 0;
  int tx_period_clk, rx_period_clk;

  Ra01RsMcsPabDelayCfg_t txPabDelayCfg, rxPabDelayCfg;

  txPabDelayCfg.enablePreemptFixedLatency = enableTxPreemptFixedLatency;
  rxPabDelayCfg.enablePreemptFixedLatency = enableRxPreemptFixedLatency;
  
  switch (portRate) {
  case MACSEC_10G:
    tx_period_clk = 71;
    rx_period_clk = 75;
    break;
  case MACSEC_5G:
    tx_period_clk = 87;
    rx_period_clk = 94;
    break;
  case MACSEC_2_5G:
    tx_period_clk = 87;
    rx_period_clk = 94;
    break;
  case MACSEC_1G:
    tx_period_clk = 225;
    rx_period_clk = 206;
    break;
  case MACSEC_100M:
    tx_period_clk = 1888;
    rx_period_clk = 1553;
    break;
  case MACSEC_10M:
    tx_period_clk = 18520;
    rx_period_clk = 15017;
    break;
  default:
    tx_period_clk = 0;
    rx_period_clk = 0;
  }

  if (!enableTxFixedLatency) {
    tx_period_clk = 0;
  } 
  if (!enableRxFixedLatency) {
    rx_period_clk = 0;
  }

  int flops = MACSEC_PAB_OUTPUT_FLOP_COUNT + MACSEC_PEX_OUTPUT_FLOP_COUNT;
  txPabDelayCfg.fixedLatency = tx_period_clk - ((tx_period_clk < flops) ? 0 : flops);
  rxPabDelayCfg.fixedLatency = rx_period_clk - ((rx_period_clk < flops) ? 0 : flops);
  ret = Ra01AccRsMcsPabDelayCfg(rmsDev_p, RA01_RS_MCS_PAB_RX_SLAVE, 0, &rxPabDelayCfg, RA01_WRITE_OP);
  DBGRETURN(ret);
  ret = Ra01AccRsMcsPabDelayCfg(rmsDev_p, RA01_RS_MCS_PAB_TX_SLAVE, 0, &txPabDelayCfg, RA01_WRITE_OP);
  DBGRETURN(ret);

  return ret;
}

int MacsecEnableAutoRekey( RmsDev_t * rmsDev_p, unsigned index, bool enable )
{
    if( rmsDev_p == NULL ) {
        return -EINVAL;
    }

    return autoRekey( rmsDev_p, index, enable );
}




int MacsecSetEnableSc( RmsDev_t * rmsDev_p, unsigned index, bool enable )
{
    int ret = -EINVAL;

    if( rmsDev_p == NULL ) {
        return -EINVAL;
    }

    if( index < (INGRESS_SC_CAM_LOOKUP_TABLE_SIZE) ) {
        Ra01RsMcsCpmRxScCamEnable_t en;
        unsigned bit = index % (sizeof(ra01_register_t)*8);
        unsigned regIdx = index / (sizeof( ra01_register_t )*8);

        ret = Ra01AccRsMcsCpmRxScCamEnable( rmsDev_p, regIdx, &en, RA01_READ_OP);
        DBGRETURN(ret);

        if( enable ) {
            en.enable |= (1 << bit );
        } else {
            en.enable &= ~(1 << bit );
        }
        ret = Ra01AccRsMcsCpmRxScCamEnable( rmsDev_p, regIdx, &en, RA01_WRITE_OP);
    }

    return ret;
}

int MacsecGetEnableSc( RmsDev_t * rmsDev_p, unsigned index, bool * enabled )
{
    int ret = -EINVAL;

    if( (rmsDev_p == NULL) || (enabled == NULL) ) {
        return -EINVAL;
    }

    if( index < (INGRESS_SC_CAM_LOOKUP_TABLE_SIZE) ) {
        Ra01RsMcsCpmRxScCamEnable_t en;
        unsigned bit = index % (sizeof(ra01_register_t)*8);
        unsigned regIdx = index / (sizeof( ra01_register_t )*8);

        ret = Ra01AccRsMcsCpmRxScCamEnable( rmsDev_p, regIdx, &en, RA01_READ_OP);
        DBGRETURN(ret);

        *enabled = !!(en.enable & (1 << bit ));
    }

    return ret;
}

//------------------------------------------------------------------------------
// SA section
//------------------------------------------------------------------------------

int MacsecSetSaPolicy( RmsDev_t *               rmsDev_p,
                             MacsecDirection_t        direction,
                             unsigned                 index,
                             MacsecSaPolicy_t  * policy,
                             bool                     lock )
{
    int ret = -EINVAL;

    if( (rmsDev_p == NULL) || (policy == NULL) ) {
        return -EINVAL;
    }

    if( direction == MACSEC_INGRESS ) {
        ret = Ra01WriteRsMcsCpmRxSaPlcy( rmsDev_p, index, &(policy->ingress) );
        DBGRETURN(ret);
    } else {
        ret = Ra01WriteRsMcsCpmTxSaPlcy( rmsDev_p, index, &(policy->egress) );
        DBGRETURN(ret);
    }

    ret = updateSaKeyLockout( rmsDev_p, direction, index, lock );

    return ret;
}

int MacsecGetSaPolicy( RmsDev_t *               rmsDev_p,
                                  MacsecDirection_t        direction,
                                  unsigned                 index,
                                  MacsecSaPolicy_t *       policy,
                                  bool *                   lock )
{
    int ret = -EINVAL;
    unsigned reg_index = index / (8 * sizeof(ra01_register_t));
    unsigned bit_index = index % (8 * sizeof(ra01_register_t));

    if( (rmsDev_p == NULL) || (policy == NULL) ) {
        return -EINVAL;
    }

    if( direction == MACSEC_INGRESS ) {
        Ra01RsMcsCpmRxSaKeyLockout_t value;

        ret = Ra01ReadRsMcsCpmRxSaPlcy( rmsDev_p, index, &(policy->ingress) );
        DBGRETURN(ret);
        
        ret = Ra01AccRsMcsCpmRxSaKeyLockout( rmsDev_p, reg_index, &value, RA01_READ_OP);
        DBGRETURN(ret);
        
        (*lock) = (value.sa >> bit_index)?true:false;
    } else {
        Ra01RsMcsCpmTxSaKeyLockout_t value;

        ret = Ra01ReadRsMcsCpmTxSaPlcy( rmsDev_p, index, &(policy->egress) );
        DBGRETURN(ret);
        
        ret = Ra01AccRsMcsCpmTxSaKeyLockout( rmsDev_p, reg_index, &value, RA01_READ_OP);
        DBGRETURN(ret);        

        (*lock) = (value.sa >> bit_index)?true:false;
    }

    return ret;
}

int MacsecSetNextPn( RmsDev_t * rmsDev_p, MacsecDirection_t direction, unsigned index, uint64_t nextPn )
{
    int ret = 0;

    if( rmsDev_p == NULL ) {
        return -EINVAL;
    }

    if( direction == MACSEC_INGRESS ) {
        Ra01IfRsMcsCpmRxSaPnTable_t pnTable;

        pnTable.nextPn = nextPn;

        ret = Ra01WriteRsMcsCpmRxSaPnTable( rmsDev_p, index, &pnTable );
    } else {
        Ra01IfRsMcsCpmTxSaPnTable_t pnTable;

        pnTable.nextPn = nextPn;

        ret = Ra01WriteRsMcsCpmTxSaPnTable( rmsDev_p, index, &pnTable );
    }

    return ret;
}

int MacsecGetNextPn( RmsDev_t * rmsDev_p, MacsecDirection_t direction, unsigned index, uint64_t * nextPn )
{
    int ret = 0;

    if( (rmsDev_p == NULL) || (nextPn == NULL) ) {
        return -EINVAL;
    }

    if( direction == MACSEC_INGRESS ) {
        Ra01IfRsMcsCpmRxSaPnTable_t pnTable;

        ret = Ra01ReadRsMcsCpmRxSaPnTable( rmsDev_p, index, &pnTable );
        DBGRETURN(ret);

        *nextPn = pnTable.nextPn;
    } else {
        Ra01IfRsMcsCpmTxSaPnTable_t pnTable;

        ret = Ra01ReadRsMcsCpmTxSaPnTable( rmsDev_p, index, &pnTable );
        DBGRETURN(ret);

        *nextPn = pnTable.nextPn;
    }

    return ret;
}

int MacsecGetEnableIngressSa( RmsDev_t * rmsDev_p, unsigned index, uint32_t *saIndex, bool *enable )
{
    Ra01IfRsMcsCpmRxSaMap_t map;
    int ret = 0;
    
    if( rmsDev_p == NULL ) {
        return -EINVAL;
    }

    ret = Ra01ReadRsMcsCpmRxSaMap( rmsDev_p, index, &map );

    (*enable)  = map.saInUse;

    (*saIndex) = (uint32_t)map.saIndex;

    
    return ret;
}

int MacsecEnableIngressSa( RmsDev_t * rmsDev_p, unsigned index, uint32_t saIndex, bool enable )
{
    Ra01IfRsMcsCpmRxSaMap_t map;

    if( rmsDev_p == NULL ) {
        return -EINVAL;
    }

    map.saInUse = enable;
#if RA01_RS_MCS_CPM_TX_SA_MAP_MEM_SIZE <= 256
    map.saIndex = (uint8_t)saIndex;
#else
    map.saIndex = (uint16_t)saIndex;
#endif

    return Ra01WriteRsMcsCpmRxSaMap( rmsDev_p, index, &map );
}

int MacsecGetEgressSaMap( RmsDev_t * rmsDev_p, unsigned index, MacsecEgressSaMapEntry_t * saMap )
{
    int ret = 0;

    if( (rmsDev_p == NULL) || (saMap == NULL) ) {
        return -EINVAL;
    }

    if( index >= (EGRESS_SECY_TO_SA_MAP_TABLE_SIZE) ) {
        return -EINVAL;
    }

    Ra01IfRsMcsCpmTxSaMap_t sa;
    ret = Ra01ReadRsMcsCpmTxSaMap( rmsDev_p, index, &sa );

    saMap->autoRekeyEnable = sa.autoRekeyEnable;
    saMap->saIndex0Vld     = sa.saIndex0Vld;
    saMap->saIndex1Vld     = sa.saIndex1Vld;
    saMap->txSaActive      = sa.txSaActive;
    saMap->saIndex0        = sa.saIndex0;
    saMap->saIndex1        = sa.saIndex1;
    saMap->sectagSci       = sa.sectagSci;

    return ret;
}

int MacsecSetEgressSaMap( RmsDev_t * rmsDev_p, unsigned index, MacsecEgressSaMapEntry_t * saMap )
{
    int ret = 0;

    if( (rmsDev_p == NULL) || (saMap == NULL) ) {
        return -EINVAL;
    }

    if( index >= (EGRESS_SECY_TO_SA_MAP_TABLE_SIZE) ) {
        return -EINVAL;
    }

    Ra01IfRsMcsCpmTxSaMap_t sa;   
    sa.autoRekeyEnable = saMap->autoRekeyEnable;
    sa.saIndex0Vld     = saMap->saIndex0Vld;
    sa.saIndex1Vld     = saMap->saIndex1Vld;
    sa.txSaActive      = saMap->txSaActive;
#if RA01_RS_MCS_CPM_TX_SA_MAP_MEM_SIZE <= 256
    sa.saIndex0        = (uint8_t)saMap->saIndex0;
    sa.saIndex1        = (uint8_t)saMap->saIndex1;
#else
    sa.saIndex0        = (uint16_t)saMap->saIndex0;
    sa.saIndex1        = (uint16_t)saMap->saIndex1;
#endif
    sa.sectagSci       = saMap->sectagSci;
    ret = Ra01WriteRsMcsCpmTxSaMap( rmsDev_p, index, &sa );

    return ret;
}


//------------------------------------------------------------------------------
// Slave interfaces
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// MCU_TOP
//------------------------------------------------------------------------------
int MacsecSetConfiguration( RmsDev_t * rmsDev_p, MacsecConfig_t * cfg )
{
    int      ret = 0;
    unsigned i;

    //SIM_PRINT(SIM_DEBUG, "Entry: %s", __func__ );

    if( (rmsDev_p == NULL) || (cfg == NULL) ) {
        return -EINVAL;
    }

    ret = MacsecPabSetPortMode( rmsDev_p, cfg->portMode );
    DBGRETURN(ret);

    if( cfg->ingress.valid ) {
        ret = setIngressParameters( rmsDev_p, cfg );
        DBGRETURN(ret);
    }

    if( cfg->egress.valid ) {
        ret = setEgressParameters( rmsDev_p, cfg );
        DBGRETURN(ret);
    }

    for( i = 0; i < MACSEC_PORT_COUNT; i++ ) {
        if( cfg->symmetric[i].valid ) {
            ret = Ra01AccRsMcsTopPortConfig( rmsDev_p, i, &(cfg->symmetric[i].portConfig) , RA01_WRITE_OP);
        }
    }

    Ra01RsMcsPabFifoSkidCfg_t skidCfg;
    skidCfg.bbePabSkidExp = 24;
    skidCfg.bbePabSkidPre = 24;
    Ra01AccRsMcsPabFifoSkidCfg(rmsDev_p, RA01_RS_MCS_PAB_TX_SLAVE, 0, &skidCfg, RA01_WRITE_OP); 
    Ra01AccRsMcsPabFifoSkidCfg(rmsDev_p, RA01_RS_MCS_PAB_RX_SLAVE, 0, &skidCfg, RA01_WRITE_OP); 
    return ret;
}

int MacsecGetConfiguration( RmsDev_t * rmsDev_p, MacsecConfig_t * cfg )
{
    int              ret = 0;
    int              i;

    if( (rmsDev_p == NULL) || (cfg == NULL) ) {
        return -EINVAL;
    }

    ret = Ra01AccRsMcsPabIncludeTagCnt( rmsDev_p, RA01_RS_MCS_PAB_RX_SLAVE, &(cfg->ingress.opt.includeTagCount) , RA01_READ_OP);
    DBGRETURN(ret);

    ret =  Ra01AccRsMcsPabIncludeTagCnt( rmsDev_p, RA01_RS_MCS_PAB_TX_SLAVE, &(cfg->egress.opt.includeTagCount) , RA01_READ_OP);
    DBGRETURN(ret);

    cfg->ingress.valid = true;
    ret = getIngressParameters( rmsDev_p, &(cfg->ingress) );
    DBGRETURN(ret);

    cfg->egress.valid = true;
    ret = getEgressParameters( rmsDev_p, &(cfg->egress) );
    DBGRETURN(ret);

    for( i = 0; i < MACSEC_PORT_COUNT; i++ ) {
        ret = Ra01AccRsMcsTopPortConfig( rmsDev_p, i, &(cfg->symmetric[i].portConfig) , RA01_READ_OP);
        DBGRETURN(ret);

        cfg->symmetric[i].valid = true;
    }

    cfg->portMode = MACSEC_PORT_MODE_1X10;

    return ret;
}

int MacsecSetPnThreshold( RmsDev_t *                rmsDev_p,
                          MacsecDirection_t         direction,
			  int                       pnThreshold ) {
  int ret;

  if (direction == MACSEC_EGRESS) {
    Ra01RsMcsCpmTxPnThreshold_t txPnThreshold;
    txPnThreshold.pnThreshold = pnThreshold;
    ret = Ra01AccRsMcsCpmTxPnThreshold(rmsDev_p, &txPnThreshold, RA01_WRITE_OP);
  } else {
    Ra01RsMcsCpmRxPnThreshold_t rxPnThreshold;
    rxPnThreshold.pnThreshold = pnThreshold;
    ret = Ra01AccRsMcsCpmRxPnThreshold(rmsDev_p, &rxPnThreshold, RA01_WRITE_OP);
  }

  DBGRETURN(ret);
  return ret;
}

int MacsecSetXpnThreshold( RmsDev_t *                rmsDev_p,
			   MacsecDirection_t         direction,
			   long long                 xpnThreshold ) {
  int ret;

  if (direction == MACSEC_EGRESS) {
    Ra01RsMcsCpmTxXpnThreshold_t txXpnThreshold;
    txXpnThreshold.xpnThresholdLsb = (uint32_t)xpnThreshold;
    txXpnThreshold.xpnThresholdMsb = (uint32_t)(xpnThreshold >> 32);
    ret = Ra01AccRsMcsCpmTxXpnThreshold(rmsDev_p, &txXpnThreshold, RA01_WRITE_OP);
  } else {
    Ra01RsMcsCpmRxXpnThreshold_t rxXpnThreshold;
    rxXpnThreshold.xpnThresholdLsb = (uint32_t)xpnThreshold;
    rxXpnThreshold.xpnThresholdMsb = (uint32_t)(xpnThreshold >> 32);
    ret = Ra01AccRsMcsCpmRxXpnThreshold(rmsDev_p, &rxXpnThreshold, RA01_WRITE_OP);
  }

  DBGRETURN(ret);
  return ret;
}

int MacsecSetChannelBypass(RmsDev_t * rmsDev_p, unsigned index, bool isBypass) {
  int ret;
  Ra01RsMcsTopChannelConfig_t chCfg;
  chCfg.chBypass = isBypass;
  ret = Ra01AccRsMcsTopChannelConfig( rmsDev_p, index, &chCfg , RA01_WRITE_OP);
  DBGRETURN(ret);
  return ret;
}

int MacsecGetChannelBypass(RmsDev_t * rmsDev_p, unsigned index, bool * isBypass) {
  int ret;
  Ra01RsMcsTopChannelConfig_t chCfg;
  ret = Ra01AccRsMcsTopChannelConfig( rmsDev_p, index, &chCfg , RA01_READ_OP);
  DBGRETURN(ret);
  *isBypass = chCfg.chBypass;
  return ret;
}

int MacsecEnableInterrupts( RmsDev_t * rmsDev_p ) {
  BlockInterruptEnables_t blockInterruptEnables;
  int ret;

  blockInterruptEnables.rxBbeSlaveInterruptEnb = 1;
  blockInterruptEnables.txBbeSlaveInterruptEnb = 1;
  blockInterruptEnables.rxPabSlaveInterruptEnb = 1;
  blockInterruptEnables.txPabSlaveInterruptEnb = 1;
  blockInterruptEnables.rxPexSlaveInterruptEnb = 1;
  blockInterruptEnables.txPexSlaveInterruptEnb = 1;
  blockInterruptEnables.rxCpmSlaveInterruptEnb = 1;
  blockInterruptEnables.txCpmSlaveInterruptEnb = 1;
  ret = MacsecEnableBlockInterrupts(rmsDev_p, &(blockInterruptEnables));
  DBGRETURN(ret);

  return ret;
}

int MacsecEnableBlockInterrupts( RmsDev_t * rmsDev_p, BlockInterruptEnables_t const * blockInterruptEnables )
{
  int      ret = 0;

  if( (rmsDev_p == NULL) || (blockInterruptEnables == NULL) ) {
    return -EINVAL;
  }

  Ra01RsMcsTopRsMcsTopSlaveIntSumEnb_t topIntEnb;
  topIntEnb.rsMcsBbeSlaveInterruptEnb = ((uint8_t)(blockInterruptEnables->txBbeSlaveInterruptEnb << 1)) | (uint8_t)blockInterruptEnables->rxBbeSlaveInterruptEnb;
  topIntEnb.rsMcsPabSlaveInterruptEnb = ((uint8_t)(blockInterruptEnables->txPabSlaveInterruptEnb << 1)) | (uint8_t)blockInterruptEnables->rxPabSlaveInterruptEnb;
  topIntEnb.rsMcsPexSlaveInterruptEnb = ((uint8_t)(blockInterruptEnables->txPexSlaveInterruptEnb << 1)) | (uint8_t)blockInterruptEnables->rxPexSlaveInterruptEnb;
  topIntEnb.rsMcsCpmRxSlaveInterruptEnb = blockInterruptEnables->rxCpmSlaveInterruptEnb;
  topIntEnb.rsMcsCpmTxSlaveInterruptEnb = blockInterruptEnables->txCpmSlaveInterruptEnb;
        
  ret = Ra01AccRsMcsTopRsMcsTopSlaveIntSumEnb(rmsDev_p, &(topIntEnb), RA01_WRITE_OP);
  DBGRETURN(ret);

  if (blockInterruptEnables->rxBbeSlaveInterruptEnb) {
    Ra01RsMcsBbeBbeIntEnb_t rx_bbe_int_enb;
    rx_bbe_int_enb.dfifoOverflowEnb = 1;
    rx_bbe_int_enb.plfifoOverflowEnb = 1;
    ret = Ra01AccRsMcsBbeBbeIntEnb(rmsDev_p, RA01_RS_MCS_BBE_RX_SLAVE, &(rx_bbe_int_enb), RA01_WRITE_OP);
    DBGRETURN(ret);
  }

  if (blockInterruptEnables->txBbeSlaveInterruptEnb) {
    Ra01RsMcsBbeBbeIntEnb_t tx_bbe_int_enb;
    tx_bbe_int_enb.dfifoOverflowEnb = 1;
    tx_bbe_int_enb.plfifoOverflowEnb = 1;
    ret = Ra01AccRsMcsBbeBbeIntEnb(rmsDev_p, RA01_RS_MCS_BBE_TX_SLAVE, &(tx_bbe_int_enb), RA01_WRITE_OP);
    DBGRETURN(ret);
  }

  if (blockInterruptEnables->rxPabSlaveInterruptEnb) {
    Ra01RsMcsPabPabIntEnb_t rx_pab_int_enb;
    rx_pab_int_enb.overflowEnb = 0x3;
    ret = Ra01AccRsMcsPabPabIntEnb(rmsDev_p, RA01_RS_MCS_PAB_RX_SLAVE, &(rx_pab_int_enb), RA01_WRITE_OP);
    DBGRETURN(ret);
  }

  if (blockInterruptEnables->txPabSlaveInterruptEnb) {
    Ra01RsMcsPabPabIntEnb_t tx_pab_int_enb;
    tx_pab_int_enb.overflowEnb = 0x3;
    ret = Ra01AccRsMcsPabPabIntEnb(rmsDev_p, RA01_RS_MCS_PAB_TX_SLAVE, &(tx_pab_int_enb), RA01_WRITE_OP);
    DBGRETURN(ret);
  }

  if (blockInterruptEnables->rxPexSlaveInterruptEnb) {
    Ra01RsMcsPexPexIntEnb_t rx_pex_int_enb;
    rx_pex_int_enb.earlyPreemptErrEnb = 1;
    ret = Ra01AccRsMcsPexPexIntEnb(rmsDev_p, RA01_RS_MCS_PEX_RX_SLAVE, &(rx_pex_int_enb), RA01_WRITE_OP);
    DBGRETURN(ret);
  }

  if (blockInterruptEnables->txPexSlaveInterruptEnb) {
    Ra01RsMcsPexPexIntEnb_t tx_pex_int_enb;
    tx_pex_int_enb.earlyPreemptErrEnb = 1;
    ret = Ra01AccRsMcsPexPexIntEnb(rmsDev_p, RA01_RS_MCS_PEX_TX_SLAVE, &(tx_pex_int_enb), RA01_WRITE_OP);
    DBGRETURN(ret);
  }

  if (blockInterruptEnables->rxCpmSlaveInterruptEnb) {
    Ra01RsMcsCpmRxCpmRxIntEnb_t rx_cpm_int_enb;
    rx_cpm_int_enb.sectagVEq1Enb = 1;
    rx_cpm_int_enb.sectagEEq0CEq1Enb = 1;
    rx_cpm_int_enb.sectagSlGte48Enb = 1;
    rx_cpm_int_enb.sectagEsEq1ScEq1Enb = 1;
    rx_cpm_int_enb.sectagScEq1ScbEq1Enb = 1;
    rx_cpm_int_enb.packetXpnEq0Enb = 1;
    rx_cpm_int_enb.pnThreshReachedEnb = 1;
    rx_cpm_int_enb.scExpiryPreTimeoutEnb = 1;
    rx_cpm_int_enb.scExpiryActualTimeoutEnb = 1;
    ret = Ra01AccRsMcsCpmRxCpmRxIntEnb(rmsDev_p, &(rx_cpm_int_enb), RA01_WRITE_OP);
    DBGRETURN(ret);
    Ra01RsMcsCpmRxScExpiryPreTimeoutEnb_t preTimeoutEnb;
    preTimeoutEnb.scExpiryPreTimeoutIntEnb = 0xffffffff;
    Ra01AccRsMcsCpmRxScExpiryPreTimeoutEnb(rmsDev_p, 0, &preTimeoutEnb, RA01_WRITE_OP);
    DBGRETURN(ret);
    Ra01RsMcsCpmRxScExpiryActualTimeoutEnb_t actualTimeoutEnb;
    actualTimeoutEnb.scExpiryActualTimeoutIntEnb = 0xffffffff;
    Ra01AccRsMcsCpmRxScExpiryActualTimeoutEnb(rmsDev_p, 0, &actualTimeoutEnb, RA01_WRITE_OP);
    DBGRETURN(ret);
  }

  if (blockInterruptEnables->txCpmSlaveInterruptEnb) {
    Ra01RsMcsCpmTxCpmTxIntEnb_t tx_cpm_int_enb;
    tx_cpm_int_enb.packetXpnEq0Enb = 1;
    tx_cpm_int_enb.pnThreshReachedEnb = 1;
    tx_cpm_int_enb.saNotValidEnb = 1;
    tx_cpm_int_enb.scExpiryPreTimeoutEnb = 1;
    tx_cpm_int_enb.scExpiryActualTimeoutEnb = 1;
    ret = Ra01AccRsMcsCpmTxCpmTxIntEnb(rmsDev_p, &(tx_cpm_int_enb), RA01_WRITE_OP);
    DBGRETURN(ret);
    Ra01RsMcsCpmTxScExpiryPreTimeoutEnb_t preTimeoutEnb;
    preTimeoutEnb.scExpiryPreTimeoutIntEnb = 0xffffffff;
    Ra01AccRsMcsCpmTxScExpiryPreTimeoutEnb(rmsDev_p, 0, &preTimeoutEnb, RA01_WRITE_OP);
    DBGRETURN(ret);
    Ra01RsMcsCpmTxScExpiryActualTimeoutEnb_t actualTimeoutEnb;
    actualTimeoutEnb.scExpiryActualTimeoutIntEnb = 0xffffffff;
    Ra01AccRsMcsCpmTxScExpiryActualTimeoutEnb(rmsDev_p, 0, &actualTimeoutEnb, RA01_WRITE_OP);
    DBGRETURN(ret);
  }

  return ret;
}

int MacsecGetInterruptEnables( RmsDev_t * rmsDev_p, MacsecInterrupts_t * interruptEnables )
{
  int ret = 0;

    if( (rmsDev_p == NULL) || (interruptEnables == NULL) ) {
        return -EINVAL;
    }

 Ra01RsMcsBbeBbeIntEnb_t rx_bbe_int_enb;
 ret = Ra01AccRsMcsBbeBbeIntEnb(rmsDev_p, RA01_RS_MCS_BBE_RX_SLAVE, &(rx_bbe_int_enb), RA01_READ_OP);
 DBGRETURN(ret);
 interruptEnables->bbeRxInterrupts.dfifoOverflow = rx_bbe_int_enb.dfifoOverflowEnb;
 interruptEnables->bbeRxInterrupts.plfifoOverflow = rx_bbe_int_enb.plfifoOverflowEnb;

 Ra01RsMcsBbeBbeIntEnb_t tx_bbe_int_enb;
 ret = Ra01AccRsMcsBbeBbeIntEnb(rmsDev_p, RA01_RS_MCS_BBE_TX_SLAVE, &(tx_bbe_int_enb), RA01_READ_OP);
 DBGRETURN(ret);
 interruptEnables->bbeTxInterrupts.dfifoOverflow = tx_bbe_int_enb.dfifoOverflowEnb;
 interruptEnables->bbeTxInterrupts.plfifoOverflow = tx_bbe_int_enb.plfifoOverflowEnb;

 Ra01RsMcsPabPabIntEnb_t rx_pab_int_enb;
 ret = Ra01AccRsMcsPabPabIntEnb(rmsDev_p, RA01_RS_MCS_PAB_RX_SLAVE, &(rx_pab_int_enb), RA01_READ_OP);
 DBGRETURN(ret);
 interruptEnables->pabRxInterrupts.overflow = rx_pab_int_enb.overflowEnb;

 Ra01RsMcsPabPabIntEnb_t tx_pab_int_enb;
 ret = Ra01AccRsMcsPabPabIntEnb(rmsDev_p, RA01_RS_MCS_PAB_TX_SLAVE, &(tx_pab_int_enb), RA01_READ_OP);
 DBGRETURN(ret);
 interruptEnables->pabTxInterrupts.overflow = tx_pab_int_enb.overflowEnb;

 Ra01RsMcsPexPexIntEnb_t rx_pex_int_enb;
 ret = Ra01AccRsMcsPexPexIntEnb(rmsDev_p, RA01_RS_MCS_PEX_RX_SLAVE, &(rx_pex_int_enb), RA01_READ_OP);
 DBGRETURN(ret);
 interruptEnables->pexRxInterrupts.earlyPreemptErr = rx_pex_int_enb.earlyPreemptErrEnb;

 Ra01RsMcsPexPexIntEnb_t tx_pex_int_enb;
 ret = Ra01AccRsMcsPexPexIntEnb(rmsDev_p, RA01_RS_MCS_PEX_TX_SLAVE, &(tx_pex_int_enb), RA01_READ_OP);
 DBGRETURN(ret);
 interruptEnables->pexTxInterrupts.earlyPreemptErr = tx_pex_int_enb.earlyPreemptErrEnb;

 Ra01RsMcsCpmRxCpmRxIntEnb_t rx_cpm_int_enb;
 ret = Ra01AccRsMcsCpmRxCpmRxIntEnb(rmsDev_p, &(rx_cpm_int_enb), RA01_READ_OP);
 DBGRETURN(ret);
 interruptEnables->cpmRxInterrupts.sectagVEq1 = rx_cpm_int_enb.sectagVEq1Enb;
 interruptEnables->cpmRxInterrupts.sectagEEq0CEq1 = rx_cpm_int_enb.sectagEEq0CEq1Enb;
 interruptEnables->cpmRxInterrupts.sectagSlGte48 = rx_cpm_int_enb.sectagSlGte48Enb;
 interruptEnables->cpmRxInterrupts.sectagEsEq1ScEq1 = rx_cpm_int_enb.sectagEsEq1ScEq1Enb;
 interruptEnables->cpmRxInterrupts.sectagScEq1ScbEq1 = rx_cpm_int_enb.sectagScEq1ScbEq1Enb;
 interruptEnables->cpmRxInterrupts.packetXpnEq0 = rx_cpm_int_enb.packetXpnEq0Enb;
 interruptEnables->cpmRxInterrupts.pnThreshReached = rx_cpm_int_enb.pnThreshReachedEnb;

 Ra01RsMcsCpmRxScExpiryPreTimeoutEnb_t rxScExpiryPreTimeoutEnb;
 Ra01AccRsMcsCpmRxScExpiryPreTimeoutEnb(rmsDev_p, 0, &rxScExpiryPreTimeoutEnb, RA01_READ_OP);
 interruptEnables->cpmRxInterrupts.scExpiryPreTimeout = rxScExpiryPreTimeoutEnb.scExpiryPreTimeoutIntEnb;
 Ra01RsMcsCpmRxScExpiryActualTimeoutEnb_t rxScExpiryActualTimeoutEnb;
 Ra01AccRsMcsCpmRxScExpiryActualTimeoutEnb(rmsDev_p, 0, &rxScExpiryActualTimeoutEnb, RA01_READ_OP);
 interruptEnables->cpmRxInterrupts.scExpiryActualTimeout = rxScExpiryActualTimeoutEnb.scExpiryActualTimeoutIntEnb;

 Ra01RsMcsCpmTxCpmTxIntEnb_t tx_cpm_int_enb;
 ret = Ra01AccRsMcsCpmTxCpmTxIntEnb(rmsDev_p, &(tx_cpm_int_enb), RA01_READ_OP);
 DBGRETURN(ret);
 interruptEnables->cpmTxInterrupts.packetXpnEq0 = tx_cpm_int_enb.packetXpnEq0Enb;
 interruptEnables->cpmTxInterrupts.pnThreshReached = tx_cpm_int_enb.pnThreshReachedEnb;
 interruptEnables->cpmTxInterrupts.saNotValid = tx_cpm_int_enb.saNotValidEnb;
 
 Ra01RsMcsCpmTxScExpiryPreTimeoutEnb_t txScExpiryPreTimeoutEnb;
 Ra01AccRsMcsCpmTxScExpiryPreTimeoutEnb(rmsDev_p, 0, &txScExpiryPreTimeoutEnb, RA01_READ_OP);
 interruptEnables->cpmTxInterrupts.scExpiryPreTimeout = txScExpiryPreTimeoutEnb.scExpiryPreTimeoutIntEnb;
 Ra01RsMcsCpmTxScExpiryActualTimeoutEnb_t txScExpiryActualTimeoutEnb;
 Ra01AccRsMcsCpmTxScExpiryActualTimeoutEnb(rmsDev_p, 0, &txScExpiryActualTimeoutEnb, RA01_READ_OP);
 interruptEnables->cpmTxInterrupts.scExpiryActualTimeout = txScExpiryActualTimeoutEnb.scExpiryActualTimeoutIntEnb;
    return ret;
}

int MacsecClearInterrupts( RmsDev_t * rmsDev_p, MacsecInterrupts_t clearInterrupts ) {
   int ret = 0;

    if( rmsDev_p == NULL ) {
        return -EINVAL;
    }

   bool clearRxScExpiryPreTimeout = false, clearRxScExpiryActualTimeout = false, clearTxScExpiryPreTimeout = false, clearTxScExpiryActualTimeout = false;
   // if any bits cleared for leaf registers, we W1C to _INT register then propogate up one level if all bits now clear
   if (clearInterrupts.cpmRxInterrupts.scExpiryPreTimeout) {
      Ra01RsMcsCpmRxScExpiryPreTimeout_t rxScExpiryPreTimeout;
      ret = Ra01AccRsMcsCpmRxScExpiryPreTimeout(rmsDev_p, 0, &rxScExpiryPreTimeout, RA01_READ_OP);
      DBGRETURN(ret);
      if (clearInterrupts.cpmRxInterrupts.scExpiryPreTimeout == rxScExpiryPreTimeout.scExpiryPreTimeoutInt) {
         clearRxScExpiryPreTimeout = true;
      }
      rxScExpiryPreTimeout.scExpiryPreTimeoutInt = clearInterrupts.cpmRxInterrupts.scExpiryPreTimeout;
      ret = Ra01AccRsMcsCpmRxScExpiryPreTimeout(rmsDev_p, 0, &rxScExpiryPreTimeout, RA01_WRITE_OP);
      DBGRETURN(ret);
   }

   if (clearInterrupts.cpmRxInterrupts.scExpiryActualTimeout) {
      Ra01RsMcsCpmRxScExpiryActualTimeout_t rxScExpiryActualTimeout;
      ret = Ra01AccRsMcsCpmRxScExpiryActualTimeout(rmsDev_p, 0, &rxScExpiryActualTimeout, RA01_READ_OP);
      DBGRETURN(ret);
      if (clearInterrupts.cpmRxInterrupts.scExpiryActualTimeout == rxScExpiryActualTimeout.scExpiryActualTimeoutInt) {
         clearRxScExpiryActualTimeout = true;
      }
      rxScExpiryActualTimeout.scExpiryActualTimeoutInt = clearInterrupts.cpmRxInterrupts.scExpiryActualTimeout;
      ret = Ra01AccRsMcsCpmRxScExpiryActualTimeout(rmsDev_p, 0, &rxScExpiryActualTimeout, RA01_WRITE_OP);
      DBGRETURN(ret);
   }

   if (clearInterrupts.cpmTxInterrupts.scExpiryPreTimeout) {
      Ra01RsMcsCpmTxScExpiryPreTimeout_t txScExpiryPreTimeout;
      ret = Ra01AccRsMcsCpmTxScExpiryPreTimeout(rmsDev_p, 0, &txScExpiryPreTimeout, RA01_READ_OP);
      DBGRETURN(ret);
      if (clearInterrupts.cpmTxInterrupts.scExpiryPreTimeout == txScExpiryPreTimeout.scExpiryPreTimeoutInt) {
         clearTxScExpiryPreTimeout = true;
      }
      txScExpiryPreTimeout.scExpiryPreTimeoutInt = clearInterrupts.cpmTxInterrupts.scExpiryPreTimeout;
      ret = Ra01AccRsMcsCpmTxScExpiryPreTimeout(rmsDev_p, 0, &txScExpiryPreTimeout, RA01_WRITE_OP);
      DBGRETURN(ret);
   }

   if (clearInterrupts.cpmTxInterrupts.scExpiryActualTimeout) {
      Ra01RsMcsCpmTxScExpiryActualTimeout_t txScExpiryActualTimeout;
      ret = Ra01AccRsMcsCpmTxScExpiryActualTimeout(rmsDev_p, 0, &txScExpiryActualTimeout, RA01_READ_OP);
      DBGRETURN(ret);
      if (clearInterrupts.cpmTxInterrupts.scExpiryActualTimeout == txScExpiryActualTimeout.scExpiryActualTimeoutInt) {
         clearTxScExpiryActualTimeout = true;
      }
      txScExpiryActualTimeout.scExpiryActualTimeoutInt = clearInterrupts.cpmTxInterrupts.scExpiryActualTimeout;
      ret = Ra01AccRsMcsCpmTxScExpiryActualTimeout(rmsDev_p, 0, &txScExpiryActualTimeout, RA01_WRITE_OP);
      DBGRETURN(ret);
   }

   if (clearInterrupts.cpmRxInterrupts.sectagVEq1 ||
       clearInterrupts.cpmRxInterrupts.sectagEEq0CEq1 ||
       clearInterrupts.cpmRxInterrupts.sectagSlGte48 ||
       clearInterrupts.cpmRxInterrupts.sectagEsEq1ScEq1 ||
       clearInterrupts.cpmRxInterrupts.sectagScEq1ScbEq1 ||
       clearInterrupts.cpmRxInterrupts.packetXpnEq0 ||
       clearInterrupts.cpmRxInterrupts.pnThreshReached ||
       clearRxScExpiryPreTimeout ||
       clearRxScExpiryActualTimeout) {
      Ra01RsMcsCpmRxCpmRxInt_t rx_cpm_int_status;
      rx_cpm_int_status.sectagVEq1 = clearInterrupts.cpmRxInterrupts.sectagVEq1;
      rx_cpm_int_status.sectagEEq0CEq1 = clearInterrupts.cpmRxInterrupts.sectagEEq0CEq1;
      rx_cpm_int_status.sectagSlGte48 = clearInterrupts.cpmRxInterrupts.sectagSlGte48;
      rx_cpm_int_status.sectagEsEq1ScEq1 = clearInterrupts.cpmRxInterrupts.sectagEsEq1ScEq1;
      rx_cpm_int_status.sectagScEq1ScbEq1 = clearInterrupts.cpmRxInterrupts.sectagScEq1ScbEq1;
      rx_cpm_int_status.packetXpnEq0 = clearInterrupts.cpmRxInterrupts.packetXpnEq0;
      rx_cpm_int_status.pnThreshReached = clearInterrupts.cpmRxInterrupts.pnThreshReached;
      rx_cpm_int_status.scExpiryPreTimeout = clearRxScExpiryPreTimeout;
      rx_cpm_int_status.scExpiryActualTimeout = clearRxScExpiryActualTimeout;
      ret = Ra01AccRsMcsCpmRxCpmRxInt(rmsDev_p, &(rx_cpm_int_status), RA01_WRITE_OP);
      DBGRETURN(ret);
   }

   if (clearInterrupts.cpmTxInterrupts.packetXpnEq0 ||
       clearInterrupts.cpmTxInterrupts.pnThreshReached ||
       clearInterrupts.cpmTxInterrupts.saNotValid ||
       clearTxScExpiryPreTimeout ||
       clearTxScExpiryActualTimeout) {
      Ra01RsMcsCpmTxCpmTxInt_t tx_cpm_int_status;
      tx_cpm_int_status.packetXpnEq0 = clearInterrupts.cpmTxInterrupts.packetXpnEq0;
      tx_cpm_int_status.pnThreshReached = clearInterrupts.cpmTxInterrupts.pnThreshReached;
      tx_cpm_int_status.saNotValid = clearInterrupts.cpmTxInterrupts.saNotValid;
      tx_cpm_int_status.scExpiryPreTimeout = clearTxScExpiryPreTimeout;
      tx_cpm_int_status.scExpiryActualTimeout = clearTxScExpiryActualTimeout;
      ret = Ra01AccRsMcsCpmTxCpmTxInt(rmsDev_p, &(tx_cpm_int_status), RA01_WRITE_OP);
      DBGRETURN(ret);
   }

   if (clearInterrupts.bbeRxInterrupts.dfifoOverflow || clearInterrupts.bbeRxInterrupts.plfifoOverflow) {
      Ra01RsMcsBbeBbeInt_t rx_bbe_int_status;
      rx_bbe_int_status.dfifoOverflow = clearInterrupts.bbeRxInterrupts.dfifoOverflow;
      rx_bbe_int_status.plfifoOverflow = clearInterrupts.bbeRxInterrupts.plfifoOverflow;
      ret = Ra01AccRsMcsBbeBbeInt(rmsDev_p, RA01_RS_MCS_BBE_RX_SLAVE, &(rx_bbe_int_status), RA01_WRITE_OP);
      DBGRETURN(ret);
   }

   if (clearInterrupts.bbeTxInterrupts.dfifoOverflow || clearInterrupts.bbeTxInterrupts.plfifoOverflow) {
      Ra01RsMcsBbeBbeInt_t tx_bbe_int_status;
      tx_bbe_int_status.dfifoOverflow = clearInterrupts.bbeTxInterrupts.dfifoOverflow;
      tx_bbe_int_status.plfifoOverflow = clearInterrupts.bbeTxInterrupts.plfifoOverflow;
      ret = Ra01AccRsMcsBbeBbeInt(rmsDev_p, RA01_RS_MCS_BBE_TX_SLAVE, &(tx_bbe_int_status), RA01_WRITE_OP);
      DBGRETURN(ret);
   }
 
   if (clearInterrupts.pabRxInterrupts.overflow) {
      Ra01RsMcsPabPabInt_t rx_pab_int_status;
      rx_pab_int_status.overflow = clearInterrupts.pabRxInterrupts.overflow;
      ret = Ra01AccRsMcsPabPabInt(rmsDev_p, RA01_RS_MCS_PAB_RX_SLAVE, &(rx_pab_int_status), RA01_WRITE_OP);
      DBGRETURN(ret);
   }

   if (clearInterrupts.pabTxInterrupts.overflow) {
      Ra01RsMcsPabPabInt_t tx_pab_int_status;
      tx_pab_int_status.overflow = clearInterrupts.pabTxInterrupts.overflow;
      ret = Ra01AccRsMcsPabPabInt(rmsDev_p, RA01_RS_MCS_PAB_TX_SLAVE, &(tx_pab_int_status), RA01_WRITE_OP);
      DBGRETURN(ret);
   }
  
   if (clearInterrupts.pexRxInterrupts.earlyPreemptErr) {
      Ra01RsMcsPexPexInt_t rx_pex_int_status;
      rx_pex_int_status.earlyPreemptErr = clearInterrupts.pexRxInterrupts.earlyPreemptErr;
      ret = Ra01AccRsMcsPexPexInt(rmsDev_p, RA01_RS_MCS_PEX_RX_SLAVE, &(rx_pex_int_status), RA01_WRITE_OP);
      DBGRETURN(ret);
   }

   if (clearInterrupts.pexTxInterrupts.earlyPreemptErr) {
      Ra01RsMcsPexPexInt_t tx_pex_int_status;
      tx_pex_int_status.earlyPreemptErr = clearInterrupts.pexTxInterrupts.earlyPreemptErr;
      ret = Ra01AccRsMcsPexPexInt(rmsDev_p, RA01_RS_MCS_PEX_TX_SLAVE, &(tx_pex_int_status), RA01_WRITE_OP);
      DBGRETURN(ret);
   }
    return ret;
}

int MacsecGetInterruptStatus( RmsDev_t * rmsDev_p, MacsecInterrupts_t * interruptStatus ) {

  int ret = 0;

    if( (rmsDev_p == NULL) || (interruptStatus == NULL) ) {
        return -EINVAL;
    }

 Ra01RsMcsBbeBbeInt_t rx_bbe_int_status;
 ret = Ra01AccRsMcsBbeBbeInt(rmsDev_p, RA01_RS_MCS_BBE_RX_SLAVE, &(rx_bbe_int_status), RA01_READ_OP);
 DBGRETURN(ret);
 interruptStatus->bbeRxInterrupts.dfifoOverflow = rx_bbe_int_status.dfifoOverflow;
 interruptStatus->bbeRxInterrupts.plfifoOverflow = rx_bbe_int_status.plfifoOverflow;

 Ra01RsMcsBbeBbeInt_t tx_bbe_int_status;
 ret = Ra01AccRsMcsBbeBbeInt(rmsDev_p, RA01_RS_MCS_BBE_TX_SLAVE, &(tx_bbe_int_status), RA01_READ_OP);
 DBGRETURN(ret);
 interruptStatus->bbeTxInterrupts.dfifoOverflow = tx_bbe_int_status.dfifoOverflow;
 interruptStatus->bbeTxInterrupts.plfifoOverflow = tx_bbe_int_status.plfifoOverflow;

 Ra01RsMcsPabPabInt_t rx_pab_int_status;
 ret = Ra01AccRsMcsPabPabInt(rmsDev_p, RA01_RS_MCS_PAB_RX_SLAVE, &(rx_pab_int_status), RA01_READ_OP);
 DBGRETURN(ret);
 interruptStatus->pabRxInterrupts.overflow = rx_pab_int_status.overflow;

 Ra01RsMcsPabPabInt_t tx_pab_int_status;
 ret = Ra01AccRsMcsPabPabInt(rmsDev_p, RA01_RS_MCS_PAB_TX_SLAVE, &(tx_pab_int_status), RA01_READ_OP);
 DBGRETURN(ret);
 interruptStatus->pabTxInterrupts.overflow = tx_pab_int_status.overflow;

 Ra01RsMcsPexPexInt_t rx_pex_int_status;
 ret = Ra01AccRsMcsPexPexInt(rmsDev_p, RA01_RS_MCS_PEX_RX_SLAVE, &(rx_pex_int_status), RA01_READ_OP);
 DBGRETURN(ret);
 interruptStatus->pexRxInterrupts.earlyPreemptErr = rx_pex_int_status.earlyPreemptErr;

 Ra01RsMcsPexPexInt_t tx_pex_int_status;
 ret = Ra01AccRsMcsPexPexInt(rmsDev_p, RA01_RS_MCS_PEX_TX_SLAVE, &(tx_pex_int_status), RA01_READ_OP);
 DBGRETURN(ret);
 interruptStatus->pexTxInterrupts.earlyPreemptErr = tx_pex_int_status.earlyPreemptErr;

 Ra01RsMcsCpmRxCpmRxInt_t rx_cpm_int_status;
 ret = Ra01AccRsMcsCpmRxCpmRxInt(rmsDev_p, &(rx_cpm_int_status), RA01_READ_OP);
 DBGRETURN(ret);
 interruptStatus->cpmRxInterrupts.sectagVEq1 = rx_cpm_int_status.sectagVEq1;
 interruptStatus->cpmRxInterrupts.sectagEEq0CEq1 = rx_cpm_int_status.sectagEEq0CEq1;
 interruptStatus->cpmRxInterrupts.sectagSlGte48 = rx_cpm_int_status.sectagSlGte48;
 interruptStatus->cpmRxInterrupts.sectagEsEq1ScEq1 = rx_cpm_int_status.sectagEsEq1ScEq1;
 interruptStatus->cpmRxInterrupts.sectagScEq1ScbEq1 = rx_cpm_int_status.sectagScEq1ScbEq1;
 interruptStatus->cpmRxInterrupts.packetXpnEq0 = rx_cpm_int_status.packetXpnEq0;
 interruptStatus->cpmRxInterrupts.pnThreshReached = rx_cpm_int_status.pnThreshReached;

 Ra01RsMcsCpmRxScExpiryPreTimeout_t rxScExpiryPreTimeout;
 Ra01AccRsMcsCpmRxScExpiryPreTimeout(rmsDev_p, 0, &rxScExpiryPreTimeout, RA01_READ_OP);
 interruptStatus->cpmRxInterrupts.scExpiryPreTimeout = rxScExpiryPreTimeout.scExpiryPreTimeoutInt;
 Ra01RsMcsCpmRxScExpiryActualTimeout_t rxScExpiryActualTimeout;
 Ra01AccRsMcsCpmRxScExpiryActualTimeout(rmsDev_p, 0, &rxScExpiryActualTimeout, RA01_READ_OP);
 interruptStatus->cpmRxInterrupts.scExpiryActualTimeout = rxScExpiryActualTimeout.scExpiryActualTimeoutInt;

 Ra01RsMcsCpmTxCpmTxInt_t tx_cpm_int_status;
 ret = Ra01AccRsMcsCpmTxCpmTxInt(rmsDev_p, &(tx_cpm_int_status), RA01_READ_OP);
 DBGRETURN(ret);
 interruptStatus->cpmTxInterrupts.packetXpnEq0 = tx_cpm_int_status.packetXpnEq0;
 interruptStatus->cpmTxInterrupts.pnThreshReached = tx_cpm_int_status.pnThreshReached;
 interruptStatus->cpmTxInterrupts.saNotValid = tx_cpm_int_status.saNotValid;
 
 Ra01RsMcsCpmTxScExpiryPreTimeout_t txScExpiryPreTimeout;
 Ra01AccRsMcsCpmTxScExpiryPreTimeout(rmsDev_p, 0, &txScExpiryPreTimeout, RA01_READ_OP);
 interruptStatus->cpmTxInterrupts.scExpiryPreTimeout = txScExpiryPreTimeout.scExpiryPreTimeoutInt;
 Ra01RsMcsCpmTxScExpiryActualTimeout_t txScExpiryActualTimeout;
 Ra01AccRsMcsCpmTxScExpiryActualTimeout(rmsDev_p, 0, &txScExpiryActualTimeout, RA01_READ_OP);
 interruptStatus->cpmTxInterrupts.scExpiryActualTimeout = txScExpiryActualTimeout.scExpiryActualTimeoutInt;
    return ret;
}


// -------------------------
// Rekey
// -------------------------

int MacsecGetPacketSeen( RmsDev_t * rmsDev_p, unsigned sc, unsigned an, bool * value) {
   int ret = 0;
   int reg_idx = sc/(sizeof(ra01_register_t)*8);
   int d_idx = sc%(sizeof(ra01_register_t)*8);
   if ((rmsDev_p == NULL) || (sc >= MACSEC_NUM_SC) || (an >= MACSEC_NUM_AN)) {
     return -EINVAL;
   }
   switch(an) {
      case 0: 
         {
            Ra01RsMcsCpmRxPacketSeenRwAn0_t d;
            ret |= Ra01AccRsMcsCpmRxPacketSeenRwAn0(rmsDev_p, reg_idx, &d, RA01_READ_OP);
            *value = (bool)(0x1 & (d.sc >> d_idx));
            break;
         }
      case 1: 
         {
            Ra01RsMcsCpmRxPacketSeenRwAn1_t d;
            ret |= Ra01AccRsMcsCpmRxPacketSeenRwAn1(rmsDev_p, reg_idx, &d, RA01_READ_OP);
            *value = (bool)(0x1 & (d.sc >> d_idx));
            break;
         }
      case 2: 
         {
            Ra01RsMcsCpmRxPacketSeenRwAn2_t d;
            ret |= Ra01AccRsMcsCpmRxPacketSeenRwAn2(rmsDev_p, reg_idx, &d, RA01_READ_OP);
            *value = (bool)(0x1 & (d.sc >> d_idx));
            break;
         }
      case 3: 
         {
            Ra01RsMcsCpmRxPacketSeenRwAn3_t d;
            ret |= Ra01AccRsMcsCpmRxPacketSeenRwAn3(rmsDev_p, reg_idx, &d, RA01_READ_OP);
            *value = (bool)(0x1 & (d.sc >> d_idx));
            break;
         }
      default:
         ret = -EINVAL;
         break;
   }
   return ret;
}

int MacsecClearPacketSeen( RmsDev_t * rmsDev_p, unsigned sc, unsigned an) {
   int ret = 0;
   int reg_idx = sc/(sizeof(ra01_register_t)*8);
   int d_idx = sc%(sizeof(ra01_register_t)*8);
   if ((rmsDev_p == NULL) || (sc >= MACSEC_NUM_SC) || (an >= MACSEC_NUM_AN)) {
     return -EINVAL;
   }
   switch(an) {
      case 0: 
         {
            Ra01RsMcsCpmRxPacketSeenW1cAn0_t d;
            d.sc = (0x1 << d_idx);
            ret |= Ra01AccRsMcsCpmRxPacketSeenW1cAn0(rmsDev_p, reg_idx, &d, RA01_WRITE_OP);
            break;
         }
      case 1: 
         {
            Ra01RsMcsCpmRxPacketSeenW1cAn1_t d;
            d.sc = (0x1 << d_idx);
            ret |= Ra01AccRsMcsCpmRxPacketSeenW1cAn1(rmsDev_p, reg_idx, &d, RA01_WRITE_OP);
            break;
         }
      case 2: 
         {
            Ra01RsMcsCpmRxPacketSeenW1cAn2_t d;
            d.sc = (0x1 << d_idx);
            ret |= Ra01AccRsMcsCpmRxPacketSeenW1cAn2(rmsDev_p, reg_idx, &d, RA01_WRITE_OP);
            break;
         }
      case 3: 
         {
            Ra01RsMcsCpmRxPacketSeenW1cAn3_t d;
            d.sc = (0x1 << d_idx);
            ret |= Ra01AccRsMcsCpmRxPacketSeenW1cAn3(rmsDev_p, reg_idx, &d, RA01_WRITE_OP);
            break;
         }
      default:
         ret = -EINVAL;
         break;
   }
   return ret;
}

int MacsecTxRekeyHandler( RmsDev_t * rmsDev_p, unsigned sc, MacsecTxSaPolicy_t * newPolicy, bool updatePolicy ) {
  int ret = 0;
  bool saActive;
  unsigned saIndex;

  if ((rmsDev_p == NULL) || (newPolicy == NULL) || (sc >= MACSEC_NUM_SC)) {
    return -EINVAL;
  }

  Ra01IfRsMcsCpmTxSaMap_t txSaMap;
  memset(&txSaMap, 0, sizeof(Ra01IfRsMcsCpmTxSaMap_t));
  ret = Ra01ReadRsMcsCpmTxSaMap(rmsDev_p, sc, &txSaMap);
  DBGRETURN(ret);

  if (txSaMap.txSaActive) {
    saActive = 0;
    saIndex = txSaMap.saIndex0;
  } else {
    saActive = 1;
    saIndex = txSaMap.saIndex1;
  }

  // reset pn table for chosen SA
  Ra01IfRsMcsCpmTxSaPnTable_t saPnTable;
  saPnTable.nextPn = 1;
  ret = Ra01WriteRsMcsCpmTxSaPnTable(rmsDev_p, saIndex, &saPnTable);
  DBGRETURN(ret);

  if (updatePolicy && !txSaMap.autoRekeyEnable) {
    // write new SA policy
    ret = Ra01WriteRsMcsCpmTxSaPlcy(rmsDev_p, saIndex, newPolicy);
    DBGRETURN(ret);
  }

  // set valid bit for new SA
  if (saActive) {
    txSaMap.saIndex1Vld = 1;
    txSaMap.saIndex0Vld = txSaMap.autoRekeyEnable;    
  } else {
    txSaMap.saIndex0Vld = 1;
    txSaMap.saIndex1Vld = txSaMap.autoRekeyEnable;
  }

  // switch active SA if auto-rekey disabled
  if (!txSaMap.autoRekeyEnable) {
    txSaMap.txSaActive = saActive;
  }

  ret = Ra01WriteRsMcsCpmTxSaMap(rmsDev_p, sc, &txSaMap);
  DBGRETURN(ret);

  return ret;
}

int MacsecRxRekeyHandler( RmsDev_t * rmsDev_p, unsigned sc, unsigned an) {
  int ret = 0;

  if ((rmsDev_p == NULL) || (sc >= MACSEC_NUM_SC) || (an >= MACSEC_NUM_AN)) {
    return -EINVAL;
  }

  Ra01IfRsMcsCpmRxSaMap_t rxSaMap;
  ret = Ra01ReadRsMcsCpmRxSaMap(rmsDev_p, ((sc * MACSEC_NUM_AN) + an), &rxSaMap);

  // Clear inUse bit for the old AN to reset the PN_REACHED status register
  unsigned saIndex = rxSaMap.saIndex;
  rxSaMap.saInUse = 0;
  ret = Ra01WriteRsMcsCpmRxSaMap(rmsDev_p, ((sc * MACSEC_NUM_AN) + an), &rxSaMap);
  DBGRETURN(ret);

  // reset pn table for chosen SA
  Ra01IfRsMcsCpmRxSaPnTable_t saPnTable;
  saPnTable.nextPn = 1;
  Ra01WriteRsMcsCpmRxSaPnTable(rmsDev_p, saIndex, &saPnTable);

  // Set inUse back to 1 to subsequent rekey
  rxSaMap.saInUse = 1;
  ret = Ra01WriteRsMcsCpmRxSaMap(rmsDev_p, ((sc * MACSEC_NUM_AN) + an), &rxSaMap);
  DBGRETURN(ret);

  // Clear the packet SEEN for the old AN
  ret = MacsecClearPacketSeen(rmsDev_p, sc, an);
  DBGRETURN(ret);

  return ret;
}

int MacsecConfigureScTimer( RmsDev_t * rmsDev_p, MacsecDirection_t direction, MacsecScTimerThresholds_t thresholds, MacsecScTimerTickCycles tick ) {
   int ret = 0;
   if (rmsDev_p == NULL) {
     return -EINVAL;
   }
   if( direction == MACSEC_EGRESS ) {
      Ra01RsMcsCpmTxTimeUnitTick_t ra01Tick;
      ra01Tick.numCycles = tick;
      Ra01AccRsMcsCpmTxTimeUnitTick(rmsDev_p, &ra01Tick, RA01_WRITE_OP);
      for(unsigned i=0; i<4; i++) {
         Ra01RsMcsCpmTxScTimerTimeoutThresh_t d;
         d.threshold = thresholds[i];
         ret |= Ra01AccRsMcsCpmTxScTimerTimeoutThresh(rmsDev_p, i, &d, RA01_WRITE_OP);
      }
   }
   else {
      Ra01RsMcsCpmRxTimeUnitTick_t ra01Tick;
      ra01Tick.numCycles = tick;
      Ra01AccRsMcsCpmRxTimeUnitTick(rmsDev_p, &ra01Tick, RA01_WRITE_OP);
      for(unsigned i=0; i<4; i++) {
         Ra01RsMcsCpmRxScTimerTimeoutThresh_t d;
         d.threshold = thresholds[i];
         ret |= Ra01AccRsMcsCpmRxScTimerTimeoutThresh(rmsDev_p, i, &d, RA01_WRITE_OP);
      }
   }
   return ret;
}

int MacsecPacketToFips(RmsDev_t * rmsDev_p, MacsecFipsOperation_t fipsOperation, MacsecFipsMode_t fipsMode, MacsecFipsSakData_t sakData, volatile MacsecPacket_t packet, bool * resultPass, char * tag) {
   int ret = 0;
   if (rmsDev_p == NULL || resultPass == NULL || tag == NULL || (packet.pre_st_aad_start > packet.pre_st_aad_end)) {
     return -EINVAL;
   }

   Ra01RsMcsGaeInstance_t instance;
   instance = (fipsOperation == MACSEC_FIPS_ENCRYPT) ? RA01_RS_MCS_GAE_TX_SLAVE : RA01_RS_MCS_GAE_RX_SLAVE;

   ret = FipsReset(rmsDev_p, instance);
   DBGRETURN(ret);
   if (ret == -EAGAIN) {
      return ret;
   }

   // policy data
   ret = FipsPolicy(rmsDev_p, fipsMode, sakData, instance);
   DBGRETURN(ret);
   
   // AAD
   int text_size = (int)packet.size - (int)packet.aad_size - (int)packet.aad_start_offset;
   int fips_block_size_in_bytes = MACSEC_FIPS_BLOCK_SIZE / 8;
   int pre_st_aad_size = (packet.pre_st_aad_end - packet.pre_st_aad_start);
   int total_aad_size = pre_st_aad_size + (int)packet.aad_size;
   uint32_t ctr = sakData.initCtr;

   if ((fipsMode.ecbMode == MACSEC_FIPS_AES_ECB_DISABLED) && (fipsMode.ctrMode == MACSEC_FIPS_CTR_AUTO)) {
      int num_full_aad_blocks = total_aad_size / fips_block_size_in_bytes;

      if (total_aad_size == 0 && text_size != 0) {
         ret = FipsCtl(rmsDev_p, instance, 0, 1, 0, 0, 0, FipsNextIcv(fipsOperation, total_aad_size, text_size, 1, 1));
         DBGRETURN(ret);
         ret = FipsStart(rmsDev_p, instance);
         DBGRETURN(ret);
         if (ret == -EAGAIN) {
            return ret;
         }
      } else {
         bool has_partial_block = (total_aad_size % fips_block_size_in_bytes) != 0;
         for (int ii = 0; ii < (num_full_aad_blocks + (has_partial_block ? 1 : 0)); ii++) {
            uint64_t block_lsb = 0, block_msb = 0, block_mask = 0, next_byte_64b = 0;
            int block_size_in_bytes, byte_offset;

            if (ii == num_full_aad_blocks) {
               block_size_in_bytes = (total_aad_size % fips_block_size_in_bytes);
            } else {
               block_size_in_bytes = fips_block_size_in_bytes;
            }

            // copy bytes into block
            for (int jj = 0; jj < block_size_in_bytes; jj++) {
               char next_byte;
               byte_offset = ii * fips_block_size_in_bytes + jj;
               
               if (byte_offset < pre_st_aad_size) {
                  next_byte = packet.data[packet.pre_st_aad_start + byte_offset];
               } else {
                  next_byte = packet.data[packet.aad_start_offset + (byte_offset - pre_st_aad_size)];
               }
               if (jj < 8) {
                  block_mask = 255ULL << ((7 - jj) * 8);
                  next_byte_64b = next_byte & 255ULL;
                  block_msb = ((next_byte_64b << ((7 - jj) * 8)) & block_mask) | (block_msb & ~block_mask);
               } else {
                  block_mask = 255ULL << ((7 - (jj - 8)) * 8);
                  next_byte_64b = next_byte & 255ULL;
                  block_lsb = ((next_byte_64b << ((7 - (jj - 8)) * 8)) & block_mask) | (block_lsb & ~block_mask);
               }
            }
            
            ret = FipsWriteBlock(rmsDev_p, instance, block_msb, block_lsb);
            DBGRETURN(ret);
            bool is_last_round = (ii == num_full_aad_blocks) || ((ii == num_full_aad_blocks - 1) && !has_partial_block);
            bool is_eop = (fipsOperation == MACSEC_FIPS_ENCRYPT) && is_last_round;
            ret = FipsCtl(rmsDev_p, instance, 1, 1, is_eop, ((block_size_in_bytes == fips_block_size_in_bytes) ? 0 : block_size_in_bytes), 0, FipsNextIcv(fipsOperation, total_aad_size, text_size, is_last_round, 1));
            DBGRETURN(ret);
            ret = FipsStart(rmsDev_p, instance);
            DBGRETURN(ret);
            if (ret == -EAGAIN) {
               return ret;
            }           
         }
      }
   }

   // Text blocks
   int text_offset = packet.aad_start_offset + packet.aad_size;
   int num_full_text_blocks = text_size / fips_block_size_in_bytes;
   bool has_partial_block = (text_size % fips_block_size_in_bytes) != 0;
   for (int ii = 0; ii < (num_full_text_blocks + (has_partial_block ? 1 : 0)); ii++) {
      uint64_t block_lsb = 0, block_msb = 0, block_mask = 0;
      int block_size_in_bytes, byte_offset;

      if (ii == num_full_text_blocks) {
         block_size_in_bytes = (text_size % fips_block_size_in_bytes);
      } else {
         block_size_in_bytes = fips_block_size_in_bytes;
      }

      // copy bytes into block
      for (int jj = 0; jj < block_size_in_bytes; jj++) {
         char next_byte;
         uint64_t next_byte_64b;
         byte_offset = ii * fips_block_size_in_bytes + jj;
         
         next_byte = packet.data[text_offset + byte_offset];
         next_byte_64b = next_byte & 255ULL;

         if (jj < 8) {
            block_mask = 255ULL << ((7 - jj) * 8);
            block_msb = ((next_byte_64b << ((7 - jj) * 8)) & block_mask) | (block_msb & ~block_mask);
         } else {
            block_mask = 255ULL << ((7 - (jj - 8)) * 8);
            block_lsb = ((next_byte_64b << ((7 - (jj - 8)) * 8)) & block_mask) | (block_lsb & ~block_mask);
         }
      }
      
      if (fipsMode.ctrMode == MACSEC_FIPS_CTR_MANUAL) {
         Ra01RsMcsGaeFipsCtr_t fipsCtr;
         fipsCtr.fipsCtr = ctr++;
         ret = Ra01AccRsMcsGaeFipsCtr(rmsDev_p, instance, &fipsCtr, RA01_WRITE_OP);
      }

      ret = FipsWriteBlock(rmsDev_p, instance, block_msb, block_lsb);
      DBGRETURN(ret);

      bool is_last_round = (ii == num_full_text_blocks) || ((ii == num_full_text_blocks - 1) && !has_partial_block);
      bool is_eop = (fipsOperation == MACSEC_FIPS_ENCRYPT) && is_last_round;

      if (fipsMode.ecbMode == MACSEC_FIPS_AES_ECB_DISABLED) {
         ret = FipsCtl(rmsDev_p, instance, 1, 0, is_eop, ((block_size_in_bytes == fips_block_size_in_bytes) ? 0 : block_size_in_bytes), 0, FipsNextIcv(fipsOperation, total_aad_size, text_size, is_last_round, 0));
         DBGRETURN(ret);
      } else {
         ret = FipsCtl(rmsDev_p, instance, 0, 1, 0, 0, 0, 0);
      }

      ret = FipsStart(rmsDev_p, instance);
      DBGRETURN(ret);

      if (ret == -EAGAIN) {
         return ret;
      }           

      ret = FipsReadBlock(rmsDev_p, instance, &block_msb, &block_lsb);
      DBGRETURN(ret);
 
      // copy block into bytes
      for (int jj = 0; jj < block_size_in_bytes; jj++) {
         char next_byte;
         byte_offset = ii * fips_block_size_in_bytes + jj;
         
         if (jj < 8) {
            next_byte = (block_msb >> ((7 - jj) * 8)) & 0xff;
         } else {
            next_byte = (block_lsb >> ((7 - (jj - 8)) * 8)) & 0xff;
         }

         packet.data[text_offset + byte_offset] = next_byte;
      }

      if ((fipsMode.ecbMode == MACSEC_FIPS_AES_ECB_ENABLED) && !is_last_round) {
         ret = FipsReset(rmsDev_p, instance);
         DBGRETURN(ret);
         if (ret == -EAGAIN) {
            return ret;
         }
      }
   }

   // fips end
   if ((fipsMode.ecbMode == MACSEC_FIPS_AES_ECB_DISABLED) && (fipsMode.ctrMode == MACSEC_FIPS_CTR_AUTO)) {
      if (fipsOperation == MACSEC_FIPS_DECRYPT) {
         if (total_aad_size == 0 && text_size == 0) {
            ret = FipsCtl(rmsDev_p, instance, 0, 1, 0, 0, 0, FipsNextIcv(fipsOperation, total_aad_size, text_size, 1, 0));
            DBGRETURN(ret);
            ret = FipsStart(rmsDev_p, instance);
            DBGRETURN(ret);
            if (ret == -EAGAIN) {
               return ret;
            }           
         }
         // write icv block
         uint64_t block_msb = 0, block_lsb = 0, block_mask = 0;
         uint64_t next_byte_64b = 0;
         for (int ii = 0; ii < 8; ii++) {
            block_mask = 255ULL << ((7 - ii) * 8);
            next_byte_64b = tag[ii] & 255ULL;
            block_msb = ((next_byte_64b << ((7 - ii) * 8)) & block_mask) | (block_msb & ~block_mask); 
         }
         for (int ii = 0; ii < 8; ii++) {
            block_mask = 255ULL << ((7 - ii) * 8);
            next_byte_64b = tag[ii + 8] & 255ULL;
            block_lsb = ((next_byte_64b << ((7 - ii) * 8)) & block_mask) | (block_lsb & ~block_mask); 
         }
         ret = FipsWriteBlock(rmsDev_p, instance, block_msb, block_lsb);
         DBGRETURN(ret);
         ret = FipsCtl(rmsDev_p, instance, 1, 0, 1, 0, 0, 0);
         DBGRETURN(ret);
         ret = FipsStart(rmsDev_p, instance);
         DBGRETURN(ret);
         if (ret == -EAGAIN) {
            return ret;
         }           
         Ra01RsMcsGaeFipsResultPass_t fipsResultPass;
         ret = Ra01GetRsMcsGaeFipsResultPass(rmsDev_p, instance, &fipsResultPass);
         DBGRETURN(ret);
         *resultPass = fipsResultPass.fipsResultPass;
      } else {
         if (total_aad_size == 0 && text_size == 0) {
            ret = FipsCtl(rmsDev_p, instance, 0, 1, 1, 0, 0, 0);
            DBGRETURN(ret);
            ret = FipsStart(rmsDev_p, instance);
            DBGRETURN(ret);
            if (ret == -EAGAIN) {
               return ret;
            }           
         }
         
         ret = FipsCtl(rmsDev_p, instance, 0, 1, 0, 0, 1, 0);
         DBGRETURN(ret);
         ret = FipsStart(rmsDev_p, instance);
         DBGRETURN(ret);
         if (ret == -EAGAIN) {
            return ret;
         }           

         // read icv registers
         uint64_t icv_msb, icv_lsb;
         Ra01RsMcsGaeFipsResultIcvBits12764_t fipsResultIcvMsb;
         ret = Ra01GetRsMcsGaeFipsResultIcvBits12764(rmsDev_p, instance, &fipsResultIcvMsb);
         DBGRETURN(ret);

         Ra01RsMcsGaeFipsResultIcvBits630_t fipsResultIcvLsb;
         ret = Ra01GetRsMcsGaeFipsResultIcvBits630(rmsDev_p, instance, &fipsResultIcvLsb);
         DBGRETURN(ret);

         icv_msb = (uint64_t)fipsResultIcvMsb.dataMsb;
         icv_msb = ((icv_msb << 32) & 0xffffffff00000000) | (fipsResultIcvMsb.dataLsb & 0x00000000ffffffff);
         icv_lsb = (uint64_t)fipsResultIcvLsb.dataMsb;
         icv_lsb = ((icv_lsb << 32) & 0xffffffff00000000) | (fipsResultIcvLsb.dataLsb & 0x00000000ffffffff);
         
         for (int ii = 0; ii < 8; ii++) {
            tag[ii] = (icv_msb >> ((7 - ii) * 8)) & 0xff;
         }

         for (int ii = 0; ii < 8; ii++) {
            tag[ii + 8] = (icv_lsb >> ((7 - ii) * 8)) & 0xff;
         }
      }
   }

   return ret;
}

int FipsPolicy(RmsDev_t * rmsDev_p, MacsecFipsMode_t fipsMode, MacsecFipsSakData_t sakData, Ra01RsMcsGaeInstance_t instance) {
   int ret = 0;
   Ra01RsMcsGaeFipsMode_t gaeFipsMode;
   gaeFipsMode.ctrMode = fipsMode.ctrMode == MACSEC_FIPS_CTR_MANUAL;
   gaeFipsMode.keylen = fipsMode.keyLen;
   gaeFipsMode.channel = 2;
   gaeFipsMode.ecb = fipsMode.ecbMode;
   ret = Ra01AccRsMcsGaeFipsMode( rmsDev_p, instance, &gaeFipsMode, RA01_WRITE_OP);
   DBGRETURN(ret);

   if (fipsMode.keyLen == MACSEC_FIPS_KEY_LEN_256) {
      Ra01RsMcsGaeFipsSakBits255192_t sakBits255_192;
      sakBits255_192.dataMsb = (uint32_t)((sakData.sakBits255_192 & 0xffffffff00000000) >> 32);
      sakBits255_192.dataLsb = sakData.sakBits255_192 & 0x00000000ffffffff;
      Ra01AccRsMcsGaeFipsSakBits255192(rmsDev_p, instance, &sakBits255_192, RA01_WRITE_OP);
      DBGRETURN(ret);

      Ra01RsMcsGaeFipsSakBits191128_t sakBits191_128;
      sakBits191_128.dataMsb = (uint32_t)((sakData.sakBits191_128 & 0xffffffff00000000) >> 32);
      sakBits191_128.dataLsb = sakData.sakBits191_128 & 0x00000000ffffffff;
      Ra01AccRsMcsGaeFipsSakBits191128(rmsDev_p, instance, &sakBits191_128, RA01_WRITE_OP);
      DBGRETURN(ret);
   }

   Ra01RsMcsGaeFipsSakBits12764_t sakBits127_64;
   sakBits127_64.dataMsb = (uint32_t)((sakData.sakBits127_64 & 0xffffffff00000000) >> 32);
   sakBits127_64.dataLsb = sakData.sakBits127_64 & 0x00000000ffffffff;
   Ra01AccRsMcsGaeFipsSakBits12764(rmsDev_p, instance, &sakBits127_64, RA01_WRITE_OP);
   DBGRETURN(ret);

   Ra01RsMcsGaeFipsSakBits630_t sakBits63_0;
   sakBits63_0.dataMsb = (uint32_t)((sakData.sakBits63_0 & 0xffffffff00000000) >> 32);
   sakBits63_0.dataLsb = sakData.sakBits63_0 & 0x00000000ffffffff;
   Ra01AccRsMcsGaeFipsSakBits630(rmsDev_p, instance, &sakBits63_0, RA01_WRITE_OP);
   DBGRETURN(ret);

   Ra01RsMcsGaeFipsHashkeyBits12764_t hashkeyBits127_64;
   hashkeyBits127_64.dataMsb = (uint32_t)((sakData.hashkeyBits127_64 & 0xffffffff00000000) >> 32);
   hashkeyBits127_64.dataLsb = sakData.hashkeyBits127_64 & 0x00000000ffffffff;
   Ra01AccRsMcsGaeFipsHashkeyBits12764(rmsDev_p, instance, &hashkeyBits127_64, RA01_WRITE_OP);
   DBGRETURN(ret);

   Ra01RsMcsGaeFipsHashkeyBits630_t hashkeyBits63_0;
   hashkeyBits63_0.dataMsb = (uint32_t)((sakData.hashkeyBits63_0 & 0xffffffff00000000) >> 32);
   hashkeyBits63_0.dataLsb = sakData.hashkeyBits63_0 & 0x00000000ffffffff;
   Ra01AccRsMcsGaeFipsHashkeyBits630(rmsDev_p, instance, &hashkeyBits63_0, RA01_WRITE_OP);
   DBGRETURN(ret);

   if (fipsMode.ecbMode == MACSEC_FIPS_AES_ECB_DISABLED) {
      Ra01RsMcsGaeFipsIvBits9564_t ivBits95_64;
      ivBits95_64.data = sakData.ivBits95_64;
      Ra01AccRsMcsGaeFipsIvBits9564(rmsDev_p, instance, &ivBits95_64, RA01_WRITE_OP);
      DBGRETURN(ret);

      Ra01RsMcsGaeFipsIvBits630_t ivBits63_0;
      ivBits63_0.dataMsb = (uint32_t)((sakData.ivBits63_0 & 0xffffffff00000000) >> 32);
      ivBits63_0.dataLsb = sakData.ivBits63_0 & 0x00000000ffffffff;
      Ra01AccRsMcsGaeFipsIvBits630(rmsDev_p, instance, &ivBits63_0, RA01_WRITE_OP);
      DBGRETURN(ret);
   }

   return ret;
}

int FipsStart(RmsDev_t * rmsDev_p, Ra01RsMcsGaeInstance_t instance) {
   int ret = 0;
   int readCount = 0;

   Ra01RsMcsGaeFipsStart_t fipsStart;
   fipsStart.fipsStart = 1;
   ret = Ra01AccRsMcsGaeFipsStart( rmsDev_p, instance, &fipsStart, RA01_WRITE_OP);
   DBGRETURN(ret);

   // spin wait until fipsStart goes low again
   bool waiting = 1;
   readCount = 0;
   while (waiting) {
      ret = Ra01AccRsMcsGaeFipsStart( rmsDev_p, instance, &fipsStart, RA01_READ_OP);
      DBGRETURN(ret);
      waiting = fipsStart.fipsStart & 0x1;
      readCount++;
      if (readCount == 1000) {
         return -EAGAIN;
      }
   }
   return ret;
}

int FipsReset(RmsDev_t * rmsDev_p, Ra01RsMcsGaeInstance_t instance) {
   int ret = 0;
   int readCount = 0;

   Ra01RsMcsGaeFipsReset_t fipsReset;
   fipsReset.fipsReset = 1;
   ret = Ra01AccRsMcsGaeFipsReset( rmsDev_p, instance, &fipsReset, RA01_WRITE_OP);
   DBGRETURN(ret);

   // spin wait until fipsReset goes low again
   bool waiting = 1;
   readCount = 0;
   while (waiting) {
      ret = Ra01AccRsMcsGaeFipsReset( rmsDev_p, instance, &fipsReset, RA01_READ_OP);
      DBGRETURN(ret);
      waiting = fipsReset.fipsReset & 0x1;
      readCount++;
      if (readCount == 1000) {
         return -EAGAIN;
      }
   }
   return ret;
}

int FipsCtl(RmsDev_t * rmsDev_p, Ra01RsMcsGaeInstance_t instance,
            bool valid,
            bool isAad,
            bool eop,
            uint8_t blockSize,
            bool lenRound,
            bool nextIcv) {
   int ret = 0;
   Ra01RsMcsGaeFipsCtl_t fipsCtl;
   fipsCtl.vld = valid;
   fipsCtl.blockIsAad = isAad;
   fipsCtl.eop = eop;
   fipsCtl.blockSize = blockSize;
   fipsCtl.lenRound = lenRound;
   fipsCtl.nextIcv = nextIcv;
   ret = Ra01AccRsMcsGaeFipsCtl(rmsDev_p, instance, &fipsCtl, RA01_WRITE_OP);
   DBGRETURN(ret);
   return ret;
}

bool FipsNextIcv(MacsecFipsOperation_t fipsOperation, int aadSize, int textSize, bool isLastRound, int isAad) {
   return (fipsOperation == MACSEC_FIPS_DECRYPT && 
            ((aadSize == 0 && textSize == 0) ||
             (isAad && isLastRound && textSize == 0) ||
             (!isAad && isLastRound)));
}

int FipsWriteBlock(RmsDev_t * rmsDev_p, Ra01RsMcsGaeInstance_t instance, uint64_t blockMsb, uint64_t blockLsb) {
   int ret = 0;
   Ra01RsMcsGaeFipsBlockBits12764_t fipsBlockMsb;
   
   fipsBlockMsb.dataLsb = blockMsb & 0x00000000ffffffff;
   blockMsb = blockMsb >> 32;
   fipsBlockMsb.dataMsb = blockMsb & 0x00000000ffffffff;
   ret = Ra01AccRsMcsGaeFipsBlockBits12764(rmsDev_p, instance, &fipsBlockMsb, RA01_WRITE_OP);
   DBGRETURN(ret);

   Ra01RsMcsGaeFipsBlockBits630_t fipsBlockLsb;
   fipsBlockLsb.dataLsb = blockLsb & 0x00000000ffffffff;
   blockLsb = blockLsb >> 32;
   fipsBlockLsb.dataMsb = blockLsb & 0x00000000ffffffff;
   ret = Ra01AccRsMcsGaeFipsBlockBits630(rmsDev_p, instance, &fipsBlockLsb, RA01_WRITE_OP);
   DBGRETURN(ret);
   return ret;
}

int FipsReadBlock(RmsDev_t * rmsDev_p, Ra01RsMcsGaeInstance_t instance, uint64_t * blockMsb, uint64_t * blockLsb) {
   int ret = 0;
   Ra01RsMcsGaeFipsResultBlockBits12764_t fipsBlockMsb;
   ret = Ra01GetRsMcsGaeFipsResultBlockBits12764(rmsDev_p, instance, &fipsBlockMsb);
   *blockMsb = (uint64_t)fipsBlockMsb.dataMsb;
   *blockMsb = ((*blockMsb << 32) & 0xffffffff00000000) | (fipsBlockMsb.dataLsb & 0x00000000ffffffff);
   DBGRETURN(ret);

   Ra01RsMcsGaeFipsResultBlockBits630_t fipsBlockLsb;
   ret = Ra01GetRsMcsGaeFipsResultBlockBits630(rmsDev_p, instance, &fipsBlockLsb);
   *blockLsb = (uint64_t)fipsBlockLsb.dataMsb;
   *blockLsb = ((*blockLsb << 32) & 0xffffffff00000000) | (fipsBlockLsb.dataLsb & 0x00000000ffffffff);
    DBGRETURN(ret);
   return ret;
}

