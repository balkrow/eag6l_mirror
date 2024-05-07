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
* @file sregCheetah3.h
*
* @brief Defines for Cheetah3 memory registers access.
*
* @version   56
********************************************************************************
*/
#ifndef __sregCheetah3h
#define __sregCheetah3h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* ingress PCL Configuration Table Entry<n> - Base address*/
#define SMEM_CHT3_IPCL_CONFIG_TBL_MEM                               (0x0B840000)

    /* Policer Counters Table Data0 Access Register */
#define SMEM_CHT3_POLICER_CNT_TBL_DATA0_REG(dev)\
(SMEM_CHT_MAC_REG_DB_GET(dev))->IPLR.ingrPolicerTableAccessData[0]

/* VLAN Table 4K Entries */
#define SMEM_CHT3_VLAN_TBL_MEM                                      (0x0BC00000)

/* Span State Group<n> Entry (0<=n<256) * 2 words in step 0x8 */
#define SMEM_CHT3_STP_TBL_MEM                                       (0x0BB00000)


#define SMEM_CHT3_PCL_CONFIG_TBL_MEM(entry_indx)                         \
            (SMEM_CHT3_IPCL_CONFIG_TBL_MEM + (entry_indx * 0x8))

/* address of the policy tcam table */
#define SMEM_CHT3_PCL_TCAM_DATA_TBL_MEM(dev, entry, word) \
    SMEM_CHT_PCL_TCAM_TBL_MEM(dev, entry, word)

/* address of the policy tcam ctrl table */
#define SMEM_CHT3_PCL_TCAM_CTRL_TBL_MEM(dev, entry, word) \
    (SMEM_CHT3_PCL_TCAM_DATA_TBL_MEM(dev, entry, word) + 0x4)

/* address of the policy tcam data mask table */
#define SMEM_CHT3_PCL_TCAM_MASK_TBL_MEM(dev, entry, word) \
    (SMEM_CHT3_PCL_TCAM_DATA_TBL_MEM(dev, entry, word) + 0x20)

/* address of the policy tcam ctrl mask table */
#define SMEM_CHT3_PCL_TCAM_CTRL_MASK_TBL_MEM(dev, entry, word) \
    (SMEM_CHT3_PCL_TCAM_DATA_TBL_MEM(dev, entry, word) + 0x24)

/* address of the routing tcam table */
#define SSMEM_CHT3_TCAM_X_DATA_TBL_MEM(dev)       \
    SMEM_TABLE_ENTRY_2_PARAMS_INDEX_GET_MAC(dev,routerTcam,0,0)

/* address of the routing tcam ctrl table */
#define SSMEM_CHT3_TCAM_ROUTER_TT_X_CONTROL_TBL_MEM(dev)          (SSMEM_CHT3_TCAM_X_DATA_TBL_MEM(dev)+0x4)

/* address of the routing tcam data mask table */
#define SSMEM_CHT3_TCAM_Y_DATA_TBL_MEM(dev)                       (SSMEM_CHT3_TCAM_X_DATA_TBL_MEM(dev)+0x8)

/* address of the routing tcam ctrl mask table */
#define SSMEM_CHT3_TCAM_ROUTER_TT_Y_CONTROL_TBL_MEM(dev)          (SSMEM_CHT3_TCAM_X_DATA_TBL_MEM(dev)+0xc)

/* cheetah3 pcl counter ??? */
#define SMEM_CHT3_PCL_RULE_MATCH_TBL_MEM                            (0x0B801000)

/* Egress Policy Global Registry */
#define SMEM_CHT3_EPCL_GLOBAL_REG(dev) \
    (SMEM_CHT_IS_SIP5_GET(dev) ? SMEM_CHT_MAC_REG_DB_SIP5_GET(dev)->EPCL.egrPolicyGlobalConfig : \
        SMEM_XCAT_EPCL_GLOBAL_REG(dev))

/* Egress Policy TO CPU Configuration */
#define SMEM_CHT3_EPCL_TO_CPU_CONFIG_REG(dev) \
        SMEM_XCAT_EPCL_TO_CPU_CONFIG_REG(dev)

/* Egress Policy FROM CPU -data - Configuration */
#define SMEM_CHT3_EPCL_FROM_CPU_DATA_CONFIG_REG(dev) \
        SMEM_XCAT_EPCL_FROM_CPU_DATA_CONFIG_REG(dev)

/* Egress Policy FROM CPU -control - Configuration */
#define SMEM_CHT3_EPCL_FROM_CPU_CONFIG_REG(dev) \
        SMEM_XCAT_EPCL_FROM_CPU_CONFIG_REG(dev)

/* Egress Policy TO ANALYZER configuration */
#define SMEM_CHT3_EPCL_TO_ANALYZER_CONFIG_REG(dev) \
        SMEM_XCAT_EPCL_TO_ANALYZER_CONFIG_REG(dev)

/* Egress Policy Forward configuration ??? */
#define SMEM_CHT3_EPCL_TS_FORWARD_CONFIG_REG(dev) \
        SMEM_XCAT_EPCL_TS_FORWARD_CONFIG_REG(dev)

/* Egress Policy Forward configuration ??? */
#define SMEM_CHT3_EPCL_NTS_FORWARD_CONFIG_REG(dev) \
        SMEM_XCAT_EPCL_NTS_FORWARD_CONFIG_REG(dev)


/* Egress Policy Configuration Table Access mode configuration */
#define SMEM_CHT3_EPCL_TBL_ACCESS_MODE_CONFIG_REG(dev) \
        SMEM_XCAT_EPCL_TBL_ACCESS_MODE_CONFIG_REG(dev)


/* UDP Broadcast Destination Port<n> Configuration Table */
#define SMEM_CHT3_UDP_BROADCAST_DEST_PORT_TBL_MEM(entry_index) \
            0x02000500 + ((entry_index) * 4)

/* Ingress VLAN translation table */
#define SMEM_CHT3_INGRESS_VLAN_TRANSLATION_TBL_MEM                  (0x0b804000)

/* Egress VLAN translation table */
#define SMEM_CHT3_EGRESS_VLAN_TRANSLATION_TBL_MEM                   (0x07fc0000)

/* FDB Update Message Queue Configuration Registers ??? */
#define SMEM_CHT3_FU_Q_BASE_ADDR_REG(dev)\
(SMEM_CHT_MAC_REG_DB_GET(dev))->MG.addrUpdateQueueConfig.FUAddrUpdateQueueCtrl

#define SMEM_CHT3_ROUTER_ACCESS_DATA_CTRL_REG(dev)\
(SMEM_CHT_MAC_REG_DB_GET(dev))->TCCUpperIPvX.TCCIPvX.routerTCAMAccessCtrlReg0

/* not used by SIP5 devices */
#define SMEM_CHT3_MLL_REGISTERS_BASE_TBL_MEM(dev, offset)      \
    (offset) + (dev)->memUnitBaseAddrInfo.mllRegBase

#define SMEM_CHT3_MLL_GLB_CONTROL_REG(dev)\
    SMEM_CHT3_MLL_REGISTERS_BASE_TBL_MEM(dev,0x0)

/* QoS Profile to Route Block Offset Entry ??? */
#define SMEM_CHT3_QOS_ROUTING_TBL_MEM(dev, entry_indx) \
        (0x02800300 + (0x4 * (entry_indx)))

/* Unicast Multicast Router Next Hop Entry ??? */
#define SMEM_CHT3_NEXT_HOP_ENTRY_TBL_MEM(entry_indx)          \
        (0x02B00000 + (0x10 * (entry_indx)))

/* IPv6 Scope Prefix Table */
#define SMEM_CHT3_ROUTER_SCOPE_PREFIX_TBL_MEM(entry_indx) \
        (0x02800250 + (0x4 * (entry_indx)))
/* IPv6 Unicast Scope level Table */
#define SMEM_CHT3_ROUTER_UNICAST_SCOPE_LEVEL_TBL_MEM(entry_indx) \
        (0x02800260 + (0x4 * (entry_indx)))
/* IPv6 Unicast Scope Command Table */
#define SMEM_CHT3_ROUTER_UNICAST_SCOPE_COMMAND_TBL_MEM(entry_indx) \
        (0x02800278 + (0x4 * (entry_indx)))
/* IPv6 Multicast Scope Command Table */
#define SMEM_CHT3_ROUTER_MULTICAST_SCOPE_COMMAND_TBL_MEM(entry_indx) \
        (0x02800288 + (0x4 * (entry_indx)))
/*  Multicast replicator Table  */
#define SMEM_CHT3_ROUTER_MULTICAST_LIST_TBL_MEM(dev,entry_indx) \
        SMEM_TABLE_ENTRY_INDEX_GET_MAC(dev,mllTable,entry_indx)

#define SMEM_CHT3_VRFID_TBL_MEM(dev, entry_indx)  \
        SMEM_TABLE_ENTRY_INDEX_GET_MAC(dev,vrfId,entry_indx)


#define SMEM_CHT3_QOS_RESERV_OFFSET_INDEX_CNS                        (128)

/* FU upload messages Miscellaneous to CPU has exceeded its configured
  threshold , Cht2 */
#define SMEM_CHT3_MISCELLANEOUS_FU_FULL_INT                          (1 << 11)

/* FU upload Message to CPU is ready in the Miscellaneous  */
#define SMEM_CHT3_MISCELLANEOUS_FU_MSG_PENDING_INT                   (1 << 12)

/* Egress PCL Configuration Table Entry<n> (0<=n<4159)          */
/* offset formula : 0x07F00000 + entry<n>*0x4                   */
#define SMEM_CHT3_EPCL_CONFIG_TBL_MEM(dev, entry_indx)          \
    (dev->tablesInfo.epclConfigTable.commonInfo.baseAddress != 0 ? SMEM_TABLE_ENTRY_INDEX_GET_MAC(dev,epclConfigTable,entry_indx) : \
        SMEM_XCAT_EPCL_CONFIG_TBL_MEM(dev, entry_indx))

/* Tunnel Action Entry<n> (0<=n<5120)  */
#define SMEM_CHT3_TUNNEL_ACTION_TBL_MEM(dev,entry_indx) \
    SMEM_CHT3_TUNNEL_ACTION_WORD_TBL_MEM(dev,entry_indx,0)

#define SMEM_CHT3_TUNNEL_ACTION_WORD_TBL_MEM(dev,entry_indx,word) \
    SMEM_TABLE_ENTRY_2_PARAMS_INDEX_GET_MAC(dev,ttiAction,entry_indx,word)


/* Action Table and Policy TCAM Access Data Register */
#define SMEM_CHT3_PCL_ACTION_TCAM_DATA_TBL_MEM(dev,entry_indx) \
        SMEM_TABLE_ENTRY_INDEX_GET_MAC(dev,pclActionTcamData,entry_indx)

/* Policy TCAM indirect access Data Parameters1 register */
#define SMEM_CHT3_PCL_ACTION_TCAM_DATA_PARAM1_REG(dev)  \
    (SMEM_CHT_MAC_REG_DB_GET(dev))->TCCLowerIPCL[0].TCCIPCL.actionTableAndPolicyTCAMAccessCtrlParameters1


/* Policy TCAM indirect access Data Parameters2 register */
#define SMEM_CHT3_PCL_ACTION_TCAM_DATA_PARAM2_REG(dev)  \
(SMEM_CHT_MAC_REG_DB_GET(dev))->TCCLowerIPCL[0].TCCIPCL.actionTableAndPolicyTCAMAccessCtrlParameters2

/* router TCAM Entry<n> Data Word <0> (0<=n<5120, 0<=m<3) */
#define SMEM_CHT3_TCAM_X_DATA_TBL_MEM(dev,entry_indx) \
        (SSMEM_CHT3_TCAM_X_DATA_TBL_MEM(dev) + (0x10 * (entry_indx)))

/* router TCAM Entry<n> control Word <0> (0<=n<5120 0<=m<3) */
#define SMEM_CHT3_TCAM_ROUTER_TT_X_CONTROL_TBL_MEM(dev,entry_indx) \
        (SSMEM_CHT3_TCAM_ROUTER_TT_X_CONTROL_TBL_MEM(dev) + (0x10 * (entry_indx)))

/* router TCAM Entry<n> Data Word Mask <0> (0<=n<5120, 0<=m<3) */
#define SMEM_CHT3_TCAM_Y_DATA_TBL_MEM(dev,entry_indx) \
        (SSMEM_CHT3_TCAM_Y_DATA_TBL_MEM(dev) + (0x10 * (entry_indx)) )

/* router TCAM Entry<n> control Word <0> (0<=n<5120 0<=m<3) */
#define SMEM_CHT3_TCAM_ROUTER_TT_Y_CONTROL_TBL_MEM(dev,entry_indx) \
        (SSMEM_CHT3_TCAM_ROUTER_TT_Y_CONTROL_TBL_MEM(dev) + (0x10 * (entry_indx)))


/*  Port<n> Status Register0 (0<=n<24 , CPUPort = 63) */
#define SMEM_CHT3_PORT_STATUS0_REG(dev, port)\
    (((port) == SNET_CHT_CPU_PORT_CNS && ((dev)->cpuPortNoSpecialMac == 0)) ? \
       (SMEM_CHT_MAC_REG_DB_GET(dev))->GOP.ports.gigPort63CPU.portStatusReg0 : \
            IS_CHT_HYPER_GIGA_PORT(dev,port) ? \
                (SMEM_CHT_MAC_REG_DB_GET(dev))->GOP.ports.XLGIP[port].portStatus  : \
                (SMEM_CHT_MAC_REG_DB_GET(dev))->GOP.ports.gigPort[port].portStatusReg0)

#define SNET_CHT3_TCP_SANITY_CHECK_CMD(confVal, command) \
        *command = (confVal) ? \
            SKERNEL_EXT_PKT_CMD_HARD_DROP_E : \
            SKERNEL_EXT_PKT_CMD_FORWARD_E

/* Header Alteration VLAN Translation Enable Register Address: 0x07800130 */
#define SMEM_CHT3_HA_VLAN_TRANS_EN_REG(dev)   \
        SMEM_XCAT_HA_VLAN_TRANS_EN_REG(dev)


#define SMEM_CHT3_INGR_VLAN_TRANS_TBL_MEM(dev, entry_index) \
    SMEM_TABLE_ENTRY_INDEX_GET_MAC(dev,ingressVlanTranslation,entry_index)


#define SMEM_CHT3_EGR_VLAN_TRANS_TBL_MEM(dev, entry_index) \
    SMEM_TABLE_ENTRY_INDEX_GET_MAC(dev,egressVlanTranslation,entry_index)


#define SMEM_CHT3_MAC_TO_ME_TBL_MEM(dev, entry_index) \
    SMEM_TABLE_ENTRY_INDEX_GET_MAC(dev,macToMe,entry_index)

/* Fast Stack Register */
#define SMEM_CHT3_FAST_STACK_REG(dev) \
        SMEM_XCAT_FAST_STACK_REG(dev)

/* Fast loop port Register */
#define SMEM_CHT3_LOOP_PORT_REG(dev)\
        SMEM_XCAT_LOOP_PORT_REG(dev)

/* Fast CFM Ethertype Register */
#define SMEM_CHT3_CFM_ETHERTYPE_REG(dev) \
    (SKERNEL_IS_LION_REVISON_B0_DEV(dev) ? \
        SMEM_LION_CFM_ETHERTYPE_REG(dev) : \
            SMEM_XCAT_SPECIAL_ETHERTYPES_REG(dev) )


/* 802.3ah OAM Loopback Enable Per Port  */
#define SMEM_CHT_TXQ_OAM_FIRST_REG_LOOPBACK_FILTER_EN_REG(dev)\
    (SMEM_CHT_IS_SIP5_GET(dev) ? SMEM_CHT_MAC_REG_DB_SIP5_GET(dev)->EGF_eft.egrFilterConfigs.OAMLoopbackFilterEn[0] : \
    (   \
      (dev)->txqRevision == 0 ?                                     \
        (SMEM_CHT_MAC_REG_DB_GET(dev))->egrAndTxqVer0.oam8023ahLoopbackEnablePerPortReg.oam8023ahLoopbackEnablePerPortReg : \
        (SMEM_CHT_MAC_REG_DB_GET(dev))->egrAndTxqVer1.TXQ.TXQ_EGR[0].OAMLoopbackFilterEn[0]))

/* Fast DP2CFI Register */
#define SMEM_CHT3_DP2CFI_REG(dev) \
    (SMEM_CHT_MAC_REG_DB_GET(dev))->egrAndTxqVer0.DPToCFIMap.DP2CFIReg

/* Fast DP2CFI Enable Register\DP To CFI Remap En <%n> */
#define SMEM_CHT3_DP2CFI_ENABLE_REG(dev)\
    (SMEM_CHT_IS_SIP5_GET(dev) ? SMEM_CHT_MAC_REG_DB_SIP5_GET(dev)->EGF_qag.distributor.DPRemap[0] : \
      (dev)->txqRevision == 0 ?                                     \
        (SMEM_CHT_MAC_REG_DB_GET(dev))->egrAndTxqVer0.DPToCFIMap.DP2CFIEnablePerPortReg : \
        (SMEM_CHT_MAC_REG_DB_GET(dev))->egrAndTxqVer1.TXQ.TXQ_Queue.CongestionNotification.DPToCFIRemapEn[0])

/* Ingress Policer Port Metering Enable */
#define SMEM_CHT3_POLICER_PORT_METER_REG(dev, cycle, port) \
            SMEM_XCAT_POLICER_PORT_METER_REG(dev, cycle, port)

/* Ingress Policer Management Counters Memory */
#define SMEM_CHT3_POLICER_MNG_CNT_TBL_MEM(dev, cycle, counter) \
            SMEM_XCAT_POLICER_MNG_CNT_TBL_MEM(dev, cycle, counter)

#define SMEM_CHT3_IPLR_MNG_CNT_SET_GREEN_TBL_MEM(dev,counter) \
        (SMEM_CHT3_POLICER_MNG_CNT_TBL_MEM(dev, 0, counter) + ((counter) * 0x20))

#define SMEM_CHT3_IPLR_MNG_CNT_SET_YELLOW_TBL_MEM(dev,counter) \
        (SMEM_CHT3_POLICER_MNG_CNT_TBL_MEM(dev,  0, counter) + 0x8 + ((counter) * 0x20))

#define SMEM_CHT3_IPLR_MNG_CNT_SET_RED_TBL_MEM(dev,counter) \
        (SMEM_CHT3_POLICER_MNG_CNT_TBL_MEM(dev,  0, counter) + 0x10 + ((counter) * 0x20))

#define SMEM_CHT3_IPLR_MNG_CNT_SET_DROP_TBL_MEM(dev,counter) \
        (SMEM_CHT3_POLICER_MNG_CNT_TBL_MEM(dev, 0, counter) + 0x18 + ((counter) * 0x20))

/* Ingress Policer Initial DP<n> */
#define SMEM_CHT3_POLICER_INITIAL_DP_REG(dev, cycle,qosProfile) \
        SMEM_XCAT_POLICER_INITIAL_DP_REG(dev, cycle,qosProfile)

/* CNC Fast Dump Trigger Register */
#define SMEM_CHT3_CNC_FAST_DUMP_TRIG_REG(dev,_cncUnitIndex)                            \
SMEM_CHT_IS_SIP5_GET(dev) ? SMEM_CHT_MAC_REG_DB_SIP5_GET(dev)->CNC[_cncUnitIndex].globalRegs.CNCFastDumpTriggerReg : \
(SMEM_CHT_MAC_REG_DB_GET(dev))->centralizedCntrs[_cncUnitIndex].globalRegs.CNCFastDumpTriggerReg

/* CNC Interrupt Cause Register */
#define SMEM_CHT3_CNC_INTR_CAUSE_REG(dev,_cncUnitIndex)                                \
SMEM_CHT_IS_SIP5_GET(dev) ? SMEM_CHT_MAC_REG_DB_SIP5_GET(dev)->CNC[_cncUnitIndex].globalRegs.CNCInterruptSummaryCauseReg : \
(SMEM_CHT_MAC_REG_DB_GET(dev))->centralizedCntrs[_cncUnitIndex].globalRegs.CNCInterruptSummaryCauseReg

/* CNC Interrupt Mask Register */
#define SMEM_CHT3_CNC_INTR_MASK_REG(dev,_cncUnitIndex)                                 \
SMEM_CHT_IS_SIP5_GET(dev) ? SMEM_CHT_MAC_REG_DB_SIP5_GET(dev)->CNC[_cncUnitIndex].globalRegs.CNCInterruptSummaryMaskReg : \
(SMEM_CHT_MAC_REG_DB_GET(dev))->centralizedCntrs[_cncUnitIndex].globalRegs.CNCInterruptSummaryMaskReg

/* CNC Block Configuration Register */
#define SMEM_CHT3_CNC_BLOCK_CNF_REG(dev, block, client,_cncUnitIndex) \
SMEM_CHT_IS_SIP5_GET(dev) ? SMEM_CHT_MAC_REG_DB_SIP5_GET(dev)->CNC[_cncUnitIndex].perBlockRegs.clientEnable.CNCBlockConfigReg0[block][0/*bwc*/] : \
(SMEM_CHT_MAC_REG_DB_GET(dev))->centralizedCntrs[_cncUnitIndex].perBlockRegs. \
                                    CNCBlockConfigReg[block][ SKERNEL_IS_XCAT3_BASED_DEV(dev) ? (client/2) : (client) ]

#define SMEM_CHT_ANY_CNC_BLOCK_CNF_REG(dev, block, client,_cncUnitIndex,_clientInstance) \
        SKERNEL_IS_LION_REVISON_B0_DEV(dev)                    \
        ? SMEM_LION_CNC_BLOCK_CNF0_REG(dev, block,_cncUnitIndex,_clientInstance/*0..2*/)        \
        : SMEM_CHT3_CNC_BLOCK_CNF_REG(dev, block, client,_cncUnitIndex)

/* CNC Block<n> Wraparound Status Register<m> */
#define SMEM_CHT3_CNC_BLOCK_WRAP_AROUND_STATUS_REG(dev, block, entry,_cncUnitIndex) \
SMEM_CHT_IS_SIP5_GET(dev) ? SMEM_CHT_MAC_REG_DB_SIP5_GET(dev)->CNC[_cncUnitIndex].perBlockRegs.wraparound.CNCBlockWraparoundStatusReg[block][entry] : \
(SMEM_CHT_MAC_REG_DB_GET(dev))->centralizedCntrs[_cncUnitIndex].perBlockRegs.CNCBlockWraparoundStatusReg[block][entry]

/* Counters blocks memory */
#define SMEM_CHT3_CNC_BLOCK_COUNTER_TBL_MEM(dev,block,index , cncUnit) \
        SMEM_TABLE_ENTRY_2_PARAMS_INDEX_DUP_TBL_GET_MAC(dev, cncMemory, block , index , cncUnit)


/* L2/L3 Ingress VLAN Counting Enable Register */
#define SMEM_CHT3_CNC_VLAN_EN_REG(dev, port) \
            SMEM_XCAT_CNC_VLAN_EN_REG(dev, port)

/* CNC Counting Mode Register */
#define SMEM_CHT3_CNC_COUNT_MODE_REG(dev) \
            SMEM_XCAT_CNC_COUNT_MODE_REG(dev)

/* CNC modes register  */
#define SMEM_CHT3_CNC_MODES_REG(dev)\
    (SMEM_CHT_IS_SIP5_GET(dev) ? SMEM_CHT_MAC_REG_DB_SIP5_GET(dev)->SIP5_TXQ_QUEUE.peripheralAccess.CNCModes.CNCModes : \
    (   \
      (dev)->txqRevision == 0 ?                                     \
        (SMEM_CHT_MAC_REG_DB_GET(dev))->egrAndTxqVer0.CNCModes.CNCModesReg : \
        (SMEM_CHT_MAC_REG_DB_GET(dev))->egrAndTxqVer1.TXQ.TXQ_Queue.PeripheralAccess.CNCModesReg))

/* Router Header Alteration Global Configuration */
#define SMEM_CHT3_ROUTE_HA_GLB_CNF_REG(dev)\
        SMEM_XCAT_ROUTE_HA_GLB_CNF_REG(dev)

/* CNC Global Configuration Register */
#define SMEM_CHT3_CNC_GLB_CONF_REG(dev,_cncUnitIndex)                                  \
SMEM_CHT_IS_SIP5_GET(dev) ? SMEM_CHT_MAC_REG_DB_SIP5_GET(dev)->CNC[_cncUnitIndex].globalRegs.CNCGlobalConfigReg : \
(SMEM_CHT_MAC_REG_DB_GET(dev))->centralizedCntrs[_cncUnitIndex].globalRegs.CNCGlobalConfigReg
/* CNC Clear By Read Value Register Word 0 */
#define SMEM_CHT3_CNC_ROC_WORD0_REG(dev,_cncUnitIndex)                                 \
SMEM_CHT_IS_SIP5_GET(dev) ? SMEM_CHT_MAC_REG_DB_SIP5_GET(dev)->CNC[_cncUnitIndex].globalRegs.CNCClearByReadValueRegWord0 : \
(SMEM_CHT_MAC_REG_DB_GET(dev))->centralizedCntrs[_cncUnitIndex].globalRegs.CNCClearByReadValueRegWord0

/* L2i - ingress log flow registers */
#define SMEM_CHT3_INGRESS_LOG_BASE_REG(dev)                    \
    (SKERNEL_DEVICE_FAMILY_LION_PORT_GROUP_DEV(dev) ?                    \
        0x02000700:                                                 \
        0x02040700)
                                                                                                                     /* other    , lion     */
#define SMEM_CHT3_INGRESS_LOG_ETHER_TYPE_REG(dev)             (SMEM_CHT3_INGRESS_LOG_BASE_REG(dev) + 0x00) /*0x02040700 0x02000700*/
#define SMEM_CHT3_INGRESS_LOG_DA_LOW_REG(dev)                 (SMEM_CHT3_INGRESS_LOG_BASE_REG(dev) + 0x04) /*0x02040704 0x02000704*/
#define SMEM_CHT3_INGRESS_LOG_DA_HIGH_REG(dev)                (SMEM_CHT3_INGRESS_LOG_BASE_REG(dev) + 0x08) /*0x02040708 0x02000708*/
#define SMEM_CHT3_INGRESS_LOG_DA_LOW_MASK_REG(dev)            (SMEM_CHT3_INGRESS_LOG_BASE_REG(dev) + 0x0c) /*0x0204070c 0x0200070c*/
#define SMEM_CHT3_INGRESS_LOG_DA_HIGH_MASK_REG(dev)           (SMEM_CHT3_INGRESS_LOG_BASE_REG(dev) + 0x10) /*0x02040710 0x02000710*/
#define SMEM_CHT3_INGRESS_LOG_SA_LOW_REG(dev)                 (SMEM_CHT3_INGRESS_LOG_BASE_REG(dev) + 0x14) /*0x02040714 0x02000714*/
#define SMEM_CHT3_INGRESS_LOG_SA_HIGH_REG(dev)                (SMEM_CHT3_INGRESS_LOG_BASE_REG(dev) + 0x18) /*0x02040718 0x02000718*/
#define SMEM_CHT3_INGRESS_LOG_SA_LOW_MASK_REG(dev)            (SMEM_CHT3_INGRESS_LOG_BASE_REG(dev) + 0x1c) /*0x0204071c 0x0200071c*/
#define SMEM_CHT3_INGRESS_LOG_SA_HIGH_MASK_REG(dev)           (SMEM_CHT3_INGRESS_LOG_BASE_REG(dev) + 0x20) /*0x02040720 0x02000720*/
#define SMEM_CHT3_INGRESS_LOG_MAC_DA_RESULT_REG(dev)          (SMEM_CHT3_INGRESS_LOG_BASE_REG(dev) + 0x24) /*0x02040724 0x02000724*/
#define SMEM_CHT3_INGRESS_LOG_MAC_SA_RESULT_REG(dev)          (SMEM_CHT3_INGRESS_LOG_BASE_REG(dev) + 0x28) /*0x02040728 0x02000728*/

/* IPvX - log flow registers */
#define SMEM_CHT3_IP_HIT_LOG_DIP_REG(dev, n)                        \
(SMEM_CHT_MAC_REG_DB_GET(dev))->IPvX.IPLog.IPHitLogDIPAddrReg[n]

#define SMEM_CHT3_IP_HIT_LOG_DIP_MASK_REG(dev, n)                   \
(SMEM_CHT_MAC_REG_DB_GET(dev))->IPvX.IPLog.IPHitLogDIPMaskAddrReg[n]

#define SMEM_CHT3_IP_HIT_LOG_SIP_REG(dev, n)                        \
(SMEM_CHT_MAC_REG_DB_GET(dev))->IPvX.IPLog.IPHitLogSIPAddrReg[n]

#define SMEM_CHT3_IP_HIT_LOG_SIP_MASK_REG(dev, n)                   \
(SMEM_CHT_MAC_REG_DB_GET(dev))->IPvX.IPLog.IPHitLogSIPMaskAddrReg[n]

#define SMEM_CHT3_IP_HIT_LOG_LAYER4_DESTINATION_PORT_REG(dev)       \
(SMEM_CHT_MAC_REG_DB_GET(dev))->IPvX.IPLog.IPHitLogLayer4DestinationPortReg

#define SMEM_CHT3_IP_HIT_LOG_LAYER4_SOURCE_PORT_PORT_REG(dev)       \
(SMEM_CHT_MAC_REG_DB_GET(dev))->IPvX.IPLog.IPHitLogLayer4SourcePortReg

#define SMEM_CHT3_IP_HIT_LOG_PROTOCOL_REG(dev)                      \
(SMEM_CHT_MAC_REG_DB_GET(dev))->IPvX.IPLog.IPHitLogProtocolReg

#define SMEM_CHT3_IP_HIT_LOG_DESTINATION_IP_NHE_ENTRY_REG(dev, n)   \
(SMEM_CHT_MAC_REG_DB_GET(dev))->IPvX.IPLog.IPHitLogDestinationIPNHEEntryRegWord[n]

#define SMEM_CHT3_IP_HIT_LOG_SOURCE_IP_NHE_ENTRY_REG(dev, n)   \
(SMEM_CHT_MAC_REG_DB_GET(dev))->IPvX.IPLog.IPHitLogSourceIPNHEEntryRegWord[n]


/* Secondary Target Port Map  Table */
#define SMEM_CHT3_SECONDARY_TARGET_PORT_MAP_TBL_MEM(dev, port) \
    SMEM_TABLE_ENTRY_INDEX_GET_MAC(dev, secondTargetPort, port)

/* Port<n> Serial Parameters 1 Configuration Register (xCat2)
   n = 0..27 */
#define SMEM_XCAT2_PORT_SERIAL_PARAMS1_REG(dev, port)    \
     (((port) == SNET_CHT_CPU_PORT_CNS && ((dev)->cpuPortNoSpecialMac == 0) ? \
          SMAIN_NOT_VALID_CNS : \
          (SMEM_CHT_MAC_REG_DB_GET(dev))->GOP.ports.gigPort[port].portSerialParameters1Config))

/* Port<n> MAC Control Register0 */
#define SMEM_CHT3_MAC_CONTROL0_REG(dev,port) \
     (((port) == SNET_CHT_CPU_PORT_CNS && ((dev)->cpuPortNoSpecialMac == 0) ? \
             (SMEM_CHT_MAC_REG_DB_GET(dev))->GOP.ports.gigPort63CPU.portMACCtrlReg[0] : \
         IS_CHT_HYPER_GIGA_PORT(dev,port) ? \
             (SMEM_CHT_MAC_REG_DB_GET(dev))->GOP.ports.XLGIP[port].portMACCtrlReg[0]   : \
             (SMEM_CHT_MAC_REG_DB_GET(dev))->GOP.ports.gigPort[port].portMACCtrlReg[0]))

/* Port<n> MAC Control Register1, Cht3 */
#define SMEM_CHT3_MAC_CONTROL1_REG(dev,port)\
            (SMEM_CHT3_MAC_CONTROL0_REG(dev,port) + 0x4)

/* Port<n> MAC Control Register2 */
#define SMEM_CHT3_MAC_CONTROL2_REG(dev,port) \
            (SMEM_CHT3_MAC_CONTROL0_REG(dev,port) + 0x8)

/* Port<n> Interrupt Cause Register , Cht3 */
#define SMEM_CHT3_PORT_INT_CAUSE_REG(dev,port) \
    ( ((port) == SNET_CHT_CPU_PORT_CNS && ((dev)->cpuPortNoSpecialMac == 0)) ? \
       (SMEM_CHT_MAC_REG_DB_GET(dev))->GOP.ports.gigPort63CPU.portInterruptCause : \
            (IS_CHT_HYPER_GIGA_PORT(dev,port)) ? \
                (SMEM_CHT_MAC_REG_DB_GET(dev))->GOP.ports.XLGIP[port].portInterruptCause  : \
                (SMEM_CHT_MAC_REG_DB_GET(dev))->GOP.ports.gigPort[port].portInterruptCause)


/* Port<n> Serial Parameters Configuration , Cht3 */
/* not for XG ports */
#define SMEM_CHT3_PORT_SERIAL_PARAMETERS_REG(dev,port) \
        ((IS_CHT_HYPER_GIGA_PORT(dev,port)) ?  0 :  \
            (   \
                ((port) == SNET_CHT_CPU_PORT_CNS && ((dev)->cpuPortNoSpecialMac == 0)) ? \
                   (SMEM_CHT_MAC_REG_DB_GET(dev))->GOP.ports.gigPort63CPU.portSerialParametersConfig :  \
                   (SMEM_CHT_MAC_REG_DB_GET(dev))->GOP.ports.gigPort[port].portSerialParametersConfig))

/* use bit 20 as "use designated port" other then 63 */
/* in table of SMEM_CHT_CPU_CODE_MEM entry */
#define  SMEM_CHT3_USE_DESIGNATED_PORT_BIT   20  /* bit 20 is first free */


/* 4K entries that indexed by the VID
   each entry hold 12 bits of the mllPointer for linked list of : virtual ports

   each entry:
   bits 0..11 - mllPointer


   NOTE :
   1.   if pointer is NULL (0)  -- it is ignored.
   2. this list is in addition to the physical ports of vid

*/
#define SMEM_CHT3_VID_TO_MLL_MAP_TBL_MEM(vidIndex)\
    (SMEM_CHT3_IEEE_RSRV_MCST_CONF_SECONDARY_TBL_MEM(256) + \
        ((vidIndex)*4))


/* 4K entries that indexed by the VIDX
   each entry hold 12 bits of the mllPointer for linked list of : virtual ports

   each entry:
   bits 0..11 - mllPointer


   NOTE :
   1. if pointer is NULL (0)  -- it is ignored.
   2. this list is in addition to the physical ports of vidx

*/
#define SMEM_CHT3_VIDX_TO_MLL_MAP_ENTRY_TBL_MEM(vidxIndex)\
    (SMEM_CHT3_VID_TO_MLL_MAP_TBL_MEM(4096) + \
        ((vidxIndex)*4))


/* 4 tables to hold the mapping between qosIndex to tid , AckPolicy for 802.11e frames*/
/* 4 tables , each table 128 entries , each entry is single register */
/* profile is the table index 0..3 , qosProfile is the entry index 0..127 */
/* TID - bits 0..3 */
/* AckPolicy - bits 4..5 */
#define SMEM_CHT3_QOS_MAP_TO_802_11E_TID_TBL_MEM(profile, qosProfile)   ((SMEM_CHT3_VIDX_TO_MLL_MAP_ENTRY_TBL_MEM(4096)) + \
                                                                        (((profile) * 128) + (qosProfile))*4)

/* register to hold the <OSM CPU code> used by the TTI action and the TS */
/* single register :
    bits 0..7 - OSM CPU code (8 bits)
*/
#define SMEM_CHT3_OSM_CPU_CODE_TBL_MEM   \
    SMEM_CHT3_QOS_MAP_TO_802_11E_TID_TBL_MEM(4, 0)

/* xCat3 , sip5 , pipe : GIG MAC MIB control register */
#define SMEM_SIP5_GIG_PORT_MIB_COUNT_CONTROL_REG(dev,port)  \
    ((SMEM_CHT_MAC_REG_DB_GET(dev))->GOP.ports.gigPort[(port)].stackPortMIBCntrsCtrl)

#define SMEM_CHT3_HGS_PORT_MIB_COUNT_CONTROL_REG(dev,port)  \
(   \
    (((SMEM_CHT_MAC_REG_DB_GET(dev))->GOP.ports.XLGIP[(port)].xgMIBCountersControl == SMAIN_NOT_VALID_CNS) ? \
      SMEM_SIP5_GIG_PORT_MIB_COUNT_CONTROL_REG(dev,port) : \
      (SMEM_CHT_MAC_REG_DB_GET(dev))->GOP.ports.XLGIP[(port)].xgMIBCountersControl))

#define SMEM_CHT3_HGS_PORT_MAC_MIB_COUNT_REG(dev, port) \
    smemMibCounterAddrByPortGet(dev, port)

#define SMEM_CHT3_HGS_PORT_CAPTURE_MAC_MIB_COUNT_REG(dev, port) \
    (SMEM_CHT3_HGS_PORT_MAC_MIB_COUNT_REG(dev, port) + 0x80)

#define SMEM_CHT3_HGS_PORT_MAC_MIB_COUNT_TABLE0_REG(dev, port) \
    SMEM_TABLE_ENTRY_INDEX_GET_MAC(dev, xgPortMibCounters, port)


#define SMEM_CHT3_HGS_PORT_MAC_MIB_COUNT_TABLE1_REG(dev, index) \
    SMEM_TABLE_ENTRY_INDEX_GET_MAC(dev, xgPortMibCounters_1, index)


/* MLL Out Interface counter Entry */
#define SMEM_CHT3_MLL_OUT_INTERFACE_COUNTER_TBL_MEM(dev,entry_indx)         \
    SMEM_TABLE_ENTRY_INDEX_GET_MAC(dev,mllOutInterfaceCounter,entry_indx)

/* MLL Out Interface counter Entry */
#define SMEM_CHT3_MLL_OUT_INTERFACE_CNFG_TBL_MEM(dev,entry_indx)         \
    SMEM_TABLE_ENTRY_INDEX_GET_MAC(dev,mllOutInterfaceConfig,entry_indx)

/* MLL fifo drop counter Entry  */
#define SMEM_CHT3_MLL_DROP_CNTR_TBL_MEM(dev)                             \
    SMEM_CHT3_MLL_REGISTERS_BASE_TBL_MEM(dev,0x984)

/* disable lookup1 for not routed packets */
#define SMEM_CHT3_PCL_LOOKUP1_NOT_ROUTED_PKTS_REG(dev)              \
    (0x0B800808)

/*XG Ports Interrupt Cause register*/
#define SMEM_CHT3_XG_PORTS_INTERRUPT_CAUSE_REG(dev,treeId)                  \
(SMEM_CHT_MAC_REG_DB_GET(dev))->MG.globalInterrupt[treeId].stackPortsInterruptCause

/*XG Ports Interrupt Cause mask*/
#define SMEM_CHT3_XG_PORTS_INTERRUPT_MASK_REG(dev,treeId)                  \
(SMEM_CHT_MAC_REG_DB_GET(dev))->MG.globalInterrupt[treeId].stackPortsInterruptMask

#define SMEM_CHT3_SERDES_SPEED_1_REG(dev)                           \
(SMEM_CHT_MAC_REG_DB_GET(dev))->GOP.ports.gigPort[0].portMACCtrlReg[0]

/* Ingress Policer Table Access Data */
#define SMEM_CHT3_INGRESS_POLICER_TBL_DATA_TBL_MEM(dev, cycle, entry_index)         \
    SMEM_TABLE_ENTRY_INDEX_DUP_TBL_GET_MAC(dev, policerTblAccessData, entry_index, cycle)


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif  /* __sregCheetah3h */


