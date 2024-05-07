/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* cs802_1cb.c
*
* DESCRIPTION:
*  This file provides case-examples to create a TSN standard 802.1cb applications
*  and use-cases with the CPSS APIs
*
*
* FILE REVISION NUMBER:
*       $Revision: 1.0 $
*
*******************************************************************************/

#include "cs802_1cb.h"


/**
* @internal csRefTsn802_1cb_ProxyNode_main function
* @endinternal
*
* @brief  This function configure a TSN 802.1cb Proxy-Node functionally with 2 flows:
* Flow-1: Send a tagged packet without an R-TAG that will be mapped to a stream-id by the FDB (MAC:00:00:00:00:11:11,VLAN=100)
*         and the stream-id will be map to a VIDX with 2 egress ports.
* ------  Each packet will be transmitted with a generated increment sequence number.
*                                                                                 ------------------------------------------------------
*                                   |-----------------------------------|         |      MAC-DA      |    VLAN    | R-TAG (incr seqNum) |
*  --------------------------       |             seqNum=0,1.. ||--->|port-48|--> |00:00:00:00:11:11 | 0x8100 100 | 0xf1c1  0,1,2,3...  |
*  |      MAC-DA      VLAN  |-->|port-16|                      ||       |          ------------------------------------------------------
*  |00:00:00:00:11:11  100  |       | FDB   -> StreamID=743->vidx=1302  |
*  --------------------------       |lookup                    ||       |         ------------------------------------------------------
*                                   |             seqNum=0,1.. ||--->|port-51|-->|      MAC-DA      |    VLAN    | R-TAG (incr seqNum) |
*                                   |-----------------------------------|        |00:00:00:00:11:11 | 0x8100 100 | 0xf1c1  0,1,2,3...  |
*                                                                                 ------------------------------------------------------
*
* Flow-2: Send a tagged packet without an R-TAG that will be mapped to a stream-id by the IPCL (MAC-DA,VLAN), trigger the l2mll to duplicate the packet
*         for 2 different eport with different DA-MAC addresses.
* ------  Each packet will be transmitted with a generated increment sequence number.
*
*                                                                                                        ------------------------------------------------------
*                                   |----------------------------------------------------------|         |      MAC-DA      |    VLAN    | R-TAG (incr seqNum) |
*  --------------------------       |                         seqNum=0,1..  ||-->ePort 457->|port-48|--> |00:00:00:00:00:77 | 0x8100 101 | 0xf1c1  0,1,2,3...  |
*  |      MAC-DA      VLAN  |-->|port-16|                                   ||                 |          ------------------------------------------------------
*  |00:00:00:00:22:22  200  |       | IPCL ->StreamID=669->vidx=2847--> Mll[846]               |
*  --------------------------       | HIT                                   ||                 |        ------------------------------------------------------
*                                   |                         seqNum=0,1..  ||-->ePort 620->|port-51|-->|      MAC-DA      |    VLAN    | R-TAG (incr seqNum) |
*                                   |-----------------------------------------------------------        |00:00:00:00:00:78 | 0x8100 201 | 0xf1c1  0,1,2,3...  |
*                                                                                                        ------------------------------------------------------
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] cleanup_config        - If 0: Run example's configuration. If 1: Remove all previous configurations.
* @param[in] flow_1_configurations - Run configurations for flow-1
* @param[in] flow_2_configurations - Run configurations for flow-2
* @param[in] devNum                - device number
* @param[in] ingress_port          - Ingress port for the transmitted packet
* @param[in] egress_port_1         - Egress port-1 for the first duplicated R-TAG packet
* @param[in] egress_port_2         - Egress port-2 for the second duplicated R-TAG packet
* @param[in] vlan_1                - Incoming VLAN tag for flow-1 to be bridged
* @param[in] vlan_2                - Incoming VLAN tag for flow-2
* @param[in] vlan_3                - Outgoing VLAN tag for flow-1 for the first egress-port
* @param[in] vlan_4                - Outgoing VLAN tag for flow-2 for the first second-port
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - parameter value more then HW bit field
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*
* @note
*
* To run flow-1 configurations only:
*     shell-execute csRefTsn802_1cb_ProxyNode_main 0 1 0 0 16 48 51 100 200 101 201
* To remove flow-1 configuraton only:
*     shell-execute csRefTsn802_1cb_ProxyNode_main 1 1 0 0 16 48 51 100 200 101 201
*
* To run flow-2 configurations only:
*     shell-execute csRefTsn802_1cb_ProxyNode_main 0 0 1 0 16 48 51 100 200 101 201
* To remove flow-2 configuraton only:
*     shell-execute csRefTsn802_1cb_ProxyNode_main 1 0 1 0 16 48 51 100 200 101 201
*
* To run flow-1&flow-2 configurations:
*     shell-execute csRefTsn802_1cb_ProxyNode_main 0 1 1 0 16 48 51 100 200 101 201
* To remove flow-1&flow-2 configurations:
*     shell-execute csRefTsn802_1cb_ProxyNode_main 1 1 1 0 16 48 51 100 200 101 201
*
* To read counter for IPCL HIT: cpss-api call cpssDxChCncCounterGet devNum 0 blockNum 0 index 1 format CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_0_E
*/
GT_STATUS csRefTsn802_1cb_ProxyNode_main
(
  IN GT_BOOL cleanup_config,
  IN GT_BOOL flow_1_configurations,
  IN GT_BOOL flow_2_configurations,
  IN GT_U8  devNum,
  IN GT_U32 ingress_port,
  IN GT_U32 egress_port_1,
  IN GT_U32 egress_port_2,
  IN GT_U32 vlan_1,     /* Assigned as the eVLAN of flow-1 to bridged the packets */
  IN GT_U32 vlan_2,     /* Assigned as the eVLAN of flow-2 to bridged the packets */
  IN GT_U32 vlan_3,     /* Outgoing other VLAN of egress egress_port_1 */
  IN GT_U32 vlan_4      /* Outgoing other VLAN of egress egress_port_2 */
)
{
    GT_STATUS    rc                =  GT_OK;
    GT_U32       eport_1           =  457;  /* eport_1 maps to egress_port_1 */
    GT_U32       eport_2           =  620;  /* eport_2 maps to egress_port_2 */
    GT_U32       vTcamMngId        =  0;
    GT_U32       vTcamId           =  0;
    GT_U32       ruleId            =  0;
    GT_U32       stream_index_1    =  669;
    GT_U32       stream_index_2    =  743;
    GT_U32       vlan_3_up0        =  2;    /* vlan_3's UP value */
    GT_U32       vlan_4_up0        =  7;    /* vlan_4's UP value */
    GT_ETHERADDR macAddr_flow_1    =  {{0x00,0x00,0x00,0x00,0x11,0x11}};
    GT_ETHERADDR macAddr_flow_2    =  {{0x00,0x00,0x00,0x00,0x22,0x22}};
    GT_ETHERADDR arp_1_mac_addr    =  {{0x00,0x00,0x00,0x00,0x00,0x77}};
    GT_ETHERADDR arp_2_mac_addr    =  {{0x00,0x00,0x00,0x00,0x00,0x78}};
    GT_U16       vidx_1            =  1302;
    GT_U16       vidx_2            =  2847;
    GT_U32       maxVidxIndexthreshold =  2000;
    /* lttIndex is 846 because: vidx_2(2847) - 2000 (threshold) - 1 = 846 */
    GT_U32       lttIndex          =  vidx_2 - maxVidxIndexthreshold - 1;
    GT_U32       mllEntryIndex     =  20;
    GT_U16       tpid_tag_protocol =  0xF1C1;
    GT_U32       arp_entry_1       =  1;
    GT_U32       arp_entry_2       =  2;


    /* ---- If true: Remove flow's configurations only ---- */
    if (cleanup_config)
    {
      csRefTsn802_1cb_DeleteAllGlobalConfigurations(devNum);
      if (flow_1_configurations)
      {
        csRefTsn802_1cb_DeleteAllConfigurationsForFlow1(devNum, ingress_port,
              egress_port_1, egress_port_2,stream_index_1, vlan_1,
            vidx_1, macAddr_flow_1);
      }
      if (flow_2_configurations)
      {
        csRefTsn802_1cb_DeleteAllConfigurationsForFlow2(devNum, ingress_port, egress_port_1,
              egress_port_2, eport_1, eport_2,stream_index_2, vlan_2, vTcamMngId,
              vTcamId, ruleId, lttIndex, mllEntryIndex, arp_entry_1, arp_entry_2,
              arp_1_mac_addr, arp_2_mac_addr);
      }
      return rc;
    }
    /* Enable the SNG (Sequence Number Generation) */
    CPSS_CALL(cpssDxChStreamSngEnableSet(devNum, GT_TRUE));
    osPrintf("--- cpssDxChStreamSngEnableSet(): GT_TRUE\n");

    /* Enable the IRF (Individual Recovery Function) */
    CPSS_CALL(cpssDxChStreamIrfEnableSet(devNum, GT_TRUE));
    osPrintf("--- cpssDxChStreamIrfEnableSet(): GT_TRUE\n");

    /* Set the counting mode for streams */
    CPSS_CALL(cpssDxChStreamCountingModeSet(devNum,CPSS_DXCH_CNC_BYTE_COUNT_MODE_L2_E, GT_TRUE));
    osPrintf("--- cpssDxChStreamCountingModeSet(): CPSS_DXCH_CNC_BYTE_COUNT_MODE_L2_E\n");

    /* Flash all FDB entries */
    CPSS_CALL(cpssDxChBrgFdbStaticDelEnable(devNum, CPSS_DXCH_BRG_FDB_DEL_MODE_ALL_E));
    osPrintf("--- cpssDxChBrgFdbStaticDelEnable(): CPSS_DXCH_BRG_FDB_DEL_MODE_ALL_E\n");

    /* Global Configuration - If packet src-id descriptor arrives with/without
							  "Add Tag1" bit -> Add/Remove Tag1 (R-Tag) from packet */
    csRefTsn802_1cb_ConfigureTag1(devNum);

    if (flow_1_configurations)
    {
      /* TPID configurations, use egress ports */
      csRefTsn802_1cb_ConfigureTPID(devNum, tpid_tag_protocol, ingress_port,
                                    egress_port_1, egress_port_2);

      /* Confiure the stream entry with sequence generation without individual recovery function */
      csRefTsn802_1cb_ConfigureStream(devNum, stream_index_1, GT_TRUE, GT_FALSE);
      csRefTsn802_1cb_EnableStreamAssignmentFromFdb(devNum);

      /* ---- Add all ports to VLAN A: ---- */
      csRefTsn802_1cb_CreateVlan(devNum, vlan_1);
      csRefTsn802_1cb_AddPortMemberToVlan(devNum, vlan_1, ingress_port);
      csRefTsn802_1cb_AddPortMemberToVlan(devNum, vlan_1, egress_port_1);
      csRefTsn802_1cb_AddPortMemberToVlan(devNum, vlan_1, egress_port_2);

      csRefTsn802_1cb_AddPortToMc(devNum, vidx_1, egress_port_1);
      csRefTsn802_1cb_AddPortToMc(devNum, vidx_1, egress_port_2);

      /* The relevant packet will be forwarded to vidx_1 and assosiate with stream_1 */
      csRefTsn802_1cb_CreateStaticMacEntry(devNum, vlan_1, macAddr_flow_1, vidx_1, stream_index_1);
    }
    if (flow_2_configurations)
    {
      /* ---- TPID configurations, use egress ePorts: ---- */
      csRefTsn802_1cb_ConfigureTPID(devNum, tpid_tag_protocol, ingress_port, eport_1, eport_2);

      /* Confiure the stream entry with sequence generation without individual recovery function */
      csRefTsn802_1cb_ConfigureStream(devNum, stream_index_2, GT_TRUE, GT_FALSE);

      /* ---- Add all ports to VLAN B: ---- */
      csRefTsn802_1cb_CreateVlan(devNum, vlan_2);
      csRefTsn802_1cb_AddPortMemberToVlan(devNum, vlan_2, ingress_port);
      csRefTsn802_1cb_AddPortMemberToVlan(devNum, vlan_2, egress_port_1);
      csRefTsn802_1cb_AddPortMemberToVlan(devNum, vlan_2, egress_port_2);

      /* ---- IPCL Rule to catch packets with: ---- */
      csRefTsn802_1cb_ConfigureVTcam(devNum, vTcamMngId, vTcamId);
      /* Value needs to be grater than stream_index_2 so the assigned StreamID!=0 */
      csRefTsn802_1cb_ConfigureIpcl(devNum,
                        CPSS_DXCH_PCL_FLOW_ID_ACTION_ASSIGNMENT_MODE_FLOW_ID_STREAM_ID_E,
                        ingress_port);
      CPSS_CALL(cpssDxChPclNumberOfStreamIdsSet(devNum, CPSS_PCL_DIRECTION_INGRESS_E, 1000));
      csRefTsn802_1cb_VTcamAddRule(vTcamMngId, vTcamId, ruleId, vidx_2, stream_index_2,
                        macAddr_flow_2, vlan_2);
      csRefTsn802_1cb_ConfigureCounterToRule(devNum);

      csRefTsn802_1cb_ConfigureEgressPort(devNum, eport_1);
      csRefTsn802_1cb_ConfigureEgressPort(devNum, eport_2);

      /* Configure the threshold to invoke MLL usage */
      CPSS_CALL(cpssDxChL2MllLookupMaxVidxIndexSet(devNum, maxVidxIndexthreshold));
      osPrintf("--- cpssDxChL2MllLookupMaxVidxIndexSet(): Configuring threshold for %d\n",
              maxVidxIndexthreshold);

      /* Create L2 LTT entry */
      csRefTsn802_1cb_ConfigureMllPairs(devNum, lttIndex, mllEntryIndex, ingress_port,
                                        eport_1, eport_2);

      /* Map the eport to the relevant egress port */
      csRefTsn802_1cb_MapEportToPhysicalPorts(devNum, eport_1, ingress_port, egress_port_1);
      csRefTsn802_1cb_MapEportToPhysicalPorts(devNum, eport_2, ingress_port, egress_port_2);

      /* Create an egress object (ARP entry) for eport_1 */
      csRefTsn802_1cb_CreateArpEntryWithMac(devNum, eport_1, arp_entry_1, arp_1_mac_addr);

      /* Create an egress object (ARP entry) for eport_2 */
      csRefTsn802_1cb_CreateArpEntryWithMac(devNum, eport_2, arp_entry_2, arp_2_mac_addr);

      /* Configuration to VLAN manipulate the outer vlan VID value and UP */
      csRefTsn802_1cb_ConfigureEgressPortVlan(devNum, eport_1, vlan_3, vlan_3_up0);
      csRefTsn802_1cb_ConfigureEgressPortVlan(devNum, eport_2, vlan_4, vlan_4_up0);
    }

    return rc;
}

/**
* @internal csRefTsn802_1cb_splitterNode_main function
* @endinternal
*
*
* Flow-1: Send a tagged packet without an R-TAG that will be mapped to a stream-id by the IPCL (MAC-DA,VLAN),
*         trigger the l2mll to duplicate the packet for 2 different eport with different DA-MAC address.
* ------  Each packet will be transmitted with a generated increment sequence number.
*
*                                                                                                                     ------------------------------------------------------
*                                                |----------------------------------------------------------|         |      MAC-DA      |    VLAN    | R-TAG (incr seqNum) |
*  ----------------------------------------      |                        seqNum=0,1.. ||-->ePort 457->|port-48|-->   |00:00:00:00:00:77 | 0x8100 101 | 0xf1c1  0,1,2,3...  |
*  |      MAC-DA     | VLAN |R-TAG (seqNum)|-->|port-16|                               ||                   |          ------------------------------------------------------
*  |00:00:00:00:22:22|  200 |0x8100  120   |     |IPCL ->StreamID=650->vidx=2847--> Mll[846]                |
*  ----------------------------------------      | HIT                                 ||                   |      ------------------------------------------------------
*                                                |                        seqNum=0,1.. ||-->ePort 620->|port-51|-->|      MAC-DA      |    VLAN    | R-TAG (incr seqNum) |
*                                                |-----------------------------------------------------------      |00:00:00:00:00:78 | 0x8100 201 | 0xf1c1  0,1,2,3...  |
*                                                                                                                  ------------------------------------------------------
*
*
* @brief  This function configure a TSN 802.1cb splitter-Node functionally with one flow:
* Flow-1: Packet will be mapped to a stream-id by the IPCL (MAC-DA,VLAN) and the stream-id will be map to a VIDX with 2 egress-port
* @note   APPLICABLE DEVICES:      Ironman
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] cleanup_config        - If 0: Run example's configuration. If 1: Remove all previous configurations.
* @param[in] devNum                - device number
* @param[in] ingress_port          - Ingress port for the transmitted packet
* @param[in] egress_port_1         - Egress port-1 for the first duplicated R-TAG packet
* @param[in] egress_port_2         - Egress port-2 for the second duplicated R-TAG packet
* @param[in] in_vlan               - Incoming VLAN tag for flow-1 to be bridged
* @param[in] out_vlan_1            - Outgoing VLAN tag for flow-1 for the first egress-port
* @param[in] out_vlan_2            - Outgoing VLAN tag for flow-2 for the first second-port
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_OUT_OF_RANGE          - parameter value more then HW bit field
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*
* @note
*
* To run flow-1 configurations only:
*     shell-execute csRefTsn802_1cb_splitterNode_main 0 0 16 48 51 200 101 201
* To remove flow-1 configuraton only:
*     shell-execute csRefTsn802_1cb_splitterNode_main 1 0 16 48 51 200 101 201
*
* To read counter for IPCL HIT: cpss-api call cpssDxChCncCounterGet devNum 0 blockNum 0 index 1 format CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_0_E
*/
GT_STATUS csRefTsn802_1cb_splitterNode_main
(
  IN GT_BOOL cleanup_config,
  IN GT_U8   devNum,
  IN GT_U32  ingress_port,
  IN GT_U32  egress_port_1,
  IN GT_U32  egress_port_2,
  IN GT_U32  in_vlan,      /* VLAN to be assign as the eVLAN of flow-2 to bridged the packets */
  IN GT_U32  out_vlan_1,   /* Outgoing other VLAN of egress egress_port_1 */
  IN GT_U32 out_vlan_2    /* Outgoing other VLAN of egress egress_port_2 */
)
{
    GT_STATUS    rc                =  GT_OK;
    GT_U32       eport_1           =  457;   /* eport_1 maps to egress_port_1 */
    GT_U32       eport_2           =  620;   /* eport_2 maps to egress_port_2 */
    GT_U32       vTcamMngId        =  0;
    GT_U32       vTcamId           =  0;
    GT_U32       ruleId            =  0;
    GT_U32       stream_index      =  650;
    GT_U32       out_vlan_1_up0    =  2;    /* out_vlan_1's UP value */
    GT_U32       out_vlan_2_up0    =  7;    /* out_vlan_2's UP value */
    GT_ETHERADDR mac_da_addr       =  {{0x00,0x00,0x00,0x00,0x22,0x22}};
    GT_ETHERADDR arp_1_mac_addr    =  {{0x00,0x00,0x00,0x00,0x00,0x77}};
    GT_ETHERADDR arp_2_mac_addr    =  {{0x00,0x00,0x00,0x00,0x00,0x78}};
    GT_U16       vidx              =  2847;
    GT_U32       maxVidxIndexthreshold =  2000;
    /* lttIndex is 846 because: vidx(2847) - 2000 (threshold) - 1 = 846 */
    GT_U32       lttIndex          =  vidx - maxVidxIndexthreshold - 1;
    GT_U32       mllEntryIndex     =  20;
    GT_U16       tpid_tag_protocol =  0xF1C1;
    GT_U32       arp_entry_1       =  1;
    GT_U32       arp_entry_2       =  2;


    /* ---- If true: Remove flow's configurations only ---- */
    if (cleanup_config)
    {
      csRefTsn802_1cb_DeleteAllGlobalConfigurations(devNum);
      csRefTsn802_1cb_DeleteAllConfigurationsForFlow2(devNum, ingress_port, egress_port_1,
                                                      egress_port_2, eport_1, eport_2,
                                                      stream_index, in_vlan, vTcamMngId,
                                                      vTcamId, ruleId, lttIndex, mllEntryIndex,
                                                      arp_entry_1, arp_entry_2, arp_1_mac_addr,
                                                      arp_2_mac_addr);
      return rc;
    }

    /* Enable the SNG (Sequence Number Generation) */
    CPSS_CALL(cpssDxChStreamSngEnableSet(devNum, GT_TRUE));
    osPrintf("--- cpssDxChStreamSngEnableSet(): GT_TRUE\n");

    /* Enable the IRF (Individual Recovery Function) */
    CPSS_CALL(cpssDxChStreamIrfEnableSet(devNum, GT_TRUE));
    osPrintf("--- cpssDxChStreamIrfEnableSet(): GT_TRUE\n");

    /* Set the counting mode for streams */
    CPSS_CALL(cpssDxChStreamCountingModeSet(devNum, CPSS_DXCH_CNC_BYTE_COUNT_MODE_L2_E, GT_TRUE));
    osPrintf("--- cpssDxChStreamCountingModeSet(): CPSS_DXCH_CNC_BYTE_COUNT_MODE_L2_E\n");

    /* Flash all FDB entries */
    CPSS_CALL(cpssDxChBrgFdbStaticDelEnable(devNum, CPSS_DXCH_BRG_FDB_DEL_MODE_ALL_E));
    osPrintf("--- cpssDxChBrgFdbStaticDelEnable(): CPSS_DXCH_BRG_FDB_DEL_MODE_ALL_E\n");

    /* TPID configurations, use egress ePorts */
    csRefTsn802_1cb_ConfigureTPID(devNum, tpid_tag_protocol, ingress_port, eport_1, eport_2);

    /* Confiure the stream entry with no sequence generation with individual recovery function */
    csRefTsn802_1cb_ConfigureStream(0, stream_index, GT_FALSE, GT_TRUE);

    /* ---- Add all ports to VLAN B ----: */
    csRefTsn802_1cb_AddPortMemberToVlan(devNum, in_vlan, ingress_port);
    csRefTsn802_1cb_AddPortMemberToVlan(devNum, in_vlan, egress_port_1);
    csRefTsn802_1cb_AddPortMemberToVlan(devNum, in_vlan, egress_port_2);

    /* ---- IPCL Rule to catch packets with: ---- */
    csRefTsn802_1cb_ConfigureVTcam(devNum, vTcamMngId, vTcamId);
    csRefTsn802_1cb_ConfigureIpcl(devNum,
                      CPSS_DXCH_PCL_FLOW_ID_ACTION_ASSIGNMENT_MODE_STREAM_ID_ONLY_E, ingress_port);
    csRefTsn802_1cb_VTcamAddRule(vTcamMngId, vTcamId, ruleId, vidx, stream_index,
                                mac_da_addr, in_vlan);
    csRefTsn802_1cb_ConfigureCounterToRule(devNum);

    /* Configure the threshold to invoke MLL usage */
    CPSS_CALL(cpssDxChL2MllLookupMaxVidxIndexSet(devNum, maxVidxIndexthreshold));
    osPrintf("--- cpssDxChL2MllLookupMaxVidxIndexSet(): Configuring threshold for %d\n",
    				maxVidxIndexthreshold);

    csRefTsn802_1cb_ConfigureEgressPort(devNum, eport_1);
    csRefTsn802_1cb_ConfigureEgressPort(devNum, eport_2);

    /* Create L2 LTT entry */
    csRefTsn802_1cb_ConfigureMllPairs(devNum, lttIndex, mllEntryIndex, ingress_port,
                                      eport_1, eport_2);

    /* Map the eport to the relevant egress port */
    csRefTsn802_1cb_MapEportToPhysicalPorts(devNum, eport_1, ingress_port, egress_port_1);
    csRefTsn802_1cb_MapEportToPhysicalPorts(devNum, eport_2, ingress_port, egress_port_2);

    /* Create an egress object (ARP entry) for eport_1 */
    csRefTsn802_1cb_CreateArpEntryWithMac(devNum, eport_1, arp_entry_1, arp_1_mac_addr);

    /* Create an egress object (ARP entry) for eport_2 */
    csRefTsn802_1cb_CreateArpEntryWithMac(devNum, eport_2, arp_entry_2, arp_2_mac_addr);

    /* Configuration to VLAN manipulate the outer vlan VID value and UP */
    csRefTsn802_1cb_ConfigureEgressPortVlan(devNum, eport_1, out_vlan_1, out_vlan_1_up0);
    csRefTsn802_1cb_ConfigureEgressPortVlan(devNum, eport_2, out_vlan_2, out_vlan_2_up0);

    /* Global Configuration - If packet src-id descriptor arrives with/without
        "Add Tag1" bit -> Add/Remove Tag1 (R-Tag) from packet */
    csRefTsn802_1cb_ConfigureTag1(devNum);

    return rc;
}



/*************************************************************************************************/
/******************************************* LOCAL FUNCTIONS *************************************/
/*************************************************************************************************/

/**
* @internal csRefTsn802_1cb_ConfigureTPID function
* @endinternal
*
* @brief   This function configures globally the TPID and per port (ingress/egress) to support the 802.1cb tag
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum            - device number.
* @param[in] tpid_tag_protocol - the TPID protocol for TSN 802.1cb (0xf1c1).
* @param[in] ingress_port      - the ingress port that needs to be adapted to receives 802.1cb packets.
* @param[in] egress_port_1     - first egress port.
* @param[in] egress_port_2     - second egress port.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*
*/
GT_STATUS csRefTsn802_1cb_ConfigureTPID
(
  IN GT_U8  devNum,
  IN GT_U16 tpid_tag_protocol,
  IN GT_U32 ingress_port,
  IN GT_U32 egress_port_1,
  IN GT_U32 egress_port_2
)
{
    GT_STATUS    rc                =  GT_OK;
    GT_U32       tpid_index        =  1;
    GT_U32       tpid_profile      =  0;
    /* holds the 6B tag parse related configuration*/
    CPSS_DXCH_BRG_VLAN_8B_TAG_PARSE_CONFIG_STC tag_struct;

    /* Set TPID tag 0xF1C1 for ingress table */
    CPSS_CALL(cpssDxChBrgVlanTpidEntrySet(devNum, CPSS_DXCH_ETHERTYPE_TABLE_INGRESS_E, tpid_index,
                                          tpid_tag_protocol));
    osPrintf("--- cpssDxChBrgVlanTpidEntrySet(): Configure TPID Tag:\n");
    osPrintf("    Ingress profile index: %d\n", tpid_index);
    osPrintf("    TPID value: 0x%x\n", tpid_tag_protocol);

    CPSS_CALL(cpssDxChBrgVlanTpidEntrySet(devNum, CPSS_DXCH_ETHERTYPE_TABLE_EGRESS_E, tpid_index,
                                    tpid_tag_protocol)); /* Set TPID tag 0xF1C1 for egress table */
    osPrintf("--- cpssDxChBrgVlanTpidEntrySet(): Configure TPID Tag:\n");
    osPrintf("    Egress profile index: %d\n", tpid_index);
    osPrintf("    TPID value: 0x%x\n", tpid_tag_protocol);

    /* ---- 802.1cb support 6B header: ---- */
    CPSS_CALL(cpssDxChBrgVlanTpidTagTypeSet(devNum, CPSS_DIRECTION_INGRESS_E, tpid_index,
                                            CPSS_BRG_TPID_SIZE_TYPE_6_BYTES_E));
    CPSS_CALL(cpssDxChBrgVlanTpidTagTypeSet(devNum, CPSS_DIRECTION_EGRESS_E, tpid_index,
                                            CPSS_BRG_TPID_SIZE_TYPE_6_BYTES_E));
    osPrintf("--- cpssDxChBrgVlanTpidTagTypeSet(): Set Ingress & Egress TPID tag to 6 Bytes\n");

    /* Enable 8-Tag parse and set offset to 2 to indicate from where to copy the Tag1(R-Tag)
     * VID1/UP1/CFI1 value. The offset is in Bytes and relative to the start of the TCI,
     * which is after the TPID value */
    osMemSet(&tag_struct, 0, sizeof(CPSS_DXCH_BRG_VLAN_8B_TAG_PARSE_CONFIG_STC));
    tag_struct.enable = GT_TRUE;
    tag_struct.extractedValueOffset = 2;
    CPSS_CALL(cpssDxChBrgVlanPort8BTagInfoSet(devNum, CPSS_DIRECTION_INGRESS_E, tpid_index,
                                              &tag_struct));

    CPSS_CALL(cpssDxChBrgVlanIngressTpidProfileSet(devNum, tpid_profile, CPSS_VLAN_ETHERTYPE0_E,
                                                  0x00000001));
    CPSS_CALL(cpssDxChBrgVlanIngressTpidProfileSet(devNum, tpid_profile, CPSS_VLAN_ETHERTYPE1_E,
                                                  0x00000002));

    CPSS_CALL(cpssDxChBrgVlanPortIngressTpidProfileSet(devNum, ingress_port,
                                                      CPSS_VLAN_ETHERTYPE0_E, GT_FALSE,
                                                      tpid_profile));
    CPSS_CALL(cpssDxChBrgVlanPortIngressTpidProfileSet(devNum, ingress_port,
                                                      CPSS_VLAN_ETHERTYPE1_E,GT_FALSE,
                                                      tpid_profile));

    CPSS_CALL(cpssDxChBrgVlanPortEgressTpidSet(devNum, egress_port_1,
                          CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ETHERTYPE0_E, 0));
    CPSS_CALL(cpssDxChBrgVlanPortEgressTpidSet(devNum, egress_port_2,
                          CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ETHERTYPE0_E, 0));
    CPSS_CALL(cpssDxChBrgVlanPortEgressTpidSet(devNum, egress_port_1,
                          CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ETHERTYPE1_E, 1));
    CPSS_CALL(cpssDxChBrgVlanPortEgressTpidSet(devNum, egress_port_2,
                          CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ETHERTYPE1_E, 1));

    return rc;
}


/**
* @internal csRefTsn802_1cb_ConfigureVTcam function
* @endinternal
*
* @brief   This function creates the VTCAM
*
* @note   APPLICABLE DEVICES:      All.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum            - device number.
* @param[in] tpid_tag_protocol - the TPID protocol for TSN 802.1cb (0xf1c1).
* @param[in] ingress_port      - ingress port that needs to be adapted to receive 802.1cb packets.
* @param[in] egress_port_1     - first egress port.
* @param[in] egress_port_2     - second egress port.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*
*/
GT_STATUS csRefTsn802_1cb_ConfigureVTcam
(
  IN GT_U8   devNum,
  IN GT_U32  vTcamMngId,
  IN GT_U32  vTcamId
)
{
    GT_STATUS    rc             =  GT_OK;
    GT_U32       tcamGroup      =  0;
    GT_U8        devListArr[]   =  {0};
    CPSS_DXCH_VIRTUAL_TCAM_MNG_CONFIG_PARAM_STC vTcamMngCfg;
    CPSS_DXCH_VIRTUAL_TCAM_INFO_STC             vTcamInfo;

    osPrintf("--- cpssDxChTcamPortGroupClientGroupSet(): %d\n", tcamGroup);
    CPSS_CALL(cpssDxChTcamPortGroupClientGroupSet(devNum, 0xffffffff, CPSS_DXCH_TCAM_IPCL_0_E,
                                                  tcamGroup, GT_TRUE));

    osPrintf("--- cpssDxChVirtualTcamManagerCreate(): %d\n", vTcamMngId);
    osMemSet(&vTcamMngCfg, 0, sizeof(CPSS_DXCH_VIRTUAL_TCAM_MNG_CONFIG_PARAM_STC));
    vTcamMngCfg.haSupportEnabled = GT_TRUE;
    vTcamMngCfg.haFeaturesEnabledBmp = 0;
    CPSS_CALL(cpssDxChVirtualTcamManagerCreate(vTcamMngId, &vTcamMngCfg));

    osPrintf("--- cpssDxChVirtualTcamManagerDevListAdd():\n");
    CPSS_CALL(cpssDxChVirtualTcamManagerDevListAdd(vTcamMngId, devListArr, 1 /*num of devices*/));

    osMemSet(&vTcamInfo, 0, sizeof(CPSS_DXCH_VIRTUAL_TCAM_INFO_STC));
    vTcamInfo.clientGroup = 0;
    vTcamInfo.hitNumber   = 0;
    vTcamInfo.ruleSize    = CPSS_DXCH_VIRTUAL_TCAM_RULE_SIZE_20_B_E;
    vTcamInfo.autoResize  = GT_FALSE;
    vTcamInfo.guaranteedNumOfRules = 6;
    vTcamInfo.tcamSpaceUnmovable   = CPSS_DXCH_VIRTUAL_TCAM_MOVABLE_E;
    vTcamInfo.ruleAdditionMethod   = CPSS_DXCH_VIRTUAL_TCAM_RULE_ADDITION_METHOD_LOGICAL_INDEX_E;
    vTcamInfo.allocationSpaceArrSize = 0;
    CPSS_CALL(cpssDxChVirtualTcamCreate(vTcamMngId, vTcamId, &vTcamInfo));

    osPrintf("--- cpssDxChVirtualTcamCreate():\n");
    osPrintf("    vTcamMngId: %d\n", vTcamMngId);
    osPrintf("    vTcamId:    %d\n", vTcamId);

    return rc;

}


/**
* @internal csRefTsn802_1cb_ConfigureIpcl function
* @endinternal
*
* @brief   This function sets the entire IPCL configuration needed to classified a stream-ID.
*
* @note   APPLICABLE DEVICES:      All.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum          - device number.
* @param[in] assignment_mode - assignment mode for IPCL/EPCL action: Stream-id only, flow-id only, both stream-id and flow-id.
* @param[in] ingress_port    - the ingress port to be configured to classify packets to stream-id.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*
*/
GT_STATUS csRefTsn802_1cb_ConfigureIpcl
(
  IN GT_U8 devNum,
  IN CPSS_DXCH_PCL_FLOW_ID_ACTION_ASSIGNMENT_MODE_ENT assignment_mode,
  IN GT_U32 ingress_port
)
{
    GT_STATUS    rc             =  GT_OK;

    /* ---- Configure IPCL flowID assignment: ---- */
    CPSS_CALL(cpssDxChPclFlowIdActionAssignModeSet(devNum, CPSS_PCL_DIRECTION_INGRESS_E,
                                                  CPSS_PCL_LOOKUP_0_0_E,
                                                  0/*hit num*/, assignment_mode));
    CPSS_CALL(cpssDxChPclInit(devNum));
    CPSS_CALL(cpssDxChPclIngressPolicyEnable(devNum, GT_TRUE));
    CPSS_CALL(csRefTsn802_1cb_ConfigureIpclPerPort(devNum, ingress_port));

    return rc;
}


/**
* @internal csRefTsn802_1cb_ConfigureIpclPerPort function
* @endinternal
*
* @brief   This function sets the IPCL configuration for the ingress port.
*
* @note   APPLICABLE DEVICES:      All.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum         - device number.
* @param[in] ingress_port   - the ingress port to be configured to classify packets to stream-id.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*
*/
GT_STATUS csRefTsn802_1cb_ConfigureIpclPerPort
(
  IN GT_U8  devNum,
  IN GT_U32 ingress_port
)
{
    GT_STATUS                     rc                  =  GT_OK;
    CPSS_INTERFACE_INFO_STC       interfaceInfo;
    CPSS_DXCH_PCL_LOOKUP_CFG_STC  lookupCfg;
    CPSS_DXCH_PCL_UDB_SELECT_STC  udbSelectStruct;

    CPSS_CALL(cpssDxChPclPortIngressPolicyEnable(devNum, ingress_port, GT_TRUE));
    CPSS_CALL(cpssDxChPclPortLookupCfgTabAccessModeSet(devNum, ingress_port,
                    CPSS_PCL_DIRECTION_INGRESS_E,CPSS_PCL_LOOKUP_0_0_E,
    0 ,CPSS_DXCH_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E));

    osMemSet(&interfaceInfo, 0, sizeof(CPSS_INTERFACE_INFO_STC));
    osMemSet(&lookupCfg, 0, sizeof(CPSS_DXCH_PCL_LOOKUP_CFG_STC));

    /* Configure interfaceInfo */
    interfaceInfo.type             = CPSS_INTERFACE_PORT_E;
    interfaceInfo.devPort.hwDevNum = devNum;
    interfaceInfo.devPort.portNum  = ingress_port;

    /* Configure lookupCfg */
    lookupCfg.enableLookup  = GT_TRUE;
    lookupCfg.pclId         = 0;
    lookupCfg.dualLookup    = GT_FALSE;
    lookupCfg.pclIdL01      = 0;
    lookupCfg.groupKeyTypes.nonIpKey = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_20_E;
    lookupCfg.groupKeyTypes.ipv4Key  = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_20_E;
    lookupCfg.groupKeyTypes.ipv6Key  = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_20_E;
    lookupCfg.udbKeyBitmapEnable = GT_FALSE;
    lookupCfg.tcamSegmentMode    = CPSS_DXCH_PCL_TCAM_SEGMENT_MODE_4_TCAMS_E;

    CPSS_CALL(cpssDxChPclCfgTblSet(devNum, &interfaceInfo, CPSS_PCL_DIRECTION_INGRESS_E,
                                  CPSS_PCL_LOOKUP_0_0_E, &lookupCfg));

    osMemSet(&udbSelectStruct, 0, sizeof(CPSS_DXCH_PCL_UDB_SELECT_STC));
    udbSelectStruct.udbSelectArr[0] = 0;
    udbSelectStruct.udbSelectArr[1] = 0;
    udbSelectStruct.udbSelectArr[2] = 14;
    udbSelectStruct.udbSelectArr[3] = 15;
    udbSelectStruct.udbSelectArr[4] = 16;
    udbSelectStruct.udbSelectArr[5] = 17;
    udbSelectStruct.udbSelectArr[6] = 18;
    udbSelectStruct.udbSelectArr[7] = 19;
    udbSelectStruct.udbSelectArr[8] = 20;
    udbSelectStruct.udbSelectArr[9] = 21;
    udbSelectStruct.udbSelectArr[10] = 22;
    udbSelectStruct.udbSelectArr[11] = 23;

    CPSS_CALL(cpssDxChPclUserDefinedBytesSelectSet(devNum,
                                                  CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_20_E,
                                                  CPSS_DXCH_PCL_PACKET_TYPE_ETHERNET_OTHER_E,
                                                  CPSS_PCL_LOOKUP_0_0_E, &udbSelectStruct));

    /* Configure the D-MAC UDBs: */
    CPSS_CALL(cpssDxChPclUserDefinedByteSet(devNum,
                                            CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_20_E,
                                            CPSS_DXCH_PCL_PACKET_TYPE_ETHERNET_OTHER_E,
                                            CPSS_PCL_DIRECTION_INGRESS_E, 14,
                                            CPSS_DXCH_PCL_OFFSET_L2_E, 0));

    CPSS_CALL(cpssDxChPclUserDefinedByteSet(devNum,
                                            CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_20_E,
                                            CPSS_DXCH_PCL_PACKET_TYPE_ETHERNET_OTHER_E,
                                            CPSS_PCL_DIRECTION_INGRESS_E, 15,
                                            CPSS_DXCH_PCL_OFFSET_L2_E, 1));

    CPSS_CALL(cpssDxChPclUserDefinedByteSet(devNum,
                                            CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_20_E,
                                            CPSS_DXCH_PCL_PACKET_TYPE_ETHERNET_OTHER_E,
                                            CPSS_PCL_DIRECTION_INGRESS_E, 16,
                                            CPSS_DXCH_PCL_OFFSET_L2_E, 2));

    CPSS_CALL(cpssDxChPclUserDefinedByteSet(devNum,
                                            CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_20_E,
                                            CPSS_DXCH_PCL_PACKET_TYPE_ETHERNET_OTHER_E,
                                            CPSS_PCL_DIRECTION_INGRESS_E, 17,
                                            CPSS_DXCH_PCL_OFFSET_L2_E, 3));

    CPSS_CALL(cpssDxChPclUserDefinedByteSet(devNum,
                                            CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_20_E,
                                            CPSS_DXCH_PCL_PACKET_TYPE_ETHERNET_OTHER_E,
                                            CPSS_PCL_DIRECTION_INGRESS_E, 18,
                                            CPSS_DXCH_PCL_OFFSET_L2_E, 4));

    CPSS_CALL(cpssDxChPclUserDefinedByteSet(devNum,
                                            CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_20_E,
                                            CPSS_DXCH_PCL_PACKET_TYPE_ETHERNET_OTHER_E,
                                            CPSS_PCL_DIRECTION_INGRESS_E, 19,
                                            CPSS_DXCH_PCL_OFFSET_L2_E, 5));
    /* Configure the VLAN Tag UDBs: */
    CPSS_CALL(cpssDxChPclUserDefinedByteSet(devNum,
                                            CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_20_E,
                                            CPSS_DXCH_PCL_PACKET_TYPE_ETHERNET_OTHER_E,
                                            CPSS_PCL_DIRECTION_INGRESS_E, 20,
                                            CPSS_DXCH_PCL_OFFSET_L2_E, 14));

    CPSS_CALL(cpssDxChPclUserDefinedByteSet(devNum,
                                            CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_20_E,
                                            CPSS_DXCH_PCL_PACKET_TYPE_ETHERNET_OTHER_E,
                                            CPSS_PCL_DIRECTION_INGRESS_E, 21,
                                            CPSS_DXCH_PCL_OFFSET_L2_E, 15));

    return rc;
}


/**
* @internal csRefTsn802_1cb_SetRuleIngrUdbOnly function
* @endinternal
*
* @brief   This function sets the IPCL rule to catch packets with a specific MAC address and VLAN-id and classify them to a stream-id and redirect the packet to a vidx.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[out] mask        - a pointer to the rule's mask to be filled accordingly.
* @param[out] pattern     - a pointer to the rule's pattern to be filled accordingly with the user's params.
* @param[out] pclAction   - a pointer to the rule's action to be filled accordingly with the user's params.
* @param[in] macAddr      - the dmac that is used as a classifier  for the stream-id.
* @param[in] vlanId       - the vlan-id that is used as a classifier  for the stream-id.
* @param[in] vidx         - this vidx values is used as the redirect interface of a matched packet.
* @param[in] stream_index - the stream-id value that is used to classify by the dmac and vlan-id.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*
*/
GT_STATUS csRefTsn802_1cb_SetRuleIngrUdbOnly
(
  OUT CPSS_DXCH_PCL_RULE_FORMAT_UNT *mask,
  OUT CPSS_DXCH_PCL_RULE_FORMAT_UNT *pattern,
  OUT CPSS_DXCH_PCL_ACTION_STC *pclAction,
  IN GT_ETHERADDR macAddr,
  IN GT_U16 vlanId,
  IN GT_U16  vidx,
  IN GT_U16  stream_index
)
{

    GT_STATUS  rc      =  GT_OK;

    /* Set Mask */
    mask->ruleIngrUdbOnly.udb[2]  = 0xFF;  /* destination MAC */
    mask->ruleIngrUdbOnly.udb[3]  = 0xFF;
    mask->ruleIngrUdbOnly.udb[4]  = 0xFF;
    mask->ruleIngrUdbOnly.udb[5]  = 0xFF;
    mask->ruleIngrUdbOnly.udb[6]  = 0xFF;
    mask->ruleIngrUdbOnly.udb[7]  = 0xFF;
    mask->ruleIngrUdbOnly.udb[8]  = 0x00;  /* Source MAC */
    mask->ruleIngrUdbOnly.udb[9]  = 0x00;
    mask->ruleIngrUdbOnly.udb[10] = 0x00;
    mask->ruleIngrUdbOnly.udb[11] = 0x00;
    mask->ruleIngrUdbOnly.udb[12] = 0x00;
    mask->ruleIngrUdbOnly.udb[13] = 0x00;
    mask->ruleIngrUdbOnly.udb[14] = 0x00;
    mask->ruleIngrUdbOnly.udb[15] = 0x00; /* VLAN ID */
    mask->ruleIngrUdbOnly.udb[16] = 0x00;  /* Source port */
    mask->ruleIngrUdbOnly.udb[17] = 0x00;  /* Ethertype */
    mask->ruleIngrUdbOnly.udb[18] = 0x00;

    /* Set Pattern */
    pattern->ruleIngrUdbOnly.udb[2]  = macAddr.arEther[0];
    pattern->ruleIngrUdbOnly.udb[3]  = macAddr.arEther[1]; /* destination MAC */
    pattern->ruleIngrUdbOnly.udb[4]  = macAddr.arEther[2];
    pattern->ruleIngrUdbOnly.udb[5]  = macAddr.arEther[3];
    pattern->ruleIngrUdbOnly.udb[6]  = macAddr.arEther[4];
    pattern->ruleIngrUdbOnly.udb[7]  = macAddr.arEther[5];
    pattern->ruleIngrUdbOnly.udb[8]  = 0x00; /* Source MAC */
    pattern->ruleIngrUdbOnly.udb[9]  = 0x00;
    pattern->ruleIngrUdbOnly.udb[10] = 0x00;
    pattern->ruleIngrUdbOnly.udb[11] = 0x00;
    pattern->ruleIngrUdbOnly.udb[12] = 0x00;
    pattern->ruleIngrUdbOnly.udb[13] = 0x00; /* Source MAC */
    pattern->ruleIngrUdbOnly.udb[14] = (GT_U8)((vlanId >> 8)& 0x0F);
    pattern->ruleIngrUdbOnly.udb[15] = (GT_U8)(vlanId);
    pattern->ruleIngrUdbOnly.udb[16] = 0x00; /* Source port*/
    pattern->ruleIngrUdbOnly.udb[17] = 0x00; /* Ethertype */
    pattern->ruleIngrUdbOnly.udb[18] = 0x00;

    /* Set Action */
    pclAction->pktCmd = CPSS_PACKET_CMD_FORWARD_E;
    pclAction->bypassIngressPipe       = GT_TRUE;
    pclAction->redirect.redirectCmd    = CPSS_DXCH_PCL_ACTION_REDIRECT_CMD_OUT_IF_E;
    pclAction->redirect.data.outIf.outInterface.type = CPSS_INTERFACE_VIDX_E;
    pclAction->redirect.data.outIf.outInterface.vidx = vidx;
    pclAction->redirect.data.outIf.modifyMacDa = GT_TRUE;
    pclAction->redirect.data.outIf.arpPtr      = 1;
    pclAction->flowId = stream_index;
    pclAction->matchCounter.matchCounterIndex = 0;
    pclAction->matchCounter.enableMatchCount = GT_TRUE;

    return rc;
}


/**
* @internal csRefTsn802_1cb_VTcamAddRule function
* @endinternal
*
* @brief   This function adds a vTcam rule to classify a packet by dmac and vlan-id to a stream-id and redirect the packet to a vidx.
*
* @note   APPLICABLE DEVICES:      All.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] vTcamMngId  - the vTcam manager ID.
* @param[in] vTcamId   - the vTcam ID.
* @param[in] ruleId - the rule index.
* @param[in] vidx   - vidx value that is used as the action's redirect operation.
* @param[in] stream_index   - the stream-id value to be classify.
* @param[in] macAddr   - dmac address that is used to classify a packet to a stream-id.
* @param[in] in_vlan   - vlan-id that is used to classify a packet to a stream-id.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*
*/
GT_STATUS csRefTsn802_1cb_VTcamAddRule
(
  IN GT_U32  vTcamMngId,
  IN GT_U32  vTcamId,
  IN GT_U32  ruleId,
  IN GT_U16  vidx,
  IN GT_U16  stream_index,
  IN GT_ETHERADDR macAddr,
  IN GT_U16  in_vlan
)
{
    GT_STATUS  rc             =  GT_OK;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_ATTRIBUTES_STC     ruleAttributes;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_STC           ruleType;
    CPSS_DXCH_VIRTUAL_TCAM_RULE_DATA_STC           ruleData;
    CPSS_DXCH_PCL_RULE_FORMAT_UNT                  mask;
    CPSS_DXCH_PCL_RULE_FORMAT_UNT                  pattern;
    CPSS_DXCH_PCL_ACTION_STC                       action;

    /* Init structs */
    osMemSet(&ruleAttributes, 0, sizeof(CPSS_DXCH_VIRTUAL_TCAM_RULE_ATTRIBUTES_STC));
    osMemSet(&ruleType, 0, sizeof(CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_STC));
    osMemSet(&ruleData, 0, sizeof(CPSS_DXCH_VIRTUAL_TCAM_RULE_DATA_STC));
    osMemSet(&mask, 0, sizeof(CPSS_DXCH_PCL_RULE_FORMAT_UNT));
    osMemSet(&pattern, 0, sizeof(CPSS_DXCH_PCL_RULE_FORMAT_UNT));
    osMemSet(&action, 0, sizeof(CPSS_DXCH_PCL_ACTION_STC));

    ruleAttributes.priority       = 0;
    ruleType.rule.pcl.ruleFormat  = CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_20_E;
    ruleType.ruleType             = CPSS_DXCH_VIRTUAL_TCAM_RULE_TYPE_PCL_E;

    csRefTsn802_1cb_SetRuleIngrUdbOnly(&mask, &pattern, &action, macAddr, in_vlan,
                                      vidx, stream_index);

    ruleData.rule.pcl.maskPtr    =  &mask;
    ruleData.rule.pcl.patternPtr =  &pattern;
    ruleData.rule.pcl.actionPtr  =  &action;
    ruleData.valid               =  GT_TRUE;

    CPSS_CALL(cpssDxChVirtualTcamRuleWrite(vTcamMngId, vTcamId, ruleId,
                                          &ruleAttributes, &ruleType, &ruleData));

    osPrintf("--- cpssDxChVirtualTcamRuleWrite(): Create a new IPCL rule:\n");
    osPrintf("    vTcamMngId: %d\n", vTcamMngId);
    osPrintf("    vTcamId:    %d\n", vTcamId);
    osPrintf("    ruleId:     %d\n", ruleId);
    osPrintf("    Qulifier 1: Dst MAC address: %02x:%02x:%02x:%02x:%02x:%02x\n",
            pattern.ruleIngrUdbOnly.udb[2], pattern.ruleIngrUdbOnly.udb[3],
            pattern.ruleIngrUdbOnly.udb[4], pattern.ruleIngrUdbOnly.udb[5],
            pattern.ruleIngrUdbOnly.udb[6], pattern.ruleIngrUdbOnly.udb[7]);
    osPrintf("    Qulifier 2: VID0 TPID:       0x%02x%02x\n", pattern.ruleIngrUdbOnly.udb[17],
            pattern.ruleIngrUdbOnly.udb[18]);
    osPrintf("                VID0 Tag:        0x%02x%02x\n", pattern.ruleIngrUdbOnly.udb[14],
            pattern.ruleIngrUdbOnly.udb[15]);
    osPrintf("    Action:\n");
    osPrintf("     - bypassIngressPipe:       GT_TRUE\n");
    osPrintf("     - outIf.outInterface.type: CPSS_INTERFACE_VIDX_E\n");
    osPrintf("     - outIf.outInterface.vidx: %d\n", vidx);
    osPrintf("     - flowId (stream id):      %d\n", stream_index);

    return rc;
}


/**
* @internal csRefTsn802_1cb_ConfigureCounterToRule function
* @endinternal
*
* @brief   This function adds a counter to an IPCL client and prints the counter-id to the user to be used and read.
*          Incremented for each rule hit.
*
* @note   APPLICABLE DEVICES:      All.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum  - device id.
*
* @retval GT_OK                      - on success
* @retval GT_BAD_PARAM          - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*
*/
GT_STATUS csRefTsn802_1cb_ConfigureCounterToRule
(
  IN GT_U8 devNum
)
{
    GT_STATUS    rc             =  GT_OK;
    GT_U64       indexRangesBmp =  {{0,1}};
    GT_U32       block_num      =  0;

    /* Create counter PACKETS/BYTES */
    CPSS_CALL(cpssDxChCncCounterFormatSet(devNum, block_num,
                                          CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_0_E));
    /* Binds the CNC counter to IPCL client */
    CPSS_CALL(cpssDxChCncBlockClientEnableSet(devNum, block_num,
                      CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_PARALLEL_0_E, GT_TRUE));
    CPSS_CALL(cpssDxChCncBlockClientRangesSet(devNum ,block_num,
                      CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_PARALLEL_0_E, &indexRangesBmp));

    osPrintf("--- csRefTsn802_1cb_ConfigureCounterToRule():\n");
    osPrintf("    block number: %d\n", block_num);
    osPrintf("    format:       CPSS_DXCH_CNC_COUNTER_FORMAT_MODE_0_E\n");
    osPrintf("    client:       CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_PARALLEL_0_E\n");
    osPrintf("    index:        0x%08x%08x\n", indexRangesBmp.l[0], indexRangesBmp.l[1]);

    return rc;
}


/**
* @internal csRefTsn802_1cb_ConfigureStream function
* @endinternal
*
* @brief   This function responsible for configuring the stream entry.
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum  - device id.
* @param[in] index   - the stream-id value.
* @param[in] seqNumAssignmentEnable    - Indicates whether to enable packet Sequence Number (currentSeqNum) increment.
* @param[in] individualRecoveryEnable  - Indicates whether to enable Match Recovery Algorithm for stream,
*                                        thus enabling packet discarding.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*
*/
GT_STATUS csRefTsn802_1cb_ConfigureStream
(
  IN GT_U8  devNum,
  IN GT_U32 index,
  IN GT_BOOL seqNumAssignmentEnable,
  IN GT_BOOL individualRecoveryEnable
)
{
    GT_STATUS  rc     =  GT_OK;
    CPSS_DXCH_STREAM_SNG_IRF_ENTRY_STC     sngIrfInfo;
    CPSS_DXCH_STREAM_SRF_MAPPING_ENTRY_STC srfMappingInfo;

    osMemSet(&sngIrfInfo, 0, sizeof(CPSS_DXCH_STREAM_SNG_IRF_ENTRY_STC));
    sngIrfInfo.currentSeqNum            = 0;
    sngIrfInfo.seqNumAssignmentEnable   = seqNumAssignmentEnable;
    sngIrfInfo.resetSeqNum              = GT_FALSE;
    sngIrfInfo.individualRecoveryEnable = individualRecoveryEnable;
    sngIrfInfo.takeAnySeqNum            = GT_TRUE;
    sngIrfInfo.recoverySeqNum           = 0;

    osMemSet(&srfMappingInfo, 0, sizeof(CPSS_DXCH_STREAM_SRF_MAPPING_ENTRY_STC));
    CPSS_CALL(cpssDxChStreamEntrySet(devNum, index, &sngIrfInfo, &srfMappingInfo));

    osPrintf("--- cpssDxChStreamEntrySet():\n");
    osPrintf("    index (stream id):        %d\n", index);
    osPrintf("    currentSeqNum:            %d\n", sngIrfInfo.currentSeqNum);
    osPrintf("    seqNumAssignmentEnable:   %d\n", sngIrfInfo.seqNumAssignmentEnable);
    osPrintf("    resetSeqNum:              %d\n", sngIrfInfo.resetSeqNum);
    osPrintf("    individualRecoveryEnable: %d\n", sngIrfInfo.individualRecoveryEnable);
    osPrintf("    takeAnySeqNum:            %d\n", sngIrfInfo.takeAnySeqNum);
    osPrintf("    recoverySeqNum:           %d\n", sngIrfInfo.recoverySeqNum);

    return rc;
}


/**
* @internal csRefTsn802_1cb_EnableStreamAssignmentFromFdb function
* @endinternal
*
* @brief   This function sets the muxing mode so bits [10:0] will be assign for the stream-id and bit 11 will be assigned for the FDB lookup type (SA/DA).
*
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum  - device id.
*
* @retval GT_OK                      - on success
* @retval GT_BAD_PARAM          - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*
*/
GT_STATUS csRefTsn802_1cb_EnableStreamAssignmentFromFdb
(
  IN GT_U8  devNum
)
{
    GT_STATUS  rc     =  GT_OK;
    CPSS_DXCH_BRG_FDB_EPG_CONFIG_STC epgConfig;

    osMemSet(&epgConfig, 0, sizeof(CPSS_DXCH_BRG_FDB_EPG_CONFIG_STC));
    epgConfig.streamIdEpgMode =
                       CPSS_DXCH_BRG_FDB_STREAM_ID_EPG_MODE_STREAM_ID_11_BITS_LOOKUP_MODE_1_BIT_E;
    CPSS_CALL(cpssDxChBrgFdbEpgConfigSet(devNum, &epgConfig));

    osPrintf("--- cpssDxChBrgFdbEpgConfigSet(): Set Stream assignment based on FDB mode: " \
            "CPSS_DXCH_BRG_FDB_STREAM_ID_EPG_MODE_STREAM_ID_11_BITS_LOOKUP_MODE_1_BIT_E\n");

    return rc;
}


/**
* @internal csRefTsn802_1cb_CreateVlan function
* @endinternal
*
* @brief   This function creates a new VLAN.
*
* @note   APPLICABLE DEVICES:      All.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum  - device id.
* @param[in] vlan_id  - the vlan to create.
*
* @retval GT_OK                      - on success
* @retval GT_BAD_PARAM          - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*
*/
GT_STATUS csRefTsn802_1cb_CreateVlan
(
  IN GT_U8  devNum,
  IN GT_U16 vlan_id
)
{
    GT_STATUS  rc     =  GT_OK;
    CPSS_PORTS_BMP_STC portsMembers;
    CPSS_PORTS_BMP_STC portsTagging;
    CPSS_DXCH_BRG_VLAN_INFO_STC  vlanInfo;
    CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC portsTaggingCmd;

    osMemSet(&portsMembers, 0, sizeof(CPSS_PORTS_BMP_STC));
    osMemSet(&portsTagging, 0, sizeof(CPSS_PORTS_BMP_STC));
    osMemSet(&vlanInfo, 0, sizeof(CPSS_DXCH_BRG_VLAN_INFO_STC));
    osMemSet(&portsTaggingCmd, 0, sizeof(CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC));
    vlanInfo.autoLearnDisable = GT_TRUE;
    vlanInfo.naMsgToCpuEn     = GT_TRUE;
    vlanInfo.vrfId            = 0;
    vlanInfo.floodVidx        = 0x0fff;
    vlanInfo.fidValue         = vlan_id;
    vlanInfo.unregIpmEVidx    = 0x0fff;
    CPSS_CALL(cpssDxChBrgVlanEntryWrite(devNum, vlan_id, &portsMembers, &portsTagging,
                                        &vlanInfo, &portsTaggingCmd));

    osPrintf("--- cpssDxChBrgVlanEntryWrite(): Create new VLAN: %d\n", vlan_id);

    return rc;
}


/**
* @internal csRefTsn802_1cb_ClearVlanEntry function
* @endinternal
*
* @brief   This function remove (clears) a VLAN entry by index.
*
* @note   APPLICABLE DEVICES:      All.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum  - device id.
* @param[in] vlan_id  - the vlan to create.
*
* @retval GT_OK                      - on success
* @retval GT_BAD_PARAM          - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*
*/
GT_STATUS csRefTsn802_1cb_ClearVlanEntry
(
  IN GT_U8  devNum,
  IN GT_U16 vlan_id
)
{
    GT_STATUS  rc     =  GT_OK;
    CPSS_PORTS_BMP_STC portsMembers;
    CPSS_PORTS_BMP_STC portsTagging;
    CPSS_DXCH_BRG_VLAN_INFO_STC  vlanInfo;
    CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC portsTaggingCmd;

    osMemSet(&portsMembers, 0, sizeof(CPSS_PORTS_BMP_STC));
    osMemSet(&portsTagging, 0, sizeof(CPSS_PORTS_BMP_STC));
    osMemSet(&vlanInfo, 0, sizeof(CPSS_DXCH_BRG_VLAN_INFO_STC));
    osMemSet(&portsTaggingCmd, 0, sizeof(CPSS_DXCH_BRG_VLAN_PORTS_TAG_CMD_STC));
    CPSS_CALL(cpssDxChBrgVlanEntryWrite(devNum, vlan_id, &portsMembers, &portsTagging,
                                        &vlanInfo, &portsTaggingCmd));

    return rc;
}


/**
* @internal csRefTsn802_1cb_CreateStaticMacEntry function
* @endinternal
*
* @brief   This function creates a new static mac address entry.
* @note   APPLICABLE DEVICES:      All.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum  - device id.
* @param[in] vlan_id  - vlan for the fdb lookup.
* @param[in] macAddr  - the mac address for the fdb lookup.
* @param[in] vidx  - the vidx target for packets with an fdb lookup hit.
* @param[in] stream_index  - stream-id value classification for packets with an fdb lookup hit.
*
* @retval GT_OK                      - on success
* @retval GT_BAD_PARAM          - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*
*/
GT_STATUS csRefTsn802_1cb_CreateStaticMacEntry
(
  IN GT_U8  devNum,
  IN GT_U16 vlan_id,
  IN GT_ETHERADDR macAddr,
  IN GT_U16 vidx,
  IN GT_U16 stream_index
)
{
    GT_STATUS  rc     =  GT_OK;
    CPSS_MAC_ENTRY_EXT_STC macEntry;

    osMemSet(&macEntry, 0, sizeof(CPSS_MAC_ENTRY_EXT_STC));
    macEntry.key.entryType = CPSS_MAC_ENTRY_EXT_TYPE_MAC_ADDR_E;
    macEntry.key.vid1      = 0;
    macEntry.key.key.macVlan.macAddr = macAddr;
    macEntry.key.key.macVlan.vlanId  = vlan_id;
    macEntry.dstInterface.type = CPSS_INTERFACE_VIDX_E;
    macEntry.dstInterface.vidx = vidx;
    macEntry.age = GT_FALSE;
    macEntry.isStatic = GT_TRUE;
    macEntry.epgNumber = stream_index;
    CPSS_CALL(cpssDxChBrgFdbMacEntrySet(devNum, &macEntry));

    osPrintf("--- cpssDxChBrgFdbMacEntrySet(): Add new static MC FDB entry:\n");
    osPrintf("    mac:                 %02x:%02x:%02x:%02x:%02x:%02x\n", macAddr.arEther[0],
                                       macAddr.arEther[1], macAddr.arEther[2], macAddr.arEther[3],
                                       macAddr.arEther[4], macAddr.arEther[5]);
    osPrintf("    vlan:                %d\n", vlan_id);
    osPrintf("    target vidx:         %d\n", vidx);
    osPrintf("    assosiate stream id: %d\n", stream_index);
    return rc;
}


/**
* @internal csRefTsn802_1cb_AddPortMemberToVlan function
* @endinternal
*
* @brief   This function adds a port to a vlan as a member.
* @note   APPLICABLE DEVICES:      All.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum  - device id.
* @param[in] vlan_id - the vlan to add a port into it.
* @param[in] port    - the port member to add to the vlan.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*
*/
GT_STATUS csRefTsn802_1cb_AddPortMemberToVlan
(
  IN GT_U8  devNum,
  IN GT_U16 vlan_id,
  IN GT_U32 port
)
{
    GT_STATUS  rc     =  GT_OK;

    CPSS_CALL(cpssDxChBrgVlanMemberAdd(devNum, vlan_id, port, GT_FALSE /*isTagged*/,
                                      CPSS_DXCH_BRG_VLAN_PORT_OUTER_TAG0_INNER_TAG1_CMD_E));
    osPrintf("--- cpssDxChBrgVlanMemberAdd(): Add port %d to VLAN %d\n", port, vlan_id);

    return rc;
}


/**
* @internal csRefTsn802_1cb_AddPortToMc function
* @endinternal
*
* @brief   This function adds a port to multicast group
* @note   APPLICABLE DEVICES:      All.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum  - device id.
* @param[in] vidx  - vidx
* @param[in] port  - the port member to add to the vidx.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*
*/
GT_STATUS csRefTsn802_1cb_AddPortToMc
(
  IN GT_U8  devNum,
  IN GT_U16 vidx,
  IN GT_U32 port
)
{
    GT_STATUS  rc     =  GT_OK;

    CPSS_CALL(cpssDxChBrgMcMemberAdd(devNum, vidx, port));
    osPrintf("--- cpssDxChBrgMcMemberAdd(): Add port %d to vidx %d\n", port, vidx);

    return rc;
}


/**
* @internal csRefTsn802_1cb_ConfigureTag1 function
* @endinternal
*
* @brief   This function responsible for adding/removing the 802.1cb r-tag
* @note   APPLICABLE DEVICES:      Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum  - device id.
*
* @retval GT_OK                      - on success
* @retval GT_BAD_PARAM          - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
* @retval GT_FAIL                  - otherwise
*
*/
GT_STATUS csRefTsn802_1cb_ConfigureTag1
(
  IN GT_U8  devNum
)
{
    GT_STATUS  rc     =  GT_OK;

    /* set Src-ID value to add Tag1 */
    CPSS_CALL(cpssDxChBrgSrcIdAddTag1LocationSet(devNum, 11));

    /* remove the R-Tag (Tag1) from the packet if arrives without Tag1 or the Src-ID=0 */
    CPSS_CALL(cpssDxChBrgVlanRemoveVlanTag1IfZeroModeSet(devNum,
          CPSS_DXCH_BRG_VLAN_REMOVE_TAG1_IF_INGRESS_WITHOUT_TAG1_AND_NOT_ADD_TAG_1_FROM_SRC_ID_E));

    return rc;
}


/**
* @internal csRefTsn802_1cb_ConfigureEgressPort function
* @endinternal
*
* @brief   This function responsible for configuring the egress ports (eports) of the replicated packet
*          to allow double-tagged packets where the inner tag (tag1) is the R-tag.
* @note   APPLICABLE DEVICES:      All.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum  - device id.
* @param[in] eport   - the egress port for the outgoing replicated 802.cb packet.
*
* @retval GT_OK                     - on success
* @retval GT_BAD_PARAM              - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE  - on not applicable device
* @retval GT_FAIL                   - otherwise
*
*/
GT_STATUS csRefTsn802_1cb_ConfigureEgressPort
(
  IN GT_U8  devNum,
  IN GT_U32 eport
)
{
    GT_STATUS  rc     =  GT_OK;

    CPSS_CALL(cpssDxChBrgVlanEgressPortTagStateModeSet(devNum, eport,
                              CPSS_DXCH_BRG_VLAN_EPORT_TAG_STATE_EPORT_MODE_E));
    CPSS_CALL(cpssDxChBrgVlanEgressPortTagStateSet(devNum, eport,
                              CPSS_DXCH_BRG_VLAN_PORT_OUTER_TAG0_INNER_TAG1_CMD_E));

    return rc;
}


/**
* @internal csRefTsn802_1cb_ConfigureMllPairs function
* @endinternal
*
* @brief   This function configuring the L2 MLL lookup translation table (LTT) entry and its correspond L2 MLL entry for duplicating
*          802.1cb packets. When L2MLL is triggered and target ePorts assigned to the packet, it is possible to update macDA
*          and Vlan of the outgoing packet.
* @note   APPLICABLE DEVICES:      All.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum        - device id.
* @param[in] lttIndex      - the LTT entry index. needs to be exactly the value of the assigned vidx minus the threshold value for
*                            maximal VIDX value that refers to a port distribution list(VIDX).
* @param[in] mllEntryIndex - the MLL entry index.
* @param[in] ingress_port  - ingress port for the incoming packet.
* @param[in] eport_1       - first egress eport of a replicated packet.
* @param[in] eport_2       - first egress eport of a replicated packet.
*
* @retval GT_OK                     - on success
* @retval GT_BAD_PARAM              - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE  - on not applicable device
* @retval GT_FAIL                   - otherwise
*
*/
GT_STATUS csRefTsn802_1cb_ConfigureMllPairs
(
  IN GT_U8  devNum,
  IN GT_U32 lttIndex,
  IN GT_U32 mllEntryIndex,
  IN GT_U32 ingress_port,
  IN GT_U32 eport_1,
  IN GT_U32 eport_2
)
{
    GT_STATUS                       rc         =  GT_OK;
    CPSS_DXCH_L2_MLL_LTT_ENTRY_STC  lttEntry;
    CPSS_DXCH_L2_MLL_PAIR_STC       mllPairEntry;

    osMemSet(&lttEntry, 0, sizeof(CPSS_DXCH_L2_MLL_LTT_ENTRY_STC));
    lttEntry.mllPointer    = mllEntryIndex;
    lttEntry.entrySelector = CPSS_DXCH_L2_MLL_PAIR_ENTRY_SELECTOR_FIRST_MLL_E;
    lttEntry.mllMaskProfileEnable = GT_FALSE;
    lttEntry.mllMaskProfileEnable = 0x00000000;
    CPSS_CALL(cpssDxChL2MllLttEntrySet(devNum, lttIndex, &lttEntry));

    osPrintf("---cpssDxChL2MllLttEntrySet(): Configure L2 MLL LTT entry:\n");
    osPrintf("   lttIndex:      %d:\n", lttIndex);
    osPrintf("   mllEntryIndex: %d:\n", mllEntryIndex);

    osMemSet(&mllPairEntry, 0, sizeof(CPSS_DXCH_L2_MLL_PAIR_STC));
    mllPairEntry.firstMllNode.egressInterface.type              = CPSS_INTERFACE_PORT_E;
    mllPairEntry.firstMllNode.egressInterface.devPort.hwDevNum  = ingress_port;
    mllPairEntry.firstMllNode.egressInterface.devPort.portNum   = eport_1;
    mllPairEntry.secondMllNode.last                             = GT_TRUE;
    mllPairEntry.secondMllNode.egressInterface.type             = CPSS_INTERFACE_PORT_E;
    mllPairEntry.secondMllNode.egressInterface.devPort.hwDevNum = ingress_port;
    mllPairEntry.secondMllNode.egressInterface.devPort.portNum  = eport_2;
    /* Point to next mll pointer */
    mllPairEntry.nextPointer                                    = mllEntryIndex + 1;
    mllPairEntry.entrySelector       =  CPSS_DXCH_L2_MLL_PAIR_ENTRY_SELECTOR_FIRST_MLL_E;

    CPSS_CALL(cpssDxChL2MllPairWrite(devNum, mllEntryIndex, CPSS_DXCH_PAIR_READ_WRITE_WHOLE_E,
                                     &mllPairEntry)); /* Write L2 MLL entry to L2 MLL Table. */

    osPrintf("---cpssDxChL2MllPairWrite(): Configure L2 MLL table entry fields:\n");
    osPrintf("   firstMllNode:\n");
    osPrintf("        hwDevNum:        %d:\n", ingress_port);
    osPrintf("        portNum (eport): %d:\n", eport_1);
    osPrintf("        type (eport):    CPSS_INTERFACE_PORT_E:\n");
    osPrintf("        portNum (eport): %d:\n", eport_1);
    osPrintf("   secondMllNode:\n");
    osPrintf("        hwDevNum:        %d:\n", ingress_port);
    osPrintf("        portNum (eport): %d:\n", eport_2);
    osPrintf("        type (eport):    CPSS_INTERFACE_PORT_E:\n");

    return rc;
}


/**
* @internal csRefTsn802_1cb_MapEportToPhysicalPorts function
* @endinternal
*
* @brief   This function maps between a physical port to a eport.
* @note   APPLICABLE DEVICES:      All.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum       - device id.
* @param[in] eport        - eport value.
* @param[in] ingress_port - the ingress port of the incoming packet.
* @param[in] egress_port  - physical egress port to assign the eport.
*
* @retval GT_OK                     - on success
* @retval GT_BAD_PARAM              - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE  - on not applicable device
* @retval GT_FAIL                   - otherwise
*
*/
GT_STATUS csRefTsn802_1cb_MapEportToPhysicalPorts
(
  IN GT_U8  devNum,
  IN GT_U32 eport,
  IN GT_U32 ingress_port,
  IN GT_U32 egress_port
)
{
    GT_STATUS               rc     =  GT_OK;
    CPSS_INTERFACE_INFO_STC physicalInfo;

    osMemSet(&physicalInfo, 0, sizeof(CPSS_INTERFACE_INFO_STC));
    physicalInfo.type             = CPSS_INTERFACE_PORT_E;
    physicalInfo.devPort.hwDevNum = ingress_port;
    physicalInfo.devPort.portNum  = egress_port;
    CPSS_CALL(cpssDxChBrgEportToPhysicalPortTargetMappingTableSet(devNum, eport, &physicalInfo));

    osPrintf("---cpssDxChBrgEportToPhysicalPortTargetMappingTableSet(): " \
            "Map eport to phyical port:\n");
    osPrintf("   eport():  %d\n", eport);
    osPrintf("   hwDevNum: %d\n", ingress_port);
    osPrintf("   portNum:  %d\n", egress_port);

    return rc;
}


/**
* @internal csRefTsn802_1cb_CreateArpEntryWithMac function
* @endinternal
*
* @brief   This function configures an ARP entry with a changed MAC address and assigned eport.
* @note   APPLICABLE DEVICES:      All.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum     - device id.
* @param[in] eport      - eport to be assigned  with the ARP entry.
* @param[in] arp_entry  - ARP entry index.
* @param[in] arpMacAddr - D-MAC address of the MAC entry. This will be the new D-MAC of the replicated packet.
*
* @retval GT_OK                     - on success
* @retval GT_BAD_PARAM              - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE  - on not applicable device
* @retval GT_FAIL                   - otherwise
*
*/
GT_STATUS csRefTsn802_1cb_CreateArpEntryWithMac
(
  IN GT_U8  devNum,
  IN GT_U32 eport,
  IN GT_U32 arp_entry,
  IN GT_ETHERADDR arpMacAddr
)
{
    GT_STATUS                          rc     =  GT_OK;
    CPSS_DXCH_BRG_EGRESS_PORT_INFO_STC egressInfo;

    osMemSet(&egressInfo, 0, sizeof(CPSS_DXCH_BRG_EGRESS_PORT_INFO_STC));
    egressInfo.tsPassengerPacketType  = CPSS_DXCH_TUNNEL_PASSENGER_ETHERNET_E;
    egressInfo.arpPtr                 = arp_entry;
    egressInfo.modifyMacSa            = GT_FALSE;
    egressInfo.modifyMacDa            = GT_TRUE;
    egressInfo.forceArpTsPortDecision = GT_FALSE;
    CPSS_CALL(cpssDxChBrgEportToPhysicalPortEgressPortInfoSet(devNum, eport, &egressInfo));

    /* Write an ARP entry to router ARP table */
    CPSS_CALL(cpssDxChIpRouterArpAddrWrite(devNum, arp_entry, &arpMacAddr));

    osPrintf("---cpssDxChIpRouterArpAddrWrite(): Create new ARP entry (modifyMacDa):\n");
    osPrintf("   eport:      %d\n", eport);
    osPrintf("   arp_entry:  %d\n", arp_entry);
    osPrintf("   arpMacAddr: %02x:%02x:%02x:%02x:%02x:%02x\n", arpMacAddr.arEther[0],
                                                                arpMacAddr.arEther[1],
                                                                arpMacAddr.arEther[2],
                                                                arpMacAddr.arEther[3],
                                                                arpMacAddr.arEther[4],
                                                                arpMacAddr.arEther[5]);

    return rc;
}


/**
* @internal csRefTsn802_1cb_ConfigureEgressPortVlan function
* @endinternal
*
* @brief  This function configures the egress eport to be able to change the outgoing packet's VID0 value and VID0-UP value.
* @note   APPLICABLE DEVICES:      All.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum    - device id.
* @param[in] eport     - the configured eport.
* @param[in] vlan      - vlan-id value of the outer vlan of the outgoing packet.
* @param[in] vlan_up0  - vlan's UP value of the outer vlan of the outgoing packet.
*
* @retval GT_OK                     - on success
* @retval GT_BAD_PARAM              - on wrong parameters
* @retval GT_NOT_APPLICABLE_DEVICE  - on not applicable device
* @retval GT_FAIL                   - otherwise
*
*/
GT_STATUS csRefTsn802_1cb_ConfigureEgressPortVlan
(
  IN GT_U8  devNum,
  IN GT_U32 eport,
  IN GT_U16 vlan,
  IN GT_U16 vlan_up0
)
{
    GT_STATUS  rc     =  GT_OK;

    /* Enable changing UP0 of the egress packet */
    CPSS_CALL(cpssDxChBrgVlanPortVid0CommandEnableSet(devNum, eport, GT_TRUE));
    /* Sets port default VID0 to vlan*/
    CPSS_CALL(cpssDxChBrgVlanPortVidSet(devNum, eport, CPSS_DIRECTION_EGRESS_E, vlan));
    /* Enable changing UP1 of the egress packet */
    CPSS_CALL(cpssDxChBrgVlanPortUp0CommandEnableSet(devNum, eport, GT_TRUE));
    /* Sets a new UP value to the egress packet */
    CPSS_CALL(cpssDxChBrgVlanPortUp0Set(devNum, eport, vlan_up0));

    osPrintf("---csRefTsn802_1cb_ConfigureEgressPortVlan(): Configure egress port vid0 values:\n");
    osPrintf("   vid0:    %d\n", vlan);
    osPrintf("   vid0 UP: %d\n", vlan_up0);

    return rc;
}



GT_STATUS csRefTsn802_1cb_DeleteAllGlobalConfigurations
(
  IN GT_U8  devNum
)
{
  GT_STATUS  rc                =  GT_OK;
  GT_U32     tpid_index        =  1;
  GT_U32     tpid_profile      =  0;
  GT_U32     tpid_tag_protocol = 0x8100;
  CPSS_DXCH_BRG_VLAN_8B_TAG_PARSE_CONFIG_STC tag_struct;
  CPSS_DXCH_BRG_FDB_EPG_CONFIG_STC           epgConfig;

  osPrintf("--- csRefTsn802_1cb_DeleteAllGlobalConfigurations(): Delete Global configurations.\n");

  /* ---------------------- Restore global stream configurations: ---------------------- */
  /* Disable the SNG (Sequence Number Generation) */
  CPSS_CALL(cpssDxChStreamSngEnableSet(devNum, GT_FALSE));
  /* Disable the IRF (Individual Recovery Function) */
  CPSS_CALL(cpssDxChStreamIrfEnableSet(devNum, GT_FALSE));
  /* Set the counting mode for streams */
  CPSS_CALL(cpssDxChStreamCountingModeSet(devNum, CPSS_DXCH_CNC_BYTE_COUNT_MODE_L2_E, GT_TRUE));
  CPSS_CALL(cpssDxChBrgFdbStaticDelEnable(devNum, CPSS_DXCH_BRG_FDB_DEL_MODE_DYNAMIC_ONLY_E));

  /* ---------------------- Restore TPID global configurations: ---------------------- */
  /* Set TPID tag 0x8100 for ingress table */
  CPSS_CALL(cpssDxChBrgVlanTpidEntrySet(devNum, CPSS_DXCH_ETHERTYPE_TABLE_INGRESS_E,
                                        tpid_index, tpid_tag_protocol));
  /* Set TPID tag 0xF1C1 for egress table */
  CPSS_CALL(cpssDxChBrgVlanTpidEntrySet(devNum, CPSS_DXCH_ETHERTYPE_TABLE_EGRESS_E,
                                        tpid_index, tpid_tag_protocol));

  CPSS_CALL(cpssDxChBrgVlanTpidTagTypeSet(devNum, CPSS_DIRECTION_INGRESS_E,
                                          tpid_index, CPSS_BRG_TPID_SIZE_TYPE_4_BYTES_E));
  CPSS_CALL(cpssDxChBrgVlanTpidTagTypeSet(devNum, CPSS_DIRECTION_EGRESS_E,
                                          tpid_index, CPSS_BRG_TPID_SIZE_TYPE_4_BYTES_E));

  osMemSet(&tag_struct, 0, sizeof(CPSS_DXCH_BRG_VLAN_8B_TAG_PARSE_CONFIG_STC));
  CPSS_CALL(cpssDxChBrgVlanPort8BTagInfoSet(devNum, CPSS_DIRECTION_INGRESS_E,
                                            tpid_index, &tag_struct));

  CPSS_CALL(cpssDxChBrgVlanIngressTpidProfileSet(devNum, tpid_profile,
                                                CPSS_VLAN_ETHERTYPE0_E, 255));
  CPSS_CALL(cpssDxChBrgVlanIngressTpidProfileSet(devNum, tpid_profile,
                                                CPSS_VLAN_ETHERTYPE1_E, 255));

  /* Reset FDB assignment mode: */
  osMemSet(&epgConfig, 0, sizeof(CPSS_DXCH_BRG_FDB_EPG_CONFIG_STC));
  epgConfig.streamIdEpgMode = CPSS_DXCH_BRG_FDB_STREAM_ID_EPG_MODE_EPG_12_BITS_E;
  CPSS_CALL(cpssDxChBrgFdbEpgConfigSet(devNum, &epgConfig));

  /* Restore tag1 Global configurations */
  CPSS_CALL(cpssDxChBrgSrcIdAddTag1LocationSet(devNum, 15));
  CPSS_CALL(cpssDxChBrgVlanRemoveVlanTag1IfZeroModeSet(devNum,
                            CPSS_DXCH_BRG_VLAN_REMOVE_TAG1_IF_ZERO_DISABLE_E));

  return rc;
}


GT_STATUS csRefTsn802_1cb_DeleteAllConfigurationsForFlow1
(
  IN GT_U8  devNum,
  IN GT_U32 ingress_port,
  IN GT_U32 egress_port_1,
  IN GT_U32 egress_port_2,
  IN GT_U32 streamIndex,
  IN GT_U32 vlan,
  IN GT_U16 vidx,
  IN GT_ETHERADDR macAddr
)
{
  GT_STATUS  rc             =  GT_OK;
  GT_U32     tpid_profile   =  0;
  CPSS_DXCH_STREAM_SNG_IRF_ENTRY_STC     sngIrfInfo;
  CPSS_DXCH_STREAM_SRF_MAPPING_ENTRY_STC srfMappingInfo;
  CPSS_MAC_ENTRY_EXT_KEY_STC             macEntryKey;

  osPrintf("--- csRefTsn802_1cb_DeleteAllConfigurationsForFlow1(): " \
          "Delete flow-1's configurations.\n");

  osPrintf("--- Restore TPID configurations.\n");
  CPSS_CALL(cpssDxChBrgVlanPortIngressTpidProfileSet(devNum, ingress_port, CPSS_VLAN_ETHERTYPE0_E,
                                                    GT_FALSE, tpid_profile));
  CPSS_CALL(cpssDxChBrgVlanPortIngressTpidProfileSet(devNum, ingress_port, CPSS_VLAN_ETHERTYPE1_E,
                                                    GT_FALSE, tpid_profile));
  CPSS_CALL(cpssDxChBrgVlanPortEgressTpidSet(devNum, egress_port_1,
                            CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ETHERTYPE0_E, 0));
  CPSS_CALL(cpssDxChBrgVlanPortEgressTpidSet(devNum, egress_port_2,
                            CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ETHERTYPE0_E, 0));
  CPSS_CALL(cpssDxChBrgVlanPortEgressTpidSet(devNum, egress_port_1,
                            CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ETHERTYPE1_E, 0));
  CPSS_CALL(cpssDxChBrgVlanPortEgressTpidSet(devNum, egress_port_2,
                            CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ETHERTYPE1_E, 0));

  osPrintf("--- Clean stream entry.\n");
  osMemSet(&sngIrfInfo, 0, sizeof(CPSS_DXCH_STREAM_SNG_IRF_ENTRY_STC));
  osMemSet(&srfMappingInfo, 0, sizeof(CPSS_DXCH_STREAM_SRF_MAPPING_ENTRY_STC));
  sngIrfInfo.recoverySeqNum = 0xffff;
  CPSS_CALL(cpssDxChStreamEntrySet(devNum, streamIndex, &sngIrfInfo, &srfMappingInfo));

  osPrintf("--- Remove ports from VLAN and clean VLAN entry.\n");
  CPSS_CALL(cpssDxChBrgVlanMemberSet(devNum, vlan, ingress_port, GT_FALSE, GT_FALSE,
                                    CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E));
  CPSS_CALL(cpssDxChBrgVlanMemberSet(devNum, vlan, egress_port_1, GT_FALSE, GT_FALSE,
                                    CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E));
  CPSS_CALL(cpssDxChBrgVlanMemberSet(devNum, vlan, egress_port_2, GT_FALSE, GT_FALSE,
                                    CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E));

  osPrintf("--- Remove ports from MC group.\n");
  CPSS_CALL(cpssDxChBrgMcMemberDelete(devNum, vidx, egress_port_1));
  CPSS_CALL(cpssDxChBrgMcMemberDelete(devNum, vidx, egress_port_2));

  osPrintf("--- Clean MAC entry.\n");
  osMemSet(&macEntryKey, 0, sizeof(CPSS_MAC_ENTRY_EXT_KEY_STC));
  macEntryKey.key.macVlan.macAddr = macAddr;
  macEntryKey.key.macVlan.vlanId  = vlan;
  CPSS_CALL(cpssDxChBrgFdbMacEntryDelete(devNum, &macEntryKey));
  /* Clean VLAN entry: */
  CPSS_CALL(csRefTsn802_1cb_ClearVlanEntry(devNum, vlan));

  return rc;
}


GT_STATUS csRefTsn802_1cb_DeleteAllConfigurationsForFlow2
(
  IN GT_U8  devNum,
  IN GT_U32 ingress_port,
  IN GT_U32 egress_port_1,
  IN GT_U32 egress_port_2,
  IN GT_U32 eport_1,
  IN GT_U32 eport_2,
  IN GT_U32 streamIndex,
  IN GT_U32 vlan,
  IN GT_U32 vTcamMngId,
  IN GT_U32 vTcamId,
  IN GT_U32 ruleId,
  IN GT_U32 lttIndex,
  IN GT_U32 mllEntryIndex,
  IN GT_U32 arp_entry_1,
  IN GT_U32 arp_entry_2,
  IN GT_ETHERADDR arp_1_mac_addr,
  IN GT_ETHERADDR arp_2_mac_addr
)
{
  GT_STATUS  rc             =  GT_OK;
  GT_U32     tpid_profile   =  0;
  GT_U8      devListArr[]   =  {0};
  GT_U32     i;
  GT_U64     indexRangesBmp =  {{0,0}};
  GT_U32     block_num      =  0;
  CPSS_DXCH_STREAM_SNG_IRF_ENTRY_STC     sngIrfInfo;
  CPSS_DXCH_STREAM_SRF_MAPPING_ENTRY_STC srfMappingInfo;
  CPSS_DXCH_PCL_UDB_SELECT_STC           udbSelectStruct;
  CPSS_DXCH_L2_MLL_PAIR_STC              mllPairEntry;
  CPSS_DXCH_L2_MLL_LTT_ENTRY_STC         lttEntry;
  CPSS_INTERFACE_INFO_STC                physicalInfo;
  CPSS_DXCH_BRG_EGRESS_PORT_INFO_STC     egressInfo;

  osPrintf("--- csRefTsn802_1cb_DeleteAllConfigurationsForFlow2(): " \
          "Delete flow-2's configurations:\n");

  osPrintf("--- Restore TPID configurations.\n");
  CPSS_CALL(cpssDxChBrgVlanPortIngressTpidProfileSet(devNum, ingress_port, CPSS_VLAN_ETHERTYPE0_E,
                                                    GT_FALSE, tpid_profile));
  CPSS_CALL(cpssDxChBrgVlanPortIngressTpidProfileSet(devNum, ingress_port, CPSS_VLAN_ETHERTYPE1_E,
                                                    GT_FALSE, tpid_profile));
  CPSS_CALL(cpssDxChBrgVlanPortEgressTpidSet(devNum, eport_1,
                            CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ETHERTYPE0_E, 0));
  CPSS_CALL(cpssDxChBrgVlanPortEgressTpidSet(devNum, eport_2,
                            CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ETHERTYPE0_E, 0));
  CPSS_CALL(cpssDxChBrgVlanPortEgressTpidSet(devNum, eport_1,
                            CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ETHERTYPE1_E, 0));
  CPSS_CALL(cpssDxChBrgVlanPortEgressTpidSet(devNum, eport_2,
                            CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ETHERTYPE1_E, 0));

  osPrintf("--- Clean stream entry.\n");
  osMemSet(&sngIrfInfo, 0, sizeof(CPSS_DXCH_STREAM_SNG_IRF_ENTRY_STC));
  osMemSet(&srfMappingInfo, 0, sizeof(CPSS_DXCH_STREAM_SRF_MAPPING_ENTRY_STC));
  sngIrfInfo.recoverySeqNum = 0xffff;
  CPSS_CALL(cpssDxChStreamEntrySet(devNum, streamIndex, &sngIrfInfo, &srfMappingInfo));
  CPSS_CALL(cpssDxChPclNumberOfStreamIdsSet(devNum, CPSS_PCL_DIRECTION_INGRESS_E, 0));

  osPrintf("--- Remove ports from VLAN and clean VLAN entry.\n");
  CPSS_CALL(cpssDxChBrgVlanMemberSet(devNum, vlan, ingress_port, GT_FALSE,
                                    GT_FALSE, CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E));
  CPSS_CALL(cpssDxChBrgVlanMemberSet(devNum, vlan, egress_port_1, GT_FALSE,
                                    GT_FALSE, CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E));
  CPSS_CALL(cpssDxChBrgVlanMemberSet(devNum, vlan, egress_port_2, GT_FALSE,
                                    GT_FALSE, CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E));
  CPSS_CALL(csRefTsn802_1cb_ClearVlanEntry(devNum, vlan));

  osPrintf("--- Remove IPCL configuations.\n");
  CPSS_CALL(cpssDxChPclFlowIdActionAssignModeSet(devNum, CPSS_PCL_DIRECTION_INGRESS_E,
                              CPSS_PCL_LOOKUP_0_0_E, 0,
                              CPSS_DXCH_PCL_FLOW_ID_ACTION_ASSIGNMENT_MODE_FLOW_ID_STREAM_ID_E));
  CPSS_CALL(cpssDxChPclPortIngressPolicyEnable(devNum, ingress_port, GT_FALSE));
  /* Reset User Defined Bytes: */
  for (i=14; i<22; i++)
  {
    CPSS_CALL(cpssDxChPclUserDefinedByteSet(devNum, CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_20_E,
                                            CPSS_DXCH_PCL_PACKET_TYPE_ETHERNET_OTHER_E,
                                            CPSS_PCL_DIRECTION_INGRESS_E,
                                            i,CPSS_DXCH_PCL_OFFSET_INVALID_E, 0));
  }
  osMemSet(&udbSelectStruct, 0, sizeof(CPSS_DXCH_PCL_UDB_SELECT_STC));
  CPSS_CALL(cpssDxChPclUserDefinedBytesSelectSet(devNum,
                                                CPSS_DXCH_PCL_RULE_FORMAT_INGRESS_UDB_20_E,
                                                CPSS_DXCH_PCL_PACKET_TYPE_ETHERNET_OTHER_E,
                                                CPSS_PCL_LOOKUP_0_0_E, &udbSelectStruct));

  osPrintf("--- Remove TCAM rule and vTCAM configuations.\n");
  CPSS_CALL(cpssDxChVirtualTcamRuleDelete(vTcamMngId, vTcamId, ruleId));
  CPSS_CALL(cpssDxChVirtualTcamRemove(vTcamMngId, vTcamId));
  CPSS_CALL(cpssDxChVirtualTcamManagerDevListRemove(vTcamMngId, devListArr, 1));
  CPSS_CALL(cpssDxChVirtualTcamManagerDelete(vTcamMngId));

  osPrintf("--- Clear counter configuration.\n");
  CPSS_CALL(cpssDxChCncBlockClientEnableSet(devNum, block_num,
                                            CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_PARALLEL_0_E,
                                            GT_FALSE));
  CPSS_CALL(cpssDxChCncBlockClientRangesSet(devNum ,block_num,
                                            CPSS_DXCH_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_PARALLEL_0_E,
                                            &indexRangesBmp));

  osPrintf("--- Remove MLL configurations.\n");
  CPSS_CALL(cpssDxChL2MllLookupMaxVidxIndexSet(devNum, 0xfff));
  osMemSet(&lttEntry, 0, sizeof(CPSS_DXCH_L2_MLL_LTT_ENTRY_STC));
  osMemSet(&mllPairEntry, 0, sizeof(CPSS_DXCH_L2_MLL_PAIR_STC));
  CPSS_CALL(cpssDxChL2MllPairWrite(devNum, mllEntryIndex,
                                  CPSS_DXCH_PAIR_READ_WRITE_WHOLE_E, &mllPairEntry));
  CPSS_CALL(cpssDxChL2MllLttEntrySet(devNum, lttIndex, &lttEntry));

  osPrintf("--- Detached eport from physical port.\n");
  osMemSet(&physicalInfo, 0, sizeof(CPSS_INTERFACE_INFO_STC));
  CPSS_CALL(cpssDxChBrgEportToPhysicalPortTargetMappingTableSet(devNum, eport_1, &physicalInfo));
  CPSS_CALL(cpssDxChBrgEportToPhysicalPortTargetMappingTableSet(devNum, eport_2, &physicalInfo));

  osPrintf("--- Remove ARP entry configurations.\n");
  osMemSet(&egressInfo, 0, sizeof(CPSS_DXCH_BRG_EGRESS_PORT_INFO_STC));
  CPSS_CALL(cpssDxChBrgEportToPhysicalPortEgressPortInfoSet(devNum, eport_1, &egressInfo));
  CPSS_CALL(cpssDxChBrgEportToPhysicalPortEgressPortInfoSet(devNum, eport_2, &egressInfo));
  CPSS_CALL(cpssDxChIpRouterArpAddrWrite(devNum, arp_entry_1, &arp_1_mac_addr));
  CPSS_CALL(cpssDxChIpRouterArpAddrWrite(devNum, arp_entry_2, &arp_2_mac_addr));

  osPrintf("--- Restore eport configurations.\n");
  CPSS_CALL(cpssDxChBrgVlanEgressPortTagStateModeSet(devNum, eport_1,
                                                CPSS_DXCH_BRG_VLAN_EPORT_TAG_STATE_EVLAN_MODE_E));
  CPSS_CALL(cpssDxChBrgVlanEgressPortTagStateModeSet(devNum, eport_2,
                                                CPSS_DXCH_BRG_VLAN_EPORT_TAG_STATE_EVLAN_MODE_E));
  CPSS_CALL( cpssDxChBrgVlanEgressPortTagStateSet(devNum, eport_1,
                                                CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E));
  CPSS_CALL(cpssDxChBrgVlanEgressPortTagStateSet(devNum, eport_2,
                                                CPSS_DXCH_BRG_VLAN_PORT_UNTAGGED_CMD_E));

  osPrintf("--- Remove eports Tag0 configurations.\n");
  /* Remove eport_1's Tag0 configurations: */
  CPSS_CALL(cpssDxChBrgVlanPortVid0CommandEnableSet(devNum, eport_1, GT_FALSE));
  CPSS_CALL(cpssDxChBrgVlanPortVidSet(devNum, eport_1, CPSS_DIRECTION_EGRESS_E, 1));
  CPSS_CALL(cpssDxChBrgVlanPortUp0CommandEnableSet(devNum, eport_1, GT_FALSE));
  CPSS_CALL(cpssDxChBrgVlanPortUp0Set(devNum, eport_1, 0));

  /* Remove eport_2's Tag0 configurations: */
  CPSS_CALL(cpssDxChBrgVlanPortVid0CommandEnableSet(devNum, eport_2, GT_FALSE));
  CPSS_CALL(cpssDxChBrgVlanPortVidSet(devNum, eport_2, CPSS_DIRECTION_EGRESS_E, 1));
  CPSS_CALL(cpssDxChBrgVlanPortUp0CommandEnableSet(devNum, eport_2, GT_FALSE));
  CPSS_CALL(cpssDxChBrgVlanPortUp0Set(devNum, eport_2, 0));

  return rc;
}