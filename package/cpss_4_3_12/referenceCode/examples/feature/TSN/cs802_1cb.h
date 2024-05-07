/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* cs802_1cb.h
*
* DESCRIPTION:
*  This files provide APIs to create VxLAN UNI and NNI interfaces.
*
*
* FILE REVISION NUMBER:
*       $Revision: 1.0 $
*
*******************************************************************************/
#ifndef __cs802_1cb_h
#define __cs802_1cb_h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/generic/cpssTypes.h>
#include "../../prvCsrefLog.h"
#include <gtOs/gtOsMem.h>
#include <gtOs/gtOsInet.h>
#include <cpss/dxCh/dxChxGen/cpssHwInit/cpssDxChHwInit.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgVlan.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgMc.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgE2Phy.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgEgrFlt.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgFdb.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgSrcId.h>
#include <cpss/dxCh/dxChxGen/tti/cpssDxChTti.h>
#include <cpss/dxCh/dxChxGen/tti/cpssDxChTtiTypes.h>
#include <cpss/dxCh/dxChxGen/tunnel/cpssDxChTunnel.h>
#include <cpss/dxCh/dxChxGen/tunnel/cpssDxChTunnelTypes.h>
#include <cpss/dxCh/dxChxGen/pcl/cpssDxChPcl.h>
#include <cpss/dxCh/dxChxGen/ip/cpssDxChIpCtrl.h>
#include <cpss/dxCh/dxChxGen/ip/cpssDxChIp.h>
#include <cpss/dxCh/dxChxGen/stream/cpssDxChStream.h>
#include <cpss/dxCh/dxChxGen/tcam/cpssDxChTcam.h>
#include <cpss/dxCh/dxChxGen/virtualTcam/cpssDxChVirtualTcam.h>
#include <cpss/dxCh/dxChxGen/l2mll/cpssDxChL2Mll.h>


#define CPSS_CALL(function)\
rc = (function);\
if(GT_OK != rc)                                                             \
{ \
  osPrintf("Function %s,Line %d, fail:rc=%d!\n",__FUNCNAME__,__LINE__,rc);  \
  return rc;    \
}


/***********************************************/
/*         Function Definitions                */
/***********************************************/

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
);


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
* @param[in] ingress_port      - the ingress port that needs to be adapted to receive 802.1cb packets.
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
);


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
);


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
);


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
);


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
);


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
);


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
);


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
);


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
);


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
);


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
);


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
);


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
);


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
);


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
);


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
);


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
);


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
);


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
);


GT_STATUS csRefTsn802_1cb_DeleteAllGlobalConfigurations
(
  IN GT_U8  devNum
);


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
);


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
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cs802_1cb_h */