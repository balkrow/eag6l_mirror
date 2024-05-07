/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* csRefMEF10.3Example.c
*
* DESCRIPTION:
*  This files provide infra functions to support all MEF related files.
*
*
* FILE REVISION NUMBER:
*       $Revision: 1.0 $
*
*******************************************************************************/

#include "csRefMEFUtils.h"
#include <cpss/generic/cpssTypes.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/cpssDxChHwInit.h>
#include <cpss/dxCh/dxChxGen/pcl/cpssDxChPcl.h>
#include <cpss/dxCh/dxChxGen/cnc/cpssDxChCnc.h>
#include <cpss/dxCh/dxChxGen/cos/cpssDxChCos.h>
#include "../../../../cpssEnabler/mainOs/h/gtOs/cpssOs.h"
#include <cpss/generic/policer/cpssGenPolicerTypes.h>
#include <cpss/dxCh/dxChxGen/policer/private/prvCpssDxChPolicerLog.h>
#include <cpss/dxCh/dxChxGen/policer/private/prvCpssDxChPolicer.h>


#define PCL_ID_CNS               0x10 /* Same PCL-Id for all */
#define PCL_RULE_FORMAT_CNS   CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_10_E
#define PCL_RULE_SIZE_CNS     1 /* rule size 10 Bytes */

/* CNC           */
#define PCL_LOOKUP_0_PARALLEL_0_CNC_BLOCK_NUM 3
#define PCL_LOOKUP_0_PARALLEL_1_CNC_BLOCK_NUM 4

#define QOS_TRUST_MODE_TABLE_INDEX_CNS    0
#define QOS_TRUST_MODE_GREEN_PROFILE_CNS  100
#define QOS_TRUST_MODE_YELLOW_PROFILE_CNS 108

/****** Structure to store infromation for clean up **/
/* PCL UDB configuration */
typedef struct {
    GT_U32                               udbIndex;
    CPSS_DXCH_PCL_OFFSET_TYPE_ENT        offsetType;
    GT_U8                                offset;
    GT_U8                                byteMask;
}PCL_UDB_STC;

/************************** Global Variables   *************************/
static PCL_UDB_STC pcl_UdbInfo[] = {
    /* Bytes offset 0-1 in PCL Key are for PCL ID*/
    /* Bytes offset 2-3 in PCL Key are for vlan id */
    /* Bytes offset 4-9 packet's MAC DA. */
     {4 , CPSS_DXCH_PCL_OFFSET_L2_E, 0, 0xFF} /* [0:7]: MAC-DA[40-47]   */
    ,{5 , CPSS_DXCH_PCL_OFFSET_L2_E, 1, 0xFF} /* [0:7]: MAC-DA[32-39] */
    ,{6 , CPSS_DXCH_PCL_OFFSET_L2_E, 2, 0xFF} /* [0:7]: MAC-DA[24-31]   */
    ,{7 , CPSS_DXCH_PCL_OFFSET_L2_E, 3, 0xFF} /* [0:7]: MAC-DA[16-23]   */
    ,{8 , CPSS_DXCH_PCL_OFFSET_L2_E, 4, 0xFF} /* [0:7]: MAC-DA[8-15]   */
    ,{9 , CPSS_DXCH_PCL_OFFSET_L2_E, 5, 0xFF} /* [0:7]: MAC-DA[0-7]   */
    /* must be last */
    ,{0, CPSS_DXCH_PCL_OFFSET_INVALID_E , 0, 0}
};

static GT_U32 csRefMEFUtilNumberOfPclRules = 0;          /* PCL rules entries. */
static GT_U32 csRefMEFUtilPclRulesBase;


/************************** Function Declarations   *************************/
GT_U32 appDemoDxChTcamIpclBaseIndexGet(IN GT_U8  devNum, IN GT_U32 lookupId);


/****************************************************************************/
/************************** PCL Functions *************************/
/****************************************************************************/

/**
* @internal csRefMefUtilQosTrustModeColorConfig function
* @endinternal
*
* @brief   This function sets the qos configuration globally and per the ingress port.
*
* @note   APPLICABLE DEVICES: Was tested for Aldrin-2XL
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum           - device number.
* @param[in] ingressPort      - the ingress port the apply the qos configurations.
*
* @retval GT_OK                      - on success
* @retval GT_BAD_PARAM          - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*
*/
GT_STATUS csRefMefUtilQosTrustModeColorConfig
(
  IN GT_U8        devNum,
  IN GT_PORT_NUM  ingressPort
)
{
    GT_STATUS  rc = GT_OK;
    GT_U32     up;
    CPSS_DXCH_COS_PROFILE_STC cosProfile;
    CPSS_QOS_ENTRY_STC        cpssQosCfg;

    CPSS_CALL(cpssDxChCosPortQosTrustModeSet (devNum, ingressPort, CPSS_QOS_PORT_TRUST_L2_E));
    CPSS_CALL(cpssDxChCosPortTrustQosMappingTableIndexSet (devNum, ingressPort, GT_FALSE, QOS_TRUST_MODE_TABLE_INDEX_CNS));

    /*  Green and yellow assignment profile for color aware mode. profile assgined based on CFI bit only. */
    for(up=0 ; up<7 ; up++)
    {
      CPSS_CALL(cpssDxChCosUpCfiDeiToProfileMapSet(devNum, QOS_TRUST_MODE_TABLE_INDEX_CNS, 0, up, 0/*cfi*/, QOS_TRUST_MODE_GREEN_PROFILE_CNS));
      CPSS_CALL(cpssDxChCosUpCfiDeiToProfileMapSet(devNum, QOS_TRUST_MODE_TABLE_INDEX_CNS, 0, up, 1/*cfi*/, QOS_TRUST_MODE_YELLOW_PROFILE_CNS));
    }

    /* The port's default QoSProfile and may be assigned to the packet as part of the QoS assignment algorithm. */
    osMemSet(&cpssQosCfg, 0, sizeof(cpssQosCfg));
    cpssQosCfg.qosProfileId = QOS_TRUST_MODE_GREEN_PROFILE_CNS;
    cpssQosCfg.assignPrecedence = CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_SOFT_E;
    cpssQosCfg.enableModifyUp = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
    cpssQosCfg.enableModifyDscp = CPSS_PACKET_ATTRIBUTE_MODIFY_DISABLE_E;
    CPSS_CALL(cpssDxChCosPortQosConfigSet (devNum, ingressPort, &cpssQosCfg));

    osMemSet(&cosProfile, 0, sizeof(cosProfile));
    cosProfile.dropPrecedence = CPSS_DP_GREEN_E;
    cosProfile.userPriority = 0;
    cosProfile.trafficClass = 0;
    CPSS_CALL(cpssDxChCosProfileEntrySet (devNum, QOS_TRUST_MODE_GREEN_PROFILE_CNS, &cosProfile));

    osMemSet(&cosProfile, 0, sizeof(cosProfile));
    cosProfile.dropPrecedence = CPSS_DP_YELLOW_E;
    cosProfile.userPriority = 0;
    cosProfile.trafficClass = 0;
    CPSS_CALL(cpssDxChCosProfileEntrySet (devNum, QOS_TRUST_MODE_YELLOW_PROFILE_CNS, &cosProfile));

    return rc;
}


/**
* @internal csRefMefUtilPclInit function
* @endinternal
*
* @brief   This function sets the global IPCL conigurations.
*
* @note   APPLICABLE DEVICES: Was tested for Aldrin-2XL
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum           - device number.
*
* @retval GT_OK                      - on success
* @retval GT_BAD_PARAM          - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*
*/
GT_STATUS csRefMefUtilPclInit
(
  IN GT_U8  devNum
)
{
  GT_STATUS rc = GT_OK;
  GT_U32                              index;
  CPSS_PCL_LOOKUP_NUMBER_ENT          lookupNum = CPSS_PCL_LOOKUP_NUMBER_0_E;
  CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT  ruleFormat = PCL_RULE_FORMAT_CNS;
  CPSS_PCL_DIRECTION_ENT              direction = CPSS_PCL_DIRECTION_INGRESS_E;
  CPSS_DXCH_PCL_UDB_SELECT_STC        udbSelect;
  GT_U32                              udbSelectidx;
  CPSS_DXCH_PCL_CFG_TBL_ACCESS_MODE_STC  accMode =
  {
      CPSS_DXCH_PCL_CFG_TBL_ACCESS_LOCAL_PORT_E,
      CPSS_DXCH_PCL_CFG_TBL_MAX_DEV_PORTS_32_E,
      CPSS_DXCH_PCL_CFG_TBL_DEV_PORT_BASE0_E,
      CPSS_DXCH_PCL_CFG_TBL_ACCESS_LOCAL_PORT_E,
      CPSS_DXCH_PCL_CFG_TBL_MAX_DEV_PORTS_32_E,
      CPSS_DXCH_PCL_CFG_TBL_DEV_PORT_BASE0_E
  };
  CPSS_DXCH_PCL_PACKET_TYPE_ENT   packetType;

  osPrintf("--- csRefMefUtilPclInit(): Initialize global IPCL configurations.\n");

   /* Enable PCL globally */
  CPSS_CALL(cpssDxChPclIngressPolicyEnable(devNum, GT_TRUE));

  /* disable force PCL ID configuration for all TT packets, according to the VLAN assignment */
  CPSS_CALL(cpssDxCh3PclTunnelTermForceVlanModeEnableSet(devNum, GT_FALSE));

  /* Configure access mode */
  rc = cpssDxChPclCfgTblAccessModeSet(devNum, &accMode);
  if((rc != GT_OK) && (rc != GT_NOT_APPLICABLE_DEVICE))
  {
    return rc;
  }

  for( index = 0 ; index < CPSS_DXCH_PCL_PACKET_TYPE_LAST_E ; index++)
  {
    packetType = (CPSS_DXCH_PCL_PACKET_TYPE_ENT)index;
    if(packetType == CPSS_DXCH_PCL_PACKET_TYPE_IPV6_E)
    {
      continue;
    }

    /* UDB configuration */
    osMemSet(&udbSelect, 0, sizeof(CPSS_DXCH_PCL_UDB_SELECT_STC));
    udbSelectidx = 4; /* Bytes offset 0-1 in PCL Key are for PCL ID*/

    /* Per UDE packet type (0-7), set different offset to match next segment label (label is mapped to the IP address of segment end node). */
    while(1)
    {
      if(pcl_UdbInfo[udbSelectidx-4].offsetType == CPSS_DXCH_PCL_OFFSET_INVALID_E)
        break;
      CPSS_CALL(cpssDxChPclUserDefinedByteSet(devNum, ruleFormat, packetType, direction,
                                             pcl_UdbInfo[udbSelectidx-4].udbIndex,
                                             pcl_UdbInfo[udbSelectidx-4].offsetType,
                                             pcl_UdbInfo[udbSelectidx-4].offset));

      udbSelect.udbSelectArr[udbSelectidx]=pcl_UdbInfo[udbSelectidx-4].udbIndex;
      udbSelectidx++;
    }

    /* Configure PCL selectin for key used UDBs only. */
    /* Select byte order in the key */
    udbSelect.ingrUdbReplaceArr[0] = GT_TRUE; /*  udbReplaceArr[0]  - UDB0  replaced with {PCL-ID[7:0]} */
    udbSelect.ingrUdbReplaceArr[1] = GT_TRUE; /*  udbReplaceArr[1]  - UDB1  replaced with {UDB Valid,reserved,PCL-ID[9:8]} */
    udbSelect.ingrUdbReplaceArr[2] = GT_TRUE; /*  udbReplaceArr[2]  - UDB2  replaced with eVLAN[7:0]*/
    udbSelect.ingrUdbReplaceArr[3] = GT_TRUE; /*  udbReplaceArr[3]  - UDB3  replaced with eVLAN[12:8]*/
    CPSS_CALL(cpssDxChPclUserDefinedBytesSelectSet(devNum, ruleFormat, packetType, lookupNum, &udbSelect));

  }

  csRefMEFUtilPclRulesBase = csRefMEFUtilNumberOfPclRules = 1000;
  csRefMEFUtilPclRulesBase = appDemoDxChTcamIpclBaseIndexGet(devNum, 0); /* get base index for PCL first lookup. */

  return GT_OK;
}


/**
* @internal csRefMefUtilPClIngressPortInit function
* @endinternal
*
* @brief   This function sets the IPCL conigurations for a specific ingress port.
*
* @note   APPLICABLE DEVICES: Was tested for Aldrin-2XL
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum   - device number.
* @param[in] portNum  - the ingress port the apply the IPCL configurations.
*
* @retval GT_OK                      - on success
* @retval GT_BAD_PARAM          - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*
*/
GT_STATUS csRefMefUtilPClIngressPortInit
(
  IN GT_U8        devNum,
  IN GT_PORT_NUM  portNum
)
{
  GT_STATUS rc = GT_OK;
  CPSS_PCL_LOOKUP_NUMBER_ENT     lookupNum = CPSS_PCL_LOOKUP_NUMBER_0_E;
  CPSS_DXCH_PCL_LOOKUP_CFG_STC   lookupCfg; /* lookup configuration */
  CPSS_INTERFACE_INFO_STC        interfaceInfo; /* interface data: port, VLAN, or index */
  GT_HW_DEV_NUM                  hwDevNum;
  CPSS_PCL_DIRECTION_ENT         direction = CPSS_PCL_DIRECTION_INGRESS_E;


  osPrintf("--- csRefMefUtilPClIngressPortInit(): Initialize IPCL for port %d\n", portNum);
  CPSS_CALL(cpssDxChCfgHwDevNumGet(devNum, &hwDevNum));
  CPSS_CALL(csRefMefUtilQosTrustModeColorConfig(devNum, portNum));

  /* Enable PCL on ingress port. */
  CPSS_CALL(cpssDxChPclPortIngressPolicyEnable(devNum, portNum, GT_TRUE));
  CPSS_CALL(cpssDxChPclPortLookupCfgTabAccessModeSet(devNum, portNum, direction, lookupNum, 0, CPSS_DXCH_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E));

  osMemSet(&lookupCfg, 0, sizeof(CPSS_DXCH_PCL_LOOKUP_CFG_STC));
  osMemSet(&interfaceInfo, 0, sizeof(CPSS_INTERFACE_INFO_STC));

  lookupCfg.enableLookup = GT_TRUE;
  lookupCfg.pclId        = PCL_ID_CNS; /* Same PCL-ID*/
  lookupCfg.dualLookup   = GT_FALSE;
  lookupCfg.groupKeyTypes.nonIpKey = PCL_RULE_FORMAT_CNS;
  lookupCfg.groupKeyTypes.ipv4Key  = PCL_RULE_FORMAT_CNS;
  lookupCfg.groupKeyTypes.ipv6Key  = PCL_RULE_FORMAT_CNS;
  lookupCfg.udbKeyBitmapEnable     = GT_FALSE;
  interfaceInfo.type              = CPSS_INTERFACE_PORT_E;
  interfaceInfo.devPort.hwDevNum = 0;
  interfaceInfo.devPort.portNum  = portNum;
  CPSS_CALL(cpssDxChPclCfgTblSet(devNum, &interfaceInfo, direction, lookupNum, &lookupCfg));

  return GT_OK;
}


/**
* @internal csRefMefUtilPClRuleSet function
* @endinternal
*
* @brief   This function sets the ipcl rule parameters according to the user input.
*
* @note   APPLICABLE DEVICES: Was tested for Aldrin-2XL
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum       - device number.
* @param[in] daMacDa      - dmac address that the IPCL will catch for incoming packets.
* @param[in] pclEntryIndex      - the pcl index for this rule.
* @param[in] egressPortNum      - the egress port to redirect the packet to, in case of a match.
* @param[in] meterId      - the metering index to be assigned in the action, in case of a match.
* @param[in] pktCmd       - the packet command to be assigned in the action, in case of a match.
*
* @retval GT_OK                      - on success
* @retval GT_BAD_PARAM          - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*
*/
GT_STATUS csRefMefUtilPClRuleSet
(
  IN GT_U8               devNum,
  IN GT_ETHERADDR       *daMacDa,          /* Packet's MAC-DA . */
  IN GT_U32              pclEntryIndex,
  IN GT_PORT_NUM         egressPortNum,    /* Egress port */
  IN GT_U32              meterId,
  IN CPSS_PACKET_CMD_ENT pktCmd
)
{
  GT_STATUS rc = GT_OK;

  /* PBR configuration */
  GT_U32                              udbIndex, pclId = PCL_ID_CNS;
  CPSS_DXCH_PCL_RULE_FORMAT_TYPE_ENT  pclRuleFormat   = PCL_RULE_FORMAT_CNS;
  CPSS_DXCH_PCL_RULE_OPTION_ENT       ruleOptionsBmp  = CPSS_DXCH_PCL_RULE_OPTION_WRITE_DEFAULT_E;
  CPSS_DXCH_PCL_RULE_FORMAT_UNT       pclRuleMask;
  CPSS_DXCH_PCL_RULE_FORMAT_UNT       pclRulePattern;
  CPSS_DXCH_PCL_ACTION_STC            pclAction;
  GT_HW_DEV_NUM                       hwDevNum;

  CPSS_CALL(cpssDxChCfgHwDevNumGet(devNum, &hwDevNum));

  osMemSet(&pclRuleMask    , 0, sizeof(pclRuleMask));
  osMemSet(&pclRulePattern , 0, sizeof(pclRulePattern));
  osMemSet(&pclAction      , 0, sizeof(pclAction));

  if(csRefMEFUtilPclRulesBase + (pclEntryIndex*PCL_RULE_SIZE_CNS) >= csRefMEFUtilNumberOfPclRules)
  {
    return GT_NO_RESOURCE;
  }

  /* As assigned by PCL configuration entry, indexed from TTI action above. */
  /* UDB[0] and UDB[1] replaced by PCL-ID. */
  pclId = PCL_ID_CNS;
  pclRulePattern.ruleIngrUdbOnly.replacedFld.pclId = pclId;
  pclRuleMask.ruleIngrUdbOnly.replacedFld.pclId = 0x3FF;

  /* 4 bytes VXLAN Network Identifier (VNI), udbIndex  7-9*/
  /* Offset 12-14 from start of UDP header */
  /* Bytes offset 2-4 in PCL Key/Pattern/Mask. */
  udbIndex = 4;
  pclRulePattern.ruleIngrUdbOnly.udb[udbIndex] = daMacDa->arEther[0];
  pclRuleMask.ruleIngrUdbOnly.udb[udbIndex]    = 0xFF;

  udbIndex = 5;
  pclRulePattern.ruleIngrUdbOnly.udb[udbIndex] = daMacDa->arEther[1];
  pclRuleMask.ruleIngrUdbOnly.udb[udbIndex]    = 0xFF;

  udbIndex = 6;
  pclRulePattern.ruleIngrUdbOnly.udb[udbIndex] = daMacDa->arEther[2];
  pclRuleMask.ruleIngrUdbOnly.udb[udbIndex]    = 0xFF;

  udbIndex = 7;
  pclRulePattern.ruleIngrUdbOnly.udb[udbIndex] = daMacDa->arEther[3];
  pclRuleMask.ruleIngrUdbOnly.udb[udbIndex]    = 0xFF;

  udbIndex = 8;
  pclRulePattern.ruleIngrUdbOnly.udb[udbIndex] = daMacDa->arEther[4];
  pclRuleMask.ruleIngrUdbOnly.udb[udbIndex]    = 0xFF;

  udbIndex = 9;
  pclRulePattern.ruleIngrUdbOnly.udb[udbIndex] = daMacDa->arEther[5];
  pclRuleMask.ruleIngrUdbOnly.udb[udbIndex]    = 0xFF;

  pclAction.matchCounter.enableMatchCount  = GT_TRUE;
  pclAction.matchCounter.matchCounterIndex = pclEntryIndex;

  /* PCL Action */
  pclAction.pktCmd = pktCmd /*CPSS_PACKET_CMD_FORWARD_E*/;
  pclAction.mirror.cpuCode = CPSS_NET_IPV6_ROUTE_TRAP_E; /* For pktCmd=CPSS_PACKET_CMD_DROP_XXX_E case*/
  pclAction.egressPolicy = CPSS_PCL_DIRECTION_INGRESS_E;
  pclAction.bypassBridge = GT_TRUE;

  pclAction.redirect.redirectCmd = CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_OUT_IF_E;
  pclAction.redirect.data.outIf.outInterface.devPort.hwDevNum = hwDevNum;
  pclAction.redirect.data.outIf.outInterface.devPort.portNum  = egressPortNum;

  pclAction.policer.policerEnable = CPSS_DXCH_PCL_POLICER_ENABLE_METER_AND_COUNTER_E;
  pclAction.policer.policerId = meterId;

  osPrintf("--- csRefMefUtilPClRuleSet(): Setting a new IPCL rule:\n");
  osPrintf("    index: %d\n", pclEntryIndex*PCL_RULE_SIZE_CNS);
  osPrintf("    Matching MAC address: %02x:%02x:%02x:%02x:%02x:%02x\n", daMacDa->arEther[0], daMacDa->arEther[1], daMacDa->arEther[2],
              daMacDa->arEther[3], daMacDa->arEther[4],daMacDa->arEther[5]);
  osPrintf("    matchCounterIndex: %d\n", pclAction.matchCounter.matchCounterIndex); 
  osPrintf("    Redirect action to port: %d\n", egressPortNum);
  osPrintf("    Policer meter ID: %d\n\n", pclAction.policer.policerId);  

  CPSS_CALL(cpssDxChPclRuleSet(devNum, 0, pclRuleFormat, pclEntryIndex*PCL_RULE_SIZE_CNS, ruleOptionsBmp, &pclRuleMask, &pclRulePattern, &pclAction));
  if(rc != GT_OK)
  {
    return rc;
  }

  return GT_OK;
}


/**
* @internal csRefMefUtilQosHwPolicerInit function
* @endinternal
*
* @brief   This function sets the policer global configuration in the device. 
*
* @note   APPLICABLE DEVICES: Was tested for Aldrin-2XL
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum       - device number.
*
* @retval GT_OK                      - on success
* @retval GT_BAD_PARAM          - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*
*/
GT_STATUS csRefMefUtilQosHwPolicerInit
(
  IN GT_U8   devNum
)
{
    GT_STATUS    rc = GT_OK;

    osPrintf("--- csRefMefUtilQosHwPolicerInit(): Initialize policer configurations.\n");

    /*Setting the Policer Init */
    CPSS_CALL(cpssDxCh3PolicerMeteringEnableSet (devNum, CPSS_DXCH_POLICER_STAGE_INGRESS_0_E, GT_TRUE));

    /*include everything in the metering L1+L2+L3 */
    CPSS_CALL(cpssDxCh3PolicerPacketSizeModeSet(devNum, CPSS_DXCH_POLICER_STAGE_INGRESS_0_E, CPSS_POLICER_PACKET_SIZE_L2_INCLUDE_E));

    CPSS_CALL(cpssDxCh3PolicerPacketSizeModeForTunnelTermSet (devNum, CPSS_DXCH_POLICER_STAGE_INGRESS_0_E, CPSS_DXCH3_POLICER_TT_PACKET_SIZE_REGULAR_E));

    /*Set drop mode for packets */
    CPSS_CALL(cpssDxCh3PolicerDropTypeSet (devNum, CPSS_DXCH_POLICER_STAGE_INGRESS_0_E, CPSS_DROP_MODE_SOFT_E));
    CPSS_CALL(cpssDxChPolicerStageMeterModeSet (devNum, CPSS_DXCH_POLICER_STAGE_INGRESS_0_E, CPSS_DXCH_POLICER_STAGE_METER_MODE_FLOW_E));
    CPSS_CALL(cpssDxCh3PolicerCountingColorModeSet (devNum, CPSS_DXCH_POLICER_STAGE_INGRESS_0_E, CPSS_DXCH3_POLICER_COLOR_COUNT_CL_E));
    CPSS_CALL(cpssDxCh3PolicerManagementCntrsResolutionSet (devNum, CPSS_DXCH_POLICER_STAGE_INGRESS_0_E, CPSS_DXCH3_POLICER_MNG_CNTR_SET0_E, CPSS_DXCH3_POLICER_MNG_CNTR_RESOLUTION_1B_E));
    CPSS_CALL(cpssDxChPolicerCountingModeSet (devNum, CPSS_DXCH_POLICER_STAGE_INGRESS_0_E, CPSS_DXCH_POLICER_COUNTING_BILLING_IPFIX_E));

    return GT_OK;
}
