/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* snetCheetah3TT.c
*
* DESCRIPTION:
*       Cheetah3 Asic Simulation .
*       Tunnel termination interface unit (TTI).
*       Source Code file.
*
* DEPENDENCIES:
*       None.
*
* FILE REVISION NUMBER:
*       $Revision: 188 $
*
*******************************************************************************/

#include <asicSimulation/SKernel/skernel.h>
#include <common/Utils/FrameInfo/sframeInfoAddr.h>
#include <asicSimulation/SKernel/smem/smemCheetah.h>
#include <asicSimulation/SKernel/cheetahCommon/sregCheetah.h>
#include <asicSimulation/SKernel/cheetahCommon/sregLion2.h>
#include <asicSimulation/SKernel/cheetahCommon/sregHawk.h>
#include <asicSimulation/SKernel/suserframes/snet.h>
#include <asicSimulation/SKernel/suserframes/snetCheetahPclSrv.h>
#include <asicSimulation/SKernel/suserframes/snetCheetah3TTermination.h>
#include <asicSimulation/SKernel/suserframes/snetCheetahIngress.h>
#include <asicSimulation/SKernel/suserframes/snetCheetah2Routing.h>
#include <asicSimulation/SKernel/suserframes/snetLion.h>
#include <asicSimulation/SKernel/suserframes/snetCheetah3CentralizedCnt.h>
#include <asicSimulation/SKernel/suserframes/snetXCat.h>
#include <asicSimulation/SKernel/suserframes/snetCheetah3.h>
#include <asicSimulation/SKernel/suserframes/snetCheetahEq.h>
#include <asicSimulation/SKernel/suserframes/snetXCatPcl.h>
#include <asicSimulation/SKernel/suserframes/snetLion3Tcam.h>
#include <asicSimulation/SLog/simLog.h>
#include <asicSimulation/SLog/simLogInfoTypeTcam.h>
#include <asicSimulation/SLog/simLogInfoTypePacket.h>

#include <asicSimulation/SKernel/suserframes/snetFalconExactMatch.h>
#include <asicSimulation/SKernel/suserframes/snetFalconTcam.h>
#include <asicSimulation/SKernel/suserframes/snetAasExactMatch.h>

#define SAVE_TUNNEL_INFO_MAC(field,_internalTtiInfoPtr,_descrPtr)   \
            _internalTtiInfoPtr->preTunnelTerminationInfo.field = _descrPtr->field

/* macro to return result of which bits caused the 'NO MATCH' */
#define X_Y_K_FIND_NO_MATCH(x,y,k,mask)  \
    ((~(((~x) & (~k)) | ((~y) & (k)))) & mask )

/* The range of address between 2 consecutive entries in one bank */
#define SNET_CHT3_TUNNEL_TCAM_ENTRY_WIDTH_CNS                 (0x10)
/* number of bytes between the 4 words of the TTI action */
#define SNET_CHT3_TTI_OFFSET_BETWEEN_WORDS_IN_ACTION_IN_BYTES_CNS  0x5000/*also for xCat*/


#define UDP_HEADER_LENGTH  8

#define GET_BITS(dataPtr,bit_id,numBits)\
        SMEM_U32_GET_FIELD(dataPtr[SMAIN_BIT_TO_REG_INDEX_MAC(bit_id)],   \
                               SMAIN_BIT_TO_INDEX_IN_REG_MAC(bit_id),     \
                               numBits);

#define SET_BITS(dataPtr,bit_id,numBits,value)\
        SMEM_U32_SET_FIELD(dataPtr[SMAIN_BIT_TO_REG_INDEX_MAC(bit_id)],   \
                               SMAIN_BIT_TO_INDEX_IN_REG_MAC(bit_id),     \
                               numBits,value);

#define SNET_CHT_MASK_CHECK(value,mask,key) \
    (((value)&(mask)) == ((key)&(mask)))

/* valid bit offset in control is at location: CH3 - 17, XCAT - 16  */
#define SNET_CONTROL_VALID_BIT_OFFSET_CNS(dev) \
                 16

/* bits offset in the register of SMEM_CHT3_802_11E_QOS_DEFAULT_REG */
#define DEFAULT_UP_OFFSET           0 /*3 bits*/
#define DEFAULT_QOS_INDEX_OFFSET    3 /*7 bits*/


/* bit is reserved in 0x0B800000 */
#define IPV4_TT_HEADER_ERROR_CMD_IPV6_BIT           30

/* values of fragment actions */
#define FRAGMENT_ACTION_TYPE_REDIRECT_REASSEMBLY    0
#define FRAGMENT_ACTION_TYPE_TRAP                   1
#define FRAGMENT_ACTION_TYPE_HARD_DROP              2

/* 20 bits of ePort - TRILL unknown RBID */
#define TRILL_UNKNOWN_RBID_CNS  0xFFFFF

/* 20 bits of ePort - TRILL MY RBID */
#define TRILL_MY_RBID_CNS  0xFFFFE

static GT_VOID snetXcatTTActionApplyTtPassengerParsing
(
    IN    SKERNEL_DEVICE_OBJECT             *devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC   *descrPtr,
    IN    SNET_XCAT_TT_ACTION_STC           *actionDataPtr,
    IN    INTERNAL_TTI_DESC_INFO_STC        *internalTtiInfoPtr,
    IN    GT_BOOL                           isTransitTunnel
);

/* array that holds the info about the fields */
static CHT_PCL_KEY_FIELDS_INFO_STC cht3TTKeyFieldsData[CHT3_TT_KEY_FIELDS_ID_LAST_E+1]=
{
    {0  ,9  ,GT_TRUE," CHT3_TT_KEY_FIELDS_ID_PCL_ID_E "  },
    {10 ,17 ,GT_TRUE," CHT3_TT_KEY_FIELDS_ID_SOURCE_PORT_TRUNK_E "  },
    {18 ,18 ,GT_TRUE," CHT3_TT_KEY_FIELDS_ID_SRC_IS_TRUNK_E "  },
    {19 ,30 ,GT_TRUE," CHT3_TT_KEY_FIELDS_ID_VID_E "  },
    {31 ,31 ,GT_TRUE," CHT3_TT_KEY_FIELDS_ID_PCKT_TAGGED_E "  },
    {32 ,79 ,GT_TRUE," CHT3_TT_KEY_FIELDS_ID_MAC_DA_SA_E "  },
    {80 ,86 ,GT_TRUE," CHT3_TT_KEY_FIELDS_ID_SRC_DEV_E "  },
    /* IPv4 ARP TTI KEY */
    {87 ,89 ,GT_TRUE," CHT3_TT_KEY_FIELDS_ID_TUNNEL_PROTOCOL_E "  },
    {90 ,121,GT_TRUE," CHT3_TT_KEY_FIELDS_ID_SIP_ADDRESS_E "},
    {122,153,GT_TRUE," CHT3_TT_KEY_FIELDS_ID_DIP_ADDRESS_E "},
    {154,154,GT_TRUE," CHT3_TT_KEY_FIELDS_ID_IS_ARP_E "  },
    {155,159,GT_TRUE," CHT3_TT_KEY_FIELDS_ID_RESERVED_155_159_E "  },
    {160,166,GT_TRUE," CHT3_TT_KEY_FIELDS_ID_DSA_SOURCE_PORT_TRUNK_E "  },
    {167,167,GT_TRUE," CHT3_TT_KEY_FIELDS_ID_DSA_SRC_IS_TRUNK_E "  },
    {168,173,GT_TRUE," XCAT3_TT_KEY_FIELDS_ID_DSCP_E "},
    {174,190,GT_TRUE," CHT3_TT_KEY_FIELDS_ID_RESERVED_168_190_E "},
    {191,191,GT_TRUE," CHT3_TT_KEY_FIELDS_ID_RESERVED_191_E "},
    /* MPLS TTI KEY */
    {87 ,89 ,GT_TRUE," CHT3_TT_KEY_FIELDS_ID_EXP0_E "  },
    {90 ,92 ,GT_TRUE," CHT3_TT_KEY_FIELDS_ID_EXP1_E "  },
    {93 ,112,GT_TRUE," CHT3_TT_KEY_FIELDS_ID_LABLE0_E " },
    {113,132,GT_TRUE," CHT3_TT_KEY_FIELDS_ID_LABLE1_E " },
    {133,135,GT_TRUE," CHT3_TT_KEY_FIELDS_ID_EXP2_E "  },
    {136,155,GT_TRUE," CHT3_TT_KEY_FIELDS_ID_LABLE2_E " },
    {156,157,GT_TRUE," CHT3_TT_KEY_FIELDS_ID_NUM_LABLES_E " },
    {158,159,GT_TRUE," CHT3_TT_KEY_FIELDS_ID_NLP_ABOVE_MPLS_E " },

    /* ETHERNET TTI KEY */
    {87 ,89 ,GT_TRUE," CHT3_TT_KEY_FIELDS_ID_UP0_E "  },
    {90 ,90 ,GT_TRUE," CHT3_TT_KEY_FIELDS_ID_CFI0_E "  },
    {91 ,91 ,GT_TRUE," CHT3_TT_KEY_FIELDS_ID_VLAN1_EXIST_E " },
    {92 ,103,GT_TRUE," CHT3_TT_KEY_FIELDS_ID_VLAN1_E " },
    {104,106,GT_TRUE," CHT3_TT_KEY_FIELDS_ID_UP1_E "  },
    {107,107,GT_TRUE," CHT3_TT_KEY_FIELDS_ID_CFI1_E " },
    {108,123,GT_TRUE," CHT3_TT_KEY_FIELDS_ID_ETHER_TYPE_E " },
    {124,124,GT_TRUE," CHT3_TT_KEY_FIELDS_ID_MAC_2_ME_E " },
    {125,159,GT_TRUE," CHT3_TT_KEY_FIELDS_ID_RESERVED_125_159_E " },
    {168,172,GT_TRUE," XCAT_TT_KEY_FIELDS_ID_DSA_SOURCE_ID_E " },  /* relevant only for xCat */
    {173,179,GT_TRUE," XCAT_TT_KEY_FIELDS_ID_DSA_QOS_PROFILE_E " },  /* relevant only for xCat */
    {180,181,GT_TRUE," XCAT3_TT_KEY_FIELDS_ID_GRP_E " },             /* relevant only for xCat3 */
    {182,190,GT_TRUE," XCAT_TT_KEY_FIELDS_ID_RESERVED_180_190_E " },  /* relevant only for xCat */

    /* MIM TTI KEY - relevant only for xCat */
    {91 ,91 ,GT_TRUE," XCAT_TT_KEY_FIELDS_ID_RESERVED_91_E "  },
    {92 ,115,GT_TRUE," XCAT_TT_KEY_FIELDS_ID_I_SID_E "  },
    {116,118,GT_TRUE," XCAT_TT_KEY_FIELDS_ID_I_UP_E "  },
    {119,119,GT_TRUE," XCAT_TT_KEY_FIELDS_ID_I_DP_E "  },
    {120,121,GT_TRUE," XCAT_TT_KEY_FIELDS_ID_I_RES_1_E "  },
    {122,123,GT_TRUE," XCAT_TT_KEY_FIELDS_ID_I_RES_2_E "  },
    {124,159,GT_TRUE," XCAT_TT_KEY_FIELDS_ID_RESERVED_124_159_E "  },

    /* XCAT-C0 fields (not in xcat2-B,lion2-A) */
    {168,171,GT_TRUE," XCAT_TT_KEY_FIELDS_ID_MPLS_CW_FIRST_NIBBLE_E "},           /* MPLS only */
    {172,172,GT_TRUE," XCAT_TT_KEY_FIELDS_ID_MPLS_INNER_PACKET_TAG_0_EXISTS_E " },   /* MPLS only */
    {173,184,GT_TRUE," XCAT_TT_KEY_FIELDS_ID_MPLS_INNER_PACKET_TAG_0_VID_E " },/* MPLS only */

    {189,190,GT_TRUE," LION_TT_KEY_FIELDS_ID_SRC_PORT_GROUP_BITS_0_1_E " },

    {176,180,GT_TRUE," LION3_TT_KEY_FIELDS_ID_DSA_SRC_DEV_7_11_E "},         /*from Lion 3 : IPv4 , MPLS , MIM */
    {181,185,GT_TRUE," LION3_TT_KEY_FIELDS_ID_DSA_SRC_EPORT_TRUNK_7_11_E "}, /*from Lion 3 : all keys */
    {186,187,GT_TRUE," LION3_TT_KEY_FIELDS_ID_E_VLAN_12_13_E "},             /*from Lion 3 : all keys */
    {188,190,GT_TRUE," LION2_TT_KEY_FIELDS_ID_SRC_PORT_GROUP_BITS_0_2_E " }, /*from Lion 3 : all keys */
    {125,127,GT_TRUE," LION3_TT_KEY_FIELDS_ID_ETHERNET_TAG_0_TPID_INDEX_E " },  /*from Lion 3 : ETH only */
    {128,130,GT_TRUE," LION3_TT_KEY_FIELDS_ID_ETHERNET_TAG_1_TPID_INDEX_E " },  /*from Lion 3 : ETH only */
    {131,135,GT_TRUE," LION3_TT_KEY_FIELDS_ID_ETHERNET_DSA_SRC_DEV_7_11_E " },  /*from Lion 3 : ETH only */
    {136,142,GT_TRUE," LION3_TT_KEY_FIELDS_ID_ETHERNET_DSA_SOURCE_ID_5_11_E " },/*from Lion 3 : ETH only */

    { 91, 91,GT_TRUE," LION3_TT_KEY_FIELDS_MIM_ID_MAC_2_ME_E " },               /*from Lion 3 : MIM only */
    {124,135,GT_TRUE," LION3_TT_KEY_FIELDS_MIM_INNER_PACKET_TAG_0_VID_E " },    /*from Lion 3 : MIM only */
    {136,136,GT_TRUE," LION3_TT_KEY_FIELDS_MIM_INNER_PACKET_TAG_0_EXISTS_E " }, /*from Lion 3 : MIM only */
    {137,139,GT_TRUE," LION3_TT_KEY_FIELDS_MIM_INNER_PACKET_TAG_0_UP_E " },     /*from Lion 3 : MIM only */
    {140,140,GT_TRUE," LION3_TT_KEY_FIELDS_MIM_INNER_PACKET_TAG_0_DEI_E " },    /*from Lion 3 : MIM only */

    /* dummy */
    {0, 0,  GT_FALSE,   " ------ "}
};

/*
    enum :  LION3_TRILL_ADJACENCY_KEY_FIELDS_ID_ENT

    description : enum of the TRILL Adjacency TCAM fields

*/
typedef enum{
    LION3_TRILL_ADJACENCY_KEY_FIELDS_ID_OUTER_SA_E,
    LION3_TRILL_ADJACENCY_KEY_FIELDS_ID_TRILL_M_E,
    LION3_TRILL_ADJACENCY_KEY_FIELDS_ID_TRILL_E_RBID_E,
    LION3_TRILL_ADJACENCY_KEY_FIELDS_ID_SRC_DEV_E,
    LION3_TRILL_ADJACENCY_KEY_FIELDS_ID_SRC_IS_TRUNK_E,
    LION3_TRILL_ADJACENCY_KEY_FIELDS_ID_SOURCE_PORT_TRUNK_E,

    LION3_TRILL_ADJACENCY_KEY_FIELDS_ID_LAST_E
}LION3_TRILL_ADJACENCY_KEY_FIELDS_ID_ENT;

static GT_VOID snetXcatTTActionApply
(
    IN SKERNEL_DEVICE_OBJECT                * devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC   * descrPtr  ,
    IN SNET_XCAT_TT_ACTION_STC              * actionDataPtr,
    OUT   INTERNAL_TTI_DESC_INFO_STC        *internalTtiInfoPtr
);

static GT_VOID snetXcatTTActionApplyPart2
(
    IN SKERNEL_DEVICE_OBJECT                * devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC   * descrPtr  ,
    IN SNET_XCAT_TT_ACTION_STC              * actionDataPtr
);

static GT_VOID snetXcatTTActionGet
(
    IN SKERNEL_DEVICE_OBJECT            * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC  * descrPtr,
    IN GT_U32                             matchIndex,
    OUT SNET_XCAT_TT_ACTION_STC         * actionDataPtr,
    OUT   INTERNAL_TTI_DESC_INFO_STC    * internalTtiInfoPtr
);

static GT_VOID snetLion3TTActionGet
(
    IN SKERNEL_DEVICE_OBJECT            * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC  * descrPtr,
    IN GT_U32                             matchIndex,
    IN SIP7_EXACT_MATCH_UNIT_ENT          emUnitNum,
    IN GT_BOOL                            isMatchDoneInTcamOrEm,
    IN SKERNEL_CHT3_TTI_KEY_TYPE_ENT      ttiKeyType,
    IN GT_U32                            lookupNum,
    IN GT_U32                            serialEmProfileId,
    OUT SNET_XCAT_TT_ACTION_STC         * actionDataPtr
);


static GT_BOOL snetXcatTTMplsTtlExceededException
(
    IN SKERNEL_DEVICE_OBJECT             *  devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC   *  descrPtr,
    IN SNET_XCAT_TT_ACTION_STC           * actionDataPtr
);

static GT_BOOL snetXcatTTMplsHeaderCheck
(
    IN SKERNEL_DEVICE_OBJECT             *  devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC   *  descrPtr,
    IN SNET_XCAT_TT_ACTION_STC           *  actionDataPtr
);

static GT_BOOL snetXcatTTTriggeringCheck
(
    IN  SKERNEL_DEVICE_OBJECT             *devObjPtr,
    IN  SKERNEL_FRAME_CHEETAH_DESCR_STC   *descrPtr,
    OUT INTERNAL_TTI_DESC_INFO_STC        *internalTtiInfoPtr
);

static GT_VOID snetCht3Mac2MeTblLookUp
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr
);

static GT_VOID snetLion2TTOamDescriptorSet
(
    IN SKERNEL_DEVICE_OBJECT                 * devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC    * descrPtr,
    IN SNET_XCAT_TT_ACTION_STC               * ttiActionDataPtr
);

static GT_VOID snetFalconTTEcnResolve
(
    IN SKERNEL_DEVICE_OBJECT            *devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC  *descrPtr
);

#define TTI_ERROR_BOOKMARK


typedef enum {
    SMEM_LION3_TTI_META_DATA_FIELDS_IS_ARP_E,
    SMEM_LION3_TTI_META_DATA_FIELDS_IPV4_TUNNELING_PROTOCOL_E,
    SMEM_LION3_TTI_META_DATA_FIELDS_RESERVED_0_E,
    SMEM_LION3_TTI_META_DATA_FIELDS_IPV4_ARP_DIP_E,
    SMEM_LION3_TTI_META_DATA_FIELDS_IPV4_ARP_SIP_E,
    SMEM_LION3_TTI_META_DATA_FIELDS_PROTOCOL_ABOVE_MPLS_E,
    SMEM_LION3_TTI_META_DATA_FIELDS_RESERVED_LABEL_EXISTS_E,
    SMEM_LION3_TTI_META_DATA_FIELDS_DATA_AFTER_INNER_LABEL_E,
    SMEM_LION3_TTI_META_DATA_FIELDS_RESERVED_LABEL_VALUE_E,
    SMEM_LION3_TTI_META_DATA_FIELDS_CONTROL_WORD_CHANNEL_TYPE_PROFILE_E,
    SMEM_LION3_TTI_META_DATA_FIELDS_UP0_B_UP_E,
    SMEM_LION3_TTI_META_DATA_FIELDS_CFI0_B_DEI__E,
    SMEM_LION3_TTI_META_DATA_FIELDS_VLAN_TAG0_EXISTS_E,
    SMEM_LION3_TTI_META_DATA_FIELDS_RESERVED_1_E,
    SMEM_LION3_TTI_META_DATA_FIELDS_PASSENGER_OUTER_CFI_E,
    SMEM_LION3_TTI_META_DATA_FIELDS_PASSENGER_OUTER_TAG_VID_7_0_E,
    SMEM_LION3_TTI_META_DATA_FIELDS_PASSENGER_OUTER_TAG_VID_11_8_E,
    SMEM_LION3_TTI_META_DATA_FIELDS_PASSENGER_OUTER_TAG_UP_E,
    SMEM_LION3_TTI_META_DATA_FIELDS_PASSENGER_OUTER_TAG_EXIST_E,
    SMEM_LION3_TTI_META_DATA_FIELDS_VLAN1_7_0_E,
    SMEM_LION3_TTI_META_DATA_FIELDS_VLAN1_11_8_E,
    SMEM_LION3_TTI_META_DATA_FIELDS_UP1_E,
    SMEM_LION3_TTI_META_DATA_FIELDS_CFI1_E,
    SMEM_LION3_TTI_META_DATA_FIELDS_VLAN1_EXIST_E,
    SMEM_LION3_TTI_META_DATA_FIELDS_TAG0_TPID_INDEX_E,
    SMEM_LION3_TTI_META_DATA_FIELDS_TAG1_TPID_INDEX_E,
    SMEM_LION3_TTI_META_DATA_FIELDS_RESERVED_2_E,
    SMEM_LION3_TTI_META_DATA_FIELDS_DSA_QOS_PROFILE_E,
    SMEM_LION3_TTI_META_DATA_FIELDS_RESERVED_3_E,
    SMEM_LION3_TTI_META_DATA_FIELDS_DSA_SOURCE_ID__7_0_E,
    SMEM_LION3_TTI_META_DATA_FIELDS_DSA_SOURCE_ID__11_8_E,
    SMEM_LION3_TTI_META_DATA_FIELDS_NUM_OF_MPLS_LABELS_E,
    SMEM_LION3_TTI_META_DATA_FIELDS_RESERVED_4_E,
    SMEM_LION3_TTI_META_DATA_FIELDS_TRILL_OPTIONS_1ST_NIBBLE_E,
    SMEM_LION3_TTI_META_DATA_FIELDS_TRILL_OPTIONS_EXIST_E,
    SMEM_LION3_TTI_META_DATA_FIELDS_TRILL_M_BIT_E,
    SMEM_LION3_TTI_META_DATA_FIELDS_TRILL_OPTION_LENGTH_EXCEEDED_E,
    SMEM_LION3_TTI_META_DATA_FIELDS_REP_LAST_E,
    SMEM_LION3_TTI_META_DATA_FIELDS_IPV6_TUNNELING_PROTOCOL_E,
    SMEM_LION3_TTI_META_DATA_FIELDS_L3_DATA_WORD0_FIRST_NIBBLE_E,
    SMEM_LION3_TTI_META_DATA_FIELDS_IPV6_L4_VALID_E,
    SMEM_LION3_TTI_META_DATA_FIELDS_PCLID_4_0_E,
    SMEM_LION3_TTI_META_DATA_FIELDS_LOCAL_DEVICE_SOURCE_IS_TRUNK_E,
    SMEM_LION3_TTI_META_DATA_FIELDS_DSA_TAG_SRCISTRUNK_E,
    SMEM_LION3_TTI_META_DATA_FIELDS_MAC_TO_ME_E,
    SMEM_LION3_TTI_META_DATA_FIELDS_PCLID_9_5_E,
    SMEM_LION3_TTI_META_DATA_FIELDS_SRC_COREID_E,
    SMEM_LION3_TTI_META_DATA_FIELDS_EVLAN_7_0_E,
    SMEM_LION3_TTI_META_DATA_FIELDS_EVLAN_12_8_E,
    SMEM_LION3_SIP7_TTI_META_DATA_FIELDS_EVLAN_15_13_E,
    SMEM_LION3_TTI_META_DATA_FIELDS_LOCAL_DEVICE_SOURCE_EPORT_TRUNKID_7_0_E,
    SMEM_LION3_TTI_META_DATA_FIELDS_LOCAL_DEVICE_SOURCE_EPORT_TRUNKID_12_8_E,
    SMEM_LION3_TTI_META_DATA_FIELDS_RESERVED_6_E,
    SMEM_LION3_TTI_META_DATA_FIELDS_DSA_TAG_SRCPORT_SRCEPORT_TRUNKID_7_0_E,
    SMEM_LION3_TTI_META_DATA_FIELDS_DSA_TAG_SRCPORT_SRCEPORT_TRUNKID_12_8_E,
    SMEM_LION3_TTI_META_DATA_FIELDS_RESERVED_7_E,
    SMEM_LION3_TTI_META_DATA_FIELDS_DSA_TAG_SRCDEV_7_0_E,
    SMEM_LION3_TTI_META_DATA_FIELDS_DSA_TAG_SRCDEV_9_8_E,
    SMEM_LION3_TTI_META_DATA_FIELDS_RESERVED_8_E,
    SMEM_LION3_TTI_META_DATA_FIELDS_MUST_BE_1_E,
    SMEM_LION3_TTI_META_DATA_FIELDS_SR_SEGMENT_LEFT_E,
    SMEM_LION3_TTI_META_DATA_FIELDS_SR_EH_EXISTS_E,
    SMEM_LION3_TTI_META_DATA_FIELDS_RESERVED_9_E,
    SMEM_LION3_TTI_META_DATA_FIELDS_SRC_PHY_PORT_7_0_E,
    SMEM_LION3_TTI_META_DATA_FIELDS_SRC_PHY_PORT_9_8_E,
    SMEM_LION3_TTI_META_DATA_FIELDS_PREEMPTED_E,
    SMEM_LION3_TTI_META_DATA_FIELDS_RESERVED_10_E,
    SMEM_LION3_TTI_META_DATA_FIELDS_IPVX_PROTOCOL_E,
    SMEM_LION3_TTI_META_DATA_FIELDS_IP2ME_MATCH_FOUND_E,
    SMEM_LION3_TTI_META_DATA_FIELDS_IP2ME_MATCH_INDEX_E,
    SMEM_LION3_TTI_META_DATA_FIELDS_GENERIC_CLASSIFICATION_7_0_E, SIM_TBD_BOOKMARK
    SMEM_LION3_TTI_META_DATA_FIELDS_GENERIC_CLASSIFICATION_15_8_E,

    SMEM_LION3_TTI_META_DATA_FIELDS___LAST_VALUE___E/* used for array size */
}SMEM_LION3_TTI_META_DATA_FIELDS_ENT;


#define SMEM_LION3_TTI_META_DATA_FIELDS_NAME                                \
     STR(SMEM_LION3_TTI_META_DATA_FIELDS_IS_ARP_E                                )\
    ,STR(SMEM_LION3_TTI_META_DATA_FIELDS_IPV4_TUNNELING_PROTOCOL_E               )\
    ,STR(SMEM_LION3_TTI_META_DATA_FIELDS_RESERVED_0_E                            )\
    ,STR(SMEM_LION3_TTI_META_DATA_FIELDS_IPV4_ARP_DIP_E                          )\
    ,STR(SMEM_LION3_TTI_META_DATA_FIELDS_IPV4_ARP_SIP_E                          )\
    ,STR(SMEM_LION3_TTI_META_DATA_FIELDS_PROTOCOL_ABOVE_MPLS_E                   )\
    ,STR(SMEM_LION3_TTI_META_DATA_FIELDS_RESERVED_LABEL_EXISTS_E                 )\
    ,STR(SMEM_LION3_TTI_META_DATA_FIELDS_DATA_AFTER_INNER_LABEL_E                )\
    ,STR(SMEM_LION3_TTI_META_DATA_FIELDS_RESERVED_LABEL_VALUE_E                  )\
    ,STR(SMEM_LION3_TTI_META_DATA_FIELDS_CONTROL_WORD_CHANNEL_TYPE_PROFILE_E     )\
    ,STR(SMEM_LION3_TTI_META_DATA_FIELDS_UP0_B_UP_E                              )\
    ,STR(SMEM_LION3_TTI_META_DATA_FIELDS_CFI0_B_DEI__E                           )\
    ,STR(SMEM_LION3_TTI_META_DATA_FIELDS_VLAN_TAG0_EXISTS_E                      )\
    ,STR(SMEM_LION3_TTI_META_DATA_FIELDS_RESERVED_1_E                            )\
    ,STR(SMEM_LION3_TTI_META_DATA_FIELDS_PASSENGER_OUTER_CFI_E                   )\
    ,STR(SMEM_LION3_TTI_META_DATA_FIELDS_PASSENGER_OUTER_TAG_VID_7_0_E           )\
    ,STR(SMEM_LION3_TTI_META_DATA_FIELDS_PASSENGER_OUTER_TAG_VID_11_8_E          )\
    ,STR(SMEM_LION3_TTI_META_DATA_FIELDS_PASSENGER_OUTER_TAG_UP_E                )\
    ,STR(SMEM_LION3_TTI_META_DATA_FIELDS_PASSENGER_OUTER_TAG_EXIST_E             )\
    ,STR(SMEM_LION3_TTI_META_DATA_FIELDS_VLAN1_7_0_E                             )\
    ,STR(SMEM_LION3_TTI_META_DATA_FIELDS_VLAN1_11_8_E                            )\
    ,STR(SMEM_LION3_TTI_META_DATA_FIELDS_UP1_E                                   )\
    ,STR(SMEM_LION3_TTI_META_DATA_FIELDS_CFI1_E                                  )\
    ,STR(SMEM_LION3_TTI_META_DATA_FIELDS_VLAN1_EXIST_E                           )\
    ,STR(SMEM_LION3_TTI_META_DATA_FIELDS_TAG0_TPID_INDEX_E                       )\
    ,STR(SMEM_LION3_TTI_META_DATA_FIELDS_TAG1_TPID_INDEX_E                       )\
    ,STR(SMEM_LION3_TTI_META_DATA_FIELDS_RESERVED_2_E                            )\
    ,STR(SMEM_LION3_TTI_META_DATA_FIELDS_DSA_QOS_PROFILE_E                       )\
    ,STR(SMEM_LION3_TTI_META_DATA_FIELDS_RESERVED_3_E                            )\
    ,STR(SMEM_LION3_TTI_META_DATA_FIELDS_DSA_SOURCE_ID__7_0_E                    )\
    ,STR(SMEM_LION3_TTI_META_DATA_FIELDS_DSA_SOURCE_ID__11_8_E                   )\
    ,STR(SMEM_LION3_TTI_META_DATA_FIELDS_NUM_OF_MPLS_LABELS_E                    )\
    ,STR(SMEM_LION3_TTI_META_DATA_FIELDS_RESERVED_4_E                            )\
    ,STR(SMEM_LION3_TTI_META_DATA_FIELDS_TRILL_OPTIONS_1ST_NIBBLE_E              )\
    ,STR(SMEM_LION3_TTI_META_DATA_FIELDS_TRILL_OPTIONS_EXIST_E                   )\
    ,STR(SMEM_LION3_TTI_META_DATA_FIELDS_TRILL_M_BIT_E                           )\
    ,STR(SMEM_LION3_TTI_META_DATA_FIELDS_TRILL_OPTION_LENGTH_EXCEEDED_E          )\
    ,STR(SMEM_LION3_TTI_META_DATA_FIELDS_REP_LAST_E                              )\
    ,STR(SMEM_LION3_TTI_META_DATA_FIELDS_IPV6_TUNNELING_PROTOCOL_E               )\
    ,STR(SMEM_LION3_TTI_META_DATA_FIELDS_L3_DATA_WORD0_FIRST_NIBBLE_E            )\
    ,STR(SMEM_LION3_TTI_META_DATA_FIELDS_IPV6_L4_VALID_E                         )\
    ,STR(SMEM_LION3_TTI_META_DATA_FIELDS_PCLID_4_0_E                             )\
    ,STR(SMEM_LION3_TTI_META_DATA_FIELDS_LOCAL_DEVICE_SOURCE_IS_TRUNK_E          )\
    ,STR(SMEM_LION3_TTI_META_DATA_FIELDS_DSA_TAG_SRCISTRUNK_E                    )\
    ,STR(SMEM_LION3_TTI_META_DATA_FIELDS_MAC_TO_ME_E                             )\
    ,STR(SMEM_LION3_TTI_META_DATA_FIELDS_PCLID_9_5_E                             )\
    ,STR(SMEM_LION3_TTI_META_DATA_FIELDS_SRC_COREID_E                            )\
    ,STR(SMEM_LION3_TTI_META_DATA_FIELDS_EVLAN_7_0_E                             )\
    ,STR(SMEM_LION3_TTI_META_DATA_FIELDS_EVLAN_12_8_E                            )\
    ,STR(SMEM_LION3_SIP7_TTI_META_DATA_FIELDS_EVLAN_15_13_E                      )\
    ,STR(SMEM_LION3_TTI_META_DATA_FIELDS_LOCAL_DEVICE_SOURCE_EPORT_TRUNKID_7_0_E )\
    ,STR(SMEM_LION3_TTI_META_DATA_FIELDS_LOCAL_DEVICE_SOURCE_EPORT_TRUNKID_12_8_E)\
    ,STR(SMEM_LION3_TTI_META_DATA_FIELDS_RESERVED_6_E                            )\
    ,STR(SMEM_LION3_TTI_META_DATA_FIELDS_DSA_TAG_SRCPORT_SRCEPORT_TRUNKID_7_0_E  )\
    ,STR(SMEM_LION3_TTI_META_DATA_FIELDS_DSA_TAG_SRCPORT_SRCEPORT_TRUNKID_12_8_E )\
    ,STR(SMEM_LION3_TTI_META_DATA_FIELDS_RESERVED_7_E                            )\
    ,STR(SMEM_LION3_TTI_META_DATA_FIELDS_DSA_TAG_SRCDEV_7_0_E                    )\
    ,STR(SMEM_LION3_TTI_META_DATA_FIELDS_DSA_TAG_SRCDEV_9_8_E                    )\
    ,STR(SMEM_LION3_TTI_META_DATA_FIELDS_RESERVED_8_E                            )\
    ,STR(SMEM_LION3_TTI_META_DATA_FIELDS_MUST_BE_1_E                             )\
    ,STR(SMEM_LION3_TTI_META_DATA_FIELDS_SR_SEGMENT_LEFT_E                       )\
    ,STR(SMEM_LION3_TTI_META_DATA_FIELDS_SR_EH_EXISTS_E                          )\
    ,STR(SMEM_LION3_TTI_META_DATA_FIELDS_RESERVED_9_E                            )\
    ,STR(SMEM_LION3_TTI_META_DATA_FIELDS_SRC_PHY_PORT_7_0_E                      )\
    ,STR(SMEM_LION3_TTI_META_DATA_FIELDS_SRC_PHY_PORT_9_8_E                      )\
    ,STR(SMEM_LION3_TTI_META_DATA_FIELDS_PREEMPTED_E                             )\
    ,STR(SMEM_LION3_TTI_META_DATA_FIELDS_RESERVED_10_E                           )\
    ,STR(SMEM_LION3_TTI_META_DATA_FIELDS_IPVX_PROTOCOL_E                         )\
    ,STR(SMEM_LION3_TTI_META_DATA_FIELDS_IP2ME_MATCH_FOUND_E                     )\
    ,STR(SMEM_LION3_TTI_META_DATA_FIELDS_IP2ME_MATCH_INDEX_E                     )\
    ,STR(SMEM_LION3_TTI_META_DATA_FIELDS_GENERIC_CLASSIFICATION_7_0_E            )\
    ,STR(SMEM_LION3_TTI_META_DATA_FIELDS_GENERIC_CLASSIFICATION_15_8_E           )

static char * lion3TtiMetaDataFieldsNames[
    SMEM_LION3_TTI_META_DATA_FIELDS___LAST_VALUE___E] =
    {SMEM_LION3_TTI_META_DATA_FIELDS_NAME};


static SNET_ENTRY_FORMAT_TABLE_STC lion3TtiMetaDataFieldsFormat[SMEM_LION3_TTI_META_DATA_FIELDS___LAST_VALUE___E] =
{ /* byte 0 */
    /*LION3_TTI_META_DATA_FIELDS_IS_ARP_E,                                    */
    STANDARD_FIELD_MAC(1  ),
    /*SMEM_LION3_TTI_META_DATA_FIELDS_IPV4_TUNNELING_PROTOCOL_E,                   */
    STANDARD_FIELD_MAC(3  ),
    /*SMEM_LION3_TTI_META_DATA_FIELDS_RESERVED_0_E,                                */
    STANDARD_FIELD_MAC(4  ),
/* byte 1..4 */
    /*SMEM_LION3_TTI_META_DATA_FIELDS_IPV4_ARP_DIP_E,                              */
    STANDARD_FIELD_MAC(32 ),
/* byte 5..8 */
    /*SMEM_LION3_TTI_META_DATA_FIELDS_IPV4_ARP_SIP_E,                              */
    STANDARD_FIELD_MAC(32 ),
/* byte 9 */
    /*SMEM_LION3_TTI_META_DATA_FIELDS_PROTOCOL_ABOVE_MPLS_E,                       */
    STANDARD_FIELD_MAC(2  ),
    /*SMEM_LION3_TTI_META_DATA_FIELDS_RESERVED_LABEL_EXISTS_E,                     */
    STANDARD_FIELD_MAC(1  ),
    /*SMEM_LION3_TTI_META_DATA_FIELDS_DATA_AFTER_INNER_LABEL_E,                    */
    STANDARD_FIELD_MAC(5  ),
/* byte 10 */
    /*SMEM_LION3_TTI_META_DATA_FIELDS_RESERVED_LABEL_VALUE_E,                      */
    STANDARD_FIELD_MAC(4  ),
    /*SMEM_LION3_TTI_META_DATA_FIELDS_CONTROL_WORD_CHANNEL_TYPE_PROFILE_E,         */
    STANDARD_FIELD_MAC(4  ),
/* byte 11 */
    /*SMEM_LION3_TTI_META_DATA_FIELDS_UP0_B_UP_E,                                  */
    STANDARD_FIELD_MAC(3  ),
    /*SMEM_LION3_TTI_META_DATA_FIELDS_CFI0_B_DEI__E,                               */
    STANDARD_FIELD_MAC(1  ),
    /*SMEM_LION3_TTI_META_DATA_FIELDS_VLAN_TAG0_EXISTS_E,                          */
    STANDARD_FIELD_MAC(1  ),
    /*SMEM_LION3_TTI_META_DATA_FIELDS_RESERVED_1_E,                                */
    STANDARD_FIELD_MAC(2  ),
    /*SMEM_LION3_TTI_META_DATA_FIELDS_PASSENGER_OUTER_CFI_E,                       */
    STANDARD_FIELD_MAC(1  ),
/* byte 12 */
    /*SMEM_LION3_TTI_META_DATA_FIELDS_PASSENGER_OUTER_TAG_VID_7_0_E,               */
    STANDARD_FIELD_MAC(8  ),
/* byte 13 */
    /*SMEM_LION3_TTI_META_DATA_FIELDS_PASSENGER_OUTER_TAG_VID_11_8_E,              */
    STANDARD_FIELD_MAC(4  ),
    /*SMEM_LION3_TTI_META_DATA_FIELDS_PASSENGER_OUTER_TAG_UP_E,                    */
    STANDARD_FIELD_MAC(3  ),
    /*SMEM_LION3_TTI_META_DATA_FIELDS_PASSENGER_OUTER_TAG_EXIST_E,                 */
    STANDARD_FIELD_MAC(1  ),
/* byte 14 */
    /*SMEM_LION3_TTI_META_DATA_FIELDS_VLAN1_7_0_E,                                 */
    STANDARD_FIELD_MAC(8  ),
/* byte 15 */
    /*SMEM_LION3_TTI_META_DATA_FIELDS_VLAN1_11_8_E,                                */
    STANDARD_FIELD_MAC(4  ),
    /*SMEM_LION3_TTI_META_DATA_FIELDS_UP1_E,                                       */
    STANDARD_FIELD_MAC(3  ),
    /*SMEM_LION3_TTI_META_DATA_FIELDS_CFI1_E,                                      */
    STANDARD_FIELD_MAC(1  ),
/* byte 16 */
    /*SMEM_LION3_TTI_META_DATA_FIELDS_VLAN1_EXIST_E,                               */
    STANDARD_FIELD_MAC(1  ),
    /*SMEM_LION3_TTI_META_DATA_FIELDS_TAG0_TPID_INDEX_E,                           */
    STANDARD_FIELD_MAC(3  ),
    /*SMEM_LION3_TTI_META_DATA_FIELDS_TAG1_TPID_INDEX_E,                           */
    STANDARD_FIELD_MAC(3  ),
    /*SMEM_LION3_TTI_META_DATA_FIELDS_RESERVED_2_E,                                */
    STANDARD_FIELD_MAC(1  ),
/* byte 17 */
    /*SMEM_LION3_TTI_META_DATA_FIELDS_DSA_QOS_PROFILE_E,                           */
    STANDARD_FIELD_MAC(7  ),
    /*SMEM_LION3_TTI_META_DATA_FIELDS_RESERVED_3_E,                                */
    STANDARD_FIELD_MAC(1  ),
/* byte 18 */
    /*SMEM_LION3_TTI_META_DATA_FIELDS_DSA_SOURCE_ID__7_0_E,                        */
    STANDARD_FIELD_MAC(8  ),
/* byte 19 */
    /*SMEM_LION3_TTI_META_DATA_FIELDS_DSA_SOURCE_ID__11_8_E,                       */
    STANDARD_FIELD_MAC(4  ),
    /*SMEM_LION3_TTI_META_DATA_FIELDS_NUM_OF_MPLS_LABELS_E, sip5_10 only           */
    STANDARD_FIELD_MAC(3  ),
    /*SMEM_LION3_TTI_META_DATA_FIELDS_RESERVED_4_E,                                */
    STANDARD_FIELD_MAC(1  ),
/* byte 20 */
    /*SMEM_LION3_TTI_META_DATA_FIELDS_TRILL_OPTIONS_1ST_NIBBLE_E,                  */
    STANDARD_FIELD_MAC(4  ),
    /*SMEM_LION3_TTI_META_DATA_FIELDS_TRILL_OPTIONS_EXIST_E,                       */
    STANDARD_FIELD_MAC(1  ),
    /*SMEM_LION3_TTI_META_DATA_FIELDS_TRILL_M_BIT_E,                               */
    STANDARD_FIELD_MAC(1  ),
    /*SMEM_LION3_TTI_META_DATA_FIELDS_TRILL_OPTION_LENGTH_EXCEEDED_E,              */
    STANDARD_FIELD_MAC(1  ),
    /*SMEM_LION3_TTI_META_DATA_FIELDS_REP_LAST_E,                                  */
    STANDARD_FIELD_MAC(1  ),
/* byte 21 */
    /*SMEM_LION3_TTI_META_DATA_FIELDS_IPV6_TUNNELING_PROTOCOL_E,                   */
    STANDARD_FIELD_MAC(3  ),
    /*SMEM_LION3_TTI_META_DATA_FIELDS_L3_DATA_WORD0_FIRST_NIBBLE_E,                */
    STANDARD_FIELD_MAC(4  ),
    /*SMEM_LION3_TTI_META_DATA_FIELDS_IPV6_L4_VALID_E,                             */
    STANDARD_FIELD_MAC(1  ),
/* byte 22 */
    /*SMEM_LION3_TTI_META_DATA_FIELDS_PCLID_4_0_E,                                 */
    STANDARD_FIELD_MAC(5  ),
    /*SMEM_LION3_TTI_META_DATA_FIELDS_LOCAL_DEVICE_SOURCE_IS_TRUNK_E,              */
    STANDARD_FIELD_MAC(1  ),
    /*SMEM_LION3_TTI_META_DATA_FIELDS_DSA_TAG_SRCISTRUNK_E,                        */
    STANDARD_FIELD_MAC(1  ),
    /*SMEM_LION3_TTI_META_DATA_FIELDS_MAC_TO_ME_E,                                 */
    STANDARD_FIELD_MAC(1  ),
/* byte 23 */
    /*SMEM_LION3_TTI_META_DATA_FIELDS_PCLID_9_5_E,                                 */
    STANDARD_FIELD_MAC(5  ),
    /*SMEM_LION3_TTI_META_DATA_FIELDS_SRC_COREID_E,                                */
    STANDARD_FIELD_MAC(3  ),
/* byte 24 */
    /*SMEM_LION3_TTI_META_DATA_FIELDS_EVLAN_7_0_E,                                 */
    STANDARD_FIELD_MAC(8  ),
/* byte 25 */
    /*SMEM_LION3_TTI_META_DATA_FIELDS_EVLAN_12_8_E,                                */
    STANDARD_FIELD_MAC(5  ),
    /*SMEM_LION3_SIP7_TTI_META_DATA_FIELDS_EVLAN_15_13_E,                                */
    STANDARD_FIELD_MAC(3  ),
/* byte 26 */
    /*SMEM_LION3_TTI_META_DATA_FIELDS_LOCAL_DEVICE_SOURCE_EPORT_TRUNKID_7_0_E,     */
    STANDARD_FIELD_MAC(8  ),
/* byte 27 */
    /*SMEM_LION3_TTI_META_DATA_FIELDS_LOCAL_DEVICE_SOURCE_EPORT_TRUNKID_12_8_E,    */
    STANDARD_FIELD_MAC(5  ),
    /*SMEM_LION3_TTI_META_DATA_FIELDS_RESERVED_6_E,                                */
    STANDARD_FIELD_MAC(3  ),
/* byte 28 */
    /*SMEM_LION3_TTI_META_DATA_FIELDS_DSA_TAG_SRCPORT_SRCEPORT_TRUNKID_7_0_E,      */
    STANDARD_FIELD_MAC(8  ),
/* byte 29 */
    /*SMEM_LION3_TTI_META_DATA_FIELDS_DSA_TAG_SRCPORT_SRCEPORT_TRUNKID_12_8_E,     */
    STANDARD_FIELD_MAC(5  ),
    /*SMEM_LION3_TTI_META_DATA_FIELDS_RESERVED_7_E,                                */
    STANDARD_FIELD_MAC(3  ),
/* byte 30 */
    /*SMEM_LION3_TTI_META_DATA_FIELDS_DSA_TAG_SRCDEV_7_0_E,                        */
    STANDARD_FIELD_MAC(8  ),
/* byte 31 */
    /*SMEM_LION3_TTI_META_DATA_FIELDS_DSA_TAG_SRCDEV_9_8_E,                        */
    STANDARD_FIELD_MAC(4  ),
    /*SMEM_LION3_TTI_META_DATA_FIELDS_RESERVED_8_E,                                */
    STANDARD_FIELD_MAC(3  ),
    /*SMEM_LION3_TTI_META_DATA_FIELDS_MUST_BE_1_E,                                 */
    STANDARD_FIELD_MAC(1  ),
/* SIP6 META_DATA_FIELDS */
/* byte 32 */
    /*TTI_META_DATA_FIELDS_SR_SEGMENT_LEFT_E,                                */
    STANDARD_FIELD_MAC(8  ),
/* byte 33 */
    /*TTI_META_DATA_FIELDS_SR_EH_EXISTS_E,                                */
    STANDARD_FIELD_MAC(1  ),
    /*TTI_META_DATA_FIELDS_RESERVED_9_E,                                */
    STANDARD_FIELD_MAC(7  ),
/* byte 34..35 */
    /*SMEM_LION3_TTI_META_DATA_FIELDS_SRC_PHY_PORT_7_0_E,                          */
    STANDARD_FIELD_MAC(8  ),
    /*SMEM_LION3_TTI_META_DATA_FIELDS_SRC_PHY_PORT_9_8_E,                          */
    STANDARD_FIELD_MAC(2  ),
    /*SMEM_LION3_TTI_META_DATA_FIELDS_PREEMPTED_E,                                 */
    STANDARD_FIELD_MAC(1  ),
    /*SMEM_LION3_TTI_META_DATA_FIELDS_RESERVED_10_E,                               */
    STANDARD_FIELD_MAC(5  ),
/* byte 36 */
    /*SMEM_LION3_TTI_META_DATA_FIELDS_IPVX_PROTOCOL_E                              */
    STANDARD_FIELD_MAC(8  ),
/* byte 37 */
    /*SMEM_LION3_TTI_META_DATA_FIELDS_IP2ME_MATCH_FOUND_E,                         */
    STANDARD_FIELD_MAC(1  ),
    /*SMEM_LION3_TTI_META_DATA_FIELDS_IP2ME_MATCH_INDEX_E,                         */
    STANDARD_FIELD_MAC(7  ),
/* byte 38 */
    /*SMEM_LION3_TTI_META_DATA_FIELDS_GENERIC_CLASSIFICATION_7_0_E                 */ SIM_TBD_BOOKMARK
    STANDARD_FIELD_MAC(8  ),
/* byte 39 */
    /*SMEM_LION3_TTI_META_DATA_FIELDS_GENERIC_CLASSIFICATION_15_8_E                */
    STANDARD_FIELD_MAC(8  )
};

/* macro to shorten the calling code of 'tti metadat' fields */
#define SMEM_LION3_TTI_METADAT_FIELD_SET(_devObjPtr,_descrPtr,fieldName,value) \
    SNET_TABLE_ENTRY_FIELD_SET(_devObjPtr,_descrPtr->ttiMetadataInfo,0 /* not a table */,fieldName,value,SKERNEL_TABLE_FORMAT_TTI_META_DATA_E)


/**
* @internal logPacketType function
* @endinternal
*
* @brief   Log packet type (for xCat and above)
*/
static GT_VOID logPacketType
(
    IN    SKERNEL_DEVICE_OBJECT               *devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC     *descrPtr,
    IN    PACKET_TYPE_USED_FOR_CLIENT_ENT      clientType,
    IN    SIP5_PACKET_CLASSIFICATION_TYPE_ENT  pcktType_sip5
)
{
    DECLARE_FUNC_NAME(logPacketType);

    GT_CHAR*    typeName = "unknown";
    GT_CHAR*    unitNameArr[PACKET_TYPE_USED_FOR_CLIENT___LAST___E] = {"CLIENT_TTI","CLIENT_IPCL","CLIENT_EPCL"};

    if(!simLogIsOpenFlag)
    {
        return;
    }

    if(0 == SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        switch(descrPtr->pcktType)
        {
            case CHT_PACKET_CLASSIFICATION_TYPE_IPV4_TCP_E     :
                typeName = "IPV4_TCP";
                break;
            case CHT_PACKET_CLASSIFICATION_TYPE_IPV4_UDP_E     :
                typeName = "IPV4_UCP";
                break;
            case CHT_PACKET_CLASSIFICATION_TYPE_MPLS_E         :
                typeName = "MPLS";
                break;
            case CHT_PACKET_CLASSIFICATION_TYPE_IPV4_FRAG_E    :
                typeName = "IPV4_FRAG";
                break;
            case CHT_PACKET_CLASSIFICATION_TYPE_IPV4_OTHER_E   :
                typeName = "IPV4_OTHER";
                break;
            case CHT_PACKET_CLASSIFICATION_TYPE_ETH_OTHER_E    :
                typeName = "ETH_OTHER";
                break;
            case CHT_PACKET_CLASSIFICATION_TYPE_UDE0_E         :
                typeName = "UDE0";
                break;
            case CHT_PACKET_CLASSIFICATION_TYPE_IPV6_E         :
                typeName = "IPV6";
                break;
            case CHT_PACKET_CLASSIFICATION_TYPE_UDE1_E         :
                typeName = "UDE1";
                break;
            case CHT_PACKET_CLASSIFICATION_TYPE_UDE2_E         :
                typeName = "UDE2";
                break;
            case CHT_PACKET_CLASSIFICATION_TYPE_UDE3_E         :
                typeName = "UDE3";
                break;
            case CHT_PACKET_CLASSIFICATION_TYPE_UDE4_E         :
                typeName = "UDE4";
                break;
        }
        __LOG(("packet type %s \n",typeName));
    }
    else
    {
        switch(pcktType_sip5)
        {
            case SIP5_PACKET_CLASSIFICATION_TYPE_IPV4_TCP_E     :
                typeName = "IPV4_TCP";
                break;
            case SIP5_PACKET_CLASSIFICATION_TYPE_IPV4_UDP_E     :
                typeName = "IPV4_UDP";
                break;
            case SIP5_PACKET_CLASSIFICATION_TYPE_MPLS_E         :
                typeName = "MPLS";
                break;
            case SIP5_PACKET_CLASSIFICATION_TYPE_IPV4_FRAG_E    :
                typeName = "IPV4_FRAG";
                break;
            case SIP5_PACKET_CLASSIFICATION_TYPE_IPV4_OTHER_E   :
                typeName = "IPV4_OTHER";
                break;
            case SIP5_PACKET_CLASSIFICATION_TYPE_ETH_OTHER_E    :
                typeName = "ETH_OTHER";
                break;
            case SIP5_PACKET_CLASSIFICATION_TYPE_IPV6_E         :
                typeName = "IPV6";
                break;
            case SIP5_PACKET_CLASSIFICATION_TYPE_IPV6_TCP_E     :
                typeName = "IPV6_TCP";
                break;
            case SIP5_PACKET_CLASSIFICATION_TYPE_IPV6_UDP_E     :
                typeName = "IPV6_UDP";
                break;
            case SIP5_PACKET_CLASSIFICATION_TYPE_UDE0_E         :
                typeName = "UDE0";
                break;
            case SIP5_PACKET_CLASSIFICATION_TYPE_UDE1_E         :
                typeName = "UDE1";
                break;
            case SIP5_PACKET_CLASSIFICATION_TYPE_UDE2_E         :
                typeName = "UDE2";
                break;
            case SIP5_PACKET_CLASSIFICATION_TYPE_UDE3_E         :
                typeName = "UDE3";
                break;
            case SIP5_PACKET_CLASSIFICATION_TYPE_UDE4_E         :
                typeName = "UDE4";
                break;
            case SIP5_PACKET_CLASSIFICATION_TYPE_UDE5_E         :
                typeName = "UDE5";
                break;
            case SIP5_PACKET_CLASSIFICATION_TYPE_UDE6_E         :
                typeName = "UDE6";
                break;
            default:
                typeName = "unknown";
                break;
        }
        __LOG(("%s packet type %s (sip5) \n",
                      unitNameArr[clientType] ,
                      typeName));
    }
}

/**
* @internal pcktTypeAssign function
* @endinternal
*
* @brief   Packet type assignment (for xCat and above)
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] descrPtr                 - pointer to the frame's descriptor.
* @param[in] clientType               - client type.
* @param[in,out] descrPtr                 - pointer to the frame's descriptor.
*/
GT_VOID pcktTypeAssign
(
    IN    SKERNEL_DEVICE_OBJECT             *devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC   *descrPtr,
    IN    PACKET_TYPE_USED_FOR_CLIENT_ENT   clientType
)
{
    DECLARE_FUNC_NAME(pcktTypeAssign);

    GT_U32 fieldVal;
    GT_U32 udeRegIndex = 0;
    GT_BOOL pcktType_sip5_matched = GT_FALSE;
    SIP5_PACKET_CLASSIFICATION_TYPE_ENT pcktType_sip5 = 0;
    CHT_PACKET_CLASSIFICATION_TYPE_ENT  pcktType = 0;
    GT_U32      tempValue;/*temp value*/
    GT_U32      i; /* to iterate half of register value */
    GT_U32      regAddr;

    if(clientType >= PACKET_TYPE_USED_FOR_CLIENT___LAST___E)
    {
        skernelFatalError("pcktTypeAssign: clientType[%d] out of range \n",clientType);
        return;
    }

    if ((descrPtr->isIp == 0) && (descrPtr->mpls == 0))
    {
        __LOG(("packet could not be classified as IP/MPLS - classify as UDE0, UDE 1..4 (if supported)"));

        if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
        {
            for(udeRegIndex = 0; udeRegIndex < 4 && GT_FALSE == pcktType_sip5_matched; udeRegIndex++)
            {
                regAddr = clientType == PACKET_TYPE_USED_FOR_CLIENT_TTI_E ?
                                SMEM_LION3_TTI_TTI_USER_DEFINED_ETHER_TYPES_REG(devObjPtr, udeRegIndex) :
                                SMEM_LION3_TTI_PCL_USER_DEFINED_ETHER_TYPES_REG(devObjPtr, udeRegIndex) ;

                smemRegGet(devObjPtr, regAddr, &fieldVal);
                for(i = 0; i < 2 && GT_FALSE == pcktType_sip5_matched; i++)
                {
                    /* Although UDE7 exists in the register, only UDE0-6 are handled */
                    if((i == 1) && (udeRegIndex == 3))
                    {
                        break;
                    }
                    if(SMEM_U32_GET_FIELD(fieldVal, 16*i, 16) == descrPtr->etherTypeOrSsapDsap)
                    {
                        pcktType_sip5 = (udeRegIndex * 2) + i + SIP5_PACKET_CLASSIFICATION_TYPE_UDE0_E;
                        pcktType_sip5_matched = GT_TRUE;
                    }
                }
            }
        }
        else /* non SIP5 device */
        {
            smemRegFldGet(devObjPtr, SMEM_XCAT_SPECIAL_ETHERTYPES_REG(devObjPtr),
                          0, 16, &fieldVal);
            if(fieldVal == descrPtr->etherTypeOrSsapDsap)
            {
                descrPtr->pcktType = CHT_PACKET_CLASSIFICATION_TYPE_UDE0_E;

                __LOG(("UDE0 \n"));
                return;
            }

            if (devObjPtr->pclSupport.iPclSupport5Ude)
            {
                smemRegGet(devObjPtr, SMEM_LION_TTI_USER_DEFINED_ETHERTYPES_REG(devObjPtr,udeRegIndex),
                                          &fieldVal);
                udeRegIndex++;

                if(SMEM_U32_GET_FIELD(fieldVal,0,16) == descrPtr->etherTypeOrSsapDsap)
                {
                    descrPtr->pcktType = CHT_PACKET_CLASSIFICATION_TYPE_UDE1_E;

                    __LOG(("UDE1 \n"));
                    return;
                }

                if(SMEM_U32_GET_FIELD(fieldVal,16,16) == descrPtr->etherTypeOrSsapDsap)
                {
                    descrPtr->pcktType = CHT_PACKET_CLASSIFICATION_TYPE_UDE2_E;

                    __LOG(("UDE2 \n"));
                    return;
                }

                smemRegGet(devObjPtr, SMEM_LION_TTI_USER_DEFINED_ETHERTYPES_REG(devObjPtr,udeRegIndex),
                                          &fieldVal);
                udeRegIndex++;

                if(SMEM_U32_GET_FIELD(fieldVal,0,16) == descrPtr->etherTypeOrSsapDsap)
                {
                    descrPtr->pcktType = CHT_PACKET_CLASSIFICATION_TYPE_UDE3_E;

                    __LOG(("UDE3 \n"));
                    return;
                }

                if(SMEM_U32_GET_FIELD(fieldVal,16,16) == descrPtr->etherTypeOrSsapDsap)
                {
                    descrPtr->pcktType = CHT_PACKET_CLASSIFICATION_TYPE_UDE4_E;

                    __LOG(("UDE4 \n"));
                    return;
                }
            }
        }
    }

    if(pcktType_sip5_matched == GT_TRUE)
    {
        /* already assigned UDE 0..6*/
    }
    else
    if (descrPtr->isIp == 0) /*non-ip*/
    {
        if(descrPtr->mpls)
        {
            pcktType = CHT_PACKET_CLASSIFICATION_TYPE_MPLS_E;
            pcktType_sip5 = SIP5_PACKET_CLASSIFICATION_TYPE_MPLS_E;
        }
        else
        {
            pcktType = CHT_PACKET_CLASSIFICATION_TYPE_ETH_OTHER_E;
            pcktType_sip5 = SIP5_PACKET_CLASSIFICATION_TYPE_ETH_OTHER_E;
        }
    }
    else if(descrPtr->isIPv4)  /*IPv4*/
    {
        if(descrPtr->ipv4Fragmented > 1 ) /* mid or last fragment */
        {
            pcktType = CHT_PACKET_CLASSIFICATION_TYPE_IPV4_FRAG_E;
            pcktType_sip5 = SIP5_PACKET_CLASSIFICATION_TYPE_IPV4_FRAG_E;
        }
        else if(descrPtr->ipProt == SNET_TCP_PROT_E)
        {
            pcktType = CHT_PACKET_CLASSIFICATION_TYPE_IPV4_TCP_E;
            pcktType_sip5 = SIP5_PACKET_CLASSIFICATION_TYPE_IPV4_TCP_E;
        }
        else if(descrPtr->ipProt == SNET_UDP_PROT_E)
        {
            pcktType = CHT_PACKET_CLASSIFICATION_TYPE_IPV4_UDP_E;
            pcktType_sip5 = SIP5_PACKET_CLASSIFICATION_TYPE_IPV4_UDP_E;
        }
        else
        {
            pcktType = CHT_PACKET_CLASSIFICATION_TYPE_IPV4_OTHER_E;
            pcktType_sip5 = SIP5_PACKET_CLASSIFICATION_TYPE_IPV4_OTHER_E;
        }
    }
    else /*IPv6*/
    {
        pcktType = CHT_PACKET_CLASSIFICATION_TYPE_IPV6_E;
        pcktType_sip5 = SIP5_PACKET_CLASSIFICATION_TYPE_IPV6_E;

        if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
        {
            if(descrPtr->ipProt == SNET_TCP_PROT_E)
            {
                pcktType_sip5 = SIP5_PACKET_CLASSIFICATION_TYPE_IPV6_TCP_E;
            }
            else if(descrPtr->ipProt == SNET_UDP_PROT_E)
            {
                pcktType_sip5 = SIP5_PACKET_CLASSIFICATION_TYPE_IPV6_UDP_E;
            }
        }
    }

    if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        if(clientType == PACKET_TYPE_USED_FOR_CLIENT_IPCL_E)
        {
            descrPtr->pcl_pcktType_sip5  = pcktType_sip5;

            /* support UDE from the TTI action */
            __LOG_PARAM(descrPtr->ipclUdbConfigurationTableUdeIndex);
            if(descrPtr->ipclUdbConfigurationTableUdeIndex)
            {
                tempValue = SIP5_PACKET_CLASSIFICATION_TYPE_UDE0_E +
                            (descrPtr->ipclUdbConfigurationTableUdeIndex - 1);
                __LOG(("updated pcktType_sip5 from[%d] to [%d] \n",
                              descrPtr->pcl_pcktType_sip5,tempValue));

                descrPtr->pcl_pcktType_sip5 = tempValue;
            }
        }
        else if(clientType == PACKET_TYPE_USED_FOR_CLIENT_EPCL_E)
        {
            descrPtr->epcl_pcktType_sip5 = pcktType_sip5;
        }
        else /*PACKET_TYPE_USED_FOR_CLIENT_TTI_E*/
        {
            descrPtr->tti_pcktType_sip5  = pcktType_sip5;
        }

        if(pcktType_sip5_matched == GT_TRUE)
        {
            /* for legacy support of unit that still uses descrPtr->pcktType */

            /* pcktType was not assigned */
            switch(pcktType_sip5)
            {
                case SIP5_PACKET_CLASSIFICATION_TYPE_UDE0_E:
                    pcktType = CHT_PACKET_CLASSIFICATION_TYPE_UDE0_E;
                    break;
                case SIP5_PACKET_CLASSIFICATION_TYPE_UDE1_E:
                    pcktType = CHT_PACKET_CLASSIFICATION_TYPE_UDE1_E;
                    break;
                case SIP5_PACKET_CLASSIFICATION_TYPE_UDE2_E:
                    pcktType = CHT_PACKET_CLASSIFICATION_TYPE_UDE2_E;
                    break;
                case SIP5_PACKET_CLASSIFICATION_TYPE_UDE3_E:
                    pcktType = CHT_PACKET_CLASSIFICATION_TYPE_UDE3_E;
                    break;
                case SIP5_PACKET_CLASSIFICATION_TYPE_UDE4_E:
                    pcktType = CHT_PACKET_CLASSIFICATION_TYPE_UDE4_E;
                    break;
                case SIP5_PACKET_CLASSIFICATION_TYPE_UDE5_E:
                case SIP5_PACKET_CLASSIFICATION_TYPE_UDE6_E:
                    /* legacy not aware to this type */
                    pcktType =  CHT_PACKET_CLASSIFICATION_TYPE_ETH_OTHER_E;
                    break;
                default:
                    /* legacy not aware to this type */
                    pcktType =  CHT_PACKET_CLASSIFICATION_TYPE_ETH_OTHER_E;
                    break;
            }
        }
    }

    /*relevant only to NON-sip5*/
    if(0 == SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        descrPtr->pcktType = pcktType;
    }

    logPacketType(devObjPtr, descrPtr, clientType, pcktType_sip5);
}

/**
* @internal snetCht3TTKeyFieldBuildByPointer function
* @endinternal
*
* @brief   function insert data of field to the search key in specific place in key
*
* @param[in,out] ttKeyPtr                 - (pointer to) current tti key
* @param[in] fieldValPtr              - (pointer to) data of field to insert to key
* @param[in] fieldId                  -- field id
* @param[in,out] ttKeyPtr                 - (pointer to) current tti key
*                                      RETURN:
*                                      COMMENTS:
*/
static GT_VOID snetCht3TTKeyFieldBuildByPointer
(
    INOUT SNET_CHT_POLICY_KEY_STC       *ttKeyPtr,
    IN GT_U8                            *fieldValPtr,
    IN CHT3_TT_KEY_FIELDS_ID_ENT         fieldId
)
{
    CHT_PCL_KEY_FIELDS_INFO_STC *fieldInfoPtr = &cht3TTKeyFieldsData[fieldId];

    snetChtPclSrvKeyFieldBuildByPointer(ttKeyPtr, fieldValPtr, fieldInfoPtr);

    return;
}

/**
* @internal snetCht3TTKeyFieldBuildByValue function
* @endinternal
*
* @brief   function insert data of the field to the search key
*         in specific place in tti search key
* @param[in,out] ttKeyPtr                 - (pointer to) current tti key
* @param[in] fieldVal                 - data of field to insert to key
* @param[in] fieldId                  -- field id
* @param[in,out] ttKeyPtr                 - (pointer to) current tti key
*                                      RETURN:
*                                      COMMENTS:
*/
static GT_VOID snetCht3TTKeyFieldBuildByValue
(
    INOUT SNET_CHT_POLICY_KEY_STC           *ttKeyPtr,
    IN GT_U32                               fieldVal,
    IN CHT3_TT_KEY_FIELDS_ID_ENT            fieldId
)
{
    CHT_PCL_KEY_FIELDS_INFO_STC *fieldInfoPtr = &cht3TTKeyFieldsData[fieldId];

    snetChtPclSrvKeyFieldBuildByValue(ttKeyPtr, fieldVal, fieldInfoPtr);

    return;
}


/**
* @internal snetCht3TTLookUp function
* @endinternal
*
* @brief   Tcam lookup for T.T ENTRY
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to frame descriptor.
* @param[in] ttKeyPtr                 - pointer to T.T key .
*
* @param[out] matchIndexPtr            - pointer to the matching index.
*                                      RETURN:
*                                      COMMENTS:
*                                      Router Tcam Table:
*                                      CH3:  TCAM holds up to 20K entries of 32 bits for IPv4
*                                      or 5K entries of 128 bits for IPv6.
*                                      XCAT: TCAM holds up to 13K entries of 32 bits for IPv4
*                                      or 3.25K entries of 128 bits for IPv6.
*
* @note Router Tcam Table:
*       CH3: TCAM holds up to 20K entries of 32 bits for IPv4
*       or 5K entries of 128 bits for IPv6.
*       XCAT: TCAM holds up to 13K entries of 32 bits for IPv4
*       or 3.25K entries of 128 bits for IPv6.
*
*/
static GT_VOID snetCht3TTLookUp
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr  ,
    IN SNET_CHT_POLICY_KEY_STC    * ttKeyPtr,
    OUT GT_U32 *  matchIndexPtr
)
{
    DECLARE_FUNC_NAME(snetCht3TTLookUp);

    GT_U32   maxEntries;  /* CH3: 5K entries for tunnel, XCAT 3.25K entries for tunnel */
    GT_U32   entryIndex;  /* Index to the TCAM line */
    GT_U32   bankIndex;   /* Index to the word in the TCAM rule */
    GT_U32   xcompModeData;                /* compare mode bit in the tcam rule */
    GT_U32   valid;                    /* compare mode bit in the tcam rule */
    GT_U32   ycompModeData;                /* compare mode bit in the tcam rule */
    GT_U32  *xdataPtr, *xDataTmpPtr;   /* pointer to routing TCAM data X entry */
    GT_U32  *xctrlPtr, *xCtrlTmpPtr;   /* pointer to routing TCAM ctrl X entry  */
    GT_U32  *ydataPtr, *yDataTmpPtr;   /* pointer to routing TCAM data Y entry */
    GT_U32  *yctrlPtr, *yCtrlTmpPtr;   /* pointer to routing TCAM ctrl Y entry  */
    GT_U32 * ttSearchKeyPtr;          /* pointer to tt tcam search0 key */
    GT_U32   ttSearchKey16Bits ;      /* tt tcam search 16 upper bits   */
    GT_U32   ttSearchKey32Bits ;      /* tt tcam search 32 lower bits   */
    GT_U32   keySize = 4;             /* tt tcam number of banks */
    GT_BOOL  match = GT_FALSE;
    GT_U32   compModeData;             /* compare mode bit in the tcam rule     */

    simLogPacketFrameUnitSet(SIM_LOG_FRAME_UNIT_TCAM_E);

    __LOG(("start lookup"));
    /* mark index as No Match */
    *matchIndexPtr = SNET_CHT_POLICY_NO_MATCH_INDEX_CNS;

    maxEntries = SNET_CHT3_TUNNEL_TCAM_SIZE_CNS(devObjPtr);

    maxEntries = maxEntries * 4; /*for every raw we have 4 possible indexes*/

    ttSearchKeyPtr = ttKeyPtr->key.data;

    /*  Get pointer to Tcam data entry */
    xDataTmpPtr = smemMemGet(devObjPtr, SSMEM_CHT3_TCAM_X_DATA_TBL_MEM(devObjPtr));

    /*  Get pointer to Tcam control entry */
    xCtrlTmpPtr = smemMemGet(devObjPtr,SSMEM_CHT3_TCAM_ROUTER_TT_X_CONTROL_TBL_MEM(devObjPtr));

    /*  Get pointer to Tcam data mask entry */
    yDataTmpPtr = smemMemGet(devObjPtr,SSMEM_CHT3_TCAM_Y_DATA_TBL_MEM(devObjPtr));

    /*  Get pointer to Tcam control mask entry */
    yCtrlTmpPtr = smemMemGet(devObjPtr,SSMEM_CHT3_TCAM_ROUTER_TT_Y_CONTROL_TBL_MEM(devObjPtr));

    for (entryIndex = 0 ; entryIndex <  maxEntries;  ++entryIndex)
    {
        /* Calculate the appropriate address for this entry. */
        xdataPtr = xDataTmpPtr + entryIndex * devObjPtr->routeTcamInfo.entryWidth;
        xctrlPtr = xCtrlTmpPtr + entryIndex * devObjPtr->routeTcamInfo.entryWidth;
        ydataPtr = yDataTmpPtr + entryIndex * devObjPtr->routeTcamInfo.entryWidth;
        yctrlPtr = yCtrlTmpPtr + entryIndex * devObjPtr->routeTcamInfo.entryWidth;

        for (bankIndex = 0 ; bankIndex <  keySize  ; ++bankIndex)
        {
            /* calculate 48 bits for word 0,1,2,3 */
            ycompModeData =  (yctrlPtr[0] & (3<<18)) >>18; /* get compmode */
            xcompModeData =   (xctrlPtr[0] & (3<<18)) >>18; /* get compmode */
            valid = ( ((xctrlPtr[0] >> SNET_CONTROL_VALID_BIT_OFFSET_CNS(devObjPtr))  & 1) &
                      ((~yctrlPtr[0] >> SNET_CONTROL_VALID_BIT_OFFSET_CNS(devObjPtr)) & 1)  );

            compModeData = ((xcompModeData) & (~ycompModeData));

            /* for tunnel compModeData should be '1' */
            if ( !valid || !compModeData)
                break;

            if (bankIndex == 0)
            {
                /* calculate 48 bits for bank 0 from the TUNNEL search key */
                ttSearchKey16Bits = SMEM_U32_GET_FIELD(ttSearchKeyPtr[bankIndex +1],0,16);
                ttSearchKey32Bits = ttSearchKeyPtr[bankIndex];
            }
            else if (bankIndex == 1)
            {
                /* calculate 48 bits for bank 1 from the TUNNEL search key */
                ttSearchKey32Bits = SMEM_U32_GET_FIELD(ttSearchKeyPtr[bankIndex],16,16);
                ttSearchKey32Bits |= SMEM_U32_GET_FIELD(ttSearchKeyPtr[bankIndex +1],0,16) << 16;
                ttSearchKey16Bits = SMEM_U32_GET_FIELD(ttSearchKeyPtr[bankIndex +1],16,16);
            }
            else if (bankIndex == 2)
            {
                /* calculate 48 bits for bank 2 from the TUNNEL search key */
                ttSearchKey16Bits = SMEM_U32_GET_FIELD(ttSearchKeyPtr[bankIndex +2],0,16);
                ttSearchKey32Bits = ttSearchKeyPtr[bankIndex+1];
            }
            else if (bankIndex == 3)
            {
                /* calculate 48 bits for bank 3 from the TUNNEL search key */
                ttSearchKey32Bits = SMEM_U32_GET_FIELD(ttSearchKeyPtr[bankIndex +1],16,16);
                ttSearchKey32Bits |= SMEM_U32_GET_FIELD(ttSearchKeyPtr[bankIndex +2],0,16) << 16;
                ttSearchKey16Bits = SMEM_U32_GET_FIELD(ttSearchKeyPtr[bankIndex +2],16,16);
            }
            else  /* in SIP5 this is valid too ... bankIndex == 4 */
            {
                /* calculate 48 bits for bank 4 from the TUNNEL search key */
                ttSearchKey16Bits = SMEM_U32_GET_FIELD(ttSearchKeyPtr[bankIndex +3],0,16);
                ttSearchKey32Bits = ttSearchKeyPtr[bankIndex+2];
            }

            match = GT_FALSE;

            if(simLogIsOpenFlag)
            {
                GT_U32 low32  = 0xFFFFFFFF & ~((~ydataPtr[0] & ttSearchKey32Bits) | (~xdataPtr[0] & ~ttSearchKey32Bits));
                GT_U32 high16 = 0xFFFF & ~((~yctrlPtr[0] & ttSearchKey16Bits) | (~xctrlPtr[0] & ~ttSearchKey16Bits));
                GT_U32 indexInTcam = entryIndex / 4;
                GT_U32  dataPtr[2];
                GT_U32  maskPtr[2];
                GT_U32  keyPtr[2];
                GT_U32  globalBitIndexForLog = (48*bankIndex);
                GT_U32  x,y,k,ii;
                GT_U32  wordIndex;
                GT_U32  bitIndex;

                dataPtr[0] = xdataPtr[0];
                dataPtr[1] = xctrlPtr[0];
                maskPtr[0] = ydataPtr[0];
                maskPtr[1] = yctrlPtr[0];
                keyPtr[0] = ttSearchKey32Bits;
                keyPtr[1] = ttSearchKey16Bits;

                __LOG_NO_LOCATION_META_DATA(("indexInTcam[%d] bankIndex[%d](out of 4 banks) \n",
                    indexInTcam , bankIndex));

                if (low32 || high16) /* no match */
                {
                    __LOG_NO_LOCATION_META_DATA(("NOTE: in terms of CPSS the 'no match' cpssCurrentIndex [0x%x]([%d]) \n",
                        indexInTcam,indexInTcam));
                    /* analyze the bits that are 'NO match' */
                    __LOG_NO_LOCATION_META_DATA(("analyze the GLOBAL bits that are 'NO match' \n"));
                    __LOG_NO_LOCATION_META_DATA(("the global index is in terms of the FS that describes the TTI/PCL key \n"));
                    /* the global index is in terms of the FS that describes the TTI/PCL key */
                    __LOG_NO_LOCATION_META_DATA(("Non matched Bits:"));
                    for(ii = 0 ; ii < 48 ; ii++ , globalBitIndexForLog++)
                    {
                        wordIndex = ii / 32;
                        bitIndex = ii % 32;

                        x = SMEM_U32_GET_FIELD(dataPtr[wordIndex],bitIndex,1);
                        y = SMEM_U32_GET_FIELD(maskPtr[wordIndex],bitIndex,1);
                        k = SMEM_U32_GET_FIELD(keyPtr[wordIndex] ,bitIndex,1);

                        if(X_Y_K_FIND_NO_MATCH(x,y,k,1))
                        {
                            __LOG_NO_LOCATION_META_DATA(("%d,",
                                globalBitIndexForLog));
                        }
                    }
                    __LOG_NO_LOCATION_META_DATA((". \n End of not matched Bits  \n\n"));
                }
                else
                {
                    /* match*/
                    /* explain the bits that where 'exact match' : */
                    __LOG_NO_LOCATION_META_DATA(("NOTE: in terms of CPSS the 'match' cpssCurrentIndex [0x%x]([%d]) \n",
                        indexInTcam,indexInTcam));
                    __LOG_NO_LOCATION_META_DATA(("the global index is in terms of the FS that describes the TTI/PCL key \n"));

                    /* the global index is in terms of the FS that describes the TTI/PCL key */
                    __LOG_NO_LOCATION_META_DATA(("exact matched Bits: (not don't care)"));
                    for(ii = 0 ; ii < 48 ; ii++ , globalBitIndexForLog++)
                    {
                        wordIndex = ii / 32;
                        bitIndex = ii % 32;

                        x = SMEM_U32_GET_FIELD(dataPtr[wordIndex],bitIndex,1);
                        y = SMEM_U32_GET_FIELD(maskPtr[wordIndex],bitIndex,1);

                        if(x != y)/* indication that key match 0 or 1 ant not ignored ! */
                        {
                            __LOG_NO_LOCATION_META_DATA(("%d,",
                                globalBitIndexForLog));
                        }
                    }
                    __LOG_NO_LOCATION_META_DATA((". \n End of matched Bits  \n\n"));
                }

            }

           /* lookup in TCAM */
           if( (((~ydataPtr[0] & ttSearchKey32Bits) | (~xdataPtr[0] & ~ttSearchKey32Bits)) == 0xFFFFFFFF)
                &&
                ((((~yctrlPtr[0] & ttSearchKey16Bits) | (~xctrlPtr[0] & ~ttSearchKey16Bits)) & 0xFFFF) == 0xFFFF))
            {
                /* log tcam content */
                __LOG_TCAM(("tcam matched : xdata: 0x%08X, ydata: 0x%08X, xctrl: 0x%08X, yctrl: 0x%08X\n",
                               xdataPtr[0], ydataPtr[0], xctrlPtr[0], yctrlPtr[0]));

                /* log tcam key */
                simLogTcamTTKey(devObjPtr, ttSearchKey16Bits, ttSearchKey32Bits);

                /* Get to the next bank for this entry*/
                xdataPtr = xdataPtr + devObjPtr->routeTcamInfo.bankWidth / 4;
                xctrlPtr = xctrlPtr + devObjPtr->routeTcamInfo.bankWidth / 4;
                ydataPtr = ydataPtr + devObjPtr->routeTcamInfo.bankWidth / 4;
                yctrlPtr = yctrlPtr + devObjPtr->routeTcamInfo.bankWidth / 4;

                match = GT_TRUE;
                continue;
            }
            else
            {
                /* log tcam info */
                simLogTcamTTNotMatch(devObjPtr, ttSearchKey16Bits, ttSearchKey32Bits,
                                     xdataPtr, ydataPtr, xctrlPtr, yctrlPtr);
                break;
            }

        } /*  find an entry    */

        if (match == GT_TRUE)
        {
            *matchIndexPtr = entryIndex;
            break;
        }

        /* the entries order in bank0 for ipv6 are enrty0, entry4 ...*/
        /* the loop will add additional 1 to the entryIndex */
        entryIndex += (keySize - 1);
    }

    /*we need to return line index and not entry*/
    if (*matchIndexPtr != SNET_CHT_POLICY_NO_MATCH_INDEX_CNS)
    {
        *matchIndexPtr = *matchIndexPtr / keySize;
    }


    if (match == GT_TRUE)
    {
        __LOG(("tcam %s : index[%d]\n",(match == GT_TRUE) ? "matched" : "NO match",(*matchIndexPtr)));
    }

    simLogPacketFrameUnitSet(SIM_LOG_FRAME_UNIT___RESTORE_PREVIOUS_UNIT___E);
}

/**
* @internal snetCht3TtiKeyPclIdGet function
* @endinternal
*
* @brief   Gets PCL ID for TTI key.
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to frame data buffer Id.
* @param[in] ttiKeyType               - tti key type
*                                      OUTPUT:
* @param[in] pclIdPtr                 - pointer to PCL ID
* @param[in] keySizePtr               - the key size (SIP5 device) in bytes (10 / 20 / 30)
*                                      RETURN:
*/
static GT_VOID snetCht3TtiKeyPclIdGet
(
    IN  SKERNEL_DEVICE_OBJECT             *devObjPtr,
    IN  SKERNEL_FRAME_CHEETAH_DESCR_STC   *descrPtr ,
    IN  SKERNEL_CHT3_TTI_KEY_TYPE_ENT     ttiKeyType,
    OUT GT_U32                            *pclIdPtr,
    OUT GT_U32                            *keySizePtr
)
{
    DECLARE_FUNC_NAME(snetCht3TtiKeyPclIdGet);

    GT_U32  regAddr,keySize,regVal,keyIndex;

    if(devObjPtr->supportEArch )
    {
        if((ttiKeyType >= SKERNEL_LION3_TTI_KEY_UDB_PKT_TYPE_0_E) &&
           (ttiKeyType <= SKERNEL_LION3_TTI_KEY_UDB_PKT_TYPE_15_E) )
        {
            keyIndex =  ttiKeyType - SKERNEL_LION3_TTI_KEY_UDB_PKT_TYPE_0_E;
             /* get key size */
            regAddr = SMEM_LION3_TTI_PACKET_TYPE_KEY_REG(devObjPtr, (keyIndex)/2);
            smemRegGet(devObjPtr,regAddr , &regVal);
            /* 2 bits for key size */
            keySize = SMEM_U32_GET_FIELD(regVal,10 + 16*(keyIndex%2), 2);
            if(keySize >= 3)
            {
                skernelFatalError("snetCht3TtiKeyPclIdGet: keySize[%d] not supported from the register \n");
            }

            *keySizePtr = (keySize+1)*10;

            if(pclIdPtr)
            {
                *pclIdPtr = SMEM_U32_GET_FIELD(regVal, 0 + 16*(keyIndex%2), 10);
            }

            if(pclIdPtr && SMEM_CHT_IS_SIP6_10_GET(devObjPtr) &&
                (1 == SMEM_LION3_TTI_PHYSICAL_PORT_ENTRY_FIELD_GET(devObjPtr, descrPtr,
                    SMEM_SIP6_10_TTI_PHYSICAL_PORT_TABLE_FIELDS_PCL_ID_MODE)))
            {
                *pclIdPtr = SMEM_LION3_TTI_PHYSICAL_PORT_ENTRY_FIELD_GET(devObjPtr, descrPtr,
                    SMEM_SIP6_10_TTI_PHYSICAL_PORT_TABLE_FIELDS_PCL_ID);
            }

            return;
        }

        *keySizePtr = 30;/* legacy keys */
    }

    if(pclIdPtr == NULL)
    {
        return;
    }

    if (devObjPtr->supportConfigurableTtiPclId == GT_TRUE)
    {
        switch (ttiKeyType)
        {
        case SKERNEL_CHT3_TTI_KEY_IPV4_E:
            smemRegFldGet(devObjPtr,SMEM_XCAT_TTI_PCL_ID_CONFIG0_REG(devObjPtr), 0, 10, pclIdPtr);
            break;
        case SKERNEL_CHT3_TTI_KEY_MPLS_E:
            smemRegFldGet(devObjPtr,SMEM_XCAT_TTI_PCL_ID_CONFIG0_REG(devObjPtr), 10, 10, pclIdPtr);
            break;
        case SKERNEL_CHT3_TTI_KEY_ETH_E:
            smemRegFldGet(devObjPtr,SMEM_XCAT_TTI_PCL_ID_CONFIG0_REG(devObjPtr), 20, 10, pclIdPtr);
            break;
        case SKERNEL_XCAT_TTI_KEY_MIM_E:
            smemRegFldGet(devObjPtr,SMEM_XCAT_TTI_PCL_ID_CONFIG1_REG(devObjPtr), 0, 10, pclIdPtr);
            break;
        default:
            *pclIdPtr = 0;
            break;
        }
    }
    else
    {
        switch (ttiKeyType)
        {
        case SKERNEL_CHT3_TTI_KEY_IPV4_E:
            *pclIdPtr = 1;
            break;
        case SKERNEL_CHT3_TTI_KEY_MPLS_E:
            *pclIdPtr = 2;
            break;
        case SKERNEL_CHT3_TTI_KEY_ETH_E:
            *pclIdPtr = 3;
            break;
        default:
            *pclIdPtr = 0;
            break;
        }
    }

    if(pclIdPtr && SMEM_CHT_IS_SIP6_10_GET(devObjPtr) &&
        (1 == SMEM_LION3_TTI_PHYSICAL_PORT_ENTRY_FIELD_GET(devObjPtr, descrPtr,
            SMEM_SIP6_10_TTI_PHYSICAL_PORT_TABLE_FIELDS_PCL_ID_MODE)))
    {
        *pclIdPtr = SMEM_LION3_TTI_PHYSICAL_PORT_ENTRY_FIELD_GET(devObjPtr, descrPtr,
            SMEM_SIP6_10_TTI_PHYSICAL_PORT_TABLE_FIELDS_PCL_ID);
    }

    __LOG(("pclId = %d \n", *pclIdPtr));
}


/**
* @internal snetLion3TtiUdbMetaDataBuild function
* @endinternal
*
* @brief   build the metadata byte from the packet descriptor fields
*         (relevant for SIP5 only)
* @param[in] devObjPtr                - (pointer to) device object.
* @param[in] descrPtr                 - (pointer to) frame data buffer Id
*/
static void snetLion3TtiUdbMetaDataBuild
(
    IN  SKERNEL_DEVICE_OBJECT           *devObjPtr,
    IN  SKERNEL_FRAME_CHEETAH_DESCR_STC *descrPtr,
    IN   INTERNAL_TTI_DESC_INFO_STC        *internalTtiInfoPtr
)
{
    DECLARE_FUNC_NAME(snetLion3TtiUdbMetaDataBuild);

    GT_U32 localDevSrcEPortOrTrunkId;/* local device eport or trunkid */
    GT_U32 passengerOuterCfidei,passengerOuterEvid,passengerOuterUp,passengerOuterExists;
    GT_BIT    dsaOrigIsTrunk;
    GT_U32    dsaOrigSrcEPortOrTrnk;
    GT_U32    dsaSrcDev;
    GT_U32    dsaQosProfile;
    GT_U32    dsaSrcId;
    GT_BIT    mac2me = descrPtr->mac2me || internalTtiInfoPtr->metadata.mac2me;
    GT_BIT isTag0Tagged = (descrPtr->origSrcTagged && (descrPtr->nestedVlanAccessPort == 0) && (descrPtr->srcPriorityTagged == 0)) ? 1 : 0;

    __LOG(("build metadata fields so the key can use fields from it \n"));

    localDevSrcEPortOrTrunkId = (descrPtr->localDevSrcTrunkId == 0) ?
                                descrPtr->eArchExtInfo.localDevSrcEPort :
                                descrPtr->localDevSrcTrunkId;

    if(descrPtr->marvellTagged)
    {
        dsaOrigIsTrunk          = descrPtr->ingressDsa.origIsTrunk;
        dsaOrigSrcEPortOrTrnk   = descrPtr->ingressDsa.origSrcEPortOrTrnk;
        dsaSrcDev               = descrPtr->ingressDsa.srcDev;
        dsaQosProfile           = descrPtr->ingressDsa.qosProfile;
        dsaSrcId                = descrPtr->ingressDsa.srcId;
    }
    else
    {
        dsaOrigIsTrunk          = descrPtr->origIsTrunk;
        dsaOrigSrcEPortOrTrnk   = localDevSrcEPortOrTrunkId;
        dsaSrcDev               = descrPtr->ownDev;
        dsaQosProfile           = descrPtr->qos.qosProfile;
        dsaSrcId                = 0;
    }

    if(descrPtr->ingressTunnelInfo.innerPacketL2FieldsAreValid)
    {
        passengerOuterCfidei = descrPtr->ingressTunnelInfo.innerPacketTag0CfiDei;
        passengerOuterEvid = descrPtr->ingressTunnelInfo.innerPacketTag0Vid;
        passengerOuterUp = descrPtr->ingressTunnelInfo.innerPacketTag0Up;
        passengerOuterExists = descrPtr->ingressTunnelInfo.innerTag0Exists;
    }
    else
    {
        passengerOuterCfidei = 0;
        passengerOuterEvid = 0;
        passengerOuterUp = 0;
        passengerOuterExists = 0;
    }

    SMEM_LION3_TTI_METADAT_FIELD_SET(devObjPtr,descrPtr,SMEM_LION3_TTI_META_DATA_FIELDS_IS_ARP_E , descrPtr->arp);
    SMEM_LION3_TTI_METADAT_FIELD_SET(devObjPtr,descrPtr,SMEM_LION3_TTI_META_DATA_FIELDS_IPV4_TUNNELING_PROTOCOL_E , descrPtr->ipTTKeyProtocol);
    SMEM_LION3_TTI_METADAT_FIELD_SET(devObjPtr,descrPtr,SMEM_LION3_TTI_META_DATA_FIELDS_IPV4_ARP_DIP_E,descrPtr->dip[0]);
    SMEM_LION3_TTI_METADAT_FIELD_SET(devObjPtr,descrPtr,SMEM_LION3_TTI_META_DATA_FIELDS_IPV4_ARP_SIP_E,descrPtr->sip[0]);
    SMEM_LION3_TTI_METADAT_FIELD_SET(devObjPtr,descrPtr,SMEM_LION3_TTI_META_DATA_FIELDS_PROTOCOL_ABOVE_MPLS_E,descrPtr->protOverMpls);
    SMEM_LION3_TTI_METADAT_FIELD_SET(devObjPtr,descrPtr,SMEM_LION3_TTI_META_DATA_FIELDS_RESERVED_LABEL_EXISTS_E,descrPtr->mplsReservedLabelExist);
    SMEM_LION3_TTI_METADAT_FIELD_SET(devObjPtr,descrPtr,SMEM_LION3_TTI_META_DATA_FIELDS_DATA_AFTER_INNER_LABEL_E,descrPtr->mplsPayloadData);
    SMEM_LION3_TTI_METADAT_FIELD_SET(devObjPtr,descrPtr,SMEM_LION3_TTI_META_DATA_FIELDS_RESERVED_LABEL_VALUE_E,descrPtr->mplsReservedLabelValue);
    SMEM_LION3_TTI_METADAT_FIELD_SET(devObjPtr,descrPtr,SMEM_LION3_TTI_META_DATA_FIELDS_CONTROL_WORD_CHANNEL_TYPE_PROFILE_E,descrPtr->channelTypeProfile);
    SMEM_LION3_TTI_METADAT_FIELD_SET(devObjPtr,descrPtr,SMEM_LION3_TTI_META_DATA_FIELDS_UP0_B_UP_E,descrPtr->up);
    SMEM_LION3_TTI_METADAT_FIELD_SET(devObjPtr,descrPtr,SMEM_LION3_TTI_META_DATA_FIELDS_CFI0_B_DEI__E,descrPtr->cfidei);
    SMEM_LION3_TTI_METADAT_FIELD_SET(devObjPtr,descrPtr,SMEM_LION3_TTI_META_DATA_FIELDS_VLAN_TAG0_EXISTS_E,isTag0Tagged);
    SMEM_LION3_TTI_METADAT_FIELD_SET(devObjPtr,descrPtr,SMEM_LION3_TTI_META_DATA_FIELDS_PASSENGER_OUTER_CFI_E,passengerOuterCfidei);
    SMEM_LION3_TTI_METADAT_FIELD_SET(devObjPtr,descrPtr,SMEM_LION3_TTI_META_DATA_FIELDS_PASSENGER_OUTER_TAG_VID_7_0_E,passengerOuterEvid);
    SMEM_LION3_TTI_METADAT_FIELD_SET(devObjPtr,descrPtr,SMEM_LION3_TTI_META_DATA_FIELDS_PASSENGER_OUTER_TAG_VID_11_8_E,passengerOuterEvid>>8);
    SMEM_LION3_TTI_METADAT_FIELD_SET(devObjPtr,descrPtr,SMEM_LION3_TTI_META_DATA_FIELDS_PASSENGER_OUTER_TAG_UP_E,passengerOuterUp);
    SMEM_LION3_TTI_METADAT_FIELD_SET(devObjPtr,descrPtr,SMEM_LION3_TTI_META_DATA_FIELDS_PASSENGER_OUTER_TAG_EXIST_E,passengerOuterExists);
    SMEM_LION3_TTI_METADAT_FIELD_SET(devObjPtr,descrPtr,SMEM_LION3_TTI_META_DATA_FIELDS_VLAN1_7_0_E,descrPtr->vid1);
    SMEM_LION3_TTI_METADAT_FIELD_SET(devObjPtr,descrPtr,SMEM_LION3_TTI_META_DATA_FIELDS_VLAN1_11_8_E,descrPtr->vid1>>8);
    SMEM_LION3_TTI_METADAT_FIELD_SET(devObjPtr,descrPtr,SMEM_LION3_TTI_META_DATA_FIELDS_UP1_E,descrPtr->up1);
    SMEM_LION3_TTI_METADAT_FIELD_SET(devObjPtr,descrPtr,SMEM_LION3_TTI_META_DATA_FIELDS_CFI1_E,descrPtr->cfidei1);
    SMEM_LION3_TTI_METADAT_FIELD_SET(devObjPtr,descrPtr,SMEM_LION3_TTI_META_DATA_FIELDS_VLAN1_EXIST_E,TAG1_EXIST_MAC(descrPtr));
    SMEM_LION3_TTI_METADAT_FIELD_SET(devObjPtr,descrPtr,SMEM_LION3_TTI_META_DATA_FIELDS_TAG0_TPID_INDEX_E,descrPtr->tpidIndex[SNET_CHT_TAG_0_INDEX_CNS]);
    SMEM_LION3_TTI_METADAT_FIELD_SET(devObjPtr,descrPtr,SMEM_LION3_TTI_META_DATA_FIELDS_TAG1_TPID_INDEX_E,descrPtr->tpidIndex[SNET_CHT_TAG_1_INDEX_CNS]);
    SMEM_LION3_TTI_METADAT_FIELD_SET(devObjPtr,descrPtr,SMEM_LION3_TTI_META_DATA_FIELDS_DSA_QOS_PROFILE_E,dsaQosProfile);
    SMEM_LION3_TTI_METADAT_FIELD_SET(devObjPtr,descrPtr,SMEM_LION3_TTI_META_DATA_FIELDS_DSA_SOURCE_ID__7_0_E,dsaSrcId);
    SMEM_LION3_TTI_METADAT_FIELD_SET(devObjPtr,descrPtr,SMEM_LION3_TTI_META_DATA_FIELDS_DSA_SOURCE_ID__11_8_E,dsaSrcId>>8);
    if(SMEM_CHT_IS_SIP5_10_GET(devObjPtr))
    {
        SMEM_LION3_TTI_METADAT_FIELD_SET(devObjPtr,descrPtr,SMEM_LION3_TTI_META_DATA_FIELDS_NUM_OF_MPLS_LABELS_E,
            descrPtr->mpls ? (descrPtr->numOfLabels + 1) : 0 );
    }
    SMEM_LION3_TTI_METADAT_FIELD_SET(devObjPtr,descrPtr,SMEM_LION3_TTI_META_DATA_FIELDS_TRILL_OPTIONS_1ST_NIBBLE_E,0);/*TBD*/
    SMEM_LION3_TTI_METADAT_FIELD_SET(devObjPtr,descrPtr,SMEM_LION3_TTI_META_DATA_FIELDS_TRILL_OPTIONS_EXIST_E,0);/*TBD*/
    SMEM_LION3_TTI_METADAT_FIELD_SET(devObjPtr,descrPtr,SMEM_LION3_TTI_META_DATA_FIELDS_TRILL_M_BIT_E,descrPtr->trillInfo.M);
    SMEM_LION3_TTI_METADAT_FIELD_SET(devObjPtr,descrPtr,SMEM_LION3_TTI_META_DATA_FIELDS_TRILL_OPTION_LENGTH_EXCEEDED_E,0);/*TBD*/
    SMEM_LION3_TTI_METADAT_FIELD_SET(devObjPtr,descrPtr,SMEM_LION3_TTI_META_DATA_FIELDS_REP_LAST_E,descrPtr->ttiMcDescInstance);
    SMEM_LION3_TTI_METADAT_FIELD_SET(devObjPtr,descrPtr,SMEM_LION3_TTI_META_DATA_FIELDS_IPV6_TUNNELING_PROTOCOL_E,descrPtr->ipTTKeyProtocol);
    SMEM_LION3_TTI_METADAT_FIELD_SET(devObjPtr,descrPtr,SMEM_LION3_TTI_META_DATA_FIELDS_L3_DATA_WORD0_FIRST_NIBBLE_E,
                                                            descrPtr->l4Valid && descrPtr->l4StartOffsetPtr ? (descrPtr->l4StartOffsetPtr[0] >> 4) : 0);
    SMEM_LION3_TTI_METADAT_FIELD_SET(devObjPtr,descrPtr,SMEM_LION3_TTI_META_DATA_FIELDS_IPV6_L4_VALID_E,descrPtr->l4Valid);
    SMEM_LION3_TTI_METADAT_FIELD_SET(devObjPtr,descrPtr,SMEM_LION3_TTI_META_DATA_FIELDS_PCLID_4_0_E,descrPtr->pclId);
    SMEM_LION3_TTI_METADAT_FIELD_SET(devObjPtr,descrPtr,SMEM_LION3_TTI_META_DATA_FIELDS_LOCAL_DEVICE_SOURCE_IS_TRUNK_E, descrPtr->localDevSrcTrunkId ? 1 : 0);
    SMEM_LION3_TTI_METADAT_FIELD_SET(devObjPtr,descrPtr,SMEM_LION3_TTI_META_DATA_FIELDS_DSA_TAG_SRCISTRUNK_E,dsaOrigIsTrunk);
    SMEM_LION3_TTI_METADAT_FIELD_SET(devObjPtr,descrPtr,SMEM_LION3_TTI_META_DATA_FIELDS_MAC_TO_ME_E,mac2me);
    SMEM_LION3_TTI_METADAT_FIELD_SET(devObjPtr,descrPtr,SMEM_LION3_TTI_META_DATA_FIELDS_PCLID_9_5_E,descrPtr->pclId >> 5);
    SMEM_LION3_TTI_METADAT_FIELD_SET(devObjPtr,descrPtr,SMEM_LION3_TTI_META_DATA_FIELDS_SRC_COREID_E,descrPtr->srcCoreId);
    SMEM_LION3_TTI_METADAT_FIELD_SET(devObjPtr,descrPtr,SMEM_LION3_TTI_META_DATA_FIELDS_EVLAN_7_0_E,descrPtr->eVid);
    SMEM_LION3_TTI_METADAT_FIELD_SET(devObjPtr,descrPtr,SMEM_LION3_TTI_META_DATA_FIELDS_EVLAN_12_8_E,descrPtr->eVid >> 8);
    SMEM_LION3_TTI_METADAT_FIELD_SET(devObjPtr,descrPtr,SMEM_LION3_SIP7_TTI_META_DATA_FIELDS_EVLAN_15_13_E,descrPtr->eVid >> 13);
    SMEM_LION3_TTI_METADAT_FIELD_SET(devObjPtr,descrPtr,SMEM_LION3_TTI_META_DATA_FIELDS_LOCAL_DEVICE_SOURCE_EPORT_TRUNKID_7_0_E,localDevSrcEPortOrTrunkId);
    SMEM_LION3_TTI_METADAT_FIELD_SET(devObjPtr,descrPtr,SMEM_LION3_TTI_META_DATA_FIELDS_LOCAL_DEVICE_SOURCE_EPORT_TRUNKID_12_8_E,localDevSrcEPortOrTrunkId>>8);
    SMEM_LION3_TTI_METADAT_FIELD_SET(devObjPtr,descrPtr,SMEM_LION3_TTI_META_DATA_FIELDS_DSA_TAG_SRCPORT_SRCEPORT_TRUNKID_7_0_E,dsaOrigSrcEPortOrTrnk);
    SMEM_LION3_TTI_METADAT_FIELD_SET(devObjPtr,descrPtr,SMEM_LION3_TTI_META_DATA_FIELDS_DSA_TAG_SRCPORT_SRCEPORT_TRUNKID_12_8_E,dsaOrigSrcEPortOrTrnk>>8);
    SMEM_LION3_TTI_METADAT_FIELD_SET(devObjPtr,descrPtr,SMEM_LION3_TTI_META_DATA_FIELDS_DSA_TAG_SRCDEV_7_0_E,dsaSrcDev);
    SMEM_LION3_TTI_METADAT_FIELD_SET(devObjPtr,descrPtr,SMEM_LION3_TTI_META_DATA_FIELDS_DSA_TAG_SRCDEV_9_8_E,dsaSrcDev>>8);
    SMEM_LION3_TTI_METADAT_FIELD_SET(devObjPtr,descrPtr,SMEM_LION3_TTI_META_DATA_FIELDS_MUST_BE_1_E,1);
    if(SMEM_CHT_IS_SIP6_GET(devObjPtr))
    {
        SMEM_LION3_TTI_METADAT_FIELD_SET(devObjPtr,descrPtr,SMEM_LION3_TTI_META_DATA_FIELDS_SR_SEGMENT_LEFT_E,descrPtr->srSegmentsLeft);
        SMEM_LION3_TTI_METADAT_FIELD_SET(devObjPtr,descrPtr,SMEM_LION3_TTI_META_DATA_FIELDS_SR_EH_EXISTS_E,descrPtr->srEhExists);
        SMEM_LION3_TTI_METADAT_FIELD_SET(devObjPtr,descrPtr,SMEM_LION3_TTI_META_DATA_FIELDS_SRC_PHY_PORT_7_0_E,descrPtr->eArchExtInfo.origSrcPhyPortTrunk);
        SMEM_LION3_TTI_METADAT_FIELD_SET(devObjPtr,descrPtr,SMEM_LION3_TTI_META_DATA_FIELDS_SRC_PHY_PORT_9_8_E,descrPtr->eArchExtInfo.origSrcPhyPortTrunk>>8);
        SMEM_LION3_TTI_METADAT_FIELD_SET(devObjPtr,descrPtr,SMEM_LION3_TTI_META_DATA_FIELDS_IPVX_PROTOCOL_E,descrPtr->ipProt);
    }
    if(SMEM_CHT_IS_SIP6_30_GET(devObjPtr))
    {
        SMEM_LION3_TTI_METADAT_FIELD_SET(devObjPtr,descrPtr,SMEM_LION3_TTI_META_DATA_FIELDS_PREEMPTED_E,descrPtr->preempted);
        SMEM_LION3_TTI_METADAT_FIELD_SET(devObjPtr,descrPtr,SMEM_LION3_TTI_META_DATA_FIELDS_IP2ME_MATCH_FOUND_E,descrPtr->ip2me);
        SMEM_LION3_TTI_METADAT_FIELD_SET(devObjPtr,descrPtr,SMEM_LION3_TTI_META_DATA_FIELDS_IP2ME_MATCH_INDEX_E,(descrPtr->ip2me)?descrPtr->ip2meIndex:0);
    }
    if(SMEM_CHT_IS_SIP7_GET(devObjPtr))
    {
        SMEM_LION3_TTI_METADAT_FIELD_SET(devObjPtr,descrPtr,SMEM_LION3_TTI_META_DATA_FIELDS_GENERIC_CLASSIFICATION_7_0_E,descrPtr->genericClassification[0]);
        SMEM_LION3_TTI_METADAT_FIELD_SET(devObjPtr,descrPtr,SMEM_LION3_TTI_META_DATA_FIELDS_GENERIC_CLASSIFICATION_15_8_E,descrPtr->genericClassification[0]>>8);
    }
}

/**
* @internal snetLion3TtiUdbKeyValueGet function
* @endinternal
*
* @brief   Get user defined value by user defined key.
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to frame data buffer Id.
* @param[in] ttiKeyType               - types of tti key
* @param[in] udbIdx                   - UDB index in UDB configuration entry.
*
* @param[out] byteValuePtr             - pointer to UDB value.
*                                      RETURN:
*                                      GT_OK - OK
*                                      GT_FAIL - Not valid byte
*                                      COMMENTS:
*/
static GT_STATUS  snetLion3TtiUdbKeyValueGet
(
    IN  SKERNEL_DEVICE_OBJECT           *devObjPtr,
    IN  SKERNEL_FRAME_CHEETAH_DESCR_STC *descrPtr,
    IN  SKERNEL_CHT3_TTI_KEY_TYPE_ENT    ttiKeyType,
    IN  GT_U32                           udbIdx,
    OUT GT_U8                           *byteValuePtr,
    IN   INTERNAL_TTI_DESC_INFO_STC        *internalTtiInfoPtr
)
{
    DECLARE_FUNC_NAME(snetLion3TtiUdbKeyValueGet);

    static char* anchorStr[8]={"L2",        "MPLS - 2",     "L3 - 2",       "L4",
                        "not for tti",      "not for tti",  "metadata",     "not for tti"};

    GT_U32  regAddr;            /* register address */
    GT_U32  udbData;            /* 11-bit UDB data */
    GT_U32  *regDataPtr;        /* register data pointer */
    GT_U32  userDefinedAnchor;  /* user defined byte Anchor */
    GT_U32  userDefinedOffset;  /* user defined byte offset from Anchor */
    GT_U32  startBit;           /* start bit in the "TTI User Defined Bytes Configuration" */
    GT_STATUS  rc;
    GT_U32  udbEntryIndex = 0;/* index of the UDB entry (one of 20 entries) */
    GT_U32  numBits = 11,numBitsAnchor = 7;
    GT_U32  maxUdbIdx = (SMEM_CHT_IS_SIP7_GET(devObjPtr)) ? 40 : ((SMEM_CHT_IS_SIP6_30_GET(devObjPtr)) ? 38 : ((SMEM_CHT_IS_SIP6_GET(devObjPtr)) ? 37 : 32));

    ASSERT_PTR(byteValuePtr);

    *byteValuePtr = 0;


    if((udbIdx >= maxUdbIdx) || (!SMEM_CHT_IS_SIP6_GET(devObjPtr) && (udbIdx >= 30)))
    {
        /* supporting up to max UDBs per profile */
        __LOG(("Error : got udbIdx[%d] >= %d (out of range) \n",
                      udbIdx,maxUdbIdx));
        return GT_OK;
    }

    /*Table index =
    * Packet Type keys - 0-15
    * IPv4 key - 16
    * MPLS key - 17
    * MIM key - 18
    * Ethernet key - 19*/
    switch(ttiKeyType)
    {
        case SKERNEL_CHT3_TTI_KEY_IPV4_E:
            udbEntryIndex = 16;
            break;
        case SKERNEL_CHT3_TTI_KEY_MPLS_E:
            udbEntryIndex = 17;
            break;
        case SKERNEL_XCAT_TTI_KEY_MIM_E:
            udbEntryIndex = 18;
            break;
        case SKERNEL_CHT3_TTI_KEY_ETH_E:
            udbEntryIndex = 19;
            break;
        default:
            if((ttiKeyType >= SKERNEL_LION3_TTI_KEY_UDB_PKT_TYPE_0_E) &&
               (ttiKeyType <= SKERNEL_LION3_TTI_KEY_UDB_PKT_TYPE_15_E))
            {
                udbEntryIndex = ttiKeyType - SKERNEL_LION3_TTI_KEY_UDB_PKT_TYPE_0_E;
            }
            else
            {
                skernelFatalError("snetLion3TtiUdbKeyValueGet: unknown ttiKeyType[%d] \n",ttiKeyType);
            }
            break;
    }

    if(SMEM_CHT_IS_SIP6_GET(devObjPtr))
    {
        numBits = 12;
        numBitsAnchor = 8;
    }

    startBit = (udbIdx * numBits);

    regAddr = SMEM_LION3_TTI_UDB_CONF_MEM_REG(devObjPtr, udbEntryIndex);
    regDataPtr = smemMemGet(devObjPtr, regAddr);

    /* get all 11-bit data related to the udbIdx-th UDB */
    udbData = snetFieldValueGet(regDataPtr, startBit, numBits);

    /* bit 0 - valid */
    if (0 == SMEM_U32_GET_FIELD(udbData,0,1))
    {
        __LOG(("Got UDB[%d] with 'valid' = 0 \n",
                      udbIdx));
        return GT_OK;
    }

    /* get UDB value */
    userDefinedAnchor = SMEM_U32_GET_FIELD(udbData,1,3);/* support 3 bits Anchor */
    userDefinedOffset = SMEM_U32_GET_FIELD(udbData,4,numBitsAnchor);

    if(6 == userDefinedAnchor)
    { /* metadata anchor */
        if(descrPtr->ttiMetadataReady == 0)
        {
            /* need to be built once per packet so the metadata bytes can use */
            snetLion3TtiUdbMetaDataBuild(devObjPtr, descrPtr,internalTtiInfoPtr);
            /* indication to not build it again for this packet */
            descrPtr->ttiMetadataReady = 1;
        }

        rc = snetXCatPclUserDefinedByteGet(devObjPtr, descrPtr, userDefinedAnchor,
                                          userDefinedOffset, SNET_UDB_CLIENT_TTI_E, byteValuePtr);
    }
    else if(userDefinedAnchor > 3)
    {
        /*the TTI not supports values 4,5 and 7 */
        rc = GT_FAIL;
    }
    else
    {
        rc = snetXCatPclUserDefinedByteGet(devObjPtr, descrPtr, userDefinedAnchor,
                                          userDefinedOffset, SNET_UDB_CLIENT_TTI_E, byteValuePtr);
    }

    __LOG(("Got UDB[%d] anchor[%s] byte offset[%d] with value[0x%x] \n",
                  udbIdx,
                  anchorStr[userDefinedAnchor],
                  userDefinedOffset,
                  *byteValuePtr));

    if(rc != GT_OK)
    {
        *byteValuePtr = 0;
    }

    return rc;
}

/**
* @internal snetLion3TTIKeyBuildByUdbRange function
* @endinternal
*
* @brief   function insert data of the UDBs to
*         specific place in tti search key
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] descrPtr                 - pointer to frame data buffer Id.
*                                      ttKeyPtr    - (pointer to) current tti key
* @param[in] ttiKeyType               - tti key type
* @param[in] numOfUdb                 - number of UDB bytes to read and build
*/
static GT_STATUS snetLion3TTIKeyBuildByUdbRange
(
    IN    SKERNEL_DEVICE_OBJECT           *devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC *descrPtr,
    INOUT SNET_CHT_POLICY_KEY_STC         *ttiKeyPtr,
    IN    SKERNEL_CHT3_TTI_KEY_TYPE_ENT    ttiKeyType,
    IN    GT_U32                           numOfUdb,
    IN   INTERNAL_TTI_DESC_INFO_STC        *internalTtiInfoPtr
)
{
    DECLARE_FUNC_NAME(snetLion3TTIKeyBuildByUdbRange);

    GT_U32    udbIdx;
    GT_U8     byteValue = 0;
    GT_U32    startBit  = 0;
    GT_STATUS st;

    switch(numOfUdb)
    {
        case 10:
        case 20:
        case 30:
            break;
        default:
            skernelFatalError("snetLion3TTIKeyBuildByUdbRange: unknown numOfUdb[%d] \n",numOfUdb);
            break;
    }

    __LOG(("key length (number of UDBs) [%d] \n", numOfUdb));

    /* for every UDB */
    for(udbIdx = 0; udbIdx < numOfUdb; udbIdx++, startBit += 8)
    {
        /* read UDB value */
        st = snetLion3TtiUdbKeyValueGet(devObjPtr, descrPtr, ttiKeyType, udbIdx, &byteValue,internalTtiInfoPtr);
        if(GT_OK != st)
        {
            __LOG(("Cannot get UDB byte[%d] \n", udbIdx));
            byteValue = 0;
        }

        /* build TTI key byte */
        snetChtPclKeyFieldPut(
            ttiKeyPtr->devObjPtr,
            SNET_CHT_PCL_KEY_PTR(ttiKeyPtr),
            startBit+8, /* end bit */
            startBit,
            byteValue);
    }

    return GT_OK;
}

/*
* @internal snetSip7TTIGenericRangeLookup function
* @endinternal
*
* @brief   function overwrite UDB 28..29 if a range match was found
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] descrPtr             - pointer to frame data buffer Id.
* @param[in] ttKeyPtr                 - (pointer to) current tti key
* @param[in] ttiKeyType               - tti key type
* @param[in] lookupNumber             - range check lookup number (0..1)
*/
static GT_STATUS snetSip7TTIGenericRangeLookup
(
    IN    SKERNEL_DEVICE_OBJECT           *devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC *descrPtr,
    INOUT SNET_CHT_POLICY_KEY_STC         *ttiKeyPtr,
    IN    SKERNEL_CHT3_TTI_KEY_TYPE_ENT    ttiKeyType,
    IN    GT_U32                           lookupNumber
)
{
    DECLARE_FUNC_NAME(snetSip7TTIGenericRangeLookup);

    GT_U32   regAddress,regAddress1;
    GT_U32   *regPtr,*regPtr1;
    GT_U32   rangeCheckEnable;
    GT_U32   rangeCheckAnchor;   /* generic range check byte-pair Anchor */
    GT_U32   rangeCheckOffset;   /* generic range check byte-pair offset from Anchor */
    GT_U8    rangeCheckMask[2]; /* generic range check byte-pair mask */
    GT_U8    rangeCheckByteValue[2]; /* generic range check byte-pair value */
    GT_U32   rangeCheckValue;
    GT_BOOL  matchFound;
    GT_U32   rangeMatchId = 0,calculatedRangeMatchId = 0;
    GT_U32   udbIdx = (lookupNumber == 0) ? 29 : 28;
    GT_U32   startBit = (lookupNumber == 0) ? 29*8 : 28*8;
    GT_STATUS st;
    GT_U32 i;
    static char* anchorStr[8]={"L2",        "MPLS - 2",     "L3 - 2",       "L4",
                        "not for tti",      "not for tti",  "metadata",     "not for tti"};

    regAddress = (lookupNumber == 0) ? SMEM_SIP7_TTI_UNIT_RANGE_CHECK_GLOBAL_CONFIG_0_REG(devObjPtr) :
                                       SMEM_SIP7_TTI_UNIT_RANGE_CHECK_GLOBAL_CONFIG_1_REG(devObjPtr);
    regPtr = smemMemGet(devObjPtr, regAddress);
    rangeCheckEnable = SMEM_U32_GET_FIELD(regPtr[0], 0, 1);
    rangeCheckAnchor = SMEM_U32_GET_FIELD(regPtr[0], 1, 3);
    rangeCheckOffset = SMEM_U32_GET_FIELD(regPtr[0], 4, 8);
    rangeCheckMask[0] = SMEM_U32_GET_FIELD(regPtr[0], 12, 8); /* mask is in network order */
    rangeCheckMask[1] = SMEM_U32_GET_FIELD(regPtr[0], 20, 8);
    __LOG_PARAM(lookupNumber);
    __LOG_PARAM(rangeCheckEnable);
    __LOG_PARAM(rangeCheckAnchor);
    __LOG_PARAM(rangeCheckOffset);
    __LOG_PARAM(rangeCheckMask[0]);
    __LOG_PARAM(rangeCheckMask[1]);

    /* TTI Range Check Anchor Type valid values 0..3 & 6 */
    if(6 == rangeCheckAnchor || rangeCheckAnchor <= 3)
    {
        st = snetXCatPclUserDefinedByteGet(devObjPtr, descrPtr, rangeCheckAnchor,
                                           rangeCheckOffset, SNET_UDB_CLIENT_TTI_E, &(rangeCheckByteValue[0]));
        if(st != GT_OK)
        {
            __LOG(("Cannot get rangeCheckByteValue[0] for lookup %d",lookupNumber));
            rangeCheckByteValue[0] = 0;
        }

        st = snetXCatPclUserDefinedByteGet(devObjPtr, descrPtr, rangeCheckAnchor,
                                           rangeCheckOffset+1, SNET_UDB_CLIENT_TTI_E, &(rangeCheckByteValue[1]));
        if(st != GT_OK)
        {
            __LOG(("Cannot get rangeCheckByteValue[1] for lookup %d",lookupNumber));
            rangeCheckByteValue[1] = 0;
        }
    }
    else
    {
        __LOG(("the TTI doesn't support values 4,5 and 7"));
    }

    rangeCheckByteValue[0] &= rangeCheckMask[0];
    rangeCheckByteValue[1] &= rangeCheckMask[1];
    rangeCheckValue = (rangeCheckByteValue[0] << 8) | rangeCheckByteValue[1]; /* network order */
    __LOG_PARAM(rangeCheckByteValue[0]);
    __LOG_PARAM(rangeCheckByteValue[1]);
    __LOG_PARAM(rangeCheckValue);

    if (rangeCheckEnable == 1)
    {
        matchFound = GT_FALSE;
        __LOG(("TTI Range %d Classification Lookup is triggered",lookupNumber));
        /* go over all entries in the table; entry index 0 is reserved to indicate that a match was not found */
        for (i=1; i<=127; i++)
        {
            regAddress1 = SMEM_SIP7_TTI_LU_UNIT_RANGE_CLASSIFICATION_0_REG(devObjPtr,i);
            regPtr1 = smemMemGet(devObjPtr, regAddress1);
            rangeMatchId = SMEM_U32_GET_FIELD(regPtr1[0], 12, 8); /* The assigned Range Match ID to indicate the range match in the lookup key */

            if ((SMEM_U32_GET_FIELD(regPtr1[0], 0, 1) == 1) /* entry is valid */ &&
                (SMEM_U32_GET_FIELD(regPtr1[0], 20, 1) == lookupNumber) /* entry applies to received Range Check lookup number */)
            {
                if ((SMEM_U32_GET_FIELD(regPtr1[0], 11, 1) == 1) /* entry <Generic Range Profile> is compared to the <Source Port Profile> */ &&
                    (SMEM_U32_GET_FIELD(regPtr1[0], 1, 10) !=
                     SMEM_LION3_TTI_DEFAULT_EPORT_ENTRY_FIELD_GET(devObjPtr, descrPtr,
                        SMEM_SIP7_TTI_DEFAULT_EPORT_TABLE_FIELDS_GENERIC_RANGE_SOURCE_PORT_PROFILE_E)))
                {
                    /* a range match wasn't found; continue to the next entry in the table */
                    continue;
                }

                regAddress1 = SMEM_SIP7_TTI_LU_UNIT_RANGE_CLASSIFICATION_1_REG(devObjPtr,i);
                regPtr1 = smemMemGet(devObjPtr, regAddress1);
                if ((rangeCheckValue >= SMEM_U32_GET_FIELD(regPtr1[0], 0, 16)) /* Range Min Value */ &&
                    (rangeCheckValue <= SMEM_U32_GET_FIELD(regPtr1[0], 16, 16)) /* Range Max Value */)
                {
                    matchFound = GT_TRUE;
                    /* If there are multiple entry matches, the Range Match ID reflects a binary OR
                       on the range_match_id of the matched entries.*/
                    calculatedRangeMatchId |= rangeMatchId;
                }
            }
        }

        /* The TTI Superkey last UDB (UDB29 for TTI range check 0 & UDB28 for TTI range check 1) is overridden with the calculated Range Match ID. */
        snetChtPclKeyFieldPut(
            ttiKeyPtr->devObjPtr,
            SNET_CHT_PCL_KEY_PTR(ttiKeyPtr),
            startBit+7, /* end bit */
            startBit,
            calculatedRangeMatchId);
        if (matchFound == GT_TRUE)
        {
            __LOG(("overwrite UDB[%d] anchor[%s] byte offset[%d] with value[0x%x] for lookup %d\n",
                  udbIdx,
                  anchorStr[rangeCheckAnchor],
                  rangeCheckOffset,
                  calculatedRangeMatchId,
                  lookupNumber));
        }
    }

    return GT_OK;
}

/**
* @internal snetCht3TTCreateKey function
* @endinternal
*
* @brief   Create TT search tcam key.
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] descrPtr                 - pointer to frame data buffer Id.
* @param[in] internalTtiInfoPtr       - (pointer to) internal TTI info
*                                      OUTPUT:
*                                      TTKeyPtr    - pointer to T.T key structure.
*                                      RETURN:
*/
static GT_VOID snetCht3TTCreateKey
(
    IN    SKERNEL_DEVICE_OBJECT             *devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC   *descrPtr,
    OUT   SNET_CHT_POLICY_KEY_STC           *ttKeyPtr,
    OUT   INTERNAL_TTI_DESC_INFO_STC        *internalTtiInfoPtr
)
{
    DECLARE_FUNC_NAME(snetCht3TTCreateKey);

    GT_U32  macMode = 0;        /* Assigns the MAC SA or MAC DA to the  TTI key */
    GT_U32  origSrcPortOrTrnk;/* orig src Eport / trunk */
    GT_U32  pwIncludesControlWord;
    GT_U32  keySize = 0;/*the key size (SIP5 device) in bytes (10 / 20 / 30)*/
    GT_U32  regValue;
    GT_U32  lookupNumber; /* range check lookup number */
    GT_BIT  ttiIpv4KeyDscpFieldEnable; /* Use DSCP field value in IPv4 lookup key
                                          0 = Not Use: Don't use DSCP field value in IPv4 lookup key.
                                          1 = Use: Use DSCP field value in IPv4 lookup key.*/

    SKERNEL_CHT3_TTI_KEY_TYPE_ENT   ttiKeyType = internalTtiInfoPtr->ttiKeyType;

    __LOG(("start build key"));

    snetCht3TtiKeyPclIdGet(devObjPtr, descrPtr, ttiKeyType, &descrPtr->pclId, &keySize);

    if (ttiKeyType == SKERNEL_XCAT_TTI_KEY_MIM_E) /* mim */
    {
        if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
        {
            internalTtiInfoPtr->ttiPassengerOuterTagIsTag0_1_valid = 1;
        }
    }

    if(devObjPtr->supportEArch )
    {
        if((ttiKeyType >= SKERNEL_LION3_TTI_KEY_UDB_PKT_TYPE_0_E) &&
           (ttiKeyType <= SKERNEL_LION3_TTI_KEY_UDB_PKT_TYPE_15_E) )
        { /* create UDB packet type key */
            if(descrPtr->isTrillEtherType)
            {
                internalTtiInfoPtr->ttiPassengerOuterTagIsTag0_1_valid = 1;
            }

            /* create key range */
            snetLion3TTIKeyBuildByUdbRange(devObjPtr, descrPtr, ttKeyPtr, ttiKeyType, keySize,internalTtiInfoPtr);
        }
        else
        { /* create 30 UDBs keys (4 legacy keys) */
            snetLion3TTIKeyBuildByUdbRange(devObjPtr, descrPtr, ttKeyPtr, ttiKeyType, 30,internalTtiInfoPtr);
        }

        /* range check 0..1 lookup may update UDB28,29 is a match is found */
        if(SMEM_CHT_IS_SIP7_GET(devObjPtr))
        {
            for (lookupNumber=0; lookupNumber <=1; lookupNumber++)
            {
                snetSip7TTIGenericRangeLookup(devObjPtr,descrPtr,ttKeyPtr,ttiKeyType,lookupNumber);
            }
        }

        __LOG(("end build key"));
        return;
    }

    if(devObjPtr->supportVpls &&
       ttiKeyType == SKERNEL_CHT3_TTI_KEY_MPLS_E)
    {
        __LOG(("VPLS support for MPLS packet"));

        /* PW includes control word  */
        smemRegFldGet(devObjPtr,SMEM_XCAT_TTI_PSEUDO_WIRE_CONFIG_CONF_REG(devObjPtr), 0, 1, &pwIncludesControlWord);

        descrPtr->ingressTunnelInfo.innerFrameDescrPtr =
            snetChtEqDuplicateDescr(devObjPtr, descrPtr);

        /* jump over the MPLS labels (just like done after TT of packet)*/
        descrPtr->ingressTunnelInfo.innerFrameDescrPtr->l3StartOffsetPtr += 4 *(descrPtr->ingressTunnelInfo.innerFrameDescrPtr->numOfLabels + 1);

        /*If <PW includes control word>==1 for the MPLS key: L2 parsing of inner packet fields are done assuming control word exists*/
        if(pwIncludesControlWord == 1)/* L2 parsing of inner packet fields are done assuming control word exists*/
        {
            __LOG(("PW includes control word"));
            descrPtr->ingressTunnelInfo.innerFrameDescrPtr->l3StartOffsetPtr += 4;
        }

        /* update the length and the start of frame */
        __LOG(("update the length and the start of frame"));
        descrPtr->ingressTunnelInfo.innerFrameDescrPtr->byteCount -=
            (descrPtr->ingressTunnelInfo.innerFrameDescrPtr->l3StartOffsetPtr -
                descrPtr->ingressTunnelInfo.innerFrameDescrPtr->startFramePtr);

        descrPtr->ingressTunnelInfo.innerFrameDescrPtr->startFramePtr =
            descrPtr->ingressTunnelInfo.innerFrameDescrPtr->l3StartOffsetPtr;


        /* reset the L3 pointer , because it is not relevant any more -->
                   will be set inside snetChtL2Parsing(...) for the 'passenger'  */
        descrPtr->ingressTunnelInfo.innerFrameDescrPtr->l3StartOffsetPtr = NULL;
        descrPtr->ingressTunnelInfo.innerFrameDescrPtr->macDaPtr = DST_MAC_FROM_TUNNEL_DSCR(descrPtr->ingressTunnelInfo.innerFrameDescrPtr);
        descrPtr->ingressTunnelInfo.innerFrameDescrPtr->macSaPtr = SRC_MAC_FROM_TUNNEL_DSCR(descrPtr->ingressTunnelInfo.innerFrameDescrPtr);

        /* do re-parsing on the passenger , to set the L2 info
                   (must be done prior to setting the VID from the TTI action ,
                   and other QOS setting of TTI) */
        snetChtL2Parsing(devObjPtr,descrPtr->ingressTunnelInfo.innerFrameDescrPtr,SNET_CHT_FRAME_PARSE_MODE_FROM_TTI_PASSENGER_E,internalTtiInfoPtr);
        descrPtr->ingressTunnelInfo.innerFrameDescrPtr->passengerLength =
            descrPtr->ingressTunnelInfo.innerFrameDescrPtr->byteCount -
                (descrPtr->ingressTunnelInfo.innerFrameDescrPtr->l3StartOffsetPtr -
                    descrPtr->ingressTunnelInfo.innerFrameDescrPtr->startFramePtr);

        descrPtr->ingressTunnelInfo.innerTag0Exists =
            (descrPtr->ingressTunnelInfo.innerFrameDescrPtr->ingressVlanTag0Type !=
                SKERNEL_FRAME_TR101_VLAN_TAG_NOT_FOUND_E) ?
                1 : 0;
        descrPtr->ingressTunnelInfo.innerPacketTag0Vid = descrPtr->ingressTunnelInfo.innerFrameDescrPtr->eVid;

        descrPtr->ingressTunnelInfo.innerMacDaPtr = descrPtr->ingressTunnelInfo.innerFrameDescrPtr->macDaPtr;
        descrPtr->ingressTunnelInfo.innerPacketL2FieldsAreValid = 1;

        __LOG(("did parse on the passenger Ethernet packet , to get mac sa,da , innerTag0Exists,innerPacketTag0Vid,innerPacketL2FieldsAreValid"));

    }

    snetCht3TTKeyFieldBuildByValue(ttKeyPtr, descrPtr->pclId, /* bits[0..9] PCL-ID */
                                    CHT3_TT_KEY_FIELDS_ID_PCL_ID_E);

    if (descrPtr->localDevSrcTrunkId == 0)
    {
        origSrcPortOrTrnk = descrPtr->localDevSrcPort;
        snetCht3TTKeyFieldBuildByValue(ttKeyPtr, 0 ,CHT3_TT_KEY_FIELDS_ID_SRC_IS_TRUNK_E); /* [18] src istrunk */
    }
    else
    {
        origSrcPortOrTrnk = descrPtr->localDevSrcTrunkId;
        snetCht3TTKeyFieldBuildByValue(ttKeyPtr, 1 ,CHT3_TT_KEY_FIELDS_ID_SRC_IS_TRUNK_E);
    }
    snetCht3TTKeyFieldBuildByValue(ttKeyPtr, origSrcPortOrTrnk /*origSrcPortOrTrnk*/, /* bits[10..17] source port/trunk */
                                 CHT3_TT_KEY_FIELDS_ID_SOURCE_PORT_TRUNK_E);
    snetCht3TTKeyFieldBuildByValue(ttKeyPtr, descrPtr->eVid ,CHT3_TT_KEY_FIELDS_ID_VID_E);




    snetCht3TTKeyFieldBuildByValue(ttKeyPtr,descrPtr->ingressVlanTag0Type == SKERNEL_FRAME_TR101_VLAN_TAG_NOT_FOUND_E
                                   ? 0 : 1, CHT3_TT_KEY_FIELDS_ID_PCKT_TAGGED_E);

    if (ttiKeyType == SKERNEL_CHT3_TTI_KEY_MPLS_E) /* mpls */
    {
        smemRegFldGet(devObjPtr, SMEM_XCAT_TTI_ENGINE_CONFIG_REG(devObjPtr), 1, 1, &macMode);
    }
    else if (ttiKeyType == SKERNEL_CHT3_TTI_KEY_IPV4_E) /* ipv4 */
    {
        smemRegFldGet(devObjPtr, SMEM_XCAT_TTI_ENGINE_CONFIG_REG(devObjPtr), 0, 1, &macMode);
    }
    else if (ttiKeyType == SKERNEL_CHT3_TTI_KEY_ETH_E) /* ethernet */
    {
        smemRegFldGet(devObjPtr, SMEM_XCAT_TTI_ENGINE_CONFIG_REG(devObjPtr), 2, 1, &macMode);
    }
    else if (ttiKeyType == SKERNEL_XCAT_TTI_KEY_MIM_E) /* mim */
    {
        smemRegFldGet(devObjPtr, SMEM_XCAT_TTI_ENGINE_CONFIG_REG(devObjPtr), 3, 1, &macMode);
    }

    __LOG(("macMode = [%s]",macMode? "SA" : "DA"));

    if (macMode)
    {
        snetCht3TTKeyFieldBuildByPointer(ttKeyPtr, descrPtr->macSaPtr ,/* [32..79] MAC SA */
                                        CHT3_TT_KEY_FIELDS_ID_MAC_DA_SA_E);
    }
    else
    {
        snetCht3TTKeyFieldBuildByPointer(ttKeyPtr, descrPtr->macDaPtr ,/* [32..79] MAC DA */
                                        CHT3_TT_KEY_FIELDS_ID_MAC_DA_SA_E);
    }

    snetCht3TTKeyFieldBuildByValue(ttKeyPtr, descrPtr->srcDev ,/* [80..86] src_device */
                                        CHT3_TT_KEY_FIELDS_ID_SRC_DEV_E);


    smemRegGet(devObjPtr, SMEM_XCAT_TTI_ENGINE_CONFIG_REG(devObjPtr) , &regValue);
    ttiIpv4KeyDscpFieldEnable = SMEM_U32_GET_FIELD(regValue,12,1);

    if (ttiKeyType == SKERNEL_CHT3_TTI_KEY_IPV4_E)
    {
        snetCht3TTKeyFieldBuildByValue(ttKeyPtr, descrPtr->ipTTKeyProtocol, /* bits[87..89] */
                                CHT3_TT_KEY_FIELDS_ID_TUNNEL_PROTOCOL_E);
        snetCht3TTKeyFieldBuildByValue(ttKeyPtr, descrPtr->sip[0] ,/* [90..121] */
                                        CHT3_TT_KEY_FIELDS_ID_SIP_ADDRESS_E);
        snetCht3TTKeyFieldBuildByValue(ttKeyPtr, descrPtr->dip[0]  ,/* [122..153] */
                                        CHT3_TT_KEY_FIELDS_ID_DIP_ADDRESS_E);
        snetCht3TTKeyFieldBuildByValue(ttKeyPtr,descrPtr->arp, /* [154] is arp */
                                       CHT3_TT_KEY_FIELDS_ID_IS_ARP_E);
        if(ttiIpv4KeyDscpFieldEnable == 1)
        {
            snetCht3TTKeyFieldBuildByValue(ttKeyPtr,descrPtr->dscp, /* [168..173] dscp */
                                           XCAT3_TT_KEY_FIELDS_ID_DSCP_E);
        }
    }
    else if (ttiKeyType == SKERNEL_CHT3_TTI_KEY_MPLS_E)
    {
        snetCht3TTKeyFieldBuildByValue(ttKeyPtr, descrPtr->exp[0], /* bits[87..89] exp1 */
                                        CHT3_TT_KEY_FIELDS_ID_EXP0_E);
        snetCht3TTKeyFieldBuildByValue(ttKeyPtr, descrPtr->exp[1], /* bits[90..92] exp2 */
                                        CHT3_TT_KEY_FIELDS_ID_EXP1_E);
        snetCht3TTKeyFieldBuildByValue(ttKeyPtr, descrPtr->label[0] , /* [93..112] label1 */
                                        CHT3_TT_KEY_FIELDS_ID_LABLE0_E);
        snetCht3TTKeyFieldBuildByValue(ttKeyPtr, descrPtr->label[1] ,/* [113..132] label2*/
                                        CHT3_TT_KEY_FIELDS_ID_LABLE1_E);
        snetCht3TTKeyFieldBuildByValue(ttKeyPtr, descrPtr->exp[2] ,/* [133..135]  exp3*/
                                        CHT3_TT_KEY_FIELDS_ID_EXP2_E);
        snetCht3TTKeyFieldBuildByValue(ttKeyPtr, descrPtr->label[2] , /* [136..155] label3*/
                                        CHT3_TT_KEY_FIELDS_ID_LABLE2_E);
        snetCht3TTKeyFieldBuildByValue(ttKeyPtr, descrPtr->numOfLabels ,/* [156..157] numOfLabels */
                                        CHT3_TT_KEY_FIELDS_ID_NUM_LABLES_E);
        snetCht3TTKeyFieldBuildByValue(ttKeyPtr, descrPtr->protOverMpls, /* bits[158..159]  */
                                        CHT3_TT_KEY_FIELDS_ID_NLP_ABOVE_MPLS_E);

        snetCht3TTKeyFieldBuildByValue(ttKeyPtr, descrPtr->protOverMpls, /* bits[158..159]  */
                                        CHT3_TT_KEY_FIELDS_ID_NLP_ABOVE_MPLS_E);

        if(devObjPtr->supportVpls)
        {
            /* according to chip design : "TTI MPLS Key Changes: new fields are
               placed on reserved bits, therefore no need for <VPLS mode>, legacy
               can mask in TCAM " */

            /*<CW First Nibble>*/
            snetCht3TTKeyFieldBuildByValue(ttKeyPtr,descrPtr->cwFirstNibble,
                                           XCAT_TT_KEY_FIELDS_ID_MPLS_CW_FIRST_NIBBLE_E);

            if(descrPtr->ingressTunnelInfo.innerPacketL2FieldsAreValid &&
               descrPtr->ingressTunnelInfo.innerTag0Exists)
            {
                /*<Inner VLAN-ID Exists>*/
                __LOG(("<Inner VLAN-ID Exists>"));
                snetCht3TTKeyFieldBuildByValue(ttKeyPtr,1,
                                               XCAT_TT_KEY_FIELDS_ID_MPLS_INNER_PACKET_TAG_0_EXISTS_E);
                /*<Inner VLAN-ID>*/
                snetCht3TTKeyFieldBuildByValue(ttKeyPtr, descrPtr->ingressTunnelInfo.innerPacketTag0Vid,
                                                XCAT_TT_KEY_FIELDS_ID_MPLS_INNER_PACKET_TAG_0_VID_E);
            }

            /* the use of the internal header finished */
            __LOG(("the use of the internal header finished"));
            descrPtr->ingressTunnelInfo.innerFrameDescrPtr = NULL;
        }
    }
    else if (ttiKeyType == SKERNEL_CHT3_TTI_KEY_ETH_E)
    {
        snetCht3TTKeyFieldBuildByValue(ttKeyPtr, descrPtr->up, /* bits[87..89] up */
                                        CHT3_TT_KEY_FIELDS_ID_UP0_E);
        snetCht3TTKeyFieldBuildByValue(ttKeyPtr, descrPtr->cfidei, /* bits[90] cfi*/
                                        CHT3_TT_KEY_FIELDS_ID_CFI0_E);
        snetCht3TTKeyFieldBuildByValue(ttKeyPtr,descrPtr->ingressVlanTag1Type == SKERNEL_FRAME_TR101_VLAN_TAG_NOT_FOUND_E
                                   ? 0 : 1, CHT3_TT_KEY_FIELDS_ID_VLAN1_EXIST_E); /* bits[91] vlan1 exists */
        snetCht3TTKeyFieldBuildByValue(ttKeyPtr, descrPtr->vid1 ,/* [92..103] vid1*/
                                        CHT3_TT_KEY_FIELDS_ID_VLAN1_E);
        snetCht3TTKeyFieldBuildByValue(ttKeyPtr, descrPtr->up1 ,/* [104..106]  up1*/
                                        CHT3_TT_KEY_FIELDS_ID_UP1_E);
        snetCht3TTKeyFieldBuildByValue(ttKeyPtr, descrPtr->cfidei1 , /* [107] cfi1*/
                                        CHT3_TT_KEY_FIELDS_ID_CFI1_E);
        snetCht3TTKeyFieldBuildByValue(ttKeyPtr, descrPtr->etherTypeOrSsapDsap ,/* [108..123] ethertype */
                                        CHT3_TT_KEY_FIELDS_ID_ETHER_TYPE_E);
        snetCht3TTKeyFieldBuildByValue(ttKeyPtr, descrPtr->mac2me || internalTtiInfoPtr->metadata.mac2me, /* bits[124]  */
                                        CHT3_TT_KEY_FIELDS_ID_MAC_2_ME_E);
        snetCht3TTKeyFieldBuildByValue(ttKeyPtr,descrPtr->marvellTagged ? descrPtr->sstId : 0, /* bits[168..172] DSA src id */
                                       XCAT_TT_KEY_FIELDS_ID_DSA_SOURCE_ID_E);
        snetCht3TTKeyFieldBuildByValue(ttKeyPtr, descrPtr->qos.qosProfile, /* bits[173..179] DSA qos profile */
                                       XCAT_TT_KEY_FIELDS_ID_DSA_QOS_PROFILE_E);
        snetCht3TTKeyFieldBuildByValue(ttKeyPtr, descrPtr->bpe802_1br_GRP ,/* [180..181] GRP*/
                                        XCAT3_TT_KEY_FIELDS_ID_GRP_E);
    }
    else /*if (ttiKeyType == SKERNEL_XCAT_TTI_KEY_MIM_E)*/
    {
        snetCht3TTKeyFieldBuildByValue(ttKeyPtr, descrPtr->up, /* bits[87..89] b-up */
                                       CHT3_TT_KEY_FIELDS_ID_UP0_E);
        snetCht3TTKeyFieldBuildByValue(ttKeyPtr, descrPtr->cfidei, /* bits[90] b-dp */
                                       CHT3_TT_KEY_FIELDS_ID_CFI0_E);
        snetCht3TTKeyFieldBuildByValue(ttKeyPtr, descrPtr->iSid , /* [92..115] iSid */
                                        XCAT_TT_KEY_FIELDS_ID_I_SID_E);
        snetCht3TTKeyFieldBuildByValue(ttKeyPtr, descrPtr->iUp , /* [116..118]  iUp */
                                        XCAT_TT_KEY_FIELDS_ID_I_UP_E);
        snetCht3TTKeyFieldBuildByValue(ttKeyPtr, descrPtr->iDp , /* [119] iDp */
                                        XCAT_TT_KEY_FIELDS_ID_I_DP_E);
        snetCht3TTKeyFieldBuildByValue(ttKeyPtr, descrPtr->iRes1 ,/* [120..121] iRes1 */
                                        XCAT_TT_KEY_FIELDS_ID_I_RES_1_E);
        snetCht3TTKeyFieldBuildByValue(ttKeyPtr, descrPtr->iRes2 ,/* [122..123] iRes2 */
                                        XCAT_TT_KEY_FIELDS_ID_I_RES_2_E);
    }

    snetCht3TTKeyFieldBuildByValue(ttKeyPtr, descrPtr->origSrcEPortOrTrnk, /* bits[160..166] DSA source port/trunk */
                                     CHT3_TT_KEY_FIELDS_ID_DSA_SOURCE_PORT_TRUNK_E);
    snetCht3TTKeyFieldBuildByValue(ttKeyPtr, descrPtr->origIsTrunk , /* [167] DSA src is trunk */
                                        CHT3_TT_KEY_FIELDS_ID_DSA_SRC_IS_TRUNK_E);

    if(SKERNEL_IS_LION2_DEV(devObjPtr))
    {
        snetCht3TTKeyFieldBuildByValue(ttKeyPtr, descrPtr->srcCoreId, /* [188..190] The source core number on which this packet was originally ingressed on the device. */
                                            LION2_TT_KEY_FIELDS_ID_SRC_PORT_GROUP_BITS_0_2_E);
    }
    else if(SKERNEL_IS_LION_REVISON_B0_DEV(devObjPtr))
    {
        snetCht3TTKeyFieldBuildByValue(ttKeyPtr, descrPtr->srcCoreId, /* [189..190] The source core number on which this packet was originally ingressed on the device. */
                                            LION_TT_KEY_FIELDS_ID_SRC_PORT_GROUP_BITS_0_1_E);
    }

    snetCht3TTKeyFieldBuildByValue(ttKeyPtr, 1 , /* [191] last -- must be 1 */
                                        CHT3_TT_KEY_FIELDS_ID_RESERVED_191_E);

    __LOG(("end build key"));
    return;
}



/**
* @internal snetCht3TTIPv4ExceptionCheck function
* @endinternal
*
* @brief   IPv4 exception in header , before apply the TTI action.
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to frame data buffer Id
*
* @note 13.4.3 page 358
*
*/
static GT_BOOL snetCht3TTIPv4ExceptionCheck
(
    IN SKERNEL_DEVICE_OBJECT             *  devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC   *  descrPtr,
    IN INTERNAL_TTI_DESC_INFO_STC        *internalTtiInfoPtr

)
{
    /***********************/
    /* check with CH2 code */
    /***********************/
    return snetCht2TTIPv4ExceptionCheck(devObjPtr,descrPtr,internalTtiInfoPtr);
}

/**
* @internal snetCht3TTIPv4ExceptionCheckPart2 function
* @endinternal
*
* @brief   IPv4 exception in header - after TTI action
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to frame data buffer Id
*/
static void snetCht3TTIPv4ExceptionCheckPart2
(
    IN SKERNEL_DEVICE_OBJECT             *  devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC   *  descrPtr,
    IN INTERNAL_TTI_DESC_INFO_STC        *internalTtiInfoPtr
)
{
    GT_U32  startBit,cpuCode,fldVal;
    GT_BIT ipHeaderError = internalTtiInfoPtr ?
        internalTtiInfoPtr->preTunnelTerminationInfo.ipHeaderError :
                                           descrPtr->ipHeaderError;
    GT_BIT ipTtiHeaderError = internalTtiInfoPtr ?
        internalTtiInfoPtr->preTunnelTerminationInfo.ipTtiHeaderError :
                                           descrPtr->ipTtiHeaderError;

    DECLARE_FUNC_NAME(snetCht3TTIPv4ExceptionCheckPart2);

    /* the FS describes only 'tunnel terminated' but apply also to 'transit' packets */
    if((descrPtr->tunnelTerminated ||
        descrPtr->ingressTunnelInfo.transitType != SNET_INGRESS_TUNNEL_TRANSIT_TYPE__NONE__E) &&
        internalTtiInfoPtr->preTunnelTerminationInfo.greHeaderError)
    {
        if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
        {
            /* ip header errors */
            if (ipHeaderError ||  ipTtiHeaderError || internalTtiInfoPtr->preTunnelTerminationInfo.ipOptionFragError)
            {
                /* do nothing; higher exception already determine the exception code & packet command */
                return;
            }

            __LOG(("IPv4 GRE Extension Exception \n"));
            startBit = 3;
            cpuCode = SNET_CHT_IPV4_TT_UNSUP_GRE_ERROR;

            smemRegFldGet(devObjPtr,SMEM_LION3_TTI_LOOKUP_IP_EXCEPTION_COMMANDS_REG(devObjPtr) , startBit, 3, &fldVal);

            snetChtIngressCommandAndCpuCodeResolution(devObjPtr,descrPtr,
                                                      descrPtr->packetCmd,
                                                      fldVal,
                                                      descrPtr->cpuCode,
                                                      cpuCode,
                                                      SNET_CHEETAH_ENGINE_UNIT_TTI_E,
                                                      GT_FALSE);
        }
        else
        {
            __LOG(("IPv4 GRE Extension Exception \n"));
            smemRegFldGet(devObjPtr,SMEM_XCAT_TTI_ENGINE_CONFIG_REG(devObjPtr) , 6, 1, &fldVal);

            if (fldVal == 1)
            {
                descrPtr->packetCmd = SKERNEL_CHT2_TT_CMD_HARD_DROP_E;
            }
            else
            {
                descrPtr->packetCmd = SKERNEL_CHT2_TT_CMD_TRAP_E;
                descrPtr->cpuCode = SNET_CHT_IPV4_TT_UNSUP_GRE_ERROR;
            }
        }
    }
}


/**
* @internal snetLion3FcoeExceptionCheckField function
* @endinternal
*
* @brief   FCoE exception field check, relevant for SIP5 only
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] descrPtr                 - pointer to frame data buffer Id
* @param[in] counterRegAddr           - address of counter register
* @param[in] counterRegStartBit       - field start bit in counter register
* @param[in] globalRegStartBit        - field start bit in global register
* @param[in,out] descrPtr                 - pointer to frame data buffer Id
*                                      RETURN:
*                                      COMMENTS:
*                                      This functionality is the same for every field in FCoE exception checks.
*
* @note This functionality is the same for every field in FCoE exception checks.
*
*/
static GT_VOID snetLion3FcoeExceptionCheckField
(
    IN    SKERNEL_DEVICE_OBJECT           *devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC *descrPtr,
    IN    GT_U32                          counterRegAddr,
    IN    GT_U32                          counterRegStartBit,
    IN    GT_U32                          globalRegStartBit
)
{
    DECLARE_FUNC_NAME(snetLion3FcoeExceptionCheckField);

    GT_U32 fldVal = 0;     /* register field's value */

    /* Increase FCoE field Exception Counter */
    smemRegFldGet(devObjPtr, counterRegAddr, counterRegStartBit, 8, &fldVal);

    if(fldVal < 0xff) /* when counter reach 0xff - it sticks to this value */
    {
        __LOG(("Increase FCoE field Exception Counter"));

        fldVal++; /*value of the counter*/
        smemRegFldSet(devObjPtr, counterRegAddr, counterRegStartBit, 8, fldVal);
    }

    /* Check is FCoE field Exception Enabled */
    fldVal = 0;
    smemRegFldGet(devObjPtr, SMEM_LION3_TTI_FCOE_EXC_CONFIG_REG(devObjPtr),
                                            globalRegStartBit, 1, &fldVal);
    if(fldVal)
    {
        __LOG(("FCoE field Exception Enabled"));

        descrPtr->fcoeInfo.fcoeLegal = 0;
    }
}

/**
* @internal snetLion3FcoeAssignPacketCmdAndCpuCode function
* @endinternal
*
* @brief   Assigns packet CMD and CPU code (if need).
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] descrPtr                 - pointer to frame data buffer Id
* @param[in,out] descrPtr                 - pointer to frame data buffer Id
*                                      RETURN:
*                                      COMMENTS:
*/
static GT_VOID snetLion3FcoeAssignPacketCmdAndCpuCode
(
    IN    SKERNEL_DEVICE_OBJECT             *devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC   *descrPtr
)
{
    DECLARE_FUNC_NAME(snetLion3FcoeAssignPacketCmdAndCpuCode);

    GT_U32 fcoePacketCmd;        /* FCoE Packet CMD */
    GT_U32 fcoeExceptionCpuCode; /* FCoE Exception CPU Code */

    /* Get FCoE Packet CMD */
    __LOG(("Get FCoE Packet CMD"));

    smemRegFldGet(devObjPtr, SMEM_LION3_TTI_FCOE_EXC_CONFIG_REG(devObjPtr),
                                8, 3, &fcoePacketCmd);
    /* Get FCoE CPU code */
    __LOG(("Get FCoE CPU code"));

    smemRegFldGet(devObjPtr, SMEM_LION3_TTI_FCOE_EXC_CONFIG_REG(devObjPtr),
                                0, 8, &fcoeExceptionCpuCode);

    snetChtIngressCommandAndCpuCodeResolution(devObjPtr,descrPtr,
                                              descrPtr->packetCmd,
                                              fcoePacketCmd,
                                              descrPtr->cpuCode,
                                              fcoeExceptionCpuCode,
                                              SNET_CHEETAH_ENGINE_UNIT_TTI_E,
                                              GT_FALSE);
}

/**
* @internal snetLion3FcoeExceptionCheck function
* @endinternal
*
* @brief   FCoE exception in header, relevant for SIP5 only
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] descrPtr                 - pointer to frame data buffer Id
* @param[in,out] internalTtiInfoPtr       - (pointer to) internal TTI info
* @param[in,out] descrPtr                 - pointer to frame data buffer Id
* @param[in,out] internalTtiInfoPtr       - (pointer to) internal TTI info
*                                      RETURN:
*                                      GT_BOOL   - true doing T.T / false
*                                      COMMENTS:
*/
static GT_BOOL snetLion3FcoeExceptionCheck
(
    IN    SKERNEL_DEVICE_OBJECT             *devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC   *descrPtr,
    INOUT INTERNAL_TTI_DESC_INFO_STC        *internalTtiInfoPtr
)
{
    DECLARE_FUNC_NAME(snetLion3FcoeExceptionCheck);

    descrPtr->fcoeInfo.fcoeLegal = 1;

    /* Check Ver field */
    if(internalTtiInfoPtr->fcoeL3Info.ver != 0)
    {
        __LOG(("Ver exception"));

        snetLion3FcoeExceptionCheckField(devObjPtr, descrPtr,
                    SMEM_LION3_TTI_FCOE_EXC_COUNT0_REG(devObjPtr), 0, 11);
    }

    /* Check SOF field */
    if((internalTtiInfoPtr->fcoeL3Info.sof != 0x28) &&
       (internalTtiInfoPtr->fcoeL3Info.sof != 0x2d) &&
       (internalTtiInfoPtr->fcoeL3Info.sof != 0x35) &&
       (internalTtiInfoPtr->fcoeL3Info.sof != 0x2e) &&
       (internalTtiInfoPtr->fcoeL3Info.sof != 0x36))
    {
        __LOG(("SOF exception"));

        snetLion3FcoeExceptionCheckField(devObjPtr, descrPtr,
                    SMEM_LION3_TTI_FCOE_EXC_COUNT0_REG(devObjPtr), 8, 12);
    }

    if(descrPtr->l4StartOffsetPtr != NULL)
    {
        /* Check R_CTL field */
        /* check if there is an extended header where we expect to find the FC header */
        if((descrPtr->l4StartOffsetPtr[0] == 0x50) ||
           (descrPtr->l4StartOffsetPtr[0] == 0x51) )
        {
            __LOG(("R_CTL exception"));

            snetLion3FcoeExceptionCheckField(devObjPtr, descrPtr,
                        SMEM_LION3_TTI_FCOE_EXC_COUNT0_REG(devObjPtr), 16, 13);
        }
    }

    /* Check Header Length exception */
    if( ((descrPtr->l23HeaderSize +  4) > 128) ||
        ((descrPtr->l23HeaderSize + 32) > descrPtr->byteCount) )
    {
        __LOG(("Header Length exception"));

        snetLion3FcoeExceptionCheckField(devObjPtr, descrPtr,
                    SMEM_LION3_TTI_FCOE_EXC_COUNT0_REG(devObjPtr), 24, 14);
    }

    /* Check S_ID field */
    if((descrPtr->sip[1] == 0xff) &&
       (descrPtr->sip[2] == 0xff) &&
       (descrPtr->sip[3] == 0xff))
    {
        __LOG(("S_ID exception"));

        snetLion3FcoeExceptionCheckField(devObjPtr, descrPtr,
                    SMEM_LION3_TTI_FCOE_EXC_COUNT1_REG(devObjPtr), 0, 15);
    }

    /* Check MAC DA multi-destination exception */
    if(descrPtr->macDaType == SKERNEL_MULTICAST_MAC_E)
    {
        __LOG(("MAC DA multi-destination exception"));

        snetLion3FcoeExceptionCheckField(devObjPtr, descrPtr,
                    SMEM_LION3_TTI_FCOE_EXC_COUNT1_REG(devObjPtr), 8, 16);
    }

    /* Check if packet contains VFT */
    if(internalTtiInfoPtr->fcoeL3Info.vftInfo.rCtl == 0x50)
    {
        __LOG(("packet contains VFT"));

        /* check VFT Ver field */
        if(internalTtiInfoPtr->fcoeL3Info.vftInfo.ver != 0)
        {
            __LOG(("check VFT Ver field"));

            snetLion3FcoeExceptionCheckField(devObjPtr, descrPtr,
                        SMEM_LION3_TTI_FCOE_EXC_COUNT1_REG(devObjPtr), 16, 17);
        }
        /* check VFT Type field */
        if(internalTtiInfoPtr->fcoeL3Info.vftInfo.type != 0)
        {
            __LOG(("check VFT Type field"));

            snetLion3FcoeExceptionCheckField(devObjPtr, descrPtr,
                        SMEM_LION3_TTI_FCOE_EXC_COUNT1_REG(devObjPtr), 24, 18);
        }
        /* check VFT HopCt field */
        if(internalTtiInfoPtr->fcoeL3Info.vftInfo.hopCt == 1)
        {
            __LOG(("check VFT HopCt field"));

            snetLion3FcoeExceptionCheckField(devObjPtr, descrPtr,
                        SMEM_LION3_TTI_FCOE_EXC_COUNT2_REG(devObjPtr), 0, 19);
        }
    }

    return descrPtr->fcoeInfo.fcoeLegal ? GT_TRUE : GT_FALSE;
}


/**
* @internal snetCht3TTIPv4FragmentCheck function
* @endinternal
*
* @brief   IPv6 fragment check in header
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to frame data buffer Id
*/
static void snetCht3TTIPv4FragmentCheck
(
    IN SKERNEL_DEVICE_OBJECT             *  devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC   *  descrPtr,
    IN INTERNAL_TTI_DESC_INFO_STC        *  internalTtiInfoPtr
)
{
    DECLARE_FUNC_NAME(snetCht3TTIPv4FragmentCheck);

    GT_U32 fldVal;              /* register field's value */
    GT_U32 fragment;            /* fragment packet */
    GT_U32 headerLength;/*ipv4 header length in WORDS */
    GT_BIT ipHeaderError = internalTtiInfoPtr ?
        internalTtiInfoPtr->preTunnelTerminationInfo.ipHeaderError :
                                           descrPtr->ipHeaderError;
    GT_BIT ipTtiHeaderError = internalTtiInfoPtr ?
        internalTtiInfoPtr->preTunnelTerminationInfo.ipTtiHeaderError :
                                           descrPtr->ipTtiHeaderError;
    /****************************/
    /* do extra fragment issues */
    /****************************/

    headerLength = (descrPtr->l3StartOffsetPtr[0] & 0xf);

    fragment = (descrPtr->l3StartOffsetPtr[6] & (1<<5)) != 0 ?
                GT_TRUE :/* fragment */
        /* 13 bits of FragmentOffset in the ip header */
            (((descrPtr->l3StartOffsetPtr[6] & 0x1f)<< 8 |
             (descrPtr->l3StartOffsetPtr[7])) == 0) ?
                GT_FALSE :/* not fragment */
                GT_TRUE;/* fragment */

    if (fragment == GT_FALSE || (headerLength > 5))
    {
        return;
    }

    __LOG(("tunnel terminated Ipv4 error fragment[%d]/options headerLength[%d]in words ",fragment,headerLength));

    if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        /* ip header error or extra TTI Ip header Error */
        if (ipHeaderError ||  ipTtiHeaderError)
        {
            /* do nothing; higher exception already determine the exception code & packet command */
            return;
        }

        smemRegFldGet(devObjPtr,SMEM_LION3_TTI_LOOKUP_IP_EXCEPTION_COMMANDS_REG(devObjPtr) , 12, 3, &fldVal);

        snetChtIngressCommandAndCpuCodeResolution(devObjPtr,descrPtr,
                                                  descrPtr->packetCmd,
                                                  fldVal,
                                                  descrPtr->cpuCode,
                                                  SNET_CHT_IPV4_TT_OPTION_FRAG_ERROR,
                                                  SNET_CHEETAH_ENGINE_UNIT_TTI_E,
                                                  GT_FALSE);

        internalTtiInfoPtr->preTunnelTerminationInfo.ipOptionFragError = 1;
        return ;
    }

    smemRegFldGet(devObjPtr,SMEM_XCAT_TTI_ENGINE_CONFIG_REG(devObjPtr) , 9, 1, &fldVal);

    snetChtIngressCommandAndCpuCodeResolution(devObjPtr,descrPtr,
                                              descrPtr->packetCmd,
                                              (fldVal == 1) ? SKERNEL_CHT3_TT_CMD_HARD_DROP_E : SKERNEL_CHT3_TT_CMD_TRAP_E,
                                              descrPtr->cpuCode,
                                              SNET_CHT_IPV4_TT_OPTION_FRAG_ERROR,
                                              SNET_CHEETAH_ENGINE_UNIT_TTI_E,
                                              GT_FALSE);

    return ;
}



/**
* @internal snetCht3Mac2MeTblLookUp function
* @endinternal
*
* @brief   MAC to ME identification is required to identify packets that need to
*         be tunnel terminated, trapped or routed.
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] descrPtr                 - pointer to frame data buffer Id
* @param[in,out] descrPtr                 - pointer to updated frame data buffer Id
*                                      RETURN:
*                                      COMMENTS:
*/
static GT_VOID snetCht3Mac2MeTblLookUp
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr
)
{
    DECLARE_FUNC_NAME(snetCht3Mac2MeTblLookUp);

    GT_U32 regAddress;
    GT_U32 entryIdx;
    GT_U32 * mac2MeEntryPtr;
    GT_U8  macAddr[6];
    GT_U32 vlan;
    GT_U8  macAddrMask[6];
    GT_U32 vlanMask;
    GT_U32  ii;
    GT_U32  *memPtr;/*pointer to memory*/
    GT_U32  value;/*register value */
    GT_U32  mac2MeIncludeSrcInterface;/*Mac2Me Include Src Interface */
    GT_U32  numOfEntries;
    GT_U32  vlanStartBit;
    GT_U32  vlanNumBits;

    if(! SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        numOfEntries = 8;
        vlanStartBit = 4;
        vlanNumBits = 12;
    }
    else
    {
        numOfEntries = 128;
        vlanStartBit = 0;
        vlanNumBits = 16;
    }

    if(devObjPtr->TTI_mac2me_numEntries)
    {
        numOfEntries = devObjPtr->TTI_mac2me_numEntries;
    }

    /* Lookup MAC2ME table for MAC/VID matching entry */
    __LOG(("Lookup MAC2ME table for MAC/VID matching entry"));
    for (entryIdx = 0; entryIdx < numOfEntries; entryIdx++)
    {
        __LOG(("start mac2me index [%d]",entryIdx));

        regAddress = SMEM_CHT3_MAC_TO_ME_TBL_MEM(devObjPtr, entryIdx);
        mac2MeEntryPtr = smemMemGet(devObjPtr, regAddress);

        /* value for MAC DA */
        macAddr[4] = (GT_U8)SMEM_U32_GET_FIELD(mac2MeEntryPtr[0],24, 8);
        macAddr[5] = (GT_U8)SMEM_U32_GET_FIELD(mac2MeEntryPtr[0],16, 8);
        /* Mask the compare to the VID */
        vlan = SMEM_U32_GET_FIELD(mac2MeEntryPtr[0], vlanStartBit, vlanNumBits);

        /* value for MAC DA */
        macAddr[0] = (GT_U8)SMEM_U32_GET_FIELD(mac2MeEntryPtr[1],24, 8);
        macAddr[1] = (GT_U8)SMEM_U32_GET_FIELD(mac2MeEntryPtr[1],16, 8);
        macAddr[2] = (GT_U8)SMEM_U32_GET_FIELD(mac2MeEntryPtr[1], 8, 8);
        macAddr[3] = (GT_U8)SMEM_U32_GET_FIELD(mac2MeEntryPtr[1], 0, 8);

        /* Mask mask MAC DA */
        macAddrMask[4] = (GT_U8)SMEM_U32_GET_FIELD(mac2MeEntryPtr[2],24, 8);
        macAddrMask[5] = (GT_U8)SMEM_U32_GET_FIELD(mac2MeEntryPtr[2],16, 8);
        /* Mask the compare to the VID */
        vlanMask = SMEM_U32_GET_FIELD(mac2MeEntryPtr[2], 4, 12);

        /* Mask for MAC DA */
        macAddrMask[0] = (GT_U8)SMEM_U32_GET_FIELD(mac2MeEntryPtr[3],24, 8);
        macAddrMask[1] = (GT_U8)SMEM_U32_GET_FIELD(mac2MeEntryPtr[3],16, 8);
        macAddrMask[2] = (GT_U8)SMEM_U32_GET_FIELD(mac2MeEntryPtr[3], 8, 8);
        macAddrMask[3] = (GT_U8)SMEM_U32_GET_FIELD(mac2MeEntryPtr[3], 0, 8);


        for(ii = 0 ; ii < 6 ; ii++)
        {
            if(0 == SNET_CHT_MASK_CHECK(macAddr[ii],macAddrMask[ii],descrPtr->macDaPtr[ii]))
            {
                __LOG(("no match MAC byte index [%d] ,macAddr[ii] = [0x%2.2x],macAddrMask[ii] = [0x%2.2x],descrPtr->macDaPtr[ii] = [0x%2.2x]",
                              ii,macAddr[ii],macAddrMask[ii],descrPtr->macDaPtr[ii]));
                break;
            }
        }

        if(ii != 6)
        {
            continue;
        }

        if(0 == SNET_CHT_MASK_CHECK(vlan,vlanMask,descrPtr->eVid))
        {
            __LOG(("no match vlan = [0x%2.2x],vlanMask = [0x%2.2x],descrPtr->eVid = [0x%2.2x]",
                          vlan,vlanMask,descrPtr->eVid));

            continue;
        }


        __LOG(("macToMe : matched {MAC,vlanId} in index[%d] \n",entryIdx));

        if(! SMEM_CHT_IS_SIP5_GET(devObjPtr))
        {
            /* match */
            break;
        }

        memPtr = &mac2MeEntryPtr[5];
        mac2MeIncludeSrcInterface = snetFieldValueGet(memPtr,0,2);
        /*  0 - do not use the source interface fields as part of the MAC2ME lookup
            1 - use the source interface fields as part of the MAC2ME lookup
            2 - use source interface fields as part of the MAC2ME lookup, but exclude the
                Source Device. This option is relevant when a global source ePort may be
                assigned to the packet*/
        if(mac2MeIncludeSrcInterface == 0) /*Ignore Src Inteface*/
        {
            __LOG(("index[%d] : macToMe mode : do not use the source interface fields as part of the MAC2ME lookup , so entry matched \n",
                entryIdx))
            /* match */
            break;
        }

        if(mac2MeIncludeSrcInterface == 1)
        {
            __LOG(("index[%d] : macToMe mode : use the source interface fields as part of the MAC2ME lookup \n",
                entryIdx))
        }
        else
        if(mac2MeIncludeSrcInterface == 2)
        {
            __LOG(("index[%d] : macToMe mode : use the source interface fields as part of the MAC2ME lookup , exclude srcDev \n",
                entryIdx))
        }
        else
        {
            __LOG(("index[%d] : macToMe mode : configuration ERROR : unknown mode ... consider no MATCH \n",
                entryIdx))
            continue;
        }


        memPtr = &mac2MeEntryPtr[4];
        value = snetFieldValueGet(memPtr,11,15);/* Falcon supports 15 bits */
        if(descrPtr->origSrcEPortOrTrnk !=  value)/*Source Trunk-ID/ePort*/
        {
            /* not considered mac2me */
            __LOG(("no match descrPtr->origSrcEPortOrTrnk[%d] !=  value[%d] \n",descrPtr->origSrcEPortOrTrnk ,  value));
            continue;
        }
        else
        {
            __LOG(("index [%d] : match descrPtr->origSrcEPortOrTrnk[%d] \n",
                entryIdx,
                descrPtr->origSrcEPortOrTrnk));
        }

        value = snetFieldValueGet(memPtr,10,1);
        if(descrPtr->origIsTrunk != value)
        {
            /* not considered mac2me */
            __LOG(("no match descrPtr->origIsTrunk[%d] !=  value[%d]\n",descrPtr->origIsTrunk ,  value));
            continue;
        }
        else
        {
            __LOG(("index [%d] : match descrPtr->origIsTrunk[%d] \n",
                entryIdx,
                descrPtr->origIsTrunk));
        }

        if(mac2MeIncludeSrcInterface == 1)
        {
            value = snetFieldValueGet(memPtr,0,10);
            if(descrPtr->srcDev !=  value)/*Source Dev*/
            {
                /* not considered mac2me */
                __LOG(("no match descrPtr->srcDev[%d] !=  value[%d]\n",descrPtr->srcDev ,  value));
                continue;
            }
            __LOG(("index [%d] : match descrPtr->srcDev[%d] \n",
                entryIdx,
                descrPtr->srcDev));
        }
        else
        {
            __LOG(("excluding check of descrPtr->srcDev[%d] \n",descrPtr->srcDev));
        }

        /* match */
        break;
    }

    if(entryIdx != numOfEntries)
    {
        /* we have match */
        descrPtr->mac2me = 1;

        __LOG(("macToMe matched in index[%d] \n",entryIdx));
    }
    else
    {
        __LOG(("macToMe was not matched \n"));
    }

    __LOG_PARAM(descrPtr->mac2me);

    return;
}


/**
* @internal snetSip6_30Ip2MeTblLookUp function
* @endinternal
*
* @brief   IP to ME identification is required to identify packets that need to
*         be tunnel terminated, trapped or routed.
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] descrPtr             - pointer to frame data buffer Id
* @param[in,out] descrPtr             - pointer to updated frame data buffer Id
*                                      RETURN:
*                                      COMMENTS:
*/
static GT_VOID snetSip6_30Ip2MeTblLookUp
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr
)
{
    DECLARE_FUNC_NAME(snetSip6_30Ip2MeTblLookUp);

    GT_U32 regAddress;
    GT_U32 entryIdx;
    GT_U32 *entryPtr;
    GT_U8 ipAddress[16];
    GT_U8 ipMask[16];
    GT_U8 *dipOffsetPtr = NULL;
    GT_U32 prefixLength;
    GT_BIT includeVlan;
    GT_U32 vlan,data,value;
    GT_BIT entryIsValid;
    GT_BIT entryIsIpv4; /* entry can be IPv4/IPv6 */
    GT_U32 ip2MeIncludeSrcInterface;/*IP2ME Include Src Interface */
    GT_U32 compareOffset;
    GT_BOOL matched;
    GT_U32 mask,jj;

    if (descrPtr->isIp == 0)
    {
        __LOG(("The packet is not IP. Skipping IP address classification.\n"));
        return;
    }

    if (descrPtr->isIPv4)
    {
        dipOffsetPtr = descrPtr->l3StartOffsetPtr + 16;
        compareOffset = 12;
    }
    else
    {
        dipOffsetPtr = descrPtr->l3StartOffsetPtr + 24;
        compareOffset = 0;
    }

    /* Lookup IP2ME table for DIP/VID matching entry */
    __LOG(("Lookup IP2ME table for DIP/VID matching entry"));
    for (entryIdx = 0; entryIdx < devObjPtr->limitedResources.ipToMeNum ; entryIdx++)
    {
        __LOG(("start ip2me index [%d]",entryIdx));

        regAddress = SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.IP2ME[entryIdx].IP2MEConf1;
        entryPtr = smemMemGet(devObjPtr, regAddress);
        entryIsValid = SMEM_U32_GET_FIELD(*entryPtr,29,1);

        if (entryIsValid == 0)
        {
            __LOG(("IP2ME entry %d is not valid. Continue.\n", entryIdx));
            continue;
        }

        /* entryType: IPv4 = 0,IPv6 = 1*/
        entryIsIpv4 = (SMEM_U32_GET_FIELD(*entryPtr,28,1) == 1) ? 0 : 1;
        if(entryIsIpv4 != descrPtr->isIPv4)
        {
            __LOG(("IP2ME entry[%d] skipped because : entry type [%s] but "
                   "current packet is [%s] \n", entryIdx ,
                   entryIsIpv4 ? "IPv4":"IPv6",
                   descrPtr->isIPv4 ? "IPv6":"IPv4"));
            continue;
        }

        data = SMEM_U32_GET_FIELD(*entryPtr,0,28); /* save source interface info */

        regAddress = SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.IP2ME[entryIdx].IP2MEConf0;
        entryPtr = smemMemGet(devObjPtr, regAddress);
        prefixLength = SMEM_U32_GET_FIELD(*entryPtr,0,8);
        vlan = SMEM_U32_GET_FIELD(*entryPtr,8,(SMEM_CHT_IS_SIP7_GET(devObjPtr) ? 16 : 13));
        includeVlan = SMEM_U32_GET_FIELD(*entryPtr,(SMEM_CHT_IS_SIP7_GET(devObjPtr) ? 24 : 21),1);

        memset(ipAddress, 0, sizeof(ipAddress));
        memset(ipMask, 0, sizeof(ipMask));

        if(descrPtr->isIPv4 == 0) /* IPv6 */
        {
            compareOffset = 0;
            for(jj = 0; jj < 4; jj++)
            {
                switch (jj)
                {
                case 0:
                    regAddress = SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.IP2ME[entryIdx].IP2MEDip0;/* DIP[31:0] */
                    break;
                case 1:
                    regAddress = SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.IP2ME[entryIdx].IP2MEDip1;/* DIP[63:32] */
                    break;
                case 2:
                    regAddress = SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.IP2ME[entryIdx].IP2MEDip2;/* DIP[95:64] */
                    break;
                default:
                    regAddress = SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.IP2ME[entryIdx].IP2MEDip3;/* DIP[127:96] */
                    break;
                }

                entryPtr = smemMemGet(devObjPtr, regAddress);
                ipAddress[((3-jj)*4)    ] = SMEM_U32_GET_FIELD(*entryPtr,24,8);
                ipAddress[((3-jj)*4) + 1] = SMEM_U32_GET_FIELD(*entryPtr,16,8);
                ipAddress[((3-jj)*4) + 2] = SMEM_U32_GET_FIELD(*entryPtr,8,8);
                ipAddress[((3-jj)*4) + 3] = SMEM_U32_GET_FIELD(*entryPtr,0,8);

                if(prefixLength >= 32)
                {
                    /* Prefix length more than 32 bits - fill 32 bit word[jj] by 0xFFFFFFFF */
                    ipMask[(jj*4)    ] = 0xFF;
                    ipMask[(jj*4) + 1] = 0xFF;
                    ipMask[(jj*4) + 2] = 0xFF;
                    ipMask[(jj*4) + 3] = 0xFF;
                    prefixLength = prefixLength-32;
                }
                else
                {
                    if (prefixLength != 0)
                    {
                        /* Align mask to be LSB left most bits in word[jj] */
                        mask = SMEM_BIT_MASK(prefixLength) << (32 - prefixLength);
                        ipMask[(jj*4)    ] = SMEM_U32_GET_FIELD(mask,24,8);
                        ipMask[(jj*4) + 1] = SMEM_U32_GET_FIELD(mask,16,8);
                        ipMask[(jj*4) + 2] = SMEM_U32_GET_FIELD(mask,8,8);
                        ipMask[(jj*4) + 3] = SMEM_U32_GET_FIELD(mask,0,8);
                        prefixLength = 0;
                    }
                }
            }
        }
        else /* IPv4 */
        {
            compareOffset = 12;
            regAddress = SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.IP2ME[entryIdx].IP2MEDip0; /* DIP[31:0] */
            entryPtr = smemMemGet(devObjPtr, regAddress);
            ipAddress[12 + 0] = SMEM_U32_GET_FIELD(*entryPtr,24,8);
            ipAddress[12 + 1] = SMEM_U32_GET_FIELD(*entryPtr,16,8);
            ipAddress[12 + 2] = SMEM_U32_GET_FIELD(*entryPtr,8,8);
            ipAddress[12 + 3] = SMEM_U32_GET_FIELD(*entryPtr,0,8);

            /* Align mask to be LSB left most bits in word[jj] */
            mask = SMEM_BIT_MASK(prefixLength) << (32 - prefixLength);
            ipMask[12 + 0] = SMEM_U32_GET_FIELD(mask,24,8);
            ipMask[12 + 1] = SMEM_U32_GET_FIELD(mask,16,8);
            ipMask[12 + 2] = SMEM_U32_GET_FIELD(mask,8,8);
            ipMask[12 + 3] = SMEM_U32_GET_FIELD(mask,0,8);
        }

        matched = GT_TRUE;
        for(jj = compareOffset; jj < 16; jj++)
        {
            if(0 == SNET_CHT_MASK_CHECK(ipAddress[jj],ipMask[jj],dipOffsetPtr[jj-compareOffset]))
            {
                __LOG(("IP2ME entry[%d] skipped because : at byte index [%d] "
                       "mask[0x%2.2x] , pattern[0x%2.2x] and packet[0x%2.2x] not match.\n",
                        entryIdx,jj,
                        ipMask[jj],
                        ipAddress[jj],
                        dipOffsetPtr[jj-compareOffset]));
                matched = GT_FALSE;
                break;
            }
        }
        if(matched == GT_FALSE)
        {
           continue;
        }

        if(includeVlan && (vlan != descrPtr->eVid))
        {
            __LOG(("IP2ME entry[%d] skipped because : no match vlan = [0x%2.2x],descrPtr->eVid = [0x%2.2x]",
                          entryIdx,vlan,descrPtr->eVid));

            continue;
        }

        __LOG(("IP2ME : matched {DIP,vlanId} in index[%d] \n",entryIdx));

        ip2MeIncludeSrcInterface = snetFieldValueGet(&data,26,2);
        /*  0 - do not use the source interface fields as part of the IP2ME lookup
            1 - use the source interface fields as part of the IP2ME lookup
            2 - use source interface fields as part of the IP2ME lookup, but exclude the
                Source Device. This option is relevant when a global source ePort may be
                assigned to the packet*/
        if(ip2MeIncludeSrcInterface == 0) /*Ignore Src Inteface*/
        {
            __LOG(("index[%d] : IP2ME mode : do not use the source interface fields as part of the IP2ME lookup , so entry matched \n",
                entryIdx))
            /* match */
            break;
        }

        if(ip2MeIncludeSrcInterface == 1)
        {
            __LOG(("index[%d] : IP2ME mode : use the source interface fields as part of the IP2ME lookup \n",
                entryIdx))
        }
        else
        if(ip2MeIncludeSrcInterface == 2)
        {
            __LOG(("index[%d] : IP2ME mode : use the source interface fields as part of the IP2ME lookup , exclude srcDev \n",
                entryIdx))
        }
        else
        {
            __LOG(("index[%d] : IP2ME mode : configuration ERROR : unknown mode ... consider no MATCH \n",
                entryIdx))
            continue;
        }

        value = snetFieldValueGet(&data,11,15);
        if(descrPtr->origSrcEPortOrTrnk !=  value)/*Source Trunk-ID/ePort*/
        {
            /* not considered ip2me */
            __LOG(("IP2ME entry[%d] skipped because : no match descrPtr->origSrcEPortOrTrnk[%d] !=  value[%d] \n",
                   entryIdx,descrPtr->origSrcEPortOrTrnk ,value));
            continue;
        }
        else
        {
            __LOG(("IP2ME index [%d] : match descrPtr->origSrcEPortOrTrnk[%d] \n",
                entryIdx,
                descrPtr->origSrcEPortOrTrnk));
        }

        value = snetFieldValueGet(&data,10,1);
        if(descrPtr->origIsTrunk != value)
        {
            /* not considered ip2me */
            __LOG(("IP2ME entry[%d] skipped because : no match descrPtr->origIsTrunk[%d] !=  value[%d]\n",
                   entryIdx,descrPtr->origIsTrunk ,value));
            continue;
        }
        else
        {
            __LOG(("IP2ME index [%d] : match descrPtr->origIsTrunk[%d] \n",
                entryIdx,
                descrPtr->origIsTrunk));
        }

        if(ip2MeIncludeSrcInterface == 1)
        {
            value = snetFieldValueGet(&data,0,10);
            if(descrPtr->srcDev != value)/*Source Dev*/
            {
                /* not considered ip2me */
                __LOG(("IP2ME entry[%d] skipped because : no match descrPtr->srcDev[%d] !=  value[%d]\n",
                       entryIdx,descrPtr->srcDev ,value));
                continue;
            }
            __LOG(("IP2ME index [%d] : match descrPtr->srcDev[%d] \n",
                entryIdx,
                descrPtr->srcDev));
        }
        else
        {
            __LOG(("IP2ME index [%d] excluding check of descrPtr->srcDev[%d] \n",entryIdx,descrPtr->srcDev));
        }

        /* match */
        break;
    }

    if(entryIdx != 8)
    {
        /* we have match */
        descrPtr->ip2me = 1;
        descrPtr->ip2meIndex = entryIdx;

        __LOG(("IP2ME matched in index[%d] \n",entryIdx));
    }
    else
    {
        __LOG(("IP2ME was not matched \n"));
    }

    __LOG_PARAM(descrPtr->ip2me);
    __LOG_PARAM(descrPtr->ip2meIndex);

    return;
}

/**
* @internal snetLion2TrillAdjacencyCheck function
* @endinternal
*
* @brief   TRILL Adjacency Check (Supported from SIP5)
*         Need to perform 2 adjacency checks:
*         1. TRILL UC IS-IS adjacency check :
*         Checks that the packet arrives from a (neighbor, port) for which an
*         IS-IS adjacency exists.
*         2. TRILL MC Tree Adjacency Check
*         Checks that the multi-destination frame arrives from a (neighbor, port)
*         that is a branch on the given TRILL distribution tree.
*         If there is TCAM MISS, invoke the respective UC or Multi-Target exception command:
*         1. If UC (TRILL.M == 0) assign the global<Trill IS-IS Adjacency exception Cmd>
*         2. If Multi-target (TRILL.M == 1) assign the global<Trill Tree Adjacency exception Cmd>
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] descrPtr                 - pointer to frame data buffer Id
* @param[in,out] descrPtr                 - pointer to updated frame data buffer Id
*                                      RETURN:
*                                      COMMENTS:
*/
static GT_VOID snetLion2TrillAdjacencyCheck
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr
)
{
    DECLARE_FUNC_NAME(snetLion2TrillAdjacencyCheck);

    GT_U32  adjacencyExceptionCmd;/*exception command in case of no match*/
    GT_U32  cpuCodeOffset;/*cpu code offset in case of exception command 'to_cpu'/'mirror'*/
    GT_U32  e;
    GT_U8   AdjacencyCheckEntryMacSa[6];
    GT_BIT  AdjacencyCheckEntryMbit;
    GT_U32  AdjacencyCheckEgressRbid;
    GT_U32  AdjacencyCheckEntrySrcDev;
    GT_BIT  AdjacencyCheckEntryIsTrunk;
    GT_U32  AdjacencyCheckEntrySrcEPortOrTrnk;
    GT_U32  *AdjecencyCheckEntryReg0Ptr;
    GT_U32  *AdjecencyCheckEntryReg1Ptr;
    GT_U32  *AdjecencyCheckEntryReg2Ptr;
    GT_BOOL match;

    match = GT_FALSE;
    for(e = 0 ; e <= 127 ; e++) {
        AdjecencyCheckEntryReg0Ptr = smemMemGet(devObjPtr, SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.TRILLAdjacency.adjacencyEntryReg0[e]);
        AdjecencyCheckEntryReg1Ptr = smemMemGet(devObjPtr, SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.TRILLAdjacency.adjacencyEntryReg1[e]);
        AdjecencyCheckEntryReg2Ptr = smemMemGet(devObjPtr, SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.TRILLAdjacency.adjacencyEntryReg2[e]);
        AdjacencyCheckEntryMacSa[5] = (GT_U8)SMEM_U32_GET_FIELD(*AdjecencyCheckEntryReg0Ptr, 0, 8);
        AdjacencyCheckEntryMacSa[4] = (GT_U8)SMEM_U32_GET_FIELD(*AdjecencyCheckEntryReg0Ptr, 8, 8);
        AdjacencyCheckEntryMacSa[3] = (GT_U8)SMEM_U32_GET_FIELD(*AdjecencyCheckEntryReg0Ptr, 16, 8);
        AdjacencyCheckEntryMacSa[2] = (GT_U8)SMEM_U32_GET_FIELD(*AdjecencyCheckEntryReg0Ptr, 24, 8);
        AdjacencyCheckEntryMacSa[1] = (GT_U8)SMEM_U32_GET_FIELD(*AdjecencyCheckEntryReg1Ptr, 0, 8);
        AdjacencyCheckEntryMacSa[0] = (GT_U8)SMEM_U32_GET_FIELD(*AdjecencyCheckEntryReg1Ptr, 8, 8);
        AdjacencyCheckEgressRbid = SMEM_U32_GET_FIELD(*AdjecencyCheckEntryReg1Ptr, 16, 16);
        AdjacencyCheckEntryMbit = (GT_BIT)SMEM_U32_GET_FIELD(*AdjecencyCheckEntryReg2Ptr, 0, 1);
        AdjacencyCheckEntrySrcDev = SMEM_U32_GET_FIELD(*AdjecencyCheckEntryReg2Ptr, 1, 10);
        AdjacencyCheckEntrySrcEPortOrTrnk = SMEM_U32_GET_FIELD(*AdjecencyCheckEntryReg2Ptr, 11, 15);
        AdjacencyCheckEntryIsTrunk = SMEM_U32_GET_FIELD(*AdjecencyCheckEntryReg2Ptr, 26, 1);

        if ((memcmp(descrPtr->macSaPtr, AdjacencyCheckEntryMacSa, 6) == 0) &&
            ((AdjacencyCheckEgressRbid == descrPtr->trillInfo.eRbid && descrPtr->trillInfo.M == 1) || descrPtr->trillInfo.M == 0) &&
            (AdjacencyCheckEntryMbit == descrPtr->trillInfo.M) &&
            (AdjacencyCheckEntrySrcDev == descrPtr->srcDev) &&
            (AdjacencyCheckEntrySrcEPortOrTrnk == descrPtr->origSrcEPortOrTrnk) &&
            (AdjacencyCheckEntryIsTrunk == descrPtr->origIsTrunk))
        {
            match = GT_TRUE;
            __LOG(("match on adjacency check entry %d", e));
            break;
        }
    }

    if(!match)
    {
        /* no match */
        __LOG(("no match"));
        /*
        If there is TCAM MISS, invoke the respective UC or Multi-Target exception command:
        1. If UC (TRILL.M == 0) assign the global<Trill IS-IS Adjacency exception Cmd>
        2. If Multi-target (TRILL.M == 1) assign the global<Trill Tree Adjacency exception Cmd>
        */

        if(descrPtr->trillInfo.M)
        {
            smemRegFldGet(devObjPtr,
                SMEM_LION2_TTI_TRILL_GENERAL_EXCEPTION_COMMANDS_0_REG(devObjPtr),
                3,3,&adjacencyExceptionCmd);
            cpuCodeOffset = 1;
        }
        else
        {
            smemRegFldGet(devObjPtr,
                SMEM_LION2_TTI_TRILL_GENERAL_EXCEPTION_COMMANDS_0_REG(devObjPtr),
                0,3,&adjacencyExceptionCmd);
            cpuCodeOffset = 0;
        }


        /* resolve packet command and CPU code */
        snetChtIngressCommandAndCpuCodeResolution(devObjPtr,descrPtr,
                                                  descrPtr->packetCmd,
                                                  adjacencyExceptionCmd,
                                                  descrPtr->cpuCode,
                                                  descrPtr->trillInfo.trillCpuCodeBase + cpuCodeOffset,
                                                  SNET_CHEETAH_ENGINE_UNIT_TTI_E,
                                                  GT_FALSE);

    }
}

/**
* @internal snetLion2TrillGeneralExceptionChecking function
* @endinternal
*
* @brief   The following exception checks are performed on all TRILL packet that
*         trigger TRILL engine processing:
*         If TRILL.version > global<max TRILL version>
*         assign global<Trill bad Version Cmd>
*         If TRILL.HopCount == 0
*         assign global<Trill hopcount is Zero exception Cmd>
*         If TRILL.Oplen > 0
*         assign global<Trill Options exception Cmd>
*         If TRILL.Options.CHbH ==1
*         assign global<Trill CHbH exception Cmd>
*         If Outer.VID0 != ePort<TRILL Outer VID0>
*         assign global<Trill bad Outer VID0 exception Cmd>
*         If Outer.DA is UC && Desc<MAC2ME>!=1
*         assign global<Trill UC Not2ME exception Cmd>
*         If Outer.DA is MC && Trill.M==0
*         assign global<Trill MC bad Outer.DA exception Cmd>
*         If Outer.DA is UC && Trill.M==1
*         assign global<Trill UC bad Outer.DA Cmd>
*         If Outer.DA is UC && Inner.DA is MC - bit 40 set
*         assign global<Trill outer UC inner MC exception Cmd>
*         If Trill.M=1 and Outer.DA != All-Rbridges multicast address (01-80-C2-00-00-40)
*         assign global<Trill MC with bad Outer DA exception Cmd>
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] descrPtr                 - pointer to frame data buffer Id
* @param[in,out] descrPtr                 - pointer to updated frame data buffer Id
*                                      RETURN:
*                                      COMMENTS:
*/
static GT_VOID snetLion2TrillGeneralExceptionChecking
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr
)
{
    GT_U32   exceptionCmd;/*exception command in case exception*/
    GT_U32  cpuCodeOffset;/*cpu code offset incase of exception command 'to_cpu'/'mirror'*/
    GT_U32  bitOffset;/*offset of the field from start of the register*/
    GT_U32  value,valGlobal1,valGlobal2;/*register value*/
    GT_U32  maxTrillVersion;/*max TRILL version*/
    GT_U32  tmpValue,tmpCpuCodeOffset,tmpBitOffset;

    smemRegGet(devObjPtr,SMEM_LION2_TTI_TRILL_GLOBAL_CONFIG0_REG(devObjPtr),&value);
    maxTrillVersion = SMEM_U32_GET_FIELD(value,16,2); /*max TRILL version*/
    /*    If TRILL.version > global<max TRILL version>
    *        assign global<Trill bad Version Cmd>
    */
    bitOffset = 6;
    cpuCodeOffset = 2;

    smemRegGet(devObjPtr,SMEM_LION2_TTI_TRILL_GENERAL_EXCEPTION_COMMANDS_0_REG(devObjPtr),&value);

    if(descrPtr->trillInfo.V > maxTrillVersion)
    {
        exceptionCmd = SMEM_U32_GET_FIELD(value,bitOffset,3);
        /* resolve packet command and CPU code */
        snetChtIngressCommandAndCpuCodeResolution(devObjPtr,descrPtr,
                                                  descrPtr->packetCmd,
                                                  exceptionCmd,
                                                  descrPtr->cpuCode,
                                                  descrPtr->trillInfo.trillCpuCodeBase + cpuCodeOffset,
                                                  SNET_CHEETAH_ENGINE_UNIT_TTI_E,
                                                  GT_FALSE);
    }

    /*    If TRILL.HopCount == 0
    *        assign global<Trill hopcount is Zero exception Cmd>
    */
    bitOffset = 9;
    cpuCodeOffset += 1;
    if(descrPtr->trillInfo.hopCount == 0)
    {
        exceptionCmd = SMEM_U32_GET_FIELD(value,bitOffset,3);
        /* resolve packet command and CPU code */
        snetChtIngressCommandAndCpuCodeResolution(devObjPtr,descrPtr,
                                                  descrPtr->packetCmd,
                                                  exceptionCmd,
                                                  descrPtr->cpuCode,
                                                  descrPtr->trillInfo.trillCpuCodeBase + cpuCodeOffset,
                                                  SNET_CHEETAH_ENGINE_UNIT_TTI_E,
                                                  GT_FALSE);
    }

    /*    If TRILL.Oplen > 0
    *        assign global<Trill Options exception Cmd>
    */
    bitOffset = 12;
    cpuCodeOffset += 1;
    if(descrPtr->trillInfo.opLength > 0)
    {
        if(descrPtr->trillInfo.opLength > 10)
        {
            tmpCpuCodeOffset = 16;
            tmpBitOffset = 18;

            smemRegGet(devObjPtr,SMEM_LION2_TTI_TRILL_GENERAL_EXCEPTION_COMMANDS_0_REG(devObjPtr),&tmpValue);
            exceptionCmd = SMEM_U32_GET_FIELD(tmpValue,tmpBitOffset,3);

            /* resolve packet command and CPU code */
            snetChtIngressCommandAndCpuCodeResolution(devObjPtr,descrPtr,
                                                      descrPtr->packetCmd,
                                                      exceptionCmd,
                                                      descrPtr->cpuCode,
                                                      descrPtr->trillInfo.trillCpuCodeBase + tmpCpuCodeOffset,
                                                      SNET_CHEETAH_ENGINE_UNIT_TTI_E,
                                                      GT_FALSE);
        }
        else
        {
            exceptionCmd = SMEM_U32_GET_FIELD(value,bitOffset,3);

            /* resolve packet command and CPU code */
            snetChtIngressCommandAndCpuCodeResolution(devObjPtr,descrPtr,
                                                      descrPtr->packetCmd,
                                                      exceptionCmd,
                                                      descrPtr->cpuCode,
                                                      descrPtr->trillInfo.trillCpuCodeBase + cpuCodeOffset,
                                                      SNET_CHEETAH_ENGINE_UNIT_TTI_E,
                                                      GT_FALSE);
        }
    }

    /*    If TRILL.Options.CHbH ==1
    *        assign global<Trill CHbH exception Cmd>
    */
    bitOffset = 15;
    cpuCodeOffset += 1;
    if(descrPtr->trillInfo.CHbH == 1)
    {
        exceptionCmd = SMEM_U32_GET_FIELD(value,bitOffset,3);
        /* resolve packet command and CPU code */
        snetChtIngressCommandAndCpuCodeResolution(devObjPtr,descrPtr,
                                                  descrPtr->packetCmd,
                                                  exceptionCmd,
                                                  descrPtr->cpuCode,
                                                  descrPtr->trillInfo.trillCpuCodeBase + cpuCodeOffset,
                                                  SNET_CHEETAH_ENGINE_UNIT_TTI_E,
                                                  GT_FALSE);
    }

    /*    If Outer.VID0 != ePort<TRILL Outer VID0>
    *        assign global<Trill bad Outer VID0 exception Cmd>
    */
    bitOffset = 18;
    cpuCodeOffset += 1;
    if(descrPtr->eVid !=
        SMEM_LION3_TTI_DEFAULT_EPORT_ENTRY_FIELD_GET(devObjPtr, descrPtr,
            SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_TRILL_INGRESS_OUTER_VID0))/*trill Outer VID0*/
    {
        exceptionCmd = SMEM_U32_GET_FIELD(value,bitOffset,3);
        /* resolve packet command and CPU code */
        snetChtIngressCommandAndCpuCodeResolution(devObjPtr,descrPtr,
                                                  descrPtr->packetCmd,
                                                  exceptionCmd,
                                                  descrPtr->cpuCode,
                                                  descrPtr->trillInfo.trillCpuCodeBase + cpuCodeOffset,
                                                  SNET_CHEETAH_ENGINE_UNIT_TTI_E,
                                                  GT_FALSE);
    }

    /*    If Outer.DA is UC && Desc<MAC2ME>!=1
    *        assign global<Trill UC Not2ME exception Cmd>
    */
    bitOffset = 21;
    cpuCodeOffset += 1;
    if(descrPtr->macDaType == SKERNEL_UNICAST_MAC_E && descrPtr->mac2me == 0)
    {
        exceptionCmd = SMEM_U32_GET_FIELD(value,bitOffset,3);
        /* resolve packet command and CPU code */
        snetChtIngressCommandAndCpuCodeResolution(devObjPtr,descrPtr,
                                                  descrPtr->packetCmd,
                                                  exceptionCmd,
                                                  descrPtr->cpuCode,
                                                  descrPtr->trillInfo.trillCpuCodeBase + cpuCodeOffset,
                                                  SNET_CHEETAH_ENGINE_UNIT_TTI_E,
                                                  GT_FALSE);
    }

    /*    If Outer.DA is MC && Trill.M==0
    *        assign global<Trill MC bad Outer.DA exception Cmd>
    */
    bitOffset = 24;
    cpuCodeOffset += 1;
    if(descrPtr->macDaType != SKERNEL_UNICAST_MAC_E && descrPtr->trillInfo.M == 0)
    {
        exceptionCmd = SMEM_U32_GET_FIELD(value,bitOffset,3);
        /* resolve packet command and CPU code */
        snetChtIngressCommandAndCpuCodeResolution(devObjPtr,descrPtr,
                                                  descrPtr->packetCmd,
                                                  exceptionCmd,
                                                  descrPtr->cpuCode,
                                                  descrPtr->trillInfo.trillCpuCodeBase + cpuCodeOffset,
                                                  SNET_CHEETAH_ENGINE_UNIT_TTI_E,
                                                  GT_FALSE);
    }

    /*    If Outer.DA is UC && Trill.M==1
    *        assign global<Trill UC bad Outer.DA Cmd>
    */
    bitOffset = 27;
    cpuCodeOffset += 1;
    if(descrPtr->macDaType == SKERNEL_UNICAST_MAC_E && descrPtr->trillInfo.M == 1)
    {
        exceptionCmd = SMEM_U32_GET_FIELD(value,bitOffset,3);
        /* resolve packet command and CPU code */
        snetChtIngressCommandAndCpuCodeResolution(devObjPtr,descrPtr,
                                                  descrPtr->packetCmd,
                                                  exceptionCmd,
                                                  descrPtr->cpuCode,
                                                  descrPtr->trillInfo.trillCpuCodeBase + cpuCodeOffset,
                                                  SNET_CHEETAH_ENGINE_UNIT_TTI_E,
                                                  GT_FALSE);
    }

    smemRegGet(devObjPtr,SMEM_LION2_TTI_TRILL_GENERAL_EXCEPTION_COMMANDS_1_REG(devObjPtr),&value);
    /*    If Outer.DA is UC && Inner.DA is MC - bit 40 set
    *        assign global<Trill outer UC inner MC exception Cmd>
    */
    bitOffset = 0;
    cpuCodeOffset += 1;
    if(descrPtr->macDaType == SKERNEL_UNICAST_MAC_E && (descrPtr->ingressTunnelInfo.innerFrameDescrPtr)->macDaType != SKERNEL_UNICAST_MAC_E)
    {
        exceptionCmd = SMEM_U32_GET_FIELD(value,bitOffset,3);
        /* resolve packet command and CPU code */
        snetChtIngressCommandAndCpuCodeResolution(devObjPtr,descrPtr,
                                                  descrPtr->packetCmd,
                                                  exceptionCmd,
                                                  descrPtr->cpuCode,
                                                  descrPtr->trillInfo.trillCpuCodeBase + cpuCodeOffset,
                                                  SNET_CHEETAH_ENGINE_UNIT_TTI_E,
                                                  GT_FALSE);
    }

    smemRegGet(devObjPtr,SMEM_LION2_TTI_TRILL_GLOBAL_CONFIG1_REG(devObjPtr),&valGlobal1);
    smemRegGet(devObjPtr,SMEM_LION2_TTI_TRILL_GLOBAL_CONFIG2_REG(devObjPtr),&valGlobal2);

    /*    If Trill.M=1 and Outer.DA != All-Rbridges multicast address (01-80-C2-00-00-40)
    *        assign global<Trill MC with bad Outer DA exception Cmd>
    */
    bitOffset = 3;
    cpuCodeOffset += 1;
    if(descrPtr->trillInfo.M == 1 &&
        !(descrPtr->macDaPtr[0] == SMEM_U32_GET_FIELD(valGlobal2,24,8) &&
          descrPtr->macDaPtr[1] == SMEM_U32_GET_FIELD(valGlobal2,16,8) &&
          descrPtr->macDaPtr[2] == SMEM_U32_GET_FIELD(valGlobal2, 8,8) &&
          descrPtr->macDaPtr[3] == SMEM_U32_GET_FIELD(valGlobal2, 0,8) &&
          descrPtr->macDaPtr[4] == SMEM_U32_GET_FIELD(valGlobal1,24,8) &&
          descrPtr->macDaPtr[5] == SMEM_U32_GET_FIELD(valGlobal1,16,8) ))
    {
        exceptionCmd = SMEM_U32_GET_FIELD(value,bitOffset,3);
        /* resolve packet command and CPU code */
        snetChtIngressCommandAndCpuCodeResolution(devObjPtr,descrPtr,
                                                  descrPtr->packetCmd,
                                                  exceptionCmd,
                                                  descrPtr->cpuCode,
                                                  descrPtr->trillInfo.trillCpuCodeBase + cpuCodeOffset,
                                                  SNET_CHEETAH_ENGINE_UNIT_TTI_E,
                                                  GT_FALSE);
    }


}

/**
* @internal snetLion2TrillEngine function
* @endinternal
*
* @brief   TRILL Engine (supported from Lion2)
*         NOTE: TRILL engine resides in the TTI block prior to TTI Lookup, and
*         resides AFTER the MAC2ME, Ingress VLAN translation, and protocol-based VLANs
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] descrPtr                 - pointer to frame data buffer Id
* @param[in,out] descrPtr                 - pointer to updated frame data buffer Id
*
* @param[out] internalTtiInfoPtr       - (pointer to) internal TTI info
*                                      RETURN:
*                                      COMMENTS:
*/
static GT_VOID snetLion2TrillEngine
(
    IN SKERNEL_DEVICE_OBJECT * devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC * descrPtr,
    OUT   INTERNAL_TTI_DESC_INFO_STC        *internalTtiInfoPtr
)
{
    DECLARE_FUNC_NAME(snetLion2TrillEngine);

    GT_U32  value;/*register value*/
    GT_U8   *trillHeaderPtr;/* pointer to the TRILL header in the packet */
    GT_U32  trillHeaderOffset;/*offset in bytes in the trill header */
    SKERNEL_FRAME_CHEETAH_DESCR_STC *innerFrameDescrPtr;/* inner frame descriptor pointer*/

    if(devObjPtr->supportTrill == 0)
    {
        /* the device not support TRILL */
        __LOG(("the device not support TRILL"));
        return;
    }

    if(devObjPtr->supportEArch == 0 || devObjPtr->unitEArchEnable.tti == 0)
    {
        /* the device not support EPorts */
        __LOG(("the device not support EPorts"));
        return;
    }

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

    smemRegGet(devObjPtr, SMEM_LION2_TTI_TRILL_GLOBAL_CONFIG0_REG(devObjPtr),&value);

    /*check Pkt EtherType == Global<Trill EtherType>*/
    if(descrPtr->etherTypeOrSsapDsap !=
        SMEM_U32_GET_FIELD(value,0,16))/*TRILL EtherType*/
    {
        __LOG(("Pkt EtherType != Global<Trill EtherType>"));
        return;
    }

    /* state that this is trill ethertype */
    __LOG(("state that this is trill ethertype"));
    descrPtr->isTrillEtherType = 1;

    /* parse the TRILL packet , and save info into the descriptor */
    __LOG(("parse the TRILL packet , and save info into the descriptor"));
    trillHeaderPtr = descrPtr->l3StartOffsetPtr;

    descrPtr->trillInfo.V = SMEM_U32_GET_FIELD(trillHeaderPtr[0],6,2);
            /* 2 reserved bits (after V , and before M)*/
    __LOG(("2 reserved bits (after V , and before M)"));
    descrPtr->trillInfo.M = SMEM_U32_GET_FIELD(trillHeaderPtr[0],3,1);
    descrPtr->trillInfo.opLength = (SMEM_U32_GET_FIELD(trillHeaderPtr[0],0,3) << 2) |
                                    SMEM_U32_GET_FIELD(trillHeaderPtr[1],6,2);

    descrPtr->trillInfo.hopCount = SMEM_U32_GET_FIELD(trillHeaderPtr[1],0,6);

    descrPtr->ttl = descrPtr->trillInfo.hopCount;

    descrPtr->trillInfo.eRbid = trillHeaderPtr[2] << 8 | trillHeaderPtr[3];
    descrPtr->trillInfo.iRbid = trillHeaderPtr[4] << 8 | trillHeaderPtr[5];

    trillHeaderOffset = 6;

    if(descrPtr->trillInfo.opLength)
    {
        descrPtr->trillInfo.CHbH = SMEM_U32_GET_FIELD(trillHeaderPtr[trillHeaderOffset],7,1);
        descrPtr->trillInfo.CItE = SMEM_U32_GET_FIELD(trillHeaderPtr[trillHeaderOffset],6,1);

        trillHeaderOffset += descrPtr->trillInfo.opLength * 4;

        if((descrPtr->trillInfo.opLength * 4) > SKERNEL_FRAME_CHEETAH_TRILL_OPTIONS_MAX_PARSE_BYTES_CNS)
        {
            descrPtr->ingressTunnelInfo.innerPacketL2FieldsAreValid = 0;
        }
        else
        {
            descrPtr->ingressTunnelInfo.innerPacketL2FieldsAreValid = 1;
        }
    }
    else
    {
        descrPtr->ingressTunnelInfo.innerPacketL2FieldsAreValid = 1;
    }

    if(descrPtr->ingressTunnelInfo.innerPacketL2FieldsAreValid)
    {
        descrPtr->ingressTunnelInfo.innerMacDaPtr = &descrPtr->l3StartOffsetPtr[trillHeaderOffset];

        /* Parsing for 'Inner frame' of TRILL. including L2,3,QOS assignment.
           need to be done regardless to tunnel termination (and before tunnel termination) */
        internalTtiInfoPtr->ttiPassengerOuterTagIsTag0_1_valid = 1;
        snetChtParsingTrillInnerFrame(devObjPtr, descrPtr,internalTtiInfoPtr);
        internalTtiInfoPtr->ttiPassengerOuterTagIsTag0_1_valid = 0;

        innerFrameDescrPtr = descrPtr->ingressTunnelInfo.innerFrameDescrPtr;
        if(innerFrameDescrPtr)
        {
            /* set innerTag0Exists,innerPacketTag0Vid,innerPacketTag0CfiDei,innerPacketTag0Up fields in descriptor */
            snetLion3PassengerOuterTagIsTag0_1(devObjPtr,descrPtr);
        }
    }

    /*check Src ePort Attribute<TRILL engine enabled>==1 */
    if(0 ==
        /*TRILL engine enable*/
        SMEM_LION3_TTI_DEFAULT_EPORT_ENTRY_FIELD_GET(devObjPtr, descrPtr,
            SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_TRILL_ENGINE_EN))
    {
        __LOG(("TRILL engine disabled"));
        return;
    }


    /* state that the TRILL engine triggered */
    __LOG(("state that the TRILL engine triggered"));
    descrPtr->trillEngineTriggered = 1;

    /* get the trill cpu code base */
    smemRegFldGet(devObjPtr,SMEM_LION2_TTI_TRILL_GLOBAL_CONFIG0_REG(devObjPtr),18,8,
        &descrPtr->trillInfo.trillCpuCodeBase);
    __LOG(("descrPtr->trillInfo.trillCpuCodeBase = %d", descrPtr->trillInfo.trillCpuCodeBase));

    /*TRILL Adjacency Check (using TCAM) */
    __LOG(("TRILL Adjacency Check (using TCAM)"));
    snetLion2TrillAdjacencyCheck(devObjPtr, descrPtr);

    /*TRILL Engine General Exception Checking*/
    __LOG(("TRILL Engine General Exception Checking"));
    snetLion2TrillGeneralExceptionChecking(devObjPtr, descrPtr);
}

/**
* @internal logTtiKeyType function
* @endinternal
*
* @brief   logs tti key type info
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] ttiKeyType               - tti key type
*/
static GT_VOID logTtiKeyType
(
    IN SKERNEL_DEVICE_OBJECT          *devObjPtr,
    IN SKERNEL_CHT3_TTI_KEY_TYPE_ENT   ttiKeyType
)
{
    DECLARE_FUNC_NAME(logTtiKeyType);

    __LOG(("ttiKeyType [%s]",
    ttiKeyType == SKERNEL_CHT3_TTI_KEY_IPV4_E             ? "SKERNEL_CHT3_TTI_KEY_IPV4_E   " :
    ttiKeyType == SKERNEL_CHT3_TTI_KEY_MPLS_E             ? "SKERNEL_CHT3_TTI_KEY_MPLS_E   " :
    ttiKeyType == SKERNEL_CHT3_TTI_KEY_ETH_E              ? "SKERNEL_CHT3_TTI_KEY_ETH_E    " :
    ttiKeyType == SKERNEL_XCAT_TTI_KEY_MIM_E              ? "SKERNEL_XCAT_TTI_KEY_MIM_E    " :
    ttiKeyType == SKERNEL_LION3_TTI_KEY_UDB_PKT_TYPE_0_E  ? "SIP5_PACKET_CLASSIFICATION_TYPE_IPV4_TCP_E  " :
    ttiKeyType == SKERNEL_LION3_TTI_KEY_UDB_PKT_TYPE_1_E  ? "SIP5_PACKET_CLASSIFICATION_TYPE_IPV4_UDP_E  " :
    ttiKeyType == SKERNEL_LION3_TTI_KEY_UDB_PKT_TYPE_2_E  ? "SIP5_PACKET_CLASSIFICATION_TYPE_MPLS_E      " :
    ttiKeyType == SKERNEL_LION3_TTI_KEY_UDB_PKT_TYPE_3_E  ? "SIP5_PACKET_CLASSIFICATION_TYPE_IPV4_FRAG_E " :
    ttiKeyType == SKERNEL_LION3_TTI_KEY_UDB_PKT_TYPE_4_E  ? "SIP5_PACKET_CLASSIFICATION_TYPE_IPV4_OTHER_E" :
    ttiKeyType == SKERNEL_LION3_TTI_KEY_UDB_PKT_TYPE_5_E  ? "SIP5_PACKET_CLASSIFICATION_TYPE_ETH_OTHER_E " :
    ttiKeyType == SKERNEL_LION3_TTI_KEY_UDB_PKT_TYPE_6_E  ? "SIP5_PACKET_CLASSIFICATION_TYPE_IPV6_E      " :
    ttiKeyType == SKERNEL_LION3_TTI_KEY_UDB_PKT_TYPE_7_E  ? "SIP5_PACKET_CLASSIFICATION_TYPE_IPV6_TCP_E  " :
    ttiKeyType == SKERNEL_LION3_TTI_KEY_UDB_PKT_TYPE_8_E  ? "SIP5_PACKET_CLASSIFICATION_TYPE_IPV6_UDP_E  " :
    ttiKeyType == SKERNEL_LION3_TTI_KEY_UDB_PKT_TYPE_9_E  ? "SIP5_PACKET_CLASSIFICATION_TYPE_UDE0_E      " :
    ttiKeyType == SKERNEL_LION3_TTI_KEY_UDB_PKT_TYPE_10_E ? "SIP5_PACKET_CLASSIFICATION_TYPE_UDE1_E      " :
    ttiKeyType == SKERNEL_LION3_TTI_KEY_UDB_PKT_TYPE_11_E ? "SIP5_PACKET_CLASSIFICATION_TYPE_UDE2_E      " :
    ttiKeyType == SKERNEL_LION3_TTI_KEY_UDB_PKT_TYPE_12_E ? "SIP5_PACKET_CLASSIFICATION_TYPE_UDE3_E      " :
    ttiKeyType == SKERNEL_LION3_TTI_KEY_UDB_PKT_TYPE_13_E ? "SIP5_PACKET_CLASSIFICATION_TYPE_UDE4_E      " :
    ttiKeyType == SKERNEL_LION3_TTI_KEY_UDB_PKT_TYPE_14_E ? "SIP5_PACKET_CLASSIFICATION_TYPE_UDE5_E      " :
    ttiKeyType == SKERNEL_LION3_TTI_KEY_UDB_PKT_TYPE_15_E ? "SIP5_PACKET_CLASSIFICATION_TYPE_UDE6_E      " :
    "unknown"));
}

/**
* @internal snetLion3TTIPv6ExceptionCheck function
* @endinternal
*
* @brief   IPv6 exception check in header , before apply the TTI action.
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to frame data buffer Id
* @param[in] internalTtiInfoPtr       - (pointer to) internal TTI info
*/
static GT_BOOL snetLion3TTIPv6ExceptionCheck
(
    IN SKERNEL_DEVICE_OBJECT             *devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC   *descrPtr,
    IN INTERNAL_TTI_DESC_INFO_STC        *internalTtiInfoPtr
)
{
    DECLARE_FUNC_NAME(snetLion3TTIPv6ExceptionCheck);

    GT_U32 fldVal;
    GT_U32 startBit = 0;     /* start bit in the register */
    GT_U32 cpuCode  = 0;     /* cpu code*/

    /* the order inside following if reflects exceptions checks priority */
    if(internalTtiInfoPtr->preTunnelTerminationInfo.ipHeaderError)
    {
        __LOG(("IPv6 TTI Header Exception Command \n"));
        startBit = 18;
        smemRegFldGet(devObjPtr,SMEM_LION3_TTI_LOOKUP_IP_EXCEPTION_CODES_0_REG(devObjPtr), 16, 8, &cpuCode);
    }
    else if(internalTtiInfoPtr->preTunnelTerminationInfo.ipTtiHeaderError)
    {
        __LOG(("IPv6 TTI SIP Address Exception Command \n"));
        startBit = 27;
        smemRegFldGet(devObjPtr,SMEM_LION3_TTI_LOOKUP_IP_EXCEPTION_CODES_0_REG(devObjPtr), 8, 8, &cpuCode);
    }
    else
    {
        __LOG(("no exception \n"));
        return GT_TRUE;/* no exception */
    }

    /* got exception */
    smemRegFldGet(devObjPtr,SMEM_LION3_TTI_LOOKUP_IP_EXCEPTION_COMMANDS_REG(devObjPtr), startBit, 3, &fldVal);

    snetChtIngressCommandAndCpuCodeResolution(devObjPtr,descrPtr,
                                              descrPtr->packetCmd,
                                              fldVal,
                                              descrPtr->cpuCode,
                                              cpuCode,
                                              SNET_CHEETAH_ENGINE_UNIT_TTI_E,
                                              GT_FALSE);

    if(fldVal <= SKERNEL_EXT_PKT_CMD_MIRROR_TO_CPU_E)
    {
        __LOG(("Although Exception detected, the Exception command [%d] allows further FORWARDING \n",
            fldVal));

        return GT_TRUE;
    }
    else
    {
        return GT_FALSE ;
    }

}


/**
* @internal snetLion3TTIPv6ExceptionCheckPart2 function
* @endinternal
*
* @brief   IPv6 exception in header - after TTI action
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to frame data buffer Id
* @param[in] internalTtiInfoPtr       - (pointer to) internal TTI info
*/
static void snetLion3TTIPv6ExceptionCheckPart2
(
    IN SKERNEL_DEVICE_OBJECT             *devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC   *descrPtr,
    IN INTERNAL_TTI_DESC_INFO_STC        *internalTtiInfoPtr
)
{
    DECLARE_FUNC_NAME(snetLion3TTIPv6ExceptionCheckPart2);

    GT_U32  startBit,cpuCode,fldVal;

    /* the FS describes only 'tunnel terminated' but apply also to 'transit' packets */
    if(descrPtr->tunnelTerminated == GT_FALSE &&
       descrPtr->ingressTunnelInfo.transitType == SNET_INGRESS_TUNNEL_TRANSIT_TYPE__NONE__E)
    {
        __LOG(("No extra exception checks for non tunnel terminated packets (and non-transit) \n"));

        return;
    }

    if(internalTtiInfoPtr->preTunnelTerminationInfo.ipHeaderError || internalTtiInfoPtr->preTunnelTerminationInfo.ipTtiHeaderError)
    {
        /* do nothing; higher exception already determine the exception code & packet command */
        return;
    }
    else if(internalTtiInfoPtr->preTunnelTerminationInfo.greHeaderError)
    {
        __LOG(("IPv6 GRE Extension Exception \n"));

        /*  The GRE header may optionally contain header extensions, per RFC 2890.
            If the IPv6 header <IP Next Header> = 47 (GRE), and the TTI Action <Tunnel Terminate> = 1
            (packet is to be tunnel-terminated), there is an optional check to verify that the 16 most-significant
            bits of the GRE header are all 0. The check ensures that the GRE <Version> is '0' and that the GRE
            header length is 4 bytes, (i.e., there are no GRE extension headers, for example, checksum, key,
            sequence fields).
            If this check is enabled, and the 16 most-significant bits of the GRE header are NOT all 0, the GRE
            Options exception command is applied.*/
        startBit = 15;
        smemRegFldGet(devObjPtr,SMEM_LION3_TTI_LOOKUP_IP_EXCEPTION_CODES_1_REG(devObjPtr), 8, 8, &cpuCode);
    }
    else if(internalTtiInfoPtr->preTunnelTerminationInfo.isIpV6EhHopByHop)
    {
        __LOG(("IPv6 Hop-by-Hop Extension Exception \n"));
        startBit = 21;
        smemRegFldGet(devObjPtr,SMEM_LION3_TTI_LOOKUP_IP_EXCEPTION_CODES_1_REG(devObjPtr), 0, 8, &cpuCode);
    }
    else if((!internalTtiInfoPtr->preTunnelTerminationInfo.isIpV6EhHopByHop) &&
        internalTtiInfoPtr->preTunnelTerminationInfo.isIpV6EhExists)
    {
        __LOG(("IPv6 NON Hop-by-Hop Extension Exception \n"));
        startBit = 24;
        smemRegFldGet(devObjPtr,SMEM_LION3_TTI_LOOKUP_IP_EXCEPTION_CODES_0_REG(devObjPtr), 24, 8, &cpuCode);
    }
    else
    {
        __LOG(("no exception \n"));
        return ;/* no exception */
    }

    /* got exception */
    smemRegFldGet(devObjPtr,SMEM_LION3_TTI_LOOKUP_IP_EXCEPTION_COMMANDS_REG(devObjPtr), startBit, 3, &fldVal);

    snetChtIngressCommandAndCpuCodeResolution(devObjPtr,descrPtr,
                                              descrPtr->packetCmd,
                                              fldVal,
                                              descrPtr->cpuCode,
                                              cpuCode,
                                              SNET_CHEETAH_ENGINE_UNIT_TTI_E,
                                              GT_FALSE);

    return;
}



/**
* @internal snetCht3TTIExceptionChecks_part2 function
* @endinternal
*
* @brief   exception check after apply TTI actions
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] descrPtr                 - pointer to frame data buffer Id
* @param[in,out] descrPtr                 - pointer to updated frame data buffer Id
*
* @param[out] internalTtiInfoPtr       - pointer to tti internal info
*                                      RETURN:
*                                      none
*                                      COMMENTS:
*/
static void snetCht3TTIExceptionChecks_part2
(
    IN    SKERNEL_DEVICE_OBJECT             *devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC   *descrPtr,
    IN INTERNAL_TTI_DESC_INFO_STC        *internalTtiInfoPtr
)
{
    DECLARE_FUNC_NAME(snetCht3TTIExceptionChecks);

    if(internalTtiInfoPtr->needToDoExceptionCheckPart2 == GT_TRUE)
    {
        if(internalTtiInfoPtr->preTunnelTerminationInfo.isIPv4)
        {
            __LOG(("look for extra Ipv4 Exception checks \n"));
            snetCht3TTIPv4ExceptionCheckPart2(devObjPtr, descrPtr,internalTtiInfoPtr);
        }
        else
        if(internalTtiInfoPtr->preTunnelTerminationInfo.isIp)
        { /* ipv6 packet */
            __LOG(("look for extra Ipv6 Exception checks \n"));
            snetLion3TTIPv6ExceptionCheckPart2(devObjPtr, descrPtr,internalTtiInfoPtr);
        }
    }

    return;
}

/**
* @internal snetCht3TTIExceptionChecks function
* @endinternal
*
* @brief   exception check before apply TTI actions
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] descrPtr                 - pointer to frame data buffer Id
* @param[in,out] descrPtr                 - pointer to updated frame data buffer Id
*
* @param[out] internalTtiInfoPtr       - pointer to tti internal info
*                                      RETURN:
*                                      GT_BOOL      - TRUE allow to apply action,  false - don't apply the action
*                                      COMMENTS:
*/
static GT_BOOL snetCht3TTIExceptionChecks
(
    IN    SKERNEL_DEVICE_OBJECT             *devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC   *descrPtr,
    IN INTERNAL_TTI_DESC_INFO_STC        *internalTtiInfoPtr
)
{
    DECLARE_FUNC_NAME(snetCht3TTIExceptionChecks);

    GT_BOOL  status  = GT_TRUE;
    GT_BOOL  doCheck = GT_FALSE;
    SKERNEL_CHT3_TTI_KEY_TYPE_ENT     ttiKeyType = internalTtiInfoPtr->ttiKeyType;

    if(internalTtiInfoPtr->preTunnelTerminationInfo.isIPv4)
    { /* ipv4 packet */
        if(SKERNEL_CHT3_TTI_KEY_IPV4_E == ttiKeyType)
        { /* ipv4 lookup */
            doCheck = GT_TRUE;
        }
        else if(devObjPtr->supportEArch )
        { /* sip5 packet type lookup */

            /* check for ipv4 lookups only */
            if( (ttiKeyType == (SKERNEL_LION3_TTI_KEY_UDB_PKT_TYPE_0_E + SIP5_PACKET_CLASSIFICATION_TYPE_IPV4_TCP_E)) ||
                (ttiKeyType == (SKERNEL_LION3_TTI_KEY_UDB_PKT_TYPE_0_E + SIP5_PACKET_CLASSIFICATION_TYPE_IPV4_UDP_E)) ||
                (ttiKeyType == (SKERNEL_LION3_TTI_KEY_UDB_PKT_TYPE_0_E + SIP5_PACKET_CLASSIFICATION_TYPE_IPV4_FRAG_E))||
                (ttiKeyType == (SKERNEL_LION3_TTI_KEY_UDB_PKT_TYPE_0_E + SIP5_PACKET_CLASSIFICATION_TYPE_IPV4_OTHER_E)))
            {
                doCheck = GT_TRUE;
            }
        }
        if(doCheck)
        {
            internalTtiInfoPtr->needToDoExceptionCheckPart2 = GT_TRUE;

            __LOG(("do Ipv4 Exception check"));
            /* T.T Ipv4 Exception check */
            status = snetCht3TTIPv4ExceptionCheck(devObjPtr, descrPtr,internalTtiInfoPtr);
        }
    }
    else
    if(internalTtiInfoPtr->preTunnelTerminationInfo.isIp)
    { /* ipv6 packet */

        if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
        { /* sip5 packet type lookup */

            /* check for ipv6 lookups only */
            if( (ttiKeyType != (SKERNEL_LION3_TTI_KEY_UDB_PKT_TYPE_0_E + SIP5_PACKET_CLASSIFICATION_TYPE_IPV6_E))     &&
                (ttiKeyType != (SKERNEL_LION3_TTI_KEY_UDB_PKT_TYPE_0_E + SIP5_PACKET_CLASSIFICATION_TYPE_IPV6_TCP_E)) &&
                (ttiKeyType != (SKERNEL_LION3_TTI_KEY_UDB_PKT_TYPE_0_E + SIP5_PACKET_CLASSIFICATION_TYPE_IPV6_UDP_E)))
            {
                return GT_TRUE;
            }

            __LOG(("do Ipv6 Exception check"));
            internalTtiInfoPtr->needToDoExceptionCheckPart2 = GT_TRUE;

            /* T.T Ipv6 Exception check */
            status = snetLion3TTIPv6ExceptionCheck(devObjPtr, descrPtr,internalTtiInfoPtr);
        }
    }
    else
    {
        /* mpls */
    }

    return status;
}


/**
* @internal snetTTSip5TcamCheckSegmentMode function
* @endinternal
*
* @brief   returns tcam segment mode
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] ttiKeyType               - tti key type
*/
static GT_U32 snetTTSip5TcamCheckSegmentMode
(
    IN SKERNEL_DEVICE_OBJECT          *devObjPtr,
    IN SKERNEL_CHT3_TTI_KEY_TYPE_ENT   ttiKeyType
)
{
    GT_U32    regAddr;
    GT_U32    fldVal;
    GT_U32    startBit;

    /* index in the array is ttiKeyType (type SKERNEL_CHT3_TTI_KEY_TYPE_ENT)
       value is bit index in SMEM_LION3_TTI_LOOKUP_KEY_SEGMENT_MODE_REG   */
    const GT_U32 lion3PacketTypeToSegmentModeBitIndexArr[] =
    {
        16,17,19,18,/* the 4 legacy keys*/
        0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15 /*the 16 packet type keys */
    };

    if(ttiKeyType > SKERNEL_LION3_TTI_KEY_UDB_PKT_TYPE_15_E)
    {
        skernelFatalError("snetTTSip5TcamCheckSegmentMode: invalid tti key type \n");
    }

    if(SMEM_CHT_IS_SIP5_10_GET(devObjPtr))
    {
        /* 10 types in register */
        if(lion3PacketTypeToSegmentModeBitIndexArr[ttiKeyType] > 10)
        {
            regAddr = SMEM_LION3_TTI_LOOKUP_KEY_SEGMENT_MODE_1_REG(devObjPtr);
        }
        else
        {
            regAddr = SMEM_LION3_TTI_LOOKUP_KEY_SEGMENT_MODE_0_REG(devObjPtr);
        }

        /* 10 types in register , 3 bits for each */
        startBit = 3 * (lion3PacketTypeToSegmentModeBitIndexArr[ttiKeyType] % 10);

        smemRegFldGet(devObjPtr, regAddr,
                      startBit, 3, &fldVal);

    }
    else
    {
        smemRegFldGet(devObjPtr, SMEM_LION3_TTI_LOOKUP_KEY_SEGMENT_MODE_0_REG(devObjPtr),
                      lion3PacketTypeToSegmentModeBitIndexArr[ttiKeyType], 1, &fldVal);
    }

    return fldVal;
}

#define __ACTION_OVERRIDE(_lookupId,_action_0,_action_new,field)  \
    if(_action_0->field != _action_new->field)                    \
    {                                                             \
        __LOG(("lookup[%d] [%s] - new action value[%d] overrides prev action [%d] \n", \
            _lookupId,#field,_action_new->field,_action_0->field));   \
    }                                                             \
    _action_0->field = _action_new->field

#define __ACTION_OVERRIDE_STC(_lookupId,_action_0,_action_new,field)  \
    __LOG(("lookup[%d] [%s] - new action overrides prev action \n", \
        _lookupId,#field));   \
    _action_0->field = _action_new->field

#define EARCH_ACTION_OVERRIDE(field)  \
    __ACTION_OVERRIDE(lookupId,earchAction0Ptr,earchAction1Ptr,field)

#define ACTION_OVERRIDE(field)  \
    __ACTION_OVERRIDE(lookupId,xcatAction0Ptr,xcatAction1Ptr,field)

#define LION_ACTION_OVERRIDE(field)  \
    __ACTION_OVERRIDE(lookupId,lionAction0Ptr,lionAction1Ptr,field)

#define ACTION_OVERRIDE_STC(field)  \
    __ACTION_OVERRIDE_STC(lookupId,xcatAction0Ptr,xcatAction1Ptr,field)

/**
* @internal snetLion3TTCombineTwoActions function
* @endinternal
*
* @brief   Sip5 devices: Get the combined action (for dual tti lookup)
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to frame descriptor.
* @param[in] lookupId                 - the lookupId that refer to 'xcatAction1Ptr'
* @param[in,out] internalTtiInfoPtr       - (pointer to) internal TTI info
* @param[in] xcatAction1Ptr           - pointer to next TT action
* @param[in,out] xcatAction0Ptr           - pointer to first TT action
* @param[in,out] internalTtiInfoPtr       - (pointer to) internal TTI info
* @param[in,out] xcatAction0Ptr           - pointer to combined TT action
*                                      RETURN:
*                                      COMMENTS:
*/
static GT_VOID snetLion3TTCombineTwoActions
(
    IN    SKERNEL_DEVICE_OBJECT           *devObjPtr,
    IN    SKERNEL_FRAME_CHEETAH_DESCR_STC *descrPtr,
    IN    GT_U32                          lookupId,
    INOUT INTERNAL_TTI_DESC_INFO_STC      *internalTtiInfoPtr,
    IN    SNET_XCAT_TT_ACTION_STC         *xcatAction1Ptr,
    INOUT SNET_XCAT_TT_ACTION_STC         *xcatAction0Ptr
)
{
    DECLARE_FUNC_NAME(snetLion3TTCombineTwoActions);

    SNET_LION_TT_ACTION_STC   *lionAction0Ptr; /* pointer to lion TT action 0 */
    SNET_LION_TT_ACTION_STC   *lionAction1Ptr; /* pointer to lion TT action 1 */
    SNET_E_ARCH_TT_ACTION_STC *earchAction0Ptr;/* pointer to eArch TT action 0 */
    SNET_E_ARCH_TT_ACTION_STC *earchAction1Ptr;/* pointer to eArch TT action 1 */

    earchAction0Ptr = (SNET_E_ARCH_TT_ACTION_STC*)(void*)xcatAction0Ptr;
    lionAction0Ptr  = &earchAction0Ptr->baseActionData;
    earchAction1Ptr = (SNET_E_ARCH_TT_ACTION_STC*)(void*)xcatAction1Ptr;
    lionAction1Ptr  = &earchAction1Ptr->baseActionData;

    if(xcatAction1Ptr->tunnelTerminateEn ||
       earchAction1Ptr->passengerParsingOfNonMplsTransitTunnelEnable)
    {
        /* Tunnel Header Length */
        EARCH_ACTION_OVERRIDE(ttHeaderLength);

        /* Tunnel Passenger Type */
        ACTION_OVERRIDE(passengerPacketType);
    }

    /* Tunnel Terminate */
    if(xcatAction1Ptr->tunnelTerminateEn)
    {
        ACTION_OVERRIDE(tunnelTerminateEn);
        /* fix JIRA : CPSS-11121 */
        EARCH_ACTION_OVERRIDE(tunnelHeaderLengthAnchorType);
    }

    /* Passenger Parsing of MPLS Transit Tunnels Mode */
    if(earchAction1Ptr->passengerParsingOfTransitMplsTunnelEnable)
    {
        EARCH_ACTION_OVERRIDE(passengerParsingOfTransitMplsTunnelEnable);
    }

    /* Passenger Parsing of Non-MPLS Transit Tunnels Enable */
    if(earchAction1Ptr->passengerParsingOfNonMplsTransitTunnelEnable)
    {
        EARCH_ACTION_OVERRIDE(passengerParsingOfNonMplsTransitTunnelEnable);
    }

    __LOG_PARAM(xcatAction0Ptr->mplsCmd);
    __LOG_PARAM(xcatAction1Ptr->mplsCmd);

    /* Mpls Command */
    /* POP1/POP1_SWAP and SWAP combinations */
    if (
        ((xcatAction0Ptr->mplsCmd == SKERNEL_XCAT_TTI_MPLS_POP_1_E ||
          xcatAction0Ptr->mplsCmd == SKERNEL_XCAT_TTI_MPLS_POP_SWAP_E) &&
         xcatAction1Ptr->mplsCmd == SKERNEL_XCAT_TTI_MPLS_SWAP_E) ||
        (xcatAction0Ptr->mplsCmd == SKERNEL_XCAT_TTI_MPLS_SWAP_E &&
         (xcatAction1Ptr->mplsCmd == SKERNEL_XCAT_TTI_MPLS_POP_1_E ||
          xcatAction1Ptr->mplsCmd == SKERNEL_XCAT_TTI_MPLS_POP_SWAP_E))
        )
    {
        __LOG(("resolution of POP_1 and SWAP is POP_SWAP \n"));
        xcatAction0Ptr->mplsCmd = SKERNEL_XCAT_TTI_MPLS_POP_SWAP_E;
    }
    /* POP1 and POP1_SWAP combinations */
    else
    if (
        (xcatAction0Ptr->mplsCmd == SKERNEL_XCAT_TTI_MPLS_POP_1_E &&
         xcatAction1Ptr->mplsCmd == SKERNEL_XCAT_TTI_MPLS_POP_SWAP_E)
        )
    {
        __LOG(("resolution of POP_1 and POP_SWAP is POP2_SWAP \n"));
        xcatAction0Ptr->mplsCmd = SKERNEL_XCAT_TTI_MPLS_POP2_SWAP_E;
    }
    /* POP1 and POP2_SWAP combinations */
    else
    if (
        (xcatAction0Ptr->mplsCmd == SKERNEL_XCAT_TTI_MPLS_POP_1_E &&
         xcatAction1Ptr->mplsCmd == SKERNEL_XCAT_TTI_MPLS_POP2_SWAP_E) ||
        (xcatAction0Ptr->mplsCmd == SKERNEL_XCAT_TTI_MPLS_POP2_SWAP_E &&
         xcatAction1Ptr->mplsCmd == SKERNEL_XCAT_TTI_MPLS_POP_1_E)
        )
    {
        __LOG(("resolution of POP_1 and POP2_SWAP is POP3_SWAP \n"));
        xcatAction0Ptr->mplsCmd = SKERNEL_XCAT_TTI_MPLS_POP3_SWAP_E;
    }
    /* POP1 and POP3_SWAP combinations */
    else
    if (
        (xcatAction0Ptr->mplsCmd == SKERNEL_XCAT_TTI_MPLS_POP_1_E &&
         xcatAction1Ptr->mplsCmd == SKERNEL_XCAT_TTI_MPLS_POP3_SWAP_E) ||
        (xcatAction0Ptr->mplsCmd == SKERNEL_XCAT_TTI_MPLS_POP3_SWAP_E &&
         xcatAction1Ptr->mplsCmd == SKERNEL_XCAT_TTI_MPLS_POP_1_E)
        )
    {
        __LOG(("resolution of POP_1 and POP3_SWAP is POP3_SWAP \n"));
        xcatAction0Ptr->mplsCmd = SKERNEL_XCAT_TTI_MPLS_POP3_SWAP_E;
    }
    /* POP2/POP2_SWAP and SWAP combinations */
    else
    if (
        ((xcatAction0Ptr->mplsCmd == SKERNEL_XCAT_TTI_MPLS_POP_2_E ||
          xcatAction0Ptr->mplsCmd == SKERNEL_XCAT_TTI_MPLS_POP2_SWAP_E) &&
         xcatAction1Ptr->mplsCmd == SKERNEL_XCAT_TTI_MPLS_SWAP_E) ||
        (xcatAction0Ptr->mplsCmd == SKERNEL_XCAT_TTI_MPLS_SWAP_E &&
         (xcatAction1Ptr->mplsCmd == SKERNEL_XCAT_TTI_MPLS_POP_2_E ||
          xcatAction1Ptr->mplsCmd == SKERNEL_XCAT_TTI_MPLS_POP2_SWAP_E))
        )
    {
        __LOG(("resolution of POP_2 and SWAP is POP2_SWAP \n"));
        xcatAction0Ptr->mplsCmd = SKERNEL_XCAT_TTI_MPLS_POP2_SWAP_E;
    }
    /* POP2 and POP1_SWAP/POP2_SWAP/POP3_SWAP combinations */
    else
    if (
        (xcatAction0Ptr->mplsCmd == SKERNEL_XCAT_TTI_MPLS_POP_2_E &&
         (xcatAction1Ptr->mplsCmd == SKERNEL_XCAT_TTI_MPLS_POP_SWAP_E  ||
          xcatAction1Ptr->mplsCmd == SKERNEL_XCAT_TTI_MPLS_POP2_SWAP_E ||
          xcatAction1Ptr->mplsCmd == SKERNEL_XCAT_TTI_MPLS_POP3_SWAP_E)) ||
        ((xcatAction0Ptr->mplsCmd == SKERNEL_XCAT_TTI_MPLS_POP_SWAP_E ||
          xcatAction0Ptr->mplsCmd == SKERNEL_XCAT_TTI_MPLS_POP2_SWAP_E ||
          xcatAction0Ptr->mplsCmd == SKERNEL_XCAT_TTI_MPLS_POP3_SWAP_E) &&
          xcatAction1Ptr->mplsCmd == SKERNEL_XCAT_TTI_MPLS_POP_2_E)
        )
    {
        __LOG(("resolution of POP_2 and POP_SWAP/POP2_SWAP/POP3_SWAP is POP3_SWAP \n"));
        xcatAction0Ptr->mplsCmd = SKERNEL_XCAT_TTI_MPLS_POP3_SWAP_E;
    }
    /* POP3 and SWAP/POP1_SWAP/POP2_SWAP/POP3_SWAP combinations */
    else
    if ((xcatAction0Ptr->mplsCmd == SKERNEL_XCAT_TTI_MPLS_POP_3_E &&
        (xcatAction1Ptr->mplsCmd == SKERNEL_XCAT_TTI_MPLS_SWAP_E ||
         xcatAction1Ptr->mplsCmd == SKERNEL_XCAT_TTI_MPLS_POP_SWAP_E  ||
         xcatAction1Ptr->mplsCmd == SKERNEL_XCAT_TTI_MPLS_POP2_SWAP_E ||
         xcatAction1Ptr->mplsCmd == SKERNEL_XCAT_TTI_MPLS_POP3_SWAP_E)) ||
        ((xcatAction0Ptr->mplsCmd == SKERNEL_XCAT_TTI_MPLS_SWAP_E ||
          xcatAction0Ptr->mplsCmd == SKERNEL_XCAT_TTI_MPLS_POP_SWAP_E ||
          xcatAction0Ptr->mplsCmd == SKERNEL_XCAT_TTI_MPLS_POP2_SWAP_E ||
          xcatAction0Ptr->mplsCmd == SKERNEL_XCAT_TTI_MPLS_POP3_SWAP_E) &&
          xcatAction1Ptr->mplsCmd == SKERNEL_XCAT_TTI_MPLS_POP_3_E))
    {
        __LOG(("resolution of POP_3 and SWAP/POP_SWAP/POP2_SWAP/POP3_SWAP is POP3_SWAP\n"));
        xcatAction0Ptr->mplsCmd = SKERNEL_XCAT_TTI_MPLS_POP3_SWAP_E;
    }
    /* POP1_SWAP, POP2_SWAP, POP3_SWAP combinations */
    else
    if(
       (xcatAction0Ptr->mplsCmd == SKERNEL_XCAT_TTI_MPLS_POP_SWAP_E  ||
        xcatAction0Ptr->mplsCmd == SKERNEL_XCAT_TTI_MPLS_POP2_SWAP_E ||
        xcatAction0Ptr->mplsCmd == SKERNEL_XCAT_TTI_MPLS_POP3_SWAP_E)   &&

       (xcatAction1Ptr->mplsCmd == SKERNEL_XCAT_TTI_MPLS_POP_SWAP_E  ||
        xcatAction1Ptr->mplsCmd == SKERNEL_XCAT_TTI_MPLS_POP2_SWAP_E ||
        xcatAction1Ptr->mplsCmd == SKERNEL_XCAT_TTI_MPLS_POP3_SWAP_E)
      )
    {
        GT_U32  numMPLSTagsToPopSwap_action0 = 0;
        GT_U32  numMPLSTagsToPopSwap_action1 = 0;

        switch(xcatAction0Ptr->mplsCmd)
        {
            case SKERNEL_XCAT_TTI_MPLS_POP_SWAP_E:  numMPLSTagsToPopSwap_action0 = 1; break;
            case SKERNEL_XCAT_TTI_MPLS_POP2_SWAP_E: numMPLSTagsToPopSwap_action0 = 2; break;
            case SKERNEL_XCAT_TTI_MPLS_POP3_SWAP_E: numMPLSTagsToPopSwap_action0 = 3; break;
            default:/*should not happen*/numMPLSTagsToPopSwap_action0 = 0; break;
        }

        switch(xcatAction1Ptr->mplsCmd)
        {
            case SKERNEL_XCAT_TTI_MPLS_POP_SWAP_E:  numMPLSTagsToPopSwap_action1 = 1; break;
            case SKERNEL_XCAT_TTI_MPLS_POP2_SWAP_E: numMPLSTagsToPopSwap_action1 = 2; break;
            case SKERNEL_XCAT_TTI_MPLS_POP3_SWAP_E: numMPLSTagsToPopSwap_action1 = 3; break;
            default:/*should not happen*/numMPLSTagsToPopSwap_action1 = 0; break;
        }

        __LOG_PARAM(numMPLSTagsToPopSwap_action0);
        __LOG_PARAM(numMPLSTagsToPopSwap_action1);

        if((numMPLSTagsToPopSwap_action0 + numMPLSTagsToPopSwap_action1) == 2)
        {
            __LOG(("resolution of POP%d_SWAP and POP%d_SWAP is POP2_SWAP \n",
                numMPLSTagsToPopSwap_action0 ,numMPLSTagsToPopSwap_action1));
            xcatAction0Ptr->mplsCmd = SKERNEL_XCAT_TTI_MPLS_POP2_SWAP_E;
        }
        else
        {
            __LOG(("resolution of POP%d_SWAP and POP%d_SWAP ([%d]+[%d] >= 3) is POP3_SWAP \n",
            numMPLSTagsToPopSwap_action0 ,numMPLSTagsToPopSwap_action1,
            numMPLSTagsToPopSwap_action0 ,numMPLSTagsToPopSwap_action1));
            xcatAction0Ptr->mplsCmd = SKERNEL_XCAT_TTI_MPLS_POP3_SWAP_E;
        }
    }
    /* POP1, POP2, POP3 combinations */
    else
    if(
       (xcatAction0Ptr->mplsCmd == SKERNEL_XCAT_TTI_MPLS_POP_1_E ||
        xcatAction0Ptr->mplsCmd == SKERNEL_XCAT_TTI_MPLS_POP_2_E ||
        xcatAction0Ptr->mplsCmd == SKERNEL_XCAT_TTI_MPLS_POP_3_E )   &&

       (xcatAction1Ptr->mplsCmd == SKERNEL_XCAT_TTI_MPLS_POP_1_E ||
        xcatAction1Ptr->mplsCmd == SKERNEL_XCAT_TTI_MPLS_POP_2_E ||
        xcatAction1Ptr->mplsCmd == SKERNEL_XCAT_TTI_MPLS_POP_3_E )
      )
    {
        GT_U32  numMPLSTagsToPop_action0 = 0;
        GT_U32  numMPLSTagsToPop_action1 = 0;

        switch(xcatAction0Ptr->mplsCmd)
        {
            case SKERNEL_XCAT_TTI_MPLS_POP_1_E: numMPLSTagsToPop_action0 = 1; break;
            case SKERNEL_XCAT_TTI_MPLS_POP_2_E: numMPLSTagsToPop_action0 = 2; break;
            case SKERNEL_XCAT_TTI_MPLS_POP_3_E: numMPLSTagsToPop_action0 = 3; break;
            default:/*should not happen*/numMPLSTagsToPop_action0 = 0; break;
        }

        switch(xcatAction1Ptr->mplsCmd)
        {
            case SKERNEL_XCAT_TTI_MPLS_POP_1_E: numMPLSTagsToPop_action1 = 1; break;
            case SKERNEL_XCAT_TTI_MPLS_POP_2_E: numMPLSTagsToPop_action1 = 2; break;
            case SKERNEL_XCAT_TTI_MPLS_POP_3_E: numMPLSTagsToPop_action1 = 3; break;
            default:/*should not happen*/numMPLSTagsToPop_action1 = 0; break;
        }

        __LOG_PARAM(numMPLSTagsToPop_action0);
        __LOG_PARAM(numMPLSTagsToPop_action1);

        if((numMPLSTagsToPop_action0 + numMPLSTagsToPop_action1)  == 2)
        {
            __LOG(("resolution of POP_%d and POP_%d is POP_2 \n",
                numMPLSTagsToPop_action0 ,numMPLSTagsToPop_action1));
            xcatAction0Ptr->mplsCmd = SKERNEL_XCAT_TTI_MPLS_POP_2_E;
        }
        else
        {
            __LOG(("resolution of POP_%d and POP_%d ([%d]+[%d] >=3) is POP_3 \n",
            numMPLSTagsToPop_action0 ,numMPLSTagsToPop_action1,
            numMPLSTagsToPop_action0 ,numMPLSTagsToPop_action1));
            xcatAction0Ptr->mplsCmd = SKERNEL_XCAT_TTI_MPLS_POP_3_E;
        }

    }
    /* NOP combinations */
    else
    if (xcatAction0Ptr->mplsCmd == SKERNEL_XCAT_TTI_MPLS_NOP_E &&
        xcatAction1Ptr->mplsCmd != SKERNEL_XCAT_TTI_MPLS_NOP_E)
    {
        /* No Change in xcatAction0Ptr->mplsCmd if the xcatAction1Ptr->mplsCmd is NOP */
        __LOG(("resolution of NOP in Action0 and valid action [%u] in Action1 is the valid action [%u] from Action1\n",
               xcatAction1Ptr->mplsCmd, xcatAction1Ptr->mplsCmd));
        xcatAction0Ptr->mplsCmd = xcatAction1Ptr->mplsCmd;
    }
    else
    if(xcatAction0Ptr->mplsCmd != SKERNEL_XCAT_TTI_MPLS_NOP_E &&
       xcatAction1Ptr->mplsCmd != SKERNEL_XCAT_TTI_MPLS_NOP_E)
    {
        __LOG(("resolution of MPLSCmd not 'combined' with second action \n"));
    }

    /* Enable Dec TTL */
    if(xcatAction1Ptr->ttlDecEn)
    {
        ACTION_OVERRIDE(ttlDecEn);
    }

    /* Packet command */
    /* the field will be applied independently by each eArch action */

    /* -------------------------------- */
    if(xcatAction0Ptr->redirectCmd == PCL_TTI_ACTION_REDIRECT_CMD_OUT_IF_E  &&
       xcatAction1Ptr->redirectCmd != PCL_TTI_ACTION_REDIRECT_CMD_OUT_IF_E)
    {
        __LOG(("current TTI action NOT enabled the same <Redirect Command> as first TTI action,"
               " so current TTI action will not override TTI0")); /* do nothing */
    }
    else if(xcatAction1Ptr->redirectCmd == PCL_TTI_ACTION_REDIRECT_CMD_OUT_IF_E)
    { /* Action1 will override/combine with action 0 */
        /* Target Egress Interface */
        ACTION_OVERRIDE_STC(tunnelIf);
        /* Arp Pointer */
        ACTION_OVERRIDE(arpPtr);
        /* Tunnel Start */
        ACTION_OVERRIDE(tunnelStart);
        /* Tunnel Start Pointer */
        ACTION_OVERRIDE(tunnelStartPtr);
        /* Tunnel Start Passenger Type */
        ACTION_OVERRIDE(tunnelStartPassengerType);
        /* VNTL2 echo */
        ACTION_OVERRIDE(vntL2Echo);
    }

    /* Policy Based Routing Pointer */
     if((xcatAction0Ptr->redirectCmd == PCL_TTI_ACTION_REDIRECT_CMD_PBR_OR_VPLS_E ||
         xcatAction0Ptr->redirectCmd == PCL_TTI_ACTION_REDIRECT_CMD_LTT_ROUTER_E) &&
           (xcatAction1Ptr->redirectCmd != PCL_TTI_ACTION_REDIRECT_CMD_PBR_OR_VPLS_E &&
            xcatAction1Ptr->redirectCmd != PCL_TTI_ACTION_REDIRECT_CMD_LTT_ROUTER_E))
    {
        __LOG(("current TTI action NOT enabled the same <Redirect Command> as first TTI action,"
               " so current TTI action will not override TTI0")); /* do nothing */
    }
    else if(xcatAction1Ptr->redirectCmd == PCL_TTI_ACTION_REDIRECT_CMD_PBR_OR_VPLS_E ||
            xcatAction1Ptr->redirectCmd == PCL_TTI_ACTION_REDIRECT_CMD_LTT_ROUTER_E)
    {
        ACTION_OVERRIDE(routerLTTIndex);
    }

    /* VRF-ID */
    if(xcatAction0Ptr->redirectCmd == PCL_TTI_ACTION_REDIRECT_CMD_VIRT_ROUTER_E &&
       xcatAction1Ptr->redirectCmd != PCL_TTI_ACTION_REDIRECT_CMD_VIRT_ROUTER_E )
    {
        __LOG(("current TTI action NOT enabled the same <Redirect Command> as first TTI action,"
               " so current TTI action will not override TTI0")); /* do nothing */
    }
    else if(xcatAction1Ptr->redirectCmd == PCL_TTI_ACTION_REDIRECT_CMD_VIRT_ROUTER_E)
    {
        ACTION_OVERRIDE(vrfId);
    }

    /* Redirect Command */
    if(!xcatAction0Ptr->redirectCmd && xcatAction1Ptr->redirectCmd)
    {
        ACTION_OVERRIDE(redirectCmd);
    }

    /* -------------------------------- */
    if(earchAction1Ptr->assignSrcInterfaceEnable == 2)/* sip7 only */
    {
        EARCH_ACTION_OVERRIDE(assignSrcInterfaceEnable);
        EARCH_ACTION_OVERRIDE(sourceVPort);
    }
    else
    if(earchAction1Ptr->assignSrcInterfaceEnable == 1)
    {
        /* Source ePort Assignment Enable */
        EARCH_ACTION_OVERRIDE(assignSrcInterfaceEnable);
        /* Source ePort */
        EARCH_ACTION_OVERRIDE(sourceEPort);
    }

    /* Source ID Assignment Enable */
    /* the field will be applied independently by each eArch action */
    /* Source ID */
    /* the field will be applied independently by each eArch action */

    if(devObjPtr->errata.incorrectResolutionOfTtiActionFields)
    {
       /* eVlan command */
       ACTION_OVERRIDE(vid0Cmd);
       /* eVlan */
       ACTION_OVERRIDE(vid0);
       /* VID1 command */
       ACTION_OVERRIDE(vid1Cmd);
       /* VID1 */
       ACTION_OVERRIDE(vid1);
       /* UP1 Command */
       ACTION_OVERRIDE(up1Cmd);
       /* UP1 */
       ACTION_OVERRIDE(up1);
    }
    else
    {
        /* -------------------------------- */
        if(xcatAction1Ptr->vid0Cmd)
        {
           /* eVlan command */
           ACTION_OVERRIDE(vid0Cmd);
           /* eVlan */
           ACTION_OVERRIDE(vid0);
        }

        if(xcatAction1Ptr->vid1Cmd ||
          (xcatAction1Ptr->vid1Cmd == xcatAction0Ptr->vid1Cmd))/* action1 and action0 with the 'modify untagged'  -> give action 1 priority */

        {
           /* VID1 command */
           ACTION_OVERRIDE(vid1Cmd);
           /* VID1 */
           ACTION_OVERRIDE(vid1);
        }

        /* UP1 Command */
        ACTION_OVERRIDE(up1Cmd);
        /* UP1 */
        ACTION_OVERRIDE(up1);
    }

    /* -------------------------------- */
    /* eVlan Precedence */
    if( xcatAction1Ptr->vidPrecedence == 1 ) /* HARD */
    {
        ACTION_OVERRIDE(vidPrecedence);
    }

    /* Enable Nested Vlan */
    if(xcatAction1Ptr->nestedVidEn)
    {
        ACTION_OVERRIDE(nestedVidEn);
    }

    /* -------------------------------- */
    if(xcatAction1Ptr->bindToPolicerMeter ||xcatAction1Ptr->bindToPolicerCounter)
    {
        /* Policer Index */
        ACTION_OVERRIDE(policerIndex);
    }

    /* Bind to Policer Meter */
    if(xcatAction1Ptr->bindToPolicerMeter)
    {
        ACTION_OVERRIDE(bindToPolicerMeter);
    }

    /* Bind to Policer Counter */
    if(xcatAction1Ptr->bindToPolicerCounter)
    {
        ACTION_OVERRIDE(bindToPolicerCounter);
    }

    /* -------------------------------- */
    /* Trust Qos Mapping Table Index */

    if(earchAction1Ptr->trustQosMappingTableIndex)
    {
        EARCH_ACTION_OVERRIDE(trustQosMappingTableIndex);
    }

    /* L2 Trust Table Selection Mode */
    if(earchAction1Ptr->up2QosProfileMappingMode)
    {
        EARCH_ACTION_OVERRIDE(up2QosProfileMappingMode);
    }

    if(xcatAction0Ptr->qoSPrecedence == 0 || /* SOFT */
        devObjPtr->errata.ttiActionMayOverrideHardQosPrecedence)
    {
        if (xcatAction0Ptr->qoSPrecedence == 1 )/* HARD */
        {
            __LOG(("WARNING : tti action may override 'HARD' qos profile precedence "));
        }

        ACTION_OVERRIDE(qoSPrecedence);
        /* -------------------------------- */
        /* Modify UP */
        if(xcatAction1Ptr->modifyUp != 0) /*0-keep previous, 1-modify , 2-no-modify */
        {
            /* up0 */
            ACTION_OVERRIDE(up0);
            ACTION_OVERRIDE(modifyUp);
        }
        /* Modify DSCP */
        /*0-keep previous, 1-modify , 2-no-modify */
        if(xcatAction1Ptr->modifyDscp)
        {
            ACTION_OVERRIDE(modifyDscp);
        }

        /* Keep Previous QoS */
        /* -------------------------------- */
        /* trustUp, trustDscp, trustExp, lLspQosProfileEnable, qoSProfile
           valid only when keepPreviousQoS == FALSE*/
        if(xcatAction1Ptr->keepPreviousQoS == GT_FALSE)
        {
            if(xcatAction0Ptr->keepPreviousQoS == GT_TRUE)
            {
                /*do not allow action 0 to set values when using macro TTI1_MAY_OVERRIDE_TTI0 */
                xcatAction0Ptr->trustExp = 0;
                xcatAction0Ptr->trustUp = 0;
                xcatAction0Ptr->trustDscp = 0;
                earchAction0Ptr->lLspQosProfileEnable = 0;
            }
            /*action 1 sets all next values */
            /* L-LSP Trust Mode -> must come before <trustExp> */
            if(xcatAction1Ptr->trustExp == 0)
            {
                /*lLspQosProfileEnable valid only when keepPreviousQoS == FALSE && trustExp == FALSE */
                if(earchAction1Ptr->lLspQosProfileEnable)
                {
                    EARCH_ACTION_OVERRIDE(lLspQosProfileEnable);
                }
            }
            /* Trust Up */
            if(xcatAction1Ptr->trustUp)
            {
                ACTION_OVERRIDE(trustUp);
            }
            /* Trust Dscp */
            if(xcatAction1Ptr->trustDscp)
            {
                ACTION_OVERRIDE(trustDscp);
            }
            /* Trust Exp */
            if(xcatAction1Ptr->trustExp)
            {
                ACTION_OVERRIDE(trustExp);
            }
            /* Qos Profile */
            ACTION_OVERRIDE(qoSProfile);

            xcatAction0Ptr->keepPreviousQoS = GT_FALSE;
        }
    }

    /* -------------------------------- */
    /* -------------------------------- */
    /* remap dscp */
    if(xcatAction1Ptr->dscpToDscpRemapingEn)
    {
        ACTION_OVERRIDE(dscpToDscpRemapingEn);
    }

    if(xcatAction1Ptr->policy0LookupMode ||
       xcatAction1Ptr->policy1LookupMode ||
       xcatAction1Ptr->policy2LookupMode)
    {
        /* NOTE: all 3 are updated if one of them is 'set' pcl0,pcl1,pcl2 override config index */
        /* this is UNLIKe the description in the FS , but this is according to HW behavior
           (see JIRA : CPSS-9880 : [Falcon] [TTI] [PCL] Can't override PCL Configuration Index of IPCL0 and IPCL1 at the same time)*/
        __LOG(("PCL0 Lookup Mode , PCL1 Lookup Mode , PCL2Lookup Mode - TTI1 overrides TTI0 \n"));
        ACTION_OVERRIDE(policy0LookupMode);
        ACTION_OVERRIDE(policy1LookupMode);
        ACTION_OVERRIDE(policy2LookupMode);

        /* also the index comes from the action that set one/more of the overrides */
        ACTION_OVERRIDE(ipclProfileIndex);
    }


    /* -------------------------------- */
    /* IPCL Stop */
    if(xcatAction1Ptr->actionStop)
    {
        ACTION_OVERRIDE(actionStop);
    }

    /* Mirror to Ingress Analyzer */
    if(earchAction1Ptr->analyzerIndex > earchAction0Ptr->analyzerIndex)
    {
        EARCH_ACTION_OVERRIDE(analyzerIndex);
    }

    /* CPU/DROP code */
    /* the field will be applied independently by each eArch action */

    /* bind to cnc counter */
    /* the field will be applied independently by each eArch action */

    /* cnc counter index*/
    /* the field will be applied independently by each eArch action */

    /* Bypass Bridge */
    if(xcatAction1Ptr->bypassBridge)
    {
        ACTION_OVERRIDE(bypassBridge);
    }

    /* Bypass Ingress Pipe */
    if(xcatAction1Ptr->bypassIngressPipe)
    {
        ACTION_OVERRIDE(bypassIngressPipe);
    }

    /* Hash Mask Index */
    if(lionAction1Ptr->hashMaskIndex > lionAction0Ptr->hashMaskIndex)
    {
        LION_ACTION_OVERRIDE(hashMaskIndex);
    }

    /* Modify Mac Da */
    if(lionAction1Ptr->modifyMacDa)
    {
        LION_ACTION_OVERRIDE(modifyMacDa);
    }

    /* Flow Id */
    if(earchAction1Ptr->flowId)
    {
        EARCH_ACTION_OVERRIDE(flowId);
    }

    if(xcatAction1Ptr->oamInfo.oamProcessEnable)
    {
        /* oam Process Enable */
        ACTION_OVERRIDE(oamInfo.oamProcessEnable);
        /* oam profile */
        ACTION_OVERRIDE(oamInfo.oamProfile);
    }

    /* channelTypeToOpcodeMappingEn */
    if(xcatAction1Ptr->oamInfo.channelTypeToOpcodeMappingEn)
    {
        ACTION_OVERRIDE(oamInfo.channelTypeToOpcodeMappingEn);
    }

    /* rxEnableProtectionSwitching */
    if(earchAction1Ptr->rxEnableProtectionSwitching)
    {
        EARCH_ACTION_OVERRIDE(rxEnableProtectionSwitching);
    }

    /* rxIsProtectionPath */
    if(earchAction1Ptr->rxIsProtectionPath)
    {
        EARCH_ACTION_OVERRIDE(rxIsProtectionPath);
    }

    if(xcatAction1Ptr->oamInfo.ptpTimeStampEnable)
    {
        /* ptp time stamp enable */
        ACTION_OVERRIDE(oamInfo.ptpTimeStampEnable);

        /* oam ptp offset index*/
        ACTION_OVERRIDE(oamInfo.ptpTimeStampOffsetIndex);
    }

    /* PTP */
    if(earchAction1Ptr->isPtpPacket)
    {
        EARCH_ACTION_OVERRIDE(isPtpPacket);
        EARCH_ACTION_OVERRIDE(ptpTriggerType);
        EARCH_ACTION_OVERRIDE(ptpOffset);
    }

    if(!earchAction0Ptr->isPtpPacket)
    {
        if(xcatAction1Ptr->oamInfo.ptpTimeStampEnable)
        {
            ACTION_OVERRIDE(oamInfo.ptpTimeStampEnable);
            ACTION_OVERRIDE(oamInfo.ptpTimeStampOffsetIndex);
        }
    }

    /* OAM */
    /*
    if(earchAction1Ptr->oamProcessingEnable)
    {
        earchAction0Ptr->oamProcessingEnable = earchAction1Ptr->oamProcessingEnable;
        earchAction0Ptr->oamProfile = earchAction1Ptr->oamProfile;
    }
    */

    /* ttlExpiryVccvEnable */
    if(earchAction1Ptr->ttlExpiryVccvEnable)
    {
        EARCH_ACTION_OVERRIDE(ttlExpiryVccvEnable);
    }

    /* mpls flow label exists */
    if(earchAction1Ptr->pwe3FlowLabelExist)
    {
        EARCH_ACTION_OVERRIDE(pwe3FlowLabelExist);
    }

    /* mpls control word exists */
    /* TBD: field not found */

    /* applynonDataCwCommand */
    if(earchAction1Ptr->applynonDataCwCommand)
    {
        EARCH_ACTION_OVERRIDE(applynonDataCwCommand);
    }
    /* pwCwBasedETreeEnable */
    if(earchAction1Ptr->pwCwBasedETreeEnable)
    {
        EARCH_ACTION_OVERRIDE(pwCwBasedETreeEnable);
    }

    /* setMAC2ME */
    if(earchAction1Ptr->setMAC2ME)
    {
        EARCH_ACTION_OVERRIDE(setMAC2ME);
    }

    /* Copy TTL/EXP from Tunnel Header */
    if(xcatAction1Ptr->copyTtlFromOuterHeader)
    {
        ACTION_OVERRIDE(copyTtlFromOuterHeader);
    }

    /* MPLS ttl */
    ACTION_OVERRIDE(ttl);

    /* IPCL UDB Config Index */
    if(earchAction1Ptr->ipclUdbConfigurationTableUdeIndex)
    {
        EARCH_ACTION_OVERRIDE(ipclUdbConfigurationTableUdeIndex);
    }

    /* TTI copyReservedAssignEnable */
    if(earchAction1Ptr->ttiReservedAssignEnable)
    {
        EARCH_ACTION_OVERRIDE(ttiReservedAssignEnable);

        /* TTI assign copyReserved*/
        EARCH_ACTION_OVERRIDE(ttiReservedAssignValue);
    }

    if(SMEM_CHT_IS_SIP6_10_GET(devObjPtr))
    {
        if(earchAction1Ptr->triggerCncHashClient)
        {
            /* TTI Trigger Cnc Hash Client */
            EARCH_ACTION_OVERRIDE(triggerCncHashClient);
        }
    }

    if(SMEM_CHT_IS_SIP6_30_GET(devObjPtr))
    {
        /* Source EPG value */
        if(earchAction1Ptr->srcEpg)
        {
             EARCH_ACTION_OVERRIDE(srcEpg);
        }

        /* Destination EPG value */
        if(earchAction1Ptr->dstEpg)
        {
             EARCH_ACTION_OVERRIDE(dstEpg);
        }
    }
}

/**
* @internal snetLion3TTCombinedActionGet function
* @endinternal
*
* @brief   Sip5 devices: Get the combined action (for dual/quad tti lookup)
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to frame descriptor.
* @param[in] matchIndexArr            - array of indexes in action table
* @param[in,out] internalTtiInfoPtr       - (pointer to) internal TTI info
*                                      actionInfoPtr      - first action data
*/
static GT_VOID snetLion3TTCombinedActionGet
(
    IN    SKERNEL_DEVICE_OBJECT           *devObjPtr,
    IN    SKERNEL_FRAME_CHEETAH_DESCR_STC *descrPtr,
    IN    GT_U32                          *matchIndexArr,
    INOUT INTERNAL_TTI_DESC_INFO_STC      *internalTtiInfoPtr,
    INOUT SNET_XCAT_TT_ACTION_STC         *actionDataPtr
)
{
    DECLARE_FUNC_NAME(snetLion3TTCombinedActionGet);

    GT_U32                      u32TcamSegmentMode;/*segment mode*/
    GT_U32                      lookupNum;
    GT_U32                      validHitBmp = 0;

    /* TTI unit of sip5 support only 2 lookups , sip5_10 support 4 */
    internalTtiInfoPtr->maxNumOfLookups =
        SMEM_CHT_IS_SIP5_10_GET(devObjPtr) ?
            (devObjPtr->limitedNumOfParrallelLookups ?
             devObjPtr->limitedNumOfParrallelLookups :
             SIP5_TCAM_MAX_NUM_OF_HITS_CNS) : 2;

    /* save all the actions data to the internal tti info */
    for(lookupNum = 0; lookupNum < internalTtiInfoPtr->maxNumOfLookups; lookupNum++)
    {
        if(matchIndexArr[lookupNum] == SNET_CHT_POLICY_NO_MATCH_INDEX_CNS)
        {
            __LOG(("lookup num : [%d] did not match the tcam (so no more actions to process) \n",lookupNum));
            break;
        }

        /* init pointers first */
        internalTtiInfoPtr->lookupsInfo[lookupNum].actionLionPtr =
                    &internalTtiInfoPtr->lookupsInfo[lookupNum].actionEarchData.baseActionData;
        internalTtiInfoPtr->lookupsInfo[lookupNum].actionXcatPtr =
                    &internalTtiInfoPtr->lookupsInfo[lookupNum].actionEarchData.baseActionData.xcatTTActionData;

        /* get action data */
        if( 0 == lookupNum )
        {
            /* save first (original and not modified) action to lookups info struct */
            memcpy(&internalTtiInfoPtr->lookupsInfo[0].actionEarchData,
                   (SNET_E_ARCH_TT_ACTION_STC*)(GT_VOID*)actionDataPtr,
                   sizeof(SNET_E_ARCH_TT_ACTION_STC));

            internalTtiInfoPtr->lookupsInfo[lookupNum].isValid = GT_TRUE; /* indication lookup is valid */
        }
        else
        {
            __LOG(("get next action from index[0x%x]", matchIndexArr[lookupNum]));
            snetLion3TTActionGet(devObjPtr, descrPtr,
                                 matchIndexArr[lookupNum],
                                 0 /* dummy emUnitNum */,
                                 internalTtiInfoPtr->isMatchDoneInTcamOrEmArr[lookupNum],
                                 internalTtiInfoPtr->ttiKeyType, lookupNum,
                                 0 /* dummy Serial EM profile ID - not relevant for Parallel lookup */,
                                 internalTtiInfoPtr->lookupsInfo[lookupNum].actionXcatPtr);
        }

        internalTtiInfoPtr->lookupsInfo[lookupNum].isActionDataValid = GT_TRUE; /* indication that action data is valid */

        /* continueToNextTtiLookup not valid for SIP7 devices */
        if (!SMEM_CHT_IS_SIP7_GET(devObjPtr))
        {
            /* check if next action enabled (and not last) */
            if(0 == internalTtiInfoPtr->lookupsInfo[lookupNum].actionEarchData.continueToNextTtiLookup &&
            (lookupNum != (internalTtiInfoPtr->maxNumOfLookups - 1)))
            {
                __LOG(("lookup num : [%d] not enabled next action (so no more actions to process)  \n", lookupNum));
                break;
            }
        }
    }

    /* get hits validation info */
    u32TcamSegmentMode = snetTTSip5TcamCheckSegmentMode(devObjPtr, internalTtiInfoPtr->ttiKeyType);
    if(u32TcamSegmentMode == 0)
    {
        /* two tcam mode */
        __LOG(("next actions are not relevant, because tcam mode is 'one segment' (not '2/4 segments') \n"));
        return;
    }

    if(SMEM_CHT_IS_SIP5_10_GET(devObjPtr))
    {
        __LOG(("sip5_10: TTI Quad Lookup support \n"));

        lion3PclValidActionBmpGet(devObjPtr,matchIndexArr,u32TcamSegmentMode,&validHitBmp);

        __LOG((
               "TTI : action index valid bitmap for quad results resolution [0x%x],"
               "TCAM segment Mode [%d]\n"
               ,validHitBmp,u32TcamSegmentMode));
    }
    else
    {
        __LOG(("sip5: TTI Dual Lookup support \n"));
        validHitBmp = 0x3; /* make generic code with sip5_10 */
    }

    /* combine actions*/
    for(lookupNum = 1; lookupNum < internalTtiInfoPtr->maxNumOfLookups; lookupNum++)
    {
        if(GT_FALSE == internalTtiInfoPtr->lookupsInfo[lookupNum].isActionDataValid)
        {
            __LOG(("lookup num : [%d] action data is not valid  \n",lookupNum));
            break;
        }

        if( 0 == ((validHitBmp >> lookupNum)&1) )
        {
            __LOG(("lookup num : [%d] hit is not valid (so no more actions to process)  \n",lookupNum));
            break;
        }

        internalTtiInfoPtr->lookupsInfo[lookupNum].isValid = GT_TRUE; /* indication lookup is valid */

        __LOG(("lookup num : [%d] , combine action (with previous one(s)) \n",lookupNum));
        /* get 'combined action' into actionDataPtr */
        snetLion3TTCombineTwoActions(devObjPtr, descrPtr, lookupNum , internalTtiInfoPtr,
                                     internalTtiInfoPtr->lookupsInfo[lookupNum].actionXcatPtr,
                                     actionDataPtr);
    }

    __LOG(("Ended Processing the TTI tcam Action(s) \n"));
}

/**
* @internal snetCht3TTProcessAction function
* @endinternal
*
* @brief   T.T Engine processing for outgoing frame on Cheetah3 and above:
*         actions to descriptor processing
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] descrPtr                 - pointer to frame data buffer Id
* @param[in] matchIndexArr            - array of indexes in action table
* @param[in,out] descrPtr                 - pointer to updated frame data buffer Id
*
* @param[out] internalTtiInfoPtr       - (pointer to) internal TTI info
*/
static GT_VOID snetCht3TTProcessAction
(
    IN    SKERNEL_DEVICE_OBJECT             *devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC   *descrPtr,
    IN    GT_U32                            *matchIndexArr,
    OUT   INTERNAL_TTI_DESC_INFO_STC        *internalTtiInfoPtr
)
{
    DECLARE_FUNC_NAME(snetCht3TTProcessAction);

    GT_BOOL status;

    SNET_CHT3_TT_ACTION_STC      actionData;
    SNET_E_ARCH_TT_ACTION_STC    actionDataEArch;/*E-Arch action info*/
    SNET_XCAT_TT_ACTION_STC     *actionDataXcatPtr;
    GT_U32                       saveTunnelTtl = GT_FALSE; /* save ttl from tunnel */

    /* to avoid compilation warnings */
    memset(&actionData,0,sizeof(SNET_CHT3_TT_ACTION_STC));
    memset(&actionDataEArch,0,sizeof(actionDataEArch));

    actionDataXcatPtr = &actionDataEArch.baseActionData.xcatTTActionData;

    __LOG(("read and apply T.T action (index [0x%x]) \n",
        matchIndexArr[0]));

    descrPtr->ttiLookupMatch = 1;

    __LOG(("read the action from the action table"));

    if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        snetLion3TTActionGet(devObjPtr, descrPtr, matchIndexArr[0],
                             0 /* dummy emUnitNum */,
                             internalTtiInfoPtr->isMatchDoneInTcamOrEmArr[0],
                             internalTtiInfoPtr->ttiKeyType, 0 /* lookup#0 */,
                             0 /* dummy Serial EM profile ID - not relevant for Parallel lookup */,
                             actionDataXcatPtr);

        snetLion3TTCombinedActionGet(devObjPtr, descrPtr, matchIndexArr,
                                     internalTtiInfoPtr, actionDataXcatPtr);
    }
    else
    {
        snetXcatTTActionGet(devObjPtr, descrPtr, matchIndexArr[0], actionDataXcatPtr, internalTtiInfoPtr);
    }

    /* do exceptions check , only for TTI lookup that matches a rule */
    status = snetCht3TTIExceptionChecks(devObjPtr, descrPtr, internalTtiInfoPtr);
    if(GT_FALSE == status)
    {
        __LOG(("Got exception (so will not apply the TTI action)\n"));
        return;
    }

    /* check for illegal MPLS command */
    status = snetXcatTTMplsHeaderCheck(devObjPtr, descrPtr, actionDataXcatPtr);
    if(status == GT_FALSE)
    {
        __LOG(("illegal MPLS command (so will not apply the TTI action) \n"));
        return;
    }

    __LOG(("apply the matching action"));
    snetXcatTTActionApply(devObjPtr, descrPtr, actionDataXcatPtr, internalTtiInfoPtr);

    if(actionDataXcatPtr->bindToCncCounter)
    {
        internalTtiInfoPtr->cncIndex = actionDataXcatPtr->cncCounterIndex;
    }

    __LOG(("mpls lsr ttl exception handling"));
    status = snetXcatTTMplsTtlExceededException(devObjPtr, descrPtr, actionDataXcatPtr);
    if(status == GT_FALSE)
        return;

    if(descrPtr->tunnelTerminated)
    {
        descrPtr->l3StartOffsetPtr = descrPtr->l3StartOfPassenger;

        descrPtr->payloadPtr = descrPtr->l3StartOfPassenger;
        descrPtr->payloadLength = descrPtr->passengerLength;
        saveTunnelTtl = descrPtr->ttl;

        /* reset the descriptor regarding fields of L3,L4 , so that fields that
           not set by function snetChtL3L4ProtParsing , will not stay with
           values from previous call to snetChtL3L4ProtParsing */
        snetChtL3L4ProtParsingResetDesc(devObjPtr, descrPtr);
        __LOG(("L3, L4 protocols parsing"));
        snetChtL3L4ProtParsing(devObjPtr, descrPtr, descrPtr->etherTypeOrSsapDsap, internalTtiInfoPtr);

        /****************************************************/
        /*do fragment checks - on tunnel terminated packets */
        /****************************************************/
        if(descrPtr->isIPv4)
        {
            /****************************/
            /* T.T IPv4  fragment check */
            /****************************/
            snetCht3TTIPv4FragmentCheck(devObjPtr, descrPtr, internalTtiInfoPtr);
        }


        /* If <copyTtlFromOuterHeader> is set, the packets TTL is assigned according to the
           tunnel header rather than from passenger packets TTL/Hop-Limit.
            snetChtL3L4ProtParsing is assigning the ttl from passenger */
        if (actionDataXcatPtr->copyTtlFromOuterHeader == 1)
        {
            descrPtr->ttl = saveTunnelTtl;
            descrPtr->copyTtl = GT_TRUE;
        }
        else
        {
            descrPtr->copyTtl = GT_FALSE;
        }

        /* Resolve ECN command for tunnel terminated IP packets. Applicable only for SIP6 devices */
        if (SMEM_CHT_IS_SIP6_GET(devObjPtr))
        {
            snetFalconTTEcnResolve(devObjPtr, descrPtr);
        }
    }
    else /* non terminated */
    {
        if (descrPtr->ttiLookupMatch)
        {
            if(descrPtr->mpls && actionDataXcatPtr->mplsCmd != SKERNEL_XCAT_TTI_MPLS_NOP_E)
            {
                descrPtr->innerPacketType = actionDataXcatPtr->passengerPacketType;
                if(!SMEM_CHT_IS_SIP7_GET(devObjPtr))
                {
                    descrPtr->mplsCommand = actionDataXcatPtr->mplsCmd;
                }
            }
        }
    }

    /* apply the matching action */
    snetXcatTTActionApplyPart2(devObjPtr, descrPtr , actionDataXcatPtr);

    if(devObjPtr->oamSupport.oamSupport)
    {
        __LOG(("Update relevant OAM descriptor fields"));
        snetLion2TTOamDescriptorSet(devObjPtr, descrPtr, actionDataXcatPtr);
    }

    internalTtiInfoPtr->keepPreviousQoS = actionDataXcatPtr->keepPreviousQoS;
}

/**
* @internal snetAasExactMatchSerialEmTTProcessAction function
* @endinternal
*
* @brief  Serial EM T.T Engine processing for outgoing frame on AAS and above:
*         actions to descriptor processing
* @param[in]     devObjPtr                - pointer to device object.
* @param[in,out] descrPtr                 - pointer to frame data buffer Id
* @param[in]     emUnitNum                - Exact Match unit number
* @param[in]     matchIndex               - index in action table
* @param[in]     serialEmProfileId        - Serial EM profile ID
* @param[out]    internalTtiInfoPtr       - (pointer to) internal TTI info
*/
GT_VOID snetAasExactMatchSerialEmTTProcessAction
(
    IN    SKERNEL_DEVICE_OBJECT             *devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC   *descrPtr,
    IN    SIP7_EXACT_MATCH_UNIT_ENT         emUnitNum,
    IN    GT_U32                            lookupNum,
    IN    GT_U32                            matchIndex,
    IN    GT_U32                            serialEmProfileId,
    OUT   INTERNAL_TTI_DESC_INFO_STC        *internalTtiInfoPtr
)
{
    DECLARE_FUNC_NAME(snetAasExactMatchSerialEmTTProcessAction);
    SNET_CHT3_TT_ACTION_STC     actionData;
    SNET_E_ARCH_TT_ACTION_STC   actionDataEArch;
    SNET_XCAT_TT_ACTION_STC     *actionDataXcatPtr;
    GT_BOOL                     status;
    GT_U32                      saveTunnelTtl = GT_FALSE;

    memset(&actionData,0,sizeof(SNET_CHT3_TT_ACTION_STC));
    memset(&actionDataEArch,0,sizeof(actionDataEArch));

    actionDataXcatPtr = &actionDataEArch.baseActionData.xcatTTActionData;

    __LOG(("Read and Apply T.T action (index [0x%x]) \n", matchIndex));

    descrPtr->ttiLookupMatch = 1;

    snetLion3TTActionGet(devObjPtr, descrPtr, matchIndex, emUnitNum,
                         GT_FALSE /* Dummy variable for Serial EM */,
                         0 /* Dummy variable for Serial EM */,
                         lookupNum,
                         serialEmProfileId, actionDataXcatPtr);

    /* check for illegal MPLS command */
    status = snetXcatTTMplsHeaderCheck(devObjPtr, descrPtr, actionDataXcatPtr);
    if(status == GT_FALSE)
    {
        __LOG(("Illegal MPLS command (so will not apply the TTI action) \n"));
        return;
    }

    __LOG(("apply the matching action"));
    snetXcatTTActionApply(devObjPtr, descrPtr, actionDataXcatPtr, internalTtiInfoPtr);

    if(actionDataXcatPtr->bindToCncCounter)
    {
        internalTtiInfoPtr->cncIndex = actionDataXcatPtr->cncCounterIndex;
    }

    __LOG(("MPLS lsr ttl exception handling"));
    status = snetXcatTTMplsTtlExceededException(devObjPtr, descrPtr, actionDataXcatPtr);
    if(status == GT_FALSE)
        return;

    if(descrPtr->tunnelTerminated)
    {
        descrPtr->l3StartOffsetPtr = descrPtr->l3StartOfPassenger;

        descrPtr->payloadPtr = descrPtr->l3StartOfPassenger;
        descrPtr->payloadLength = descrPtr->passengerLength;
        saveTunnelTtl = descrPtr->ttl;

        /* reset the descriptor regarding fields of L3,L4 , so that fields that
           not set by function snetChtL3L4ProtParsing , will not stay with
           values from previous call to snetChtL3L4ProtParsing */
        snetChtL3L4ProtParsingResetDesc(devObjPtr, descrPtr);
        __LOG(("L3, L4 protocols parsing"));
        snetChtL3L4ProtParsing(devObjPtr, descrPtr, descrPtr->etherTypeOrSsapDsap, internalTtiInfoPtr);

        /****************************************************/
        /*do fragment checks - on tunnel terminated packets */
        /****************************************************/
        if(descrPtr->isIPv4)
        {
            /****************************/
            /* T.T IPv4  fragment check */
            /****************************/
            snetCht3TTIPv4FragmentCheck(devObjPtr, descrPtr, internalTtiInfoPtr);
        }


        /* If <copyTtlFromOuterHeader> is set, the packets TTL is assigned according to the
           tunnel header rather than from passenger packets TTL/Hop-Limit.
            snetChtL3L4ProtParsing is assigning the ttl from passenger */
        if (actionDataXcatPtr->copyTtlFromOuterHeader == 1)
        {
            descrPtr->ttl = saveTunnelTtl;
            descrPtr->copyTtl = GT_TRUE;
        }
        else
        {
            descrPtr->copyTtl = GT_FALSE;
        }

        /* Resolve ECN command for tunnel terminated IP packets. Applicable only for SIP6 devices */
        if (SMEM_CHT_IS_SIP6_GET(devObjPtr))
        {
            snetFalconTTEcnResolve(devObjPtr, descrPtr);
        }
    }
    else /* non terminated */
    {
        if (descrPtr->ttiLookupMatch)
        {
            if(descrPtr->mpls && actionDataXcatPtr->mplsCmd != SKERNEL_XCAT_TTI_MPLS_NOP_E)
            {
                descrPtr->innerPacketType = actionDataXcatPtr->passengerPacketType;
                if(!SMEM_CHT_IS_SIP7_GET(devObjPtr))
                {
                    descrPtr->mplsCommand = actionDataXcatPtr->mplsCmd;
                }
            }
        }
    }

    /* apply the matching action */
    snetXcatTTActionApplyPart2(devObjPtr, descrPtr , actionDataXcatPtr);

    if(devObjPtr->oamSupport.oamSupport)
    {
        __LOG(("Update relevant OAM descriptor fields"));
        snetLion2TTOamDescriptorSet(devObjPtr, descrPtr, actionDataXcatPtr);
    }

    internalTtiInfoPtr->keepPreviousQoS = actionDataXcatPtr->keepPreviousQoS;
}

/**
* @internal snetXcatTTActionApplyVlanTagCmd function
* @endinternal
*
* @brief   Apply the action entry from the TTI action table - related to vlan tags changes:
*         vid,vpt.
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] descrPtr                 - pointer to frame descriptor.
* @param[in,out] descrPtr                 - pointer to frame descriptor.
*                                      RETURN:
*                                      COMMENTS:
*/
static GT_VOID snetXcatTTActionApplyVlanTagCmd
(
    IN    SKERNEL_DEVICE_OBJECT             *devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC   *descrPtr,
    INOUT INTERNAL_TTI_DESC_INFO_STC        *internalTtiInfoPtr
)
{
    DECLARE_FUNC_NAME(snetXcatTTActionApplyVlanTagCmd);

    INTERNAL_TTI_DESC_INFO_VLAN_CMD_INFO_STC newValues = internalTtiInfoPtr->vlan_origInfo;
    GT_BIT vidModified = 0;
    GT_BIT allowVid1Assignment = 0;
    GT_BIT allowUp0Modify = 0;
    GT_BIT allowUp1Modify = 0;

    GT_BIT isTag0Tagged = (descrPtr->origSrcTagged && (descrPtr->nestedVlanAccessPort == 0) && (descrPtr->srcPriorityTagged == 0)) ? 1 : 0;
    /* if tag1 exits and nested vlan also true the behaviour should be as the packet was untagged;
       do not do assignment to vlan1 from the action if the action is for untagged/prioirty tagged.*/
    GT_BIT isTag1Tagged = (TAG1_EXIST_MAC(descrPtr) == 1) && (descrPtr->vid1 != 0) ? 1 : 0;

    __LOG_PARAM(isTag0Tagged);
    __LOG_PARAM(isTag1Tagged);

    __LOG_PARAM(internalTtiInfoPtr->vlan_actionInfo.vid0Cmd);

    /* must be set here to override new vlan tag parsing , QOS setting from
      snetChtL2Parsing(...) */
    if(internalTtiInfoPtr->vlan_actionInfo.vid0Cmd == 4)
    {
        /* enVirtualBridgeVLAN :
        When the new command is assigned, eVLAN is assigned to all packets by concatenating:
        eVLAN = {TTI-Action<eVLAN[15:12]>, tag0 VID[11:0]}*/
        /* the descrPtr->eVid already contain the tag0 VID[11:0] */
        SMEM_U32_SET_FIELD(newValues.vid0,12,4, SMEM_U32_GET_FIELD(internalTtiInfoPtr->vlan_actionInfo.vid0,12,4));
        vidModified = 1;
    }
    else
    /* packet that arrives here is tunnel  */
    if (((internalTtiInfoPtr->vlan_actionInfo.vid0Cmd == 1)  && (isTag0Tagged == 0)) ||
        ((internalTtiInfoPtr->vlan_actionInfo.vid0Cmd == 2)  && (isTag0Tagged == 1)) ||
         (internalTtiInfoPtr->vlan_actionInfo.vid0Cmd == 3))
    {
        newValues.vid0 = internalTtiInfoPtr->vlan_actionInfo.vid0;
        vidModified = 1;
    }

    __LOG_PARAM(internalTtiInfoPtr->vlan_actionInfo.allowVid1Assignment);
    __LOG_PARAM(internalTtiInfoPtr->vlan_actionInfo.vid1Cmd);
    __LOG_PARAM(descrPtr->vid1);
    __LOG_PARAM(descrPtr->eVid);

    if(internalTtiInfoPtr->vlan_actionInfo.allowVid1Assignment)
    {
        if(internalTtiInfoPtr->vlan_actionInfo.vid1Cmd == 1 || /*All: The entrys <Tag1 VID> is assigned to all packets*/
           (internalTtiInfoPtr->vlan_actionInfo.vid1Cmd == 0 &&
            isTag1Tagged == 0))
        {
            allowVid1Assignment = 1;

            newValues.vid1 = internalTtiInfoPtr->vlan_actionInfo.vid1;
        }
    }

    __LOG_PARAM(internalTtiInfoPtr->vlan_actionInfo.up1Cmd);

    if(internalTtiInfoPtr->vlan_actionInfo.up1Cmd == 2 ||
       (internalTtiInfoPtr->vlan_actionInfo.up1Cmd == 0 && (0 == TAG1_EXIST_MAC(descrPtr))))
    {
        allowUp1Modify = 1;
        /*0 = Assign VLAN1 Untagged: If packet does not contain Tag1 assign
            according to action entrys <UP1>, else use Tag1<UP>*/
        /*2 = Assign All: Assign action entrys <UP1> field to all packets.*/
        newValues.up1 = internalTtiInfoPtr->vlan_actionInfo.up1;
    }
    else if(internalTtiInfoPtr->vlan_actionInfo.up1Cmd == 1)
    {
        allowUp1Modify = 1;
        /*1 = Assign VLAN0 Untagged: If packet contains Tag0 use Tag0<UP0>, else
            use action entrys <UP1> field.*/
        newValues.up1 = TAG0_EXIST_MAC(descrPtr) ?
                    descrPtr->vlanTag802dot1dInfo.vpt : /*Tag0<UP0>*/
                    internalTtiInfoPtr->vlan_actionInfo.up1;
    }

    if(TAG0_EXIST_MAC(descrPtr) == 0)
    {
        /*If the packet has a match in the TTI lookup, the Tag0 User Priority may be overridden:
            For non-tunnel-terminated packets:
                If the packet Tag0 is found, then Tag0 UP assignment is taken from the packet Tag0;
                Else, the <Tag0 UP> field in the TTI action determines the Tag0 User Priority.
            For tunnel-terminated packets :
                If the passenger packet Tag0 is found, then the Tag0 User Priority assignment
                is taken from the passenger packet Tag0.
                Else, the <Tag0 UP> field in the TTI action determines the Tag0 User Priority.
        */

        __LOG(("TTI action :The Tag0 User is overridden \n"));
        allowUp0Modify = 1;
        newValues.up0 = internalTtiInfoPtr->vlan_actionInfo.up0;
    }

    descrPtr->vidModified = vidModified ||
        internalTtiInfoPtr->vlan_origInfo.vidModified;
    if(vidModified)
    {
        if(descrPtr->eVid != newValues.vid0)
        {
            __LOG(("TTI action :%s changed from [0x%x] to [0x%x] \n",
                "descrPtr->eVid",
                 descrPtr->eVid,
                 newValues.vid0));
        }
        descrPtr->eVid = newValues.vid0;
    }

    if(allowVid1Assignment)
    {
        if(descrPtr->vid1 != newValues.vid1)
        {
            __LOG(("TTI action :%s changed from [0x%x] to [0x%x] \n",
                "descrPtr->vid1",
                 descrPtr->vid1,
                 newValues.vid1));
        }
        descrPtr->vid1 = newValues.vid1;
    }

    if(allowUp1Modify)
    {
        if(descrPtr->up1 != newValues.up1)
        {
            __LOG(("TTI action :%s changed from [0x%x] to [0x%x] \n",
                "descrPtr->up1",
                 descrPtr->up1,
                 newValues.up1));
        }
        descrPtr->up1  = newValues.up1;
    }

    if(allowUp0Modify)
    {
        if(descrPtr->up != newValues.up0)
        {
            __LOG(("TTI action : %s changed from [0x%x] to [0x%x] \n",
                "descrPtr->up",
                 descrPtr->up,
                 newValues.up0));
        }
        descrPtr->up   = newValues.up0;
    }

}

/**
* @internal snetCht3TTerminationMain function
* @endinternal
*
* @brief   T.T Engine processing for outgoing frame on Cheetah3 and above
*         asic simulation.
*         T.T processing , T.T assignment ,key forming , 1 Lookup ,
*         actions to descriptor processing
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] descrPtr                 - pointer to frame data buffer
* @param[in,out] descrPtr                 - pointer to updated frame data buffer
*
* @param[out] internalTtiInfoPtr       - (pointer to) internal TTI info
*                                      RETURN:
*                                      COMMENTS:
*                                      T.T has only one lookup cycle.
*                                      D.12.8.3 : T.T Registers
*
* @note T.T has only one lookup cycle.
*       D.12.8.3 : T.T Registers
*
*/
static GT_VOID snetCht3TTerminationMain
(
    IN    SKERNEL_DEVICE_OBJECT             * devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC   * descrPtr,
    OUT   INTERNAL_TTI_DESC_INFO_STC        *internalTtiInfoPtr
)
{
    DECLARE_FUNC_NAME(snetCht3TTerminationMain);

    GT_BOOL                         status;
    SNET_CHT_POLICY_KEY_STC         ttKey;       /* TT key structure   */
    GT_U32                          matchIndexArr[SIP5_TCAM_MAX_NUM_OF_HITS_CNS];
    GT_BOOL                         isMatchDoneInTcamOrEmArr[SIP5_TCAM_MAX_NUM_OF_HITS_CNS] = {GT_TRUE,GT_TRUE,GT_TRUE,GT_TRUE};
                                    /* is the index match done in TCAM (TRUE) or in EM (FALSE) */
    SNET_CHT_POLICY_KEY_STC        *keyPtr;
    GT_U32                          keySize = 0; /* prevent warning */
    GT_U32                          indexFromKeyType;
    GT_U32                          exactMatchProfileIdTableIndex;
    GT_U32                          tcamProfileIdIndex;
    GT_U32                          ii=0;
    GT_BOOL                         tcamOverExactMatchPriorityArr[SIP5_TCAM_MAX_NUM_OF_HITS_CNS]={GT_TRUE};



    ASSERT_PTR(internalTtiInfoPtr);
    internalTtiInfoPtr->keepPreviousQoS = GT_TRUE;

    internalTtiInfoPtr->maxNumOfLookups = 1; /* default value */

    /*  T.T LookUp Trigger 13.3.1) */
    __LOG(("T.T LookUp Trigger 13.3.1)"));
    status = snetXcatTTTriggeringCheck(devObjPtr, descrPtr, internalTtiInfoPtr);

    if (status == GT_FALSE) /* =0 , means T.T engine is disabled */
    {
        __LOG(("the packet not subject to TTI lookup"));
        return ;
    }

    logTtiKeyType(devObjPtr, internalTtiInfoPtr->ttiKeyType);

    keyPtr = &ttKey;

    if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        __LOG(("MAX 30 bytes key -- in TT (Router TCAM) \n"));

        /* the regular currently hold 26 bytes --> not enough for 30 bytes key */
        snetCht3TtiKeyPclIdGet(devObjPtr, descrPtr,
                               internalTtiInfoPtr->ttiKeyType, NULL, &keySize);
        switch(keySize)
        {
            case 10:
                keyPtr->pclKeyFormat = CHT_PCL_KEY_10B_E;
                break;
            case 20:
                keyPtr->pclKeyFormat = CHT_PCL_KEY_20B_E;
                break;
            case 30:
                keyPtr->pclKeyFormat = CHT_PCL_KEY_30B_E;
                break;
            default:
                skernelFatalError("snetCht3TTerminationMain: non valid key size",keySize);
                break;
        }
    }
    else
    {
        __LOG(("24 bytes key -- in TT (Router TCAM) \n"));

        keyPtr->pclKeyFormat = CHT_PCL_KEY_REGULAR_E;
    }

    keyPtr->updateOnlyDiff = GT_FALSE;

    memset(&keyPtr->key.triple[0],0,SNET_CHT_POLICY_KEY_SIZE_BYTES_TRIPLE_CNS);
    keyPtr->devObjPtr = devObjPtr;

    /* create T.T search key */
    __LOG(("create T.T search key"));
    snetCht3TTCreateKey(devObjPtr, descrPtr, keyPtr, internalTtiInfoPtr);

    /* Assure that simulated TCAM lookup is atomic:
     *  no TCAM changes are allowed during whole TCAM lookup and action applying */
    SCIB_SEM_TAKE;

    if(SMEM_CHT_IS_SIP6_GET(devObjPtr))
    {
        SIP5_TCAM_KEY_SIZE_ENT   sip5KeySize;

        /* convert old key format to new key size */
        sip5TcamConvertPclKeyFormatToKeySize(keyPtr->pclKeyFormat, &sip5KeySize);

        /* Get Index For Key Type */
        snetFalconTtiGetIndexForKeyType(internalTtiInfoPtr->ttiKeyType,&indexFromKeyType);
        exactMatchProfileIdTableIndex = indexFromKeyType;
        tcamProfileIdIndex = 0;
        if (SMEM_CHT_IS_SIP6_10_GET(devObjPtr))
        {
            GT_U32 profileId;
            GT_U32 profileIdMode;
            GT_U32 regAddr;
            GT_U32 index;

            profileId = SMEM_LION3_TTI_PHYSICAL_PORT_ENTRY_FIELD_GET(
                devObjPtr, descrPtr, SMEM_SIP6_10_TTI_PHYSICAL_PORT_TABLE_FIELDS_TCAM_PROFILE_ID);
            profileIdMode = SMEM_LION3_TTI_PHYSICAL_PORT_ENTRY_FIELD_GET(
                devObjPtr, descrPtr, SMEM_SIP6_10_TTI_PHYSICAL_PORT_TABLE_FIELDS_TCAM_PROFILE_ID_MODE);

            switch (profileIdMode)
            {
                case 0: /* per packet type */
                    regAddr = SMEM_TABLE_ENTRY_INDEX_GET_MAC(devObjPtr, ttiPacketTypeTcamProfileIdMapping, indexFromKeyType);
                    smemRegGet(devObjPtr, regAddr, &tcamProfileIdIndex);
                    break;
                case 1: /* per port */
                    tcamProfileIdIndex = profileId;
                    break;
                case 2: /* per port and packet type */
                    index = (indexFromKeyType << 3) | (profileId & 0x7);
                    regAddr = SMEM_TABLE_ENTRY_INDEX_GET_MAC(
                        devObjPtr, ttiPortAndPacketTypeTcamProfileIdMapping, index);
                    smemRegGet(devObjPtr, regAddr, &tcamProfileIdIndex);
                    break;
            }

        }

        /* SIP7 : Serial EM lookups */
        if (SMEM_CHT_IS_SIP7_GET(devObjPtr))
        {
            /* Perform Serial EM lookups for TTI_ILM Client */
            snetAasExactMatchSerialEmLookup(devObjPtr, descrPtr, SIP7_EMX_CLIENT_TTI_SERIAL_EM_E,
                                            internalTtiInfoPtr);
        }

        snetFalconTcamLookup(
            devObjPtr,descrPtr, SIP5_TCAM_CLIENT_TTI_E, exactMatchProfileIdTableIndex,
            tcamProfileIdIndex, keyPtr->key.data, sip5KeySize, matchIndexArr,isMatchDoneInTcamOrEmArr,
            tcamOverExactMatchPriorityArr);

        /* SIP7 : Do Exact Match lookup in EMX */
        if (SMEM_CHT_IS_SIP7_GET(devObjPtr))
        {
            snetAasExactMatchLookupInEmx(devObjPtr, descrPtr, SIP7_EMX_CLIENT_TTI_E,
                                         exactMatchProfileIdTableIndex, keyPtr->key.data, sip5KeySize,
                                         matchIndexArr, isMatchDoneInTcamOrEmArr,
                                         tcamOverExactMatchPriorityArr);
        }
    }
    else
    {
        /* search key in T.T Tcam */
        __LOG(("search key in T.T Tcam"));

        if(devObjPtr->supportEArch)
        {
            SIP5_TCAM_KEY_SIZE_ENT   sip5KeySize;

            /* convert old key format to new key size */
            sip5TcamConvertPclKeyFormatToKeySize(keyPtr->pclKeyFormat, &sip5KeySize);

            /* search the key */
            sip5TcamLookup(
                devObjPtr, SIP5_TCAM_CLIENT_TTI_E,
                0/*tcamProfileId*/, keyPtr->key.data, sip5KeySize, matchIndexArr);
        }
        else
        {
            snetCht3TTLookUp(devObjPtr, descrPtr, keyPtr, &matchIndexArr[0]);
        }
    }

    if(matchIndexArr[0] != SNET_CHT_POLICY_NO_MATCH_INDEX_CNS)
    {
        /*save parameters before the tunnel termination (if any)*/
        SAVE_TUNNEL_INFO_MAC( ipHeaderError    ,internalTtiInfoPtr,descrPtr);
        SAVE_TUNNEL_INFO_MAC( ipTtiHeaderError ,internalTtiInfoPtr,descrPtr);
        SAVE_TUNNEL_INFO_MAC( isIpV6EhHopByHop ,internalTtiInfoPtr,descrPtr);
        SAVE_TUNNEL_INFO_MAC( isIpV6EhExists   ,internalTtiInfoPtr,descrPtr);
        SAVE_TUNNEL_INFO_MAC( greHeaderError   ,internalTtiInfoPtr,descrPtr);
        SAVE_TUNNEL_INFO_MAC( isIPv4           ,internalTtiInfoPtr,descrPtr);
        SAVE_TUNNEL_INFO_MAC( isIp             ,internalTtiInfoPtr,descrPtr);
        SAVE_TUNNEL_INFO_MAC( ipOptionFragError,internalTtiInfoPtr,descrPtr);


        for (ii=0;ii<SIP5_TCAM_MAX_NUM_OF_HITS_CNS;ii++)
        {
            internalTtiInfoPtr->isMatchDoneInTcamOrEmArr[ii] = isMatchDoneInTcamOrEmArr[ii];
        }

        /* call action processing only if (first) action found */
        snetCht3TTProcessAction(devObjPtr, descrPtr, matchIndexArr, internalTtiInfoPtr);

        /* do extra exceptions check , only for TTI lookup that matches a rule */
        snetCht3TTIExceptionChecks_part2(devObjPtr, descrPtr, internalTtiInfoPtr);
    }
    else
    {
        __LOG(("Not apply action because TCAM not matched first action \n"));

        if(devObjPtr->oamSupport.oamSupport)
        {
            /* Update relevant OAM descriptor fields */
            __LOG(("Update relevant OAM descriptor fields"));
            snetLion2TTOamDescriptorSet(devObjPtr, descrPtr, NULL);
        }
    }

    SCIB_SEM_SIGNAL;

}

/**
* @internal snetLion3FcoeL3Parse function
* @endinternal
*
* @brief   This function parses FCoE frame L3. Relevant for SIP5 only.
*
* @param[in] devObjPtr                - pointer to device object
* @param[in,out] descrPtr                 - pointer to frame descriptor
* @param[in,out] internalTtiInfoPtr       - (pointer to) internal TTI info
*                                      OUTPUT:
* @param[in,out] descrPtr                 - pointer to frame descriptor
* @param[in,out] internalTtiInfoPtr       - (pointer to) internal TTI info
*                                      RETURN:
*                                      None.
*
* @note FCoE L3 starts from ethertype, UDB0 is also there
*
*/
static GT_VOID snetLion3FcoeL3Parse
(
    IN    SKERNEL_DEVICE_OBJECT           *devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC *descrPtr,
    INOUT INTERNAL_TTI_DESC_INFO_STC      *internalTtiInfoPtr
)
{
    DECLARE_FUNC_NAME(snetLion3FcoeL3Parse);

    GT_U8  *fcOffsetPtr;  /* pointer to fibre channel offset (l4 pointer) */
    GT_U8   rCtl = 0;     /* current rCtl field value */
    GT_BOOL isFirstVfId = GT_TRUE; /* indicate that there is no second VFT header */
    GT_BOOL stopParse = GT_FALSE;

    /* get ver field (4 bits long) */
    __LOG(("get ver field (4 bits long)"));

    internalTtiInfoPtr->fcoeL3Info.ver = descrPtr->l3StartOffsetPtr[0] >> 4;

    /* get SOF field (8 bits long) */
    __LOG(("get SOF field (8 bits long)"));

    internalTtiInfoPtr->fcoeL3Info.sof = descrPtr->l3StartOffsetPtr[13];

    fcOffsetPtr = &descrPtr->l3StartOffsetPtr[14];

    /* when 0 , meaning no VFT header found */
    internalTtiInfoPtr->fcoeL3Info.vftInfo.rCtl = 0;
    do{
        rCtl = fcOffsetPtr[0];

        switch(rCtl)
        {
            case 0x50:/*VFT extended header*/
                if(isFirstVfId == GT_TRUE) /* parse only outermost VFT */
                {
                    __LOG(("got VFT"));

                    /* save rCtl [31:24] */
                    internalTtiInfoPtr->fcoeL3Info.vftInfo.rCtl = rCtl;

                    /* save ver [23:22] (2 bits long) */
                    internalTtiInfoPtr->fcoeL3Info.vftInfo.ver = fcOffsetPtr[1] >> 6;

                    /* save type [21:18] (4 bits long) */
                    internalTtiInfoPtr->fcoeL3Info.vftInfo.type = (fcOffsetPtr[1] >> 2) & 0xf;

                    /* get VF_ID (12 bits long) */
                    internalTtiInfoPtr->fcoeL3Info.vftInfo.vfId =
                        ( (fcOffsetPtr[2] & 0x1F) << 7 )/*5 bits*/ | (fcOffsetPtr[3] >> 1)/*7bits*/;

                    /* save hopCt (8 bits long) */
                    internalTtiInfoPtr->fcoeL3Info.vftInfo.hopCt = fcOffsetPtr[4];

                    if(SMEM_LION3_TTI_DEFAULT_EPORT_ENTRY_FIELD_GET(devObjPtr, descrPtr,
                            SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_ASSIGN_VF_ID_EN))
                    {
                        /* assign VF_ID from outermost VFT */
                        __LOG(("assign VF_ID from outermost VFT"));
                        descrPtr->vrfId = internalTtiInfoPtr->fcoeL3Info.vftInfo.vfId;
                    }

                    isFirstVfId = GT_FALSE;
                }
                fcOffsetPtr += 8; /* 8 bytes long */
            break;

            case 0x51: /* IFR header*/
                __LOG(("IFR header"));
                fcOffsetPtr += 8; /* 8 bytes long */
            break;
            default:/* unknown type */
                stopParse = GT_TRUE;
            break;
        }

        if((fcOffsetPtr - descrPtr->startFramePtr) >= SNET_CHT_PCL_MAX_BYTE_INDEX_CNS)
        {
            stopParse = GT_TRUE;
        }
    }
    while(stopParse == GT_FALSE);

    /* save l4 start offset */
    descrPtr->l4StartOffsetPtr = fcOffsetPtr;

    descrPtr->l23HeaderSize = descrPtr->l4StartOffsetPtr - descrPtr->startFramePtr;
}

/**
* @internal snetLion3FcoeL4Parse function
* @endinternal
*
* @brief   This function parses FCoE frame L4. Relevant for SIP5 only.
*
* @param[in] devObjPtr                - pointer to device object
* @param[in,out] descrPtr                 - pointer to frame descriptor
*                                      OUTPUT:
* @param[in,out] descrPtr                 - pointer to frame descriptor
*                                      RETURN:
*                                      None.
*
* @note FCoE L4 starts from R_CTL field
*
*/
static GT_VOID snetLion3FcoeL4Parse
(
    IN    SKERNEL_DEVICE_OBJECT           *devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC *descrPtr
)
{
    DECLARE_FUNC_NAME(snetLion3FcoeL4Parse);

    GT_U32 offset = 0;

    if(descrPtr->l4StartOffsetPtr == NULL)
    {
        return;
    }

    /* save D_ID to dip */
    __LOG(("save D_ID to dip"));

    offset = 0;
    descrPtr->dip[0] = descrPtr->l4StartOffsetPtr[offset + 1] << 16 |
                       descrPtr->l4StartOffsetPtr[offset + 2] << 8  |
                       descrPtr->l4StartOffsetPtr[offset + 3] << 0;

    /* save S_ID to sip */
    __LOG(("save S_ID to sip"));

    offset = 4;
    descrPtr->sip[0] = descrPtr->l4StartOffsetPtr[offset + 1] << 16 |
                       descrPtr->l4StartOffsetPtr[offset + 2] << 8  |
                       descrPtr->l4StartOffsetPtr[offset + 3] << 0;
}

/**
* @internal snetLion3FcoeParse function
* @endinternal
*
* @brief   This function parses FCoE frame. Relevant for SIP5 only.
*
* @param[in] devObjPtr                - pointer to device object
* @param[in,out] descrPtr                 - pointer to frame descriptor
* @param[in,out] internalTtiInfoPtr       - (pointer to) internal TTI info
*                                      OUTPUT:
* @param[in,out] descrPtr                 - pointer to frame descriptor
* @param[in,out] internalTtiInfoPtr       - (pointer to) internal TTI info
*                                      RETURN:
*                                      None.
*/
static GT_VOID snetLion3FcoeParse
(
    IN    SKERNEL_DEVICE_OBJECT           *devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC *descrPtr,
    INOUT INTERNAL_TTI_DESC_INFO_STC      *internalTtiInfoPtr
)
{
    /* parse L3 */
    snetLion3FcoeL3Parse(devObjPtr, descrPtr, internalTtiInfoPtr);

    /* parse L4 */
    snetLion3FcoeL4Parse(devObjPtr, descrPtr);
}

/**
* @internal snetLion3TtiFcoeTreat function
* @endinternal
*
* @brief   FCoE treatment, relevant for SIP5 only
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] descrPtr                 - pointer to frame data buffer Id
* @param[in,out] internalTtiInfoPtr       - (pointer to) internal TTI info
* @param[in,out] descrPtr                 - pointer to frame data buffer Id
* @param[in,out] internalTtiInfoPtr       - (pointer to) internal TTI info
*/
GT_VOID snetLion3TtiFcoeTreat
(
    IN    SKERNEL_DEVICE_OBJECT             *devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC   *descrPtr,
    INOUT INTERNAL_TTI_DESC_INFO_STC        *internalTtiInfoPtr
)
{
    DECLARE_FUNC_NAME(snetLion3TtiFcoeTreat);

    GT_U32  fldVal = 0;  /* register field's value */
    GT_BOOL status = GT_FALSE;

    /* get the Ethertype of FCoE packets*/
    smemRegFldGet(devObjPtr, SMEM_LION3_TTI_FCOE_GLOBAL_CONFIG_REG(devObjPtr),
                                                        0, 16, &fldVal);
    if(descrPtr->etherTypeOrSsapDsap != fldVal)
    {
        __LOG(("not FCOE packet"));
        return; /* not FCOE packet */
    }

    /* get TTI FCoE Forwarding Enable value */
    smemRegFldGet(devObjPtr, SMEM_LION3_TTI_FCOE_GLOBAL_CONFIG_REG(devObjPtr),
                                                        16, 1, &fldVal);
    if(!fldVal)
    {
        __LOG(("FCoE forwarding disabled"));
        return; /* FCoE forwarding disabled */
    }

    /* state FCOE */
    descrPtr->isFcoe = 1;

    /* FCOE parse */
    snetLion3FcoeParse(devObjPtr, descrPtr, internalTtiInfoPtr);

    /* TT FCoE Exception check */
    status = snetLion3FcoeExceptionCheck(devObjPtr, descrPtr, internalTtiInfoPtr);
    if(status == GT_FALSE)
    {
        __LOG(("Got FCoE Exception"));

        snetLion3FcoeAssignPacketCmdAndCpuCode(devObjPtr, descrPtr);
    }

    return;
}



/**
* @internal snetCht3TtiProcessCnc function
* @endinternal
*
* @brief   TTI - process cnc
*
* @param[in] devObjPtr                - pointer to device object
* @param[in,out] descrPtr                 - pointer to packet descriptor
* @param[in,out] internalTtiInfoPtr       - (pointer to) internal TTI info
* @param[in,out] descrPtr                 - pointer to packet descriptor
*                                      RETURN:
*                                      COMMENTS:
*/
static GT_VOID snetCht3TtiProcessCnc
(
    IN    SKERNEL_DEVICE_OBJECT             *devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC   *descrPtr,
    INOUT INTERNAL_TTI_DESC_INFO_STC        *internalTtiInfoPtr
)
{
    DECLARE_FUNC_NAME(snetCht3TtiProcessCnc);

    GT_U32 isFirstActionCncEnabled = 0; /* disabled by default, used for first action only */
    GT_U32 cncIndexValue = 0;

    const GT_U32 bit14Set = 1 << 14;

    /* first lookup */
    if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        if(internalTtiInfoPtr->lookupsInfo[0].isValid &&
           internalTtiInfoPtr->lookupsInfo[0].actionXcatPtr->bindToCncCounter)
        {
            isFirstActionCncEnabled = 1;
            cncIndexValue = internalTtiInfoPtr->lookupsInfo[0].actionXcatPtr->cncCounterIndex;
        }
    }
    else if(internalTtiInfoPtr->cncIndex != SMAIN_NOT_VALID_CNS) /* not SIP5 devices */
    {
        isFirstActionCncEnabled = 1;
        cncIndexValue = internalTtiInfoPtr->cncIndex;
    }

    if(isFirstActionCncEnabled)
    {
        __LOG(("CNC - TTI client (first lookup), index[%d]", cncIndexValue));
        snetCht3CncCount(devObjPtr, descrPtr, SNET_CNC_CLIENT_TTI_ACTION_0_E, cncIndexValue);
    }

    /* second lookup */
    if (internalTtiInfoPtr->lookupsInfo[1].isValid &&
        internalTtiInfoPtr->lookupsInfo[1].actionXcatPtr->bindToCncCounter)
    {
        cncIndexValue = internalTtiInfoPtr->lookupsInfo[1].actionXcatPtr->cncCounterIndex;
        __LOG(("CNC - TTI client (second lookup), index[%d]", cncIndexValue));
        snetCht3CncCount(devObjPtr, descrPtr, SNET_CNC_CLIENT_TTI_ACTION_1_E, cncIndexValue);
    }

    if(SMEM_CHT_IS_SIP5_10_GET(devObjPtr))
    {
        /* third lookup */
        if (internalTtiInfoPtr->lookupsInfo[2].isValid &&
            internalTtiInfoPtr->lookupsInfo[2].actionXcatPtr->bindToCncCounter)
        {
            cncIndexValue = internalTtiInfoPtr->lookupsInfo[2].actionXcatPtr->cncCounterIndex;
            __LOG(("CNC - TTI client (third lookup), index[%d]", cncIndexValue));

            if(!SMEM_CHT_IS_SIP5_15_GET(devObjPtr))
            {
                /* SIP5.10: bit 14 used as indication of lookup 2 (client 0) */
                cncIndexValue |= bit14Set;
                __LOG(("CNC index with bit 14, index[%d]", cncIndexValue));

                snetCht3CncCount(devObjPtr, descrPtr, SNET_CNC_CLIENT_TTI_ACTION_0_E/*client 0*/, cncIndexValue);
            }
            else
            if(!SMEM_CHT_IS_SIP5_20_GET(devObjPtr))
            {
                /* SIP5.15: lookup 2 (client 0) accesses only blocks 16..31 */
                /* For simulation purposes, we will communicate this as client "TTI_ACTION_2" to CNC */
                __LOG(("SIP5.15: TTI lookup #2 to access higher blocks, index[%d]", cncIndexValue));

                snetCht3CncCount(devObjPtr, descrPtr, SNET_CNC_CLIENT_TTI_ACTION_2_E/*client 0 actually*/, cncIndexValue);
            }
            else
            {
                /* SIP5.20: Normal implementation */
                snetCht3CncCount(devObjPtr, descrPtr, SNET_CNC_CLIENT_TTI_ACTION_2_E, cncIndexValue);
            }
        }

        /* fourth lookup */
        if (internalTtiInfoPtr->lookupsInfo[3].isValid &&
            internalTtiInfoPtr->lookupsInfo[3].actionXcatPtr->bindToCncCounter)
        {
            cncIndexValue = internalTtiInfoPtr->lookupsInfo[3].actionXcatPtr->cncCounterIndex;
            __LOG(("CNC - TTI client (fourth lookup), index[%d]", cncIndexValue));

            if(!SMEM_CHT_IS_SIP5_15_GET(devObjPtr))
            {
                /* SIP5.10: bit 14 used as indication of lookup 3 (client 1) */
                cncIndexValue |= bit14Set;
                __LOG(("CNC index with bit 14, index[%d]", cncIndexValue));

                snetCht3CncCount(devObjPtr, descrPtr, SNET_CNC_CLIENT_TTI_ACTION_1_E/*client 1*/, cncIndexValue);
            }
            else
            if(!SMEM_CHT_IS_SIP5_20_GET(devObjPtr))
            {
                /* SIP5.15: lookup 3 (client 1) accesses only blocks 16..31 */
                /* For simulation purposes, we will communicate this as client "TTI_ACTION_3" to CNC */
                __LOG(("SIP5.15: TTI lookup #3 to access higher blocks, index[%d]", cncIndexValue));

                snetCht3CncCount(devObjPtr, descrPtr, SNET_CNC_CLIENT_TTI_ACTION_3_E/*client 1 actually*/, cncIndexValue);
            }
            else
            {
                /* SIP5.20: Normal implementation */
                snetCht3CncCount(devObjPtr, descrPtr, SNET_CNC_CLIENT_TTI_ACTION_3_E, cncIndexValue);
            }

        }
    }
}

/**
* @internal snetCht3TtiUnitPart2 function
* @endinternal
*
* @brief   TTI unit part 2, this section may be called from the 'Descriptor Duplication Engine'
*         so it need to be separated from the 'part 1'
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] descrPtr                 - pointer to frame data buffer Id
* @param[in,out] descrPtr                 - pointer to updated frame data buffer Id
*                                      RETURN:
*                                      COMMENTS:
*/
static GT_VOID snetCht3TtiUnitPart2
(
    IN    SKERNEL_DEVICE_OBJECT             *devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC   *descrPtr,
    OUT   INTERNAL_TTI_DESC_INFO_STC        *internalTtiInfoPtr
)
{
    DECLARE_FUNC_NAME(snetCht3TtiUnitPart2);

    GT_U32  regAddress;
    GT_U32  regValue;
    GT_U32  popBytesFromAction;/*number of bytes that we got during 'tti action' processing */
    SKERNEL_PORTS_BMP_STC sourcePortsBmp; /* Source ports bitmap */
    GT_U32  srcPortBpeEnable;
    GT_BIT  tag1_exists, tag1_is_outer;
    GT_U32  bpeNonBpeVidxPrefix; /* Default origVid[1:0] assignment when BPE enable 12K MC groups is enabled
                                    however BPE is disabled per port prefix */
    GT_BIT bpeMcGroups;
    GT_U32 * regPtr;            /* Register pointer */

    if(SMEM_CHT_IS_SIP7_GET(devObjPtr))
    {
        /* from IAS : Assign desc<Tag0 VID> = eVLAN assignment before TTI Lookup/Action */
        descrPtr->tag0Vid = descrPtr->eVid & 0xFFF;/* only 12 bits for the field in the descriptor */
        __LOG(("Sip7 : Assign desc<Tag0 VID>[0x%3.3x] from 12 LSBits of eVLAN [0x%3.3x] assignment before TTI Lookup/Action \n",
            descrPtr->tag0Vid,
            descrPtr->eVid));

        __LOG_PARAM(descrPtr->tag0Vid);
    }

     /* ip-to-me lookup */
    if(SMEM_CHT_IS_SIP6_30_GET(devObjPtr))
    {
        __LOG(("SIP6_30: ip-to-me lookup \n"));
        snetSip6_30Ip2MeTblLookUp(devObjPtr,descrPtr);
    }

    if(SMEM_CHT_IS_SIP7_GET(devObjPtr))
    {
        GT_U32  Generic_Classification_Field_default_value_lsb_location;
        GT_U32  Generic_Classification_Field_default_value;
        __LOG(("SIP7: generic classification field initialization \n"));

        smemRegFldGet(devObjPtr,
            SMEM_SIP6_30_TTI_UNIT_GLOBAL_CONFIG_EXT3_REG(devObjPtr),
            3, 7,
            &Generic_Classification_Field_default_value_lsb_location);
        __LOG_PARAM(Generic_Classification_Field_default_value_lsb_location);

        Generic_Classification_Field_default_value =
            SMEM_LION3_TTI_DEFAULT_EPORT_ENTRY_FIELD_GET(devObjPtr, descrPtr,
                SMEM_SIP7_TTI_DEFAULT_EPORT_TABLE_FIELDS_GENERIC_CLASSIFICATION_FIELD_DEFAULT_VALUE_E);

        snetFieldValueSet(descrPtr->genericClassification,
            Generic_Classification_Field_default_value_lsb_location,/*start bit*/
            10,/*numBits*/
            Generic_Classification_Field_default_value);

        if(simLogIsOpenFlag)
        {
            __LOG_PARAM(descrPtr->genericClassification[0]);
            __LOG_PARAM(descrPtr->genericClassification[1]);
            __LOG_PARAM(descrPtr->genericClassification[2]);
            __LOG_PARAM(descrPtr->genericClassification[3]);
        }
    }

    /* call the main TTI unit engine */
    __LOG(("call the main TTI unit engine \n"));
    snetCht3TTerminationMain(devObjPtr,descrPtr,internalTtiInfoPtr);

    if (SMEM_CHT_IS_SIP6_GET(devObjPtr))
    {
        /* TTI exception check */
        snetSip6CutThroughTtiExceptionCheck(
            devObjPtr, descrPtr);
    }


    if(devObjPtr->errata.l2echoValidityForCfmInTti)
    {
        if( (0 == descrPtr->VntL2Echo) && (GT_TRUE == descrPtr->cfm) )
        {
            __LOG(("Warning (Errata): Descriptor <CFM Packet> field is valid only if <L2 Echo> is set\n"));
            descrPtr->cfm = GT_FALSE;
        }
    }

    /* do post TTI lookup actions */
    __LOG(("do post TTI lookup actions"));

    if(devObjPtr->supportEArch && devObjPtr->unitEArchEnable.tti)
    {
        if(internalTtiInfoPtr->trillInfo.reassignsTheSourceEPort &&
           internalTtiInfoPtr->ttiActionReassignsTheSourceEPort == 0)/* not already set by TTI action */
        {
            /* need to update the descriptor only after the TTI lookup (build of the key) */
            snetLion3IngressReassignSrcEPort(devObjPtr,descrPtr,"TRILL" ,
                internalTtiInfoPtr->trillInfo.newSrcEPort);
        }
        else if(descrPtr->centralizedChassisModeEn)
        {
            /* the OrigSrcDev should be set to 'own' at this stage after TTI lookup */
            descrPtr->srcDev = descrPtr->ownDev;
            descrPtr->srcDevIsOwn = 1;
        }

        /* tti - Post-TTI Lookup Ingress ePort Table */
        __LOG(("tti - Post-TTI Lookup Ingress ePort Table \n"));
        regAddress = SMEM_LION2_TTI_POST_TTI_LOOKUP_INGRESS_EPORT_TABLE_TBL_MEM(devObjPtr,
                descrPtr->eArchExtInfo.localDevSrcEPort);
        descrPtr->eArchExtInfo.ttiPostTtiLookupIngressEPortTablePtr = smemMemGet(devObjPtr, regAddress);

        if(descrPtr->myPhyPortAssignMode &&
           devObjPtr->errata.ttiTemotePhysicalPortOrigSrcPhyIsTrunk == 0)/*fix for the Erratum */
        {
            /* init PhySrcMCFilteringEn. for all those that considered 'local dev ports' */
            descrPtr->eArchExtInfo.phySrcMcFilterEn =
                SMEM_LION3_TTI_EPORT_ATTRIBUTES_ENTRY_FIELD_GET(devObjPtr,descrPtr,
                    SMEM_LION3_TTI_EPORT_ATTRIBUTES_TABLE_FIELDS_PHY_SRC_MC_FILTERING_EN);


            __LOG(("init PhySrcMCFilteringEn [%d]. for all 'Remote physical ports' that considered 'local dev ports' \n",
                                          descrPtr->eArchExtInfo.phySrcMcFilterEn));
        }
        else
        /* the FORWARD eDSA already assigned value */
        if(!(descrPtr->marvellTagged &&
             descrPtr->marvellTaggedExtended == SKERNEL_EXT_DSA_TAG_4_WORDS_E &&
             descrPtr->incomingMtagCmd == SKERNEL_MTAG_CMD_FORWARD_E))
        {
            /* init PhySrcMCFilteringEn. for non DSA or DSA != eDSA */
            descrPtr->eArchExtInfo.phySrcMcFilterEn =
                SMEM_LION3_TTI_EPORT_ATTRIBUTES_ENTRY_FIELD_GET(devObjPtr,descrPtr,
                    SMEM_LION3_TTI_EPORT_ATTRIBUTES_TABLE_FIELDS_PHY_SRC_MC_FILTERING_EN);


            __LOG(("init PhySrcMCFilteringEn [%d]. for non DSA or DSA != eDSA \n",
                                          descrPtr->eArchExtInfo.phySrcMcFilterEn));
        }

        /* hold value that we got when processed the action .
            see function snetXcatTTActionApplyEarchTti(...)
           NOTE: that the L2 re-parsing is done only if there was TTI action
        */
        popBytesFromAction = descrPtr->numOfBytesToPop;

        /*Number of tags/bytes to Pop*/
        regValue =
                SMEM_LION3_TTI_EPORT_ATTRIBUTES_ENTRY_FIELD_GET(devObjPtr,descrPtr,
                    SMEM_LION3_TTI_EPORT_ATTRIBUTES_TABLE_FIELDS_NUM_OF_TAGS_TO_POP);

        /*Number of bytes to Pop*/
        descrPtr->numOfBytesToPop =
            SNET_CONVERT_POP_TAGS_FEILD_TO_BYTES_MAC(devObjPtr,regValue);

        __LOG_PARAM(descrPtr->numOfBytesToPop);

        if(descrPtr->numOfBytesToPop)
        {
            descrPtr->popTagsWithoutReparse = 1;

            if(popBytesFromAction == 0)
            {
                __LOG(("WARNING : NOT doing re-parsing for 'NUM_OF_TAGS_TO_POP' ([%d] bytes) because no TTI action matched action \n",
                    popBytesFromAction));
            }
            else
            if(descrPtr->tunnelTerminated &&
               devObjPtr->errata.ttiTtAndPopTagNeedToBeSupportedConcurrently)
            {
                /* JIRA : TTI-1077 : TT and POP Tag need to be supported concurrently */
                __LOG(("WARNING : NOT doing re-parsing for 'NUM_OF_TAGS_TO_POP' ([%d] bytes) because TTI action already did <tunnel Termination> !!! \n",
                    popBytesFromAction));
            }
            else
            {
                GT_U32  preAction_eVid0       = descrPtr->eVid;/* save for restoring values */
                GT_U32  preAction_vidModified = descrPtr->vidModified;/* save for restoring values */

                /*NOTE: this action must be done only after we set the:
                    descrPtr->eArchExtInfo.ttiPostTtiLookupIngressEPortTablePtr
                    see snetXCatIngressVlanTagClassify(...) called from
                    snetChtL2Parsing(...)
                */
                descrPtr->popTagsWithoutReparse = 0;

                /* do re-parsing to pop the tag */
                __LOG(("do re-parsing to pop [%d] bytes \n",
                    descrPtr->numOfBytesToPop));
                snetChtL2Parsing(devObjPtr,descrPtr,SNET_CHT_FRAME_PARSE_MODE_FROM_TTI_PASSENGER_E,internalTtiInfoPtr);

                /* restore the vid0 before the mac to me lookup */
                if(internalTtiInfoPtr->vlan_actionInfo.vid0Cmd == 0)  /* fix JIRA : CPSS-8810 : Simulator: Different Pop 4B tag behavior */
                {
                    /* not allowing the re-parse to override the previous assignment */
                    __LOG(("The TTI action vid0Cmd force the eVlan to use the value before the 're-parse' ! \n"
                           "so the value [0x%x] is ignored and using[0x%x] \n",
                           descrPtr->eVid,
                           preAction_eVid0));

                    /* restore vid0 as was before the L2 parsing */
                    descrPtr->eVid = preAction_eVid0;
                    descrPtr->vidModified = preAction_vidModified;
                }

                /* do actions on vlan related command according to 'after pop' */
                snetXcatTTActionApplyVlanTagCmd(devObjPtr, descrPtr,internalTtiInfoPtr);

                /* mac-to-me lookup -- on the passenger */
                __LOG(("mac-to-me lookup -- on the passenger \n"));
                snetCht3Mac2MeTblLookUp(devObjPtr, descrPtr);
            }
        }

    }

    /* it seems that at least in sip6 the CNC count even for those errors */
    if (SMEM_CHT_IS_SIP6_GET(devObjPtr) || ( !internalTtiInfoPtr->preTunnelTerminationInfo.ipHeaderError &&
        !internalTtiInfoPtr->preTunnelTerminationInfo.ipTtiHeaderError &&
        !internalTtiInfoPtr->preTunnelTerminationInfo.greHeaderError))

    {
        /* process tti clients cnc */
        snetCht3TtiProcessCnc(devObjPtr, descrPtr, internalTtiInfoPtr);
    }
    else
    {
        __LOG(("WARNING : no CNC for one of : 'ipHeaderError'[%d] or ipTtiHeaderError[%d] or greHeaderError[%d] \n",
            internalTtiInfoPtr->preTunnelTerminationInfo.ipHeaderError    ,
            internalTtiInfoPtr->preTunnelTerminationInfo.ipTtiHeaderError ,
            internalTtiInfoPtr->preTunnelTerminationInfo.greHeaderError
        ));
    }

    if(descrPtr->vid0Or1AfterTti == SMAIN_NOT_VALID_CNS)
    {
        /* save the 'orig Vid0/1' at the end of TTI */
        if(SKERNEL_IS_LION2_DEV(devObjPtr))
        {
            /* Orig VID Mode */
            smemRegFldGet(devObjPtr, SMEM_XCAT_A1_TTI_UNIT_GLB_CONF_REG(devObjPtr),2,1,&regValue);
            descrPtr->vid0Or1AfterTti = (regValue == 0) ?
                                descrPtr->eVid :
                                descrPtr->vid1;

            if(descrPtr->vid0Or1AfterTti >= 0x1000)
            {
                __LOG(("the Orig VID is NEVER MORE the 12 bits \n"));
                descrPtr->vid0Or1AfterTti &= 0xFFF;
            }

            __LOG(("end of TTI unit save the Orig VID [%d] from [%s]  \n",
                            descrPtr->vid0Or1AfterTti,
                            (regValue == 0)?"vid0":"vid1"));
        }
        else
        {
            descrPtr->vid0Or1AfterTti = descrPtr->eVid;
            __LOG(("end of TTI unit save the Orig VID [%d]",
                            descrPtr->vid0Or1AfterTti));
        }
    }

    __LOG_PARAM(descrPtr->vid0Or1AfterTti);

    if (devObjPtr->support802_1br_PortExtender) /* device support tag1 recognition as 'ETag' */
    {
        smemRegFldGet(devObjPtr,
                SMEM_XCAT_TTI_ENGINE_CONFIG_REG(devObjPtr), 14, 2, &bpeNonBpeVidxPrefix);
        smemRegFldGet(devObjPtr,
                SMEM_XCAT_TTI_ENGINE_CONFIG_REG(devObjPtr), 13, 1, &bpeMcGroups);

        /* Get Source Port <BPE enabled> per port */
        regPtr = smemMemGet(devObjPtr, SMEM_XCAT3_TTI_SOURCE_PORT_BPE_ENABLE_REG(devObjPtr));
        SKERNEL_FILL_PORTS_BITMAP_MAC(devObjPtr, &sourcePortsBmp, regPtr);
        srcPortBpeEnable = SKERNEL_PORTS_BMP_IS_PORT_SET_MAC(&sourcePortsBmp, descrPtr->localDevSrcPort);

        if (bpeMcGroups == 0)
        {
            if(descrPtr->firstVlanTagExtendedSize > 4)
            {
                __LOG(("BPE support : 'save' <GRP> in bits 0..1 of descrPtr->vid0Or1AfterTti \n"));
                SMEM_U32_SET_FIELD(descrPtr->vid0Or1AfterTti,0,2,
                        descrPtr->bpe802_1br_GRP);
                __LOG_PARAM(descrPtr->vid0Or1AfterTti);
            }

            tag1_is_outer = (descrPtr->ingressVlanTag1Type == SKERNEL_FRAME_TR101_VLAN_TAG_OUTER_E) ? 1 : 0;
            tag1_exists  = TAG1_EXIST_MAC(descrPtr);

            /* BPE is enabled at least on one of the source ports */
            if(0 == SKERNEL_PORTS_BMP_IS_EMPTY_MAC(devObjPtr, &sourcePortsBmp))
            {
                if(srcPortBpeEnable == 0 && (descrPtr->firstVlanTagExtendedSize <= 4))
                {
                    __LOG(("BPE support : 'save' bpeNonBpeVidxPrefix in bits 0..1 of descrPtr->vid0Or1AfterTti \n"));
                    SMEM_U32_SET_FIELD(descrPtr->vid0Or1AfterTti,0,2, bpeNonBpeVidxPrefix);
                    __LOG_PARAM(descrPtr->vid0Or1AfterTti);
                }

                __LOG(("BPE support : 'save' <tag1_is_outer> in bit 2 of descrPtr->vid0Or1AfterTti \n"));
                SMEM_U32_SET_FIELD(descrPtr->vid0Or1AfterTti,2,1,tag1_is_outer);
                __LOG(("BPE support : 'save' <tag1_exists> in bit 3 of descrPtr->vid0Or1AfterTti \n"));
                SMEM_U32_SET_FIELD(descrPtr->vid0Or1AfterTti,3,1,tag1_exists);

                __LOG_PARAM(descrPtr->vid0Or1AfterTti);
            }

            /* we should not use this value outside the TTI unit !!!! */
            descrPtr->bpe802_1br_GRP = 0;
        }
    }

    if(SMEM_CHT_IS_SIP6_10_GET(devObjPtr))
    {
        /* read ppu enable bit field from TTI Unit Global Configuration ext 2 register */
        smemRegFldGet(devObjPtr, SMEM_SIP6_TTI_UNIT_GLOBAL_CONFIG_EXT2_REG(devObjPtr),
                                  27, 1, &regValue);

        descrPtr->ttiPpuEnable = (GT_BOOL)(regValue & 0x1);
    }

    if(SMEM_CHT_IS_SIP6_GET(devObjPtr) &&
        descrPtr->vrfId == 0)/* do not overrun the TTI action assignment */
    {
        smemRegGet(devObjPtr,
            SMEM_SIP6_TTI_VRFID_EVLAN_MAPPING_TBL_MEM(devObjPtr,descrPtr->eVid),
            &regValue);

        descrPtr->vrfId = regValue & (SMEM_CHT_IS_SIP7_GET(devObjPtr) ? 0x7FFF : 0xFFF);

        __LOG(("Sip6 : (instead of L2i unit) TTI unit assign VRF-ID[%d] for vlanId[%d] \n",
            descrPtr->vrfId,
            descrPtr->eVid));

        __LOG_PARAM(descrPtr->vrfId);
    }

    if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        /* assign packet type for EPCL usage */
        /* the EPCL packet type , according to :
            1. NON TT --> the tunnel type
            2. TT --> the passenger type


           this is different from the IPCL packet type , according to :
            1. NON TT :
                if MPLS and (<TTI Unit Global Configuration<Enable X Over MPLS Parsing>> = SET
                    --> the passenger type
                else if NON MPLS and TTI_Action <Passenger Parsing of Non-MPLS Transit tunnel Enable>=1
                    --> the passenger type
                else
                    --> the tunnel type
            2. TT --> the passenger type

        */
        pcktTypeAssign(devObjPtr, descrPtr,PACKET_TYPE_USED_FOR_CLIENT_EPCL_E);
    }

    /* PTP Support */
    __LOG(("PTP Support"));
    snetLionPtpIngressTimestampProcess(devObjPtr, descrPtr);
    snetLionPtpCommandResolution(devObjPtr, descrPtr);
}

/**
* @internal snetLion3TtiMCDuplTriggerCheckMpls function
* @endinternal
*
* @brief   Multicast duplication engine mpls triggers check, relevant for SIP5 only
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] descrPtr                 - pointer to frame data buffer Id
* @param[in,out] internalTtiInfoPtr       - pointer to internal TTI info
* @param[in,out] descrPtr                 - pointer to frame data buffer Id
* @param[in,out] internalTtiInfoPtr       - pointer to internal TTI info
*
* @retval GT_BOOL           true   - doing duplication
* @retval false                    - no duplication
*/
static GT_BOOL snetLion3TtiMCDuplTriggerCheckMpls
(
    IN    SKERNEL_DEVICE_OBJECT             *devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC   *descrPtr,
    INOUT INTERNAL_TTI_DESC_INFO_STC        *internalTtiInfoPtr
)
{
    DECLARE_FUNC_NAME(snetLion3TtiMCDuplTriggerCheckMpls);

    GT_U8 mcDaMaskByte;
    GT_U8 mcDaByte;

    GT_U32 regAddr;
    GT_U32 regAddrMask;

    GT_U32 fldVal;
    GT_U32 bitNum;

    GT_U32 byteNum;


    if( ! SMEM_LION3_TTI_DEFAULT_EPORT_ENTRY_FIELD_GET(devObjPtr, descrPtr,
            SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_MPLS_MC_DUPLICATION_ENABLE))
    {
        __LOG(("disabled duplication for MPLS packets"));
        return GT_FALSE;
    }

    /*(Packet DA           &  global<MPLS MC DA Mask>) ==
      (global<MPLS MC DA>  &  global<MPLS MC DA Mask>)
            Trigger MPLS Multicast packet duplication */

    regAddr     = SMEM_LION3_TTI_MPLS_MC_DA_LOW_REG(devObjPtr);
    regAddrMask = SMEM_LION3_TTI_MPLS_MC_DA_LOW_MASK_REG(devObjPtr);

    for(byteNum = 0, bitNum = 0; byteNum < 6; byteNum++, bitNum += 8)
    {
        /* only 24 bits are used in the first register so check byteNum '3' and not '4' */
        if(byteNum == 3)
        {
            regAddr     = SMEM_LION3_TTI_MPLS_MC_DA_HIGH_REG(devObjPtr);
            regAddrMask = SMEM_LION3_TTI_MPLS_MC_DA_HIGH_MASK_REG(devObjPtr);
            bitNum = 0;
        }

        smemRegFldGet(devObjPtr, regAddr, bitNum, 8, &fldVal);
        mcDaByte = (GT_U8)fldVal;

        smemRegFldGet(devObjPtr, regAddrMask, bitNum, 8, &fldVal);
        mcDaMaskByte = (GT_U8)fldVal;

        __LOG_PARAM(mcDaByte);
        __LOG_PARAM(mcDaMaskByte);
        __LOG_PARAM( descrPtr->macDaPtr[(5 - byteNum)] );

        if( (descrPtr->macDaPtr[5 - byteNum] & mcDaMaskByte) != (mcDaByte & mcDaMaskByte) )
        {
            __LOG(("byte num %d doesn't match", byteNum));
            return GT_FALSE;
        }
    }

    __LOG(("MAC DA considered MPLS MultiCast MAC"));
    return GT_TRUE;
}

/**
* @internal snetLion3TtiMCDuplTriggerCheckIP function
* @endinternal
*
* @brief   Multicast duplication engine ip triggers check, relevant for SIP5 only
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] descrPtr                 - pointer to frame data buffer Id
* @param[in,out] internalTtiInfoPtr       - pointer to internal TTI info
* @param[in,out] descrPtr                 - pointer to frame data buffer Id
* @param[in,out] internalTtiInfoPtr       - pointer to internal TTI info
*
* @retval GT_BOOL           true   - doing duplication
* @retval false                    - no duplication
*/
static GT_BOOL snetLion3TtiMCDuplTriggerCheckIP
(
    IN    SKERNEL_DEVICE_OBJECT             *devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC   *descrPtr,
    INOUT INTERNAL_TTI_DESC_INFO_STC        *internalTtiInfoPtr
)
{
    DECLARE_FUNC_NAME(snetLion3TtiMCDuplTriggerCheckIP);

    GT_U32                                          udpPort;
    SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_ENT  modeField;

    if(descrPtr->ipm == 0)
    {
        __LOG(("Not multicast IP traffic"));
        return GT_FALSE;
    }

    modeField = descrPtr->isIPv4 ?
                    SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_IPV4_MC_DUPLICATION_MODE :
                    SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_IPV6_MC_DUPLICATION_MODE;

    switch( SMEM_LION3_TTI_DEFAULT_EPORT_ENTRY_FIELD_GET(devObjPtr, descrPtr, modeField) )
    {
        case 0:
            __LOG(("No duplication performed"));
            return GT_FALSE;

        case 1:
            __LOG(("Trigger IPv4/6 multicast duplication"));
            return GT_TRUE;

        case 2:
            if(SNET_IP_GRE_PROT_E == descrPtr->ipProt &&
               GT_TRUE == snetXcatGetGreEtherTypes(devObjPtr, descrPtr, NULL, NULL, NULL))
            {
                __LOG(("enabled duplication for matching GRE protocol only"));
                return GT_TRUE;
            }
            else
            {
                __LOG(("not GRE or not matching GRE protocol"));
                return GT_FALSE;
            }
        case 3:
            if(SNET_IP_GRE_PROT_E == descrPtr->ipProt)
            {
                __LOG(("enabled duplication for any GRE protocol only"));
                return GT_TRUE;
            }
            else
            {
                __LOG(("packet is not gre"));
                return GT_FALSE;
            }
        case 4:
            if(internalTtiInfoPtr->ttiKeyType != (SKERNEL_LION3_TTI_KEY_UDB_PKT_TYPE_0_E + SIP5_PACKET_CLASSIFICATION_TYPE_IPV4_UDP_E) &&
               internalTtiInfoPtr->ttiKeyType != (SKERNEL_LION3_TTI_KEY_UDB_PKT_TYPE_0_E + SIP5_PACKET_CLASSIFICATION_TYPE_IPV6_UDP_E))
            {
                __LOG(("not UDP key"));
                return GT_FALSE;
            }

            if(descrPtr->udpCompatible)
            {
                smemRegFldGet(devObjPtr, SMEM_LION3_TTI_DUPLICATION_CONFIGS1_REG(devObjPtr), 0, 16, &udpPort);
                if(descrPtr->l4DstPort == udpPort)
                {
                    __LOG(("enabled duplication for matching UDP protocol only"));
                    return GT_TRUE;
                }
            }

            __LOG(("not UDP or not matching UDP protocol"));
            return GT_FALSE;

        case 5:
            if(internalTtiInfoPtr->ttiKeyType != (SKERNEL_LION3_TTI_KEY_UDB_PKT_TYPE_0_E + SIP5_PACKET_CLASSIFICATION_TYPE_IPV4_UDP_E) &&
               internalTtiInfoPtr->ttiKeyType != (SKERNEL_LION3_TTI_KEY_UDB_PKT_TYPE_0_E + SIP5_PACKET_CLASSIFICATION_TYPE_IPV6_UDP_E))
            {
                __LOG(("not UDP key"));
                return GT_FALSE;
            }

            if(descrPtr->udpCompatible)
            {
                __LOG(("enabled duplication for any UDP protocol only"));
                return GT_TRUE;
            }

            __LOG(("not UDP"));
            return GT_FALSE;

        case 6: /* reserved */
        case 7: /* reserved */
        default:
            break;
    }

    return GT_FALSE;
}

/**
* @internal snetLion3TtiMCDuplTriggerCheck function
* @endinternal
*
* @brief   Multicast duplication engine triggers check, relevant for SIP5 only
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] descrPtr                 - pointer to frame data buffer Id
* @param[in,out] internalTtiInfoPtr       - pointer to internal TTI info
* @param[in,out] descrPtr                 - pointer to frame data buffer Id
* @param[in,out] internalTtiInfoPtr       - pointer to internal TTI info
*
* @retval GT_BOOL           true   - doing duplication
* @retval false                    - no duplication
*/
static GT_BOOL snetLion3TtiMCDuplTriggerCheck
(
    IN    SKERNEL_DEVICE_OBJECT             *devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC   *descrPtr,
    INOUT INTERNAL_TTI_DESC_INFO_STC        *internalTtiInfoPtr
)
{
    DECLARE_FUNC_NAME(snetLion3TtiMCDuplTriggerCheck);

    if(GT_FALSE == snetXcatTTTriggeringCheck(devObjPtr, descrPtr, internalTtiInfoPtr))
    {
        __LOG(("The packet not subject to TTI lookup"));
        return GT_FALSE;
    }

    logTtiKeyType(devObjPtr, internalTtiInfoPtr->ttiKeyType);

    /*   The device supports MC tunnel packet duplication
            for the following Multicast tunnel packet types:
                - Multicast TRILL
                - Multicast PBB
                - Multicast IPv4/6
                - Multicast MPLS
    */

    if(descrPtr->trillEngineTriggered && descrPtr->trillInfo.M)
    { /* TRILL */
        if(SMEM_LION3_TTI_DEFAULT_EPORT_ENTRY_FIELD_GET(devObjPtr, descrPtr,
                SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_TRILL_MC_DUPLICATION_ENABLE))
        {
            __LOG(("Multicast TRILL Duplication is Enabled \n"));
            return GT_TRUE;
        }
        else
        {
            __LOG(("Multicast TRILL Duplication is Disabled \n"));
        }
    }
    /* SKERNEL_XCAT_TTI_KEY_MIM_E == internalTtiInfoPtr->ttiKeyType */
    /* this condition is not exact, TTI can treat MIM packets       */
    /* using other packet type, for example UDE-UDE6                */
    else if((descrPtr->macDaType != SKERNEL_UNICAST_MAC_E /*support also broadcast MAC*/) &&
            (descrPtr->mim != 0))
    { /* MIM */

        if(SMEM_LION3_TTI_DEFAULT_EPORT_ENTRY_FIELD_GET(devObjPtr, descrPtr,
                SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_PBB_MC_DUPLICATION_ENABLE))
        {
            __LOG(("Multicast PBB Duplication is Enabled \n"));
            return GT_TRUE;
        }
        else
        {
            __LOG(("Multicast PBB Duplication is Disabled \n"));
        }
    }
    else if(descrPtr->isIp)
    { /* IP */
        if(GT_TRUE == snetLion3TtiMCDuplTriggerCheckIP(devObjPtr, descrPtr, internalTtiInfoPtr))
        {
            __LOG(("Multicast IP Duplication is Enabled \n"));
            return GT_TRUE;
        }
        else
        {
            __LOG(("Multicast IP Duplication not applied \n"));
        }
    }
    else if((SKERNEL_LION3_TTI_KEY_UDB_PKT_TYPE_0_E + SIP5_PACKET_CLASSIFICATION_TYPE_MPLS_E) ==
                 internalTtiInfoPtr->ttiKeyType)
    { /* MPLS */
        if(GT_TRUE == snetLion3TtiMCDuplTriggerCheckMpls(devObjPtr, descrPtr, internalTtiInfoPtr))
        {
            __LOG(("Multicast MPLS Duplication is Enabled \n"));
            return GT_TRUE;
        }
        else
        {
            __LOG(("Multicast MPLS Duplication not applied \n"));
        }
    }
    else
    {
        __LOG(("Current packet type is not subject to TTI MC duplication engine \n"));
    }

    return GT_FALSE;
}

/**
* @internal snetLion3TtiMulticastDuplicationEngine function
* @endinternal
*
* @brief   Multicast duplication engine, relevant for SIP5 only
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] descrPtr                 - pointer to frame data buffer Id
* @param[in,out] internalTtiInfoPtr       - pointer to internal TTI info
* @param[in,out] descrPtr                 - pointer to frame data buffer Id
* @param[in,out] internalTtiInfoPtr       - pointer to internal TTI info
*/
static GT_VOID snetLion3TtiMulticastDuplicationEngine
(
    IN    SKERNEL_DEVICE_OBJECT             *devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC   *descrPtr,
    INOUT INTERNAL_TTI_DESC_INFO_STC        *internalTtiInfoPtr
)
{
    DECLARE_FUNC_NAME(snetLion3TtiMulticastDuplicationEngine);

    SKERNEL_FRAME_CHEETAH_DESCR_STC *dupDescrPtr;/* duplicated descriptor pointer */
    INTERNAL_TTI_DESC_INFO_STC       dupTtiInfo; /* duplicated TTI info */

    /* trigger requirements */
    if ( GT_FALSE == snetLion3TtiMCDuplTriggerCheck(devObjPtr, descrPtr, internalTtiInfoPtr))
    {
        __LOG(("TTI MC duplication Engine : the packet not subject to duplication : 'not trigger' \n"));
        return;
    }

    if(descrPtr->packetCmd != SKERNEL_EXT_PKT_CMD_FORWARD_E &&
       descrPtr->packetCmd != SKERNEL_EXT_PKT_CMD_MIRROR_TO_CPU_E)
    {
        __LOG(("TTI MC duplication Engine : the packet not subject to duplication : not 'FORWARD' or 'MIRROR_TO_CPU' \n"));
        return;
    }

    __LOG(("TTI MC duplication Engine : Start duplication \n"));
    /*
        Each descriptor instance is subject to TTI Lookup.
        The TTI distinguishes between the two instances using the key field:
        <TTI MC Duplication Instance>
        The 1st descriptor instance has key field assignment <TTI MC Duplication Instance> = 0
        The 2nd descriptor instance has key field assignment <TTI MC Duplication Instance> = 1
    */

    /***************************************************************/
    __LOG(("Duplicate the packet descriptor from the 'original' \n"));
    dupDescrPtr = snetChtEqDuplicateDescr(devObjPtr,descrPtr);

    __LOG(("Duplicate the internal TTI info \n"));
    dupTtiInfo = *internalTtiInfoPtr;

    dupDescrPtr->isMultiTargetReplication = 1;
    __LOG_PARAM(dupDescrPtr->isMultiTargetReplication);

    __LOG(("The 1st descriptor instance has key field assignment <TTI MC Desc Instance> = 0 \n"));
    dupDescrPtr->ttiMcDescInstance = 0;
    __LOG_PARAM(dupDescrPtr->ttiMcDescInstance);

    __LOG(("allow 1st descriptor instance to finish the TTI unit \n"));
    snetCht3TtiUnitPart2(devObjPtr, dupDescrPtr, &dupTtiInfo);

    if( dupTtiInfo.cncIndex != SMAIN_NOT_VALID_CNS &&
        (dupTtiInfo.actionInfo.packetCmd == SKERNEL_EXT_PKT_CMD_TRAP_TO_CPU_E ||
         dupTtiInfo.actionInfo.packetCmd == SKERNEL_EXT_PKT_CMD_HARD_DROP_E ||
         dupTtiInfo.actionInfo.packetCmd == SKERNEL_EXT_PKT_CMD_SOFT_DROP_E))
    {
        /* If Action Command is TRAP(2), HARD-DROP(3), or SOFT-DROP(4)
            and the packet is TTI Multi-Target, the second instance of this
            descriptor must be assigned HARD-DROP */
        __LOG(("the second instance of this descriptor must be assigned HARD-DROP"));
        descrPtr->packetCmd = SKERNEL_EXT_PKT_CMD_HARD_DROP_E;
    }

    __LOG(("allow the 1st descriptor instance to continue the 'Ingress pipe' (after the TTI)"));
    snetChtIngressAfterTti(devObjPtr, dupDescrPtr);

    simLogPacketFrameUnitSet(SIM_LOG_FRAME_UNIT_TTI_E);

    /* we are done with the '1st descriptor instance' so no more using: dupDescrPtr, dupTtiInfo */
    /***************************************************************/

    /* fall through ...
       it will continue the TTI unit and the ingress pipe for the 2nd descriptor instance ...*/
    /* state the also this is 'replication' (although) we use the original descriptor --> needed for PCL key */
    descrPtr->isMultiTargetReplication = 1;
    __LOG_PARAM(descrPtr->isMultiTargetReplication);

    __LOG(("The 2nd descriptor instance has key field assignment <TTI MC Desc Instance> = 1"));
    descrPtr->ttiMcDescInstance = 1;
    __LOG_PARAM(descrPtr->ttiMcDescInstance);

    if(descrPtr->mim)
    {
        /* fix JIRA : [CPSS-5031] : BC2 Simulation - In PBB Multicast Duplication the MAC2ME is not set */
        __LOG(("In PBB Multicast Duplication the MAC2ME is set for 'instance 1' .\n"
               "implicit set of metadata <MAC2ME> to 1 , to allow the same TTI rule that is used \n"
               "to classify unicast PBB tunnel-termination to be shared with the PBB Multicast packet duplication instance 1 \n"));
        /*  The second PBB packet instance has the metadata field <MAC2ME> set.
            The motivation to set the <MAC2ME> field is to allow the same TTI rule that is used to classify
            Unicast PBB tunnel-termination to be shared with the PBB Multicast packet duplication instance 1,
            which is the Multicast tunnel-terminated packet instance. */
        internalTtiInfoPtr->metadata.mac2me = 1;
        LOG_FIELD_VALUE("TTI metadata:<MAC2ME>",internalTtiInfoPtr->metadata.mac2me);
    }


}

/**
* @internal snetCht3TTermination function
* @endinternal
*
* @brief   T.T Engine processing for outgoing frame on Cheetah3
*         asic simulation.
*         T.T processing , T.T assignment ,key forming , 1 Lookup ,
*         actions to descriptor processing
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] descrPtr                 - pointer to frame data buffer Id
* @param[in] internalTtiInfoPtr       -  pointer to internal TTI info
* @param[in,out] descrPtr                 - pointer to updated frame data buffer Id
*                                      RETURN:
*                                      COMMENTS:
*                                      T.T has only one lookup cycle.
*                                      D.12.8.3 : T.T Registers
*
* @note T.T has only one lookup cycle.
*       D.12.8.3 : T.T Registers
*
*/
GT_VOID snetCht3TTermination
(
    IN    SKERNEL_DEVICE_OBJECT             * devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC   * descrPtr,
    IN    INTERNAL_TTI_DESC_INFO_STC        * internalTtiInfoPtr/* internal TTI info */
)
{
    DECLARE_FUNC_NAME(snetCht3TTermination);

    if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {   /* assign packet type for TTI usage */
        pcktTypeAssign(devObjPtr, descrPtr,PACKET_TYPE_USED_FOR_CLIENT_TTI_E);
    }

    /* initialize the cnc index to not valid index */
    __LOG(("initialize the cnc index to not valid index"));
    internalTtiInfoPtr->cncIndex   = SMAIN_NOT_VALID_CNS;

    /* initialize the origVid to not valid value */
    __LOG(("initialize the origVid to not valid value"));
    descrPtr->vid0Or1AfterTti = SMAIN_NOT_VALID_CNS;
    if(devObjPtr->vplsModeEnable.tti)
    {
        /* If the TTI AE <PW Tag Mode[1:0]> 2 bits == 0x0 (Disabled) || No TTI Match */
        /* For AC traffic which doesn't need TTI lookup, the OrigVID is set by the IPCL Action */
        /* - DescOut<OrigVID> = 0x0 */
        descrPtr->vid0Or1AfterTti = 0;
    }

    /* mac-to-me lookup */
    __LOG(("mac-to-me lookup"));
    snetCht3Mac2MeTblLookUp(devObjPtr, descrPtr);

    /* TRILL support */
    __LOG(("TRILL support"));
    snetLion2TrillEngine(devObjPtr, descrPtr,internalTtiInfoPtr);

    if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        /* Multicast Tunnel Duplication Engine */
        snetLion3TtiMulticastDuplicationEngine(devObjPtr, descrPtr, internalTtiInfoPtr);
    }

    /* the 'Part 2' need to be called after the TRILL engine because the trill
       may do : 'Descriptor Duplication Engine' */
    snetCht3TtiUnitPart2(devObjPtr, descrPtr,internalTtiInfoPtr);
}


/**
* @internal snetLion2TTOamDescriptorSet function
* @endinternal
*
* @brief   Set OAM relevant fields in descriptor
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] descrPtr                 - pointer to the frame descriptor.
* @param[in] ttiActionDataPtr         - pointer to TTI action
*                                      OUTPUT:
* @param[in,out] descrPtr                 - pointer to the frame descriptor.
*
* @note When the TTI enables OAM processing on a packet, the OAM Opcode
*       and MEG level are later inserted into the descriptor by the IPCL.
*
*/
static GT_VOID snetLion2TTOamDescriptorSet
(
    IN SKERNEL_DEVICE_OBJECT                 * devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC    * descrPtr,
    IN SNET_XCAT_TT_ACTION_STC               * ttiActionDataPtr
)
{
    DECLARE_FUNC_NAME(snetLion2TTOamDescriptorSet);

    GT_U32 regAddr;                         /* Register address */
    GT_U32 * regPtr;                        /* Register pointer */
    GT_U32 fldValue;                        /* Register field value */

    if(devObjPtr->oamSupport.oamSupport == 0)
    {
        /* Device doesn't support OAM */
        __LOG(("Device doesn't support OAM"));
        return;
    }

    if(ttiActionDataPtr)
    {
        if(ttiActionDataPtr->oamInfo.ptpTimeStampEnable)
        {
            descrPtr->oamInfo.timeStampEnable = 1;
            descrPtr->oamInfo.offsetIndex = ttiActionDataPtr->oamInfo.ptpTimeStampOffsetIndex;
        }
        else
        {
            descrPtr->oamInfo.timeStampEnable = 0;
            descrPtr->oamInfo.offsetIndex = 0;
        }

        if(ttiActionDataPtr->oamInfo.oamProcessEnable)
        {
            descrPtr->oamInfo.oamProcessEnable = ttiActionDataPtr->oamInfo.oamProcessEnable;
            descrPtr->oamInfo.oamProfile = ttiActionDataPtr->oamInfo.oamProfile;
        }
        else
        {
            descrPtr->oamInfo.oamProcessEnable = 0;
        }
    }

    /* Timestamp tag parsing */
    __LOG(("Timestamp tag parsing"));
    regAddr = SMEM_LION2_TTI_TIMESTAMP_CONFIG_REG(devObjPtr);
    regPtr = smemMemGet(devObjPtr, regAddr);
    /* TBD: TS Tag Parsing Enable */
    fldValue = SMEM_U32_GET_FIELD(regPtr[0], 1, 1);
    if(fldValue)
    {
        /* The ethertype used to identify Timestamp Tag */
        __LOG(("The ethertype used to identify Timestamp Tag"));
        fldValue = SMEM_U32_GET_FIELD(regPtr[0], 16, 16);
        if(descrPtr->etherTypeOrSsapDsap == fldValue)
        {
            /* The 'normal' TS (non-hybrid) tag has the E bit - the msbit of the first byte of the payload */
            __LOG(("The 'normal' TS (non-hybrid) tag has the E bit - the msbit of the first byte of the payload"));
            descrPtr->oamInfo.timeStampTagged = (descrPtr->payloadPtr[0] >> 7) ? 2 : 1;
        }
    }
    else
    {
        /* TBD: Hybrid TS Tag Parsing Enable */
        __LOG(("TBD: Hybrid TS Tag Parsing Enable"));
        fldValue = SMEM_U32_GET_FIELD(regPtr[0], 0, 1);
        if(fldValue)
        {
            /* TBD: The ethertype used to identify Hybrid Timestamp Tag */
            fldValue = SMEM_U32_GET_FIELD(regPtr[1], 16, 16);
            if(descrPtr->etherTypeOrSsapDsap == fldValue)
            {
                descrPtr->oamInfo.timeStampTagged = 3;
            }
        }
        else
        {
            descrPtr->oamInfo.timeStampTagged = 0;
        }
    }
}

/* checks whether mpls lookup or not */
static GT_BOOL checkIsMplsLookup
(
    IN SKERNEL_DEVICE_OBJECT        * devObjPtr,
    IN INTERNAL_TTI_DESC_INFO_STC   * internalTtiInfoPtr
)
{
    if(SMEM_CHT_IS_SIP5_GET(devObjPtr) &&
       internalTtiInfoPtr->ttiKeyType == (SKERNEL_LION3_TTI_KEY_UDB_PKT_TYPE_0_E +
                                          SIP5_PACKET_CLASSIFICATION_TYPE_MPLS_E))
    {
        return GT_TRUE;
    }
    else if(internalTtiInfoPtr->ttiKeyType == SKERNEL_CHT3_TTI_KEY_MPLS_E)
    {
        return GT_TRUE;
    }
    else
    {
        return GT_FALSE;
    }
}

/**
* @internal passengerParsingOfTransitNonMplsTunnel function
* @endinternal
*
* @brief   parses transit non-mpls tunnel passenger
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] descrPtr                 - pointer to the frame descriptor.
* @param[in] actionDataPtr            - pointer to TTI action
* @param[in] internalTtiInfoPtr       - pointer to internal TTI info
*                                      OUTPUT:
* @param[in,out] descrPtr                 - pointer to the frame descriptor.
*/
static GT_VOID passengerParsingOfTransitNonMplsTunnel
(
    IN    SKERNEL_DEVICE_OBJECT             *devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC   *descrPtr,
    IN    SNET_XCAT_TT_ACTION_STC           *actionDataPtr,
    IN    INTERNAL_TTI_DESC_INFO_STC        *internalTtiInfoPtr
)
{
    DECLARE_FUNC_NAME(passengerParsingOfTransitNonMplsTunnel);

    SNET_E_ARCH_TT_ACTION_STC   *actionDataEArchPtr;/*E-Arch action info*/
    actionDataEArchPtr = (SNET_E_ARCH_TT_ACTION_STC*)(void*)actionDataPtr;

    if(!SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        return;
    }

    if(!actionDataEArchPtr->passengerParsingOfNonMplsTransitTunnelEnable)
    {
        __LOG(("passengerParsingOfNonMplsTransitTunnelEnable disabled"));
        return;
    }

    descrPtr->ingressTunnelInfo.transitType = SNET_INGRESS_TUNNEL_TRANSIT_TYPE_X_OVER_NON_MPLS_E;
    __LOG(("transitType = SNET_INGRESS_TUNNEL_TRANSIT_TYPE_X_OVER_NON_MPLS_E \n"));

    descrPtr->ingressTunnelInfo.innerFrameDescrPtr = snetChtEqDuplicateDescr(devObjPtr, descrPtr);

    /* this will update inner frame pointer */
    snetXcatTTActionApplyTtPassengerParsing(devObjPtr,
                        descrPtr->ingressTunnelInfo.innerFrameDescrPtr,
                        actionDataPtr, internalTtiInfoPtr, GT_TRUE);

    /* L3, L4 protocols parsing */
    snetChtL3L4ProtParsingResetDesc(devObjPtr, descrPtr->ingressTunnelInfo.innerFrameDescrPtr);
    /* use the ether type of the passenger that may be different then the one of the tunnel */
    snetChtL3L4ProtParsing(devObjPtr, descrPtr->ingressTunnelInfo.innerFrameDescrPtr,
                                      descrPtr->ingressTunnelInfo.innerFrameDescrPtr->etherTypeOrSsapDsap,
                                      internalTtiInfoPtr);
}

/**
* @internal passengerParsingOfTransitMplsTunnel function
* @endinternal
*
* @brief   parses transit mpls tunnel passenger
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] descrPtr                 - pointer to the frame descriptor.
* @param[in] actionDataPtr            - pointer to TTI action
* @param[in] internalTtiInfoPtr       - pointer to internal TTI info
*                                      OUTPUT:
* @param[in,out] descrPtr                 - pointer to the frame descriptor.
*/
static GT_VOID passengerParsingOfTransitMplsTunnel
(
    IN    SKERNEL_DEVICE_OBJECT             *devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC   *descrPtr,
    IN    SNET_XCAT_TT_ACTION_STC           *actionDataPtr,
    IN    INTERNAL_TTI_DESC_INFO_STC        *internalTtiInfoPtr
)
{
    DECLARE_FUNC_NAME(passengerParsingOfTransitMplsTunnel);

    SNET_E_ARCH_TT_ACTION_STC     *actionDataEArchPtr;/*E-Arch action info*/
    LION3_MPLS_TRANSIT_TUNNEL_PARSING_MODE_ENT pmode = 0; /* parsing mode */

    actionDataEArchPtr = (SNET_E_ARCH_TT_ACTION_STC*)(void*)actionDataPtr;

    if(!SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        return;
    }

    switch(actionDataEArchPtr->passengerParsingOfTransitMplsTunnelEnable)
    {
        case 0:
            /* Parsing is based on the tunnel header. The passenger packet is not parsed.*/
            __LOG(("Parsing is based on the tunnel header. The passenger packet is not parsed. \n"));
            return;
        case 1:
            /* Inner frame parse */
            __LOG(("Inner frame Parsing as IPv4/6 \n"));
            pmode = LION3_MPLS_TRANSIT_TUNNEL_PARSING_MODE_IP_ONLY_E;
            break;
        case 2:
            /* Parsing as IPv4/6 or as Ethernet that starts after control word (CW)
               of the pseudo wire (PW) */
            __LOG(("Inner frame parsing as IPv4/6 or as Ethernet that starts after control word (CW) "
                "of the pseudo wire (PW) \n"));
            pmode = LION3_MPLS_TRANSIT_TUNNEL_PARSING_MODE_IP_OR_CW_PW_ETHERNET_E;
            break;
        case 3:
            /* Parse passenger as Ethernet without CW */
            __LOG(("Inner frame Parsing as Ethernet without CW \n"));
            pmode = LION3_MPLS_TRANSIT_TUNNEL_PARSING_MODE_ETHERNET_NO_CW_PW_E;
            break;
        default:
            skernelFatalError("passengerParsingOfTransitMplsTunnel: invalid value\n");
    }

    descrPtr->ingressTunnelInfo.transitType = SNET_INGRESS_TUNNEL_TRANSIT_TYPE_X_OVER_MPLS_E;
    __LOG(("transitType = SNET_INGRESS_TUNNEL_TRANSIT_TYPE_X_OVER_MPLS_E \n"));

    snetChtMplsTransitTunnelsProtParsing(devObjPtr, descrPtr, pmode, internalTtiInfoPtr);
}

/**
* @internal snetXcatTTActionApplyRedirectCmdOutIf function
* @endinternal
*
* @brief   TT redirect cmd out interface processing
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] descrPtr                 - pointer to the frame descriptor.
* @param[in] actionDataPtr            - pointer to TTI action
* @param[in] internalTtiInfoPtr       - pointer to internal TTI info
* @param[in] isMplsLookup             - indicate is mpls lookup or not
*                                      OUTPUT:
* @param[in,out] descrPtr                 - pointer to the frame descriptor.
*/
static GT_VOID snetXcatTTActionApplyRedirectCmdOutIf
(
    IN    SKERNEL_DEVICE_OBJECT             *devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC   *descrPtr,
    IN    SNET_XCAT_TT_ACTION_STC           *actionDataPtr,
    OUT   INTERNAL_TTI_DESC_INFO_STC        *internalTtiInfoPtr,
    IN    GT_BOOL                            isMplsLookup
)
{
    DECLARE_FUNC_NAME(snetXcatTTActionApplyRedirectCmdOutIf);

    SNET_E_ARCH_TT_ACTION_STC   *actionDataEArchPtr;/*E-Arch action info*/

    actionDataEArchPtr = (SNET_E_ARCH_TT_ACTION_STC*)(void*)actionDataPtr;
    descrPtr->useVidx = 0;/* reset the value that maybe set for VIDX case */
    descrPtr->targetIsTrunk = 0;/* reset the value that maybe set for trunk case */

    /*  The redirect info */
    __LOG(("The redirect info"));
    if (SNET_GET_DEST_INTERFACE_TYPE(actionDataPtr->tunnelIf, SNET_DST_INTERFACE_VIDX_E))
    {   /* vidx */
        __LOG(("vidx"));
        descrPtr->eVidx = actionDataPtr->tunnelIf.interfaceInfo.vidx;
        descrPtr->useVidx = 1;
    }
    else if (SNET_GET_DEST_INTERFACE_TYPE(actionDataPtr->tunnelIf, SNET_DST_INTERFACE_TRUNK_E))
    {   /* trunk */
        __LOG(("trunk"));
        descrPtr->trgTrunkId =  actionDataPtr->tunnelIf.interfaceInfo.trunkId;
        descrPtr->targetIsTrunk = 1;
    }
    /* only sip7 */
    else if (SNET_GET_DEST_INTERFACE_TYPE(actionDataPtr->tunnelIf,
                                 SNET_DST_INTERFACE_VPORT_E))
    {
        descrPtr->trgIsVPort = 1;
        descrPtr->trgVPort = /* target VPort  */
            actionDataPtr->tunnelIf.interfaceInfo.vport;
        /* call after setting trgVPort */
        SIP7_INVALIDATE_TRG_EPORT_TRG_DEV_MAC(devObjPtr,descrPtr,tti);
    }
    else
    {
        descrPtr->trgIsVPort = 0;

        descrPtr->trgEPort = /* target port  */
            actionDataPtr->tunnelIf.interfaceInfo.devPort.port;
        __LOG(("target port %d", descrPtr->trgEPort));

        /* call after setting trgEPort */
        __LOG(("call after setting trgEPort"));
        SNET_E_ARCH_CLEAR_IS_TRG_PHY_PORT_VALID_MAC(devObjPtr,descrPtr,tti);

        descrPtr->trgDev = /* target dev  */
            actionDataPtr->tunnelIf.interfaceInfo.devPort.devNum;
        __LOG(("target dev %d", descrPtr->trgDev));
    }

    if((SKERNEL_IS_LION_REVISON_B0_DEV(devObjPtr)) ||
       (SMEM_CHT_IS_SIP5_GET(devObjPtr) && actionDataEArchPtr->egressFilterRegistered))
    {
        descrPtr->egressFilterRegistered = 1;
        __LOG(("descrPtr->egressFilterRegistered = 1;"));
    }

     /* indication for Tunnel Start */
    if (actionDataPtr->tunnelStart == GT_TRUE)
    {
        __LOG(("Tunnel Start"));
        descrPtr->tunnelStart = GT_TRUE;
        descrPtr->tunnelPtr = actionDataPtr->tunnelStartPtr;
        descrPtr->tunnelStartPassengerType = actionDataPtr->tunnelStartPassengerType;
    }
    else
    {
        descrPtr->arpPtr = actionDataPtr->arpPtr;
    }

    /* VntL2Echo is outside the TS/ARP union */
    descrPtr->VntL2Echo  = actionDataPtr->vntL2Echo;
}

/**
* @internal snetXcatTTActionApplyMplsTtl function
* @endinternal
*
* @brief   TT mpls ttl processing
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] descrPtr                 - pointer to the frame descriptor.
* @param[in] actionDataPtr            - pointer to TTI action
*                                      OUTPUT:
* @param[in,out] descrPtr                 - pointer to the frame descriptor.
*/
static GT_VOID snetXcatTTActionApplyMplsTtl
(
    IN    SKERNEL_DEVICE_OBJECT             *devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC   *descrPtr,
    IN    SNET_XCAT_TT_ACTION_STC           *actionDataPtr
)
{
    DECLARE_FUNC_NAME(snetXcatTTActionApplyMplsTtl);

    /*internal bit that should not be changed (to match VERIFIER code) */
    GT_U32 tti_switch_enable_mpls_lsr_glue = 1;
    GT_U32 decTtl = actionDataPtr->ttlDecEn;;

    __LOG(("start MPLS TTL assignment \n"));

    if(actionDataPtr->mplsCmd == SKERNEL_XCAT_TTI_MPLS_NOP_E) /* NOP */
    {
        if(descrPtr->tunnelTerminated &&
           actionDataPtr->copyTtlFromOuterHeader &&
           !SMEM_CHT_IS_SIP7_GET(devObjPtr))
        {
            /* take TTL from label0 */
            descrPtr->ttl = descrPtr->ttlArr[0];
            __LOG(("take TTL [%d] from first label \n",
                descrPtr->ttl));
        }
        else
        {
            __LOG(("do not modify TTL[%d] for descrPtr->tunnelTerminated[%d] and actionDataPtr->copyTtlFromOuterHeader[%d] \n",
                descrPtr->ttl,
                descrPtr->tunnelTerminated,
                actionDataPtr->copyTtlFromOuterHeader));
        }
    }
    else /* LSR operation */
    {
        __LOG(("This considered LSR operation \n"));

        if(actionDataPtr->ttl != 0)
        {
            descrPtr->ttl = actionDataPtr->ttl;
            __LOG(("take TTL [%d] from the action \n",
                descrPtr->ttl));

            decTtl = 0;
        }
        else /* actionDataPtr->ttl == 0 */
        {
            /* take TTL from label0 - 'most outer' of original packet */
            descrPtr->ttl = descrPtr->ttlArr[0];
            __LOG(("take TTL[%d] from first label - 'most outer' of original packet \n",
                descrPtr->ttl));

            switch(actionDataPtr->mplsCmd)
            {
            case SKERNEL_XCAT_TTI_MPLS_SWAP_E: /* Swap */
                __LOG(("mplsCmd: swap"));
                if(tti_switch_enable_mpls_lsr_glue)
                {
                    /* NOTE: not according to FS but aligned to VERIFIER code ! */
                    /* NOTE: the FS does state that the TS modes 1,2,3 should be used for TTL asignment so the TTL from the descriptor is 'dont care' */
                    descrPtr->ttl = descrPtr->ttlArr[1];
                    __LOG(("take TTL [%d] from second label \n",
                        descrPtr->ttl));

                    /*the TTL that should have been used for descriptor but used only for TTL exception check */
                    descrPtr->mplsUseSpecificTtlForTtlException = 1;
                    descrPtr->mplsLabelTtlForException = descrPtr->ttlArr[0];

                    decTtl = 0;
                }
                break;
            case SKERNEL_XCAT_TTI_MPLS_PUSH_E: /* Push */
                __LOG(("mplsCmd: push"));
                break;
            case SKERNEL_XCAT_TTI_MPLS_POP_1_E: /* Pop one label */
                __LOG(("mplsCmd: pop one label"));
                if (actionDataPtr->copyTtlFromOuterHeader == 0 &&
                    descrPtr->numOfLabels > 0 &&
                    !SMEM_CHT_IS_SIP7_GET(devObjPtr))
                {
                    /* take TTL from label2 */
                    descrPtr->ttl = descrPtr->ttlArr[1];
                    __LOG(("take TTL [%d] from second label \n",
                        descrPtr->ttl));
                }
                break;
            case SKERNEL_XCAT_TTI_MPLS_POP_2_E: /* Pop two labels */
                __LOG(("mplsCmd: pop two labels"));
                if (actionDataPtr->copyTtlFromOuterHeader == 0 &&
                    descrPtr->numOfLabels > 1 &&
                    !SMEM_CHT_IS_SIP7_GET(devObjPtr))
                {
                    /* take TTL from label3 */
                    descrPtr->ttl = descrPtr->ttlArr[2];
                    __LOG(("take TTL [%d] from third label \n",
                        descrPtr->ttl));
                }
                break;
            case SKERNEL_XCAT_TTI_MPLS_POP_SWAP_E: /* Pop_and_Swap */
                __LOG(("mplsCmd: pop and swap \n"));
                if(tti_switch_enable_mpls_lsr_glue)
                {
                    /* NOTE: not according to FS but aligned to VERIFIER code ! */
                    /* NOTE: the FS does state that the TS modes 1,2,3 should be used for TTL assignment so the TTL from the descriptor is 'dont care' */

                    /* take TTL from label3 */
                    descrPtr->ttl = descrPtr->ttlArr[2];
                    __LOG(("take TTL [%d] from third label \n",
                        descrPtr->ttl));

                    /*the TTL that should have been used for descriptor but used only for TTL exception check */
                    descrPtr->mplsUseSpecificTtlForTtlException = 1;
                    descrPtr->mplsLabelTtlForException = descrPtr->ttlArr[1];
                    decTtl = 0;
                }
                else
                if (actionDataPtr->copyTtlFromOuterHeader == 0 &&
                    !SMEM_CHT_IS_SIP7_GET(devObjPtr))
                {
                    /* take TTL from label2 */
                    descrPtr->ttl = descrPtr->ttlArr[1];
                    __LOG(("take TTL [%d] from second label \n",
                        descrPtr->ttl));
                }
                break;
            case SKERNEL_XCAT_TTI_MPLS_POP2_SWAP_E:
                __LOG(("mplsCmd: pop2 and swap \n"));
                if(tti_switch_enable_mpls_lsr_glue)
                {
                    /* NOTE: not according to FS but aligned to VERIFIER code ! */
                    /* NOTE: the FS does state that the TS modes 1,2,3 should be used for TTL assignment so the TTL from the descriptor is 'dont care' */

                    /* take TTL from label4 */
                    descrPtr->ttl = descrPtr->ttlArr[3];
                    __LOG(("take TTL [%d] from fourth label \n",
                        descrPtr->ttl));

                    /*the TTL that should have been used for descriptor but used only for TTL exception check */
                    descrPtr->mplsUseSpecificTtlForTtlException = 1;
                    descrPtr->mplsLabelTtlForException = descrPtr->ttlArr[2];
                    decTtl = 0;
                }
                else
                if (actionDataPtr->copyTtlFromOuterHeader == 0 &&
                    !SMEM_CHT_IS_SIP7_GET(devObjPtr))
                {
                    /* take TTL from label3 */
                    descrPtr->ttl = descrPtr->ttlArr[2];
                    __LOG(("take TTL [%d] from third label \n",
                        descrPtr->ttl));
                }
                break;

            case SKERNEL_XCAT_TTI_MPLS_POP3_SWAP_E:
                __LOG(("mplsCmd: pop3 and swap \n"));
                if(tti_switch_enable_mpls_lsr_glue)
                {
                    /* NOTE: not according to FS but aligned to VERIFIER code ! */
                    /* NOTE: the FS does state that the TS modes 1,2,3 should be used for TTL assignment so the TTL from the descriptor is 'dont care' */

                    /* take TTL from label5 */
                    descrPtr->ttl = 0xB9/*dummy value because not parsing 5 labels*/;
                    __LOG(("take TTL [%d] from fifth label (dummy value because not parsing 5 labels) \n",
                        descrPtr->ttl));

                    /*the TTL that should have been used for descriptor but used only for TTL exception check */
                    descrPtr->mplsUseSpecificTtlForTtlException = 1;
                    descrPtr->mplsLabelTtlForException = descrPtr->ttlArr[2];
                }
                else
                if (actionDataPtr->copyTtlFromOuterHeader == 0 &&
                    !SMEM_CHT_IS_SIP7_GET(devObjPtr))
                {
                    /* take TTL from label4 */
                    descrPtr->ttl = descrPtr->ttlArr[3];
                    __LOG(("take TTL [%d] from fourth label \n",
                        descrPtr->ttl));
                }
                break;

            case SKERNEL_XCAT_TTI_MPLS_POP_3_E:
                __LOG(("mplsCmd: pop 3 labels \n"));
                if (actionDataPtr->copyTtlFromOuterHeader == 0 &&
                         descrPtr->numOfLabels > 2 &&
                         !SMEM_CHT_IS_SIP7_GET(devObjPtr))
                {
                    /* take TTL from label4 */
                    descrPtr->ttl = descrPtr->ttlArr[3];
                    __LOG(("take TTL [%d] from fourth label \n",
                        descrPtr->ttl));
                }
                break;
            }
        }
    }

    if(descrPtr->mplsUseSpecificTtlForTtlException)
    {
        __LOG(("the TTL for exception check is[%d] but TTL for descriptor is [%d] \n",
            descrPtr->mplsLabelTtlForException,
            descrPtr->ttl));
    }

    if(descrPtr->mplsUseSpecificTtlForTtlException)
    {
        /* this 'orig' is used in the TunnelStart so we need to keep it with same value as used for 'Exception check' */
        descrPtr->origTunnelTtl = descrPtr->mplsLabelTtlForException;
    }
    else
    {
        descrPtr->origTunnelTtl = descrPtr->ttl; /* save this value for tunnel start */
    }

    /* Final TTL - should decTTL be considered */
    if (actionDataPtr->ttl != 0)
    {
        __LOG(("decTtl [%d] from the action is not relevant when the ttl[%d] is taken form the action \n",
                actionDataPtr->ttlDecEn,actionDataPtr->ttl));
    }
    else if(actionDataPtr->copyTtlFromOuterHeader)
    {
        descrPtr->ttl -= decTtl;
    }
    else if(descrPtr->tunnelTerminated == 1)
    {
        __LOG(("decTtl is not relevant in case of Tunnel Termination \n"));
    }
    else
    {
        /* descrPtr->ttl -= decTtl; */
        descrPtr->decTtl = actionDataPtr->ttlDecEn;
        /*This field is relevant for: MPLS packets that are not tunnel terminated*/
         __LOG(("take decTtl [%d] from the action \n",  decTtl));
    }

    __LOG(("End MPLS TTL [%d] assignment \n",descrPtr->ttl));
}

/**
* @internal snetXcatTTActionApplyEarchTti function
* @endinternal
*
* @brief   TTI Earch processing
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] descrPtr                 - pointer to the frame descriptor.
* @param[in] actionDataPtr            - pointer to TTI action
* @param[in] internalTtiInfoPtr       - pointer to internal TTI info
*                                      OUTPUT:
* @param[in,out] descrPtr                 - pointer to the frame descriptor.
*/
static GT_VOID snetXcatTTActionApplyEarchTti
(
    IN    SKERNEL_DEVICE_OBJECT             *devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC   *descrPtr,
    IN    SNET_XCAT_TT_ACTION_STC           *actionDataPtr,
    OUT   INTERNAL_TTI_DESC_INFO_STC        *internalTtiInfoPtr
)
{
    DECLARE_FUNC_NAME(snetXcatTTActionApplyEarchTti);

    GT_U32  regAddr;                /* Register's address */
    GT_U32  tmpValue;

    SNET_E_ARCH_TT_ACTION_STC   *actionDataEArchPtr;/*E-Arch action info*/
    actionDataEArchPtr = (SNET_E_ARCH_TT_ACTION_STC*)(void*)actionDataPtr;

    if(actionDataEArchPtr->assignSrcInterfaceEnable == 2)/* sip7 only */
    {
        /* assign new srcVPort */
        snetSip7IngressReassignSrcVPort(devObjPtr,descrPtr,"TTI Action" , GT_FALSE,
            actionDataEArchPtr->sourceVPort);
    }
    else
    if(actionDataEArchPtr->assignSrcInterfaceEnable == 1)
    {
        snetLion3IngressReassignSrcEPort(devObjPtr,descrPtr,"TTI Action" ,
            actionDataEArchPtr->sourceEPort);

        internalTtiInfoPtr->ttiActionReassignsTheSourceEPort = 1;
    }

    if(actionDataEArchPtr->sourceVPortProfileAssignmentEnable)/* sip7 only */
    {
        /* assign new srcVPort profile - which is held by 'src eport' */
        snetSip7IngressReassignSrcVPort(devObjPtr,descrPtr,"TTI Action" , GT_TRUE ,
            actionDataEArchPtr->sourceEPort/* the ePort used as 'vPort profile' */);
    }

    /* tti - Post-TTI Lookup Ingress ePort Table */
    __LOG(("tti - Post-TTI Lookup Ingress ePort Table"));
    regAddr = SMEM_LION2_TTI_POST_TTI_LOOKUP_INGRESS_EPORT_TABLE_TBL_MEM(devObjPtr,
            descrPtr->eArchExtInfo.localDevSrcEPort);
    descrPtr->eArchExtInfo.ttiPostTtiLookupIngressEPortTablePtr = smemMemGet(devObjPtr, regAddr);

    /* next setting must be before calling snetChtL2Parsing(...) */
    /*Number of tags to Pop*/
    tmpValue =
                SMEM_LION3_TTI_EPORT_ATTRIBUTES_ENTRY_FIELD_GET(devObjPtr,descrPtr,
                    SMEM_LION3_TTI_EPORT_ATTRIBUTES_TABLE_FIELDS_NUM_OF_TAGS_TO_POP);

    if(SMEM_CHT_IS_SIP6_GET(devObjPtr) &&
      (descrPtr->tunnelTerminated == 0) &&
      descrPtr->didLocalPortReAssign &&
      (descrPtr->myPhyPortAssignMode == 4 /* vlan mode*/ || descrPtr->myPhyPortAssignMode == 3 /* etag mode*/))
    {
       __LOG(("Sip6: the packet is not tunnel Terminated and 'physical port assignment mode' is ETAG mode or VLAN mode and physical port is reassigen, keep the descrPtr->numOfBytesToPop[%d] instead of using from ePort\n",descrPtr->numOfBytesToPop));
    }
    else
    {
        /*Number of bytes to Pop*/
        descrPtr->numOfBytesToPop =
            SNET_CONVERT_POP_TAGS_FEILD_TO_BYTES_MAC(devObjPtr,tmpValue);
        __LOG(("use from ePort : descrPtr->numOfBytesToPop[%d] \n",descrPtr->numOfBytesToPop));
    }


    /*Trust L2 QoS Tag0 or Tag1*/
    descrPtr->trustTag1Qos =
                SMEM_LION3_TTI_EPORT_ATTRIBUTES_ENTRY_FIELD_GET(devObjPtr,descrPtr,
                    SMEM_LION3_TTI_EPORT_ATTRIBUTES_TABLE_FIELDS_TRUST_L2_QOS_TAG0_OR_TAG1);

    __LOG(("use from ePort : descrPtr->trustTag1Qos[%d]",descrPtr->trustTag1Qos));
}

/**
* @internal snetLion3TTActionApplyMplsCwBasedPw function
* @endinternal
*
* @brief   TT action apply mpls control word processing
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] descrPtr                 - pointer to the frame descriptor.
* @param[in] actionDataPtr            - pointer to TTI action
* @param[in] firstNibbleAfterSBitLabel - first nibble after sbit label
* @param[in] pwe3CwWord               - pseudo wire control word
*                                      OUTPUT:
* @param[in,out] descrPtr                 - pointer to the frame descriptor.
*/
static GT_VOID snetLion3TTActionApplyMplsCwBasedPw
(
    IN    SKERNEL_DEVICE_OBJECT             *devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC   *descrPtr,
    IN    SNET_XCAT_TT_ACTION_STC           *actionDataPtr,
    IN    GT_U32                             firstNibbleAfterSBitLabel,
    IN    GT_U32                             pwe3CwWord
)
{
    DECLARE_FUNC_NAME(snetLion3TTActionApplyMplsCwBasedPw);

    GT_U32  pwConfig; /*PW configurations*/
    GT_U32  fieldVal; /* Register's field value */
    GT_U32  fieldVal1;/* Register's field value */
    GT_U32  fieldVal2;/* Register's field value */
    GT_U32  fieldVal3;/* Register's field value */
    GT_U32  fieldVal4;/* Register's field value */

    SNET_E_ARCH_TT_ACTION_STC   *actionDataEArchPtr;/*E-Arch action info*/
    actionDataEArchPtr = (SNET_E_ARCH_TT_ACTION_STC*)(void*)actionDataPtr;

   /*
   If (TTI-AE<CW Based PseudoWire> = Enable)
       If (First nibble after S_BIT_LABEL == 1) && (Apply Non-Data CW Command == Enabled)
           Assign packet command according to Non-Data CW Command
       Else If (First nibble after S_BIT_LABEL == 0)
           If (TTI-AE<Tunnel Terminate> = 1)
               If (PWE3 Data Word<FRG> != 0)
                   Assign packet command according to PW-CW Fragmented Cmd
               Else If (((TTI-AE<PW-CW Sequencing Enable> = Disable) && (PWE3 Data Word<Sequence> != 0)) ||
                        ((TTI-AE<PW-CW Sequencing Enable> = Enable) && (PWE3 Data Word<Sequence> = 0)))
                   Assign packet command according to PW-CW Sequence Error Cmd
               Else
                   do not modify packet command

               If there was no exception
                   If (TTI-AE<PW-CW Based E-Tree Enable> = Enable)
                       outDesc<SST ID>[0] = <<L> Bit> regardless of the value of :
                                   TTI AE <Source ID Assignment Enable>,
                                   TTI-AE<Source ID> [0],
                                   <SrcID mask1> and <SrcID mask2>
                       outDesc<PCL Assigned SST ID>[0] = 1
                   Else
                       do nothing
           Else // TTI-AE<Tunnel Terminate> = 0
               Do Nothing
       Else // illegal value of First nibble after S_BIT_LABEL
           Assign packet command according to PWE3 Illegal Control Word Cmd
   */

   /* get PW configurations */
   smemRegGet(devObjPtr,SMEM_XCAT_TTI_PSEUDO_WIRE_CONFIG_CONF_REG(devObjPtr), &pwConfig);

   /*PWE3 CPU Code Base*/
   fieldVal4 = SMEM_U32_GET_FIELD(pwConfig,15,8);
   __LOG(("PWE3 CPU Code Base[%d] \n",fieldVal4));

   if(firstNibbleAfterSBitLabel == 1 && actionDataEArchPtr->applynonDataCwCommand)
   {
       __LOG(("firstNibbleAfterSBitLabel == 1 && actionDataEArchPtr->applynonDataCwCommand"));
       /*Assign packet command according to Non-Data CW Command*/
       __LOG(("Assign packet command according to Non-Data CW Command"));

       snetChtIngressCommandAndCpuCodeResolution(devObjPtr,descrPtr,
                                                 descrPtr->packetCmd,
                                                 SMEM_U32_GET_FIELD(pwConfig,6,3),/*Non-Data CW Cmd*/
                                                 descrPtr->cpuCode,
                                                 fieldVal4 + 1,/*Non-Data CW CPU Code*/
                                                 SNET_CHEETAH_ENGINE_UNIT_TTI_E,
                                                 GT_FALSE);
   }
   else if(firstNibbleAfterSBitLabel == 0)
   {
       __LOG(("firstNibbleAfterSBitLabel == 0"));
       if(descrPtr->tunnelTerminated)
       {
           /*PWE3 Data Word<FRG>*/
           fieldVal = SMEM_U32_GET_FIELD(pwe3CwWord,22,2);
           /*PWE3 Data Word<Sequence>*/
           fieldVal2 = SMEM_U32_GET_FIELD(pwe3CwWord,0,16);
           /*PWE3 Data Word<L bit>*/
           fieldVal3 = SMEM_U32_GET_FIELD(pwe3CwWord,27,1);

           /*PW-CW Sequencing Enable*/
           fieldVal1 = SMEM_U32_GET_FIELD(pwConfig,23,1);

           __LOG(("From packet : PWE3 Data Word <FRG> [%d] ,"
                         "<Sequence> [0x%4.4x] "
                         "<L bit> [%d] "
                         ,fieldVal,fieldVal2,fieldVal3));

           __LOG(("From global config : PW-CW Sequencing Enable [%d]",fieldVal1));

           if(fieldVal)
           {
               /*Assign packet command according to PW-CW Fragmented Cmd*/
               __LOG(("Assign packet command according to PW-CW Fragmented Cmd"));

               snetChtIngressCommandAndCpuCodeResolution(devObjPtr,descrPtr,
                                                         descrPtr->packetCmd,
                                                         SMEM_U32_GET_FIELD(pwConfig,9,3),/*PW-CW Fragmented Cmd*/
                                                         descrPtr->cpuCode,
                                                         fieldVal4 + 2,/*PW-CW Fragmented CPU Code*/
                                                         SNET_CHEETAH_ENGINE_UNIT_TTI_E,
                                                         GT_FALSE);
           }
           else if(((fieldVal1 == 0) && fieldVal2) || (fieldVal1 && (fieldVal2 == 0)))
           {
               /*(((<PW-CW Sequencing Enable> = Disable) && (PWE3 Data Word<Sequence> != 0)) ||
                  ((<PW-CW Sequencing Enable> = Enable) && (PWE3 Data Word<Sequence> = 0)))*/

               /*Assign packet command according to PW-CW Sequence Error Cmd*/
               __LOG(("Assign packet command according to PW-CW Sequence Error Cmd"));

               snetChtIngressCommandAndCpuCodeResolution(devObjPtr,descrPtr,
                                                         descrPtr->packetCmd,
                                                         SMEM_U32_GET_FIELD(pwConfig,12,3),/*PW-CW Sequence Error Cmd*/
                                                         descrPtr->cpuCode,
                                                         fieldVal4 + 3,/*PW-CW Sequence Error CPU Code*/
                                                         SNET_CHEETAH_ENGINE_UNIT_TTI_E,
                                                         GT_FALSE);
           }
           else
           {
               /*do not modify packet command*/
                __LOG(("do not modify packet command"));
           }

           if(descrPtr->packetCmd == SKERNEL_EXT_PKT_CMD_FORWARD_E ||
              descrPtr->packetCmd == SKERNEL_EXT_PKT_CMD_MIRROR_TO_CPU_E)
           {
               /*If there was no exception*/
               __LOG(("NO PW-CW exception , actionDataEArchPtr->pwCwBasedETreeEnable[%d]",
                             actionDataEArchPtr->pwCwBasedETreeEnable));

               if(actionDataEArchPtr->pwCwBasedETreeEnable)
               {
                   /*
                       outDesc<SST ID>[0] = <<L> Bit> regardless of the value of :
                                   TTI AE <Source ID Assignment Enable>,
                                   TTI-AE<Source ID> [0],
                                   <SrcID mask1> and <SrcID mask2>
                   */
                   __LOG(("modify bit 0 of <sstId> from [0x%3.3x] with L bit value[%d]",descrPtr->sstId,fieldVal3));
                   SMEM_U32_SET_FIELD(descrPtr->sstId,0,1,fieldVal3);
                   /*  outDesc<PCL Assigned SST ID>[0] = 1 */
                   __LOG(("modify bit 0 of <pclAssignedSstId> from [0x%3.3x] with 1",descrPtr->pclAssignedSstId));
                   SMEM_U32_SET_FIELD(descrPtr->pclAssignedSstId,0,1,1);
               }
               else
               {
                   /*do nothing*/
               }
           }

       }
       else /*TTI-AE<Tunnel Terminate> = 0*/
       {
           /*Do Nothing*/
           __LOG(("TTI-AE<Tunnel Terminate> = 0  --> Do Nothing"));
       }
   }
   else
   {
       /* illegal value of First nibble after S_BIT_LABEL */
       __LOG(("illegal value of First nibble [0x%x] after S_BIT_LABEL",firstNibbleAfterSBitLabel));

       /* Assign packet command according to PWE3 Illegal Control Word Cmd */
       __LOG(("Assign packet command according to PWE3 Illegal Control Word Cmd"));

       snetChtIngressCommandAndCpuCodeResolution(devObjPtr,descrPtr,
                                                 descrPtr->packetCmd,
                                                 SMEM_U32_GET_FIELD(pwConfig,0,3),/*PWE3 Illegal Control Word CW Cmd*/
                                                 descrPtr->cpuCode,
                                                 fieldVal4 + 4,/*PWE3 Illegal Control Word CPU Code*/
                                                 SNET_CHEETAH_ENGINE_UNIT_TTI_E,
                                                 GT_FALSE);
   }

}

/**
* @internal getVariableTunnelHeaderLength function
* @endinternal
*
* @brief   Sip6.10: calculates variable tunnel header length
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] descrPtr             - pointer to the frame descriptor.
* @param[in] internalTtiInfoPtr       - pointer to internal TTI info
* @param[in] ttProfilesTableIndex     - index to tti header length profile table
*                                      OUTPUT:
* @param[in] tunnelHeaderLengthPtr    - pointer to tunnel header length
*/
static GT_VOID getVariableTunnelHeaderLength
(
    IN    SKERNEL_DEVICE_OBJECT             *devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC   *descrPtr,
    IN    INTERNAL_TTI_DESC_INFO_STC        *internalTtiInfoPtr,
    IN    GT_U32                            ttProfilesTableIndex,
    OUT   GT_U32                            *tunnelHeaderLengthPtr
)
{
    DECLARE_FUNC_NAME(getVariableTunnelHeaderLength);

    GT_STATUS rc = GT_OK;
    GT_U32  regValue;           /* register value */
    GT_U32  lengthFieldAnchor;  /* Bit[0] */
    GT_U32  lengthFieldUdbMsb;  /* Bit[4:1] */
    GT_U32  lengthFieldUdbLsb;  /* Bit[8:5] */
    GT_U32  lengthFieldSize;    /* Bit[11:9] */
    GT_U32  lengthFieldLsBit;   /* Bit[15:12] */
    GT_U32  constant;           /* Bit[22:16] */
    GT_U32  multiplier;         /* Bit[24:23] */
    GT_U32  concatWord          = 0;
    GT_U32  variableLengthValue = 0;
    GT_U32  variableLengthMask  = 0;
    GT_U32  tunnelHeaderLength  = 0;
    GT_U8   byteLsb = 0;
    GT_U8   byteMsb = 0;

    /* Read Tunnel Termination Profiles Table */
    if(SMEM_CHT_IS_SIP7_GET(devObjPtr))
    {
        smemRegGet(devObjPtr, SMEM_SIP7_TTI_LU_TUNNEL_HEADER_LENGTH_PROFILE_REG(devObjPtr, ttProfilesTableIndex), &regValue);
    }
    else
    {
        smemRegGet(devObjPtr, SMEM_SIP6_10_TTI_TUNNEL_HEADER_LENGTH_PROFILE_REG(devObjPtr, ttProfilesTableIndex), &regValue);
    }

    /* extract fields of Tunnel Termination Profiles Table entry */
    lengthFieldAnchor = (regValue >>  0) & 0x1;
    lengthFieldUdbMsb = (regValue >>  1) & 0xF;
    lengthFieldUdbLsb = (regValue >>  5) & 0xF;
    lengthFieldSize   = (regValue >>  9) & 0x7;
    lengthFieldLsBit  = (regValue >> 12) & 0xF;
    constant          = (regValue >> 16) & 0x7F;
    multiplier        = (regValue >> 23) & 0x3;

    if(simLogIsOpenFlag)
    {
        __LOG_PARAM(lengthFieldAnchor);
        __LOG_PARAM(lengthFieldUdbMsb);
        __LOG_PARAM(lengthFieldUdbLsb);
        __LOG_PARAM(lengthFieldSize  );
        __LOG_PARAM(lengthFieldLsBit );
        __LOG_PARAM(constant         );
        __LOG_PARAM(multiplier       );
    }

    if((lengthFieldUdbMsb >= descrPtr->frameBuf->actualDataSize)
        || (lengthFieldUdbLsb >= descrPtr->frameBuf->actualDataSize))
    {
        skernelFatalError("Length Field UDB out of range. Msb: 0x%X, Lsb: 0x%X, maxSize: 0x%X\n",
                lengthFieldUdbMsb, lengthFieldUdbLsb, descrPtr->frameBuf->actualDataSize);
    }

    rc = snetLion3TtiUdbKeyValueGet(devObjPtr, descrPtr, internalTtiInfoPtr->ttiKeyType,
            lengthFieldUdbLsb + 16, &byteLsb, internalTtiInfoPtr);
    if(rc != GT_OK)
    {
        skernelFatalError("snetLion3TtiUdbKeyValueGet failed rc=%d\n", rc);
    }

    rc = snetLion3TtiUdbKeyValueGet(devObjPtr, descrPtr, internalTtiInfoPtr->ttiKeyType,
            lengthFieldUdbMsb + 16 , &byteMsb, internalTtiInfoPtr);
    if(rc != GT_OK)
    {
        skernelFatalError("snetLion3TtiUdbKeyValueGet failed rc=%d\n", rc);
    }

    concatWord = (byteMsb << 8) | byteLsb;
    __LOG_PARAM(concatWord);

    if(lengthFieldSize != 0)
    {
        variableLengthMask = (1 << lengthFieldSize) - 1;
        variableLengthValue = (concatWord >> lengthFieldLsBit) & variableLengthMask;
    }

    tunnelHeaderLength = descrPtr->l2HeaderSize;
    if(lengthFieldAnchor == 1)
    {
        tunnelHeaderLength = descrPtr->l23HeaderSize;
    }
    tunnelHeaderLength += constant + (1 << multiplier)*variableLengthValue;

    __LOG_PARAM(tunnelHeaderLength);

    *tunnelHeaderLengthPtr = tunnelHeaderLength;
}

/**
* @internal getTunnelHeaderLength function
* @endinternal
*
* @brief   calculates tunnel header length
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] descrPtr                 - pointer to the frame descriptor.
* @param[in] actionDataPtr            - pointer to TTI action
* @param[in] internalTtiInfoPtr       - pointer to internal TTI info
*                                      OUTPUT:
* @param[out] tunnelHeaderLengthPtr    - pointer to tunnel header length
*/
static GT_VOID getTunnelHeaderLength
(
    IN    SKERNEL_DEVICE_OBJECT             *devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC   *descrPtr,
    IN    SNET_XCAT_TT_ACTION_STC           *actionDataPtr,
    IN    INTERNAL_TTI_DESC_INFO_STC        *internalTtiInfoPtr,
    OUT   GT_U32                            *tunnelHeaderLengthPtr
)
{
    DECLARE_FUNC_NAME(getTunnelHeaderLength);

    GT_U32                       protocol;          /* tunnel protocol */
    SNET_E_ARCH_TT_ACTION_STC   *actionDataEArchPtr;/*E-Arch action info*/
    GT_U32                      isError = 0;

    actionDataEArchPtr = (SNET_E_ARCH_TT_ACTION_STC*)(void*)actionDataPtr;

    if(SMEM_CHT_IS_SIP5_GET(devObjPtr) && actionDataEArchPtr->ttHeaderLength)
    {
        *tunnelHeaderLengthPtr = 2 * actionDataEArchPtr->ttHeaderLength;

        __LOG(("Got explicit [%d] bytes offset in the packet to get the Passenger \n",
            (*tunnelHeaderLengthPtr)));
    }
    else
    {
        *tunnelHeaderLengthPtr = 0;

        __LOG(("Do auto calc of offset in the packet to get the Passenger \n"));

        /* this is done according to packet type and not key type */
        if(descrPtr->mpls)
        {
            __LOG(("pass over the [%d] MPLS labels \n",
                descrPtr->numOfLabels +1));

            *tunnelHeaderLengthPtr = 4*(descrPtr->numOfLabels +1);
            if(descrPtr->vplsInfo.pwHasCw == 1 ||    /* xcat+ */
               actionDataEArchPtr->cwBasedPseudoWire)/* SIP5 */
            {
                __LOG(("pass over the cwBasedPseudoWire label \n"));
                *tunnelHeaderLengthPtr += 4;
            }
        }
        else
        if(descrPtr->isIp && descrPtr->isIPv4)
        {
            protocol = descrPtr->ipProt;
            (*tunnelHeaderLengthPtr) = 4* descrPtr->ipxHeaderLength;
            __LOG(("pass over IPv4 header -> total of [%d] bytes \n",
                (*tunnelHeaderLengthPtr)));

            if (protocol == 47 )
            {
                    /*pass over GRE extensions K, C, and S.*/
                __LOG(("pass also over the GRE [%d] bytes \n",
                    (descrPtr->greHeaderSize == 0) ? 4 : descrPtr->greHeaderSize));
                (*tunnelHeaderLengthPtr) += (descrPtr->greHeaderSize == 0) ? 4 : descrPtr->greHeaderSize;
            }
        }
        else
        if(descrPtr->mim)
        {
            *tunnelHeaderLengthPtr = 4;
        }

        if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
        {
            /*If the packet is TRILL and TTI AE <TT Header Length> is set to 0, the TT
                Header Length is set based on the parsing the TRILL header whenever the
                TRILL engine is enabled.
            If the key type is flexible-length Packet Type IPv4-UDP or Packet Type
                IPv6-UDP and TTI AE <TT Header Length> is set to 0, the TT Header Length
                is set to the end of the UDP header + 8B. The reason "+ 8B" is because this is
                the header size (after the UDP header) for both VXLAN and CAPWAP.
            If the packet is IPv4-GRE or IPv6-GRE and TTI AE <TT Header Length> is set
                to 0 the TT Header Length is set based on the parsing over the GRE header.
                The parser knows to pass over GRE extensions K, C, and S.*/

            if(descrPtr->isTrillEtherType)
            {
                __LOG(("pass over Trill header [%d] bytes \n",
                    (*tunnelHeaderLengthPtr)));
                *tunnelHeaderLengthPtr = 6; SIM_TBD_BOOKMARK
            }
            else if((descrPtr->tti_pcktType_sip5 == SIP5_PACKET_CLASSIFICATION_TYPE_IPV4_UDP_E ||
                     descrPtr->tti_pcktType_sip5 == SIP5_PACKET_CLASSIFICATION_TYPE_IPV6_UDP_E)
                    && descrPtr->l4StartOffsetPtr != NULL && descrPtr->l3StartOffsetPtr != NULL)
            {
                    *tunnelHeaderLengthPtr =
                        descrPtr->l4StartOffsetPtr - descrPtr->l3StartOffsetPtr + UDP_HEADER_LENGTH + 8;
                __LOG(("pass over IPv4/6 header and UDP -> total of [%d] bytes (including 8 bytes of CAPWAP/VXLAN header) \n",
                    (*tunnelHeaderLengthPtr)));
            }
            else if (descrPtr->isIp && descrPtr->isIPv4 == 0 && descrPtr->isFcoe == 0)
            {
                if (SMEM_CHT_IS_SIP6_GET(devObjPtr) && (descrPtr->ipv6HeaderLengthWithExtensionsInBytes != 0))
                {
                    *tunnelHeaderLengthPtr = descrPtr->ipv6HeaderLengthWithExtensionsInBytes;
                }
                else
                {
                    *tunnelHeaderLengthPtr = 4* descrPtr->ipxHeaderLength;
                }

                isError =  descrPtr->ipv6HeaderParserError;/* supported on sip6 devices */
                if(descrPtr->ipv6HeaderParserError)
                {
                     __LOG(("Due to the (Ipv6 tunnel) parser error , parsing of passenger will not be performed properly \n"));
                }

                __LOG(("pass over IPv6 header -> total of [%d] bytes \n",
                    (*tunnelHeaderLengthPtr)));

                if(descrPtr->ipProt == 47)/* GRE*/
                {
                    /*pass over GRE extensions K, C, and S.*/
                    __LOG(("pass also over the GRE [%d] bytes \n",
                    (descrPtr->greHeaderSize == 0) ? 4 : descrPtr->greHeaderSize));
                    (*tunnelHeaderLengthPtr) += (descrPtr->greHeaderSize == 0) ? 4 : descrPtr->greHeaderSize;
                }
            }
            else
            if((descrPtr->isIp && descrPtr->isIPv4)||
               (descrPtr->mpls))
            {
                /* already treated */
            }
            else
            if(internalTtiInfoPtr->ttiKeyType >= SKERNEL_LION3_TTI_KEY_UDB_PKT_TYPE_0_E)
            {
                /* only the legacy keys supports auto calc */
                /* this is HW behavior */
                *tunnelHeaderLengthPtr = 0;
                __LOG(("The flex length key not support auto calc offset to passenger \n"));
            }
        }
    }

    if (isError && SMEM_CHT_IS_SIP6_GET(devObjPtr))
    {
        GT_U32  drop_code_for_illegal_tunnel_length;

        smemRegFldGet(devObjPtr, SMEM_SIP6_TTI_UNIT_GLOBAL_CONFIG_EXT2_REG(devObjPtr),
                                  17, 8, &drop_code_for_illegal_tunnel_length);

         __LOG(("Apply 'HARD DROP' and CPU code [%d] for : Illegal tunnel length for passenger parsing \n",
            drop_code_for_illegal_tunnel_length));

        snetChtIngressCommandAndCpuCodeResolution(devObjPtr,descrPtr,
                                                  descrPtr->packetCmd,
                                                  SKERNEL_EXT_PKT_CMD_HARD_DROP_E, /* hard coded - not from register */
                                                  descrPtr->cpuCode,
                                                  drop_code_for_illegal_tunnel_length,
                                                  SNET_CHEETAH_ENGINE_UNIT_TTI_E,
                                                  GT_TRUE);/* not care */

         __LOG(("Due to the parser error consider the tunnel length as ZERO \n"));
         *tunnelHeaderLengthPtr = 0;
    }

    __LOG(("tunnelHeaderLength = %d \n", *tunnelHeaderLengthPtr));
}

/**
* @internal snetXcatTTActionApplyTtPassengerParsing function
* @endinternal
*
* @brief   TT pasenger parsing
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] descrPtr                 - pointer to the frame descriptor.
* @param[in] actionDataPtr            - pointer to TTI action
* @param[in] internalTtiInfoPtr       - pointer to internal TTI info
* @param[in] isTransitTunnel          - indicates is transit tunnel or not
*                                      OUTPUT:
* @param[in,out] descrPtr                 - pointer to the frame descriptor.
*/
static GT_VOID snetXcatTTActionApplyTtPassengerParsing
(
    IN    SKERNEL_DEVICE_OBJECT             *devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC   *descrPtr,
    IN    SNET_XCAT_TT_ACTION_STC           *actionDataPtr,
    IN    INTERNAL_TTI_DESC_INFO_STC        *internalTtiInfoPtr,
    IN    GT_BOOL                           isTransitTunnel
)
{
    DECLARE_FUNC_NAME(snetXcatTTActionApplyTtPassengerParsing);

    GT_U32  tunnelHeaderLength = 0; /* the length of the tunnel header in the frame */
    GT_BOOL isIpv4 = 0;             /* indication if passenger is ipv4/ipv6 */
    GT_U32  preAction_eVid0       = descrPtr->eVid;/* save for restoring values */
    GT_U32  preAction_vidModified = descrPtr->vidModified;/* save for restoring values */
    GT_U32  tmpPassPktType;         /* temporary value for passengerPacketType */
    GT_U32  firstNibble;
    SNET_E_ARCH_TT_ACTION_STC *actionDataEArchPtr;

    getTunnelHeaderLength(devObjPtr, descrPtr, actionDataPtr, internalTtiInfoPtr, &tunnelHeaderLength);

    if (SMEM_CHT_IS_SIP6_GET(devObjPtr))
    {
        actionDataEArchPtr = (SNET_E_ARCH_TT_ACTION_STC*)(void*)actionDataPtr;
        __LOG_PARAM(actionDataEArchPtr->tunnelHeaderLengthAnchorType);
        if(SMEM_CHT_IS_SIP6_10_GET(devObjPtr) && (actionDataEArchPtr->tunnelHeaderLengthAnchorType
                                    == SKERNEL_HAWK_TT_HEADER_LENGTH_ANCHOR_TYPE_PROFILE_BASED_E))
        {
            tunnelHeaderLength = 0;

            /* Get variable tunnel header length. */
            getVariableTunnelHeaderLength(devObjPtr, descrPtr, internalTtiInfoPtr,
                    (actionDataEArchPtr->ttHeaderLength & 0x7)/* ttProfilesTableIndex*/, &tunnelHeaderLength);

            /* tunnelHeaderLength = L2 Header Length + L3 Header Length + Variable tunel Header Length */

            descrPtr->passengerLength = descrPtr->byteCount;
            descrPtr->l3StartOffsetPtr = descrPtr->startFramePtr + tunnelHeaderLength;
        }
        else
        {
            if (actionDataEArchPtr->tunnelHeaderLengthAnchorType == 1)
            {
                descrPtr->passengerLength = descrPtr->byteCount - (descrPtr->l4StartOffsetPtr - descrPtr->startFramePtr);
                descrPtr->l3StartOffsetPtr = descrPtr->l4StartOffsetPtr + tunnelHeaderLength;
            }
            else
            {
                descrPtr->passengerLength = descrPtr->byteCount - (descrPtr->l3StartOffsetPtr - descrPtr->startFramePtr);
                descrPtr->l3StartOffsetPtr += tunnelHeaderLength;
            }
        }
    }
    else
    {
        descrPtr->passengerLength = descrPtr->byteCount - (descrPtr->l3StartOffsetPtr - descrPtr->startFramePtr);
        descrPtr->l3StartOffsetPtr += tunnelHeaderLength;
    }
    descrPtr->passengerLength -= tunnelHeaderLength;
    descrPtr->tunnelTerminationOffset = descrPtr->l3StartOffsetPtr - descrPtr->startFramePtr;
    __LOG_PARAM(descrPtr->tunnelTerminationOffset);

    if(isTransitTunnel == GT_FALSE)/* tunnel terminated - not transit */
    {
        /*
            descrPtr->mac2me is set if ((packet has a hit in Mac2ME table) & (packet's L2 is valid)) or TTI action has set it.
            Note that MAC2ME lookup is done twice for TT packets.
            The lookup for the passenger MAC-DA is the one that is relevant for the descriptor.
        */
        if(descrPtr->mac2me)
        {
            __LOG(("NOTE: Reset the 'descrPtr->mac2me' indication that relate to the 'tunnel' (since not relevant to the 'passenger') \n"));

            descrPtr->mac2me = 0;
            __LOG_PARAM(descrPtr->mac2me);
        }

        if(actionDataPtr->passengerPacketType < 2 /*ipv4/ipv6*/)
        {
            __LOG(("NOTE: if 'descrPtr->mac2me' indication needed it must come from the TTI action \n"));
        }
        else /* Ethernet with/without CRC */
        {
            __LOG(("NOTE: if 'descrPtr->mac2me' indication needed it is checked on Passengers mac DA \n"));
        }
    }
    else
    {
        /* the transit not modify the mac2me in the descriptor and not doing mac2me second lookup */
        /* NOTE: the 'descriptor' is of 'secondary' : descrPtr->ingressTunnelInfo.innerFrameDescrPtr */
    }

    tmpPassPktType = /*isTransitTunnel ? 2 : */actionDataPtr->passengerPacketType;

    switch(tmpPassPktType)
    {
        case 0:/* IPv4/Ipv6 */
            descrPtr->innerPacketType = SKERNEL_INNER_PACKET_TYPE_IP;
            descrPtr->l2Valid = 0;
            __LOG_PARAM(descrPtr->l2Valid);

            descrPtr->tagSrcTagged[SNET_CHT_TAG_0_INDEX_CNS] = 0;
            __LOG_PARAM(descrPtr->tagSrcTagged[SNET_CHT_TAG_0_INDEX_CNS]);

            descrPtr->macDaType = SKERNEL_UNICAST_MAC_E;
            __LOG(("macDaType == SKERNEL_UNICAST_MAC_E"));

            /* Final passenger type, 4 or 6, will be determined based on the
                       first nibble of the passenger header as this is the IP version field.*/
            /* first nibble of the passenger header - indication if it's ipv4/ipv6 */
            firstNibble = descrPtr->l3StartOffsetPtr[0] >> 4;
            __LOG_PARAM(firstNibble);
            isIpv4 = (firstNibble == 0x4) ? GT_TRUE : GT_FALSE;
            descrPtr->etherTypeOrSsapDsap = (isIpv4 == GT_TRUE ? SKERNEL_L3_PROT_TYPE_IPV4_E : SKERNEL_L3_PROT_TYPE_IPV6_E);

            if((firstNibble != 4) && (firstNibble != 6))
            {
                __LOG(("WARNING : Passenger considered IPv6 although first nibble is [%d] and not 6 !!! \n",
                    firstNibble));
            }

            __LOG(("Passenger type is IPv%d \n", isIpv4 ? 4 : 6));

            break;

        case 1:/* mpls passenger type - used only for LSR,
                  not relevant for tunnel terminated packets */
            descrPtr->innerPacketType = SKERNEL_INNER_PACKET_TYPE_MPLS;
            __LOG(("Passenger type is mpls - used only for LSR, "
                   "not relevant for tunnel terminated packets \n"));

            break;

        case 2:/* Ethernet with crc*/
            /* if TTI have not recalculated butecount ot CT packet don't update it */
            if (descrPtr->byteCount != 0x3FFF)
            {
                descrPtr->byteCount -= 4; /* remove the extra CRC */
            }
            /* fall through to Ethernet without CRC case */
            GT_ATTR_FALLTHROUGH;

        case 3:/* Ethernet without crc*/
            if(tmpPassPktType == 3)
            {
                descrPtr->innerPacketType = SKERNEL_INNER_PACKET_TYPE_ETHERNET;
                __LOG(("Passenger type is Ethernet without crc \n"));
            }
            else
            {
                descrPtr->innerPacketType = SKERNEL_INNER_PACKET_TYPE_ETHERNET_WITH_CRC;
                __LOG(("Passenger type is Ethernet with crc \n"));
            }

            /* update the length and the start of frame */
            __LOG(("start L2 re-parse of the passenger \n"));
            /* if TTI have not recalculated butecount ot CT packet don't update it */
            if (descrPtr->byteCount != 0x3FFF)
            {
                descrPtr->byteCount -= (descrPtr->l3StartOffsetPtr - descrPtr->startFramePtr);
            }

            descrPtr->startFramePtr = descrPtr->l3StartOffsetPtr;
            /* reset the L3 pointer , because it is not relevant any more -->
                       will be set inside snetChtL2Parsing(...) for the 'passenger'  */
            descrPtr->l3StartOffsetPtr = NULL;
            descrPtr->macDaPtr = DST_MAC_FROM_TUNNEL_DSCR(descrPtr);
            descrPtr->macSaPtr = SRC_MAC_FROM_TUNNEL_DSCR(descrPtr);

            /* do re-parsing on the passenger , to set the L2 info
                       (must be done prior to setting the VID from the TTI action ,
                       and other QOS setting of TTI) */
            snetChtL2Parsing(devObjPtr,descrPtr,SNET_CHT_FRAME_PARSE_MODE_FROM_TTI_PASSENGER_E,internalTtiInfoPtr);
            descrPtr->passengerLength = descrPtr->byteCount - (descrPtr->l3StartOffsetPtr - descrPtr->startFramePtr);

            /* restore the vid0 before the mac to me lookup */
            if(actionDataPtr->vid0Cmd == 0)
            {
                /* not allowing the re-parse to override the previous assignment */
                __LOG(("The TTI action vid0Cmd force the eVlan to use the value before the 're-parse' ! \n"
                       "so the value [0x%x] is ignored and using[0x%x] \n",
                       preAction_eVid0,
                       descrPtr->eVid));

                /* restore vid0 as was before the L2 parsing */
                descrPtr->eVid = preAction_eVid0;
                descrPtr->vidModified = preAction_vidModified;
            }

            if(isTransitTunnel == GT_FALSE)/* tunnel terminated - not transit */
            {
                /* mac-to-me lookup -- on the passenger */
                snetCht3Mac2MeTblLookUp(devObjPtr, descrPtr);
            }
            break;

        default:
            break;
    }

    if(descrPtr->tunnelTerminated &&
       ((descrPtr->innerPacketType == SKERNEL_INNER_PACKET_TYPE_IP) ||
        (descrPtr->innerPacketType == SKERNEL_INNER_PACKET_TYPE_MPLS)) )
    {
        /* implicit <bypassBridge> */
        __LOG(("implicit <bypassBridge> for passenger of IP or MPLS \n"));
        descrPtr->bypassBridge = 1;
        __LOG_PARAM(descrPtr->bypassBridge);
    }

    descrPtr->l3StartOfPassenger = descrPtr->l3StartOffsetPtr;
}

/**
* @internal snetXcatTTActionApplyMplsQos function
* @endinternal
*
* @brief   TT mpls qos processing
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] descrPtr                 - pointer to the frame descriptor.
* @param[in] actionDataPtr            - pointer to TTI action
* @param[in] trustQosMappingTableIndex - trust qos mapping table index
*                                      OUTPUT:
* @param[in,out] descrPtr                 - pointer to the frame descriptor.
*/
static GT_VOID snetXcatTTActionApplyMplsQos
(
    IN    SKERNEL_DEVICE_OBJECT             *devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC   *descrPtr,
    IN    SNET_XCAT_TT_ACTION_STC           *actionDataPtr,
    IN    GT_U32                             trustQosMappingTableIndex
)
{
    DECLARE_FUNC_NAME(snetXcatTTActionApplyMplsQos);

    SNET_E_ARCH_TT_ACTION_STC   *actionDataEArchPtr;/*E-Arch action info*/
    GT_BIT  mpls;
    GT_U32  regAddr;                /* Register's address */
    GT_U32  fldFirstBit;            /* Register field's first bit */
    GT_U32  exp = 0;
    GT_U32  *memPtr;                /*pointer to memory*/
    GT_U32  fieldVal;               /* Register's field value */
    GT_U32  qosPrifileNumBits;/* number of bits that the qos profile uses in next tables */
    GT_BIT  copyExpFromOuterHeader;

    actionDataEArchPtr = (SNET_E_ARCH_TT_ACTION_STC*)(void*)actionDataPtr;

    SKERNEL_CHT_DESC_INNER_FRAME_FIELD_GET_MAC(descrPtr,mpls);

    if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        qosPrifileNumBits = 10;
    }
    else
    {
        qosPrifileNumBits = 7;
    }

    if(mpls)
    {
        __LOG_PARAM(actionDataPtr->trustExp);
        if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
        {
            __LOG_PARAM(actionDataEArchPtr->lLspQosProfileEnable);
        }
    }

    if (actionDataPtr->trustExp && mpls)
    {
        copyExpFromOuterHeader =  SMEM_CHT_IS_SIP5_GET(devObjPtr) ?
                    actionDataPtr->copyTtlFromOuterHeader : 0;

        __LOG_PARAM(copyExpFromOuterHeader);

        switch(actionDataPtr->mplsCmd)
        {
            case SKERNEL_XCAT_TTI_MPLS_NOP_E: /* Nop  */
            case SKERNEL_XCAT_TTI_MPLS_SWAP_E: /* Swap */
            case SKERNEL_XCAT_TTI_MPLS_PUSH_E: /* Push */
                exp = descrPtr->exp[0];
                __LOG(("(for qosProfile)use EXP[%d] from most outer label (Nop/Swap/Push) \n",
                    exp));
                break;
            case SKERNEL_XCAT_TTI_MPLS_POP_1_E: /* Pop one label */
            case SKERNEL_XCAT_TTI_MPLS_POP_SWAP_E: /* Pop and Swap  */
                if(copyExpFromOuterHeader == 0)
                {
                    exp = descrPtr->exp[1];
                    __LOG(("(for qosProfile)use EXP[%d] from second outer label (Pop one label/Pop and Swap)\n",
                        exp));
                }
                else
                {
                    exp = descrPtr->exp[0];
                    __LOG(("(for qosProfile)use EXP[%d] from first outer label (the popped label) (Pop one label/Pop and Swap)\n",
                        exp));
                }


                break;
            case SKERNEL_XCAT_TTI_MPLS_POP_2_E: /* Pop two labels */
                if(copyExpFromOuterHeader == 0)
                {
                    exp = descrPtr->exp[2];
                    __LOG(("(for qosProfile)use EXP[%d] from third outer label (since Popped two labels) \n",
                        exp));
                }
                else
                {
                    exp = descrPtr->exp[1];
                    __LOG(("(for qosProfile)use EXP[%d] from second outer label (the last popped label) (since Popped two labels)\n",
                        exp));
                }
                break;
            default:
                __LOG(("unsupported case actionDataPtr->mplsCmd[%d] \n",
                    actionDataPtr->mplsCmd));
                break;
            case SKERNEL_XCAT_TTI_MPLS_POP2_SWAP_E :
                __LOG(("use EXP from third outer label (since Popped two + swap) \n"));
                exp = descrPtr->exp[2];
                break;
            case SKERNEL_XCAT_TTI_MPLS_POP_3_E :
                __LOG(("use EXP from fourth outer label (since Popped three labels) \n"));
                exp = descrPtr->exp[3];
                break;
            case SKERNEL_XCAT_TTI_MPLS_POP3_SWAP_E :
                __LOG(("use EXP from fourth outer label (since Popped three + swap) \n"));
                exp = descrPtr->exp[3];
                break;
        }

        __LOG_PARAM(exp);
        __LOG_PARAM(trustQosMappingTableIndex);

        __LOG(("action.TrustExp = 1 so get qosProfile ,for EXP[%d] from trustQosMappingTableIndex[%d] \n",
            trustQosMappingTableIndex , exp));

        if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
        {
            memPtr = smemMemGet(devObjPtr,
                     SMEM_CHT_EXP_2_QOS_PROF_MAP_TBL_MEM(devObjPtr,0,trustQosMappingTableIndex));

            descrPtr->qos.qosProfile = snetFieldValueGet(memPtr,qosPrifileNumBits * exp,qosPrifileNumBits);
        }
        else
        {
            regAddr = SMEM_CHT_EXP_2_QOS_PROF_MAP_TBL_MEM(devObjPtr, exp,0);
            fldFirstBit = (exp % 4) * 8;
            smemRegFldGet(devObjPtr, regAddr, fldFirstBit, qosPrifileNumBits,  &fieldVal);
            /* EXP<4n>2QoSProfile */
            descrPtr->qos.qosProfile = fieldVal;
        }
    }
    else if(mpls && (actionDataPtr->trustExp == 0) &&
            devObjPtr->supportEArch && actionDataEArchPtr->lLspQosProfileEnable)
    {
        __LOG(("for action.trustExp == 0 && action.lLspQosProfileEnable = 1 use qosProfile = (packet.exp1 + action.qosProfile << 3) \n"));
        descrPtr->qos.qosProfile = descrPtr->exp[0] + (actionDataPtr->qoSProfile << 3);
    }
    else
    {
        __LOG(("use qosProfile from action.qosProfile \n"));
        descrPtr->qos.qosProfile = actionDataPtr->qoSProfile;
    }

    __LOG(("descrPtr->qos.qosProfile = %d \n", descrPtr->qos.qosProfile));

    /* the trust L2/L3 info need to be done on the passenger info , so
       we apply this only on 'part 2' of action --> see function :
       snetXcatTTActionApplyPart2(...) */
}

/**
* @internal snetXcatTTActionApplyMplsPwTagMode function
* @endinternal
*
* @brief   TT mpls pseudo wire tag mode processing
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] descrPtr                 - pointer to the frame descriptor.
* @param[in] actionDataPtr            - pointer to TTI action
* @param[in] pwTagMode                - pseudo wire tag mode
*                                      OUTPUT:
* @param[in,out] descrPtr                 - pointer to the frame descriptor.
* @param[in] allowVid1AssignmentPtr   - indicate is allow vid1 assignment
*/
static GT_VOID snetXcatTTActionApplyMplsPwTagMode
(
    IN    SKERNEL_DEVICE_OBJECT             *devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC   *descrPtr,
    IN    SNET_XCAT_TT_ACTION_STC           *actionDataPtr,
    IN    GT_U32                             pwTagMode,
    OUT   GT_U32                            *allowVid1AssignmentPtr
)
{
    DECLARE_FUNC_NAME(snetXcatTTActionApplyMplsPwTagMode);

    GT_U32  newAfterVlanOrDsaOffset;/* new offset of after vlan or DSA tag */
    GT_U32  fieldVal;               /* Register's field value */

    switch(pwTagMode)
    {
    case 0:  /*Disabled (BWC mode)*/
        /*DescOut<Override VID0 with OrigVID> = 0*/
        descrPtr->overrideVid0WithOrigVid = 0;
        if(devObjPtr->vplsModeEnable.tti)
        {
            /*DescOut<OrigVID> = 0*/
            descrPtr->vid0Or1AfterTti = 0;
        }
        break;
    case 1:  /*RAW PW Mode (previously called <User VLAN Mode>)
                inner packet outer tag (VID0) is the user tag, used for egress VLAN filtering.
                If not found, use TTI AE <VID1>.
                If it is found packet will egress with this tag as Tag1*/

         /* DescOut<VID1> = Tag0 found in inner packet ? Inner Packet VID0 : 0
            - DescOut<UP1> = Tag0 found in inner packet ? Inner Packet UP0 : 0
            - DescOut<CFI1> = Tag0 found in inner packet ? Inner Packet CFI0 : 0
            - DescOut<OrigVID> = Tag0 found in inner packet ? Inner Packet VID0 : TTI AE<VID1>
            - DescOut<Override VID0 with OrigVID> = 1 */
        descrPtr->overrideVid0WithOrigVid = 1;

        if(TAG0_EXIST_MAC(descrPtr))
        {
            descrPtr->vid1 = descrPtr->eVid & 0xFFF;
            descrPtr->up1 = descrPtr->up;
            descrPtr->cfidei1 = descrPtr->cfidei;
            descrPtr->vid0Or1AfterTti = descrPtr->eVid;

            if(TAG1_EXIST_MAC(descrPtr))
            {
                /* modify indication that the packet has no tag1 , because it should be treated as 'payload' */
                __LOG(("modify indication that the packet has no tag1 , because it should be treated as 'payload'"));
                descrPtr->ingressVlanTag1Type = SKERNEL_FRAME_TR101_VLAN_TAG_NOT_FOUND_E;

                if(descrPtr->marvellTagged == 0)
                {
                    newAfterVlanOrDsaOffset = 4;
                }
                else  /* DSA tag */
                {
                    /* Check extended DSA tag bit */
                    __LOG(("Check extended DSA tag bit"));
                    if (descrPtr->marvellTaggedExtended == SKERNEL_EXT_DSA_TAG_4_WORDS_E)
                    {
                        newAfterVlanOrDsaOffset = 4*4;/* skip 4 word of DSA */
                    }
                    else if (descrPtr->marvellTaggedExtended == SKERNEL_EXT_DSA_TAG_3_WORDS_E)
                    {
                        newAfterVlanOrDsaOffset = 4*3;/* skip 3 word of DSA */
                    }
                    else if (descrPtr->marvellTaggedExtended == SKERNEL_EXT_DSA_TAG_2_WORDS_E)
                    {
                        newAfterVlanOrDsaOffset = 4*2;/* skip 2 words of DSA */
                    }
                    else
                    {
                        newAfterVlanOrDsaOffset = 4*1;/* skip 1 word of DSA */
                    }
                }

                /* also fix the length of 'parsed tagges' */
                __LOG(("also fix the length of 'parsed tagges'"));
                descrPtr->afterVlanOrDsaTagPtr -= (descrPtr->origVlanTagLength - newAfterVlanOrDsaOffset);
                descrPtr->afterVlanOrDsaTagLen += (descrPtr->origVlanTagLength - newAfterVlanOrDsaOffset);

                /* update the length that only single tag found */
                __LOG(("update the length that only single tag found"));
                descrPtr->origVlanTagLength = newAfterVlanOrDsaOffset;
            }
        }
        else
        {
            descrPtr->vid1 = 0;
            descrPtr->up1 = 0;
            descrPtr->cfidei1 = 0;
            descrPtr->vid0Or1AfterTti = actionDataPtr->vid1;
        }

        /*Note: The TTI AE <VID1 Command> is always ignored and does not affect the DescOut<VID1>.*/
        if(!SMEM_CHT_IS_SIP5_10_GET(devObjPtr))
        {
            __LOG(("Note: The TTI AE <VID1 Command> is always ignored and does not affect the DescOut<VID1>."));
            *allowVid1AssignmentPtr = 0;
        }

        break;
    case 2: /*TAGGED PW Mode (previously called <Service VLAN Push/Swap Mode>)
            Inner packet outer tag (VID0) is the P-Tag, which should always exist.
            If tag0 not found, this should be an exception. TBD assign a trap/drop command
            Inner packet inner tag (VID1) is the user tag, used for egress filtering. If not found, use TTI AE
            <VID1>.
            */
        /* - DescOut<VID1> = Tag1 found in inner packet ? Inner Packet VID1 : 0
            - DescOut<UP1> = Tag1 found in inner packet ? Inner Packet UP1 : 0
            - DescOut<CFI1> = Tag1 found in inner packet ? Inner Packet CFI1 : 0
            - DescOut<OrigVID> = Tag1 found in inner packet ? Inner Packet VID1 : TTI AE<VID1>
            - DescOut<Override VID0 with OrigVID> = 1.
            - Note: The TTI AE <VID1 Command> is always ignored and does not affect the DescOut<VID1>.  */

        /* Inner packet outer tag (VID0) is the P-Tag, which should always exist.
           If tag0 not found, this should be an exception*/
        if(! TAG0_EXIST_MAC(descrPtr))
        {

            if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
            {
                /* need to get action and CPU code for the packet */
                smemRegGet(devObjPtr,SMEM_LION3_TTI_PW_TAG_MODE_EXCEPTION_CONFIG_REG(devObjPtr), &fieldVal);

                __LOG(("Assign packet command according to TAG PW No Inner Tag Exception commands"));

                snetChtIngressCommandAndCpuCodeResolution(devObjPtr,descrPtr,
                                                          descrPtr->packetCmd,
                                                          SMEM_U32_GET_FIELD(fieldVal,0,3),/*TAG PW No Inner Tag Exception Command */
                                                          descrPtr->cpuCode,
                                                          SMEM_U32_GET_FIELD(fieldVal,3,8),/*TAG PW No Inner Tag Exception CPU Code*/
                                                          SNET_CHEETAH_ENGINE_UNIT_TTI_E,
                                                          GT_FALSE);
            }
            else
            {
                /* need to get action and CPU code for the packet */
                smemRegGet(devObjPtr,SMEM_XCAT_TTI_PSEUDO_WIRE_CONFIG_CONF_REG(devObjPtr), &fieldVal);

                /*have impact if former packet command is FWD*/
                if(descrPtr->packetCmd == SKERNEL_EXT_PKT_CMD_FORWARD_E)
                {
                    /* resolve packet command and CPU code */
                    snetChtIngressCommandAndCpuCodeResolution(devObjPtr,descrPtr,
                                                              descrPtr->packetCmd,
                                                              SMEM_U32_GET_FIELD(fieldVal,12,3),/*PW No Inner Tag Exception Command*/
                                                              descrPtr->cpuCode,
                                                              SMEM_U32_GET_FIELD(fieldVal,15,8),/*PW No Inner Tag Exception CPU Code*/
                                                              SNET_CHEETAH_ENGINE_UNIT_TTI_E,
                                                              GT_FALSE);
                }
            }
        }


        descrPtr->overrideVid0WithOrigVid = 1;

        if(TAG1_EXIST_MAC(descrPtr))
        {
            /*descrPtr->vid1 = descrPtr->vid1;      */  /* no change from L2 parser */
            /*descrPtr->up1 = descrPtr->up1;        */  /* no change from L2 parser */
            /*descrPtr->cfidei1 = descrPtr->cfidei1;*/  /* no change from L2 parser */
            descrPtr->vid0Or1AfterTti = descrPtr->vid1;
        }
        else
        {
            descrPtr->vid1 = 0;                     /* no change from L2 parser */
            descrPtr->up1 = 0;                      /* no change from L2 parser */
            descrPtr->cfidei1 = 0;                  /* no change from L2 parser */
            descrPtr->vid0Or1AfterTti = actionDataPtr->vid1;
        }

        /*Note: The TTI AE <VID1 Command> is always ignored and does not affect the DescOut<VID1>.*/
        if(!SMEM_CHT_IS_SIP5_10_GET(devObjPtr))
        {
            *allowVid1AssignmentPtr = 0;
        }
        break;
    case 3: /*Service VLAN Keep Mode - Obsolete - Do not Verify */
        /*
        - DescOut<OrigVID> = Tag0 found in inner packet ? Inner Packet VID0 : 0
        - DescOut<Override VID0 with OrigVID> = 1
        */
        if(TAG0_EXIST_MAC(descrPtr))
        {
            descrPtr->vid0Or1AfterTti = descrPtr->eVid;
        }
        else
        {
            descrPtr->vid0Or1AfterTti = 0;
        }
        descrPtr->overrideVid0WithOrigVid = 1;
        break;
    default:/* should not happen*/
        __LOG(("should not happen"));
        break;
    }

    __LOG(("pwTagMode = %d", pwTagMode));
}


/**
* @internal snetXcatTTActionCheckAndApplyMplsPwTagMode function
* @endinternal
*
* @brief   TT mpls pseudo wire tag mode processing
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] descrPtr                 - pointer to the frame descriptor.
* @param[in] actionDataPtr            - pointer to TTI action
*                                      OUTPUT:
* @param[in,out] descrPtr                 - pointer to the frame descriptor.
* @param[in] allowVid1AssignmentPtr   - indicate is allow vid1 assignment
*/
static GT_VOID snetXcatTTActionCheckAndApplyMplsPwTagMode
(
    IN    SKERNEL_DEVICE_OBJECT             *devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC   *descrPtr,
    IN    SNET_XCAT_TT_ACTION_STC           *actionDataPtr,
    OUT   GT_U32                            *allowVid1AssignmentPtr
)
{
    DECLARE_FUNC_NAME(snetXcatTTActionCheckAndApplyMplsPwTagMode);

    GT_U32  pwTagMode = 0;    /* the PW Tag Mode */

    SNET_E_ARCH_TT_ACTION_STC   *actionDataEArchPtr;/*E-Arch action info*/

    actionDataEArchPtr = (SNET_E_ARCH_TT_ACTION_STC*)(void*)actionDataPtr;

    if(devObjPtr->vplsModeEnable.tti &&
       actionDataPtr->redirectCmd == PCL_TTI_ACTION_REDIRECT_CMD_ASSIGN_SOURCE_LOGICAL_PORT_E)
    {
        /* this configuration is relevant only when TTI AE<TT>==1, and is applied AFTER parsing the
           inner Ethernet packet but BEFORE applying the TTI AE<VID0 Command> */
        pwTagMode = descrPtr->vplsInfo.pwTagMode;
    }
    else if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        pwTagMode = actionDataEArchPtr->pwTagMode;
    }
    else
    {
        __LOG(("PW tag mode is not supported"));
        return;
    }

    if(descrPtr->tunnelTerminated && descrPtr->l2Valid)
    {
        /* parsing of PW (Pseudo Wire) tag mode */
        snetXcatTTActionApplyMplsPwTagMode(devObjPtr, descrPtr, actionDataPtr,
                                            pwTagMode, allowVid1AssignmentPtr);
    }
}


/**
* @internal snetXcatTTActionApplyMplsCwBasedPw_legacy function
* @endinternal
*
* @brief   TT action apply mpls cw based pw processing
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] descrPtr                 - pointer to the frame descriptor.
* @param[in] actionDataPtr            - pointer to TTI action
* @param[in] firstNibbleAfterSBitLabel - first nibble after sbit label
*                                      OUTPUT:
* @param[in,out] descrPtr                 - pointer to the frame descriptor.
*/
static GT_VOID snetXcatTTActionApplyMplsCwBasedPw_legacy
(
    IN    SKERNEL_DEVICE_OBJECT             *devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC   *descrPtr,
    IN    SNET_XCAT_TT_ACTION_STC           *actionDataPtr,
    IN    GT_U32                             firstNibbleAfterSBitLabel
)
{
    DECLARE_FUNC_NAME(snetXcatTTActionApplyMplsCwBasedPw_legacy);

    GT_U32  fieldVal;               /* Register's field value */

    /*those actions are valid regardless to MPLS key */
    __LOG(("those actions are valid regardless to MPLS key"));
    descrPtr->vplsInfo.egressTagState          = actionDataPtr->vplsInfo.egressTagState;
    descrPtr->vplsInfo.egressTagStateAssigned  = actionDataPtr->vplsInfo.egressTagStateAssigned;
    descrPtr->vplsInfo.unknownSaCmd            = actionDataPtr->vplsInfo.unknownSaCmd;
    descrPtr->vplsInfo.unknownSaCmdAssigned    = actionDataPtr->vplsInfo.unknownSaCmdAssigned;
    descrPtr->vplsInfo.pwHasCw                 = actionDataPtr->vplsInfo.pwHasCw;
    descrPtr->vplsInfo.pwTagMode               = actionDataPtr->vplsInfo.pwTagMode;
    descrPtr->vplsInfo.srcMeshId               = actionDataPtr->vplsInfo.srcMeshId;

    /* allow to do again the Ethernet/IPv4/IPv6 parsing */
    __LOG(("allow to do again the Ethernet/IPv4/IPv6 parsing"));
    descrPtr->didPacketParseFromEngine[SNET_CHT_FRAME_PARSE_MODE_FROM_TTI_PASSENGER_E] = GT_FALSE;

    if(descrPtr->vplsInfo.pwHasCw == 1)
    {
        if(0 != firstNibbleAfterSBitLabel)/*first nibble after the MPLS labels*/
        {
            /* the first nibble not equal 0 -->
            assign packet command <Non-Data CW Command> with CPU Code <Non-Data CW CPU code>*/

            /* need to get action and CPU code for the packet */
            __LOG(("need to get action and CPU code for the packet"));
            smemRegGet(devObjPtr,SMEM_XCAT_TTI_PSEUDO_WIRE_CONFIG_CONF_REG(devObjPtr), &fieldVal);

            /*have impact if former packet command is FWD*/
            __LOG(("have impact if former packet command is FWD"));
            if(descrPtr->packetCmd == SKERNEL_EXT_PKT_CMD_FORWARD_E)
            {
                /* resolve packet command and CPU code */
                snetChtIngressCommandAndCpuCodeResolution(devObjPtr,descrPtr,
                                                          descrPtr->packetCmd,
                                                          SMEM_U32_GET_FIELD(fieldVal,1,3),/*Non Data CW Exception Command*/
                                                          descrPtr->cpuCode,
                                                          SMEM_U32_GET_FIELD(fieldVal,4,8),/*Non Data CW Exception CPU Code*/
                                                          SNET_CHEETAH_ENGINE_UNIT_TTI_E,
                                                          GT_FALSE);
            }
        }
    }
}

/**
* @internal snetXcatTTActionApplyMplsCwBasedPw function
* @endinternal
*
* @brief   TT action apply mpls cw based pw processing
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] descrPtr                 - pointer to the frame descriptor.
* @param[in] actionDataPtr            - pointer to TTI action
* @param[in] isMplsLookup             - indicates is mpls lookup
* @param[in,out] internalTtiInfoPtr       - pointer to internal TTI info
*                                      OUTPUT:
* @param[in,out] descrPtr                 - pointer to the frame descriptor.
*/
static GT_VOID snetXcatTTActionApplyMplsCwBasedPw
(
    IN    SKERNEL_DEVICE_OBJECT             *devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC   *descrPtr,
    IN    SNET_XCAT_TT_ACTION_STC           *actionDataPtr,
    IN    GT_BOOL                            isMplsLookup,
    INOUT INTERNAL_TTI_DESC_INFO_STC        *internalTtiInfoPtr
)
{
    DECLARE_FUNC_NAME(snetXcatTTActionApplyMplsCwBasedPw);

    SNET_E_ARCH_TT_ACTION_STC   *actionDataEArchPtr;/*E-Arch action info*/
    actionDataEArchPtr = (SNET_E_ARCH_TT_ACTION_STC*)(void*)actionDataPtr;

    if(SMEM_CHT_IS_SIP5_GET(devObjPtr) && isMplsLookup && actionDataEArchPtr->cwBasedPseudoWire)
    {
        __LOG_PARAM(actionDataEArchPtr->cwBasedPseudoWire);

        /* SIP5 devices support for Cw Based Pseudo Wire */
        snetLion3TTActionApplyMplsCwBasedPw(devObjPtr, descrPtr, actionDataPtr,
                                             internalTtiInfoPtr->firstNibbleAfterSBitLabel,
                                             internalTtiInfoPtr->pwe3CwWord);
    }

    if(devObjPtr->vplsModeEnable.tti)
    {
        /*legacy (XCAT_C0) device support for Cw Based Pseudo Wire*/
        snetXcatTTActionApplyMplsCwBasedPw_legacy(devObjPtr, descrPtr, actionDataPtr,
                                                     internalTtiInfoPtr->firstNibbleAfterSBitLabel);
    }
}

/**
* @internal snetXcatTTActionApplyRedirectCmdLionB0 function
* @endinternal
*
* @brief   TT redirect cmd processing. Relevant for LionB0 only.
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] descrPtr                 - pointer to the frame descriptor.
* @param[in] actionDataPtr            - pointer to TTI action
*                                      OUTPUT:
* @param[in,out] descrPtr                 - pointer to the frame descriptor.
*/
static GT_VOID snetXcatTTActionApplyRedirectCmdLionB0
(
    IN    SKERNEL_DEVICE_OBJECT             *devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC   *descrPtr,
    IN    SNET_XCAT_TT_ACTION_STC           *actionDataPtr
)
{
    DECLARE_FUNC_NAME(snetXcatTTActionApplyRedirectCmdLionB0);

    SNET_E_ARCH_TT_ACTION_STC   *actionDataEArchPtr;/*E-Arch action info*/
    SNET_LION_TT_ACTION_STC     *lionActionDataPtr;

    actionDataEArchPtr = (SNET_E_ARCH_TT_ACTION_STC*)(void*)actionDataPtr;
    lionActionDataPtr = &actionDataEArchPtr->baseActionData;


    switch(actionDataPtr->redirectCmd)
    {
       /* Do not redirect this packet */
       case PCL_TTI_ACTION_REDIRECT_CMD_NONE_E:
           __LOG(("Do not redirect this packet"));
           if(lionActionDataPtr->multiPortGroupTTIEn)
           {
               /* make sure to override the port 62 that was assigned in ingress */
               __LOG(("make sure to override the port 62 that was assigned in ingress"));
               descrPtr->useVidx = 0;
               descrPtr->trgDev = descrPtr->ownDev;
               descrPtr->trgEPort = 0;
                /* call after setting trgEPort */
               __LOG(("call after setting trgEPort"));
                SNET_E_ARCH_CLEAR_IS_TRG_PHY_PORT_VALID_MAC(devObjPtr,descrPtr,tti);
               descrPtr->targetIsTrunk = 0;
           }

           if(SMEM_CHT_IS_SIP5_GET(devObjPtr) && actionDataEArchPtr->flowId)
           {
               /* override only when non zero */
               descrPtr->flowId = actionDataEArchPtr->flowId;
           }

           break;
       /* Redirect this packet to egress interface */
       case PCL_TTI_ACTION_REDIRECT_CMD_OUT_IF_E:
           __LOG(("Redirect this packet to egress interface"));
           descrPtr->modifyMacDa = lionActionDataPtr->modifyMacDa;
           descrPtr->modifyMacSa = lionActionDataPtr->modifyMacSa;
           break;
       default:
           break;
   }

   descrPtr->ttiHashMaskIndex = lionActionDataPtr->hashMaskIndex;
}

/**
* @internal lion3TtiSrcIdMaskGet function
* @endinternal
*
* @brief   Lion3 : Get the srcIdMask for the lookup.
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] descrPtr                 - pointer to frame descriptor.
* @param[in,out] internalTtiInfoPtr       - (pointer to) internal TTI info
*                                      using internalTtiInfoPtr->actionId
* @param[in,out] internalTtiInfoPtr       - (pointer to) internal TTI info
*                                      value returned within the internalTtiInfoPtr->srcIdMask
*                                      RETURN:
*                                      COMMENTS:
*/
static GT_VOID lion3TtiSrcIdMaskGet
(
    IN    SKERNEL_DEVICE_OBJECT             *devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC   *descrPtr,
    INOUT INTERNAL_TTI_DESC_INFO_STC        *internalTtiInfoPtr
)
{
    DECLARE_FUNC_NAME(lion3TtiSrcIdMaskGet);

    GT_U32 regAddr;
    GT_U32 regValue;

    if(SMEM_CHT_IS_SIP7_GET(devObjPtr))
    {
        regAddr = SMEM_SIP7_TTI_LU_UNIT_TT_SRC_ID_ASSIGN_REG(devObjPtr,internalTtiInfoPtr->actionId);

        smemRegGet(devObjPtr,regAddr , &regValue);

        internalTtiInfoPtr->srcIdMask = SMEM_U32_GET_FIELD(regValue,0,12);
        __LOG_PARAM(internalTtiInfoPtr->srcIdMask);

        return;
    }

    regAddr = internalTtiInfoPtr->actionId <= 1 ?  SMEM_LION3_TTI_SOURCE_ID_ASSIGNMENT_REG(devObjPtr):
                                                  SMEM_LION3_TTI_SOURCE_ID_ASSIGNMENT1_REG(devObjPtr);

    smemRegGet(devObjPtr,regAddr , &regValue);

    internalTtiInfoPtr->srcIdMask = SMEM_U32_GET_FIELD(regValue,((internalTtiInfoPtr->actionId%2) * 12),12);
    __LOG_PARAM(internalTtiInfoPtr->srcIdMask);

}


/**
* @internal snetXcatTTActionApplyGetPclLookupMode function
* @endinternal
*
* @brief   Apply the action entry from the TTI action table: get pcl lookup mode
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] descrPtr                 - pointer to frame descriptor.
*                                      actionInfoPtr      - pointer to the tti action entry.
* @param[in,out] descrPtr                 - pointer to frame descriptor.
*                                      RETURN:
*                                      COMMENTS:
*/
static GT_VOID snetXcatTTActionApplyGetPclLookupMode
(
    IN    SKERNEL_DEVICE_OBJECT             *devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC   *descrPtr,
    IN    SNET_XCAT_TT_ACTION_STC           *actionDataPtr
)
{
    DECLARE_FUNC_NAME(snetXcatTTActionApplyGetPclLookupMode);

    if(actionDataPtr->policy0LookupMode)
    {
        descrPtr->pclLookUpMode[0] = SKERNEL_PCL_LOOKUP_MODE_DESCRIPTOR_INDEX_E;
        __LOG_PARAM(descrPtr->pclLookUpMode[0]);
    }

    if(actionDataPtr->policy1LookupMode)
    {
        descrPtr->pclLookUpMode[1] = SKERNEL_PCL_LOOKUP_MODE_DESCRIPTOR_INDEX_E;
        __LOG_PARAM(descrPtr->pclLookUpMode[1]);
    }

    if(actionDataPtr->policy2LookupMode)
    {
        descrPtr->pclLookUpMode[2] = SKERNEL_PCL_LOOKUP_MODE_DESCRIPTOR_INDEX_E;
        __LOG_PARAM(descrPtr->pclLookUpMode[2]);
    }
}

/**
* @internal snetLion3TTActionApplyTtlExpiryVccvEnable function
* @endinternal
*
* @brief   Ttl Expiry Vccv Enable check
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] descrPtr                 - pointer to frame descriptor.
* @param[in] isPwe3FlowLabelExist     - indication of Pwe3 Flow Label Exist
* @param[in,out] descrPtr                 - pointer to frame descriptor.
*                                      RETURN:
*                                      COMMENTS:
*/
static GT_VOID snetLion3TTActionApplyTtlExpiryVccvEnable
(
    IN    SKERNEL_DEVICE_OBJECT             *devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC   *descrPtr,
    IN    GT_U32                             isPwe3FlowLabelExist
)
{
    DECLARE_FUNC_NAME(snetLion3TTActionApplyTtlExpiryVccvEnable);

    GT_U32 pwLabelIndex; /* the PW label index */
    GT_U32 pwConfig;
    GT_U32 pwe3CpuCodeBase;
    GT_U32 ttl;

   /* get PW configurations */
   smemRegGet(devObjPtr,SMEM_XCAT_TTI_PSEUDO_WIRE_CONFIG_CONF_REG(devObjPtr), &pwConfig);

   /*get PWE3 CPU Code Base*/
   pwe3CpuCodeBase = SMEM_U32_GET_FIELD(pwConfig,15,8);
   __LOG(("PWE3 CPU Code Base[%d] \n",pwe3CpuCodeBase));

    /* get index of the PW_LABEL */
    if(isPwe3FlowLabelExist)
    {
        __LOG(("the FLOW label Exists so the Pseudo Wire is the one before it \n"));
        if(descrPtr->numOfLabels)
        {
            pwLabelIndex = descrPtr->numOfLabels - 1;
        }
        else
        {
            __LOG(("ERROR : only one label recognized but we have FLOW label and Pseudo Wire label ?? \n"));
            pwLabelIndex = SMAIN_NOT_VALID_CNS;
        }
    }
    else
    {
        pwLabelIndex = descrPtr->numOfLabels;
    }

    __LOG(("Pseudo Wire label index is [%d] \n",pwLabelIndex));

    switch(pwLabelIndex)
    {
        case 0:
            /* take from first label */
            ttl = descrPtr->ttlArr[0];/*ttl*/
            break;
        case 1:
            /* take from second label */
            ttl = descrPtr->ttlArr[1];/*ttl*/
            break;
        case 2:
            /* take from third label */
            ttl = descrPtr->ttlArr[2];/*ttl*/
            break;
        case 3:
            /* take from fourth label */
            ttl = descrPtr->ttlArr[3];/*ttl*/
            break;
        default:
            /* Error indication */
            ttl = 0;/*ttl*/
            break;
    }

    /* IF ((PW_LABEL[TTL] <= 1) & (TTI-AE<TTL Expiry VCCV Enable> = Enable))*/
    if(ttl <= 1 )
    {
        __LOG(("Assign packet command according to TTL Expiry VCCV Command"));
        snetChtIngressCommandAndCpuCodeResolution(devObjPtr, descrPtr,
                                                 descrPtr->packetCmd,
                                                 SMEM_U32_GET_FIELD(pwConfig,3,3),/*TTL Expiry VCCV Cmd*/
                                                 descrPtr->cpuCode,
                                                 pwe3CpuCodeBase,/*TTL Expiry VCCV CPU Code*/
                                                 SNET_CHEETAH_ENGINE_UNIT_TTI_E,
                                                 GT_FALSE);
    }

}

/**
* @internal snetXcatTTActionApply function
* @endinternal
*
* @brief   Apply the action entry from the TTI action table.
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] descrPtr                 - pointer to frame descriptor.
*                                      actionInfoPtr      - pointer to the tti action entry.
* @param[in,out] internalTtiInfoPtr       - (pointer to) internal TTI info
* @param[in,out] descrPtr                 - pointer to frame descriptor.
* @param[in,out] internalTtiInfoPtr       - (pointer to) internal TTI info
*                                      RETURN:
*                                      COMMENTS:
*/
static GT_VOID snetXcatTTActionApply
(
    IN    SKERNEL_DEVICE_OBJECT             *devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC   *descrPtr,
    IN    SNET_XCAT_TT_ACTION_STC           *actionDataPtr,
    INOUT INTERNAL_TTI_DESC_INFO_STC        *internalTtiInfoPtr
)
{
    DECLARE_FUNC_NAME(snetXcatTTActionApply);

    GT_BIT   allowVid1Assignment = 1;  /* allow vid1 assignment to the descrPtr->vid1 */
    GT_BOOL  isMplsLookup;
    GT_U32   trustQosMappingTableIndex;/* QoS mapping table index */
    GT_U32   origSstId;                /* sstId of the incoming descriptor*/
    GT_U32   lookupNum;
    GT_U32   startByte, i;

    SKERNEL_QOS_PROF_PRECED_ENT previousEngineQosProfilePrecedence = descrPtr->qosProfilePrecedence;

    SNET_E_ARCH_TT_ACTION_STC   *actionDataEArchPtr;/*E-Arch action info*/

    /*
        This is 'C++' emulation for casting a class (struct).
        The original code used : SNET_XCAT_TT_ACTION_STC
        And we added more lion info into : SNET_LION_TT_ACTION_STC
        And then we added more info into : SNET_E_ARCH_TT_ACTION_STC

        Get pointer to SNET_XCAT_TT_ACTION_STC
        But it actually holding data of : actionDataEArch.baseActionData.xcatTTActionData
        Which is the first element in SNET_E_ARCH_TT_ACTION_STC
    */
    actionDataEArchPtr = (SNET_E_ARCH_TT_ACTION_STC*)(void*)actionDataPtr;

    isMplsLookup = checkIsMplsLookup(devObjPtr, internalTtiInfoPtr);

    if(SMEM_CHT_IS_SIP7_GET(devObjPtr))
    {
        snetAasUpdateForwardingLabelInfo(devObjPtr, descrPtr, actionDataPtr->mplsCmd);
    }
    else
    {
        descrPtr->mplsCommand = actionDataPtr->mplsCmd;
    }

    descrPtr->tunnelTerminated = actionDataPtr->tunnelTerminateEn;

    /* If all the tunnel labels needs to be popped then this scenario is similar to
     * tunnel termination. Set the tunnelTerminate flag */
    if(SMEM_CHT_IS_SIP7_GET(devObjPtr))
    {
        if (descrPtr->fwdLabelsPopSwap > 0)
        {
            __LOG(("Check TT Scenario: isLabelSwapped[%u], fwdLabelsPopSwap[%u], currFwdLabelIndex[%u], isLastLabelInStack[%u]=%u\n",
                descrPtr->isLabelSwapped, descrPtr->fwdLabelsPopSwap, descrPtr->currFwdLabelIndex,
                descrPtr->fwdLabelsPopSwap - 1, descrPtr->isLastLabelInStack[descrPtr->fwdLabelsPopSwap - 1]));
        }

        /* If (Atleast one label has been popped &&
               Label is not Swapped &&
               there are no more "Forwarding Labels" in the label stack after the current popped label &&
               BoS is found )
               {
                  Consider it as TT scenario
               } */
        if ((descrPtr->isLabelSwapped == 0) &&
            (descrPtr->fwdLabelsPopSwap > 0) &&
            (descrPtr->currFwdLabelIndex == descrPtr->fwdLabelsPopSwap) &&
            (descrPtr->isLastLabelInStack[descrPtr->fwdLabelsPopSwap - 1]))
        {
            __LOG(("All labels needs to be popped. Tunnel Termination case\n"));
            descrPtr->tunnelTerminated  = GT_TRUE;
            descrPtr->totalPoppedLabels = 0;
        }
    }
    else
    {
        __LOG(("Check TT Scenario: mplsCommand[%u], numOfLabels[%u]\n", descrPtr->mplsCommand, descrPtr->numOfLabels));
        if (((descrPtr->mplsCommand == SKERNEL_XCAT_TTI_MPLS_POP_1_E) && (descrPtr->numOfLabels==0)) ||
            ((descrPtr->mplsCommand == SKERNEL_XCAT_TTI_MPLS_POP_2_E) && (descrPtr->numOfLabels==1)) ||
            ((descrPtr->mplsCommand == SKERNEL_XCAT_TTI_MPLS_POP_3_E) && (descrPtr->numOfLabels==2)))
        {
            __LOG(("All labels needs to be popped. Tunnel Termination case\n"));
            descrPtr->tunnelTerminated = GT_TRUE;
        }
    }

    internalTtiInfoPtr->actionInfo.packetCmd = actionDataPtr->packetCmd;

    /* resolve packet command and CPU code */
    snetChtIngressCommandAndCpuCodeResolution(devObjPtr,descrPtr,
                                              descrPtr->packetCmd,
                                              actionDataPtr->packetCmd,
                                              descrPtr->cpuCode,
                                              actionDataPtr->userDefinedCpuCode,
                                              SNET_CHEETAH_ENGINE_UNIT_TTI_E,
                                              GT_FALSE);

    for(lookupNum = 1; lookupNum < internalTtiInfoPtr->maxNumOfLookups; lookupNum++)
    {
        if (GT_FALSE == internalTtiInfoPtr->lookupsInfo[lookupNum].isValid)
        {
            break;
        }

        snetChtIngressCommandAndCpuCodeResolution(devObjPtr,descrPtr,
                  descrPtr->packetCmd,
                  internalTtiInfoPtr->lookupsInfo[lookupNum].actionXcatPtr->packetCmd,
                  descrPtr->cpuCode,
                  internalTtiInfoPtr->lookupsInfo[lookupNum].actionXcatPtr->userDefinedCpuCode,
                  SNET_CHEETAH_ENGINE_UNIT_TTI_E,
                  GT_FALSE);
    }

    /* Ingress mirroring command */
    if(actionDataPtr->mirrorToAnalyzerPort == GT_TRUE)
    {
        descrPtr->rxSniff = 1;
    }

    /* Select mirror analyzer index */
    __LOG(("Select mirror analyzer index"));
    snetXcatIngressMirrorAnalyzerIndexSelect(devObjPtr, descrPtr, actionDataEArchPtr->analyzerIndex);

    descrPtr->ttiRedirectCmd = actionDataPtr->redirectCmd;

    switch (actionDataPtr->redirectCmd)
    {
        case PCL_TTI_ACTION_REDIRECT_CMD_OUT_IF_E:
            snetXcatTTActionApplyRedirectCmdOutIf(devObjPtr, descrPtr, actionDataPtr,
                                                  internalTtiInfoPtr, isMplsLookup);
            break;

        case PCL_TTI_ACTION_REDIRECT_CMD_LTT_ROUTER_E:
            descrPtr->ttRouterLTT = actionDataPtr->routerLTTIndex;
            break;

        case PCL_TTI_ACTION_REDIRECT_CMD_VIRT_ROUTER_E:
            __LOG(("Due to redirect command change descrPtr->vrfId from %d to %d",descrPtr->vrfId,actionDataPtr->vrfId));
            descrPtr->vrfId = actionDataPtr->vrfId;
            break;
        case PCL_TTI_ACTION_REDIRECT_CMD_ASSIGN_SOURCE_LOGICAL_PORT_E:/* Assign Logic Port */
            descrPtr->useVidx = 0;
            descrPtr->targetIsTrunk = actionDataPtr->srcLogicPortInfo.srcIsTrunk;
            if(descrPtr->targetIsTrunk)
            {
                descrPtr->trgTrunkId = actionDataPtr->srcLogicPortInfo.srcTrunkOrPortNum;
            }
            else
            {
                descrPtr->trgDev = actionDataPtr->srcLogicPortInfo.srcDevice;
                descrPtr->trgEPort = actionDataPtr->srcLogicPortInfo.srcTrunkOrPortNum;
            }
            break;
        case PCL_TTI_ACTION_REDIRECT_CMD_GENERIC_ACTION_E:/* Assign generic action */

            /* assign new value only for non zero one in action */
            if (actionDataEArchPtr->srcEpg)
            {
                descrPtr->srcEpg = actionDataEArchPtr->srcEpg;
            }

            if (actionDataEArchPtr->dstEpg)
            {
                descrPtr->dstEpg = actionDataEArchPtr->dstEpg;
            }

            break;
    }

    if(actionDataEArchPtr->ttiReservedAssignEnable)
    {
        /* copyReserved value assignment */
        descrPtr->copyReserved = actionDataEArchPtr->ttiReservedAssignValue;
        __LOG(("desc<copyReserved> = [%d]\n", descrPtr->copyReserved));
    }


    if(SMEM_CHT_IS_SIP5_GET(devObjPtr) &&
       descrPtr->tunnelTerminated == GT_FALSE)
    {
        if(isMplsLookup)
        {
            if(!descrPtr->enableL3L4ParsingOverMpls)
            {
                __LOG(("check for transit mpls tunnel \n"));
                passengerParsingOfTransitMplsTunnel(devObjPtr, descrPtr,
                                                    actionDataPtr, internalTtiInfoPtr);
            }
        }
        else
        { /* not mpls lookup */
            __LOG(("check for transit non-mpls tunnel \n"));
            passengerParsingOfTransitNonMplsTunnel(devObjPtr, descrPtr,
                                    actionDataPtr, internalTtiInfoPtr);
        }
    }

    descrPtr->ipclProfileIndex = actionDataPtr->ipclProfileIndex;

    /* Policer info */
    __LOG(("Get policer info"));
    if (descrPtr->policerEn == 0)
    {
        descrPtr->policerEn = actionDataPtr->bindToPolicerMeter;
    }
    if (descrPtr->policerCounterEn == 0)
    {
        descrPtr->policerCounterEn = actionDataPtr->bindToPolicerCounter;
    }
    if(descrPtr->policerEn == 1 || descrPtr->policerCounterEn == 1)
    {
        descrPtr->policerPtr = actionDataPtr->policerIndex;
    }

    if (actionDataPtr->srcIdSetEn == GT_TRUE)
    {
        origSstId = descrPtr->sstId;
        if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
        {
            internalTtiInfoPtr->actionId = 0;
            lion3TtiSrcIdMaskGet(devObjPtr,descrPtr,internalTtiInfoPtr);

            /* remove the bits that should be set by this logic */
            descrPtr->sstId &= ~internalTtiInfoPtr->srcIdMask;
            /* set the bits that should be set by this logic */
            descrPtr->sstId |= (actionDataPtr->srcId & internalTtiInfoPtr->srcIdMask);

            descrPtr->pclAssignedSstId |= internalTtiInfoPtr->srcIdMask;
        }
        else
        {
            descrPtr->sstId = actionDataPtr->srcId;
        }

        descrPtr->sstIdPrecedence = SKERNEL_PRECEDENCE_ORDER_HARD;

        __LOG(("SST assignment : sstId changed from [0x%x] to [0x%x] \n",
            origSstId,descrPtr->sstId));
    }

    for(lookupNum = 1; lookupNum < internalTtiInfoPtr->maxNumOfLookups; lookupNum++)
    {

        if(internalTtiInfoPtr->lookupsInfo[lookupNum].isValid &&
           internalTtiInfoPtr->lookupsInfo[lookupNum].actionXcatPtr->srcIdSetEn == GT_TRUE)
        {
            origSstId = descrPtr->sstId;
            internalTtiInfoPtr->actionId = lookupNum;
            lion3TtiSrcIdMaskGet(devObjPtr,descrPtr,internalTtiInfoPtr);

            /* remove the bits that should be set by this logic */
            descrPtr->sstId &= ~internalTtiInfoPtr->srcIdMask;
            /* set the bits that should be set by this logic */
            descrPtr->sstId |= (internalTtiInfoPtr->lookupsInfo[lookupNum].actionXcatPtr->srcId &
                            internalTtiInfoPtr->srcIdMask);

            descrPtr->pclAssignedSstId |= internalTtiInfoPtr->srcIdMask;
            __LOG(("SST assignment (next action) : sstId changed from [0x%x] to [0x%x] \n",
                        origSstId, descrPtr->sstId));
        }
    }

    descrPtr->ActionStop = actionDataPtr->actionStop;
    descrPtr->bypassBridge = actionDataPtr->bypassBridge;
    descrPtr->bypassIngressPipe = actionDataPtr->bypassIngressPipe;

    /* get pcl lookup mode */
    snetXcatTTActionApplyGetPclLookupMode(devObjPtr, descrPtr, actionDataPtr);

    if(actionDataPtr->vid0Cmd != 0)      /* eVlan Precedence Relevant only if the TTI Action <eVLAN Command> != DO NOT MODIFY */
    {
        descrPtr->preserveVid = actionDataPtr->vidPrecedence;
        __LOG_PARAM(descrPtr->preserveVid);
    }

    /* Nested Vid  */
    if(descrPtr->nestedVlanAccessPort == 0 && actionDataPtr->nestedVidEn)
    {
        /*When <EnNestedVLAN> is set, this rule matching flow is defined as a Nested VLAN Access Flow.*/
        descrPtr->nestedVlanAccessPort = 1;

        if(!SMEM_CHT_IS_SIP5_GET(devObjPtr))
        {
             /* When a packet received on an access flow is transmitted via a
               tagged port or a cascading port, a VLAN tag is added to the packet
               (on top of the existing VLAN tag, if any). The VID field is the
               VID assigned to the packet as a result of all VLAN assignment algorithms. */
            descrPtr->tagSrcTagged[SNET_CHT_TAG_0_INDEX_CNS] = 0;
            __LOG(("Nested vlan : set descrPtr->tagSrcTagged[SNET_CHT_TAG_0_INDEX_CNS] = 0 \n"));
        }
    }

    if (descrPtr->isTrillEtherType &&
        (internalTtiInfoPtr->ttiKeyType >= SKERNEL_LION3_TTI_KEY_UDB_PKT_TYPE_0_E &&
         internalTtiInfoPtr->ttiKeyType <= SKERNEL_LION3_TTI_KEY_UDB_PKT_TYPE_15_E))
    {
        /* do nothing */
        /*do not use : actionDataPtr->ttl */
        descrPtr->decTtl = actionDataPtr->ttlDecEn;
    }
    /* incoming MPLS TTL assignment */
    else if (isMplsLookup)
    {
        snetXcatTTActionApplyMplsTtl(devObjPtr, descrPtr, actionDataPtr);
    }
    else /* not MPLS and not TRILL */
    {
        /*do not use : actionDataPtr->ttl and  actionDataPtr->ttlDecEn */
    }

     /* QoS Marking Command */
    __LOG(("QoS Marking Command"));
    if(SMEM_CHT_IS_SIP7_GET(devObjPtr))
    {
        for(lookupNum = 0; lookupNum < internalTtiInfoPtr->maxNumOfLookups; lookupNum++)
        {
            snetAasQosActionResolution(devObjPtr, descrPtr, lookupNum, actionDataEArchPtr);
            snetAasTtlActionResolution(devObjPtr, descrPtr, lookupNum, actionDataEArchPtr);
        }
    }
    /* The TTI Action QoS marker operation ignores the QoS Precedence assigned
       by port/protocol markers */

    if(actionDataPtr->keepPreviousQoS == GT_FALSE)
    {
        /* according to HW designer the qosProfilePrecedence depend on the 'keep previous'
           (and not according to FS) */
        descrPtr->qosProfilePrecedence = actionDataPtr->qoSPrecedence;
    }

    /* BC2 does not allow the TTI Action to set ModifyUP and/or ModifyDSCP if previously assigned with QoSPrecedence==HARD*/
    if (devObjPtr->errata.ttiActionNotSetModifyUpDscpWhenQosPrecedenceHard && (previousEngineQosProfilePrecedence == SKERNEL_QOS_PROF_PRECED_HARD))
    {
        /* don't modify DSCP or UP according to ttiAction assignment */
        __LOG(("NOTE: WARNING : the TTI action can not change <modifyDscp> and <modifyUp> when prior qos_presedence is 'HARD' \n"));
    }
    else
    {
        /* value 0 means -- KEEP previous */
        if(actionDataPtr->modifyDscp == 1)/*enable modify*/
        {
            descrPtr->modifyDscp = GT_TRUE;
        }
        else if(actionDataPtr->modifyDscp == 2)/*disable modify*/
        {
            descrPtr->modifyDscp = GT_FALSE;
        }

        /* value 0 means -- KEEP previous */
        if(actionDataPtr->modifyUp == 1)/*enable modify*/
        {
            descrPtr->modifyUp = GT_TRUE;
        }
        else if(actionDataPtr->modifyUp == 2)/*disable modify*/
        {
            descrPtr->modifyUp = GT_FALSE;
        }
    }

    if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        descrPtr->trustQosMappingTableIndex = actionDataEArchPtr->trustQosMappingTableIndex;
        descrPtr->up2QosProfileMappingMode = actionDataEArchPtr->up2QosProfileMappingMode;

        snetXcatTTActionApplyEarchTti(devObjPtr, descrPtr, actionDataPtr, internalTtiInfoPtr);

        trustQosMappingTableIndex = descrPtr->trustQosMappingTableIndex;
        __LOG(("trustQosMappingTableIndex[%d]",trustQosMappingTableIndex));

        if(SMEM_CHT_IS_SIP7_GET(devObjPtr) && trustQosMappingTableIndex > 71)
        {
             /* only 72 profiles in the device */
            skernelFatalError("snetXcatTTActionApply: invalid trustQosMappingTableIndex [%d] > 71 \n");
        }
        else if(trustQosMappingTableIndex > 11)
        {
            /* only 12 profiles in the device */
            skernelFatalError("snetXcatTTActionApply: invalid trustQosMappingTableIndex [%d] > 11 \n");
        }
    }
    else
    {
        trustQosMappingTableIndex = 0;
    }

    if (actionDataPtr->keepPreviousQoS == GT_FALSE)
    {
        snetXcatTTActionApplyMplsQos(devObjPtr, descrPtr, actionDataPtr, trustQosMappingTableIndex);
    }
    else
    {
        __LOG(("TTI action will not modify QoSProfile because 'keep Previous QoS' \n"));
    }

    if(SMEM_CHT_IS_SIP5_GET(devObjPtr) &&
       isMplsLookup                    &&
       actionDataEArchPtr->ttlExpiryVccvEnable)
    {
        __LOG_PARAM(actionDataEArchPtr->ttlExpiryVccvEnable);

        snetLion3TTActionApplyTtlExpiryVccvEnable(devObjPtr, descrPtr,
                                    actionDataEArchPtr->pwe3FlowLabelExist);
    }

    snetXcatTTActionApplyMplsCwBasedPw(devObjPtr, descrPtr, actionDataPtr, isMplsLookup, internalTtiInfoPtr);

    if(descrPtr->tunnelTerminated)
    {
        /* parsing of passenger due to Tunnel termination */
        snetXcatTTActionApplyTtPassengerParsing(devObjPtr, descrPtr, actionDataPtr, internalTtiInfoPtr, GT_FALSE);
    }

    snetXcatTTActionCheckAndApplyMplsPwTagMode(devObjPtr, descrPtr, actionDataPtr, &allowVid1Assignment);

    /* save value for the re-parse of 'NUM_OF_TAGS_TO_POP' */
    internalTtiInfoPtr->vlan_actionInfo.vid0Cmd = actionDataPtr->vid0Cmd ;
    internalTtiInfoPtr->vlan_actionInfo.vid0    = actionDataPtr->vid0    ;
    internalTtiInfoPtr->vlan_actionInfo.vid1Cmd = actionDataPtr->vid1Cmd ;
    internalTtiInfoPtr->vlan_actionInfo.vid1    = actionDataPtr->vid1    ;
    internalTtiInfoPtr->vlan_actionInfo.up0     = actionDataPtr->up0     ;
    internalTtiInfoPtr->vlan_actionInfo.up1Cmd  = actionDataPtr->up1Cmd  ;
    internalTtiInfoPtr->vlan_actionInfo.up1     = actionDataPtr->up1     ;
    internalTtiInfoPtr->vlan_actionInfo.vidModified = 0;/* not relevant to 'action'*/
    internalTtiInfoPtr->vlan_actionInfo.allowVid1Assignment = allowVid1Assignment;
    /* save also 'orig' */
    internalTtiInfoPtr->vlan_origInfo.vid0Cmd = 0;/* not relevant to 'orig'*/
    internalTtiInfoPtr->vlan_origInfo.vid0    = descrPtr->eVid    ;
    internalTtiInfoPtr->vlan_origInfo.vid1Cmd = 0;/* not relevant to 'orig'*/
    internalTtiInfoPtr->vlan_origInfo.vid1    = descrPtr->vid1    ;
    internalTtiInfoPtr->vlan_origInfo.up0     = descrPtr->up     ;
    internalTtiInfoPtr->vlan_origInfo.up1Cmd  = 0;/* not relevant to 'orig'*/
    internalTtiInfoPtr->vlan_origInfo.up1     = descrPtr->up1     ;
    internalTtiInfoPtr->vlan_origInfo.vidModified = descrPtr->vidModified;
    internalTtiInfoPtr->vlan_origInfo.allowVid1Assignment = 0;/* not relevant to 'orig'*/

    snetXcatTTActionApplyVlanTagCmd(devObjPtr, descrPtr, internalTtiInfoPtr);

    if(SKERNEL_IS_LION_REVISON_B0_DEV(devObjPtr))
    {
        snetXcatTTActionApplyRedirectCmdLionB0(devObjPtr, descrPtr, actionDataPtr);
    }

    if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        if(actionDataEArchPtr->setMAC2ME)
        {
            descrPtr->mac2me = 1;
        }

        descrPtr->rxIsProtectionPath = actionDataEArchPtr->rxIsProtectionPath;
        descrPtr->rxEnableProtectionSwitching = actionDataEArchPtr->rxEnableProtectionSwitching;
        descrPtr->ipclUdbConfigurationTableUdeIndex = actionDataEArchPtr->ipclUdbConfigurationTableUdeIndex;

        descrPtr->channelTypeToOpcodeMappingEn = actionDataPtr->oamInfo.channelTypeToOpcodeMappingEn;

        if(actionDataEArchPtr->isPtpPacket)
        {
            descrPtr->isPtp = actionDataEArchPtr->isPtpPacket;
            descrPtr->ptpTriggerType = actionDataEArchPtr->ptpTriggerType;
            descrPtr->ptpOffset = actionDataEArchPtr->ptpOffset;

            descrPtr->ptpGtsInfo.ptpPacketTriggered = 1;
        }
    }
    if(SMEM_CHT_IS_SIP6_GET(devObjPtr))
    {
        if(actionDataEArchPtr->skipFdbSaLookup != 0)
        {/*only sip6 have this feild*/
            descrPtr->skipFdbSaLookup  = actionDataEArchPtr->skipFdbSaLookup;
        }
        /* Apply IPv6 Segment routing End Node DIP Update */
        if((actionDataEArchPtr->setIpv6SegmentRoutingEndNode != 0) &&
            (descrPtr->srEhExists == 1) && (descrPtr->isIpV6EhExists != 0))
        {
            /* Copy the SR header end segment to DIP */
            startByte = 8 + ((descrPtr->srSegmentsLeft - 1)*16);
            for(i=0; i<4; i++, startByte+=4)
            {
                descrPtr->dip[i] = SNET_BUILD_WORD_FROM_BYTES_PTR_MAC(&descrPtr->srHeaderStartOffsetPtr[startByte]);
            }
        }
    }

    if(SMEM_CHT_IS_SIP6_10_GET(devObjPtr))
    {
        if(actionDataEArchPtr->ppuProfileIdx)
        {
            descrPtr->ppuProfileIdx = actionDataEArchPtr->ppuProfileIdx;
        }
        descrPtr->ipfixEnable   = actionDataEArchPtr->ipfixEnable;

        /* Set desc<flow_track_En> field */
        if(actionDataEArchPtr->triggerCncHashClient)
        {
            descrPtr->flowTrackEn = 1;
        }
    }
}

/**
* @internal snetXcatTTActionApplyPart2 function
* @endinternal
*
* @brief   Apply the action entry from the TTI action table - Part 2 after
*         parse of passenger
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] descrPtr                 - pointer to frame descriptor.
*                                      actionInfoPtr   - pointer to the tti action entry.
*/
static GT_VOID snetXcatTTActionApplyPart2
(
    IN SKERNEL_DEVICE_OBJECT                * devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC   * descrPtr  ,
    IN SNET_XCAT_TT_ACTION_STC              * actionDataPtr
)
{
    DECLARE_FUNC_NAME(snetXcatTTActionApplyPart2);

    GT_U32  regAddr;                /* Register's address */
    GT_U32  fieldVal;               /* Register's field value */
    GT_U32  fldFirstBit;            /* Register field's first bit */
    GT_U32 mappingParam;            /* QoS mapping parameter : DSCP/EXP/UP/UP1 ...*/
    GT_U32 upProfileIndex;          /* up profile index */
    GT_BIT isIp,tagSrcTagged[2],l2Valid;/* flags from the descriptor*/
    GT_U32 dscp,vlanEtherType1,up1,cfidei1,up,cfidei;/* fields from the descriptor*/
    GT_U32  *memPtr;/*pointer to memory*/
    GT_U32  trustQosMappingTableIndex;/* QoS mapping table index */
    GT_U32  qosPrifileNumBits;/* number of bits that the qos profile uses in next tables */

    SNET_E_ARCH_TT_ACTION_STC   *actionDataEArchPtr;/*E-Arch action info*/
    actionDataEArchPtr = (SNET_E_ARCH_TT_ACTION_STC*)(void*)actionDataPtr;


     /* QoS Marking Command */
    /* The TTI Action QoS marker operation ignores the QoS Precedence assigned
       by port/protocol markers */

    if (actionDataPtr->keepPreviousQoS == GT_TRUE)
    {
        __LOG(("keepPreviousQoS == GT_TRUE , so no extra action QOS updated \n"));
        return;
    }

    __LOG(("apply the matching action part 2 - QOS related \n"));
    /* get the 'frame parser fields' form the descriptor/'TRILL info' :
       for TRILL :
        based on the inner Ethernet packet*/
    SKERNEL_CHT_DESC_INNER_FRAME_FIELD_GET_MAC(descrPtr,isIp);
    SKERNEL_CHT_DESC_INNER_FRAME_FIELD_GET_MAC(descrPtr,tagSrcTagged[SNET_CHT_TAG_0_INDEX_CNS]);
    SKERNEL_CHT_DESC_INNER_FRAME_FIELD_GET_MAC(descrPtr,vlanEtherType1);
    SKERNEL_CHT_DESC_INNER_FRAME_FIELD_GET_MAC(descrPtr,l2Valid);



    /* TTI Action <Passenger Parsing of Non-MPLS Transit tunnel Enable> - when enabled, and
       TTI-AE<Tunnel Terminate> is disabled,
        QoS profile assignment is based on TTI AE QoS mode and passenger packet QoS fields */
    if(SMEM_CHT_IS_SIP5_GET(devObjPtr)                                  &&
       actionDataEArchPtr->passengerParsingOfNonMplsTransitTunnelEnable &&
       GT_FALSE == actionDataPtr->tunnelTerminateEn)
    {
        __LOG(("QoS profile assignment is based on TTI AE QoS mode and passenger packet QoS field"));

        SKERNEL_CHT_DESC_INNER_FRAME_FIELD_GET_MAC(descrPtr,dscp);
        SKERNEL_CHT_DESC_INNER_FRAME_FIELD_GET_MAC(descrPtr,up1);
        SKERNEL_CHT_DESC_INNER_FRAME_FIELD_GET_MAC(descrPtr,cfidei1);
        SKERNEL_CHT_DESC_INNER_FRAME_FIELD_GET_MAC(descrPtr,up);
        SKERNEL_CHT_DESC_INNER_FRAME_FIELD_GET_MAC(descrPtr,cfidei);
    }
    else
    {
        /* get the 'Qos fields' form the descriptor/'TRILL info' :
           for TRILL :
           QoS Trust Mode
           - Trust L2 and/or L3 QoS is based on the inner Ethernet packet*/
        SKERNEL_CHT_DESC_INNER_FRAME_QOS_FIELD_GET_MAC(descrPtr,dscp);
        SKERNEL_CHT_DESC_INNER_FRAME_QOS_FIELD_GET_MAC(descrPtr,up1);
        SKERNEL_CHT_DESC_INNER_FRAME_QOS_FIELD_GET_MAC(descrPtr,cfidei1);
        SKERNEL_CHT_DESC_INNER_FRAME_QOS_FIELD_GET_MAC(descrPtr,up);
        SKERNEL_CHT_DESC_INNER_FRAME_QOS_FIELD_GET_MAC(descrPtr,cfidei);
    }


    if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        trustQosMappingTableIndex = descrPtr->trustQosMappingTableIndex;
        __LOG(("trustQosMappingTableIndex[%d]",trustQosMappingTableIndex));

        qosPrifileNumBits = 10;
    }
    else
    {
        trustQosMappingTableIndex = 0;
        qosPrifileNumBits = 7;
    }

    if(SMEM_CHT_IS_SIP7_GET(devObjPtr) && trustQosMappingTableIndex > 71)
    {
        /* only 72 profiles in the device */
        skernelFatalError("snetXcatTTActionApplyPart2: invalid trustQosMappingTableIndex [%d] > 71 \n");
    }
    else if(SMEM_CHT_IS_SIP5_GET(devObjPtr) && trustQosMappingTableIndex > 11)
    {
        /* only 12 profiles in the device */
        skernelFatalError("snetXcatTTActionApplyPart2: invalid trustQosMappingTableIndex [%d] > 11 \n");
    }

    if (actionDataPtr->trustDscp && isIp)
    {
        if (actionDataPtr->dscpToDscpRemapingEn)
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
                /* DSCP2DSCP */
                __LOG(("DSCP2DSCP"));
            }

            dscp = fieldVal;
        }
        else
        {   /* packet DSCP */
            __LOG(("packet DSCP"));
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
    else if (actionDataPtr->trustUp && l2Valid && tagSrcTagged[SNET_CHT_TAG_0_INDEX_CNS])
    {
        if (descrPtr->trustTag1Qos && vlanEtherType1)
        {
            up = up1;
            cfidei = cfidei1;
        }
        else
        {
            up = up;
            cfidei = cfidei;
        }

        if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
        {
            if(descrPtr->up2QosProfileMappingMode)
            {
                trustQosMappingTableIndex = descrPtr->trustTag1Qos ? up1 : up;
                __LOG(("trustQosMappingTableIndex[%d] from [%s]",
                            trustQosMappingTableIndex,descrPtr->trustTag1Qos ? "up1" : "up0"));
            }

            if(SMEM_CHT_IS_SIP7_GET(devObjPtr) && trustQosMappingTableIndex > 71)
            {
                /* only 72 profiles in the device */
                skernelFatalError("snetXcatTTActionApply: invalid trustQosMappingTableIndex [%d] > 71 \n");
            }
            else if(trustQosMappingTableIndex > 11)
            {
                /* only 12 profiles in the device */
                skernelFatalError("snetXcatTTActionApply: invalid trustQosMappingTableIndex [%d] > 11 \n");
            }

            memPtr = smemMemGet(devObjPtr,SMEM_CHT_UP_2_QOS_PROF_MAP_TBL_MEM(devObjPtr,0,0,trustQosMappingTableIndex));

            descrPtr->qos.qosProfile = snetFieldValueGet(memPtr,qosPrifileNumBits * ((cfidei*8) + up),qosPrifileNumBits);

            goto qosProfilDone_lbl;
        }
        else
        if(SKERNEL_IS_LION_REVISON_B0_DEV(devObjPtr))
        {
            regAddr = SMEM_LION_TTI_CFI_UP_TO_QOS_PROFILE_MAPPING_TABLE_SELECTOR_REG(devObjPtr,
                descrPtr->localDevSrcPort);
            fldFirstBit = up + (descrPtr->localDevSrcPort % 4) * 8;
            smemRegFldGet(devObjPtr, regAddr, fldFirstBit, 1, &upProfileIndex);
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
        return;
    }

    /* 4 QoS profiles in register */
    fldFirstBit = (mappingParam % 4) * 8;
    smemRegFldGet(devObjPtr, regAddr, fldFirstBit, 7,  &fieldVal);
    descrPtr->qos.qosProfile = fieldVal;

qosProfilDone_lbl:
    __LOG(("new descrPtr->qos.qosProfile[%d]",descrPtr->qos.qosProfile));


    return;
}

/**
* @internal snetXcatTTActionGet function
* @endinternal
*
* @brief   Get the action entry from the TTI action table.
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to frame descriptor.
* @param[in] matchIndex               - index to the action table .
*                                      ttiKeyType      - tti key type
*/
static GT_VOID snetXcatTTActionGet
(
    IN SKERNEL_DEVICE_OBJECT            * devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC  * descrPtr,
    IN GT_U32                             matchIndex,
    OUT SNET_XCAT_TT_ACTION_STC         * actionDataPtr,
    OUT   INTERNAL_TTI_DESC_INFO_STC        *internalTtiInfoPtr
)
{
    DECLARE_FUNC_NAME(snetXcatTTActionGet);

    GT_BOOL enable;
    GT_U32  *actionEntryDataPtr;
    GT_U32  indexBaseAddress;
    SNET_LION_TT_ACTION_STC * lionActionDataPtr;
    SNET_E_ARCH_TT_ACTION_STC   * actionDataEArchPtr;/*E-Arch action info*/

    actionDataEArchPtr = (SNET_E_ARCH_TT_ACTION_STC*)(void*)actionDataPtr;
    lionActionDataPtr = &actionDataEArchPtr->baseActionData;

    indexBaseAddress = SMEM_CHT3_TUNNEL_ACTION_TBL_MEM(devObjPtr, matchIndex);

    actionEntryDataPtr = smemMemGet(devObjPtr,indexBaseAddress);

    actionDataPtr->packetCmd =
        snetFieldValueGet(actionEntryDataPtr, 0, 3);
    actionDataPtr->userDefinedCpuCode =
        snetFieldValueGet(actionEntryDataPtr, 3, 8);
    actionDataPtr->mirrorToAnalyzerPort =
        snetFieldValueGet(actionEntryDataPtr, 11, 1);
    actionDataPtr->redirectCmd =
        snetFieldValueGet(actionEntryDataPtr, 12, 3);

    actionDataPtr->ipclProfileIndex =
        snetFieldValueGet(actionEntryDataPtr, 31, 13);

    switch (actionDataPtr->redirectCmd)
    {
    case PCL_TTI_ACTION_REDIRECT_CMD_OUT_IF_E:
        actionDataPtr->ipclProfileIndex = 0;/* not relevant to this mode */
        enable = snetFieldValueGet(actionEntryDataPtr, 27, 1);
        SNET_SET_DEST_INTERFACE_TYPE(actionDataPtr->tunnelIf,
                                     SNET_DST_INTERFACE_VIDX_E, enable);

        if (SNET_GET_DEST_INTERFACE_TYPE(actionDataPtr->tunnelIf,
                                     SNET_DST_INTERFACE_VIDX_E))
        {
            actionDataPtr->tunnelIf.interfaceInfo.vidx =
                (GT_U16)snetFieldValueGet(actionEntryDataPtr, 15, 12);
        }
        else
        {
            enable = (GT_U8)snetFieldValueGet(actionEntryDataPtr, 15, 1);
            SNET_SET_DEST_INTERFACE_TYPE(actionDataPtr->tunnelIf,
                                     SNET_DST_INTERFACE_TRUNK_E, enable);

            if (SNET_GET_DEST_INTERFACE_TYPE(actionDataPtr->tunnelIf,
                                         SNET_DST_INTERFACE_TRUNK_E))
            {
                actionDataPtr->tunnelIf.interfaceInfo.trunkId =
                    (GT_U8)snetFieldValueGet(actionEntryDataPtr, 16, 8);
                /* bits [24:26] are reserved */
            }
            else
            {
                actionDataPtr->tunnelIf.interfaceInfo.devPort.port =
                    (GT_U8)snetFieldValueGet(actionEntryDataPtr, 16, 6);
                actionDataPtr->tunnelIf.interfaceInfo.devPort.devNum =
                    (GT_U8)snetFieldValueGet(actionEntryDataPtr, 22, 5);
            }
        }

        actionDataPtr->tunnelStart =
            snetFieldValueGet(actionEntryDataPtr, 28, 1);
        if (actionDataPtr->tunnelStart)
        {
            actionDataPtr->tunnelStartPtr =
                snetFieldValueGet(actionEntryDataPtr, 29, 13);
            actionDataPtr->tunnelStartPassengerType =
                snetFieldValueGet(actionEntryDataPtr, 42, 1);
        }
        else
        {
            actionDataPtr->arpPtr =
                snetFieldValueGet(actionEntryDataPtr, 29, 14);
        }

        actionDataPtr->vntL2Echo =
            snetFieldValueGet(actionEntryDataPtr, 43, 1);
        break;

    case PCL_TTI_ACTION_REDIRECT_CMD_LTT_ROUTER_E:
        actionDataPtr->routerLTTIndex =
            snetFieldValueGet(actionEntryDataPtr, 15, 15);
        /* bit [30] is reserved */
        __LOG(("bit [30] is reserved"));
        break;

    case PCL_TTI_ACTION_REDIRECT_CMD_VIRT_ROUTER_E:
        actionDataPtr->vrfId = snetFieldValueGet(actionEntryDataPtr, 15, 12);
         /* bits [27:30] are reserved */
        __LOG(("bits [27:30] are reserved"));
        break;
    case PCL_TTI_ACTION_REDIRECT_CMD_ASSIGN_SOURCE_LOGICAL_PORT_E:/* Assign Logic Port */
         /* 15        :        15        1        Reserved
            21        :        16        6        VirtualSrcPort
            26        :        22        5        VirtualSrcDev
            29        :        27        3        Reserved
            30        :        30        1        Reserved
            43        :        31        13        IPCL Profile Index */
        actionDataPtr->srcLogicPortInfo.srcIsTrunk = 0;
        actionDataPtr->srcLogicPortInfo.srcTrunkOrPortNum = snetFieldValueGet(actionEntryDataPtr,16,6);
        actionDataPtr->srcLogicPortInfo.srcDevice = snetFieldValueGet(actionEntryDataPtr,22,5);
        break;

    default:
        break;
    }

    actionDataPtr->bindToCncCounter =
        snetFieldValueGet(actionEntryDataPtr, 44, 1);
    actionDataPtr->cncCounterIndex =
        snetFieldValueGet(actionEntryDataPtr, 45, 14);
    actionDataPtr->bindToPolicerMeter =
        snetFieldValueGet(actionEntryDataPtr, 59, 1);
    actionDataPtr->bindToPolicerCounter =
        snetFieldValueGet(actionEntryDataPtr, 60, 1);
    actionDataPtr->policerIndex =
        snetFieldValueGet(actionEntryDataPtr, 61, 12);
    actionDataPtr->srcIdSetEn =
        snetFieldValueGet(actionEntryDataPtr, 73, 1);
    actionDataPtr->srcId =
        snetFieldValueGet(actionEntryDataPtr, 74, 5);
    actionDataPtr->actionStop =
        snetFieldValueGet(actionEntryDataPtr, 79, 1);
    actionDataPtr->bypassBridge =
        snetFieldValueGet(actionEntryDataPtr, 80, 1);
    actionDataPtr->bypassIngressPipe =
        snetFieldValueGet(actionEntryDataPtr, 81, 1);
    actionDataPtr->policy2LookupMode =
        snetFieldValueGet(actionEntryDataPtr, 82, 1);
    actionDataPtr->policy1LookupMode =
        snetFieldValueGet(actionEntryDataPtr, 83, 1);
    actionDataPtr->policy0LookupMode =
        snetFieldValueGet(actionEntryDataPtr, 84, 1);
    actionDataPtr->vidPrecedence =
        snetFieldValueGet(actionEntryDataPtr, 85, 1);
    actionDataPtr->nestedVidEn =
        snetFieldValueGet(actionEntryDataPtr, 86, 1);
    actionDataPtr->vid0Cmd =
        snetFieldValueGet(actionEntryDataPtr, 87, 2);
    actionDataPtr->vid0 =
        snetFieldValueGet(actionEntryDataPtr, 89, 12);
    actionDataPtr->mplsCmd =
        snetFieldValueGet(actionEntryDataPtr, 143, 3);

    if (descrPtr->isTrillEtherType &&
        (internalTtiInfoPtr->ttiKeyType >= SKERNEL_LION3_TTI_KEY_UDB_PKT_TYPE_0_E &&
         internalTtiInfoPtr->ttiKeyType <= SKERNEL_LION3_TTI_KEY_UDB_PKT_TYPE_15_E))
    {
        actionDataPtr->ttlDecEn =
            snetFieldValueGet(actionEntryDataPtr, 109, 1);
    }
    /* TTI Action <MPLS Command>= NOP */
    else
    {
        if (actionDataPtr->mplsCmd == SKERNEL_XCAT_TTI_MPLS_NOP_E)
        {
            actionDataPtr->vid1Cmd =
                snetFieldValueGet(actionEntryDataPtr, 101, 1);
            actionDataPtr->vid1 =
                snetFieldValueGet(actionEntryDataPtr, 102, 12);
        }
        else
        {
            actionDataPtr->ttl =
                snetFieldValueGet(actionEntryDataPtr, 101, 8);
            actionDataPtr->ttlDecEn =
                snetFieldValueGet(actionEntryDataPtr, 109, 1);
            /* bits [14:17] are reserved */
            __LOG(("bits [14:17] are reserved"));
        }
    }

    actionDataPtr->qoSPrecedence =
        snetFieldValueGet(actionEntryDataPtr, 114, 1);
    actionDataPtr->qoSProfile =
        snetFieldValueGet(actionEntryDataPtr, 115, 7);
    actionDataPtr->modifyDscp =
        snetFieldValueGet(actionEntryDataPtr, 122, 2);
    actionDataPtr->modifyUp =
        snetFieldValueGet(actionEntryDataPtr, 124,2);
    actionDataPtr->keepPreviousQoS =
        snetFieldValueGet(actionEntryDataPtr, 126, 1);
    actionDataPtr->trustUp =
        snetFieldValueGet(actionEntryDataPtr, 127, 1);
    actionDataPtr->trustDscp =
        snetFieldValueGet(actionEntryDataPtr, 128, 1);
    actionDataPtr->trustExp =
        snetFieldValueGet(actionEntryDataPtr, 129, 1);
    actionDataPtr->dscpToDscpRemapingEn =
        snetFieldValueGet(actionEntryDataPtr, 130, 1);
    actionDataPtr->up0 =
        snetFieldValueGet(actionEntryDataPtr, 131, 3);
    actionDataPtr->up1Cmd =
        snetFieldValueGet(actionEntryDataPtr, 134, 2);
    actionDataPtr->up1 =
        snetFieldValueGet(actionEntryDataPtr, 136, 3);
    actionDataPtr->passengerPacketType =
        snetFieldValueGet(actionEntryDataPtr, 139, 2);
    actionDataPtr->copyTtlFromOuterHeader =
        snetFieldValueGet(actionEntryDataPtr, 141, 1);
    actionDataPtr->tunnelTerminateEn =
        snetFieldValueGet(actionEntryDataPtr, 142, 1);

    if(SKERNEL_IS_LION_REVISON_B0_DEV(devObjPtr))
    {
        lionActionDataPtr->multiPortGroupTTIEn =
            snetFieldValueGet(actionEntryDataPtr, 15, 1);
        lionActionDataPtr->hashMaskIndex =
            snetFieldValueGet(actionEntryDataPtr, 146, 4);
        lionActionDataPtr->modifyMacDa =
        lionActionDataPtr->resetSrcPortGroupId =
            snetFieldValueGet(actionEntryDataPtr, 150, 1);
        lionActionDataPtr->modifyMacSa =
            snetFieldValueGet(actionEntryDataPtr, 151, 1);
    }


    /* use the new action fields only when <VPLS Mode>==1 */
    __LOG(("use the new action fields only when <VPLS Mode>==1"));
    if(devObjPtr->vplsModeEnable.tti)
    {
        actionDataPtr->vplsInfo.unknownSaCmd =
                snetFieldValueGet(actionEntryDataPtr, 58, 1) |
                (snetFieldValueGet(actionEntryDataPtr, 146, 2) << 1);
        /* flag internal for the simulation to remove the use of value '7' from the unknownSaCmd */
        __LOG(("flag internal for the simulation to remove the use of value '7' from the unknownSaCmd"));
        if(actionDataPtr->vplsInfo.unknownSaCmd > 5)
        {
            actionDataPtr->vplsInfo.unknownSaCmdAssigned = 0;
        }
        else
        {
            actionDataPtr->vplsInfo.unknownSaCmdAssigned = 1;
        }

        actionDataPtr->vplsInfo.pwHasCw =
                snetFieldValueGet(actionEntryDataPtr, 57, 1);

        /* PW has CW and Unknown SA Cmd[0] bits come instead of <CNC Counter Index> bits 12,13 */
        SMEM_U32_SET_FIELD(actionDataPtr->cncCounterIndex,12,2,0);


        /* the <IPCL Profile Index> bits 11,12 are not used regardless to redirect command type */
        SMEM_U32_SET_FIELD(actionDataPtr->ipclProfileIndex,11,2,0);

        if(actionDataPtr->redirectCmd == PCL_TTI_ACTION_REDIRECT_CMD_ASSIGN_SOURCE_LOGICAL_PORT_E)
        {
            actionDataPtr->vplsInfo.egressTagState =
                snetFieldValueGet(actionEntryDataPtr, 15, 1) |
                (snetFieldValueGet(actionEntryDataPtr, 42, 2) << 1);

            /* flag internal for the simulation to remove the use of value '7' from the egressTagState */
            __LOG(("flag internal for the simulation to remove the use of value '7' from the egressTagState"));
            if(actionDataPtr->vplsInfo.egressTagState == 7)
            {
                actionDataPtr->vplsInfo.egressTagStateAssigned = 0;
            }
            else
            {
                actionDataPtr->vplsInfo.egressTagStateAssigned = 1;
            }

            actionDataPtr->vplsInfo.pwTagMode =
                snetFieldValueGet(actionEntryDataPtr, 27, 2);

            actionDataPtr->vplsInfo.srcMeshId =
                snetFieldValueGet(actionEntryDataPtr, 29, 2);


        }

    }


}

/**
* @internal snetXcatTTMplsTtlExceededException function
* @endinternal
*
* @brief   mpls lsr ttl exception handling
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to frame data buffer Id
*/
static GT_BOOL snetXcatTTMplsTtlExceededException
(
    IN SKERNEL_DEVICE_OBJECT             *  devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC   *  descrPtr,
    IN SNET_XCAT_TT_ACTION_STC           * actionDataPtr
)
{
    DECLARE_FUNC_NAME(snetXcatTTMplsTtlExceededException);

    GT_U32 fldVal;              /* register field's value */
    GT_U32 ttl;     /* ttl for the exception check */

    if(descrPtr->mplsUseSpecificTtlForTtlException)
    {
        /*do not check the value in the descriptor as it hold value that may not be taken from the existing Labels */
        /* special code to match VERIFIER behavior */
        ttl = descrPtr->mplsLabelTtlForException;
    }
    else
    {
        ttl = descrPtr->ttl;
    }

    __LOG(("the TTL [%d] for exception check \n",
            ttl));


    if ((ttl == 0 || (ttl == 1 && descrPtr->decTtl == GT_TRUE)) && actionDataPtr->mplsCmd != SKERNEL_XCAT_TTI_MPLS_NOP_E)
    {
        /* TTI Illegal MPLS TTL Error Command */
        __LOG(("TTI Illegal MPLS TTL Error Command"));

        if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
        {
            smemRegFldGet(devObjPtr,SMEM_LION3_TTI_LOOKUP_MPLS_EXCEPTION_COMMANDS_REG(devObjPtr) , 0, 3, &fldVal);

            snetChtIngressCommandAndCpuCodeResolution(devObjPtr,descrPtr,
                                                      descrPtr->packetCmd,
                                                      fldVal,
                                                      descrPtr->cpuCode,
                                                      SNET_XCAT_MPLS_TT_LSR_TTL_EXCEEDED,
                                                      SNET_CHEETAH_ENGINE_UNIT_TTI_E,
                                                      GT_FALSE);

            return (descrPtr->packetCmd > SKERNEL_EXT_PKT_CMD_MIRROR_TO_CPU_E) ?
                    GT_FALSE :
                    GT_TRUE;
        }

        smemRegFldGet(devObjPtr, SMEM_XCAT_TTI_ENGINE_CONFIG_REG(devObjPtr),7, 1, &fldVal);

        if (fldVal == 1)
        {
            descrPtr->packetCmd = SKERNEL_CHT2_TT_CMD_HARD_DROP_E;
        }
        else
        {
            descrPtr->packetCmd = SKERNEL_CHT2_TT_CMD_TRAP_E;
            descrPtr->cpuCode = SNET_XCAT_MPLS_TT_LSR_TTL_EXCEEDED;
        }

        return GT_FALSE;
    }

    return GT_TRUE;
}

/**
* @internal snetXcatTTMplsHeaderCheck function
* @endinternal
*
* @brief   illegal MPLS command or Tunnel Terminate
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to frame data buffer Id
*/
static GT_BOOL snetXcatTTMplsHeaderCheck
(
    IN SKERNEL_DEVICE_OBJECT             *  devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC   *  descrPtr,
    IN SNET_XCAT_TT_ACTION_STC           *  actionDataPtr
)
{
    DECLARE_FUNC_NAME(snetXcatTTMplsHeaderCheck);

    GT_U32 fldVal;              /* register field's value */
    GT_STATUS status = GT_TRUE;
    /* Maximum MPLS Labels allowed
     * SIP7 : 10 labels
     * SIP5 : 6 Labels
     * Below SIP5: 4 labels */
    GT_U32 maxLabels = SMEM_CHT_IS_SIP5_GET(devObjPtr) ?
                       ((SMEM_CHT_IS_SIP7_GET(devObjPtr))? 10: 6):
                       4;

    GT_U32  numLabelsNeeded = 0;
    GT_BIT checkNumLabels = 0;

    if(actionDataPtr->tunnelTerminateEn==GT_FALSE)
    {
        /*this check done only for non terminated */
        checkNumLabels = 1;
        switch(actionDataPtr->mplsCmd)
        {
            case SKERNEL_XCAT_TTI_MPLS_POP_2_E:
            case SKERNEL_XCAT_TTI_MPLS_POP_SWAP_E:
                numLabelsNeeded = 2;
                break;
            case SKERNEL_XCAT_TTI_MPLS_POP2_SWAP_E:
            case SKERNEL_XCAT_TTI_MPLS_POP_3_E:
                numLabelsNeeded = 3;
                break;
            case SKERNEL_XCAT_TTI_MPLS_POP3_SWAP_E:
                numLabelsNeeded = 4;
                break;
            default:
                checkNumLabels = 0;
                break;
        }
    }

    __LOG_PARAM(maxLabels);
    /* The <Tunnel Terminate> field is enabled for an MPLS packet, and the packet has more than
       three MPLS labels */
    if (descrPtr->mpls && actionDataPtr->tunnelTerminateEn &&
        ((descrPtr->numOfLabels + 1) > maxLabels || descrPtr->l3NotValid))
    {
        __LOG(("illegal MPLS : The <Tunnel Terminate> field is enabled for an MPLS packet, and the packet has more than [%d] MPLS labels \n",
            maxLabels));
        status = GT_FALSE;
    }
    /* <MPLS Command> != NOP/Push and packet is not MPLS.*/
    else
    if (actionDataPtr->mplsCmd != SKERNEL_XCAT_TTI_MPLS_NOP_E && actionDataPtr->mplsCmd != SKERNEL_XCAT_TTI_MPLS_PUSH_E && descrPtr->mpls == GT_FALSE)
    {
        __LOG(("illegal MPLS : <MPLS Command> != NOP/Push and packet is not MPLS."));
        status = GT_FALSE;
    }
    else
    if (checkNumLabels && ((descrPtr->numOfLabels + 1) < numLabelsNeeded))
    {
        __LOG(("illegal MPLS : <MPLS Command> requires to Pop [%d] labels ,packet got only [%d] labels  \n",
                       numLabelsNeeded,
                       descrPtr->numOfLabels + 1));
        status = GT_FALSE;
    }

    if (status)
    {
        return GT_TRUE;
    }

    if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
    {
        smemRegFldGet(devObjPtr,SMEM_LION3_TTI_LOOKUP_MPLS_EXCEPTION_COMMANDS_REG(devObjPtr) , 3, 3, &fldVal);

        snetChtIngressCommandAndCpuCodeResolution(devObjPtr,descrPtr,
                                                  descrPtr->packetCmd,
                                                  fldVal,
                                                  descrPtr->cpuCode,
                                                  SNET_XCAT_MPLS_TT_HEADER_CHECK_ERROR,
                                                  SNET_CHEETAH_ENGINE_UNIT_TTI_E,
                                                  GT_FALSE);

        __LOG(("After CPU Code Resolution: packetCmd = [%u], cpuCode = [%u]\n",
               descrPtr->packetCmd, descrPtr->cpuCode));

        return (descrPtr->packetCmd > SKERNEL_EXT_PKT_CMD_MIRROR_TO_CPU_E) ?
                GT_FALSE :
                GT_TRUE;
    }

    /* TTI Unsupported MPLS Error Command */
    smemRegFldGet(devObjPtr, SMEM_XCAT_TTI_ENGINE_CONFIG_REG(devObjPtr),8, 1, &fldVal);

    if (fldVal == 1)
    {
        descrPtr->packetCmd = SKERNEL_CHT2_TT_CMD_HARD_DROP_E;
    }
    else
    {
        descrPtr->packetCmd = SKERNEL_CHT2_TT_CMD_TRAP_E;
        descrPtr->cpuCode = SNET_XCAT_MPLS_TT_HEADER_CHECK_ERROR;
    }

    __LOG(("new descrPtr->packetCmd = [%d]",descrPtr->packetCmd));


    return GT_FALSE;
}

/**
* @internal snetXcatTTIpKeyProtocolGet function
* @endinternal
*
* @brief   get ip TT key protocol
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in,out] descrPtr                 - pointer to frame data buffer Id
* @param[in,out] descrPtr                 - pointer to frame data buffer Id
*                                      RETURN:
*                                      GT_BOOL      - is the Ip can be considered TT only
*                                      COMMENTS:
*/
static GT_BOOL snetXcatTTIpKeyProtocolGet
(
    IN    SKERNEL_DEVICE_OBJECT             *devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC   *descrPtr
)
{
    DECLARE_FUNC_NAME(snetXcatTTIpKeyProtocolGet);

    GT_U32  greEtherType;                 /* ethertype inside the IP tunnel --> checked to be GRE */
    GT_U32  gre0EtherType, gre1EtherType; /* GRE protocols that are recognized as
                                             Ethernet-over-GRE tunneling by TTI Engine */

    if (GT_FALSE == snetXcatGetGreEtherTypes(devObjPtr, descrPtr,
                        &greEtherType, &gre0EtherType, &gre1EtherType))
    {
        if(!descrPtr->isIPv4 && !SMEM_CHT_IS_SIP5_GET(devObjPtr))
        {
            __LOG(("ipv6 and not sip5  --> not TT only"));
            return GT_FALSE;
        }
    }

    /* the next considered TT only */
    if (descrPtr->ipProt == 4)
    {
        descrPtr->ipTTKeyProtocol = 0;
    }
    else if (descrPtr->ipProt == 41)
    {
        descrPtr->ipTTKeyProtocol = 1;
    }
    else if (descrPtr->ipProt == 47) /*GRE*/
    {
        if (greEtherType == SKERNEL_L3_PROT_TYPE_IPV4_E)
        {
            descrPtr->ipTTKeyProtocol = 2;
        }
        else if (SMEM_CHT_IS_SIP5_GET(devObjPtr) && greEtherType == SKERNEL_L3_PROT_TYPE_IPV6_E)
        {
            descrPtr->ipTTKeyProtocol = 3;
        }
        else if (greEtherType == gre0EtherType)
        {
            descrPtr->ipTTKeyProtocol = 4;
        }
        else if (greEtherType == gre1EtherType)
        {
            descrPtr->ipTTKeyProtocol = 5;
        }
        else if (SMEM_CHT_IS_SIP5_GET(devObjPtr))
        {
            descrPtr->ipTTKeyProtocol = 6;
        }
        else
        {
            descrPtr->ipTTKeyProtocol = 3;
        }
    }
    else
    {
        /*unknown tunnel*/
        descrPtr->ipTTKeyProtocol = 7;
        return GT_FALSE;
    }

    return GT_TRUE;
}

/**
* @internal snetXcatTTTriggeringCheck function
* @endinternal
*
* @brief   IPv4 tunnels and MPLS tunnels have separate T.T trigger
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to frame data buffer Id
* @param[in] internalTtiInfoPtr       - (pointer to) internal TTI info
*
* @param[out] internalTtiInfoPtr       - (pointer to) internal TTI info
*                                      RETURN:
*                                      GT_BOOL TRUE DOING T.T false
*                                      COMMENTS:
*/
static GT_BOOL snetXcatTTTriggeringCheck
(
    IN  SKERNEL_DEVICE_OBJECT            *devObjPtr,
    IN  SKERNEL_FRAME_CHEETAH_DESCR_STC  *descrPtr,
    OUT INTERNAL_TTI_DESC_INFO_STC       *internalTtiInfoPtr
)
{
    DECLARE_FUNC_NAME(snetXcatTTTriggeringCheck);

    GT_U32  regAddress;             /* register entry       */
    GT_U32  *portVlanCfgEntryPtr;   /* table entry pointer  */
    GT_U32  ipV4ttEn;               /* Enable IPv4 TT action for packet rx in port  */

    GT_U32  mplsEn;                 /* Enable mpls TT action for packet rx in port  */
    GT_U32  eterEn;                 /* Enable Ethernet TT action for packet rx in port  */
    GT_U32  mimEn;                  /* Enable mim TT action for packet rx in port  */
    GT_U32  enIPv4Mac2Me;           /* Enable triggering IPv4 TTI search for mac2me packets */

    GT_U32  enMPLSMac2Me;           /* Enable triggering MPLS TTI search for mac2me packets */
    GT_U32  enMimMac2Me;            /* Enable triggering MIM TTI search for mac2me packets */
    GT_U32  *ttiPreTtiLookupIngressEPortTablePtr = NULL;/* pointer to the entry in : tti - Pre-TTI Lookup Ingress ePort Table  */

    GT_BOOL isIpTTOnly = GT_FALSE;  /* is the Ip can be considered TT only */
    GT_U32  ipForceOnly_isIpTTOnly; /* the IP lookup can be force to check only those "considered TT only" */

    GT_U32  ipv4McEnable;           /* TTI IPv4 MC Enable */
    GT_U32  bitNum;                 /* register's bit number  */

    enum
    {
        IPV4_LOOKUP_E = 0,
        IPV4_TT_ONLY_E,
        IPV4_MAC2ME_E,
        MPLS_LOOKUP_E,
        MPLS_MAC2ME_E,
        ETH_LOOKUP_E,
        MIM_LOOKUP_E,
        MIM_MAC2ME_E,
        TRILL_LOOKUP_E
    };

    static const struct{
        GT_U32 word;
        GT_U32 bit;
    }
     wordBitXcat[] = {{1,19},       /* IPV4_LOOKUP_E  */
                      {1,20},       /* IPV4_TT_ONLY_E */
                      {1,22},       /* IPV4_MAC2ME_E  */
                      {1,23},       /* MPLS_LOOKUP_E  */
                      {1,24},       /* MPLS_MAC2ME_E  */
                      {1,25},       /* ETH_LOOKUP_E   */
                      {1,28},       /* MIM_LOOKUP_E   */
                      {1,29},       /* MIM_MAC2ME_E   */
                      },
     *wordBitPtr;

#define GET_BIT(bit_name) \
      (SMEM_U32_GET_FIELD(portVlanCfgEntryPtr[wordBitPtr[bit_name].word], wordBitPtr[bit_name].bit, 1))


    /* get ipv4 mc enable value */
    bitNum = SMEM_CHT_IS_SIP5_GET(devObjPtr)? 2 : 5;
    smemRegFldGet(devObjPtr, SMEM_XCAT_TTI_ENGINE_CONFIG_REG(devObjPtr), bitNum, 1, &ipv4McEnable);

    /* Set init value */
    internalTtiInfoPtr->ttiKeyType = SKERNEL_CHT3_TTI_KEY_TYPE_LAST_E;

    /* tti - Pre-TTI Lookup Ingress ePort Table */
    ttiPreTtiLookupIngressEPortTablePtr = descrPtr->eArchExtInfo.ttiPreTtiLookupIngressEPortTablePtr;

    wordBitPtr = wordBitXcat;
    if(ttiPreTtiLookupIngressEPortTablePtr)
    {
        portVlanCfgEntryPtr = NULL;
    }
    else
    {
        regAddress = SMEM_CHT_PORT_VLAN_QOS_CONFIG_TBL_MEM(devObjPtr,
                                                            descrPtr->localDevSrcPort);
        portVlanCfgEntryPtr = smemMemGet(devObjPtr, regAddress);
    }

    /* Packets with DSA Tag != Forward are not triggered for TTI lookup */
    if((descrPtr->marvellTagged) && (descrPtr->incomingMtagCmd != SKERNEL_MTAG_CMD_FORWARD_E))
    {
        __LOG(("trigger failed, because: marvellTagged and incomingMtagCmd != SKERNEL_MTAG_CMD_FORWARD_E"));
        return GT_FALSE;
    }

    /* Packet has not been assigned Hard Drop/Soft Drop/Trap command by previous stages */
    if(descrPtr->packetCmd > SKERNEL_EXT_PKT_CMD_MIRROR_TO_CPU_E)
    {
        __LOG(("trigger failed, because: descrPtr->packetCmd > SKERNEL_EXT_PKT_CMD_MIRROR_TO_CPU_E"));
        return GT_FALSE;
    }

    if(descrPtr->eArchExtInfo.ttiPreTtiLookupIngressEPortTablePtr)
    {
        /* MPLS lookup   */
        mplsEn =
            SMEM_LION3_TTI_DEFAULT_EPORT_ENTRY_FIELD_GET(devObjPtr, descrPtr,
                SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_MPLS_TUNNEL_TERMINATION_EN);
        /* Enable triggering MPLS TTI search for mac2me packets */
        enMPLSMac2Me =
            SMEM_LION3_TTI_DEFAULT_EPORT_ENTRY_FIELD_GET(devObjPtr, descrPtr,
                SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_TTI_MPLS_MAC_TO_ME_EN);
        /* check if IPV4 lookup   */
        ipV4ttEn =
            SMEM_LION3_TTI_DEFAULT_EPORT_ENTRY_FIELD_GET(devObjPtr, descrPtr,
                SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_IPV4_TUNNEL_TERMINATION_EN);
        /* IPv4 TT Enable bit */
        ipForceOnly_isIpTTOnly =
            SMEM_LION3_TTI_DEFAULT_EPORT_ENTRY_FIELD_GET(devObjPtr, descrPtr,
                SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_IPV4_TTI_FOR_TT_ONLY);
        /* Enable triggering IPv4 TTI search for mac2me packets */
        enIPv4Mac2Me =
            SMEM_LION3_TTI_DEFAULT_EPORT_ENTRY_FIELD_GET(devObjPtr, descrPtr,
                SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_TTI_IPV4_MAC_TO_ME_EN);
        /* MIM lookup  */
        mimEn =
            SMEM_LION3_TTI_DEFAULT_EPORT_ENTRY_FIELD_GET(devObjPtr, descrPtr,
                SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_EN_802_1AH_TTI_LOOKUP);
        /* Enable triggering MIM TTI search for mac2me packets */
        enMimMac2Me =
            SMEM_LION3_TTI_DEFAULT_EPORT_ENTRY_FIELD_GET(devObjPtr, descrPtr,
                SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_TTI_802_1AH_MAC_TO_ME_EN);
        /* ETH lookup  */
        eterEn =
            SMEM_LION3_TTI_DEFAULT_EPORT_ENTRY_FIELD_GET(devObjPtr, descrPtr,
                SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_ETHERNET_CLASSIFIER_EN);
    }
    else
    {
        /* MPLS lookup   */
        mplsEn = GET_BIT(MPLS_LOOKUP_E);
        /* Enable triggering MPLS TTI search for mac2me packets */
        enMPLSMac2Me = GET_BIT(MPLS_MAC2ME_E);
        /* check if IPV4 lookup   */
        ipV4ttEn = GET_BIT(IPV4_LOOKUP_E);
        /* IPv4 TT Enable bit */
        ipForceOnly_isIpTTOnly = GET_BIT(IPV4_TT_ONLY_E);
            /* Enable triggering IPv4 TTI search for mac2me packets */
            enIPv4Mac2Me = GET_BIT(IPV4_MAC2ME_E);
        /* MIM lookup  */
        mimEn = GET_BIT(MIM_LOOKUP_E);
        /* Enable triggering MIM TTI search for mac2me packets */
        enMimMac2Me = GET_BIT(MIM_MAC2ME_E);
        /* ETH lookup  */
        eterEn = GET_BIT(ETH_LOOKUP_E);
    }



    /* check MPLS triggering */
    if (descrPtr->mpls == 1)
    {
        internalTtiInfoPtr->ttiKeyType = SKERNEL_CHT3_TTI_KEY_MPLS_E;

        __LOG(("mpls : mplsEn =[%d] enMPLSMac2Me = [%d]",mplsEn,enMPLSMac2Me));

        if (mplsEn)
        {
            if(enMPLSMac2Me == 0)
            {
                return GT_TRUE;
            }
            else if(descrPtr->mac2me)
            {
                return GT_TRUE;
            }
        }
    }

    if(descrPtr->isIp) /* ipv4 / ipv6 */
    {
        isIpTTOnly = snetXcatTTIpKeyProtocolGet(devObjPtr, descrPtr);
    }



    /* check ipv4/arp triggering */
    if ((descrPtr->isIPv4) || descrPtr->arp)
    {
        internalTtiInfoPtr->ttiKeyType = SKERNEL_CHT3_TTI_KEY_IPV4_E;

        __LOG(("descrPtr->isIPv4 || descrPtr->arp : ipV4ttEn =[%d] ",ipV4ttEn));

        if(ipV4ttEn == 0)
        {
            /* ipv4 lookup is disabled */
            goto non_ipv4_arp_xcat_lbl;
        }

        if(descrPtr->ipm == 0)
        {
            /* unicast */

            __LOG(("descrPtr->ipm == 0 : TTonly =[%d] enIPv4Mac2Me[%d]",ipForceOnly_isIpTTOnly,enIPv4Mac2Me));

            if(ipForceOnly_isIpTTOnly && (!isIpTTOnly))
            {
                goto non_ipv4_arp_xcat_lbl;
            }

            if(enIPv4Mac2Me == 0)
            {
                return GT_TRUE;
            }
            else if(descrPtr->mac2me)
            {
                return GT_TRUE;
            }
            else
            {
                goto non_ipv4_arp_xcat_lbl;
            }

        }
        else
        {
            if(SMEM_CHT_IS_SIP5_GET(devObjPtr))
            {
                if(ipForceOnly_isIpTTOnly && (!isIpTTOnly))
                {
                    goto non_ipv4_arp_xcat_lbl;
                }
            }

            /* multicast */
            __LOG(("descrPtr->ipm != 0 : ipv4McEnable =[%d]",ipv4McEnable));

            /* Enables TTI search for IPv4 MC packets */
            if (ipv4McEnable)
            {
                return GT_TRUE;
            }
        }
    }

non_ipv4_arp_xcat_lbl:
    /* check MIM triggering */
    if (descrPtr->mim == 1)
    {
        internalTtiInfoPtr->ttiKeyType = SKERNEL_XCAT_TTI_KEY_MIM_E;

        /* Packet is classified as VLAN tag 0 exists*/
        if (descrPtr->ingressVlanTag0Type == SKERNEL_FRAME_TR101_VLAN_TAG_NOT_FOUND_E)
        {
            goto non_ipv4_arp_lbl;
        }

        __LOG(("descrPtr->mim == 1 : mimEn =[%d] enMimMac2Me = [%d]",mimEn,enMimMac2Me));

        if (mimEn == 0)
        {
            goto non_ipv4_arp_lbl;
        }

        if(enMimMac2Me == 0)
        {
            return GT_TRUE;
        }
        else if(descrPtr->mac2me)
        {
            return GT_TRUE;
        }
        else
        {
            goto non_ipv4_arp_lbl;
        }
    }

non_ipv4_arp_lbl:

    /* check UDB packet type triggering */
    if( devObjPtr->supportEArch )
    {
        GT_U32  udbKeyEn =
            SMEM_LION3_TTI_DEFAULT_EPORT_ENTRY_FIELD_GET(devObjPtr, descrPtr,
                SMEM_LION3_TTI_DEFAULT_E_PORT_TABLE_FIELDS_TTI_PKT_TYPE_UDB_KEY_I_ENABLE);

        __LOG(("TTI Packet Type UDB vector = 0x%x  \n", udbKeyEn));


        if(SMEM_U32_GET_FIELD(udbKeyEn,descrPtr->tti_pcktType_sip5,1))
        {
            internalTtiInfoPtr->ttiKeyType = SKERNEL_LION3_TTI_KEY_UDB_PKT_TYPE_0_E + descrPtr->tti_pcktType_sip5;

            __LOG(("UDB lookup enabled for sip5 UDB Packet Type = [%d] \n",
               descrPtr->tti_pcktType_sip5));

            return GT_TRUE;
        }
        else{
            __LOG(("UDB lookup disabled for sip5 UDB Packet Type = [%d] \n",
               descrPtr->tti_pcktType_sip5));
        }
    }

    /* check Ethernet triggering */
    internalTtiInfoPtr->ttiKeyType = SKERNEL_CHT3_TTI_KEY_ETH_E;

    __LOG(("Ethernet other : eterEn =[%d]",eterEn));

    if (eterEn)
    {
        return GT_TRUE;
    }

    return GT_FALSE;

#undef GET_BIT

}

/************************************/
/* --------- testing section ------ */
/************************************/

#define TEST_VIRTUAL_DEV_NUM            20

#define TEST_VIRTUAL_PORT_NUM_1         5
#define TEST_VIRTUAL_PORT_NUM_2         6
#define TEST_VIRTUAL_PORT_NUM_3         7
#define TEST_VIRTUAL_PORT_NUM_4         8
#define TEST_VIRTUAL_PORT_NUM_5         9
#define TEST_VIRTUAL_PORT_NUM_6         10
#define TEST_VIRTUAL_PORT_NUM_7         11
#define TEST_VIRTUAL_PORT_NUM_8         12

#define TEST_UNREG_NO_FILTER      0
#define TEST_UNREG_FILTER         1

#define TEST_VLAN_EGREES_UNTAGGED 0
#define TEST_VLAN_EGREES_TAGGED   1

#define TEST_SET_HALF_MLL_DEFAULTS(halfMll)                                   \
    halfMll.rpf_fail_cmd = 0;                                                 \
    halfMll.ttlThres = 0x11;                                                  \
    halfMll.excludeSrcVlan = 0;                                               \
    halfMll.last = 0;                                                         \
    halfMll.lll.dstInterface = SNET_DST_INTERFACE_PORT_E ;                    \
    halfMll.lll.interfaceInfo.devPort.port   = TEST_VIRTUAL_PORT_NUM_1;       \
    halfMll.lll.interfaceInfo.devPort.devNum = TEST_VIRTUAL_DEV_NUM;          \
    halfMll.vid = vidOVidxIndex;                                              \
    halfMll.isTunnelStart = 0;/* not Tunnel start --> will be in the virtual to physical mapping */\
    halfMll.tsInfo.tunnelStartType = 0;/* don't care */                       \
    halfMll.tsInfo.tunnelStartPtr = 0;/* don't care */                        \
    halfMll.unregBcFiltering      = TEST_UNREG_NO_FILTER;                     \
    halfMll.unregMcFiltering      = TEST_UNREG_NO_FILTER;                     \
    halfMll.unknownUcFiltering    = TEST_UNREG_NO_FILTER;                     \
    halfMll.vlanEgressTagMode     = TEST_VLAN_EGREES_UNTAGGED;

#define TEST_SST_ID     9

#define TEST_PHYSICAL_MAPPED_DEVICE             0

/* next ports are "UP" and connected to SLANs */
#define TEST_PHYSICAL_MAPPED_PORT_NUM_1         8
#define TEST_PHYSICAL_MAPPED_PORT_NUM_2         18
#define TEST_PHYSICAL_MAPPED_PORT_NUM_3         23

#define TEST_PHYSICAL_MAPPED_PORT_NUM_5         8
#define TEST_PHYSICAL_MAPPED_PORT_NUM_6         18
#define TEST_PHYSICAL_MAPPED_PORT_NUM_7         23

#define TEST_TUNNEL_START_INDEX_NUM             160 /* 0xA0 */
#define TEST_SRC_ID_FILTER_BMP_NUM              0xffffffff

#define TEST_MLL_POINTER                        320 /*0x140*/

#define TEST_TS_UP_MARKING_MODE                 0
#define TEST_TS_UP                              6
#define TEST_TS_TAG_ENABLE                      1
#define TEST_TS_VID                             5
#define TEST_TS_TTL_OR_HOT_LIMIT                48 /*(0x30)*/
#define TEST_TS_W_BIT                           0
#define TEST_TS_M_BIT                           0
#define TEST_TS_EGRESS_OSM_REDIRECT             0
#define TEST_TS_NEXT_HOP_MAC_DA_0_31            (0x22334455)
#define TEST_TS_NEXT_HOP_MAC_DA_32_47           (0x0011)
#define TEST_TS_WLAN_ID_BITMAP                  0xf
#define TEST_TS_DSCP                            34 /*(0x22)*/
#define TEST_TS_DSCP_MARKING_MODE               1
#define TEST_TS_802_11E_MAPPING_PROFILE         2
#define TEST_TS_UDP_SRC_PORT                    (0x1617)
#define TEST_TS_UDP_DST_PORT                    (0x2627)
#define TEST_TS_BSSID_OR_TA_0_31                (0xccddeeff)
#define TEST_TS_BSSID_OR_TA_32_47               (0xaabb)
#define TEST_TS_RA_0_15                         (0xa5a6)
#define TEST_TS_RA_16_47                        (0xa1a2a3a4)

#define TEST_MAC_TABLE_RSSI_WEIGHT_PROFILE      2
#define TEST_RSSI_AVERAGE                       50
#define TEST_RSSI_MIN                           25
#define TEST_MAC_TABLE_MAC_DA_NEW_VID           16 /*0x10*/
#define TEST_MAC_TABLE_EGRESS_MAPPING_TABLE_PROFILE 3

extern GT_VOID snetCht3MacCalcKeyAndMask
(
    IN    GT_U8  * macAddrPtr,
    OUT   GT_U32 * enrtyKeyPtr,
    OUT   GT_U32 * entryMaskPtr
);

#define SMEM_LION3_TTI_ACTION_TABLE_FIELDS_NAME                                                                         \
 STR(   SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Command                                                                   )  \
,STR(   SMEM_LION3_TTI_ACTION_TABLE_FIELDS_CPUCode                                                                   )  \
,STR(   SMEM_LION3_TTI_ACTION_TABLE_FIELDS_MirrorToAnalyzerPort                                                      )  \
,STR(   SMEM_LION3_TTI_ACTION_TABLE_FIELDS_RedirectCmd                                                               )  \
                                                                                                                        \
/*If <Redirect Command> = No Redirect (0)*/                                                                             \
,STR(      SMEM_LION3_TTI_ACTION_TABLE_FIELDS_No_Redirect_Flow_ID                                                    )  \
,STR(      SMEM_LION3_TTI_ACTION_TABLE_FIELDS_No_Redirect_IPCL_UDB_Configuration_Table_UDE_Index                     )  \
,STR(      SMEM_LION3_TTI_ACTION_TABLE_FIELDS_No_Redirect_reserved                                                   )  \
/*     <Redirect Command> != Egress_Interface (0,2,3..)*/                                                               \
,STR(      SMEM_LION3_TTI_ACTION_TABLE_FIELDS_No_Redirect_to_egress_interface_TTI_reserved_Enable                    )  \
,STR(      SMEM_LION3_TTI_ACTION_TABLE_FIELDS_No_Redirect_to_egress_interface_TTI_reserved_Value                     )  \
,STR(      SMEM_LION3_TTI_ACTION_TABLE_FIELDS_No_Redirect_to_egress_interface_Policy2_Lookup_Mode                    )  \
,STR(      SMEM_LION3_TTI_ACTION_TABLE_FIELDS_No_Redirect_to_egress_interface_Policy1_Lookup_Mode                    )  \
,STR(      SMEM_LION3_TTI_ACTION_TABLE_FIELDS_No_Redirect_to_egress_interface_Policy0_Lookup_Mode                    )  \
,STR(      SMEM_LION3_TTI_ACTION_TABLE_FIELDS_No_Redirect_to_egress_interface_IPCL_Profile_Index                     )  \
                                                                                                                        \
/*else If <Redirect Command> = Redirect to Egress Interface (1)*/                                                       \
    /*    if <UseVIDX> = 0*/                                                                                            \
,STR(            SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Interface_IsTrunk                             )  \
    /*    else if <UseVIDX> = 0 & <IsTrunk> = 1*/                                                                       \
,STR(            SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Interface_TrunkNumber                         )  \
    /*            SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Interface_Reserved*/                            \
    /*    else if <UseVIDX> = 0 & <IsTrunk> = 0*/                                                                       \
,STR(            SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Interface_ePortNumber                         )  \
,STR(            SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Interface_DeviceNumber                        )  \
    /*    else, when <UseVIDX> = 1*/                                                                                    \
,STR(            SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Interface_eVIDX                               )  \
,STR(            SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Interface_Reserved                            )  \
                                                                                                                        \
,STR(        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Interface_UseEVIDX                                )  \
,STR(        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Interface_VNT_L2_Echo                             )  \
    /*    Tunnel Start*/                                                                                                \
,STR(        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Interface_Tunnel_Start                            )  \
                                                                                                                        \
    /*    if (<Tunnel Start> == 1)*/                                                                                    \
,STR(            SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Interface_Tunnel_Index                        )  \
,STR(            SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Interface_Tunnel_Start_Passenger_Type         )  \
    /*            SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Interface_Reserved*/                            \
    /*    else, <Tunnel Start> == 0*/                                                                                   \
,STR(           SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Interface_ARP_Pointer                          )  \
                                                                                                                        \
,STR(       SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Modify_MAC_DA                                      )  \
,STR(       SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Modify_MAC_SA                                      )  \
                                                                                                                        \
                                                                                                                        \
/*else If <Redirect Command> = Redirect To Router (2)*/                                                                 \
,STR(       SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_To_Router_Router_LTT_Index                                   )  \
    /*        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect To Router    TTI reserved                                     \
            SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect To Router    IPCL Configuration Table Pointer*/                 \
/*else If <Redirect Command> = Assign VRF-ID (4)*/                                                                      \
,STR(       SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Assign_VRF_ID_VRF_ID                                                  )  \
    /*        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Assign VRF-ID     Reserved                                             \
            SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Assign VRF-ID     TTI reserved                                           \
            SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Assign VRF-ID     IPCL Configuration Table Pointer*/                     \
                                                                                                                        \
,STR(    SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Bind_To_CNC_Counter                                                      )  \
,STR(    SMEM_LION3_TTI_ACTION_TABLE_FIELDS_CNC_Counter_Index                                                        )  \
,STR(    SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Bind_To_Policer_Meter                                                    )  \
,STR(    SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Bind_To_Policer_Counter                                                  )  \
,STR(    SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Policer_Index                                                            )  \
,STR(    SMEM_LION3_TTI_ACTION_TABLE_FIELDS_SourceID_Set_Enable                                                      )  \
,STR(    SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Source_ID                                                                )  \
,STR(    SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Action_Stop                                                              )  \
,STR(    SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Bypass_Bridge                                                            )  \
,STR(    SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Bypass_Ingress_Pipe                                                      )  \
,STR(    SMEM_LION3_TTI_ACTION_TABLE_FIELDS_eVLAN_Precedence                                                         )  \
,STR(    SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Enable_Nested_VLAN                                                       )  \
,STR(    SMEM_LION3_TTI_ACTION_TABLE_FIELDS_eVID_Cmd                                                                 )  \
,STR(    SMEM_LION3_TTI_ACTION_TABLE_FIELDS_eVLAN                                                                    )  \
,STR(    SMEM_LION3_TTI_ACTION_TABLE_FIELDS_VID1_Cmd                                                                 )  \
,STR(    SMEM_LION3_TTI_ACTION_TABLE_FIELDS_VID1                                                                     )  \
,STR(    SMEM_LION3_TTI_ACTION_TABLE_FIELDS_QoS_Precedence                                                           )  \
,STR(    SMEM_LION3_TTI_ACTION_TABLE_FIELDS_QoSProfile                                                               )  \
,STR(    SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Modify_DSCP                                                              )  \
,STR(    SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Modify_UP                                                                )  \
,STR(    SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Keep_Previous_QoS                                                        )  \
,STR(    SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Trust_UP                                                                 )  \
,STR(    SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Trust_DSCP                                                               )  \
,STR(    SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Trust_EXP                                                                )  \
,STR(    SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Map_DSCP                                                                 )  \
,STR(    SMEM_LION3_TTI_ACTION_TABLE_FIELDS_UP0                                                                      )  \
,STR(    SMEM_LION3_TTI_ACTION_TABLE_FIELDS_UP1_Command                                                              )  \
,STR(    SMEM_LION3_TTI_ACTION_TABLE_FIELDS_UP1                                                                      )  \
,STR(    SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Passenger_Packet_Type                                                    )  \
,STR(    SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Copy_TTL                                                                 )  \
,STR(    SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Tunnel_Termination                                                       )  \
,STR(    SMEM_LION3_TTI_ACTION_TABLE_FIELDS_MPLS_Command                                                             )  \
,STR(    SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Hash_Mask_Index                                                          )  \
,STR(    SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Trust_QoS_Mapping_Table_Index                                            )  \
,STR(    SMEM_LION3_TTI_ACTION_TABLE_FIELDS_L_LSP_QoS_Profile_Enable                                                 )  \
,STR(    SMEM_LION3_TTI_ACTION_TABLE_FIELDS_TTL                                                                      )  \
,STR(    SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Enable_Dec_TTL                                                           )  \
,STR(    SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Source_ePort_Assignment_Enable                                           )  \
,STR(    SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Source_ePort                                                             )  \
,STR(    SMEM_LION3_TTI_ACTION_TABLE_FIELDS_TT_Header_Length                                                         )  \
,STR(    SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Is_PTP_Packet                                                            )  \
                                                                                                                        \
/*If <Is PTP PAcket> = 0*/                                                                                              \
,STR(       SMEM_LION3_TTI_ACTION_TABLE_FIELDS_NON_PTP_Packet_Timestamp_Enable                                       )  \
,STR(       SMEM_LION3_TTI_ACTION_TABLE_FIELDS_NON_PTP_Packet_Offset_Index                                           )  \
    /*        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_    reserved*/                                                         \
/*else If <Is PTP PAcket> =1*/                                                                                          \
,STR(       SMEM_LION3_TTI_ACTION_TABLE_FIELDS_PTP_Packet_PTP_Trigger_Type                                           )  \
,STR(       SMEM_LION3_TTI_ACTION_TABLE_FIELDS_PTP_Packet_PTP_Offset                                                 )  \
                                                                                                                        \
,STR(    SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Enable_OAM_Processing_When_GAL_or_OAL_Exists                             )  \
,STR(    SMEM_LION3_TTI_ACTION_TABLE_FIELDS_OAM_Processing_Enable                                                    )  \
,STR(    SMEM_LION3_TTI_ACTION_TABLE_FIELDS_CW_Based_Pseudo_Wire                                                     )  \
,STR(    SMEM_LION3_TTI_ACTION_TABLE_FIELDS_TTL_Expiry_VCCV_Enable                                                   )  \
,STR(    SMEM_LION3_TTI_ACTION_TABLE_FIELDS_PWE3_Flow_Label_Exist                                                    )  \
,STR(    SMEM_LION3_TTI_ACTION_TABLE_FIELDS_PW_CW_Based_E_Tree_Enable                                                )  \
,STR(    SMEM_LION3_TTI_ACTION_TABLE_FIELDS_UP_CFI_to_QoS_table_select_mode                                          )  \
,STR(    SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Rx_Is_Protection_Path                                                    )  \
,STR(    SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Rx_Enable_Protection_Switching                                           )  \
,STR(    SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Set_MAC2ME                                                               )  \
,STR(    SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Oam_Profile                                                              )  \
,STR(    SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Apply_non_Data_CW_Command                                                )  \
,STR(    SMEM_LION3_TTI_ACTION_TABLE_FIELDS_PW_Tag_Mode                                                              )  \
,STR(    SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Enable_Next_Lookup                                                       )  \
,STR(    SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Channel_Type_to_Opcode_Mapping_En                                        )  \
,STR(    SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Passenger_Parsing_of_Transit_MPLS_Tunnel_Enable                          )  \
,STR(    SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Passenger_Parsing_of_Non_MPLS_Transit_tunnel_Enable                      )  \
,STR(    SMEM_SIP6_TTI_ACTION_TABLE_FIELDS_Skip_Fdb_SA_Lookup                                                        )  \
,STR(    SMEM_SIP6_TTI_ACTION_TABLE_FIELDS_Set_Ipv6_Segment_Routing_End_Node                                         )  \
,STR(    SMEM_SIP6_TTI_ACTION_TABLE_FIELDS_Tunnel_Header_Length_Anchor_Type                                          )  \
,STR(    SMEM_SIP6_10_TTI_ACTION_TABLE_FIELDS_Triger_Cnc_Hash_Client                                                 )  \
,STR(    SMEM_SIP6_10_TTI_ACTION_TABLE_FIELDS_Ipfix_enable                                                           )  \
,STR(    SMEM_SIP6_10_TTI_ACTION_TABLE_FIELDS_Ppu_Profile                                                            )  \
                                                                                                                        \
,STR(    SMEM_SIP6_30_TTI_ACTION_TABLE_FIELDS_Group_IDs_Assignment_Enable                                            )  \
,STR(    SMEM_SIP6_30_TTI_ACTION_TABLE_FIELDS_Group_IDs                                                              )  \
,STR(    SMEM_SIP6_30_TTI_ACTION_TABLE_FIELDS_Generic_12b_Action                                                     )  \
                                                                                                                        \
,STR(    SMEM_SIP7_TTI_ACTION_TABLE_FIELDS_trg_vport                                                                 )  \
,STR(    SMEM_SIP7_TTI_ACTION_TABLE_FIELDS_use_trg_vport                                                             )  \
,STR(    SMEM_SIP7_TTI_ACTION_TABLE_FIELDS_stop_mpls                                                                 )  \
,STR(    SMEM_SIP7_TTI_ACTION_TABLE_FIELDS_srv6_psp_enabled                                                          )  \
,STR(    SMEM_SIP7_TTI_ACTION_TABLE_FIELDS_qcn_dlb_proc_en                                                           )  \
,STR(    SMEM_SIP7_TTI_ACTION_TABLE_FIELDS_generic_classification_value                                              )  \
,STR(    SMEM_SIP7_TTI_ACTION_TABLE_FIELDS_qos_mapping_mode                                                          )  \
,STR(    SMEM_SIP7_TTI_ACTION_TABLE_FIELDS_force_dp                                                                  )  \
,STR(    SMEM_SIP7_TTI_ACTION_TABLE_FIELDS_qos_model                                                                 )  \
,STR(    SMEM_SIP7_TTI_ACTION_TABLE_FIELDS_ttl_model                                                                 )  \
,STR(    SMEM_SIP7_TTI_ACTION_TABLE_FIELDS_source_vport                                                              )  \
,STR(    SMEM_SIP7_TTI_ACTION_TABLE_FIELDS_Source_Interface_Assignment_Enable                                            )  \
,STR(    SMEM_SIP7_TTI_ACTION_TABLE_FIELDS_source_Vport_profile_assignment_en                                        )  \
,STR(    SMEM_SIP7_TTI_ACTION_TABLE_FIELDS_disable_em_lookup                                                         )  \
,STR(    SMEM_SIP7_TTI_ACTION_TABLE_FIELDS_cancel_drop                                                               )  \
,STR(    SMEM_SIP7_TTI_ACTION_TABLE_FIELDS_No_Redirect_to_egress_interface_Mpcl_Lookup_Mode                          )

char * lion3TtiActionTableFieldsNames[SMEM_LION3_TTI_ACTION_TABLE_FIELDS___LAST_VALUE___E] =
    {SMEM_LION3_TTI_ACTION_TABLE_FIELDS_NAME};

static SNET_ENTRY_FORMAT_TABLE_STC lion3TtiActionTableFieldsFormat[SMEM_LION3_TTI_ACTION_TABLE_FIELDS___LAST_VALUE___E] =
{
/*        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Command*/
    STANDARD_FIELD_MAC(3),

/*        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_CPUCode*/
    STANDARD_FIELD_MAC(8),

/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_MirrorToAnalyzerPort*/
    STANDARD_FIELD_MAC(3),

/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_RedirectCmd*/
    STANDARD_FIELD_MAC(3),

/*If <Redirect Command> = No Redirect (0)*/
/*,           SMEM_LION3_TTI_ACTION_TABLE_FIELDS_No_Redirect_Flow_ID*/
        {FIELD_SET_IN_RUNTIME_CNS,
         12,
         SMEM_LION3_TTI_ACTION_TABLE_FIELDS_RedirectCmd},
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_No_Redirect_IPCL_UDB_Configuration_Table_UDE_Index*/
        STANDARD_FIELD_MAC(3),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_No_Redirect_reserved*/
        STANDARD_FIELD_MAC(1),
/*     <Redirect Command> != Egress_Interface (0,2,3..)*/
/*,           SMEM_LION3_TTI_ACTION_TABLE_FIELDS_No_Redirect_to_egress_interface_TTI_reserved_Enable*/
        STANDARD_FIELD_MAC(1),
/*,           SMEM_LION3_TTI_ACTION_TABLE_FIELDS_No_Redirect_to_egress_interface_TTI_reserved_Value*/
        STANDARD_FIELD_MAC(13),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_No_Redirect_to_egress_interface_Policy2_Lookup_Mode*/
        STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_No_Redirect_to_egress_interface_Policy1_Lookup_Mode*/
        STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_No_Redirect_to_egress_interface_Policy0_Lookup_Mode*/
        STANDARD_FIELD_MAC(1),
/*,           SMEM_LION3_TTI_ACTION_TABLE_FIELDS_No_Redirect_to_egress_interface_IPCL_Profile_Index*/
        STANDARD_FIELD_MAC(13),

/*else If <Redirect Command> = Redirect to Egress Interface (1)*/
/*    if <UseVIDX> = 0*/
/*,                SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Interface_IsTrunk*/
            {FIELD_SET_IN_RUNTIME_CNS,
             1,
             SMEM_LION3_TTI_ACTION_TABLE_FIELDS_RedirectCmd},
    /*    else if <UseVIDX> = 0 & <IsTrunk> = 1*/
/*,                SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Interface_TrunkNumber*/
                    {FIELD_SET_IN_RUNTIME_CNS,
                     12,
                     SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Interface_IsTrunk},
    /*            SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Interface_Reserved*/
    /*    else if <UseVIDX> = 0 & <IsTrunk> = 0*/
/*,                SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Interface_ePortNumber*/
                    {FIELD_SET_IN_RUNTIME_CNS,
                     13,
                     SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Interface_IsTrunk},
/*,                SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Interface_DeviceNumber*/
                    STANDARD_FIELD_MAC(10),
/*    else, when <UseVIDX> = 1*/
/*,                SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Interface_eVIDX*/
            {FIELD_SET_IN_RUNTIME_CNS,
             16,
             SMEM_LION3_TTI_ACTION_TABLE_FIELDS_RedirectCmd},
/*,               SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Interface_Reserved*/
            STANDARD_FIELD_MAC(8),
/*,            SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Interface_UseEVIDX*/
        STANDARD_FIELD_MAC(1),
/*,            SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Interface_VNT_L2_Echo*/
        STANDARD_FIELD_MAC(1),

/*    Tunnel Start*/
/*,            SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Interface_Tunnel_Start*/
        STANDARD_FIELD_MAC(1),
/*    if (<Tunnel Start> == 1)*/
/*,                SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Interface_Tunnel_Index*/
            STANDARD_FIELD_MAC(15),
/*,                SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Interface_Tunnel_Start_Passenger_Type*/
            STANDARD_FIELD_MAC(1),
/*            SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Interface_Reserved*/
/*    else, <Tunnel Start> == 0*/
/*,                SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Interface_ARP_Pointer*/
            {FIELD_SET_IN_RUNTIME_CNS,
             17,
             SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Interface_Tunnel_Start},

/*,           SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Modify_MAC_DA*/
        STANDARD_FIELD_MAC(1),
/*,           SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Modify_MAC_SA*/
        STANDARD_FIELD_MAC(1),

/*else If <Redirect Command> = Redirect To Router (2)*/
/*,           SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_To_Router_Router_LTT_Index*/
        {FIELD_SET_IN_RUNTIME_CNS,
         16,
         SMEM_LION3_TTI_ACTION_TABLE_FIELDS_RedirectCmd},

/*        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect To Router    TTI reserved
        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect To Router    IPCL Configuration Table Pointer*/

/*else If <Redirect Command> = Assign VRF-ID (4)*/
/*,           SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Assign_VRF_ID_VRF_ID*/
        {FIELD_SET_IN_RUNTIME_CNS,
         12,
         SMEM_LION3_TTI_ACTION_TABLE_FIELDS_RedirectCmd},
/*        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Assign VRF-ID     Reserved
        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Assign VRF-ID     TTI reserved
        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Assign VRF-ID     IPCL Configuration Table Pointer*/

/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Bind_To_CNC_Counter*/
    {FIELD_SET_IN_RUNTIME_CNS,
     1,
     SMEM_LION3_TTI_ACTION_TABLE_FIELDS_No_Redirect_to_egress_interface_IPCL_Profile_Index},
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_CNC_Counter_Index*/
    STANDARD_FIELD_MAC(14),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Bind_To_Policer_Meter*/
    STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Bind_To_Policer_Counter*/
    STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Policer_Index*/
    STANDARD_FIELD_MAC(16),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_SourceID_Set_Enable*/
    STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Source_ID*/
    STANDARD_FIELD_MAC(12),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Action_Stop*/
    STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Bypass_Bridge*/
    STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Bypass_Ingress_Pipe*/
    STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_eVLAN_Precedence*/
    STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Enable_Nested_VLAN*/
    STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_eVID_Cmd*/
    STANDARD_FIELD_MAC(3),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_eVLAN*/
    STANDARD_FIELD_MAC(13),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_VID1_Cmd*/
    STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_VID1*/
    STANDARD_FIELD_MAC(12),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_QoS_Precedence*/
    STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_QoSProfile*/
    STANDARD_FIELD_MAC(7),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Modify_DSCP*/
    STANDARD_FIELD_MAC(2),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Modify_UP*/
    STANDARD_FIELD_MAC(2),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Keep_Previous_QoS*/
    STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Trust_UP*/
    STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Trust_DSCP*/
    STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Trust_EXP*/
    STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Map_DSCP*/
    STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_UP0*/
    STANDARD_FIELD_MAC(3),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_UP1_Command*/
    STANDARD_FIELD_MAC(2),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_UP1*/
    STANDARD_FIELD_MAC(3),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Passenger_Packet_Type*/
    STANDARD_FIELD_MAC(2),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Copy_TTL*/
    STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Tunnel_Termination*/
    STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_MPLS_Command*/
    STANDARD_FIELD_MAC(3),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Hash_Mask_Index*/
    STANDARD_FIELD_MAC(4),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Trust_QoS_Mapping_Table_Index*/
    STANDARD_FIELD_MAC(4),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_L_LSP_QoS_Profile_Enable*/
    STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_TTL*/
    STANDARD_FIELD_MAC(8),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Enable_Dec_TTL*/
    STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Source_ePort_Assignment_Enable*/
    STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Source_ePort*/
    STANDARD_FIELD_MAC(13),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_TT_Header_Length*/
    STANDARD_FIELD_MAC(5),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Is_PTP_Packet*/
    STANDARD_FIELD_MAC(1),

/*If <Is PTP PAcket> = 0*/
/*,           SMEM_LION3_TTI_ACTION_TABLE_FIELDS_NON_PTP_Packet_Timestamp_Enable*/
        {FIELD_SET_IN_RUNTIME_CNS,
         1,
         SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Is_PTP_Packet},
/*,           SMEM_LION3_TTI_ACTION_TABLE_FIELDS_NON_PTP_Packet_Offset_Index*/
        STANDARD_FIELD_MAC(7),
/*        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_    reserved*/
/*else If <Is PTP PAcket> =1*/
/*,           SMEM_LION3_TTI_ACTION_TABLE_FIELDS_PTP_Packet_PTP_Trigger_Type*/
        {FIELD_SET_IN_RUNTIME_CNS,
         2,
         SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Is_PTP_Packet},
/*,           SMEM_LION3_TTI_ACTION_TABLE_FIELDS_PTP_Packet_PTP_Offset*/
        STANDARD_FIELD_MAC(7),

/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Enable_OAM_Processing_When_GAL_or_OAL_Exists*/
    {FIELD_SET_IN_RUNTIME_CNS,
     0,
     SMEM_LION3_TTI_ACTION_TABLE_FIELDS_PTP_Packet_PTP_Offset},
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_OAM_Processing_Enable*/
     STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_CW_Based_Pseudo_Wire*/
     STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_TTL_Expiry_VCCV_Enable*/
     STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_PWE3_Flow_Label_Exist*/
     STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_PW_CW_Based_E_Tree_Enable*/
     STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_UP_CFI_to_QoS_table_select_mode*/
     STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Rx_Is_Protection_Path*/
     STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Rx_Enable_Protection_Switching*/
     STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Set_MAC2ME*/
     STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Oam_Profile*/
     STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Apply_non_Data_CW_Command*/
     STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_PW_Tag_Mode*/
     STANDARD_FIELD_MAC(2),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Enable_Next_Lookup*/
     STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Channel_Type_to_Opcode_Mapping_En*/
     STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Passenger_Parsing_of_Transit_MPLS_Tunnel_Enable*/
     STANDARD_FIELD_MAC(2),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Passenger_Parsing_of_Non_MPLS_Transit_tunnel_Enable*/
     STANDARD_FIELD_MAC(1)
};

static SNET_ENTRY_FORMAT_TABLE_STC sip5_10TtiActionTableFieldsFormat[SMEM_LION3_TTI_ACTION_TABLE_FIELDS___LAST_VALUE___E] =
{
/*        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Command*/
    STANDARD_FIELD_MAC(3),

/*        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_CPUCode*/
    STANDARD_FIELD_MAC(8),

/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_MirrorToAnalyzerPort*/
    STANDARD_FIELD_MAC(3),

/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_RedirectCmd*/
    STANDARD_FIELD_MAC(3),

/*If <Redirect Command> = No Redirect (0)*/
/*,           SMEM_LION3_TTI_ACTION_TABLE_FIELDS_No_Redirect_Flow_ID*/
        {FIELD_SET_IN_RUNTIME_CNS,
         12,
         SMEM_LION3_TTI_ACTION_TABLE_FIELDS_RedirectCmd},
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_No_Redirect_IPCL_UDB_Configuration_Table_UDE_Index*/
        STANDARD_FIELD_MAC(3),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_No_Redirect_reserved*/
        STANDARD_FIELD_MAC(1),
/*     <Redirect Command> != Egress_Interface (0,2,3..)*/
/*,           SMEM_LION3_TTI_ACTION_TABLE_FIELDS_No_Redirect_to_egress_interface_TTI_reserved_Enable*/
        STANDARD_FIELD_MAC(1),
/*,           SMEM_LION3_TTI_ACTION_TABLE_FIELDS_No_Redirect_to_egress_interface_TTI_reserved_Value*/
        STANDARD_FIELD_MAC(13),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_No_Redirect_to_egress_interface_Policy2_Lookup_Mode*/
        STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_No_Redirect_to_egress_interface_Policy1_Lookup_Mode*/
        STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_No_Redirect_to_egress_interface_Policy0_Lookup_Mode*/
        STANDARD_FIELD_MAC(1),
/*,           SMEM_LION3_TTI_ACTION_TABLE_FIELDS_No_Redirect_to_egress_interface_IPCL_Profile_Index*/
        STANDARD_FIELD_MAC(13),

/*else If <Redirect Command> = Redirect to Egress Interface (1)*/
/*    if <UseVIDX> = 0*/
/*,                SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Interface_IsTrunk*/
            {FIELD_SET_IN_RUNTIME_CNS,
             1,
             SMEM_LION3_TTI_ACTION_TABLE_FIELDS_RedirectCmd},
    /*    else if <UseVIDX> = 0 & <IsTrunk> = 1*/
/*,                SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Interface_TrunkNumber*/
                    {FIELD_SET_IN_RUNTIME_CNS,
                     12,
                     SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Interface_IsTrunk},
    /*            SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Interface_Reserved*/
    /*    else if <UseVIDX> = 0 & <IsTrunk> = 0*/
/*,                SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Interface_ePortNumber*/
                    {FIELD_SET_IN_RUNTIME_CNS,
                     13,
                     SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Interface_IsTrunk},
/*,                SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Interface_DeviceNumber*/
                    STANDARD_FIELD_MAC(10),
/*    else, when <UseVIDX> = 1*/
/*,                SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Interface_eVIDX*/
            {FIELD_SET_IN_RUNTIME_CNS,
             16,
             SMEM_LION3_TTI_ACTION_TABLE_FIELDS_RedirectCmd},
/*,               SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Interface_Reserved*/
            STANDARD_FIELD_MAC(8),
/*,            SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Interface_UseEVIDX*/
        STANDARD_FIELD_MAC(1),
/*,            SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Interface_VNT_L2_Echo*/
        STANDARD_FIELD_MAC(1),

/*    Tunnel Start*/
/*,            SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Interface_Tunnel_Start*/
        STANDARD_FIELD_MAC(1),
/*    if (<Tunnel Start> == 1)*/
/*,                SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Interface_Tunnel_Index*/
            STANDARD_FIELD_MAC(15),
/*,                SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Interface_Tunnel_Start_Passenger_Type*/
            STANDARD_FIELD_MAC(1),
/*            SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Interface_Reserved*/
/*    else, <Tunnel Start> == 0*/
/*,                SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Interface_ARP_Pointer*/
            {FIELD_SET_IN_RUNTIME_CNS,
             17,
             SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Interface_Tunnel_Start},

/*,           SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Modify_MAC_DA*/
        STANDARD_FIELD_MAC(1),
/*,           SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Modify_MAC_SA*/
        STANDARD_FIELD_MAC(1),

/*else If <Redirect Command> = Redirect To Router (2)*/
/*,           SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_To_Router_Router_LTT_Index*/
        {FIELD_SET_IN_RUNTIME_CNS,
         16,
         SMEM_LION3_TTI_ACTION_TABLE_FIELDS_RedirectCmd},

/*        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect To Router    TTI reserved
        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect To Router    IPCL Configuration Table Pointer*/

/*else If <Redirect Command> = Assign VRF-ID (4)*/
/*,           SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Assign_VRF_ID_VRF_ID*/
        {FIELD_SET_IN_RUNTIME_CNS,
         12,
         SMEM_LION3_TTI_ACTION_TABLE_FIELDS_RedirectCmd},
/*        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Assign VRF-ID     Reserved
        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Assign VRF-ID     TTI reserved
        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Assign VRF-ID     IPCL Configuration Table Pointer*/

/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Bind_To_CNC_Counter*/
    {FIELD_SET_IN_RUNTIME_CNS,
     1,
     SMEM_LION3_TTI_ACTION_TABLE_FIELDS_No_Redirect_to_egress_interface_IPCL_Profile_Index},
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_CNC_Counter_Index*/
    STANDARD_FIELD_MAC(14),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Bind_To_Policer_Meter*/
    STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Bind_To_Policer_Counter*/
    STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Policer_Index*/
    STANDARD_FIELD_MAC(16),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_SourceID_Set_Enable*/
    STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Source_ID*/
    STANDARD_FIELD_MAC(12),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Action_Stop*/
    STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Bypass_Bridge*/
    STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Bypass_Ingress_Pipe*/
    STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_eVLAN_Precedence*/
    STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Enable_Nested_VLAN*/
    STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_eVID_Cmd*/
    STANDARD_FIELD_MAC(3),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_eVLAN*/
    STANDARD_FIELD_MAC(13),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_VID1_Cmd*/
    STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_VID1*/
    STANDARD_FIELD_MAC(12),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_QoS_Precedence*/
    STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_QoSProfile*/
    STANDARD_FIELD_MAC(7),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Modify_DSCP*/
    STANDARD_FIELD_MAC(2),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Modify_UP*/
    STANDARD_FIELD_MAC(2),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Keep_Previous_QoS*/
    STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Trust_UP*/
    STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Trust_DSCP*/
    STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Trust_EXP*/
    STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Map_DSCP*/
    STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_UP0*/
    STANDARD_FIELD_MAC(3),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_UP1_Command*/
    STANDARD_FIELD_MAC(2),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_UP1*/
    STANDARD_FIELD_MAC(3),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Passenger_Packet_Type*/
    STANDARD_FIELD_MAC(2),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Copy_TTL*/
    STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Tunnel_Termination*/
    STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_MPLS_Command*/
    STANDARD_FIELD_MAC(3),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Hash_Mask_Index*/
    STANDARD_FIELD_MAC(4),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Trust_QoS_Mapping_Table_Index*/
    STANDARD_FIELD_MAC(4),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_L_LSP_QoS_Profile_Enable*/
    STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_TTL*/
    STANDARD_FIELD_MAC(8),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Enable_Dec_TTL*/
    STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Source_ePort_Assignment_Enable*/
    STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Source_ePort*/
    STANDARD_FIELD_MAC(13),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_TT_Header_Length*/
    STANDARD_FIELD_MAC(5),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Is_PTP_Packet*/
    STANDARD_FIELD_MAC(1),

/*If <Is PTP PAcket> = 0*/
/*,           SMEM_LION3_TTI_ACTION_TABLE_FIELDS_NON_PTP_Packet_Timestamp_Enable*/
        {FIELD_SET_IN_RUNTIME_CNS,
         1,
         SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Is_PTP_Packet},
/*,           SMEM_LION3_TTI_ACTION_TABLE_FIELDS_NON_PTP_Packet_Offset_Index*/
        STANDARD_FIELD_MAC(7),
/*        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_    reserved*/
/*else If <Is PTP PAcket> =1*/
/*,           SMEM_LION3_TTI_ACTION_TABLE_FIELDS_PTP_Packet_PTP_Trigger_Type*/
        {FIELD_SET_IN_RUNTIME_CNS,
         2,
         SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Is_PTP_Packet},
/*,           SMEM_LION3_TTI_ACTION_TABLE_FIELDS_PTP_Packet_PTP_Offset*/
        STANDARD_FIELD_MAC(6),

/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Enable_OAM_Processing_When_GAL_or_OAL_Exists*/
    {FIELD_SET_IN_RUNTIME_CNS,
     1,/*was 0 in sip 5*/
     SMEM_LION3_TTI_ACTION_TABLE_FIELDS_PTP_Packet_PTP_Offset},
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_OAM_Processing_Enable*/
     STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_CW_Based_Pseudo_Wire*/
     STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_TTL_Expiry_VCCV_Enable*/
     STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_PWE3_Flow_Label_Exist*/
     STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_PW_CW_Based_E_Tree_Enable*/
     STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_UP_CFI_to_QoS_table_select_mode*/
     STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Rx_Is_Protection_Path*/
     STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Rx_Enable_Protection_Switching*/
     STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Set_MAC2ME*/
     STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Oam_Profile*/
     STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Apply_non_Data_CW_Command*/
     STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_PW_Tag_Mode*/
     STANDARD_FIELD_MAC(2),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Enable_Next_Lookup*/
     STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Channel_Type_to_Opcode_Mapping_En*/
     STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Passenger_Parsing_of_Transit_MPLS_Tunnel_Enable*/
     STANDARD_FIELD_MAC(2),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Passenger_Parsing_of_Non_MPLS_Transit_tunnel_Enable*/
     STANDARD_FIELD_MAC(1)
};

static SNET_ENTRY_FORMAT_TABLE_STC sip5_20TtiActionTableFieldsFormat[SMEM_LION3_TTI_ACTION_TABLE_FIELDS___LAST_VALUE___E] =
{
/*        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Command*/
    STANDARD_FIELD_MAC(3),

/*        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_CPUCode*/
    STANDARD_FIELD_MAC(8),

/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_MirrorToAnalyzerPort*/
    STANDARD_FIELD_MAC(3),

/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_RedirectCmd*/
    STANDARD_FIELD_MAC(3),

/*If <Redirect Command> = No Redirect (0)*/
/*,           SMEM_LION3_TTI_ACTION_TABLE_FIELDS_No_Redirect_Flow_ID*/
        {FIELD_SET_IN_RUNTIME_CNS,
         13,/*in sip 5 was 12*/
         SMEM_LION3_TTI_ACTION_TABLE_FIELDS_RedirectCmd},
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_No_Redirect_IPCL_UDB_Configuration_Table_UDE_Index*/
        STANDARD_FIELD_MAC(3),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_No_Redirect_reserved*/
        STANDARD_FIELD_MAC(2),
/*     <Redirect Command> != Egress_Interface (0,2,3..)*/
/*,           SMEM_LION3_TTI_ACTION_TABLE_FIELDS_No_Redirect_to_egress_interface_TTI_reserved_Enable*/
        STANDARD_FIELD_MAC(1),
/*,           SMEM_LION3_TTI_ACTION_TABLE_FIELDS_No_Redirect_to_egress_interface_TTI_reserved_Value*/
        STANDARD_FIELD_MAC(13),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_No_Redirect_to_egress_interface_Policy2_Lookup_Mode*/
        STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_No_Redirect_to_egress_interface_Policy1_Lookup_Mode*/
        STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_No_Redirect_to_egress_interface_Policy0_Lookup_Mode*/
        STANDARD_FIELD_MAC(1),
/*,           SMEM_LION3_TTI_ACTION_TABLE_FIELDS_No_Redirect_to_egress_interface_IPCL_Profile_Index*/
        STANDARD_FIELD_MAC(13),

/*else If <Redirect Command> = Redirect to Egress Interface (1)*/
/*    if <UseVIDX> = 0*/
/*,                SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Interface_IsTrunk*/
            {FIELD_SET_IN_RUNTIME_CNS,
             1,
             SMEM_LION3_TTI_ACTION_TABLE_FIELDS_RedirectCmd},
    /*    else if <UseVIDX> = 0 & <IsTrunk> = 1*/
/*,                SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Interface_TrunkNumber*/
                    {FIELD_SET_IN_RUNTIME_CNS,
                     12,
                     SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Interface_IsTrunk},
    /*            SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Interface_Reserved*/
    /*    else if <UseVIDX> = 0 & <IsTrunk> = 0*/
/*,                SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Interface_ePortNumber*/
                    {FIELD_SET_IN_RUNTIME_CNS,
                     14,/* in sip 5 was 13 */
                     SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Interface_IsTrunk},
/*,                SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Interface_DeviceNumber*/
                    STANDARD_FIELD_MAC(10),
/*    else, when <UseVIDX> = 1*/
/*,                SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Interface_eVIDX*/
            {FIELD_SET_IN_RUNTIME_CNS,
             16,
             SMEM_LION3_TTI_ACTION_TABLE_FIELDS_RedirectCmd},
/*,               SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Interface_Reserved*/
            STANDARD_FIELD_MAC(9),
/*,            SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Interface_UseEVIDX*/
        {FIELD_SET_IN_RUNTIME_CNS,
         1,
         SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Interface_DeviceNumber},
/*,            SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Interface_VNT_L2_Echo*/
        STANDARD_FIELD_MAC(1),

/*    Tunnel Start*/
/*,            SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Interface_Tunnel_Start*/
        STANDARD_FIELD_MAC(1),
/*    if (<Tunnel Start> == 1)*/
/*,                SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Interface_Tunnel_Index*/
            STANDARD_FIELD_MAC(16),/* in sip 5 was 15 */
/*,                SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Interface_Tunnel_Start_Passenger_Type*/
            STANDARD_FIELD_MAC(1),
/*            SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Interface_Reserved*/
/*    else, <Tunnel Start> == 0*/
/*,                SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Interface_ARP_Pointer*/
            {FIELD_SET_IN_RUNTIME_CNS,
             18,/* in sip 5 was 17 */
             SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Interface_Tunnel_Start},

/*,           SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Modify_MAC_DA*/
        STANDARD_FIELD_MAC(1),
/*,           SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Modify_MAC_SA*/
        STANDARD_FIELD_MAC(1),

/*else If <Redirect Command> = Redirect To Router (2)*/
/*,           SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_To_Router_Router_LTT_Index*/
        {FIELD_SET_IN_RUNTIME_CNS,
         18,/*in sip5 was 16*/
         SMEM_LION3_TTI_ACTION_TABLE_FIELDS_RedirectCmd},

/*        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect To Router    TTI reserved
        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect To Router    IPCL Configuration Table Pointer*/

/*else If <Redirect Command> = Assign VRF-ID (4)*/
/*,           SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Assign_VRF_ID_VRF_ID*/
        {FIELD_SET_IN_RUNTIME_CNS,
         12,
         SMEM_LION3_TTI_ACTION_TABLE_FIELDS_RedirectCmd},
/*        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Assign VRF-ID     Reserved
        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Assign VRF-ID     TTI reserved
        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Assign VRF-ID     IPCL Configuration Table Pointer*/

/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Bind_To_CNC_Counter*/
    {FIELD_SET_IN_RUNTIME_CNS,
     1,
     SMEM_LION3_TTI_ACTION_TABLE_FIELDS_No_Redirect_to_egress_interface_IPCL_Profile_Index},
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_CNC_Counter_Index*/
    STANDARD_FIELD_MAC(14),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Bind_To_Policer_Meter*/
    STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Bind_To_Policer_Counter*/
    STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Policer_Index*/
    STANDARD_FIELD_MAC(14),/* in sip 5 was 16 */
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_SourceID_Set_Enable*/
    STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Source_ID*/
    STANDARD_FIELD_MAC(12),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Action_Stop*/
    STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Bypass_Bridge*/
    STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Bypass_Ingress_Pipe*/
    STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_eVLAN_Precedence*/
    STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Enable_Nested_VLAN*/
    STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_eVID_Cmd*/
    STANDARD_FIELD_MAC(3),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_eVLAN*/
    STANDARD_FIELD_MAC(13),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_VID1_Cmd*/
    STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_VID1*/
    STANDARD_FIELD_MAC(12),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_QoS_Precedence*/
    STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_QoSProfile*/
    STANDARD_FIELD_MAC(7),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Modify_DSCP*/
    STANDARD_FIELD_MAC(2),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Modify_UP*/
    STANDARD_FIELD_MAC(2),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Keep_Previous_QoS*/
    STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Trust_UP*/
    STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Trust_DSCP*/
    STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Trust_EXP*/
    STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Map_DSCP*/
    STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_UP0*/
    STANDARD_FIELD_MAC(3),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_UP1_Command*/
    STANDARD_FIELD_MAC(2),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_UP1*/
    STANDARD_FIELD_MAC(3),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Passenger_Packet_Type*/
    STANDARD_FIELD_MAC(2),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Copy_TTL*/
    STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Tunnel_Termination*/
    STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_MPLS_Command*/
    STANDARD_FIELD_MAC(3),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Hash_Mask_Index*/
    STANDARD_FIELD_MAC(4),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Trust_QoS_Mapping_Table_Index*/
    STANDARD_FIELD_MAC(4),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_L_LSP_QoS_Profile_Enable*/
    STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_TTL*/
    STANDARD_FIELD_MAC(8),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Enable_Dec_TTL*/
    STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Source_ePort_Assignment_Enable*/
    STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Source_ePort*/
    STANDARD_FIELD_MAC(14),/* in sip 5 was 13 */
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_TT_Header_Length*/
    STANDARD_FIELD_MAC(6),/* in sip 5 was 5*/
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Is_PTP_Packet*/
    STANDARD_FIELD_MAC(1),

/*If <Is PTP PAcket> = 0*/
/*,           SMEM_LION3_TTI_ACTION_TABLE_FIELDS_NON_PTP_Packet_Timestamp_Enable*/
        {FIELD_SET_IN_RUNTIME_CNS,
         1,
         SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Is_PTP_Packet},
/*,           SMEM_LION3_TTI_ACTION_TABLE_FIELDS_NON_PTP_Packet_Offset_Index*/
        STANDARD_FIELD_MAC(7),
/*        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_    reserved*/
/*else If <Is PTP PAcket> =1*/
/*,           SMEM_LION3_TTI_ACTION_TABLE_FIELDS_PTP_Packet_PTP_Trigger_Type*/
        {FIELD_SET_IN_RUNTIME_CNS,
         2,
         SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Is_PTP_Packet},
/*,           SMEM_LION3_TTI_ACTION_TABLE_FIELDS_PTP_Packet_PTP_Offset*/
        STANDARD_FIELD_MAC(6),/*was 7 in sip 5*/

/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Enable_OAM_Processing_When_GAL_or_OAL_Exists*/
    {FIELD_SET_IN_RUNTIME_CNS,
     1,/*was 0 in sip 5*/
     SMEM_LION3_TTI_ACTION_TABLE_FIELDS_PTP_Packet_PTP_Offset},
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_OAM_Processing_Enable*/
     STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_CW_Based_Pseudo_Wire*/
     STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_TTL_Expiry_VCCV_Enable*/
     STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_PWE3_Flow_Label_Exist*/
     STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_PW_CW_Based_E_Tree_Enable*/
     STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_UP_CFI_to_QoS_table_select_mode*/
     STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Rx_Is_Protection_Path*/
     STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Rx_Enable_Protection_Switching*/
     STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Set_MAC2ME*/
     STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Oam_Profile*/
     STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Apply_non_Data_CW_Command*/
     STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_PW_Tag_Mode*/
     STANDARD_FIELD_MAC(2),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Enable_Next_Lookup*/
     STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Channel_Type_to_Opcode_Mapping_En*/
     STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Passenger_Parsing_of_Transit_MPLS_Tunnel_Enable*/
     STANDARD_FIELD_MAC(2),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Passenger_Parsing_of_Non_MPLS_Transit_tunnel_Enable*/
     STANDARD_FIELD_MAC(1)
};

static SNET_ENTRY_FORMAT_TABLE_STC sip6_ttiActionTableFieldsFormat[SMEM_LION3_TTI_ACTION_TABLE_FIELDS___LAST_VALUE___E] =
{
/*        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Command*/
    STANDARD_FIELD_MAC(3),

/*        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_CPUCode*/
    STANDARD_FIELD_MAC(8),

/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_MirrorToAnalyzerPort*/
    STANDARD_FIELD_MAC(3),

/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_RedirectCmd*/
    STANDARD_FIELD_MAC(3),

/*If <Redirect Command> = No Redirect (0)*/
/*,           SMEM_LION3_TTI_ACTION_TABLE_FIELDS_No_Redirect_Flow_ID*/
        {FIELD_SET_IN_RUNTIME_CNS,
         13,/*in sip 5 was 12*/
         SMEM_LION3_TTI_ACTION_TABLE_FIELDS_RedirectCmd},
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_No_Redirect_IPCL_UDB_Configuration_Table_UDE_Index*/
        STANDARD_FIELD_MAC(3),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_No_Redirect_reserved*/
        STANDARD_FIELD_MAC(2),
/*     <Redirect Command> != Egress_Interface (0,2,3..)*/
/*,           SMEM_LION3_TTI_ACTION_TABLE_FIELDS_No_Redirect_to_egress_interface_TTI_reserved_Enable*/
        STANDARD_FIELD_MAC(1),
/*,           SMEM_LION3_TTI_ACTION_TABLE_FIELDS_No_Redirect_to_egress_interface_TTI_reserved_Value*/
        STANDARD_FIELD_MAC(13),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_No_Redirect_to_egress_interface_Policy2_Lookup_Mode*/
        STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_No_Redirect_to_egress_interface_Policy1_Lookup_Mode*/
        STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_No_Redirect_to_egress_interface_Policy0_Lookup_Mode*/
        STANDARD_FIELD_MAC(1),
/*,           SMEM_LION3_TTI_ACTION_TABLE_FIELDS_No_Redirect_to_egress_interface_IPCL_Profile_Index*/
        STANDARD_FIELD_MAC(13),

/*else If <Redirect Command> = Redirect to Egress Interface (1)*/
/*    if <UseVIDX> = 0*/
/*,                SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Interface_IsTrunk*/
            {FIELD_SET_IN_RUNTIME_CNS,
             1,
             SMEM_LION3_TTI_ACTION_TABLE_FIELDS_RedirectCmd},
    /*    else if <UseVIDX> = 0 & <IsTrunk> = 1*/
/*,                SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Interface_TrunkNumber*/
                    {FIELD_SET_IN_RUNTIME_CNS,
                     12,
                     SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Interface_IsTrunk},
    /*            SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Interface_Reserved*/
    /*    else if <UseVIDX> = 0 & <IsTrunk> = 0*/
/*,                SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Interface_ePortNumber*/
                    {FIELD_SET_IN_RUNTIME_CNS,
                     14,/* in sip 5 was 13 */
                     SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Interface_IsTrunk},
/*,                SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Interface_DeviceNumber*/
                    STANDARD_FIELD_MAC(10),
/*    else, when <UseVIDX> = 1*/
/*,                SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Interface_eVIDX*/
            {FIELD_SET_IN_RUNTIME_CNS,
             16,
             SMEM_LION3_TTI_ACTION_TABLE_FIELDS_RedirectCmd},
/*,               SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Interface_Reserved*/
            STANDARD_FIELD_MAC(9),
/*,            SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Interface_UseEVIDX*/
        {FIELD_SET_IN_RUNTIME_CNS,
         1,
         SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Interface_DeviceNumber},
/*,            SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Interface_VNT_L2_Echo*/
        STANDARD_FIELD_MAC(1),

/*    Tunnel Start*/
/*,            SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Interface_Tunnel_Start*/
        STANDARD_FIELD_MAC(1),
/*    if (<Tunnel Start> == 1)*/
/*,                SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Interface_Tunnel_Index*/
            STANDARD_FIELD_MAC(16),/* in sip 5 was 15 */
/*,                SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Interface_Tunnel_Start_Passenger_Type*/
            STANDARD_FIELD_MAC(1),
/*            SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Interface_Reserved*/
/*    else, <Tunnel Start> == 0*/
/*,                SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Interface_ARP_Pointer*/
            {FIELD_SET_IN_RUNTIME_CNS,
             18,/* in sip 5 was 17 */
             SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Interface_Tunnel_Start},

/*,           SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Modify_MAC_DA*/
        STANDARD_FIELD_MAC(1),
/*,           SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Modify_MAC_SA*/
        STANDARD_FIELD_MAC(1),

/*else If <Redirect Command> = Redirect To Router (2)*/
/*,           SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_To_Router_Router_LTT_Index*/
        {FIELD_SET_IN_RUNTIME_CNS,
         18,/*in sip5 was 16*/
         SMEM_LION3_TTI_ACTION_TABLE_FIELDS_RedirectCmd},

/*        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect To Router    TTI reserved
        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect To Router    IPCL Configuration Table Pointer*/

/*else If <Redirect Command> = Assign VRF-ID (4)*/
/*,           SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Assign_VRF_ID_VRF_ID*/
        {FIELD_SET_IN_RUNTIME_CNS,
         12,
         SMEM_LION3_TTI_ACTION_TABLE_FIELDS_RedirectCmd},
/*        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Assign VRF-ID     Reserved
        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Assign VRF-ID     TTI reserved
        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Assign VRF-ID     IPCL Configuration Table Pointer*/

/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Bind_To_CNC_Counter*/
    {FIELD_SET_IN_RUNTIME_CNS,
     1,
     SMEM_LION3_TTI_ACTION_TABLE_FIELDS_No_Redirect_to_egress_interface_IPCL_Profile_Index},
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_CNC_Counter_Index*/
    STANDARD_FIELD_MAC(14),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Bind_To_Policer_Meter*/
    STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Bind_To_Policer_Counter*/
    STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Policer_Index*/
    STANDARD_FIELD_MAC(14),/* in sip 5 was 16 */
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_SourceID_Set_Enable*/
    STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Source_ID*/
    STANDARD_FIELD_MAC(12),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Action_Stop*/
    STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Bypass_Bridge*/
    STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Bypass_Ingress_Pipe*/
    STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_eVLAN_Precedence*/
    STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Enable_Nested_VLAN*/
    STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_eVID_Cmd*/
    STANDARD_FIELD_MAC(3),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_eVLAN*/
    STANDARD_FIELD_MAC(13),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_VID1_Cmd*/
    STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_VID1*/
    STANDARD_FIELD_MAC(12),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_QoS_Precedence*/
    STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_QoSProfile*/
    STANDARD_FIELD_MAC(7),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Modify_DSCP*/
    STANDARD_FIELD_MAC(2),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Modify_UP*/
    STANDARD_FIELD_MAC(2),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Keep_Previous_QoS*/
    STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Trust_UP*/
    STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Trust_DSCP*/
    STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Trust_EXP*/
    STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Map_DSCP*/
    STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_UP0*/
    STANDARD_FIELD_MAC(3),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_UP1_Command*/
    STANDARD_FIELD_MAC(2),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_UP1*/
    STANDARD_FIELD_MAC(3),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Passenger_Packet_Type*/
    STANDARD_FIELD_MAC(2),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Copy_TTL*/
    STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Tunnel_Termination*/
    STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_MPLS_Command*/
    STANDARD_FIELD_MAC(3),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Hash_Mask_Index*/
    STANDARD_FIELD_MAC(4),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Trust_QoS_Mapping_Table_Index*/
    STANDARD_FIELD_MAC(4),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_L_LSP_QoS_Profile_Enable*/
    STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_TTL*/
    STANDARD_FIELD_MAC(8),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Enable_Dec_TTL*/
    STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Source_ePort_Assignment_Enable*/
    STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Source_ePort*/
    STANDARD_FIELD_MAC(14),/* in sip 5 was 13 */
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_TT_Header_Length*/
    STANDARD_FIELD_MAC(6),/* in sip 5 was 5*/
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Is_PTP_Packet*/
    STANDARD_FIELD_MAC(1),

/*If <Is PTP PAcket> = 0*/
/*,           SMEM_LION3_TTI_ACTION_TABLE_FIELDS_NON_PTP_Packet_Timestamp_Enable*/
        {FIELD_SET_IN_RUNTIME_CNS,
         1,
         SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Is_PTP_Packet},
/*,           SMEM_LION3_TTI_ACTION_TABLE_FIELDS_NON_PTP_Packet_Offset_Index*/
        STANDARD_FIELD_MAC(7),
/*        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_    reserved*/
/*else If <Is PTP PAcket> =1*/
/*,           SMEM_LION3_TTI_ACTION_TABLE_FIELDS_PTP_Packet_PTP_Trigger_Type*/
        {FIELD_SET_IN_RUNTIME_CNS,
         2,
         SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Is_PTP_Packet},
/*,           SMEM_LION3_TTI_ACTION_TABLE_FIELDS_PTP_Packet_PTP_Offset*/
        STANDARD_FIELD_MAC(6),/*was 7 in sip 5*/

/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Enable_OAM_Processing_When_GAL_or_OAL_Exists*/
    {FIELD_SET_IN_RUNTIME_CNS,
     1,/*was 0 in sip 5*/
     SMEM_LION3_TTI_ACTION_TABLE_FIELDS_PTP_Packet_PTP_Offset},
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_OAM_Processing_Enable*/
     STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_CW_Based_Pseudo_Wire*/
     STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_TTL_Expiry_VCCV_Enable*/
     STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_PWE3_Flow_Label_Exist*/
     STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_PW_CW_Based_E_Tree_Enable*/
     STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_UP_CFI_to_QoS_table_select_mode*/
     STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Rx_Is_Protection_Path*/
     STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Rx_Enable_Protection_Switching*/
     STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Set_MAC2ME*/
     STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Oam_Profile*/
     STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Apply_non_Data_CW_Command*/
     STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_PW_Tag_Mode*/
     STANDARD_FIELD_MAC(2),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Enable_Next_Lookup*/
     STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Channel_Type_to_Opcode_Mapping_En*/
     STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Passenger_Parsing_of_Transit_MPLS_Tunnel_Enable*/
     STANDARD_FIELD_MAC(2),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Passenger_Parsing_of_Non_MPLS_Transit_tunnel_Enable*/
     STANDARD_FIELD_MAC(1),
/*,        SMEM_SIP6_TTI_ACTION_TABLE_FIELDS_Skip_Fdb_SA_Lookup*/
     STANDARD_FIELD_MAC(1),
/*,        SMEM_SIP6_TTI_ACTION_TABLE_FIELDS_Set_Ipv6_Segment_Routing_End_Node*/
     STANDARD_FIELD_MAC(1),
/*,        SMEM_SIP6_TTI_ACTION_TABLE_FIELDS_Tunnel_Header_Length_Anchor_Type*/
     STANDARD_FIELD_MAC(1)
};

static SNET_ENTRY_FORMAT_TABLE_STC sip6_10ttiActionTableFieldsFormat[SMEM_LION3_TTI_ACTION_TABLE_FIELDS___LAST_VALUE___E] =
{
/*        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Command*/
    STANDARD_FIELD_MAC(3),

/*        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_CPUCode*/
    STANDARD_FIELD_MAC(8),

/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_MirrorToAnalyzerPort*/
    STANDARD_FIELD_MAC(3),

/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_RedirectCmd*/
    STANDARD_FIELD_MAC(3),

/*If <Redirect Command> = No Redirect (0)*/
/*,           SMEM_LION3_TTI_ACTION_TABLE_FIELDS_No_Redirect_Flow_ID*/
        {FIELD_SET_IN_RUNTIME_CNS,
         16,/*in sip 6 was 13*/
         SMEM_LION3_TTI_ACTION_TABLE_FIELDS_RedirectCmd},

/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_No_Redirect_IPCL_UDB_Configuration_Table_UDE_Index*/
        STANDARD_FIELD_MAC(3),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_No_Redirect_reserved*/
        STANDARD_FIELD_MAC(0),
/*     <Redirect Command> != Egress_Interface (0,2,3..)*/
/*,           SMEM_LION3_TTI_ACTION_TABLE_FIELDS_No_Redirect_to_egress_interface_TTI_reserved_Enable*/
        {FIELD_SET_IN_RUNTIME_CNS,
             1,
             SMEM_LION3_TTI_ACTION_TABLE_FIELDS_No_Redirect_IPCL_UDB_Configuration_Table_UDE_Index},
/*,           SMEM_LION3_TTI_ACTION_TABLE_FIELDS_No_Redirect_to_egress_interface_TTI_reserved_Value*/
        STANDARD_FIELD_MAC(13),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_No_Redirect_to_egress_interface_Policy2_Lookup_Mode*/
        STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_No_Redirect_to_egress_interface_Policy1_Lookup_Mode*/
        STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_No_Redirect_to_egress_interface_Policy0_Lookup_Mode*/
        STANDARD_FIELD_MAC(1),
/*,           SMEM_LION3_TTI_ACTION_TABLE_FIELDS_No_Redirect_to_egress_interface_IPCL_Profile_Index*/
        STANDARD_FIELD_MAC(13),

/*else If <Redirect Command> = Redirect to Egress Interface (1)*/
/*    if <UseVIDX> = 0*/
/*,                SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Interface_IsTrunk*/
            {FIELD_SET_IN_RUNTIME_CNS,
             1,
             SMEM_LION3_TTI_ACTION_TABLE_FIELDS_RedirectCmd},
    /*    else if <UseVIDX> = 0 & <IsTrunk> = 1*/
/*,                SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Interface_TrunkNumber*/
                    {FIELD_SET_IN_RUNTIME_CNS,
                     12,
                     SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Interface_IsTrunk},
    /*            SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Interface_Reserved*/
    /*    else if <UseVIDX> = 0 & <IsTrunk> = 0*/
/*,                SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Interface_ePortNumber*/
                    {FIELD_SET_IN_RUNTIME_CNS,
                     14,/* in sip 5 was 13 */
                     SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Interface_IsTrunk},
/*,                SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Interface_DeviceNumber*/
                    STANDARD_FIELD_MAC(10),
/*    else, when <UseVIDX> = 1*/
/*,                SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Interface_eVIDX*/
            {FIELD_SET_IN_RUNTIME_CNS,
             16,
             SMEM_LION3_TTI_ACTION_TABLE_FIELDS_RedirectCmd},
/*,               SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Interface_Reserved*/
            STANDARD_FIELD_MAC(9),
/*,            SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Interface_UseEVIDX*/
        {FIELD_SET_IN_RUNTIME_CNS,
         1,
         SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Interface_DeviceNumber},
/*,            SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Interface_VNT_L2_Echo*/
        STANDARD_FIELD_MAC(1),

/*    Tunnel Start*/
/*,            SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Interface_Tunnel_Start*/
        STANDARD_FIELD_MAC(1),
/*    if (<Tunnel Start> == 1)*/
/*,                SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Interface_Tunnel_Index*/
            STANDARD_FIELD_MAC(16),/* in sip 5 was 15 */
/*,                SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Interface_Tunnel_Start_Passenger_Type*/
            STANDARD_FIELD_MAC(1),
/*            SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Interface_Reserved*/
/*    else, <Tunnel Start> == 0*/
/*,                SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Interface_ARP_Pointer*/
            {FIELD_SET_IN_RUNTIME_CNS,
             18,/* in sip 5 was 17 */
             SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Interface_Tunnel_Start},

/*,           SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Modify_MAC_DA*/
        STANDARD_FIELD_MAC(1),
/*,           SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Modify_MAC_SA*/
        STANDARD_FIELD_MAC(1),

/*else If <Redirect Command> = Redirect To Router (2)*/
/*,           SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_To_Router_Router_LTT_Index*/
        {FIELD_SET_IN_RUNTIME_CNS,
         18,/*in sip5 was 16*/
         SMEM_LION3_TTI_ACTION_TABLE_FIELDS_RedirectCmd},

/*        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect To Router    TTI reserved
        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect To Router    IPCL Configuration Table Pointer*/

/*else If <Redirect Command> = Assign VRF-ID (4)*/
/*,           SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Assign_VRF_ID_VRF_ID
 * else If <Redirect Command> = Assign Generic Action (5)
 *,           SMEM_SIP6_30_TTI_ACTION_TABLE_FIELDS_Generic_Action */
        {FIELD_SET_IN_RUNTIME_CNS,
         12,
         SMEM_LION3_TTI_ACTION_TABLE_FIELDS_RedirectCmd},
/*        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Assign VRF-ID     Reserved
        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Assign VRF-ID     TTI reserved
        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Assign VRF-ID     IPCL Configuration Table Pointer*/


/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Bind_To_CNC_Counter*/
        EXPLICIT_FIELD_MAC(80,1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_CNC_Counter_Index*/
         {FIELD_SET_IN_RUNTIME_CNS,
         15,/* in sip6 was 14 */
         SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Bind_To_CNC_Counter},
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Bind_To_Policer_Meter*/
    STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Bind_To_Policer_Counter*/
    STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Policer_Index*/
    STANDARD_FIELD_MAC(14),/* in sip 5 was 16 */
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_SourceID_Set_Enable*/
         {FIELD_SET_IN_RUNTIME_CNS,
         1,
         SMEM_LION3_TTI_ACTION_TABLE_FIELDS_No_Redirect_to_egress_interface_IPCL_Profile_Index},
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Source_ID*/
    STANDARD_FIELD_MAC(12),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Action_Stop*/
    EXPLICIT_FIELD_MAC(246,1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Bypass_Bridge*/
       {FIELD_SET_IN_RUNTIME_CNS,
         1,
         SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Source_ID},
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Bypass_Ingress_Pipe*/
    EXPLICIT_FIELD_MAC(248,1),

/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_eVLAN_Precedence*/
       {FIELD_SET_IN_RUNTIME_CNS,
         1,
         SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Policer_Index},

/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Enable_Nested_VLAN*/
    STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_eVID_Cmd*/
    STANDARD_FIELD_MAC(3),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_eVLAN*/
    STANDARD_FIELD_MAC(13),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_VID1_Cmd*/
    STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_VID1*/
    STANDARD_FIELD_MAC(12),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_QoS_Precedence*/
    STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_QoSProfile*/
    STANDARD_FIELD_MAC(7),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Modify_DSCP*/
    STANDARD_FIELD_MAC(2),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Modify_UP*/
    STANDARD_FIELD_MAC(2),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Keep_Previous_QoS*/
    STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Trust_UP*/
    STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Trust_DSCP*/
    STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Trust_EXP*/
    STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Map_DSCP*/
    STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_UP0*/
    STANDARD_FIELD_MAC(3),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_UP1_Command*/
    STANDARD_FIELD_MAC(2),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_UP1*/
    STANDARD_FIELD_MAC(3),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Passenger_Packet_Type*/
    STANDARD_FIELD_MAC(2),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Copy_TTL*/
    STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Tunnel_Termination*/
    STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_MPLS_Command*/
    STANDARD_FIELD_MAC(3),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Hash_Mask_Index*/
    STANDARD_FIELD_MAC(4),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Trust_QoS_Mapping_Table_Index*/
    STANDARD_FIELD_MAC(4),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_L_LSP_QoS_Profile_Enable*/
    STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_TTL*/
    STANDARD_FIELD_MAC(8),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Enable_Dec_TTL*/
    STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Source_ePort_Assignment_Enable*/
    STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Source_ePort*/
    STANDARD_FIELD_MAC(14),/* in sip 5 was 13 */
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_TT_Header_Length*/
    STANDARD_FIELD_MAC(6),/* in sip 5 was 5*/
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Is_PTP_Packet*/
    STANDARD_FIELD_MAC(1),

/*If <Is PTP PAcket> = 0*/
/*,           SMEM_LION3_TTI_ACTION_TABLE_FIELDS_NON_PTP_Packet_Timestamp_Enable*/
        {FIELD_SET_IN_RUNTIME_CNS,
         1,
         SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Is_PTP_Packet},
/*,           SMEM_LION3_TTI_ACTION_TABLE_FIELDS_NON_PTP_Packet_Offset_Index*/
        STANDARD_FIELD_MAC(7),
/*        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_    reserved*/
/*else If <Is PTP PAcket> =1*/
/*,           SMEM_LION3_TTI_ACTION_TABLE_FIELDS_PTP_Packet_PTP_Trigger_Type*/
        {FIELD_SET_IN_RUNTIME_CNS,
         2,
         SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Is_PTP_Packet},
/*,           SMEM_LION3_TTI_ACTION_TABLE_FIELDS_PTP_Packet_PTP_Offset*/
        STANDARD_FIELD_MAC(6),/*was 7 in sip 5*/

/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Enable_OAM_Processing_When_GAL_or_OAL_Exists*/
    {FIELD_SET_IN_RUNTIME_CNS,
     1,/*was 0 in sip 5*/
     SMEM_LION3_TTI_ACTION_TABLE_FIELDS_PTP_Packet_PTP_Offset},
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_OAM_Processing_Enable*/
     STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_CW_Based_Pseudo_Wire*/
     STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_TTL_Expiry_VCCV_Enable*/
     STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_PWE3_Flow_Label_Exist*/
     STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_PW_CW_Based_E_Tree_Enable*/
     STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_UP_CFI_to_QoS_table_select_mode*/
     STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Rx_Is_Protection_Path*/
     STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Rx_Enable_Protection_Switching*/
     STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Set_MAC2ME*/
     STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Oam_Profile*/
     STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Apply_non_Data_CW_Command*/
     STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_PW_Tag_Mode*/
     STANDARD_FIELD_MAC(2),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Enable_Next_Lookup*/
     STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Channel_Type_to_Opcode_Mapping_En*/
     STANDARD_FIELD_MAC(1),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Passenger_Parsing_of_Transit_MPLS_Tunnel_Enable*/
     STANDARD_FIELD_MAC(2),
/*,        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Passenger_Parsing_of_Non_MPLS_Transit_tunnel_Enable*/
     STANDARD_FIELD_MAC(1),
/*,        SMEM_SIP6_TTI_ACTION_TABLE_FIELDS_Skip_Fdb_SA_Lookup*/
     STANDARD_FIELD_MAC(1),
/*,        SMEM_SIP6_TTI_ACTION_TABLE_FIELDS_Set_Ipv6_Segment_Routing_End_Node*/
     STANDARD_FIELD_MAC(1),
/*,        SMEM_SIP6_TTI_ACTION_TABLE_FIELDS_Tunnel_Header_Length_Anchor_Type*/
     STANDARD_FIELD_MAC(2),
/*,        SMEM_SIP6_10_TTI_ACTION_TABLE_FIELDS_Triger_Cnc_Hash_Client*/
        {FIELD_SET_IN_RUNTIME_CNS,
         1,
         SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Action_Stop},
/*,        SMEM_SIP6_10_TTI_ACTION_TABLE_FIELDS_Ipfix_enable*/
        {FIELD_SET_IN_RUNTIME_CNS,
         1,
         SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Bypass_Ingress_Pipe},
/*,        SMEM_SIP6_10_TTI_ACTION_TABLE_FIELDS_Ppu_Profile*/
     STANDARD_FIELD_MAC(4)
/*SMEM_SIP6_30_TTI_ACTION_TABLE_FIELDS_Group_IDs_Assignment_Enable                            */
   ,EXPLICIT_FIELD_MAC(36,1)  /*tti_reserved_assign_en*/
/*SMEM_SIP6_30_TTI_ACTION_TABLE_FIELDS_Group_IDs                                              */
   ,EXPLICIT_FIELD_MAC(37,13) /*tti_to_txq_reserved*/
/*SMEM_SIP6_30_TTI_ACTION_TABLE_FIELDS_Generic_12b_Action                                     */
   ,EXPLICIT_FIELD_MAC(17,12)
};

static SNET_ENTRY_FORMAT_TABLE_STC sip7_ttiActionTableFieldsFormat[SMEM_LION3_TTI_ACTION_TABLE_FIELDS___LAST_VALUE___E] =
{
/*SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Command                                                  */
    EXPLICIT_FIELD_MAC(8,3)
/*SMEM_LION3_TTI_ACTION_TABLE_FIELDS_CPUCode                                                  */
   ,EXPLICIT_FIELD_MAC(0,8)
/*SMEM_LION3_TTI_ACTION_TABLE_FIELDS_MirrorToAnalyzerPort                                     */
   ,EXPLICIT_FIELD_MAC(96,8)
/*SMEM_LION3_TTI_ACTION_TABLE_FIELDS_RedirectCmd                                              */
   ,EXPLICIT_FIELD_MAC(11,3)
/*SMEM_LION3_TTI_ACTION_TABLE_FIELDS_No_Redirect_Flow_ID                                      */
   ,EXPLICIT_FIELD_MAC(16,19)
/*SMEM_LION3_TTI_ACTION_TABLE_FIELDS_No_Redirect_IPCL_UDB_Configuration_Table_UDE_Index       */
   ,EXPLICIT_FIELD_MAC(35,3)
/*SMEM_LION3_TTI_ACTION_TABLE_FIELDS_No_Redirect_reserved                                     */
   ,EXPLICIT_FIELD_MAC(0,0)
/*SMEM_LION3_TTI_ACTION_TABLE_FIELDS_No_Redirect_to_egress_interface_TTI_reserved_Enable      */
   ,EXPLICIT_FIELD_MAC(38,1) /* got new name : SMEM_SIP6_30_TTI_ACTION_TABLE_FIELDS_Group_IDs_Assignment_Enable */
/*SMEM_LION3_TTI_ACTION_TABLE_FIELDS_No_Redirect_to_egress_interface_TTI_reserved_Value       */
   ,EXPLICIT_FIELD_MAC(40,1) /* got new name : SMEM_SIP6_30_TTI_ACTION_TABLE_FIELDS_Group_IDs */
/*SMEM_LION3_TTI_ACTION_TABLE_FIELDS_No_Redirect_to_egress_interface_Policy2_Lookup_Mode      */
   ,EXPLICIT_FIELD_MAC(61,1)
/*SMEM_LION3_TTI_ACTION_TABLE_FIELDS_No_Redirect_to_egress_interface_Policy1_Lookup_Mode      */
   ,EXPLICIT_FIELD_MAC(62,1)
/*SMEM_LION3_TTI_ACTION_TABLE_FIELDS_No_Redirect_to_egress_interface_Policy0_Lookup_Mode      */
   ,EXPLICIT_FIELD_MAC(55,1)/*mpcl_lookup_mode*/
/*SMEM_LION3_TTI_ACTION_TABLE_FIELDS_No_Redirect_to_egress_interface_IPCL_Profile_Index       */
   ,EXPLICIT_FIELD_MAC(48,13)
/*SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Interface_IsTrunk                     */
   ,EXPLICIT_FIELD_MAC(42,1)
/*SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Interface_TrunkNumber                 */
   ,EXPLICIT_FIELD_MAC(16,12)
/*SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Interface_ePortNumber                 */
   ,EXPLICIT_FIELD_MAC(16,15)
/*SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Interface_DeviceNumber                */
   ,EXPLICIT_FIELD_MAC(32,10)
/*SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Interface_eVIDX                       */
   ,EXPLICIT_FIELD_MAC(16,17)
/*SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Interface_Reserved                    */
   ,EXPLICIT_FIELD_MAC(0,0)
/*SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Interface_UseEVIDX                    */
   ,EXPLICIT_FIELD_MAC(43,1)
/*SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Interface_VNT_L2_Echo                 */
   ,EXPLICIT_FIELD_MAC(44,1)
/*SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Interface_Tunnel_Start                */
   ,EXPLICIT_FIELD_MAC(45,1)
/*SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Interface_Tunnel_Index                */
   ,EXPLICIT_FIELD_MAC(48,19)
/*SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Interface_Tunnel_Start_Passenger_Type */
   ,EXPLICIT_FIELD_MAC(67,1)
/*SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Interface_ARP_Pointer                 */
   ,EXPLICIT_FIELD_MAC(48,20)
/*SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Modify_MAC_DA                         */
   ,EXPLICIT_FIELD_MAC(68,1)
/*SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Modify_MAC_SA                         */
   ,EXPLICIT_FIELD_MAC(47,1)
/*SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_To_Router_Router_LTT_Index                      */
   ,EXPLICIT_FIELD_MAC(16,21)/*ip_next_hop_entry_index*/
/*SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Assign_VRF_ID_VRF_ID                                     */
   ,EXPLICIT_FIELD_MAC(16,15)
/*SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Bind_To_CNC_Counter                                      */
   ,EXPLICIT_FIELD_MAC(140,1)
/*SMEM_LION3_TTI_ACTION_TABLE_FIELDS_CNC_Counter_Index                                        */
   ,EXPLICIT_FIELD_MAC(120,20)
/*SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Bind_To_Policer_Meter                                    */
   ,EXPLICIT_FIELD_MAC(89,1)
/*SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Bind_To_Policer_Counter                                  */
   ,EXPLICIT_FIELD_MAC(90,1)
/*SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Policer_Index                                            */
   ,EXPLICIT_FIELD_MAC(72,17)
/*SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Source ID_Set_Enable                                      */
   ,EXPLICIT_FIELD_MAC(116,1)
/*SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Source_ID                                                */
   ,EXPLICIT_FIELD_MAC(104,12)
/*SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Action_Stop                                              */
   ,EXPLICIT_FIELD_MAC(14,1)
/*SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Bypass_Bridge                                            */
   ,EXPLICIT_FIELD_MAC(15,1)
/*SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Bypass_Ingress_Pipe                                      */
   ,EXPLICIT_FIELD_MAC(275,1)
/*SMEM_LION3_TTI_ACTION_TABLE_FIELDS_eVLAN_Precedence                                         */
   ,EXPLICIT_FIELD_MAC(141,1)
/*SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Enable_Nested_VLAN                                       */
   ,EXPLICIT_FIELD_MAC(316,1)
/*SMEM_LION3_TTI_ACTION_TABLE_FIELDS_eVID_Cmd                                                 */
   ,EXPLICIT_FIELD_MAC(117,3)
/*SMEM_LION3_TTI_ACTION_TABLE_FIELDS_eVLAN                                                    */
   ,EXPLICIT_FIELD_MAC(144,16)
/*SMEM_LION3_TTI_ACTION_TABLE_FIELDS_VID1_Cmd                                                 */
   ,EXPLICIT_FIELD_MAC(142,1)
/*SMEM_LION3_TTI_ACTION_TABLE_FIELDS_VID1                                                     */
   ,EXPLICIT_FIELD_MAC(176,12)
/*SMEM_LION3_TTI_ACTION_TABLE_FIELDS_QoS_Precedence                                           */
   ,EXPLICIT_FIELD_MAC(167,1)
/*SMEM_LION3_TTI_ACTION_TABLE_FIELDS_QoSProfile                                               */
   ,EXPLICIT_FIELD_MAC(160,7)
/*SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Modify_DSCP                                              */
   ,EXPLICIT_FIELD_MAC(172,2)
/*SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Modify_UP                                                */
   ,EXPLICIT_FIELD_MAC(174,2)
/*SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Keep_Previous_QoS                                        */
   ,EXPLICIT_FIELD_MAC(191,1)
/*SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Trust_UP                                                 */
   ,EXPLICIT_FIELD_MAC(312,2)
/*SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Trust_DSCP                                               */
   ,EXPLICIT_FIELD_MAC(314,1)
/*SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Trust_EXP                                                */
   ,EXPLICIT_FIELD_MAC(315,1)
/*SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Map_DSCP                                                 */
   ,EXPLICIT_FIELD_MAC(171,1)  /*en_dscp_to_dscp_remapping*/
/*SMEM_LION3_TTI_ACTION_TABLE_FIELDS_UP0                                                      */
   ,EXPLICIT_FIELD_MAC(69,3)
/*SMEM_LION3_TTI_ACTION_TABLE_FIELDS_UP1_Command                                              */
   ,EXPLICIT_FIELD_MAC(273,2)
/*SMEM_LION3_TTI_ACTION_TABLE_FIELDS_UP1                                                      */
   ,EXPLICIT_FIELD_MAC(188,3)
/*SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Passenger_Packet_Type                                    */
   ,EXPLICIT_FIELD_MAC(208,2) /*tt_passenger_packet_type*/
/*SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Copy_TTL                                                 */
   ,EXPLICIT_FIELD_MAC(264,1) /* !!! obsolete in AAS !!! copy_ttl_from_outer_header */
/*SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Tunnel_Termination                                       */
   ,EXPLICIT_FIELD_MAC(215,1)
/*SMEM_LION3_TTI_ACTION_TABLE_FIELDS_MPLS_Command                                             */
   ,EXPLICIT_FIELD_MAC(320,3)
/*SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Hash_Mask_Index                                          */
   ,EXPLICIT_FIELD_MAC(276,4)
/*SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Trust_QoS_Mapping_Table_Index                            */
   ,EXPLICIT_FIELD_MAC(232,7)
/*SMEM_LION3_TTI_ACTION_TABLE_FIELDS_L_LSP_QoS_Profile_Enable                                 */
   ,EXPLICIT_FIELD_MAC(269,1)
/*SMEM_LION3_TTI_ACTION_TABLE_FIELDS_TTL                                                      */
   ,EXPLICIT_FIELD_MAC(192,8) /*MPLS TTL*/
/*SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Enable_Dec_TTL                                           */
   ,EXPLICIT_FIELD_MAC(143,1)
/*SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Source_ePort_Assignment_Enable                           */
   ,EXPLICIT_FIELD_MAC(0,0)/* NOT USED .. see Source_Interface_Assignment_Enable */
/*SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Source_ePort                                             */
   ,EXPLICIT_FIELD_MAC(248,15)
/*SMEM_LION3_TTI_ACTION_TABLE_FIELDS_TT_Header_Length                                         */
   ,EXPLICIT_FIELD_MAC(200,7)
/*SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Is_PTP_Packet                                            */
   ,EXPLICIT_FIELD_MAC(168,1)
/*SMEM_LION3_TTI_ACTION_TABLE_FIELDS_NON_PTP_Packet_Timestamp_Enable                          */
   ,EXPLICIT_FIELD_MAC(223,1)
/*SMEM_LION3_TTI_ACTION_TABLE_FIELDS_NON_PTP_Packet_Offset_Index                              */
   ,EXPLICIT_FIELD_MAC(216,7) /* OAM PTP Offset Index*/
/*SMEM_LION3_TTI_ACTION_TABLE_FIELDS_PTP_Packet_PTP_Trigger_Type                              */
   ,EXPLICIT_FIELD_MAC(169,2)
/*SMEM_LION3_TTI_ACTION_TABLE_FIELDS_PTP_Packet_PTP_Offset                                    */
   ,EXPLICIT_FIELD_MAC(216,7) /*PTP Header Offset*/
/*SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Enable_OAM_Processing_When_GAL_or_OAL_Exists             */
   ,EXPLICIT_FIELD_MAC(224,1)
/*SMEM_LION3_TTI_ACTION_TABLE_FIELDS_OAM_Processing_Enable                                    */
   ,EXPLICIT_FIELD_MAC(225,1)
/*SMEM_LION3_TTI_ACTION_TABLE_FIELDS_CW_Based_Pseudo_Wire                                     */
   ,EXPLICIT_FIELD_MAC(226,1)
/*SMEM_LION3_TTI_ACTION_TABLE_FIELDS_TTL_Expiry_VCCV_Enable                                   */
   ,EXPLICIT_FIELD_MAC(227,1)
/*SMEM_LION3_TTI_ACTION_TABLE_FIELDS_PWE3_Flow_Label_Exist                                    */
   ,EXPLICIT_FIELD_MAC(228,1)/*MPLS Flow Label Exist*/
/*SMEM_LION3_TTI_ACTION_TABLE_FIELDS_PW_CW_Based_E_Tree_Enable                                */
   ,EXPLICIT_FIELD_MAC(229,1)
/*SMEM_LION3_TTI_ACTION_TABLE_FIELDS_UP_CFI_to_QoS_table_select_mode                          */
   ,EXPLICIT_FIELD_MAC(230,1)   /*L2 Trust Table Selection Mode*/
/*SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Rx_Is_Protection_Path                                    */
   ,EXPLICIT_FIELD_MAC(231,1)
/*SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Rx_Enable_Protection_Switching                           */
   ,EXPLICIT_FIELD_MAC(240,1)
/*SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Set_MAC2ME                                               */
   ,EXPLICIT_FIELD_MAC(241,1)
/*SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Oam_Profile                                              */
   ,EXPLICIT_FIELD_MAC(242,1)
/*SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Apply_non_Data_CW_Command                                */
   ,EXPLICIT_FIELD_MAC(243,1)
/*SMEM_LION3_TTI_ACTION_TABLE_FIELDS_PW_Tag_Mode                                              */
   ,EXPLICIT_FIELD_MAC(246,0) /* obsolete in sip 5 */
/*SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Enable_Next_Lookup                                       */
   ,EXPLICIT_FIELD_MAC(271,1)/*continue_to_next_tti_lookup*/
/*SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Channel_Type_to_Opcode_Mapping_En                        */
   ,EXPLICIT_FIELD_MAC(270,1)
/*SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Passenger_Parsing_of_Transit_MPLS_Tunnel_Enable          */
   ,EXPLICIT_FIELD_MAC(210,2) /* was 1 bit in legacy */
/*SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Passenger_Parsing_of_Non_MPLS_Transit_tunnel_Enable      */
   ,EXPLICIT_FIELD_MAC(214,1)
/*SMEM_SIP6_TTI_ACTION_TABLE_FIELDS_Skip_Fdb_SA_Lookup                                        */
   ,EXPLICIT_FIELD_MAC(268,1)
/*SMEM_SIP6_TTI_ACTION_TABLE_FIELDS_Set_Ipv6_Segment_Routing_End_Node                         */
   ,EXPLICIT_FIELD_MAC(265,2)
/*SMEM_SIP6_TTI_ACTION_TABLE_FIELDS_Tunnel_Header_Length_Anchor_Type                          */
   ,EXPLICIT_FIELD_MAC(212,2)
/*SMEM_SIP6_10_TTI_ACTION_TABLE_FIELDS_Triger_Cnc_Hash_Client                                 */
   ,EXPLICIT_FIELD_MAC(91,1)
/*SMEM_SIP6_10_TTI_ACTION_TABLE_FIELDS_Ipfix_enable                                           */
   ,EXPLICIT_FIELD_MAC(207,1)
/*SMEM_SIP6_10_TTI_ACTION_TABLE_FIELDS_Ppu_Profile                                            */
   ,EXPLICIT_FIELD_MAC(92,4)
/*SMEM_SIP6_30_TTI_ACTION_TABLE_FIELDS_Group_IDs_Assignment_Enable                            */
   ,EXPLICIT_FIELD_MAC(38,1)  /*tti_reserved_assign_en*/
/*SMEM_SIP6_30_TTI_ACTION_TABLE_FIELDS_Group_IDs                                              */
   ,EXPLICIT_FIELD_MAC(40,1) /*tti_to_txq_reserved*/
/*SMEM_SIP6_30_TTI_ACTION_TABLE_FIELDS_Generic_12b_Action                                     */
   ,EXPLICIT_FIELD_MAC(16,12)
/*SMEM_SIP7_TTI_ACTION_TABLE_FIELDS_trg_vport                                                 */
   ,EXPLICIT_FIELD_MAC(16,18)
/*SMEM_SIP7_TTI_ACTION_TABLE_FIELDS_use_trg_vport                                             */
   ,EXPLICIT_FIELD_MAC(46,1)
/*SMEM_SIP7_TTI_ACTION_TABLE_FIELDS_stop_mpls                                                 */
   ,EXPLICIT_FIELD_MAC(239,1)
/*SMEM_SIP7_TTI_ACTION_TABLE_FIELDS_srv6_psp_enabled                                          */
   ,EXPLICIT_FIELD_MAC(267,1)
/*SMEM_SIP7_TTI_ACTION_TABLE_FIELDS_qcn_dlb_proc_en                                           */
   ,EXPLICIT_FIELD_MAC(272,1)
/*SMEM_SIP7_TTI_ACTION_TABLE_FIELDS_generic_classification_value                              */
   ,EXPLICIT_FIELD_MAC(280,32)
/*SMEM_SIP7_TTI_ACTION_TABLE_FIELDS_qos_mapping_mode                                          */
   ,EXPLICIT_FIELD_MAC(317,1)
/*SMEM_SIP7_TTI_ACTION_TABLE_FIELDS_force_dp                                                  */
   ,EXPLICIT_FIELD_MAC(318,2)
/*SMEM_SIP7_TTI_ACTION_TABLE_FIELDS_qos_model                                                 */
   ,EXPLICIT_FIELD_MAC(323,3)
/*SMEM_SIP7_TTI_ACTION_TABLE_FIELDS_ttl_model                                                 */
   ,EXPLICIT_FIELD_MAC(326,2)
/*SMEM_SIP7_TTI_ACTION_TABLE_FIELDS_source_vport                                              */
   ,EXPLICIT_FIELD_MAC(328,18)
/*SMEM_SIP7_TTI_ACTION_TABLE_FIELDS_Source_Interface_Assignment_Enable                            */
   ,EXPLICIT_FIELD_MAC(346,2)
/*SMEM_SIP7_TTI_ACTION_TABLE_FIELDS_source_Vport_profile_assignment_en                        */
   ,EXPLICIT_FIELD_MAC(348,1)
/*SMEM_SIP7_TTI_ACTION_TABLE_FIELDS_disable_em_lookup                                         */
   ,EXPLICIT_FIELD_MAC(271,1)
/*SMEM_SIP7_TTI_ACTION_TABLE_FIELDS_cancel_drop                                               */
   ,EXPLICIT_FIELD_MAC(264,1)
/*SMEM_SIP7_TTI_ACTION_TABLE_FIELDS_No_Redirect_to_egress_interface_Mpcl_Lookup_Mode          */
   ,EXPLICIT_FIELD_MAC(63,1)
};


/**
* @internal snetTtiTablesFormatInit function
* @endinternal
*
* @brief   init the format of TTI tables.
*
* @param[in] devObjPtr                - pointer to device object.
*/
void snetTtiTablesFormatInit(
    IN SKERNEL_DEVICE_OBJECT            * devObjPtr
)
{
    if (SMEM_CHT_IS_SIP7_GET(devObjPtr))
    {
            LION3_TABLES_FORMAT_INIT_MAC(
                devObjPtr, SKERNEL_TABLE_FORMAT_TTI_ACTION_E,
                sip7_ttiActionTableFieldsFormat, lion3TtiActionTableFieldsNames);
    }
    else
    if (SMEM_CHT_IS_SIP6_10_GET(devObjPtr))
    {
            LION3_TABLES_FORMAT_INIT_MAC(
                devObjPtr, SKERNEL_TABLE_FORMAT_TTI_ACTION_E,
                sip6_10ttiActionTableFieldsFormat, lion3TtiActionTableFieldsNames);
    }
    else
    {
        if (SMEM_CHT_IS_SIP6_GET(devObjPtr))
        {
                LION3_TABLES_FORMAT_INIT_MAC(
                    devObjPtr, SKERNEL_TABLE_FORMAT_TTI_ACTION_E,
                    sip6_ttiActionTableFieldsFormat, lion3TtiActionTableFieldsNames);
        }
        else
        {
            if (SMEM_CHT_IS_SIP5_20_GET(devObjPtr))
            {
                LION3_TABLES_FORMAT_INIT_MAC(
                    devObjPtr, SKERNEL_TABLE_FORMAT_TTI_ACTION_E,
                    sip5_20TtiActionTableFieldsFormat, lion3TtiActionTableFieldsNames);

            }
            else
            {
                if(SMEM_CHT_IS_SIP5_10_GET(devObjPtr))
                {
                    LION3_TABLES_FORMAT_INIT_MAC(
                        devObjPtr, SKERNEL_TABLE_FORMAT_TTI_ACTION_E,
                        sip5_10TtiActionTableFieldsFormat, lion3TtiActionTableFieldsNames);
                }
                else
                {
                    LION3_TABLES_FORMAT_INIT_MAC(
                        devObjPtr, SKERNEL_TABLE_FORMAT_TTI_ACTION_E,
                        lion3TtiActionTableFieldsFormat, lion3TtiActionTableFieldsNames);
                }
            }

        }
    }
    LION3_TABLES_FORMAT_INIT_MAC(
        devObjPtr, SKERNEL_TABLE_FORMAT_TTI_META_DATA_E,
        lion3TtiMetaDataFieldsFormat, lion3TtiMetaDataFieldsNames);

}

/**
* @internal snetLion3TTActionGet function
* @endinternal
*
* @brief   Sip5 devices : Get the action entry from the TTI action table.
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to frame descriptor.
* @param[in] matchIndex               - index to the action
*                                       table
* @param[in] isMatchDoneInTcamOrEm    - match done in TCAM table
* @param[in] ttiKeyType               - TTI Key Type
* @param[in] lookupNum                - TTI lookup number
* @param[out]actionDataPtr            - (pointer to) action data
*/
static GT_VOID snetLion3TTActionGet
(
    IN SKERNEL_DEVICE_OBJECT            *devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC  *descrPtr,
    IN GT_U32                            matchIndex,
    IN SIP7_EXACT_MATCH_UNIT_ENT         emUnitNum,
    IN GT_BOOL                           isMatchDoneInTcamOrEm,
    IN SKERNEL_CHT3_TTI_KEY_TYPE_ENT     ttiKeyType,
    IN GT_U32                            lookupNum,
    IN GT_U32                            serialEmProfileId,
    OUT SNET_XCAT_TT_ACTION_STC         *actionDataPtr
)
{
    DECLARE_FUNC_NAME(snetLion3TTActionGet);

    GT_U32 enable;
    GT_U32  *actionEntryDataPtr;
    GT_U32  indexBaseAddress;
    SNET_LION_TT_ACTION_STC * lionActionDataPtr;
    SNET_E_ARCH_TT_ACTION_STC   * actionDataEArchPtr;/*E-Arch action info*/
    GT_U32  tempValue;
    GT_U32  genericActionMode;

    GT_U32  exactMatchProfileIdTableIndex=0;
    GT_U32  exactMatchActionDataArr[SIP7_EXACT_MATCH_FULL_ACTION_SIZE_WORD_CNS];/* 384 bits of data */

    actionDataEArchPtr = (SNET_E_ARCH_TT_ACTION_STC*)(void*)actionDataPtr;
    lionActionDataPtr = &actionDataEArchPtr->baseActionData;

    if (SMEM_CHT_IS_SIP7_GET(devObjPtr) && (serialEmProfileId != 0))
    {
        __LOG(("Serial EM profile ID is non-zero\n"));
        __LOG(("Get the Expanded Action for matchIndex=[%u], serialEmProfileId=[%u] in EM unit=[%u]\n",
              matchIndex, serialEmProfileId, emUnitNum));
        snetAasExactMatchSerialEmActionGet(devObjPtr, descrPtr, matchIndex, emUnitNum, lookupNum,
                                           serialEmProfileId, &exactMatchActionDataArr[0]);
        actionEntryDataPtr = &exactMatchActionDataArr[0];
    }
    else if(isMatchDoneInTcamOrEm==GT_TRUE)/* TCAM has priority */
    {
        indexBaseAddress = SMEM_LION3_TCAM_ACTION_TBL_MEM(devObjPtr, matchIndex);
        actionEntryDataPtr = smemMemGet(devObjPtr,indexBaseAddress);
    }
    else/* EM has priority over TCAM */
    {
        snetFalconTtiGetIndexForKeyType(ttiKeyType,&exactMatchProfileIdTableIndex);
        if (SMEM_CHT_IS_SIP7_GET(devObjPtr))
        {
            snetAasExactMatchActionGet(devObjPtr, descrPtr, matchIndex, SIP7_EMX_CLIENT_TTI_E,
                                       lookupNum, exactMatchProfileIdTableIndex,
                                       &exactMatchActionDataArr[0]);
        }
        else
        {
            snetFalconExactMatchActionGet(devObjPtr, descrPtr, matchIndex, SIP5_TCAM_CLIENT_TTI_E,
                                          exactMatchProfileIdTableIndex, &exactMatchActionDataArr[0]);
        }
        actionEntryDataPtr = &exactMatchActionDataArr[0];
    }

    actionDataPtr->packetCmd =
        SMEM_LION3_TTI_ACTION_ENTRY_FIELD_GET(devObjPtr,actionEntryDataPtr,
            matchIndex,
            SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Command);

    actionDataPtr->userDefinedCpuCode =
        SMEM_LION3_TTI_ACTION_ENTRY_FIELD_GET(devObjPtr,actionEntryDataPtr,
            matchIndex,
            SMEM_LION3_TTI_ACTION_TABLE_FIELDS_CPUCode);


    /* Mirror To Analyzer Index */
    actionDataEArchPtr->analyzerIndex =
        SMEM_LION3_TTI_ACTION_ENTRY_FIELD_GET(devObjPtr,actionEntryDataPtr,
            matchIndex,
            SMEM_LION3_TTI_ACTION_TABLE_FIELDS_MirrorToAnalyzerPort);

    actionDataPtr->mirrorToAnalyzerPort = (actionDataEArchPtr-> analyzerIndex) ? GT_TRUE : GT_FALSE;


    actionDataPtr->redirectCmd =
        SMEM_LION3_TTI_ACTION_ENTRY_FIELD_GET(devObjPtr,actionEntryDataPtr,
            matchIndex,
            SMEM_LION3_TTI_ACTION_TABLE_FIELDS_RedirectCmd);

    if(actionDataPtr->redirectCmd != PCL_TTI_ACTION_REDIRECT_CMD_OUT_IF_E)
    {
        if(SMEM_CHT_IS_SIP6_30_GET(devObjPtr))
        {
            actionDataEArchPtr->ttiReservedAssignEnable =
                SMEM_LION3_TTI_ACTION_ENTRY_FIELD_GET(devObjPtr,actionEntryDataPtr,
                    matchIndex,
                    SMEM_SIP6_30_TTI_ACTION_TABLE_FIELDS_Group_IDs_Assignment_Enable);

            actionDataEArchPtr->ttiReservedAssignValue =
                SMEM_LION3_TTI_ACTION_ENTRY_FIELD_GET(devObjPtr,actionEntryDataPtr,
                    matchIndex,
                    SMEM_SIP6_30_TTI_ACTION_TABLE_FIELDS_Group_IDs);
        }
        else
        {
            actionDataEArchPtr->ttiReservedAssignEnable =
                SMEM_LION3_TTI_ACTION_ENTRY_FIELD_GET(devObjPtr,actionEntryDataPtr,
                    matchIndex,
                    SMEM_LION3_TTI_ACTION_TABLE_FIELDS_No_Redirect_to_egress_interface_TTI_reserved_Enable);

            actionDataEArchPtr->ttiReservedAssignValue =
                SMEM_LION3_TTI_ACTION_ENTRY_FIELD_GET(devObjPtr,actionEntryDataPtr,
                    matchIndex,
                    SMEM_LION3_TTI_ACTION_TABLE_FIELDS_No_Redirect_to_egress_interface_TTI_reserved_Value);
        }

        if( (SMEM_CHT_IS_SIP6_GET(devObjPtr)) && (!SMEM_CHT_IS_SIP6_10_GET(devObjPtr))  )
        {
            actionDataEArchPtr->triggerCncHashClient =
                SMEM_LION3_TTI_ACTION_ENTRY_FIELD_GET(devObjPtr,actionEntryDataPtr,
                matchIndex,
                SMEM_SIP6_10_TTI_ACTION_TABLE_FIELDS_Triger_Cnc_Hash_Client);
        }

        actionDataPtr->policy2LookupMode =
            SMEM_LION3_TTI_ACTION_ENTRY_FIELD_GET(devObjPtr,actionEntryDataPtr,
                matchIndex,
                SMEM_LION3_TTI_ACTION_TABLE_FIELDS_No_Redirect_to_egress_interface_Policy2_Lookup_Mode);
        actionDataPtr->policy1LookupMode =
            SMEM_LION3_TTI_ACTION_ENTRY_FIELD_GET(devObjPtr,actionEntryDataPtr,
                matchIndex,
                SMEM_LION3_TTI_ACTION_TABLE_FIELDS_No_Redirect_to_egress_interface_Policy1_Lookup_Mode);
        actionDataPtr->policy0LookupMode =
            SMEM_LION3_TTI_ACTION_ENTRY_FIELD_GET(devObjPtr,actionEntryDataPtr,
                matchIndex,
                SMEM_LION3_TTI_ACTION_TABLE_FIELDS_No_Redirect_to_egress_interface_Policy0_Lookup_Mode);

        actionDataPtr->ipclProfileIndex =
            SMEM_LION3_TTI_ACTION_ENTRY_FIELD_GET(devObjPtr,actionEntryDataPtr,
                matchIndex,
                SMEM_LION3_TTI_ACTION_TABLE_FIELDS_No_Redirect_to_egress_interface_IPCL_Profile_Index);
    }

    /* In case of SIP_6_30 redirectCmd == 5, its REDIRECT_CMD_GENERIC_ACTION */
    if(SMEM_CHT_IS_SIP6_30_GET(devObjPtr))
    {
        actionDataPtr->redirectCmd =
            (actionDataPtr->redirectCmd == 5) ? PCL_TTI_ACTION_REDIRECT_CMD_GENERIC_ACTION_E :
            actionDataPtr->redirectCmd;
    }

    switch (actionDataPtr->redirectCmd)
    {
    case PCL_TTI_ACTION_REDIRECT_CMD_NONE_E:
        lionActionDataPtr->multiPortGroupTTIEn = 0;/* no such field*/
        actionDataEArchPtr->flowId =
            SMEM_LION3_TTI_ACTION_ENTRY_FIELD_GET(devObjPtr,actionEntryDataPtr,
                matchIndex,
                SMEM_LION3_TTI_ACTION_TABLE_FIELDS_No_Redirect_Flow_ID);
        actionDataEArchPtr->ipclUdbConfigurationTableUdeIndex =
            SMEM_LION3_TTI_ACTION_ENTRY_FIELD_GET(devObjPtr,actionEntryDataPtr,
                matchIndex,
                SMEM_LION3_TTI_ACTION_TABLE_FIELDS_No_Redirect_IPCL_UDB_Configuration_Table_UDE_Index);
        break;

    case PCL_TTI_ACTION_REDIRECT_CMD_OUT_IF_E:
        enable =
            SMEM_LION3_TTI_ACTION_ENTRY_FIELD_GET(devObjPtr,actionEntryDataPtr,
                matchIndex,
                SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Interface_UseEVIDX);

        SNET_SET_DEST_INTERFACE_TYPE(actionDataPtr->tunnelIf,
                                     SNET_DST_INTERFACE_VIDX_E, enable);

        if (SNET_GET_DEST_INTERFACE_TYPE(actionDataPtr->tunnelIf,
                                     SNET_DST_INTERFACE_VIDX_E))
        {
            actionDataPtr->tunnelIf.interfaceInfo.vidx = (GT_U16)
                SMEM_LION3_TTI_ACTION_ENTRY_FIELD_GET(devObjPtr,actionEntryDataPtr,
                    matchIndex,
                    SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Interface_eVIDX);
        }
        else
        {
            enable =
                SMEM_LION3_TTI_ACTION_ENTRY_FIELD_GET(devObjPtr,actionEntryDataPtr,
                    matchIndex,
                    SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Interface_IsTrunk);

            SNET_SET_DEST_INTERFACE_TYPE(actionDataPtr->tunnelIf,
                                     SNET_DST_INTERFACE_TRUNK_E, enable);

            if (SNET_GET_DEST_INTERFACE_TYPE(actionDataPtr->tunnelIf,
                                         SNET_DST_INTERFACE_TRUNK_E))
            {
                actionDataPtr->tunnelIf.interfaceInfo.trunkId = (GT_U16)
                    SMEM_LION3_TTI_ACTION_ENTRY_FIELD_GET(devObjPtr,actionEntryDataPtr,
                        matchIndex,
                        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Interface_TrunkNumber);
            }
            else
            {
                if(SMEM_CHT_IS_SIP7_GET(devObjPtr) &&
                    SMEM_LION3_TTI_ACTION_ENTRY_FIELD_GET(devObjPtr,actionEntryDataPtr,
                        matchIndex,
                        SMEM_SIP7_TTI_ACTION_TABLE_FIELDS_use_trg_vport))
                {
                    SNET_SET_DEST_INTERFACE_TYPE(actionDataPtr->tunnelIf,
                                     SNET_DST_INTERFACE_VPORT_E, 1);
                    actionDataPtr->tunnelIf.interfaceInfo.vport =
                        SMEM_LION3_TTI_ACTION_ENTRY_FIELD_GET(devObjPtr,actionEntryDataPtr,
                        matchIndex,
                        SMEM_SIP7_TTI_ACTION_TABLE_FIELDS_trg_vport);
                }
                else
                {
                    actionDataPtr->tunnelIf.interfaceInfo.devPort.port =
                        SMEM_LION3_TTI_ACTION_ENTRY_FIELD_GET(devObjPtr,actionEntryDataPtr,
                            matchIndex,
                            SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Interface_ePortNumber);
                    actionDataPtr->tunnelIf.interfaceInfo.devPort.devNum =
                        SMEM_LION3_TTI_ACTION_ENTRY_FIELD_GET(devObjPtr,actionEntryDataPtr,
                            matchIndex,
                            SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Interface_DeviceNumber);
                }
            }
        }

        actionDataPtr->vntL2Echo =
            SMEM_LION3_TTI_ACTION_ENTRY_FIELD_GET(devObjPtr,actionEntryDataPtr,
                matchIndex,
                SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Interface_VNT_L2_Echo);

        actionDataPtr->tunnelStart =
            SMEM_LION3_TTI_ACTION_ENTRY_FIELD_GET(devObjPtr,actionEntryDataPtr,
                matchIndex,
                SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Interface_Tunnel_Start);

        if (actionDataPtr->tunnelStart)
        {
            actionDataPtr->tunnelStartPtr =
                SMEM_LION3_TTI_ACTION_ENTRY_FIELD_GET(devObjPtr,actionEntryDataPtr,
                    matchIndex,
                    SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Interface_Tunnel_Index);
            actionDataPtr->tunnelStartPassengerType =
                SMEM_LION3_TTI_ACTION_ENTRY_FIELD_GET(devObjPtr,actionEntryDataPtr,
                    matchIndex,
                    SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Interface_Tunnel_Start_Passenger_Type);
        }
        else
        {
            actionDataPtr->arpPtr =
                SMEM_LION3_TTI_ACTION_ENTRY_FIELD_GET(devObjPtr,actionEntryDataPtr,
                    matchIndex,
                    SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Interface_ARP_Pointer);
        }

        lionActionDataPtr->modifyMacDa =
            SMEM_LION3_TTI_ACTION_ENTRY_FIELD_GET(devObjPtr,actionEntryDataPtr,
                matchIndex,
                SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Modify_MAC_DA);
        lionActionDataPtr->modifyMacSa =
            SMEM_LION3_TTI_ACTION_ENTRY_FIELD_GET(devObjPtr,actionEntryDataPtr,
                matchIndex,
                SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_to_Egress_Modify_MAC_SA);

        break;

    case PCL_TTI_ACTION_REDIRECT_CMD_LTT_ROUTER_E:
        actionDataPtr->routerLTTIndex =
            SMEM_LION3_TTI_ACTION_ENTRY_FIELD_GET(devObjPtr,actionEntryDataPtr,
                matchIndex,
                SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Redirect_To_Router_Router_LTT_Index);
        break;

    case PCL_TTI_ACTION_REDIRECT_CMD_VIRT_ROUTER_E:
        actionDataPtr->vrfId =
            SMEM_LION3_TTI_ACTION_ENTRY_FIELD_GET(devObjPtr,actionEntryDataPtr,
                matchIndex,
                SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Assign_VRF_ID_VRF_ID);

        break;
    case PCL_TTI_ACTION_REDIRECT_CMD_ASSIGN_SOURCE_LOGICAL_PORT_E:/* Assign Logic Port */
        __LOG(("NON supported case for SIP5 : 'Assign Logic Port' \n"));

        break;
    case PCL_TTI_ACTION_REDIRECT_CMD_GENERIC_ACTION_E:/* Assign generic action */
        if(SMEM_CHT_IS_SIP6_30_GET(devObjPtr))
        {
            /* get value of Generic Action from muxed field */
            tempValue =
                SMEM_LION3_TTI_ACTION_ENTRY_FIELD_GET(devObjPtr,actionEntryDataPtr,
                        matchIndex,
                        SMEM_SIP6_30_TTI_ACTION_TABLE_FIELDS_Generic_12b_Action);

            /* get mode of Generic Action */
            if(SMEM_CHT_IS_SIP7_GET(devObjPtr))
            {
                smemRegFldGet(devObjPtr,
                            SMEM_SIP7_TTI_LU_UNIT_GENERIC_ACTION_CONFIG_REG(devObjPtr),
                            (lookupNum*2)+3, 2,
                            &genericActionMode);
            }
            else
            {
                smemRegFldGet(devObjPtr,
                            SMEM_SIP6_30_TTI_UNIT_GLOBAL_CONFIG_EXT3_REG(devObjPtr),
                            (lookupNum*2)+3, 2,
                            &genericActionMode);
            }
            __LOG(("genericActionMode = [%d]\n", genericActionMode));

            switch(genericActionMode)
            {
                case 2:
                    actionDataEArchPtr->srcEpg = tempValue;
                    __LOG(("genericActionMode : assign srcEpg [%d]\n", tempValue));
                    break;
                case 1:
                    actionDataEArchPtr->dstEpg = tempValue;
                    __LOG(("genericActionMode : assign dstEpg [%d]\n", tempValue));
                    break;
                default:
                    __LOG(("Invalid genericActionMode = [%d]\n", genericActionMode));
            }
        }
        else
        {
            __LOG(("Configuration ERROR : generic action : Ignored for non SIP6_30 devices \n"));
        }
        break;
    default:
        break;
    }

    actionDataPtr->bindToCncCounter =
        SMEM_LION3_TTI_ACTION_ENTRY_FIELD_GET(devObjPtr,actionEntryDataPtr,
            matchIndex,
            SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Bind_To_CNC_Counter);
    actionDataPtr->cncCounterIndex =
        SMEM_LION3_TTI_ACTION_ENTRY_FIELD_GET(devObjPtr,actionEntryDataPtr,
            matchIndex,
            SMEM_LION3_TTI_ACTION_TABLE_FIELDS_CNC_Counter_Index);
    actionDataPtr->bindToPolicerMeter =
        SMEM_LION3_TTI_ACTION_ENTRY_FIELD_GET(devObjPtr,actionEntryDataPtr,
            matchIndex,
            SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Bind_To_Policer_Meter);
    actionDataPtr->bindToPolicerCounter =
        SMEM_LION3_TTI_ACTION_ENTRY_FIELD_GET(devObjPtr,actionEntryDataPtr,
            matchIndex,
            SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Bind_To_Policer_Counter);
    actionDataPtr->policerIndex =
        SMEM_LION3_TTI_ACTION_ENTRY_FIELD_GET(devObjPtr,actionEntryDataPtr,
            matchIndex,
            SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Policer_Index);
    actionDataPtr->srcIdSetEn =
        SMEM_LION3_TTI_ACTION_ENTRY_FIELD_GET(devObjPtr,actionEntryDataPtr,
            matchIndex,
            SMEM_LION3_TTI_ACTION_TABLE_FIELDS_SourceID_Set_Enable);
    actionDataPtr->srcId =
        SMEM_LION3_TTI_ACTION_ENTRY_FIELD_GET(devObjPtr,actionEntryDataPtr,
            matchIndex,
            SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Source_ID);
    actionDataPtr->actionStop =
        SMEM_LION3_TTI_ACTION_ENTRY_FIELD_GET(devObjPtr,actionEntryDataPtr,
            matchIndex,
            SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Action_Stop);
    actionDataPtr->bypassBridge =
        SMEM_LION3_TTI_ACTION_ENTRY_FIELD_GET(devObjPtr,actionEntryDataPtr,
            matchIndex,
            SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Bypass_Bridge);
    actionDataPtr->bypassIngressPipe =
        SMEM_LION3_TTI_ACTION_ENTRY_FIELD_GET(devObjPtr,actionEntryDataPtr,
            matchIndex,
            SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Bypass_Ingress_Pipe);
    actionDataPtr->vidPrecedence =
        SMEM_LION3_TTI_ACTION_ENTRY_FIELD_GET(devObjPtr,actionEntryDataPtr,
            matchIndex,
            SMEM_LION3_TTI_ACTION_TABLE_FIELDS_eVLAN_Precedence);
    actionDataPtr->nestedVidEn =
        SMEM_LION3_TTI_ACTION_ENTRY_FIELD_GET(devObjPtr,actionEntryDataPtr,
            matchIndex,
            SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Enable_Nested_VLAN);
    actionDataPtr->vid0Cmd =
        SMEM_LION3_TTI_ACTION_ENTRY_FIELD_GET(devObjPtr,actionEntryDataPtr,
            matchIndex,
            SMEM_LION3_TTI_ACTION_TABLE_FIELDS_eVID_Cmd);
    actionDataPtr->vid0 =
        SMEM_LION3_TTI_ACTION_ENTRY_FIELD_GET(devObjPtr,actionEntryDataPtr,
            matchIndex,
            SMEM_LION3_TTI_ACTION_TABLE_FIELDS_eVLAN);
    actionDataPtr->vid1Cmd =
        SMEM_LION3_TTI_ACTION_ENTRY_FIELD_GET(devObjPtr,actionEntryDataPtr,
            matchIndex,
            SMEM_LION3_TTI_ACTION_TABLE_FIELDS_VID1_Cmd);
    actionDataPtr->vid1 =
        SMEM_LION3_TTI_ACTION_ENTRY_FIELD_GET(devObjPtr,actionEntryDataPtr,
            matchIndex,
            SMEM_LION3_TTI_ACTION_TABLE_FIELDS_VID1);
    actionDataPtr->qoSPrecedence =
        SMEM_LION3_TTI_ACTION_ENTRY_FIELD_GET(devObjPtr,actionEntryDataPtr,
            matchIndex,
            SMEM_LION3_TTI_ACTION_TABLE_FIELDS_QoS_Precedence);
    actionDataPtr->qoSProfile =
        SMEM_LION3_TTI_ACTION_ENTRY_FIELD_GET(devObjPtr,actionEntryDataPtr,
            matchIndex,
            SMEM_LION3_TTI_ACTION_TABLE_FIELDS_QoSProfile);
    actionDataPtr->modifyDscp =
        SMEM_LION3_TTI_ACTION_ENTRY_FIELD_GET(devObjPtr,actionEntryDataPtr,
            matchIndex,
            SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Modify_DSCP);
    actionDataPtr->modifyUp =
        SMEM_LION3_TTI_ACTION_ENTRY_FIELD_GET(devObjPtr,actionEntryDataPtr,
            matchIndex,
            SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Modify_UP);
    actionDataPtr->keepPreviousQoS =
        SMEM_LION3_TTI_ACTION_ENTRY_FIELD_GET(devObjPtr,actionEntryDataPtr,
            matchIndex,
            SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Keep_Previous_QoS);
    actionDataPtr->trustUp =
        SMEM_LION3_TTI_ACTION_ENTRY_FIELD_GET(devObjPtr,actionEntryDataPtr,
            matchIndex,
            SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Trust_UP);
    actionDataPtr->trustDscp =
        SMEM_LION3_TTI_ACTION_ENTRY_FIELD_GET(devObjPtr,actionEntryDataPtr,
            matchIndex,
            SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Trust_DSCP);
    actionDataPtr->trustExp =
        SMEM_LION3_TTI_ACTION_ENTRY_FIELD_GET(devObjPtr,actionEntryDataPtr,
            matchIndex,
            SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Trust_EXP);
    actionDataPtr->dscpToDscpRemapingEn =
        SMEM_LION3_TTI_ACTION_ENTRY_FIELD_GET(devObjPtr,actionEntryDataPtr,
            matchIndex,
            SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Map_DSCP);
    actionDataPtr->up0 =
        SMEM_LION3_TTI_ACTION_ENTRY_FIELD_GET(devObjPtr,actionEntryDataPtr,
            matchIndex,
            SMEM_LION3_TTI_ACTION_TABLE_FIELDS_UP0);
    actionDataPtr->up1Cmd =
        SMEM_LION3_TTI_ACTION_ENTRY_FIELD_GET(devObjPtr,actionEntryDataPtr,
            matchIndex,
            SMEM_LION3_TTI_ACTION_TABLE_FIELDS_UP1_Command);
    actionDataPtr->up1 =
        SMEM_LION3_TTI_ACTION_ENTRY_FIELD_GET(devObjPtr,actionEntryDataPtr,
            matchIndex,
            SMEM_LION3_TTI_ACTION_TABLE_FIELDS_UP1);
    actionDataPtr->passengerPacketType =
        SMEM_LION3_TTI_ACTION_ENTRY_FIELD_GET(devObjPtr,actionEntryDataPtr,
            matchIndex,
            SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Passenger_Packet_Type);
    actionDataPtr->copyTtlFromOuterHeader =
        SMEM_LION3_TTI_ACTION_ENTRY_FIELD_GET(devObjPtr,actionEntryDataPtr,
            matchIndex,
            SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Copy_TTL);
    actionDataPtr->tunnelTerminateEn =
        SMEM_LION3_TTI_ACTION_ENTRY_FIELD_GET(devObjPtr,actionEntryDataPtr,
            matchIndex,
            SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Tunnel_Termination);
    actionDataPtr->mplsCmd =
        SMEM_LION3_TTI_ACTION_ENTRY_FIELD_GET(devObjPtr,actionEntryDataPtr,
            matchIndex,
            SMEM_LION3_TTI_ACTION_TABLE_FIELDS_MPLS_Command);
    lionActionDataPtr->hashMaskIndex =
        SMEM_LION3_TTI_ACTION_ENTRY_FIELD_GET(devObjPtr,actionEntryDataPtr,
            matchIndex,
            SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Hash_Mask_Index);

    actionDataEArchPtr->egressFilterRegistered = 0;/* no such field any more */

    actionDataEArchPtr->trustQosMappingTableIndex =
        SMEM_LION3_TTI_ACTION_ENTRY_FIELD_GET(devObjPtr,actionEntryDataPtr,
            matchIndex,
            SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Trust_QoS_Mapping_Table_Index);
    actionDataEArchPtr->lLspQosProfileEnable =
        SMEM_LION3_TTI_ACTION_ENTRY_FIELD_GET(devObjPtr,actionEntryDataPtr,
            matchIndex,
            SMEM_LION3_TTI_ACTION_TABLE_FIELDS_L_LSP_QoS_Profile_Enable);
    actionDataPtr->ttl =
        SMEM_LION3_TTI_ACTION_ENTRY_FIELD_GET(devObjPtr,actionEntryDataPtr,
            matchIndex,
            SMEM_LION3_TTI_ACTION_TABLE_FIELDS_TTL);
    actionDataPtr->ttlDecEn =
        SMEM_LION3_TTI_ACTION_ENTRY_FIELD_GET(devObjPtr,actionEntryDataPtr,
            matchIndex,
            SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Enable_Dec_TTL);

    if(!SMEM_CHT_IS_SIP7_GET(devObjPtr))
    {
        actionDataEArchPtr->assignSrcInterfaceEnable =
            SMEM_LION3_TTI_ACTION_ENTRY_FIELD_GET(devObjPtr,actionEntryDataPtr,
                matchIndex,
                SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Source_ePort_Assignment_Enable);
    }
    else
    {
        actionDataEArchPtr->assignSrcInterfaceEnable =
            SMEM_LION3_TTI_ACTION_ENTRY_FIELD_GET(devObjPtr,actionEntryDataPtr,
                matchIndex,
                SMEM_SIP7_TTI_ACTION_TABLE_FIELDS_Source_Interface_Assignment_Enable);
    }

    actionDataEArchPtr->sourceEPort =
        SMEM_LION3_TTI_ACTION_ENTRY_FIELD_GET(devObjPtr,actionEntryDataPtr,
            matchIndex,
            SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Source_ePort);
    actionDataEArchPtr->ttHeaderLength =
        SMEM_LION3_TTI_ACTION_ENTRY_FIELD_GET(devObjPtr,actionEntryDataPtr,
            matchIndex,
            SMEM_LION3_TTI_ACTION_TABLE_FIELDS_TT_Header_Length);

    if(SMEM_CHT_IS_SIP5_16_GET(devObjPtr) &&
      !SMEM_CHT_IS_SIP5_20_GET(devObjPtr) )
    {
        if(actionDataPtr->redirectCmd != PCL_TTI_ACTION_REDIRECT_CMD_OUT_IF_E)
        {
            /* the fix depends on : Metal_fix[0] called <Tunnel Enalargment Disable> */
            smemRegGet(devObjPtr,
                SMEM_CHT_MAC_REG_DB_SIP5_GET(devObjPtr)->TTI.TTI_Internal.MetalFixRegister,
                &tempValue);
            LOG_FIELD_VALUE("<Tunnel Enlargement Disable>",(tempValue & 1));

            if(0 == (tempValue & 1))
            {
                /* fix of JIRA : TTI-1459 : New PD requirement - support tunnels > 80B */
                tempValue =
                    SMEM_LION3_TTI_ACTION_ENTRY_FIELD_GET(devObjPtr,actionEntryDataPtr,
                        matchIndex,
                        SMEM_LION3_TTI_ACTION_TABLE_FIELDS_No_Redirect_to_egress_interface_TTI_reserved_Value);

                __LOG(("use bit 0 (value[%d]) of 'TTI_reserved_Value' as MSBit of 'TT_Header_Length' \n",
                    (tempValue & 1)));
                /*field <ttHeaderLength> is in granularity of 2 */
                actionDataEArchPtr->ttHeaderLength |= (tempValue & 1) << 5;

                LOG_FIELD_VALUE("new TT_Header_Length",actionDataEArchPtr->ttHeaderLength);
            }
        }
    }

    actionDataEArchPtr->isPtpPacket =
        SMEM_LION3_TTI_ACTION_ENTRY_FIELD_GET(devObjPtr,actionEntryDataPtr,
            matchIndex,
            SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Is_PTP_Packet);
    actionDataPtr->oamInfo.ptpTimeStampEnable =
        SMEM_LION3_TTI_ACTION_ENTRY_FIELD_GET(devObjPtr,actionEntryDataPtr,
            matchIndex,
            SMEM_LION3_TTI_ACTION_TABLE_FIELDS_NON_PTP_Packet_Timestamp_Enable);
    actionDataPtr->oamInfo.ptpTimeStampOffsetIndex =
        SMEM_LION3_TTI_ACTION_ENTRY_FIELD_GET(devObjPtr,actionEntryDataPtr,
            matchIndex,
            SMEM_LION3_TTI_ACTION_TABLE_FIELDS_NON_PTP_Packet_Offset_Index);
    actionDataEArchPtr->ptpTriggerType =
        SMEM_LION3_TTI_ACTION_ENTRY_FIELD_GET(devObjPtr,actionEntryDataPtr,
            matchIndex,
            SMEM_LION3_TTI_ACTION_TABLE_FIELDS_PTP_Packet_PTP_Trigger_Type);
    actionDataEArchPtr->ptpOffset =
        SMEM_LION3_TTI_ACTION_ENTRY_FIELD_GET(devObjPtr,actionEntryDataPtr,
            matchIndex,
            SMEM_LION3_TTI_ACTION_TABLE_FIELDS_PTP_Packet_PTP_Offset);
    actionDataPtr->oamInfo.oamProcessEnable =
        SMEM_LION3_TTI_ACTION_ENTRY_FIELD_GET(devObjPtr,actionEntryDataPtr,
            matchIndex,
            SMEM_LION3_TTI_ACTION_TABLE_FIELDS_OAM_Processing_Enable);
    actionDataEArchPtr->cwBasedPseudoWire =
        SMEM_LION3_TTI_ACTION_ENTRY_FIELD_GET(devObjPtr,actionEntryDataPtr,
            matchIndex,
            SMEM_LION3_TTI_ACTION_TABLE_FIELDS_CW_Based_Pseudo_Wire);
    actionDataEArchPtr->ttlExpiryVccvEnable =
        SMEM_LION3_TTI_ACTION_ENTRY_FIELD_GET(devObjPtr,actionEntryDataPtr,
            matchIndex,
            SMEM_LION3_TTI_ACTION_TABLE_FIELDS_TTL_Expiry_VCCV_Enable);
    actionDataEArchPtr->pwe3FlowLabelExist =
        SMEM_LION3_TTI_ACTION_ENTRY_FIELD_GET(devObjPtr,actionEntryDataPtr,
            matchIndex,
            SMEM_LION3_TTI_ACTION_TABLE_FIELDS_PWE3_Flow_Label_Exist);
    actionDataEArchPtr->pwCwBasedETreeEnable =
        SMEM_LION3_TTI_ACTION_ENTRY_FIELD_GET(devObjPtr,actionEntryDataPtr,
            matchIndex,
            SMEM_LION3_TTI_ACTION_TABLE_FIELDS_PW_CW_Based_E_Tree_Enable);
    actionDataEArchPtr->up2QosProfileMappingMode =
        SMEM_LION3_TTI_ACTION_ENTRY_FIELD_GET(devObjPtr,actionEntryDataPtr,
            matchIndex,
            SMEM_LION3_TTI_ACTION_TABLE_FIELDS_UP_CFI_to_QoS_table_select_mode);
    actionDataEArchPtr->rxIsProtectionPath =
        SMEM_LION3_TTI_ACTION_ENTRY_FIELD_GET(devObjPtr,actionEntryDataPtr,
            matchIndex,
            SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Rx_Is_Protection_Path);
    actionDataEArchPtr->rxEnableProtectionSwitching =
        SMEM_LION3_TTI_ACTION_ENTRY_FIELD_GET(devObjPtr,actionEntryDataPtr,
            matchIndex,
            SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Rx_Enable_Protection_Switching);
    actionDataEArchPtr->setMAC2ME =
        SMEM_LION3_TTI_ACTION_ENTRY_FIELD_GET(devObjPtr,actionEntryDataPtr,
            matchIndex,
            SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Set_MAC2ME);
    actionDataPtr->oamInfo.oamProfile =
        SMEM_LION3_TTI_ACTION_ENTRY_FIELD_GET(devObjPtr,actionEntryDataPtr,
            matchIndex,
            SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Oam_Profile);
    actionDataEArchPtr->applynonDataCwCommand =
        SMEM_LION3_TTI_ACTION_ENTRY_FIELD_GET(devObjPtr,actionEntryDataPtr,
            matchIndex,
            SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Apply_non_Data_CW_Command);
    actionDataEArchPtr->pwTagMode =
        SMEM_LION3_TTI_ACTION_ENTRY_FIELD_GET(devObjPtr,actionEntryDataPtr,
            matchIndex,
            SMEM_LION3_TTI_ACTION_TABLE_FIELDS_PW_Tag_Mode);
    if(SMEM_CHT_IS_SIP7_GET(devObjPtr))
    {
        /* only <stop mpls> for actionDataPtr->mplsCmd != NOP
           and <action stop> ,
            will not allow to continue */
        actionDataEArchPtr->continueToNextTtiLookup = 1;
    }
    else
    {
        actionDataEArchPtr->continueToNextTtiLookup =
            SMEM_LION3_TTI_ACTION_ENTRY_FIELD_GET(devObjPtr,actionEntryDataPtr,
                matchIndex,
                SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Enable_Next_Lookup);
    }
    actionDataPtr->oamInfo.channelTypeToOpcodeMappingEn =
        SMEM_LION3_TTI_ACTION_ENTRY_FIELD_GET(devObjPtr,actionEntryDataPtr,
            matchIndex,
            SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Channel_Type_to_Opcode_Mapping_En);
    actionDataEArchPtr->passengerParsingOfTransitMplsTunnelEnable =
        SMEM_LION3_TTI_ACTION_ENTRY_FIELD_GET(devObjPtr,actionEntryDataPtr,
            matchIndex,
            SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Passenger_Parsing_of_Transit_MPLS_Tunnel_Enable);
    actionDataEArchPtr->passengerParsingOfNonMplsTransitTunnelEnable =
        SMEM_LION3_TTI_ACTION_ENTRY_FIELD_GET(devObjPtr,actionEntryDataPtr,
            matchIndex,
            SMEM_LION3_TTI_ACTION_TABLE_FIELDS_Passenger_Parsing_of_Non_MPLS_Transit_tunnel_Enable);

    if(SMEM_CHT_IS_SIP6_GET(devObjPtr))
    {
        actionDataEArchPtr->skipFdbSaLookup =
            SMEM_LION3_TTI_ACTION_ENTRY_FIELD_GET(devObjPtr,actionEntryDataPtr,
            matchIndex,
            SMEM_SIP6_TTI_ACTION_TABLE_FIELDS_Skip_Fdb_SA_Lookup);

        actionDataEArchPtr->setIpv6SegmentRoutingEndNode =
            SMEM_LION3_TTI_ACTION_ENTRY_FIELD_GET(devObjPtr,actionEntryDataPtr,
            matchIndex,
            SMEM_SIP6_TTI_ACTION_TABLE_FIELDS_Set_Ipv6_Segment_Routing_End_Node);

            actionDataEArchPtr->tunnelHeaderLengthAnchorType =
                SMEM_LION3_TTI_ACTION_ENTRY_FIELD_GET(devObjPtr,actionEntryDataPtr,
                matchIndex,
                SMEM_SIP6_TTI_ACTION_TABLE_FIELDS_Tunnel_Header_Length_Anchor_Type);
    }

    if(SMEM_CHT_IS_SIP6_10_GET(devObjPtr))
    {
        actionDataEArchPtr->ppuProfileIdx =
            SMEM_LION3_TTI_ACTION_ENTRY_FIELD_GET(devObjPtr,actionEntryDataPtr,
            matchIndex,
            SMEM_SIP6_10_TTI_ACTION_TABLE_FIELDS_Ppu_Profile);

        actionDataEArchPtr->ipfixEnable =
            SMEM_LION3_TTI_ACTION_ENTRY_FIELD_GET(devObjPtr,actionEntryDataPtr,
            matchIndex,
            SMEM_SIP6_10_TTI_ACTION_TABLE_FIELDS_Ipfix_enable);

        actionDataEArchPtr->triggerCncHashClient =
            SMEM_LION3_TTI_ACTION_ENTRY_FIELD_GET(devObjPtr,actionEntryDataPtr,
            matchIndex,
            SMEM_SIP6_10_TTI_ACTION_TABLE_FIELDS_Triger_Cnc_Hash_Client);
    }

    if(SMEM_CHT_IS_SIP7_GET(devObjPtr)) /* same logic as in snetSip7PclActionSrcVportAssignmentGet */
    {
        /*already got : actionDataEArchPtr->assignSrcInterfaceEnable */
        actionDataEArchPtr->sourceVPort =
            SMEM_LION3_TTI_ACTION_ENTRY_FIELD_GET(devObjPtr,actionEntryDataPtr,
            matchIndex,
            SMEM_SIP7_TTI_ACTION_TABLE_FIELDS_source_vport);

        actionDataEArchPtr->sourceVPortProfileAssignmentEnable =
            SMEM_LION3_TTI_ACTION_ENTRY_FIELD_GET(devObjPtr,actionEntryDataPtr,
            matchIndex,
            SMEM_SIP7_TTI_ACTION_TABLE_FIELDS_source_Vport_profile_assignment_en);

        /* QoS Action Resolution */
        actionDataEArchPtr->qosModel =
            SMEM_LION3_TTI_ACTION_ENTRY_FIELD_GET(devObjPtr,actionEntryDataPtr,
            matchIndex,
            SMEM_SIP7_TTI_ACTION_TABLE_FIELDS_qos_model);

        /* TTL Action Resolution */
        actionDataEArchPtr->ttlModel =
            SMEM_LION3_TTI_ACTION_ENTRY_FIELD_GET(devObjPtr,actionEntryDataPtr,
            matchIndex,
            SMEM_SIP7_TTI_ACTION_TABLE_FIELDS_ttl_model);

        /* QoS Mapping Mode */
        actionDataEArchPtr->qosMappingMode =
            SMEM_LION3_TTI_ACTION_ENTRY_FIELD_GET(devObjPtr,actionEntryDataPtr,
            matchIndex,
            SMEM_SIP7_TTI_ACTION_TABLE_FIELDS_qos_mapping_mode);

    }
    return;
}

/**
* @internal snetFalconTTEcnResolve function
* @endinternal
*
* @brief   Sip6 devices : Resolve ECN value for tunnel-terminated packet.
*
* @param[in] devObjPtr                - pointer to device object.
* @param[in] descrPtr                 - pointer to frame descriptor.
*
*/
static GT_VOID snetFalconTTEcnResolve
(
    IN SKERNEL_DEVICE_OBJECT            *devObjPtr,
    IN SKERNEL_FRAME_CHEETAH_DESCR_STC  *descrPtr
)
{
    IP_ECN_ENT                   outerEcn;
    IP_ECN_ENT                   passengerEcn;
    GT_U32                       regValue;
    IP_ECN_ENT                   ecnValue;
    SKERNEL_EXT_PACKET_CMD_ENT   pktCmd  = 0;
    GT_U32                       cpuCode = 0;
    GT_U32                       ipCheckSum;
    GT_BOOL                      applyCommand = GT_FALSE;

    DECLARE_FUNC_NAME(snetFalconTTEcnResolve);

    if (descrPtr->origInfoBeforeTunnelTermination.isIp &&
                descrPtr->isIp)
    {
        outerEcn = descrPtr->origInfoBeforeTunnelTermination.originalL3Ptr[1] & 0x3;
        passengerEcn = descrPtr->l3StartOffsetPtr[1] & 0x3;
        __LOG(("Outer ECN value: [%d]", outerEcn));
        __LOG(("Passenger ECN value: [%d]", passengerEcn));
        if (outerEcn == IP_ECN_ECT_1_E && passengerEcn == IP_ECN_ECT_0_E)
        {
            ecnValue = IP_ECN_ECT_1_E;
            __LOG(("Set ECT_1 value"));
        }
        else if (outerEcn == IP_ECN_CE_E && passengerEcn == IP_ECN_NOT_ECT_E)
        {
            ecnValue = IP_ECN_NOT_ECT_E;
            __LOG(("Set NOT_ECT value"));
            smemRegGet(devObjPtr,
                SMEM_SIP6_TTI_ECN_DROPPED_CONFIG_REG_MAC(devObjPtr),
                &regValue);
            pktCmd = regValue & 0x7;
            __LOG(("TTI ECN Dropped config packet command: [%d]", pktCmd));
            if (pktCmd != SKERNEL_EXT_PKT_CMD_FORWARD_E)
            {
                cpuCode = (regValue >> 3) & 0xFF;
                __LOG(("TTI ECN Dropped config CPU code: [%d]", cpuCode));
            }
            applyCommand = GT_TRUE;
        }
        else if (outerEcn == IP_ECN_CE_E && (passengerEcn == IP_ECN_ECT_0_E || passengerEcn == IP_ECN_ECT_1_E))
        {
            ecnValue= IP_ECN_CE_E;
            __LOG(("Set CE value"));
        }
        else
        {
            ecnValue = passengerEcn;
            __LOG(("Copy ECN value [%d] from passenger", ecnValue));
        }
        descrPtr->l3StartOffsetPtr[1] &= 0xFC;
        descrPtr->l3StartOffsetPtr[1] |= ecnValue;
        __LOG(("Resolved ECN value: [%d]", ecnValue));
        if(descrPtr->isIPv4)
        {
            __LOG(("Recalculate IP header checksum"));
            descrPtr->l3StartOffsetPtr[10] = 0;
            descrPtr->l3StartOffsetPtr[11] = 0;
            ipCheckSum = ipV4CheckSumCalc(descrPtr->l3StartOffsetPtr,
                (GT_U16)((descrPtr->l3StartOffsetPtr[0] & 0xF) * 4));
            descrPtr->l3StartOffsetPtr[10] = (GT_U8)(ipCheckSum >> 8);
            descrPtr->l3StartOffsetPtr[11] = (GT_U8)(ipCheckSum);
            __LOG (("New IP header checksum: [%d]", ipCheckSum));
            if (applyCommand)
            {
                snetChtIngressCommandAndCpuCodeResolution(devObjPtr,descrPtr,
                                    descrPtr->packetCmd,
                                    pktCmd,
                                    descrPtr->cpuCode,
                                    cpuCode,
                                    SNET_CHEETAH_ENGINE_UNIT_TTI_E,
                                    GT_FALSE);
            }
        }
    }

    return;
}

/**
* @internal snetAasUpdateForwardingLabelInfo function
* @endinternal
*
* @brief  Updates the MPLS label descriptor fields based on
*         the TTI Action
* @param[in]     devObjPtr                - pointer to device object.
* @param[in,out] descrPtr                 - pointer to frame data buffer Id
* @param[in]     mplsCommand              - TTI Action : MPLS Command
*/
GT_VOID snetAasUpdateForwardingLabelInfo
(
    IN    SKERNEL_DEVICE_OBJECT             *devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC   *descrPtr,
    IN    SKERNEL_XCAT_TTI_MPLS_COMMAND_ENT  mplsCommand
)
{
    DECLARE_FUNC_NAME(snetAasUpdateForwardingLabelInfo);
    switch(mplsCommand)
    {
        case SKERNEL_XCAT_TTI_MPLS_SWAP_E:
            descrPtr->currFwdLabelIndex += 1;
            descrPtr->fwdLabelsPopSwap  += 1;
            descrPtr->isLabelSwapped = 1;
            break;
        case SKERNEL_XCAT_TTI_MPLS_POP_1_E:
            descrPtr->currFwdLabelIndex += 1;
            descrPtr->fwdLabelsPopSwap  += 1;
            descrPtr->totalPoppedLabels += 1;
            break;
        case SKERNEL_XCAT_TTI_MPLS_POP_2_E:
            descrPtr->currFwdLabelIndex += 2;
            descrPtr->fwdLabelsPopSwap  += 2;
            descrPtr->totalPoppedLabels += 2;
            break;
        case SKERNEL_XCAT_TTI_MPLS_POP_3_E:
            descrPtr->currFwdLabelIndex += 3;
            descrPtr->fwdLabelsPopSwap  += 3;
            descrPtr->totalPoppedLabels += 3;
            break;
        case SKERNEL_XCAT_TTI_MPLS_POP_SWAP_E:
            descrPtr->currFwdLabelIndex += 2;
            descrPtr->fwdLabelsPopSwap  += 2;
            descrPtr->isLabelSwapped = 1;
            descrPtr->totalPoppedLabels += 1;
            break;
        case SKERNEL_XCAT_TTI_MPLS_PUSH_E:
            /* This code is added in WM to indicate other modules
             * about PUSH operation */
            descrPtr->isLabelPushed = 1;
            break;
        case SKERNEL_XCAT_TTI_MPLS_NOP_E:
        default:
            __LOG (("Not Handling this MPLS Action[%u]\n", mplsCommand));
    }
}

/**
* @internal snetAasQosActionResolution function
* @endinternal
*
* @brief  Performs the QoS Action Resolution
*
* @param[in]     devObjPtr                - pointer to device object.
* @param[in,out] descrPtr                 - pointer to frame data buffer Id
* @param[in]     lookupNum                - Parallel Lookup Num
* @param[in]     actionDataEArchPtr       - (pointer to) TTI Action data
*/
GT_VOID snetAasQosActionResolution
(
    IN    SKERNEL_DEVICE_OBJECT             *devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC   *descrPtr,
    IN    GT_U32                             lookupNum,
    IN    SNET_E_ARCH_TT_ACTION_STC         *actionDataEArchPtr
)
{
    DECLARE_FUNC_NAME(snetAasQosActionResolution);
    GT_U32  regVal;
    GT_U32  entryIndex;
    GT_U32  nextQosModelSelectionMode;
    GT_U32  nextQosMappingTableSelectionMode;
    GT_U32  nextQosTrustFieldsSelectionMode;
    GT_U32  nextExp4QosSelectionMode;
    GT_U32  nextExp4EgressSelectionMode;

    __LOG (("Entering snetAasQosActionResolution, lookupNum[%u]\n", lookupNum));

    /* Generate the QoS Table index
     * Qos Action resolution Table is a 64-entry table that is
     * indexed by the 6b concatentation:
     * {Internal <Selected QoS Model> (3b) // the previous resolution,
     * Current Action <QoS Model> (3b) // the current Action assignment }
     */
    entryIndex = ((descrPtr->qosModel << 3) | (actionDataEArchPtr->qosModel));

    __LOG_PARAM(descrPtr->qosModel);
    __LOG_PARAM(actionDataEArchPtr->qosModel);
    __LOG_PARAM(entryIndex);

    /* Read the entry from QoS Action Table */
    smemRegGet(devObjPtr, SMEM_SIP7_TTI_LU_PARALLEL_LOOKUP_QOS_ACTION_REG(devObjPtr, lookupNum, entryIndex), &regVal);

    nextQosModelSelectionMode        = (regVal&0x1);
    nextQosMappingTableSelectionMode = ((regVal>>1)&0x1);
    nextQosTrustFieldsSelectionMode  = ((regVal>>2)&0x1);
    nextExp4QosSelectionMode         = ((regVal>>3)&0x3);
    nextExp4EgressSelectionMode      = ((regVal>>5)&0x3);

    __LOG_PARAM(nextQosModelSelectionMode);
    __LOG_PARAM(nextQosMappingTableSelectionMode);
    __LOG_PARAM(nextQosTrustFieldsSelectionMode);
    __LOG_PARAM(nextExp4QosSelectionMode);
    __LOG_PARAM(nextExp4EgressSelectionMode);

    if (nextQosModelSelectionMode == TTI_QOS_MODEL_CURRENT_E)
    {
        __LOG (("QoS Model Selection Mode is Current for lookupNum[%u]\n", lookupNum));
        __LOG (("Setting QoS model [%u] from Action\n",actionDataEArchPtr->qosModel));
        descrPtr->qosModel = actionDataEArchPtr->qosModel;
    }
    else
    {
        __LOG (("Qos Model Selection Mode is Previous for lookupNum[%u]. No more changes to Qos Params. Returning\n",
                lookupNum));
        return;
    }

    if (nextQosMappingTableSelectionMode == TTI_TRUST_QOS_MAPPING_TABLE_CURRENT_E)
    {
        __LOG (("Qos Mapping Table Selection Mode is Current for lookupNum[%u]\n", lookupNum));
        descrPtr->trustQosMappingTableIndex = actionDataEArchPtr->trustQosMappingTableIndex;
    }

    if (nextQosTrustFieldsSelectionMode == TTI_QOS_TRUST_FIELDS_SELECTION_CURRENT_E)
    {
        __LOG (("Qos Trust Fields Selection Mode is Current for lookupNum[%u]\n", lookupNum));
        descrPtr->trustExpFlag  = actionDataEArchPtr->baseActionData.xcatTTActionData.trustExp;
        descrPtr->trustDscpFlag = actionDataEArchPtr->baseActionData.xcatTTActionData.trustDscp;
        descrPtr->trustUpFlag   = actionDataEArchPtr->baseActionData.xcatTTActionData.trustUp;
    }
    else
    {
        __LOG (("Qos Trust Fields Selection Mode: Keeping Previous QoS Selection for lookupNum[%u]\n", lookupNum));
        actionDataEArchPtr->baseActionData.xcatTTActionData.keepPreviousQoS = GT_TRUE;
    }

    /* TBD: Update the fields accordingly */
    if (nextExp4QosSelectionMode == TTI_EXP_4_QOS_SELECTION_CURRENT_E)
    {
        __LOG (("Qos Exp4Qos Selection Mode is Current for lookupNum[%u]\n", lookupNum));
        if (descrPtr->isLabelValid[descrPtr->currFwdLabelIndex])
        {
        __LOG (("Setting exp4Qos [%u] from current label index [%u]\n",
                descrPtr->exp[descrPtr->currFwdLabelIndex], descrPtr->currFwdLabelIndex));
            descrPtr->exp4Qos = descrPtr->exp[descrPtr->currFwdLabelIndex];
        }
        else
        {
            __LOG (("Current Label is not Valid: Configuration Error, Setting Exp4Qos to 0\n"));
            /* configuration error - current label should be valid */
            descrPtr->exp4Qos = 0;
            /* DebugCounter.EXP4QoS.InvalidLabel++ */
        }
    }
    else if (nextExp4QosSelectionMode == TTI_EXP_4_QOS_SELECTION_NEXT_E)
    {
        __LOG (("Qos Exp4Qos Selection Mode is Next for lookupNum[%u]\n", lookupNum));
        if (descrPtr->isLabelValid[descrPtr->currFwdLabelIndex + 1])
        {
            __LOG (("Setting exp4Qos [%u] from next label index [%u]\n",
                    descrPtr->exp[descrPtr->currFwdLabelIndex+1], descrPtr->currFwdLabelIndex + 1));
            descrPtr->exp4Qos = descrPtr->exp[descrPtr->currFwdLabelIndex + 1];
        }
        else
        {
            __LOG (("Next Label is not Valid: Configuration Error, Setting Exp4Qos to 0\n"));
            /* configuration error - next label should be valid */
            descrPtr->exp4Qos = 0;
            /* DebugCounter.EXP4QoS.InvalidLabel++ */
        }
    }

    if (nextExp4EgressSelectionMode == TTI_EXP_4_EGRESS_SELECTION_CURRENT_E)
    {
        __LOG (("Qos Exp4Egress Selection Mode is Current for lookupNum[%u]\n", lookupNum));
        if (descrPtr->isLabelValid[descrPtr->currFwdLabelIndex])
        {
        __LOG (("Setting exp4Egress [%u] from current label index [%u]\n",
                descrPtr->exp[descrPtr->currFwdLabelIndex], descrPtr->currFwdLabelIndex));
            descrPtr->exp4Egress = descrPtr->exp[descrPtr->currFwdLabelIndex];
        }
        else
        {
            __LOG (("Current Label is not Valid: Configuration Error, Setting Exp4Egress to 0\n"));
            /* configuration error - current label should be valid */
            descrPtr->exp4Egress = 0;
            /* DebugCounter.EXP4Egress.InvalidLabel++ */
        }
    }
    else if (nextExp4EgressSelectionMode == TTI_EXP_4_EGRESS_SELECTION_NEXT_E)
    {
        __LOG (("Qos Exp4Qos Selection Mode is Next for lookupNum[%u]\n", lookupNum));
        if (descrPtr->isLabelValid[descrPtr->currFwdLabelIndex + 1])
        {
            __LOG (("Setting exp4Qos [%u] from next label index [%u]\n",
                    descrPtr->exp[descrPtr->currFwdLabelIndex + 1], descrPtr->currFwdLabelIndex + 1));
            descrPtr->exp4Egress = descrPtr->exp[descrPtr->currFwdLabelIndex + 1];
        }
        else
        {
            __LOG (("Next Label is not Valid: Configuration Error, Setting Exp4Egress to 0\n"));
            /* configuration error - next label should be valid */
            descrPtr->exp4Egress = 0;
            /* DebugCounter.EXP4Egress.InvalidLabel++ */
        }
    }

    /* Setting the QoS Mapping Mode */
    descrPtr->qos.ingressExtendedMode = actionDataEArchPtr->qosMappingMode;
}

/**
* @internal snetAasTtlActionResolution function
* @endinternal
*
* @brief  Performs the TTL Action Resolution
*
* @param[in]     devObjPtr                - pointer to device object.
* @param[in,out] descrPtr                 - pointer to frame data buffer Id
* @param[in]     lookupNum                - Parallel Lookup Num
* @param[in]     actionDataEArchPtr       - (pointer to) TTI Action data
*/
GT_VOID snetAasTtlActionResolution
(
    IN    SKERNEL_DEVICE_OBJECT             *devObjPtr,
    INOUT SKERNEL_FRAME_CHEETAH_DESCR_STC   *descrPtr,
    IN    GT_U32                             lookupNum,
    IN    SNET_E_ARCH_TT_ACTION_STC         *actionDataEArchPtr
)
{
    DECLARE_FUNC_NAME(snetAasTtlActionResolution);
    GT_U32  regVal;
    GT_U32  entryIndex;
    GT_U32  nextTTLModelSelectionMode;
    GT_U32  nextTTLSelectionMode;

    __LOG (("Entering snetAasTtlActionResolution, lookupNum[%u]\n", lookupNum));

    /* Generate the TTL Table index
     * TTL Action resolution Table is a 15-entry table that is
     * indexed by the 4b concatentation:
     * {Internal <Selected TTL Model> (2b) // the previous resolution,
     * Current Action <TTL Model> (2b) // the current Action assignment }
     */
    entryIndex = ((descrPtr->ttlModel << 2) | (actionDataEArchPtr->ttlModel));

    __LOG_PARAM(descrPtr->ttlModel);
    __LOG_PARAM(actionDataEArchPtr->ttlModel);
    __LOG_PARAM(entryIndex);

    /* Read the entry from QoS Action Table */
    smemRegGet(devObjPtr, SMEM_SIP7_TTI_LU_PARALLEL_LOOKUP_TTL_ACTION_REG(devObjPtr, lookupNum, entryIndex), &regVal);

    nextTTLModelSelectionMode    = (regVal&0x1);
    nextTTLSelectionMode         = ((regVal>>1)&0x3);

    __LOG_PARAM(nextTTLModelSelectionMode);
    __LOG_PARAM(nextTTLSelectionMode);

    if (nextTTLModelSelectionMode == TTI_TTL_MODEL_CURRENT_E)
    {
        __LOG (("TTL Model Selection Mode is Current for lookupNum[%u]\n", lookupNum));
        __LOG (("Setting TTL model [%u] from Action\n",actionDataEArchPtr->ttlModel));
        descrPtr->ttlModel = actionDataEArchPtr->ttlModel;
    }
    else
    {
        __LOG (("TTL Model Selection Mode is Previous for lookupNum[%u]. No more changes to TTL params. Returning\n",
               lookupNum));
        return;
    }

    if (nextTTLSelectionMode == TTI_TTL_SELECT_CURRENT_E)
    {
        __LOG (("TTL Selection Mode is Current for lookupNum[%u]\n", lookupNum));
        if (descrPtr->isLabelValid[descrPtr->currFwdLabelIndex])
        {
        __LOG (("Setting TTL [%u] from current label Index[%u]\n",
                descrPtr->ttlArr[descrPtr->currFwdLabelIndex], descrPtr->currFwdLabelIndex));
            descrPtr->ttl = descrPtr->ttlArr[descrPtr->currFwdLabelIndex];
        }
        else if(descrPtr->isIp)
        {
        __LOG (("IPv4/IPv6 case: Setting TTL [%u] from L3 header\n", descrPtr->l3StartOffsetPtr[8]));
            /* IPv4 or IPv6 case */
            descrPtr->ttl = descrPtr->l3StartOffsetPtr[8];
        }
        else
        {
        __LOG (("Current label is not valid, setting TTL to default value(255)\n"));
            /* Current label is not valid and packet is not IP */
            /* Default TTL value */
            descrPtr->ttl = 255;
        }
    }
    else if (nextTTLSelectionMode == TTI_TTL_SELECT_NEXT_E)
    {
        __LOG (("TTL Selection Mode is Next for lookupNum[%u]\n", lookupNum));
        if (descrPtr->isLabelValid[descrPtr->currFwdLabelIndex + 1])
        {
            descrPtr->ttl = descrPtr->ttlArr[descrPtr->currFwdLabelIndex + 1];
        }
        else
        {
        __LOG (("Next label is not valid, setting copyTTL as TRUE\n"));
            /* Assign TTL from Passenger */
            descrPtr->copyTtl = GT_TRUE;
        }
    }
}
