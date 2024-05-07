/*! 
 * @file macsec_ieee_api.c 
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
#include "macsec_ieee_api.h"
#include "macsec_api.h"

#include "mcs_internals_ieee.h"


//------------------------------------------------------------------------------
// T-CAM section
//------------------------------------------------------------------------------

static int readIngressFlowIdTcam( RmsDev_t *				rmsDev_p,
                                  int					tableIndex,
                                  Ra01IfRsMcsCpmRxFlowidTcamData_t *	data_p,
                                  Ra01IfRsMcsCpmRxFlowidTcamMask_t *	mask_p )
{
    int ret = 0;

    ret = Ra01ReadRsMcsCpmRxFlowidTcamData( rmsDev_p, tableIndex, data_p);
    DBGRETURN(ret);

    ret = Ra01ReadRsMcsCpmRxFlowidTcamMask( rmsDev_p, tableIndex, mask_p);

    return ret;
}


static int writeIngressFlowIdTcam( RmsDev_t *				rmsDev_p,
                                  int					tableIndex,
                                  Ra01IfRsMcsCpmRxFlowidTcamData_t *	data_p,
                                  Ra01IfRsMcsCpmRxFlowidTcamMask_t *	mask_p )
{
    int ret = 0;

    ret = Ra01WriteRsMcsCpmRxFlowidTcamData( rmsDev_p, tableIndex, data_p);
    DBGRETURN(ret);

    ret = Ra01WriteRsMcsCpmRxFlowidTcamMask( rmsDev_p, tableIndex, mask_p);

    return ret;
}

static int readEgressFlowIdTcam( RmsDev_t *				rmsDev_p,
                                 int					tableIndex,
				 Ra01IfRsMcsCpmTxFlowidTcamData_t *	data_p,
				 Ra01IfRsMcsCpmTxFlowidTcamMask_t *	mask_p )
{
    int ret = 0;

    ret = Ra01ReadRsMcsCpmTxFlowidTcamData( rmsDev_p, tableIndex, data_p);
    DBGRETURN(ret);

    ret = Ra01ReadRsMcsCpmTxFlowidTcamMask( rmsDev_p, tableIndex, mask_p);

    return ret;
}

static int writeEgressFlowIdTcam( RmsDev_t *				rmsDev_p,
                                 int					tableIndex,
				 Ra01IfRsMcsCpmTxFlowidTcamData_t *	data_p,
				 Ra01IfRsMcsCpmTxFlowidTcamMask_t *	mask_p )
{
    int ret = 0;

    ret = Ra01WriteRsMcsCpmTxFlowidTcamData( rmsDev_p, tableIndex, data_p);
    DBGRETURN(ret);

    ret = Ra01WriteRsMcsCpmTxFlowidTcamMask( rmsDev_p, tableIndex, mask_p);

    return ret;
}


int MacsecReadTcam( RmsDev_t * rmsDev_p, MacsecDirection_t direction, unsigned index, MacsecFlowCfg_t * tcam )
{
    int ret = 0;

    if( (rmsDev_p == NULL) || (tcam == NULL) ) {
        return -EINVAL;
    }

    if( direction == MACSEC_INGRESS ) {
      Ra01IfRsMcsCpmRxFlowidTcamData_t 	data;
      Ra01IfRsMcsCpmRxFlowidTcamMask_t 	mask;

      ret = readIngressFlowIdTcam( rmsDev_p, index,  &data, &mask );
      DBGRETURN(ret);

      tcam->d.ingress.data.express		= data.express;
      tcam->d.ingress.data.port			= data.port;
      tcam->d.ingress.data.tagMatchBitmap	= data.tagMatchBitmap;
      tcam->d.ingress.data.bonusData		= data.bonusData;
      tcam->d.ingress.data.etherType		= data.etherType;
      tcam->d.ingress.data.innerVlanType	= data.innerVlanType;
      tcam->d.ingress.data.numTags		= data.numTags;
      tcam->d.ingress.data.outerPriority	= data.outerPriority;
      tcam->d.ingress.data.outerTagId		= data.outerTagId;
      tcam->d.ingress.data.outerVlanType	= data.outerVlanType;
      tcam->d.ingress.data.packetType		= data.packetType;
      tcam->d.ingress.data.secondOuterPriority	= data.secondOuterPriority;
      tcam->d.ingress.data.secondOuterTagId	= data.secondOuterTagId;
      tcam->d.ingress.data.macDa		= data.macDa;
      tcam->d.ingress.data.macSa		= data.macSa;

      tcam->d.ingress.mask.express		= mask.maskExpress;
      tcam->d.ingress.mask.port			= mask.maskPort;
      tcam->d.ingress.mask.tagMatchBitmap	= mask.maskTagMatchBitmap;
      tcam->d.ingress.mask.bonusData		= mask.maskBonusData;
      tcam->d.ingress.mask.etherType		= mask.maskEtherType;
      tcam->d.ingress.mask.innerVlanType	= mask.maskInnerVlanType;
      tcam->d.ingress.mask.numTags		= mask.maskNumTags;
      tcam->d.ingress.mask.outerPriority	= mask.maskOuterPriority;
      tcam->d.ingress.mask.outerTagId		= mask.maskOuterTagId;
      tcam->d.ingress.mask.outerVlanType	= mask.maskOuterVlanType;
      tcam->d.ingress.mask.packetType		= mask.maskPacketType;
      tcam->d.ingress.mask.secondOuterPriority	= mask.maskSecondOuterPriority;
      tcam->d.ingress.mask.secondOuterTagId	= mask.maskSecondOuterTagId;
      tcam->d.ingress.mask.macDa		= mask.maskMacDa;
      tcam->d.ingress.mask.macSa		= mask.maskMacSa;

      ret = getRxSecyMap( rmsDev_p, index, &tcam->secY, &tcam->controlPacket );
    } else {
      Ra01IfRsMcsCpmTxFlowidTcamData_t 	data;
      Ra01IfRsMcsCpmTxFlowidTcamMask_t 	mask;

      ret = readEgressFlowIdTcam( rmsDev_p, index,  &data, &mask );
      DBGRETURN(ret);

      tcam->d.egress.data.express		= data.express;
      tcam->d.egress.data.port			= data.port;
      tcam->d.egress.data.tagMatchBitmap	= data.tagMatchBitmap;
      tcam->d.egress.data.bonusData		= data.bonusData;
      tcam->d.egress.data.etherType		= data.etherType;
      tcam->d.egress.data.innerVlanType		= data.innerVlanType;
      tcam->d.egress.data.numTags		= data.numTags;
      tcam->d.egress.data.outerPriority		= data.outerPriority;
      tcam->d.egress.data.outerTagId		= data.outerTagId;
      tcam->d.egress.data.outerVlanType		= data.outerVlanType;
      tcam->d.egress.data.packetType		= data.packetType;
      tcam->d.egress.data.secondOuterPriority	= data.secondOuterPriority;
      tcam->d.egress.data.secondOuterTagId	= data.secondOuterTagId;
      tcam->d.egress.data.macDa			= data.macDa;
      tcam->d.egress.data.macSa			= data.macSa;

      tcam->d.egress.mask.express		= mask.maskExpress;
      tcam->d.egress.mask.port			= mask.maskPort;
      tcam->d.egress.mask.tagMatchBitmap	= mask.maskTagMatchBitmap;
      tcam->d.egress.mask.bonusData		= mask.maskBonusData;
      tcam->d.egress.mask.etherType		= mask.maskEtherType;
      tcam->d.egress.mask.innerVlanType		= mask.maskInnerVlanType;
      tcam->d.egress.mask.numTags		= mask.maskNumTags;
      tcam->d.egress.mask.outerPriority		= mask.maskOuterPriority;
      tcam->d.egress.mask.outerTagId		= mask.maskOuterTagId;
      tcam->d.egress.mask.outerVlanType		= mask.maskOuterVlanType;
      tcam->d.egress.mask.packetType		= mask.maskPacketType;
      tcam->d.egress.mask.secondOuterPriority	= mask.maskSecondOuterPriority;
      tcam->d.egress.mask.secondOuterTagId	= mask.maskSecondOuterTagId;
      tcam->d.egress.mask.macDa			= mask.maskMacDa;
      tcam->d.egress.mask.macSa			= mask.maskMacSa;

      ret = getTxSecyMap( rmsDev_p, index, &tcam->secY, &tcam->d.egress.sc, &tcam->controlPacket, &tcam->d.egress.auxiliaryPolicy );
    }

    return ret;
}


int MacsecWriteTcam( RmsDev_t * rmsDev_p, MacsecDirection_t direction, unsigned index, MacsecFlowCfg_t * tcam )
{
    int ret = 0;

    if( (rmsDev_p == NULL) || (tcam == NULL) ) {
        return -EINVAL;
    }

    if( direction == MACSEC_INGRESS ) {
      Ra01IfRsMcsCpmRxFlowidTcamData_t 	data;
      Ra01IfRsMcsCpmRxFlowidTcamMask_t 	mask;

      data.express                      = tcam->d.ingress.data.express;
      data.port                         = tcam->d.ingress.data.port;
      data.tagMatchBitmap               = tcam->d.ingress.data.tagMatchBitmap;
      data.bonusData                    = tcam->d.ingress.data.bonusData;
      data.etherType                    = tcam->d.ingress.data.etherType;
      data.innerVlanType                = tcam->d.ingress.data.innerVlanType;
      data.numTags                      = tcam->d.ingress.data.numTags;
      data.outerPriority                = tcam->d.ingress.data.outerPriority;
      data.outerTagId                   = tcam->d.ingress.data.outerTagId;
      data.outerVlanType                = tcam->d.ingress.data.outerVlanType;
      data.packetType                   = tcam->d.ingress.data.packetType;
      data.secondOuterPriority          = tcam->d.ingress.data.secondOuterPriority;
      data.secondOuterTagId             = tcam->d.ingress.data.secondOuterTagId;
      data.macDa                        = tcam->d.ingress.data.macDa;
      data.macSa                        = tcam->d.ingress.data.macSa;

      mask.maskExpress                  = tcam->d.ingress.mask.express;
      mask.maskPort                     = tcam->d.ingress.mask.port;
      mask.maskTagMatchBitmap           = tcam->d.ingress.mask.tagMatchBitmap;
      mask.maskBonusData                = tcam->d.ingress.mask.bonusData;
      mask.maskEtherType                = tcam->d.ingress.mask.etherType;
      mask.maskInnerVlanType            = tcam->d.ingress.mask.innerVlanType;
      mask.maskNumTags                  = tcam->d.ingress.mask.numTags;
      mask.maskOuterPriority            = tcam->d.ingress.mask.outerPriority;
      mask.maskOuterTagId               = tcam->d.ingress.mask.outerTagId;
      mask.maskOuterVlanType            = tcam->d.ingress.mask.outerVlanType;
      mask.maskPacketType               = tcam->d.ingress.mask.packetType;
      mask.maskSecondOuterPriority      = tcam->d.ingress.mask.secondOuterPriority;
      mask.maskSecondOuterTagId         = tcam->d.ingress.mask.secondOuterTagId;
      mask.maskMacDa                    = tcam->d.ingress.mask.macDa;
      mask.maskMacSa                    = tcam->d.ingress.mask.macSa;

      ret = writeIngressFlowIdTcam( rmsDev_p, index, &data, &mask );
      DBGRETURN(ret);

      ret = setRxSecyMap( rmsDev_p, index, tcam->secY, tcam->controlPacket );
      DBGRETURN(ret);
    } else {
      Ra01IfRsMcsCpmTxFlowidTcamData_t 	data;
      Ra01IfRsMcsCpmTxFlowidTcamMask_t 	mask;

      data.express                      = tcam->d.egress.data.express;
      data.port                         = tcam->d.egress.data.port;
      data.tagMatchBitmap               = tcam->d.egress.data.tagMatchBitmap;
      data.bonusData                    = tcam->d.egress.data.bonusData;
      data.etherType                    = tcam->d.egress.data.etherType;
      data.innerVlanType                = tcam->d.egress.data.innerVlanType;
      data.numTags                      = tcam->d.egress.data.numTags;
      data.outerPriority                = tcam->d.egress.data.outerPriority;
      data.outerTagId                   = tcam->d.egress.data.outerTagId;
      data.outerVlanType                = tcam->d.egress.data.outerVlanType;
      data.packetType                   = tcam->d.egress.data.packetType;
      data.secondOuterPriority          = tcam->d.egress.data.secondOuterPriority;
      data.secondOuterTagId             = tcam->d.egress.data.secondOuterTagId;
      data.macDa                        = tcam->d.egress.data.macDa;
      data.macSa                        = tcam->d.egress.data.macSa;

      mask.maskExpress                  = tcam->d.egress.mask.express;
      mask.maskPort                     = tcam->d.egress.mask.port;
      mask.maskTagMatchBitmap           = tcam->d.egress.mask.tagMatchBitmap;
      mask.maskBonusData                = tcam->d.egress.mask.bonusData;
      mask.maskEtherType                = tcam->d.egress.mask.etherType;
      mask.maskInnerVlanType            = tcam->d.egress.mask.innerVlanType;
      mask.maskNumTags                  = tcam->d.egress.mask.numTags;
      mask.maskOuterPriority            = tcam->d.egress.mask.outerPriority;
      mask.maskOuterTagId               = tcam->d.egress.mask.outerTagId;
      mask.maskOuterVlanType            = tcam->d.egress.mask.outerVlanType;
      mask.maskPacketType               = tcam->d.egress.mask.packetType;
      mask.maskSecondOuterPriority      = tcam->d.egress.mask.secondOuterPriority;
      mask.maskSecondOuterTagId         = tcam->d.egress.mask.secondOuterTagId;
      mask.maskMacDa                    = tcam->d.egress.mask.macDa;
      mask.maskMacSa                    = tcam->d.egress.mask.macSa;

      ret = writeEgressFlowIdTcam( rmsDev_p, index, &data, &mask );
      DBGRETURN(ret);

      ret = setTxSecyMap( rmsDev_p, index, tcam->secY, tcam->d.egress.sc, tcam->controlPacket, tcam->d.egress.auxiliaryPolicy );
    }

    return ret;
}


int MacsecEnableTcam( RmsDev_t * rmsDev_p, MacsecDirection_t direction, unsigned index, bool enable )
{
    int      ret = -EINVAL;
    unsigned bit;
    unsigned regIdx;

    if( rmsDev_p == NULL ) {
        return -EINVAL;
    }
    if( direction == MACSEC_INGRESS ) {
        Ra01RsMcsCpmRxRxFlowidTcamEnable_t en;

        if( index > INGRESS_FLOW_ID_TCAM_TABLE_SIZE ) {
            return -EINVAL;
        }
        regIdx = index / (sizeof(ra01_register_t)*8);
        bit = index % (sizeof(ra01_register_t)*8);

        ret = Ra01AccRsMcsCpmRxRxFlowidTcamEnable( rmsDev_p, regIdx, &en , RA01_READ_OP);
        DBGRETURN(ret);

        if( enable ){
            en.enable |= 1 << bit;
        } else {
            en.enable &= ~(1 << bit);
        }

        ret = Ra01AccRsMcsCpmRxRxFlowidTcamEnable( rmsDev_p, regIdx, &en , RA01_WRITE_OP);
    } else if (direction == MACSEC_EGRESS ) {
        Ra01RsMcsCpmTxTxFlowidTcamEnable_t en;
        if( index > EGRESS_FLOW_ID_TCAM_TABLE_SIZE ) {
            return -EINVAL;
        }
        regIdx = index / (sizeof(ra01_register_t)*8);
        bit = index % (sizeof(ra01_register_t)*8);

        ret = Ra01AccRsMcsCpmTxTxFlowidTcamEnable( rmsDev_p, regIdx, &en , RA01_READ_OP);
        DBGRETURN(ret);

        if( enable ){
            en.enable |= 1 << bit;
        } else {
            en.enable &= ~(1 << bit);
        }

        ret = Ra01AccRsMcsCpmTxTxFlowidTcamEnable( rmsDev_p, regIdx, &en , RA01_WRITE_OP);
    }

    return ret;
}


//------------------------------------------------------------------------------
// SecY section
//------------------------------------------------------------------------------
int MacsecReadSecyPolicy( RmsDev_t *                rmsDev_p,
                          MacsecDirection_t         direction,
                          unsigned                  index,
                          MacsecSecyPolicyTable_t * policy )
{
    int ret = 0;

    if( (rmsDev_p == NULL) || (policy == NULL) ) {
        return -EINVAL;
    }

    if( direction == MACSEC_INGRESS ) {
        ret = Ra01ReadRsMcsCpmRxSecyPlcy( rmsDev_p, index, &(policy->ingress) );
        DBGRETURN(ret);
    } else {
        ret = Ra01ReadRsMcsCpmTxSecyPlcy( rmsDev_p, index, &(policy->egress) );
        DBGRETURN(ret);
    }

    return ret;
}

int MacsecWriteSecyPolicy( RmsDev_t *                      rmsDev_p,
                           MacsecDirection_t               direction,
                           unsigned                        index,
                           MacsecSecyPolicyTable_t *       policy )
{
    int ret = 0;

    if( (rmsDev_p == NULL) || (policy == NULL) ) {
        return -EINVAL;
    }

    if( direction == MACSEC_INGRESS ) {
        ret = Ra01WriteRsMcsCpmRxSecyPlcy( rmsDev_p, index, &(policy->ingress) );
        DBGRETURN(ret);
    } else {
        ret = Ra01WriteRsMcsCpmTxSecyPlcy( rmsDev_p, index, &(policy->egress) );
        DBGRETURN(ret);
    }

    return ret;
}

//------------------------------------------------------------------------------
// SC section
//------------------------------------------------------------------------------
int MacsecGetIngressScCamSci( RmsDev_t * rmsDev_p, unsigned index, unsigned * secY, uint64_t * sci, bool * enable )
{
    int               ret = 0;
    Ra01IfRsMcsCpmRxScCam_t scCam;

    if( rmsDev_p == NULL ) {
        return -EINVAL;
    }

    ret = Ra01ReadRsMcsCpmRxScCam( rmsDev_p, index, &scCam );
    DBGRETURN(ret);
    (*secY) = scCam.secy;
    (*sci)  = scCam.sci;

    ret = MacsecGetEnableSc( rmsDev_p, index, enable );

    return ret;
}

int MacsecSetIngressScCamSci( RmsDev_t * rmsDev_p, unsigned index, unsigned secY, uint64_t sci, bool enable )
{
    int               ret = 0;
    Ra01IfRsMcsCpmRxScCam_t scCam;

    if( rmsDev_p == NULL ) {
        return -EINVAL;
    }

#if RA01_RS_MCS_CPM_RX_SECY_PLCY_MEM_SIZE <= 256
    scCam.secy = (uint8_t)secY;
#else
    scCam.secy = (uint16_t)secY;
#endif
    scCam.sci = sci;

    ret = Ra01WriteRsMcsCpmRxScCam( rmsDev_p, index, &scCam );
    DBGRETURN(ret);

    ret = MacsecSetEnableSc( rmsDev_p, index, enable );

    return ret;
}
//------------------------------------------------------------------------------
// PEX
//------------------------------------------------------------------------------

int MacsecSetPacketFilter( RmsDev_t * rmsDev_p, MacsecDirection_t direction, MacsecPacketFilter_t * filter )
{
    int                         ret = 0;
    Ra01RsMcsPexPexConfiguration_t    pexCfg;
    Ra01RsMcsPexCtlPktRuleEnable_t    ruleEnOuter;
    Ra01RsMcsPexInstance_t            instance;
    unsigned                    i;

    if( (rmsDev_p == NULL) || (filter == NULL) ) {
        return -EINVAL;
    }

    if (direction == MACSEC_INGRESS) {
        instance = RA01_RS_MCS_PEX_RX_SLAVE;
    } else {
      instance = RA01_RS_MCS_PEX_TX_SLAVE;
    }
    // Set the PEX_CONFIGURATION register
    //
    pexCfg.nonDixErr = filter->nonDixErrEn;
    pexCfg.vlanAfterCustom = filter->vlanAfterCustomEn;
    ret = Ra01AccRsMcsPexPexConfiguration( rmsDev_p, instance, &pexCfg , RA01_WRITE_OP);
    DBGRETURN(ret);

    // Set all the rules associated with control packet matching

    ruleEnOuter.ctlPktRuleEtypeEn = 0;
    for( i = 0; i < MACSEC_FILTER_ETYPE_RULES; i++ ) {
        Ra01RsMcsPexCtlPktRuleEtype_t re;
        re.ruleEtype = filter->ctrl_rules.etype[i].etype;
        ret = Ra01AccRsMcsPexCtlPktRuleEtype( rmsDev_p, instance, i, &re , RA01_WRITE_OP);
        DBGRETURN(ret);
        ruleEnOuter.ctlPktRuleEtypeEn      |= filter->ctrl_rules.etype[i].enOuter << i;
    }

    ruleEnOuter.ctlPktRuleDaEn = 0;
    for( i = 0; i < MACSEC_FILTER_DA_RULES; i++ ) {
        Ra01RsMcsPexCtlPktRuleDa_t rda;

        rda.ruleDaLsb = filter->ctrl_rules.da[i].da & 0xffffffff;
        rda.ruleDaMsb = (filter->ctrl_rules.da[i].da >> 32u) & 0xffff;
        ret = Ra01AccRsMcsPexCtlPktRuleDa( rmsDev_p, instance, i, &rda , RA01_WRITE_OP);
        DBGRETURN(ret);
        ruleEnOuter.ctlPktRuleDaEn      |= filter->ctrl_rules.da[i].enOuter << i;
    }

    ruleEnOuter.ctlPktRuleDaRangeEn = 0;
    for( i = 0; i < MACSEC_FILTER_DA_RANGE_RULES; i++ ) {
        Ra01RsMcsPexCtlPktRuleDaRangeMin_t min;
        Ra01RsMcsPexCtlPktRuleDaRangeMax_t max;

        min.ruleRangeMinMsb = (filter->ctrl_rules.daRange[i].minDa >> 32) & 0xffff;
        min.ruleRangeMinLsb = filter->ctrl_rules.daRange[i].minDa & 0xffffffff;
        max.ruleRangeMaxMsb = (filter->ctrl_rules.daRange[i].maxDa >> 32) & 0xffff;
        max.ruleRangeMaxLsb = filter->ctrl_rules.daRange[i].maxDa & 0xffffffff;
        ret = Ra01AccRsMcsPexCtlPktRuleDaRangeMin( rmsDev_p, instance, i, &min , RA01_WRITE_OP);
        DBGRETURN(ret);
        ret = Ra01AccRsMcsPexCtlPktRuleDaRangeMax( rmsDev_p, instance, i, &max , RA01_WRITE_OP);
        DBGRETURN(ret);
        ruleEnOuter.ctlPktRuleDaRangeEn      |= filter->ctrl_rules.daRange[i].enOuter << i;
    }

    ruleEnOuter.ctlPktRuleComboEn = 0;
    for( i = 0; i < MACSEC_FILTER_COMBO_RULES; i++ ) {
        Ra01RsMcsPexCtlPktRuleComboMin_t min;
        Ra01RsMcsPexCtlPktRuleComboMax_t max;
        Ra01RsMcsPexCtlPktRuleComboEt_t  et;

        min.ruleComboMinMsb = (filter->ctrl_rules.comboRule[i].minDa >> 32) & 0xffff;
        min.ruleComboMinLsb = filter->ctrl_rules.comboRule[i].minDa & 0xffffffff;
        max.ruleComboMaxMsb = (filter->ctrl_rules.comboRule[i].maxDa >> 32) & 0xffff;
        max.ruleComboMaxLsb = filter->ctrl_rules.comboRule[i].maxDa & 0xffffffff;
        et.ruleComboEt = filter->ctrl_rules.comboRule[i].et;
        ret = Ra01AccRsMcsPexCtlPktRuleComboMin( rmsDev_p, instance, i, &min , RA01_WRITE_OP);
        DBGRETURN(ret);
        ret = Ra01AccRsMcsPexCtlPktRuleComboMax( rmsDev_p, instance, i, &max , RA01_WRITE_OP);
        DBGRETURN(ret);
        ret = Ra01AccRsMcsPexCtlPktRuleComboEt( rmsDev_p, instance, i, &et , RA01_WRITE_OP);
        DBGRETURN(ret);

        ruleEnOuter.ctlPktRuleComboEn      |= filter->ctrl_rules.comboRule[i].enOuter << i;
    }

    Ra01RsMcsPexCtlPktRuleDaPrefix_t mac;
    mac.ruleDaPrefixLsb = filter->ctrl_rules.mac.mac & 0xffffffff;
    mac.ruleDaPrefixMsb = (filter->ctrl_rules.mac.mac >> 32) & 0xffff;
    ret = Ra01AccRsMcsPexCtlPktRuleDaPrefix( rmsDev_p, instance, &mac , RA01_WRITE_OP);
    DBGRETURN(ret);
    ruleEnOuter.ctlPktRuleMacEn      = filter->ctrl_rules.mac.enOuter;

    // Set enables
    ret = Ra01AccRsMcsPexCtlPktRuleEnable( rmsDev_p, instance, 0, &ruleEnOuter , RA01_WRITE_OP);
    DBGRETURN(ret);

    // Set all the rules for e-type matching

    for( i = 0; i < MACSEC_VLAN_TAGS; i++ ) {
        Ra01RsMcsPexVlanCfg_t vlanCfg;
        memset(&vlanCfg, 0, sizeof(Ra01RsMcsPexVlanCfg_t));
        vlanCfg.vlanEn    = filter->vlanTag[i].vlanEn;
        vlanCfg.vlanEtype = filter->vlanTag[i].vlanEtype;
        vlanCfg.vlanIndx  = filter->vlanTag[i].vlanIndx;
        vlanCfg.vlanSize  = filter->vlanTag[i].vlanSize;
        vlanCfg.isVlan  = filter->vlanTag[i].isVlan;
        vlanCfg.vlanBonus  = filter->vlanTag[i].vlanBonus;
        ret = Ra01AccRsMcsPexVlanCfg( rmsDev_p, instance, i, &vlanCfg , RA01_WRITE_OP);
        DBGRETURN(ret);
    }

    for( i = 0; i < MACSEC_MPLS_TAGS; i++ ) {
       Ra01RsMcsPexMplsCfg_t   mplsCfg;
       memset(&mplsCfg, 0, sizeof(Ra01RsMcsPexMplsCfg_t));
       mplsCfg.mplsEtype = filter->mplsTag[i].etype;
       mplsCfg.mplsEn	 = filter->mplsTag[i].enable;
       ret = Ra01AccRsMcsPexMplsCfg( rmsDev_p, instance, i, &mplsCfg , RA01_WRITE_OP);
       DBGRETURN(ret);
    }

    Ra01RsMcsPexSectagCfg_t sectagCfg;
    memset(&sectagCfg, 0, sizeof(Ra01RsMcsPexSectagCfg_t));
    sectagCfg.sectagEn = filter->sectag.enable;
    sectagCfg.sectagEtype = filter->sectag.etype;
    ret = Ra01AccRsMcsPexSectagCfg( rmsDev_p, instance,0, &sectagCfg , RA01_WRITE_OP);
    DBGRETURN(ret);

    Ra01RsMcsPexIpv4Cfg_t ipv4Cfg;
    memset(&ipv4Cfg, 0, sizeof(Ra01RsMcsPexIpv4Cfg_t));
    ipv4Cfg.ipv4En = filter->ipv4.enable;
    ipv4Cfg.ipv4Etype = filter->ipv4.etype;
    ret = Ra01AccRsMcsPexIpv4Cfg( rmsDev_p, instance, &ipv4Cfg , RA01_WRITE_OP);
    DBGRETURN(ret);

    Ra01RsMcsPexIpv6Cfg_t ipv6Cfg;
    memset(&ipv6Cfg, 0, sizeof(Ra01RsMcsPexIpv6Cfg_t));
    ipv6Cfg.ipv6En = filter->ipv6.enable;
    ipv6Cfg.ipv6Etype = filter->ipv6.etype;
    ret = Ra01AccRsMcsPexIpv6Cfg( rmsDev_p, instance, &ipv6Cfg , RA01_WRITE_OP);
    DBGRETURN(ret);

    Ra01RsMcsPexMcsHeaderCfg_t optHdr;
    optHdr.rxMcsHdrEn	= filter->optRxMcsHeader.enable;
    optHdr.rxMcsHeader	= filter->optRxMcsHeader.etype;
    optHdr.txMcsHdrEn	= filter->optTxMcsHeader.enable;
    optHdr.txMcsHeader	= filter->optTxMcsHeader.etype;
    ret = Ra01AccRsMcsPexMcsHeaderCfg( rmsDev_p, instance, &optHdr , RA01_WRITE_OP);
    DBGRETURN(ret);

    Ra01RsMcsPexUdpCfg_t udpCfg;
    udpCfg.udpEn      = filter->udp.enable;
    udpCfg.udpProtNum = filter->udp.udpProto;
    ret = Ra01AccRsMcsPexUdpCfg( rmsDev_p, instance, &udpCfg , RA01_WRITE_OP);
    DBGRETURN(ret);

    for (i = 0; i < MACSEC_NUM_PTP_RULES; i++) {
      Ra01RsMcsPexPtpCfg_t ptpCfg;
      ptpCfg.ptpEn   = filter->ptp[i].enable;
      ptpCfg.ptpPort = filter->ptp[i].ptpPort;
      ret = Ra01AccRsMcsPexPtpCfg( rmsDev_p, instance, i, &ptpCfg , RA01_WRITE_OP);
      DBGRETURN(ret);
    }

    for( i = 0; i < MACSEC_PORT_COUNT; i++ ) {
        if( filter->customTagSelect[i].enable ) {
            Ra01RsMcsPexPortConfig_t ct;
            ct.vlanTagRelModeSel = filter->customTagSelect[i].select;
            ct.preamble = 0;
            ret = Ra01AccRsMcsPexPortConfig( rmsDev_p, instance, i, &ct , RA01_WRITE_OP);
            DBGRETURN(ret);
        }
    }

    return ret;
}

int MacsecGetPacketFilter( RmsDev_t * rmsDev_p, MacsecDirection_t direction, MacsecPacketFilter_t * filter )
{
    int                               ret = 0;
    Ra01RsMcsPexPexConfiguration_t    pexCfg;
    Ra01RsMcsPexCtlPktRuleEnable_t    ruleEnOuter;
    unsigned                          i;
    Ra01RsMcsPexInstance_t            instance;

    if( (rmsDev_p == NULL) || (filter == NULL) ) {
        return -EINVAL;
    }
    if (direction == MACSEC_INGRESS) {
        instance = RA01_RS_MCS_PEX_RX_SLAVE;
    } else {
        instance = RA01_RS_MCS_PEX_TX_SLAVE;
    }

    ret = Ra01AccRsMcsPexPexConfiguration( rmsDev_p, instance, &pexCfg , RA01_READ_OP);
    DBGRETURN(ret);
    filter->nonDixErrEn = pexCfg.nonDixErr;
    filter->vlanAfterCustomEn = pexCfg.vlanAfterCustom;

    // Get enables
    ret = Ra01AccRsMcsPexCtlPktRuleEnable( rmsDev_p, instance, 0, &ruleEnOuter , RA01_READ_OP);
    DBGRETURN(ret);

    for( i = 0; i < MACSEC_FILTER_ETYPE_RULES; i++ ) {
        Ra01RsMcsPexCtlPktRuleEtype_t re;
        ret = Ra01AccRsMcsPexCtlPktRuleEtype( rmsDev_p, instance, i, &re , RA01_READ_OP);
        DBGRETURN(ret);
        filter->ctrl_rules.etype[i].etype = re.ruleEtype;
        filter->ctrl_rules.etype[i].enOuter = (!!(ruleEnOuter.ctlPktRuleEtypeEn      & (1 << i)))?true:false;
        filter->ctrl_rules.etype[i].enInner = 0;
    }
    for( i = 0; i < MACSEC_FILTER_DA_RULES; i++ ) {
        Ra01RsMcsPexCtlPktRuleDa_t rda;
        ret = Ra01AccRsMcsPexCtlPktRuleDa( rmsDev_p, instance, i, &rda , RA01_READ_OP);
        DBGRETURN(ret);
        filter->ctrl_rules.da[i].da      = ((uint64_t)rda.ruleDaMsb << (uint64_t)32) | (uint64_t)rda.ruleDaLsb;
        filter->ctrl_rules.da[i].enOuter = (!!(ruleEnOuter.ctlPktRuleDaEn      & (1 << i)))?true:false;
        filter->ctrl_rules.da[i].enInner = 0;
    }
    for( i = 0; i < MACSEC_FILTER_DA_RANGE_RULES; i++ ) {
        Ra01RsMcsPexCtlPktRuleDaRangeMin_t min;
        Ra01RsMcsPexCtlPktRuleDaRangeMax_t max;

        ret = Ra01AccRsMcsPexCtlPktRuleDaRangeMin( rmsDev_p, instance, i, &min , RA01_READ_OP);
        DBGRETURN(ret);
        ret = Ra01AccRsMcsPexCtlPktRuleDaRangeMax( rmsDev_p, instance, i, &max , RA01_READ_OP);
        DBGRETURN(ret);

        filter->ctrl_rules.daRange[i].minDa = ((uint64_t)(min.ruleRangeMinMsb) << (uint64_t)32) | (uint64_t)min.ruleRangeMinLsb;
        filter->ctrl_rules.daRange[i].maxDa = ((uint64_t)(max.ruleRangeMaxMsb) << (uint64_t)32) | (uint64_t)max.ruleRangeMaxLsb;
        filter->ctrl_rules.daRange[i].enOuter = ( !!( ruleEnOuter.ctlPktRuleDaRangeEn      & (1 << i)) )?true:false;
        filter->ctrl_rules.daRange[i].enInner = 0;
    }
    for( i = 0; i < MACSEC_FILTER_COMBO_RULES; i++ ) {
        Ra01RsMcsPexCtlPktRuleComboMin_t min;
        Ra01RsMcsPexCtlPktRuleComboMax_t max;
        Ra01RsMcsPexCtlPktRuleComboEt_t  et;

        ret = Ra01AccRsMcsPexCtlPktRuleComboMin( rmsDev_p, instance, i, &min , RA01_READ_OP);
        DBGRETURN(ret);
        ret = Ra01AccRsMcsPexCtlPktRuleComboMax( rmsDev_p, instance, i, &max , RA01_READ_OP);
        DBGRETURN(ret);
        ret = Ra01AccRsMcsPexCtlPktRuleComboEt( rmsDev_p, instance, i, &et , RA01_READ_OP);
        DBGRETURN(ret);
        filter->ctrl_rules.comboRule[i].minDa = ((uint64_t)(min.ruleComboMinMsb) << (uint64_t)32) | (uint64_t)min.ruleComboMinLsb;
        filter->ctrl_rules.comboRule[i].maxDa = ((uint64_t)(max.ruleComboMaxMsb) << (uint64_t)32) | (uint64_t)max.ruleComboMaxLsb;
        filter->ctrl_rules.comboRule[i].et = et.ruleComboEt;
        filter->ctrl_rules.comboRule[i].enOuter = ( !!( ruleEnOuter.ctlPktRuleComboEn      & (1 << i)) )?true:false;
        filter->ctrl_rules.comboRule[i].enInner = 0;
    }

    Ra01RsMcsPexCtlPktRuleDaPrefix_t mac;
    ret = Ra01AccRsMcsPexCtlPktRuleDaPrefix( rmsDev_p, instance, &mac , RA01_READ_OP);
    DBGRETURN(ret);
    filter->ctrl_rules.mac.mac = 0;
    filter->ctrl_rules.mac.mac |= (uint64_t)mac.ruleDaPrefixLsb & (uint64_t)0xffffffff;
    filter->ctrl_rules.mac.mac |= ((uint64_t)mac.ruleDaPrefixMsb & 0xffff) << 32;
    filter->ctrl_rules.mac.enOuter = ruleEnOuter.ctlPktRuleMacEn;
    filter->ctrl_rules.mac.enInner = 0;

    Ra01RsMcsPexVlanCfg_t vlanCfg;
    for( i = 0; i < MACSEC_VLAN_TAGS; i++ ) {
        ret = Ra01AccRsMcsPexVlanCfg( rmsDev_p, instance, i, &vlanCfg , RA01_READ_OP);
        DBGRETURN(ret);
        filter->vlanTag[i].vlanEn    = vlanCfg.vlanEn;
        filter->vlanTag[i].vlanEtype = vlanCfg.vlanEtype;
        filter->vlanTag[i].vlanIndx  = vlanCfg.vlanIndx ;
        filter->vlanTag[i].vlanSize  = vlanCfg.vlanSize;
        filter->vlanTag[i].isVlan  = vlanCfg.isVlan;
        filter->vlanTag[i].vlanBonus  = vlanCfg.vlanBonus;
    }

    Ra01RsMcsPexMplsCfg_t mplsCfg;
    for( i = 0; i < MACSEC_MPLS_TAGS; i++ ) {
        ret = Ra01AccRsMcsPexMplsCfg( rmsDev_p, instance, i, &mplsCfg , RA01_READ_OP);
        DBGRETURN(ret);
        filter->mplsTag[i].etype = mplsCfg.mplsEtype;
        filter->mplsTag[i].enable = mplsCfg.mplsEn;
    }

    Ra01RsMcsPexSectagCfg_t sectagCfg;
    ret = Ra01AccRsMcsPexSectagCfg( rmsDev_p, instance, 0, &sectagCfg , RA01_READ_OP);
    DBGRETURN(ret);
    filter->sectag.etype = sectagCfg.sectagEtype;
    filter->sectag.enable = sectagCfg.sectagEn?true:false;

    Ra01RsMcsPexIpv4Cfg_t ipv4Cfg;
    ret = Ra01AccRsMcsPexIpv4Cfg( rmsDev_p, instance, &ipv4Cfg , RA01_READ_OP);
    DBGRETURN(ret);
    filter->ipv4.etype = ipv4Cfg.ipv4Etype;
    filter->ipv4.enable = ipv4Cfg.ipv4En?true:false;

    Ra01RsMcsPexIpv6Cfg_t ipv6Cfg;
    ret = Ra01AccRsMcsPexIpv6Cfg( rmsDev_p, instance, &ipv6Cfg , RA01_READ_OP);
    DBGRETURN(ret);
    filter->ipv6.etype = ipv6Cfg.ipv6Etype;
    filter->ipv6.enable = ipv6Cfg.ipv6En?true:false;

    Ra01RsMcsPexMcsHeaderCfg_t optHdr;
    ret = Ra01AccRsMcsPexMcsHeaderCfg( rmsDev_p, instance, &optHdr , RA01_READ_OP);
    DBGRETURN(ret);
    filter->optRxMcsHeader.etype = optHdr.rxMcsHeader;
    filter->optTxMcsHeader.etype = optHdr.txMcsHeader;
    filter->optRxMcsHeader.enable = (optHdr.rxMcsHdrEn == 1)?true:false;
    filter->optTxMcsHeader.enable = (optHdr.txMcsHdrEn == 1)?true:false;

    Ra01RsMcsPexUdpCfg_t udpCfg;
    ret = Ra01AccRsMcsPexUdpCfg( rmsDev_p, instance, &udpCfg , RA01_READ_OP);
    DBGRETURN(ret);
    filter->udp.enable  = udpCfg.udpEn;
    filter->udp.udpProto = udpCfg.udpProtNum;

    for (i = 0; i < MACSEC_NUM_PTP_RULES; i++) {
      Ra01RsMcsPexPtpCfg_t ptpCfg;
      ret = Ra01AccRsMcsPexPtpCfg( rmsDev_p, instance, i, &ptpCfg , RA01_READ_OP);
      DBGRETURN(ret);
      filter->ptp[i].enable = ptpCfg.ptpEn;
      filter->ptp[i].ptpPort = ptpCfg.ptpPort;
    }

    for( i = 0; i < MACSEC_PORT_COUNT; i++ ) {
        Ra01RsMcsPexPortConfig_t ct;
        ret = Ra01AccRsMcsPexPortConfig( rmsDev_p, instance, i, &ct , RA01_READ_OP);
        DBGRETURN(ret);
        filter->customTagSelect[i].enable = true;
        filter->customTagSelect[i].select = ct.vlanTagRelModeSel;
    }
    return ret;
}
