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
* @file tgfTrafficGenerator_forXml.h
*
* @brief -  set functions that the XML of cpss-api call will support
*           to allow python tests via Jason to use the traffic generator of the 'from cpu'
*           so add this file to not collide with already existing files of the traffic generator
*
* @version  1
********************************************************************************
*/
#ifndef __tgfTrafficGenerator_forXml_H
#define __tgfTrafficGenerator_forXml_H

#include <extUtils/trafficEngine/tgfTrafficGenerator.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @struct: TGF_PART_L2_STC
 *
 * @brief: struct for L2 part (mac addresses)
 */
typedef struct
{
    /** @brief: for proper CPSS-API CALL : not use TGF_MAC_ADDR */
    GT_ETHERADDR        daMac;
    /** @brief: for proper CPSS-API CALL : not use TGF_MAC_ADDR */
    GT_ETHERADDR        saMac;
} TGF_PART_L2_STC;

/**
 * @struct: TGF_PART_IPV4_STC
 *
 * @brief: struct for IPv4 part (header)
 */
typedef struct
{
    TGF_PROT_VER        version;
    GT_U8               headerLen;
    TGF_TYPE_OF_SERVICE typeOfService;
    GT_U16              totalLen;
    TGF_IPV4_ID_FIELD   id;
    TGF_FLAG            flags;
    GT_U16              offset;
    TGF_TTL             timeToLive;
    TGF_PROT            protocol;
    /** @brief: can use TGF_PACKET_AUTO_CALC_CHECKSUM_CNS for auto checksum */
    TGF_HEADER_CRC      csum;
    /** @brief: for proper CPSS-API CALL : not use TGF_IPV4_ADDR */
    GT_IPADDR           srcAddr;
    /** @brief: for proper CPSS-API CALL : not use TGF_IPV4_ADDR */
    GT_IPADDR           dstAddr;
} TGF_PART_IPV4_STC;

/**
 * @struct: TGF_PART_IPV6_STC
 *
 * @brief: struct for IPv6 part (header)
 */
typedef struct
{
    TGF_PROT_VER        version;
    TGF_TRAFFIC_CLASS   trafficClass;
    TGF_FLOW_LABEL      flowLabel;
    GT_U16              payloadLen;
    /** @brief: like IPv4 protocol */
    TGF_NEXT_HEADER     nextHeader;
    /** @brief: like IPv4 timeToLive (TTL) */
    TGF_HOP_LIMIT       hopLimit;
    /** @brief: for proper CPSS-API CALL : not use TGF_IPV6_ADDR */
    GT_IPV6ADDR         srcAddr;
    /** @brief: for proper CPSS-API CALL : not use TGF_IPV6_ADDR */
    GT_IPV6ADDR         dstAddr;
} TGF_PART_IPV6_STC;

/**
 * @struct: TGF_PART_ARP_STC
 *
 * @brief: struct for ARP part (header)
 */
typedef struct
{
    TGF_ARP_HW_TYPE     hwType;
    TGF_PROT_TYPE       protType;
    GT_U8               hwLen;
    GT_U8               protLen;
    GT_U16              opCode;
    /** @brief: for proper CPSS-API CALL : not use TGF_MAC_ADDR */
    GT_ETHERADDR        srcMac;
    /** @brief: for proper CPSS-API CALL : not use TGF_IPV4_ADDR */
    GT_IPADDR           srcIp;
    /** @brief: for proper CPSS-API CALL : not use TGF_MAC_ADDR */
    GT_ETHERADDR        dstMac;
    /** @brief: for proper CPSS-API CALL : not use TGF_IPV4_ADDR */
    GT_IPADDR           dstIp;
} TGF_PART_ARP_STC;

/**
 * @struct: TGF_PART____INTERNAL_USE____GENERIC_STC
 *
 * @brief: struct for generic part in the packet
 *         NOTE:  for internal use only !!!
 *                not to be used by the caller of 'cpss-api call'
 *
*/
typedef struct
{
    /** @brief: number of bytes in this generic part */
    GT_U32              dataLength;
    /** @brief: pointer to those bytes */
    GT_U8              *dataPtr;
} TGF_PART____INTERNAL_USE____GENERIC_STC;

/**
 * @struct: TGF_PACKET_PART_UNT
 *
 * @brief: union of parts that can be used to form a packet
 *         selected member according to enum : TGF_PACKET_PART_ENT
 *
*/
typedef union {
    TGF_PART_L2_STC                    l2Info;              /*TGF_PACKET_PART_L2_E,                               */
    TGF_PACKET_VLAN_TAG_STC            vlanTagInfo;         /*TGF_PACKET_PART_VLAN_TAG_E,                         */
    TGF_PACKET_SGT_TAG_STC             sgtTagInfo;          /*TGF_PACKET_PART_SGT_TAG_E,                          */
    TGF_PACKET_DSA_TAG_STC             dsaTagInfo;          /*TGF_PACKET_PART_DSA_TAG_E,                          */
    TGF_PACKET_ETHERTYPE_STC           ethernetInfo;        /*TGF_PACKET_PART_ETHERTYPE_E,                        */
    TGF_PACKET_MPLS_STC                mplsInfo;            /*TGF_PACKET_PART_MPLS_E,                             */
    TGF_PART_IPV4_STC                  ipv4Info;            /*TGF_PACKET_PART_IPV4_E,                             */
    TGF_PART_IPV6_STC                  ipv6Info;            /*TGF_PACKET_PART_IPV6_E,                             */
    TGF_PACKET_TCP_STC                 tcpInfo;             /*TGF_PACKET_PART_TCP_E,                              */
    TGF_PACKET_UDP_STC                 udpInfo;             /*TGF_PACKET_PART_UDP_E,                              */
    TGF_PART_ARP_STC                   arpInfo;             /*TGF_PACKET_PART_ARP_E,                              */
    TGF_PACKET_ICMP_STC                icmpInfo;            /*TGF_PACKET_PART_ICMP_E,                             */
    /* use tgfTrafficGeneratorXmlPacketPartGenericSet */    /*TGF_PACKET_PART_WILDCARD_E,                         */
    /* use tgfTrafficGeneratorXmlPacketPartGenericSet */    /*TGF_PACKET_PART_PAYLOAD_E,                          */
    TGF_PACKET_TRILL_STC               trillInfo;           /*TGF_PACKET_PART_TRILL_E,                            */
    TGF_PACKET_TRILL_FIRST_OPTION_STC  trillFirstOptionInfo;/*TGF_PACKET_PART_TRILL_FIRST_OPTION_E,               */
    TGF_PACKET_TRILL_GEN_OPTION_STC    trillGenOptionInfo;  /*TGF_PACKET_PART_TRILL_GENERAL_OPTION_E,             */
    TGF_PACKET_GRE_STC                 greInfo;             /*TGF_PACKET_PART_GRE_E,                              */
    /* use tgfTrafficGeneratorXmlPacketPartGenericSet */    /*TGF_PACKET_PART_TUNNEL_HEADER_FORCE_TILL_PASSENGER_E,*/
    /* use tgfTrafficGeneratorXmlPacketPartGenericSet */    /*TGF_PACKET_PART_CRC_E,                               */
    TGF_PACKET_PTP_V2_STC              ptpV2Info;           /*TGF_PACKET_PART_PTP_V2_E,                           */
    /* use tgfTrafficGeneratorXmlPacketPartGenericSet */    /*TGF_PACKET_PART_TEMPLATE_E,                         */
    TGF_PACKET_VXLAN_GPE_STC           vxlanGpeInfo;        /*TGF_PACKET_PART_VXLAN_GPE_E,                        */
    TGF_PACKET_NSH_STC                 nshInfo;             /*TGF_PACKET_PART_NSH_E,                              */
    TGF_PACKET_SRH_STC                 srhInfo;             /*TGF_PACKET_PART_SRH_E,                              */

    TGF_PART____INTERNAL_USE____GENERIC_STC     forInternalUseOnly_generic;/* for internal use of API tgfTrafficGeneratorXmlPacketPartGenericSet */
}TGF_PACKET_PART_UNT;

/**
* @internal tgfTrafficGeneratorXmlPacketPartSet function
* @endinternal
*
* @brief   set single specific part of the packet , per {devNum,portNum,packetIndex} ,
*          allow to set new/override part at index 'partIndex' .
*          so this function allow to build a stream for {devNum,portNum} that built
*          from several packet parts.
*
*          NOTE: for 'generic' part (generic string) see API:
*           tgfTrafficGeneratorXmlPacketPartGenericSet(...)
*            for parts than not have 'stucture' format ,like :
*            TGF_PACKET_PART_WILDCARD_E , TGF_PACKET_PART_PAYLOAD_E ,
*            TGF_PACKET_PART_TUNNEL_HEADER_FORCE_TILL_PASSENGER_E,
*            TGF_PACKET_PART_CRC_E , TGF_PACKET_PART_TEMPLATE_E
*
* @param[in] devNum                - the SW device number
* @param[in] portNum               - the port number
* @param[in] packetIndex           - the packet index (streamId)
* @param[in] partIndex             - the part index
* @param[in] partType              - the type of part that is to be selected
*                                    from the union of partInfoPtr
* @param[in] partInfoPtr           - (pointer to) the part info
*
* @retval GT_OK                    - on success
* @retval GT_BAD_STATE             - on internal bad state
* @retval GT_BAD_PTR               - on NULL pointer
*
*/
GT_STATUS tgfTrafficGeneratorXmlPacketPartSet
(
    IN GT_U32               devNum,
    IN GT_PORT_NUM          portNum,
    IN GT_U32               packetIndex,
    IN GT_U32               partIndex,
    IN TGF_PACKET_PART_ENT  partType,
    IN TGF_PACKET_PART_UNT  *partInfoPtr
);

/**
* @internal tgfTrafficGeneratorXmlPacketPartGenericSet function
* @endinternal
*
* @brief   set single specific part of the packet as 'generic' stream of bytes
*          this function is complimentary to tgfTrafficGeneratorXmlPacketPartSet(...)
*          to allow to form a packet (stream)
*
* @param[in] devNum                - the SW device number
* @param[in] portNum               - the port number
* @param[in] packetIndex           - the packet index (streamId)
* @param[in] partIndex             - the part index
* @param[in] stringOfHexNibbles    - the string that represent the bytes of the part of the packet.
*                                    1. all nibbles must be number of HEX (0..9,a..f,A..F) characters
*                                    2. number of nibbles must be even (2/4/6/8/10...)
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on stringOfHexNibbles if hold character that not HEX (0..9,a..f,A..F)
*                                    on stringOfHexNibbles if length is not even (2/4/6/8/10...)
* @retval GT_BAD_STATE             - on internal bad state
* @retval GT_BAD_PTR               - on NULL pointer
*
*/
GT_STATUS tgfTrafficGeneratorXmlPacketPartGenericSet
(
    IN GT_U32               devNum,
    IN GT_PORT_NUM          portNum,
    IN GT_U32               packetIndex,
    IN GT_U32               partIndex,
    IN GT_STRING            stringOfHexNibbles
);

/**
* @internal tgfTrafficGeneratorXmlPacketSizeSet function
* @endinternal
*
* @brief   set explicit packet size for of the packet , per {devNum,portNum,packetIndex} .
*          by default the TGF engine can calculate the packet size by it's own ,
*          from the 'packet parts' that are given for this packet.
*          but this function allow the test to be more flexible.
*          to restore the 'default' of auto-calc the length , set value of GT_NA
*          in the packetSize
*
* @param[in] devNum                - the SW device number
* @param[in] portNum               - the port number
* @param[in] packetIndex           - the packet index (streamId)
* @param[in] packetSize            - the packet size
*                                   value GT_NA means 'auto-calc' according to packet parts
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on packetSize >= 16K (when not GT_NA)
* @retval GT_NOT_FOUND             - when the {devNum,portNum,packetIndex} not defined yet
*                                   by tgfTrafficGeneratorXmlPacketPartSet or
*                                   by tgfTrafficGeneratorXmlPacketPartGenericSet
*
*/
GT_STATUS tgfTrafficGeneratorXmlPacketSizeSet
(
    IN GT_U32               devNum,
    IN GT_PORT_NUM          portNum,
    IN GT_U32               packetIndex,
    IN GT_U32               packetSize
);

/**
* @internal tgfTrafficGeneratorXmlPacketPartsDelete function
* @endinternal
*
* @brief   delete all the existing parts of packet that relate to the {devNum,portNum,packetIndex}
*          NOTE: portNum     == GT_NA meaning delete all streams (on all ports, all devices)
*          NOTE: packetIndex == GT_NA meaning delete all streams on {devNum,portNum}
*
* @param[in] devNum                - the SW device number
* @param[in] portNum               - the port number
* @param[in] packetIndex           - the packet index (streamId)
*
* @retval GT_OK                    - on success
*
*/
GT_STATUS tgfTrafficGeneratorXmlPacketPartsDelete
(
    IN GT_U32               devNum,
    IN GT_PORT_NUM          portNum,
    IN GT_U32               packetIndex
);

/**
* @internal tgfTrafficGeneratorXmlPacketVfdSet function
* @endinternal
*
* @brief   set single VFD (allow to override value(s) in the final packet)
*
* @param[in] devNum                - the SW device number
* @param[in] portNum               - the port number
* @param[in] vfdIndex              - the vfd index
* @param[in] vfdInfoPtr            - (pointer to) the VFD info
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PTR               - on NULL pointer
*/
GT_STATUS tgfTrafficGeneratorXmlPacketVfdSet
(
    IN GT_U32               devNum,
    IN GT_PORT_NUM          portNum,
    IN GT_U32               vfdIndex,
    IN TGF_VFD_INFO_STC     *vfdInfoPtr
);

/**
* @internal tgfTrafficGeneratorXmlPacketVfdsDelete function
* @endinternal            delete all the existing VFDs that relate to the port
*
* @brief   delete all the existing VFDs that relate to the {devNum,portNum}
*          NOTE: portNum == GT_NA meaning delete all streams on {devNum}
*
* @param[in] devNum                - the SW device number
* @param[in] portNum               - the port number
*
* @retval GT_OK                    - on success
*
*/
GT_STATUS tgfTrafficGeneratorXmlPacketVfdsDelete
(
    IN GT_U32               devNum,
    IN GT_PORT_NUM          portNum
);

/**
* @internal tgfTrafficGeneratorXmlPacketTransmit function
* @endinternal
*
* @brief   transmit packet from the CPU to the port , per {devNum,portNum,packetIndex}.
*          according to packet parts that set by :
*             tgfTrafficGeneratorXmlPacketPartSet
*             and by :
*             tgfTrafficGeneratorXmlPacketPartGenericSet
*          and according to VFDs that set by :
*             tgfTrafficGeneratorXmlPacketVfdSet
*
*         NOTE:
*         1. allow to transmit with chunks of burst:
*           after x sent frames --> do sleep of y millisecond
*           see parameters
*
* @param[in] devNum                - the SW device number
* @param[in] portNum               - the port number
* @param[in] packetIndex           - the packet index (streamId)
* @param[in] burstCount            - number of frames (non-zero value)
* @param[in] sleepAfterXCount      - do 'sleep' after X packets sent
*                                      when = 0 , meaning NO SLEEP needed during the burst
*                                      of 'burstCount'
* @param[in] sleepTime             - sleep time (in milliseconds) after X packets sent , see
*                                      parameter sleepAfterXCount
* @param[in] traceBurstCount       - number of packets in burst count that will be printed
* @param[in] loopbackEnabled       - when GT_TRUE  - the function set the port in PCS loopback
*                                    when GT_FALSE - the function set the port in 'force link UP'
*                                                    (by setting PCS loopback with 'PCS RX disable')
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_BAD_PARAM             - wrong interface
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note The packet will ingress to the device after 'loopback' and will act as
*       it was received from a traffic generator connected to the port
*
*/
GT_STATUS tgfTrafficGeneratorXmlPacketTransmit
(
    IN GT_U32               devNum,
    IN GT_PORT_NUM          portNum,
    IN GT_U32               packetIndex,
    IN GT_U32               burstCount,
    IN GT_U32               sleepAfterXCount,
    IN GT_U32               sleepTime,
    IN GT_U32               traceBurstCount,
    IN GT_BOOL              loopbackEnabled
);

/**
* @internal tgfPortMacSpecificCounterGet function
* @endinternal
*
* @brief   Get value of specific counter from the port mac counters.
*
* @param[in] devNum                - the SW device number
* @param[in] portNum               - the port number
* @param[in] cntrName              - specific counter name
*
* @param[out] cntrValuePtr         - (pointer to) current counter value.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_BAD_PARAM             - on wrong port number, device or counter name
* @retval GT_HW_ERROR              - on hardware error
* @retval GT_NOT_APPLICABLE_DEVICE - on not applicable device
*
*/
GT_STATUS tgfPortMacSpecificCounterGet
(
    IN  GT_U8                       devNum,
    IN  GT_U32                      portNum,
    IN  CPSS_PORT_MAC_COUNTERS_ENT  cntrName,
    OUT GT_U64                      *cntrValuePtr
);

/**
* @internal tgfTrafficGeneratorXmlRxInCpuGet function
* @endinternal
*
* @brief   Get entry from rxNetworkIf table (from table that got packets in the
*          CPU from any port , for any reason)
*
* @param[in] packetType            - the packet type to get
* @param[in] getFirst              - get first/next entry
* @param[in] trace                 - enable/disable packet tracing
*
* @param[out] packetNibblesPtr     - (pointer to) get the packet as a string on HEX nibbles.
*                                    NOTE: the caller need to copy the string for further use
*                                       and must not keep using it , as the string is in static memory
*                                       that will be used again for next call.
*                                    meaning that caller get the chance to know the length of packet and it's content
*                                    so it can allocate proper size for it (and copy it) , if more treatment is needed.
* @param[out] devNumPtr            - (pointer to)packet's device number
* @param[out] queuePtr             - (pointer to)Rx queue in which the packet was received.
* @param[out] isToCpuDsaPtr        - (pointer to)indication that the packet to to CPU with DSA {TO_CPU or FORWARD} or with other 'unexpected' format
* @param[out] commonDsaInfoPtr     - (pointer to)common DSA info that came with this packet
* @param[out] toCpuDsaInfoPtr      - (pointer to)TO_CPU DSA info that came with this packet
*
* @retval GT_OK                    - on success
* @retval GT_NO_MORE               - on more entries
* @retval GT_BAD_PTR               - on NULL pointer
*
* @note For 'captured' see API tgfTrafficGeneratorPortTxEthCaptureSet
*
*/
GT_STATUS tgfTrafficGeneratorXmlRxInCpuGet
(
    IN    TGF_PACKET_TYPE_ENT     packetType,
    IN    GT_BOOL                 getFirst,
    IN    GT_BOOL                 trace,
    OUT   GT_STRING               *packetNibblesPtr,
    OUT   GT_U8                   *devNumPtr,
    OUT   GT_U8                   *queuePtr,
    OUT   GT_BOOL                 *isToCpuDsaPtr,
    OUT   TGF_DSA_DSA_COMMON_STC  *commonDsaInfoPtr,
    OUT   TGF_DSA_DSA_TO_CPU_STC  *toCpuDsaInfoPtr
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __tgfTrafficGenerator_forXml_H */

