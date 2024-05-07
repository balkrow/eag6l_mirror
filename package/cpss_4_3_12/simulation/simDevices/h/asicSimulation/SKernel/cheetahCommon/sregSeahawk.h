/******************************************************************************
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
* @file sregSeahawk.h
*
* @brief Defines for Ironman memory registers access.
*
* @version   1
********************************************************************************
*/
#ifndef __sregSeahawkh
#define __sregSeahawkh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <asicSimulation/SKernel/smem/smemHawk.h>
#include <asicSimulation/SKernel/smem/smemCheetah.h>
#include <asicSimulation/SKernel/smem/smemAas.h>
#include <asicSimulation/SKernel/suserframes/snetAasHa.h>

/* TTI - global registers */
#define SIP7_TTI_REG_MAC(dev, regName) \
    SIP_7_REG(dev, TTI.regName)

#define SMEM_SIP7_TTI_UNIT_RANGE_CHECK_GLOBAL_CONFIG_0_REG(dev)     \
        SIP7_TTI_REG_MAC(dev, ttiRangeCheckGlobalConfig0)

#define SMEM_SIP7_TTI_UNIT_RANGE_CHECK_GLOBAL_CONFIG_1_REG(dev)     \
        SIP7_TTI_REG_MAC(dev, ttiRangeCheckGlobalConfig1)

/* TTI_LU - global registers */
#define SIP7_TTI_LU_REG_MAC(dev, regName) \
        SIP_7_REG(dev, TTI_LU.regName)

#define SMEM_SIP7_TTI_LU_UNIT_GENERAL_GLOBAL_CONFIG_REG(dev)     \
        SIP7_TTI_LU_REG_MAC(dev, generalRegs.ttiLuGlobalConfig)

#define SMEM_SIP7_TTI_LU_UNIT_GENERIC_ACTION_CONFIG_REG(dev)     \
        SIP7_TTI_LU_REG_MAC(dev, ttiLuActionConfig.ttiLuGenericActionConfig)

#define SMEM_SIP7_TTI_LU_UNIT_TT_SRC_ID_ASSIGN_REG(dev,index)     \
        SIP7_TTI_LU_REG_MAC(dev, ttiLuActionConfig.ttiLuTtSrcIdAssignment[index])

#define SMEM_SIP7_TTI_LU_TUNNEL_HEADER_LENGTH_PROFILE_REG(dev, profileIdx) \
        SIP7_TTI_LU_REG_MAC(dev,ttiLuActionConfig.ttiLuTunnelHeaderLenProfile[profileIdx])

#define SMEM_SIP7_TTI_LU_UNIT_RANGE_CLASSIFICATION_0_REG(dev,index)    \
        SIP7_TTI_LU_REG_MAC(dev, ttiLuGenericRangesConfig.ttiLuRangeClassification0[index])

#define SMEM_SIP7_TTI_LU_UNIT_RANGE_CLASSIFICATION_1_REG(dev,index)    \
        SIP7_TTI_LU_REG_MAC(dev, ttiLuGenericRangesConfig.ttiLuRangeClassification1[index])

#define SMEM_SIP7_LPM_MEMORY_TBL_MEM(dev, entry) \
    /* the LPM build of 48 rams of up to 256k lines in each */ \
    SMEM_TABLE_ENTRY_2_PARAMS_INDEX_GET_MAC(dev, lpmMemory, \
        ((entry)%(256*1024))/*line index*/ , ((entry)/(256*1024))/* ram index 0..37*/)

/* LPM - lpm Vrf Id table - hold ipv4/ipv6/fcoe */
#define SMEM_SIP7_LPM_COMMON_VRF_ID_TBL_MEM(dev, entry,lpmUnitIndex) \
    SMEM_TABLE_ENTRY_INDEX_DUP_TBL_GET_MAC(dev, lpmCommonVrfId, entry,lpmUnitIndex)

/* L2I Bridge Global Configuration Register4 */
#define SMEM_SIP7_L2I_BRDG_GLB_CONF4_REG(dev)                                 \
    SIP_7_REG(dev,L2I.bridgeEngineConfig.bridgeGlobalConfig4)

/* L2I Bridge Global Configuration Register5 */
#define SMEM_SIP7_L2I_BRDG_GLB_CONF5_REG(dev)                                 \
    SIP_7_REG(dev,L2I.bridgeEngineConfig.bridgeGlobalConfig5)

/* L2I bridge Mac Spoof Protection Config 0 */
#define SMEM_SIP7_L2I_BRDG_MAC_SPOOF_PROTECTION_CONF0_REG(dev)                \
    SIP_7_REG(dev,L2I.bridgeEngineConfig.bridgeMacSpoofProtectionConfig0)

/* L2I bridge Mac Spoof Protection Config 1 */
#define SMEM_SIP7_L2I_BRDG_MAC_SPOOF_PROTECTION_CONF1_REG(dev)                \
    SIP_7_REG(dev,L2I.bridgeEngineConfig.bridgeMacSpoofProtectionConfig1)

/* MLL - multi-target vPort - range base */
#define SMEM_SIP7_MLL_MULTI_TARGET_VPORT_RANGE_BASE_REG(dev) \
    SIP_7_REG(dev,MLL.multiTargetVPortMap.multiTargetVportRangeBase)

/* MLL - multi-target vPort - range size */
#define SMEM_SIP7_MLL_MULTI_TARGET_VPORT_RANGE_SIZE_REG(dev) \
    SIP_7_REG(dev,MLL.multiTargetVPortMap.multiTargetVportRangeSize)

/* MLL - multi-target vPort - range eVidx */
#define SMEM_SIP7_MLL_MULTI_TARGET_VPORT_RANGE_EVIDX_REG(dev) \
    SIP_7_REG(dev,MLL.multiTargetVPortMap.multiTargetVportRangeEVidx)

/* MLL - multi-target vPort - sub range size (per instance)*/
#define SMEM_SIP7_MLL_MULTI_TARGET_VPORT_SUB_RANGE_SIZE_REG(dev,instance) \
    SIP_7_REG(dev,MLL.multiTargetVPortMap.subRangeArr[instance].multiTargetVPortSubRangeSize)

/* MLL - multi-target vPort - sub range backup Cfg Value (per instance,chunk)*/
#define SMEM_SIP7_MLL_MULTI_TARGET_VPORT_SUB_RANGE_BACKUP_CFG_VALUE_REG(dev,instance,chunk) \
    SIP_7_REG(dev,MLL.multiTargetVPortMap.subRangeArr[instance].backupChunks[chunk].backupCfgValue)

/* MLL - multi-target vPort - sub range backup Cfg Mask (per instance,chunk)*/
#define SMEM_SIP7_MLL_MULTI_TARGET_VPORT_SUB_RANGE_BACKUP_CFG_MASK_REG(dev,instance,chunk) \
    SIP_7_REG(dev,MLL.multiTargetVPortMap.subRangeArr[instance].backupChunks[chunk].backupCfgMask)

/*  MLL multi Target Vport Mapping - multi Target Vport Ingress Device */
#define SMEM_SIP7_MLL_MULTI_TARGET_VPORT_MAPPING_INGRESS_DEVICE_REG(dev) \
    SIP_7_REG(dev, MLL.multiTargetVPortMap.multiTargetVportIngressDevice)

/* Exact match register macros */
#define SMEM_SIP7_TTI_LU_SERIAL_EM_PROFILE_REG(dev, serialEm, index)         \
        SIP_7_REG(dev,TTI_LU.ilmLuConfig[serialEm].ilmLuSerialEmProfile[index])

#define SMEM_SIP7_EMX_CLIENT_CHANNEL_MAPPING_REG(dev, index)         \
        SIP_7_REG(dev,EMX.emChannel[index])

#define SMEM_SIP7_EXACT_MATCH_PROFILE_CONFIG_REG(dev, emUnitNum, index)                      \
        (emUnitNum == SIP7_EXACT_MATCH_UNIT_REDUCED_E)?                                       \
        SIP_7_REG(dev,EMX.ilmExactMatchProfileTable.ilmExactMatchProfileTableConfig[index]): \
        SIP_7_REG(dev,EMX.exactMatchProfilesTable[emUnitNum].exactMatchProfileTableConfig[index])

#define SMEM_SIP7_EXACT_MATCH_PROFILE_TABLE_DEFAULT_ACTION_DATA_REG(dev, emUnitNum, index, wordNum)              \
        (emUnitNum == SIP7_EXACT_MATCH_UNIT_REDUCED_E)?                                                           \
        SIP_7_REG(dev,EMX.ilmExactMatchProfileTable.ilmExactMatchProfileTableDefaultActionData[index][wordNum]): \
        SIP_7_REG(dev,EMX.exactMatchProfilesTable[emUnitNum].exactMatchProfileTableDefaultActionData[index][wordNum])

#define SMEM_SIP7_EXACT_MATCH_PROFILE_TABLE_MASK_DATA_REG(dev, emUnitNum, index, word)               \
        (emUnitNum == SIP7_EXACT_MATCH_UNIT_REDUCED_E)?                                               \
        SIP_7_REG(dev,EMX.ilmExactMatchProfileTable.ilmExactMatchProfileTableMaskData[index][word]): \
        SIP_7_REG(dev,EMX.exactMatchProfilesTable[emUnitNum].exactMatchProfilesTableMaskData[index][word])

#define SMEM_SIP7_EXACT_MATCH_ACTION_ASSIGNMENT_REG(dev, emUnitNum, index, byte)                              \
        (emUnitNum == SIP7_EXACT_MATCH_UNIT_REDUCED_E)?                                                        \
        SIP_7_REG(dev,EMX.ilmExactMatchProfileTable.ilmExactMatchProfileTableReducedActionData[index][byte]): \
        SIP_7_REG(dev,EMX.exactMatchProfilesTable[emUnitNum].exactMatchProfileTableReducedActionData[index][byte])

#define SMEM_SIP7_EXACT_MATCH_PROFILE_10_BYTE_SELECT_REG(dev, emUnitNum, index, word)         \
        SIP_7_REG(dev,EMX.exactMatchProfilesTable[emUnitNum].exactMatchProfileTable10BSelect[index][word])

#define SMEM_SIP7_EXACT_MATCH_GLOBAL_CONFIGURATION_REG(dev, emUnitNum, regName) \
        (emUnitNum == SIP7_EXACT_MATCH_UNIT_REDUCED_E)?                          \
        SIP_7_REG(dev,REDUCED_EM.emGlobalConfiguration.regName):                \
        SIP_7_REG(dev,EXACT_MATCH[emUnitNum].emGlobalConfiguration.regName)

/* Converts key size to number of Bytes  */
#define SMEM_SIP7_EXACT_MATCH_KEY_SIZE_TO_BYTE_MAC(keySize)       \
    (                                                      \
     (keySize) == 0x0 /*SIP7_EXACT_MATCH_KEY_SIZE_5B_E */  ? 5  :   \
     (keySize) == 0x1 /*SIP7_EXACT_MATCH_KEY_SIZE_21B_E*/  ? 21 :   \
     (keySize) == 0x2 /*SIP7_EXACT_MATCH_KEY_SIZE_37B_E*/  ? 37 :   \
     (keySize) == 0x3 /*SIP7_EXACT_MATCH_KEY_SIZE_53B_E*/  ? 53 :   \
     0)

#define SMEM_SIP7_HIDDEN_EXACT_MATCH_PTR(dev, emUnit, entry_index) \
    smemGenericHiddenFindMem(dev,SMEM_GENERIC_HIDDEN_MEM_EXACT_MATCH_TILE_0_TABLE_0_E + emUnit, \
                             entry_index,NULL)

/* QoS and TTL Action Resolution Regs */
#define SMEM_SIP7_TTI_LU_PARALLEL_LOOKUP_QOS_ACTION_REG(dev, lookupNum, index) \
        SIP_7_REG(dev,TTI_LU.ttiLuActionConfig.ttiLuParallelLookupIdxQosActionResolution[lookupNum].qosEntry[index])

#define SMEM_SIP7_TTI_LU_PARALLEL_LOOKUP_TTL_ACTION_REG(dev, lookupNum, index) \
        SIP_7_REG(dev,TTI_LU.ttiLuActionConfig.ttiLuParallelLookupIdxTtlActionResolution[lookupNum].ttlEntry[index])

#define SMEM_SIP7_TTI_LU_SERIAL_LOOKUP_QOS_ACTION_REG(dev, lookupNum, index)   \
        SIP_7_REG(dev,TTI_LU.ttiLuActionConfig.ttiLuIlmLookupIdxQosActionResolution[lookupNum].qosEntry[index])

#define SMEM_SIP7_TTI_LU_SERIAL_LOOKUP_TTL_ACTION_REG(dev, lookupNum, index)   \
        SIP_7_REG(dev,TTI_LU.ttiLuActionConfig.ttiLuIlmLookupIdxTtlActionResolution[lookupNum].ttlEntry[index])

/* Reduced EM Table */
#define SMEM_SIP7_REDUCED_EM_TBL_MEM(dev, index) \
    SMEM_TABLE_ENTRY_INDEX_GET_MAC(dev, reducedEmTable, index)

/* EQ - vport mapping table */
#define SMEM_SIP7_EQ_VPORT_MAPPING_TBL_MEM(dev, index) \
    SMEM_TABLE_ENTRY_INDEX_GET_MAC(dev, eqVPortMapping, index)

/* EQ - Queue Group Profile table */
#define SMEM_SIP7_EQ_QUEUE_GROUP_PROFILE_TBL_MEM(dev, index) \
    SMEM_TABLE_ENTRY_INDEX_GET_MAC(dev, eqQueueGroupProfile, index)

/* EQ - H-ECMP status vector table */
#define SMEM_SIP7_EQ_H_ECMP_STATUS_VECTOR_TBL_MEM(dev, index) \
    SMEM_TABLE_ENTRY_INDEX_GET_MAC(dev, eqHEcmpStatusVector, index)

/* EQ - H-ECMP LTT table */
#define SMEM_SIP7_EQ_H_ECMP_LTT_TBL_MEM(dev, index) \
    SMEM_TABLE_ENTRY_INDEX_GET_MAC(dev, eqHEcmpLttTable, index)

/* EQ - H-ECMP ECMP table */
#define SMEM_SIP7_EQ_H_ECMP_ECMP_TBL_MEM(dev, index) \
    SMEM_TABLE_ENTRY_INDEX_GET_MAC(dev, eqHEcmpEcmpTable, index)

/* EQ - tsEcmp config */
#define SMEM_SIP7_EQ_TSECMP_CONFIG_REG(dev) \
    SIP_7_REG(dev,EQ.tsEcmp.tsecmp_config)

/* EQ - vPort global config */
#define SMEM_SIP7_EQ_VPORT_GLOBAL_CONFIG_REG(dev) \
    SIP_7_REG(dev,EQ.vport.vPort_global_config)

/* EQ - stage Path Info , per stage (0,1,2) */
#define SMEM_SIP7_EQ_STAGE_PATH_INFO_REG(dev,_stage) \
    SIP_7_REG(dev,EQ.GCF.stagePathInfo[_stage])

/* EQ - Circuits base address in E2PHY  */
#define SMEM_SIP7_EQ_CIRCUITS_BASE_ADDRESS_IN_E2PHY_REG(dev) \
    SIP_7_REG(dev,EQ.E2PHY.Circuits_base_address_in_E2PHY)


/* EQ - mll To Other device  */
#define SMEM_SIP7_EQ_MLL_TO_OTHER_DEVICE_REG(dev) \
    SIP_7_REG(dev,EQ.E2PHY.mll_To_Other_device)

/* EQ - Hierarchical ECMP out of sync  */
#define SMEM_SIP7_EQ_HIERARCHICAL_ECMP_OUT_OF_SYNC_REG(dev) \
    SIP_7_REG(dev,EQ.E2PHY.Hierarchical_ECMP_out_of_sync)

/* GDMA Enable Queue Register  */
#define SMEM_SIP7_GDMA_QUEUE_EN_REG(dev, gdmaUnit, ring)                                 \
    SIP_7_REG(dev,CNM.GDMA[gdmaUnit].queueStatusAndControlRegs[ring].queueEnable)

/* GDMA Queue Control Register  */
#define SMEM_SIP7_GDMA_QUEUE_CONTROL_REG(dev, gdmaUnit, ring)                                 \
    SIP_7_REG(dev,CNM.GDMA[gdmaUnit].queueStatusAndControlRegs[ring].queueControl)

/* GDMA Descriptor Pointer */
#define SMEM_SIP7_GDMA_BASE_DESC_POINTER_REG(dev, gdmaUnit, ring) \
    SIP_7_REG(dev,CNM.GDMA[gdmaUnit].queueStatusAndControlRegs[ring].wruSGDPointer)

/* GDMA rx buf byte cnt (this is NOT counter , but config for 'counting' )*/
#define SMEM_SIP7_GDMA_RX_BUF_BYTE_CNT_REG(dev, gdmaUnit, ring) \
    SIP_7_REG(dev,CNM.GDMA[gdmaUnit].queueStatusAndControlRegs[ring].rxBufferByteCount)

/* GDMA Chain Size */
#define SMEM_SIP7_GDMA_CHAIN_SIZE_REG(dev, gdmaUnit, ring) \
    SIP_7_REG(dev,CNM.GDMA[gdmaUnit].sgdAxiControlRegs[ring].sgdChainSize)

/* GDMA payload Buffer Address High */
#define SMEM_SIP7_GDMA_PAYLOAD_BUFFER_ADDRESS_HIGH_REG(dev, gdmaUnit, ring) \
    SIP_7_REG(dev,CNM.GDMA[gdmaUnit].sgdAxiControlRegs[ring].payloadBufferAddressHigh)

/* GDMA chain Address low */
#define SMEM_SIP7_GDMA_CHAIN_ADDRESS_LOW_REG(dev, gdmaUnit, ring) \
    SIP_7_REG(dev,CNM.GDMA[gdmaUnit].sgdAxiControlRegs[ring].sgdChainBaseAddressLow)

/* GDMA chain Address high */
#define SMEM_SIP7_GDMA_CHAIN_ADDRESS_HIGH_REG(dev, gdmaUnit, ring) \
    SIP_7_REG(dev,CNM.GDMA[gdmaUnit].sgdAxiControlRegs[ring].sgdChainBaseAddressHigh)

/* GDMA RING Interrupt Mask Register*/
#define SMEM_SIP7_GDMA_RING_INT_MASK_REG(dev,gdmaUnit, ring)                   \
    SIP_7_REG(dev,CNM.GDMA[gdmaUnit].interruptAndDebugRegs.gdmaQInterruptMask[ring])

/* GDMA RING Interrupt Registers  */
#define SMEM_SIP7_GDMA_RING_INT_CAUSE_REG(dev,gdmaUnit, ring)                  \
    SIP_7_REG(dev,CNM.GDMA[gdmaUnit].interruptAndDebugRegs.gdmaQInterruptCause[ring])

/* GDMA Stat packet/message counter low */
#define SMEM_SIP7_GDMA_STAT_COUNTER_REG(dev, gdmaUnit, ring) \
    SIP_7_REG(dev,CNM.GDMA[gdmaUnit].queueStatusAndControlRegs[ring].stat_counter[0])

/* GDMA Stat byte counter (of packet/message) low */
#define SMEM_SIP7_GDMA_STAT_BYTE_COUNTER_REG(dev, gdmaUnit, ring) \
    SIP_7_REG(dev,CNM.GDMA[gdmaUnit].queueStatusAndControlRegs[ring].stat_byte_counter[0])


/* GDMA RX drop counter low */
#define SMEM_SIP7_GDMA_RX_DROP_COUNTER_REG(dev, gdmaUnit, ring) \
    SIP_7_REG(dev,CNM.GDMA[gdmaUnit].queueStatusAndControlRegs[ring].RX_Drop_counter[0])

/* GDMA free SGDs Status Register */
#define SMEM_SIP7_GDMA_FREE_SGDS_STATUS_REG(dev, gdmaUnit, ring) \
    SIP_7_REG(dev,CNM.GDMA[gdmaUnit].queueStatusAndControlRegs[ring].freeSGDsStatusRegister)

/* GDMA PDI Master Control Register */
#define SMEM_SIP7_GDMA_PDI_MASTER_CONTROL_REG(dev, gdmaUnit) \
    SIP_7_REG(dev,CNM.GDMA[gdmaUnit].pdiMasterRegs.pdiMasterControl)


/* GDMA Dispatcher - Packet Debug Suffix */
#define SMEM_SIP7_GDMA_PACKET_DEBUG_SUFFIX_REG(dev) \
    SIP_7_REG(dev, CNM.GDMA_Dispatcher.packetDebugSuffix)

/* GDMA Dispatcher - gdmaDispatcherControl*/
#define SMEM_SIP7_GDMA_DISPATCHER_CONTROL_REG(dev) \
    SIP_7_REG(dev,CNM.GDMA_Dispatcher.gdmaDispatcherControl)

/* GDMA Dispatcher - DSA Field Offsets */
#define SMEM_SIP7_GDMA_DISPATCHER_DSA_FIELD_OFFSETS_REG(dev) \
    SIP_7_REG(dev,CNM.GDMA_Dispatcher.dsaFieldOffsets)


/* GDMA Dispatcher - Cpu Code To Gdma Q */
#define SMEM_SIP7_GDMA_CPU_CODE_TO_GDMA_Q_TBL_MEM(dev, entry_index) \
    SMEM_TABLE_ENTRY_INDEX_GET_MAC(dev,gdmaDispatcherCpuCodeToGdmaQ,entry_index)

/* HA - Analyzers Target Circuits */
#define SMEM_SIP7_HA_ANALYZERS_TARGET_CIRCUITS_TBL_MEM(dev, index) \
      SMEM_TABLE_ENTRY_INDEX_GET_MAC(dev, haAnalyzersTargetCircuits, index)

/* HA - QoS Mapping */
#define SMEM_SIP7_HA_QOS_MAPPING_TBL_MEM(dev, index) \
      SMEM_TABLE_ENTRY_INDEX_GET_MAC(dev, haQosMapping, index)

/* HA - Ingress Physical port 1 HA Attributes */
#define SMEM_SIP7_HA_INGRESS_PHYSICAL_PORT_2_ATTRIBUTES_TBL_MEM(dev, ePort) \
    SMEM_TABLE_ENTRY_INDEX_GET_MAC(dev, haIngressPhyPort2, ePort)

/* HA - Ingress EPort Attribute Table 1 */
#define SMEM_SIP7_HA_EGRESS_EPORT_ATTRIBUTE_1_TBL_MEM(dev, ePort) \
    SMEM_TABLE_ENTRY_INDEX_GET_MAC(dev, haIngressEPortAttr1, ePort)

/* HA - Ingress EPort Attribute Table 2 */
#define SMEM_SIP7_HA_EGRESS_EPORT_ATTRIBUTE_2_TBL_MEM(dev, ePort) \
    SMEM_TABLE_ENTRY_INDEX_GET_MAC(dev, haIngressEPortAttr2, ePort)

/* HA - Target Circuits */
#define SMEM_SIP7_HA_TARGET_CIRCUITS_TBL_MEM(dev, index) \
      SMEM_TABLE_ENTRY_INDEX_GET_MAC(dev, haTargetCircuits, index)

/* HA - Queue Mapping */
#define SMEM_SIP7_HA_QUEUE_MAPPING_TBL_MEM(dev, index) \
      SMEM_TABLE_ENTRY_INDEX_GET_MAC(dev, haQueueMapping, index)

/* HA - Header Fields Table 0 */
#define SMEM_SIP7_HA_HEADER_FIELDS_0_TBL_MEM(dev, index) \
      SMEM_TABLE_ENTRY_INDEX_GET_MAC(dev, haHeaderFieldsTable0, index)

/* HA - Header Fields Table 1 */
#define SMEM_SIP7_HA_HEADER_FIELDS_1_TBL_MEM(dev, index) \
      SMEM_TABLE_ENTRY_INDEX_GET_MAC(dev, haHeaderFieldsTable1, index)

/* HA MPLS Ethertypes Configuration Register */
#define SMEM_SIP7_HA_MPLS_ETHERTYPES_REG(dev, idx)          \
    SMEM_CHT_MAC_REG_DB_SIP5_GET(dev)->HA.MPLSEtherType[idx]

/* EGF-EFT : eVlan table */
#define SMEM_SIP7_EGF_EFT_EVLAN_TBL_MEM(dev, eVlan) \
      SMEM_TABLE_ENTRY_INDEX_GET_MAC(dev, egfEftEVlanAttributesTable, eVlan)

/* RFU : device Id register  (moved from the MG unit that not exists any more) */
#define SMEM_SIP7_DEVICE_ID_REG(dev)     \
    SIP_7_REG(dev,CNM.RFU.Misc.deviceID)

/* Router Global Ctrl - generic Classification Field Dip Lookup Nhe Fetched */
#define SMEM_SIP7_IPVX_ROUTER_GLOBAL_CTRL_GCF_DIP_LOOKUP_NHE_FETCHED_REG(dev) \
    SIP_7_REG(dev, IPvX.routerGlobalCtrl.genericClassificationFieldDipLookupNheFetched)

/* Router Global Ctrl - generic Classification Field Dip Lookup prifix match */
#define SMEM_SIP7_IPVX_ROUTER_GLOBAL_CTRL_GCF_DIP_LOOKUP_PREFIX_MATCH_REG(dev) \
    SIP_7_REG(dev, IPvX.routerGlobalCtrl.genericClassificationFieldDipLookupPrefixMatch)

/* Router Global Ctrl - generic Classification Field Dip Lookup prifix match */
#define SMEM_SIP7_IPVX_ROUTER_GLOBAL_CTRL_GCF_SIP_LOOKUP_PREFIX_MATCH_REG(dev) \
    SIP_7_REG(dev, IPvX.routerGlobalCtrl.genericClassificationFieldSipLookupPrefixMatch)

/* Router Global Ctrl - router UC Global Cfg Value */
#define SMEM_SIP7_IPVX_ROUTER_GLOBAL_CTRL_ROUTER_UC_GLOBAL_CFG_VALUE_REG(dev, entry_indx) \
    SMEM_CHT_MAC_REG_DB_SIP5_GET(dev)->IPvX.routerGlobalCtrl.routerUcGlobalCfgValue[(entry_indx)]

/* Router Global Ctrl - router UC Global Cfg Value */
#define SMEM_SIP7_IPVX_ROUTER_GLOBAL_CTRL_ROUTER_UC_GLOBAL_CFG_MASK_REG(dev, entry_indx) \
    SMEM_CHT_MAC_REG_DB_SIP5_GET(dev)->IPvX.routerGlobalCtrl.routerUcGlobalCfgMask[(entry_indx)]


/* Router Global Ctrl - router UC Global Cfg Value */
#define SMEM_SIP7_IPVX_ROUTER_GLOBAL_CTRL_ROUTER_UC_GLOBAL_CFG_MASK_REG(dev, entry_indx) \
    SMEM_CHT_MAC_REG_DB_SIP5_GET(dev)->IPvX.routerGlobalCtrl.routerUcGlobalCfgMask[(entry_indx)]

/*  MLL Global Ctrl - etree Configuration */
#define SMEM_SIP7_MLL_GLOBAL_CTRL_ETREE_CONFIG_REG(dev) \
    SIP_7_REG(dev, MLL.MLLGlobalCtrl.etreeConfiguration)

/* MLL Global Ctrl - IP Mll Global Gcf Config Value */
#define SMEM_SIP7_MLL_GLOBAL_CTRL_IP_MLL_GLOBAL_CONFIG_VALUE_REG(dev, reg_instace) \
    SMEM_CHT_MAC_REG_DB_SIP5_GET(dev)->MLL.MLLGlobalCtrl.ipMllGlobalGcfConfigValue[(reg_instace)]

/* MLL Global Ctrl - IP Mll Global Gcf Config Value */
#define SMEM_SIP7_MLL_GLOBAL_CTRL_IP_MLL_GLOBAL_CONFIG_MASK_REG(dev, reg_instace) \
    SMEM_CHT_MAC_REG_DB_SIP5_GET(dev)->MLL.MLLGlobalCtrl.ipMllGlobalGcfConfigMask[(reg_instace)]

/*  MLL Global Ctrl - bier Configuration*/
#define SMEM_SIP7_MLL_GLOBAL_CTRL_BIER_CONFIG_REG(dev) \
    SIP_7_REG(dev, MLL.MLLGlobalCtrl.bierConfiguration)

/* TTI - global registers */
#define SIP7_TTI_REG_MAC(dev, regName) \
    SIP_7_REG(dev, TTI.regName)

/* TTI - TTI Unit Global Configurations Ext4 */
#define SMEM_SIP7_TTI_UNIT_GLOBAL_CONFIG_EXT4_REG(dev)        \
        SIP7_TTI_REG_MAC(dev, TTIUnitGlobalConfigs.TTIUnitGlobalConfigExt4)

/* TTI - TTI Unit Global Configurations Ext5 */
#define SMEM_SIP7_TTI_UNIT_GLOBAL_CONFIG_EXT5_REG(dev)        \
        SIP7_TTI_REG_MAC(dev, TTIUnitGlobalConfigs.TTIUnitGlobalConfigExt5)

/* EGF_QAG -  Queue Map Table */
#define SMEM_SIP7_EGF_QAG_QUEUE_MAP_TBL_MEM(dev, index) \
        SMEM_TABLE_ENTRY_INDEX_GET_MAC(dev, egfQueueMapTable, index)

/* EQ - TRUNK ECMP table */
#define SMEM_SIP7_EQ_TRUNK_ECMP_TBL_MEM(dev, index) \
    SMEM_TABLE_ENTRY_INDEX_GET_MAC(dev, eqTrunkEcmp, index)

/* eft : global hash Selection register */
#define SMEM_SIP7_EFT_GLOBAL_HASH_SELECTION_REG(dev)                 \
    SIP_7_REG(dev,EGF_eft.egrFilterConfigs.globalHashSelection)


/* HBU - port mapping table */
#define SMEM_SIP7_HBU_PORT_MAPPING_TBL_MEM(dev, index) \
    SMEM_TABLE_ENTRY_INDEX_GET_MAC(dev, hbuPortMappingTable, index)


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  /* __sregSeahawkh */

