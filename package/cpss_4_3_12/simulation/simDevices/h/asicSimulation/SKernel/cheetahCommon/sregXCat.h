/******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* sregxCat.h
*
* DESCRIPTION:
*       Defines for xCat memory registers access.
*
* DEPENDENCIES:
*       None.
*
* FILE REVISION NUMBER:
*       $Revision: 68 $
*
*******************************************************************************/
#ifndef __sregXCath
#define __sregXCath

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Ingress Policer0,1 Interrupt Cause Register Summary bits */
#define SNET_XCAT_INGR_POLICER0_INTR_SUM_E(dev)  \
        (1 << 22)

#define SNET_XCAT_INGR_POLICER1_INTR_SUM_E(dev)  \
        (1 << 23)

/*
    get the base address of the policer units
    cycle 0 - iplr 0
    cycle 1 - iplr 1
    cycle 2 - eplr

*/
#define SMEM_XCAT_POLICER_BASE_ADDR_MAC(dev, cycle)      \
    dev->memUnitBaseAddrInfo.policer[cycle]

/*
    get the register address according to offset from the base address of the
    policer unit

    cycle 0 - iplr 0
    cycle 1 - iplr 1
    cycle 2 - eplr


    offset - register offset from start of the unit
*/
#define SMEM_XCAT_POLICER_OFFSET_MAC(dev, cycle , offset)      \
    (SMEM_XCAT_POLICER_BASE_ADDR_MAC(dev, cycle) + (offset))

/* Ingress global TPID */
#define SMEM_XCAT_INGR_GLOBAL_TAG_TPID_REG(dev)                      \
    (SMEM_CHT_IS_SIP5_GET(dev) ? SMEM_CHT_MAC_REG_DB_SIP5_GET(dev)->TTI.VLANAssignment.ingrTPIDConfig_[0] : \
    (SMEM_CHT_MAC_REG_DB_GET(dev))->TTI.VLANAssignment.ingrTPIDConfig[0])

/* Ingress Port Tag0/Tag1 TPID */
#define SMEM_XCAT_INGR_PORT_TPID_REG(dev)                            \
    (SMEM_CHT_IS_SIP5_GET(dev) ? SMAIN_NOT_VALID_CNS :            \
    (SMEM_CHT_MAC_REG_DB_GET(dev))->TTI.VLANAssignment.ingrTPIDSelect[0])

/* HA - Egress global TPID */
#define SMEM_XCAT_HA_GLOBAL_TAG_TPID_REG(dev,index)                      \
    (SMEM_CHT_IS_SIP5_GET(dev) ? SMEM_CHT_MAC_REG_DB_SIP5_GET(dev)->HA.TPIDConfigReg[index]  :            \
    (SMEM_CHT_MAC_REG_DB_GET(dev))->HA.TPIDConfigReg[index])

/* Egress Port Tag0 TPID */
#define SMEM_XCAT_TAG0_EGR_PORT_TPID_REG(dev)                       \
    (SMEM_CHT_IS_SIP5_GET(dev) ? SMAIN_NOT_VALID_CNS :            \
    (SMEM_CHT_MAC_REG_DB_GET(dev))->HA.portTag0TPIDSelect[0])

/* Egress Port Tag1 TPID */
#define SMEM_XCAT_TAG1_EGR_PORT_TPID_REG(dev)                       \
    (SMEM_CHT_IS_SIP5_GET(dev) ? SMAIN_NOT_VALID_CNS :            \
    (SMEM_CHT_MAC_REG_DB_GET(dev))->HA.portTag1TPIDSelect[0])

/* Header Alteration Global Configuration */
#define SMEM_XCAT_ROUTE_HA_GLB_CNF_REG(dev) \
    (SMEM_CHT_IS_SIP5_GET(dev) ? SMEM_CHT_MAC_REG_DB_SIP5_GET(dev)->HA.haGlobalConfig : \
    (SMEM_CHT_MAC_REG_DB_GET(dev))->HA.haGlobalConfig)

/* Header Alteration VLAN Translation Enable Register Address */
#define SMEM_XCAT_HA_VLAN_TRANS_EN_REG(dev)                        \
    (SMEM_CHT_IS_SIP5_GET(dev) ? SMAIN_NOT_VALID_CNS :            \
    (SMEM_CHT_MAC_REG_DB_GET(dev))->HA.haVLANTranslationEnable[0])


/* Header Alteration I Ethertype global configuration */
#define SMEM_XCAT_HA_I_ETHERTYPE_REG(dev)                           \
    (SMEM_CHT_IS_SIP5_GET(dev) ? SMEM_CHT_MAC_REG_DB_SIP5_GET(dev)->HA.IEtherType  :            \
    (SMEM_CHT_MAC_REG_DB_GET(dev))->HA.IEthertype)

/* Cascading and Header Insertion Configuration Register */
#define SMEM_XCAT_CAS_HDR_INS_REG(dev)                              \
    (SMEM_CHT_IS_SIP5_GET(dev) ? SMAIN_NOT_VALID_CNS : \
    (SMEM_CHT_MAC_REG_DB_GET(dev))->HA.cascadingAndHeaderInsertionConfig[0])

/* Router Header Alteration Enable MAC SA Modification Register */
#define SMEM_XCAT_ROUT_HA_MAC_SA_EN_REG(dev)                        \
    (SMEM_CHT_IS_SIP5_GET(dev) ? SMAIN_NOT_VALID_CNS : \
    (SMEM_CHT_MAC_REG_DB_GET(dev))->HA.routerHaEnableMACSAModification[0])

/* QoSProfile to DP Register<%n> */
#define SMEM_XCAT_HA_QOS_PROFILE_TO_DP_REG(dev)                    \
    (SMEM_CHT_IS_SIP5_GET(dev) ? SMAIN_NOT_VALID_CNS : \
    (SMEM_CHT_MAC_REG_DB_GET(dev))->HA.qoSProfileToDPReg[0])

/* Router MAC SA Base Register0 - xCat */
#define SMEM_XCAT_MAC_SA_BASE0_REG(dev)                             \
    (SMEM_CHT_IS_SIP5_GET(dev) ? SMEM_CHT_MAC_REG_DB_SIP5_GET(dev)->HA.routerMACSABaseReg0 : \
    (SMEM_CHT_MAC_REG_DB_GET(dev))->HA.routerMACSABaseReg0)

/* Router Header Alteration MAC SA Modification Mode */
#define SMEM_XCAT_MAC_SA_MODIFICATION_MODE_REG(dev)                 \
    (SMEM_CHT_IS_SIP5_GET(dev) ? SMAIN_NOT_VALID_CNS : \
    (SMEM_CHT_MAC_REG_DB_GET(dev))->HA.routerHaMACSAModificationMode[0])

/* Egress DSA Tag Type Configuration */
#define SMEM_XCAT_DSA_TAG_TYPE_REG(dev)                              \
    (SMEM_CHT_IS_SIP5_GET(dev) ? SMAIN_NOT_VALID_CNS : \
    (SMEM_CHT_MAC_REG_DB_GET(dev))->HA.egrDSATagTypeConfig[0])

/* L2 Port Isolation Table Entry%n */
#define SMEM_XCAT_L2_PORT_ISOLATION_REG(dev, index)  \
    SMEM_TABLE_ENTRY_INDEX_GET_MAC(dev, l2PortIsolation, index)

/* L3 Port Isolation Table Entry%n */
#define SMEM_XCAT_L3_PORT_ISOLATION_REG(dev, index)  \
    SMEM_TABLE_ENTRY_INDEX_GET_MAC(dev, l3PortIsolation, index)

/* Transmit Queue Resource Sharing */
#define SMEM_XCAT_TXQ_RESOURCE_SHARE_REG(dev)                       \
    (   \
      (dev)->txqRevision == 0 ?                                     \
        (SMEM_CHT_MAC_REG_DB_GET(dev))->egrAndTxqVer0.transmitQueueGlobalConfig.transmitQueueResourceSharing : \
        SMAIN_NOT_VALID_CNS)

/* Transmit Queue Extended Control1 */
#define SMEM_XCAT_TXQ_EXTENDED_CONTROL1_REG(dev)                    \
    (   \
      (dev)->txqRevision == 0 ?                                     \
        (SMEM_CHT_MAC_REG_DB_GET(dev))->egrAndTxqVer0.transmitQueueGlobalConfig.transmitQueueExtCtrl1 : \
        SMAIN_NOT_VALID_CNS)

/* TTI Internal, Metal Fix*/
#define SMEM_XCAT_TTI_INTERNAL_METAL_FIX_REG(dev)                   \
    (SMEM_CHT_MAC_REG_DB_GET(dev))->TTI.metalFix

/* Protocol based VLAN registers - config 0 */
#define SMEM_XCAT_TTI_PROT_ENCAP_CONF_REG(dev)                      \
    (SMEM_CHT_IS_SIP5_GET(dev) ? SMEM_CHT_MAC_REG_DB_SIP5_GET(dev)->TTI.protocolMatch.protocolsEncapsulationConfig0 : \
    (SMEM_CHT_MAC_REG_DB_GET(dev))->TTI.protocolMatch.protocolsEncapsulationConfig0)

/* Protocol based VLAN registers - config 1 */
#define SMEM_XCAT_TTI_PROT_ENCAP_CONF1_REG(dev)                      \
    (SMEM_CHT_IS_SIP5_GET(dev) ? SMEM_CHT_MAC_REG_DB_SIP5_GET(dev)->TTI.protocolMatch.protocolsEncapsulationConfig1 : \
    (SMEM_CHT_MAC_REG_DB_GET(dev))->TTI.protocolMatch.protocolsEncapsulationConfig1)

#define SMEM_XCAT_TTI_PROT_CONF_REG(dev)                            \
    (SMEM_CHT_IS_SIP5_GET(dev) ? SMEM_CHT_MAC_REG_DB_SIP5_GET(dev)->TTI.protocolMatch.protocolsConfig[0] : \
    (SMEM_CHT_MAC_REG_DB_GET(dev))->TTI.protocolMatch.protocolsConfig[0])

/* IPv6 Solicited-Node Multicast Address Configuration Register0 */
#define SMEM_XCAT_SOLIC_NODE_MCST_ADDR_REG(dev)                     \
    (SMEM_CHT_IS_SIP5_GET(dev) ? SMEM_CHT_MAC_REG_DB_SIP5_GET(dev)->TTI.IPv6SolicitedNode.IPv6SolicitedNodeMcAddrConfigReg0 : \
    (SMEM_CHT_MAC_REG_DB_GET(dev))->TTI.IPv6SolicitedNodeMcAddrConfigReg[0])

/* IPv6 Solicited-Node Multicast Address Mask Register0 */
#define SMEM_XCAT_SOLIC_NODE_MCST_MASK_REG(dev)                     \
    (SMEM_CHT_IS_SIP5_GET(dev) ? SMEM_CHT_MAC_REG_DB_SIP5_GET(dev)->TTI.IPv6SolicitedNode.IPv6SolicitedNodeMcAddrMaskReg0 : \
    (SMEM_CHT_MAC_REG_DB_GET(dev))->TTI.IPv6SolicitedNodeMcAddrMaskReg[0])

/* Port<n> Protocol<m> VID and QoS Configuration Entry */
#define SMEM_XCAT_TTI_PROT_VLAN_QOS_TBL_MEM(protocol, port) \
            (0xc004000 | ((port) * 0x80) | ((protocol) * 0x8))

/* TTI Unit Global Configuration */
#define SMEM_XCAT_A1_TTI_UNIT_GLB_CONF_REG(dev)                     \
    (SMEM_CHT_IS_SIP5_GET(dev) ? SMEM_CHT_MAC_REG_DB_SIP5_GET(dev)->TTI.TTIUnitGlobalConfigs.TTIUnitGlobalConfig : \
    (SMEM_CHT_MAC_REG_DB_GET(dev))->TTI.TTIUnitGlobalConfig)

/* TTI Engine Configuration */
#define SMEM_XCAT_TTI_ENGINE_CONFIG_REG(dev)                        \
    (SMEM_CHT_IS_SIP5_GET(dev) ? SMEM_CHT_MAC_REG_DB_SIP5_GET(dev)->TTI.TTIEngine.TTIEngineConfig : \
    (SMEM_CHT_MAC_REG_DB_GET(dev))->TTI.TTIEngineConfig)

/* TTI PCL ID Configuration 0*/
#define SMEM_XCAT_TTI_PCL_ID_CONFIG0_REG(dev)                       \
    (SMEM_CHT_IS_SIP5_GET(dev) ? SMEM_CHT_MAC_REG_DB_SIP5_GET(dev)->TTI.TTIEngine.TTIPCLIDConfig0 : \
    (SMEM_CHT_MAC_REG_DB_GET(dev))->TTI.TTIPCLIDConfig0)

/* TTI PCL ID Configuration 1*/
#define SMEM_XCAT_TTI_PCL_ID_CONFIG1_REG(dev)                       \
    (SMEM_CHT_IS_SIP5_GET(dev) ? SMEM_CHT_MAC_REG_DB_SIP5_GET(dev)->TTI.TTIEngine.TTIPCLIDConfig1 : \
    (SMEM_CHT_MAC_REG_DB_GET(dev))->TTI.TTIPCLIDConfig1)

/* TTI PCL TCP/UDP comparators */
#define SMEM_XCAT_TTI_PCL_TCP_PORT_RANGE_COMPARE_REG(dev)           \
    (SMEM_CHT_IS_SIP5_GET(dev) ? SMEM_CHT_MAC_REG_DB_SIP5_GET(dev)->TTI.userDefinedBytes.IPCLTCPPortRangeComparatorConfig[0] : \
    (SMEM_CHT_MAC_REG_DB_GET(dev))->TTI.userDefinedBytes.IPCLTCPPortRangeComparatorConfig[0])

#define SMEM_XCAT_TTI_PCL_UDP_PORT_RANGE_COMPARE_REG(dev)           \
    (SMEM_CHT_IS_SIP5_GET(dev) ? SMEM_CHT_MAC_REG_DB_SIP5_GET(dev)->TTI.userDefinedBytes.IPCLUDPPortRangeComparatorConfig[0] : \
    (SMEM_CHT_MAC_REG_DB_GET(dev))->TTI.userDefinedBytes.IPCLUDPPortRangeComparatorConfig[0])

/* User Defined Bytes Configuration Memory */
#define SMEM_XCAT_POLICY_UDB_CONF_MEM_REG(dev, index) \
      SMEM_TABLE_ENTRY_INDEX_GET_MAC(dev, ipclUserDefinedBytesConf, index)

/* Policy Engine User Defined Bytes Configuration */
#define SMEM_XCAT_POLICY_UDB_CONF_REG(dev)                          \
    (SMEM_CHT_IS_SIP5_GET(dev) ? SMEM_CHT_MAC_REG_DB_SIP5_GET(dev)->PCL.policyEngineUserDefinedBytesConfig: \
    (SMEM_CHT_MAC_REG_DB_GET(dev))->IPCL.policyEngineUserDefinedBytesConfig)

/* Policy_Engine_Configuration */
#define SMEM_XCAT_POLICY_ENGINE_CONF_REG(dev)                       \
    (SMEM_CHT_IS_SIP5_GET(dev) ? SMEM_CHT_MAC_REG_DB_SIP5_GET(dev)->PCL.policyEngineConfig: \
    (SMEM_CHT_MAC_REG_DB_GET(dev))->IPCL.policyEngineConfig)

/* Special EtherTypes */
#define SMEM_XCAT_SPECIAL_ETHERTYPES_REG(dev)                       \
    (SMEM_CHT_IS_SIP5_GET(dev) ? SMEM_CHT_MAC_REG_DB_SIP5_GET(dev)->TTI.specialEtherTypes: \
    (SMEM_CHT_MAC_REG_DB_GET(dev))->TTI.specialEtherTypes)

/* Egress Policy Global Configuration */
#define SMEM_XCAT_EPCL_GLOBAL_REG(dev)                  \
    (SMEM_CHT_MAC_REG_DB_GET(dev))->EPCL.egrPolicyGlobalConfig

/* Egress Policy TO CPU Configuration */
#define SMEM_XCAT_EPCL_TO_CPU_CONFIG_REG(dev)           \
    (SMEM_CHT_MAC_REG_DB_GET(dev))->EPCL.egrPolicyToCpuConfig[0]

/* Egress Policy FROM CPU Data Configuration */
#define SMEM_XCAT_EPCL_FROM_CPU_DATA_CONFIG_REG(dev)         \
    (SMEM_CHT_MAC_REG_DB_GET(dev))->EPCL.egrPolicyFromCpuDataConfig[0]

/* Egress Policy FROM CPU control Configuration */
#define SMEM_XCAT_EPCL_FROM_CPU_CONFIG_REG(dev)         \
    (SMEM_CHT_MAC_REG_DB_GET(dev))->EPCL.egrPolicyFromCpuCtrlConfig[0]

/* Egress Policy TO ANALYZER Configuration */
#define SMEM_XCAT_EPCL_TO_ANALYZER_CONFIG_REG(dev)      \
    (SMEM_CHT_MAC_REG_DB_GET(dev))->EPCL.egrPolicyToAnalyzerConfig[0]

/* Egress Policy Non TS Data Pkts Configuration */
#define SMEM_XCAT_EPCL_NTS_FORWARD_CONFIG_REG(dev)      \
    (SMEM_CHT_MAC_REG_DB_GET(dev))->EPCL.egrPolicyNonTSDataPktsConfig[0]

/* Egress Policy TS Data Pkts Configuration */
#define SMEM_XCAT_EPCL_TS_FORWARD_CONFIG_REG(dev)       \
    (SMEM_CHT_MAC_REG_DB_GET(dev))->EPCL.egrPolicyTSDataPktsConfig[0]

/* Egress PCL Configuration Table */
#define SMEM_XCAT_EPCL_CONFIG_TBL_MEM(dev, index)         \
    (SKERNEL_IS_LION_REVISON_B0_DEV(dev)                 \
        ? SMEM_LION_EPCL_CONFIG_TBL_MEM(dev, index) : 0xf808000 + 0x4 * (index))

/* Egress Policy Configuration Table Access mode Configuration */
#define SMEM_XCAT_EPCL_TBL_ACCESS_MODE_CONFIG_REG(dev)      \
    (SMEM_CHT_MAC_REG_DB_GET(dev))->EPCL.egrPolicyConfigTableAccessModeConfig[0]


/* L2/L3 Ingress VLAN Counting Enable Register */
#define SMEM_XCAT_CNC_VLAN_EN_REG(dev, port) \
    (SMEM_CHT_IS_SIP5_GET(dev) ? SMEM_CHT_MAC_REG_DB_SIP5_GET(dev)->PCL.L2L3IngrVLANCountingEnable[(port) / 32]: \
    (SMEM_CHT_MAC_REG_DB_GET(dev))->IPCL.L2L3IngrVLANCountingEnable[(port) / 32])

/* Counting Mode Configuration */
#define SMEM_XCAT_CNC_COUNT_MODE_REG(dev) \
    (SMEM_CHT_IS_SIP5_GET(dev) ? SMEM_CHT_MAC_REG_DB_SIP5_GET(dev)->PCL.countingModeConfig : \
    (SMEM_CHT_MAC_REG_DB_GET(dev))->IPCL.countingModeConfig)

/* Ingress Policer Metering Memory */
#define SMEM_XCAT_INGR_POLICER_METER_ENTRY_TBL_MEM(dev, cycle, index) \
    SMEM_TABLE_ENTRY_2_PARAMS_INDEX_GET_MAC(dev, policer, index, cycle)

/* Egress Policer Metering Memory */
#define SMEM_XCAT_EGR_POLICER_METER_ENTRY_TBL_MEM(dev, index) \
    (SMEM_CHT_IS_SIP5_20_GET(dev) ?                           \
    SMEM_XCAT_INGR_POLICER_METER_ENTRY_TBL_MEM(dev, SMEM_SIP5_PP_PLR_UNIT_EPLR_E, index): \
    SMEM_TABLE_ENTRY_INDEX_GET_MAC(dev, egressPolicerMeters, index))

/* Policer Port Metering */
#define SMEM_XCAT_POLICER_PORT_METER_REG(dev, cycle, port) \
    (SMEM_CHT_IS_SIP6_GET(dev) ? SMEM_XCAT_POLICER_PORT_METER_POINTER_TBL_MEM(dev,cycle,port) : \
    (SMEM_CHT_IS_SIP5_GET(dev) ? SMEM_CHT_MAC_REG_DB_SIP5_GET(dev)->PLR[cycle].policerPortMeteringReg[(port) / 32] : \
    (   \
        ((port < 31) ?            \
            (SMEM_CHT_MAC_REG_DB_GET(dev))->PLR[cycle].policerPortMetering0 : \
            (SMEM_CHT_MAC_REG_DB_GET(dev))->PLR[cycle].policerPortMetering1))))

/* Ingress_Policer_Initial DP */
#define SMEM_XCAT_POLICER_INITIAL_DP_REG(dev, cycle,qosProfile) \
    (SMEM_CHT_IS_SIP6_GET(dev) ? SMEM_SIP6_POLICER_QOS_ATTRIBUTE_TBL_MEM(dev,cycle,qosProfile) : \
    (SMEM_CHT_IS_SIP5_GET(dev) ? SMEM_CHT_MAC_REG_DB_SIP5_GET(dev)->PLR[cycle].policerInitialDP[(qosProfile)/16] : \
    (SMEM_CHT_MAC_REG_DB_GET(dev))->PLR[cycle].policerInitialDP[(qosProfile)/16]))

/* Policer Control0 */
#define SMEM_XCAT_POLICER_GLB_CONF_REG(dev, cycle) \
    (SMEM_CHT_IS_SIP5_GET(dev) ? SMEM_CHT_MAC_REG_DB_SIP5_GET(dev)->PLR[cycle].policerCtrl0 : \
    (SMEM_CHT_MAC_REG_DB_GET(dev))->PLR[cycle].policerCtrl0)

/* Policer Control1 */
#define SMEM_XCAT_POLICER_GLB1_CONF_REG(dev, cycle) \
    (SMEM_CHT_IS_SIP5_GET(dev) ? SMEM_CHT_MAC_REG_DB_SIP5_GET(dev)->PLR[cycle].policerCtrl1 : \
    (SMEM_CHT_MAC_REG_DB_GET(dev))->PLR[cycle].policerCtrl1)

/* Policer hierarchical policer ctrl register */
#define SMEM_XCAT_PLR_HIERARCHICAL_POLICER_CTRL_REG(dev, cycle) \
    (SMEM_CHT_IS_SIP5_GET(dev)  ? SMEM_CHT_MAC_REG_DB_SIP5_GET(dev)->PLR[cycle].hierarchicalPolicerCtrl : \
    (SMEM_CHT_MAC_REG_DB_GET(dev))->PLR[cycle].hierarchicalPolicerCtrl)

#define SMEM_XCAT_POLICER_MNG_CNT_TBL_MEM(dev, cycle, counter) \
    SMEM_TABLE_ENTRY_INDEX_DUP_TBL_GET_MAC(dev, policerManagementCounters , (counter*4) , cycle)

/* Ingress/Egress Policer IPFIX dropped packet count WA threshold */
#define SMEM_XCAT_POLICER_IPFIX_DROP_CNT_WA_TRESH_REG(dev, cycle) \
    (SMEM_CHT_IS_SIP5_GET(dev)  ? SMEM_CHT_MAC_REG_DB_SIP5_GET(dev)->PLR[cycle].policerIPFIXDroppedPktCountWAThreshold : \
    (SMEM_CHT_MAC_REG_DB_GET(dev))->PLR[cycle].policerIPFIXDroppedPktCountWAThreshold)


/* Ingress/Egress Policer IPFIX packet count WA threshold */
#define SMEM_XCAT_POLICER_IPFIX_PCKT_CNT_WA_TRESH_REG(dev, cycle) \
    (SMEM_CHT_IS_SIP5_GET(dev)  ? SMEM_CHT_MAC_REG_DB_SIP5_GET(dev)->PLR[cycle].policerIPFIXPktCountWAThresholdLSB : \
    (SMEM_CHT_MAC_REG_DB_GET(dev))->PLR[cycle].policerIPFIXPktCountWAThreshold)

/* Ingress/Egress Policer IPFIX bytecount WA threshold LSB */
#define SMEM_XCAT_POLICER_IPFIX_BYTE_CNT_LSB_WA_TRESH_REG(dev, cycle) \
    (SMEM_CHT_IS_SIP5_GET(dev)  ? SMEM_CHT_MAC_REG_DB_SIP5_GET(dev)->PLR[cycle].policerIPFIXByteCountWAThresholdLSB : \
    (SMEM_CHT_MAC_REG_DB_GET(dev))->PLR[cycle].policerIPFIXBytecountWAThresholdLSB)

/* Ingress/Egress Policer IPFIX Control */
#define SMEM_XCAT_POLICER_IPFIX_CTRL_REG(dev, cycle) \
    (SMEM_CHT_IS_SIP5_GET(dev)  ? SMEM_CHT_MAC_REG_DB_SIP5_GET(dev)->PLR[cycle].IPFIXCtrl : \
    (SMEM_CHT_MAC_REG_DB_GET(dev))->PLR[cycle].IPFIXCtrl)


/* Ingress Policer Interrupt Cause */
#define SMEM_XCAT_POLICER_IPFIX_INT_CAUSE_REG(dev, cycle) \
    (SMEM_CHT_IS_SIP5_GET(dev)  ? SMEM_CHT_MAC_REG_DB_SIP5_GET(dev)->PLR[cycle].policerInterruptCause : \
    (SMEM_CHT_MAC_REG_DB_GET(dev))->PLR[cycle].policerInterruptCause)

/* Ingress/Egress IPFIX Sample Entries Log0 */
#define SMEM_XCAT_POLICER_IPFIX_SAMPLE_LOG0_REG(dev, cycle) \
    (SMEM_CHT_IS_SIP5_GET(dev)  ? SMEM_CHT_MAC_REG_DB_SIP5_GET(dev)->PLR[cycle].ipfixSampleEntriesLog_n[0] : \
    (SMEM_CHT_MAC_REG_DB_GET(dev))->PLR[cycle].IPFIXSampleEntriesLog0)

/* Ingress/Egress management counters policer Shadow 0 */
#define SMEM_XCAT_POLICER_MANAGEMENT_COUNTERS_POLICER_SHADOW_REG(dev, cycle) \
    (SMEM_CHT_IS_SIP5_GET(dev)  ? SMEM_CHT_MAC_REG_DB_SIP5_GET(dev)->PLR[cycle].policerShadow[0] : \
    (SMEM_CHT_MAC_REG_DB_GET(dev))->PLR[cycle].policerShadow[0])

/* Ingress/Egress policer MRU */
#define SMEM_XCAT_POLICER_MRU_REG(dev, cycle) \
    (SMEM_CHT_IS_SIP5_GET(dev)  ? SMEM_CHT_MAC_REG_DB_SIP5_GET(dev)->PLR[cycle].policerMRU : \
    (SMEM_CHT_MAC_REG_DB_GET(dev))->PLR[cycle].policerMRU)

/* Ingress/Egress hierarchical table */
#define SMEM_XCAT_POLICER_HIERARCHCIAL_TBL_MEM(dev, index) \
    SMEM_TABLE_ENTRY_INDEX_GET_MAC(dev, policerHierarchicalQos, index)

/* Egress policer relevant registers */

/* Ingress Policer Counting Memory */
#define SMEM_XCAT_INGR_POLICER_CNT_ENTRY_REG(dev, index, cycle) \
    SMEM_TABLE_ENTRY_2_PARAMS_INDEX_GET_MAC(dev, policerCounters, index, cycle)

/* Egress Policer Counting Memory */
#define SMEM_XCAT_EGR_POLICER_CNT_ENTRY_REG(dev, index) \
    (SMEM_CHT_IS_SIP5_20_GET(dev) ?                     \
     SMEM_XCAT_INGR_POLICER_CNT_ENTRY_REG(dev, index, SMEM_SIP5_PP_PLR_UNIT_EPLR_E): \
     SMEM_TABLE_ENTRY_INDEX_GET_MAC(dev, egressPolicerCounters, index))

/* VLT Tables Access Data Register */
#define SMEM_XCAT_VLAN_TBL_ACCESS_BASE_REG(dev)                     \
    (   \
      (dev)->txqRevision == 0 ?                                     \
        (SMEM_CHT_MAC_REG_DB_GET(dev))->egrAndTxqVer0.VLANAndMcGroupAndSpanStateGroupTables.VLTTablesAccessDataReg[5] : \
        SMAIN_NOT_VALID_CNS)

/* Links Status Register */
#define SMEM_LION_LINKS_STATUS_REG_MAC(dev)                         \
    (   \
      (dev)->txqRevision == 0 ?                                     \
        (SMEM_CHT_MAC_REG_DB_GET(dev))->egrAndTxqVer0.transmitQueueGlobalConfig.portsLinkEnable : \
        SMAIN_NOT_VALID_CNS)

/* Port Ingress Mirror Index */
#define SMEM_XCAT_INGR_MIRROR_INDEX_REG(dev, port) \
    (SMEM_CHT_IS_SIP5_GET(dev)  ? SMAIN_NOT_VALID_CNS : \
    (SMEM_CHT_MAC_REG_DB_GET(dev))->EQ.mirrToAnalyzerPortConfigs.portIngrMirrorIndex[((port) / 10)])

/* Port Egress Mirror Index */
#define SMEM_XCAT_EGR_MIRROR_INDEX_REG(dev, port) \
    (SMEM_CHT_IS_SIP5_GET(dev)  ? SMAIN_NOT_VALID_CNS : \
      (SMEM_CHT_MAC_REG_DB_GET(dev))->EQ.mirrToAnalyzerPortConfigs.portEgrMirrorIndex[(port) >> 6][(((port) & 0x3f) / 10)])

/* Mirror Interface Parameter Register */
#define SMEM_XCAT_MIRROR_INTERFACE_PARAM_REG(dev, index) \
    (SMEM_CHT_IS_SIP5_GET(dev)  ? (SMEM_CHT_MAC_REG_DB_SIP5_GET(dev))->EQ.mirrToAnalyzerPortConfigs.mirrorInterfaceParameterReg[index] : \
    (SMEM_CHT_MAC_REG_DB_GET(dev))->EQ.mirrToAnalyzerPortConfigs.mirrorInterfaceParameterReg[index])

/* Analyzer Port Global Configuration */
#define SMEM_XCAT_ANALYZER_PORT_GLOBAL_CONF_REG(dev)                \
    (SMEM_CHT_IS_SIP5_GET(dev)  ? (SMEM_CHT_MAC_REG_DB_SIP5_GET(dev))->EQ.mirrToAnalyzerPortConfigs.analyzerPortGlobalConfig : \
    (SMEM_CHT_MAC_REG_DB_GET(dev))->EQ.mirrToAnalyzerPortConfigs.analyzerPortGlobalConfig)


/* IEEE_table_select< 0, 1 > */
#define SMEM_XCAT_IEEE_TBL_SELECT_REG_MAC(dev, port) \
    SMEM_TABLE_ENTRY_INDEX_GET_MAC(dev, ieeeTblSelect, port)

/* Unicast Routing Engine Configuration Register */
#define SMEM_XCAT_UC_ROUT_ENG_CONF_REG(dev)                         \
    (SMEM_CHT_MAC_REG_DB_GET(dev))->ucRouter.ucRoutingEngineConfigReg

/* Trunk Hash Configuration Register0 */
#define SMEM_XCAT_TRUNK_HASH_CONF_REGISTER0(dev) \
    (SMEM_CHT_IS_SIP5_GET(dev) ? SMEM_CHT_MAC_REG_DB_SIP5_GET(dev)->TTI.trunkHashSimpleHash.trunkHashConfigReg0 : \
    (SMEM_CHT_MAC_REG_DB_GET(dev))->TTI.trunkHash.trunkHashConfigReg0)

/* Logical Target Mapping Table Access Control */
#define SMEM_XCAT_LOGICAL_TRG_MAPPING_TBL_ACCESS_REG(dev)           \
    (SMEM_CHT_IS_SIP5_GET(dev) ? SMAIN_NOT_VALID_CNS :            \
    (SMEM_CHT_MAC_REG_DB_GET(dev))->EQ.logicalTargetMapTable.logicalTargetMapTableAccessCtrl )

/* Logical Target Mapping Table Data Access */
#define SMEM_XCAT_LOGICAL_TRG_MAPPING_TBL_DATA_REG(dev)             \
    (SMEM_CHT_IS_SIP5_GET(dev) ? SMAIN_NOT_VALID_CNS :            \
    (SMEM_CHT_MAC_REG_DB_GET(dev))->EQ.logicalTargetMapTable.logicalTargetMapTableDataAccess)

/* Logical Target Mapping Table Data Access 2 */
#define SMEM_XCAT_LOGICAL_TRG_MAPPING_TBL_DATA2_REG(dev)             \
    (SMEM_CHT_IS_SIP5_GET(dev) ? SMAIN_NOT_VALID_CNS :            \
    (SMEM_CHT_MAC_REG_DB_GET(dev))->EQ.logicalTargetMapTable.logicalTargetMapTableDataAccess2 )


/* Logical Target Device Mapping Configuration */
#define SMEM_XCAT_LOGICAL_TRG_DEV_MAPPING_CONF_REG(dev)             \
    (SMEM_CHT_IS_SIP5_GET(dev) ? SMAIN_NOT_VALID_CNS : \
    (SMEM_CHT_MAC_REG_DB_GET(dev))->EQ.preEgrEngineGlobalConfig.logicalTargetDeviceMapConfig)

/* Ingress/Egress Policer Metering Memory */
#define SMEM_XCAT_POLICER_METER_ENTRY_TBL_MEM(dev, direction, cycle, index) \
     ((direction) == SMAIN_DIRECTION_INGRESS_E) ?                  \
        SMEM_XCAT_INGR_POLICER_METER_ENTRY_TBL_MEM(dev, cycle, index) : \
        SMEM_XCAT_EGR_POLICER_METER_ENTRY_TBL_MEM(dev, index)

/* Ingress/Egress Policer Metering Memory */
#define SMEM_XCAT_POLICER_METER_ENTRY_TBL_MEM_ANY(dev, cycle, index) \
     ((cycle) < 2) ?                  \
        SMEM_XCAT_INGR_POLICER_METER_ENTRY_TBL_MEM(dev, cycle, index) : \
        SMEM_XCAT_EGR_POLICER_METER_ENTRY_TBL_MEM(dev, index)


 /* Ingress/Egress Policer Counter Memory */
#define SMEM_XCAT_POLICER_CNT_ENTRY_TBL_MEM(dev, direction, cycle, index) \
     ((direction) == SMAIN_DIRECTION_INGRESS_E) ?                  \
            ( smemChtPolicerTableIndexCheck(dev, cycle, index) ?                 \
             SMEM_XCAT_INGR_POLICER_CNT_ENTRY_REG(dev, index, cycle) : SMAIN_NOT_VALID_CNS ) : \
             SMEM_XCAT_EGR_POLICER_CNT_ENTRY_REG(dev, index)

/* Ingress/Egress Policer Port metering pointer index */
#define SMEM_XCAT_POLICER_PORT_METER_POINTER_TBL_MEM(dev, cycle, index) \
    SMEM_TABLE_ENTRY_INDEX_DUP_TBL_GET_MAC(dev, policerMeterPointer, index, cycle)

/* Logical Target Mapping Table */
#define SMEM_XCAT_LOGICAL_TRG_MAPPING_TBL_MEM(dev, trg_dev, trg_port)         \
    SMEM_TABLE_ENTRY_2_PARAMS_INDEX_GET_MAC(dev, logicalTrgMappingTable, trg_dev, trg_port)

/* Ingress Policer IPFIX nano timer stamp upload */
#define SMEM_XCAT_POLICER_IPFIX_NANO_TS_UPLOAD_REG(dev, cycle) \
    (SMEM_CHT_IS_SIP5_GET(dev)  ? SMEM_CHT_MAC_REG_DB_SIP5_GET(dev)->PLR[cycle].IPFIXNanoTimerStampUpload : \
    (SMEM_CHT_MAC_REG_DB_GET(dev))->PLR[cycle].IPFIXNanoTimerStampUpload )


/* Ingress/Egress IPFIX wrap around alert Memory */
#define SMEM_XCAT_POLICER_IPFIX_WA_ALERT_TBL_MEM(dev, cycle, index) \
    SMEM_TABLE_ENTRY_INDEX_DUP_TBL_GET_MAC(dev, policerIpfixWaAlert, index, cycle)

/* Ingress/Egress IPFIX aging alert Memory */
#define SMEM_XCAT_POLICER_IPFIX_AGING_ALERT_TBL_MEM(dev, cycle, index) \
    SMEM_TABLE_ENTRY_INDEX_DUP_TBL_GET_MAC(dev, policerIpfixAgingAlert, index, cycle)

/* Policer Interrupt Mask */
#define SMEM_XCAT_POLICER_IPFIX_INT_MASK_REG(dev, cycle) \
    ((SMEM_XCAT_POLICER_IPFIX_INT_CAUSE_REG(dev, cycle)) + 0x4)

/* Policer Timer Memory */
#define SMEM_XCAT_POLICER_TIMER_TBL_MEM(dev, cycle, index)  \
    SMEM_TABLE_ENTRY_INDEX_DUP_TBL_GET_MAC(dev, policerTimer, index, cycle)

/* Policer Table Access Data */
#define SMEM_XCAT_POLICER_INDIRECT_ACCESS_DATA_ENTRY_REG(dev, cycle, entry_index) \
    SMEM_TABLE_ENTRY_INDEX_DUP_TBL_GET_MAC(dev, policerTblAccessData, entry_index, cycle)

/* DSA configuration and fast stack register */
#define SMEM_XCAT_FAST_STACK_REG(dev) \
    (SMEM_CHT_IS_SIP5_GET(dev) ? SMEM_CHT_MAC_REG_DB_SIP5_GET(dev)->TTI.DSAConfig : \
    (SMEM_CHT_MAC_REG_DB_GET(dev))->TTI.DSAConfig)

/* Loop Port<%n> */
#define SMEM_XCAT_LOOP_PORT_REG(dev)\
    (SMEM_CHT_IS_SIP5_GET(dev) ? SMAIN_NOT_VALID_CNS : \
    (SMEM_CHT_MAC_REG_DB_GET(dev))->TTI.loopPort[0])

/* TO ANALYZER VLAN Tag Add Enable Configuration Register */
#define SMEM_XCAT_TO_ANALYZER_VLAN_TAG_ADD_EN_REG(dev)              \
    (SMEM_CHT_IS_SIP5_GET(dev) ? SMAIN_NOT_VALID_CNS : \
    (SMEM_CHT_MAC_REG_DB_GET(dev))->HA.toAnalyzerVLANTagAddEnableConfig[0])

/* HA : Ingress Analyzer VLAN Tag Configuration */
#define SMEM_XCAT_HA_INGR_ANALYZER_VLAN_TAG_CONF_REG(dev)              \
    (SMEM_CHT_IS_SIP5_GET(dev) ? SMEM_CHT_MAC_REG_DB_SIP5_GET(dev)->HA.ingrAnalyzerVLANTagConfig : \
    (SMEM_CHT_MAC_REG_DB_GET(dev))->HA.ingrAnalyzerVLANTagConfig)

/* Egress Analyzer VLAN Tag Configuration */
#define SMEM_XCAT_HA_EGR_ANALYZER_VLAN_TAG_CONF_REG(dev)               \
    (SMEM_CHT_IS_SIP5_GET(dev) ? SMEM_CHT_MAC_REG_DB_SIP5_GET(dev)->HA.egrAnalyzerVLANTagConfig : \
    (SMEM_CHT_MAC_REG_DB_GET(dev))->HA.egrAnalyzerVLANTagConfig)

/* Metal Fix */
#define SMEM_XCAT_MG_METAL_FIX_REG(dev)                             \
    SMAIN_NOT_VALID_CNS


/* Egress Analyzer VLAN Tag Configuration */
#define SMEM_XCAT_HA_QOS_PROFILE_TO_EXP_REG(dev)               \
    (SMEM_CHT_IS_SIP5_GET(dev) ? SMAIN_NOT_VALID_CNS : \
    (SMEM_CHT_MAC_REG_DB_GET(dev))->HA.qoSProfileToEXPReg[0])

/* Port<4n...4n+3> TrunkNum Configuration Register<%n> - the register holds trunk ID per egress port */
#define SMEM_XCAT_HA_TRUNK_ID_FOR_PORT_BASE_REG(dev, port)   \
    (SMEM_CHT_IS_SIP5_GET(dev) ? SMAIN_NOT_VALID_CNS : \
    (SMEM_CHT_MAC_REG_DB_GET(dev))->HA.portTrunkNumConfigReg[(port) >> 2])

/* CPID Register1*/
#define SMEM_XCAT_HA_CPID_1_REG(dev)                         \
    (SMEM_CHT_IS_SIP5_GET(dev) ? SMAIN_NOT_VALID_CNS : \
    (SMEM_CHT_MAC_REG_DB_GET(dev))->HA.CPIDReg1)

/* TTI MPLS Ethertypes Configuration Register */
#define SMEM_XCAT_TTI_MPLS_ETHERTYPES_REG(dev)                   \
    (SMEM_CHT_IS_SIP5_GET(dev) ? SMEM_CHT_MAC_REG_DB_SIP5_GET(dev)->TTI.MPLSEtherTypes : \
    (SMEM_CHT_MAC_REG_DB_GET(dev))->TTI.MPLSEtherTypes)

/* TTI IPv6 Extension Value Register */
#define SMEM_XCAT_TTI_IPV6_EXTENSION_VALUE_REG(dev)                   \
    (SMEM_CHT_IS_SIP5_GET(dev) ? SMEM_CHT_MAC_REG_DB_SIP5_GET(dev)->TTI.IPv6ExtensionValue : \
    (SMEM_CHT_MAC_REG_DB_GET(dev))->TTI.IPv6ExtensionValue)

/* HA MPLS Ethertypes Configuration Register */
#define SMEM_XCAT_HA_MPLS_ETHERTYPES_REG(dev)                \
    (SMEM_CHT_IS_SIP5_GET(dev) ? SMEM_CHT_MAC_REG_DB_SIP5_GET(dev)->HA.MPLSEtherType[0] : \
    (SMEM_CHT_MAC_REG_DB_GET(dev))->HA.MPLSEthertype[0])

/* TTI IPv4 GRE Ethertype */
#define SMEM_XCAT_TTI_IPV4_GRE_ETHERTYPE_REG(dev)                   \
    (SMEM_CHT_IS_SIP5_GET(dev) ? SMEM_CHT_MAC_REG_DB_SIP5_GET(dev)->TTI.TTIIPv4GREEthertype : \
    (SMEM_CHT_MAC_REG_DB_GET(dev))->TTI.TTIIPv4GREEthertype)

/* XG/XLG Port MAC Control Register3 */
#define SMEM_XCAT_XG_MAC_CONTROL3_REG(dev, port)                   \
     (((SMEM_CHT_IS_SIP5_GET(dev) == 0) && (port) == SNET_CHT_CPU_PORT_CNS) ? \
            SMAIN_NOT_VALID_CNS : \
            (SMEM_CHT_MAC_REG_DB_GET(dev))->GOP.ports.XLGIP[(port)].portMACCtrlReg[3])

/* XG/XLG Port MAC Control Register4 */
#define SMEM_XCAT_XG_MAC_CONTROL4_REG(dev, port)    \
     (SMEM_CHT_IS_SIP5_GET(dev) ?                   \
            (SMEM_CHT_MAC_REG_DB_GET(dev))->GOP.ports.XLGIP[(port)].portMACCtrlReg[4] : \
            SMAIN_NOT_VALID_CNS \
     )

/* GE Port MAC Control Register3 */
#define SMEM_XCAT_GE_MAC_CONTROL3_REG(dev, port)                   \
     (((port) == SNET_CHT_CPU_PORT_CNS) ? \
        (SMEM_CHT_MAC_REG_DB_GET(dev))->GOP.ports.gigPort63CPU.portMACCtrlReg[3] : \
            (IS_CHT_HYPER_GIGA_PORT(dev,port)) ? \
                SMAIN_NOT_VALID_CNS  : \
                (SMEM_CHT_MAC_REG_DB_GET(dev))->GOP.ports.gigPort[(port)].portMACCtrlReg[3])

/* L2MLL - virtual port to mll mapping. */
#define SMEM_XCAT_L2_MLL_VIRTUAL_PORT_TO_MLL_MAPPING_TBL_MEM(dev, index) \
    SMEM_TABLE_ENTRY_INDEX_GET_MAC(dev, l2MllVirtualPortToMllMapping, index)

/* L2MLL - vidx to mll mapping. */
#define SMEM_XCAT_L2_MLL_VIDX_TO_MLL_MAPPING_TBL_MEM(dev, index) \
    SMEM_TABLE_ENTRY_INDEX_GET_MAC(dev, l2MllVidxToMllMapping, index)

/* EQ - LP Egress VLAN member table. */
#define SMEM_XCAT_EQ_LOGICAL_PORT_EGRESS_VLAN_MEMBER_TBL_MEM(dev, index) \
    SMEM_TABLE_ENTRY_INDEX_GET_MAC(dev, eqLogicalPortEgressVlanMember, index)

/* EQ - VLAN mapping table. */
#define SMEM_XCAT_EQ_VLAN_MAPPING_TABLE_TBL_MEM(dev, index) \
    SMEM_TABLE_ENTRY_INDEX_GET_MAC(dev, eqVlanMapping, index)

/*EQ - vlan Egress Filter Counter */
#define SMEM_XCAT_EQ_VLAN_EGRESS_FILTER_COUNTER_REG(dev)                       \
    (SMEM_CHT_IS_SIP5_GET(dev) ? SMAIN_NOT_VALID_CNS : \
    (SMEM_CHT_MAC_REG_DB_GET(dev))->EQ.vlanEgressFilter.vlanEgressFilterCounter)

/* EQ - egress Filter Vlan Map table -- indirect data register */
#define SMEM_XCAT_EQ_EGRESS_FILTER_VLAN_MAP_TBL_DATA_REG(dev)             \
    (SMEM_CHT_IS_SIP5_GET(dev) ? SMAIN_NOT_VALID_CNS : \
    (SMEM_CHT_MAC_REG_DB_GET(dev))->EQ.egressFilterVlanMap.egressFilterVlanMapTableDataAccess)

/* L2MLL VIDX enable table -- indirect data register */
#define SMEM_XCAT_L2_MLL_VIDX_ENABLE_TBL_DATA_REG(dev)             \
    (SMEM_CHT_MAC_REG_DB_GET(dev))->MLL.l2MllVidxEnableTable.l2MllVidxEnableTableDataAccess

/* L2MLL pointer map table -- indirect data register */
#define SMEM_XCAT_L2_MLL_POINTER_MAP_TBL_DATA_REG(dev)             \
    (SMEM_CHT_MAC_REG_DB_GET(dev))->MLL.l2MllPointerMapTable.l2MllPointerMapTableDataAccess

/* EQ - Egress Filter Vlan Member Table -- indirect data register */
#define SMEM_XCAT_EQ_EGRESS_FILTER_VLAN_MEMBER_TABLE_TBL_DATA_REG(dev)             \
    (SMEM_CHT_IS_SIP5_GET(dev) ? SMAIN_NOT_VALID_CNS : \
    (SMEM_CHT_MAC_REG_DB_GET(dev))->EQ.egressFilterVlanMemberTable.egressFilterVlanMemberTableDataAccess )


/* TTI - Pseudo Wire Configurations */
#define SMEM_XCAT_TTI_PSEUDO_WIRE_CONFIG_CONF_REG(dev)                     \
    (SMEM_CHT_IS_SIP5_GET(dev) ? SMEM_CHT_MAC_REG_DB_SIP5_GET(dev)->TTI.pseudoWire.PWE3Config : \
    (SMEM_CHT_MAC_REG_DB_GET(dev))->TTI.pseudoWireConfig)

/* TTI - Ingress Port Tag0/Tag1 TPID - for ethernet passenger .*/
#define SMEM_XCAT_PASSENGER_INGR_PORT_TPID_REG(dev)                            \
    (SMEM_CHT_IS_SIP5_GET(dev) ? SMAIN_NOT_VALID_CNS : \
    (SMEM_CHT_MAC_REG_DB_GET(dev))->TTI.VLANAssignment.passengerIngrTPIDSelect[0])

/* HA - tunnel start Egress Port Tag TPID */
#define SMEM_XCAT_HA_TS_EGR_PORT_TPID_REG(dev)                       \
    (SMEM_CHT_IS_SIP5_GET(dev) ? SMAIN_NOT_VALID_CNS : \
    (SMEM_CHT_MAC_REG_DB_GET(dev))->HA.portTsTagTpidSelect[0])

/* HA - Egress global TPID */
#define SMEM_XCAT_HA_TS_EGR_GLOBAL_TAG_TPID_REG(dev)                      \
    (SMEM_CHT_IS_SIP5_GET(dev) ? SMAIN_NOT_VALID_CNS : \
    (SMEM_CHT_MAC_REG_DB_GET(dev))->HA.tsTPIDConfigReg[0])


/* HA - Egress global TPID (index is 1..7) */
#define SMEM_XCAT_HA_TS_MPLS_CONTROL_WORD_REG(dev,index)                      \
    (SMEM_CHT_IS_SIP5_GET(dev)  ? SMEM_CHT_MAC_REG_DB_SIP5_GET(dev)->HA.PWCtrlWord[(index-1)] : \
    (SMEM_CHT_MAC_REG_DB_GET(dev))->HA.mplsControlWord[(index-1)])

/* HA - Passenger TPID Index source */
#define SMEM_XCAT_PASSENGER_TPID_INDEX_SOURCE_REG(dev)                       \
    (SMEM_CHT_IS_SIP5_GET(dev) ? SMAIN_NOT_VALID_CNS : \
    (SMEM_CHT_MAC_REG_DB_GET(dev))->HA.passengerTpidIndexSource)

/* Passenger TPID0_TPID1 table */
#define SMEM_XCAT_PASSENGER_TPID_CONFIG_REG(dev)                       \
    (SMEM_CHT_IS_SIP5_GET(dev) ? SMAIN_NOT_VALID_CNS : \
    (SMEM_CHT_MAC_REG_DB_GET(dev))->HA.passengerTpid0Tpid1[0])

/* HA - FROM_CPU constants to fanout devices */
#define SMEM_XCAT_HA_DSA_FROM_CPU_CONSTANTS_TO_FANOUT_DEVICES_REG(dev)                       \
    (SMEM_CHT_IS_SIP5_GET(dev) ? SMAIN_NOT_VALID_CNS : \
    (SMEM_CHT_MAC_REG_DB_GET(dev))->HA.from_cpuConstantBits)

/* HA - dsa Port is Fanout Device */
#define SMEM_XCAT_HA_DSA_PORT_IS_FANOUT_DEVICE_REG(dev)                       \
    (SMEM_CHT_IS_SIP5_GET(dev) ? SMAIN_NOT_VALID_CNS : \
    (SMEM_CHT_MAC_REG_DB_GET(dev))->HA.dsaPortisFanoutDevice)

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  /* __sregXCath */

