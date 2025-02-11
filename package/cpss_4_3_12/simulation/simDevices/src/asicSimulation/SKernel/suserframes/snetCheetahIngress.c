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
* @file snetCheetahIngress.c
*
* @brief This is a external API definition for snetChtIngress module of SKernel.
*
* @version   300
********************************************************************************
*/
#include <asicSimulation/SKernel/suserframes/snet.h>
#include <asicSimulation/SKernel/suserframes/snetCheetahIngress.h>
#include <asicSimulation/SKernel/smem/smemCheetah.h>
#include <asicSimulation/SKernel/skernel.h>
#include <asicSimulation/SKernel/cheetahCommon/sregCheetah.h>
#include <asicSimulation/SKernel/sfdb/sfdbCheetah.h>
#include <common/SHOST/GEN/INTR/EXP/INTR.H>
#include <gmSimulation/GM/GMApi.h>
#include <asicSimulation/SKernel/suserframes/snetCheetah3CentralizedCnt.h>
#include <asicSimulation/SKernel/suserframes/snetCheetahEgress.h>
#include <asicSimulation/SKernel/suserframes/snetFalconEgress.h>
#include <asicSimulation/SKernel/suserframes/snetXCat.h>
#include <asicSimulation/SKernel/suserframes/snetLion.h>
#include <asicSimulation/SKernel/suserframes/snetCheetah3.h>
#include <asicSimulation/SKernel/suserframes/snetCheetah2TStart.h>
#include <asicSimulation/SKernel/suserframes/snetCheetahPolicer.h>
#include <asicSimulation/SKernel/suserframes/snetCheetah3Policer.h>
#include <asicSimulation/SKernel/suserframes/snetCheetahEq.h>
#include <asicSimulation/SKernel/suserframes/snetCheetah3TTermination.h>
#include <asicSimulation/SLog/simLog.h>
#include <asicSimulation/SLog/simLogInfoTypePacket.h>
#include <asicSimulation/SLog/simLogInfoTypeDevice.h>
#include <asicSimulation/SKernel/suserframes/snetCheetahTxQ.h>
#include <asicSimulation/SKernel/suserframes/snetXCatPolicer.h>
#include <asicSimulation/SKernel/suserframes/snetLion2TrafficGenerator.h>
#include <asicSimulation/SKernel/smem/smemBobcat2.h>
#include <asicSimulation/SKernel/suserframes/snetCheetah2Routing.h>
#include <asicSimulation/SKernel/suserframes/snetPipe.h>
#include <common/Utils/Math/sMath.h>
#include <asicSimulation/SKernel/suserframes/snetFalconExactMatch.h>
#include <asicSimulation/SKernel/suserframes/snetHawkPpu.h>
#include <asicSimulation/SKernel/suserframes/snetGm.h>
#include <asicSimulation/SKernel/suserframes/snetLion2Oam.h>
#include <asicSimulation/SKernel/suserframes/snetAasGdma.h>
#include <asicSimulation/SKernel/suserframes/snetAasExactMatch.h>
#include <asicSimulation/SKernel/suserframes/snetAasHa.h>
#include <asicSimulation/SKernel/suserframes/snetAasIpe.h>
#include <asicSimulation/SKernel/suserframes/snetAasMll.h>

/* option for testing future fix ! */
void simulationOfFutureFix_TTI_remotePhysicalPort_origSrcPhyIsTrunk_set(GT_U32   newValue)
{
    GT_U32  devNum = 0;
    SKERNEL_DEVICE_OBJECT * devObjPtr = smemTestDeviceIdToDevPtrConvert(devNum);

    devObjPtr->errata.ttiTemotePhysicalPortOrigSrcPhyIsTrunk = newValue;
}

/* maximal number of local ports those may be connected to PHY */
#define SNET_CHT_MAX_PORT_TO_PHY_CNS  128
static GT_BOOL connectedPortToPhy[SNET_CHT_MAX_PORT_TO_PHY_CNS] = {GT_FALSE};
GT_STATUS wmSetPortConnectedToPhy(IN GT_U32 portNum, IN GT_BOOL connected)
{
    connectedPortToPhy[portNum] = connected;

    return GT_OK;
}

/* Maximum number of IPv6 extension headers that can be parsed */
#define IPV6_EXTENSIONS_MAX_SUPPORTED_CNS       4

#define TTI_PARSER_DEPTH(dev)   ((GT_U32)(SMEM_CHT_IS_SIP6_GET(dev) ? 160 : 128))

typedef struct{
    GT_U8       value;
    GT_CHAR*    name;
    GT_U32      lengthBytesUnit;/* some are 8 bytes granularity , some 4 bytes */
}IPV6_EXT_HEADERS_STC;
#define STANDARD_8_OCTETS   8
#define SPECIAL_4_OCTETS    4
#define NAME_AND_VALUE(name,lengthBytesUnit)    {name , #name,lengthBytesUnit}
static IPV6_EXT_HEADERS_STC ipv6ExtHeaders[] =
{
    NAME_AND_VALUE(SNET_CHT_IPV6_EXT_HDR_HOP_BY_HOP_CNS          ,STANDARD_8_OCTETS), /* Hop By Hop                     */
    NAME_AND_VALUE(SNET_CHT_IPV6_EXT_HDR_ROUTING_CNS             ,STANDARD_8_OCTETS), /* Routing                        */
    NAME_AND_VALUE(SNET_CHT_IPV6_EXT_HDR_FRAGMENT_CNS            ,STANDARD_8_OCTETS), /* Fragment                       */
    NAME_AND_VALUE(SNET_CHT_IPV6_EXT_HDR_ENCAP_SECUR_PAYLOAD_CNS ,STANDARD_8_OCTETS), /* Encapsulating Security Payload */
    NAME_AND_VALUE(SNET_CHT_IPV6_EXT_HDR_AUTENTICATION_CNS       ,SPECIAL_4_OCTETS ), /* Authentication                 */
    NAME_AND_VALUE(SNET_CHT_IPV6_EXT_HDR_DESTINATION_OPTIONS_CNS ,STANDARD_8_OCTETS), /* Destination Options            */
    NAME_AND_VALUE(SNET_CHT_IPV6_EXT_HDR_MOBILITY_HEADER_CNS     ,STANDARD_8_OCTETS), /* Mobility Header                */
};

static GT_U32 ipv6ExtHeadersAmount =
    sizeof(ipv6ExtHeaders) / sizeof(ipv6ExtHeaders[0]);

/* additional in sip 6 */
static IPV6_EXT_HEADERS_STC sip6_ipv6ExtHeaders[] =
{
    NAME_AND_VALUE(SNET_CHT_IPV6_EXT_HDR_HOST_IDENTIFY_PRTOCOL   ,STANDARD_8_OCTETS),
    NAME_AND_VALUE(SNET_CHT_IPV6_EXT_HDR_SHIM6_PRTOCOL           ,STANDARD_8_OCTETS),
    NAME_AND_VALUE(SNET_CHT_IPV6_EXT_HDR_EXPERIMENTAL_1          ,STANDARD_8_OCTETS),
    NAME_AND_VALUE(SNET_CHT_IPV6_EXT_HDR_EXPERIMENTAL_2          ,STANDARD_8_OCTETS),
};

static GT_U32 sip6_ipv6ExtHeadersAmount =
    sizeof(sip6_ipv6ExtHeaders) / sizeof(sip6_ipv6ExtHeaders[0]);

#define CPY_DESC_FIELD_MAC(targetDescrPtr,sourceDescrPtr,field)   \
    {                                                             \
        if(targetDescrPtr->field != sourceDescrPtr->field)        \
        {                                                         \
            __LOG_NO_LOCATION_META_DATA(("\t\t %s = [0x%x] \n",#field , sourceDescrPtr->field));\
        }                                                         \
        targetDescrPtr->field = sourceDescrPtr->field;            \
    }

#define CMD_FORWARD   SKERNEL_EXT_PKT_CMD_FORWARD_E
#define CMD_MIRROR    SKERNEL_EXT_PKT_CMD_MIRROR_TO_CPU_E
#define CMD_TRAP      SKERNEL_EXT_PKT_CMD_TRAP_TO_CPU_E
#define CMD_SOFT_DROP SKERNEL_EXT_PKT_CMD_SOFT_DROP_E
#define CMD_HARD_DROP SKERNEL_EXT_PKT_CMD_HARD_DROP_E

/* Number of Transmit Queues in Cheetah */
#define SNET_CHT_SDMA_TX_QUEUE_MAX_NUMBER       8

/* CHT3 OAM PROTOCOL */
#define SNET_CHT3_ETHERTYPE_OAM_SLOW_PROTOCOL       0x8809

/* Flow Control Packet Ethertype */
#define SNET_CHT_FC_ETHERTYPE                       0x8808

/* Flow Control Packet OpCode */
#define SNET_CHT_FC_OPCODE                          0x0001

/* Priority Flow Control Packet OpCode */
#define SNET_CHT_PFC_OPCODE                         0x0101

/* double indexed : with one of
SKERNEL_EXT_PKT_CMD_FORWARD_E
SKERNEL_EXT_PKT_CMD_MIRROR_TO_CPU_E
SKERNEL_EXT_PKT_CMD_TRAP_TO_CPU_E
SKERNEL_EXT_PKT_CMD_HARD_DROP_E
SKERNEL_EXT_PKT_CMD_SOFT_DROP_E
*/
static SKERNEL_EXT_PACKET_CMD_ENT chtResovedPacketCmdTable[5][5]=
{
    {CMD_FORWARD,   CMD_MIRROR,    CMD_TRAP,      CMD_HARD_DROP, CMD_SOFT_DROP},
    {CMD_MIRROR,    CMD_MIRROR,    CMD_TRAP,      CMD_HARD_DROP, CMD_TRAP},
    {CMD_TRAP,      CMD_TRAP,      CMD_TRAP,      CMD_HARD_DROP, CMD_TRAP},
    {CMD_HARD_DROP, CMD_HARD_DROP, CMD_HARD_DROP, CMD_HARD_DROP, CMD_HARD_DROP},
    {CMD_SOFT_DROP, CMD_TRAP,      CMD_TRAP,      CMD_HARD_DROP, CMD_SOFT_DROP}
};

/* VLAN mode */
typedef enum {
    SNET_NOT_CHANGE_E = 0,
    SNET_UNTAGGED_ONLY_E,
    SNET_TAGGED_ONLY_E,
    SNET_ALL_PACKETS_E
} SNET_VLAN_MODE_ENT;

/* QoS Trust Mode */
#define SNET_TRUST_L2_E         1
#define SNET_TRUST_L3_E         2

/* IPV6 Trunk Hushing Mode */
typedef enum {
    SNET_IPV6_TRUNK_HASH0_E = 0,
    SNET_IPV6_TRUNK_HASH1_E,
    SNET_IPV6_TRUNK_HASH2_E,
    SNET_IPV6_TRUNK_HASH3_E
} SNET_IPV6_TRUNK_LBH_ENT;

/* Classification of IEEE802.3x Flow Control Packets:
     SNET_NOT_FC_PACKET_E       -   Not FC packet
     SNET_FC_PACKET_E           -   Valid FC packet
     SNET_PFC_PACKET_E          -   Priority Flow control frame
     SNET_UNKNOWN_FC_PACKET_E   -   Unknown MAC control frame */
typedef enum {
    SNET_NOT_FC_PACKET_E = 0,
    SNET_FC_PACKET_E,
    SNET_PFC_PACKET_E,
    SNET_UNKNOWN_FC_PACKET_E
} SNET_FLOW_CONTROL_PACKET_TYPE_ENT;

/* USX unit */
#define USX_PORT_EXT(dev,_macPort)\
    SMEM_CHT_MAC_REG_DB_SIP5_GET(dev)->sip6_MTI[_macPort].USX_MAC_PORT
#define USX_PORT_GLOBAL(dev,_representativePort)\
    SMEM_CHT_MAC_REG_DB_SIP5_GET(dev)->sip6_MTI_EXTERNAL[_representativePort].USX_GLOBAL
#define MTI_PORT_EXT(dev,_macPort)\
    SMEM_CHT_MAC_REG_DB_SIP5_GET(dev)->sip6_MTI[_macPort].MTI_EXT_PORT
#define MTI_PCS_PORT_EXT(dev,_macPort)\
    SMEM_CHT_MAC_REG_DB_SIP5_GET(dev)->sip6_MTI[_macPort].MTI_PCS_EXT_PORT
#define MTI_CPU_PORT_EXT(dev,_macPort)\
    SMEM_CHT_MAC_REG_DB_SIP5_GET(dev)->sip6_MTI_cpu_port[_macPort].MTI_EXT_PORT


/*#define SNET_LION3_PCH_TOD_CLOCK_GET_MAC(dev, group, inst) \
    SIM_OS_MAC(simOsTickGet)() - (dev)->eTodTimeStampClockDiff[(group)][(inst)]*/

/* force reduced number of bits from the eDSA (like actual device !!! - not documented behavior) */
#define REDUCE_NUM_BIT_FROM_EDSA_MAC(dev,field,type)                            \
        {                                                                       \
            GT_U32  __origValue = field;                                        \
            field &= SMEM_BIT_MASK(dev->flexFieldNumBitsSupport.type);          \
            if(field != __origValue)                                            \
            {                                                                   \
                __LOG(("NOTE: force reduced number of bits from the eDSA : for field[%s] from[0x%x] to [0x%x] (type [%s] support max [%d] bits) \n" ,\
                    #field , __origValue , field,                               \
                    #type, dev->flexFieldNumBitsSupport.type));                 \
            }                                                                   \
        }

#define SMEM_LION3_TTI_PHYSICAL_PORT_TABLE_FIELDS_NAME                                     \
     STR(   SMEM_LION3_TTI_PHYSICAL_PORT_TABLE_FIELDS_PORT_DEFAULT_SOURCE_EPORT_NUMBER       )\
    ,STR(   SMEM_LION3_TTI_PHYSICAL_PORT_TABLE_FIELDS_PORT_IS_RING_CORE_PORT                 )\
    ,STR(   SMEM_LION3_TTI_PHYSICAL_PORT_TABLE_FIELDS_PORT_IS_LOOPED                         )\
    ,STR(   SMEM_LION3_TTI_PHYSICAL_PORT_TABLE_FIELDS_ENABLE_MRU_CHECK_ON_CASCADE_PORT       )\
    ,STR(   SMEM_LION3_TTI_PHYSICAL_PORT_TABLE_FIELDS_CC_LINE_CARD_PORTS_DEFAULT_EPORT_BASE  )\
    ,STR(   SMEM_LION3_TTI_PHYSICAL_PORT_TABLE_FIELDS_CC_LINE_CARD_TRUNK_DEFAULT_EPORT_BASE  )\
    ,STR(   SMEM_LION3_TTI_PHYSICAL_PORT_TABLE_FIELDS_CENTRALIZED_CHASSIS_PORT_ENABLE        )\
    ,STR(   SMEM_LION3_TTI_PHYSICAL_PORT_TABLE_FIELDS_TRUNK_ID                               )\
    ,STR(   SMEM_LION3_TTI_PHYSICAL_PORT_TABLE_FIELDS_PORT_QOS_MODE                          )

char * lion3TtiPhysicalPortFieldsTableNames[SMEM_LION3_TTI_PHYSICAL_PORT_TABLE_FIELDS___LAST_VALUE___E] =
    {SMEM_LION3_TTI_PHYSICAL_PORT_TABLE_FIELDS_NAME};

SNET_ENTRY_FORMAT_TABLE_STC lion3TtiPhysicalPortTableFieldsFormat[SMEM_LION3_TTI_PHYSICAL_PORT_TABLE_FIELDS___LAST_VALUE___E] =
{
/*    SMEM_LION3_TTI_PHYSICAL_PORT_TABLE_FIELDS_PORT_DEFAULT_SOURCE_EPORT_NUMBER            */
    STANDARD_FIELD_MAC(13),
/*    ,SMEM_LION3_TTI_PHYSICAL_PORT_TABLE_FIELDS_PORT_IS_RING_CORE_PORT                     */
    STANDARD_FIELD_MAC(1),
/*    ,SMEM_LION3_TTI_PHYSICAL_PORT_TABLE_FIELDS_PORT_IS_LOOPED                              */
    STANDARD_FIELD_MAC(1),
/*    ,SMEM_LION3_TTI_PHYSICAL_PORT_TABLE_FIELDS_ENABLE_MRU_CHECK_ON_CASCADE_PORT            */
    STANDARD_FIELD_MAC(1),
/*    ,SMEM_LION3_TTI_PHYSICAL_PORT_TABLE_FIELDS_CC_LINE_CARD_PORTS_DEFAULT_EPORT_BASE       */
    STANDARD_FIELD_MAC(13),
/*    ,SMEM_LION3_TTI_PHYSICAL_PORT_TABLE_FIELDS_CC_LINE_CARD_TRUNK_DEFAULT_EPORT_BASE       */
    STANDARD_FIELD_MAC(13),
/*    ,SMEM_LION3_TTI_PHYSICAL_PORT_TABLE_FIELDS_CENTRALIZED_CHASSIS_PORT_ENABLE             */
    STANDARD_FIELD_MAC(1),
/*    ,SMEM_LION3_TTI_PHYSICAL_PORT_TABLE_FIELDS_TRUNK_ID                                    */
    STANDARD_FIELD_MAC(12),
/*    ,SMEM_LION3_TTI_PHYSICAL_PORT_TABLE_FIELDS_PORT_QOS_MODE                               */
    STANDARD_FIELD_MAC(1)
};

#define SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_NAME                                    \
     STR( SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_PROT_BASED_QOS_EN                    )\
    ,STR( SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_PROT_BASED_VLAN_EN                   )\
    ,STR( SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_DEF_TAG1_VLAN_ID                     )\
    ,STR( SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_PORT_UP0                             )\
    ,STR( SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_INGRESS_TAG0_TPID_PROFILE            )\
    ,STR( SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_INGRESS_TAG1_TPID_PROFILE            )\
    ,STR( SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_VLAN_TRANSLATION                     )\
    ,STR( SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_P_VID_PRECEDENCE                     )\
    ,STR( SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_P_EVLAN_MODE                         )\
    ,STR( SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_P_EVLAN                              )\
    ,STR( SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_TTI_802_1AH_MAC_TO_ME_EN             )\
    ,STR( SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_EN_802_1AH_TTI_LOOKUP                )\
    ,STR( SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_ETHERNET_CLASSIFIER_EN               )\
    ,STR( SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_MPLS_TUNNEL_TERMINATION_EN           )\
    ,STR( SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_TTI_MPLS_MAC_TO_ME_EN                )\
    ,STR( SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_TTI_IPV4_MAC_TO_ME_EN                )\
    ,STR( SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_IPV4_TTI_FOR_TT_ONLY                 )\
    ,STR( SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_IPV4_TUNNEL_TERMINATION_EN           )\
    ,STR( SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_EXTENDED_DSA_BYPASS_BRIDGE           )\
    ,STR( SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_PORT_QOS_PROFILE                     )\
    ,STR( SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_TRUST_DSA_TAG_QOS                    )\
    ,STR( SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_TRUST_EXP                            )\
    ,STR( SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_TRUST_DSCP                           )\
    ,STR( SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_TRUST_UP                             )\
    ,STR( SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_MAP_DSCP_TO_DSCP                     )\
    ,STR( SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_TRUST_QOS_MAPPING_TABLE_INDEX        )\
    ,STR( SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_PORT_MODIFY_DSCP                     )\
    ,STR( SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_PORT_MODIFY_UP                       )\
    ,STR( SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_PORT_QOS_PRECEDENCE                  )\
    ,STR( SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_NESTED_VLAN_ACCESS_PORT              )\
    ,STR( SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_OVERSIZE_UNTAGGED_PKTS_FILTER_EN     )\
    ,STR( SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_TRILL_ENGINE_EN                      )\
    ,STR( SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_TRILL_INGRESS_OUTER_VID0             )\
    ,STR( SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_TTI_802_1AH_PASSENGER_STAG_IS_TAG0_1 )\
    ,STR( SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_IPV4_6_TOTAL_LENGTH_DEDUCTION_EN     )\
    ,STR( SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_OAM_LINK_LAYER_PDU_TRAP_EN           )\
    ,STR( SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_OAM_LINK_LAYER_LOOPBACK_EN           )\
    ,STR( SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_UP_CFI_TO_QOS_TABLE_SELECT_MODE      )\
    ,STR( SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_LOOKUP0_PCL_CFG_MODE                 )\
    ,STR( SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_LOOKUP1_PCL_CFG_MODE                 )\
    ,STR( SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_LOOKUP2_PCL_CFG_MODE                 )\
    ,STR( SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_ASSIGN_VF_ID_EN                      )\
    ,STR( SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_TRUST_L2_QOS_TAG0_OR_TAG1            )\
    ,STR( SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_TTI_PKT_TYPE_UDB_KEY_I_ENABLE        )\
    ,STR( SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_IPV4_MC_DUPLICATION_MODE             )\
    ,STR( SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_IPV6_MC_DUPLICATION_MODE             )\
    ,STR( SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_MPLS_MC_DUPLICATION_ENABLE           )\
    ,STR( SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_TRILL_MC_DUPLICATION_ENABLE          )\
    ,STR( SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_PBB_MC_DUPLICATION_ENABLE            )\
    ,STR( SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_MRU_INDEX                            )\
    ,STR( SMEM_SIP6_30_TTI_DEFAULT_EPORT_TABLE_FIELDS_SOURCE_EPG_E                        )\
    ,STR( SMEM_SIP6_30_TTI_DEFAULT_EPORT_TABLE_FIELDS_COPY_TAG1_VID_TO_SRC_EPG_E          )\
    ,STR( SMEM_SIP6_30_TTI_DEFAULT_EPORT_TABLE_FIELDS_EPORT_UP1_E                         )\
    ,STR( SMEM_SIP6_30_TTI_DEFAULT_EPORT_TABLE_FIELDS_EPORT_DEI1_E                        )\
    ,STR( SMEM_SIP6_30_TTI_DEFAULT_EPORT_TABLE_FIELDS_EPORT_DEI0_E                        )\
    ,STR( SMEM_SIP6_30_TTI_DEFAULT_EPORT_TABLE_FIELDS_PRP_ENABLED_E                       )\
    ,STR( SMEM_SIP6_30_TTI_DEFAULT_EPORT_TABLE_FIELDS_PRT_PORT_LAN_ID_E                   )\
    ,STR( SMEM_SIP6_30_TTI_DEFAULT_EPORT_TABLE_FIELDS_PRT_TREAT_WRONG_LAN_ID_AS_RCT_EXISTS_E )\
    ,STR( SMEM_SIP6_30_TTI_DEFAULT_EPORT_TABLE_FIELDS_LSDU_CONSTANT_E                     )\
    ,STR( SMEM_SIP7_TTI_DEFAULT_EPORT_TABLE_FIELDS_EVLAN_EXTENSION_E                      )\
    ,STR( SMEM_SIP7_TTI_DEFAULT_EPORT_TABLE_FIELDS_GENERIC_RANGE_SOURCE_PORT_PROFILE_E    )\
    ,STR( SMEM_SIP7_TTI_DEFAULT_EPORT_TABLE_FIELDS_SERIAL_EM_PORT_PROFILE_ID_E            )\
    ,STR( SMEM_SIP7_TTI_DEFAULT_EPORT_TABLE_FIELDS_GENERIC_CLASSIFICATION_FIELD_DEFAULT_VALUE_E )\
    ,STR( SMEM_SIP7_TTI_DEFAULT_EPORT_TABLE_FIELDS_SERIAL_EM1_LOOKUP_KEY_MODE_E           )\
    ,STR( SMEM_SIP7_TTI_DEFAULT_EPORT_TABLE_FIELDS_SERIAL_EM2_LOOKUP_KEY_MODE_E           )\
    ,STR( SMEM_SIP7_TTI_DEFAULT_E_PORT_TABLE_FIELDS_MIDWAY_PCL_CFG_MODE_E                 )\
    ,STR( SMEM_SIP7_TTI_DEFAULT_E_PORT_TABLE_FIELDS_EXCESSIVE_L2_TAG_EXCEPTION_EN_E       )\
    ,STR( SMEM_SIP7_TTI_DEFAULT_E_PORT_TABLE_FIELDS_INGRESS_CASCADE_DBL_EN_E              )\
    ,STR( SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_TRUST_QOS_MAPPING_TABLE_INDEX_EXTENSION_E)\


char * lion3TtiDefaultEPortFieldsTableNames[SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS___LAST_VALUE___E] =
    {SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_NAME};

SNET_ENTRY_FORMAT_TABLE_STC lion3TtiDefaultEPortTableFieldsFormat[SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS___LAST_VALUE___E] =
{
/* SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_PROT_BASED_QOS_EN                             */
   STANDARD_FIELD_MAC(1),
/* SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_PROT_BASED_VLAN_EN                            */
   STANDARD_FIELD_MAC(1),
/* SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_DEF_TAG1_VLAN_ID                              */
   STANDARD_FIELD_MAC(12),
/* SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_PORT_UP0                                      */
   STANDARD_FIELD_MAC(3),
/* SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_INGRESS_TAG0_TPID_PROFILE                     */
   STANDARD_FIELD_MAC(3),
/* SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_INGRESS_TAG1_TPID_PROFILE                     */
   STANDARD_FIELD_MAC(3),
/* SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_VLAN_TRANSLATION                              */
   STANDARD_FIELD_MAC(1),
/* SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_P_VID_PRECEDENCE                              */
   STANDARD_FIELD_MAC(1),
/* SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_P_EVLAN_MODE                                  */
   STANDARD_FIELD_MAC(1),
/* SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_P_EVLAN                                       */
   STANDARD_FIELD_MAC(13),
/* SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_TTI_802_1AH_MAC_TO_ME_EN                      */
   STANDARD_FIELD_MAC(1),
/* SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_EN_802_1AH_TTI_LOOKUP                         */
   STANDARD_FIELD_MAC(1),
/* SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_ETHERNET_CLASSIFIER_EN                        */
   STANDARD_FIELD_MAC(1),
/* SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_MPLS_TUNNEL_TERMINATION_EN                    */
   STANDARD_FIELD_MAC(1),
/* SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_TTI_MPLS_MAC_TO_ME_EN                         */
   STANDARD_FIELD_MAC(1),
/* SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_TTI_IPV4_MAC_TO_ME_EN                         */
   STANDARD_FIELD_MAC(1),
/* SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_IPV4_TTI_FOR_TT_ONLY                          */
   STANDARD_FIELD_MAC(1),
/* SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_IPV4_TUNNEL_TERMINATION_EN                    */
   STANDARD_FIELD_MAC(1),
/* SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_EXTENDED_DSA_BYPASS_BRIDGE                    */
   STANDARD_FIELD_MAC(1),
/* SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_PORT_QOS_PROFILE                              */
   STANDARD_FIELD_MAC(10),
/* SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_TRUST_DSA_TAG_QOS                             */
   STANDARD_FIELD_MAC(1),
/* SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_TRUST_EXP                                     */
   STANDARD_FIELD_MAC(1),
/* SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_TRUST_DSCP                                    */
   STANDARD_FIELD_MAC(1),
/* SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_TRUST_UP                                      */
   STANDARD_FIELD_MAC(1),
/* SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_MAP_DSCP_TO_DSCP                              */
   STANDARD_FIELD_MAC(1),
/* SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_TRUST_QOS_MAPPING_TABLE_INDEX                 */
   STANDARD_FIELD_MAC(4),
/* SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_PORT_MODIFY_DSCP                              */
   STANDARD_FIELD_MAC(1),
/* SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_PORT_MODIFY_UP                                */
   STANDARD_FIELD_MAC(1),
/* SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_PORT_QOS_PRECEDENCE                           */
   STANDARD_FIELD_MAC(1),
/* SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_NESTED_VLAN_ACCESS_PORT                       */
   STANDARD_FIELD_MAC(1),
/* SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_OVERSIZE_UNTAGGED_PKTS_FILTER_EN              */
   STANDARD_FIELD_MAC(1),
/* SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_TRILL_ENGINE_EN                               */
   STANDARD_FIELD_MAC(1),
/* SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_TRILL_INGRESS_OUTER_VID0                      */
   STANDARD_FIELD_MAC(12),
/* SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_TTI_802_1AH_PASSENGER_STAG_IS_TAG0_1          */
   STANDARD_FIELD_MAC(1),
/* SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_IPV4_6_TOTAL_LENGTH_DEDUCTION_EN              */
   STANDARD_FIELD_MAC(1),
/* SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_OAM_LINK_LAYER_PDU_TRAP_EN                    */
   STANDARD_FIELD_MAC(1),
/* SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_OAM_LINK_LAYER_LOOPBACK_EN                    */
   STANDARD_FIELD_MAC(1),
/* SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_UP_CFI_TO_QOS_TABLE_SELECT_MODE               */
   STANDARD_FIELD_MAC(1),
/* SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_LOOKUP0_PCL_CFG_MODE                          */
   STANDARD_FIELD_MAC(2),
/* SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_LOOKUP1_PCL_CFG_MODE                          */
   STANDARD_FIELD_MAC(2),
/* SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_LOOKUP2_PCL_CFG_MODE                          */
   STANDARD_FIELD_MAC(2),
/* SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_ASSIGN_VF_ID_EN                               */
   STANDARD_FIELD_MAC(1),
/* SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_TRUST_L2_QOS_TAG0_OR_TAG1                     */
   STANDARD_FIELD_MAC(1),
/* SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_TTI_PKT_TYPE_UDB_KEY_I_ENABLE                 */
   STANDARD_FIELD_MAC(16),
/* SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_IPV4_MC_DUPLICATION_MODE                      */
   STANDARD_FIELD_MAC(3),
/* SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_IPV6_MC_DUPLICATION_MODE                      */
   STANDARD_FIELD_MAC(3),
/* SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_MPLS_MC_DUPLICATION_ENABLE                    */
   STANDARD_FIELD_MAC(1),
/* SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_TRILL_MC_DUPLICATION_ENABLE                   */
   STANDARD_FIELD_MAC(1),
/* SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_PBB_MC_DUPLICATION_ENABLE                     */
   STANDARD_FIELD_MAC(1),
/* SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_MRU_INDEX                                     */
   STANDARD_FIELD_MAC(3),
/* SMEM_SIP6_30_TTI_DEFAULT_EPORT_TABLE_FIELDS_SOURCE_EPG_E                               */
    STANDARD_FIELD_MAC(12),
/* SMEM_SIP6_30_TTI_DEFAULT_EPORT_TABLE_FIELDS_COPY_TAG1_VID_TO_SRC_EPG_E                 */
    STANDARD_FIELD_MAC(1),
/* SMEM_SIP6_30_TTI_DEFAULT_EPORT_TABLE_FIELDS_EPORT_UP1_E                                */
    STANDARD_FIELD_MAC(3),
/* SMEM_SIP6_30_TTI_DEFAULT_EPORT_TABLE_FIELDS_EPORT_DEI1_E                               */
    STANDARD_FIELD_MAC(1),
/* SMEM_SIP6_30_TTI_DEFAULT_EPORT_TABLE_FIELDS_EPORT_DEI0_E                               */
    STANDARD_FIELD_MAC(1),
/* SMEM_SIP6_30_TTI_DEFAULT_EPORT_TABLE_FIELDS_PRP_ENABLED_E                              */
    STANDARD_FIELD_MAC(1),
/* SMEM_SIP6_30_TTI_DEFAULT_EPORT_TABLE_FIELDS_PRT_PORT_LAN_ID_E                          */
    STANDARD_FIELD_MAC(4),
/* SMEM_SIP6_30_TTI_DEFAULT_EPORT_TABLE_FIELDS_PRT_TREAT_WRONG_LAN_ID_AS_RCT_EXISTS_E     */
    STANDARD_FIELD_MAC(1),
/* SMEM_SIP6_30_TTI_DEFAULT_EPORT_TABLE_FIELDS_LSDU_CONSTANT_E                            */
    STANDARD_FIELD_MAC(5),
/* SMEM_SIP7_TTI_DEFAULT_EPORT_TABLE_FIELDS_EVLAN_EXTENSION_E                             */
    STANDARD_FIELD_MAC(2),
/* SMEM_SIP7_TTI_DEFAULT_EPORT_TABLE_FIELDS_GENERIC_RANGE_SOURCE_PORT_PROFILE_E           */
    STANDARD_FIELD_MAC(10),
/* SMEM_SIP7_TTI_DEFAULT_EPORT_TABLE_FIELDS_SERIAL_EM_PORT_PROFILE_ID_E                   */
    STANDARD_FIELD_MAC(3),
/* SMEM_SIP7_TTI_DEFAULT_EPORT_TABLE_FIELDS_GENERIC_CLASSIFICATION_FIELD_DEFAULT_VALUE_E  */
    STANDARD_FIELD_MAC(10),
/* SMEM_SIP7_TTI_DEFAULT_EPORT_TABLE_FIELDS_SERIAL_EM1_LOOKUP_KEY_MODE_E  */
    STANDARD_FIELD_MAC(2),
/* SMEM_SIP7_TTI_DEFAULT_EPORT_TABLE_FIELDS_SERIAL_EM2_LOOKUP_KEY_MODE_E  */
    STANDARD_FIELD_MAC(2),
/* SMEM_SIP7_TTI_DEFAULT_E_PORT_TABLE_FIELDS_MIDWAY_PCL_CFG_MODE_E                         */
    STANDARD_FIELD_MAC(0), /* not present */
/* SMEM_SIP7_TTI_DEFAULT_E_PORT_TABLE_FIELDS_EXCESSIVE_L2_TAG_EXCEPTION_EN_E               */
    STANDARD_FIELD_MAC(0), /* not present */
/* SMEM_SIP7_TTI_DEFAULT_E_PORT_TABLE_FIELDS_INGRESS_CASCADE_DBL_EN_E                      */
    STANDARD_FIELD_MAC(0), /* not present */
/* SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_TRUST_QOS_MAPPING_TABLE_INDEX_EXTENSION_E    */
    STANDARD_FIELD_MAC(0) /* not present */
};

SNET_ENTRY_FORMAT_TABLE_STC sip7TtiDefaultEPortTableFieldsFormat[SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS___LAST_VALUE___E] =
{
/* SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_PROT_BASED_QOS_EN                             */
   EXPLICIT_FIELD_MAC(0,1), /* 0 0 Protocol Based QoS Enable*/
/* SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_PROT_BASED_VLAN_EN                            */
   EXPLICIT_FIELD_MAC(1,1), /* 1 1 Protocol Based eVLAN Enable*/
/* SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_DEF_TAG1_VLAN_ID                              */
   EXPLICIT_FIELD_MAC(2,12), /* 2 13 Default Tag1 VLAN-ID*/
/* SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_PORT_UP0                                      */
   EXPLICIT_FIELD_MAC(14,3), /* 14 16 ePort UP0*/
/* SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_INGRESS_TAG0_TPID_PROFILE                     */
   EXPLICIT_FIELD_MAC(17,3), /* 17 19 Ingress Tag0 TPID profile Index*/
/* SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_INGRESS_TAG1_TPID_PROFILE                     */
   EXPLICIT_FIELD_MAC(20,3), /* 20 22 Ingress Tag1 TPID profile Index*/
/* SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_VLAN_TRANSLATION                              */
   EXPLICIT_FIELD_MAC(23,1), /* 23 23 Ingress VLAN translation enable*/
/* SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_P_VID_PRECEDENCE                              */
   EXPLICIT_FIELD_MAC(24,1), /* 24 24 eVLAN Precedence*/
/* SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_P_EVLAN_MODE                                  */
   EXPLICIT_FIELD_MAC(25,1), /* 25 25 Port eVLAN Mode*/
/* SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_P_EVLAN                                       */
   EXPLICIT_FIELD_MAC(26,13), /* 26 38 Default ePort eVLAN*/
/* SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_TTI_802_1AH_MAC_TO_ME_EN                      */
   EXPLICIT_FIELD_MAC(39,1), /* 39 39 TTI 802.1ah MAC to ME Enable*/
/* SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_EN_802_1AH_TTI_LOOKUP                         */
   EXPLICIT_FIELD_MAC(40,1), /* 40 40 Enable 802.1ah TTI Lookup*/
/* SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_ETHERNET_CLASSIFIER_EN                        */
   EXPLICIT_FIELD_MAC(41,1), /* 41 41 Enable Ethernet TTI Lookup*/
/* SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_MPLS_TUNNEL_TERMINATION_EN                    */
   EXPLICIT_FIELD_MAC(42,1), /* 42 42 Enable MPLS TTI Lookup*/
/* SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_TTI_MPLS_MAC_TO_ME_EN                         */
   EXPLICIT_FIELD_MAC(43,1), /* 43  43 TTI MPLS MAC to ME Enable*/
/* SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_TTI_IPV4_MAC_TO_ME_EN                         */
   EXPLICIT_FIELD_MAC(44,1), /* 44 44 TTI IPv4 MAC to ME Enable*/
/* SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_IPV4_TTI_FOR_TT_ONLY                          */
   EXPLICIT_FIELD_MAC(45,1), /* 45 45 IPv4 TTI for TT Only*/
/* SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_IPV4_TUNNEL_TERMINATION_EN                    */
   EXPLICIT_FIELD_MAC(46,1), /* 46 46 Enable IPv4 ARP TTI Lookup*/
/* SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_EXTENDED_DSA_BYPASS_BRIDGE                    */
   EXPLICIT_FIELD_MAC(47,1), /* 47 47 DSA Bypass Bridge*/
/* SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_PORT_QOS_PROFILE                              */
   EXPLICIT_FIELD_MAC(48,10), /* 48 57 ePort QoS Profile*/
/* SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_TRUST_DSA_TAG_QOS                             */
   EXPLICIT_FIELD_MAC(58,1), /* 58 58 Trust DSA Tag QoS*/
/* SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_TRUST_EXP                                     */
   EXPLICIT_FIELD_MAC(59,1), /* 59 59 Trust EXP*/
/* SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_TRUST_DSCP                                    */
   EXPLICIT_FIELD_MAC(60,1), /* 60 60 Trust DSCP*/
/* SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_TRUST_UP                                      */
   EXPLICIT_FIELD_MAC(61,2), /* 61 62 Trust UP*/
/* SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_MAP_DSCP_TO_DSCP                              */
   EXPLICIT_FIELD_MAC(71,1), /* 71 71 Map DSCP to DSCP*/
/* SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_TRUST_QOS_MAPPING_TABLE_INDEX                 */
   EXPLICIT_FIELD_MAC(63,4), /* 63 66 Trust QoS Mapping Table Index*/
/* SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_PORT_MODIFY_DSCP                              */
   EXPLICIT_FIELD_MAC(67,1), /* 67 67 Port Modify DSCP EXP*/
/* SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_PORT_MODIFY_UP                                */
   EXPLICIT_FIELD_MAC(68,1), /* 68 68 ePort Modify UP*/
/* SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_PORT_QOS_PRECEDENCE                           */
   EXPLICIT_FIELD_MAC(69,1), /* 69 69 ePort QoS Precedence*/
/* SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_NESTED_VLAN_ACCESS_PORT                       */
   EXPLICIT_FIELD_MAC(70,1), /* 70 70 Nested VLAN Access Port*/
/* SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_OVERSIZE_UNTAGGED_PKTS_FILTER_EN              */
   STANDARD_FIELD_MAC(0),
/* SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_TRILL_ENGINE_EN                               */
   EXPLICIT_FIELD_MAC(72,1), /* 72 72 TRILL engine enable*/
/* SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_TRILL_INGRESS_OUTER_VID0                      */
   EXPLICIT_FIELD_MAC(73,12), /* 73 84 TRILL Ingress outer VID0*/
/* SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_TTI_802_1AH_PASSENGER_STAG_IS_TAG0_1          */
   EXPLICIT_FIELD_MAC(85,1), /* 85 85 TTI passenger Outer Tag is Tag0_1*/
/* SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_IPV4_6_TOTAL_LENGTH_DEDUCTION_EN              */
   EXPLICIT_FIELD_MAC(86,1), /* 86 86 IPv4/6 Total Length Deduction Enable*/
/* SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_OAM_LINK_LAYER_PDU_TRAP_EN                    */
   EXPLICIT_FIELD_MAC(87,1), /* 87 87 OAM Link Layer PDU Trap Enable*/
/* SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_OAM_LINK_LAYER_LOOPBACK_EN                    */
   EXPLICIT_FIELD_MAC(88,1), /* 88 88 OAM LinkLayer LoopBack Enable*/
/* SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_UP_CFI_TO_QOS_TABLE_SELECT_MODE               */
   EXPLICIT_FIELD_MAC(89,1), /* 89 89 UP-CFI to QoS table select mode*/
/* SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_LOOKUP0_PCL_CFG_MODE                          */
   STANDARD_FIELD_MAC(0),
/* SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_LOOKUP1_PCL_CFG_MODE                          */
   EXPLICIT_FIELD_MAC(92,2), /* 92 93 IPCL1 Lookup PCL Configuration Mode*/
/* SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_LOOKUP2_PCL_CFG_MODE                          */
   EXPLICIT_FIELD_MAC(94,2), /* 94 95 IPCL2 Lookup PCL Configuration Mode*/
/* SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_ASSIGN_VF_ID_EN                               */
   EXPLICIT_FIELD_MAC(96,2), /* 96 96 Assign VF_ID Enable*/
/* SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_TRUST_L2_QOS_TAG0_OR_TAG1                     */
   EXPLICIT_FIELD_MAC(97,1), /* 97 97 Trust L2 QoS Tag0 or Tag1*/
/* SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_TTI_PKT_TYPE_UDB_KEY_I_ENABLE                 */
   EXPLICIT_FIELD_MAC(98,16), /* 98 113 TTI Pkt Type UDB Key i Enable1*/
/* SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_IPV4_MC_DUPLICATION_MODE                      */
   EXPLICIT_FIELD_MAC(114,3), /* 114 116 IPv4 MC Duplication Mode*/
/* SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_IPV6_MC_DUPLICATION_MODE                      */
   EXPLICIT_FIELD_MAC(117,3), /* 117 119 IPv6 MC Duplication Mode*/
/* SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_MPLS_MC_DUPLICATION_ENABLE                    */
   EXPLICIT_FIELD_MAC(120,1), /* 120 120 MPLS MC Duplication Enable*/
/* SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_TRILL_MC_DUPLICATION_ENABLE                   */
   EXPLICIT_FIELD_MAC(121,1), /* 121 121 TRILL MC Duplication Enable*/
/* SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_PBB_MC_DUPLICATION_ENABLE                     */
   EXPLICIT_FIELD_MAC(122,1), /* 122 122 PBB MC Duplication Enable*/
/* SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_MRU_INDEX                                     */
   EXPLICIT_FIELD_MAC(123,3), /* 123 125 MRU Index*/
/* SMEM_SIP6_30_TTI_DEFAULT_EPORT_TABLE_FIELDS_SOURCE_EPG_E                               */
   EXPLICIT_FIELD_MAC(126,12), /* 126 137 Source EPG*/
/* SMEM_SIP6_30_TTI_DEFAULT_EPORT_TABLE_FIELDS_COPY_TAG1_VID_TO_SRC_EPG_E                 */
   EXPLICIT_FIELD_MAC(138,1), /* 138 138 Copy Tag1 VID to Src EPG*/
/* SMEM_SIP6_30_TTI_DEFAULT_EPORT_TABLE_FIELDS_EPORT_UP1_E                                */
   EXPLICIT_FIELD_MAC(139,3), /* 139 141 ePort UP1*/
/* SMEM_SIP6_30_TTI_DEFAULT_EPORT_TABLE_FIELDS_EPORT_DEI1_E                               */
   EXPLICIT_FIELD_MAC(142,1), /* 142 142 ePort DEI1*/
/* SMEM_SIP6_30_TTI_DEFAULT_EPORT_TABLE_FIELDS_EPORT_DEI0_E                               */
   EXPLICIT_FIELD_MAC(143,1), /* 143 143 ePort DEI0*/
/* SMEM_SIP6_30_TTI_DEFAULT_EPORT_TABLE_FIELDS_PRP_ENABLED_E                              */
   EXPLICIT_FIELD_MAC(144,1), /* 144 144 PRP enabled*/
/* SMEM_SIP6_30_TTI_DEFAULT_EPORT_TABLE_FIELDS_PRT_PORT_LAN_ID_E                          */
   EXPLICIT_FIELD_MAC(145,4), /* 145 148 Port LAN ID*/
/* SMEM_SIP6_30_TTI_DEFAULT_EPORT_TABLE_FIELDS_PRT_TREAT_WRONG_LAN_ID_AS_RCT_EXISTS_E     */
   EXPLICIT_FIELD_MAC(149,1), /* 149 149 Treat wrong LAN ID as RCT exists*/
/* SMEM_SIP6_30_TTI_DEFAULT_EPORT_TABLE_FIELDS_LSDU_CONSTANT_E                            */
   EXPLICIT_FIELD_MAC(150,5), /* 150 154 LSDU constant*/
/* SMEM_SIP7_TTI_DEFAULT_EPORT_TABLE_FIELDS_EVLAN_EXTENSION_E                             */
   EXPLICIT_FIELD_MAC(155,3), /* 155 157 Default ePort eVLAN Extension*/
/* SMEM_SIP7_TTI_DEFAULT_EPORT_TABLE_FIELDS_GENERIC_RANGE_SOURCE_PORT_PROFILE_E           */
   EXPLICIT_FIELD_MAC(158,10), /* 158 167 source port profile*/
/* SMEM_SIP7_TTI_DEFAULT_EPORT_TABLE_FIELDS_SERIAL_EM_PORT_PROFILE_ID_E                   */
   EXPLICIT_FIELD_MAC(168,3), /* 168 170 Serial EM Port Profile ID*/
/* SMEM_SIP7_TTI_DEFAULT_EPORT_TABLE_FIELDS_GENERIC_CLASSIFICATION_FIELD_DEFAULT_VALUE_E  */
   EXPLICIT_FIELD_MAC(171,10), /* 170 179 Generic Classification Field default value*/
/* SMEM_SIP7_TTI_DEFAULT_EPORT_TABLE_FIELDS_SERIAL_EM1_LOOKUP_KEY_MODE_E  */
   EXPLICIT_FIELD_MAC(183,2), /* 183 184 Serial EM1 Lookup Key Mode*/
/* SMEM_SIP7_TTI_DEFAULT_EPORT_TABLE_FIELDS_SERIAL_EM2_LOOKUP_KEY_MODE_E  */
   EXPLICIT_FIELD_MAC(185,2), /* 185 186 Serial EM2 Lookup Key Mode*/
/* SMEM_SIP7_TTI_DEFAULT_E_PORT_TABLE_FIELDS_MIDWAY_PCL_CFG_MODE_E                         */
   EXPLICIT_FIELD_MAC(90,2), /* 90 91 MPCL Lookup PCL Configuration Mode*/
/* SMEM_SIP7_TTI_DEFAULT_E_PORT_TABLE_FIELDS_EXCESSIVE_L2_TAG_EXCEPTION_EN_E               */
   EXPLICIT_FIELD_MAC(181,1), /* 181 181 Excessive L2 Tag Exception Enable*/
/* SMEM_SIP7_TTI_DEFAULT_E_PORT_TABLE_FIELDS_INGRESS_CASCADE_DBL_EN_E                      */
   EXPLICIT_FIELD_MAC(182,1), /* 182 182 Ingress Cascade DLB Enable*/
/* SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_TRUST_QOS_MAPPING_TABLE_INDEX_EXTENSION_E    */
   EXPLICIT_FIELD_MAC(187,3) /* 187 189 Trust QoS Mapping Table Index Extension*/

};

#define SMEM_LION3_TTI_DEFAULT_PORT_PROTOCOL_EVLAN_AND_QOS_CONFIGURATION_TABLE_FIELDS_NAME                                     \
     STR( SMEM_LION3_TTI_DEFAULT_PORT_PROTOCOL_EVLAN_AND_QOS_CONFIGURATION_TABLE_FIELDS_PROTOCOL_EVLAN                 )\
    ,STR( SMEM_LION3_TTI_DEFAULT_PORT_PROTOCOL_EVLAN_AND_QOS_CONFIGURATION_TABLE_FIELDS_EVLAN_COMMAND                  )\
    ,STR( SMEM_LION3_TTI_DEFAULT_PORT_PROTOCOL_EVLAN_AND_QOS_CONFIGURATION_TABLE_FIELDS_EVLAN_PRECEDENCE               )\
    ,STR( SMEM_LION3_TTI_DEFAULT_PORT_PROTOCOL_EVLAN_AND_QOS_CONFIGURATION_TABLE_FIELDS_VALID                          )\
    ,STR( SMEM_LION3_TTI_DEFAULT_PORT_PROTOCOL_EVLAN_AND_QOS_CONFIGURATION_TABLE_FIELDS_PROTOCOL_MODIFY_UP             )\
    ,STR( SMEM_LION3_TTI_DEFAULT_PORT_PROTOCOL_EVLAN_AND_QOS_CONFIGURATION_TABLE_FIELDS_PROTOCOL_MODIFY_DSCP           )\
    ,STR( SMEM_LION3_TTI_DEFAULT_PORT_PROTOCOL_EVLAN_AND_QOS_CONFIGURATION_TABLE_FIELDS_PROTOCOL_MODIFY_QOS_PROFILE    )\
    ,STR( SMEM_LION3_TTI_DEFAULT_PORT_PROTOCOL_EVLAN_AND_QOS_CONFIGURATION_TABLE_FIELDS_QOS_PROFILE                    )\
    ,STR( SMEM_LION3_TTI_DEFAULT_PORT_PROTOCOL_EVLAN_AND_QOS_CONFIGURATION_TABLE_FIELDS_PROTOCOL_QOS_PRECEDENCE        )

char * lion3TtiDefaultPortProtocolEvlanAndQosConfigurationFieldsTableNames[
    SMEM_LION3_TTI_DEFAULT_PORT_PROTOCOL_EVLAN_AND_QOS_CONFIGURATION_TABLE_FIELDS___LAST_VALUE___E] =
    {SMEM_LION3_TTI_DEFAULT_PORT_PROTOCOL_EVLAN_AND_QOS_CONFIGURATION_TABLE_FIELDS_NAME};

SNET_ENTRY_FORMAT_TABLE_STC lion3TtiDefaultPortProtocolEvlanAndQosConfigurationTableFieldsFormat[
    SMEM_LION3_TTI_DEFAULT_PORT_PROTOCOL_EVLAN_AND_QOS_CONFIGURATION_TABLE_FIELDS___LAST_VALUE___E] =
{
    /*SMEM_LION3_TTI_DEFAULT_PORT_PROTOCOL_EVLAN_AND_QOS_CONFIGURATION_TABLE_FIELDS_PROTOCOL_EVLAN                */
    STANDARD_FIELD_MAC(13),
    /*SMEM_LION3_TTI_DEFAULT_PORT_PROTOCOL_EVLAN_AND_QOS_CONFIGURATION_TABLE_FIELDS_EVLAN_COMMAND                 */
    STANDARD_FIELD_MAC(2),
    /*SMEM_LION3_TTI_DEFAULT_PORT_PROTOCOL_EVLAN_AND_QOS_CONFIGURATION_TABLE_FIELDS_EVLAN_PRECEDENCE              */
    STANDARD_FIELD_MAC(1),
    /*SMEM_LION3_TTI_DEFAULT_PORT_PROTOCOL_EVLAN_AND_QOS_CONFIGURATION_TABLE_FIELDS_VALID                         */
    STANDARD_FIELD_MAC(1),
    /*SMEM_LION3_TTI_DEFAULT_PORT_PROTOCOL_EVLAN_AND_QOS_CONFIGURATION_TABLE_FIELDS_PROTOCOL_MODIFY_UP            */
    STANDARD_FIELD_MAC(1),
    /*SMEM_LION3_TTI_DEFAULT_PORT_PROTOCOL_EVLAN_AND_QOS_CONFIGURATION_TABLE_FIELDS_PROTOCOL_MODIFY_DSCP          */
    STANDARD_FIELD_MAC(1),
    /*SMEM_LION3_TTI_DEFAULT_PORT_PROTOCOL_EVLAN_AND_QOS_CONFIGURATION_TABLE_FIELDS_PROTOCOL_MODIFY_QOS_PROFILE   */
    STANDARD_FIELD_MAC(2),
    /*SMEM_LION3_TTI_DEFAULT_PORT_PROTOCOL_EVLAN_AND_QOS_CONFIGURATION_TABLE_FIELDS_QOS_PROFILE                   */
    STANDARD_FIELD_MAC(10),
    /*SMEM_LION3_TTI_DEFAULT_PORT_PROTOCOL_EVLAN_AND_QOS_CONFIGURATION_TABLE_FIELDS_PROTOCOL_QOS_PRECEDENCE       */
    STANDARD_FIELD_MAC(1),
};

#define SMEM_LION3_TTI_EPORT_ATTRIBUTES_TABLE_FIELDS_NAME                            \
     STR( SMEM_LION3_TTI_EPORT_ATTRIBUTES_TABLE_FIELDS_ING_TAG0_TPID_PROFILE        )\
    ,STR( SMEM_LION3_TTI_EPORT_ATTRIBUTES_TABLE_FIELDS_ING_TAG1_TPID_PROFILE        )\
    ,STR( SMEM_LION3_TTI_EPORT_ATTRIBUTES_TABLE_FIELDS_NUM_OF_TAGS_TO_POP           )\
    ,STR( SMEM_LION3_TTI_EPORT_ATTRIBUTES_TABLE_FIELDS_ING_POLICY_EN                )\
    ,STR( SMEM_LION3_TTI_EPORT_ATTRIBUTES_TABLE_FIELDS_DIS_IPCL0_FOR_ROUTED         )\
    ,STR( SMEM_LION3_TTI_EPORT_ATTRIBUTES_TABLE_FIELDS_TRUST_L2_QOS_TAG0_OR_TAG1    )\
    ,STR( SMEM_LION3_TTI_EPORT_ATTRIBUTES_TABLE_FIELDS_OVERRIDE_MASK_HASH_EN        )\
    ,STR( SMEM_LION3_TTI_EPORT_ATTRIBUTES_TABLE_FIELDS_HASH_MASK_INDEX              )\
    ,STR( SMEM_LION3_TTI_EPORT_ATTRIBUTES_TABLE_FIELDS_PHY_SRC_MC_FILTERING_EN      )

char * lion3TtiEPortAttributesFieldsTableNames[
    SMEM_LION3_TTI_EPORT_ATTRIBUTES_TABLE_FIELDS___LAST_VALUE___E] =
    {SMEM_LION3_TTI_EPORT_ATTRIBUTES_TABLE_FIELDS_NAME};

SNET_ENTRY_FORMAT_TABLE_STC lion3TtiEPortAttributesTableFieldsFormat[
    SMEM_LION3_TTI_EPORT_ATTRIBUTES_TABLE_FIELDS___LAST_VALUE___E] =
{
/*SMEM_LION3_TTI_EPORT_ATTRIBUTES_TABLE_FIELDS_ING_TAG0_TPID_PROFILE      */
    STANDARD_FIELD_MAC(3),
/*SMEM_LION3_TTI_EPORT_ATTRIBUTES_TABLE_FIELDS_ING_TAG1_TPID_PROFILE      */
    STANDARD_FIELD_MAC(3),
/*SMEM_LION3_TTI_EPORT_ATTRIBUTES_TABLE_FIELDS_NUM_OF_TAGS_TO_POP         */
    STANDARD_FIELD_MAC(2),
/*SMEM_LION3_TTI_EPORT_ATTRIBUTES_TABLE_FIELDS_ING_POLICY_EN              */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_TTI_EPORT_ATTRIBUTES_TABLE_FIELDS_DIS_IPCL0_FOR_ROUTED       */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_TTI_EPORT_ATTRIBUTES_TABLE_FIELDS_TRUST_L2_QOS_TAG0_OR_TAG1  */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_TTI_EPORT_ATTRIBUTES_TABLE_FIELDS_OVERRIDE_MASK_HASH_EN      */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_TTI_EPORT_ATTRIBUTES_TABLE_FIELDS_HASH_MASK_INDEX            */
    STANDARD_FIELD_MAC(4),
/*SMEM_LION3_TTI_EPORT_ATTRIBUTES_TABLE_FIELDS_PHY_SRC_MC_FILTERING_EN    */
    STANDARD_FIELD_MAC(1)
};

SNET_ENTRY_FORMAT_TABLE_STC sip7TtiEPortAttributesTableFieldsFormat[
    SMEM_LION3_TTI_EPORT_ATTRIBUTES_TABLE_FIELDS___LAST_VALUE___E] =
{
/*SMEM_LION3_TTI_EPORT_ATTRIBUTES_TABLE_FIELDS_ING_TAG0_TPID_PROFILE      */
    STANDARD_FIELD_MAC(3),
/*SMEM_LION3_TTI_EPORT_ATTRIBUTES_TABLE_FIELDS_ING_TAG1_TPID_PROFILE      */
    STANDARD_FIELD_MAC(3),
/*SMEM_LION3_TTI_EPORT_ATTRIBUTES_TABLE_FIELDS_NUM_OF_TAGS_TO_POP         */
    STANDARD_FIELD_MAC(2),
/*SMEM_LION3_TTI_EPORT_ATTRIBUTES_TABLE_FIELDS_ING_POLICY_EN              */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_TTI_EPORT_ATTRIBUTES_TABLE_FIELDS_DIS_IPCL0_FOR_ROUTED       */
    STANDARD_FIELD_MAC(0),
/*SMEM_LION3_TTI_EPORT_ATTRIBUTES_TABLE_FIELDS_TRUST_L2_QOS_TAG0_OR_TAG1  */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_TTI_EPORT_ATTRIBUTES_TABLE_FIELDS_OVERRIDE_MASK_HASH_EN      */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_TTI_EPORT_ATTRIBUTES_TABLE_FIELDS_HASH_MASK_INDEX            */
    STANDARD_FIELD_MAC(4),
/*SMEM_LION3_TTI_EPORT_ATTRIBUTES_TABLE_FIELDS_PHY_SRC_MC_FILTERING_EN    */
    STANDARD_FIELD_MAC(1)
};


#define SMEM_LION3_L2_MLL_TABLE_FIELDS_NAME                                    \
     STR(SMEM_LION3_L2_MLL_TABLE_FIELDS_LAST_0                                 )\
    ,STR(SMEM_LION3_L2_MLL_TABLE_FIELDS_BIND_TO_MLL_COUNTER_0                  )\
    ,STR(SMEM_LION3_L2_MLL_TABLE_FIELDS_MASK_BITMAP_0                          )\
    ,STR(SMEM_LION3_L2_MLL_TABLE_FIELDS_USE_VIDX_0                             )\
    ,STR(SMEM_LION3_L2_MLL_TABLE_FIELDS_TARGET_IS_TRUNK_0                      )\
    ,STR(SMEM_SIP7_L2_MLL_TABLE_FIELDS_USE_VPORT_0                             )\
    ,STR(SMEM_LION3_L2_MLL_TABLE_FIELDS_TRG_EPORT_0                            )\
    ,STR(SMEM_LION3_L2_MLL_TABLE_FIELDS_TRG_TRUNK_ID_0                         )\
    ,STR(SMEM_LION3_L2_MLL_TABLE_FIELDS_VIDX_0                                 )\
    ,STR(SMEM_SIP7_L2_MLL_TABLE_FIELDS_VPORT_0                                 )\
    ,STR(SMEM_LION3_L2_MLL_TABLE_FIELDS_UNKNOWN_UC_FILTER_ENABLE_0             )\
    ,STR(SMEM_LION3_L2_MLL_TABLE_FIELDS_UNREGISTERED_MC_FILTER_ENABLE_0        )\
    ,STR(SMEM_LION3_L2_MLL_TABLE_FIELDS_BC_FILTER_ENABLE_0                     )\
    ,STR(SMEM_LION3_L2_MLL_TABLE_FIELDS_TRG_DEV_0                              )\
    ,STR(SMEM_LION3_L2_MLL_TABLE_FIELDS_ONE_PLUS_ONE_FILTERING_ENABLE_0        )\
    ,STR(SMEM_LION3_L2_MLL_TABLE_FIELDS_TTL_THRESHOLD_0                        )\
    ,STR(SMEM_LION3_L2_MLL_TABLE_FIELDS_MESH_ID_0                              )\
    ,STR(SMEM_LION3_L2_MLL_TABLE_FIELDS_MC_LOCAL_SWITCHING_ENABLE_0            )\
    ,STR(SMEM_LION3_L2_MLL_TABLE_FIELDS_MAX_HOP_COUNT_ENABLE_0                 )\
    ,STR(SMEM_LION3_L2_MLL_TABLE_FIELDS_MAX_OUTGOING_HOP_COUNT_0               )\
    ,STR(SMEM_LION3_L2_MLL_TABLE_FIELDS_RESERVED_1                             )\
    ,STR(SMEM_LION3_L2_MLL_TABLE_FIELDS_LAST_1                                 )\
    ,STR(SMEM_LION3_L2_MLL_TABLE_FIELDS_BIND_TO_MLL_COUNTER_1                  )\
    ,STR(SMEM_LION3_L2_MLL_TABLE_FIELDS_MASK_BITMAP_1                          )\
    ,STR(SMEM_LION3_L2_MLL_TABLE_FIELDS_USE_VIDX_1                             )\
    ,STR(SMEM_LION3_L2_MLL_TABLE_FIELDS_TARGET_IS_TRUNK_1                      )\
    ,STR(SMEM_SIP7_L2_MLL_TABLE_FIELDS_USE_VPORT_1                             )\
    ,STR(SMEM_LION3_L2_MLL_TABLE_FIELDS_TRG_EPORT_1                            )\
    ,STR(SMEM_LION3_L2_MLL_TABLE_FIELDS_TRG_TRUNK_ID_1                         )\
    ,STR(SMEM_LION3_L2_MLL_TABLE_FIELDS_VIDX_1                                 )\
    ,STR(SMEM_SIP7_L2_MLL_TABLE_FIELDS_VPORT_1                                 )\
    ,STR(SMEM_LION3_L2_MLL_TABLE_FIELDS_UNKNOWN_UC_FILTER_ENABLE_1             )\
    ,STR(SMEM_LION3_L2_MLL_TABLE_FIELDS_UNREGISTERED_MC_FILTER_ENABLE_1        )\
    ,STR(SMEM_LION3_L2_MLL_TABLE_FIELDS_BC_FILTER_ENABLE_1                     )\
    ,STR(SMEM_LION3_L2_MLL_TABLE_FIELDS_TRG_DEV_1                              )\
    ,STR(SMEM_LION3_L2_MLL_TABLE_FIELDS_ONE_PLUS_ONE_FILTERING_ENABLE_1        )\
    ,STR(SMEM_LION3_L2_MLL_TABLE_FIELDS_TTL_THRESHOLD_1                        )\
    ,STR(SMEM_LION3_L2_MLL_TABLE_FIELDS_MESH_ID_1                              )\
    ,STR(SMEM_LION3_L2_MLL_TABLE_FIELDS_MC_LOCAL_SWITCHING_ENABLE_1            )\
    ,STR(SMEM_LION3_L2_MLL_TABLE_FIELDS_MAX_HOP_COUNT_ENABLE_1                 )\
    ,STR(SMEM_LION3_L2_MLL_TABLE_FIELDS_MAX_OUTGOING_HOP_COUNT_1               )\
    ,STR(SMEM_LION3_L2_MLL_TABLE_FIELDS_RESERVED_2                             )\
    ,STR(SMEM_LION3_L2_MLL_TABLE_FIELDS_ENTRY_SELECTOR                         )\
    ,STR(SMEM_LION3_L2_MLL_TABLE_FIELDS_L2_NEXT_MLL_PTR                        )


static char * lion3L2MllFieldsTableNames[
    SMEM_LION3_L2_MLL_TABLE_FIELDS___LAST_VALUE___E] =
    {SMEM_LION3_L2_MLL_TABLE_FIELDS_NAME};

static SNET_ENTRY_FORMAT_TABLE_STC lion3L2MllTableFieldsFormat[
    SMEM_LION3_L2_MLL_TABLE_FIELDS___LAST_VALUE___E] =
{
/*SMEM_LION3_L2_MLL_TABLE_FIELDS_LAST_0                                    */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_L2_MLL_TABLE_FIELDS_BIND_TO_MLL_COUNTER_0                     */
    STANDARD_FIELD_MAC(2),
/*SMEM_LION3_L2_MLL_TABLE_FIELDS_MASK_BITMAP_0                             */
    STANDARD_FIELD_MAC(15),
/*SMEM_LION3_L2_MLL_TABLE_FIELDS_USE_VIDX_0                                */
    STANDARD_FIELD_MAC(1),

    /*UseVIDX0 = "False"*/
    /*SMEM_LION3_L2_MLL_TABLE_FIELDS_TARGET_IS_TRUNK_0                         */
        {FIELD_SET_IN_RUNTIME_CNS,
         1,
         SMEM_LION3_L2_MLL_TABLE_FIELDS_USE_VIDX_0},

        /*SMEM_SIP7_L2_MLL_TABLE_FIELDS_USE_VPORT_0*/
        STANDARD_FIELD_MAC(0),

        /*(UseVIDX0 = "False") and (TargetIsTrunk0 = "False")*/
        /*SMEM_LION3_L2_MLL_TABLE_FIELDS_TRG_EPORT_0                               */
            {FIELD_SET_IN_RUNTIME_CNS,
             13,
             SMEM_LION3_L2_MLL_TABLE_FIELDS_TARGET_IS_TRUNK_0},

        /*(UseVIDX0 = "False") and (TargetIsTrunk0 = "True")*/
        /*SMEM_LION3_L2_MLL_TABLE_FIELDS_TRG_TRUNK_ID_0                             */
            {FIELD_SET_IN_RUNTIME_CNS,
             12,
             SMEM_LION3_L2_MLL_TABLE_FIELDS_TARGET_IS_TRUNK_0},

    /*UseVIDX0 = "True"*/
    /*SMEM_LION3_L2_MLL_TABLE_FIELDS_VIDX_0                                    */
        {FIELD_SET_IN_RUNTIME_CNS,
         12,
         SMEM_LION3_L2_MLL_TABLE_FIELDS_TARGET_IS_TRUNK_0},

    /*SMEM_SIP7_L2_MLL_TABLE_FIELDS_VPORT_0*/
    STANDARD_FIELD_MAC(0),

/*SMEM_LION3_L2_MLL_TABLE_FIELDS_UNKNOWN_UC_FILTER_ENABLE_0                */
    {FIELD_SET_IN_RUNTIME_CNS,
     1,
     SMEM_LION3_L2_MLL_TABLE_FIELDS_TRG_EPORT_0},
/*SMEM_LION3_L2_MLL_TABLE_FIELDS_UNREGISTERED_MC_FILTER_ENABLE_0           */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_L2_MLL_TABLE_FIELDS_BC_FILTER_ENABLE_0                        */
    STANDARD_FIELD_MAC(1),

/*(UseVIDX0 = "False") and (TargetIsTrunk0 = "False")*/
/*SMEM_LION3_L2_MLL_TABLE_FIELDS_TRG_DEV_0                                 */
    STANDARD_FIELD_MAC(10),

/*SMEM_LION3_L2_MLL_TABLE_FIELDS_ONE_PLUS_ONE_FILTERING_ENABLE_0           */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_L2_MLL_TABLE_FIELDS_TTL_THRESHOLD_0                           */
    STANDARD_FIELD_MAC(8),
/*SMEM_LION3_L2_MLL_TABLE_FIELDS_MESH_ID_0                                 */
    STANDARD_FIELD_MAC(8),
/*SMEM_LION3_L2_MLL_TABLE_FIELDS_MC_LOCAL_SWITCHING_ENABLE_0               */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_L2_MLL_TABLE_FIELDS_MAX_HOP_COUNT_ENABLE_0                    */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_L2_MLL_TABLE_FIELDS_MAX_OUTGOING_HOP_COUNT_0                  */
    STANDARD_FIELD_MAC(6),
/*SMEM_LION3_L2_MLL_TABLE_FIELDS_RESERVED_1*/
    STANDARD_FIELD_MAC(0),


/*SMEM_LION3_L2_MLL_TABLE_FIELDS_LAST_1                                    */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_L2_MLL_TABLE_FIELDS_BIND_TO_MLL_COUNTER_1                     */
    STANDARD_FIELD_MAC(2),
/*SMEM_LION3_L2_MLL_TABLE_FIELDS_MASK_BITMAP_1                             */
    STANDARD_FIELD_MAC(15),
/*SMEM_LION3_L2_MLL_TABLE_FIELDS_USE_VIDX_1                                */
    STANDARD_FIELD_MAC(1),

    /*UseVIDX0 = "False"*/
    /*SMEM_LION3_L2_MLL_TABLE_FIELDS_TARGET_IS_TRUNK_1                         */
        {FIELD_SET_IN_RUNTIME_CNS,
         1,
         SMEM_LION3_L2_MLL_TABLE_FIELDS_USE_VIDX_1},

        /*SMEM_SIP7_L2_MLL_TABLE_FIELDS_USE_VPORT_1*/
        STANDARD_FIELD_MAC(0),

        /*(UseVIDX0 = "False") and (TargetIsTrunk0 = "False")*/
        /*SMEM_LION3_L2_MLL_TABLE_FIELDS_TRG_EPORT_1                               */
            {FIELD_SET_IN_RUNTIME_CNS,
             13,
             SMEM_LION3_L2_MLL_TABLE_FIELDS_TARGET_IS_TRUNK_1},

        /*(UseVIDX0 = "False") and (TargetIsTrunk0 = "True")*/
        /*SMEM_LION3_L2_MLL_TABLE_FIELDS_TRG_TRUNK_ID_1                             */
            {FIELD_SET_IN_RUNTIME_CNS,
             12,
             SMEM_LION3_L2_MLL_TABLE_FIELDS_TARGET_IS_TRUNK_1},

    /*UseVIDX0 = "True"*/
    /*SMEM_LION3_L2_MLL_TABLE_FIELDS_VIDX_1                                    */
        {FIELD_SET_IN_RUNTIME_CNS,
         12,
         SMEM_LION3_L2_MLL_TABLE_FIELDS_TARGET_IS_TRUNK_1},

    /*SMEM_SIP7_L2_MLL_TABLE_FIELDS_VPORT_1*/
    STANDARD_FIELD_MAC(0),

/*SMEM_LION3_L2_MLL_TABLE_FIELDS_UNKNOWN_UC_FILTER_ENABLE_1                */
    {FIELD_SET_IN_RUNTIME_CNS,
     1,
     SMEM_LION3_L2_MLL_TABLE_FIELDS_TRG_EPORT_1},
/*SMEM_LION3_L2_MLL_TABLE_FIELDS_UNREGISTERED_MC_FILTER_ENABLE_1           */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_L2_MLL_TABLE_FIELDS_BC_FILTER_ENABLE_1                        */
    STANDARD_FIELD_MAC(1),

/*(UseVIDX0 = "False") and (TargetIsTrunk0 = "False")*/
/*SMEM_LION3_L2_MLL_TABLE_FIELDS_TRG_DEV_1                                 */
    STANDARD_FIELD_MAC(10),

/*SMEM_LION3_L2_MLL_TABLE_FIELDS_ONE_PLUS_ONE_FILTERING_ENABLE_1           */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_L2_MLL_TABLE_FIELDS_TTL_THRESHOLD_1                           */
    STANDARD_FIELD_MAC(8),
/*SMEM_LION3_L2_MLL_TABLE_FIELDS_MESH_ID_1                                 */
    STANDARD_FIELD_MAC(8),
/*SMEM_LION3_L2_MLL_TABLE_FIELDS_MC_LOCAL_SWITCHING_ENABLE_1               */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_L2_MLL_TABLE_FIELDS_MAX_HOP_COUNT_ENABLE_1                    */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_L2_MLL_TABLE_FIELDS_MAX_OUTGOING_HOP_COUNT_1                  */
    STANDARD_FIELD_MAC(6),
/*SMEM_LION3_L2_MLL_TABLE_FIELDS_RESERVED_2*/
    STANDARD_FIELD_MAC(0),

/*SMEM_LION3_L2_MLL_TABLE_FIELDS_ENTRY_SELECTOR                            */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_L2_MLL_TABLE_FIELDS_L2_NEXT_MLL_PTR                           */
    STANDARD_FIELD_MAC(15)
};

static SNET_ENTRY_FORMAT_TABLE_STC sip5_20L2MllTableFieldsFormat[
    SMEM_LION3_L2_MLL_TABLE_FIELDS___LAST_VALUE___E] =
{
/*SMEM_LION3_L2_MLL_TABLE_FIELDS_LAST_0                                    */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_L2_MLL_TABLE_FIELDS_BIND_TO_MLL_COUNTER_0                     */
    STANDARD_FIELD_MAC(2),
/*SMEM_LION3_L2_MLL_TABLE_FIELDS_MASK_BITMAP_0                             */
    STANDARD_FIELD_MAC(15),
/*SMEM_LION3_L2_MLL_TABLE_FIELDS_USE_VIDX_0                                */
    STANDARD_FIELD_MAC(1),

    /*UseVIDX0 = "False"*/
    /*SMEM_LION3_L2_MLL_TABLE_FIELDS_TARGET_IS_TRUNK_0                         */
        {FIELD_SET_IN_RUNTIME_CNS,
         1,
         SMEM_LION3_L2_MLL_TABLE_FIELDS_USE_VIDX_0},

        /*SMEM_SIP7_L2_MLL_TABLE_FIELDS_USE_VPORT_0*/
        STANDARD_FIELD_MAC(0),

        /*(UseVIDX0 = "False") and (TargetIsTrunk0 = "False")*/
        /*SMEM_LION3_L2_MLL_TABLE_FIELDS_TRG_EPORT_0                               */
            {FIELD_SET_IN_RUNTIME_CNS,
             14,/*was 13 in sip5*/
             SMEM_LION3_L2_MLL_TABLE_FIELDS_TARGET_IS_TRUNK_0},

        /*(UseVIDX0 = "False") and (TargetIsTrunk0 = "True")*/
        /*SMEM_LION3_L2_MLL_TABLE_FIELDS_TRG_TRUNK_ID_0                             */
            {FIELD_SET_IN_RUNTIME_CNS,
             12,
             SMEM_LION3_L2_MLL_TABLE_FIELDS_TARGET_IS_TRUNK_0},

    /*UseVIDX0 = "True"*/
    /*SMEM_LION3_L2_MLL_TABLE_FIELDS_VIDX_0                                    */
        {FIELD_SET_IN_RUNTIME_CNS,
         12,
         SMEM_LION3_L2_MLL_TABLE_FIELDS_TARGET_IS_TRUNK_0},

    /*SMEM_SIP7_L2_MLL_TABLE_FIELDS_VPORT_0*/
    STANDARD_FIELD_MAC(0),

/*SMEM_LION3_L2_MLL_TABLE_FIELDS_UNKNOWN_UC_FILTER_ENABLE_0                */
    {FIELD_SET_IN_RUNTIME_CNS,
     1,
     SMEM_LION3_L2_MLL_TABLE_FIELDS_TRG_EPORT_0},
/*SMEM_LION3_L2_MLL_TABLE_FIELDS_UNREGISTERED_MC_FILTER_ENABLE_0           */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_L2_MLL_TABLE_FIELDS_BC_FILTER_ENABLE_0                        */
    STANDARD_FIELD_MAC(1),

/*(UseVIDX0 = "False") and (TargetIsTrunk0 = "False")*/
/*SMEM_LION3_L2_MLL_TABLE_FIELDS_TRG_DEV_0                                 */
    STANDARD_FIELD_MAC(10),

/*SMEM_LION3_L2_MLL_TABLE_FIELDS_ONE_PLUS_ONE_FILTERING_ENABLE_0           */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_L2_MLL_TABLE_FIELDS_TTL_THRESHOLD_0                           */
    STANDARD_FIELD_MAC(8),
/*SMEM_LION3_L2_MLL_TABLE_FIELDS_MESH_ID_0                                 */
    STANDARD_FIELD_MAC(8),
/*SMEM_LION3_L2_MLL_TABLE_FIELDS_MC_LOCAL_SWITCHING_ENABLE_0               */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_L2_MLL_TABLE_FIELDS_MAX_HOP_COUNT_ENABLE_0                    */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_L2_MLL_TABLE_FIELDS_MAX_OUTGOING_HOP_COUNT_0                  */
    STANDARD_FIELD_MAC(6),
/*SMEM_LION3_L2_MLL_TABLE_FIELDS_RESERVED_1*/
    STANDARD_FIELD_MAC(1),


/*SMEM_LION3_L2_MLL_TABLE_FIELDS_LAST_1                                    */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_L2_MLL_TABLE_FIELDS_BIND_TO_MLL_COUNTER_1                     */
    STANDARD_FIELD_MAC(2),
/*SMEM_LION3_L2_MLL_TABLE_FIELDS_MASK_BITMAP_1                             */
    STANDARD_FIELD_MAC(15),
/*SMEM_LION3_L2_MLL_TABLE_FIELDS_USE_VIDX_1                                */
    STANDARD_FIELD_MAC(1),

    /*UseVIDX0 = "False"*/
    /*SMEM_LION3_L2_MLL_TABLE_FIELDS_TARGET_IS_TRUNK_1                         */
        {FIELD_SET_IN_RUNTIME_CNS,
         1,
         SMEM_LION3_L2_MLL_TABLE_FIELDS_USE_VIDX_1},

        /*SMEM_SIP7_L2_MLL_TABLE_FIELDS_USE_VPORT_0*/
        STANDARD_FIELD_MAC(0),

        /*(UseVIDX0 = "False") and (TargetIsTrunk0 = "False")*/
        /*SMEM_LION3_L2_MLL_TABLE_FIELDS_TRG_EPORT_1                               */
            {FIELD_SET_IN_RUNTIME_CNS,
             14,/*was 13 in sip5*/
             SMEM_LION3_L2_MLL_TABLE_FIELDS_TARGET_IS_TRUNK_1},

        /*(UseVIDX0 = "False") and (TargetIsTrunk0 = "True")*/
        /*SMEM_LION3_L2_MLL_TABLE_FIELDS_TRG_TRUNK_ID_1                             */
            {FIELD_SET_IN_RUNTIME_CNS,
             12,
             SMEM_LION3_L2_MLL_TABLE_FIELDS_TARGET_IS_TRUNK_1},

    /*UseVIDX0 = "True"*/
    /*SMEM_LION3_L2_MLL_TABLE_FIELDS_VIDX_1                                    */
        {FIELD_SET_IN_RUNTIME_CNS,
         12,
         SMEM_LION3_L2_MLL_TABLE_FIELDS_TARGET_IS_TRUNK_1},

    /*SMEM_SIP7_L2_MLL_TABLE_FIELDS_VPORT_1*/
    STANDARD_FIELD_MAC(0),

/*SMEM_LION3_L2_MLL_TABLE_FIELDS_UNKNOWN_UC_FILTER_ENABLE_1                */
    {FIELD_SET_IN_RUNTIME_CNS,
     1,
     SMEM_LION3_L2_MLL_TABLE_FIELDS_TRG_EPORT_1},
/*SMEM_LION3_L2_MLL_TABLE_FIELDS_UNREGISTERED_MC_FILTER_ENABLE_1           */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_L2_MLL_TABLE_FIELDS_BC_FILTER_ENABLE_1                        */
    STANDARD_FIELD_MAC(1),

/*(UseVIDX0 = "False") and (TargetIsTrunk0 = "False")*/
/*SMEM_LION3_L2_MLL_TABLE_FIELDS_TRG_DEV_1                                 */
    STANDARD_FIELD_MAC(10),

/*SMEM_LION3_L2_MLL_TABLE_FIELDS_ONE_PLUS_ONE_FILTERING_ENABLE_1           */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_L2_MLL_TABLE_FIELDS_TTL_THRESHOLD_1                           */
    STANDARD_FIELD_MAC(8),
/*SMEM_LION3_L2_MLL_TABLE_FIELDS_MESH_ID_1                                 */
    STANDARD_FIELD_MAC(8),
/*SMEM_LION3_L2_MLL_TABLE_FIELDS_MC_LOCAL_SWITCHING_ENABLE_1               */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_L2_MLL_TABLE_FIELDS_MAX_HOP_COUNT_ENABLE_1                    */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_L2_MLL_TABLE_FIELDS_MAX_OUTGOING_HOP_COUNT_1                  */
    STANDARD_FIELD_MAC(6),
/*SMEM_LION3_L2_MLL_TABLE_FIELDS_RESERVED_2*/
    STANDARD_FIELD_MAC(1),

/*SMEM_LION3_L2_MLL_TABLE_FIELDS_ENTRY_SELECTOR                            */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_L2_MLL_TABLE_FIELDS_L2_NEXT_MLL_PTR                           */
    STANDARD_FIELD_MAC(15)
};


static SNET_ENTRY_FORMAT_TABLE_STC sip6_10L2MllTableFieldsFormat[
    SMEM_LION3_L2_MLL_TABLE_FIELDS___LAST_VALUE___E] =
{
/*SMEM_LION3_L2_MLL_TABLE_FIELDS_LAST_0                                    */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_L2_MLL_TABLE_FIELDS_BIND_TO_MLL_COUNTER_0                     */
    STANDARD_FIELD_MAC(2),
/*SMEM_LION3_L2_MLL_TABLE_FIELDS_MASK_BITMAP_0                             */
    STANDARD_FIELD_MAC(15),
/*SMEM_LION3_L2_MLL_TABLE_FIELDS_USE_VIDX_0                                */
    STANDARD_FIELD_MAC(1),

    /*UseVIDX0 = "False"*/
    /*SMEM_LION3_L2_MLL_TABLE_FIELDS_TARGET_IS_TRUNK_0                         */
        {FIELD_SET_IN_RUNTIME_CNS,
         1,
         SMEM_LION3_L2_MLL_TABLE_FIELDS_USE_VIDX_0},

        /*SMEM_SIP7_L2_MLL_TABLE_FIELDS_USE_VPORT_0*/
        STANDARD_FIELD_MAC(0),

        /*(UseVIDX0 = "False") and (TargetIsTrunk0 = "False")*/
        /*SMEM_LION3_L2_MLL_TABLE_FIELDS_TRG_EPORT_0                               */
            {FIELD_SET_IN_RUNTIME_CNS,
             14,/*was 13 in sip5*/
             SMEM_LION3_L2_MLL_TABLE_FIELDS_TARGET_IS_TRUNK_0},

        /*(UseVIDX0 = "False") and (TargetIsTrunk0 = "True")*/
        /*SMEM_LION3_L2_MLL_TABLE_FIELDS_TRG_TRUNK_ID_0                             */
            {FIELD_SET_IN_RUNTIME_CNS,
             12,
             SMEM_LION3_L2_MLL_TABLE_FIELDS_TARGET_IS_TRUNK_0},

    /*UseVIDX0 = "True"*/
    /*SMEM_LION3_L2_MLL_TABLE_FIELDS_VIDX_0                                    */
        {FIELD_SET_IN_RUNTIME_CNS,
         14,/* was 12 in sip6 */
         SMEM_LION3_L2_MLL_TABLE_FIELDS_TARGET_IS_TRUNK_0},

    /*SMEM_SIP7_L2_MLL_TABLE_FIELDS_VPORT_0*/
    STANDARD_FIELD_MAC(0),

/*SMEM_LION3_L2_MLL_TABLE_FIELDS_UNKNOWN_UC_FILTER_ENABLE_0                */
    {FIELD_SET_IN_RUNTIME_CNS,
     1,
     SMEM_LION3_L2_MLL_TABLE_FIELDS_TRG_EPORT_0},
/*SMEM_LION3_L2_MLL_TABLE_FIELDS_UNREGISTERED_MC_FILTER_ENABLE_0           */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_L2_MLL_TABLE_FIELDS_BC_FILTER_ENABLE_0                        */
    STANDARD_FIELD_MAC(1),

/*(UseVIDX0 = "False") and (TargetIsTrunk0 = "False")*/
/*SMEM_LION3_L2_MLL_TABLE_FIELDS_TRG_DEV_0                                 */
    STANDARD_FIELD_MAC(10),

/*SMEM_LION3_L2_MLL_TABLE_FIELDS_ONE_PLUS_ONE_FILTERING_ENABLE_0           */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_L2_MLL_TABLE_FIELDS_TTL_THRESHOLD_0                           */
    STANDARD_FIELD_MAC(8),
/*SMEM_LION3_L2_MLL_TABLE_FIELDS_MESH_ID_0                                 */
    STANDARD_FIELD_MAC(8),
/*SMEM_LION3_L2_MLL_TABLE_FIELDS_MC_LOCAL_SWITCHING_ENABLE_0               */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_L2_MLL_TABLE_FIELDS_MAX_HOP_COUNT_ENABLE_0                    */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_L2_MLL_TABLE_FIELDS_MAX_OUTGOING_HOP_COUNT_0                  */
    STANDARD_FIELD_MAC(6),
/*SMEM_LION3_L2_MLL_TABLE_FIELDS_RESERVED_1*/
    STANDARD_FIELD_MAC(1),


/*SMEM_LION3_L2_MLL_TABLE_FIELDS_LAST_1                                    */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_L2_MLL_TABLE_FIELDS_BIND_TO_MLL_COUNTER_1                     */
    STANDARD_FIELD_MAC(2),
/*SMEM_LION3_L2_MLL_TABLE_FIELDS_MASK_BITMAP_1                             */
    STANDARD_FIELD_MAC(15),
/*SMEM_LION3_L2_MLL_TABLE_FIELDS_USE_VIDX_1                                */
    STANDARD_FIELD_MAC(1),

    /*UseVIDX0 = "False"*/
    /*SMEM_LION3_L2_MLL_TABLE_FIELDS_TARGET_IS_TRUNK_1                         */
        {FIELD_SET_IN_RUNTIME_CNS,
         1,
         SMEM_LION3_L2_MLL_TABLE_FIELDS_USE_VIDX_1},

        /*SMEM_SIP7_L2_MLL_TABLE_FIELDS_USE_VPORT_1*/
        STANDARD_FIELD_MAC(0),

        /*(UseVIDX0 = "False") and (TargetIsTrunk0 = "False")*/
        /*SMEM_LION3_L2_MLL_TABLE_FIELDS_TRG_EPORT_1                               */
            {FIELD_SET_IN_RUNTIME_CNS,
             14,/*was 13 in sip5*/
             SMEM_LION3_L2_MLL_TABLE_FIELDS_TARGET_IS_TRUNK_1},

        /*(UseVIDX0 = "False") and (TargetIsTrunk0 = "True")*/
        /*SMEM_LION3_L2_MLL_TABLE_FIELDS_TRG_TRUNK_ID_1                             */
            {FIELD_SET_IN_RUNTIME_CNS,
             12,
             SMEM_LION3_L2_MLL_TABLE_FIELDS_TARGET_IS_TRUNK_1},

    /*UseVIDX0 = "True"*/
    /*SMEM_LION3_L2_MLL_TABLE_FIELDS_VIDX_1                                    */
        {FIELD_SET_IN_RUNTIME_CNS,
         14,/* was 12 in sip6 */
         SMEM_LION3_L2_MLL_TABLE_FIELDS_TARGET_IS_TRUNK_1},

    /*SMEM_SIP7_L2_MLL_TABLE_FIELDS_VPORT_1*/
    STANDARD_FIELD_MAC(0),

/*SMEM_LION3_L2_MLL_TABLE_FIELDS_UNKNOWN_UC_FILTER_ENABLE_1                */
    {FIELD_SET_IN_RUNTIME_CNS,
     1,
     SMEM_LION3_L2_MLL_TABLE_FIELDS_TRG_EPORT_1},
/*SMEM_LION3_L2_MLL_TABLE_FIELDS_UNREGISTERED_MC_FILTER_ENABLE_1           */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_L2_MLL_TABLE_FIELDS_BC_FILTER_ENABLE_1                        */
    STANDARD_FIELD_MAC(1),

/*(UseVIDX0 = "False") and (TargetIsTrunk0 = "False")*/
/*SMEM_LION3_L2_MLL_TABLE_FIELDS_TRG_DEV_1                                 */
    STANDARD_FIELD_MAC(10),

/*SMEM_LION3_L2_MLL_TABLE_FIELDS_ONE_PLUS_ONE_FILTERING_ENABLE_1           */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_L2_MLL_TABLE_FIELDS_TTL_THRESHOLD_1                           */
    STANDARD_FIELD_MAC(8),
/*SMEM_LION3_L2_MLL_TABLE_FIELDS_MESH_ID_1                                 */
    STANDARD_FIELD_MAC(8),
/*SMEM_LION3_L2_MLL_TABLE_FIELDS_MC_LOCAL_SWITCHING_ENABLE_1               */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_L2_MLL_TABLE_FIELDS_MAX_HOP_COUNT_ENABLE_1                    */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_L2_MLL_TABLE_FIELDS_MAX_OUTGOING_HOP_COUNT_1                  */
    STANDARD_FIELD_MAC(6),
/*SMEM_LION3_L2_MLL_TABLE_FIELDS_RESERVED_2*/
    STANDARD_FIELD_MAC(1),

/*SMEM_LION3_L2_MLL_TABLE_FIELDS_ENTRY_SELECTOR                            */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_L2_MLL_TABLE_FIELDS_L2_NEXT_MLL_PTR                           */
    STANDARD_FIELD_MAC(15)
};

static SNET_ENTRY_FORMAT_TABLE_STC sip7L2MllTableFieldsFormat[
    SMEM_LION3_L2_MLL_TABLE_FIELDS___LAST_VALUE___E] =
{
/*SMEM_LION3_L2_MLL_TABLE_FIELDS_LAST_0                                    */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_L2_MLL_TABLE_FIELDS_BIND_TO_MLL_COUNTER_0                     */
    STANDARD_FIELD_MAC(2),
/*SMEM_LION3_L2_MLL_TABLE_FIELDS_MASK_BITMAP_0                             */
    STANDARD_FIELD_MAC(15),
/*SMEM_LION3_L2_MLL_TABLE_FIELDS_USE_VIDX_0                                */
    EXPLICIT_FIELD_MAC(21,1),

    /*UseVIDX0 = "False"*/
    /*SMEM_LION3_L2_MLL_TABLE_FIELDS_TARGET_IS_TRUNK_0                         */
        {FIELD_SET_IN_RUNTIME_CNS,
         1,
         SMEM_LION3_L2_MLL_TABLE_FIELDS_USE_VIDX_0},

    /*SMEM_SIP7_L2_MLL_TABLE_FIELDS_USE_VPORT_0*/
        STANDARD_FIELD_MAC(1),


        /*(UseVIDX0 = "False") and (TargetIsTrunk0 = "False")*/
        /*SMEM_LION3_L2_MLL_TABLE_FIELDS_TRG_EPORT_0                               */
        {FIELD_SET_IN_RUNTIME_CNS,
         15,
         SMEM_SIP7_L2_MLL_TABLE_FIELDS_USE_VPORT_0},

        /*(UseVIDX0 = "False") and (TargetIsTrunk0 = "True")*/
        /*SMEM_LION3_L2_MLL_TABLE_FIELDS_TRG_TRUNK_ID_0                             */
        {FIELD_SET_IN_RUNTIME_CNS,
         12,
         SMEM_SIP7_L2_MLL_TABLE_FIELDS_USE_VPORT_0},

    /*UseVIDX0 = "True"*/
    /*SMEM_LION3_L2_MLL_TABLE_FIELDS_VIDX_0                                    */
        {FIELD_SET_IN_RUNTIME_CNS,
         14,
         SMEM_SIP7_L2_MLL_TABLE_FIELDS_USE_VPORT_0},

/*SMEM_SIP7_L2_MLL_TABLE_FIELDS_VPORT_0*/
        {FIELD_SET_IN_RUNTIME_CNS,
         18,
         SMEM_SIP7_L2_MLL_TABLE_FIELDS_USE_VPORT_0},


/*SMEM_LION3_L2_MLL_TABLE_FIELDS_UNKNOWN_UC_FILTER_ENABLE_0                */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_L2_MLL_TABLE_FIELDS_UNREGISTERED_MC_FILTER_ENABLE_0           */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_L2_MLL_TABLE_FIELDS_BC_FILTER_ENABLE_0                        */
    STANDARD_FIELD_MAC(1),

/*(UseVIDX0 = "False") and (TargetIsTrunk0 = "False")*/
/*SMEM_LION3_L2_MLL_TABLE_FIELDS_TRG_DEV_0                                 */
    STANDARD_FIELD_MAC(10),

/*SMEM_LION3_L2_MLL_TABLE_FIELDS_ONE_PLUS_ONE_FILTERING_ENABLE_0           */
    EXPLICIT_FIELD_MAC(63,1),
/*SMEM_LION3_L2_MLL_TABLE_FIELDS_TTL_THRESHOLD_0                           */
    {FIELD_SET_IN_RUNTIME_CNS,
     8,
     SMEM_LION3_L2_MLL_TABLE_FIELDS_TRG_DEV_0},
/*SMEM_LION3_L2_MLL_TABLE_FIELDS_MESH_ID_0                                 */
     {FIELD_SET_IN_RUNTIME_CNS,
     8,
     SMEM_LION3_L2_MLL_TABLE_FIELDS_ONE_PLUS_ONE_FILTERING_ENABLE_0},
/*SMEM_LION3_L2_MLL_TABLE_FIELDS_MC_LOCAL_SWITCHING_ENABLE_0               */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_L2_MLL_TABLE_FIELDS_MAX_HOP_COUNT_ENABLE_0                    */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_L2_MLL_TABLE_FIELDS_MAX_OUTGOING_HOP_COUNT_0                  */
    STANDARD_FIELD_MAC(6),
/*SMEM_LION3_L2_MLL_TABLE_FIELDS_RESERVED_1*/
    STANDARD_FIELD_MAC(4),


/*SMEM_LION3_L2_MLL_TABLE_FIELDS_LAST_1                                    */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_L2_MLL_TABLE_FIELDS_BIND_TO_MLL_COUNTER_1                     */
    STANDARD_FIELD_MAC(2),
/*SMEM_LION3_L2_MLL_TABLE_FIELDS_MASK_BITMAP_1                             */
    STANDARD_FIELD_MAC(15),
/*SMEM_LION3_L2_MLL_TABLE_FIELDS_USE_VIDX_1                                */
    EXPLICIT_FIELD_MAC(105,1),

    /*UseVIDX0 = "False"*/
    /*SMEM_LION3_L2_MLL_TABLE_FIELDS_TARGET_IS_TRUNK_1                         */
        {FIELD_SET_IN_RUNTIME_CNS,
         1,
         SMEM_LION3_L2_MLL_TABLE_FIELDS_USE_VIDX_1},

    /*SMEM_SIP7_L2_MLL_TABLE_FIELDS_USE_VPORT_1*/
        STANDARD_FIELD_MAC(1),

        /*(UseVIDX0 = "False") and (TargetIsTrunk0 = "False")*/
        /*SMEM_LION3_L2_MLL_TABLE_FIELDS_TRG_EPORT_1                               */
        {FIELD_SET_IN_RUNTIME_CNS,
         15,
         SMEM_SIP7_L2_MLL_TABLE_FIELDS_USE_VPORT_1},

        /*(UseVIDX0 = "False") and (TargetIsTrunk0 = "True")*/
        /*SMEM_LION3_L2_MLL_TABLE_FIELDS_TRG_TRUNK_ID_1                             */
        {FIELD_SET_IN_RUNTIME_CNS,
         12,
         SMEM_SIP7_L2_MLL_TABLE_FIELDS_USE_VPORT_1},

    /*UseVIDX0 = "True"*/
    /*SMEM_LION3_L2_MLL_TABLE_FIELDS_VIDX_1                                    */
        {FIELD_SET_IN_RUNTIME_CNS,
         14,
         SMEM_SIP7_L2_MLL_TABLE_FIELDS_USE_VPORT_1},

/*SMEM_SIP7_L2_MLL_TABLE_FIELDS_VPORT_1*/
        {FIELD_SET_IN_RUNTIME_CNS,
         18,
         SMEM_SIP7_L2_MLL_TABLE_FIELDS_USE_VPORT_1},

/*SMEM_LION3_L2_MLL_TABLE_FIELDS_UNKNOWN_UC_FILTER_ENABLE_1                */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_L2_MLL_TABLE_FIELDS_UNREGISTERED_MC_FILTER_ENABLE_1           */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_L2_MLL_TABLE_FIELDS_BC_FILTER_ENABLE_1                        */
    STANDARD_FIELD_MAC(1),

/*(UseVIDX0 = "False") and (TargetIsTrunk0 = "False")*/
/*SMEM_LION3_L2_MLL_TABLE_FIELDS_TRG_DEV_1                                 */
    STANDARD_FIELD_MAC(10),

/*SMEM_LION3_L2_MLL_TABLE_FIELDS_ONE_PLUS_ONE_FILTERING_ENABLE_1           */
   EXPLICIT_FIELD_MAC(147,1),
/*SMEM_LION3_L2_MLL_TABLE_FIELDS_TTL_THRESHOLD_1                           */
    {FIELD_SET_IN_RUNTIME_CNS,
     8,
     SMEM_LION3_L2_MLL_TABLE_FIELDS_TRG_DEV_1},
/*SMEM_LION3_L2_MLL_TABLE_FIELDS_MESH_ID_1                                 */
     {FIELD_SET_IN_RUNTIME_CNS,
     8,
     SMEM_LION3_L2_MLL_TABLE_FIELDS_ONE_PLUS_ONE_FILTERING_ENABLE_1},
/*SMEM_LION3_L2_MLL_TABLE_FIELDS_MC_LOCAL_SWITCHING_ENABLE_1               */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_L2_MLL_TABLE_FIELDS_MAX_HOP_COUNT_ENABLE_1                    */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_L2_MLL_TABLE_FIELDS_MAX_OUTGOING_HOP_COUNT_1                  */
    STANDARD_FIELD_MAC(6),
/*SMEM_LION3_L2_MLL_TABLE_FIELDS_RESERVED_2*/
    STANDARD_FIELD_MAC(3),

/*SMEM_LION3_L2_MLL_TABLE_FIELDS_ENTRY_SELECTOR                            */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_L2_MLL_TABLE_FIELDS_L2_NEXT_MLL_PTR                           */
    STANDARD_FIELD_MAC(17)
};

#define SMEM_LION3_IP_MLL_TABLE_FIELDS_NAME                                    \
     STR(SMEM_LION3_IP_MLL_TABLE_FIELDS_LAST_0                                  )\
    ,STR(SMEM_LION3_IP_MLL_TABLE_FIELDS_START_OF_TUNNEL_0                       )\
    ,STR(SMEM_LION3_IP_MLL_TABLE_FIELDS_MLL_RPF_FAIL_CMD_0                      )\
    ,STR(SMEM_LION3_IP_MLL_TABLE_FIELDS_MLL_EVID_0                              )\
    ,STR(SMEM_LION3_IP_MLL_TABLE_FIELDS_USE_VIDX_0                              )\
    ,STR(SMEM_LION3_IP_MLL_TABLE_FIELDS_TARGET_IS_TRUNK_0                       )\
    ,STR(SMEM_SIP7_IP_MLL_TABLE_FIELDS_USE_VPORT_0                              )\
    ,STR(SMEM_LION3_IP_MLL_TABLE_FIELDS_EVIDX_0                                 )\
    ,STR(SMEM_LION3_IP_MLL_TABLE_FIELDS_TRG_EPORT_0                             )\
    ,STR(SMEM_LION3_IP_MLL_TABLE_FIELDS_TRG_TRUNK_ID_0                          )\
    ,STR(SMEM_SIP7_IP_MLL_TABLE_FIELDS_VPORT_0                                  )\
    ,STR(SMEM_SIP7_IP_MLL_TABLE_FIELDS_DISABLE_MAC_SA_MODIFICATION_0            )\
    ,STR(SMEM_LION3_IP_MLL_TABLE_FIELDS_RESERVED_2                              )\
    ,STR(SMEM_LION3_IP_MLL_TABLE_FIELDS_TRG_DEV_0                               )\
    ,STR(SMEM_LION3_IP_MLL_TABLE_FIELDS_EXCLUDE_SRC_VLAN_0                      )\
    ,STR(SMEM_LION3_IP_MLL_TABLE_FIELDS_TTL_THRESHOLD_0_OR_HOP_LIMIT_THRESHOLD_0)\
    ,STR(SMEM_LION3_IP_MLL_TABLE_FIELDS_TUNNEL_PTR_0                            )\
    ,STR(SMEM_LION3_IP_MLL_TABLE_FIELDS_TUNNEL_TYPE_0                           )\
    ,STR(SMEM_LION3_IP_MLL_TABLE_FIELDS_LAST_1                                  )\
    ,STR(SMEM_LION3_IP_MLL_TABLE_FIELDS_START_OF_TUNNEL_1                       )\
    ,STR(SMEM_LION3_IP_MLL_TABLE_FIELDS_MLL_RPF_FAIL_CMD_1                      )\
    ,STR(SMEM_LION3_IP_MLL_TABLE_FIELDS_MLL_EVID_1                              )\
    ,STR(SMEM_LION3_IP_MLL_TABLE_FIELDS_USE_VIDX_1                              )\
    ,STR(SMEM_LION3_IP_MLL_TABLE_FIELDS_TARGET_IS_TRUNK_1                       )\
    ,STR(SMEM_SIP7_IP_MLL_TABLE_FIELDS_USE_VPORT_1                              )\
    ,STR(SMEM_LION3_IP_MLL_TABLE_FIELDS_EVIDX_1                                 )\
    ,STR(SMEM_LION3_IP_MLL_TABLE_FIELDS_TRG_EPORT_1                             )\
    ,STR(SMEM_LION3_IP_MLL_TABLE_FIELDS_TRG_TRUNK_ID_1                          )\
    ,STR(SMEM_SIP7_IP_MLL_TABLE_FIELDS_VPORT_1                                  )\
    ,STR(SMEM_SIP7_IP_MLL_TABLE_FIELDS_DISABLE_MAC_SA_MODIFICATION_1            )\
    ,STR(SMEM_LION3_IP_MLL_TABLE_FIELDS_RESERVED_3                              )\
    ,STR(SMEM_LION3_IP_MLL_TABLE_FIELDS_TRG_DEV_1                               )\
    ,STR(SMEM_LION3_IP_MLL_TABLE_FIELDS_EXCLUDE_SRC_VLAN_1                      )\
    ,STR(SMEM_LION3_IP_MLL_TABLE_FIELDS_TTL_THRESHOLD_1_OR_HOP_LIMIT_THRESHOLD_1)\
    ,STR(SMEM_LION3_IP_MLL_TABLE_FIELDS_TUNNEL_PTR_1                            )\
    ,STR(SMEM_LION3_IP_MLL_TABLE_FIELDS_TUNNEL_TYPE_1                           )\
    ,STR(SMEM_LION3_IP_MLL_TABLE_FIELDS_NEXT_MLL_PTR                            )

static char * lion3IpMllFieldsTableNames[
    SMEM_LION3_IP_MLL_TABLE_FIELDS___LAST_VALUE___E] =
    {SMEM_LION3_IP_MLL_TABLE_FIELDS_NAME};

static SNET_ENTRY_FORMAT_TABLE_STC lion3IpMllTableFieldsFormat[
    SMEM_LION3_IP_MLL_TABLE_FIELDS___LAST_VALUE___E] =
{
/*SMEM_LION3_IP_MLL_TABLE_FIELDS_LAST_0                                      */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_IP_MLL_TABLE_FIELDS_START_OF_TUNNEL_0                           */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_IP_MLL_TABLE_FIELDS_MLL_RPF_FAIL_CMD_0                          */
    STANDARD_FIELD_MAC(3),
/*SMEM_LION3_IP_MLL_TABLE_FIELDS_MLL_EVID_0                                  */
    STANDARD_FIELD_MAC(13),
/*SMEM_LION3_IP_MLL_TABLE_FIELDS_USE_VIDX_0                                  */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_IP_MLL_TABLE_FIELDS_TARGET_IS_TRUNK_0                           */
    STANDARD_FIELD_MAC(1),
/*SMEM_SIP7_IP_MLL_TABLE_FIELDS_USE_VPORT_0*/
    STANDARD_FIELD_MAC(0),
    /*UseVIDX0 = "True"*/
    /*SMEM_LION3_IP_MLL_TABLE_FIELDS_EVIDX_0                                     */
        {FIELD_SET_IN_RUNTIME_CNS,
         16,
         SMEM_LION3_IP_MLL_TABLE_FIELDS_TARGET_IS_TRUNK_0},

    /*UseVIDX0 = "False") and (TargetIsTrunk0 = "False")*/
    /*SMEM_LION3_IP_MLL_TABLE_FIELDS_TRG_EPORT_0                                 */
        {FIELD_SET_IN_RUNTIME_CNS,
         13,
         SMEM_LION3_IP_MLL_TABLE_FIELDS_TARGET_IS_TRUNK_0},

    /*(UseVIDX0 = "False") and (TargetIsTrunk0 = "True")*/
    /*SMEM_LION3_IP_MLL_TABLE_FIELDS_TRG_TRUNK_ID_0                              */
        {FIELD_SET_IN_RUNTIME_CNS,
         12,
         SMEM_LION3_IP_MLL_TABLE_FIELDS_TARGET_IS_TRUNK_0},
    /*SMEM_SIP7_IP_MLL_TABLE_FIELDS_VPORT_0*/
    STANDARD_FIELD_MAC(0),/* not exist */
    /*SMEM_SIP7_IP_MLL_TABLE_FIELDS_DISABLE_MAC_SA_MODIFICATION_0*/
    STANDARD_FIELD_MAC(0),/* not exist */

    /*SMEM_LION3_IP_MLL_TABLE_FIELDS_RESERVED_2*/
    STANDARD_FIELD_MAC(0),/* not exist */

/*(UseVIDX0 = "False") and (TargetIsTrunk0 = "False")*/
/*SMEM_LION3_IP_MLL_TABLE_FIELDS_TRG_DEV_0                                   */
    {FIELD_SET_IN_RUNTIME_CNS,
     10,
     SMEM_LION3_IP_MLL_TABLE_FIELDS_EVIDX_0},

/*SMEM_LION3_IP_MLL_TABLE_FIELDS_EXCLUDE_SRC_VLAN_0                          */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_IP_MLL_TABLE_FIELDS_TTL_THRESHOLD_0_OR_HOP_LIMIT_THRESHOLD_0    */
    STANDARD_FIELD_MAC(8),
/*SMEM_LION3_IP_MLL_TABLE_FIELDS_TUNNEL_PTR_0                                */
    STANDARD_FIELD_MAC(15),
/*SMEM_LION3_IP_MLL_TABLE_FIELDS_TUNNEL_TYPE_0                               */
    STANDARD_FIELD_MAC(1),


/*SMEM_LION3_IP_MLL_TABLE_FIELDS_LAST_1                                      */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_IP_MLL_TABLE_FIELDS_START_OF_TUNNEL_1                           */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_IP_MLL_TABLE_FIELDS_MLL_RPF_FAIL_CMD_1                          */
    STANDARD_FIELD_MAC(3),
/*SMEM_LION3_IP_MLL_TABLE_FIELDS_MLL_EVID_1                                  */
    STANDARD_FIELD_MAC(13),
/*SMEM_LION3_IP_MLL_TABLE_FIELDS_USE_VIDX_1                                  */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_IP_MLL_TABLE_FIELDS_TARGET_IS_TRUNK_1                           */
    STANDARD_FIELD_MAC(1),
/*SMEM_SIP7_IP_MLL_TABLE_FIELDS_USE_VPORT_1*/
    STANDARD_FIELD_MAC(0),/* not exist */

    /*UseVIDX0 = "True"*/
    /*SMEM_LION3_IP_MLL_TABLE_FIELDS_EVIDX_1                                     */
        {FIELD_SET_IN_RUNTIME_CNS,
         16,
         SMEM_LION3_IP_MLL_TABLE_FIELDS_TARGET_IS_TRUNK_1},

    /*UseVIDX0 = "False") and (TargetIsTrunk0 = "False")*/
    /*SMEM_LION3_IP_MLL_TABLE_FIELDS_TRG_EPORT_1                                 */
        {FIELD_SET_IN_RUNTIME_CNS,
         13,
         SMEM_LION3_IP_MLL_TABLE_FIELDS_TARGET_IS_TRUNK_1},

    /*(UseVIDX0 = "False") and (TargetIsTrunk0 = "True")*/
    /*SMEM_LION3_IP_MLL_TABLE_FIELDS_TRG_TRUNK_ID_1                              */
        {FIELD_SET_IN_RUNTIME_CNS,
         12,
         SMEM_LION3_IP_MLL_TABLE_FIELDS_TARGET_IS_TRUNK_1},

/*SMEM_SIP7_IP_MLL_TABLE_FIELDS_VPORT_1*/
    STANDARD_FIELD_MAC(0),/* not exist */
/*SMEM_SIP7_IP_MLL_TABLE_FIELDS_DISABLE_MAC_SA_MODIFICATION_1*/
    STANDARD_FIELD_MAC(0),/* not exist */

    /*SMEM_LION3_IP_MLL_TABLE_FIELDS_RESERVED_3*/
    STANDARD_FIELD_MAC(0),/* not exist */

/*(UseVIDX0 = "False") and (TargetIsTrunk0 = "False")*/
/*SMEM_LION3_IP_MLL_TABLE_FIELDS_TRG_DEV_1                                   */
    {FIELD_SET_IN_RUNTIME_CNS,
     10,
     SMEM_LION3_IP_MLL_TABLE_FIELDS_EVIDX_1},

/*SMEM_LION3_IP_MLL_TABLE_FIELDS_EXCLUDE_SRC_VLAN_1                          */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_IP_MLL_TABLE_FIELDS_TTL_THRESHOLD_1_OR_HOP_LIMIT_THRESHOLD_1    */
    STANDARD_FIELD_MAC(8),
/*SMEM_LION3_IP_MLL_TABLE_FIELDS_TUNNEL_PTR_1                                */
    STANDARD_FIELD_MAC(15),
/*SMEM_LION3_IP_MLL_TABLE_FIELDS_TUNNEL_TYPE_1                               */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_IP_MLL_TABLE_FIELDS_NEXT_MLL_PTR                                */
    STANDARD_FIELD_MAC(16)
};

static SNET_ENTRY_FORMAT_TABLE_STC sip5_20IpMllTableFieldsFormat[
    SMEM_LION3_IP_MLL_TABLE_FIELDS___LAST_VALUE___E] =
{
/*SMEM_LION3_IP_MLL_TABLE_FIELDS_LAST_0                                      */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_IP_MLL_TABLE_FIELDS_START_OF_TUNNEL_0                           */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_IP_MLL_TABLE_FIELDS_MLL_RPF_FAIL_CMD_0                          */
    STANDARD_FIELD_MAC(3),
/*SMEM_LION3_IP_MLL_TABLE_FIELDS_MLL_EVID_0                                  */
    STANDARD_FIELD_MAC(13),
/*SMEM_LION3_IP_MLL_TABLE_FIELDS_USE_VIDX_0                                  */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_IP_MLL_TABLE_FIELDS_TARGET_IS_TRUNK_0                           */
    STANDARD_FIELD_MAC(1),
/*SMEM_SIP7_IP_MLL_TABLE_FIELDS_USE_VPORT_0*/
    STANDARD_FIELD_MAC(0),

    /*UseVIDX0 = "True"*/
    /*SMEM_LION3_IP_MLL_TABLE_FIELDS_EVIDX_0                                     */
        {FIELD_SET_IN_RUNTIME_CNS,
         16,
         SMEM_LION3_IP_MLL_TABLE_FIELDS_TARGET_IS_TRUNK_0},

    /*UseVIDX0 = "False") and (TargetIsTrunk0 = "False")*/
    /*SMEM_LION3_IP_MLL_TABLE_FIELDS_TRG_EPORT_0                                 */
        {FIELD_SET_IN_RUNTIME_CNS,
         14,/*was 13 in sip5 */
         SMEM_LION3_IP_MLL_TABLE_FIELDS_TARGET_IS_TRUNK_0},

    /*(UseVIDX0 = "False") and (TargetIsTrunk0 = "True")*/
    /*SMEM_LION3_IP_MLL_TABLE_FIELDS_TRG_TRUNK_ID_0                              */
        {FIELD_SET_IN_RUNTIME_CNS,
         12,
         SMEM_LION3_IP_MLL_TABLE_FIELDS_TARGET_IS_TRUNK_0},

    /*SMEM_SIP7_IP_MLL_TABLE_FIELDS_VPORT_0*/
    STANDARD_FIELD_MAC(0),/* not exist */
    /*SMEM_SIP7_IP_MLL_TABLE_FIELDS_DISABLE_MAC_SA_MODIFICATION_0*/
    STANDARD_FIELD_MAC(0),/* not exist */

    /*SMEM_LION3_IP_MLL_TABLE_FIELDS_RESERVED_2*/
    {FIELD_SET_IN_RUNTIME_CNS,
     1,
     SMEM_LION3_IP_MLL_TABLE_FIELDS_EVIDX_0},

/*(UseVIDX0 = "False") and (TargetIsTrunk0 = "False")*/
/*SMEM_LION3_IP_MLL_TABLE_FIELDS_TRG_DEV_0                                   */
    STANDARD_FIELD_MAC(10),

/*SMEM_LION3_IP_MLL_TABLE_FIELDS_EXCLUDE_SRC_VLAN_0                          */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_IP_MLL_TABLE_FIELDS_TTL_THRESHOLD_0_OR_HOP_LIMIT_THRESHOLD_0    */
    STANDARD_FIELD_MAC(8),
/*SMEM_LION3_IP_MLL_TABLE_FIELDS_TUNNEL_PTR_0                                */
    STANDARD_FIELD_MAC(16),/* was 15 in sip 5*/
/*SMEM_LION3_IP_MLL_TABLE_FIELDS_TUNNEL_TYPE_0                               */
    STANDARD_FIELD_MAC(1),


/*SMEM_LION3_IP_MLL_TABLE_FIELDS_LAST_1                                      */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_IP_MLL_TABLE_FIELDS_START_OF_TUNNEL_1                           */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_IP_MLL_TABLE_FIELDS_MLL_RPF_FAIL_CMD_1                          */
    STANDARD_FIELD_MAC(3),
/*SMEM_LION3_IP_MLL_TABLE_FIELDS_MLL_EVID_1                                  */
    STANDARD_FIELD_MAC(13),
/*SMEM_LION3_IP_MLL_TABLE_FIELDS_USE_VIDX_1                                  */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_IP_MLL_TABLE_FIELDS_TARGET_IS_TRUNK_1                           */
    STANDARD_FIELD_MAC(1),
/*SMEM_SIP7_IP_MLL_TABLE_FIELDS_USE_VPORT_1*/
    STANDARD_FIELD_MAC(0),/* not exist */

    /*UseVIDX0 = "True"*/
    /*SMEM_LION3_IP_MLL_TABLE_FIELDS_EVIDX_1                                     */
        {FIELD_SET_IN_RUNTIME_CNS,
         16,
         SMEM_LION3_IP_MLL_TABLE_FIELDS_TARGET_IS_TRUNK_1},

    /*UseVIDX0 = "False") and (TargetIsTrunk0 = "False")*/
    /*SMEM_LION3_IP_MLL_TABLE_FIELDS_TRG_EPORT_1                                 */
        {FIELD_SET_IN_RUNTIME_CNS,
         14,/*was 13 in sip5 */
         SMEM_LION3_IP_MLL_TABLE_FIELDS_TARGET_IS_TRUNK_1},

    /*(UseVIDX0 = "False") and (TargetIsTrunk0 = "True")*/
    /*SMEM_LION3_IP_MLL_TABLE_FIELDS_TRG_TRUNK_ID_1                              */
        {FIELD_SET_IN_RUNTIME_CNS,
         12,
         SMEM_LION3_IP_MLL_TABLE_FIELDS_TARGET_IS_TRUNK_1},

/*SMEM_SIP7_IP_MLL_TABLE_FIELDS_VPORT_1*/
    STANDARD_FIELD_MAC(0),/* not exist */
/*SMEM_SIP7_IP_MLL_TABLE_FIELDS_DISABLE_MAC_SA_MODIFICATION_1*/
    STANDARD_FIELD_MAC(0),/* not exist */

    /*SMEM_LION3_IP_MLL_TABLE_FIELDS_RESERVED_3*/
    {FIELD_SET_IN_RUNTIME_CNS,
     1,
     SMEM_LION3_IP_MLL_TABLE_FIELDS_EVIDX_1},

/*(UseVIDX0 = "False") and (TargetIsTrunk0 = "False")*/
/*SMEM_LION3_IP_MLL_TABLE_FIELDS_TRG_DEV_1                                   */
    STANDARD_FIELD_MAC(10),

/*SMEM_LION3_IP_MLL_TABLE_FIELDS_EXCLUDE_SRC_VLAN_1                          */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_IP_MLL_TABLE_FIELDS_TTL_THRESHOLD_1_OR_HOP_LIMIT_THRESHOLD_1    */
    STANDARD_FIELD_MAC(8),
/*SMEM_LION3_IP_MLL_TABLE_FIELDS_TUNNEL_PTR_1                                */
    STANDARD_FIELD_MAC(16),/* was 15 in sip 5*/
/*SMEM_LION3_IP_MLL_TABLE_FIELDS_TUNNEL_TYPE_1                               */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_IP_MLL_TABLE_FIELDS_NEXT_MLL_PTR                                */
    STANDARD_FIELD_MAC(16)
};

static SNET_ENTRY_FORMAT_TABLE_STC sip7IpMllTableFieldsFormat[
    SMEM_LION3_IP_MLL_TABLE_FIELDS___LAST_VALUE___E] =
{
/*SMEM_LION3_IP_MLL_TABLE_FIELDS_LAST_0                                      */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_IP_MLL_TABLE_FIELDS_START_OF_TUNNEL_0                           */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_IP_MLL_TABLE_FIELDS_MLL_RPF_FAIL_CMD_0                          */
    STANDARD_FIELD_MAC(3),
/*SMEM_LION3_IP_MLL_TABLE_FIELDS_MLL_EVID_0                                  */
    STANDARD_FIELD_MAC(16),/*was 13*/
/*SMEM_LION3_IP_MLL_TABLE_FIELDS_USE_VIDX_0                                  */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_IP_MLL_TABLE_FIELDS_TARGET_IS_TRUNK_0                           */
    STANDARD_FIELD_MAC(1),
/*SMEM_SIP7_IP_MLL_TABLE_FIELDS_USE_VPORT_0*/
    STANDARD_FIELD_MAC(1),

    /*UseVIDX0 = "True"*/
    /*SMEM_LION3_IP_MLL_TABLE_FIELDS_EVIDX_0                                     */
        {FIELD_SET_IN_RUNTIME_CNS,
         17,
         SMEM_SIP7_IP_MLL_TABLE_FIELDS_USE_VPORT_0},

    /*UseVIDX0 = "False") and (TargetIsTrunk0 = "False")*/
    /*SMEM_LION3_IP_MLL_TABLE_FIELDS_TRG_EPORT_0                                 */
        {FIELD_SET_IN_RUNTIME_CNS,
         15,/*was 14 */
         SMEM_SIP7_IP_MLL_TABLE_FIELDS_USE_VPORT_0},

    /*(UseVIDX0 = "False") and (TargetIsTrunk0 = "True")*/
    /*SMEM_LION3_IP_MLL_TABLE_FIELDS_TRG_TRUNK_ID_0                              */
        {FIELD_SET_IN_RUNTIME_CNS,
         12,
         SMEM_SIP7_IP_MLL_TABLE_FIELDS_USE_VPORT_0},

    /*SMEM_SIP7_IP_MLL_TABLE_FIELDS_VPORT_0*/
        {FIELD_SET_IN_RUNTIME_CNS,
         18,
         SMEM_SIP7_IP_MLL_TABLE_FIELDS_USE_VPORT_0},

    /*SMEM_SIP7_IP_MLL_TABLE_FIELDS_DISABLE_MAC_SA_MODIFICATION_0*/
    STANDARD_FIELD_MAC(1),

    /*SMEM_LION3_IP_MLL_TABLE_FIELDS_RESERVED_2*/
    STANDARD_FIELD_MAC(2),

/*(UseVIDX0 = "False") and (TargetIsTrunk0 = "False")*/
/*SMEM_LION3_IP_MLL_TABLE_FIELDS_TRG_DEV_0                                   */
    STANDARD_FIELD_MAC(10),

/*SMEM_LION3_IP_MLL_TABLE_FIELDS_EXCLUDE_SRC_VLAN_0                          */
    EXPLICIT_FIELD_MAC(63,1),
/*SMEM_LION3_IP_MLL_TABLE_FIELDS_TTL_THRESHOLD_0_OR_HOP_LIMIT_THRESHOLD_0    */
    EXPLICIT_FIELD_MAC(55,8),
/*SMEM_LION3_IP_MLL_TABLE_FIELDS_TUNNEL_PTR_0                                */
    {FIELD_SET_IN_RUNTIME_CNS,
     19,
     SMEM_LION3_IP_MLL_TABLE_FIELDS_EXCLUDE_SRC_VLAN_0},
/*SMEM_LION3_IP_MLL_TABLE_FIELDS_TUNNEL_TYPE_0                               */
    STANDARD_FIELD_MAC(1),

/*SMEM_LION3_IP_MLL_TABLE_FIELDS_LAST_1                                      */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_IP_MLL_TABLE_FIELDS_START_OF_TUNNEL_1                           */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_IP_MLL_TABLE_FIELDS_MLL_RPF_FAIL_CMD_1                          */
    STANDARD_FIELD_MAC(3),
/*SMEM_LION3_IP_MLL_TABLE_FIELDS_MLL_EVID_1                                  */
    STANDARD_FIELD_MAC(16),/*was 13*/
/*SMEM_LION3_IP_MLL_TABLE_FIELDS_USE_VIDX_1                                  */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_IP_MLL_TABLE_FIELDS_TARGET_IS_TRUNK_1                           */
    STANDARD_FIELD_MAC(1),
/*SMEM_SIP7_IP_MLL_TABLE_FIELDS_USE_VPORT_1*/
    STANDARD_FIELD_MAC(1),/* not exist */

    /*UseVIDX0 = "True"*/
    /*SMEM_LION3_IP_MLL_TABLE_FIELDS_EVIDX_1                                     */
        {FIELD_SET_IN_RUNTIME_CNS,
         17,/*was 16*/
         SMEM_SIP7_IP_MLL_TABLE_FIELDS_USE_VPORT_1},

    /*UseVIDX0 = "False") and (TargetIsTrunk0 = "False")*/
    /*SMEM_LION3_IP_MLL_TABLE_FIELDS_TRG_EPORT_1                                 */
        {FIELD_SET_IN_RUNTIME_CNS,
         15,/*was 14 */
         SMEM_SIP7_IP_MLL_TABLE_FIELDS_USE_VPORT_1},

    /*(UseVIDX0 = "False") and (TargetIsTrunk0 = "True")*/
    /*SMEM_LION3_IP_MLL_TABLE_FIELDS_TRG_TRUNK_ID_1                              */
        {FIELD_SET_IN_RUNTIME_CNS,
         12,
         SMEM_SIP7_IP_MLL_TABLE_FIELDS_USE_VPORT_1},

/*SMEM_SIP7_IP_MLL_TABLE_FIELDS_VPORT_1*/
        {FIELD_SET_IN_RUNTIME_CNS,
         18,
         SMEM_SIP7_IP_MLL_TABLE_FIELDS_USE_VPORT_1},
/*SMEM_SIP7_IP_MLL_TABLE_FIELDS_DISABLE_MAC_SA_MODIFICATION_1*/
    STANDARD_FIELD_MAC(1),

    /*SMEM_LION3_IP_MLL_TABLE_FIELDS_RESERVED_3*/
    STANDARD_FIELD_MAC(2),

/*(UseVIDX0 = "False") and (TargetIsTrunk0 = "False")*/
/*SMEM_LION3_IP_MLL_TABLE_FIELDS_TRG_DEV_1                                   */
    STANDARD_FIELD_MAC(10),

/*SMEM_LION3_IP_MLL_TABLE_FIELDS_EXCLUDE_SRC_VLAN_1                          */
    EXPLICIT_FIELD_MAC(147,1),
/*SMEM_LION3_IP_MLL_TABLE_FIELDS_TTL_THRESHOLD_1_OR_HOP_LIMIT_THRESHOLD_1    */
    EXPLICIT_FIELD_MAC(139,8),
/*SMEM_LION3_IP_MLL_TABLE_FIELDS_TUNNEL_PTR_1                                */
    {FIELD_SET_IN_RUNTIME_CNS,
     19,/*was 16*/
     SMEM_LION3_IP_MLL_TABLE_FIELDS_EXCLUDE_SRC_VLAN_1},
/*SMEM_LION3_IP_MLL_TABLE_FIELDS_TUNNEL_TYPE_1                               */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_IP_MLL_TABLE_FIELDS_NEXT_MLL_PTR                                */
    STANDARD_FIELD_MAC(17)/*was 16*/
};

/* BOBCAT2 B0 TABLES - Start */
#define SMEM_BOBCAT2_B0_TTI_PHYSICAL_PORT_TABLE_FIELDS_NAME                                            \
     STR(SMEM_LION3_TTI_PHYSICAL_PORT_TABLE_FIELDS_PORT_DEFAULT_SOURCE_EPORT_NUMBER     )\
    ,STR(SMEM_LION3_TTI_PHYSICAL_PORT_TABLE_FIELDS_PORT_IS_RING_CORE_PORT               )\
    ,STR(SMEM_LION3_TTI_PHYSICAL_PORT_TABLE_FIELDS_PORT_IS_LOOPED                       )\
    ,STR(SMEM_LION3_TTI_PHYSICAL_PORT_TABLE_FIELDS_ENABLE_MRU_CHECK_ON_CASCADE_PORT     )\
    ,STR(SMEM_LION3_TTI_PHYSICAL_PORT_TABLE_FIELDS_CC_LINE_CARD_PORTS_DEFAULT_EPORT_BASE)\
    ,STR(SMEM_LION3_TTI_PHYSICAL_PORT_TABLE_FIELDS_CC_LINE_CARD_TRUNK_DEFAULT_EPORT_BASE)\
    ,STR(SMEM_LION3_TTI_PHYSICAL_PORT_TABLE_FIELDS_CENTRALIZED_CHASSIS_PORT_ENABLE      )\
    ,STR(SMEM_LION3_TTI_PHYSICAL_PORT_TABLE_FIELDS_TRUNK_ID                             )\
    ,STR(SMEM_LION3_TTI_PHYSICAL_PORT_TABLE_FIELDS_PORT_QOS_MODE                        )

char * bobcat2B0TtiPhysicalPortTableFieldsNames[
    SMEM_LION3_TTI_PHYSICAL_PORT_TABLE_FIELDS___LAST_VALUE___E] =
    {SMEM_BOBCAT2_B0_TTI_PHYSICAL_PORT_TABLE_FIELDS_NAME};

SNET_ENTRY_FORMAT_TABLE_STC bobcat2B0TtiPhysicalPortTableFieldsFormat[SMEM_LION3_TTI_PHYSICAL_PORT_TABLE_FIELDS___LAST_VALUE___E] =
{
/* SMEM_LION3_TTI_PHYSICAL_PORT_TABLE_FIELDS_PORT_DEFAULT_SOURCE_EPORT_NUMBER */
    STANDARD_FIELD_MAC(13),
/* SMEM_LION3_TTI_PHYSICAL_PORT_TABLE_FIELDS_PORT_IS_RING_CORE_PORT                     */
    STANDARD_FIELD_MAC(0),/* field not exists ! was removed ! but this line must exists !!! */
/* SMEM_LION3_TTI_PHYSICAL_PORT_TABLE_FIELDS_PORT_IS_LOOPED */
    STANDARD_FIELD_MAC(1),
/* SMEM_LION3_TTI_PHYSICAL_PORT_TABLE_FIELDS_ENABLE_MRU_CHECK_ON_CASCADE_PORT */
    STANDARD_FIELD_MAC(1),
/* SMEM_LION3_TTI_PHYSICAL_PORT_TABLE_FIELDS_CC_LINE_CARD_PORTS_DEFAULT_EPORT_BASE */
    STANDARD_FIELD_MAC(13),
/* SMEM_LION3_TTI_PHYSICAL_PORT_TABLE_FIELDS_CC_LINE_CARD_TRUNK_DEFAULT_EPORT_BASE */
    STANDARD_FIELD_MAC(13),
/* SMEM_LION3_TTI_PHYSICAL_PORT_TABLE_FIELDS_CENTRALIZED_CHASSIS_PORT_ENABLE */
    STANDARD_FIELD_MAC(1),
/* SMEM_LION3_TTI_PHYSICAL_PORT_TABLE_FIELDS_TRUNK_ID */
    STANDARD_FIELD_MAC(12),
/* SMEM_LION3_TTI_PHYSICAL_PORT_TABLE_FIELDS_PORT_QOS_MODE */
    STANDARD_FIELD_MAC(1)
};

/* BOBCAT2 B0 TABLES - End */

static SNET_ENTRY_FORMAT_TABLE_STC sip5_20TtiPhysicalPortTableFieldsFormat[SMEM_LION3_TTI_PHYSICAL_PORT_TABLE_FIELDS___LAST_VALUE___E] =
{
/*    SMEM_LION3_TTI_PHYSICAL_PORT_TABLE_FIELDS_PORT_DEFAULT_SOURCE_EPORT_NUMBER            */
    STANDARD_FIELD_MAC(14), /* + 1 from sip 5 */
/*    ,SMEM_LION3_TTI_PHYSICAL_PORT_TABLE_FIELDS_PORT_IS_RING_CORE_PORT                     */
    STANDARD_FIELD_MAC(0),/* field not exists ! was removed ! but this line must exists !!! */
/*    ,SMEM_LION3_TTI_PHYSICAL_PORT_TABLE_FIELDS_PORT_IS_LOOPED                              */
    STANDARD_FIELD_MAC(1),
/*    ,SMEM_LION3_TTI_PHYSICAL_PORT_TABLE_FIELDS_ENABLE_MRU_CHECK_ON_CASCADE_PORT            */
    STANDARD_FIELD_MAC(1),
/*    ,SMEM_LION3_TTI_PHYSICAL_PORT_TABLE_FIELDS_CC_LINE_CARD_PORTS_DEFAULT_EPORT_BASE       */
    STANDARD_FIELD_MAC(14), /* + 1 from sip 5 */
/*    ,SMEM_LION3_TTI_PHYSICAL_PORT_TABLE_FIELDS_CC_LINE_CARD_TRUNK_DEFAULT_EPORT_BASE       */
    STANDARD_FIELD_MAC(14), /* + 1 from sip 5 */
/*    ,SMEM_LION3_TTI_PHYSICAL_PORT_TABLE_FIELDS_CENTRALIZED_CHASSIS_PORT_ENABLE             */
    STANDARD_FIELD_MAC(1), /* + 1 from sip 5 */
/*    ,SMEM_LION3_TTI_PHYSICAL_PORT_TABLE_FIELDS_TRUNK_ID                                    */
    STANDARD_FIELD_MAC(12),
/*    ,SMEM_LION3_TTI_PHYSICAL_PORT_TABLE_FIELDS_PORT_QOS_MODE                               */
    STANDARD_FIELD_MAC(1)
};

#define SMEM_SIP6_10_TTI_PHYSICAL_PORT_TABLE_FIELDS_NAME                                  \
     STR(SMEM_LION3_TTI_PHYSICAL_PORT_TABLE_FIELDS_PORT_DEFAULT_SOURCE_EPORT_NUMBER     )\
    ,STR(SMEM_LION3_TTI_PHYSICAL_PORT_TABLE_FIELDS_PORT_IS_RING_CORE_PORT               )\
    ,STR(SMEM_LION3_TTI_PHYSICAL_PORT_TABLE_FIELDS_PORT_IS_LOOPED                       )\
    ,STR(SMEM_LION3_TTI_PHYSICAL_PORT_TABLE_FIELDS_ENABLE_MRU_CHECK_ON_CASCADE_PORT     )\
    ,STR(SMEM_LION3_TTI_PHYSICAL_PORT_TABLE_FIELDS_CC_LINE_CARD_PORTS_DEFAULT_EPORT_BASE)\
    ,STR(SMEM_LION3_TTI_PHYSICAL_PORT_TABLE_FIELDS_CC_LINE_CARD_TRUNK_DEFAULT_EPORT_BASE)\
    ,STR(SMEM_LION3_TTI_PHYSICAL_PORT_TABLE_FIELDS_CENTRALIZED_CHASSIS_PORT_ENABLE      )\
    ,STR(SMEM_LION3_TTI_PHYSICAL_PORT_TABLE_FIELDS_TRUNK_ID                             )\
    ,STR(SMEM_LION3_TTI_PHYSICAL_PORT_TABLE_FIELDS_PORT_QOS_MODE                        )\
    ,STR(SMEM_SIP6_TTI_PHYSICAL_PORT_TABLE_FIELDS_FLOW_TRACK_ENABLE                     )\
    ,STR(SMEM_SIP6_TTI_PHYSICAL_PORT_TABLE_FIELDS_ACCEPT_EDSA_SKIP_FDB_SA_LOOKUP        )\
    ,STR(SMEM_SIP6_10_TTI_PHYSICAL_PORT_TABLE_FIELDS_PCL_ID_MODE                        )\
    ,STR(SMEM_SIP6_10_TTI_PHYSICAL_PORT_TABLE_FIELDS_EM_PROFILE_ID_MODE                 )\
    ,STR(SMEM_SIP6_10_TTI_PHYSICAL_PORT_TABLE_FIELDS_PCL_ID                             )\
    ,STR(SMEM_SIP6_10_TTI_PHYSICAL_PORT_TABLE_FIELDS_EM_LOOKUP_PROFILE_ID1              )\
    ,STR(SMEM_SIP6_10_TTI_PHYSICAL_PORT_TABLE_FIELDS_EM_LOOKUP_PROFILE_ID2              )\
    ,STR(SMEM_SIP6_10_TTI_PHYSICAL_PORT_TABLE_FIELDS_TCAM_PROFILE_ID_MODE               )\
    ,STR(SMEM_SIP6_10_TTI_PHYSICAL_PORT_TABLE_FIELDS_TCAM_PROFILE_ID                    )\
    ,STR(SMEM_SIP6_10_TTI_PHYSICAL_PORT_TABLE_FIELDS_PPU_PROFILE                        )

char * sip6_10TtiPhysicalPortTableFieldsNames[
    SMEM_SIP6_10_TTI_PHYSICAL_PORT_TABLE_FIELDS___LAST_VALUE___E] =
    {SMEM_SIP6_10_TTI_PHYSICAL_PORT_TABLE_FIELDS_NAME};

static SNET_ENTRY_FORMAT_TABLE_STC sip6_10TtiPhysicalPortTableFieldsFormat[SMEM_SIP6_10_TTI_PHYSICAL_PORT_TABLE_FIELDS___LAST_VALUE___E] =
{
/*    SMEM_LION3_TTI_PHYSICAL_PORT_TABLE_FIELDS_PORT_DEFAULT_SOURCE_EPORT_NUMBER            */
    STANDARD_FIELD_MAC(13), /* -1 from sip 5 */
/*    ,SMEM_LION3_TTI_PHYSICAL_PORT_TABLE_FIELDS_PORT_IS_RING_CORE_PORT                     */
    STANDARD_FIELD_MAC(0),/* field not exists ! was removed ! but this line must exists !!! */
/*    ,SMEM_LION3_TTI_PHYSICAL_PORT_TABLE_FIELDS_PORT_IS_LOOPED                              */
    EXPLICIT_FIELD_MAC(14,1),
/*    ,SMEM_LION3_TTI_PHYSICAL_PORT_TABLE_FIELDS_ENABLE_MRU_CHECK_ON_CASCADE_PORT            */
    STANDARD_FIELD_MAC(1),
/*    ,SMEM_LION3_TTI_PHYSICAL_PORT_TABLE_FIELDS_CC_LINE_CARD_PORTS_DEFAULT_EPORT_BASE       */
    STANDARD_FIELD_MAC(14), /* + 1 from sip 5 */
/*    ,SMEM_LION3_TTI_PHYSICAL_PORT_TABLE_FIELDS_CC_LINE_CARD_TRUNK_DEFAULT_EPORT_BASE       */
    STANDARD_FIELD_MAC(14), /* + 1 from sip 5 */
/*    ,SMEM_LION3_TTI_PHYSICAL_PORT_TABLE_FIELDS_CENTRALIZED_CHASSIS_PORT_ENABLE             */
    STANDARD_FIELD_MAC(1), /* + 1 from sip 5 */
/*    ,SMEM_LION3_TTI_PHYSICAL_PORT_TABLE_FIELDS_TRUNK_ID                                    */
    STANDARD_FIELD_MAC(12),
/*    ,SMEM_LION3_TTI_PHYSICAL_PORT_TABLE_FIELDS_PORT_QOS_MODE                               */
    STANDARD_FIELD_MAC(1),
/*    ,SMEM_SIP6_TTI_PHYSICAL_PORT_TABLE_FIELDS_FLOW_TRACK_ENABLE                            */
    STANDARD_FIELD_MAC(1),
/*    ,SMEM_SIP6_TTI_PHYSICAL_PORT_TABLE_FIELDS_ACCEPT_EDSA_SKIP_FDB_SA_LOOKUP */
    {FIELD_SET_IN_RUNTIME_CNS,
     1,
     SMEM_LION3_TTI_PHYSICAL_PORT_TABLE_FIELDS_PORT_DEFAULT_SOURCE_EPORT_NUMBER},
/*    ,SMEM_SIP6_10_TTI_PHYSICAL_PORT_TABLE_FIELDS_PCL_ID_MODE                               */
    /*    ,SMEM_SIP6_TTI_PHYSICAL_PORT_TABLE_FIELDS_ACCEPT_EDSA_SKIP_FDB_SA_LOOKUP */
    {FIELD_SET_IN_RUNTIME_CNS,
     1,
     SMEM_SIP6_TTI_PHYSICAL_PORT_TABLE_FIELDS_FLOW_TRACK_ENABLE},
/*    ,SMEM_SIP6_10_TTI_PHYSICAL_PORT_TABLE_FIELDS_EM_PROFILE_ID_MODE                        */
    STANDARD_FIELD_MAC(2),
/*    ,SMEM_SIP6_10_TTI_PHYSICAL_PORT_TABLE_FIELDS_PCL_ID                                    */
    STANDARD_FIELD_MAC(10),
/*    ,SMEM_SIP6_10_TTI_PHYSICAL_PORT_TABLE_FIELDS_EM_LOOKUP_PROFILE_ID1                     */
    STANDARD_FIELD_MAC(4),
/*    ,SMEM_SIP6_10_TTI_PHYSICAL_PORT_TABLE_FIELDS_EM_LOOKUP_PROFILE_ID2                     */
    STANDARD_FIELD_MAC(4),
/*    ,SMEM_SIP6_10_TTI_PHYSICAL_PORT_TABLE_FIELDS_TCAM_PROFILE_ID_MODE                      */
    STANDARD_FIELD_MAC(2),
/*    ,SMEM_SIP6_10_TTI_PHYSICAL_PORT_TABLE_FIELDS_TCAM_PROFILE_ID                           */
    STANDARD_FIELD_MAC(6),
/*    ,SMEM_SIP6_10_TTI_PHYSICAL_PORT_TABLE_FIELDS_PPU_PROFILE                               */
    STANDARD_FIELD_MAC(4)

};

#define SMEM_FALCON_TTI_PHYSICAL_PORT_TABLE_FIELDS_NAME                                  \
     STR(SMEM_LION3_TTI_PHYSICAL_PORT_TABLE_FIELDS_PORT_DEFAULT_SOURCE_EPORT_NUMBER     )\
    ,STR(SMEM_LION3_TTI_PHYSICAL_PORT_TABLE_FIELDS_PORT_IS_RING_CORE_PORT               )\
    ,STR(SMEM_LION3_TTI_PHYSICAL_PORT_TABLE_FIELDS_PORT_IS_LOOPED                       )\
    ,STR(SMEM_LION3_TTI_PHYSICAL_PORT_TABLE_FIELDS_ENABLE_MRU_CHECK_ON_CASCADE_PORT     )\
    ,STR(SMEM_LION3_TTI_PHYSICAL_PORT_TABLE_FIELDS_CC_LINE_CARD_PORTS_DEFAULT_EPORT_BASE)\
    ,STR(SMEM_LION3_TTI_PHYSICAL_PORT_TABLE_FIELDS_CC_LINE_CARD_TRUNK_DEFAULT_EPORT_BASE)\
    ,STR(SMEM_LION3_TTI_PHYSICAL_PORT_TABLE_FIELDS_CENTRALIZED_CHASSIS_PORT_ENABLE      )\
    ,STR(SMEM_LION3_TTI_PHYSICAL_PORT_TABLE_FIELDS_TRUNK_ID                             )\
    ,STR(SMEM_LION3_TTI_PHYSICAL_PORT_TABLE_FIELDS_PORT_QOS_MODE                        )\
    ,STR(SMEM_SIP6_TTI_PHYSICAL_PORT_TABLE_FIELDS_FLOW_TRACK_ENABLE                     )\
    ,STR(SMEM_SIP6_TTI_PHYSICAL_PORT_TABLE_FIELDS_ACCEPT_EDSA_SKIP_FDB_SA_LOOKUP        )

char * falconTtiPhysicalPortTableFieldsNames[
    SMEM_SIP6_TTI_PHYSICAL_PORT_TABLE_FIELDS___LAST_VALUE___E] =
    {SMEM_FALCON_TTI_PHYSICAL_PORT_TABLE_FIELDS_NAME};

static SNET_ENTRY_FORMAT_TABLE_STC sip6TtiPhysicalPortTableFieldsFormat[SMEM_SIP6_TTI_PHYSICAL_PORT_TABLE_FIELDS___LAST_VALUE___E] =
{
/*    SMEM_LION3_TTI_PHYSICAL_PORT_TABLE_FIELDS_PORT_DEFAULT_SOURCE_EPORT_NUMBER            */
    STANDARD_FIELD_MAC(13), /* -1 from sip 5 */
/*    ,SMEM_LION3_TTI_PHYSICAL_PORT_TABLE_FIELDS_PORT_IS_RING_CORE_PORT                     */
    STANDARD_FIELD_MAC(0),/* field not exists ! was removed ! but this line must exists !!! */
/*    ,SMEM_LION3_TTI_PHYSICAL_PORT_TABLE_FIELDS_PORT_IS_LOOPED                              */
    EXPLICIT_FIELD_MAC(14,1),
/*    ,SMEM_LION3_TTI_PHYSICAL_PORT_TABLE_FIELDS_ENABLE_MRU_CHECK_ON_CASCADE_PORT            */
    STANDARD_FIELD_MAC(1),
/*    ,SMEM_LION3_TTI_PHYSICAL_PORT_TABLE_FIELDS_CC_LINE_CARD_PORTS_DEFAULT_EPORT_BASE       */
    STANDARD_FIELD_MAC(14), /* + 1 from sip 5 */
/*    ,SMEM_LION3_TTI_PHYSICAL_PORT_TABLE_FIELDS_CC_LINE_CARD_TRUNK_DEFAULT_EPORT_BASE       */
    STANDARD_FIELD_MAC(14), /* + 1 from sip 5 */
/*    ,SMEM_LION3_TTI_PHYSICAL_PORT_TABLE_FIELDS_CENTRALIZED_CHASSIS_PORT_ENABLE             */
    STANDARD_FIELD_MAC(1), /* + 1 from sip 5 */
/*    ,SMEM_LION3_TTI_PHYSICAL_PORT_TABLE_FIELDS_TRUNK_ID                                    */
    STANDARD_FIELD_MAC(12),
/*    ,SMEM_LION3_TTI_PHYSICAL_PORT_TABLE_FIELDS_PORT_QOS_MODE                               */
    STANDARD_FIELD_MAC(1),
/*    ,SMEM_SIP6_TTI_PHYSICAL_PORT_TABLE_FIELDS_FLOW_TRACK_ENABLE                            */
    STANDARD_FIELD_MAC(1),
/*    ,SMEM_SIP6_TTI_PHYSICAL_PORT_TABLE_FIELDS_ACCEPT_EDSA_SKIP_FDB_SA_LOOKUP */
    {FIELD_SET_IN_RUNTIME_CNS,
     1,
     SMEM_LION3_TTI_PHYSICAL_PORT_TABLE_FIELDS_PORT_DEFAULT_SOURCE_EPORT_NUMBER}

};

#define SMEM_SIP5_20_TTI_PHYSICAL_PORT_2_TABLE_FIELDS_NAME                                            \
     STR(SMEM_LION3_TTI_PHYSICAL_PORT_2_TABLE_FIELD_MESSAGE_TYPE_00_DOMAIN_X_PTP_PACKET_COMMAND)\
    ,STR(SMEM_LION3_TTI_PHYSICAL_PORT_2_TABLE_FIELD_MESSAGE_TYPE_01_DOMAIN_X_PTP_PACKET_COMMAND)\
    ,STR(SMEM_LION3_TTI_PHYSICAL_PORT_2_TABLE_FIELD_MESSAGE_TYPE_02_DOMAIN_X_PTP_PACKET_COMMAND)\
    ,STR(SMEM_LION3_TTI_PHYSICAL_PORT_2_TABLE_FIELD_MESSAGE_TYPE_03_DOMAIN_X_PTP_PACKET_COMMAND)\
    ,STR(SMEM_LION3_TTI_PHYSICAL_PORT_2_TABLE_FIELD_MESSAGE_TYPE_04_DOMAIN_X_PTP_PACKET_COMMAND)\
    ,STR(SMEM_LION3_TTI_PHYSICAL_PORT_2_TABLE_FIELD_MESSAGE_TYPE_05_DOMAIN_X_PTP_PACKET_COMMAND)\
    ,STR(SMEM_LION3_TTI_PHYSICAL_PORT_2_TABLE_FIELD_MESSAGE_TYPE_06_DOMAIN_X_PTP_PACKET_COMMAND)\
    ,STR(SMEM_LION3_TTI_PHYSICAL_PORT_2_TABLE_FIELD_MESSAGE_TYPE_07_DOMAIN_X_PTP_PACKET_COMMAND)\
    ,STR(SMEM_LION3_TTI_PHYSICAL_PORT_2_TABLE_FIELD_MESSAGE_TYPE_08_DOMAIN_X_PTP_PACKET_COMMAND)\
    ,STR(SMEM_LION3_TTI_PHYSICAL_PORT_2_TABLE_FIELD_MESSAGE_TYPE_09_DOMAIN_X_PTP_PACKET_COMMAND)\
    ,STR(SMEM_LION3_TTI_PHYSICAL_PORT_2_TABLE_FIELD_MESSAGE_TYPE_10_DOMAIN_X_PTP_PACKET_COMMAND)\
    ,STR(SMEM_LION3_TTI_PHYSICAL_PORT_2_TABLE_FIELD_MESSAGE_TYPE_11_DOMAIN_X_PTP_PACKET_COMMAND)\
    ,STR(SMEM_LION3_TTI_PHYSICAL_PORT_2_TABLE_FIELD_MESSAGE_TYPE_12_DOMAIN_X_PTP_PACKET_COMMAND)\
    ,STR(SMEM_LION3_TTI_PHYSICAL_PORT_2_TABLE_FIELD_MESSAGE_TYPE_13_DOMAIN_X_PTP_PACKET_COMMAND)\
    ,STR(SMEM_LION3_TTI_PHYSICAL_PORT_2_TABLE_FIELD_MESSAGE_TYPE_14_DOMAIN_X_PTP_PACKET_COMMAND)\
    ,STR(SMEM_LION3_TTI_PHYSICAL_PORT_2_TABLE_FIELD_MESSAGE_TYPE_15_DOMAIN_X_PTP_PACKET_COMMAND)\
    ,STR(SMEM_LION3_TTI_PHYSICAL_PORT_2_TABLE_FIELD_PIGGYBACKED_TIMESTAMP_ENABLE               )\
    ,STR(SMEM_LION3_TTI_PHYSICAL_PORT_2_TABLE_FIELD_PORT_LIST_BIT_VECTOR_OFFSET                )\
    ,STR(SMEM_LION3_TTI_PHYSICAL_PORT_2_TABLE_FIELD_PORT_GROUP                                 )\
    ,STR(SMEM_LION3_TTI_PHYSICAL_PORT_2_TABLE_FIELD_EXTRACT_HASH_FROM_FORWARD_EDSA             )

static char * sip5_20TtiPhysicalPort2TableFieldsNames[
    SMEM_LION3_TTI_PHYSICAL_PORT_2_TABLE_FIELDS___LAST_VALUE___E] =
    {SMEM_SIP5_20_TTI_PHYSICAL_PORT_2_TABLE_FIELDS_NAME};

static SNET_ENTRY_FORMAT_TABLE_STC sip5_20TtiPhysicalPort2TableFieldsFormat[SMEM_LION3_TTI_PHYSICAL_PORT_2_TABLE_FIELDS___LAST_VALUE___E] =
{
     STANDARD_FIELD_MAC(15) /*  SMEM_LION3_TTI_PHYSICAL_PORT_2_TABLE_FIELD_MESSAGE_TYPE_00_DOMAIN_X_PTP_PACKET_COMMAND */
    ,STANDARD_FIELD_MAC(15) /*  SMEM_LION3_TTI_PHYSICAL_PORT_2_TABLE_FIELD_MESSAGE_TYPE_01_DOMAIN_X_PTP_PACKET_COMMAND */
    ,STANDARD_FIELD_MAC(15) /*  SMEM_LION3_TTI_PHYSICAL_PORT_2_TABLE_FIELD_MESSAGE_TYPE_02_DOMAIN_X_PTP_PACKET_COMMAND */
    ,STANDARD_FIELD_MAC(15) /*  SMEM_LION3_TTI_PHYSICAL_PORT_2_TABLE_FIELD_MESSAGE_TYPE_03_DOMAIN_X_PTP_PACKET_COMMAND */
    ,STANDARD_FIELD_MAC(15) /*  SMEM_LION3_TTI_PHYSICAL_PORT_2_TABLE_FIELD_MESSAGE_TYPE_04_DOMAIN_X_PTP_PACKET_COMMAND */
    ,STANDARD_FIELD_MAC(15) /*  SMEM_LION3_TTI_PHYSICAL_PORT_2_TABLE_FIELD_MESSAGE_TYPE_05_DOMAIN_X_PTP_PACKET_COMMAND */
    ,STANDARD_FIELD_MAC(15) /*  SMEM_LION3_TTI_PHYSICAL_PORT_2_TABLE_FIELD_MESSAGE_TYPE_06_DOMAIN_X_PTP_PACKET_COMMAND */
    ,STANDARD_FIELD_MAC(15) /*  SMEM_LION3_TTI_PHYSICAL_PORT_2_TABLE_FIELD_MESSAGE_TYPE_07_DOMAIN_X_PTP_PACKET_COMMAND */
    ,STANDARD_FIELD_MAC(15) /*  SMEM_LION3_TTI_PHYSICAL_PORT_2_TABLE_FIELD_MESSAGE_TYPE_08_DOMAIN_X_PTP_PACKET_COMMAND */
    ,STANDARD_FIELD_MAC(15) /*  SMEM_LION3_TTI_PHYSICAL_PORT_2_TABLE_FIELD_MESSAGE_TYPE_09_DOMAIN_X_PTP_PACKET_COMMAND */
    ,STANDARD_FIELD_MAC(15) /*  SMEM_LION3_TTI_PHYSICAL_PORT_2_TABLE_FIELD_MESSAGE_TYPE_10_DOMAIN_X_PTP_PACKET_COMMAND */
    ,STANDARD_FIELD_MAC(15) /*  SMEM_LION3_TTI_PHYSICAL_PORT_2_TABLE_FIELD_MESSAGE_TYPE_11_DOMAIN_X_PTP_PACKET_COMMAND */
    ,STANDARD_FIELD_MAC(15) /*  SMEM_LION3_TTI_PHYSICAL_PORT_2_TABLE_FIELD_MESSAGE_TYPE_12_DOMAIN_X_PTP_PACKET_COMMAND */
    ,STANDARD_FIELD_MAC(15) /*  SMEM_LION3_TTI_PHYSICAL_PORT_2_TABLE_FIELD_MESSAGE_TYPE_13_DOMAIN_X_PTP_PACKET_COMMAND */
    ,STANDARD_FIELD_MAC(15) /*  SMEM_LION3_TTI_PHYSICAL_PORT_2_TABLE_FIELD_MESSAGE_TYPE_14_DOMAIN_X_PTP_PACKET_COMMAND */
    ,STANDARD_FIELD_MAC(15) /*  SMEM_LION3_TTI_PHYSICAL_PORT_2_TABLE_FIELD_MESSAGE_TYPE_15_DOMAIN_X_PTP_PACKET_COMMAND */

   /*SMEM_LION3_TTI_PHYSICAL_PORT_2_TABLE_FIELD_PIGGYBACKED_TIMESTAMP_ENABLE    */
   ,STANDARD_FIELD_MAC(1)
   /*SMEM_LION3_TTI_PHYSICAL_PORT_2_TABLE_FIELD_PORT_LIST_BIT_VECTOR_OFFSET     */
   ,STANDARD_FIELD_MAC(5)
   /*SMEM_LION3_TTI_PHYSICAL_PORT_2_TABLE_FIELD_PORT_GROUP                      */
   ,STANDARD_FIELD_MAC(4)
   /*SMEM_LION3_TTI_PHYSICAL_PORT_2_TABLE_FIELD_EXTRACT_HASH_FROM_FORWARD_EDSA  */
   ,STANDARD_FIELD_MAC(1)
};


#define SMEM_SIP5_20_EGF_QAG_PORT_SOURCE_ATTRIBUTES_TABLE_FIELDS_NAME                                          \
     STR(SMEM_SIP5_20_EGF_QAG_PORT_SOURCE_ATTRIBUTES_TABLE_FIELDS_SOURCE_PHYSICAL_PORT_LOOPBACK_PROFILE_E)     \
    ,STR(SMEM_SIP5_20_EGF_QAG_PORT_SOURCE_ATTRIBUTES_TABLE_FIELDS_SOURCE_PHYSICAL_PORT_TC_PROFILE_E)           \
    ,STR(SMEM_SIP6_EGF_QAG_PORT_SOURCE_ATTRIBUTES_TABLE_FIELDS_SOURCE_PHYSICAL_PORT_SPEED_E)

static char * sip5_20EgfQagPortSourceAttributesTableFieldsNames[
    SMEM_SIP5_20_EGF_QAG_PORT_SOURCE_ATTRIBUTES_TABLE_FIELDS___LAST_VALUE___E] = {
    SMEM_SIP5_20_EGF_QAG_PORT_SOURCE_ATTRIBUTES_TABLE_FIELDS_NAME};

static SNET_ENTRY_FORMAT_TABLE_STC sip5_20EgfQagPortSourceAttributesTableFieldsFormat[SMEM_SIP5_20_EGF_QAG_PORT_SOURCE_ATTRIBUTES_TABLE_FIELDS___LAST_VALUE___E] =
{
     /* SMEM_SIP5_20_EGF_QAG_PORT_SOURCE_ATTRIBUTES_TABLE_FIELDS_SOURCE_PHYSICAL_PORT_LOOPBACK_PROFILE_E */
    STANDARD_FIELD_MAC(2)
    /* SMEM_SIP5_20_EGF_QAG_PORT_SOURCE_ATTRIBUTES_TABLE_FIELDS_SOURCE_PHYSICAL_PORT_TC_PROFILE_E */
   ,STANDARD_FIELD_MAC(2)
    /* SMEM_SIP6_EGF_QAG_PORT_SOURCE_ATTRIBUTES_TABLE_FIELDS_SOURCE_PHYSICAL_PORT_SPEED_E */
   ,STANDARD_FIELD_MAC(5)
};

#define SMEM_SIP5_20_EGRESS_EGF_QAG_TARGET_PORT_MAPPER_TABLE_FIELDS_NAME                \
     STR(SMEM_SIP5_20_EGRESS_EGF_QAG_TARGET_PORT_MAPPER_TABLE_FIELDS_PORT_BASE_E)       \
    ,STR(SMEM_SIP6_EGRESS_EGF_QAG_TARGET_PORT_MAPPER_TABLE_FIELDS_PORT_SPEED_E)         \
    ,STR(SMEM_SIP7_EGRESS_EGF_QAG_TARGET_PORT_MAPPER_TABLE_FIELDS_UNKNOWN_CT_ENABLE_E)  \
    ,STR(SMEM_SIP7_EGRESS_EGF_QAG_TARGET_PORT_MAPPER_TABLE_FIELDS_QUEUE_GROUP_INDEX_E)  \
    ,STR(SMEM_SIP7_EGRESS_EGF_QAG_TARGET_PORT_MAPPER_TABLE_FIELDS_DP_CORE_E          )  \
    ,STR(SMEM_SIP7_EGRESS_EGF_QAG_TARGET_PORT_MAPPER_TABLE_FIELDS_SET_Q_MODE_E       )  \
    ,STR(SMEM_SIP7_EGRESS_EGF_QAG_TARGET_PORT_MAPPER_TABLE_FIELDS_TC_PROFILE_E       )  \
    ,STR(SMEM_SIP7_EGRESS_EGF_QAG_TARGET_PORT_MAPPER_TABLE_FIELDS_SET_TC_PROF_MODE_E )  \
    ,STR(SMEM_SIP7_EGRESS_EGF_QAG_TARGET_PORT_MAPPER_TABLE_FIELDS_DESC_TC_PROF_FIRST_BIT_E) \
    ,STR(SMEM_SIP7_EGRESS_EGF_QAG_TARGET_PORT_MAPPER_TABLE_FIELDS_DESC_TC_PROF_LAST_BIT_E ) \
    ,STR(SMEM_SIP7_EGRESS_EGF_QAG_TARGET_PORT_MAPPER_TABLE_FIELDS_TRG_IS_CASCADE_E        )


static char * sip5_20EgfQagTargetPortMapperTableFieldsNames[
    SMEM_SIP5_20_EGRESS_EGF_QAG_TARGET_PORT_MAPPER_TABLE_FIELDS___LAST_VALUE___E] = {
    SMEM_SIP5_20_EGRESS_EGF_QAG_TARGET_PORT_MAPPER_TABLE_FIELDS_NAME};

static SNET_ENTRY_FORMAT_TABLE_STC sip5_20EgfQagTargetPortMapperTableFieldsFormat[
    SMEM_SIP5_20_EGRESS_EGF_QAG_TARGET_PORT_MAPPER_TABLE_FIELDS___LAST_VALUE___E] =
{
     /* SMEM_SIP5_20_EGRESS_EGF_QAG_TARGET_PORT_MAPPER_TABLE_FIELDS_PORT_BASE_E */
    STANDARD_FIELD_MAC(10)
    /* SMEM_SIP6_EGRESS_EGF_QAG_TARGET_PORT_MAPPER_TABLE_FIELDS_PORT_SPEED_E */
   ,STANDARD_FIELD_MAC(5)
  /*  ,SMEM_SIP7_EGRESS_EGF_QAG_TARGET_PORT_MAPPER_TABLE_FIELDS_UNKNOWN_CT_ENABLE_E    */
      ,STANDARD_FIELD_MAC(0)
  /*  ,SMEM_SIP7_EGRESS_EGF_QAG_TARGET_PORT_MAPPER_TABLE_FIELDS_QUEUE_GROUP_INDEX_E    */
    ,STANDARD_FIELD_MAC(0)
  /*  ,SMEM_SIP7_EGRESS_EGF_QAG_TARGET_PORT_MAPPER_TABLE_FIELDS_DP_CORE_E              */
      ,STANDARD_FIELD_MAC(0)
  /*  ,SMEM_SIP7_EGRESS_EGF_QAG_TARGET_PORT_MAPPER_TABLE_FIELDS_SET_Q_MODE_E           */
    ,STANDARD_FIELD_MAC(0)
  /*  ,SMEM_SIP7_EGRESS_EGF_QAG_TARGET_PORT_MAPPER_TABLE_FIELDS_TC_PROFILE_E           */
      ,STANDARD_FIELD_MAC(0)
  /*  ,SMEM_SIP7_EGRESS_EGF_QAG_TARGET_PORT_MAPPER_TABLE_FIELDS_SET_TC_PROF_MODE_E     */
    ,STANDARD_FIELD_MAC(0)
  /*  ,SMEM_SIP7_EGRESS_EGF_QAG_TARGET_PORT_MAPPER_TABLE_FIELDS_DESC_TC_PROF_FIRST_BIT_E  */
      ,STANDARD_FIELD_MAC(0)
  /*  ,SMEM_SIP7_EGRESS_EGF_QAG_TARGET_PORT_MAPPER_TABLE_FIELDS_DESC_TC_PROF_LAST_BIT_E   */
    ,STANDARD_FIELD_MAC(0)
  /*  ,SMEM_SIP7_EGRESS_EGF_QAG_TARGET_PORT_MAPPER_TABLE_FIELDS_TRG_IS_CASCADE_E      */
      ,STANDARD_FIELD_MAC(0)
};

#define SMEM_SIP5_20_EGF_QAG_PORT_TARGET_ATTRIBUTES_TABLE_FIELDS_NAME                                          \
     STR(SMEM_SIP5_20_EGF_QAG_PORT_TARGET_ATTRIBUTES_TABLE_FIELDS_TARGET_PHYSICAL_PORT_LOOPBACK_PROFILE_E      )\
    ,STR(SMEM_SIP5_20_EGF_QAG_PORT_TARGET_ATTRIBUTES_TABLE_FIELDS_TARGET_PHYSICAL_PORT_TC_PROFILE_E            )\
    ,STR(SMEM_SIP5_20_EGF_QAG_PORT_TARGET_ATTRIBUTES_TABLE_FIELDS_TARGET_PHYSICAL_PORT_EVLAN_MIRRORING_ENABLE_E)\
    ,STR(SMEM_SIP5_20_EGF_QAG_PORT_TARGET_ATTRIBUTES_TABLE_FIELDS_USE_VLAN_TAG_1_FOR_TAG_STATE_E               )\
    ,STR(SMEM_SIP5_20_EGF_QAG_PORT_TARGET_ATTRIBUTES_TABLE_FIELDS_TARGET_PHYSICAL_PORT_ENQ_PROFILE_E           )\
    ,STR(SMEM_SIP5_20_EGF_QAG_PORT_TARGET_ATTRIBUTES_TABLE_FIELDS_LOOPBACK_ENQ_PROFILE_E                       )\
    ,STR(SMEM_SIP5_20_EGF_QAG_PORT_TARGET_ATTRIBUTES_TABLE_FIELDS_LOOPBACK_PORT_E                              )

static char * sip5_20EgfQagPortTargetAttributesTableFieldsNames[
    SMEM_SIP5_20_EGF_QAG_PORT_TARGET_ATTRIBUTES_TABLE_FIELDS___LAST_VALUE___E] =
    {SMEM_SIP5_20_EGF_QAG_PORT_TARGET_ATTRIBUTES_TABLE_FIELDS_NAME};

static SNET_ENTRY_FORMAT_TABLE_STC sip5_20EgfQagPortTargetAttributesTableFieldsFormat[SMEM_SIP5_20_EGF_QAG_PORT_TARGET_ATTRIBUTES_TABLE_FIELDS___LAST_VALUE___E] =
{
    /*SMEM_SIP5_20_EGF_QAG_PORT_TARGET_ATTRIBUTES_TABLE_FIELDS_TARGET_PHYSICAL_PORT_LOOPBACK_PROFILE_E       */
    STANDARD_FIELD_MAC(2)
    /*SMEM_SIP5_20_EGF_QAG_PORT_TARGET_ATTRIBUTES_TABLE_FIELDS_TARGET_PHYSICAL_PORT_TC_PROFILE_E             */
   ,STANDARD_FIELD_MAC(2)
    /*SMEM_SIP5_20_EGF_QAG_PORT_TARGET_ATTRIBUTES_TABLE_FIELDS_TARGET_PHYSICAL_PORT_EVLAN_MIRRORING_ENABLE_E */
   ,STANDARD_FIELD_MAC(1)
    /*SMEM_SIP5_20_EGF_QAG_PORT_TARGET_ATTRIBUTES_TABLE_FIELDS_USE_VLAN_TAG_1_FOR_TAG_STATE_E                */
   ,STANDARD_FIELD_MAC(1)
    /*SMEM_SIP5_20_EGF_QAG_PORT_TARGET_ATTRIBUTES_TABLE_FIELDS_TARGET_PHYSICAL_PORT_ENQ_PROFILE_E            */
   ,STANDARD_FIELD_MAC(1)
    /*SMEM_SIP5_20_EGF_QAG_PORT_TARGET_ATTRIBUTES_TABLE_FIELDS_LOOPBACK_ENQ_PROFILE_E                        */
   ,STANDARD_FIELD_MAC(1)
    /*SMEM_SIP5_20_EGF_QAG_PORT_TARGET_ATTRIBUTES_TABLE_FIELDS_LOOPBACK_PORT_E                               */
   ,STANDARD_FIELD_MAC(9)
};

static SNET_ENTRY_FORMAT_TABLE_STC sip6EgfQagPortTargetAttributesTableFieldsFormat[SMEM_SIP5_20_EGF_QAG_PORT_TARGET_ATTRIBUTES_TABLE_FIELDS___LAST_VALUE___E] =
{
    /*SMEM_SIP5_20_EGF_QAG_PORT_TARGET_ATTRIBUTES_TABLE_FIELDS_TARGET_PHYSICAL_PORT_LOOPBACK_PROFILE_E       */
    STANDARD_FIELD_MAC(2)
    /*SMEM_SIP5_20_EGF_QAG_PORT_TARGET_ATTRIBUTES_TABLE_FIELDS_TARGET_PHYSICAL_PORT_TC_PROFILE_E             */
   ,STANDARD_FIELD_MAC(4)/*changed in sip6 to 4 */
    /*SMEM_SIP5_20_EGF_QAG_PORT_TARGET_ATTRIBUTES_TABLE_FIELDS_TARGET_PHYSICAL_PORT_EVLAN_MIRRORING_ENABLE_E */
   ,STANDARD_FIELD_MAC(1)
    /*SMEM_SIP5_20_EGF_QAG_PORT_TARGET_ATTRIBUTES_TABLE_FIELDS_USE_VLAN_TAG_1_FOR_TAG_STATE_E                */
   ,STANDARD_FIELD_MAC(1)
    /*SMEM_SIP5_20_EGF_QAG_PORT_TARGET_ATTRIBUTES_TABLE_FIELDS_TARGET_PHYSICAL_PORT_ENQ_PROFILE_E            */
   ,STANDARD_FIELD_MAC(1)
    /*SMEM_SIP5_20_EGF_QAG_PORT_TARGET_ATTRIBUTES_TABLE_FIELDS_LOOPBACK_ENQ_PROFILE_E                        */
   ,STANDARD_FIELD_MAC(1)
    /*SMEM_SIP5_20_EGF_QAG_PORT_TARGET_ATTRIBUTES_TABLE_FIELDS_LOOPBACK_PORT_E                               */
   ,STANDARD_FIELD_MAC(10)
};


#define SMEM_SIP5_EGF_QAG_CPU_CODE_TO_LB_MAPPER_TABLE_FIELDS_NAME                     \
     STR(SMEM_SIP5_EGF_QAG_CPU_CODE_TO_LB_MAPPER_TABLE_FIELDS_LOOPBACK_PORT_E)        \
    ,STR(SMEM_SIP5_EGF_QAG_CPU_CODE_TO_LB_MAPPER_TABLE_FIELDS_TXQ_PORT_E)             \
    ,STR(SMEM_SIP5_EGF_QAG_CPU_CODE_TO_LB_MAPPER_TABLE_FIELDS_LOOPBACK_ENQ_PROFILE_E) \
    ,STR(SMEM_SIP5_EGF_QAG_CPU_CODE_TO_LB_MAPPER_TABLE_FIELDS_LOOPBACK_ENABLE_E)

static char * sip5EgfQagCpuCodeToLbMapperTableFieldsNames[SMEM_SIP5_EGF_QAG_CPU_CODE_TO_LB_MAPPER_TABLE_FIELDS___LAST_VALUE___E] = {SMEM_SIP5_EGF_QAG_CPU_CODE_TO_LB_MAPPER_TABLE_FIELDS_NAME};

static SNET_ENTRY_FORMAT_TABLE_STC sip5EgfQagCpuCodeToLbMapperTableFieldsFormat[SMEM_SIP5_EGF_QAG_CPU_CODE_TO_LB_MAPPER_TABLE_FIELDS___LAST_VALUE___E] =
{
    /* SMEM_SIP5_EGF_QAG_CPU_CODE_TO_LB_MAPPER_TABLE_FIELDS_LOOPBACK_PORT_E) */
    STANDARD_FIELD_MAC(8),
    /* SMEM_SIP5_EGF_QAG_CPU_CODE_TO_LB_MAPPER_TABLE_FIELDS_TXQ_PORT_E) */
    STANDARD_FIELD_MAC(8),
    /* SMEM_SIP5_EGF_QAG_CPU_CODE_TO_LB_MAPPER_TABLE_FIELDS_LOOPBACK_ENQ_PROFILE_E) */
    STANDARD_FIELD_MAC(0),      /* doesn't exists */
    /* SMEM_SIP5_EGF_QAG_CPU_CODE_TO_LB_MAPPER_TABLE_FIELDS_LOOPBACK_ENABLE_E) */
    STANDARD_FIELD_MAC(1)
};


static SNET_ENTRY_FORMAT_TABLE_STC sip5_20EgfQagCpuCodeToLbMapperTableFieldsFormat[SMEM_SIP5_EGF_QAG_CPU_CODE_TO_LB_MAPPER_TABLE_FIELDS___LAST_VALUE___E] =
{
    /* SMEM_SIP5_EGF_QAG_CPU_CODE_TO_LB_MAPPER_TABLE_FIELDS_LOOPBACK_PORT_E) */
    EXPLICIT_FIELD_MAC(2, 9),
    /* SMEM_SIP5_EGF_QAG_CPU_CODE_TO_LB_MAPPER_TABLE_FIELDS_TXQ_PORT_E) */
    EXPLICIT_FIELD_MAC(1,0),  /* doesn't exists */
    /* SMEM_SIP5_EGF_QAG_CPU_CODE_TO_LB_MAPPER_TABLE_FIELDS_LOOPBACK_ENQ_PROFILE_E) */
    EXPLICIT_FIELD_MAC(1,1),
    /* SMEM_SIP5_EGF_QAG_CPU_CODE_TO_LB_MAPPER_TABLE_FIELDS_LOOPBACK_ENABLE_E) */
    EXPLICIT_FIELD_MAC(0,1)
};

#define SMEM_SIP6_30_SMU_IRF_SNG_TABLE_FIELDS_NAME                                  \
     STR(SMEM_SIP6_30_SMU_IRF_SNG_TABLE_FIELDS_IRF_INDIVIDUAL_RECOVERY_ENABLED_E)   \
    ,STR(SMEM_SIP6_30_SMU_IRF_SNG_TABLE_FIELDS_IRF_RECOVERY_SEQUENCE_NUM_E)         \
    ,STR(SMEM_SIP6_30_SMU_IRF_SNG_TABLE_FIELDS_IRF_TAKE_ANY_E)                      \
    ,STR(SMEM_SIP6_30_SMU_IRF_SNG_TABLE_FIELDS_SNG_SEQUENCE_NUMBER_ASSIGNMENT_E)    \
    ,STR(SMEM_SIP6_30_SMU_IRF_SNG_TABLE_FIELDS_SNG_RESET_PACKET_COUNTER_E)          \
    ,STR(SMEM_SIP6_30_SMU_IRF_SNG_TABLE_FIELDS_SNG_PACKET_COUNTER_E)

static char * sip6_30SmuIrfSngTableFieldsNames[SMEM_SIP6_30_SMU_IRF_SNG_TABLE_FIELDS___LAST_VALUE___E] =
    {SMEM_SIP6_30_SMU_IRF_SNG_TABLE_FIELDS_NAME};

static SNET_ENTRY_FORMAT_TABLE_STC sip6_30SmuIrfSngTableFieldsFormat[SMEM_SIP6_30_SMU_IRF_SNG_TABLE_FIELDS___LAST_VALUE___E] =
{
/* SMEM_SIP6_30_SMU_IRF_SNG_TABLE_FIELDS_IRF_INDIVIDUAL_RECOVERY_ENABLED_E*/
    STANDARD_FIELD_MAC(1),
/* SMEM_SIP6_30_SMU_IRF_SNG_TABLE_FIELDS_IRF_RECOVERY_SEQUENCE_NUM_E*/
    STANDARD_FIELD_MAC(16),
/* SMEM_SIP6_30_SMU_IRF_SNG_TABLE_FIELDS_IRF_TAKE_ANY_E*/
    STANDARD_FIELD_MAC(1),
/* SMEM_SIP6_30_SMU_IRF_SNG_TABLE_FIELDS_SNG_SEQUENCE_NUMBER_ASSIGNMENT_E*/
    STANDARD_FIELD_MAC(1),
/* SMEM_SIP6_30_SMU_IRF_SNG_TABLE_FIELDS_SNG_RESET_PACKET_COUNTER_E*/
    STANDARD_FIELD_MAC(1),
/* SMEM_SIP6_30_SMU_IRF_SNG_TABLE_FIELDS_SNG_PACKET_COUNTER_E*/
    STANDARD_FIELD_MAC(16)
};


#define SMEM_SIP6_30_SMU_IRF_COUNTERS_TABLE_FIELDS_NAME                                      \
     STR(SMEM_SIP6_30_SMU_IRF_COUNTERS_TABLE_FIELDS_IRF_PASSED_PACKETS_COUNTER_E)            \
    ,STR(SMEM_SIP6_30_SMU_IRF_COUNTERS_TABLE_FIELDS_IRF_DISCARDED_PACKETS_COUNTER_E)         \
    ,STR(SMEM_SIP6_30_SMU_IRF_COUNTERS_TABLE_FIELDS_IRF_OUT_OF_ORDER_PACKETS_COUNTER_E)      \
    ,STR(SMEM_SIP6_30_SMU_IRF_COUNTERS_TABLE_FIELDS_IRF_NUMBER_OF_TAGLESS_PACKETS_COUNTER_E) \
    ,STR(SMEM_SIP6_30_SMU_IRF_COUNTERS_TABLE_FIELDS_IRF_BYTE_COUNT_E)

static char * sip6_30SmuIrfCountersTableFieldsNames[SMEM_SIP6_30_SMU_IRF_COUNTERS_TABLE_FIELDS___LAST_VALUE___E] =
    {SMEM_SIP6_30_SMU_IRF_COUNTERS_TABLE_FIELDS_NAME};

static SNET_ENTRY_FORMAT_TABLE_STC sip6_30SmuIrfCountersTableFieldsFormat[SMEM_SIP6_30_SMU_IRF_COUNTERS_TABLE_FIELDS___LAST_VALUE___E] =
{
/* SMEM_SIP6_30_SMU_IRF_COUNTERS_TABLE_FIELDS_IRF_PASSED_PACKETS_COUNTER_E*/
    STANDARD_FIELD_MAC(32),
/* SMEM_SIP6_30_SMU_IRF_COUNTERS_TABLE_FIELDS_IRF_DISCARDED_PACKETS_COUNTER_E*/
    STANDARD_FIELD_MAC(12),
/* SMEM_SIP6_30_SMU_IRF_COUNTERS_TABLE_FIELDS_IRF_OUT_OF_ORDER_PACKETS_COUNTER_E*/
    STANDARD_FIELD_MAC(32),
/* SMEM_SIP6_30_SMU_IRF_COUNTERS_TABLE_FIELDS_IRF_NUMBER_OF_TAGLESS_PACKETS_COUNTER_E*/
    STANDARD_FIELD_MAC(32),
/* SMEM_SIP6_30_SMU_IRF_COUNTERS_TABLE_FIELDS_IRF_BYTE_COUNT_E*/
    STANDARD_FIELD_MAC(38)
};

#define SMEM_SIP6_30_SMU_SGC_INTERVAL_MAX_TABLE_FIELDS_NAME                                  \
     STR(SMEM_SIP6_30_SMU_SGC_INTERVAL_MAX_TABLE_FIELDS_SGC_INTERVAL_MAX_E)

static char * sip6_30SmuSgcIntervalMaxTableFieldsNames[SMEM_SIP6_30_SMU_SGC_INTERVAL_MAX_TABLE_FIELDS___LAST_VALUE___E] =
    {SMEM_SIP6_30_SMU_SGC_INTERVAL_MAX_TABLE_FIELDS_NAME};

static SNET_ENTRY_FORMAT_TABLE_STC sip6_30SmuSgcIntervalMaxTableFieldsFormat[SMEM_SIP6_30_SMU_SGC_INTERVAL_MAX_TABLE_FIELDS___LAST_VALUE___E] =
{
/* SMEM_SIP6_30_SMU_SGC_INTERVAL_MAX_TABLE_FIELDS_SGC_INTERVAL_MAX_E*/
    STANDARD_FIELD_MAC(32)
};

#define SMEM_SIP6_30_SMU_SGC_OCTET_COUNTERS_TABLE_FIELDS_NAME                                  \
     STR(SMEM_SIP6_30_SMU_SGC_OCTET_COUNTERS_TABLE_FIELDS_SGC_OCTET_COUNTER_E)

static char * sip6_30SmuSgcOctetCountersTableFieldsNames[SMEM_SIP6_30_SMU_SGC_OCTET_COUNTERS_TABLE_FIELDS___LAST_VALUE___E] =
    {SMEM_SIP6_30_SMU_SGC_OCTET_COUNTERS_TABLE_FIELDS_NAME};

static SNET_ENTRY_FORMAT_TABLE_STC sip6_30SmuSgcOctetCountersTableFieldsFormat[SMEM_SIP6_30_SMU_SGC_OCTET_COUNTERS_TABLE_FIELDS___LAST_VALUE___E] =
{
/* SMEM_SIP6_30_SMU_SGC_OCTET_COUNTERS_TABLE_FIELDS_SGC_OCTET_COUNTER_E*/
    STANDARD_FIELD_MAC(32)
};

#define SMEM_SIP6_30_SMU_SGC_TABLE_SET_TIME_CONFIGURATIONS_TABLE_FIELDS_NAME                            \
     STR(SMEM_SIP6_30_SMU_TABLE_SET_TIME_CONFIGURATIONS_TABLE_FIELDS_SGC_INGRESS_TOD_OFFSET_E)          \
    ,STR(SMEM_SIP6_30_SMU_TABLE_SET_TIME_CONFIGURATIONS_TABLE_FIELDS_SGC_INGRESS_TIMESTAMP_OFFSET_E)    \
    ,STR(SMEM_SIP6_30_SMU_TABLE_SET_TIME_CONFIGURATIONS_TABLE_FIELDS_SGC_CYCLE_TIME_E)                  \
    ,STR(SMEM_SIP6_30_SMU_TABLE_SET_TIME_CONFIGURATIONS_TABLE_FIELDS_SGC_REMAINING_BITS_FACTOR_E)       \
    ,STR(SMEM_SIP6_30_SMU_TABLE_SET_TIME_CONFIGURATIONS_TABLE_FIELDS_SGC_BYTE_COUNT_ADJUST_E)           \
    ,STR(SMEM_SIP6_30_SMU_TABLE_SET_TIME_CONFIGURATIONS_TABLE_FIELDS_SGC_BYTE_COUNT_ADJUST_POLARITY_E)  \
    ,STR(SMEM_SIP6_30_SMU_TABLE_SET_TIME_CONFIGURATIONS_TABLE_FIELDS_SGC_REMAINING_BITS_RESOLUTION_E)   \

static char * sip6_30SmuSgcTableSetTimeConfigurationsTableFieldsNames[SMEM_SIP6_30_SMU_SGC_TABLE_SET_TIME_CONFIGURATIONS_TABLE_FIELDS___LAST_VALUE___E] =
    {SMEM_SIP6_30_SMU_SGC_TABLE_SET_TIME_CONFIGURATIONS_TABLE_FIELDS_NAME};

static SNET_ENTRY_FORMAT_TABLE_STC sip6_30SmuSgcTableSetTimeConfigurationsTableFieldsFormat[SMEM_SIP6_30_SMU_SGC_TABLE_SET_TIME_CONFIGURATIONS_TABLE_FIELDS___LAST_VALUE___E] =
{
/* SMEM_SIP6_30_SMU_TABLE_SET_TIME_CONFIGURATIONS_TABLE_FIELDS_SGC_INGRESS_TOD_OFFSET_E*/
    STANDARD_FIELD_MAC(30),
/* SMEM_SIP6_30_SMU_TABLE_SET_TIME_CONFIGURATIONS_TABLE_FIELDS_SGC_INGRESS_TIMESTAMP_OFFSET_E*/
    STANDARD_FIELD_MAC(30),
/* SMEM_SIP6_30_SMU_TABLE_SET_TIME_CONFIGURATIONS_TABLE_FIELDS_SGC_CYCLE_TIME_E*/
    STANDARD_FIELD_MAC(32),
/* SMEM_SIP6_30_SMU_TABLE_SET_TIME_CONFIGURATIONS_TABLE_FIELDS_SGC_REMAINING_BITS_FACTOR_E*/
    STANDARD_FIELD_MAC(8),
/* SMEM_SIP6_30_SMU_TABLE_SET_TIME_CONFIGURATIONS_TABLE_FIELDS_SGC_BYTE_COUNT_ADJUST_E*/
    STANDARD_FIELD_MAC(7),
/* SMEM_SIP6_30_SMU_TABLE_SET_TIME_CONFIGURATIONS_TABLE_FIELDS_SGC_BYTE_COUNT_ADJUST_POLARITY_E*/
    STANDARD_FIELD_MAC(1),
/* SMEM_SIP6_30_SMU_TABLE_SET_TIME_CONFIGURATIONS_TABLE_FIELDS_SGC_REMAINING_BITS_RESOLUTION_E*/
    STANDARD_FIELD_MAC(2),
};

#define SMEM_SIP6_30_SMU_SGC_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_NAME                    \
     STR(SMEM_SIP6_30_SMU_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_SGC_NEW_SLOT_0_E)          \
    ,STR(SMEM_SIP6_30_SMU_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_SGC_STREAM_GATE_STATE_0_E) \
    ,STR(SMEM_SIP6_30_SMU_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_SGC_LENGTH_AWARE_0_E)      \
    ,STR(SMEM_SIP6_30_SMU_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_SGC_IPV_0_E)               \
    ,STR(SMEM_SIP6_30_SMU_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_SGC_IMX_PROFILE_0_E)       \
    ,STR(SMEM_SIP6_30_SMU_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_SGC_NEW_SLOT_1_E)          \
    ,STR(SMEM_SIP6_30_SMU_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_SGC_STREAM_GATE_STATE_1_E) \
    ,STR(SMEM_SIP6_30_SMU_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_SGC_LENGTH_AWARE_1_E)      \
    ,STR(SMEM_SIP6_30_SMU_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_SGC_IPV_1_E)               \
    ,STR(SMEM_SIP6_30_SMU_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_SGC_IMX_PROFILE_1_E)       \
    ,STR(SMEM_SIP6_30_SMU_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_SGC_NEW_SLOT_2_E)          \
    ,STR(SMEM_SIP6_30_SMU_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_SGC_STREAM_GATE_STATE_2_E) \
    ,STR(SMEM_SIP6_30_SMU_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_SGC_LENGTH_AWARE_2_E)      \
    ,STR(SMEM_SIP6_30_SMU_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_SGC_IPV_2_E)               \
    ,STR(SMEM_SIP6_30_SMU_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_SGC_IMX_PROFILE_2_E)       \
    ,STR(SMEM_SIP6_30_SMU_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_SGC_NEW_SLOT_3_E)          \
    ,STR(SMEM_SIP6_30_SMU_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_SGC_STREAM_GATE_STATE_3_E) \
    ,STR(SMEM_SIP6_30_SMU_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_SGC_LENGTH_AWARE_3_E)      \
    ,STR(SMEM_SIP6_30_SMU_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_SGC_IPV_3_E)               \
    ,STR(SMEM_SIP6_30_SMU_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_SGC_IMX_PROFILE_3_E)       \
    ,STR(SMEM_SIP6_30_SMU_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_SGC_NEW_SLOT_4_E)          \
    ,STR(SMEM_SIP6_30_SMU_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_SGC_STREAM_GATE_STATE_4_E) \
    ,STR(SMEM_SIP6_30_SMU_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_SGC_LENGTH_AWARE_4_E)      \
    ,STR(SMEM_SIP6_30_SMU_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_SGC_IPV_4_E)               \
    ,STR(SMEM_SIP6_30_SMU_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_SGC_IMX_PROFILE_4_E)       \
    ,STR(SMEM_SIP6_30_SMU_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_SGC_NEW_SLOT_5_E)          \
    ,STR(SMEM_SIP6_30_SMU_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_SGC_STREAM_GATE_STATE_5_E) \
    ,STR(SMEM_SIP6_30_SMU_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_SGC_LENGTH_AWARE_5_E)      \
    ,STR(SMEM_SIP6_30_SMU_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_SGC_IPV_5_E)               \
    ,STR(SMEM_SIP6_30_SMU_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_SGC_IMX_PROFILE_5_E)       \
    ,STR(SMEM_SIP6_30_SMU_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_SGC_NEW_SLOT_6_E)          \
    ,STR(SMEM_SIP6_30_SMU_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_SGC_STREAM_GATE_STATE_6_E) \
    ,STR(SMEM_SIP6_30_SMU_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_SGC_LENGTH_AWARE_6_E)      \
    ,STR(SMEM_SIP6_30_SMU_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_SGC_IPV_6_E)               \
    ,STR(SMEM_SIP6_30_SMU_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_SGC_IMX_PROFILE_6_E)       \
    ,STR(SMEM_SIP6_30_SMU_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_SGC_NEW_SLOT_7_E)          \
    ,STR(SMEM_SIP6_30_SMU_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_SGC_STREAM_GATE_STATE_7_E) \
    ,STR(SMEM_SIP6_30_SMU_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_SGC_LENGTH_AWARE_7_E)      \
    ,STR(SMEM_SIP6_30_SMU_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_SGC_IPV_7_E)               \
    ,STR(SMEM_SIP6_30_SMU_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_SGC_IMX_PROFILE_7_E)

static char * sip6_30SmuSgcTimeSlotAttributesTableFieldsNames[SMEM_SIP6_30_SMU_SGC_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS___LAST_VALUE___E] =
    {SMEM_SIP6_30_SMU_SGC_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_NAME};

static SNET_ENTRY_FORMAT_TABLE_STC sip6_30SmuSgcTimeSlotAttributesTableFieldsFormat[SMEM_SIP6_30_SMU_SGC_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS___LAST_VALUE___E] =
{
/* SMEM_SIP6_30_SMU_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_SGC_NEW_SLOT_0_E*/
    STANDARD_FIELD_MAC(1),
/* SMEM_SIP6_30_SMU_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_SGC_STREAM_GATE_STATE_0_E*/
    STANDARD_FIELD_MAC(1),
/* SMEM_SIP6_30_SMU_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_SGC_LENGTH_AWARE_0_E*/
    STANDARD_FIELD_MAC(1),
/* SMEM_SIP6_30_SMU_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_SGC_IPV_0_E*/
    STANDARD_FIELD_MAC(4),
/* SMEM_SIP6_30_SMU_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_SGC_IMX_PROFILE_0_E*/
    STANDARD_FIELD_MAC(8),
/* SMEM_SIP6_30_SMU_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_SGC_NEW_SLOT_1_E*/
    STANDARD_FIELD_MAC(1),
/* SMEM_SIP6_30_SMU_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_SGC_STREAM_GATE_STATE_1_E*/
    STANDARD_FIELD_MAC(1),
/* SMEM_SIP6_30_SMU_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_SGC_LENGTH_AWARE_1_E*/
    STANDARD_FIELD_MAC(1),
/* SMEM_SIP6_30_SMU_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_SGC_IPV_1_E*/
    STANDARD_FIELD_MAC(4),
/* SMEM_SIP6_30_SMU_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_SGC_IMX_PROFILE_1_E*/
    STANDARD_FIELD_MAC(8),
/* SMEM_SIP6_30_SMU_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_SGC_NEW_SLOT_2_E*/
    STANDARD_FIELD_MAC(1),
/* SMEM_SIP6_30_SMU_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_SGC_STREAM_GATE_STATE_2_E*/
    STANDARD_FIELD_MAC(1),
/* SMEM_SIP6_30_SMU_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_SGC_LENGTH_AWARE_2_E*/
    STANDARD_FIELD_MAC(1),
/* SMEM_SIP6_30_SMU_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_SGC_IPV_2_E*/
    STANDARD_FIELD_MAC(4),
/* SMEM_SIP6_30_SMU_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_SGC_IMX_PROFILE_2_E*/
    STANDARD_FIELD_MAC(8),
/* SMEM_SIP6_30_SMU_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_SGC_NEW_SLOT_3_E*/
    STANDARD_FIELD_MAC(1),
/* SMEM_SIP6_30_SMU_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_SGC_STREAM_GATE_STATE_3_E*/
    STANDARD_FIELD_MAC(1),
/* SMEM_SIP6_30_SMU_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_SGC_LENGTH_AWARE_3_E*/
    STANDARD_FIELD_MAC(1),
/* SMEM_SIP6_30_SMU_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_SGC_IPV_3_E*/
    STANDARD_FIELD_MAC(4),
/* SMEM_SIP6_30_SMU_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_SGC_IMX_PROFILE_3_E*/
    STANDARD_FIELD_MAC(8),
/* SMEM_SIP6_30_SMU_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_SGC_NEW_SLOT_4_E*/
    STANDARD_FIELD_MAC(1),
/* SMEM_SIP6_30_SMU_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_SGC_STREAM_GATE_STATE_4_E*/
    STANDARD_FIELD_MAC(1),
/* SMEM_SIP6_30_SMU_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_SGC_LENGTH_AWARE_4_E*/
    STANDARD_FIELD_MAC(1),
/* SMEM_SIP6_30_SMU_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_SGC_IPV_4_E*/
    STANDARD_FIELD_MAC(4),
/* SMEM_SIP6_30_SMU_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_SGC_IMX_PROFILE_4_E*/
    STANDARD_FIELD_MAC(8),
/* SMEM_SIP6_30_SMU_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_SGC_NEW_SLOT_5_E*/
    STANDARD_FIELD_MAC(1),
/* SMEM_SIP6_30_SMU_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_SGC_STREAM_GATE_STATE_5_E*/
    STANDARD_FIELD_MAC(1),
/* SMEM_SIP6_30_SMU_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_SGC_LENGTH_AWARE_5_E*/
    STANDARD_FIELD_MAC(1),
/* SMEM_SIP6_30_SMU_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_SGC_IPV_5_E*/
    STANDARD_FIELD_MAC(4),
/* SMEM_SIP6_30_SMU_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_SGC_IMX_PROFILE_5_E*/
    STANDARD_FIELD_MAC(8),
/* SMEM_SIP6_30_SMU_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_SGC_NEW_SLOT_6_E*/
    STANDARD_FIELD_MAC(1),
/* SMEM_SIP6_30_SMU_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_SGC_STREAM_GATE_STATE_6_E*/
    STANDARD_FIELD_MAC(1),
/* SMEM_SIP6_30_SMU_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_SGC_LENGTH_AWARE_6_E*/
    STANDARD_FIELD_MAC(1),
/* SMEM_SIP6_30_SMU_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_SGC_IPV_6_E*/
    STANDARD_FIELD_MAC(4),
/* SMEM_SIP6_30_SMU_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_SGC_IMX_PROFILE_6_E*/
    STANDARD_FIELD_MAC(8),
/* SMEM_SIP6_30_SMU_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_SGC_NEW_SLOT_7_E*/
    STANDARD_FIELD_MAC(1),
/* SMEM_SIP6_30_SMU_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_SGC_STREAM_GATE_STATE_7_E*/
    STANDARD_FIELD_MAC(1),
/* SMEM_SIP6_30_SMU_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_SGC_LENGTH_AWARE_7_E*/
    STANDARD_FIELD_MAC(1),
/* SMEM_SIP6_30_SMU_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_SGC_IPV_7_E*/
    STANDARD_FIELD_MAC(4),
/* SMEM_SIP6_30_SMU_TIME_SLOT_ATTRIBUTES_TABLE_FIELDS_SGC_IMX_PROFILE_7_E*/
    STANDARD_FIELD_MAC(8)
};

#define SMEM_SIP6_30_SMU_SGC_TIME_TO_ADVANCE_TABLE_FIELDS_NAME                                  \
     STR(SMEM_SIP6_30_SMU_SGC_TIME_TO_ADVANCE_TABLE_FIELDS_SGC_TIME_TO_ADVANCE_E)

static char * sip6_30SmuSgcTimeToAdvanceTableFieldsNames[SMEM_SIP6_30_SMU_SGC_TIME_TO_ADVANCE_TABLE_FIELDS___LAST_VALUE___E] =
    {SMEM_SIP6_30_SMU_SGC_TIME_TO_ADVANCE_TABLE_FIELDS_NAME};

static SNET_ENTRY_FORMAT_TABLE_STC sip6_30SmuSgcTimeToAdvanceTableFieldsFormat[SMEM_SIP6_30_SMU_SGC_TIME_TO_ADVANCE_TABLE_FIELDS___LAST_VALUE___E] =
{
/* SMEM_SIP6_30_SMU_SGC_TIME_TO_ADVANCE_TABLE_FIELDS_SGC_TIME_TO_ADVANCE_E*/
    STANDARD_FIELD_MAC(32)
};

static SNET_ENTRY_FORMAT_TABLE_STC sip7EgfQagPortTargetAttributesTableFieldsFormat[SMEM_SIP5_20_EGF_QAG_PORT_TARGET_ATTRIBUTES_TABLE_FIELDS___LAST_VALUE___E] =
{
    /*SMEM_SIP5_20_EGF_QAG_PORT_TARGET_ATTRIBUTES_TABLE_FIELDS_TARGET_PHYSICAL_PORT_LOOPBACK_PROFILE_E       */
    STANDARD_FIELD_MAC(2)
    /*SMEM_SIP5_20_EGF_QAG_PORT_TARGET_ATTRIBUTES_TABLE_FIELDS_TARGET_PHYSICAL_PORT_TC_PROFILE_E             */
   ,STANDARD_FIELD_MAC(0)/* not exists in sip7 ... maybe moved ? */
    /*SMEM_SIP5_20_EGF_QAG_PORT_TARGET_ATTRIBUTES_TABLE_FIELDS_TARGET_PHYSICAL_PORT_EVLAN_MIRRORING_ENABLE_E */
   ,STANDARD_FIELD_MAC(1)
    /*SMEM_SIP5_20_EGF_QAG_PORT_TARGET_ATTRIBUTES_TABLE_FIELDS_USE_VLAN_TAG_1_FOR_TAG_STATE_E                */
   ,STANDARD_FIELD_MAC(1)
    /*SMEM_SIP5_20_EGF_QAG_PORT_TARGET_ATTRIBUTES_TABLE_FIELDS_TARGET_PHYSICAL_PORT_ENQ_PROFILE_E            */
   ,STANDARD_FIELD_MAC(0)/* not exists in sip7 ... maybe moved ? */
    /*SMEM_SIP5_20_EGF_QAG_PORT_TARGET_ATTRIBUTES_TABLE_FIELDS_LOOPBACK_ENQ_PROFILE_E                        */
   ,STANDARD_FIELD_MAC(0)/* not exists in sip7 ... maybe moved ? */
    /*SMEM_SIP5_20_EGF_QAG_PORT_TARGET_ATTRIBUTES_TABLE_FIELDS_LOOPBACK_PORT_E                               */
   ,STANDARD_FIELD_MAC(10)
};

static SNET_ENTRY_FORMAT_TABLE_STC sip7EgfQagTargetPortMapperTableFieldsFormat[
    SMEM_SIP5_20_EGRESS_EGF_QAG_TARGET_PORT_MAPPER_TABLE_FIELDS___LAST_VALUE___E] =
{
     /* SMEM_SIP5_20_EGRESS_EGF_QAG_TARGET_PORT_MAPPER_TABLE_FIELDS_PORT_BASE_E */
    STANDARD_FIELD_MAC(0)
    /* SMEM_SIP6_EGRESS_EGF_QAG_TARGET_PORT_MAPPER_TABLE_FIELDS_PORT_SPEED_E */
   ,STANDARD_FIELD_MAC(5)
  /*  ,SMEM_SIP7_EGRESS_EGF_QAG_TARGET_PORT_MAPPER_TABLE_FIELDS_UNKNOWN_CT_ENABLE_E    */
      ,STANDARD_FIELD_MAC(1)
  /*  ,SMEM_SIP7_EGRESS_EGF_QAG_TARGET_PORT_MAPPER_TABLE_FIELDS_QUEUE_GROUP_INDEX_E    */
    ,STANDARD_FIELD_MAC(17)
  /*  ,SMEM_SIP7_EGRESS_EGF_QAG_TARGET_PORT_MAPPER_TABLE_FIELDS_DP_CORE_E              */
      ,STANDARD_FIELD_MAC(3)
  /*  ,SMEM_SIP7_EGRESS_EGF_QAG_TARGET_PORT_MAPPER_TABLE_FIELDS_SET_Q_MODE_E           */
    ,STANDARD_FIELD_MAC(1)
  /*  ,SMEM_SIP7_EGRESS_EGF_QAG_TARGET_PORT_MAPPER_TABLE_FIELDS_TC_PROFILE_E           */
      ,STANDARD_FIELD_MAC(6)
  /*  ,SMEM_SIP7_EGRESS_EGF_QAG_TARGET_PORT_MAPPER_TABLE_FIELDS_SET_TC_PROF_MODE_E     */
    ,STANDARD_FIELD_MAC(2)
  /*  ,SMEM_SIP7_EGRESS_EGF_QAG_TARGET_PORT_MAPPER_TABLE_FIELDS_DESC_TC_PROF_FIRST_BIT_E  */
      ,STANDARD_FIELD_MAC(6)
  /*  ,SMEM_SIP7_EGRESS_EGF_QAG_TARGET_PORT_MAPPER_TABLE_FIELDS_DESC_TC_PROF_LAST_BIT_E   */
    ,STANDARD_FIELD_MAC(6)
  /*  ,SMEM_SIP7_EGRESS_EGF_QAG_TARGET_PORT_MAPPER_TABLE_FIELDS_TRG_IS_CASCADE_E      */
      ,STANDARD_FIELD_MAC(1)
};

static SNET_ENTRY_FORMAT_TABLE_STC sip7EgfQagPortSourceAttributesTableFieldsFormat[SMEM_SIP5_20_EGF_QAG_PORT_SOURCE_ATTRIBUTES_TABLE_FIELDS___LAST_VALUE___E] =
{
     /* SMEM_SIP5_20_EGF_QAG_PORT_SOURCE_ATTRIBUTES_TABLE_FIELDS_SOURCE_PHYSICAL_PORT_LOOPBACK_PROFILE_E */
    STANDARD_FIELD_MAC(2)
    /* SMEM_SIP5_20_EGF_QAG_PORT_SOURCE_ATTRIBUTES_TABLE_FIELDS_SOURCE_PHYSICAL_PORT_TC_PROFILE_E */
   ,STANDARD_FIELD_MAC(0)/* not exists here */
    /* SMEM_SIP6_EGF_QAG_PORT_SOURCE_ATTRIBUTES_TABLE_FIELDS_SOURCE_PHYSICAL_PORT_SPEED_E */
   ,STANDARD_FIELD_MAC(5)
};

static SNET_ENTRY_FORMAT_TABLE_STC sip7EgfQagCpuCodeToLbMapperTableFieldsFormat[SMEM_SIP5_EGF_QAG_CPU_CODE_TO_LB_MAPPER_TABLE_FIELDS___LAST_VALUE___E] =
{
    /* SMEM_SIP5_EGF_QAG_CPU_CODE_TO_LB_MAPPER_TABLE_FIELDS_LOOPBACK_PORT_E) */
    EXPLICIT_FIELD_MAC(1,10),
    /* SMEM_SIP5_EGF_QAG_CPU_CODE_TO_LB_MAPPER_TABLE_FIELDS_TXQ_PORT_E) */
    EXPLICIT_FIELD_MAC(0,0),  /* doesn't exists */
    /* SMEM_SIP5_EGF_QAG_CPU_CODE_TO_LB_MAPPER_TABLE_FIELDS_LOOPBACK_ENQ_PROFILE_E) */
    EXPLICIT_FIELD_MAC(0,0),  /* doesn't exists */
    /* SMEM_SIP5_EGF_QAG_CPU_CODE_TO_LB_MAPPER_TABLE_FIELDS_LOOPBACK_ENABLE_E) */
    EXPLICIT_FIELD_MAC(0,1)
};


#ifdef _WIN32
extern void SHOSTG_psos_reg_asic_task(void);
#endif /*_WIN32*/

static GT_BOOL internal_snetChtRxPort(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr
);

static GT_VOID snetSip6ChtRxPortCutThrough
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr
);
static GT_VOID snetChtFrameParsingVlanAssign(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN    INTERNAL_TTI_DESC_INFO_STC        * internalTtiInfoPtr
);

static GT_VOID snetChtCfmEtherTypeCheck(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr
);

static GT_VOID snetChtPpu(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr
);

static GT_VOID snetChtIPcl(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr
);

static GT_VOID snetChtIPolicer(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr
);

static GT_VOID snetChtRouting(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr
);

static GT_VOID snetChtRxMacCountUpdate(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_BOOL overSize,
    IN SNET_FLOW_CONTROL_PACKET_TYPE_ENT fcPacketType,
    IN GT_U32  macPort
);

static GT_VOID snetChtIngressIsLoopedAndDropOnSource
(
    IN  SKERNEL_DEVICE_OBJECT           *devObjPtr,
    IN  SKERNEL_FRAME_CHEETAH_DESCR_STC *descrPtr
);

static GT_VOID snetChtMarvellTagParsing(
    IN  SKERNEL_DEVICE_OBJECT           *devObjPtr,
    IN  SKERNEL_FRAME_CHEETAH_DESCR_STC *descrPtr,
    IN  INTERNAL_TTI_DESC_INFO_STC      *internalTtiInfoPtr
);

static GT_VOID snetChtMarvellTagParsing_lengthGet(
    IN  SKERNEL_DEVICE_OBJECT           *devObjPtr,
    IN  SKERNEL_FRAME_CHEETAH_DESCR_STC *descrPtr,
    OUT   GT_U32                       *mtagWords  /* ignored if NULL , filled if exists */
);


static GT_VOID snetChtVlanQosProfAssign(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32 etherType,
    IN SKERNEL_L2_ENCAPSULATION_TYPE_ENT l2encapsul,
    IN GT_U32 * portVlanCfgEntryPtr
);

static GT_VOID snetChtPortProtVlans(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32 etherType,
    IN SKERNEL_L2_ENCAPSULATION_TYPE_ENT l2encapsul,
    IN GT_U32     portProtVlanEnabled
);

static GT_VOID  snetChtQoSAssignPart1
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32 * portVlanCfgEntryPtr
);

static GT_VOID  snetChtQoSAssignPart2
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32 * portVlanCfgEntryPtr,
    IN GT_BIT   usePortBasedQos
);

static GT_VOID snetXCatA1TrunkHash(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * escrPtr
);

static GT_VOID snetChtLinkStateNotify(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32 port,
    IN GT_U32 linkState
);

extern GT_VOID snetChtL2i(
    IN    SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr
);

static GT_VOID snetChtPerformFromCpuDma(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32 txQue,
    IN  SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    OUT GT_BOOL * queueEmptyPtr,
    OUT GT_BOOL * isLastPacketPtr
);

extern GT_VOID  snetChtUcRouting(
    SKERNEL_DEVICE_OBJECT * devObjPtr,
    SKERNEL_FRAME_CHEETAH_DESCR_STC *  descrPtr
);

extern GT_VOID snetCht2Routing(
    IN    SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr
);


extern GT_BOOL snetCht2L3iGetRpfFailCmdFromMll(
    IN SKERNEL_DEVICE_OBJECT                    * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC          * descrPtr,
    IN GT_U32                                   mll_selector,
    INOUT SNET_CHEETAH2_L3_CNTRL_PACKET_INFO    *cntrlPcktInfo
);

static GT_VOID snetChtIngressL2MllQueueSelection
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC   * descrPtr
);

static GT_VOID snetChtTTermination
(
    IN    SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN    INTERNAL_TTI_DESC_INFO_STC        * internalTtiInfoPtr
);

static GT_VOID snetChtFrameProcess
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SBUF_BUF_ID bufferId,
    IN GT_U32 srcPort
);

static GT_VOID snetChtFromCpuDmaProcess
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SBUF_BUF_ID bufferId
);

static GT_VOID snetChtMll
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC   * descrPtr
);

static GT_VOID snetChtIngressL2MllEngine
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC   * descrPtr
);


static GT_BOOL snetChtDoForwardFcFrame
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    OUT SNET_FLOW_CONTROL_PACKET_TYPE_ENT   *fcPacketTypePtr
);

static GT_VOID snetChtTimestampTagAndTimestampInfo
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32 tstStartOffset
);

static GT_VOID snetChtTimestampTagDetection(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    INOUT GT_U32 * ethTypeOffsetPtr
);

static GT_BOOL sip6MACIngressIsChannelIdle
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32               txFifoPort
);

#define SNET_CHT_PORT_VLAN_QOS_DSA_CSCD_BYPASS_BRG_GET_MAC(dev, entry) \
        SMEM_U32_GET_FIELD(entry[0], 8, 1)

#define SNET_CHT_PORT_VLAN_QOS_MIRR_INGR_ANALAYZER_GET_MAC(dev, entry) \
        SMEM_U32_GET_FIELD(entry[0], 7, 1)

#define SNET_CHT_PORT_VLAN_QOS_TRUNK_ID_GET_MAC(dev, entry) \
        SMEM_U32_GET_FIELD(entry[0], 0, 7)

#define SNET_CHT_PORT_VLAN_QOS_PORT_POLICY_EN_GET_MAC(dev, entry) \
        SMEM_U32_GET_FIELD(entry[0], 10, 1)

#define SNET_CHT_PORT_VLAN_QOS_PCL0_0_LOOKUP_CONFIGURATION_MODE_GET_MAC(dev, entry) \
        SMEM_U32_GET_FIELD(entry[0], 11, 1)

#define SNET_CHT_PORT_VLAN_QOS_PCL0_1_LOOKUP_CONFIGURATION_MODE_GET_MAC(dev, entry) \
        SMEM_U32_GET_FIELD(entry[0], 12, 1)

#define SNET_CHT_PORT_VLAN_QOS_PCL1_0_LOOKUP_CONFIGURATION_MODE_GET_MAC(dev, entry) \
        SMEM_U32_GET_FIELD(entry[0], 13, 1)

#define SNET_CHT_PORT_VLAN_QOS_PVID_MODE_GET_MAC(dev, entry) \
        SMEM_U32_GET_FIELD(entry[0], 27, 1)

#define SNET_CHT_PORT_VLAN_QOS_PVID_GET_MAC(dev, entry) \
        SMEM_U32_GET_FIELD(entry[0], 15, 12)

#define SNET_CHT_PORT_VLAN_QOS_PVID_PRECEDENCE_GET_MAC(dev, entry) \
        SMEM_U32_GET_FIELD(entry[0], 29, 1)

#define SNET_CHT_PORT_VLAN_QOS_VLAN_TRANSLATION_GET_MAC(dev, entry) \
        SMEM_U32_GET_FIELD(entry[0], 31, 1)

#define SNET_CHT_PORT_VLAN_QOS_TRUST_DSA_QOS_MAC(dev, entry) \
        SMEM_U32_GET_FIELD(entry[1], 10, 1)

#define SNET_CHT_PORT_VLAN_QOS_TRUST_PKT_QOS_MODE_MAC(dev, entry) \
        SMEM_U32_GET_FIELD(entry[1], 7, 2)

/* MPLS EXP supported only by XCAT A1 and above */
#define SNET_CHT_PORT_VLAN_QOS_TRUST_PKT_MPLS_EXP_MAC(dev, entry) \
        SMEM_U32_GET_FIELD(entry[1], 9, 2)

#define SNET_CHT_PORT_VLAN_QOS_MAP_DSCP_TO_DSCP_MAC(dev, entry) \
        SMEM_U32_GET_FIELD(entry[1], 6, 1)

#define SNET_CHT_PORT_VLAN_QOS_QOS_PROFILE_MAC(dev, entry) \
        SMEM_U32_GET_FIELD(entry[1], 12, 7)

#define SNET_CHT_PORT_VLAN_QOS_MODIFY_UP_MAC(dev, entry) \
        SMEM_U32_GET_FIELD(entry[1], 4, 1)

#define SNET_CHT_PORT_VLAN_QOS_MODIFY_DSCP_MAC(dev, entry) \
        SMEM_U32_GET_FIELD(entry[1], 5, 1)

#define SNET_CHT_PORT_VLAN_QOS_QOS_PECEDENCE_MAC(dev, entry) \
        SMEM_U32_GET_FIELD(entry[1], 3, 1)

#define SNET_CHT_PORT_VLAN_QOS_PROT_BASED_QOS_MAC(dev, entry) \
        SMEM_U32_GET_FIELD(entry[1], 11, 1)

#define SNET_CHT_PORT_VLAN_QOS_PROT_BASED_VLAN_MAC(dev, entry) \
        SMEM_U32_GET_FIELD(entry[0], 30, 1)

#define SNET_CHT_PORT_VLAN_QOS_UP_MAC(dev, entry) \
        SMEM_U32_GET_FIELD(entry[1],  0, 3)

#define SNET_CHT_PORT_VLAN_QOS_ACCESS_PORT_MAC(dev, entry) \
        SMEM_U32_GET_FIELD(entry[0], 28, 1)

/* relevant only for XCAT A1 and above devices */
#define SNET_CHT_PORT_VLAN_QOS_TRUST_VLAN_TAG1_QOS_MAC(dev, entry) \
        SMEM_U32_GET_FIELD(entry[2],  1, 1)

#define SNET_CHT_PORT_PROT_VLAN_QOS_QOS_PROFILE_MODE_MAC(dev, entry) \
        SMEM_U32_GET_FIELD(entry, 18, 2)
#define SNET_CHT_PORT_PROT_VLAN_QOS_QOS_PROFILE_MAC(dev, entry) \
        SMEM_U32_GET_FIELD(entry, 20, 7)

#define SNET_CHT_PORT_PROT_VLAN_QOS_MODIFY_UP_MAC(dev, entry) \
        SMEM_U32_GET_FIELD(entry, 16, 1)

#define SNET_CHT_PORT_PROT_VLAN_QOS_MODIFY_DSCP_MAC(dev, entry) \
        SMEM_U32_GET_FIELD(entry, 17, 1)

#define SNET_CHT_PORT_PROT_VLAN_QOS_QOS_PECEDENCE_MAC(dev, entry) \
        SMEM_U32_GET_FIELD(entry, 27, 1)

#define SNET_CHT_PORT_PROT_IS_VALID_MAC(dev, entry) \
        SMEM_U32_GET_FIELD((entry), 15, 1)

#define SNET_CHT_PORT_PROT_VLAN_MODE_MAC(dev, entry) \
        SMEM_U32_GET_FIELD((entry), 12, 2)

#define SNET_CHT_PORT_PROT_VID_PRECEDENCE_MAC(dev, entry) \
        SMEM_U32_GET_FIELD((entry), 14, 1)

/* ipv4 hash -- byte index after the shift */
#define IPV4_SHIFT_INDEX_MAC(origByte,byteShift)  (((origByte)+(byteShift)) & 3)

/* ipv4 hash -- byte index after the shift */
#define IPV6_SHIFT_INDEX_MAC(origByte,byteShift)  (((origByte)+(byteShift)) & 15)

/**
* @internal snetChtPktCmdResolution function
* @endinternal
*
* @brief   resolve from old and current commands the new command
*
* @param[in] prevCmd                  - previous command
* @param[in] currCmd                  - current command
*
* @note [2] Table 5: cpu code changes conflict resolution - page 17
*
*/
extern GT_U32 snetChtPktCmdResolution
(
    IN SKERNEL_EXT_PACKET_CMD_ENT prevCmd,
    IN SKERNEL_EXT_PACKET_CMD_ENT currCmd
)
{
    SKERNEL_EXT_PACKET_CMD_ENT newCommand;
    SKERNEL_DEVICE_OBJECT * devObjPtr = NULL;

    if(prevCmd >= SKERNEL_EXT_PKT_CMD_TO_TRG_SNIFFER_E/*5*/ &&
       currCmd <= SKERNEL_EXT_PKT_CMD_SOFT_DROP_E/*5*/)
    {
        /* keep the previous command unless 'HARD DROP' */
        newCommand = (currCmd == CMD_HARD_DROP) ? CMD_HARD_DROP : prevCmd;
    }
    else
    {
        if(prevCmd >= 5 || currCmd >= 5)
        {
            /* fatal error */
            skernelFatalError("snetChtPktCmdResolution: bad parameters \n");
        }

        newCommand = chtResovedPacketCmdTable[prevCmd][currCmd];
    }

    if(newCommand != prevCmd)
    {
        __LOG_NO_LOCATION_META_DATA__WITH_SCIB_LOCK(("do packet command resolution between : "));
        __LOG_NO_LOCATION_META_DATA__WITH_SCIB_LOCK(("Orig:"));
        simLogPacketDescrPacketCmdDump(devObjPtr,prevCmd);

        __LOG_NO_LOCATION_META_DATA__WITH_SCIB_LOCK(("Assign:"));
        simLogPacketDescrPacketCmdDump(devObjPtr,currCmd);

        __LOG_NO_LOCATION_META_DATA__WITH_SCIB_LOCK(("new command is: "));
        simLogPacketDescrPacketCmdDump(devObjPtr,newCommand);
        __LOG_NO_LOCATION_META_DATA__WITH_SCIB_LOCK(("\n"));
    }

    return newCommand;
}


/**
* @internal snetChtProcessInit function
* @endinternal
*
* @brief   Init module.
*/
GT_VOID snetChtProcessInit
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr
)
{
    GT_U32  descNumber; /* number of descriptors for the Cheetah device */
    if (devObjPtr->gmDeviceType == GOLDEN_MODEL)
    {
        devObjPtr->devFrameProcFuncPtr = snetGmProcessFrameFromSlan;
        devObjPtr->devPortLinkUpdateFuncPtr = snetGmLinkStateNotify;
        return;
    }
    descNumber = 512;

    devObjPtr->prependNumBytes = 2;

    /**************************/
    /* init specific features */
    /**************************/
    devObjPtr->descriptorPtr =
        (void *)smemDeviceObjMemoryAlloc(devObjPtr,descNumber, sizeof(SKERNEL_FRAME_CHEETAH_DESCR_STC));

    if (devObjPtr->descriptorPtr == 0) {
        skernelFatalError("smemChtInit: allocation error\n");
    }
    devObjPtr->descrNumber = descNumber;

    /* initiation of internal cheetah functions */
    if(!SMEM_IS_PIPE_FAMILY_GET(devObjPtr))
    {
        devObjPtr->devFrameProcFuncPtr = snetChtFrameProcess;
        devObjPtr->devPortLinkUpdateFuncPtr = snetChtLinkStateNotify;
        devObjPtr->devFdbMsgProcFuncPtr = sfdbChtMsgProcess;
        devObjPtr->devMacTblTrigActFuncPtr = sfdbChtMacTableTriggerAction;
        devObjPtr->devMacEntriesAutoAgingFuncPtr = sfdbChtMacTableAutomaticAging;
        devObjPtr->devMacTblAgingProcFuncPtr = sfdbChtAutoAging;
        devObjPtr->devMacTblUploadProcFuncPtr = sfdbCht2MacTableUploadAction;
        devObjPtr->devCncFastDumpFuncPtr = snetCht3CncFastDumpFuncPtr;
        devObjPtr->devFromEmbeddedCpuFuncPtr = NULL;

        if(devObjPtr->supportGdma)/*sip7*/
        {
            /* not using the 'SMAIN_CPU_TX_SDMA_QUEUE_E' message type */
            /* instead there is use of the 'generic' : SMAIN_MSG_TYPE_GENERIC_FUNCTION_E */
            /* that will call the snetAasGdmaFromCpuProcess(...) function , to
               handle the 'from_cpu' packets */
            devObjPtr->devFromCpuDmaFuncPtr = NULL;
        }
        else
        {
            devObjPtr->devFromCpuDmaFuncPtr = snetChtFromCpuDmaProcess;
        }

    }
    else
    {
        devObjPtr->devFrameProcFuncPtr = snetPipeFrameProcess;
        devObjPtr->devPortLinkUpdateFuncPtr = snetChtLinkStateNotify;
        devObjPtr->devCncFastDumpFuncPtr = snetCht3CncFastDumpFuncPtr;
        devObjPtr->devFromCpuDmaFuncPtr = snetPipeFromCpuDmaProcess;

        devObjPtr->pipeDevice.pipe_descrNumber = descNumber;
        devObjPtr->pipeDevice.pipe_descriptorPtr =
            (void *)smemDeviceObjMemoryAlloc(devObjPtr,descNumber, sizeof(SKERNEL_FRAME_PIPE_DESCR_STC));

        if (devObjPtr->pipeDevice.pipe_descriptorPtr == 0) {
            skernelFatalError("smemChtInit: allocation error\n");
        }
    }

    devObjPtr->devSoftResetFunc = skernelDeviceSoftResetGeneric;
    devObjPtr->devSoftResetFuncPart2 = skernelDeviceSoftResetGenericPart2;
}

/**
* @internal snetChtExtendedPortMacGet function
* @endinternal
*
* @brief   Get MAC number for extended ports.
*/
GT_U32 snetChtExtendedPortMacGet
(
    IN SKERNEL_DEVICE_OBJECT *devObjPtr,
    IN GT_U32                 port,
    IN GT_BOOL                isRx
)
{
    DECLARE_FUNC_NAME(snetChtExtendedPortMacGet);

    GT_U32  regAddr;   /* register address */
    GT_U32  fldOffset; /* field offset in register */
    GT_U32  fldValue;  /* field value */
    GT_U32  macNum;    /* MAC number */

    GT_U32  port0Num;
    GT_U32  port1Num;

    if (devObjPtr->supportExtPortMac == 0)
    {
        return port;
    }

    switch(devObjPtr->deviceFamily)
    {
        case SKERNEL_XCAT3_FAMILY:
        case SKERNEL_AC5_FAMILY:
            port0Num = 25;
            port1Num = 27;

            /* register:
                /Cider/EBU/Alleycat3/Alleycat3 {Current}/Switching Core/GOP/<XLG_MAC> XLG MAC IP Units%p/External control;
                Physical port #24, base address 0x120D8000, register offset 0x0090.
            */
            regAddr = 0x120D8090;
            fldOffset = (port == port0Num) ?  0 : 1;

            break;
        case SKERNEL_LION2_PORT_GROUP_FAMILY:
            port0Num =  9;
            port1Num = 11;

            if(GT_TRUE == isRx)
            {
                regAddr = SMEM_LION2_RXDMA_HIGH_DMA_IF_CONFIG_REG(devObjPtr);
                fldOffset = (port == port0Num) ?  7 : 15;
            }
            else
            { /* Tx */
                regAddr = SMEM_LION2_TXDMA_ENGINE_CONFIG_EXT_PORT_CONFIG_REG(devObjPtr);
                fldOffset = (port == port0Num) ?  0 : 1;
            }

            break;

        default:
            return port;
    }

    if ((port != port0Num) && (port != port1Num))
    {
        return port;
    }

    smemRegFldGet(devObjPtr, regAddr, fldOffset, 1, &fldValue);

    if(fldValue)
    {
        __LOG(("enabled extended port mode for port %d\n", port));
    }

    macNum = port + 3 * fldValue;

    if(SKERNEL_IS_XCAT3_BASED_DEV(devObjPtr) &&
       port == port1Num &&
       fldValue)
    {
        /* exception from formula above case, macNum is 29 and not 30 */
        macNum = 29;
    }
    __LOG(("macNum is %d\n", macNum));

    return macNum;
}


/**
* @internal snetChtRxMacPortGet function
* @endinternal
*
* @brief   Get RX MAC number.
*/
static GT_U32 snetChtRxMacPortGet
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                  slanPort
)
{
    return snetChtExtendedPortMacGet(devObjPtr, slanPort, GT_TRUE/*Rx*/);
}

/**
* @internal internal_snetChtFrameProcess function
* @endinternal
*
* @brief   Process frames in the Cheetah
*/
static GT_VOID internal_snetChtFrameProcess
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SBUF_BUF_ID             bufferId,
    IN GT_U32                  srcPort,
    IN GT_BOOL                 bypassMacLayer
)
{
    /* pointer to the frame's descriptor */
    SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr;
    GT_U32  mutexUsed;

    simLogPacketFrameUnitSet(SIM_LOG_FRAME_UNIT___ALLOW_ALL_UNITS___E);/* wild card for LOG without the unit filter */
    simLogPacketFrameCommandSet(SIM_LOG_FRAME_COMMAND_TYPE_GENERAL_E);

    descrPtr = snetChtEqDuplicateDescr(devObjPtr, NULL);

    SIM_LOG_PACKET_DESCR_SAVE

    descrPtr->localDevSrcPort = srcPort;

    if(devObjPtr->numOfPipes)
    {
        /* Update 'currentPipeId' and get new (local) srcPort */
        smemConvertGlobalPortToCurrentPipeId(devObjPtr,srcPort,&srcPort);
    }

    /* save the ingress device*/
    descrPtr->ingressDevObjPtr = devObjPtr;

    descrPtr->frameBuf = bufferId;

    /* get the RXDMA port number */
    srcPort = SMEM_LION2_LOCAL_PORT_TO_RX_DMA_PORT_MAC(devObjPtr,srcPort);

    if(SMEM_CHT_IS_SIP6_GET(devObjPtr))
    {
        /* do not convert the srcPort */
        srcPort = descrPtr->localDevSrcPort;/* will set descrPtr->ingressRxDmaPortNumber with global number */
        /* meaning that ignoring the assignment from smemConvertGlobalPortToCurrentPipeId(devObjPtr,srcPort,&srcPort)
           we only needed to assign 'pipeId' ... but for the RxDMA registers we need to used 'global DMA' that is converted to local info in MACRO :
           SIP6_RXDMA_PER_CHANNEL_REG_MAC(...) ,
                RXDMA_PER_CHANNEL_REG_MAC(...)
        */
    }

    descrPtr->ingressRxDmaPortNumber = srcPort;
    /* get the MAC port number */
    descrPtr->ingressGopPortNumber = snetChtRxMacPortGet(devObjPtr,descrPtr->localDevSrcPort);

    descrPtr->byteCount = bufferId->actualDataSize;

    if(devObjPtr->portGroupSharedDevObjPtr || simLogIsOpenFlag)
    {
        mutexUsed = 1;
        /* protect the full processing */
        SIM_OS_MAC(simOsMutexLock)(LOG_fullPacketWalkThroughProtectMutex);
    }
    else
    {
        mutexUsed = 0;
    }

    SIM_LOG_PACKET_DESCR_COMPARE("internal_snetChtFrameProcess : prepare packet from network port ");

    descrPtr->ingressBypassRxMacLayer = bypassMacLayer;

    if(SMEM_CHT_IS_SIP6_10_GET(devObjPtr) &&
      (bufferId->srcType == SMAIN_SRC_TYPE_LOOPBACK_PORT_E ||
       bufferId->srcType == SMAIN_SRC_TYPE_INTERNAL_CONNECTION_E))
    {
        /* the 'egress side' that send packet knows if it sent it on 'preemptive channel' */


        /* from this point till end of this functions ... all the 'sub functions' (in the 'MAC') can do 'Get'
            to retrieve the isPreemptiveChannel of this task (in this device) */
        /* need to be set before any access to the 'MAC registers' as those depends on the 'isPreemptiveChannel' */
        smemSetCurrentTaskExtParamValue(devObjPtr,TASK_EXT_PARAM_INDEX_IS_PREEMPTIVE_CHANNEL,
            bufferId->isPreemptiveChannel);

        /* NOTE: this flag will be reset to 0 , right after internal_snetChtRxPort(...) */
    }

    snetChtIngress(devObjPtr, descrPtr);

    if(mutexUsed)
    {
        /* protect the egress processing */
        SIM_OS_MAC(simOsMutexUnlock)(LOG_fullPacketWalkThroughProtectMutex);
    }
}

/**
* @internal snetChtFrameProcess function
* @endinternal
*
* @brief   Process frames in the Cheetah
*/
static GT_VOID snetChtFrameProcess
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SBUF_BUF_ID bufferId,
    IN GT_U32 srcPort
)
{
    internal_snetChtFrameProcess(devObjPtr,bufferId,srcPort,GT_FALSE/* do not bypass MAC layer */);
}

/**
* @internal snetChtFrameProcess_bypassRxMacLayer function
* @endinternal
*
* @brief   Process frames in the Cheetah with MAC layer bypass
*/
GT_VOID snetChtFrameProcess_bypassRxMacLayer
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SBUF_BUF_ID bufferId,
    IN GT_U32 srcPort
)
{
    internal_snetChtFrameProcess(devObjPtr,bufferId,srcPort,GT_TRUE/* bypass MAC layer */);
}


/**
* @internal snetChtIngressAfterTti function
* @endinternal
*
* @brief   ingress pipe processing after the TTI unit
*/
GT_VOID snetChtIngressAfterTti
(
        IN SKERNEL_DEVICE_OBJECT * devObjPtr,
        IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr
)
{
    DECLARE_FUNC_NAME(snetChtIngressAfterTti);

    if(SMEM_CHT_IS_SIP6_10_GET(devObjPtr) && (descrPtr->ttiPpuEnable == GT_TRUE))
    {
        /* Programmable Parser Unit (PPU) processing */
        SIM_LOG_PACKET_DESCR_SAVE
        snetChtPpu(devObjPtr, descrPtr);
        SIM_LOG_PACKET_DESCR_COMPARE("snetChtPpu");
    }

    /* Policy (PCL and IP-Classifier) processing */
    SIM_LOG_PACKET_DESCR_SAVE
    snetChtIPcl(devObjPtr, descrPtr);
    SIM_LOG_PACKET_DESCR_COMPARE("snetChtIPcl");

    /* Save Ingress VID modification after the PCL,TTI Processing
        (and before the Router) */
    descrPtr->ingressPipeVid = descrPtr->eVid;

    /* L2 Processing */
    SIM_LOG_PACKET_DESCR_SAVE
    snetChtL2i(devObjPtr, descrPtr);
    SIM_LOG_PACKET_DESCR_COMPARE("snetChtL2i");

    if (descrPtr->bypassIngressPipe == GT_FALSE)
    {
        /* L3 routing */
        SIM_LOG_PACKET_DESCR_SAVE
        snetChtRouting(devObjPtr, descrPtr);
        SIM_LOG_PACKET_DESCR_COMPARE("snetChtRouting");
    }
    else
    {
        __LOG(("Router bypassed due to <bypassIngressPipe> \n"));
    }

    if (SMEM_CHT_IS_SIP7_GET(devObjPtr))
    {
        SIM_LOG_PACKET_DESCR_SAVE
        snetSip7MPcl(devObjPtr, descrPtr);
        SIM_LOG_PACKET_DESCR_COMPARE("snetSip7MPcl");
    }

    if(SMEM_CHT_IS_SIP5_GET(devObjPtr) &&
       !descrPtr->bypassRouterAndPolicer)
    {
        /* Ingress OAM processing */
        SIM_LOG_PACKET_DESCR_SAVE
        __LOG(("Ingress OAM processing (IOAM) \n"));
        snetLion2IOamProcess(devObjPtr, descrPtr);
        SIM_LOG_PACKET_DESCR_COMPARE("snetLion2IOamProcess");

        if(SMEM_CHT_IS_SIP6_30_GET(devObjPtr))
        {
            /* SMU processing */
            SIM_LOG_PACKET_DESCR_SAVE
            __LOG(("SMU processing \n"));
            snetSip6_30SmuProcess(devObjPtr, descrPtr);
            SIM_LOG_PACKET_DESCR_COMPARE("snetSip6_30SmuProcess");
        }
    }

    /* Policer Processing  */
    SIM_LOG_PACKET_DESCR_SAVE
    snetChtIPolicer(devObjPtr, descrPtr);
    SIM_LOG_PACKET_DESCR_COMPARE("snetChtIPolicer");

    /* MLL unit entry point */
    SIM_LOG_PACKET_DESCR_SAVE
    snetChtMll(devObjPtr, descrPtr);
    SIM_LOG_PACKET_DESCR_COMPARE("snetChtMll");

    /* EQ Block processing */
    snetChtEq(devObjPtr, descrPtr);
}

/*******************************************************************************
*
* snetChtIngressAfterL3IpReplication:
*       ingress pipe processing after the IP MLL (L3 IP replication) unit
*       this function treats only replications of IPMLL , the 'orig' goes out
*       snetCht3IngressL3IpReplication (from snetChtIngressL3IpReplication)
*       and will call snetChtIngressL2MllEngine (from snetChtMll)
*
* INPUTS:
*       devObjPtr    - pointer to device object.
*       descrPtr     - frame data buffer Id
*
* RETURN:
*
*******************************************************************************/
GT_VOID snetChtIngressAfterL3IpReplication
(
        IN SKERNEL_DEVICE_OBJECT * devObjPtr,
        IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr
)
{
    DECLARE_FUNC_NAME(snetChtIngressAfterL3IpReplication);
    /* MLL Ping-Pong Identifier */
    if(SMEM_CHT_IS_SIP6_10_GET(devObjPtr))
    {
        /* New descriptor bit <REP_MLL> in Ironman, is used to indicate if the
         * descriptor was duplicated by IPMLL or L2MLL. For previous
         * devices, Copy Reserved bit 19 is used.
         */
        if(SMEM_CHT_IS_SIP6_30_GET(devObjPtr))
        {
            descrPtr->repMll = 1;
        }
        else
        {
            descrPtr->copyReserved |= (1 << 19);
        }
        __LOG(("MLL Ping-Pong: repMll [0x%x] copyReserved [0x%x]\n", descrPtr->repMll, descrPtr->copyReserved));
    }

    /* L2 MLL Replication */
    SIM_LOG_PACKET_DESCR_SAVE
    snetChtIngressL2MllEngine(devObjPtr, descrPtr);
    SIM_LOG_PACKET_DESCR_COMPARE("snetChtIngressL2MllEngine \n");

    /* EQ Block processing */
    snetChtEq(devObjPtr, descrPtr);

    simLogPacketFrameUnitSet(SIM_LOG_FRAME_UNIT_MLL_E);
}

/**
* @internal snetLion3RxdmaToTti function
* @endinternal
*
* @brief   perform RXDMA unit to TTI unit changes
*
*   return indication of pass/drop from the RxDma Unit
*   GT_TRUE  - pass the packet
*   GT_FALSE - drop the packet
*/
static GT_BOOL snetLion3RxdmaToTti
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr
)
{
    DECLARE_FUNC_NAME(snetLion3RxdmaToTti);

    GT_U32      pchEnabled;
    GT_U32      virtualPortNum;/*virtualPortNum from the RXDMA*/

    simLogPacketFrameUnitSet(SIM_LOG_FRAME_UNIT_RXDMA_E);

    /*perform RXDMA mapping from local port to 'virual' port on the field of:
      localDevSrcPort */
    smemRegFldGet(devObjPtr,
        SMEM_LION3_RXDMA_SCDMA_CONFIG_1_REG(devObjPtr,descrPtr->ingressRxDmaPortNumber),
        0,
        SMEM_CHT_IS_SIP6_GET(devObjPtr) ? 10 :
        SMEM_CHT_IS_SIP5_20_GET(devObjPtr) ? 9 : 8,
        &virtualPortNum);

    __LOG(("perform RXDMA mapping from local port[%d] to virtual port[%d] on the field of <localDevSrcPort>",
                  descrPtr->localDevSrcPort,virtualPortNum));

    /* change the <localDevSrcPort> only after we are done with the MAC counters
       and the MAC configurations that are according to 'RXDMA port num'*/


    /* this must be done before first access to the TTI-Physical Port Attribute Table:
       <LocalDevSrcPort> as received from WRDMA, i.e. global physical port number */
    descrPtr->localDevSrcPort = virtualPortNum;

    /* from this point : <LocalDevSrcPort> as received from WRDMA, i.e. global physical port number */

    /* save info that used by the BMA/BM (buffer management) of the device */
    /* the RxDma for BMA number is 'local' based . */
    descrPtr->ingressRxDmaPortNumber_forBma =
        SMEM_DATA_PATH_RELATIVE_PORT_GET(devObjPtr,descrPtr->ingressRxDmaPortNumber);

    /* get the ID of the RxDMA unit from the global ingressRxDmaPortNumber port index */
    descrPtr->ingressRxDmaUnitId =
        SMEM_DUAL_UNIT_DMA_UNIT_INDEX_GET(devObjPtr,descrPtr->ingressRxDmaPortNumber);

    __LOG_PARAM(descrPtr->ingressRxDmaPortNumber_forBma);
    __LOG_PARAM(descrPtr->ingressRxDmaUnitId);

    if(SMEM_CHT_IS_SIP6_GET(devObjPtr))
    {

        if(SMEM_CHT_IS_SIP7_GET(devObjPtr))
        {
            smemRegFldGet(devObjPtr,
                SMEM_SIP6_RXDMA_CHANNEL_GENERAL_CONFIG_REG(devObjPtr,descrPtr->ingressRxDmaPortNumber),
                1,1,&pchEnabled);
        }
        else
        {
            smemRegFldGet(devObjPtr,
                SMEM_SIP6_RXDMA_CHANNEL_PCH_CONFIG_REG(devObjPtr,descrPtr->ingressRxDmaPortNumber),
                0,1,&pchEnabled);
        }

        if(descrPtr->isFromSdma && pchEnabled)
        {
            __LOG(("Simulation DROP the PACKET !!! because : The CPU SDMA physical port [%d] (DMA[%d]) hold 'pchEnabled = 1' --> configuration error \n"
                   "will cause in real HW to shift of 8 bytes in parsing by the TTI unit \n",
                descrPtr->localDevSrcPort,
                descrPtr->ingressRxDmaPortNumber));

            return GT_FALSE;/* drop the packet */
        }

    }

    return GT_TRUE;/* forward the packet */
}

/**
* @internal snetLion3IngressReassignSrcEPort function
* @endinternal
*
* @brief   TRILL/TTI/PCL reassign new src EPort
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] descrPtr                 - pointer to frame descriptor
* @param[in] clientNamePtr            - the name of the client (for the LOGGER)
* @param[in] newSrcEPort              - the new srcEPort
*/
GT_VOID snetLion3IngressReassignSrcEPort
(
    IN    SKERNEL_DEVICE_OBJECT             *devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC   *descrPtr,
    IN    GT_CHAR*                          clientNamePtr,
    IN    GT_U32                            newSrcEPort
)
{
    DECLARE_FUNC_NAME(snetLion3IngressReassignSrcEPort);

    if(descrPtr->origIsTrunk)
    {
        __LOG(("src orig was 'trunk' [%d] will be modified to 'eport' [%d] \n",
            descrPtr->origSrcEPortOrTrnk,
            newSrcEPort));
    }
    else
    if(descrPtr->srcIsVPort == 1)
    {
        __LOG(("src orig was 'vport' [%d] will be modified to 'eport' [%d] \n",
            descrPtr->srcVPort,
            newSrcEPort));
    }

    if(descrPtr->srcDev != descrPtr->ownDev)
    {
        __LOG(("src device [%d]will be modified to [%d] \n",
            descrPtr->srcDev,
            descrPtr->ownDev));
    }

    descrPtr->eArchExtInfo.localDevSrcEPort = newSrcEPort;
    descrPtr->origSrcEPortOrTrnk = newSrcEPort;
    descrPtr->origIsTrunk = 0;
    descrPtr->srcDev = descrPtr->ownDev;
    descrPtr->srcDevIsOwn = 1;

    __LOG(("[%s] Reassigns new Source EPort [%d] \n",
        clientNamePtr ? clientNamePtr : "unknown",
        newSrcEPort));

    descrPtr->srcIsVPort = 0;/* relevant to sip7 */

    return;
}

/**
* @internal snetSip7IngressReassignSrcVPortProfile function
* @endinternal
*
* @brief   TRILL/TTI/PCL reassign new src VPort profile
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] descrPtr             - pointer to frame descriptor
* @param[in] clientNamePtr            - the name of the client (for the LOGGER)
* @param[in] newSrcVPortProfile       - the new srcVPort profile
* @param[in,out] descrPtr             - pointer to frame descriptor
* RETURN:
* COMMENTS:
*/
static GT_VOID snetSip7IngressReassignSrcVPortProfile
(
    IN    SKERNEL_DEVICE_OBJECT             *devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC   *descrPtr,
    IN    GT_CHAR*                          clientNamePtr,
    IN    GT_U32                            newSrcVPortProfile
)
{
    DECLARE_FUNC_NAME(snetSip7IngressReassignSrcVPortProfile);

    /* not updating the origIsTrunk and srcIsVPort */
    /* not changing descrPtr->origSrcEPortOrTrnk , that is muxed with descrPtr->srcVPort */

    descrPtr->eArchExtInfo.localDevSrcEPort = newSrcVPortProfile;

    descrPtr->srcDev = descrPtr->ownDev;
    descrPtr->srcDevIsOwn = 1;

    __LOG(("[%s] Reassigns new Source vPort Profile [%d] \n",
        clientNamePtr ? clientNamePtr : "unknown",
        newSrcVPortProfile));
}

/**
* @internal snetSip7IngressReassignSrcVPort function
* @endinternal
*
* @brief   TRILL/TTI/PCL reassign new src VPort or new src VPort profile
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] descrPtr             - pointer to frame descriptor
* @param[in] clientNamePtr            - the name of the client (for the LOGGER)
* @param[in] isUsingVPortProfile      - indication to use the new srcVPort as 'vPort profile' and not as 'vPort'
* @param[in] newSrcVPort              - the new srcVPort
* @param[in,out] descrPtr             - pointer to frame descriptor
* RETURN:
* COMMENTS:
*/
GT_VOID snetSip7IngressReassignSrcVPort
(
    IN    SKERNEL_DEVICE_OBJECT             *devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC   *descrPtr,
    IN    GT_CHAR*                          clientNamePtr,
    IN    GT_BOOL                           isUsingVPortProfile,
    IN    GT_U32                            newSrcVPort
)
{
    DECLARE_FUNC_NAME(snetSip7IngressReassignSrcVPort);

    if(isUsingVPortProfile)
    {
        /* not updating the origIsTrunk and srcIsVPort */
        snetSip7IngressReassignSrcVPortProfile(devObjPtr,descrPtr,clientNamePtr,newSrcVPort);
        return;
    }

    if(descrPtr->origIsTrunk)
    {
        __LOG(("src orig was 'trunk' [%d] will be modified to 'vport' [%d] \n",
            descrPtr->origSrcEPortOrTrnk,
            newSrcVPort));
    }
    else
    if(descrPtr->srcIsVPort == 0)
    {
        __LOG(("src orig was 'eport' [%d] will be modified to 'vport' [%d] \n",
            descrPtr->origSrcEPortOrTrnk,
            newSrcVPort));
    }

    if(descrPtr->srcDev != descrPtr->ownDev)
    {
        __LOG(("src device [%d]will be modified to [%d] \n",
            descrPtr->srcDev,
            descrPtr->ownDev));
    }

    descrPtr->origIsTrunk = 0;
    descrPtr->srcIsVPort  = 1;
    descrPtr->srcVPort    = newSrcVPort;

    descrPtr->srcDev = descrPtr->ownDev;
    descrPtr->srcDevIsOwn = 1;

    __LOG(("[%s] Reassigns new Source vPort [%d] \n",
        clientNamePtr ? clientNamePtr : "unknown",
        newSrcVPort));

    return;
}


/**
* @internal snetSip6_30PtpUsxgmiiPchRxProcessing function
* @endinternal
*
* @brief   USXGMII PCH ingress processing.
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in] pchInfoPtr               - pointer to PCH PTP information.
*
*/
static GT_VOID snetSip6_30PtpUsxgmiiPchRxProcessing
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN SNET_PHY_SWITCH_USGMII_PCH_PTP_INFO_STC * pchInfoPtr
)
{
    DECLARE_FUNC_NAME(snetSip6_30PtpUsxgmiiPchRxProcessing);

    GT_U32 regAddr;    /* Register address */
    GT_U32 *regPtr;    /* Register pointer */
    GT_U32 macPort;    /* the MAC port number */
    ENHANCED_PORT_INFO_STC portInfo;

    if (!SMEM_CHT_IS_SIP6_30_GET(devObjPtr) || devObjPtr->supportPtp != 1)
    {
        descrPtr->ingressPchInfoValid = 0;
        return;
    }

    macPort = descrPtr->ingressGopPortNumber;
    if (devObjPtr->portsArr[macPort].state == SKERNEL_PORT_STATE_MTI_USX_E)
    {
        regAddr = MTI_PORT_EXT(devObjPtr, macPort).portEmacUsxPchRxControl;
    }
    else if (devObjPtr->portsArr[macPort].state == SKERNEL_PORT_STATE_MTI_CPU_E)
    {
        devObjPtr->devMemPortInfoGetPtr(devObjPtr,SMEM_UNIT_TYPE_MTI_MAC_CPU_E,macPort,&portInfo);
        regAddr = MTI_CPU_PORT_EXT(devObjPtr, portInfo.simplePortInfo.unitIndex).portEmacUsxPchRxControl;
    }
    else
    {
        regAddr = SMAIN_NOT_VALID_CNS;
    }
    regPtr = smemMemGet(devObjPtr, regAddr);

    /* check USXGMII Rx processing condition */
    if(SMEM_U32_GET_FIELD(regPtr[0], 0, 1) == 1 /* PROCESS */)
    {
        descrPtr->ingressPchInfoValid = 1;
        __LOG(("Ingress PCH TAI 4 Timestamp is valid.\n"));
    }
    else
    {
        descrPtr->ingressPchInfoValid = 0;
        __LOG(("Ingress PCH TAI 4 Timestamp is not valid.\n"));
    }

    return;
}

/**
* @internal snetSip6_30PtpTimestampingTai4 function
* @endinternal
*
* @brief   Timestamp free tunning TAI 4
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in] pchInfoPtr               - pointer to PCH PTP information.
*
*/
static GT_BOOL snetSip6_30PtpTimestampingTai4
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN SNET_PHY_SWITCH_USGMII_PCH_PTP_INFO_STC * pchInfoPtr
)
{
    DECLARE_FUNC_NAME(snetSip6_30PtpTimestampingTai4);

    GT_U32 taiGroup;

    if (!SMEM_CHT_IS_SIP6_30_GET(devObjPtr) || devObjPtr->supportPtp != 1)
        return GT_TRUE;

    if(descrPtr->ingressGopPortNumber == SMAIN_NOT_VALID_CNS)
    {
        __LOG(("The ingress port [%d] does not hold valid GOP port number , so no timestamp support \n",descrPtr->localDevSrcPort));
        return GT_TRUE;
    }

    taiGroup = SNET_LION3_PORT_NUM_TO_TOD_GROUP_CONVERT_MAC(devObjPtr ,descrPtr->ingressGopPortNumber);

    /* if srcPort is connected to PHY && port is USXGMII */
    if(connectedPortToPhy[descrPtr->localDevSrcPort] && devObjPtr->portsArr[descrPtr->ingressGopPortNumber].state == SKERNEL_PORT_STATE_MTI_USX_E)
    {
        /* check if RX PCH is valid */
        snetSip6_30PtpUsxgmiiPchRxProcessing(devObjPtr,descrPtr,pchInfoPtr);

        /* Extract the USXGMII_PCH<timestamp> and use it as the packet ingress timestamp instead of the MAC TOD value */
        if(descrPtr->ingressPchInfoValid)
        {
            descrPtr->tai4Timestamp[SMAIN_DIRECTION_INGRESS_E].secondTimer = descrPtr->ingressPchTai4Timestamp.secondTimer;
            descrPtr->tai4Timestamp[SMAIN_DIRECTION_INGRESS_E].nanoSecondTimer = descrPtr->ingressPchTai4Timestamp.nanoSecondTimer;
            return GT_TRUE;
        }
    }

    /* Get timestamp for internal,free running TAI4 */
    snetLion3PtpTodGetTimeCounter(devObjPtr,
                                  taiGroup,
                                  4,
                                  &descrPtr->tai4Timestamp[SMAIN_DIRECTION_INGRESS_E]);

    return GT_TRUE;
}

/**
* @internal snetSip6_30PtpIngressPhyBuildUsxgmiiPch function
* @endinternal
*
* @brief   Ingress PHY build USXGMII PCH PTP header.
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[out] pchInfoPtr              - pointer to PCH PTP information.
*
*/
static GT_VOID snetSip6_30PtpIngressPhyBuildUsxgmiiPch
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    OUT SNET_PHY_SWITCH_USGMII_PCH_PTP_INFO_STC * pchInfoPtr
)
{
    DECLARE_FUNC_NAME(snetSip6_30PtpIngressPhyBuildUsxgmiiPch);

    GT_U32 taiGroup;
    SNET_TOD_TIMER_STC tai4TimeCounter;

    if (!SMEM_CHT_IS_SIP6_30_GET(devObjPtr) || devObjPtr->supportPtp != 1)
        return;

    __LOG(("Ingress PHY builds USXGMII PCH"));
    pchInfoPtr->packetType = SNET_PHY_SWITCH_USGMII_PCH_PACKET_TYPE_ETHERNET_WITH_PCH_E;
    pchInfoPtr->extentionType = SNET_PHY_SWITCH_USGMII_PCH_EXTENTION_TYPE_PTP_E;
    pchInfoPtr->subPortId = 0; /* dummy */

    if(descrPtr->ingressGopPortNumber == SMAIN_NOT_VALID_CNS)
    {
        __LOG(("The ingress port [%d] does not hold valid GOP port number , so no timestamp support \n",descrPtr->localDevSrcPort));
        return;
    }

    taiGroup = SNET_LION3_PORT_NUM_TO_TOD_GROUP_CONVERT_MAC(devObjPtr ,descrPtr->ingressGopPortNumber);

    /* Get timestamp for internal,free running TAI4 */
    snetLion3PtpTodGetTimeCounter(devObjPtr,
                                  taiGroup,
                                  4,
                                  &tai4TimeCounter);

    pchInfoPtr->timestamp.nanoSecondTimer = tai4TimeCounter.nanoSecondTimer;
    pchInfoPtr->timestamp.secondTimer = tai4TimeCounter.secondTimer;

    return;
}

/**
* @internal snetChtRxPort function
* @endinternal
*
* @brief   Rx Port layer processing of frames
*/
GT_BOOL snetChtRxPort
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr
)
{
    DECLARE_FUNC_NAME(snetChtRxPort);

    GT_BOOL st;
    SNET_PHY_SWITCH_USGMII_PCH_PTP_INFO_STC pchInfo;

    memset(&pchInfo,0,sizeof(SNET_PHY_SWITCH_USGMII_PCH_PTP_INFO_STC));

    /* if srcPort is connected to PHY && port is USXGMII */
    if((descrPtr->localDevSrcPort < SNET_CHT_MAX_PORT_TO_PHY_CNS) &&
        connectedPortToPhy[descrPtr->localDevSrcPort] &&
       (descrPtr->ingressGopPortNumber < SKERNEL_DEV_MAX_SUPPORTED_PORTS_CNS) &&
       (devObjPtr->portsArr[descrPtr->ingressGopPortNumber].state == SKERNEL_PORT_STATE_MTI_USX_E))
    {
        simLogPacketFrameUnitSet(SIM_LOG_FRAME_UNIT_PHY_INGRESS_E);

        /* Ingress PHY builds USXGMII PCH */
        snetSip6_30PtpIngressPhyBuildUsxgmiiPch(devObjPtr,descrPtr,&pchInfo);
    }

    simLogPacketFrameUnitSet(SIM_LOG_FRAME_UNIT_PORT_MAC_INGRESS_E);

    st = internal_snetChtRxPort(devObjPtr, descrPtr);

    /* restore to 0 any value that was set inside internal_snetChtRxPort(...) */
    smemSetCurrentTaskExtParamValue(devObjPtr,TASK_EXT_PARAM_INDEX_IS_PREEMPTIVE_CHANNEL,0);

    simLogPacketFrameUnitSet(SIM_LOG_FRAME_UNIT___ALLOW_ALL_UNITS___E);/* wild card for LOG without the unit filter */

    /* Rx Port layer processing of frames */
    if (st == GT_FALSE)
    {
        __LOG(("Packet was dropped by the 'MAC/PHY/Rxdma' layer.\n"));

        __LOG((SIM_LOG_ENDED_IN_PACKET_STR
                "ended processing frame from: deviceName[%s],deviceId[%d], \n"
                "                    portGroupId[%d],srcPort[%d] \n",
                devObjPtr->deviceName,
                devObjPtr->deviceId,
                devObjPtr->portGroupId ,
                descrPtr->localDevSrcPort));

        return st;
    }

    /* sample free running TAI4 */
    return snetSip6_30PtpTimestampingTai4(devObjPtr,descrPtr,&pchInfo);
}

/**
* @internal snetChtIngress function
* @endinternal
*
* @brief   Ingress processing of frames
*/
GT_VOID snetChtIngress
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr
)
{
    DECLARE_FUNC_NAME(snetChtIngress);

    GT_BOOL st;
    INTERNAL_TTI_DESC_INFO_STC  internalTtiInfo;

    /* save info to log */
    __LOG((SIM_LOG_IN_PACKET_STR
            "start new frame on: deviceName[%s],deviceId[%d], \n"
            "                    portGroupId[%d],srcPort[%d] \n",
            devObjPtr->deviceName,
            devObjPtr->deviceId,
            devObjPtr->portGroupId ,
            descrPtr->localDevSrcPort));

    /* save info to the descriptor even if packet discard , for the LOG info */
    SIM_LOG_PACKET_DESCR_SAVE
    descrPtr->startFramePtr = descrPtr->frameBuf->actualDataPtr;
    descrPtr->byteCount = descrPtr->frameBuf->actualDataSize;
    descrPtr->origByteCount = descrPtr->byteCount;

    if(!descrPtr->isFromSdma)
    {
        if(devObjPtr->errata.byte_count_missing_4_bytes)
        {
            descrPtr->is_byte_count_missing_4_bytes = 1;
        }

        if(devObjPtr->errata.byte_count_missing_4_bytes_in_HA)
        {
            descrPtr->is_byte_count_missing_4_bytes_in_HA = 1;
        }

        if(devObjPtr->errata.byte_count_missing_4_bytes_in_L2i)
        {
            descrPtr->is_byte_count_missing_4_bytes_in_L2i = 1;
        }
    }

    __LOG(("frame dump:  \n"));
    simLogPacketDescrFrameDump(devObjPtr, descrPtr);

    st = snetChtRxPort(devObjPtr, descrPtr);
    SIM_LOG_PACKET_DESCR_COMPARE("snetChtRxPort");
    /* Rx Port layer processing of frames */
    if (st == GT_FALSE)
    {
        return;
    }

    /* Init the internal TTI unit info */
    memset(&internalTtiInfo,0,sizeof(INTERNAL_TTI_DESC_INFO_STC));

    simLogPacketFrameUnitSet(SIM_LOG_FRAME_UNIT_TTI_E);

    SIM_LOG_PACKET_DESCR_SAVE
    /* Parse frames header and assign VLAN/QoS Port Protocol */
    snetChtFrameParsingVlanAssign(devObjPtr, descrPtr,&internalTtiInfo);
    SIM_LOG_PACKET_DESCR_COMPARE("snetChtFrameParsingVlanAssign");

    /* Tunnel Termination processing */
    SIM_LOG_PACKET_DESCR_SAVE
    snetChtTTermination(devObjPtr, descrPtr,&internalTtiInfo);
    SIM_LOG_PACKET_DESCR_COMPARE("snetChtTTermination");

    /* continue the ingress pipe after the TTI unit */
    snetChtIngressAfterTti(devObjPtr, descrPtr);

    simLogPacketFrameUnitSet(SIM_LOG_FRAME_UNIT___ALLOW_ALL_UNITS___E);/* wild card for LOG without the unit filter */

    __LOG((SIM_LOG_ENDED_IN_PACKET_STR
            "ended processing frame from: deviceName[%s],deviceId[%d], \n"
            "                    portGroupId[%d],srcPort[%d] \n",
            devObjPtr->deviceName,
            devObjPtr->deviceId,
            devObjPtr->portGroupId ,
            descrPtr->localDevSrcPort));

    return;
}
/* the types of PIP priorities */
typedef enum{
    PIP_PRIORITY_HIGH_E         = 0,
    PIP_PRIORITY_MEDIUM_E       = 1,
    PIP_PRIORITY_LOW_E          = 2,
    PIP_PRIORITY_VERY_HIGH_E    = 3
}PIP_PRIORITY_ENT;

/* the types of sip6 RxDma drop counting priorities */
typedef enum{
     SIP6_RXDMA_DROP_REASON_PIP_E
    ,SIP6_RXDMA_DROP_REASON_CONTEXT_ID_E

    /* must be last */
    ,SIP6_RXDMA_DROP_REASON___LAST___E
}SIP6_RXDMA_DROP_REASON_ENT;

typedef struct{
    /* the 'name' (string) sip6 RxDma drop reasons */
    GT_CHAR* nameStr;
    /* the start bit for sip6 RxDma drop reasons */
    GT_U32   startBit;
}SIP6_RXDMA_DROP_REASON_INFO_STC;

/* info about sip6 rxdma drop reasons */
/* index to this array is value from SIP6_RXDMA_DROP_REASON_ENT */
static SIP6_RXDMA_DROP_REASON_INFO_STC dropReasonInfo[SIP6_RXDMA_DROP_REASON___LAST___E] = {
     {"drop by PIP"       , 0}
    ,{"drop by contex-id" , 2}
};

/**
* @internal snetSip6RxDmaDropPacketCount function
* @endinternal
*
* @brief   counting drops for sip6 RxDma Pre-Ingress Prioritization (PIP)
*          sip6 hold single counter for the unit (not per port , not per priority)
*/
static void snetSip6RxDmaDropPacketCount
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SIP6_RXDMA_DROP_REASON_ENT dropReason,
    IN GT_U32                   unitIndex,
    IN PIP_PRIORITY_ENT         pipPriority,
    IN GT_U32                   portNum
)
{
    DECLARE_FUNC_NAME(snetSip6RxDmaDropPacketCount);

    GT_U32 regAddr;          /* register address */
    GT_U32 fldValue;         /* register's field address */
    GT_U32 fldValue_pattern; /* register's field address */
    GT_U32 fldValue_mask;    /* register's field address */
    GT_U32 localPortNum = smemMultiDpUnitRelativePortGet(devObjPtr,portNum);
    GT_U32 checkedValue;
    static const GT_U32 counter_pip_prio_convert[4] = {2,1,0,3};
    GT_U32 counter_pip_prio = counter_pip_prio_convert[pipPriority];
    SIP6_RXDMA_DROP_REASON_INFO_STC *dropInfoPtr = &dropReasonInfo[dropReason];

    regAddr = SMEM_SIP6_RXDMA_DP_RX_ENABLE_DEBUG_COUNTERS_REG_MAC(devObjPtr, unitIndex);
    smemRegFldGet(devObjPtr, regAddr, 0, 1, &fldValue);
    if(fldValue == 0)
    {
        __LOG(("Warning : there is '%s' that will not be counted because RxDma counters globally disabled \n",
            dropInfoPtr->nameStr));
        return;
    }

    if (SMEM_CHT_IS_SIP7_GET(devObjPtr))
    {
        regAddr = SMEM_SIP7_RXDMA_RX_INGRESS_DROP_COUNT_TYPE_REF_REG_MAC (
            devObjPtr, unitIndex, localPortNum, counter_pip_prio) ;
        smemRegGet(devObjPtr, regAddr, &fldValue_pattern);
        regAddr   = SMEM_SIP7_RXDMA_RX_INGRESS_DROP_COUNT_TYPE_MASK_REG_MAC(
            devObjPtr, unitIndex, localPortNum, counter_pip_prio) ;
        smemRegGet(devObjPtr, regAddr, &fldValue_mask);

        checkedValue = (1 << dropInfoPtr->startBit);

        __LOG_PARAM(counter_pip_prio);
        __LOG_PARAM(fldValue_pattern);
        __LOG_PARAM(fldValue_mask   );
        __LOG_PARAM(checkedValue    );
        __LOG_PARAM(checkedValue     & fldValue_mask);
        __LOG_PARAM(fldValue_pattern & fldValue_mask);

        if((checkedValue & fldValue_mask) == (fldValue_pattern & fldValue_mask))
        {
            regAddr = SMEM_SIP7_RXDMA_RX_INGRESS_DROP_COUNTER_MAC(
                devObjPtr, unitIndex, localPortNum, counter_pip_prio);

            smemRegGet(devObjPtr, regAddr, &fldValue);
            __LOG(("RxDma drop counter: Increment Counter from [0x%x] {reason[%s],local_DMA[%d],priority[%d]} \n",
                fldValue,
                dropInfoPtr->nameStr,
                localPortNum,
                pipPriority));
            smemRegSet(devObjPtr, regAddr, ++fldValue);
        }
        else
        {
            __LOG(("the counter not configured to count current packet {reason[%s],local_DMA[%d],priority[%d]} \n ",
                dropInfoPtr->nameStr,
                localPortNum,
                pipPriority));
        }
    }
    else
    {
        regAddr = SMEM_SIP6_RXDMA_RX_INGRESS_DROP_COUNT_TYPE_REF_REG_MAC (devObjPtr, unitIndex) ;
        smemRegGet(devObjPtr, regAddr, &fldValue_pattern);
        regAddr   = SMEM_SIP6_RXDMA_RX_INGRESS_DROP_COUNT_TYPE_MASK_REG_MAC(devObjPtr, unitIndex) ;
        smemRegGet(devObjPtr, regAddr, &fldValue_mask);

        __LOG_PARAM(counter_pip_prio);
        checkedValue = (1 << dropInfoPtr->startBit) |
                       localPortNum      << 8 |
                       counter_pip_prio  << 6;

        __LOG_PARAM(fldValue_pattern);
        __LOG_PARAM(fldValue_mask   );
        __LOG_PARAM(checkedValue    );

        __LOG_PARAM(checkedValue     & fldValue_mask);
        __LOG_PARAM(fldValue_pattern & fldValue_mask);

        if((checkedValue & fldValue_mask) == (fldValue_pattern & fldValue_mask))
        {
            regAddr = SMEM_SIP6_RXDMA_RX_INGRESS_DROP_COUNTER_MAC(devObjPtr, unitIndex);

            smemRegGet(devObjPtr, regAddr, &fldValue);
            __LOG(("RxDma drop counter: Increment Counter from [0x%x] {reason[%s],local_DMA[%d],priority[%d]} \n",
                fldValue,
                dropInfoPtr->nameStr,
                localPortNum,
                pipPriority));
            smemRegSet(devObjPtr, regAddr, ++fldValue);
        }
        else
        {
            __LOG(("the counter not configured to count current packet {reason[%s],local_DMA[%d],priority[%d]} \n ",
                dropInfoPtr->nameStr,
                localPortNum,
                pipPriority));
        }
    }
}

static GT_U32  FIFOFillLevel_contextId = 1;

/**
* @internal snet6RxDmaPipCheckThresholds_contextId function
* @endinternal
*
* @brief   sip6 RxDma Pre-Ingress Prioritization (PIP) check thresholds for the 'context-id'
*   return indication if need to drop the packet
*/
static GT_U32 snet6RxDmaPipCheckThresholds_contextId
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32                   unitIndex,
    IN PIP_PRIORITY_ENT         pipPriority,
    IN GT_U32                   portNum

)
{
    DECLARE_FUNC_NAME(snet6RxDmaPipCheckThresholds_contextId);

    GT_U32 regAddress;          /* register address */
    GT_U32 fldValue;            /* register's field address */
    GT_U32  contextId_thresholds[4];
    GT_U32  dropPacket = 0;

    regAddress = SMEM_SIP_5_10_RXDMA_PIP_ENABLE_REG(devObjPtr,unitIndex);
    smemRegFldGet(devObjPtr, regAddress, 0, 1, &fldValue);
    if(fldValue == 0)
    {
        __LOG(("contextId : PIP : in RxDma unit : Globally disabled \n"));
        return 0;/* not dropped by the 'context-id' */
    }

    if(descrPtr->rxdmaParser.packetType == SNET_RXDMA_PARSER_PACKET_TYPE_LATENCY_SENSITIVE_E)
    {
        /* NOTE: unlike the IAS 1.8 that described to set priority to 'very high'
            the designer of the unit explained next : */
        __LOG(("contextId : PIP : the LATENCY SENSITIVE packets, do not enter the flow.    \n"
               "They are dropped only if there are no free context IDs at all. \n"
               "'OS Priority' is not modified \n"));

        return 0;/* not dropped by the 'context-id' */
    }

    regAddress = SMEM_SIP6_RXDMA_CONTEXT_INDEX_PIP_PRIO_THRESH0_REG(devObjPtr,
        unitIndex);
    smemRegGet(devObjPtr, regAddress, &fldValue);
    contextId_thresholds[PIP_PRIORITY_VERY_HIGH_E] = fldValue & 0xFFFF;
    contextId_thresholds[PIP_PRIORITY_HIGH_E]      = fldValue >>16;

    regAddress = SMEM_SIP6_RXDMA_CONTEXT_INDEX_PIP_PRIO_THRESH1_REG(devObjPtr,
        unitIndex);
    smemRegGet(devObjPtr, regAddress, &fldValue);
    contextId_thresholds[PIP_PRIORITY_MEDIUM_E]    = fldValue & 0xFFFF;
    contextId_thresholds[PIP_PRIORITY_LOW_E]       = fldValue >>16;

    __LOG_PARAM(contextId_thresholds[PIP_PRIORITY_LOW_E]);
    __LOG_PARAM(contextId_thresholds[PIP_PRIORITY_MEDIUM_E]);
    __LOG_PARAM(contextId_thresholds[PIP_PRIORITY_HIGH_E]);
    __LOG_PARAM(contextId_thresholds[PIP_PRIORITY_VERY_HIGH_E]);

    if(FIFOFillLevel_contextId < contextId_thresholds[PIP_PRIORITY_LOW_E])
    {
        __LOG(("contextId fill level [%d] is lower than 'low' prio threshold : allow all traffic to ingress \n",
            FIFOFillLevel_contextId));
    }
    else
    if(FIFOFillLevel_contextId < contextId_thresholds[PIP_PRIORITY_MEDIUM_E])
    {
        __LOG(("contextId : PIP :  fill level [%d] is lower than 'medium' prio threshold : do not allow 'low' priority traffic to ingress \n",
            FIFOFillLevel_contextId));
        if(pipPriority == PIP_PRIORITY_LOW_E)
        {
            dropPacket = 1;
        }
    }
    else
    if(FIFOFillLevel_contextId < contextId_thresholds[PIP_PRIORITY_HIGH_E])
    {
        __LOG(("contextId : PIP :  fill level [%d] is lower than 'high' prio threshold : do not allow 'low/medium' priority traffic to ingress \n",
            FIFOFillLevel_contextId));
        if(pipPriority == PIP_PRIORITY_LOW_E ||
           pipPriority == PIP_PRIORITY_MEDIUM_E )
        {
            dropPacket = 1;
        }
    }
    else
    if(FIFOFillLevel_contextId < contextId_thresholds[PIP_PRIORITY_VERY_HIGH_E])
    {
        __LOG(("contextId : PIP :  fill level [%d] is lower than 'very-high' prio threshold : allow only 'very-high' priority traffic to ingress \n",
            FIFOFillLevel_contextId));
        if(pipPriority != PIP_PRIORITY_VERY_HIGH_E)
        {
            dropPacket = 1;
        }
    }
    else
    {
        __LOG(("contextId : PIP :  fill level [%d] is equal of higher than 'very-high' prio threshold :  do not allow any traffic to ingress \n",
            FIFOFillLevel_contextId));
        dropPacket = 1;
    }

    if(dropPacket)
    {
        /* dropped by the contextId threshold */
        __LOG(("contextId : PIP : Simulation Dropped the packet as threshold is 0 (check if need to count it) \n"));

        /* JIRA : (RX7-150) Oversubscription drop counter limitation */

        /* sip6 hold single counter for the unit (not per port , not per priority) */
        snetSip6RxDmaDropPacketCount(devObjPtr,SIP6_RXDMA_DROP_REASON_CONTEXT_ID_E,unitIndex,pipPriority,portNum);
    }

    return dropPacket;
}

/* debug option for sip_6 PIP logic */
enum{SIP6_PIP_CHECK_THRESHOLD_MODE_SINGLE_E,
     SIP6_PIP_CHECK_THRESHOLD_MODE_MULTI_E};
static GT_U32   sip6_pip_check_threshold_mode = SIP6_PIP_CHECK_THRESHOLD_MODE_MULTI_E;
static GT_U32  FIFOFillLevel = 1;

/**
* @internal snet6RxDmaPipCheckThresholds function
* @endinternal
*
* @brief   sip6 RxDma Pre-Ingress Prioritization (PIP) check thresholds
*   return GT_TRUE  - packet not dropped , can continue processing
*   return GT_FALSE - packet dropped , can not continue processing
*/
static GT_BOOL snet6RxDmaPipCheckThresholds
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32                   unitIndex,
    IN PIP_PRIORITY_ENT         pipPriority,
    IN GT_U32                   portNum

)
{
    DECLARE_FUNC_NAME(snet6RxDmaPipCheckThresholds);

    GT_U32 regAddress;          /* register address */
    GT_U32 fldValue;            /* register's field address */
    GT_U32  thresholds[4];
    GT_U32 threshold_sensitive_threshold;
    GT_U32  pipeId = smemGetCurrentPipeId(devObjPtr);
    GT_U32  dropPacket = 0;

    __LOG(("PIP : the pip priority considered as [%s] \n" ,
        (pipPriority == PIP_PRIORITY_VERY_HIGH_E) ? "VERY HIGH" :
        (pipPriority == PIP_PRIORITY_HIGH_E) ? "HIGH" :
        (pipPriority == PIP_PRIORITY_MEDIUM_E) ? "MEDIUM" :
                             "LOW" ));

    if(descrPtr->rxdmaParser.packetType == SNET_RXDMA_PARSER_PACKET_TYPE_LATENCY_SENSITIVE_E)
    {
        /* NOTE: in this case we not refer to 'latency sensitive' as 'very high' priority */
        /* only the 'contextId' will treat it as such ! */

        __LOG(("PIP : the LATENCY SENSITIVE traffic hold dedicated FIFO and threshold \n"));
        regAddress = SMEM_SIP6_IA_PIP_LATENCY_SENSITIVE_THRESHOLD_REG(devObjPtr,
            pipeId);
        smemRegGet(devObjPtr, regAddress, &threshold_sensitive_threshold);
        __LOG_PARAM(threshold_sensitive_threshold);

        if(threshold_sensitive_threshold == 0)
        {
            dropPacket = 1;
            __LOG(("LATENCY SENSITIVE dropped due to dedicated LATENCY SENSITIVE threshold = 0 \n"));
        }

        goto check_to_drop_packet_lbl;
    }

    regAddress = SMEM_SIP_5_20_IA_PIP_PRIO_THRESH0_REG(devObjPtr,
        pipeId);
    smemRegGet(devObjPtr, regAddress, &fldValue);
    thresholds[PIP_PRIORITY_VERY_HIGH_E] = fldValue & 0xFFFF;
    thresholds[PIP_PRIORITY_HIGH_E]      = fldValue >>16;

    regAddress = SMEM_SIP_5_20_IA_PIP_PRIO_THRESH1_REG(devObjPtr,
        pipeId);
    smemRegGet(devObjPtr, regAddress, &fldValue);
    thresholds[PIP_PRIORITY_MEDIUM_E]    = fldValue & 0xFFFF;
    thresholds[PIP_PRIORITY_LOW_E]       = fldValue >>16;

    __LOG_PARAM(thresholds[PIP_PRIORITY_LOW_E]);
    __LOG_PARAM(thresholds[PIP_PRIORITY_MEDIUM_E]);
    __LOG_PARAM(thresholds[PIP_PRIORITY_HIGH_E]);
    __LOG_PARAM(thresholds[PIP_PRIORITY_VERY_HIGH_E]);

    if(sip6_pip_check_threshold_mode == SIP6_PIP_CHECK_THRESHOLD_MODE_SINGLE_E)
    {
        /* like in previous devices the simulation checks single threshold .(the 'priority threshold') */
        if(thresholds[pipPriority] < FIFOFillLevel)
        {
            /* need to drop */
            dropPacket = 1;
            __LOG(("dropped due to threshold = 0 \n"));
        }
    }
    else  /* this is more like the scheme according to IAS */
    {
        if(thresholds[PIP_PRIORITY_VERY_HIGH_E]  < FIFOFillLevel)
        {
            /* need to drop */
            dropPacket = 1;
            __LOG(("all are dropped due to [very-high] threshold = 0 \n"));
        }
        else
        if(thresholds[PIP_PRIORITY_HIGH_E]  < FIFOFillLevel &&
            pipPriority != PIP_PRIORITY_VERY_HIGH_E)
        {
            /* need to drop */
            dropPacket = 1;
            __LOG(("low/medium/high priority dropped due to [high] threshold = 0 \n"));
        }
        else
        if(thresholds[PIP_PRIORITY_MEDIUM_E] < FIFOFillLevel &&
            pipPriority != PIP_PRIORITY_VERY_HIGH_E &&
            pipPriority != PIP_PRIORITY_HIGH_E)
        {
            /* need to drop */
            dropPacket = 1;
            __LOG(("low/medium priority dropped due to [medium] threshold = 0 \n"));
        }
        else
        if(thresholds[PIP_PRIORITY_LOW_E]  < FIFOFillLevel &&
           pipPriority == PIP_PRIORITY_LOW_E)
        {
            /* need to drop */
            dropPacket = 1;
            __LOG(("low priority dropped due to [low] threshold = 0 \n"));
        }
    }

check_to_drop_packet_lbl:

    if (dropPacket)
    {
        __LOG(("PIP : Simulation Dropped the packet as threshold is 0 (check if need to count it) \n"));

        /* JIRA : (RX7-150) Oversubscription drop counter limitation */

        /* sip6 hold single counter for the unit (not per port , not per priority) */
        snetSip6RxDmaDropPacketCount(devObjPtr,SIP6_RXDMA_DROP_REASON_PIP_E,unitIndex,pipPriority,portNum);

        return GT_FALSE;
    }
    else
    {
        return GT_TRUE;
    }
}

/* names of sip6 rxdma packet type recognition */
static GT_CHAR* sip6RxdmaPacketTypeNames[SNET_RXDMA_PARSER_PACKET_TYPE___LAST___E+1] =
{
     STR(SNET_RXDMA_PARSER_PACKET_TYPE_LLC_E)
    ,STR(SNET_RXDMA_PARSER_PACKET_TYPE_MPLS_E)
    ,STR(SNET_RXDMA_PARSER_PACKET_TYPE_IPV4_E)
    ,STR(SNET_RXDMA_PARSER_PACKET_TYPE_IPV6_E)
    ,STR(SNET_RXDMA_PARSER_PACKET_TYPE_GENERIC_E)
    ,STR(SNET_RXDMA_PARSER_PACKET_TYPE_LATENCY_SENSITIVE_E)
    ,STR(SNET_RXDMA_PARSER_PACKET_TYPE_ETHERNET_E)

    ,STR(SNET_RXDMA_PARSER_PACKET_TYPE___LAST___E)
};

/**
* @internal snetSip6RxDmaParser function
* @endinternal
*
* @brief   do RxDma parser looking for 'L3 layer'
*/
static GT_VOID snetSip6RxDmaParser
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr
)
{
    DECLARE_FUNC_NAME(snetSip6RxDmaParser);

    SNET_RXDMA_PARSER_PACKET_TYPE_ENT packetType;
    GT_U32 regAddress;          /* register address */
    GT_U32 fldValue;            /* register's field address */
    GT_U32  portNum = descrPtr->ingressRxDmaPortNumber;/* rxDma port number */
    GT_U32  unitIndex = SMEM_DUAL_UNIT_DMA_UNIT_INDEX_GET(devObjPtr,portNum);
    GT_U32  ii,profileIndex,skipTagIndex;
    GT_BOOL tpidMatched;
    GT_U8  *positionPtr;/* position in the packet */
    GT_U32  currEtherTypeOnPacket;
    GT_U32  TPID_Tag_Size;
    GT_BOOL typeSet;

    positionPtr = &descrPtr->startFramePtr[12];/* after MAC SA */

    /* the default if will not be matched by any specific type */
    packetType = SNET_RXDMA_PARSER_PACKET_TYPE_ETHERNET_E;

    descrPtr->rxdmaParser.packetType   = packetType;
    descrPtr->rxdmaParser.ethertypePtr = positionPtr;

    if(descrPtr->marvellTagged)
    {
        /* get the length of the DSA tag , and set it in descrPtr->incomingMtagCmd
           the function called from TTI or RxDma unit. */
        snetChtMarvellTagParsing_lengthGet(devObjPtr,descrPtr,NULL);

        __LOG(("hold DSA of [%d] words , need to skip it \n",
            descrPtr->marvellTaggedExtended+1));

        positionPtr += 4 * (descrPtr->marvellTaggedExtended+1);
    }

    for(skipTagIndex = 0 ; skipTagIndex < 4 ; skipTagIndex++)
    {
        currEtherTypeOnPacket =  positionPtr[0] << 8 | positionPtr[1];
        __LOG_PARAM(currEtherTypeOnPacket);
        tpidMatched = GT_FALSE;
        for(profileIndex = 0 ; profileIndex < 4 ; profileIndex++)
        {
            /* check to skip 'TPIDs' with length 4/6/8 bytes (up to 4 tags can be skipped) */
            regAddress = SMEM_SIP6_RXDMA_PIP_PROFILE_TPID_REG(devObjPtr,profileIndex,unitIndex);
            smemRegGet(devObjPtr, regAddress,&fldValue);

            if(SMEM_U32_GET_FIELD(fldValue,0,16) != currEtherTypeOnPacket)
            {
                continue;
            }

            /* need to skip the number of bytes */
            TPID_Tag_Size = SMEM_U32_GET_FIELD(fldValue,16,2);

            switch(TPID_Tag_Size)
            {
                case 1:
                    __LOG(("TPID tag of 4 bytes skipped \n"));
                    positionPtr += 4;
                    break;
                case 2:
                    __LOG(("TPID tag of 6 bytes skipped \n"));
                    positionPtr += 6;
                    break;
                case 3:
                    __LOG(("TPID tag of 8 bytes skipped \n"));
                    positionPtr += 8;
                    break;
                default:
                    __LOG(("ERROR : configuration ERROR : unsupported <TPID_Tag_Size> value [%d]\n"
                           " simulation ignores the value and stop the RXDMA parser \n" ,
                        TPID_Tag_Size));

                    return;/* configuration ERROR : exist the parser .... */
            }

            /* found match */
            tpidMatched = GT_TRUE;

            break;
        }

        if(tpidMatched == GT_FALSE)
        {
            /* no more tags to skip */
            break;
        }
    }

    currEtherTypeOnPacket =  positionPtr[0] << 8 | positionPtr[1];
    __LOG_PARAM(currEtherTypeOnPacket);

    typeSet = GT_FALSE;

    if(currEtherTypeOnPacket <= 1500)
    {
        packetType = SNET_RXDMA_PARSER_PACKET_TYPE_LLC_E;
        typeSet = GT_TRUE;
        __LOG(("LLC because ethertype <= 1500 \n"));
    }

    if(currEtherTypeOnPacket == 0x8870)
    {
        packetType = SNET_RXDMA_PARSER_PACKET_TYPE_LLC_E;
        typeSet = GT_TRUE;
        __LOG(("LLC because ethertype == 0x8870 \n"));
    }

    if(typeSet == GT_FALSE)
    {
        smemRegFldGet(devObjPtr,
                      SMEM_SIP_5_10_RXDMA_PIP_MPLS_ETHERTYPE_CONF_N_REG(devObjPtr, 0,unitIndex),
                      0, 16, &fldValue);

        if(currEtherTypeOnPacket == fldValue)
        {
            packetType = SNET_RXDMA_PARSER_PACKET_TYPE_MPLS_E;
            typeSet = GT_TRUE;
            __LOG(("MPLS (0) because ethertype == [0x%4.4x] \n",
                   currEtherTypeOnPacket));
            descrPtr->rxdmaParser.indexOfMatch = 0;
        }
        else
        {
            smemRegFldGet(devObjPtr,
                          SMEM_SIP_5_10_RXDMA_PIP_MPLS_ETHERTYPE_CONF_N_REG(devObjPtr, 1,unitIndex),
                          0, 16, &fldValue);
            if(currEtherTypeOnPacket == fldValue)
            {
                packetType = SNET_RXDMA_PARSER_PACKET_TYPE_MPLS_E;
                typeSet = GT_TRUE;
                __LOG(("MPLS (1) because ethertype == [0x%4.4x] \n",
                       currEtherTypeOnPacket));
                descrPtr->rxdmaParser.indexOfMatch = 1;
            }
        }
    }

    if(typeSet == GT_FALSE)
    {
        smemRegFldGet(devObjPtr,
            SMEM_SIP_5_10_RXDMA_PIP_IPV4_ETHERTYPE_CONF_REG(devObjPtr,unitIndex),
            0, 16, &fldValue);
        if(currEtherTypeOnPacket == fldValue)
        {
            packetType = SNET_RXDMA_PARSER_PACKET_TYPE_IPV4_E;
            typeSet = GT_TRUE;
            __LOG(("Ipv4 because ethertype == [0x%4.4x] \n",
                currEtherTypeOnPacket));
        }
    }

    if(typeSet == GT_FALSE)
    {
        smemRegFldGet(devObjPtr,
            SMEM_SIP_5_10_RXDMA_PIP_IPV6_ETHERTYPE_CONF_REG(devObjPtr,unitIndex),
            0, 16, &fldValue);
        if(currEtherTypeOnPacket == fldValue)
        {
            packetType = SNET_RXDMA_PARSER_PACKET_TYPE_IPV6_E;
            typeSet = GT_TRUE;
            __LOG(("Ipv4 because ethertype == [0x%4.4x] \n",
                currEtherTypeOnPacket));
        }
    }

    if(typeSet == GT_FALSE)
    {
        for(ii = 0 ; ii < 4 ; ii++)
        {
            smemRegFldGet(devObjPtr,
                SMEM_SIP_5_10_RXDMA_PIP_GENERIC_ETHERTYPE_N_REG(devObjPtr, ii,unitIndex),
                0, 16, &fldValue);

            if(currEtherTypeOnPacket != fldValue)
            {
                continue;
            }

            packetType = SNET_RXDMA_PARSER_PACKET_TYPE_GENERIC_E;
            typeSet = GT_TRUE;
            __LOG(("Generic[%d] (UDE) because ethertype == [0x%4.4x] \n",
                ii,
                currEtherTypeOnPacket));
            descrPtr->rxdmaParser.indexOfMatch = ii;

            break;
        }
    }

    if(typeSet == GT_FALSE)
    {
        for(ii = 0 ; ii < 4 ; ii++)
        {
            smemRegFldGet(devObjPtr,
                SMEM_SIP6_RXDMA_PIP_LATENCY_SENSITIVE_ETHER_TYPE_REG(devObjPtr, ii,unitIndex),
                0, 16, &fldValue);

            if(currEtherTypeOnPacket != fldValue)
            {
                continue;
            }

            packetType = SNET_RXDMA_PARSER_PACKET_TYPE_LATENCY_SENSITIVE_E;
            typeSet = GT_TRUE;
            __LOG(("latency sensitive[%d] because ethertype == [0x%4.4x] \n",
                ii,
                currEtherTypeOnPacket));

            descrPtr->rxdmaParser.indexOfMatch = ii;
            break;
        }
    }

    if(typeSet == GT_FALSE)
    {
        __LOG(("EtherType [0x%x] was not recognized as any specific type \n",
        currEtherTypeOnPacket));
    }

    __LOG(("sip6 Rxdma parser classify packet as [%s] \n",
        sip6RxdmaPacketTypeNames[packetType]));
    /* save the value to the descriptor */
    descrPtr->rxdmaParser.packetType   = packetType;
    descrPtr->rxdmaParser.ethertypePtr = positionPtr;
}

/**
* @internal snetSip5_10RxDmaPipCheck function
* @endinternal
*
* @brief   RxDma Pre-Ingress Prioritization (PIP) processing of frames
*   return GT_TRUE  - packet not dropped , can continue processing
*   return GT_FALSE - packet dropped , can not continue processing
*/
static GT_BOOL snetSip5_10RxDmaPipCheck
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr
)
{
    DECLARE_FUNC_NAME(snetSip5_10RxDmaPipCheck);

    GT_U32 regAddress;          /* register address */
    GT_U32 fldValue;            /* register's field address */
    GT_U32 portNum = descrPtr->ingressRxDmaPortNumber;/* rxDma port number */
    GT_U32 Port_Default_PIP_Priority;
    GT_U32 pipProfile;
    PIP_PRIORITY_ENT pipPriority = PIP_PRIORITY_HIGH_E;/*0*/
    GT_U32 thrOff;
    GT_U32 pipeId = 0;
    GT_U32 firstWordAfterMacSa,secondWordAfterMacSa;
    GT_U32 byteIndex;
    GT_U32 fieldIndex = 0;
    GT_U32 etherType,vid;
    GT_U32 packetEtherType;
    GT_U32 ii;
    GT_U32 prioFldSize;
    GT_BIT matched = 0;
    GT_U32 mpls_exp , ipv4_tos , ipv6_tc;
    GT_U32 packet_macDa_2_msb , packet_macDa_4_lsb;
    GT_U32 macDa_2_msb , macDa_4_lsb;
    GT_U32 macDa_2_msb_mask , macDa_4_lsb_mask;
    GT_U32 otherDpValue; /* FIFO threshold value from non source port RXDMA */
    GT_U32  unitIndex = SMEM_DUAL_UNIT_DMA_UNIT_INDEX_GET(devObjPtr,portNum);
    GT_U32 counterDisabled;

    if(SMEM_IS_PIPE_FAMILY_GET(devObjPtr))
    {
        /* the PIPE device not supports the PIP (not hold the IA unit , like BC3 where the 'PIP_ENABLE' exists)*/
        __LOG(("PIPE device : not supports the PIP feature \n"));
        return GT_TRUE;
    }

    if(SMEM_CHT_IS_SIP5_20_GET(devObjPtr))
    {
        if(SMAIN_NOT_VALID_CNS == UNIT_BASE_ADDR_GET_ALLOW_NON_EXIST_UNIT_MAC(devObjPtr,UNIT_IA))
        {
            /* The IA unit not exists , so the feature is not supported */
            __LOG(("PIP : (IA unit) not exists , so the feature is not supported \n"));
            return GT_TRUE;
        }

        pipeId = smemGetCurrentPipeId(devObjPtr);
        regAddress = SMEM_SIP_5_20_IA_PIP_ENABLE_REG(devObjPtr, pipeId);
    }
    else
    {
        regAddress = SMEM_SIP_5_10_RXDMA_PIP_ENABLE_REG(devObjPtr,unitIndex);
    }
    smemRegFldGet(devObjPtr, regAddress, 0, 1, &fldValue);

    if(fldValue == 0)
    {
        if(SMEM_CHT_IS_SIP5_20_GET(devObjPtr))
        {
            __LOG(("PIP : (IA unit) Globally disabled \n"));
        }
        else
        {
            __LOG(("PIP : (RxDma unit) Globally disabled \n"));
        }
        return GT_TRUE;
    }

    regAddress = SMEM_SIP_5_10_RXDMA_PIP_SCDMA_N_PIP_CONFIG_REG(devObjPtr,portNum);
    smemRegFldGet(devObjPtr, regAddress, 0, 1, &fldValue);
    smemRegFldGet(devObjPtr, regAddress, 1, 2, &Port_Default_PIP_Priority);

    if(fldValue == 0)
    {
        __LOG(("PIP : the ingress rxDMA port [%d] is 'not trusted' ... use default PIP priority [%d] \n",
            portNum ,Port_Default_PIP_Priority));

        pipPriority = Port_Default_PIP_Priority;
    }
    else
    {
        smemRegFldGet(devObjPtr, regAddress, 3, 2, &pipProfile);

        __LOG_PARAM(pipProfile);

        __LOG(("PIP : the ingress rxDMA port [%d] is 'trusted' \n",
            portNum));

        byteIndex = 12;
        firstWordAfterMacSa =    descrPtr->macDaPtr[byteIndex + 0] << 24 |
                                 descrPtr->macDaPtr[byteIndex + 1] << 16 |
                                 descrPtr->macDaPtr[byteIndex + 2] <<  8 |
                                 descrPtr->macDaPtr[byteIndex + 3] <<  0 ;
        byteIndex +=4 ;
        secondWordAfterMacSa =   descrPtr->macDaPtr[byteIndex + 0] << 24 |
                                 descrPtr->macDaPtr[byteIndex + 1] << 16 |
                                 descrPtr->macDaPtr[byteIndex + 2] <<  8 |
                                 descrPtr->macDaPtr[byteIndex + 3] <<  0 ;

        __LOG_PARAM(firstWordAfterMacSa);
        __LOG_PARAM(secondWordAfterMacSa);

        if(descrPtr->marvellTagged)
        {
            /* check the DSA type */
            if(SMEM_U32_GET_FIELD(firstWordAfterMacSa,30,2) == 3 /*Tag Command*/&&
               SMEM_U32_GET_FIELD(firstWordAfterMacSa,12,1) /*Extend*/)
            {
                fieldIndex = SMEM_U32_GET_FIELD(secondWordAfterMacSa,13,7);

                __LOG(("PIP: DSA TAG 'FORWARD' Extended/eDsa so use <QoSProfile> [%d] \n",
                    fieldIndex));

                regAddress = SMEM_SIP_5_10_RXDMA_PIP_QOS_DSA_PRIORITY_ARRAY_PROFILE_N_SEGMENT_M_REG(
                                devObjPtr,pipProfile,fieldIndex,unitIndex);
            }
            else
            {
                fieldIndex = SMEM_U32_GET_FIELD(firstWordAfterMacSa,13,3);

                __LOG(("PIP: DSA TAG without <QoSProfile> so use <UP> [%d] \n",
                    fieldIndex));

                regAddress = SMEM_SIP_5_10_RXDMA_PIP_UP_DSA_TAG_ARRAY_PROFILE_N_REG(
                                devObjPtr,pipProfile,unitIndex);
            }

            matched = 1;
        }
        else
        {
            packetEtherType = SMEM_U32_GET_FIELD(firstWordAfterMacSa,16,16);
            __LOG_PARAM(packetEtherType);

            /* check Is <portN> and <VID classification enabled> and (EtherType & VID) pair
                equal to one of 4 pairs of (<VLAN EtherType Conf> & <VID Conf>) arrays */
            smemRegFldGet(devObjPtr, regAddress, 5, 1, &fldValue);
            if(fldValue)
            {
                __LOG(("PIP : rxDMA port [%d] is enabled for {etherType,vid} lookup \n",
                    portNum));

                for(ii = 0 ; ii < 4 ; ii++)
                {
                    smemRegFldGet(devObjPtr,
                        SMEM_SIP_5_10_RXDMA_PIP_VLAN_ETHERTYPE_CONF_N_REG(devObjPtr, ii,unitIndex),
                        0, 16, &etherType);

                    smemRegFldGet(devObjPtr,
                        SMEM_SIP_5_10_RXDMA_PIP_VID_CONF_N_REG(devObjPtr, ii,unitIndex),
                        0, 12, &vid);

                    __LOG(("PIP : check match for etherType [0x%x] and vid [%d] in index [%d] \n",
                        etherType,vid,ii));

                    if(vid       == SMEM_U32_GET_FIELD(firstWordAfterMacSa, 0,12) &&
                       etherType == packetEtherType)
                    {
                        __LOG(("PIP : the etherType [0x%x] and vid [%d] matched index [%d] \n",
                            etherType,vid,ii))

                        smemRegFldGet(devObjPtr,
                            SMEM_SIP_5_10_RXDMA_PIP_SCDMA_N_PIP_CONFIG_REG(devObjPtr,portNum),
                            6, 2, &fldValue);

                        pipPriority = fldValue;

                        __LOG(("PIP : so use (per port) VID classification PIP priority [%d] \n",
                            pipPriority));

                        goto got_pip_priority_lbl;
                    }
                }

                /* no match in <VLAN EtherType Conf> & <VID Conf> */
                __LOG(("PIP : the etherType [0x%x] and vid [%d] was not matched \n",
                    packetEtherType,
                    SMEM_U32_GET_FIELD(firstWordAfterMacSa, 0,12)));
            }
            else
            {
                __LOG(("PIP : rxDMA port [%d] is disabled for {etherType,vid} lookup \n",
                    portNum));
            }

            __LOG(("PIP : Check if EtherType is 'vlan tag' (TPID) \n"));

            /* check if EtherType equals one of 4 values in <VLAN EtherType Conf> array */
            for(ii = 0 ; ii < 4 ; ii++)
            {
                smemRegFldGet(devObjPtr,
                    SMEM_SIP_5_10_RXDMA_PIP_VLAN_ETHERTYPE_CONF_N_REG(devObjPtr, ii,unitIndex),
                    0, 16, &etherType);

                __LOG(("PIP : check match for etherType [0x%x] in index [%d] \n",
                    etherType,ii));

                if(etherType == packetEtherType)
                {
                    fieldIndex = SMEM_U32_GET_FIELD(firstWordAfterMacSa,13,3);
                    __LOG(("PIP : the etherType [0x%x] matched index [%d] - as vlan tag  , use <UP> [%d]  \n",
                        etherType,ii,fieldIndex))

                    regAddress = SMEM_SIP_5_10_RXDMA_PIP_VLAN_UP_PRIORITY_ARRAY_PROFILE_N_REG(
                                    devObjPtr,pipProfile,unitIndex);

                    matched = 1;
                    break;
                }
            }/* vlan tag */

            if(0 == matched)
            {
                __LOG(("PIP : the etherType [0x%x] not matched as 'vlan tag' \n",
                    packetEtherType));
            }

            if(SMEM_CHT_IS_SIP6_GET(devObjPtr))
            {
                /* in sip6 we already did packet type parsing ,and we have next info :
                descrPtr->rxdmaParser.packetType
                descrPtr->rxdmaParser.indexOfMatch
                descrPtr->rxdmaParser.ethertypePtr
                */
                /* next words use as 'L3 info' and should be updated ...
                   sip6 knows to jump over up to 4 tags including 4/6/8 bytes each */
                firstWordAfterMacSa =    descrPtr->rxdmaParser.ethertypePtr[0] << 24 |
                                         descrPtr->rxdmaParser.ethertypePtr[1] << 16 |
                                         descrPtr->rxdmaParser.ethertypePtr[2] <<  8 |
                                         descrPtr->rxdmaParser.ethertypePtr[3] <<  0 ;
                __LOG_PARAM(firstWordAfterMacSa);

                secondWordAfterMacSa =   descrPtr->rxdmaParser.ethertypePtr[4] << 24 |
                                         descrPtr->rxdmaParser.ethertypePtr[5] << 16 |
                                         descrPtr->rxdmaParser.ethertypePtr[6] <<  8 |
                                         descrPtr->rxdmaParser.ethertypePtr[7] <<  0 ;
                __LOG_PARAM(secondWordAfterMacSa);

                packetEtherType = descrPtr->rxdmaParser.ethertypePtr[0] << 8 |
                                  descrPtr->rxdmaParser.ethertypePtr[1];
                __LOG_PARAM(packetEtherType);
            }

            if(0 == matched &&
                (!SMEM_CHT_IS_SIP6_GET(devObjPtr) || descrPtr->rxdmaParser.packetType == SNET_RXDMA_PARSER_PACKET_TYPE_MPLS_E))
            {
                __LOG(("PIP : Check if EtherType is MPLS \n"));
                for(ii = 0 ; ii < 2 ; ii++)
                {
                    smemRegFldGet(devObjPtr,
                        SMEM_SIP_5_10_RXDMA_PIP_MPLS_ETHERTYPE_CONF_N_REG(devObjPtr, ii,unitIndex),
                        0, 16, &etherType);

                    __LOG(("PIP : check match for etherType [0x%x] in index [%d] \n",
                        etherType,ii));

                    if(etherType == packetEtherType)
                    {
                        mpls_exp = SMEM_U32_GET_FIELD(secondWordAfterMacSa, 25,3);
                        __LOG_PARAM(mpls_exp);

                        __LOG(("PIP : the etherType [0x%x] matched index [%d] - as MPLS , use <EXP> [%d] \n",
                            etherType,ii,mpls_exp));

                        fieldIndex = mpls_exp;

                        regAddress = SMEM_SIP_5_10_RXDMA_PIP_MPLS_EXP_PRIORITY_ARRAY_PROFILE_N_REG(
                                        devObjPtr,pipProfile,unitIndex);

                        matched = 1;
                        break;
                    }
                }

                if(0 == matched)
                {
                    __LOG(("PIP : the etherType [0x%x] not matched as MPLS \n",
                        packetEtherType));
                }
            }/* mpls */

            if(0 == matched &&
                (!SMEM_CHT_IS_SIP6_GET(devObjPtr) || descrPtr->rxdmaParser.packetType == SNET_RXDMA_PARSER_PACKET_TYPE_IPV4_E))
            {
                __LOG(("PIP : Check if EtherType is IPv4 \n"));
                smemRegFldGet(devObjPtr,
                    SMEM_SIP_5_10_RXDMA_PIP_IPV4_ETHERTYPE_CONF_REG(devObjPtr,unitIndex),
                    0, 16, &etherType);

                if(etherType == packetEtherType)
                {
                    ipv4_tos = SMEM_U32_GET_FIELD(firstWordAfterMacSa, 0,8);
                    __LOG_PARAM(ipv4_tos);

                    __LOG(("PIP : the etherType [0x%x] matched as IPv4 , use <TOS> [%d] \n",
                        etherType,ipv4_tos))

                    fieldIndex = ipv4_tos;

                    regAddress = SMEM_SIP_5_10_RXDMA_PIP_IPV4_TOS_PRIORITY_ARRAY_PROFILE_N_SEGMENT_M_REG(
                                    devObjPtr,pipProfile,fieldIndex,unitIndex);

                    matched = 1;
                }
                else
                {
                    __LOG(("PIP : the etherType [0x%x] not matched as IPv4 \n",
                        packetEtherType));
                }
            }/*IPv4*/

            if(0 == matched &&
                (!SMEM_CHT_IS_SIP6_GET(devObjPtr) || descrPtr->rxdmaParser.packetType == SNET_RXDMA_PARSER_PACKET_TYPE_IPV6_E))
            {
                __LOG(("PIP : Check if EtherType is IPv6 \n"));
                smemRegFldGet(devObjPtr,
                    SMEM_SIP_5_10_RXDMA_PIP_IPV6_ETHERTYPE_CONF_REG(devObjPtr,unitIndex),
                    0, 16, &etherType);

                if(etherType == packetEtherType)
                {
                    ipv6_tc = SMEM_U32_GET_FIELD(firstWordAfterMacSa, 4,8);
                    __LOG_PARAM(ipv6_tc);

                    fieldIndex = ipv6_tc;

                    if (devObjPtr->errata.rxdmaPipIpv6WrongBitsTc)
                    {
                        __LOG(("WARNING : Erratum : only the ipv6_tc taken from wrong offset \n"));
                        /* we not use the actual 'TC' from the ipv6 header ... but the 8 bits from 'wrong' offset */
                        fieldIndex = SMEM_U32_GET_FIELD(firstWordAfterMacSa, 2/*wrong offset*/,8);
                        __LOG(("PIP : the etherType [0x%x] matched as IPv6 , with <TC from ipv6 header> [%d] , but 'register access actual tc' is [%d] \n",
                            etherType,ipv6_tc,fieldIndex))
                    }
                    else
                    {
                        __LOG(("PIP : the etherType [0x%x] matched as IPv6 , with <TC> [%d] \n",
                            etherType,ipv6_tc))
                    }

                    regAddress = SMEM_SIP_5_10_RXDMA_PIP_IPV6_TC_PRIORITY_ARRAY_PROFILE_N_SEGMENT_M_REG(
                                    devObjPtr,pipProfile,fieldIndex,unitIndex);

                    matched = 1;
                }
                else
                {
                    __LOG(("PIP : the etherType [0x%x] not matched as IPv6 \n",
                        packetEtherType));
                }
            }/* ipv6 */

            if(0 == matched &&
                (!SMEM_CHT_IS_SIP6_GET(devObjPtr) || descrPtr->rxdmaParser.packetType == SNET_RXDMA_PARSER_PACKET_TYPE_GENERIC_E))
            {
                __LOG(("PIP : Check if EtherType is UDE (user defined etherType) - generic etherType \n"));
                for(ii = 0 ; ii < 4 ; ii++)
                {
                    smemRegFldGet(devObjPtr,
                        SMEM_SIP_5_10_RXDMA_PIP_GENERIC_ETHERTYPE_N_REG(devObjPtr, ii,unitIndex),
                        0, 16, &etherType);

                    __LOG(("check match for etherType [0x%x] in index [%d] \n",
                        etherType,ii));

                    if(etherType == packetEtherType)
                    {
                        __LOG(("PIP : the etherType [0x%x] matched index [%d] - as UDE \n",
                            etherType,ii))

                        fieldIndex = ii;

                        regAddress = SMEM_SIP_5_10_RXDMA_PIP_ETHERTYPE_PRIORITY_ARRAY_PROFILE_N_REG(
                                        devObjPtr,pipProfile,unitIndex);

                        matched = 1;
                        break;
                    }
                }

                if(0 == matched)
                {
                    __LOG(("PIP : the etherType [0x%x] not matched as UDE \n",
                        packetEtherType));
                }
            }/* UDE */

            if(0 == matched)
            {
                __LOG(("PIP : Check if MAC DA match 'well known' address \n"));


                byteIndex = 0;
                packet_macDa_2_msb = descrPtr->macDaPtr[byteIndex + 0] <<  8 |
                                     descrPtr->macDaPtr[byteIndex + 1] <<  0 ;
                byteIndex += 2;
                packet_macDa_4_lsb = descrPtr->macDaPtr[byteIndex + 0] << 24 |
                                     descrPtr->macDaPtr[byteIndex + 1] << 16 |
                                     descrPtr->macDaPtr[byteIndex + 2] <<  8 |
                                     descrPtr->macDaPtr[byteIndex + 3] <<  0 ;

                __LOG(("PIP : packet mac DA [0x%4.4x%8.8x] \n",
                    packet_macDa_2_msb,packet_macDa_4_lsb));

                for(ii = 0 ; ii < 4 ; ii++)
                {
                    smemRegFldGet(devObjPtr,
                        SMEM_SIP_5_10_RXDMA_PIP_MAC_DA_2_MSB_BYTES_CONF_N_REG(devObjPtr, ii,unitIndex),
                        0, 16, &macDa_2_msb);
                    smemRegGet(devObjPtr,
                        SMEM_SIP_5_10_RXDMA_PIP_MAC_DA_4_LSB_BYTES_CONF_N_REG(devObjPtr, ii,unitIndex),
                        &macDa_4_lsb);

                    smemRegFldGet(devObjPtr,
                        SMEM_SIP_5_10_RXDMA_PIP_MAC_DA_MASK_2_MSB_CONF_N_REG(devObjPtr, ii,unitIndex),
                        0, 16, &macDa_2_msb_mask);
                    smemRegGet(devObjPtr,
                        SMEM_SIP_5_10_RXDMA_PIP_MAC_DA_MASK_4_LSB_CONF_N_REG(devObjPtr, ii,unitIndex),
                        &macDa_4_lsb_mask);

                    __LOG(("check match for macDa in index [%d] with value [0x%4.4x%8.8x] mask[0x%4.4x%8.8x] \n",
                        ii,
                        macDa_2_msb,macDa_4_lsb,
                        macDa_2_msb_mask,macDa_4_lsb_mask));


                    if((packet_macDa_2_msb & macDa_2_msb_mask) == macDa_2_msb &&
                       (packet_macDa_4_lsb & macDa_4_lsb_mask) == macDa_4_lsb )
                    {
                        __LOG(("PIP : the mac addr matched index [%d] \n",
                            ii))

                        fieldIndex = ii;

                        regAddress = SMEM_SIP_5_10_RXDMA_PIP_MAC_DA_PRIORITY_ARRAY_PROFILE_N_REG(
                                        devObjPtr,pipProfile,unitIndex);

                        matched = 1;
                        break;
                    }
                }

                if(0 == matched)
                {
                    __LOG(("PIP : the macDa not matched \n"));
                }
            }/* macDa */
        }

        if(0 == matched)
        {
            __LOG(("PIP : no classification matched the packet ... use default PIP priority [%d] \n",
                Port_Default_PIP_Priority));

            pipPriority = Port_Default_PIP_Priority;
        }
        else
        {
            smemRegFldGet(devObjPtr, regAddress, 2 * (fieldIndex % 16) , 2, &fldValue);

            __LOG(("PIP : the classification resulted PIP priority [%d] \n",
                fldValue));

            pipPriority = fldValue;
        }
    }

    if(SMEM_CHT_IS_SIP5_20_GET(devObjPtr))
    {
        if(pipPriority > PIP_PRIORITY_VERY_HIGH_E)
        {
            __LOG(("WARNING : potential configuration ERROR : "
                "pipPriority [%d] more than 'max' 3 \n",
                 pipPriority));
            __LOG(("Simulation will treat it as 2 'low priority' \n"));
            pipPriority = PIP_PRIORITY_LOW_E;
        }
    }
    else
    {
        if(pipPriority > PIP_PRIORITY_LOW_E)
        {
            __LOG(("WARNING : potential configuration ERROR : "
                "pipPriority [%d] more than 'max' 2 \n",
                 pipPriority));
            __LOG(("Simulation will treat it as 2 'low priority' \n"));
            pipPriority = PIP_PRIORITY_LOW_E;
        }
    }

got_pip_priority_lbl:

    switch (pipPriority)
    {
        default:
        case PIP_PRIORITY_HIGH_E:
            descrPtr->pipPriority = 0;
            break;
        case PIP_PRIORITY_MEDIUM_E:
            descrPtr->pipPriority = 1;
            break;
        case PIP_PRIORITY_LOW_E:
            descrPtr->pipPriority = 2;
            break;
        case PIP_PRIORITY_VERY_HIGH_E:
            descrPtr->pipPriority = 3;
            break;
    }

    if(SMEM_CHT_IS_SIP6_GET(devObjPtr))
    {
        /* check if should be dropped by the context-id threshold */
        if(snet6RxDmaPipCheckThresholds_contextId(devObjPtr,descrPtr,unitIndex,pipPriority,portNum))
        {
            /* packet was dropped !!! by the contextId thresholds */
            return GT_FALSE;
        }

        return snet6RxDmaPipCheckThresholds(devObjPtr,descrPtr,unitIndex,pipPriority,portNum);
    }

    if(SMEM_CHT_IS_SIP5_20_GET(devObjPtr))
    {
        __LOG(("PIP : the pip priority considered as [%s] \n" ,
            (pipPriority == PIP_PRIORITY_VERY_HIGH_E) ? "VERY HIGH" :
            (pipPriority == PIP_PRIORITY_HIGH_E) ? "HIGH" :
            (pipPriority == PIP_PRIORITY_MEDIUM_E) ? "MEDIUM" :
                                 "LOW" ));

        thrOff = (pipPriority == PIP_PRIORITY_HIGH_E)   ? 16: /* High priority      */
                 (pipPriority == PIP_PRIORITY_MEDIUM_E) ? 0 : /* Medium priority    */
                 (pipPriority == PIP_PRIORITY_LOW_E)    ? 16: /* Low priority       */
                 0;/*pipPriority == 3*/   /* Very high priority */
        if((PIP_PRIORITY_HIGH_E      == pipPriority) ||
           (PIP_PRIORITY_VERY_HIGH_E == pipPriority))
        {
            regAddress = SMEM_SIP_5_20_IA_PIP_PRIO_THRESH0_REG(devObjPtr,
                pipeId);
        }
        else
        {
            regAddress = SMEM_SIP_5_20_IA_PIP_PRIO_THRESH1_REG(devObjPtr,
                pipeId);
        }

        prioFldSize = 16;
    }
    else
    {
        __LOG(("PIP : the pip priority considered as [%s] \n" ,
            (pipPriority == PIP_PRIORITY_HIGH_E) ? "HIGH" :
            (pipPriority == PIP_PRIORITY_MEDIUM_E) ? "MEDIUM" :
                                 "LOW" ));

        regAddress =
            SMEM_SIP_5_10_RXDMA_PIP_PRIO_THRESHOLDS_REG(devObjPtr,unitIndex);

        thrOff = pipPriority * 10;

        prioFldSize = 10;
    }

    smemRegFldGet(devObjPtr, regAddress, thrOff, prioFldSize, &fldValue);
    __LOG(("PIP : The global threshold value for this pip priority is [%d] \n",
        fldValue));

    /* Caelum and Aldrin has RXDMA Oversubscription FIFO synchronization.
       BC3 does not have such.
       This synch interface works by following way:
       1. Drop all Low priority traffic if one RXDMA starts to drop Low priority traffic.
       2. Drop all Low and Medium priority traffic if one RXDMA starts to drop Medium priority traffic.
       3. There is no synch for High priority. */
    if(fldValue && pipPriority &&
       ((!SMEM_CHT_IS_SIP5_20_GET(devObjPtr) && devObjPtr->multiDataPath.supportMultiDataPath && IS_PORT_0_EXISTS(devObjPtr))))
    {
        /* check non source port RXDMA units */
        for(ii = 0; ii < devObjPtr->multiDataPath.maxDp; ii++)
        {
            /* skip source port unit */
            if(ii == unitIndex)
            {
                continue;
            }

            regAddress = SMEM_SIP_5_10_RXDMA_PIP_PRIO_THRESHOLDS_REG(devObjPtr,unitIndex);

            /* read medium priority threshold */
            smemRegFldGet(devObjPtr, regAddress, 10*1, 10, &otherDpValue);

            if(otherDpValue == 0)
            {
                /* both Low and Medium priority traffic dropped if one Medium FIFO is closed */
                fldValue = 0;
                __LOG(("PIP : Medium priority threshold of RXDMA [%d] is 0 \n", ii));
                break;
            }

            if(pipPriority == PIP_PRIORITY_LOW_E)
            {
                /* source port has low priority. Need to check low priority threshold. */
                smemRegFldGet(devObjPtr, regAddress, 10*2, 10, &otherDpValue);

                if(otherDpValue == 0)
                {
                    /* Low FIFO is closed */
                    fldValue = 0;
                    __LOG(("PIP : Low priority threshold of RXDMA [%d] is 0 \n", ii));
                    break;
                }
            }
        }
    }

    if (fldValue == 0)
    {
        __LOG(("PIP : Simulation Dropped the packet as threshold is 0 \n"));

        /* do counting : 'per priority' and 'per port' */

        /* per port */
        regAddress = SMEM_SIP_5_10_RXDMA_PIP_SCDMA_N_DROP_PKT_COUNTER(devObjPtr,portNum);
        smemRegGet(devObjPtr, regAddress, &fldValue);
        __LOG(("PIP : port [%d] counter : Increment PIP drop Counter from [%d] \n",
            portNum,fldValue));
        smemRegSet(devObjPtr, regAddress, ++fldValue);

        /* per priority */
        regAddress = SMEM_SIP_5_10_RXDMA_PIP_PRIORITY_DROP_GLOBAL_COUNTERS_CLEAR_REG(devObjPtr,unitIndex);
        smemRegGet(devObjPtr, regAddress, &fldValue);


        if(SMEM_CHT_IS_SIP5_20_GET(devObjPtr))
        {
            /* bit 0 - low       ---> pipPriority = 2 */
            /* bit 1 - med       ---> pipPriority = 1 */
            /* bit 2 - high      ---> pipPriority = 0 */
            /* bit 3 - very high ---> pipPriority = 3 */
            if(pipPriority != PIP_PRIORITY_VERY_HIGH_E)
            {
                counterDisabled = (1 == (fldValue & (1 << (2 - pipPriority))));
            }
            else
            {
                counterDisabled = (1 << 3) == (fldValue & (1 << 3));
            }

        }
        else
        {
            /* bit 0 - low  ---> pipPriority = 2 */
            /* bit 1 - med  ---> pipPriority = 1 */
            /* bit 2 - high ---> pipPriority = 0 */
            counterDisabled = (1 == (fldValue & (1 << (2 - pipPriority))));
        }

        if(counterDisabled)
        {
            __LOG(("PIP : Global configuration not allow priority [%d] to count the PIP drops \n",
                pipPriority))
        }
        else
        {
            SCIB_SEM_TAKE;

            if(SMEM_CHT_IS_SIP5_20_GET(devObjPtr))
            {
                /* increment the counter in the shadow (not the actual registers/counters) */
                smemCheetahInternalSimulationUseMemForSip_5_20_RxdmaPipGlobalCountersIncrement(devObjPtr,unitIndex,pipPriority);
            }
            else
            if(SMEM_CHT_IS_SIP5_15_GET(devObjPtr))
            {
                /* increment the counter in the shadow (not the actual registers/counters) */
                smemCheetahInternalSimulationUseMemForSip_5_15_RxdmaPipGlobalCountersIncrement(devObjPtr,unitIndex,pipPriority);
            }
            else
            {
                GT_U32  regAddr_msb,regAddr_lsb,fldValue1,fldValue2;

                /* get into regAddr_msb,regAddr_lsb the addresses of 'rxdma pip global drop counters' (per pipPriority)*/
                SMEM_CHEETAH_SIP_5_10_RXDMA_PIP_DROP_GLOBAL_COUNTER_ADDR_GET(devObjPtr,unitIndex,pipPriority,regAddr_msb,regAddr_lsb);

                smemRegGet(devObjPtr, regAddr_msb, &fldValue1);
                smemRegGet(devObjPtr, regAddr_lsb, &fldValue2);
                __LOG(("PIP : priority [%d] counter : Increment PIP drop Counter from [0x%x](high32)[0x%x](low32) \n",
                    pipPriority,fldValue1,fldValue2));
                smemRegSet(devObjPtr, regAddr_lsb, ++fldValue2);
                if(fldValue2 == 0)
                {
                    smemRegSet(devObjPtr, regAddr_msb, ++fldValue1);
                }
            }

            SCIB_SEM_SIGNAL;
        }


        return GT_FALSE;
    }

    __LOG(("PIP : processing ended ... packet pass to the ingress pipe \n"));

    return GT_TRUE;
}


/**
* @internal internal_snetChtRxPort function
* @endinternal
*
* @brief   Rx Port layer processing of frames
*/
static GT_BOOL internal_snetChtRxPort
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr
)
{
    DECLARE_FUNC_NAME(internal_snetChtRxPort);

    GT_U32 * regPtr;            /* register entry pointer */
    GT_U32 regAddress;          /* register address */
    GT_U32 fldValue,fldValue1;  /* register's field address */
    GT_BOOL retVal;             /* return flag */
    GT_BOOL overSize;           /* frame size oversized flag */
    GT_U32         outputPortBit,outputPortBit1;  /* the bit index for the egress port  */
    GT_BOOL        isSecondRegister;/* is the per port config is in second register -- for bmp of ports */
    SNET_FLOW_CONTROL_PACKET_TYPE_ENT   fcPacketType=SNET_NOT_FC_PACKET_E;   /* classification of Flow Control packet */
    GT_U32  macPort;   /* MAC Port number */
    GT_U32  currentClock;   /* current clock value */
    GT_BOOL isEgressLoopback;

    /* setting of : devObjPtr->descrFreeIndx
       since the simulation actually have single task to process the packets ,
       we can be assure that no 'previous' descriptors for this device/portGroup
       are in use .

       it was moved from the functions of : snetChtFrameProcess
       because the traffic enter the device also from : snetChtFromCpuDmaProcess
    */
    devObjPtr->descrFreeIndx = 1;

    /* state the packet has L2 valid info */
    descrPtr->l2Valid = 1;

    descrPtr->bmpOfHemisphereMapperPtr = &descrPtr->bmpOfHemisphereMapper;

    macPort = descrPtr->ingressGopPortNumber;

    if(SMEM_CHT_IS_SIP5_GET(devObjPtr) ||
       SKERNEL_IS_XCAT3_BASED_DEV(devObjPtr))
    {
        smemDfxRegFldGet(devObjPtr, SMEM_LION2_DFX_SERVER_RESET_CONTROL_REG(devObjPtr), 0, 1, &fldValue);
    }
    else if(SKERNEL_IS_LION2_DEV(devObjPtr))
    {
        smemRegFldGet(devObjPtr, SMEM_LION2_DFX_SERVER_RESET_CONTROL_REG(devObjPtr), 0, 1, &fldValue);
    }
    else
    {
        smemRegFldGet(devObjPtr, SMEM_CHT_GLB_CTRL_REG(devObjPtr), 0, 1, &fldValue);
    }

    /* DeviceEn */
    if (fldValue != 1)
    {
        __LOG(("packet dropped : <DeviceEn> == 0"));
        return GT_FALSE;
    }

    overSize = GT_FALSE;

    isEgressLoopback = smemIsPortEgressLoopbackGet(devObjPtr, macPort);
    if (isEgressLoopback)
    {
        __LOG(("packet RX on Egress Loopback port %d\n", macPort));
        descrPtr->ingressBypassRxMacLayer = 1;
    }

    if(descrPtr->isFromSdma)
    {
        /* SDMA CPU port not care about mac related issues */
        __LOG(("CPU port as 'SDMA' not have 'link up' and 'port enabled' issues \n"));
    }
    else
    if(descrPtr->ingressBypassRxMacLayer)
    {
        /* do not do ingress MAC processing */
        __LOG(("The packet bypass MAC checks of : 'link up' and 'port enabled' \n"));
    }
    else
    {
        if(descrPtr->ingressGopPortNumber >= SKERNEL_DEV_MAX_SUPPORTED_PORTS_CNS)
        {
            skernelFatalError("internal_snetChtRxPort: invalid MAC number [%d] \n",
                descrPtr->ingressGopPortNumber);
        }

        if (CHT_IS_MTI_MAC_USED_MAC(devObjPtr, macPort))
        {
            /* check MTI port RX path enable status */
            fldValue = snetChtPortMacFieldGet(devObjPtr, macPort,
                SNET_CHT_PORT_MAC_FIELDS_rx_path_en_E);
        }
        else
        {
            /* check Port Enable bit for regular ports and MII/RGMII CPU Ports */
            fldValue = snetChtPortMacFieldGet(devObjPtr, macPort,
                SNET_CHT_PORT_MAC_FIELDS_PortEn_E);
        }
        /* PortEn */
        if (fldValue != 1)
        {
            __LOG(("packet dropped : <PortEn> == 0 \n"));
            return GT_FALSE;
        }

        if(SMEM_CHT_IS_SIP5_GET(devObjPtr) ||
           (macPort != SNET_CHT_CPU_PORT_CNS))
        {
            fldValue = snetChtPortMacFieldGet(devObjPtr, macPort,
                SNET_CHT_PORT_MAC_FIELDS_LinkState_E);
            /* LinkUp */
            if (fldValue != 1)
            {
                __LOG(("packet dropped : <LinkUp> == 0 \n"));
                return GT_FALSE;
            }
        }


        if(SMEM_CHT_IS_SIP6_10_GET(devObjPtr))
        {
            GT_U32  local_dma_port_number;
            GT_U32  global_dma_Rx_channel_id_number;
            GT_U32  dpUnitIndex;/*local DP in the pipe*/
            GT_U32  pipeId;
            GT_U32  mif_is_Rx_channel_enable;
            GT_U32  mif_is_clock_enable;
            GT_U32  isPreemptiveChannel;

            isPreemptiveChannel = smemGetCurrentTaskExtParamValue(devObjPtr,TASK_EXT_PARAM_INDEX_IS_PREEMPTIVE_CHANNEL);

            snetHawkMifIngressMacToRxDmaConvert(
                devObjPtr,macPort,isPreemptiveChannel,
                &mif_is_clock_enable,
                &mif_is_Rx_channel_enable,
                &local_dma_port_number);

            if(!mif_is_clock_enable)
            {
                __LOG(("packet dropped : <mif_is_clock_enable> == 0 \n"));
                return GT_FALSE;
            }

            if(!mif_is_Rx_channel_enable)
            {
                __LOG(("packet dropped : <mif_is_Rx_channel_enable> == 0 \n"));
                return GT_FALSE;
            }

            dpUnitIndex = SMEM_DUAL_UNIT_DMA_UNIT_INDEX_GET(devObjPtr,descrPtr->ingressRxDmaPortNumber);
            dpUnitIndex %= devObjPtr->multiDataPath.maxDp;/*local DP in the pipe*/

            pipeId = smemGetCurrentPipeId(devObjPtr);

            /* convert local dma to global dma */
            smemConvertPipeIdAndLocalPortToGlobal_withPipeAndDpIndex(
                devObjPtr,
                pipeId,/* pipe id */
                dpUnitIndex,           /* dp unit index */
                local_dma_port_number, /* local dma in DP unit */
                GT_FALSE,/* not CPU SDMA */
                &global_dma_Rx_channel_id_number);

            if(isPreemptiveChannel)
            {
                __LOG(("MIF : global mac[%d] : 'PREEMPTIVE MAC' convert local_dma_port_number[%d] to global_dma_Rx_channel_id_number[%d] on dp unit[%d] pipeId[%d] \n",
                    macPort,
                    local_dma_port_number,
                    global_dma_Rx_channel_id_number,
                    dpUnitIndex,
                    pipeId));
            }
            else
            {
                __LOG(("MIF : global mac[%d] : convert local_dma_port_number[%d] to global_dma_Rx_channel_id_number[%d] on dp unit[%d] pipeId[%d] \n",
                    macPort,
                    local_dma_port_number,
                    global_dma_Rx_channel_id_number,
                    dpUnitIndex,
                    pipeId));
            }

            if(global_dma_Rx_channel_id_number != descrPtr->ingressRxDmaPortNumber)
            {
                __LOG(("Warning : MIF : potential configuration ERROR : for MAC[%d] global_mif_Rx_channel_id_number[%d] != descrPtr->ingressRxDmaPortNumber[%d] \n",
                    macPort,
                    global_dma_Rx_channel_id_number,
                    descrPtr->ingressRxDmaPortNumber));

                descrPtr->ingressRxDmaPortNumber = global_dma_Rx_channel_id_number;
                __LOG_PARAM(descrPtr->ingressRxDmaPortNumber);

            }
        }
    }

    currentClock = SIM_OS_MAC(simOsTickGet)();
    descrPtr->packetTimestamp = SNET_CONVERT_MILLISEC_TO_TIME_STAMP_MAC(currentClock);

    /* IPFix Time stamp value is measured in milliseconds */
    descrPtr->ipFixTimeStampValue = currentClock;
    descrPtr->macDaPtr = DST_MAC_FROM_DSCR(descrPtr);
    descrPtr->macSaPtr = SRC_MAC_FROM_DSCR(descrPtr);

    /* Fill MAC data type of descriptor */
    if (SGT_MAC_ADDR_IS_MCST(descrPtr->macDaPtr))
    {
        if (SGT_MAC_ADDR_IS_BCST(descrPtr->macDaPtr))
        {
            descrPtr->macDaType = SKERNEL_BROADCAST_MAC_E;
            __LOG(("macDaType == SKERNEL_BROADCAST_MAC_E"));
        }
        else
        {
            descrPtr->macDaType = SKERNEL_MULTICAST_MAC_E;
            __LOG(("macDaType == SKERNEL_MULTICAST_MAC_E"));
        }
    }
    else
    {
        descrPtr->macDaType = SKERNEL_UNICAST_MAC_E;
        __LOG(("macDaType == SKERNEL_UNICAST_MAC_E"));
    }

    if(descrPtr->isFromSdma)
    {
        /* SDMA CPU port not care about mac related issues */
        __LOG(("CPU port as 'SDMA' not have 'flow control' and 'MAC mib counters' issues \n"));
    }
    else
    if(descrPtr->ingressBypassRxMacLayer)
    {
        /* do not do ingress MAC processing */
        __LOG(("The packet bypass MAC checks of :  'flow control' and 'MAC mib counters' \n"));
    }
    else
    if(snetChtDoForwardFcFrame(devObjPtr, descrPtr, &fcPacketType) == GT_FALSE)
    {
        /* protect RX MAC MIB counters from simultaneous access with management. */
        SCIB_SEM_TAKE;

        /* Update pause frame MAC counter */
        snetChtRxMacCountUpdate(devObjPtr, descrPtr, overSize, fcPacketType, macPort);

        SCIB_SEM_SIGNAL;

        __LOG(("packet dropped : not forward FC frame"));

        return GT_FALSE;
    }

    if(SMEM_CHT_IS_SIP7_GET(devObjPtr))
    {
        smemSip6DfxServerOwnDevNum(devObjPtr,GT_FALSE,&fldValue);

        __LOG(("SIP7 : use DFX-SERVER unit to get the <ownDev> [0x%3.3x] value \n",
            fldValue));
    }
    else
    if(SMEM_CHT_IS_SIP6_GET(devObjPtr))
    {
        smemSip6DfxServerOwnDevNum(devObjPtr,GT_FALSE,&fldValue);

        __LOG(("SIP6 : use DFX-SERVER unit to get the <ownDev> [0x%3.3x] value (not from MG unit) \n",
            fldValue));

        /* compare the MG value with the DFX-value ... just to notify the LOG in case of mismatch */
        smemRegFldGet(devObjPtr, SMEM_CHT_GLB_CTRL_REG(devObjPtr), 4,
            devObjPtr->flexFieldNumBitsSupport.hwDevNum,
            &fldValue1);

        if(fldValue != fldValue1)
        {
            __LOG(("Warning : the MG <ownDev> [0x%3.3x] but DFX <ownDev> is [0x%3.3x] (using DFX value) \n",
                fldValue1,/*MG*/
                fldValue  /*DFX*/));
        }

    }
    else
    {
        smemRegFldGet(devObjPtr, SMEM_CHT_GLB_CTRL_REG(devObjPtr), 4,
            devObjPtr->flexFieldNumBitsSupport.hwDevNum,
            &fldValue);

        __LOG(("use MG unit to get the <ownDev> [0x%3.3x] value \n",
            fldValue));
    }
    /* ownDevNum */
    descrPtr->ownDev = fldValue;

    /* set the 'hard wired' port group Id of the port group */
    descrPtr->srcCoreId = devObjPtr->portGroupId;

    if(SMEM_CHT_IS_SIP7_GET(devObjPtr))
    {
        smemRegFldGet(devObjPtr,
            SMEM_SIP6_RXDMA_CHANNEL_GENERAL_CONFIG_REG(
                devObjPtr,descrPtr->ingressRxDmaPortNumber),
            0,1,&descrPtr->marvellTagged);

        smemRegFldGet(devObjPtr,
            SMEM_SIP6_RXDMA_CHANNEL_GENERAL_CONFIG_REG(
                devObjPtr, descrPtr->ingressRxDmaPortNumber),
            4,2,&fldValue);
        descrPtr->rxRecalcCrc = fldValue ? GT_TRUE : GT_FALSE;
    }
    else
    if(SMEM_CHT_IS_SIP6_GET(devObjPtr))
    {
        smemRegFldGet(devObjPtr,
            SMEM_SIP6_RXDMA_CHANNEL_CASCADE_PORT_REG(devObjPtr,descrPtr->ingressRxDmaPortNumber),
            0,1,&descrPtr->marvellTagged);

        smemRegGet(devObjPtr,
            SMEM_SIP6_RXDMA_CHANNEL_GENERAL_CONFIG_REG(
                devObjPtr, descrPtr->ingressRxDmaPortNumber),
            &fldValue);
        descrPtr->rxRecalcCrc = (fldValue & 1) ? GT_TRUE : GT_FALSE;
    }
    else
    if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        /* Cascade port enable */
        smemRegFldGet(devObjPtr,
            SMEM_LION3_RXDMA_SCDMA_CONFIG_0_REG(devObjPtr,descrPtr->ingressRxDmaPortNumber),
            3,1,&descrPtr->marvellTagged);
    }
    else if(devObjPtr->txqRevision) /* LionB , Lion2 */
    {
        /* Cascade port enable */
        smemRegFldGet(devObjPtr,
            SMEM_LION_RXDMA_CUT_THROUGH_CASCADING_PORT_REG(devObjPtr),
            (descrPtr->ingressRxDmaPortNumber & 0xf),1,&descrPtr->marvellTagged);
    }
    else
    {
        /* !!! get register from the HA unit !!! */
        __LOG(("!!! get <marvellTagged> indication from the HA unit !!!"));
        snetChtHaPerPortInfoGet(devObjPtr,descrPtr->ingressRxDmaPortNumber,&isSecondRegister,&outputPortBit);

        if (descrPtr->ingressRxDmaPortNumber == SNET_CHT_CPU_PORT_CNS && /* send packet to cpu */
            devObjPtr->supportHa64Ports == 0)
        {
            /* get the cpu port bit index */
            outputPortBit1 = SMEM_CHT_GET_CPU_PORT_BIT_INDEX_IN_CSCD_STATUS(devObjPtr);
        }
        else
        {
            outputPortBit1 = outputPortBit;
        }

        regAddress = SMEM_CHT_CAS_HDR_INS_REG(devObjPtr);
        if(isSecondRegister == GT_TRUE)
        {
            regAddress +=4;
        }

        regPtr = smemMemGet(devObjPtr, regAddress);
        descrPtr->marvellTagged = SMEM_U32_GET_FIELD(regPtr[0],outputPortBit1,1);
    }

    __LOG_PARAM(descrPtr->marvellTagged);

    if(descrPtr->isFromSdma)
    {
        /* SDMA CPU port not care about mac related issues */
        retVal = GT_TRUE;

        if(descrPtr->byteCount < 64)
        {
            /* undersize --> drop it */
            retVal = GT_FALSE;
            __LOG(("packet dropped : undersize byteCount[%d] (less than 64 including CRC)",
                   descrPtr->byteCount));
        }
    }
    else
    if(descrPtr->ingressBypassRxMacLayer)
    {
        /* do not do ingress MAC processing */
        __LOG(("The packet bypass MAC checks of :  undersize byteCount \n"));
        retVal = GT_TRUE;/* not dropped */
        /* check ports d2d calendars . if the port dont have calendars slices, drop the packet */
        if(SMEM_CHT_IS_SIP6_GET(devObjPtr) && devObjPtr->numOfRavens)
        {
            if (sip6MACIngressIsChannelIdle(devObjPtr, macPort) == GT_FALSE)
            {
                retVal = GT_FALSE;
                __LOG(("Rx MAC Processing - packet dropped due to calendars miss configurations - port[%d]\n",
                       macPort));
            }
        }
    }
    else
    {
        if(CHT_IS_100G_MAC_USED_MAC(devObjPtr,macPort) &&
            descrPtr->byteCount < 64)
        {
            /* Packets under 64 bytes are dropped by CG IP, and not delivered
                any reject indication to DMA and no interrupt for them. */
            /* undersize --> drop it */
            retVal = GT_FALSE;
            __LOG(("packet dropped by CG MAC (100G MAC): undersize byteCount[%d] (less than 64 including CRC)",
                   descrPtr->byteCount));
        }
        else if (CHT_IS_MTI_MAC_USED_MAC(devObjPtr,macPort) &&
            descrPtr->byteCount < 64)
        {
            /* Packets under 64 bytes are dropped by MTI MAC and not delivered
                any reject indication to DMA and no interrupt for them. */
            /* undersize --> drop it */
            retVal = GT_FALSE;
            __LOG(("packet dropped by MTI MAC: undersize byteCount[%d] (less than 64 including CRC)",
                   descrPtr->byteCount));
        }
        else
        {
            fldValue = snetChtPortMacFieldGet(devObjPtr, macPort,
                SNET_CHT_PORT_MAC_FIELDS_mru_E);
            /* FrameSizeLimit - Maximal Received Packet Size (MRU)*/
            /* The resolution of this field is 2 bytes and
             * the default value is 1522 bytes => 0x2f9
             */
            if (fldValue >= descrPtr->byteCount)
            {
                overSize = GT_FALSE;
                retVal = GT_TRUE;
                if(descrPtr->byteCount < 60)
                {
                    /* undersize --> drop it */
                    retVal = GT_FALSE;
                    __LOG(("packet dropped : undersize byteCount[%d] (less than 60)",
                        descrPtr->byteCount));
                }
            }
            else
            {
                retVal = GT_FALSE;
                overSize = GT_TRUE;
                __LOG(("packet dropped : packet oversize : byteCount[%d] more than MRU [%d] \n",
                    descrPtr->byteCount,
                    fldValue));
            }
        }

        if(descrPtr->frameBuf->overSize != 0 &&
           overSize == GT_FALSE)
        {
            /* the packet is larger than the WM supports , we must drop the packet .
               it should have been dropped by the MRU limit !
            */
            retVal = GT_FALSE;
            overSize = GT_TRUE;
            __LOG(("packet dropped : packet oversize : byteCount[%d] more than the WM supports [%d] \n",
                descrPtr->byteCount,
                SBUF_DATA_SIZE_CNS));
        }

        if (CHT_IS_MTI_MAC_USED_MAC(devObjPtr,macPort))
        {
            fldValue = 1;
        }
        else
        {
            fldValue = snetChtPortMacFieldGet(devObjPtr, macPort,
                SNET_CHT_PORT_MAC_FIELDS_MIBCntEn_E);
        }

        /* MIBCntEn */
        if (fldValue)
        {
            /* protect RX MAC MIB counters from simultaneous access with management. */
            SCIB_SEM_TAKE;

            snetChtRxMacCountUpdate(devObjPtr, descrPtr, overSize, fcPacketType, macPort);

            SCIB_SEM_SIGNAL;
        }
        else
        {
            __LOG(("MIB counters are not allowed to be update for mac [%d] (update is disabled)\n" , macPort));
        }

        /* check ports d2d calendars . if the port dont have calendars slices, drop the packet */
        if(SMEM_CHT_IS_SIP6_GET(devObjPtr) && devObjPtr->numOfRavens)
        {
            if (sip6MACIngressIsChannelIdle(devObjPtr, macPort) == GT_FALSE)
            {
                retVal = GT_FALSE;
                __LOG(("Rx MAC Processing - packet dropped due to calendars miss configurations - port[%d]\n",
                       macPort));
            }
        }

        if(retVal == GT_FALSE)
        {
            /* the Mac dropped the packet */
            return GT_FALSE;
        }
    }

    /* Set initial values for target device/port */
    descrPtr->trgEPort = SNET_CHT_NULL_PORT_CNS;
    descrPtr->trgDev = descrPtr->ownDev;
    descrPtr->eArchExtInfo.isTrgPhyPortValid = 0;
    descrPtr->eArchExtInfo.trgPhyPort = SNET_CHT_NULL_PORT_CNS;
    /* Use Ingress VID modification */
    descrPtr->useIngressPipeVid = GT_TRUE;
    descrPtr->tag0Ptr =  NULL;
    descrPtr->tag1Ptr =  NULL;
    /* not initialized yet - the 'per port' is done in the L2I unit */
    descrPtr->sstId = 0;

    /* the bc2 CPU port hold no GOP and value of ingressGopPortNumber == SMAIN_NOT_VALID_CNS,
       so need to avoid access violation into devObjPtr->portsArr[] */
    if( (descrPtr->ingressGopPortNumber < SKERNEL_DEV_MAX_SUPPORTED_PORTS_CNS) &&
        devObjPtr->portsArr[descrPtr->ingressGopPortNumber].
                                physicalToVirtualPortMappingEn == GT_TRUE)
    {
        descrPtr->localDevSrcPort =
            devObjPtr->portsArr[descrPtr->ingressGopPortNumber].virtualMapping;
        __LOG(("Hard-coded mapping from physical port[%d] to virtual port[%d] on the field of <localDevSrcPort>",
                  descrPtr->ingressGopPortNumber,descrPtr->localDevSrcPort));
    }
    else if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        if(GT_FALSE == snetLion3RxdmaToTti(devObjPtr, descrPtr))
        {
            /* the RxDma drops the packet */
            return GT_FALSE;
        }
    }

    if(SMEM_CHT_IS_SIP7_GET(devObjPtr))
    {
        GT_U32  pchEnabled;
        smemRegFldGet(devObjPtr,
            SMEM_SIP6_RXDMA_CHANNEL_GENERAL_CONFIG_REG(devObjPtr,descrPtr->ingressRxDmaPortNumber),
            1,1,&pchEnabled);

        if(pchEnabled)
        {
            GT_U32  dpCore = descrPtr->ingressRxDmaUnitId;
            GT_U32  localDpPort = descrPtr->ingressRxDmaPortNumber_forBma;
            GT_U32  index =
                (localDpPort & 0x7F) |    /* 7 bits */
                ((dpCore & 0x3) << 7 );   /* 2 bits */

            /* HBU port mapping */
            regAddress = SMEM_SIP7_HBU_PORT_MAPPING_TBL_MEM(devObjPtr,index);
            smemRegFldGet(devObjPtr, regAddress,0,10,&fldValue);

            __LOG(("HBU : access to 'port mapping' table index[0x%x] (dpCore[%d],localDpPort[%d]) , got value localDevSrcPort[0x%x] ([%d]) \n",
                index,dpCore,localDpPort,fldValue/*in HEX*/,fldValue/*in DEC*/));

            if(fldValue != descrPtr->localDevSrcPort)
            {
                __LOG(("sip7 : warning : HBU port mapping will change localDevSrcPort from [%d] to [%d] \n",
                    descrPtr->localDevSrcPort,fldValue));
                descrPtr->localDevSrcPort = fldValue;
            }
        }
    }

    /* NOTE: the functionality had errata in EGF of sip5 , and it was decided to
       remove the configuration in sip5_10 */
    if(!SMEM_CHT_IS_SIP5_10_GET(devObjPtr))
    {
        /* get value of 'basic mode' from MG global control register .*/
        smemRegFldGet(devObjPtr, SMEM_CHT_GLB_CTRL_REG(devObjPtr), 3, 1, &descrPtr->basicMode);
    }

    if(retVal == GT_TRUE) /* packet not dropped by the MAC */
    {
        if(descrPtr->ingressBypassRxMacLayer)
        {
            /* do not do ingress MAC processing */
            __LOG(("The packet bypass MAC checks of :  EEE support \n"));
        }
        else
        if(!descrPtr->isFromSdma)
        {
            /* check for EEE support */
            snetBobcat2EeeProcess(devObjPtr,macPort,SMAIN_DIRECTION_INGRESS_E);
        }

        if(SMEM_CHT_IS_SIP6_GET(devObjPtr))
        {
            /* do RxDma parser looking for 'L3 layer' */
            snetSip6RxDmaParser(devObjPtr,descrPtr);

            /* Rx Port layer Cut Through triggering
                need to be after 'RxDma parser' */
            snetSip6ChtRxPortCutThrough(devObjPtr, descrPtr);
        }

        if(SMEM_CHT_IS_SIP5_10_GET(devObjPtr))
        {
            /* do Pre-Ingress Prioritization (PIP) */
            retVal = snetSip5_10RxDmaPipCheck(devObjPtr,descrPtr);
        }

        if(SMEM_CHT_IS_SIP6_GET(devObjPtr))
        {
            /* update debug counters */
            snetSip6ChtRxPortDebugRxToCpCounterUpdate(devObjPtr,descrPtr);
        }

        if(SMEM_CHT_IS_SIP6_30_GET(devObjPtr))
        {
            /* add the last 6 bytes (that comes before the CRC) of the packet to the descriptor so TTI can use */
            GT_U16  _2bytes;
            GT_U8   *prpPtr = &descrPtr->startFramePtr[
                                    descrPtr->rxRecalcCrc ? descrPtr->origByteCount - (6+4) :
                                                            descrPtr->origByteCount - 6];

            _2bytes = prpPtr[0] << 8 | prpPtr[1];/*16 bits*/
            descrPtr->prpInfo.prpSequenceNumber = _2bytes;

            _2bytes = prpPtr[2] << 8 | prpPtr[3];/*16 bits*/
            descrPtr->prpInfo.prpLanId          = _2bytes >> 12;/*4 bits*/
            descrPtr->prpInfo.prpLSDUSize       = _2bytes & 0xFFF;/*12 bits*/

            _2bytes = prpPtr[4] << 8 | prpPtr[5];/*16 bits*/
            descrPtr->prpInfo.prpSuffix         = _2bytes;/*16 bits*/
        }
    }

    return retVal;
}

/**
* @internal snetSip6ChtRxPortCutThrough function
* @endinternal
*
* @brief   Rx Port layer Cut Through triggering
*/
static GT_VOID snetSip6ChtRxPortCutThrough
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr
)
{
    /* cut through triggering */
    DECLARE_FUNC_NAME(snetSip6ChtRxPortCutThrough);

    GT_U32 addr;
    GT_U32 rxDmaUnitIndex;
    GT_U8  *packetDataPtr;
    GT_U8  *packetPtr;
    GT_U32 regValue;
    GT_U32 ctPortEnable;
    GT_U32 ctPortUntaggedEnable;
    GT_U32 ctPortMinCtByteCount;
    GT_U32 ctUpEnable;
    GT_U32 ctByteCount;
    GT_U32 ctUdeEnable;
    GT_U32 ctEnableAny;
    GT_U32 ctEnableLlc;
    GT_U32 ctEnableIpv4;
    GT_U32 ctEnableIpv6;
    GT_U32 ctEnableMpls;
    GT_U32 CT2SFEnable;
    GT_U32 index;
    GT_U16 configVlanEtherType;
    GT_U16 packetEtherType;
    GT_U32 isTagged;
    GT_U32 up;
    GT_U32 pchConfig;


    rxDmaUnitIndex = SMEM_DUAL_UNIT_DMA_UNIT_INDEX_GET(
        devObjPtr,descrPtr->ingressRxDmaPortNumber);
    packetDataPtr = descrPtr->frameBuf->actualDataPtr;

    /* cut through enable per port */
    addr =
        SMEM_SIP6_RXDMA_CHANNEL_CUT_THROUGH_CONFIG_REG(devObjPtr,descrPtr->ingressRxDmaPortNumber);
    smemRegGet(devObjPtr, addr, &regValue);
    __LOG(
        ("RX_DMA Cut Through - port: %d, channel_cut_through_config addr 0x%08X regValue 0x%08X",
        descrPtr->ingressRxDmaPortNumber, addr, regValue));
    ctPortEnable = (1 & regValue);
    if (ctPortEnable == 0)
    {
        __LOG(("RX_DMA Cut Through Disabled on port: [%d]\n",
            descrPtr->ingressRxDmaPortNumber));
        return;
    }

    __LOG(("RX_DMA Cut Through Enabled on port: [%d]\n",
        descrPtr->ingressRxDmaPortNumber));

    ctPortUntaggedEnable = (1 & (regValue >> 1));
    ctPortMinCtByteCount = (0x3FFF & (regValue >> 2));


    /* check if L2/L3 allow CT , using 'parser' logic */
    addr = SMEM_SIP6_RXDMA_CUT_THROUGH_CONFIG_GENERAL_CONFIG_REG_MAC(devObjPtr, rxDmaUnitIndex);
    smemRegGet(devObjPtr, addr, &regValue);
    __LOG(
        ("RX_DMA Cut Through - cut_through_gen_config addr 0x%08X regValue 0x%08X",
        addr, regValue));

    if(SMEM_CHT_IS_SIP6_10_GET(devObjPtr))
    {
        ctEnableAny      = (1 & (regValue >> 19));
        ctEnableMpls     = (1 & (regValue >> 18)); /* Hawk */
    }
    else
    {
        ctEnableAny      = (1 & (regValue >> 18));
        ctEnableMpls     = 0; /*not supported by Falon */
    }
    ctEnableLlc      = (1 & (regValue >> 17));
    ctEnableIpv4     = (1 & (regValue >> 16));
    ctEnableIpv6     = (1 & (regValue >> 15));
    ctByteCount      = (0x3FFF & (regValue >> 1));
    CT2SFEnable      = (1 & (regValue >> 0));

    packetEtherType = descrPtr->rxdmaParser.ethertypePtr[0] << 8 |
                      descrPtr->rxdmaParser.ethertypePtr[1];

    if(ctEnableAny)
    {
        __LOG(("allow all packets for checking CT \n"));
    }
    else
    if (descrPtr->rxdmaParser.packetType == SNET_RXDMA_PARSER_PACKET_TYPE_GENERIC_E)
    {
        index = descrPtr->rxdmaParser.indexOfMatch;

        /* found generic ethernet type */
        smemRegGet(devObjPtr,
                   SMEM_SIP6_RXDMA_CUT_THROUGH_CONFIG_GENERIC_ETH_CONFIG_REG_MAC(
                       devObjPtr, index, rxDmaUnitIndex), &regValue);
        ctUdeEnable = ((regValue >> 16) & 1);
        if(ctUdeEnable)
        {
            __LOG(("allow 'generic' (UDE) packets for checking CT \n"));
        }
        else
        {
            __LOG(("deny 'generic' (UDE) packets for checking CT \n"));
            return;
        }
    }
    else
    if (descrPtr->rxdmaParser.packetType == SNET_RXDMA_PARSER_PACKET_TYPE_LLC_E &&
        packetEtherType != 0x8870)
    {
        if(ctEnableLlc)
        {
            __LOG(("allow 'LLC' (that are not 0x8870) packets for checking CT \n"));
        }
        else
        {
            __LOG(("deny 'LLC' (that are not 0x8870) packets for checking CT \n"));
            return;
        }
    }
    else
    if (descrPtr->rxdmaParser.packetType == SNET_RXDMA_PARSER_PACKET_TYPE_IPV4_E)
    {
        if(ctEnableIpv4)
        {
            __LOG(("allow 'ipv4' packets for checking CT \n"));
        }
        else
        {
            __LOG(("deny 'ipv4' packets for checking CT \n"));
            return;
        }
    }
    else
    if (descrPtr->rxdmaParser.packetType == SNET_RXDMA_PARSER_PACKET_TYPE_IPV6_E)
    {
        if(ctEnableIpv6)
        {
            __LOG(("allow 'ipv6' packets for checking CT \n"));
        }
        else
        {
            __LOG(("deny 'ipv6' packets for checking CT \n"));
            return;
        }
    }
    else
    if (descrPtr->rxdmaParser.packetType == SNET_RXDMA_PARSER_PACKET_TYPE_MPLS_E)
    {
        if(ctEnableMpls)
        {
            __LOG(("allow 'Mpls' packets for checking CT \n"));
        }
        else
        {
            __LOG(("deny 'Mpls' packets for checking CT \n"));
            return;
        }
    }
    else
    {
        __LOG(("The packet type not applicable for CT \n"));
        return;
    }

    if (descrPtr->rxdmaParser.packetType == SNET_RXDMA_PARSER_PACKET_TYPE_GENERIC_E)
    {
        index = descrPtr->rxdmaParser.indexOfMatch;

        /* found generic ethernet type */
        smemRegGet(devObjPtr,
                   SMEM_SIP6_RXDMA_CUT_THROUGH_CONFIG_GENERIC_ETH_CONFIG_REG_MAC(
                       devObjPtr, index, rxDmaUnitIndex), &regValue);

        /* use dedicated byte count per UDE and not 'per port' */
        ctByteCount  = (0x3FFF & regValue);

        if (ctByteCount != 0x3FFF)
        {
            if(SMEM_CHT_IS_SIP7_GET(devObjPtr))
            {
                smemRegFldGet(devObjPtr,
                    SMEM_SIP6_RXDMA_CHANNEL_GENERAL_CONFIG_REG(
                        devObjPtr,descrPtr->ingressRxDmaPortNumber),
                    1,1,&pchConfig);
            }
            else
            {
                smemRegGet(devObjPtr,
                    SMEM_SIP6_RXDMA_CHANNEL_PCH_CONFIG_REG(
                        devObjPtr, descrPtr->ingressRxDmaPortNumber),
                    &pchConfig);
            }

            /* For Ironman PCH already out byte count in RXDMA */
            if ((pchConfig & 1) && (SMEM_CHT_IS_SIP6_30_GET(devObjPtr) == 0))
            {
                if (ctByteCount < 8)
                {
                    __LOG(("'generic' (UDE) packet wrong configured byte count %d \n", ctByteCount));
                    return;
                }
                /* remove PCH data size */
                ctByteCount -= 8;
                __LOG(("'generic' (UDE) packet byte count decreased by 8 due to PCH data \n"));
            }

            if (descrPtr->rxRecalcCrc != GT_FALSE)
            {
                ctByteCount += 4;
                __LOG(("'generic' (UDE) packet byte count increased by 4 due to rxRecalcCrc \n"));
            }
        }

        __LOG(("RX_DMA Cut Through - packet UDE index [%d] with ctUdeByteCount[0x%04X]",
            index,  ctByteCount));
    }

    /* skip MAC_DA and MAC_SA */
    packetPtr = packetDataPtr + 12;

    if (descrPtr->marvellTagged == 0)
    {
        /* check if packet tagged */
        isTagged = 0;
        up = 0;
        packetEtherType = (((GT_U16)packetPtr[0] << 8) + packetPtr[1]);

        /* check if one of the configuration ether types match the ethertype of the packet */
        for (index = 0; index < 4; index++)
        {
            smemRegGet(devObjPtr,
                SMEM_SIP_5_10_RXDMA_PIP_VLAN_ETHERTYPE_CONF_N_REG(devObjPtr, index, rxDmaUnitIndex),
                &regValue);
            configVlanEtherType = (GT_U16)(regValue & 0xFFFF);

            if (packetEtherType == configVlanEtherType)
            {
                isTagged = 1;
                up = ((packetPtr[2] >> 5) & 0x7);

                __LOG(("[%d] RX_DMA Cut Through - configuration ethertype[0x%4.4x] MATCH packet etherType \n",
                    index,
                    configVlanEtherType));
                break;
            }
            else
            {
                __LOG(("[%d] RX_DMA Cut Through - configuration ethertype[0x%4.4x] not match packet etherType[0x%4.4x] \n",
                    index,
                    configVlanEtherType,
                    packetEtherType));
            }
        }

        if(isTagged)
        {
            __LOG(("RX_DMA Cut Through - packet without DSA Tag , 'tagged' with up[%d] \n",
                up));
        }
        else
        {
            __LOG(("RX_DMA Cut Through - packet without DSA Tag , 'untagged' (not have 'up') \n"));
        }
    }
    else
    {
        /* DSA tagged packet                                       */
        isTagged = ((packetPtr[0] >> 5) & 1); /* word0 bit[29]     */
        up       = ((packetPtr[2] >> 5) & 7); /* word0 bits[15:13] */


        if(isTagged)
        {
            __LOG(("RX_DMA Cut Through - packet with DSA Tag , considered 'tagged' with up[%d] \n",
                up));
        }
        else
        {
            __LOG(("RX_DMA Cut Through - packet with DSA Tag , considered 'untagged' (not using 'up') \n"));
        }
    }

    if ((ctPortUntaggedEnable == 0) && (isTagged == 0))
    {
        __LOG(("RX_DMA Cut Through not allow : for untagged packets \n"));
        return;
    }

    /* check cut through enable by UP */
    if (isTagged)
    {
        addr =
            SMEM_SIP6_RXDMA_CUT_THROUGH_CONFIG_UP_ENABLE_REG_MAC(devObjPtr, rxDmaUnitIndex);
        smemRegGet(devObjPtr, addr, &ctUpEnable);
        __LOG(
            ("RX_DMA Cut Through - UP[%d] is [%s] for cut through \n",
            up, (ctUpEnable & (1 << up)) ? "enabled" : "disabled"));
        if ((ctUpEnable & (1 << up)) == 0)
        {
            __LOG(("RX_DMA Cut Through not allow : for tagged packets \n"));
            return;
        }
    }

    /* save to use in ingress control pipe */
    if (ctPortMinCtByteCount >= descrPtr->frameBuf->actualDataSize)
    {
        ctByteCount  = descrPtr->frameBuf->actualDataSize;
    }
    else
    {
        __LOG(("RX_DMA Cut Through : the byte count is 'unknown' and maybe set by the TTI unit \n"));
        __LOG(("RX_DMA Cut Through : currently use value from the configurations <byteCount> = [0x%x] \n" ,
            ctByteCount));
        descrPtr->sip6_isCtByteCount    = 1;
    }

    descrPtr->byteCount            = ctByteCount;
    descrPtr->origByteCount        = ctByteCount;
    descrPtr->cutThroughModeEnabled = 1;

    /* save to use in egress control pipe */
    descrPtr->cutThrough2StoreAndForward = CT2SFEnable ;

    __LOG_PARAM(descrPtr->byteCount);
    __LOG_PARAM(descrPtr->origByteCount);
    __LOG_PARAM(descrPtr->cutThroughModeEnabled);
    __LOG_PARAM(descrPtr->cutThrough2StoreAndForward);
    __LOG_PARAM(descrPtr->sip6_isCtByteCount);

    __LOG(("RX_DMA Cut Through : The Packet is considered 'low latency' \n"));
}

/**
* @internal snetSip6ChtRxPortDebugRxToCpCounterUpdate function
* @endinternal
*
* @brief   Rx to CP counter update
*/
GT_VOID snetSip6ChtRxPortDebugRxToCpCounterUpdate
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr
)
{
    GT_U32 addr;
    GT_U32 rxDmaUnitIndex;
    GT_U32 channel;
    GT_U32 countValue;
    GT_U32 refValue;
    GT_U32 maskValue;
    GT_U32 packetKey;

    DECLARE_FUNC_NAME(snetSip6ChtRxPortDebugRxToCpCounterUpdate);

    rxDmaUnitIndex = SMEM_DUAL_UNIT_DMA_UNIT_INDEX_GET(
        devObjPtr,descrPtr->ingressRxDmaPortNumber);
    channel = SMEM_DATA_PATH_RELATIVE_PORT_GET(
        devObjPtr,descrPtr->ingressRxDmaPortNumber);

    addr = SMEM_SIP6_RXDMA_RX_TO_CP_COUNT_TYPE_REF_REG_MAC(devObjPtr, rxDmaUnitIndex);
    smemRegGet(devObjPtr, addr, &refValue);

    addr = SMEM_SIP6_RXDMA_RX_TO_CP_COUNT_TYPE_MASK_REG_MAC(devObjPtr, rxDmaUnitIndex);
    smemRegGet(devObjPtr, addr, &maskValue);

    /* fields:                                     */
    /* bit0 0 - Store and Forward, 1 - Cut Through */
    /* bits2:1 - PIP priority                      */
    /* bit3 - BOOL Packet is Latency Sensitive     */
    /* bit4 - 0 -tail_disp, 1 - head_disp          */
    /* bits10:5 - channel ID                       */
    packetKey = 0;
    packetKey |= descrPtr->cutThroughModeEnabled;
    packetKey |= (descrPtr->pipPriority << 1);
    packetKey |= (channel << 5);

    if ((packetKey & maskValue) == (refValue & maskValue))
    {
        addr = SMEM_SIP6_RXDMA_RX_TO_CP_COUNTER_MAC(devObjPtr, rxDmaUnitIndex);
        smemRegGet(devObjPtr, addr, &countValue);
        countValue ++;
        smemRegSet(devObjPtr, addr, countValue);
        __LOG(
            ("RX_DMA Debug RX to CP counter incremented unit %d channel %d counter %d",
             rxDmaUnitIndex, channel, countValue));
    }
}

/**
* @internal snetSip6ChtRxPortDebugCpToRxCounterUpdate function
* @endinternal
*
* @brief   CP to Rx counter update
*/
GT_VOID snetSip6ChtRxPortDebugCpToRxCounterUpdate
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr
)
{
    GT_U32 addr;
    GT_U32 rxDmaUnitIndex;
    GT_U32 channel;
    GT_U32 countValue;
    GT_U32 refValue;
    GT_U32 maskValue;
    GT_U32 packetKey;
    GT_U32 channelOffset;

    DECLARE_FUNC_NAME(snetSip6ChtRxPortDebugCpToRxCounterUpdate);

    /* prevent additional increment RX to CP counter of the same packet */
    if (descrPtr->rxDmaDebugCpTpRxCntDone)
    {
        return;
    }
    descrPtr->rxDmaDebugCpTpRxCntDone = 1;

    rxDmaUnitIndex = SMEM_DUAL_UNIT_DMA_UNIT_INDEX_GET(
        devObjPtr,descrPtr->ingressRxDmaPortNumber);
    channel = SMEM_DATA_PATH_RELATIVE_PORT_GET(
        devObjPtr,descrPtr->ingressRxDmaPortNumber);

    addr = SMEM_SIP6_RXDMA_CP_TO_RX_COUNT_TYPE_REF_REG_MAC(devObjPtr, rxDmaUnitIndex);
    smemRegGet(devObjPtr, addr, &refValue);

    addr = SMEM_SIP6_RXDMA_CP_TO_RX_COUNT_TYPE_MASK_REG_MAC(devObjPtr, rxDmaUnitIndex);
    smemRegGet(devObjPtr, addr, &maskValue);

    /* fields:                                       */
    /* common:                                       */
    /* bit0 - 0 - unicast, 1 - multicast             */
    /* bit1 - 0 - SF, 1 - Cut Through                */
    /* bit2 - CT to SF packet type.                  */
    /* bit3 - truncated header                       */
    /* bit4 - dummy descriptor                       */
    /* Falcon:                                       */
    /* bits10:5 - channel ID                         */
    /* Hawk:                                         */
    /* bit5 - highPriorityPacket                     */
    /* bit6 - contextDone                            */
    /* bit7 - descriptorValid                        */
    /* bits13:8 - channel ID                         */
    /* Ironman:                                      */
    /* bit0 - multicastPacket                        */
    /* bit1 - cutThroughPacket                       */
    /* bit2 - cutThroughTerminated                   */
    /* bit3 - layer2CutThrough                       */
    /* bit4 - trunkatedHeader                        */
    /* bit5 - dummyDescriptor                        */
    /* bit6 - highPriorityPacket                     */
    /* bit7 - contextDone                            */
    /* bit8 - descriptorValid                        */
    /* bits14:9 - channel ID                         */

    packetKey = 0;
    packetKey |= descrPtr->isMultiTargetReplication;
    packetKey |= (descrPtr->cutThroughModeEnabled << 1);
    packetKey |= ((descrPtr->cutThroughModeTerminated | descrPtr->cutThrough2StoreAndForward) << 2);
    if (SMEM_CHT_IS_SIP6_30_GET(devObjPtr) && (descrPtr->egressByteCount > 0x3F00))
    {
        /* packet reached RX_DMA without calculated size in all CP */
        /* RX_DMA waits receiving all of it and uses actual size   */
        packetKey |= (1 << 2);
    }
    if (SMEM_CHT_IS_SIP6_30_GET(devObjPtr))
    {
        channelOffset = 9;
    }
    else
    {
        channelOffset = SMEM_CHT_IS_SIP6_10_GET(devObjPtr) ? 8 : 5;
    }
    packetKey |= (channel << channelOffset);

    if ((packetKey & maskValue) == (refValue & maskValue))
    {
        addr = SMEM_SIP6_RXDMA_CP_TO_RX_COUNTER_MAC(devObjPtr, rxDmaUnitIndex);
        smemRegGet(devObjPtr, addr, &countValue);
        countValue ++;
        smemRegSet(devObjPtr, addr, countValue);
        __LOG(
            ("RX_DMA Debug CP to RX counter incremented unit %d channel %d counter %d",
             rxDmaUnitIndex, channel, countValue));
    }
}

/**
* @internal sip6MACIngressIsChannelIdle function
* @endinternal
*
* @brief   Falcon : validate port configuration on Eagle D2D and
*          Raven D2D.
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] txFifoPort                - global dma port number.
* @param[in] direction                - Egress or Ingress
*
* @retval is port config               - GT_TRUE : port have at
*                                       least one slices.
*                                       GT_FALSE : port don't
*                                       have slices
*/
static GT_BOOL sip6MACIngressIsChannelIdle
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32               rxPort
)
{
    DECLARE_FUNC_NAME(sip6MACIngressIsChannelIdle);

    /*
    this function checks that the source port have gop calendars slices.
    the port connected to the gop through 2 D2D units.
    In Ingress direction, the relevant calendars are :
    D2D Raven side:
        Tx pcs calendar

    D2D Eagle side
        Rx mac calendr
        Rx pcs calendar
       */

    GT_BOOL retVal = GT_TRUE;
    GT_BOOL isPortD2dEagleConfig; /*indication that port config on Eagle */
    GT_BOOL isPortD2dRavenConfig; /*indication that port config on Raven */

    simLogPacketFrameUnitSet(SIM_LOG_FRAME_UNIT___ALLOW_ALL_UNITS___E);/* wild card for LOG without the unit filter */
    /* D2D on eagle side*/
    isPortD2dEagleConfig = sip6MacD2dEagleIsPortConfig( devObjPtr,rxPort,GT_TRUE /*ingress*/);
    if(isPortD2dEagleConfig == GT_FALSE)
    {
        __LOG(("miss port configurations on d2d Eagle side, ingress - port[%d] \n",rxPort));
        retVal = GT_FALSE;
    }

    /* D2D on raven side*/
    isPortD2dRavenConfig = sip6MacD2dRavenIsPortConfig( devObjPtr,rxPort,GT_TRUE /*ingress*/);
    if(isPortD2dRavenConfig == GT_FALSE)
    {
        __LOG(("miss port configurations on d2d Raven side, ingress - port[%d] \n",rxPort));
        retVal = GT_FALSE;
    }

    return retVal;
}

/**
* @internal snetLion3TtiDefaulteEportExceptionMruCheck function
* @endinternal
*
* @brief   sip5 TTI default ePort MRU exception check (in addition to MAC port's MRU check)
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] descrPtr             - in : pointer to frame data buffer Id
*                                       out: pointer to updated frame data buffer Id
*/
static GT_VOID snetLion3TtiDefaulteEportExceptionMruCheck
(
    IN    SKERNEL_DEVICE_OBJECT             *devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC   *descrPtr
)
{
    DECLARE_FUNC_NAME(snetLion3TtiDefaulteEportExceptionMruCheck);

    GT_U32 cpuCode;
    GT_U32 exceptionCmd;
    GT_U32 mruProfile;
    GT_U32 mruSize;
    GT_U32 isMruCheckEnabled;
    GT_U32 byteCount;

    if( NULL == descrPtr->eArchExtInfo.ttiPreTtiLookupIngressEPortTablePtr)
    {
        __LOG(("can never happen but just to be on the safe side \n"));
        return;
    }

    if(descrPtr->sip6_isCtByteCount)
    {
        __LOG(("Not doing default ePort MRU check , because packet is 'cut through' without valid <byte count>[%d]\n",
            descrPtr->byteCount));
        return;
    }

    smemRegFldGet(devObjPtr, SMEM_LION_TTI_INTERNAL_CONF_REG(devObjPtr), 4, 1, &isMruCheckEnabled);

    if(!isMruCheckEnabled)
    {
        __LOG(("TTI default ePort MRU Exception check is globally disabled\n"));
        return;
    }

    __LOG(("do TTI MRU Exception check\n"));

    mruProfile = SMEM_LION3_TTI_DEFAULT_EPORT_ENTRY_FIELD_GET(devObjPtr, descrPtr,
                            SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_MRU_INDEX);

    __LOG(("got default ePort MRU profile: [%d] \n", mruProfile));


    smemRegFldGet(devObjPtr, SMEM_LION3_TTI_MRU_SIZE_REG(devObjPtr, mruProfile/2),  mruProfile % 2 * 14, 14, &mruSize);

    byteCount = descrPtr->byteCount;

    if(descrPtr->is_byte_count_missing_4_bytes)
    {
        byteCount -= 4;
        __LOG(("got relevant MRU size: [%d] to compare with byteCount [%d] (missing 4 bytes CRC)\n", mruSize, byteCount));
    }
    else
    {
        __LOG(("got relevant MRU size: [%d] to compare with byteCount [%d] \n", mruSize, byteCount));
    }


    if(byteCount > mruSize)
    {
        __LOG(("there is MRU exception \n"));

        smemRegFldGet(devObjPtr, SMEM_LION3_TTI_MRU_EXCEPTION_COMMAND_REG(devObjPtr), 0, 8, &cpuCode);
        smemRegFldGet(devObjPtr, SMEM_LION3_TTI_MRU_EXCEPTION_COMMAND_REG(devObjPtr), 8, 3, &exceptionCmd);

        snetChtIngressCommandAndCpuCodeResolution(devObjPtr, descrPtr,
                                                  descrPtr->packetCmd,
                                                  exceptionCmd,
                                                  descrPtr->cpuCode,
                                                  cpuCode,
                                                  SNET_CHEETAH_ENGINE_UNIT_TTI_E,
                                                  GT_FALSE);
    }


}

/**
* @internal snetChtL2Parsing_Bpe_ETag_Format function
* @endinternal
*
* @brief   IEEE 802.1 header parsing. (xCat3)
*/
static GT_VOID snetChtL2Parsing_Bpe_ETag_Format
(
    IN SKERNEL_DEVICE_OBJECT          * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32                          tpid1Offset,
    IN SNET_CHT_FRAME_PARSE_MODE_ENT   parseMode
)
{
    GT_U32  bits0to15AfterTpid1 ;/* the bits  0..15 the ETag (after TPID)*/
    GT_U32  bits16to31AfterTpid1;/* the bits 16..31 the ETag (after TPID) */
    GT_U32  GRP;                 /* field GRP in the ETag */
    GT_U32  ingress_E_CID_base;  /* field <ingress_E_CID_base> in the ETag*/
    GT_U32  E_CID_base;          /* field <E_CID_base> in the ETag*/

    DECLARE_FUNC_NAME(snetChtL2Parsing_Bpe_ETag_Format);

    if(parseMode != SNET_CHT_FRAME_PARSE_MODE_PORT_E)
    {
        /* not for TT packets */
        return;
    }

    if(devObjPtr->support802_1br_PortExtender == 0)
    {
        /* not supported by this device */
        return;
    }

    if(descrPtr->firstVlanTagExtendedSize <= 4)
    {
        /* ETag was not recognized on this packet */
        return;
    }


    bits0to15AfterTpid1 =
        ((descrPtr->startFramePtr[(tpid1Offset) + 2] << 8) |
         (descrPtr->startFramePtr[(tpid1Offset) + 3]));
    bits16to31AfterTpid1 =
        ((descrPtr->startFramePtr[(tpid1Offset) + 4] << 8) |
         (descrPtr->startFramePtr[(tpid1Offset) + 5]));

    __LOG_PARAM(bits0to15AfterTpid1);
    __LOG_PARAM(bits16to31AfterTpid1);

    /* parse the ETag */
    ingress_E_CID_base = SMEM_U32_GET_FIELD(bits0to15AfterTpid1,0,12);
    E_CID_base = SMEM_U32_GET_FIELD(bits16to31AfterTpid1,0,12);
    GRP = SMEM_U32_GET_FIELD(bits16to31AfterTpid1,12,2);

    __LOG_PARAM(ingress_E_CID_base);
    __LOG_PARAM(E_CID_base);
    __LOG_PARAM(GRP);

    /* the 'standered' vid1 parsing assign vid the value from  <ingress_E-CID_base>
        but we need it from <E-CID_base> .. which located 2 bytes after.

       so we use 'extra' '+2' offset from where we usually take the value
    */

    if(GRP == 0)/* unicast (single destination) */
    {
        __LOG_PARAM_WITH_NAME("ETag with <GRP=0> (unicast (single destination)) --> recognize vid1 from:",E_CID_base);
        descrPtr->vid1 = E_CID_base;
    }
    else
    {
        __LOG_PARAM_WITH_NAME("ETag with <GRP!=0> (multicast (multi destination)) --> recognize vid1 from:",ingress_E_CID_base);
        descrPtr->vid1 = ingress_E_CID_base;
        __LOG_PARAM_WITH_NAME("ETag with <GRP!=0> (multicast (multi destination)) --> recognize vidx from:",E_CID_base);
        descrPtr->eVidx = E_CID_base;
        descrPtr->useVidx = 1;
    }

    /* save GRP for later use ... but this is for TTI unit only !!!
       as the HW device not have such field in descriptor between units
    */
    descrPtr->bpe802_1br_GRP = GRP;

    return;
}

/**
* @internal snetSip6CutThroughTtiSupport function
* @endinternal
*
* @brief   Cut through support in TTI unit
*          recalculation packet Byte count.
*/
static GT_VOID snetSip6CutThroughTtiSupport
(
    IN    SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN    SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN    GT_U32                           etherType,
    IN    INTERNAL_TTI_DESC_INFO_STC      *internalTtiInfoPtr
)
{
    DECLARE_FUNC_NAME(snetSip6CutThroughTtiSupport);
    GT_U32 byteCount;
    GT_U32 origByteCount;
    GT_U32 regValue;
    GT_U32 ctIpv4Enable;
    GT_U32 ctIpv6Enable;
    GT_U32 ctLlcEnable;
    GT_U32 llcSizeBasedL3Offset;
    GT_U8  *llcSizePtr;
    GT_U32 llcSize;
    GT_U32 index;
    GT_U32 bcSubtractOrAdd;
    GT_U32 bcSubtractOrAddValue;

    if ((descrPtr->origByteCount & 0x3F00) != 0x3F00)
    {
        /* byte count already calculated                   */
        /* the 0x3FFF is a default value                   */
        /* set by RX_DMA on triggering Cut Through mode    */
        /* for inner frame (passenger) it can be decreased */
        /* by calculation based on original value 0x3FFF   */
        __LOG(("This is NOT in Cut Through Mode, byte count [0x%x] is already calculated, returning from here\n",
               descrPtr->origByteCount));
        return;
    }

    origByteCount = 0; /* not-calculated */
    byteCount = 0;     /* not-calculated */

    smemRegGet(devObjPtr,
        SMEM_SIP6_TTI_CUT_THROUGH_CONFIG_REG_MAC(devObjPtr),
        &regValue);

    ctIpv4Enable = ((regValue >> 2) & 1);
    ctIpv6Enable = ((regValue >> 1) & 1);
    ctLlcEnable  = (regValue & 1);

    __LOG(
        ("TTI Cut Through - ctIpv4Enable %d ctIpv6Enable %d ctLlcEnable %d",
        ctIpv4Enable, ctIpv6Enable, ctLlcEnable));


    /* Descriptor Byte Count modification */
    if ((etherType == SKERNEL_L3_PROT_TYPE_IPV4_E) && (ctIpv4Enable))
    {/* Internet Protocol, Version 4 (IPv4) */
        /* IPV4-header-offset + total-length-from-IPV4-header */
        if (descrPtr->ttiIpv4CheckSumError == 0)
        {
            origByteCount =
                (descrPtr->l3StartOffsetPtr - descrPtr->frameBuf->actualDataPtr)
                + descrPtr->ipxLength + 4 /*L2 checksum*/;
            __LOG(("TTI Cut Through - Packet is IPV4 origByteCount %d", origByteCount));
        }
    }
    else if ((etherType == SKERNEL_L3_PROT_TYPE_IPV6_E) && (ctIpv6Enable))
    {/* Internet Protocol, Version 6 (IPv6) */
        /* IPV6-header-offset + IPV6-header-size + payload-length-from-IPV6-header */
        origByteCount =
            (descrPtr->l3StartOffsetPtr - descrPtr->frameBuf->actualDataPtr)
            + descrPtr->ipxLength + 4 /*L2 checksum*/;
        __LOG(("TTI Cut Through - Packet is IPV6 origByteCount %d", origByteCount));
    }
    else if (ctLlcEnable)
    {/* LLC */
        switch (descrPtr->l2Encaps)
        {
            case SKERNEL_LLC_E:
                llcSizeBasedL3Offset = 4; /* LLC size(2) + Ethertype size(2) */
                break;
            case SKERNEL_LLC_SNAP_E:
                llcSizeBasedL3Offset = 10; /* LLC size(2) + SNAP_hdr(6) + Ethertype size(2) */
                break;
            default:
                llcSizeBasedL3Offset = 0; /* not LLC */
                break;
        }

        if (llcSizeBasedL3Offset)
        {
            llcSizePtr = descrPtr->l3StartOffsetPtr - llcSizeBasedL3Offset;
            llcSize = (((llcSizePtr[0] << 8) | llcSizePtr[1]) & 0xFFFF);

            /* LLC-header-offset + payload-size */
            origByteCount =
                (llcSizePtr - descrPtr->frameBuf->actualDataPtr)
                + 2 /*ethertype*/ + llcSize + 4 /*L2 checksum*/;
            __LOG(("TTI Cut Through - Packet is LLC origByteCount %d", origByteCount));
        }
    }

    if (origByteCount != 0)
    {
        /* update descriptor fields */
        byteCount = origByteCount -
            ((descrPtr->l2Valid ? descrPtr->macDaPtr : descrPtr->l3StartOffsetPtr)
             - descrPtr->frameBuf->actualDataPtr);

        /* Fixed addition per port */

        index =  descrPtr->localDevSrcPort & 0xFF;

        if(index >= devObjPtr->TTI_myPhysicalPortAttributes_numEntries &&
           devObjPtr->TTI_myPhysicalPortAttributes_numEntries != 0)
        {
            __LOG(("Warning : the access to 'myPhysicalPortAttributes' will be at 'last index'[%d] instead of [%d] that is higher \n",
                devObjPtr->TTI_myPhysicalPortAttributes_numEntries - 1,
                index));
            /* use last entry of the table */
            index = devObjPtr->TTI_myPhysicalPortAttributes_numEntries - 1;
        }

        smemRegGet(devObjPtr,
            SMEM_LION3_TTI_MY_PHY_PORT_ATTRIBUTES_REG(devObjPtr, index),
            &regValue);

        if( SMEM_CHT_IS_SIP6_10_GET(devObjPtr))
        {
            bcSubtractOrAdd      = ((regValue >> 20) & 1);
            bcSubtractOrAddValue = ((regValue >> 17) & 7);
        }
        else
        {
            bcSubtractOrAdd      = ((regValue >> 19) & 1);
            bcSubtractOrAddValue = ((regValue >> 16) & 7);
        }

        if (bcSubtractOrAdd)
        {
            byteCount -= bcSubtractOrAddValue;
        }
        else
        {
            byteCount += bcSubtractOrAddValue;
        }

        __LOG_PARAM(bcSubtractOrAddValue);

        if (bcSubtractOrAddValue)
        {
            __LOG(("TTI Cut Through - Bytecount adjusted to %d", byteCount));
        }

        /* since we gave 'real value' of byte count ... not considered to hold 'CT byte count' value */
        descrPtr->sip6_isCtByteCount = 0;

        __LOG(("TTI Cut Through - Descriptor Byte count is updated"));

        /* update byteCount in descriptor */
        descrPtr->byteCount     = byteCount;
        descrPtr->origByteCount = descrPtr->byteCount;
        descrPtr->payloadLength = descrPtr->origByteCount -
                                   (descrPtr->payloadPtr -
                                     ((descrPtr->l2Valid ? descrPtr->macDaPtr : descrPtr->l3StartOffsetPtr)));

        __LOG_PARAM(descrPtr->byteCount);
        __LOG_PARAM(descrPtr->origByteCount);
        __LOG_PARAM(descrPtr->payloadLength);
    }
}

/**
* @internal snetSip6CutThroughTtiExceptionCheck function
* @endinternal
*
* @brief   Cut through support in TTI unit
*          Check TTI exception and increment parsing error counter.
*/
extern GT_VOID snetSip6CutThroughTtiExceptionCheck
(
    IN    SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN    SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr
)
{
    DECLARE_FUNC_NAME(snetSip6CutThroughTtiExceptionCheck);
    GT_U32 origByteCount;
    GT_U32 byteCount;
    GT_U32 regValue;
    GT_U32 hdrIntegrityException;
    GT_U32 hdrIntegrityIpv4Checksum;
    GT_U32 hdrIntegrityBcCheck;
    GT_U32 hdrIntegrityBcMin;
    GT_U32 hdrIntegrityBcMax;
    GT_U32 hdrIntegrityPacketCommand;
    GT_U32 hdrIntegrityCpuCode;


    origByteCount = descrPtr->origByteCount;
    if ((descrPtr->origByteCount & 0x3F00) == 0x3F00)
    {
        origByteCount = 0;
    }
    byteCount = descrPtr->byteCount;

    if (origByteCount == 0)
    {
        /* cannot calculate byteCount */
        /* increase CT_byte_count_extarction_fail_counter */
        smemRegGet(devObjPtr,
            SMEM_SIP6_TTI_CUT_THROUGH_BC_EXTRACTION_FAIL_COUNTER_REG_MAC(devObjPtr),
            &regValue);
        regValue ++;
        smemRegSet(devObjPtr,
            SMEM_SIP6_TTI_CUT_THROUGH_BC_EXTRACTION_FAIL_COUNTER_REG_MAC(devObjPtr),
            regValue);
        __LOG(("TTI Cut Through - Byte count not calculated fail-counter increased to %d", regValue));
    }

    /* Packet Header Integrity exception */
    smemRegGet(devObjPtr,
        SMEM_SIP6_TTI_CUT_THROUGH_HEADER_INTEGRITY_EXCEPTION_CONFIG1_REG_MAC(devObjPtr),
        &regValue);

    hdrIntegrityIpv4Checksum = ((regValue >> 29) & 1);
    hdrIntegrityBcCheck      = ((regValue >> 28) & 1);
    hdrIntegrityBcMin        = (regValue & 0x3FFF);
    hdrIntegrityBcMax        = ((regValue >> 14) & 0x3FFF);

    __LOG(
        ("TTI Cut Through - header integrity check Ipv4Checksum %d BcCheck %d BcMin %d BcMax %d  ",
          hdrIntegrityIpv4Checksum, hdrIntegrityBcCheck, hdrIntegrityBcMin, hdrIntegrityBcMax));

    hdrIntegrityException = 0;
    if (descrPtr->isIPv4)
    {/* Internet Protocol, Version 4 (IPv4) */
        /* IPV4-header-offset + total-length-from-IPV4-header */
        if (descrPtr->ttiIpv4CheckSumError == 0 ||
            hdrIntegrityIpv4Checksum       == 0)
        {
            if (descrPtr->ttiIpv4CheckSumError)
            {
                __LOG(("TTI Cut Through - the IPV4 header hold 'checkSum error' but we ignores it !!! \n"
                       "(because the check is disabled for TTI Cut Through) \n"));
            }
        }
        else
        {
            hdrIntegrityException = 1;
            __LOG(("TTI Cut Through - Packet is IPV4 with checkSum error , so do not modify origByteCount \n"));
        }
    }

    if (origByteCount != 0)
    {
        byteCount -= 4;/* the next calculations not include 4 bytes CRC ! */

        if(!hdrIntegrityBcCheck)
        {
            __LOG(("TTI Cut Through - NOT checking for out of boundaries (the check is disabled) \n"));
        }
        /* check byte count boundaries */
        else
        if ((byteCount < hdrIntegrityBcMin) ||
            (byteCount > hdrIntegrityBcMax))
        {
            __LOG(("TTI Cut Through - Byte count [%d] out of boundaries [%d..%d] \n", byteCount,
                hdrIntegrityBcMin,hdrIntegrityBcMax));
            hdrIntegrityException = 1;

            __LOG(("TTI Cut Through - set cutThroughModeTerminated = 1 to disable CT \n"));
            /* JIRA CPSS-15156: CT should be disabled when BC range check fails. */
            if(descrPtr->cutThroughModeEnabled)
            {
                descrPtr->cutThroughModeTerminated = 1;
            }
        }
        else
        {
            __LOG(("TTI Cut Through - Byte count [%d] is in the boundaries [%d..%d] \n", byteCount,
                hdrIntegrityBcMin,hdrIntegrityBcMax));
        }
    }

    if (hdrIntegrityException)
    {
        smemRegGet(devObjPtr,
            SMEM_SIP6_TTI_CUT_THROUGH_HEADER_INTEGRITY_EXCEPTION_CONFIG2_REG_MAC(devObjPtr),
            &regValue);

        hdrIntegrityPacketCommand = ((regValue >> 8) & 7);
        hdrIntegrityCpuCode       = (regValue & 0xFF);

        __LOG(("TTI Cut Through - Exception packetCmd %d cpuCode %d \n",
               hdrIntegrityPacketCommand, hdrIntegrityCpuCode));
        snetChtIngressCommandAndCpuCodeResolution(
            devObjPtr, descrPtr,
            descrPtr->packetCmd, hdrIntegrityPacketCommand,
            descrPtr->cpuCode, hdrIntegrityCpuCode,
            SNET_CHEETAH_ENGINE_UNIT_TTI_E, GT_FALSE);
    }
}

/**
* @internal snetSip6_10_8B_Tag_parse function
* @endinternal
*
* @brief   8B Tag parsing support for SIP_6_10 devices
*          (or 6B for SIP6_30)
*
* return GT_OK              - Successfully parsed
*        GT_FAIL            - Error
*        GT_NOT_SUPPORTED   - Not supported
*        GT_NOT_FOUND       - Tag1 was not found (Assign default)
*/
GT_STATUS snetSip6_10_8B_Tag_parse
(
    IN SKERNEL_DEVICE_OBJECT            *devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC  *descrPtr,
    IN GT_U32                            ethTypeOffset,
    IN GT_U32                            tpidIndex
)
{
    DECLARE_FUNC_NAME(snetSip6_10_8B_Tag_parse);

    GT_U16      tagTciValueLsb;
    GT_U32      tagTciValueMsb;
    GT_U16      tagTciMaskLsb;
    GT_U32      tagTciMaskMsb;
    GT_U16      pktTciValueLsb;
    GT_U32      pktTciValueMsb;
    GT_U8       extractedValueOffset;
    GT_U32      regAddress;              /* register address */
    GT_U32      regValue;
    GT_U32      *regPtr;                 /* register entry pointer */
    GT_U32      hsrTag = 0;

    if(!SMEM_CHT_IS_SIP6_10_GET(devObjPtr))
    {
        /* not relevant */
        return GT_NOT_SUPPORTED;
    }

    regAddress  = SMEM_SIP6_10_INGR_TPID_ENHANCED_CLASS_REG(devObjPtr, tpidIndex);
    regPtr      = smemMemGet(devObjPtr, regAddress);
    regValue    = SMEM_U32_GET_FIELD(regPtr[0], 0, 1);
    if(!regValue)
    {
        __LOG(("8B tag parsing is not enabled on tpidIndex[%d] \n",tpidIndex));
        return GT_NOT_SUPPORTED;
    }
    __LOG(("8B tag parsing is enabled on tpidIndex[%d] \n",tpidIndex));

    if(SMEM_CHT_IS_SIP6_30_GET(devObjPtr))
    {
        hsrTag  = SMEM_U32_GET_FIELD(regPtr[0], 20, 1);
        __LOG_PARAM(hsrTag);
    }
    /* 3 Bits offset - eight_byte_tag_extracted_value_offset */
    extractedValueOffset = SMEM_U32_GET_FIELD(regPtr[0], 1, 3);

    /* Tag TCI Value */
    tagTciValueLsb  = (GT_U16)SMEM_U32_GET_FIELD(regPtr[0], 4, 16);
    regAddress      = SMEM_SIP6_10_INGR_TPID_ENHANCED_CLASS_EXT_REG(devObjPtr, tpidIndex);
    regPtr          = smemMemGet(devObjPtr, regAddress);
    tagTciValueMsb  = regPtr[0];

    /* TAG TCI Mask */
    regAddress      = SMEM_SIP6_10_INGR_TPID_ENHANCED_CLASS_EXT2_REG(devObjPtr, tpidIndex);
    regPtr          = smemMemGet(devObjPtr, regAddress);
    tagTciMaskLsb   = (GT_U16)SMEM_U32_GET_FIELD(regPtr[0], 0, 16);
    regAddress      = SMEM_SIP6_10_INGR_TPID_ENHANCED_CLASS_EXT3_REG(devObjPtr, tpidIndex);
    regPtr          = smemMemGet(devObjPtr, regAddress);
    tagTciMaskMsb   = regPtr[0];

    /* Pkt TCI Value */
    pktTciValueMsb  = SNET_GET_PCKT_6B_TAG_TCI_GET_HIGH_32_MAC(descrPtr, ethTypeOffset);
    pktTciValueLsb  = SNET_GET_PCKT_6B_TAG_TCI_GET_LOW_16_MAC(descrPtr, ethTypeOffset);

    __LOG_PARAM(pktTciValueLsb);
    __LOG_PARAM(pktTciValueMsb);
    __LOG_PARAM(tagTciValueLsb);
    __LOG_PARAM(tagTciValueMsb);
    __LOG_PARAM(tagTciMaskLsb);
    __LOG_PARAM(tagTciMaskMsb);

    /* Match packet TCI with configured tag TCI value */
    if(((pktTciValueLsb & tagTciMaskLsb) == (tagTciValueLsb & tagTciMaskLsb)) &&
            ((pktTciValueMsb & tagTciMaskMsb) == (tagTciValueMsb & tagTciMaskMsb)))
    {
        __LOG(("matched : 8B tag bytes Found \n"));

        __LOG_PARAM(ethTypeOffset);

        if(hsrTag)
        {
            GT_U16  _2bytes;
            GT_U8   *hsrPtr = &descrPtr->startFramePtr[ethTypeOffset + 2];
            GT_U32  lsduSize;
            /* the logic from the IAS :
                but in WM the 'descrPtr->byteCount' is with the CRC 4 bytes when descrPtr->rxRecalcCrc = 1.
            */
            GT_U32 CalcLSDU    = descrPtr->origByteCount -
                                (ethTypeOffset + 2) -    /* remove up to start of header (without TPID 2 bytes)*/
                                (descrPtr->rxRecalcCrc == 1 /* not 0 as in IAS */ ? 4 : 0) ; /*remove CRC if exists*/
            GT_U32 hsrLsduSizeCommand;
            GT_U32 hsrLsduSizeCommandCpuCode;

            _2bytes = hsrPtr[0] << 8 | hsrPtr[1];/*16 bits*/

            descrPtr->hsrPrpInfo.headerPathId = _2bytes >> 12; /*4 bits */
            lsduSize = _2bytes & 0xFFF;/* 12 bits */

            __LOG_PARAM(lsduSize);
            __LOG_PARAM(CalcLSDU);

            if(descrPtr->sip6_isCtByteCount) /* from IAS : Desc<Byte Count> != rf_ct_byte_count */
            {
                __LOG(("The HSR lsduSize can't be checked in cut-through mode \n"));
            }
            else
            if (lsduSize != CalcLSDU)
            {
                regAddress = SMEM_SIP6_30_TTI_UNIT_HSR_PRP_GLOBAL_CONFIG0_REG(devObjPtr);
                regPtr     = smemMemGet(devObjPtr, regAddress);

                hsrLsduSizeCommand        = SMEM_U32_GET_FIELD(regPtr[0], 16, 3);
                hsrLsduSizeCommandCpuCode = SMEM_U32_GET_FIELD(regPtr[0], 19, 8);

                 /* apply the packet command <HSR LSDU Size Command> according to the packets resolution rules.
                    If the packet command != FORWARD assign <HSR LSDU Size CPU/Drop Code> */
                __LOG(("The HSR lsduSize [0x%3.3x] not match the CalcLSDU[0x%3.3x] so apply packetCmd[%d] cpuCode[%d] \n",
                    lsduSize,CalcLSDU,
                    hsrLsduSizeCommand,hsrLsduSizeCommandCpuCode));


                snetChtIngressCommandAndCpuCodeResolution(devObjPtr, descrPtr,
                                                  descrPtr->packetCmd,
                                                  hsrLsduSizeCommand,
                                                  descrPtr->cpuCode,
                                                  hsrLsduSizeCommandCpuCode,
                                                  SNET_CHEETAH_ENGINE_UNIT_TTI_E,
                                                  GT_FALSE);
            }
            else
            {
                __LOG(("The HSR lsduSize [0x%3.3x] match the CalcLSDU \n",
                    lsduSize));
            }
        }

        __LOG_PARAM(descrPtr->vid1);
        __LOG_PARAM(descrPtr->vlanEtherType1);
        __LOG_PARAM(descrPtr->up1);
        __LOG_PARAM(descrPtr->cfidei1);
        /* Group ID position in TCI value */
        ethTypeOffset += extractedValueOffset;
        descrPtr->vid1 =
            SNET_GET_PCKT_TAG_VLAN_ID_MAC(descrPtr, ethTypeOffset);
        descrPtr->vlanEtherType1 =
            SNET_GET_PCKT_TAG_ETHER_TYPE_MAC(descrPtr, ethTypeOffset);
        descrPtr->up1 =
            SNET_GET_PCKT_TAG_UP_MAC(descrPtr, ethTypeOffset);
        descrPtr->cfidei1 =
            SNET_GET_PCKT_TAG_CFI_DEI_MAC(descrPtr, ethTypeOffset);

        __LOG_PARAM(ethTypeOffset);
        __LOG_PARAM(descrPtr->vid1);
        __LOG_PARAM(descrPtr->vlanEtherType1);
        __LOG_PARAM(descrPtr->up1);
        __LOG_PARAM(descrPtr->cfidei1);

    }
    else
    {
        /* Tag1 was not found - Assign default */
        __LOG(("NOT matched : 8B tag not Found \n"));
        return GT_NOT_FOUND;
    }
    return GT_OK;
}

/**
* @internal snetChtL2Parsing function
* @endinternal
*
* @brief   L2 header Parsing (vlan tag , ethertype , nested vlan , encapsulation)
*         coming from port interface or comming from tunnel termination interface
*         for Ethernet over MPLS.
*/
extern GT_VOID snetChtL2Parsing
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN SNET_CHT_FRAME_PARSE_MODE_ENT   parseMode,
    IN INTERNAL_TTI_DESC_INFO_STC        * internalTtiInfoPtr
)
{
    DECLARE_FUNC_NAME(snetChtL2Parsing);

    GT_U32 regAddress;              /* register address */
    GT_U32 * regPtr;                /* register entry pointer */
    GT_U32 inVlanEtherType0;        /* VLAN EtherType */
    GT_U32 inVlanEtherType1;        /* VLAN EtherType */
    GT_U32 inVlanEtherType;         /* VLAN EtherType */
    GT_U32 * portVlanCfgEntryPtr;   /* table entry pointer */
    GT_U32 ethTypeOffset;           /* ethernet type byte offset */
    GT_U32 etherType;               /* real frame ethernet type */
    GT_U8 firstNestedTagNum;        /* nested tag order */
    SKERNEL_L2_ENCAPSULATION_TYPE_ENT l2encapsul;  /* encapsulation */
    GT_U8 regularTag;           /* nested VLAN or regular tag */
    GT_U8 i;                        /* index */
    GT_U32 cfiRelayEn;              /* CFI enable relay bit */
    GT_U32  *ttiPreTtiLookupIngressEPortTablePtr = NULL;/* pointer to the entry in : tti - Pre-TTI Lookup Ingress ePort Table  */
    GT_U32  tagLenToDecrement=0;/* number of bytes to decrement from the byte count for the 'overSize Untagged Packets Filter Enable' logic */
    GT_U32  overSizeUntaggedPacketsFilterEnable;/*When Enabled, if the packets
                untagged byte count is greater than 1518, then the packet is hard dropped.
                    0 = Disable , 1 = Enable*/
    GT_U32  enableMruCheckOnCascadePort;
    GT_U32  tagExtendedSize = 0; /* indicates whether tag extended (sip5 only) or not */
    GT_U32  defaultPortUp;/* the UP0 that the 'per' port' hold */
    GT_U32  marvellTaggedUsed = (descrPtr->marvellTagged && parseMode == SNET_CHT_FRAME_PARSE_MODE_PORT_E) ? 1 : 0;
    GT_U32  dsaTagLength = 0;
    GT_U32  maxVlanTagsToCheck;/* max number of vlan tag that we may recognize */
    GT_U32  tpid1Offset; /* the offset in bytes from start of packet to point to start of TPID1 */
    GT_U32  secondTagLength = 4;

    __LOG_PARAM(marvellTaggedUsed);

    descrPtr->tag1LocalDevSrcTagged = 0;

    if(SMEM_CHT_IS_SIP6_GET(devObjPtr))
    {
        if (SMEM_LION3_TTI_PHYSICAL_PORT_ENTRY_FIELD_GET(devObjPtr, descrPtr,
                                                         SMEM_SIP6_TTI_PHYSICAL_PORT_TABLE_FIELDS_FLOW_TRACK_ENABLE))
        {
            if(!SMEM_CHT_IS_SIP6_10_GET(devObjPtr))
            {
                descrPtr->copyReserved |= (1 << SNET_CNC_COPY_RESERVED_FLOW_TRACK_ENABLE_BIT_CNS);
                __LOG_PARAM(descrPtr->copyReserved);
            }
            else
            {
                descrPtr->flowTrackEn = 1;
                __LOG_PARAM(descrPtr->flowTrackEn);
            }
        }
    }

    if(parseMode < SNET_CHT_FRAME_PARSE_MODE_LAST_E)
    {
        if(descrPtr->didPacketParseFromEngine[parseMode] == GT_TRUE)
        {
            /* eliminate duplications */
            __LOG(("eliminate duplications"));
            return ;
        }
        /* state that this mode did parsing */
        descrPtr->didPacketParseFromEngine[parseMode] = GT_TRUE;
    }

    if(parseMode != SNET_CHT_FRAME_PARSE_MODE_PORT_E)
    {
        /* support parse of the passenger fill MAC data type */
        if (SGT_MAC_ADDR_IS_MCST(descrPtr->macDaPtr))
        {
            if (SGT_MAC_ADDR_IS_BCST(descrPtr->macDaPtr))
            {
                descrPtr->macDaType = SKERNEL_BROADCAST_MAC_E;
                __LOG(("macDaType == SKERNEL_BROADCAST_MAC_E"));
            }
            else
            {
                descrPtr->macDaType = SKERNEL_MULTICAST_MAC_E;
                __LOG(("macDaType == SKERNEL_MULTICAST_MAC_E"));
            }
        }
        else
        {
            descrPtr->macDaType = SKERNEL_UNICAST_MAC_E;
            __LOG(("macDaType == SKERNEL_UNICAST_MAC_E"));
        }

        if(descrPtr->numOfBytesToPop == 0)
        {
            __LOG(("do L2 re-parse for inner packet \n"));

            /* dump the packet after the re-parse of L2 */
            simLogPacketDump(devObjPtr,
                            GT_TRUE/*ingress*/,
                            descrPtr->localDevSrcPort,/* local device source port */
                            descrPtr->macDaPtr,/*start of inner packet*/
                            descrPtr->byteCount);
        }
        else
        {
            __LOG(("do L2 re-parse to 'pop' numOfBytesToPop = [%d] \n",
                descrPtr->numOfBytesToPop));
        }


        /* the re-parse of the L2 need to recalc the 'orig Src tagging' CQ#134872 */
        __LOG(("the re-parse of the L2 need to recalc the 'orig Src tagging' (and srcPriorityTagged)"));
        descrPtr->origSrcTagged = 0;
    }

    ethTypeOffset = SGT_MAC_ADDR_BYTES*2; /* skip 12 bytes of  mac addresses */

    if (marvellTaggedUsed)
    {
        /* take the info about the orig src tagged from the DSA tag info
            use both : tagSrcTagged[SNET_CHT_TAG_0_INDEX_CNS] or trgTagged because only one of them is
            used per frame.
        */
        descrPtr->origSrcTagged = descrPtr->tagSrcTagged[SNET_CHT_TAG_0_INDEX_CNS] || descrPtr->trgTagged;

        /* save info in case of 802.1D */
        descrPtr->vlanTag802dot1dInfo.vid = descrPtr->eVid & 0xFFF;
        descrPtr->vlanTag802dot1dInfo.vpt = descrPtr->up;
        descrPtr->vlanTag802dot1dInfo.cfi = descrPtr->cfidei;

        dsaTagLength = 4 * (descrPtr->marvellTaggedExtended + 1);
        __LOG_PARAM(dsaTagLength);

        ethTypeOffset += dsaTagLength;

        descrPtr->srcDevIsOwn =
            SKERNEL_IS_MATCH_DEVICES_MAC(descrPtr->srcDev, descrPtr->ownDev,
                                         devObjPtr->dualDeviceIdEnable.tti);
    }
    else
    if(parseMode == SNET_CHT_FRAME_PARSE_MODE_PORT_E)
    {
        if(0 == SMEM_CHT_IS_SIP5_GET(devObjPtr))
        {
            descrPtr->origSrcEPortOrTrnk = descrPtr->localDevSrcPort;
        }
        else
        {
            /* sip5 already assigned origSrcEPortOrTrnk = descrPtr->eArchExtInfo.localDevSrcEPort */
            /* in snetChtFrameParsingVlanAssign */
        }
        descrPtr->srcDev = descrPtr->ownDev;
        descrPtr->srcDevIsOwn = 1;
    }
    /* support both DSA tagged and none DSA tagged packets */
    if(descrPtr->srcDevIsOwn && devObjPtr ->dualDeviceIdEnable.tti)
    {
       /* set the extra bits after 6 bits of the port */
        __LOG(("set the extra bits after 6 bits of the port"));
        descrPtr->extraSrcPortBits = (descrPtr->srcDev & 0x1) << 6;
    }

    /* egressFilterRegistered is not reached from the extended DSA tag when != frw and != from_cpu */
    if(marvellTaggedUsed &&
       (descrPtr->marvellTaggedExtended > SKERNEL_EXT_DSA_TAG_1_WORDS_E &&
        (
           (descrPtr->packetCmd != SKERNEL_EXT_PKT_CMD_FORWARD_E &&
            descrPtr->packetCmd != SKERNEL_EXT_PKT_CMD_FROM_CPU_E)
        )
       )
      )
    {
        descrPtr->egressFilterRegistered = 1;
        __LOG(("set descrPtr->egressFilterRegistered = 1 , because the DSA format not hold it \n"));
    }

    if(descrPtr->numOfBytesToPop)
    { /* sip5 only */
        __LOG_PARAM(descrPtr->numOfBytesToPop);

        /* skip needed number of tags */
        ethTypeOffset += descrPtr->numOfBytesToPop;

        /* reset the flag , because we now re-parse the L2 parts */
        descrPtr->firstVlanTagExtendedSize = 0;
    }
    __LOG_PARAM(ethTypeOffset);
    if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        if (parseMode == SNET_CHT_FRAME_PARSE_MODE_PORT_E)
        {
            /* Timestamp tag Detection */
            snetChtTimestampTagDetection(devObjPtr, descrPtr, &ethTypeOffset);
        }
    }

    /* Real ethernet type */
    etherType  =
        SNET_GET_PCKT_TAG_ETHER_TYPE_MAC(descrPtr, ethTypeOffset);
    __LOG_PARAM(etherType);

    /* allow up to 6 tags recognition for SIP5 device */
    maxVlanTagsToCheck = SMEM_CHT_IS_SIP5_GET(devObjPtr) ? 6 : 4;

    /* need to be calculated before changing <ethTypeOffset> */
    descrPtr->maxAllowedVlanTagOffset = ethTypeOffset + maxVlanTagsToCheck*4 - dsaTagLength;
    __LOG_PARAM(descrPtr->maxAllowedVlanTagOffset);

    if(devObjPtr->supportEArch && devObjPtr->unitEArchEnable.tti)
    {
        /* tti - Pre-TTI Lookup Ingress ePort Table */
        __LOG(("tti - Pre-TTI Lookup Ingress ePort Table"));
        ttiPreTtiLookupIngressEPortTablePtr = descrPtr->eArchExtInfo.ttiPreTtiLookupIngressEPortTablePtr;

        portVlanCfgEntryPtr = NULL;
    }
    else
    {
        regAddress = SMEM_CHT_PORT_VLAN_QOS_CONFIG_TBL_MEM(devObjPtr,
                                                            descrPtr->localDevSrcPort);
        portVlanCfgEntryPtr = smemMemGet(devObjPtr, regAddress);
    }

    snetXCatIngressVlanTagClassify(devObjPtr, descrPtr, ethTypeOffset,
                                   &inVlanEtherType0, &inVlanEtherType1,
                                   parseMode);

    /* calculate the implied field of descrPtr->srcTagState */
    if(descrPtr->ingressVlanTag0Type == SKERNEL_FRAME_TR101_VLAN_TAG_NOT_FOUND_E)
    {
        descrPtr->srcTagState =
            (descrPtr->ingressVlanTag1Type == SKERNEL_FRAME_TR101_VLAN_TAG_NOT_FOUND_E) ?
            SKERNEL_FRAME_TR101_VLAN_INGR_TAG_UNTAGGED_E :  /* no tags */
            SKERNEL_FRAME_TR101_VLAN_INGR_TAG_TAG1_E;       /* tag 1 only */
    }
    else if(descrPtr->ingressVlanTag0Type == SKERNEL_FRAME_TR101_VLAN_TAG_OUTER_E)
    {
        descrPtr->srcTagState =
            (descrPtr->ingressVlanTag1Type == SKERNEL_FRAME_TR101_VLAN_TAG_NOT_FOUND_E) ?
            SKERNEL_FRAME_TR101_VLAN_INGR_TAG_TAG0_E :              /* tag 0 only */
            SKERNEL_FRAME_TR101_VLAN_INGR_TAG_OUT_TAG0_IN_TAG1_E;   /* tag 0 outer tag 1 inner */
    }
    else /*if(descrPtr->ingressVlanTag0Type == SKERNEL_FRAME_TR101_VLAN_TAG_INNER_E)*/
    {
        descrPtr->srcTagState = SKERNEL_FRAME_TR101_VLAN_INGR_TAG_OUT_TAG1_IN_TAG0_E;   /* tag 1 outer tag 0 inner */
    }


    if(descrPtr->ingressVlanTag0Type ==
       SKERNEL_FRAME_TR101_VLAN_TAG_OUTER_E)
    {
        inVlanEtherType = inVlanEtherType0;
    }
    else if(descrPtr->ingressVlanTag1Type ==
            SKERNEL_FRAME_TR101_VLAN_TAG_OUTER_E)
    {
        inVlanEtherType = inVlanEtherType1;
    }
    else
    {
        inVlanEtherType = 0xFFFFFFFF;
    }

    if(parseMode == SNET_CHT_FRAME_PARSE_MODE_PORT_E)
    {
        /* save original tagging for the TO_CPU packets */
        descrPtr->origSrcTagState =  descrPtr->srcTagState;
        if(descrPtr->marvellTagged &&
            (descrPtr->origSrcTagged == GT_FALSE))
        {
            /* If Src tagged from DSA says untagged, it can't be tagged */
            descrPtr->origSrcTagState = SKERNEL_FRAME_TR101_VLAN_INGR_TAG_UNTAGGED_E;
        }
    }

    /* NestedVLAN AccessPortEn */
    if(parseMode == SNET_CHT_FRAME_PARSE_MODE_PORT_E)
    {
        if(ttiPreTtiLookupIngressEPortTablePtr)
        {
            descrPtr->nestedVlanAccessPort =
                SMEM_LION3_TTI_DEFAULT_EPORT_ENTRY_FIELD_GET(devObjPtr, descrPtr,
                    SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_NESTED_VLAN_ACCESS_PORT);
        }
        else
        {

            if(portVlanCfgEntryPtr == NULL)
            {
                /* fix for coverity warning "FORWARD_NULL" */
                skernelFatalError("snetChtL2Parsing: invalid table entry pointer\n");
            }

            descrPtr->nestedVlanAccessPort =
                SNET_CHT_PORT_VLAN_QOS_ACCESS_PORT_MAC(devObjPtr, portVlanCfgEntryPtr);
        }
    }

    if(parseMode == SNET_CHT_FRAME_PARSE_MODE_PORT_E)
    {
        if(descrPtr->eArchExtInfo.ttiPhysicalPortAttributePtr)
        {
            enableMruCheckOnCascadePort =
                SMEM_LION3_TTI_PHYSICAL_PORT_ENTRY_FIELD_GET(devObjPtr,descrPtr,
                    SMEM_LION3_TTI_PHYSICAL_PORT_TABLE_FIELDS_ENABLE_MRU_CHECK_ON_CASCADE_PORT);
        }
        else
        {
            enableMruCheckOnCascadePort = snetFieldValueGet(portVlanCfgEntryPtr,64,1);
        }

        if(ttiPreTtiLookupIngressEPortTablePtr)
        {
            if(SMEM_CHT_IS_SIP6_GET(devObjPtr))
            {
                /* feature was removed from Falcon and following devices */
                overSizeUntaggedPacketsFilterEnable = 0;
                __LOG(("'OVERSIZE_UNTAGGED_PKTS_FILTER_EN' field is ignored for sip6 device"));
            }
            else
            {
                overSizeUntaggedPacketsFilterEnable =
                    SMEM_LION3_TTI_DEFAULT_EPORT_ENTRY_FIELD_GET(devObjPtr, descrPtr,
                        SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_OVERSIZE_UNTAGGED_PKTS_FILTER_EN);
            }
        }
        else
        {
            overSizeUntaggedPacketsFilterEnable =
                snetFieldValueGet(portVlanCfgEntryPtr,9,1);
        }

        __LOG(("overSizeUntaggedPacketsFilterEnable[%d] , enableMruCheckOnCascadePort[%d]",
                      overSizeUntaggedPacketsFilterEnable , enableMruCheckOnCascadePort));

        if(marvellTaggedUsed)
        {
            tagLenToDecrement = 4*(descrPtr->marvellTaggedExtended + 1);
        }
        else
        if(descrPtr->srcTagState !=
           SKERNEL_FRAME_TR101_VLAN_INGR_TAG_UNTAGGED_E)
        {
            /* the logic removes only single tag !
               regardless to number of tags recognized */
            tagLenToDecrement = 4;
            if(descrPtr->firstVlanTagExtendedSize > 4)
            {
                tagLenToDecrement += descrPtr->firstVlanTagExtendedSize - 4;
            }
        }
        else
        {
            tagLenToDecrement = 0;
        }

        /* this feature is not take into account any 'Nested Vlan' issues */
        if(descrPtr->sip6_isCtByteCount)
        {
            __LOG(("No MRU check on cascade port, because packet is 'cut through' without valid <byte count>[%d]\n",
                descrPtr->byteCount));
        }
        else
        if(overSizeUntaggedPacketsFilterEnable &&
            ((marvellTaggedUsed == 0) || (marvellTaggedUsed == 1 && enableMruCheckOnCascadePort)))
        {
            GT_U32  byteCount = descrPtr->byteCount;

            if(descrPtr->is_byte_count_missing_4_bytes)
            {
                byteCount -= 4;
            }

            if((byteCount - tagLenToDecrement) > 1518)
            {
                __LOG(("assign HARD DROP : (byteCount[%d] - tagLenToDecrement[%d]) > 1518",
                              byteCount,tagLenToDecrement));

                /*assigned a HARD_DROP packet*/
                snetChtIngressCommandAndCpuCodeResolution(devObjPtr,descrPtr,
                                                          descrPtr->packetCmd,
                                                          SKERNEL_EXT_PKT_CMD_HARD_DROP_E,
                                                          descrPtr->cpuCode,
                                                          0,/*don't care*/
                                                          SNET_CHEETAH_ENGINE_UNIT_TTI_E,
                                                          GT_TRUE);
            }
        }
    }

    if(
       descrPtr->ingressVlanTag0Type == SKERNEL_FRAME_TR101_VLAN_TAG_NOT_FOUND_E)
    {
        /* save the 'per port' UP */
        defaultPortUp = descrPtr->up;
    }
    else
    {
        /* dummy init will not be used */
        defaultPortUp = 0;
    }

    descrPtr->origVlanTagPtr = descrPtr->macSaPtr + SGT_MAC_ADDR_BYTES +
                               ((devObjPtr->errata.haBuildWrongTagsWhenNumOfTagsToPopIs2 &&
                                 descrPtr->numOfBytesToPop == 8) ? 0 : descrPtr->numOfBytesToPop);

    __LOG_PARAM(descrPtr->ingressTimestampTagSize);
    descrPtr->origVlanTagPtr += descrPtr->ingressTimestampTagSize; /* timestamp tag size (if exists) */

    descrPtr->origVlanTagLength = 0;
    __LOG_PARAM(descrPtr->ingressVlanTag0Type);
    __LOG_PARAM(descrPtr->ingressVlanTag1Type);
    __LOG_PARAM(ethTypeOffset);
    __LOG_PARAM(descrPtr->firstVlanTagExtendedSize);


    if(marvellTaggedUsed)
    {
        /* the logic did not 'jumped' the tag after the DSA */
        tpid1Offset = 12 + dsaTagLength;
    }
    else if(descrPtr->ingressVlanTag1Type == SKERNEL_FRAME_TR101_VLAN_TAG_OUTER_E)
    {
        /* tag1 is outer */
        tpid1Offset = ethTypeOffset;
    }
    else if(descrPtr->firstVlanTagExtendedSize > 4)/* tag0 outer is extended */
    {
        tpid1Offset = ethTypeOffset + descrPtr->firstVlanTagExtendedSize;
    }
    else if (descrPtr->ingressVlanTag0Type == SKERNEL_FRAME_TR101_VLAN_TAG_NOT_FOUND_E)
    {
        tpid1Offset = ethTypeOffset;/* tag0 not exists */
    }
    else
    {
        tpid1Offset = ethTypeOffset + 4;/* tag0 exists */
    }
    __LOG_PARAM(tpid1Offset);

    if(parseMode == SNET_CHT_FRAME_PARSE_MODE_PORT_E &&
       SMEM_CHT_IS_SIP6_30_GET(devObjPtr) &&
       SMEM_LION3_TTI_DEFAULT_EPORT_ENTRY_FIELD_GET(devObjPtr, descrPtr,
            SMEM_SIP6_30_TTI_DEFAULT_EPORT_TABLE_FIELDS_PRP_ENABLED_E))
    {
        GT_U32  global_PRP_Suffix;

        regAddress = SMEM_SIP6_30_TTI_UNIT_HSR_PRP_GLOBAL_CONFIG0_REG(devObjPtr);
        /* PRP is enabled on the port , lets check if the trailer is valid */
        /* default if 0x88FB of this field */
        smemRegFldGet(devObjPtr, regAddress, 0, 16, &global_PRP_Suffix);

        __LOG_PARAM(global_PRP_Suffix);
        if(global_PRP_Suffix == descrPtr->prpInfo.prpSuffix)
        {
            GT_U32 Offset2Tag1 = tpid1Offset;
            GT_U32 Offset2LSDU = Offset2Tag1 + 2;
            GT_U32 LSDU_constant = SMEM_LION3_TTI_DEFAULT_EPORT_ENTRY_FIELD_GET(devObjPtr, descrPtr,
                SMEM_SIP6_30_TTI_DEFAULT_EPORT_TABLE_FIELDS_LSDU_CONSTANT_E);
            /* the logic from the IAS :
                but in WM the 'descrPtr->byteCount' is with the CRC 4 bytes when descrPtr->rxRecalcCrc = 1.
            */
            GT_U32 CalcLSDU    = descrPtr->origByteCount - Offset2LSDU -
                (descrPtr->rxRecalcCrc == 1 /* not 0 as in IAS */ ? 4 : 0)+ (LSDU_constant - 15);
            GT_U32  portLanId;

            __LOG_PARAM(descrPtr->origByteCount);

            __LOG(("Matched the <PRP Suffix> [0x%4.4x]\n",global_PRP_Suffix));

            __LOG_PARAM_WITH_NAME("Offset2Tag1:" ,tpid1Offset);
            __LOG_PARAM_WITH_NAME("Offset2LSDU:" ,Offset2Tag1 +2);
            __LOG_PARAM(CalcLSDU);

            if(CalcLSDU == descrPtr->prpInfo.prpLSDUSize)
            {
                /* RCT detected */
                __LOG(("Matched the <prp LSDU Size> [0x%4.4x] --> so 'RCT was detected' \n",
                    descrPtr->prpInfo.prpLSDUSize));

                descrPtr->prpInfo.prpRctDetected = 1;

                descrPtr->hsrPrpInfo.headerPathId = descrPtr->prpInfo.prpLanId;

                __LOG(("assign the prpSequenceNumber[0x%4.4x] into the tag1 info \n",
                    descrPtr->prpInfo.prpSequenceNumber));
                descrPtr->vid1      = descrPtr->prpInfo.prpSequenceNumber & 0x0FFF;
                descrPtr->cfidei1   = descrPtr->prpInfo.prpSequenceNumber >> 12 & 0x1;
                descrPtr->up1       = descrPtr->prpInfo.prpSequenceNumber >> 13;

                __LOG_PARAM(descrPtr->prpInfo.prpSequenceNumber);
                __LOG_PARAM(descrPtr->vid1);
                __LOG_PARAM(descrPtr->cfidei1);
                __LOG_PARAM(descrPtr->up1);

                __LOG_PARAM(descrPtr->prpInfo.prpLanId);
                portLanId = SMEM_LION3_TTI_DEFAULT_EPORT_ENTRY_FIELD_GET(devObjPtr, descrPtr,
                    SMEM_SIP6_30_TTI_DEFAULT_EPORT_TABLE_FIELDS_PRT_PORT_LAN_ID_E);
                if (descrPtr->prpInfo.prpLanId == portLanId)
                {
                    __LOG(("The <PRP LanId>[0x%x] from the packet match the <port LanId> , So set 'RCT exists' bit 5 in the SSTID (srcId) \n"));
                    SET_RCT_EXISTS_IN_SSTID_MAC(descrPtr);
                    descrPtr->pclAssignedSstId |= 1 << 5;
                    __LOG_PARAM(descrPtr->sstId);
                    __LOG_PARAM(descrPtr->pclAssignedSstId);
                }
                else /* wrong LanId */
                {
                    if(SMEM_LION3_TTI_DEFAULT_EPORT_ENTRY_FIELD_GET(devObjPtr, descrPtr,
                        SMEM_SIP6_30_TTI_DEFAULT_EPORT_TABLE_FIELDS_PRT_TREAT_WRONG_LAN_ID_AS_RCT_EXISTS_E))
                    {
                        __LOG(("The <PRP LanId>[0x%x] from the packet NOT match the <port LanId>[0x%x] \n",
                            descrPtr->prpInfo.prpLanId,
                            portLanId));
                        __LOG((" but ignored due to 'TREAT_WRONG_LAN_ID_AS_RCT_EXISTS' , so set 'RCT exists' bit 5 in the SSTID (srcId) \n"));
                        SET_RCT_EXISTS_IN_SSTID_MAC(descrPtr);
                        descrPtr->pclAssignedSstId |= 1 << 5;
                        __LOG_PARAM(descrPtr->sstId);
                        __LOG_PARAM(descrPtr->pclAssignedSstId);
                    }
                    else
                    {
                        __LOG(("The <PRP LanId>[0x%x] from the packet NOT match the <port LanId>[0x%x] and not ignored \n",
                            descrPtr->prpInfo.prpLanId,
                            portLanId));
                        __LOG(("So not set 'RCT exists' bit 5 in the SSTID (srcId)\n"));
                    }

                    descrPtr->prpInfo.rctWithWrongLanId = 1;
                    __LOG_PARAM(descrPtr->prpInfo.rctWithWrongLanId);
                }
            }
            else
            {
                __LOG(("NOT PRP , because : the <prp LSDU Size> [0x%4.4x] from the packet not match the calculated value 'CalcLSDU'[0x%4.4x] \n",
                    descrPtr->prpInfo.prpLSDUSize,
                    CalcLSDU));
            }
        }
        else
        {
            __LOG(("NOT PRP , because : the <prp Suffix> [0x%4.4x] from the packet not match the 'global PRP prefix'[0x%4.4x] \n",
                descrPtr->prpInfo.prpSuffix,
                global_PRP_Suffix));
        }
    }

    /* The ingress VLAN EtherType */
    if(marvellTaggedUsed)
    {
        firstNestedTagNum = 1;/* first vlan tag already in the DSA */

        /*jump over the DSA tags */
        descrPtr->origVlanTagPtr += dsaTagLength;

        if(descrPtr->firstVlanTagExtendedSize > 4)
        {
            ethTypeOffset += descrPtr->firstVlanTagExtendedSize;
            descrPtr->origVlanTagLength += descrPtr->firstVlanTagExtendedSize;

            __LOG_PARAM(ethTypeOffset);
        }
    }
    else
    if (etherType == inVlanEtherType)
    {
        /* VLAN Tag found */
        __LOG(("VLAN Tag found"));
        firstNestedTagNum = 1;

        if( descrPtr->ingressVlanTag0Type !=
            SKERNEL_FRAME_TR101_VLAN_TAG_NOT_FOUND_E )
        {
            /* packet came 'TAGGED' -- regardless to nested VLAN issues */
            __LOG(("packet came 'TAGGED' -- regardless to nested VLAN issues"));
            descrPtr->origSrcTagged = 1;
        }

        /* NestedVLAN AccessPortEn */
        if (descrPtr->nestedVlanAccessPort)
        {
            regularTag = 0;

            if(!SMEM_CHT_IS_SIP5_GET(devObjPtr))
            {
                descrPtr->tagSrcTagged[SNET_CHT_TAG_0_INDEX_CNS] = 0;/*  legacy behavior */
            }
        }
        else
        {
            descrPtr->tagSrcTagged[SNET_CHT_TAG_0_INDEX_CNS] = 1;
            regularTag = 1;
        }

        if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
        {
            /* regardless to nested vlan */
            descrPtr->tagSrcTagged[SNET_CHT_TAG_0_INDEX_CNS] =
                (descrPtr->ingressVlanTag0Type !=
                SKERNEL_FRAME_TR101_VLAN_TAG_NOT_FOUND_E) ? 1 : 0;

            /* regardless to nested vlan */
            descrPtr->tagSrcTagged[SNET_CHT_TAG_1_INDEX_CNS] =
                (descrPtr->ingressVlanTag1Type !=
                SKERNEL_FRAME_TR101_VLAN_TAG_NOT_FOUND_E) ? 1 : 0;
        }

        if(parseMode == SNET_CHT_FRAME_PARSE_MODE_PORT_E)
        {
            /* save info in case of 802.1D */
            descrPtr->vlanTag802dot1dInfo.vid =
                SNET_GET_PCKT_TAG_VLAN_ID_MAC(descrPtr, ethTypeOffset);
            descrPtr->vlanTag802dot1dInfo.vpt =
                SNET_GET_PCKT_TAG_UP_MAC(descrPtr, ethTypeOffset);
            descrPtr->vlanTag802dot1dInfo.cfi =
                SNET_GET_PCKT_TAG_CFI_DEI_MAC(descrPtr, ethTypeOffset);


            __LOG_PARAM(descrPtr->vlanTag802dot1dInfo.vid);
            __LOG_PARAM(descrPtr->vlanTag802dot1dInfo.cfi);
            __LOG_PARAM(descrPtr->vlanTag802dot1dInfo.cfi);
        }

        if (regularTag)
        {
            descrPtr->tag0Ptr =  &descrPtr->startFramePtr[ethTypeOffset];

            if(parseMode != SNET_CHT_FRAME_PARSE_MODE_PORT_E)
            {
                /* Re-compute srcPriorityTagged on the re-parsed Vlan tag */
                descrPtr->srcPriorityTagged = descrPtr->origSrcTagged &&
                                                (descrPtr->eVid == 0);
            }
        }

        /* Jump over the vlan tag to the first nested vlan tag */
        __LOG(("Jump over the outer vlan tag \n"));
        ethTypeOffset += 4;

        __LOG_PARAM(ethTypeOffset);
        descrPtr->origVlanTagLength = 4;

        if(descrPtr->firstVlanTagExtendedSize > 4)
        {
            ethTypeOffset += (descrPtr->firstVlanTagExtendedSize - 4);
            descrPtr->origVlanTagLength += (descrPtr->firstVlanTagExtendedSize - 4);
        }
    }
    else
    {
        firstNestedTagNum = 0;
    }

    /* xCat A1 and above - xCat A1, Lion B0 */
    regPtr = smemMemGet(devObjPtr, SMEM_XCAT_A1_TTI_UNIT_GLB_CONF_REG(devObjPtr));
    cfiRelayEn = SMEM_U32_GET_FIELD(regPtr[0], 28, 1);

    __LOG_PARAM(ethTypeOffset);
    /* Set pointer after DSA/VLAN/SA tag before traverse nested VLANs  */
    descrPtr->afterVlanOrDsaTagPtr = &descrPtr->startFramePtr[ethTypeOffset];
    __LOG_POINTER(descrPtr->afterVlanOrDsaTagPtr,16);

    if(descrPtr->marvellTaggedExtended == SKERNEL_EXT_DSA_TAG_4_WORDS_E &&
       devObjPtr->supportEArch == 0)
    {
        maxVlanTagsToCheck = 1;
        __LOG(("Don't parse more than one VLAN tag in packets with eDSA tag for non eArch devices \n"));
    }

    /* Nested VLANs parsing relevant for packet without DSA tag or who's packetCmd == SKERNEL_EXT_PKT_CMD_FORWARD_E */
    /* allow 'from_cpu' also recognize tags properly. -- nested vlan not relevant to it */
    /*if(marvellTaggedUsed == 0 ||
       (descrPtr->packetCmd == SKERNEL_EXT_PKT_CMD_FORWARD_E ||
        descrPtr->packetCmd == SKERNEL_EXT_PKT_CMD_FROM_CPU_E))*/
    {
        for (i = firstNestedTagNum; i < maxVlanTagsToCheck; i++,
                ethTypeOffset+=4,descrPtr->origVlanTagLength += 4)
        {
            etherType =
                SNET_GET_PCKT_TAG_ETHER_TYPE_MAC(descrPtr, ethTypeOffset);
            __LOG_PARAM(etherType);

            /* Nested VLAN processing.Up to 4 VLAN tags  may be in the frame */
            /* xCat supports 8 ethertypes (TPIDs).
               Check of full 8 bits of the global configuration */
            if(GT_FALSE == snetXCatVlanTagMatchWithoutTag0Tag1Classification(devObjPtr, descrPtr, ethTypeOffset, 0xff, i, &tagExtendedSize))
            {
                __LOG(("etherType = [0x%4.4x] is NOT vlan tag , so break the jump over vlan tags\n",etherType));
                break;
            }
            __LOG(("Recognized another vlan tag etherType = [0x%4.4x]\n",etherType));

            if(tagExtendedSize > 4)
            { /* got vlan tpid match, and tag extended, and sip5 only */

                if(i == 0)/* must be the 'outer tag'*/
                {
                    descrPtr->firstVlanTagExtendedSize = tagExtendedSize;
                }
                else if (i == 1)
                {
                    secondTagLength = tagExtendedSize;
                }

                ethTypeOffset+=(tagExtendedSize-4);
                descrPtr->origVlanTagLength += (tagExtendedSize-4);
            }

            if(i == (maxVlanTagsToCheck - 1))
            {
                /* the last tag was matched so update the Ethertype */
                etherType =
                    SNET_GET_PCKT_TAG_ETHER_TYPE_MAC(descrPtr, (ethTypeOffset+4));
                __LOG_PARAM(etherType);

                __LOG(("etherType = [0x%4.4x] is NOT vlan tag , as the device not supports more vlan tags \n",etherType));
            }

            __LOG(("Jump over the last vlan tag \n"));
        }
    }

    if(descrPtr->ingressVlanTag0Type == SKERNEL_FRAME_TR101_VLAN_TAG_NOT_FOUND_E)
    {
        /* restore the 'per port' UP */
        descrPtr->up = defaultPortUp;
    }

    /* check double tag case */
    if((descrPtr->ingressVlanTag1Type !=
            SKERNEL_FRAME_TR101_VLAN_TAG_NOT_FOUND_E) &&
       (descrPtr->ingressVlanTag0Type !=
            SKERNEL_FRAME_TR101_VLAN_TAG_NOT_FOUND_E))
    {
        if(marvellTaggedUsed && (descrPtr->firstVlanTagExtendedSize > 4))
        {
            __LOG(("the 'extended tag' was already skipped after the DSA \n"));
        }
        else
        if(SMEM_CHT_IS_SIP5_GET(devObjPtr) &&
            marvellTaggedUsed &&
            descrPtr->tag1LocalDevSrcTagged == 0)
        {
            __LOG(("Do not trust the eDSA that there is second tag ! \n"));
            __LOG(("because we could not recognized it on the local device \n"));
        }
        else
        {
            /* move pointer to be after second tag */
            __LOG(("move afterVlanOrDsaTagPtr to be after second tag \n"));
            descrPtr->afterVlanOrDsaTagPtr += secondTagLength;
            __LOG_POINTER(descrPtr->afterVlanOrDsaTagPtr,16);
            /* the length of those 4 bytes was already taken into account !
            if(descrPtr->firstVlanTagExtendedSize > 4 )
            {
                descrPtr->afterVlanOrDsaTagPtr += (descrPtr->firstVlanTagExtendedSize - 4);
            }
            */
        }
    }

    /* check for IEEE 802.1 header parsing. (xCat3)*/
    /* need to be before save 'origVid1' */
    snetChtL2Parsing_Bpe_ETag_Format(devObjPtr,descrPtr,tpid1Offset,parseMode);

    descrPtr->originalVid1 = descrPtr->vid1;

    if(descrPtr->ingressVlanTag0Type != SKERNEL_FRAME_TR101_VLAN_TAG_NOT_FOUND_E)
    {
        if (cfiRelayEn == 0)
        {
            /* Tagged packets transmitted from the device have their CFI bit set to 0 */
            __LOG(("Tagged packets transmitted from the device have their CFI bit set to 0"));
            descrPtr->cfidei = 0;
        }
    }

    /* L2 encapsulation parsing */
    if (etherType < 0x0600)
    {
        ethTypeOffset += 2 ;
        etherType = SNET_GET_PCKT_TAG_ETHER_TYPE_MAC(descrPtr, ethTypeOffset);

        if (etherType == 0xAAAA &&
            descrPtr->startFramePtr[ethTypeOffset + 2] == 0x3)
        {
            __LOG(("l2Encaps : IEEE 802.3 LLC/SNAP (LLC-SNAP) \n"));
            l2encapsul = SKERNEL_LLC_SNAP_E;
            ethTypeOffset += 6;
            etherType = (descrPtr->startFramePtr[ethTypeOffset] << 8) |
                         descrPtr->startFramePtr[ethTypeOffset + 1];

        }
        else
        {
            __LOG(("l2Encaps : IEEE 802.3 (LLC Encapsulation) (non-SNAP) \n"));
            l2encapsul = SKERNEL_LLC_E;
            /*ethTypeOffset += 1; cause mess in HA */
        }
    }
    else
    {
        l2encapsul = SKERNEL_ETHERNET_II_E;
        __LOG(("l2Encaps : Ethernet v2 \n"));
    }

    descrPtr->l2Encaps = l2encapsul;

    descrPtr->etherTypeOrSsapDsap = etherType;
    __LOG_PARAM(descrPtr->etherTypeOrSsapDsap); /* the final EtherType */

    if(parseMode == SNET_CHT_FRAME_PARSE_MODE_PORT_E)
    {
        descrPtr->origInfoBeforeTunnelTermination.origEtherType = etherType;
        descrPtr->origInfoBeforeTunnelTermination.l2Encaps = l2encapsul;
    }

    descrPtr->l3StartOffsetPtr = descrPtr->macDaPtr + ethTypeOffset + 2;

    if(descrPtr->afterVlanOrDsaTagPtr > descrPtr->l3StartOffsetPtr)
    {
        /* we got calculation error !!! --> must fix and not ignore it !! */
        skernelFatalError("snetChtL2Parsing: descrPtr->afterVlanOrDsaTagPtr > descrPtr->l3StartOffsetPtr \n");
    }

    descrPtr->l2HeaderSize = descrPtr->l3StartOffsetPtr - descrPtr->macDaPtr;
    __LOG_PARAM(descrPtr->l2HeaderSize);

    descrPtr->afterVlanOrDsaTagLen =
        descrPtr->l3StartOffsetPtr - descrPtr->afterVlanOrDsaTagPtr;

    __LOG_PARAM(descrPtr->afterVlanOrDsaTagLen);
    if(descrPtr->afterVlanOrDsaTagLen < 2)
    {
        descrPtr->afterVlanOrDsaTagLen += 2;
        __LOG(("set descrPtr->afterVlanOrDsaTagLen[%d] because the HA needs at least 2 bytes of 'etherType' \n",
            descrPtr->afterVlanOrDsaTagLen));
    }
    __LOG_POINTER(descrPtr->l3StartOffsetPtr,16);
    __LOG_POINTER(descrPtr->afterVlanOrDsaTagPtr,16);
    __LOG_POINTER(descrPtr->origVlanTagPtr,descrPtr->origVlanTagLength);

    /* check for overlapping of the 2 sections - CQ#134872 */
    if(descrPtr->origVlanTagLength &&
       (&descrPtr->origVlanTagPtr[descrPtr->origVlanTagLength]) >
       (&descrPtr->afterVlanOrDsaTagPtr[0]))
    {
        __LOG_PARAM(descrPtr->origVlanTagLength);/* before the change */
        /* make sure that there are no overlapping between the 2 sections */
        __LOG(("synch origVlanTagLength according to afterVlanOrDsaTagPtr \n"));
        descrPtr->origVlanTagLength -= (&descrPtr->origVlanTagPtr[descrPtr->origVlanTagLength]) - (&descrPtr->afterVlanOrDsaTagPtr[0]);
        __LOG_PARAM(descrPtr->origVlanTagLength);
    }

    if(devObjPtr->supportCutThrough && (parseMode == SNET_CHT_FRAME_PARSE_MODE_PORT_E)
        && (! SMEM_CHT_IS_SIP6_GET(devObjPtr)))
    {
        snetLionCutThroughTrigger(devObjPtr, descrPtr);
    }

    __LOG_PARAM(TAG0_EXIST_MAC(descrPtr));
    __LOG_PARAM(TAG1_EXIST_MAC(descrPtr));

    if (SMEM_CHT_IS_SIP6_GET(devObjPtr) &&
        parseMode == SNET_CHT_FRAME_PARSE_MODE_PORT_E &&
       descrPtr->marvellTagged &&
       descrPtr->marvellTaggedExtended  == SKERNEL_EXT_DSA_TAG_4_WORDS_E &&
       descrPtr->incomingMtagCmd        == SKERNEL_MTAG_CMD_FORWARD_E)
    {
        if(SMEM_LION3_TTI_PHYSICAL_PORT_ENTRY_FIELD_GET(devObjPtr,descrPtr,
            SMEM_SIP6_TTI_PHYSICAL_PORT_TABLE_FIELDS_ACCEPT_EDSA_SKIP_FDB_SA_LOOKUP))
        {
            /* allow the value parsed from eDsa <skipFdbSaLookup> */
            __LOG(("The eDSA <skipFdbSaLookup> field hold value[%d] that was accepted by the ingress physical port[%d] \n",
                descrPtr->skipFdbSaLookup ,
                descrPtr->localDevSrcPort));
        }
        else
        if (descrPtr->skipFdbSaLookup)/* was parsed from eDSA as value 1 ...  but need to be ignored (treated as 0) */
        {
            /* ignore the value parsed from eDsa <skipFdbSaLookup> */
            __LOG(("NOTE : the field <skipFdbSaLookup> from the eDSA ignored (NOT accepted by the ingress physical port[%d]) \n",
                descrPtr->localDevSrcPort));

            /* parse <skipFdbSaLookup> from the edsa */
            descrPtr->skipFdbSaLookup = 0;
        }
        else
        {
            /* eDSA<skipFdbSaLookup> = 0 and 'ignored' by the srcPort */
            __LOG(("field <skipFdbSaLookup> is 0 from the eDSA and anyway also ignored (NOT accepted by the ingress physical port[%d]) \n",
                descrPtr->localDevSrcPort));
        }
    }
}

static GT_BOOL snetLion3RemotePortAssignmentDsaOrEtagOrVlanMode
(
    IN    SKERNEL_DEVICE_OBJECT                              *devObjPtr,
    IN    SNET_CHT_INGRESS_PHYSICAL_PORT_ASSIGNMENT_MODE_ENT phyPortMode,
    IN    GT_U32                                             phyPortBase,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC                    *descrPtr
)
{

    DECLARE_FUNC_NAME(snetLion3RemotePortAssignmentDsaOrEtagOrVlanMode);

    GT_U32                           regAddr;
    GT_U32                           tpid = 0;
    GT_U32                           idValue = 0;
    GT_U32                           packetEtherType = 0;
    GT_U32                           etherTypeOffset;
    GT_U32                           info = 0;
    GT_U32                           numOfBytesToPop = 0;
    GT_U32                           numOfDsaSourcePortBits,numOfDsaSourceDevBits;

    /* < EtherType == global remote physical port TPID>  */
    regAddr = SMEM_SIP6_TTI_REMOTE_PHY_PORT_ASSIGN_GLB_CONF_EXT_REG(devObjPtr);
    __LOG_PARAM(phyPortBase);

    if (phyPortMode == SNET_CHT_INGRESS_PHYSICAL_PORT_ASSIGNMENT_MODE_DSA_E)
    {
        regAddr = SMEM_LION3_TTI_REMOTE_PHY_PORT_ASSIGN_GLB_CONF_REG(devObjPtr);
        smemRegFldGet(devObjPtr, regAddr, 0, 4, &numOfDsaSourcePortBits);
        smemRegFldGet(devObjPtr, regAddr, 4, 4, &numOfDsaSourceDevBits);

        __LOG_PARAM(numOfDsaSourcePortBits);
        __LOG_PARAM(numOfDsaSourceDevBits);
        __LOG_PARAM(descrPtr->ingressDsa.srcDev);
        __LOG_PARAM(descrPtr->ingressDsa.origSrcEPortOrTrnk);

        descrPtr->localDevSrcPort = phyPortBase +
            (SMEM_U32_GET_FIELD(descrPtr->ingressDsa.srcDev, 0, numOfDsaSourceDevBits) << numOfDsaSourcePortBits) +
             SMEM_U32_GET_FIELD(descrPtr->ingressDsa.origSrcEPortOrTrnk, 0, numOfDsaSourcePortBits);

        if (descrPtr->egressFilterRegistered)
        {
            if(SMEM_CHT_IS_SIP6_10_GET(devObjPtr))
            {
                /* fix of : TTI-1603 : Desc<Egress Filtered Registered> should be set to 0 for Remote Physical Ports */
                descrPtr->egressFilterRegistered = 0;
                __LOG(("Sip6.10: reset the egressFilterRegistered = 1 (set it to 0) , that came from the DSA tag ! (fix for Jira TTI-1603) \n"));
            }
            else
            {
                __LOG(("Warning: the egressFilterRegistered = 1 , from the DSA tag (may cause skip of 'unknn/unreg' filtering on 'flooding') (Jira TTI-1603) \n"));
            }
        }
    }
    else /* sip 6 only*/
    {
        if (!SMEM_CHT_IS_SIP6_GET(devObjPtr))
        {
            __LOG(("ERROR : Physical Port Assignment Mode: [%d] not supported \n" , phyPortMode));
            return GT_FALSE;
        }
        etherTypeOffset = 12; /* ethrType offset from start of packet*/
        packetEtherType =
             descrPtr->macDaPtr[etherTypeOffset + 0] << 8 |
             descrPtr->macDaPtr[etherTypeOffset + 1] << 0;

        if (phyPortMode == SNET_CHT_INGRESS_PHYSICAL_PORT_ASSIGNMENT_MODE_E_TAG_E)
        {
            smemRegFldGet(devObjPtr, regAddr, 0, 16, &tpid);
            info =
                descrPtr->macDaPtr[etherTypeOffset +4 + 0] << 8 |
                descrPtr->macDaPtr[etherTypeOffset +4 + 1] << 0 ;
            numOfBytesToPop = 8;
        }
        else if (phyPortMode == SNET_CHT_INGRESS_PHYSICAL_PORT_ASSIGNMENT_MODE_VLAN_TAG_E)
        {
            smemRegFldGet(devObjPtr, regAddr, 16, 16, &tpid);
            info =
                descrPtr->macDaPtr[etherTypeOffset + 2] << 8 |
                descrPtr->macDaPtr[etherTypeOffset + 3] << 0 ;
            numOfBytesToPop = 4;
        }
        else
        {
            return GT_FALSE;
        }
        __LOG(("sip 6: check for vlan tpid or etherType TPID  match \n"));
        __LOG_PARAM(packetEtherType);
        __LOG_PARAM(tpid);
        if (packetEtherType == tpid)
        {
          /* E TAG mode: E-CID
             VLAN mode : VID */
            idValue = SMEM_U32_GET_FIELD(info,0,12);

            __LOG_PARAM(phyPortBase);
            __LOG_PARAM(idValue);

            descrPtr->localDevSrcPort = phyPortBase + idValue;

            /*<Num Of Bytes To Pop>*/
            /* Pop tag of 8 bytes --> AKA BPE (802.1BR)
               Pop tag of 4 bytes --> AKA EVB */
            descrPtr -> numOfBytesToPop = numOfBytesToPop;
        }
        else
        {
            return GT_FALSE;
        }
    }

    if(descrPtr->localDevSrcPort > SMEM_BIT_MASK(devObjPtr->flexFieldNumBitsSupport.phyPort))
    {
        /* mask the value with supported number of bits */
        descrPtr->localDevSrcPort &= SMEM_BIT_MASK(devObjPtr->flexFieldNumBitsSupport.phyPort);

        __LOG(("new value of descrPtr->localDevSrcPort is [%d] (using only[%d] LSBits !)\n",
               descrPtr->localDevSrcPort,
               devObjPtr->flexFieldNumBitsSupport.phyPort));
    }
    else
    {
        __LOG(("new value of descrPtr->localDevSrcPort is [%d] \n",
               descrPtr->localDevSrcPort));
    }
    /* reassign value that we previously initialized to descrPtr->localDevSrcPort */
    descrPtr->srcTrgPhysicalPort = descrPtr->localDevSrcPort;

    __LOG(("new value of descrPtr->srcTrgPhysicalPort is [%d] \n",
           descrPtr->srcTrgPhysicalPort));

    return  GT_TRUE;


}


/**
* @internal snetLion3RemotePortAssignmentGet function
* @endinternal
*
* @brief   Remote Physical Port Assignment
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] descrPtr             - frame descriptor pointer
*
* @retval  GT_TRUE  - did Local Port Re-Assign
* @retval  GT_FALSE - did not do Local Port Re-Assign
*/
static GT_BOOL snetLion3RemotePortAssignmentGet
(
    IN    SKERNEL_DEVICE_OBJECT           *devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC *descrPtr
)
{
    DECLARE_FUNC_NAME(snetLion3RemotePortAssignmentGet);

    GT_U32                                              myPhyPortRegAddr;
    SNET_CHT_INGRESS_PHYSICAL_PORT_ASSIGNMENT_MODE_ENT  phyPortMode;
    GT_U32                                              phyPortBase;
    GT_U32                                              srcDevAssignEn;
    GT_U32                                              numBitsPortBase = 8;
    GT_U32                                              index =  descrPtr->localDevSrcPort & 0xFF;
    GT_BOOL                                             didLocalPortReAssign = GT_FALSE;
    GT_U32                                              fldSize;
    GT_U32                                              tmpValue;

    if( !SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        return didLocalPortReAssign;
    }

    if(index < descrPtr->localDevSrcPort)
    {
        __LOG(("NOTE (warning): index 'my phy port attributes' registers with only 8 LSbits [0x%x] of 'localDevSrcPort'[0x%x] \n",
            index , descrPtr->localDevSrcPort));
    }

    if(index >= devObjPtr->TTI_myPhysicalPortAttributes_numEntries &&
       devObjPtr->TTI_myPhysicalPortAttributes_numEntries != 0)
    {
        __LOG(("Warning : the access to 'myPhysicalPortAttributes' will be at 'last index'[%d] instead of [%d] that is higher \n",
            devObjPtr->TTI_myPhysicalPortAttributes_numEntries - 1,
            index));
        /* use last entry of the table */
        index = devObjPtr->TTI_myPhysicalPortAttributes_numEntries - 1;
    }

    myPhyPortRegAddr = SMEM_LION3_TTI_MY_PHY_PORT_ATTRIBUTES_REG(devObjPtr, index);
    if( SMEM_CHT_IS_SIP5_20_GET(devObjPtr))
    {
        numBitsPortBase = 10;
    }

    fldSize = SMEM_CHT_IS_SIP6_GET(devObjPtr) ? 3 : 2;
    smemRegFldGet(devObjPtr, myPhyPortRegAddr, numBitsPortBase+1, fldSize, &tmpValue);
    phyPortMode = tmpValue;

    descrPtr->myPhyPortAssignMode = phyPortMode;

    smemRegFldGet(devObjPtr, myPhyPortRegAddr, 1, numBitsPortBase, &phyPortBase);

    switch(phyPortMode)
    {
        case SNET_CHT_INGRESS_PHYSICAL_PORT_ASSIGNMENT_MODE_DISABLED_E:
            __LOG(("Physical Port Assignment Mode: disabled, do nothing \n"));
            return didLocalPortReAssign;
        case SNET_CHT_INGRESS_PHYSICAL_PORT_ASSIGNMENT_MODE_INTERLAKEN_E:
            __LOG(("Physical Port Assignment Mode: interlaken, TBD, not supported \n"));
            return didLocalPortReAssign;
        case SNET_CHT_INGRESS_PHYSICAL_PORT_ASSIGNMENT_MODE_DSA_E:
            __LOG(("Physical Port Assignment Mode: DSA \n"));
            if(descrPtr->marvellTagged)
            {
                if(descrPtr->incomingMtagCmd != SKERNEL_MTAG_CMD_FORWARD_E)
                {
                    __LOG(("Only DSA 'FORWARD' remapped to local port \n"));
                }
                else
                if(0 == descrPtr->ingressDsa.origIsTrunk)
                {
                    didLocalPortReAssign = snetLion3RemotePortAssignmentDsaOrEtagOrVlanMode(devObjPtr,phyPortMode,phyPortBase,descrPtr);
                }
                else
                {
                    __LOG(("DSA with <srcIsTrunk> =1 are not remapped to local port \n"));
                }
            }
            else
            {
                __LOG(("ERROR : packet is not DSA tagged ?! \n"));
            }
            break;
        case SNET_CHT_INGRESS_PHYSICAL_PORT_ASSIGNMENT_MODE_E_TAG_E:
            if (SMEM_CHT_IS_SIP6_GET(devObjPtr))
            {
                __LOG(("Physical Port Assignment Mode: E-TAG \n"));
                if(descrPtr->marvellTagged)
                {
                    __LOG(("ERROR : Assignment Mode E-Tag for marvell Tagged packet \n"));
                    break;
                }
                didLocalPortReAssign = snetLion3RemotePortAssignmentDsaOrEtagOrVlanMode(devObjPtr,phyPortMode,phyPortBase,descrPtr);
            }
            else
            {
                __LOG(("ERROR : Physical Port Assignment Mode: [%d] not supported \n" , phyPortMode));
            }
            break;

        case SNET_CHT_INGRESS_PHYSICAL_PORT_ASSIGNMENT_MODE_VLAN_TAG_E:
            if (SMEM_CHT_IS_SIP6_GET(devObjPtr))
            {
                __LOG(("Physical Port Assignment Mode: VLAN-TAG \n"));
                __LOG_PARAM(descrPtr->marvellTagged);
                if(descrPtr->marvellTagged)
                {
                    __LOG(("ERROR : Assignment Mode VLAN-TAG for marvell Tagged packet \n"));
                    break;
                }
                didLocalPortReAssign = snetLion3RemotePortAssignmentDsaOrEtagOrVlanMode(devObjPtr,phyPortMode,phyPortBase,descrPtr);
            }
            else
            {
                __LOG(("ERROR : Physical Port Assignment Mode: [%d] not supported \n" , phyPortMode));
            }
            break;
        default:
            __LOG(("ERROR : Physical Port Assignment Mode: [%d] not supported \n" , phyPortMode));
            break;
    }


    smemRegFldGet(devObjPtr, myPhyPortRegAddr, 0, 1, &srcDevAssignEn);

    __LOG_PARAM(srcDevAssignEn);

    descrPtr->srcDevAssignEn = srcDevAssignEn;

    if(srcDevAssignEn)
    {
        descrPtr->srcDev = descrPtr->ownDev;
        __LOG(("new value of descrPtr->srcDev is [%d] (own device) \n", descrPtr->srcDev));
    }
    else
    {
        __LOG(("Warning : Potential Configuration ERROR : <srcDevAssignEn> is 'disabled' although working in 'Physical Port Assignment Mode' \n"));
    }

    return didLocalPortReAssign;
}

/**
* @internal lion3CentralizedChassisModeSupport function
* @endinternal
*
* @brief   sip5 Centralized Chassis Mode Support,
*         relevant when Centralized Chassis Mode enabled
*
* @note this function is relevant only for sip5, extended and eDSA FORWARD DSA tag
*
*/
static GT_VOID lion3CentralizedChassisModeSupport
(
    IN  SKERNEL_DEVICE_OBJECT           *devObjPtr,
    IN  SKERNEL_FRAME_CHEETAH_DESCR_STC *descrPtr
)
{
    DECLARE_FUNC_NAME(lion3CentralizedChassisModeSupport);

    GT_U32  value;/*register value*/
    GT_U32  ccLineCardDefaultEportBase,dsaSrcDev2ePortLsb,
            dsaSrcPort2ePortLsb,dsaSrcTrunk2ePortLsb;/* fields relate to centralized Chassis Mode*/
    GT_U32  centralizedChassisDefaultSrcEPort = 0;

    if(descrPtr->marvellTagged == 0 ||
       descrPtr->marvellTaggedExtended == SKERNEL_EXT_DSA_TAG_1_WORDS_E ||
       descrPtr->incomingMtagCmd != SKERNEL_MTAG_CMD_FORWARD_E)
    {
        __LOG(("function is relevant only for sip5, extended and eDSA FORWARD DSA tag"));
        return;
    }

    /* must be set after dsa tag type check */
    descrPtr->centralizedChassisModeEn = 1;

    /*
        LocalDevSRCePort = Map DSA<SrcPort/Trunk>
        OrigSRCePort/Trunk = Map DSA<SRCePort/Trunk>
    */

    if (descrPtr->marvellTaggedExtended == SKERNEL_EXT_DSA_TAG_4_WORDS_E)
    {
        if(descrPtr->origIsTrunk == 1)
        {
            /* CC not supported when origin is trunk for eDSA */
            descrPtr->centralizedChassisModeEn = 0;
            __LOG(("function is relevant only for none Trunk eDSA FORWARD DSA tag"));
            return;
        }

        /* use value from eDSA as-is */
        centralizedChassisDefaultSrcEPort = descrPtr->origSrcEPortOrTrnk;
    }
    else
    {
        smemRegGet(devObjPtr, SMEM_LION2_TTI_UNIT_GLOBAL_CONF_FOR_CC_PORTS_REG(devObjPtr),&value);
        dsaSrcDev2ePortLsb          = SMEM_U32_GET_FIELD(value,6,3);
        dsaSrcPort2ePortLsb         = SMEM_U32_GET_FIELD(value,3,3);
        dsaSrcTrunk2ePortLsb        = SMEM_U32_GET_FIELD(value,0,3);

        /*
        Desc<OrigIsTrunk> = 0;
        Assign the same new value to both Desc<OrigSRCePort/Trunk> and Desc<LocalDevSRCePort>:
        If DSA<SrcIsTrunk> == 0
            Assign <CC Line-Card Ports Default ePort Base> +
            {DSA<SrcDev>[<CC Default ePort Mapping DSA<SrcDev> LSB Amount>:0],
             DSA<SrcPort>[<CC Default ePort Mapping DSA<SrcPort> LSB Amount>:0]};
        Else // DSA<SrcIsTrunk> == 1
            Assign <CC Line-Card Trunk Default ePort Base> +
                    DSA<SrcTrunk>[<CC Default ePort Mapping DSA<SrcTrunk> LSB Amount>:0];
        */
        if(descrPtr->origIsTrunk == 0)
        {
            /*CC Line-Card Ports Default ePort Base*/
            ccLineCardDefaultEportBase   =
                SMEM_LION3_TTI_PHYSICAL_PORT_ENTRY_FIELD_GET(devObjPtr,descrPtr,
                    SMEM_LION3_TTI_PHYSICAL_PORT_TABLE_FIELDS_CC_LINE_CARD_PORTS_DEFAULT_EPORT_BASE);

            SMEM_U32_SET_FIELD(centralizedChassisDefaultSrcEPort,
                dsaSrcPort2ePortLsb ,                      /* port offset */
                dsaSrcDev2ePortLsb,
                descrPtr->srcDev);/*DSA<SrcDev>*/

            SMEM_U32_SET_FIELD(centralizedChassisDefaultSrcEPort,
                0 ,
                dsaSrcPort2ePortLsb,
                descrPtr->origSrcEPortOrTrnk);/*DSA<SrcPort>*/
        }
        else /*came from trunk*/
        {
            /*CC Line-Card Trunk Default ePort Base*/
            ccLineCardDefaultEportBase   =
                SMEM_LION3_TTI_PHYSICAL_PORT_ENTRY_FIELD_GET(devObjPtr,descrPtr,
                    SMEM_LION3_TTI_PHYSICAL_PORT_TABLE_FIELDS_CC_LINE_CARD_TRUNK_DEFAULT_EPORT_BASE);

            SMEM_U32_SET_FIELD(centralizedChassisDefaultSrcEPort,
                0 ,
                dsaSrcTrunk2ePortLsb,
                descrPtr->origSrcEPortOrTrnk);/*DSA<SrcTrunk>*/

        }

        centralizedChassisDefaultSrcEPort += ccLineCardDefaultEportBase;
    }

    descrPtr->trgEPort = SNET_CHT_NULL_PORT_CNS;/*62*/
    descrPtr->eArchExtInfo.trgPhyPort = descrPtr->trgEPort;
    descrPtr->trgDev = 0;
    descrPtr->targetIsTrunk = 0;
    descrPtr->useVidx = 0;
    descrPtr->eArchExtInfo.isTrgPhyPortValid = 0;

    __LOG(("CC line card changed default Source EPort [%d] (but it will not effect TTI lookup fields)",
                      centralizedChassisDefaultSrcEPort));

    snetLion3IngressReassignSrcEPort(devObjPtr,descrPtr, "CC line card", centralizedChassisDefaultSrcEPort);

    /*
        This assignment is not performed at this initial stage, as the original DSA<SrcDev>
        may be used for the TTI lookup key. This assignment is performed unconditionally
        and after TTI lookup is performed, as part of the post-TTI lookup descriptor field
        assignments
    */
    __LOG(("Restore OrigSrcDev from the DSA info \n"));
    descrPtr->srcDev      = descrPtr->ingressDsa.srcDev;
    descrPtr->srcDevIsOwn = SKERNEL_IS_MATCH_DEVICES_MAC(descrPtr->srcDev, descrPtr->ownDev,
                                                         devObjPtr->dualDeviceIdEnable.tti);
    __LOG_PARAM(descrPtr->srcDev);
    __LOG_PARAM(descrPtr->srcDevIsOwn);
}

/**
* @internal snetChtFrameParsingVlanAssign function
* @endinternal
*
* @brief   Frames header Parsing and VLAN/QoS Port Protocol or default assignment
*/
static GT_VOID snetChtFrameParsingVlanAssign
(
    IN SKERNEL_DEVICE_OBJECT           *devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC *descrPtr,
    IN INTERNAL_TTI_DESC_INFO_STC      *internalTtiInfoPtr
)
{
    DECLARE_FUNC_NAME(snetChtFrameParsingVlanAssign);

    GT_U32 regAddress;
    GT_U32 * portVlanCfgEntryPtr;   /* table entry pointer */
    GT_U32 bypassEn;                /* Bridge Bypass Enable */
    GT_U32 portOamLoopBackEn;       /* Port for IEEE 802.3ah loopback mode enable */
    GT_U32  centralizedChassisModeEn = 0;/*Centralized Chassis mode*/
    GT_U32  *ttiPreTtiLookupIngressEPortTablePtr = NULL;/* pointer to the entry in : tti - Pre-TTI Lookup Ingress ePort Table  */
    GT_U32  oamPduTrapEn;/* Enable OAM-PDU packets trapping to CPU */
    GT_BOOL didLocalPortReAssign;
    GT_U32  copyTag1VIDtoSrcEPG; /* value of Copy Tag1 VID to EPG flag */
    GT_U32  ingressExtendedMode; /* QoS ingress Extended Mode*/

    if (descrPtr->marvellTagged)
    {
        __LOG(("Start : ingress Marvell tag parsing \n"));
        SIM_LOG_PACKET_DESCR_SAVE
        snetChtMarvellTagParsing(devObjPtr, descrPtr, internalTtiInfoPtr);
        SIM_LOG_PACKET_DESCR_COMPARE("snetChtMarvellTagParsing");
        __LOG(("Ended : ingress Marvell tag parsing \n"));
    }

    if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        didLocalPortReAssign = snetLion3RemotePortAssignmentGet(devObjPtr, descrPtr);

        __LOG(("TTI - Physical Port Attribute Table, index[%d]", descrPtr->localDevSrcPort));
        regAddress = SMEM_LION2_TTI_PHYSICAL_PORT_ATTRIBUTE_TBL_MEM(devObjPtr, descrPtr->localDevSrcPort);
        descrPtr->eArchExtInfo.ttiPhysicalPortAttributePtr = smemMemGet(devObjPtr, regAddress);

        if(SMEM_CHT_IS_SIP5_20_GET(devObjPtr))
        {
            __LOG(("TTI - Physical Port 2 Attribute Table, index[%d]", descrPtr->localDevSrcPort));
            regAddress = SMEM_SIP5_20_TTI_PHYSICAL_PORT_2_ATTRIBUTE_TBL_MEM(devObjPtr, descrPtr->localDevSrcPort);
            descrPtr->eArchExtInfo.ttiPhysicalPort2AttributePtr = smemMemGet(devObjPtr, regAddress);
        }

        /* check the : Centralized Chassis mode Enable */
        centralizedChassisModeEn =
            SMEM_LION3_TTI_PHYSICAL_PORT_ENTRY_FIELD_GET(devObjPtr,descrPtr,
                SMEM_LION3_TTI_PHYSICAL_PORT_TABLE_FIELDS_CENTRALIZED_CHASSIS_PORT_ENABLE);

        __LOG_PARAM(centralizedChassisModeEn);

        if(centralizedChassisModeEn)
        {
            /* Accessing to table SMEM_LION2_TTI_PHYSICAL_PORT_ATTRIBUTE_TBL_MEM(devObjPtr,descrPtr->localDevSrcPort);
               Is done only after snetLion3RemotePortAssignmentGet that may change descrPtr->localDevSrcPort */
            lion3CentralizedChassisModeSupport(devObjPtr, descrPtr);
        }

        descrPtr->didLocalPortReAssign = didLocalPortReAssign;

        if(didLocalPortReAssign && descrPtr->myPhyPortAssignMode)
        {
            /* code moved from snetLion3RemotePortAssignmentGet(...)
               that was doing (bug) :
               descrPtr->origSrcEPortOrTrnk = descrPtr->localDevSrcPort;
            */
            if(devObjPtr->errata.dsaTagOrigSrcPortNotUpdated)
            {
                __LOG(("Warning (Errata): the Original Src ePort is not updated to the newly assigned source physical port \n"));
                __LOG(("                  but remains the original source ePort from the DSA  \n"));
            }
            else
            {
                descrPtr->origSrcEPortOrTrnk =
                    SMEM_LION3_TTI_PHYSICAL_PORT_ENTRY_FIELD_GET(devObjPtr,descrPtr,
                        SMEM_LION3_TTI_PHYSICAL_PORT_TABLE_FIELDS_PORT_DEFAULT_SOURCE_EPORT_NUMBER);

                __LOG(("new value of descrPtr->origSrcEPortOrTrnk is [%d] \n",
                    descrPtr->origSrcEPortOrTrnk));
            }
        }
        else
        {
            /* you keep the value from the DSA !!! */
        }

        /* set the default ePort of this physical port */
        if(descrPtr->centralizedChassisModeEn == 0)
        {
            descrPtr->eArchExtInfo.localDevSrcEPort =
                SMEM_LION3_TTI_PHYSICAL_PORT_ENTRY_FIELD_GET(devObjPtr,descrPtr,
                    SMEM_LION3_TTI_PHYSICAL_PORT_TABLE_FIELDS_PORT_DEFAULT_SOURCE_EPORT_NUMBER);
            if(descrPtr->marvellTagged == 0)
            {
                descrPtr->origSrcEPortOrTrnk = descrPtr->eArchExtInfo.localDevSrcEPort;
            }
        }
        else
        {
            /* already set in lion3CentralizedChassisModeSupport */
        }
        __LOG(("The default ePort of this physical port is[%d] \n", descrPtr->eArchExtInfo.localDevSrcEPort));

        /* ingressExtendedMode can be set, cannot be cleared */
        ingressExtendedMode =
                SMEM_LION3_TTI_PHYSICAL_PORT_ENTRY_FIELD_GET(devObjPtr, descrPtr,
                    SMEM_LION3_TTI_PHYSICAL_PORT_TABLE_FIELDS_PORT_QOS_MODE);
        if (ingressExtendedMode)
        {
            descrPtr->qos.ingressExtendedMode = ingressExtendedMode;
        }

        if(descrPtr->marvellTagged == 0 /* non cascade port*/ &&
           descrPtr->qos.ingressExtendedMode)
        {
            /* we continue as usual but such config can cause non wanted behavior */
            __LOG(("suspected as configuration ERROR! PORT_QOS_MODE = 'extended' on NON cascade port"));
        }

        /* save the info for LOG usage */
        descrPtr->eArchExtInfo.defaultSrcEPort = descrPtr->eArchExtInfo.localDevSrcEPort;

        /* tti - Pre-TTI Lookup Ingress ePort Table */
        regAddress = SMEM_LION2_TTI_PRE_TTI_LOOKUP_INGRESS_EPORT_TABLE_TBL_MEM(devObjPtr,
                descrPtr->eArchExtInfo.localDevSrcEPort);
        descrPtr->eArchExtInfo.ttiPreTtiLookupIngressEPortTablePtr = smemMemGet(devObjPtr, regAddress);
        ttiPreTtiLookupIngressEPortTablePtr = descrPtr->eArchExtInfo.ttiPreTtiLookupIngressEPortTablePtr;

        portVlanCfgEntryPtr = NULL;
    }
    else
    {
        regAddress = SMEM_CHT_PORT_VLAN_QOS_CONFIG_TBL_MEM(devObjPtr,
                                                            descrPtr->localDevSrcPort);

        portVlanCfgEntryPtr = smemMemGet(devObjPtr, regAddress);

    }

    if(0 == descrPtr->marvellTagged)
    {
        /* moved the UP assignment only for non DSA because :
           1. for DSA we get UP from the DSA
           2. if the port is CC-line card then the 'pre-tti lookup table'
              indexed according to internalTtiInfoPtr->ccLineCard.newSrcEPort
           */
        if(ttiPreTtiLookupIngressEPortTablePtr)
        {
            /*Port UP0*/
            descrPtr->up =
                SMEM_LION3_TTI_DEFAULT_EPORT_ENTRY_FIELD_GET(devObjPtr, descrPtr,
                    SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_PORT_UP0);

            if(SMEM_CHT_IS_SIP6_30_GET(devObjPtr))
            {
                /* assign default values CFIDE1 */
                descrPtr->cfidei =
                    SMEM_LION3_TTI_DEFAULT_EPORT_ENTRY_FIELD_GET(devObjPtr, descrPtr,
                         SMEM_SIP6_30_TTI_DEFAULT_EPORT_TABLE_FIELDS_EPORT_DEI0_E);

                __LOG(("Port CFI/DEI0 = [%d] \n", descrPtr->cfidei));
            }
        }
        else
        {
            /* Port Default User Priority */
            descrPtr->up =
                SNET_CHT_PORT_VLAN_QOS_UP_MAC(devObjPtr, portVlanCfgEntryPtr);
        }

        __LOG(("Port UP0 = [%d] \n",
                      descrPtr->up));

        descrPtr->ingressDsa.qosProfile = 0;
    }
    else
    {
        /* this function can be called only after
           descrPtr->eArchExtInfo.ttiPhysicalPortAttributePtr is not NULL */

        /* do logic of <IsLooped> and <DropOnSource> */
        snetChtIngressIsLoopedAndDropOnSource(devObjPtr, descrPtr);
    }


    /* Multi-Port Group FDB Lookup support */
    if(devObjPtr->supportMultiPortGroupFdbLookUpMode)
    {
        SIM_LOG_PACKET_DESCR_SAVE
        snetLionIngressSourcePortGroupIdGet(devObjPtr, descrPtr);
        SIM_LOG_PACKET_DESCR_COMPARE("snetLionIngressSourcePortGroupIdGet");
    }

    /* do L2 parsing (vlan tag , ethertype , nested vlan , encapsulation) */
    __LOG(("do L2 parsing (vlan tag , ethertype , nested vlan , encapsulation)"));
    SIM_LOG_PACKET_DESCR_SAVE
    snetChtL2Parsing(devObjPtr, descrPtr,SNET_CHT_FRAME_PARSE_MODE_PORT_E,internalTtiInfoPtr);
    SIM_LOG_PACKET_DESCR_COMPARE("snetChtL2Parsing");

    /*******************************************************/
    /* save original pointer in case of Tunnel termination */
    /*******************************************************/
    descrPtr->origInfoBeforeTunnelTermination.originalL2Ptr = descrPtr->macDaPtr;
    descrPtr->origInfoBeforeTunnelTermination.originalL3Ptr = descrPtr->l3StartOffsetPtr;

    descrPtr->payloadPtr = descrPtr->origInfoBeforeTunnelTermination.originalL3Ptr;
    /* use descrPtr->origByteCount to calculate payload length because
       descrPtr->byteCount may be some constant value for cut through packets */
    descrPtr->payloadLength = descrPtr->origByteCount - (descrPtr->payloadPtr - descrPtr->macDaPtr);

    /* L3, L4 protocols parsing */
    __LOG(("L3, L4 protocols parsing"));
    SIM_LOG_PACKET_DESCR_SAVE
    snetChtL3L4ProtParsing(devObjPtr, descrPtr, descrPtr->etherTypeOrSsapDsap, internalTtiInfoPtr);
    SIM_LOG_PACKET_DESCR_COMPARE("snetChtL3L4ProtParsing");

    /* save original l23HeaderSize in case of Tunnel Termination */
    descrPtr->origInfoBeforeTunnelTermination.originalL23HeaderSize = descrPtr->l23HeaderSize;

    if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        /* do MRU only after the <byte count> may be changed inside
           snetChtL3L4ProtParsing(...) due to 'cut through' logic.

           for sip6 see function snetSip6CutThroughTtiSupport(...)
        */
        snetLion3TtiDefaulteEportExceptionMruCheck(devObjPtr, descrPtr);
    }

    /* save info that maybe needed by PCL */
    descrPtr->origInfoBeforeTunnelTermination.arp = descrPtr->arp;
    descrPtr->origInfoBeforeTunnelTermination.mpls = descrPtr->mpls;
    descrPtr->origInfoBeforeTunnelTermination.isIp = descrPtr->isIp;
    descrPtr->origInfoBeforeTunnelTermination.isIPv4 = descrPtr->isIPv4;

    if (descrPtr->marvellTagged)
    {
        if((descrPtr->packetCmd == SKERNEL_EXT_PKT_CMD_FORWARD_E) ||
            (descrPtr->packetCmd == SKERNEL_EXT_PKT_CMD_MIRROR_TO_CPU_E))
        {
            if (descrPtr->marvellTaggedExtended != SKERNEL_EXT_DSA_TAG_1_WORDS_E)
            {
                if(ttiPreTtiLookupIngressEPortTablePtr)
                {
                    bypassEn =
                        SMEM_LION3_TTI_DEFAULT_EPORT_ENTRY_FIELD_GET(devObjPtr, descrPtr,
                            SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_EXTENDED_DSA_BYPASS_BRIDGE);
                }
                else
                {
                    /* Bridge BypassEn */
                    bypassEn =
                        SNET_CHT_PORT_VLAN_QOS_DSA_CSCD_BYPASS_BRG_GET_MAC(devObjPtr,
                                                                           portVlanCfgEntryPtr);
                }

                if (bypassEn == 1)
                {
                    __LOG(("Bridge BypassEn == 1"));
                    descrPtr->bypassBridge = bypassEn;
                }
                else
                if(SMEM_CHT_IS_SIP5_25_GET(devObjPtr))
                {
                    /* do not modify descrPtr->egressFilterRegistered */
                    /* NOTE: I do it only for sip5.25 because LUA test 'vss_cc5_vss3'
                       fail when done for all sip5 devices.
                       because currently WM code in L2i not know to set 'egressFilterRegistered = 0'
                       on all cases.
                       but in sip 5.25 the L2i never set to '0' if incoming to L2i as
                       'egressFilterRegistered = 1'
                    */
                }
                else
                {
                    /* we are going to do FDB lookup again */
                    __LOG(("Bridge BypassEn == 0 , we are going to do FDB lookup again. (set descrPtr->egressFilterRegistered = 0)"));
                    descrPtr->egressFilterRegistered = 0;
                }

                if (descrPtr->nestedVlanAccessPort)
                {
                    descrPtr->tagSrcTagged[SNET_CHT_TAG_0_INDEX_CNS] = 0;
                }
            }
        }
        else
        {
            descrPtr->bypassBridge = 1;
        }
    }
    else
    {
        if(portVlanCfgEntryPtr)
        {
            /* MirrorToIngress AnalyzerPort */
            __LOG(("MirrorToIngress AnalyzerPort"));
            descrPtr->rxSniff =
                SNET_CHT_PORT_VLAN_QOS_MIRR_INGR_ANALAYZER_GET_MAC(devObjPtr,
                                                                   portVlanCfgEntryPtr);
        }
    }

    /* VLAN and QoS Profile Assign */
    __LOG(("VLAN and QoS Profile Assign"));

    SIM_LOG_PACKET_DESCR_SAVE
    snetChtVlanQosProfAssign(devObjPtr, descrPtr, descrPtr->etherTypeOrSsapDsap, descrPtr->l2Encaps,
                             portVlanCfgEntryPtr);
    SIM_LOG_PACKET_DESCR_COMPARE("snetChtVlanQosProfAssign");

    if(descrPtr->eArchExtInfo.ttiPhysicalPortAttributePtr)
    {
        descrPtr->localDevSrcTrunkId =
            SMEM_LION3_TTI_PHYSICAL_PORT_ENTRY_FIELD_GET(devObjPtr,descrPtr,
                SMEM_LION3_TTI_PHYSICAL_PORT_TABLE_FIELDS_TRUNK_ID);
    }
    else
    {
        descrPtr->localDevSrcTrunkId =
            SNET_CHT_PORT_VLAN_QOS_TRUNK_ID_GET_MAC(devObjPtr,
                                                    portVlanCfgEntryPtr);
    }

    if(descrPtr->localDevSrcTrunkId)
    {
        __LOG(("localDevSrcTrunkId[%d] \n",
                      descrPtr->localDevSrcTrunkId));
    }

    if(ttiPreTtiLookupIngressEPortTablePtr)
    {
        /* 802.3ah OAM Loopback Enable Per Port */
        portOamLoopBackEn =
            SMEM_LION3_TTI_DEFAULT_EPORT_ENTRY_FIELD_GET(devObjPtr, descrPtr,
                SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_OAM_LINK_LAYER_LOOPBACK_EN);

        /* Enable OAM-PDU packets trapping to CPU */
        oamPduTrapEn =
            SMEM_LION3_TTI_DEFAULT_EPORT_ENTRY_FIELD_GET(devObjPtr, descrPtr,
                SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_OAM_LINK_LAYER_PDU_TRAP_EN);

    }
    else
    {
     /* 802.3ah OAM Loopback Enable Per Port */
        portOamLoopBackEn =
            SMEM_U32_GET_FIELD(portVlanCfgEntryPtr[1], 30, 1);
        oamPduTrapEn =
            SMEM_U32_GET_FIELD(portVlanCfgEntryPtr[1], 31, 1);
    }

    __LOG(("OAM Loopback Enable [%d] trap[%d]",portOamLoopBackEn,oamPduTrapEn));

    if ((descrPtr->srcDevIsOwn && descrPtr->localDevSrcTrunkId > 0) &&
        /* for DSA tagged on eArch device we not override the logic of the DSA assignment */
        ((devObjPtr->supportEArch == 0) || (devObjPtr->supportEArch == 1 && descrPtr->marvellTagged == 0)))
    {
        descrPtr->origIsTrunk = 1;
        descrPtr->origSrcEPortOrTrnk = descrPtr->localDevSrcTrunkId;
    }

    if(centralizedChassisModeEn == 0 &&
       descrPtr->srcDevIsOwn &&
       descrPtr->myPhyPortAssignMode &&
       descrPtr->marvellTagged)
    {
        /* support for remote physical port is trunk member */
        descrPtr->origIsTrunk = (descrPtr->localDevSrcTrunkId > 0) ? 1 : 0;
        if(descrPtr->origIsTrunk)
        {
            descrPtr->origSrcEPortOrTrnk = descrPtr->localDevSrcTrunkId;
            __LOG(("The remote physical port recognized as member of trunk [%d] \n",
                descrPtr->localDevSrcTrunkId));
        }
        else
        {
            if(devObjPtr->errata.dsaTagOrigSrcPortNotUpdated == 0)
            {
                descrPtr->origSrcEPortOrTrnk = descrPtr->eArchExtInfo.defaultSrcEPort;
                __LOG(("The remote physical port not recognized as member of trunk , so use defaultSrcEPort[%d] \n",
                    descrPtr->origSrcEPortOrTrnk));
            }
            else
            {
                __LOG(("WARNING (Erratum): The remote physical port not recognized as member of trunk , but NOT use defaultSrcEPort[%d] \n",
                    descrPtr->origSrcEPortOrTrnk));
            }
        }

        if(devObjPtr->errata.ttiTemotePhysicalPortOrigSrcPhyIsTrunk == 0)/*fix for the Erratum */
        {
            /*
                OutDesc <OrigPhySrcIsTrunk>             =  Physical Port <Trunk Group ID> != 0  ? 1 : 0
                OutDesc <OrigPhySrcPort/trunk>         =  Physical Port <TrunkID> != 0  ? Port<Trunk Group ID> :  Desc<LocalDevSrcPort>
                OutDesc <PhySrcMcFilteringEn>          = SRCePort<PhySrcMCFilteringEn> // Should be already set
            */
            descrPtr->eArchExtInfo.origSrcPhyIsTrunk   = descrPtr->origIsTrunk;
            descrPtr->eArchExtInfo.origSrcPhyPortTrunk = descrPtr->origSrcEPortOrTrnk;

            /* simulation do logic only after TTI action (see snetCht3TtiUnitPart2(...))
               so we not doing it here */
            /*OutDesc <PhySrcMcFilteringEn>          = SRCePort<PhySrcMCFilteringEn> -- Should be already set */
        }
    }

    if(SMEM_CHT_IS_SIP5_GET(devObjPtr)&& descrPtr->marvellTagged == 0)/* for DSA tag was already set*/
    {
        descrPtr->eArchExtInfo.origSrcPhyIsTrunk   = descrPtr->origIsTrunk;
        descrPtr->eArchExtInfo.origSrcPhyPortTrunk = (descrPtr->origIsTrunk == 1) ? descrPtr->localDevSrcTrunkId : descrPtr->localDevSrcPort;
    }

    if ((descrPtr->l3StartOffsetPtr[0]/*subType*/ == 0x03) && (descrPtr->etherTypeOrSsapDsap == SNET_CHT3_ETHERTYPE_OAM_SLOW_PROTOCOL)
        &&
        (descrPtr->macDaPtr[0] == 0x01) &&
        (descrPtr->macDaPtr[1] == 0x80) &&
        (descrPtr->macDaPtr[2] == 0xc2) &&
        (descrPtr->macDaPtr[3] == 0x00) &&
        (descrPtr->macDaPtr[4] == 0x00) &&
        (descrPtr->macDaPtr[5] == 0x02))

    {
        descrPtr->oam = GT_TRUE;
    }

    if(devObjPtr->pclSupport.pclUseCFMEtherType == 0)
    {
        __LOG(("Do CFM classification from TTI unit \n"));
        snetChtCfmEtherTypeCheck(devObjPtr, descrPtr);
    }
    else
    {
        __LOG(("CFM classification could be done in PCL unit \n"));
    }

    if(devObjPtr->portGroupSharedDevObjPtr)
    {
        if(devObjPtr->supportEArch)
        {
            /* the port is already global port , no need to convert */
            descrPtr->localPortGroupPortAsGlobalDevicePort = descrPtr->localDevSrcPort;
        }
        else
        {
            /* set the port */
            descrPtr->localPortGroupPortAsGlobalDevicePort =  /* global port */
                SMEM_CHT_GLOBAL_PORT_FROM_LOCAL_PORT_MAC(devObjPtr,
                            descrPtr->localDevSrcPort);
        }

        /* set the port/trunk */
        if((descrPtr->origIsTrunk == 0) && (descrPtr->marvellTagged == 0))
        {
            /* came from local port group port */
            descrPtr->localPortTrunkAsGlobalPortTrunk =
                descrPtr->localPortGroupPortAsGlobalDevicePort;
        }
        else
        {
            /* came from cascade port / came from trunk*/
            descrPtr->localPortTrunkAsGlobalPortTrunk = descrPtr->origSrcEPortOrTrnk;
        }
    }
    else
    {
        /* set the port */
        descrPtr->localPortGroupPortAsGlobalDevicePort = descrPtr->localDevSrcPort;

        /* set the port/trunk */
        descrPtr->localPortTrunkAsGlobalPortTrunk = descrPtr->origSrcEPortOrTrnk;
    }
    __LOG(("localPortGroupPortAsGlobalDevicePort = [%d]",descrPtr->localPortGroupPortAsGlobalDevicePort));
    __LOG(("localPortTrunkAsGlobalPortTrunk = [%d]",descrPtr->localPortTrunkAsGlobalPortTrunk));

    if(devObjPtr->supportOamPduTrap && descrPtr->oam && oamPduTrapEn)
    {
        /* OAM_TRAP is assigned by the TTI block (but we simulate it here) */
        __LOG(("OAM_TRAP is assigned by the TTI block (but we simulate it here)"));
        descrPtr->packetCmd = SKERNEL_EXT_PKT_CMD_TRAP_TO_CPU_E;
        if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
        {
            smemRegFldGet(devObjPtr, SMEM_LION3_TTI_UNIT_GLOBAL_CONFIG_EXT_REG(devObjPtr),
                          17, 8, &descrPtr->cpuCode);
        }
        else
        {
            descrPtr->cpuCode = SNET_XCAT_OAM_PDU_TRAP;
        }

    }
    else if(portOamLoopBackEn && devObjPtr->supportPortOamLoopBack)
    {
        /* OAM loopback is forwarded to the pipe with a FROM_CPU DSA tag,
           so it bypasses all ingress/egress filters */
        descrPtr->packetCmd = SKERNEL_EXT_PKT_CMD_FROM_CPU_E;
        descrPtr->bypassIngressPipe = 1;

        /* set fields relevant to 'from_CPU' processing */
        descrPtr->egressFilterEn = 0;
        /* modify value so EPCL will not need to recognize OAM */
        descrPtr->ingressDsa.fromCpu_egressFilterEn = descrPtr->egressFilterEn;
        descrPtr->useVidx = 0;
        descrPtr->targetIsTrunk = 0;
        descrPtr->trgDev = descrPtr->ownDev;
        descrPtr->trgEPort = descrPtr->localDevSrcPort;
    }

    if(SMEM_CHT_IS_SIP6_10_GET(devObjPtr))
    {
        descrPtr->ppuProfileIdx = SMEM_LION3_TTI_PHYSICAL_PORT_ENTRY_FIELD_GET(devObjPtr, descrPtr,
                    SMEM_SIP6_10_TTI_PHYSICAL_PORT_TABLE_FIELDS_PPU_PROFILE);
    }

    /* Source EPG assignment */
    if(SMEM_CHT_IS_SIP6_30_GET(devObjPtr))
    {
        descrPtr->srcEpg = SMEM_LION3_TTI_DEFAULT_EPORT_ENTRY_FIELD_GET(devObjPtr, descrPtr,
                SMEM_SIP6_30_TTI_DEFAULT_EPORT_TABLE_FIELDS_SOURCE_EPG_E);

        __LOG(("Assigned default Source EPG value [%d]\n" ,descrPtr->srcEpg));

        copyTag1VIDtoSrcEPG = SMEM_LION3_TTI_DEFAULT_EPORT_ENTRY_FIELD_GET(devObjPtr, descrPtr,
                SMEM_SIP6_30_TTI_DEFAULT_EPORT_TABLE_FIELDS_COPY_TAG1_VID_TO_SRC_EPG_E);

        __LOG(("copy Tag1 VID to Src EPG value [%d]\n" ,copyTag1VIDtoSrcEPG));
        if (copyTag1VIDtoSrcEPG && descrPtr->tagSrcTagged[SNET_CHT_TAG_1_INDEX_CNS])
        {
            /* copy value of Tag1 VID to source EPG */
            descrPtr->srcEpg = descrPtr->vid1;
            __LOG(("Assigned Source EPG by Tag1 VID value [%d]\n" ,descrPtr->srcEpg));
        }
    }
}

static GT_VOID snetChtCfmEtherTypeCheck
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr
)
{
    GT_U32 * regPtr;                /* register entry pointer */
    GT_U32 cfmEtherType;            /* CFM ethertype */

    DECLARE_FUNC_NAME(snetChtCfmEtherTypeCheck);

    regPtr = smemMemGet(devObjPtr, SMEM_CHT3_CFM_ETHERTYPE_REG(devObjPtr));

    /* CFM Ethertype */
    cfmEtherType = SMEM_U32_GET_FIELD(regPtr[0], 0, 16);

    if (cfmEtherType == descrPtr->etherTypeOrSsapDsap)
    {
        __LOG(("CFM Ethertype \n"));
        descrPtr->cfm  = GT_TRUE;
        descrPtr->udb[0] = descrPtr->l3StartOffsetPtr[0];
        descrPtr->udb[1] = descrPtr->l3StartOffsetPtr[1];
        descrPtr->udb[2] = descrPtr->l3StartOffsetPtr[2];
    }
}

/**
* @internal snetChtMacCounter1024OrMoreGet function
* @endinternal
*
* @brief   get the counter that need to count the 1024+ bytes of the packet
*         !!! NOT relevant to CG MAC (100G MAC) !!! (because hold different enum for counters!)
*/
SNET_CHT_PORT_MIB_COUNTERS_ENT snetChtMacCounter1024OrMoreGet
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32  frameSize,
    IN GT_U32  macPort
)
{
    SNET_CHT_PORT_MIB_COUNTERS_ENT  mibCounter;
    GT_U32 mib4CountSelect; /* 0: 1024to1518, 1518toMax counters are disabled or not supported
                                  1: 1024toMax counter is divided into 1024to1518 and 1519toMax  (xCat2 and above)  */
    GT_U32 mib4LimitSelect;  /*  This bit define threshold between 1024to1518 and 1518toMax counters:
                                    0 => 1518; 1 => 1522. Valid only if mib4CountSelect == 1; */

    if(CHT_IS_CG_OR_MTI_MAC_USED_MAC(devObjPtr,macPort))
    {
        skernelFatalError("snetChtMacCounter1024OrMoreGet: GC/MTI MAC not supported \n");
    }

    mib4CountSelect = snetChtPortMacFieldGet(devObjPtr,macPort,SNET_CHT_PORT_MAC_FIELDS_mib_4_count_hist_E);
    mib4LimitSelect = snetChtPortMacFieldGet(devObjPtr,macPort,SNET_CHT_PORT_MAC_FIELDS_mib_4_limit_1518_1522_E);

    if (mib4CountSelect == 0)
    {
        __LOG_NO_LOCATION_META_DATA(("increment counter of: 1024_TO_MAX_OCTETS : counting 1024 to max \n"));
        mibCounter = SNET_CHT_PORT_MIB_COUNTERS_FRAMES_1024_TO_MAX_OCTETS_E;  /* Frames1024toMaxOctets */
    }
    else
    {
        if (frameSize > (GT_U32)(mib4LimitSelect ?
                     SNET_MIB_CNT_THRESHOLD_1522 : SNET_MIB_CNT_THRESHOLD_1518))
        {
            if(mib4LimitSelect)
            {
                __LOG_NO_LOCATION_META_DATA(("increment counter of: 1024_TO_MAX_OCTETS : counting 1523 to max \n"));
            }
            else
            {
                __LOG_NO_LOCATION_META_DATA(("increment counter of: 1024_TO_MAX_OCTETS : counting 1519 to max \n"));
            }

            mibCounter = SNET_CHT_PORT_MIB_COUNTERS_FRAMES_1024_TO_MAX_OCTETS_E;  /* Frames1519toMaxOctets replace Frames1024toMaxOctets */
        }
        else
        {
            if(mib4LimitSelect)
            {
                __LOG_NO_LOCATION_META_DATA(("increment counter of: 1024_TO_1518_OCTETS : counting 1024 to 1522 \n"));
            }
            else
            {
                __LOG_NO_LOCATION_META_DATA(("increment counter of: 1024_TO_1518_OCTETS : counting 1024 to 1518 \n"));
            }

            mibCounter = SNET_CHT_PORT_MIB_COUNTERS_FRAMES_1024_TO_1518_OCTETS_E;  /* Frames1024to1518Octets replace Sent deferred */
        }
    }


    return mibCounter;
}

/**
* @internal snetChtRxMacCountUpdate function
* @endinternal
*
* @brief   Global update counters function (MAC MIB Counters + CPU port)
*/
static GT_VOID snetChtRxMacCountUpdate
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_BOOL overSize,
    IN SNET_FLOW_CONTROL_PACKET_TYPE_ENT fcPacketType,
    IN GT_U32  macPort
)
{
    DECLARE_FUNC_NAME(snetChtRxMacCountUpdate);

    GT_U32 regAddr;                     /* Register address */
    GT_U32 * regPtr;                    /* Register entry pointer */
    GT_U32 fieldVal;                    /* Register field value */
    GT_U32 octets;                      /* Frame size */
    GT_U32 rxHistEnable;                /* This bit enables update of histogram
                                           counters for incoming traffic.
                                            0 = Disabled.  1 = Enabled.*/
    GT_U32 forwardPfcFramesEnable;      /* register field value */
    GT_U32  *baseMibPtr;/*base memory of the MIB counters of the port*/
    SNET_CHT_PORT_MIB_COUNTERS_ENT mibCounter;
    SNET_CHT_PORT_MIB_COUNTERS_100G_PORT_ENT mibCounter_100G;
    SNET_CHT_PORT_MTI_STATISTIC_COUNTERS_ENT mtiCounterType;
    GT_BOOL isCgOrMtiPort = CHT_IS_CG_OR_MTI_MAC_USED_MAC(devObjPtr,macPort);

    if(SMEM_CHT_IS_SIP5_GET(devObjPtr) && SNET_NOT_FC_PACKET_E != fcPacketType &&
      !SMEM_IS_PIPE_FAMILY_GET(devObjPtr))/* PIPE not hold the TTI unit */
    {
        /* Increment Received Flow Control Packets Counter */
        regAddr = SMEM_LION3_TTI_RECEIVED_FLOW_CONTROL_PACKET_COUNTER_REG(devObjPtr);
        smemRegFldGet(devObjPtr, regAddr, 0, 16, &fieldVal);
        __LOG(("Global counter : Increment Received Flow Control Packets Counter from [%d] \n", fieldVal));
        smemRegFldSet(devObjPtr, regAddr, 0, 16, ++fieldVal);
    }

    if(devObjPtr->cpuPortNoSpecialMac)
    {
        if(GT_TRUE == descrPtr->isFromSdma)
        {
            /* device does not have dedicated MAC counters for CPU port */
            __LOG(("SDMA traffic hold hold counters on port[%d] \n",macPort));
            return;
        }
    }
    else
    {
        if (IS_CHT_CPU_PORT(macPort))
        {
            if( GT_FALSE == descrPtr->isFromSdma )
            {
                regAddr = SMEM_CHT_CPU_PORT_GOOD_FRAMES_COUNT_REG(devObjPtr);
                regPtr = smemMemGet(devObjPtr, regAddr);
                (*regPtr)++;

                regAddr = SMEM_CHT_CPU_PORT_GOOD_OCTETS_COUNT_REG(devObjPtr);
                regPtr = smemMemGet(devObjPtr, regAddr);
                (*regPtr) += descrPtr->byteCount;

                __LOG(("Update RX MAC counters on SGMII CPU port[%d] \n",macPort));
            }
            else
            {
                __LOG(("SDMA traffic hold hold counters on port[%d] \n",macPort));
            }
            return;
        }
    }

    __LOG(("Update RX MAC counters on port[%d] \n",macPort));

    if(!CHT_IS_CG_OR_MTI_MAC_USED_MAC(devObjPtr,macPort) &&
       !CHT_MAC_PORT_MIB_COUNTERS_IS_XG_LOGIC_MAC(devObjPtr,macPort))
    {
        fieldVal = snetChtPortMacFieldGet(devObjPtr, macPort,
            SNET_CHT_PORT_MAC_FIELDS_MIBCntEn_E);

        /* check that MIB counters are enabled or not */
        if (fieldVal == 0)
        {
            return;
        }
    }

    /* get Rx Histogram En */
    rxHistEnable = snetChtPortMacFieldGet(devObjPtr, macPort,
        SNET_CHT_PORT_MAC_FIELDS_histogram_rx_en_E);

    if (CHT_IS_MTI_MAC_USED_MAC(devObjPtr,macPort))
    {
        baseMibPtr = NULL;
    }
    else
    {
        /* address of the start of counters for this port */
        regAddr = CHT_MAC_PORTS_MIB_COUNTERS_BASE_ADDRESS_CNS(devObjPtr,macPort);
        baseMibPtr = smemMemGet(devObjPtr,regAddr);
    }

    if(fcPacketType != SNET_NOT_FC_PACKET_E)
    {
        if(baseMibPtr && CHT_IS_100G_MAC_USED_MAC(devObjPtr,macPort))
        {
            /* Valid Flow Control packet */
            __LOG(("increment counter of: Valid Flow Control packet \n"));
            snetChtPortMibOffsetUpdate_100GMac(devObjPtr,macPort,baseMibPtr,
                    SNET_CHT_PORT_MIB_COUNTERS_100G_PORT_aCBFCPAUSEFramesReceived_0_E/*queue ignored*/,
                    1);
        }
        else if (CHT_IS_MTI_MAC_USED_MAC(devObjPtr,macPort))
        {
            /* Valid Flow Control packet */
            __LOG(("increment MTI counter of: Valid Flow Control packet \n"));
            snetFalconPortStatisticCounterUpdate(devObjPtr,macPort,
                    SNET_CHT_PORT_MTI_STATISTIC_COUNTERS_RX_aCBFCPAUSEFramesReceived0_E,
                    1);
        }
    }

    if(fcPacketType == SNET_FC_PACKET_E)
    {
        if(isCgOrMtiPort)
        {
        }
        else if(baseMibPtr)
        {
            /* Valid Flow Control packet */
            __LOG(("increment counter of: Valid Flow Control packet \n"));
            snetChtPortMibOffsetUpdate(devObjPtr,macPort,baseMibPtr,
                    SNET_CHT_PORT_MIB_COUNTERS_FC_RECEIVED_E,
                    1);
        }

        return;
    }

    if ( (devObjPtr->supportForwardPfcFrames) &&
         (fcPacketType == SNET_PFC_PACKET_E) &&
         (macPort != SNET_CHT_CPU_PORT_CNS))
    {
        forwardPfcFramesEnable = snetChtPortMacFieldGet(devObjPtr, macPort,
            SNET_CHT_PORT_MAC_FIELDS_forwardPfcFramesEnable_E);
        if (forwardPfcFramesEnable == 1)
        {
            if(isCgOrMtiPort)
            {
            }
            else if(baseMibPtr)
            {
                /* PFC packet, processed by Ingress pipe - update 802.3x Received MIB counter */
                __LOG(("increment counter of:PFC packet, processed by Ingress pipe - update 802.3x Received MIB counter \n"));
                snetChtPortMibOffsetUpdate(devObjPtr,macPort,baseMibPtr,
                    SNET_CHT_PORT_MIB_COUNTERS_FC_RECEIVED_E,
                    1);
            }

            return;
        }

        if(isCgOrMtiPort)
        {
        }
        else if(baseMibPtr)
        {
            /* Terminated PFC packet - update UknownMacControlFrame MIB counter (multiplexed with Late Collision counter) */
            __LOG(("increment counter of: Terminated PFC packet - update UknownMacControlFrame MIB counter (multiplexed with Late Collision counter) \n"));
            snetChtPortMibOffsetUpdate(devObjPtr,macPort,baseMibPtr,
                    SNET_CHT_PORT_MIB_COUNTERS_LATE_COLLISION_E,
                    1);
        }
        return;
    }

    if ( (devObjPtr->supportForwardUnknowMacControlFrames) &&
         (fcPacketType == SNET_UNKNOWN_FC_PACKET_E) )
    {
        if(isCgOrMtiPort)
        {
        }
        else if(baseMibPtr)
        {
            /* multiplexed with Late Collision counter */
            __LOG(("increment counter of:multiplexed with Late Collision counter \n"));
            snetChtPortMibOffsetUpdate(devObjPtr,macPort,baseMibPtr,
                    SNET_CHT_PORT_MIB_COUNTERS_LATE_COLLISION_E,
                    1);
        }

        return;
    }

    if(descrPtr->byteCount < 60)
    {
        /* undersize packets received */
        __LOG(("increment counter of: undersize packets received \n"));
        if(baseMibPtr && CHT_IS_100G_MAC_USED_MAC(devObjPtr,macPort))
        {
            snetChtPortMibOffsetUpdate_100GMac(devObjPtr,macPort,baseMibPtr,
                    SNET_CHT_PORT_MIB_COUNTERS_100G_PORT_etherStatsUndersizePkts_E,
                    1);
        }
        else if (CHT_IS_MTI_MAC_USED_MAC(devObjPtr,macPort))
        {
            __LOG(("increment MTI counter of: undersize packets received \n"));
            snetFalconPortStatisticCounterUpdate(devObjPtr,macPort,
                    SNET_CHT_PORT_MTI_STATISTIC_COUNTERS_RX_etherStatsUndersizePkts_E,
                    1);
        }
        else if(baseMibPtr)
        {
            snetChtPortMibOffsetUpdate(devObjPtr,macPort,baseMibPtr,
                    SNET_CHT_PORT_MIB_COUNTERS_UNDERSIZE_E,
                    1);
        }

        /* bad octets received */
        __LOG(("increment counter of: bad octets received \n"));

        if(baseMibPtr && CHT_IS_100G_MAC_USED_MAC(devObjPtr,macPort))
        {
            snetChtPortMibOffsetUpdate_100GMac(devObjPtr,macPort,baseMibPtr,
                    SNET_CHT_PORT_MIB_COUNTERS_100G_PORT_ifInErrors_E,
                    1);
        }
        else if (CHT_IS_MTI_MAC_USED_MAC(devObjPtr,macPort))
        {
            /* Valid Flow Control packet */
            __LOG(("increment MTI counter of: undersize packets received \n"));
            snetFalconPortStatisticCounterUpdate(devObjPtr,macPort,
                    SNET_CHT_PORT_MTI_STATISTIC_COUNTERS_RX_ifInErrors_E,
                    1);
        }
        else if(baseMibPtr)
        {
            snetChtPortMibOffsetUpdate(devObjPtr,macPort,baseMibPtr,
                    SNET_CHT_PORT_MIB_COUNTERS_BAD_OCTETS_RECEIVED_E,
                    descrPtr->byteCount);
        }

        return;
    }

    if (overSize == GT_TRUE)
    {
        /* oversize packets received */
        __LOG(("increment counter of: oversize packets received \n"));
        if(baseMibPtr && CHT_IS_100G_MAC_USED_MAC(devObjPtr,macPort))
        {
            snetChtPortMibOffsetUpdate_100GMac(devObjPtr,macPort,baseMibPtr,
                    SNET_CHT_PORT_MIB_COUNTERS_100G_PORT_etherStatsOversizePkts_E,
                    1);
        }
        else if (CHT_IS_MTI_MAC_USED_MAC(devObjPtr,macPort))
        {
            __LOG(("increment MTI counter of: oversize packets received \n"));
            snetFalconPortStatisticCounterUpdate(devObjPtr,macPort,
                    SNET_CHT_PORT_MTI_STATISTIC_COUNTERS_RX_etherStatsOversizePkts_E,
                    1);
        }
        else if(baseMibPtr)
        {
            snetChtPortMibOffsetUpdate(devObjPtr,macPort,baseMibPtr,
                    SNET_CHT_PORT_MIB_COUNTERS_OVERSIZE_E,
                    1);
        }

        /* bad octets received */
        if(isCgOrMtiPort)
        {
        }
        else if(baseMibPtr)
        {
            /* Read MRU size */
            fieldVal = snetChtPortMacFieldGet(devObjPtr, macPort,
                SNET_CHT_PORT_MAC_FIELDS_mru_E);

            __LOG(("increment counter of: bad octets received \n"));
            snetChtPortMibOffsetUpdate(devObjPtr,macPort,baseMibPtr,
                    SNET_CHT_PORT_MIB_COUNTERS_BAD_OCTETS_RECEIVED_E,
                    fieldVal);
        }
    }
    else
    if (descrPtr->macDaType == SKERNEL_MULTICAST_MAC_E)
    {
        __LOG(("increment counter of: multicast mac \n"));
        if(baseMibPtr && CHT_IS_100G_MAC_USED_MAC(devObjPtr,macPort))
        {
            snetChtPortMibOffsetUpdate_100GMac(devObjPtr,macPort,baseMibPtr,
                    SNET_CHT_PORT_MIB_COUNTERS_100G_PORT_ifInMulticastPkts_E,
                    1);
        }
        else if (CHT_IS_MTI_MAC_USED_MAC(devObjPtr,macPort))
        {
            __LOG(("increment MTI counter of: multicast mac \n"));
            snetFalconPortStatisticCounterUpdate(devObjPtr,macPort,
                    SNET_CHT_PORT_MTI_STATISTIC_COUNTERS_RX_ifInMulticastPkts_E,
                    1);
        }
        else if(baseMibPtr)
        {
            snetChtPortMibOffsetUpdate(devObjPtr,macPort,baseMibPtr,
                    SNET_CHT_PORT_MIB_COUNTERS_MULTICAST_FRAMES_RECEIVED_E,
                    1);
        }
    }
    else
    if (descrPtr->macDaType == SKERNEL_BROADCAST_MAC_E)
    {
        __LOG(("increment counter of: broadcast mac \n"));
        if(baseMibPtr && CHT_IS_100G_MAC_USED_MAC(devObjPtr,macPort))
        {
            snetChtPortMibOffsetUpdate_100GMac(devObjPtr,macPort,baseMibPtr,
                    SNET_CHT_PORT_MIB_COUNTERS_100G_PORT_ifInBroadcastPkts_E,
                    1);
        }
        else if (CHT_IS_MTI_MAC_USED_MAC(devObjPtr,macPort))
        {
            __LOG(("increment MTI counter of: broadcast mac \n"));
            snetFalconPortStatisticCounterUpdate(devObjPtr,macPort,
                    SNET_CHT_PORT_MTI_STATISTIC_COUNTERS_RX_ifInBroadcastPkts_E,
                    1);
        }
        else if(baseMibPtr)
        {
            snetChtPortMibOffsetUpdate(devObjPtr,macPort,baseMibPtr,
                    SNET_CHT_PORT_MIB_COUNTERS_BROADCAST_FRAMES_RECEIVED_E,
                    1);
        }
    }
    else
    {
        __LOG(("increment counter of: unicast mac \n"));
        if(baseMibPtr && CHT_IS_100G_MAC_USED_MAC(devObjPtr,macPort))
        {
            snetChtPortMibOffsetUpdate_100GMac(devObjPtr,macPort,baseMibPtr,
                    SNET_CHT_PORT_MIB_COUNTERS_100G_PORT_ifInUcastPkts_E,
                    1);
        }
        else if (CHT_IS_MTI_MAC_USED_MAC(devObjPtr,macPort))
        {
            __LOG(("increment MTI counter of: unicast mac \n"));
            snetFalconPortStatisticCounterUpdate(devObjPtr,macPort,
                    SNET_CHT_PORT_MTI_STATISTIC_COUNTERS_RX_ifInUcastPkts_E,
                    1);
        }
        else if(baseMibPtr)
        {
            snetChtPortMibOffsetUpdate(devObjPtr,macPort,baseMibPtr,
                    SNET_CHT_PORT_MIB_COUNTERS_GOOD_UNICAST_FRAMES_RECEIVED_E,
                    1);
        }
    }

    /* Good OctetsReceived */
    if (overSize == GT_FALSE)
    {
        __LOG(("increment counter of: Good Octets Received \n"));
        if(baseMibPtr && CHT_IS_100G_MAC_USED_MAC(devObjPtr,macPort))
        {
            snetChtPortMibOffsetUpdate_100GMac(devObjPtr,macPort,baseMibPtr,
                    SNET_CHT_PORT_MIB_COUNTERS_100G_PORT_ifInOctets_E,
                    descrPtr->byteCount);
        }
        else if (CHT_IS_MTI_MAC_USED_MAC(devObjPtr,macPort))
        {
            __LOG(("increment MTI counter of: Good Octets Received \n"));
            snetFalconPortStatisticCounterUpdate(devObjPtr,macPort,
                    SNET_CHT_PORT_MTI_STATISTIC_COUNTERS_RX_OctetsReceivedOK_E,
                    descrPtr->byteCount);

            /* the OctetsReceivedOK and etherStatsOctets should be the same because
               we not have 'BAD_OCTETS_RCV' in WM simulation */
            snetFalconPortStatisticCounterUpdate(devObjPtr,macPort,
                    SNET_CHT_PORT_MTI_STATISTIC_COUNTERS_RX_etherStatsOctets_E,
                    descrPtr->byteCount);

            snetFalconPortStatisticCounterUpdate(devObjPtr,macPort,
                    SNET_CHT_PORT_MTI_STATISTIC_COUNTERS_RX_aFramesReceivedOK_E,
                    1);

            snetFalconPortStatisticCounterUpdate(devObjPtr,macPort,
                    SNET_CHT_PORT_MTI_STATISTIC_COUNTERS_RX_etherStatsPkts_E,
                    1);
        }
        else if(baseMibPtr)
        {
            snetChtPortMibOffsetUpdate(devObjPtr,macPort,baseMibPtr,
                    SNET_CHT_PORT_MIB_COUNTERS_GOOD_OCTETS_RECEIVED_E,
                    descrPtr->byteCount);
        }

        if(rxHistEnable)
        {
            octets = SNET_GET_NUM_OCTETS_IN_FRAME(descrPtr->byteCount);
            mibCounter = SNET_CHT_PORT_MIB_COUNTERS____LAST____E;
            mibCounter_100G = SNET_CHT_PORT_MIB_COUNTERS_100G_PORT____LAST____E;
            mtiCounterType = SNET_CHT_PORT_MTI_STATISTIC_COUNTERS____LAST____E;
            switch (octets)
            {
                case SNET_FRAMES_1024_TO_MAX_OCTETS:

                    if(CHT_IS_100G_MAC_USED_MAC(devObjPtr,macPort))
                    {
                        mibCounter_100G = descrPtr->byteCount > 1518 ?
                            SNET_CHT_PORT_MIB_COUNTERS_100G_PORT_etherStatsPkts1519toMaxOctets_E :
                            SNET_CHT_PORT_MIB_COUNTERS_100G_PORT_etherStatsPkts1024to1518Octets_E;
                    }
                    else if (CHT_IS_MTI_MAC_USED_MAC(devObjPtr,macPort))
                    {
                        mtiCounterType = descrPtr->byteCount > 1518 ?
                            SNET_CHT_PORT_MTI_STATISTIC_COUNTERS_RX_etherStatsPkts1519toMaxOctets_E :
                            SNET_CHT_PORT_MTI_STATISTIC_COUNTERS_RX_etherStatsPkts1024to1518Octets_E;
                    }
                    else
                    {
                        mibCounter = snetChtMacCounter1024OrMoreGet(devObjPtr,
                                    descrPtr,descrPtr->byteCount,macPort);
                    }
                    break;
                case SNET_FRAMES_512_TO_1023_OCTETS:
                    __LOG(("increment counter of: 512_TO_1023_OCTETS \n"));
                    if(CHT_IS_100G_MAC_USED_MAC(devObjPtr,macPort))
                    {
                        mibCounter_100G =
                            SNET_CHT_PORT_MIB_COUNTERS_100G_PORT_etherStatsPkts512to1023Octets_E;
                    }
                    else if (CHT_IS_MTI_MAC_USED_MAC(devObjPtr,macPort))
                    {
                        mtiCounterType =
                            SNET_CHT_PORT_MTI_STATISTIC_COUNTERS_RX_etherStatsPkts512to1023Octets_E;
                    }
                    else
                    {
                        mibCounter = SNET_CHT_PORT_MIB_COUNTERS_FRAMES_512_TO_1023_OCTETS_E;
                    }
                    break;
                case SNET_FRAMES_256_TO_511_OCTETS:
                    __LOG(("increment counter of: 256_TO_511_OCTETS \n"));
                    if(CHT_IS_100G_MAC_USED_MAC(devObjPtr,macPort))
                    {
                        mibCounter_100G =
                            SNET_CHT_PORT_MIB_COUNTERS_100G_PORT_etherStatsPkts256to511Octets_E;
                    }
                    else if (CHT_IS_MTI_MAC_USED_MAC(devObjPtr,macPort))
                    {
                        mtiCounterType =
                            SNET_CHT_PORT_MTI_STATISTIC_COUNTERS_RX_etherStatsPkts256to511Octets_E;
                    }
                    else
                    {
                        mibCounter = SNET_CHT_PORT_MIB_COUNTERS_FRAMES_256_TO_511_OCTETS_E;
                    }
                    break;
                case SNET_FRAMES_128_TO_255_OCTETS:
                    __LOG(("increment counter of: 128_TO_255_OCTETS \n"));
                    if(CHT_IS_100G_MAC_USED_MAC(devObjPtr,macPort))
                    {
                        mibCounter_100G =
                            SNET_CHT_PORT_MIB_COUNTERS_100G_PORT_etherStatsPkts128to255Octets_E;
                    }
                    else if (CHT_IS_MTI_MAC_USED_MAC(devObjPtr,macPort))
                    {
                        mtiCounterType =
                            SNET_CHT_PORT_MTI_STATISTIC_COUNTERS_RX_etherStatsPkts128to255Octets_E;
                    }
                    else
                    {
                        mibCounter = SNET_CHT_PORT_MIB_COUNTERS_FRAMES_128_TO_255_OCTETS_E;
                    }
                    break;
                case SNET_FRAMES_65_TO_127_OCTETS:
                    __LOG(("increment counter of: 65_TO_127_OCTETS \n"));
                    if(CHT_IS_100G_MAC_USED_MAC(devObjPtr,macPort))
                    {
                        mibCounter_100G =
                            SNET_CHT_PORT_MIB_COUNTERS_100G_PORT_etherStatsPkts65to127Octets_E;
                    }
                    else if (CHT_IS_MTI_MAC_USED_MAC(devObjPtr,macPort))
                    {
                        mtiCounterType =
                            SNET_CHT_PORT_MTI_STATISTIC_COUNTERS_RX_etherStatsPkts65to127Octets_E;
                    }
                    else
                    {
                        mibCounter = SNET_CHT_PORT_MIB_COUNTERS_FRAMES_65_TO_127_OCTETS_E;
                    }
                    break;
                case SNET_FRAMES_64_OCTETS:
                    __LOG(("increment counter of: 64_OCTETS \n"));
                    if(CHT_IS_100G_MAC_USED_MAC(devObjPtr,macPort))
                    {
                        mibCounter_100G =
                            SNET_CHT_PORT_MIB_COUNTERS_100G_PORT_etherStatsPkts64Octets_E;
                    }
                    else if (CHT_IS_MTI_MAC_USED_MAC(devObjPtr,macPort))
                    {
                        mtiCounterType =
                            SNET_CHT_PORT_MTI_STATISTIC_COUNTERS_RX_etherStatsPkts64Octets_E;
                    }
                    else
                    {
                        mibCounter = SNET_CHT_PORT_MIB_COUNTERS_FRAMES_64_OCTETS_E;
                    }
                    break;
            }

            if(baseMibPtr && CHT_IS_100G_MAC_USED_MAC(devObjPtr,macPort))
            {
                if (mibCounter_100G < SNET_CHT_PORT_MIB_COUNTERS_100G_PORT____LAST____E)
                {
                    snetChtPortMibOffsetUpdate_100GMac(devObjPtr,macPort,baseMibPtr,
                            mibCounter_100G,
                            1);
                }
            }
            else if (CHT_IS_MTI_MAC_USED_MAC(devObjPtr,macPort))
            {
                if (mtiCounterType < SNET_CHT_PORT_MTI_STATISTIC_COUNTERS____LAST____E)
                {
                    snetFalconPortStatisticCounterUpdate(devObjPtr,macPort,
                            mtiCounterType,
                            1);
                }
            }
            else if(baseMibPtr)
            {
                if (mibCounter < SNET_CHT_PORT_MIB_COUNTERS____LAST____E)
                {
                    snetChtPortMibOffsetUpdate(devObjPtr,macPort,baseMibPtr,
                            mibCounter,
                            1);
                }
            }
        }
        else
        {
            __LOG(("WARNING : RX histogram counters (disabled) are not changed by the traffic , on MAC port [%d] \n",
                macPort));
        }
    }
}


/**
* @internal snetXcatGetGreEtherTypes function
* @endinternal
*
* @brief   check if GRE over IPv4/6, and get the 'GRE ethertypes'
*
* @param[in] devObjPtr                - pointer to device object
* @param[in] descrPtr                 - pointer to frame's descriptor
*
* @param[out] greEtherTypePtr          - gre etherType, relevant if function returns GT_TRUE(can be NULL)
* @param[out] gre0EtherTypePtr         - gre 0 etherType, GRE protocols that are recognized as Ethernet-over-GRE(can be NULL)
* @param[out] gre1EtherTypePtr         - gre 1 etherType, GRE protocols that are recognized as Ethernet-over-GRE(can be NULL)
*                                      RETURN:
*                                      GT_BOOL      - is gre or not
*                                      COMMENTS:
*/
GT_BOOL snetXcatGetGreEtherTypes
(
    IN    SKERNEL_DEVICE_OBJECT             *devObjPtr,
    IN    SKERNEL_FRAME_CHEETAH_DESCR_STC   *descrPtr,
    OUT   GT_U32                            *greEtherTypePtr,
    OUT   GT_U32                            *gre0EtherTypePtr,
    OUT   GT_U32                            *gre1EtherTypePtr
)
{
    DECLARE_FUNC_NAME(snetXcatGetGreEtherTypes);

    GT_U32  regAddress;   /* register addres  */
    GT_U32 *memoryPtr;    /* pointer to memory */
    GT_U32  greEtherType, gre0EtherType, gre1EtherType;
    GT_BOOL  isGre = GT_FALSE;
    GT_U32 globalConfigExt;
    GT_BIT  greHeaderFirst2BytesNonZero;

    if(greEtherTypePtr)
    {
        *greEtherTypePtr = 0;
    }
    if(gre0EtherTypePtr)
    {
        *gre0EtherTypePtr = 0;
    }
    if(gre1EtherTypePtr)
    {
        *gre1EtherTypePtr = 0;
    }


    descrPtr->greHeaderSize = 0;
    descrPtr->greHeaderError = 0;

    if(NULL == descrPtr->l4StartOffsetPtr)
    {
        /* when l4StartOffsetPtr == NULL the function must treat it as 'NOT GRE',
           because l4StartOffsetPtr == NULL when 'recognized as fragment'
           and there is no protocol after the ip header,  only payload. */
        __LOG(("l4StartOffsetPtr == NULL  --> can not recognize GRE \n"));
        return GT_FALSE;
    }


    if(descrPtr->isIPv4)
    {
        regAddress = SMEM_XCAT_TTI_IPV4_GRE_ETHERTYPE_REG(devObjPtr);
    }
    else  /* ipv6 */
    {
        if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
        {
            regAddress = SMEM_LION3_TTI_IPV6_GRE_ETHERTYPE_REG(devObjPtr);
        }
        else
        {
            __LOG(("ipv6 GRE recognition not supported for the device \n"));
            return GT_FALSE;
        }
    }

    /* Get gre etherType */
    greEtherType = (descrPtr->l4StartOffsetPtr[2] << 8) |
                    descrPtr->l4StartOffsetPtr[3];

    if(greEtherTypePtr)
    {
        *greEtherTypePtr = greEtherType;
    }

    if(/*descrPtr->isIPv4 && */greEtherType == SKERNEL_L3_PROT_TYPE_IPV4_E)
    { /* ipv4-O-GRE */
        __LOG(("greEtherType [0x%4.4x] is recognized as IPv4-over-GRE \n",
            greEtherType));
        isGre = GT_TRUE;
    }
    else if(/*(descrPtr->isIPv4 == 0) && */greEtherType == SKERNEL_L3_PROT_TYPE_IPV6_E)
    { /* ipv6-O-GRE */
        __LOG(("greEtherType [0x%4.4x] is recognized as IPv6-over-GRE \n",
            greEtherType));
        isGre = GT_TRUE;
    }
    else /* ethernet-O-GRE */
    {
        memoryPtr = smemMemGet(devObjPtr, regAddress);

        /* set gre0EtherType */
        gre0EtherType = SMEM_U32_GET_FIELD(*memoryPtr, 0,  16);

        /* set gre1EtherType */
        gre1EtherType = SMEM_U32_GET_FIELD(*memoryPtr, 16, 16);
        __LOG_PARAM(gre0EtherType);
        __LOG_PARAM(gre1EtherType);

        if(gre0EtherType == 0 && gre1EtherType == 0)
        {
            /* support simulation of legacy devices that not initialized this register */
            isGre = GT_FALSE;
        }
        else
        {
            if(gre0EtherTypePtr)
            {
                *gre0EtherTypePtr = gre0EtherType;
            }

            if(gre1EtherTypePtr)
            {
                *gre1EtherTypePtr = gre1EtherType;
            }

            isGre = ((greEtherType != gre0EtherType) &&
                     (greEtherType != gre1EtherType)) ? GT_FALSE : GT_TRUE;
        }

        __LOG(("greEtherType [0x%4.4x] is %s recognized as Ethernet-over-GRE",
            greEtherType,((isGre == GT_TRUE) ? "" : "not")));
    }


    if(isGre == GT_TRUE)
    {
        greHeaderFirst2BytesNonZero =
            (descrPtr->l4StartOffsetPtr[0] ||
             descrPtr->l4StartOffsetPtr[1]) ? 1 : 0;

        if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
        {
            descrPtr->greHeaderSize = 4; /* Basic size of GRE header */
            descrPtr->greHeaderSize += ((descrPtr->l4StartOffsetPtr[0] >> 7)&0x1) * 4; /* Checksum bit*/
            descrPtr->greHeaderSize += ((descrPtr->l4StartOffsetPtr[0] >> 5)&0x1) * 4; /* Key bit*/
            descrPtr->greHeaderSize += ((descrPtr->l4StartOffsetPtr[0] >> 4)&0x1) * 4; /* Sequence Number Present bit*/
        }
        else
        {
            descrPtr->greHeaderSize = 4;
        }

        if(greHeaderFirst2BytesNonZero)/* GRE Extensions and version 0 check */
        {
            if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
            {
                smemRegGet(devObjPtr,SMEM_LION3_TTI_UNIT_GLOBAL_CONFIG_EXT_REG(devObjPtr), &globalConfigExt);
                if(0 == SMEM_U32_GET_FIELD(globalConfigExt,25,1))/*Enable GRE Extensions*/
                {
                    descrPtr->greHeaderError = 1;
                }
            }
            else
            {
                /* for legacy no EXTENTION support */
                descrPtr->greHeaderError = 1;
            }
        }

        __LOG_PARAM(descrPtr->greHeaderSize);
        __LOG_PARAM(descrPtr->greHeaderError);
    }

    return isGre;
}

/**
* @internal snetLion3IpTotalLengthAfterDeduction function
* @endinternal
*
* @brief   ipv4/6 total length after deduction
*/
static GT_U32 snetLion3IpTotalLengthAfterDeduction
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32 origLength
)
{
    DECLARE_FUNC_NAME(snetLion3IpTotalLengthAfterDeduction);

    GT_U32  regAddr, fldValue, bitNum;

    if(SMEM_CHT_IS_SIP5_GET(devObjPtr) == 0)
    {
        __LOG(("No deduction of ipv4/6 total length for this device type \n"));
        return origLength;
    }

    if(descrPtr->tunnelTerminated == 1)
    {
        __LOG(("No deduction of ipv4/6 total length for tunnelTerminated == 1 \n"));
        return origLength;
    }


    if (GT_FALSE == snetXcatGetGreEtherTypes(devObjPtr, descrPtr, NULL, NULL, NULL))
    {
        __LOG(("deduction not relevant because --> not GRE"));
        return origLength;
    }

    /*For MACSEC packets over IPv4/6 tunnel, that are to be tunnel
      terminated, this configuration enables aligning the IPv4 total header
      length to the correct offset (taking into account the additional
      MACSEC header).*/

    /*IPv4/6 Total Length Deduction Enable*/
    if(0 == SMEM_LION3_TTI_DEFAULT_EPORT_ENTRY_FIELD_GET(devObjPtr, descrPtr,
                SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_IPV4_6_TOTAL_LENGTH_DEDUCTION_EN))
    {
        __LOG(("deduction not enabled on default eport"));
        return origLength;
    }

    regAddr = SMEM_LION3_TTI_IP_LENGTH_CHECK_REG(devObjPtr);
    bitNum  = descrPtr->isIPv4 ? 0 : 6;

    smemRegFldGet(devObjPtr, regAddr, bitNum, 6,  &fldValue);

    __LOG(("IPv4/6 Total Length Deduction Enabled, with length[%d]", fldValue));
    return (origLength - fldValue);
}

/**
* @internal snetChtMplsTransitTunnelsProtParsing function
* @endinternal
*
* @brief   Parsing of transit mpls tunnels
*/
GT_VOID snetChtMplsTransitTunnelsProtParsing
(
    IN SKERNEL_DEVICE_OBJECT           *devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC *descrPtr,
    IN LION3_MPLS_TRANSIT_TUNNEL_PARSING_MODE_ENT    pmode,
    IN INTERNAL_TTI_DESC_INFO_STC      *internalTtiInfoPtr
)
{
    DECLARE_FUNC_NAME(snetChtMplsTransitTunnelsProtParsing);

    GT_U32   etherType = 0;
    GT_BIT   l2Parsing;/* indication that we need L2 parsing (beside the L3L4)
                            to support Ethernet parsing */
    SKERNEL_FRAME_CHEETAH_DESCR_STC *innerFrameDescrPtr;/* pointer to the passenger descriptor */
    GT_U32   tunnelHdrSize;

    __LOG(("parsing mode: %s \n",
                    pmode == LION3_MPLS_TRANSIT_TUNNEL_PARSING_MODE_IP_ONLY_E ? "IP_ONLY" :
                    pmode == LION3_MPLS_TRANSIT_TUNNEL_PARSING_MODE_IP_OR_CW_PW_ETHERNET_E ? "IP_OR_CW_PW_ETHERNET" :
                    pmode == LION3_MPLS_TRANSIT_TUNNEL_PARSING_MODE_ETHERNET_NO_CW_PW_E ? "ETHERNET_NO_CW_PW" :
                    "unknown"
                    ));

    if(pmode != LION3_MPLS_TRANSIT_TUNNEL_PARSING_MODE_ETHERNET_NO_CW_PW_E)
    {
        l2Parsing = 0;

        __LOG_PARAM(descrPtr->protOverMpls);

        switch(descrPtr->protOverMpls)
        {
            case 0:
                __LOG(("parse passenger as Ipv4 \n"));
                etherType = SKERNEL_L3_PROT_TYPE_IPV4_E; /*Ipv4*/
                break;
            case 1:
                __LOG(("parse passenger as Ipv6 \n"));
                etherType = SKERNEL_L3_PROT_TYPE_IPV6_E; /*Ipv6*/
                break;
            default:
            case 2:
                if(pmode == LION3_MPLS_TRANSIT_TUNNEL_PARSING_MODE_IP_ONLY_E)
                {
                    /* not recognized option : do not parse passenger */
                    /* not support non IP passenger */
                    __LOG(("WARNING : the 'IP_ONLY' mode NOT parse Ethernet (no passenger parsing) \n"));
                    return;
                }
                __LOG(("parse passenger as Ethernet \n"));
                l2Parsing = 1;
                break;
        }
    }
    else /*LION3_MPLS_TRANSIT_TUNNEL_PARSING_MODE_ETHERNET_NO_CW_PW_E*/
    {
        __LOG(("parse passenger as Ethernet (no CW_PW) \n"));
        l2Parsing = 1;
    }

    descrPtr->ingressTunnelInfo.innerFrameDescrPtr = snetChtEqDuplicateDescr(devObjPtr, descrPtr);
    innerFrameDescrPtr = descrPtr->ingressTunnelInfo.innerFrameDescrPtr;

    __LOG(("jump the MPLS labels (before parsing the passenger) \n"));

    /* jump the MPLS labels */
    innerFrameDescrPtr->l3StartOffsetPtr =  descrPtr->afterMplsLabelsPtr;

    if(l2Parsing)
    {
        if(LION3_MPLS_TRANSIT_TUNNEL_PARSING_MODE_IP_OR_CW_PW_ETHERNET_E == pmode)
        {
            __LOG(("jump 4 byte of CW (Control Word) (before parsing the passenger) \n"));
            /* jump 4 byte of CW (Control Word) */
            innerFrameDescrPtr->l3StartOffsetPtr += 4;
        }

        __LOG(("Do L2 passenger parsing (Ethernet parsing) \n"));

        innerFrameDescrPtr->byteCount -= (innerFrameDescrPtr->l3StartOffsetPtr - innerFrameDescrPtr->startFramePtr);
        __LOG_PARAM(innerFrameDescrPtr->byteCount);

        innerFrameDescrPtr->startFramePtr = innerFrameDescrPtr->l3StartOffsetPtr;
        /* reset the L3 pointer , because it is not relevant any more -->
                   will be set inside snetChtL2Parsing(...) for the 'passenger'  */
        innerFrameDescrPtr->l3StartOffsetPtr = NULL;
        innerFrameDescrPtr->macDaPtr = DST_MAC_FROM_TUNNEL_DSCR(innerFrameDescrPtr);
        innerFrameDescrPtr->macSaPtr = SRC_MAC_FROM_TUNNEL_DSCR(innerFrameDescrPtr);

        /* do re-parsing on the passenger , to set the L2 info
                   (must be done prior to setting the VID from the TTI action ,
                   and other QOS setting of TTI) */
        snetChtL2Parsing(devObjPtr, innerFrameDescrPtr,
                         SNET_CHT_FRAME_PARSE_MODE_FROM_TTI_PASSENGER_E, internalTtiInfoPtr);
        innerFrameDescrPtr->passengerLength = innerFrameDescrPtr->byteCount -
                                (innerFrameDescrPtr->l3StartOffsetPtr - innerFrameDescrPtr->startFramePtr);

        __LOG_PARAM(innerFrameDescrPtr->passengerLength);

        /* now after the L2 parsing we know the ethertype for the L3L4 parsing */
        etherType = innerFrameDescrPtr->etherTypeOrSsapDsap;
    }

    /* reset the descriptor regarding fields of L3,L4 , so that fields that
       not set by function snetChtL3L4ProtParsing , will not stay with
       values from previous call to snetChtL3L4ProtParsing */
    snetChtL3L4ProtParsingResetDesc(devObjPtr, innerFrameDescrPtr);

    __LOG(("Do L3L4 passenger parsing according to etherType[0x%4.4x] \n",
                    etherType));
    /* L3, L4 protocols parsing */
    snetChtL3L4ProtParsing(devObjPtr, innerFrameDescrPtr, etherType, internalTtiInfoPtr);

    if ((descrPtr->origByteCount & 0x3F00) == 0x3F00)
    {
        /* Case of cut through - rxDma set descrPtr->origByteCount default value 0x3FFF */
        /* Passenger packet parser could decrease it by passenger origin offset         */
        /* by calculation based on original value 0x3FFF                                */
        descrPtr->origByteCount = innerFrameDescrPtr->origByteCount;
        descrPtr->byteCount     = innerFrameDescrPtr->byteCount;
        descrPtr->payloadLength = innerFrameDescrPtr->payloadLength;
        if (l2Parsing)
        {
            tunnelHdrSize = innerFrameDescrPtr->macDaPtr - descrPtr->macDaPtr;
            descrPtr->origByteCount += tunnelHdrSize;
            descrPtr->byteCount     += tunnelHdrSize;
        }
    }

    /*NOTE: the trunk hashing (snetXCatA1TrunkHash(...)) will be according to
        the descrPtr->mplsInfo.l3l4DescrPtr */

    /* the function snetXCatA1TrunkHash(...) updated the descrPtr->mplsInfo.l3l4DescrPtr->pktHash
    and we need it in the original descriptor too. */
    descrPtr->pktHash = innerFrameDescrPtr->pktHash;

    __LOG(("update the pktHash in the original descriptor [0x%8.8x]\n",
                    descrPtr->pktHash));
}

/**
* @internal snetLion3TTMplsOamChannelTypeProfile function
* @endinternal
*
* @brief   TT MPLS OAM Channel Type Profile support
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] descrPtr                 - pointer to the frame descriptor.
* @param[in] firstNibbleAfterSBitLabel - first nibble after sbit label
* @param[in] pwe3CwWord               - pseudo wire control word
*
*/
static GT_VOID snetLion3TTMplsOamChannelTypeProfile
(
    IN    SKERNEL_DEVICE_OBJECT             *devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC   *descrPtr,
    IN    GT_U32                             firstNibbleAfterSBitLabel,
    IN    GT_U32                             pwe3CwWord
)
{
    DECLARE_FUNC_NAME(snetLion3TTMplsOamChannelTypeProfile);

    GT_U32 pwe3ChannelType = pwe3CwWord >> 16;
    GT_U32 tableChannelType;
    GT_U32 index;
    GT_U32 validBit;

   /*
        PWE3_Channel_Type = 2 LSBs of 4B word following the MPLS header
                (PWE3 CW Control Word Format)
        IF (First nibble following the MPLS header = 4:b0001) // PWE3 word exists

            TTI-Key[Channel Type Profile] = 0 // default, no match

            FOR EACH entry N in the MPLS OAM Channel Type Profile Table)

                IF ((N[Channel Type] = PWE3_Channel_Type) & (N[Valid] = 1))
                    TTI-Key[Channel Type Profile] = N+1 // Entry 0 is profile 1 etc.
            END FOR
   */

    /* Figure 356: Associated Channel Header (ACH)
         0                             1                             2                             3
         0  1  2  3  4  5  6  7  8  9  0  1  2  3  4  5  6  7  8  9  0  1  2  3  4  5  6  7  8  9  0  1
        -----------------------------------------------------------------------------------------------
        |0  0  0  1 |Version     |       Reserved        |       Channel  Type                        |
        ----------------------------------------------------------------------------------------------
    */

    /* set default value */
    descrPtr->channelTypeProfile = 0;

    if(firstNibbleAfterSBitLabel != 1)
    {
        return;
    }

    for(index = 0; index < 15; index++)
    {
        /* get table channel type */
        smemRegFldGet(devObjPtr, SMEM_LION3_TTI_MPLS_OAM_CHANNEL_TYPE_PROFILE_REG(devObjPtr, index), 1, 16, &tableChannelType);

        if(tableChannelType != pwe3ChannelType)
        {
            continue;
        }

        /* check valid bit first */
        smemRegFldGet(devObjPtr,SMEM_LION3_TTI_MPLS_OAM_CHANNEL_TYPE_PROFILE_REG(devObjPtr, index), 0, 1, &validBit);

        if(validBit)
        {
            /* got match  and valid bit is set */
            descrPtr->channelTypeProfile = index + 1;
            __LOG(("channelTypeProfile: %d", descrPtr->channelTypeProfile));

            /* only one match allowed */
            break;
        }
    }
}


/**
* @internal parseMpls function
* @endinternal
*
* @brief   Parsing of mpls protocol header
*
* @retval GT_BOOL                  - indicates that parsing is done or not
*/
static GT_BOOL parseMpls
(
    IN SKERNEL_DEVICE_OBJECT           *devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC *descrPtr,
    IN GT_U32                           etherType,
    IN INTERNAL_TTI_DESC_INFO_STC      *internalTtiInfoPtr
)
{
    DECLARE_FUNC_NAME(parseMpls);

    GT_U32 regAddress;          /* register address */
    GT_U32 fldValue;            /* register's field address */
    GT_U8  sbit;                /* indication of end of label            */
    GT_U32 firstNibbleAfterMPLS;/* first nibble after MPLS header */
    GT_BIT isMpls = 0;          /* bit to identify if this is MPLS unicast */
    GT_U32 byteOffset;          /* byte offset in the packet */
    GT_U32 mplsIndex,mplsIndexMax;/* mpls index iterator , and mpls index max iterator*/
    GT_U8  ttlArr[10] = {0};      /* TTL from MPLS labels */
    GT_U32 labelArr[10];         /* label from MPLS labels */
    GT_U32 expArr[10];           /* EXP from MPLS labels */
    GT_U32 index;
    GT_U32 maxMplsLabelsForHash;
    GT_U32 mplsStackIndex;
    GT_BOOL labelExceeded = GT_FALSE;

    __LOG(("Read the Ethertype used to identify MPLS packets"));
    regAddress = SMEM_XCAT_TTI_MPLS_ETHERTYPES_REG(devObjPtr);
    smemRegFldGet(devObjPtr, regAddress, 0, 16, &fldValue);
    if (etherType == fldValue)
    {
        isMpls = 1;
    }

    if(!isMpls && SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        smemRegFldGet(devObjPtr, regAddress, 16, 16, &fldValue);
        if (etherType == fldValue)
        {
            isMpls = 1;
        }
    }

    __LOG_PARAM(isMpls);

    if (isMpls == 1)
    {
        __LOG(("this is MPLS\n"));

        descrPtr->isIp = 0;
        descrPtr->isIPv4 = 0;
        descrPtr->mpls = 1;

        __LOG_PARAM(descrPtr->isIp);
        __LOG_PARAM(descrPtr->isIPv4);



        /*save the outer label for PCL key*/
        descrPtr->origInfoBeforeTunnelTermination.origMplsOuterLabel =
                descrPtr->l3StartOffsetPtr[0] << 24 |
                descrPtr->l3StartOffsetPtr[1] << 16 |
                descrPtr->l3StartOffsetPtr[2] <<  8 |
                descrPtr->l3StartOffsetPtr[3] <<  0;

        __LOG_PARAM(descrPtr->origInfoBeforeTunnelTermination.origMplsOuterLabel);

        /* Max number of MPLS labels to parse
         * SIP7: 10 lables
         * SIP5 and above: 6 labels
         * Legacy: 4 labels*/
        mplsIndexMax = SMEM_CHT_IS_SIP5_GET(devObjPtr) ?
                       ((SMEM_CHT_IS_SIP7_GET(devObjPtr))? 9: 5):
                       3;
        __LOG_PARAM(mplsIndexMax);

        byteOffset = 0;
        descrPtr->numOfLabels = 0;
        for(mplsIndex = 0 ; mplsIndex <= mplsIndexMax ; mplsIndex++,byteOffset+=4)
        {
            labelArr[mplsIndex] = (GT_U32)(descrPtr->l3StartOffsetPtr[byteOffset+0] ) <<  12 |
                                  (GT_U32)(descrPtr->l3StartOffsetPtr[byteOffset+1] ) <<  4 |
                                  (GT_U32)(((descrPtr->l3StartOffsetPtr[byteOffset+2] ) & 0xf0 ) >> 4)  ;

            expArr[mplsIndex]   = (GT_U32)((descrPtr->l3StartOffsetPtr[byteOffset+2] & 0x0e) >> 1);
            sbit                = (GT_U8)(descrPtr->l3StartOffsetPtr[byteOffset+2] & 0x1);
            ttlArr[mplsIndex]   = (GT_U8)(descrPtr->l3StartOffsetPtr[byteOffset+3]);

            if(sbit == 1)
            {
                /* no more labels */
                break;
            }

            if ((mplsIndex == mplsIndexMax) &&
                (sbit == 0))
            {
                __LOG(("ERROR : Sbit was not set on the last label and the Number of Labels exceeded maximum limit [%u]\n",
                       mplsIndexMax));
                labelExceeded = GT_TRUE;
                __LOG_PARAM(labelExceeded);
            }

            if(mplsIndex != mplsIndexMax)
            {
                descrPtr->numOfLabels++; /* value 0 means 1 label
                                            value 1 means 2 labels
                                            value 2 means 3 labels
                                            value 3 means 4 labels (non sip5 --> or more than 4)
                                            value 4 means 5 labels (sip5 --> or more than 5) */
            }
        }

        descrPtr->mplsReservedLabelExist = 0;
        for (mplsIndex = 0 ; mplsIndex <= descrPtr->numOfLabels ; mplsIndex++)
        {
            /* RFC 3032 defines the MPLS label values in the range 0-15 as a reserved label range */
            if (labelArr[mplsIndex] <= 15)
            {
                descrPtr->mplsReservedLabelExist = 1;

                if(SMEM_CHT_IS_SIP5_15_GET(devObjPtr) &&
                    labelArr[mplsIndex] == 0xf)
                {
                    mplsIndex++;/* jump to the ESPL and use it's value */

                    __LOG(("the ESPL value [0x%x] considered as the reserved label value (for TTI metadata <MPLS Reserved Label Value>)\n" ,
                        labelArr[mplsIndex]));
                }
                else
                {
                    __LOG(("the Reserved value [0x%x] considered as the reserved label value (for TTI metadata <MPLS Reserved Label Value>)\n" ,
                        labelArr[mplsIndex]));
                }

                /* SIP7 devices */
                if(SMEM_CHT_IS_SIP7_GET(devObjPtr))
                {
                    /* Check for Entropy Label Indicator(ELI) label(0x7) */
                    if (labelArr[mplsIndex] == 0x7)
                    {
                        if (mplsIndex == descrPtr->numOfLabels)
                        {
                            __LOG(("BoS is found on the ELI Label. This is an Exception\n"));
                            /* TBD: Exception Code */
                        }
                    }
                }

                /* This field contains the value of the outermost MPLS Reserved label found in the
                   outermost 4 labels in label stack*/
                descrPtr->mplsReservedLabelValue = labelArr[mplsIndex];

                break;
            }
        }
        __LOG_PARAM(descrPtr->mplsReservedLabelExist);
        __LOG_PARAM(descrPtr->mplsReservedLabelValue);

        __LOG_PARAM(descrPtr->numOfLabels);

        firstNibbleAfterMPLS = descrPtr->l3StartOffsetPtr[byteOffset+4] >> 4;
        __LOG_PARAM(firstNibbleAfterMPLS);

          /* 5 bits that immediately follow the bottom of label stack */
        descrPtr->mplsPayloadData = descrPtr->l3StartOffsetPtr[byteOffset+4] >> 3;
        __LOG_PARAM(descrPtr->mplsPayloadData);

        /* pointer to payload data after MPLS lables */
        descrPtr->afterMplsLabelsPtr = &descrPtr->l3StartOffsetPtr[byteOffset+4];

        descrPtr->ttl = ttlArr[0];

        mplsIndex = 0; /* Keeps track of forwarding labels */
        for (mplsStackIndex = 0; mplsStackIndex <= descrPtr->numOfLabels; mplsStackIndex++)
        {
            /* Copy the label, EXP and TTL info to descriptor */
            descrPtr->label[mplsStackIndex]  = labelArr[mplsStackIndex];
            descrPtr->exp[mplsStackIndex]    = expArr[mplsStackIndex];
            descrPtr->ttlArr[mplsStackIndex] = ttlArr[mplsStackIndex];

            __LOG(("MPLS label [%d] recognized: label[0x%x] exp[%d] ttl[0x%x] \n",
                mplsStackIndex,
                labelArr[mplsStackIndex],
                expArr[mplsStackIndex],
                ttlArr[mplsStackIndex]));

            if(SMEM_CHT_IS_SIP7_GET(devObjPtr))
            {
                /* Ignore Non-Forwarding Labels
                 * Entropy Label Indicator(ELI) label(0x7)
                 * XL+ESPL label(0xF)
                 */
                if ((labelArr[mplsStackIndex] == 0x7) || (labelArr[mplsStackIndex] == 0xf))
                {
                    __LOG(("ELI label recognized at index [%u]: label[0x%x]\n", mplsStackIndex, labelArr[mplsStackIndex]));
                }
                /* Ignore Explicit NULL label(0x0, 0x2)
                 * If found in BoS then consider it as forwarding label
                 */
                else if ((labelArr[mplsStackIndex] == 0x0) || (labelArr[mplsStackIndex] == 0x2))
                {
                    if (mplsStackIndex != descrPtr->numOfLabels)
                    {
                        __LOG(("Explicit NULL label recognized at index [%u]: label[0x%x]\n", mplsStackIndex, labelArr[mplsStackIndex]));
                        continue;
                    }
                    else
                    {
                        __LOG(("Explicit NULL label recognized at BoS with index [%u]: label[0x%x]. Hence this is a forwarding label\n",
                               mplsStackIndex, labelArr[mplsStackIndex]));
                    }
                }
            } /* end of SIP7 check */

            /* Updates the Forwarding Label index in the stack and its Valid bit */
            if(SMEM_CHT_IS_SIP7_GET(devObjPtr))
            {
                descrPtr->isLabelValid[mplsIndex]    = GT_TRUE;
                descrPtr->fwdLabelStackIndex[mplsIndex] = mplsStackIndex;
                __LOG(("MPLS Forwarding label [%d] at fwdLabelStackIndex[%u] = %u\n", mplsIndex, mplsIndex, mplsStackIndex));
            }

            /* Increments only for forwarding labels */
            mplsIndex++;
        }

        /* If the incoming packet has more than maximum allowed labels
         * then it needs to be handled as per the Exception configuration */
        if(labelExceeded)
        {
            __LOG(("Setting L3 Not valid Flag\n"));
            /* Setting the Flag which will be used to raise exception in snetXcatTTMplsHeaderCheck() */
            descrPtr->l3NotValid = 1;
        }

        if(SMEM_CHT_IS_SIP7_GET(devObjPtr))
        {
            __LOG(("Current Forwarding Label Index [%u]\n", mplsIndex));
            descrPtr->isLastLabelInStack[mplsStackIndex-1] = GT_TRUE;
            __LOG(("MPLS label [0x%x] is the last label in the label stack, stackIndex[%u]\n",
                   labelArr[mplsStackIndex-1], mplsStackIndex-1));
        }

        __LOG(("firstNibbleAfterMPLS [0x%x] \n",
            firstNibbleAfterMPLS));

        switch (firstNibbleAfterMPLS)
        {
            case 4:
                descrPtr->protOverMpls = 0; /* protocol over MPLS is IPv4 */
                break;
            case 6:
                descrPtr->protOverMpls = 1; /* protocol over MPLS is IPv6 */
                break;
            default:
                descrPtr->protOverMpls = 2; /* protocol over MPLS is Other */
                break;
        }

        descrPtr->cwFirstNibble = firstNibbleAfterMPLS;

        /* Maximum MPLS lables used for Hash calculation
         * Legacy: 4; SIP7: 10
         */
        if (SMEM_CHT_IS_SIP7_GET(devObjPtr))
        {
            maxMplsLabelsForHash = 10;
        }
        else
        {
            maxMplsLabelsForHash = 4;
        }

        /* Update the Label Value for Hash */
        for (index = 0; index < maxMplsLabelsForHash; index++)
        {
            descrPtr->mplsLabelValueForHash[index] = descrPtr->label[index];
        }

        if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
        {
            regAddress = SMEM_LION3_TTI_UNIT_GLOBAL_CONFIG_EXT_REG(devObjPtr);
            smemRegFldGet(devObjPtr, regAddress, 27, 1, &fldValue);
            descrPtr->enableL3L4ParsingOverMpls = fldValue;

            if(SMEM_CHT_IS_SIP5_15_GET(devObjPtr))
            {
                /* <Exclude Reserved MPLS Labels from Hash Key> */
                smemRegFldGet(devObjPtr, regAddress, 29, 1, &fldValue);

                /* The MPLS label with 'Reserved value' ignored in hashing (RFC 7274) */
                if(fldValue == 1)
                {
                    __LOG(("<Exclude Reserved MPLS Labels from Hash Key> = 1 \n"));
                    /* we consider 0 value of MPLS reserved values */
                    for (mplsIndex = 0 ; mplsIndex <= descrPtr->numOfLabels ; mplsIndex++)
                    {
                        if(descrPtr->mplsLabelValueForHash[mplsIndex] <= 0xf)
                        {
                            fldValue = descrPtr->mplsLabelValueForHash[mplsIndex];

                            /* ignore from hash the reserved label */
                            descrPtr->mplsLabelValueForHash[mplsIndex] = 0;

                            if(fldValue == 0xf)
                            {
                                __LOG(("The MPLS label index [%d] with value [0x%x] considered 'Extended Lable' , so ignored in hashing (RFC 7274)\n"
                                       "ignoring also the 'next label' -->  Extended Special Purpose Label (ESPL) from the hashing \n",
                                    mplsIndex,
                                    fldValue));

                                /* ignore from hash also the Extended Special Purpose Label (ESPL) */
                                mplsIndex++;
                                descrPtr->mplsLabelValueForHash[mplsIndex] = 0;
                            }
                            else
                            {
                                __LOG(("The MPLS label index [%d] with value [0x%x] considered 'reserved value' , so ignored in hashing (RFC 7274)\n",
                                    mplsIndex,
                                    fldValue));
                            }
                        }
                    }
                }
            }

        }
        else
        {
            /* Get Enable L3 L4 Over MPLS Parsing */
            regAddress = SMEM_XCAT_A1_TTI_UNIT_GLB_CONF_REG(devObjPtr);
            smemRegFldGet(devObjPtr, regAddress, 23, 1, &fldValue);
            descrPtr->enableL3L4ParsingOverMpls = fldValue;
        }

        __LOG(("Enable L3 L4 Over MPLS Parsing: %d \n",
              descrPtr->enableL3L4ParsingOverMpls));

        if(descrPtr->enableL3L4ParsingOverMpls &&
            (descrPtr->protOverMpls == 0 || /*Ipv4*/
             descrPtr->protOverMpls == 1))  /*Ipv6*/
        {
            descrPtr->ingressTunnelInfo.transitType = SNET_INGRESS_TUNNEL_TRANSIT_TYPE_LEGACY_IP_OVER_MPLS_TUNNEL_E;
            __LOG(("transitType = SNET_INGRESS_TUNNEL_TRANSIT_TYPE_LEGACY_IP_OVER_MPLS_TUNNEL_E \n"));

            __LOG(("This packet considered as Ipv4/6 over MPLS transit tunnel (if not tunnel terminated in TTI) \n"));
            snetChtMplsTransitTunnelsProtParsing(devObjPtr, descrPtr,
                LION3_MPLS_TRANSIT_TUNNEL_PARSING_MODE_IP_ONLY_E, internalTtiInfoPtr);

            __LOG(("parsing on passenger is done \n"));
            return GT_TRUE; /* indicates that mpls parsing is done */
        }


        if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
        {
            /* the offset from the L3 of the packet of the first byte that come after the recognized MPLS labels */
            GT_U32  offset = 4*(descrPtr->numOfLabels +1);
            __LOG_PARAM(offset);

            /* build the first word after the recognized MPLS labels */
            internalTtiInfoPtr->pwe3CwWord = SNET_BUILD_WORD_FROM_BYTES_PTR_MAC(&descrPtr->l3StartOffsetPtr[offset]);
            __LOG_PARAM(internalTtiInfoPtr->pwe3CwWord);

            /* get the first nibble (in network order) that come after the recognized MPLS labels */
            internalTtiInfoPtr->firstNibbleAfterSBitLabel = SMEM_U32_GET_FIELD(internalTtiInfoPtr->pwe3CwWord, 28, 4); ;
            __LOG_PARAM(internalTtiInfoPtr->firstNibbleAfterSBitLabel);


            snetLion3TTMplsOamChannelTypeProfile(devObjPtr, descrPtr,
                                                 internalTtiInfoPtr->firstNibbleAfterSBitLabel,
                                                 internalTtiInfoPtr->pwe3CwWord);
        }
    }

    __LOG(("mpls parsing isn't done \n"));
    return GT_FALSE; /* indicates that mpls parsing isn't done */
}

/**
* @internal snetChtL3L4ProtParsingArp function
* @endinternal
*
* @brief   Parsing of L3 and L4 protocols header (arp)
*
* @param[in,out] descrPtr                 - pointer to the frame's descriptor.
*/
static GT_VOID snetChtL3L4ProtParsingArp
(
    IN    SKERNEL_DEVICE_OBJECT           *devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC *descrPtr
)
{
    DECLARE_FUNC_NAME(snetChtL3L4ProtParsingArp);

    GT_U32 startByte;                /* start Byte for ip addresses */

    descrPtr->arp = 1;
    __LOG_PARAM(descrPtr->arp);

    if (descrPtr->macDaType == SKERNEL_BROADCAST_MAC_E)
    {
        __LOG(("macDaType == SKERNEL_BROADCAST_ARP_E"));
        descrPtr->macDaType = SKERNEL_BROADCAST_ARP_E;
    }

    /* SIP */
    startByte = 14;
    descrPtr->sip[0] = SNET_BUILD_WORD_FROM_BYTES_PTR_MAC(&descrPtr->l3StartOffsetPtr[startByte]);

    startByte = 24;
    descrPtr->dip[0] = SNET_BUILD_WORD_FROM_BYTES_PTR_MAC(&descrPtr->l3StartOffsetPtr[startByte]);

    __LOG_PARAM(descrPtr->sip[0]);
    __LOG_PARAM(descrPtr->dip[0]);

}


/**
* @internal checkIpHeader function
* @endinternal
*
* @brief   Checks ip header (it performes single check)
*
* @param[in,out] descrPtr                 - pointer to the frame's descriptor.
*/
static GT_VOID checkIpHeader
(
    IN    SKERNEL_DEVICE_OBJECT               *devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC     *descrPtr,
    IN    SNET_PRE_ROUT_HEADER_EXCEPTION_CHECK_ENT  maskId,
    IN    GT_CHAR                             *headerErrorDescriptionPtr
)
{
    DECLARE_FUNC_NAME(checkIpHeader);

    GT_U32 maskBit = maskId;
    GT_U32 isMaskEnabled = 0;

    if(descrPtr->ipHeaderError)
    {
        /* 1. support 'priority' of errors
           2. do not allow to set descrPtr->ipHeaderError = 0 after previous 'error' already set it.
        */
        return;
    }

    /* set default value */
    descrPtr->ipHeaderError = 1;
    descrPtr->ipHeaderErrorMask = SMAIN_NOT_VALID_CNS;

    if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
    { /* check pre-route exception mask */

        /* multicast mask bit = unicast mask bit + 1 */
        if(descrPtr->ipm)
        {
            maskBit++;
        }

        smemRegFldGet(devObjPtr, SMEM_LION3_TTI_PRE_ROUTE_EXCEPTION_MASKS_REG(devObjPtr),
                        maskBit, 1, &isMaskEnabled);

        if(isMaskEnabled)
        {
            descrPtr->ipHeaderError = 0; /* doesn't trigger header error exception */
        }
    }

    if (descrPtr->ipHeaderError)
    {
        if (descrPtr->isIPv4)
        {
            if (maskId < SNET_IPV4_UC_HDR_SIP_NE_DIP_ERR_MASK_E)
            {
                descrPtr->ipHeaderErrorMask = maskId;
                __LOG(("IPV4 descrPtr->ipHeaderErrorMask =  [%d] \n", maskId));
            }
        }
        else
        {
            if (maskId < SNET_IPV6_UC_HDR_SIP_NE_DIP_ERR_MASK_E)
            {
                descrPtr->ipHeaderErrorMask = maskId;
                __LOG(("IPV6 descrPtr->ipHeaderErrorMask =  [%d] \n", maskId));
            }
        }
    }

    __LOG(("descrPtr->ipHeaderError = [%d] [%s] \n", descrPtr->ipHeaderError,
         descrPtr->ipHeaderError ? headerErrorDescriptionPtr : "NONE"));
}

/**
* @internal checkIpv4Header function
* @endinternal
*
* @brief   Checks ipv4 header
*
* @param[in,out] descrPtr                 - pointer to the frame's descriptor.
*/
static GT_VOID checkIpv4Header
(
    IN    SKERNEL_DEVICE_OBJECT           *devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC *descrPtr
)
{
    DECLARE_FUNC_NAME(checkIpv4Header);

    GT_U32 packetIpCheckSum;    /* calculated checkSum from the ip header */
    GT_U32 headerIpCheckSum;    /* field checkSum from the ip header */
    GT_U32 ipHeadVersion;       /* Ip header version */
    GT_U32 l3TotalLength;       /* Layer 3 total length */
    GT_U32 l3TotalLengthAfterDeduction; /* Layer 3 total length after deduction - for macSec*/
    GT_U32 l3byteOffset;        /* Layer 3 byte offset */
    GT_U32 headerLength;        /* protocol header length (in bytes) */
    GT_U32 crcOnPassengerLength = 0;/* crc bytes of ethernet passenger 'with CRC' */

    /* get protocol header length (in bytes) */
    headerLength = descrPtr->ipxHeaderLength * 4;

    /* the actual checksum received on the packet */
    headerIpCheckSum = (descrPtr->l3StartOffsetPtr[10] << 8) |
                        descrPtr->l3StartOffsetPtr[11];

    descrPtr->l3StartOffsetPtr[10] = 0;
    descrPtr->l3StartOffsetPtr[11] = 0;
    /* calculate the expected checksum */
    packetIpCheckSum = ipV4CheckSumCalc(descrPtr->l3StartOffsetPtr, (GT_U16)headerLength);
    /* restore checksum values */
    descrPtr->l3StartOffsetPtr[10] = (GT_U8)(headerIpCheckSum >> 8);
    descrPtr->l3StartOffsetPtr[11] = (GT_U8)headerIpCheckSum;

    ipHeadVersion = descrPtr->l3StartOffsetPtr[0] >> 4;
    l3TotalLength = (descrPtr->l3StartOffsetPtr[2] << 8) |
                     descrPtr->l3StartOffsetPtr[3];

    /* check deduction due to macsec */
    l3TotalLengthAfterDeduction = snetLion3IpTotalLengthAfterDeduction(devObjPtr, descrPtr, l3TotalLength);

    /* save total ipv4 length (in bytes) */
    descrPtr->ipxLength = l3TotalLengthAfterDeduction;

    l3byteOffset = descrPtr->l2HeaderSize;

    if(headerIpCheckSum != packetIpCheckSum)
    {
        descrPtr->ttiIpv4CheckSumError = 1;
        checkIpHeader(devObjPtr, descrPtr, SNET_IPV4_UC_HDR_CHKSUM_ERR_MASK_E, "Wrong IP header checksum");
        __LOG_PARAM(headerIpCheckSum);
        __LOG_PARAM(packetIpCheckSum);
    }

    if(ipHeadVersion != 4)
    {
        checkIpHeader(devObjPtr, descrPtr, SNET_IPV4_UC_HDR_VER_ERR_MASK_E, "Wrong IP header version");
        __LOG_PARAM(ipHeadVersion);
    }

    if(headerLength < 20 /* 5*4 */)
    {
        checkIpHeader(devObjPtr, descrPtr, SNET_IPV4_UC_HDR_LENGTH_ERR_MASK_E, "Wrong IP header header length");
        __LOG_PARAM(headerLength);
    }

    if(headerLength > l3TotalLength)
    {
        checkIpHeader(devObjPtr, descrPtr, SNET_IPV4_UC_HDR_LENGTH_ERR_MASK_E,
                            "Header length exceeds total datagram length");
        __LOG_PARAM(headerLength);
        __LOG_PARAM(l3TotalLength);
    }

    if (descrPtr->tunnelTerminated &&
        descrPtr->innerPacketType == SKERNEL_INNER_PACKET_TYPE_ETHERNET_WITH_CRC)
    {
        crcOnPassengerLength = 4;
        __LOG_PARAM(crcOnPassengerLength);
    }

    if((l3TotalLengthAfterDeduction + l3byteOffset + crcOnPassengerLength
              /* + 4 bytes CRC length, but simulation does not create CRC*/ ) > descrPtr->byteCount)
    {
        checkIpHeader(devObjPtr, descrPtr, SNET_IPV4_UC_HDR_LENGTH_ERR_MASK_E,
                            "Layer 3 length exceeds total frame length");
    }

    if(descrPtr->sip[0] == descrPtr->dip[0])
    {
        checkIpHeader(devObjPtr, descrPtr, SNET_IPV4_UC_HDR_SIP_NE_DIP_ERR_MASK_E,
                                        "Layer 3 header SIP must be != DIP");
    }

    /* extra TTI checking on ch3 and above */
    if(descrPtr->l3StartOffsetPtr[12] == 127 || /*127/8*/
       descrPtr->l3StartOffsetPtr[12] == 224 || /*multicast*/
       descrPtr->sip[0] == 0xffffffff)/* BC */
    {
        descrPtr->ipTtiHeaderError = 1;
        __LOG(("descrPtr->ipTtiHeaderError = 1;"));
    }

    if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        /* extra TTI checking on SIP5 */
        if(((descrPtr->l3StartOffsetPtr[12] == 255)&&
            (descrPtr->l3StartOffsetPtr[13] == 255)&&
            (descrPtr->l3StartOffsetPtr[14] == 255)&&
            (descrPtr->l3StartOffsetPtr[15] == 255)))/*255.255.255.255*/
        {
            descrPtr->ipTtiHeaderError = 1;
            __LOG(("descrPtr->ipTtiHeaderError = 1;"));
        }
    }
    else
    {
        if(descrPtr->l3StartOffsetPtr[12] == 240)/*class E*/
        {
            descrPtr->ipTtiHeaderError = 1;
            __LOG(("descrPtr->ipTtiHeaderError = 1;"));
        }
    }
}


/**
* @internal snetChtL3L4ProtParsingIpv4 function
* @endinternal
*
* @brief   Parsing of L3 and L4 protocols header (Ipv4)
*
* @param[in,out] descrPtr                 - pointer to the frame's descriptor.
*/
static GT_VOID snetChtL3L4ProtParsingIpv4
(
    IN    SKERNEL_DEVICE_OBJECT           *devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC *descrPtr
)
{
    DECLARE_FUNC_NAME(snetChtL3L4ProtParsingIpv4);

    GT_U32 startByte;            /* start byte for ip addresses */
    GT_U32 regAddress;
    GT_U32 regValue;
    GT_BIT ipv4MoreFragmentBit;

    __LOG(("Internet Protocol, Version 4 (IPv4)"));

    descrPtr->isIp = 1;
    descrPtr->isIPv4 = 1;
    descrPtr->dscp = (descrPtr->l3StartOffsetPtr[1] >> 2) & 0x3f;
    descrPtr->ipProt = descrPtr->l3StartOffsetPtr[9];
    descrPtr->ipv4DontFragmentBit = (descrPtr->l3StartOffsetPtr[6]>>6) & 1;
    ipv4MoreFragmentBit = (descrPtr->l3StartOffsetPtr[6]>>5) & 1;

    __LOG_PARAM(descrPtr->isIp);
    __LOG_PARAM(descrPtr->isIPv4);
    __LOG_PARAM(descrPtr->dscp);
    __LOG_PARAM(descrPtr->ipProt);
    __LOG_PARAM(descrPtr->ipv4DontFragmentBit);
    __LOG_PARAM(ipv4MoreFragmentBit);

    /* for ipv4 ip header length (in words) = IHL */
    descrPtr->ipxHeaderLength = (descrPtr->l3StartOffsetPtr[0] & 0xf); /* IHL */

    __LOG_PARAM(descrPtr->ipxHeaderLength);

    descrPtr->ipv4HeaderOptionsExists = (descrPtr->ipxHeaderLength > 5) ? 1 : 0;

    __LOG_PARAM(descrPtr->ipv4HeaderOptionsExists);

    descrPtr->ipv4FragmentOffset = ((GT_U32)(descrPtr->l3StartOffsetPtr[6] & 0x1f) << 8) |
                                      (descrPtr->l3StartOffsetPtr[7]);
    __LOG_PARAM(descrPtr->ipv4FragmentOffset);

    /* Check if packet is eligible for ECN marking */
    if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        regAddress = SMEM_XCAT_A1_TTI_UNIT_GLB_CONF_REG(devObjPtr);
        smemRegFldGet(devObjPtr, regAddress, 22, 1, &regValue);
        if (regValue && (descrPtr->l3StartOffsetPtr[1] & 0x3) &&
            (descrPtr->packetCmd == SKERNEL_EXT_PKT_CMD_FORWARD_E ||
             descrPtr->packetCmd == SKERNEL_EXT_PKT_CMD_FROM_CPU_E))
        {
            descrPtr->ecnCapable = 1;
        }
        else
        {
            descrPtr->ecnCapable = 0;
        }
        __LOG_PARAM(descrPtr->ecnCapable);
    }

    if ( descrPtr->ipv4FragmentOffset == 0 )
    {
        descrPtr->ipv4Fragmented = ipv4MoreFragmentBit? 1 /* first fragment*/ : 0 /* no fragment*/ ;
        descrPtr->l4StartOffsetPtr = descrPtr->l3StartOffsetPtr + descrPtr->ipxHeaderLength * 4;
        __LOG_PARAM(descrPtr->l4StartOffsetPtr);
    }
    else
    {
        descrPtr->ipv4Fragmented = ipv4MoreFragmentBit? 2 /*mid fragment*/ : 3 /*last fragment*/ ;
    }
    __LOG_PARAM(descrPtr->ipv4Fragmented);
    descrPtr->ttl = descrPtr->l3StartOffsetPtr[8];

    __LOG_PARAM(descrPtr->ttl);

    descrPtr->udpCompatible = (descrPtr->ipProt == SNET_UDP_PROT_E);

    if (descrPtr->udpCompatible && descrPtr->l4StartOffsetPtr)
    {
        if (descrPtr->macDaType == SKERNEL_BROADCAST_MAC_E &&
            descrPtr->l4StartOffsetPtr[2] == 0x02 &&
            descrPtr->l4StartOffsetPtr[3] == 0x08)
        {
            descrPtr->ipv4Ripv1 = 1;
            __LOG(("descrPtr->ipv4Ripv1 = 1;"));
        }
    }
    else if (descrPtr->ipProt == SNET_IGMP_PROT_E && descrPtr->l4StartOffsetPtr)
    {
        if (descrPtr->l4StartOffsetPtr[0] == 0x11)
        {
            descrPtr->igmpQuery = 1;
            __LOG(("descrPtr->igmpQuery = 1;"));
        }
        else
        {
            descrPtr->igmpNonQuery = 1;
            __LOG(("descrPtr->igmpNonQuery = 1;"));
        }
    }
    else if (descrPtr->ipProt == SNET_IPV4_ICMP_PROT_E)
    {
        descrPtr->ipv4Icmp = 1;
        __LOG(("descrPtr->ipv4Icmp = 1;"));
    }

    startByte = 12;

    /* SIP */
    descrPtr->sip[0] = SNET_BUILD_WORD_FROM_BYTES_PTR_MAC(&descrPtr->l3StartOffsetPtr[startByte]);

    startByte += 4;
    descrPtr->dip[0] = SNET_BUILD_WORD_FROM_BYTES_PTR_MAC(&descrPtr->l3StartOffsetPtr[startByte]);

    __LOG_PARAM(descrPtr->sip[0]);
    __LOG_PARAM(descrPtr->dip[0]);

    if ((descrPtr->dip[0] & 0xffffff00) == 0xE0000000)
    {
        descrPtr->ipXMcLinkLocalProt = 1;
        __LOG(("descrPtr->ipXMcLinkLocalProt = 1;"));
    }


    if ((descrPtr->dip[0] & 0xf0000000) == 0xE0000000)
    {
        /* DIP is multicast */
        descrPtr->ipm = 1;
        __LOG(("DIP is multicast"));
    }

    /* check header validity */
    checkIpv4Header(devObjPtr, descrPtr);

    descrPtr->l4Valid = 1;

    if (descrPtr->ipHeaderError)
    {
        if (descrPtr->ipHeaderErrorMask != SMAIN_NOT_VALID_CNS)
        {
            descrPtr->l4Valid = 0;
            __LOG(("IPv4 - L4 not valid: descrPtr->ipHeaderErrorMask = %d\n", descrPtr->ipHeaderErrorMask));
        }
    }
    else
    {
        if (descrPtr->ipTtiHeaderError || descrPtr->ipv4FragmentOffset != 0)
        {
            descrPtr->l4Valid = 0;
            if (descrPtr->ipTtiHeaderError)
            {
                __LOG(("IPv4 - L4 not valid: descrPtr->ipTtiHeaderError = %d\n", descrPtr->ipTtiHeaderError));
            }
            else
            {
                __LOG(("IPv4 - L4 not valid: descrPtr->ipv4FragmentOffset = %d\n", descrPtr->ipv4FragmentOffset));
            }
        }
        else
        {
            __LOG(("IPv4 - there are no header errors\n"));
        }
    }

    __LOG_PARAM(descrPtr->l4Valid);
}

/**
* @internal calcIpv6HeaderLength function
* @endinternal
*
* @brief   calculates ipv6 header length (in words)
*
* @param[in,out] descrPtr                 - pointer to the frame's descriptor.
*
* @retval ipv6 header length (in words) - 0 means not relevant
*/
static GT_U32 calcIpv6HeaderLength
(
    IN    SKERNEL_DEVICE_OBJECT           *devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC *descrPtr
)
{
    GT_U32 val = 0;
    GT_U32 hdrExtLen = 0;

    if((descrPtr->ipProt == SNET_IPV6_HBH_PROT_E) &&
            (descrPtr->l3StartOffsetPtr[40 + 16*2] == SNET_IPV6_NO_NEXT_HEADER_PROT_E))
    { /* If a single HBH header exists */

        /* get Hdr-Ext-Len -
           Length of HBH header in 8-octet units, not including the first 8 octets. */
        hdrExtLen = descrPtr->l3StartOffsetPtr[40 + 16*2 + 1];

        /* Desc<IPx Header Length> = 10 + (Hdr-Ext-Len+1)*2.
           Where Hdr-Ext-Len is taken from the HBH header */
        val = 10 + (hdrExtLen+1)*2;
    }
    else
    { /* If there are no header extensions Desc<IPx Header Length> = 10 */
        val = 10;
    }

    return val;
}


/**
* @internal isIpv6ExtendedHeaderExists function
* @endinternal
*
* @brief   Checks whether ipv6 extended header exists or not
*
* @param[in] devObjPtr  - pointer to device object.
* @param[in] nextHeader - next header value.
*
* @retval  lengthBytesUnit - != 0 the 'bytes' units of the 'length', 0 no header
*/
GT_U32 isIpv6ExtendedHeaderExists
(
    IN    SKERNEL_DEVICE_OBJECT *devObjPtr,
    IN    GT_U32                nextHeader
)
{
    DECLARE_FUNC_NAME(isIpv6ExtendedHeaderExists);

    GT_U32  i;
    GT_U32  regValue;


    /* check standard extended headers */
    for (i = 0; i < ipv6ExtHeadersAmount; i++)
    {
        if (nextHeader == ipv6ExtHeaders[i].value)
        {
            __LOG(("Byte value [0x%2.2x] is ipv6 extended header of type = [%s] \n",
                ipv6ExtHeaders[i].value,
                ipv6ExtHeaders[i].name));
            return ipv6ExtHeaders[i].lengthBytesUnit;
        }
    }

    if (SMEM_CHT_IS_SIP6_GET(devObjPtr)) /* sip6 recognize more types */
    {
        /* check standard extended headers */
        for (i = 0; i < sip6_ipv6ExtHeadersAmount; i++)
        {
            if (nextHeader == sip6_ipv6ExtHeaders[i].value)
            {
                __LOG(("(sip6) Byte value [0x%2.2x] is ipv6 extended header of type = [%s] \n",
                    sip6_ipv6ExtHeaders[i].value,
                    sip6_ipv6ExtHeaders[i].name));
                return sip6_ipv6ExtHeaders[i].lengthBytesUnit;
            }
        }
    }

    /*
         Check for extension values
            - <IPv6 Extension Value0>
            - <IPv6 Extension Value1>
    */
    smemRegGet(devObjPtr, SMEM_XCAT_TTI_IPV6_EXTENSION_VALUE_REG(devObjPtr) ,&regValue);
    if(SMEM_U32_GET_FIELD(regValue,0,8) == nextHeader ||
       SMEM_U32_GET_FIELD(regValue,8,8) == nextHeader )
    {
        __LOG(("Byte value [0x%2.2x] is ipv6 extended header (matched (from register) <IPv6 Extension Value0> or <IPv6 Extension Value1>)\n",
            nextHeader));

        return STANDARD_8_OCTETS;
    }

    __LOG(("Byte value [0x%2.2x] not recognized as extended headers. so the ipv6 hold no more extended headers \n",
        nextHeader));
    return 0;
}

/**
* @internal parseIpv6ExtenstionHeaders function
* @endinternal
*
* @brief   Parse IPv6 extension headers.
*
* @param[in] devObjPtr              - pointer to device object.
* @param[in,out] descrPtr           - pointer to the frame's descriptor.
*
*/
static GT_VOID parseIpv6ExtenstionHeaders
(
    IN    SKERNEL_DEVICE_OBJECT           *devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC *descrPtr
)
{
    GT_U8 *l3HeaderPtr;
    GT_U32 l3Length;
    GT_U32 ehLength= 0;
    GT_U32 ehCount = 1;
    GT_BOOL isExtensionHeader = GT_TRUE;
    GT_U32  nextHeader = 0;
    GT_BOOL isError = GT_FALSE;
    GT_U32  l4Offset = 0;
    GT_U32  lengthBytesUnit = STANDARD_8_OCTETS;
    GT_U32  last_lengthBytesUnit;
    GT_U32  regAddress;          /* Register address */
    GT_U32  regValue;            /* Register value */

    DECLARE_FUNC_NAME(parseIpv6ExtenstionHeaders);

    /* case when Encapsulate Secure Payload is the first extension */
    if (SNET_CHT_IPV6_EXT_HDR_ENCAP_SECUR_PAYLOAD_CNS == descrPtr->ipProt)
    {
        /* Encapsulate Secure Payload header is the last        */
        /* the payload follows after it secure encoded          */
        /* the contents of this header not parsed by the device */
        /* and the size of it not determinated                  */
        /* the L4 offset is invalid                             */
        descrPtr->l4Valid = 0;
        /* all the packet after IPx header - IPV6 extensions */
        descrPtr->ipv6HeaderLengthWithExtensionsInBytes =
            descrPtr->byteCount - descrPtr->l2HeaderSize;
        /* L4 is invalid, pointer set to the end of the packet */
        descrPtr->l4StartOffsetPtr =
            descrPtr->l3StartOffsetPtr + descrPtr->ipv6HeaderLengthWithExtensionsInBytes;
        __LOG(("IPV6 EH - Encapsulate Secure Payload obtained\n"));
        __LOG(("Total L3 header with extensions of [%d] bytes, the standard header bytes length [%d]\n",
            descrPtr->ipv6HeaderLengthWithExtensionsInBytes, (descrPtr->ipxHeaderLength * 4)));
        return;
    }

    last_lengthBytesUnit = isIpv6ExtendedHeaderExists(devObjPtr, descrPtr->ipProt);

    l3HeaderPtr = descrPtr->l3StartOffsetPtr + 40;
    l3Length = descrPtr->ipxHeaderLength*4;
    nextHeader = descrPtr->ipProt;
    while(ehCount <= IPV6_EXTENSIONS_MAX_SUPPORTED_CNS)
    {
        if (SMEM_CHT_IS_SIP6_GET(devObjPtr) && nextHeader == SNET_CHT_IPV6_EXT_HDR_ROUTING_CNS)
        {
            regAddress = SMEM_SIP6_TTI_TTI_UNIT_GLB_CONF_EXT_2_REG(devObjPtr);
            smemRegFldGet(devObjPtr, regAddress, 0, 8, &regValue);
            if (regValue == l3HeaderPtr[2] /*routing type in SR header*/)
            {
                descrPtr->srEhExists = 1;
                descrPtr->srSegmentsLeft = l3HeaderPtr[3];
                descrPtr->srHeaderStartOffsetPtr = &l3HeaderPtr[0];
            }
        }

        nextHeader = l3HeaderPtr[0];
        lengthBytesUnit = isIpv6ExtendedHeaderExists(devObjPtr, nextHeader);
        if (0 == lengthBytesUnit)
        {
            isExtensionHeader = GT_FALSE;
            break;
        }
        last_lengthBytesUnit = lengthBytesUnit;

        if (SNET_CHT_IPV6_EXT_HDR_ENCAP_SECUR_PAYLOAD_CNS == nextHeader)
        {
            /* Encapsulate Secure Payload header is the last        */
            /* the payload follows after it secure encoded          */
            /* the contents of this header not parsed by the device */
            /* and the size of it not determinated                  */
            /* the IPx protecol Id of such packet should be set to  */
            /* SNET_CHT_IPV6_EXT_HDR_ENCAP_SECUR_PAYLOAD_CNS        */
            /* the L4 offset is invalid                             */
            descrPtr->ipProt = SNET_CHT_IPV6_EXT_HDR_ENCAP_SECUR_PAYLOAD_CNS;
            descrPtr->l4Valid = 0;
            /* all the packet after IPx header - IPV6 extensions */
            descrPtr->ipv6HeaderLengthWithExtensionsInBytes =
                descrPtr->byteCount - descrPtr->l2HeaderSize;
            /* L4 is invalid, pointer set to the end of the packet */
            descrPtr->l4StartOffsetPtr =
                descrPtr->l3StartOffsetPtr + descrPtr->ipv6HeaderLengthWithExtensionsInBytes;
            __LOG(("IPV6 EH - Encapsulate Secure Payload obtained\n"));
            __LOG(("Total L3 header with extensions of [%d] bytes, the standard header bytes length [%d]\n",
                descrPtr->ipv6HeaderLengthWithExtensionsInBytes, (descrPtr->ipxHeaderLength * 4)));
            return;
        }

        if (ehCount == IPV6_EXTENSIONS_MAX_SUPPORTED_CNS)
        {
             __LOG(("Parser limitation : number of extensions exceed 4 , considered as Parser error \n"));
            isError = GT_TRUE;
            break;
        }
        ehLength = 8 + l3HeaderPtr[1] * lengthBytesUnit;
        __LOG(("[0x%2.2x] header extension with length [%d] bytes \n",
            nextHeader,
            ehLength));
        isExtensionHeader = GT_TRUE;
        if ((descrPtr->l2HeaderSize + l3Length + l4Offset + ehLength) <= descrPtr->byteCount)
        {
            /* shift L3 to next header */
            l3HeaderPtr = l3HeaderPtr + ehLength;
            l4Offset += ehLength;
        }
        else
        {
            isError = GT_TRUE;
            break;
        }
        ehCount++;
    }

    if (!isError && !isExtensionHeader)
    {
        ehLength = 8 + l3HeaderPtr[1]*last_lengthBytesUnit;
        __LOG(("[0x%2.2x] header extension with length [%d] bytes \n",
            l3HeaderPtr[1],
            ehLength));
        if ((descrPtr->l2HeaderSize + l3Length + l4Offset + ehLength) <= descrPtr->byteCount)
        {
            l4Offset += ehLength;
        }
        else
        {
            isError = GT_TRUE;
        }
    }

    if(isError == GT_FALSE && l4Offset > 64)
    {
        __LOG(("Parser limitation : extensions length [%d] exceed 64 , considered as Parser error \n",
            l4Offset));
        isError = GT_TRUE;
    }

    __LOG(("Got total extensions of [%d] bytes in addition to standard header bytes length [%d] \n",
        l4Offset , 4* descrPtr->ipxHeaderLength));

    descrPtr->ipv6HeaderLengthWithExtensionsInBytes = l4Offset + 4* descrPtr->ipxHeaderLength;
    __LOG_PARAM(descrPtr->ipv6HeaderLengthWithExtensionsInBytes);

    if (GT_TRUE == isError)
    {
        __LOG(("Ipv6 Parser error : (set descrPtr->l4Valid = 0) \n"));
        descrPtr->ipv6HeaderParserError = 1;
        __LOG_PARAM(descrPtr->ipv6HeaderParserError);
        descrPtr->l4Valid = 0;
        return;
    }

    if (isExtensionHeader)
    {
        descrPtr->ipv6HeaderParserError = 1;
        __LOG_PARAM(descrPtr->ipv6HeaderParserError);
        descrPtr->l4Valid = 0;
        __LOG(("(Parser error) Last IPv6 next header [%d] is an extension header (set descrPtr->l4Valid = 0)\n", nextHeader));
    }
    else
    {
        descrPtr->ipProt = nextHeader;
        __LOG(("Updated descrPtr->ipProt[%d] ",descrPtr->ipProt));
        __LOG(("Last IPv6 next header [%d] is not an extension header \n", nextHeader));
        descrPtr->l4StartOffsetPtr = descrPtr->l3StartOffsetPtr + descrPtr->ipv6HeaderLengthWithExtensionsInBytes;
    }
}

/**
* @internal checkIpv6Header function
* @endinternal
*
* @brief   Checks ipv6 header
*
* @param[in,out] descrPtr                 - pointer to the frame's descriptor.
*/
static GT_VOID checkIpv6Header
(
    IN    SKERNEL_DEVICE_OBJECT           *devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC *descrPtr
)
{
    DECLARE_FUNC_NAME(checkIpv6Header);

    GT_U32 ipHeadVersion;       /* Ip header version */
    GT_U32 payLoadLength;       /* Ipv6 Payload length */
    GT_U32 l3byteOffset;        /* Layer 3 byte offset */
    GT_U32 l3TotalLengthAfterDeduction; /* Layer 3 total length after deduction - for macSec*/
    GT_U32 sipLoopback;        /* SIP Loopback ?   */
    GT_U32 sipMulticast;       /* SIP Multicast ?  */
    GT_U32 crcOnPassengerLength = 0;/* crc bytes of ethernet passenger 'with CRC' */

    ipHeadVersion = descrPtr->l3StartOffsetPtr[0] >> 4;
    payLoadLength = (descrPtr->l3StartOffsetPtr[4] << 8) |
                     descrPtr->l3StartOffsetPtr[5];

    /* check deduction due to macsec */
    l3TotalLengthAfterDeduction = snetLion3IpTotalLengthAfterDeduction(devObjPtr, descrPtr, payLoadLength);
    __LOG_PARAM(l3TotalLengthAfterDeduction);

    descrPtr->ipxLength = l3TotalLengthAfterDeduction + descrPtr->ipxHeaderLength * 4;

    l3byteOffset = descrPtr->l2HeaderSize;

    if (ipHeadVersion != 6)
    {
        checkIpHeader(devObjPtr, descrPtr, SNET_IPV6_UC_HDR_VER_ERR_MASK_E, "Wrong IP header version");
        __LOG_PARAM(ipHeadVersion);
    }

    if (descrPtr->tunnelTerminated &&
        descrPtr->innerPacketType == SKERNEL_INNER_PACKET_TYPE_ETHERNET_WITH_CRC)
    {
        crcOnPassengerLength = 4;
        __LOG_PARAM(crcOnPassengerLength);
    }

    if ((descrPtr->ipxLength + l3byteOffset + crcOnPassengerLength
        /* +  4 bytes CRC length,but simulation does not create CRC*/ ) > descrPtr->byteCount)
    {
        checkIpHeader(devObjPtr, descrPtr, SNET_IPV6_UC_HDR_LENGTH_ERR_MASK_E, "Wrong header length");
    }


    if ((descrPtr->sip[0] == descrPtr->dip[0]) && (descrPtr->sip[1] == descrPtr->dip[1]) &&
        (descrPtr->sip[2] == descrPtr->dip[2]) && (descrPtr->sip[3] == descrPtr->dip[3]))
    {
        checkIpHeader(devObjPtr, descrPtr, SNET_IPV6_UC_HDR_SIP_NE_DIP_ERR_MASK_E,
                                        "Layer 3 header SIP must be != DIP");
    }

    /*
        An IPv6 TTI SIP Address Error is triggered if ANY of the following TRUE:
        - IPv6 header <SIP> is ::1/128 (Loopback address)
        - IPv6 header <SIP> is FF::/8 (Multicast address)
    */
    sipLoopback =
        ((descrPtr->sip[0] == 0) && (descrPtr->sip[1] == 0)
         && (descrPtr->sip[2] == 0) && (descrPtr->sip[3] == 1));

    sipMulticast = ((descrPtr->sip[0] & 0xFF000000) == 0xFF000000);

    if (sipLoopback || sipMulticast)
    {
        descrPtr->ipTtiHeaderError = 1;
        __LOG(("descrPtr->ipTtiHeaderError = 1;"));
        __LOG_PARAM(sipLoopback);
        __LOG_PARAM(sipMulticast);
    }


    descrPtr->l4Valid = 1;

    if(isIpv6ExtendedHeaderExists(devObjPtr, descrPtr->ipProt))
    {
        if (SMEM_CHT_IS_SIP6_GET(devObjPtr))
        {
            parseIpv6ExtenstionHeaders(devObjPtr, descrPtr);
        }
        else
        {
            /*If any of the ipv6 extended headers are identified,
            the <Is L4 Valid> indication in the IPCL key is set to NOT VALID: */
            descrPtr->l4Valid = 0;
        }
    }

    if (descrPtr->l4Valid && (descrPtr->ipxHeaderLength > 32))
    { /* For IPv6 if Desc<L4 valid> = 1 then the IP header must be less than 128 bytes */

        checkIpHeader(devObjPtr, descrPtr, SNET_IPV6_UC_HDR_LENGTH_ERR_MASK_E, "Wrong header length");
    }

}

/**
* @internal snetChtL3L4ProtParsingIpv6 function
* @endinternal
*
* @brief   Parsing of L3 and L4 protocols header (Ipv6)
*
* @param[in,out] descrPtr                 - pointer to the frame's descriptor.
*/
static GT_VOID snetChtL3L4ProtParsingIpv6
(
    IN    SKERNEL_DEVICE_OBJECT           *devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC *descrPtr
)
{
    DECLARE_FUNC_NAME(snetChtL3L4ProtParsingIpv6);

    GT_U32 startByte;            /* start Byte for ip addresses */
    GT_U32 soliciteMask[4];     /* IPv6 Solicited-Node Mcst Mask */
    GT_U32 soliciteValue[4];    /* IPv6 Solicited-Node Mcst Value */
    GT_U32 *regDataPtr;         /* Registers data pointer */
    GT_U32 regNum;              /* Register index */
    GT_U32 regAddress;          /* Register address */
    GT_U32 regValue;            /* Register value */
    GT_BOOL fragment = GT_FALSE;/* is ipv4/6 considered 'fragment' */
    GT_U8 icmpMsgType;
    GT_U32  outOfRangeOffset=0,numBytesFromStartOfPacket;


    __LOG(("Internet Protocol, Version 6 (IPv6)"));

    descrPtr->isIp = 1;
    descrPtr->isIPv4 = 0;
    descrPtr->flowLabel = ((descrPtr->l3StartOffsetPtr[1] & 0xf) << 16 |
                           (descrPtr->l3StartOffsetPtr[2] << 8 ) |
                           (descrPtr->l3StartOffsetPtr[3]));

    descrPtr->dscp = ((descrPtr->l3StartOffsetPtr[0] << 2) |
                      (descrPtr->l3StartOffsetPtr[1] >> 6) ) & 0x3f;

    /* Check if packet is eligible for ECN marking */
    if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        regAddress = SMEM_XCAT_A1_TTI_UNIT_GLB_CONF_REG(devObjPtr);
        smemRegFldGet(devObjPtr, regAddress, 23, 1, &regValue);
        if (regValue && ((descrPtr->l3StartOffsetPtr[1] >> 4) & 0x3) &&
            (descrPtr->packetCmd == SKERNEL_EXT_PKT_CMD_FORWARD_E ||
             descrPtr->packetCmd == SKERNEL_EXT_PKT_CMD_FROM_CPU_E))
        {
            descrPtr->ecnCapable = 1;
        }
        else
        {
            descrPtr->ecnCapable = 0;
        }
        __LOG_PARAM(descrPtr->ecnCapable);
    }

    descrPtr->ipProt = descrPtr->l3StartOffsetPtr[6];

    __LOG_PARAM(descrPtr->isIp);
    __LOG_PARAM(descrPtr->isIPv4);
    __LOG_PARAM(descrPtr->flowLabel);
    __LOG_PARAM(descrPtr->dscp);
    __LOG_PARAM(descrPtr->ipProt);

    /* calculate ipv6 header length */
    descrPtr->ipxHeaderLength = calcIpv6HeaderLength(devObjPtr, descrPtr);

    __LOG_PARAM(descrPtr->ipxHeaderLength);

    fragment = (descrPtr->ipProt == 44) ? GT_TRUE : GT_FALSE;

    __LOG_PARAM(fragment);

    if(fragment == GT_FALSE)
    {
        descrPtr->l4StartOffsetPtr = descrPtr->l3StartOffsetPtr + descrPtr->ipxHeaderLength*4;
    }

    descrPtr->ttl = descrPtr->l3StartOffsetPtr[7];

    __LOG_PARAM(descrPtr->ttl);

    descrPtr->udpCompatible = (descrPtr->ipProt == SNET_UDP_PROT_E);

    if (descrPtr->ipProt == SNET_IPV6_ICMP_PROT_E && descrPtr->l4StartOffsetPtr)
    {
        descrPtr->ipv6Icmp = 1;
        descrPtr->ipv6IcmpType = descrPtr->l4StartOffsetPtr[0];
    }
    else if (descrPtr->ipProt == 0 && descrPtr->l4StartOffsetPtr &&
             descrPtr->l4StartOffsetPtr[0] == SNET_IPV6_ICMP_PROT_E)
    {
        descrPtr->ipv6Icmp = 1;
        descrPtr->ipv6IcmpType = descrPtr->l4StartOffsetPtr[8];
    }

    __LOG_PARAM(descrPtr->ipv6Icmp);
    __LOG_PARAM(descrPtr->ipv6IcmpType);

    descrPtr->isIpV6EhExists = isIpv6ExtendedHeaderExists(devObjPtr, descrPtr->ipProt) ? 1 : 0;

    descrPtr->isIpV6EhHopByHop =
        (descrPtr->ipProt == SNET_CHT_IPV6_EXT_HDR_HOP_BY_HOP_CNS)
        ? 1 : 0;

    __LOG_PARAM(descrPtr->isIpV6EhExists);
    __LOG_PARAM(descrPtr->isIpV6EhHopByHop);


    if (descrPtr->isIpV6EhExists)
    {
        /* skip extended header */
        GT_U32 payloadSize = (descrPtr->l3StartOffsetPtr[4] << 8) + descrPtr->l3StartOffsetPtr[5];
        icmpMsgType = descrPtr->l3StartOffsetPtr[payloadSize];
    }
    else
    {
        icmpMsgType = descrPtr->l4StartOffsetPtr[0];
    }

    __LOG_PARAM(icmpMsgType);

    switch (icmpMsgType)
    {
        case 130: /* Multicast Listener Query */
        case 131: /* Group Membership Report */
        case 132: /* Group Membership Reduction */
        case 143: /* MLDv2 Multicast Listener Report */
            descrPtr->isIpv6Mld = 1;
            break;
        default:
            descrPtr->isIpv6Mld = 0;
            break;
    }

    __LOG_PARAM(descrPtr->isIpv6Mld);

    startByte = 8;

    /* SIP */
    __LOG(("SIP"));
    descrPtr->sip[0] = SNET_BUILD_WORD_FROM_BYTES_PTR_MAC(&descrPtr->l3StartOffsetPtr[startByte]);
    startByte+=4;
    descrPtr->sip[1] = SNET_BUILD_WORD_FROM_BYTES_PTR_MAC(&descrPtr->l3StartOffsetPtr[startByte]);
    startByte+=4;
    descrPtr->sip[2] = SNET_BUILD_WORD_FROM_BYTES_PTR_MAC(&descrPtr->l3StartOffsetPtr[startByte]);
    startByte+=4;
    descrPtr->sip[3] = SNET_BUILD_WORD_FROM_BYTES_PTR_MAC(&descrPtr->l3StartOffsetPtr[startByte]);

    __LOG_PARAM(descrPtr->sip[0]);
    __LOG_PARAM(descrPtr->sip[1]);
    __LOG_PARAM(descrPtr->sip[2]);
    __LOG_PARAM(descrPtr->sip[3]);

    /* DIP */
    __LOG(("DIP"));
    startByte+=4;
    descrPtr->dip[0] = SNET_BUILD_WORD_FROM_BYTES_PTR_MAC(&descrPtr->l3StartOffsetPtr[startByte]);
    startByte+=4;
    descrPtr->dip[1] = SNET_BUILD_WORD_FROM_BYTES_PTR_MAC(&descrPtr->l3StartOffsetPtr[startByte]);
    startByte+=4;
    descrPtr->dip[2] = SNET_BUILD_WORD_FROM_BYTES_PTR_MAC(&descrPtr->l3StartOffsetPtr[startByte]);
    startByte+=4;
    descrPtr->dip[3] = SNET_BUILD_WORD_FROM_BYTES_PTR_MAC(&descrPtr->l3StartOffsetPtr[startByte]);
    startByte+=4;

    __LOG_PARAM(descrPtr->dip[0]);
    __LOG_PARAM(descrPtr->dip[1]);
    __LOG_PARAM(descrPtr->dip[2]);
    __LOG_PARAM(descrPtr->dip[3]);

    numBytesFromStartOfPacket = &descrPtr->l3StartOffsetPtr[startByte] -
                                 descrPtr->origInfoBeforeTunnelTermination.originalL2Ptr;
    if(numBytesFromStartOfPacket > TTI_PARSER_DEPTH(devObjPtr))
    {
        __LOG(("Warning : IPv6 header parser error : the device limited to [%d] bytes from start of packet ,but the IP DIP is out of range (ends at byte[%d])\n",
            TTI_PARSER_DEPTH(devObjPtr),
            numBytesFromStartOfPacket-1));
        /* set indication for later use of header error */
        outOfRangeOffset = 1;
    }


    if ((descrPtr->dip[0] & 0xffffffff) == 0xFF020000 &&
        (descrPtr->dip[1] & 0xffffffff) == 0 &&
        (descrPtr->dip[2] & 0xffffffff)  == 0 &&
        (descrPtr->dip[3] & 0xffff0000) == 0 )
    {
        descrPtr->ipXMcLinkLocalProt = 1;
    }

    __LOG_PARAM(descrPtr->ipXMcLinkLocalProt);

    regDataPtr = smemMemGet(devObjPtr, SMEM_CHT_SOLIC_NODE_MCST_ADDR_REG(devObjPtr));
    for(regNum = 0; regNum < 4; regNum++)
    {
        soliciteValue[3 - regNum] = regDataPtr[regNum];
    }

    regDataPtr = smemMemGet(devObjPtr, SMEM_CHT_SOLIC_NODE_MCST_MASK_REG(devObjPtr));
    for(regNum = 0; regNum < 4; regNum++)
    {
        soliciteMask[3 - regNum] = regDataPtr[regNum];
    }

    if (((descrPtr->dip[0] & soliciteMask[0]) == (soliciteMask[0] & soliciteValue[0])) &&
        ((descrPtr->dip[1] & soliciteMask[1]) == (soliciteMask[1] & soliciteValue[1])) &&
        ((descrPtr->dip[2] & soliciteMask[2]) == (soliciteMask[2] & soliciteValue[2])) &&
        ((descrPtr->dip[3] & soliciteMask[3]) == (soliciteMask[3] & soliciteValue[3])))
    {
        descrPtr->solicitationMcastMsg = 1;
    }

    __LOG_PARAM(descrPtr->solicitationMcastMsg);

    if((descrPtr->dip[0] & 0xFF000000) == 0xFF000000)
    {
        /* DIP is multicast */
        descrPtr->ipm = 1;
        __LOG(("DIP is multicast"));
    }

    /* check header validity */
    checkIpv6Header(devObjPtr, descrPtr);

    if (descrPtr->ipHeaderError)
    {
        if (descrPtr->ipHeaderErrorMask != SMAIN_NOT_VALID_CNS)
        {
            descrPtr->l4Valid = 0;
            descrPtr->l4StartOffsetPtr = NULL;/* CQ#150407 */
            __LOG(("IPv6 - L4 not valid: descrPtr->ipHeaderErrorMask = %d\n", descrPtr->ipHeaderErrorMask));
        }
    }
    else
    {
        if (descrPtr->ipTtiHeaderError)
        {
            descrPtr->l4Valid = 0;
            descrPtr->l4StartOffsetPtr = NULL;/* CQ#150407 */
            __LOG(("IPv6 - L4 not valid: descrPtr->ipTtiHeaderError = %d\n", descrPtr->ipTtiHeaderError));
        }
        else
        {
            __LOG(("IPv6 - there are no header errors"));
        }
    }

    if(outOfRangeOffset)
    {
        descrPtr->ipHeaderError = 1;
        __LOG(("ipHeaderError = 1 due to ipv6 header info is out of range of the parser \n"));
        __LOG_PARAM(descrPtr->ipHeaderError);
    }

    __LOG_PARAM(descrPtr->ipm);
    __LOG_PARAM(descrPtr->l4Valid);

}

/**
* @internal snetChtL3L4ProtParsingMim function
* @endinternal
*
* @brief   Parsing of L3 and L4 protocols header (MIM)
*
* @param[in,out] descrPtr                 - pointer to the frame's descriptor.
*/
static GT_VOID snetChtL3L4ProtParsingMim
(
    IN    SKERNEL_DEVICE_OBJECT           *devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC *descrPtr,
    IN    GT_U32                           etherType,
    IN INTERNAL_TTI_DESC_INFO_STC      *internalTtiInfoPtr
)
{
    DECLARE_FUNC_NAME(snetChtL3L4ProtParsingMim);

    GT_U32 mimIEtherType;         /* MIM I-EtherType */
    GT_U32 startBit;

    if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        startBit = 0;
    }
    else
    {
        startBit = 16;
    }

    /* MIM packets are identified according to the configurable I-EtherType */
    smemRegFldGet(devObjPtr, SMEM_XCAT_SPECIAL_ETHERTYPES_REG(devObjPtr),
                    startBit, 16, &mimIEtherType);

    __LOG_PARAM(mimIEtherType);

    if (etherType != mimIEtherType)
    {
        __LOG(("packet is NOT MIM (mac in mac) \n"));
        return;
    }

    __LOG(("packet is MIM (mac in mac) \n"));

    descrPtr->mim   = 1;
    descrPtr->iUp   = (GT_U32)((descrPtr->l3StartOffsetPtr[0] >> 5) & 0x7);
    descrPtr->iDp   = (GT_BIT)((descrPtr->l3StartOffsetPtr[0] >> 4) & 0x1);
    descrPtr->iRes1 = (GT_U32)((descrPtr->l3StartOffsetPtr[0] >> 2) & 0x3);
    descrPtr->iRes2 = (GT_U32)((descrPtr->l3StartOffsetPtr[0] >> 0) & 0x3);
    descrPtr->iSid  = (GT_U32)((descrPtr->l3StartOffsetPtr[1] << 16)) |
                      (GT_U32)((descrPtr->l3StartOffsetPtr[2] << 8))  |
                      (GT_U32)((descrPtr->l3StartOffsetPtr[3]));

    __LOG_PARAM(descrPtr->mim);
    __LOG_PARAM(descrPtr->iUp);
    __LOG_PARAM(descrPtr->iDp);
    __LOG_PARAM(descrPtr->iRes1);
    __LOG_PARAM(descrPtr->iRes2);
    __LOG_PARAM(descrPtr->iSid);

    descrPtr->ingressTunnelInfo.innerFrameDescrPtr =
            snetChtEqDuplicateDescr(devObjPtr, descrPtr);
    /*
        <-- startFramePtr here -->
        6 bytes - Backbone destination address (B-DA) (six bytes)
        6 bytes - Backbone source address (B-SA) (six bytes)
        2 bytes - EtherType 0x88A8 (two bytes)
        2 bytes - B-TAG/B-VID (two bytes), this is the backbone VLAN indicator
            Service encapsulation, that has:
        2 bytes - EtherType 0x88E7 (two bytes)
        <-- l3StartOffsetPtr here -->
        1 byte - Flags that contain priority, Drop Eligible Indicator (DEI) and No Customer Address (NCA) indication (e.g. OAM frames).
        3 bytes - I-SID, the service identifier (three bytes)
        =============
        total 22 bytes from start of packet, or 4 bytes from L3 offset
    */
    descrPtr->ingressTunnelInfo.innerFrameDescrPtr->startFramePtr =
        descrPtr->l3StartOffsetPtr + 4;                     /* start of 'inner frame' */
    descrPtr->ingressTunnelInfo.innerFrameDescrPtr->byteCount -=
        (descrPtr->ingressTunnelInfo.innerFrameDescrPtr->startFramePtr -
         descrPtr->startFramePtr);

    /* reset the L3 pointer , because it is not relevant any more -->
               will be set inside snetChtL2Parsing(...) for the 'passenger'  */
    descrPtr->ingressTunnelInfo.innerFrameDescrPtr->l3StartOffsetPtr = NULL;
    descrPtr->ingressTunnelInfo.innerFrameDescrPtr->macDaPtr = DST_MAC_FROM_TUNNEL_DSCR(descrPtr->ingressTunnelInfo.innerFrameDescrPtr);
    descrPtr->ingressTunnelInfo.innerFrameDescrPtr->macSaPtr = SRC_MAC_FROM_TUNNEL_DSCR(descrPtr->ingressTunnelInfo.innerFrameDescrPtr);

    /* do re-parsing on the passenger , to set the L2 info about vlan tag*/
    snetChtL2Parsing(devObjPtr,
        descrPtr->ingressTunnelInfo.innerFrameDescrPtr,
        SNET_CHT_FRAME_PARSE_MODE_FROM_TTI_PASSENGER_E,
        internalTtiInfoPtr);

    descrPtr->ingressTunnelInfo.innerMacDaPtr = descrPtr->ingressTunnelInfo.innerFrameDescrPtr->macDaPtr;
    descrPtr->ingressTunnelInfo.innerPacketL2FieldsAreValid = 1;
    /* set innerTag0Exists,innerPacketTag0Vid,innerPacketTag0CfiDei,innerPacketTag0Up fields in descriptor */
    snetLion3PassengerOuterTagIsTag0_1(devObjPtr,descrPtr);

    /* we not keep this internal frame info to not impact our QOS/hash decisions */
    descrPtr->ingressTunnelInfo.innerFrameDescrPtr = NULL;
}

/**
* @internal snetChtL3L4ProtParsing function
* @endinternal
*
* @brief   Parsing of L3 and L4 protocols header
*
* @param[in,out] descrPtr                 - pointer to the frame's descriptor.
*/
extern GT_VOID snetChtL3L4ProtParsing
(
    IN    SKERNEL_DEVICE_OBJECT           *devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC *descrPtr,
    IN    GT_U32                           etherType,
    IN    INTERNAL_TTI_DESC_INFO_STC      *internalTtiInfoPtr
)
{
    DECLARE_FUNC_NAME(snetChtL3L4ProtParsing);
    GT_U32  Tcp_data_offset;
    GT_U32  stopParsing = 0;

    __LOG(("Dump first 32 bytes of the L3 header \n"));
    /* dump the start of L3 */
    simLogPacketDump(devObjPtr,
                    GT_TRUE/*ingress*/,
                    descrPtr->localDevSrcPort,/* local device source port */
                    descrPtr->l3StartOffsetPtr,/*start of L3 header */
                    32);/* dump 32 bytes of the header */

    __LOG(("************************************************************* \n"));
    __LOG(("************** start parse L3 header of the packet ********** \n"));
    __LOG(("************************************************************* \n"));
    if (etherType == SKERNEL_L3_PROT_TYPE_ARP_E)      /* Address Resolution Protocol (ARP) */
    {
        __LOG(("L3 is ARP \n"));
        snetChtL3L4ProtParsingArp(devObjPtr, descrPtr);
    }
    else if (etherType == SKERNEL_L3_PROT_TYPE_IPV4_E) /* Internet Protocol, Version 4 (IPv4) */
    {
        __LOG(("L3 is Ipv4 \n"));
        snetChtL3L4ProtParsingIpv4(devObjPtr, descrPtr);
    }
    else if (etherType == SKERNEL_L3_PROT_TYPE_IPV6_E) /* Internet Protocol, Version 6 (IPv6) */
    {
        __LOG(("L3 is Ipv6 \n"));
        snetChtL3L4ProtParsingIpv6(devObjPtr, descrPtr);
    }
    else if( parseMpls(devObjPtr, descrPtr, etherType, internalTtiInfoPtr) )/* check if MPLS */
    {
        __LOG(("MPLS passenger (inner frame) parsing is done\n"));
        stopParsing = 1;
    }
    else
    {
        snetChtL3L4ProtParsingMim(devObjPtr, descrPtr, etherType, internalTtiInfoPtr);

        if(SMEM_CHT_IS_SIP5_GET(devObjPtr) && descrPtr->mim == 0)
        {
            /* FCOE */
            __LOG(("Check for FCoE Recognition \n"));
            snetLion3TtiFcoeTreat(devObjPtr, descrPtr, internalTtiInfoPtr);
        }
    }

    if (SMEM_CHT_IS_SIP6_GET(devObjPtr) && (descrPtr->mpls == 0))
    {
        /* for MPLS packets called at passenger parsing only */
        snetSip6CutThroughTtiSupport(
            devObjPtr, descrPtr, etherType, internalTtiInfoPtr);
    }

    __LOG(("************************************************************* \n"));
    __LOG(("************** end parse L3 header of the packet   ********** \n"));
    __LOG(("************************************************************* \n"));

    if(stopParsing)
    {
        /* do the return after LOG info */
        return;
    }


    if (descrPtr->l4StartOffsetPtr)
    {
        __LOG(("Dump first 16 bytes of the L4 header \n"));
        /* dump the start of L4 */
        simLogPacketDump(devObjPtr,
                        GT_TRUE/*ingress*/,
                        descrPtr->localDevSrcPort,/* local device source port */
                        descrPtr->l4StartOffsetPtr,/*start of L4 header */
                        16);/* dump 32 bytes of the header */
    }
    else
    {
        __LOG(("NO L4 header in this packet \n"));
    }

    __LOG_PARAM(descrPtr->ipProt);

    if((descrPtr->udpCompatible || descrPtr->ipProt == SNET_TCP_PROT_E)
       && descrPtr->l4StartOffsetPtr)
    {
        __LOG(("************************************************************* \n"));
        __LOG(("************** start parse L4 header of the packet ********** \n"));
        __LOG(("************************************************************* \n"));

        descrPtr->l4SrcPort = (descrPtr->l4StartOffsetPtr[0] << 8) |
                     (descrPtr->l4StartOffsetPtr[1]);

        descrPtr->l4DstPort = (descrPtr->l4StartOffsetPtr[2] << 8) |
                     (descrPtr->l4StartOffsetPtr[3]);

        __LOG_PARAM(descrPtr->l4SrcPort);
        __LOG_PARAM(descrPtr->l4DstPort);

        if(descrPtr->ipProt == SNET_TCP_PROT_E)
        {
            __LOG(("L4 is TCP \n"));

            descrPtr->tcpFlags = descrPtr->l4StartOffsetPtr[13] & 0x3F;/*6 bits*/
            descrPtr->tcpSyn = SMEM_U32_GET_FIELD(descrPtr->tcpFlags,1,1);
            if(descrPtr->tcpSyn)
            {
                Tcp_data_offset = descrPtr->l4StartOffsetPtr[12] >> 4;/*4 MSBits*/

                if(descrPtr->ipxLength  != ((Tcp_data_offset*4) + descrPtr->ipxLength))
                {
                    /* there are bytes abter the TCP header .. so we consider it as "SYN with data" */
                    descrPtr->tcpSynWithData = 1;
                }
            }

            __LOG_PARAM(descrPtr->tcpFlags);
            __LOG_PARAM(descrPtr->tcpSyn);
            __LOG_PARAM(descrPtr->tcpSynWithData);
        }
        else
        {
            __LOG(("L4 is UDP \n"));
        }

        __LOG(("************************************************************* \n"));
        __LOG(("************** end parse L4 header of the packet   ********** \n"));
        __LOG(("************************************************************* \n"));
    }

    if(descrPtr->l4StartOffsetPtr)
    {
        descrPtr->l23HeaderSize = descrPtr->l4StartOffsetPtr - descrPtr->startFramePtr;
    }

    __LOG(("****Start Trunk Hash (simple hash) calculation \n"));
    /* Trunk Hash calculation */
    snetXCatA1TrunkHash(devObjPtr, descrPtr);

    __LOG(("****End Trunk Hash (simple hash) calculation \n"));
}

/**
* @internal snetChtL3L4ProtParsingResetDesc function
* @endinternal
*
* @brief   reset the fileds that involved in Parsing of L3 and L4 protocols header
*         this is needed since the TTI need to reparse the L3,L4 when it terminates
*         the tunnel.
*/
extern GT_VOID snetChtL3L4ProtParsingResetDesc
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr
)
{
    if(descrPtr->macDaType == SKERNEL_BROADCAST_ARP_E)
    {
        descrPtr->macDaType = SKERNEL_BROADCAST_MAC_E;
    }

    descrPtr->arp = 0;
    descrPtr->isIp = 0;
    descrPtr->isIPv4 = 0;
    descrPtr->isFcoe = 0;
    descrPtr->l4StartOffsetPtr = NULL;
    descrPtr->udpCompatible = GT_FALSE;
    descrPtr->igmpQuery = 0;
    descrPtr->ipv4Ripv1 = 0;
    descrPtr->igmpNonQuery = 0;
    descrPtr->ipv4Icmp = 0;
    descrPtr->ipXMcLinkLocalProt = 0;
    descrPtr->ipm = 0;
    descrPtr->ipv6Icmp = 0;
    descrPtr->solicitationMcastMsg = 0;
    descrPtr->mpls = 0;
    descrPtr->numOfLabels = 0;
    descrPtr->ipProt = 0;
    descrPtr->mim = 0;
    descrPtr->l4Valid = 0;
    descrPtr->ipHeaderError = 0;
    descrPtr->ipHeaderErrorMask = SMAIN_NOT_VALID_CNS;
    descrPtr->tcpFlags = 0;
    descrPtr->tcpSyn = 0;
    descrPtr->tcpSynWithData = 0;
    descrPtr->ttl = 0;
    descrPtr->ecnCapable = 0;
}

/**
* @internal snetChtL3L4ProtCopyDesc function
* @endinternal
*
* @brief   copy ONLY L3/L4 relevant fields from source to target descriptor
*/
extern GT_VOID snetChtL3L4ProtCopyDesc
(
    IN SKERNEL_DEVICE_OBJECT            * devObjPtr,
    OUT SKERNEL_FRAME_CHEETAH_DESCR_STC * trgDescrPtr,
    IN  SKERNEL_FRAME_CHEETAH_DESCR_STC * srcDescrPtr
)
{
    /* scibAccessLock the needed for __LOG_NO_LOCATION_META_DATA inside CPY_DESC_FIELD_MAC */
    scibAccessLock();
    CPY_DESC_FIELD_MAC(trgDescrPtr,srcDescrPtr,    macDaType                );
    CPY_DESC_FIELD_MAC(trgDescrPtr,srcDescrPtr,    isIp                     );
    CPY_DESC_FIELD_MAC(trgDescrPtr,srcDescrPtr,    isIPv4                   );
    CPY_DESC_FIELD_MAC(trgDescrPtr,srcDescrPtr,    dscp                     );
    CPY_DESC_FIELD_MAC(trgDescrPtr,srcDescrPtr,    l3StartOffsetPtr         );
    CPY_DESC_FIELD_MAC(trgDescrPtr,srcDescrPtr,    l4StartOffsetPtr         );
    CPY_DESC_FIELD_MAC(trgDescrPtr,srcDescrPtr,    ttl                      );
    CPY_DESC_FIELD_MAC(trgDescrPtr,srcDescrPtr,    udpCompatible            );
    CPY_DESC_FIELD_MAC(trgDescrPtr,srcDescrPtr,    ipv4Ripv1                );
    CPY_DESC_FIELD_MAC(trgDescrPtr,srcDescrPtr,    igmpQuery                );
    CPY_DESC_FIELD_MAC(trgDescrPtr,srcDescrPtr,    igmpNonQuery             );
    CPY_DESC_FIELD_MAC(trgDescrPtr,srcDescrPtr,    ipv4Icmp                 );
    CPY_DESC_FIELD_MAC(trgDescrPtr,srcDescrPtr,    ipXMcLinkLocalProt       );
    CPY_DESC_FIELD_MAC(trgDescrPtr,srcDescrPtr,    ipm                      );
    CPY_DESC_FIELD_MAC(trgDescrPtr,srcDescrPtr,    ipTtiHeaderError         );
    CPY_DESC_FIELD_MAC(trgDescrPtr,srcDescrPtr,    ipHeaderError            );
    CPY_DESC_FIELD_MAC(trgDescrPtr,srcDescrPtr,    flowLabel                );
    CPY_DESC_FIELD_MAC(trgDescrPtr,srcDescrPtr,    ipv6Icmp                 );
    CPY_DESC_FIELD_MAC(trgDescrPtr,srcDescrPtr,    ipv6IcmpType             );
    CPY_DESC_FIELD_MAC(trgDescrPtr,srcDescrPtr,    sip[0]                   );
    CPY_DESC_FIELD_MAC(trgDescrPtr,srcDescrPtr,    sip[1]                   );
    CPY_DESC_FIELD_MAC(trgDescrPtr,srcDescrPtr,    sip[2]                   );
    CPY_DESC_FIELD_MAC(trgDescrPtr,srcDescrPtr,    sip[3]                   );
    CPY_DESC_FIELD_MAC(trgDescrPtr,srcDescrPtr,    dip[0]                   );
    CPY_DESC_FIELD_MAC(trgDescrPtr,srcDescrPtr,    dip[1]                   );
    CPY_DESC_FIELD_MAC(trgDescrPtr,srcDescrPtr,    dip[2]                   );
    CPY_DESC_FIELD_MAC(trgDescrPtr,srcDescrPtr,    dip[3]                   );
    CPY_DESC_FIELD_MAC(trgDescrPtr,srcDescrPtr,    ipXMcLinkLocalProt       );
    CPY_DESC_FIELD_MAC(trgDescrPtr,srcDescrPtr,    solicitationMcastMsg     );
    CPY_DESC_FIELD_MAC(trgDescrPtr,srcDescrPtr,    mpls                     );
    CPY_DESC_FIELD_MAC(trgDescrPtr,srcDescrPtr,    label[0]                 );
    CPY_DESC_FIELD_MAC(trgDescrPtr,srcDescrPtr,    exp[0]                   );
    CPY_DESC_FIELD_MAC(trgDescrPtr,srcDescrPtr,    ttlArr[0]                );
    CPY_DESC_FIELD_MAC(trgDescrPtr,srcDescrPtr,    label[1]                 );
    CPY_DESC_FIELD_MAC(trgDescrPtr,srcDescrPtr,    exp[1]                   );
    CPY_DESC_FIELD_MAC(trgDescrPtr,srcDescrPtr,    ttlArr[1]                );
    CPY_DESC_FIELD_MAC(trgDescrPtr,srcDescrPtr,    label[2]                 );
    CPY_DESC_FIELD_MAC(trgDescrPtr,srcDescrPtr,    exp[2]                   );
    CPY_DESC_FIELD_MAC(trgDescrPtr,srcDescrPtr,    ttlArr[2]                );
    CPY_DESC_FIELD_MAC(trgDescrPtr,srcDescrPtr,    numOfLabels              );
    CPY_DESC_FIELD_MAC(trgDescrPtr,srcDescrPtr,    protOverMpls             );
    CPY_DESC_FIELD_MAC(trgDescrPtr,srcDescrPtr,    mim                      );
    CPY_DESC_FIELD_MAC(trgDescrPtr,srcDescrPtr,    iUp                      );
    CPY_DESC_FIELD_MAC(trgDescrPtr,srcDescrPtr,    iDp                      );
    CPY_DESC_FIELD_MAC(trgDescrPtr,srcDescrPtr,    iRes1                    );
    CPY_DESC_FIELD_MAC(trgDescrPtr,srcDescrPtr,    iRes2                    );
    CPY_DESC_FIELD_MAC(trgDescrPtr,srcDescrPtr,    iSid                     );
    CPY_DESC_FIELD_MAC(trgDescrPtr,srcDescrPtr,    ipProt                   );
    CPY_DESC_FIELD_MAC(trgDescrPtr,srcDescrPtr,    l4SrcPort                );
    CPY_DESC_FIELD_MAC(trgDescrPtr,srcDescrPtr,    l4DstPort                );
    CPY_DESC_FIELD_MAC(trgDescrPtr,srcDescrPtr,    l23HeaderSize            );
    CPY_DESC_FIELD_MAC(trgDescrPtr,srcDescrPtr,    pktHash                  );
    CPY_DESC_FIELD_MAC(trgDescrPtr,srcDescrPtr,    ipv4FragmentOffset       );
    CPY_DESC_FIELD_MAC(trgDescrPtr,srcDescrPtr,    ipv4Fragmented           );

    CPY_DESC_FIELD_MAC(trgDescrPtr,srcDescrPtr,    mplsLabelValueForHash[0]       );
    CPY_DESC_FIELD_MAC(trgDescrPtr,srcDescrPtr,    mplsLabelValueForHash[1]       );
    CPY_DESC_FIELD_MAC(trgDescrPtr,srcDescrPtr,    mplsLabelValueForHash[2]       );
    CPY_DESC_FIELD_MAC(trgDescrPtr,srcDescrPtr,    mplsLabelValueForHash[3]       );
    CPY_DESC_FIELD_MAC(trgDescrPtr,srcDescrPtr,    mplsLabelValueForHash[4]       );

    scibAccessUnlock();
}

/**
* @internal snetBc3EDsaTagHashGet function
* @endinternal
*
* @brief   Extract Hash from FORWARD eDSA tag
*/
static GT_VOID snetBc3EDsaTagHashGet
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32   mtag[/*4*/]
)
{
    DECLARE_FUNC_NAME(snetBc3EDsaTagHashGet);

    if (SMEM_CHT_IS_SIP5_20_GET(devObjPtr) == 0)
    {
        __LOG(("snetBc3EDsaTagHashGet 'hash in eDsa' not supported for current device \n"));
        return;
    }

    /* word 0 */
    __LOG_PARAM_WITH_NAME("DSA Packet Hash[1:0]",SMEM_U32_GET_FIELD(mtag[0], 17, 1));
    SMEM_U32_SET_FIELD(descrPtr->pktHash,0,1,
                       SMEM_U32_GET_FIELD(mtag[0], 17, 1));/* Hash[0] */
    /* word 2 */
    __LOG_PARAM_WITH_NAME("DSA Packet Hash[1:1]",SMEM_U32_GET_FIELD(mtag[2], 13, 1));
    SMEM_U32_SET_FIELD(descrPtr->pktHash,1,1,
                       SMEM_U32_GET_FIELD(mtag[2], 13, 1));/* Hash[1] */
    __LOG_PARAM_WITH_NAME("DSA Packet Hash[2:2]",SMEM_U32_GET_FIELD(mtag[2], 19, 2));
    SMEM_U32_SET_FIELD(descrPtr->pktHash,2,2,
                       SMEM_U32_GET_FIELD(mtag[2], 19, 2));/* Hash[3:2] */

    /* word 3 */
    if(descrPtr->useVidx == 0)
    {
        __LOG_PARAM_WITH_NAME("DSA Packet Hash[4:2]",SMEM_U32_GET_FIELD(mtag[3], 5, 2));
        SMEM_U32_SET_FIELD(descrPtr->pktHash,4,2,
                           SMEM_U32_GET_FIELD(mtag[3], 5, 2));/* Hash[5:4] */
    }
    else
    {
        __LOG_PARAM_WITH_NAME("DSA Packet Hash[4:2]",SMEM_U32_GET_FIELD(mtag[3], 4, 2));
        SMEM_U32_SET_FIELD(descrPtr->pktHash,4,2,
                           SMEM_U32_GET_FIELD(mtag[3], 4, 2));/* Hash[5:4] */
    }
}

/**
* @internal snetChtDsaTag4WordsTagParsing function
* @endinternal
*
* @brief   Parsing of Marvell tag of 4 words. extra/update parse according to 4
*         words DSA tag format
*         NOTE: words 0,1 already parsed before calling this function.
*/
static GT_VOID snetChtDsaTag4WordsTagParsing
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32   mtag[/*4*/]
)
{
    DECLARE_FUNC_NAME(snetChtDsaTag4WordsTagParsing);

    switch(descrPtr->incomingMtagCmd)
    {
        case SKERNEL_MTAG_CMD_FORWARD_E:
            /**********/
            /* word 1 */
            /**********/

            if(descrPtr->origIsTrunk == 0)
            {
                /* Src ePort[6] */
                SMEM_U32_SET_FIELD(descrPtr->origSrcEPortOrTrnk,6,1,
                    SMEM_U32_GET_FIELD(mtag[1],30,1));
            }

            if(descrPtr->useVidx == 0)
            {
                /*TrgPort[0..6]*/
                __LOG(("TrgPort[0..6]"));
                descrPtr->eArchExtInfo.trgPhyPort = SMEM_U32_GET_FIELD(mtag[1],5,7);

                descrPtr->trgEPort = SNET_CHT_NULL_PORT_CNS;/* not valid yet */
            }

            /**********/
            /* word 2 */
            /**********/
            if(descrPtr->useVidx == 0)
            {
                /*Indicates whether the target physical port field <TrgPort> in this DSA tag is valid*/
                descrPtr->eArchExtInfo.isTrgPhyPortValid = SMEM_U32_GET_FIELD(mtag[2],29,1);
                /*TrgPort[7]*/
                SMEM_U32_SET_FIELD(descrPtr->eArchExtInfo.trgPhyPort,7,1,
                    SMEM_U32_GET_FIELD(mtag[2],28,1));
            }

            SMEM_U32_SET_FIELD(descrPtr->sstId,5,7,
                SMEM_U32_GET_FIELD(mtag[2],21,7));

            if(SMEM_CHT_IS_SIP5_20_GET(devObjPtr))
            {
                /* 5 more bits */
                /* other 2 bits for Hash[3:2] see function snetBc3EDsaTagHashGet(...) */
                SMEM_U32_SET_FIELD(descrPtr->srcDev,5,5,
                    SMEM_U32_GET_FIELD(mtag[2],14,5));
            }
            else
            {
                /* 7 more bits */
                SMEM_U32_SET_FIELD(descrPtr->srcDev,5,7,
                    SMEM_U32_GET_FIELD(mtag[2],14,7));
            }
            if(descrPtr->origIsTrunk)
            {
                SMEM_U32_SET_FIELD(descrPtr->origSrcEPortOrTrnk,7,5,
                    SMEM_U32_GET_FIELD(mtag[2],3,5));
            }
            else
            {
                SMEM_U32_SET_FIELD(descrPtr->origSrcEPortOrTrnk,7,10,
                    SMEM_U32_GET_FIELD(mtag[2],3,10));
            }
            /*The TPID index of Tag0*/
            descrPtr->tpidIndex[SNET_CHT_TAG_0_INDEX_CNS] = SMEM_U32_GET_FIELD(mtag[2],0,3);

            /**********/
            /* word 3 */
            /**********/
            SMEM_U32_SET_FIELD(descrPtr->eVid,12,4,
                SMEM_U32_GET_FIELD(mtag[3],27,4));

            /*Indicates if the packet was received tagged with tag1*/
            descrPtr->tagSrcTagged[SNET_CHT_TAG_1_INDEX_CNS] = SMEM_U32_GET_FIELD(mtag[3],26,1);
            /* When both <SrcTagged / Tag0SrcTagged> = 1 (which is Tag0 SrcTagged) AND <Tag1 SrcTagged> = 1,
               indicating that the packet was received tagged with both Tag0 and Tag1, this bit indicates
               which of these two tags is the outer tag */
            descrPtr->tag0IsOuterTag = SMEM_U32_GET_FIELD(mtag[3],25,1);

            if(descrPtr->useVidx)
            {
                SMEM_U32_SET_FIELD(descrPtr->eVidx,12,4,
                    SMEM_U32_GET_FIELD(mtag[3],20,4));


                descrPtr->eArchExtInfo.origSrcPhyIsTrunk   = SMEM_U32_GET_FIELD(mtag[3],19,1);
                if(descrPtr->eArchExtInfo.origSrcPhyIsTrunk)
                {
                    /* trunkId */
                    descrPtr->eArchExtInfo.origSrcPhyPortTrunk = SMEM_U32_GET_FIELD(mtag[3],7,12);
                }
                else
                {
                    /* port */
                    /* since the field is 12 bits in the DSA it is not set to only 8 bits
                       of physical port ! all 12 bits allowed !!!

                       verified with GM BC3
                    */
                    descrPtr->eArchExtInfo.origSrcPhyPortTrunk = SMEM_U32_GET_FIELD(mtag[3],7,12);
                }
                __LOG(("The fields <origSrcPhyIsTrunk>[%d] , <origSrcPhyPortTrunk>[%d] parsed from the DSA tag \n",
                   descrPtr->eArchExtInfo.origSrcPhyIsTrunk,
                   descrPtr->eArchExtInfo.origSrcPhyPortTrunk));
                /* Indicates to the remote device whether to perform VIDX MC source filtering based on <OrigSrcPhyPort/Trunk>
                    0 = Do not perform VIDX physical source filtering
                    1 = Perform VIDX physical source filtering */
                descrPtr->eArchExtInfo.phySrcMcFilterEn = SMEM_U32_GET_FIELD(mtag[3],6,1);

            }
            else
            {
                descrPtr->eArchExtInfo.phySrcMcFilterEn = 0;
                descrPtr->trgEPort = SMEM_U32_GET_FIELD(mtag[3],7,17);
                if(SMEM_CHT_IS_SIP5_20_GET(devObjPtr) == 0)
                {
                    SMEM_U32_SET_FIELD(descrPtr->trgDev,5,7,
                        SMEM_U32_GET_FIELD(mtag[3],0,7));
                }
                else
                {
                    /* the trgDev with only 10 bits , the other 2 bits are in 'hash' bits
                       see function snetBc3EDsaTagHashGet */
                    SMEM_U32_SET_FIELD(descrPtr->trgDev,5,5,
                        SMEM_U32_GET_FIELD(mtag[3],0,5));
                }
            }

            snetBc3EDsaTagHashGet(devObjPtr,descrPtr,mtag);

            if(SMEM_CHT_IS_SIP6_GET(devObjPtr))
            {
                /* get single bit from the eDSA (reserved bit in legacy devices) */
                /* NOTE: this value may be restored to 0 ... see use of xxx*/
                descrPtr->skipFdbSaLookup = SMEM_U32_GET_FIELD(mtag[2],30,1);
            }


            break;

        case SKERNEL_MTAG_CMD_FROM_CPU_E:
            /**********/
            /* word 0 */
            /**********/
            /* NOTE: parsed from the DSA even if descrPtr->useVidx == 0 !!! */
            descrPtr->tagSrcTagged[SNET_CHT_TAG_0_INDEX_CNS] =
                SMEM_U32_GET_FIELD(mtag[0], 29, 1);
            if(descrPtr->useVidx)
            {
                /* this field is not relevant when useVidx = 1 */
                descrPtr->trgTagged = 0;
            }

            /**********/
            /* word 1 */
            /**********/
            if(descrPtr->useVidx == 0)
            {
                /*TrgPort[0..5]*/
                descrPtr->eArchExtInfo.trgPhyPort = descrPtr->trgEPort;

                /*TrgPort[5..6]*/
                SMEM_U32_SET_FIELD(descrPtr->eArchExtInfo.trgPhyPort,5,2,
                    SMEM_U32_GET_FIELD(mtag[1],10,2));
            }

            /**********/
            /* word 2 */
            /**********/
            if(descrPtr->useVidx == 0)
            {
                /*Indicates whether the target physical port field <TrgPort> in this DSA tag is valid*/
                descrPtr->eArchExtInfo.isTrgPhyPortValid = SMEM_U32_GET_FIELD(mtag[2],29,1);
                /*TrgPort[7]*/
                SMEM_U32_SET_FIELD(descrPtr->eArchExtInfo.trgPhyPort,7,1,
                    SMEM_U32_GET_FIELD(mtag[2],28,1));
            }

            SMEM_U32_SET_FIELD(descrPtr->sstId,5,7,
                SMEM_U32_GET_FIELD(mtag[2],21,7));

            SMEM_U32_SET_FIELD(descrPtr->srcDev,5,7,
                SMEM_U32_GET_FIELD(mtag[2],14,7));

            if(descrPtr->useVidx)
            {
                if(descrPtr->excludeIsTrunk)
                {
                    SMEM_U32_SET_FIELD(descrPtr->excludedTrunk,7,5,
                        SMEM_U32_GET_FIELD(mtag[2],3,5));
                }
                else /*excluded port */
                {
                    SMEM_U32_SET_FIELD(descrPtr->excludedPort,6,11,
                        SMEM_U32_GET_FIELD(mtag[2],3,11));
                }
            }

            /*The TPID index of Tag0*/
            descrPtr->tpidIndex[SNET_CHT_TAG_0_INDEX_CNS] = SMEM_U32_GET_FIELD(mtag[2],0,3);

            /**********/
            /* word 3 */
            /**********/
            SMEM_U32_SET_FIELD(descrPtr->eVid,12,4,
                SMEM_U32_GET_FIELD(mtag[3],27,4));

            /* NOTE: parsed from the eDSA even if descrPtr->useVidx == 0 !!! */
            /*Indicates if the packet was received tagged with tag1*/
            descrPtr->tagSrcTagged[SNET_CHT_TAG_1_INDEX_CNS] = SMEM_U32_GET_FIELD(mtag[3],26,1);
            /* When both <SrcTagged / Tag0SrcTagged> = 1 (which is Tag0 SrcTagged) AND <Tag1 SrcTagged> = 1,
               indicating that the packet was received tagged with both Tag0 and Tag1, this bit indicates
               which of these two tags is the outer tag */
            descrPtr->tag0IsOuterTag = SMEM_U32_GET_FIELD(mtag[3],25,1);
            if(descrPtr->useVidx)
            {
                SMEM_U32_SET_FIELD(descrPtr->eVidx,12,4,
                    SMEM_U32_GET_FIELD(mtag[3],20,4));

                if(descrPtr->excludeIsTrunk == 0)
                {
                    /* This field determines whether the value in <ExcludedPort /ExcludedePort> contains a 20b
                        ePort number to exclude from an L2MLL multicast group, or a physical 8b port number to
                        exclude from an L2 MC group
                        0 = ePort: The <ExcludedPort /ExcludedePort> field contains a 20b ePort number to exclude
                        1 = Port: The <ExcludedPort /ExcludedePort> field contains an 8b Physical port number to
                        exclude */
                    descrPtr->eArchExtInfo.fromCpu.excludedIsPhyPort =
                        SMEM_U32_GET_FIELD(mtag[3],19,1);

                    SMEM_U32_SET_FIELD(descrPtr->excludedDevice,5,7,
                        SMEM_U32_GET_FIELD(mtag[3],0,7));
                }
            }
            else
            {
                if (SMEM_CHT_IS_SIP6_GET(devObjPtr))
                {
                    /*TrgPort[9:8]*/
                    SMEM_U32_SET_FIELD(descrPtr->eArchExtInfo.trgPhyPort,8,2,
                        SMEM_U32_GET_FIELD(mtag[3],23,2));
                }

                descrPtr->trgEPort = SMEM_U32_GET_FIELD(mtag[3],7,17);
                SMEM_U32_SET_FIELD(descrPtr->trgDev,5,7,
                    SMEM_U32_GET_FIELD(mtag[3],0,7));
            }

            break;
        case SKERNEL_MTAG_CMD_TO_CPU_E:

            /**********/
            /* word 1 */
            /**********/
            /*SrcPort[6]/TrgPort[6]/SrcTrunk[6]*/
            SMEM_U32_SET_FIELD(descrPtr->srcTrgPhysicalPort,6,1,
                SMEM_U32_GET_FIELD(mtag[1],11,1));

            /**********/
            /* word 2 */
            /**********/
            descrPtr->eArchExtInfo.packetIsTT = SMEM_U32_GET_FIELD(mtag[2],25,1);

            /*TrgPort[7]*/
            SMEM_U32_SET_FIELD(descrPtr->srcTrgPhysicalPort,7,1,
                SMEM_U32_GET_FIELD(mtag[2],20,1));

            if(descrPtr->origIsTrunk)
            {
                descrPtr->origSrcEPortOrTrnk = SMEM_U32_GET_FIELD(mtag[2],3,12);
                descrPtr->eArchExtInfo.srcTrgEPort = SMEM_U32_GET_FIELD(mtag[2],3,12);
            }
            else
            {
                descrPtr->eArchExtInfo.srcTrgEPort = SMEM_U32_GET_FIELD(mtag[2],3,17);

                descrPtr->origSrcEPortOrTrnk = (descrPtr->srcTrg == 0) ?
                        descrPtr->eArchExtInfo.srcTrgEPort :
                        0;
            }

            /*The TPID index of Tag0*/
            descrPtr->tpidIndex[SNET_CHT_TAG_0_INDEX_CNS] = SMEM_U32_GET_FIELD(mtag[2],0,3);

            /**********/
            /* word 3 */
            /**********/
            SMEM_U32_SET_FIELD(descrPtr->eVid,12,4,
                SMEM_U32_GET_FIELD(mtag[3],27,4));

            descrPtr->flowId = SMEM_U32_GET_FIELD(mtag[3],7,20);
            if(descrPtr->eArchExtInfo.packetIsTT)
            {
                descrPtr->tunnelTerminationOffset = descrPtr->flowId;
                descrPtr->flowId = 0;
            }


            /*srcTrgDev[11:5]*/
            SMEM_U32_SET_FIELD(descrPtr->srcTrgDev,5,7,
                SMEM_U32_GET_FIELD(mtag[3],0,7));

            if (descrPtr->srcTrg == 1)
            {
                descrPtr->trgDev = descrPtr->srcTrgDev;
            }

            /*TrgPort*/
            descrPtr->eArchExtInfo.trgPhyPort = (descrPtr->srcTrg == 1) /*target*/?
                                  descrPtr->srcTrgPhysicalPort :
                                  SNET_CHT_NULL_PORT_CNS;

            descrPtr->trgEPort  = descrPtr->eArchExtInfo.trgPhyPort;/*Don't care (assign same logic as used for <TrgPort>)*/

            descrPtr->srcDev = descrPtr->srcTrgDev;
            break;

        case SKERNEL_MTAG_CMD_TO_TRG_SNIFFER_E:
            /**********/
            /* word 1 */
            /**********/
            /*TrgPort[6]/SrcPort[6]*/
            SMEM_U32_SET_FIELD(descrPtr->srcTrgPhysicalPort,6,1,
                SMEM_U32_GET_FIELD(mtag[1],11,1));

            /*Analyzer_UseVIDX*/
            descrPtr->useVidx =
                SMEM_U32_GET_FIELD(mtag[1],26,1);

            if(descrPtr->useVidx == 0)
            {
                /*Analyzer_isTrgPhyPortValid*/
                descrPtr->eArchExtInfo.isTrgPhyPortValid =
                    SMEM_U32_GET_FIELD(mtag[1],27,1);
            }
            /*Analyzer_TrgDev*/
            descrPtr->trgDev =
                SMEM_U32_GET_FIELD(mtag[1],14,12);
            /*Analyzer_TrgPort*/
            descrPtr->eArchExtInfo.trgPhyPort =
                SMEM_U32_GET_FIELD(mtag[1],0,8);

            /**********/
            /* word 2 */
            /**********/
            /*TrgPort[7]/SrcPort[7]*/
            SMEM_U32_SET_FIELD(descrPtr->srcTrgPhysicalPort,7,1,
                SMEM_U32_GET_FIELD(mtag[2],20,1));

            descrPtr->eArchExtInfo.srcTrgEPort = SMEM_U32_GET_FIELD(mtag[2],3,17);

            /*The TPID index of Tag0*/
            descrPtr->tpidIndex[SNET_CHT_TAG_0_INDEX_CNS] = SMEM_U32_GET_FIELD(mtag[2],0,3);

            /**********/
            /* word 3 */
            /**********/
            /* next note from the eBook 1.7 */
            /* NOTE: In TO_ANALYZER eDSA tag, instead of setting the eVLAN, the header alteration
                sets 12 bits Tag0 VID. eVLAN[15:12] bits are set in this tag as well, but are not
                used by the ingress assignment

                descrPtr->eVid = {4'b0, DSA<VID>}*/
            /*SMEM_U32_SET_FIELD(descrPtr->eVid,12,4,
                SMEM_U32_GET_FIELD(mtag[3],27,4));*/


            if(descrPtr->useVidx == 0)
            {
                descrPtr->trgEPort =
                    SMEM_U32_GET_FIELD(mtag[3],7,17);
            }
            else
            {
                descrPtr->eVidx =
                    SMEM_U32_GET_FIELD(mtag[3],7,16);
            }

            /*TrgDev[11:5]/SrcDev[11:5]*/
            SMEM_U32_SET_FIELD(descrPtr->srcTrgDev,5,7,
                SMEM_U32_GET_FIELD(mtag[3],0,7));

            /* next fields not used any more in sip5 */
            {  SIM_TBD_BOOKMARK /* sip 5 : need to remove use of those */
                descrPtr->eArchExtInfo.toTargetSniffInfo.sniffUseVidx = descrPtr->useVidx;
                descrPtr->eArchExtInfo.toTargetSniffInfo.sniffisTrgPhyPortValid = descrPtr->eArchExtInfo.isTrgPhyPortValid;
                descrPtr->sniffTrgDev = descrPtr->trgDev;
                descrPtr->sniffTrgPort = descrPtr->eArchExtInfo.trgPhyPort;
                descrPtr->eArchExtInfo.toTargetSniffInfo.sniffTrgEPort = descrPtr->trgEPort;
                descrPtr->eArchExtInfo.toTargetSniffInfo.sniffEVidx = descrPtr->eVidx;
            }

            descrPtr->srcDev = descrPtr->srcTrgDev;
            break;
        default:
            skernelFatalError("snetChtDsaTag4WordsTagParsing: should not happen \n");
    }
}

/**
* @internal snetChtIngressIsLoopedAndDropOnSource function
* @endinternal
*
* @brief   check the 'is looped' and 'drop on source' for packets that came with DSA tag.
*/
static GT_VOID snetChtIngressIsLoopedAndDropOnSource
(
    IN  SKERNEL_DEVICE_OBJECT           *devObjPtr,
    IN  SKERNEL_FRAME_CHEETAH_DESCR_STC *descrPtr
)
{
    DECLARE_FUNC_NAME(snetChtIngressIsLooped);

    GT_BIT  fastStackEn;
    SKERNEL_PORTS_BMP_STC   portsBmp;
    GT_BIT ingressPortIsLooped;
    GT_U32  *regPtr;
    GT_BIT  final_pktIsLooped = 0;
    GT_BIT  final_dropOnSource = 0;
    GT_BIT srcDevIsOwn = SKERNEL_IS_MATCH_DEVICES_MAC(descrPtr->srcDev, descrPtr->ownDev,
                                         devObjPtr->dualDeviceIdEnable.tti);

    /*Enable for fast stack recovery.*/
    smemRegFldGet(devObjPtr, SMEM_CHT3_FAST_STACK_REG(devObjPtr) , 0, 1, &fastStackEn);

    __LOG_PARAM(fastStackEn);
    __LOG_PARAM(descrPtr->pktIsLooped);
    __LOG_PARAM(descrPtr->dropOnSource);
    __LOG_PARAM(srcDevIsOwn);


    if(fastStackEn == 0)
    {
        __LOG(("TTI global setting of 'fast stack recovery' is disabled : pktIsLooped,dropOnSource considered 0 \n"
               "(regardless on those bits in the ingress DSA tag) \n"));

        goto  final_pktIsLooped_lbl;
    }

    if(descrPtr->dropOnSource && srcDevIsOwn)
    {
        __LOG(("WARNING : packet returned to source device with drop on source , meaning it got here for the second time ! \n"
               "need to drop it \n"));

        if(devObjPtr->errata.fastStackFailover_Drop_on_source == 0)
        {
            /*assigned a HARD_DROP packet*/
            snetChtIngressCommandAndCpuCodeResolution(devObjPtr,descrPtr,
                                                      descrPtr->packetCmd,
                                                      SKERNEL_EXT_PKT_CMD_HARD_DROP_E,
                                                      descrPtr->cpuCode,
                                                      0,/* unknown drop code */
                                                      SNET_CHEETAH_ENGINE_UNIT_TTI_E,
                                                      GT_TRUE);/* not care */
        }
        else
        {
            __LOG(("WARNING : ERRATA : Fast Stack Failover - drop_on_source packets not dropped \n"));
        }
    }

    if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        /* bit per ingress physical port , in per port table entry */
        ingressPortIsLooped =
            SMEM_LION3_TTI_PHYSICAL_PORT_ENTRY_FIELD_GET(devObjPtr,descrPtr,
                SMEM_LION3_TTI_PHYSICAL_PORT_TABLE_FIELDS_PORT_IS_LOOPED);
    }
    else
    {
        /* bit per ingress physical port , in bmp of ports */
        regPtr = smemMemGet(devObjPtr, SMEM_CHT3_LOOP_PORT_REG(devObjPtr));
        /* fill the bmp of ports */
        SKERNEL_FILL_PORTS_BITMAP_MAC(devObjPtr, &portsBmp, regPtr);

        /* check if the port is set in the bmp */
        ingressPortIsLooped =
            SKERNEL_PORTS_BMP_IS_PORT_SET_MAC(&portsBmp,descrPtr->localDevSrcPort);
    }

    if(ingressPortIsLooped)
    {
        __LOG(("Ingress physical port [%d] set for 'Fast Failover on the ring port' (Port is looped) \n" ,
            descrPtr->localDevSrcPort));

        __LOG((" NOTE: that packets from this ingress port will be directed by the device to the 'backup ring physical port' (defined at txq/egf) \n"));
    }
    else
    {
        __LOG(("Ingress physical port [%d] in not set for 'Fast Failover on the ring port' (Port is NOT looped) \n",
            descrPtr->localDevSrcPort));
    }

    if(descrPtr->pktIsLooped)
    {
        if(ingressPortIsLooped)
        {
            __LOG(("ingress packet came with dsa stated as 'pktIsLooped = 1' , we need to 'terminate the loop' and consider it as 'overcome the stack fail' \n"));

            final_pktIsLooped = 0;
        }
        else
        {
            __LOG(("ingress packet came with dsa stated as 'pktIsLooped = 1' , from non looped port meaning it looking for target device to 'overcome the stack fail' \n"));

            final_pktIsLooped = 1;
        }
    }
    else  /*pktIsLooped = 0*/
    {
        if(ingressPortIsLooped)
        {
            __LOG(("ingress packet considered to start the road to 'overcome the stack fail' \n"));
            final_pktIsLooped = 1;
        }
        else
        {
            __LOG(("ingress packet came from port that not relevant to 'stack fail over' and with 'pktIsLooped = 0' \n"));
            final_pktIsLooped = 0;
        }
    }

final_pktIsLooped_lbl:

    __LOG_PARAM(final_pktIsLooped);

    if(fastStackEn && final_pktIsLooped && (descrPtr->dropOnSource == 0) && srcDevIsOwn)
    {
        final_dropOnSource = 1;
    }
    else
    {
        final_dropOnSource = descrPtr->dropOnSource;
    }

    __LOG_PARAM(final_dropOnSource);

    if(final_pktIsLooped != descrPtr->pktIsLooped)
    {
        __LOG(("logic of pktIsLooped changed value from [%d] to [%d] \n",
            descrPtr->pktIsLooped ,
            final_pktIsLooped));
    }
    else
    {
        __LOG(("logic of pktIsLooped keep value [%d] \n",
            final_pktIsLooped));
    }

    if(final_dropOnSource != descrPtr->dropOnSource)
    {
        __LOG(("logic of dropOnSource changed value from [%d] to [%d] \n",
            descrPtr->dropOnSource ,
            final_dropOnSource));
    }
    else
    {
        __LOG(("logic of dropOnSource keep value [%d] \n",
            final_dropOnSource));
    }

    descrPtr->pktIsLooped  = final_pktIsLooped;
    descrPtr->dropOnSource = final_dropOnSource;

    return;
}

/**
* @internal snetChtMarvellTagParsing_lengthGet function
* @endinternal
*
* @brief   get the length of the DSA tag , and set it in descrPtr->incomingMtagCmd
*       the function called from TTI or RxDma unit.
*       (so it can use descrPtr->incomingMtagCmd)
*   NOTE: should be called only if descrPtr->marvellTagged
*/
static GT_VOID snetChtMarvellTagParsing_lengthGet
(
    IN  SKERNEL_DEVICE_OBJECT           *devObjPtr,
    IN  SKERNEL_FRAME_CHEETAH_DESCR_STC *descrPtr,
    OUT   GT_U32                       *mtagWords  /* ignored if NULL , filled if exists */
)
{
    GT_U32  index = 0;/* dsa tag word index*/
    GT_U32 * mtagPtr; /* pointer to marvell tag */

    descrPtr->marvellTaggedExtended = SKERNEL_EXT_DSA_TAG_1_WORDS_E;
    /* check DSA tag format : TO_CPU ? */
    if((descrPtr->startFramePtr[12] >> 6) == 0)
    {
        if((descrPtr->startFramePtr[13] & 7) == 7 &&
           (descrPtr->startFramePtr[14] & 0x10))
        {
            descrPtr->marvellTaggedExtended = SKERNEL_EXT_DSA_TAG_2_WORDS_E;
        }
    }
    else
    {
        if(descrPtr->startFramePtr[14] & 0x10)
        {
            descrPtr->marvellTaggedExtended = SKERNEL_EXT_DSA_TAG_2_WORDS_E;
        }
    }

    mtagPtr = (GT_U32 *)(&descrPtr->startFramePtr[12]);

    if(mtagWords)
    {

        /* Set init values */
        memset(mtagWords, 0, (SKERNEL_EXT_DSA_TAG_4_WORDS_E + 1) * sizeof(GT_U32));

        mtagWords[index] = SGT_LIB_SWAP_BYTES_AND_WORDS_MAC(*mtagPtr);
    }
    index++;

    if (descrPtr->marvellTaggedExtended != SKERNEL_EXT_DSA_TAG_1_WORDS_E)
    {
        mtagPtr++;
        if(mtagWords)
        {
            mtagWords[index] = SGT_LIB_SWAP_BYTES_AND_WORDS_MAC(*mtagPtr);
        }
        index++;

        if(descrPtr->startFramePtr[12+4] & 0x80)
        {
            mtagPtr++;
            if(mtagWords)
            {
                mtagWords[index] = SGT_LIB_SWAP_BYTES_AND_WORDS_MAC(*mtagPtr);
            }
            index++;

            if(descrPtr->startFramePtr[12+8] & 0x80)
            {
                mtagPtr++;
                if(mtagWords)
                {
                    mtagWords[index] = SGT_LIB_SWAP_BYTES_AND_WORDS_MAC(*mtagPtr);
                }
                index++;

                descrPtr->marvellTaggedExtended = SKERNEL_EXT_DSA_TAG_4_WORDS_E;
            }
            else
            {
                descrPtr->marvellTaggedExtended = SKERNEL_EXT_DSA_TAG_3_WORDS_E;
                skernelFatalError("snetChtMarvellTagParsing_lengthGet: not supported 3 words DSA tag \n");
            }
        }
    }

    return;
}

/**
* @internal snetChtMarvellTagParsing function
* @endinternal
*
* @brief   Parsing of Marvell tag
*/
static GT_VOID snetChtMarvellTagParsing
(
    IN  SKERNEL_DEVICE_OBJECT           *devObjPtr,
    IN  SKERNEL_FRAME_CHEETAH_DESCR_STC *descrPtr,
    IN  INTERNAL_TTI_DESC_INFO_STC      *internalTtiInfoPtr
)
{
    DECLARE_FUNC_NAME(snetChtMarvellTagParsing);

    GT_U32 mtag[4];             /* 4 words DSA tag (in little endian format) */
    GT_U32 * regPtr;            /* register pointer */
    GT_U32  index;/* dsa tag word index*/
    GT_U32  treatMirroredAsIngressMirrored;
    GT_U32  mtagQosProfile = 0;
    GT_U32  word1origSrcEPortSize; /* number of bits in word1 for source port - bits 29, 30 */

    if(devObjPtr->supportEArch)
    {
        /* When enabled, all incoming TO_ANALYZER and TO_CPU DSA tagged packets are treated internally as ingress mirrored
           Internal note: when this field is enabled, the TTI assigns Desc<RxSniff>=1 in TO_ANALYZER, and Desc<SrcTrg>=Src in TO_CPU. */
        smemRegFldGet(devObjPtr, SMEM_XCAT_A1_TTI_UNIT_GLB_CONF_REG(devObjPtr),31,1,&treatMirroredAsIngressMirrored);

        /* for none eDSA tag info , set 'srcTrgPhysicalPort' as 'src' and of local device */
        descrPtr->srcTrgPhysicalPort = descrPtr->localDevSrcPort;
    }
    else
    {
        treatMirroredAsIngressMirrored = 0;
    }

    descrPtr->tag0Ptr =  &descrPtr->startFramePtr[12];
    /* get the length of the DSA tag , and set it in descrPtr->incomingMtagCmd
       the function called from TTI or RxDma unit. */
    snetChtMarvellTagParsing_lengthGet(devObjPtr,descrPtr,&mtag[0]);

    if(simLogIsOpenFlag)
    {
        index = 4 * (descrPtr->marvellTaggedExtended + 1);
        scibAccessLock();

        __LOG(("[%s]",
                    descrPtr->marvellTaggedExtended == SKERNEL_EXT_DSA_TAG_1_WORDS_E ? "SKERNEL_EXT_DSA_TAG_1_WORDS_E" :
                    descrPtr->marvellTaggedExtended == SKERNEL_EXT_DSA_TAG_2_WORDS_E ? "SKERNEL_EXT_DSA_TAG_2_WORDS_E" :
                    descrPtr->marvellTaggedExtended == SKERNEL_EXT_DSA_TAG_3_WORDS_E ? "SKERNEL_EXT_DSA_TAG_3_WORDS_E" :
                    descrPtr->marvellTaggedExtended == SKERNEL_EXT_DSA_TAG_4_WORDS_E ? "SKERNEL_EXT_DSA_TAG_4_WORDS_E" :
                    "unknown DSA length format ?!"
                      ));

        __LOG(("dump the bytes of the DSA : (network order)" ));
        /* dump the bytes of the DSA */
        simLogPacketDump(devObjPtr,GT_TRUE/*ingress*/,
            descrPtr->localDevSrcPort,
            &descrPtr->startFramePtr[12] ,
            index);

        scibAccessUnlock();
    }

    /* TagCommand */
    /* save the DSA incoming command for later use */
    descrPtr->incomingMtagCmd = SMEM_U32_GET_FIELD(mtag[0], 30, 2);

    descrPtr->eVid       = SMEM_U32_GET_FIELD(mtag[0], 0, 12);
    descrPtr->up        = SMEM_U32_GET_FIELD(mtag[0], 13, 3);
    descrPtr->cfidei    = SMEM_U32_GET_FIELD(mtag[0], 16, 1);

    switch(descrPtr->incomingMtagCmd)
    {
        case SKERNEL_MTAG_CMD_FORWARD_E:
            __LOG(("[%s]","SKERNEL_MTAG_CMD_FORWARD_E"));
            descrPtr->packetCmd = SKERNEL_EXT_PKT_CMD_FORWARD_E;

            /* SrcTagged */
            descrPtr->tagSrcTagged[SNET_CHT_TAG_0_INDEX_CNS] = SMEM_U32_GET_FIELD(mtag[0], 29, 1);

            descrPtr->srcDev =
                SMEM_U32_GET_FIELD(mtag[0], 24, 5);
            descrPtr->origIsTrunk =
                SMEM_U32_GET_FIELD(mtag[0], 18, 1);

            if(SKERNEL_IS_LION2_DEV(devObjPtr))
            {
                descrPtr->dsaCoreIdBit2 =
                    SMEM_U32_GET_FIELD(mtag[0], 17, 1);
            }

            descrPtr->origSrcEPortOrTrnk =
                SMEM_U32_GET_FIELD(mtag[0], 19, 5);

            if (descrPtr->marvellTaggedExtended != SKERNEL_EXT_DSA_TAG_1_WORDS_E)
            {
                /* SIP5 devices use two bits [30:29] for source port */
                word1origSrcEPortSize = (SMEM_CHT_IS_SIP5_GET(devObjPtr)) ? 2 : 1;
                descrPtr->origSrcEPortOrTrnk |=
                    SMEM_U32_GET_FIELD(mtag[1], 29, (descrPtr->origIsTrunk ? 2 : word1origSrcEPortSize)) << 5;
                descrPtr->egressFilterRegistered =
                    SMEM_U32_GET_FIELD(mtag[1], 28, 1);
                descrPtr->pktIsLooped =
                    SMEM_U32_GET_FIELD(mtag[1], 26, 1);
                descrPtr->dropOnSource =
                    SMEM_U32_GET_FIELD(mtag[1], 27, 1);
                descrPtr->routed =
                    SMEM_U32_GET_FIELD(mtag[1], 25, 1);
                descrPtr->sstId =
                    SMEM_U32_GET_FIELD(mtag[1], 20, 5);
                descrPtr->useVidx =
                    SMEM_U32_GET_FIELD(mtag[1], 12, 1);
                if (descrPtr->useVidx)
                {
                    descrPtr->eVidx =
                        SMEM_U32_GET_FIELD(mtag[1], 0, 12);
                }
                else
                {
                    descrPtr->trgEPort =
                        SMEM_U32_GET_FIELD(mtag[1], 5, 6);
                    descrPtr->trgDev =
                        SMEM_U32_GET_FIELD(mtag[1], 0, 5);
                }
                /* QoSProfile */
                mtagQosProfile = SMEM_U32_GET_FIELD(mtag[1], 13, 7);
            }
            else
            {
                /* non-extended DSA tag */
                /* the value of 'cpu port' (31) in non extended DSA tag systems
                   is converted to the 'cpu port' (63) of the extended DSA tag
                   system
                */
                if((!descrPtr->origIsTrunk) &&
                    descrPtr->origSrcEPortOrTrnk == (SNET_CHT_CPU_PORT_CNS & 0x1f))
                {
                    /* update the port to be 'cpu port num' ! */
                    descrPtr->origSrcEPortOrTrnk = SNET_CHT_CPU_PORT_CNS;
                }
            }

            descrPtr->eArchExtInfo.srcTrgEPort = descrPtr->origSrcEPortOrTrnk;
            break;
        case SKERNEL_MTAG_CMD_TO_CPU_E:
            __LOG(("[%s]","SKERNEL_MTAG_CMD_TO_CPU_E"));
            descrPtr->packetCmd = SKERNEL_EXT_PKT_CMD_FROM_CSCD_TO_CPU_E;

            if (descrPtr->marvellTaggedExtended == SKERNEL_EXT_DSA_TAG_1_WORDS_E)
            {
                descrPtr->tagSrcTagged[SNET_CHT_TAG_0_INDEX_CNS] =
                    SMEM_U32_GET_FIELD(mtag[0], 29, 1);

                descrPtr->srcTrgDev =
                    SMEM_U32_GET_FIELD(mtag[0], 24, 5);
                descrPtr->origSrcEPortOrTrnk =
                    SMEM_U32_GET_FIELD(mtag[0], 19, 5);
                descrPtr->cpuCode =
                    SMEM_U32_GET_FIELD(mtag[0], 12, 1) |
                    SMEM_U32_GET_FIELD(mtag[0], 16, 3) << 1;
            }
            else
            {
                descrPtr->srcTrg =
                    SMEM_U32_GET_FIELD(mtag[1], 8, 1);

                if(treatMirroredAsIngressMirrored == 1)
                {
                    if(descrPtr->srcTrg == 1)
                    {
                        __LOG(("descrPtr->srcTrg = 1 was changed to 0 because <treatMirroredAsIngressMirrored>"));
                    }
                    descrPtr->srcTrg = 0;
                }

                if (descrPtr->srcTrg == 1)
                {
                    descrPtr->trgTagged =
                        SMEM_U32_GET_FIELD(mtag[0], 29, 1);
                    descrPtr->trgDev =
                        SMEM_U32_GET_FIELD(mtag[0], 24, 5);
                    descrPtr->srcTrgDev = descrPtr->trgDev;
                    descrPtr->trgEPort =
                        SMEM_U32_GET_FIELD(mtag[0], 19, 5);
                    descrPtr->trgEPort |=
                        SMEM_U32_GET_FIELD(mtag[1], 10, 1) << 5;
                }
                else
                {
                    descrPtr->origIsTrunk =
                        SMEM_U32_GET_FIELD(mtag[1], 27, 1);
                    descrPtr->tagSrcTagged[SNET_CHT_TAG_0_INDEX_CNS] =
                        SMEM_U32_GET_FIELD(mtag[0], 29, 1);
                    descrPtr->srcTrgDev =
                        SMEM_U32_GET_FIELD(mtag[0], 24, 5);
                    descrPtr->origSrcEPortOrTrnk =
                        SMEM_U32_GET_FIELD(mtag[0], 19, 5);
                    descrPtr->origSrcEPortOrTrnk |=
                        SMEM_U32_GET_FIELD(mtag[1], 10, 1) << 5;
                    if(descrPtr->origIsTrunk)
                    {
                        descrPtr->origSrcEPortOrTrnk |=
                            SMEM_U32_GET_FIELD(mtag[1], 11, 1) << 6;
                    }
                }

                descrPtr->inDsaPktOrigBC = SMEM_U32_GET_FIELD(mtag[1], 12, 14);
                descrPtr->cpuCode =
                    SMEM_U32_GET_FIELD(mtag[1], 0, 8);

                descrPtr->pktIsLooped =
                    SMEM_U32_GET_FIELD(mtag[1], 28, 1);
                descrPtr->dropOnSource =
                    SMEM_U32_GET_FIELD(mtag[1], 29, 1);

                descrPtr->cfidei =
                    SMEM_U32_GET_FIELD(mtag[1], 30, 1);

                /* DSA_Cmd == TO_CPU */
                descrPtr->sstId = 0;
                descrPtr->sstIdPrecedence = SKERNEL_PRECEDENCE_ORDER_HARD;
            }

            descrPtr->srcDev = descrPtr->srcTrgDev;

            if(devObjPtr->supportEArch)
            {
                if (descrPtr->srcTrg == 1)
                {
                    descrPtr->eArchExtInfo.srcTrgEPort = descrPtr->trgEPort;
                }
                else
                {
                    descrPtr->eArchExtInfo.srcTrgEPort = descrPtr->origSrcEPortOrTrnk;
                }

                descrPtr->srcTrgPhysicalPort = descrPtr->eArchExtInfo.srcTrgEPort;

                /*TrgPort*/
                descrPtr->eArchExtInfo.trgPhyPort = (descrPtr->srcTrg == 1) /*target*/?
                                      descrPtr->eArchExtInfo.srcTrgEPort :
                                      SNET_CHT_NULL_PORT_CNS;

                descrPtr->trgEPort  = descrPtr->eArchExtInfo.trgPhyPort;/*Don't care (assign same logic as used for <TrgPort>)*/

                descrPtr->origSrcEPortOrTrnk = descrPtr->eArchExtInfo.localDevSrcEPort;/* not from the DSA tag */
            }

            break;
        case SKERNEL_MTAG_CMD_FROM_CPU_E:
            __LOG(("[%s]","SKERNEL_MTAG_CMD_FROM_CPU_E"));
            descrPtr->packetCmd = SKERNEL_EXT_PKT_CMD_FROM_CPU_E;

            descrPtr->trgTagged =
                SMEM_U32_GET_FIELD(mtag[0], 29, 1);
            descrPtr->useVidx =
                SMEM_U32_GET_FIELD(mtag[0], 18, 1);
            if (descrPtr->useVidx)
            {
                descrPtr->eVidx =
                    SMEM_U32_GET_FIELD(mtag[0], 19, 10);
            }
            else
            {
                descrPtr->trgDev =
                    SMEM_U32_GET_FIELD(mtag[0], 24, 5);
                descrPtr->trgEPort =
                    SMEM_U32_GET_FIELD(mtag[0], 19, 5);
            }

            descrPtr->qos.fromCpuQos.fromCpuTc = (GT_U8)
                SMEM_U32_GET_FIELD(mtag[0], 17, 1);
            if (descrPtr->marvellTaggedExtended != SKERNEL_EXT_DSA_TAG_1_WORDS_E)
            {
                descrPtr->qos.fromCpuQos.fromCpuTc |=
                   (SMEM_U32_GET_FIELD(mtag[1], 14, 1) << 1 |
                    SMEM_U32_GET_FIELD(mtag[1], 27, 1) << 2);

                descrPtr->qos.fromCpuQos.contolTc =
                    SMEM_U32_GET_FIELD(mtag[1], 29, 1);
                /* DSA_Cmd == FROM_CPU */
                descrPtr->sstId =
                    SMEM_U32_GET_FIELD(mtag[1], 20, 5);
                descrPtr->egressFilterRegistered =
                    SMEM_U32_GET_FIELD(mtag[1], 28, 1);
                descrPtr->srcDev =
                    SMEM_U32_GET_FIELD(mtag[1], 15, 5);
                descrPtr->egressFilterEn =
                    SMEM_U32_GET_FIELD(mtag[1], 30, 1);

                /*save value for the EPCL !! */
                descrPtr->ingressDsa.fromCpu_egressFilterEn = descrPtr->egressFilterEn;

                if (descrPtr->useVidx)
                {
                    descrPtr->excludeIsTrunk =
                        SMEM_U32_GET_FIELD(mtag[1], 11, 1);
                    if(descrPtr->excludeIsTrunk)
                    {
                        descrPtr->excludedTrunk =
                            SMEM_U32_GET_FIELD(mtag[1], 0, 7);

                        descrPtr->mirrorToAllCpus =
                            SMEM_U32_GET_FIELD(mtag[1], 10, 1);
                    }
                    else
                    {
                        if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
                        {
                            /* for non-eDSA the default is 'excludedIsPhyPort = 1' */
                            descrPtr->eArchExtInfo.fromCpu.excludedIsPhyPort = 1;
                        }

                        descrPtr->excludedDevice =
                            SMEM_U32_GET_FIELD(mtag[1], 0, 5);
                        descrPtr->excludedPort =
                            SMEM_U32_GET_FIELD(mtag[1], 5, 6);
                    }

                    descrPtr->eVidx |=
                        SMEM_U32_GET_FIELD(mtag[1], 12, 2) << 10;
                }
                else
                {
                    descrPtr->mailBoxToNeighborCPU =
                        SMEM_U32_GET_FIELD(mtag[1], 13, 1);
                    descrPtr->trgEPort |=
                        SMEM_U32_GET_FIELD(mtag[1], 10, 1) << 5;
                }

                descrPtr->pktIsLooped =
                    SMEM_U32_GET_FIELD(mtag[1], 25, 1);
                descrPtr->dropOnSource =
                    SMEM_U32_GET_FIELD(mtag[1], 26, 1);

                regPtr =
                    smemMemGet(devObjPtr, SMEM_CHT3_FAST_STACK_REG(devObjPtr));
                descrPtr->qos.fromCpuQos.fromCpuDp =
                    (GT_U8) (SMEM_U32_GET_FIELD(regPtr[0], 1, 2));

            }
            break;
        default:    /*SKERNEL_MTAG_CMD_TO_TRG_SNIFFER_E*/
            __LOG(("[%s]","SKERNEL_MTAG_CMD_TO_TRG_SNIFFER_E"));
            descrPtr->packetCmd = SKERNEL_EXT_PKT_CMD_TO_TRG_SNIFFER_E;

            descrPtr->rxSniff =
                SMEM_U32_GET_FIELD(mtag[0], 18, 1);

            if(treatMirroredAsIngressMirrored == 1)
            {
                if(descrPtr->rxSniff == 0)
                {
                    __LOG(("descrPtr->rxSniff = 0 was changed to 1 because <treatMirroredAsIngressMirrored>"));
                }
                descrPtr->rxSniff = 1;
            }

            if (descrPtr->rxSniff)
            {
                descrPtr->trgTagged = 0;
                descrPtr->tagSrcTagged[SNET_CHT_TAG_0_INDEX_CNS] =
                    SMEM_U32_GET_FIELD(mtag[0], 29, 1);
            }
            else
            {
                descrPtr->tagSrcTagged[SNET_CHT_TAG_0_INDEX_CNS] = 0;
                descrPtr->trgTagged =
                    SMEM_U32_GET_FIELD(mtag[0], 29, 1);
            }

            /* for ALL DSA tags */
            descrPtr->srcTrgDev =
                SMEM_U32_GET_FIELD(mtag[0], 24, 5);
            descrPtr->origSrcEPortOrTrnk =
                SMEM_U32_GET_FIELD(mtag[0], 19, 5);
            descrPtr->srcTrgPhysicalPort = descrPtr->origSrcEPortOrTrnk;

            if (descrPtr->marvellTaggedExtended != SKERNEL_EXT_DSA_TAG_1_WORDS_E)
            {
                descrPtr->origSrcEPortOrTrnk |=
                        SMEM_U32_GET_FIELD(mtag[1], 10, 1) << 5;
                descrPtr->srcTrgPhysicalPort = descrPtr->origSrcEPortOrTrnk;

                if (descrPtr->rxSniff == 0)
                {
                    descrPtr->trgDev = descrPtr->srcTrgDev;
                    descrPtr->trgEPort = descrPtr->origSrcEPortOrTrnk;
                }

                descrPtr->pktIsLooped =
                    SMEM_U32_GET_FIELD(mtag[1], 28, 1);
                descrPtr->dropOnSource =
                    SMEM_U32_GET_FIELD(mtag[1], 29, 1);

                /* DSA_Cmd == TO_ANALYSER */
                descrPtr->sstId = 0;
                descrPtr->sstIdPrecedence = SKERNEL_PRECEDENCE_ORDER_HARD;
            }

            if(devObjPtr->supportEArch)
            {
                descrPtr->srcDev = descrPtr->srcTrgDev;
                /*descrPtr->srcDev = descrPtr->ownDev;*//*not from the DSA tag*/
                descrPtr->eArchExtInfo.srcTrgEPort = descrPtr->srcTrgPhysicalPort;
                descrPtr->origIsTrunk = 0;
                descrPtr->origSrcEPortOrTrnk = descrPtr->eArchExtInfo.localDevSrcEPort;/* not from the DSA tag */
                descrPtr->eArchExtInfo.trgPhyPort = 0;
                descrPtr->trgDev = 0;
                descrPtr->useVidx = 0;
                descrPtr->trgEPort = 0;

                descrPtr->eArchExtInfo.isTrgPhyPortValid = 0;
            }
            else
            {
                descrPtr->srcDev = descrPtr->srcTrgDev;
            }

            break;
    }

    /* the per port can FORCE it's sstId on DSA tagged packets */
    __LOG(("the per port can FORCE it's sstId on DSA tagged packets"));
    descrPtr->sstIdPrecedence = SKERNEL_PRECEDENCE_ORDER_SOFT;

    switch(descrPtr->incomingMtagCmd)
    {
        case SKERNEL_MTAG_CMD_FORWARD_E:
        case SKERNEL_MTAG_CMD_FROM_CPU_E:
            descrPtr->eArchExtInfo.trgPhyPort = descrPtr->trgEPort;
            descrPtr->eArchExtInfo.isTrgPhyPortValid = 0;
            break;
        case SKERNEL_MTAG_CMD_TO_CPU_E:
            descrPtr->eArchExtInfo.trgPhyPort = descrPtr->trgEPort;
            break;
        default:
            break;
    }

    if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        /* set values from the DSA tag */
        /* values maybe overridden by EDSA useVidx=1 */
        descrPtr->eArchExtInfo.origSrcPhyIsTrunk   = descrPtr->origIsTrunk;
        descrPtr->eArchExtInfo.origSrcPhyPortTrunk = descrPtr->origSrcEPortOrTrnk;
        __LOG(("The fields <origSrcPhyIsTrunk>[%d] from <origIsTrunk>,\n"
               "<origSrcPhyPortTrunk>[%d] from <origSrcEPortOrTrnk> parsed from the DSA tag \n",
               descrPtr->eArchExtInfo.origSrcPhyIsTrunk,
               descrPtr->eArchExtInfo.origSrcPhyPortTrunk));
    }

    if(descrPtr->marvellTaggedExtended == SKERNEL_EXT_DSA_TAG_4_WORDS_E &&
       devObjPtr->supportEArch)
    {
        /* extra/update parse according to 4 words DSA tag format */
        __LOG(("extra/update parse according to 4 words DSA tag format"));
        SIM_LOG_PACKET_DESCR_SAVE
        snetChtDsaTag4WordsTagParsing(devObjPtr,descrPtr,mtag);

        /* fixing descrPtr->eArchExtInfo.trgPhyPort when isTrgPhyPortValid == 0 */

        if (descrPtr->eArchExtInfo.isTrgPhyPortValid == 0)
        {
            descrPtr->eArchExtInfo.trgPhyPort = SNET_CHT_NULL_PORT_CNS;
        }

        SIM_LOG_PACKET_DESCR_COMPARE("snetChtDsaTag4WordsTagParsing");

        /* force reduced number of bits from the eDSA (like actual device !!!) */
        REDUCE_NUM_BIT_FROM_EDSA_MAC(devObjPtr,descrPtr->origSrcEPortOrTrnk,ePort);
        REDUCE_NUM_BIT_FROM_EDSA_MAC(devObjPtr,descrPtr->trgEPort,ePort);
        REDUCE_NUM_BIT_FROM_EDSA_MAC(devObjPtr,descrPtr->eArchExtInfo.srcTrgEPort,ePort);
        REDUCE_NUM_BIT_FROM_EDSA_MAC(devObjPtr,descrPtr->eArchExtInfo.toTargetSniffInfo.sniffTrgEPort,ePort);

        REDUCE_NUM_BIT_FROM_EDSA_MAC(devObjPtr,descrPtr->eVid,eVid);

        REDUCE_NUM_BIT_FROM_EDSA_MAC(devObjPtr,descrPtr->eVidx,eVidx);
        REDUCE_NUM_BIT_FROM_EDSA_MAC(devObjPtr,descrPtr->eArchExtInfo.toTargetSniffInfo.sniffEVidx,eVidx);

    }
    else if(devObjPtr->supportEArch) /* non eDSA -- no valid trg physical port info (info is eport: descrPtr->trgEPort)*/
    {
        descrPtr->eArchExtInfo.isTrgPhyPortValid = 0;
        descrPtr->eArchExtInfo.trgPhyPort = SNET_CHT_NULL_PORT_CNS;
    }

    /* save DSA info */
    descrPtr->ingressDsa.origIsTrunk        = descrPtr->origIsTrunk;
    descrPtr->ingressDsa.origSrcEPortOrTrnk = descrPtr->origSrcEPortOrTrnk;
    descrPtr->ingressDsa.srcDev             = descrPtr->srcDev;
    descrPtr->ingressDsa.qosProfile         = mtagQosProfile;
    descrPtr->ingressDsa.srcId              = descrPtr->sstId;

    descrPtr->ingressDsa.dsaWords[0] = mtag[0];
    descrPtr->ingressDsa.dsaWords[1] = mtag[1];
    descrPtr->ingressDsa.dsaWords[2] = mtag[2];
    descrPtr->ingressDsa.dsaWords[3] = mtag[3];

}

/**
* @internal snetChtVlanQosProfAssign function
* @endinternal
*
* @brief   VLAN and QoS/CoS assign
*/
static GT_VOID snetChtVlanQosProfAssign
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    GT_U32 etherType,
    IN SKERNEL_L2_ENCAPSULATION_TYPE_ENT l2encapsul,
    IN GT_U32 * portVlanCfgEntryPtr
)
{
    DECLARE_FUNC_NAME(snetChtVlanQosProfAssign);

    GT_U8 pvidAssign;               /* force PVID assign */
    GT_U32 pvidMode;                /* PVID assignment mode */
    GT_U32 pvidPrec;                /* PVID Precedence */
    GT_U32 protBaseQosEn;           /* ProtBased QoS Enabled */
    GT_U32 protBaseVlanEn;          /* ProtBased VLAN Enabled */
    GT_U32 regAddress;              /* register address */
    GT_U32 regValue;                /* register value */
    GT_U32  *ttiPreTtiLookupIngressEPortTablePtr = NULL;/* pointer to the entry in : tti - Pre-TTI Lookup Ingress ePort Table  */
    GT_BOOL bypassVlanAssignment;/* do we bypass vlan assignment ? support fix for CQ#129303 */

    /* tti - Pre-TTI Lookup Ingress ePort Table */
    ttiPreTtiLookupIngressEPortTablePtr = descrPtr->eArchExtInfo.ttiPreTtiLookupIngressEPortTablePtr;

    if(ttiPreTtiLookupIngressEPortTablePtr)
    {
        portVlanCfgEntryPtr = NULL;
    }

    if((descrPtr->packetCmd == SKERNEL_EXT_PKT_CMD_TO_TRG_SNIFFER_E) ||
       (descrPtr->packetCmd == SKERNEL_EXT_PKT_CMD_FROM_CSCD_TO_CPU_E) ||
       (descrPtr->packetCmd == SKERNEL_EXT_PKT_CMD_FROM_CPU_E))
    {
        /* non forward commands - no vlan re-assignment (with DSA to_cpu/to_analyzer/from_cpu) */
        __LOG(("non forward commands - no vlan re-assignment (with DSA to_cpu/to_analyzer/from_cpu)"));
        bypassVlanAssignment = GT_TRUE;
    }
    else  /* Packets received on DSA-tagged ports are subject to the
            VLAN assignment mechanisms only if the DSA tag command is FORWARD*/
    {
        if (descrPtr->eVid == 0 &&
                TAG0_EXIST_MAC(descrPtr)) /* tag 0 exists -- (DSA tags considered tag 0) */
        {
            descrPtr->srcPriorityTagged = 1;
        }

        bypassVlanAssignment = GT_FALSE;
    }

    if (bypassVlanAssignment == GT_FALSE)
    {
        /* Process VLAN assign */
        __LOG(("Process VLAN assign"));
        if (descrPtr->nestedVlanAccessPort == 0)
        {
            __LOG(("NOT nested vlan \n"));
            /* Regular port */
            if(ttiPreTtiLookupIngressEPortTablePtr)
            {
                pvidMode =
                    SMEM_LION3_TTI_DEFAULT_EPORT_ENTRY_FIELD_GET(devObjPtr, descrPtr,
                        SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_P_EVLAN_MODE);
            }
            else
            {
                if(portVlanCfgEntryPtr == NULL)
                {
                    /* fix for coverity warning "FORWARD_NULL" */
                    skernelFatalError("snetChtVlanQosProfAssign: invalid table entry pointer\n");
                }

                pvidMode = SNET_CHT_PORT_VLAN_QOS_PVID_MODE_GET_MAC(devObjPtr,
                                                                portVlanCfgEntryPtr);
            }

            __LOG(("pvidMode [%d] \n",
                          pvidMode));

            if (pvidMode == 0)
            {
                /* Not Force PVID assign */

                /* if DSA state that remote device device got the packet ''
                    then we assign it pvid as if came untagged from local port */
                if(descrPtr->centralizedChassisModeEn &&
                   (0 == descrPtr->tagSrcTagged[SNET_CHT_TAG_0_INDEX_CNS]))
                {
                    __LOG(("centralizedChassisModeEn : packet received untagged on line card port - treat it as if it was received untagged locally \n"));
                    pvidAssign = 1;
                }
                else
                if ((0 == TAG0_EXIST_MAC(descrPtr)) || descrPtr->srcPriorityTagged)
                {
                    /* tag 0 not exists or priority tag0 - assign PVID */
                    if(0 == TAG0_EXIST_MAC(descrPtr))
                    {
                        __LOG(("tag 0 not exists - assign PVID \n"));
                    }
                    else
                    {
                        __LOG(("tag 0 is 'priority tagged' - assign PVID \n"));
                    }
                    pvidAssign = 1;
                }
                else
                {
                    /* Vid is already in descriptor therefore descrPtr->preserveVid = 0  */
                    __LOG(("No PVID assign \n"));
                    descrPtr->preserveVid = 0;
                    pvidAssign = 0;
                }
            }
            else
            {
                /* Force PVID assign */
                __LOG(("Force PVID assign to all packets \n"));
                pvidAssign = 1;
            }
        }
        else
        {
            __LOG(("Nested vlan (access port) \n"));
            /* Access Port */
            pvidAssign = 1;
        }

        if (pvidAssign)
        {
            if(ttiPreTtiLookupIngressEPortTablePtr)
            {
                descrPtr->eVid =
                    SMEM_LION3_TTI_DEFAULT_EPORT_ENTRY_FIELD_GET(devObjPtr, descrPtr,
                        SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_P_EVLAN);
            }
            else
            {
                descrPtr->eVid = SNET_CHT_PORT_VLAN_QOS_PVID_GET_MAC(devObjPtr,
                                                                    portVlanCfgEntryPtr);
            }

            __LOG(("Assigned PVID (port's default vid) - value : [%d] \n",
                descrPtr->eVid));

            if (descrPtr->tagSrcTagged[SNET_CHT_TAG_0_INDEX_CNS])
            {
                descrPtr->vidModified = 1;
            }

            if(ttiPreTtiLookupIngressEPortTablePtr)
            {
                pvidPrec =
                    SMEM_LION3_TTI_DEFAULT_EPORT_ENTRY_FIELD_GET(devObjPtr, descrPtr,
                        SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_P_VID_PRECEDENCE);
            }
            else
            {
                pvidPrec =
                    SNET_CHT_PORT_VLAN_QOS_PVID_PRECEDENCE_GET_MAC(devObjPtr,
                                                                   portVlanCfgEntryPtr);
            }

            if (pvidPrec == 1)
            {
                descrPtr->preserveVid = 1;
            }
        }

        if (pvidAssign == 0)
        {
            if(ttiPreTtiLookupIngressEPortTablePtr)
            {
                regValue =
                    SMEM_LION3_TTI_DEFAULT_EPORT_ENTRY_FIELD_GET(devObjPtr, descrPtr,
                        SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_VLAN_TRANSLATION);
            }
            else
            {
                regValue =
                    SNET_CHT_PORT_VLAN_QOS_VLAN_TRANSLATION_GET_MAC(devObjPtr,
                                                                    portVlanCfgEntryPtr);
            }

            /* The packet Vlan is translated to Internal Vlan */
            if (regValue)
            {
                regAddress =
                    SMEM_CHT3_INGR_VLAN_TRANS_TBL_MEM(devObjPtr,
                        SMEM_U32_GET_FIELD(descrPtr->eVid,0,(SMEM_CHT_IS_SIP7_GET(devObjPtr) ? 15 :
                                                            (SMEM_CHT_IS_SIP6_GET(devObjPtr) ? 13 : 12))));

                smemRegFldGet(devObjPtr, regAddress, 0,
                    devObjPtr->flexFieldNumBitsSupport.eVid,
                    &regValue);

                descrPtr->eVid = regValue;
                descrPtr->vidModified = 1;
                __LOG(("The packet Vlan is translated to Internal Vlan [0x%3.3x]",
                    descrPtr->eVid));
            }
        }

        /* get two bits ProtBasedQoSEn and ProtBasedVLANEn */
        if(ttiPreTtiLookupIngressEPortTablePtr)
        {
            protBaseQosEn =
                SMEM_LION3_TTI_DEFAULT_EPORT_ENTRY_FIELD_GET(devObjPtr, descrPtr,
                    SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_PROT_BASED_QOS_EN);
            protBaseVlanEn =
                SMEM_LION3_TTI_DEFAULT_EPORT_ENTRY_FIELD_GET(devObjPtr, descrPtr,
                    SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_PROT_BASED_VLAN_EN);
        }
        else
        {
            protBaseQosEn  =
                SNET_CHT_PORT_VLAN_QOS_PROT_BASED_QOS_MAC(
                    devObjPtr, portVlanCfgEntryPtr);
            protBaseVlanEn =
                SNET_CHT_PORT_VLAN_QOS_PROT_BASED_VLAN_MAC(
                    devObjPtr, portVlanCfgEntryPtr);
        }

        if (protBaseVlanEn | protBaseQosEn)
        {
            /* Port Protocol VLANs */
            __LOG(("Port Protocol VLANs"));
            snetChtPortProtVlans(devObjPtr, descrPtr, etherType, l2encapsul,
                                 protBaseVlanEn);
        }
    }

    /* QoS/CoS assign Part 1 */
    __LOG(("QoS/CoS assign Part 1"));
    snetChtQoSAssignPart1(devObjPtr, descrPtr, portVlanCfgEntryPtr);

    /* QoS/CoS assign Part 2 */
    __LOG(("QoS/CoS assign Part 2"));
    snetChtQoSAssignPart2(
        devObjPtr, descrPtr, portVlanCfgEntryPtr,GT_TRUE);

}

/**
* @internal snetChtPortProtVlans function
* @endinternal
*
* @brief   Port Protocol VLANs
*/
static GT_VOID snetChtPortProtVlans
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32 etherType,
    IN SKERNEL_L2_ENCAPSULATION_TYPE_ENT l2encapsul,
    IN GT_U32     portProtVlanEnabled
)
{
    DECLARE_FUNC_NAME(snetChtPortProtVlans);

    GT_U32 protNum;                 /* Zero based protocol index */
    GT_U32 isProtValid;             /* Protocol enable */
    GT_U32 protEncapsBmp;           /* Protocols encapsulation bitmap */
    GT_U32 protValue;               /* Protocol value */
    GT_U32 setProtVid;              /* Set protocol VLAN id */
    GT_U32 regAddr;                 /* Register's address */
    GT_U32 * regPtr;                /* Register's entry pointer */
    SNET_VLAN_MODE_ENT vlanMode;    /* VLAN assignment mode */
    GT_U32 * protCfgRegsPtr;        /* Protocols Configuration Register pointer*/
    GT_U32 numOfProt;               /* Number of supported protocols */
    GT_U32 protInReg;               /* Number of protocol in register */

    /* get number of protocols */
    numOfProt = 12;

    /* Protocols Encapsulation Configuration 0 Register */
    regAddr = SMEM_CHT_PROT_ENCAP_CONF_REG(devObjPtr);
    regPtr = smemMemGet(devObjPtr, regAddr);

    /* Protocol Configuration Register */
    regAddr = SMEM_CHT_PROT_CONF_REG(devObjPtr);
    protCfgRegsPtr = smemMemGet(devObjPtr, regAddr);

    for (protNum = 0; protNum < numOfProt; protNum++)
    {
        /* jump to next Protocols Encapsulation Configuration Register
           for Xcat A1 and above */
        if (protNum == 8)
        {
            /* Protocols Encapsulation Configuration 1 Register  */
            regAddr = SMEM_XCAT_TTI_PROT_ENCAP_CONF1_REG(devObjPtr);
            regPtr = smemMemGet(devObjPtr, regAddr);
        }

        /* each registers has 8 protocols configuration */
        protInReg = protNum % 8;
        if(protNum >= 8)
        {
            isProtValid = (*regPtr >> (12 + protInReg)) & 0x1;
        }
        else
        {
            isProtValid = (*regPtr >> (24 + protInReg)) & 0x1;
        }

        protEncapsBmp = (*regPtr >> (protInReg * 3)) & 0x7;
        if (isProtValid &&
           (protEncapsBmp & l2encapsul))
        {
            protValue = (protCfgRegsPtr[protNum / 2] >> ((protNum % 2) * 16))
                         & 0xffff;

            if (protValue == etherType)
            {
                break;
            }
        }
    }

    if(protNum == numOfProt)
    {
        /* no match */
        return;
    }

    /* Port<n> Protocol<m> VID and QoS Configuration Entry  */
    __LOG(("the etherType[0x%4.4x] matched protocol index[%d]",etherType,protNum));
    regAddr =
        SMEM_CHT_PROT_VLAN_QOS_TBL_MEM(devObjPtr,
                                        protNum,
                                        descrPtr->localDevSrcPort);

    regPtr = smemMemGet(devObjPtr, regAddr);

    /* save the pointer to the entry , for the use of the QoS classification */
    descrPtr->perProtocolInfo.portProtMatchedMemoryPointer = regPtr;

    if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        isProtValid =
            SMEM_LION3_TTI_DEFAULT_PORT_PROTOCOL_EVLAN_AND_QOS_CONFIGURATION_ENTRY_FIELD_GET(
                devObjPtr,descrPtr,
                SMEM_LION3_TTI_DEFAULT_PORT_PROTOCOL_EVLAN_AND_QOS_CONFIGURATION_TABLE_FIELDS_VALID);
    }
    else
    {
        isProtValid = SNET_CHT_PORT_PROT_IS_VALID_MAC(devObjPtr, *regPtr);
    }

    if (isProtValid)
    {
        descrPtr->perProtocolInfo.portProtMatch = GT_TRUE;
        setProtVid = GT_FALSE;
    }
    else
    {
        __LOG(("the matched protocol index[%d] is not valid",protNum));
        return;
    }

    /* check that Protocol Based VLAN assignment enabled on ingress port*/
    if(portProtVlanEnabled == 0)
    {
        __LOG(("Protocol Based VLAN assignment disabled on ingress port"));
        return;
    }

    if (descrPtr->preserveVid == 0)
    {
        if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
        {
            vlanMode  =
                SMEM_LION3_TTI_DEFAULT_PORT_PROTOCOL_EVLAN_AND_QOS_CONFIGURATION_ENTRY_FIELD_GET(
                    devObjPtr,descrPtr,
                    SMEM_LION3_TTI_DEFAULT_PORT_PROTOCOL_EVLAN_AND_QOS_CONFIGURATION_TABLE_FIELDS_EVLAN_COMMAND);
            descrPtr->preserveVid =
                SMEM_LION3_TTI_DEFAULT_PORT_PROTOCOL_EVLAN_AND_QOS_CONFIGURATION_ENTRY_FIELD_GET(
                    devObjPtr,descrPtr,
                    SMEM_LION3_TTI_DEFAULT_PORT_PROTOCOL_EVLAN_AND_QOS_CONFIGURATION_TABLE_FIELDS_EVLAN_PRECEDENCE);
        }
        else
        {
            vlanMode = SNET_CHT_PORT_PROT_VLAN_MODE_MAC(devObjPtr, *regPtr);
            descrPtr->preserveVid = SNET_CHT_PORT_PROT_VID_PRECEDENCE_MAC(devObjPtr, *regPtr);
        }


        if (vlanMode == SNET_UNTAGGED_ONLY_E)
        {
            if (descrPtr->tagSrcTagged[SNET_CHT_TAG_0_INDEX_CNS] == 0)
            {
                setProtVid = GT_TRUE;
            }
        }
        else
        if (vlanMode == SNET_TAGGED_ONLY_E)
        {
            if (descrPtr->tagSrcTagged[SNET_CHT_TAG_0_INDEX_CNS])
            {
                setProtVid = GT_TRUE;
            }
        }
        else
        if (vlanMode == SNET_ALL_PACKETS_E)
        {
            setProtVid = GT_TRUE;
        }
    }

    if (setProtVid == GT_TRUE)
    {
        if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
        {
            descrPtr->eVid =
                SMEM_LION3_TTI_DEFAULT_PORT_PROTOCOL_EVLAN_AND_QOS_CONFIGURATION_ENTRY_FIELD_GET(
                    devObjPtr,descrPtr,
                    SMEM_LION3_TTI_DEFAULT_PORT_PROTOCOL_EVLAN_AND_QOS_CONFIGURATION_TABLE_FIELDS_PROTOCOL_EVLAN);
        }
        else
        {
            descrPtr->eVid = SMEM_U32_GET_FIELD(*regPtr, 0, 12);
        }

        if (descrPtr->tagSrcTagged[SNET_CHT_TAG_0_INDEX_CNS] == 1)
        {
            descrPtr->vidModified = 1;
        }
    }
}

/**
* @internal snetChtQoSAssignPart1 function
* @endinternal
*
* @brief   QoS assign - part 1
*/
static GT_VOID  snetChtQoSAssignPart1
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32 * portVlanCfgEntryPtr
)
{
    DECLARE_FUNC_NAME(snetChtQoSAssignPart1);

    GT_U32 trustDsaTagQos;                /* trust Dsa Tag Qos */
    GT_U32  *ttiPreTtiLookupIngressEPortTablePtr;/* pointer to the entry in : tti - Pre-TTI Lookup Ingress ePort Table  */
    GT_U32  trustQosMappingTableIndexExt; /* additonal 3 bits of trustQosMappingTableIndex */

    /* tti - Pre-TTI Lookup Ingress ePort Table */
    ttiPreTtiLookupIngressEPortTablePtr = descrPtr->eArchExtInfo.ttiPreTtiLookupIngressEPortTablePtr;
    if(ttiPreTtiLookupIngressEPortTablePtr)
    {
        portVlanCfgEntryPtr = NULL;
    }

    if(ttiPreTtiLookupIngressEPortTablePtr)
    {
        /* default value */
        __LOG(("default value"));
        descrPtr->qos.qosProfile =
            SMEM_LION3_TTI_DEFAULT_EPORT_ENTRY_FIELD_GET(devObjPtr, descrPtr,
                SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_PORT_QOS_PROFILE);
        /* Trust DSA tag QoS */
        trustDsaTagQos =
            SMEM_LION3_TTI_DEFAULT_EPORT_ENTRY_FIELD_GET(devObjPtr, descrPtr,
                SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_TRUST_DSA_TAG_QOS);

        /*UP2QoS-Profile Mapping Table Index*/
        descrPtr->up2QosProfileMappingMode =
            SMEM_LION3_TTI_DEFAULT_EPORT_ENTRY_FIELD_GET(devObjPtr, descrPtr,
                SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_UP_CFI_TO_QOS_TABLE_SELECT_MODE);

        /*Trust QoS Mapping Table Index*/
        descrPtr->trustQosMappingTableIndex =
            SMEM_LION3_TTI_DEFAULT_EPORT_ENTRY_FIELD_GET(devObjPtr, descrPtr,
                SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_TRUST_QOS_MAPPING_TABLE_INDEX);
        if (SMEM_CHT_IS_SIP7_GET(devObjPtr))
        {
            trustQosMappingTableIndexExt =
                SMEM_LION3_TTI_DEFAULT_EPORT_ENTRY_FIELD_GET(devObjPtr, descrPtr,
                    SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_TRUST_QOS_MAPPING_TABLE_INDEX_EXTENSION_E);
            descrPtr->trustQosMappingTableIndex |= (trustQosMappingTableIndexExt << 4);
        }

    }
    else
    {
        if(portVlanCfgEntryPtr == NULL)
        {
            /* fix for coverity warning "FORWARD_NULL" */
            skernelFatalError("snetChtQoSAssignPart1: invalid table entry pointer\n");
        }

        /* default value */
        descrPtr->qos.qosProfile =
            SNET_CHT_PORT_VLAN_QOS_QOS_PROFILE_MAC(
                devObjPtr, portVlanCfgEntryPtr);

        /* TrustMarvellTagQoS */
        trustDsaTagQos = SNET_CHT_PORT_VLAN_QOS_TRUST_DSA_QOS_MAC(
            devObjPtr, portVlanCfgEntryPtr);
    }

    if (
        descrPtr->marvellTaggedExtended != SKERNEL_EXT_DSA_TAG_1_WORDS_E &&
        descrPtr->packetCmd == SKERNEL_EXT_PKT_CMD_FORWARD_E && trustDsaTagQos)
    {
        descrPtr->qos.qosProfile = descrPtr->ingressDsa.qosProfile;
    }
    else
    if(descrPtr->marvellTagged/* cascade port*/ &&
       descrPtr->marvellTaggedExtended != SKERNEL_EXT_DSA_TAG_1_WORDS_E &&
       descrPtr->packetCmd == SKERNEL_EXT_PKT_CMD_FORWARD_E &&
       descrPtr->qos.ingressExtendedMode &&
       (trustDsaTagQos == 0))
    {
        /* we continue as usual but such config can cause non wanted behavior */
        __LOG(("suspected as configuration ERROR! PORT_QOS_MODE = 'extended' but TRUST_DSA_TAG_QOS = 0 \n"));
    }



    /* Enables the modification of the packet's DSCP field */
    __LOG(("Enables the modification of the packet's DSCP field"));
    if(ttiPreTtiLookupIngressEPortTablePtr)
    {
        descrPtr->modifyDscp =
            SMEM_LION3_TTI_DEFAULT_EPORT_ENTRY_FIELD_GET(devObjPtr, descrPtr,
                SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_PORT_MODIFY_DSCP);
        descrPtr->modifyUp =
            SMEM_LION3_TTI_DEFAULT_EPORT_ENTRY_FIELD_GET(devObjPtr, descrPtr,
                SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_PORT_MODIFY_UP);
        descrPtr->qosProfilePrecedence =
            SMEM_LION3_TTI_DEFAULT_EPORT_ENTRY_FIELD_GET(devObjPtr, descrPtr,
                SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_PORT_QOS_PRECEDENCE);
    }
    else
    {
        descrPtr->modifyDscp =
            SNET_CHT_PORT_VLAN_QOS_MODIFY_DSCP_MAC(
                devObjPtr, portVlanCfgEntryPtr);

        /* Enables the modification of the packet's 802.1p User Priority field */
        __LOG(("Enables the modification of the packet's 802.1p User Priority field"));
        descrPtr->modifyUp =
            SNET_CHT_PORT_VLAN_QOS_MODIFY_UP_MAC(
                devObjPtr, portVlanCfgEntryPtr);

        /* Port Marking of the QoS Precedence */
        descrPtr->qosProfilePrecedence =
            SNET_CHT_PORT_VLAN_QOS_QOS_PECEDENCE_MAC(
                devObjPtr, portVlanCfgEntryPtr);

    }
}

/**
* @internal snetChtQoSAssignPart2 function
* @endinternal
*
* @brief   QoS assign - part 2 -- get the 'descrPtr->trustTag1Qos' and act according to it
*/
static GT_VOID  snetChtQoSAssignPart2
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32 * portVlanCfgEntryPtr,
    IN GT_BIT   usePortBasedQos
)
{
    DECLARE_FUNC_NAME(snetChtQoSAssignPart2);

    GT_U32 trustDsaTag;             /* trust DSA tag */
    GT_U32 trustExp;                /* trust MPLS Exp */
    GT_U32 takeProtQos;             /* Do protocol QoS */
    GT_U32 regAddr = 0;                 /* Register's address */
    GT_U32 fldFirstBit;             /* Register field's first bit */
    GT_U32 remapValid = 1;          /* do we do remap of parameter */
    GT_U32 upProfileIndex;          /* up profile index */
    GT_U32 fieldVal;                /* Register's field value */
    GT_U32 mappingParam=0;          /* QoS mapping parameter : DSCP/EXP/UP/UP1 ...*/
    GT_U32  *ttiPreTtiLookupIngressEPortTablePtr = NULL;/* pointer to the entry in : tti - Pre-TTI Lookup Ingress ePort Table  */
    GT_BIT mpls,isIp,tagSrcTagged[2],marvellTagged;/* flags from the descriptor*/
    GT_U32 exp[1],dscp,vlanEtherType1,up1,cfidei1,up,cfidei;/* fields from the descriptor*/
    GT_U32  *memPtr;/*pointer to memory*/
    GT_U32  trustQosMappingTableIndex;/* QoS mapping table index */
    GT_U32  qosPrifileNumBits;/* number of bits that the qos profile uses in next tables */

    usePortBasedQos = usePortBasedQos;/* not used parameter */

    /* get the 'frame parser fields' form the descriptor/'TRILL info' :
       for TRILL :
        based on the inner Ethernet packet*/
    SKERNEL_CHT_DESC_INNER_FRAME_FIELD_GET_MAC(descrPtr,mpls);
    SKERNEL_CHT_DESC_INNER_FRAME_FIELD_GET_MAC(descrPtr,isIp);
    SKERNEL_CHT_DESC_INNER_FRAME_FIELD_GET_MAC(descrPtr,tagSrcTagged[SNET_CHT_TAG_0_INDEX_CNS]);
    SKERNEL_CHT_DESC_INNER_FRAME_FIELD_GET_MAC(descrPtr,marvellTagged);
    SKERNEL_CHT_DESC_INNER_FRAME_FIELD_GET_MAC(descrPtr,vlanEtherType1);

    /* get the 'Qos fields' form the descriptor/'TRILL info' :
       for TRILL :
       QoS Trust Mode
       - Trust L2 and/or L3 QoS is based on the inner Ethernet packet*/
    SKERNEL_CHT_DESC_INNER_FRAME_QOS_FIELD_GET_MAC(descrPtr,dscp);
    SKERNEL_CHT_DESC_INNER_FRAME_QOS_FIELD_GET_MAC(descrPtr,exp[0]);
    SKERNEL_CHT_DESC_INNER_FRAME_QOS_FIELD_GET_MAC(descrPtr,up1);
    SKERNEL_CHT_DESC_INNER_FRAME_QOS_FIELD_GET_MAC(descrPtr,cfidei1);
    SKERNEL_CHT_DESC_INNER_FRAME_QOS_FIELD_GET_MAC(descrPtr,up);
    SKERNEL_CHT_DESC_INNER_FRAME_QOS_FIELD_GET_MAC(descrPtr,cfidei);

    /* tti - Pre-TTI Lookup Ingress ePort Table */
    ttiPreTtiLookupIngressEPortTablePtr = descrPtr->eArchExtInfo.ttiPreTtiLookupIngressEPortTablePtr;
    if(ttiPreTtiLookupIngressEPortTablePtr)
    {
        qosPrifileNumBits = 10;

        trustExp =
            SMEM_LION3_TTI_DEFAULT_EPORT_ENTRY_FIELD_GET(devObjPtr, descrPtr,
                SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_TRUST_EXP);
        /* Trust DSA tag QoS */
        trustDsaTag =
            SMEM_LION3_TTI_DEFAULT_EPORT_ENTRY_FIELD_GET(devObjPtr, descrPtr,
                SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_TRUST_DSA_TAG_QOS);

        descrPtr->trustTag1Qos =
            SMEM_LION3_TTI_DEFAULT_EPORT_ENTRY_FIELD_GET(devObjPtr, descrPtr,
                SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_TRUST_L2_QOS_TAG0_OR_TAG1);
    }
    else
    {
        qosPrifileNumBits = 7;

        if(portVlanCfgEntryPtr == NULL)
        {
            /* fix for coverity warning "FORWARD_NULL" */
            skernelFatalError("snetChtQoSAssignPart2: invalid table entry pointer\n");
        }

        trustExp = SNET_CHT_PORT_VLAN_QOS_TRUST_PKT_MPLS_EXP_MAC(
            devObjPtr, portVlanCfgEntryPtr);

        /* TrustMarvellTagQoS */
        trustDsaTag = SNET_CHT_PORT_VLAN_QOS_TRUST_DSA_QOS_MAC(
            devObjPtr, portVlanCfgEntryPtr);

        descrPtr->trustTag1Qos =
                    SNET_CHT_PORT_VLAN_QOS_TRUST_VLAN_TAG1_QOS_MAC(
                        devObjPtr, portVlanCfgEntryPtr);
    }

    /*Trust L2 QoS Tag0 or Tag1*/
    __LOG(("trustExp[%d] , trustDsaTag[%d] , descrPtr->trustTag1Qos[%d]",trustExp,trustDsaTag,descrPtr->trustTag1Qos));

    if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        trustQosMappingTableIndex = descrPtr->trustQosMappingTableIndex;
        __LOG(("trustQosMappingTableIndex[%d]",trustQosMappingTableIndex));
    }
    else
    {
        trustQosMappingTableIndex = 0;
    }

    if(SMEM_CHT_IS_SIP7_GET(devObjPtr) && trustQosMappingTableIndex > 71)
    {
        /* only 72 profiles in the device */
        skernelFatalError("snetChtQoSAssignPart2: invalid trustQosMappingTableIndex [%d] > 71 \n",
            trustQosMappingTableIndex);
    }
    else if(SMEM_CHT_IS_SIP5_GET(devObjPtr) && trustQosMappingTableIndex > 11)
    {
        /* only 12 profiles in the device */
        skernelFatalError("snetChtQoSAssignPart2: invalid trustQosMappingTableIndex [%d] > 11 \n",
            trustQosMappingTableIndex);
    }

    if (
        descrPtr->marvellTaggedExtended != SKERNEL_EXT_DSA_TAG_1_WORDS_E&&
        descrPtr->packetCmd == SKERNEL_EXT_PKT_CMD_FORWARD_E && trustDsaTag)
    {
        /* nothing more */
    }
    else
    {
        if (mpls && trustExp)
        {
            __LOG(("selected mode: mpls && trustExp , exp1[%d]",exp[0]));

            if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
            {
                memPtr = smemMemGet(devObjPtr,SMEM_CHT_EXP_2_QOS_PROF_MAP_TBL_MEM(devObjPtr,0,trustQosMappingTableIndex));

                descrPtr->qos.qosProfile = snetFieldValueGet(memPtr,qosPrifileNumBits * exp[0],qosPrifileNumBits);
                goto qosProfilDone_lbl;
            }
            /* MPLS Exp trust mode - XCAT A1 and above */
            regAddr = SMEM_CHT_EXP_2_QOS_PROF_MAP_TBL_MEM(devObjPtr, exp[0],0);
            mappingParam = exp[0];
        }
        else
        {
            /* not MPLS L2 and L3 trust modes */
            if(ttiPreTtiLookupIngressEPortTablePtr)
            {
                GT_U32  _dscp =
                    SMEM_LION3_TTI_DEFAULT_EPORT_ENTRY_FIELD_GET(devObjPtr, descrPtr,
                        SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_TRUST_DSCP);
                GT_U32  _up =
                    SMEM_LION3_TTI_DEFAULT_EPORT_ENTRY_FIELD_GET(devObjPtr, descrPtr,
                        SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_TRUST_UP);

                /* QoSTrustMode */
                fieldVal = (_dscp << 1) | /*DSCP*/
                            _up;          /*UP*/
            }
            else
            {
                /* QoSTrustMode */
                fieldVal = SNET_CHT_PORT_VLAN_QOS_TRUST_PKT_QOS_MODE_MAC(
                    devObjPtr, portVlanCfgEntryPtr);
            }

            if ((fieldVal & SNET_TRUST_L3_E) && /* trust L3 */
                 isIp)
            {
                __LOG(("selected mode: L3 trust"));

                /* ReMapDSCP */
                if(ttiPreTtiLookupIngressEPortTablePtr)
                {
                    fieldVal =
                        SMEM_LION3_TTI_DEFAULT_EPORT_ENTRY_FIELD_GET(devObjPtr, descrPtr,
                            SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_MAP_DSCP_TO_DSCP);
                }
                else
                {
                    fieldVal = SNET_CHT_PORT_VLAN_QOS_MAP_DSCP_TO_DSCP_MAC(
                        devObjPtr, portVlanCfgEntryPtr);
                }

                __LOG(("ReMapDSCP[%d]",fieldVal));

                if (fieldVal)
                {
                    if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
                    {
                        memPtr = smemMemGet(devObjPtr,SMEM_CHT_DSCP_2_DSCP_MAP_TBL_MEM(devObjPtr,0,trustQosMappingTableIndex));
                        fieldVal = snetFieldValueGet(memPtr,6 * dscp,6);
                    }
                    else
                    {
                        regAddr = SMEM_CHT_DSCP_2_DSCP_MAP_TBL_MEM(devObjPtr, dscp,0);
                        fldFirstBit = (dscp % 4) * 8;
                        smemRegFldGet(devObjPtr, regAddr, fldFirstBit, 6, &fieldVal);
                    }

                    __LOG(("old dscp[%d] , new dscp after remap [%d]",dscp,fieldVal));

                    /* DSCP2DSCP */
                    dscp = fieldVal;
                }
                else
                {
                    dscp = dscp;
                }

                if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
                {
                    memPtr = smemMemGet(devObjPtr,SMEM_CHT_DSCP_2_QOS_PROF_MAP_REG(devObjPtr,0,trustQosMappingTableIndex));

                    descrPtr->qos.qosProfile = snetFieldValueGet(memPtr,qosPrifileNumBits * dscp,qosPrifileNumBits);
                    goto qosProfilDone_lbl;
                }

                regAddr = SMEM_CHT_DSCP_2_QOS_PROF_MAP_REG(devObjPtr, dscp,0);

                mappingParam = dscp;
            }
            else
            if ((fieldVal & SNET_TRUST_L2_E) && /* trust L2 */
                (tagSrcTagged[SNET_CHT_TAG_0_INDEX_CNS] || marvellTagged))
            {
                if (descrPtr->trustTag1Qos && vlanEtherType1)
                {
                    __LOG(("selected mode: L2 trust , use up1[%d] cfi1[%d]",up1,cfidei1));

                    up = up1;
                    cfidei = cfidei1;
                }
                else
                {
                    __LOG(("selected mode: L2 trust , use up[%d] cfi[%d]",up,cfidei));
                    up = up;
                    cfidei = cfidei;
                }

                if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
                {

                    if(descrPtr->up2QosProfileMappingMode)
                    {
                        trustQosMappingTableIndex = descrPtr->trustTag1Qos ? up1 : up;
                        __LOG(("trustQosMappingTableIndex[%d] from [%s]",trustQosMappingTableIndex,descrPtr->trustTag1Qos ? "up1" : "up0"));
                    }

                    memPtr = smemMemGet(devObjPtr,SMEM_CHT_UP_2_QOS_PROF_MAP_TBL_MEM(devObjPtr,0,0,trustQosMappingTableIndex));

                    descrPtr->qos.qosProfile = snetFieldValueGet(memPtr,qosPrifileNumBits * ((cfidei*8) + up),qosPrifileNumBits);
                    goto qosProfilDone_lbl;
                }

                if(SKERNEL_IS_LION_REVISON_B0_DEV(devObjPtr))
                {
                    regAddr = SMEM_LION_TTI_CFI_UP_TO_QOS_PROFILE_MAPPING_TABLE_SELECTOR_REG(devObjPtr,
                        descrPtr->localDevSrcPort);
                    fldFirstBit = up + (descrPtr->localDevSrcPort % 4) * 8;
                    smemRegFldGet(devObjPtr, regAddr, fldFirstBit, 1, &upProfileIndex);

                    __LOG(("upProfileIndex[%d]\n",upProfileIndex));
                }
                else
                {
                    /* the device not support the upProfileIndex parameter */
                    __LOG(("the device not support the upProfileIndex parameter"));
                    upProfileIndex = 0;
                }

                regAddr = SMEM_CHT_UP_2_QOS_PROF_MAP_TBL_MEM(devObjPtr,up,cfidei,upProfileIndex);

                mappingParam = up;
            }
            else
            {
                __LOG(("selected mode: not trust COS info from the packet"));

                remapValid = 0;
            }
        }

        if(remapValid == 1)
        {
            /* 4 QoS profiles in register */
            fldFirstBit = (mappingParam % 4) * 8;
            smemRegFldGet(devObjPtr, regAddr, fldFirstBit, 7,  &fieldVal);
            descrPtr->qos.qosProfile = fieldVal;
        }
    }

qosProfilDone_lbl:
    __LOG(("new descrPtr->qos.qosProfile[%d]",descrPtr->qos.qosProfile));

    /* when we do 'port based' we need to also do 'port protocol' */

    if(ttiPreTtiLookupIngressEPortTablePtr)
    {
        fieldVal =
            SMEM_LION3_TTI_DEFAULT_EPORT_ENTRY_FIELD_GET(devObjPtr, descrPtr,
                SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_PROT_BASED_QOS_EN);
    }
    else
    {
        fieldVal =
            SNET_CHT_PORT_VLAN_QOS_PROT_BASED_QOS_MAC(
                devObjPtr, portVlanCfgEntryPtr);
    }
    /* Protocol based QoS enable */
    __LOG(("Protocol based QoS enabled [%d] \n",fieldVal));

    if (descrPtr->qosProfilePrecedence == SKERNEL_QOS_PROF_PRECED_SOFT &&
        descrPtr->perProtocolInfo.portProtMatch == GT_TRUE &&
        fieldVal && descrPtr->packetCmd <= SKERNEL_EXT_PKT_CMD_SOFT_DROP_E &&
        descrPtr->perProtocolInfo.portProtMatchedMemoryPointer)
    {
        takeProtQos = GT_FALSE;
        if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
        {
            /* Protocol Modify QoS Profile */
            fieldVal =
                SMEM_LION3_TTI_DEFAULT_PORT_PROTOCOL_EVLAN_AND_QOS_CONFIGURATION_ENTRY_FIELD_GET(
                    devObjPtr,descrPtr,
                    SMEM_LION3_TTI_DEFAULT_PORT_PROTOCOL_EVLAN_AND_QOS_CONFIGURATION_TABLE_FIELDS_PROTOCOL_MODIFY_QOS_PROFILE);
        }
        else
        {
            /* Protocol QoSProfile Mode */
            fieldVal =
                SNET_CHT_PORT_PROT_VLAN_QOS_QOS_PROFILE_MODE_MAC(
                    devObjPtr, *descrPtr->perProtocolInfo.portProtMatchedMemoryPointer);
        }

        __LOG(("Protocol Modify QoS Profile Mode [%s]",
                        (fieldVal == SNET_NOT_CHANGE_E) ? "SNET_NOT_CHANGE_E" :
                        (fieldVal == SNET_UNTAGGED_ONLY_E) ? "SNET_UNTAGGED_ONLY_E" :
                        (fieldVal == SNET_TAGGED_ONLY_E) ? "SNET_TAGGED_ONLY_E" :
                        "SNET_ALL_PACKETS_E"
                      ));

        if (tagSrcTagged[SNET_CHT_TAG_0_INDEX_CNS])
        {
            if (fieldVal == SNET_TAGGED_ONLY_E ||
                fieldVal == SNET_ALL_PACKETS_E)
            {
                takeProtQos = GT_TRUE;
            }
        }
        else
        {
            if (fieldVal == SNET_UNTAGGED_ONLY_E ||
                fieldVal == SNET_ALL_PACKETS_E)
            {
                takeProtQos = GT_TRUE;
            }
        }

        if (takeProtQos == GT_TRUE)
        {
            if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
            {
                descrPtr->qos.qosProfile =
                    SMEM_LION3_TTI_DEFAULT_PORT_PROTOCOL_EVLAN_AND_QOS_CONFIGURATION_ENTRY_FIELD_GET(
                        devObjPtr,descrPtr,
                        SMEM_LION3_TTI_DEFAULT_PORT_PROTOCOL_EVLAN_AND_QOS_CONFIGURATION_TABLE_FIELDS_QOS_PROFILE);
                descrPtr->modifyDscp =
                    SMEM_LION3_TTI_DEFAULT_PORT_PROTOCOL_EVLAN_AND_QOS_CONFIGURATION_ENTRY_FIELD_GET(
                        devObjPtr,descrPtr,
                        SMEM_LION3_TTI_DEFAULT_PORT_PROTOCOL_EVLAN_AND_QOS_CONFIGURATION_TABLE_FIELDS_PROTOCOL_MODIFY_DSCP);
                descrPtr->modifyUp =
                    SMEM_LION3_TTI_DEFAULT_PORT_PROTOCOL_EVLAN_AND_QOS_CONFIGURATION_ENTRY_FIELD_GET(
                        devObjPtr,descrPtr,
                        SMEM_LION3_TTI_DEFAULT_PORT_PROTOCOL_EVLAN_AND_QOS_CONFIGURATION_TABLE_FIELDS_PROTOCOL_MODIFY_UP);
                descrPtr->qosProfilePrecedence =
                    SMEM_LION3_TTI_DEFAULT_PORT_PROTOCOL_EVLAN_AND_QOS_CONFIGURATION_ENTRY_FIELD_GET(
                        devObjPtr,descrPtr,
                        SMEM_LION3_TTI_DEFAULT_PORT_PROTOCOL_EVLAN_AND_QOS_CONFIGURATION_TABLE_FIELDS_PROTOCOL_QOS_PRECEDENCE);
            }
            else
            {
                descrPtr->qos.qosProfile =
                    SNET_CHT_PORT_PROT_VLAN_QOS_QOS_PROFILE_MAC(
                        devObjPtr, *descrPtr->perProtocolInfo.portProtMatchedMemoryPointer);
                descrPtr->modifyDscp =
                    SNET_CHT_PORT_PROT_VLAN_QOS_MODIFY_DSCP_MAC(
                        devObjPtr, *descrPtr->perProtocolInfo.portProtMatchedMemoryPointer);
                descrPtr->modifyUp =
                    SNET_CHT_PORT_PROT_VLAN_QOS_MODIFY_UP_MAC(
                        devObjPtr, *descrPtr->perProtocolInfo.portProtMatchedMemoryPointer);
                descrPtr->qosProfilePrecedence =
                    SNET_CHT_PORT_PROT_VLAN_QOS_QOS_PECEDENCE_MAC(
                        devObjPtr, *descrPtr->perProtocolInfo.portProtMatchedMemoryPointer);
            }

            __LOG(("port protocol modify qos parameters: "
                          "descrPtr->qos.qosProfile[%d] "
                          "descrPtr->modifyDscp[%d] "
                          "descrPtr->modifyUp[%d] "
                          "descrPtr->qosProfilePrecedence[%d] \n"
                          ,descrPtr->qos.qosProfile
                          ,descrPtr->modifyDscp
                          ,descrPtr->modifyUp
                          ,descrPtr->qosProfilePrecedence));
        }
        else
        {
            __LOG(("port protocol not applicable for the traffic type \n"));
        }
    }
}

/**
* @internal snetXCatA1TrunkHash function
* @endinternal
*
* @brief   Hash Index Generation Procedure
*/
static GT_VOID snetXCatA1TrunkHash
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr
)
{
    DECLARE_FUNC_NAME(snetXCatA1TrunkHash);

    GT_U32 *regPtr;                 /* Pointer to register's entry */
    GT_U32 fieldValue;              /* Register field value */
    GT_U32 hashVal;                 /* hashing trunk value         */
    /* 6-bit values to use in HASH function      */
    GT_U32 hFlowLabel;     /* Hash part of IPV6 header flow label   */
    GT_U32 hSrcTcpPort;    /* Hash part of source TCP/UDP port      */
    GT_U32 hDstTcpPort;    /* Hash part of destination TCP/UDP port */
    GT_U32 hDstMac;        /* Hash part of destination MAC address  */
    GT_U32 hSrcMac;        /* Hash part of source MAC address       */
    GT_U8* hSip;            /* pointer to source IP address         */
    GT_U8* hDip;            /* pointer to destination IP address    */
    GT_U32 hMplsLabel0;    /* Hash part of MPLS label0              */
    GT_U32 hMplsLabel1;    /* Hash part of MPLS label0              */
    GT_U32 hMplsLabel2;    /* Hash part of MPLS label0              */
    /* HASH configuration fields */
    GT_U32 l4LongHash;              /* L4 long Hash             */
    GT_U32 ipv6HashMode;            /* IPV6 Hash mode           */
    GT_U32 trunkLbhMode;            /* trunk Lbh  mode          */
    GT_U32 addL4Hash;               /* add L4 Hash              */
    GT_U32 addIpHash;               /* add IP Hash              */
    GT_U32 addMplsHash;             /* add MPLS Hash            */
    GT_U32 addMacHash;              /* add MAC Hash             */
    GT_U32 ipv6SipMask;             /* ipv6 Sip Mask            */
    GT_U32 ipv6DipMask;             /* ipv6 Dip Mask            */
    GT_U32 ipv4DipShift;            /* ipv4 Dip Shift           */
    GT_U32 ipv4SipShift;            /* ipv4 Sip Shift           */
    GT_U32 ipv6DipShift;            /* ipv6 Dip Shift           */
    GT_U32 ipv6SipShift;            /* ipv6 Sip Shift           */
    GT_U32 macSaMask;               /* MAC SA Mask              */
    GT_U32 macDaMask;               /* MAC DA Mask              */
    GT_U32 ipv6FlowMask;            /* ipv6 Flow Mask           */
    GT_U32 ipv4SipMask;             /* ipv4 Sip Mask            */
    GT_U32 ipv4DipMask;             /* ipv4 Dip Mask            */
    GT_U32 mplsLab0Mask;            /* MPLS Label0 Mask         */
    GT_U32 mplsLab1Mask;            /* MPLS Label1 Mask         */
    GT_U32 mplsLab2Mask;            /* MPLS Label2 Mask         */
    GT_U32 l4SrcPortMask;           /* L4 Source port Mask      */
    GT_U32 l4DstPortMask;           /* L4 Destination port Mask */

    if (SMEM_CHT_IS_SIP5_20_GET(devObjPtr))
    {
        if(descrPtr->eArchExtInfo.ttiPhysicalPort2AttributePtr)
        {
            fieldValue = SMEM_SIP5_20_TTI_PHYSICAL_PORT_2_ENTRY_FIELD_GET(devObjPtr,descrPtr,
                SMEM_LION3_TTI_PHYSICAL_PORT_2_TABLE_FIELD_EXTRACT_HASH_FROM_FORWARD_EDSA);

            __LOG_PARAM_WITH_NAME("Extract Hash from FORWARD eDSA Enable", fieldValue);
        }
        else
        {
            fieldValue = 0;
        }

        if (fieldValue)
        {
            __LOG(("Desc<hash[5:0]> is not overridden by the TTI/IPCL hash generation function \n"));
            descrPtr->keepPreviousHash = 1;
            return;
        }
    }

    /* retrieve HASH control parameters */
    regPtr = smemMemGet(devObjPtr, SMEM_XCAT_TRUNK_HASH_CONF_REGISTER0(devObjPtr));
    l4LongHash    = SMEM_U32_GET_FIELD(regPtr[0], 0, 1);
    ipv6HashMode  = SMEM_U32_GET_FIELD(regPtr[0], 1, 2);
    trunkLbhMode  = SMEM_U32_GET_FIELD(regPtr[0], 3, 1);
    addL4Hash     = SMEM_U32_GET_FIELD(regPtr[0], 4, 1);
    addIpHash     = SMEM_U32_GET_FIELD(regPtr[0], 5, 1);
    addMplsHash   = SMEM_U32_GET_FIELD(regPtr[0], 6, 1);
    addMacHash    = SMEM_U32_GET_FIELD(regPtr[0], 7, 1);
    ipv6SipMask   = SMEM_U32_GET_FIELD(regPtr[0], 8, 6);
    ipv6DipMask   = SMEM_U32_GET_FIELD(regPtr[0], 14, 6);
    ipv4DipShift  = SMEM_U32_GET_FIELD(regPtr[0], 20, 2);
    ipv4SipShift  = SMEM_U32_GET_FIELD(regPtr[0], 22, 2);
    ipv6DipShift  = SMEM_U32_GET_FIELD(regPtr[0], 24, 4);
    ipv6SipShift  = SMEM_U32_GET_FIELD(regPtr[0], 28, 4);
    macSaMask     = SMEM_U32_GET_FIELD(regPtr[1], 0, 6);
    macDaMask     = SMEM_U32_GET_FIELD(regPtr[1], 6, 6);
    ipv4SipMask   = SMEM_U32_GET_FIELD(regPtr[1], 12, 6);
    ipv4DipMask   = SMEM_U32_GET_FIELD(regPtr[1], 18, 6);
    ipv6FlowMask  = SMEM_U32_GET_FIELD(regPtr[1], 24, 6);
    mplsLab0Mask  = SMEM_U32_GET_FIELD(regPtr[2], 0, 6);
    mplsLab1Mask  = SMEM_U32_GET_FIELD(regPtr[2], 6, 6);
    mplsLab2Mask  = SMEM_U32_GET_FIELD(regPtr[2], 12, 6);
    l4SrcPortMask = SMEM_U32_GET_FIELD(regPtr[2], 18, 6);
    l4DstPortMask = SMEM_U32_GET_FIELD(regPtr[2], 24, 6);

    if(simLogIsOpenFlag)
    {
        __LOG_PARAM(l4LongHash   );
        __LOG_PARAM(ipv6HashMode );
        __LOG_PARAM(trunkLbhMode );
        __LOG_PARAM(addL4Hash    );
        __LOG_PARAM(addIpHash    );
        __LOG_PARAM(addMplsHash  );
        __LOG_PARAM(addMacHash   );
        __LOG_PARAM(ipv6SipMask  );
        __LOG_PARAM(ipv6DipMask  );
        __LOG_PARAM(ipv4DipShift );
        __LOG_PARAM(ipv4SipShift );
        __LOG_PARAM(ipv6DipShift );
        __LOG_PARAM(ipv6SipShift );
        __LOG_PARAM(macSaMask    );
        __LOG_PARAM(macDaMask    );
        __LOG_PARAM(ipv4SipMask  );
        __LOG_PARAM(ipv4DipMask  );
        __LOG_PARAM(ipv6FlowMask );
        __LOG_PARAM(mplsLab0Mask );
        __LOG_PARAM(mplsLab1Mask );
        __LOG_PARAM(mplsLab2Mask );
        __LOG_PARAM(l4SrcPortMask);
        __LOG_PARAM(l4DstPortMask);
    }

    /* TrunkLBH Mode */
    if (trunkLbhMode == 0)
    {
        /* Hash is based on the packets data */
        __LOG(("Hash is based on the packets data \n"));

        hDstMac = descrPtr->macDaPtr[5] & macDaMask;
        hSrcMac = descrPtr->macSaPtr[5] & macSaMask;

        __LOG_PARAM(hDstMac);
        __LOG_PARAM(hSrcMac);

        if(descrPtr->isIp && addIpHash && descrPtr->ipHeaderError)
        {
            __LOG(("WARNING : the IP hash parameters are not used for hash when 'ip header error' !!!"));
        }

        if ((descrPtr->isIp == 0) && descrPtr->mpls && addMplsHash)
        {
            /* MPLS hash function */
            __LOG(("hashing for MPLS packet \n"));
            hMplsLabel0 = descrPtr->mplsLabelValueForHash[0] & mplsLab0Mask;
            hMplsLabel1 = descrPtr->mplsLabelValueForHash[1] & mplsLab1Mask;
            hMplsLabel2 = descrPtr->mplsLabelValueForHash[2] & mplsLab2Mask;

            __LOG_PARAM(hMplsLabel0);
            __LOG_PARAM(hMplsLabel1);
            __LOG_PARAM(hMplsLabel2);

            hashVal = hMplsLabel0 ^ hMplsLabel1 ^ hMplsLabel2;

            __LOG_PARAM(hashVal);

            /* AddMACHash */
            if (addMacHash)
            {
                __LOG(("Add MAC Hash \n"));
                hashVal = hashVal ^ hSrcMac ^ hDstMac;
                __LOG_PARAM(hashVal);
            }
        }
        else if (descrPtr->isIp && addIpHash && (descrPtr->ipHeaderError == 0))
        {
            /* IP hash function */
            if (descrPtr->isIPv4 == 0)
            {
                /* IPV6 packet */
                __LOG(("hashing for IPV6 packet \n"));
                hSip = &descrPtr->l3StartOffsetPtr[8];
                hDip = &descrPtr->l3StartOffsetPtr[8+16];

                switch (ipv6HashMode)
                {
                    default:
                    case SNET_IPV6_TRUNK_HASH0_E:
                        __LOG(("ipv6HashMode = SNET_IPV6_TRUNK_HASH0_E \n"));
                        __LOG(("SIP[5:0] ^ SIP[21:16] ^ DIP[5:0] ^ DIP[21:16] ^ flow[5:0] \n"));
                        /* SIP[5:0] ^ SIP[21:16] ^ DIP[5:0] ^ DIP[21:16] ^ flow[5:0] */
                        hFlowLabel = descrPtr->flowLabel & ipv6FlowMask;

                        hashVal = (hSip[IPV6_SHIFT_INDEX_MAC(15,ipv6SipShift)] & ipv6SipMask) ^
                                  (hSip[IPV6_SHIFT_INDEX_MAC(13,ipv6SipShift)] & ipv6SipMask) ^
                                  (hDip[IPV6_SHIFT_INDEX_MAC(15,ipv6DipShift)] & ipv6DipMask) ^
                                  (hDip[IPV6_SHIFT_INDEX_MAC(13,ipv6DipShift)] & ipv6DipMask) ^
                                  hFlowLabel;
                        break;
                    case SNET_IPV6_TRUNK_HASH1_E:
                        __LOG(("ipv6HashMode = SNET_IPV6_TRUNK_HASH1_E \n"));
                        __LOG(("SIP[69:64] ^ SIP[125:120] ^ DIP[69:64] ^ DIP[125:120 ^ flow[13:8] \n"));
                        /* SIP[69:64] ^ SIP[125:120] ^ DIP[69:64] ^ DIP[125:120 ^ flow[13:8] */
                        hFlowLabel = (descrPtr->flowLabel >> 8) & ipv6FlowMask;
                        hashVal = (hSip[IPV6_SHIFT_INDEX_MAC(7,ipv6SipShift)] & ipv6SipMask) ^
                                  (hSip[IPV6_SHIFT_INDEX_MAC(0,ipv6SipShift)] & ipv6SipMask) ^
                                  (hDip[IPV6_SHIFT_INDEX_MAC(7,ipv6DipShift)] & ipv6DipMask) ^
                                  (hDip[IPV6_SHIFT_INDEX_MAC(0,ipv6DipShift)] & ipv6DipMask) ^
                                  hFlowLabel;
                        break;
                    case SNET_IPV6_TRUNK_HASH2_E:
                        __LOG(("ipv6HashMode = SNET_IPV6_TRUNK_HASH2_E \n"));
                        __LOG(("SIP[5:0] ^ SIP[21:16] ^ DIP[5:0] ^ DIP[21:16] ^ flow[5:0] \n"));
                        __LOG(("SIP[69:64] ^ SIP[125:120] ^ DIP[69:64] ^ DIP[125:120 ^ flow[13:8] \n"));
                        /* hashVal_for_case_0 ^ hashVal_for_case_1 */
                        /* SIP[5:0] ^ SIP[21:16] ^ DIP[5:0] ^ DIP[21:16] ^ flow[5:0] */
                        /* SIP[69:64] ^ SIP[125:120] ^ DIP[69:64] ^ DIP[125:120 ^ flow[13:8] */

                        /* hashVal_for_case_0 */
                        hFlowLabel = descrPtr->flowLabel & ipv6FlowMask;

                        hashVal = (hSip[IPV6_SHIFT_INDEX_MAC(15,ipv6SipShift)] & ipv6SipMask) ^
                                  (hSip[IPV6_SHIFT_INDEX_MAC(13,ipv6SipShift)] & ipv6SipMask) ^
                                  (hDip[IPV6_SHIFT_INDEX_MAC(15,ipv6DipShift)] & ipv6DipMask) ^
                                  (hDip[IPV6_SHIFT_INDEX_MAC(13,ipv6DipShift)] & ipv6DipMask) ^
                                  hFlowLabel;

                        /* ^ hashVal_for_case_1 */
                        hFlowLabel = (descrPtr->flowLabel >> 8) & ipv6FlowMask;
                        hashVal = (hSip[IPV6_SHIFT_INDEX_MAC(7,ipv6SipShift)] & ipv6SipMask) ^
                                  (hSip[IPV6_SHIFT_INDEX_MAC(0,ipv6SipShift)] & ipv6SipMask) ^
                                  (hDip[IPV6_SHIFT_INDEX_MAC(7,ipv6DipShift)] & ipv6DipMask) ^
                                  (hDip[IPV6_SHIFT_INDEX_MAC(0,ipv6DipShift)] & ipv6DipMask) ^
                                  hFlowLabel ^
                                  hashVal;
                        break;
                    case SNET_IPV6_TRUNK_HASH3_E:
                        __LOG(("ipv6HashMode = SNET_IPV6_TRUNK_HASH3_E \n"));
                        __LOG(("SIP[5:0] ^ SIP[21:16] ^ DIP[5:0] ^ DIP[21:16] \n"));
                        /* SIP[5:0] ^ SIP[21:16] ^ DIP[5:0] ^ DIP[21:16] */
                        hashVal = (hSip[IPV6_SHIFT_INDEX_MAC(15,ipv6SipShift)] & ipv6SipMask) ^
                                  (hSip[IPV6_SHIFT_INDEX_MAC(13,ipv6SipShift)] & ipv6SipMask) ^
                                  (hDip[IPV6_SHIFT_INDEX_MAC(15,ipv6DipShift)] & ipv6DipMask) ^
                                  (hDip[IPV6_SHIFT_INDEX_MAC(13,ipv6DipShift)] & ipv6DipMask) ;
                        break;
                }
            }
            else
            {
                /* IPV4 packet */
                __LOG(("hashing for IPV4 packet \n"));
                hSip = &descrPtr->l3StartOffsetPtr[12];
                hDip = &descrPtr->l3StartOffsetPtr[12+4];

                /* SIP[5:0] ^ SIP[21:16] ^ DIP[5:0] ^ DIP[21:16] */
                hashVal = (hSip[IPV4_SHIFT_INDEX_MAC(3,ipv4SipShift)] & ipv4SipMask) ^
                          (hSip[IPV4_SHIFT_INDEX_MAC(1,ipv4SipShift)] & ipv4SipMask) ^
                          (hDip[IPV4_SHIFT_INDEX_MAC(3,ipv4DipShift)] & ipv4DipMask) ^
                          (hDip[IPV4_SHIFT_INDEX_MAC(1,ipv4DipShift)] & ipv4DipMask) ;
            }

            __LOG_PARAM(hashVal);

            /* addL4Hash */
            if (addL4Hash &&
               (descrPtr->udpCompatible ||
                descrPtr->ipProt == SNET_TCP_PROT_E) &&
                descrPtr->l4StartOffsetPtr)
            {
                __LOG(("ADD L4 info into the hash \n"));

                hSrcTcpPort = descrPtr->l4SrcPort & l4SrcPortMask;
                hDstTcpPort = descrPtr->l4DstPort & l4DstPortMask;
                __LOG_PARAM(hSrcTcpPort);
                __LOG_PARAM(hDstTcpPort);

                hashVal = hashVal ^ hSrcTcpPort ^ hDstTcpPort;
                __LOG_PARAM(hashVal);

                if (l4LongHash)
                {
                    __LOG(("ADD L4 LONG info into the hash \n"));

                    /* L4LongTrunk LBH */
                    hSrcTcpPort = (descrPtr->l4SrcPort >> 8) & l4SrcPortMask;
                    hDstTcpPort = (descrPtr->l4DstPort >> 8) & l4DstPortMask;
                    __LOG_PARAM(hSrcTcpPort);
                    __LOG_PARAM(hDstTcpPort);

                    hashVal = hashVal ^ hSrcTcpPort ^ hDstTcpPort;
                    __LOG_PARAM(hashVal);
                }

            }
            /* AddMACHash */
            if (addMacHash)
            {
                __LOG(("Add MAC Hash \n"));
                hashVal = hashVal ^ hSrcMac ^ hDstMac;
            }
        }
        else
        {
            /* not IP and not MPLS hash function */
            __LOG(("not IP and not MPLS hash function"));
            hashVal = hSrcMac ^ hDstMac;
        }
    }
    else
    {
        /* Hash is based on the packet's Source Port# or Source Trunk# */
        __LOG(("Hash is based on the packet's Source Port# or Source Trunk# [0x%x]",
            descrPtr->origSrcEPortOrTrnk));
        hashVal = descrPtr->origSrcEPortOrTrnk & 0x3f;
    }

    __LOG(("(NOTE: 6 bits hash) Final packet Hash value [0x%x] \n" ,
        hashVal));

    descrPtr->pktHash = hashVal;
}

/**
* @internal snetChtLinkStateNotify function
* @endinternal
*
* @brief   Notify devices database that link state changed
*/
static GT_VOID snetChtLinkStateNotify
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32 port,
    IN GT_U32 linkState
)
{
    GT_U32 regAddress;          /* Register address */
    GT_U32 fldValue;            /* Register's field value */
    GT_U32 regData;             /* Register's data */
    GT_U32 portInGlobBmp;       /* Port's sum interrupt bmp in the global register */
    GT_U32 fldPortBit;          /* Port sum interrupt bit */
    GT_U32 doInterrupt = 1;     /* Trigger global interrupt */
    GT_U32 oldLinkState;        /* Link status before the change*/
    GT_U32 portIntCauseReg;     /* Port interrupt cause register*/
    GT_U32 portIntMaskReg;      /* Port interrupt mask register*/
    GT_U32 outputPortBit;       /* the bit index for the egress port */
    GT_BOOL isSecondRegister;   /* is the per port config is in second register -- for bmp of ports */
    GT_U32 autoNeg, newSpeed;
    GT_U32  forceLinkDown , forceLinkPass;
    GT_U32  macPort;            /* MAC port number */
    GT_U32  bypassCheckLinkChange = 0;
    GT_U32  PortMACReset;

    if(!IS_CHT_VALID_PORT(devObjPtr,port))
    {
        /* port not exists */
        return;
    }

    if (CHT_IS_CG_OR_MTI_MAC_USED_MAC(devObjPtr, port))
    {
        /* function called 'Falcon' but also for BC3/Aldrin2/PIPE CG ports */
        snetFalconLinkStateNotify(devObjPtr,port,linkState);
        return;
    }

    if (devObjPtr->numOfPipes)
    {
        GT_U32  dummyPort;
        /* Update 'currentPipeId' and get new (local) port */
        smemConvertGlobalPortToCurrentPipeId(devObjPtr,port,&dummyPort);
    }

    macPort = snetChtRxMacPortGet(devObjPtr,port);

    forceLinkDown = snetChtPortMacFieldGet(devObjPtr, macPort,
        SNET_CHT_PORT_MAC_FIELDS_forceLinkDown_E);
    forceLinkPass = snetChtPortMacFieldGet(devObjPtr, macPort,
        SNET_CHT_PORT_MAC_FIELDS_forceLinkPass_E);

    /* not GIG  (not tested on HW)
       not CG   (not work on HW)
       only XLG (checked on HW and works)
       */
    if(simulationInitReady /* check only after simulation is ready */&&
       IS_CHT_HYPER_GIGA_PORT(devObjPtr,macPort) &&
       (0 == CHT_IS_CG_OR_MTI_MAC_USED_MAC(devObjPtr,macPort)))
    {
        /* when the 'old PortMACReset' was 'reset' and the current is 'not reset' */
        /* we send link change interrupt ... bypass the check of 'old state'/'new state'  !!! */

        PortMACReset = snetChtPortMacFieldGet(devObjPtr, macPort,
            SNET_CHT_PORT_MAC_FIELDS_PortMACReset_E);

        if(PortMACReset == 0)/* port not in reset now */
        {
            bypassCheckLinkChange = 1;
        }
    }

    if(simulationInitReady /* check only after simulation is ready */&&
       (forceLinkDown || forceLinkPass))
    {
        bypassCheckLinkChange = 1;
    }


    /* NOTE: force link pass holds priority vs force link down !!!
        this is the HW behavior (tested on XG port)
    */
    if(forceLinkPass == 1)
    {
        /* ignore the 'request' and force the link up */
        linkState = 1;
    }
    else
    if(forceLinkDown == 1)
    {
        /* ignore the 'request' and force the link down */
        linkState = 0;
    }

    oldLinkState = snetChtPortMacFieldGet(devObjPtr, macPort,
        SNET_CHT_PORT_MAC_FIELDS_LinkState_E);

    if(bypassCheckLinkChange)
    {
        /* we bypass the check of 'old state'/'new state' */
    }
    else
    if(linkState == oldLinkState)
    {
        /* no need to change status */
        return;
    }

    snetChtPortMacFieldSet(devObjPtr, macPort,
        SNET_CHT_PORT_MAC_FIELDS_LinkState_E,
        linkState);

    if(SKERNEL_DEVICE_FAMILY_LION_PORT_GROUP_DEV(devObjPtr) &&
        !devObjPtr->myInterruptsDbPtr)
    {
        if(devObjPtr->txqRevision == 0)
        {
            fldPortBit = (IS_CHT_CPU_PORT(port)) ? 28 : port;

            /* set also register that all ports links show */
            smemRegFldSet(devObjPtr,SMEM_LION_LINKS_STATUS_REG_MAC(devObjPtr), fldPortBit, 1, linkState);
        }
        else
        {
            if(SMEM_CHT_IS_SIP6_GET(devObjPtr))
            {
                /* currently the TXQ-LL unit was removed from the simulation so no update to the register */
            }
            else
            if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
            {
                regAddress = SMEM_LION_TXQ_PORTS_LINK_ENABLE_STATUS_REG(devObjPtr,macPort);
                outputPortBit = macPort & 0x1f;
                /* set also register that all ports links show */
                smemRegFldSet(devObjPtr, regAddress, outputPortBit, 1, linkState);
            }
            else
            {
                regAddress = SMEM_LION_TXQ_PORTS_LINK_ENABLE_STATUS_REG(devObjPtr,0/*not used*/);
                snetChtHaPerPortInfoGet(devObjPtr, port, &isSecondRegister, &outputPortBit);
                if(isSecondRegister == GT_TRUE)
                {
                    regAddress += 4;
                }

                /* set also register that all ports links show */
                smemRegFldSet(devObjPtr, regAddress, outputPortBit, 1, linkState);
            }
        }
    }

    /* Global Interrupt Cause register SUM bmp */
    portInGlobBmp = SMEM_CHT_PORT_CAUSE_INT_SUM_BMP(devObjPtr,macPort);

    if(devObjPtr->myInterruptsDbPtr)
    {
        /* the device has support for the tree of the port interrupts.
        from the port's mac (giga/XLG) to MG unit (global summary) */
        snetChetahDoInterrupt(devObjPtr,
                              SMEM_CHT_PORT_INT_CAUSE_REG(devObjPtr,macPort),
                              SMEM_CHT_PORT_INT_CAUSE_MASK_REG(devObjPtr,macPort),
                              (1 << 1),/* in Giga and in XLG bit 1 is the 'LINK STATUS CHANGED' */
                              portInGlobBmp);

        /* check for EEE interrupt support */
        snetBobcat2EeeCheckInterrupts(devObjPtr,macPort);

        return;
    }

    portIntCauseReg = SMEM_CHT_PORT_INT_CAUSE_REG(devObjPtr, macPort);
    smemRegGet(devObjPtr, portIntCauseReg, &regData);

    /* Link status changed on port (interrupt cause register) */
    SMEM_U32_SET_FIELD(regData, 1, 1, 1);
    /* Sum of all GOP interrupts */
    SMEM_U32_SET_FIELD(regData, 0, 1, 1);

    smemRegSet(devObjPtr, portIntCauseReg, regData);

    portIntMaskReg = SMEM_CHT_PORT_INT_CAUSE_MASK_REG(devObjPtr, macPort);
    smemRegFldGet(devObjPtr, portIntMaskReg, 1, 1, &fldValue);
    /* Link status changed on port bit (interrupt mask register) */
    if (fldValue != 1)
    {
        /* do not trigger global interrupt but still fill the cause
           registers in the tree to allow the interrupts be ready when the
           mask register value will be changed ...
        */
        doInterrupt = 0;
    }

    if(IS_CHT_CPU_PORT(port))
    {
        /* generate interrupt */
        snetChetahDoInterruptLimited(devObjPtr,
                              portIntCauseReg,
                              portIntMaskReg,
                              (1 << 1),
                              portInGlobBmp,
                              doInterrupt);/* let the function know if to set interrupt or not */
    }
    else if(IS_CHT_REGULAR_PORT(devObjPtr,port) &&
            (!SKERNEL_IS_LION2_DEV(devObjPtr)))/* Gig port in Lion2 similar to XLG(XG) port*/
    {
        smemRegFldGet(devObjPtr,
                      SMEM_CHT_PORT_AUTO_NEG_REG(devObjPtr, macPort), 7, 1, &autoNeg);
        if(autoNeg)
        {
            /* Link state UP */
            if(linkState)
            {
                if(devObjPtr->portsArr[port].state == SKERNEL_PORT_STATE_GE_E)
                {
                    /* Port speed 1000 Mbps */
                    newSpeed = 1;
                }
                else
                {
                    /* Port speed 100 Mbps */
                    newSpeed = 2;
                }
            }
            else
            {
                /* Default speed 10 Mbps */
                newSpeed = 0;
            }

            regAddress = SMEM_CHT_PORT_STATUS0_REG(devObjPtr,macPort);
            smemRegFldSet(devObjPtr, regAddress, 1, 2, newSpeed);
        }

        /* GE Ports */
        /* set summary bit in the
           Tri-Speed Ports Interrupt Summary Register 0x05004010 */
        regAddress = SMEM_CHT_TRI_SPEED_PORTS_INTERRUPT_SUMMARY_REG(devObjPtr);
        smemRegGet(devObjPtr, regAddress, &regData);

        /* set per port bit */
        SMEM_U32_SET_FIELD(regData, (macPort + 1), 1, 1);

        /* set Summary of all port interrupts */
        SMEM_U32_SET_FIELD(regData, 0, 1, 1);

        smemRegSet(devObjPtr, regAddress, regData);

        /* check interrupts mask
          Tri-Speed Ports Interrupt Summary Mask Register
          Offset: 0x04805114 */
        regAddress = SMEM_CHT_TRI_SPEED_PORTS_INTERRUPT_SUMMARY_MASK_REG(devObjPtr);
        smemRegFldGet(devObjPtr, regAddress, (macPort + 1), 1, &fldValue);
        if (fldValue != 1)
        {
            /* do not trigger global interrupt but still fill the cause
               registers in the tree to allow the interrupts be ready when the
               mask register value will be changed ...
            */
            doInterrupt = 0;
        }

        fldPortBit = 5;

        /* generate interrupt */
        snetChetahDoInterruptLimited(devObjPtr,
                              SMEM_CHT_PORTS_MIB_CNT_INT_SUM_REG(devObjPtr),
                              SMEM_CHT_PORTS_MIB_CNT_INT_SUM_MASK_REG(devObjPtr),
                              (1 << fldPortBit),
                              portInGlobBmp,
                              doInterrupt);/* let the function know if to set interrupt or not */
    }
    else
    {
        /* XG Ports ,
            and also 'stacking ports' in GE mode :
            SKERNEL_PORT_STATE_GE_STACK_A0_E,
            SKERNEL_PORT_STATE_GE_STACK_A1_E
        */
        /*
            in Lion2 also support Gig port mode
        */

        /* set summary bit in the
           XG Ports Interrupt Cause register 0x00000080 */
        regAddress = SMEM_CHT3_XG_PORTS_INTERRUPT_CAUSE_REG(devObjPtr,0);
        smemRegGet(devObjPtr, regAddress, &regData);

        fldPortBit = macPort == 0 ? 31 : macPort;
        /* set per port bit */
        SMEM_U32_SET_FIELD(regData, fldPortBit, 1, 1);

        /* set Summary of all port interrupts */
        SMEM_U32_SET_FIELD(regData, 0, 1, 1);

        smemRegSet(devObjPtr, regAddress, regData);

        /* check interrupts mask
          XG Ports Interrupt mask register
          Offset: 0x00000084 */
        regAddress = SMEM_CHT3_XG_PORTS_INTERRUPT_MASK_REG(devObjPtr,0);

        fldPortBit &= 0x1f;/* patch for bobcat2 with port > 32 */

        smemRegFldGet(devObjPtr, regAddress, fldPortBit, 1, &fldValue);
        if (fldValue != 1)
        {
            /* do not trigger global interrupt but still fill the cause
               registers in the tree to allow the interrupts be ready when the
               mask register value will be changed ...
            */
            doInterrupt = 0;
        }

        if(SKERNEL_IS_LION2_DEV(devObjPtr) &&
           devObjPtr->portGroupSharedDevObjPtr == 0 &&
           (IS_CHT_HYPER_GIGA_PORT(devObjPtr,port) == 0))
        {
            /* patch for Bobcat2 and not for Lion3 , as not supporting GOP of Lion3 yet */
            /* those are ports without XLG mac , so can't trigger interrupt in the XLG mac */
        }
        else
        if( SKERNEL_IS_LION2_DEV(devObjPtr) || /* Lion2 support all modes (GE/XLG/HGL...)*/
            IS_CHT_HYPER_GIGA_PORT(devObjPtr,port))/* not 'Stacking GE' mode */
        {
            /* the xcat/lion device has another hierarchy of cause registers before
               the XG/GE port */
            /* set summary bit in the
               XG Ports Summary Interrupt Cause register 0x08800058 + port*0x400 */
            /* in Lion2 this register is summary for GE,XLG,HGL..*/
            regAddress = SMEM_LION_XG_PORTS_SUMMARY_INTERRUPT_CAUSE_REG(devObjPtr,macPort);
            smemRegGet(devObjPtr, regAddress, &regData);

            if(IS_CHT_HYPER_GIGA_PORT(devObjPtr,port))
            {
                /* set bit 1 <XG Mac Cause Summary> */
                fldPortBit = 1;
            }
            else/* flor Lion 2 - support GE port */
            {
                /* set bit 2 <Giga port Cause Summary> */
                fldPortBit = 2;
            }

            SMEM_U32_SET_FIELD(regData, fldPortBit, 1, 1);

            /* set Summary of the port XG/GE interrupt */
            SMEM_U32_SET_FIELD(regData, 0, 1, 1);

            smemRegSet(devObjPtr, regAddress, regData);

            if(SKERNEL_IS_LION2_DEV(devObjPtr))
            {

                /* check interrupts mask
                   set summary bit in the
                   XG Ports Summary Interrupt Mask register 0x088C0058 + port*0x1000 */
                regAddress = SMEM_LION2_XG_PORTS_SUMMARY_INTERRUPT_MASK_REG(devObjPtr,macPort);
                smemRegFldGet(devObjPtr, regAddress, fldPortBit, 1, &fldValue);
                if (fldValue != 1)
                {
                    /* do not trigger global interrupt but still fill the cause
                       registers in the tree to allow the interrupts be ready when the
                       mask register value will be changed ...
                    */
                    doInterrupt = 0;
                }
            }
            else
            {
                /**********************************************************/
                /* There is no 'mask register'  for this cause register ! */
                /* So the logic always as those bits are unmasked         */
                /**********************************************************/
            }
        }

        /* generate interrupt */
        snetChetahDoInterruptLimited(devObjPtr,
                              portIntCauseReg,
                              portIntMaskReg,
                              (1 << 1),
                              portInGlobBmp,
                              doInterrupt);/* let the function know if to set interrupt or not */

    }
}

/**
* @internal snetChtInterruptTxSdmaEnd function
* @endinternal
*
* @brief   generate TX SDMA END interrupt on specific queue
*/
static GT_VOID snetChtInterruptTxSdmaEnd
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32 txQue
)
{
    /* Generate interrupt Tx DMA processed all descriptors in queue */
     /* this interrupt in MG should go to all interrupt trees of the device */
    snetChtDoInterruptInMgInAllTrees(devObjPtr,
       SMEM_CHT_TX_SDMA_INT_CAUSE_REG(devObjPtr,0),
        SMEM_CHT_TX_SDMA_INT_MASK_REG(devObjPtr,0),
        SMEM_CHT_TX_END_INT(txQue),
        SMEM_CHT_TX_SDMA_SUM_INT(devObjPtr));
}


/**
* @internal snetChtFromCpuDmaProcess function
* @endinternal
*
* @brief   Process transmitted SDMA queue frames in the Cheetah
*/
static GT_VOID snetChtFromCpuDmaProcess
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SBUF_BUF_ID bufferId
)
{
    GT_U32 startSdmaBmp;                        /* tx queue SDMA bitmap */
    GT_U32 txQue;                               /* index of queue */
    GT_U32 txSdmaCmdReg;                        /* Register entry value */
    SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr; /* frame descriptor pointer */
    GT_U32 i;                                   /* tx queue index */
    GT_U32 packetGenerationEnable;                       /* bypass owner bit is enabled */
    GT_BOOL isLastPacket;                       /* is last packet in chain */
    GT_U32 * txSdmaPktGenCfgRegPtr;             /* Pointer to register entry value */
    GT_U32  mgUnit = smemGetCurrentMgUnitIndex(devObjPtr);

    /*state from cpu command ()*/
    simLogPacketFrameCommandSet(SIM_LOG_FRAME_COMMAND_TYPE_FROM_CPU_E);

    descrPtr = snetChtEqDuplicateDescr(devObjPtr, NULL);
    /* save the ingress device*/
    descrPtr->ingressDevObjPtr = devObjPtr;

    packetGenerationEnable = 0;

    descrPtr->frameBuf = bufferId;

    /* Get tx SMDA queue bitmap */
    startSdmaBmp = bufferId->userInfo.data.txSdmaQueueBmp;

    /* Transmit SDMA Queue Command Register */
    smemRegGet(devObjPtr, SMEM_CHT_TX_SDMA_QUE_CMD_REG(devObjPtr), &txSdmaCmdReg);

    /* Scan all 8 bits in SDMA queue bitmap starting from highest */
    for (i = 0; i < SNET_CHT_SDMA_TX_QUEUE_MAX_NUMBER; i++)
    {
        txQue = SNET_CHT_SDMA_TX_QUEUE_MAX_NUMBER - (i+1);

        if (((txSdmaCmdReg >> (txQue + 8)) & 1) == 1)
        {
            /* Queue disabled */
            continue;
        }

        if (((startSdmaBmp >> txQue) & 1) == 0)
        {
            /* Queue was not triggered */
            continue;
        }


        if(devObjPtr->oamSupport.keepAliveSupport)
        {
            /* Tx SDMA Packet Generator Config Queue */
            txSdmaPktGenCfgRegPtr =
                &devObjPtr->sdmaTransmitInfo[mgUnit].sdmaTransmitData[txQue].txQueuePacketGenCfgReg;

            packetGenerationEnable = SMEM_U32_GET_FIELD(*txSdmaPktGenCfgRegPtr, 0, 1);
        }

        if (packetGenerationEnable == 0)
        {
            /* send all packets that are in this queue (fix CQ#119136) */
            do
            {
                snetChtFromCpuDmaTxQueue(devObjPtr, descrPtr, txQue, &isLastPacket);
                if (((txSdmaCmdReg >> (txQue + 8)) & 1) == 1)
                {
                    /* Generate interrupt in case if Tx SDMA stopped processing
                    after setting DISQ bit */
                    snetChtInterruptTxSdmaEnd(devObjPtr,txQue);
                    break;
                }
            } while (isLastPacket == GT_FALSE);
        }
        else
        {
            /* Bypass owner bit is enabled */
            if(devObjPtr->sdmaTransmitInfo[mgUnit].sdmaTransmitData[txQue].taskHandle == 0)
            {
                devObjPtr->sdmaTransmitInfo[mgUnit].sdmaTransmitData[txQue].mgUnit = mgUnit;
                devObjPtr->sdmaTransmitInfo[mgUnit].sdmaTransmitData[txQue].txQueue = txQue;
                devObjPtr->sdmaTransmitInfo[mgUnit].sdmaTransmitData[txQue].devObjPtr = devObjPtr;
                /* Create SKernel task */
                devObjPtr->sdmaTransmitInfo[mgUnit].sdmaTransmitData[txQue].taskHandle =
                    SIM_OS_MAC(simOsTaskCreate)(
                        GT_TASK_PRIORITY_NORMAL,
                        (unsigned (__TASKCONV *)(void*))snetLion3SdmaTaskPerQueue,
                        (void *)&devObjPtr->sdmaTransmitInfo[mgUnit].sdmaTransmitData[txQue]);

                if (devObjPtr->sdmaTransmitInfo[mgUnit].sdmaTransmitData[txQue].taskHandle == NULL)
                {
                    skernelFatalError(" snetChtFromCpuDmaProcess: cannot create SDMA transmission task for"\
                                       " queue %u", txQue);
                }

                SCIB_SEM_TAKE;
                devObjPtr->numThreadsOnMe++;
                SCIB_SEM_SIGNAL;
            }
        }
    }

}

/**
* @internal snetChtFromCpuDmaTxQueue function
* @endinternal
*
* @brief   Process transmitted frames per single SDMA queue
*/
GT_VOID snetChtFromCpuDmaTxQueue
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32 txQue,
    OUT GT_BOOL *isLastPacketPtr
)
{
    DECLARE_FUNC_NAME(snetChtFromCpuDmaTxQueue);

    GT_U32  mutexUsed;
    GT_BOOL queueEmpty;                         /* queue is empty */
    GT_U32  mgUnit = smemGetCurrentMgUnitIndex(devObjPtr);

    /* packet from cpu (DMA)*/
    __LOG((SIM_LOG_INGRESS_PACKET_FROM_CPU_DMA_STR " \n"));

    /* increment the number of packets in the system */
    skernelNumOfPacketsInTheSystemSet(GT_TRUE);

    /* consider as RXDMA of the CPU port */
    simLogPacketFrameUnitSet(SIM_LOG_FRAME_UNIT_RXDMA_E);

    SIM_LOG_PACKET_DESCR_SAVE

    /* Fill frame descriptor from Tx SDMA descriptor */
    snetChtPerformFromCpuDma(devObjPtr, txQue, descrPtr, &queueEmpty, isLastPacketPtr);

    SIM_LOG_PACKET_DESCR_COMPARE("snetChtPerformFromCpuDma : prepare packet from CPU SDMA");

    if(queueEmpty == GT_FALSE)
    {
        if(devObjPtr->portGroupSharedDevObjPtr || simLogIsOpenFlag)
        {
            mutexUsed = 1;
            /* protect the full processing */
            SIM_OS_MAC(simOsMutexLock)(LOG_fullPacketWalkThroughProtectMutex);
        }
        else
        {
            mutexUsed = 0;
        }

        snetChtIngress(devObjPtr, descrPtr);

        /* restore the current MG */
        smemSetCurrentMgUnitIndex(devObjPtr,mgUnit);

        if(mutexUsed)
        {
            /* protect the egress processing */
            SIM_OS_MAC(simOsMutexUnlock)(LOG_fullPacketWalkThroughProtectMutex);
        }
    }

    /* decrement the number of packets in the system */
    skernelNumOfPacketsInTheSystemSet(GT_FALSE);

    /* packet from cpu (DMA) - Ended */
    __LOG((SIM_LOG_INGRESS_PACKET_FROM_CPU_DMA_ENDED_STR " \n"));
}

/* log the read/write DMA to log */
static void logScibDmaOperation
(
    IN SNET_CHT_DMA_CLIENT_ENT clientName,
    IN GT_BOOL doWrite,
    IN GT_U32 deviceId,
    IN GT_U32 address_high,
    IN GT_U32 address_low,
    IN GT_U32 memSize,
    IN GT_U32 * memPtr,
    IN GT_U32  dataIsWords
)
{
    GT_U32  ii;
    static GT_CHAR *operName[2] = {SIM_LOG_DMA_OPERATION_WRITE_TO_STR , SIM_LOG_DMA_OPERATION_READ_FROM_STR};
    GT_U32  index = (doWrite == GT_TRUE) ? 0 : 1;
    static GT_CHAR *clientNamesArr[SNET_CHT_DMA_CLIENT___LAST___E+1] = {
             STR(SNET_CHT_DMA_CLIENT_PACKET_FROM_CPU_E)
            ,STR(SNET_CHT_DMA_CLIENT_PACKET_TO_CPU_E)
            ,STR(SNET_CHT_DMA_CLIENT_AUQ_E)
            ,STR(SNET_CHT_DMA_CLIENT_FUQ_E)
            ,STR(SNET_CHT_DDR_CM3_E)
            ,STR(SNET_CHT_DMA_CLIENT_CNC_E)
            ,STR(SNET_CHT_DMA_CLIENT_IPE_E)
            ,"unknown client"
        };
    GT_CHAR *clientNamePtr;

    if(clientName >= SNET_CHT_DMA_CLIENT___LAST___E)
    {
        /*"unknown client"*/
        clientNamePtr = clientNamesArr[SNET_CHT_DMA_CLIENT___LAST___E];
    }
    else
    {
        clientNamePtr = clientNamesArr[clientName];
    }

    scibAccessLock();

    simLogInfoSave(NULL,NULL,0,NULL,SIM_LOG_INFO_TYPE_MEMORY_E);
    simLogInternalLog(SIM_LOG_DMA_OPERATION_STR " %s ] the DMA by[ %s ] [%d] words: \n",
        operName[index],
        clientNamePtr,
        memSize
        );

    for(ii = 0 ; ii < memSize; ii++)
    {
        if(address_high == 0)
        {
            simLogInternalLog("address[0x%8.8x] value[0x%8.8x] \n",
                address_low + (4*ii) , memPtr[ii]);
        }
        else
        {
            simLogInternalLog("address_high[0x%8.8x] address_low[0x%8.8x] value[0x%8.8x] \n",
                address_high , address_low + (4*ii) , memPtr[ii]);
        }
    }

    scibAccessUnlock();
}


/**
* @internal snetChtPerformScibDmaWrite function
* @endinternal
*
* @brief   wrap the scibDmaWrite to allow the LOG parser to emulate the DMA:
*         write to HOST CPU DMA memory function.
*         Asic is calling this function to write DMA.
* @param[in] clientName               - the DMA client name
* @param[in] deviceId                 - device id. (of the device in the simulation)
* @param[in] address                  - physical  that PP refer to.
*                                      HOST CPU must convert it to HOST memory address
* @param[in] memSize                  - number of words of ASIC memory to write .
* @param[in] memPtr                   - (pointer to) data to write to HOST CPU memory.
* @param[in] dataIsWords              - the data to read is words or bytes
*                                      1 - words --> swap network order to cpu order
*                                      0 - bytes --> NO swap network order to cpu order
*/
void snetChtPerformScibDmaWrite
(
    IN SNET_CHT_DMA_CLIENT_ENT clientName,
    IN GT_U32 deviceId,
    IN GT_U32 address,
    IN GT_U32 memSize,
    IN GT_U32 * memPtr,
    IN GT_U32  dataIsWords
)
{
    SKERNEL_DEVICE_OBJECT    *devObjPtr = smemTestDeviceIdToDevPtrConvert(deviceId);
    GT_U32 addr_low=address,addr_high=0;

    if(devObjPtr->deviceForceBar0Bar2)
    {
        SMEM_CHT_GENERIC_DEV_MEM_INFO *devMemInfoPtr = devObjPtr->deviceMemory;
        SMEM_DMA_CNM_OATU_CLIENT_ENT client = ((clientName == SNET_CHT_MEM_OVER_PCIe_CLIENT_CM3_E) ?
            SMEM_DMA_CNM_OATU_CLIENT_CM3_E :
            SMEM_DMA_CNM_OATU_CLIENT_SDMA_E);

        if(NULL == devMemInfoPtr->common.dmaCnMoAtuFunc)
        {
            skernelFatalError("snetChtPerformScibDmaWrite: dmaCnMoAtuFunc not implemented ?!\n" );
            return;
        }

        /* convert address of AUQ/TxSdma/RxSdma/other to address that goes on the 'PEX'
           toward the DMA in the CPU periphery .
           this address may be 64 bits (if the CPU) supports it */
        devMemInfoPtr->common.dmaCnMoAtuFunc(devObjPtr,client,address,&addr_low,&addr_high);
        if(addr_low == SMAIN_NOT_VALID_CNS)
        {
            /* the address did not find proper window for 'convert' */
            __LOG_NO_LOCATION_META_DATA__WITH_SCIB_LOCK(("DMA WRITE ERROR : clientName[%d] address [0x%8.8x] not find proper window for 'convert' \n",
                clientName,address));
            /* ignore the write */
        }
        else
        {
            scibDmaWrite64BitAddr(addr_high,addr_low,memSize,memPtr);
        }
    }
    else
    if(simLogToRuntimeIsActive)
    {
        simLogToRuntime_scibDmaWrite(clientName,deviceId,address,memSize,memPtr,dataIsWords);
    }
    else
    {
        scibDmaWrite(deviceId,address,memSize,memPtr,dataIsWords);
    }

    if(simLogIsOpenFlag)
    {
        logScibDmaOperation(clientName,GT_TRUE/*write*/,deviceId,addr_high,addr_low,memSize,memPtr,dataIsWords);
    }

}



/**
* @internal snetChtPerformScibDmaRead function
* @endinternal
*
* @brief   wrap the scibDmaRead to allow the LOG parser to emulate the DMA:
*         write to HOST CPU DMA memory function.
*         Asic is calling this function to write DMA.
* @param[in] clientName               - the DMA client name
* @param[in] deviceId                 - device id. (of the device in the simulation)
* @param[in] address                  - physical  that PP refer to.
*                                      HOST CPU must convert it to HOST memory address
* @param[in] memSize                  - the number of words/bytes (according to dataIsWords)
* @param[in] dataIsWords              - the data to read is words or bytes
*                                      1 - words --> swap network order to cpu order
*                                      0 - bytes --> NO swap network order to cpu order
*
* @param[out] memPtr                   - (pointer to) PP's memory in which HOST CPU memory will be
*                                      copied.
*                                      RETURN:
*                                      COMMENTS:
*/
void snetChtPerformScibDmaRead
(
    IN SNET_CHT_DMA_CLIENT_ENT clientName,
    IN GT_U32 deviceId,
    IN GT_U32 address,
    IN GT_U32 memSize,
    OUT GT_U32 * memPtr,
    IN GT_U32  dataIsWords
)
{
    SKERNEL_DEVICE_OBJECT    *devObjPtr = smemTestDeviceIdToDevPtrConvert(deviceId);
    GT_U32 addr_low=address,addr_high=0;

    if(devObjPtr->deviceForceBar0Bar2)
    {
        SMEM_CHT_GENERIC_DEV_MEM_INFO *devMemInfoPtr = devObjPtr->deviceMemory;
        GT_U32 ii;
        SMEM_DMA_CNM_OATU_CLIENT_ENT client = ((clientName == SNET_CHT_MEM_OVER_PCIe_CLIENT_CM3_E) ?
            SMEM_DMA_CNM_OATU_CLIENT_CM3_E :
            SMEM_DMA_CNM_OATU_CLIENT_SDMA_E);

        if(NULL == devMemInfoPtr->common.dmaCnMoAtuFunc)
        {
            skernelFatalError("snetChtPerformScibDmaWrite: dmaCnMoAtuFunc not implemented ?!\n" );
            return;
        }
        /* convert address of AUQ/TxSdma/RxSdma/other to address that goes on the 'PEX'
           toward the DMA in the CPU periphery .
           this address may be 64 bits (if the CPU) supports it */
        devMemInfoPtr->common.dmaCnMoAtuFunc(devObjPtr,client,address,&addr_low,&addr_high);

        if(addr_low == SMAIN_NOT_VALID_CNS)
        {
            /* the address did not find proper window for 'convert' */
            __LOG_NO_LOCATION_META_DATA__WITH_SCIB_LOCK(("DMA READ ERROR : clientName[%d] address [0x%8.8x] not find proper window for 'convert' \n",
                clientName,address));

            for(ii = 0 ; ii < memSize ; ii++)
            {
                /* ignore the read , and assign bad values to it */
                memPtr[ii] = SMAIN_NOT_VALID_CNS;
            }
        }
        else
        {
            scibDmaRead64BitAddr(addr_high,addr_low,memSize,memPtr);
        }
    }
    else
    if(simLogToRuntimeIsActive)
    {
        simLogToRuntime_scibDmaRead(clientName,deviceId,address,memSize,memPtr,dataIsWords);
    }
    else
    {
        scibDmaRead(deviceId,address,memSize,memPtr,dataIsWords);
    }

    if(simLogIsOpenFlag)
    {
        logScibDmaOperation(clientName,GT_FALSE/*read*/,deviceId,addr_high,addr_low,memSize,memPtr,dataIsWords);
    }
}

/**
* @internal snetChtPerformScibDmaRead_64BitAddr  function
* @endinternal
*
* @brief   wrap the scibDmaRead to allow the LOG parser to emulate the DMA:
*         write to HOST CPU DMA memory function.
*         Asic is calling this function to write DMA.
* @param[in] clientName               - the DMA client name
* @param[in] subClientId              - the GDMA unit id or CM7 unit id (relative to the CnM --> not 'global to the device' in multi-tiles)
* @param[in] deviceId                 - device id. (of the device in the simulation)
* @param[in] address_low              - physical  that PP refer to. (lower 32 bits : 0..31)
*                                      HOST CPU must convert it to HOST memory address
* @param[in] address_high             - physical  that PP refer to. (higher 32 bits : 32..63)
*                                      HOST CPU must convert it to HOST memory address
* @param[in] memSize                  - the number of words/bytes (according to dataIsWords)
* @param[in] dataIsWords              - the data to read is words or bytes
*                                      1 - words --> swap network order to cpu order
*                                      0 - bytes --> NO swap network order to cpu order
*
* @param[out] memPtr                   - (pointer to) PP's memory in which HOST CPU memory will be
*                                      copied.
*                                      RETURN:
*                                      COMMENTS:
*/
void snetChtPerformScibDmaRead_64BitAddr
(
    IN SNET_CHT_DMA_CLIENT_ENT clientName,
    IN GT_U32 subClientId,
    IN GT_U32 deviceId,
    IN GT_U32 address_low,
    IN GT_U32 address_high,
    IN GT_U32 memSize,
    OUT GT_U32 * memPtr,
    IN GT_U32  dataIsWords
)
{
    SKERNEL_DEVICE_OBJECT    *devObjPtr = smemTestDeviceIdToDevPtrConvert(deviceId);
    SMEM_CHT_GENERIC_DEV_MEM_INFO *devMemInfoPtr = devObjPtr->deviceMemory;
    SMEM_DMA_CNM_OATU_CLIENT_ENT client = ((clientName == SNET_CHT_MEM_OVER_PCIe_CLIENT_CM3_E) ?
        SMEM_DMA_CNM_OATU_CLIENT_CM3_E :
        SMEM_DMA_CNM_OATU_CLIENT_SDMA_E);

    if(NULL == devMemInfoPtr->common.dmaCnMoAtuFunc)
    {
        skernelFatalError("snetChtPerformScibDmaRead_64BitAddr: dmaCnMoAtuFunc not implemented ?!\n" );
        return;
    }
    /* convert address of AUQ/TxSdma/RxSdma/other to address that goes on the 'PEX'
       toward the DMA in the CPU periphery .
       this address may be 64 bits (if the CPU) supports it */
    devMemInfoPtr->common.dmaCnMoAtuFunc(devObjPtr,client,subClientId,
        INOUT&address_low,INOUT&address_high);

    if(address_high == 0)
    {
        /* get use of 'scibDmaUpper32Bits' */
        scibDmaRead(deviceId,address_low,memSize,memPtr,dataIsWords);
    }
    else
    {
        scibDmaRead64BitAddr(address_high,address_low,memSize,memPtr);
    }

    if(simLogIsOpenFlag)
    {
        logScibDmaOperation(clientName,GT_FALSE/*read*/,deviceId,address_high,address_low,memSize,memPtr,dataIsWords);
    }
}

/**
* @internal snetChtPerformScibDmaWrite_64BitAddr  function
* @endinternal
*
* @brief   wrap the scibDmaWrite to allow the LOG parser to emulate the DMA:
*         write to HOST CPU DMA memory function.
*         Asic is calling this function to write DMA.
* @param[in] clientName               - the DMA client name
* @param[in] subClientId              - the GDMA unit id or CM7 unit id (relative to the CnM --> not 'global to the device' in multi-tiles)
* @param[in] deviceId                 - device id. (of the device in the simulation)
* @param[in] address_low              - physical  that PP refer to. (lower 32 bits : 0..31)
*                                      HOST CPU must convert it to HOST memory address
* @param[in] address_high             - physical  that PP refer to. (higher 32 bits : 32..63)
*                                      HOST CPU must convert it to HOST memory address
* @param[in] memSize                  - number of words of ASIC memory to write .
* @param[in] memPtr                   - (pointer to) data to write to HOST CPU memory.
* @param[in] dataIsWords              - the data to read is words or bytes
*                                      1 - words --> swap network order to cpu order
*                                      0 - bytes --> NO swap network order to cpu order
*/
void snetChtPerformScibDmaWrite_64BitAddr
(
    IN SNET_CHT_DMA_CLIENT_ENT clientName,
    IN GT_U32 subClientId,
    IN GT_U32 deviceId,
    IN GT_U32 address_low,
    IN GT_U32 address_high,
    IN GT_U32 memSize,
    IN GT_U32 * memPtr,
    IN GT_U32  dataIsWords
)
{
    SKERNEL_DEVICE_OBJECT    *devObjPtr = smemTestDeviceIdToDevPtrConvert(deviceId);
    SMEM_CHT_GENERIC_DEV_MEM_INFO *devMemInfoPtr = devObjPtr->deviceMemory;
    SMEM_DMA_CNM_OATU_CLIENT_ENT client = ((clientName == SNET_CHT_MEM_OVER_PCIe_CLIENT_CM3_E) ?
        SMEM_DMA_CNM_OATU_CLIENT_CM3_E :
        SMEM_DMA_CNM_OATU_CLIENT_SDMA_E);

    if(NULL == devMemInfoPtr->common.dmaCnMoAtuFunc)
    {
        skernelFatalError("snetChtPerformScibDmaRead_64BitAddr: dmaCnMoAtuFunc not implemented ?!\n" );
        return;
    }
    /* convert address of AUQ/TxSdma/RxSdma/other to address that goes on the 'PEX'
       toward the DMA in the CPU periphery .
       this address may be 64 bits (if the CPU) supports it */
    devMemInfoPtr->common.dmaCnMoAtuFunc(devObjPtr,client,subClientId,
        INOUT&address_low,INOUT&address_high);

    if(address_high == 0)
    {
        /* get use of 'scibDmaUpper32Bits' */
        scibDmaWrite(deviceId,address_low,memSize,memPtr,dataIsWords);
    }
    else
    {
        scibDmaWrite64BitAddr(address_high,address_low,memSize,memPtr);
    }

    if(simLogIsOpenFlag)
    {
        logScibDmaOperation(clientName,GT_TRUE/*write*/,deviceId,address_high,address_low,memSize,memPtr,dataIsWords);
    }
}


/**
* @internal snetChtPerformScibCm3DdrRead function
* @endinternal
*
* @brief   wrap the snetChtPerformScibDmaRead to allow CM3 read DDR:
*         write to HOST CPU DMA memory function.
*         CM3  is calling this function to read  DDR.

* @param[in] deviceId                 - device id. (of the device in the simulation)
* @param[in] address                  - physical  that PP refer to.
*                                      HOST CPU must convert it to HOST memory address
* @param[in] memSize                  - the number of words
*
* @param[out] memPtr                   - (pointer to) PP's memory in which HOST CPU memory will be
*                                      copied.
*                                      RETURN:
*                                      COMMENTS:
*/
void snetChtPerformScibCm3DdrRead
(
    IN GT_U32 deviceId,
    IN GT_U32 address,
    IN GT_U32 memSize,
    OUT GT_U32 * memPtr
)
{
    snetChtPerformScibDmaRead(SNET_CHT_MEM_OVER_PCIe_CLIENT_CM3_E,deviceId,
         address,memSize,memPtr,1);
}


/**
* @internal snetChtPerformScibSetInterrupt function
* @endinternal
*
* @brief   wrap the scibSetInterrupt to allow the LOG parser to emulate the interrupt:
*         Generate interrupt for SKernel device.
* @param[in] deviceId                 - ID of device.
* @param[in] treeId                   - support the 3 interrupt trees that the device gives.
*/
void snetChtPerformScibSetInterrupt
(
    IN  GT_U32        deviceId,
    IN  GT_U32        treeId
)
{
    SKERNEL_DEVICE_OBJECT    *devObjPtr = smemTestDeviceIdToDevPtrConvert(deviceId);
    if(simLogToRuntimeIsActive)
    {
        simLogToRuntime_scibSetInterrupt(deviceId, treeId);
    }
    else
    {
        scibSetInterrupt(deviceId, treeId);
    }

    __LOG_NO_LOCATION_META_DATA__WITH_SCIB_LOCK(("Set interrupt by dev[%d] treeId[%d]\n",
        deviceId,treeId));

}
/**
* @internal snetChtPerformScibUnSetInterrupt function
* @endinternal
*
* @brief   wrap the scibSetInterrupt to allow the LOG parser to emulate the de-assert of interrupt.
* @param[in] deviceId                 - ID of device.
* @param[in] treeId                   - support the 3 interrupt trees that the device gives.
*/
void snetChtPerformScibUnSetInterrupt
(
    IN  GT_U32        deviceId,
    IN  GT_U32        treeId
)
{
    SKERNEL_DEVICE_OBJECT    *devObjPtr = smemTestDeviceIdToDevPtrConvert(deviceId);

    SIM_TBD_BOOKMARK /* how to notify the CPU about 'treeId' */
    scibUnSetInterrupt(deviceId);

    __LOG_NO_LOCATION_META_DATA__WITH_SCIB_LOCK(("Unset (de-assert) interrupt by dev[%d] treeId[%d] \n",
        deviceId,treeId));
}

/**
* @internal snetChtPerformFromCpuDma function
* @endinternal
*
* @brief   Copy frame data from CPU memory to bufferId
*/
static GT_VOID snetChtPerformFromCpuDma
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32 txQue,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    OUT GT_BOOL * queueEmptyPtr,
    OUT GT_BOOL * isLastPacketPtr
)
{
    DECLARE_FUNC_NAME(snetChtPerformFromCpuDma);

    GT_U32 fldValue;                   /* register field value */
    GT_U32 copySize;                   /* size of copied data */
    SNET_STRUCT_TX_DESC * txDescPtr;   /* pointer to Tx descriptor */
    GT_U8 * dataPtr;                   /* pointer to buffer */
    SNET_STRUCT_TX_DESC txDesc;        /* buffer for TX descriptor */
    SNET_STRUCT_TX_DESC nextTxDesc;    /* buffer for next TX descriptor */
    GT_U32  currentDmaAddr;            /* current address for DMA */
    GT_BOOL resetEnqBit;               /* need to reset ENQ bit */
    GT_U32 *txSdmaPktGenCfgRegPtr;         /* Register data */
    GT_U32 packetGenerationEnable;              /* Bypass owner bit updating */
    GT_BIT  isLast;                    /* is last descriptor */
    GT_U32 nextDescPointer;            /* the next descriptor 'pointer' */
    GT_U32  regAddr_CUR_DESC_PTR0 =  SMEM_CHT_TX_SDMA_CUR_DESC_PTR0_REG(devObjPtr) + (txQue * 0x4);
    GT_U32  mgUnit = smemGetCurrentMgUnitIndex(devObjPtr);

    /* init to indicate there are no more frames in the queue */
    *queueEmptyPtr = GT_TRUE;
    *isLastPacketPtr = GT_TRUE;

    /* read descriptor address of appropriate queue */
    smemRegGet(devObjPtr,regAddr_CUR_DESC_PTR0, &fldValue);

    /*txDescPtr = (SNET_STRUCT_TX_DESC *)fldValue;*/
    currentDmaAddr = fldValue;
    /* Init DMA first and last descriptor address */
    txDescPtr = &txDesc;

    /* Do not bypass - default mode */
    packetGenerationEnable = 0;
    if(devObjPtr->oamSupport.keepAliveSupport)
    {
        /* Tx SDMA Packet Generator Config Queue */
        txSdmaPktGenCfgRegPtr =
            &devObjPtr->sdmaTransmitInfo[mgUnit].sdmaTransmitData[txQue].txQueuePacketGenCfgReg;

        packetGenerationEnable = SMEM_U32_GET_FIELD(*txSdmaPktGenCfgRegPtr, 0, 1);
    }

    if(currentDmaAddr == 0)
    {
        /* Generate interrupt Tx DMA stopped processing the queue */
        /* this interrupt in MG should go to all interrupt trees of the device */
        snetChtDoInterruptInMgInAllTrees(devObjPtr,
                              SMEM_CHT_TX_SDMA_INT_CAUSE_REG(devObjPtr,0),
                              SMEM_CHT_TX_SDMA_INT_MASK_REG(devObjPtr,0),
                              SMEM_CHT_TX_ERROR_INT(txQue),
                              SMEM_CHT_TX_SDMA_SUM_INT(devObjPtr));

        /* although there is special interrupt for this case we need
           the FATAL ERROR --- because this state indicate that there is error
           in PSS */
        skernelFatalError("snetChtPerformFromCpuDma: NULL descriptor \n");
        return ;
    }

    /* read the DMA for needed value , put data into txDescPtr */
    snetChtPerformScibDmaRead(SNET_CHT_DMA_CLIENT_PACKET_FROM_CPU_E,
        devObjPtr->deviceId,currentDmaAddr,NUM_BYTES_TO_WORDS(sizeof(*txDescPtr)), (GT_U32*)txDescPtr,SCIB_DMA_WORDS);

    /* Indicates first buffer of a frame */
    if(TX_DESC_GET_FIRST_BIT(txDescPtr) == 0)
    {
        /* the SW did not prepared descriptors yet.
          This is legal situation when SW prepares packets to send without
          triggering. And only after this (may be in scope of other task)
          it triggers sending of batch of packets .*/

        /* although 'valid' ... lets give LOG 'warning' , because not sure that is was intended ! */
        __LOG(("WARNING : the SW did not prepared descriptors for the 'SDMA Transmit' yet. ignore the 'trigger' on queue[%d] on MG[%d]\n",
            txQue,mgUnit));

        /* Resets the matching ENQ bit */
        smemRegFldSet(devObjPtr, SMEM_CHT_TX_SDMA_QUE_CMD_REG(devObjPtr), txQue, 1, 0);
        return;
    }

    /* Point to the beginning of buffer */
    dataPtr = descrPtr->frameBuf->actualDataPtr;
    copySize = 0;

    while(currentDmaAddr != 0)
    {
        /* Buffer is owned by CPU */
        if (TX_DESC_GET_OWN_BIT(txDescPtr) == TX_DESC_CPU_OWN)
        {
            /* Resets the matching ENQ bit */
            smemRegFldSet(devObjPtr, SMEM_CHT_TX_SDMA_QUE_CMD_REG(devObjPtr), txQue, 1, 0);

            /* Generate interrupt Tx DMA stopped processing the queue */
            __LOG(("Generate interrupt Tx DMA stopped processing the queue"));
            /* this interrupt in MG should go to all interrupt trees of the device */
            snetChtDoInterruptInMgInAllTrees(devObjPtr,
                                  SMEM_CHT_TX_SDMA_INT_CAUSE_REG(devObjPtr,0),
                                  SMEM_CHT_TX_SDMA_INT_MASK_REG(devObjPtr,0),
                                  SMEM_CHT_TX_ERROR_INT(txQue),
                                  SMEM_CHT_TX_SDMA_SUM_INT(devObjPtr));

            smemRegSet(devObjPtr,regAddr_CUR_DESC_PTR0,currentDmaAddr);
            return;
        }

        /* Byte Count */
        fldValue = SMEM_U32_GET_FIELD(txDescPtr->word2, 16, 14);
        __LOG(("<Byte Count> : Added [%d] bytes to the 'SDMA packet' from current descriptor \n ",fldValue));


        copySize += fldValue;
        if(copySize > SBUF_DATA_SIZE_CNS)
        {
            /* we can't support packet larger then SBUF_DATA_SIZE_CNS */
            skernelFatalError("snetChtPerformFromCpuDma: packet too long \n");
        }

        /*memcpy(dataPtr, (GT_U32 *)txDescPtr->buffPointer, fldValue);*/
        snetChtPerformScibDmaRead(SNET_CHT_DMA_CLIENT_PACKET_FROM_CPU_E,
            devObjPtr->deviceId,txDescPtr->buffPointer,NUM_BYTES_TO_WORDS(fldValue),(GT_U32*)dataPtr,
                RX_BYTE_ORDER_MAC(devObjPtr));

        dataPtr += fldValue;

        /* Update next descriptor to feed.
          Do it before return ownership on descr to CPU in order to prevent race
          condition on the register between "CPU" tasks and simulation tasks.
          The "CPU" task may pool Own bit as indication that device finished
          descriptor processing. And "CPU" task changes current descriptor
          register when device finished processing of descriptor. Therefore
          simulation should update register before change the Own bit.  */
        smemRegSet(devObjPtr,regAddr_CUR_DESC_PTR0,txDescPtr->nextDescPointer);

        /* Last buffer of frame. Need to check next descriptor. And need to
          resets the matching ENQ bit if next descriptor is not ready.
          Do it before return ownership on descr to CPU in order to prevent race
          condition. */
        isLast = TX_DESC_GET_LAST_BIT(txDescPtr);
        /* get value before update the ownership of the descriptor */
        nextDescPointer = txDescPtr->nextDescPointer;

        if (isLast)
        {
            if (txDescPtr->nextDescPointer)
            {
                /* read next descriptor*/
                snetChtPerformScibDmaRead(SNET_CHT_DMA_CLIENT_PACKET_FROM_CPU_E,
                    devObjPtr->deviceId,txDescPtr->nextDescPointer,NUM_BYTES_TO_WORDS(sizeof(nextTxDesc)), (GT_U32*)&nextTxDesc,SCIB_DMA_WORDS);
                if((TX_DESC_GET_FIRST_BIT(&nextTxDesc) == 0) ||
                   (TX_DESC_GET_OWN_BIT(&nextTxDesc) == TX_DESC_CPU_OWN))
                {
                    /* next descriptor is not ready. Need to reset ENQ bit. */
                    resetEnqBit = GT_TRUE;
                }
                else
                {
                    /* next descriptor is ready. Do not reset ENQ bit. */
                    resetEnqBit = GT_FALSE;
                    *isLastPacketPtr = GT_FALSE;
                }
            }
            else
            {
                /* current descriptor is last one. Need to reset ENQ bit. */
                resetEnqBit = GT_TRUE;
            }

            if (resetEnqBit == GT_TRUE)
            {
                /* Resets the matching ENQ bit */
                smemRegFldSet(devObjPtr, SMEM_CHT_TX_SDMA_QUE_CMD_REG(devObjPtr), txQue, 1, 0);
            }
        }

        /* Do not bypass */
        if(packetGenerationEnable == 0)
        {
            /* Return ownership on descr to CPU to enable release of descr */
            TX_DESC_SET_OWN_BIT(txDescPtr, TX_DESC_CPU_OWN);

            /* write back to DMA the changes in the descriptor */
            __LOG(("write the DMA for return 'ownership' to CPU \n"));
            snetChtPerformScibDmaWrite(SNET_CHT_DMA_CLIENT_PACKET_FROM_CPU_E,
                         devObjPtr->deviceId,currentDmaAddr,
                         1/* update first word txDescPtr->word1*/,
                         (GT_U32*)txDescPtr,
                         SCIB_DMA_WORDS);
        }

        /*txDescPtr = (SNET_STRUCT_TX_DESC * )txDescPtr->nextDescPointer;*/
        currentDmaAddr = nextDescPointer;

        /* Last buffer of frame */
        if (isLast)
        {
            __LOG(("Last descriptor with last buffer of frame \n"));
            break;
        }

        if(currentDmaAddr)
        {
            /* read the DMA for needed value , put data into txDescPtr */
            __LOG(("read the DMA for next descriptor \n"));
            snetChtPerformScibDmaRead(SNET_CHT_DMA_CLIENT_PACKET_FROM_CPU_E,
                devObjPtr->deviceId,currentDmaAddr,NUM_BYTES_TO_WORDS(sizeof(*txDescPtr)), (GT_U32*)txDescPtr,SCIB_DMA_WORDS);
        }
    }

    if (SMEM_CHT_IS_SIP6_30_GET(devObjPtr))
    {
        /* Ironman Tx Handle sends packet from CPU without CRC */
        copySize += 4;
    }

    __LOG(("Total [%d] bytes for the 'SDMA packet' \n ",copySize));

    descrPtr->frameBuf->actualDataSize = copySize;

    /* Last buffer has received and interrupt is enable */
    if (currentDmaAddr && TX_DESC_GET_INT_BIT(txDescPtr))
    {
        /* Generate interrupt Tx DMA stopped processing the queue */
        /* this interrupt in MG should go to all interrupt trees of the device */
        snetChtDoInterruptInMgInAllTrees(devObjPtr,
                              SMEM_CHT_TX_SDMA_INT_CAUSE_REG(devObjPtr,0),
                              SMEM_CHT_TX_SDMA_INT_MASK_REG(devObjPtr,0),
                              SMEM_CHT_TX_BUFFER_INT(txQue),
                              SMEM_CHT_TX_SDMA_SUM_INT(devObjPtr));

    }

    /* Set CPU port for local device source port */
    descrPtr->localDevSrcPort = SNET_CHT_CPU_PORT_CNS;

    if(devObjPtr->dmaNumOfCpuPort)
    {
        /* the packet came from the DMA of the CPU */
        descrPtr->ingressRxDmaPortNumber = smemMultiDpGlobalCpuSdmaNumFromCurrentMgGet(devObjPtr);
        descrPtr->localDevSrcPort = descrPtr->ingressRxDmaPortNumber;
    }
    else
    {
        /* get the RXDMA port number */
        descrPtr->ingressRxDmaPortNumber = SMEM_LION2_LOCAL_PORT_TO_RX_DMA_PORT_MAC(devObjPtr,descrPtr->localDevSrcPort);
    }

    if(devObjPtr->numOfPipes)
    {
        GT_U32  dummyPort;
        /* Update 'currentPipeId' and get new (local) port */
        smemConvertGlobalPortToCurrentPipeId(devObjPtr,descrPtr->ingressRxDmaPortNumber,&dummyPort);
        /* restore the MG index that may changed in smemConvertGlobalPortToCurrentPipeId() */
        smemSetCurrentMgUnitIndex(devObjPtr,mgUnit);
    }


    if(devObjPtr->cpuPortNoSpecialMac)
    {
        /* the SDMA CPU port does not hold 'GOP' !!! */
        descrPtr->ingressGopPortNumber = SMAIN_NOT_VALID_CNS;/* value to cause fatal error for using 'GOP' when should not */
    }
    else
    {
        /* get the MAC port number */
        descrPtr->ingressGopPortNumber = snetChtRxMacPortGet(devObjPtr,descrPtr->localDevSrcPort);
    }

    __LOG_PARAM_NO_LOCATION_META_DATA(descrPtr->localDevSrcPort);
    __LOG_PARAM_NO_LOCATION_META_DATA(descrPtr->ingressRxDmaPortNumber);
    __LOG_PARAM_NO_LOCATION_META_DATA(descrPtr->ingressGopPortNumber);

    /* Indicate frame received through DMA mechanism */
    descrPtr->isFromSdma = GT_TRUE;

    /* there might be more frames in the queue */
    *queueEmptyPtr = GT_FALSE;

    if (*isLastPacketPtr == GT_TRUE)
    {
        /* Generate interrupt Tx DMA processed all descriptors in queue */
        snetChtInterruptTxSdmaEnd(devObjPtr,txQue);
    }

    return;
}
/**
* @internal snetChtPerformFromCpuDma_forPipeDevice function
* @endinternal
*
* @brief   needed by PIPE device : Copy frame data from CPU memory to bufferId
*/
GT_VOID snetChtPerformFromCpuDma_forPipeDevice
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32 txQue,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    OUT GT_BOOL * queueEmptyPtr,
    OUT GT_BOOL * isLastPacketPtr
)
{
    snetChtPerformFromCpuDma(devObjPtr,txQue,descrPtr,queueEmptyPtr,isLastPacketPtr);
}

/**
* @internal snetChtInterruptTxSdmaEnd_forPipeDevice function
* @endinternal
*
* @brief   needed by PIPE device : generate TX SDMA END interrupt on specific queue
*/
GT_VOID snetChtInterruptTxSdmaEnd_forPipeDevice
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32 txQue
)
{
    snetChtInterruptTxSdmaEnd(devObjPtr,txQue);
}

/**
* @internal snetChtTTermination function
* @endinternal
*
* @brief   T.T Engine processing for outgoing frame on Cheetah2/Cheetah3
*         asic simulation.
*         T.T processing, T.T assignment, key forming, 1 Lookup,
*         actions to descriptor processing
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] descrPtr             - in : pointer to frame data buffer Id
*                                       out: pointer to updated frame data buffer Id
* @param[in] internalTtiInfoPtr       - pointer to internal TTI info
*/
static GT_VOID snetChtTTermination
(
    IN    SKERNEL_DEVICE_OBJECT             * devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC   * descrPtr,
    IN    INTERNAL_TTI_DESC_INFO_STC        * internalTtiInfoPtr
)
{
    DECLARE_FUNC_NAME(snetChtTTermination);

    snetCht3TTermination(devObjPtr, descrPtr,internalTtiInfoPtr);

    if(descrPtr->packetCmd == SKERNEL_EXT_PKT_CMD_HARD_DROP_E ||
       descrPtr->packetCmd >= SKERNEL_EXT_PKT_CMD_TO_TRG_SNIFFER_E)
    {
        __LOG(("At the end of TTI unit do next implicit Set of bypassBridge = 1 for HARD_DROP / not forward DSA tag \n"));
        descrPtr->bypassBridge = 1;
        __LOG_PARAM(descrPtr->bypassBridge);
    }
}

/**
* @internal pclAfterTtiPclLookupModeGet function
* @endinternal
*
* @brief   get pcl lookup mode after TTI
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] descrPtr             - pointer to frame descriptor.
*
*/
static GT_VOID pclAfterTtiPclLookupModeGet
(
    IN    SKERNEL_DEVICE_OBJECT             *devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC   *descrPtr
)
{
    DECLARE_FUNC_NAME(snetXcatPclAfterTtiPclLookupModeGet);

    GT_U32 * portVlanCfgEntryPtr;   /* table entry pointer */
    GT_U32  regAddress;
    GT_U32   fieldVal;   /* Register's field value */
    GT_U32   bitNum;     /* Register's bit number  */
    GT_BIT  modifyArr[4] = {0,0,0,0};

    if(devObjPtr->supportEArch && devObjPtr->unitEArchEnable.tti)
    {
        portVlanCfgEntryPtr = NULL;
    }
    else
    {
        regAddress = SMEM_CHT_PORT_VLAN_QOS_CONFIG_TBL_MEM(devObjPtr,
                                                            descrPtr->localDevSrcPort);
        portVlanCfgEntryPtr = smemMemGet(devObjPtr, regAddress);
    }

    modifyArr[0] = modifyArr[1] = modifyArr[2] = modifyArr[3]= 0;

    if(descrPtr->pclLookUpMode[0] == SKERNEL_PCL_LOOKUP_MODE_DESCRIPTOR_INDEX_E)
    {
        __LOG_PARAM(descrPtr->pclLookUpMode[0]);
    }

    if(descrPtr->pclLookUpMode[1] == SKERNEL_PCL_LOOKUP_MODE_DESCRIPTOR_INDEX_E)
    {
        __LOG_PARAM(descrPtr->pclLookUpMode[1]);
    }

    if(descrPtr->pclLookUpMode[2] == SKERNEL_PCL_LOOKUP_MODE_DESCRIPTOR_INDEX_E)
    {
        __LOG_PARAM(descrPtr->pclLookUpMode[2]);
    }

    if(descrPtr->mpclLookUpMode == SKERNEL_PCL_LOOKUP_MODE_DESCRIPTOR_INDEX_E)
    {
        __LOG_PARAM(descrPtr->mpclLookUpMode);
    }

    if(descrPtr->eArchExtInfo.ttiPostTtiLookupIngressEPortTablePtr)
    {
        /*IPCL enable*/
        descrPtr->policyOnPortEn =
            SMEM_LION3_TTI_EPORT_ATTRIBUTES_ENTRY_FIELD_GET(devObjPtr,descrPtr,
                SMEM_LION3_TTI_EPORT_ATTRIBUTES_TABLE_FIELDS_ING_POLICY_EN);

        /*PCL0-0 Lookup Configuration Mode*/
        if(descrPtr->pclLookUpMode[0] != SKERNEL_PCL_LOOKUP_MODE_DESCRIPTOR_INDEX_E)
        {
            modifyArr[0] = 1;
            descrPtr->pclLookUpMode[0] =
                SMEM_LION3_TTI_DEFAULT_EPORT_ENTRY_FIELD_GET(devObjPtr, descrPtr,
                    SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_LOOKUP0_PCL_CFG_MODE);
        }

        /*PCL0-1 Lookup Configuration Mode*/
        if(descrPtr->pclLookUpMode[1] != SKERNEL_PCL_LOOKUP_MODE_DESCRIPTOR_INDEX_E)
        {
            modifyArr[1] = 1;
            descrPtr->pclLookUpMode[1] =
                SMEM_LION3_TTI_DEFAULT_EPORT_ENTRY_FIELD_GET(devObjPtr, descrPtr,
                    SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_LOOKUP1_PCL_CFG_MODE);
        }

        /*PCL1-0 Lookup Configuration Mode*/
        if(descrPtr->pclLookUpMode[2] != SKERNEL_PCL_LOOKUP_MODE_DESCRIPTOR_INDEX_E)
        {
            modifyArr[2] = 1;
            descrPtr->pclLookUpMode[2] =
                SMEM_LION3_TTI_DEFAULT_EPORT_ENTRY_FIELD_GET(devObjPtr, descrPtr,
                    SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_LOOKUP2_PCL_CFG_MODE);
        }
        /*MPCLLookup Configuration Mode*/
        if(SMEM_CHT_IS_SIP7_GET(devObjPtr) &&
           descrPtr->mpclLookUpMode != SKERNEL_PCL_LOOKUP_MODE_DESCRIPTOR_INDEX_E)
        {
            modifyArr[3] = 1;
            descrPtr->mpclLookUpMode =
                SMEM_LION3_TTI_DEFAULT_EPORT_ENTRY_FIELD_GET(devObjPtr, descrPtr,
                    SMEM_SIP7_TTI_DEFAULT_E_PORT_TABLE_FIELDS_MIDWAY_PCL_CFG_MODE_E);
        }

    }
    else if(portVlanCfgEntryPtr == NULL)
    {
        /* fix for coverity warning "FORWARD_NULL" */
        skernelFatalError("pclAfterTtiPclLookupModeGet: invalid table entry pointer\n");
    }
    else
    {
        /* Ingress Policy Enable */
        descrPtr->policyOnPortEn =
            SNET_CHT_PORT_VLAN_QOS_PORT_POLICY_EN_GET_MAC(devObjPtr,
                                                          portVlanCfgEntryPtr);

        /*PCL0-0 Lookup Configuration Mode*/
        if(descrPtr->pclLookUpMode[0] != SKERNEL_PCL_LOOKUP_MODE_DESCRIPTOR_INDEX_E)
        {
            modifyArr[0] = 1;
            descrPtr->pclLookUpMode[0] =
                SNET_CHT_PORT_VLAN_QOS_PCL0_0_LOOKUP_CONFIGURATION_MODE_GET_MAC(devObjPtr,portVlanCfgEntryPtr);
        }

        /*PCL0-1 Lookup Configuration Mode*/
        if(descrPtr->pclLookUpMode[1] != SKERNEL_PCL_LOOKUP_MODE_DESCRIPTOR_INDEX_E)
        {
            modifyArr[1] = 1;
            descrPtr->pclLookUpMode[1] =
                SNET_CHT_PORT_VLAN_QOS_PCL0_1_LOOKUP_CONFIGURATION_MODE_GET_MAC(devObjPtr,portVlanCfgEntryPtr);
        }

        /*PCL1-0 Lookup Configuration Mode*/
        if(descrPtr->pclLookUpMode[2] != SKERNEL_PCL_LOOKUP_MODE_DESCRIPTOR_INDEX_E)
        {
            modifyArr[2] = 1;
            descrPtr->pclLookUpMode[2] =
                SNET_CHT_PORT_VLAN_QOS_PCL1_0_LOOKUP_CONFIGURATION_MODE_GET_MAC(devObjPtr,portVlanCfgEntryPtr);
        }
    }

    if(modifyArr[0])
    {
        __LOG_PARAM(descrPtr->pclLookUpMode[0]);
    }

    if(modifyArr[1])
    {
        __LOG_PARAM(descrPtr->pclLookUpMode[1]);
    }

    if(modifyArr[2])
    {
        __LOG_PARAM(descrPtr->pclLookUpMode[2]);
    }

    modifyArr[0] = modifyArr[1] = modifyArr[2] = 0;

    __LOG(("Ingress Policy Enable: [%d]", descrPtr->policyOnPortEn));

    if(descrPtr->tunnelTerminated)
    {
        /* get Force_VLAN_Mode_on_TTI field value */
        if (SMEM_CHT_IS_SIP7_GET(devObjPtr))
        {
            bitNum = 24;
            smemRegFldGet(devObjPtr, SMEM_SIP7_TTI_LU_UNIT_GENERAL_GLOBAL_CONFIG_REG(devObjPtr), bitNum, 1, &fieldVal);
        }
        else
        {
            bitNum = SMEM_CHT_IS_SIP5_GET(devObjPtr)? 5 : 10;
            smemRegFldGet(devObjPtr, SMEM_XCAT_TTI_ENGINE_CONFIG_REG(devObjPtr), bitNum, 1, &fieldVal);
        }

        if(fieldVal)
        {
            /*If TTI rule hit Policy Configuration table, access is according to the packet's VLAN-ID.*/
            __LOG(("enabled Force_VLAN_Mode_on_TTI \n"));
            descrPtr->pclLookUpMode[0] =
            descrPtr->pclLookUpMode[1] =
            descrPtr->pclLookUpMode[2] = SKERNEL_PCL_LOOKUP_MODE_VID_E;

            modifyArr[0] = modifyArr[1] = modifyArr[2] = 1;
        }
    }

    if(modifyArr[0])
    {
        __LOG_PARAM(descrPtr->pclLookUpMode[0]);
    }

    if(modifyArr[1])
    {
        __LOG_PARAM(descrPtr->pclLookUpMode[1]);
    }

    if(modifyArr[2])
    {
        __LOG_PARAM(descrPtr->pclLookUpMode[2]);
    }

}

/**
* @internal snetChtAfterIPclGlue function
* @endinternal
*
* @brief   glue function after IPCL unit (and before the bridge) that used for LSR updates
*
* @param[in] devObjPtr                - pointer to device object
* @param[in] descrPtr                 - pointer to frame descriptor
*                                      OUTPUT:
*                                      RETURN:
*/
static GT_VOID snetChtAfterIPclGlue(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr
)
{
    DECLARE_FUNC_NAME(snetChtAfterIPclGlue);

    GT_BIT isSwap;/* is mpls command involve swap (swap/pop1_swap/pop2_swap)*/
    GT_U32 offset = 0;/* offset of sections in the packet (from start of packet)*/

    if(descrPtr->tunnelTerminated == GT_FALSE)
    {
        if(descrPtr->mpls && descrPtr->mplsCommand != SKERNEL_XCAT_TTI_MPLS_NOP_E)
        {
            if(descrPtr->VntL2Echo == GT_TRUE)
            {
                __LOG(("Do not treat LSR for VntL2Echo = 1 \n"));
            }
            else
            {
                __LOG(("For MPLS LSR set 'implicitly' the descrPtr->tunnelTerminated = GT_TRUE , for NON-tunnel terminated action ! \n"));
                descrPtr->tunnelTerminated = GT_TRUE;/* the HW set this bit !!! */
                descrPtr->isMplsLsr = 1;

                switch(descrPtr->mplsCommand)
                {
                case SKERNEL_XCAT_TTI_MPLS_SWAP_E:
                case SKERNEL_XCAT_TTI_MPLS_POP_SWAP_E:
                /* those cases are not treated here as swap (like in verifier)
                case SKERNEL_XCAT_TTI_MPLS_POP2_SWAP_E:
                case SKERNEL_XCAT_TTI_MPLS_POP3_SWAP_E:*/
                    isSwap = 1;
                    break;
                default:
                    isSwap = 0;
                    break;
                }
                /* an LSR is an intermediate MPLS router in the network */
                descrPtr->doRouterHa = isSwap ? 0 : 1;/*logic from Verifier*/
                descrPtr->routed = 1;
                if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
                {
                    /*
                        IF TTI Action<MPLS Command>==SWAP && Number_Of_Labels == 1
                            Desc<Inner Packet Type> = UNKNOWN (4)
                        Else if TTI Action<MPLS Command>==POP-SWAP && Number_Of_Labels == 2
                            Desc<Inner Packet Type> = UNKNOWN (4)
                        Else
                            Desc<Inner Packet Type> = MPLS (1)
                    */

                    if(descrPtr->mplsCommand == SKERNEL_XCAT_TTI_MPLS_SWAP_E && ((descrPtr->numOfLabels + 1) == 1))
                    {
                        descrPtr->innerPacketType = SKERNEL_INNER_PACKET_TYPE_UNKNOWN;
                    }
                    else
                    if(descrPtr->mplsCommand == SKERNEL_XCAT_TTI_MPLS_POP_SWAP_E/*POP-SWAP*/ && ((descrPtr->numOfLabels + 1) == 2))
                    {
                        descrPtr->innerPacketType = SKERNEL_INNER_PACKET_TYPE_UNKNOWN;
                    }
                    else
                    if(descrPtr->mplsCommand == SKERNEL_XCAT_TTI_MPLS_POP2_SWAP_E/*POP2-SWAP*/ && ((descrPtr->numOfLabels + 1) == 3))
                    {
                        descrPtr->innerPacketType = SKERNEL_INNER_PACKET_TYPE_UNKNOWN;
                    }
                    else
                    if(descrPtr->mplsCommand == SKERNEL_XCAT_TTI_MPLS_POP3_SWAP_E/*POP3-SWAP*/ && ((descrPtr->numOfLabels + 1) == 4))
                    {
                        descrPtr->innerPacketType = SKERNEL_INNER_PACKET_TYPE_UNKNOWN;
                    }
                    else
                    {
                        descrPtr->innerPacketType = SKERNEL_INNER_PACKET_TYPE_MPLS;
                    }
                }
                else
                {
                    /* the descrPtr->innerPacketType was already set according
                       to the passengerPacketType from the action */
                }

                /* LSR label operations */
                __LOG(("MPLS LSR treatment \n"));

                switch (descrPtr->mplsCommand)
                {
                    case SKERNEL_XCAT_TTI_MPLS_SWAP_E: /* Swap operation requires the top label in the stack to be replaced by a different label */
                        offset = 4;
                        /* the new label and will be added later */
                        break;
                    case SKERNEL_XCAT_TTI_MPLS_PUSH_E: /* Push operation inserts one, two, or three new labels on top of an existing packet label stack */
                        /* Do nothing */
                        /* the new labels and will be added later */
                        break;
                    case SKERNEL_XCAT_TTI_MPLS_POP_1_E: /* Pop one label operation pops one MPLS label from top of the incoming label stack */
                        offset = 4;
                        break;
                    case SKERNEL_XCAT_TTI_MPLS_POP_2_E: /* Pop two labels operation pops two MPLS labels from top of the incoming label stack */
                        offset = (descrPtr->numOfLabels > 1) ? 8 : 4;
                        break;
                    case SKERNEL_XCAT_TTI_MPLS_POP_SWAP_E: /* Pop_and_Swap operation pop one MPLS label from top of the incoming label stack
                        then requires the top label in the stack to be replaced by a different label*/
                        offset = descrPtr->numOfLabels ? 8 : 4;
                        /* the new label and will be added later */
                        break;

                    case SKERNEL_XCAT_TTI_MPLS_POP2_SWAP_E:  /*new in sip5 */
                    case SKERNEL_XCAT_TTI_MPLS_POP_3_E:      /*new in sip5 */
                        offset = (descrPtr->numOfLabels+1) >= 3 ?
                                12 : /*jump 3 labels*/
                                (descrPtr->numOfLabels + 1) * 4 ;/* do not jump more than existing labels */
                        break;
                    case SKERNEL_XCAT_TTI_MPLS_POP3_SWAP_E:  /*new in sip5 */
                        offset = (descrPtr->numOfLabels+1) >= 4 ?
                                16 : /*jump 4 labels*/
                                (descrPtr->numOfLabels + 1) * 4 ;/* do not jump more than existing labels */
                        break;

                    default:
                        skernelFatalError("descrPtr->mplsCommand: not valid mode[%d]",descrPtr->mplsCommand);
                        break;
                }

                /* the LSR operation is without explicit 'tunnel termination' but the
                   logic is as if we have <tunnel Termination Offset>

                   so we need to add the L2 header + the MPLS offset
                */
                descrPtr->tunnelTerminationOffset = offset + (descrPtr->l3StartOffsetPtr - descrPtr->startFramePtr);
                /* save the offset from the start of original packet to the new place of the 'l3' */
                __LOG_PARAM(descrPtr->tunnelTerminationOffset);
            }
        }
    }

}

/**
* @internal sip7LsrGlue function
* @endinternal
*
* @brief   SIP7 glue function after IPCL unit (and before the bridge) that used for LSR updates
*
* @param[in] devObjPtr                - pointer to device object
* @param[in] descrPtr                 - pointer to frame descriptor
*                                      OUTPUT:
*                                      RETURN:
*/
static GT_VOID sip7LsrGlue(
    IN SKERNEL_DEVICE_OBJECT           *devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC *descrPtr
)
{
    DECLARE_FUNC_NAME(sip7LsrGlue);
    GT_U32 offset;/* offset of sections in the packet (from start of packet)*/

    if(descrPtr->tunnelTerminated == GT_FALSE)
    {
        if(descrPtr->mpls &&
           (descrPtr->totalPoppedLabels > 0 || descrPtr->isLabelSwapped || descrPtr->isLabelPushed))
        {
            if(descrPtr->VntL2Echo == GT_TRUE)
            {
                __LOG(("Do not treat LSR for VntL2Echo = 1 \n"));
            }
            else
            {
                __LOG(("For MPLS LSR set 'implicitly' the descrPtr->tunnelTerminated = GT_TRUE , for NON-tunnel terminated action ! \n"));
                descrPtr->tunnelTerminated = GT_TRUE; /* Set the Tunnel termination */
                descrPtr->isMplsLsr = 1; /* Trigger MPLS LST Glue */

                /* an LSR is an intermediate MPLS router in the network */
                descrPtr->doRouterHa = descrPtr->isLabelSwapped ? 0 : 1;/*logic from Verifier*/
                descrPtr->routed = 1;
                if (descrPtr->isLabelSwapped &&
                    descrPtr->fwdLabelsPopSwap == (descrPtr->numOfLabels + 1) &&
                    descrPtr->isLastLabelInStack[descrPtr->currFwdLabelIndex - 1])
                {
                    descrPtr->innerPacketType = SKERNEL_INNER_PACKET_TYPE_UNKNOWN;
                }
                else
                {
                    descrPtr->innerPacketType = SKERNEL_INNER_PACKET_TYPE_MPLS;
                }
                __LOG(("Inner Packet Type [%u]\n", descrPtr->innerPacketType));
                offset = (descrPtr->totalPoppedLabels)*4;
                offset += (descrPtr->isLabelSwapped)?4:0;
                __LOG(("MPLS Label offset is [%u], totalPoppedLabels [%u], isLabelSwapped[%u]\n",
                       offset, descrPtr->totalPoppedLabels, descrPtr->isLabelSwapped));

                descrPtr->tunnelTerminationOffset = offset + (descrPtr->l3StartOffsetPtr - descrPtr->startFramePtr);
                /* save the offset from the start of original packet to the new place of the 'l3' */
                __LOG_PARAM(descrPtr->tunnelTerminationOffset);
            }
        }
        else
        {
            __LOG(("Total Number of Popped labels is 0 or non MPLS packet. MPLS LSR not enabled\n"));
        }
    }
    else
    {
        __LOG(("Tunnel Terminate is FALSE. MPLS LSR not enabled\n"));
    }
}

/**
* @internal snetChtPpu function
* @endinternal
*
* @param[in] devObjPtr                - pointer to device object
* @param[in] descrPtr                 - pointer to frame descriptor
*                                      OUTPUT:
*                                      RETURN:
*
* @note Call to sip_6_10 PPU.
*
*/
static GT_VOID snetChtPpu(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr
)
{
    simLogPacketFrameUnitSet(SIM_LOG_FRAME_UNIT_PPU_E);

    snetHawkPpu(devObjPtr, descrPtr);
}

/**
* @internal snetChtIPcl function
* @endinternal
*
* @param[in] devObjPtr                - pointer to device object
* @param[in] descrPtr                 - pointer to frame descriptor
*                                      OUTPUT:
*                                      RETURN:
*
* @note Call to cheetah2 or cheetah ingress PCL.
*
*/
static GT_VOID snetChtIPcl(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr
)
{
    DECLARE_FUNC_NAME(snetChtIPcl);

    simLogPacketFrameUnitSet(SIM_LOG_FRAME_UNIT_IPCL_E);

    if(descrPtr->packetCmd >= SKERNEL_EXT_PKT_CMD_TO_TRG_SNIFFER_E)
    {
        /* the commands of :
            SKERNEL_EXT_PKT_CMD_TO_TRG_SNIFFER_E    --> only from DSA tag --> not doing IPCL
            SKERNEL_EXT_PKT_CMD_FROM_CSCD_TO_CPU_E  --> only from DSA tag --> not doing IPCL
            SKERNEL_EXT_PKT_CMD_FROM_CPU_E          --> only from DSA tag --> not doing IPCL
            SKERNEL_EXT_PKT_CMD_BRIDGE_MIRROR_E,    --> only inside the router engine --> never get here
            SKERNEL_EXT_PKT_CMD_BRIDGE_E,           --> only inside the router engine --> never get here
        */
        return;
    }

    /* function should be in TTI unit , but was easier to fix code in PCL unit
       then to move to TTI */
    __LOG(("Do pcl lookup mode resolution from TTI \n"));
    pclAfterTtiPclLookupModeGet(devObjPtr,descrPtr);

    snetXCatIPcl(devObjPtr, descrPtr);

    /* L2/L3 Ingress VLAN CNC Trigger */
    __LOG(("CNC - L2/L3 Ingress VLAN CNC Trigger \n"));
    snetCht3CncCount(devObjPtr, descrPtr,
                     SNET_CNC_CLIENT_INGRESS_VLAN_L2_L3_E, 0);


    if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        __LOG(("CNC - 'source eport' : use index from localDevSrcEPort \n"));
        /*  CNC counter index is taken from LocalDevSRCePort[13:0] after the
            optional assignment of source ePort by PCL/TTI action */
        snetCht3CncCount(devObjPtr, descrPtr,
                         SNET_CNC_CLIENT_SOURCE_EPORT_E,
                         descrPtr->eArchExtInfo.localDevSrcEPort);
    }

    /* call the glue that sets in the end of the IPCL (before) L2i */
    if(SMEM_CHT_IS_SIP7_GET(devObjPtr))
    {
        sip7LsrGlue(devObjPtr,descrPtr);
    }
    else
    {
        snetChtAfterIPclGlue(devObjPtr,descrPtr);
    }

    if(devObjPtr->pclSupport.pclUseCFMEtherType)
    {
        if(descrPtr->VntL2Echo)
        {
            __LOG(("Do CFM classification from PCL unit \n"));
            snetChtCfmEtherTypeCheck(devObjPtr, descrPtr);
        }
        else
        {
            __LOG(("NO CFM classification when VntL2Echo = 0 \n"));
        }
    }


    if(descrPtr->packetCmd == SKERNEL_EXT_PKT_CMD_HARD_DROP_E/* ||
       descrPtr->packetCmd >= SKERNEL_EXT_PKT_CMD_TO_TRG_SNIFFER_E*/)
    {
        __LOG(("At the end of PCL unit do next implicit Set of bypassBridge = 1 for HARD_DROP \n"));
        descrPtr->bypassBridge = 1;
        __LOG_PARAM(descrPtr->bypassBridge);
    }
}

/**
* @internal snetChtIPolicer function
* @endinternal
*
* @brief   Policer Processing --- Policer Counters updates
*/
static GT_VOID snetChtIPolicer(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr
)
{
    simLogPacketFrameUnitSet(SIM_LOG_FRAME_UNIT_IPLR_E);

    /* ch3 and above */
    snetCht3Policer(devObjPtr, descrPtr);

    /* reset the field as it hold no meaning outside the POLICER and it makes 'diff' for the LOGGER */
    descrPtr->policerCycle = 0;
}

/**
* @internal snetChtRouting function
* @endinternal
*
* @param[in] devObjPtr                - pointer to device object
* @param[in] descrPtr                 - pointer to frame descriptor
*                                      OUTPUT:
*                                      RETURN:
*
* @note Call to cheetah2 or cheetah ingress ROUTING.
*
*/
static GT_VOID snetChtRouting(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr
)
{
    DECLARE_FUNC_NAME(snetChtRouting);

    GT_U32  regAddr;/* register address */
    GT_U32  regVal;/* register value */
    GT_BIT  useCheetahUcRouter = 0;/* do we use cheetah 1 UC router */

    simLogPacketFrameUnitSet(SIM_LOG_FRAME_UNIT_IPVX_E);

    if (SMEM_CHT_IS_SIP5_GET(devObjPtr) == 0)
    {
        if (devObjPtr->supportOnlyUcRouter) /* device with only 'router glue' */
        {
            /* cheetah 1, BullsEye */
            __LOG(("cheetah 1, BullsEye"));
            useCheetahUcRouter = 1;
        }
        else
        {
            /* devices with router engine */
            __LOG(("devices with router engine"));
            /* check if the router glue enabled */
            regAddr = SMEM_CHT2_ROUTER_ADDITIONAL_CONTROL_REG(devObjPtr);
            smemRegFldGet(devObjPtr,regAddr,0,1,&regVal);
            if(regVal == 1)
            {
                /* router mode */
                __LOG(("router mode"));
                if(descrPtr->pclRedirectCmd == PCL_TTI_ACTION_REDIRECT_CMD_PBR_OR_VPLS_E)
                {
                    /* don't do routing */
                    __LOG(("don't do routing"));
                    return;
                }
            }
            else
            {
                /* router glue mode */
                __LOG(("router glue mode"));
                useCheetahUcRouter = 1;
            }
        }
    }

    if (useCheetahUcRouter == 0)     /* : cheetah2,3 ROUTER */
    {
        snetCht2Routing(devObjPtr, descrPtr);
    }
    else
    {
        snetChtUcRouting(devObjPtr,descrPtr);     /* : cheetah ROUTER */

        if(devObjPtr->errata.routerGlueResetPartsOfDescriptor)
        {
            /* IN HW : bits [214:249] in the descriptor that is going out to the plr are zeroed*/

            /* the router glue reset some of the fields of the descriptor */
            descrPtr->cfidei = 0;               /* cfi */
            descrPtr->pktIsLooped = 0;          /*PktIsLooped*/
            /* DropOnSource --> not implemented */
            /* OrigVID --> not implemented */
            descrPtr->cfm = 0;                 /*CFMPkt*/
            descrPtr->policerEn = 0;           /*PolicerEn*/
            descrPtr->policerPtr = 0;          /*PolicerPtr*/
            descrPtr->policerCounterEn = 0;    /*BillingEn*/
        }
    }
}

/**
* @internal snetChtIngressL3IpReplication function
* @endinternal
*
* @brief   Ingress L2 tunnel replication
*
* @param[in] devObjPtr                - pointer to device object
* @param[in,out] descrPtr             - pointer to frame descriptor
*/
static GT_VOID snetChtIngressL3IpReplication
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC   * descrPtr
)
{
    DECLARE_FUNC_NAME(snetChtIngressL3IpReplication);
    GT_U32 regAddr;
    GT_U32 fieldVal;

    if(descrPtr->validMll == 0)
    {
        /* not valid MLL to act according */
        __LOG(("not valid MLL to act according"));
        return;
    }

    if(SMEM_CHT_IS_SIP5_20_GET(devObjPtr))
    {
        regAddr = SMEM_CHT2_MLL_GLB_CONTROL_REG(devObjPtr);
        smemRegFldGet(devObjPtr, regAddr, 20, 1, &fieldVal);

        if(fieldVal)
        {
            __LOG(("MLL L3 replication disabled"));
            return;
        }
    }
    snetCht3IngressL3IpReplication(devObjPtr,descrPtr);
}

/**
* @internal snetSip5MllMultiTargetEPorts function
* @endinternal
*
* @brief   SIP5 : Multi Target ePorts support
*
* @param[in] devObjPtr                  - pointer to device object
* @param[in,out] descrPtr               - pointer to frame descriptor
*/
static GT_VOID snetSip5MllMultiTargetEPorts
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC   * descrPtr
)
{
    DECLARE_FUNC_NAME(snetSip5MllMultiTargetEPorts);

    GT_U32  regValue;

    GT_U32  multiTargetePortBase;
    GT_U32  multiTargetePortValue,multiTargetePortMask;/* sip5 :value,mask for multi-target global eports */
    GT_U32  globalEPortMinValue,globalEPortMaxValue;   /* sip6 :min,max for multi-target global eports */
    GT_U32  ePortToEVidxBase;
    GT_32   eVidx;/* !!! signed integer !!!*/

    if(SMEM_CHT_IS_SIP6_GET(devObjPtr))
    {
        smemRegGet(devObjPtr, SMEM_SIP6_MLL_MULTI_TARGET_GLOBAL_E_PORT_MIN_VALUE_REG(devObjPtr), &regValue);
        globalEPortMinValue = SMEM_U32_GET_FIELD(regValue,0,17);
        smemRegGet(devObjPtr, SMEM_SIP6_MLL_MULTI_TARGET_GLOBAL_E_PORT_MAX_VALUE_REG(devObjPtr) , &regValue);
        globalEPortMaxValue = SMEM_U32_GET_FIELD(regValue,0,17);

        __LOG(("The range of Multi-Target global eports[%d..%d] \n",
            globalEPortMinValue,
            globalEPortMaxValue));

        /*(E >= <Global ePort Min Value> && E <= <Global ePort Max Value>)*/
        if(descrPtr->trgEPort < globalEPortMinValue)
        {
            __LOG(("The eport[%d] is below min range of Multi-Target global eports[%d] \n",
                descrPtr->trgEPort,
                globalEPortMinValue));
            __LOG(("the target eport is not considered as Multi-Target global eport \n"));
            return;
        }

        if(descrPtr->trgEPort > globalEPortMaxValue)
        {
            __LOG(("The eport[%d] is above max range of Multi-Target global eports[%d] \n",
                descrPtr->trgEPort,
                globalEPortMaxValue));
            __LOG(("the target eport is not considered as Multi-Target global eport \n"));
            return;
        }

        __LOG(("The eport[%d] is in range of Multi-Target global eports[%d..%d] \n",
            descrPtr->trgEPort,
            globalEPortMinValue,
            globalEPortMaxValue));
    }
    else
    {
        smemRegGet(devObjPtr, SMEM_LION3_MLL_MULTI_TARGET_EPORT_VALUE_REG(devObjPtr) ,&regValue);
        multiTargetePortValue = SMEM_U32_GET_FIELD(regValue,0,17);

        smemRegGet(devObjPtr, SMEM_LION3_MLL_MULTI_TARGET_EPORT_MASK_REG(devObjPtr) ,&regValue);
        multiTargetePortMask = SMEM_U32_GET_FIELD(regValue,0,17);

        if((descrPtr->trgEPort & multiTargetePortMask) != multiTargetePortValue)
        {
            __LOG(("Multi-Target ePort : the target ePort[0x%x] is not in range for mapping. \n",
                descrPtr->trgEPort));
            return;
        }
    }

    smemRegGet(devObjPtr, SMEM_LION3_MLL_MULTI_TARGET_EPORT_BASE_REG(devObjPtr) ,&regValue);
    multiTargetePortBase = SMEM_U32_GET_FIELD(regValue,0,17);

    smemRegGet(devObjPtr, SMEM_LION3_MLL_MULTI_TARGET_EPORT_TO_EVIDX_BASE_REG(devObjPtr) ,&regValue);
    ePortToEVidxBase = SMEM_U32_GET_FIELD(regValue,0,16);

    /*
        Desc<UseVIDX>=1
        Desc<eVIDX>=Desc<TRGePort>-<Multi-Target ePort Base>+<ePort to eVIDX Base>

        NOTE: If the result of the eVIDX in the last line is < 0, the MLL assigns eVIDX=0.
        If the result is > 0xFFFF, the MLL assigns eVIDX=0xFFFF.
    */

    eVidx = descrPtr->trgEPort - multiTargetePortBase +  ePortToEVidxBase;
    if(eVidx < 0)
    {
        eVidx = 0;
    }
    else if(eVidx > 0xFFFF)
    {
        eVidx = 0xFFFF;
    }

    descrPtr->useVidx = 1;
    descrPtr->eVidx = (GT_U32)eVidx;

    __LOG(("Multi-Target ePort : the target ePort[0x%x] mapped to: useVidx = 1 and eVidx[0x%x] ! \n",
        descrPtr->trgEPort,
        descrPtr->eVidx));

    /* 'destroy' the target ePort .
        it is not needed but just to change it from previous value */
    descrPtr->trgEPort = 0;
}

/**
* @internal snetSip7MllVPortsRangeClassification function
* @endinternal
*
* @brief   SIP7 : Multi Target vPorts support
*
* @param[in] devObjPtr                  - pointer to device object
* @param[in,out] descrPtr               - pointer to frame descriptor
* @param[out] cfgBackupValuePtr         - array of cfg Backup Value (4 words)
* @param[out] cfgBackupMaskPtr          - array of cfg Backup Mask  (4 words)
*/
static GT_VOID snetSip7MllVPortsRangeClassification
(
    IN SKERNEL_DEVICE_OBJECT                * devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC   * descrPtr,
    OUT GT_U32                              cfgBackupValueArr[4],
    OUT GT_U32                              cfgBackupMaskArr[4]
)
{
    DECLARE_FUNC_NAME(snetSip7MllVPortsRangeClassification);
    GT_U32  regValue;
    GT_U32 *cfgBackupValuePtr;
    GT_U32 *cfgBackupMaskPtr;
    GT_U32 vPortBase;
    GT_U32 rangeSize;
    GT_32  diff; /* !!! signed integer !!!*/
    GT_U32  ii;
    GT_U32 lastMatchedIndex = SMAIN_NOT_VALID_CNS;

    __LOG(("Start L2MLL : vPort Range Classification \n"));
    /*  vPort Range Classification
        d= <Trg vPort>-<Multi-Target vPort range base>
        <CFG mask>= 0

        For x=0 to 3
            * If d >= 0 and d < <Multi-Target vPort Range x size> then
            *       <CFG Backup value> = <Multi-Target vPort Range x Backup CFG value>
            *       <CFG mask> = <Multi-Target vPort Range x CFG mask>
            * d= d - <Multi-Target vPort Range x size>
    */
    smemRegGet(devObjPtr, SMEM_SIP7_MLL_MULTI_TARGET_VPORT_RANGE_BASE_REG(devObjPtr), &regValue);
    vPortBase = SMEM_U32_GET_FIELD(regValue,0,18);

    diff = descrPtr->trgVPort - vPortBase;
    __LOG_PARAM(diff);

    for(ii = 0 ; ii < 4 ; ii++)
    {
        smemRegGet(devObjPtr, SMEM_SIP7_MLL_MULTI_TARGET_VPORT_SUB_RANGE_SIZE_REG(devObjPtr,ii), &regValue);
        rangeSize = SMEM_U32_GET_FIELD(regValue,0,18);
        __LOG_PARAM(rangeSize);

        if(diff >= 0 && diff < (GT_32)rangeSize)
        {
            lastMatchedIndex = ii;
            __LOG_PARAM(lastMatchedIndex);
        }

        diff -= rangeSize;
        __LOG_PARAM(diff);
    }

    if(lastMatchedIndex != SMAIN_NOT_VALID_CNS)
    {
        __LOG(("vPort Range Classification : Range [%d] was matched \n",
            lastMatchedIndex));

        ii = lastMatchedIndex;
        cfgBackupValuePtr = smemMemGet(devObjPtr, SMEM_SIP7_MLL_MULTI_TARGET_VPORT_SUB_RANGE_BACKUP_CFG_VALUE_REG(devObjPtr,ii,0));
        cfgBackupMaskPtr  = smemMemGet(devObjPtr, SMEM_SIP7_MLL_MULTI_TARGET_VPORT_SUB_RANGE_BACKUP_CFG_MASK_REG (devObjPtr,ii,0));

        for(ii = 0 ; ii < 4; ii++)/* loop on 4 words in cfgBackupValueArr[]*/
        {
            cfgBackupValueArr[ii] = cfgBackupValuePtr[ii];
            cfgBackupMaskArr [ii] = cfgBackupMaskPtr [ii];
        }
    }
    else
    {
        __LOG(("vPort Range Classification : No Range was matched \n"));

        for(ii = 0 ; ii < 4; ii++)/* loop on 4 words in cfgBackupValueArr[]*/
        {
            cfgBackupValueArr[ii] = 0;
            cfgBackupMaskArr [ii] = 0;
        }
    }
}

/**
* @internal snetSip7MllMultiTargetVPorts function
* @endinternal
*
* @brief   SIP7 : Multi Target vPorts support
*
* @param[in] devObjPtr                  - pointer to device object
* @param[in,out] descrPtr               - pointer to frame descriptor
*/
static GT_VOID snetSip7MllMultiTargetVPorts
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC   * descrPtr
)
{
    DECLARE_FUNC_NAME(snetSip7MllMultiTargetVPorts);

    GT_U32  regValue;
    GT_U32  vPortBase,vPortSize,vPortLast;
    GT_U32  multi_target_vport_evidx;

    smemRegGet(devObjPtr, SMEM_SIP7_MLL_MULTI_TARGET_VPORT_RANGE_BASE_REG(devObjPtr), &regValue);
    vPortBase = SMEM_U32_GET_FIELD(regValue,0,18);
    smemRegGet(devObjPtr, SMEM_SIP6_MLL_MULTI_TARGET_GLOBAL_E_PORT_MAX_VALUE_REG(devObjPtr) , &regValue);
    vPortSize = SMEM_U32_GET_FIELD(regValue,0,18);

    if(vPortSize == 0)
    {
        /* can't match any vPort , the same as 'globally disabled' */
        __LOG(("WARNING : potential configuration error : Multi-Target vPort : <Multi-Target vPort Range Size> == 0 , will not match any vPort (the same as 'globally disabled') \n"));
        return;
    }

    vPortLast = vPortBase + vPortSize - 1;

    if(descrPtr->trgVPort < vPortBase)
    {
        __LOG(("The vPort[%d] is below min range of Multi-Target vPorts[%d..%d] (not considered as Multi-Target vPort)\n",
            descrPtr->trgVPort,
            vPortBase,vPortLast));
        return;
    }

    if(descrPtr->trgVPort > vPortLast)
    {
        __LOG(("The vPort[%d] is above max range of Multi-Target vPorts[%d..%d] (not considered as Multi-Target vPort)\n",
            descrPtr->trgVPort,
            vPortBase,vPortLast));
        return;
    }

    __LOG(("The vPort[%d] is in range of Multi-Target vPorts[%d..%d] \n",
        descrPtr->trgVPort,
        vPortBase,vPortLast));

    /*
    * 2. temp= inDesc<Trg vPort>
    * 3. Trigger the L2MLL by setting:
    * 4. Desc<Use VIDX>=1
    * 5. eVIDX= Global<Multi-Target vPort eVIDX>
    * 6. For all replicas overwrite L2MLL<Trg vPort 0(1)> such that L2MLL<Trg vPort 0(1)>= temp
    */

    smemRegGet(devObjPtr, SMEM_SIP7_MLL_MULTI_TARGET_VPORT_RANGE_EVIDX_REG(devObjPtr) ,&regValue);
    multi_target_vport_evidx = SMEM_U32_GET_FIELD(regValue,0,16);

    __LOG(("Send the packet to eVidx[%d] , that expected to have 2 nodes each"
           "will send each replica to trgVPort [%d] (the 'HW' will force the send to trgVPort [%d]) \n",
        multi_target_vport_evidx,
        descrPtr->trgVPort));

    descrPtr->useVidx = 1;
    descrPtr->eVidx = multi_target_vport_evidx;

    /* give indication to the L2MLL to know to do :
        For all replicas overwrite L2MLL<Trg vPort 0(1)> such that L2MLL<Trg vPort 0(1)>= temp (inDesc<Trg vPort>)
    */
    descrPtr->multiTargetVPort.isUsed       = 1;
    descrPtr->multiTargetVPort.origTrgVPort = descrPtr->trgVPort;

    /* 'destroy' the target vPort indication in the descriptor.
        it is not needed but just to change it from previous value */
    descrPtr->trgVPort   = 0;
    descrPtr->trgIsVPort = 0;
}

/**
* @internal snetLion3MllStart function
* @endinternal
*
* @brief   SIP5 : entry point to MLL unit , before any L3/L2 replications
*
* @param[in] devObjPtr                  - pointer to device object
* @param[in,out] descrPtr               - pointer to frame descriptor
*/
static GT_VOID snetLion3MllStart
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC   * descrPtr
)
{
    DECLARE_FUNC_NAME(snetLion3MllStart);

    GT_U32  regValue;
    GT_32   startBit;

    if(descrPtr->useVidx || descrPtr->targetIsTrunk)
    {
        __LOG(("Multi-Target ePort/vPort : not relevant to useVidx == 1 or targetIsTrunk == 1 \n"));
        return;
    }

    smemRegGet(devObjPtr, SMEM_LION3_MLL_REPLICATED_TRAFFIC_CTRL_REG(devObjPtr) ,&regValue);
    if(descrPtr->trgIsVPort)/* sip7 */
    {
        /*<Multi-Target vPort Enable>*/
        if(0 == SMEM_U32_GET_FIELD(regValue,15,1))
        {
            __LOG(("Multi-Target vPort : globally disabled \n"));
            return;
        }

        __LOG(("Multi-Target vPort : globally enabled \n"));
        snetSip7MllMultiTargetVPorts(devObjPtr,descrPtr);
    }
    else
    {
        startBit = SMEM_CHT_IS_SIP6_30_GET(devObjPtr) ? 14 : 11;

        /*<Multi-Target ePort Enable>*/
        if(0 == SMEM_U32_GET_FIELD(regValue,startBit,1))
        {
            __LOG(("Multi-Target vPort : globally disabled \n"));
            return;
        }

        __LOG(("Multi-Target ePort : globally enabled \n"));
        snetSip5MllMultiTargetEPorts(devObjPtr,descrPtr);
    }

    return;

}

/**
* @internal snetChtMll function
* @endinternal
*
* @brief   Ingress MLL unit (L2,L3 replications)
*
* @param[in] devObjPtr                - pointer to device object
* @param[in,out] descrPtr                 - pointer to frame descriptor
*/
static GT_VOID snetChtMll
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC   * descrPtr
)
{
    simLogPacketFrameUnitSet(SIM_LOG_FRAME_UNIT_MLL_E);

    if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        /* entry point to MLL unit , before any L3/L2 replications */
        SIM_LOG_PACKET_DESCR_SAVE
        snetLion3MllStart(devObjPtr, descrPtr);
        SIM_LOG_PACKET_DESCR_COMPARE("snetLion3MllStart");

        /*Simulates queue selection */
        SIM_LOG_PACKET_DESCR_SAVE
        snetChtIngressL2MllQueueSelection(devObjPtr,descrPtr);
        SIM_LOG_PACKET_DESCR_COMPARE("snetChtIngressL2MllQueueSelection");
    }

    /* L3 (Ipv4/6) MLL Replication */
    SIM_LOG_PACKET_DESCR_SAVE
    snetChtIngressL3IpReplication(devObjPtr, descrPtr);
    SIM_LOG_PACKET_DESCR_COMPARE("(on orig descriptor) snetChtIngressL3IpReplication \n");

    /* L2 MLL Replication */
    SIM_LOG_PACKET_DESCR_SAVE
    snetChtIngressL2MllEngine(devObjPtr, descrPtr);
    SIM_LOG_PACKET_DESCR_COMPARE("(on orig descriptor) snetChtIngressL2MllEngine \n");
}

/**
* @internal snetChtIngressL2MllEngineVer1 function
* @endinternal
*
* @brief   Ingress L2 MLL engine - version 1 (xCat-C0)
*
* @param[in] devObjPtr                - pointer to device object
* @param[in,out] descrPtr             - pointer to frame descriptor
*
* @note code based on snetCht3IngressMllReplication
*
*/
static GT_VOID snetChtIngressL2MllEngineVer1
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC   * descrPtr
)
{
    DECLARE_FUNC_NAME(snetChtIngressL2MllEngineVer1);

    SKERNEL_FRAME_CHEETAH_DESCR_STC *origDescPtr;/* (pointer to)original descriptor info */
    GT_U32  index;/* index to a table */
    GT_U32  *memPtr;/*pointer to memory*/
    GT_U32  l2MllIndex;/* L2 MLL Index */
    GT_BOOL sendReplication; /* check if to send replication to current single
                                MLL , or skip it */
    GT_U32  origDescrFreeIndx;/* original number of descriptors used before starting the replications */
    SNET_CHT3_DOUBLE_MLL_STC mllEntry;/* L3 MLL entry format ! most of it is the same as L2 MLL format */
    SNET_CHT3_SINGLE_MLL_STC *singleMllPtr; /* pointer to the info about single
                                            L3 MLL (first/second half of the pair) most of it is the same as L2 MLL format*/
    GT_U32  ii;/* iterator */
    GT_U32  unkUcFilter,unkMcFilter,bcFilter;   /* L2Mll specific fields*/
    GT_U32  meshId;                             /* L2Mll specific fields*/
    GT_U32  localSwitchingEnable;               /* L2Mll specific fields*/
    GT_U32  isAps1Plus1LogicalPort;/*L2Mll specific fields .
                        Used for Source filtering on UNI-PW when duplication is done for 1+1 protection
                        0x0 = NonProtection_1Plus1; NonProtection_1Plus1; This duplication is not for 1+1 protection;
                        0x1 = Protection_1Plus1; Protection_1Plus1; This duplication is done for 1+1 protection;   */
    GT_U32  localSwitchingPortMask;/* mask on the port when do local switching to support 1+1 protection */

    GT_U32  bitsOffset;/* bits offset*/

    if(descrPtr->packetCmd != SKERNEL_EXT_PKT_CMD_FORWARD_E &&
       descrPtr->packetCmd != SKERNEL_EXT_PKT_CMD_MIRROR_TO_CPU_E &&
       descrPtr->packetCmd != SKERNEL_EXT_PKT_CMD_TO_TRG_SNIFFER_E )
    {
        return;
    }

    if(devObjPtr->vplsModeEnable.mll == 0 ||
       descrPtr->validMll == 1 ||
       descrPtr->tunnelStart == 1 ||
       descrPtr->routed == 1)
    {
        /* not triggering the L2Mll format */
        __LOG(("not triggering the L2Mll format"));
        return;
    }

    if(descrPtr->useVidx)
    {
        /*If (Enable L2MLL table entry==1) DescOut<MLLPtr>=InDesc<VIDX>*/
        index = descrPtr->eVidx / 32;/* each entry hold 32 L2MllEn bits*/

        memPtr = smemMemGet(devObjPtr,SMEM_XCAT_L2_MLL_VIDX_TO_MLL_MAPPING_TBL_MEM(devObjPtr,index));

        if( 0 ==  SMEM_U32_GET_FIELD(memPtr[0],(descrPtr->eVidx & 0x1f) , 1))
        {
            /* the L2Mll not enabled for this vidx */
            __LOG(("the L2Mll not enabled for this vidx"));
            return;
        }
        /*VIDX value is used as MLLPtr*/
        l2MllIndex = descrPtr->eVidx;
    }
    else   /* used for supporting 1+1 protection of unicast packets*/
    {
        if(descrPtr->targetIsTrunk)
        {
            return;
        }
        /*Map {TrgDev, TrgPort} to DescOut<MLLPtr> using Virtual Port to MLL mapping table*/

        if(descrPtr->trgDev & 0xFFFFFFF0)
        {
            /*If TrgDev[4]==1 then DescOut<MLLPtr> is set to 0x0 (NULL)*/
            return;
        }

        index = (descrPtr->trgEPort & 0x3f) | /* 6 bits from the target port */
                (descrPtr->trgDev & 0xf) << 6;/* 4 bits from the target device */

        memPtr = smemMemGet(devObjPtr,SMEM_XCAT_L2_MLL_VIRTUAL_PORT_TO_MLL_MAPPING_TBL_MEM(devObjPtr,index));

        /*Table entry is the new DescOut<MLLPtr> except when entry value is 0x0 (NULL)*/
        l2MllIndex = memPtr[0];

        if(l2MllIndex == 0)
        {
            return;
        }
    }

    /* duplicate descriptor from the ingress core */
    origDescPtr = snetChtEqDuplicateDescr(descrPtr->ingressDevObjPtr,descrPtr);

    /* save the number of used descriptors after the general duplication */
    origDescrFreeIndx = devObjPtr->descrFreeIndx;

    /**************************/
    /* start the replications */
    /**************************/

    while(1)
    {
        snetCht3ReadMllEntry(devObjPtr,l2MllIndex,&mllEntry);

        memPtr = smemMemGet(devObjPtr,SMEM_CHT3_ROUTER_MULTICAST_LIST_TBL_MEM(devObjPtr,l2MllIndex));

        /* check the 2 MLL sections */
        __LOG(("check the 2 MLL sections"));
        for(ii = 0 ; ii < 2 ; ii++)
        {
            sendReplication = GT_TRUE;
            if(ii == 0)
            {
                singleMllPtr = &mllEntry.first_mll;
            }
            else
            {
                singleMllPtr = &mllEntry.second_mll;
            }

            bitsOffset = (64*ii);

            unkUcFilter  =   snetFieldValueGet(memPtr,2 + bitsOffset ,1);
            unkMcFilter  =   snetFieldValueGet(memPtr,3 + bitsOffset ,1);
            bcFilter     =   snetFieldValueGet(memPtr,4 + bitsOffset ,1);
            meshId       =   snetFieldValueGet(memPtr,5 + bitsOffset ,2);
            isAps1Plus1LogicalPort  = snetFieldValueGet(memPtr,7 + bitsOffset ,1);

            localSwitchingEnable    = snetFieldValueGet(memPtr,50 + bitsOffset ,1);

            if(meshId)
            {
                if(meshId == descrPtr->vplsInfo.srcMeshId)
                {
                    /*if matching MLL action entry<meshId> descriptor is not duplicated. Used for split horizon filtering */
                    __LOG(("if matching MLL action entry<meshId> descriptor is not duplicated. Used for split horizon filtering"));
                    sendReplication = GT_FALSE;
                }
            }

            if(localSwitchingEnable == 0)
            {
                localSwitchingPortMask = isAps1Plus1LogicalPort ?
                                        0xFFFFFFFE :/* bit 0 is not in the mask */
                                        0xFFFFFFFF; /* all bits are in the mask */

                if((singleMllPtr->lll.dstInterface ==  SNET_DST_INTERFACE_PORT_E) &&
                    ((singleMllPtr->lll.interfaceInfo.devPort.port & localSwitchingPortMask) == (descrPtr->bridgeToMllInfo.virtualSrcPort & localSwitchingPortMask) &&
                    (singleMllPtr->lll.interfaceInfo.devPort.devNum == descrPtr->bridgeToMllInfo.virtualSrcDev)))
                {
                    /* came from the {dev,port} that need to send to it --> filter it */
                    __LOG(("came from thedev,port} that need to send to it --> filter it"));
                    sendReplication = GT_FALSE;
                }
            }


            /* check replication for the unknown/unregistered traffic */
            if(descrPtr->egressFilterRegistered == 0)
            {
                switch(descrPtr->macDaType)
                {
                    case SKERNEL_UNICAST_MAC_E:
                        if(unkUcFilter)
                        {
                            sendReplication = GT_FALSE;
                        }
                        break;
                    case SKERNEL_MULTICAST_MAC_E:
                        if(unkMcFilter)
                        {
                            sendReplication = GT_FALSE;
                        }
                        break;
                    default:/* BC traffic */
                        if(bcFilter)
                        {
                            sendReplication = GT_FALSE;
                        }
                        break;
                }
            }

            if(sendReplication == GT_TRUE)
            {
                /* the vid is not part of the L2Mll entry ! */
                singleMllPtr->vid = descrPtr->eVid;

                /* set the target outLif into descriptor - for single MLL
                   and Update mll counters */
                snetCht3IngressMllSingleMllOutlifSet(devObjPtr, descrPtr,singleMllPtr);

                /* send the replication down the pipe */
                snetChtEq(devObjPtr, descrPtr);

                simLogPacketFrameUnitSet(SIM_LOG_FRAME_UNIT_MLL_E);

                /* restore values of the descriptor */
                *descrPtr = *origDescPtr;

                /* restore the number of used descriptors in the device */
                devObjPtr->descrFreeIndx = origDescrFreeIndx;

                /*******************/
                /* end replication */
                /*******************/
            }

            if (singleMllPtr->last == 1)
            {
                 break;
            }
        }

        if(mllEntry.nextPtr == 0)
        {
            /* NOTE : we should not get here because the second section of MLL
                should have been set to singleMllPtr->last = 0 */
            break;
        }
        /* update the address for the next MLL */
        l2MllIndex = mllEntry.nextPtr;
    }

    /*NOTE: the original descriptor should not be flooded !*/
    /*assigned a HARD_DROP packet*/
    snetChtIngressCommandAndCpuCodeResolution(devObjPtr,descrPtr,
                                              descrPtr->packetCmd,
                                              SKERNEL_EXT_PKT_CMD_HARD_DROP_E,
                                              descrPtr->cpuCode,
                                              0,/*don't care*/
                                              SNET_CHEETAH_ENGINE_UNIT_L2MLL_E,
                                              GT_TRUE);


    return;
}


/**
* @internal snetLion3IngressMllAccessCheck function
* @endinternal
*
* @brief   Lion3 : check that IP/L2 MLL is not access out of range.
*         generate interrupt in case of access violation.
* @param[in] devObjPtr                - pointer to device object
* @param[in,out] descrPtr                 - pointer to frame descriptor
* @param[in] usedForIp                - GT_TRUE  - used for IP-MLL
*                                      GT_FALSE - used for L2-MLL
* @param[in] index                    -  into the MLL table
*
* @retval GT_TRUE  - error (access out of range) , and interrupt was generated.
*                    the MLL memory should NOT be accessed
*         GT_FALSE - no error, can continue MLL processing.
*/
GT_BOOL snetLion3IngressMllAccessCheck
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC   * descrPtr,
    IN GT_BOOL      usedForIp,
    IN GT_U32       index
)
{
    DECLARE_FUNC_NAME(snetLion3IngressMllAccessCheck);

    GT_U32  typeIndex = (usedForIp == GT_TRUE) ? 0 : 1;
    GT_BOOL error = GT_FALSE;
    GT_CHAR *typeNameArr[2] = { "IP-MLL" , "L2-MLL" };
    GT_CHAR *typeNamePtr = typeNameArr[typeIndex];
    GT_U32  interruptBitIndexArr[2] = {1,2};/* bit in register : MLL Interrupt Cause Register*/
    GT_U32  regAddr;
    GT_U32  regValue;
    GT_U32  limit;
    GT_U32  mllPointerLimitOffset=0;
    GT_U32  mllPointerLimitSize=0;
    GT_U32  mllAccessCheckMinMaxOffset=0;
    GT_U32  mllAccessCheckMinMaxSize=0;
    GT_U32  mllTableInvalidAccessCheckEnableOffset=0;
    GT_U32  mllTableInvalidAccessCheckEnableSize=0;

    if(usedForIp == GT_TRUE)
    {
        regAddr = SMEM_LION3_MLL_IP_MLL_TABLE_RESOURCE_SHARING_CONFIG_REG(devObjPtr);
    }
    else
    {
        regAddr = SMEM_LION3_MLL_L2_MLL_TABLE_RESOURCE_SHARING_CONFIG_REG(devObjPtr);
    }

    if(SMEM_CHT_IS_SIP7_GET(devObjPtr))
    {
        mllPointerLimitOffset=0;
        mllPointerLimitSize=17;
        mllAccessCheckMinMaxOffset=17;
        mllAccessCheckMinMaxSize=1;
        mllTableInvalidAccessCheckEnableOffset=18;
        mllTableInvalidAccessCheckEnableSize=1;
    }
    else
    {
        mllPointerLimitOffset=0;
        mllPointerLimitSize=16;
        mllAccessCheckMinMaxOffset=16;
        mllAccessCheckMinMaxSize=1;
        mllTableInvalidAccessCheckEnableOffset=17;
        mllTableInvalidAccessCheckEnableSize=1;
    }

    smemRegGet(devObjPtr, regAddr,&regValue);
    /*<L2 MLL Table Invalid Access Check Enable>*/
    if(SMEM_U32_GET_FIELD(regValue,mllTableInvalidAccessCheckEnableOffset,mllTableInvalidAccessCheckEnableSize))
    {
        __LOG(("%s Table Invalid Access Check Enabled \n",
            typeNamePtr));

        limit = SMEM_U32_GET_FIELD(regValue,mllPointerLimitOffset,mllPointerLimitSize);
        /*Determines whether <L2 MLL Pointer Limit> is used as an upper limit (max), or a lower limit (min)*/
        /*<L2 MLL Access Check MinMax>*/
        if(SMEM_U32_GET_FIELD(regValue,mllAccessCheckMinMaxOffset,mllAccessCheckMinMaxSize))
        {
            /*Max; <L2 MLL Pointer Limit> is used as an upper limit*/
            __LOG(("< %s MLL Pointer Limit> is used as an upper limit \n",
                typeNamePtr));

            if(index > limit)
            {
                /* limit error */
                error = GT_TRUE;
                __LOG(("ERROR : %s limit error : index[%x] > max[%x] \n",
                    typeNamePtr,
                    index,
                    limit));
            }
        }
        else
        {
            /*Min; <L2 MLL Pointer Limit> is used as a lower limit*/
            __LOG(("< %s MLL Pointer Limit> is used as a lower limit \n",
                typeNamePtr));
            if(index < limit)
            {
                /* limit error */
                error = GT_TRUE;
                __LOG(("ERROR : %s limit error : index[%x] < min[%x] \n",
                    typeNamePtr,
                    index,
                    limit));
            }
        }
    }

    if(error == GT_TRUE)
    {
        /* generate interrupt */
        __LOG(("Generate interrupt %s access violation: in MLL Interrupt Cause Register \n",
            typeNamePtr));
        snetChetahDoInterrupt(devObjPtr,
                              SMEM_LION3_MLL_INTERRUPT_CAUSE_REG(devObjPtr),
                              SMEM_LION3_MLL_INTERRUPT_MASK_REG(devObjPtr),
                              (1 << interruptBitIndexArr[typeIndex]),
                              (GT_U32)(1<<31));/* dummy index in global register */
    }

    return error;
}

/**
* @internal snetChtIngressL2Mll_preCallToEq function
* @endinternal
*
* @brief   Ingress L2 MLL engine , just before call to EQ
*
* @param[in] devObjPtr                - pointer to device object
* @param[in,out] descrPtr                 - pointer to frame descriptor
*/
static GT_VOID snetChtIngressL2Mll_preCallToEq
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC   * descrPtr
)
{
    DECLARE_FUNC_NAME(snetChtIngressL2Mll_preCallToEq);
    /* Update L2 mll counters */
    __LOG(("Update L2_MLL counters \n"));
    snetCht3mllCounters(devObjPtr, descrPtr , GT_TRUE/*count L2 MLL (not IP MLL)*/);

    if(SMEM_CHT_IS_SIP6_GET(devObjPtr) &&
       descrPtr->useVidx == 0 &&
       descrPtr->packetCmd == SKERNEL_EXT_PKT_CMD_FROM_CPU_E)
    {
        /* this is fix for JIRA : MLL-427 : FROM_CPU with target eVIDX replications have UNTAGGED egress tag state */
        __LOG(("Sip6 : 'FROM_CPU' replications to eports changed to 'FORWARD' to allow proper vlan tagging on egress \n"));
        descrPtr->packetCmd = SKERNEL_EXT_PKT_CMD_FORWARD_E;
    }

    /* MLL Ping-Pong Identifier */
    if(SMEM_CHT_IS_SIP6_10_GET(devObjPtr))
    {
        /* New descriptor bit <REP_MLL> in Ironman, is used to indicate if the
         * descriptor was duplicated by IPMLL or L2MLL. For previous
         * devices, Copy Reserved bit 19 is used.
         */
        if(SMEM_CHT_IS_SIP6_30_GET(devObjPtr))
        {
            descrPtr->repMll = 1;
        }
        else
        {
            descrPtr->copyReserved |= (1 << 19);
        }
        __LOG(("MLL Ping-Pong: repMll [0x%x] copyReserved [0x%x]\n", descrPtr->repMll, descrPtr->copyReserved));
    }
}


/**
* @internal snetChtIngressL2MllQueueCheck function
* @endinternal
*
* @brief   check given mll Queue
*          in case of queue weight == 0 set packet to hard drope
*
* @param[in] devObjPtr                  - pointer to device object
* @param[in] tcQueue                    - nuber of queue
* @param[in,out] descrPtr               - pointer to frame descriptor
*/
static GT_VOID snetChtIngressL2MllQueueCheck
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                  tcQueue,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC   * descrPtr
)
{
    DECLARE_FUNC_NAME(snetChtIngressL2MllQueueCheck);

    GT_U32 regValue = 0;
    GT_U32 queueWeight;
    GT_U32 offset ,fieldLength ;

    if (tcQueue > 3)
    {
        __LOG(("ERROR : selected Queue is out of range tcQueue > 3" ));
        return;
    }

    if(SMEM_CHT_IS_SIP6_GET(devObjPtr))
    {
        offset = tcQueue*2;
        fieldLength=2;
    }
    else
    {
        offset = tcQueue*1;
        fieldLength=1;
    }

    /*continue check only in case of SDWRR , SDWRR == 0 SP ==1*/
    smemRegGet(devObjPtr, SMEM_LION3_MLL_MULTI_TARGET_TC_QUEUES_PRIORITY_CONF_REG(devObjPtr), &regValue);
    if (0 == SMEM_U32_GET_FIELD(regValue,offset,fieldLength))
    {
        /*get the queue weight */
        smemRegGet(devObjPtr, SMEM_LION3_MLL_MULTI_TARGET_TC_QUEUES_WEIGHT_CONF_REG(devObjPtr), &regValue);
        queueWeight = SMEM_U32_GET_FIELD(regValue,tcQueue*8,8);
        __LOG (("selected tcQueue %d is part of SDWRR with weight %d \n",tcQueue,queueWeight));
        if (queueWeight == 0)
        {
            __LOG (("queueWeight == 0 ,set packetCmd to HARD_DROP \n"));
            /*assigned a HARD_DROP packet*/
            snetChtIngressCommandAndCpuCodeResolution(devObjPtr,descrPtr,
                                                      descrPtr->packetCmd,
                                                      SKERNEL_EXT_PKT_CMD_HARD_DROP_E,
                                                      descrPtr->cpuCode,
                                                      0,/*don't care*/
                                                      SNET_CHEETAH_ENGINE_UNIT_L2MLL_E,
                                                      GT_TRUE);

            smemRegGet(devObjPtr, SMEM_LION3_MLL_MC_QUEUE_DROP_CNTR_REG(devObjPtr,tcQueue), &regValue);
            regValue++;
            smemRegSet(devObjPtr, SMEM_LION3_MLL_MC_QUEUE_DROP_CNTR_REG(devObjPtr,tcQueue), regValue);\
        }
    }
}


/**
* @internal snetChtIngressL2MllQueueSelection function
* @endinternal
*
* @brief   Simulates mll queue selection
*
* @param[in] devObjPtr                - pointer to device object
* @param[in,out] descrPtr                 - pointer to frame descriptor
*/
static GT_VOID snetChtIngressL2MllQueueSelection
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC   * descrPtr
)
{
    DECLARE_FUNC_NAME(snetChtIngressL2MllQueueSelection);

    GT_BOOL queueSelected = GT_FALSE;
    GT_U32 regValue = 0;
    GT_U32 ControlMCTCQueue,mCRPFFailTCQueue,tcQueue = 0xff;
    GT_U32 qosOffset,profile;
    GT_U32 isIpv6 = (descrPtr->isIPv4 || descrPtr->isFcoe) ? 0 : 1;
    SNET_CHEETAH2_L3_CNTRL_PACKET_INFO    cntrlPcktInfo;
    GT_U32    fieldOffset = 16;

    if(SMEM_CHT_IS_SIP6_30_GET(devObjPtr))
    {
        fieldOffset = 24;
    }

    memset(&cntrlPcktInfo, 0, sizeof(SNET_CHEETAH2_L3_CNTRL_PACKET_INFO));

    smemRegGet(devObjPtr, SMEM_LION3_MLL_MULTI_TARGET_TC_GLOBAL_CONF_REG(devObjPtr) ,&regValue);
    ControlMCTCQueue = SMEM_U32_GET_FIELD(regValue,fieldOffset,2);
    mCRPFFailTCQueue = SMEM_U32_GET_FIELD(regValue,fieldOffset+2,2);

    if (descrPtr->useVidx == 0)
    {
        __LOG(("snetChtIngressL2MllQueueSelection : No L2MLL : because not 'multi-destination' (useVidx == 0) \n"));
        return;
    }

    /*Multi-target Control traffic*/
    if (descrPtr->packetCmd == SKERNEL_EXT_PKT_CMD_FROM_CPU_E || descrPtr->packetCmd == SKERNEL_EXT_PKT_CMD_MIRROR_TO_CPU_E)
    {
        __LOG(("packet is Multi-target Control traffic \n" ));
        tcQueue = ControlMCTCQueue ;
        queueSelected = GT_TRUE;
    }

    /*RPF Fail Traffic*/
    if (queueSelected == GT_FALSE && descrPtr->ipm == 1 && (descrPtr->isIPv4 || isIpv6 ) )
    {
        if (GT_TRUE == snetCht2L3iGetRpfFailCmdFromMll(devObjPtr, descrPtr,descrPtr->mllSelector, &cntrlPcktInfo))
        {
            __LOG(("packet is  RPF Fail Traffic \n"));
            tcQueue = mCRPFFailTCQueue ;
            queueSelected = GT_TRUE;
        }
    }

    /*Multicast Tunnel Duplicated Traffic*/
    if (queueSelected == GT_FALSE && descrPtr->isMultiTargetReplication ==1 )
    {
        __LOG(("packet is Multicast Tunnel Duplicated Traffic \n"));
        tcQueue = 0;
    }

    if (queueSelected == GT_FALSE )
    {
        profile = descrPtr->qos.qosProfile;
        if(descrPtr->marvellTagged == 1 && descrPtr->qos.ingressExtendedMode)
        {
            __LOG(("packet queue is set by qos Extended Mode \n"));
            if (profile >= 32)
            {
                __LOG(("QoS profile index[%d] too big in Extended Mode, should be < 32\n", profile));
                profile %= 32;
                __LOG(("WARNING : Use only 5 LS bits of QoS profile, final index is[%d]\n", profile));
            }

            smemRegGet(devObjPtr, SMEM_LION3_MLL_EXT_QOS_PROFILE_TO_MULTI_TARGET_TC_QUEUES_REG(devObjPtr,profile), &regValue);
        }
        else
        {
            __LOG(("packet queue is set by qos \n" ));
            smemRegGet(devObjPtr, SMEM_LION3_MLL_QOS_PROFILE_TO_MULTI_TARGET_TC_QUEUES_REG(devObjPtr,profile), &regValue);
        }
        qosOffset = (profile % 16) * 2;
        tcQueue = SMEM_U32_GET_FIELD(regValue,qosOffset,2);
        queueSelected = GT_TRUE;
    }

    __LOG(("selected queue %d \n",tcQueue ));

    snetChtIngressL2MllQueueCheck(devObjPtr,tcQueue,descrPtr);
}

/**
* @internal snetChtIngressL2MllEngine function
* @endinternal
*
* @brief   Ingress L2 MLL engine
*
* @param[in] devObjPtr                - pointer to device object
* @param[in,out] descrPtr                 - pointer to frame descriptor
*/
static GT_VOID snetChtIngressL2MllEngine
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC   * descrPtr
)
{
    DECLARE_FUNC_NAME(snetChtIngressL2MllEngine);

    SKERNEL_FRAME_CHEETAH_DESCR_STC *previousDescPtr;/* (pointer to)'previous' descriptor info */
    SKERNEL_FRAME_CHEETAH_DESCR_STC *nextDescPtr;/* (pointer to)'next' descriptor info */
    SKERNEL_FRAME_CHEETAH_DESCR_STC *origDescPtr;/* (pointer to)original descriptor info */
    GT_U32  enableMllLookupForAllEVidx;/* Enable MLL Lookup for All eVIDX */
    GT_U32  l2MllLttIndex;/* index to the LTT table (of L2MLL) */
    GT_U32  *memPtr;/*pointer to memory*/
    GT_U32  l2MllIndex;/* L2 MLL Index */
    GT_U32  entrySelector;/* the L2 MLL entry hold 2 parts : 0,1.*/
    GT_U32  maskBitmap;/*mask bitmap from the L2Mll entry*/
    GT_U32  egressMeshId;/* meshId from the L2Mll entry */
    GT_BIT  filterEnable;/* UC/MC/BC filter - from the L2Mll entry*/
    GT_U32  ttlThreshold;/*ttl Threshold - from the L2Mll entry*/
    GT_U32  last;/* indication that last replication*/
    GT_U32  mllMaskProfile;/* MLL Mask Profile */
    GT_U32  value,value2;/*field value*/
    GT_BOOL sendReplication; /* check if to send replication to current single
                                MLL , or skip it */
    GT_BIT  TTL_EXCEPTION_FLAG;
    GT_BIT  targetIsTrunk;/* mll target is trunk ?*/
    GT_BIT  targetIsVidx; /* mll target is vidx ?*/
    GT_U32  targetEPort=0;  /* mll target eport */
    GT_U32  targetDevice=0; /* mll target device */
    GT_U32  targetTrunkID=0;/* mll target trunk */
    GT_U32  targetVidx=0;   /* mll target vidx */
    GT_BIT  targetIsVPort=0;/* SIP7 : mll target is vPort ?*/
    GT_U32  targetVPort=0;  /* SIP7 : mll target vPort */
    GT_U32  globalEPortVal=0;/*SIP5 :Value for Global ePort. Refer to Global ePort logic below*/
    GT_U32  globalEPortMask=0;/*SIP5 :"0" masks the corresponding bit in the <GLOBALePortVal>*/
    GT_U32  globalEPortMinValue=0,globalEPortMaxValue=0;/* sip6 :min,max and value for l2-ecmp global eports */
    GT_U32  l2mllGlobalEPortEnable;/*sip6 : indication that the global eport range is enabled */
    GT_BOOL didAnyReplication = GT_FALSE;/*No Replication There is a corner case where a packet
            accesses the L2 MLL and does not trigger any replications.
            This is possible if all the elements in the linked list are masked out, or if all copies are filtered.
            In these cases the original descriptor is assigned a SOFT_DROP packet command and forwarded to the EQ.*/
    GT_U32  origDescrFreeIndx;/* original number of descriptors used before starting the replications */
    GT_U32  ingressMeshId;/* mesh id for this packet */
    GT_U32  meshIdOffset;/* offset from sstId to use for ingressMeshId */
    GT_U32  meshIdSize;/* number of bits from sstId to use for ingressMeshId */
    GT_U32  ePortMaskForSrcFilter;/*mask on the ePort when doing SRC filtering */
    static GT_CHAR *srcFilterReasonNameArr[2] = {"MC local switching filtering" , "One Plus One Filtering"};
    GT_CHAR *srcFilterReasonNamePtr;
    GT_BIT  mcLocalSwitchingEnable;/*Determines whether traffic can be replicated to the source ePort, i.e., if the source {dev,ePort}
                                    is equal to the {dev,ePort} in this entry, then <MCLocalSwitchingEnable>
                                    determines wether the packet is replicated or not*/
    GT_U32  srcFilter_isTrunk;/* the isTrunk to use for src filtering */
    GT_U32  srcFilter_portTrunk;/* the port/trunk to use for src filtering */
    GT_U32  srcFilter_device;/* the device to use for src filtering */
    GT_U32  srcFilter_isVPort=0;/* sip7 : the isVPort to use for src filtering */
    GT_U32  srcFilter_vPort=0; /* sip7 : the vPort to use for src filtering */
    GT_U32  isLastReplication;/*indication that this is last replication of the MLL unit */
    GT_BIT  nextReplicationExists = GT_FALSE;/* there is at least one more L2Mll replication */
    GT_U32  regAddr;
    GT_U32  fieldVal;
    GT_BOOL filter_copy = GT_FALSE;
    GT_U32  vidxNumBits;
    GT_U32  numBits_mllIndex = SMEM_CHT_IS_SIP7_GET(devObjPtr) ? 17 : 15;
    GT_U32  numNodes;/* number of nodes in the L2MLL for the specific eVidx */
    GT_U32  orig_eVidx;/* the original eVidx */
    GT_U32  cfgBackupValueArr[4];/* sip7 : array of cfg Backup Value (4 words) */
    GT_U32  cfgBackupMaskArr[4]; /* sip7 : array of cfg Backup Mask  (4 words) */
    GT_U32  ii;

    GT_U32 tempMllPairArr[8];/* support 256 bits (although needed 185 bits) */
    GT_U32 mllPairsLineIndex;
    GT_U32 mllPairsStartBit;

    if(SMEM_CHT_IS_SIP5_20_GET(devObjPtr))
    {
        regAddr = SMEM_CHT2_MLL_GLB_CONTROL_REG(devObjPtr);
        smemRegFldGet(devObjPtr, regAddr, 20, 1, &fieldVal);

        if(fieldVal)
        {
            __LOG(("sip5_20 : No L2MLL : because globally disabled \n"));
            return;
        }
    }

    switch(devObjPtr->l2MllVersionSupport)
    {
        case 0:
            /* those are legacy devices (that are not xCat-C0)*/
            return;
        case 1:
            /* do the xCat-C0 L2 MLL */
            __LOG(("do the xCat-C0 L2 MLL"));
            snetChtIngressL2MllEngineVer1(devObjPtr,descrPtr);
            return;
        default:
            /* this is SIP5 device */
            break;
    }

    if(SMEM_CHT_IS_SIP6_10_GET(devObjPtr))
    {
        vidxNumBits = 14;
    }
    else
    {
        vidxNumBits = 12;
    }

    /* by default the vidx is 12 LSB of the eVidx*/
    descrPtr->eArchExtInfo.vidx = SMEM_U32_GET_FIELD(descrPtr->eVidx,0,vidxNumBits);

    if(descrPtr->packetCmd != SKERNEL_EXT_PKT_CMD_FORWARD_E &&
       descrPtr->packetCmd != SKERNEL_EXT_PKT_CMD_MIRROR_TO_CPU_E &&
       descrPtr->packetCmd != SKERNEL_EXT_PKT_CMD_FROM_CPU_E )
    {
        __LOG(("No L2MLL : because not 'FORWARD'/'MIRROR_TO_CPU'/'FROM_CPU' \n"));
        return;
    }

    if(descrPtr->useVidx == 0)
    {
        __LOG(("No L2MLL : because not 'multi-destination' (useVidx == 0) \n"));
        return;
    }

    orig_eVidx = descrPtr->eVidx;

    smemRegFldGet(devObjPtr, SMEM_LION3_MLL_LOOKUP_TRIGGER_CONFIG_REG(devObjPtr), 0, vidxNumBits+1, &value);
    enableMllLookupForAllEVidx = SMEM_U32_GET_FIELD(value,vidxNumBits,1);

   __LOG_PARAM(enableMllLookupForAllEVidx);

    if(enableMllLookupForAllEVidx == 0)
    {
        value = SMEM_U32_GET_FIELD(value,0,vidxNumBits);

        if(descrPtr->eVidx < (value + 1))
        {
            /* this is considered as VIDX and not as eVIDX */
            __LOG(("No L2MLL : because descrPtr->eVidx[0x%x] <= max_vidx_index[0x%x] \n",
                descrPtr->eVidx ,
                value));
            return;
        }

        l2MllLttIndex = descrPtr->eVidx - (value + 1);
    }
    else
    {
        l2MllLttIndex = descrPtr->eVidx;
    }

    __LOG_PARAM(l2MllLttIndex);

    smemRegGet(devObjPtr, SMEM_LION3_MLL_SOURCE_BASED_L2_MLL_FILTERING_REG(devObjPtr), &value);
    meshIdOffset = SMEM_U32_GET_FIELD(value,0,4);
    meshIdSize   = SMEM_U32_GET_FIELD(value,4,4);

    __LOG_PARAM(meshIdOffset);
    __LOG_PARAM(meshIdSize);


    if(meshIdOffset > 11)
    {
        __LOG(("Global Config: ERROR <meshIdOffset>[%d] > 11 \n",
            meshIdOffset));
    }
    else if (meshIdSize > 8)
    {
        __LOG(("Global Config: ERROR <meshIdSize>[%d] > 8 \n",
            meshIdSize));
    }

    ingressMeshId = SMEM_U32_GET_FIELD(descrPtr->sstId,meshIdOffset,meshIdSize);
    __LOG_PARAM(ingressMeshId);
    if(ingressMeshId)
    {
        __LOG(("calculated ingressMeshId according SST-ID and global config is [%d]",ingressMeshId));
    }


    /* the eVidx >= 4K or enableMllLookupForAllEVidx == 1 -->
       use the eVidx (or eVidx-4K) as LTT index */

    memPtr = smemMemGet(devObjPtr,SMEM_LION3_L2_MLL_LTT_TBL_MEM(devObjPtr,l2MllLttIndex));

    /* L2 MLL Index */
    entrySelector = snetFieldValueGet(memPtr,0,1);

    l2MllIndex = snetFieldValueGet(memPtr,1,numBits_mllIndex);
    /* MLL Mask Profile */
    mllMaskProfile = snetFieldValueGet(memPtr,numBits_mllIndex+1,4);

    __LOG_PARAM(entrySelector);
    __LOG_PARAM(l2MllIndex);
    __LOG_PARAM(mllMaskProfile);

    if(SMEM_CHT_IS_SIP6_GET(devObjPtr))
    {
        /* check if global eport is enabled */
        regAddr = SMEM_CHT2_MLL_GLB_CONTROL_REG(devObjPtr);
        /*<L2MLL Global ePort Enable>*/
        smemRegFldGet(devObjPtr, regAddr, 22, 1, &l2mllGlobalEPortEnable);

        if(l2mllGlobalEPortEnable)
        {
            smemRegGet(devObjPtr, SMEM_SIP6_L2_MLL_GLOBAL_E_PORT_MIN_VALUE_REG(devObjPtr), &value);
            globalEPortMinValue = SMEM_U32_GET_FIELD(value,0,17);
            smemRegGet(devObjPtr, SMEM_SIP6_L2_MLL_GLOBAL_E_PORT_MAX_VALUE_REG(devObjPtr) , &value);
            globalEPortMaxValue = SMEM_U32_GET_FIELD(value,0,17);

            __LOG(("L2MLL : Global EPort Range is enabled {%d..%d} ({min..max})\n",
                globalEPortMinValue,globalEPortMaxValue));

            __LOG_PARAM(globalEPortMinValue);
            __LOG_PARAM(globalEPortMaxValue);
        }
        else
        {
            __LOG(("L2MLL : Global EPort Range is disabled \n"));
        }

    }
    else
    {
        l2mllGlobalEPortEnable = 1;/* for sharing the same code with sip6 */

        smemRegGet(devObjPtr, SMEM_LION3_L2_MLL_GLOBAL_E_PORT_REG(devObjPtr), &value);
        globalEPortVal = SMEM_U32_GET_FIELD(value,0,20);
        smemRegGet(devObjPtr, SMEM_LION2_L3_MLL_GLOBAL_E_PORT_MASK_REG(devObjPtr) , &value);
        globalEPortMask = SMEM_U32_GET_FIELD(value,0,20);

        __LOG_PARAM(globalEPortVal);
        __LOG_PARAM(globalEPortMask);
    }

    /* 'orig descriptor' : duplicate descriptor from the ingress core */
    origDescPtr = snetChtEqDuplicateDescr(descrPtr->ingressDevObjPtr,descrPtr);
    /* 'next replication' : duplicate descriptor from the ingress core */
    nextDescPtr = snetChtEqDuplicateDescr(descrPtr->ingressDevObjPtr,descrPtr);
    /* 'previous replication' : duplicate descriptor from the ingress core */
    previousDescPtr = snetChtEqDuplicateDescr(descrPtr->ingressDevObjPtr,descrPtr);
    /* save the number of used descriptors after the general duplication */
    origDescrFreeIndx = devObjPtr->descrFreeIndx;

    TTL_EXCEPTION_FLAG = 0;

    /* build the info needed for 'src filterring' */
    if(descrPtr->packetCmd == SKERNEL_EXT_PKT_CMD_FROM_CPU_E)
    {
        /*from_cpu :  MC excluded filtering for FromCpu */
        srcFilter_isTrunk   = descrPtr->excludeIsTrunk;
        srcFilter_portTrunk = descrPtr->excludeIsTrunk ?
                                    descrPtr->excludedTrunk :
                                    descrPtr->excludedPort;
        srcFilter_device    = descrPtr->excludedDevice;

        if(srcFilter_isTrunk == 0 && descrPtr->eArchExtInfo.fromCpu.excludedIsPhyPort == 1)
        {
            /* set values to make sure 'no match' of the ePort */
            srcFilter_portTrunk = 0xFFFFFFFF;
        }

        if(SMEM_CHT_IS_SIP7_GET(devObjPtr) && (srcFilter_isTrunk == 0))
        {
            /* is excludedIsPhyPort with value '2' */
            srcFilter_isVPort   = (descrPtr->eArchExtInfo.fromCpu.excludedIsPhyPort == 2);
            srcFilter_vPort     = descrPtr->excludedVPort;
        }
    }
    else
    {
        /* forward / mirrorToCpu : MC local switching filtering */
        srcFilter_isTrunk   = descrPtr->origIsTrunk;
        srcFilter_portTrunk = descrPtr->origSrcEPortOrTrnk;
        srcFilter_device    = descrPtr->srcDev;

        if(SMEM_CHT_IS_SIP7_GET(devObjPtr) && (srcFilter_isTrunk == 0))
        {
            /* is excludedIsPhyPort with value '2' */
            srcFilter_isVPort   = descrPtr->srcIsVPort;
            srcFilter_vPort     = descrPtr->srcVPort;
        }
    }

    __LOG_PARAM(srcFilter_isTrunk);
    __LOG_PARAM(srcFilter_portTrunk);
    __LOG_PARAM(srcFilter_device);
    if(SMEM_CHT_IS_SIP7_GET(devObjPtr))
    {
        __LOG_PARAM(srcFilter_isVPort);
        __LOG_PARAM(srcFilter_vPort);
    }

    __LOG(("Start L2 MLL replications \n"));

    numNodes = 0;

    do
    {
        sendReplication = GT_FALSE;
        filter_copy = GT_FALSE;

        if(GT_TRUE ==
            snetLion3IngressMllAccessCheck(devObjPtr,descrPtr,GT_FALSE,l2MllIndex))
        {
            /* the l2MllIndex is 'out of range' */
            break;
        }

        numNodes++;

        if(!SMEM_CHT_IS_SIP7_GET(devObjPtr))
        {
                memPtr = smemMemGet(devObjPtr,SMEM_CHT3_ROUTER_MULTICAST_LIST_TBL_MEM(devObjPtr,l2MllIndex));
        }
        else
        {
            mllPairsLineIndex = l2MllIndex / devObjPtr->sip7_mllInfo.numMllPairsEntriesInLine;
            mllPairsStartBit  = (l2MllIndex % devObjPtr->sip7_mllInfo.numMllPairsEntriesInLine) * devObjPtr->sip7_mllInfo.numMllPairsBitsPerEntry;
            /* get pointer to start of line with the 4 mll pairs */
            memPtr = smemMemGet(devObjPtr,SMEM_CHT3_ROUTER_MULTICAST_LIST_TBL_MEM(devObjPtr, mllPairsLineIndex));

            /* copy the specific mll pair into tempMllPairArr[0] */
            wm_copyBits(tempMllPairArr/*target ptr*/,0/*target bit*/,
                        memPtr/*source ptr*/ , mllPairsStartBit /*source bit*/ ,
                        devObjPtr->sip7_mllInfo.numMllPairsBitsPerEntry/*num bits to copy*/);

            /* set the 'mll pair' memory ptr to the 'temp' memory that hold the single entry */
            memPtr = &tempMllPairArr[0];
        }

        if(descrPtr->multiTargetVPort.isUsed)
        {
            if(numNodes > 2)
            {
                __LOG(("WARNING : potential configuration error : the eVidx [%d] L2MLL for 'multi-Target-VPort' should have only 2 nodes , but got [%d] \n",
                    orig_eVidx , numNodes));
            }

            /* treat the MLL as if targetIsVidx == 0 , targetIsTrunk == 0 , targetIsVPort == 1 */
            targetIsVidx  = 0;
            targetIsTrunk = 0;
            targetIsVPort = 1;

            targetVPort = descrPtr->multiTargetVPort.origTrgVPort;
            __LOG(("'Multi Target vPorts' support : ignore targetIsVidx,targetIsTrunk,targetIsVPort from MLL entry and sent to vPort[%d] \n",
                targetVPort));

            /* jump to point after vidx/trunk/port/vport info from the L2MLL entry */
            goto l2MllAfterTargetInterface_lbl;
        }

        targetIsVidx =
            SMEM_LION3_L2_MLL_ENTRY_FIELD_GET(devObjPtr,memPtr,l2MllIndex,
                entrySelector ?
                    SMEM_LION3_L2_MLL_TABLE_FIELDS_USE_VIDX_1:
                    SMEM_LION3_L2_MLL_TABLE_FIELDS_USE_VIDX_0);

        targetIsTrunk = 0;
        targetIsVPort = 0;

        if(targetIsVidx == 0)
        {
            targetIsTrunk =
                SMEM_LION3_L2_MLL_ENTRY_FIELD_GET(devObjPtr,memPtr,l2MllIndex,
                    entrySelector ?
                        SMEM_LION3_L2_MLL_TABLE_FIELDS_TARGET_IS_TRUNK_1:
                        SMEM_LION3_L2_MLL_TABLE_FIELDS_TARGET_IS_TRUNK_0);

            if(targetIsTrunk)
            {
                targetTrunkID =
                    SMEM_LION3_L2_MLL_ENTRY_FIELD_GET(devObjPtr,memPtr,l2MllIndex,
                        entrySelector ?
                            SMEM_LION3_L2_MLL_TABLE_FIELDS_TRG_TRUNK_ID_1:
                            SMEM_LION3_L2_MLL_TABLE_FIELDS_TRG_TRUNK_ID_0);
            }
            else
            if(SMEM_CHT_IS_SIP7_GET(devObjPtr) &&
                SMEM_LION3_L2_MLL_ENTRY_FIELD_GET(devObjPtr,memPtr,l2MllIndex,
                    entrySelector ?
                        SMEM_SIP7_L2_MLL_TABLE_FIELDS_USE_VPORT_1:
                        SMEM_SIP7_L2_MLL_TABLE_FIELDS_USE_VPORT_0))
            {
                targetIsVPort = 1;
                targetVPort =
                    SMEM_LION3_L2_MLL_ENTRY_FIELD_GET(devObjPtr,memPtr,l2MllIndex,
                        entrySelector ?
                            SMEM_SIP7_L2_MLL_TABLE_FIELDS_VPORT_1:
                            SMEM_SIP7_L2_MLL_TABLE_FIELDS_VPORT_0);
            }
            else
            {
                targetEPort =
                    SMEM_LION3_L2_MLL_ENTRY_FIELD_GET(devObjPtr,memPtr,l2MllIndex,
                        entrySelector ?
                            SMEM_LION3_L2_MLL_TABLE_FIELDS_TRG_EPORT_1:
                            SMEM_LION3_L2_MLL_TABLE_FIELDS_TRG_EPORT_0);
                targetDevice =
                    SMEM_LION3_L2_MLL_ENTRY_FIELD_GET(devObjPtr,memPtr,l2MllIndex,
                        entrySelector ?
                            SMEM_LION3_L2_MLL_TABLE_FIELDS_TRG_DEV_1:
                            SMEM_LION3_L2_MLL_TABLE_FIELDS_TRG_DEV_0);

                if(targetEPort == SNET_CHT_NULL_PORT_CNS)
                {
                    filter_copy = GT_TRUE;
                }
            }
        }
        else
        {
            targetVidx =
                SMEM_LION3_L2_MLL_ENTRY_FIELD_GET(devObjPtr,memPtr,l2MllIndex,
                    entrySelector ?
                        SMEM_LION3_L2_MLL_TABLE_FIELDS_VIDX_1:
                        SMEM_LION3_L2_MLL_TABLE_FIELDS_VIDX_0);
        }

        /* we are after setting : vidx/trunk/port/vPort as target for the replication */
        l2MllAfterTargetInterface_lbl:

        maskBitmap =
                SMEM_LION3_L2_MLL_ENTRY_FIELD_GET(devObjPtr,memPtr,l2MllIndex,
                    entrySelector ?
                        SMEM_LION3_L2_MLL_TABLE_FIELDS_MASK_BITMAP_1:
                        SMEM_LION3_L2_MLL_TABLE_FIELDS_MASK_BITMAP_0);

        /*Mask Bitmap*/
        if (mllMaskProfile && (0 == (maskBitmap & (1 << (mllMaskProfile - 1)))))
        {
            filter_copy = GT_TRUE;
        }

        /* check replication for the unknown/unregistered traffic */
        switch(descrPtr->macDaType)
        {
            case SKERNEL_UNICAST_MAC_E:
                /*Unknown UC Filter Enable*/
                filterEnable =
                    SMEM_LION3_L2_MLL_ENTRY_FIELD_GET(devObjPtr,memPtr,l2MllIndex,
                        entrySelector ?
                            SMEM_LION3_L2_MLL_TABLE_FIELDS_UNKNOWN_UC_FILTER_ENABLE_1:
                            SMEM_LION3_L2_MLL_TABLE_FIELDS_UNKNOWN_UC_FILTER_ENABLE_0);
                if(descrPtr->egressFilterRegistered == 0 && filterEnable)
                {
                    __LOG(("Unknown UC Filter Enable"));
                    filter_copy = GT_TRUE;
                }
                break;
            case SKERNEL_MULTICAST_MAC_E:
                /*Unregistered MC Filter Enable*/
                filterEnable =
                    SMEM_LION3_L2_MLL_ENTRY_FIELD_GET(devObjPtr,memPtr,l2MllIndex,
                        entrySelector ?
                            SMEM_LION3_L2_MLL_TABLE_FIELDS_UNREGISTERED_MC_FILTER_ENABLE_1:
                            SMEM_LION3_L2_MLL_TABLE_FIELDS_UNREGISTERED_MC_FILTER_ENABLE_0);
                if(descrPtr->egressFilterRegistered == 0 && filterEnable)
                {
                    __LOG(("Unregistered MC Filter Enable"));
                    filter_copy = GT_TRUE;
                }
                break;
            default:/* BC traffic */
                /*BC Filter Enable*/ /* NOTE: also for registered BC */
                filterEnable =
                    SMEM_LION3_L2_MLL_ENTRY_FIELD_GET(devObjPtr,memPtr,l2MllIndex,
                        entrySelector ?
                            SMEM_LION3_L2_MLL_TABLE_FIELDS_BC_FILTER_ENABLE_1:
                            SMEM_LION3_L2_MLL_TABLE_FIELDS_BC_FILTER_ENABLE_0);
                if(filterEnable)
                {
                    __LOG(("BC Filter Enable(also for registered BC)"));
                    filter_copy = GT_TRUE;
                }
                break;
        }

        egressMeshId =
            SMEM_LION3_L2_MLL_ENTRY_FIELD_GET(devObjPtr,memPtr,l2MllIndex,
                entrySelector ?
                    SMEM_LION3_L2_MLL_TABLE_FIELDS_MESH_ID_1:
                    SMEM_LION3_L2_MLL_TABLE_FIELDS_MESH_ID_0);

        if(ingressMeshId)
        {
            if(egressMeshId == ingressMeshId)
            {
                /*if matching MLL action entry<meshId> descriptor is not duplicated. Used for split horizon filtering */
                __LOG(("Source-based (Split Horizon) Filtering on meshId [%d] \n",
                    ingressMeshId));
                filter_copy = GT_TRUE;
            }
        }

        ttlThreshold =
            SMEM_LION3_L2_MLL_ENTRY_FIELD_GET(devObjPtr,memPtr,l2MllIndex,
                entrySelector ?
                    SMEM_LION3_L2_MLL_TABLE_FIELDS_TTL_THRESHOLD_1:
                    SMEM_LION3_L2_MLL_TABLE_FIELDS_TTL_THRESHOLD_0);
        /*TTL Threshold*/
        if(descrPtr->ttl < ttlThreshold)
        {
            __LOG(("failed to pass TTL Threshold [%d] < [%d] \n",
                descrPtr->ttl,ttlThreshold));
            TTL_EXCEPTION_FLAG = 1;
            filter_copy = GT_TRUE;
        }

        mcLocalSwitchingEnable =
            SMEM_LION3_L2_MLL_ENTRY_FIELD_GET(devObjPtr,memPtr,l2MllIndex,
                entrySelector ?
                    SMEM_LION3_L2_MLL_TABLE_FIELDS_MC_LOCAL_SWITCHING_ENABLE_1:
                    SMEM_LION3_L2_MLL_TABLE_FIELDS_MC_LOCAL_SWITCHING_ENABLE_0);

        /*MC local switching Enable*/
        if(mcLocalSwitchingEnable == 0 && (targetIsVidx == 0))
        {
            GT_BOOL src_filter = GT_FALSE;
            filterEnable =
                SMEM_LION3_L2_MLL_ENTRY_FIELD_GET(devObjPtr,memPtr,l2MllIndex,
                    entrySelector ?
                        SMEM_LION3_L2_MLL_TABLE_FIELDS_ONE_PLUS_ONE_FILTERING_ENABLE_1:
                        SMEM_LION3_L2_MLL_TABLE_FIELDS_ONE_PLUS_ONE_FILTERING_ENABLE_0);
            if(targetIsTrunk == 0 && targetIsVPort)
            {
                /* The comparison above in case of vPorts should be done regardless of the value of
                    L2MLL<One plus one enabled>. I.e. lsb is always used for comparison*/
                srcFilterReasonNamePtr = srcFilterReasonNameArr[0];
                ePortMaskForSrcFilter = 0xFFFFFFFF; /* dummy not used */
                if(filterEnable)
                {
                    __LOG(("One Plus One Enabled \n"));

                    /* apply VPort range classification */
                    snetSip7MllVPortsRangeClassification(devObjPtr,descrPtr,
                        cfgBackupValueArr,cfgBackupMaskArr);

                    for(ii = 0 ; ii < 4 ; ii++)
                    {
                        nextDescPtr->genericClassification[ii] =
                            (origDescPtr->genericClassification[ii] & ~cfgBackupMaskArr[ii]) | /* clear 'mask' bits */
                            (cfgBackupValueArr[ii] & cfgBackupMaskArr[ii]);/* add 'value' bits */
                    }
                    if(simLogIsOpenFlag)
                    {
                        __LOG(("apply generic classification \n"));
                        __LOG_PARAM(nextDescPtr->genericClassification[0]);
                        __LOG_PARAM(nextDescPtr->genericClassification[1]);
                        __LOG_PARAM(nextDescPtr->genericClassification[2]);
                        __LOG_PARAM(nextDescPtr->genericClassification[3]);
                    }
                }
            }
            else
            if(filterEnable) /* if not 'vPort' */
            {
                /* When enabled, and <MC Local Switching Enable> is disabled,
                   source filtering ignores the least significant bit of the ePort.
                   The assumption is that the two ePorts in the 1+1 scheme are two
                   contiguous ePort numbers, that differ only in the lsbit.*/

                __LOG(("One Plus One Filtering Enabled --> do source filtering and ignore the least significant bit of the ePort \n"));

                ePortMaskForSrcFilter = 0xFFFFFFFE;/* ignore the LSBit*/
                srcFilterReasonNamePtr = srcFilterReasonNameArr[1];
            }
            else
            {
                ePortMaskForSrcFilter = 0xFFFFFFFF;
                srcFilterReasonNamePtr = srcFilterReasonNameArr[0];
            }

            if(targetIsTrunk == 0 && targetIsVPort)
            {
                if(srcFilter_isVPort &&
                   srcFilter_vPort == targetVPort)
                {
                    __LOG(("%s - filter this Target vPort [%x] \n",
                        srcFilterReasonNamePtr,
                        targetVPort));
                    filter_copy = GT_TRUE;
                    src_filter = GT_TRUE;
                }
            }
            else
            if(targetIsTrunk == 0)
            {   /* target is EPort */
                if(srcFilter_isTrunk == 0 && /* src not from trunk (from DSA/not)*/
                   (srcFilter_portTrunk & ePortMaskForSrcFilter) == (targetEPort & ePortMaskForSrcFilter)) /* src Eport == target EPORT */
                {
                    if(SMEM_CHT_IS_SIP6_GET(devObjPtr))
                    {
                        if(l2mllGlobalEPortEnable == 0)
                        {
                            __LOG(("The EPort[%d] not checked as Global EPort , because feature disabled \n",
                                targetEPort));
                        }
                        else
                        /*(E >= <Global ePort Min Value> && E <= <Global ePort Max Value>)*/
                        if(targetEPort < globalEPortMinValue)
                        {
                            __LOG(("The eport[%d] is below min range of global eports[%d] \n",
                                targetEPort,
                                globalEPortMinValue));
                            __LOG(("the target eport is not considered as global eport \n"));
                        }
                        else
                        if(targetEPort > globalEPortMaxValue)
                        {
                            __LOG(("The eport[%d] is above max range of global eports[%d] \n",
                                targetEPort,
                                globalEPortMaxValue));
                            __LOG(("the target eport is not considered as global eport \n"));
                        }
                        else
                        {
                            __LOG(("The eport[%d] is in range of global eports[%d..%d] \n",
                                targetEPort,
                                globalEPortMinValue,
                                globalEPortMaxValue));

                            __LOG(("%s - filter this Target global ePort [%x] \n",
                                srcFilterReasonNamePtr,
                                targetEPort));
                            filter_copy = GT_TRUE;
                            src_filter = GT_TRUE;
                        }
                    }
                    else
                    {
                        if((targetEPort & globalEPortMask) == globalEPortVal) /*global port not need to check <device> */
                        {
                            __LOG(("%s - filter this Target global ePort [%x] \n",
                                srcFilterReasonNamePtr,
                                targetEPort));
                            filter_copy = GT_TRUE;
                            src_filter = GT_TRUE;
                        }
                    }

                    if(srcFilter_device == targetDevice)/* not global port so need to check <device>*/
                    {
                        __LOG(("%s - filter this Target ePort [%x] in device[%x] \n",
                            srcFilterReasonNamePtr,
                            targetEPort,
                            targetDevice));
                        filter_copy = GT_TRUE;
                        src_filter = GT_TRUE;
                    }
                }
            }
            else /*targetIsTrunk == 1*/
            {
                if(srcFilter_isTrunk == 1 && /* src from trunk (from DSA/not)*/
                   srcFilter_portTrunk == targetTrunkID) /* src trunk == target trunk */
                {
                    __LOG(("%s - filter this Target trunk [%x] \n",
                        srcFilterReasonNamePtr,
                        targetTrunkID));
                    filter_copy = GT_TRUE;
                    src_filter = GT_TRUE;
                }
            }

            if(src_filter == GT_FALSE)
            {
                /* the src filter not filter this replication */
                __LOG(("MC local switching Forbidden (but there was NO SRC filtering match) \n"));
            }
            else
            {
                __LOG(("MC local switching Forbidden (and did SRC filtering ) \n"));
            }
        }
        else
        {
            __LOG(("MC local switching Enabled (so NO SRC filtering check) \n"));
        }

        if (GT_FALSE == filter_copy)
        {
             sendReplication = GT_TRUE;
        }

        /*********************/
        /* start replication */
        /*********************/
        if(descrPtr->packetCmd != SKERNEL_EXT_PKT_CMD_FROM_CPU_E)
        {
            /* the replication must be 'forward' unless orig is 'from_cpu' */
            __LOG(("the replication must be 'forward' unless orig is 'from_cpu' \n"));
            nextDescPtr->packetCmd = SKERNEL_EXT_PKT_CMD_FORWARD_E;
        }
        nextDescPtr->analyzerIndex = 0;/* must be 0 in all replications other than 'last one' */

        /* reset the values that relate to 'target' as
           we are going to set only some of them next */
        nextDescPtr->trgEPort = 0;
        nextDescPtr->trgDev   = 0;
        nextDescPtr->useVidx  = 0;
        nextDescPtr->targetIsTrunk = 0;
        nextDescPtr->trgIsVPort    = 0;

        /* assign egress interface */
        if(targetIsVPort == 1 && /* only in sip7 */
           targetIsTrunk == 0 &&
           targetIsVidx  == 0)
        {
            __LOG(("assign egress interface vPort [%d] \n",
                targetVPort));

            nextDescPtr->trgIsVPort    = 1;
            nextDescPtr->trgVPort      = targetVPort;

            /* called after setting trgVPort */
            SIP7_INVALIDATE_TRG_EPORT_TRG_DEV_MAC(devObjPtr,nextDescPtr,l2mll);
        }
        else
        if(targetIsVPort == 0 &&   /* always ZERO in non-sip7 devices */
           targetIsTrunk == 0 &&
           targetIsVidx  == 0)
        {
            nextDescPtr->trgIsVPort    = 0;

            __LOG(("assign egress interface ePort [%d] Device [%d] \n",
                targetEPort,targetDevice));
            nextDescPtr->trgEPort = targetEPort;
            nextDescPtr->trgDev = targetDevice;

            SNET_E_ARCH_CLEAR_IS_TRG_PHY_PORT_VALID_MAC(devObjPtr,nextDescPtr,l2mll);
        }
        else if (targetIsTrunk == 1)
        {
            __LOG(("assign egress interface TrunkID [%d] \n",
                targetTrunkID));
            nextDescPtr->trgTrunkId = targetTrunkID;
            nextDescPtr->targetIsTrunk = 1;
        }
        else if (targetIsVidx == 1)
        {
            __LOG(("assign egress interface vidx [%d] \n",
                targetVidx));
            nextDescPtr->useVidx = 1;
            /*descrPtr->eVidx = targetVidx; no eVidx modification (should save it for the DSA tagging)*/
            nextDescPtr->eArchExtInfo.vidx = targetVidx;
        }

        value =
            SMEM_LION3_L2_MLL_ENTRY_FIELD_GET(devObjPtr,memPtr,l2MllIndex,
                entrySelector ?
                    SMEM_LION3_L2_MLL_TABLE_FIELDS_MAX_HOP_COUNT_ENABLE_1:
                    SMEM_LION3_L2_MLL_TABLE_FIELDS_MAX_HOP_COUNT_ENABLE_0);
        /*Max Hop Count Enable*/
        if(value)
        {
            value =
                SMEM_LION3_L2_MLL_ENTRY_FIELD_GET(devObjPtr,memPtr,l2MllIndex,
                    entrySelector ?
                        SMEM_LION3_L2_MLL_TABLE_FIELDS_MAX_OUTGOING_HOP_COUNT_1:
                        SMEM_LION3_L2_MLL_TABLE_FIELDS_MAX_OUTGOING_HOP_COUNT_0);
            /*Max Outgoing Hop Count*/
            if(descrPtr->ttl > value)
            {
                nextDescPtr->ttl = value;
            }
        }

        if (GT_TRUE == filter_copy)
        {
            goto nextMllPointer_lbl;
        }
        /*Bind To MLL Counter*/
        value =
            SMEM_LION3_L2_MLL_ENTRY_FIELD_GET(devObjPtr,memPtr,l2MllIndex,
                entrySelector ?
                    SMEM_LION3_L2_MLL_TABLE_FIELDS_BIND_TO_MLL_COUNTER_1:
                    SMEM_LION3_L2_MLL_TABLE_FIELDS_BIND_TO_MLL_COUNTER_0);

        if(value)
        {
            __LOG(("Bind To MLL Counter[%d]",value));

            smemRegGet(devObjPtr, SMEM_LION3_L2_MLL_VALID_PROCESSED_ENTRIES_COUNTER_REG(devObjPtr, value) , &value2);
            value2++;/*value of the counter*/
            smemRegSet(devObjPtr, SMEM_LION3_L2_MLL_VALID_PROCESSED_ENTRIES_COUNTER_REG(devObjPtr, value) , value2);
        }


        if(nextReplicationExists == GT_TRUE)
        {
            if(simLogIsOpenFlag)
            {
                /*restore original values for the 'compare' of descriptors 'old and new'*/
                *descrPtr = *origDescPtr;
            }

            /* because there is at least one more replication ,
               it means that this is not the 'last' replication */
            SIM_LOG_PACKET_DESCR_SAVE

            *descrPtr = *previousDescPtr;

            SIM_LOG_PACKET_DESCR_COMPARE("L2MllEngine replication ('NON last' replication) \n");

            /* do common things just before call to EQ */
            snetChtIngressL2Mll_preCallToEq(devObjPtr, descrPtr);

            /* send the replication down the pipe */
            __LOG(("send the replication to EQ (only after assured that this is 'NOT LAST' replication) \n"));
            snetChtEq(devObjPtr, descrPtr);

            simLogPacketFrameUnitSet(SIM_LOG_FRAME_UNIT_MLL_E);

            /* restore the number of used descriptors in the device */
            devObjPtr->descrFreeIndx = origDescrFreeIndx;

            /* restore values of the descriptor */
            *descrPtr = *origDescPtr;
        }

        /* save the 'next' one info , as 'previous' */
        nextReplicationExists = GT_TRUE;
        *previousDescPtr = *nextDescPtr;

        /*******************/
        /* end replication */
        /*******************/

        didAnyReplication = GT_TRUE;

nextMllPointer_lbl:
        if(sendReplication == GT_FALSE)
        {
            /* count the number of MLLs that not send replications */
            __LOG(("Increment the 'Skip Counter' of the number of MLLs that not send replications \n"));
            smemRegGet(devObjPtr, SMEM_LION3_L2_MLL_EXCEPTION_COUNTER_SKIP_COUNTER_REG(devObjPtr) , &value);
            value++;/*value of the counter*/
            smemRegSet(devObjPtr, SMEM_LION3_L2_MLL_EXCEPTION_COUNTER_SKIP_COUNTER_REG(devObjPtr) , value);
        }

        /*last*/
        last =
            SMEM_LION3_L2_MLL_ENTRY_FIELD_GET(devObjPtr,memPtr,l2MllIndex,
                entrySelector ?
                    SMEM_LION3_L2_MLL_TABLE_FIELDS_LAST_1:
                    SMEM_LION3_L2_MLL_TABLE_FIELDS_LAST_0);

        if(last == 1)
        {
            /* this was the last replication by the L2Mll */
            /*Bobcat3 count the last replication even if its filtered*/
            if ((SMEM_CHT_IS_SIP5_20_GET(devObjPtr)) && (GT_FALSE == sendReplication ) && ( GT_TRUE == didAnyReplication))
            {
                 __LOG(("Update L2_MLL counters for BC3 last replication even that the copy is filtered \n"));
                 snetCht3mllCounters(devObjPtr, nextDescPtr , GT_TRUE/*count L2 MLL (not IP MLL)*/);
            }
            break;
        }

        if(entrySelector == 0)
        {
            /* use now the second part of the entry */
            entrySelector++;
            continue;
        }

        /* get the entry section to use after we jump to new entry */
        /* we can update 'next' entrySelector only after all field already updated
           with 'currnt' entrySelector */
        entrySelector =
            SMEM_LION3_L2_MLL_ENTRY_FIELD_GET(devObjPtr,memPtr,l2MllIndex,
                SMEM_LION3_L2_MLL_TABLE_FIELDS_ENTRY_SELECTOR);

        /* Next MLL Pointer */
        l2MllIndex =
            SMEM_LION3_L2_MLL_ENTRY_FIELD_GET(devObjPtr,memPtr,l2MllIndex,
                    SMEM_LION3_L2_MLL_TABLE_FIELDS_L2_NEXT_MLL_PTR);

    }
    while(1);

    __LOG(("L2 MLL replications Ended \n"));

    __LOG_PARAM(TTL_EXCEPTION_FLAG);

    /* TTL Exception affects only that last copy that is not skipped.*/
    if(TTL_EXCEPTION_FLAG)
    {
        smemRegGet(devObjPtr, SMEM_LION3_L2_MLL_EXCEPTION_COUNTER_TTL_EXCEPTION_COUNTER_REG(devObjPtr) , &value);
        value++;/*value of the counter*/
        smemRegSet(devObjPtr, SMEM_LION3_L2_MLL_EXCEPTION_COUNTER_TTL_EXCEPTION_COUNTER_REG(devObjPtr) , value);
    }

    __LOG_PARAM(didAnyReplication);

    if(didAnyReplication == GT_TRUE)
    {
        /* indication that we not send the 'last' replication !
        and the info is in nextDescPtr / previousDescPtr */

        SIM_LOG_PACKET_DESCR_SAVE

        *descrPtr = *previousDescPtr;

        isLastReplication = 1;

        __LOG_PARAM(origDescPtr->ipm);
        if(origDescPtr->ipm)
        {
            __LOG_PARAM(origDescPtr->routed);
            if(origDescPtr->routed == 1)
            {
                /* this is NOT the last copy of the IP_MLL */
                isLastReplication = 0;
            }
        }

        __LOG_PARAM(isLastReplication);

        if(isLastReplication == 0)
        {
            /* this is NOT the last copy of the IP_MLL , so we NOT allow the mirrorIndex */
            descrPtr->analyzerIndex = 0;
        }
        else
        {
            /* use the original analyzer index */
            descrPtr->analyzerIndex = origDescPtr->analyzerIndex;
        }

        __LOG_PARAM(descrPtr->analyzerIndex);

        /* use the original command */
        descrPtr->packetCmd = origDescPtr->packetCmd;
        __LOG_PARAM(descrPtr->packetCmd);

        /* TTL Exception affects only that last copy that is not skipped.*/
        if(TTL_EXCEPTION_FLAG)
        {
            smemRegGet(devObjPtr, SMEM_LION3_L2_MLL_GLOBAL_TTL_EXCEPTION_CONFIG_REG(devObjPtr) , &value);
            if(SMEM_U32_GET_FIELD(value,8,1))
            {
                /* resolve packet command and CPU code */
                __LOG(("resolve packet command and CPU code"));
                snetChtIngressCommandAndCpuCodeResolution(devObjPtr,descrPtr,
                                                          descrPtr->packetCmd,
                                                          SKERNEL_EXT_PKT_CMD_MIRROR_TO_CPU_E,
                                                          descrPtr->cpuCode,
                                                          SMEM_U32_GET_FIELD(value,0,8),/*CPU code*/
                                                          SNET_CHEETAH_ENGINE_UNIT_L2MLL_E,
                                                          GT_TRUE);
            }
        }

        SIM_LOG_PACKET_DESCR_COMPARE("L2MllEngine replication ('LAST' replication) \n");

        /* do common things just before call to EQ */
        snetChtIngressL2Mll_preCallToEq(devObjPtr, descrPtr);

        /* send the replication down the pipe */
        __LOG(("send the replication to EQ (this is 'LAST' replication) \n"));

        /* restore the number of used descriptors in the device */
        devObjPtr->descrFreeIndx = origDescrFreeIndx;

        /* ... continue to the EQ ... */
        __LOG(("... continue to the EQ ... \n"));
    }
    else
    {
        memPtr = smemMemGet(devObjPtr, SMEM_LION3_L2_MLL_SILENT_DROP_COUNTER_REG(devObjPtr));
        __LOG(("increment l2 mll silent drop Counter from [%d]\n", *memPtr));
        (*memPtr)++;


        /*assigned a SOFT_DROP packet*/
        __LOG(("assigned a SOFT_DROP packet , because not did any replications (use original descriptor) \n"));
        snetChtIngressCommandAndCpuCodeResolution(devObjPtr,descrPtr,
                                                  descrPtr->packetCmd,
                                                  SKERNEL_EXT_PKT_CMD_SOFT_DROP_E,
                                                  descrPtr->cpuCode,
                                                  0,/*don't care*/
                                                  SNET_CHEETAH_ENGINE_UNIT_L2MLL_E,
                                                  GT_TRUE);
    }

    return;
}


/**
* @internal snetChtCpuCodeResolution function
* @endinternal
*
* @brief   resolve from old and new commands the new packet cpu code / DROP code
*
* @param[in] devObjPtr                - (pointer to) device object.
* @param[in,out] descrPtr                 - (pointer to) frame descriptor
* @param[in] prevCmd                  - previous command
* @param[in] currCmd                  - current command
* @param[in] afterResolutionCmd       - the command after the resolution
* @param[in] prevCpuCode              - previous cpu code
* @param[in] currCpuCode              - current cpu code
*/
extern void snetChtCpuCodeResolution
(
    IN SKERNEL_DEVICE_OBJECT                    *devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC       *descrPtr,
    IN SKERNEL_EXT_PACKET_CMD_ENT prevCmd,
    IN SKERNEL_EXT_PACKET_CMD_ENT currCmd,
    IN SKERNEL_EXT_PACKET_CMD_ENT afterResolutionCmd,
    IN GT_U32                prevCpuCode,
    IN GT_U32                currCpuCode
)
{


    GT_BOOL cpuCodeUpdated = GT_FALSE;
    enum _RESOLUTION_COMMAND_ENT{
        PREV_STRONGER_E,
        PREV_WEAKER_E,
        PREV_EQUAL_E,
    };
    GT_U32 resolutionCommand;
    static GT_U32 _resolutionCommands[5/*prev*/][5/*new*/] = {
/*prev*/ /*new*//* {CMD_FORWARD,     CMD_MIRROR,    CMD_TRAP,      CMD_HARD_DROP, CMD_SOFT_DROP} */
/*CMD_FORWARD,  */ {PREV_EQUAL_E,    PREV_WEAKER_E, PREV_WEAKER_E,    PREV_WEAKER_E, PREV_WEAKER_E},
/*CMD_MIRROR,   */ {PREV_STRONGER_E, PREV_EQUAL_E,  PREV_WEAKER_E,    PREV_WEAKER_E, PREV_STRONGER_E},
/*CMD_TRAP,     */ {PREV_STRONGER_E, PREV_STRONGER_E, PREV_EQUAL_E,   PREV_WEAKER_E, PREV_STRONGER_E},
/*CMD_HARD_DROP,*/ {PREV_STRONGER_E, PREV_STRONGER_E, PREV_STRONGER_E,PREV_EQUAL_E,  PREV_STRONGER_E},
/*CMD_SOFT_DROP,*/ {PREV_STRONGER_E, PREV_WEAKER_E, PREV_WEAKER_E,  PREV_WEAKER_E, PREV_EQUAL_E}
    };

    if(currCmd >= SKERNEL_EXT_PKT_CMD_TO_TRG_SNIFFER_E)
    {
        /* CPU code is not changed */
        return;
    }

    if(prevCmd >= SKERNEL_EXT_PKT_CMD_TO_TRG_SNIFFER_E)
    {
        if(currCmd == CMD_HARD_DROP)
        {
            /* update the CPU/DROP code */
            descrPtr->cpuCode = currCpuCode;

            if(simLogIsOpenFlag)
            {
                scibAccessLock();

                __LOG_NO_LOCATION_META_DATA(("prevCpuCode:"));
                simLogPacketDescrCpuCodeDump(devObjPtr,prevCpuCode);
                __LOG_NO_LOCATION_META_DATA(("\n"));

                __LOG_NO_LOCATION_META_DATA(("descrPtr->cpuCode:"));
                simLogPacketDescrCpuCodeDump(devObjPtr,descrPtr->cpuCode);
                __LOG_NO_LOCATION_META_DATA(("\n"));

                scibAccessUnlock();
            }
        }
        else
        {
           /* Don't change CPU code */
        }

        return;

    }

    if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        /* the device support CPU code for any non-forward command */
        if(afterResolutionCmd != SKERNEL_EXT_PKT_CMD_FORWARD_E)
        {
            resolutionCommand = _resolutionCommands[prevCmd][currCmd];

            /* If the previous command is 'stronger' than the current command:
               The CPU/Drop code is unchanged.*/
            if(resolutionCommand != PREV_STRONGER_E)
            {
                /* If the previous command is equal to the current command:
                   If the resulting command is not FORWARD, the CPU/Drop code is
                   changed to reflect the value of the current assignment.*/

                /*If the previous command is 'weaker' than the current command:
                 The CPU/Drop code is changed to reflect the value of the
                 current assignment.*/
                cpuCodeUpdated = GT_TRUE;
            }
        }
    }
    else if(afterResolutionCmd == SKERNEL_EXT_PKT_CMD_TRAP_TO_CPU_E ||
            afterResolutionCmd == SKERNEL_EXT_PKT_CMD_MIRROR_TO_CPU_E)
    {
        /* the device support CPU codes only for 'trap'/'mirror' */
        if(currCmd == SKERNEL_EXT_PKT_CMD_TRAP_TO_CPU_E)
        {
            cpuCodeUpdated = GT_TRUE;
        }
        else if(prevCmd == SKERNEL_EXT_PKT_CMD_TRAP_TO_CPU_E)
        {
        }
        else if(currCmd == SKERNEL_EXT_PKT_CMD_MIRROR_TO_CPU_E)
        {
            cpuCodeUpdated = GT_TRUE;
        }
    }

    if(currCpuCode > 0xFF)
    {
        /* the 'cpu code' is 8 bits in the register but some places in the logic
           there are '+' on the value read from the register.
           the simulation must recognize when value > 255 and notify into the LOG about it.
           anyway it should do cpuCode = (cpuCode & 0xff) */


        /* for example see: snetLion2TrillGeneralExceptionChecking , snetLion2TrillRbidTable
            snetLion3TTActionApplyMplsCwBasedPw */

        __LOG_NO_LOCATION_META_DATA__WITH_SCIB_LOCK(("WARNING: The CPU code [0x%x] > 0xFF !!! meaning that logic of '+' added it to be value greater then 0xFF \n",
            currCpuCode));

        currCpuCode &= 0xFF;

        __LOG_NO_LOCATION_META_DATA__WITH_SCIB_LOCK(("The CPU code will be trimmed to 8 bits --> [0x%x] \n",
            currCpuCode));
    }

    if(cpuCodeUpdated == GT_FALSE || descrPtr->cpuCode == currCpuCode)
    {
        /*__LOG(("CPU code not changed \n"));*/
        return;
    }

    /* update the CPU code */
    descrPtr->cpuCode = currCpuCode;

    if(simLogIsOpenFlag)
    {
        scibAccessLock();

        __LOG_NO_LOCATION_META_DATA(("prevCpuCode:"));
        simLogPacketDescrCpuCodeDump(devObjPtr,prevCpuCode);
        __LOG_NO_LOCATION_META_DATA(("\n"));

        __LOG_NO_LOCATION_META_DATA(("descrPtr->cpuCode:"));
        simLogPacketDescrCpuCodeDump(devObjPtr,descrPtr->cpuCode);
        __LOG_NO_LOCATION_META_DATA(("\n"));

        scibAccessUnlock();
    }
}


/**
* @internal snetChtIngressCommandAndCpuCodeResolution function
* @endinternal
*
* @brief   1. resolve from old and current commands the new packet command
*         2. resolve from old and new commands the new packet cpu code (relevant
*         to copy that goes to CPU)
* @param[in] devObjPtr                - (pointer to) device object.
* @param[in,out] descrPtr                 - (pointer to) frame descriptor
* @param[in] prevCmd                  - previous command
* @param[in] currCmd                  - current command
* @param[in] prevCpuCode              - previous cpu code
* @param[in] currCpuCode              - current cpu code
* @param[in] engineUnit               - the engine unit that need resolution with previous engine
*                                       (isFirst = GT_TRUE) or need resolution with previous
*                                       hit inside the same engine (isFirst = GT_FALSE)
* @param[in] isFirst                  - indication that the resolution is within the same engine or
*                                       with previous engine
* @param[in,out] descrPtr            - (pointer to) frame descriptor
*/
extern void snetChtIngressCommandAndCpuCodeResolution
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC   * descrPtr,
    IN SKERNEL_EXT_PACKET_CMD_ENT prevCmd,
    IN SKERNEL_EXT_PACKET_CMD_ENT currCmd,
    IN GT_U32                     prevCpuCode,
    IN GT_U32                     currCpuCode,
    IN SNET_CHEETAH_ENGINE_UNIT_ENT engineUnit,
    IN GT_BOOL                    isFirst
)
{
    DECLARE_FUNC_NAME(snetChtIngressCommandAndCpuCodeResolution);

    SKERNEL_PRECEDENCE_ORDER_ENT  cpuCodePrecedence = SKERNEL_PRECEDENCE_ORDER_HARD;/*CPU Code Precedence */
    GT_BOOL usePrecedence = GT_FALSE;
    GT_U32  value;/* tempo register value */
    GT_BOOL keepPreviousCpuCode;/* do we keep previous CPU code */

    if(engineUnit == SNET_CHEETAH_ENGINE_UNIT_PCL_E)
    {
        /* Policy Global Configuration Register */
        smemRegFldGet(devObjPtr,SMEM_XCAT_A1_TTI_UNIT_GLB_CONF_REG(devObjPtr), 6, 1, &value);

        __LOG(("PCL cpuCodePrecedence [%s] \n" , (value == 0) ? "SKERNEL_PRECEDENCE_ORDER_HARD" : "SKERNEL_PRECEDENCE_ORDER_SOFT"));
        if(value == 0)
        {
            cpuCodePrecedence = SKERNEL_PRECEDENCE_ORDER_HARD;
        }
        else
        {
            cpuCodePrecedence = SKERNEL_PRECEDENCE_ORDER_SOFT;
        }

        usePrecedence = GT_TRUE;
    }

    /* when we use precedence , this is actually the precedence of
       previous inter-engine match */
    if((usePrecedence == GT_TRUE) &&
       (currCmd == prevCmd) &&
       (cpuCodePrecedence == SKERNEL_PRECEDENCE_ORDER_HARD) &&
       (isFirst == GT_FALSE))
    {
        keepPreviousCpuCode = GT_TRUE;
    }
    else
    {
        keepPreviousCpuCode = GT_FALSE;
    }

    /* Apply packet command */
    descrPtr->packetCmd = snetChtPktCmdResolution(prevCmd,currCmd);

    if(keepPreviousCpuCode == GT_FALSE)
    {
        /* allow to do resolution on the CPU/DROP code */
        snetChtCpuCodeResolution(devObjPtr,descrPtr,
            prevCmd,currCmd,descrPtr->packetCmd,
            prevCpuCode,currCpuCode);
    }

}

/**
* @internal snetChtClassifyFlowControlPacket function
* @endinternal
*
* @brief   Flow Control Packet Recognition
*/
static SNET_FLOW_CONTROL_PACKET_TYPE_ENT snetChtClassifyFlowControlPacket
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr
)
{
    DECLARE_FUNC_NAME(snetChtClassifyFlowControlPacket);

    GT_U32 regAddr;                     /* Register address */
    GT_U32 * regPtr;                    /* Register pointer */
    GT_U32 fldValue;                    /* Register field value */
    SGT_MAC_ADDR_TYP portMacAddr;       /* Ports configured MAC Address */
    GT_U32 etherType;                   /* Flow Control Ethernet type */
    GT_U32 opCode;                      /* Flow Control OpCode */
    GT_BOOL isFcMac;                    /* Packet's MAC DA is 01-80-C2-00-00-01 or the port's configured MAC Address */

    /* Flow Control Ethernet type */
    etherType = SNET_GET_PCKT_TAG_ETHER_TYPE_MAC(descrPtr, 12);
    __LOG_PARAM(etherType);
    if(etherType != SNET_CHT_FC_ETHERTYPE)
    {
        if (SGT_MAC_ADDR_IS_PAUSE(descrPtr->macDaPtr))
        {
            /* even if etherType not match, the 'DA mac is enough' to classify it as 'pause frame' =>  ch3 HW behavior */
            __LOG(("recognized 'flow control packet' according to the MAC DA , even though ethertype is not [0x%x] \n",SNET_CHT_FC_ETHERTYPE));
            return SNET_FC_PACKET_E;
        }

        __LOG(("pack is not 'flow control' \n"));
        return SNET_NOT_FC_PACKET_E;
    }
/*xxx*/
    regAddr = SMEM_CHT_SRC_ADDRESS_MID_REG(devObjPtr);
    regPtr = smemMemGet(devObjPtr, regAddr);

    portMacAddr.bytes[4] = SMEM_U32_GET_FIELD(regPtr[0], 0, 8);
    portMacAddr.bytes[3] = SMEM_U32_GET_FIELD(regPtr[1], 0, 8);
    portMacAddr.bytes[2] = SMEM_U32_GET_FIELD(regPtr[1], 8, 8);
    portMacAddr.bytes[1] = SMEM_U32_GET_FIELD(regPtr[1], 16, 8);
    portMacAddr.bytes[0] = SMEM_U32_GET_FIELD(regPtr[1], 24, 8);

    if(descrPtr->ingressGopPortNumber >= SKERNEL_DEV_MAX_SUPPORTED_PORTS_CNS)
    {
        skernelFatalError("snetChtClassifyFlowControlPacket: invalid MAC number [%d] \n",
            descrPtr->ingressGopPortNumber);
    }

    if(CHT_IS_MTI_MAC_USED_MAC(devObjPtr,descrPtr->ingressGopPortNumber))
    {
        fldValue = snetChtPortMacFieldGet(devObjPtr, descrPtr->ingressGopPortNumber,
            SNET_CHT_PORT_MAC_FIELDS_mti_mac_addr_0_E);

        fldValue &= 0xFF;/* using only 8 bits */
    }
    else
    if(IS_CHT_HYPER_GIGA_PORT(devObjPtr,descrPtr->ingressGopPortNumber))
    {
        /* XG port */
        regAddr = SMEM_CHT_MAC_CONTROL2_REG(devObjPtr, descrPtr->ingressGopPortNumber);
        regPtr = smemMemGet(devObjPtr, regAddr);

        fldValue = SMEM_U32_GET_FIELD(regPtr[0], 0, 8);
    }
    else
    {
        /* Gig port */
        regAddr = SMEM_CHT_MAC_CONTROL_REG(devObjPtr, descrPtr->ingressGopPortNumber);
        regPtr = smemMemGet(devObjPtr, regAddr);

        fldValue = SMEM_U32_GET_FIELD(regPtr[0], 7, 8);
    }

    portMacAddr.bytes[5] = fldValue;

    /* Packet's MAC DA is 01-80-C2-00-00-01 or the port's configured MAC Address */
    isFcMac = ( (SGT_MAC_ADDR_IS_PAUSE(descrPtr->macDaPtr)) ||
                (SGT_MAC_ADDR_ARE_EQUAL(descrPtr->macDaPtr, portMacAddr.bytes)) );

    if (isFcMac)
    {
        /* Flow Control OpCode */
        opCode = (descrPtr->startFramePtr[14] << 8) | descrPtr->startFramePtr[15];
        __LOG_PARAM_WITH_NAME("Flow Control OpCode : ",opCode);
        if (opCode == SNET_CHT_FC_OPCODE)
        {
            __LOG(("recognized Flow control packet (FC) \n"));
            return SNET_FC_PACKET_E;
        }
        if (opCode == SNET_CHT_PFC_OPCODE)
        {
            __LOG(("recognized Priority Flow control packet (PFC) \n"));
            return SNET_PFC_PACKET_E;
        }
    }

    __LOG(("unknown Flow control packet (ethertype of 'FC' but mac DA not match standard and not match 'the FC mac of port') \n"));
    return SNET_UNKNOWN_FC_PACKET_E;
}

/**
* @internal snetChtDoForwardFcFrame function
* @endinternal
*
* @brief   Forward (to ingress pipe) or terminate Flow Control frame.
*         Some of FC packet types must be terminated, depend on configuration
*         Not FC packets are always forwarded
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - frame data buffer Id
*
* @param[out] fcPacketTypePtr          - (pointer to) FC packet type classification.
*                                      RETURN:
*                                      GT_TRUE - continue packet processing
*                                      GT_FALSE - terminate packet processing
*/
static GT_BOOL snetChtDoForwardFcFrame
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    OUT SNET_FLOW_CONTROL_PACKET_TYPE_ENT   *fcPacketTypePtr
)
{
    GT_U32      fldValue;
    GT_BOOL     forward;    /* GT_TRUE - continue pkt processing;  GT_FALSE - terminate */

    *fcPacketTypePtr = snetChtClassifyFlowControlPacket(devObjPtr, descrPtr);

    if(descrPtr->ingressGopPortNumber >= SKERNEL_DEV_MAX_SUPPORTED_PORTS_CNS)
    {
        skernelFatalError("snetChtDoForwardFcFrame: invalid MAC number [%d] \n",
            descrPtr->ingressGopPortNumber);
    }

    switch (*fcPacketTypePtr)
    {
        case SNET_FC_PACKET_E:
            forward = GT_FALSE;
            if (devObjPtr->supportForwardFcPackets)
            {
                fldValue = snetChtPortMacFieldGet(devObjPtr, descrPtr->ingressGopPortNumber,
                    SNET_CHT_PORT_MAC_FIELDS_forwardFcPacketsEnable_E);
                /* forwardFcEnable */
                forward = fldValue ? GT_TRUE : GT_FALSE;
            }
            break;
        case SNET_UNKNOWN_FC_PACKET_E:
            forward = GT_TRUE; /* ch3 forward such packets */
            if (devObjPtr->supportForwardUnknowMacControlFrames &&
                (descrPtr->ingressGopPortNumber != SNET_CHT_CPU_PORT_CNS))
            {
                fldValue = snetChtPortMacFieldGet(devObjPtr, descrPtr->ingressGopPortNumber,
                    SNET_CHT_PORT_MAC_FIELDS_forwardUnknownMacControlFramesEnable_E);
                /* forwardUnknownMacControlFramesEnable */
                forward = fldValue ? GT_TRUE : GT_FALSE;
            }
            break;
        case SNET_PFC_PACKET_E:
            forward = GT_TRUE; /* ch3 forward such packets */
            if (devObjPtr->supportForwardPfcFrames &&
                (descrPtr->ingressGopPortNumber != SNET_CHT_CPU_PORT_CNS))
            {
                fldValue = snetChtPortMacFieldGet(devObjPtr, descrPtr->ingressGopPortNumber,
                    SNET_CHT_PORT_MAC_FIELDS_forwardPfcFramesEnable_E);
                /* forwardPfcFramesEnable */
                forward = fldValue ? GT_TRUE : GT_FALSE;
            }
            break;
        default:
            forward = GT_TRUE;  /* forward non Flow-control packets */
            break;
    }
    return forward;
}

/**
* @internal parsingTrillInnerFrame function
* @endinternal
*
* @brief   L2,3,QOS Parsing for 'Inner frame' of TRILL. need to be done regardless
*         to tunnel termination.
*/
static GT_VOID parsingTrillInnerFrame
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN INTERNAL_TTI_DESC_INFO_STC        * internalTtiInfoPtr
)
{
    DECLARE_FUNC_NAME(parsingTrillInnerFrame);

    /* update the length of the inner packet */
    descrPtr->byteCount -=
        (descrPtr->ingressTunnelInfo.innerMacDaPtr - /* start of the inner packet */
         descrPtr->startFramePtr);           /* start of the outer packet */

    /* update the start of the inner packet */
    descrPtr->startFramePtr = descrPtr->ingressTunnelInfo.innerMacDaPtr;
    /* reset the L3 pointer , because it is not relevant any more -->
               will be set inside snetChtL2Parsing(...) for the inner packet */
    descrPtr->l3StartOffsetPtr = NULL;
    descrPtr->macDaPtr = descrPtr->startFramePtr;
    descrPtr->macSaPtr = descrPtr->startFramePtr + SGT_MAC_ADDR_BYTES;

    /* Fill MAC data type of descriptor */
    __LOG(("Fill MAC data type of descriptor"));
    if (SGT_MAC_ADDR_IS_MCST(descrPtr->macDaPtr))
    {
        if (SGT_MAC_ADDR_IS_BCST(descrPtr->macDaPtr))
        {
            descrPtr->macDaType = SKERNEL_BROADCAST_MAC_E;
        }
        else
        {
            descrPtr->macDaType = SKERNEL_MULTICAST_MAC_E;
        }
    }
    else
    {
        descrPtr->macDaType = SKERNEL_UNICAST_MAC_E;
    }

    /* do L2 parsing (vlan tag , ethertype , nested vlan , encapsulation) */
    snetChtL2Parsing(devObjPtr, descrPtr,SNET_CHT_FRAME_PARSE_MODE_TRILL_E,internalTtiInfoPtr);

    if(descrPtr->l3StartOffsetPtr == NULL)
    {
        /* function snetChtL2Parsing should set descrPtr->l3StartOffsetPtr */

        /* fix for coverity warning "FORWARD_NULL" */
        skernelFatalError("parsingTrillInnerFrame: invalid table entry pointer\n");
    }

    /* L3, L4 protocols parsing */
    __LOG(("L3, L4 protocols parsing"));
    snetChtL3L4ProtParsing(devObjPtr, descrPtr, descrPtr->etherTypeOrSsapDsap, internalTtiInfoPtr);

    /* VLAN and QoS Profile Assign */
    __LOG(("VLAN and QoS Profile Assign"));
    snetChtVlanQosProfAssign(devObjPtr, descrPtr,
                             descrPtr->etherTypeOrSsapDsap,
                             descrPtr->l2Encaps,
                             NULL/*portVlanCfgEntryPtr*/);
}

/**
* @internal snetChtParsingTrillInnerFrame function
* @endinternal
*
* @brief   L2,3,QOS Parsing for 'Inner frame' of TRILL. need to be done regardless
*         to tunnel termination.
*         the function will save the 'parsing descriptor' in descrPtr->ingressTunnelInfo.innerFrameDescrPtr
*/
extern GT_VOID snetChtParsingTrillInnerFrame
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN INTERNAL_TTI_DESC_INFO_STC        * internalTtiInfoPtr
)
{
    DECLARE_FUNC_NAME(snetChtParsingTrillInnerFrame);

    if(descrPtr->ingressTunnelInfo.innerPacketL2FieldsAreValid == 0)
    {
        /* can't parse the inner packet , because the parser can't reach all the
          fields due to long offset from the start of the packet */
        return;
    }

    /* duplicate the inner packet descriptor from the 'original' */
    __LOG(("duplicate the inner packet descriptor from the 'original'"));
    descrPtr->ingressTunnelInfo.innerFrameDescrPtr = snetChtEqDuplicateDescr(devObjPtr,descrPtr);

    /* start using the 'Inner frame' descriptor --> do parsing of the inner frame */
    __LOG(("start using the 'Inner frame' descriptor --> do parsing of the inner frame"));
    parsingTrillInnerFrame(devObjPtr,descrPtr->ingressTunnelInfo.innerFrameDescrPtr,internalTtiInfoPtr);
}

/**
* @internal snetChtTimestampTagAndTimestampInfo function
* @endinternal
*
* @brief   Save TST and timestamp information.
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the descriptor of the original frame (outer frame)
* @param[in] tstStartOffset           -  offset to the start of the TST
*/
static GT_VOID snetChtTimestampTagAndTimestampInfo(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32 tstStartOffset
)
{
    GT_U32  internalOffset;

    if( (descrPtr->timestampTagged[SMAIN_DIRECTION_INGRESS_E] ==
         SKERNEL_TIMESTAMP_TAG_TYPE_NON_EXTENDED_E) ||
        (descrPtr->timestampTagged[SMAIN_DIRECTION_INGRESS_E] ==
         SKERNEL_TIMESTAMP_TAG_TYPE_EXTENDED_E) )
    {
        descrPtr->timestampTagInfo[SMAIN_DIRECTION_INGRESS_E].U =
            (descrPtr->startFramePtr[tstStartOffset + 2] & 0x01);

        descrPtr->timestampTagInfo[SMAIN_DIRECTION_INGRESS_E].T =
            (descrPtr->startFramePtr[tstStartOffset + 2] & 0x02);

        descrPtr->timestampTagInfo[SMAIN_DIRECTION_INGRESS_E].OffsetProfile =
            (descrPtr->startFramePtr[tstStartOffset + 3] & 0x7F);

        descrPtr->timestampTagInfo[SMAIN_DIRECTION_INGRESS_E].OE =
            (descrPtr->startFramePtr[tstStartOffset + 3] & 0x80);

        internalOffset = 4;
        descrPtr->timestampTagInfo[SMAIN_DIRECTION_INGRESS_E].timestamp.nanoSecondTimer =
                (descrPtr->startFramePtr[tstStartOffset + internalOffset + 0] << 24) |
                (descrPtr->startFramePtr[tstStartOffset + internalOffset + 1] << 16) |
                (descrPtr->startFramePtr[tstStartOffset + internalOffset + 2] <<  8) |
                (descrPtr->startFramePtr[tstStartOffset + internalOffset + 3] <<  0) ;

        if( descrPtr->timestampTagged[SMAIN_DIRECTION_INGRESS_E] ==
            SKERNEL_TIMESTAMP_TAG_TYPE_EXTENDED_E )
        {
            internalOffset = 8;

            descrPtr->timestampTagInfo[SMAIN_DIRECTION_INGRESS_E].timestamp.secondTimer.l[1] =
                (descrPtr->startFramePtr[tstStartOffset + internalOffset + 0] <<  8) |
                (descrPtr->startFramePtr[tstStartOffset + internalOffset + 1] <<  0) ;

            descrPtr->timestampTagInfo[SMAIN_DIRECTION_INGRESS_E].timestamp.secondTimer.l[0] =
                (descrPtr->startFramePtr[tstStartOffset + internalOffset + 2] << 24) |
                (descrPtr->startFramePtr[tstStartOffset + internalOffset + 3] << 16) |
                (descrPtr->startFramePtr[tstStartOffset + internalOffset + 4] <<  8) |
                (descrPtr->startFramePtr[tstStartOffset + internalOffset + 5] <<  0) ;
        }
    }
    else if ( descrPtr->timestampTagged[SMAIN_DIRECTION_INGRESS_E] ==
              SKERNEL_TIMESTAMP_TAG_TYPE_HYBRID_E)
    {
        internalOffset = 2;

        descrPtr->timestampTagInfo[SMAIN_DIRECTION_INGRESS_E].timestamp.secondTimer.l[1] =
            (descrPtr->startFramePtr[tstStartOffset + internalOffset + 0] <<  8) |
            (descrPtr->startFramePtr[tstStartOffset + internalOffset + 1] <<  0) ;
        descrPtr->timestampTagInfo[SMAIN_DIRECTION_INGRESS_E].timestamp.secondTimer.l[0] =
            (descrPtr->startFramePtr[tstStartOffset + internalOffset + 2] << 24) |
            (descrPtr->startFramePtr[tstStartOffset + internalOffset + 3] << 16) |
            (descrPtr->startFramePtr[tstStartOffset + internalOffset + 4] <<  8) |
            (descrPtr->startFramePtr[tstStartOffset + internalOffset + 5] <<  0) ;
    }

    descrPtr->timestampTagInfo[SMAIN_DIRECTION_INGRESS_E].timestamp.fractionalNanoSecondTimer = 0;
}

/**
* @internal snetChtTimestampTagDetection function
* @endinternal
*
* @brief   Identify the timestamp tag in the incoming packet.
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the descriptor of the original frame (outer frame)
* @param[in,out] ethTypeOffsetPtr     -  pointer to start of the timestamp tag (if exists)
*
*/
static GT_VOID snetChtTimestampTagDetection(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    INOUT GT_U32 * ethTypeOffsetPtr
)
{
    DECLARE_FUNC_NAME(snetChtTimestampTagDetection);

    GT_U32 etherType;              /* real frame ethernet type */
    GT_U32 regAddress;             /* register addres  */
    GT_U32 *memoryPtr;             /* pointer to memory */
    GT_U32 tstEtherType;           /* ethertype of the Timestamp Tag */
    GT_U32 hybridTstEtherType;     /* ethertype of the Hybrid Timestamp Tag */
    GT_U32 tstStartOffset;         /* offset to the start of the TST is exists */
    GT_U8  *packetEthTypePtr = &descrPtr->startFramePtr[*ethTypeOffsetPtr];

    /* Real ethernet type */
    etherType =
        SNET_GET_PCKT_TAG_ETHER_TYPE_MAC(descrPtr, *ethTypeOffsetPtr);
    __LOG_PARAM(etherType);

    regAddress = SMEM_LION3_TTI_PTP_TS_ETHERTYPES_REG(devObjPtr);
    memoryPtr = smemMemGet(devObjPtr, regAddress);
    tstEtherType = SMEM_U32_GET_FIELD(*memoryPtr, 0,  16);
    hybridTstEtherType = SMEM_U32_GET_FIELD(*memoryPtr, 16, 16);

    regAddress = SMEM_LION3_TTI_PTP_TS_CONFIG_REG(devObjPtr);
    memoryPtr = smemMemGet(devObjPtr, regAddress);

    tstStartOffset = *ethTypeOffsetPtr;

    __LOG(("ingress : check for TS Tag \n"));

    if( (SMEM_U32_GET_FIELD(*memoryPtr, 0, 1)) && (etherType == tstEtherType) )
    {
        /* TS Tag Parsing */
        if( (descrPtr->startFramePtr[*ethTypeOffsetPtr + 2] & 0x80) == 0 )
        {
            __LOG(("ingress : TS Tag Parsing recognized 'non exteneded' (8 bytes) [%02x %02x %02x %02x %02x %02x %02x %02x]\n",
                packetEthTypePtr[0],
                packetEthTypePtr[1],
                packetEthTypePtr[2],
                packetEthTypePtr[3],
                packetEthTypePtr[4],
                packetEthTypePtr[5],
                packetEthTypePtr[6],
                packetEthTypePtr[7]
                ));

            descrPtr->timestampTagged[SMAIN_DIRECTION_INGRESS_E] =
                SKERNEL_TIMESTAMP_TAG_TYPE_NON_EXTENDED_E;
            descrPtr->ingressTimestampTagPtr = &descrPtr->startFramePtr[*ethTypeOffsetPtr];
            descrPtr->ingressTimestampTagSize = 8;
            *ethTypeOffsetPtr += 8;

        }
        else
        {
            __LOG(("ingress : TS Tag Parsing recognized 'exteneded' (16 bytes) [%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x]\n",
                packetEthTypePtr[0],
                packetEthTypePtr[1],
                packetEthTypePtr[2],
                packetEthTypePtr[3],
                packetEthTypePtr[4],
                packetEthTypePtr[5],
                packetEthTypePtr[6],
                packetEthTypePtr[7],
                packetEthTypePtr[8],
                packetEthTypePtr[9],
                packetEthTypePtr[10],
                packetEthTypePtr[11],
                packetEthTypePtr[12],
                packetEthTypePtr[13],
                packetEthTypePtr[14],
                packetEthTypePtr[15]
                ));

            descrPtr->timestampTagged[SMAIN_DIRECTION_INGRESS_E] =
                SKERNEL_TIMESTAMP_TAG_TYPE_EXTENDED_E;
            descrPtr->ingressTimestampTagPtr = &descrPtr->startFramePtr[*ethTypeOffsetPtr];
            descrPtr->ingressTimestampTagSize = 16;
            *ethTypeOffsetPtr += 16;

        }
    }
    else if( (SMEM_U32_GET_FIELD(*memoryPtr, 1, 1)) && (etherType == hybridTstEtherType) )
    {
            __LOG(("ingress : Hybrid TS Tag Parsing recognized (8 bytes) [%02x %02x %02x %02x %02x %02x %02x %02x]\n",
                packetEthTypePtr[0],
                packetEthTypePtr[1],
                packetEthTypePtr[2],
                packetEthTypePtr[3],
                packetEthTypePtr[4],
                packetEthTypePtr[5],
                packetEthTypePtr[6],
                packetEthTypePtr[7]
                ));
        /* Hybrid TST Parsing */
        descrPtr->timestampTagged[SMAIN_DIRECTION_INGRESS_E] =
            SKERNEL_TIMESTAMP_TAG_TYPE_HYBRID_E;
        descrPtr->ingressTimestampTagPtr = &descrPtr->startFramePtr[*ethTypeOffsetPtr];
        descrPtr->ingressTimestampTagSize = 8;
        *ethTypeOffsetPtr += 8;
    }
    else
    {
        __LOG(("ingress : TS Tag was not recognized \n"));
        descrPtr->timestampTagged[SMAIN_DIRECTION_INGRESS_E] =
            SKERNEL_TIMESTAMP_TAG_TYPE_UNTAGGED_E;
        descrPtr->ingressTimestampTagPtr = NULL;
        descrPtr->ingressTimestampTagSize = 0;
    }

    snetChtTimestampTagAndTimestampInfo(devObjPtr, descrPtr, tstStartOffset);
}

/**
* @internal localTablesFormatInit function
* @endinternal
*
* @brief   init the format of local tables.
*
* @param[in] devObjPtr                - pointer to device object.
*/
static void localTablesFormatInit(
    IN SKERNEL_DEVICE_OBJECT            * devObjPtr
)
{

    if(SMEM_CHT_IS_SIP6_10_GET(devObjPtr))
    {
        LION3_TABLES_FORMAT_INIT_MAC(
            devObjPtr, SKERNEL_TABLE_FORMAT_TTI_PHYSICAL_PORT_ATTRIBUTE_E,
            sip6_10TtiPhysicalPortTableFieldsFormat, sip6_10TtiPhysicalPortTableFieldsNames);
    }
    else
    if(SMEM_CHT_IS_SIP6_GET(devObjPtr))
    {
        LION3_TABLES_FORMAT_INIT_MAC(
            devObjPtr, SKERNEL_TABLE_FORMAT_TTI_PHYSICAL_PORT_ATTRIBUTE_E,
            sip6TtiPhysicalPortTableFieldsFormat, falconTtiPhysicalPortTableFieldsNames);
    }
    else
    if(SMEM_CHT_IS_SIP5_20_GET(devObjPtr))
    {
        LION3_TABLES_FORMAT_INIT_MAC(
            devObjPtr, SKERNEL_TABLE_FORMAT_TTI_PHYSICAL_PORT_ATTRIBUTE_E,
            sip5_20TtiPhysicalPortTableFieldsFormat, bobcat2B0TtiPhysicalPortTableFieldsNames);
    }
    else
    if(SMEM_CHT_IS_SIP5_10_GET(devObjPtr))
    {
        LION3_TABLES_FORMAT_INIT_MAC(
            devObjPtr, SKERNEL_TABLE_FORMAT_TTI_PHYSICAL_PORT_ATTRIBUTE_E,
            bobcat2B0TtiPhysicalPortTableFieldsFormat, bobcat2B0TtiPhysicalPortTableFieldsNames);
    }
    else
    {
        LION3_TABLES_FORMAT_INIT_MAC(
            devObjPtr, SKERNEL_TABLE_FORMAT_TTI_PHYSICAL_PORT_ATTRIBUTE_E,
            lion3TtiPhysicalPortTableFieldsFormat, lion3TtiPhysicalPortFieldsTableNames);
    }

    if (SMEM_CHT_IS_SIP7_GET(devObjPtr))
    {
        LION3_TABLES_FORMAT_INIT_MAC(
            devObjPtr, SKERNEL_TABLE_FORMAT_TTI_DEFAULT_EPORT_E,
            sip7TtiDefaultEPortTableFieldsFormat, lion3TtiDefaultEPortFieldsTableNames);
    }
    else
    {
        LION3_TABLES_FORMAT_INIT_MAC(
            devObjPtr, SKERNEL_TABLE_FORMAT_TTI_DEFAULT_EPORT_E,
            lion3TtiDefaultEPortTableFieldsFormat, lion3TtiDefaultEPortFieldsTableNames);
    }

    LION3_TABLES_FORMAT_INIT_MAC(
        devObjPtr, SKERNEL_TABLE_FORMAT_VLAN_PORT_PROTOCOL_E,
        lion3TtiDefaultPortProtocolEvlanAndQosConfigurationTableFieldsFormat, lion3TtiDefaultPortProtocolEvlanAndQosConfigurationFieldsTableNames);

    if (SMEM_CHT_IS_SIP7_GET(devObjPtr))
    {
        LION3_TABLES_FORMAT_INIT_MAC(
            devObjPtr, SKERNEL_TABLE_FORMAT_TTI_EPORT_ATTRIBUTES_E,
            sip7TtiEPortAttributesTableFieldsFormat, lion3TtiEPortAttributesFieldsTableNames);
    }
    else
    {
        LION3_TABLES_FORMAT_INIT_MAC(
            devObjPtr, SKERNEL_TABLE_FORMAT_TTI_EPORT_ATTRIBUTES_E,
            lion3TtiEPortAttributesTableFieldsFormat, lion3TtiEPortAttributesFieldsTableNames);
    }

    if(SMEM_CHT_IS_SIP7_GET(devObjPtr))
    {
        LION3_TABLES_FORMAT_INIT_MAC(devObjPtr, SKERNEL_TABLE_FORMAT_L2_MLL_E,
            sip7L2MllTableFieldsFormat, lion3L2MllFieldsTableNames);

        LION3_TABLES_FORMAT_INIT_MAC(devObjPtr, SKERNEL_TABLE_FORMAT_IP_MLL_E,
            sip7IpMllTableFieldsFormat, lion3IpMllFieldsTableNames);

        snetAasMllTablesFormatInit(devObjPtr);
    }
    else
    if(SMEM_CHT_IS_SIP5_20_GET(devObjPtr))
    {
        if(SMEM_CHT_IS_SIP6_10_GET(devObjPtr))
        {
            LION3_TABLES_FORMAT_INIT_MAC(devObjPtr, SKERNEL_TABLE_FORMAT_L2_MLL_E,
                sip6_10L2MllTableFieldsFormat, lion3L2MllFieldsTableNames);
        }
        else
        {
            LION3_TABLES_FORMAT_INIT_MAC(devObjPtr, SKERNEL_TABLE_FORMAT_L2_MLL_E,
                sip5_20L2MllTableFieldsFormat, lion3L2MllFieldsTableNames);
        }

        LION3_TABLES_FORMAT_INIT_MAC(
            devObjPtr, SKERNEL_TABLE_FORMAT_IP_MLL_E,
            sip5_20IpMllTableFieldsFormat, lion3IpMllFieldsTableNames);
    }
    else
    {
        LION3_TABLES_FORMAT_INIT_MAC(devObjPtr, SKERNEL_TABLE_FORMAT_L2_MLL_E,
            lion3L2MllTableFieldsFormat, lion3L2MllFieldsTableNames);

        LION3_TABLES_FORMAT_INIT_MAC(
            devObjPtr, SKERNEL_TABLE_FORMAT_IP_MLL_E,
            lion3IpMllTableFieldsFormat, lion3IpMllFieldsTableNames);
    }


    if(SMEM_CHT_IS_SIP5_20_GET(devObjPtr))
    {
        LION3_TABLES_FORMAT_INIT_MAC(
            devObjPtr, SKERNEL_TABLE_FORMAT_TTI_PHYSICAL_PORT_2_ATTRIBUTE_E,
            sip5_20TtiPhysicalPort2TableFieldsFormat, sip5_20TtiPhysicalPort2TableFieldsNames);

        LION3_TABLES_FORMAT_INIT_MAC(
            devObjPtr, SKERNEL_TABLE_FORMAT_EGF_QAG_PORT_TARGET_ATTRIBUTES_E,
            sip5_20EgfQagPortTargetAttributesTableFieldsFormat, sip5_20EgfQagPortTargetAttributesTableFieldsNames);

        LION3_TABLES_FORMAT_INIT_MAC(
            devObjPtr, SKERNEL_TABLE_FORMAT_EGF_QAG_PORT_SOURCE_ATTRIBUTES_E,
            sip5_20EgfQagPortSourceAttributesTableFieldsFormat, sip5_20EgfQagPortSourceAttributesTableFieldsNames);

        LION3_TABLES_FORMAT_INIT_MAC(devObjPtr,
                                     SKERNEL_TABLE_FORMAT_EGF_QAG_CPU_CODE_TO_LB_MAPPER_E,
                                     sip5_20EgfQagCpuCodeToLbMapperTableFieldsFormat,
                                     sip5EgfQagCpuCodeToLbMapperTableFieldsNames);
    }
    else
    {
        LION3_TABLES_FORMAT_INIT_MAC(devObjPtr,
                                     SKERNEL_TABLE_FORMAT_EGF_QAG_CPU_CODE_TO_LB_MAPPER_E,
                                     sip5EgfQagCpuCodeToLbMapperTableFieldsFormat,
                                     sip5EgfQagCpuCodeToLbMapperTableFieldsNames);
    }


    if(SMEM_CHT_IS_SIP7_GET(devObjPtr))
    {
        LION3_TABLES_FORMAT_INIT_MAC(
            devObjPtr, SKERNEL_TABLE_FORMAT_EGF_QAG_PORT_TARGET_ATTRIBUTES_E,
            sip7EgfQagPortTargetAttributesTableFieldsFormat, sip5_20EgfQagPortTargetAttributesTableFieldsNames);
        LION3_TABLES_FORMAT_INIT_MAC(
            devObjPtr, SKERNEL_TABLE_FORMAT_EGF_QAG_TARGET_PORT_MAPPER_E,
            sip7EgfQagTargetPortMapperTableFieldsFormat, sip5_20EgfQagTargetPortMapperTableFieldsNames);
        LION3_TABLES_FORMAT_INIT_MAC(
            devObjPtr, SKERNEL_TABLE_FORMAT_EGF_QAG_PORT_SOURCE_ATTRIBUTES_E,
            sip7EgfQagPortSourceAttributesTableFieldsFormat, sip5_20EgfQagPortSourceAttributesTableFieldsNames);
        LION3_TABLES_FORMAT_INIT_MAC(
            devObjPtr, SKERNEL_TABLE_FORMAT_EGF_QAG_CPU_CODE_TO_LB_MAPPER_E,
            sip7EgfQagCpuCodeToLbMapperTableFieldsFormat, sip5EgfQagCpuCodeToLbMapperTableFieldsNames);
    }
    else
    if(SMEM_CHT_IS_SIP6_GET(devObjPtr))
    {

        LION3_TABLES_FORMAT_INIT_MAC(
            devObjPtr, SKERNEL_TABLE_FORMAT_EGF_QAG_PORT_TARGET_ATTRIBUTES_E,
            sip6EgfQagPortTargetAttributesTableFieldsFormat, sip5_20EgfQagPortTargetAttributesTableFieldsNames);
        LION3_TABLES_FORMAT_INIT_MAC(
            devObjPtr, SKERNEL_TABLE_FORMAT_EGF_QAG_TARGET_PORT_MAPPER_E,
            sip5_20EgfQagTargetPortMapperTableFieldsFormat, sip5_20EgfQagTargetPortMapperTableFieldsNames);
    }

    if(SMEM_CHT_IS_SIP6_30_GET(devObjPtr))
    {
        LION3_TABLES_FORMAT_INIT_MAC(devObjPtr, SKERNEL_TABLE_FORMAT_SMU_IRF_SNG_E,
            sip6_30SmuIrfSngTableFieldsFormat, sip6_30SmuIrfSngTableFieldsNames);

        LION3_TABLES_FORMAT_INIT_MAC(devObjPtr, SKERNEL_TABLE_FORMAT_SMU_IRF_COUNTERS_E,
            sip6_30SmuIrfCountersTableFieldsFormat, sip6_30SmuIrfCountersTableFieldsNames);

        LION3_TABLES_FORMAT_INIT_MAC(devObjPtr, SKERNEL_TABLE_FORMAT_SMU_SGC_INTERVAL_MAX_E,
            sip6_30SmuSgcIntervalMaxTableFieldsFormat, sip6_30SmuSgcIntervalMaxTableFieldsNames);

        LION3_TABLES_FORMAT_INIT_MAC(devObjPtr, SKERNEL_TABLE_FORMAT_SMU_SGC_OCTET_COUNTERS_E,
            sip6_30SmuSgcOctetCountersTableFieldsFormat, sip6_30SmuSgcOctetCountersTableFieldsNames);

        LION3_TABLES_FORMAT_INIT_MAC(devObjPtr, SKERNEL_TABLE_FORMAT_SMU_SGC_TABLE_SET_TIME_CONFIGURATIONS_E,
            sip6_30SmuSgcTableSetTimeConfigurationsTableFieldsFormat, sip6_30SmuSgcTableSetTimeConfigurationsTableFieldsNames);

        LION3_TABLES_FORMAT_INIT_MAC(devObjPtr, SKERNEL_TABLE_FORMAT_SMU_SGC_TIME_SLOT_ATTRIBUTES_E,
            sip6_30SmuSgcTimeSlotAttributesTableFieldsFormat, sip6_30SmuSgcTimeSlotAttributesTableFieldsNames);

        LION3_TABLES_FORMAT_INIT_MAC(devObjPtr, SKERNEL_TABLE_FORMAT_SMU_SGC_TIME_TO_ADVANCE_E,
            sip6_30SmuSgcTimeToAdvanceTableFieldsFormat, sip6_30SmuSgcTimeToAdvanceTableFieldsNames);
    }

}

/**
* @internal snetIngressTablesFormatInit function
* @endinternal
*
* @brief   init the format of ingress tables.
*
* @param[in] devObjPtr                - pointer to device object.
*/
void snetIngressTablesFormatInit(
    IN SKERNEL_DEVICE_OBJECT            * devObjPtr
)
{
    GT_U32 ii;

    /* init local tables */
    localTablesFormatInit(devObjPtr);
    /* init TTI tables */
    snetTtiTablesFormatInit(devObjPtr);
    /* init PCL tables */
    snetPclTablesFormatInit(devObjPtr);
    /* init L2i tables */
    snetL2iTablesFormatInit(devObjPtr);
    /* init EQ tables */
     if(SMEM_CHT_IS_SIP7_GET(devObjPtr))
     {
         /* init IPE tables */
         snetAasIpeTablesFormatInit(devObjPtr);
     }
    snetEqTablesFormatInit(devObjPtr);
    /* init PLR tables */
    snetPlrTablesFormatInit(devObjPtr);
    /* init HA tables */
    snetHaTablesFormatInit(devObjPtr);
    /* init IPvx tables */
    snetIpvxTablesFormatInit(devObjPtr);
    /* init Exact match tables */
    if(SMEM_CHT_IS_SIP7_GET(devObjPtr))
    {
        snetSip7ExactMatchTablesFormatInit(devObjPtr);
    }
    else
    {
        snetExactMatchTablesFormatInit(devObjPtr);
    }
    /* init OAM tables */
    snetOamTablesFormatInit(devObjPtr);

    if(SMEM_CHT_IS_SIP6_GET(devObjPtr))
    {
        /* init preq tables */
        snetFalconPreqTablesFormatInit(devObjPtr);
    }

    if(SMEM_CHT_IS_SIP7_GET(devObjPtr))
    {
        snetGdmaTablesFormatInit(devObjPtr);

        /* init HA tables */
        snetAasHaTablesFormatInit(devObjPtr);
    }

    for(ii = 0; ii < SKERNEL_TABLE_FORMAT_LAST_E; ii++)
    {
        if(devObjPtr->tableFormatInfo[ii].fieldsInfoPtr)
        {
            snetFillFieldsStartBitInfo(devObjPtr,
                devObjPtr->tableFormatInfo[ii].formatNamePtr,
                devObjPtr->tableFormatInfo[ii].numFields,
                devObjPtr->tableFormatInfo[ii].fieldsInfoPtr,
                devObjPtr->tableFormatInfo[ii].fieldsNamePtr);
        }
    }

    return;
}

/**
* @internal snetLion3PassengerOuterTagIsTag0_1 function
* @endinternal
*
* @brief   set innerTag0Exists,innerPacketTag0Vid,innerPacketTag0CfiDei,innerPacketTag0Up fields in descriptor
*
* @param[in,out] descrPtr                 - pointer to the frame's descriptor.
*/
void  snetLion3PassengerOuterTagIsTag0_1(
    IN    SKERNEL_DEVICE_OBJECT           *devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC *descrPtr
)
{
    DECLARE_FUNC_NAME(snetLion3PassengerOuterTagIsTag0_1);

    /* ttiPassengerOuterTagIsTag0_1 - need to swap tag0 and tag 1 recognitions */
    if((descrPtr->eArchExtInfo.ttiPreTtiLookupIngressEPortTablePtr)&&
       (SMEM_LION3_TTI_DEFAULT_EPORT_ENTRY_FIELD_GET(devObjPtr, descrPtr,
        SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_TTI_802_1AH_PASSENGER_STAG_IS_TAG0_1)))
    {
        __LOG(("ttiPassengerOuterTagIsTag0_1 --> check if tag 1 is the outer tag (of passenger) \n"));
        descrPtr->ingressTunnelInfo.innerTag0Exists = (descrPtr->ingressTunnelInfo.innerFrameDescrPtr->ingressVlanTag1Type ==
                SKERNEL_FRAME_TR101_VLAN_TAG_OUTER_E) ?
            1 : 0;
        if(descrPtr->ingressTunnelInfo.innerTag0Exists)
        {
            /* use values from TAG 1 */
            __LOG(("use values from TAG 1 (vid1,cfidei1,up1) \n"));
            descrPtr->ingressTunnelInfo.innerPacketTag0Vid = descrPtr->ingressTunnelInfo.innerFrameDescrPtr->vid1;
            descrPtr->ingressTunnelInfo.innerPacketTag0CfiDei = descrPtr->ingressTunnelInfo.innerFrameDescrPtr->cfidei1;
            descrPtr->ingressTunnelInfo.innerPacketTag0Up     = descrPtr->ingressTunnelInfo.innerFrameDescrPtr->up1;
        }
        else
        {
            __LOG(("TAG 1 was not recognized as the outer tag (of passenger)\n"));
            if(descrPtr->ingressTunnelInfo.innerFrameDescrPtr->ingressVlanTag1Type == SKERNEL_FRAME_TR101_VLAN_TAG_INNER_E)
            {
                __LOG(("Note that tag 0 was recognized as 'outer' and tag 1 as 'inner' (of passenger)\n "));
            }
            else
            if(descrPtr->ingressTunnelInfo.innerFrameDescrPtr->ingressVlanTag0Type == SKERNEL_FRAME_TR101_VLAN_TAG_OUTER_E)
            {
                __LOG(("Note that tag 0 was recognized as 'outer' and tag 1 NOT exists (of passenger)\n "));
            }
        }

    }
    else
    {
        __LOG(("ttiPassengerOuterTagIsTag0_1 --> check if tag 0 is the outer tag (of passenger) \n"));
        descrPtr->ingressTunnelInfo.innerTag0Exists = (descrPtr->ingressTunnelInfo.innerFrameDescrPtr->ingressVlanTag0Type ==
                SKERNEL_FRAME_TR101_VLAN_TAG_OUTER_E) ?
            1 : 0;
        if(descrPtr->ingressTunnelInfo.innerTag0Exists)
        {
            /* use values from TAG 0 */
            __LOG(("use values from TAG 0 (eVid,cfidei,up) \n"));
            descrPtr->ingressTunnelInfo.innerPacketTag0Vid = descrPtr->ingressTunnelInfo.innerFrameDescrPtr->eVid;
            descrPtr->ingressTunnelInfo.innerPacketTag0CfiDei = descrPtr->ingressTunnelInfo.innerFrameDescrPtr->cfidei;
            descrPtr->ingressTunnelInfo.innerPacketTag0Up     = descrPtr->ingressTunnelInfo.innerFrameDescrPtr->up;
        }
        else
        {
            __LOG(("TAG 0 was not recognized as the outer tag (of passenger)\n"));

            if(descrPtr->ingressTunnelInfo.innerFrameDescrPtr->ingressVlanTag0Type == SKERNEL_FRAME_TR101_VLAN_TAG_INNER_E)
            {
                __LOG(("Note that tag 1 was recognized as 'outer' and tag 0 as 'inner' (of passenger)\n "));
            }
            else
            if(descrPtr->ingressTunnelInfo.innerFrameDescrPtr->ingressVlanTag1Type == SKERNEL_FRAME_TR101_VLAN_TAG_OUTER_E)
            {
                __LOG(("Note that tag 1 was recognized as 'outer' and tag 0 NOT exists (of passenger)\n "));
            }
        }
    }


    __LOG_PARAM(descrPtr->ingressTunnelInfo.innerPacketTag0Vid);
    __LOG_PARAM(descrPtr->ingressTunnelInfo.innerPacketTag0Up);
    __LOG_PARAM(descrPtr->ingressTunnelInfo.innerPacketTag0CfiDei);
    __LOG_PARAM(descrPtr->ingressTunnelInfo.innerTag0Exists);

}


#if 0
/**
* @internal sdmaTxPacketGeneratorConfigSet_test function
* @endinternal
*
* @brief   init the format of ingress tables.
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] txQue                    - SDMA TxQ number
* @param[in] firstDmaAddr             - first SDMA descriptor used to close descriptor ring
* @param[in] lastDmaAddr              - last SDMA descriptor used to close descriptor ring
*/
GT_VOID sdmaTxPacketGeneratorConfigSet_test
(
IN SKERNEL_DEVICE_OBJECT * devObjPtr,
IN GT_U32 txQue,
IN GT_U32 firstDmaAddr,
IN GT_U32 lastDmaAddr
)
{
    SNET_STRUCT_TX_DESC * txDescPtr;
    SNET_STRUCT_TX_DESC txDesc;                 /* buffer for TX descriptor */
    GT_U32 currentDmaAddr = firstDmaAddr;
    GT_U32 regVal;
    GT_U32  regAddr_CUR_DESC_PTR0 =  SMEM_CHT_TX_SDMA_CUR_DESC_PTR0_REG(devObjPtr) + (txQue * 0x4);

    txDescPtr = &txDesc;

    /* Start read from first descriptor in chain */
    smemRegSet(devObjPtr,regAddr_CUR_DESC_PTR0,currentDmaAddr);

    /* read the DMA for needed value , put data into txDescPtr */
    snetChtPerformScibDmaRead(SNET_CHT_DMA_CLIENT_PACKET_FROM_CPU_E,
                              devObjPtr->deviceId, currentDmaAddr,
                              NUM_BYTES_TO_WORDS(sizeof(*txDescPtr)),
                              (GT_U32*)txDescPtr,SCIB_DMA_WORDS);

    while(currentDmaAddr != lastDmaAddr)
    {

        /* Set ownership on descr to SDMA */
        TX_DESC_SET_OWN_BIT(txDescPtr, TX_DESC_DMA_OWN);

        /* Write DMA memory new descriptor */
        snetChtPerformScibDmaWrite(SNET_CHT_DMA_CLIENT_PACKET_FROM_CPU_E,
                                   devObjPtr->deviceId, currentDmaAddr,
                                   NUM_BYTES_TO_WORDS(sizeof(*txDescPtr)),
                                   (GT_U32*)txDescPtr, SCIB_DMA_WORDS);

        currentDmaAddr = txDescPtr->nextDescPointer;

        /* Update next descriptor to feed */
        smemRegSet(devObjPtr,regAddr_CUR_DESC_PTR0,currentDmaAddr);

        /* Read the DMA for needed value, put data into txDescPtr */
        snetChtPerformScibDmaRead(SNET_CHT_DMA_CLIENT_PACKET_FROM_CPU_E,
                                  devObjPtr->deviceId, currentDmaAddr,
                                  NUM_BYTES_TO_WORDS(sizeof(*txDescPtr)),
                                  (GT_U32*)txDescPtr, SCIB_DMA_WORDS);
    }

    /* Set ownership on descr to SDMA */
    TX_DESC_SET_OWN_BIT(txDescPtr, TX_DESC_DMA_OWN);
    /* Last descriptor in chain */
    TX_DESC_SET_LAST_BIT(txDescPtr, 1);
    /* Close descriptors ring */
    txDescPtr->nextDescPointer = firstDmaAddr;

    /* Write DMA memory last descriptor */
    snetChtPerformScibDmaWrite(SNET_CHT_DMA_CLIENT_PACKET_FROM_CPU_E,
                               devObjPtr->deviceId, lastDmaAddr,
                               NUM_BYTES_TO_WORDS(sizeof(*txDescPtr)),
                               (GT_U32*)txDescPtr, SCIB_DMA_WORDS);

    /* Set descriptor pointer to first one */
    smemRegSet(devObjPtr,regAddr_CUR_DESC_PTR0,firstDmaAddr);

    /* Enable packet generator, enable packet interval counter max value */
    regVal = 0xffffffff;

    /* Tx SDMA Packet Generator Config Queue */
    scibWriteMemory(devObjPtr->deviceId,
                    SMEM_CHT_PACKET_GENERATOR_CONFIG_QUEUE_REG(devObjPtr, txQue),
                    1, &regVal);

    scibReadMemory(devObjPtr->deviceId, 0x00002868, 1, &regVal);

    /* Disable the Tx SDMA queue  */
    SMEM_U32_SET_FIELD(regVal, txQue, 1, 0);

    /* Transmit SDMA Fixed Priority Configuration Register */
    smemRegFldSet(devObjPtr, 0x00002870, txQue, 1, 1);

    /* Enable the Tx SDMA queue  */
    SMEM_U32_SET_FIELD(regVal, txQue, 1, 1);

    scibWriteMemory(devObjPtr->deviceId, 0x00002868, 1, &regVal);
}
#endif
/**
* @internal snetBobcat2EeeProcess function
* @endinternal
*
* @brief   process EEE
*/
void snetBobcat2EeeProcess(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                   macPort,
    IN SMAIN_DIRECTION_ENT      direction
)
{
    DECLARE_FUNC_NAME(snetBobcat2EeeProcess);
    GT_BIT enable;

    if (! SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        __LOG(("EEE not implemented by simulation on this device \n"));
        return;
    }

    if(IS_CHT_HYPER_GIGA_PORT(devObjPtr, macPort))
    {
        __LOG(("port [%d] is not in GE mode : EEE only on GE ports 10/100/1000 \n",macPort));
        return;
    }

    /* check if the EEE enabled on the port */
    /*<LPI request enable>*/
    smemRegFldGet(devObjPtr,
        SMEM_SIP_5_MAC_LPI_CTRL_REG(devObjPtr,macPort,1),
        0,1,&enable);

    if (enable == 0)
    {
        __LOG(("EEE is not enabled on the port[%d] \n",macPort));
        return;
    }

    /* make atomic action */
    scibAccessLock();

    if (direction == SMAIN_DIRECTION_INGRESS_E)
    {
        __LOG(("EEE is enabled on the port[%d] , generate Rx direction interrupts (PCS,MAC) \n",macPort));
        /* <PCS Rx Path Received LPI> */
        smemBobcat2DoPortInterrupt(devObjPtr,macPort,11);
        /* <MAC Rx Path Received LPI> */
        smemBobcat2DoPortInterrupt(devObjPtr,macPort,13);
    }
    else
    {
        __LOG(("EEE is enabled on the port[%d] , generate Tx direction interrupt (PCS) \n",macPort));
        /* <PCS Tx Path Received LPI> */
        smemBobcat2DoPortInterrupt(devObjPtr,macPort,12);
    }

    /* release the action */
    scibAccessUnlock();

    /* NOTE: due to Erratum the counter is not incremented ..
       and there is no intention to fix it */

}
/**
* @internal snetBobcat2EeeCheckInterrupts function
* @endinternal
*
* @brief   check for EEE interrupts
*/
void snetBobcat2EeeCheckInterrupts(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32                   macPort
)
{
    DECLARE_FUNC_NAME(snetBobcat2EeeCheckInterrupts);
    GT_BIT fldValue;

    if (! SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        __LOG(("EEE not implemented by simulation on this device \n"));
        return;
    }

    if(IS_CHT_HYPER_GIGA_PORT(devObjPtr, macPort))
    {
        __LOG(("port [%d] is not in GE mode : EEE only on GE ports 10/100/1000 \n",macPort));
        return;
    }

    /*<LinkUp>*/
    fldValue = snetChtPortMacFieldGet(devObjPtr, macPort,
        SNET_CHT_PORT_MAC_FIELDS_LinkState_E);
    if (fldValue != 1)
    {
        /* link is down */
        return;
    }

    /* check if the EEE enabled on the port */
    fldValue = snetChtPortMacFieldGet(devObjPtr, macPort,
        SNET_CHT_PORT_MAC_FIELDS_LPI_request_enable_E);
    if (fldValue == 0)
    {
        __LOG(("EEE is not enabled on the port[%d] \n",macPort));
        return;
    }

    /*portInLoopBack*/
    fldValue = snetChtPortMacFieldGet(devObjPtr, macPort,
        SNET_CHT_PORT_MAC_FIELDS_portInLoopBack_E);
    if(CHT_IS_CG_OR_MTI_MAC_USED_MAC(devObjPtr,macPort))
    {
        __LOG(("EEE is enabled on the port[%d] , but interrupt not happen on MTI/CG : 100G port \n",macPort));
        return;
    }
    /* make atomic action */
    scibAccessLock();

    __LOG(("EEE is enabled on the port[%d] , generate Tx direction interrupt (PCS) \n",macPort));
    /* <PCS Tx Path Received LPI> */
    fldValue = snetChtPortMacFieldGet(devObjPtr, macPort,
        SNET_CHT_PORT_MAC_FIELDS_int_bit_for_PCS_Tx_Path_Received_LPI_E);
    smemBobcat2DoPortInterrupt(devObjPtr,macPort,fldValue);

    if(fldValue)
    {
        __LOG(("EEE is enabled on the port[%d] in loopback mode , generate Rx direction interrupts (PCS,MAC) \n",macPort));
        /* <PCS Rx Path Received LPI> */
        fldValue = snetChtPortMacFieldGet(devObjPtr, macPort,
            SNET_CHT_PORT_MAC_FIELDS_int_bit_for_PCS_Rx_Path_Received_LPI_E);
        smemBobcat2DoPortInterrupt(devObjPtr,macPort,fldValue);

        /* <MAC Rx Path Received LPI> */
        fldValue = snetChtPortMacFieldGet(devObjPtr, macPort,
            SNET_CHT_PORT_MAC_FIELDS_int_bit_for_MAC_Rx_Path_Received_LPI_E);
        smemBobcat2DoPortInterrupt(devObjPtr,macPort,fldValue);
    }

    /* release the action */
    scibAccessUnlock();

    /* NOTE: due to Erratum the counter is not incremented ..
       and there is no intention to fix it */

}

static MIB_COUNTER_INFO_STC   portMibInfoArr[SNET_CHT_PORT_MIB_COUNTERS____LAST____E] =
{
    {0x00  ,2  ,STR(GOOD_OCTETS_RECEIVED)},        /*    SNET_CHT_PORT_MIB_COUNTERS_GOOD_OCTETS_RECEIVED_E,            */
    {0x08  ,1  ,STR(BAD_OCTETS_RECEIVED)},         /*    SNET_CHT_PORT_MIB_COUNTERS_BAD_OCTETS_RECEIVED_E,             */
    {0x0c  ,1  ,STR(CRC_ERRORS_SENT)},             /*    SNET_CHT_PORT_MIB_COUNTERS_CRC_ERRORS_SENT_E,                 */
    {0x10  ,1  ,STR(GOOD_UNICAST_FRAMES_RECEIVED)},/*    SNET_CHT_PORT_MIB_COUNTERS_GOOD_UNICAST_FRAMES_RECEIVED_E,    */
    {0x14  ,1  ,STR(RESERVED_1)},                  /*    SNET_CHT_PORT_MIB_COUNTERS_FRAMES_1024_TO_1518_OCTETS_E,      */
    {0x18  ,1  ,STR(BROADCAST_FRAMES_RECEIVED)},   /*    SNET_CHT_PORT_MIB_COUNTERS_BROADCAST_FRAMES_RECEIVED_E,       */
    {0x1c  ,1  ,STR(MULTICAST_FRAMES_RECEIVED)},   /*    SNET_CHT_PORT_MIB_COUNTERS_MULTICAST_FRAMES_RECEIVED_E,       */
    {0x20  ,1  ,STR(FRAMES_64_OCTETS)},            /*    SNET_CHT_PORT_MIB_COUNTERS_FRAMES_64_OCTETS_E,                */
    {0x24  ,1  ,STR(FRAMES_65_TO_127_OCTETS)},     /*    SNET_CHT_PORT_MIB_COUNTERS_FRAMES_65_TO_127_OCTETS_E,         */
    {0x28  ,1  ,STR(FRAMES_128_TO_255_OCTETS)},    /*    SNET_CHT_PORT_MIB_COUNTERS_FRAMES_128_TO_255_OCTETS_E,        */
    {0x2c  ,1  ,STR(FRAMES_256_TO_511_OCTETS)},    /*    SNET_CHT_PORT_MIB_COUNTERS_FRAMES_256_TO_511_OCTETS_E,        */
    {0x30  ,1  ,STR(FRAMES_512_TO_1023_OCTETS)},   /*    SNET_CHT_PORT_MIB_COUNTERS_FRAMES_512_TO_1023_OCTETS_E,       */
    {0x34  ,1  ,STR(FRAMES_1024_TO_MAX_OCTETS)},   /*    SNET_CHT_PORT_MIB_COUNTERS_FRAMES_1024_TO_MAX_OCTETS_E,       */
    {0x38  ,2  ,STR(GOOD_OCTETS_SENT)},            /*    SNET_CHT_PORT_MIB_COUNTERS_GOOD_OCTETS_SENT_E,                */
    {0x40  ,1  ,STR(UNICAST_FRAME_SENT)},          /*    SNET_CHT_PORT_MIB_COUNTERS_UNICAST_FRAME_SENT_E,              */
    {0x44  ,1  ,STR(RESERVED_2)},                  /*    SNET_CHT_PORT_MIB_COUNTERS_RESERVED_2_E,                      */
    {0x48  ,1  ,STR(MULTICAST_FRAMES_SENT)},       /*    SNET_CHT_PORT_MIB_COUNTERS_MULTICAST_FRAMES_SENT_E,           */
    {0x4c  ,1  ,STR(BROADCAST_FRAMES_SENT)},       /*    SNET_CHT_PORT_MIB_COUNTERS_BROADCAST_FRAMES_SENT_E,           */
    {0x50  ,1  ,STR(RESERVED_3)},                  /*    SNET_CHT_PORT_MIB_COUNTERS_RESERVED_3_E,                      */
    {0x54  ,1  ,STR(FC_SENT)},                     /*    SNET_CHT_PORT_MIB_COUNTERS_FC_SENT_E,                         */
    {0x58  ,1  ,STR(FC_RECEIVED)},                 /*    SNET_CHT_PORT_MIB_COUNTERS_FC_RECEIVED_E,                     */
    {0x5c  ,1  ,STR(RECEIVED_FIFO_OVERRUN)},       /*    SNET_CHT_PORT_MIB_COUNTERS_RECEIVED_FIFO_OVERRUN_E,           */
    {0x60  ,1  ,STR(UNDERSIZE)},                   /*    SNET_CHT_PORT_MIB_COUNTERS_UNDERSIZE_E,                       */
    {0x64  ,1  ,STR(FRAGMENTS)},                   /*    SNET_CHT_PORT_MIB_COUNTERS_FRAGMENTS_E,                       */
    {0x68  ,1  ,STR(OVERSIZE)},                    /*    SNET_CHT_PORT_MIB_COUNTERS_OVERSIZE_E,                        */
    {0x6c  ,1  ,STR(JABBER)},                      /*    SNET_CHT_PORT_MIB_COUNTERS_JABBER_E,                          */
    {0x70  ,1  ,STR(RX_ERROR_FRAME_RECEIVED)},     /*    SNET_CHT_PORT_MIB_COUNTERS_RX_ERROR_FRAME_RECEIVED_E,         */
    {0x74  ,1  ,STR(BAD_CRC)},                     /*    SNET_CHT_PORT_MIB_COUNTERS_BAD_CRC_E,                         */
    {0x78  ,1  ,STR(COLLISION)},                   /*    SNET_CHT_PORT_MIB_COUNTERS_COLLISION_E,                       */
    {0x7c  ,1  ,STR(LATE_COLLISION)}               /*    SNET_CHT_PORT_MIB_COUNTERS_LATE_COLLISION_E,                  */
};

/**
* @internal snetChtPortMibOffsetGet function
* @endinternal
*
* @brief   get the offset from the start of the MIB memory of the port , for
*         specific MIB counter type
*/
static GT_U32  snetChtPortMibOffsetGet
(
    IN SKERNEL_DEVICE_OBJECT            * devObjPtr,
    IN GT_U32                           macPort,
    IN SNET_CHT_PORT_MIB_COUNTERS_ENT   mibType
)
{
    if(devObjPtr->portMibCounters64Bits)
    {
        return 8 * mibType;
    }

    return portMibInfoArr[mibType].offset;
}
/**
* @internal snetChtPortMibWordsSizeGet function
* @endinternal
*
* @brief   get the size in words of specific MIB counter type
*/
static GT_U32  snetChtPortMibWordsSizeGet
(
    IN SKERNEL_DEVICE_OBJECT            * devObjPtr,
    IN GT_U32                           macPort,
    IN SNET_CHT_PORT_MIB_COUNTERS_ENT   mibType
)
{
    if(devObjPtr->portMibCounters64Bits)
    {
        return 2; /*all registers 2 words */
    }

    return portMibInfoArr[mibType].numWords;
}

/**
* @internal snetChtPortMibOffsetUpdate function
* @endinternal
*
* @brief   update the offset from the start of the MIB memory of the port , for
*         specific MIB counter type
*/
void  snetChtPortMibOffsetUpdate
(
    IN SKERNEL_DEVICE_OBJECT            * devObjPtr,
    IN GT_U32                           macPort,
    IN GT_U32                           *mibMemoryPtr,
    IN SNET_CHT_PORT_MIB_COUNTERS_ENT   mibType,
    IN GT_U32                           incValue
)
{
    DECLARE_FUNC_NAME(snetChtPortMibOffsetUpdate);
    GT_U32  offset;
    GT_CHAR* mibName;
    GT_U32   numWords;
    GT_U32  *currMibPtr;
    GT_U64 orig64Value;
    GT_U64 new64Value;
    GT_U64 inc64Value;

    if(CHT_IS_CG_OR_MTI_MAC_USED_MAC(devObjPtr,macPort))
    {
        /* must call function snetChtPortMibOffsetUpdate_100GMac(...) with proper counter type */
        skernelFatalError("snetChtPortMibOffsetUpdate: macPort[%d] is in MTI/CG(100G) mode and should be handled elsewhere \n",
            macPort);
        return;
    }

    if(mibType >= SNET_CHT_PORT_MIB_COUNTERS____LAST____E)
    {
        skernelFatalError("snetChtPortMibOffsetUpdate: mibType[%d] out of range \n",
            mibType);
        return;
    }

    mibName = portMibInfoArr[mibType].name;
    offset = snetChtPortMibOffsetGet(devObjPtr,macPort,mibType);
    numWords = snetChtPortMibWordsSizeGet(devObjPtr,macPort,mibType);


    currMibPtr = mibMemoryPtr + (offset / 4);

    if(numWords == 1) /*32 bits counter*/
    {
        __LOG(("update PORT MIB counter [%s] - increment by [%d] from value[0x%8.8x]\n",
            mibName,
            incValue,
            currMibPtr[0]));

        currMibPtr[0] += incValue;
    }
    else              /*64 bits counter*/
    {
        __LOG(("update PORT MIB counter [%s] - increment by [%d] from value: H[0x%8.8x] L[0x%8.8x]\n",
            mibName,
            incValue,
            currMibPtr[1],
            currMibPtr[0]));

        orig64Value.l[0] = currMibPtr[0];
        orig64Value.l[1] = currMibPtr[1];

        inc64Value.l[0] = incValue;
        inc64Value.l[1] = 0;

        new64Value = prvSimMathAdd64(orig64Value, inc64Value);

        currMibPtr[0] = new64Value.l[0];
        currMibPtr[1] = new64Value.l[1];
    }

    return;
}

#ifndef offsetof
#define offsetof(TYPE, MEMBER) ( (GT_U32) ( (GT_UINTPTR) (&((TYPE *)0)->MEMBER) ) )
#endif
#define offsetof_GOP_100G_MIB_REGS_ADDR(counter) offsetof(SMEM_SIP5_PP_REGS_ADDR_STC,GOP.ports_100G_CG_MAC[0].CG_MIBs.counter)

#define PORT_MIB_INFO_ENTRY_100G(counterName) \
    offsetof_GOP_100G_MIB_REGS_ADDR(counterName)                      ,2,STR(counterName)

#define DUMMY_MIB_INFO_ENTRY_100G   \
    0                      ,2,"dummy"

static MIB_COUNTER_INFO_STC portMibInfoArr_100G[SNET_CHT_PORT_MIB_COUNTERS_100G_PORT____LAST____E] =
{

     {PORT_MIB_INFO_ENTRY_100G(aFramesTransmittedOK)             }
    ,{PORT_MIB_INFO_ENTRY_100G(aFramesReceivedOK)                }
    ,{PORT_MIB_INFO_ENTRY_100G(aFrameCheckSequenceErrors)        }
    ,{PORT_MIB_INFO_ENTRY_100G(aAlignmentErrors)                 }
    ,{PORT_MIB_INFO_ENTRY_100G(aPAUSEMACCtrlFramesTransmitted)   }
    ,{PORT_MIB_INFO_ENTRY_100G(aPAUSEMACCtrlFramesReceived)      }
    ,{PORT_MIB_INFO_ENTRY_100G(aFrameTooLongErrors)              }
    ,{PORT_MIB_INFO_ENTRY_100G(aInRangeLengthErrors)             }
    ,{PORT_MIB_INFO_ENTRY_100G(VLANTransmittedOK)                }
    ,{PORT_MIB_INFO_ENTRY_100G(VLANReceivedOK)                   }

    ,{PORT_MIB_INFO_ENTRY_100G(ifOutOctets)                      }
    ,{PORT_MIB_INFO_ENTRY_100G(ifInOctets)                       }
    ,{PORT_MIB_INFO_ENTRY_100G(ifInUcastPkts)                    }
    ,{PORT_MIB_INFO_ENTRY_100G(ifInMulticastPkts)                }
    ,{PORT_MIB_INFO_ENTRY_100G(ifInBroadcastPkts)                }
    ,{PORT_MIB_INFO_ENTRY_100G(ifOutErrors)                      }
    ,{PORT_MIB_INFO_ENTRY_100G(ifOutUcastPkts)                   }
    ,{PORT_MIB_INFO_ENTRY_100G(ifOutMulticastPkts)               }
    ,{PORT_MIB_INFO_ENTRY_100G(ifOutBroadcastPkts)               }
    ,{PORT_MIB_INFO_ENTRY_100G(etherStatsDropEvents)             }
    ,{PORT_MIB_INFO_ENTRY_100G(etherStatsOctets)                 }
    ,{PORT_MIB_INFO_ENTRY_100G(etherStatsPkts)                   }
    ,{PORT_MIB_INFO_ENTRY_100G(etherStatsUndersizePkts)          }
    ,{PORT_MIB_INFO_ENTRY_100G(etherStatsPkts64Octets)           }
    ,{PORT_MIB_INFO_ENTRY_100G(etherStatsPkts65to127Octets)      }
    ,{PORT_MIB_INFO_ENTRY_100G(etherStatsPkts128to255Octets)     }
    ,{PORT_MIB_INFO_ENTRY_100G(etherStatsPkts256to511Octets)     }
    ,{PORT_MIB_INFO_ENTRY_100G(etherStatsPkts512to1023Octets)    }
    ,{PORT_MIB_INFO_ENTRY_100G(etherStatsPkts1024to1518Octets)   }
    ,{PORT_MIB_INFO_ENTRY_100G(etherStatsPkts1519toMaxOctets)    }
    ,{PORT_MIB_INFO_ENTRY_100G(etherStatsOversizePkts)           }
    ,{PORT_MIB_INFO_ENTRY_100G(etherStatsJabbers)                }
    ,{PORT_MIB_INFO_ENTRY_100G(etherStatsFragments)              }
    ,{PORT_MIB_INFO_ENTRY_100G(ifInErrors)                       }
    ,{PORT_MIB_INFO_ENTRY_100G(aCBFCPAUSEFramesTransmitted)      }
    /* dummy 7 entries ... for 'alignment' */
    ,{DUMMY_MIB_INFO_ENTRY_100G}
    ,{DUMMY_MIB_INFO_ENTRY_100G}
    ,{DUMMY_MIB_INFO_ENTRY_100G}
    ,{DUMMY_MIB_INFO_ENTRY_100G}
    ,{DUMMY_MIB_INFO_ENTRY_100G}
    ,{DUMMY_MIB_INFO_ENTRY_100G}
    ,{DUMMY_MIB_INFO_ENTRY_100G}
    ,{PORT_MIB_INFO_ENTRY_100G(aCBFCPAUSEFramesReceived)         }
    /* dummy 7 entries ... for 'alignment' */
    ,{DUMMY_MIB_INFO_ENTRY_100G}
    ,{DUMMY_MIB_INFO_ENTRY_100G}
    ,{DUMMY_MIB_INFO_ENTRY_100G}
    ,{DUMMY_MIB_INFO_ENTRY_100G}
    ,{DUMMY_MIB_INFO_ENTRY_100G}
    ,{DUMMY_MIB_INFO_ENTRY_100G}
    ,{DUMMY_MIB_INFO_ENTRY_100G}
    ,{PORT_MIB_INFO_ENTRY_100G(aMACControlFramesTransmitted)     }
    ,{PORT_MIB_INFO_ENTRY_100G(aMACControlFramesReceived)        }
};

/* PIPE device not hold port 0 addresses ... so cant use info in portMibInfoArr_100G[ii].offset */
#define PORT_12 12
#define offsetof_port_12_GOP_100G_MIB_REGS_ADDR(counter) offsetof(SMEM_SIP5_PP_REGS_ADDR_STC,GOP.ports_100G_CG_MAC[PORT_12].CG_MIBs.counter)
#define offsetof_port_12_DUMMY_MIB_INFO_ENTRY_100G  0
static GT_U32 offsetof_port_12_PIPE_portMibInfoArr_100G[SNET_CHT_PORT_MIB_COUNTERS_100G_PORT____LAST____E] =
{

     offsetof_port_12_GOP_100G_MIB_REGS_ADDR(aFramesTransmittedOK)
    ,offsetof_port_12_GOP_100G_MIB_REGS_ADDR(aFramesReceivedOK)
    ,offsetof_port_12_GOP_100G_MIB_REGS_ADDR(aFrameCheckSequenceErrors)
    ,offsetof_port_12_GOP_100G_MIB_REGS_ADDR(aAlignmentErrors)
    ,offsetof_port_12_GOP_100G_MIB_REGS_ADDR(aPAUSEMACCtrlFramesTransmitted)
    ,offsetof_port_12_GOP_100G_MIB_REGS_ADDR(aPAUSEMACCtrlFramesReceived)
    ,offsetof_port_12_GOP_100G_MIB_REGS_ADDR(aFrameTooLongErrors)
    ,offsetof_port_12_GOP_100G_MIB_REGS_ADDR(aInRangeLengthErrors)
    ,offsetof_port_12_GOP_100G_MIB_REGS_ADDR(VLANTransmittedOK)
    ,offsetof_port_12_GOP_100G_MIB_REGS_ADDR(VLANReceivedOK)

    ,offsetof_port_12_GOP_100G_MIB_REGS_ADDR(ifOutOctets)
    ,offsetof_port_12_GOP_100G_MIB_REGS_ADDR(ifInOctets)
    ,offsetof_port_12_GOP_100G_MIB_REGS_ADDR(ifInUcastPkts)
    ,offsetof_port_12_GOP_100G_MIB_REGS_ADDR(ifInMulticastPkts)
    ,offsetof_port_12_GOP_100G_MIB_REGS_ADDR(ifInBroadcastPkts)
    ,offsetof_port_12_GOP_100G_MIB_REGS_ADDR(ifOutErrors)
    ,offsetof_port_12_GOP_100G_MIB_REGS_ADDR(ifOutUcastPkts)
    ,offsetof_port_12_GOP_100G_MIB_REGS_ADDR(ifOutMulticastPkts)
    ,offsetof_port_12_GOP_100G_MIB_REGS_ADDR(ifOutBroadcastPkts)
    ,offsetof_port_12_GOP_100G_MIB_REGS_ADDR(etherStatsDropEvents)
    ,offsetof_port_12_GOP_100G_MIB_REGS_ADDR(etherStatsOctets)
    ,offsetof_port_12_GOP_100G_MIB_REGS_ADDR(etherStatsPkts)
    ,offsetof_port_12_GOP_100G_MIB_REGS_ADDR(etherStatsUndersizePkts)
    ,offsetof_port_12_GOP_100G_MIB_REGS_ADDR(etherStatsPkts64Octets)
    ,offsetof_port_12_GOP_100G_MIB_REGS_ADDR(etherStatsPkts65to127Octets)
    ,offsetof_port_12_GOP_100G_MIB_REGS_ADDR(etherStatsPkts128to255Octets)
    ,offsetof_port_12_GOP_100G_MIB_REGS_ADDR(etherStatsPkts256to511Octets)
    ,offsetof_port_12_GOP_100G_MIB_REGS_ADDR(etherStatsPkts512to1023Octets)
    ,offsetof_port_12_GOP_100G_MIB_REGS_ADDR(etherStatsPkts1024to1518Octets)
    ,offsetof_port_12_GOP_100G_MIB_REGS_ADDR(etherStatsPkts1519toMaxOctets)
    ,offsetof_port_12_GOP_100G_MIB_REGS_ADDR(etherStatsOversizePkts)
    ,offsetof_port_12_GOP_100G_MIB_REGS_ADDR(etherStatsJabbers)
    ,offsetof_port_12_GOP_100G_MIB_REGS_ADDR(etherStatsFragments)
    ,offsetof_port_12_GOP_100G_MIB_REGS_ADDR(ifInErrors)
    ,offsetof_port_12_GOP_100G_MIB_REGS_ADDR(aCBFCPAUSEFramesTransmitted)
    /* dummy 7 entries ... for 'alignment' */
    ,offsetof_port_12_DUMMY_MIB_INFO_ENTRY_100G
    ,offsetof_port_12_DUMMY_MIB_INFO_ENTRY_100G
    ,offsetof_port_12_DUMMY_MIB_INFO_ENTRY_100G
    ,offsetof_port_12_DUMMY_MIB_INFO_ENTRY_100G
    ,offsetof_port_12_DUMMY_MIB_INFO_ENTRY_100G
    ,offsetof_port_12_DUMMY_MIB_INFO_ENTRY_100G
    ,offsetof_port_12_DUMMY_MIB_INFO_ENTRY_100G
    ,offsetof_port_12_GOP_100G_MIB_REGS_ADDR(aCBFCPAUSEFramesReceived)
    /* dummy 7 entries ... for 'alignment' */
    ,offsetof_port_12_DUMMY_MIB_INFO_ENTRY_100G
    ,offsetof_port_12_DUMMY_MIB_INFO_ENTRY_100G
    ,offsetof_port_12_DUMMY_MIB_INFO_ENTRY_100G
    ,offsetof_port_12_DUMMY_MIB_INFO_ENTRY_100G
    ,offsetof_port_12_DUMMY_MIB_INFO_ENTRY_100G
    ,offsetof_port_12_DUMMY_MIB_INFO_ENTRY_100G
    ,offsetof_port_12_DUMMY_MIB_INFO_ENTRY_100G
    ,offsetof_port_12_GOP_100G_MIB_REGS_ADDR(aMACControlFramesTransmitted)
    ,offsetof_port_12_GOP_100G_MIB_REGS_ADDR(aMACControlFramesReceived)
};



/**
* @internal snetChtPortMibOffsetUpdate_100GMac function
* @endinternal
*
* @brief   for 100G mac (CG port)
*         update the offset from the start of the MIB memory of the port , for
*         specific MIB counter type
*/
void  snetChtPortMibOffsetUpdate_100GMac
(
    IN SKERNEL_DEVICE_OBJECT            * devObjPtr,
    IN GT_U32                           macPort,
    IN GT_U32                           *mibMemoryPtr,
    IN SNET_CHT_PORT_MIB_COUNTERS_100G_PORT_ENT   mibType,
    IN GT_U32                           incValue
)
{
    DECLARE_FUNC_NAME(snetChtPortMibOffsetUpdate_100GMac);
    GT_U32  offset;
    GT_CHAR* mibName;
    GT_U32  *currMibPtr;
    GT_U64 orig64Value;
    GT_U64 new64Value;
    GT_U64 inc64Value;
    GT_U32  *regAddrPtr,*regAddrBasePtr;
    GT_U32  offsetoffset_inSTCBaseAddr,offset_inSTC;

    if(!CHT_IS_100G_MAC_USED_MAC(devObjPtr,macPort))
    {
        /* must call function snetChtPortMibOffsetUpdate(...) with proper counter type */
        skernelFatalError("snetChtPortMibOffsetUpdate_100GMac: macPort[%d] is NOT in CG(100G) mode and should be handled elsewhere \n",
            macPort);
        return;
    }

    if(mibType >= SNET_CHT_PORT_MIB_COUNTERS_100G_PORT____LAST____E)
    {
        skernelFatalError("snetChtPortMibOffsetUpdate_100GMac: mibType[%d] out of range \n",
            mibType);
        return;
    }

    mibName = portMibInfoArr_100G[mibType].name;

    if(SMEM_IS_PIPE_FAMILY_GET(devObjPtr) && macPort == 12)
    {
        offset_inSTC = offsetof_port_12_PIPE_portMibInfoArr_100G[mibType];
        /* we need wordOffset from start of the Mibs for this port */
        offsetoffset_inSTCBaseAddr = offsetof_port_12_GOP_100G_MIB_REGS_ADDR(CG_MEMBER_LOWEST_ADDR);
    }
    else
    {
        offset_inSTC = portMibInfoArr_100G[mibType].offset;
        /* we need wordOffset from start of the Mibs for this port */
        offsetoffset_inSTCBaseAddr = offsetof_GOP_100G_MIB_REGS_ADDR(CG_MEMBER_LOWEST_ADDR);
    }

    /* the 'offset' is in STC of SMEM_SIP5_PP_REGS_ADDR_STC */
    regAddrPtr = (GT_U32*)(((GT_CHAR*)SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)) + offset_inSTC);

    /* use wordOffset from start of the Mibs for this port */
    regAddrBasePtr = (GT_U32*)(((GT_CHAR*)SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)) + offsetoffset_inSTCBaseAddr);

    offset = (*regAddrPtr) - (*regAddrBasePtr);

    currMibPtr = mibMemoryPtr + (offset / 4);

    __LOG(("macPort[%d] : update 100G PORT MIB counter [%s] - increment by [%d] from value: H[0x%8.8x] L[0x%8.8x]\n",
        macPort,
        mibName,
        incValue,
        currMibPtr[1],
        currMibPtr[0]));

    orig64Value.l[0] = currMibPtr[0];
    orig64Value.l[1] = currMibPtr[1];

    inc64Value.l[0] = incValue;
    inc64Value.l[1] = 0;

    new64Value = prvSimMathAdd64(orig64Value, inc64Value);

    currMibPtr[0] = new64Value.l[0];
    currMibPtr[1] = new64Value.l[1];

    return;
}

#define FIELD_ENUM_AND_NAME(field) field,STR(field)

typedef struct{
    GT_CHAR* name;
}FIELD_IN_REG_INFO_STC;

static FIELD_IN_REG_INFO_STC    fieldsInMacDbArr[] =
{
     {STR(SNET_CHT_PORT_MAC_FIELDS_PortEn_E)                                        }
    ,{STR(SNET_CHT_PORT_MAC_FIELDS_PortMACReset_E)                                  }
    ,{STR(SNET_CHT_PORT_MAC_FIELDS_LinkState_E)                                     }
    ,{STR(SNET_CHT_PORT_MAC_FIELDS_mru_E)                                           }
    ,{STR(SNET_CHT_PORT_MAC_FIELDS_MIBCntEn_E)                                      }
    ,{STR(SNET_CHT_PORT_MAC_FIELDS_forwardPfcFramesEnable_E)                        }
    ,{STR(SNET_CHT_PORT_MAC_FIELDS_forwardUnknownMacControlFramesEnable_E)          }
    ,{STR(SNET_CHT_PORT_MAC_FIELDS_forceLinkDown_E)                                 }
    ,{STR(SNET_CHT_PORT_MAC_FIELDS_forceLinkPass_E)                                 }
    ,{STR(SNET_CHT_PORT_MAC_FIELDS_LPI_request_enable_E)                            }
    ,{STR(SNET_CHT_PORT_MAC_FIELDS_portInLoopBack_E)                                }
    ,{STR(SNET_CHT_PORT_MAC_FIELDS_int_bit_for_PCS_Tx_Path_Received_LPI_E)          }
    ,{STR(SNET_CHT_PORT_MAC_FIELDS_int_bit_for_PCS_Rx_Path_Received_LPI_E)          }
    ,{STR(SNET_CHT_PORT_MAC_FIELDS_int_bit_for_MAC_Rx_Path_Received_LPI_E)          }
    ,{STR(SNET_CHT_PORT_MAC_FIELDS_Unidirectional_enable_support_802_3ah_OAM_E)     }
    ,{STR(SNET_CHT_PORT_MAC_FIELDS_PaddingDis_E)                                    }
    ,{STR(SNET_CHT_PORT_MAC_FIELDS_histogram_rx_en_E)                               }
    ,{STR(SNET_CHT_PORT_MAC_FIELDS_histogram_tx_en_E)                               }
    ,{STR(SNET_CHT_PORT_MAC_FIELDS_mib_4_count_hist_E)                              }
    ,{STR(SNET_CHT_PORT_MAC_FIELDS_mib_4_limit_1518_1522_E)                         }
    ,{STR(SNET_CHT_PORT_MAC_FIELDS_Port_Clear_After_Read_E)                         }
    ,{STR(SNET_CHT_PORT_MAC_FIELDS_rx_path_en_E)                                    }
    ,{STR(SNET_CHT_PORT_MAC_FIELDS_tx_path_en_E)                                    }
    ,{STR(SNET_CHT_PORT_MAC_FIELDS_open_drain_E)                                    }
    ,{STR(SNET_CHT_PORT_MAC_FIELDS_open_drain_counter_increment_E)                  }
    ,{STR(SNET_CHT_PORT_MAC_FIELDS_LMU_enable_E)                                    }
    ,{STR(SNET_CHT_PORT_MAC_FIELDS_is_mti_segmented_E)                              }
    ,{STR(SNET_CHT_PORT_MAC_FIELDS_mti_mac_addr_0_E  )                              }
    ,{STR(SNET_CHT_PORT_MAC_FIELDS_mti_mac_addr_1_E  )                              }
    ,{STR(SNET_CHT_PORT_MAC_FIELDS_mti_mib_counter_is_emac_pmac_shared_E)           }
    ,{STR(SNET_CHT_PORT_MAC_FIELDS_forwardFcPacketsEnable_E)                        }
    ,{STR(SNET_CHT_PORT_MAC_FIELDS_clock_Enable_E)                                  }
    ,{STR(SNET_CHT_PORT_MAC_FIELDS_segmented_E)                                     }
    ,{STR(SNET_CHT_PORT_MAC_FIELDS_fec_RS_FEC_Enable_Lane_E)                        }
    ,{STR(SNET_CHT_PORT_MAC_FIELDS_fec_KP_Mode_Enable_E)                            }
    ,{STR(SNET_CHT_PORT_MAC_FIELDS_sgmii_speed_E       )                            }

};
    #define MTI_CPU_PORT_GLOBAL(dev,_cpu_port_index)\
        SMEM_CHT_MAC_REG_DB_SIP5_GET(dev)->sip6_MTI_cpu_port[_cpu_port_index].MTI_GLOBAL
    #define MTI_CPU_PORT_MAC(dev,_cpu_port_index)\
        SMEM_CHT_MAC_REG_DB_SIP5_GET(dev)->sip6_MTI_cpu_port[_cpu_port_index].MTI_cpuMac
    #define MTI_CPU_PORT_PCS(dev,_cpu_port_index)\
        SMEM_CHT_MAC_REG_DB_SIP5_GET(dev)->sip6_MTI_cpu_port[_cpu_port_index].MTI_cpuPcs
    #define MTI_CPU_PORT_EXT_STAT(dev,_cpu_port_index)\
        SMEM_CHT_MAC_REG_DB_SIP5_GET(dev)->sip6_MTI_cpu_port[_cpu_port_index].cpu_MTI_STATS
    #define MTI_CPU_PORT_MPFS(dev,_cpu_port_index)\
        SMEM_CHT_MAC_REG_DB_SIP5_GET(dev)->sip6_MTI_cpu_port[_cpu_port_index].MPFS_CPU

    #define MTI_PORT_GLOBAL(dev,_representativePort)\
        SMEM_CHT_MAC_REG_DB_SIP5_GET(dev)->sip6_MTI_EXTERNAL[_representativePort].MTI_GLOBAL
    #define MTI_PORT_WRAP(dev,_macPort)\
        SMEM_CHT_MAC_REG_DB_SIP5_GET(dev)->sip6_MTI[_macPort].MTI_WRAPPER
    #define MTI_PORT_EXT(dev,_macPort)\
        SMEM_CHT_MAC_REG_DB_SIP5_GET(dev)->sip6_MTI[_macPort].MTI_EXT_PORT
    #define MTI_PORT_GLOBAL_STAT(dev,_representativePort)\
        SMEM_CHT_MAC_REG_DB_SIP5_GET(dev)->sip6_MTI_EXTERNAL[_representativePort].MTI_STATS
    #define MTI_PORT_MPFS(dev,_macPort)\
        SMEM_CHT_MAC_REG_DB_SIP5_GET(dev)->sip6_MTI[_macPort].MPFS

    #define MTI_HIGH_SPEED_PORT_MPFS(dev,_macPort)    \
        SMEM_CHT_MAC_REG_DB_SIP5_GET(dev)->sip6_high_speed[_macPort].MPFS
    #define MTI_HIGH_SPEED_PORT_PCS(dev,_macPort)    \
        SMEM_CHT_MAC_REG_DB_SIP5_GET(dev)->sip6_high_speed[_macPort].PCS_400G_200G
    #define MTI_HIGH_SPEED_PORT_MAC(dev,_macPort)    \
        SMEM_CHT_MAC_REG_DB_SIP5_GET(dev)->sip6_high_speed[_macPort].MTI400_MAC

/**
* @internal internal_snetFalconPortMacFieldGet function
* @endinternal
*
* @brief   set/get the field value to/from the MAC according to its current 'MAC' used
*/
static GT_U32    internal_snetFalconPortMacFieldGet
(
    IN SKERNEL_DEVICE_OBJECT            * devObjPtr,
    IN GT_U32                           macPort,
    IN SNET_CHT_PORT_MAC_FIELDS_ENT     fieldType,
    IN GT_BOOL                          updateField,
    IN GT_U32                           newValue
)
{
    DECLARE_FUNC_NAME(internal_snetFalconPortMacFieldGet);

    GT_CHAR* fieldName;
    GT_U32  regAddr, regValue;
    GT_U32  startBit, numBits;
    GT_U32  globalRavenNum,localRavenInPipe,localChannelGroup;
    GT_U32  isCpuPort;
    GT_U32  invert , invertValue;
    GT_U32  increment;
    GT_U32  use_highSpeedIndex,use_highSpeedPcs=0;
    GT_U32  compareValue;
    GT_U32  cpuPortIndex;
    GT_U32  is800GOptional=0, is400GOptional,is200GOptional_1,is200GOptional_2;
    GT_U32  is50GOptional;
    GT_U32  is800GActive = 0;
    GT_U32  isUsx = 0;
    GT_U32  localPortInGop;
    SMEM_UNIT_TYPE_ENT     unitType;
    ENHANCED_PORT_INFO_STC portInfo;
    ENHANCED_PORT_INFO_STC lpcsPortInfo;
    GT_U32  sip6_MTI_EXTERNAL_representativePort;
    GT_U32  isPreemptivePort=0;

    fieldName = fieldsInMacDbArr[fieldType].name;

    regAddr = 0;
    regValue = 0;
    startBit = 0;
    numBits = 1;/* default : single bit */
    invert = 0;
    increment = 0;
    compareValue = 0xFFFFFFFF;/* unused */

    if(devObjPtr->numOfRavens == 0)/*Hawk*/
    {
        is400GOptional   = devObjPtr->portsArr[macPort].state == SKERNEL_PORT_STATE_MTI_400_E;
        is200GOptional_1 = devObjPtr->portsArr[macPort].state == SKERNEL_PORT_STATE_MTI_400_E ||
                           devObjPtr->portsArr[macPort].state == SKERNEL_PORT_STATE_MTI_200_E;
        is200GOptional_2 = devObjPtr->portsArr[macPort].state == SKERNEL_PORT_STATE_MTI_200_E;
        isCpuPort        = devObjPtr->portsArr[macPort].state == SKERNEL_PORT_STATE_MTI_CPU_E;
        is50GOptional    = devObjPtr->portsArr[macPort].state == SKERNEL_PORT_STATE_MTI_50_E;
        isUsx            = devObjPtr->portsArr[macPort].state == SKERNEL_PORT_STATE_MTI_USX_E;

        unitType = SMEM_MTI_UNIT_TYPE_GET(devObjPtr,macPort);

        devObjPtr->devMemPortInfoGetPtr(devObjPtr,unitType  ,macPort,&portInfo);

        localPortInGop    = portInfo.simplePortInfo.indexInUnit;
        localChannelGroup = 0;/* calculated for LMU when needed ! */
        cpuPortIndex      = portInfo.simplePortInfo.unitIndex;
        localRavenInPipe  = 0;/*ignored anyway*/

        if(localPortInGop == 0)
        {
            is200GOptional_2 = 0;
        }
        else if(localPortInGop == 4)
        {
            is200GOptional_1 = 0;
        }

        sip6_MTI_EXTERNAL_representativePort = portInfo.sip6_MTI_EXTERNAL_representativePortIndex;

        if (is50GOptional && (portInfo.simplePortInfo.unitIndex == 0))
        {
            /* Phoenix only (not Falcon , not Hawk)*/
            regAddr = MTI_PORT_GLOBAL(devObjPtr,sip6_MTI_EXTERNAL_representativePort).globalChannelControl;
            startBit = 2;/*<CG Enable channel 0>*/
            numBits  = 1;
            smemRegFldGet(devObjPtr, regAddr, startBit, numBits, &regValue);
            use_highSpeedPcs = regValue;
        }

        /* currently only Hawk ports may get 'GT_TRUE' on it */
        if(devObjPtr->portsArr[macPort].portSupportPreemption == GT_TRUE)
        {
            isPreemptivePort = smemGetCurrentTaskExtParamValue(devObjPtr,TASK_EXT_PARAM_INDEX_IS_PREEMPTIVE_CHANNEL);

#if 0
            isPreemptivePort = 1;
#endif
        }

    }
    else
    {
        /* Convert physical port to Raven port, channel and die indexes */
        smemRavenGlobalPortConvertGet(devObjPtr,macPort,&globalRavenNum,&localRavenInPipe,NULL,&localPortInGop,&localChannelGroup,&isCpuPort);
        cpuPortIndex = globalRavenNum;

        if(isCpuPort)
        {
            is400GOptional   = 0;
            is200GOptional_1 = 0;
            is200GOptional_2 = 0;
            is50GOptional    = 0;
        }
        else
        {
            is800GOptional   =  (macPort & 0xF) == 0;
            is400GOptional   =  (macPort & 0x7) == 0;
            is200GOptional_1 =  is400GOptional;
            is200GOptional_2 =  (macPort & 0x7) == 4;

            is50GOptional    = (macPort & 1) == 1;
        }

        sip6_MTI_EXTERNAL_representativePort = macPort / 8;
    }



    if(isCpuPort || !(is200GOptional_1 || is200GOptional_2))/* CPU or not 0/4/8/12 in the Raven */
    {
        use_highSpeedIndex = 0;
        if(fieldType == SNET_CHT_PORT_MAC_FIELDS_segmented_E)
        {
            return use_highSpeedIndex;
        }
    }
    else
    {
        /*
            /Cider/Switching Dies/Raven A0/Raven_A0 {Current}/Device Units/<GOP TAP 0>GOP TAP 0/
            <GOP TAP 0> <MTIP IP WRAPPER>MTIP IP WRAPPER/<MTIP EXT> MTIP EXT/
            MTIP EXT Units %j/Global Channel Control


            Controls per 4 channels if used by "segmented" (200/400G) or "non-segmented" (10M-100G).
            gc_pcs400_ena_in[1:0] = 2'b00 - all channels 0-7 according to Quad configuration.
            gc_pcs400_ena_in[1:0] = 2'b01 - 200G-R4 on channel 0, channel 4-7 according to Quad configuration.
            gc_pcs400_ena_in[1:0] = 2'b10 - 200G-R4 on channel 4, channel 0-3 according to Quad configuration.
            gc_pcs400_ena_in[1:0] = 2'b11 - 400G-R8 , 200G-R8 or 2x200G-R4.
        */

        if (devObjPtr->deviceFamily == SKERNEL_AAS_FAMILY)
        {
            ENHANCED_PORT_INFO_STC tempPortInfo;
            GT_U32 mode800G_regValue = 0;
            devObjPtr->devMemPortInfoGetPtr(devObjPtr, SMEM_UNIT_TYPE_MTI_PCS_50G_E  ,macPort, &tempPortInfo);
            sip6_MTI_EXTERNAL_representativePort = tempPortInfo.sip6_MTI_EXTERNAL_representativePortIndex;
            regAddr = MTI_PORT_GLOBAL(devObjPtr,sip6_MTI_EXTERNAL_representativePort).globalChannelControl;
            startBit = 0;
            numBits  = 4;
            smemRegFldGet(devObjPtr, regAddr, startBit, numBits, &regValue);
            smemRegFldGet(devObjPtr, regAddr, 8, 1, &mode800G_regValue);
            if(mode800G_regValue == 0x1)
            {
                is800GActive = 1;
            }
            else
            {
                if(portInfo.simplePortInfo.unitIndex % 2)
                {
                    /*MACs 8..15*/
                    regValue = regValue >> 2;
                }
                else
                {
                    /*MACs 0..7*/
                    regValue &= 0x3;
                }
            }
        }
        else
        {
            /*Quad Modes*/
            regAddr = MTI_PORT_GLOBAL(devObjPtr,sip6_MTI_EXTERNAL_representativePort).globalChannelControl;
            startBit = 6;
            numBits  = 2;
            smemRegFldGet(devObjPtr, regAddr, startBit, numBits, &regValue);
        }

        use_highSpeedIndex = 0;
        if(regValue == 1 && is200GOptional_1)/*200G-R4 on channel 0, channel 4-7 according to Quad configuration.*/
        {
            use_highSpeedIndex = 1;
        }
        else
        if(regValue == 2 && is200GOptional_2)/*200G-R4 on channel 4, channel 0-3 according to Quad configuration.*/
        {
            use_highSpeedIndex = 1;
        }
        else
        if(regValue == 3 /*&& (0 == (macPort & 0x3))*/)/*400G-R8 , 200G-R8 or 2x200G-R4.*/
        {
            use_highSpeedIndex = 1;
        }
        else
        if((is800GActive == 1) && (is800GOptional))
        {
            use_highSpeedIndex = 1;
        }

        if(fieldType == SNET_CHT_PORT_MAC_FIELDS_segmented_E)
        {
            if(updateField)
            {
                if(newValue != use_highSpeedIndex)
                {
                    if(newValue)
                    {
                        switch(regValue)
                        {
                            case 0:
                                regValue = is200GOptional_1 ? 1 : 2;
                                break;
                            default:
                                regValue = 3;
                                break;
                        }
                    }
                    else
                    {
                        switch(regValue)
                        {
                            case 3:
                                regValue = is200GOptional_1 ? 2 : 1;
                                break;
                            default:
                                regValue = 0;
                                break;
                        }
                    }

                    /* need to update the value */
                    smemRegFldSet(devObjPtr, regAddr, startBit, numBits, regValue);
                }
            }

            return use_highSpeedIndex;
        }

        if(use_highSpeedIndex)
        {
            __LOG(("MAC [%d] is high speed port ! \n",macPort));
        }
    }

    switch(fieldType)
    {
        case SNET_CHT_PORT_MAC_FIELDS_forwardFcPacketsEnable_E:
            startBit = 7;
            numBits  = 1;
            if(use_highSpeedIndex)
            {
                regAddr = MTI_HIGH_SPEED_PORT_MAC(devObjPtr,macPort).commandConfig;
            }
            else
            if(isCpuPort)
            {
                regAddr = MTI_CPU_PORT_MAC(devObjPtr,cpuPortIndex).commandConfig;
            }
            else
            if(isPreemptivePort)
            {
                regAddr = MTI_PORT_WRAP(devObjPtr,macPort).MTI_PREEMPTION_MAC.commandConfig;
            }
            else
            {
                regAddr = MTI_PORT_WRAP(devObjPtr,macPort).MTI64_MAC.commandConfig;
            }
            break;
        case SNET_CHT_PORT_MAC_FIELDS_mti_mac_addr_0_E:
            startBit = 0;
            numBits  = 32;
            if(use_highSpeedIndex)
            {
                regAddr = MTI_HIGH_SPEED_PORT_MAC(devObjPtr,macPort).macAddr0;
            }
            else
            if(isCpuPort)
            {
                regAddr = MTI_CPU_PORT_MAC(devObjPtr,cpuPortIndex).macAddr0;
            }
            else
            if(isPreemptivePort)
            {
                regAddr = MTI_PORT_WRAP(devObjPtr,macPort).MTI_PREEMPTION_MAC.macAddr0;
            }
            else
            {
                regAddr = MTI_PORT_WRAP(devObjPtr,macPort).MTI64_MAC.macAddr0;
            }
        break;
        case SNET_CHT_PORT_MAC_FIELDS_mti_mac_addr_1_E:
            startBit = 0;
            numBits  = 16;
            if(isPreemptivePort)
            {
                regAddr = MTI_PORT_WRAP(devObjPtr,macPort).MTI_PREEMPTION_MAC.macAddr1;
            }
            else
            {
                regAddr = MTI_PORT_WRAP(devObjPtr,macPort).MTI64_MAC.macAddr1;
            }
        break;

        case SNET_CHT_PORT_MAC_FIELDS_mti_mib_counter_is_emac_pmac_shared_E:
            if(!isPreemptivePort)
            {
                __LOG(("snetFalconPortMacFieldGet: for global macPort[%d] fieldType[%s] not in preemptive mode (treated as value[1])\n",
                    macPort,
                    fieldName));
                return 1;
            }

            regAddr  = MTI_PORT_WRAP(devObjPtr,macPort)._802_3_BR.brControl;
            startBit = 24;/*<PMAC_TO_EMAC_STATS>*/

            break;

        case SNET_CHT_PORT_MAC_FIELDS_open_drain_E:
            if(SMEM_CHT_IS_SIP6_10_GET(devObjPtr))
            {
                /* the SAU not exists ! */
                __LOG(("snetFalconPortMacFieldGet: for global macPort[%d] fieldType[%s] not supported by the MAC !!! (treated as value[0])\n",
                    macPort,
                    fieldName));
                return 0;
            }
            startBit = 30;
            if(use_highSpeedIndex)
            {
                regAddr = MTI_HIGH_SPEED_PORT_MPFS(devObjPtr,macPort).SAUControl;
            }
            else
            if(isCpuPort)
            {
                regAddr = MTI_CPU_PORT_MPFS(devObjPtr,cpuPortIndex).SAUControl;
            }
            else
            {
                regAddr = MTI_PORT_MPFS(devObjPtr,macPort).SAUControl;
            }
            numBits = 2;

            compareValue = 0x1;/* the 2 bits must be value 1 (not 0/2/3) */
            break;

        case SNET_CHT_PORT_MAC_FIELDS_open_drain_counter_increment_E:
            if(SMEM_CHT_IS_SIP6_10_GET(devObjPtr))
            {
                /* the SAU not exists ! */
                __LOG(("snetFalconPortMacFieldGet: for global macPort[%d] fieldType[%s] not supported by the MAC !!! (treated as value[0])\n",
                    macPort,
                    fieldName));
                return 0;
            }
            startBit = 0;
            if(use_highSpeedIndex)
            {
                regAddr = MTI_HIGH_SPEED_PORT_MPFS(devObjPtr,macPort).SAUStatus;
            }
            else
            if(isCpuPort)
            {
                regAddr = MTI_CPU_PORT_MPFS(devObjPtr,cpuPortIndex).SAUStatus;
            }
            else
            {
                regAddr = MTI_PORT_MPFS(devObjPtr,macPort).SAUStatus;
            }
            numBits = 32;
            increment = 1;
            break;

        case SNET_CHT_PORT_MAC_FIELDS_PortMACReset_E:
            if(isCpuPort)
            {
                regAddr = MTI_CPU_PORT_GLOBAL(devObjPtr,cpuPortIndex).globalResetControl;
                startBit = 1;/* Rx direction */
            }
            else
            {
                regAddr = MTI_PORT_GLOBAL(devObjPtr,sip6_MTI_EXTERNAL_representativePort).globalResetControl;
                startBit = localPortInGop; /* Rx direction */
            }
            numBits = 1;

            break;
        case SNET_CHT_PORT_MAC_FIELDS_fec_RS_FEC_Enable_Lane_E:
            regAddr = MTI_PORT_GLOBAL(devObjPtr,sip6_MTI_EXTERNAL_representativePort).globalFECControl;
            startBit = 16 + localPortInGop;
            numBits  = newValue & 0xFFFF;/* the value hold : {value , numBits} */
            newValue = newValue >> 16;
            break;
        case SNET_CHT_PORT_MAC_FIELDS_fec_KP_Mode_Enable_E:
            regAddr = MTI_PORT_GLOBAL(devObjPtr,sip6_MTI_EXTERNAL_representativePort).globalFECControl;
            startBit = 24 + localPortInGop;
            numBits  = newValue & 0xFFFF;/* the value hold : {value , numBits} */
            newValue = newValue >> 16;
            break;
        case SNET_CHT_PORT_MAC_FIELDS_sgmii_speed_E:
            if(updateField == GT_TRUE)
            {
                if (devObjPtr->deviceFamily == SKERNEL_AAS_FAMILY)
                {
                    GT_U32 LPCS_representativePortIndex;

                    devObjPtr->devMemPortInfoGetPtr(devObjPtr, SMEM_UNIT_TYPE_MTI_PCS_50G_E, macPort, &lpcsPortInfo);
                    LPCS_representativePortIndex = lpcsPortInfo.sip6_MTI_EXTERNAL_representativePortIndex*2;
                    if((macPort / 8) > 0)
                    {
                        LPCS_representativePortIndex++;
                    }
                    regAddr = SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_EXTERNAL[LPCS_representativePortIndex].PCS_common.LPCS_common.gmode;
                    regValue = 1;/* using lspcs */
                    smemRegFldSet(devObjPtr, regAddr, (macPort%8) , 1, regValue);
                }
                else
                {
                    regAddr = SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_EXTERNAL[sip6_MTI_EXTERNAL_representativePort].PCS_common.LPCS_common.gmode;
                    regValue = 1;/* using lspcs */
                    smemRegFldSet(devObjPtr, regAddr, localPortInGop , 1, regValue);
                }
            }
            regAddr = MTI_PORT_WRAP(devObjPtr, macPort).PCS_LPCS.usxgmii_rep;
            numBits  = 10;
            startBit = 0;
            newValue = newValue == 1000 ? 1 /*1000M*/ : 0/*10M*/;
            break;
        case SNET_CHT_PORT_MAC_FIELDS_clock_Enable_E:
            if(use_highSpeedIndex)
            {
                startBit = 8 + (localPortInGop/4)/*0/1*/;
            }
            else
            {
                startBit = localPortInGop;
            }
            regAddr = MTI_PORT_GLOBAL(devObjPtr,sip6_MTI_EXTERNAL_representativePort).globalClockEnable;
            numBits = 1;

            break;

        case SNET_CHT_PORT_MAC_FIELDS_portInLoopBack_E:
            startBit = 14;
            numBits = 1;
            if(use_highSpeedIndex || use_highSpeedPcs)
            {
                regAddr = MTI_HIGH_SPEED_PORT_PCS(devObjPtr,macPort).control1;
            }
            else
            if (isCpuPort)
            {
                if(devObjPtr->deviceFamily == SKERNEL_IRONMAN_FAMILY)
                {
                    startBit = 6;
                    numBits = 1;
                    regAddr = MTI_CPU_PORT_PCS(devObjPtr,cpuPortIndex).extControl;
                }
                else
                {
                    regAddr = MTI_CPU_PORT_PCS(devObjPtr,cpuPortIndex).control1;
                }
            }
            else
            if (is50GOptional)
            {

                if (devObjPtr->deviceFamily == SKERNEL_AAS_FAMILY)
                {
                    GT_U32 LPCS_representativePortIndex;

                    devObjPtr->devMemPortInfoGetPtr(devObjPtr, SMEM_UNIT_TYPE_MTI_PCS_50G_E, macPort, &lpcsPortInfo);
                    LPCS_representativePortIndex = lpcsPortInfo.sip6_MTI_EXTERNAL_representativePortIndex*2;
                    if((macPort / 8) > 0)
                    {
                        LPCS_representativePortIndex++;
                    }
                    regAddr = SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_EXTERNAL[LPCS_representativePortIndex].PCS_common.LPCS_common.gmode;
                    regValue = 1;/* using lspcs */
                    smemRegFldSet(devObjPtr, regAddr, (macPort%8) , 1, regValue);
                }
                else
                {
                    regAddr = SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_EXTERNAL[sip6_MTI_EXTERNAL_representativePort].PCS_common.LPCS_common.gmode;
                    smemRegFldGet(devObjPtr, regAddr, localPortInGop , 1, &regValue);
                }

                if (regValue) /* using lpcs */
                {
                    regAddr = MTI_PORT_WRAP(devObjPtr, macPort).PCS_LPCS.control1;
                }
                else
                {
                    regAddr = MTI_PORT_WRAP(devObjPtr,macPort).PCS_10254050.control1;
                }
            }
            else if(isUsx)
            {
                if(devObjPtr->deviceFamily == SKERNEL_IRONMAN_FAMILY)
                {
                    startBit = 6;
                    numBits = 1;
                    regAddr = MTI_PORT_WRAP(devObjPtr,macPort).USX_PCS.extControl;
                }
                else
                {
                    regAddr = SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_EXTERNAL[sip6_MTI_EXTERNAL_representativePort].PCS_common.LPCS_common.gmode;
                    if (regAddr == 0xffffffff)
                    {
                        regValue = 0;
                    }
                    else
                    {
                        smemRegFldGet(devObjPtr, regAddr, localPortInGop , 1, &regValue);
                        if(regValue == 0)
                        {
                            /* check if <Usgmii8_enable> == 1 --> that means that all 8 ports
                               are using the LSPCS */
                            smemRegFldGet(devObjPtr, regAddr, 10 , 1, &regValue);
                        }
                    }

                    if (regValue) /* using lpcs */
                    {
                        regAddr = MTI_PORT_WRAP(devObjPtr, macPort).USX_LSPCS.control1;
                    }
                    else
                    {
                        regAddr = MTI_PORT_WRAP(devObjPtr,macPort).USX_PCS.control1;
                    }
                }
            }
            else
            {
                if (devObjPtr->deviceFamily == SKERNEL_AAS_FAMILY)
                {
                    GT_U32 LPCS_representativePortIndex;

                    devObjPtr->devMemPortInfoGetPtr(devObjPtr, SMEM_UNIT_TYPE_MTI_PCS_50G_E, macPort, &lpcsPortInfo);
                    LPCS_representativePortIndex = lpcsPortInfo.sip6_MTI_EXTERNAL_representativePortIndex*2;
                    if((macPort / 8) > 0)
                    {
                        LPCS_representativePortIndex++;
                    }
                    regAddr = SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_EXTERNAL[LPCS_representativePortIndex].PCS_common.LPCS_common.gmode;
                    regValue = 1;/* using lspcs */
                    smemRegFldSet(devObjPtr, regAddr, (macPort%8) , 1, regValue);
                }
                else
                {
                    regAddr = SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->sip6_MTI_EXTERNAL[sip6_MTI_EXTERNAL_representativePort].PCS_common.LPCS_common.gmode;
                    smemRegFldGet(devObjPtr, regAddr, localPortInGop , 1, &regValue);
                }

                if (regValue) /* using lpcs */
                {
                    regAddr = MTI_PORT_WRAP(devObjPtr, macPort).PCS_LPCS.control1;
                }
                else
                {
                    regAddr = MTI_PORT_WRAP(devObjPtr,macPort).PCS_10TO100.control1;
                }
            }

            break;
        case SNET_CHT_PORT_MAC_FIELDS_rx_path_en_E:
        case SNET_CHT_PORT_MAC_FIELDS_tx_path_en_E:
            if(use_highSpeedIndex)
            {
                regAddr = MTI_HIGH_SPEED_PORT_MAC(devObjPtr,macPort).commandConfig;
            }
            else
            if(isCpuPort)
            {
                regAddr = MTI_CPU_PORT_MAC(devObjPtr,cpuPortIndex).commandConfig;
            }
            else
            if(isPreemptivePort)
            {
                regAddr = MTI_PORT_WRAP(devObjPtr,macPort).MTI_PREEMPTION_MAC.commandConfig;
            }
            else
            {
                regAddr = MTI_PORT_WRAP(devObjPtr,macPort).MTI64_MAC.commandConfig;
            }

            startBit = (fieldType == SNET_CHT_PORT_MAC_FIELDS_rx_path_en_E) ? 1 : 0;
            numBits = 1;

            break;
        case SNET_CHT_PORT_MAC_FIELDS_LinkState_E:
            startBit = 0;
            numBits = 1;

            if((updateField == GT_TRUE) && !(SMEM_CHT_IS_SIP7_GET(devObjPtr)))
            {
                /* set 2 bits the same :
                bit 0 : <link_ok>
                bit 1 : <link_status>
                */
                numBits  = 2;
                newValue = newValue ? 3 : 0;

                /* NOTE: on 'get' we check only bit 0 */
            }

            if(use_highSpeedIndex)
            {
                regAddr = MTI_PORT_EXT(devObjPtr,macPort).segPortStatus;

                /* AAS: {link_status},{lpcs_link_status} fields update */
                if ((SMEM_CHT_IS_SIP7_GET(devObjPtr)) && (updateField == GT_TRUE))
                {
                    GT_U32 regAddr1;
                    GT_U32 newValue1;

                    newValue1 = newValue ? 3 : 0;
                    regAddr1 = MTI_PCS_PORT_EXT(devObjPtr,macPort).portStatus;
                    smemRegFldSet(devObjPtr, regAddr1, 0, 2, newValue1);

                }
            }
            else
            if(isCpuPort)
            {
                regAddr = MTI_CPU_PORT_EXT(devObjPtr,cpuPortIndex).portStatus;
            }
            else
            {
                if((updateField == GT_TRUE) && !(SMEM_CHT_IS_SIP7_GET(devObjPtr)))
                {
                    /* set 3 bits the same :
                    bit 0 : <link_ok>
                    bit 1 : <link_status>
                    bit 2 : <lpcs_link_status>
                    bit 3 : <lpcs_rx_sync>
                    */
                    numBits  = 4;
                    newValue = newValue ? 15 : 0;

                    /* NOTE: on 'get' we check only bit 0 */
                }

                /* AAS: {link_status},{lpcs_link_status} fields update */
                if ((SMEM_CHT_IS_SIP7_GET(devObjPtr)) && (updateField == GT_TRUE))
                {
                    GT_U32 regAddr1;
                    GT_U32 newValue1;

                    newValue1 = newValue ? 3 : 0;
                    regAddr1 = MTI_PCS_PORT_EXT(devObjPtr,macPort).portStatus;
                    smemRegFldSet(devObjPtr, regAddr1, 0, 2, newValue1);

                }
                if (devObjPtr->deviceFamily == SKERNEL_HAWK_FAMILY)
                {
                    if (isUsx)
                    {
                        regAddr = USX_PORT_EXT(devObjPtr, macPort).portStatus;
                    }
                    else
                    {
                        regAddr = MTI_PORT_EXT(devObjPtr, macPort).portStatus;
                    }
                }
                else
                {
                    regAddr = MTI_PORT_EXT(devObjPtr, macPort).portStatus;
                }
            }

            break;
        case SNET_CHT_PORT_MAC_FIELDS_mru_E:
            if(use_highSpeedIndex)
            {
                regAddr = MTI_HIGH_SPEED_PORT_MAC(devObjPtr,macPort).frmLength;
            }
            else
            if(isCpuPort)
            {
                regAddr = MTI_CPU_PORT_MAC(devObjPtr,cpuPortIndex).frmLength;
            }
            else
            if(isPreemptivePort)
            {
                regAddr = MTI_PORT_WRAP(devObjPtr,macPort).MTI_PREEMPTION_MAC.frmLength;
            }
            else
            {
                regAddr = MTI_PORT_WRAP(devObjPtr,macPort).MTI64_MAC.frmLength;
            }
            startBit = 0;
            numBits = 16;

            break;
        case SNET_CHT_PORT_MAC_FIELDS_Port_Clear_After_Read_E:
            if(isCpuPort)
            {
                regAddr = MTI_CPU_PORT_EXT_STAT(devObjPtr,cpuPortIndex).config;
            }
            else
            {
                regAddr = MTI_PORT_GLOBAL_STAT(devObjPtr,sip6_MTI_EXTERNAL_representativePort).config;
            }
            startBit = 1;
            numBits = 1;

            break;

        case SNET_CHT_PORT_MAC_FIELDS_LMU_enable_E:
            if(SMEM_CHT_IS_SIP6_30_GET(devObjPtr) && devObjPtr->numOfLmus == 0)
            {
                __LOG(("The device not supports Latency monitoring (LMU) processing \n"));
                return 0;
            }
            else
            if(devObjPtr->numOfRavens == 0)/*Hawk*/
            {
                devObjPtr->devMemPortInfoGetPtr(devObjPtr,SMEM_UNIT_TYPE_PCA_LMU_CHANNEL_E  ,macPort,&portInfo);

                localChannelGroup = portInfo.simplePortInfo.unitIndex;
                startBit          = portInfo.simplePortInfo.indexInUnit;
            }
            else
            {
                if(isCpuPort)
                {
                    __LOG(("LMU for 'network CPU port' : global macPort[%d] fieldType[%s] not supported by the MAC !!! (treated as value[0])\n",
                        macPort,
                        fieldName));
                    return 0;
                }

                if(use_highSpeedIndex)
                {

                    if(is200GOptional_1)
                    {
                        startBit = FALCON_PORTS_PER_DP;/*8*/
                    }
                    else
                    {
                        startBit = 1 + FALCON_PORTS_PER_DP;/*9*/
                    }
                }
                else
                {

                    startBit = localPortInGop; /*0..7*/
                }
            }

            numBits = 1;
            regAddr = SMEM_SIP6_LMU_CHANNEL_ENABLE_REG(devObjPtr, localRavenInPipe , localChannelGroup);
            break;

        case SNET_CHT_PORT_MAC_FIELDS_is_mti_segmented_E:
            return use_highSpeedIndex;

        case SNET_CHT_PORT_MAC_FIELDS_histogram_rx_en_E:
        case SNET_CHT_PORT_MAC_FIELDS_histogram_tx_en_E:
            return 1;

        case SNET_CHT_PORT_MAC_FIELDS_PortEn_E:
        case SNET_CHT_PORT_MAC_FIELDS_int_bit_for_PCS_Tx_Path_Received_LPI_E:
        case SNET_CHT_PORT_MAC_FIELDS_int_bit_for_PCS_Rx_Path_Received_LPI_E:
        case SNET_CHT_PORT_MAC_FIELDS_int_bit_for_MAC_Rx_Path_Received_LPI_E:
        case SNET_CHT_PORT_MAC_FIELDS_Unidirectional_enable_support_802_3ah_OAM_E:
        case SNET_CHT_PORT_MAC_FIELDS_PaddingDis_E:
        case SNET_CHT_PORT_MAC_FIELDS_MIBCntEn_E:
        case SNET_CHT_PORT_MAC_FIELDS_forceLinkDown_E:
        case SNET_CHT_PORT_MAC_FIELDS_forceLinkPass_E:
            __LOG(("snetFalconPortMacFieldGet: for global macPort[%d] fieldType[%s] not supported by the MAC !!! (treated as value[0])\n",
                macPort,
                fieldName));
            return 0;
        default:
            skernelFatalError("snetChtPortMacFieldGet: fieldType[%s] not supported \n",
                fieldName);
    }

    if(updateField == GT_FALSE)
    {
        smemRegFldGet(devObjPtr, regAddr, startBit, numBits, &regValue);

        if(invert)
        {
            invertValue = (1 << numBits) - 1;
            regValue = invertValue - regValue;
        }

        if(compareValue != 0xFFFFFFFF &&
           compareValue != regValue)
        {
            /* the value not match the 'enabled' value */
            regValue = 0;
        }


        if(devObjPtr->portsArr[macPort].portSupportPreemption == GT_TRUE)
        {
            __LOG(("global macPort[%d] <%s> hold value [0x%x] (use [%s])\n",
                macPort,
                fieldName,
                regValue,
                isPreemptivePort ? "PMAC" : "EMAC"));
        }
        else
        {
            __LOG(("global macPort[%d] <%s> hold value [0x%x] \n",
                macPort,
                fieldName,
                regValue));
        }

        return regValue;
    }
    else
    {
        if(invert)
        {
            invertValue = (1 << numBits) - 1;
            newValue = invertValue - newValue;
        }

        if(increment)
        {
            smemRegFldGet(devObjPtr, regAddr, startBit, numBits, &newValue);
            newValue = newValue + 1;
        }

        smemRegFldSet(devObjPtr, regAddr, startBit, numBits, newValue);

        if(devObjPtr->portsArr[macPort].portSupportPreemption == GT_TRUE)
        {
            __LOG(("<%s> updated with new value [%d] for global macPort[%d] (use [%s])\n",
                fieldName,
                newValue,
                macPort,
                isPreemptivePort ? "PMAC" : "EMAC"));
        }
        else
        {
            __LOG(("<%s> updated with new value [%d] for global macPort[%d] \n",
                fieldName,
                newValue,
                macPort));
        }

        return 0;
    }
}

/**
* @internal internal_snetChtPortMacFieldGet function
* @endinternal
*
* @brief   set/get the field value to/from the MAC according to its current 'MAC' used
*/
static GT_U32    internal_snetChtPortMacFieldGet
(
    IN SKERNEL_DEVICE_OBJECT            * devObjPtr,
    IN GT_U32                           macPort,
    IN SNET_CHT_PORT_MAC_FIELDS_ENT     fieldType,
    IN GT_BOOL                          updateField,
    IN GT_U32                           newValue
)
{
    DECLARE_FUNC_NAME(internal_snetChtPortMacFieldGet);
    GT_CHAR* fieldName;
    GT_BIT  isCg = CHT_IS_100G_MAC_USED_MAC(devObjPtr,macPort);
    GT_BIT  isXg = isCg ? 0 : IS_CHT_HYPER_GIGA_PORT(devObjPtr,macPort);

    GT_U32  regAddr,value,value1=0,factor=1;
    GT_U32  startBit,numBits;
    GT_BIT  invert = 0;
    GT_U32  cgRegAddr,cgStartBit,cgNmBits;
    GT_U32  cgFinalValue,portFinalValue;
    GT_U32  isXgCounters;
    GT_U32  isGigaControlCounters;/* indication that the Gig mac hold MIB control register */

    if(fieldType >= SNET_CHT_PORT_MAC_FIELDS____LAST____E)
    {
        skernelFatalError("snetChtPortMacFieldGet: fieldType[%d] out of range \n",
            fieldType);
        return 0;
    }

    fieldName = fieldsInMacDbArr[fieldType].name;

    startBit = 0;
    regAddr = 0;
    numBits = 1;
    cgNmBits = 1;
    cgRegAddr = 0;
    cgStartBit = 0;
    cgFinalValue = 0;
    portFinalValue = 0;

    isXgCounters = CHT_MAC_PORT_MIB_COUNTERS_IS_XG_LOGIC_MAC(devObjPtr,macPort);
    isGigaControlCounters = 0;

    if((!isCg) && (!isXg) && /* GIGA port */
       devObjPtr->isMsmGigPortOnAllPorts)
    {
        /* use GIG mac MIB control register */
        isGigaControlCounters = 1;
    }


    switch(fieldType)
    {
        case SNET_CHT_PORT_MAC_FIELDS_PortEn_E:
            startBit = 0;
            regAddr = SMEM_CHT_MAC_CONTROL0_REG(devObjPtr,macPort);

            cgRegAddr = SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP.ports_100G_CG_MAC[macPort].CG_CONVERTERS.CGMAControl0;
            cgStartBit = 20;
            break;
        case SNET_CHT_PORT_MAC_FIELDS_PortMACReset_E:
            invert   = (isXg || isCg) ? 1 : 0;
            startBit = (isXg ) ? 1 : 6;
            regAddr  = (isXg ) ? SMEM_CHT_MAC_CONTROL0_REG(devObjPtr,macPort):
                                 SMEM_CHT_MAC_CONTROL2_REG(devObjPtr,macPort);

            cgRegAddr = SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP.ports_100G_CG_MAC[macPort].CG_CONVERTERS.CGMAConvertersResets;
            cgStartBit = 28;
            break;
        case SNET_CHT_PORT_MAC_FIELDS_LinkState_E:
            startBit = 0;
            regAddr = SMEM_CHT_PORT_STATUS0_REG(devObjPtr,macPort);

            cgRegAddr = SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP.ports_100G_CG_MAC[macPort].CG_CONVERTERS.CGMAConvertersIpStatus;
            cgStartBit = 29;

            break;
        case SNET_CHT_PORT_MAC_FIELDS_mru_E:
            numBits  = 13;
            startBit = isXg ? 0 : 2;
            regAddr  = isXg ? SMEM_CHT_MAC_CONTROL_REG(devObjPtr,macPort) :
                              SMEM_CHT_MAC_CONTROL0_REG(devObjPtr,macPort);
            factor = 2;/* granularity is 2 bytes */

            cgRegAddr = SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP.ports_100G_CG_MAC[macPort].CGPORTMACFrmLength;
            cgStartBit = 0;
            cgNmBits = 16;
            if(isCg) factor = 1; /* granularity is 1 byte */
            break;
        case SNET_CHT_PORT_MAC_FIELDS_MIBCntEn_E:
            invert = isXg;
            regAddr = SMEM_CHT_MAC_CONTROL0_REG(devObjPtr,macPort);
            startBit = 15;
            cgFinalValue = 1;/* The CG not supports disabling of counting */
            break;
        case SNET_CHT_PORT_MAC_FIELDS_forwardPfcFramesEnable_E:
            regAddr = SMEM_XCAT2_PORT_SERIAL_PARAMS1_REG(devObjPtr,macPort);
            startBit = 1;

            cgRegAddr = SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP.ports_100G_CG_MAC[macPort].CG_CONVERTERS.CGMAConvertersFcControl0;
            cgStartBit = 17;
            break;
        case SNET_CHT_PORT_MAC_FIELDS_forwardUnknownMacControlFramesEnable_E:
            regAddr = SMEM_XCAT2_PORT_SERIAL_PARAMS1_REG(devObjPtr,macPort);
            startBit = 2;

            cgRegAddr = SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP.ports_100G_CG_MAC[macPort].CG_CONVERTERS.CGMAConvertersFcControl0;
            cgStartBit = 18;
            break;
        case SNET_CHT_PORT_MAC_FIELDS_forwardFcPacketsEnable_E:
            startBit = isXg ? 5 : 3;
            regAddr = isXg ? SMEM_XCAT_XG_MAC_CONTROL4_REG(devObjPtr, macPort) :
                             SMEM_CHT3_PORT_SERIAL_PARAMETERS_REG(devObjPtr, macPort);
            break;
        case SNET_CHT_PORT_MAC_FIELDS_forceLinkDown_E:
            startBit = isXg ? 2 : 0;
            regAddr = isXg ? SMEM_CHT_MAC_CONTROL0_REG(devObjPtr,macPort) :
                             SMEM_CHT_PORT_AUTO_NEG_REG(devObjPtr,macPort);
            cgRegAddr = SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP.ports_100G_CG_MAC[macPort].CG_CONVERTERS.CGMAControl0;
            cgStartBit = 0;/* tx_loc_fault ... setting this bit to 1 cause the 'link partener'
                              to send us back 'link down' indication */
            if(simulationInitReady)
            {
                /* consider SNET_CHT_PORT_MAC_FIELDS_PortMACReset_E as force link down ... to help generate interrupts */
                value1 = internal_snetChtPortMacFieldGet(devObjPtr,macPort,SNET_CHT_PORT_MAC_FIELDS_PortMACReset_E,GT_FALSE,0);
            }
            break;
        case SNET_CHT_PORT_MAC_FIELDS_forceLinkPass_E:
            startBit = isXg ? 3 : 1;
            regAddr = isXg ? SMEM_CHT_MAC_CONTROL0_REG(devObjPtr,macPort) :
                             SMEM_CHT_PORT_AUTO_NEG_REG(devObjPtr,macPort);
            cgRegAddr = SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP.ports_100G_CG_MAC[macPort].CG_CONVERTERS.CGMAControl0;
            cgStartBit = 27;
            break;
        case SNET_CHT_PORT_MAC_FIELDS_LPI_request_enable_E:
            startBit = 0;
            regAddr = SMEM_SIP_5_MAC_LPI_CTRL_REG(devObjPtr,macPort,1);

            cgRegAddr = SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP.ports_100G_CG_MAC[macPort].CGPORTMACCommandConfig;
            cgStartBit = 23;
            break;
        case SNET_CHT_PORT_MAC_FIELDS_portInLoopBack_E:
            startBit = 0;
            regAddr = SMEM_CHT_MAC_CONTROL_REG(devObjPtr, macPort);
            startBit = isXg ? 13 : 5;
            numBits = 2;

            if (SKERNEL_IS_LION2_DEV(devObjPtr) && isXg)
            {
                /* check PSC MAC Rx Loopback also for Lion2 and above */
                smemRegFldGet(devObjPtr,
                    SMEM_LION2_GOP_MPCS_PCS40G_COMMON_CONTROL_REG(devObjPtr, macPort),
                    3, 1, &value1);
            }

            /* CG mac loopback is supported only in PCS*/
            cgRegAddr = SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP.ports_100G_CG_PCS[macPort].CGPORTPCSControl1;
            cgStartBit = 14;

            break;
        case SNET_CHT_PORT_MAC_FIELDS_int_bit_for_PCS_Tx_Path_Received_LPI_E:
            if(isCg) break;
            return 12;/*exit quietly without LOG info*/
        case SNET_CHT_PORT_MAC_FIELDS_int_bit_for_PCS_Rx_Path_Received_LPI_E:
            if(isCg) break;
            return 11;/*exit quietly without LOG info*/
        case SNET_CHT_PORT_MAC_FIELDS_int_bit_for_MAC_Rx_Path_Received_LPI_E:
            if(isCg) break;
            return 13;/*exit quietly without LOG info*/
        case SNET_CHT_PORT_MAC_FIELDS_Unidirectional_enable_support_802_3ah_OAM_E:
            startBit = isXg ? 8 : 0;
            regAddr = isXg ? SMEM_CHT3_MAC_CONTROL2_REG(devObjPtr, macPort) :
                             SMEM_CHT3_PORT_SERIAL_PARAMETERS_REG(devObjPtr, macPort);
            cgRegAddr = SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->GOP.ports_100G_CG_MAC[macPort].CG_CONVERTERS.CGMAControl0;
            cgStartBit = 31;
            break;
        case SNET_CHT_PORT_MAC_FIELDS_PaddingDis_E:
            regAddr = isXg ? SMEM_CHT_MAC_CONTROL0_REG(devObjPtr, macPort) :
                             SMEM_CHT_MAC_CONTROL2_REG(devObjPtr, macPort);
            startBit = isXg ? 14 : 5;

            cgFinalValue = 0;/* The CG not supports disabling padding ! -- considered 'support pedding' */

            break;
        case SNET_CHT_PORT_MAC_FIELDS_histogram_rx_en_E:
        case SNET_CHT_PORT_MAC_FIELDS_histogram_tx_en_E:
            if(isCg)
            {
                if(fieldType == SNET_CHT_PORT_MAC_FIELDS_histogram_tx_en_E)
                {
                    cgFinalValue = 0;/* The CG not supports Tx Histogram En !
                                        NOT counting on EGRESS !!!
                                        to the 'Histogram counters' */
                }
                else
                {
                    cgFinalValue = 1;/* The CG not supports Rx Histogram En !
                                    considered as enabled*/
                }
            }
            else
            {
                if(isGigaControlCounters)
                {
                    /* GIG MAC register */
                    regAddr = SMEM_SIP5_GIG_PORT_MIB_COUNT_CONTROL_REG(devObjPtr,macPort);
                    startBit = (fieldType == SNET_CHT_PORT_MAC_FIELDS_histogram_tx_en_E) ?
                               3:2;
                }
                else
                if (isXgCounters)
                {
                    /* XG MAC register */
                    regAddr = SMEM_CHT3_HGS_PORT_MIB_COUNT_CONTROL_REG(devObjPtr,macPort);
                    startBit = (fieldType == SNET_CHT_PORT_MAC_FIELDS_histogram_tx_en_E) ?
                               3:2;
                }
                else  /* LMS register : 6 ports in register */
                {
                    /* ports 0..23 */
                    regAddr = SMEM_CHT_MIB_COUNT_CONTROL_REG(devObjPtr, macPort);
                    startBit = (fieldType == SNET_CHT_PORT_MAC_FIELDS_histogram_tx_en_E) ?
                               6:5;
                }
            }
            break;

        case SNET_CHT_PORT_MAC_FIELDS_mib_4_count_hist_E:
        case SNET_CHT_PORT_MAC_FIELDS_mib_4_limit_1518_1522_E:
            if(isCg)
            {
                cgFinalValue = 0;/* The CG not supports those modes */
            }
            else
            if (devObjPtr->support1024to1518MibCounter)
            {
                if(isGigaControlCounters)
                {
                    /* GIG MAC register */
                    regAddr = SMEM_SIP5_GIG_PORT_MIB_COUNT_CONTROL_REG(devObjPtr,macPort);
                    startBit = (fieldType == SNET_CHT_PORT_MAC_FIELDS_mib_4_count_hist_E) ?
                                6:7;
                }
                else
                if (isXgCounters)
                {
                    /* XG MAC register */
                    regAddr = SMEM_CHT3_HGS_PORT_MIB_COUNT_CONTROL_REG(devObjPtr,macPort);
                    startBit = (fieldType == SNET_CHT_PORT_MAC_FIELDS_mib_4_count_hist_E) ?
                            11:12;
                }
                else  /* LMS register : 6 ports in register */
                {
                    /* ports 0..23 */
                    regAddr = SMEM_CHT_MIB_COUNT_CONTROL_REG(devObjPtr, macPort);
                    startBit = (fieldType == SNET_CHT_PORT_MAC_FIELDS_mib_4_count_hist_E) ?
                                16:22;
                    startBit += (macPort % 6);
                }
            }
            else
            {
                regAddr = 0;/* The device not supports those modes */
            }

            break;
        case SNET_CHT_PORT_MAC_FIELDS_Port_Clear_After_Read_E:
            if(isCg)
            {
                cgFinalValue = 0;/* The CG uses different active memory */
            }
            else
            {
                if(isGigaControlCounters)
                {
                    /* GIG MAC register */
                    regAddr = SMEM_SIP5_GIG_PORT_MIB_COUNT_CONTROL_REG(devObjPtr,macPort);
                    startBit = 1;
                }
                else
                if (isXgCounters)
                {
                    regAddr = SMEM_CHT3_HGS_PORT_MIB_COUNT_CONTROL_REG(devObjPtr, macPort);
                    startBit = 1;
                }
                else
                {
                    /* we not implement this case ... as this case already
                       implemented in caller code */
                    regAddr = 0;/* The device not supports those modes */
                }

                invert = 1;
            }

            break;

        case SNET_CHT_PORT_MAC_FIELDS_open_drain_E:
            return 0;/* not open drain */
        case SNET_CHT_PORT_MAC_FIELDS_open_drain_counter_increment_E:
            return 0;/* ignore the action */
        default:
            skernelFatalError("snetChtPortMacFieldGet: fieldType[%s] not supported \n",
                fieldName);
            return 0;
    }

    if(isCg)
    {
        if(cgRegAddr == 0)
        {
            __LOG(("snetChtPortMacFieldGet: for global macPort[%d] fieldType[%s] not supported by the CG MAC !!! (treated as value[%d]) \n",
                macPort,
                fieldName,
                cgFinalValue));

            return cgFinalValue;
        }

        regAddr = cgRegAddr;
        startBit = cgStartBit;
        numBits = cgNmBits;
    }

    if(regAddr == 0)
    {
        __LOG(("snetChtPortMacFieldGet: for global macPort[%d] fieldType[%s] not supported by the MAC !!! (treated as value[%d]) (used [%s] MAC)\n",
            macPort,
            fieldName,
            portFinalValue,
            isXg ? "XG(10G)"  :
                   "Gig(1G)"));

        return portFinalValue;
    }

    if(updateField ==GT_FALSE)
    {
        smemRegFldGet(devObjPtr, regAddr,
                       startBit, numBits, &value);

        if(invert)
        {
            value = 1 - value;
        }

        value += value1;

        value *= factor;

        __LOG(("global macPort[%d] <%s> hold value [0x%x] (used [%s] MAC) \n",
            macPort,
            fieldName,
            value,
            (isCg ? "CG(100G)" :
             isXg ? "XG(10G)"  :
                   "Gig(1G)")));
        return value;
    }
    else
    {
        if(invert)
        {
            newValue = 1 - newValue;
        }

        newValue /= factor;

        smemRegFldSet(devObjPtr, regAddr,
                       startBit, numBits, newValue);
        __LOG(("global macPort[%d] <%s> updated with new value [%d] (used [%s] MAC) \n",
            macPort,
            fieldName,
            newValue,
            (isCg ? "CG(100G)" :
             isXg ? "XG(10G)"  :
                   "Gig(1G)")));

        return 0;
    }
}

/**
* @internal snetChtPortMacFieldGet function
* @endinternal
*
* @brief   get the field value from the MAC according to its current 'MAC' used
*/
GT_U32    snetChtPortMacFieldGet
(
    IN SKERNEL_DEVICE_OBJECT            * devObjPtr,
    IN GT_U32                           macPort,
    IN SNET_CHT_PORT_MAC_FIELDS_ENT     fieldType
)
{
    if (CHT_IS_MTI_MAC_USED_MAC(devObjPtr,macPort))
    {
        return internal_snetFalconPortMacFieldGet(devObjPtr, macPort, fieldType, GT_FALSE,0);
    }
    return internal_snetChtPortMacFieldGet(devObjPtr,macPort,fieldType,GT_FALSE,0);
}
/**
* @internal snetChtPortMacFieldSet function
* @endinternal
*
* @brief   set field value to the MAC according to its current 'MAC' used
*/
void snetChtPortMacFieldSet
(
    IN SKERNEL_DEVICE_OBJECT            * devObjPtr,
    IN GT_U32                           macPort,
    IN SNET_CHT_PORT_MAC_FIELDS_ENT     fieldType,
    IN GT_U32                           value
)
{
    if (CHT_IS_MTI_MAC_USED_MAC(devObjPtr,macPort))
    {
        internal_snetFalconPortMacFieldGet(devObjPtr, macPort, fieldType, GT_TRUE,value);
    }
    else
    {
        internal_snetChtPortMacFieldGet(devObjPtr,macPort,fieldType,GT_TRUE,value);
    }
}

/**
* @internal snetHawkTxDmaToMacConvert function
* @endinternal
*
* @brief   convert (return) global txdma port num to global mac port num
*          using the MIF tx mapping feature
*/
static GT_U32    snetHawkTxDmaToMacConvert
(
    IN SKERNEL_DEVICE_OBJECT            * devObjPtr,
    IN GT_U32                           txDmaGlobalPortNum,
    INOUT   GT_U32                      *nextIndexPtr,
    OUT GT_U32                          *egress_isPreemptiveChannelPtr
)
{
    DECLARE_FUNC_NAME(snetHawkTxDmaToMacConvert);

    GT_U32  ii;
    SKERNEL_PORT_MIF_INFO_STC   *portMifInfoPtr;

    if(nextIndexPtr)
    {
        ii = (*nextIndexPtr);
    }
    else
    {
        ii = 0;
    }

    portMifInfoPtr = &devObjPtr->portsArr[txDmaGlobalPortNum].mifInfo[0];
    for(/*already init*/ ; ii < SKERNEL_PORT_MIF_INFO_MAX_CNS; ii++,portMifInfoPtr++)
    {
        if(!portMifInfoPtr->txEnabled)
        {
            continue;
        }

        if(nextIndexPtr)
        {
            *nextIndexPtr = ii+1;
        }

        __LOG(("MIF : Converted global TxDMA port [%d] to global MAC[%d] \n",
            txDmaGlobalPortNum , portMifInfoPtr->txMacNum));

        if(egress_isPreemptiveChannelPtr)
        {
            *egress_isPreemptiveChannelPtr = portMifInfoPtr->egress_isPreemptiveChannel;
        }

        return portMifInfoPtr->txMacNum;
    }

    if(nextIndexPtr == NULL || (*nextIndexPtr) == 0)/*no ports for' first iteration' */
    {
        __LOG(("MIF : NO Convert found from global TxDMA port [%d] to global MAC (WARNING : traffic will not egress the port) \n",
            txDmaGlobalPortNum));
    }

    if(egress_isPreemptiveChannelPtr)
    {
        *egress_isPreemptiveChannelPtr = 0;
    }

    return SMAIN_NOT_VALID_CNS;

}

/**
* @internal snetHawkMifTxDmaToMacConvert function
* @endinternal
*
* @brief   convert global txdma port num to global mac port num and indication if
*          the port is 'preemptive' and if the MIF enabled (clock and force link).
*          using the MIF tx mapping feature
*
*/
void snetHawkMifTxDmaToMacConvert
(
    IN SKERNEL_DEVICE_OBJECT   *devObjPtr,
    IN GT_U32                  txDmaGlobalPort,
    OUT GT_U32                 *globalMacPtr ,
    OUT GT_U32                 *isPreemptiveChannelPtr,
    OUT GT_U32                 *mif_is_clock_enable_Ptr,
    OUT GT_U32                 *mif_is_channel_force_link_down_Ptr
)
{
    GT_STATUS rc;
    ENHANCED_PORT_INFO_STC portInfo;
    GT_U32  regAddr,value,mifType;
    GT_U32  macPort;
    GT_U32  isCpuPort;
    GT_U32  cpuPortIndex;
    GT_U32  mifUnit;
    GT_U32  mifChannel;

    /* Ironman */
    if (devObjPtr->devMemDmaIsPreemptivePtr)
    {
        *isPreemptiveChannelPtr =
            devObjPtr->devMemDmaIsPreemptivePtr(devObjPtr, txDmaGlobalPort);
    }
    if (devObjPtr->devMemMifChannelFindByDmaPtr)
    {

        rc = devObjPtr->devMemMifChannelFindByDmaPtr(
            devObjPtr, txDmaGlobalPort, 1/*isTx*/, &mifUnit, &mifChannel);
        if (rc != GT_OK)
        {
            *mif_is_clock_enable_Ptr = 0;/* this will 'kill' the traffic to this channel */
            *mif_is_channel_force_link_down_Ptr = 0;
            *globalMacPtr = 0;
            return;
        }
        regAddr = SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->
            sip6_30_MIF[0].mif8Dp.unit[mifUnit].mif_channel_mapping_register[mifChannel];
        smemRegFldGet(devObjPtr, regAddr ,18 ,1, mif_is_clock_enable_Ptr);

        regAddr = SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->
            sip6_30_MIF[0].mif8Dp.unit[mifUnit].mif_link_fsm_control_register[mifChannel];
        smemRegFldGet(devObjPtr, regAddr, 1, 2, &value);
        /* (value == 1) means forced to down */
        *mif_is_channel_force_link_down_Ptr = (value == 1) ? 1 : 0;
    }
    if (devObjPtr->devMemMifToMacChannelConvertPtr)
    {
        rc = devObjPtr->devMemMifToMacChannelConvertPtr(
            mifUnit, mifChannel, *isPreemptiveChannelPtr, globalMacPtr);
        if (rc != GT_OK)
        {
            *mif_is_clock_enable_Ptr = 0;/* this will 'kill' the traffic to this channel */
            *mif_is_channel_force_link_down_Ptr = 0;
            *globalMacPtr = 0;
        }
        return;
    }

    /* Hawk, Phoenux, Harrier */
    SCIB_SEM_TAKE;
    macPort = snetHawkTxDmaToMacConvert(devObjPtr,txDmaGlobalPort,NULL,isPreemptiveChannelPtr);
    SCIB_SEM_SIGNAL;
    if(macPort == SMAIN_NOT_VALID_CNS)
    {
        *globalMacPtr = 0;
        *isPreemptiveChannelPtr = 0;
        *mif_is_clock_enable_Ptr = 0;/* this will 'kill' the traffic to this channel */
        *mif_is_channel_force_link_down_Ptr = 0;
        return;
    }

    *globalMacPtr = macPort;


    isCpuPort = devObjPtr->portsArr[macPort].state == SKERNEL_PORT_STATE_MTI_CPU_E;
    if(isCpuPort)
    {
        devObjPtr->devMemPortInfoGetPtr(devObjPtr,SMEM_UNIT_TYPE_MTI_MAC_CPU_E  ,macPort,&portInfo);
        cpuPortIndex = portInfo.simplePortInfo.unitIndex;
    }
    else
    {
        cpuPortIndex = 0;/* dont care */
    }

    devObjPtr->devMemPortInfoGetPtr(devObjPtr,SMEM_UNIT_TYPE_EPI_MIF_TYPE_E  ,macPort,&portInfo);
    mifType  = portInfo.simplePortInfo.unitIndex;

    if(*isPreemptiveChannelPtr)
    {
        if(isCpuPort)
        {
            regAddr = MTI_CPU_PORT_MIF_PORT(devObjPtr,cpuPortIndex).preemption.mif_channel_mapping_register[mifType];
        }
        else
        {
            regAddr = MTI_PORT_MIF(devObjPtr,macPort).preemption.mif_channel_mapping_register[mifType];
        }
    }
    else
    {
        if(isCpuPort)
        {
            regAddr = MTI_CPU_PORT_MIF_PORT(devObjPtr,cpuPortIndex).mif_channel_mapping_register[mifType];
        }
        else
        {
            regAddr = MTI_PORT_MIF(devObjPtr,macPort).mif_channel_mapping_register[mifType];
        }
    }
    smemRegFldGet(devObjPtr, regAddr ,18 ,1, mif_is_clock_enable_Ptr);

    if(isCpuPort)
    {
        regAddr = MTI_CPU_PORT_MIF_PORT(devObjPtr,cpuPortIndex).mif_link_fsm_control_register[mifType];
    }
    else
    {
        regAddr = MTI_PORT_MIF(devObjPtr,macPort).mif_link_fsm_control_register[mifType];
    }
    smemRegFldGet(devObjPtr, regAddr ,1 ,2, &value);

    if(value == 1)
    {
        *mif_is_channel_force_link_down_Ptr = 1; /*forced to down*/
    }
    else
    {
        *mif_is_channel_force_link_down_Ptr = 0;
    }

    return;
}

/**
* @internal snetHawkMifIngressMacToRxDmaConvert function
* @endinternal
*
* @brief   convert global mac port num and indication of port is 'preemptive' to
*          local txdma and indication if the MIF enabled (clock and enabled).
*
*/
void snetHawkMifIngressMacToRxDmaConvert
(
    IN SKERNEL_DEVICE_OBJECT   *devObjPtr,
    IN GT_U32                  macPort ,
    IN GT_U32                  isPreemptiveChannel,
    OUT GT_U32                 *mif_is_clock_enable_Ptr,
    OUT GT_U32                 *mif_is_Rx_channel_enable_Ptr,
    OUT GT_U32                 *local_dma_port_number_Ptr
)
{
    ENHANCED_PORT_INFO_STC portInfo;
    GT_U32  regAddr,mifType;
    GT_U32  isCpuPort;
    GT_U32  cpuPortIndex;
    GT_U32  sip6_MTI_EXTERNAL_representativePort;
    GT_U32  local_mif_Rx_channel_id_number;

    /* Ironman */
    if (devObjPtr->devMemMacToMifChannelConvertPtr)
    {
        GT_STATUS rc;
        GT_U32   mifUnit;
        GT_U32   mifChannel;

        rc = devObjPtr->devMemMacToMifChannelConvertPtr(
            macPort, isPreemptiveChannel, &mifUnit, &mifChannel);
        if (rc != GT_OK)
        {
            *mif_is_clock_enable_Ptr = 0;
            *mif_is_Rx_channel_enable_Ptr = 0;
            *local_dma_port_number_Ptr = 0;
            return;
        }
        regAddr = SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->
            sip6_30_MIF[0].mif8Dp.unit[mifUnit].mif_channel_mapping_register[mifChannel];
        smemRegFldGet(devObjPtr, regAddr, 18, 1, mif_is_clock_enable_Ptr);
        smemRegFldGet(devObjPtr, regAddr, 14, 1, mif_is_Rx_channel_enable_Ptr);
        smemRegFldGet(devObjPtr, regAddr,  8, 6, local_dma_port_number_Ptr);
        return;
    }

    isCpuPort = devObjPtr->portsArr[macPort].state == SKERNEL_PORT_STATE_MTI_CPU_E;
    if(isCpuPort)
    {
        devObjPtr->devMemPortInfoGetPtr(devObjPtr,SMEM_UNIT_TYPE_MTI_MAC_CPU_E  ,macPort,&portInfo);
        cpuPortIndex = portInfo.simplePortInfo.unitIndex;
    }
    else
    {
        cpuPortIndex = 0;/* dont care */
    }

    /* get the local channel index in this unit */
    devObjPtr->devMemPortInfoGetPtr(devObjPtr,SMEM_UNIT_TYPE_EPI_MIF_CHANNEL_E  ,macPort,&portInfo);
    local_mif_Rx_channel_id_number =  portInfo.simplePortInfo.indexInUnit;

    /* MIF hold dedicated logic for representative port (apply to 'CPU port' too!) */
    sip6_MTI_EXTERNAL_representativePort = portInfo.sip6_MTI_EXTERNAL_representativePortIndex;

    devObjPtr->devMemPortInfoGetPtr(devObjPtr,SMEM_UNIT_TYPE_EPI_MIF_TYPE_E  ,macPort,&portInfo);
    mifType  = portInfo.simplePortInfo.unitIndex;

    if(isPreemptiveChannel)
    {
        if(isCpuPort)
        {
            regAddr = MTI_CPU_PORT_MIF_PORT(devObjPtr,cpuPortIndex).preemption.mif_channel_mapping_register[mifType];
        }
        else
        {
            regAddr = MTI_PORT_MIF(devObjPtr,macPort).preemption.mif_channel_mapping_register[mifType];
        }
    }
    else
    {
        if(isCpuPort)
        {
            regAddr = MTI_CPU_PORT_MIF_PORT(devObjPtr,cpuPortIndex).mif_channel_mapping_register[mifType];
        }
        else
        {
            regAddr = MTI_PORT_MIF(devObjPtr,macPort).mif_channel_mapping_register[mifType];
        }
    }

    smemRegFldGet(devObjPtr, regAddr ,18 ,1, mif_is_clock_enable_Ptr);

    smemRegFldGet(devObjPtr, regAddr , 8 ,6, local_dma_port_number_Ptr);

    if (SMEM_CHT_IS_SIP7_GET(devObjPtr))
    {
        smemRegFldGet(devObjPtr, regAddr , 14 ,1, mif_is_Rx_channel_enable_Ptr);
    }
    else
    {
        if(isCpuPort)
        {
            regAddr = MTI_CPU_PORT_MIF_GLOBAL(devObjPtr,sip6_MTI_EXTERNAL_representativePort).mif_rx_control_register[mifType];
        }
        else
        {
            regAddr = MTI_PORT_MIF_GLOBAL(devObjPtr,sip6_MTI_EXTERNAL_representativePort).mif_rx_control_register[mifType];
        }

        if(mifType == SMEM_CHT_PORT_MTI_MIF_TYPE_128_E)
        {
            /* bit 0 for local dma 0 , bit 1 for local dma 4 */
            local_mif_Rx_channel_id_number = local_mif_Rx_channel_id_number ? 1 : 0;
        }

        smemRegFldGet(devObjPtr, regAddr , local_mif_Rx_channel_id_number ,1, mif_is_Rx_channel_enable_Ptr);
    }

    return;
}


