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
* @file snetCheetahHa.c
*
* @brief This is a external API definition for HA unit (part of egress processing)
*
* @version   121
********************************************************************************
*/
#include <asicSimulation/SKernel/smain/smain.h>
#include <asicSimulation/SKernel/smem/smemCheetah.h>
#include <asicSimulation/SKernel/suserframes/snet.h>
#include <asicSimulation/SKernel/skernel.h>
#include <asicSimulation/SKernel/cheetahCommon/sregCheetah.h>
#include <asicSimulation/SKernel/cheetahCommon/sregBobcat2.h>
#include <asicSimulation/SKernel/cheetahCommon/sregFalcon.h>
#include <asicSimulation/SKernel/suserframes/snetCheetahEgress.h>
#include <asicSimulation/SKernel/suserframes/snetCheetah2TStart.h>
#include <asicSimulation/SKernel/suserframes/snetCheetah3.h>
#include <asicSimulation/SKernel/suserframes/snetXCat.h>
#include <asicSimulation/SKernel/suserframes/snetCheetah3CentralizedCnt.h>
#include <asicSimulation/SKernel/suserframes/snetXCatPolicer.h>
#include <asicSimulation/SKernel/suserframes/snetCheetahEq.h>
#include <asicSimulation/SKernel/suserframes/snetLion.h>
#include <asicSimulation/SKernel/suserframes/snetAasHa.h>
#include <asicSimulation/SLog/simLog.h>
#include <asicSimulation/SLog/simLogInfoTypePacket.h>

/* option for testing future fix ! */
void simulationOfFutureFix_HA_origIsTrunk_reset_set(GT_U32   newValue)
{
    GT_U32  devNum = 0;
    SKERNEL_DEVICE_OBJECT * devObjPtr = smemTestDeviceIdToDevPtrConvert(devNum);

    devObjPtr->errata.haOrigIsTrunkReset = newValue;
}

/* option for testing future fix ! */
void simulationOfFutureFix_HA_SrcIdIgnored_set(GT_U32   newValue)
{
    GT_U32  devNum = 0;
    SKERNEL_DEVICE_OBJECT * devObjPtr = smemTestDeviceIdToDevPtrConvert(devNum);

    devObjPtr->errata.haSrcIdIgnored = newValue;
}

#define TT_RESERVED_PORT_NUM      59 /* 0x3b */
#define TS_RESERVED_PORT_NUM      60 /* 0x3c - used to indicate TS done in the previous device */
#define ROUTER_RESERVED_PORT_NUM  61 /* 0x3d */

#define IPV4_L3_SIP_OFFSET  12 /* ipv4 header l3 source address offset */
#define IPV4_L3_DIP_OFFSET  16 /* ipv4 header l3 destination address offset */
#define IPV4_L4_SRC_PORT_OFFSET 0 /* ipv4 header l4 sourse port address offset */
#define IPV4_L4_DST_PORT_OFFSET 2 /* ipv4 header l4 destination address offset */

/* pseudo header length for L4 checksum header calc */
#define L4_MAX_CHECKSUM_BYTE_INDEX  16
/* checksum num of bytes */
#define CHECKSUM_NUM_BYTES  2
/* pseudo header length for L4 checksum header calc */
#define L3_AND_l4_PSEUDO_HEADER_LENGTH  (12+L4_MAX_CHECKSUM_BYTE_INDEX+CHECKSUM_NUM_BYTES)

#define CHECKSUM_OFFSET_IN_L4_MAC(ipProt) \
    (((ipProt) == SNET_UDP_PROT_E) ? 6 /*udp*/: 16/*tcp*/)

#define IPV6_L3_SIP_OFFSET  8  /* ipv6 header l3 source address offset */
#define IPV6_L3_DIP_OFFSET  24 /* ipv6 header l3 destination address offset */

/* change target only if target not set before */
#define SET_IF_NULL_MAC(target,source)                  \
    if((target) == 0)                                   \
    {                                                   \
        /* add LOG info */                              \
        if(simLogIsOpenFlag)                              \
        {                                               \
            __LOG(("SET_IF_NULL_MAC [%s] = [%s] start 4 bytes[%2.2x %2.2x %2.2x %2.2x]\n",  \
            #target , #source,                          \
            ((GT_U8*)(source))[0],                     \
            ((GT_U8*)(source))[1],                     \
            ((GT_U8*)(source))[2],                     \
            ((GT_U8*)(source))[3]));                    \
        }                                               \
        (target) = (source);                            \
    }

/* force to set the target */
#define SET_FORCE_MAC(target,source)                    \
    /* add LOG info */                                  \
    if(simLogIsOpenFlag)                                  \
    {                                                   \
        if((source) == NULL)                            \
        {                                               \
            __LOG(("SET_FORCE_MAC [%s] = [NULL]\n",#target));   \
        }                                               \
        else                                            \
        {                                               \
            __LOG(("SET_FORCE_MAC [%s] = [%s] start 4 bytes[%2.2x %2.2x %2.2x %2.2x]\n", \
                #target , #source,                          \
                ((GT_U8*)(source))[0],                     \
                ((GT_U8*)(source))[1],                     \
                ((GT_U8*)(source))[2],                     \
                ((GT_U8*)(source))[3]));                    \
        }                                               \
    }                                                   \
    (target) = (source)

/* force to set the target */
#define SET_FORCE_VALUE_MAC(target,source)              \
    /* add LOG info */                                  \
    if(simLogIsOpenFlag)                                  \
    {                                                   \
        __LOG(("SET_FORCE_VALUE_MAC [%s] = [%s] value[0x%x]\n", \
            #target , #source, source));                \
    }                                                   \
    (target) = (source)



/* build GT_U32 from pointer to 4 bytes */
#define SNET_BUILD_BYTES_FROM_WORD_WITH_INDEX_MAC(word,bytesPtr,index)\
    bytesPtr[index]   = (GT_U8)SMEM_U32_GET_FIELD(word,24,8);\
    bytesPtr[index+1] = (GT_U8)SMEM_U32_GET_FIELD(word,16,8);\
    bytesPtr[index+2] = (GT_U8)SMEM_U32_GET_FIELD(word, 8,8);\
    bytesPtr[index+3] = (GT_U8)SMEM_U32_GET_FIELD(word, 0,8)

/* print PTP header */
#define PTP_HEADER_LOG(memSrcPtr, bytes)                            \
    if(simLogIsOpenFlag)                                            \
    {                                                               \
        {                                                           \
            GT_U32  ii;                                             \
            __LOG(("bytes:"));                                      \
            for(ii = 0 ; ii < bytes && ii < 20 ;ii++)               \
            {                                                       \
                __LOG(("%2.2x ",(memSrcPtr)[ii]));                  \
            }                                                       \
            if(bytes > 20)                                          \
            {                                                       \
                __LOG(("(only first 20 bytes) \n"));                \
            }                                                       \
            else                                                    \
            {                                                       \
                __LOG(("\n"));                                      \
            }                                                       \
        }                                                           \
    }

static void  snetSip6_30HaPrpTrailerBuild
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U8    destVlanTagged
);

static void snetHaSip6_30EgressTag1After4BytesExtension
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN HA_INTERNAL_INFO_STC *haInfoPtr,
    INOUT GT_U8   *egressTagPtr
);

static void  haProcessNat44L3L4HeaderModify
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    GT_U8    *l3HeaderChangedBuffer/*[40]*/,
    GT_U8    *l4HeaderChangedBuffer/*[18]*/,
    GT_U32   l4HeaderChangedLen,
    GT_BOOL  isL2Nat
);

static void  haProcessNat66L3HeaderModify
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    GT_U8    *l3HeaderChangedBuffer/*[40]*/
);

/**
* @enum SNET_CHT_HA_MODIFY_COMMAND_ENT
 *
 * @brief sip5 transit tunnels parsing mode
*/
typedef enum{

    /** Replace the SIP prefix with the NAT<Address>. */
    SNET_CHT_HA_MODIFY_COMMAND_MODIFY_SIP_PREFIX_E,

    /** Replace the DIP prefix with the NAT<Address> */
    SNET_CHT_HA_MODIFY_COMMAND_MODIFY_DIP_PREFIX_E,

    /** Replace the SIP address with NAT<Address> */
    SNET_CHT_HA_MODIFY_COMMAND_MODIFY_SIP_ADDRESS_E,

    /** Replace the DIP address with NAT<Address> */
    SNET_CHT_HA_MODIFY_COMMAND_MODIFY_DIP_ADDRESS_E,

    /** Replace the SIP prefix with NAT<Address>;
     *  without performing checksum-neutral mapping.
     *  APPLICABLE only for Falcon */
    SNET_CHT_HA_MODIFY_COMMAND_MODIFY_SIP_PREFIX_WO_E,

    /** Replace the DIP prefix with NAT<Address>;
     *  without performing checksum-neutral mapping.
     *  APPLICABLE only for Falcon */
    SNET_CHT_HA_MODIFY_COMMAND_MODIFY_DIP_PREFIX_WO_E

} SNET_CHT_HA_MODIFY_COMMAND_ENT;

#define SMEM_LION3_HA_PHYSICAL_PORT_TABLE_1_FIELDS_NAME                                           \
     STR(SMEM_LION3_HA_PHYSICAL_PORT_TABLE_1_FIELDS_ROUTED_MAC_SA_MOD_EN                         )\
    ,STR(SMEM_LION3_HA_PHYSICAL_PORT_TABLE_1_FIELDS_ROUTER_MAC_SA_ASSIGNMENT_MODE                )\
    ,STR(SMEM_LION3_HA_PHYSICAL_PORT_TABLE_1_FIELDS_PER_UP0_KEEP_VLAN1_ENABLE                    )\
    ,STR(SMEM_LION3_HA_PHYSICAL_PORT_TABLE_1_FIELDS_IP_TUNNEL_LENGTH_OFFSET_ENABLE               )\
    ,STR(SMEM_LION3_HA_PHYSICAL_PORT_TABLE_1_FIELDS_EPCL_LOOKUP_CONFIGURATION_MODE               )\
    ,STR(SMEM_LION3_HA_PHYSICAL_PORT_TABLE_1_FIELDS_EPCL_NON_TS_DATA_PORT_ENABLE                 )\
    ,STR(SMEM_LION3_HA_PHYSICAL_PORT_TABLE_1_FIELDS_EPCL_TS_DATA_PORT_ENABLE                     )\
    ,STR(SMEM_LION3_HA_PHYSICAL_PORT_TABLE_1_FIELDS_EPCL_FROM_CPU_DATA_PACKETS_PORT_ENABLE       )\
    ,STR(SMEM_LION3_HA_PHYSICAL_PORT_TABLE_1_FIELDS_EPCL_FROM_CPU_CONTROL_PACKETS_PORT_ENABLE    )\
    ,STR(SMEM_LION3_HA_PHYSICAL_PORT_TABLE_1_FIELDS_PORT_GROUP                                   )\
    ,STR(SMEM_LION3_HA_PHYSICAL_PORT_TABLE_1_FIELDS_PORT_LIST_BIT_VECTOR_OFFSET                  )\
    ,STR(SMEM_HAWK_HA_PHYSICAL_PORT_TABLE_1_FIELDS_PCLID2_E                                      )\
    ,STR(SMEM_HAWK_HA_PHYSICAL_PORT_TABLE_1_FIELDS_SET_SRC_ID_E                                  )\
    ,STR(SMEM_HAWK_HA_PHYSICAL_PORT_TABLE_1_FIELDS_SRC_ID_E                                      )\
    ,STR(SMEM_SIP6_30_HA_PHYSICAL_PORT_TABLE_1_FIELDS_SET_DESTINATION_EPG_E  /*58*/              )\
    ,STR(SMEM_SIP6_30_HA_PHYSICAL_PORT_TABLE_1_FIELDS_DESTINATION_EPG_E      /*59..70*/          )\
    ,STR(SMEM_SIP6_30_HA_PHYSICAL_PORT_TABLE_1_FIELDS_PRP_TRAILER_ACTION_E   /*71..72*/          )\
    ,STR(SMEM_SIP6_30_HA_PHYSICAL_PORT_TABLE_1_FIELDS_PRP_PADDING_SIZE_E     /*73..74*/          )\
    ,STR(SMEM_SIP6_30_HA_PHYSICAL_PORT_TABLE_1_FIELDS_PRP_BASE_LSDU_OFFSET_E /*75..80*/          )




static char * lion3HaPhysicalPort1FieldsTableNames[
    SMEM_LION3_HA_PHYSICAL_PORT_TABLE_1_FIELDS____LAST_VALUE___E]=
    {SMEM_LION3_HA_PHYSICAL_PORT_TABLE_1_FIELDS_NAME};


static SNET_ENTRY_FORMAT_TABLE_STC lion3HaPhysicalPort1TableFieldsFormat[
    SMEM_LION3_HA_PHYSICAL_PORT_TABLE_1_FIELDS____LAST_VALUE___E]=
{
/*SMEM_LION3_HA_PHYSICAL_PORT_TABLE_1_FIELDS_ROUTED_MAC_SA_MOD_EN                      */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_HA_PHYSICAL_PORT_TABLE_1_FIELDS_ROUTER_MAC_SA_ASSIGNMENT_MODE             */
    STANDARD_FIELD_MAC(2),
/*SMEM_LION3_HA_PHYSICAL_PORT_TABLE_1_FIELDS_PER_UP0_KEEP_VLAN1_ENABLE                 */
    STANDARD_FIELD_MAC(8),
/*SMEM_LION3_HA_PHYSICAL_PORT_TABLE_1_FIELDS_IP_TUNNEL_LENGTH_OFFSET_ENABLE            */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_HA_PHYSICAL_PORT_TABLE_1_FIELDS_EPCL_LOOKUP_CONFIGURATION_MODE            */
    STANDARD_FIELD_MAC(2),
/*SMEM_LION3_HA_PHYSICAL_PORT_TABLE_1_FIELDS_EPCL_NON_TS_DATA_PORT_ENABLE              */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_HA_PHYSICAL_PORT_TABLE_1_FIELDS_EPCL_TS_DATA_PORT_ENABLE                  */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_HA_PHYSICAL_PORT_TABLE_1_FIELDS_EPCL_FROM_CPU_DATA_PACKETS_PORT_ENABLE    */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_HA_PHYSICAL_PORT_TABLE_1_FIELDS_EPCL_FROM_CPU_CONTROL_PACKETS_PORT_ENABLE */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_HA_PHYSICAL_PORT_TABLE_1_FIELDS_PORT_GROUP                                */
    STANDARD_FIELD_MAC(4),
/*SMEM_LION3_HA_PHYSICAL_PORT_TABLE_1_FIELDS_PORT_LIST_BIT_VECTOR_OFFSET               */
    STANDARD_FIELD_MAC(5),
/*SMEM_HAWK_HA_PHYSICAL_PORT_TABLE_1_FIELDS_PCLID2_E                                   */
    STANDARD_FIELD_MAC(24),
/*SMEM_HAWK_HA_PHYSICAL_PORT_TABLE_1_FIELDS_SET_SRC_ID_E                               */
    STANDARD_FIELD_MAC(1),
/*SMEM_HAWK_HA_PHYSICAL_PORT_TABLE_1_FIELDS_SRC_ID_E                                   */
    STANDARD_FIELD_MAC(6),
/*SMEM_SIP6_30_HA_PHYSICAL_PORT_TABLE_1_FIELDS_SET_DESTINATION_EPG_E  */ /*58*/
    STANDARD_FIELD_MAC(1),
/*SMEM_SIP6_30_HA_PHYSICAL_PORT_TABLE_1_FIELDS_DESTINATION_EPG_E      */ /*59..70*/
    STANDARD_FIELD_MAC(12),
/*SMEM_SIP6_30_HA_PHYSICAL_PORT_TABLE_1_FIELDS_PRP_TRAILER_ACTION_E   */ /*71..72*/
    STANDARD_FIELD_MAC(2),
/*SMEM_SIP6_30_HA_PHYSICAL_PORT_TABLE_1_FIELDS_PRP_PADDING_SIZE_E     */ /*73..74*/
    STANDARD_FIELD_MAC(2),
/*SMEM_SIP6_30_HA_PHYSICAL_PORT_TABLE_1_FIELDS_PRP_BASE_LSDU_OFFSET_E */ /*75..80*/
    STANDARD_FIELD_MAC(6)
};


#define SMEM_LION3_HA_PHYSICAL_PORT_TABLE_2_FIELDS_NAME                                            \
     STR(SMEM_LION3_HA_PHYSICAL_PORT_TABLE_2_FIELDS_CASCADE_PORT_ENABLE                           )\
    ,STR(SMEM_LION3_HA_PHYSICAL_PORT_TABLE_2_FIELDS_EGRESS_DSA_TAG_TYPE                           )\
    ,STR(SMEM_LION3_HA_PHYSICAL_PORT_TABLE_2_FIELDS_SER_CHECK_ENABLE                              )\
    ,STR(SMEM_LION3_HA_PHYSICAL_PORT_TABLE_2_FIELDS_DISABLE_CRC_ADDING                            )\
    ,STR(SMEM_LION3_HA_PHYSICAL_PORT_TABLE_2_FIELDS_RETAIN_EXTERNAL_CRC                           )\
    ,STR(SMEM_LION3_HA_PHYSICAL_PORT_TABLE_2_FIELDS_PACKET_ID_ENABLE                              )\
    ,STR(SMEM_LION3_HA_PHYSICAL_PORT_TABLE_2_FIELDS_ROUTED_SRC_DEVICE_ID_PORT_MODE_ENABLE         )\
    ,STR(SMEM_LION3_HA_PHYSICAL_PORT_TABLE_2_FIELDS_SET_SOURCE_TO_LOCAL                           )\
    ,STR(SMEM_LION3_HA_PHYSICAL_PORT_TABLE_2_FIELDS_LOCAL_DEVICE_NUMBER                           )\
    ,STR(SMEM_LION3_HA_PHYSICAL_PORT_TABLE_2_FIELDS_LOCAL_SOURCE_ID                               )\
    ,STR(SMEM_LION3_HA_PHYSICAL_PORT_TABLE_2_FIELDS_EPCL_TO_ANALYZER_PACKETS_PORT_ENABLE          )\
    ,STR(SMEM_LION3_HA_PHYSICAL_PORT_TABLE_2_FIELDS_EPCL_TO_CPU_PACKETS_PORT_ENABLE               )\
    ,STR(SMEM_LION3_HA_PHYSICAL_PORT_TABLE_2_FIELDS_PTP_TIMESTAMP_TAG_MODE                        )\
    ,STR(SMEM_LION3_HA_PHYSICAL_PORT_TABLE_2_FIELDS_INVALID_CRC_MODE                              )\
    ,STR(SMEM_LION3_HA_PHYSICAL_PORT_TABLE_2_FIELDS_TIMESTAMP_RECEPTION_EN                        )\
    ,STR(SMEM_LION3_HA_PHYSICAL_PORT_TABLE_2_FIELDS_TRUNK_ID                                      )\
    ,STR(SMEM_LION3_HA_PHYSICAL_PORT_TABLE_2_FIELDS_FORCE_NEW_DSA_TO_CPU                          )\
    ,STR(SMEM_LION3_HA_PHYSICAL_PORT_TABLE_2_FIELDS_FORCE_NEW_DSA_FORWARD_OR_FROM_CPU             )\
    ,STR(SMEM_LION3_HA_PHYSICAL_PORT_TABLE_2_FIELDS_PRE_PEND_TWO_BYTES_HEADER_EN                  )\
    ,STR(SMEM_LION3_HA_PHYSICAL_PORT_TABLE_2_FIELDS_DSA_QOS_MODE                                  )\
    ,STR(SMEM_LION3_HA_PHYSICAL_PORT_TABLE_2_FIELDS_REMOTE_PHYSICAL_PORT_MAP_ENABLE               )\
    ,STR(SMEM_LION3_HA_PHYSICAL_PORT_TABLE_2_FIELDS_REMOTE_PHYSICAL_PORT_NUMBER                   )\
    ,STR(SMEM_LION3_HA_PHYSICAL_PORT_TABLE_2_FIELDS_REMOTE_PHYSICAL_PORT_DEVICE_ID                )\
    ,STR(SMEM_SIP5_15_HA_PHYSICAL_PORT_TABLE_2_FIELDS_TRANS_FWD_TO_4B_FROM_CPU_DSA                )\
    ,STR(SMEM_SIP5_15_HA_PHYSICAL_PORT_TABLE_2_FIELDS_TRANS_TO_ANALYZER_TO_4B_FROM_CPU_DSA        )\
    ,STR(SMEM_SIP5_15_HA_PHYSICAL_PORT_TABLE_2_FIELDS_TRANS_FROM_CPU_TO_4B_FROM_CPU_DSA           )\
    ,STR(SMEM_SIP5_20_HA_PHYSICAL_PORT_TABLE_2_FIELDS_INSERT_HASH_INTO_FWD_EDSA_EN                )\
    ,STR(SMEM_SIP6_10_HA_PHYSICAL_PORT_TABLE_2_FIELDS_PUSHED_TAG_VLAN_ID_E                        )\
    ,STR(SMEM_SIP6_10_HA_PHYSICAL_PORT_TABLE_2_FIELDS_USE_PHYSICAL_PORT_PUSH_TAG_VID_EN_E         )\
    ,STR(SMEM_SIP6_30_HA_PHYSICAL_PORT_TABLE_2_FIELDS_HSR_BASE_LSDU_OFFSET_E /*95..100*/          )



static char * lion3HaPhysicalPort2FieldsTableNames[
    SMEM_LION3_HA_PHYSICAL_PORT_TABLE_2_FIELDS____LAST_VALUE___E]=
    {SMEM_LION3_HA_PHYSICAL_PORT_TABLE_2_FIELDS_NAME};


static SNET_ENTRY_FORMAT_TABLE_STC lion3HaPhysicalPort2TableFieldsFormat[
    SMEM_LION3_HA_PHYSICAL_PORT_TABLE_2_FIELDS____LAST_VALUE___E]=
{
/*SMEM_LION3_HA_PHYSICAL_PORT_TABLE_2_FIELDS_CASCADE_PORT_ENABLE                          */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_HA_PHYSICAL_PORT_TABLE_2_FIELDS_EGRESS_DSA_TAG_TYPE                          */
    STANDARD_FIELD_MAC(2),
/*SMEM_LION3_HA_PHYSICAL_PORT_TABLE_2_FIELDS_SER_CHECK_ENABLE                             */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_HA_PHYSICAL_PORT_TABLE_2_FIELDS_DISABLE_CRC_ADDING                           */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_HA_PHYSICAL_PORT_TABLE_2_FIELDS_RETAIN_EXTERNAL_CRC                          */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_HA_PHYSICAL_PORT_TABLE_2_FIELDS_PACKET_ID_ENABLE                             */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_HA_PHYSICAL_PORT_TABLE_2_FIELDS_ROUTED_SRC_DEVICE_ID_PORT_MODE_ENABLE_E      */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_HA_PHYSICAL_PORT_TABLE_2_FIELDS_SET_SOURCE_TO_LOCAL                          */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_HA_PHYSICAL_PORT_TABLE_2_FIELDS_LOCAL_DEVICE_NUMBER                          */
    STANDARD_FIELD_MAC(12),
/*SMEM_LION3_HA_PHYSICAL_PORT_TABLE_2_FIELDS_LOCAL_SOURCE_ID                              */
    STANDARD_FIELD_MAC(12),
/*SMEM_LION3_HA_PHYSICAL_PORT_TABLE_2_FIELDS_EPCL_TO_ANALYZER_PACKETS_PORT_ENABLE         */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_HA_PHYSICAL_PORT_TABLE_2_FIELDS_EPCL_TO_CPU_PACKETS_PORT_ENABLE              */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_HA_PHYSICAL_PORT_TABLE_2_FIELDS_PTP_TIMESTAMP_TAG_MODE                       */
    STANDARD_FIELD_MAC(3),
/*SMEM_LION3_HA_PHYSICAL_PORT_TABLE_2_FIELDS_INVALID_CRC_MODE                             */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_HA_PHYSICAL_PORT_TABLE_2_FIELDS_TIMESTAMP_RECEPTION_EN                       */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_HA_PHYSICAL_PORT_TABLE_2_FIELDS_TRUNK_ID                                     */
    STANDARD_FIELD_MAC(12),
/*SMEM_LION3_HA_PHYSICAL_PORT_TABLE_2_FIELDS_FORCE_NEW_DSA_TO_CPU                         */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_HA_PHYSICAL_PORT_TABLE_2_FIELDS_FORCE_NEW_DSA_FORWARD_OR_FROM_CPU            */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_HA_PHYSICAL_PORT_TABLE_2_FIELDS_PRE_PEND_TWO_BYTES_HEADER_EN                 */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_HA_PHYSICAL_PORT_TABLE_2_FIELDS_DSA_QOS_MODE                                 */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_HA_PHYSICAL_PORT_TABLE_2_FIELDS_REMOTE_PHYSICAL_PORT_MAP_ENABLE              */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_HA_PHYSICAL_PORT_TABLE_2_FIELDS_REMOTE_PHYSICAL_PORT_NUMBER                  */
    STANDARD_FIELD_MAC(8),
/*SMEM_LION3_HA_PHYSICAL_PORT_TABLE_2_FIELDS_REMOTE_PHYSICAL_PORT_DEVICE_ID               */
    STANDARD_FIELD_MAC(12),
/*SMEM_SIP5_15_HA_PHYSICAL_PORT_TABLE_2_FIELDS_TRANS_FWD_TO_4B_FROM_CPU_DSA               */
    STANDARD_FIELD_MAC(1),
/*SMEM_SIP5_15_HA_PHYSICAL_PORT_TABLE_2_FIELDS_TRANS_TO_ANALYZER_TO_4B_FROM_CPU_DSA       */
    STANDARD_FIELD_MAC(1),
/*SMEM_SIP5_15_HA_PHYSICAL_PORT_TABLE_2_FIELDS_TRANS_FROM_CPU_TO_4B_FROM_CPU_DSA          */
    STANDARD_FIELD_MAC(1),
/*SMEM_SIP5_20_HA_PHYSICAL_PORT_TABLE_2_FIELDS_INSERT_HASH_INTO_FWD_EDSA_EN               */
    STANDARD_FIELD_MAC(1)
};

static SNET_ENTRY_FORMAT_TABLE_STC sip6HaPhysicalPort2TableFieldsFormat[
    SMEM_LION3_HA_PHYSICAL_PORT_TABLE_2_FIELDS____LAST_VALUE___E]=
{
/*SMEM_LION3_HA_PHYSICAL_PORT_TABLE_2_FIELDS_CASCADE_PORT_ENABLE                          */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_HA_PHYSICAL_PORT_TABLE_2_FIELDS_EGRESS_DSA_TAG_TYPE                          */
    STANDARD_FIELD_MAC(2),
/*SMEM_LION3_HA_PHYSICAL_PORT_TABLE_2_FIELDS_SER_CHECK_ENABLE                             */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_HA_PHYSICAL_PORT_TABLE_2_FIELDS_DISABLE_CRC_ADDING                           */
    STANDARD_FIELD_MAC(0), /* removed from sip6 */
/*SMEM_LION3_HA_PHYSICAL_PORT_TABLE_2_FIELDS_RETAIN_EXTERNAL_CRC                          */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_HA_PHYSICAL_PORT_TABLE_2_FIELDS_PACKET_ID_ENABLE                             */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_HA_PHYSICAL_PORT_TABLE_2_FIELDS_ROUTED_SRC_DEVICE_ID_PORT_MODE_ENABLE_E      */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_HA_PHYSICAL_PORT_TABLE_2_FIELDS_SET_SOURCE_TO_LOCAL                          */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_HA_PHYSICAL_PORT_TABLE_2_FIELDS_LOCAL_DEVICE_NUMBER                          */
    STANDARD_FIELD_MAC(12),
/*SMEM_LION3_HA_PHYSICAL_PORT_TABLE_2_FIELDS_LOCAL_SOURCE_ID                              */
    STANDARD_FIELD_MAC(12),
/*SMEM_LION3_HA_PHYSICAL_PORT_TABLE_2_FIELDS_EPCL_TO_ANALYZER_PACKETS_PORT_ENABLE         */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_HA_PHYSICAL_PORT_TABLE_2_FIELDS_EPCL_TO_CPU_PACKETS_PORT_ENABLE              */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_HA_PHYSICAL_PORT_TABLE_2_FIELDS_PTP_TIMESTAMP_TAG_MODE                       */
    STANDARD_FIELD_MAC(3),
/*SMEM_LION3_HA_PHYSICAL_PORT_TABLE_2_FIELDS_INVALID_CRC_MODE                             */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_HA_PHYSICAL_PORT_TABLE_2_FIELDS_TIMESTAMP_RECEPTION_EN                       */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_HA_PHYSICAL_PORT_TABLE_2_FIELDS_TRUNK_ID                                     */
    STANDARD_FIELD_MAC(12),
/*SMEM_LION3_HA_PHYSICAL_PORT_TABLE_2_FIELDS_FORCE_NEW_DSA_TO_CPU                         */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_HA_PHYSICAL_PORT_TABLE_2_FIELDS_FORCE_NEW_DSA_FORWARD_OR_FROM_CPU            */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_HA_PHYSICAL_PORT_TABLE_2_FIELDS_PRE_PEND_TWO_BYTES_HEADER_EN                 */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_HA_PHYSICAL_PORT_TABLE_2_FIELDS_DSA_QOS_MODE                                 */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_HA_PHYSICAL_PORT_TABLE_2_FIELDS_REMOTE_PHYSICAL_PORT_MAP_ENABLE              */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_HA_PHYSICAL_PORT_TABLE_2_FIELDS_REMOTE_PHYSICAL_PORT_NUMBER                  */
    STANDARD_FIELD_MAC(10), /* was 8 */
/*SMEM_LION3_HA_PHYSICAL_PORT_TABLE_2_FIELDS_REMOTE_PHYSICAL_PORT_DEVICE_ID               */
    STANDARD_FIELD_MAC(12),
/*SMEM_SIP5_15_HA_PHYSICAL_PORT_TABLE_2_FIELDS_TRANS_FWD_TO_4B_FROM_CPU_DSA               */
    STANDARD_FIELD_MAC(1),
/*SMEM_SIP5_15_HA_PHYSICAL_PORT_TABLE_2_FIELDS_TRANS_TO_ANALYZER_TO_4B_FROM_CPU_DSA       */
    STANDARD_FIELD_MAC(1),
/*SMEM_SIP5_15_HA_PHYSICAL_PORT_TABLE_2_FIELDS_TRANS_FROM_CPU_TO_4B_FROM_CPU_DSA          */
    STANDARD_FIELD_MAC(1),
/*SMEM_SIP5_20_HA_PHYSICAL_PORT_TABLE_2_FIELDS_INSERT_HASH_INTO_FWD_EDSA_EN               */
    STANDARD_FIELD_MAC(1),
/*SMEM_SIP6_10_HA_PHYSICAL_PORT_TABLE_2_FIELDS_PUSHED_TAG_VLAN_ID_E                       */
    STANDARD_FIELD_MAC(12),
/*SMEM_SIP6_10_HA_PHYSICAL_PORT_TABLE_2_FIELDS_USE_PHYSICAL_PORT_PUSH_TAG_VID_EN_E        */
    STANDARD_FIELD_MAC(1),
/*SMEM_SIP6_30_HA_PHYSICAL_PORT_TABLE_2_FIELDS_HSR_BASE_LSDU_OFFSET_E*/ /*95..100*/
    STANDARD_FIELD_MAC(6)
};

static SNET_ENTRY_FORMAT_TABLE_STC sip7HaPhysicalPort2TableFieldsFormat[
    SMEM_LION3_HA_PHYSICAL_PORT_TABLE_2_FIELDS____LAST_VALUE___E]=
{
/*SMEM_LION3_HA_PHYSICAL_PORT_TABLE_2_FIELDS_CASCADE_PORT_ENABLE                          */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_HA_PHYSICAL_PORT_TABLE_2_FIELDS_EGRESS_DSA_TAG_TYPE                          */
    STANDARD_FIELD_MAC(3),
/*SMEM_LION3_HA_PHYSICAL_PORT_TABLE_2_FIELDS_SER_CHECK_ENABLE                             */
    STANDARD_FIELD_MAC(0),
/*SMEM_LION3_HA_PHYSICAL_PORT_TABLE_2_FIELDS_DISABLE_CRC_ADDING                           */
    STANDARD_FIELD_MAC(0), /* removed from sip6 */
/*SMEM_LION3_HA_PHYSICAL_PORT_TABLE_2_FIELDS_RETAIN_EXTERNAL_CRC                          */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_HA_PHYSICAL_PORT_TABLE_2_FIELDS_PACKET_ID_ENABLE                             */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_HA_PHYSICAL_PORT_TABLE_2_FIELDS_ROUTED_SRC_DEVICE_ID_PORT_MODE_ENABLE_E      */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_HA_PHYSICAL_PORT_TABLE_2_FIELDS_SET_SOURCE_TO_LOCAL                          */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_HA_PHYSICAL_PORT_TABLE_2_FIELDS_LOCAL_DEVICE_NUMBER                          */
    STANDARD_FIELD_MAC(12),
/*SMEM_LION3_HA_PHYSICAL_PORT_TABLE_2_FIELDS_LOCAL_SOURCE_ID                              */
    STANDARD_FIELD_MAC(12),
/*SMEM_LION3_HA_PHYSICAL_PORT_TABLE_2_FIELDS_EPCL_TO_ANALYZER_PACKETS_PORT_ENABLE         */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_HA_PHYSICAL_PORT_TABLE_2_FIELDS_EPCL_TO_CPU_PACKETS_PORT_ENABLE              */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_HA_PHYSICAL_PORT_TABLE_2_FIELDS_PTP_TIMESTAMP_TAG_MODE                       */
    STANDARD_FIELD_MAC(3),
/*SMEM_LION3_HA_PHYSICAL_PORT_TABLE_2_FIELDS_INVALID_CRC_MODE                             */
    STANDARD_FIELD_MAC(0), /*moved to HA Src Physical Port Table2 */
/*SMEM_LION3_HA_PHYSICAL_PORT_TABLE_2_FIELDS_TIMESTAMP_RECEPTION_EN                       */
    STANDARD_FIELD_MAC(0), /*moved to HA Src Physical Port Table2 */
/*SMEM_LION3_HA_PHYSICAL_PORT_TABLE_2_FIELDS_TRUNK_ID                                     */
    STANDARD_FIELD_MAC(12),/*also in HA Src Physical Port Table2 */
/*SMEM_LION3_HA_PHYSICAL_PORT_TABLE_2_FIELDS_FORCE_NEW_DSA_TO_CPU                         */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_HA_PHYSICAL_PORT_TABLE_2_FIELDS_FORCE_NEW_DSA_FORWARD_OR_FROM_CPU            */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_HA_PHYSICAL_PORT_TABLE_2_FIELDS_PRE_PEND_TWO_BYTES_HEADER_EN                 */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_HA_PHYSICAL_PORT_TABLE_2_FIELDS_DSA_QOS_MODE                                 */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_HA_PHYSICAL_PORT_TABLE_2_FIELDS_REMOTE_PHYSICAL_PORT_MAP_ENABLE              */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_HA_PHYSICAL_PORT_TABLE_2_FIELDS_REMOTE_PHYSICAL_PORT_NUMBER                  */
    STANDARD_FIELD_MAC(10), /* was 8 */
/*SMEM_LION3_HA_PHYSICAL_PORT_TABLE_2_FIELDS_REMOTE_PHYSICAL_PORT_DEVICE_ID               */
    STANDARD_FIELD_MAC(12),
/*SMEM_SIP5_15_HA_PHYSICAL_PORT_TABLE_2_FIELDS_TRANS_FWD_TO_4B_FROM_CPU_DSA               */
    STANDARD_FIELD_MAC(1),
/*SMEM_SIP5_15_HA_PHYSICAL_PORT_TABLE_2_FIELDS_TRANS_TO_ANALYZER_TO_4B_FROM_CPU_DSA       */
    STANDARD_FIELD_MAC(1),
/*SMEM_SIP5_15_HA_PHYSICAL_PORT_TABLE_2_FIELDS_TRANS_FROM_CPU_TO_4B_FROM_CPU_DSA          */
    STANDARD_FIELD_MAC(1),
/*SMEM_SIP5_20_HA_PHYSICAL_PORT_TABLE_2_FIELDS_INSERT_HASH_INTO_FWD_EDSA_EN               */
    STANDARD_FIELD_MAC(1),
/*SMEM_SIP6_10_HA_PHYSICAL_PORT_TABLE_2_FIELDS_PUSHED_TAG_VLAN_ID_E                       */
    STANDARD_FIELD_MAC(12),
/*SMEM_SIP6_10_HA_PHYSICAL_PORT_TABLE_2_FIELDS_USE_PHYSICAL_PORT_PUSH_TAG_VID_EN_E        */
    STANDARD_FIELD_MAC(1),
/*SMEM_SIP6_30_HA_PHYSICAL_PORT_TABLE_2_FIELDS_HSR_BASE_LSDU_OFFSET_E*/ /*95..100*/
    STANDARD_FIELD_MAC(0)/*moved to HA Src Physical Port Table2 */
};

#define SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_NAME  \
      STR(SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_VLAN_TRANS_EN              )\
     ,STR(SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_EGRESS_TAG0_TPID_INDEX     )\
     ,STR(SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_EGRESS_TAG1_TPID_INDEX     )\
     ,STR(SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_TS_HEADER_TPID_INDEX       )\
     ,STR(SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_TS_EXT                     )\
     ,STR(SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_MPLS_PW_LABEL_PUSH_ENABLE  )\
     ,STR(SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_MPLS_PW_LABEL              )\
     ,STR(SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_MPLS_PW_LABEL_EXP          )\
     ,STR(SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_MPLS_PW_LABEL_TTL          )\
     ,STR(SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_PUSH_MPLS_FLOW_LABEL_ENABLE)\
     ,STR(SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_FORCE_ARP_TS_EPORT_DECISION)\
     ,STR(SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_TUNNEL_START               )\
     ,STR(SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_TS_POINTER                 )\
     ,STR(SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_ARP_POINTER                )\
     ,STR(SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_TS_PASSENGER_TYPE          )\
     ,STR(SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_MODIFY_MAC_SA              )\
     ,STR(SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_MODIFY_MAC_DA              )\
     ,STR(SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_TRILL_INTERFACE_ENABLE     )\
     ,STR(SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_KEEP_ORIGINAL_TAG0_TPID    )\
     ,STR(SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_KEEP_ORIGINAL_TAG1_TPID    )\
     ,STR(SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_ENABLE_EGRESS_UP_MAPPING   )\
     ,STR(SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_ENABLE_EGRESS_DSCP_MAPPING )\
     ,STR(SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_ENABLE_EGRESS_EXP_MAPPING  )\
     ,STR(SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_ENABLE_EGRESS_TC_DP_MAPPING)\
     ,STR(SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_QOS_MAPPING_TABLE_INDEX    )\
     ,STR(SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_ROUTER_MAC_SA_INDEX        )\
     ,STR(SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_DP_TO_CFI_MAP_ENABLE       )\
     ,STR(SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_PUSH_SOURCE_BASED_MPLS_LABEL)\
     ,STR(SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_PUSH_EVLAN_BASED_MPLS_LABEL )\
     ,STR( SMEM_SIP6_HA_EPORT_TABLE_1_FIELDS_PHA_THREAD_NUMBER           )\
     ,STR( SMEM_SIP6_10_HA_EPORT_TABLE_1_FIELDS_NESTED_VLAN_EN           )\
     ,STR(SMEM_SIP6_30_HA_EPORT_TABLE_1_FIELDS_NESTED_VLAN_MODE_E /*97..98*/  )\
     ,STR(SMEM_SIP6_30_HA_EPORT_TABLE_1_FIELDS_HSR_PRP_LAN_ID_E   /*99..102*/ )\
     ,STR(SMEM_SIP6_30_HA_EPORT_TABLE_1_FIELDS_L2NAT_ENABLE_E     /*103*/     )\
     ,STR(SMEM_SIP6_30_HA_EPORT_TABLE_1_FIELDS_NAT_PTR_E          /*46..61*/  )


/*static*/ char * lion3HaEPort1FieldsTableNames[
    SMEM_LION3_HA_EPORT_TABLE_1_FIELDS____LAST_VALUE___E] =
    {SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_NAME};

static SNET_ENTRY_FORMAT_TABLE_STC lion3HaEPort1TableFieldsFormat[
    SMEM_LION3_HA_EPORT_TABLE_1_FIELDS____LAST_VALUE___E] =
{
/*SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_VLAN_TRANS_EN                   */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_EGRESS_TAG0_TPID_INDEX          */
    STANDARD_FIELD_MAC(3),
/*SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_EGRESS_TAG1_TPID_INDEX          */
    STANDARD_FIELD_MAC(3),
/*SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_TS_HEADER_TPID_INDEX            */
    STANDARD_FIELD_MAC(3),

    /* TunnelType != "MPLS" */

        /*SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_TS_EXT                          */
        {FIELD_SET_IN_RUNTIME_CNS,
         32,
         SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_TS_HEADER_TPID_INDEX},

    /* TunnelType == "MPLS" */

        /*SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_MPLS_PW_LABEL_PUSH_ENABLE       */
        {FIELD_SET_IN_RUNTIME_CNS,
         1,
         SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_TS_HEADER_TPID_INDEX},
        /*SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_MPLS_PW_LABEL                   */
        STANDARD_FIELD_MAC(20),
        /*SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_MPLS_PW_LABEL_EXP               */
        STANDARD_FIELD_MAC(3),
        /*SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_MPLS_PW_LABEL_TTL               */
        STANDARD_FIELD_MAC(8),
        /*SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_PUSH_MPLS_FLOW_LABEL_ENABLE     */
        STANDARD_FIELD_MAC(1),
        /*SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_FORCE_ARP_TS_EPORT_DECISION     */
        STANDARD_FIELD_MAC(1),
        /*SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_TUNNEL_START                    */
        STANDARD_FIELD_MAC(1),

    /*TunnelStart = "1"*/
        /*SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_TS_POINTER                      */
        {FIELD_SET_IN_RUNTIME_CNS,
         18,
         SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_TUNNEL_START},

    /*TunnelStart = "0"*/
        /*SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_ARP_POINTER                     */
        {FIELD_SET_IN_RUNTIME_CNS,
         20,
         SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_TUNNEL_START},

    /*SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_TS_PASSENGER_TYPE               */
        {FIELD_SET_IN_RUNTIME_CNS,
         1,
         SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_TS_POINTER},

    /*SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_MODIFY_MAC_SA                   */
        {FIELD_SET_IN_RUNTIME_CNS,
         1,
         SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_ARP_POINTER},

/*SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_MODIFY_MAC_DA                   */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_TRILL_INTERFACE_ENABLE          */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_KEEP_ORIGINAL_TAG0_TPID         */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_KEEP_ORIGINAL_TAG1_TPID         */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_ENABLE_EGRESS_UP_MAPPING        */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_ENABLE_EGRESS_DSCP_MAPPING      */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_ENABLE_EGRESS_EXP_MAPPING       */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_ENABLE_EGRESS_TC_DP_MAPPING     */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_QOS_MAPPING_TABLE_INDEX         */
    STANDARD_FIELD_MAC(4),
/*SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_ROUTER_MAC_SA_INDEX             */
    STANDARD_FIELD_MAC(8),
/*SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_DP_TO_CFI_MAP_ENABLE             */
    STANDARD_FIELD_MAC(1)
};

static SNET_ENTRY_FORMAT_TABLE_STC sip5_10HaEPort1TableFieldsFormat[
    SMEM_LION3_HA_EPORT_TABLE_1_FIELDS____LAST_VALUE___E] =
{
/*SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_VLAN_TRANS_EN                   */
    STANDARD_FIELD_MAC(2),/* was 1 bit in sip5*/
/*SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_EGRESS_TAG0_TPID_INDEX          */
    STANDARD_FIELD_MAC(3),
/*SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_EGRESS_TAG1_TPID_INDEX          */
    STANDARD_FIELD_MAC(3),
/*SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_TS_HEADER_TPID_INDEX            */
    STANDARD_FIELD_MAC(3),

    /* TunnelType != "MPLS" */

        /*SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_TS_EXT                          */
        {FIELD_SET_IN_RUNTIME_CNS,
         32,
         SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_TS_HEADER_TPID_INDEX},

    /* TunnelType == "MPLS" */

        /*SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_MPLS_PW_LABEL_PUSH_ENABLE       */
        {FIELD_SET_IN_RUNTIME_CNS,
         1,
         SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_TS_HEADER_TPID_INDEX},
        /*SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_MPLS_PW_LABEL                   */
        STANDARD_FIELD_MAC(20),
        /*SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_MPLS_PW_LABEL_EXP               */
        STANDARD_FIELD_MAC(3),
        /*SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_MPLS_PW_LABEL_TTL               */
        STANDARD_FIELD_MAC(8),
        /*SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_PUSH_MPLS_FLOW_LABEL_ENABLE     */
        STANDARD_FIELD_MAC(1),
        /*SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_FORCE_ARP_TS_EPORT_DECISION     */
        STANDARD_FIELD_MAC(1),
        /*SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_TUNNEL_START                    */
        STANDARD_FIELD_MAC(1),

    /*TunnelStart = "1"*/
        /*SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_TS_POINTER                      */
        {FIELD_SET_IN_RUNTIME_CNS,
         15,/*was 18 in sip5*/
         SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_TUNNEL_START},

    /*TunnelStart = "0"*/
        /*SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_ARP_POINTER                     */
        {FIELD_SET_IN_RUNTIME_CNS,
         17,/*was 20 in sip5*/
         SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_TUNNEL_START},

    /*SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_TS_PASSENGER_TYPE               */
        {FIELD_SET_IN_RUNTIME_CNS,
         1,
         SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_TS_POINTER},

    /*SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_MODIFY_MAC_SA                   */
        {FIELD_SET_IN_RUNTIME_CNS,
         1,
         SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_ARP_POINTER},

/*SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_MODIFY_MAC_DA                   */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_TRILL_INTERFACE_ENABLE          */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_KEEP_ORIGINAL_TAG0_TPID         */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_KEEP_ORIGINAL_TAG1_TPID         */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_ENABLE_EGRESS_UP_MAPPING        */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_ENABLE_EGRESS_DSCP_MAPPING      */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_ENABLE_EGRESS_EXP_MAPPING       */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_ENABLE_EGRESS_TC_DP_MAPPING     */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_QOS_MAPPING_TABLE_INDEX         */
    STANDARD_FIELD_MAC(4),
/*SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_ROUTER_MAC_SA_INDEX             */
    STANDARD_FIELD_MAC(8),
/*SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_DP_TO_CFI_MAP_ENABLE             */
    STANDARD_FIELD_MAC(1)
};

static SNET_ENTRY_FORMAT_TABLE_STC sip5_15HaEPort1TableFieldsFormat[
    SMEM_LION3_HA_EPORT_TABLE_1_FIELDS____LAST_VALUE___E] =
{
/*SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_VLAN_TRANS_EN                   */
    STANDARD_FIELD_MAC(2),/* was 1 bit in sip5*/
/*SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_EGRESS_TAG0_TPID_INDEX          */
    STANDARD_FIELD_MAC(3),
/*SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_EGRESS_TAG1_TPID_INDEX          */
    STANDARD_FIELD_MAC(3),
/*SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_TS_HEADER_TPID_INDEX            */
    STANDARD_FIELD_MAC(3),

    /* TunnelType != "MPLS" */

        /*SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_TS_EXT                          */
        {FIELD_SET_IN_RUNTIME_CNS,
         32,
         SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_TS_HEADER_TPID_INDEX},

    /* TunnelType == "MPLS" */

        /*SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_MPLS_PW_LABEL_PUSH_ENABLE       */
        {FIELD_SET_IN_RUNTIME_CNS,
         1,
         SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_TS_HEADER_TPID_INDEX},
        /*SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_MPLS_PW_LABEL                   */
        STANDARD_FIELD_MAC(20),
        /*SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_MPLS_PW_LABEL_EXP               */
        STANDARD_FIELD_MAC(3),
        /*SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_MPLS_PW_LABEL_TTL               */
        STANDARD_FIELD_MAC(8),
        /*SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_PUSH_MPLS_FLOW_LABEL_ENABLE     */
        STANDARD_FIELD_MAC(1),
        /*SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_FORCE_ARP_TS_EPORT_DECISION     */
        STANDARD_FIELD_MAC(1),
        /*SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_TUNNEL_START                    */
        STANDARD_FIELD_MAC(1),

    /*TunnelStart = "1"*/
        /*SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_TS_POINTER                      */
        {FIELD_SET_IN_RUNTIME_CNS,
         14,/*was 15 in sip5_10*/
         SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_TUNNEL_START},

    /*TunnelStart = "0"*/
        /*SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_ARP_POINTER                     */
        {FIELD_SET_IN_RUNTIME_CNS,
         16,/*was 17 in sip5_10*/
         SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_TUNNEL_START},

    /*SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_TS_PASSENGER_TYPE               */
        {FIELD_SET_IN_RUNTIME_CNS,
         1,
         SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_TS_POINTER},

/*SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_MODIFY_MAC_SA                   */
    {FIELD_SET_IN_RUNTIME_CNS,
     1,
     SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_ARP_POINTER},

/*SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_MODIFY_MAC_DA                   */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_TRILL_INTERFACE_ENABLE          */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_KEEP_ORIGINAL_TAG0_TPID         */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_KEEP_ORIGINAL_TAG1_TPID         */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_ENABLE_EGRESS_UP_MAPPING        */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_ENABLE_EGRESS_DSCP_MAPPING      */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_ENABLE_EGRESS_EXP_MAPPING       */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_ENABLE_EGRESS_TC_DP_MAPPING     */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_QOS_MAPPING_TABLE_INDEX         */
    STANDARD_FIELD_MAC(4),
/*SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_ROUTER_MAC_SA_INDEX             */
    STANDARD_FIELD_MAC(8),
/*SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_DP_TO_CFI_MAP_ENABLE             */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_PUSH_SOURCE_BASED_MPLS_LABEL     */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_PUSH_EVLAN_BASED_MPLS_LABEL      */
    STANDARD_FIELD_MAC(1)
};


static SNET_ENTRY_FORMAT_TABLE_STC sip5_20HaEPort1TableFieldsFormat[
    SMEM_LION3_HA_EPORT_TABLE_1_FIELDS____LAST_VALUE___E] =
{
/*SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_VLAN_TRANS_EN                   */
    STANDARD_FIELD_MAC(2),/* was 1 bit in sip5*/
/*SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_EGRESS_TAG0_TPID_INDEX          */
    STANDARD_FIELD_MAC(3),
/*SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_EGRESS_TAG1_TPID_INDEX          */
    STANDARD_FIELD_MAC(3),
/*SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_TS_HEADER_TPID_INDEX            */
    STANDARD_FIELD_MAC(3),

    /* TunnelType != "MPLS" */

        /*SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_TS_EXT                          */
        {FIELD_SET_IN_RUNTIME_CNS,
         32,
         SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_TS_HEADER_TPID_INDEX},

    /* TunnelType == "MPLS" */

        /*SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_MPLS_PW_LABEL_PUSH_ENABLE       */
        {FIELD_SET_IN_RUNTIME_CNS,
         1,
         SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_TS_HEADER_TPID_INDEX},
        /*SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_MPLS_PW_LABEL                   */
        STANDARD_FIELD_MAC(20),
        /*SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_MPLS_PW_LABEL_EXP               */
        STANDARD_FIELD_MAC(3),
        /*SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_MPLS_PW_LABEL_TTL               */
        STANDARD_FIELD_MAC(8),
        /*SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_PUSH_MPLS_FLOW_LABEL_ENABLE     */
        STANDARD_FIELD_MAC(1),
        /*SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_FORCE_ARP_TS_EPORT_DECISION     */
        STANDARD_FIELD_MAC(1),
        /*SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_TUNNEL_START                    */
        STANDARD_FIELD_MAC(1),

    /*TunnelStart = "1"*/
        /*SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_TS_POINTER                      */
        {FIELD_SET_IN_RUNTIME_CNS,
         16,/*was 14 in sip5_15*/
         SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_TUNNEL_START},

    /*TunnelStart = "0"*/
        /*SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_ARP_POINTER                     */
        {FIELD_SET_IN_RUNTIME_CNS,
         18,/*was 16 in sip5_15*/
         SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_TUNNEL_START},

    /*SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_TS_PASSENGER_TYPE               */
        {FIELD_SET_IN_RUNTIME_CNS,
         1,
         SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_TS_POINTER},

/*SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_MODIFY_MAC_SA                   */
    {FIELD_SET_IN_RUNTIME_CNS,
     1,
     SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_ARP_POINTER},

/*SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_MODIFY_MAC_DA                   */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_TRILL_INTERFACE_ENABLE          */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_KEEP_ORIGINAL_TAG0_TPID         */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_KEEP_ORIGINAL_TAG1_TPID         */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_ENABLE_EGRESS_UP_MAPPING        */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_ENABLE_EGRESS_DSCP_MAPPING      */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_ENABLE_EGRESS_EXP_MAPPING       */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_ENABLE_EGRESS_TC_DP_MAPPING     */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_QOS_MAPPING_TABLE_INDEX         */
    STANDARD_FIELD_MAC(4),
/*SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_ROUTER_MAC_SA_INDEX             */
    STANDARD_FIELD_MAC(8),
/*SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_DP_TO_CFI_MAP_ENABLE             */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_PUSH_SOURCE_BASED_MPLS_LABEL     */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_PUSH_EVLAN_BASED_MPLS_LABEL      */
    STANDARD_FIELD_MAC(1),
/*SMEM_SIP6_HA_EPORT_TABLE_1_FIELDS_PHA_THREAD_NUMBER                 */
    STANDARD_FIELD_MAC(8),
/*SMEM_SIP6_10_HA_EPORT_TABLE_1_FIELDS_NESTED_VLAN_EN                 */
    STANDARD_FIELD_MAC(1),
/*SMEM_SIP6_30_HA_EPORT_TABLE_1_FIELDS_NESTED_VLAN_MODE_E */ /*97..98*/
    STANDARD_FIELD_MAC(2),
/*SMEM_SIP6_30_HA_EPORT_TABLE_1_FIELDS_HSR_PRP_LAN_ID_E   */ /*99..102*/
    STANDARD_FIELD_MAC(4),
/*SMEM_SIP6_30_HA_EPORT_TABLE_1_FIELDS_L2NAT_ENABLE_E     */ /*103*/
    STANDARD_FIELD_MAC(1),
    /* SMEM_SIP6_30_HA_EPORT_TABLE_1_FIELDS_NAT_PTR_E */
    {46,
     16,
     SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_TUNNEL_START}
};

#define SMEM_LION3_HA_EPORT_TABLE_2_FIELDS_NAME  \
     STR(SMEM_LION3_HA_EPORT_TABLE_2_MIRROR_TO_ANALYZER_KEEP_TAGS   )\
    ,STR(SMEM_LION3_HA_EPORT_TABLE_2_TO_ANALYZER_VLAN_TAG_ADD_EN    )\
    ,STR(SMEM_LION3_HA_EPORT_TABLE_2_PUSH_VLAN_COMMAND              )\
    ,STR(SMEM_LION3_HA_EPORT_TABLE_2_PUSHED_TAG_TPID_SELECT         )\
    ,STR(SMEM_LION3_HA_EPORT_TABLE_2_PUSHED_TAG_VALUE               )\
    ,STR(SMEM_LION3_HA_EPORT_TABLE_2_UP_CFI_ASSIGNMENT_COMMAND      )\
    ,STR(SMEM_LION3_HA_EPORT_TABLE_2_UP                             )\
    ,STR(SMEM_LION3_HA_EPORT_TABLE_2_CFI                            )\
    ,STR(SMEM_LION3_HA_EPORT_TABLE_2_FORCE_E_TAG_IE_PID_TO_DEFAULT  )

char * lion3HaEPort2FieldsTableNames[
    SMEM_LION3_HA_EPORT_TABLE_2_FIELDS____LAST_VALUE___E]=
    {SMEM_LION3_HA_EPORT_TABLE_2_FIELDS_NAME};

SNET_ENTRY_FORMAT_TABLE_STC lion3HaEPort2TableFieldsFormat[
    SMEM_LION3_HA_EPORT_TABLE_2_FIELDS____LAST_VALUE___E]=
{
/*SMEM_LION3_HA_EPORT_TABLE_2_MIRROR_TO_ANALYZER_KEEP_TAGS   */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_HA_EPORT_TABLE_2_TO_ANALYZER_VLAN_TAG_ADD_EN    */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_HA_EPORT_TABLE_2_PUSH_VLAN_COMMAND              */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_HA_EPORT_TABLE_2_PUSHED_TAG_TPID_SELECT         */
    STANDARD_FIELD_MAC(3),
/*SMEM_LION3_HA_EPORT_TABLE_2_PUSHED_TAG_VALUE               */
    STANDARD_FIELD_MAC(12),
/*SMEM_LION3_HA_EPORT_TABLE_2_UP_CFI_ASSIGNMENT_COMMAND      */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_HA_EPORT_TABLE_2_UP                             */
    STANDARD_FIELD_MAC(3),
/*SMEM_LION3_HA_EPORT_TABLE_2_CFI                            */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_HA_EPORT_TABLE_2_FORCE_E_TAG_IE_PID_TO_DEFAULT  */
    STANDARD_FIELD_MAC(1)
};

SNET_ENTRY_FORMAT_TABLE_STC sip5_10HaEPort2TableFieldsFormat[
    SMEM_LION3_HA_EPORT_TABLE_2_FIELDS____LAST_VALUE___E]=
{
/*SMEM_LION3_HA_EPORT_TABLE_2_MIRROR_TO_ANALYZER_KEEP_TAGS   */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_HA_EPORT_TABLE_2_TO_ANALYZER_VLAN_TAG_ADD_EN    */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_HA_EPORT_TABLE_2_PUSH_VLAN_COMMAND              */
    STANDARD_FIELD_MAC(2),/*was 1 bit in sip5 */
/*SMEM_LION3_HA_EPORT_TABLE_2_PUSHED_TAG_TPID_SELECT         */
    STANDARD_FIELD_MAC(3),
/*SMEM_LION3_HA_EPORT_TABLE_2_PUSHED_TAG_VALUE               */
    STANDARD_FIELD_MAC(12),
/*SMEM_LION3_HA_EPORT_TABLE_2_UP_CFI_ASSIGNMENT_COMMAND      */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_HA_EPORT_TABLE_2_UP                             */
    STANDARD_FIELD_MAC(3),
/*SMEM_LION3_HA_EPORT_TABLE_2_CFI                            */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_HA_EPORT_TABLE_2_FORCE_E_TAG_IE_PID_TO_DEFAULT  */
    STANDARD_FIELD_MAC(1)
};

#define SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_NAME                    \
     STR(SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_TUNNEL_TYPE             )\
    ,STR(SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_UP_MARKING_MODE         )\
    ,STR(SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_UP                      )\
    ,STR(SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_TAG_ENABLE              )\
    ,STR(SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_VID                     )\
    ,STR(SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_TTL                     )\
    ,STR(SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_RETAIN_INNER_CRC        )\
    ,STR(SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_RESERVED                )\
    ,STR(SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_B_SA_ASSIGN_MODE        )\
    ,STR(SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_GRE_KEY_MODE            )\
    ,STR(SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_PROTOCOL                )\
    ,STR(SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_NEXT_HOP_MAC_DA         )\
    ,STR(SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_GRE_PROTOCOL            )\
    ,STR(SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_RESERVED_1              )\
    ,STR(SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_IPV4_GRE_KEY_1          )\
    ,STR(SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_IP_PROTOCOL          )\
    ,STR(SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_UDP_DST_PORT            )\
    ,STR(SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_DSCP                    )\
    ,STR(SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_DSCP_MARKING_MODE       )\
    ,STR(SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_FLOW_LABEL_MODE         )\
    ,STR(SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_DF_FLAG                 )\
    ,STR(SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_RESERVED_2              )\
    ,STR(SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_RID                     )\
    ,STR(SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_CAPWAP_FLAGS            )\
    ,STR(SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_AUTO_TUNNEL_OFFSET      )\
    ,STR(SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_AUTO_TUNNEL_FLAG        )\
    ,STR(SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_IPV4_GRE_ENABLE         )\
    ,STR(SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_IPV4_GRE_KEY_0          )\
    ,STR(SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_RESERVED_4              )\
    ,STR(SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_TRILL_HEADER            )\
    ,STR(SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_LABEL1                  )\
    ,STR(SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_EXP1                    )\
    ,STR(SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_EXP1_MARKING_MODE       )\
    ,STR(SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_NUMBER_OF_MPLS_LABELS   )\
    ,STR(SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_RESERVED_3              )\
    ,STR(SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_SET_S_BIT               )\
    ,STR(SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_SWAP_TTL_MODE           )\
    ,STR(SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_MPLS_MC_UPSTREAM_ASSIGNED_LABEL)\
    ,STR(SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_RESERVED_7              )\
    ,STR(SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_LABEL2                  )\
    ,STR(SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_EXP2                    )\
    ,STR(SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_EXP2_MARKING_MODE       )\
    ,STR(SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_PW_CONTROL_INDEX        )\
    ,STR(SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_RESERVED_5              )\
    ,STR(SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_LABEL3                  )\
    ,STR(SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_EXP3                    )\
    ,STR(SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_EXP3_MARKING_MODE       )\
    ,STR(SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_I_SID                   )\
    ,STR(SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_I_UP_MARKING_MODE       )\
    ,STR(SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_RESERVED_6              )\
    ,STR(SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_I_UP                    )\
    ,STR(SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_I_DEI                   )\
    ,STR(SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_I_DEI_MARKING_MODE      )\
    ,STR(SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_I_SID_ASSIGN_MODE       )\
    ,STR(SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_I_TAG_RES2              )\
    ,STR(SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_I_TAG_RES1              )\
    ,STR(SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_I_TAG_NCA               )\
    ,STR(SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_B_DA_ASSIGN_MODE        )\
    ,STR(SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_PROFILE_NUMBER          )\
    ,STR(SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_GRE_FLAGS_AND_VERSION   )\
    ,STR(SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_UDP_SRC_PORT            )\
    ,STR(SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_DIP                     )\
    ,STR(SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_SIP                     )\
    ,STR(SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_DIP_IPV6                )\
    ,STR(SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_SIP_IPV6                )\
    ,STR(SMEM_SIP5_10_HA_TUNNEL_START_TABLE_FIELDS_MPLS_PW_EXP_MARKING_MODE )\
    ,STR(SMEM_SIP5_15_HA_TUNNEL_START_TABLE_FIELDS_PUSH_ELI_AND_EL_AFTER_LABEL_1)\
    ,STR(SMEM_SIP5_15_HA_TUNNEL_START_TABLE_FIELDS_PUSH_ELI_AND_EL_AFTER_LABEL_2)\
    ,STR(SMEM_SIP5_15_HA_TUNNEL_START_TABLE_FIELDS_PUSH_ELI_AND_EL_AFTER_LABEL_3)\
    ,STR(SMEM_SIP7_HA_TUNNEL_START_TABLE_FIELDS_IPV4_MAC_SA_INDEX_E)\
    ,STR(SMEM_SIP7_HA_TUNNEL_START_TABLE_FIELDS_IPV6_MAC_SA_INDEX_E)\
    ,STR(SMEM_SIP7_HA_TUNNEL_START_TABLE_FIELDS_NUMBER_OF_SEGMENTS_E)\
    ,STR(SMEM_SIP7_HA_TUNNEL_START_TABLE_FIELDS_TAG0_QOS_MARKING_MODE_E)\
    ,STR(SMEM_SIP7_HA_TUNNEL_START_TABLE_FIELDS_TAG1_QOS_MARKING_MODE_E)\
    ,STR(SMEM_SIP7_HA_TUNNEL_START_TABLE_FIELDS_UP0_E)\
    ,STR(SMEM_SIP7_HA_TUNNEL_START_TABLE_FIELDS_DEI0_E)\
    ,STR(SMEM_SIP7_HA_TUNNEL_START_TABLE_FIELDS_UP1_E)\
    ,STR(SMEM_SIP7_HA_TUNNEL_START_TABLE_FIELDS_DEI1_E)\
    ,STR(SMEM_SIP7_HA_TUNNEL_START_TABLE_FIELDS_ANALYZER_INDEX_E)\
    ,STR(SMEM_SIP7_HA_TUNNEL_START_TABLE_FIELDS_CNC_PTR_E)\
    ,STR(SMEM_SIP7_HA_TUNNEL_START_TABLE_FIELDS_FLOW_ID_GCF_ASSIGNMENT_PROFILE_E)\
    ,STR(SMEM_SIP7_HA_TUNNEL_START_TABLE_FIELDS_FLOW_ID_GCF_BITS_E)\
    ,STR(SMEM_SIP7_HA_TUNNEL_START_TABLE_FIELDS_FLOW_LABEL_MODE_E)\
    ,STR(SMEM_SIP7_HA_TUNNEL_START_TABLE_FIELDS_FLOW_LABEL_ENTROPY_MODE_E)\
    ,STR(SMEM_SIP7_HA_TUNNEL_START_TABLE_FIELDS_FLOW_LABEL_VALUE_E)\
    ,STR(SMEM_SIP7_HA_TUNNEL_START_TABLE_FIELDS_THREAD_ID_E)\
    ,STR(SMEM_SIP7_HA_TUNNEL_START_TABLE_FIELDS_EGRESS_MIRRORING_MODE_E)\
    ,STR(SMEM_SIP7_HA_TUNNEL_START_TABLE_FIELDS_RAW_LABELS_E)\
    ,STR(SMEM_SIP7_HA_TUNNEL_START_TABLE_FIELDS_MPLS_ETHERTYPE_SELECT_E)\
    ,STR(SMEM_SIP7_HA_TUNNEL_START_TABLE_FIELDS_FLOW_ID_GCF_PROFILE_ID_E)\
    ,STR(SMEM_SIP7_HA_TUNNEL_START_TABLE_FIELDS_NUMBER_OF_LABELS_E)\
    ,STR(SMEM_SIP7_HA_TUNNEL_START_TABLE_FIELDS_LAST_QOS_UNIFORM_LABEL_E)\
    ,STR(SMEM_SIP7_HA_TUNNEL_START_TABLE_FIELDS_QOS_MAPPING_TABLE_INDEX_E)\
    ,STR(SMEM_SIP7_HA_TUNNEL_START_TABLE_FIELDS_LAST_TTL_UNIFORM_LABEL_E)\
    ,STR(SMEM_SIP7_HA_TUNNEL_START_TABLE_FIELDS_CW_INDEX_E)\
    ,STR(SMEM_SIP7_HA_TUNNEL_START_TABLE_FIELDS_FLOW_LABEL_INSERTION_E)

static char * lion3HaTunnelStartFieldsTableNames[
    SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS____LAST_VALUE___E] =
    {SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_NAME};

static SNET_ENTRY_FORMAT_TABLE_STC lion3TunnelStartTableFieldsFormat[
    SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS____LAST_VALUE___E] =
{
/*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_TUNNEL_TYPE           */
    STANDARD_FIELD_MAC(3),
/*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_UP_MARKING_MODE       */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_UP                    */
    STANDARD_FIELD_MAC(3),
/*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_TAG_ENABLE            */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_VID                   */
    STANDARD_FIELD_MAC(12),
/*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_TTL                   */
    STANDARD_FIELD_MAC(8),
/*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_RETAIN_INNER_CRC      */
    /*TunnelType = "MPLS" or TunnelType = "MacInMac" or TunnelType = "CAPWAP"*/
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_RESERVED              */
    STANDARD_FIELD_MAC(1),
    /*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_B_SA_ASSIGN_MODE              */
    /*TunnelType = "MacInMac*/
    STANDARD_FIELD_MAC(1),


    /*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_GRE_KEY_MODE          */
        {FIELD_SET_IN_RUNTIME_CNS,
         2,
         SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_RESERVED},

    /*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_PROTOCOL              */
        {FIELD_SET_IN_RUNTIME_CNS,
         2,
         SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_RESERVED},


/*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_NEXT_HOP_MAC_DA       */
    STANDARD_FIELD_MAC(48),


    /*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_GRE_PROTOCOL          */
        /*TunnelType = "IPv4"*/
        {FIELD_SET_IN_RUNTIME_CNS,
         16,
         SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_NEXT_HOP_MAC_DA},

    /*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_RESERVED_1            */
        {FIELD_SET_IN_RUNTIME_CNS,
         16,
         SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_NEXT_HOP_MAC_DA},

    /*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_IPV4_GRE_KEY_1*/
        /*TunnelType = "IPv4"*/
        {FIELD_SET_IN_RUNTIME_CNS,
         16,
         SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_NEXT_HOP_MAC_DA},
    /*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_IP_PROTOCOL*/
        {FIELD_SET_IN_RUNTIME_CNS,
         8,
         SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_NEXT_HOP_MAC_DA},

    /*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_UDP_DST_PORT*/
        /*TunnelType = "CAPWAP"*/
        {FIELD_SET_IN_RUNTIME_CNS,
         16,
         SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_NEXT_HOP_MAC_DA},


/*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_DSCP                  */
    /*TunnelType = "IPv4" or TunnelType = "CAPWAP"*/
    STANDARD_FIELD_MAC(6),
/*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_DSCP_MARKING_MODE     */
    /*TunnelType = "IPv4" or TunnelType = "CAPWAP"*/
    STANDARD_FIELD_MAC(1),

/*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_FLOW_LABEL_MODE       */
    /*TunnelType = "generic IPv6" */
    {FIELD_SET_IN_RUNTIME_CNS,
     1,
     SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_DSCP_MARKING_MODE},

/*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_DF_FLAG               */
    /*TunnelType = "IPv4" or TunnelType = "CAPWAP"*/
    {FIELD_SET_IN_RUNTIME_CNS,
     1,
     SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_DSCP_MARKING_MODE},

    /*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_RESERVED_2            */
        {FIELD_SET_IN_RUNTIME_CNS,
         24,
         SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_DF_FLAG},

    /*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_RID*/
        /*TunnelType = "CAPWAP"*/
        {FIELD_SET_IN_RUNTIME_CNS,
         5,
         SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_DF_FLAG},
    /*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_CAPWAP_FLAGS*/
        /*TunnelType = "CAPWAP"*/
        STANDARD_FIELD_MAC(3),

    /*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_AUTO_TUNNEL_OFFSET    */
        /*TunnelType = "IPv4"*/
        {FIELD_SET_IN_RUNTIME_CNS,
         4,
         SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_DF_FLAG},
    /*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_AUTO_TUNNEL_FLAG      */
        STANDARD_FIELD_MAC(1),
    /*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_IPV4_GRE_ENABLE*/
        /*TunnelType = "IPv4"*/
        STANDARD_FIELD_MAC(1),
    /*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_IPV4_GRE_KEY_0*/
        /*TunnelType = "IPv4"*/
        STANDARD_FIELD_MAC(16),
    /*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_RESERVED_4*/
        /*TunnelType = "IPv4"*/
        STANDARD_FIELD_MAC(2),

    /*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_TRILL_HEADER*/
        /*TunnelType = "TRILL"*/
        {FIELD_SET_IN_RUNTIME_CNS,
         48,
         SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_UDP_DST_PORT},


    /*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_LABEL1      */
        /*TunnelType = "MPLS"*/
        {FIELD_SET_IN_RUNTIME_CNS,
         20,
         SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_UDP_DST_PORT},
    /*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_EXP1*/
        /*TunnelType = "MPLS"*/
        STANDARD_FIELD_MAC(3),
    /*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_EXP1_MARKING_MODE*/
        STANDARD_FIELD_MAC(1),
    /*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_NUMBER_OF_MPLS_LABELS */
        STANDARD_FIELD_MAC(2),
    /*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_RESERVED_3*/
        STANDARD_FIELD_MAC(1),
    /*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_SET_S_BIT*/
        STANDARD_FIELD_MAC(1),
    /*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_SWAP_TTL_MODE*/
        STANDARD_FIELD_MAC(2),
    /*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_MPLS_MC_UPSTREAM_ASSIGNED_LABEL*/
        STANDARD_FIELD_MAC(1),
    /*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_RESERVED_7*/
        STANDARD_FIELD_MAC(1),
    /*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_LABEL2*/
        STANDARD_FIELD_MAC(20),
    /*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_EXP2*/
        STANDARD_FIELD_MAC(3),
    /*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_EXP2_MARKING_MODE*/
        STANDARD_FIELD_MAC(1),
    /*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_PW_CONTROL_INDEX*/
        STANDARD_FIELD_MAC(4),
    /*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_RESERVED_5*/
        STANDARD_FIELD_MAC(4),
    /*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_LABEL3*/
        STANDARD_FIELD_MAC(20),
    /*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_EXP3*/
        STANDARD_FIELD_MAC(3),
    /*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_EXP3_MARKING_MODE*/
        STANDARD_FIELD_MAC(1),


    /*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_I_SID       */
        /*TunnelType = "MacInMac"*/
        {FIELD_SET_IN_RUNTIME_CNS,
         24,
         SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_UDP_DST_PORT},
    /*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_I_UP_MARKING_MODE*/
        STANDARD_FIELD_MAC(1),
    /*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_RESERVED_6*/
        STANDARD_FIELD_MAC(1),
    /*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_I_UP*/
        STANDARD_FIELD_MAC(3),
    /*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_I_DEI*/
        STANDARD_FIELD_MAC(1),
    /*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_I_DEI_MARKING_MODE*/
        STANDARD_FIELD_MAC(1),
    /*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_I_SID_ASSIGN_MODE*/
        STANDARD_FIELD_MAC(1),
    /*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_I_TAG_RES2*/
        STANDARD_FIELD_MAC(2),
    /*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_I_TAG_RES1*/
        STANDARD_FIELD_MAC(1),
    /*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_I_TAG_NCA*/
        STANDARD_FIELD_MAC(1),
    /*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_B_DA_ASSIGN_MODE*/
        STANDARD_FIELD_MAC(1),

    /*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_PROFILE_NUMBER*/
        /*TunnelType = "generic IPv4"*/
        {FIELD_SET_IN_RUNTIME_CNS,
         3,
         SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_AUTO_TUNNEL_FLAG},

    /*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_GRE_FLAGS_AND_VERSION */
        /*TunnelType = "generic IPv4"*/
        {FIELD_SET_IN_RUNTIME_CNS,
         16,
         SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_PROFILE_NUMBER},

    /*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_UDP_SRC_PORT          */
        /*TunnelType = "generic IPv4"*/
        {FIELD_SET_IN_RUNTIME_CNS,
         16,
         SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_PROFILE_NUMBER},
    /*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_DIP                   */
        STANDARD_FIELD_MAC(32),
    /*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_SIP                   */
        STANDARD_FIELD_MAC(32),

    /*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_DIP_IPV6              */
        /*TunnelType = "generic IPv6"*/
        {FIELD_SET_IN_RUNTIME_CNS,
         128,
         SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_UDP_SRC_PORT},
    /*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_SIP_IPV6                 */
        STANDARD_FIELD_MAC(128),

    /* bobcat B0 */
    /*SMEM_SIP5_10_HA_TUNNEL_START_TABLE_FIELDS_MPLS_PW_EXP_MARKING_MODE*/
    {80,/*startBit*/
     1,
     0},
    /*SMEM_SIP5_15_HA_TUNNEL_START_TABLE_FIELDS_PUSH_ELI_AND_EL_AFTER_LABEL_1*/
        STANDARD_FIELD_MAC(1),
    /*SMEM_SIP5_15_HA_TUNNEL_START_TABLE_FIELDS_PUSH_ELI_AND_EL_AFTER_LABEL_2*/
        STANDARD_FIELD_MAC(1),
    /*SMEM_SIP5_15_HA_TUNNEL_START_TABLE_FIELDS_PUSH_ELI_AND_EL_AFTER_LABEL_3*/
        STANDARD_FIELD_MAC(1),

    /* SIP6 Generic TS entry fields */
    /*SMEM_SIP6_HA_TUNNEL_START_TABLE_FIELDS_GENERIC_TS_TYPE*/
        EXPLICIT_FIELD_MAC(20, 3),
    /*SMEM_SIP6_HA_TUNNEL_START_TABLE_FIELDS_ETHERTYPE*/
        EXPLICIT_FIELD_MAC(80, 16),
    /*SMEM_SIP6_HA_TUNNEL_START_TABLE_FIELDS_DATA1*/
        STANDARD_FIELD_MAC(96),
    /*SMEM_SIP6_HA_TUNNEL_START_TABLE_FIELDS_DATA2*/
        STANDARD_FIELD_MAC(192)
};

static SNET_ENTRY_FORMAT_TABLE_STC sip7TunnelStartTableFieldsFormat[
    SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS____LAST_VALUE___E] =
{
    /*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_TUNNEL_TYPE           */
    STANDARD_FIELD_MAC(3),
    /*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_UP_MARKING_MODE       */
    STANDARD_FIELD_MAC(1),
    /*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_UP                    */
    STANDARD_FIELD_MAC(3),
    /*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_TAG_ENABLE            */
    STANDARD_FIELD_MAC(1),
    /*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_VID                   */
    STANDARD_FIELD_MAC(12),
    /*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_TTL                   */
    STANDARD_FIELD_MAC(8),
    /*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_RETAIN_INNER_CRC      */
    /*TunnelType = "MPLS" or TunnelType = "MacInMac" or TunnelType = "CAPWAP"*/
    STANDARD_FIELD_MAC(0),

        /*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_RESERVED              */
        /* TunnelType = "MPLS" */
        {FIELD_SET_IN_RUNTIME_CNS,
        2,
        SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_TTL},

    /*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_B_SA_ASSIGN_MODE              */
    /*TunnelType = "MacInMac*/
    EXPLICIT_FIELD_MAC(28, 1),

    /*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_GRE_KEY_MODE          */
        {FIELD_SET_IN_RUNTIME_CNS,
         2,
         SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_TTL},

    /*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_PROTOCOL              */
        {FIELD_SET_IN_RUNTIME_CNS,
         2,
         SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_RESERVED},


/*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_NEXT_HOP_MAC_DA       */
     STANDARD_FIELD_MAC(48),


    /*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_GRE_PROTOCOL          */
        /*TunnelType = "IPv4"*/
        {FIELD_SET_IN_RUNTIME_CNS,
         16,
         SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_NEXT_HOP_MAC_DA},

    /*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_RESERVED_1            */
        {FIELD_SET_IN_RUNTIME_CNS,
         16,
         SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_NEXT_HOP_MAC_DA},

    /*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_IPV4_GRE_KEY_1*/
        /*TunnelType = "IPv4"*/
        {FIELD_SET_IN_RUNTIME_CNS,
         16,
         SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_NEXT_HOP_MAC_DA},

    /*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_IP_PROTOCOL*/
        {FIELD_SET_IN_RUNTIME_CNS,
         8,
         SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_NEXT_HOP_MAC_DA},

    /*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_UDP_DST_PORT*/
        /*TunnelType = "CAPWAP"*/
        {FIELD_SET_IN_RUNTIME_CNS,
         16,
         SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_NEXT_HOP_MAC_DA},


/*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_DSCP                  */
    /*TunnelType = "IPv4" or TunnelType = "CAPWAP"*/
    EXPLICIT_FIELD_MAC(96, 6),
/*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_DSCP_MARKING_MODE     */
    /*TunnelType = "IPv4" or TunnelType = "CAPWAP"*/
    STANDARD_FIELD_MAC(1),

/*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_FLOW_LABEL_MODE       */
    /*TunnelType = "generic IPv6" */
    {FIELD_SET_IN_RUNTIME_CNS,
     1,
     SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_DSCP_MARKING_MODE},

/*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_DF_FLAG               */
    /*TunnelType = "IPv4" or TunnelType = "CAPWAP"*/
    {FIELD_SET_IN_RUNTIME_CNS,
     1,
     SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_DSCP_MARKING_MODE},

    /*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_RESERVED_2            */
        {FIELD_SET_IN_RUNTIME_CNS,
         24,
         SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_DF_FLAG},

    /*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_RID*/
        /*TunnelType = "CAPWAP"*/
        {FIELD_SET_IN_RUNTIME_CNS,
         5,
         SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_DF_FLAG},
    /*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_CAPWAP_FLAGS*/
        /*TunnelType = "CAPWAP"*/
        STANDARD_FIELD_MAC(3),

    /*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_AUTO_TUNNEL_OFFSET    */
        /*TunnelType = "IPv4"*/
        {FIELD_SET_IN_RUNTIME_CNS,
         4,
         SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_DF_FLAG},
    /*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_AUTO_TUNNEL_FLAG      */
        STANDARD_FIELD_MAC(1),
    /*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_IPV4_GRE_ENABLE*/
        /*TunnelType = "IPv4"*/
        STANDARD_FIELD_MAC(1),
    /*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_IPV4_GRE_KEY_0*/
        /*TunnelType = "IPv4"*/
        STANDARD_FIELD_MAC(16),
    /*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_RESERVED_4*/
        /*TunnelType = "IPv4"*/
        STANDARD_FIELD_MAC(2),

    /*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_TRILL_HEADER*/
        /*TunnelType = "TRILL"*/
         {FIELD_SET_IN_RUNTIME_CNS,
         48,
         SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_UDP_DST_PORT},

    /*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_LABEL1      */
        /*TunnelType = "MPLS"*/
        {FIELD_SET_IN_RUNTIME_CNS,
         20,
         SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_UDP_DST_PORT},

    /*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_EXP1*/
        /*TunnelType = "MPLS"*/
        STANDARD_FIELD_MAC(3),
    /*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_EXP1_MARKING_MODE*/
        STANDARD_FIELD_MAC(1),
    /*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_NUMBER_OF_MPLS_LABELS */
        STANDARD_FIELD_MAC(2),
    /*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_RESERVED_3*/
        STANDARD_FIELD_MAC(1),
    /*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_SET_S_BIT*/
        STANDARD_FIELD_MAC(1),
    /*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_SWAP_TTL_MODE*/
        STANDARD_FIELD_MAC(2),
    /*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_MPLS_MC_UPSTREAM_ASSIGNED_LABEL*/
        STANDARD_FIELD_MAC(1),
    /*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_RESERVED_7*/
        STANDARD_FIELD_MAC(1),
    /*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_LABEL2*/
        STANDARD_FIELD_MAC(20),
    /*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_EXP2*/
        STANDARD_FIELD_MAC(3),
    /*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_EXP2_MARKING_MODE*/
        STANDARD_FIELD_MAC(1),
    /*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_PW_CONTROL_INDEX*/
        STANDARD_FIELD_MAC(4),
    /*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_RESERVED_5*/
        STANDARD_FIELD_MAC(4),
    /*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_LABEL3*/
        STANDARD_FIELD_MAC(20),
    /*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_EXP3*/
        STANDARD_FIELD_MAC(3),
    /*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_EXP3_MARKING_MODE*/
        STANDARD_FIELD_MAC(1),

    /*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_I_SID       */
        /*TunnelType = "MacInMac"*/
        {FIELD_SET_IN_RUNTIME_CNS,
         24,
         SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_UDP_DST_PORT},

    /*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_I_UP_MARKING_MODE*/
        STANDARD_FIELD_MAC(1),
    /*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_RESERVED_6*/
        STANDARD_FIELD_MAC(1),
    /*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_I_UP*/
        STANDARD_FIELD_MAC(3),
    /*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_I_DEI*/
        STANDARD_FIELD_MAC(1),
    /*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_I_DEI_MARKING_MODE*/
        STANDARD_FIELD_MAC(1),
    /*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_I_SID_ASSIGN_MODE*/
        STANDARD_FIELD_MAC(1),
    /*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_I_TAG_RES2*/
        STANDARD_FIELD_MAC(2),
    /*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_I_TAG_RES1*/
        STANDARD_FIELD_MAC(1),
    /*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_I_TAG_NCA*/
        STANDARD_FIELD_MAC(1),
    /*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_B_DA_ASSIGN_MODE*/
        STANDARD_FIELD_MAC(1),

    /*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_PROFILE_NUMBER*/
        /*TunnelType = "generic IPv4"*/
        {FIELD_SET_IN_RUNTIME_CNS,
         3,
         SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_AUTO_TUNNEL_FLAG},

    /*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_GRE_FLAGS_AND_VERSION */
        /*TunnelType = "generic IPv4"*/
        {FIELD_SET_IN_RUNTIME_CNS,
         16,
         SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_PROFILE_NUMBER},

    /*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_UDP_SRC_PORT          */
        /*TunnelType = "generic IPv4"*/
        {FIELD_SET_IN_RUNTIME_CNS,
         16,
         SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_PROFILE_NUMBER},
    /*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_DIP                   */
        STANDARD_FIELD_MAC(32),
    /*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_SIP                   */
        STANDARD_FIELD_MAC(32),

     /*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_DIP_IPV6              */
        /*TunnelType = "generic IPv6"*/
        {FIELD_SET_IN_RUNTIME_CNS,
         128,
         SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_UDP_SRC_PORT},
    /*SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_SIP_IPV6                 */
        STANDARD_FIELD_MAC(128),


    /*SMEM_SIP5_10_HA_TUNNEL_START_TABLE_FIELDS_MPLS_PW_EXP_MARKING_MODE*/
    {FIELD_SET_IN_RUNTIME_CNS,
     1,
     SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_NEXT_HOP_MAC_DA},
    /*SMEM_SIP5_15_HA_TUNNEL_START_TABLE_FIELDS_PUSH_ELI_AND_EL_AFTER_LABEL_1*/
        STANDARD_FIELD_MAC(1),
    /*SMEM_SIP5_15_HA_TUNNEL_START_TABLE_FIELDS_PUSH_ELI_AND_EL_AFTER_LABEL_2*/
        STANDARD_FIELD_MAC(1),
    /*SMEM_SIP5_15_HA_TUNNEL_START_TABLE_FIELDS_PUSH_ELI_AND_EL_AFTER_LABEL_3*/
        STANDARD_FIELD_MAC(1),

    /* SIP6 Generic TS entry fields */
    /*SMEM_SIP6_HA_TUNNEL_START_TABLE_FIELDS_GENERIC_TS_TYPE*/
        EXPLICIT_FIELD_MAC(20, 3),
    /*SMEM_SIP6_HA_TUNNEL_START_TABLE_FIELDS_ETHERTYPE*/
        {FIELD_SET_IN_RUNTIME_CNS,
         16,
         SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_NEXT_HOP_MAC_DA},
    /*SMEM_SIP6_HA_TUNNEL_START_TABLE_FIELDS_DATA1*/
        STANDARD_FIELD_MAC(96),
    /*SMEM_SIP6_HA_TUNNEL_START_TABLE_FIELDS_DATA2*/
        STANDARD_FIELD_MAC(192),

    /*SMEM_SIP7_HA_TUNNEL_START_TABLE_FIELDS_IPV4_MAC_SA_INDEX_E*/
        EXPLICIT_FIELD_MAC(190, 8),
    /*SMEM_SIP7_HA_TUNNEL_START_TABLE_FIELDS_IPV6_MAC_SA_INDEX_E*/
        EXPLICIT_FIELD_MAC(384, 8),
    /*SMEM_SIP7_HA_TUNNEL_START_TABLE_FIELDS_NUMBER_OF_SEGMENTS_E*/
        EXPLICIT_FIELD_MAC(392, 4),

    /* SIP7 MPLS vPort Tunnel Start Fields */
    /* SMEM_SIP7_HA_TUNNEL_START_TABLE_FIELDS_TAG0_QOS_MARKING_MODE_E */
    {3,
     2,
     SMEM_LION3_HA_TUNNEL_START_TABLE_FIELDS_TUNNEL_TYPE},
    /* SMEM_SIP7_HA_TUNNEL_START_TABLE_FIELDS_TAG1_QOS_MARKING_MODE_E */
        STANDARD_FIELD_MAC(2),
    /* SMEM_SIP7_HA_TUNNEL_START_TABLE_FIELDS_UP0_E */
        STANDARD_FIELD_MAC(2),
    /* SMEM_SIP7_HA_TUNNEL_START_TABLE_FIELDS_DEI0_E */
        STANDARD_FIELD_MAC(1),
    /* SMEM_SIP7_HA_TUNNEL_START_TABLE_FIELDS_UP1_E */
        STANDARD_FIELD_MAC(2),
    /* SMEM_SIP7_HA_TUNNEL_START_TABLE_FIELDS_DEI1_E */
        STANDARD_FIELD_MAC(1),
    /* SMEM_SIP7_HA_TUNNEL_START_TABLE_FIELDS_THREAD_ID_E */
        STANDARD_FIELD_MAC(8),
    /* SMEM_SIP7_HA_TUNNEL_START_TABLE_FIELDS_ANALYZER_INDEX_E */
        STANDARD_FIELD_MAC(6),
    /* SMEM_SIP7_HA_TUNNEL_START_TABLE_FIELDS_EGRESS_MIRRORING_MODE_E */
        STANDARD_FIELD_MAC(2),
    /* SMEM_SIP7_HA_TUNNEL_START_TABLE_FIELDS_RAW_LABELS_E */
        STANDARD_FIELD_MAC(1),
    /* SMEM_SIP7_HA_TUNNEL_START_TABLE_FIELDS_MPLS_ETHERTYPE_SELECT_E */
        STANDARD_FIELD_MAC(3),
    /* SMEM_SIP7_HA_TUNNEL_START_TABLE_FIELDS_CNC_PTR_E */
        STANDARD_FIELD_MAC(19),
    /* SMEM_SIP7_HA_TUNNEL_START_TABLE_FIELDS_FLOW_ID_GCF_PROFILE_ID_E */
        STANDARD_FIELD_MAC(4),
    /* SMEM_SIP7_HA_TUNNEL_START_TABLE_FIELDS_FLOW_ID_GCF_BITS_E */
        STANDARD_FIELD_MAC(32),
    /* SMEM_SIP7_HA_TUNNEL_START_TABLE_FIELDS_NUMBER_OF_LABELS_E */
        STANDARD_FIELD_MAC(4),
    /* SMEM_SIP7_HA_TUNNEL_START_TABLE_FIELDS_LAST_QOS_UNIFORM_LABEL_E */
        STANDARD_FIELD_MAC(4),
    /* SMEM_SIP7_HA_TUNNEL_START_TABLE_FIELDS_QOS_MAPPING_TABLE_INDEX_E */
        STANDARD_FIELD_MAC(3),
    /* SMEM_SIP7_HA_TUNNEL_START_TABLE_FIELDS_LAST_TTL_UNIFORM_LABEL_E */
        STANDARD_FIELD_MAC(4),
    /* SMEM_SIP7_HA_TUNNEL_START_TABLE_FIELDS_CW_INDEX_E */
        STANDARD_FIELD_MAC(4),
    /* SMEM_SIP7_HA_TUNNEL_START_TABLE_FIELDS_FLOW_LABEL_INSERTION_E */
        STANDARD_FIELD_MAC(1),
    /* SMEM_SIP7_HA_TUNNEL_START_TABLE_FIELDS_MPLS_LABEL_STACK_E */
        STANDARD_FIELD_MAC(512),
};

/*sip 5_10 additional fields */

#define SMEM_LION3_HA_NAT44_TABLE_FIELDS_NAME                         \
     STR(SMEM_LION3_HA_NAT44_TABLE_FIELDS_MAC_DA                   )\
    ,STR(SMEM_LION3_HA_NAT44_TABLE_FIELDS_MODIFY_DIP               )\
    ,STR(SMEM_LION3_HA_NAT44_TABLE_FIELDS_NEW_DIP                  )\
    ,STR(SMEM_LION3_HA_NAT44_TABLE_FIELDS_MODIFY_SIP               )\
    ,STR(SMEM_LION3_HA_NAT44_TABLE_FIELDS_NEW_SIP                  )\
    ,STR(SMEM_LION3_HA_NAT44_TABLE_FIELDS_MODIFY_TCP_UDP_DST_PORT  )\
    ,STR(SMEM_LION3_HA_NAT44_TABLE_FIELDS_NEW_TCP_UDP_DST_PORT     )\
    ,STR(SMEM_LION3_HA_NAT44_TABLE_FIELDS_MODIFY_TCP_UDP_SRC_PORT  )\
    ,STR(SMEM_LION3_HA_NAT44_TABLE_FIELDS_NEW_TCP_UDP_SRC_PORT     )\
    ,STR(SMEM_SIP6_30_HA_NAT44_TABLE_FIELDS_NAT_SIP_PREFIX_LENGTH  )\
    ,STR(SMEM_SIP6_30_HA_NAT44_TABLE_FIELDS_NAT_DIP_PREFIX_LENGTH  )\
    ,STR(SMEM_LION3_HA_NAT44_TABLE_FIELDS_NAT_ENTRY_TYPE           )

char * lion3HaNat44FieldsTableNames[
    SMEM_LION3_HA_NAT44_TABLE_FIELDS____LAST_VALUE___E] =
    {SMEM_LION3_HA_NAT44_TABLE_FIELDS_NAME};

SNET_ENTRY_FORMAT_TABLE_STC lion3HaNat44TableFieldsFormat[
    SMEM_LION3_HA_NAT44_TABLE_FIELDS____LAST_VALUE___E] =
{
/*SMEM_LION3_HA_NAT44_TABLE_FIELDS_MAC_DA           */
    STANDARD_FIELD_MAC(48),
/*SMEM_LION3_HA_NAT44_TABLE_FIELDS_MODIFY_DIP       */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_HA_NAT44_TABLE_FIELDS_NEW_DIP          */
    STANDARD_FIELD_MAC(32),
/*SMEM_LION3_HA_NAT44_TABLE_FIELDS_MODIFY_SIP       */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_HA_NAT44_TABLE_FIELDS_NEW_SIP          */
    STANDARD_FIELD_MAC(32),
/*SMEM_LION3_HA_NAT44_TABLE_FIELDS_MODIFY_TCP_UDP_DST_PORT   */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_HA_NAT44_TABLE_FIELDS_NEW_TCP_UDP_DST_PORT      */
    STANDARD_FIELD_MAC(16),
/*SMEM_LION3_HA_NAT44_TABLE_FIELDS_MODIFY_TCP_UDP_SRC_PORT   */
    STANDARD_FIELD_MAC(1),
/*SMEM_LION3_HA_NAT44_TABLE_FIELDS_NEW_TCP_UDP_SRC_PORT     */
    STANDARD_FIELD_MAC(16),
/*SMEM_SIP6_30_HA_NAT44_TABLE_FIELDS_NAT_SIP_PREFIX_LENGTH */
    STANDARD_FIELD_MAC(5),
/*SMEM_SIP6_30_HA_NAT44_TABLE_FIELDS_NAT_DIP_PREFIX_LENGTH */
    STANDARD_FIELD_MAC(5),
/*SMEM_LION3_HA_NAT44_TABLE_FIELDS_NAT_ENTRY_TYPE   */
    EXPLICIT_FIELD_MAC(190,2)
};

/*sip 5_15 additional fields */

#define SMEM_SIP5_15_HA_NAT66_TABLE_FIELDS_NAME                      \
     STR(SMEM_SIP5_15_HA_NAT66_TABLE_FIELDS_MAC_DA                  )\
    ,STR(SMEM_SIP5_15_HA_NAT66_TABLE_FIELDS_MODIFY_COMMAND          )\
    ,STR(SMEM_SIP5_15_HA_NAT66_TABLE_FIELDS_ADDRESS                 )\
    ,STR(SMEM_SIP5_15_HA_NAT66_TABLE_FIELDS_PREFIX_SIZE             )\
    ,STR(SMEM_SIP5_15_HA_NAT66_TABLE_FIELDS_RESERVED                )\
    ,STR(SMEM_SIP5_15_HA_NAT66_TABLE_FIELDS_NAT_ENTRY_TYPE          )

char * sip5_15HaNat66FieldsTableNames[
    SMEM_SIP5_15_HA_NAT66_TABLE_FIELDS____LAST_VALUE___E] =
    {SMEM_SIP5_15_HA_NAT66_TABLE_FIELDS_NAME};

SNET_ENTRY_FORMAT_TABLE_STC sip5_15HaNat66TableFieldsFormat[
    SMEM_SIP5_15_HA_NAT66_TABLE_FIELDS____LAST_VALUE___E] =
{
/*SMEM_SIP5_15_HA_NAT66_TABLE_FIELDS_MAC_DA           */
    STANDARD_FIELD_MAC(48),
/*SMEM_SIP5_15_HA_NAT66_TABLE_FIELDS_MODIFY_COMMAND   */
    STANDARD_FIELD_MAC(2),
/*SMEM_SIP5_15_HA_NAT66_TABLE_FIELDS_ADDRESS          */
    STANDARD_FIELD_MAC(128),
/*SMEM_SIP5_15_HA_NAT66_TABLE_FIELDS_PREFIX_SIZE      */
    STANDARD_FIELD_MAC(6),
/*SMEM_SIP5_15_HA_NAT66_TABLE_FIELDS_RESERVED         */
    STANDARD_FIELD_MAC(6),
/*SMEM_SIP5_15_HA_NAT66_TABLE_FIELDS_NAT_ENTRY_TYPE   */
    STANDARD_FIELD_MAC(2)
};

SNET_ENTRY_FORMAT_TABLE_STC sip6HaNat66TableFieldsFormat[
    SMEM_SIP5_15_HA_NAT66_TABLE_FIELDS____LAST_VALUE___E] =
{
/*SMEM_SIP5_15_HA_NAT66_TABLE_FIELDS_MAC_DA           */
    STANDARD_FIELD_MAC(48),
/*SMEM_SIP5_15_HA_NAT66_TABLE_FIELDS_MODIFY_COMMAND   */
    STANDARD_FIELD_MAC(3),
/*SMEM_SIP5_15_HA_NAT66_TABLE_FIELDS_ADDRESS          */
    STANDARD_FIELD_MAC(128),
/*SMEM_SIP5_15_HA_NAT66_TABLE_FIELDS_PREFIX_SIZE      */
    STANDARD_FIELD_MAC(6),
/*SMEM_SIP5_15_HA_NAT66_TABLE_FIELDS_RESERVED         */
    STANDARD_FIELD_MAC(6),
/*SMEM_SIP5_15_HA_NAT66_TABLE_FIELDS_NAT_ENTRY_TYPE   */
    {190,2,0/*NA*/} /*bit 190-191, fixed startBit position  */
};
#define SMEM_LION3_HA_GENERIC_TS_PROFILE_TABLE_FIELDS_NAME                             \
     STR(SMEM_LION3_HA_GENERIC_TS_PROFILE_TABLE_FIELDS_16_TEMPLATE_BYTES_FOR_8_BITS   )\
    ,STR(SMEM_LION3_HA_GENERIC_TS_PROFILE_TABLE_FIELDS_HASH_CIRCULAR_SHIFT_LEFT       )\
    ,STR(SMEM_LION3_HA_GENERIC_TS_PROFILE_TABLE_FIELDS_TEMPLATE_SIZE                  )\
    ,STR(SMEM_LION3_HA_GENERIC_TS_PROFILE_TABLE_FIELDS_UDP_SOURCE_PORT_MODE           )\
    ,STR(SMEM_LION3_HA_GENERIC_TS_PROFILE_TABLE_FIELDS_SERVICE_ID_CIRCULAR_SHIFT_SIZE )\
    ,STR(SMEM_LION3_HA_GENERIC_TS_PROFILE_TABLE_FIELDS_MAC_DA_MODE                    )\
    ,STR(SMEM_LION3_HA_GENERIC_TS_PROFILE_TABLE_FIELDS_PROFILE_DIP_MODE               )\
    ,STR(SMEM_LION3_HA_GENERIC_TS_PROFILE_TABLE_FIELDS_CONTROL_WORD_INDEX             )\
    ,STR(SMEM_SIP7_HA_GENERIC_TS_PROFILE_TABLE_FIELDS_IPV6_SHR_ROUTING_TYPE           )\
    ,STR(SMEM_SIP7_HA_GENERIC_TS_PROFILE_TABLE_FIELDS_IPV6_SHR_MODE                   )\
    ,STR(SMEM_SIP7_HA_GENERIC_TS_PROFILE_TABLE_FIELDS_THREAD_ID                       )\
    ,STR(SMEM_SIP7_HA_GENERIC_TS_PROFILE_TABLE_FIELDS_EGRESS_MIRRORING_MODE           )\
    ,STR(SMEM_SIP7_HA_GENERIC_TS_PROFILE_TABLE_FIELDS_IP_PROTOCOL                     )\
    ,STR(SMEM_SIP7_HA_GENERIC_TS_PROFILE_TABLE_FIELDS_UDP_DST_PORT                    )\
    ,STR(SMEM_SIP7_HA_GENERIC_TS_PROFILE_TABLE_FIELDS_GRE_PROTOCOL                    )\
    ,STR(SMEM_SIP7_HA_GENERIC_TS_PROFILE_TABLE_FIELDS_SRV6_PACKET_EDITOR_MODE         )


char *lion3HaGenericTsProfileFieldsTableNames[
    SMEM_LION3_HA_GENERIC_TS_PROFILE_TABLE_FIELDS___LAST_VALUE___E] =
    {SMEM_LION3_HA_GENERIC_TS_PROFILE_TABLE_FIELDS_NAME};

SNET_ENTRY_FORMAT_TABLE_STC lion3HaGenericTsProfileTableFieldsFormat
    [SMEM_LION3_HA_GENERIC_TS_PROFILE_TABLE_FIELDS___LAST_VALUE___E] =
{
    /* SMEM_LION3_HA_GENERIC_TS_PROFILE_TABLE_FIELDS_16_TEMPLATE_BYTES_FOR_8_BITS */
         STANDARD_FIELD_MAC((16*8*4))
    /* SMEM_LION3_HA_GENERIC_TS_PROFILE_TABLE_FIELDS_HASH_CIRCULAR_SHIFT_LEFT */
        ,STANDARD_FIELD_MAC(3)
    /*SMEM SMEM_LION3_HA_GENERIC_TS_PROFILE_TABLE_FIELDS_TEMPLATE_SIZE    */
        ,STANDARD_FIELD_MAC(3)
    /*SMEM SMEM_LION3_HA_GENERIC_TS_PROFILE_TABLE_FIELDS_UDP_SOURCE_PORT_MODE    */
        ,STANDARD_FIELD_MAC(1)
    /*SMEM SMEM_LION3_HA_GENERIC_TS_PROFILE_TABLE_FIELDS_SERVICE_ID_CIRCULAR_SHIFT_SIZE */
        ,STANDARD_FIELD_MAC(5)
    /*SMEM SMEM_LION3_HA_GENERIC_TS_PROFILE_TABLE_FIELDS_MAC_DA_MODE */
        ,STANDARD_FIELD_MAC(5)
    /*SMEM SMEM_LION3_HA_GENERIC_TS_PROFILE_TABLE_FIELDS_PROFILE_DIP_MODE */
        ,STANDARD_FIELD_MAC(5)
    /*SMEM_LION3_HA_GENERIC_TS_PROFILE_TABLE_FIELDS_CONTROL_WORD_INDEX */
        ,STANDARD_FIELD_MAC(3)
    /*SMEM_SIP7_HA_GENERIC_TS_PROFILE_TABLE_FIELDS_IPV6_SHR_ROUTING_TYPE*/
        ,STANDARD_FIELD_MAC(8)
    /*SMEM_SIP7_HA_GENERIC_TS_PROFILE_TABLE_FIELDS_IPV6_SHR_MODE*/
        ,STANDARD_FIELD_MAC(1)
    /*SMEM_SIP7_HA_GENERIC_TS_PROFILE_TABLE_FIELDS_THREAD_ID*/
        ,STANDARD_FIELD_MAC(8)
    /*SMEM_SIP7_HA_GENERIC_TS_PROFILE_TABLE_FIELDS_EGRESS_MIRRORING_MODE*/
        ,STANDARD_FIELD_MAC(2)
    /*SMEM_SIP7_HA_GENERIC_TS_PROFILE_TABLE_FIELDS_IP_PROTOCOL*/
    /* protocol = "Generic */
        ,{FIELD_SET_IN_RUNTIME_CNS,
         7,
         SMEM_SIP7_HA_GENERIC_TS_PROFILE_TABLE_FIELDS_EGRESS_MIRRORING_MODE}

    /*protocol = "UDP"*/
    /*SMEM_SIP7_HA_GENERIC_TS_PROFILE_TABLE_FIELDS_UDP_DST_PORT                      */
        ,{FIELD_SET_IN_RUNTIME_CNS,
         16,
         SMEM_SIP7_HA_GENERIC_TS_PROFILE_TABLE_FIELDS_EGRESS_MIRRORING_MODE}

    /*protocol = "GRE"*/
        /*SMEM_SIP7_HA_GENERIC_TS_PROFILE_TABLE_FIELDS_GRE_PROTOCOL                     */
        ,{FIELD_SET_IN_RUNTIME_CNS,
         16,
         SMEM_SIP7_HA_GENERIC_TS_PROFILE_TABLE_FIELDS_EGRESS_MIRRORING_MODE}

    /*SMEM_SIP7_HA_GENERIC_TS_PROFILE_TABLE_FIELDS_SRV6_PACKET_EDITOR_MODE*/
        ,STANDARD_FIELD_MAC(1)
};


/*
 * Typedef: struct SNET_DSA_TAG_FIELDS_STCT
 *
 * Description: A structure to hold info about DSA tag fields
 *
 * Fields:
 *        trgDev            - dsa targed dev field value
 *        useVidx           - dsa useVidx field value
 *        isTrgPhyPortValid - dsa targed phy port field value
 *        trgPhyPort        - dsa targed phy port field value
 *        trgEPort          - dsa targed ePort/port field value
 *        srcTrgDev         - dsa srcTrgDev field value
 *        srcTrgPhysicalPort- dsa srcTrgPhysicalPort field value
 */
typedef struct SNET_DSA_TAG_FIELDS_STCT
{
    GT_U32  trgDev;
    GT_U32  useVidx;
    GT_U32  isTrgPhyPortValid;
    GT_U32  trgPhyPort;
    GT_U32  trgEPort;
    GT_U32  srcTrgDev;
    GT_U32  srcTrgPhysicalPort;
} SNET_DSA_TAG_FIELDS_STC ;


/**
* @internal snetChtHaArpTblEntryGet function
* @endinternal
*
* @brief   Ha - Get ARP table mac address entry
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] entryIndex               - pointer to ARP memory table entry.
*                                      OUTPUT:
* @param[in] arpEntry                 - pointer to ARP entry
*                                       COMMENT:
*/
GT_VOID snetChtHaArpTblEntryGet
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32 entryIndex,
    OUT SGT_MAC_ADDR_TYP * arpEntry
)
{
    DECLARE_FUNC_NAME(snetChtHaArpTblEntryGet);

    GT_U32 regAddr;
    GT_U32 * regPtr;
    GT_U32  numEntriesInMemoryLine;/* number of mac entries in memory line */
    GT_U32  startBit;

    if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        numEntriesInMemoryLine = 8;/* in sip 5 - 8 ARP entries in 1 memory line */
    }
    else
    {
        numEntriesInMemoryLine = 4;/* in ch2..Lion2 - 4 ARP entries in 1 memory line */
    }

    /* need to jump 48 bits from MAC to MAC */
    startBit = 48 * (entryIndex % numEntriesInMemoryLine);

    regAddr = SMEM_CHT_ARP_TBL_MEM(devObjPtr, entryIndex);
    regPtr = smemMemGet(devObjPtr, regAddr);

    /* Pointer to 48-bit MAC Address */
    arpEntry->bytes[0] = (GT_U8)snetFieldValueGet(regPtr, startBit + (5*8), 8);
    arpEntry->bytes[1] = (GT_U8)snetFieldValueGet(regPtr, startBit + (4*8), 8);
    arpEntry->bytes[2] = (GT_U8)snetFieldValueGet(regPtr, startBit + (3*8), 8);
    arpEntry->bytes[3] = (GT_U8)snetFieldValueGet(regPtr, startBit + (2*8), 8);
    arpEntry->bytes[4] = (GT_U8)snetFieldValueGet(regPtr, startBit + (1*8), 8);
    arpEntry->bytes[5] = (GT_U8)snetFieldValueGet(regPtr, startBit + (0*8), 8);


    __LOG(("ARP SA : 6 Bytes [0x%2.2x%2.2x%2.2x%2.2x%2.2x%2.2x] \n",
        arpEntry->bytes[0],
        arpEntry->bytes[1],
        arpEntry->bytes[2],
        arpEntry->bytes[3],
        arpEntry->bytes[4],
        arpEntry->bytes[5]
        ));

}

/**
* @internal snetChtHaPerPortInfoGet function
* @endinternal
*
* @brief   Header Alteration - get indication about the index of per port and use
*         of second register
* @param[in] devObjPtr                -  pointer to device object.
*                                      descrPtr        - pointer to the frame's descriptor.
*                                      egressPort      - egress port (local port on multi-port group device)
*                                      CPU port is port 63
*
* @param[out] isSecondRegisterPtr      - pointer to 'use the second register'
* @param[out] outputPortBitPtr         - pointer to the bit index for the egress port
*/
void snetChtHaPerPortInfoGet
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN GT_U32   localEgressPort,
    OUT GT_BOOL     *isSecondRegisterPtr,
    OUT GT_U32     *outputPortBitPtr
)
{
    if(devObjPtr->supportHa64Ports &&                /* support 64 ports */
       localEgressPort > 31)                              /* port > 31 */
    {
        /* need to use the 'Second register' */
        *isSecondRegisterPtr = GT_TRUE;
        *outputPortBitPtr = localEgressPort - 32;
    }
    else
    {
        *isSecondRegisterPtr = GT_FALSE;

        if(localEgressPort == SNET_CHT_CPU_PORT_CNS)
        {
            /* cpu port is in bit 31 */
            *outputPortBitPtr = 31;
        }
        else if(localEgressPort < 32)
        {
            *outputPortBitPtr = localEgressPort;
        }
        else /* egressPort >=32 */
        {
            /* should not happen , but if happen we need to understand what to do ...*/
            skernelFatalError(" snetChtHaPerPortInfoGet : port[%d] >= 32 ?! \n",localEgressPort);
        }
    }

    return;
}

/**
* @internal snetChtHaEgressEPortEntryGet function
* @endinternal
*
* @brief   function of the HA unit
*         get pointers to both HA egress ePort tables : table1,table2
*         Set value into descrPtr->eArchExtInfo.haEgressEPortAtt1TablePtr
*         Set value into descrPtr->eArchExtInfo.haEgressEPortAtt2TablePtr
*         Set value into descrPtr->eArchExtInfo.epclEgressEPortTablePtr
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - Cht frame descriptor
*                                      localEgressPort - physical egress port -- global port number
*                                      (referred as <LocalDevTrgPhyPort> in documentation ,
*                                      but it is not part of the 'descriptor' !)
*/
static void snetChtHaEgressEPortEntryGet
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32    egressPort
)
{
    DECLARE_FUNC_NAME(snetChtHaEgressEPortEntryGet);

    GT_U32  index1,index2;/* table1 index,table2 index*/
    GT_U32  regAddr;/* register address */

    if ((descrPtr->outGoingMtagCmd == SKERNEL_MTAG_CMD_TO_TRG_SNIFFER_E && descrPtr->rxSniff == 0)/*egress mirror*/ ||
        (descrPtr->outGoingMtagCmd == SKERNEL_MTAG_CMD_TO_CPU_E && descrPtr->srcTrg == 1)/* target */)
    {
        /*
            if (Desc<SrcTrgDev> == OwnDev) // i.e the device that replicated the egress mirrored packet
                Access table with index = Desc<Src Trg ePort>
            Else // on target device access with analyzer target ePort
                if (Desc<UseVIDX>==0) && (Desc<TrgDev> == OwnDev || Desc<AssignTRGePortAttLocaly>==1)
                    Access table with index = Desc<TRGePort>
                Else // Target ePort is not assigned, access with physical port the packet is queued in
                    Access table with (Desc<LocalDevTrgPhyPort>) // use physical port number
        */

        if(descrPtr->txqToEq && descrPtr->outGoingMtagCmd == SKERNEL_MTAG_CMD_TO_CPU_E)
        {
            /* we are from the EQ after the egress pipe did trap/mirror-to-cpu */
            index1 = descrPtr->origDescrPtr->eArchExtInfo.haEgressEPortAtt1Table_index;
            __LOG(("The HA,EPCL use configurations of the 'orig egress eport' [0x%x] and not analyzer/cpu egress port[0x%x] \n",
                index1,
                descrPtr->trgEPort))
        }
        else
        /*the device that replicated the egress mirrored packet*/
        if(SKERNEL_IS_MATCH_DEVICES_MAC(descrPtr->srcTrgDev, descrPtr->ownDev,
                                          devObjPtr->dualDeviceIdEnable.ha))
        {
            index1 = descrPtr->eArchExtInfo.srcTrgEPort;

            /* use the eport that the 'orig descriptor' used */
            __LOG(("The HA,EPCL use configurations of the 'orig egress eport' [0x%x] and not analyzer/cpu egress port[0x%x] \n",
                index1,
                descrPtr->trgEPort));
        }
        else /*on target device access with analyzer target ePort*/
        {
            /* use the index that the 'orig descriptor' used */
            if(descrPtr->useVidx == 0 &&
                ((SKERNEL_IS_MATCH_DEVICES_MAC(descrPtr->trgDev, descrPtr->ownDev,
                                              devObjPtr->dualDeviceIdEnable.ha)) ||
                    descrPtr->eArchExtInfo.assignTrgEPortAttributesLocally ))
            {
                /* use the eport */
                index1 = descrPtr->trgEPort;
            }
            else
            {
                /* use the physical egress port */
                index1 = egressPort;
            }
        }
    }
    else
    {
        if(descrPtr->useVidx == 0 &&
            ((SKERNEL_IS_MATCH_DEVICES_MAC(descrPtr->trgDev, descrPtr->ownDev,
                                          devObjPtr->dualDeviceIdEnable.ha)) ||
                descrPtr->eArchExtInfo.assignTrgEPortAttributesLocally ))
        {
            index1 = descrPtr->trgEPort;
        }
        else
        {
            /* use the physical egress port */
            index1 = egressPort;
        }
    }

    if (descrPtr->outGoingMtagCmd == SKERNEL_MTAG_CMD_TO_TRG_SNIFFER_E ||
        descrPtr->outGoingMtagCmd == SKERNEL_MTAG_CMD_TO_CPU_E)
    {
        if(descrPtr->txqToEq && descrPtr->outGoingMtagCmd == SKERNEL_MTAG_CMD_TO_CPU_E)
        {
            /* we are from the EQ after the egress pipe did trap/mirror-to-cpu */
            index2 = descrPtr->trgEPort;
        }
        else
        {
            /* both ingress and egress mirrored packets access table 2,
                using the analyzer target ePort as an index to this table */
            index2 = descrPtr->eArchExtInfo.toTargetSniffInfo.sniffTrgEPort;
        }
    }
    else
    {
        index2 = index1;
    }

    __LOG(("HA - Egress Eport Attribute Table 1 - access with index[%d] \n",
        index1));
    __LOG(("HA - Egress Eport Attribute Table 2 - access with index[%d] \n",
        index2));

    /* HA - Egress Eport Attribute Table 1 */
    regAddr = SMEM_LION2_HA_EGRESS_EPORT_ATTRIBUTE_1_TBL_MEM(devObjPtr,index1);
    descrPtr->eArchExtInfo.haEgressEPortAtt1TablePtr = smemMemGet(devObjPtr, regAddr);
    descrPtr->eArchExtInfo.haEgressEPortAtt1Table_index = index1;

    /* HA - Egress Eport Attribute Table 2 */
    regAddr = SMEM_LION2_HA_EGRESS_EPORT_ATTRIBUTE_2_TBL_MEM(devObjPtr,index2);
    descrPtr->eArchExtInfo.haEgressEPortAtt2TablePtr = smemMemGet(devObjPtr, regAddr);
    descrPtr->eArchExtInfo.haEgressEPortAtt2Table_index = index2;

    return;
}

/**
* @internal snetChtHaEgressPhyPortEntryGet function
* @endinternal
*
* @brief   function of the HA unit
*         get pointers to the HA physical attributes egress ePort tables
*         Set value into descrPtr->eArchExtInfo.haEgressPhyPort1TablePtr
*         Set value into descrPtr->eArchExtInfo.haEgressPhyPort2TablePtr
*         Set value into descrPtr->eArchExtInfo.haEgressPhyPort2_bySrc_PhysicalPort_TablePtr
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - Cht frame descriptor
* @param[in] egressPort               - physical egress port -- global port
*                                      (referred as <LocalDevTrgPhyPort> in documentation ,
*                                      but it is not part of the 'descriptor' !)
*/
static void snetChtHaEgressPhyPortEntryGet
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32    egressPort
)
{
    DECLARE_FUNC_NAME(snetChtHaEgressPhyPortEntryGet);

    GT_U32  index;/* table indexes*/
    GT_U32  regAddr;/* register address */

    if (descrPtr->isPktMirrorInLocalDev &&
        ((descrPtr->outGoingMtagCmd == SKERNEL_MTAG_CMD_TO_TRG_SNIFFER_E && descrPtr->rxSniff == 0)/*egress mirror*/ ||
        (descrPtr->outGoingMtagCmd == SKERNEL_MTAG_CMD_TO_CPU_E && descrPtr->srcTrg == 1)/* target */))
    {
        if(descrPtr->txqToEq && descrPtr->outGoingMtagCmd == SKERNEL_MTAG_CMD_TO_CPU_E)
        {
            /* we are from the EQ after the egress pipe did trap/mirror-to-cpu */
            index = descrPtr->origDescrPtr->eArchExtInfo.haEgressPhyPort1Table_index;
        }
        else
        {
            /* use the egressPort that the 'orig descriptor' used */
            index = descrPtr->egressTrgPort;
        }
        /* use the egressPort that the 'orig descriptor' used */
        __LOG(("The HA,EPCL use configurations of the 'orig egress physical port' [%d] and not analyzer/cpu egress port[%d] \n",
            index,
            egressPort))
    }
    else
    {
        /* use the egress port */
        index =  egressPort;
    }

    /* HA - Physical port 1 HA Attributes Table */
    regAddr = SMEM_LION2_HA_PHYSICAL_PORT_1_ATTRIBUTES_TBL_MEM(devObjPtr,index);
    descrPtr->eArchExtInfo.haEgressPhyPort1TablePtr = smemMemGet(devObjPtr, regAddr);
    descrPtr->eArchExtInfo.haEgressPhyPort1Table_index = index;

    /* Always accessed by InDesc<LocalDevTrgPhyPort> */
    index = egressPort;

    /* HA - Physical port 2 HA Attributes Table */
    regAddr = SMEM_LION2_HA_PHYSICAL_PORT_2_ATTRIBUTES_TBL_MEM(devObjPtr,index);
    descrPtr->eArchExtInfo.haEgressPhyPort2TablePtr = smemMemGet(devObjPtr, regAddr);
    descrPtr->eArchExtInfo.haEgressPhyPort2Table_index = index;

    __LOG(("HA - Physical port 1 HA Attributes Table - access with index[%d] \n",
        descrPtr->eArchExtInfo.haEgressPhyPort1Table_index));
    __LOG(("HA - Physical port 2 HA Attributes Table - access with index[%d] \n",
        descrPtr->eArchExtInfo.haEgressPhyPort2Table_index));

    return;
}

/**
* @internal snetChtHaEgressTagDataExtGet function
* @endinternal
*
* @brief   HA - build VLAN tag info , in network order
*/
void snetChtHaEgressTagDataExtGet
(
    IN  GT_U8   vpt,
    IN  GT_U16  vid,
    IN  GT_U8   cfiDeiBit,
    IN  GT_U16  etherType,
    OUT GT_U8   tagData[] /* 4 bytes */
)
{
    ASSERT_PTR(tagData);

    /* form ieee802.1q tag - Network order */

    /* ether type -- bits 16..31 (16 bits) */
    /* vpt - bits 13..15 (3 bits) */
    /* cfi/dei - bit 12 (1 bit) */
    /* vid - bits 0..11 (12 bits)*/
    tagData[0] = etherType >> 8;
    tagData[1] = etherType & 0xff;
    tagData[2] =    (((vid & 0xf00) >> 8)   |
                     ((vpt & 0x7) << 5)     |
                     ((cfiDeiBit & 0x1) << 4));
    tagData[3] = vid & 0xff;

    return;
}

/**
* @internal haExtendedTagIsReturnToSrcInterface function
* @endinternal
*
* @brief   HA - part of build BPE tag (6/8) bytes
*         returns 'is trg Interface the same as src interface'
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - Cht frame descriptor
*/
static GT_BOOL haExtendedTagIsReturnToSrcInterface(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32    localEgressPort
)
{
    DECLARE_FUNC_NAME(haExtendedTagIsReturnToSrcInterface);

    GT_U32  egressTrunkId;
    GT_BIT  forceETagPidToDefault;
    GT_BIT  useETagPidToDefault;
    GT_U32  haGlbConfigVal;
    GT_U32  forceIngressE_cid = 0;

    forceETagPidToDefault = SMEM_LION3_HA_EPORT_2_ENTRY_FIELD_AUTO_GET(devObjPtr,
        descrPtr,
        SMEM_LION3_HA_EPORT_TABLE_2_FORCE_E_TAG_IE_PID_TO_DEFAULT);

    if(descrPtr->outGoingMtagCmd != SKERNEL_MTAG_CMD_FORWARD_E)
    {
        __LOG_PARAM_WITH_NAME("use 'default' EPID , due to:",
            descrPtr->outGoingMtagCmd != SKERNEL_MTAG_CMD_FORWARD_E);

        useETagPidToDefault = 1;
    }
    else
    {
        egressTrunkId =
            SMEM_LION3_HA_PHYSICAL_PORT_2_ENTRY_FIELD_AUTO_GET(devObjPtr,
                descrPtr,
                SMEM_LION3_HA_PHYSICAL_PORT_TABLE_2_FIELDS_TRUNK_ID);

        __LOG_PARAM(egressTrunkId);
        __LOG_PARAM(descrPtr->eArchExtInfo.origSrcPhyIsTrunk);
        __LOG_PARAM(descrPtr->eArchExtInfo.origSrcPhyPortTrunk);
        __LOG_PARAM(descrPtr->srcDev);
        __LOG_PARAM(descrPtr->ownDev);

        /* Get the value of <force Ingress_E-CID> field */
        if(SMEM_CHT_IS_SIP6_GET(devObjPtr))
        {
            smemRegGet(devObjPtr, SMEM_XCAT_ROUTE_HA_GLB_CNF_REG(devObjPtr), &haGlbConfigVal);
            forceIngressE_cid = SMEM_U32_GET_FIELD(haGlbConfigVal, 15, 1);

            __LOG_PARAM(forceIngressE_cid);

        }

        /* all next cases will use 'default EPID' expect for last case */
        useETagPidToDefault = 1;

        /* check if Packet source-physical port/LAG == Packet target physical-port/LAG */
        if(descrPtr->eArchExtInfo.origSrcPhyIsTrunk &&
            descrPtr->eArchExtInfo.origSrcPhyPortTrunk != egressTrunkId && forceIngressE_cid == 0)
        {
            __LOG_PARAM_WITH_NAME("use 'default' EPID , due to (not egress from ingress trunk):",
                descrPtr->eArchExtInfo.origSrcPhyPortTrunk != egressTrunkId);
        }
        else
        if (descrPtr->eArchExtInfo.origSrcPhyIsTrunk == 0 &&
            (0 == SKERNEL_IS_MATCH_DEVICES_MAC(descrPtr->srcDev, descrPtr->ownDev,
                    devObjPtr->dualDeviceIdEnable.ha)) && forceIngressE_cid == 0)
        {
            __LOG_PARAM_WITH_NAME("use 'default' EPID , due to (not egress from ingress device):",
                (descrPtr->srcDev != descrPtr->ownDev));
        }
        else
        if (descrPtr->eArchExtInfo.origSrcPhyIsTrunk == 0 &&
            descrPtr->eArchExtInfo.origSrcPhyPortTrunk != localEgressPort && forceIngressE_cid == 0)
        {
            __LOG_PARAM_WITH_NAME("use 'default' EPID , due to (not egress from ingress port):",
                (descrPtr->eArchExtInfo.origSrcPhyPortTrunk != localEgressPort));
        }
        else
        if(forceETagPidToDefault)
        {
            __LOG_PARAM_WITH_NAME("use 'default' EPID , due to:",
                forceETagPidToDefault);
        }
        else
        if(descrPtr->tunnelTerminated)
        {
            __LOG_PARAM_WITH_NAME("use 'default' EPID , due to:",
                descrPtr->tunnelTerminated);
        }
        else
        if(descrPtr->tunnelStart)
        {
            __LOG_PARAM_WITH_NAME("use 'default' EPID , due to:",
                descrPtr->tunnelStart);
        }
        else
        if(descrPtr->doRouterHa)
        {
            __LOG_PARAM_WITH_NAME("use 'default' EPID , due to (mac SA modification):",
                descrPtr->doRouterHa);
        }
        else
        if(descrPtr->eArchExtInfo.haInfo.ePortModifyMacSa)
        {
            __LOG_PARAM_WITH_NAME("use 'default' EPID , due to (mac SA modification):",
                descrPtr->eArchExtInfo.haInfo.ePortModifyMacSa);
        }
        else
        if(descrPtr->VntL2Echo)
        {
            __LOG_PARAM_WITH_NAME("use 'default' EPID , due to (mac SA modification):",
                descrPtr->VntL2Echo);
        }
        else
        {
            useETagPidToDefault = 0;
        }

    }

    if(useETagPidToDefault == 0)
    {
        __LOG(("packet return to same interface that it came from \n"));
    }
    else
    {
        __LOG(("packet egress not from interface that it came from \n"));
    }

    return (useETagPidToDefault == 0) ?
        GT_TRUE : /* return to same interface that came from */
        GT_FALSE; /* return to other interface from where came */
}

/**
* @internal haExtendedTagGetPushVidOfSrcEport function
* @endinternal
*
* @brief   HA - part of build BPE tag (6/8) bytes
*         returns the 'ingressECidBase' (<push vid> of the src EPort)
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - Cht frame descriptor
*/
static GT_U32   haExtendedTagGetPushVidOfSrcEport(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr
)
{
    DECLARE_FUNC_NAME(haExtendedTagGetPushVidOfSrcEport);

    GT_U32  index,*memPtr;
    GT_U32  ingressECidBase;/* BPE : 12 bits of <ingress_E-CID_base> */

    /* the <ingress_E-CID_base> is taken from the 'E-CID' that represent the SRC eport:
        <PUSHED_TAG_VALUE> of the src eport */
    /* HA - Egress Eport Attribute Table 2 */
    index = descrPtr->origSrcEPortOrTrnk;
    memPtr = smemMemGet(devObjPtr,
        SMEM_LION2_HA_EGRESS_EPORT_ATTRIBUTE_2_TBL_MEM(devObjPtr,index));

    ingressECidBase =
        SMEM_LION3_HA_EPORT_2_ENTRY_FIELD_GET(devObjPtr,
            memPtr,
            index,
            SMEM_LION3_HA_EPORT_TABLE_2_PUSHED_TAG_VALUE);

    __LOG_PARAM_WITH_NAME("ingressECidBase from <E-CID_base> (PUSHED_TAG_VALUE) of srcEport",
        ingressECidBase);

    return ingressECidBase;
}

/**
* @internal haExtendedTagGetMcGroup function
* @endinternal
*
* @brief   HA - part of build BPE tag (6/8) bytes
*         returns the target MC group for the tag
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - Cht frame descriptor
*/
static GT_U32 haExtendedTagGetMcGroup(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr
)
{
    DECLARE_FUNC_NAME(haExtendedTagGetMcGroup);

    GT_U32  BPEeTagEPIDOffset;/* The eTag<E-PID> field for multi-destination packets reflects the packets'
                                Multicast group.
                                This value is set to eVIDX value +/- this configurable offset (according to <BPE ETag EPID Offset sign>).
                                NOTE: The user must make sure that no wraparound (overflow / underflow) occurs.
                                since Etag<EPID> is 14 bits, so does the offset (and only the evidx[13:0] is used for this calc')*/
    GT_U32  BPEeTagEPIDOffsetSign;/*Indicates the sign (+/-) of <BPE eTag EPID Offset>
                            0 = Positive: <BPE ETag EPID Offset> is a positive value.
                                    should be added to the eVidx value to generate the Etag<EPID>
                            1 = Negative: <BPE ETag EPID Offset> is a negative value.
                                    should be subtracted from the eVidx value to generate the Etag<EPID>*/
    GT_U32  eVidx_14bits;
    GT_U32  BPEConfigReg1Val;/* value of registers : BPE Config Reg 1,2 */
    GT_U32  eCidBaseAndGrp; /* BPE : 12 bits <E-CID_base> + 2 bits <GRP> */

    smemRegGet(devObjPtr, SMEM_LION3_HA_BPE_CONFIG_1_REG(devObjPtr), &BPEConfigReg1Val);

    /* BPE ETag EPID Offset*/
    BPEeTagEPIDOffset = SMEM_U32_GET_FIELD(BPEConfigReg1Val, 12 , 14);
    __LOG_PARAM_WITH_NAME("<BPE ETag EPID Offset> (from register) ",BPEeTagEPIDOffset);
    BPEeTagEPIDOffsetSign = SMEM_U32_GET_FIELD(BPEConfigReg1Val, 26 , 1);
    /*BPE ETag EPID Offset Sign*/
    __LOG_PARAM_WITH_NAME("<BPE ETag EPID Offset Sign> (from register) ",BPEeTagEPIDOffsetSign);

    eVidx_14bits = (descrPtr->eVidx & 0x3FFF);

    if(BPEeTagEPIDOffsetSign == 0)
    {
        /* Positive: <BPE ETag EPID Offset> is a positive value. should be added to the eVidx value to generate the Etag<EPID> */

        eCidBaseAndGrp = (eVidx_14bits + BPEeTagEPIDOffset) & 0x3FFF;
        __LOG_PARAM_WITH_NAME("eCidBaseAndGrp", (eVidx_14bits + BPEeTagEPIDOffset) & 0x3FFF );
    }
    else
    {
        eCidBaseAndGrp = (eVidx_14bits - BPEeTagEPIDOffset) & 0x3FFF;
        __LOG_PARAM_WITH_NAME("eCidBaseAndGrp", (eVidx_14bits - BPEeTagEPIDOffset) & 0x3FFF);
    }

    return eCidBaseAndGrp;
}
/**
* @internal snetChtHaEgressBuild6BytesTag function
* @endinternal
*
* @brief   HA - build 6 Bytes v-Tag
*         NOTE: The tag is pushed as the outer tag of the packet
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - Cht frame descriptor
* @param[in] localEgressPort          - physical egress port -- local port number (port 0..15) !!!
*                                      (referred as <LocalDevTrgPhyPort> in documentation ,
*                                      but it is not part of the 'descriptor' !)
* @param[in,out] haInfoPtr                - pointer to EVB/BPE tags
* @param[in] etherType                - etherType
* @param[in,out] haInfoPtr                - pointer to EVB/BPE tags , filled with info
* @param[in,out] haInfoPtr                - EVB/BPE tags length (0/4/8/6)
* @param[in,out] haInfoPtr                - is EVB on passenger in case of TS.
*                                      0 - the EVB/BPE on TS header (in case of TS)
*                                      1 - the EVB/BPE on Passenger (in case of TS)
*                                      if non TS --> both behave the same.
*/
static GT_VOID snetChtHaEgressBuild6BytesTag
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32    localEgressPort,
    INOUT HA_INTERNAL_INFO_STC *haInfoPtr,
    IN GT_U32   etherType
)
{
    DECLARE_FUNC_NAME(snetChtHaEgressBuild6BytesTag);

    GT_U32 regAddr;
    GT_U32 regValue;
    GT_BIT tagPortExtenderEnabled;/* Specifies how to generate a 6-bytes-Tag.
             0x0 = Disable; Disable; The device works as a Controlling Bridge: The 6-bytes-Tag Looped flag and Direction flag are determined by the device.
             0x1 = Enable; Enable; The device works as a Port Extender: The 6-bytes-Tag Looped flag and Direction flag are taken from the incoming packet (transferred via the Source-ID).
            */
    GT_U32  tagSrcId_L_Bit; /* L bit in src-id field */
    GT_U32  tagSrcId_D_Bit; /* D bit in src-id field */
    GT_BIT  dBit,pBit,lBit,rBit=0;/*single bit*/
    GT_U32  ver = 0;/*2bits*/
    GT_U32  pushedVidFromSrcEport;/* ePort<Push-VID> where the table is accessed with desc<Src-ePort> */
    GT_U32  pushedVidFromTrgEport;/* ePort<Push-VID> where the table is accessed with desc<Trg-ePort>. 2 MSB are zero. */
    GT_U32  eCidBaseAndGrp; /* BPE : 12 bits <E-CID_base> + 2 bits <GRP> */
    GT_U32  startBit; /* start bit in the register */
    GT_BIT  remotePhyPortMapEnable;
    GT_BIT  buildMcTag;

    remotePhyPortMapEnable =
        SMEM_LION3_HA_PHYSICAL_PORT_2_ENTRY_FIELD_AUTO_GET(devObjPtr,
                                                           descrPtr,
                                                           SMEM_LION3_HA_PHYSICAL_PORT_TABLE_2_FIELDS_REMOTE_PHYSICAL_PORT_MAP_ENABLE);

    if(descrPtr->useVidx == 0)
    {
        __LOG(("Build UC 6-bytes-Tag due to useVidx = 0 \n"));
        buildMcTag = 0;
    }
    else
    if(SMEM_CHT_IS_SIP6_GET(devObjPtr) &&
       remotePhyPortMapEnable)
    {
        /* fix JARA : HA-3657 : A MC E-Tag/VN-Tag instead of UC E-Tag/VN-Tag is pushed to replicated packets to remote physical ports*/
        __LOG(("Sip6 : Build UC 6-bytes-Tag even though useVidx = 1 , because Target Phy Port<Remote Physical Port Map Enable> != 0 \n"));
        buildMcTag = 0;
    }
    else
    {
        __LOG(("Build MC 6-bytes-Tag due to useVidx = 1 \n"));
        buildMcTag = 1;
    }

    regAddr = SMEM_XCAT_ROUTE_HA_GLB_CNF_REG(devObjPtr);
    smemRegGet(devObjPtr, regAddr , &regValue);

    startBit = SMEM_CHT_IS_SIP5_20_GET(devObjPtr) ? 26 : 25;

    tagPortExtenderEnabled = SMEM_U32_GET_FIELD(regValue, startBit + 4, 1);
    tagSrcId_L_Bit  = SMEM_U32_GET_FIELD(regValue, startBit , 4);

    regAddr = SMEM_LION2_HA_GLOBAL_CONF1_REG(devObjPtr);
    smemRegGet(devObjPtr, regAddr , &regValue);
    tagSrcId_D_Bit         = SMEM_U32_GET_FIELD(regValue, 26, 4);

    __LOG_PARAM(tagPortExtenderEnabled);
    __LOG_PARAM(tagSrcId_L_Bit);
    __LOG_PARAM(tagSrcId_D_Bit);

    /* push 6-bytes-Tag */

    pBit = buildMcTag;
    __LOG_PARAM_WITH_NAME("6-bytes-Tag : pBit from :", buildMcTag);

    if(tagPortExtenderEnabled == 0)/*Control Bridge*/
    {
        __LOG(("6-bytes-Tag : D bit : always 1 on control bridge \n"));
        dBit = 1;

       if(buildMcTag)
       {
            if(GT_TRUE == haExtendedTagIsReturnToSrcInterface(devObjPtr,descrPtr,localEgressPort))
            {
                __LOG(("6-bytes-Tag : L bit : MC : 1 because return to src interface \n"));
                lBit = 1;
            }
            else
            {
                __LOG(("6-bytes-Tag : L bit : MC : 0 because NOT return to src interface \n"));
                lBit = 0;
            }
        }
        else
        {
            lBit = 0;
            __LOG(("6-bytes-Tag : L bit : UC : always 0 on control bridge \n"));
        }

    }
    else  /* port extender */
    {
        __LOG_PARAM(descrPtr->sstId);

        if(tagSrcId_L_Bit >= devObjPtr->flexFieldNumBitsSupport.sstId)
        {
            __LOG(("6-bytes-Tag : WARNING : Probably configuration ERROR : L_Bit uses bit [%d] from srcId ,but srcId limited to only [%d] bits \n",
                tagSrcId_L_Bit))
        }

        if(tagSrcId_D_Bit >= devObjPtr->flexFieldNumBitsSupport.sstId)
        {
            __LOG(("6-bytes-Tag : WARNING : Probably configuration ERROR : D_Bit uses bit [%d] from srcId ,but srcId limited to only [%d] bits \n",
                tagSrcId_D_Bit))
        }

        if(tagSrcId_L_Bit == tagSrcId_D_Bit)
        {
            __LOG(("6-bytes-Tag : WARNING : Probably configuration ERROR : L_Bit and D_Bit uses the same bit [%d] from srcId \n",
                tagSrcId_L_Bit))
        }

        dBit = SMEM_U32_GET_FIELD(descrPtr->sstId,tagSrcId_D_Bit,1);
        lBit = SMEM_U32_GET_FIELD(descrPtr->sstId,tagSrcId_L_Bit,1);

        __LOG_PARAM_WITH_NAME("6-bytes-Tag : dBit from :",
            SMEM_U32_GET_FIELD(descrPtr->sstId,tagSrcId_D_Bit,1));
        __LOG_PARAM_WITH_NAME("6-bytes-Tag : lBit from :",
            SMEM_U32_GET_FIELD(descrPtr->sstId,tagSrcId_L_Bit,1));
    }

    if(!buildMcTag || lBit == 1)
    {
        pushedVidFromSrcEport =
            haExtendedTagGetPushVidOfSrcEport(devObjPtr,descrPtr);
        __LOG_PARAM_WITH_NAME("6-bytes-Tag : pushedVidFromSrcEport for :",
            (!buildMcTag || lBit == 1));
    }
    else
    {
        pushedVidFromSrcEport = 0;
    }

    __LOG_PARAM(pushedVidFromSrcEport);

    if(!buildMcTag)
    {
        pushedVidFromTrgEport =
            SMEM_LION3_HA_EPORT_2_ENTRY_FIELD_AUTO_GET(devObjPtr,
                descrPtr,
                SMEM_LION3_HA_EPORT_TABLE_2_PUSHED_TAG_VALUE);
        eCidBaseAndGrp = 0;
    }
    else
    {
        pushedVidFromTrgEport = 0;
        eCidBaseAndGrp =
            haExtendedTagGetMcGroup(devObjPtr,descrPtr);
    }

    __LOG_PARAM(pushedVidFromTrgEport);
    __LOG_PARAM(eCidBaseAndGrp);

    /* always one of those is 0 */
    regValue = pushedVidFromTrgEport + eCidBaseAndGrp;

    /* build the 6 bytes */
    haInfoPtr->evbBpeTagPtr[0] = (GT_U8)(etherType >> 8);
    haInfoPtr->evbBpeTagPtr[1] = (GT_U8)(etherType >> 0);
    haInfoPtr->evbBpeTagPtr[2] = (GT_U8)((dBit<<7) | (pBit << 6) | (regValue >> 8));
    haInfoPtr->evbBpeTagPtr[3] = (GT_U8)(regValue >> 0);
    haInfoPtr->evbBpeTagPtr[4] = (GT_U8)((lBit<<7) | (rBit << 6) | (ver << 4) | (pushedVidFromSrcEport >> 8));
    haInfoPtr->evbBpeTagPtr[5] = (GT_U8)(pushedVidFromSrcEport >> 0);

    haInfoPtr->evbBpeTagLength = 6;
    __LOG(("6-bytes-Tag 6 BYTES:"));
    simLogDump(devObjPtr, SIM_LOG_INFO_TYPE_PACKET_E,
               (GT_PTR)haInfoPtr->evbBpeTagPtr, haInfoPtr->evbBpeTagLength);

}

/**
* @internal snetChtHaEgressEPortEvbBpeTag function
* @endinternal
*
* @brief   HA - check if need to build EVB extra tag
*         NOTE: The tag is pushed as the outer tag of the packet after all other
*         tag processing and manipulation is completed
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - Cht frame descriptor
* @param[in] localEgressPort          - physical egress port -- local port number (port 0..15) !!!
*                                      (referred as <LocalDevTrgPhyPort> in documentation ,
*                                      but it is not part of the 'descriptor' !)
* @param[in,out] haInfoPtr                - pointer to EVB/BPE tags
* @param[in,out] haInfoPtr                - pointer to EVB/BPE tags , filled with info
* @param[in,out] haInfoPtr                - EVB/BPE tags length (0/4/8/6)
* @param[in,out] haInfoPtr                - is EVB on passenger in case of TS.
*                                      0 - the EVB/BPE on TS header (in case of TS)
*                                      1 - the EVB/BPE on Passenger (in case of TS)
*                                      if non TS --> both behave the same.
*/
static GT_VOID snetChtHaEgressEPortEvbBpeTag
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32    localEgressPort,
    INOUT HA_INTERNAL_INFO_STC *haInfoPtr
)
{
    DECLARE_FUNC_NAME(snetChtHaEgressEPortEvbBpeTag);

    GT_U32   tpId;/* TP-id */
    GT_U16   vlanId;
    GT_U32   upAndCfiAssignmentCommand;
    GT_U32   etherType;/* ethertype to use */
    GT_U32   pushVlanCommand;/* push vlan command */
    GT_U8    up,cfi;
    GT_U32  tagExtendedSize; /* size of tag extended, NULL means not used (sip5 only)*/
    GT_U32  ingressECidBase;/* BPE : 12 bits of <ingress_E-CID_base> */
    GT_U32  eCidBaseAndGrp; /* BPE : 12 bits <E-CID_base> + 2 bits <GRP> */
    GT_U32  BPEConfigReg1Val ,BPEConfigReg2Val;/* value of registers : BPE Config Reg 1,2 */
    GT_U32  BPEeTagReserved0,BPEeTagReserved1;
    GT_BIT  useETagPidToDefault;
    GT_BIT  usePhysicalPortPushTagVid = 0; /* status of use of physical port pushTag VLAN Id value */
    GT_BIT  remotePhyPortMapEnable;
    GT_BIT  buildMcTag;

    haInfoPtr->evbBpeTagLength = 0;
    haInfoPtr->evbBpeIsPassenger = 0;
    /* get EPort info */

    /*Push VLAN Command (EVB support) */
    pushVlanCommand =
        SMEM_LION3_HA_EPORT_2_ENTRY_FIELD_AUTO_GET(devObjPtr,
            descrPtr,
            SMEM_LION3_HA_EPORT_TABLE_2_PUSH_VLAN_COMMAND);

    /*sip5 :
        0=disabled:none:No tag is pushed for this packet
        1=enabled:Push_Tag:Push a tag. access the TPID table to extract the tag type (4B VLAN or 8B E-Tag)
    */

    /* sip5_10
        0=disabled:none:No tag is pushed for this packet
        1=Push_Tag:Push_Tag:Push a Push single tag on the Tunnel Start layer 2 header. access the TPID table to extract the tag type (4B VLAN or 8B E-Tag)
        2=Push_Tag_TS:Push_Tag_TS:Push single VLAN tag on passenger packet layer 2 header, prior to TS encapsulation
        3=Reserved:Reserved:
    */
    if(pushVlanCommand == 0)
    {
        __LOG(("EVB/BPE support: Disabled \n"));
        return;
    }

    __LOG(("Push VLAN Command [%d](EVB/BPE support) \n",pushVlanCommand));

    if(pushVlanCommand == 1 || (0 == SMEM_CHT_IS_SIP5_10_GET(devObjPtr)))
    {
        /* sip 5 supports only value 1 . sip 5_10 supports value 1,2 */
        haInfoPtr->evbBpeIsPassenger = 0;
        __LOG(("Push a single tag on the Tunnel Start layer 2 header. access the TPID table to extract the tag type (4B VLAN or 8B E-Tag) \n"));
    }
    else
    {
        haInfoPtr->evbBpeIsPassenger = 1;
        __LOG(("Push single VLAN tag on passenger packet layer 2 header, prior to TS encapsulation \n"));
    }

    tpId =
        SMEM_LION3_HA_EPORT_2_ENTRY_FIELD_AUTO_GET(devObjPtr,
            descrPtr,
            SMEM_LION3_HA_EPORT_TABLE_2_PUSHED_TAG_TPID_SELECT);

    /* Get the status of use physical port push tag VLAN Id value */
    if(SMEM_CHT_IS_SIP6_10_GET(devObjPtr))
    {
        usePhysicalPortPushTagVid = SMEM_LION3_HA_PHYSICAL_PORT_2_ENTRY_FIELD_AUTO_GET(devObjPtr,
                                    descrPtr,
                                    SMEM_SIP6_10_HA_PHYSICAL_PORT_TABLE_2_FIELDS_USE_PHYSICAL_PORT_PUSH_TAG_VID_EN_E);
        __LOG(("EVB/BPE: use VLAN Id from physical port entry enable state [%d]\n", usePhysicalPortPushTagVid));
    }

    /* Set the push tag VLAN Id value */
    if(usePhysicalPortPushTagVid == 1)
    {
        /* Set the push tag VLAN Id value based on target physical port */
        vlanId = SMEM_LION3_HA_PHYSICAL_PORT_2_ENTRY_FIELD_AUTO_GET(
                            devObjPtr,
                            descrPtr,
                            SMEM_SIP6_10_HA_PHYSICAL_PORT_TABLE_2_FIELDS_PUSHED_TAG_VLAN_ID_E);
        __LOG(("EVB/BPE : use VLAN Id[%d] from physical port entry \n", vlanId));
    }
    else
    {
        /* Set the push tag VLAN Id value based on target ePort */
        vlanId = SMEM_LION3_HA_EPORT_2_ENTRY_FIELD_AUTO_GET(devObjPtr,
                                                            descrPtr,
                                                            SMEM_LION3_HA_EPORT_TABLE_2_PUSHED_TAG_VALUE);
        __LOG(("EVB/BPE : use VLAN Id[%d] from ePort entry \n", vlanId));
    }
    /*UP and CFI assignment command*/
    upAndCfiAssignmentCommand =
        SMEM_LION3_HA_EPORT_2_ENTRY_FIELD_AUTO_GET(devObjPtr,
            descrPtr,
            SMEM_LION3_HA_EPORT_TABLE_2_UP_CFI_ASSIGNMENT_COMMAND);

    if(upAndCfiAssignmentCommand)
    {
        /* value from the EPort */
        up =
            SMEM_LION3_HA_EPORT_2_ENTRY_FIELD_AUTO_GET(devObjPtr,
                descrPtr,
                SMEM_LION3_HA_EPORT_TABLE_2_UP);
        cfi =
            SMEM_LION3_HA_EPORT_2_ENTRY_FIELD_AUTO_GET(devObjPtr,
                descrPtr,
                SMEM_LION3_HA_EPORT_TABLE_2_CFI);

        __LOG(("EVB/BPE : use up[%d],cfi[%d] from Eport entry \n",
            up,cfi));
    }
    else
    {
        /* values from the descriptor */
        up = descrPtr->up;
        cfi = descrPtr->cfidei;

        __LOG(("EVB/BPE : use up[%d],cfi[%d] from descriptor \n",
            up,cfi));
    }

    /* get ethertype according to the TPID */
    snetXCatHaEgressTagEtherTypeByTpid(devObjPtr,descrPtr,tpId,&etherType,&tagExtendedSize,GT_FALSE/*not tag1*/);
    __LOG_PARAM(tagExtendedSize);
    if(tagExtendedSize == 4)
    {
        /* push S-Tag */
        __LOG(("Build S-Tag (4 bytes) \n"));
        snetChtHaEgressTagDataExtGet(up, vlanId,cfi, (GT_U16)etherType, &haInfoPtr->evbBpeTagPtr[0]);

        haInfoPtr->evbBpeTagLength = 4;

        __LOG(("EVB :"));
        __LOG(("S-Tag BYTES:"));
        simLogDump(devObjPtr, SIM_LOG_INFO_TYPE_PACKET_E,
                   (GT_PTR)haInfoPtr->evbBpeTagPtr, haInfoPtr->evbBpeTagLength);
    }
    else if (tagExtendedSize == 6)  /* value valid only on sip5_15 */
    {
        snetChtHaEgressBuild6BytesTag(devObjPtr,descrPtr,localEgressPort,haInfoPtr,etherType);
    }
    else /*if (tagExtendedSize == 8)*/
    {
        smemRegGet(devObjPtr, SMEM_LION3_HA_BPE_CONFIG_1_REG(devObjPtr), &BPEConfigReg1Val);
        smemRegGet(devObjPtr, SMEM_LION3_HA_BPE_CONFIG_2_REG(devObjPtr), &BPEConfigReg2Val);

        BPEeTagReserved0 = SMEM_U32_GET_FIELD(BPEConfigReg2Val, 0, 16);
        BPEeTagReserved1 = SMEM_U32_GET_FIELD(BPEConfigReg2Val, 16, 2);

        remotePhyPortMapEnable =
            SMEM_LION3_HA_PHYSICAL_PORT_2_ENTRY_FIELD_AUTO_GET(devObjPtr,
                                                           descrPtr,
                                                           SMEM_LION3_HA_PHYSICAL_PORT_TABLE_2_FIELDS_REMOTE_PHYSICAL_PORT_MAP_ENABLE);

        if(descrPtr->useVidx == 0)
        {
            __LOG(("Build UC E-Tag (8 bytes) due to useVidx = 0 \n"));
            buildMcTag = 0;
        }
        else
        if(SMEM_CHT_IS_SIP6_GET(devObjPtr) &&
           remotePhyPortMapEnable)
        {
            /* fix JARA : HA-3657 : A MC E-Tag/VN-Tag instead of UC E-Tag/VN-Tag is pushed to replicated packets to remote physical ports*/
            __LOG(("Sip6 : Build UC E-Tag (8 bytes) even though useVidx = 1 , because Target Phy Port<Remote Physical Port Map Enable> != 0 \n"));
            buildMcTag = 0;
        }
        else
        {
            __LOG(("Build MC E-Tag (8 bytes) due to useVidx = 1 \n"));
            buildMcTag = 1;
        }

        /* push E-Tag */
        if(buildMcTag)
        {
            __LOG(("E-Tag : Build E-Tag (8 bytes) for multi destination \n"));

            if(GT_TRUE ==
                haExtendedTagIsReturnToSrcInterface(devObjPtr,descrPtr,localEgressPort))
            {
                useETagPidToDefault = 0;
            }
            else
            {
                useETagPidToDefault = 1;
            }

            if(useETagPidToDefault)
            {
                /*<BPE Ingress EPID default>*/
                ingressECidBase = SMEM_U32_GET_FIELD(BPEConfigReg1Val, 0, 12);
                __LOG_PARAM_WITH_NAME("<BPE Ingress EPID default> (from register) ",ingressECidBase);
            }
            else
            {
                ingressECidBase =
                    haExtendedTagGetPushVidOfSrcEport(devObjPtr,descrPtr);
            }

            eCidBaseAndGrp =
                haExtendedTagGetMcGroup(devObjPtr,descrPtr);

        }
        else
        {
            __LOG(("E-Tag : Build E-Tag (8 bytes) for single destination \n"));
            /*<BPE Ingress EPID default>*/
            ingressECidBase = SMEM_U32_GET_FIELD(BPEConfigReg1Val, 0, 12);
            __LOG_PARAM_WITH_NAME("<BPE Ingress EPID default> (from register) ",ingressECidBase);

            /* the CID of the egress port , GRP = 0 */
            eCidBaseAndGrp = vlanId;

            __LOG_PARAM_WITH_NAME("eCidBaseAndGrp",vlanId);
        }


        __LOG_PARAM_WITH_NAME("E-Tag : <E-TPID> 16 bits ",etherType);
        __LOG_PARAM_WITH_NAME("E-Tag : <E-PCP> 3 bits ",up);
        __LOG_PARAM_WITH_NAME("E-Tag : <E-DEI> 3 bits ",cfi);
        __LOG_PARAM_WITH_NAME("E-Tag : <ingress_E-CID_base> 12 bits ",ingressECidBase);

        __LOG_PARAM_WITH_NAME("E-Tag : <E-CID_base> 12 bits ",(eCidBaseAndGrp & 0xFFF));
        __LOG_PARAM_WITH_NAME("E-Tag : <GRP> 2 bits ",(eCidBaseAndGrp >> 12));
        __LOG_PARAM_WITH_NAME("E-Tag : <Re-> 2 reserved bits ",BPEeTagReserved1);
        __LOG_PARAM_WITH_NAME("E-Tag : <ingress_E-CID_ext> and <E-CID_ext> 16 reserved bits ",BPEeTagReserved0);

        /* the Ingress_E-CID_base field is build from BPE_IE-CID_Default */
        snetChtHaEgressTagDataExtGet(up, ingressECidBase,cfi, (GT_U16)etherType, &haInfoPtr->evbBpeTagPtr[0]);

        /*set 14 bits of 12 bits <E-CID_base> + 2 bits <GRP> + 2 bits <re>*/
        haInfoPtr->evbBpeTagPtr[4] = (GT_U8)((eCidBaseAndGrp >> 8) | BPEeTagReserved1 << 6);
        haInfoPtr->evbBpeTagPtr[5] = (GT_U8)(eCidBaseAndGrp >> 0);
        haInfoPtr->evbBpeTagPtr[6] = (GT_U8)(BPEeTagReserved0 >> 8);
        haInfoPtr->evbBpeTagPtr[7] = (GT_U8)(BPEeTagReserved0 >> 0);

        haInfoPtr->evbBpeTagLength = 8;

        __LOG(("BPE :"));
        __LOG(("E-Tag BYTES:"));
        simLogDump(devObjPtr, SIM_LOG_INFO_TYPE_PACKET_E,
                   (GT_PTR)haInfoPtr->evbBpeTagPtr, haInfoPtr->evbBpeTagLength);
    }
}

/**
* @internal snetChtHaRouteMacSaModeGet function
* @endinternal
*
* @brief   HA - get the route MAC SA mode
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in] egressPort               - egress port
* @param[in] usedForTunnelStart       - indication if used by tunnel start or by the routed packet.
*                                      GT_TRUE - used by the TunnelStart
*                                      GT_FALSE - used by the routed packet
*
* @param[out] routeMacSaModePtr        - pointer to the route MAC SA mode
*                                      COMMENTS:
*/
void snetChtHaRouteMacSaModeGet
(
    IN  SKERNEL_DEVICE_OBJECT           *devObjPtr,
    IN  SKERNEL_FRAME_CHEETAH_DESCR_STC *descrPtr,
    IN  GT_U32                          egressPort,
    IN GT_BOOL                          usedForTunnelStart,
    OUT GT_U32                          *routeMacSaModePtr
)
{
    DECLARE_FUNC_NAME(snetChtHaRouteMacSaModeGet);

    GT_U32  *regPtr;
    GT_U32  tmpPort;

    if (devObjPtr->supportMacSaAssignModePerPort == GT_FALSE)
    {
        /* Router Header Alteration Global Configuration Register */
        regPtr = smemMemGet(devObjPtr, SMEM_CHT3_ROUTE_HA_GLB_CNF_REG(devObjPtr));

        /* The 8 LSBs Router MAC SA */
        *routeMacSaModePtr = SMEM_U32_GET_FIELD(regPtr[0], 0, 2);
    }
    else /* (devObjPtr->supportMacSaAssignModePerPort == GT_TRUE) */
    {
        if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
        {
            if(usedForTunnelStart == GT_TRUE)
            {
                __LOG(("The MAC SA for the Tunnel Start header use only mode 3 (take ALL 48 bits from global table) \n"));
                /* used for the MAC SA of the Tunnel Start header */
                *routeMacSaModePtr = 3;
            }
            else
            {
                /*Router MAC SA assignment mode*/
                *routeMacSaModePtr =
                    SMEM_LION3_HA_PHYSICAL_PORT_1_ENTRY_FIELD_AUTO_GET(devObjPtr,
                        descrPtr,
                        SMEM_LION3_HA_PHYSICAL_PORT_TABLE_1_FIELDS_ROUTER_MAC_SA_ASSIGNMENT_MODE);
            }
        }
        else
        {
            tmpPort = egressPort;
            if(devObjPtr->supportHa64Ports == 0)
            {
                tmpPort &= 0x1f;
            }

            /* Router Header Alteration MAC SA Modification Mode */
            regPtr = smemMemGet(devObjPtr, SMEM_CHT_MAC_SA_MODIFICATION_MODE_REG(devObjPtr));

            /* The 8 LSBs Router MAC SA */
            *routeMacSaModePtr = SMEM_U32_GET_FIELD(regPtr[tmpPort/16], ((tmpPort%16)*2), 2);
        }
    }
}

/**
* @internal snetChtHaMacFromMeBuild function
* @endinternal
*
* @brief   HA - get the SRC mac address to use as "mac from me"
*
* @param[out] macAddrPtr               - pointer to the mac address (6 bytes)
*                                      COMMENTS:
*/
void snetChtHaMacFromMeBuild
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32   egressPort,
    IN GT_BOOL  usedForTunnelStart,
    OUT GT_U8   *macAddrPtr
)
{
    DECLARE_FUNC_NAME(snetChtHaMacFromMeBuild);

    GT_U32 * regPtr;                /* pointer to ARP table entry */
    GT_U32 routMacSaMode;           /* Router MAC SA Mode */
    GT_U32  index;
    GT_U32  egressVid = 0;
    GT_U32  setEgressVid = 0;
    GT_U32  macLsBits;


    regPtr = smemMemGet(devObjPtr, SMEM_CHT_MAC_SA_BASE0_REG(devObjPtr));
    /* Copy The 32 LSBs of the Router MAC SA Base address */
    macAddrPtr[4] = (GT_U8)SMEM_U32_GET_FIELD(regPtr[0], 0, 8);
    macAddrPtr[3] = (GT_U8)SMEM_U32_GET_FIELD(regPtr[0], 8, 8);
    macAddrPtr[2] = (GT_U8)SMEM_U32_GET_FIELD(regPtr[0],16, 8);
    macAddrPtr[1] = (GT_U8)SMEM_U32_GET_FIELD(regPtr[0],24, 8);

    /* The 8 MSBs of the Router MAC SA Base address */
    macAddrPtr[0] = (GT_U8)SMEM_U32_GET_FIELD(regPtr[1], 0, 8);

    /* get the TS/Router mac SA mode */
    snetChtHaRouteMacSaModeGet(devObjPtr,descrPtr,egressPort,usedForTunnelStart,&routMacSaMode);

    switch (routMacSaMode)
    {
        case 0:
            if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
            {/*NOTE: in sip5 TS will not get here !!! (only to 'case 3')*/
                if(descrPtr->passangerTr101EgressVlanTagMode == SKERNEL_FRAME_TR101_VLAN_EGR_TAG_TAG1_E ||
                   descrPtr->passangerTr101EgressVlanTagMode == SKERNEL_FRAME_TR101_VLAN_EGR_TAG_OUT_TAG1_IN_TAG0_E)
                {
                    __LOG(("egressVid : taken from vid1 (not evlan)\n"));
                    egressVid = descrPtr->vid1;
                    setEgressVid = 1;
                }
            }

            if(setEgressVid == 0)
            {
                __LOG(("egressVid : taken from vid0 \n"));
                egressVid = descrPtr->eVid;    /* NOT taken from the 'egress vlan translation' */
            }

            macAddrPtr[4] &= 0xf0;
            macAddrPtr[4] |= ((egressVid >> 8) & 0xf);
            macAddrPtr[5] = (GT_U8)(egressVid & 0xff);

            __LOG(("Use 12 LSBs [0x%3.3x] Router MAC SA - from egress VID \n",
                egressVid));
            break;
        case 1:
            if(!SMEM_CHT_IS_SIP6_GET(devObjPtr))
            {
                index = descrPtr->eVid & 0xFFF;/* NOT taken from the 'egress vlan translation' */
            }
            else
            {
                /* part of JIRA HA-3597 : Routed Packet MAC SA Modification - Increase scaling of VLAN mode */
                /* supports all eVlans ! */
                index = descrPtr->eVid; /* NOT taken from the 'egress vlan translation' */
            }

            regPtr = smemMemGet(devObjPtr,
                                SMEM_CHT_MAC_SA_TBL_MEM(devObjPtr, index));

            if(!SMEM_CHT_IS_SIP6_GET(devObjPtr))
            {
                macAddrPtr[5] = (GT_U8)SMEM_U32_GET_FIELD(regPtr[0], 0, 8);
                __LOG(("Use  8 LSBs [0x%2.2x] Router MAC SA - from 'per vlan' - used index[%d] \n",
                    macAddrPtr[5],
                    index));
            }
            else
            {
                /* part of JIRA HA-3597 : Routed Packet MAC SA Modification - Increase scaling of VLAN mode */
                macLsBits = SMEM_U32_GET_FIELD(regPtr[0], 0, 12);
                macAddrPtr[5] = (GT_U8)macLsBits;
                macAddrPtr[4] &= 0xF0; /*clear the needed bits */
                macAddrPtr[4] |=(GT_U8)(macLsBits >> 8);/*set the bits*/
                __LOG(("(Sip6)Use  12 LSBs [0x%3.3x] Router MAC SA - from 'per vlan' - used index[%d] \n",
                    macLsBits,
                    index));
            }
            break;
        case 2:
            if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
            {
                __LOG((" -- ERROR -- : obsolete mode in SIP5 \n"));

                macAddrPtr[0] =
                macAddrPtr[1] =
                macAddrPtr[2] =
                macAddrPtr[3] =
                macAddrPtr[4] =
                macAddrPtr[5] = 0;
            }
            else
            {
                index =  egressPort + 4096;

                regPtr = smemMemGet(devObjPtr,
                                    SMEM_CHT_MAC_SA_TBL_MEM(devObjPtr, index));

                macAddrPtr[5] = (GT_U8)SMEM_U32_GET_FIELD(regPtr[0], 0, 8);

                __LOG(("Use  8 LSBs [0x%2.2x] Router MAC SA - from 'per port'  - used index[%d] \n",
                    macAddrPtr[5],
                    index));
            }
            break;

        case 3:
            if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
            {
                /* the full 48-bit MAC SA taken from one of 256 global entries in
                    Global <MAC SA Table>, indexed by the TRGePort <Router MAC SA Index>*/
                index =
                    SMEM_LION3_HA_EPORT_1_ENTRY_FIELD_AUTO_GET(devObjPtr,
                        descrPtr,
                        SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_ROUTER_MAC_SA_INDEX);

                regPtr = smemMemGet(devObjPtr,
                                    SMEM_LION2_HA_GLOBAL_MAC_SA_TBL_MEM(devObjPtr, index));

                macAddrPtr[0]= snetFieldValueGet(regPtr,40,8);
                macAddrPtr[1]= snetFieldValueGet(regPtr,32,8);
                macAddrPtr[2]= snetFieldValueGet(regPtr,24,8);
                macAddrPtr[3]= snetFieldValueGet(regPtr,16,8);
                macAddrPtr[4]= snetFieldValueGet(regPtr, 8,8);
                macAddrPtr[5]= snetFieldValueGet(regPtr, 0,8);

                __LOG(("use 48 bits Router MAC SA - from 'global table' selected by 'per ePort' - used index[%d] \n",
                        index
                    ));

            }
            else
            {
                __LOG((" -- ERROR -- : not supported mode \n"));
                macAddrPtr[5] = 0;
            }

            break;
        default:
            __LOG((" -- ERROR -- : not supported mode \n"));
            macAddrPtr[5] = 0;
            break;
    }

    __LOG(("Final %s MAC SA is [%2.2x:%2.2x:%2.2x:%2.2x:%2.2x:%2.2x] \n",
            ((usedForTunnelStart == GT_FALSE)? "Router's" : "Tunnel Start's") ,
            macAddrPtr[0],
            macAddrPtr[1],
            macAddrPtr[2],
            macAddrPtr[3],
            macAddrPtr[4],
            macAddrPtr[5]
        ));

    return;
}



/**
* @internal snetChtHaCreateAnalyzerVlanTag function
* @endinternal
*
* @brief   HA - Create remote analyzer VLAN tag added to packet forwarded to
*         the Ingress/Egress analyzer
* @param[in] devObjPtr                -  pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
*
* @note Only network ports can be set to add the remote analyzer VLAN tag.
*       Cascading ports must not be set to add the remote analyzer VLAN tag.
*
*/
static void snetChtHaCreateAnalyzerVlanTag
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    OUT GT_U8_PTR analyzerVlanTagPtr,
    OUT GT_U32   * analyzerVlanTagLengthPtr
)
{
    DECLARE_FUNC_NAME(snetChtHaCreateAnalyzerVlanTag);

    GT_U32 regAddr;                     /* register address */
    GT_U32 * regPtr;                    /* register data pointer */
    GT_U32 up;                          /* UP of the VLAN tag  */
    GT_U32 cfidei;                      /* CFI of the VLAN tag */
    GT_U32 vid;                         /* VID of the VLAN tag */
    GT_U32 vlanEtherType;               /* VLAN Ethertype of the VLAN tag */

    *analyzerVlanTagLengthPtr = 0;

    if (descrPtr->analyzerVlanTagAdd == 0)
    {
        return;
    }

    /* Analyzer VLAN Tag Configuration */
    regAddr = (descrPtr->rxSniff) ?
        SMEM_CHT_HA_INGR_ANALYZER_VLAN_TAG_CONF_REG(devObjPtr) :
        SMEM_CHT_HA_EGR_ANALYZER_VLAN_TAG_CONF_REG(devObjPtr);

    regPtr = smemMemGet(devObjPtr, regAddr);

    /* MirrorVID */
    vid = SMEM_U32_GET_FIELD(*regPtr, 0, 12);
    /* MirrorCFI  */
    cfidei = SMEM_U32_GET_FIELD(*regPtr, 12, 1);
    /* MirrorUP  */
    up = SMEM_U32_GET_FIELD(*regPtr, 13, 3);
    /* MirrorVLANEtherType */
    vlanEtherType = SMEM_U32_GET_FIELD(*regPtr, 16, 16);

    /* Build Analyzer VLAN Tag */
    __LOG(("Build Analyzer VLAN Tag"));
    snetChtHaEgressTagDataExtGet((GT_U8)up, (GT_U16)vid,
                      (GT_U8)cfidei, (GT_U16)vlanEtherType, analyzerVlanTagPtr);

    /* Analyzer VLAN Tag length */
    *analyzerVlanTagLengthPtr = 4;
}
/**
* @internal snetLion3HaCreateTimestampTag function
* @endinternal
*
* @brief   Create (if needed) the egress Timestamp Tag.
*
* @param[in] devObjPtr                -  pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
*
* @param[out] timestampTagPtr          - pointer to TST
* @param[out] timestampTagLengthPtr    - pointer to TST length
*/
static GT_VOID snetLion3HaCreateTimestampTag
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    OUT GT_U8_PTR timestampTagPtr,
    OUT GT_U32   * timestampTagLengthPtr
)
{
    DECLARE_FUNC_NAME(snetLion3HaCreateTimestampTag);

    GT_U32 regAddr;                     /* register address */
    GT_U32 *regPtr;                    /* register data pointer */
    GT_U16 tstEtherType;                /* Ethertype of the TST */
    GT_U32 fieldData;                   /* field Data */
    GT_U32 useLocalDeviceTime;          /* use Local Device Ingress Time */
    GT_U64 secondTimer;                 /* second Timer          */
    GT_U32 nanoSecondTimer;             /* nano Second Timer     */
    GT_BIT  tagFieldT;                  /* tagFieldT             */
    GT_BIT  tagFieldU;                  /* tagFieldU             */
    GT_BIT  tagFieldOE;                 /* tagFieldOE            */
    GT_U32  tagFieldOffsetProfile;      /* tagFieldOffsetProfile */

    SKERNEL_TIMESTAMP_TAG_TYPE_ENT egressTst;   /* egress timestamp tag type */
    SKERNEL_TIMESTAMP_TAG_TYPE_ENT ingressTst;   /* ingress timestamp tag type */

    egressTst = descrPtr->timestampTagged[SMAIN_DIRECTION_EGRESS_E];
    ingressTst = descrPtr->timestampTagged[SMAIN_DIRECTION_INGRESS_E];

    *timestampTagLengthPtr = 0;

    __LOG_PARAM(egressTst);
    regAddr = SMEM_LION3_HA_TST_ETHERTYPE_REG(devObjPtr);
    regPtr = smemMemGet(devObjPtr, regAddr);

    if( egressTst == SKERNEL_TIMESTAMP_TAG_TYPE_HYBRID_E )
    {
        tstEtherType = SMEM_U32_GET_FIELD(*regPtr, 0, 16);
    }
    else
    {
        tstEtherType = SMEM_U32_GET_FIELD(*regPtr, 16, 16);
    }

    /* Port Timestamp Reception Enable indication */
    if(SMEM_CHT_IS_SIP7_GET(devObjPtr))
    {
        fieldData = SMEM_SIP7_HA_SRC_PHYSICAL_PORT_2_ENTRY_FIELD_GET(devObjPtr,
            descrPtr->localDevSrcPort,/* the src physical port */
            SMEM_SIP7_HA_SRC_PHYSICAL_PORT_2_TABLE_FIELDS_PORT_TIMESTAMP_RECEPTION_EN_E);
    }
    else
    {
        fieldData = SMEM_LION3_HA_PHYSICAL_PORT_2_ENTRY_FIELD_BY_SRC_PORT_GET(devObjPtr,
            descrPtr->localDevSrcPort,/* the physical port */
            SMEM_LION3_HA_PHYSICAL_PORT_TABLE_2_FIELDS_TIMESTAMP_RECEPTION_EN);
    }

    useLocalDeviceTime = (fieldData == 0);

    if( (egressTst == SKERNEL_TIMESTAMP_TAG_TYPE_UNTAGGED_E) ||
        (egressTst == SKERNEL_TIMESTAMP_TAG_TYPE_PIGGY_BACKED_E) )
    {
        /* if egress port is configured as non TST and packet has TST -  remove it */
        if ( (ingressTst == SKERNEL_TIMESTAMP_TAG_TYPE_NON_EXTENDED_E) ||
             (ingressTst == SKERNEL_TIMESTAMP_TAG_TYPE_EXTENDED_E) ||
             (ingressTst == SKERNEL_TIMESTAMP_TAG_TYPE_HYBRID_E) )
        {

            if (ingressTst == SKERNEL_TIMESTAMP_TAG_TYPE_EXTENDED_E)
            {
                /* packet has Extended Timestamp Tag */
                __LOG(("remove Extended Timestamp Tag \n"));
                return;
            }
            else
            {
                /* packet has Non-Extended/Hybrid Timestamp Tag */
                __LOG(("remove Non-Extended/Hybrid Timestamp Tag \n"));
                return;
            }
        }

        /* if egress port is configured as non TST and packet don't have TST - do nothing */
        __LOG(("No Timestamp Tag needed \n"));
    }
    else
    {
        if ( (ingressTst == SKERNEL_TIMESTAMP_TAG_TYPE_NON_EXTENDED_E) ||
             (ingressTst == SKERNEL_TIMESTAMP_TAG_TYPE_EXTENDED_E) ||
             (ingressTst == SKERNEL_TIMESTAMP_TAG_TYPE_HYBRID_E) )
        {
            /* if egress port is configured as TST and packet already has TST -  preserve incoming (don't modify the value) */
            __LOG(("preserve incoming Timestamp Tag \n"));
            memcpy(timestampTagPtr, descrPtr->ingressTimestampTagPtr, descrPtr->ingressTimestampTagSize);
            *timestampTagLengthPtr = descrPtr->ingressTimestampTagSize;
            return;

        }
        /* if egress port is configured as TST and packet don't have TST -  add new TST with ingress time of current device */
        __LOG(("Create Timestamp Tag \n"));

        /* TST ethertype */
        timestampTagPtr[0] = (tstEtherType & 0xFF00) >> 8;
        timestampTagPtr[1] = (tstEtherType & 0x00FF);

        if(egressTst == SKERNEL_TIMESTAMP_TAG_TYPE_HYBRID_E)
        {
            *timestampTagLengthPtr = 8;
        }
        else if(egressTst == SKERNEL_TIMESTAMP_TAG_TYPE_NON_EXTENDED_E)
        {
            *timestampTagLengthPtr = 8;
        }
        else  /* SKERNEL_TIMESTAMP_TAG_TYPE_EXTENDED_E*/
        {
            *timestampTagLengthPtr = 16;
        }

        if (useLocalDeviceTime)
        {
            /* local device ingress time */
            secondTimer      =
                descrPtr->timestamp[descrPtr->ptpTaiSelect][SMAIN_DIRECTION_INGRESS_E].secondTimer;
            nanoSecondTimer  =
                descrPtr->timestamp[descrPtr->ptpTaiSelect][SMAIN_DIRECTION_INGRESS_E].nanoSecondTimer;
            tagFieldT             = descrPtr->ptpTaiSelect & 1;
            tagFieldU             = 0;
            tagFieldOE            = 0;
            tagFieldOffsetProfile = 0;
        }
        else
        {
            /* ingress time retrieved from ingress timestamp tag */
            secondTimer     =
                descrPtr->timestampTagInfo[SMAIN_DIRECTION_INGRESS_E].
                timestamp.secondTimer;
            nanoSecondTimer =
                descrPtr->timestampTagInfo[SMAIN_DIRECTION_INGRESS_E].
                    timestamp.nanoSecondTimer;
            tagFieldT             =
                descrPtr->timestampTagInfo[SMAIN_DIRECTION_INGRESS_E].T;
            tagFieldU             =
                descrPtr->timestampTagInfo[SMAIN_DIRECTION_INGRESS_E].U;
            tagFieldOE            =
                descrPtr->timestampTagInfo[SMAIN_DIRECTION_INGRESS_E].OE;
            tagFieldOffsetProfile =
                descrPtr->timestampTagInfo[SMAIN_DIRECTION_INGRESS_E].OffsetProfile;
        }

        if(egressTst == SKERNEL_TIMESTAMP_TAG_TYPE_NON_EXTENDED_E)
        {
            /* The non-extended timestamp is represented as a 32-bit signed nanosecond value, that is computed as follows:
               The conversion is performed by taking the Nanosecond field from the ingress timestamp, and adding the lsbit of the Second field.
               Seconds[0] * 10^9 + Nanoseconds */
            nanoSecondTimer += (secondTimer.l[0] & 1)*1000000000;
        }

        if(egressTst == SKERNEL_TIMESTAMP_TAG_TYPE_HYBRID_E)
        {
            timestampTagPtr[2] = (GT_U8)((secondTimer.l[1] >> 8) & 0xFF);
            timestampTagPtr[3] = (GT_U8)(secondTimer.l[1] & 0xFF);
            timestampTagPtr[4] = (GT_U8)((secondTimer.l[0] >> 24) & 0xFF);
            timestampTagPtr[5] = (GT_U8)((secondTimer.l[0] >> 16) & 0xFF);
            timestampTagPtr[6] = (GT_U8)((secondTimer.l[0] >> 8) & 0xFF);
            timestampTagPtr[7] = (GT_U8)(secondTimer.l[0] & 0xFF);
        }
        else /* SKERNEL_TIMESTAMP_TAG_TYPE_NON_EXTENDED_E || SKERNEL_TIMESTAMP_TAG_TYPE_EXTENDED_E*/
        {
           timestampTagPtr[2] =  (GT_U8)((tagFieldT << 1) + tagFieldU);
           timestampTagPtr[3] =  (GT_U8)((tagFieldOE << 7) + tagFieldOffsetProfile);

           timestampTagPtr[4] = (GT_U8)((nanoSecondTimer >> 24) & 0xFF);
           timestampTagPtr[5] = (GT_U8)((nanoSecondTimer >> 16) & 0xFF);
           timestampTagPtr[6] = (GT_U8)((nanoSecondTimer >> 8) & 0xFF);
           timestampTagPtr[7] = (GT_U8)(nanoSecondTimer & 0xFF);

           if(egressTst == SKERNEL_TIMESTAMP_TAG_TYPE_EXTENDED_E)
           {
               *timestampTagLengthPtr = 16;

               /* TST E bit setting */
               timestampTagPtr[2] += (1<<7);

               timestampTagPtr[8]  = (GT_U8)((secondTimer.l[1] >> 8) & 0xFF);
               timestampTagPtr[9]  = (GT_U8)(secondTimer.l[1] & 0xFF);
               timestampTagPtr[10] = (GT_U8)((secondTimer.l[0] >> 24) & 0xFF);
               timestampTagPtr[11] = (GT_U8)((secondTimer.l[0] >> 16) & 0xFF);
               timestampTagPtr[12] = (GT_U8)((secondTimer.l[0] >> 8) & 0xFF);
               timestampTagPtr[13] = (GT_U8)(secondTimer.l[0] & 0xFF);
           }
        }
    }
}

/**
* @internal snetChtHaIsMtagPacketChanged function
* @endinternal
*
* @brief   HA - Check if the DSA tagged packet is changed by the ingress pipeline
*
* @param[in] descrPtr                 - pointer to the frame's descriptor.
*
* @retval GT_TRUE                  - if packet was changed
*/
static GT_BOOL snetChtHaIsMtagPacketChanged
(
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC  *descrPtr
)
{
    /* when building DSA tag any field may be changed from ingress */
    return GT_TRUE;
}

/**
* @internal snetChtHaIsPacketChanged function
* @endinternal
*
* @brief   HA - Check if the packet is changed by the ingress pipeline
*
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in] destVlanTagged           - support tr101 tagging modes
* @param[in] vlanEtherType            - egress VLAN EtherType for Tag0
* @param[in] vlanEtherType1           - egress VLAN EtherType for Tag1
*
* @retval GT_TRUE                  - if packet is changed
*/
static GT_BOOL snetChtHaIsPacketChanged
(
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U8 destVlanTagged,
    IN GT_U32 vlanEtherType,
    IN GT_U32 vlanEtherType1
)
{
    /* check if the packet is received with marvell(extended) tag */
    if (descrPtr->marvellTagged)
    {
        return GT_TRUE;
    }

    if(descrPtr->prpInfo.prpCmd != PRP_CMD_DO_NOTHING_E)
    {
        return GT_TRUE;
    }

    if (descrPtr->truncated && descrPtr->byteCount> 128)
    {
        /* the packet came longer than it can go out */
        return GT_TRUE;
    }

    if (descrPtr->outGoingMtagCmd == SKERNEL_MTAG_CMD_TO_TRG_SNIFFER_E)
    {
        if (descrPtr->analyzerKeepVlanTag == 0)
            return GT_TRUE;

        if (descrPtr->rxSniff && descrPtr->analyzerVlanTagAdd == 0 &&
            descrPtr->tunnelStart == 0/* support ERSPAN */)
            return GT_FALSE;
        else
            return GT_TRUE;
    }

    if(descrPtr->basicMode == 1)
    {
        if(descrPtr->marvellTagged == 0)
        {
            /* in basic mode the packet must egress "as is" unmodified */
            return GT_FALSE;
        }
    }

    if(descrPtr->numOfBytesToPop)
    {
        return GT_TRUE;
    }

    if(descrPtr->tunnelTerminated)
    {
        /* packet terminated need to 'rebuild it'*/
        return GT_TRUE;
    }
    else if(descrPtr->tunnelStart)
    {
        /* tunnel start - need to rebuild packet */
        return GT_TRUE;
    }
    else if(descrPtr->useArpForMacSa)
    {
        /* change the mac SA - need to rebuild packet */
        return GT_TRUE;
    }

    if ( (descrPtr->modifyUp) ||
         (descrPtr->modifyDscp) ||
         (descrPtr->vidModified) )
    {
        return GT_TRUE;
    }

    if ( ((SKERNEL_FRAME_TR101_VLAN_EGR_TAG_TAG1_E == destVlanTagged) ||
          (SKERNEL_FRAME_TR101_VLAN_EGR_TAG_OUT_TAG0_IN_TAG1_E == destVlanTagged) ||
          (SKERNEL_FRAME_TR101_VLAN_EGR_TAG_OUT_TAG1_IN_TAG0_E == destVlanTagged)) &&
         (descrPtr->originalVid1 != descrPtr->vid1) )
    {
        /* tag1 VID exists in egress packet and changed from ingress value */
        return GT_TRUE;
    }

    if(descrPtr->tr101ModeEn)
    {
        if ( descrPtr->srcTagState != destVlanTagged )
        {
            return GT_TRUE;
        }

        if(descrPtr->srcTagState)
        {
            if (descrPtr->vlanEtherType != vlanEtherType ||
                descrPtr->vlanEtherType1 != vlanEtherType1)
            {
                return GT_TRUE;
            }
        }
    }
    else
    {
        if ( descrPtr->tagSrcTagged[SNET_CHT_TAG_0_INDEX_CNS] != destVlanTagged )
        {
            return GT_TRUE;
        }

        if(descrPtr->tagSrcTagged[SNET_CHT_TAG_0_INDEX_CNS])
        {
            if (descrPtr->vlanEtherType != vlanEtherType)
            {
                return GT_TRUE;
            }
        }
    }

    /* Route unicast packet */
    if (descrPtr->doRouterHa || descrPtr->routed)
    {
        return GT_TRUE;
    }

    /* l2  VntL2Echo*/
    if (descrPtr->VntL2Echo)
    {
        return GT_TRUE;
    }

    if(descrPtr->eArchExtInfo.haInfo.ePortModifyMacSa ||
       descrPtr->eArchExtInfo.haInfo.ePortModifyMacDa)
    {
        return GT_TRUE;
    }

    if( (descrPtr->timestampTagged[SMAIN_DIRECTION_EGRESS_E] == SKERNEL_TIMESTAMP_TAG_TYPE_UNTAGGED_E) ||
        (descrPtr->timestampTagged[SMAIN_DIRECTION_EGRESS_E] == SKERNEL_TIMESTAMP_TAG_TYPE_PIGGY_BACKED_E) )
     {

        if ( (descrPtr->timestampTagged[SMAIN_DIRECTION_INGRESS_E] == SKERNEL_TIMESTAMP_TAG_TYPE_NON_EXTENDED_E) ||
             (descrPtr->timestampTagged[SMAIN_DIRECTION_INGRESS_E] == SKERNEL_TIMESTAMP_TAG_TYPE_EXTENDED_E) ||
             (descrPtr->timestampTagged[SMAIN_DIRECTION_INGRESS_E] == SKERNEL_TIMESTAMP_TAG_TYPE_HYBRID_E) )

            /* if egress port is configured as non TST and ingress port is configured as TST - need to remove it */
            return GT_TRUE;
    }
    else
    {
        if ( (descrPtr->timestampTagged[SMAIN_DIRECTION_INGRESS_E] == SKERNEL_TIMESTAMP_TAG_TYPE_UNTAGGED_E) ||
             (descrPtr->timestampTagged[SMAIN_DIRECTION_INGRESS_E] == SKERNEL_TIMESTAMP_TAG_TYPE_PIGGY_BACKED_E) )

            /* if egress port is configured as TST and ingress port is configured as non TST -  add new TST with ingress time of current device */
            return GT_TRUE;
    }

    if(descrPtr->isNat)
    {
        /*Packet considered changed due to NAT*/
        return GT_TRUE;
    }

    if(descrPtr->nestedVlanAccessPort)
    {
        return GT_TRUE;
    }

    return GT_FALSE;
}

/**
* @internal snetLion3HaDSATrgInterfaceRemoteFieldsReAssignment function
* @endinternal
*
* @brief   The 4B/8B/16B DSA tag target interface fields assignment
*         - remote physical port support
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] descrPtr                 - pointer to the frame's descriptor.
* @param[in] egressPort               - egress port (local port on multi-port group device)
*                                      CPU port is port 63
* @param[in,out] descrPtr                 - pointer to the frame's descriptor.
*
* @param[out] dsaFieldsPtr             pointer to dsa targed field values
*
* @retval GT_BOOL                  - values were remapped (GT_TRUE), or not (GT_FALSE)
*
* @note If (<Remote Physical Port Map Enable>==1) AND (Egress DSA tag is 16B eDSA)
*       (
*       DSA<Trg Phy Port> = <Remote Physical Port Number>
*       DSA<Trg Dev> = <Remote Physical Port Device ID> DSA<Trg ePort> = <Remote Physical Port Device ID>
*       DSA<Is Trg Phy Port Valid> = 1;
*       DSA<Use VIDX> = 0;
*       )
*       Else If (<Remote Physical Port Map Enable>==1) AND (Egress DSA tag is 8B DSA)
*       (
*       DSA<Trg Port> = <Remote Physical Port Number>
*       DSA<Trg Dev> = <Remote Physical Port Device ID>
*       DSA<Use VIDX> = 0
*       )
*       Else If sip5_15 AND (<Remote Physical Port Map Enable>==1) AND (Egress DSA tag is 4B DSA)
*       (
*       DSA<Trg Phy Port> = <Remote Physical Port Number>
*       DSA<Trg Dev> = <Remote Physical Port Device ID>
*       If (Descr<outGoingMtagCmd> == FORWARD) AND (<Transmit FORWARD packets with a forced 4B FROM_CPU DSA tag>=1>)
*       Descr<outGoingMtagCmd> = FROM_CPU DSA
*       If (Descr<outGoingMtagCmd> == TO_ANALYZER) AND (<Transmit TO_ANALYZER packets with a forced 4B FROM_CPU DSA tag>=1>)
*       Descr<outGoingMtagCmd> = FROM_CPU DSA
*       If (Descr<outGoingMtagCmd> == FROM_CPU) AND (<Transmit FROM_CPU packets with a forced 4B FROM_CPU DSA tag>=1>)
*       Descr<outGoingMtagCmd> = FROM_CPU DSA
*       )
*       Else // <Remote Physical Port Map Enable> == 0
*       No remapping in egress DSA target interface fields
*
*/
static GT_BOOL  snetLion3HaDSATrgInterfaceRemoteFieldsReAssignment
(
    IN    SKERNEL_DEVICE_OBJECT           *devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC *descrPtr,
    IN    GT_U32                           egressPort,
    OUT   SNET_DSA_TAG_FIELDS_STC         *dsaFieldsPtr
)
{
    DECLARE_FUNC_NAME(snetLion3HaDSATrgInterfaceRemoteFieldsReAssignment);

    GT_BIT remotePhyPortMapEnable;
    GT_U32 remotePhyPortNumber, remotePhyDeviceId;

    if(descrPtr->egrMarvellTagType != MTAG_4_WORDS_E  &&
       descrPtr->egrMarvellTagType != MTAG_EXTENDED_E &&
       SMEM_CHT_IS_SIP5_15_GET(devObjPtr) == 0)
    {
        __LOG(("Remote Physical Port Remapping relevant only for extDSA and eDSA - nothing to do \n"));
        return GT_FALSE;
    }

    remotePhyPortMapEnable =
                SMEM_LION3_HA_PHYSICAL_PORT_2_ENTRY_FIELD_AUTO_GET(devObjPtr,
                    descrPtr,
                    SMEM_LION3_HA_PHYSICAL_PORT_TABLE_2_FIELDS_REMOTE_PHYSICAL_PORT_MAP_ENABLE);

    if(0 == remotePhyPortMapEnable)
    {
        __LOG(("Remote Physical Port Map Disabled - nothing to do \n"));
        return GT_FALSE;
    }

    remotePhyPortNumber =
                SMEM_LION3_HA_PHYSICAL_PORT_2_ENTRY_FIELD_AUTO_GET(devObjPtr,
                    descrPtr,
                    SMEM_LION3_HA_PHYSICAL_PORT_TABLE_2_FIELDS_REMOTE_PHYSICAL_PORT_NUMBER);

    remotePhyDeviceId =
                SMEM_LION3_HA_PHYSICAL_PORT_2_ENTRY_FIELD_AUTO_GET(devObjPtr,
                    descrPtr,
                    SMEM_LION3_HA_PHYSICAL_PORT_TABLE_2_FIELDS_REMOTE_PHYSICAL_PORT_DEVICE_ID);


    __LOG_PARAM(remotePhyPortMapEnable);
    __LOG_PARAM(remotePhyPortNumber);
    __LOG_PARAM(remotePhyDeviceId);


    dsaFieldsPtr->trgDev            = remotePhyDeviceId;
    dsaFieldsPtr->useVidx           = 0;

    /* for 'extended' dsa tag the 'phy' fields are not used anyway */
    dsaFieldsPtr->isTrgPhyPortValid = 1;
    dsaFieldsPtr->trgPhyPort        = remotePhyPortNumber;
    dsaFieldsPtr->trgEPort          = remotePhyPortNumber;

    if (descrPtr->forceToAddFromCpu4BytesDsaTag)
    {
        __LOG(("SIP5.15: Remote Physical Port Map Enabled and 4 Bytes FROM_CPU DSA is added to the packet, "
               "rather than converting the outer VLAN tag to a DSA tag - remap descriptor fields \n"));
    }
    else
    {
        __LOG(("Remote Physical Port Map Enabled and DSA tag is 8/16 bytes - remap descriptor fields \n"));
    }

    return GT_TRUE;
}


/**
* @internal convertDsaTagWordsToPacketBytes function
* @endinternal
*
* @brief   convert DSA tag format in little endian (in words) and convert to stream
*         of bytes in network order
* @param[in] devObjPtr                -  pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in] dsaType                  - dsa tag type
* @param[in] mtag[/4/]                - 4 words for the build of the DSA tag
*
* @param[out] mrvlTagPtr               - pointer to DSA tag
*/
static void convertDsaTagWordsToPacketBytes
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC *descrPtr,
    IN DSA_TAG_TYPE_E   dsaType,
    IN GT_U32           mtag[/*4*/],
    OUT GT_U8_PTR       mrvlTagPtr
)
{
    DECLARE_FUNC_NAME(convertDsaTagWordsToPacketBytes);

    GT_U32 ii;/* bytes iterator */
    GT_U32 numOfWords;/*number of words in the DSA tag*/
    GT_U32 index;/*words iterator*/

    switch(dsaType)
    {
        case MTAG_STANDARD_E:
            numOfWords = 1;
            break;
        case MTAG_EXTENDED_E:
            numOfWords = 2;
            break;
        case MTAG_3_WORDS_E:
            numOfWords = 3;
            break;
        case MTAG_4_WORDS_E:
            numOfWords = 4;
            break;
        default:
            numOfWords = 0;
            break;
    }


    for(ii = 0 , index = 0 ; index < numOfWords ; index++)
    {
        mrvlTagPtr[ii++] = (GT_U8)SMEM_U32_GET_FIELD(mtag[index],24,8);
        mrvlTagPtr[ii++] = (GT_U8)SMEM_U32_GET_FIELD(mtag[index],16,8);
        mrvlTagPtr[ii++] = (GT_U8)SMEM_U32_GET_FIELD(mtag[index], 8,8);
        mrvlTagPtr[ii++] = (GT_U8)SMEM_U32_GET_FIELD(mtag[index], 0,8);


        descrPtr->egress_dsaWords[index] = mtag[index];
    }

    /* pad with Zeroes */
    for(/*continue*/ ; index <= SKERNEL_EXT_DSA_TAG_4_WORDS_E ; index++)
    {
        descrPtr->egress_dsaWords[index] = 0;
    }

    __LOG(("DSA BYTES:"));
    simLogDump(devObjPtr, SIM_LOG_INFO_TYPE_PACKET_E,
               (GT_PTR)mrvlTagPtr, (numOfWords*4));

}


/**
* @internal snetChtHaCreate4WordsMarvellToCpu function
* @endinternal
*
* @brief   HA - Create 4 words DSA tag with TO_CPU command
*
* @param[in] devObjPtr                -  pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in] dsaType                  - dsa tag type
* @param[in,out] mtag[/4/]                - 4 words for the build of the DSA tag
* @param[in] srcDevTrgDev             - src dev / trg dev
* @param[in] srcPortTrgPort           - src port / trg port
* @param[in] vidToUse                 - the vid to use in the DSA tag
* @param[in,out] mtag[/4/]                - 4 words for updated
*/
static void snetChtHaCreate4WordsMarvellToCpu
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN DSA_TAG_TYPE_E dsaType,
    INOUT GT_U32 mtag[/*4*/],
    IN GT_U32 srcDevTrgDev,
    IN GT_U32 srcPortTrgPort,
    IN GT_U32 vidToUse
)
{
    DECLARE_FUNC_NAME(snetChtHaCreate4WordsMarvellToCpu);

    GT_BIT  tunnelTerminated;
    GT_U32  regAddr, ttSendPassengerOffsetToCpu;
    GT_U32 *regPtr;

    /* packetIsTT is dependent on the HA GLOBAL CONFIG REG 1-> bit[9] */
    regAddr = SMEM_LION2_HA_GLOBAL_CONF1_REG(devObjPtr);
    regPtr = smemMemGet(devObjPtr, regAddr);
    ttSendPassengerOffsetToCpu = SMEM_U32_GET_FIELD(regPtr[0], 9, 1);
    if (ttSendPassengerOffsetToCpu == 0)
    {
        __LOG(("Global mode : ttSendPassengerOffsetToCpu = 0 : means use FlowId \n"));
        tunnelTerminated = 0;
    }
    else
    {
        __LOG(("Global mode : ttSendPassengerOffsetToCpu = 1 : means use Tunnel terminated and tunnel offset info \n"));
        tunnelTerminated = (descrPtr->eArchExtInfo.packetIsTT || descrPtr->tunnelTerminated);
    }

    /* word 0 - no changes */

    /**********/
    /* word 1 */
    /**********/
    SMEM_U32_SET_FIELD(mtag[1], 31, 1, 1);/* Extend */

    /* <orig is trunk> */
    __LOG_PARAM(descrPtr->origIsTrunk);
    SMEM_U32_SET_FIELD(mtag[1], 27, 1, descrPtr->origIsTrunk);

    /*SrcPort[6]/TrgPort[6]*/
    __LOG_PARAM(srcPortTrgPort);
    SMEM_U32_SET_FIELD(mtag[1], 11 , 1,
        SMEM_U32_GET_FIELD(srcPortTrgPort,6,1));

    /**********/
    /* word 2 */
    /**********/
    /*Extend*/
    SMEM_U32_SET_FIELD(mtag[2], 31 , 1, 1);

    __LOG_PARAM(tunnelTerminated);
    SMEM_U32_SET_FIELD(mtag[2],25,1 , tunnelTerminated);

    /*SrcPort[7]/TrgPort[7]*/
    SMEM_U32_SET_FIELD(mtag[2], 20 , 1,
        SMEM_U32_GET_FIELD(srcPortTrgPort,7,1));

    if(SMEM_CHT_IS_SIP6_GET(devObjPtr))
    {
        /*SrcPort[9:8]/TrgPort[9:8]*/
        SMEM_U32_SET_FIELD(mtag[2], 21 , 2,
                SMEM_U32_GET_FIELD(srcPortTrgPort,8,2));
    }

    else if(SMEM_CHT_IS_SIP5_20_GET(devObjPtr))
    {
        /* 9'th bit ONLY in the TO_CPU eDSA format !!!
           and only in HA (and not in TTI) */
        /* [HA-3515] - Increase TO_CPU eDSA physical port fields to 9 bits */

        /*SrcPort[8]/TrgPort[8]*/
        SMEM_U32_SET_FIELD(mtag[2], 21 , 1,
            SMEM_U32_GET_FIELD(srcPortTrgPort,8,1));
    }

    /* SRCePort[16:0]/ TRGePort[16:0] /SrcTrunk[11:0]*/
    __LOG_PARAM(descrPtr->eArchExtInfo.srcTrgEPort);
    if(SMEM_CHT_IS_SIP6_GET(devObjPtr))
    {
        SMEM_U32_SET_FIELD(mtag[2],3,16,descrPtr->eArchExtInfo.srcTrgEPort);
    }
    else
    {
        /* SRCePort[15:0]/ TRGePort[15:0] /SrcTrunk[11:0]*/
        SMEM_U32_SET_FIELD(mtag[2],3,17,descrPtr->eArchExtInfo.srcTrgEPort);
    }

    /*Tag0 TPID Index*/
    __LOG_PARAM(descrPtr->tpidIndex[SNET_CHT_TAG_0_INDEX_CNS]);
    SMEM_U32_SET_FIELD(mtag[2], 0 , 3, descrPtr->tpidIndex[SNET_CHT_TAG_0_INDEX_CNS]);

    /**********/
    /* word 3 */
    /**********/
    /*eVLAN[15:12]*/
    __LOG_PARAM(vidToUse);
    SMEM_U32_SET_FIELD(mtag[3], 27 , 4,
        SMEM_U32_GET_FIELD(vidToUse,12,4));

    /*Flow-ID / TTOffset*/
    if(tunnelTerminated)
    {
        __LOG_PARAM(descrPtr->tunnelTerminationOffset);
        SMEM_U32_SET_FIELD(mtag[3], 7 , 20,descrPtr->tunnelTerminationOffset);
    }
    else
    {
        __LOG_PARAM(descrPtr->flowId);
        SMEM_U32_SET_FIELD(mtag[3], 7 , 20,descrPtr->flowId);
    }

    if(SMEM_CHT_IS_SIP6_GET(devObjPtr))
    {
        /*word3[6:5] bits are cleared to 0*/
        SMEM_U32_SET_FIELD(mtag[3], 5, 2,0);
        /*SrcDev[9:5]/TrgDev[9:5]*/
        __LOG_PARAM(srcDevTrgDev);
        SMEM_U32_SET_FIELD(mtag[3], 0 , 5,
            SMEM_U32_GET_FIELD(srcDevTrgDev,5,5));
    }
    else
    {
        /*SrcDev[11:5]/TrgDev[11:5]*/
        __LOG_PARAM(srcDevTrgDev);
        SMEM_U32_SET_FIELD(mtag[3], 0 , 7,
            SMEM_U32_GET_FIELD(srcDevTrgDev,5,7));
    }

}


/**
* @internal snetChtHaCreateMarvellTagToCpuToAnalyzerVidCalc function
* @endinternal
*
* @brief   HA - calc TO_CPU/TO_ANALYZER vid
*
* @param[in] devObjPtr                -  pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in,out] mtag[/4/]                - (pointer to)4 words for the build of the DSA tag
* @param[in] useOrigVlanTagInfo       - do we use the original vlan tag info
* @param[in,out] mtag[/4/]                - (pointer to)4 words for the build of the DSA tag
*
* @param[out] vidToUsePtr              - (pointer to)the vid to use in the dsa
*/
static void snetChtHaCreateMarvellTagToCpuToAnalyzerVidCalc
(
    IN  SKERNEL_DEVICE_OBJECT           *devObjPtr,
    IN  SKERNEL_FRAME_CHEETAH_DESCR_STC *descrPtr,
    INOUT GT_U32                         mtag[/*4*/],
    IN GT_BOOL                           useOrigVlanTagInfo,
    OUT GT_U32                           *vidToUsePtr
)
{
    DECLARE_FUNC_NAME(snetChtHaCreateMarvellTagToCpuToAnalyzerVidCalc);

    GT_U32  vidToUse;

    if(useOrigVlanTagInfo == GT_FALSE)
    {
        __LOG_PARAM_WITH_NAME("bits [13:15] up" , descrPtr->up);
        SMEM_U32_SET_FIELD(mtag[0], 13, 3, descrPtr->up);/* bit [13:3] &0x7*/

        if(descrPtr->useIngressPipeVid == GT_FALSE || descrPtr->tunnelTerminated)
        {
            __LOG_PARAM_WITH_NAME("use vid from 'descrPtr->haAction.vid0'",descrPtr->haAction.vid0);
            vidToUse = descrPtr->haAction.vid0; /* support egress vlan translation */
        }
        else  /* this part of fix bugs# 91490 , 91072 */
        {
            if(devObjPtr->errata.ingressPipeTrapToCpuUseNotOrig802dot1Vid)
            {
                /* Errata in bobcat2 */
                __LOG_PARAM_WITH_NAME("use vid from 'descrPtr->haAction.vid0'",descrPtr->haAction.vid0);
                vidToUse = descrPtr->haAction.vid0; /* support egress vlan translation */
            }
            else
            {
                __LOG_PARAM_WITH_NAME("use vid from 'descrPtr->eVid'",descrPtr->eVid);
                vidToUse = descrPtr->eVid;
            }
        }
    }
    else
    {
        __LOG_PARAM_WITH_NAME("bits [13:15] up" , descrPtr->vlanTag802dot1dInfo.vpt);
        SMEM_U32_SET_FIELD(mtag[0], 13, 3, descrPtr->vlanTag802dot1dInfo.vpt);/* bit [13:3] &0x7*/

        if(devObjPtr->errata.ingressPipeTrapToCpuUseNotOrig802dot1Vid)
        {
            __LOG_PARAM_WITH_NAME("use vid from 'ORIG_VID' field",descrPtr->vid0Or1AfterTti);
            /* Errata in bobcat2 */
            vidToUse = descrPtr->vid0Or1AfterTti;
        }
        else
        {
            __LOG_PARAM_WITH_NAME("use vid from 'orig value on packet'",descrPtr->vlanTag802dot1dInfo.vid);
            vidToUse = descrPtr->vlanTag802dot1dInfo.vid;
        }

        if(descrPtr->srcTaggedTrgTagged && vidToUse > 0xFFF)
        {
            __LOG(("the copy should show the same vlan tag that was on src/trg vlan tag \n"
                    "so it is limited to 12 bits only (remove MSBits) \n" ));

            vidToUse &= 0xFFF;
        }
    }

    __LOG_PARAM(vidToUse);

    *vidToUsePtr = vidToUse;
}


/**
* @internal snetChtHaCreateMarvellTagToCpu function
* @endinternal
*
* @brief   HA - Create DSA tag with TO_CPU command
*
* @param[in] devObjPtr                -  pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in] egressPort               - egress port
*
* @param[out] mrvlTagPtr               - pointer to DSA tag
*/
static void snetChtHaCreateMarvellTagToCpu
(
    IN  SKERNEL_DEVICE_OBJECT           *devObjPtr,
    IN  SKERNEL_FRAME_CHEETAH_DESCR_STC *descrPtr,
    IN  GT_U32                           egressPort,
    OUT GT_U8_PTR                        mrvlTagPtr
)
{
    DECLARE_FUNC_NAME(snetChtHaCreateMarvellTagToCpu);

    GT_U32 mtag[4]={0,0,0,0};
    GT_U32 srcDevTrgDev;
    GT_U32 srcPortTrgPort;
    GT_BOOL   useOrigVlanTagInfo;/* do we use the original vlan tag info */
    GT_BIT  useSrcTrunkInDsaTag = 0;/* When a packets DSA Tag is replaced from FORWARD to TO_CPU and
            FORWARD_DSA<SrcIsTrunk> = 1 and SrcTrg = 0, this bit is set to 1 and
            this Tag {Word1[11:10], Word0[23:19]} = SrcTrunk[6:0] that is the
            Trunk number is extracted from the FORWARD DSA Tag.


            NOTE: for MTAG_4_WORDS_E , the <SrcIsTrunk> indicates actual 'srcIsTrunk' !!!
                 the eDSA hold place for trunkId in addition to phyPort !
                 the trunkId is muxed with ePort and not with phyPort
            */
    GT_U32  trgPhyPort;/* target physical port */
    GT_U32 * haGlobalConfRegPtr;
    GT_U32  value;
    GT_U32 timeStamp;
    GT_BIT forceNewDsaToCpu;
    GT_U32 vidToUse;/* the vid to use in the DSA tag */
    GT_U32 pktOrigBC; /* Packet Original Byte Count */

    /* Header Alteration Global Configuration */
    haGlobalConfRegPtr =
        smemMemGet(devObjPtr, SMEM_CHT3_ROUTE_HA_GLB_CNF_REG(devObjPtr));


    if(devObjPtr->supportEArch == 0)
    {
        trgPhyPort = descrPtr->trgEPort;
    }
    else
    {
        trgPhyPort = descrPtr->eArchExtInfo.trgPhyPort;
    }

    if(descrPtr->egrMarvellTagType == MTAG_4_WORDS_E)
    {
        /* the source/target info must come from the (ingress) DSA tag proper fields.
           do not override with local logic */
        srcDevTrgDev = descrPtr->srcTrgDev ;
        srcPortTrgPort = descrPtr->srcTrgPhysicalPort ;
    }
    else
    if (descrPtr->packetCmd == SKERNEL_EXT_PKT_CMD_FROM_CSCD_TO_CPU_E)
    {
        __LOG_PARAM(descrPtr->srcTrg);
        if ( descrPtr->srcTrg == 0 )
        {
            /* src */
            __LOG(("src"));
            srcDevTrgDev = descrPtr->srcDev ;
            srcPortTrgPort = (descrPtr->marvellTagged) ? descrPtr->origSrcEPortOrTrnk :
                            descrPtr->localDevSrcPort;
        }
        else
        {
            /*trg*/
            __LOG(("trg"));
            __LOG_PARAM(trgPhyPort);
            srcDevTrgDev = descrPtr->trgDev ;
            srcPortTrgPort = trgPhyPort  ;
        }
    }
    else
    {
        srcDevTrgDev = descrPtr->srcTrgDev ;
        srcPortTrgPort = descrPtr->srcTrgPhysicalPort ;

        if (devObjPtr->supportEArch &&
            (descrPtr->incomingMtagCmd != SKERNEL_MTAG_CMD_TO_CPU_E) && (descrPtr->egrMarvellTagType <= MTAG_EXTENDED_E))
        {
            /*
                If outDSA is DSA/ExtDSA
                    // when packet is received with inDSA != TO_CPU, and is trapped locally to CPU,
                       this field may contain the source trunk ID received in inDSA
                    OutDSA<SrcPort/TrgPort> = Desc<SrcTRGePort>
                Else // Egress eDSA tag
                    OutDSA<SrcPort/TrgPort> = Desc<Src Trg Phy Port>
            */

            srcPortTrgPort = descrPtr->eArchExtInfo.srcTrgEPort ;
        }

    }

    /* WORD 0 implementation */
    /* packet command - to CPU  & 0x3*/
    __LOG_PARAM(descrPtr->outGoingMtagCmd);
    SMEM_U32_SET_FIELD(mtag[0], 30, 2, descrPtr->outGoingMtagCmd);
    /* bit[29] SrcTagged/TrgTagged */
    __LOG_PARAM(srcDevTrgDev);
    SMEM_U32_SET_FIELD(mtag[0], 24, 5, srcDevTrgDev);/* bit [28:24] &0x3F*/
    __LOG_PARAM(srcPortTrgPort);
    SMEM_U32_SET_FIELD(mtag[0], 19, 5, srcPortTrgPort);/* bit [23:19] &0x1F*/
    if ( descrPtr->srcTrg == 0 )
    {
        __LOG_PARAM(descrPtr->tagSrcTagged[SNET_CHT_TAG_0_INDEX_CNS]);
        SMEM_U32_SET_FIELD(mtag[0], 29, 1, descrPtr->tagSrcTagged[SNET_CHT_TAG_0_INDEX_CNS]);/* bit [29] &0x1 */
    }
    else   /* descrPtr->srcTrg=1*/
    {
        __LOG_PARAM(descrPtr->trgTagged);
        SMEM_U32_SET_FIELD(mtag[0], 29, 1, descrPtr->trgTagged);/* bit [29] &0x1 */
    }

    useOrigVlanTagInfo = GT_FALSE;
    if((descrPtr->tagSrcTagged[SNET_CHT_TAG_0_INDEX_CNS] || descrPtr->marvellTagged))
    {
        /* this part of fix bugs# 91490 , 91072 */
        /* frame came with tag to the device/devices and it should be mirrored/trap
        to CPU 'as is' */

        /* note : in cheetah+ the behavior is not such */
        if(descrPtr->vlanTag802dot1dInfo.vid)
            useOrigVlanTagInfo = GT_TRUE;
    }

    if(descrPtr->basicMode == 1)
    {
        useOrigVlanTagInfo = GT_TRUE;
    }

    if(devObjPtr->supportForceNewDsaToCpu)
    {
        if ((descrPtr->marvellTagged) &&
            (descrPtr->incomingMtagCmd == SKERNEL_MTAG_CMD_TO_CPU_E))
        {
            /* the Force New DSA is not applied for incoming TO_CPU DSA packet */
            forceNewDsaToCpu = 0;
        }
        else
        {
            /* Read feature enabled/disabled state from register. */
            if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
            {
                forceNewDsaToCpu =
                    SMEM_LION3_HA_PHYSICAL_PORT_2_ENTRY_FIELD_AUTO_GET(devObjPtr,
                        descrPtr,
                        SMEM_LION3_HA_PHYSICAL_PORT_TABLE_2_FIELDS_FORCE_NEW_DSA_TO_CPU);
            }
            else
            {
                /* Save value to the descriptor for later use */
                forceNewDsaToCpu =
                    SMEM_U32_GET_FIELD(haGlobalConfRegPtr[0], 17, 1);
            }
        }

        if(forceNewDsaToCpu)
        {
            useOrigVlanTagInfo = GT_FALSE;
            /*when there is a TO_CPU <force new DSA> we'll always have <Tag0 Src/Trg Tagged> = 0*/
            __LOG(("NOTE: TO_CPU <force new DSA> forcing <Tag0 Src/Trg Tagged> = 0 (word 0 bit [29]) \n"));
            SMEM_U32_SET_FIELD(mtag[0], 29, 1, 0);/* bit [29] &0x1 */
        }
        else if(descrPtr->nestedVlanAccessPort && descrPtr->origVlanTagPtr &&
            descrPtr->tagSrcTagged[SNET_CHT_TAG_0_INDEX_CNS] &&
            SMEM_CHT_IS_SIP5_GET(devObjPtr))
        {
            useOrigVlanTagInfo = GT_TRUE;
        }

        if(descrPtr->origSrcTagged && descrPtr->useIngressPipeVid /* &&
           (descrPtr->vlanTag802dot1dInfo.vid != descrPtr->ingressPipeVid)*/)
        {
            /* The forceNewDsaToCpu relevant only for :
            1. Tagged packets
            2. The VID was modified by Ingress Pipe
            */
        }
        else
        {
            forceNewDsaToCpu = 0;
        }

        descrPtr->forceNewDsaToCpu = forceNewDsaToCpu;

        __LOG_PARAM(descrPtr->forceNewDsaToCpu);
    }

    snetChtHaCreateMarvellTagToCpuToAnalyzerVidCalc(devObjPtr,descrPtr,mtag,
        useOrigVlanTagInfo,&vidToUse);

    SMEM_U32_SET_FIELD(mtag[0], 0, 12, vidToUse);/* bit [0:11] &0xFFF*/

    /* the next byte is relevant for extended DSA tag only */
    if (descrPtr->egrMarvellTagType >= MTAG_EXTENDED_E )
    {
        /* CPU_Code[3:0]. Must be set to 0xF to indicate an Extended DSA tag */
        __LOG(("CPU_Code[3:0]. Must be set to 0xF to indicate an Extended DSA tag"));
        SMEM_U32_SET_FIELD(mtag[0], 12, 1, 0x1);/* bit [12] */
        SMEM_U32_SET_FIELD(mtag[0], 16, 3, 0x7);/* bit [18:16] */

        __LOG_PARAM(descrPtr->pktIsLooped);
        SMEM_U32_SET_FIELD(mtag[1], 28,1,descrPtr->pktIsLooped);/* bits [28] & 0x1 */

        __LOG_PARAM(descrPtr->dropOnSource);
        SMEM_U32_SET_FIELD(mtag[1], 29,1,descrPtr->dropOnSource);/* bits [29] & 0x1 */

        __LOG_PARAM(descrPtr->truncated);
        SMEM_U32_SET_FIELD(mtag[1], 26, 1, descrPtr->truncated);/* bit [18:16] always 0xF in word0*/

        if ((descrPtr->marvellTagged) &&
            (descrPtr->incomingMtagCmd == SKERNEL_MTAG_CMD_TO_CPU_E) &&
             descrPtr->marvellTaggedExtended >= SKERNEL_EXT_DSA_TAG_2_WORDS_E)/* field not exists in single word 'TO_CPU' */
        {
            /* use incoming DSA tag value for transit case. */
            pktOrigBC = descrPtr->inDsaPktOrigBC;
        }
        else
        {
            /* use ingress packet size */
            pktOrigBC = descrPtr->byteCount;

            if(descrPtr->marvellTagged)
            {
                /* When a packet forwarded to the CPU is from another device, it
                   gets the packet's byte count as it enters the second device and
                   not the original byte count of the packet that was received on
                   the first device. FEr#89 */
                __LOG(("FEr#89: pktOrigBC is packet's byte count (include the DSA bytes that ingress with) \n"));
            }

            if(descrPtr->is_byte_count_missing_4_bytes_in_HA)
            {
                pktOrigBC -= 4;
                __LOG(("Warning : The <pktOrigBC> [%d] is using 4 bytes less than it should \n",
                    pktOrigBC));
            }

        }

        __LOG_PARAM(pktOrigBC);
        SMEM_U32_SET_FIELD(mtag[1], 12, 14, pktOrigBC);/*bit [12:25] &0x3fff*/

        SMEM_U32_SET_FIELD(mtag[1], 10, 1, SMEM_U32_GET_FIELD(srcPortTrgPort,5,1));/*bit [10] &0x10*/

        /* If IPFIX timestamping is enabled */
       /* __LOG_PARAM(descrPtr->ipFixTimeStampValue);*/ /* prevent log diffs every run */
        if(descrPtr->ipFixTimeStampValue)
        {

            if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
            {
                value = SMEM_U32_GET_FIELD(haGlobalConfRegPtr[0], 13, 1);
            }
            else
            {
                value = SMEM_U32_GET_FIELD(haGlobalConfRegPtr[0], 12, 1);
            }


            /* If IPFIX timestamping to CPU is enabled */
            if(value)
            {
                /* Convert clock value in ticks to time stamp format */
                snetXcatIpfixTimestampFormat(devObjPtr, descrPtr->ipFixTimeStampValue, &timeStamp);
                __LOG_PARAM(timeStamp);
                if(timeStamp >= 0x8000)
                {
                    __LOG(("Note : only 15 LSBits (out of 16) of timeStamp into the DSA = [0x%x] \n",(timeStamp & 0x7FF)));
                }
                SMEM_U32_SET_FIELD(mtag[1], 12, 14, SMEM_U32_GET_FIELD(timeStamp,1,14));/*bit [12:25] &0x3fff*/
                SMEM_U32_SET_FIELD(mtag[1], 9, 1,   SMEM_U32_GET_FIELD(timeStamp,0,1));/*bit [9] &0x1*/
            }
        }

        __LOG_PARAM(descrPtr->srcTrg);
        SMEM_U32_SET_FIELD(mtag[1], 8, 1, descrPtr->srcTrg);/*bit [8] &0x1*/
        __LOG_PARAM(descrPtr->cpuCode);
        SMEM_U32_SET_FIELD(mtag[1], 0, 8, descrPtr->cpuCode);/*bit [0:7] &0xFF*/

        if ((descrPtr->marvellTagged) &&
            (descrPtr->incomingMtagCmd == SKERNEL_MTAG_CMD_TO_CPU_E) &&
             descrPtr->marvellTaggedExtended >= SKERNEL_EXT_DSA_TAG_2_WORDS_E)/* field not exists in single word 'TO_CPU' */
        {
            __LOG_PARAM(descrPtr->cfidei);
            /* This is the VLAN Tag CFI bit with which the packet was received on ingress DSA */
            SMEM_U32_SET_FIELD(mtag[1], 30, 1, descrPtr->cfidei);
        }
        else
        if(descrPtr->tagSrcTagged[SNET_CHT_TAG_0_INDEX_CNS])
        {
            __LOG_PARAM(descrPtr->vlanTag802dot1dInfo.cfi);
            /* This is the VLAN Tag CFI bit with which the packet was received from the network port */
            SMEM_U32_SET_FIELD(mtag[1], 30, 1, descrPtr->vlanTag802dot1dInfo.cfi);
        }
    }
    else
    {
        __LOG_PARAM(descrPtr->cpuCode);
        SMEM_U32_SET_FIELD(mtag[0], 16, 3, SMEM_U32_GET_FIELD(descrPtr->cpuCode,1,3)); /* bit [16:18] */
        SMEM_U32_SET_FIELD(mtag[0], 12, 1, SMEM_U32_GET_FIELD(descrPtr->cpuCode,0,1));/* bit [12]*/
    }

    if (descrPtr->srcTrg == 0 && descrPtr->origIsTrunk &&
        descrPtr->marvellTagged && descrPtr->incomingMtagCmd == SKERNEL_MTAG_CMD_FORWARD_E)
    {
        /* When a packets DSA Tag is replaced from FORWARD to TO_CPU and
            FORWARD_DSA<SrcIsTrunk> = 1 and SrcTrg = 0, this bit is set to 1 and
            this Tag {Word1[11:10], Word0[23:19]} = SrcTrunk[6:0] that is the
            Trunk number is extracted from the FORWARD DSA Tag. */
        useSrcTrunkInDsaTag = 1;
    }

    if(descrPtr->egrMarvellTagType == MTAG_4_WORDS_E)
    {
        /* the 12 bits of the srcTrunk will be set in word [2] bits[19:3]
        by snetChtHaCreate4WordsMarvellToCpu*/

        /* the eDSA hold place for trunkId in addition to phyPort ! */
        /* the trunkId is muxed with ePort and not with phyPort */
        useSrcTrunkInDsaTag = descrPtr->origIsTrunk;
    }
    else
    if(descrPtr->srcTrg == 0 && descrPtr->origIsTrunk &&
       devObjPtr->errata.srcTrunkToCpuIndication && (useSrcTrunkInDsaTag == 0))
    {
        /*
            the erratum not relevant to DSA forward that is converted to TO_CPU
        */

        /* Simulate an erratum:
            Wrong Trunk-ID/Source Port Information of Packet to CPU */
        GT_U32 trunkTmp = descrPtr->origSrcEPortOrTrnk >> 1;
        __LOG_PARAM(trunkTmp);
        SMEM_U32_SET_FIELD(mtag[0], 19, 5, trunkTmp);
        if (descrPtr->egrMarvellTagType >= MTAG_EXTENDED_E)
        {
            SMEM_U32_SET_FIELD(mtag[1], 10, 2, SMEM_U32_GET_FIELD(trunkTmp,5,2));
        }
    }
    else if(useSrcTrunkInDsaTag)
    {
        /* srcPortTrgPort is already trunk id */
        /* In the B0 silicon (and ch2,3..) the format of DSA was changed to
        provide WA for FEr#164. */
        __LOG_PARAM(descrPtr->origSrcEPortOrTrnk);
        SMEM_U32_SET_FIELD(mtag[0], 19, 5, descrPtr->origSrcEPortOrTrnk);
        if (descrPtr->egrMarvellTagType >= MTAG_EXTENDED_E)
        {
            SMEM_U32_SET_FIELD(mtag[1], 10, 2, SMEM_U32_GET_FIELD(descrPtr->origSrcEPortOrTrnk,5,2));
            SMEM_U32_SET_FIELD(mtag[1], 27, 1, 1);
        }
    }

    if (descrPtr->egrMarvellTagType == MTAG_4_WORDS_E)
    {
        snetChtHaCreate4WordsMarvellToCpu(devObjPtr,descrPtr,descrPtr->egrMarvellTagType,mtag,
            srcDevTrgDev,srcPortTrgPort,vidToUse);
    }

    convertDsaTagWordsToPacketBytes(devObjPtr,descrPtr,descrPtr->egrMarvellTagType,mtag,mrvlTagPtr);
}


/**
* @internal snetChtHaEDsaTpidGet_from_cpu function
* @endinternal
*
* @brief   HA - build of the EDSA FROM_CPU - get the TPID that is needed.
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
*                                      egressPort      - egress port
*
* @param[out] tpidIndexPtr             - pointer to the TPID index
*/
static void snetChtHaEDsaTpidGet_from_cpu
(
    IN    SKERNEL_DEVICE_OBJECT           *devObjPtr,
    IN    SKERNEL_FRAME_CHEETAH_DESCR_STC *descrPtr,
    IN    GT_U32                        tag_0_exist,
    IN    GT_U32                        tag_1_exist,
    IN    GT_U32                        tag0_is_outer_tag,
    OUT   GT_U32                        *tpidIndexPtr
)
{
    DECLARE_FUNC_NAME(snetChtHaEDsaTpidGet_from_cpu);

    GT_U32  tpidIndex;

    if(tag_0_exist && tag_1_exist) /* both exists */
    {
        if(tag0_is_outer_tag)
        {
            tpidIndex = descrPtr->tpidIndex[SNET_CHT_TAG_0_INDEX_CNS];
            __LOG(("the EDSA use tpidIndex[%d] from the TPID of most outer tag0 \n",
                tpidIndex));
        }
        else
        {
            tpidIndex = descrPtr->tpidIndex[SNET_CHT_TAG_1_INDEX_CNS];
            __LOG(("the EDSA use tpidIndex[%d] from the TPID of most outer tag1 \n",
                tpidIndex));
        }
    }
    else if(tag_0_exist) /* only tag 0 exists */
    {
        tpidIndex = descrPtr->tpidIndex[SNET_CHT_TAG_0_INDEX_CNS];
        __LOG(("the EDSA use tpidIndex[%d] from the TPID of most outer tag0 \n",
            tpidIndex));
    }
    else if(tag_1_exist) /* only tag 1 exists */
    {
        tpidIndex = descrPtr->tpidIndex[SNET_CHT_TAG_1_INDEX_CNS];
        __LOG(("the EDSA use tpidIndex[%d] from the TPID of most outer tag1 \n",
            tpidIndex));
    }
    else  /* no tags exists */
    {
        tpidIndex = descrPtr->tpidIndex[SNET_CHT_TAG_0_INDEX_CNS];
        __LOG(("the EDSA use tpidIndex[%d] of egress untagged (from tag0) \n",
            tpidIndex));
    }

    *tpidIndexPtr = tpidIndex;
}


/**
* @internal snetChtHaCreate4WordsMarvellFromCpu function
* @endinternal
*
* @brief   HA - Create 4 words DSA tag with FROM_CPU command
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in,out] mtag[/4/]                - 4 words for the build of the DSA tag
* @param[in,out] mtag[/4/]                - 4 words for updated
*/
static void snetChtHaCreate4WordsMarvellFromCpu
(
    IN    SKERNEL_DEVICE_OBJECT           *devObjPtr,
    IN    SKERNEL_FRAME_CHEETAH_DESCR_STC *descrPtr,
    INOUT GT_U32                           mtag[/*4*/],
    IN    GT_U32                           vidToUse
)
{
    DECLARE_FUNC_NAME(snetChtHaCreate4WordsMarvellFromCpu);
    GT_U32  tpidIndex;
    /* word 0 - no changes */

    /**********/
    /* word 1 */
    /**********/
    SMEM_U32_SET_FIELD(mtag[1], 31, 1, 1);/* Extend */
    if(descrPtr->useVidx == 0)
    {
        /*trgPhyPort[6]*/
        __LOG(("trgPhyPort[6]"));
        SMEM_U32_SET_FIELD(mtag[1], 11 , 1,
            SMEM_U32_GET_FIELD(descrPtr->eArchExtInfo.trgPhyPort,6,1));
    }

    /**********/
    /* word 2 */
    /**********/
    /*Extend*/
    SMEM_U32_SET_FIELD(mtag[2], 31 , 1, 1);

    if(descrPtr->useVidx == 0)
    {
        /*isTrgPhyPortValid*/
        __LOG(("isTrgPhyPortValid"));
        SMEM_U32_SET_FIELD(mtag[2], 29 , 1, descrPtr->eArchExtInfo.isTrgPhyPortValid);/* assume CPU set it on ingress*/
        /*trgPhyPort[7]*/
        SMEM_U32_SET_FIELD(mtag[2], 28 , 1,
            SMEM_U32_GET_FIELD(descrPtr->eArchExtInfo.trgPhyPort,7,1));
    }

    /*SrcID[11:5]*/
    SMEM_U32_SET_FIELD(mtag[2], 21 , 7,
        SMEM_U32_GET_FIELD(descrPtr->sstId,5,7));
    /*SrcDev[11:5]*/
    SMEM_U32_SET_FIELD(mtag[2], 14 , 7,
        SMEM_U32_GET_FIELD(descrPtr->srcDev,5,7));

    if(descrPtr->useVidx)
    {
        if (descrPtr->excludeIsTrunk)
        {
            /*ExcludedTrunk[11:7]*/
            __LOG(("ExcludedTrunk[11:7]"));
            SMEM_U32_SET_FIELD(mtag[2], 3 , 5,
                SMEM_U32_GET_FIELD(descrPtr->excludedTrunk,7,5));
        }
        else
        {
            /*ExcludedePort[16:6]*/
            __LOG(("ExcludedePort[16:6]"));
            SMEM_U32_SET_FIELD(mtag[2], 3 , 11,
                SMEM_U32_GET_FIELD(descrPtr->excludedPort,6,11));
        }
    }

    /*TPID Index*/
    snetChtHaEDsaTpidGet_from_cpu(devObjPtr,descrPtr,
        descrPtr->tagSrcTagged[SNET_CHT_TAG_0_INDEX_CNS],
        descrPtr->tagSrcTagged[SNET_CHT_TAG_1_INDEX_CNS],
        descrPtr->tag0IsOuterTag,
        &tpidIndex);
    __LOG_PARAM((tpidIndex));

    SMEM_U32_SET_FIELD(mtag[2], 0 , 3, tpidIndex);

    /**********/
    /* word 3 */
    /**********/
    /*eVLAN[15:12]*/
    SMEM_U32_SET_FIELD(mtag[3], 27 , 4,
        SMEM_U32_GET_FIELD(vidToUse,12,4));

    if(descrPtr->useVidx)
    {
        /*Indicates if the packet was received tagged with tag1*/
        SMEM_U32_SET_FIELD(mtag[3],26,1,descrPtr->tagSrcTagged[SNET_CHT_TAG_1_INDEX_CNS]);
        /* When both <SrcTagged / Tag0SrcTagged> = 1 (which is Tag0 SrcTagged) AND <Tag1 SrcTagged> = 1,
           indicating that the packet was received tagged with both Tag0 and Tag1, this bit indicates
           which of these two tags is the outer tag */
        SMEM_U32_SET_FIELD(mtag[3],25,1,descrPtr->tag0IsOuterTag);

        /*eVIDX[15:12]*/
        SMEM_U32_SET_FIELD(mtag[3], 20 , 4,
            SMEM_U32_GET_FIELD(descrPtr->eVidx,12,4));

        if(descrPtr->excludeIsTrunk == 0)
        {
            /*excludedIsPhyPort*/
            SMEM_U32_SET_FIELD(mtag[3], 19 , 1,descrPtr->eArchExtInfo.fromCpu.excludedIsPhyPort);

            /*ExcludedDev[11:5]*/
            __LOG(("ExcludedDev[11:5]"));
            SMEM_U32_SET_FIELD(mtag[3], 0 , 7,
                SMEM_U32_GET_FIELD(descrPtr->excludedDevice,5,7));
        }
    }
    else
    {
        if(SMEM_CHT_IS_SIP6_GET(devObjPtr))
        {
            /*trgPhyPort[9:8]*/
            SMEM_U32_SET_FIELD(mtag[3], 23 , 2,
                    SMEM_U32_GET_FIELD(descrPtr->eArchExtInfo.trgPhyPort,8,2));

            /*TRGePort[15:0]*/
            __LOG(("TRGePort[15:0]"));
            SMEM_U32_SET_FIELD(mtag[3], 7 , 16, descrPtr->trgEPort);
        }
        else
        {
            /*TRGePort[16:0]*/
            __LOG(("TRGePort[16:0]"));
            SMEM_U32_SET_FIELD(mtag[3], 7 , 17, descrPtr->trgEPort);
        }
        /*TrgDev[11:5]*/
        SMEM_U32_SET_FIELD(mtag[3], 0 , 7,
            SMEM_U32_GET_FIELD(descrPtr->trgDev,5,7));
    }
}


/**
* @internal snetChtHaCreateMarvellTagFromCpu function
* @endinternal
*
* @brief   HA - Create DSA tag with FROM_CPU command
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
*
* @param[out] mrvlTagPtr               - pointer to DSA tag
*/
static void snetChtHaCreateMarvellTagFromCpu
(
    IN  SKERNEL_DEVICE_OBJECT           *devObjPtr,
    IN  SKERNEL_FRAME_CHEETAH_DESCR_STC *descrPtr,
    OUT GT_U8_PTR                        mrvlTagPtr
)
{
    DECLARE_FUNC_NAME(snetChtHaCreateMarvellTagFromCpu);

    GT_U32 mtag[4]={0,0,0,0};             /* DSA tag 2 words */
    GT_U32  trgPhyPort;/* target physical port */
    GT_U32  TrgTagged;
    GT_U32 vidToUse;/* the vid to use in the DSA tag */

    vidToUse =  descrPtr->haAction.vid0;/* support vlan translation */

    if(devObjPtr->supportEArch == 0)
    {
        trgPhyPort = descrPtr->trgEPort;
        __LOG_PARAM_WITH_NAME("<trgPhyPort>",descrPtr->trgEPort);
    }
    else
    {
        trgPhyPort = descrPtr->eArchExtInfo.trgPhyPort;
        __LOG_PARAM_WITH_NAME("<trgPhyPort>",descrPtr->eArchExtInfo.trgPhyPort);
    }

    /* WORD 0 implementation */
    /* packet command - from CPU  & 0x3*/
    SMEM_U32_SET_FIELD(mtag[0], 30, 2, descrPtr->outGoingMtagCmd);

    /* TrgTagged or Untagged - bit [29]  & 0x1 */
    if(descrPtr->eArchExtInfo.haInfo.forceNewDsaFwdFromCpu ||
       descrPtr->forceToAddFromCpu4BytesDsaTag)
    {
        __LOG(("<TrgTagged> = 0 , due to : force to add DSA FROM_CPU \n"));
        TrgTagged = 0;
    }
    else
    {
        TrgTagged = descrPtr->trgTagged;
        __LOG_PARAM_WITH_NAME("<TrgTagged>",descrPtr->trgTagged);
    }

    SMEM_U32_SET_FIELD(mtag[0], 29, 1, TrgTagged);


    __LOG_PARAM(descrPtr->useVidx);
    if (descrPtr->useVidx)/*mc packet*/
    {
        __LOG_PARAM(descrPtr->eVidx);
        if (descrPtr->egrMarvellTagType == MTAG_STANDARD_E)
        { /* vidx , bit[20:28] , & 0x1FF */
            SMEM_U32_SET_FIELD(mtag[0], 20, 9, descrPtr->eVidx);
        }
        else
        {
            SMEM_U32_SET_FIELD(mtag[0], 19, 10, descrPtr->eVidx);
        }
    }
    else/*unicast packet*/
    {
        SMEM_U32_SET_FIELD(mtag[0], 24, 5, descrPtr->trgDev);/* bits [24:28]*/
        SMEM_U32_SET_FIELD(mtag[0], 19, 5, trgPhyPort);/* bits [19:23]*/
    }

    __LOG_PARAM(descrPtr->tc);
    __LOG_PARAM(descrPtr->cfidei);
    __LOG_PARAM(descrPtr->up);

    SMEM_U32_SET_FIELD(mtag[0], 18, 1, descrPtr->useVidx);/* bits [18] & 0x1 */
    SMEM_U32_SET_FIELD(mtag[0], 17, 1, descrPtr->tc);/* bits [17] & 0x1 */
    SMEM_U32_SET_FIELD(mtag[0], 16, 1, descrPtr->cfidei);/* bits [16] & 0x1 */
    SMEM_U32_SET_FIELD(mtag[0], 13, 3, descrPtr->up);/* bit [13:15] */

    /* fix JIRA : CPSS-9618 : [Falcon] [Centralized Chassis] [WM only] -
       egress 4B DSA tag is being constructed with wrong param*/
    if(SMEM_CHT_IS_SIP5_25_GET(devObjPtr) &&
        (descrPtr->eArchExtInfo.haInfo.forceNewDsaFwdFromCpu ||
         descrPtr->forceToAddFromCpu4BytesDsaTag))
    {
        /* fix for : HA-3532: Forced 4B From_CPU <UP, CFI> fields should be assigned to packet's
            <Traffic Class, Drop Precedence> */
        __LOG_PARAM(descrPtr->tc);
        SMEM_U32_SET_FIELD(mtag[0], 13, 3, descrPtr->tc);/* bit [13:15] */
        /* in Aldrin2 only the UP was fixed (according to JIRA info) */
        if(SMEM_CHT_IS_SIP6_GET(devObjPtr))
        {
            __LOG_PARAM(descrPtr->dp);
            SMEM_U32_SET_FIELD(mtag[0], 16, 1, descrPtr->dp);/* bits [16] & 0x1 */

            __LOG(("sip6: dsa 4B : UP from 'tc[%d]' (not from 'up[%d]') , cfi from 'dp[%d]' (not from cfidei[%d])\n",
                descrPtr->tc,
                descrPtr->up,
                descrPtr->dp,
                descrPtr->cfidei));
        }
        else
        {
            __LOG(("sip5.20: dsa 4B : UP from 'tc[%d]' (not from 'up[%d]') \n",
                descrPtr->tc,
                descrPtr->dp));
        }
    }

    if (descrPtr->egrMarvellTagType >= MTAG_EXTENDED_E)
    {
        SMEM_U32_SET_FIELD(mtag[0], 12, 1, 0x1);/* bit [12] */
    }

    __LOG_PARAM(vidToUse);

    SMEM_U32_SET_FIELD(mtag[0] , 0, 12, vidToUse);/* bit [0:11] */

    __LOG_PARAM(descrPtr->basicMode);
    if(descrPtr->basicMode)
    {
        __LOG_PARAM(descrPtr->vlanTag802dot1dInfo.cfi);
        __LOG_PARAM(descrPtr->vlanTag802dot1dInfo.vpt);
        __LOG_PARAM(descrPtr->vlanTag802dot1dInfo.vid);

        SMEM_U32_SET_FIELD(mtag[0], 16, 1, descrPtr->vlanTag802dot1dInfo.cfi);/* bits [16] & 0x1 */
        SMEM_U32_SET_FIELD(mtag[0], 13, 3, descrPtr->vlanTag802dot1dInfo.vpt);/* bit [13:15] */
        SMEM_U32_SET_FIELD(mtag[0] , 0, 12, descrPtr->vlanTag802dot1dInfo.vid);/* bit [0:11] */
    }


    if (descrPtr->egrMarvellTagType >= MTAG_EXTENDED_E)
    {
        __LOG_PARAM(descrPtr->egressFilterEn);
        __LOG_PARAM(descrPtr->qos.fromCpuQos.contolTc);
        __LOG_PARAM(descrPtr->egressFilterRegistered);


        SMEM_U32_SET_FIELD(mtag[1], 30, 1, descrPtr->egressFilterEn);/* bit [30] &0x1*/
        SMEM_U32_SET_FIELD(mtag[1], 29, 1, descrPtr->qos.fromCpuQos.contolTc);/* bit [29] &0x1*/
        SMEM_U32_SET_FIELD(mtag[1], 28, 1, descrPtr->egressFilterRegistered);/* bit [28] &0x1*/
        SMEM_U32_SET_FIELD(mtag[1], 27, 1, SMEM_U32_GET_FIELD(descrPtr->tc,2,1));/* bit [27] &0x4 */

        __LOG_PARAM(descrPtr->pktIsLooped);
        __LOG_PARAM(descrPtr->dropOnSource);
        SMEM_U32_SET_FIELD(mtag[1], 25,1,descrPtr->pktIsLooped);/* bits [25] & 0x1 */
        SMEM_U32_SET_FIELD(mtag[1], 26,1,descrPtr->dropOnSource);/* bits [26] & 0x1 */

        __LOG_PARAM(descrPtr->sstId);
        __LOG_PARAM(descrPtr->srcDev);
        SMEM_U32_SET_FIELD(mtag[1], 20, 5, descrPtr->sstId);  /* bit[20:24] & 0x1F */
        SMEM_U32_SET_FIELD(mtag[1], 15, 5, descrPtr->srcDev); /* bit[15:19] & 0x1F */
        SMEM_U32_SET_FIELD(mtag[1], 14, 1, SMEM_U32_GET_FIELD(descrPtr->tc,1,1));/* bit [14] &0x2 */

        if (descrPtr->useVidx)
        {
            __LOG_PARAM(descrPtr->excludeIsTrunk);


            SMEM_U32_SET_FIELD(mtag[1], 12, 2, descrPtr->eVidx>>10);/* VIDX[11:10] to bit[12:13] */
            SMEM_U32_SET_FIELD(mtag[1], 11, 1, descrPtr->excludeIsTrunk);/*bit[11] &0x1*/
            if (descrPtr->excludeIsTrunk)
            {

                __LOG_PARAM(descrPtr->mirrorToAllCpus);
                __LOG_PARAM(descrPtr->excludedTrunk);

                SMEM_U32_SET_FIELD(mtag[1], 10, 1, descrPtr->mirrorToAllCpus);/*bit10*/
                SMEM_U32_SET_FIELD(mtag[1], 0, 7, descrPtr->excludedTrunk);/*bit[6:0] &0x7F*/
            }
            else
            {
                __LOG_PARAM(descrPtr->excludedPort);
                __LOG_PARAM(descrPtr->excludedDevice);


                SMEM_U32_SET_FIELD(mtag[1], 5, 6, descrPtr->excludedPort);/*bit[10:5] &0x3F*/
                SMEM_U32_SET_FIELD(mtag[1], 0, 5, descrPtr->excludedDevice);/*bit[10:5] &0x1F*/
            }
        }
        else
        {/* bits [11:13] are reserved in this case */
            __LOG_PARAM(descrPtr->mailBoxToNeighborCPU);

            SMEM_U32_SET_FIELD(mtag[1], 13, 1, descrPtr->mailBoxToNeighborCPU);/*bit[13] &0x1 */
            SMEM_U32_SET_FIELD(mtag[1], 10, 1, SMEM_U32_GET_FIELD(trgPhyPort,5,1));/*bit[10] & 0x20*/
        }
    }

    if (descrPtr->egrMarvellTagType == MTAG_4_WORDS_E)
    {
        snetChtHaCreate4WordsMarvellFromCpu(devObjPtr, descrPtr, mtag , vidToUse);
    }

    convertDsaTagWordsToPacketBytes(devObjPtr,descrPtr,descrPtr->egrMarvellTagType,mtag,mrvlTagPtr);
}

/**
* @internal snetChtHaCreate4WordsMarvellToAnalyzer function
* @endinternal
*
* @brief   HA - Create 4 words DSA tag with TO_ANALYZER DSA tag
*
* @param[in] devObjPtr                -  pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in,out] mtag[/4/]                - 4 words for the build of the DSA tag
* @param[in] vlanId                   - vlan Id
* @param[in] srcDevTrgDev             - src dev / trg dev
* @param[in] srcPortTrgPort           - src port / trg port
* @param[in,out] mtag[/4/]                - 4 words for updated
*/
static void snetChtHaCreate4WordsMarvellToAnalyzer
(
    IN    SKERNEL_DEVICE_OBJECT           *devObjPtr,
    IN    SKERNEL_FRAME_CHEETAH_DESCR_STC *descrPtr,
    INOUT GT_U32                           mtag[/*4*/],
    IN    GT_U32                           vlanId,
    IN    GT_U32                           srcDevTrgDev,
    IN    GT_U32                           srcPortTrgPort
)
{
    DECLARE_FUNC_NAME(snetChtHaCreate4WordsMarvellToAnalyzer);
    GT_U32  analyzer_TrgPhyPort;
    GT_U32  analyzer_TrgDev;

    /* word 0 - no changes */

    /**********/
    /* word 1 */
    /**********/
    __LOG_PARAM_WITH_NAME("Extended word 1" , 1);
    SMEM_U32_SET_FIELD(mtag[1], 31, 1, 1);/* Extend */

    __LOG_PARAM_WITH_NAME("Analyzer_isTrgPhyPortValid" , descrPtr->eArchExtInfo.isTrgPhyPortValid);
    /*Analyzer_isTrgPhyPortValid*/
    SMEM_U32_SET_FIELD(mtag[1], 27 , 1,
        descrPtr->eArchExtInfo.isTrgPhyPortValid);

    __LOG_PARAM_WITH_NAME("Analyzer_UseVIDX" , descrPtr->useVidx);
    /*Analyzer_UseVIDX*/
    SMEM_U32_SET_FIELD(mtag[1], 26 , 1,
        descrPtr->useVidx);

    analyzer_TrgDev = (descrPtr->useVidx)? 0 /*Mirroring to VIDX - MC packet - Trg Dev is not valid*/
                                           : descrPtr->trgDev;
    __LOG_PARAM_WITH_NAME("Analyzer_TrgDev" , analyzer_TrgDev);
    /*Analyzer_TrgDev*/
    SMEM_U32_SET_FIELD(mtag[1], 14 , 12,analyzer_TrgDev);

    __LOG_PARAM_WITH_NAME("SrcPort[6]/TrgPort[6]/AnalyzerPort[6]" , SMEM_U32_GET_FIELD(srcPortTrgPort,6,1));
    /*SrcPort[6]/TrgPort[6]/AnalyzerPort[6]*/
    SMEM_U32_SET_FIELD(mtag[1], 11 , 1,
            SMEM_U32_GET_FIELD(srcPortTrgPort,6,1));

    if(descrPtr->eArchExtInfo.isTrgPhyPortValid && descrPtr->useVidx == 0)
    {
         if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
        {
            analyzer_TrgPhyPort = descrPtr->eArchExtInfo.trgPhyPort ; /*in eArch - use the trgPhyPort*/
        }
        else
        {
            analyzer_TrgPhyPort =  descrPtr->sniffTrgPort;
        }
    }
    else
    {
        __LOG(("analyzer_TrgPort : Valid only when <Analyzer Is Trg Phy Port Valid> == 1 && <Analyzer Use eVIDX> = 0)) \n"));
        analyzer_TrgPhyPort =  0;
    }
    __LOG_PARAM_WITH_NAME("Analyzer_TrgPhyPort" , analyzer_TrgPhyPort);
    /*Analyzer_TrgPort*/
    SMEM_U32_SET_FIELD(mtag[1], 0 , 8, analyzer_TrgPhyPort);
    /**********/
    /* word 2 */
    /**********/
    __LOG_PARAM_WITH_NAME("Extended word 2" , 1);
    /*Extend*/
    SMEM_U32_SET_FIELD(mtag[2], 31 , 1, 1);
    __LOG_PARAM_WITH_NAME("SrcPort[7]/TrgPort[7]/AnalyzerPort[7]" , SMEM_U32_GET_FIELD(srcPortTrgPort,7,1));

    if(SMEM_CHT_IS_SIP6_GET(devObjPtr))
    {
        /*SrcPort[9:7]/TrgPort[9:7]/AnalyzerPort[9:7]*/
        SMEM_U32_SET_FIELD(mtag[2], 20 , 3,
                SMEM_U32_GET_FIELD(srcPortTrgPort,7,3));

        __LOG_PARAM_WITH_NAME("SRCePort[15:0]/ TRGePort[15:0]" , descrPtr->eArchExtInfo.srcTrgEPort);
        /* SRCePort[15:0]/ TRGePort[15:0] */
        SMEM_U32_SET_FIELD(mtag[2], 3 , 16,
                descrPtr->eArchExtInfo.srcTrgEPort) ;
    }
    else
    {
        /*SrcPort[7]/TrgPort[7]/AnalyzerPort[7]*/
        SMEM_U32_SET_FIELD(mtag[2], 20 , 1,
                SMEM_U32_GET_FIELD(srcPortTrgPort,7,1));

        __LOG_PARAM_WITH_NAME("SRCePort[16:0]/ TRGePort[16:0]" , descrPtr->eArchExtInfo.srcTrgEPort);
        /* SRCePort[16:0]/ TRGePort[16:0] */
        SMEM_U32_SET_FIELD(mtag[2], 3 , 17,
                descrPtr->eArchExtInfo.srcTrgEPort) ;
    }

    __LOG_PARAM_WITH_NAME("Tag0 TPID Index" , descrPtr->tpidIndex[SNET_CHT_TAG_0_INDEX_CNS]);
    /*Tag0 TPID Index*/
    SMEM_U32_SET_FIELD(mtag[2], 0 , 3, descrPtr->tpidIndex[SNET_CHT_TAG_0_INDEX_CNS]);

    /**********/
    /* word 3 */
    /**********/
    __LOG_PARAM_WITH_NAME("eVLAN[15:12]" , SMEM_U32_GET_FIELD(vlanId,12,4));
    /*eVLAN[15:12]*/
    SMEM_U32_SET_FIELD(mtag[3], 27 , 4,
        SMEM_U32_GET_FIELD(vlanId,12,4));

    if(descrPtr->useVidx)
    {
        /*eVIDX (sniffEVidx) */
        __LOG_PARAM_WITH_NAME("eVIDX (sniffEVidx)" , descrPtr->eVidx);
        SMEM_U32_SET_FIELD(mtag[3], 7 , 16,
            descrPtr->eVidx);
    }
    else
    {
        /*AnalyzerEPort*/
        __LOG_PARAM_WITH_NAME("AnalyzerEPort" , descrPtr->trgEPort);
        SMEM_U32_SET_FIELD(mtag[3], 7 , 17,
            descrPtr->trgEPort);
    }

    if(SMEM_CHT_IS_SIP6_GET(devObjPtr))
    {
        /*word3[6:5] bits are cleared to 0*/
        SMEM_U32_SET_FIELD(mtag[3], 5, 2, 0);
        /*SrcDev[9:5]/TrgDev[9:5]*/
        __LOG_PARAM_WITH_NAME("SrcDev[11:5]/TrgDev[11:5]/AnalyzerDev[11:5]" , SMEM_U32_GET_FIELD(srcDevTrgDev,5,5));
        SMEM_U32_SET_FIELD(mtag[3], 0 , 5,
            SMEM_U32_GET_FIELD(srcDevTrgDev, 5, 5));
    }
    else
    {
        /*SrcDev[11:5]/TrgDev[11:5]/AnalyzerDev[11:5]*/
        __LOG_PARAM_WITH_NAME("SrcDev[11:5]/TrgDev[11:5]/AnalyzerDev[11:5]" , SMEM_U32_GET_FIELD(srcDevTrgDev,5,7));
        SMEM_U32_SET_FIELD(mtag[3], 0 , 7,
                           SMEM_U32_GET_FIELD(srcDevTrgDev, 5, 7));
    }



}
/**
* @internal snetChtHaCreateMarvellTagToAnalyzer function
* @endinternal
*
* @brief   HA - Create DSA tag with TO_ANALYZER command
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
*
* @param[out] mrvlTagPtr               - pointer to DSA tag
*/
static void snetChtHaCreateMarvellTagToAnalyzer
(
    IN  SKERNEL_DEVICE_OBJECT           *devObjPtr,
    IN  SKERNEL_FRAME_CHEETAH_DESCR_STC *descrPtr,
    OUT GT_U8_PTR                        mrvlTagPtr
)
{
    DECLARE_FUNC_NAME(snetChtHaCreateMarvellTagToAnalyzer);

    GT_U32 mtag[4]={0,0,0,0};
    GT_U32  trgPhyPort;/* target physical port */
    GT_U32  vidToUse;
    GT_U32 srcDevTrgDev;
    GT_U32 srcPortTrgPort;
    GT_BOOL   useOrigVlanTagInfo;/* do we use the original vlan tag info */

    if(devObjPtr->supportEArch == 0)
    {
        trgPhyPort = descrPtr->trgEPort;
    }
    else
    {
        trgPhyPort = descrPtr->eArchExtInfo.trgPhyPort;
    }

    /* WORD 0 implementation */
    __LOG_PARAM_WITH_NAME("packet command 30:31" , descrPtr->outGoingMtagCmd);
    /* packet command 30:31 */
    SMEM_U32_SET_FIELD(mtag[0], 30, 2, descrPtr->outGoingMtagCmd);

    /* bit[29] SrcTagged/TrgTagged */
    __LOG_PARAM_WITH_NAME("bit[29] SrcTagged/TrgTagged", descrPtr->srcTaggedTrgTagged);
    SMEM_U32_SET_FIELD(mtag[0], 29, 1, descrPtr->srcTaggedTrgTagged);

    srcDevTrgDev   = descrPtr->srcTrgDev;
    srcPortTrgPort = descrPtr->srcTrgPhysicalPort;

    __LOG_PARAM(trgPhyPort);
    __LOG_PARAM(srcDevTrgDev);
    __LOG_PARAM(srcPortTrgPort);


    __LOG_PARAM_WITH_NAME("bits [24:28] SrcDev/TrgDev" , srcDevTrgDev);
    SMEM_U32_SET_FIELD(mtag[0], 24, 5, srcDevTrgDev);/* bit [24:28] 0x1f*/
    __LOG_PARAM_WITH_NAME("bits[19:23] SrcPort/TrgPort" , srcPortTrgPort);
    SMEM_U32_SET_FIELD(mtag[0], 19, 5, srcPortTrgPort);/*bit[19:23]&0x1f*/

    __LOG_PARAM_WITH_NAME("bit [18] rxSniff" , descrPtr->rxSniff);
    SMEM_U32_SET_FIELD(mtag[0], 18, 1, descrPtr->rxSniff);/* bit [18] &0x1*/
    __LOG_PARAM_WITH_NAME("bit [16] cfi" , descrPtr->cfidei);
    SMEM_U32_SET_FIELD(mtag[0], 16, 1, descrPtr->cfidei);/* bits [16] & 0x1 */
    __LOG_PARAM_WITH_NAME("bits [13:15] up" , descrPtr->up);
    SMEM_U32_SET_FIELD(mtag[0], 13, 3, descrPtr->up);/* bit [13:15] &0x1*/

    if (descrPtr->egrMarvellTagType >= MTAG_EXTENDED_E)
    {
        __LOG_PARAM_WITH_NAME("bit [12] extended" , 1);
        SMEM_U32_SET_FIELD(mtag[0], 12, 1, 0x1);/* bit [12] */
    }

    useOrigVlanTagInfo = GT_FALSE;
    if(descrPtr->tagSrcTagged[SNET_CHT_TAG_0_INDEX_CNS])
    {
        /* this part of fix bugs# 91490 , 91072 */
        /* frame came with tag to the device/devices and it should be mirrored/trap
        to CPU 'as is' */

        /* note : in cheetah+ the behavior is not such */
        useOrigVlanTagInfo = GT_TRUE;
    }

    if(descrPtr->basicMode == 1)
    {
        useOrigVlanTagInfo = GT_TRUE;
    }

    if(descrPtr->rxSniff == 0)
    {
        descrPtr->useIngressPipeVid = GT_FALSE;
    }

    if(descrPtr->useIngressPipeVid == GT_FALSE)
    {
        useOrigVlanTagInfo = GT_FALSE;
    }

    __LOG_PARAM(useOrigVlanTagInfo);

    snetChtHaCreateMarvellTagToCpuToAnalyzerVidCalc(devObjPtr,descrPtr,mtag,
        useOrigVlanTagInfo,&vidToUse);

    if(descrPtr->basicMode)
    {
        __LOG_PARAM_WITH_NAME("basicMode : bit [16] cfi" , descrPtr->vlanTag802dot1dInfo.cfi);
        SMEM_U32_SET_FIELD(mtag[0], 16, 1, descrPtr->vlanTag802dot1dInfo.cfi);/* bits [16] & 0x1 */
        __LOG_PARAM_WITH_NAME("basicMode : bit [13:15] vpt" , descrPtr->vlanTag802dot1dInfo.vpt);
        SMEM_U32_SET_FIELD(mtag[0], 13, 3, descrPtr->vlanTag802dot1dInfo.vpt);/* bit [13:15] &0x1*/
    }

    __LOG_PARAM_WITH_NAME("[0:11] vlanId" , vidToUse);
    SMEM_U32_SET_FIELD(mtag[0], 0, 12, vidToUse);/* bit [0:11] &0xFFF */

    if (descrPtr->egrMarvellTagType >= MTAG_EXTENDED_E)
    {
        __LOG_PARAM_WITH_NAME("SrcPort[5]/TrgPort[5]/AnalyzerPort[5]" , SMEM_U32_GET_FIELD(srcPortTrgPort,5,1));
        SMEM_U32_SET_FIELD(mtag[1], 10, 1, SMEM_U32_GET_FIELD(srcPortTrgPort,5,1));/*bit[10]& 0x20*/
    }

    __LOG_PARAM_WITH_NAME("pktIsLooped" , descrPtr->pktIsLooped);
    SMEM_U32_SET_FIELD(mtag[1],28,1,descrPtr->pktIsLooped);/* bits [28] & 0x1 */
    __LOG_PARAM_WITH_NAME("dropOnSource" , descrPtr->dropOnSource);
    SMEM_U32_SET_FIELD(mtag[1],29,1,descrPtr->dropOnSource);/* bits [29] & 0x1 */

    if (descrPtr->egrMarvellTagType == MTAG_4_WORDS_E)
    {
        snetChtHaCreate4WordsMarvellToAnalyzer(devObjPtr,descrPtr,mtag,vidToUse,srcDevTrgDev,srcPortTrgPort);
    }

    convertDsaTagWordsToPacketBytes(devObjPtr,descrPtr,descrPtr->egrMarvellTagType,mtag,mrvlTagPtr);
}

/**
* @internal lion3HaEgressVlanTagsForDsaInfoGet function
* @endinternal
*
* @brief   (sip5) get vlan tag 0,1 info for DSA build -
*         relate to most outer vlan tags ,that may exists in the DSA and immediate
*         after it
* @param[in] devObjPtr                - (pointer to) device object.
* @param[in,out] descrPtr                 - (pointer to) frame data buffer Id
*
* @param[out] tag0ExistPtr             - pointer to is tag 0 exists (in final egress packet)
* @param[out] tag1ExistPtr             - pointer to is tag 1 exists (in final egress packet)
* @param[out] outerVlanIsTag1Ptr       - pointer to indication that tag 1 it the outer tag(in final egress packet)
*                                      RETURN:
*                                      None
*                                      COMMENTS:
*/
static void lion3HaEgressVlanTagsForDsaInfoGet
(
    IN  SKERNEL_DEVICE_OBJECT           *devObjPtr,
    INOUT  SKERNEL_FRAME_CHEETAH_DESCR_STC *descrPtr,
    OUT GT_BIT                  *tag0ExistPtr,
    OUT GT_BIT                  *tag1ExistPtr,
    OUT GT_BIT                  *outerVlanIsTag1Ptr

)
{
    DECLARE_FUNC_NAME(lion3HaEgressVlanTagsForDsaInfoGet);

    GT_BIT egress_tag_0_exist=0,egress_tag_1_exist=0,egress_tag_1_outer=0;

    switch(descrPtr->tr101EgressVlanTagMode)/* look in the tunnel in case of TS */
    {
        case SKERNEL_FRAME_TR101_VLAN_EGR_TAG_UNTAGGED_E:
            break;
        case SKERNEL_FRAME_TR101_VLAN_EGR_TAG_TAG0_E:
            egress_tag_0_exist = 1;
            break;
        case SKERNEL_FRAME_TR101_VLAN_EGR_TAG_TAG1_E:
            egress_tag_1_exist = 1;
            egress_tag_1_outer = 1;
            break;
        case SKERNEL_FRAME_TR101_VLAN_EGR_TAG_OUT_TAG0_IN_TAG1_E:
            egress_tag_0_exist = 1;
            egress_tag_1_exist = 1;
            break;
        case SKERNEL_FRAME_TR101_VLAN_EGR_TAG_OUT_TAG1_IN_TAG0_E:
            egress_tag_0_exist = 1;
            egress_tag_1_exist = 1;
            egress_tag_1_outer = 1;
            break;
        case SKERNEL_FRAME_TR101_VLAN_EGR_TAG_PUSH_TAG0_E:
            egress_tag_0_exist = 1;
            if(descrPtr->ingressVlanTag1Type == SKERNEL_FRAME_TR101_VLAN_TAG_OUTER_E)
            {
                egress_tag_1_exist = 1;
            }
            break;
        case SKERNEL_FRAME_TR101_VLAN_EGR_TAG_POP_OUT_TAG_E:
            if(descrPtr->ingressVlanTag1Type == SKERNEL_FRAME_TR101_VLAN_TAG_INNER_E)
            {
                egress_tag_1_exist = 1;
                egress_tag_1_outer = 1;
            }
            else
            if(descrPtr->ingressVlanTag0Type == SKERNEL_FRAME_TR101_VLAN_TAG_INNER_E)
            {
                egress_tag_0_exist = 1;
            }
            break;
        default:
            break;
    }

    __LOG_PARAM(egress_tag_0_exist);
    __LOG_PARAM(egress_tag_1_exist);


    *tag0ExistPtr      = egress_tag_0_exist;
    *tag1ExistPtr      = egress_tag_1_exist;
    *outerVlanIsTag1Ptr = egress_tag_1_outer;

    return;
}

/**
* @internal lion3HaFinalDsaUpdate_farward function
* @endinternal
*
* @brief   HA - update FORWARD DSA fields with info that was not known during build of the DSA.
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
*                                      egressPort      - egress port
*                                       is the DSA updated
*/
static void lion3HaFinalDsaUpdate_farward
(
    IN    SKERNEL_DEVICE_OBJECT           *devObjPtr,
    IN    SKERNEL_FRAME_CHEETAH_DESCR_STC *descrPtr
)
{
    DECLARE_FUNC_NAME(lion3HaFinalDsaUpdate_farward);

    GT_BIT  egress_tag_0_exist=0,egress_tag_1_exist=0;
    GT_BIT  forceNewDsaFwdFromCpu = descrPtr->eArchExtInfo.haInfo.forceNewDsaFwdFromCpu;
    GT_BIT  dsaReplacesVlan = descrPtr->eArchExtInfo.haInfo.dsaReplacesVlan;
    GT_BIT  stripL2 = descrPtr->eArchExtInfo.haInfo.stripL2;
    GT_BIT  outerVlanIsTag1;
    GT_BIT  tag0SrcTagged = descrPtr->tagSrcTagged[SNET_CHT_TAG_0_INDEX_CNS];
    GT_BIT  tag1SrcTagged = descrPtr->tagSrcTagged[SNET_CHT_TAG_1_INDEX_CNS];
    GT_U32  field;
    GT_BIT  eDsaOut  = descrPtr->egrMarvellTagType == MTAG_4_WORDS_E ? 1 : 0 ;

    /* info from the egress */
    lion3HaEgressVlanTagsForDsaInfoGet(devObjPtr, descrPtr,
        &egress_tag_0_exist, &egress_tag_1_exist,&outerVlanIsTag1);

    /*
      trkConstructed->Set("SrcTagged", (m_bForceNewDsaFwdFromCpu ? false :
                                    m_bTS ? m_bDsaReplacesVlan :
                                    m_bStripL2 ? (!m_bEDsaOut ? m_bDsaReplacesVlan : m_bEgressTag0Exists) :
                                    _bNestedVlan ? false :
                                    //L2 Bridged - pass SrcTagged info
                                    (!m_bEDsaOut && m_bOuterVlanIsTag1) ? _bTag1SrcTagged : _bTag0SrcTagged));
    */

    if(forceNewDsaFwdFromCpu)
    {
        __LOG(("<tag0SrcTagged> force 0 when 'force New Dsa Fwd From Cpu' \n"));
        field = 0;
    }
    else
    if(descrPtr->tunnelStart)
    {
        __LOG(("<tag0SrcTagged> set according to 'dsa Replaces Vlan' when 'tunnel Start' \n"));
        field = dsaReplacesVlan;
    }
    else
    if(stripL2)
    {
        if(eDsaOut == 0)
        {
            __LOG(("<tag0SrcTagged> set according to 'dsa Replaces Vlan' when 'stripL2' and 'non EDSA' \n"));
            field = dsaReplacesVlan;
        }
        else
        {
            __LOG(("<tag0SrcTagged> set according to 'egress tag 0 exist' when 'stripL2' and 'EDSA' \n"));
            field = egress_tag_0_exist;
        }
    }
    else
    if(descrPtr->nestedVlanAccessPort)
    {
        __LOG(("<tag0SrcTagged> forced to 0 when 'nested Vlan' \n"));
        field = 0;
    }
    else
    {
        if(eDsaOut == 0 && outerVlanIsTag1)
        {
            __LOG(("<tag0SrcTagged> set to 'tag1 Src Tagged' when 'non EDSA' and 'outer Vlan Is Tag1' (default case) \n"));
            field = tag1SrcTagged;
        }
        else
        {
            __LOG(("<tag0SrcTagged> set to 'tag0 Src Tagged' when 'EDSA' or 'outer Vlan Is NOT Tag1' (default case) \n"));
            field = tag0SrcTagged;
        }
    }

    field &= 1;
    descrPtr->eArchExtInfo.haInfo.dsa_forword.dsa_tag0_Src_Tagged = field;
    __LOG_PARAM(descrPtr->eArchExtInfo.haInfo.dsa_forword.dsa_tag0_Src_Tagged);


    if(eDsaOut == 0)
    {
        /* no more for non eDSA */
        return ;
    }


/*
    trkConstructed->Set("Tag0TpidIndex", (m_bForceNewDsaFwdFromCpu ? (unsigned)m_trkDescIn->Get("tag0_tpid_index") :
                                          m_bTS ? (unsigned)GetEPortAttribute1("ts_header_tpid_index") :
                                          m_bDsaReplacesVlan ?
                                          (m_bOuterVlanIsTag1 ? m_uEgressTag1TpidIndex : m_uEgressTag0TpidIndex) :
                                          (unsigned)m_trkDescIn->Get("tag0_tpid_index")));
*/
    if(forceNewDsaFwdFromCpu)
    {
        __LOG(("<tpidIndex> force 'ingress tag0 tpid index' when 'force New Dsa Fwd From Cpu' \n"));
        field = descrPtr->eArchExtInfo.haInfo.tpidIndex_atStartOfHa[SNET_CHT_TAG_0_INDEX_CNS];
    }
    else
    if(descrPtr->tunnelStart)
    {
        /* the EDSA should hold TPID of the vlan tag of the tunnel start*/
        __LOG(("<tpidIndex> set according to 'ts header tpid index' when 'tunnel Start' \n"));
        field = descrPtr->tpidIndexTunnelstart;
    }
    else
    if(dsaReplacesVlan)
    {
        if(outerVlanIsTag1)
        {
            __LOG(("<tpidIndex> set according to 'Egress Tag1 Tpid Index' when 'dsa Replaces Vlan' and 'outer Vlan Is Tag1' \n"));
            field = descrPtr->tpidIndex[SNET_CHT_TAG_1_INDEX_CNS];
        }
        else
        {
            __LOG(("<tpidIndex> set according to 'Egress Tag0 Tpid Index' when 'dsa Replaces Vlan' and 'outer Vlan Is NOT Tag1' \n"));
            field = descrPtr->tpidIndex[SNET_CHT_TAG_0_INDEX_CNS];
        }
    }
    else
    {
        __LOG(("<tpidIndex> set according to 'Ingress Tag0 Tpid Index' (default case) \n"));
        field = descrPtr->eArchExtInfo.haInfo.tpidIndex_atStartOfHa[SNET_CHT_TAG_0_INDEX_CNS];
    }

    field &= 0x7;
    descrPtr->eArchExtInfo.haInfo.dsa_forword.dsa_tpid_index = field;
    __LOG_PARAM(descrPtr->eArchExtInfo.haInfo.dsa_forword.dsa_tpid_index);

/*
    bool bDsaSrcTagged = trkConstructed->Get("SrcTagged");
    trkConstructed->Set("SrcTag0IsOuterTag", (!bDsaSrcTagged ? false :
                                              m_bTS ? true :
                                              m_bStripL2 ? !m_bOuterVlanIsTag1 :
                                              (bool)m_trkDescIn->Get("src_tag0_is_outer_tag")));
*/

    if(!descrPtr->eArchExtInfo.haInfo.dsa_forword.dsa_tag0_Src_Tagged)
    {
        __LOG(("<SrcTag0IsOuterTag> force to 0 when 'DSA <Tag0 Src Tagged>' = 0 \n"));
        field = 0;
    }
    else
    if(descrPtr->tunnelStart)
    {
        __LOG(("<SrcTag0IsOuterTag> force to 1 when 'tunnel start' \n"));
        field = 1;
    }
    else
    if(stripL2)
    {
        if(outerVlanIsTag1)
        {
            __LOG(("<SrcTag0IsOuterTag> set to 0 when 'stripL2' and 'outer Vlan Is Tag1' \n"));
            field = 0;
        }
        else
        {
            __LOG(("<SrcTag0IsOuterTag> set to 1 when 'stripL2' and 'outer Vlan Is NOT Tag1' \n"));
            field = 1;
        }
    }
    else
    {
        __LOG(("<SrcTag0IsOuterTag> set according to 'ingress tag0 Is Outer Tag' (default case) \n"));
        field = descrPtr->tag0IsOuterTag;
    }

    descrPtr->eArchExtInfo.haInfo.dsa_forword.dsa_tag0_is_outer_tag = field;
    __LOG_PARAM(descrPtr->eArchExtInfo.haInfo.dsa_forword.dsa_tag0_is_outer_tag);

/*
    trkConstructed->Set("Tag1SrcTagged", (m_bForceNewDsaFwdFromCpu || m_bTS || _bNestedVlan ? false :
                                          m_bStripL2 ? m_bEgressTag1Exists :
                                          (bool)m_trkDescIn->Get("tag1_src_tagged")));
*/

    if(forceNewDsaFwdFromCpu || descrPtr->tunnelStart || descrPtr->nestedVlanAccessPort)
    {
        if(forceNewDsaFwdFromCpu)
        {
            __LOG(("<Tag1SrcTagged> force to 0 when 'force New Dsa Fwd From Cpu' \n"));
        }
        else
        if(descrPtr->tunnelStart)
        {
            __LOG(("<Tag1SrcTagged> force to 0 when 'tunnel Start' \n"));
        }
        else
        {
            __LOG(("<Tag1SrcTagged> force to 0 when 'nested vlan' \n"));
        }
        field = 0;
    }
    else
    if(stripL2)
    {
        __LOG(("<Tag1SrcTagged> set according to 'egress tag1 exist' when stripL2 \n"));
        field = egress_tag_1_exist;
    }
    else
    {
        __LOG(("<Tag1SrcTagged> set according to 'ingress tag1 Src Tagged' (default case) \n"));
        field = tag1SrcTagged;
    }

    descrPtr->eArchExtInfo.haInfo.dsa_forword.dsa_tag1_Src_Tagged = field;
    __LOG_PARAM(descrPtr->eArchExtInfo.haInfo.dsa_forword.dsa_tag1_Src_Tagged);

    return ;
}
/**
* @internal lion3HaPrepareDsaAndTagsInfo function
* @endinternal
*
* @brief   HA - prepare values needed in HA when build DSA and vlan tags.
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
*
* @param[out] descrPtr                 - pointer to the frame's descriptor.
*/
static void lion3HaPrepareDsaAndTagsInfo
(
    IN    SKERNEL_DEVICE_OBJECT           *devObjPtr,
    IN    SKERNEL_FRAME_CHEETAH_DESCR_STC *descrPtr
)
{
    DECLARE_FUNC_NAME(lion3HaPrepareDsaAndTagsInfo);

    if(descrPtr->tunnelStart)
    {
        /*for TS that is not with ethernet passenger this is NULL */
        /*for TS that is     with ethernet passenger this is the mac da of the passenger */
        if(descrPtr->haToEpclInfo.macDaSaPtr == NULL)
        {
            __LOG(("set 'stripL2' = 0 , for TS that is not with ethernet passenger \n"));
        }
        else
        {
            __LOG(("set 'stripL2' = 1 , for TS that is     with ethernet passenger \n"));
        }
        descrPtr->eArchExtInfo.haInfo.stripL2 = descrPtr->haToEpclInfo.macDaSaPtr ? 1 : 0;
    }
    else
    if(descrPtr->tunnelTerminated)
    {
        __LOG(("set 'stripL2' according to 'l2 Valid' of the passenger when tunnel terminated (in ingress) \n"));
        descrPtr->eArchExtInfo.haInfo.stripL2 = descrPtr->l2Valid ? 0 : 1;
    }
    else
    {
        __LOG(("set 'stripL2' according to 'do Router Ha' (default case) \n"));
        descrPtr->eArchExtInfo.haInfo.stripL2 = descrPtr->doRouterHa ? 1 : 0;
    }

    __LOG_PARAM(descrPtr->eArchExtInfo.haInfo.stripL2);

    if(descrPtr->egrMarvellTagType == MTAG_TYPE_NONE_E)
    {
        __LOG(("'dsa Replaces Vlan' not relevant when not egress DSA tagged \n"));
        descrPtr->eArchExtInfo.haInfo.dsaReplacesVlan = 0;
    }
    else
    if (descrPtr->forceToAddFromCpu4BytesDsaTag)
    {
        __LOG(("force 'dsa Replaces Vlan' to 0 when '4 Bytes FROM_CPU' DSA is added to the packet \n"));
        descrPtr->eArchExtInfo.haInfo.dsaReplacesVlan = 0;
    }
    else
    if (descrPtr->eArchExtInfo.haInfo.forceNewDsaFwdFromCpu)
    {
        __LOG(("force 'dsa Replaces Vlan' to 0 when 'force New Dsa Fwd From Cpu' \n"));
        descrPtr->eArchExtInfo.haInfo.dsaReplacesVlan = 0;
    }
    else
    if (descrPtr->eArchExtInfo.haInfo.doubleTagToCpu ||
        (descrPtr->tunnelTerminated && descrPtr->eArchExtInfo.haInfo.rxTrappedOrMonitored))
    {
        __LOG(("force 'dsa Replaces Vlan' to 0 when 'double Tag To Cpu' or ('tunnel terminated' and 'rx Trapped Or Monitored')\n"));
        descrPtr->eArchExtInfo.haInfo.dsaReplacesVlan = 0;
    }
    else
    {
        __LOG(("set 'dsa Replaces Vlan' according to 'is egress need tags (tag0 or tag1)' \n"));
        descrPtr->eArchExtInfo.haInfo.dsaReplacesVlan =
            (descrPtr->tr101EgressVlanTagMode != SKERNEL_FRAME_TR101_VLAN_EGR_TAG_UNTAGGED_E) ?
            1 : 0;
    }

    __LOG_PARAM(descrPtr->eArchExtInfo.haInfo.dsaReplacesVlan);

    return;
}

/**
* @internal snetBc3HaForwardEDsaTrunkHashSet function
* @endinternal
*
* @brief   Updates the FORWARD eDSA tag to include 6 hash bits copied from the packet hash calculation
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in,out] mtag[/4/]                - 4 words for the build of the DSA tag
* @param[in,out] mtag[/4/]                - 4 words for updated
*/
static void snetBc3HaForwardEDsaTrunkHashSet
(
    IN SKERNEL_DEVICE_OBJECT           *devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC *descrPtr,
    INOUT GT_U32 mtag[/*4*/]
)
{
    DECLARE_FUNC_NAME(snetBc3HaForwardEDsaTrunkHashSet);

    GT_U32  fieldValue;

    if (SMEM_CHT_IS_SIP5_20_GET(devObjPtr) == 0)
    {
        __LOG(("snetBc3HaForwardEDsaTrunkHashSet:'hash in eDsa' not supported for current device \n"));
        return;
    }

    fieldValue =
        SMEM_LION3_HA_PHYSICAL_PORT_2_ENTRY_FIELD_AUTO_GET(devObjPtr,
           descrPtr,
           SMEM_SIP5_20_HA_PHYSICAL_PORT_TABLE_2_FIELDS_INSERT_HASH_INTO_FWD_EDSA_EN);

    __LOG_PARAM_WITH_NAME("Insert Hash into FORWARD eDSA Enable", fieldValue);

    if (fieldValue == 0)
    {
        return;
    }

    /* word 0 */
    __LOG_PARAM_WITH_NAME("Hash[0]",SMEM_U32_GET_FIELD(descrPtr->pktHash,0,1));
    SMEM_U32_SET_FIELD(mtag[0], 17, 1,
                       SMEM_U32_GET_FIELD(descrPtr->pktHash,0,1));/* Hash[0] */

    /* word 2 */
    __LOG_PARAM_WITH_NAME("Hash[1]",SMEM_U32_GET_FIELD(descrPtr->pktHash,1,1));
    SMEM_U32_SET_FIELD(mtag[2], 13, 1,
                       SMEM_U32_GET_FIELD(descrPtr->pktHash,1,1));/* Hash[1] */
    __LOG_PARAM_WITH_NAME("Hash[3:2]",SMEM_U32_GET_FIELD(descrPtr->pktHash,2,2));
    SMEM_U32_SET_FIELD(mtag[2], 19, 2,
                       SMEM_U32_GET_FIELD(descrPtr->pktHash,2,2));/* Hash[3:2] */

    /* word 3 */
    __LOG_PARAM_WITH_NAME("Hash[5:4]",SMEM_U32_GET_FIELD(descrPtr->pktHash,4,2));
    if(descrPtr->useVidx == 0)
    {
        SMEM_U32_SET_FIELD(mtag[3], 5, 2,
                           SMEM_U32_GET_FIELD(descrPtr->pktHash,4,2));/* Hash[5:4] */
    }
    else
    {
        SMEM_U32_SET_FIELD(mtag[3], 4, 2,
                           SMEM_U32_GET_FIELD(descrPtr->pktHash,4,2));/* Hash[5:4] */

    }
}

/**
* @internal snetChtHaCreate4WordsMarvellForward function
* @endinternal
*
* @brief   HA - Create 4 words DSA tag with FORWARD command
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in,out] mtag[/4/]                - 4 words for the build of the DSA tag
* @param[in,out] mtag[/4/]                - 4 words for updated
*/
static void snetChtHaCreate4WordsMarvellForward
(
    IN SKERNEL_DEVICE_OBJECT           *devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC *descrPtr,
    IN GT_U32                           dsaSrcDev,
    IN GT_U32                           origSrcEPortOrTrnk,
    IN GT_BIT                           origIsTrunk,
    IN GT_U32                           sstId,
    IN GT_U32                           trgPhyPort,
    INOUT GT_U32                        mtag[/*4*/],
    IN GT_U32                           vidToUse
)
{
    DECLARE_FUNC_NAME(snetChtHaCreate4WordsMarvellForward);

    /* word 0 - no changes */

    /**********/
    /* word 1 */
    /**********/
    __LOG_PARAM_WITH_NAME("word 1 Extend",1);
    SMEM_U32_SET_FIELD(mtag[1], 31, 1, 1);/* Extend */
    if(descrPtr->useVidx == 0)
    {
        __LOG_PARAM_WITH_NAME("trgPhyPort[6]",SMEM_U32_GET_FIELD(trgPhyPort,6,1));
        /*trgPhyPort[6]*/
        SMEM_U32_SET_FIELD(mtag[1], 11 , 1,
            SMEM_U32_GET_FIELD(trgPhyPort,6,1));
    }

    if(origIsTrunk == 0)
    {
        __LOG_PARAM_WITH_NAME("origSrcEPortOrTrnk[6]",SMEM_U32_GET_FIELD(origSrcEPortOrTrnk,6,1));
        SMEM_U32_SET_FIELD(mtag[1], 30 , 1,
            SMEM_U32_GET_FIELD(origSrcEPortOrTrnk,6,1));
    }

    /**********/
    /* word 2 */
    /**********/
    __LOG_PARAM_WITH_NAME("word 2 Extend",1);
    /*Extend*/
    SMEM_U32_SET_FIELD(mtag[2], 31 , 1, 1);

    if (SMEM_CHT_IS_SIP6_GET(devObjPtr))
    {
        /* skipFdbSaLookup (was 'reserved' in legacy devices) */
        SMEM_U32_SET_FIELD(mtag[2], 30 , 1, descrPtr->skipFdbSaLookup);
        __LOG_PARAM_WITH_NAME("skipFdbSaLookup",descrPtr->skipFdbSaLookup);
        if(descrPtr->skipFdbSaLookup)
        {
            /* in legacy device the bit would be '0' but here it is set to '1' */
            __LOG(("SIP6 : the <skipFdbSaLookup> in the eDSA word[2] bit [30] got value 1 , used to be 'reserved' in legacy devices \n"));
        }
    }


    if(descrPtr->useVidx == 0)
    {
        /*isTrgPhyPortValid*/
        __LOG_PARAM(descrPtr->eArchExtInfo.isTrgPhyPortValid);
        if(descrPtr->eArchExtInfo.isTrgPhyPortValid)
        {
            __LOG_PARAM_WITH_NAME("isTrgPhyPortValid",descrPtr->eArchExtInfo.isTrgPhyPortValid);
            SMEM_U32_SET_FIELD(mtag[2], 29 , 1, 1);
        }
        /*trgPhyPort[7] - The value in this field is valid only when <IsTrgPhyPortValid> == TRUE*/
        __LOG_PARAM_WITH_NAME("trgPhyPort[7]",SMEM_U32_GET_FIELD(trgPhyPort,7,1));
        SMEM_U32_SET_FIELD(mtag[2], 28 , 1,
            SMEM_U32_GET_FIELD(trgPhyPort,7,1));
    }

    /*SrcID[11:5]*/
    __LOG_PARAM_WITH_NAME("SrcID[11:5]",SMEM_U32_GET_FIELD(sstId,5,7));
    SMEM_U32_SET_FIELD(mtag[2], 21 , 7,
                       SMEM_U32_GET_FIELD(sstId,5,7));

    if(SMEM_CHT_IS_SIP6_GET(devObjPtr))
    {
        /*SrcDev[9:5]*/
        __LOG_PARAM_WITH_NAME("SrcDev[9:5]",SMEM_U32_GET_FIELD(dsaSrcDev,5,5));
        SMEM_U32_SET_FIELD(mtag[2], 14, 5, SMEM_U32_GET_FIELD(dsaSrcDev,5,5));
    }
    else
    {
        /*SrcDev[11:5]*/
        __LOG_PARAM_WITH_NAME("SrcDev[11:5]",SMEM_U32_GET_FIELD(dsaSrcDev,5,7));
        SMEM_U32_SET_FIELD(mtag[2], 14, 7,
                           SMEM_U32_GET_FIELD(dsaSrcDev,5,7));
    }

    if(origIsTrunk)
    {
        /*SrcTrunk[11:7]*/
        __LOG_PARAM_WITH_NAME("SrcTrunk[11:7]",SMEM_U32_GET_FIELD(origSrcEPortOrTrnk,7,5));
        SMEM_U32_SET_FIELD(mtag[2], 3 , 5,
            SMEM_U32_GET_FIELD(origSrcEPortOrTrnk,7,5));
    }
    else
    {
        /*SRCePort[16:7]*/
        __LOG_PARAM_WITH_NAME("SRCePort[16:7]",SMEM_U32_GET_FIELD(origSrcEPortOrTrnk,7,10));
        SMEM_U32_SET_FIELD(mtag[2], 3 , 10,
            SMEM_U32_GET_FIELD(origSrcEPortOrTrnk,7,10));
    }

    /**********/
    /* word 3 */
    /**********/
    /*eVLAN[15:12]*/
    __LOG_PARAM_WITH_NAME("eVLAN[15:12]", SMEM_U32_GET_FIELD(vidToUse,12,4));
    SMEM_U32_SET_FIELD(mtag[3], 27 , 4,
        SMEM_U32_GET_FIELD(vidToUse,12,4));

    /*Tag0 TPID Index*/
    __LOG_PARAM_WITH_NAME("Tag0 TPID Index", descrPtr->eArchExtInfo.haInfo.dsa_forword.dsa_tpid_index);
    SMEM_U32_SET_FIELD(mtag[2], 0 , 3, descrPtr->eArchExtInfo.haInfo.dsa_forword.dsa_tpid_index);

    /*Tag1 SrcTagged*/
    __LOG_PARAM_WITH_NAME("Tag1 SrcTagged", descrPtr->eArchExtInfo.haInfo.dsa_forword.dsa_tag1_Src_Tagged);
    SMEM_U32_SET_FIELD(mtag[3], 26 , 1, descrPtr->eArchExtInfo.haInfo.dsa_forword.dsa_tag1_Src_Tagged);

    /*SrcTag0 Is Outer Tag*/
    __LOG_PARAM_WITH_NAME("SrcTag0 Is Outer Tag", descrPtr->eArchExtInfo.haInfo.dsa_forword.dsa_tag0_is_outer_tag);
    SMEM_U32_SET_FIELD(mtag[3], 25 , 1, descrPtr->eArchExtInfo.haInfo.dsa_forword.dsa_tag0_is_outer_tag);

    if(descrPtr->useVidx)
    {
        /*eVIDX[15:12]*/
        __LOG_PARAM_WITH_NAME("eVIDX[15:12]", SMEM_U32_GET_FIELD(descrPtr->eVidx,12,4));
        SMEM_U32_SET_FIELD(mtag[3], 20 , 4,
            SMEM_U32_GET_FIELD(descrPtr->eVidx,12,4));

        /*Indicates whether the packet was received in the ingress device on a physical port that is a
          trunk member
            0 = Packet arrived on physical port that is configured as non-trunk member
            1 = Packet arrived on physical port that is configured as a trunk member*/
        __LOG_PARAM_WITH_NAME("origSrcPhyIsTrunk", descrPtr->eArchExtInfo.origSrcPhyIsTrunk);
        SMEM_U32_SET_FIELD(mtag[3],19,1,descrPtr->eArchExtInfo.origSrcPhyIsTrunk);
        if(descrPtr->eArchExtInfo.origSrcPhyIsTrunk)
        {
            __LOG_PARAM_WITH_NAME("origSrcPhyTrunk (12 bits)", descrPtr->eArchExtInfo.origSrcPhyPortTrunk);
            /* trunkId */
            SMEM_U32_SET_FIELD(mtag[3],7,12,descrPtr->eArchExtInfo.origSrcPhyPortTrunk);
        }
        else
        {
            /* since the field is 12 bits in the DSA it is not set to only 8 bits
               of physical port ! all 12 bits allowed !!!

               verified with GM BC3
            */
            __LOG_PARAM_WITH_NAME("origSrcPhyPort (12 bits)", descrPtr->eArchExtInfo.origSrcPhyPortTrunk);
            /* port */
            SMEM_U32_SET_FIELD(mtag[3],7,12,descrPtr->eArchExtInfo.origSrcPhyPortTrunk);
        }
        /* Indicates to the remote device whether to perform VIDX MC source filtering based on <OrigSrcPhyPort/Trunk>
            0 = Do not perform VIDX physical source filtering
            1 = Perform VIDX physical source filtering */
        __LOG_PARAM_WITH_NAME("phySrcMcFilterEn", descrPtr->eArchExtInfo.phySrcMcFilterEn);
        SMEM_U32_SET_FIELD(mtag[3],6,1,descrPtr->eArchExtInfo.phySrcMcFilterEn);

    }
    else
    {
        /* Applicable only for SIP6 devices */
        if(SMEM_CHT_IS_SIP6_GET(devObjPtr))
        {
            /*isTrgPhyPortValid*/
            /*trgPhyPort[9:8] - The value in this field is valid only when <IsTrgPhyPortValid> == TRUE*/
            __LOG_PARAM_WITH_NAME("trgPhyPort[9:8]",SMEM_U32_GET_FIELD(trgPhyPort,8,2));
            SMEM_U32_SET_FIELD(mtag[3], 23 , 2, SMEM_U32_GET_FIELD(trgPhyPort,8,2));

            /* Falcon TRGePort[15:0]*/
            __LOG_PARAM_WITH_NAME("TRGePort[15:0]", descrPtr->trgEPort);
            SMEM_U32_SET_FIELD(mtag[3], 7 , 16, descrPtr->trgEPort);
        }
        else
        {
            /*TRGePort[19:0]*/
            __LOG_PARAM_WITH_NAME("TRGePort[16:0]", descrPtr->trgEPort);
            SMEM_U32_SET_FIELD(mtag[3], 7 , 17, descrPtr->trgEPort);
        }

        if(SMEM_CHT_IS_SIP6_GET(devObjPtr))
        {
            /*TrgDev[9:5]*/
            __LOG_PARAM_WITH_NAME("TrgDev[9:5]", SMEM_U32_GET_FIELD(descrPtr->trgDev,5,5));
            SMEM_U32_SET_FIELD(mtag[3], 0 , 5,
                               SMEM_U32_GET_FIELD(descrPtr->trgDev,5,5));
        }
        else
        {
            /*TrgDev[11:5]*/
            __LOG_PARAM_WITH_NAME("TrgDev[11:5]", SMEM_U32_GET_FIELD(descrPtr->trgDev,5,7));
            SMEM_U32_SET_FIELD(mtag[3], 0 , 7,
                               SMEM_U32_GET_FIELD(descrPtr->trgDev,5,7));
        }
    }

    snetBc3HaForwardEDsaTrunkHashSet(devObjPtr,descrPtr,mtag);
}

/**
* @internal snetChtHaCreateMarvellForward_setCfiVptVid function
* @endinternal
*
* @brief   HA - Create DSA tag with FORWARD command
*         set fields cfi, vpt, vid to the tag
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
*/
static void snetChtHaCreateMarvellForward_setCfiVptVid
(
    IN    SKERNEL_DEVICE_OBJECT           *devObjPtr,
    IN    SKERNEL_FRAME_CHEETAH_DESCR_STC *descrPtr,
    IN    HA_INTERNAL_INFO_STC            *haInfoPtr,
    INOUT GT_U32                          *mtag,
    OUT   GT_U32                          *vidToUsePtr
)
{
    DECLARE_FUNC_NAME(snetChtHaCreateMarvellForward_setCfiVptVid);

    GT_U32 cfiBit,vpt,vid;

    if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        lion3HaFinalDsaUpdate_farward(devObjPtr,descrPtr);
    }


    /* default values (tag 0) into the DSA tag */
    cfiBit  = descrPtr->cfidei;
    vpt     = descrPtr->up;
    vid     =  descrPtr->haAction.vid0;/* support vlan translation */

    if(descrPtr->tr101ModeEn)
    {
        switch(descrPtr->tr101EgressVlanTagMode)/* tag state of the tunnel in case of TS */
        {
            case SKERNEL_FRAME_TR101_VLAN_EGR_TAG_POP_OUT_TAG_E:
                /* we pop the out tag , so if we had 2 tags in the ingress and
                   the inner is tag 1 , we need to use tag 1 , otherwise we keep
                   tag 0 info */
                if(descrPtr->ingressVlanTag1Type == SKERNEL_FRAME_TR101_VLAN_TAG_INNER_E)
                {
                    __LOG(("use tag 1 info in the DSA tag (popped outer tag 0 keep tag 1) \n"));
                    cfiBit  = descrPtr->cfidei1;
                    vpt     = descrPtr->up1;
                    vid     = descrPtr->vid1;
                }
                break;

            case SKERNEL_FRAME_TR101_VLAN_EGR_TAG_TAG1_E:
            case SKERNEL_FRAME_TR101_VLAN_EGR_TAG_OUT_TAG1_IN_TAG0_E:
                __LOG(("use tag 1 info in the DSA tag \n"));
                cfiBit  = descrPtr->cfidei1;
                vpt     = descrPtr->up1;
                vid     = descrPtr->vid1;
                break;

            default:
                break;
        }
    }

    if(haInfoPtr->tunnelStart && haInfoPtr->tsVlanTagInfo.vlanTagged == 0)
    {
        vid = haInfoPtr->tsVlanTagInfo.vlanId;
        __LOG(("VID [%d] always use tunnel start info in the DSA tag (regardless to 'TS tagged/not')\n",
            vid));
    }

    if(haInfoPtr->tunnelStart && haInfoPtr->tsVlanTagInfo.vlanTagged)
    {
        __LOG(("use tunnel start info in the DSA tag \n"));

        /* the TS vlan tag info is NOT in :
           descrPtr->up , descrPtr->eVid , descrPtr->cfidei */
        vpt = haInfoPtr->tsVlanTagInfo.up;
        vid = haInfoPtr->tsVlanTagInfo.vlanId;
        cfiBit = haInfoPtr->tsVlanTagInfo.cfi;
    }

    __LOG_PARAM(descrPtr->basicMode);

    if(descrPtr->basicMode == 1)
    {
        __LOG(("in basic mode use orig packet info \n"));

        cfiBit = descrPtr->vlanTag802dot1dInfo.cfi;
        vpt = descrPtr->vlanTag802dot1dInfo.vpt;
        vid = descrPtr->vlanTag802dot1dInfo.vid;
    }

    __LOG_PARAM(vid);
    __LOG_PARAM(vpt);
    __LOG_PARAM(cfiBit);

    SMEM_U32_SET_FIELD(mtag[0],16, 1,cfiBit);/* bits [16] & 0x1 */
    SMEM_U32_SET_FIELD(mtag[0],13,3,vpt);/* bit[13] & 0x7*/
    SMEM_U32_SET_FIELD(mtag[0],0,12,vid);/* bit [0:11] & 0xfff*/

    *vidToUsePtr = vid;
}

/*******************************************************************************
*   snetChtHaCreateMarvellForward
*
* DESCRIPTION:
*      HA - Create DSA tag with FORWARD command
* INPUTS:
*        devObjPtr       - pointer to device object.
*        descrPtr        - pointer to the frame's descriptor.
*        egressPort      - the local egress port (not global port)
*        destVlanTagged  - support tr101 tagging modes
*
* OUTPUTS:
*        mrvlTagPtr      - pointer to DSA tag
*
* RETURNS:
*
* COMMENTS:
*
*******************************************************************************/
static void snetChtHaCreateMarvellForward
(
    IN  SKERNEL_DEVICE_OBJECT           *devObjPtr,
    IN  SKERNEL_FRAME_CHEETAH_DESCR_STC *descrPtr,
    IN  GT_U32                           egressPort,
    IN  HA_INTERNAL_INFO_STC            *haInfoPtr,
    OUT GT_U8_PTR                        mrvlTagPtr
)
{
    DECLARE_FUNC_NAME(snetChtHaCreateMarvellForward);

    GT_BIT DSATagSrcDevMode = 0;
    GT_U32 mtag[4]={0,0,0,0};
    GT_U32  vidx;                  /*vidx to use in the DSA*/
    GT_U32  srcDev = 0;            /* src device to the DSA tag */
    GT_U32  origSrcEPortOrTrnk = 0;/* src port/trunk into the DSA tag*/
    GT_BIT  origIsTrunk = 0;       /* src is trunk ? into the DSA tag*/
    GT_U32  sstId = 0;             /*source-ID to the DSA tag*/
    GT_U32  regAddr; /* register address */
    GT_BIT  srcDevIsSetToLocal;/* is srcDev set per egress port (similar logic also apply to srcId)*/
    GT_BIT  routedSrcDeviceIdPortModeEnable = 0;
    GT_BIT  DSA_Src_modification_of_TT_packets = 0;
    GT_BIT  DSA_Src_modification_of_TS_packets = 0;
    GT_U32  qosProfileValue;
    GT_U32  trgPhyPort = 0;            /* target physical port */
    GT_U32  routed;
    GT_BIT  SrcTagged;
    GT_U32  haGlobalConfigValue;
    GT_U32  outputPortBit;  /* the bit index for the egress port */
    GT_BOOL isSecondRegister;/* is the per port config is in second register -- for bmp of ports */
    GT_BIT  lion2StackAggregatorEn = 0;
    GT_U32  lion2StackAggregator_srcDev = 0;
    GT_U32  lion2StackAggregator_origSrcEPortOrTrnk = 0;
    GT_U32  lion2StackAggregator_sstId = 0;
    GT_BIT  support_4words_dsa = SMEM_CHT_IS_SIP5_GET(devObjPtr) ? 1: 0;
    GT_BIT  holdErratum_haOrigIsTrunkReset = 0;/* indication that 'haOrigIsTrunkReset' not fixed in the device and relevant to current packet */
    GT_BIT  mismatchIsTrunkAndEPoprt = 0;/* indication that The eDSA/extDSA < Src Trunk/ Src ePort> may hold info about 'ePort' ... but <origIsTrunk> = 1. */
    GT_BIT  holdErratum_haSrcIdIgnored = 0;/* indication that 'haSrcIdIgnored' not fixed in the device and relevant to current packet */
    GT_U32  tmpField;/*field from register*/
    GT_U32  vidToUse;

    /* WORD 0 implementation */
    /* packet command 30:31 - FORWARD*/
    __LOG_PARAM_WITH_NAME("packet command 30:31 - FORWARD",descrPtr->outGoingMtagCmd);
    SMEM_U32_SET_FIELD(mtag[0], 30, 2, descrPtr->outGoingMtagCmd);

    /* set cfi, vpt, vid */
    snetChtHaCreateMarvellForward_setCfiVptVid(devObjPtr, descrPtr, haInfoPtr, mtag , &vidToUse);

    if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        /*enhanced logic taken from verifier*/
        SrcTagged = descrPtr->eArchExtInfo.haInfo.dsa_forword.dsa_tag0_Src_Tagged;
    }
    else
    {
        SrcTagged = descrPtr->tagSrcTagged[SNET_CHT_TAG_0_INDEX_CNS];
    }

    __LOG_PARAM(SrcTagged);
    /* bit[29] SrcTagged/TrgTagged */
    __LOG_PARAM_WITH_NAME("bit[29] SrcTagged/TrgTagged",SrcTagged);
    SMEM_U32_SET_FIELD(mtag[0],29,1,SrcTagged);/* bit [29] & 0x1*/


    if(devObjPtr->supportStackAggregation)
    {
        /* check that aggregator port enabled */
        regAddr = SMEM_LION2_HA_DSA_ERSPAN_REG(devObjPtr, 0);
        smemRegFldGet(devObjPtr, regAddr, egressPort, 1, &lion2StackAggregatorEn);

        if(lion2StackAggregatorEn)
        {
            GT_U32 tmp = 0;
            __LOG(("lion2 Stack Aggregator En == 1 --> override srcDev, srcPort, srcTrunk, srcId ,srcIsTrunk \n"));

            if(egressPort > 11)
            {
                regAddr = SMEM_LION2_HA_DSA_ERSPAN_REG(devObjPtr, 7);
            }
            else
            {
                regAddr = SMEM_LION2_HA_DSA_ERSPAN_REG(devObjPtr, egressPort/2 + 1);
            }
            tmp = (egressPort%2) * 16;

            /* override srcDev, srcPort, srcTrunk, srcId */
            smemRegFldGet(devObjPtr, regAddr, 0+tmp, 5, &lion2StackAggregator_srcDev);
            smemRegFldGet(devObjPtr, regAddr, 5+tmp, 6, &lion2StackAggregator_origSrcEPortOrTrnk);
            smemRegFldGet(devObjPtr, regAddr, 11+tmp, 5, &lion2StackAggregator_sstId);
        }
    }

    srcDev = descrPtr->srcDev;
    origSrcEPortOrTrnk = descrPtr->origSrcEPortOrTrnk;
    origIsTrunk = descrPtr->origIsTrunk;
    sstId = descrPtr->sstId;

    __LOG_PARAM(srcDev);
    __LOG_PARAM(origSrcEPortOrTrnk);
    __LOG_PARAM(origIsTrunk);
    __LOG_PARAM(sstId);


    regAddr = SMEM_CHT3_ROUTE_HA_GLB_CNF_REG(devObjPtr);
    smemRegGet(devObjPtr, regAddr, &haGlobalConfigValue);

    if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        srcDevIsSetToLocal = SMEM_LION3_HA_PHYSICAL_PORT_2_ENTRY_FIELD_AUTO_GET(devObjPtr,
                descrPtr,
                SMEM_LION3_HA_PHYSICAL_PORT_TABLE_2_FIELDS_SET_SOURCE_TO_LOCAL);

        routedSrcDeviceIdPortModeEnable = SMEM_LION3_HA_PHYSICAL_PORT_2_ENTRY_FIELD_AUTO_GET(devObjPtr,
                descrPtr,
                SMEM_LION3_HA_PHYSICAL_PORT_TABLE_2_FIELDS_ROUTED_SRC_DEVICE_ID_PORT_MODE_ENABLE);

        DSA_Src_modification_of_TT_packets = SMEM_U32_GET_FIELD(haGlobalConfigValue,18,1);
        DSA_Src_modification_of_TS_packets = SMEM_U32_GET_FIELD(haGlobalConfigValue,14,1);
    }
    else
    {
        srcDevIsSetToLocal                 = 0;

        snetChtHaPerPortInfoGet(devObjPtr,egressPort,&isSecondRegister,&outputPortBit);

        regAddr = SMEM_CHT_HA_ROUTE_DEVICE_ID_MODIFICATION_ENABLE_REG(devObjPtr);
        if(isSecondRegister == GT_TRUE)
        {
            regAddr += 4;
        }

        smemRegFldGet(devObjPtr, regAddr, outputPortBit, 1, &routedSrcDeviceIdPortModeEnable);

        if(SKERNEL_IS_LION_REVISON_B0_DEV(devObjPtr))/*lionB,xcat2,lion2*/
        {
            DSA_Src_modification_of_TT_packets = SMEM_U32_GET_FIELD(haGlobalConfigValue,25,1);
            DSA_Src_modification_of_TS_packets = SMEM_U32_GET_FIELD(haGlobalConfigValue,26,1);
        }
        else /*no such configuration , so logic is forced */
        {
            DSA_Src_modification_of_TT_packets = 1;
            DSA_Src_modification_of_TS_packets = 1;
        }
    }

    __LOG_PARAM(routedSrcDeviceIdPortModeEnable);
    __LOG_PARAM(DSA_Src_modification_of_TT_packets);
    __LOG_PARAM(DSA_Src_modification_of_TS_packets);

    /* <SET_SOURCE_TO_LOCAL> */
    if(srcDevIsSetToLocal)
    {
        __LOG(("(EgressPortConfig<Set Source to Local>==1) \n"));
    }
    else if(lion2StackAggregatorEn)
    {
        srcDevIsSetToLocal = 1;
        __LOG(("lion2 Stack Aggregator En == 1 \n"));
    }
    else if(descrPtr->marvellTagged && support_4words_dsa &&
            descrPtr->marvellTaggedExtended == SKERNEL_EXT_DSA_TAG_4_WORDS_E &&
            descrPtr->egrMarvellTagType <= MTAG_EXTENDED_E &&
            descrPtr->useVidx == 1 &&
            descrPtr->eArchExtInfo.origSrcPhyIsTrunk)
    {
        srcDevIsSetToLocal = 1;
        __LOG(("(inDSA == eDSA) &&  (outDSA == DSA/ExtDSA) && (Desc<Use_VIDX> == 1)&& (Desc<OrigSrcPhyIsTrunk> == 0) \n"));
    }
    else if(descrPtr->marvellTagged &&
            descrPtr->marvellTaggedExtended == SKERNEL_EXT_DSA_TAG_1_WORDS_E &&
            descrPtr->egrMarvellTagType <= MTAG_EXTENDED_E)
    {
        srcDevIsSetToLocal = 1;
        __LOG(("(inDSA == DSA) && (outDSA == ExtDSA) \n"));
    }
    else if(descrPtr->marvellTagged &&
            descrPtr->marvellTaggedExtended <= SKERNEL_EXT_DSA_TAG_2_WORDS_E &&
            descrPtr->egrMarvellTagType == MTAG_4_WORDS_E)
    {
        srcDevIsSetToLocal = 1;
        __LOG(("(inDSA == DSA/ExtDSA) && (outDSA == eDSA) \n"));
    }
    else if(descrPtr->doRouterHa && routedSrcDeviceIdPortModeEnable)
    {
        srcDevIsSetToLocal = 1;
        __LOG(("(<DoRouteHA> AND Config<DevIDModEn>) \n"));

    }
    else if(descrPtr->tunnelTerminated && DSA_Src_modification_of_TT_packets)
    {
        srcDevIsSetToLocal = 1;
        __LOG(("(TT && Config<TT modification>) \n"));
    }
    else if(haInfoPtr->tunnelStart && DSA_Src_modification_of_TS_packets)
    {
        srcDevIsSetToLocal = 1;
        __LOG(("(TS && Config<TS modification>) \n"));
    }
    else if (0 == SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        /*
            if all of the following are conditions are met:
            - packet is received with a non-extended DSA Tag FORWARD and
            - DSA<SrcIsTrunk> = 0 and
            - packet is transmitted with an extended DSA Tag FORWARD
        */
        if(descrPtr->marvellTagged &&
           descrPtr->marvellTaggedExtended == SKERNEL_EXT_DSA_TAG_1_WORDS_E &&
           descrPtr->origIsTrunk == 0 &&
           descrPtr->egrMarvellTagType >= MTAG_EXTENDED_E)
        {
            if(0 == SKERNEL_IS_LION_REVISON_B0_DEV(devObjPtr))
            {
                smemRegFldGet(devObjPtr, SMEM_CHT_DSA_TAG_TYPE_REG(devObjPtr),28 , 1 , &DSATagSrcDevMode);
            }
            else
            {
                smemRegFldGet(devObjPtr, haGlobalConfigValue,16 , 1 , &DSATagSrcDevMode);
            }

            if(DSATagSrcDevMode)
            {
                __LOG(("Support connecting to SOHO devices \n"));
                srcDevIsSetToLocal = 1;
            }
        }
    }

    if(srcDevIsSetToLocal ||
       descrPtr->marvellTagged == 0)/* not ingress with DSA */
    {
        if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
        {
            /* LOCAL_DEVICE_NUMBER */
            srcDev =
                SMEM_LION3_HA_PHYSICAL_PORT_2_ENTRY_FIELD_AUTO_GET(devObjPtr,
                    descrPtr,
                    SMEM_LION3_HA_PHYSICAL_PORT_TABLE_2_FIELDS_LOCAL_DEVICE_NUMBER);
        }
        else if(lion2StackAggregatorEn)
        {
            srcDev = lion2StackAggregator_srcDev;
        }
        else
        {
            srcDev = descrPtr->ownDev;
        }

        if(lion2StackAggregatorEn)
        {
            __LOG((" lion2 Stack Aggregator En == 1 --> srcDev = 0x%x \n",
                srcDev));
        }
        else
        if(srcDevIsSetToLocal)
        {
            __LOG((" srcDevIsSetToLocal = 1 --> srcDev = 0x%x \n",
                srcDev));
        }
        else
        {
            __LOG((" descrPtr->marvellTagged = 0 --> srcDev = 0x%x \n",
                srcDev));
        }

        __LOG_PARAM(srcDev);
    }

    if(srcDevIsSetToLocal && SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        if(devObjPtr->errata.haSrcIdIgnored)
        {
            holdErratum_haSrcIdIgnored = 1;
        }
        else
        {
            smemRegFldGet(devObjPtr, SMEM_SIP5_25_HA_GLOBAL_CONFIG_2_REG(devObjPtr),0 , 1 ,
                &tmpField);
            __LOG(("HA Global config2 , for SRC-ID : bit 0 <SrcDevIsSetToLocal is ignored> hold value [%d] \n",tmpField));
            if(tmpField)
            {
                holdErratum_haSrcIdIgnored = 0;
            }
            else
            {
                holdErratum_haSrcIdIgnored = 1;
            }
        }

        if(holdErratum_haSrcIdIgnored)
        {
            /* LOCAL_SOURCE_ID */
            sstId =
                SMEM_LION3_HA_PHYSICAL_PORT_2_ENTRY_FIELD_AUTO_GET(devObjPtr,
                    descrPtr,
                    SMEM_LION3_HA_PHYSICAL_PORT_TABLE_2_FIELDS_LOCAL_SOURCE_ID);

            __LOG((" srcDevIsSetToLocal = 1 --> sstId = 0x%x \n",
                sstId));
        }
        else
        {
            __LOG((" sstId = 0x%x , taken from the descrPtr->srcId \n",
                sstId));
        }

        __LOG_PARAM(sstId);
    }
    else if(lion2StackAggregatorEn)
    {
        sstId = lion2StackAggregator_sstId;
        __LOG(("lion2 Stack Aggregator En == 1 --> sstId = 0x%x \n",
            sstId));

        __LOG_PARAM(sstId);
    }

    if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        if(srcDevIsSetToLocal && (devObjPtr->errata.haOrigIsTrunkReset == 0)/*the device support fix for erratum*/)
        {
            smemRegFldGet(devObjPtr, SMEM_SIP5_25_HA_GLOBAL_CONFIG_2_REG(devObjPtr),1 , 1 ,
                &tmpField);
            __LOG(("HA Global Config2 , for <origIsTrunk> : bit 1 <origIsTrunk NOT reset for eDSA> hold value [%d] \n",tmpField));
            if(tmpField)
            {
                holdErratum_haOrigIsTrunkReset = 0;
            }
            else
            {
                holdErratum_haOrigIsTrunkReset = 1;
            }
        }
        else
        {
            holdErratum_haOrigIsTrunkReset = 1;
        }
    }

    /**************************************/
    /****** logic for <origIsTrunk> *******/
    /**************************************/
    if(SMEM_CHT_IS_SIP5_GET(devObjPtr) && srcDevIsSetToLocal && (0 == holdErratum_haOrigIsTrunkReset /*fixed the Erratum */))
    {
        origIsTrunk = descrPtr->origIsTrunk;

        if(descrPtr->egrMarvellTagType == MTAG_4_WORDS_E)
        {
            __LOG((" Egress with eDSA --> origIsTrunk = [%d] (from 'descrPtr->origIsTrunk') \n",
                origIsTrunk));
        }
        else
        {
            __LOG((" Egress with ext-DSA (and ingress with 4B-DSA) --> origIsTrunk = [%d] (from 'descrPtr->origIsTrunk') \n",
                origIsTrunk));
        }
    }
    else
    if(srcDevIsSetToLocal)
    {
        origIsTrunk = 0;

       if(holdErratum_haOrigIsTrunkReset)/* the erratum relevant to this packet */
        {
            /* the behavior is NOT OK and (since we RESET the 'origIsTrunk' --- ignored the descriptor value) */
            __LOG(("Erratum : 'Remote physical port issue' : since srcDevIsSetToLocal = 1 --> origIsTrunk = 0 (and not according to descrPtr->origIsTrunk[1]) \n"));
            __LOG(("Erratum : (ext-DSA or) eDSA Orig Src Trunk is lost when remote Source Physical Port mapping is enabled \n"));
        }
        else
        {
            /* the behavior is OK and expected */

            __LOG((" srcDevIsSetToLocal = 1 --> origIsTrunk = 0 \n"));
        }
    }

    /*********************************************/
    /****** logic for <origSrcEPortOrTrnk> *******/
    /*********************************************/
    if(srcDevIsSetToLocal)
    {
        if(SMEM_CHT_IS_SIP5_GET(devObjPtr) && srcDevIsSetToLocal && (0 == holdErratum_haOrigIsTrunkReset /*fixed the Erratum */))
        {
            origSrcEPortOrTrnk = descrPtr->origSrcEPortOrTrnk;
            __LOG(("origSrcEPortOrTrnk = [0x%x] --> from descrPtr->origSrcEPortOrTrnk \n",
                origSrcEPortOrTrnk));
        }
        else
        if(haInfoPtr->tunnelStart && DSA_Src_modification_of_TS_packets)
        {
            origSrcEPortOrTrnk = TS_RESERVED_PORT_NUM;
            __LOG(("(TS && Config<TS modification>) --> origSrcEPortOrTrnk = TS_RESERVED_PORT_NUM (60) \n"));

            if(origIsTrunk)
            {
                mismatchIsTrunkAndEPoprt = 1;
            }
        }
        else if(descrPtr->tunnelTerminated && DSA_Src_modification_of_TT_packets)
        {
            if(descrPtr->doRouterHa)
            {
                __LOG(("(TT && Config<TT modification>) && doRouterHa = 1 --> origSrcEPortOrTrnk = ROUTER_RESERVED_PORT_NUM (61) \n"));
                origSrcEPortOrTrnk = ROUTER_RESERVED_PORT_NUM;
            }
            else
            {
                __LOG(("(TT && Config<TT modification>) && doRouterHa = 0 --> origSrcEPortOrTrnk = TT_RESERVED_PORT_NUM (59) \n"));
                origSrcEPortOrTrnk = TT_RESERVED_PORT_NUM;
            }

            if(origIsTrunk)
            {
                mismatchIsTrunkAndEPoprt = 1;
            }

        }
        else if(descrPtr->doRouterHa && routedSrcDeviceIdPortModeEnable)
        {
            origSrcEPortOrTrnk = ROUTER_RESERVED_PORT_NUM;
            __LOG(("(doRouterHa && routedSrcDeviceIdPortModeEnable) --> origSrcEPortOrTrnk = ROUTER_RESERVED_PORT_NUM (61) \n"));

            if(origIsTrunk)
            {
                mismatchIsTrunkAndEPoprt = 1;
            }
        }
        else if(lion2StackAggregatorEn)
        {
            origSrcEPortOrTrnk = lion2StackAggregator_origSrcEPortOrTrnk;
            __LOG(("lion2 Stack Aggregator En == 1 --> origSrcEPortOrTrnk = lion2StackAggregator_origSrcEPortOrTrnk (%d) \n",
                origSrcEPortOrTrnk));

            if(origIsTrunk)
            {
                mismatchIsTrunkAndEPoprt = 1;
            }
        }
        else if(DSATagSrcDevMode == 1) /* not relevant to sip5 */
        {
            origSrcEPortOrTrnk = descrPtr->localDevSrcPort;
            __LOG(("DSATagSrcDevMode == 1 --> origSrcEPortOrTrnk = descrPtr->localDevSrcPort (%d) \n",
                origSrcEPortOrTrnk));

            if(origIsTrunk)
            {
                mismatchIsTrunkAndEPoprt = 1;
            }
        }
        else
        if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
        {
            origSrcEPortOrTrnk = descrPtr->eArchExtInfo.localDevSrcEPort;

            if(holdErratum_haOrigIsTrunkReset)/* the erratum relevant to this packet */
            {
                /* the behavior is NOT OK and (since we use the 'localDevSrcEPort' --- ignoring the descriptor 'origSrcEPortOrTrnk' value) */
                __LOG(("Erratum : 'Remote physical port issue' : since srcDevIsSetToLocal = 1 --> descrPtr->eArchExtInfo.localDevSrcEPort (0x%x) (and not according to descrPtr->origSrcEPortOrTrnk[0x%x]) \n",
                    descrPtr->eArchExtInfo.localDevSrcEPort,
                    descrPtr->origSrcEPortOrTrnk));
            }
            else
            {
                __LOG(("srcDevIsSetToLocal = 1 --> origSrcEPortOrTrnk = descrPtr->eArchExtInfo.localDevSrcEPort (%d) \n",
                    origSrcEPortOrTrnk));
            }

            if(origIsTrunk)
            {
                mismatchIsTrunkAndEPoprt = 1;
            }
        }

        __LOG_PARAM(origSrcEPortOrTrnk);
    }


    if(mismatchIsTrunkAndEPoprt)
    {
        __LOG(("ERROR : mismatch between <origIsTrunk> = 1 and the origSrcEPortOrTrnk[%d] that hold port/ePORT info !!! \n",
            origSrcEPortOrTrnk));
    }

    __LOG_PARAM_WITH_NAME("bits[28:24] SrcDev/TrgDev",srcDev);
    SMEM_U32_SET_FIELD(mtag[0],24,5,srcDev);/* bit[28:24 & 0x3f*/
    __LOG_PARAM_WITH_NAME("bit[19:23] origSrcEPortOrTrnk",origSrcEPortOrTrnk);
    SMEM_U32_SET_FIELD(mtag[0],19,5,origSrcEPortOrTrnk);/* bit[19:23] & 0x1f*/
    __LOG_PARAM_WITH_NAME("bit[18] origIsTrunk",origIsTrunk);
    SMEM_U32_SET_FIELD(mtag[0],18,1,origIsTrunk);/* bit[18] & 0x1*/

    if (descrPtr->egrMarvellTagType >= MTAG_EXTENDED_E)
    {
        /* extended DSA used */
        __LOG_PARAM_WITH_NAME("bit [12] extended",1);
        SMEM_U32_SET_FIELD(mtag[0], 12, 1, 1);/* bit [12] & 0x1*/

        if (origIsTrunk)
        {
            __LOG_PARAM_WITH_NAME("bit[29:30] origSrcEPortOrTrnk[6:5]",origSrcEPortOrTrnk >> 5);
            SMEM_U32_SET_FIELD(mtag[1],29,2,origSrcEPortOrTrnk >> 5);/*bit[29:30]&0x60*/
        }
        else
        {
            if (SMEM_CHT_IS_SIP5_GET(devObjPtr))
            {
                __LOG_PARAM_WITH_NAME("bit[29:30] origSrcEPortOrTrnk[6:5]",origSrcEPortOrTrnk >> 5);
                SMEM_U32_SET_FIELD(mtag[1], 29, 2, origSrcEPortOrTrnk >> 5); /*bit[29:30]&0x60*/
            }
            else
            {
                __LOG_PARAM_WITH_NAME("bit[29] origSrcEPortOrTrnk[5]",origSrcEPortOrTrnk >> 5);
                SMEM_U32_SET_FIELD(mtag[1], 29, 1, origSrcEPortOrTrnk >> 5); /*bit[29:30]&0x20*/
            }
        }

        __LOG_PARAM_WITH_NAME("bit[28] egressFilterRegistered",descrPtr->egressFilterRegistered);
        SMEM_U32_SET_FIELD(mtag[1],28,1,descrPtr->egressFilterRegistered);/*bit[28]&0x1*/

        __LOG_PARAM_WITH_NAME("bit[26] pktIsLooped",descrPtr->pktIsLooped);
        SMEM_U32_SET_FIELD(mtag[1],26,1,descrPtr->pktIsLooped);/* bits [26] & 0x1 */
        __LOG_PARAM_WITH_NAME("bit[27] dropOnSource" , descrPtr->dropOnSource);
        SMEM_U32_SET_FIELD(mtag[1], 27,1,descrPtr->dropOnSource);/* bits [27] & 0x1 */

        routed = descrPtr->routed;

        if(routed == 0 &&
           (haInfoPtr->tunnelStart || descrPtr->eArchExtInfo.haInfo.ePortModifyMacSa))
        {
            if(haInfoPtr->tunnelStart)
            {
                __LOG(("DSA <routed> = 1 --> from tunnelStart \n"));
            }
            else
            {
                __LOG(("DSA <routed> = 1 --> from ePortModifyMacSa \n"));
            }
            routed = 1;
        }

        __LOG_PARAM(routed);
        __LOG_PARAM(descrPtr->qos.qosProfile);
        __LOG_PARAM(descrPtr->useVidx);

        __LOG_PARAM_WITH_NAME("bit[25] routed",routed);
        SMEM_U32_SET_FIELD(mtag[1],25,1,routed);/*bit[25]&0x1*/
        __LOG_PARAM_WITH_NAME("bit[20:24] sstId",sstId);
        SMEM_U32_SET_FIELD(mtag[1],20,5,sstId);/*bit[20:24]&0x1f*/

        qosProfileValue = descrPtr->qos.qosProfile;

        if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
        {
            descrPtr->qos.egressExtendedMode =
                    SMEM_LION3_HA_PHYSICAL_PORT_2_ENTRY_FIELD_AUTO_GET(devObjPtr,
                                descrPtr,
                            SMEM_LION3_HA_PHYSICAL_PORT_TABLE_2_FIELDS_DSA_QOS_MODE);


            if(descrPtr->qos.egressExtendedMode)
            {
                __LOG(("The TC,DP should be in the DSA instead of qosProfile \n"
                       "at this stage only the TC [%d] is known \n",
                       descrPtr->tc));
                qosProfileValue = descrPtr->tc << 2;
            }
        }

        __LOG_PARAM_WITH_NAME("bit[13:19] qosProfileValue",qosProfileValue);
        SMEM_U32_SET_FIELD(mtag[1],13,7,qosProfileValue);/*bit[13:19]&0x7f*/

        __LOG_PARAM_WITH_NAME("bit[12] qosProfileValue",descrPtr->useVidx);
        SMEM_U32_SET_FIELD(mtag[1],12,1,descrPtr->useVidx);/*bit[12]&0x1 */

        if (descrPtr->useVidx)
        {
            if(devObjPtr->supportEArch == 0 || descrPtr->egrMarvellTagType != MTAG_EXTENDED_E)
            {
                vidx = descrPtr->eVidx;
            }
            else
            {
                vidx = descrPtr->eArchExtInfo.vidx;
            }

            __LOG_PARAM(vidx);

            __LOG_PARAM_WITH_NAME("bits [0:11] vidx",descrPtr->useVidx);
            SMEM_U32_SET_FIELD(mtag[1],0,12,vidx);/* bit [0:11] & 0xfff*/
        }
        else
        {
            if(devObjPtr->supportEArch == 0)
            {
                trgPhyPort = descrPtr->trgEPort;
            }
            else
            {
                if(descrPtr->egrMarvellTagType == MTAG_4_WORDS_E ||
                   descrPtr->eArchExtInfo.isTrgPhyPortValid )
                {
                    trgPhyPort = descrPtr->eArchExtInfo.trgPhyPort;
                }
                else
                {
                    trgPhyPort = descrPtr->trgEPort;
                }
            }
            __LOG_PARAM(trgPhyPort);
            __LOG_PARAM(descrPtr->trgDev);

            __LOG_PARAM_WITH_NAME("bits [5:10] trgPhyPort",trgPhyPort);
            SMEM_U32_SET_FIELD(mtag[1],5,6,trgPhyPort);/* bit [5:10] & 0x3f*/
            __LOG_PARAM_WITH_NAME("bits [0:4] descrPtr->trgDev",descrPtr->trgDev);
            SMEM_U32_SET_FIELD(mtag[1],0,5,descrPtr->trgDev);/* bit [0:4] & 0x1F*/
        }
    }

    if (descrPtr->egrMarvellTagType == MTAG_4_WORDS_E)
    {
        snetChtHaCreate4WordsMarvellForward(devObjPtr,descrPtr,
            srcDev,origSrcEPortOrTrnk,
            origIsTrunk,sstId,trgPhyPort,
            mtag,
            vidToUse);
    }

    convertDsaTagWordsToPacketBytes(devObjPtr,descrPtr,descrPtr->egrMarvellTagType,mtag,mrvlTagPtr);
}

/**
* @internal snetChtHaTsArpPointerInfo function
* @endinternal
*
* @brief   HA - get info about the need to do tunnel start and Arp modifications:
*         1. tunnel start + TS pointer
*         2. mac DA modification + ARP pointer
*         3. mac SA modification
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to frame data buffer Id
* @param[in] egressPort               - egress port.
* @param[in] needOriginalInfo         - we need original packet for rx_analyzer and to_cpu_from_ingress.
*/
static GT_VOID snetChtHaTsArpPointerInfo
(
    IN SKERNEL_DEVICE_OBJECT             *  devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC   *  descrPtr,
    IN GT_U32                               egressPort,
    IN GT_BOOL                              needOriginalInfo,
    OUT HA_INTERNAL_INFO_STC                *haInfoPtr
)
{
    DECLARE_FUNC_NAME(snetChtHaTsArpPointerInfo);

    GT_U32   fldVal;/* field value */
    GT_BIT   ePortTunnelStart;/* indication the target EPort request TS/ARP.
                                0 - ARP
                                1 - TS */
    GT_BIT   forceArpTsEPortDecision;/* force ARP/TS ePort Decision ? */
    GT_BIT   useArpFromEPort = 0;

    if (needOriginalInfo == GT_FALSE)
    {
        haInfoPtr->tunnelStart = descrPtr->tunnelStart;
        haInfoPtr->doRouterHa = descrPtr->doRouterHa;
        haInfoPtr->routed = descrPtr->routed;
    }
    else
    {
        /* disregard ingress pipe TS and Routing decisions for RX analyzer and TO_CPU from
           ingress descriptors. TS and Routing modifications may be enabled per ePort. */
        haInfoPtr->tunnelStart = 0;
        haInfoPtr->doRouterHa = 0;
        haInfoPtr->routed = 0;
    }

    haInfoPtr->tunnelStartPassengerType = descrPtr->tunnelStartPassengerType;
    haInfoPtr->arpPointer = descrPtr->arpPtr;

    /*  coming from the ingress Pipe : (also in eArch)
        the HA not support TS and (<doRouterHa>,<routed>) at the same time.
        the TS has priority to define the SA,DA */
    if(haInfoPtr->tunnelStart == 1)
    {
        __LOG(("haInfoPtr->tunnelStart == 1 , so set doRouterHa = 0 , routed = 0 \n"));
        haInfoPtr->doRouterHa = 0;
        haInfoPtr->routed     = 0;
    }
    else
    if(descrPtr->useArpForMacSa == GT_TRUE)/*relevant to sip5 only*/
    {
        /*Replace MAC SA with content of ARP entry pointed by inDesc<Tunnel/ArpPtr/NATptr>
          (and not by ARP pointer from target ePort attribute table).
          Do not use any of the existing MAC SA modification modes,
          and don't set <Routed> bit in eDSA/DSA*/
        __LOG(("descrPtr->useArpForMacSa == GT_TRUE , so set doRouterHa = 0 , routed = 0 \n"
               "do not check target ePort for ARP pointer \n" ));
        haInfoPtr->doRouterHa = 0;
        haInfoPtr->routed     = 0;

        return;
    }

    if(0 == SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        return;
    }
    /* check if tunnelStart enabled from the target EPort */
    /* tunnel start indication */
    ePortTunnelStart =
        SMEM_LION3_HA_EPORT_1_ENTRY_FIELD_AUTO_GET(devObjPtr,
            descrPtr,
            SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_TUNNEL_START);

    /* tunnel start is triggered from 'descriptor' of from the 'target EPort' */
    descrPtr->eArchExtInfo.haInfo.tunnelStart = (ePortTunnelStart || haInfoPtr->tunnelStart) ? 1 : 0;

    /* need resolution from where to take the tunnelStartPointer */
    /* Force ARP/TS ePort Decision */
    forceArpTsEPortDecision =
        SMEM_LION3_HA_EPORT_1_ENTRY_FIELD_AUTO_GET(devObjPtr,
            descrPtr,
            SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_FORCE_ARP_TS_EPORT_DECISION);

    if(descrPtr->eArchExtInfo.haInfo.tunnelStart)
    {
        if(((haInfoPtr->tunnelStart == ePortTunnelStart) && forceArpTsEPortDecision) ||/* both triggers , so use 'force' indication*/
            ((ePortTunnelStart == 1) && (haInfoPtr->tunnelStart == 0)))/* only TS from EPort is set */
        {
            /* take pointer from the EPort */
            __LOG(("take TS pointer from the EPort \n"));
            /* TS Pointer */
            descrPtr->eArchExtInfo.haInfo.tunnelStartPointer =
                SMEM_LION3_HA_EPORT_1_ENTRY_FIELD_AUTO_GET(devObjPtr,
                    descrPtr,
                    SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_TS_POINTER);
            /* TS Passenger Type */
            haInfoPtr->tunnelStartPassengerType =
                SMEM_LION3_HA_EPORT_1_ENTRY_FIELD_AUTO_GET(devObjPtr,
                    descrPtr,
                    SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_TS_PASSENGER_TYPE);
        }
        else
        {
            /* take pointer from the incoming descriptor */
            __LOG(("take TS pointer from the incoming descriptor \n"));
            descrPtr->eArchExtInfo.haInfo.tunnelStartPointer = descrPtr->tunnelPtr;
            haInfoPtr->tunnelStartPassengerType = descrPtr->tunnelStartPassengerType;
        }

        haInfoPtr->tunnelStart = 1;

        descrPtr->tunnelStart = haInfoPtr->tunnelStart;
        descrPtr->tunnelPtr = descrPtr->eArchExtInfo.haInfo.tunnelStartPointer;
        descrPtr->tunnelStartPassengerType = haInfoPtr->tunnelStartPassengerType;
    }

    if(ePortTunnelStart == 0)/* in the EPort : the ARP Pointer relevant only when no TS */
    {
        /*Modify MAC DA*/
        fldVal =
            SMEM_LION3_HA_EPORT_1_ENTRY_FIELD_AUTO_GET(devObjPtr,
                descrPtr,
                SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_MODIFY_MAC_DA);

        descrPtr->eArchExtInfo.haInfo.ePortModifyMacDa = fldVal;

        if(haInfoPtr->doRouterHa || fldVal)
        {
            if(((haInfoPtr->doRouterHa == fldVal) && forceArpTsEPortDecision) ||/* both triggers , so use 'force' indication*/
                ((fldVal == 1) && (haInfoPtr->doRouterHa == 0)))/* only 'Modify mac DA' from EPort is set */
            {
                __LOG(("take ARP pointer from the EPort \n"));
                /* take pointer from the EPort */
                haInfoPtr->arpPointer =
                    SMEM_LION3_HA_EPORT_1_ENTRY_FIELD_AUTO_GET(devObjPtr,
                        descrPtr,
                        SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_ARP_POINTER);
                useArpFromEPort = 1;
            }
            else
            {
                /* take pointer from the incoming descriptor */
                __LOG(("take ARP pointer from the incoming descriptor \n"));
                haInfoPtr->arpPointer = descrPtr->arpPtr;
            }

            haInfoPtr->doRouterHa = 1;
        }
    }

    if (descrPtr->isNat && (0 == useArpFromEPort))
    {
        __LOG(("NAT: Use the 'arpPtr' as value derived from descrPtr->arpPtr * 4 , for getting macDA \n"));
        /* the arpPtr was set for NAT in the Router */
        haInfoPtr->arpPointer = descrPtr->arpPtr * 4;
    }

    /* save the ARP pointer for the CNC counting */
    descrPtr->eArchExtInfo.haInfo.arpPointer =  haInfoPtr->arpPointer;

    /*Modify MAC SA*/
    fldVal =
        SMEM_LION3_HA_EPORT_1_ENTRY_FIELD_AUTO_GET(devObjPtr,
            descrPtr,
            SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_MODIFY_MAC_SA);

    descrPtr->eArchExtInfo.haInfo.ePortModifyMacSa = fldVal;

    if(haInfoPtr->routed || fldVal)
    {
        haInfoPtr->routed = 1;
    }

    return;
}

/**
* @internal snetChtHaVlanTranslate function
* @endinternal
*
* @brief   HA - Egress VLAN Translation logic.
*         supported from ch3
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to frame data buffer Id
* @param[in] egressPort               - egress port.
*/
static GT_VOID snetChtHaVlanTranslate
(
    IN SKERNEL_DEVICE_OBJECT             *  devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC   *  descrPtr,
    IN GT_U32                               egressPort
)
{
    DECLARE_FUNC_NAME(snetChtHaVlanTranslate);

    GT_U32                  regAddress;     /* register address */
    GT_U32                  regValue;       /* register field value */
    GT_BOOL                 translateVid;   /* VID has to be translated */
    GT_U32         outputPortBit;  /* the bit index for the egress port */
    GT_BOOL        isSecondRegister;/* is the per port config is in second register -- for bmp of ports */
    GT_U32          transateMode = 0;/*sip5_10*/
    GT_U32          ServiceID;/*sip5_10 : The eVLAN Service-ID. This can be used as I-SID in MiM TS, TNI in NVGRE, VNI in VXLAN. */


    if(SMEM_CHT_IS_SIP7_GET(devObjPtr))
    {
        /* Tag0 VID assignment was in EGF-EFT */
        descrPtr->haAction.vid0 = descrPtr->tag0Vid;
        __LOG(("sip7 : use egress vlan : VID0 [0x%3.3x] (as used by EGF-EFT) \n",
            descrPtr->haAction.vid0));
    }
    else
    {
        /* the HA need to know what vid0 to use ... by default it comes from 'eVid' */
        descrPtr->haAction.vid0 = descrPtr->eVid;
    }

    if(descrPtr->pktIsLooped)
    {
        __LOG(("egress vlan TRANSLATION : no VID translation , due to <pktIsLooped> == 1 \n"));

        return;
    }


    snetChtHaPerPortInfoGet(devObjPtr,egressPort,&isSecondRegister,&outputPortBit);

    translateVid = GT_FALSE;

    if(
       descrPtr->tunnelStart &&
       descrPtr->tunnelStartType == SKERNEL_FRAME_TUNNEL_START_TYPE_MPLS_E)
    {
        /* Packet is Ethernet-o-MPLS Tunnel-Start */

        if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
        {
            /*Enable Passenger VLAN Translation For TS*/
            smemRegFldGet(devObjPtr, SMEM_CHT3_ROUTE_HA_GLB_CNF_REG(devObjPtr),
                          11, 1, &regValue);
        }
        else
        {
            /*Enable Passenger VLAN Translation For TS*/
            smemRegFldGet(devObjPtr, SMEM_CHT3_ROUTE_HA_GLB_CNF_REG(devObjPtr),
                          10, 1, &regValue);
        }

        /* Controls Egress VLAN Translation of Ethernet tunnel start passengers */
        if (regValue)
        {
            __LOG(("Passenger VLAN Translation For TS is ENABELED \n"));
            translateVid = GT_TRUE;
            transateMode = regValue;
        }
    }
    else
    if(
       descrPtr->srcTrgPhysicalPort == SNET_CHT_PORT_60_CNS)
    {
        /* This is a reserved SrcPort indicating packet was TS on prior device */
        __LOG(("This is a reserved SrcPort indicating packet was TS on prior device"));
        translateVid = GT_FALSE;
    }
    else
    {
        if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
        {
            /*Enable egress VLAN translation per port*/
            regValue =
                SMEM_LION3_HA_EPORT_1_ENTRY_FIELD_AUTO_GET(devObjPtr,
                    descrPtr,
                    SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_VLAN_TRANS_EN);

            transateMode = regValue;
        }
        else
        {
            regAddress = SMEM_CHT3_HA_VLAN_TRANS_EN_REG(devObjPtr);
            if(isSecondRegister == GT_TRUE)
            {
                regAddress +=4;
            }

            smemRegFldGet(devObjPtr, regAddress, outputPortBit, 1, &regValue);
        }

        /* Enable Egress VLAN Translation */
        if(regValue)
        {
            __LOG(("the egress eport enable Egress VLAN Translation , check if packet type allow it too \n"));

            if(descrPtr->outGoingMtagCmd == SKERNEL_MTAG_CMD_FROM_CPU_E)
            {
                if(descrPtr->egressFilterEn)
                {
                    /* eVLAN translation is performed on FROM CPU packets only if inDesc<Egress Filter En> == 1 */
                    translateVid = GT_TRUE;
                    __LOG(("'FROM_CPU' with <Egress Filter En> == 1 ... allow egress vlan translation \n"));
                }
                else
                {
                    __LOG(("'FROM_CPU' with <Egress Filter En> == 0 ... not allow egress vlan translation \n"));
                }
            }
            else
            if (descrPtr->outGoingMtagCmd == SKERNEL_MTAG_CMD_TO_CPU_E)
            {
                if(descrPtr->srcTrg == 0)
                {
                    __LOG(("'TO_CPU' with <srcTrg> == 'SRC' ... not allow egress vlan translation \n"));
                }
                else
                if(!SKERNEL_IS_MATCH_DEVICES_MAC(descrPtr->srcTrgDev, descrPtr->ownDev,
                    devObjPtr->dualDeviceIdEnable.ha))
                {
                    __LOG(("'TO_CPU' not generated on local device ... so not allow egress vlan translation \n"));
                }
                else
                {
                    __LOG(("'TO_CPU' generated on local device ... allow egress vlan translation \n"));
                    translateVid = GT_TRUE;
                }
            }
            else
            if (descrPtr->outGoingMtagCmd == SKERNEL_MTAG_CMD_TO_TRG_SNIFFER_E)
            {
                if(descrPtr->rxSniff == 1)
                {
                    __LOG(("'TO_TRG_SNIFFER' with <rxSniff> == 1 ... so not allow egress vlan translation \n"));
                }
                else
                if(!SKERNEL_IS_MATCH_DEVICES_MAC(descrPtr->srcTrgDev, descrPtr->ownDev,
                    devObjPtr->dualDeviceIdEnable.ha))
                {
                    __LOG(("'TO_TRG_SNIFFER' not generated on local device ... not allow egress vlan translation \n"));
                }
                else
                {
                    __LOG(("'TO_TRG_SNIFFER' generated on local device ... allow egress vlan translation \n"));
                    translateVid = GT_TRUE;
                }
            }
            else
            if (descrPtr->packetCmd == SKERNEL_EXT_PKT_CMD_FORWARD_E)
            {
                __LOG(("'FORWARD packet' ... allow egress vlan translation \n"));
                translateVid = GT_TRUE;
            }
        }
    }

    if (translateVid == GT_TRUE)
    {
        regAddress = SMEM_CHT3_EGR_VLAN_TRANS_TBL_MEM(devObjPtr, descrPtr->eVid);
        smemRegGet(devObjPtr, regAddress, &regValue);

        if(SMEM_CHT_IS_SIP5_10_GET(devObjPtr))
        {
            /*The eVLAN Service-ID (24 bits). This can be used as I-SID in MiM TS, TNI in NVGRE, VNI in VXLAN.*/
            if(SMEM_CHT_IS_SIP7_GET(devObjPtr))
            {
                ServiceID = regValue & 0x00FFFFFF;
            }
            else
            {
                ServiceID = regValue >> 12 ;
            }

            __LOG_PARAM(ServiceID);
            __LOG_PARAM((ServiceID >> 12));

            /*SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_VLAN_TRANS_EN*/
            /* sip5_10 :
                0=Disabled:VLAN translation of packets transmitted via this port is disabled.
                1=Translate_eVlan_VID_2_VID0:Use the egress VLAN translation table to translate the evlan<VID> to VID0
                2=Translate_eVlan_SID_2_VID0:Assign the 12 MS-bits of the evlan<Service-ID> to VID0
                3=Translate_VID0_VID1:Assign the 12 MS-bits of the evlan<Service-ID> to VID0. Assign the 12 LS-bits of the eVLAN<Service-ID> to VID1.
            */
            if(transateMode == 1)/*Use the egress VLAN translation table to translate the evlan<VID> to VID0 */
            {
                /* set full value ! not only 12 LSBits */
                if(SMEM_CHT_IS_SIP7_GET(devObjPtr))
                {
                    /* translation was in EGF-EFT */
                    descrPtr->haAction.vid0 = descrPtr->tag0Vid;
                    __LOG(("sip7 : use egress vlan : VID0 [0x%3.3x] (as used by EGF-EFT) \n",
                        descrPtr->haAction.vid0));
                }
                else
                {
                    descrPtr->haAction.vid0 = regValue;
                    __LOG(("egress vlan TRANSLATION : VID0 to [0x%3.3x] \n",
                        descrPtr->haAction.vid0));
                }

                /* modify only 12 LSB of the eVlan */
/*
                SMEM_U32_SET_FIELD(descrPtr->eVid,0,12,regValue);
                descrPtr->vidModified = 1;
                __LOG(("TRANSLATION of VID to [%4.4x]\n",
                    descrPtr->eVid));
*/
            }
            else
            if(transateMode == 2)/*Assign the 12 MS-bits of the evlan<Service-ID> to VID0 */
            {
                /* only 12 LSB of the eVlan by 12 MSbits of ServiceID*/
                SMEM_U32_SET_FIELD(descrPtr->haAction.vid0,0,12,(ServiceID >> 12)/*12 MSbits*/);
                __LOG(("egress vlan TRANSLATION of VID according to ServiceID [0x%6.6x] (using 12 MSBits) to [0x%4.4x]\n",
                    ServiceID,/*(24 bits)*/
                    descrPtr->haAction.vid0));

                /* modify only 12 LSB of the eVlan by 12 MSbits of ServiceID*/
/*
                SMEM_U32_SET_FIELD(descrPtr->eVid,0,12,(ServiceID >> 12)--12 MSbits--);
                descrPtr->vidModified = 1;
                __LOG(("TRANSLATION of VID according to ServiceID [0x%6.6x] (using 12 MSBits[0x%3.3x]) to [%4.4x]\n",
                    ServiceID,--(24 bits)--
                    (ServiceID >> 12),--12 MSbits--
                    descrPtr->eVid));
*/
            }
            else
            if(transateMode == 3)/*Assign the 12 MS-bits of the evlan<Service-ID> to VID0.
                                   Assign the 12 LS-bits of the eVLAN<Service-ID> to VID1. */
            {
                /* only 12 LSB of the eVlan by 12 MSbits of ServiceID*/
                SMEM_U32_SET_FIELD(descrPtr->haAction.vid0,0,12,(ServiceID >> 12)/*12 MSbits*/);

                /* modify only 12 LSB of the eVlan by 12 MSbits of ServiceID*/
/*
                SMEM_U32_SET_FIELD(descrPtr->eVid,0,12,(ServiceID >> 12)--12 MSbits--);
                descrPtr->vidModified = 1;
*/
                /* modify vid1 by 12 LSbits of ServiceID*/
                descrPtr->vid1 = ServiceID & 0xFFF;

                __LOG(("egress vlan TRANSLATION of VID0 and VID1 according to ServiceID [0x%6.6x] (using 12 MSBits) to VID0[0x%4.4x] , (using 12 LSBits) VID1[0x%3.3x]\n",
                    ServiceID,/*(24 bits)*/
                    descrPtr->haAction.vid0,
                    descrPtr->vid1));

                __LOG_PARAM(descrPtr->vid1);
            }
        }
        else
        {
            SMEM_U32_SET_FIELD(descrPtr->haAction.vid0,0,12,regValue);
            __LOG(("egress vlan TRANSLATION : VID0 to [0x%4.4x]\n",
                descrPtr->haAction.vid0));

            /* modify only 12 LSB of the eVlan */
/*
            SMEM_U32_SET_FIELD(descrPtr->eVid,0,12,regValue);
            descrPtr->vidModified = 1;
            __LOG(("TRANSLATION of VID to [%4.4x]\n",
                descrPtr->eVid));
*/
        }

        descrPtr->vidModified = 1;/* indication for snetChtHaIsPacketChanged(...) */
    }
    else
    {
        __LOG(("egress vlan TRANSLATION : no VID translation \n"));
    }

    __LOG_PARAM(descrPtr->haAction.vid0);
    __LOG_PARAM(descrPtr->eVid);
    __LOG_PARAM(descrPtr->vid1);
}

/**
* @internal snetChtHaDsaForwardToFromCpuManipulation function
* @endinternal
*
* @brief   HA - It is required to support a mode per physical port, where a unicast FWD DSA packet is turned into a
*         FROM_CPU packet. This makes the line card device a simple fanout device without the ability to
*         manipulate the outgoing packet.
*         This logic is located at the last stage of the HA
* @param[in] devObjPtr                -  pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in] egressPort               - egress port (local port on multi-port group device)
*                                      CPU port is port 63
* @param[in] destVlanTagged           - send frame with tag.
* @param[in,out] dsaOrVlanTag[/32/]       - DSA TAG 8 words -- the DSA tag may require 4 words
* @param[in,out] dsaOrVlanTag[/32/]       - updated DSA TAG 8 words -- the DSA tag may require 4 words
*/
static void snetChtHaDsaForwardToFromCpuManipulation
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32   egressPort,
    IN GT_U8    destVlanTagged,
    INOUT GT_U8    dsaOrVlanTag[/*32*/]
)
{
    DECLARE_FUNC_NAME(snetChtHaDsaForwardToFromCpuManipulation);

    GT_U32  word0;/* word 0 of the DSA tag */
    GT_U32  word1;/* word 0 of the DSA tag */
    GT_U32  newWord0;/* updated word 0 of the DSA tag */
    GT_U32  newWord1;/* updated word 1 of the DSA tag */
    GT_U32  trgTagged;/*0 = outgoing packet DSA does not contain VID0 values
                        1 = outgoing packet DSA contains the VID0 values*/
    GT_U32  fldVal;/* temporary field value */
    GT_U32  MailBoxToNeighborCPU; /* the bit of MailBoxToNeighborCPU in the DSA tag */
    GT_U32  EgressFilterEn; /* the bit of EgressFilterEn in the DSA tag */
    GT_U32  use_vidx; /* the bit of use_vidx in the DSA tag */
#if 0
    GT_U32  Fanout_device_Reserved; /* the other constant bits in the DSA tag */
#endif

    smemRegFldGet(devObjPtr,
                SMEM_XCAT_HA_DSA_PORT_IS_FANOUT_DEVICE_REG(devObjPtr),
                (egressPort & 0x1f), 1, &fldVal);

    if(fldVal == 0)
    {
        return;
    }

    smemRegGet(devObjPtr,
                SMEM_XCAT_HA_DSA_FROM_CPU_CONSTANTS_TO_FANOUT_DEVICES_REG(devObjPtr),
                 &fldVal);

    MailBoxToNeighborCPU = SMEM_U32_GET_FIELD(fldVal,15,1);
#if 0
    Fanout_device_Reserved = SMEM_U32_GET_FIELD(fldVal,2,13);   SIM_TBD_BOOKMARK/* Fanout_device_Reserved : don't know how to use it*/
#endif
    EgressFilterEn = SMEM_U32_GET_FIELD(fldVal,1,1);
    use_vidx = SMEM_U32_GET_FIELD(fldVal,0,1);

    /*retrieve word 0,1 from the FORWARD DSA tag */
    word0 = dsaOrVlanTag[0] << 24 |
            dsaOrVlanTag[1] << 16 |
            dsaOrVlanTag[2] << 8 |
            dsaOrVlanTag[3] << 0 ;
    word1 = dsaOrVlanTag[4] << 24 |
            dsaOrVlanTag[5] << 16 |
            dsaOrVlanTag[6] << 8 |
            dsaOrVlanTag[7] << 0 ;

    /*base for the new word 0 is the existing word 0 */
    newWord0 = word0;
    newWord1 = 0;
    trgTagged = 0;

    if(descrPtr->vplsInfo.egressTagStateAssigned)
    {
        /* the device did assignment from the PCL/TTI/logical port table of descrPtr->vplsInfo.egressTagStateAssigned
            use it instead of info from the TXQ !
         */
        destVlanTagged = descrPtr->vplsInfo.egressTagState;
    }

    switch(destVlanTagged)
    {
        case SKERNEL_FRAME_TR101_VLAN_EGR_TAG_TAG0_E:
        case SKERNEL_FRAME_TR101_VLAN_EGR_TAG_OUT_TAG0_IN_TAG1_E:
            /* tag0 is 'outer' so it will be valid in the DSA */
            __LOG(("tag0 is 'outer' so it will be valid in the DSA"));
            trgTagged = 1;
            break;
        case SKERNEL_FRAME_TR101_VLAN_EGR_TAG_PUSH_TAG0_E:
            /* the 'pushed' tag0 go into the DSA */
            __LOG(("the 'pushed' tag0 go into the DSA"));
            trgTagged = 1;
            break;
        case SKERNEL_FRAME_TR101_VLAN_EGR_TAG_POP_OUT_TAG_E:
            /*implies that both tags 0,1 exists and tag1 is 'inner'
              so after the 'pop' tag0 still exists */
            if(descrPtr->ingressVlanTag1Type == SKERNEL_FRAME_TR101_VLAN_TAG_INNER_E)
            {
                trgTagged = 1;
            }
            break;
        default:
            break;
    }


    /* newWord0 */
    SMEM_U32_SET_FIELD(newWord0,30,2,1);/*31:30        TagCommand        1 = FROM_CPU, Packet from CPU */
    SMEM_U32_SET_FIELD(newWord0,29,1,trgTagged);/*29        TrgTagged*/
    SMEM_U32_SET_FIELD(newWord0,24,5,SMEM_U32_GET_FIELD(word1,0,5));/*28:24        TrgDev*/
    SMEM_U32_SET_FIELD(newWord0,19,5,SMEM_U32_GET_FIELD(word1,5,5));/*23:19        Trg Port[4:0]*/
    SMEM_U32_SET_FIELD(newWord0,18,1,use_vidx);/*18        use_vidx*/
    SMEM_U32_SET_FIELD(newWord0,17,1,descrPtr->tc);/*17        TC[0]*/

    /* newWord1 */
    SMEM_U32_SET_FIELD(newWord1,31,1,SMEM_U32_GET_FIELD(word1,31,1));/*31        Extend*/
    SMEM_U32_SET_FIELD(newWord1,30,1,EgressFilterEn);/*30        EgressFilterEn*/
    SMEM_U32_SET_FIELD(newWord1,29,1,0);/*29        Reserved*/
    SMEM_U32_SET_FIELD(newWord1,28,1,SMEM_U32_GET_FIELD(word1,28,1));/*28        EgressFilterRegistered*/
    SMEM_U32_SET_FIELD(newWord1,27,1,descrPtr->tc>>2);/*27        TC[2]*/
    SMEM_U32_SET_FIELD(newWord1,26,1,SMEM_U32_GET_FIELD(word1,27/*27 not bug*/,1));/*26        DropOnSource*/
    SMEM_U32_SET_FIELD(newWord1,25,1,SMEM_U32_GET_FIELD(word1,26/*26 not bug*/,1));/*25        PacketIsLooped*/
    SMEM_U32_SET_FIELD(newWord1,20,5,SMEM_U32_GET_FIELD(word1,20,5));/*24:20        Src-ID*/
    SMEM_U32_SET_FIELD(newWord1,15,5,SMEM_U32_GET_FIELD(word0/*word0 not bug*/,24,5));/*19:15        SrcDev*/
    SMEM_U32_SET_FIELD(newWord1,14,1,descrPtr->tc>>1);/*14        TC[1]*/
    SMEM_U32_SET_FIELD(newWord1,13,1,MailBoxToNeighborCPU);/*13        MailBoxToNeighborCPU*/
    SMEM_U32_SET_FIELD(newWord1,11,2,0);/*12:11        Reserved*/
    SMEM_U32_SET_FIELD(newWord1,10,1,SMEM_U32_GET_FIELD(word1,10,1));/*10        TrgPort[5]*/
    SMEM_U32_SET_FIELD(newWord1,0,10,0);/*9:0        Reserved*/

    /* build back the buffer bytes*/
    dsaOrVlanTag[0] = (GT_U8)SMEM_U32_GET_FIELD(newWord0,24,8);
    dsaOrVlanTag[1] = (GT_U8)SMEM_U32_GET_FIELD(newWord0,16,8);
    dsaOrVlanTag[2] = (GT_U8)SMEM_U32_GET_FIELD(newWord0, 8,8);
    dsaOrVlanTag[3] = (GT_U8)SMEM_U32_GET_FIELD(newWord0, 0,8);

    dsaOrVlanTag[4] = (GT_U8)SMEM_U32_GET_FIELD(newWord1,24,8);
    dsaOrVlanTag[5] = (GT_U8)SMEM_U32_GET_FIELD(newWord1,16,8);
    dsaOrVlanTag[6] = (GT_U8)SMEM_U32_GET_FIELD(newWord1, 8,8);
    dsaOrVlanTag[7] = (GT_U8)SMEM_U32_GET_FIELD(newWord1, 0,8);

    return;
}


/**
* @internal snetChtHaUpdateIPTotalLength function
* @endinternal
*
* @brief   updates ip total length packet field,
*         relevant for ethernet over ip tunnels
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in] haInfoPtr                - ha internal info
*
* @note function snetChtHaTsArpPointerInfo(...) could not know if passenger
*       will egress tagged/untagged tag0/1...
*       so snetChtHaTsArpPointerInfo(...) only calculated <length> in the
*       ip header according to L3 pointer of the passenger
*
*/
static void snetChtHaUpdateIPTotalLength
(
    IN SKERNEL_DEVICE_OBJECT           *devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC *descrPtr,
    IN HA_INTERNAL_INFO_STC            *haInfoPtr,
    IN GT_U32                           extraLength
)
{
    DECLARE_FUNC_NAME(snetChtHaUpdateIPTotalLength);

    GT_U32    fldVal;
    GT_U8    *totalLengthPtr;

    if(haInfoPtr->tsIpv4HeaderPtr)
    {
        totalLengthPtr = &haInfoPtr->tsIpv4HeaderPtr[2];
    }

    else if(haInfoPtr->tsIpv6HeaderPtr)
    {
        totalLengthPtr = &haInfoPtr->tsIpv6HeaderPtr[4];
    }
    else
    {
        return; /* nothing to update */
    }

    /* get current packet total length value */
    fldVal =  totalLengthPtr[0] << 8 |
              totalLengthPtr[1];

    /*add the extra length*/
    fldVal += extraLength;

    /*update the packet*/
    totalLengthPtr[0] = (GT_U8)(fldVal >> 8);
    totalLengthPtr[1] = (GT_U8) fldVal;

    __LOG(("Added extra passenger L2 bytes: %d, updated total length is %d \n", extraLength, fldVal));

    if(haInfoPtr->tsUdpHeaderPtr)
    {
        totalLengthPtr = &haInfoPtr->tsUdpHeaderPtr[4];

        /* get current length value */
        fldVal =  totalLengthPtr[0] << 8 |
                  totalLengthPtr[1];

        /*add the extra length*/
        fldVal += extraLength;

        /*update the packet*/
        totalLengthPtr[0] = (GT_U8)(fldVal >> 8);
        totalLengthPtr[1] = (GT_U8) fldVal;

        __LOG(("UDP Added extra passenger L2 bytes: %d, updated total length is %d \n", extraLength, fldVal));
    }



}

/**
* @internal snetChtHaCreateDsaTag function
* @endinternal
*
* @brief   HA - Create DSA tag
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in] egressPort               - the local egress port (not global port)
* @param[in] haInfoPtr                - pointer to internal ha info
*/
static void snetChtHaCreateDsaTag
(
    IN  SKERNEL_DEVICE_OBJECT           *devObjPtr,
    IN  SKERNEL_FRAME_CHEETAH_DESCR_STC *descrPtr,
    IN  GT_U32                           egressPort,
    IN  HA_INTERNAL_INFO_STC            *haInfoPtr,
    OUT GT_U8_PTR                        dsaOrVlanTag
)
{
    DECLARE_FUNC_NAME(snetChtHaCreateDsaTag);

    SNET_DSA_TAG_FIELDS_STC     dsaFields;    /*dsa tag fields that may differ from the descriptor values */
    SNET_DSA_TAG_FIELDS_STC     origDsaFields;/* original dsa tag fields that reflace the descriptor values */
    GT_BOOL     trgValuesRemapped = GT_FALSE;
    GT_U32      orig_outGoingMtagCmd = descrPtr->outGoingMtagCmd;

    /* set default values */
    dsaFields.useVidx            = descrPtr->useVidx;
    dsaFields.isTrgPhyPortValid  = descrPtr->eArchExtInfo.isTrgPhyPortValid;
    dsaFields.trgPhyPort         = devObjPtr->supportEArch ? descrPtr->eArchExtInfo.trgPhyPort : descrPtr->trgEPort;
    dsaFields.trgDev             = descrPtr->trgDev;
    dsaFields.trgEPort           = descrPtr->trgEPort;
    dsaFields.srcTrgDev          = descrPtr->srcTrgDev;
    dsaFields.srcTrgPhysicalPort = descrPtr->srcTrgPhysicalPort;

    /* save the orig info */
    origDsaFields = dsaFields;

    if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        trgValuesRemapped = snetLion3HaDSATrgInterfaceRemoteFieldsReAssignment(devObjPtr, descrPtr, egressPort, &dsaFields);

        if(GT_TRUE == trgValuesRemapped)
        {
            /* modify the descriptor with updated fields */
            descrPtr->useVidx                        = dsaFields.useVidx;
            descrPtr->eArchExtInfo.isTrgPhyPortValid = dsaFields.isTrgPhyPortValid;
            descrPtr->eArchExtInfo.trgPhyPort        = dsaFields.trgPhyPort;
            descrPtr->trgDev                         = dsaFields.trgDev;
            descrPtr->trgEPort                       = dsaFields.trgEPort;

            if((descrPtr->outGoingMtagCmd == SKERNEL_MTAG_CMD_TO_CPU_E && (descrPtr->srcTrg == 1)) ||
               (descrPtr->outGoingMtagCmd == SKERNEL_MTAG_CMD_TO_TRG_SNIFFER_E && (descrPtr->rxSniff == 0)) )
            {
                /* the 'srcTrg' used as 'target' so we need to update those fields too */
                dsaFields.srcTrgDev          = dsaFields.trgDev;
                dsaFields.srcTrgPhysicalPort = dsaFields.trgPhyPort;
            }
        }

        if (descrPtr->forceToAddFromCpu4BytesDsaTag)
        {
            /* Force the outgoing DSA tag to be 4B FROM_CPU, */
            descrPtr->egrMarvellTagType = MTAG_STANDARD_E;
            descrPtr->outGoingMtagCmd = SKERNEL_MTAG_CMD_FROM_CPU_E;
        }
    }

    __LOG_PARAM(dsaFields.useVidx);
    __LOG_PARAM(dsaFields.isTrgPhyPortValid);
    __LOG_PARAM(dsaFields.trgPhyPort);
    __LOG_PARAM(dsaFields.trgDev);
    __LOG_PARAM(dsaFields.trgEPort);
    if((descrPtr->outGoingMtagCmd == SKERNEL_MTAG_CMD_TO_CPU_E && (descrPtr->srcTrg == 1)) ||
       (descrPtr->outGoingMtagCmd == SKERNEL_MTAG_CMD_TO_TRG_SNIFFER_E && (descrPtr->rxSniff == 0)) )
    {
        __LOG_PARAM(dsaFields.srcTrgDev);
        __LOG_PARAM(dsaFields.srcTrgPhysicalPort);
    }

    __LOG(("build the needed DSA tag"));

    switch (descrPtr->outGoingMtagCmd)
    {
        case SKERNEL_MTAG_CMD_TO_CPU_E :
            snetChtHaCreateMarvellTagToCpu(devObjPtr, descrPtr, egressPort, dsaOrVlanTag);
            break;
        case SKERNEL_MTAG_CMD_FROM_CPU_E:
            snetChtHaCreateMarvellTagFromCpu(devObjPtr, descrPtr, dsaOrVlanTag);
            break;
        case SKERNEL_MTAG_CMD_TO_TRG_SNIFFER_E:
            snetChtHaCreateMarvellTagToAnalyzer(devObjPtr, descrPtr, dsaOrVlanTag);
            break;
        case SKERNEL_MTAG_CMD_FORWARD_E:
            snetChtHaCreateMarvellForward(devObjPtr, descrPtr, egressPort,
                                           haInfoPtr, dsaOrVlanTag);
            break;
        default:
            break;
    }

    /* restore command type , to build tr101 tags properly */
    descrPtr->outGoingMtagCmd = orig_outGoingMtagCmd;

    if(GT_TRUE == trgValuesRemapped)
    {
        /* restore the descriptor with original fields */
        descrPtr->useVidx                        = origDsaFields.useVidx;
        descrPtr->eArchExtInfo.isTrgPhyPortValid = origDsaFields.isTrgPhyPortValid;
        descrPtr->eArchExtInfo.trgPhyPort        = origDsaFields.trgPhyPort;
        descrPtr->trgDev                         = origDsaFields.trgDev;
        descrPtr->trgEPort                       = origDsaFields.trgEPort;

        descrPtr->srcTrgDev                      = origDsaFields.srcTrgDev;
        descrPtr->srcTrgPhysicalPort             = origDsaFields.srcTrgPhysicalPort;
    }
}

/**
* @internal snetLion3HaOamChannelTypeProfile function
* @endinternal
*
* @brief   HA OAM Channel Type Profile support
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] descrPtr                 - pointer to the frame descriptor.
*                                      OUTPUT:
* @param[in,out] descrPtr                 - pointer to the frame descriptor.
*/
static GT_VOID snetLion3HaOamChannelTypeProfile
(
    IN    SKERNEL_DEVICE_OBJECT             *devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC   *descrPtr
)
{
    DECLARE_FUNC_NAME(snetLion3HaOamChannelTypeProfile);

    GT_U32 firstNibbleAfterSBitLabel;           /* first nibble after sbit label */
    GT_U32 pwe3CwWord;                          /* pseudo wire control word */
    GT_U32 pwe3ChannelType;                     /* pseudo wire channel type */
    GT_U32 tableChannelType;                    /* pseudo wire channel type */
    GT_U32 index;                               /* table entry index */
    GT_U32 validBit;                            /* valid bit field  */
    GT_U32 offset;                              /* byte offset from the begging of packet */
    GT_U8 * afterMplsLabelsPtr;                 /* header offset after mpls lables */

    /*
        NOTE: for MPLS LSR use the indication from the ingress about the place of the end of the labels.
    */

    if(descrPtr->tsEgressMplsControlWordExist && (descrPtr->isMplsLsr == 0))
    {
        /* from TS of mpls with CW*/
        offset = 4*(descrPtr->tsEgressMplsNumOfLabels + 1);
        afterMplsLabelsPtr = &descrPtr->haToEpclInfo.tunnelStartL3StartOffsetPtr[offset];
    }
    else if(descrPtr->mpls) /* also include the case of TS for LSR */
    {
        /* MPLS packet from ingress pipe */
        afterMplsLabelsPtr = descrPtr->afterMplsLabelsPtr;
    }
    else
    {
        /* Not MPLS packet */
        return;
    }

    /* build the first word after the recognized MPLS labels */
    pwe3CwWord = SNET_BUILD_WORD_FROM_BYTES_PTR_MAC(afterMplsLabelsPtr);
    /* get the first nibble (in network order) that come after the recognized MPLS labels */
    firstNibbleAfterSBitLabel = SMEM_U32_GET_FIELD(pwe3CwWord, 28, 4);
    /* get <Channel Type> field - the 2 LSBs of the 4B MPLS control word */
    pwe3ChannelType = SMEM_U32_GET_FIELD(pwe3CwWord, 0, 16);

   /*
        PWE3_Channel_Type = 2 LSBs of 4B word following the MPLS header
                (PWE3 CW Control Word Format)

        IF (First nibble following the MPLS header = 4'b0001) // PWE3 word exists

        Desc<Channel Type Profile> = 0 // default if there is no match in the for below

        FOR EACH entry n in the <MPLS Channel Type Profile Entry<n>>

            IF <Valid <n>>==1 && <Channel Type <n>>==Packet<Channel Type>

                Desc<Channel Type Profile> = n+1 // Entry 0 is profile 1 etc.

        END-FOR

    -- Figure 356: Associated Channel Header (ACH)
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
        if(index == 14)
        {
            /* get table channel type */
            smemRegFldGet(devObjPtr, SMEM_LION3_HA_MPLS_CHANNEL_TYPE_PROFILE7_REG(devObjPtr), 0, 16, &tableChannelType);
        }
        else
        {
            /* get table channel type */
            smemRegFldGet(devObjPtr, SMEM_LION3_HA_MPLS_CHANNEL_TYPE_PROFILE_REG(devObjPtr, index / 2), (index % 2) * 16, 16, &tableChannelType);
        }

        if(tableChannelType != pwe3ChannelType)
        {
            continue;
        }

        /* check valid bit first */
        smemRegFldGet(devObjPtr, SMEM_LION3_HA_MPLS_CHANNEL_TYPE_PROFILE7_REG(devObjPtr), 16 + index, 1, &validBit);

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
* @internal snetChtHaPtpZeroResevedField function
* @endinternal
*
* @brief   zero reserved field in PTP header
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in] egressPort               - egress port.
* @param[in] ptpPtr                   - pointer to PTP header
*/
static void  snetChtHaPtpZeroResevedField
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32  egressPort,
    IN GT_U8 * ptpPtr
)
{
    DECLARE_FUNC_NAME(snetChtHaPtpZeroResevedField);

    GT_U8    *ptpStart; /* pointer to start of timestamp related packet */

    /* When a packet come in with a piggy TST over the reserved field, but is going out without -
       the reserved field should be explicitly overridden to 0x0 */
     if ((descrPtr->timestampTagged[SMAIN_DIRECTION_INGRESS_E] == SKERNEL_TIMESTAMP_TAG_TYPE_PIGGY_BACKED_E ||
          descrPtr->timestampTagged[SMAIN_DIRECTION_INGRESS_E] == SKERNEL_TIMESTAMP_TAG_TYPE_HYBRID_E) &&
         (descrPtr->timestampTagged[SMAIN_DIRECTION_EGRESS_E] != SKERNEL_TIMESTAMP_TAG_TYPE_PIGGY_BACKED_E &&
          descrPtr->timestampTagged[SMAIN_DIRECTION_EGRESS_E] != SKERNEL_TIMESTAMP_TAG_TYPE_HYBRID_E) &&
         descrPtr->timestampActionInfo.packetFormat == SKERNEL_TIMESTAMP_PACKET_FORMAT_PTPV2_E && (egressPort != SNET_CHT_NULL_PORT_CNS))
     {
         __LOG_PARAM(descrPtr->timestampTagged[SMAIN_DIRECTION_INGRESS_E]);
         __LOG_PARAM(descrPtr->timestampTagged[SMAIN_DIRECTION_EGRESS_E]);

         /* PTP timestamping based on Local Action Table */
        ptpStart = ptpPtr;

         __LOG(("Dump PTP header BEFORE [20] bytes \n"));
         PTP_HEADER_LOG(ptpPtr,20);

        ptpStart += 16 /* reserved field */;
        *(ptpStart++) = 0;
        *(ptpStart++) = 0;
        *(ptpStart++) = 0;
        *(ptpStart)   = 0;

        __LOG(("Reserved field in PTP header was set to '0'\n"));

        __LOG(("Dump PTP header AFTER [20] bytes \n"));
        PTP_HEADER_LOG(ptpPtr,20);
    }

     return;
}

/**
* @internal haBuildPacketNonChanged function
* @endinternal
*
* @brief   build packet that is not changed by the HA
*
* @param[in] devObjPtr                -  pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in] egressBufferPtr          - the egress buffer
*/
static void  haBuildPacketNonChanged
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32  egressPort,
    IN GT_U8   *egressBufferPtr,
    OUT GT_U8 **frameDataPtrPtr,
    OUT GT_U32 *frameDataSize
)
{
    DECLARE_FUNC_NAME(haBuildPacketNonChanged);

    GT_U8    prePendArr[2] = {0};
    GT_U32   offset;
    GT_U32   realByteCount;
    GT_U8    *ptpStart; /* pointer to start of timestamp related packet */

    /* packet not changed , using original frame */
    __LOG(("PACKET NOT CHANGED, USING ORIGINAL FRAME!"));

    /* set prepend bytes (if needed) */
    if(descrPtr->haToEpclInfo.prePendLength)
    {
        MEM_APPEND_WITH_LOG(egressBufferPtr,prePendArr,descrPtr->haToEpclInfo.prePendLength);
    }

    /* save info for the EPCL */
    __LOG(("save info for the EPCL"));
    offset = 0;
    SET_FORCE_MAC(
        descrPtr->haToEpclInfo.macDaSaPtr,egressBufferPtr + offset);

    if(descrPtr->afterVlanOrDsaTagPtr != (12 + descrPtr->startFramePtr))
    {
        offset = 12;
        SET_FORCE_MAC(
            descrPtr->haToEpclInfo.outerVlanTagPtr,egressBufferPtr + offset);
    }

    if(descrPtr->tag0Ptr)
    {
        offset = (descrPtr->tag0Ptr - descrPtr->startFramePtr);
        SET_FORCE_MAC(
            descrPtr->haToEpclInfo.vlanTag0Ptr,egressBufferPtr + offset);
    }

    if(descrPtr->tag1Ptr)
    {
        offset = (descrPtr->tag1Ptr - descrPtr->startFramePtr);
        SET_FORCE_MAC(
            descrPtr->haToEpclInfo.vlanTag1Ptr,egressBufferPtr + offset);
    }

    offset = descrPtr->payloadPtr - descrPtr->startFramePtr;
    SET_FORCE_MAC(
        descrPtr->haToEpclInfo.l3StartOffsetPtr,egressBufferPtr + offset );

    realByteCount = descrPtr->byteCount;
    if (descrPtr->cutThroughModeEnabled
        && (descrPtr->cutThrough2StoreAndForward || descrPtr->cutThroughModeTerminated))
    {
        realByteCount      = descrPtr->frameBuf->actualDataSize;
        __LOG(
            ("Use original frame size [%d] as Byte Count due to Cut Through termination",
             realByteCount));
    }

    MEM_APPEND_WITH_LOG(egressBufferPtr,descrPtr->startFramePtr, realByteCount);

    /* PTP timestamping based on Local Action Table */
    ptpStart = descrPtr->haToEpclInfo.l3StartOffsetPtr +
               (descrPtr->ptpGtsInfo.ptpMessageHeaderPtr - descrPtr->payloadPtr);

    /* zero reserved field in PTP header */
    snetChtHaPtpZeroResevedField(devObjPtr,descrPtr,egressPort,ptpStart);

    *frameDataSize   = realByteCount + descrPtr->haToEpclInfo.prePendLength;
    *frameDataPtrPtr = devObjPtr->egressBuffer;

    return;
}

/**
* @internal haBuildPacketWithOrigInfoFromIngressToCpuOrFromIngressToAnalyzer function
* @endinternal
*
* @brief   build packet that is not changed by the HA
*         (descrPtr->outGoingMtagCmd == SKERNEL_MTAG_CMD_TO_CPU_E && descrPtr->srcTrg == 0)-->Src ||
*         (descrPtr->outGoingMtagCmd == SKERNEL_MTAG_CMD_TO_TRG_SNIFFER_E && descrPtr->rxSniff)
* @param[in] devObjPtr                -  pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in] egressBufferPtr          - the egress buffer
* @param[in] haInfoPtr                - pointer to HA info
*
* @note Ha unit add timestamp tag to the original packet.
*       the timestamp tag is after the DSA tag and before the vlan tag.
*
*/
static void  haBuildPacketWithOrigInfoFromIngressToCpuOrFromIngressToAnalyzer
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U8    *egressBufferPtr,
    IN GT_U32   dsaOrVlanTagLength,
    IN GT_U8    *dsaOrVlanTag,
    IN GT_U32   vlanEtherType,
    IN GT_U8    *timestampTag,          /* timestamp tag */
    IN GT_U32   timestampTagLength,   /* up to 16 bytes */
    OUT GT_U8 **frameDataPtrPtr,
    OUT GT_U32 *frameDataSize,
    IN HA_INTERNAL_INFO_STC *haInfoPtr
)
{
    DECLARE_FUNC_NAME(haBuildPacketWithOrigInfoFromIngressToCpuOrFromIngressToAnalyzer);

    GT_U32  macDaSaLength=12;
    GT_U32   extractFromDSADueToForceNewDsaToCpuTagLength = 0;/* the <Force New Dsa To_Cpu> will add new tag for ingress DSA with <srcTagged> */
    GT_U8    extractFromDSADueToForceNewDsaToCpuTag[4];
    GT_U32  ingressL2Len;
    GT_U32  ingressL2FirstTagLen;
    GT_U32  egressL2Len;
    GT_U32  offset;
    GT_U8   prePendArr[2] = {0};
    GT_U8   *payloadPtr;
    GT_U32  payloadLen;
    GT_U32  totalLength;
    GT_U32  newDsaToCpuTagLength = 0;/* offset to tag 0 */
    GT_BIT  tag0Outer = 0;/* indication that tag 0 is outer , */
    GT_BIT  tag1Outer = 0;/* indication that tag 1 is outer , */
    GT_U32  ingressEvbBpeTagsLen = 0;
    GT_U32  egressEvbBpeTagsLen  = 0;
    GT_U32  skippedDsaTagLen     = 0;

    __LOG_PARAM(descrPtr->egrMarvellTagType );
    if (descrPtr->egrMarvellTagType != MTAG_TYPE_NONE_E && descrPtr->forceToAddFromCpu4BytesDsaTag)
    {
        /* Forced FROM_CPU DSA tag doesn't replace outer vlan tag but precedes it. So
           vlan tags pointers will be shifted over DSA tag to point vlan tags */
        skippedDsaTagLen = (descrPtr->egrMarvellTagType + 1) * 4;

        if (dsaOrVlanTagLength == skippedDsaTagLen)
        {
            /* don't skip if packet is not vlan tagged */
            skippedDsaTagLen = 0;
        }
    }
    /* set prepend bytes (if needed) */
    if(descrPtr->haToEpclInfo.prePendLength)
    {
        MEM_APPEND_WITH_LOG(egressBufferPtr,prePendArr,descrPtr->haToEpclInfo.prePendLength);
    }

    if(descrPtr->numOfBytesToPop)
    {
        if(descrPtr->tunnelTerminated)
        {
            __LOG(("The 'Copy' to cpu/analyzer ignore the <numOfBytesToPop>[%d] that associated with the 'passenger' \n",
                descrPtr->numOfBytesToPop));
        }
        else
        {
            /* the ingress tags are 'popped' and will not appear in the 'copy' !!! */
            ingressEvbBpeTagsLen = descrPtr->numOfBytesToPop;
            __LOG_PARAM(ingressEvbBpeTagsLen);
        }
    }

    /* save info for the EPCL */
    offset = 0;
    SET_FORCE_MAC(
        descrPtr->haToEpclInfo.macDaSaPtr,egressBufferPtr + offset);

    offset = macDaSaLength;

    /* function is called for NON TS packet. Need to ignore haInfoPtr->evbBpeIsPassenger. */
    if(haInfoPtr->evbBpeTagLength)
    {
        egressEvbBpeTagsLen = haInfoPtr->evbBpeTagLength;
        __LOG_PARAM(egressEvbBpeTagsLen);
    }

    if (descrPtr->egrMarvellTagType == MTAG_TYPE_NONE_E)
    {
        /* the vlan tags comes after the EVB/PBE tag. */
        offset += egressEvbBpeTagsLen;
    }

    if(dsaOrVlanTagLength)
    {
        SET_FORCE_MAC(
            descrPtr->haToEpclInfo.outerVlanTagPtr,egressBufferPtr + offset + skippedDsaTagLen);
    }
    if (skippedDsaTagLen)
    {
        SET_FORCE_MAC(
            descrPtr->haToEpclInfo.outerVlanTagPtr,egressBufferPtr + offset);
    }

    descrPtr->tag0Ptr = NULL;
    descrPtr->tag1Ptr = NULL;

    if(descrPtr->marvellTagged)
    {
        ingressL2FirstTagLen = 4 * (descrPtr->marvellTaggedExtended + 1);

        if(descrPtr->forceNewDsaToCpu && descrPtr->origSrcTagged)
        {
            /* need to build vlan tag outside the DSA !

              meaning that the vid from the src DSA will be in new vlan TAG
            */
            extractFromDSADueToForceNewDsaToCpuTagLength = 4;
        }
    }
    else
    {
        if(descrPtr->origSrcTagState == SKERNEL_FRAME_TR101_VLAN_INGR_TAG_UNTAGGED_E ||
                descrPtr->forceNewDsaToCpu || descrPtr->nestedVlanAccessPort)
        {
            if((descrPtr->forceNewDsaToCpu == 0) && descrPtr->nestedVlanAccessPort &&
                descrPtr->tagSrcTagged[SNET_CHT_TAG_0_INDEX_CNS] &&
                SMEM_CHT_IS_SIP5_GET(devObjPtr))
            {
                ingressL2FirstTagLen = 4;
            }
            else
            {
                ingressL2FirstTagLen = 0;
            }
        }
        else
        {
            /* packet came with at least one tag */
            ingressL2FirstTagLen = 4;

            if(devObjPtr->errata.ttCopyToCpuWithAdditionalTag &&  descrPtr->rxSniff == 1 &&
               descrPtr->tunnelTerminated)
            {
                /* allow the outer tag to not be recognized , so packet to CPU
                   will be with DSA and the most outer tag (and the other tags...)*/
                ingressL2FirstTagLen = 0;
            }
        }
    }

    if(descrPtr->forceNewDsaToCpu && descrPtr->origSrcTagged &&
       descrPtr->egrMarvellTagType != MTAG_TYPE_NONE_E)
    {
        /* packet came vlan tagged and need to add DSA .. consider the tag0
           to be outside the DSA */
        newDsaToCpuTagLength = dsaOrVlanTagLength;
    }

    /* payload is after the first vlan/DSA tag (and after ingress TS tag) */
    payloadPtr = descrPtr->origInfoBeforeTunnelTermination.originalL2Ptr + macDaSaLength + ingressL2FirstTagLen +
            ingressEvbBpeTagsLen + descrPtr->ingressTimestampTagSize;

    switch(descrPtr->origSrcTagState)
    {
        default:
        case SKERNEL_FRAME_TR101_VLAN_INGR_TAG_UNTAGGED_E:
            break;
        case SKERNEL_FRAME_TR101_VLAN_INGR_TAG_TAG0_E:
            descrPtr->tag0Ptr = descrPtr->origInfoBeforeTunnelTermination.originalL2Ptr + macDaSaLength;
            break;
        case SKERNEL_FRAME_TR101_VLAN_INGR_TAG_TAG1_E:
            descrPtr->tag1Ptr = descrPtr->origInfoBeforeTunnelTermination.originalL2Ptr + macDaSaLength;
            break;
        case SKERNEL_FRAME_TR101_VLAN_INGR_TAG_OUT_TAG0_IN_TAG1_E:
            descrPtr->tag0Ptr = descrPtr->origInfoBeforeTunnelTermination.originalL2Ptr + macDaSaLength;
            descrPtr->tag1Ptr = descrPtr->tag0Ptr + 4;
            break;
        case SKERNEL_FRAME_TR101_VLAN_INGR_TAG_OUT_TAG1_IN_TAG0_E:
            descrPtr->tag1Ptr = descrPtr->origInfoBeforeTunnelTermination.originalL2Ptr + macDaSaLength;
            descrPtr->tag0Ptr = descrPtr->tag1Ptr + 4;
            break;
    }

    if(descrPtr->tag0Ptr > descrPtr->tag1Ptr)/* check that tag0 is outer */
    {
        tag0Outer = 1;
    }

    if(descrPtr->tag1Ptr > descrPtr->tag0Ptr)/* check that tag1 is outer */
    {
        tag1Outer = 1;
    }

    if(descrPtr->tag0Ptr)
    {
        if(tag0Outer)
        {
            /* the tag0 will jump over the DSA */
            descrPtr->tag0Ptr += newDsaToCpuTagLength;
        }

        offset = (descrPtr->tag0Ptr - descrPtr->origInfoBeforeTunnelTermination.originalL2Ptr);

        SET_FORCE_MAC(
            descrPtr->haToEpclInfo.vlanTag0Ptr,egressBufferPtr + offset + skippedDsaTagLen);
    }

    if(descrPtr->tag1Ptr)
    {
        if(tag1Outer)
        {
            /* the tag0 will jump over the DSA */
            descrPtr->tag1Ptr += newDsaToCpuTagLength;
        }

        offset = (descrPtr->tag1Ptr - descrPtr->origInfoBeforeTunnelTermination.originalL2Ptr);
        SET_FORCE_MAC(
            descrPtr->haToEpclInfo.vlanTag1Ptr,egressBufferPtr + offset + skippedDsaTagLen);
    }

    if (SMEM_CHT_IS_SIP6_GET(devObjPtr) && descrPtr->cutThroughModeEnabled)
    {
        /* descrPtr->frameBuf->actualDataSize replaced by origByteCount */
        payloadLen = descrPtr->origByteCount
            - (payloadPtr - descrPtr->origInfoBeforeTunnelTermination.originalL2Ptr);
    }
    else
    {
        payloadLen = descrPtr->frameBuf->actualDataSize
            - (payloadPtr - descrPtr->origInfoBeforeTunnelTermination.originalL2Ptr);
    }

    /* calculate the position of the L3 in the egress buffer .. for EPCL */

    /* the length of the ingress vlan/DSA tags + ethertype */
    ingressL2Len = (descrPtr->origInfoBeforeTunnelTermination.originalL3Ptr - descrPtr->origInfoBeforeTunnelTermination.originalL2Ptr) - macDaSaLength;

    /* the length of the egress vlan/DSA tags + ethertype */
    egressL2Len = (ingressL2Len - ingressL2FirstTagLen) + dsaOrVlanTagLength;

    /* the egress will hold 4 bytes that where embedded in the DSA of the ingress */
    egressL2Len += extractFromDSADueToForceNewDsaToCpuTagLength;

    /* the egress add it's own the EVB/BPE tag */
    egressL2Len += egressEvbBpeTagsLen;

    /* in case of egress TimeStamp*/
    egressL2Len += timestampTagLength;

    /* this pointer is to the point after the etherType */
    __LOG_PARAM(egressL2Len);
    __LOG_PARAM(macDaSaLength);
    offset = egressL2Len + macDaSaLength;

    SET_FORCE_MAC(
        descrPtr->haToEpclInfo.l3StartOffsetPtr,egressBufferPtr + offset );

    /* mac SA ,DA */
    MEM_APPEND_WITH_LOG(egressBufferPtr,descrPtr->origInfoBeforeTunnelTermination.originalL2Ptr,macDaSaLength);


    if(egressEvbBpeTagsLen && (descrPtr->egrMarvellTagType == MTAG_TYPE_NONE_E))
    {
        /* egress EVB/BPE tag (before other vlan tags) */
        MEM_APPEND_WITH_LOG(egressBufferPtr,haInfoPtr->evbBpeTagPtr,egressEvbBpeTagsLen);
    }
    __LOG_PARAM(timestampTagLength);
    __LOG_PARAM(skippedDsaTagLen);

    if (descrPtr->egrMarvellTagType != MTAG_TYPE_NONE_E &&
        timestampTagLength &&
        (GT_U32)((descrPtr->egrMarvellTagType + 1) * 4) < dsaOrVlanTagLength)
    {
        /* DSA + TS Tag + Vlan tag */
        /* need to put TS tag after DSA */
        GT_U32 dsaLen = ((descrPtr->egrMarvellTagType + 1) * 4);
        /* DSA tag */
        MEM_APPEND_WITH_LOG(egressBufferPtr,dsaOrVlanTag,dsaLen);

        if(egressEvbBpeTagsLen)
        {
            /* egress EVB/BPE tag after DSA */
            MEM_APPEND_WITH_LOG(egressBufferPtr,haInfoPtr->evbBpeTagPtr,egressEvbBpeTagsLen);
        }

        /* TS tag */
        SET_FORCE_MAC(
            descrPtr->haToEpclInfo.timeStampTagPtr, egressBufferPtr);
        MEM_APPEND_WITH_LOG(egressBufferPtr, timestampTag, timestampTagLength);

        /* vlan tag */
        MEM_APPEND_WITH_LOG(egressBufferPtr,dsaOrVlanTag+dsaLen,dsaOrVlanTagLength-dsaLen);
    }
    else
    {
        /* vlan/DSA tag */
        MEM_APPEND_WITH_LOG(egressBufferPtr,dsaOrVlanTag,dsaOrVlanTagLength);

        if(egressEvbBpeTagsLen && (descrPtr->egrMarvellTagType != MTAG_TYPE_NONE_E))
        {
            /* egress EVB/BPE tag after DSA */
            MEM_APPEND_WITH_LOG(egressBufferPtr,haInfoPtr->evbBpeTagPtr,egressEvbBpeTagsLen);
        }

        /*the timestemp come before vlan tag and after mac */
        if(timestampTagLength)
        {
            /* If the packet is untagged and non-DSA tagged, then the Timestamp Tag is after the MACs */
            SET_FORCE_MAC(
                descrPtr->haToEpclInfo.timeStampTagPtr, egressBufferPtr);
            MEM_APPEND_WITH_LOG(egressBufferPtr, timestampTag, timestampTagLength);
        }
    }

    if(extractFromDSADueToForceNewDsaToCpuTagLength)
    {
        snetChtHaEgressTagDataExtGet((GT_U8)descrPtr->vlanTag802dot1dInfo.vpt,
                  (GT_U16)descrPtr->vlanTag802dot1dInfo.vid,
                  (GT_U8)descrPtr->vlanTag802dot1dInfo.cfi,
                (GT_U16)vlanEtherType,
                extractFromDSADueToForceNewDsaToCpuTag);

        MEM_APPEND_WITH_LOG(egressBufferPtr,
                extractFromDSADueToForceNewDsaToCpuTag,
                extractFromDSADueToForceNewDsaToCpuTagLength);
    }



    /* other info */
    /* check total size first - overflow issue */
    if((payloadLen + dsaOrVlanTagLength + macDaSaLength + timestampTagLength +
        extractFromDSADueToForceNewDsaToCpuTagLength +
        descrPtr->haToEpclInfo.prePendLength) > SBUF_DATA_SIZE_CNS)
    {
        skernelFatalError("haBuildPacketWithOrigInfoFromIngressToCpuOrFromIngressToAnalyzer: overflow packet size, must be < %d,\n", SBUF_DATA_SIZE_CNS);
    }

    MEM_APPEND_WITH_LOG(egressBufferPtr,payloadPtr,payloadLen);

    *frameDataSize   = egressBufferPtr - devObjPtr->egressBuffer;
    *frameDataPtrPtr = devObjPtr->egressBuffer;

    totalLength = *frameDataSize;

    if(totalLength < SGT_MIN_FRAME_LEN)
    {
        /* NOTE: this case is probably not relevant as those 'to cpu'/'to analyzer' as
           sending the 'original packet' that was with bytecount >= 64 */
        /* actual padding done in the mac */
        __LOG(("use minimal egress packet size with padding to 60 bytes (+4 bytes for CRC) \n"));

        totalLength = SGT_MIN_FRAME_LEN;
    }
    else
    if (descrPtr->outGoingMtagCmd == SKERNEL_MTAG_CMD_TO_CPU_E)
    {
        /* check if the frame needs to be truncated */
        if ( (descrPtr->truncated) && (totalLength > 128) )
        {
            __LOG(("frame TO_CPU needs to be truncated --> to 128 bytes (was [%d] bytes)",
                totalLength));

            totalLength = 128;
        }
        else
        {
            __LOG(("frame TO_CPU was not truncated and keeps [%d] bytes (include the CRC) \n",
                totalLength));
            descrPtr->truncated = 0;/* is was not truncated */
        }
    }
    else
    if (descrPtr->outGoingMtagCmd == SKERNEL_MTAG_CMD_TO_TRG_SNIFFER_E ||
        descrPtr->rxSniff)
    {
        /* check if the frame needs to be truncated */
        if ( (descrPtr->truncated) && (totalLength > 128) )
        {
            __LOG(("frame TO_ANALYZER needs to be truncated --> to 128 bytes (was [%d] bytes)",
                totalLength));

            totalLength = 128;
        }
        else
        {
            __LOG(("frame TO_ANALYZER was not truncated and keeps [%d] bytes (include the CRC) \n",
                totalLength));
            descrPtr->truncated = 0;/* is was not truncated */
        }
    }

    *frameDataSize = totalLength;

}


/**
* @internal chtHaIpHeaderModifications function
* @endinternal
*
* @brief   HA - do ipv4/6 header modifications (tunnel start or passenger)
*
* @param[in] devObjPtr                -  pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in] egressPort               - egress port (local port on multi-port group device)
*                                      CPU port is port 63
* @param[in] doTunnelHeader           - indication to do tunnel start header modifications
* @param[in] l3HeaderChangedBuffer    - pointer to ipv4/6 header
*                                      relevant only when doTunnelHeader = GT_FALSE
* @param[in] haInfoPtr                - pointer to HA info
*                                      relevant only when doTunnelHeader = GT_TRUE
* @param[in] l4HeaderChangedBuffer    - pointer to TCP/UDP header
*                                      relevant only when not NULL and l4HeaderChangedLen != 0
* @param[in] l4HeaderChangedLen       - number of bytes in l4HeaderChangedBuffer
* @param[in] isL2Nat                  - flag to indicate whether L2NAT functionality
*                                       is enabled/disabled
*/
static void  chtHaIpHeaderModifications
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32   egressPort,
    IN GT_BOOL  doTunnelHeader,
    GT_U8    *l3HeaderChangedBuffer/*[40]*/,
    HA_INTERNAL_INFO_STC    *haInfoPtr,
    GT_U8    *l4HeaderChangedBuffer/*[18]*/,
    GT_U32   l4HeaderChangedLen,
    GT_BOOL  isL2Nat
)
{
    DECLARE_FUNC_NAME(chtHaIpHeaderModifications);

    GT_BOOL  recalcIpv4CheckSumNeeded = GT_FALSE;/* flag to state if recalc of ipv4 checksum needed */
    GT_BIT   allowIpHeaderModification;/* allow modifications in the IP header */
    GT_U32   ipCheckSum;/* ipv4 check sum value */

    if(doTunnelHeader == GT_FALSE) /* update passenger or Non-TS ip */
    {
        __LOG(("Check for IPv4/6 header modification(DSCP,TTL,checksum) (for NON tunnel start / passenger) \n"));

        if(SMEM_CHT_IS_SIP5_GET(devObjPtr) &&
            descrPtr->ipHeaderError)
        {
            allowIpHeaderModification = 0;
        }
        else
        {
            allowIpHeaderModification = 1;
        }

        __LOG_PARAM(allowIpHeaderModification);

        if(allowIpHeaderModification)
        {
            if (descrPtr->isNat || isL2Nat)
            {
                if(descrPtr->isIPv4)
                {
                    __LOG(("NAT enabled: indication to recalculate ipv4 (l3) checksum \n"));
                    recalcIpv4CheckSumNeeded = GT_TRUE;

                    __LOG(("NAT-44 : Do L3,L4 header modifications (SIP,DIP,sPort,dPort,L4-checksum) \n"));
                    haProcessNat44L3L4HeaderModify(devObjPtr,descrPtr,
                        l3HeaderChangedBuffer,
                        l4HeaderChangedBuffer,l4HeaderChangedLen,isL2Nat);
                }
                else
                {
                    __LOG(("NAT-66 : Do L3 header modifications (SIP or DIP) \n"));
                    haProcessNat66L3HeaderModify(devObjPtr,descrPtr,
                        l3HeaderChangedBuffer);
                }
            }

            /*update dscp if remark*/
            if (descrPtr->modifyDscp)
            {
                __LOG(("modify Dscp to [%d]\n",
                    descrPtr->dscp));
                if(descrPtr->isIPv4 == 1)
                {
                    recalcIpv4CheckSumNeeded = GT_TRUE;
                    l3HeaderChangedBuffer[1] = (GT_U8)(descrPtr->dscp << 2);
                }
                else  /*ipv6*/
                {
                    /* set 4 MSB of DSCP to byte 0 LSB */
                    l3HeaderChangedBuffer[0] &= 0xF0;
                    l3HeaderChangedBuffer[0] |= ((GT_U8)(descrPtr->dscp >> 2));

                    /* set 2 LSB of DSCP to byte 1 MSB */
                    l3HeaderChangedBuffer[1] &= 0x3F;
                    l3HeaderChangedBuffer[1] |= ((GT_U8)((descrPtr->dscp & 0x3) << 6));
                }
            }
            else
            {
                __LOG(("modify Dscp is not needed \n"));
            }

            if(descrPtr->copyTtl)
            {
                __LOG(("exctart TTL from tunnel header [%d]\n",descrPtr->ttl));
                l3HeaderChangedBuffer[descrPtr->isIPv4 ? 8 : 7] = (GT_U8)(descrPtr->ttl);
            }

            /*update ttl if needed*/
            if(descrPtr->decTtl == GT_TRUE && (descrPtr->ttl != 0))
            {
                __LOG(("decrement TTL to [%d]\n",
                    descrPtr->ttl - 1));

                if(descrPtr->isIPv4 == 1)
                {
                    recalcIpv4CheckSumNeeded = GT_TRUE;
                }
                l3HeaderChangedBuffer[descrPtr->isIPv4 ? 8 : 7] = (GT_U8)(descrPtr->ttl - 1);
            }
            else
            {
                if(descrPtr->decTtl == GT_TRUE)
                {
                    __LOG(("WARNING : decrement TTL required but TTL was already 0 ! (so TTL not modified) \n"));
                }
                else
                {
                    __LOG(("decrement TTL is not required \n"));
                }
            }
        }
        else/* only in sip 5*/
        {
            __LOG(("WARNING : Due to descrPtr->ipHeaderError = 1 , the modification of IP header is not allowed \n"));
            if (descrPtr->modifyDscp)
            {
                __LOG(("Although descrPtr->modifyDscp == 1 --> DSCP not modified \n"));
            }

            if(descrPtr->decTtl == GT_TRUE && (descrPtr->ttl != 0))
            {
                __LOG(("Although descrPtr->decTtl == GT_TRUE && (descrPtr->ttl != 0) --> TTL not modified \n"));
            }
        }

        /* The packets IPv4 header checksum field is updated to reflect
           changes in the TTL and/or DSCP. */
        if(recalcIpv4CheckSumNeeded) /* in case of TS this is the passenger IPV4 header */
        {
#define MAX_IP_HEADER_SIZE_CNS  60
            const GT_U16 checksumNumBytes = (GT_U16)((l3HeaderChangedBuffer[0] & 0xF) * 4);/*take from egress packet*/
            const GT_U32 l3HeaderChangedBufferLen = 20;
            static GT_U8 ipv4l3HeaderChecksumBuffer[MAX_IP_HEADER_SIZE_CNS];

            /* set check sum to 0 prior to re-calculation */
            l3HeaderChangedBuffer[10] = 0;
            l3HeaderChangedBuffer[11] = 0;

            memcpy(ipv4l3HeaderChecksumBuffer, l3HeaderChangedBuffer, l3HeaderChangedBufferLen);
            if(checksumNumBytes > MAX_IP_HEADER_SIZE_CNS)
            {
                skernelFatalError("need to modify define of max header \n");
            }
            else
            if(checksumNumBytes > l3HeaderChangedBufferLen)
            {
                /* append ipv4 options data to checksum */
                memcpy(ipv4l3HeaderChecksumBuffer+l3HeaderChangedBufferLen,
                       descrPtr->payloadPtr+l3HeaderChangedBufferLen,
                       checksumNumBytes -l3HeaderChangedBufferLen);
            }

            /* calculate IP packet checksum */
            ipCheckSum = ipV4CheckSumCalc(ipv4l3HeaderChecksumBuffer, checksumNumBytes);

            /* checksum of the IP header and IP options */
            l3HeaderChangedBuffer[10] = (GT_U8)(ipCheckSum >> 8);
            l3HeaderChangedBuffer[11] = (GT_U8)(ipCheckSum);

            __LOG(("the calculated ipv4 checksum is [0x%4.4x] \n",
                ipCheckSum));
        }
    }
    else
    if(haInfoPtr->tunnelStart && haInfoPtr->tsIpv4HeaderPtr)/* in case of TS this is the TS IPV4 header */
    {
        /* set check sum to 0 prior to re-calculation */
        haInfoPtr->tsIpv4HeaderPtr[10] = 0;
        haInfoPtr->tsIpv4HeaderPtr[11] = 0;

        /* calculate IP packet checksum of the TS header */
        ipCheckSum = ipV4CheckSumCalc(haInfoPtr->tsIpv4HeaderPtr,
                                     (GT_U16)((haInfoPtr->tsIpv4HeaderPtr[0] & 0xF) * 4));
        /* checksum of the IP header and IP options */
        haInfoPtr->tsIpv4HeaderPtr[10] = (GT_U8)(ipCheckSum >> 8);
        haInfoPtr->tsIpv4HeaderPtr[11] = (GT_U8)(ipCheckSum);

        __LOG(("TS Ipv4 header : calculated ipv4 checksum is [0x%4.4x] \n",
            ipCheckSum));
    }
}


/**
* @internal snetHaEgressVlanTag1Removed function
* @endinternal
*
* @brief   HA - update tag state to support Remove Vlan Tag1 If Zero
*
* @param[in] devObjPtr                - pointer to device object
* @param[in] descrPtr                 - pointer to the frame's descriptor.
*                                      vlanTag0EtherType - VLAN Tag0 EtherType
*                                      vlanTag1EtherType - VLAN Tag1 EtherType
*                                      destVlanTagged  - current Tag state.
*                                      OUTPUT:
*                                      updatedDestVlanTaggedPtr  - new tag state
*                                      RETURN:
*                                      none
*
* @note This feature enables Tag1 removal from the egress port Tag State
*       if Tag1 VID is assigned a value of 0.
*
*/
static void snetHaEgressVlanTag1Removed
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr
)
{
    DECLARE_FUNC_NAME(snetHaEgressVlanTag1Removed);

    GT_U32 regAddress;
    GT_U32 * regPtr;
    GT_U32 enableBit;
    GT_U32 doubleTagBit;

    if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        /* the logic was already in the EGF_QAG unit .
        see snetChtEgfQagVlanTag1Removed(...) */
        return ;
    }

    /* Supported only in XCat A2 and above */
    if (devObjPtr->supportRemoveVlanTag1WhenEmpty == 0)
    {
        return ;
    }

    /* The Tag1 Vlan != 0 */
    if(descrPtr->vid1 != 0)
    {
        __LOG(("remove tag 1 : not relevant when descrPtr->vid1 != 0 \n"));
        return ;
    }

    if(devObjPtr->vplsModeEnable.ha &&
       descrPtr->vplsInfo.egressTagStateAssigned)
    {
        /* the device logic was added to the HA registers ! (not only to the TXQ) */
        regAddress = SMEM_CHT3_ROUTE_HA_GLB_CNF_REG(devObjPtr);
        enableBit = 18;
        doubleTagBit = 17;
    }
    else
    {
        /* Lion B1 and above devices */
        if(devObjPtr->txqRevision)
        {
            if(SKERNEL_DEVICE_FAMILY_LION2_PORT_GROUP_DEV(devObjPtr))
            {
                regAddress = SMEM_LION_TXQ_DISTR_GENERAL_CONF_REG(devObjPtr);
                enableBit = 13;
                doubleTagBit = 14;
            }
            else
            {
                /* DQ Metal Fix Register */
                regAddress = SMEM_LION_TXQ_DQ_METAL_FIX_REG(devObjPtr);
                enableBit = 23;
                doubleTagBit = 22;
            }
        }
        else
        {
            /* Transmit Queue Extended Control1 */
            regAddress = SMEM_XCAT_TXQ_EXTENDED_CONTROL1_REG(devObjPtr);
            enableBit = 6;
            doubleTagBit = 7;
        }
    }

    regPtr = smemMemGet(devObjPtr, regAddress);

    /* Remove VLAN Tag1 If Zero - disabled */
    if(SMEM_U32_GET_FIELD(*regPtr, enableBit, 1) == 0)
    {
        __LOG(("remove tag 1 : globally disabled \n"));
        return ;
    }

    /* Packet was ingressed with two tags */
    if(descrPtr->srcTagState ==
            SKERNEL_FRAME_TR101_VLAN_INGR_TAG_OUT_TAG0_IN_TAG1_E ||
       descrPtr->srcTagState ==
            SKERNEL_FRAME_TR101_VLAN_INGR_TAG_OUT_TAG1_IN_TAG0_E)
    {
        /* Check if Tag1 with VID 0 removal is enabled for double tagged packets */
        if(SMEM_U32_GET_FIELD(*regPtr, doubleTagBit, 1) == 0)
        {
            __LOG(("remove tag 1 : not allowed (by global config) when ingress with 2 tags \n"));
            return ;
        }
    }

    switch(descrPtr->passangerTr101EgressVlanTagMode)
    {
        case SKERNEL_FRAME_TR101_VLAN_EGR_TAG_TAG1_E:
            __LOG(("remove tag 1 : changed from 'TAG1' to 'UNTAGGED' \n"));
            descrPtr->passangerTr101EgressVlanTagMode = SKERNEL_FRAME_TR101_VLAN_EGR_TAG_UNTAGGED_E;
            break;
        case SKERNEL_FRAME_TR101_VLAN_EGR_TAG_OUT_TAG0_IN_TAG1_E:
            __LOG(("remove tag 1 : changed from 'OUT_TAG0_IN_TAG1' to 'TAG0' \n"));
            descrPtr->passangerTr101EgressVlanTagMode = SKERNEL_FRAME_TR101_VLAN_EGR_TAG_TAG0_E;
            break;
        case SKERNEL_FRAME_TR101_VLAN_EGR_TAG_OUT_TAG1_IN_TAG0_E:
            __LOG(("remove tag 1 : changed from 'OUT_TAG1_IN_TAG0' to 'TAG0' \n"));
            descrPtr->passangerTr101EgressVlanTagMode = SKERNEL_FRAME_TR101_VLAN_EGR_TAG_TAG0_E;
            break;
        case SKERNEL_FRAME_TR101_VLAN_EGR_TAG_PUSH_TAG0_E:
            __LOG(("remove tag 1 : changed from 'PUSH_TAG0' to 'TAG0' \n"));
            descrPtr->passangerTr101EgressVlanTagMode = SKERNEL_FRAME_TR101_VLAN_EGR_TAG_TAG0_E;
            break;
        case SKERNEL_FRAME_TR101_VLAN_EGR_TAG_POP_OUT_TAG_E:
            __LOG(("remove tag 1 : changed from 'POP_OUT_TAG' to 'UANTAGGED' \n"));
            descrPtr->passangerTr101EgressVlanTagMode = SKERNEL_FRAME_TR101_VLAN_EGR_TAG_UNTAGGED_E;
            break;
        case SKERNEL_FRAME_TR101_VLAN_EGR_TAG_UNTAGGED_E:
        case SKERNEL_FRAME_TR101_VLAN_EGR_TAG_TAG0_E:
        case SKERNEL_FRAME_TR101_VLAN_EGR_TAG_DO_NOT_MODIFIED_E:/*not relevant for non sip5*/
        default:
            __LOG(("remove tag 1 : tag1 was not needed \n"));
            break;
    }

    return ;
}

/**
* @internal chtHaCalcTr101FinalState function
* @endinternal
*
* @brief   HA - calcl tr101 final tag state of tags 0,1
*
* @param[in] devObjPtr                -  pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in] egressPort               - egress port.
* @param[in] destVlanTagged           - pre HA needed tag state
* @param[in] passengerDestVlanTagged  - tag states for the passenger (when TS)
*/
static void  chtHaCalcTr101FinalState
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32           egressPort,
    IN GT_U32           destVlanTagged,
    IN GT_U32           passengerDestVlanTagged,
    IN GT_BIT           passengerNeedL2Info
)
{
    DECLARE_FUNC_NAME(chtHaCalcTr101FinalState);

    SKERNEL_FRAME_TR101_VLAN_EGR_TAG_STATE_ENT passangerTr101EgressVlanTagMode;
    GT_BOOL mtagTxSniffOrToCpu;

    descrPtr->passangerTr101EgressVlanTagMode = passengerDestVlanTagged;
    descrPtr->tr101EgressVlanTagMode          = destVlanTagged;

    mtagTxSniffOrToCpu = /* fix JIRA CPSS-16630: extend TR101 FWD condition */
        (descrPtr->outGoingMtagCmd == SKERNEL_MTAG_CMD_TO_CPU_E && descrPtr->srcTrg == 1) ||
        (descrPtr->outGoingMtagCmd == SKERNEL_MTAG_CMD_TO_TRG_SNIFFER_E && descrPtr->rxSniff == 0);
    mtagTxSniffOrToCpu &= descrPtr->isPktMirrorInLocalDev;

    if(descrPtr->outGoingMtagCmd != SKERNEL_MTAG_CMD_FORWARD_E &&
       mtagTxSniffOrToCpu != GT_TRUE)
    {
        __LOG(("Tr101 logic not relevant to non forward command \n"));
        return;
    }

    if(descrPtr->tr101ModeEn == 0)
    {
        __LOG(("not support TR101 \n"));

        /* reduce to 2 values */
        destVlanTagged = destVlanTagged ? 1 /*TAG0*/ : 0/*untagged*/;
        passengerDestVlanTagged = passengerDestVlanTagged ? 1 : 0;
    }

    if(devObjPtr->vplsModeEnable.ha &&
       descrPtr->vplsInfo.egressTagStateAssigned)
    {
        /* the device did assignment from the PCL/TTI/logical port table of descrPtr->vplsInfo.egressTagStateAssigned
            use it instead of info from the TXQ !
         */
         __LOG(("vplsModeEnable and egressTagStateAssigned , use egress tag state set by PCL/TTI/logical port \n"));
        descrPtr->passangerTr101EgressVlanTagMode = descrPtr->vplsInfo.egressTagState;
    }
    else
    if(descrPtr->tunnelStart && passengerNeedL2Info)/*relevant only to Ethernet-Over-X tunnel start */
    {
        if(descrPtr->ethernetOverXPassengerTagMode == 1)
        {
            /* force tag mode on the passenger */
            if(descrPtr->ethernetOverXPassengerTagged == 0)
            {
                /* passenger with no tags */
                descrPtr->passangerTr101EgressVlanTagMode = SKERNEL_FRAME_TR101_VLAN_EGR_TAG_UNTAGGED_E;
            }
            else
            {
                /* passenger with single tag */
                descrPtr->passangerTr101EgressVlanTagMode = SKERNEL_FRAME_TR101_VLAN_EGR_TAG_TAG0_E;
            }
            __LOG(("force Ethernet passenger egress vlan tag mode : use [%s] \n",
                descrPtr->passangerTr101EgressVlanTagMode ? "TAG0" : "UNTAGGED"));
        }
        else
        {
            /* state from the vlan */
            __LOG(("Passenger is Ethernet so may need vlan tags  \n"));
            descrPtr->passangerTr101EgressVlanTagMode = passengerDestVlanTagged;
        }
    }
    else
    if(descrPtr->tunnelStart && (passengerNeedL2Info == 0))
    {
        __LOG(("Passenger is not Ethernet so not never need vlan tags  \n"));
        descrPtr->passangerTr101EgressVlanTagMode = SKERNEL_FRAME_TR101_VLAN_EGR_TAG_UNTAGGED_E;
    }
    else
    {
        /* state from the vlan */
        descrPtr->passangerTr101EgressVlanTagMode = destVlanTagged;
    }
    __LOG_PARAM(descrPtr->passangerTr101EgressVlanTagMode);

    passangerTr101EgressVlanTagMode = descrPtr->passangerTr101EgressVlanTagMode;

    /* call logic of 'remote tag1 if ZERO' --> may update passangerTr101EgressVlanTagMode */
    snetHaEgressVlanTag1Removed(devObjPtr, descrPtr);
    if(passangerTr101EgressVlanTagMode != descrPtr->passangerTr101EgressVlanTagMode)
    {
        __LOG_PARAM(descrPtr->passangerTr101EgressVlanTagMode);
        passangerTr101EgressVlanTagMode = descrPtr->passangerTr101EgressVlanTagMode;
    }

    /* call logic of 'keep tag1' --> may update passangerTr101EgressVlanTagMode */
    snetLionHaKeepVlan1Check(devObjPtr, descrPtr, egressPort);
    if(passangerTr101EgressVlanTagMode != descrPtr->passangerTr101EgressVlanTagMode)
    {
        __LOG_PARAM(descrPtr->passangerTr101EgressVlanTagMode);
        passangerTr101EgressVlanTagMode = descrPtr->passangerTr101EgressVlanTagMode;
    }

    if(descrPtr->tunnelStart == 0)
    {
        descrPtr->tr101EgressVlanTagMode = descrPtr->passangerTr101EgressVlanTagMode;
    }
    else
    {
        descrPtr->tr101EgressVlanTagMode = destVlanTagged;
    }

    __LOG_PARAM(descrPtr->tr101EgressVlanTagMode);

    return;
}


/**
* @internal chtHaTargetAnalyzerTagModeGet function
* @endinternal
*
* @brief   HA - to sniffer analyzer tags info
*
* @param[in] devObjPtr                -  pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in] destVlanTagged           - send frame with tag.
*/
static GT_U8  chtHaTargetAnalyzerTagModeGet
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U8    destVlanTagged,
    IN GT_BOOL  isSecondRegister,
    IN GT_U32         outputPortBit
)
{
    DECLARE_FUNC_NAME(chtHaTargetAnalyzerTagModeGet);

    GT_U32   regAddr;           /* register address */
    GT_U32   *regPtr;           /* register data pointer */

    destVlanTagged = descrPtr->srcTaggedTrgTagged;
    __LOG(("TO_TRG_SNIFFER : use destVlanTagged[%d] from descrPtr->srcTaggedTrgTagged \n",
        destVlanTagged));

    /* Mirrored packets to analyzer may keep/add VLAN tag */
    descrPtr->analyzerKeepVlanTag = 1;

    /* Check that target sniff on 'own' device */
    if(SKERNEL_IS_MATCH_DEVICES_MAC(descrPtr->sniffTrgDev, descrPtr->ownDev,
                                  devObjPtr->dualDeviceIdEnable.ha))
    {
        if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
        {
            /*TO ANALYZER VLAN tag add enable*/
            descrPtr->analyzerVlanTagAdd =
                SMEM_LION3_HA_EPORT_2_ENTRY_FIELD_AUTO_GET(devObjPtr,
                    descrPtr,
                    SMEM_LION3_HA_EPORT_TABLE_2_TO_ANALYZER_VLAN_TAG_ADD_EN);

        }
        else
        {
            /* TO ANALYZER VLAN Tag Add Enable Configuration Register */
            regAddr = SMEM_CHT_TO_ANALYZER_VLAN_TAG_ADD_EN_REG(devObjPtr);
            if(isSecondRegister == GT_TRUE)
            {
                regAddr +=4;
            }

            regPtr = smemMemGet(devObjPtr, regAddr);
            /* Bit per analyzer port indicating if a VLAN Tag is to be added to TO_ANALYZER Packets */
            descrPtr->analyzerVlanTagAdd =
                SMEM_U32_GET_FIELD(*regPtr, outputPortBit, 1);
        }

        if(descrPtr->analyzerVlanTagAdd)
        {
            __LOG(("TO ANALYZER VLAN tag add enable \n"));
        }

        if(SKERNEL_IS_LION_REVISON_B0_DEV(devObjPtr))
        {
            if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
            {
                /*Mirror to analyzer header (send tagged or not)*/
                descrPtr->analyzerKeepVlanTag =
                    SMEM_LION3_HA_EPORT_2_ENTRY_FIELD_AUTO_GET(devObjPtr,
                        descrPtr,
                        SMEM_LION3_HA_EPORT_TABLE_2_MIRROR_TO_ANALYZER_KEEP_TAGS);
            }
            else
            {
                regAddr = SMEM_LION_HA_MIRR_TO_ANALYZER_HEADER_CONF_REG(devObjPtr);
                if(isSecondRegister == GT_TRUE)
                {
                    regAddr+=4;
                }

                regPtr = smemMemGet(devObjPtr, regAddr);
                /* VLAN tag removal is configurable per analyzer port */
                descrPtr->analyzerKeepVlanTag =
                    SMEM_U32_GET_FIELD(*regPtr, outputPortBit, 1);
            }

            /* Mirrored packets do not keep the VLAN tag */
            if(descrPtr->analyzerKeepVlanTag == 0)
            {
                /* Remove VLAN tag from TO_ANALYZER packets */
                destVlanTagged = 0;
                __LOG(("descrPtr->analyzerKeepVlanTag == 0 :Remove VLAN tag from TO_ANALYZER packets , use destVlanTagged[%d] \n",
                    destVlanTagged));
            }
        }
    }

    return destVlanTagged;
}

/**
* @internal snetCalcTcpUdpIncChecksum function
* @endinternal
*
* @brief   Checksum Adjustment
*         NAT modifications are per packet based and can be very compute
*         intensive, as they involve one or more checksum modifications in
*         addition to simple field translations. Luckily, we have an algorithm
*         below, which makes checksum adjustment to IP, TCP, UDP and ICMP
*         headers very simple and efficient. Since all these headers use a
*         one's complement sum, it is sufficient to calculate the arithmetic
*         difference between the before-translation and after-translation
*         addresses and add this to the checksum. The algorithm below is
*         applicable only for even offsets (i.e., optr below must be at an even
*         offset from start of header) and even lengths (i.e., olen and nlen
*         below must be even). Sample code (in C) for this is as follows.
* @param[in,out] chksum                   -  points to the  in the packet
* @param[in] optr                     - points to the old data in the packet
* @param[in] olen                     - old data length
* @param[in] nptr                     - points to the new data in the packet
* @param[in] nlen                     - new data length
* @param[in,out] chksum                   -  points to the  in the packet
*                                      RETURN:
*                                      None
*/
static GT_VOID snetCalcTcpUdpIncChecksum
(
    INOUT GT_U8   *chksum,
    IN    GT_U8   *optr,
    IN    GT_U32   olen,
    IN    GT_U8   *nptr,
    IN    GT_U32   nlen
)
{

     long x, old, new;
     x=chksum[0]*256+chksum[1];
     x=~x & 0xFFFF;
     while (olen)
     {
         old=optr[0]*256+optr[1]; optr+=2;
         x-=old & 0xffff;
         if (x<=0) { x--; x&=0xffff; }
         olen-=2;
     }
     while (nlen)
     {
         new=nptr[0]*256+nptr[1]; nptr+=2;
         x+=new & 0xffff;
         if (x & 0x10000) { x++; x&=0xffff; }
         nlen-=2;
     }
     x=~x & 0xFFFF;
     chksum[0]=x/256; chksum[1]=x & 0xff;
}

/**
* @internal snetBc2B0HaNatIpv4CheckException function
* @endinternal
*
* @brief   NAT: ipv4 check exception
*         For NAT L4 modification If the IPv4 header length (IPv4 IHL)
*         is too large for l4 modifications:
*         - For UDP the maximal IHL is 14
*         - For TCP the maximal IHL is 11
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to packet descriptor
*/
static GT_BOOL snetBc2B0HaNatIpv4CheckException
(
    IN SKERNEL_DEVICE_OBJECT               *devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC     *descrPtr
)
{
    DECLARE_FUNC_NAME(snetBc2B0HaNatIpv4CheckException);

    GT_U32  maxIHL = 14;
    GT_U32  currentIHL = descrPtr->ipxHeaderLength;
    GT_U32  regAddr = SMEM_BOBCAT2_HA_NAT_EXCEPTION_DROP_COUNTER_REG(devObjPtr);
    GT_U32  counterValue = 0;

    GT_BOOL isNatException = GT_FALSE;

    if(descrPtr->ipProt == SNET_TCP_PROT_E)
    {
        maxIHL = 11;
    }

    if(currentIHL < 5)
    {
        __LOG(("IHL is less than minimal value 5: [%d] \n", currentIHL));
        isNatException = GT_TRUE;
    }

    if(currentIHL  > maxIHL)
    {
        __LOG(("IHL is more than maximal value %d: [%d] \n", maxIHL, currentIHL));
        isNatException = GT_TRUE;
    }

    if(GT_TRUE == isNatException)
    {
        __LOG(("NAT exception, packet will be dropped \n"));

        __LOG(("Generate interrupt <Oversize L4 Offset for NAT> \n"));
        snetChetahDoInterrupt(devObjPtr,
                              SMEM_LION3_HA_INTERRUPT_CAUSE_REG(devObjPtr),
                              SMEM_LION3_HA_INTERRUPT_MASK_REG(devObjPtr),
                              (1<<15),/*Oversize L4 Offset for NAT*/
                              (GT_U32)SMEM_LION3_HA_SUM_INT(devObjPtr));


        smemRegGet(devObjPtr, regAddr, &counterValue);
        smemRegSet(devObjPtr, regAddr, ++counterValue);
        __LOG(("Incremented NAT Drop Counter Register by 1 from [%d] \n", (counterValue-1)));

        __LOG(("Drop the packet \n"));
        descrPtr->haAction.drop = SKERNEL_EXT_PKT_CMD_HARD_DROP_E;
    }

    return isNatException;
}

/**
* @internal snetBc2B0HaNatIpv4L4ChecksumPreparePseudoHeader function
* @endinternal
*
* @brief   NAT: Prepare pseudo-header for L4 checksum computation (IPv4)
*         +--------+--------+--------+--------+
*         |      Source Address     |
*         +--------+--------+--------+--------+
*         |     Destination Address    |
*         +--------+--------+--------+--------+
*         | zero |Protocol| TCP/UDP Length |
*         +--------+--------+--------+--------+
*         + TCP or UDP header
*         + packet data
*/
static GT_VOID snetBc2B0HaNatIpv4L4ChecksumPreparePseudoHeader
(
    IN SKERNEL_DEVICE_OBJECT               *devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC     *descrPtr,
    IN GT_U8                               *l3StartOffsetPtr,
    IN GT_U8                               *l4StartOffsetPtr,
    IN GT_U8                               pseudoHeaderArr[L3_AND_l4_PSEUDO_HEADER_LENGTH],
    IN GT_U32                              *dataLenPtr
)
{
    DECLARE_FUNC_NAME(snetBc2B0HaNatIpv4L4ChecksumPreparePseudoHeader);

    GT_U32 length,checksumOffset;

    if(descrPtr->ipProt == SNET_UDP_PROT_E)
    {
        length = (l4StartOffsetPtr[4] << 8) + l4StartOffsetPtr[5];
        __LOG(("L4 UDP len [0x%X] \n", length));
    }
    else if(descrPtr->ipProt == SNET_TCP_PROT_E)
    {
        length = (l3StartOffsetPtr[2] << 8) + l3StartOffsetPtr[3];
        __LOG(("L3 len [0x%X] \n", length));

        length -= (descrPtr->ipxHeaderLength*4);
        __LOG(("L4 (TCP+DATA) len [0x%X] \n", length));
    }
    else
    {
        __LOG(("unknown L4 protocol, do nothing"));
        return;
    }

    checksumOffset = CHECKSUM_OFFSET_IN_L4_MAC(descrPtr->ipProt);

    __LOG(("Prepare pseudo-header for L4 checksum computation (IPv4)\n" ));
    memcpy(pseudoHeaderArr, &l3StartOffsetPtr[IPV4_L3_SIP_OFFSET], 8); /* sip and dip */

    pseudoHeaderArr[8]  = 0; /* zero */
    pseudoHeaderArr[9]  = descrPtr->ipProt; /* ip protocol */
    pseudoHeaderArr[10] = length >> 8;
    pseudoHeaderArr[11] = length & 0xFF;

    /* copy needed bytes from the L4 , till the checksum */
    memcpy(&pseudoHeaderArr[12],l4StartOffsetPtr,checksumOffset);
    /* clear the checksum and the rest of the bytes */
    memset(&pseudoHeaderArr[12+checksumOffset],0,(30-(12+checksumOffset)));

    *dataLenPtr = length /*L4 length*/ - (checksumOffset+CHECKSUM_NUM_BYTES) /* L4 bytes copied to pseudoHeaderArr[] */;

    __LOG(("L4 checksum data len [0x%X] \n", *dataLenPtr));
}


/**
* @internal snetBc2B0HaNatIpv4L4ChecksumUpdate function
* @endinternal
*
* @brief   NAT: ipv4 L4 checksum update
*         TCP checksum should be updated as described in RFC 3022 section 4.2.
*         UDP checksum should not be updated if the original UDP checksum is 0,
*         otherwise it is updated as described in RFC 3022 section 4.2.
*         If the result of the update is 0x0 then the checksum should be set to 0xFFFF
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] descrPtr                 - pointer to packet descriptor
* @param[in] ingPseudoHeaderArr[L3_AND_l4_PSEUDO_HEADER_LENGTH/30/] - pointer to old header data
* @param[in] egrPseudoHeaderArr[L3_AND_l4_PSEUDO_HEADER_LENGTH/30/] - len old header data
* @param[in,out] checksumBuffPtr[CHECKSUM_NUM_BYTES/2/] - pointer to buffer
* @param[in,out] descrPtr                 - pointer to packet descriptor
*/
static GT_VOID snetBc2B0HaNatIpv4L4ChecksumUpdate
(
    IN    SKERNEL_DEVICE_OBJECT               *devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC     *descrPtr,
    /* ingress info */
    IN    GT_U8                               ingPseudoHeaderArr[L3_AND_l4_PSEUDO_HEADER_LENGTH/*30*/],
    /* egress info */
    IN    GT_U8                               egrPseudoHeaderArr[L3_AND_l4_PSEUDO_HEADER_LENGTH/*30*/],
    /* the place to update the checksum */
    INOUT GT_U8                                checksumBuffPtr[CHECKSUM_NUM_BYTES/*2*/]
)
{
    DECLARE_FUNC_NAME(snetBc2B0HaNatIpv4L4ChecksumUpdate);

    /* update the checksum : first part */
    snetCalcTcpUdpIncChecksum(checksumBuffPtr, ingPseudoHeaderArr, L3_AND_l4_PSEUDO_HEADER_LENGTH, egrPseudoHeaderArr, L3_AND_l4_PSEUDO_HEADER_LENGTH);

    if(0 == checksumBuffPtr[0] &&
       0 == checksumBuffPtr[1] )
    {
       checksumBuffPtr[0] = 0xFF;
       checksumBuffPtr[1] = 0xFF;
        __LOG(("The result of the checksum can't be 0x0 --> changed to 0xFFFF \n"));
    }
    else
    {
        __LOG(("Updated checksum is: 0x%X%X\n", checksumBuffPtr[0],  checksumBuffPtr[1]));
    }
}
/**
* @internal snetCalcIncChecksum function
* @endinternal
*
* @brief   Calculate incremental 16bit TCP/UDP checksum based on original header and new header.
*
* @param[in] origPrefixPtr            - (pointer to) original prefix
* @param[in] newPrefixPtr             - (pointer to) new prefix
* @param[in] origChecksum             - original checksum
*/
static GT_U16 snetCalcIncChecksum
(
    IN GT_U32 *origPrefixPtr,
    IN GT_U32 *newPrefixPtr,
    IN GT_U16 origChecksum
)
{
    GT_U32 newChecksum = ~origChecksum & 0xFFFF; /* HC' = ~HC */
    GT_U32 i;

    /* doing checksum calculation on 2 words */
     for (i=0; i<2; ++i)
     {
         /* HC' += (~m + m') */
         newChecksum += ((~origPrefixPtr[i] & 0xffff) + (newPrefixPtr[i] & 0xffff) +
             ((~origPrefixPtr[i] >> 16) & 0xffff) + ((newPrefixPtr[i] >> 16) & 0xffff));
     }

    while (newChecksum > 0xffff) {
        /* Add also the carry to the sum */
        newChecksum = (newChecksum & 0xffff) + ((newChecksum >> 16) & 0xffff);
    }

    /* HC' = ~HC' */
    newChecksum = ~newChecksum & 0xffff;

  return newChecksum;
}

/**
* @internal haNatCheckEntryValidity function
* @endinternal
*
* @brief   NAT: ipv4/6 check NAT entry type validity.
*         For NAT44 If the packet is ipv4 but the NAT entry type is ipv6_over_ipv6
*         For NAT66 If the packet is ipv6 but the NAT entry type is ipv4_over_ipv4
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to packet descriptor
* @param[in] natEntryType             - the nat entry type
*/
static GT_BOOL haNatCheckEntryValidity
(
    IN SKERNEL_DEVICE_OBJECT               *devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC     *descrPtr,
    IN GT_U32   natEntryType
)
{
    DECLARE_FUNC_NAME(haNatCheckException);

    GT_U32 regAddr = SMEM_BOBCAT2_HA_NAT_EXCEPTION_DROP_COUNTER_REG(devObjPtr);
    GT_U32 counterValue;
    GT_BOOL isNatException = GT_FALSE;

    if (descrPtr->isIPv4)
    {
        if (natEntryType != 0)
        {
            __LOG(("packet is IPv4 but NAT entry type is not NAT44 so the packet is dropped \n"));
            isNatException = GT_TRUE;
        }
    }
    else /* packet is ipv6 */
    {
        if (natEntryType != 1)
        {
            __LOG(("packet is IPv6 but NAT entry type is not NAT66 so the packet is dropped \n"));
            isNatException = GT_TRUE;
        }
    }


    if(GT_TRUE == isNatException)
    {
        __LOG(("NAT exception, packet will be dropped \n"));

        smemRegGet(devObjPtr, regAddr, &counterValue);
        smemRegSet(devObjPtr, regAddr, ++counterValue);
        __LOG(("Incremented NAT Drop Counter Register by 1 from [%d] \n", (counterValue-1)));

        __LOG(("Drop the packet \n"));
        descrPtr->haAction.drop = SKERNEL_EXT_PKT_CMD_HARD_DROP_E;
    }

    return isNatException;
}

/**
* @internal haProcessNat44L3L4HeaderModify function
* @endinternal
*
* @brief   HA - NAT : do ipv4 and L4 header modifications.
*
* @param[in] devObjPtr                -  pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in] l3HeaderChangedBuffer    - pointer to ipv4/6 header
* @param[in] l4HeaderChangedBuffer    - pointer to TCP/UDP header
*                                      relevant only when not NULL and l4HeaderChangedLen != 0
* @param[in] l4HeaderChangedLen       - number of bytes in l4HeaderChangedBuffer
* @param[in] isL2Nat                  - flag to indicate whether L2NAT functionality
*                                       is enabled/disabled
*/
static void  haProcessNat44L3L4HeaderModify
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    GT_U8    *l3HeaderChangedBuffer/*[40]*/,
    GT_U8    *l4HeaderChangedBuffer/*[18]*/,
    GT_U32   l4HeaderChangedLen,
    GT_BOOL  isL2Nat
)
{
#define SIP5_10_NAT_FIELD_GET_MAC(field)\
    SMEM_LION3_HA_NAT44_ENTRY_FIELD_GET(devObjPtr, tunnelStartActionPtr, natIndex, field)

    DECLARE_FUNC_NAME(haProcessNat44L3L4HeaderModify);

    GT_U32 natIndex = descrPtr->arpPtr;
    GT_U32 *tunnelStartActionPtr;
    GT_U8  *checksumPtr;
    GT_U8  tmpBuffer[4]    = {0};
    GT_U32 tmpWord;
    GT_U8  pseudoHeaderArr[L3_AND_l4_PSEUDO_HEADER_LENGTH];/* (ingress info) pointer to pseudo-header data */
    GT_U8  egrPseudoHeaderArr[L3_AND_l4_PSEUDO_HEADER_LENGTH];/* (egress info) pointer to pseudo-header data */
    GT_U32 oldChecksumDataLen,newChecksumDataLen,checksumOffset;
    GT_U32  natEntryType;
    GT_U32  l2NatPtr;
    GT_U32  l2NatPtrOffset = 0;
    GT_U32  l2NatSrcIdSize;
    GT_U32  l2NatSrcIdLsb;


    if(isL2Nat)/* if L2NAT functionality is enabled */
    {
        /*Get NAT pointer from EPORT table */
        l2NatPtr = SMEM_LION3_HA_EPORT_1_ENTRY_FIELD_AUTO_GET(devObjPtr, descrPtr,
                                  SMEM_SIP6_30_HA_EPORT_TABLE_1_FIELDS_NAT_PTR_E);
        /* Get L2NAT_SrcId Size from HA global configuration */
        smemRegFldGet(devObjPtr,
                      SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->HA.haGlobalConfig2,
                      14, 4, &l2NatSrcIdSize);
        __LOG_PARAM(l2NatSrcIdSize);

        /* Get L2NAT_SrcId LSB from HA global configuration */
        smemRegFldGet(devObjPtr,
                      SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->HA.haGlobalConfig2,
                      18, 4, &l2NatSrcIdLsb);
        __LOG_PARAM(l2NatSrcIdLsb);

        /* Calculate NAT pointer offset */
        if (l2NatSrcIdSize == 0)
        {
            l2NatPtrOffset = 0;
        }
        else
        {
            /* Derive from SrcID size and LSB */
            l2NatPtrOffset = SMEM_U32_GET_FIELD(descrPtr->sstId, l2NatSrcIdLsb, l2NatSrcIdSize);
            __LOG_PARAM(descrPtr->sstId);
        }
        __LOG_PARAM(l2NatPtrOffset);

        /* Derive NAT index */
        natIndex = l2NatPtr + l2NatPtrOffset;

        /* isL2Nat and arpPtr will be used in CNC update logic */
        descrPtr->isL2Nat = 1;
        descrPtr->arpPtr = natIndex;
    }
    else
    {
        if (descrPtr->l4StartOffsetPtr == NULL)
        {
            __LOG(("ERROR: L4 header not exists \n"));
            return;
        }

        if(l4HeaderChangedLen == 0 || l4HeaderChangedBuffer == NULL)
        {
            skernelFatalError("haProcessNat44L3L4HeaderModify : NULL pointer \n");
            return;
        }
    }

    __LOG_PARAM(descrPtr->isL2Nat);

    if (SMEM_CHT_IS_SIP7_GET(devObjPtr))
    {
        /* the format of the entry as in SIP5 devices, but the entry is in a new ARP table */
        GT_U32  entryNumBits = 192;
        GT_U32  lineNumBits  = 384;
        GT_U32  entriesPerLine = lineNumBits / entryNumBits;
        GT_U32  indexBitResolution =
            lineNumBits / devObjPtr->tablesInfo.arpTable.paramInfo[0].divider;
        GT_U32  indexMult = entryNumBits / indexBitResolution;
        tunnelStartActionPtr = smemMemGet(
            devObjPtr, SMEM_TABLE_ENTRY_INDEX_GET_MAC(devObjPtr, arpTable, (natIndex * indexMult)));
        tunnelStartActionPtr += ((entryNumBits * (natIndex % entriesPerLine)) / 32);
    }
    else
    {
        tunnelStartActionPtr = smemMemGet(
            devObjPtr, SMEM_CHT2_TUNNEL_START_TBL_MEM(devObjPtr,natIndex));
        if(natIndex & 1)
        {
            /* the odd entries are in the second half of the entry (offset of 192 bits = 6 words) */
            tunnelStartActionPtr += 6;
        }
    }

    __LOG(("get NAT entry by index: %d \n", natIndex));

    natEntryType = SIP5_10_NAT_FIELD_GET_MAC(SMEM_LION3_HA_NAT44_TABLE_FIELDS_NAT_ENTRY_TYPE);
    if (GT_TRUE == haNatCheckEntryValidity(devObjPtr, descrPtr,natEntryType))
    {
        return;
    }


    /****  check nat exception  ****/
    if (GT_TRUE == snetBc2B0HaNatIpv4CheckException(devObjPtr, descrPtr))
    {
        return; /* exception, packet dropped */
    }

    __LOG(("apply NAT entry to the packet \n"));

    /****  layer 3 changes  ****/
    if(SIP5_10_NAT_FIELD_GET_MAC(SMEM_LION3_HA_NAT44_TABLE_FIELDS_MODIFY_DIP))
    {
        /* In L2NAT when Desc<IPM>=1 : SIP address is modified and DIP address
         * is unmodified */
        if (isL2Nat && descrPtr->ipm)
        {
            __LOG(("L2NAT is enabled and packet is IP Multicast hence DIP is not changed \n"));
        }
        else
        {
            /* packet DIP applied to buffer here, and not in chtHaUnit */
            tmpWord = SIP5_10_NAT_FIELD_GET_MAC(SMEM_LION3_HA_NAT44_TABLE_FIELDS_NEW_DIP);

            if(SMEM_CHT_IS_SIP6_30_GET(devObjPtr))
            {
                GT_U32  ipMaskNumBits =
                    SIP5_10_NAT_FIELD_GET_MAC(SMEM_SIP6_30_HA_NAT44_TABLE_FIELDS_NAT_DIP_PREFIX_LENGTH);
                GT_U32  ipMask = 0xffffffff << (31 - ipMaskNumBits);
                GT_U32  oldIp = SNET_BUILD_WORD_FROM_BYTES_PTR_MAC(&l3HeaderChangedBuffer[IPV4_L3_DIP_OFFSET]);

                __LOG(("sip6.30 : The DIP for NAT uses 'prefix' of [%d] bits (take [0x%8.8x] mask from 'NAT enty')\n",
                            ipMaskNumBits+1,ipMask));

                tmpWord = (tmpWord & ipMask) | (oldIp & (~ipMask));
            }


            SNET_BUILD_BYTES_FROM_WORD_MAC(tmpWord, tmpBuffer);
            memcpy(&l3HeaderChangedBuffer[IPV4_L3_DIP_OFFSET], tmpBuffer, 4); /* nextBuffer points to L3 dip */

            __LOG(("packet DIP was changed by NAT [0x%8.8x]\n",tmpWord));
        }
    }

    if(SIP5_10_NAT_FIELD_GET_MAC(SMEM_LION3_HA_NAT44_TABLE_FIELDS_MODIFY_SIP))
    {
        /* packet SIP applied to buffer here, and not in chtHaUnit */
        tmpWord = SIP5_10_NAT_FIELD_GET_MAC(SMEM_LION3_HA_NAT44_TABLE_FIELDS_NEW_SIP);

        if(SMEM_CHT_IS_SIP6_30_GET(devObjPtr))
        {
            GT_U32  ipMaskNumBits =
                SIP5_10_NAT_FIELD_GET_MAC(SMEM_SIP6_30_HA_NAT44_TABLE_FIELDS_NAT_SIP_PREFIX_LENGTH);
            GT_U32  ipMask = 0xffffffff << (31 - ipMaskNumBits);
            GT_U32  oldIp = SNET_BUILD_WORD_FROM_BYTES_PTR_MAC(&l3HeaderChangedBuffer[IPV4_L3_SIP_OFFSET]);

            __LOG(("sip6.30 : The SIP for NAT uses 'prefix' of [%d] bits (take [0x%8.8x] mask from 'NAT enty')\n",
                ipMaskNumBits+1,ipMask));

            tmpWord = (tmpWord & ipMask) | (oldIp & (~ipMask));
        }

        SNET_BUILD_BYTES_FROM_WORD_MAC(tmpWord, tmpBuffer);
        memcpy(&l3HeaderChangedBuffer[IPV4_L3_SIP_OFFSET], tmpBuffer, 4); /* nextBuffer points to L3 sip */

        __LOG(("packet SIP was changed by NAT [0x%8.8x]\n",tmpWord));

    }
    /* ip (l3) checksum will be updated in chtHaUnit */

    checksumOffset = CHECKSUM_OFFSET_IN_L4_MAC(descrPtr->ipProt);
    checksumPtr = &descrPtr->l4StartOffsetPtr[checksumOffset];

    /****  layer 4 changes  ****/
     if(descrPtr->l4Valid && descrPtr->l4StartOffsetPtr &&   /* layer 4 valid  and*/
       (descrPtr->ipProt == SNET_TCP_PROT_E ||              /* packet is TCP or UDP  and*/
        descrPtr->ipProt == SNET_UDP_PROT_E)  &&
        0 == descrPtr->ipv4FragmentOffset)                   /* packet is not fragmented */
     {

        if(SIP5_10_NAT_FIELD_GET_MAC(SMEM_LION3_HA_NAT44_TABLE_FIELDS_MODIFY_TCP_UDP_DST_PORT))
        {
            /* During L2NAT only SIP and DIP fields sould be modified.
             * If translation on other fields is required then trap the packet to CPU */
            if (isL2Nat)
            {
                __LOG(("Trying to modify L4 header(TCP_UDP_DST_PORT) in L2NAT. Trapping packet to CPU\n"));
                descrPtr->packetCmd = SKERNEL_EXT_PKT_CMD_TRAP_TO_CPU_E;
                return;
            }
            /* packet dst port applied to buffer here, and not in chtHaUnit */
            tmpWord = SIP5_10_NAT_FIELD_GET_MAC(SMEM_LION3_HA_NAT44_TABLE_FIELDS_NEW_TCP_UDP_DST_PORT);
            SNET_BUILD_BYTES_FROM_WORD_MAC(tmpWord, tmpBuffer);
            memcpy(&l4HeaderChangedBuffer[IPV4_L4_DST_PORT_OFFSET], &tmpBuffer[2], 2); /* nextBuffer points to L4 dst port */

            __LOG(("packet tcp/udp destination port was changed by NAT: new value [%d] \n", tmpWord));
        }

        if(SIP5_10_NAT_FIELD_GET_MAC(SMEM_LION3_HA_NAT44_TABLE_FIELDS_MODIFY_TCP_UDP_SRC_PORT))
        {
            /* During L2NAT only SIP and DIP fields sould be modified.
             * If translation on other fields is required then trap the packet to CPU */
            if (isL2Nat)
            {
                __LOG(("Trying to modify L4 header(TCP_UDP_SRC_PORT) in L2NAT. Trapping packet to CPU\n"));
                descrPtr->packetCmd = SKERNEL_EXT_PKT_CMD_TRAP_TO_CPU_E;
                return;
            }
            /* packet src port applied to buffer here, and not in chtHaUnit */
            tmpWord = SIP5_10_NAT_FIELD_GET_MAC(SMEM_LION3_HA_NAT44_TABLE_FIELDS_NEW_TCP_UDP_SRC_PORT);
            SNET_BUILD_BYTES_FROM_WORD_MAC(tmpWord, tmpBuffer);
            memcpy(&l4HeaderChangedBuffer[IPV4_L4_SRC_PORT_OFFSET], &tmpBuffer[2], 2); /* nextBuffer points to L4 src port */

            __LOG(("packet tcp/udp source port was changed by NAT: new value [%d] \n", tmpWord));
        }

        if(descrPtr->ipProt == SNET_UDP_PROT_E &&
           0 == checksumPtr[0] &&
           0 == checksumPtr[1] )
        {
            __LOG(("UDP checksum in not be updated since the original UDP checksum is 0\n"));
        }
        else
        {
            /* on ingress buffer -- build pseudo header for the l4 checksum calculation */
            snetBc2B0HaNatIpv4L4ChecksumPreparePseudoHeader(devObjPtr, descrPtr,
                                descrPtr->l3StartOffsetPtr,
                                descrPtr->l4StartOffsetPtr,
                                &pseudoHeaderArr[0],
                                &oldChecksumDataLen);


            /* on egress buffer -- build pseudo header for the l4 checksum calculation */
            snetBc2B0HaNatIpv4L4ChecksumPreparePseudoHeader(devObjPtr, descrPtr,
                                l3HeaderChangedBuffer,/* L3 header*/
                                l4HeaderChangedBuffer,/*L4 header*/
                                &egrPseudoHeaderArr[0],
                                &newChecksumDataLen);

            /* copy the original checksum from the ingress packet */
            l4HeaderChangedBuffer[checksumOffset+0] = checksumPtr[0];
            l4HeaderChangedBuffer[checksumOffset+1] = checksumPtr[1];
            /* get new tcp/udp l4 checksum */
            snetBc2B0HaNatIpv4L4ChecksumUpdate(devObjPtr, descrPtr,
                   /* ingress info */
                   pseudoHeaderArr,
                   /* egress info */
                   egrPseudoHeaderArr,
                   /* the place to update the checksum */
                   &l4HeaderChangedBuffer[checksumOffset]
             );
        }
    }
    else
    {
        __LOG(("l4 is not changed by NAT\n"));
    }

#undef SIP5_10_NAT_FIELD_GET_MAC
}



/**
* @internal snetBobKHaNatIpv6L3ChecksumUpdate function
* @endinternal
*
* @brief   NAT: ipv6 L3 checksum update
*         Enumeration of the IPv6 Address [RFC4291]
*         0  15 16  31 32  47 48  63 64  79 80  95 96 111 112 127
*         +-------+-------+-------+-------+-------+-------+-------+-------+
*         |   Routing Prefix  | Subnet|  Interface Identifier (IID) |
*         +-------+-------+-------+-------+-------+-------+-------+-------+
*         NAT ipv6 with a /48 or Shorter Prefix
*         When an NPTv6 Translator is configured with internal and external
*         prefixes that are 48 bits in length (a /48) or shorter, the
*         adjustment MUST be added to or subtracted from bits 48..63 of the
*         address.
*         This mapping results in no modification of the Interface Identifier
*         (IID), which is held in the lower half of the IPv6 address, so it
*         will not interfere with future protocols that may use unique IIDs for
*         node identification.
*         NAT ipv6 with a /49 or Longer Prefix
*         When an NPTv6 Translator is configured with internal and external
*         prefixes that are longer than 48 bits in length (such as a /52, /56,
*         or /60), the adjustment must be added to or subtracted from one of
*         the words in bits 64..79, 80..95, 96..111, or 112..127 of the
*         address. While the choice of word is immaterial as long as it is
*         consistent, these words MUST be inspected in that sequence and the
*         first that is not initially 0xFFFF chosen, for consistency's sake.
*         [RFC6296]
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to packet descriptor
* @param[in] tunnelStartActionPtr     - pointer to the NAT entry memory
* @param[in] performChecksumUpdate    - GT_TRUE: perform checksum-neutral mapping
*                                       GT_FALSE: without performing checksum-neutral
* @param[in] oldAddress               - the old sip/dip address
* @param[out] newAddress              - the new sip/dip address
*/
static GT_VOID snetBobKHaNatIpv6L3ChecksumUpdate
(
    IN    SKERNEL_DEVICE_OBJECT               *devObjPtr,
    IN    SKERNEL_FRAME_CHEETAH_DESCR_STC     *descrPtr,
    IN    GT_U32                              *tunnelStartActionPtr,
    IN    GT_BOOL                             performChecksumUpdate,
    IN    GT_U8                               oldAddressU8[16],
    OUT   GT_U8                               newAddressU8[16]
)
{
#define SIP5_15_NAT_FIELD_GET_MAC(field)\
    SMEM_SIP5_15_HA_NAT66_ENTRY_FIELD_GET(devObjPtr, tunnelStartActionPtr, natIndex, field)

    DECLARE_FUNC_NAME(snetBobKHaNatIpv6L3ChecksumUpdate);

    GT_U32 natIndex = descrPtr->arpPtr;
    GT_U32 regAddr = SMEM_BOBCAT2_HA_NAT_EXCEPTION_DROP_COUNTER_REG(devObjPtr);
    GT_U32 counterValue = 0;
    GT_U16 newChecksum = 0;
    GT_U16 origChecksum = 0;
    GT_U32 bytePos; /* the minimal byte position for prefixSize > 48 claculation */
    GT_U32 i;
    GT_U32 ipv6AddressWords[4] = {0}; /* new ip address to from nat entry */
    GT_U32 ipv6NatAddressU32[4];      /* new ip address from nat entry network order with Array of words */
    GT_U8  ipv6NatAddressU8[16];      /* new ip address from nat entry network order with Array of bytes */
    GT_U16 oldAddressU16[8];          /* existing ip address with array of 16 bits */
    GT_U32 numBitsInField, byteIndex;
    GT_U32 prefixSize;
    GT_U32 tempU32,word = 0;
    GT_U32 newAddressU32[4] = {0};
    GT_U32 oldAddressU32[4] = {0};


    prefixSize = SIP5_15_NAT_FIELD_GET_MAC(SMEM_SIP5_15_HA_NAT66_TABLE_FIELDS_PREFIX_SIZE);
    if (prefixSize == 0)
    {
        /* Set the size of the new prefix of the packet as 64*/
        prefixSize = 64;
    }

    SMEM_SIP5_15_HA_NAT66_ENTRY_FIELD_IPV6_ADDR_GET(devObjPtr, tunnelStartActionPtr, natIndex, ipv6AddressWords);

    /* convert the ip address to network order */
    ipv6NatAddressU32[3] = ipv6AddressWords[0];
    ipv6NatAddressU32[2] = ipv6AddressWords[1];
    ipv6NatAddressU32[1] = ipv6AddressWords[2];
    ipv6NatAddressU32[0] = ipv6AddressWords[3];

    /* convert the ip address to GT_U32 */
    for (i=0; i<4; i++)
    {
        oldAddressU32[i] = SNET_BUILD_WORD_FROM_BYTES_PTR_MAC(&oldAddressU8[i*4]);
    }
    /* convert the ip addresses to GT_U8 */
    for (i=0; i<4; i++)
    {
        SNET_BUILD_BYTES_FROM_WORD_WITH_INDEX_MAC(ipv6NatAddressU32[i],ipv6NatAddressU8,i*4);
    }

    /* copy the old ip address by bytes */
    memcpy(&newAddressU8[0],&oldAddressU8[0],16);
    memcpy(&newAddressU32[0],&oldAddressU32[0],16);

    /* NAT ipv6 with a /48 or Shorter Prefix */
    if (prefixSize <= 48)
    {
        origChecksum = ((oldAddressU8[6] << 8) | oldAddressU8[7]);
        i = 0; /* word '0' */

        /* prefix size is max 48 bits */
        if (prefixSize >= 32)
        {
            /* write whole first word */
            SNET_BUILD_BYTES_FROM_WORD_MAC(ipv6NatAddressU32[0],newAddressU8); /* copy the first word of the prefix to the new ip address */
            i = 1;/* word '1' */
            prefixSize -= 32;
        }

        if (prefixSize != 0)
        {
            /* write the remaining MS bits of the prefix */
            tempU32 = ipv6NatAddressU32[i] >> (32-prefixSize); /* clean the not relevant LSBits */
            tempU32 = tempU32 << (32-prefixSize);
            word = SNET_BUILD_WORD_FROM_BYTES_PTR_MAC(&newAddressU8[i*4]);
            word = word << prefixSize; /* clean the not relevant MSBits */
            word = word >> prefixSize;
            word = word | tempU32;    /* copy the remaining bits of the prefix to the */
                                                        /* word of the new ip address */

            SNET_BUILD_BYTES_FROM_WORD_WITH_INDEX_MAC(word,newAddressU8,i*4);
        }

        /* If the Original Address[48:63]= 0xFFFF the packet is dropped */
        /* According to [RFC1071], network using NPTv6 Translation and a /48 external prefix MUST NOT
           use the value 0xFFFF to designate a subnet that it expects to be translated. */
        if((performChecksumUpdate==GT_TRUE) && (origChecksum == 0xFFFF))
        {
            smemRegGet(devObjPtr, regAddr, &counterValue);
            smemRegSet(devObjPtr, regAddr, ++counterValue);
            __LOG(("Incremented NAT Drop Counter Register by 1 from [%d] \n", (counterValue-1)));

            __LOG(("The subnet == 0xFFFF so the packet is dropped \n"));
            descrPtr->haAction.drop = SKERNEL_EXT_PKT_CMD_HARD_DROP_E;
        }

        /* convert the address to U32*/
        newAddressU32[0] = SNET_BUILD_WORD_FROM_BYTES_PTR_MAC(&newAddressU8[0]);
        newAddressU32[1] = SNET_BUILD_WORD_FROM_BYTES_PTR_MAC(&newAddressU8[4]);

        if (performChecksumUpdate==GT_TRUE)
        {
            /* calc new checksum */
            newChecksum = snetCalcIncChecksum(oldAddressU32,newAddressU32,origChecksum);
        }
        else
        {
            newChecksum = origChecksum;
        }

        /* Adjust the checksum. */
        if (newChecksum == 0xFFFF)
        {
            newAddressU8[6] = 0;
            newAddressU8[7] = 0;
        }
        else
        {
            newAddressU8[6] = newChecksum >> 8; /* copy the checksum to the new ip address */
            newAddressU8[7] = newChecksum & 0xFF;
        }
    }
    /* NAT ipv6 with a /49 or Longer Prefix */
    else
    {
        /* convert the address to GT_U16 */
        for (i=0; i<8; i++)
        {
            oldAddressU16[i] = SNET_BUILD_HALF_WORD_FROM_BYTES_PTR_MAC(&oldAddressU8[2*i]);
        }

        /* bytePos is the minimal number that meets the following condition:
          (bytePos=64 || bytePos=80 || bytePos= 96 || bytePos=112) && Original Address[bytePos:bytePos+15]!=0xFFFF */
        bytePos = 0;
        for (i=4; i<=7; i++)
        {
            if (oldAddressU16[i] != 0xFFFF)
            {
               bytePos = i;
               break;
            }
        }

        origChecksum = oldAddressU16[bytePos];

        /*  if bytePos doesn't exist the packet is dropped */
        if (bytePos == 0)
        {
            smemRegGet(devObjPtr, regAddr, &counterValue);
            smemRegSet(devObjPtr, regAddr, ++counterValue);
            __LOG(("Incremented NAT Drop Counter Register by 1 from [%d] \n", (counterValue-1)));

            __LOG(("No minimal number that meets the condition so the packet is dropped\n"));
            descrPtr->haAction.drop = SKERNEL_EXT_PKT_CMD_HARD_DROP_E;
        }

        /* prefix size is max 64 bits and bigger then 48 */
        numBitsInField = prefixSize; /* the amount of MS bits to write  */
        byteIndex = 0;
        while (numBitsInField)
        {
            if (numBitsInField > 8)
            {
                /* write whole byte */
                SMEM_U32_SET_FIELD(newAddressU8[byteIndex],/*data*/
                           0, /*offset*/
                           8, /*length*/
                           ipv6NatAddressU8[byteIndex]);/*val*/
                numBitsInField -= 8;
                byteIndex++;

            }
            else
            {
                SMEM_U32_SET_FIELD(newAddressU8[byteIndex],/*data*/
                                   8-numBitsInField, /*offset*/
                                   numBitsInField, /*length*/
                                   ipv6NatAddressU8[byteIndex] >> (8-numBitsInField));/*val*/
                break;
            }

        }

        /* convert the address to U32*/
        newAddressU32[0] = SNET_BUILD_WORD_FROM_BYTES_PTR_MAC(&newAddressU8[0]);
        newAddressU32[1] = SNET_BUILD_WORD_FROM_BYTES_PTR_MAC(&newAddressU8[4]);

        if (performChecksumUpdate==GT_TRUE)
        {
            /* calc new checksum */
            newChecksum = snetCalcIncChecksum(oldAddressU32,newAddressU32,origChecksum);
        }
        else
        {
            newChecksum = origChecksum;
        }

        /* Adjust the checksum. */
        if (newChecksum == 0xFFFF)
        {
            newChecksum = 0;
        }

        /* copy the checksum to the new ip address */
        newAddressU8[bytePos*2] = newChecksum >> 8;
        newAddressU8[bytePos*2+1] = newChecksum & 0xFF;
    }

    return;

}

/**
* @internal haProcessNat66L3HeaderModify function
* @endinternal
*
* @brief   HA - NAT : do ipv6 header modifications.
*
* @param[in] devObjPtr                -  pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in] l3HeaderChangedBuffer    - pointer to ipv6 header
*/
static void  haProcessNat66L3HeaderModify
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    GT_U8    *l3HeaderChangedBuffer/*[40]*/
)
{
#define SIP5_15_NAT_FIELD_GET_MAC(field)\
    SMEM_SIP5_15_HA_NAT66_ENTRY_FIELD_GET(devObjPtr, tunnelStartActionPtr, natIndex, field)

    DECLARE_FUNC_NAME(haProcessNat66L3HeaderModify);

    GT_U32 natIndex = descrPtr->arpPtr;
    GT_U32 *tunnelStartActionPtr;
    GT_U8  tmpBuffer[16]   = {0};
    GT_U32 tmpWords[4];
    GT_U32 i;
    GT_U32 ipv6AddressWords[4] = {0};
    GT_U32  natEntryType;

    if (SMEM_CHT_IS_SIP7_GET(devObjPtr))
    {
        /* the format of the entry as in SIP5 devices, but the entry is in a new ARP table */
        GT_U32  entryNumBits = 192;
        GT_U32  lineNumBits  = 384;
        GT_U32  entriesPerLine = lineNumBits / entryNumBits;
        GT_U32  indexBitResolution =
            lineNumBits / devObjPtr->tablesInfo.arpTable.paramInfo[0].divider;
        GT_U32  indexMult = entryNumBits / indexBitResolution;
        tunnelStartActionPtr = smemMemGet(
            devObjPtr, SMEM_TABLE_ENTRY_INDEX_GET_MAC(devObjPtr, arpTable, (natIndex * indexMult)));
        tunnelStartActionPtr += ((entryNumBits * (natIndex % entriesPerLine)) / 32);
    }
    else
    {
        tunnelStartActionPtr = smemMemGet(
            devObjPtr, SMEM_CHT2_TUNNEL_START_TBL_MEM(devObjPtr,natIndex));
        if(natIndex & 1)
        {
            /* the odd entries are in the second half of the entry (offset of 192 bits = 6 words) */
            tunnelStartActionPtr += 6;
        }
    }

    __LOG(("get NAT entry by index: %d \n", natIndex));

    __LOG(("apply NAT entry to the packet \n"));

    natEntryType = SIP5_15_NAT_FIELD_GET_MAC(SMEM_SIP5_15_HA_NAT66_TABLE_FIELDS_NAT_ENTRY_TYPE);
    if (GT_TRUE == haNatCheckEntryValidity(devObjPtr, descrPtr,natEntryType))
    {
        return;
    }

    /****  layer 3 changes  ****/
    switch (SIP5_15_NAT_FIELD_GET_MAC(SMEM_SIP5_15_HA_NAT66_TABLE_FIELDS_MODIFY_COMMAND))
    {
    case SNET_CHT_HA_MODIFY_COMMAND_MODIFY_SIP_PREFIX_E:

        __LOG(("modify command MODIFY SIP PREFIX \n"));

        /* Ipv6 L3 checksum update */

        /* replace the prefix size MSB of the SIP address with the prefix size MSB of NAT<Address> */
        /* packet SIP applied to buffer here, and not in chtHaUnit */
         snetBobKHaNatIpv6L3ChecksumUpdate(devObjPtr,descrPtr,tunnelStartActionPtr,
                                           GT_TRUE,/*perform checksum-neutral mapping*/
                                           &descrPtr->l3StartOffsetPtr[IPV6_L3_SIP_OFFSET],
                                           &l3HeaderChangedBuffer[IPV6_L3_SIP_OFFSET]);


        __LOG(("packet SIP prefix was changed by NAT \n"));
        break;

    case SNET_CHT_HA_MODIFY_COMMAND_MODIFY_DIP_PREFIX_E:

        __LOG(("modify command MODIFY DIP PREFIX \n"));

        /* Ipv6 L3 checksum update */

        /* replace the prefix size MSB of the DIP address with the prefix size MSB of NAT<Address> */
        /* packet DIP applied to buffer here, and not in chtHaUnit */
         snetBobKHaNatIpv6L3ChecksumUpdate(devObjPtr,descrPtr,tunnelStartActionPtr,
                                           GT_TRUE,/*perform checksum-neutral mapping*/
                                           &descrPtr->l3StartOffsetPtr[IPV6_L3_DIP_OFFSET],
                                           &l3HeaderChangedBuffer[IPV6_L3_DIP_OFFSET]);


        __LOG(("packet DIP prefix was changed by NAT \n"));
        break;

    case SNET_CHT_HA_MODIFY_COMMAND_MODIFY_SIP_ADDRESS_E:

        __LOG(("modify command MODIFY SIP ADRESS \n"));

         /* packet SIP applied to buffer here, and not in chtHaUnit */
        SMEM_SIP5_15_HA_NAT66_ENTRY_FIELD_IPV6_ADDR_GET(devObjPtr, tunnelStartActionPtr, natIndex, ipv6AddressWords);

        /* convert the ip address to network order */
        tmpWords[3] = ipv6AddressWords[0];
        tmpWords[2] = ipv6AddressWords[1];
        tmpWords[1] = ipv6AddressWords[2];
        tmpWords[0] = ipv6AddressWords[3];
        for (i=0; i<4; i++)
        {
            SNET_BUILD_BYTES_FROM_WORD_MAC(tmpWords[i], tmpBuffer);
            memcpy(&l3HeaderChangedBuffer[IPV6_L3_SIP_OFFSET + i*4], tmpBuffer, 4);
        }

        __LOG(("packet SIP was changed by NAT\n"));
        break;

    case SNET_CHT_HA_MODIFY_COMMAND_MODIFY_DIP_ADDRESS_E:

        __LOG(("modify command MODIFY DIP ADDRESS \n"));

        /* packet DIP applied to buffer here, and not in chtHaUnit */
        SMEM_SIP5_15_HA_NAT66_ENTRY_FIELD_IPV6_ADDR_GET(devObjPtr, tunnelStartActionPtr, natIndex, ipv6AddressWords);

        /* convert the ip address to network order */
        tmpWords[3] = ipv6AddressWords[0];
        tmpWords[2] = ipv6AddressWords[1];
        tmpWords[1] = ipv6AddressWords[2];
        tmpWords[0] = ipv6AddressWords[3];
        for (i=0; i<4; i++)
        {
            SNET_BUILD_BYTES_FROM_WORD_MAC(tmpWords[i], tmpBuffer);
            memcpy(&l3HeaderChangedBuffer[IPV6_L3_DIP_OFFSET + i*4], tmpBuffer, 4);
        }

        __LOG(("packet DIP was changed by NAT \n"));
        break;
     case SNET_CHT_HA_MODIFY_COMMAND_MODIFY_SIP_PREFIX_WO_E:

        __LOG(("modify command MODIFY SIP PREFIX WO \n"));

        /* Ipv6 L3 checksum update */

        /* replace the prefix size MSB of the SIP address with the prefix size MSB of NAT<Address> */
        /* packet SIP applied to buffer here, and not in chtHaUnit */
         snetBobKHaNatIpv6L3ChecksumUpdate(devObjPtr,descrPtr,tunnelStartActionPtr,
                                           GT_FALSE,/* without performing checksum-neutral */
                                           &descrPtr->l3StartOffsetPtr[IPV6_L3_SIP_OFFSET],
                                           &l3HeaderChangedBuffer[IPV6_L3_SIP_OFFSET]);


        __LOG(("packet SIP prefix was changed by NAT \n"));
        break;

    case SNET_CHT_HA_MODIFY_COMMAND_MODIFY_DIP_PREFIX_WO_E:

        __LOG(("modify command MODIFY DIP PREFIX WO \n"));

        /* Ipv6 L3 checksum update */

        /* replace the prefix size MSB of the DIP address with the prefix size MSB of NAT<Address> */
        /* packet DIP applied to buffer here, and not in chtHaUnit */
         snetBobKHaNatIpv6L3ChecksumUpdate(devObjPtr,descrPtr,tunnelStartActionPtr,
                                           GT_FALSE,/* without performing checksum-neutral */
                                           &descrPtr->l3StartOffsetPtr[IPV6_L3_DIP_OFFSET],
                                           &l3HeaderChangedBuffer[IPV6_L3_DIP_OFFSET]);


        __LOG(("packet DIP prefix was changed by NAT \n"));
        break;
    default:
        break; /* not a valid option, only 2 bits */
    }

    /* set the length of L3 info */
    descrPtr->tunnelStartRestOfHeaderInfoLen =  descrPtr->ipxHeaderLength*4; /* IPv6 header */

#undef SIP5_15_NAT_FIELD_GET_MAC
}

/**
* @internal sip6_10HaNestedVlanDecision function
* @endinternal
*
* @brief   HA - nested vlan logic
*
* @param[in] devObjPtr                -  pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in] destVlanTaggedPtr        - send frame with tag.
*/
static void  sip6_10HaNestedVlanDecision
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    INOUT GT_U8                    *destVlanTaggedPtr
)
{
    DECLARE_FUNC_NAME(sip6_10HaNestedVlanDecision);

    GT_U32  nested_vlan_en , nested_vlan_mode;

    if(SMEM_CHT_IS_SIP6_30_GET(devObjPtr))
    {
        nested_vlan_mode =
            SMEM_LION3_HA_EPORT_1_ENTRY_FIELD_AUTO_GET(devObjPtr,
                descrPtr,SMEM_SIP6_30_HA_EPORT_TABLE_1_FIELDS_NESTED_VLAN_MODE_E);
    }
    else
    {
        nested_vlan_mode = 0;/* legacy devices not support ingress modes */
    }

    nested_vlan_en =
        SMEM_LION3_HA_EPORT_1_ENTRY_FIELD_AUTO_GET(devObjPtr,
             descrPtr,SMEM_SIP6_10_HA_EPORT_TABLE_1_FIELDS_NESTED_VLAN_EN);

    if(nested_vlan_mode == 0x2 /*IGNORE_INGRESS_NESTED_VLAN*/)
    {
        if(descrPtr->nestedVlanAccessPort)
        {
            __LOG(("Ignore nested vlan indication from the ingress pipe use egress mode [%d] \n",
                nested_vlan_en));
        }

        descrPtr->nestedVlanAccessPort = nested_vlan_en;
    }
    else
    if(nested_vlan_mode == 1/*FORCE_TAG_STATE_UNTAGGED*/)
    {
        if(descrPtr->nestedVlanAccessPort == 1 && nested_vlan_en == 0)
        {
                /* came with ingress indication of nested vlan ,
                   and nested vlan not 'allowed' by the egress port */
            __LOG(("Got nested vlan indication from the ingress pipe and no indication of nested vlan from the egress port \n"));

            __LOG(("force egress untagged : egress vlan tag mode changed from [%d] to [0] (untagged) \n",
                *destVlanTaggedPtr));
            /*we force egress untagged */
            *destVlanTaggedPtr = SKERNEL_FRAME_TR101_VLAN_EGR_TAG_UNTAGGED_E;
        }

        /* do the same logic as mode 0 , regarding 'nested vlan' */
        goto nested_vlan_ingress_or_egress_lbl;
    }
    else
    /*nested_vlan_en == 0 ENABLE_NESTED_VLAN*/
    {
        nested_vlan_ingress_or_egress_lbl:

        if(descrPtr->nestedVlanAccessPort == 0)
        {
            __LOG(("No nested vlan indication from the ingress pipe , so use egress mode [%d] \n",
                nested_vlan_en));
            descrPtr->nestedVlanAccessPort = nested_vlan_en;
        }
        else
        {
            __LOG(("Got nested vlan indication from the ingress pipe , so use it \n"));
        }
    }

    __LOG_PARAM(descrPtr->nestedVlanAccessPort);
}

/**
* @internal chtHaUnit function
* @endinternal
*
* @brief   HA - HA unit logic
*
* @param[in] devObjPtr                -  pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in] egressPort               - egress port (local port on multi-port group device)
*                                      CPU port is port 63
*                                      destPorts       - number of egress port.
* @param[in] destVlanTagged           - send frame with tag.
*/
static void  chtHaUnit
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32   egressPort,
    IN GT_U8    destVlanTagged,
    OUT GT_U8 **frameDataPtrPtr,
    OUT GT_U32 *frameDataSize,
    OUT GT_BOOL *isPciSdma
)
{
    DECLARE_FUNC_NAME(chtHaUnit);

    GT_U32         cascadeHIData = 0;       /* content of header insertion register */
    GT_U32         dsaTagTypeConfigData;/* content of DSA type register address */
    GT_BIT         egrDsaTagEn;     /* DSA tag enabled */
    GT_U32         outputPortBit,outputPortBit1;  /* the bit index for the egress port  */
    GT_BOOL        isSecondRegister;/* is the per port config is in second register -- for bmp of ports */
    GT_U32         prePend2Bytes=0;/* pending two bytes in case the packet is sent to CPU*/
    GT_U32         fldVal;         /* temporary field value */
    GT_U32         vlanEtherType;  /* VLAN EtherType */
    GT_U32         vlanEtherType1 = 0; /* VLAN EtherType1 */
    GT_U32         tunnelStartEtherType = SMAIN_NOT_VALID_CNS;/* VLAN EtherType for tunnel start tag */
    SGT_MAC_ADDR_TYP macAddr;       /* MAC address */
    GT_U8    prePendArr[2] = {0};
    GT_U8    macDaSa[12];
    GT_U32   macDaSaLength=12;
    GT_U8    dsaOrVlanTag[64];  /* 16 words -- the DSA tag may require 4 words , but additional tags for WA */
    GT_U32   dsaOrVlanTagLength;    /* DSA or VLAN (or both) tags length on dsaOrVlanTag */
    GT_U32   dsaTagLength = 0; /* DSA length (not including VLAN tags length) on dsaOrVlanTag */
    GT_U32   replaceDsaWithVlanTagLength = 0;
    GT_U8    *replaceDsaWithVlanTagPtr = NULL;
    GT_U8    replcaeDsaWithVlanTagArr[4];
    GT_U32   firstTagLength;
    GT_U32   secondTagLength;
    GT_U8    timestampTag[16];          /* timestamp tag */
    GT_U32   timestampTagLength = 0;    /* up to 16 bytes */
    GT_U8    analyzerVlanTag[4];
    GT_U32   analyzerVlanTagLength = 0;
    GT_U32   origVlanTagLength = 0;
    GT_U8   *origVlanTagPtr = NULL;
    GT_U8    etherType[2];
    GT_U32   etherTypeLength=2;
    GT_U8    *ethernetEncapsulationPtr;
    GT_U32   ethernetEncapsulationLength;
    GT_U8*   tsEgressBuff = NULL;
    GT_U32   tsEgressBuffLength;
    GT_U32   payloadLen=0;
    GT_U8    *payloadPtr;
    GT_U8    l3HeaderChangedBuffer[MAX_IP_HEADER_SIZE_CNS]; /* max ipv4/6 size*/
    GT_U32   l3HeaderChangedLen = 0;
    GT_U8    l4HeaderChangedBuffer[L4_MAX_CHECKSUM_BYTE_INDEX + CHECKSUM_NUM_BYTES]; /* start of L4 with the checksum */
    GT_U32   l4HeaderChangedLen = 0;
    GT_U8    paddingZeroForLess64Bytes[64]={0};
    GT_U8    paddingZeroForLess64BytesLength;
    GT_U32   totalLength=0;
    GT_U8    *egressBufferPtr;
    GT_BOOL  pcktIsChng;     /* packet is changed in ingress/egress pipe line,
                                till the HA point (include Tunnel Start)*/
    GT_U32   passengerNeedL2Info=0;/* in case of tunnel start , does the passenger needs the L2 (mac da,sa+vlan tag+etherType) info */
    GT_U8    passengerL2MasDaSa[12];/* in case of tunnel start and passengerNeedL2Info = 1 , the mac DA and SA  */
    GT_U32   regAddr;           /* register address */
    GT_U32   *regPtr;           /* register data pointer */
    GT_BIT   allowMplsExpModification;/* allow modifications in the EXP in the most outer MPLS label */
    GT_BIT   checkIfL2isNeeded = 0; /* for tunnel start type MPLS or IPv4/IPv4_GRE check if the passenger needs the L2 info */
    GT_U32   tmpPort;/*temp port*/
    GT_U32   offset;/* offset of sections in the packet (from start of packet)*/
    GT_U32   tag0OffsetInTag = SMAIN_NOT_VALID_CNS;/*tag 0 offset in the dsaOrVlanTag[] */
    GT_U32   tag1OffsetInTag = SMAIN_NOT_VALID_CNS;/*tag 1 offset in the dsaOrVlanTag[] */
    HA_INTERNAL_INFO_STC    haInfo;
    GT_U8    evbBpeExtraTag[4*2];/* up to 2 tags - for EVB */
    GT_U8   *macDaSaPtr = NULL;/* pointer to mac addresses DA,SA (in case of TS ETH-o-X this is of the passenger) */
    GT_U32  extraLength;/* length to add into the ipv4 header to cover the L2 info of eth-o-ipv4 TS */
    GT_U8    passengerVlanTagMode;/* tagging mode of the passenger (relevant when TS of ETH-over-X )*/
    GT_U8    passengerVlanTag[4 * 3] = {0};/* 3 words -- of vlan tags for the passenger only  */
    GT_U32   passengerVlanTagLength = 0;/* number of bytes in the passengerVlanTag[] to use */
    GT_U32   passengerTag0OffsetInTag = SMAIN_NOT_VALID_CNS;/*tag 0 offset in the passengerVlanTag[] */
    GT_U32   passengerTag1OffsetInTag = SMAIN_NOT_VALID_CNS;/*tag 1 offset in the passengerVlanTag[] */
    GT_BOOL  needOriginalInfo;/* we need original packet for rx_anlyzer and to_cpu_from_ingress*/
    GT_BOOL  useDataAfterOuterTagAsData = GT_FALSE;/*use inner tag (if exists) after the outer tag as data ?
                                                    meaning not subject to TPID modifications / tag modifications */
    GT_BIT  tag1_exists, tag1_is_outer;
    GT_BIT  bpeTag1ExistMode;
    GT_BOOL isL2Nat = GT_FALSE; /* flag to indicate whether L2NAT functionality is enabled/disabled */
    GT_BOOL updateTtl = GT_FALSE;
    GT_BOOL mtagTxSniffOrToCpu;
    GT_U32  afterDsaTagLength; /* length of tags already added after DSA one, does not include TR101 tags */

    egressBufferPtr = devObjPtr->egressBuffer;

    memset(&haInfo,0,sizeof(haInfo));

    haInfo.tag0Length = 4;/* default for BWC code */
    haInfo.tag1Length = 4;/* default for BWC code */

    haInfo.evbBpeTagPtr = &evbBpeExtraTag[0];

    if (devObjPtr->support802_1br_PortExtender)
    {
        smemRegFldGet(devObjPtr,
                                SMEM_XCAT3_HA_BPE_INTERNAL_CONFIG_REG (devObjPtr), 1, 1, &bpeTag1ExistMode);
        if (bpeTag1ExistMode == 0)
        {
            tag1_is_outer  = SMEM_U32_GET_FIELD(descrPtr->vid0Or1AfterTti,2,1);
            tag1_exists  = SMEM_U32_GET_FIELD(descrPtr->vid0Or1AfterTti,3,1);
        }
        else
        {
            tag1_is_outer = (descrPtr->ingressVlanTag1Type == SKERNEL_FRAME_TR101_VLAN_TAG_OUTER_E) ? 1 : 0;
            tag1_exists  = TAG1_EXIST_MAC(descrPtr);

        }

        descrPtr->haToEpclInfo.bpe_802_1br_ingressInfo.tag1_exists = tag1_exists;
        descrPtr->haToEpclInfo.bpe_802_1br_ingressInfo.tag1_is_outer = tag1_is_outer;
    }

    if (descrPtr->outGoingMtagCmd == SKERNEL_MTAG_CMD_FORWARD_E &&
        descrPtr->modifyDscp)/* modify DSCP / EXP */
    {
        if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
        {
            smemRegFldGet(devObjPtr,
                SMEM_LION2_HA_QOS_PROFILE_TO_EXP_TBL_MEM(devObjPtr,descrPtr->qos.qosProfile),
                0,3,
                &descrPtr->exp[0]);
        }
        else
        {
            /* use the QOS profile index to get the EXP */
            fldVal = 3 * (descrPtr->qos.qosProfile % 8);
            smemRegFldGet(devObjPtr,
                            SMEM_XCAT_HA_QOS_PROFILE_TO_EXP_REG(devObjPtr) +
                            4 * (descrPtr->qos.qosProfile / 8),
                            fldVal, 3, &descrPtr->exp[0]);
        }
    }

    if((descrPtr->outGoingMtagCmd == SKERNEL_MTAG_CMD_TO_CPU_E && descrPtr->srcTrg == 0)/*Src*/ ||
       (descrPtr->outGoingMtagCmd == SKERNEL_MTAG_CMD_TO_TRG_SNIFFER_E && descrPtr->rxSniff) )
    {
        /* packet that goes to CPU (from ingress pipe) should have its original payload ,
           packet that goes to analyzer (rxMirror) should have its original payload */
        needOriginalInfo = GT_TRUE;
    }
    else
    {
        needOriginalInfo = GT_FALSE;
    }

    if(SMEM_CHT_IS_SIP6_10_GET(devObjPtr))
    {
        sip6_10HaNestedVlanDecision(devObjPtr,descrPtr,&destVlanTagged);
    }

    /* Use original packet length in HA */
    descrPtr->byteCount = descrPtr->origByteCount;

    offset = 0;

    /* LSR label operations */
    if (descrPtr->isMplsLsr)
    {
        if(descrPtr->tunnelTerminationOffset >= (descrPtr->l3StartOffsetPtr - descrPtr->startFramePtr))
        {
            offset = descrPtr->tunnelTerminationOffset - (descrPtr->l3StartOffsetPtr - descrPtr->startFramePtr);
        }
        else
        {
            skernelFatalError("chtHaUnit: descrPtr->tunnelTerminationOffset %d cannot be less than\n"
                              "(descrPtr->l3StartOffsetPtr - descrPtr->startFramePtr) %d",
                              descrPtr->tunnelTerminationOffset,
                              (descrPtr->l3StartOffsetPtr - descrPtr->startFramePtr));
        }
    }
    /* modifyDscp/EXP for LSR packets . SIP5 allows EXP modification in bridge MPLS as well */
    allowMplsExpModification = (SMEM_CHT_IS_SIP5_GET(devObjPtr)) ?  1 : descrPtr->isMplsLsr;

    if(allowMplsExpModification &&
       descrPtr->mpls &&
       devObjPtr->errata.mplsNonLsrNotModifyExp &&
       descrPtr->isMplsLsr == 0  &&
       descrPtr->modifyDscp)
    {
        __LOG(("the device not supports to modify EXP for MPLS that are non-LSR packets \n"));
        allowMplsExpModification = 0;
    }

    descrPtr->payloadLength -= offset;
    descrPtr->payloadPtr    += offset;
    descrPtr->mplsLsrOffset  = offset;

    descrPtr->l3StartOffsetPtr = descrPtr->payloadPtr;

    /* HA - get info about the need to do tunnel start and Arp modifications:
       1. tunnel start + TS pointer
       2. mac DA modification + ARP pointer
       3. mac SA modification
    */
    snetChtHaTsArpPointerInfo(devObjPtr,descrPtr,egressPort,needOriginalInfo,&haInfo);

    if(haInfo.tunnelStart)
    {
        __LOG(("Build the tunnel Start \n"));

        if ((haInfo.tunnelStartPassengerType != 0/*not ethernet*/)&&(descrPtr->isIp==GT_FALSE)&&(descrPtr->mpls==GT_FALSE))
        {
            __LOG(("WARNING: Misconfiguration - Passenger Type is not Ethernet BUT packet is not IP nor MPLS\n"));
        }

        if(SMEM_CHT_IS_SIP7_GET(devObjPtr))
        {
            if (descrPtr->hft0Triggered)
            {
                /* HFT_0 handling */
                sip7HaHft(devObjPtr, descrPtr, SMEM_SIP7_HA_HFT_TABLE_0_E, &haInfo, egressPort);
            }
            if (descrPtr->hft1Triggered)
            {
                /* HFT_1 handling */
                sip7HaHft(devObjPtr, descrPtr, SMEM_SIP7_HA_HFT_TABLE_1_E, &haInfo, egressPort);
            }
        }
        /* we need to take tunnel start parameters */
        if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
        {
            snetLion3HaTunnelStart(devObjPtr,descrPtr,&haInfo,egressPort);
        }
        else
        {
            snetCht2HaTunnelStart(devObjPtr,descrPtr,&haInfo,egressPort);
        }

        /* for tunnel start type MPLS or IPv4/IPv4_GRE check if the passenger needs the L2 info */
        if(descrPtr->tunnelStartType == SKERNEL_FRAME_TUNNEL_START_TYPE_MPLS_E)
        {
            checkIfL2isNeeded = 1;
        }
        else if (descrPtr->tunnelStartType == SKERNEL_FRAME_TUNNEL_START_TYPE_IPV4_E &&
            devObjPtr->supportTunnelStartEthOverIpv4)
        {
            checkIfL2isNeeded = 1;
        }
        else if(descrPtr->tunnelStartType == SKERNEL_FRAME_TUNNEL_START_TYPE_TRILL_E)
        {
            if(descrPtr->tunnelStartTrillTransit == 0)
            {
                /*native frame that go to tunnel */
                __LOG(("native frame that go to tunnel"));
                checkIfL2isNeeded = 1;
            }
        }
        else if(SMEM_CHT_IS_SIP5_GET(devObjPtr) && haInfo.tunnelStartPassengerType == 0/*ethernet over X*/)
        {
            checkIfL2isNeeded = 1;
        }

        if(checkIfL2isNeeded)
        {
            /* tunnelStartPassengerType: 0x0 = Ethernet, 0x1 = Other */
            __LOG(("tunnelStartPassengerType: 0x0 = Ethernet, 0x1 = Other"));
            passengerNeedL2Info = (haInfo.tunnelStartPassengerType == 0) ? 1 : 0;
        }
        else if (descrPtr->tunnelStartType == SKERNEL_FRAME_TUNNEL_START_TYPE_MIM_E && (SMEM_CHT_IS_SIP5_GET(devObjPtr) == 0))
        {
            /* MIM tunneled packets always keep layer 2 of the original packet */
            __LOG(("MIM tunneled packets always keep layer 2 of the original packet"));
            passengerNeedL2Info = 1;
        }

    }

    /* Egress VLAN Translation  */
    snetChtHaVlanTranslate(devObjPtr, descrPtr, egressPort);

    /* convert local port to register offset and to bit offset */
    snetChtHaPerPortInfoGet(devObjPtr,egressPort,&isSecondRegister,&outputPortBit);

    if (egressPort == SNET_CHT_CPU_PORT_CNS && /* send packet to cpu */
        devObjPtr->supportHa64Ports == 0)
    {
        /* get the cpu port bit index */
        __LOG(("get the cpu port bit index"));
        outputPortBit1 = SMEM_CHT_GET_CPU_PORT_BIT_INDEX_IN_CSCD_STATUS(devObjPtr);
    }
    else
    {
        outputPortBit1 = outputPortBit;
    }

    if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        /* Cascade port enable */
        egrDsaTagEn =
            SMEM_LION3_HA_PHYSICAL_PORT_2_ENTRY_FIELD_AUTO_GET(devObjPtr,
                descrPtr,
                SMEM_LION3_HA_PHYSICAL_PORT_TABLE_2_FIELDS_CASCADE_PORT_ENABLE);
    }
    else
    {
        regAddr = SMEM_CHT_CAS_HDR_INS_REG(devObjPtr);
        if(isSecondRegister == GT_TRUE)
        {
            regAddr +=4;
        }
        smemRegGet(devObjPtr, regAddr, &cascadeHIData );
        egrDsaTagEn = SMEM_U32_GET_FIELD(cascadeHIData,outputPortBit1,1);
    }

    if(egrDsaTagEn)
    {
        __LOG(("Cascade port enable , on port [%d]",
            egressPort));
    }

    if(egrDsaTagEn == 0)
    {
        descrPtr->egrMarvellTagType = MTAG_TYPE_NONE_E;
    }
    else
    {
        if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
        {
            /* Egress DSA tag type */
            descrPtr->egrMarvellTagType =
                SMEM_LION3_HA_PHYSICAL_PORT_2_ENTRY_FIELD_AUTO_GET(devObjPtr,
                    descrPtr,
                    SMEM_LION3_HA_PHYSICAL_PORT_TABLE_2_FIELDS_EGRESS_DSA_TAG_TYPE);
        }
        else
        {
            regAddr = SMEM_CHT_DSA_TAG_TYPE_REG(devObjPtr);
            if(isSecondRegister == GT_TRUE)
            {
                regAddr +=4;
            }

            smemRegGet(devObjPtr, regAddr, &dsaTagTypeConfigData );

            descrPtr->egrMarvellTagType = ((dsaTagTypeConfigData & (1 << outputPortBit1)) == 0) ?
                                MTAG_STANDARD_E : MTAG_EXTENDED_E;
        }

        __LOG(("Egress DSA tag type [%s] , on port[%d] \n",
            descrPtr->egrMarvellTagType == MTAG_STANDARD_E ? "MTAG_STANDARD_E" :
            descrPtr->egrMarvellTagType == MTAG_EXTENDED_E ? "MTAG_EXTENDED_E" :
            descrPtr->egrMarvellTagType == MTAG_3_WORDS_E ? "MTAG_3_WORDS_E" :
            descrPtr->egrMarvellTagType == MTAG_4_WORDS_E ? "MTAG_4_WORDS_E" :
            "unknown",
            egressPort
            ));
    }


    /* Egress Tag0 VLAN and Tag1 VLAN ethertype */
    __LOG(("Egress Tag0 VLAN and Tag1 VLAN ethertype"));
    snetXCatHaEgressTagEtherType(devObjPtr, descrPtr,egressPort , &haInfo, &vlanEtherType,
                               &vlanEtherType1 , &tunnelStartEtherType);

    if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        /* Pre-Pend Two Bytes Header Enable */
        prePend2Bytes =
            SMEM_LION3_HA_PHYSICAL_PORT_2_ENTRY_FIELD_AUTO_GET(devObjPtr,
                descrPtr,
                SMEM_LION3_HA_PHYSICAL_PORT_TABLE_2_FIELDS_PRE_PEND_TWO_BYTES_HEADER_EN);
    }
    else
    if (egressPort == SNET_CHT_CPU_PORT_CNS) /* send packet to cpu */
    {
        if(SKERNEL_DEVICE_FAMILY_LION2_PORT_GROUP_DEV(devObjPtr))
        {
            /* from Lion2 the bit is in TXDMA and not in the HA */
            smemRegFldGet(devObjPtr,
                SMEM_LION2_TXDMA_ENGINE_CONFIG_BUFFER_MEMORY_MISC_CONFIG_REG(devObjPtr),
                26 , 1 , &prePend2Bytes);
        }
        else
        if(0 == SKERNEL_IS_LION_REVISON_B0_DEV(devObjPtr))
        {
            prePend2Bytes = (cascadeHIData >> 28) & 0x1;
        }
        else
        {
            smemRegFldGet(devObjPtr, SMEM_CHT3_ROUTE_HA_GLB_CNF_REG(devObjPtr),15 , 1 , &prePend2Bytes);
        }
    }

    /* check if SDMA bus is enabled ? */
    *isPciSdma  = devObjPtr->isPciCompatible;

    descrPtr->haToEpclInfo.prePendLength = prePend2Bytes ? 2 : 0;

    /* save the tagging mode of the passenger */
    passengerVlanTagMode = destVlanTagged;
    __LOG(("use passengerVlanTagMode[%d] \n",
        passengerVlanTagMode));

    /* check if need to modify the ingress buffer -- before sending to egress */
    if (descrPtr->egrMarvellTagType != MTAG_TYPE_NONE_E)
    {
        pcktIsChng = snetChtHaIsMtagPacketChanged(descrPtr);
        __LOG_PARAM(pcktIsChng);

        if(haInfo.tunnelStart)
        {
            destVlanTagged = haInfo.tsVlanTagInfo.vlanTagged ?
                                1 : /*SKERNEL_FRAME_TR101_VLAN_EGR_TAG_TAG0_E*/
                                0;  /*SKERNEL_FRAME_TR101_VLAN_EGR_TAG_UNTAGGED_E*/

            __LOG(("haInfo.tunnelStart == 1 : the tunnel header use [%s] egress tag state (passenger may have it's own tag0,1) \n",
                destVlanTagged ? "TAGGED" : "UNTAGGED"));
        }
        if(descrPtr->packetCmd == SKERNEL_EXT_PKT_CMD_FROM_CPU_E &&
           descrPtr->useVidx == 0)
        {
            /*the info after the orig DSA is data*/
            useDataAfterOuterTagAsData = GT_TRUE;
        }

        if (descrPtr->outGoingMtagCmd == SKERNEL_MTAG_CMD_TO_TRG_SNIFFER_E)
        {
            /* take info from the DSA tag about the needed egress tag state */
            destVlanTagged =
                chtHaTargetAnalyzerTagModeGet(devObjPtr,descrPtr,
                    destVlanTagged,isSecondRegister,outputPortBit);
        }
    }
    else
    {
        if(descrPtr->basicMode == 1)
        {
            /* should egress the same way that was ingress*/
            destVlanTagged = descrPtr->tagSrcTagged[SNET_CHT_TAG_0_INDEX_CNS] ? 1 : 0;
            __LOG(("descrPtr->basicMode : packet should egress the same way that was ingress , use destVlanTagged[%d] \n",
                destVlanTagged));
        }

        if(descrPtr->packetCmd == SKERNEL_EXT_PKT_CMD_FROM_CPU_E &&
           descrPtr->useVidx == 0)
        {
            destVlanTagged = descrPtr->trgTagged;
            /*the info after the orig DSA is data*/
            useDataAfterOuterTagAsData = GT_TRUE;
            __LOG(("FROM_CPU : use destVlanTagged[%d] from descrPtr->trgTagged \n",
                destVlanTagged));

            /* If FROM_CPU<UseVIDX>=0
               <trgTagged> determines if the FROM_CPU eDSA is stripped or converted to VLAN tag.
               Same as in 8B DSA */
            if(destVlanTagged)
            {
                __LOG(("The FROM_CPU DSA is converted to VLAN tag \n"));
            }
            else
            {
                __LOG(("The FROM_CPU DSA is stripped \n"));
            }
        }

        if (descrPtr->outGoingMtagCmd == SKERNEL_MTAG_CMD_TO_TRG_SNIFFER_E)
        {
            /* take info from the DSA tag about the needed egress tag state */
            destVlanTagged =
                chtHaTargetAnalyzerTagModeGet(devObjPtr,descrPtr,
                    destVlanTagged,isSecondRegister,outputPortBit);
        }

        /* save the tagging mode of the passenger (used only on ETH-o-X passenger of TS)*/
        passengerVlanTagMode = destVlanTagged;

        __LOG(("use passengerVlanTagMode[%d] \n",
            passengerVlanTagMode));

        if(haInfo.tunnelStart)
        {
            destVlanTagged = haInfo.tsVlanTagInfo.vlanTagged ?
                                1 : /*SKERNEL_FRAME_TR101_VLAN_EGR_TAG_TAG0_E*/
                                0;  /*SKERNEL_FRAME_TR101_VLAN_EGR_TAG_UNTAGGED_E*/

            __LOG(("haInfo.tunnelStart == 1 : the tunnel header use [%s] egress tag state (passenger may have it's own tag0,1) \n",
                destVlanTagged ? "TAGGED" : "UNTAGGED"));
        }

        pcktIsChng = snetChtHaIsPacketChanged(descrPtr, destVlanTagged,
                                            vlanEtherType, vlanEtherType1);
        __LOG_PARAM(pcktIsChng);
    }

    /* calc tr101 final egress tag state of tag0,1 */
    chtHaCalcTr101FinalState(devObjPtr,descrPtr,egressPort,
        destVlanTagged,passengerVlanTagMode,
        passengerNeedL2Info);

    if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        __LOG(("check for EVB/BPE tags \n"));
        snetChtHaEgressEPortEvbBpeTag(devObjPtr,descrPtr,egressPort,&haInfo);
        if(haInfo.evbBpeTagLength)
        {
            if(pcktIsChng == 0)
            {
                __LOG(("Packet considered changed due to adding EVB/ETag \n"));
            }

            pcktIsChng = 1;
        }
    }
    else if(devObjPtr->support802_1br_PortExtender)
    {
        SKERNEL_PORTS_BMP_STC trgPortsBmp; /* Target ports bitmap */
        /* Get Target Port <BPE enabled> per port */
        GT_U32  *regPtr = smemMemGet(devObjPtr, SMEM_XCAT3_HA_PER_PORT_BPE_ENABLE_REG(devObjPtr));
        GT_BIT  trgPortBpeEnable;

        SKERNEL_FILL_PORTS_BITMAP_MAC(devObjPtr, &trgPortsBmp, regPtr);
        trgPortBpeEnable = SKERNEL_PORTS_BMP_IS_PORT_SET_MAC(&trgPortsBmp, egressPort);

        if (trgPortBpeEnable)
        {
            if(pcktIsChng == 0)
            {
                __LOG(("Packet considered changed due to (possible) setting ETag \n"));
            }

            pcktIsChng = 1;
        }

    }

    if(pcktIsChng == 0)
    {
        /* packet not changed , use original frame */
        haBuildPacketNonChanged(devObjPtr,descrPtr,egressPort,egressBufferPtr,
            frameDataPtrPtr,frameDataSize);
        return ;
    }

    /*packet changed, so it need calculate crc */
    __LOG(("packet changed, so it need calculate crc"));
    descrPtr->calcCrc = GT_TRUE;

    /*1.  pre pend 2 bytes if needed*/
    totalLength += descrPtr->haToEpclInfo.prePendLength;

    if(descrPtr->useArpForMacSa == GT_TRUE)
    {
        /* to support the 'needOriginalInfo == GT_TRUE' and for the need to use the MAC SA from the ARP table */
        /* restore the mac SA to the original place on the packet , before modified by the IPCL */
        descrPtr->macSaPtr = descrPtr->origMacSaPtr;
    }

    /*passengerL2MasDaSa[] relevant only when passengerNeedL2Info == 1 */
    memcpy(&passengerL2MasDaSa[0],descrPtr->macDaPtr,6);
    memcpy(&passengerL2MasDaSa[6],descrPtr->macSaPtr,6);

    /*2.  build mac addresses -- in buffer
          set length of mac addresses*/

    if(haInfo.tunnelStart)
    {
        /* copy from the tunnelStart buffer */
        __LOG(("copy from the tunnelStart buffer"));
        memcpy(&macDaSa[0],descrPtr->tunnelStartMacInfoPtr,12);
    }
    else
    {
        memcpy(&macDaSa[0],descrPtr->macDaPtr,6);
        memcpy(&macDaSa[6],descrPtr->macSaPtr,6);
    }

    if (needOriginalInfo == GT_FALSE)
    {
        macDaSaPtr = (passengerNeedL2Info)?&passengerL2MasDaSa[0]:&macDaSa[0];

        /* DA modification */
        if(haInfo.doRouterHa)
        {
            /* IP router or MPLS 'route' : MAC sa,da setting , to non-TS interface
                in EArch this can be for TS too.*/
            if ((0 == SMEM_CHT_IS_SIP5_GET(devObjPtr)) || /* the next check not relevant to non sip5 */
                (haInfo.tunnelStart == 0) || /* for non TS egress packet */
                 (passengerNeedL2Info == 1) ) /* for passenger of TS that need MacDa modification */
            {
                /* Copy DA MAC address */
                if((descrPtr->isIp == 0) || descrPtr->ipm == 0)
                {
                    if(descrPtr->isNat)
                    {
                        __LOG(("for NAT use 'NAT entry' as 'ARP entry' (index[%d])! , the NAT entry starts with the needed MAC DA !",
                            haInfo.arpPointer));
                    }

                    /* Get destination ARP address */
                    snetChtHaArpTblEntryGet(devObjPtr, haInfo.arpPointer , &macAddr);
                    descrPtr->haToEpclInfo.useArpForDa = GT_TRUE;

                    memcpy(&macDaSaPtr[0],macAddr.bytes,6);
                }
                else
                {
                    /* Multicast Routed MAC DA Modification */
                    if(descrPtr->isIPv4)
                    {
                        /* IPv4 packet */
                        __LOG(("IPv4 packet"));
                        macDaSaPtr[0] = 0x01;
                        macDaSaPtr[1] = 0x00;
                        macDaSaPtr[2] = 0x5E;
                       /* Least significant 23 bits of the DIP are mapped
                        to the 23 least significant bits of the 48-bit MAC address: */
                        macDaSaPtr[3] = (descrPtr->dip[0] >> 16) & 0x7f;
                        macDaSaPtr[4] = (descrPtr->dip[0] >> 8) & 0xff;
                        macDaSaPtr[5] = (descrPtr->dip[0]) & 0xff;
                    }
                    else
                    {
                        /* IPv6 packet */
                        __LOG(("IPv6 packet"));
                        macDaSaPtr[0] = 0x33;
                        macDaSaPtr[1] = 0x33;
                       /* Least significant 32 bits of the DIP are mapped
                       to the 32 least significant bits of the 48-bit MAC address: */
                        macDaSaPtr[2] = (descrPtr->dip[3] >> 24) & 0xff;
                        macDaSaPtr[3] = (descrPtr->dip[3] >> 16) & 0xff;
                        macDaSaPtr[4] = (descrPtr->dip[3] >> 8) & 0xff;
                        macDaSaPtr[5] = (descrPtr->dip[3]) & 0xff;
                    }
                }

            }

        }

        /* SA modification */
        if(descrPtr->useArpForMacSa == GT_TRUE)
        {
            /* Get Source ARP address */
            snetChtHaArpTblEntryGet(devObjPtr, haInfo.arpPointer , &macAddr);
            descrPtr->haToEpclInfo.useArpForSa = GT_TRUE;

            memcpy(&macDaSaPtr[6],macAddr.bytes,6);
        }

        if(!passengerNeedL2Info)
        {
            /* perform MAC swap for L2 echo after router DA, SA HA.
               because router HA has precedence over L2 echo. */
            if (descrPtr->VntL2Echo)
            {
                /* Layer 2 Echo packets are transmitted with their MAC DA and MAC SA swapped */
                __LOG(("Layer 2 Echo packets are transmitted with their MAC DA and MAC SA swapped"));
                memcpy(&macDaSaPtr[0],descrPtr->macSaPtr,6);
                memcpy(&macDaSaPtr[6],descrPtr->macDaPtr,6);
            }
        }

        /* SA modification */
        /* allowed on ETH passenger when TS or when no TS */
        if(haInfo.routed &&
           (passengerNeedL2Info || haInfo.tunnelStart == 0))
        {
            if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
            {
                /*Enable MAC SA modification for routed packets*/
                fldVal =
                    SMEM_LION3_HA_PHYSICAL_PORT_1_ENTRY_FIELD_AUTO_GET(devObjPtr,
                        descrPtr,
                        SMEM_LION3_HA_PHYSICAL_PORT_TABLE_1_FIELDS_ROUTED_MAC_SA_MOD_EN);
            }
            else
            {
                regAddr = SMEM_CHT_ROUT_HA_MAC_SA_EN_REG(devObjPtr);
                if(isSecondRegister == GT_TRUE)
                {
                    regAddr +=4;
                }

                /* Enable Routed packets MAC SA Modification */
                smemRegFldGet(devObjPtr, regAddr , outputPortBit , 1,& fldVal);
            }

            /* Per Egress port bit */
            if (fldVal)
            {
                __LOG(("Enable Routed packets MAC SA Modification \n"));

                if(descrPtr->takeMacSaFromTunnel == GT_FALSE)
                {
                    __LOG((" Take passenger MAC SA  based on  target ePort \n"));

                    /* Copy SA MAC address */
                    snetChtHaMacFromMeBuild(devObjPtr,descrPtr,egressPort,GT_FALSE,&macDaSaPtr[6]);
                }
                else
                {
                    __LOG((" Take passenger MAC SA from tunnel Mac SA \n"));

                    memcpy(&macDaSaPtr[6],&descrPtr->tunnelStartMacInfoPtr[6],6);
                }
            }
        }
    }
    else
    {
        /* macDaSa already set by TS logic. */
        if(haInfo.tunnelStart == 0)
        {
            memcpy(&macDaSa[0],descrPtr->origInfoBeforeTunnelTermination.originalL2Ptr,12);
        }
        else
        {
            __LOG(("Use TS macDa and macSA \n"));
        }
    }

    totalLength += macDaSaLength;

    if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        /* PATCH for sip 5 for egress purposes treat as if tag0 or tag 1*/
         /* for the forceNewDsaToCpu issues */
       if(needOriginalInfo == GT_TRUE && descrPtr->origSrcTagState != SKERNEL_FRAME_TR101_VLAN_INGR_TAG_UNTAGGED_E)
       {
            descrPtr->origSrcTagged = 1;
       }
       else
       if(needOriginalInfo == GT_FALSE && descrPtr->srcTagState != SKERNEL_FRAME_TR101_VLAN_INGR_TAG_UNTAGGED_E)
       {
            descrPtr->origSrcTagged = 1;
       }
    }

    if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        lion3HaPrepareDsaAndTagsInfo(devObjPtr, descrPtr);
    }

    /* init variable */
    afterDsaTagLength = 0;

    /*3. set DSA/vlan tag*/
    if (descrPtr->egrMarvellTagType != MTAG_TYPE_NONE_E)
    {
        snetChtHaCreateDsaTag(devObjPtr, descrPtr, egressPort, &haInfo, dsaOrVlanTag);

        /* Multi-Port Group FDB Lookup support */
        if(devObjPtr->supportMultiPortGroupFdbLookUpMode)
        {
            __LOG(("Multi-Port Group FDB Lookup support \n"));
            snetLionHaEgressMarvellTagSourcePortGroupId(devObjPtr, descrPtr, egressPort, dsaOrVlanTag);
        }

        dsaOrVlanTagLength = (descrPtr->egrMarvellTagType + 1) * 4;

        if(descrPtr->forceToAddFromCpu4BytesDsaTag)
        {
            if(descrPtr->outGoingMtagCmd == SKERNEL_MTAG_CMD_TO_TRG_SNIFFER_E)
            {
                /* Create the remote analyzer VLAN tag if needed */
                __LOG(("Create the remote analyzer VLAN tag if needed"));
                snetChtHaCreateAnalyzerVlanTag(devObjPtr, descrPtr, &dsaOrVlanTag[dsaOrVlanTagLength], &afterDsaTagLength);
                dsaOrVlanTagLength += afterDsaTagLength;
                /* we added the analyzer vlan after the DSA */
                analyzerVlanTagLength = 0;
            }

            if(haInfo.tunnelStart && haInfo.tsVlanTagInfo.vlanTagged)
            {
                __LOG(("build TS vlan tag \n"));
                if(tunnelStartEtherType != SMAIN_NOT_VALID_CNS)
                {
                    haInfo.tsVlanTagInfo.vlanEtherType = tunnelStartEtherType;
                }
                else
                {
                    haInfo.tsVlanTagInfo.vlanEtherType = vlanEtherType;
                }

                /* the TS vlan tag info is NOT in :
                   descrPtr->up , descrPtr->eVid , descrPtr->cfidei */
                snetChtHaEgressTagDataExtGet(haInfo.tsVlanTagInfo.up,
                        haInfo.tsVlanTagInfo.vlanId,
                        haInfo.tsVlanTagInfo.cfi,
                        (GT_U16)haInfo.tsVlanTagInfo.vlanEtherType,
                        &dsaOrVlanTag[dsaOrVlanTagLength]);
                dsaTagLength = (descrPtr->egrMarvellTagType + 1) * 4;
                dsaOrVlanTagLength += 4;
                afterDsaTagLength += 4;
                /* we added the TS vlan tag after the DSA */
            }
            else if (destVlanTagged)
            {
                /* in case of <force 4B FROM_CPU> DSA tag doesn't replace vlan tag0
                 * but is placed before it. So calculate vlan tag too. */
                snetChtHaEgressTagDataExtGet((GT_U8)descrPtr->up, (GT_U16)descrPtr->haAction.vid0,
                                             (GT_U8)descrPtr->cfidei, (GT_U16)vlanEtherType,
                                             &dsaOrVlanTag[dsaOrVlanTagLength]);

                if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
                {
                    /* check if need to build egress tag0 longer than 4 bytes */
                    snetHaSip5EgressTagAfter4BytesExtension(devObjPtr,
                                                descrPtr,&haInfo,GT_TRUE/*tag0*/,
                                                &dsaOrVlanTag[dsaOrVlanTagLength]);
                    /* TBD : where the extra bytes after the first 4 bytes , is considered ? */
                }

                dsaTagLength = (descrPtr->egrMarvellTagType + 1) * 4;
                dsaOrVlanTagLength += dsaTagLength;

            }
         }
    }
    else
    {
        if(descrPtr->outGoingMtagCmd == SKERNEL_MTAG_CMD_TO_TRG_SNIFFER_E)
        {
            /* Create the remote analyzer VLAN tag if needed */
            __LOG(("Create the remote analyzer VLAN tag if needed"));
            snetChtHaCreateAnalyzerVlanTag(devObjPtr, descrPtr, analyzerVlanTag, &analyzerVlanTagLength);
            totalLength += analyzerVlanTagLength;
        }

        if (destVlanTagged)
        {
            /* Signaling GEM Port ID Through VLAN Tag(DSA tag is disabled in both this device and the OLT chipset) */
            if(devObjPtr->supportGemUserId)
            {
                regAddr = SMEM_LION_HA_REPLACE_VID_WITH_USER_ID_REG(devObjPtr);
                if(isSecondRegister == GT_TRUE)
                {
                    regAddr += 4;
                }
                regPtr = smemMemGet(devObjPtr, regAddr);

                /* PON WT-156 standard. If enabled, the VID in the outer VLAN tag is replaced by the User ID (GEM port ID) */
                fldVal = SMEM_U32_GET_FIELD(*regPtr, outputPortBit, 1);

                tmpPort = descrPtr->trgEPort;

                switch(fldVal)
                {
                    case 1:
                        __LOG(("PON WT-156 standard. If enabled, the VID in the outer VLAN tag is replaced by the User ID (GEM port ID)"));
                        descrPtr->haToEpclInfo.xPonVid =
                            (descrPtr->trgDev & 0x1) << 11 |
                            SMEM_U32_GET_FIELD(tmpPort,0,5) << 5 |
                            SMEM_U32_GET_FIELD(descrPtr->sstId,0,5);
                        descrPtr->haToEpclInfo.xponChanges = 1;
                        break;
                    default:
                        break;
                }

                if(descrPtr->useVidx)
                {
                    descrPtr->haToEpclInfo.xponChanges = 0;
                }
            }

            dsaOrVlanTagLength = 4;
            if(haInfo.tunnelStart && haInfo.tsVlanTagInfo.vlanTagged)
            {
                if(tunnelStartEtherType != SMAIN_NOT_VALID_CNS)
                {
                    haInfo.tsVlanTagInfo.vlanEtherType = tunnelStartEtherType;
                }
                else
                {
                    haInfo.tsVlanTagInfo.vlanEtherType = vlanEtherType;
                }

                /* the TS vlan tag info is NOT in :
                   descrPtr->up , descrPtr->eVid , descrPtr->cfidei */
                snetChtHaEgressTagDataExtGet(haInfo.tsVlanTagInfo.up,
                        haInfo.tsVlanTagInfo.vlanId,
                        haInfo.tsVlanTagInfo.cfi,
                        (GT_U16)haInfo.tsVlanTagInfo.vlanEtherType,
                        dsaOrVlanTag);
            }
            else
            {
                snetChtHaEgressTagDataExtGet((GT_U8)descrPtr->up, (GT_U16)descrPtr->haAction.vid0,
                      (GT_U8)descrPtr->cfidei, (GT_U16)vlanEtherType, dsaOrVlanTag);
                if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
                {
                    /* check if need to build egress tag0 longer than 4 bytes */
                    snetHaSip5EgressTagAfter4BytesExtension(devObjPtr,
                                                descrPtr,&haInfo,GT_TRUE/*tag0*/,
                                                dsaOrVlanTag);
                }
                dsaOrVlanTagLength = haInfo.tag0Length;
            }

        }
        else
        {
            /* non vlan tag needed */
            __LOG(("non vlan tag needed"));
            dsaOrVlanTagLength = 0;
        }
    }

    if(SMEM_CHT_IS_SIP5_GET(devObjPtr) && haInfo.evbBpeTagLength)
    {
        if((descrPtr->egrMarvellTagType == MTAG_TYPE_NONE_E) ||
           (haInfo.evbBpeTagLength == 4) ||
           (haInfo.evbBpeIsPassenger && haInfo.tunnelStart))
        {
            descrPtr->haToEpclInfo.evbBpeRspanTagSize += haInfo.evbBpeTagLength;
            totalLength += haInfo.evbBpeTagLength;
        }
        else
        {
            __LOG(("Push [%d]B E-Tag should not be done on cascading port. (ignore the BPE tag !) \n",
                haInfo.evbBpeTagLength));
            haInfo.evbBpeTagLength = 0;
        }
    }

    if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        /* Create Timestamp Tag if needed */
        snetLion3HaCreateTimestampTag(devObjPtr, descrPtr, timestampTag, &timestampTagLength);
        totalLength += timestampTagLength;
    }

    if(needOriginalInfo == GT_TRUE && descrPtr->analyzerVlanTagAdd == 0 && haInfo.tunnelStart == 0)
    {
        /*  (descrPtr->outGoingMtagCmd == SKERNEL_MTAG_CMD_TO_CPU_E && descrPtr->srcTrg == 0)-->Src ||
            (descrPtr->outGoingMtagCmd == SKERNEL_MTAG_CMD_TO_TRG_SNIFFER_E && descrPtr->rxSniff) */

        haBuildPacketWithOrigInfoFromIngressToCpuOrFromIngressToAnalyzer(devObjPtr,descrPtr,
            egressBufferPtr,
            dsaOrVlanTagLength,
            dsaOrVlanTag,
            vlanEtherType,
            timestampTag,
            timestampTagLength,
            frameDataPtrPtr,frameDataSize,
            &haInfo);

        return ;

    }

    mtagTxSniffOrToCpu = /* fix JIRA CPSS-16630: extend TR101 FWD condition */
        (descrPtr->outGoingMtagCmd == SKERNEL_MTAG_CMD_TO_CPU_E && descrPtr->srcTrg == 1) ||
        (descrPtr->outGoingMtagCmd == SKERNEL_MTAG_CMD_TO_TRG_SNIFFER_E && descrPtr->rxSniff == 0);
    mtagTxSniffOrToCpu &= descrPtr->isPktMirrorInLocalDev;

    if(descrPtr->tr101ModeEn &&
       ((descrPtr->outGoingMtagCmd == SKERNEL_MTAG_CMD_FORWARD_E) ||
       (mtagTxSniffOrToCpu == GT_TRUE) ||
       /* fix JIRA : CPSS-14135 WM Simulation: Wrong Tag1 on packet From_CPU*/
       (descrPtr->outGoingMtagCmd == SKERNEL_MTAG_CMD_FROM_CPU_E && descrPtr->useVidx == 1))
       &&
       haInfo.tunnelStart == 0)/* the TS get it's tagging regardless to tr101 of the vlan entry */
    {
        /* Build Tag0 VLAN and Tag1 VLAN according to Tag state */
        snetXCatHaEgressTagBuild(devObjPtr, descrPtr, &haInfo, vlanEtherType, vlanEtherType1,
                               descrPtr->egrMarvellTagType, afterDsaTagLength,
                               descrPtr->passangerTr101EgressVlanTagMode,
                               dsaOrVlanTag, &dsaOrVlanTagLength,
                               &tag0OffsetInTag,&tag1OffsetInTag);
    }
    else
    {
        tag0OffsetInTag = destVlanTagged ? dsaTagLength : SMAIN_NOT_VALID_CNS;/*if tag 0 exists it is first in the tag (no tag1) */
    }

    /* egress extended DSA tag , and ingress with longer DSA tag */
    if (descrPtr->egrMarvellTagType == MTAG_EXTENDED_E &&
        (0 == SMEM_CHT_IS_SIP5_GET(devObjPtr)) &&
        descrPtr->tunnelTerminated == 0 && /* the ingress dsa is not relevant , when TT */
        descrPtr->marvellTagged &&
        descrPtr->marvellTaggedExtended > SKERNEL_EXT_DSA_TAG_2_WORDS_E)
    {
        GT_U32  index;

        __LOG(("Support for 'longer' DSA tag formats : \n"));

        __LOG(("ingress packet came with [%d] words DSA tag , so need to preserve the words after the first 2 words \n",
            (descrPtr->marvellTaggedExtended+1)));

        /* copy from ingress buffer */
        for (index = 8 ; index < (GT_U32)((descrPtr->marvellTaggedExtended+1)*4) ; index ++)
        {
            dsaOrVlanTag[index] = descrPtr->startFramePtr[12+index];
        }

        __LOG(("Add indication to 'word 1 bit 31' that there are more words in the DSA  \n"));
        dsaOrVlanTag[4] |= 1 << 7;

        if(descrPtr->outGoingMtagCmd == SKERNEL_MTAG_CMD_FORWARD_E)
        {
            GT_U8   bit6 = (1 << 6);/* bit 6 in byte 4 is --> bit 30 in word 1 */
            /* keep word 1 bit 30 from the ingress DSA tag !
                this field is Src_ePort[6] ... regardless to srcIsTrunk !*/

            /* clear bit 30 */
            dsaOrVlanTag[4] &= ~bit6;
            /* set bit 30 if was in ingress DSA */
            if(bit6 & descrPtr->startFramePtr[12+4])
            {
                dsaOrVlanTag[4] |= bit6;

                __LOG(("'word 1 bit 30' = '1' is taken from ingress DSA regardless to srcIsTrunk \n"));
            }
            else
            {
                __LOG(("'word 1 bit 30' = '0' is taken from ingress DSA regardless to srcIsTrunk \n"));
            }
        }

        /* keep the length of the 'ingress' */
        dsaOrVlanTagLength = index;
        __LOG(("DSA BYTES:"));
        simLogDump(devObjPtr, SIM_LOG_INFO_TYPE_PACKET_E,
                   (GT_PTR)dsaOrVlanTag, dsaOrVlanTagLength);
        __LOG(("(this includes the DSA words after the 'extended dsa')"));
    }

    if(descrPtr->VntL2Echo)
    {
        if(!SMEM_CHT_IS_SIP5_GET(devObjPtr))
        {
            /* Layer 2 Echo packets transmitted with their preserved VLAN tag.
               don't use modified vlan/DSA tags */
            dsaOrVlanTagLength = 0;

            /* use ONLY the original tag */
            __LOG(("descrPtr->VntL2Echo : Layer 2 Echo packets transmitted with their preserved VLAN tag \n"));
            origVlanTagLength = descrPtr->origVlanTagLength;
            origVlanTagPtr = descrPtr->origVlanTagPtr;
        }
        else
        {
            /* In SIP5, we don't "preserved VLAN tag", so egress tagging is subject
               to normal egress state modifications, as well as changes of TPID, CFI, UP.
            */
            __LOG(("NOTE: Layer 2 Echo packets egress tagging subject to normal egress state modifications \n"));
        }
    }

    totalLength += dsaOrVlanTagLength;

    __LOG(("haInfo.tunnelStart = [%d] , passengerNeedL2Info = [%d] \n",
        haInfo.tunnelStart ,passengerNeedL2Info));

    if(haInfo.tunnelStart == 0 || passengerNeedL2Info)
    {
        /* Set pointer to the original VLAN tag section in case that
            1.  The 'NESTED vlan' forced to ignore it during ingress processing
            2. "Tagging on CPU Port" feature enabled and packet is TO_CPU (LION B0 and above)
            3. "Forced 4B FROM_CPU DSA Tag"  feature enabled and packet is FROM_CPU, TO_ANALYZER, FORWARD  (relevant to SIP5_15 ) */

        __LOG(("forceNewDsaToCpu = [%d] , nestedVlanAccessPort = [%d] origSrcTagged[%d] , origVlanTagLength[%d]\n",
            descrPtr->forceNewDsaToCpu ,
            descrPtr->nestedVlanAccessPort,
            descrPtr->origSrcTagged ,
            descrPtr->origVlanTagLength
            ));

        if((descrPtr->forceNewDsaToCpu || descrPtr->nestedVlanAccessPort) &&
           (descrPtr->origSrcTagged && descrPtr->origVlanTagLength))
        {
            origVlanTagLength = descrPtr->origVlanTagLength;
            origVlanTagPtr = descrPtr->origVlanTagPtr;
        }

        if(descrPtr->nestedVlanAccessPort &&
           descrPtr->marvellTagged &&
           descrPtr->origSrcTagState != SKERNEL_FRAME_TR101_VLAN_INGR_TAG_UNTAGGED_E && /* the DSA hold vlan info */
           !descrPtr->tunnelTerminated)
        {
            __LOG(("got DSA with nested vlan ,need to add the 4 bytes of the DSA tag ... but in vlan tag format \n"));
            /* the DSA info is considered 'tag 0' by the ingress pipe */
            replaceDsaWithVlanTagLength = 4;
            replaceDsaWithVlanTagPtr = &replcaeDsaWithVlanTagArr[0];

            snetChtHaEgressTagDataExtGet(
                descrPtr->vlanTag802dot1dInfo.vpt,
                descrPtr->vlanTag802dot1dInfo.vid,
                (GT_U8)(descrPtr->incomingMtagCmd == SKERNEL_MTAG_CMD_TO_CPU_E ?
                    descrPtr->vlanTag802dot1dInfo.cfi :
                    descrPtr->cfidei),  /*In TO_CPU non-Ext there's no CFI bit.*/
                (GT_U16)vlanEtherType,
                replcaeDsaWithVlanTagArr);

        }

        /* set pointer to the original frame to the encapsulation section */
        ethernetEncapsulationPtr = descrPtr->afterVlanOrDsaTagPtr;
        /* ignore 2 bytes of etherType , because it may done tunnel termination */
        ethernetEncapsulationLength = descrPtr->afterVlanOrDsaTagLen - 2;

        if(useDataAfterOuterTagAsData == GT_TRUE)
        {
            __LOG(("Consider the bytes (of ingress packet) after the outer vlan tag as 'data' \n"));
            origVlanTagLength = descrPtr->origVlanTagLength;
            origVlanTagPtr = descrPtr->origVlanTagPtr;

#if 0 /* the function snetChtL2Parsing already synch origVlanTagLength according to afterVlanOrDsaTagPtr */
            /* remove those bytes from the 'afterVlanOrDsaTagPtr' */
            if(origVlanTagLength <= ethernetEncapsulationLength)
            {
                ethernetEncapsulationLength -= origVlanTagLength;
                ethernetEncapsulationPtr    += origVlanTagLength;
            }
            else
            {
                ethernetEncapsulationLength = 0;
                ethernetEncapsulationPtr    = NULL;
            }
#endif
        }

        /* Update pointer to ethertype of double tagged packets
          for xCat A1 and above devices and for TO_CPU packets
          or TR101 disabled.
          Because ingress moves afterVlanOrDsaTagPtr to point after second
          VLAN tag. But devices should take care of second VLAN tag as
          payload for TO_CPU packets or in TR101 disabled state. */
        if(
           (ethernetEncapsulationLength >= 4))/*meaning that there are 4 bytes for the ethernetEncapsulationPtr -= 4 */
        {
            if(((descrPtr->srcTagState == SKERNEL_FRAME_TR101_VLAN_INGR_TAG_OUT_TAG0_IN_TAG1_E) ||
                (descrPtr->srcTagState == SKERNEL_FRAME_TR101_VLAN_INGR_TAG_OUT_TAG1_IN_TAG0_E)))
            {
                if (descrPtr->tr101ModeEn == 0 ||
                    descrPtr->outGoingMtagCmd == SKERNEL_MTAG_CMD_TO_CPU_E)
                {
                    __LOG(("need to restore the second tag \n"));
                    /* need to restore the second tag */
                    ethernetEncapsulationPtr -= 4;
                    ethernetEncapsulationLength += 4;
                }
            }
        }

        if(descrPtr->doRouterHa &&
           descrPtr->l2Encaps == SKERNEL_LLC_SNAP_E &&
           ethernetEncapsulationLength >= 8)
        {
            /* skip 8 bytes */

            /* ignore the ingress encapsulation - 8 bytes */
            /* Length - 2 bytes
               DSAP - 0xAA (1 byte)
               SSAP - 0xAA (1 byte)
               CTRL - 0x03 (1 byte)
               OUI  - 3 bytes
               ===============
               total 8 bytes need to be removed.
            */
            ethernetEncapsulationPtr    += 8;
            ethernetEncapsulationLength -= 8;

            /* packet is converted to valid EthV2 packet , because :
               LLC-SNAP is basically a historical format used for interworking
               Ethernet with other LAN types, and is not commonly in use today*/
            __LOG(("NOTE: LLC-SNAP with <doRouterHa> : removes the LLC-SNAP 8 bytes header and 'converted' to ETH-V2 !!! \n"));
        }

        /* fill the EtherType info */
        if(devObjPtr->errata.llcNonSnapWithDoRouterHaCurruptL2Header &&
           descrPtr->doRouterHa &&
           descrPtr->l2Encaps == SKERNEL_LLC_E /* llc non-snap */)
        {
            /* reset the ethertype */
            descrPtr->etherTypeOrSsapDsap = 0;
            etherType[0]    = 0;/*this emulate HW behavior (in GM those bytes are not reset) */
            etherType[1]    = 0;
            etherTypeLength = 2;
            /* skip 3 bytes */
            /* ignore the ingress encapsulation - 2 bytes */
            ethernetEncapsulationPtr = NULL;
            ethernetEncapsulationLength = 0;
            /* so we are left with 1 byte in the payload that need to be skipped*/
            descrPtr->l3StartOffsetPtr += 1;
            descrPtr->payloadPtr += 1;
            descrPtr->payloadLength -= 1;
            /* Erratum : the encapsulation of LLC-non snap removed (5 bytes) and replaced by 2 bytes of ethertype that is '0' (2 bytes) */
            __LOG(("NOTE: ERROR : Errata : LLC-non-SNAP with <doRouterHa> : removes the LLC-non-SNAP (5 bytes) header and 'converted' to ETH-V2 with ethertype =0 (2 bytes) \n"));
            __LOG(("NOTE: ERROR : Errata : LLC-non-SNAP with <doRouterHa> : so we set EtherType=0 and packet shorten by 3 bytes (5-2=3) \n"));
            __LOG_PARAM(descrPtr->etherTypeOrSsapDsap);
        }
        else
        {
            etherType[0]    = (GT_U8)(descrPtr->etherTypeOrSsapDsap >> 8);
            etherType[1]    = (GT_U8)(descrPtr->etherTypeOrSsapDsap);
            __LOG(("fill the EtherType info [0x%4.4x] \n",
                descrPtr->etherTypeOrSsapDsap));
            etherTypeLength = 2;
        }
    }
    else
    {
        __LOG(("The tunnel start should set it's own Encapsulation and EtherType \n"));
        /* the tunnel start should set it's own Encapsulation and EtherType */
        etherTypeLength = 0;
        ethernetEncapsulationPtr = NULL;
        ethernetEncapsulationLength = 0;
    }

    __LOG(("passengerNeedL2Info [%d] \n",
        passengerNeedL2Info));

    if(passengerNeedL2Info)
    {
        totalLength += macDaSaLength;/* the mac addresses of the passenger */

        if(descrPtr->tr101ModeEn &&
           descrPtr->outGoingMtagCmd == SKERNEL_MTAG_CMD_FORWARD_E )
        {
            /* Build Tag0 VLAN and Tag1 VLAN according to passenger Tag state */
            snetXCatHaEgressTagBuild(devObjPtr, descrPtr, &haInfo , vlanEtherType, vlanEtherType1,
                                   MTAG_TYPE_NONE_E, 0, descrPtr->passangerTr101EgressVlanTagMode,
                                   passengerVlanTag, &passengerVlanTagLength,
                                   &passengerTag0OffsetInTag,&passengerTag1OffsetInTag);
        }
        else
        if(descrPtr->passangerTr101EgressVlanTagMode != SKERNEL_FRAME_TR101_VLAN_EGR_TAG_UNTAGGED_E)
        {
            /* single tag */
            __LOG(("single tag \n"));
            passengerVlanTagLength = haInfo.tag0Length;
            snetChtHaEgressTagDataExtGet((GT_U8)descrPtr->up, (GT_U16)descrPtr->haAction.vid0,
                  (GT_U8)descrPtr->cfidei, (GT_U16)vlanEtherType, passengerVlanTag);
            if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
            {
                /* check if need to build egress tag0 longer than 4 bytes */
                snetHaSip5EgressTagAfter4BytesExtension(devObjPtr,
                                            descrPtr,&haInfo,GT_TRUE/*tag0*/,
                                            passengerVlanTag);
            }
        }
        else
        {
            /* no tags */
            __LOG(("no tags \n"));
            passengerVlanTagLength = 0;
        }

        /* update the total length */
        totalLength += passengerVlanTagLength;

        /* need to add the macSa,macDa,vlanTags into the <length> calculation
           and then to do <checksum> */
        extraLength = macDaSaLength +   /* mac SA DA - passenger */
                      passengerVlanTagLength + /* vlan tag(s) - passenger */
                      ethernetEncapsulationLength + /* encapsulation and extra vlan tags - passenger */
                      etherTypeLength;/* 2 bytes ethertype - passenger */

        if((descrPtr->egrMarvellTagType == MTAG_TYPE_NONE_E) ||
           (haInfo.evbBpeTagLength == 4) ||
           (haInfo.evbBpeIsPassenger && haInfo.tunnelStart))
        {
            /*fix JIRA : CPSS-7475 */
            extraLength += haInfo.evbBpeTagLength;
        }

        snetChtHaUpdateIPTotalLength(devObjPtr, descrPtr, &haInfo, extraLength);
    }

    totalLength += replaceDsaWithVlanTagLength;

    totalLength += origVlanTagLength;

    totalLength += ethernetEncapsulationLength;

    totalLength += etherTypeLength;

    tsEgressBuffLength = 0;

    payloadPtr = descrPtr->payloadPtr;
    payloadLen = descrPtr->payloadLength;
    /* if Cut Through Packet byte count not calculated, use original size */
    if (descrPtr->cutThroughModeEnabled
        && (descrPtr->cutThrough2StoreAndForward || descrPtr->cutThroughModeTerminated))
    {
        GT_U32 payloadOffset =
            ((GT_U8*)descrPtr->payloadPtr
            - (GT_U8*)descrPtr->frameBuf->actualDataPtr);
        payloadLen = (descrPtr->frameBuf->actualDataSize - payloadOffset);
        __LOG(
            ("Use original frame size [%d] as Byte Count due to Cut Through termination, payloadLen %d",
             descrPtr->frameBuf->actualDataSize, payloadLen));
    }

    if(devObjPtr->crcPortsBytesAdd == 0 &&
       devObjPtr->errata.tunnelStartPassengerEthPacketCrcRemoved == 0 &&
       haInfo.retainCrc == 1)
    {
        /* the simulation not counted the CRC of the passenger in the ingress
           so need to add it now */
        payloadLen += 4;
    }

    /* Copy 20 bytes (Layer3 header) that can be modified  */
    if(descrPtr->isNat && descrPtr->isIp && descrPtr->isIPv4 == 0)
    {
        /* ipv6 for NAT (not needed for TS) */
        /* we need the SIP and DIP to be inside the l3HeaderChangedBuffer when
           getting into chtHaIpHeaderModifications */
        l3HeaderChangedLen = 40;
    }
    else
    {
        l3HeaderChangedLen = 20;

        if(descrPtr->isNat && descrPtr->isIPv4)
        {
            l4HeaderChangedLen =
                CHECKSUM_OFFSET_IN_L4_MAC(descrPtr->ipProt) + /* 6 (udp) or 16 (tcp)*/
                CHECKSUM_NUM_BYTES;/*2 bytes checksum*/
            /* copy needed bytes from start of L4 of the packet */
            memcpy(l4HeaderChangedBuffer,
                payloadPtr + (4*descrPtr->ipxHeaderLength),
                l4HeaderChangedLen);
        }

    }
    memcpy(l3HeaderChangedBuffer, payloadPtr, l3HeaderChangedLen);

    /*4. add tunnel start section or do routing modifications */
    if(haInfo.tunnelStart)
    {
        tsEgressBuff = descrPtr->tunnelStartRestOfHeaderInfoPtr;
        tsEgressBuffLength = descrPtr->tunnelStartRestOfHeaderInfoLen;

        if(descrPtr->tunnelStartType == SKERNEL_FRAME_TUNNEL_START_TYPE_TRILL_E &&
           descrPtr->tunnelStartTrillTransit == 1)
        {
            /* the TRILL header <Hop Count> is decremented */
            /* Overwrite TRILL header <Hop Count> with desc<TTL> */
            if(descrPtr->decTtl == GT_TRUE && (descrPtr->ttl != 0))
            {
                l3HeaderChangedLen = 2;/* update byte [1] in the TRILL header */

                l3HeaderChangedBuffer[1] &= 0xc0;/* clear 6 bits */
                l3HeaderChangedBuffer[1] = descrPtr->ttl - 1;/* update the hopCount with the TTL*/
            }
        }
        /* For IP tunnel with IP passenger set ECN value of outer packet */
        /* Applicable only for SIP6 devices */
        if (SMEM_CHT_IS_SIP6_GET(devObjPtr))
        {
            IP_ECN_ENT  ecnValue;
            if(descrPtr->isIp &&
                (descrPtr->tunnelStartType == SKERNEL_FRAME_TUNNEL_START_TYPE_GENERIC_IPV4_E ||
                descrPtr->tunnelStartType == SKERNEL_FRAME_TUNNEL_START_TYPE_GENERIC_IPV6_E))
            {
                if (!descrPtr->markEcn)
                {
                    GT_U32 regVal;
                    smemRegGet(devObjPtr, SMEM_LION3_ERMRK_GLOBAL_CONFIG_REG(devObjPtr), &regVal);
                    fldVal = SMEM_U32_GET_FIELD(regVal, 15, 1);
                    tsEgressBuff[3] &= 0xFC; /* clear ECN value */
                    __LOG(("IP passenger over IP Tunnel Start : ECN MODE [%d]:", fldVal));
                    if (fldVal) /* 0 - Compatibility ECN mode; 1 - Normal ECN mode */
                    {
                        /* In normal mode outer packet ECN copied from passenger */
                        ecnValue = l3HeaderChangedBuffer[1] & 0x3;
                        __LOG(("Copy ECN value [%d] from passenger", ecnValue));
                    }
                    else
                    {
                        ecnValue = IP_ECN_NOT_ECT_E;
                        __LOG(("Set NOT_ECT ECN value"));
                    }
                }
                else
                {
                    ecnValue = IP_ECN_CE_E;
                    __LOG(("Set CE ECN value"));
                }
                tsEgressBuff[3] |= ecnValue;
            }
        }
        if (haInfo.tsIpv4HeaderPtr)
        {
            /*Support tunnel start IPv4*/
            chtHaIpHeaderModifications(devObjPtr,descrPtr,egressPort,GT_TRUE,l3HeaderChangedBuffer,&haInfo,
                NULL,0,GT_FALSE);
        }
    }
    if(SMEM_CHT_IS_SIP6_GET(devObjPtr))
    {
        /* Apply ERMRK ECN logic */
        if(!haInfo.tunnelStart && descrPtr->isIp && descrPtr->markEcn)
        {
            l3HeaderChangedBuffer[3] &= 0xFC; /* clear ECN value */
            l3HeaderChangedBuffer[3] |= IP_ECN_CE_E;
            __LOG(("Set CE ECN value"));
        }
    }

    /* Enable L2NAT only if following conditions are satified */
    if (descrPtr->isIPv4                                      /* if this is IPv4 packet */
        && !(descrPtr->isNat)                                 /* if L3NAT is not set */
        && !(descrPtr->eArchExtInfo.haInfo.tunnelStart)       /* if EPORT tunnelStart is not set */
        && !(descrPtr->eArchExtInfo.haInfo.ePortModifyMacDa)  /* if EPORT Modify Mac DA is not set */
        && !(descrPtr->VntL2Echo)                             /* if not L2 Echo Packet */
        && !(descrPtr->cfm)                                   /* if not CFM packet */
        && SMEM_CHT_IS_SIP6_30_GET(devObjPtr))                /* if only SIP6_30 devices */
    {
        /* Get L2NAT enable flag from EPORT table */
        isL2Nat = SMEM_LION3_HA_EPORT_1_ENTRY_FIELD_AUTO_GET(devObjPtr, descrPtr,
                            SMEM_SIP6_30_HA_EPORT_TABLE_1_FIELDS_L2NAT_ENABLE_E);
    }

    if((descrPtr->isIp && (descrPtr->modifyDscp || descrPtr->doRouterHa))/* IP routed */
       || isL2Nat) /* L2NAT enabled */
    {
        /*Support IP modification (for NON tunnel start / passenger)*/
        chtHaIpHeaderModifications(devObjPtr,descrPtr,egressPort,GT_FALSE,l3HeaderChangedBuffer,&haInfo,
                l4HeaderChangedBuffer,l4HeaderChangedLen,isL2Nat);
    }

    if (descrPtr->mpls)
    {
        GT_U32  oldExp;

        oldExp = (l3HeaderChangedBuffer[2] >> 1) & 0x7;

        if(haInfo.tunnelStart == 0)
        {
            /*update ttl if needed*/
            if(descrPtr->decTtl == GT_TRUE && (descrPtr->ttl != 0)) /*update TTL assignment*/
            {
                __LOG(("decrement TTL [%d] in the first label (most outer) \n",
                       descrPtr->ttl - 1));
                l3HeaderChangedBuffer[3] = (GT_U8)(descrPtr->ttl - 1);
            }
            else if (descrPtr->isMplsLsr) /* Pop<1>..Pop<3> */
            {
                __LOG(("LSR : set TTL [%d] in the first label (most outer) \n",
                       descrPtr->ttl));
                l3HeaderChangedBuffer[3] = (GT_U8)descrPtr->ttl;
            }

            if(allowMplsExpModification && descrPtr->modifyDscp)
            {
                __LOG(("(due to: descrPtr->modifyDscp) modify the EXP [%d] to [%d] in the first label (most outer) \n",
                    oldExp , descrPtr->exp[0]));

                l3HeaderChangedBuffer[2] &= 0xf1;/* clear bits 1..3 --> the EXP in the outer label */
                l3HeaderChangedBuffer[2] |= descrPtr->exp[0] << 1;
            }
        }
        else
        if(haInfo.tunnelStart &&
           descrPtr->tunnelStartType == SKERNEL_FRAME_TUNNEL_START_TYPE_MPLS_E &&
           descrPtr->isMplsLsr &&
           (descrPtr->mplsLsrOffset < ((descrPtr->numOfLabels + 1) * 4)))/* check that we have valid label to update (not remove all labels) */
        {/*next code added to fix CQ#148940,#149353 */
            if(descrPtr->modifyDscp)
            {
                if (SMEM_CHT_IS_SIP7_GET(devObjPtr))
                {
                    if (descrPtr->totalPoppedLabels > 0)
                    {
                        __LOG(("(due to: descrPtr->modifyDscp) modify the EXP [%d] to [%d] in the first label after the TS labels \n",
                              oldExp , descrPtr->exp[0]));
                        l3HeaderChangedBuffer[2] &= 0xf1;/* clear bits 1..3 --> the EXP in the outer label */
                        l3HeaderChangedBuffer[2] |= descrPtr->exp[0] << 1;
                    }
                    else
                    {
                        __LOG(("WARNING : according to FS : The QoS attribute <Modify DSCP/EXP> must be cleared for packets whose TTI Action \n"
                            " <MPLS Command> is either Push, Swap, or Pop<n>-Swap \n"));

                    }
                }
                else
                {
                    if(descrPtr->mplsCommand == SKERNEL_XCAT_TTI_MPLS_POP_1_E ||
                       descrPtr->mplsCommand == SKERNEL_XCAT_TTI_MPLS_POP_2_E ||
                       descrPtr->mplsCommand == SKERNEL_XCAT_TTI_MPLS_POP_3_E)
                    {
                        __LOG(("(due to: descrPtr->modifyDscp) modify the EXP [%d] to [%d] in the first label after the TS labels \n",
                            oldExp , descrPtr->exp[0]));
                        l3HeaderChangedBuffer[2] &= 0xf1;/* clear bits 1..3 --> the EXP in the outer label */
                        l3HeaderChangedBuffer[2] |= descrPtr->exp[0] << 1;
                    }
                    else
                    {
                        __LOG(("WARNING : according to FS : The QoS attribute <Modify DSCP/EXP> must be cleared for packets whose TTI Action \n"
                            " <MPLS Command> is either Push, Swap, or Pop<n>-Swap \n"));

                    }
                }
            }

            if (SMEM_CHT_IS_SIP7_GET(devObjPtr))
            {
                updateTtl = (descrPtr->totalPoppedLabels)?GT_TRUE:GT_FALSE;
            }
            else
            {
                /* When <MPLS Command> is NOP, decTtl field should be disabled. */
                updateTtl = (descrPtr->mplsCommand != SKERNEL_XCAT_TTI_MPLS_NOP_E)?GT_TRUE:GT_FALSE;
            }

            if (updateTtl)
            {
                /*update ttl if needed*/
                if(descrPtr->decTtl == GT_TRUE && (descrPtr->ttl != 0)) /*update TTL assignment*/
                {
                    __LOG(("Tunnel start add labels but still update on incoming MPLS label : do decrement TTL [%d] in the first label (most outer) \n",
                            descrPtr->ttl - 1));
                    l3HeaderChangedBuffer[3] = (GT_U8)(descrPtr->ttl - 1);
                }
                else if (descrPtr->isMplsLsr) /* Pop<1>..Pop<3> */
                {
                    __LOG(("Tunnel start add labels but still update on incoming MPLS label : (do LSR) set TTL [%d] in the first label (most outer) \n",
                           descrPtr->ttl));
                    l3HeaderChangedBuffer[3] = (GT_U8)descrPtr->ttl;
                }
            }
        }
    }

    /* HA CFM Loopback configuration */
    if (descrPtr->cfm && descrPtr->VntL2Echo)
    {
        if (SMEM_CHT_IS_SIP5_GET(devObjPtr))
        {
            regAddr = SMEM_LION2_HA_GLOBAL_CONF1_REG(devObjPtr);
            regPtr = smemMemGet(devObjPtr, regAddr);
            fldVal = SMEM_U32_GET_FIELD(regPtr[0], 10, 1);
            /* CFM reply mode */
            if(fldVal)
            {
                /* Change the lsbit of the opcode to zero */
                __LOG(("Change the lsbit of the opcode to zero"));
                fldVal = l3HeaderChangedBuffer[CFM_OPCODE_BYTE_OFFSET_CNS] & ~0x1;
            }
            else
            {
                /* Backward compatible mode */
                /* CFM Loopback Reply Opcode */
                __LOG(("CFM Loopback Reply Opcode"));
                regAddr = SMEM_CHT3_ROUTE_HA_GLB_CNF_REG(devObjPtr);
                regPtr = smemMemGet(devObjPtr, regAddr);
                fldVal = SMEM_U32_GET_FIELD(regPtr[0], 3, 8);
            }

            __LOG(("cfm , VntL2Echo : update the CPM opcode , from [%x] to [%x] \n",
                l3HeaderChangedBuffer[CFM_OPCODE_BYTE_OFFSET_CNS],
                fldVal));

            l3HeaderChangedBuffer[CFM_OPCODE_BYTE_OFFSET_CNS] = fldVal;
        }
        else
        {
            /* update the CPM opcode in the egress buffer ,
               only when also did swap of mac sa,da */
            smemRegFldGet(devObjPtr, SMEM_CHT3_ROUTE_HA_GLB_CNF_REG(devObjPtr),
                              2, 8, &fldVal);

            __LOG(("cfm , VntL2Echo : update the CPM opcode , from [%x] to [%x] \n",
                l3HeaderChangedBuffer[CFM_OPCODE_BYTE_OFFSET_CNS],
                fldVal));

            l3HeaderChangedBuffer[CFM_OPCODE_BYTE_OFFSET_CNS] = fldVal;
        }
    }

    totalLength += (tsEgressBuffLength + payloadLen);

    /*pad Zero to 64 to bytes if needed*/

    paddingZeroForLess64BytesLength = 0;
    if(totalLength < SGT_MIN_FRAME_LEN)
    {
        /* add minimal 64 bytes logic */
        /* the HA unit hold no knowledge about padding in the MAC ... it assumes 'Pad to 64' */

        /* Enable padding of transmitted packets shorter than 64B */
        __LOG(("Consider Padding of packet length [%d] (shorter than 64 bytes) \n",
            totalLength));

        paddingZeroForLess64BytesLength = SGT_MIN_FRAME_LEN - totalLength;
        if(descrPtr->calcCrc && payloadLen >= 4)
        {
            __LOG(("need to recalc CRC (override existing 4 bytes) \n"));

            /* the 'incomming' CRC section is inside the short packet , but
               the 'outgoing' CRC is in the last 4 bytes. so we need to 'delete'
               the orig CRC */
            payloadLen -= 4;/* remove old CRC */
            totalLength -= 4;
            paddingZeroForLess64BytesLength += 4;/*make the old CRC with value 0 */
        }
    }

    totalLength += paddingZeroForLess64BytesLength;

    /* check if the frame needs to be truncated */
    if (descrPtr->outGoingMtagCmd == SKERNEL_MTAG_CMD_TO_CPU_E)
    {
        if ( (descrPtr->truncated) && (totalLength > 128) )
        {
            __LOG(("TO_CPU : truncated to use max of 128 bytes \n"));
            totalLength = 128;
        }
    }
    else if (descrPtr->outGoingMtagCmd == SKERNEL_MTAG_CMD_TO_TRG_SNIFFER_E ||
             descrPtr->rxSniff)
    {
        if ( (descrPtr->truncated) && (totalLength > 128) )
        {
            __LOG(("TO_ANALYZER : truncated to use max of 128 bytes \n"));
            totalLength = 128;
        }
    }

    /* use dedicated egress buffer */

    /* set perpend bytes (if needed) */
    if(descrPtr->haToEpclInfo.prePendLength)
    {
        MEM_APPEND_WITH_LOG(egressBufferPtr,prePendArr,descrPtr->haToEpclInfo.prePendLength);
    }

    /* set mac sa,da bytes (if needed) */
    if(macDaSaLength)
    {
        /* save info for the EPCL */
        SET_IF_NULL_MAC(
            descrPtr->haToEpclInfo.macDaSaPtr,egressBufferPtr);

        if(haInfo.tunnelStart)
        {
            /* save for the EPCL explicit place of the TS L2 */
            SET_FORCE_MAC(descrPtr->haToEpclInfo.tunnelStartL2StartOffsetPtr,egressBufferPtr);
        }

        MEM_APPEND_WITH_LOG(egressBufferPtr,macDaSa,macDaSaLength);
    }

    /* new Pushed tag is added before VLAN tags but after DSA tag */
    if(haInfo.evbBpeTagLength &&
       (haInfo.tunnelStart == 0 || haInfo.evbBpeIsPassenger == 0) &&
       (descrPtr->egrMarvellTagType == MTAG_TYPE_NONE_E))
    {
        __LOG(("ADD EVB tag before any other tag incase no TS or TS that need to hold it (and not passenger) \n"));
        /* ADD EVB tag before any other tag */

        SET_IF_NULL_MAC(
            descrPtr->haToEpclInfo.outerVlanTagPtr,egressBufferPtr);
        MEM_APPEND_WITH_LOG(egressBufferPtr, evbBpeExtraTag, haInfo.evbBpeTagLength);
    }

    /* add analyzer VLAN Tag */
    if(analyzerVlanTagLength)
    {
        /* save info for the EPCL */
        SET_IF_NULL_MAC(
            descrPtr->haToEpclInfo.outerVlanTagPtr,egressBufferPtr);

        MEM_APPEND_WITH_LOG(egressBufferPtr, analyzerVlanTag, 4);
    }

    /* set vlan tag/dsa tag bytes (if needed) */
    if(dsaOrVlanTagLength)
    {
        if(devObjPtr->vplsModeEnable.ha  && descrPtr->useVidx == 0 &&
           descrPtr->outGoingMtagCmd == SKERNEL_MTAG_CMD_FORWARD_E &&
           descrPtr->egrMarvellTagType != MTAG_TYPE_NONE_E)
        {
            /*It is required to support a mode per physical port, where a unicast FWD DSA packet is turned into a
            FROM_CPU packet. This makes the line card device a simple fanout device without the ability to
            manipulate the outgoing packet.
            This logic is located at the last stage of the HA*/
            snetChtHaDsaForwardToFromCpuManipulation(devObjPtr,descrPtr,
                        egressPort,destVlanTagged,dsaOrVlanTag);
        }

        /* save info for the EPCL */

        /* skip DSA tag in case when DSA tag precedes (instead of replace) outer vlan tag */
        SET_IF_NULL_MAC(
            descrPtr->haToEpclInfo.outerVlanTagPtr,egressBufferPtr + dsaTagLength);

        SET_FORCE_VALUE_MAC(
            descrPtr->haToEpclInfo.dsaTagLength, dsaTagLength);
        if (dsaTagLength)
        {
            SET_FORCE_MAC(
                descrPtr->haToEpclInfo.forcedFromCpuDsaPtr, egressBufferPtr);
        }

        if(tag0OffsetInTag != SMAIN_NOT_VALID_CNS)
        {
            offset = tag0OffsetInTag;
            SET_FORCE_MAC(
                descrPtr->haToEpclInfo.vlanTag0Ptr,egressBufferPtr + offset);
        }

        if(tag1OffsetInTag != SMAIN_NOT_VALID_CNS)
        {
            offset = tag1OffsetInTag;
            SET_FORCE_MAC(
                descrPtr->haToEpclInfo.vlanTag1Ptr,egressBufferPtr + offset);
        }

        __LOG_PARAM(timestampTagLength);
        __LOG_PARAM(descrPtr->egrMarvellTagType);
        /* If the packet is tagged but non-DSA tagged */
        if((descrPtr->egrMarvellTagType == MTAG_TYPE_NONE_E) || (timestampTagLength == 0))
        {
            if(timestampTagLength)
            {
                /* Timestamp Tag is after the MACs */
                SET_FORCE_MAC(
                descrPtr->haToEpclInfo.timeStampTagPtr, egressBufferPtr);

                /* put the timestamp tag on the buffer and jump over it */
                MEM_APPEND_WITH_LOG(egressBufferPtr, timestampTag, timestampTagLength);
            }

            MEM_APPEND_WITH_LOG(egressBufferPtr, dsaOrVlanTag, dsaOrVlanTagLength);

            if(haInfo.evbBpeTagLength &&
               (haInfo.tunnelStart == 0 || haInfo.evbBpeIsPassenger == 0) &&
               (descrPtr->egrMarvellTagType != MTAG_TYPE_NONE_E))
            {
                __LOG(("ADD EVB tag after DSA tag without timestamp incase no TS or TS that need to hold it (and not passenger) \n"));

                SET_IF_NULL_MAC(
                    descrPtr->haToEpclInfo.outerVlanTagPtr,egressBufferPtr);
                MEM_APPEND_WITH_LOG(egressBufferPtr, evbBpeExtraTag, haInfo.evbBpeTagLength);
            }
        }
        else
        {
            /* the packet is DSA tagged and Timestamp Tag exists */
            if (tag0OffsetInTag < tag1OffsetInTag)
            {
                /* tag 0 before tag 1 */
                if(tag1OffsetInTag != SMAIN_NOT_VALID_CNS)
                {
                    __LOG(("tag 0 before tag 1 \n"));
                    /* both tags are valid */
                    firstTagLength = (tag1OffsetInTag  - tag0OffsetInTag);
                }
                else
                {
                    __LOG(("only tag 0 is valid \n"));
                    /* only tag 0 is valid */
                    firstTagLength = dsaOrVlanTagLength;
                }
            }
            else
            {
                /* tag 1 before tag 0 */
                if(tag0OffsetInTag != SMAIN_NOT_VALID_CNS)
                {
                    __LOG(("tag 1 before tag 0 \n"));
                    /* both tags are valid */
                    firstTagLength = (tag0OffsetInTag  - tag1OffsetInTag);
                }
                else
                {
                    __LOG(("only tag 1 is valid \n"));
                    /* only tag 1 is valid */
                    firstTagLength = dsaOrVlanTagLength;
                }
            }

            secondTagLength = dsaOrVlanTagLength - tag0OffsetInTag;

            if(firstTagLength)
            {
                /* add the first tag info */
                MEM_APPEND_WITH_LOG(egressBufferPtr, dsaOrVlanTag, firstTagLength);
            }

            /* packet is DSA tagged and the Timestamp tag is immediately after the DSA tag */
            SET_FORCE_MAC(
                descrPtr->haToEpclInfo.timeStampTagPtr, egressBufferPtr);
            /* put the timestamp tag on the buffer and jump over it */
            MEM_APPEND_WITH_LOG(egressBufferPtr, timestampTag, timestampTagLength);

            if(secondTagLength)
            {
                /* add the second tag info */
                MEM_APPEND_WITH_LOG(egressBufferPtr, dsaOrVlanTag + firstTagLength , secondTagLength);
            }

            if(haInfo.evbBpeTagLength &&
               (haInfo.tunnelStart == 0 || haInfo.evbBpeIsPassenger == 0) &&
               (descrPtr->egrMarvellTagType != MTAG_TYPE_NONE_E))
            {
                __LOG(("ADD EVB tag after DSA tag with timestamp incase no TS or TS that need to hold it (and not passenger) \n"));

                SET_IF_NULL_MAC(
                    descrPtr->haToEpclInfo.outerVlanTagPtr,egressBufferPtr);
                MEM_APPEND_WITH_LOG(egressBufferPtr, evbBpeExtraTag, haInfo.evbBpeTagLength);
            }
        }
    }
    else
    {
        /* untagged and non DSA */
        if(timestampTagLength)
        {
            /* If the packet is untagged and non-DSA tagged, then the Timestamp Tag is after the MACs */
            SET_FORCE_MAC(
                descrPtr->haToEpclInfo.timeStampTagPtr, egressBufferPtr);

            /* put the timestamp tag on the buffer and jump over it */
            MEM_APPEND_WITH_LOG(egressBufferPtr, timestampTag, timestampTagLength);
         }
    }

      /* set tunnel start L3 bytes (if needed) */
    if(tsEgressBuffLength)
    {
        if(descrPtr->tunnelStartType == SKERNEL_FRAME_TUNNEL_START_TYPE_MPLS_E)
        {
            /*save info of the MPLS TS for later EPCL modifications , need to skip the 2 bytes of the etherType */
            SET_FORCE_MAC(descrPtr->haToEpclInfo.l3StartOffsetPtr,(egressBufferPtr + 2));
        }
        else
        {
            /* others like IP TS will need 2 pointers , one to the ip TS header
               and one to the IP of passenger , due to the option of EPCL/IPLR
               to modify any one of them */
            /* TBD ... */
        }

        /* save for the EPCL explicit place of the TS L3 (after the etherType of the TS) */
        SET_FORCE_MAC(descrPtr->haToEpclInfo.tunnelStartL3StartOffsetPtr,(egressBufferPtr + 2));

        /* add the rest of the L3 of the TS into the egress buffer */
        MEM_APPEND_WITH_LOG(egressBufferPtr,tsEgressBuff,tsEgressBuffLength);
    }
    else
    if(descrPtr->tunnelStart)
    {
        descrPtr->haToEpclInfo.tunnelStartL3StartOffsetPtr = (egressBufferPtr + 2);
    }

    if(passengerNeedL2Info)
    {
        /* set mac sa,da bytes */
        if(macDaSaLength)
        {
            /* save info for the EPCL */
            SET_FORCE_MAC(/* support TS passenger */
                descrPtr->haToEpclInfo.macDaSaPtr,egressBufferPtr);

            MEM_APPEND_WITH_LOG(egressBufferPtr,passengerL2MasDaSa,12);
        }

        if(haInfo.evbBpeTagLength && haInfo.evbBpeIsPassenger == 1)
        {
            __LOG(("ADD EVB tag before any other tag on the passenger (and not on the TS) \n"));
            /* ADD EVB tag before any other tag */

            SET_IF_NULL_MAC(
                descrPtr->haToEpclInfo.outerVlanTagPtr,egressBufferPtr);
            MEM_APPEND_WITH_LOG(egressBufferPtr, evbBpeExtraTag, haInfo.evbBpeTagLength);
        }

        __LOG(("recalc the vlan tag 0,1 indications because we have Ethernet passenger \n"));
        __LOG(("BUT indications about 'outer vlan' are not changed into the passenger \n"));
        SET_FORCE_MAC(descrPtr->haToEpclInfo.vlanTag0Ptr,NULL);
        SET_FORCE_MAC(descrPtr->haToEpclInfo.vlanTag1Ptr,NULL);

        if(passengerVlanTagLength)
        {
            /* save info for the EPCL */
            if(passengerTag0OffsetInTag != SMAIN_NOT_VALID_CNS)
            {
                offset = passengerTag0OffsetInTag;
                SET_FORCE_MAC(
                    descrPtr->haToEpclInfo.vlanTag0Ptr,egressBufferPtr + offset);
            }

            /* save info for the EPCL */
            if(passengerTag1OffsetInTag != SMAIN_NOT_VALID_CNS)
            {
                offset = passengerTag1OffsetInTag;
                SET_FORCE_MAC(
                    descrPtr->haToEpclInfo.vlanTag1Ptr,egressBufferPtr + offset);
            }

            MEM_APPEND_WITH_LOG(egressBufferPtr, passengerVlanTag, passengerVlanTagLength);
        }
    }
    else
    if(haInfo.tunnelStart)
    {
        /* support TS without ethernet passenger */
        __LOG(("Reset indication of L2 header in passenger of TS that is not Ethernet \n"));
        SET_FORCE_MAC(descrPtr->haToEpclInfo.macDaSaPtr,NULL);
    }

    /* Add the original vlan tag(s) in case that:
        1. Packet came tagged on 'Nested vlan access port', or
        2. If "Tagging on CPU Port" feature enabled and packet is TO_CPU (LION B0 and above) */
    if(origVlanTagLength || replaceDsaWithVlanTagLength)
    {
        /* save info for the EPCL */
        SET_IF_NULL_MAC(
            descrPtr->haToEpclInfo.outerVlanTagPtr,egressBufferPtr);

        if(descrPtr->tag0Ptr == descrPtr->origVlanTagPtr)
        {
            SET_IF_NULL_MAC(
                descrPtr->haToEpclInfo.vlanTag0Ptr,egressBufferPtr);
        }

        if(descrPtr->tag1Ptr == descrPtr->origVlanTagPtr)
        {
            SET_IF_NULL_MAC(
                descrPtr->haToEpclInfo.vlanTag1Ptr,egressBufferPtr);
        }

        if(replaceDsaWithVlanTagLength)
        {
            __LOG(("Add vlan tag that in ingress packet was in DSA \n"));
            MEM_APPEND_WITH_LOG(egressBufferPtr,replaceDsaWithVlanTagPtr,replaceDsaWithVlanTagLength);
        }

        if(origVlanTagLength)
        {
            __LOG(("Add original vlan tag(s) \n"));
            MEM_APPEND_WITH_LOG(egressBufferPtr,origVlanTagPtr,origVlanTagLength);
        }
    }

    /* set Ethernet encapsulation bytes (if needed) */
    if(ethernetEncapsulationLength)
    {
        MEM_APPEND_WITH_LOG(egressBufferPtr,ethernetEncapsulationPtr,ethernetEncapsulationLength);
    }

    /* set Ethernet type bytes (if needed) */
    if(etherTypeLength)
    {
        MEM_APPEND_WITH_LOG(egressBufferPtr,etherType,etherTypeLength);
    }

    /* set payload bytes (if needed) */
    if(payloadLen)
    {
        GT_U8*    tmpL3Info = egressBufferPtr;
        /* save the payload PTR only if not set before */
        SET_IF_NULL_MAC(
            descrPtr->haToEpclInfo.l3StartOffsetPtr,egressBufferPtr);

        if (l3HeaderChangedLen)
        {
            if(payloadPtr != descrPtr->l3StartOffsetPtr)
            {
                skernelFatalError("chtHaUnit: the assumption of same pointer is wrong ?! \n");
            }
            if(payloadLen < l3HeaderChangedLen)
            {/*make sure not to overrun with the length*/
                l3HeaderChangedLen = payloadLen;
            }

            if(timestampTagLength == 0)
            {
                /* zero reserved field in PTP header */
                snetChtHaPtpZeroResevedField(devObjPtr,descrPtr,egressPort,l3HeaderChangedBuffer);
            }

            /* Add to egress buffer 12 bytes of changed l3 header */
            MEM_APPEND_WITH_LOG(egressBufferPtr,l3HeaderChangedBuffer,l3HeaderChangedLen);
            payloadPtr += l3HeaderChangedLen;
            payloadLen -= l3HeaderChangedLen;
        }

        MEM_APPEND_WITH_LOG(egressBufferPtr,payloadPtr,payloadLen);

        if (l4HeaderChangedLen)
        {
            /* update the L4 on the egress packet */
            memcpy(tmpL3Info + (4*descrPtr->ipxHeaderLength) ,
                l4HeaderChangedBuffer,l4HeaderChangedLen);
        }

    }

    /* set Zero padding bytes (if needed) */
    if(paddingZeroForLess64BytesLength)
    {
        /* save the padding length */
        SET_FORCE_VALUE_MAC(descrPtr->haToEpclInfo.paddingZeroForLess64BytesLength,
            paddingZeroForLess64BytesLength);

        MEM_APPEND_WITH_LOG(egressBufferPtr,paddingZeroForLess64Bytes,paddingZeroForLess64BytesLength);
    }

    *frameDataSize   = totalLength;
    *frameDataPtrPtr = devObjPtr->egressBuffer;

    return;
}

#define BUFFER_HEADER_BYTES    128

/**
* @internal snetlion3HaInterruptEvent function
* @endinternal
*
* @brief   Setting interrupt for HA unit.
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - frame data buffer Id
* @param[in] interruptBit             - the interrupt bit to set.
*                                      OUTPUT:
*                                      None
*/
static GT_VOID snetlion3HaInterruptEvent
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32 interruptBit
)
{
    /* Generate HA interrupt */
    snetChetahDoInterrupt(devObjPtr,
                          SMEM_LION3_HA_INTERRUPT_CAUSE_REG(devObjPtr),
                          SMEM_LION3_HA_INTERRUPT_MASK_REG(devObjPtr),
                          (1<<interruptBit),
                          (GT_U32)SMEM_LION3_HA_SUM_INT(devObjPtr));

    descrPtr->isPtpException = 1;
}

/**
* @internal snetLion3HaOutgoingTimestampTag function
* @endinternal
*
* @brief   Setting the type of the outgoing timestamp tag.
*/
static GT_VOID snetLion3HaOutgoingTimestampTag
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32 egressPort
)
{
    DECLARE_FUNC_NAME(snetLion3HaOutgoingTimestampTag);

    GT_U32 regAddr;                     /* register's address */
    GT_U32 *regPtr;                     /* register entry pointer */
    GT_U32 fieldData;                   /* register field value */
    GT_U32 isTstShort;                  /* indication if timestamp tag is short in length*/
    GT_U32 isTstLong;                   /* indication if timestamp tag is long in length*/

    SKERNEL_TIMESTAMP_TAG_TYPE_ENT ingressTst;  /* ingress timestamp tag type */
    SKERNEL_TIMESTAMP_TAG_TYPE_ENT egressTst;   /* egress timestamp tag type */

    GT_U32 portTstMode;                 /* port timestamp tag mode */
    GT_U32 cpuCode;
    GT_U32 pktCmd;

    ingressTst = descrPtr->timestampTagged[SMAIN_DIRECTION_INGRESS_E];

    /* Port Timestamp Reception Enable indication */
    if(SMEM_CHT_IS_SIP7_GET(devObjPtr))
    {
        fieldData = SMEM_SIP7_HA_SRC_PHYSICAL_PORT_2_ENTRY_FIELD_GET(devObjPtr,
            descrPtr->localDevSrcPort,/* the src physical port */
            SMEM_SIP7_HA_SRC_PHYSICAL_PORT_2_TABLE_FIELDS_PORT_TIMESTAMP_RECEPTION_EN_E);
    }
    else
    {
        fieldData = SMEM_LION3_HA_PHYSICAL_PORT_2_ENTRY_FIELD_BY_SRC_PORT_GET(devObjPtr,
            descrPtr->localDevSrcPort,/* the physical port */
            SMEM_LION3_HA_PHYSICAL_PORT_TABLE_2_FIELDS_TIMESTAMP_RECEPTION_EN);
    }

    isTstShort = 0;
    isTstLong = 0;
    if(fieldData)
    {
        if((ingressTst == SKERNEL_TIMESTAMP_TAG_TYPE_NON_EXTENDED_E) ||
           (ingressTst == SKERNEL_TIMESTAMP_TAG_TYPE_PIGGY_BACKED_E))
        {
            isTstShort = 1;
        }
        else
        {
            isTstShort = 0;
        }

        if((ingressTst == SKERNEL_TIMESTAMP_TAG_TYPE_EXTENDED_E) ||
           (ingressTst == SKERNEL_TIMESTAMP_TAG_TYPE_HYBRID_E))
        {
            isTstLong = 1;
        }
        else
        {
            isTstLong = 0;
        }
    }

    /* timeStampTagMode */
    portTstMode = SMEM_LION3_HA_PHYSICAL_PORT_2_ENTRY_FIELD_AUTO_GET(devObjPtr,
        descrPtr,
        SMEM_LION3_HA_PHYSICAL_PORT_TABLE_2_FIELDS_PTP_TIMESTAMP_TAG_MODE);

    if(simLogIsOpenFlag)
    {
        /* log all info related to below selection of egressTst */
        __LOG_PARAM(egressPort);
        __LOG_PARAM(portTstMode);
        __LOG_PARAM(isTstShort);
        __LOG_PARAM(isTstLong);
        __LOG_PARAM(descrPtr->oamInfo.timeStampEnable);
        __LOG_PARAM(descrPtr->ptpActionIsLocal);
        __LOG_PARAM(descrPtr->ptpGtsInfo.gtsEntry.ptpVersion);
    }

    if( portTstMode == SKERNEL_TIMESTAMP_TAG_MODE_NONE_E )
    {
        egressTst = SKERNEL_TIMESTAMP_TAG_TYPE_UNTAGGED_E;
    }
    else if( (portTstMode == SKERNEL_TIMESTAMP_TAG_MODE_ALL_NON_EXTENDED_E) &&
             (isTstLong == 0) )
    {
        egressTst = SKERNEL_TIMESTAMP_TAG_TYPE_NON_EXTENDED_E;
    }
    else if( (portTstMode == SKERNEL_TIMESTAMP_TAG_MODE_ALL_EXTENDED_E) &&
             (isTstShort == 0) )
    {
        egressTst = SKERNEL_TIMESTAMP_TAG_TYPE_EXTENDED_E;
    }
    else if( (portTstMode == SKERNEL_TIMESTAMP_TAG_MODE_NON_EXTENDED_E) &&
             (descrPtr->ptpActionIsLocal ||
              descrPtr->oamInfo.timeStampEnable ||
              isTstShort) &&
             (isTstLong == 0) )
    {
        egressTst = SKERNEL_TIMESTAMP_TAG_TYPE_NON_EXTENDED_E;
    }
    else if( (portTstMode == SKERNEL_TIMESTAMP_TAG_MODE_EXTENDED_E) &&
             (descrPtr->ptpActionIsLocal ||
              descrPtr->oamInfo.timeStampEnable ||
              isTstLong) &&
             (isTstShort == 0) )
    {
        egressTst = SKERNEL_TIMESTAMP_TAG_TYPE_EXTENDED_E;
    }
    else if( (portTstMode == SKERNEL_TIMESTAMP_TAG_MODE_PIGGYBACK_E) &&
             descrPtr->isPtp &&
             (descrPtr->ptpGtsInfo.gtsEntry.ptpVersion == 2) &&
             (descrPtr->ptpActionIsLocal || isTstShort) &&
             (isTstLong == 0) )
    {
        /* HA does not add a piggybacked timestamp, the PSU add it. */
        egressTst = SKERNEL_TIMESTAMP_TAG_TYPE_PIGGY_BACKED_E;
    }
    else if( (portTstMode == SKERNEL_TIMESTAMP_TAG_MODE_HYBRID_E) &&
             descrPtr->isPtp &&
             (descrPtr->ptpGtsInfo.gtsEntry.ptpVersion == 2) &&
             (descrPtr->ptpActionIsLocal || isTstLong) &&
             (isTstShort == 0) )
    {
        egressTst = SKERNEL_TIMESTAMP_TAG_TYPE_HYBRID_E;
    }
    else
    {
        egressTst = SKERNEL_TIMESTAMP_TAG_TYPE_UNTAGGED_E;
    }

    __LOG_PARAM(egressTst);

    descrPtr->timestampTagged[SMAIN_DIRECTION_EGRESS_E] = egressTst;

    if( ((egressTst == SKERNEL_TIMESTAMP_TAG_TYPE_HYBRID_E) ||
         (egressTst == SKERNEL_TIMESTAMP_TAG_TYPE_PIGGY_BACKED_E)) &&
        ((descrPtr->ptpGtsInfo.gtsEntry.ptpVersion != 2) ||
        ((descrPtr->ptpGtsInfo.ptpMessageHeaderPtr -
              descrPtr->startFramePtr + 20) > BUFFER_HEADER_BYTES )) )
    {
        /* Invalid Egress Piggyback Interrupt */
        __LOG(("Invalid Egress Piggyback Interrupt"));

        if (SMEM_CHT_IS_SIP6_GET(devObjPtr))
        {
            regAddr = SMEM_LION3_HA_PTP_AND_TS_EXCEPTION_CONFIG_REG(devObjPtr);
            smemRegFldGet(devObjPtr, regAddr, 11, 11, &pktCmd);
            cpuCode = pktCmd >> 3;
            pktCmd &= 0x07;
            descrPtr->cpuCode = cpuCode;

            __LOG(("Apply new packet command"));

            snetChtEgressCommandAndCpuCodeResolution(devObjPtr,descrPtr,
                                              descrPtr->packetCmd,
                                              pktCmd,
                                              descrPtr->cpuCode,
                                              cpuCode
                                              );

            if (descrPtr->packetCmd == SKERNEL_EXT_PKT_CMD_HARD_DROP_E)
            {
                descrPtr->haAction.drop = SKERNEL_EXT_PKT_CMD_HARD_DROP_E;
            }
        }
        else
        {
            regAddr = SMEM_LION3_HA_PTP_AND_TS_EXCEPTION_CONFIG_REG(devObjPtr);
            regPtr = smemMemGet(devObjPtr, regAddr);

            if(SMEM_U32_GET_FIELD(*regPtr, 2,  1))
            {
                descrPtr->haAction.drop = SKERNEL_EXT_PKT_CMD_HARD_DROP_E;
            }
        }
        /* Generate Invalid Egress Piggyback interrupt */
        snetlion3HaInterruptEvent(devObjPtr, descrPtr, 12);

        /* Invalid Egress Piggyback Counter increment */
        regAddr = SMEM_LION3_HA_INVALID_PTP_OUT_PIGGYBACK_CNTR_REG(devObjPtr);
        regPtr = smemMemGet(devObjPtr, regAddr);
        fieldData = SMEM_U32_GET_FIELD(*regPtr, 0,  8);
        if(fieldData == 0xFF)
        {
            fieldData = 0;
        }
        else
        {
            fieldData++;
        }
    }
}

/**
* @internal snetLion3HaPtpExtractionAndAction function
* @endinternal
*
* @brief   PTP Field Extraction and Timestamp Action Resolution
*/
static GT_VOID snetLion3HaPtpExtractionAndAction
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32 egressPort
)
{
    DECLARE_FUNC_NAME(snetLion3HaPtpExtractionAndAction);

    GT_U32 regAddr;                     /* register's address */
    GT_U32 *regPtr;                     /* register entry pointer */
    GT_U32 fieldData;                   /* register field value */
    GT_U32 pktCmd;                      /* register field value */
    GT_U32 cpuCode;                     /* register field value */
    GT_U32  TIMESTAMP_RECEPTION_EN;

    SNET_LION_PTP_GTS_INFO_STC *gtsInfoPtr = &descrPtr->ptpGtsInfo;

    if(devObjPtr->supportPtp == 0)
    {
        /* Device doesn't support PTP message processing */
        __LOG(("Device doesn't support PTP message processing"));
        return;
    }

    if ((descrPtr->isPtp == 1) && (descrPtr->oamInfo.timeStampEnable == 0) &&
        (descrPtr->oamInfo.lmCounterInsertEnable == 0))
    {
        /* PTP fields already extracted in The TTI */

        /* Some of the PTP fields cannot be extracted */
        if( ((gtsInfoPtr->gtsEntry.ptpVersion == 2) &&
            ((gtsInfoPtr->ptpMessageHeaderPtr -
              descrPtr->startFramePtr + 44) > BUFFER_HEADER_BYTES )) ||
            ((gtsInfoPtr->gtsEntry.ptpVersion == 1) &&
            ((gtsInfoPtr->ptpMessageHeaderPtr -
              descrPtr->startFramePtr + 48) > BUFFER_HEADER_BYTES )) )
        {
            /* Invalid PTP Interrupt */
            __LOG(("Invalid PTP Interrupt"));
            descrPtr->ptpActionIsLocal = 0;
            __LOG_PARAM(descrPtr->ptpActionIsLocal);

            if (SMEM_CHT_IS_SIP6_GET(devObjPtr))
            {
                regAddr = SMEM_SIP6_HA_PTP_EXCEPTION_CPU_CODE_CONFIG_REG(devObjPtr);
                smemRegFldGet(devObjPtr, regAddr, 0, 11, &pktCmd);
                cpuCode = pktCmd >> 3;
                pktCmd &= 0x07;
                descrPtr->cpuCode = cpuCode;

                __LOG(("Apply new packet command"));

                snetChtEgressCommandAndCpuCodeResolution(devObjPtr,descrPtr,
                                                  descrPtr->packetCmd,
                                                  pktCmd,
                                                  descrPtr->cpuCode,
                                                  cpuCode
                                                  );

                if (descrPtr->packetCmd == SKERNEL_EXT_PKT_CMD_HARD_DROP_E)
                {
                    descrPtr->haAction.drop = SKERNEL_EXT_PKT_CMD_HARD_DROP_E;
                }
            }
            else
            {
                regAddr = SMEM_LION3_HA_PTP_AND_TS_EXCEPTION_CONFIG_REG(devObjPtr);
                regPtr = smemMemGet(devObjPtr, regAddr);

                if(SMEM_U32_GET_FIELD(*regPtr, 0,  1))
                {
                    descrPtr->haAction.drop = SKERNEL_EXT_PKT_CMD_HARD_DROP_E;
                    __LOG(("Assign HARD DROP \n"));
                }
                else
                {
                    __LOG(("DO NOT Assign HARD DROP \n"));
                }
           }
            /* Generate Invalid PTP interrupt */
            snetlion3HaInterruptEvent(devObjPtr, descrPtr, 10);

            /* Invalid PTP Counter increment */
            regAddr = SMEM_LION3_HA_INVALID_PTP_HEADER_CNTR_REG(devObjPtr);
            regPtr = smemMemGet(devObjPtr, regAddr);
            fieldData = SMEM_U32_GET_FIELD(*regPtr, 0,  8);
            if(fieldData == 0xFF)
            {
                fieldData = 0;
            }
            else
            {
                fieldData++;
            }
        }
        else
        {
            /* PTP Timestamp Triggering - setting of <PTP Action is Local> */
            regAddr = SMEM_LION3_HA_PTP_DOMAIN_TBL_MEM(devObjPtr,
                                                       egressPort,
                                                       descrPtr->ptpDomain);
            regPtr = smemMemGet(devObjPtr, regAddr);


            __LOG_PARAM(descrPtr->ptpTriggerType);
            __LOG_PARAM(gtsInfoPtr->gtsEntry.transportSpecific);
            __LOG_PARAM(gtsInfoPtr->gtsEntry.msgType);

            __LOG(("PTP Domain Entry : \n"));
            __LOG(("PTP over ethernet timestamp enable[%d] \n",
                snetFieldValueGet(regPtr, 0, 1)));
            __LOG(("PTP over IPv4 UDP timestamp enable[%d] \n",
                snetFieldValueGet(regPtr, 1, 1)));
            __LOG(("PTP over IPv6 UDP timestamp enable[%d] \n",
                snetFieldValueGet(regPtr, 2, 1)));

            __LOG(("Message Type <msgType> Timestamping Enable [%d] \n",
                snetFieldValueGet(regPtr, (3 + gtsInfoPtr->gtsEntry.msgType), 1)));

            __LOG(("Transport Specific <transportSpecific> Timestamping Enable [%d] \n",
                snetFieldValueGet(regPtr, (19 + gtsInfoPtr->gtsEntry.transportSpecific), 1)));

            if( (((descrPtr->ptpTriggerType == SKERNEL_PTP_TRIGGER_TYPE_PTP_OVER_L2_E) &&
                  snetFieldValueGet(regPtr, 0, 1)) ||
                  ((descrPtr->ptpTriggerType == SKERNEL_PTP_TRIGGER_TYPE_PTP_OVER_IPV4_UDP_E) &&
                  snetFieldValueGet(regPtr, 1, 1)) ||
                  ((descrPtr->ptpTriggerType == SKERNEL_PTP_TRIGGER_TYPE_PTP_OVER_IPV6_UDP_E) &&
                  snetFieldValueGet(regPtr, 2, 1))) &&
                (snetFieldValueGet(regPtr, (19 + gtsInfoPtr->gtsEntry.transportSpecific), 1)) &&
                (snetFieldValueGet(regPtr, (3 + gtsInfoPtr->gtsEntry.msgType), 1)) )
            {
                descrPtr->ptpActionIsLocal = 1;
            }
            else
            {
                descrPtr->ptpActionIsLocal = 0;
            }
            __LOG_PARAM(descrPtr->ptpActionIsLocal);
        }
    }

    /* timeStamp Reception Mode */
    if(SMEM_CHT_IS_SIP7_GET(devObjPtr))
    {
        TIMESTAMP_RECEPTION_EN = SMEM_SIP7_HA_SRC_PHYSICAL_PORT_2_ENTRY_FIELD_GET(devObjPtr,
            descrPtr->localDevSrcPort,/* the src physical port */
            SMEM_SIP7_HA_SRC_PHYSICAL_PORT_2_TABLE_FIELDS_PORT_TIMESTAMP_RECEPTION_EN_E);
    }
    else
    {
        TIMESTAMP_RECEPTION_EN = SMEM_LION3_HA_PHYSICAL_PORT_2_ENTRY_FIELD_BY_SRC_PORT_GET(devObjPtr,
            descrPtr->localDevSrcPort,/* the physical port */
            SMEM_LION3_HA_PHYSICAL_PORT_TABLE_2_FIELDS_TIMESTAMP_RECEPTION_EN);
    }

    if( (descrPtr->oamInfo.timeStampEnable == 0) &&
        TIMESTAMP_RECEPTION_EN &&
        ((descrPtr->timestampTagged[SMAIN_DIRECTION_INGRESS_E] ==
          SKERNEL_TIMESTAMP_TAG_TYPE_NON_EXTENDED_E) ||
         (descrPtr->timestampTagged[SMAIN_DIRECTION_INGRESS_E] ==
          SKERNEL_TIMESTAMP_TAG_TYPE_EXTENDED_E)) &&
        (descrPtr->timestampTagInfo[SMAIN_DIRECTION_INGRESS_E].OE) )
    {
        descrPtr->oamInfo.timeStampEnable = 1;
        descrPtr->oamInfo.offsetIndex =
            descrPtr->timestampTagInfo[SMAIN_DIRECTION_INGRESS_E].OffsetProfile;
    }

    if( ((descrPtr->timestampTagged[SMAIN_DIRECTION_INGRESS_E] ==
          SKERNEL_TIMESTAMP_TAG_TYPE_HYBRID_E) ||
         (descrPtr->timestampTagged[SMAIN_DIRECTION_INGRESS_E] ==
          SKERNEL_TIMESTAMP_TAG_TYPE_PIGGY_BACKED_E)) &&
        ((gtsInfoPtr->gtsEntry.ptpVersion != 2) ||
        ((gtsInfoPtr->ptpMessageHeaderPtr -
              descrPtr->startFramePtr + 20) > BUFFER_HEADER_BYTES )) )
    {
        /* Invalid Ingress Piggyback Interrupt */
        __LOG(("Invalid Ingress Piggyback Interrupt"));

        /* Timestamp Tag Fields derived from PTP<reserved[31:0] are '0' */
        descrPtr->timestampTagInfo[SMAIN_DIRECTION_INGRESS_E].timestamp.nanoSecondTimer = 0x0;
        descrPtr->timestampTagInfo[SMAIN_DIRECTION_INGRESS_E].U = 0;
        descrPtr->timestampTagInfo[SMAIN_DIRECTION_INGRESS_E].T = 0;

        if (SMEM_CHT_IS_SIP6_GET(devObjPtr))
        {
            regAddr = SMEM_LION3_HA_PTP_AND_TS_EXCEPTION_CONFIG_REG(devObjPtr);
            smemRegFldGet(devObjPtr, regAddr, 0, 11, &pktCmd);
            cpuCode = pktCmd >> 3;
            pktCmd &= 0x07;
            descrPtr->cpuCode = cpuCode;

            __LOG(("Apply new packet command"));

            snetChtEgressCommandAndCpuCodeResolution(devObjPtr,descrPtr,
                                              descrPtr->packetCmd,
                                              pktCmd,
                                              descrPtr->cpuCode,
                                              cpuCode
                                              );

            if (descrPtr->packetCmd == SKERNEL_EXT_PKT_CMD_HARD_DROP_E)
            {
                descrPtr->haAction.drop = SKERNEL_EXT_PKT_CMD_HARD_DROP_E;
            }
        }
        else
        {
            regAddr = SMEM_LION3_HA_PTP_AND_TS_EXCEPTION_CONFIG_REG(devObjPtr);
            regPtr = smemMemGet(devObjPtr, regAddr);

            if(SMEM_U32_GET_FIELD(*regPtr, 1,  1))
            {
                descrPtr->haAction.drop = SKERNEL_EXT_PKT_CMD_HARD_DROP_E;
            }
        }

        /* Generate Invalid Ingress Piggyback interrupt */
        snetlion3HaInterruptEvent(devObjPtr, descrPtr, 11);

        /* Invalid Ingress Piggyback Counter increment */
        regAddr = SMEM_LION3_HA_INVALID_PTP_IN_PIGGYBACK_CNTR_REG(devObjPtr);
        regPtr = smemMemGet(devObjPtr, regAddr);
        fieldData = SMEM_U32_GET_FIELD(*regPtr, 0,  8);
        if(fieldData == 0xFF)
        {
            fieldData = 0;
        }
        else
        {
            fieldData++;
        }
    }

    snetLion3HaOutgoingTimestampTag(devObjPtr, descrPtr, egressPort);

    /* Timestamp Selection */
    if( TIMESTAMP_RECEPTION_EN &&
        (descrPtr->timestampTagged[SMAIN_DIRECTION_INGRESS_E] !=
          SKERNEL_TIMESTAMP_TAG_TYPE_UNTAGGED_E) )
    {
        descrPtr->ptpIsTimestampLocal = 0;
        descrPtr->ptpUField = descrPtr->timestampTagInfo[SMAIN_DIRECTION_INGRESS_E].U;

        /* in Ironman descrPtr->ptpTaiSelect is set by the TTI */
        if(!SMEM_CHT_IS_SIP6_30_GET(devObjPtr))
        {
            descrPtr->ptpTaiSelect = descrPtr->timestampTagInfo[SMAIN_DIRECTION_INGRESS_E].T;
        }
    }
    else
    {
        descrPtr->ptpIsTimestampLocal = 1;
        /* other related fields are already in the descriptor */
    }
}


/**
* @internal lion3HaInfoForEgressPipe function
* @endinternal
*
* @brief   HA - add info for the EPCL , and other units to use down the egress pipe.
*
* @param[in] devObjPtr                -  pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in] egressPort               - egress port.
*/
void lion3HaInfoForEgressPipe
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32   egressPort
)
{
    DECLARE_FUNC_NAME(lion3HaInfoForEgressPipe);

    GT_U32  pha_threadId;

    descrPtr->haToEpclInfo.qos.qosMapTableIndex =
        SMEM_LION3_HA_EPORT_1_ENTRY_FIELD_AUTO_GET(devObjPtr,
            descrPtr,
            SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_QOS_MAPPING_TABLE_INDEX);

    descrPtr->haToEpclInfo.qos.egressExpMapEn =
        SMEM_LION3_HA_EPORT_1_ENTRY_FIELD_AUTO_GET(devObjPtr,
            descrPtr,
            SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_ENABLE_EGRESS_EXP_MAPPING);

    descrPtr->haToEpclInfo.qos.egressUpMapEn =
        SMEM_LION3_HA_EPORT_1_ENTRY_FIELD_AUTO_GET(devObjPtr,
            descrPtr,
            SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_ENABLE_EGRESS_UP_MAPPING);

    descrPtr->haToEpclInfo.qos.egressDscpMapEn =
        SMEM_LION3_HA_EPORT_1_ENTRY_FIELD_AUTO_GET(devObjPtr,
            descrPtr,
            SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_ENABLE_EGRESS_DSCP_MAPPING);

    descrPtr->haToEpclInfo.qos.egressTcDpMapEn =
        SMEM_LION3_HA_EPORT_1_ENTRY_FIELD_AUTO_GET(devObjPtr,
            descrPtr,
            SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_ENABLE_EGRESS_TC_DP_MAPPING);

    descrPtr->haToEpclInfo.qos.egressDpToCfiMapEn =
        SMEM_LION3_HA_EPORT_1_ENTRY_FIELD_AUTO_GET(devObjPtr,
            descrPtr,
            SMEM_LION3_HA_EPORT_TABLE_1_FIELDS_DP_TO_CFI_MAP_ENABLE);

    if(SMEM_CHT_IS_SIP6_GET(devObjPtr))
    {
        pha_threadId =
            SMEM_LION3_HA_EPORT_1_ENTRY_FIELD_AUTO_GET(devObjPtr,
                descrPtr,
                SMEM_SIP6_HA_EPORT_TABLE_1_FIELDS_PHA_THREAD_NUMBER);

        /* override the descriptor only if value !=0 in the entry */
        if(pha_threadId)
        {
            descrPtr->pha.pha_threadId = pha_threadId;

            __LOG_PARAM(descrPtr->pha.pha_threadId);
        }
    }


    __LOG_PARAM(descrPtr->haToEpclInfo.qos.qosMapTableIndex);
    __LOG_PARAM(descrPtr->haToEpclInfo.qos.egressTcDpMapEn);
    __LOG_PARAM(descrPtr->haToEpclInfo.qos.egressExpMapEn);
    __LOG_PARAM(descrPtr->haToEpclInfo.qos.egressDscpMapEn);
    __LOG_PARAM(descrPtr->haToEpclInfo.qos.egressUpMapEn);
    __LOG_PARAM(descrPtr->haToEpclInfo.qos.egressDpToCfiMapEn);
}

/**
* @internal lion3HaUnitInitInfoGet function
* @endinternal
*
* @brief   HA - get some init info
*
* @param[in] devObjPtr                -  pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in] egressPort               - egress port.
*/
static void  lion3HaUnitInitInfoGet
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32           egressPort
)
{
    DECLARE_FUNC_NAME(lion3HaUnitInitInfoGet);

    GT_U32  remotePhyPortMapEnable;
    GT_U32  tmpValue;

    descrPtr->eArchExtInfo.haInfo.forceNewDsaFwdFromCpu = 0;
    descrPtr->eArchExtInfo.haInfo.doubleTagToCpu = 0;
    descrPtr->forceToAddFromCpu4BytesDsaTag = 0;

    switch (descrPtr->outGoingMtagCmd)
    {
        case SKERNEL_MTAG_CMD_FROM_CPU_E:
            descrPtr->eArchExtInfo.haInfo.forceNewDsaFwdFromCpu = SMEM_LION3_HA_PHYSICAL_PORT_2_ENTRY_FIELD_AUTO_GET(devObjPtr,
                descrPtr,
                SMEM_LION3_HA_PHYSICAL_PORT_TABLE_2_FIELDS_FORCE_NEW_DSA_FORWARD_OR_FROM_CPU);
            if(SMEM_CHT_IS_SIP5_15_GET(devObjPtr))
            {
                descrPtr->forceToAddFromCpu4BytesDsaTag =
                    SMEM_LION3_HA_PHYSICAL_PORT_2_ENTRY_FIELD_AUTO_GET(devObjPtr,
                        descrPtr,
                        SMEM_SIP5_15_HA_PHYSICAL_PORT_TABLE_2_FIELDS_TRANS_FROM_CPU_TO_4B_FROM_CPU_DSA);
            }

            break;
        case SKERNEL_MTAG_CMD_FORWARD_E:
            descrPtr->eArchExtInfo.haInfo.forceNewDsaFwdFromCpu = SMEM_LION3_HA_PHYSICAL_PORT_2_ENTRY_FIELD_AUTO_GET(devObjPtr,
                descrPtr,
                SMEM_LION3_HA_PHYSICAL_PORT_TABLE_2_FIELDS_FORCE_NEW_DSA_FORWARD_OR_FROM_CPU);

            if(SMEM_CHT_IS_SIP5_15_GET(devObjPtr))
            {
                descrPtr->forceToAddFromCpu4BytesDsaTag =
                    SMEM_LION3_HA_PHYSICAL_PORT_2_ENTRY_FIELD_AUTO_GET(devObjPtr,
                        descrPtr,
                        SMEM_SIP5_15_HA_PHYSICAL_PORT_TABLE_2_FIELDS_TRANS_FWD_TO_4B_FROM_CPU_DSA);
            }
            break;
        case SKERNEL_MTAG_CMD_TO_TRG_SNIFFER_E:
            if(SMEM_CHT_IS_SIP5_15_GET(devObjPtr))
            {
                descrPtr->forceToAddFromCpu4BytesDsaTag =
                    SMEM_LION3_HA_PHYSICAL_PORT_2_ENTRY_FIELD_AUTO_GET(devObjPtr,
                        descrPtr,
                        SMEM_SIP5_15_HA_PHYSICAL_PORT_TABLE_2_FIELDS_TRANS_TO_ANALYZER_TO_4B_FROM_CPU_DSA);
            }
            break;
        default:/*to cpu*/
            descrPtr->eArchExtInfo.haInfo.doubleTagToCpu = SMEM_LION3_HA_PHYSICAL_PORT_2_ENTRY_FIELD_AUTO_GET(devObjPtr,
                descrPtr,
                SMEM_LION3_HA_PHYSICAL_PORT_TABLE_2_FIELDS_FORCE_NEW_DSA_TO_CPU);
            break;
    }

    remotePhyPortMapEnable =
                SMEM_LION3_HA_PHYSICAL_PORT_2_ENTRY_FIELD_AUTO_GET(devObjPtr,
                    descrPtr,
                    SMEM_LION3_HA_PHYSICAL_PORT_TABLE_2_FIELDS_REMOTE_PHYSICAL_PORT_MAP_ENABLE);

    if(descrPtr->forceToAddFromCpu4BytesDsaTag && !remotePhyPortMapEnable)
    {
        __LOG(("Warning : the 'adding of 4 Bytes FROM_CPU DSA' is IGNORED when not doing 'remote physical port mapping' \n"));

        descrPtr->forceToAddFromCpu4BytesDsaTag = 0;
    }


    if((descrPtr->outGoingMtagCmd == SKERNEL_MTAG_CMD_TO_CPU_E && descrPtr->srcTrg == 0)/*Src*/ ||
       (descrPtr->outGoingMtagCmd == SKERNEL_MTAG_CMD_TO_TRG_SNIFFER_E && descrPtr->rxSniff) )
    {
        /* packet that goes to CPU (from ingress pipe) should have its original payload ,
           packet that goes to analyzer (rxMirror) should have its original payload */
        descrPtr->eArchExtInfo.haInfo.rxTrappedOrMonitored = 1;
    }
    else
    {
        descrPtr->eArchExtInfo.haInfo.rxTrappedOrMonitored = 0;
    }


    __LOG_PARAM(descrPtr->forceToAddFromCpu4BytesDsaTag);
    __LOG_PARAM(descrPtr->eArchExtInfo.haInfo.doubleTagToCpu);
    __LOG_PARAM(descrPtr->eArchExtInfo.haInfo.forceNewDsaFwdFromCpu);
    __LOG_PARAM(descrPtr->eArchExtInfo.haInfo.rxTrappedOrMonitored);

    /* save values at were at start of the HA before setting the 'egress values' */
    descrPtr->eArchExtInfo.haInfo.tpidIndex_atStartOfHa[SNET_CHT_TAG_0_INDEX_CNS] =
        descrPtr->tpidIndex[SNET_CHT_TAG_0_INDEX_CNS] ;
    descrPtr->eArchExtInfo.haInfo.tpidIndex_atStartOfHa[SNET_CHT_TAG_1_INDEX_CNS] =
        descrPtr->tpidIndex[SNET_CHT_TAG_1_INDEX_CNS] ;

    __LOG_PARAM(descrPtr->eArchExtInfo.haInfo.tpidIndex_atStartOfHa[SNET_CHT_TAG_0_INDEX_CNS]);
    __LOG_PARAM(descrPtr->eArchExtInfo.haInfo.tpidIndex_atStartOfHa[SNET_CHT_TAG_1_INDEX_CNS]);

    /* save the info for later use */
    descrPtr->eArchExtInfo.haInfo.nestedVlanAccessPort_atStartOfHa =
        descrPtr->nestedVlanAccessPort;
    __LOG_PARAM(descrPtr->eArchExtInfo.haInfo.nestedVlanAccessPort_atStartOfHa);

    if(SMEM_CHT_IS_SIP6_10_GET(devObjPtr) &&
        SMEM_LION3_HA_PHYSICAL_PORT_1_ENTRY_FIELD_AUTO_GET(devObjPtr,descrPtr,
        SMEM_HAWK_HA_PHYSICAL_PORT_TABLE_1_FIELDS_SET_SRC_ID_E))
    {
        GT_U32  src_id_len,src_id_msb;
        smemRegFldGet(devObjPtr, SMEM_SIP5_25_HA_GLOBAL_CONFIG_2_REG(devObjPtr),
            6 , 3 ,
            &src_id_len);

        smemRegFldGet(devObjPtr, SMEM_SIP5_25_HA_GLOBAL_CONFIG_2_REG(devObjPtr),
            9 , 4 ,
            &src_id_msb);

        __LOG_PARAM(src_id_msb);
        __LOG_PARAM(src_id_len);
        __LOG_PARAM(descrPtr->sstId);

        tmpValue = SMEM_U32_GET_FIELD(descrPtr->sstId,(src_id_msb-src_id_len+1) , src_id_len);

        __LOG(("'Value' = inDesc<SST ID>[<Src ID msb>:<Src ID msb>- <Src ID len>+1] = [0x%3.3x] \n",
            tmpValue));
        if(tmpValue == 0)
        {
            tmpValue =
                SMEM_LION3_HA_PHYSICAL_PORT_1_ENTRY_FIELD_AUTO_GET(devObjPtr,
                    descrPtr,
                    SMEM_HAWK_HA_PHYSICAL_PORT_TABLE_1_FIELDS_SRC_ID_E);

            __LOG(("update the bits[%d..%d] in the sstId \n",
                (src_id_msb-src_id_len+1), /*first bit*/
                src_id_msb));/*last bit*/

            SMEM_U32_SET_FIELD(descrPtr->sstId,(src_id_msb-src_id_len+1) , src_id_len , tmpValue);

            __LOG_PARAM(descrPtr->sstId);
        }
        else
        {
            __LOG(("As 'Value' [0x%3.3x]  != 0 --> no SRC-ID change based on egress port \n",
                tmpValue));
        }
    }

    if(SMEM_CHT_IS_SIP6_30_GET(devObjPtr) && (descrPtr->dstEpg == 0))
    {
        tmpValue = SMEM_LION3_HA_PHYSICAL_PORT_1_ENTRY_FIELD_AUTO_GET(devObjPtr, descrPtr,
                    SMEM_SIP6_30_HA_PHYSICAL_PORT_TABLE_1_FIELDS_SET_DESTINATION_EPG_E);
        if (tmpValue)
        {
            /* update Destination EPG value */
            descrPtr->dstEpg = SMEM_LION3_HA_PHYSICAL_PORT_1_ENTRY_FIELD_AUTO_GET(devObjPtr, descrPtr,
                    SMEM_SIP6_30_HA_PHYSICAL_PORT_TABLE_1_FIELDS_DESTINATION_EPG_E);

            __LOG(("Update Destination EPG value in dstEpg to [%d]\n",descrPtr->dstEpg));
        }
    }
    return;
}


/**
* @internal snetChtHaCncClients function
* @endinternal
*
* @brief   HA - handle the CNC clients
*
* @param[in] devObjPtr                -  pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
*/
static void  snetChtHaCncClients
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr
)
{
    DECLARE_FUNC_NAME(snetChtHaCncClients);

    if(descrPtr->haAction.drop == SKERNEL_EXT_PKT_CMD_HARD_DROP_E)
    {
        __LOG(("do not do CNC counting when packet dropped by the HA \n"));
        return;
    }

    if(descrPtr->outGoingMtagCmd != SKERNEL_MTAG_CMD_FORWARD_E &&
       descrPtr->outGoingMtagCmd != SKERNEL_MTAG_CMD_FROM_CPU_E)
    {
        /* taken from bc2 verifier */
        __LOG(("Packet outgoing Mtag command is not FORWARD/FROM_CPU. No CNC updates. \n"));
        return;
    }

    /* NOTE: we need to do the HA unit CNC client with values of 'egress byte count'
      and only know we know those ... */
    if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        /* count 'per target eport' */
        __LOG(("Check for CNC client : target EPort \n"));
        snetCht3CncCount(devObjPtr, descrPtr,
                         SNET_CNC_CLIENT_TARGET_EPORT_E, 0);
    }

    if (descrPtr->tunnelStart)
    {
        /* Tunnel Start CNC Trigger */
        __LOG(("Check for CNC client : Tunnel Start \n"));
        snetCht3CncCount(devObjPtr, descrPtr,
                         SNET_CNC_CLIENT_TUNNEL_START_INDEX_E,
                         0);
    }

    if (descrPtr->isL2Nat)
    {
        /* Tunnel Start CNC Trigger */
        __LOG(("Check for CNC client : L2 NAT (ARP) \n"));
        snetCht3CncCount(devObjPtr, descrPtr,
                         SNET_CNC_CLIENT_ARP_INDEX_E,
                         0);
    }

    if(descrPtr->haToEpclInfo.useArpForDa == GT_TRUE ||
       descrPtr->haToEpclInfo.useArpForSa == GT_TRUE)
    {
        if(descrPtr->haToEpclInfo.useArpForDa == GT_TRUE)
        {
            if (descrPtr->isNat)
            {
                /* ARP Table CNC Trigger */
                __LOG(("Check for CNC client : NAT (ARP) index (for MAC DA) \n"));
            }
            else
            {
                /* ARP Table CNC Trigger */
                __LOG(("Check for CNC client : NAT index (for MAC DA) \n"));
            }

        }
        else
        {
            /* ARP Table CNC Trigger */
            __LOG(("Check for CNC client : ARP index (for MAC SA) \n"));
        }

        snetCht3CncCount(devObjPtr, descrPtr, SNET_CNC_CLIENT_ARP_INDEX_E,
            0);
    }
}

/**
* @internal snetChtHaInitPhaMetadata function
* @endinternal
*
* @brief   HA - handle the PHA Metadata init
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
*/
static void snetChtHaInitPhaMetadata
(
    IN SKERNEL_DEVICE_OBJECT            *devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC  *descrPtr
)
{
    GT_STATUS           rc;
    GT_U32              udbId = 54;     /* UDB Start index for PHA metadata */
    GT_U32              i;
    GT_U8               phaMetadata[6]; /* udbIndex 54 -> Array Index 0, udbIndex 59 -> Array Index 5 */

    DECLARE_FUNC_NAME(snetChtHaInitPhaMetadata);

    /* Copy 6 UDBs to PHA metadata */
    for(i = 0; i < 6; i++, udbId++)
    {
        rc = lion3EPclUdbKeyValueGet(devObjPtr, descrPtr, udbId, &phaMetadata[i]);
        if(rc != GT_OK)
        {
            __LOG(("lion3EPclUdbKeyValueGet Failed\n"));
            return;
        }
    }

    /* UDB Assignment in case SGT Threads
     *     UDB57 => phaMetadata[3] =  UP + CFI (4Bits)
     *     UDB56 => phaMetadata[2] =  VLAN ID - MSB(4Bits)
     *     UDB55 => phaMetadata[1] =  VLAN ID - LSB(8Bits)
     *     UDB54 => phaMetadata[0] =  EgressTag0Exists(1bit) + EgressInnerSpecialTag(3bits)
     *
     *     -----------------------------------------------------------------------------------------------
     *    |    phaMD[5]   |   phaMD[4]    |   phaMD[3]    |   phaMD[2]    |   phaMD[1]    |   phaMD[0]    |
     *     -----------------------------------------------------------------------------------------------
     *    |7|6|5|4|3|2|1|0|7|6|5|4|3|2|1|0|7|6|5|4|3|2|1|0|7|6|5|4|3|2|1|0|7|6|5|4|3|2|1|0|7|6|5|4|3|2|1|0|
     *     -----------------------------------------------------------------------------------------------
     *                                    |v v v v        |        v v v v|v v v v v v v v|  v v v   v    |
     *                                       -up-                  ----------vid----------           EgressTag0Exists
     *                                     cfi                                               EgressInnerSpecialTag
     */

    descrPtr->pha.pha_metadata_ext[1] = phaMetadata[5];
    descrPtr->pha.pha_metadata_ext[0] = phaMetadata[4];

    descrPtr->pha.pha_metadata[0]    = ((phaMetadata[3] << 24) |
                                        (phaMetadata[2] << 16) |
                                        (phaMetadata[1] << 8)  |
                                         phaMetadata[0]);

}

/**
* @internal snetChtHaMain function
* @endinternal
*
* @brief   HA - main HA unit logic
*
* @param[in] devObjPtr                -  pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in] egressPort               - egress port.
* @param[in] destVlanTagged           - send frame with tag.
*/
void  snetChtHaMain
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U32   egressPort,
    IN GT_U8    destVlanTagged,
    OUT GT_U8 **frameDataPtrPtr,
    OUT GT_U32 *frameDataSize,
    OUT GT_BOOL *isPciSdma
)
{
    DECLARE_FUNC_NAME(snetChtHaMain);

    GT_U32 outerVid = 0;
    GT_BIT modifiyVid = 0;
    GT_U32 oversizeTags;
    GT_U32 maxTagsLength;/* the max number of bytes that the HA allows */
    CHT_PCL_EXTRA_PACKET_INFO_STC   pclExtraData;

    simLogPacketFrameUnitSet(SIM_LOG_FRAME_UNIT_HA_E);

    __LOG(("HA - start handle for egress port[%d] \n",
        egressPort));

    /* Set packetCmd assigned by egress */
    descrPtr->packetCmdAssignedByEgress = 0;
    if(descrPtr->l2Valid)
    {
        /* offset from L2 (but after DSA tag) to L3 */
        oversizeTags = descrPtr->origVlanTagLength +
                       descrPtr->afterVlanOrDsaTagLen;

        if(descrPtr->l2Encaps != SKERNEL_LLC_E)
        {
            /* remove 2 of the ethertype */
            oversizeTags -= 2;
        }
    }
    else
    {
        oversizeTags = 0;
    }

    if(SMEM_CHT_IS_SIP6_GET(devObjPtr))
    {
        maxTagsLength = 32;/* 32 bytes = 8 tags of 4 bytes */
    }
    else
    if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        maxTagsLength = 20;/* 20 bytes = 5 tags of 4 bytes */
    }
    else
    {
        maxTagsLength = 0xFFFFFFFF;/* dummy value for BWC behavior */
    }

    if(oversizeTags > maxTagsLength)
    {
        /*Design can hold up to 5 VLAN tags of the incoming passenger packet (at header partition stage).
          If there are more than 5 VLAN tags, packet is dropped and interrupt issued.*/
        __LOG(("WARNING : The HA recognized too many vlan tags ([%d] bytes --> more than [%d] bytes) so packet is dropped (will not egress the port) \n",
            oversizeTags ,maxTagsLength));

        /* Header Alteration can handle up to 20 bytes of tags (including LLC/SNAP).
           The tags size is determined starting from the end of the MACs
           (or after the DSA and/or Timestamp if exist) to the Layer3 offset. */

        __LOG(("Generate interrupt <Oversize Tag received> \n"));
        /* Generate HA interrupt */
        snetChetahDoInterrupt(devObjPtr,
                              SMEM_LION3_HA_INTERRUPT_CAUSE_REG(devObjPtr),
                              SMEM_LION3_HA_INTERRUPT_MASK_REG(devObjPtr),
                              (1<<5),/*Oversize Tag received*/
                              (GT_U32)SMEM_LION3_HA_SUM_INT(devObjPtr));

        /* indicate to drop that packet */
        descrPtr->haAction.drop = SKERNEL_EXT_PKT_CMD_HARD_DROP_E;

        __LOG(("HA - end handle for egress port[%d] (assign hard drop) \n",
            egressPort));
        return;
    }

    if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        /* get the info about the physical port (table1,2) */
        snetChtHaEgressPhyPortEntryGet(devObjPtr,descrPtr,egressPort);
        /* get the info about the eport (table1,2) */
        snetChtHaEgressEPortEntryGet(devObjPtr,descrPtr,egressPort);

        /* HA - Egress eVlan Table */
        descrPtr->eArchExtInfo.haEgressVlanTablePtr =
            smemMemGet(devObjPtr,
                SMEM_CHT3_EGR_VLAN_TRANS_TBL_MEM(devObjPtr,descrPtr->eVid));

        /*get some global info */
        lion3HaUnitInitInfoGet(devObjPtr, descrPtr,egressPort);

        /* HA PTP support for PTP Field Extraction and Timestamp Action */
        snetLion3HaPtpExtractionAndAction(devObjPtr, descrPtr, egressPort);
    }

    chtHaUnit(devObjPtr,descrPtr,egressPort,destVlanTagged,
                frameDataPtrPtr,frameDataSize,isPciSdma);

    if(SMEM_CHT_IS_SIP6_10_GET(devObjPtr))
    {
        /* Parse extra data needs by EPCL engine */
        __LOG(("Parse extra data needs by EPCL engine"));
        snetChtPclSrvParseExtraData(devObjPtr, descrPtr, &pclExtraData);
        descrPtr->pclExtraDataPtr = &pclExtraData;

        /* Init PHA metadata from UDBs */
        snetChtHaInitPhaMetadata(devObjPtr, descrPtr);
    }

    if(simLogIsOpenFlag)/* LOG indication about the pointers HA to EPCL */
    {
        __LOG_IS_PTR_VALID(descrPtr->haToEpclInfo.macDaSaPtr);
        __LOG_IS_PTR_VALID(descrPtr->haToEpclInfo.outerVlanTagPtr);
        __LOG_IS_PTR_VALID(descrPtr->haToEpclInfo.timeStampTagPtr);
        __LOG_IS_PTR_VALID(descrPtr->haToEpclInfo.vlanTag0Ptr);
        __LOG_IS_PTR_VALID(descrPtr->haToEpclInfo.vlanTag1Ptr);

        __LOG_IS_PTR_VALID(descrPtr->haToEpclInfo.l3StartOffsetPtr);

        __LOG_IS_PTR_VALID(descrPtr->haToEpclInfo.tunnelStartL3StartOffsetPtr);
        __LOG_IS_PTR_VALID(descrPtr->haToEpclInfo.tunnelStartL2StartOffsetPtr);
    }

    if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        /* set channel type profile for MPLS packets */
        snetLion3HaOamChannelTypeProfile(devObjPtr, descrPtr);

        lion3HaInfoForEgressPipe(devObjPtr, descrPtr , egressPort);
    }

    /* save the value for the egress counters */
    descrPtr->egressByteCount = *frameDataSize;

    /* save info for the EPCL */
    descrPtr->egressTagged = descrPtr->haToEpclInfo.outerVlanTagPtr ? 1 : 0;

    if(SMEM_CHT_IS_SIP6_30_GET(devObjPtr))
    {
        /* check for PRP trailer info */
        /* done after we have 'egressByteCount' */
        snetSip6_30HaPrpTrailerBuild(devObjPtr,descrPtr,destVlanTagged);
    }

    /* HA - PTP Support */
    __LOG(("HA - PTP Support"));
    snetLionHaPtpEgressTimestampProcess(devObjPtr, descrPtr, egressPort);

    if(descrPtr->haToEpclInfo.outerVlanTagPtr && descrPtr->haToEpclInfo.modifyVid )
    {
        modifiyVid = 1;
        outerVid = descrPtr->haToEpclInfo.outerVid;
    }

    if(descrPtr->haToEpclInfo.outerVlanTagPtr && descrPtr->haToEpclInfo.xponChanges)
    {
        modifiyVid = 1;
        outerVid = descrPtr->haToEpclInfo.xPonVid;
    }

    if(modifiyVid)
    {
        /* setting outer VID --- for EPCL to use */
        __LOG(("setting outer VID --- for EPCL to use"));
        descrPtr->haToEpclInfo.outerVlanTagPtr[2] &= ~(0xf);
        descrPtr->haToEpclInfo.outerVlanTagPtr[2] |= (outerVid >> 8);
        descrPtr->haToEpclInfo.outerVlanTagPtr[3] = outerVid & 0xFF;
    }

    /* NOTE: we need to do the HA unit CNC client with values of 'egress byte count' for eArch.
      (and only now we know it ...) */
    snetChtHaCncClients(devObjPtr, descrPtr);

    __LOG(("HA - end handle for egress port[%d] \n",
        egressPort));

}

/**
* @internal snetHaTablesFormatInit function
* @endinternal
*
* @brief   init the format of HA tables.
*
* @param[in] devObjPtr                - pointer to device object.
*/
void snetHaTablesFormatInit(
    IN SKERNEL_DEVICE_OBJECT            * devObjPtr
)
{
    LION3_TABLES_FORMAT_INIT_MAC(
        devObjPtr, SKERNEL_TABLE_FORMAT_HA_PHYSICAL_PORT_1_E,
        lion3HaPhysicalPort1TableFieldsFormat, lion3HaPhysicalPort1FieldsTableNames);

    if(SMEM_CHT_IS_SIP7_GET(devObjPtr))
    {
        LION3_TABLES_FORMAT_INIT_MAC(
            devObjPtr, SKERNEL_TABLE_FORMAT_HA_PHYSICAL_PORT_2_E,
            sip7HaPhysicalPort2TableFieldsFormat, lion3HaPhysicalPort2FieldsTableNames);
    }
    else
    if(SMEM_CHT_IS_SIP6_GET(devObjPtr) == 0)
    {
        LION3_TABLES_FORMAT_INIT_MAC(
            devObjPtr, SKERNEL_TABLE_FORMAT_HA_PHYSICAL_PORT_2_E,
            lion3HaPhysicalPort2TableFieldsFormat, lion3HaPhysicalPort2FieldsTableNames);
    }
    else
    {
        LION3_TABLES_FORMAT_INIT_MAC(
            devObjPtr, SKERNEL_TABLE_FORMAT_HA_PHYSICAL_PORT_2_E,
            sip6HaPhysicalPort2TableFieldsFormat, lion3HaPhysicalPort2FieldsTableNames);
    }

    if(SMEM_CHT_IS_SIP7_GET(devObjPtr))
    {
        /* sip7 init SKERNEL_TABLE_FORMAT_HA_EGRESS_EPORT_1_E table
           in snetAasHaTablesFormatInit() */
    }
    else
    if(SMEM_CHT_IS_SIP5_10_GET(devObjPtr) == 0)
    {    /*sip 5*/
        LION3_TABLES_FORMAT_INIT_MAC(
            devObjPtr, SKERNEL_TABLE_FORMAT_HA_EGRESS_EPORT_1_E,
            lion3HaEPort1TableFieldsFormat, lion3HaEPort1FieldsTableNames);
    }
    else
    if(SMEM_CHT_IS_SIP5_15_GET(devObjPtr) == 0)
    {   /* sip 5_10 */
        LION3_TABLES_FORMAT_INIT_MAC(
            devObjPtr, SKERNEL_TABLE_FORMAT_HA_EGRESS_EPORT_1_E,
            sip5_10HaEPort1TableFieldsFormat, lion3HaEPort1FieldsTableNames);
    }
    else
    if(SMEM_CHT_IS_SIP5_20_GET(devObjPtr) == 0)
    {   /* sip 5_15 */
        LION3_TABLES_FORMAT_INIT_MAC(
            devObjPtr, SKERNEL_TABLE_FORMAT_HA_EGRESS_EPORT_1_E,
            sip5_15HaEPort1TableFieldsFormat, lion3HaEPort1FieldsTableNames);
    }
    else
    {    /* sip 5_20 */
        LION3_TABLES_FORMAT_INIT_MAC(
            devObjPtr, SKERNEL_TABLE_FORMAT_HA_EGRESS_EPORT_1_E,
            sip5_20HaEPort1TableFieldsFormat, lion3HaEPort1FieldsTableNames);
    }

    if(SMEM_CHT_IS_SIP5_10_GET(devObjPtr) == 0)
    {
        LION3_TABLES_FORMAT_INIT_MAC(
            devObjPtr, SKERNEL_TABLE_FORMAT_HA_EGRESS_EPORT_2_E,
            lion3HaEPort2TableFieldsFormat, lion3HaEPort2FieldsTableNames);
    }
    else
    {
        LION3_TABLES_FORMAT_INIT_MAC(
            devObjPtr, SKERNEL_TABLE_FORMAT_HA_EGRESS_EPORT_2_E,
            sip5_10HaEPort2TableFieldsFormat, lion3HaEPort2FieldsTableNames);
    }

    if(SMEM_CHT_IS_SIP7_GET(devObjPtr))
    {
        LION3_TABLES_FORMAT_INIT_MAC(
            devObjPtr, SKERNEL_TABLE_FORMAT_TUNNEL_START_E,
            sip7TunnelStartTableFieldsFormat, lion3HaTunnelStartFieldsTableNames);
    }
    else
    {
        LION3_TABLES_FORMAT_INIT_MAC(
            devObjPtr, SKERNEL_TABLE_FORMAT_TUNNEL_START_E,
            lion3TunnelStartTableFieldsFormat, lion3HaTunnelStartFieldsTableNames);
    }

    LION3_TABLES_FORMAT_INIT_MAC(
        devObjPtr, SKERNEL_TABLE_FORMAT_HA_NAT44_E,
        lion3HaNat44TableFieldsFormat, lion3HaNat44FieldsTableNames);

    if(SMEM_CHT_IS_SIP6_GET(devObjPtr) == 0)
    {
        LION3_TABLES_FORMAT_INIT_MAC(
        devObjPtr, SKERNEL_TABLE_FORMAT_HA_NAT66_E,
        sip5_15HaNat66TableFieldsFormat, sip5_15HaNat66FieldsTableNames);
    }
    else
    {
        LION3_TABLES_FORMAT_INIT_MAC(
        devObjPtr, SKERNEL_TABLE_FORMAT_HA_NAT66_E,
        sip6HaNat66TableFieldsFormat ,sip5_15HaNat66FieldsTableNames);
    }

    LION3_TABLES_FORMAT_INIT_MAC(
        devObjPtr, SKERNEL_TABLE_FORMAT_HA_GENERIC_TS_PROFILE_E,
        lion3HaGenericTsProfileTableFieldsFormat, lion3HaGenericTsProfileFieldsTableNames);

}

/**
* @internal snetHaSip5EgressTagAfter4BytesExtension function
* @endinternal
*
* @brief  set the egress tag extra bytes after the first 4 bytes
* @param[in] devObjPtr                - (pointer to) device object.
* @param[in] descrPtr                 - (pointer to) frame descriptor
* @param[in] haInfoPtr                - (pointer to) the HA internal info
* @param[in] isTag0                   - indication for tag0 or tag1
* @param[inout] egressTagPtr          - the (start of) buffer that hold egress tag0 or tag1
*                                       the first 4 bytes expected to hold info set by snetChtHaEgressTagDataExtGet
*/
void snetHaSip5EgressTagAfter4BytesExtension
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN HA_INTERNAL_INFO_STC *haInfoPtr,
    IN GT_BOOL      isTag0,
    INOUT GT_U8   *egressTagPtr
)
{
    if(!isTag0 /*tag1*/ && SMEM_CHT_IS_SIP6_30_GET(devObjPtr))
    {
        snetHaSip6_30EgressTag1After4BytesExtension(devObjPtr,
            descrPtr,haInfoPtr,egressTagPtr);
        return;
    }
    else
    {
        DECLARE_FUNC_NAME(snetHaSip5EgressTagAfter4BytesExtension);

        GT_U32  tagLength      = isTag0 ?  haInfoPtr->tag0Length : haInfoPtr->tag1Length;
        GT_U8   *ingressTagPtr = isTag0 ?  descrPtr->tag0Ptr    : descrPtr->tag1Ptr;
        GT_CHAR*    tagName    = isTag0 ?  "tag0"               : "tag1";

        if(tagLength <= 4)
        {
            return;
        }

        if(ingressTagPtr)
        {
            /* copy extra bytes from the ingress */
            memcpy(&egressTagPtr[4],&ingressTagPtr[4],tagLength-4);
            __LOG(("Egress [%s] is [%d] bytes , copy last [%d] bytes from ingress [%s] \n",
                tagName,
                tagLength,
                tagLength-4,
                tagName));
        }
        else
        {
            /* don't have tag in the 'original packet' .. pad with 0 */
            memset(&egressTagPtr[4],0,tagLength-4);
            __LOG(("Egress [%s] is [%d] bytes , ZERO last [%d] bytes (because ingress without [%s]) \n",
                tagName,
                tagLength,
                tagLength-4,
                tagName));
        }

        return;
    }
}

/**
* @internal snetSip6_30HaHsrTagBuild function
* @endinternal
*
* @brief   HA build HSR tag
*
* @param[in] devObjPtr                -  pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in] haInfoPtr                - (pointer to) the HA internal info
* @param[inout] egressTagPtr          - the (start of) buffer that hold egress tag1
*
*/
static void  snetSip6_30HaHsrTagBuild
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN HA_INTERNAL_INFO_STC *haInfoPtr,
    INOUT GT_U8   *egressTagPtr
)
{
    DECLARE_FUNC_NAME(snetSip6_30HaHsrTagBuild);

    GT_U32  TimestampTagSize;
    GT_U32  Tag0Size;
    GT_U32  HSR_BaseLSDUoffset;
    GT_U32  Tag1AddedSize;
    GT_U32  Offset2LSDU;
    GT_U32  LSDUSize;
    GT_U32  Path_ID;
    GT_U32  SeqNr;
    GT_U16  _2bytes;
    GT_U32  Remove = 0;
    GT_U32  prp_trailer_action;

    prp_trailer_action =
        SMEM_LION3_HA_PHYSICAL_PORT_1_ENTRY_FIELD_AUTO_GET(devObjPtr,
            descrPtr,
            SMEM_SIP6_30_HA_PHYSICAL_PORT_TABLE_1_FIELDS_PRP_TRAILER_ACTION_E);

    if(prp_trailer_action == 0x2)/*REMOVE_PRP*/
    {
        __LOG(("Packet need to add HSR tag and Remove PRP trailer ! \n"));
        Remove = 1;
    }
    else
    if(prp_trailer_action == 0x1)/*ADD_PRP*/
    {
        __LOG(("WARNING : Configuration ERROR : packet need to add HSR tag and PRP trailer ?! \n"));
    }

    if(haInfoPtr->tag1Length != 6)
    {
        __LOG(("WARNING : Configuration ERROR : TPID with 'HSR tag = 1' but the size is [%d] bytes instead of 6 bytes \n",
            haInfoPtr->tag1Length));

        haInfoPtr->tag1Length = 6;
    }

    TimestampTagSize =
        descrPtr->timestampTagged[SMAIN_DIRECTION_INGRESS_E] == SKERNEL_TIMESTAMP_TAG_TYPE_NON_EXTENDED_E ? 8 :
        descrPtr->timestampTagged[SMAIN_DIRECTION_INGRESS_E] == SKERNEL_TIMESTAMP_TAG_TYPE_EXTENDED_E     ? 16:
        descrPtr->timestampTagged[SMAIN_DIRECTION_INGRESS_E] == SKERNEL_TIMESTAMP_TAG_TYPE_HYBRID_E       ? 8 : 0;

    __LOG_PARAM(TimestampTagSize);

    if(SMEM_CHT_IS_SIP7_GET(devObjPtr))
    {
        HSR_BaseLSDUoffset = SMEM_SIP7_HA_SRC_PHYSICAL_PORT_2_ENTRY_FIELD_GET(devObjPtr,
            descrPtr->localDevSrcPort,/* the src physical port */
            SMEM_SIP7_HA_SRC_PHYSICAL_PORT_2_TABLE_FIELDS_HSR_BASE_LSDU_OFFSET_E);
    }
    else
    {
        HSR_BaseLSDUoffset = SMEM_LION3_HA_PHYSICAL_PORT_2_ENTRY_FIELD_BY_SRC_PORT_GET(devObjPtr,
            descrPtr->localDevSrcPort,/* the src physical port */
            SMEM_SIP6_30_HA_PHYSICAL_PORT_TABLE_2_FIELDS_HSR_BASE_LSDU_OFFSET_E);
    }
    __LOG_PARAM(HSR_BaseLSDUoffset);

    Tag0Size = (!descrPtr->marvellTagged  && descrPtr->tagSrcTagged[SNET_CHT_TAG_0_INDEX_CNS]) ? 4: 0;
    __LOG_PARAM(Tag0Size);

    Offset2LSDU = HSR_BaseLSDUoffset + TimestampTagSize + Tag0Size;
    __LOG_PARAM(Offset2LSDU);

    Tag1AddedSize = descrPtr->tag1LocalDevSrcTagged ? 0 : 6;
    __LOG_PARAM(Tag1AddedSize);

    /* the logic from the IAS :
        but in WM the 'descrPtr->byteCount' is with the CRC 4 bytes when descrPtr->rxRecalcCrc = 1.
    */
    LSDUSize = descrPtr->byteCount -
                Offset2LSDU - (descrPtr->rxRecalcCrc == 1 /* not 0 as in IAS */ ? 4 : 0) +
                Tag1AddedSize - (Remove==1 ? 6 : 0);
    __LOG_PARAM(LSDUSize);

    Path_ID =
        SMEM_LION3_HA_EPORT_1_ENTRY_FIELD_AUTO_GET(devObjPtr,
            descrPtr,
            SMEM_SIP6_30_HA_EPORT_TABLE_1_FIELDS_HSR_PRP_LAN_ID_E);
    __LOG_PARAM(Path_ID);

    SeqNr = descrPtr->up1     << 13 |
            descrPtr->cfidei1 << 12 |
            descrPtr->vid1;
    __LOG_PARAM(SeqNr);

    _2bytes = (GT_U16)(Path_ID << 12 | (LSDUSize & 0xFFF));
    egressTagPtr[2] = (GT_U8)(_2bytes >> 8);
    egressTagPtr[3] = (GT_U8) _2bytes      ;

    _2bytes = (GT_U16)SeqNr;
    egressTagPtr[4] = (GT_U8)(_2bytes >> 8);
    egressTagPtr[5] = (GT_U8) _2bytes      ;


    __LOG(("Final HSR (Tag1) 6 bytes are : %2.2x %2.2x %2.2x %2.2x %2.2x %2.2x \n",
        egressTagPtr[0],egressTagPtr[1],
        egressTagPtr[2],egressTagPtr[3],
        egressTagPtr[4],egressTagPtr[5]
        ));

    return;
}
/**
* @internal snetHaSip6_30EgressTag1After4BytesExtension function
* @endinternal
*
* @brief  sip6.30 : set the egress tag extra bytes after the first 4 bytes :
*           !!! ONLY for tag 1 !!!
* @param[in] devObjPtr                - (pointer to) device object.
* @param[in] descrPtr                 - (pointer to) frame descriptor
* @param[in] haInfoPtr                - (pointer to) the HA internal info
* @param[inout] egressTagPtr          - the (start of) buffer that hold egress tag1
*  the first 4 bytes expected to hold info set by snetChtHaEgressTagDataExtGet
*/
static void snetHaSip6_30EgressTag1After4BytesExtension
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN HA_INTERNAL_INFO_STC *haInfoPtr,
    INOUT GT_U8   *egressTagPtr
)
{
    DECLARE_FUNC_NAME(snetHaSip6_30EgressTag1After4BytesExtension);

    GT_U32  tpidIndex;
    GT_U32  regValue;
    GT_U32  _8B_6B_Tag_Value_Offset;
    GT_U8   *tagBytesAfterTpidPtr;
    GT_U32  vid1info;
    GT_U32  hsr_tag;

    tagBytesAfterTpidPtr = egressTagPtr + 2;/* after the TPID */

    tpidIndex     = descrPtr->tpidIndex[SNET_CHT_TAG_1_INDEX_CNS];
    __LOG_PARAM(tpidIndex);

    smemRegGet(devObjPtr,
        SMEM_SIP6_30_HA_EGRESS_TPID_CONFIG_EXT1_REG(devObjPtr,tpidIndex),
        &regValue);

    hsr_tag = SMEM_U32_GET_FIELD(regValue,0,1);
    __LOG_PARAM(hsr_tag);
    if(hsr_tag)
    {
        /* build HSR tag regardless to tag1Length */
        snetSip6_30HaHsrTagBuild(devObjPtr,descrPtr,haInfoPtr,egressTagPtr);
        return;
    }

    if(haInfoPtr->tag1Length <= 4)
    {
        return;
    }

    /*<8B/6B Tag Value Offset>*/
    _8B_6B_Tag_Value_Offset = SMEM_U32_GET_FIELD(regValue,1,3);
    __LOG_PARAM(_8B_6B_Tag_Value_Offset);

    if(haInfoPtr->tag1Length > 6)
    {
        tagBytesAfterTpidPtr[4] = (GT_U8)SMEM_U32_GET_FIELD(regValue,12,8);
        tagBytesAfterTpidPtr[5] = (GT_U8)SMEM_U32_GET_FIELD(regValue, 4,8);

        smemRegGet(devObjPtr,
            SMEM_SIP6_30_HA_EGRESS_TPID_CONFIG_EXT2_REG(devObjPtr,tpidIndex),
            &regValue);

        tagBytesAfterTpidPtr[0] = (GT_U8)SMEM_U32_GET_FIELD(regValue,24,8);
        tagBytesAfterTpidPtr[1] = (GT_U8)SMEM_U32_GET_FIELD(regValue,16,8);
        tagBytesAfterTpidPtr[2] = (GT_U8)SMEM_U32_GET_FIELD(regValue, 8,8);
        tagBytesAfterTpidPtr[3] = (GT_U8)SMEM_U32_GET_FIELD(regValue, 0,8);
    }
    else
    {
        smemRegGet(devObjPtr,
            SMEM_SIP6_30_HA_EGRESS_TPID_CONFIG_EXT2_REG(devObjPtr,tpidIndex),
            &regValue);

        tagBytesAfterTpidPtr[0] = (GT_U8)SMEM_U32_GET_FIELD(regValue,24,8);
        tagBytesAfterTpidPtr[1] = (GT_U8)SMEM_U32_GET_FIELD(regValue,16,8);
        tagBytesAfterTpidPtr[2] = (GT_U8)SMEM_U32_GET_FIELD(regValue, 8,8);
        tagBytesAfterTpidPtr[3] = (GT_U8)SMEM_U32_GET_FIELD(regValue, 0,8);
    }

    if(_8B_6B_Tag_Value_Offset < (haInfoPtr->tag1Length - 1))
    {
        /* assign VID1,UP1,CFI1 */
        __LOG(("assign VID1/UP1/CFI1 into the tag at offset [%d..%d] \n",
            _8B_6B_Tag_Value_Offset,
            _8B_6B_Tag_Value_Offset+1));
        vid1info = descrPtr->up1 << 13 |
                   descrPtr->cfidei1 << 12 |
                   descrPtr->vid1;
        tagBytesAfterTpidPtr[_8B_6B_Tag_Value_Offset + 0] = (GT_U8)SMEM_U32_GET_FIELD(vid1info,8,8);
        tagBytesAfterTpidPtr[_8B_6B_Tag_Value_Offset + 1] = (GT_U8)SMEM_U32_GET_FIELD(vid1info,0,8);
    }
    else
    {
        /* assign VID1,UP1,CFI1 */
        __LOG(("NOT assign VID1/UP1/CFI1 into the tag at offset \n"));
    }

    if(haInfoPtr->tag1Length > 6)
    {
        __LOG(("Final Tag1 8 bytes are : %2.2x %2.2x %2.2x %2.2x %2.2x %2.2x %2.2x %2.2x \n",
            egressTagPtr[0],egressTagPtr[1],
            egressTagPtr[2],egressTagPtr[3],
            egressTagPtr[4],egressTagPtr[5],
            egressTagPtr[6],egressTagPtr[7]
            ));
    }
    else
    {
        __LOG(("Final Tag1 6 bytes are : %2.2x %2.2x %2.2x %2.2x %2.2x %2.2x \n",
            egressTagPtr[0],egressTagPtr[1],
            egressTagPtr[2],egressTagPtr[3],
            egressTagPtr[4],egressTagPtr[5]
            ));
    }

    return;
}

/**
* @internal snetSip6_30HaPrpTrailerBuild function
* @endinternal
*
* @brief   HA build PRP trailer (if needed)
*
* @param[in] devObjPtr                -  pointer to device object.
* @param[in] descrPtr                 - pointer to the frame's descriptor.
* @param[in] destVlanTagged           - send frame with tag.
*
*/
static void  snetSip6_30HaPrpTrailerBuild
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    IN GT_U8    destVlanTagged
)
{
    DECLARE_FUNC_NAME(snetSip6_30HaPrpTrailerBuild);

    GT_U32  Egress_Tag0_Exists;
    GT_U32  prp_trailer_action;
    GT_U32  prp_padding_size;
    GT_U32  PaddingSize;
    GT_U32  TimestampTagSize;
    GT_U32  PRP_BaseLSDUoffset;
    GT_U32  Tag0Size;
    GT_U32  Offset2LSDU;
    GT_U32  egressByteCount_withoutCrc = descrPtr->egressByteCount - 4;/* the WM hold value with 4 bytes CRC */

    descrPtr->prpInfo.prpCmd = PRP_CMD_DO_NOTHING_E;

    if(descrPtr->eArchExtInfo.haInfo.nestedVlanAccessPort_atStartOfHa &&
        (0x1 == SMEM_LION3_HA_EPORT_1_ENTRY_FIELD_AUTO_GET(devObjPtr,
                descrPtr,SMEM_SIP6_30_HA_EPORT_TABLE_1_FIELDS_NESTED_VLAN_MODE_E)))
    {
        __LOG(("The ingress 'nested vlan indication' with 'NESTED_VLAN_MODE == 0x1'(untagged) , is NOT subject to PRP trailer (Add/Remove) \n"));
        return;
    }
    else
    if(descrPtr->truncated)/* at the end of HA , this is indication that actual truncate happen */
    {
        __LOG(("The egress was truncated to 128 bytes so not doing PRP modification on it , is NOT subject to PRP trailer (Add/Remove) \n"));
        return;
    }
    else
    if(descrPtr->outGoingMtagCmd == SKERNEL_MTAG_CMD_TO_TRG_SNIFFER_E &&
       descrPtr->rxSniff == 1) /* packet is ingress monitored */
    {
        __LOG(("Ingress monitored (TO_TRG_SNIFFER && rxSniff) , is NOT subject to PRP trailer (Add/Remove) \n"));
        return;
    }
    else
    if(descrPtr->outGoingMtagCmd == SKERNEL_MTAG_CMD_TO_TRG_SNIFFER_E &&
       descrPtr->rxSniff == 0)/* packet is egress monitored */
    {
        if (descrPtr->marvellTagged == 0)
        {
            __LOG(("Egress monitored (TO_TRG_SNIFFER && rxSniff == 0) without Incoming DSA , may have PRP trailer (Add/Remove) \n"));
        }
        else
        {
            switch(descrPtr->incomingMtagCmd)
            {
                case SKERNEL_MTAG_CMD_TO_CPU_E:
                    __LOG(("Egress monitored (TO_TRG_SNIFFER && rxSniff) , with Incoming DSA 'TO_CPU' , is NOT subject to PRP trailer (Add/Remove) \n"));
                    return;
                case SKERNEL_MTAG_CMD_TO_TRG_SNIFFER_E:
                    __LOG(("Egress monitored (TO_TRG_SNIFFER && rxSniff) , with Incoming DSA 'TO_TRG_SNIFFER' , is NOT subject to PRP trailer (Add/Remove) \n"));
                    return;
                case SKERNEL_MTAG_CMD_FROM_CPU_E:
                    if(descrPtr->useVidx == 0)
                    {
                        __LOG(("Egress monitored (TO_TRG_SNIFFER && rxSniff) , with Incoming DSA 'FROM_CPU'  with useVidx == 0, is NOT subject to PRP trailer (Add/Remove) \n"));
                        return;
                    }
                    else
                    {
                        __LOG(("Egress monitored (TO_TRG_SNIFFER && rxSniff) , with Incoming DSA 'FROM_CPU'  with useVidx == 1 , may have PRP trailer (Add/Remove) \n"));
                    }
                    break;
                default:/*SKERNEL_MTAG_CMD_FORWARD_E*/
                    __LOG(("Egress monitored (TO_TRG_SNIFFER && rxSniff) , with Incoming DSA 'FORWARD' , may have PRP trailer (Add/Remove) \n"));
                    break;
            }
        }
    }
    else
    if(descrPtr->outGoingMtagCmd == SKERNEL_MTAG_CMD_FROM_CPU_E)
    {
        if(descrPtr->useVidx == 0)
        {
            __LOG(("'FROM_CPU' with useVidx == 0, is NOT subject to PRP trailer (Add/Remove) \n"));
            return;
        }
        else
        {
            __LOG(("'FROM_CPU' with useVidx == 1 , may have PRP trailer (Add/Remove) \n"));
        }
    }

    __LOG_PARAM(descrPtr->sstId);

    prp_trailer_action =
        SMEM_LION3_HA_PHYSICAL_PORT_1_ENTRY_FIELD_AUTO_GET(devObjPtr,
            descrPtr,
            SMEM_SIP6_30_HA_PHYSICAL_PORT_TABLE_1_FIELDS_PRP_TRAILER_ACTION_E);

    if(prp_trailer_action == 0x0)/*DO_NOTHING*/
    {
        __LOG(("the prp_trailer_action is DO_NOTHING , so no PRP trailer (Add/Remove) modifications \n"));
        return;
    }

    if(prp_trailer_action == 0x1)/*ADD_PRP*/
    {
        if(0 == GET_DONT_ADD_RCT_IN_SSTID_MAC(descrPtr))
        {
            __LOG(("the prp_trailer_action is ADD_PRP , and no indication from 'srcId' to not add RCT , so add PRP trailer \n"));
        }
        else
        {
            __LOG(("the prp_trailer_action is ADD_PRP , but with indication from 'srcId' to not add RCT , so no PRP add trailer modifications \n"));
            return;
        }
    }
    else
    if(prp_trailer_action == 0x2)/*REMOVE_PRP*/
    {
        if(1 == GET_REMOVE_RCT_IN_SSTID_MAC(descrPtr))
        {
            __LOG(("the prp_trailer_action is REMOVE_PRP , with indication from 'srcId' to remove RCT , so remove PRP trailer \n"));
        }
        else
        {
            __LOG(("the prp_trailer_action is REMOVE_PRP , without indication from 'srcId' to remove RCT , so no PRP remove trailer modifications \n"));
            return;
        }
    }

    prp_padding_size =
        SMEM_LION3_HA_PHYSICAL_PORT_1_ENTRY_FIELD_AUTO_GET(devObjPtr,
            descrPtr,
            SMEM_SIP6_30_HA_PHYSICAL_PORT_TABLE_1_FIELDS_PRP_PADDING_SIZE_E);

    switch(destVlanTagged)
    {
        case SKERNEL_FRAME_TR101_VLAN_EGR_TAG_TAG0_E:
        case SKERNEL_FRAME_TR101_VLAN_EGR_TAG_OUT_TAG0_IN_TAG1_E:
        case SKERNEL_FRAME_TR101_VLAN_EGR_TAG_OUT_TAG1_IN_TAG0_E:
            Egress_Tag0_Exists = 1;
            break;
        default:
            Egress_Tag0_Exists = 0;
            break;
    }

    if(prp_padding_size == 1 || ((prp_padding_size == 0) && (Egress_Tag0_Exists == 0)))
    {
        PaddingSize = 60;
        if(prp_trailer_action == 0x1)/*ADD_PRP*/
        {
            descrPtr->prpInfo.prpCmd = PRP_CMD_ADD60_E;
            if(prp_padding_size == 1)
            {
                __LOG(("ADD_PRP : prp_cmd_Add60 for prp_padding_size = 1 \n"));
            }
            else
            {
                __LOG(("ADD_PRP : prp_cmd_Add60 for prp_padding_size = 0 and egress without tag0 \n"));
            }
        }
    }
    else
    if(prp_padding_size == 2 || ((prp_padding_size == 0) && (Egress_Tag0_Exists == 1)))
    {
        PaddingSize = 64;
        if(prp_trailer_action == 0x1)/*ADD_PRP*/
        {
            descrPtr->prpInfo.prpCmd = PRP_CMD_ADD64_E;
            if(prp_padding_size == 2)
            {
                __LOG(("ADD_PRP : prp_cmd_Add60 for prp_padding_size = 2 \n"));
            }
            else
            {
                __LOG(("ADD_PRP : prp_cmd_Add60 for prp_padding_size = 0 and egress with tag0 \n"));
            }
        }
    }
    else /* prp_padding_size == 3 */
    {
        __LOG(("configuration error : unknown prp_padding_size [%d] \n",
            prp_padding_size));
        return;/* we keep the command prp_cmd_Do_Nothing */
    }


    if(prp_trailer_action == 0x2)/*REMOVE_PRP*/
    {
        descrPtr->prpInfo.prpCmd = PRP_CMD_REMOVE_E;
        __LOG(("REMOVE_PRP : prp_cmd_Remove  \n"));
    }

    descrPtr->prpInfo.egress_prpPathId =
        SMEM_LION3_HA_EPORT_1_ENTRY_FIELD_AUTO_GET(devObjPtr,
            descrPtr,
            SMEM_SIP6_30_HA_EPORT_TABLE_1_FIELDS_HSR_PRP_LAN_ID_E);

    if(PaddingSize > egressByteCount_withoutCrc)
    {
        PaddingSize -= egressByteCount_withoutCrc;
    }
    else
    {
        PaddingSize = 0;
    }

    __LOG_PARAM(PaddingSize);
    __LOG_PARAM(egressByteCount_withoutCrc);

    TimestampTagSize =
        descrPtr->timestampTagged[SMAIN_DIRECTION_EGRESS_E] == SKERNEL_TIMESTAMP_TAG_TYPE_NON_EXTENDED_E ? 8 :
        descrPtr->timestampTagged[SMAIN_DIRECTION_EGRESS_E] == SKERNEL_TIMESTAMP_TAG_TYPE_EXTENDED_E     ? 16:
        descrPtr->timestampTagged[SMAIN_DIRECTION_EGRESS_E] == SKERNEL_TIMESTAMP_TAG_TYPE_HYBRID_E       ? 8 : 0;

    __LOG_PARAM(TimestampTagSize);


    Tag0Size = (descrPtr->egrMarvellTagType == MTAG_TYPE_NONE_E && Egress_Tag0_Exists) ? 4: 0;
    __LOG_PARAM(Tag0Size);

    PRP_BaseLSDUoffset =
        SMEM_LION3_HA_PHYSICAL_PORT_1_ENTRY_FIELD_AUTO_GET(devObjPtr,
            descrPtr,
            SMEM_SIP6_30_HA_PHYSICAL_PORT_TABLE_1_FIELDS_PRP_BASE_LSDU_OFFSET_E);

    Offset2LSDU = PRP_BaseLSDUoffset + TimestampTagSize + Tag0Size;
    __LOG_PARAM(Offset2LSDU);

    descrPtr->prpInfo.egress_prpLSDUSize = egressByteCount_withoutCrc -
                Offset2LSDU + 6 + PaddingSize;

    __LOG_PARAM(descrPtr->prpInfo.egress_prpLSDUSize);

    return;
}

