/*******************************************************************************
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
* @file cpssDxChIpTypes.h
*
* @brief DXCH cpss definitions for data types used by Ip
*
* @version   29
********************************************************************************
*/
#ifndef __cpssDxChIpTypesh
#define __cpssDxChIpTypesh

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>
#include <cpss/generic/ip/cpssIpTypes.h>
#include <cpss/generic/cos/cpssCosTypes.h>
#include <cpss/dxCh/dxChxGen/tunnel/cpssDxChTunnelTypes.h>
#include <cpss/dxCh/dxChxGen/cpssDxChTypes.h>
#include <cpss/dxCh/dxChxGen/private/lpm/prvCpssDxChLpmTypes.h>
#include <cpss/dxCh/dxChxGen/lpm/cpssDxChLpmTypes.h>

/* max number of ECMP/QOS group paths for DxCh devices (0-7)*/
#define CPSS_DXCH_IP_MAX_ECMP_QOS_GROUP   7

/* max number of ECMP group paths for LionBo Devices (0-63) */
#define CPSS_DXCH_IP_MAX_LION_ECMP_GROUP   63

/* max number of QoS group paths for LionBo Devices (0-7) */
#define CPSS_DXCH_IP_MAX_LION_QOS_GROUP    7

/* Maximum bucket size in bytes */
#define CPSS_DXCH_MULTI_TARGET_SHAPER_MAX_BUCKET_SIZE_CNS 0xFFF

/**
* @struct CPSS_DXCH_IPV4_PREFIX_STC
 *
 * @brief IPv4 prefix entry
*/
typedef struct{

    /** @brief virtual router id, relevant only if isMcSource
     *  is GT_FALSE
     */
    GT_U32 vrId;

    /** The IP address of this prefix. */
    GT_IPADDR ipAddr;

    /** @brief GT_TRUE: writing MC source ip
     *  GT_FALSE: writing MC group ip or UC ip
     *  isMcSource is GT_TRUE
     *  isMcSource is GT_TRUE
     */
    GT_BOOL isMcSource;

    /** mc Group index row, relevant only if */
    GT_U32 mcGroupIndexRow;

    /** mc Group index column, relevant only if */
    GT_U32 mcGroupIndexColumn;

} CPSS_DXCH_IPV4_PREFIX_STC;

/**
* @struct CPSS_DXCH_IPV6_PREFIX_STC
 *
 * @brief IPv6 prefix entry
*/
typedef struct{

    /** @brief virtual router id, relevant only if isMcSource
     *  is GT_FALSE
     */
    GT_U32 vrId;

    /** The IP address of this prefix. */
    GT_IPV6ADDR ipAddr;

    /** @brief GT_TRUE: writing MC source ip
     *  GT_FALSE: writing MC group ip or UC ip
     *  isMcSource is GT_TRUE
     */
    GT_BOOL isMcSource;

    /** mc Group index row, relevant only if */
    GT_U32 mcGroupIndexRow;

} CPSS_DXCH_IPV6_PREFIX_STC;

/**
* @enum CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ENT
 *
 * @brief Each Look Up Translation Entry may point to a Route/s entry/ies
 * Each (block) route entry block has a type.
*/
typedef enum{

    /** @brief Equal-Cost-MultiPath block,
     *  where traffic is split among the route entries according to a
     *  hash function.
     */
    CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E     = 0,

    /** @brief Quality-of-Service block, where
     *  the route entry is according to the QoSProfiletoRouteBlockOffset
     *  Table.
     */
    CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_QOS_E      = 1,

    /** @brief Regular next hop.
     *  (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_REGULAR_E  = 2,

    /**  @brief Multipath next hop.
     *  (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_MULTIPATH_E  = 3


} CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ENT;

/* Old enumerators - remained for backward compatibility, unused in the code */
#define CPSS_DXCH_IP_ECMP_ROUTE_ENTRY_GROUP_E    CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E
#define CPSS_DXCH_IP_QOS_ROUTE_ENTRY_GROUP_E     CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_QOS_E
#define CPSS_DXCH_IP_REGULAR_ROUTE_ENTRY_GROUP_E CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_REGULAR_E

/*
 * Typedef: struct CPSS_DXCH_IP_ECMP_ENTRY_STC
 *
 * Description: ECMP table entry (APPLICABLE DEVICES: Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X.)
 *
 * Fields:
 *      randomEnable           - Whether to do random selection of next hop.
 *      numOfPaths             - The number of ECMP or QoS paths.
 *                               (APPLICABLE RANGES: 1..4096)
 *      routeEntryBaseIndex    - The base address of the route entries block.
 *                               (APPLICABLE RANGES: 0..24575)
 *      multiPathMode          - multipath mode. Applicable values:
 *                                CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ECMP_E,
 *                                CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_QOS_E.
 *                                (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman)
 */
typedef struct CPSS_DXCH_IP_ECMP_ENTRY_STCT
{
    GT_BOOL                              randomEnable;
    GT_U32                               numOfPaths;
    GT_U32                               routeEntryBaseIndex;
    CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ENT  multiPathMode;
} CPSS_DXCH_IP_ECMP_ENTRY_STC;

/*
 * Typedef: struct CPSS_DXCH_IP_LTT_ENTRY_STC
 *
 * Description: LTT (lookup translation table) entry
 *
 * Fields:
 *      routeType              - Route Entry Type.
 *      numOfPaths             - The number of route paths bound to the lookup.
 *                               This value ranges from 0 (a single path) to
 *                               7 (8 ECMP or QoS route paths).
 *                               For Lion2 and above, value ranges from 0 to 63
 *                               (64 ECMP or 8 QoS).
 *                               (APPLICABLE DEVICES: xCat3; AC5; Lion2)
 *      routeEntryBaseIndex    - For xCat3, Lion2:
 *                               If numOfPaths = 0, this is the direct index to
 *                               the single Route entry. If numOfPaths is
 *                               greater than 0, this is the base index to a
 *                               block of contiguous Route entries.
 *                               (APPLICABLE RANGES: 0..4095)
 *                               For eArch devices:
 *                                If routeType is
 *                                CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_REGULAR_E then
 *                                this is the pointer to the route entry
 *                                (APPLICABLE RANGES: 0..24575)
 *                                Otherwise it is the pointer to ECMP/QoS entry
 *                                (APPLICABLE RANGES: 0..12287)
 *      ucRPFCheckEnable         - Enable Unicast RPF check for this Entry.
 *      sipSaCheckMismatchEnable - Enable Unicast SIP MAC SA match check.
 *      ipv6MCGroupScopeLevel  - This is the IPv6 Multicast group scope level.
 *      priority               - resolution priority between LPM and FDB match results
 *                               (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman)
 *      applyPbr               - apply policy based routing. PBR result has higher priority than LPM result.
 *                               (APPLICABLE DEVICES: AC5P; AC5X; Harrier; Ironman)
 *      epgAssignedToLeafNode  - Valid if EntryType="Leaf" (pointToSip == GT_FALSE)
 *                               EPG assigned to the leaf node
 *                               On SIP search - assigns source_EPG
 *                               On DIP search - assigns target EPG
 *                              (APPLICALBE DEVICES: AAS)
 *
 * Comments:
 *      In eArch devices routeEntryBaseIndex can point to either NH or ECMP
 *      entry. routeType determines the type of pointer: if routeType is
 *      CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_REGULAR_E then it points to NH, else -
 *      to ECMP entry.
 */
typedef struct CPSS_DXCH_IP_LTT_ENTRY_STCT
{
    CPSS_DXCH_IP_ROUTE_ENTRY_METHOD_ENT           routeType;
    GT_U32                                        numOfPaths;
    GT_U32                                        routeEntryBaseIndex;
    GT_BOOL                                       ucRPFCheckEnable;
    GT_BOOL                                       sipSaCheckMismatchEnable;
    CPSS_IPV6_PREFIX_SCOPE_ENT                    ipv6MCGroupScopeLevel;
    CPSS_DXCH_LPM_LEAF_ENTRY_PRIORITY_TYPE_ENT    priority;
    GT_BOOL                                       applyPbr;
    GT_U32                                        epgAssignedToLeafNode;
}CPSS_DXCH_IP_LTT_ENTRY_STC;



/**
* @enum CPSS_DXCH_IP_CPU_CODE_INDEX_ENT
 *
 * @brief this value is added to the CPU code assignment in corse of trap
 * or mirror
*/
typedef enum{

    /** CPU code added index 0 */
    CPSS_DXCH_IP_CPU_CODE_IDX_0_E   = 0,

    /** CPU code added index 1 */
    CPSS_DXCH_IP_CPU_CODE_IDX_1_E   = 1,

    /** CPU code added index 2 */
    CPSS_DXCH_IP_CPU_CODE_IDX_2_E   = 2,

    /** CPU code added index 3 */
    CPSS_DXCH_IP_CPU_CODE_IDX_3_E   = 3

} CPSS_DXCH_IP_CPU_CODE_INDEX_ENT;

/*
 * Typedef: struct CPSS_DXCH_IP_UC_ROUTE_ENTRY_FORMAT_STC
 *
 * Description: Unicast Route Entry
 *
 * Fields:
 *  cmd                - Route entry command. supported commands:
 *                          CPSS_PACKET_CMD_ROUTE_E,
 *                          CPSS_PACKET_CMD_ROUTE_AND_MIRROR_E,
 *                          CPSS_PACKET_CMD_TRAP_TO_CPU_E,
 *                          CPSS_PACKET_CMD_DROP_SOFT_E ,
 *                          CPSS_PACKET_CMD_DROP_HARD_E.
 *  cpuCodeIdx         - the cpu code index of the specific code when trap or
 *                       mirror.
 *  appSpecificCpuCodeEnable- Enable CPU code overwritting according to the
 *                        application specific CPU Code assignment mechanism.
 *  unicastPacketSipFilterEnable - Enable: Assign the command HARD DROP if the
 *                       SIP lookup matches this entry
 *                       Disable: Do not assign the command HARD DROP due to
 *                       the SIP lookup matching this entry
 *  ttlHopLimitDecEnable - Enable TTL/Hop Limit Decrement
 *  ttlHopLimDecOptionsExtChkByPass- TTL/HopLimit Decrement and option/
 *                       Extention check bypass.
 *  ingressMirror      - mirror to ingress analyzer.
 *  ingressMirrorToAnalyzerIndex - One of the possible analyzers. Relevant
 *                       when ingressMirror is GT_TRUE.
 *                       If a previous engine in the pipe assigned a different
 *                       analyzer index, the higher index wins.
 *                       (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; AAS)
 *                       (APPLICABLE RANGES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman 0..6),
*                        (APPLICABLE RANGES: AAS 0..62)
 *  qosProfileMarkingEnable- Enable Qos profile assignment.
 *  qosProfileIndex    - the qos profile to assign in case of
 *                       qosProfileMarkingEnable = GT_TRUE
 *  qosPrecedence      - whether this packet Qos parameters can be overridden
 *                       after this assigment.
 *  modifyUp           - whether to change the packets UP and how.
 *  modifyDscp         - whether to change the packets DSCP and how.
 *  countSet           - The counter set this route entry is linked to
 *  trapMirrorArpBcEnable - enable Trap/Mirror ARP Broadcasts with DIP matching
 *                       this entry
 *  sipAccessLevel     - The security level associated with the SIP.
 *  dipAccessLevel     - The security level associated with the DIP.
 *  ICMPRedirectEnable - Enable performing ICMP Redirect Exception Mirroring.
 *  scopeCheckingEnable- Enable IPv6 Scope Checking.
 *  siteId             - The site id of this route entry.
 *  mtuProfileIndex    - One of the eight global configurable MTU sizes (0..7).
 *  isTunnelStart      - weather this nexthop is tunnel start enrty, in which
 *                       case the outInteface & mac are irrlevant and the tunnel
 *                       id is used.
 *  isNat              - Indicate that the packet is subject to NAT,
 *                       and nextHopNatPointer should be used as a pointer to a NAT entry.
 *                       Relevant only if isTunnelStart = GT_TRUE
 *                       GT_FALSE: use nextHopTunnelPointer as a Tunnel pointer
 *                       GT_TRUE: use nextHopNatPointer as a NAT pointer
 *                       (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
 *  nextHopVlanId      - the output vlan id (used also for SIP RPF check, and
 *                       ICMP check)
 *  nextHopInterface   - the output interface this next hop sends to. relevant
 *                       only if the isTunnelStart = GT_FALSE
 *  nextHopARPPointer  - The ARP Pointer indicating the routed packet MAC DA,
 *                       relevant only if the isTunnelStart = GT_FALSE
 *  nextHopTunnelPointer- the tunnel pointer in case this is a tunnel start
 *                       isTunnelStart = GT_TRUE
 *  nextHopNatPointer   - the NAT pointer in case that
 *                       isTunnelStart = GT_TRUE and isNat = GT_TRUE
 *                       (APPLICABLE DEVICES: Bobcat2; Caelum; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Aldrin; AC3X)
 *  nextHopVlanId1      - The next hop VID1 associated with the Unicast DIP
 *                        lookup and the unicast RPF VID1 assocaited with the
 *                        SIP lookup. (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
 *  targetEpg           - The target EPG(destination group-id) assigned to the packet
 *                        Relevant only if Next hop muxing = CPSS_DXCH_IP_NEXT_HOP_MUX_MODE_TRG_EPG_FOR_UC_E or
 *                        CPSS_DXCH_IP_NEXT_HOP_MUX_MODE_TRG_EPG_FOR_UC_MC_E
 *                        (APPLICABLE RANGES: 0..511)
 *                        (APPLICABLE DEVICES: AC5P; AC5X; Harrier; Ironman)
 */
typedef struct CPSS_DXCH_IP_UC_ROUTE_ENTRY_FORMAT_STCT
{
    CPSS_PACKET_CMD_ENT                 cmd;
    CPSS_DXCH_IP_CPU_CODE_INDEX_ENT     cpuCodeIdx;
    GT_BOOL                             appSpecificCpuCodeEnable;
    GT_BOOL                             unicastPacketSipFilterEnable;
    GT_BOOL                             ttlHopLimitDecEnable;
    GT_BOOL                             ttlHopLimDecOptionsExtChkByPass;
    GT_BOOL                             ingressMirror;
    GT_U32                              ingressMirrorToAnalyzerIndex;
    GT_BOOL                             qosProfileMarkingEnable;
    GT_U32                              qosProfileIndex;
    CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_ENT qosPrecedence;
    CPSS_PACKET_ATTRIBUTE_MODIFY_TYPE_ENT       modifyUp;
    CPSS_PACKET_ATTRIBUTE_MODIFY_TYPE_ENT       modifyDscp;
    CPSS_IP_CNT_SET_ENT                 countSet;
    GT_BOOL                             trapMirrorArpBcEnable;
    GT_U32                              sipAccessLevel;
    GT_U32                              dipAccessLevel;
    GT_BOOL                             ICMPRedirectEnable;
    GT_BOOL                             scopeCheckingEnable;
    CPSS_IP_SITE_ID_ENT                 siteId;
    GT_U32                              mtuProfileIndex;
    GT_BOOL                             isTunnelStart;
    GT_BOOL                             isNat;
    GT_U16                              nextHopVlanId;
    CPSS_INTERFACE_INFO_STC             nextHopInterface;
    GT_U32                              nextHopARPPointer;
    GT_U32                              nextHopTunnelPointer;
    GT_U32                              nextHopNatPointer;
    GT_U16                              nextHopVlanId1;
    GT_U32                              targetEpg;
}CPSS_DXCH_IP_UC_ROUTE_ENTRY_FORMAT_STC;

/*
 * Typedef: struct CPSS_DXCH_IP_UC_ECMP_RPF_FORMAT_STC
 *
 * Description: Unicast ECMP/QoS RPF check route entry format
 *              (APPLICABLE DEVICES: xCat3; AC5; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
 *
 * Fields:
 *      vlanArray     - array of vlan Ids that are compared against the
 *                      packet vlan as part of the unicast RPF check
 *
 */
typedef struct CPSS_DXCH_IP_UC_ECMP_RPF_FORMAT_STCT
{
    GT_U16      vlanArray[8];

}CPSS_DXCH_IP_UC_ECMP_RPF_FORMAT_STC;


/**
* @union CPSS_DXCH_IP_UC_ROUTE_ENTRY_UNT
 *
 * @brief Holds the two possible unicast route entry format.
 *
*/

typedef union{
    /** regular route entry format */
    CPSS_DXCH_IP_UC_ROUTE_ENTRY_FORMAT_STC regularEntry;

    /** @brief special route entry format for unicast ECMP/QOS RPF
     *  check (APPLICABLE DEVICES: xCat3; AC5; Bobcat2; Caelum;
     *  Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    CPSS_DXCH_IP_UC_ECMP_RPF_FORMAT_STC ecmpRpfCheck;

} CPSS_DXCH_IP_UC_ROUTE_ENTRY_UNT;


/**
* @enum CPSS_DXCH_IP_UC_ROUTE_ENTRY_TYPE_ENT
 *
 * @brief the type of uc route entry
*/
typedef enum{

    /** regular route entry format */
    CPSS_DXCH_IP_UC_ROUTE_ENTRY_E   = 0,

    /** @brief special route entry format for
     *  unicast ECMP/QOS RPF check
     *  (APPLICABLE DEVICES: xCat3; AC5)
     */
    CPSS_DXCH_IP_UC_ECMP_RPF_E      = 1

} CPSS_DXCH_IP_UC_ROUTE_ENTRY_TYPE_ENT;

/*
 * Typedef: struct CPSS_DXCH_IP_UC_ROUTE_ENTRY_STC
 *
 * Description: Unicast route entry
 *
 * Fields:
 *      type      - type of the route entry (refer to CPSS_DXCH_IP_UC_ROUTE_ENTRY_TYPE_ENT)
 *      entry     - the route entry information
 *
 */
typedef struct CPSS_DXCH_IP_UC_ROUTE_ENTRY_STCT
{
    CPSS_DXCH_IP_UC_ROUTE_ENTRY_TYPE_ENT    type;
    CPSS_DXCH_IP_UC_ROUTE_ENTRY_UNT         entry;

}CPSS_DXCH_IP_UC_ROUTE_ENTRY_STC;


/**
* @enum CPSS_DXCH_IP_MULTICAST_RPF_FAIL_COMMAND_MODE_ENT
 *
 * @brief decides on the way the RPF fail command is chosen when an RPF
 * fail occurs:
*/
typedef enum{

    /** Use the Multicast Route entry <RPF Fail Command>. */
    CPSS_DXCH_IP_MULTICAST_ROUTE_ENTRY_RPF_FAIL_COMMAND_MODE_E = 0,

    /** @brief Use the MLL entry <RPF Fail Command> in the MLL entry whose <VLAN>
     *  matches the packet VLAN. If none of the MLL entries have a <VLAN>
     *  that matches the packet VLAN, then use the Multicast Route entry
     *  <RPF Fail Command>.
     */
    CPSS_DXCH_IP_MULTICAST_MLL_RPF_FAIL_COMMAND_MODE_E = 1

} CPSS_DXCH_IP_MULTICAST_RPF_FAIL_COMMAND_MODE_ENT;

/**
* @enum CPSS_DXCH_IP_MULTICAST_INGRESS_VLAN_CHECK_ENT
 *
 * @brief Multicast Ingress VLAN check mode for packet
*/
typedef enum{

    /** @brief Multicast RPF check is performed by comparing the entry MC RPF eVLAN
     *  with the eVLAN assigned to the packet.
     */
    CPSS_DXCH_IP_MULTICAST_INGRESS_VLAN_CHECK_RPF_CHECK_E                = 0,

    /** @brief Bidirectional Tree check is performed by checking the state of the
     *  eVLAN assigned to the packet in the MC Routing Shared Tree.
     */
    CPSS_DXCH_IP_MULTICAST_INGRESS_VLAN_CHECK_BIDIRECTIONAL_TREE_CHECK_E = 1

} CPSS_DXCH_IP_MULTICAST_INGRESS_VLAN_CHECK_ENT;

/*
 * Typedef: struct CPSS_DXCH_IP_MC_ROUTE_ENTRY_STC
 *
 * Description:     Representation of the IP_MC_ROUTE_ENTRY in HW,
 *
 * Fields:
 *  cmd                - Route entry command. supported commands:
 *                          CPSS_PACKET_CMD_ROUTE_E,
 *                          CPSS_PACKET_CMD_ROUTE_AND_MIRROR_E,
 *                          CPSS_PACKET_CMD_TRAP_TO_CPU_E,
 *                          CPSS_PACKET_CMD_DROP_SOFT_E,
 *                          CPSS_PACKET_CMD_DROP_HARD_E ,
 *                          CPSS_PACKET_CMD_BRIDGE_AND_MIRROR_E,
 *                          CPSS_PACKET_CMD_BRIDGE_E
 *  cpuCodeIdx         - the cpu code index of the specific code when trap or
 *                       mirror.
 *  appSpecificCpuCodeEnable- Enable CPU code overwritting according to the
 *                        application specific CPU Code assignment mechanism.
 *  ttlHopLimitDecEnable - Enable TTL/Hop Limit Decrement
 *  ttlHopLimDecOptionsExtChkByPass- TTL/HopLimit Decrement and option/
 *                       Extention check bypass.
 *  ingressMirror      - mirror to ingress analyzer.
 *  ingressMirrorToAnalyzerIndex - One of the possible analyzers. Relevant
 *                       when ingressMirror is GT_TRUE.
 *                       If a previous engine in the pipe assigned a different
 *                       analyzer index, the higher index wins.
 *                       (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; AAS)
 *                       (APPLICABLE RANGES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman 0..6),
 *                       (APPLICABLE RANGES: AAS 0..62)
 *  qosProfileMarkingEnable- Enable Qos profile assignment.
 *  qosProfileIndex    - the qos profile to assign in case of
 *                       qosProfileMarkingEnable = GT_TRUE
 *  qosPrecedence      - whether this packet Qos parameters can be overridden
 *                       after this assigment.
 *  modifyUp           - whether to change the packets UP and how.
 *  modifyDscp         - whether to change the packets DSCP and how.
 *  countSet           - The counter set this route entry is linked to
 *  multicastRPFCheckEnable  - Enable the Multicast RPF check for this entry
 *  multicastIngressVlanCheck - Multicast Ingress VLAN check mode for the packet.
 *                       Relevant when multicastRPFCheckEnable is GT_TRUE.
 *                       (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
 *  multicastRPFVlan   - RPF check VLAN-ID . Relevant only if
 *                       multicastRPFCheckEnable = GT_TRUE.
 *  multicastRPFRoutingSharedTreeIndex - The shared tree number of this routing
 *                       entry. It is used when multicastIngressVlanCheck is set to
 *                       CPSS_DXCH_IP_MULTICAST_INGRESS_VLAN_CHECK_BIDIRECTIONAL_TREE_CHECK_E.
 *                       (APPLICABLE DEVICES: Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
 *  multicastRPFFailCommandMode - MC RPF fail command mode, which way the fail
 *                       command is chosen.
 *  RPFFailCommand     - the route entry RPF fail command, relvant only if
 *                       multicastRPFCheckEnable = GT_TRUE and
 *                       multicastRPFFailCommandMode =
 *                       CPSS_DXCH_IP_MULTICAST_ROUTE_ENTRY_RPF_FAIL_COMMAND_MODE_E.
 *                       supported commands:
 *                          CPSS_PACKET_CMD_TRAP_TO_CPU_E,
 *                          CPSS_PACKET_CMD_DROP_SOFT_E,
 *                          CPSS_PACKET_CMD_DROP_HARD_E ,
 *                          CPSS_PACKET_CMD_BRIDGE_AND_MIRROR_E,
 *                          CPSS_PACKET_CMD_BRIDGE_E
 *  scopeCheckingEnable- Enable IPv6 Scope Checking.
 *  siteId             - The site id of this route entry.
 *  mtuProfileIndex    - One of the eight global configurable MTU sizes (0..7).
 *  internalMLLPointer - for ipv4 it is the pointer of the first Mll entry
 *                       for ipv6 it is the pointer of the first internal Mll
 *                       entry.
 *  externalMLLPointer - relvant only for IPV6 , it is the pointer of the first
 *                       external Mll entry.
 *  targetEpg           - The target EPG(destination group-id) assigned to the packet
 *                        Relevant only if Next hop muxing = CPSS_DXCH_IP_NEXT_HOP_MUX_MODE_TRG_EPG_FOR_UC_E or
 *                        CPSS_DXCH_IP_NEXT_HOP_MUX_MODE_TRG_EPG_FOR_UC_MC_E
 *                        (APPLICABLE RANGES: 0..511)
 *                        (APPLICABLE DEVICES: AC5P; AC5X; Harrier; Ironman)
 */
typedef struct CPSS_DXCH_IP_MC_ROUTE_ENTRY_STCT
{
    CPSS_PACKET_CMD_ENT                 cmd;
    CPSS_DXCH_IP_CPU_CODE_INDEX_ENT     cpuCodeIdx;
    GT_BOOL                             appSpecificCpuCodeEnable;
    GT_BOOL                             ttlHopLimitDecEnable;
    GT_BOOL                             ttlHopLimDecOptionsExtChkByPass;
    GT_BOOL                             ingressMirror;
    GT_U32                              ingressMirrorToAnalyzerIndex;
    GT_BOOL                             qosProfileMarkingEnable;
    GT_U32                              qosProfileIndex;
    CPSS_PACKET_ATTRIBUTE_ASSIGN_PRECEDENCE_ENT qosPrecedence;
    CPSS_PACKET_ATTRIBUTE_MODIFY_TYPE_ENT       modifyUp;
    CPSS_PACKET_ATTRIBUTE_MODIFY_TYPE_ENT       modifyDscp;
    CPSS_IP_CNT_SET_ENT                 countSet;
    GT_BOOL                             multicastRPFCheckEnable;
    CPSS_DXCH_IP_MULTICAST_INGRESS_VLAN_CHECK_ENT multicastIngressVlanCheck;
    GT_U16                              multicastRPFVlan;
    GT_U8                               multicastRPFRoutingSharedTreeIndex;
    CPSS_DXCH_IP_MULTICAST_RPF_FAIL_COMMAND_MODE_ENT multicastRPFFailCommandMode;
    CPSS_PACKET_CMD_ENT                 RPFFailCommand;
    GT_BOOL                             scopeCheckingEnable;
    CPSS_IP_SITE_ID_ENT                 siteId;
    GT_U32                              mtuProfileIndex;
    GT_U32                              internalMLLPointer;
    GT_U32                              externalMLLPointer;
    GT_U32                              targetEpg;
}CPSS_DXCH_IP_MC_ROUTE_ENTRY_STC;

/*
 * Typedef: struct CPSS_DXCH_IP_MLL_STC
 *
 * Description:     Representation of the CPSS_DXCH_IP_MLL_STC in HW,
 *
 * Fields:
 *
 *  mllRPFFailCommand    - the MLL entry RPF fail command, supported commands:
 *                          CPSS_PACKET_CMD_TRAP_TO_CPU_E,
 *                          CPSS_PACKET_CMD_DROP_SOFT_E,
 *                          CPSS_PACKET_CMD_DROP_HARD_E ,
 *                          CPSS_PACKET_CMD_BRIDGE_AND_MIRROR_E,
 *                          CPSS_PACKET_CMD_BRIDGE_E
 *  isTunnelStart        - weather this nexthop is tunnel start enrty, in which
 *                         case the outInteface & mac are irrlevant and the
 *                         tunnel id is used.
 *  nextHopInterface     - the output interface this mll entry sends to. relevant
 *                         only if the isTunnelStart = GT_FALSE
 *  nextHopVlanId        - VLAN-ID used for the downstream next hop.
 *  nextHopTunnelPointer - the tunnel pointer in case this is a tunnel start
 *                         isTunnelStart = GT_TRUE
 *  tunnelStartPassengerType - the type of the tunneled, passenger packet.
 *                         (APPLICABLE DEVICES: xCat3; AC5)
 *  ttlHopLimitThreshold - minimal hop limit required for the packet to be
 *                         forwarded per this entry.
 *  excludeSrcVlan       - if GT_TRUE then if the packet's vlan equals the mll
 *                         entry vlan id then skip this entry.
 *  last                 - if GT_TRUE then there are more mll entries in this
 *                         list if GT_FALSE this is the end.
 */
typedef struct CPSS_DXCH_IP_MLL_STCT
{
    CPSS_PACKET_CMD_ENT     mllRPFFailCommand;
    GT_BOOL                 isTunnelStart;
    CPSS_INTERFACE_INFO_STC nextHopInterface;
    GT_U16                  nextHopVlanId;
    GT_U32                  nextHopTunnelPointer;
    CPSS_DXCH_TUNNEL_PASSANGER_TYPE_ENT tunnelStartPassengerType;
    GT_U16                  ttlHopLimitThreshold;
    GT_BOOL                 excludeSrcVlan;
    GT_BOOL                 last;
}CPSS_DXCH_IP_MLL_STC;

/*
 * Typedef: struct CPSS_DXCH_IP_MLL_PAIR_STC
 *
 * Description:     Representation of the IP muticast Link List pair in HW,
 *
 * Fields:
 *
 *  firstMllNode    - the first Mll of the pair
 *  secondMllNode   - the second Mll of the pair
 *  nextPointer     - pointer to the next MLL entry
 *
 */
typedef struct CPSS_DXCH_IP_MLL_PAIR_STCT
{
    CPSS_DXCH_IP_MLL_STC firstMllNode;
    CPSS_DXCH_IP_MLL_STC secondMllNode;

    GT_U16 nextPointer;

}CPSS_DXCH_IP_MLL_PAIR_STC;


/**
* @enum CPSS_DXCH_IP_MLL_PAIR_PART_LAST_BIT_WRITE_ENT
 *
 * @brief the possible parts to write the MLL entry last bit for.
*/
typedef enum{

    /** @brief writes the first MLL entry's
     *  last bit
     */
    CPSS_DXCH_IP_MLL_PAIR_WRITE_FIRST_LAST_BIT_E   = 0,

    /** @brief writes the second MLL entry's
     *  last bit
     */
    CPSS_DXCH_IP_MLL_PAIR_WRITE_SECOND_LAST_BIT_E  = 1

} CPSS_DXCH_IP_MLL_PAIR_PART_LAST_BIT_WRITE_ENT;


/* Old enumerators - remained for backward compatibility, unused in the code */
#define CPSS_DXCH_IP_MLL_PAIR_READ_WRITE_FIRST_MLL_ONLY_E                     CPSS_DXCH_PAIR_READ_WRITE_FIRST_ONLY_E
#define CPSS_DXCH_IP_MLL_PAIR_READ_WRITE_SECOND_MLL_NEXT_POINTER_ONLY_E       CPSS_DXCH_PAIR_READ_WRITE_SECOND_NEXT_POINTER_ONLY_E
#define CPSS_DXCH_IP_MLL_PAIR_READ_WRITE_WHOLE_E                              CPSS_DXCH_PAIR_READ_WRITE_WHOLE_E
#define CPSS_DXCH_IP_MLL_PAIR_READ_WRITE_FORM_ENT                             CPSS_DXCH_PAIR_READ_WRITE_FORM_ENT


/**
* @enum CPSS_DXCH_IP_BRG_EXCP_CMD_ENT
 *
 * @brief these are the type of bridge exceptions commands that it is
 * possible to enable routing for
*/
typedef enum{

    /** @brief for Bridged Traffic of IPv4 Unicast
     *  packets that arrive at the Router engine
     *  with a packet command of TRAP.
     */
    CPSS_DXCH_IP_BRG_UC_IPV4_TRAP_EXCP_CMD_E         = 0,

    /** @brief for Bridged Traffic of IPv6 Unicast
     *  packets that arrive at the Router engine
     *  with a packet command of TRAP.
     */
    CPSS_DXCH_IP_BRG_UC_IPV6_TRAP_EXCP_CMD_E         = 1,

    /** @brief for Bridged Traffic of IPv4 Unicast
     *  packets that arrive at the Router engine
     *  with a packet command of SOFT DROP.
     */
    CPSS_DXCH_IP_BRG_UC_IPV4_SOFT_DROP_EXCP_CMD_E    = 2,

    /** @brief for Bridged Traffic of IPv6 Unicast
     *  packets that arrive at the Router engine
     *  with a packet command of SOFT DROP
     */
    CPSS_DXCH_IP_BRG_UC_IPV6_SOFT_DROP_EXCP_CMD_E    = 3,

    /** @brief for Bridged Traffic of IPv4 Multicast
     *  packets that arrive at the Router engine
     *  with a packet command of TRAP
     */
    CPSS_DXCH_IP_BRG_MC_IPV4_TRAP_EXCP_CMD_E         = 4,

    /** @brief for Bridged Traffic of IPv6 Multicast
     *  packets that arrive at the Router engine
     *  with a packet command of TRAP
     */
    CPSS_DXCH_IP_BRG_MC_IPV6_TRAP_EXCP_CMD_E         = 5,

    /** @brief for Bridged Traffic of IPv4 Multicast
     *  packets that arrive at the Router engine
     *  with a packet command of SOFT DROP
     */
    CPSS_DXCH_IP_BRG_MC_IPV4_SOFT_DROP_EXCP_CMD_E    = 6,

    /** @brief for Bridged Traffic of IPv6 Multicast
     *  packets that arrive at the Router engine
     *  with a packet command of SOFT DROP
     */
    CPSS_DXCH_IP_BRG_MC_IPV6_SOFT_DROP_EXCP_CMD_E    = 7,

    /** @brief for bridged ARP packets that were assigned a
     *  command of TRAP prior to the Router engine
     */
    CPSS_DXCH_IP_BRG_ARP_TRAP_EXCP_CMD_E             = 8,

    /** @brief for bridged ARP packets that were
     *  assigned a command of SOFT DROP prior to
     *  the Router engine.
     */
    CPSS_DXCH_IP_BRG_ARP_SOFT_DROP_EXCP_CMD_E        = 9

} CPSS_DXCH_IP_BRG_EXCP_CMD_ENT;




/**
* @enum CPSS_DXCH_IP_EXCEPTION_TYPE_ENT
 *
 * @brief these are the type of exceptions that it is possible to set
 * a command for out of CPSS_DXCH_IP_ENTRY_CMD.
*/
typedef enum{

    /** @brief IP UC Header Errors.
     *  Possible commands:
     *  CPSS_PACKET_CMD_DROP_HARD_E,
     *  CPSS_PACKET_CMD_DROP_SOFT_E,
     *  CPSS_PACKET_CMD_TRAP_TO_CPU_E
     */
    CPSS_DXCH_IP_EXCP_UC_HDR_ERROR_E                 = 0,

    /** @brief IP MC Header Errors.
     *  Possible commands:
     *  CPSS_PACKET_CMD_DROP_HARD_E,
     *  CPSS_PACKET_CMD_DROP_SOFT_E,
     *  CPSS_PACKET_CMD_TRAP_TO_CPU_E,
     *  CPSS_PACKET_CMD_BRIDGE_AND_MIRROR_E,
     *  CPSS_PACKET_CMD_BRIDGE_E
     */
    CPSS_DXCH_IP_EXCP_MC_HDR_ERROR_E                 = 1,

    /** @brief IP UC Illegal Address.
     *  Possible commands:
     *  CPSS_PACKET_CMD_DROP_HARD_E,
     *  CPSS_PACKET_CMD_DROP_SOFT_E,
     *  CPSS_PACKET_CMD_TRAP_TO_CPU_E
     */
    CPSS_DXCH_IP_EXCP_UC_ILLEGAL_ADDRESS_E           = 2,

    /** @brief IP MC Illegal Address.
     *  Possible commands:
     *  CPSS_PACKET_CMD_DROP_HARD_E,
     *  CPSS_PACKET_CMD_DROP_SOFT_E,
     *  CPSS_PACKET_CMD_TRAP_TO_CPU_E,
     *  CPSS_PACKET_CMD_BRIDGE_AND_MIRROR_E,
     *  CPSS_PACKET_CMD_BRIDGE_E
     */
    CPSS_DXCH_IP_EXCP_MC_ILLEGAL_ADDRESS_E           = 3,

    /** @brief IP UC DIP to DA mismatch.
     *  Possible commands:
     *  CPSS_PACKET_CMD_DROP_HARD_E,
     *  CPSS_PACKET_CMD_DROP_SOFT_E,
     *  CPSS_PACKET_CMD_TRAP_TO_CPU_E
     */
    CPSS_DXCH_IP_EXCP_UC_DIP_DA_MISMATCH_E           = 4,

    /** @brief IP MC DIP to DA mismatch.
     *  Possible commands:
     *  CPSS_PACKET_CMD_DROP_HARD_E,
     *  CPSS_PACKET_CMD_DROP_SOFT_E,
     *  CPSS_PACKET_CMD_TRAP_TO_CPU_E,
     *  CPSS_PACKET_CMD_BRIDGE_AND_MIRROR_E,
     *  CPSS_PACKET_CMD_BRIDGE_E
     */
    CPSS_DXCH_IP_EXCP_MC_DIP_DA_MISMATCH_E           = 5,

    /** @brief IP UC MTU EXCEEDED exception.
     *  for IPv4, supported by Ch2 only
     *  Possible commands:
     *  CPSS_PACKET_CMD_DROP_HARD_E,
     *  CPSS_PACKET_CMD_DROP_SOFT_E,
     *  CPSS_PACKET_CMD_TRAP_TO_CPU_E
     */
    CPSS_DXCH_IP_EXCP_UC_MTU_EXCEEDED_E              = 6,

    /** @brief IP MC MTU EXCEEDED exception.
     *  for IPv4, supported by Ch2 only
     *  Possible commands:
     *  CPSS_PACKET_CMD_DROP_HARD_E,
     *  CPSS_PACKET_CMD_DROP_SOFT_E,
     *  CPSS_PACKET_CMD_TRAP_TO_CPU_E,
     *  CPSS_PACKET_CMD_BRIDGE_AND_MIRROR_E,
     *  CPSS_PACKET_CMD_BRIDGE_E
     */
    CPSS_DXCH_IP_EXCP_MC_MTU_EXCEEDED_E              = 7,

    /** @brief IP UC ALL ZERO exception.
     *  Possible commands:
     *  CPSS_PACKET_CMD_NONE_E,
     *  CPSS_PACKET_CMD_DROP_SOFT_E
     */
    CPSS_DXCH_IP_EXCP_UC_ALL_ZERO_SIP_E              = 8,

    /** @brief IP MC ALL ZERO exception.
     *  Possible commands:
     *  CPSS_PACKET_CMD_NONE_E,
     *  CPSS_PACKET_CMD_BRIDGE_E
     */
    CPSS_DXCH_IP_EXCP_MC_ALL_ZERO_SIP_E              = 9,

    /** @brief IP UC Option/HopByHop Exception.
     *  Possible commands:
     *  CPSS_PACKET_CMD_DROP_HARD_E,
     *  CPSS_PACKET_CMD_DROP_SOFT_E,
     *  CPSS_PACKET_CMD_TRAP_TO_CPU_E,
     *  CPSS_IP_CMD_MIRROR_AND_ROUTE_E,
     *  CPSS_PACKET_CMD_ROUTE_E
     */
    CPSS_DXCH_IP_EXCP_UC_OPTION_HOP_BY_HOP_E         = 10,

    /** @brief IP MC Option/HopByHop Exception
     *  Possible commands:
     *  CPSS_PACKET_CMD_DROP_HARD_E,
     *  CPSS_PACKET_CMD_DROP_SOFT_E,
     *  CPSS_PACKET_CMD_TRAP_TO_CPU_E,
     *  CPSS_IP_CMD_MIRROR_AND_ROUTE_E,
     *  CPSS_PACKET_CMD_ROUTE_E,
     *  CPSS_PACKET_CMD_BRIDGE_AND_MIRROR_E,
     *  CPSS_PACKET_CMD_BRIDGE_E
     */
    CPSS_DXCH_IP_EXCP_MC_OPTION_HOP_BY_HOP_E         = 11,

    /** @brief IP UC non HopByHop Extention
     *  Exception. This is for Ipv6 only.
     *  Possible commands:
     *  CPSS_PACKET_CMD_DROP_HARD_E,
     *  CPSS_PACKET_CMD_DROP_SOFT_E,
     *  CPSS_PACKET_CMD_TRAP_TO_CPU_E,
     *  CPSS_IP_CMD_MIRROR_AND_ROUTE_E,
     *  CPSS_PACKET_CMD_ROUTE_E
     */
    CPSS_DXCH_IP_EXCP_UC_NON_HOP_BY_HOP_EXT_E        = 12,

    /** @brief IP MC non HopByHop Extention
     *  Exception. This is for Ipv6 only.
     *  Possible commands:
     *  CPSS_PACKET_CMD_DROP_HARD_E,
     *  CPSS_PACKET_CMD_DROP_SOFT_E,
     *  CPSS_PACKET_CMD_TRAP_TO_CPU_E,
     *  CPSS_IP_CMD_MIRROR_AND_ROUTE_E,
     *  CPSS_PACKET_CMD_ROUTE_E,
     *  CPSS_PACKET_CMD_BRIDGE_AND_MIRROR_E,
     *  CPSS_PACKET_CMD_BRIDGE_E
     */
    CPSS_DXCH_IP_EXCP_MC_NON_HOP_BY_HOP_EXT_E        = 13,

    /** @brief IP UC TTL Exceeded error.
     *  Possible commands:
     *  CPSS_PACKET_CMD_DROP_HARD_E,
     *  CPSS_PACKET_CMD_DROP_SOFT_E,
     *  CPSS_PACKET_CMD_TRAP_TO_CPU_E
     */
    CPSS_DXCH_IP_EXCP_UC_TTL_EXCEED_E                = 14,

    /** @brief IP UC RPF FAIL.
     *  Possible commands:
     *  CPSS_PACKET_CMD_DROP_HARD_E,
     *  CPSS_PACKET_CMD_DROP_SOFT_E,
     *  CPSS_PACKET_CMD_TRAP_TO_CPU_E
     */
    CPSS_DXCH_IP_EXCP_UC_RPF_FAIL_E                  = 15,

    /** @brief IP UC SIP to SA match failure.
     *  Possible commands:
     *  CPSS_PACKET_CMD_DROP_HARD_E,
     *  CPSS_PACKET_CMD_DROP_SOFT_E,
     *  CPSS_PACKET_CMD_TRAP_TO_CPU_E
     */
    CPSS_DXCH_IP_EXCP_UC_SIP_SA_FAIL_E               = 16,

    /** @brief IP UC MTU EXCEEDED for non
     *  "don't fragment" (DF) packets exception
     *  supported only for Ch3 ipv4
     *  Possible commands:
     *  CPSS_PACKET_CMD_DROP_HARD_E,
     *  CPSS_PACKET_CMD_DROP_SOFT_E,
     *  CPSS_PACKET_CMD_TRAP_TO_CPU_E
     *  CPSS_PACKET_CMD_ROUTE_E,
     */
    CPSS_DXCH_IP_EXCP_UC_MTU_EXCEEDED_FOR_NON_DF_E   = 17,

    /** @brief IP MC MTU EXCEEDED for non "don't
     *  fragment" (DF) packets exception.
     *  supported only for Ch3 ipv4
     *  Possible commands:
     *  CPSS_PACKET_CMD_DROP_HARD_E,
     *  CPSS_PACKET_CMD_DROP_SOFT_E,
     *  CPSS_PACKET_CMD_TRAP_TO_CPU_E,
     *  CPSS_PACKET_CMD_BRIDGE_AND_MIRROR_E,
     *  CPSS_PACKET_CMD_BRIDGE_E
     *  CPSS_PACKET_CMD_ROUTE_E,
     */
    CPSS_DXCH_IP_EXCP_MC_MTU_EXCEEDED_FOR_NON_DF_E   = 18,

    /** @brief IP UC MTU EXCEEDED for "don't
     *  fragment" (DF) packets exception.
     *  supported only for Ch3 ipv4
     *  Possible commands:
     *  CPSS_PACKET_CMD_DROP_HARD_E,
     *  CPSS_PACKET_CMD_DROP_SOFT_E,
     *  CPSS_PACKET_CMD_TRAP_TO_CPU_E
     *  CPSS_PACKET_CMD_ROUTE_E (NOT APPLICABLE DEVICES: AC3/AC5),
     */
    CPSS_DXCH_IP_EXCP_UC_MTU_EXCEEDED_FOR_DF_E       = 19,

    /** @brief IP MC MTU EXCEEDED for "don't
     *  fragment" (DF) packets exception.
     *  supported only for Ch3 ipv4
     *  Possible commands:
     *  CPSS_PACKET_CMD_DROP_HARD_E,
     *  CPSS_PACKET_CMD_DROP_SOFT_E,
     *  CPSS_PACKET_CMD_TRAP_TO_CPU_E,
     *  CPSS_PACKET_CMD_BRIDGE_AND_MIRROR_E,
     *  CPSS_PACKET_CMD_BRIDGE_E
     *  CPSS_PACKET_CMD_ROUTE_E (NOT APPLICABLE DEVICES: AC3/AC5),
     */
    CPSS_DXCH_IP_EXCP_MC_MTU_EXCEEDED_FOR_DF_E       = 20,

    /** last element in enum */
    CPSS_DXCH_IP_EXCEPTION_TYPE_LAST_E               = 21

} CPSS_DXCH_IP_EXCEPTION_TYPE_ENT;

/**
* @enum CPSS_DXCH_IP_HEADER_ERROR_ENT
 *
 * @brief Types of IP header errors
*/
typedef enum{

    /** Incorrect checksum */
    CPSS_DXCH_IP_HEADER_ERROR_CHECKSUM_ENT = 0,

    /** Invalid version */
    CPSS_DXCH_IP_HEADER_ERROR_VERSION_ENT = 1,

    /** @brief For IPv4 packet:
     *  IPv4 header <total Length> + length of L2 header + 4 (CRC length) <=
     *  MAC layer packet byte count
     *  For IPv6 packet:
     *  IPv6 header <Payload Length> + 40 (IPv6 header length) + 4 (CRC length) <=
     *  MAC layer packet byte count
     */
    CPSS_DXCH_IP_HEADER_ERROR_LENGTH_ENT = 2,

    /** SIP and DIP addresses are equal */
    CPSS_DXCH_IP_HEADER_ERROR_SIP_DIP_ENT = 3

} CPSS_DXCH_IP_HEADER_ERROR_ENT;

/**
* @enum CPSS_DXCH_IP_MT_TC_QUEUE_SCHED_MODE_ENT
 *
 * @brief the IP Multi-Target TC queue scheduling mode
*/
typedef enum{

    /** @brief the IP Multi-Target TC queue
     *  will be part of the SDWRR scheduling.
     */
    CPSS_DXCH_IP_MT_TC_QUEUE_SDWRR_SCHED_MODE_E  = 0,

    /** @brief the IP Multi-Target TC queue will
     *  be part of the SP (strict priority) scheduling.
     */
    CPSS_DXCH_IP_MT_TC_QUEUE_SP_SCHED_MODE_E     = 1

} CPSS_DXCH_IP_MT_TC_QUEUE_SCHED_MODE_ENT;


/**
* @enum CPSS_DXCH_IP_BRG_SERVICE_ENT
 *
 * @brief the router bridge services
*/
typedef enum{

    /** @brief IP Header Check for Bridged
     *  Unicast IPv4/6 Packets
     */
    CPSS_DXCH_IP_HEADER_CHECK_BRG_SERVICE_E = 0,

    /** @brief Unicast RPF Check for Bridged
     *  IPv4/6 and ARP Packets
     */
    CPSS_DXCH_IP_UC_RPF_CHECK_BRG_SERVICE_E = 1,

    /** @brief SIP/SA Check for Bridged Unicast
     *  IPv4/6 and ARP Packets
     */
    CPSS_DXCH_IP_SIP_SA_CHECK_BRG_SERVICE_E = 2,

    /** @brief SIP Filtering for Bridged Unicast
     *  IPv4/6 and ARP Packets
     */
    CPSS_DXCH_IP_SIP_FILTER_BRG_SERVICE_E   = 3,

    /** last element in enum */
    CPSS_DXCH_IP_BRG_SERVICE_LAST_E         = 4

} CPSS_DXCH_IP_BRG_SERVICE_ENT;

/**
* @enum CPSS_DXCH_IP_BRG_SERVICE_ENABLE_DISABLE_MODE_ENT
 *
 * @brief the router bridge services enable/disable mode
*/
typedef enum{

    /** @brief enable/disable the service
     *  for ipv4 packets
     */
    CPSS_DXCH_IP_BRG_SERVICE_IPV4_ENABLE_DISABLE_E= 0,

    /** @brief enable/disable the service
     *  for ipv6 packets
     */
    CPSS_DXCH_IP_BRG_SERVICE_IPV6_ENABLE_DISABLE_E = 1,

    /** @brief enable/disable the service
     *  for arp packets
     */
    CPSS_DXCH_IP_BRG_SERVICE_ARP_ENABLE_DISABLE_E = 2

} CPSS_DXCH_IP_BRG_SERVICE_ENABLE_DISABLE_MODE_ENT;


/**
* @enum CPSS_DXCH_IP_MT_UC_SCHED_MTU_ENT
 *
 * @brief Multi target/unicast Scheduler MTUs
*/
typedef enum{

    /** 2k bytes MTU */
    CPSS_DXCH_IP_MT_UC_SCHED_MTU_2K_E   = 0,

    /** 8k bytes MTU */
    CPSS_DXCH_IP_MT_UC_SCHED_MTU_8K_E   = 1

} CPSS_DXCH_IP_MT_UC_SCHED_MTU_ENT;

/*
 * Typedef: struct CPSS_DXCH_IP_COUNTER_SET_STC
 *
 * Description: IPvX Counter Set
 *
 * Fields:
 *      inUcPkts                 - number of ingress unicast packets
 *      inMcPkts                 - number of ingress multicast packets
 *      inUcNonRoutedExcpPkts    - number of ingress Unicast packets that were
 *                                 not routed due to one or more exceptions.
 *      inUcNonRoutedNonExcpPkts - number of ingress Unicast packets that were
 *                                 not routed but didn't recive any excption.
 *      inMcNonRoutedExcpPkts    - number of ingress multicast packets that were
 *                                 not routed due to one or more exceptions.
 *      inMcNonRoutedNonExcpPkts - number of ingress multicast packets that were
 *                                 not routed but didn't recive any excption.
 *      inUcTrappedMirrorPkts    - number of ingress unicast packets that were
 *                                 trapped or mirrored by the router engine.
 *      inMcTrappedMirrorPkts    - number of ingress multicast packets that were
 *                                 trapped or mirrored by the router engine.
 *      mcRfpFailPkts            - number of multicast packets with fail RPF.
 *      outUcRoutedPkts          - the number of egress unicast routed packets.
 */
typedef struct CPSS_DXCH_IP_COUNTER_SET_STCT
{
    GT_U32 inUcPkts;
    GT_U32 inMcPkts;
    GT_U32 inUcNonRoutedExcpPkts;
    GT_U32 inUcNonRoutedNonExcpPkts;
    GT_U32 inMcNonRoutedExcpPkts;
    GT_U32 inMcNonRoutedNonExcpPkts;
    GT_U32 inUcTrappedMirrorPkts;
    GT_U32 inMcTrappedMirrorPkts;
    GT_U32 mcRfpFailPkts;
    GT_U32 outUcRoutedPkts;
}CPSS_DXCH_IP_COUNTER_SET_STC;

/**
* @enum CPSS_DXCH_IP_CNT_SET_MODE_ENT
 *
 * @brief the counter sets modes
*/
typedef enum{

    /** @brief Interface counter, the counter will
     *  count all packets bounded to an interface
     *  according to the counter set bounded
     *  inteface configuration.
     */
    CPSS_DXCH_IP_CNT_SET_INTERFACE_MODE_E    = 0,

    /** @brief Route Entry counter, the counter
     *  counts all packets which are binded to it
     *  by the next hops (UC & MC)
     */
    CPSS_DXCH_IP_CNT_SET_ROUTE_ENTRY_MODE_E      = 1

} CPSS_DXCH_IP_CNT_SET_MODE_ENT;

/**
* @enum CPSS_DXCH_IP_PORT_TRUNK_CNT_MODE_ENT
 *
 * @brief the counter Set Port/Trunk mode
*/
typedef enum{

    /** @brief This counter-set counts all
     *  packets regardless of their
     *  In / Out port or Trunk.
     */
    CPSS_DXCH_IP_DISREGARD_PORT_TRUNK_CNT_MODE_E = 0,

    /** @brief This counter-set counts packets Received /
     *  Transmitted via binded Port + Dev.
     */
    CPSS_DXCH_IP_PORT_CNT_MODE_E                 = 1,

    /** @brief This counter-set counts packets Received /
     *  Transmitted via binded Trunk.
     */
    CPSS_DXCH_IP_TRUNK_CNT_MODE_E                = 2

} CPSS_DXCH_IP_PORT_TRUNK_CNT_MODE_ENT;

/**
* @enum CPSS_DXCH_IP_VLAN_CNT_MODE_ENT
 *
 * @brief the counter Set vlan mode
*/
typedef enum{

    /** @brief This counter-set counts all packets
     *  regardless of their In / Out vlan.
     */
    CPSS_DXCH_IP_DISREGARD_VLAN_CNT_MODE_E = 0,

    /** @brief This counter-set counts packets Received
     *  / Transmitted via binded vlan.
     */
    CPSS_DXCH_IP_USE_VLAN_CNT_MODE_E       = 1

} CPSS_DXCH_IP_VLAN_CNT_MODE_ENT;


/**
* @struct CPSS_DXCH_IP_PORT_TRUNK_STC
*
*  @brief is port or Trunk
*
*/
typedef struct
{
    /** @brief relevant only if portTrunkCntMode =
     *         CPSS_DXCH_IP_PORT_CNT_MODE_E
     */
    GT_U32                      port;

    /** @brief relevant only if portTrunkCntMode =
     *         CPSS_DXCH_IP_TRUNK_CNT_MODE_E
     */
    GT_U16                      trunk;
}CPSS_DXCH_IP_PORT_TRUNK_STC;

/*
 * Typedef: struct CPSS_DXCH_IP_COUNTER_SET_INTERFACE_CFG_STC
 *
 * Description: IPvX Counter Set interface mode configuration
 *
 * Fields:
 *      portTrunkCntMode - the counter Set Port/Trunk mode
 *      ipMode           - the cnt set IP interface mode , which protocol stack
 *                         ipv4 ,ipv6 or both (which is actully disregarding the
 *                         ip protocol)
 *      vlanMode         - the counter Set vlan mode.
 *      hwDevNum         - the binded HW devNum , relevant only if
 *                         portTrunkCntMode = CPSS_DXCH_IP_PORT_CNT_MODE_E
 *      portTrunk        - the port or trunk
 *      vlanId           - the binded vlan , relevant only if
 *                         vlanMode = CPSS_DXCH_IP_USE_VLAN_CNT_MODE_E
 */
typedef struct CPSS_DXCH_IP_COUNTER_SET_INTERFACE_CFG_STCT
{
    CPSS_DXCH_IP_PORT_TRUNK_CNT_MODE_ENT portTrunkCntMode;
    CPSS_IP_PROTOCOL_STACK_ENT           ipMode;
    CPSS_DXCH_IP_VLAN_CNT_MODE_ENT       vlanMode;
    GT_HW_DEV_NUM                        hwDevNum;
    CPSS_DXCH_IP_PORT_TRUNK_STC          portTrunk;
    GT_U16                               vlanId;
}CPSS_DXCH_IP_COUNTER_SET_INTERFACE_CFG_STC;


/**
* @enum CPSS_DXCH_IP_DROP_CNT_MODE_ENT
 *
 * @brief the drop counter modes
*/
typedef enum{

    /** @brief Count all packets dropped by
     *  the Router.
     */
    CPSS_DXCH_IP_DROP_CNT_COUNT_ALL_MODE_E               = 0,

    /** @brief Count packets dropped due to
     *  IP Header Error.
     */
    CPSS_DXCH_IP_DROP_CNT_IP_HEADER_MODE_E               = 1,

    /** @brief Count packets dropped due to
     *  DIP/DA Mismatch Error.
     */
    CPSS_DXCH_IP_DROP_CNT_DIP_DA_MISMATCH_MODE_E         = 2,

    /** @brief Count packets dropped due to
     *  SIP/SA Mismatch Error.
     */
    CPSS_DXCH_IP_DROP_CNT_SIP_SA_MISMATCH_MODE_E         = 3,

    /** @brief Count packets dropped due to
     *  Illegal Address Error.
     */
    CPSS_DXCH_IP_DROP_CNT_ILLEGAL_ADDRESS_MODE_E         = 4,

    /** @brief Count packets dropped due to
     *  Unicast RPF Fail.
     */
    CPSS_DXCH_IP_DROP_CNT_UC_RPF_MODE_E                  = 5,

    /** @brief Count packets dropped due to
     *  Multicast RPF Fail.
     */
    CPSS_DXCH_IP_DROP_CNT_MC_RPF_MODE_E                  = 6,

    /** @brief Count packets dropped
     *  due to IPv4 TTL or IPv6 Hop
     *  Limit Exceeded.
     */
    CPSS_DXCH_IP_DROP_CNT_TTL_HOP_LIMIT_EXCEEDED_MODE_E  = 7,

    /** @brief Count packets dropped due to
     *  MTU Exceeded.
     */
    CPSS_DXCH_IP_DROP_CNT_MTU_EXCEEDED_MODE_E            = 8,

    /** @brief Count packets dropped due to
     *  IPv4 Options or IPv6
     *  Hop-by-Hop Option.
     */
    CPSS_DXCH_IP_DROP_CNT_OPTION_MODE_E                  = 9,

    /** @brief Count packets dropped due to
     *  IPv6 Scope Exception.
     */
    CPSS_DXCH_IP_DROP_CNT_IPV6_SCOPE_MODE_E              = 10,

    /** @brief Count packets dropped due to
     *  Unicast Packet SIP Filter.
     */
    CPSS_DXCH_IP_DROP_CNT_UC_SIP_FILTER_MODE_E           = 11,

    /** @brief Count packets dropped due to
     *  Packet Dropped due to next hop
     *  command SoftDrop or HardDrop.
     */
    CPSS_DXCH_IP_DROP_CNT_NH_CMD_MODE_E                  = 12,

    /** @brief Count packets dropped due to
     *  Access Matrix Filter.
     */
    CPSS_DXCH_IP_DROP_CNT_ACCESS_MATRIX_MODE_E           = 13,

    /** @brief Count packets dropped due to
     *  SIP all zeros Error.
     */
    CPSS_DXCH_IP_DROP_CNT_SIP_ALL_ZEROS_MODE_E           = 14

} CPSS_DXCH_IP_DROP_CNT_MODE_ENT;



/**
* @enum CPSS_DXCH_IP_URPF_MODE_ENT
 *
 * @brief Defines the uRPF check modes
*/
typedef enum{

    /** uRPF check is disabled */
    CPSS_DXCH_IP_URPF_DISABLE_MODE_E,

    /** @brief If ECMP uRPF is globally enabled,
     *  then uRPF check is done by comparing the
     *  packet VID to the VID in the additional
     *  route entry, otherwise it is done using
     *  the SIP-Next Hop Entry VID.
     *  Note: VLAN-based uRPF mode is not
     *  supported for ECMP blocks
     *  greater than 8 entries
     */
    CPSS_DXCH_IP_URPF_VLAN_MODE_E,

    /** @brief uRPF check is done by comparing the
     *  packet source (device,port)/Trunk to
     *  the SIP-Next Hop Entry (device,port)/Trunk.
     *  Note: Port-based uRPF mode is not
     *  supported if the address is
     *  associated with an ECMP/QoS
     *  block of nexthop entries.
     */
    CPSS_DXCH_IP_URPF_PORT_TRUNK_MODE_E,

    /** @brief uRPF check is done by checking
     *  the SIPNext Hop Entry Route
     *  Command. uRPF check fails if Route
     *  command is not "Route" or
     *  "Route and Mirror".
     *  Note: Loose-based uRPF mode is not
     *  supported if the address is
     *  associated with an ECMP/QoS
     *  block of nexthop entries.
     */
    CPSS_DXCH_IP_URPF_LOOSE_MODE_E

} CPSS_DXCH_IP_URPF_MODE_ENT;

/**
* @enum CPSS_DXCH_MULTI_TARGET_SHAPER_GRANULARITY_ENT
 *
 * @brief Enumeration of shaper granularity.
*/
typedef enum{

    /** Granularity is 64 core clock cycles. */
    CPSS_DXCH_MULTI_TARGET_SHAPER_GRANULARITY_64_CORE_CLOCKS_E,

    /** Granularity is 1024 core clock cycles. */
    CPSS_DXCH_MULTI_TARGET_SHAPER_GRANULARITY_1024_CORE_CLOCKS_E

} CPSS_DXCH_MULTI_TARGET_SHAPER_GRANULARITY_ENT;

/**
* @enum CPSS_DXCH_URPF_LOOSE_MODE_TYPE_ENT
 *
 * @brief This enum defines the loose mode type.
*/
typedef enum{

    /** @brief uRPF check fails if the SIP Next Hop <Packet Command> is HARD/SOFT DROP,
     *  TRAP, or DEFAULT_ROUTE_ENTRY (default)
     */
    CPSS_DXCH_URPF_LOOSE_MODE_TYPE_0_E = 0,

    /** @brief uRPF check fails if the SIP Next Hop <Packet Command> is HARD/SOFT DROP,
     *  or DEFAULT_ROUTE_ENTRY (default)
     */
    CPSS_DXCH_URPF_LOOSE_MODE_TYPE_1_E = 1

} CPSS_DXCH_URPF_LOOSE_MODE_TYPE_ENT;

/**
* @enum CPSS_DXCH_IP_NEXT_HOP_MUX_MODE_ENT
 *
 * @brief Global config for Next hop entry muxing modes
*/
typedef enum{

    /** @brief Next hop entry includes QoS fields
     */
    CPSS_DXCH_IP_NEXT_HOP_MUX_MODE_QOS_E               = 0,

    /** @brief Next hop entry includes VID1 assignment
     */
    CPSS_DXCH_IP_NEXT_HOP_MUX_MODE_VID1_E              = 1,

    /** @brief Next hop entry includes 
     * Target EPG assignment UC only.
     * (APPLICABLE DEVICES: AC5P; AC5X; Harrier; Ironman)
     */
    CPSS_DXCH_IP_NEXT_HOP_MUX_MODE_TRG_EPG_FOR_UC_E    = 2,

    /** @brief Next hop entry includes 
     *  Target EPG assignment UC & MC.
     * (APPLICABLE DEVICES: AC5P; AC5X; Harrier; Ironman)
     */
    CPSS_DXCH_IP_NEXT_HOP_MUX_MODE_TRG_EPG_FOR_UC_MC_E = 3
    
}CPSS_DXCH_IP_NEXT_HOP_MUX_MODE_ENT;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChIpTypesh */

