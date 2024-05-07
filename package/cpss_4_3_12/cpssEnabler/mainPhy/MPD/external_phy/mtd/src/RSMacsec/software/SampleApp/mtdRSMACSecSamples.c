/*******************************************************************************
Copyright (C) 2014 - 2022, Marvell International Ltd. and its affiliates
If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.
*******************************************************************************/

/********************************************************************
This file contains MACSec sample functions use in the Marvell
CUX3610/CUE2610 MACSec enable Ethernet PHYs.
********************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "mtdFeatures.h"
#include "mtdApiTypes.h"
#include "mtdAPI.h"
#include "mtdIntr.h"
#include "macsec_api.h"
#include "macsec_ieee_api.h"
#include "mtdRSMACSecHwCntl.h"
#include "mtdRSMACSecSamples.h"
#include "mtdRSMACSecTypes.h"
#include "mtdRSMACSecAPI.h"


MTD_STATUS mtdRSMACSecSampleConfigMACSec
(
    IN MTD_DEV_PTR pDev,
    IN MTD_U16 mdioPort,
    IN MTD_RSMACS_SAMPLE_OPTION sampleOptions
)
{
    RmsDev_t *mcsDev_p;
    MTD_U8 major, minor, buildID;

    mtdRSMACSecGetAPIVersion(&major, &minor, &buildID);
    MTD_DBG_INFO("MACSec SDK version: %u.%u.%u\n", major, minor, buildID);

    mcsDev_p = &pDev->rsMACSecCntl.rmsDev;

    /* (Flow_ID → SecY → SC → SA) */ 

    /* config and enable device MACSec */ 
    MTD_ATTEMPT(mtdRSMACSecConfigEnable(pDev, mdioPort, 0));

    /* config port (MacsecSetConfiguration) */ 
    MTD_ATTEMPT(mtdRSMACSecSampleConfigPort(pDev, mdioPort, sampleOptions));

    /* config control packet filter (MacsecSetPacketFilter) */ 
    MTD_ATTEMPT(mtdRSMACSecSampleSetParserControlPkt(pDev, mdioPort, sampleOptions));

    /* config rules (TCAM flow) and mapped SecY (MacsecWriteTcam/MacsecEnableTcam) */ 
    /* config SecY and assigned policy to SecY (MacsecWriteSecyPolicy) */ 
    /* config and enable SC with assigned SCI and SecY (6B MAC/2B Port) (MacsecSetIngressScCamSci) */ 
    MTD_ATTEMPT(mtdRSMACSecSampleEnableRulesSetSC(pDev, mdioPort, sampleOptions));

    /* config and set SAs (MacsecSetSaPolicy, MacsecEnableIngressSa, MacsecEnableEgressSa) */ 
    /* mapped sci to SAs (MacsecSetEgressSaMap) */ 
    MTD_ATTEMPT(mtdRSMACSecSampleSetSA(pDev, mdioPort, sampleOptions));

    return MTD_OK;

}

MTD_STATUS mtdRSMACSecSampleConfigPort
(
    IN MTD_DEV_PTR pDev,
    IN MTD_U16 mdioPort,
    IN MTD_RSMACS_SAMPLE_OPTION sampleOptions
)
{
    RmsDev_t *mcsDev_p;

    mcsDev_p = &pDev->rsMACSecCntl.rmsDev;

    /*3.1 Port Mode*/ 
    MacsecConfig_t portCfg;
    memset(&portCfg, 0, sizeof(MacsecConfig_t));
    MTD_ATTEMPT(MacsecGetConfiguration(mcsDev_p, &portCfg));

    portCfg.portMode = MACSEC_PORT_MODE_1X10;

    portCfg.ingress.valid = true;
    portCfg.ingress.chBypass[0] = false; /*Prempt Channel*/ 
    portCfg.ingress.chBypass[1] = false; /*Express Channel*/ 

    portCfg.egress.valid = true;
    portCfg.egress.chBypass[0] = false; /*Prempt Channel*/ 
    portCfg.egress.chBypass[1] = false; /*Express Channel*/ 

    portCfg.ingress.statsClearOnRead = true;
    portCfg.egress.statsClearOnRead = true;

    if (sampleOptions & MTD_RSMACS_SAMPLE_XPN_PN_THRESHOLD)
    {
        MTD_DBG_INFO("mtdRSMACSecSampleConfigPort: MTD_RSMACS_SAMPLE_XPN_PN_THRESHOLD option\n");
        if (sampleOptions & MTD_RSMACS_SAMPLE_256_XPN)
        {
            portCfg.ingress.xpnThreshold = 0x00FFFFFF; /* Re-key to new SA after 0x00FFFFFF ingress extended packet number */ 
            portCfg.egress.xpnThreshold = 0x00FFFFFF;  /* Re-key to new SA after 0x00FFFFFF egress extended packet number */ 
        }
        else
        {
            portCfg.ingress.pnThreshold = 0x00FFFFFF; /* Re-key to new SA after 0x00FFFFFF ingress packet number */ 
            portCfg.egress.pnThreshold = 0x00FFFFFF;  /* Re-key to new SA after 0x00FFFFFF egress packet number */ 
        }
    }

    if (sampleOptions & MTD_RSMACS_SAMPLE_INTERRUPTS)
    {
        MTD_DBG_INFO("mtdRSMACSecSampleConfigPort: MTD_RSMACS_SAMPLE_INTERRUPTS option\n");

        /* mtdRSMACInterruptEnable enables PHY device top level interrupt and MACSec block 
           MTD_PTP_MPM_INTR interrupt; this will propagate the MACSec interrupt to
           the top level */
        MTD_ATTEMPT(mtdRSMACInterruptEnable(pDev, mdioPort, MTD_TRUE));
    }

    MTD_ATTEMPT(MacsecSetConfiguration(mcsDev_p, &portCfg));

    return MTD_OK;
}

MTD_STATUS mtdRSMACSecSampleSetParserControlPkt
(
    IN MTD_DEV_PTR pDev,
    IN MTD_U16 mdioPort,
    IN MTD_RSMACS_SAMPLE_OPTION sampleOptions
)
{
    RmsDev_t *mcsDev_p;

    mcsDev_p = &pDev->rsMACSecCntl.rmsDev;

    /* 3.2 Parser Configuration */ 
    /*
    * Ingress
    */
    MacsecPacketFilter_t ingressFilter;

    /* Filter configuration */ 
    memset(&ingressFilter, 0, sizeof(MacsecPacketFilter_t));

    if (sampleOptions & MTD_RSMACS_SAMPLE_CONTROL_PKTS)
    {
        MTD_DBG_INFO("mtdRSMACSecSampleSetParserControlPkt: MTD_RSMACS_SAMPLE_CONTROL_PKTS option\n");
        /* Use of the PEX control packet classification function eliminates the need
        for a dedicated TCAM entry defining a control packet. */
        ingressFilter.ctrl_rules.etype[0].enInner = 1;
        ingressFilter.ctrl_rules.etype[0].enOuter = 1;
        ingressFilter.ctrl_rules.etype[0].etype = 0x88F7;
    }

    ingressFilter.sectag.enable = true;
    ingressFilter.sectag.etype = 0x88e5;

    MTD_ATTEMPT(MacsecSetPacketFilter(mcsDev_p,
        MACSEC_INGRESS,
        &ingressFilter));

    /*
    * Egress
    */
    MacsecPacketFilter_t egressFilter;

    /* Filter configuration */ 
    memset(&egressFilter, 0, sizeof(MacsecPacketFilter_t));

    if (sampleOptions & MTD_RSMACS_SAMPLE_CONTROL_PKTS)
    {
        egressFilter.ctrl_rules.etype[0].enInner = 1;
        egressFilter.ctrl_rules.etype[0].enOuter = 1;
        egressFilter.ctrl_rules.etype[0].etype = 0x88F7;
    }

    MTD_ATTEMPT(MacsecSetPacketFilter(mcsDev_p,
        MACSEC_EGRESS,
        &egressFilter));

    return MTD_OK;
}

MTD_STATUS mtdRSMACSecSampleEnableRulesSetSC
(
    IN MTD_DEV_PTR pDev,
    IN MTD_U16 mdioPort,
    IN MTD_RSMACS_SAMPLE_OPTION sampleOptions
)
{
    RmsDev_t *mcsDev_p;

    mcsDev_p = &pDev->rsMACSecCntl.rmsDev;
    /*
    * Ingress
    */

    /*
    * SecY
    */
    MacsecFlowCfg_t          ingressTcam;
    MacsecSecyPolicyTable_t     ingressPolicy;

    memset(&ingressTcam, 0, sizeof(MacsecFlowCfg_t));

    /* RS_MCS_CPM_RX_SLAVE.SECY_MAP_MEM[0] */ 
    ingressTcam.controlPacket = false;
    ingressTcam.secY = secY;

    /* Data values */ 
    /* RS_CMCS_CCPM_RX_SLAVE.FLOWID_TCAM_DATA[0] */ 
    ingressTcam.d.ingress.data.express = true;
    

    ingressTcam.d.ingress.data.port = 0;
    ingressTcam.d.ingress.data.numTags = 1;
    ingressTcam.d.ingress.data.packetType = 0;
    ingressTcam.d.ingress.data.macDa = 0x0000979d22c7;
    ingressTcam.d.ingress.data.macSa = 0x0000deadbeef;
    ingressTcam.d.ingress.data.etherType = 0x88E5;

    /* Data mask */ 
    /* RS_CMCS_CCPM_RX_SLAVE.FLOWID_TCAM_MASK[0] */ 
    ingressTcam.d.ingress.mask.express = 1;
    ingressTcam.d.ingress.mask.port = 0;
    ingressTcam.d.ingress.mask.numTags = 0;
    ingressTcam.d.ingress.mask.packetType = 0;
    ingressTcam.d.ingress.mask.bonusData = 0xFFFF;
    ingressTcam.d.ingress.mask.etherType = 0; /* 0xFFFF; */ 

    if (sampleOptions & MTD_RSMACS_SAMPLE_DEFAULT_MATCH)
    {
        MTD_DBG_INFO("mtdRSMACSecSampleEnableRulesSetSC: MTD_RSMACS_SAMPLE_DEFAULT_MATCH option\n");
        ingressTcam.d.ingress.mask.macDa = 0xffffffffffff; /* default match MAC (Wildcard for all MAC)*/
        ingressTcam.d.ingress.mask.macSa = 0xffffffffffff;
    }
    else
    {
        ingressTcam.d.ingress.mask.macDa = 0x000000000000; /* exact match MAC address */
        ingressTcam.d.ingress.mask.macSa = 0x000000000000;
    }

    MTD_ATTEMPT(MacsecWriteTcam(mcsDev_p,
        MACSEC_INGRESS,
        ingressFlowId,
        &ingressTcam));

    MTD_ATTEMPT(MacsecEnableTcam(mcsDev_p,
        MACSEC_INGRESS,
        ingressFlowId,
        true)); /* enable */ 

    if (sampleOptions & MTD_RSMACS_SAMPLE_CONTROL_PKTS)
    {
        /* add another TCAM for allowing control packet to hit the TCAM */
        ingressTcam.d.ingress.data.etherType = (MTD_U16)0x88F7;

        MTD_ATTEMPT(MacsecWriteTcam(mcsDev_p,
            MACSEC_INGRESS,
            ingressFlowId + 1,
            &ingressTcam));

        MTD_ATTEMPT(MacsecEnableTcam(mcsDev_p,
            MACSEC_INGRESS,
            ingressFlowId + 1,
            true)); /* enable */ 
    }

    memset(&ingressPolicy, 0, sizeof(MacsecSecyPolicyTable_t));

    /* RS_CMCS_CCPM_RX_SLAVE.SECY_PLCY_MEM[0] */ 
    ingressPolicy.ingress.controlledPortEnabled = 1;
    ingressPolicy.ingress.validateFrames = 2;
    ingressPolicy.ingress.stripSectagIcv = 0;
    if (sampleOptions & MTD_RSMACS_SAMPLE_256_XPN)
    {
        MTD_DBG_INFO("mtdRSMACSecSampleEnableRulesSetSC: MTD_RSMACS_SAMPLE_256_XPN option\n");
        /* cipher options:0 = GCM-AES-128
        1 = GCM-AES-256
        2 = GCM-AES-XPN-128
        3 = GCM-AES-XPN-256 */
        ingressPolicy.ingress.cipher = 3;
    }
    else
    {
        ingressPolicy.ingress.cipher = 0;
    }

    ingressPolicy.ingress.confidentialityOffset = 0;
    ingressPolicy.ingress.preSectagAuthEnable = 0;
    ingressPolicy.ingress.replayProtect = 1;
    ingressPolicy.ingress.replayWindow = 10;

    MTD_ATTEMPT(MacsecWriteSecyPolicy(mcsDev_p,
        MACSEC_INGRESS,
        secY,
        &ingressPolicy));

    /*
    * SC
    */
    MTD_ATTEMPT(MacsecSetIngressScCamSci(mcsDev_p,
        sc,
        secY,
        sci,
        true)); /* enable */ 

    /*
    * Egress
    */

    /*
    * SecY
    */
    MacsecFlowCfg_t          egressTcam;
    MacsecSecyPolicyTable_t     egressPolicy;

    memset(&egressTcam, 0, sizeof(MacsecFlowCfg_t));

    /* RS_MCS_CPM_TX_SLAVE.SECY_MAP_MEM[0] */ 
    egressTcam.controlPacket = false;
    egressTcam.secY = secY;
    egressTcam.d.egress.sc = 0;
    egressTcam.d.egress.sci = 0x0000000000000000;
    egressTcam.d.egress.auxiliaryPolicy = false;

    /* Data values */ 
    /* RS_CMCS_CCPM_TX_SLAVE.FLOWID_TCAM_DATA[0] */ 
    egressTcam.d.egress.data.express = true;

    egressTcam.d.egress.data.port = 0;
    egressTcam.d.egress.data.numTags = 0x001;
    egressTcam.d.egress.data.packetType = 0;
    egressTcam.d.egress.data.macDa = 0x0000979d22c7;
    egressTcam.d.egress.data.macSa = 0x0000deadbeef;
    egressTcam.d.egress.data.etherType = 0; /* 0x0800; */ 

    /* Data mask */ 
    /* RS_CMCS_CCPM_TX_SLAVE.FLOWID_TCAM_MASK[0] */ 
    egressTcam.d.egress.mask.express = true;
    egressTcam.d.egress.mask.port = 0;
    egressTcam.d.egress.mask.numTags = 0x000;
    egressTcam.d.egress.mask.packetType = 0;
    /* egressTcam.d.egress.mask.macDa = 0xffffffffffff; */
    /* egressTcam.d.egress.mask.macSa = 0xffffffffffff; */
    egressTcam.d.egress.mask.bonusData = 0xFFFF;
    egressTcam.d.egress.mask.etherType = 0xFFFF;  /* allow all egress etypes */

    if (sampleOptions & MTD_RSMACS_SAMPLE_DEFAULT_MATCH)
    {
        egressTcam.d.egress.mask.macDa = 0xffffffffffff; /* default match MAC (Wildcard for all MAC)*/
        egressTcam.d.egress.mask.macSa = 0xffffffffffff;
    }
    else
    {
        egressTcam.d.egress.mask.macDa = 0x000000000000; /* exact match MAC address */
        egressTcam.d.egress.mask.macSa = 0x000000000000;
    }

    MTD_ATTEMPT(MacsecWriteTcam(mcsDev_p,
        MACSEC_EGRESS,
        egressFlowId,
        &egressTcam));

    MTD_ATTEMPT(MacsecEnableTcam(mcsDev_p,
        MACSEC_EGRESS,
        egressFlowId,
        true)); /* enable */ 

    memset(&egressPolicy, 0, sizeof(MacsecSecyPolicyTable_t));

    /* RS_CMCS_CCPM_TX_SLAVE.SECY_PLCY_MEM[0] */ 
    egressPolicy.egress.controlledPortEnabled = true;
    egressPolicy.egress.preSectagAuthEnable = false;
    egressPolicy.egress.protectFrames = true;
    egressPolicy.egress.mtu = 65535;
    if (sampleOptions & MTD_RSMACS_SAMPLE_256_XPN)
    {
        /* cipher options:0 = GCM-AES-128
        1 = GCM-AES-256
        2 = GCM-AES-XPN-128
        3 = GCM-AES-XPN-256 */
        egressPolicy.egress.cipher = 3;
    }
    else
    {
        egressPolicy.egress.cipher = 0;
    }
    egressPolicy.egress.confidentialityOffset = 0;
    egressPolicy.egress.sectagOffset = 12;
    egressPolicy.egress.sectagTci = 11;

    MTD_ATTEMPT(MacsecWriteSecyPolicy(mcsDev_p,
        MACSEC_EGRESS,
        secY,
        &egressPolicy));

    return MTD_OK;
}

MTD_STATUS mtdRSMACSecSampleSetSA
(
    IN MTD_DEV_PTR pDev,
    IN MTD_U16 mdioPort,
    IN MTD_RSMACS_SAMPLE_OPTION sampleOptions
)
{
    RmsDev_t *mcsDev_p;

    mcsDev_p = &pDev->rsMACSecCntl.rmsDev;

    /*
    * INGRESS SA
    */
    MacsecSaPolicy_t    saPolicy;
    bool                saLock;

    memset(&saPolicy, 0, sizeof(MacsecSaPolicy_t));

    saPolicy.ingress.ssci = 0xac99ddca;
    memcpy(saPolicy.ingress.hashkey, &hashkey[0], sizeof(uint8_t) * 16);
    memcpy(saPolicy.ingress.sak, &sak[0], sizeof(uint8_t) * 32);
    memcpy(saPolicy.ingress.salt, &salt[0], sizeof(uint8_t) * 12);
    saLock = false; /* Don't lockout SAK/HashKey */ 

    MTD_ATTEMPT(MacsecSetSaPolicy(mcsDev_p,
        MACSEC_INGRESS,
        0, /* ingressSa */
        &saPolicy,
        saLock));

    memset(&saPolicy, 0, sizeof(MacsecSaPolicy_t));

    saPolicy.ingress.ssci = 0x4918eb7a;
    memcpy(saPolicy.ingress.hashkey, &hashkey[1], sizeof(uint8_t) * 16);
    memcpy(saPolicy.ingress.sak, &sak[1], sizeof(uint8_t) * 32);
    memcpy(saPolicy.ingress.salt, &salt[1], sizeof(uint8_t) * 12);
    saLock = false; /* Don't lockout SAK/HashKey */ 

    MTD_ATTEMPT(MacsecSetSaPolicy(mcsDev_p,
        MACSEC_INGRESS,
        1, /* ingressSa */
        &saPolicy,
        saLock));

    uint64_t igNextPn;

    igNextPn = 0x0000000000000001;
    MTD_ATTEMPT(MacsecSetNextPn(mcsDev_p,
        MACSEC_INGRESS,
        0,
        igNextPn));

    igNextPn = 0x0000000000000001;
    MTD_ATTEMPT(MacsecSetNextPn(mcsDev_p,
        MACSEC_INGRESS,
        1,
        igNextPn));

    MTD_ATTEMPT(MacsecEnableIngressSa(mcsDev_p,
        0, /*index*/
        0, /*saIndex*/
        true));      /* enable */ 
    MTD_ATTEMPT(MacsecEnableIngressSa(mcsDev_p,
        1, /*index*/
        1, /*saIndex*/
        true));      /* enable */ 

    /*
    * EGRESS SA
    */
    memset(&saPolicy, 0, sizeof(MacsecSaPolicy_t));

    saPolicy.egress.ssci = 0xac99ddca;
    saPolicy.egress.sectagAn = 0;
    memcpy(saPolicy.egress.hashkey, &eghashkey[0], sizeof(uint8_t) * 16);
    memcpy(saPolicy.egress.sak, &egsak[0], sizeof(uint8_t) * 32);
    memcpy(saPolicy.egress.salt, &egsalt[0], sizeof(uint8_t) * 12);
    saLock = false; /* Don't lockout SAK/HashKey */ 

    MTD_ATTEMPT(MacsecSetSaPolicy(mcsDev_p,
        MACSEC_EGRESS,
        0, /* egress saIndex0 */
        &saPolicy,
        saLock));

    memset(&saPolicy, 0, sizeof(MacsecSaPolicy_t));

    saPolicy.egress.ssci = 0x4918eb7a;
    saPolicy.egress.sectagAn = 1;
    memcpy(saPolicy.egress.hashkey, &eghashkey[1], sizeof(uint8_t) * 16);
    memcpy(saPolicy.egress.sak, &egsak[1], sizeof(uint8_t) * 32);
    memcpy(saPolicy.egress.salt, &egsalt[1], sizeof(uint8_t) * 12);
    saLock = false; /* Don't lockout SAK/HashKey */ 

    MTD_ATTEMPT(MacsecSetSaPolicy(mcsDev_p,
        MACSEC_EGRESS,
        1, /* egress saIndex1 */
        &saPolicy,
        saLock));

    uint64_t egNextPn;

    egNextPn = 0x0000000000000001;
    MTD_ATTEMPT(MacsecSetNextPn(mcsDev_p,
        MACSEC_EGRESS,
        0,
        egNextPn));

    egNextPn = 0x0000000000000001;
    MTD_ATTEMPT(MacsecSetNextPn(mcsDev_p,
        MACSEC_EGRESS,
        1,
        egNextPn));

    MacsecEgressSaMapEntry_t saMap;

    memset(&saMap, 0, sizeof(MacsecEgressSaMapEntry_t));

    /* refer to the programming guide for more details on rekey options */
    saMap.autoRekeyEnable = true;
    saMap.saIndex0Vld = true; /* True indicates that the corresponding SC's SA index0 is valid. */
    saMap.saIndex1Vld = true; /* True indicates that the corresponding SC's SA index1 is valid. */
    saMap.txSaActive = 0;     /* If 0, the sa_index0 is the currently active SA index */
    saMap.saIndex0 = 0;       /* SA index assigned to saIndex0 use for this packet */
    saMap.saIndex1 = 1;       /* SA index assigned to saIndex1 use for this packet */
    saMap.sectagSci = sci;

    MTD_ATTEMPT(MacsecSetEgressSaMap(mcsDev_p,
        0,
        &saMap));

    return MTD_OK;
}


MTD_STATUS mtdRSMACSecSampleRekeyHandler
(
    IN MTD_DEV_PTR pDev,
    IN MTD_U16 mdioPort
)
{
    RmsDev_t* mcsDev_p;

    mcsDev_p = &pDev->rsMACSecCntl.rmsDev;

    MTD_ATTEMPT(MacsecTxRekeyHandler(mcsDev_p, sc, NULL, false));
    MTD_ATTEMPT(MacsecRxRekeyHandler(mcsDev_p, sc, 0));
    MTD_ATTEMPT(MacsecRxRekeyHandler(mcsDev_p, sc, 1));

    /* MTD_ATTEMPT(MacsecClearPacketSeen(mcsDev_p, sc, 0)); */
    /* MTD_ATTEMPT(MacsecClearPacketSeen(mcsDev_p, sc, 1)); */

    MTD_ATTEMPT(MacsecSetNextPn(mcsDev_p, MACSEC_INGRESS, 0, 1));
    MTD_ATTEMPT(MacsecSetNextPn(mcsDev_p, MACSEC_EGRESS, 0, 1));

    MTD_DBG_INFO("mtdRSMACSecSampleRekeyHandler\n");

    return MTD_OK;
}


MTD_STATUS mtdRSMACSecSampleGetInterrupts
(
    IN MTD_DEV_PTR pDev,
    IN MTD_U16 mdioPort
)
{
    RmsDev_t* mcsDev_p;
    MTD_BOOL interruptTrig;
    MTD_INTR_STATUS deviceInterruptStatus;
    MacsecInterrupts_t mcsInterruptStatus;

    mcsDev_p = &pDev->rsMACSecCntl.rmsDev;

    MTD_DBG_INFO("mtdRSMACSecSampleGetInterrupts: MTD_RSMACS_SAMPLE_INTERRUPTS option\n");

    /*  */
    MTD_ATTEMPT(mtdGetPortIntrStatus(pDev, mdioPort, &interruptTrig, &deviceInterruptStatus));
    MTD_DBG_INFO("mtdGetPortIntrStatus interruptTrigFlag:%x deviceInterruptStatus.cunitIntr:%x\n",
        interruptTrig, deviceInterruptStatus.cunitIntr);

    if (interruptTrig)
    {
        if (deviceInterruptStatus.cunitIntr & MTD_PTP_MPM_INTR)
        {
            MTD_DBG_INFO("mtdRSMACSecSampleGetInterrupts: Top level MACSec interrupt triggered\n");
        }
        else
        {
            MTD_DBG_INFO("mtdRSMACSecSampleGetInterrupts: Top level MACSec interrupt not triggered\n");
            return MTD_OK;
        }
    }
    else
    {
        MTD_DBG_INFO("mtdRSMACSecSampleGetInterrupts: No MACSec interrupt\n");
        return MTD_OK;
    }

    /* */
    MTD_ATTEMPT(MacsecGetInterruptStatus(mcsDev_p, &mcsInterruptStatus));

    MTD_DBG_INFO("mtdRSMACSecSampleGetInterrupts   : \n");
    MTD_DBG_INFO("bbeRxInterrupts.dfifoOverflow    : %x\n", mcsInterruptStatus.bbeRxInterrupts.dfifoOverflow);
    MTD_DBG_INFO("bbeRxInterrupts.plfifoOverflow   : %x\n", mcsInterruptStatus.bbeRxInterrupts.plfifoOverflow);

    MTD_DBG_INFO("bbeTxInterrupts.dfifoOverflow    : %x\n", mcsInterruptStatus.bbeTxInterrupts.dfifoOverflow);
    MTD_DBG_INFO("bbeTxInterrupts.plfifoOverflow   : %x\n", mcsInterruptStatus.bbeTxInterrupts.plfifoOverflow);

    MTD_DBG_INFO("pabRxInterrupts.overflow         : %x\n", mcsInterruptStatus.pabRxInterrupts.overflow);

    MTD_DBG_INFO("pabTxInterrupts.overflow         : %x\n", mcsInterruptStatus.pabTxInterrupts.overflow);

    MTD_DBG_INFO("pexRxInterrupts.earlyPreemptErr  : %x\n", mcsInterruptStatus.pexRxInterrupts.earlyPreemptErr);

    MTD_DBG_INFO("pexTxInterrupts.earlyPreemptErr  : %x\n", mcsInterruptStatus.pexTxInterrupts.earlyPreemptErr);

    MTD_DBG_INFO("cpmRxInterrupts.sectagVEq1       : %x\n", mcsInterruptStatus.cpmRxInterrupts.sectagVEq1);
    MTD_DBG_INFO("cpmRxInterrupts.sectagEEq0CEq1   : %x\n", mcsInterruptStatus.cpmRxInterrupts.sectagEEq0CEq1);
    MTD_DBG_INFO("cpmRxInterrupts.sectagSlGte48    : %x\n", mcsInterruptStatus.cpmRxInterrupts.sectagSlGte48);
    MTD_DBG_INFO("cpmRxInterrupts.sectagEsEq1ScEq1 : %x\n", mcsInterruptStatus.cpmRxInterrupts.sectagEsEq1ScEq1);
    MTD_DBG_INFO("cpmRxInterrupts.sectagScEq1ScbEq1: %x\n", mcsInterruptStatus.cpmRxInterrupts.sectagScEq1ScbEq1);
    MTD_DBG_INFO("cpmRxInterrupts.packetXpnEq0     : %x\n", mcsInterruptStatus.cpmRxInterrupts.packetXpnEq0);
    MTD_DBG_INFO("cpmRxInterrupts.pnThreshReached  : %x\n", mcsInterruptStatus.cpmRxInterrupts.pnThreshReached);

    MTD_DBG_INFO("cpmRxInterrupts.scExpiryPreTimeout   : %x\n", mcsInterruptStatus.cpmRxInterrupts.scExpiryPreTimeout);
    MTD_DBG_INFO("cpmRxInterrupts.scExpiryActualTimeout: %x\n", mcsInterruptStatus.cpmRxInterrupts.scExpiryActualTimeout);
    MTD_DBG_INFO("cpmTxInterrupts.packetXpnEq0         : %x\n", mcsInterruptStatus.cpmTxInterrupts.packetXpnEq0);
    MTD_DBG_INFO("cpmTxInterrupts.pnThreshReached      : %x\n", mcsInterruptStatus.cpmTxInterrupts.pnThreshReached);
    MTD_DBG_INFO("cpmTxInterrupts.saNotValid           : %x\n", mcsInterruptStatus.cpmTxInterrupts.saNotValid);
        
    MTD_DBG_INFO("cpmTxInterrupts.scExpiryPreTimeout   : %x\n", mcsInterruptStatus.cpmTxInterrupts.scExpiryPreTimeout);
    MTD_DBG_INFO("cpmTxInterrupts.scExpiryActualTimeout: %x\n", mcsInterruptStatus.cpmTxInterrupts.scExpiryActualTimeout);

    /* note that MACSec interrupt are not self-clearing. To clear the MACSec interrupts, call
     MacsecClearInterrupts() with the selected interrupts in MacsecInterrupts_t to clear */
    MTD_ATTEMPT(MacsecClearInterrupts(mcsDev_p, mcsInterruptStatus));

    return MTD_OK;
}


/* www.ieee802.org/1/files/public/docs2011/bn-randall-test-vectors-0511-v1.pdf
2.8.1 75-byte Packet Encryption Using GCM-AES-128 */
MTD_STATUS mtdRSMACSecFIPsSample_1_1_GCM_AES128_75B
(
    IN MTD_DEV_PTR pDev,
    IN MTD_U16 mdioPort
)
{
    RmsDev_t* mcsDev_p;
    MacsecFipsOperation_t FipsOperation;
    MacsecFipsMode_t FipsMode;
    MacsecFipsSakData_t FipsSakData;
    MacsecPacket_t Packet; /* input clear text, output encrypted */ 
    bool result;
    char tag[16] = ""; /* tag will be returned as ICV */ 

    mcsDev_p = &pDev->rsMACSecCntl.rmsDev;
    FipsOperation = MACSEC_FIPS_ENCRYPT;
    FipsMode.ctrMode = MACSEC_FIPS_CTR_AUTO;
    FipsMode.ecbMode = MACSEC_FIPS_AES_ECB_DISABLED;
    FipsMode.keyLen = MACSEC_FIPS_KEY_LEN_128;


    FipsSakData.sakBits127_64 = 0x88EE087FD95DA9FB;
    FipsSakData.sakBits63_0 = 0xF6725AA9D757B0CD;
    FipsSakData.hashkeyBits127_64 = 0xAE19118C3B704FCE;
    FipsSakData.hashkeyBits63_0 = 0x42AE0D15D2C15C7A;
    FipsSakData.ivBits95_64 = 0x7AE8E2CA;
    FipsSakData.ivBits63_0 = 0x4EC500012E58495C;

    Packet.size = 83;
    Packet.pre_st_aad_start = 0;
    Packet.pre_st_aad_end = 12;
    Packet.aad_size = 8;
    Packet.aad_start_offset = 12;

    Packet.data = (char*)malloc(sizeof(char) * (Packet.size + 1));
    if (Packet.data == NULL)
    {
        MTD_DBG_ERROR("failed to allocate memory for Packet data");
        return MTD_FAIL;
    }

    memset(Packet.data, 0, Packet.size);
    memcpy(Packet.data, &packet75_A[0], sizeof(char) * Packet.size);

    MTD_DBG_INFO("input packet data:\n");
    for (int i = 0; i < Packet.size; i++)
    {
        MTD_DBG_INFO("%02X", (unsigned char)Packet.data[i]);
        if ((i > 0 && i % 16 == 15) || (i == Packet.size - 1))
            MTD_DBG_INFO("\n");
    }
    MacsecPacketToFips(mcsDev_p, FipsOperation, FipsMode, FipsSakData,
        Packet, &result, tag);

    MTD_DBG_INFO("\n\nmtdRSMACSecSampleFIPs result %d.\n", result);
    MTD_DBG_INFO("\noutput tag:\n");
    for (int i = 0; i < 16; i++)
    {
        MTD_DBG_INFO("%02X", (unsigned char)tag[i]);
    }

    MTD_DBG_INFO("\noutput packet data:\n");
    for (int i = 0; i < Packet.size; i++)
    {
        MTD_DBG_INFO("%02X", (unsigned char)Packet.data[i]);
        if ((i > 0 && i % 16 == 15) || (i == Packet.size - 1))
            MTD_DBG_INFO("\n");
    }

    free(Packet.data);

    return MTD_OK;
}

/* www.ieee802.org/1/files/public/docs2011/bn-randall-test-vectors-0511-v1.pdf
2.8.2 75-byte Packet Encryption Using GCM-AES-256 */
MTD_STATUS mtdRSMACSecFIPsSample_1_2_GCM_AES256_75B
(
    IN MTD_DEV_PTR pDev,
    IN MTD_U16 mdioPort
)
{
    RmsDev_t* mcsDev_p;
    MacsecFipsOperation_t FipsOperation;
    MacsecFipsMode_t FipsMode;
    MacsecFipsSakData_t FipsSakData;
    MacsecPacket_t Packet;
    bool result;
    char tag[16] = "";

    mcsDev_p = &pDev->rsMACSecCntl.rmsDev;
    FipsOperation = MACSEC_FIPS_ENCRYPT;
    FipsMode.ctrMode = MACSEC_FIPS_CTR_AUTO;
    FipsMode.ecbMode = MACSEC_FIPS_AES_ECB_DISABLED;
    FipsMode.keyLen = MACSEC_FIPS_KEY_LEN_256;

    FipsSakData.sakBits255_192 = 0x4C973DBC73646216;
    FipsSakData.sakBits191_128 = 0x74F8B5B89E5C1551;
    FipsSakData.sakBits127_64 = 0x1FCED9216490FB1C;
    FipsSakData.sakBits63_0 = 0x1A2CAA0FFE0407E5;
    FipsSakData.hashkeyBits127_64 = 0x9A5E559A96459C21;
    FipsSakData.hashkeyBits63_0 = 0xE43C0DFF0FA426F3;
    FipsSakData.ivBits95_64 = 0x7AE8E2CA;
    FipsSakData.ivBits63_0 = 0x4EC500012E58495C;

    /* packet size include AAD and payload data */
    Packet.size = 83;
    /* In this case AAD include DA, SA and MACsec TAG total 20 bytes*/
    Packet.pre_st_aad_start = 0;
    Packet.pre_st_aad_end = 12;
    Packet.aad_size = 8;
    Packet.aad_start_offset = 12;
    int input_size = Packet.size;/* +Packet.aad_size; */ 
    int output_size = input_size; /* output_size does not include ICV, ICV is returned in tag */ 

    /* allocate enough memory for both input and output packet data */ 
    /* for encryption, output data has additional 16byte ICV compared to input data */ 
    Packet.data = (char*)malloc(sizeof(char) * (output_size + 1));
    if (Packet.data == NULL)
    {
        MTD_DBG_ERROR("failed to allocate memory for Packet data");
        return MTD_FAIL;
    }

    memset(Packet.data, 0, output_size);
    memcpy(Packet.data, &packet75_A[0], sizeof(char) * input_size);

    /* memset(&tag, 0, sizeof(char) * 16); */

    MTD_DBG_INFO("input packet data:\n");
    for (int i = 0; i < input_size; i++)
    {
        MTD_DBG_INFO("%02X", (unsigned char)Packet.data[i]);
        if ((i > 0 && i % 16 == 15) || (i == input_size - 1))
            MTD_DBG_INFO("\n");
    }
    MacsecPacketToFips(mcsDev_p, FipsOperation, FipsMode, FipsSakData,
        Packet, &result, tag);

    MTD_DBG_INFO("\n\nmtdRSMACSecSampleFIPs result %d.\n", result);
    MTD_DBG_INFO("\noutput tag:\n");
    for (int i = 0; i < 16; i++)
    {
        MTD_DBG_INFO("%02X", (unsigned char)tag[i]);
    }

    MTD_DBG_INFO("\noutput packet data:\n");
    for (int i = 0; i < output_size; i++)
    {
        MTD_DBG_INFO("%02X", (unsigned char)Packet.data[i]);
        if ((i > 0 && i % 16 == 15) || (i == output_size - 1))
            MTD_DBG_INFO("\n");
    }

    free(Packet.data);

    return MTD_OK;
}
/* 54-byte Packet Encryption Using GCM-AES-128 */
MTD_STATUS mtdRSMACSecFIPsSample_2_1_GCM_AES128_54B
(
    IN MTD_DEV_PTR pDev,
    IN MTD_U16 mdioPort
)
{
    RmsDev_t* mcsDev_p;
    MacsecFipsOperation_t FipsOperation;
    MacsecFipsMode_t FipsMode;
    MacsecFipsSakData_t FipsSakData;
    MacsecPacket_t Packet;
    bool result;
    char tag[16] = "";

    mcsDev_p = &pDev->rsMACSecCntl.rmsDev;
    FipsOperation = MACSEC_FIPS_ENCRYPT;
    FipsMode.ctrMode = MACSEC_FIPS_CTR_AUTO;
    FipsMode.ecbMode = MACSEC_FIPS_AES_ECB_DISABLED;
    FipsMode.keyLen = MACSEC_FIPS_KEY_LEN_128;


    FipsSakData.sakBits127_64 = 0x071B113B0CA743FE;
    FipsSakData.sakBits63_0 = 0xCCCF3D051F737382;
    FipsSakData.hashkeyBits127_64 = 0xE4E01725D724C121;
    FipsSakData.hashkeyBits63_0 = 0x5C7309AD34539257;
    FipsSakData.ivBits95_64 = 0xF0761E8D;
    FipsSakData.ivBits63_0 = 0xCD3D000176D457ED;

    Packet.pre_st_aad_start = 0;
    Packet.pre_st_aad_end = 12;
    Packet.aad_size = 8;
    Packet.aad_start_offset = 12;
    Packet.size = 62;

    Packet.data = (char *)malloc(sizeof(char) * (Packet.size + 1));
    if (Packet.data == NULL)
    {
        MTD_DBG_ERROR("failed to allocate memory for Packet data");
        return MTD_FAIL;
    }

    memset(Packet.data, 0, Packet.size);
    memcpy(Packet.data, &packet54_A[0], sizeof(char) * Packet.size);

    
    /* memset(&tag, 0, sizeof(char) * 16); */

    MTD_DBG_INFO("input packet data:\n");
    for (int i = 0; i < Packet.size; i++)
    {
        MTD_DBG_INFO("%02X", (unsigned char)Packet.data[i]);
        if ((i>0 && i % 16 == 15) || (i == Packet.size - 1))
            MTD_DBG_INFO("\n");
    }
    MacsecPacketToFips(mcsDev_p, FipsOperation, FipsMode, FipsSakData,
        Packet, &result, tag);

    MTD_DBG_INFO("\n\nmtdRSMACSecSampleFIPs result %d.\n", result);
    MTD_DBG_INFO("\noutput tag:\n");
    for (int i = 0; i < 16; i++)
    {
        MTD_DBG_INFO("%02X", (unsigned char)tag[i]);
    }
  
    MTD_DBG_INFO("\noutput packet data:\n");
    for (int i = 0; i < Packet.size; i++)
    {
        MTD_DBG_INFO("%02X", (unsigned char)Packet.data[i]);
        if ((i > 0 && i % 16 == 15) || (i == Packet.size - 1))
            MTD_DBG_INFO("\n");
    }

    free(Packet.data);

    return MTD_OK;
}
/* 54-byte Packet Decryption Using GCM-AES-128
   input packet is encrypted, output packet is clear text
   input packet use output from mtdRSMACSecFIPsSample_2_1_GCM_AES128_54B
*/
MTD_STATUS mtdRSMACSecFIPsSample_2_2_GCM_AES128_54B
(
    IN MTD_DEV_PTR pDev,
    IN MTD_U16 mdioPort
)
{
    RmsDev_t* mcsDev_p;
    MacsecFipsOperation_t FipsOperation;
    MacsecFipsMode_t FipsMode;
    MacsecFipsSakData_t FipsSakData;
    MacsecPacket_t Packet;
    bool result;
    char tag[16] = "";

    mcsDev_p = &pDev->rsMACSecCntl.rmsDev;
    FipsOperation = MACSEC_FIPS_DECRYPT;
    FipsMode.ctrMode = MACSEC_FIPS_CTR_AUTO;
    FipsMode.ecbMode = MACSEC_FIPS_AES_ECB_DISABLED;
    FipsMode.keyLen = MACSEC_FIPS_KEY_LEN_128;


    FipsSakData.sakBits127_64 = 0x071B113B0CA743FE;
    FipsSakData.sakBits63_0 = 0xCCCF3D051F737382;
    FipsSakData.hashkeyBits127_64 = 0xE4E01725D724C121;
    FipsSakData.hashkeyBits63_0 = 0x5C7309AD34539257;
    FipsSakData.ivBits95_64 = 0xF0761E8D;
    FipsSakData.ivBits63_0 = 0xCD3D000176D457ED;

    Packet.pre_st_aad_start = 0;
    Packet.pre_st_aad_end = 12;
    Packet.aad_size = 8;
    Packet.aad_start_offset = 12;
    Packet.size = 62;

    Packet.data = (char*)malloc(sizeof(char) * (Packet.size + 1));
    if (Packet.data == NULL)
    {
        MTD_DBG_ERROR("failed to allocate memory for Packet data");
        return MTD_FAIL;
    }

    memset(Packet.data, 0, Packet.size);
    memcpy(Packet.data, &packet54_A_C[0], sizeof(char) * Packet.size);


    /* memset(&tag, 0, sizeof(char) * 16); */

    MTD_DBG_INFO("input packet data:\n");
    for (int i = 0; i < Packet.size; i++)
    {
        MTD_DBG_INFO("%02X", (unsigned char)Packet.data[i]);
        if ((i > 0 && i % 16 == 15) || (i == Packet.size - 1))
            MTD_DBG_INFO("\n");
    }
    MacsecPacketToFips(mcsDev_p, FipsOperation, FipsMode, FipsSakData,
        Packet, &result, tag);

    MTD_DBG_INFO("\n\nmtdRSMACSecSampleFIPs result %d.\n", result);
    MTD_DBG_INFO("\noutput tag:\n");
    for (int i = 0; i < 16; i++)
    {
        MTD_DBG_INFO("%02X", (unsigned char)tag[i]);
    }

    MTD_DBG_INFO("\noutput packet data:\n");
    for (int i = 0; i < Packet.size; i++)
    {
        MTD_DBG_INFO("%02X", (unsigned char)Packet.data[i]);
        if ((i > 0 && i % 16 == 15) || (i == Packet.size - 1))
            MTD_DBG_INFO("\n");
    }

    free(Packet.data);

    return MTD_OK;
}

/* 54-byte Packet Encryption Using AES_ECB 128 
   no MACsec tag or ICV is involved, aad set to 0
   clear text packet is simply encrypted at output
*/
MTD_STATUS mtdRSMACSecFIPsSample_3_1_ECB_AES128_54B
(
    IN MTD_DEV_PTR pDev,
    IN MTD_U16 mdioPort
)
{
    RmsDev_t* mcsDev_p;
    MacsecFipsOperation_t FipsOperation;
    MacsecFipsMode_t FipsMode;
    MacsecFipsSakData_t FipsSakData;
    MacsecPacket_t Packet;
    bool result;
    char tag[16] = "";

    mcsDev_p = &pDev->rsMACSecCntl.rmsDev;
    FipsOperation = MACSEC_FIPS_ENCRYPT;
    FipsMode.ctrMode = MACSEC_FIPS_CTR_AUTO;
    FipsMode.ecbMode = MACSEC_FIPS_AES_ECB_ENABLED;
    FipsMode.keyLen = MACSEC_FIPS_KEY_LEN_128;

    FipsSakData.sakBits127_64 = 0x071B113B0CA743FE;
    FipsSakData.sakBits63_0 = 0xCCCF3D051F737382;
    FipsSakData.hashkeyBits127_64 = 0xE4E01725D724C121;
    FipsSakData.hashkeyBits63_0 = 0x5C7309AD34539257;
    FipsSakData.ivBits95_64 = 0xF0761E8D;
    FipsSakData.ivBits63_0 = 0xCD3D000176D457ED;

    Packet.pre_st_aad_start = 0;
    Packet.pre_st_aad_end = 0;
    Packet.aad_size = 0;
    Packet.aad_start_offset = 0;
    Packet.size = 54;

    Packet.data = (char*)malloc(sizeof(char) * (Packet.size + 1));
    if (Packet.data == NULL)
    {
        MTD_DBG_ERROR("failed to allocate memory for Packet data");
        return MTD_FAIL;
    }

    memset(Packet.data, 0, Packet.size);
    memcpy(Packet.data, &packet54[0], sizeof(char) * Packet.size);

    MTD_DBG_INFO("input packet data:\n");
    for (int i = 0; i < Packet.size; i++)
    {
        MTD_DBG_INFO("%02X", (unsigned char)Packet.data[i]);
        if ((i > 0 && i % 16 == 15) || (i == Packet.size - 1))
            MTD_DBG_INFO("\n");
    }
    MacsecPacketToFips(mcsDev_p, FipsOperation, FipsMode, FipsSakData,
        Packet, &result, tag);

    MTD_DBG_INFO("\n\nmtdRSMACSecSampleFIPs result %d.\n", result);
    MTD_DBG_INFO("\noutput tag:\n");
    for (int i = 0; i < 16; i++)
    {
        MTD_DBG_INFO("%02X", (unsigned char)tag[i]);
    }

    MTD_DBG_INFO("\noutput packet data:\n");
    for (int i = 0; i < Packet.size; i++)
    {
        MTD_DBG_INFO("%02X", (unsigned char)Packet.data[i]);
        if ((i > 0 && i % 16 == 15) || (i == Packet.size - 1))
            MTD_DBG_INFO("\n");
    }

    free(Packet.data);

    return MTD_OK;
}
MTD_STATUS mtdRSMACSecSampleDumpMCSRegs
(
    IN MTD_DEV_PTR pDev,
    IN MTD_U16 mdioPort
)
{
    RmsDev_t *mcsDev_p;
    MTD_U32 data32;
    MTD_U16 i;

    mcsDev_p = &pDev->rsMACSecCntl.rmsDev;

    MTD_DBG_INFO("mtdRSMACSecSampleDumpMCSRegs.\n");

    for (i = 0; i < 0x7FFF/2; i++)
    {
        MTD_ATTEMPT(mtdRSMACSecRegRead32(pDev, (0x0000 + (i * 2)) * 2, &data32));
        MTD_DBG_INFO("X3610Reg:1F.0x%04X RiantaReg:1F.0x%04X - Val:0x%08X\n", (0x0000 + (i * 2)), (0x0000 + (i * 2)) * 2, data32);
    }

    return MTD_OK;

}
