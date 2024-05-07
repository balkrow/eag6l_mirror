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
* @file prvTgfIngressMirrorSflow.c
*
* @brief Ingress SFLOW use cases testing.
*        Supporting SFLOW V5 use case
*
*        Ports allocation:
*           ^ Port#1: Analyzer port
*           ^ Port#2: Target port
*           ^ Port#3: Ingress port
*
*        Packet flow:
*
* @version   1
********************************************************************************
*/
#include <mirror/prvTgfIngressMirrorSflow.h>
#include <oam/prvTgfOamGeneral.h>
#include <common/tgfL2MllGen.h>
#include <cpss/dxCh/dxChxGen/pha/cpssDxChPha.h>
#include <cpss/dxCh/dxChxGen/oam/cpssDxChOam.h>
#include <common/tgfConfigGen.h>
#include <common/tgfBridgeL2EcmpGen.h>
#include <common/tgfMirror.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgE2Phy.h>
#include <cpss/dxCh/dxChxGen/pha/cpssDxChPha.h>
#include <common/tgfCscdGen.h>
#include <cpss/dxCh/dxChxGen/bridge/cpssDxChBrgPrvEdgeVlan.h>

/*************************** Constants definitions ***********************************/

/* PHA fw thread IDs of */
#define PRV_TGF_PHA_THREAD_ID_SFLOW_V5_IPV4_CNS   57

#define PRV_TGF_PHA_THREAD_ID_ENHANCED_SFLOW_SAVE_TARGET_DEV_CNS    61
#define PRV_TGF_PHA_THREAD_ID_ENHANCED_SFLOW_IPV4_CNS               62
#define PRV_TGF_PHA_THREAD_ID_ENHANCED_SFLOW_IPV6_CNS               63
#define PRV_TGF_PHA_THREAD_ID_ENHANCED_SFLOW_CNS                    66
/*EPCL rule index*/
#define PRV_TGF_ENHANCED_SFLOW_EPCL_IDX_CNS             0
#define PRV_TGF_ENHANCED_SFLOW_SAVE_TARGET_EPCL_IDX_CNS 4
#define PRV_TGF_ENHANCED_SFLOW_IPV4_EPCL_IDX_CNS        8
#define PRV_TGF_ENHANCED_SFLOW_IPV6_EPCL_IDX_CNS        12
/* Template profile index */
#define PRV_TGF_TEMPLATE_PROFILE_INDEX_CNS  1

/* Tunnel start entry index */
#define PRV_TGF_TUNNEL_START_IPV4_INDEX_CNS      10
#define PRV_TGF_TUNNEL_START_IPV6_INDEX_CNS      20

/* default VLAN0 Id */
#define PRV_TGF_VLANID_CNS  5

/*SFLOW UDP port number*/
#define PRV_TGF_SFLOW_V5_UDP_PORT_CNS 6343

/* port number to receive traffic from*/
#define PRV_TGF_TX_PORT_IDX_CNS        0
#define PRV_TGF_ANALYZER_PORT_IDX_CNS  1
#define PRV_TGF_ANALYZER_PORT2_IDX_CNS 2
#define PRV_TGF_SERVICE_PORT_IDX_CNS   3
#define PRV_TGF_RX_PORT_IDX_CNS        PRV_TGF_SERVICE_PORT_IDX_CNS
#define PRV_TGF_LB_PORT_IDX_CNS        PRV_TGF_ANALYZER_PORT2_IDX_CNS

#define PRV_TGF_DEFAULT_EPORT_CNS     1024
#define PRV_TGF_PASS2_EPORT_CNS       1027

/* Packet flow Id to set by PCL Action  */
#define PRV_TGF_EPCL_FLOW_ID_CNS            25
/*analyzer index for mirroring sflow */
#define PRV_TGF_SFLOW_MIRROR_IDX_CNS 2

/*sampling rate*/
#define PRV_TGF_SFLOW_MIRROR_RATE_CNS 1

/*SFLOW V5 offsets*/
#define PRV_TGF_SFLOW_V5_IPV6_OFFSET_CNS (TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS + TGF_IPV6_HEADER_SIZE_CNS + TGF_UDP_HEADER_SIZE_CNS )
#define PRV_TGF_SFLOW_V5_IPV4_OFFSET_CNS (TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS + TGF_IPV4_HEADER_SIZE_CNS + TGF_UDP_HEADER_SIZE_CNS )
/*Standard Enterprise data format type*/
#define PRV_TGF_SFLOW_V5_DATA_FORMAT_CNS 0x01
/*Enahnced sflow shim header len
sflow version(4B) + interface info (4B) + flags, MD (4B) + sequnece num(4B)*/
#define PRV_TGF_ENHANCED_SFLOW_SHIM_HEADER_LEN_CNS 16
/*Enahnced sflow rx_ts_shim header len
checksum(4B) + rx timestamp (6B) + port info (4B)*/
#define PRV_TGF_ENHANCED_SFLOW_RX_TS_SHIM_HEADER_LEN_CNS 14
/*enhanced sflow srcModId*/
#define PRV_TGF_ENHANCED_SFLOW_SRC_MODID_CNS 222
/*sFlow Version*/
#define PRV_TGF_SFLOW_V5_VERSION_CNS 5
/*sFlow IP Version*/
#define PRV_TGF_SFLOW_V5_IP_VERSION_CNS 1
/*Number of samples*/
#define PRV_TGF_SFLOW_V5_NUMBER_OF_SAMPLES_CNS 1
/*Output interface*/
#define PRV_TGF_SFLOW_V5_OUTPUT_INTERFACE_CNS 0
/*Header protocol*/
#define PRV_TGF_SFLOW_V5_HEADER_PROTOCOL_CNS 1
/*Stripped*/
#define PRV_TGF_SFLOW_V5_STRIPPED_CNS 4

/*Reserved eth type*/
#define PRV_TGF_SFLOW_RESERVED_ETHER_TYPE_CNS 0xFFFF

#define PRV_TGF_TTI0_INDEX_CNS prvTgfTtiTcamAbsoluteIndexWithinTheLookupHitGet(0, 3)

/*Agent IPv4 address per collector*/
static GT_U8 sflowV5AgentIpv4RandomAddrCollector1[4];

/*TTI keytype*/
static CPSS_DXCH_TTI_KEY_TYPE_ENT  keyType = CPSS_DXCH_TTI_KEY_UDB_ETHERNET_OTHER_E;
/*TTI keytype*/
static PRV_TGF_TTI_KEY_TYPE_ENT  tgfKeyType = PRV_TGF_TTI_KEY_UDB_ETHERNET_OTHER_E;
/* PCL UDB configuration */
typedef struct{
    GT_U32                               udbIndex;
    PRV_TGF_PCL_OFFSET_TYPE_ENT          offsetType;
    GT_U8                                offset;
    GT_U8                                byteMask;
}prvTgfPclUdbsflowV5;

/* parameters that are needed to be restored */
static struct
{
    PRV_TGF_MIRROR_ANALYZER_INTERFACE_STC      interface;
    PRV_TGF_BRG_EGRESS_PORT_INFO_STC           egressInfo;
    PRV_TGF_BRG_EGRESS_PORT_INFO_STC           egressInfo2;
    GT_BOOL                                    samplingEn;
    GT_U32                                     samplingRatio;
    PRV_TGF_BRG_EGRESS_PORT_INFO_STC           brgEgressInfo;
    CPSS_INTERFACE_INFO_STC                    phyPortInfo;
    CPSS_INTERFACE_INFO_STC                    phyPortInfo2;
    GT_BOOL                                    rxMirrorEnable;
    GT_BOOL                                    rxMirrorIdx;
    GT_BOOL                                    ttiEnable;
    GT_U32                                     phaThreadId;
    GT_U16                                     pvid;
    PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_ENT pclPortAccessModeCfgGet;
    CPSS_DXCH_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_ENT pclTsAccessModeCfgGet;
    PRV_TGF_TTI_OFFSET_TYPE_ENT                 ttiAnchor[3];
    GT_U8                                       ttiOffset[3];
    CPSS_DXCH_TTI_KEY_SIZE_ENT                  ttiKeySize;
    GT_BOOL                                     ttiLookupEnable;
    GT_U32                                      analyzerIndex;
    GT_U32                                      epclMetadata;
    GT_U32                                      dataFormat;
} prvTgfRestoreCfg;

static prvTgfPclUdbsflowV5 prvTgfsflowV5EpclMetadataUdbInfo[] = {

     {0 , PRV_TGF_PCL_OFFSET_METADATA_E, 53, 0xFE},   /* Analyzer ePort[0:6] */
     {1 , PRV_TGF_PCL_OFFSET_METADATA_E, 54, 0x3F},   /* Analyzer ePort[12:7] */
     {2 , PRV_TGF_PCL_OFFSET_METADATA_E, 57, 0x20},   /* Analyzer ePort[13] */
     {3 , PRV_TGF_PCL_OFFSET_METADATA_E, 64, 0x03},   /* egress marvell tagged[0:1] TO_ANALYZER */

     /* must be last */
     {4, PRV_TGF_PCL_OFFSET_INVALID_E , 0, 0}
};
static prvTgfPclUdbsflowV5 prvTgfEnhancedSflowEpclMetadataUdbInfo[] = {

     {0 , PRV_TGF_PCL_OFFSET_METADATA_E, 10, 0xFF},   /* target phy port[0:7] */
     {1 , PRV_TGF_PCL_OFFSET_METADATA_E, 11, 0x30},   /* target phy port [8:9] */
     {2 , PRV_TGF_PCL_OFFSET_METADATA_E, 53, 0xFE},   /* Analyzer ePort[0:6] */
     {3 , PRV_TGF_PCL_OFFSET_METADATA_E, 54, 0x3F},   /* Analyzer ePort[12:7] */
     {4 , PRV_TGF_PCL_OFFSET_METADATA_E, 57, 0x20},   /* Analyzer ePort[13] */
     {5 , PRV_TGF_PCL_OFFSET_METADATA_E, 64, 0x03},   /* egress marvell tagged[0:1] */
     {6 , PRV_TGF_PCL_OFFSET_METADATA_E, 85, 0x03},   /* src phy port[8:9] */
     {7 , PRV_TGF_PCL_OFFSET_METADATA_E, 16, 0xFF},   /* src phy port[0:7] */

     /* must be last */
     {8, PRV_TGF_PCL_OFFSET_INVALID_E , 0, 0}
};
/* Mirrored packet, Tunnel part */
/* L2 part */
static TGF_PACKET_L2_STC prvTgfMirroredPacketTunnelL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x44, 0x04},               /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x45, 0x05}                /* saMac */
};

/* L3 part */
/* IPv4*/
static TGF_PACKET_IPV4_STC prvTgfMirroredPacketTunnelIpv4Part = {
    4,                  /* version */
    5,                  /* headerLen */
    0,                  /* typeOfService */
    0x3f,               /* totalLen */
    0,                  /* id */
    1,                  /* flags */
    0,                  /* offset */
    0x2,                /* timeToLive */
    0x04,               /* protocol */
    0x5EA0,             /* csum */
    {20,  1,  1,  3},   /* srcAddr */
    {20,  1,  1,  2}    /* dstAddr */
};

/* IPv6 */
static TGF_PACKET_IPV6_STC prvTgfMirroredPacketTunnelIpv6Part =
{
    6,                                       /* version */
    0,                                       /* trafficClass */
    0,                                       /* flowLabel */
    0x02,                                    /* payloadLen */
    0x3b,                                    /* nextHeader */
    0x40,                                    /* hopLimit */
    {0x6545, 0, 0, 0, 0, 0, 0x0000, 0x3212}, /* TGF_IPV6_ADDR srcAddr */
    {0xff02, 0, 0, 0, 0, 0, 0xccdd, 0xeeff}  /* TGF_IPV6_ADDR dstAddr */
};

/* Original Ingress Packet */
/* L2 part */
static TGF_PACKET_L2_STC prvTgfOriginalPacketL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x02},               /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x03}                /* saMac */
};
static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanTag0Part = {
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS,                    /* etherType */
    0, 0, PRV_TGF_VLANID_CNS                            /* pri, cfi, VlanId */
};

/* DATA of packet */
static GT_U8 prvTgfPayloadDataArr[] = {
    0x66, 0x66, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05,
    0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d,
    0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15,
    0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d,
    0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25,
    0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d,
    0x2e, 0x2f, 0x30, 0x31 /* length 52 bytes */
};

/* PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart = {
    sizeof(prvTgfPayloadDataArr),                       /* dataLength */
    prvTgfPayloadDataArr                                /* dataPtr */
};

/* EtherType part (for untagged packet) = lentgh of the packet's payload */
static TGF_PACKET_ETHERTYPE_STC prvTgfPackeEthertypePart =
{
    sizeof(prvTgfPayloadDataArr)
};

/* parts of the original packet */
static TGF_PACKET_PART_STC prvTgfEnhancedSflowPacketPartsArray[] = {
     {TGF_PACKET_PART_L2_E,        &prvTgfOriginalPacketL2Part},
     {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanTag0Part},
     {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPackeEthertypePart},
     {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
 };
/* final original packet info */
static TGF_PACKET_STC prvTgfEnhancedSflowPacketInfo = {
    TGF_L2_HEADER_SIZE_CNS+TGF_ETHERTYPE_SIZE_CNS+sizeof(prvTgfPayloadDataArr), /* totalLen */
    sizeof(prvTgfEnhancedSflowPacketPartsArray) / sizeof(TGF_PACKET_PART_STC),/* numOfParts */
    prvTgfEnhancedSflowPacketPartsArray                                       /* partsArray */
};
static TGF_IPV4_ADDR agentIpAddr = {0xa,0xb,0xc,0xd};
/* Number of test packets to send */
static GT_U32 prvTgfBurstCount = 4;
/*----------------------------------------------------------------------------*/

/**
* @internal prvTgfIngresssflowV5IpTsConfigReset function
* @endinternal
*
* @brief   Tunnel start configurations reset
*           invalidate TS rule based on TS type IPv4/v6
*/
static GT_VOID prvTgfIngresssflowV5IpTsConfigReset(GT_U32 tunnelIdx, CPSS_IP_PROTOCOL_STACK_ENT protocol)
{
    GT_STATUS rc;
    PRV_TGF_TUNNEL_START_ENTRY_UNT      tunnelEntry;

    cpssOsMemSet(&tunnelEntry, 0, sizeof(tunnelEntry));

    rc = prvTgfTunnelStartEntrySet(tunnelIdx, protocol == CPSS_IP_PROTOCOL_IPV4_E? CPSS_TUNNEL_GENERIC_IPV4_E: CPSS_TUNNEL_GENERIC_IPV6_E, &tunnelEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfTunnelStartEntrySet");
}

/**
* @internal prvTgfIngresssflowV5MirrorConfigReset function
* @endinternal
*
* @brief   Ingress mirror config restore
*           Disable mirroring for rx port
*           Restore eport config, and e2phy map
*
*/
static GT_VOID prvTgfIngresssflowV5MirrorConfigReset()
{

    GT_STATUS rc;

    /* AUTODOC: Restore analyzer interface from index 1 */
    rc = prvTgfMirrorAnalyzerInterfaceSet(PRV_TGF_SFLOW_MIRROR_IDX_CNS, &prvTgfRestoreCfg.interface);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfMirrorAnalyzerInterfaceSet: %d", PRV_TGF_SFLOW_MIRROR_IDX_CNS);

    /* AUTODOC: Restore ePort attributes configuration */
    prvTgfBrgEportToPhysicalPortTargetMappingTableSet(prvTgfDevNum,
                                                       PRV_TGF_DEFAULT_EPORT_CNS, &prvTgfRestoreCfg.phyPortInfo );
    /*Restore sampling ratio*/
    rc = cpssDxChMirrorRxStatMirrorToAnalyzerIndexAndRatioSet(prvTgfDevNum, PRV_TGF_SFLOW_MIRROR_IDX_CNS, prvTgfRestoreCfg.samplingEn, prvTgfRestoreCfg.samplingRatio);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChMirrorRxStatMirrorToAnalyzerIndexAndRatioSet");

    /* AUTODOC: Restore analyzer ePort#1 attributes configuration */
    rc = prvTgfBrgEportToPhysicalPortEgressPortInfoSet(prvTgfDevNum,
                                                       PRV_TGF_DEFAULT_EPORT_CNS,
                                                       &prvTgfRestoreCfg.brgEgressInfo);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfBrgEportToPhysicalPortEgressPortInfoSet");
}

/**
* @internal prvTgfMirrorEgressEnhancedSflowEpclUDBconfig function
* @endinternal
*
* @brief   EPCL UDB config for enhanced sflow.
*          config UDB bytes and UDB select table
*/
static GT_VOID prvTgfMirrorEgressEnhancedSflowEpclUDBconfig()
{
    PRV_TGF_PCL_UDB_SELECT_STC          udbSelect;
    GT_U32                              udbSelectidx;
    GT_STATUS                           rc;

    cpssOsMemSet(&udbSelect, 0, sizeof(PRV_TGF_PCL_UDB_SELECT_STC));
    udbSelectidx = 0;

    while(1)
    {
        if(prvTgfEnhancedSflowEpclMetadataUdbInfo[udbSelectidx].offsetType == PRV_TGF_PCL_OFFSET_INVALID_E)
            break;

        /* AUTODOC:   configure User Defined Byte(UDB) */
        /* AUTODOC:   format CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_10_E */
        /* AUTODOC:   packet type PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E */
        /* AUTODOC:   offsetType PRV_TGF_PCL_OFFSET_METADATA_E */
        /* AUTODOC:   offset 53 Analyzer ePort[0:6] */
        /* AUTODOC:   offset 54 Analyzer ePort[12:7] */
        /* AUTODOC:   offset 57 Analyzer ePort[13] */
        /* AUTODOC:   offset 64 egress marvell tagged[1:0] */
        rc = prvTgfPclUserDefinedByteSet(PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E,
                                        PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E,
                                        CPSS_PCL_DIRECTION_EGRESS_E,
                                        prvTgfEnhancedSflowEpclMetadataUdbInfo[udbSelectidx].udbIndex,
                                        prvTgfEnhancedSflowEpclMetadataUdbInfo[udbSelectidx].offsetType,
                                        prvTgfEnhancedSflowEpclMetadataUdbInfo[udbSelectidx].offset);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclUserDefinedByteSet: %d for udbSelectidx = %d\n", prvTgfDevNum, udbSelectidx);

        udbSelect.udbSelectArr[udbSelectidx]=prvTgfEnhancedSflowEpclMetadataUdbInfo[udbSelectidx].udbIndex;
        udbSelectidx++;
    }

    rc = prvTgfPclUserDefinedBytesSelectSet(PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E, PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E,
                                           CPSS_PCL_LOOKUP_0_E, &udbSelect);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclUserDefinedBytesSelectSet: %d\n", prvTgfDevNum);
}
/**
* @internal prvTgfMirrorEgressEnhancedSflowEpcl1config function
* @endinternal
*
* @brief   EPCL configurations
*           Enable analyzer port for EPCL trigger
*           Enable port mode for config table
*           10B UDB.
*           keys= MD<srcPhyPort> = sample port, MD<egr mtag cmd> = FORWARD
*           Action = trigger enhanced SFLOW PHA thread to save the target details
*/
static GT_VOID prvTgfMirrorEgressEnhancedSflowEpcl1config
(
)
{
    GT_STATUS                           rc = GT_OK;
    PRV_TGF_PCL_RULE_FORMAT_UNT         mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT         pattern;
    PRV_TGF_PCL_ACTION_STC              action;
    GT_U32                              ruleIndex;
    PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT    ruleFormat;
    GT_U32                              srcPhyPort;

    PRV_UTF_LOG0_MAC("======= Setting EPCL Configuration =======\n");

    /* AUTODOC: init PCL Engine for Egress PCL for analyzer port */
    rc = prvTgfPclDefPortInit(
            prvTgfPortsArray[PRV_TGF_RX_PORT_IDX_CNS],
            CPSS_PCL_DIRECTION_EGRESS_E,
            CPSS_PCL_LOOKUP_0_E,
            PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E,
            PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E,
            PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfPclDefPortInit");

    /*trigger mirror for original egress packet.
     * It can be with or without TS*/
    rc = prvTgfPclEgressPclPacketTypesSet(prvTgfDevNum,
                                          prvTgfPortsArray[PRV_TGF_RX_PORT_IDX_CNS],
                                          PRV_TGF_PCL_EGRESS_PKT_TS_E, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfPclEgressPclPacketTypesSet");
    rc = prvTgfPclEgressPclPacketTypesSet(prvTgfDevNum,
                                          prvTgfPortsArray[PRV_TGF_RX_PORT_IDX_CNS],
                                          PRV_TGF_PCL_EGRESS_PKT_NON_TS_E, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfPclEgressPclPacketTypesSet");

    /* AUTODOC: Configure ePort access mode for EPCL lookup */
    rc = prvTgfPclPortLookupCfgTabAccessModeGet(prvTgfPortsArray[PRV_TGF_RX_PORT_IDX_CNS],
                           CPSS_PCL_DIRECTION_EGRESS_E, CPSS_PCL_LOOKUP_0_E, 0,
                           &prvTgfRestoreCfg.pclPortAccessModeCfgGet);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclPortLookupCfgTabAccessModeGet");

    rc = prvTgfPclPortLookupCfgTabAccessModeSet(prvTgfPortsArray[PRV_TGF_RX_PORT_IDX_CNS],
                           CPSS_PCL_DIRECTION_EGRESS_E, CPSS_PCL_LOOKUP_0_E, 0,
                           PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclPortLookupCfgTabAccessModeSet");

    /* AUTODOC: set PCL rule 0 */
    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));
    cpssOsMemSet(&action, 0, sizeof(action));

    srcPhyPort = prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS];
    PRV_UTF_LOG1_MAC( "=== srcPhyPort: 0x%x\n", srcPhyPort);
    /* bits 0:7 is trg port[0:7] */
    pattern.ruleEgrUdbOnly.udb[0] = 0x0;
    mask.ruleEgrUdbOnly.udb[0] = 0x0;

    /* bits 4:5 is trg port[8:9] */
    pattern.ruleEgrUdbOnly.udb[1] = 0x0;
    mask.ruleEgrUdbOnly.udb[1] = 0x0;

    /*ignore the analyzer eport */
    pattern.ruleEgrUdbOnly.udb[2] = 0x00;
    mask.ruleEgrUdbOnly.udb[2] = 0x00;

    pattern.ruleEgrUdbOnly.udb[3] = 0x00;
    mask.ruleEgrUdbOnly.udb[3] = 0x00;

    pattern.ruleEgrUdbOnly.udb[4] = 0x00;
    mask.ruleEgrUdbOnly.udb[4] = 0x00;

    /*egress pkt cmd = FORWARD*/
    pattern.ruleEgrUdbOnly.udb[5] = 0x03;
    mask.ruleEgrUdbOnly.udb[5] = prvTgfEnhancedSflowEpclMetadataUdbInfo[5].byteMask;

    /*src phy port [8:9] of the original packet*/
    pattern.ruleEgrUdbOnly.udb[6] = (srcPhyPort >> 8) & 0x3;
    mask.ruleEgrUdbOnly.udb[6] = prvTgfEnhancedSflowEpclMetadataUdbInfo[6].byteMask;

    /*src phy port [0:7] of the original packet*/
    pattern.ruleEgrUdbOnly.udb[7] = srcPhyPort & 0xFF;
    mask.ruleEgrUdbOnly.udb[7] = prvTgfEnhancedSflowEpclMetadataUdbInfo[7].byteMask;

    ruleIndex                                    = PRV_TGF_ENHANCED_SFLOW_SAVE_TARGET_EPCL_IDX_CNS;
    ruleFormat                                   = PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E;
    action.pktCmd                                = CPSS_PACKET_CMD_FORWARD_E;
    action.oam.oamProcessEnable                  = GT_FALSE;
    action.flowId                                = PRV_TGF_SFLOW_MIRROR_IDX_CNS;
    action.policer.policerEnable                 = PRV_TGF_PCL_POLICER_DISABLE_ALL_E;
    action.egressPolicy                          = GT_TRUE;
    action.mirror.mirrorToTxAnalyzerPortEn       = GT_TRUE;
    action.mirror.egressMirrorToAnalyzerIndex    = PRV_TGF_SFLOW_MIRROR_IDX_CNS;
    action.epclPhaInfo.phaThreadIdAssignmentMode = PRV_TGF_PCL_ACTION_EGRESS_PHA_THREAD_ID_ASSIGNMENT_MODE_ENABLED_E;
    action.epclPhaInfo.phaThreadId               = PRV_TGF_PHA_THREAD_ID_ENHANCED_SFLOW_SAVE_TARGET_DEV_CNS ;
    rc = prvTgfPclRuleSet(ruleFormat, ruleIndex, &mask, &pattern, &action);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPclRuleSet, prvTgfDevNum:%d,"
                                            "ruleFormat:%d, ruleIndex:%d", prvTgfDevNum, ruleFormat, ruleIndex);
}
/**
* @internal prvTgfMirrorEgressEnhancedSflowEpcl2config function
* @endinternal
*
* @brief   EPCL configurations
*           Enable analyzer port for EPCL trigger
*           Enable port mode for config table
*           10B UDB keys with eport, egr mtag cmd = TO_ANALYZER
*           Action = trigger enhanced SFLOW PHA thread,
*/
static GT_VOID prvTgfMirrorEgressEnhancedSflowEpcl2config
(
)
{
    GT_STATUS                           rc = GT_OK;
    PRV_TGF_PCL_RULE_FORMAT_UNT         mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT         pattern;
    PRV_TGF_PCL_ACTION_STC              action;
    GT_U32                              ruleIndex;
    PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT    ruleFormat;

    PRV_UTF_LOG0_MAC("======= Setting EPCL Configuration =======\n");

    /* AUTODOC: init PCL Engine for Egress PCL for analyzer port */
    rc = prvTgfPclDefPortInit(
            prvTgfPortsArray[PRV_TGF_ANALYZER_PORT_IDX_CNS],
            CPSS_PCL_DIRECTION_EGRESS_E,
            CPSS_PCL_LOOKUP_0_E,
            PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E,
            PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E,
            PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfPclDefPortInit");

    /* AUTODOC: enable EPCL on Analyzer port */
    rc = prvTgfPclEgressPclPacketTypesSet(prvTgfDevNum,
                                          prvTgfPortsArray[PRV_TGF_ANALYZER_PORT_IDX_CNS],
                                          PRV_TGF_PCL_EGRESS_PKT_TO_ANALYZER_E, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfPclEgressPclPacketTypesSet");
    rc = prvTgfPclEgressPclPacketTypesSet(prvTgfDevNum,
                                          prvTgfPortsArray[PRV_TGF_ANALYZER_PORT_IDX_CNS],
                                          PRV_TGF_PCL_EGRESS_PKT_TS_E, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfPclEgressPclPacketTypesSet");
    rc = prvTgfPclEgressPclPacketTypesSet(prvTgfDevNum,
                                          prvTgfPortsArray[PRV_TGF_ANALYZER_PORT_IDX_CNS],
                                          PRV_TGF_PCL_EGRESS_PKT_NON_TS_E, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfPclEgressPclPacketTypesSet");

    /* AUTODOC: Configure ePort access mode for EPCL lookup */
    rc = prvTgfPclPortLookupCfgTabAccessModeGet(prvTgfPortsArray[PRV_TGF_ANALYZER_PORT_IDX_CNS],
                           CPSS_PCL_DIRECTION_EGRESS_E, CPSS_PCL_LOOKUP_0_E, 0,
                           &prvTgfRestoreCfg.pclPortAccessModeCfgGet);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclPortLookupCfgTabAccessModeGet");

    rc = prvTgfPclPortLookupCfgTabAccessModeSet(prvTgfPortsArray[PRV_TGF_ANALYZER_PORT_IDX_CNS],
                           CPSS_PCL_DIRECTION_EGRESS_E, CPSS_PCL_LOOKUP_0_E, 0,
                           PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclPortLookupCfgTabAccessModeSet");

    /* AUTODOC: Configure "Access to the EPCL configuration" table with Port Mode */
    rc = cpssDxChPclEgressTunnelStartPacketsCfgTabAccessModeGet(prvTgfDevNum,
                                &prvTgfRestoreCfg.pclTsAccessModeCfgGet);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPclEgressTunnelStartPacketsCfgTabAccessModeSet");

    rc = cpssDxChPclEgressTunnelStartPacketsCfgTabAccessModeSet(prvTgfDevNum,
                                CPSS_DXCH_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPclEgressTunnelStartPacketsCfgTabAccessModeSet");

    /* AUTODOC: set PCL rule 0 */
    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));
    cpssOsMemSet(&action, 0, sizeof(action));

    /*ignore the target port */
    pattern.ruleEgrUdbOnly.udb[0] = 0x00;
    mask.ruleEgrUdbOnly.udb[0] = 0x00;

    pattern.ruleEgrUdbOnly.udb[1] = 0x00;
    mask.ruleEgrUdbOnly.udb[1] = 0x00;

    /* bits 1:7 is ePort[0:6] */
    pattern.ruleEgrUdbOnly.udb[2] = (PRV_TGF_DEFAULT_EPORT_CNS&0x7f) <<1;
    mask.ruleEgrUdbOnly.udb[2] = prvTgfEnhancedSflowEpclMetadataUdbInfo[2].byteMask;

    /* bits 0:5 is ePort[12:7] */
    pattern.ruleEgrUdbOnly.udb[3] = ((PRV_TGF_DEFAULT_EPORT_CNS>>7)&0x3f);
    mask.ruleEgrUdbOnly.udb[3] = prvTgfEnhancedSflowEpclMetadataUdbInfo[3].byteMask;

    /* bits 5 is ePort[13] */
    pattern.ruleEgrUdbOnly.udb[4] = (PRV_TGF_DEFAULT_EPORT_CNS>>8)&0x20;
    mask.ruleEgrUdbOnly.udb[4] = prvTgfEnhancedSflowEpclMetadataUdbInfo[4].byteMask;

    /*egress pkt cmd = MIRROR*/
    pattern.ruleEgrUdbOnly.udb[5] = 0x02;
    mask.ruleEgrUdbOnly.udb[5] = prvTgfEnhancedSflowEpclMetadataUdbInfo[5].byteMask;

    /*ignore the src port */
    pattern.ruleEgrUdbOnly.udb[6] = 0x00;
    mask.ruleEgrUdbOnly.udb[6] = 0x00;

    pattern.ruleEgrUdbOnly.udb[7] = 0x00;
    mask.ruleEgrUdbOnly.udb[7] = 0x00;

    ruleIndex                                    = PRV_TGF_ENHANCED_SFLOW_EPCL_IDX_CNS;
    ruleFormat                                   = PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E;
    action.pktCmd                                = CPSS_PACKET_CMD_FORWARD_E;
    action.oam.oamProcessEnable                  = GT_FALSE;
    action.flowId                                = PRV_TGF_SFLOW_MIRROR_IDX_CNS;
    action.policer.policerEnable                 = PRV_TGF_PCL_POLICER_DISABLE_ALL_E;
    action.egressPolicy                          = GT_TRUE;
    action.epclPhaInfo.phaThreadIdAssignmentMode = PRV_TGF_PCL_ACTION_EGRESS_PHA_THREAD_ID_ASSIGNMENT_MODE_ENABLED_E;
    action.epclPhaInfo.phaThreadId               = PRV_TGF_PHA_THREAD_ID_ENHANCED_SFLOW_CNS;
    action.epclPhaInfo.phaThreadType             = 0;
    rc = prvTgfPclRuleSet(ruleFormat, ruleIndex, &mask, &pattern, &action);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPclRuleSet, prvTgfDevNum:%d,"
                                            "ruleFormat:%d, ruleIndex:%d", prvTgfDevNum, ruleFormat, ruleIndex);
}
/**
* @internal prvTgfMirrorEgressEnhancedSflowEpcl3config function
* @endinternal
*
* @brief   EPCL configurations
*           Enable analyzer port for EPCL trigger
*           Enable port mode for config table
*           10B UDB keys with eport, egr mtag cmd = TO_ANALYZER
*           Action = trigger enhanced SFLOW PHA thread per IP protocol,
*/
static GT_VOID prvTgfMirrorEgressEnhancedSflowEpcl3config
(
    GT_U32 protocol
)
{
    GT_STATUS                           rc = GT_OK;
    PRV_TGF_PCL_RULE_FORMAT_UNT         mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT         pattern;
    PRV_TGF_PCL_ACTION_STC              action;
    GT_U32                              ruleIndex;
    PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT    ruleFormat;
    GT_U32                              targetPhyPort;

    PRV_UTF_LOG0_MAC("======= Setting EPCL Configuration =======\n");

    /* AUTODOC: init PCL Engine for Egress PCL for analyzer port */
    rc = prvTgfPclDefPortInit(
            prvTgfPortsArray[PRV_TGF_ANALYZER_PORT_IDX_CNS],
            CPSS_PCL_DIRECTION_EGRESS_E,
            CPSS_PCL_LOOKUP_0_E,
            PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E,
            PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E,
            PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfPclDefPortInit");

    /* AUTODOC: enable EPCL on Analyzer port */
    rc = prvTgfPclEgressPclPacketTypesSet(prvTgfDevNum,
                                          prvTgfPortsArray[PRV_TGF_ANALYZER_PORT_IDX_CNS],
                                          PRV_TGF_PCL_EGRESS_PKT_TO_ANALYZER_E, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfPclEgressPclPacketTypesSet");
    rc = prvTgfPclEgressPclPacketTypesSet(prvTgfDevNum,
                                          prvTgfPortsArray[PRV_TGF_ANALYZER_PORT_IDX_CNS],
                                          PRV_TGF_PCL_EGRESS_PKT_TS_E, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfPclEgressPclPacketTypesSet");

    /* AUTODOC: Configure ePort access mode for EPCL lookup */
    rc = prvTgfPclPortLookupCfgTabAccessModeGet(prvTgfPortsArray[PRV_TGF_ANALYZER_PORT_IDX_CNS],
                           CPSS_PCL_DIRECTION_EGRESS_E, CPSS_PCL_LOOKUP_0_E, 0,
                           &prvTgfRestoreCfg.pclPortAccessModeCfgGet);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclPortLookupCfgTabAccessModeGet");

    rc = prvTgfPclPortLookupCfgTabAccessModeSet(prvTgfPortsArray[PRV_TGF_ANALYZER_PORT_IDX_CNS],
                           CPSS_PCL_DIRECTION_EGRESS_E, CPSS_PCL_LOOKUP_0_E, 0,
                           PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclPortLookupCfgTabAccessModeSet");

    /* AUTODOC: Configure "Access to the EPCL configuration" table with Port Mode */
    rc = cpssDxChPclEgressTunnelStartPacketsCfgTabAccessModeGet(prvTgfDevNum,
                                &prvTgfRestoreCfg.pclTsAccessModeCfgGet);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPclEgressTunnelStartPacketsCfgTabAccessModeSet");

    rc = cpssDxChPclEgressTunnelStartPacketsCfgTabAccessModeSet(prvTgfDevNum,
                                CPSS_DXCH_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPclEgressTunnelStartPacketsCfgTabAccessModeSet");

    /* AUTODOC: set PCL rule 3 */
    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));
    cpssOsMemSet(&action, 0, sizeof(action));

    targetPhyPort = prvTgfPortsArray[PRV_TGF_ANALYZER_PORT_IDX_CNS];
    PRV_UTF_LOG1_MAC("Enhanced sflow: LB targetPhyPort: 0x%x\n", targetPhyPort);
    /* bits 0:7 is trg port[0:7] */
    pattern.ruleEgrUdbOnly.udb[0] = (targetPhyPort&0xFF);
    mask.ruleEgrUdbOnly.udb[0] = prvTgfEnhancedSflowEpclMetadataUdbInfo[0].byteMask;

    /* bits 4:5 is trg port[8:9] */
    pattern.ruleEgrUdbOnly.udb[1] = ((targetPhyPort>>8)&0x3) << 4;
    mask.ruleEgrUdbOnly.udb[1] = prvTgfEnhancedSflowEpclMetadataUdbInfo[1].byteMask;

    /*ignore the analyzer eport */
    pattern.ruleEgrUdbOnly.udb[2] = 0x00;
    mask.ruleEgrUdbOnly.udb[2] = 0x00;

    pattern.ruleEgrUdbOnly.udb[3] = 0x00;
    mask.ruleEgrUdbOnly.udb[3] = 0x00;

    pattern.ruleEgrUdbOnly.udb[4] = 0x00;
    mask.ruleEgrUdbOnly.udb[4] = 0x00;

    /*egress pkt cmd = FORWARD*/
    pattern.ruleEgrUdbOnly.udb[5] = 0x03;
    mask.ruleEgrUdbOnly.udb[5] = prvTgfEnhancedSflowEpclMetadataUdbInfo[5].byteMask;

    /*ignore the src port */
    pattern.ruleEgrUdbOnly.udb[6] = 0x00;
    mask.ruleEgrUdbOnly.udb[6] = 0x00;

    pattern.ruleEgrUdbOnly.udb[7] = 0x00;
    mask.ruleEgrUdbOnly.udb[7] = 0x00;

    ruleIndex                                    = protocol == CPSS_IP_PROTOCOL_IPV4_E ?
                                                    PRV_TGF_ENHANCED_SFLOW_IPV4_EPCL_IDX_CNS :
                                                    PRV_TGF_ENHANCED_SFLOW_IPV6_EPCL_IDX_CNS;
    ruleFormat                                   = PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E;
    action.pktCmd                                = CPSS_PACKET_CMD_FORWARD_E;
    action.oam.oamProcessEnable                  = GT_FALSE;
    action.flowId                                = PRV_TGF_SFLOW_MIRROR_IDX_CNS;
    action.policer.policerEnable                 = PRV_TGF_PCL_POLICER_DISABLE_ALL_E;
    action.egressPolicy                          = GT_TRUE;
    action.epclPhaInfo.phaThreadIdAssignmentMode = PRV_TGF_PCL_ACTION_EGRESS_PHA_THREAD_ID_ASSIGNMENT_MODE_ENABLED_E;
    action.epclPhaInfo.phaThreadId               = protocol == CPSS_IP_PROTOCOL_IPV4_E ?
                                                    PRV_TGF_PHA_THREAD_ID_ENHANCED_SFLOW_IPV4_CNS :
                                                    PRV_TGF_PHA_THREAD_ID_ENHANCED_SFLOW_IPV6_CNS ;
    action.epclPhaInfo.phaThreadType             = PRV_TGF_PCL_ACTION_EGRESS_PHA_THREAD_ENHANCED_SFLOW_E;
    action.epclPhaInfo.phaThreadUnion.epclEnhancedSflow.agentIpAddr.arIP[0]             = agentIpAddr[0];
    action.epclPhaInfo.phaThreadUnion.epclEnhancedSflow.agentIpAddr.arIP[1]             = agentIpAddr[1];
    action.epclPhaInfo.phaThreadUnion.epclEnhancedSflow.agentIpAddr.arIP[2]             = agentIpAddr[2];
    action.epclPhaInfo.phaThreadUnion.epclEnhancedSflow.agentIpAddr.arIP[3]             = agentIpAddr[3];
    rc = prvTgfPclRuleSet(ruleFormat, ruleIndex, &mask, &pattern, &action);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPclRuleSet, prvTgfDevNum:%d,"
                                            "ruleFormat:%d, ruleIndex:%d", prvTgfDevNum, ruleFormat, ruleIndex);
}
/**
* @internal prvTgfMirrorEgressEnhancedSflowPhaConfig function
* @endinternal
*
* @brief   PHA thread config
*           Set the FW thread ids 61, 62, 63 with enhanced SFLOW saveTarget/IPv4/v6 thread id
*           Set the IPv4/v6 thread memory with target dev id
*
*/
static GT_VOID prvTgfMirrorEgressEnhancedSflowPhaConfig()
{
    GT_STATUS rc;

    /* AUTODOC: Set the thread entry */
    CPSS_DXCH_PHA_THREAD_INFO_UNT            extInfo;
    CPSS_DXCH_PHA_COMMON_THREAD_INFO_STC     commonInfo;

    /*load FW image 01*/
    prvTgfPhaFWLoad(CPSS_DXCH_PHA_FW_IMAGE_ID_01_E);

    cpssOsMemSet(&commonInfo, 0, sizeof(commonInfo));
    cpssOsMemSet(&extInfo, 0, sizeof(extInfo));
    commonInfo.statisticalProcessingFactor = 0;
    commonInfo.busyStallMode = CPSS_DXCH_PHA_BUSY_STALL_MODE_ORDERED_BYPASS_E;
    commonInfo.stallDropCode = CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E;

    /*config the CPSS_DXCH_PHA_THREAD_TYPE_ENHANCED_SFLOW_SAVE_TARGET_PORT_E*/
    rc = cpssDxChPhaThreadIdEntrySet(prvTgfDevNum,
                PRV_TGF_PHA_THREAD_ID_ENHANCED_SFLOW_SAVE_TARGET_DEV_CNS,
                &commonInfo,
                CPSS_DXCH_PHA_THREAD_TYPE_ENHANCED_SFLOW_SAVE_TARGET_PORT_E,
                &extInfo);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPhaThreadIdEntrySet enhanced sflow save_src_mod_id");

    /*config the CPSS_DXCH_PHA_THREAD_TYPE_ENHANCED_SFLOW_FILL_REMAIN_IPV4_E */
    rc = cpssDxChPhaThreadIdEntrySet(prvTgfDevNum,
                PRV_TGF_PHA_THREAD_ID_ENHANCED_SFLOW_IPV4_CNS,
                &commonInfo,
                CPSS_DXCH_PHA_THREAD_TYPE_ENHANCED_SFLOW_FILL_REMAIN_IPV4_E,
                &extInfo);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPhaThreadIdEntrySet enhanced sflow ipv4");
    /*config the CPSS_DXCH_PHA_THREAD_TYPE_ENHANCED_SFLOW_FILL_REMAIN_IPV6_E */
    rc = cpssDxChPhaThreadIdEntrySet(prvTgfDevNum,
                PRV_TGF_PHA_THREAD_ID_ENHANCED_SFLOW_IPV6_CNS,
                &commonInfo,
                CPSS_DXCH_PHA_THREAD_TYPE_ENHANCED_SFLOW_FILL_REMAIN_IPV6_E,
                &extInfo);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPhaThreadIdEntrySet enhanced sflow ipv6");
    /*config the CPSS_DXCH_PHA_THREAD_TYPE_ENHANCED_SFLOW_E */
    extInfo.enhancedSflow.srcModId = PRV_TGF_ENHANCED_SFLOW_SRC_MODID_CNS;
    rc = cpssDxChPhaThreadIdEntrySet(prvTgfDevNum,
                PRV_TGF_PHA_THREAD_ID_ENHANCED_SFLOW_CNS,
                &commonInfo,
                CPSS_DXCH_PHA_THREAD_TYPE_ENHANCED_SFLOW_E,
                &extInfo);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPhaThreadIdEntrySet enhanced sflow common");
}
/**
* @internal prvTgfEnahncedSflowMirrorConfig function
* @endinternal
*
* @brief   Ingress mirror config
*           Enable source port mirroring for RX port and Set analyzer index to 1
*           map analyzer index to eport, and set sampling ratio
*           map eport to phy port#2 and enable TS for the eport
*
*/
static GT_VOID prvTgfEnahncedSflowMirrorConfig()
{

    GT_STATUS rc;
    PRV_TGF_MIRROR_ANALYZER_INTERFACE_STC interface;
    CPSS_INTERFACE_INFO_STC phyPortInfo;

    /* AUTODOC: Save analyzer interface */
    rc = prvTgfMirrorAnalyzerInterfaceGet(prvTgfDevNum, PRV_TGF_SFLOW_MIRROR_IDX_CNS, &prvTgfRestoreCfg.interface);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfMirrorAnalyzerInterfaceGet: %d", prvTgfDevNum);

    /* AUTODOC: set analyzer interface index=1: */
    /* AUTODOC:   analyzer devNum=0, eport = 1024 , phy port=2 */
    interface.interface.type = CPSS_INTERFACE_PORT_E;
    interface.interface.devPort.hwDevNum  = prvTgfDevNum;
    interface.interface.devPort.portNum = PRV_TGF_DEFAULT_EPORT_CNS;
    rc = prvTgfMirrorAnalyzerInterfaceSet(PRV_TGF_SFLOW_MIRROR_IDX_CNS, &interface);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfMirrorAnalyzerInterfaceSet: %d", PRV_TGF_SFLOW_MIRROR_IDX_CNS);

    /* AUTODOC: Save ePort to phyport configuration */
    rc = prvTgfBrgEportToPhysicalPortTargetMappingTableGet(prvTgfDevNum,
                                                      PRV_TGF_DEFAULT_EPORT_CNS,
                                                       &(prvTgfRestoreCfg.phyPortInfo));
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfBrgEportToPhysicalPortTargetMappingTableGet");
    phyPortInfo.type = CPSS_INTERFACE_PORT_E;
    phyPortInfo.devPort.hwDevNum = prvTgfDevNum;
    phyPortInfo.devPort.portNum = prvTgfPortsArray[PRV_TGF_LB_PORT_IDX_CNS];
    prvTgfBrgEportToPhysicalPortTargetMappingTableSet(prvTgfDevNum,
                                                       PRV_TGF_DEFAULT_EPORT_CNS, &phyPortInfo );
    /*Set sampling ratio*/
    rc = cpssDxChMirrorRxStatMirrorToAnalyzerIndexAndRatioGet(prvTgfDevNum, PRV_TGF_SFLOW_MIRROR_IDX_CNS, &prvTgfRestoreCfg.samplingEn, &prvTgfRestoreCfg.samplingRatio);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChMirrorRxStatMirrorToAnalyzerIndexAndRatioGet");
    rc = cpssDxChMirrorRxStatMirrorToAnalyzerIndexAndRatioSet(prvTgfDevNum, PRV_TGF_SFLOW_MIRROR_IDX_CNS, GT_TRUE, PRV_TGF_SFLOW_MIRROR_RATE_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChMirrorRxStatMirrorToAnalyzerIndexAndRatioSet");

}
/**
* @internal prvTgfEnhancedSflowMirrorConfig function
* @endinternal
*
* @brief   eport config for the pass #2
*           e2phy + TS
*
*/
static GT_VOID prvTgfEnhancedSflowMirrorConfig(CPSS_IP_PROTOCOL_STACK_ENT protocol)
{
    GT_STATUS rc;
    CPSS_INTERFACE_INFO_STC phyPortInfo;
    PRV_TGF_BRG_EGRESS_PORT_INFO_STC     egressInfo;
    /* AUTODOC: Save ePort to phyport configuration */
    rc = prvTgfBrgEportToPhysicalPortTargetMappingTableGet(prvTgfDevNum,
                                                      PRV_TGF_PASS2_EPORT_CNS,
                                                       &(prvTgfRestoreCfg.phyPortInfo2));
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfBrgEportToPhysicalPortTargetMappingTableGet");
    phyPortInfo.type = CPSS_INTERFACE_PORT_E;
    phyPortInfo.devPort.hwDevNum = prvTgfDevNum;
    phyPortInfo.devPort.portNum = prvTgfPortsArray[PRV_TGF_ANALYZER_PORT_IDX_CNS];
    prvTgfBrgEportToPhysicalPortTargetMappingTableSet(prvTgfDevNum,
                                                       PRV_TGF_PASS2_EPORT_CNS, &phyPortInfo );
    /* AUTODOC: Set analyzer ePort#1 attributes configuration */
    /* AUTODOC: Tunnel Start = ENABLE, tunnelStartPtr = 8, tsPassenger = ETHERNET */
    /*Get eport attribute config*/
    rc = prvTgfBrgEportToPhysicalPortEgressPortInfoGet(prvTgfDevNum,
                                                       PRV_TGF_PASS2_EPORT_CNS,
                                                       &prvTgfRestoreCfg.egressInfo2);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfBrgEportToPhysicalPortEgressPortInfoGet");
    cpssOsMemSet(&egressInfo,0,sizeof(egressInfo));
    egressInfo = prvTgfRestoreCfg.egressInfo;
    egressInfo.tunnelStart            = GT_TRUE;
    egressInfo.tunnelStartPtr         = protocol == CPSS_IP_PROTOCOL_IPV4_E? PRV_TGF_TUNNEL_START_IPV4_INDEX_CNS : PRV_TGF_TUNNEL_START_IPV6_INDEX_CNS;
    egressInfo.tsPassengerPacketType  = PRV_TGF_TUNNEL_PASSENGER_ETHERNET_E;
    rc = prvTgfBrgEportToPhysicalPortEgressPortInfoSet(prvTgfDevNum,
                                                       PRV_TGF_PASS2_EPORT_CNS,
                                                       &egressInfo);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfBrgEportToPhysicalPortEgressPortInfoSet");
}

/**
* @internal prvTgfMirrorEgressEnhancedSflowTtiConfig function
* @endinternal
*
* @brief   Enhanced Sflow pass #2.
*          TTI config to redirect partially built sflow packet to PHA
*               Enable TTI lookup for LB
*               Create UDB size 10 for ETH_Other,
*                   key : sflow ver, src mod id, src port
*                   action: redirect to eport, bridge bypass
*/
static GT_VOID prvTgfMirrorEgressEnhancedSflowTtiConfig(GT_VOID)
{
    GT_STATUS                rc = GT_OK;
    PRV_TGF_TTI_ACTION_STC   ttiAction;
    PRV_TGF_TTI_RULE_UNT     ttiPattern;
    PRV_TGF_TTI_RULE_UNT     ttiMask;

    /* AUTODOC: clear pattern and mask */
    cpssOsMemSet((GT_VOID*) &ttiPattern, 0, sizeof(ttiPattern));
    cpssOsMemSet((GT_VOID*) &ttiMask,    0, sizeof(ttiMask));
    cpssOsMemSet((GT_VOID*) &ttiAction, 0, sizeof(ttiAction));

    PRV_UTF_LOG0_MAC("======= Setting TTI Configuration =======\n");

    /* AUTODOC: set UDB byte for the key keyType:
     * anchor L2 header
     * offset:3 = sflow version
     * offset:4 = src mod id
     * offset:5 = src port*/
    rc = prvTgfTtiUserDefinedByteGet(prvTgfDevNum, tgfKeyType, 0, &prvTgfRestoreCfg.ttiAnchor[0], &prvTgfRestoreCfg.ttiOffset[0]);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiUserDefinedByteGet: %d", prvTgfDevNum);
    rc = prvTgfTtiUserDefinedByteGet(prvTgfDevNum, tgfKeyType, 1, &prvTgfRestoreCfg.ttiAnchor[1], &prvTgfRestoreCfg.ttiOffset[1]);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiUserDefinedByteGet: %d", prvTgfDevNum);
    rc = prvTgfTtiUserDefinedByteGet(prvTgfDevNum, tgfKeyType, 2, &prvTgfRestoreCfg.ttiAnchor[2], &prvTgfRestoreCfg.ttiOffset[2]);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiUserDefinedByteGet: %d", prvTgfDevNum);

    rc = prvTgfTtiUserDefinedByteSet(prvTgfDevNum, tgfKeyType, 0, PRV_TGF_TTI_OFFSET_L2_E, 3);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiUserDefinedByteSet: %d", prvTgfDevNum);
    rc = prvTgfTtiUserDefinedByteSet(prvTgfDevNum, tgfKeyType, 1, PRV_TGF_TTI_OFFSET_L2_E, 4);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiUserDefinedByteSet: %d", prvTgfDevNum);
    rc = prvTgfTtiUserDefinedByteSet(prvTgfDevNum, tgfKeyType, 2, PRV_TGF_TTI_OFFSET_L2_E, 5);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiUserDefinedByteSet: %d", prvTgfDevNum);


    /* AUTODOC: set the TTI Rule Pattern and Mask to match
        reserved Ether type 0xFFFF*/
    ttiPattern.udbArray.udb[0] = 0x5;
    ttiMask.udbArray.udb[0]    = 0xFF;
    ttiPattern.udbArray.udb[1] = PRV_TGF_ENHANCED_SFLOW_SRC_MODID_CNS & 0xFF;
    ttiMask.udbArray.udb[1]    = 0xFF;
    ttiPattern.udbArray.udb[2] = prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS] & 0xFF;
    ttiMask.udbArray.udb[2]    = 0xFF;

    /* AUTODOC: set the TTI Rule Action for matched packet */
    ttiAction.interfaceInfo.type  = CPSS_INTERFACE_PORT_E;
    ttiAction.interfaceInfo.devPort.hwDevNum  = prvTgfDevNum;
    ttiAction.interfaceInfo.devPort.portNum  = PRV_TGF_PASS2_EPORT_CNS;
    ttiAction.bridgeBypass        = GT_TRUE;
    ttiAction.ingressPipeBypass        = GT_TRUE;
    ttiAction.redirectCommand     = PRV_TGF_TTI_REDIRECT_TO_EGRESS_E;

    /* AUTODOC: enable the TTI lookup for keyType at the ingress remote port */
    rc = cpssDxChTtiPortLookupEnableGet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_LB_PORT_IDX_CNS], keyType, &prvTgfRestoreCfg.ttiEnable);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChTtiPortLookupEnableGet");

    rc = cpssDxChTtiPortLookupEnableSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_LB_PORT_IDX_CNS], keyType, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChTtiPortLookupEnableSet");

    /* AUTODOC: set key size TTI_KEY_SIZE_10_B_E for the key keyType */
    rc = cpssDxChTtiPacketTypeKeySizeGet(prvTgfDevNum, keyType, &prvTgfRestoreCfg.ttiKeySize);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPacketTypeKeySizeGet: %d", prvTgfDevNum);

    rc = cpssDxChTtiPacketTypeKeySizeSet(prvTgfDevNum, keyType, CPSS_DXCH_TTI_KEY_SIZE_10_B_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPacketTypeKeySizeSet: %d", prvTgfDevNum);

    /* AUTODOC: add TTI rule 1 */
    rc = prvTgfTtiRuleSet(PRV_TGF_TTI0_INDEX_CNS, PRV_TGF_TTI_RULE_UDB_10_E, &ttiPattern, &ttiMask, &ttiAction);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiRule2Set");
}
/**
* @internal prvTgfMirrorEgressEnhancedSflowLBConfig function
* @endinternal
*
* @brief   LB config for the service port.
*
*/
static GT_VOID prvTgfMirrorEgressEnhancedSflowLBConfig()
{

    CPSS_INTERFACE_INFO_STC  portInterface;
    GT_STATUS rc;

    cpssOsMemSet(&portInterface, 0, sizeof(portInterface));

    portInterface.type            = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_LB_PORT_IDX_CNS];

    /* set loopback mode on port */
    rc = tgfTrafficGeneratorPortLoopbackModeEnableSet(&portInterface, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Failure in tgfTrafficGeneratorPortLoopbackModeEnableSet port %d",prvTgfPortsArray[PRV_TGF_LB_PORT_IDX_CNS]);
    /*give port mgr time to config the LB*/
    cpssOsTimerWkAfter(10);
}
/**
* @internal prvTgfIngresssflowV5Ipv4TsConfig function
* @endinternal
*
* @brief   TS configurations for IPv4 tunnel
*           Set Tunnel Start entry 8 with:
*           tunnelType = Generic IPv4, tagEnable = TRUE, vlanId=5, ipHeaderProtocol = UDP, udp port = 6343
*           MACDA = 00:00:00:00:44:04 , DIP = 20.1.1.2, SIP=20.1.1.3
*           Tunnel start profile with 16B zeros
*
*/
static GT_VOID prvTgfEnhancedSflowIpTsConfig(CPSS_IP_PROTOCOL_STACK_ENT protocol)
{
    GT_STATUS rc;
    PRV_TGF_TUNNEL_START_ENTRY_UNT      tunnelEntry;

    cpssOsMemSet(&tunnelEntry, 0, sizeof(tunnelEntry));
    if (protocol == CPSS_IP_PROTOCOL_IPV4_E)
    {
        tunnelEntry.ipv4Cfg.tagEnable              = GT_TRUE;
        tunnelEntry.ipv4Cfg.vlanId                 = PRV_TGF_VLANID_CNS;
        tunnelEntry.ipv4Cfg.ttl                    = 20;
        tunnelEntry.ipv4Cfg.ipHeaderProtocol       = PRV_TGF_TUNNEL_START_IP_HEADER_PROTOCOL_UDP_E;
        tunnelEntry.ipv4Cfg.udpDstPort             = PRV_TGF_SFLOW_V5_UDP_PORT_CNS;
        tunnelEntry.ipv4Cfg.udpSrcPort             = PRV_TGF_SFLOW_V5_UDP_PORT_CNS;
        cpssOsMemCpy(tunnelEntry.ipv4Cfg.macDa.arEther, prvTgfMirroredPacketTunnelL2Part.daMac,    sizeof(TGF_MAC_ADDR));
        cpssOsMemCpy(tunnelEntry.ipv4Cfg.destIp.arIP, prvTgfMirroredPacketTunnelIpv4Part.dstAddr, sizeof(TGF_IPV4_ADDR));
        cpssOsMemCpy(tunnelEntry.ipv4Cfg.srcIp.arIP, prvTgfMirroredPacketTunnelIpv4Part.srcAddr, sizeof(TGF_IPV4_ADDR));
        rc = prvTgfTunnelStartEntrySet(PRV_TGF_TUNNEL_START_IPV4_INDEX_CNS, CPSS_TUNNEL_GENERIC_IPV4_E, &tunnelEntry);
    }
    else
    {
        tunnelEntry.ipv6Cfg.tagEnable              = GT_TRUE;
        tunnelEntry.ipv6Cfg.vlanId                 = PRV_TGF_VLANID_CNS;
        tunnelEntry.ipv6Cfg.ttl                    = 20;
        tunnelEntry.ipv6Cfg.ipHeaderProtocol       = PRV_TGF_TUNNEL_START_IP_HEADER_PROTOCOL_UDP_E;
        tunnelEntry.ipv6Cfg.udpDstPort             = PRV_TGF_SFLOW_V5_UDP_PORT_CNS;
        tunnelEntry.ipv6Cfg.udpSrcPort             = PRV_TGF_SFLOW_V5_UDP_PORT_CNS;
        cpssOsMemCpy(tunnelEntry.ipv6Cfg.macDa.arEther, prvTgfMirroredPacketTunnelL2Part.daMac,    sizeof(TGF_MAC_ADDR));
        cpssOsMemCpy(tunnelEntry.ipv6Cfg.destIp.arIP, prvTgfMirroredPacketTunnelIpv6Part.dstAddr, sizeof(TGF_IPV6_ADDR));
        cpssOsMemCpy(tunnelEntry.ipv6Cfg.srcIp.arIP, prvTgfMirroredPacketTunnelIpv6Part.srcAddr, sizeof(TGF_IPV6_ADDR));
        rc = prvTgfTunnelStartEntrySet(PRV_TGF_TUNNEL_START_IPV6_INDEX_CNS, CPSS_TUNNEL_GENERIC_IPV6_E, &tunnelEntry);
    }
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfTunnelStartEntrySet ");

}
/**
* @internal prvTgfMirrorEgressEnhancedSflowConfig function
* @endinternal
*
* @brief   PHA thread config
*     Create VLAN with tagged ports [0,2,3]
*    config enhanced PHA threads with correct firmware#
*    config 1st EPCL rule. action: analyzer index + PHA_save_target_port
*    config the analyzer entry with eport+sampling rate
*    config eport to TS + egr phy port
*    config egr mirror as ingress mirror
*    config TS entry
*    config 2nd EPCL rule to match mirror packet
*            action: PHA id = PHA_enhanced_sflow_ipv4, flow id = analyzer id
*
*/
GT_VOID prvTgfMirrorEgressEnhancedSflowConfig(CPSS_IP_PROTOCOL_STACK_ENT protocol)
{
    GT_STATUS rc;
    PRV_TGF_BRG_MAC_ENTRY_STC     prvTgfMacEntry;

    /* Create VLAN with tagged ports [0,2,3] */
    rc = prvTgfBrgDefVlanEntryWriteWithTaggingCmd(PRV_TGF_VLANID_CNS, PRV_TGF_BRG_VLAN_PORT_TAG0_CMD_E);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWrite: %d", PRV_TGF_VLANID_CNS);
    prvTgfBrgVlanMemberRemove(prvTgfDevNum, PRV_TGF_VLANID_CNS, prvTgfPortsArray[PRV_TGF_ANALYZER_PORT_IDX_CNS]);
    prvTgfBrgVlanMemberRemove(prvTgfDevNum, PRV_TGF_VLANID_CNS, prvTgfPortsArray[PRV_TGF_LB_PORT_IDX_CNS]);


    /* Add MAC Entry  */
    cpssOsMemSet(&prvTgfMacEntry, 0, sizeof(prvTgfMacEntry));

    prvTgfMacEntry.key.entryType                = PRV_TGF_FDB_ENTRY_TYPE_MAC_ADDR_E;
    prvTgfMacEntry.key.key.macVlan.vlanId       = PRV_TGF_VLANID_CNS;
    prvTgfMacEntry.dstInterface.type            = CPSS_INTERFACE_PORT_E;
    prvTgfMacEntry.dstInterface.devPort.hwDevNum  = prvTgfDevNum; /*prvTgfDevsArray[PRV_TGF_RX_PORT_IDX_CNS]*/;/*the sender*/
    prvTgfMacEntry.dstInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_RX_PORT_IDX_CNS];/*the sender*/
    prvTgfMacEntry.isStatic                     = GT_FALSE;
    prvTgfMacEntry.daCommand                    = PRV_TGF_PACKET_CMD_FORWARD_E;
    prvTgfMacEntry.saCommand                    = PRV_TGF_PACKET_CMD_FORWARD_E;

    cpssOsMemCpy(prvTgfMacEntry.key.key.macVlan.macAddr.arEther, prvTgfOriginalPacketL2Part.daMac, sizeof(TGF_MAC_ADDR));

    /* AUTODOC: add FDB entry with MAC 00:00:00:00:34:02, VLAN 5, port RX*/
    rc = prvTgfBrgFdbMacEntrySet(&prvTgfMacEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbMacEntrySet");

    /*config enhanced PHA threads with correct firmware#*/
    prvTgfMirrorEgressEnhancedSflowPhaConfig();

    /*config 1st EPCL rule. action: analyzer index + PHA_save_target_port*/
    prvTgfMirrorEgressEnhancedSflowEpclUDBconfig();
    prvTgfMirrorEgressEnhancedSflowEpcl1config();

    /*config the analyzer entry with eport+sampling rate*/
    prvTgfEnahncedSflowMirrorConfig();

    /*config 2nd EPCL rule to match mirror packet, to insert sflow header
      *action: PHA id = PHA_enhanced_sflow, flow id = analyzer id*/
    prvTgfMirrorEgressEnhancedSflowEpcl2config();

    /**PASS 2**/
    prvTgfMirrorEgressEnhancedSflowLBConfig();
    prvTgfMirrorEgressEnhancedSflowTtiConfig();
    /*config eport for LB packet*/
    prvTgfEnhancedSflowMirrorConfig(protocol);
    /*config TS entry*/
    prvTgfEnhancedSflowIpTsConfig(protocol);
    /*config 2nd EPCL rule to match mirror packet
      *action: PHA id = PHA_enhanced_sflow_ipv4, flow id = analyzer id*/
    prvTgfMirrorEgressEnhancedSflowEpcl3config(protocol);
}

/**
* @internal prvTgfEnhancedSflowEpclConfigReset function
* @endinternal
*
* @brief   EPCL configurations restore
*           Restore port config
*           invalidate EPCL rule
*/
static GT_VOID prvTgfEnhancedSflowEpclConfigReset()
{
    GT_STATUS rc;

    /* AUTODOC: disable all PCL configurations on the ports */
    prvTgfPclPortsRestoreAll();
    /* AUTODOC: Invalidate PCL rules */
    rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_10_BYTES_E, PRV_TGF_ENHANCED_SFLOW_EPCL_IDX_CNS, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Failure in prvTgfPclRuleValidStatusSet, index = %d", PRV_TGF_ENHANCED_SFLOW_EPCL_IDX_CNS);
    rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_10_BYTES_E, PRV_TGF_ENHANCED_SFLOW_SAVE_TARGET_EPCL_IDX_CNS, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Failure in prvTgfPclRuleValidStatusSet, index = %d", PRV_TGF_ENHANCED_SFLOW_SAVE_TARGET_EPCL_IDX_CNS);
    rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_10_BYTES_E, PRV_TGF_ENHANCED_SFLOW_IPV4_EPCL_IDX_CNS, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Failure in prvTgfPclRuleValidStatusSet, index = %d", PRV_TGF_ENHANCED_SFLOW_IPV4_EPCL_IDX_CNS);
    rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_10_BYTES_E, PRV_TGF_ENHANCED_SFLOW_IPV6_EPCL_IDX_CNS, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Failure in prvTgfPclRuleValidStatusSet, index = %d", PRV_TGF_ENHANCED_SFLOW_IPV6_EPCL_IDX_CNS);
}
/**
* @internal prvTgfMirrorEgressEnhancedSflowLBConfigReset function
* @endinternal
*
* @brief   Restore the Loopback config for enhanced sflow configurations
*/
static GT_VOID prvTgfMirrorEgressEnhancedSflowLBConfigReset()
{
    CPSS_INTERFACE_INFO_STC  portInterface;
    GT_STATUS rc;

    cpssOsMemSet(&portInterface, 0, sizeof(portInterface));
    /* reset loopback mode on port */
    portInterface.type            = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevNum;
    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_LB_PORT_IDX_CNS];
    rc = tgfTrafficGeneratorPortLoopbackModeEnableSet(&portInterface, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Failure in tgfTrafficGeneratorPortLoopbackModeEnableSet port %d",prvTgfPortsArray[PRV_TGF_LB_PORT_IDX_CNS]);

}
/**
* @internal prvTgfMirrorEgressEnhancedSflowTtiConfigReset function
* @endinternal
*
* @brief   Multi target Sflow pass #2.
*          Reset TTI config to get eVIDX for multiple SLOW collectors
*               Disable TTI lookup for srcPort derived from remote Vlan tag
*               Delete TTI rule
*/
static GT_VOID prvTgfMirrorEgressEnhancedSflowTtiConfigReset(GT_VOID)
{
    GT_STATUS   rc;
    GT_U32      i;

    /* AUTODOC: disable TTI lookup for port 0, for keyType */
    rc = cpssDxChTtiPortLookupEnableSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_LB_PORT_IDX_CNS], keyType, prvTgfRestoreCfg.ttiEnable);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChTtiPortLookupEnableSet");

    rc = cpssDxChTtiPacketTypeKeySizeSet(prvTgfDevNum, keyType, prvTgfRestoreCfg.ttiKeySize);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiPacketTypeKeySizeSet: %d", prvTgfDevNum);

    /* AUTODOC: invalidate TTI rules */
    rc = prvTgfTtiRuleValidStatusSet(PRV_TGF_TTI0_INDEX_CNS, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleValidStatusSet");

    /*Restore TTI UDB config*/
    for (i=0; i <= 2 ; i++)
    {
        rc = prvTgfTtiUserDefinedByteSet(prvTgfDevNum, tgfKeyType, 0, prvTgfRestoreCfg.ttiAnchor[i], prvTgfRestoreCfg.ttiOffset[i]);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfTtiUserDefinedByteSet: index %d", i);
    }
}
/**
* @internal prvTgfMirrorEgressEnhancedSflowConfigReset function
* @endinternal
*
* @brief   Restore the enahnced sflow configurations
*/
GT_VOID prvTgfMirrorEgressEnhancedSflowConfigReset(CPSS_IP_PROTOCOL_STACK_ENT protocol)
{
    GT_STATUS rc;
    /*reset Vlan and flush fdb*/
    prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_CNS);
    /* Flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);
    /*invalidate the IPv4 and IPV6 TS entries*/
    if (protocol == CPSS_IP_PROTOCOL_IPV4_E)
        prvTgfIngresssflowV5IpTsConfigReset(PRV_TGF_TUNNEL_START_IPV4_INDEX_CNS , CPSS_IP_PROTOCOL_IPV4_E);
    else
        prvTgfIngresssflowV5IpTsConfigReset(PRV_TGF_TUNNEL_START_IPV6_INDEX_CNS , CPSS_IP_PROTOCOL_IPV6_E);

    /*invalidate the EPCL rule for IPv4 and IPV6 entries*/
    prvTgfEnhancedSflowEpclConfigReset();

    /*restore LB config*/
    prvTgfMirrorEgressEnhancedSflowLBConfigReset();
    prvTgfMirrorEgressEnhancedSflowTtiConfigReset();
    /*invalidate the mirror entry*/
    prvTgfIngresssflowV5MirrorConfigReset();
    /*restore PHA FW img*/
    prvTgfPhaFWRestore();
    /*clear the packet buffers*/
    tgfTrafficTableRxPcktTblClear();
}

/**
* @internal prvTgfMirrorEgressEnhancedSflowValidatePacket function
* @endinternal
*
* @brief   Send and test packet for enhanced sflow configurations
*/
static void prvTgfMirrorEgressEnhancedSflowValidatePacket(GT_U8* packetBuf, CPSS_IP_PROTOCOL_STACK_ENT protocol)
{
    GT_U32                  ipHeaderOffset;
    GT_U32                  offset, value, localDevNum;
    GT_STATUS               rc;

    if (protocol == CPSS_IP_PROTOCOL_IPV4_E)
    {
        ipHeaderOffset = PRV_TGF_SFLOW_V5_IPV4_OFFSET_CNS;
    }
    else
    {
        ipHeaderOffset = PRV_TGF_SFLOW_V5_IPV6_OFFSET_CNS;
    }
    /*use PRV_TGF_ENHANCED_SFLOW_SHIM_HEADER_LEN_CNS to verify
      IP high and low in ts_shim headers*/

    /*check sflow version*/
    offset = ipHeaderOffset;
    value = packetBuf[offset] << 24 |
               packetBuf[offset + 1 ] << 16 |
               packetBuf[offset + 2 ] << 8 |
               packetBuf[offset + 3 ];
    UTF_VERIFY_EQUAL0_STRING_MAC(5, value, "sflow version mismatch");

    /*src mod id = sflow_version + 1*/
    offset += 4;
    value = packetBuf[offset];
    UTF_VERIFY_EQUAL0_STRING_MAC(PRV_TGF_ENHANCED_SFLOW_SRC_MODID_CNS, value, "source mod id mismatch");
    /*src port 8 bits = sflow_version + 2*/
    value = packetBuf[offset+1];
    UTF_VERIFY_EQUAL0_STRING_MAC((prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS]&0XFF), value, "source port mismatch");
    /*dst mod id = sflow_version + 3*/
    rc = cpssDxChCfgHwDevNumGet(prvTgfDevNum, &localDevNum);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvUtfHwDeviceNumberGet \n");

    value = packetBuf[offset+2];

    UTF_VERIFY_EQUAL0_STRING_MAC(localDevNum, value, "Destination mod id mismatch");
    /*dst mod id = sflow_version + 4*/
    value = packetBuf[offset+3];
    UTF_VERIFY_EQUAL0_STRING_MAC((prvTgfPortsArray[PRV_TGF_RX_PORT_IDX_CNS] & 0XFF), value, "destination port mismatch");

    /*Next offset to RX shim header*/
    offset += 12;
    /*IP addr = pkt[CS + TS + RSVD]*/
    UTF_VERIFY_EQUAL1_STRING_MAC(agentIpAddr[0], packetBuf[offset + 11], "ip[0] mismatch at offset %d",offset + 11);
    UTF_VERIFY_EQUAL1_STRING_MAC(agentIpAddr[1], packetBuf[offset + 12], "ip[1] mismatch at offset %d",offset + 12);
    /*Next offset to TX shim header*/
    offset += 14;
    /*IP addr = pkt[CS + TS + RSVD]*/
    UTF_VERIFY_EQUAL1_STRING_MAC(agentIpAddr[2], packetBuf[offset + 11], "ip[2] mismatch at offset %d",offset + 11);
    UTF_VERIFY_EQUAL1_STRING_MAC(agentIpAddr[3], packetBuf[offset + 12], "ip[3] mismatch at offset %d",offset + 11);
}
/**
* @internal prvTgfMirrorEgressEnhancedSflowTrafficSend function
* @endinternal
*
* @brief  Traffic test for ingress mirroring functionality
*           with sflow v5 IPv4 encapsulation
*           RX port #1
*           TX analyzer port #2
*           Check in the SFLOW header for
*           1. agent IPv4 address
*           2. enterprise format
*           3. sampling ratio
*/
void prvTgfMirrorEgressEnhancedSflowTrafficSend(CPSS_IP_PROTOCOL_STACK_ENT protocol)
{

    CPSS_INTERFACE_INFO_STC  analyzerPortInterface;
    GT_U8                 packetBuf[256];
    GT_U32                packetLen = sizeof(packetBuf);
    GT_U32                packetActualLength = 0;
    GT_U8                 queue = 0;
    GT_U8                 dev = 0;
    GT_BOOL                 getFirst = GT_TRUE;
    TGF_NET_DSA_STC       rxParam;
    GT_STATUS                           rc = GT_OK;

    /* Enable capture on analyzer port */
    analyzerPortInterface.type              = CPSS_INTERFACE_PORT_E;
    analyzerPortInterface.devPort.hwDevNum  = prvTgfDevNum;
    analyzerPortInterface.devPort.portNum   = prvTgfPortsArray[PRV_TGF_ANALYZER_PORT_IDX_CNS];
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&analyzerPortInterface,
                                                TGF_CAPTURE_MODE_MIRRORING_E,
                                                GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet");

    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfEnhancedSflowPacketInfo, 1, 0, NULL);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfSetTxSetupEth");

    PRV_UTF_LOG0_MAC("======= Send first packet =======\n");
    /* Send first packet  */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfStartTransmitingEth");

    /* Disable capture on analyzer port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(
            &analyzerPortInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet");

    /* AUTODOC: Get the first rx pkt on egress port */
        rc = tgfTrafficGeneratorRxInCpuFromExpectedInterfaceGet(&analyzerPortInterface,
                TGF_PACKET_TYPE_CAPTURE_E,
                getFirst, GT_TRUE, packetBuf,
                &packetLen, &packetActualLength,
                &dev, &queue, &rxParam);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                "Error: failed capture on inPortId %d outPortId %d \n" );

    tgfTrafficTableRxPcktTblClear();

    /*verify the packet*/
    prvTgfMirrorEgressEnhancedSflowValidatePacket(packetBuf, protocol);
}

/**
* @internal prvTgfIngressMirrorSflowV5BridgeConfigSet function
* @endinternal
*
* @brief  Bridge config for vlan 5 traffic
*/
static void prvTgfIngressMirrorSflowV5BridgeConfigSet()
{
    GT_STATUS rc;

    prvTgfBrgVlanEntryStore(1);

    rc = prvTgfBrgVlanMemberRemove(prvTgfDevNum, 1, prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanMemberRemove: port %d", prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS]);
    rc = prvTgfBrgVlanMemberRemove(prvTgfDevNum, 1, prvTgfPortsArray[PRV_TGF_SERVICE_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanMemberRemove: port %d", prvTgfPortsArray[PRV_TGF_SERVICE_PORT_IDX_CNS]);

    /* Create VLAN 5 with untagged ports [0,1,2,3] */
    rc = prvTgfBrgDefVlanEntryWrite(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryWrite: %d", PRV_TGF_VLANID_CNS);

    /*remove all, except ingress, remote ports from vlan 5*/
    rc = prvTgfBrgVlanMemberRemove(prvTgfDevNum, PRV_TGF_VLANID_CNS, prvTgfPortsArray[PRV_TGF_ANALYZER_PORT2_IDX_CNS]);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanMemberRemove: port %d", prvTgfPortsArray[PRV_TGF_ANALYZER_PORT2_IDX_CNS]);
    rc = prvTgfBrgVlanMemberRemove(prvTgfDevNum, PRV_TGF_VLANID_CNS, prvTgfPortsArray[PRV_TGF_ANALYZER_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanMemberRemove: port %d", prvTgfPortsArray[PRV_TGF_ANALYZER_PORT_IDX_CNS]);
    rc = prvTgfBrgVlanMemberRemove(prvTgfDevNum, PRV_TGF_VLANID_CNS, prvTgfPortsArray[PRV_TGF_SERVICE_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanMemberRemove: port %d", prvTgfPortsArray[PRV_TGF_SERVICE_PORT_IDX_CNS]);

    rc = prvTgfBrgVlanPortVidGet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS], &prvTgfRestoreCfg.pvid);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanMemberRemove: port %d", prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS]);
    rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS], PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgVlanMemberRemove: port %d", prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS]);
}

/**
* @internal prvTgfIngresssMirrorSflowV5MirrorPortConfig function
* @endinternal
*
* @brief   Ingress mirror port config
*           Enable source port mirroring for RX port and Set analyzer index to 1
*           map analyzer index to eport, and set sampling ratio
*           map eport to phy port#2 and enable TS for the eport
*
*/
static GT_VOID prvTgfIngresssMirrorSflowV5MirrorPortConfig()
{

    GT_STATUS                             rc;
    PRV_TGF_MIRROR_ANALYZER_INTERFACE_STC interface;
    PRV_TGF_BRG_EGRESS_PORT_INFO_STC      egressInfo;
    CPSS_INTERFACE_INFO_STC               phyPortInfo;
    PRV_TGF_TUNNEL_START_ENTRY_UNT        tunnelEntry;
    GT_U32                                analyzerIndex;

    /* AUTODOC: Save analyzer interface */
    prvTgfRestoreCfg.analyzerIndex = cpssOsRand() % 6 /* analyzer indexes 0..6 */;
    /* analyzer index 0 is already used by some other tests so skip that */
    if(prvTgfRestoreCfg.analyzerIndex == 0)
    {
        prvTgfRestoreCfg.analyzerIndex += 1;
    }
    PRV_UTF_LOG1_MAC("Analyzer index %d\n", prvTgfRestoreCfg.analyzerIndex);

    analyzerIndex = prvTgfRestoreCfg.analyzerIndex;
    rc = prvTgfMirrorAnalyzerInterfaceGet(prvTgfDevNum, analyzerIndex, &prvTgfRestoreCfg.interface);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfMirrorAnalyzerInterfaceGet: %d", prvTgfDevNum);

    /* AUTODOC: set analyzer interface index=1: */
    /* AUTODOC:   analyzer devNum=0, eport = 1024 , phy port=2 */
    interface.interface.type = CPSS_INTERFACE_PORT_E;
    interface.interface.devPort.hwDevNum  = prvTgfDevNum;
    interface.interface.devPort.portNum = PRV_TGF_DEFAULT_EPORT_CNS;
    rc = prvTgfMirrorAnalyzerInterfaceSet(analyzerIndex, &interface);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfMirrorAnalyzerInterfaceSet: %d", analyzerIndex);

    /* AUTODOC: Save ePort to phyport configuration */
    rc = prvTgfBrgEportToPhysicalPortTargetMappingTableGet(prvTgfDevNum,
                                                      PRV_TGF_DEFAULT_EPORT_CNS,
                                                       &(prvTgfRestoreCfg.phyPortInfo));
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfBrgEportToPhysicalPortTargetMappingTableGet");
    phyPortInfo.type = CPSS_INTERFACE_PORT_E;
    phyPortInfo.devPort.hwDevNum = prvTgfDevNum;
    phyPortInfo.devPort.portNum = prvTgfPortsArray[PRV_TGF_LB_PORT_IDX_CNS];
    prvTgfBrgEportToPhysicalPortTargetMappingTableSet(prvTgfDevNum,
                                                       PRV_TGF_DEFAULT_EPORT_CNS, &phyPortInfo );
    /*Set sampling ratio*/
    rc = cpssDxChMirrorRxStatMirrorToAnalyzerIndexAndRatioGet(prvTgfDevNum, analyzerIndex, &prvTgfRestoreCfg.samplingEn, &prvTgfRestoreCfg.   samplingRatio);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChMirrorRxStatMirrorToAnalyzerIndexAndRatioGet");
    rc = cpssDxChMirrorRxStatMirrorToAnalyzerIndexAndRatioSet(prvTgfDevNum, analyzerIndex, GT_TRUE, PRV_TGF_SFLOW_MIRROR_RATE_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChMirrorRxStatMirrorToAnalyzerIndexAndRatioSet");

    /***********************************************/
    /* Egress Port TS = Generic TS */
    /***********************************************/
    /* AUTODOC: Egress ePort TS SETUP CONFIGURATION: */
    PRV_UTF_LOG0_MAC("======= Setting Egress tunnel ipv6 Configuration =======\n");

    /* AUTODOC: Save ePort attributes configuration */
    rc = prvTgfBrgEportToPhysicalPortEgressPortInfoGet(prvTgfDevNum,
                                                       PRV_TGF_DEFAULT_EPORT_CNS,
                                                       &(prvTgfRestoreCfg.egressInfo));
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfBrgEportToPhysicalPortEgressPortInfoGet");

    /* AUTODOC: Set ePort attributes configuration */
    cpssOsMemSet(&egressInfo,0,sizeof(egressInfo));
    egressInfo                         = prvTgfRestoreCfg.egressInfo;
    egressInfo.tunnelStart             = GT_TRUE;
    egressInfo.tunnelStartPtr          = PRV_TGF_TUNNEL_START_IPV4_INDEX_CNS+8;
    egressInfo.tsPassengerPacketType   = PRV_TGF_TUNNEL_PASSENGER_ETHERNET_E;

    /* AUTODOC: config ePort attributes: */
    /* AUTODOC:   Tunnel Start=ENABLE, tunnelStartPtr=8+8 */
    /* AUTODOC:   tsPassenger=ETHERNET */
    rc = prvTgfBrgEportToPhysicalPortEgressPortInfoSet(prvTgfDevNum,
                                        PRV_TGF_DEFAULT_EPORT_CNS,
                                        &egressInfo);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfBrgEportToPhysicalPortEgressPortInfoSet");

    /* AUTODOC: Set Tunnel Start entry 8+8 with: */
    /* AUTODOC: genericType = long, tagEnable = TRUE, vlanId=1 */
    cpssOsMemSet(&tunnelEntry, 0, sizeof(tunnelEntry));
    tunnelEntry.genCfg.tagEnable            = GT_TRUE;
    tunnelEntry.genCfg.vlanId               = PRV_TGF_VLANID_CNS;
    tunnelEntry.genCfg.genericType          = PRV_TGF_TUNNEL_START_GENERIC_LONG_TYPE_E;
    tunnelEntry.genCfg.etherType            = PRV_TGF_SFLOW_RESERVED_ETHER_TYPE_CNS;

    cpssOsMemCpy(tunnelEntry.genCfg.macDa.arEther, prvTgfOriginalPacketL2Part.daMac, sizeof(TGF_MAC_ADDR));

    rc = prvTgfTunnelStartEntrySet(PRV_TGF_TUNNEL_START_IPV4_INDEX_CNS+8, CPSS_TUNNEL_GENERIC_E, &tunnelEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfTunnelStartEntrySet");

}

static GT_VOID prvTgfIngresssMirrorSflowV5RxPortEnableConfig()
{
    GT_STATUS rc;
    GT_U32    analyzerIndex;

    analyzerIndex = prvTgfRestoreCfg.analyzerIndex;
    rc = prvTgfMirrorRxPortGet(prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS], &prvTgfRestoreCfg.rxMirrorEnable, &prvTgfRestoreCfg.rxMirrorIdx);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfMirrorRxPortGet");
    rc = prvTgfMirrorRxPortSet(prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS], GT_TRUE, analyzerIndex);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfMirrorRxPortSet");
}

/**
* @internal prvTgfIngressMIrrorSflowV5PhaConfig function
* @endinternal
*
* @brief   PHA thread config
*           Set the FW thread id 57 with SFLOWV5 thread id
*           Set the thread memory with IPv4 agent address
*           Set the thread memory with sflow data format
*
*/
static GT_VOID prvTgfIngressMirrorSflowV5PhaConfig()
{
    GT_STATUS rc;
    GT_U32    iter;

    /* AUTODOC: Set the thread entry */
    CPSS_DXCH_PHA_THREAD_INFO_UNT            extInfo;
    CPSS_DXCH_PHA_COMMON_THREAD_INFO_STC     commonInfo;

    /* AUTODOC: Set the thread entry */
    cpssOsMemSet(&commonInfo, 0, sizeof(commonInfo));
    cpssOsMemSet(&extInfo, 0, sizeof(extInfo));
    prvTgfRestoreCfg.dataFormat = cpssOsRand() % 32;
    PRV_UTF_LOG1_MAC("Data Format %d\n", prvTgfRestoreCfg.dataFormat);
    extInfo.sflowV5Mirror.sflowDataFormat = prvTgfRestoreCfg.dataFormat;
    commonInfo.statisticalProcessingFactor = 0;
    commonInfo.busyStallMode = CPSS_DXCH_PHA_BUSY_STALL_MODE_ORDERED_BYPASS_E;
    commonInfo.stallDropCode = CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E;

    for(iter = 0; iter < 4; iter++)
    {
        sflowV5AgentIpv4RandomAddrCollector1[iter] = (GT_U8) (cpssOsRand() % 256);
        PRV_UTF_LOG2_MAC("IP address: octet %d value %d\n", iter, sflowV5AgentIpv4RandomAddrCollector1[iter]);
    }
    extInfo.sflowV5Mirror.sflowAgentIpAddr.arIP[0] = sflowV5AgentIpv4RandomAddrCollector1[0];
    extInfo.sflowV5Mirror.sflowAgentIpAddr.arIP[1] = sflowV5AgentIpv4RandomAddrCollector1[1];
    extInfo.sflowV5Mirror.sflowAgentIpAddr.arIP[2] = sflowV5AgentIpv4RandomAddrCollector1[2];
    extInfo.sflowV5Mirror.sflowAgentIpAddr.arIP[3] = sflowV5AgentIpv4RandomAddrCollector1[3];
    rc = cpssDxChPhaThreadIdEntrySet(prvTgfDevNum,
            PRV_TGF_PHA_THREAD_ID_SFLOW_V5_IPV4_CNS,
            &commonInfo,
            CPSS_DXCH_PHA_THREAD_TYPE_SFLOW_V5_ENT0_FORMAT1_RAW_PACKET_E,
            &extInfo);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPhaThreadIdEntrySet");

    /* AUTODOC: Assign thread to the ePort */
    rc = cpssDxChPhaPortThreadIdSet(prvTgfDevNum,
                                    prvTgfPortsArray[PRV_TGF_LB_PORT_IDX_CNS],
                                    GT_TRUE,
                                    PRV_TGF_PHA_THREAD_ID_SFLOW_V5_IPV4_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPhaPortThreadIdSet");
}

/**
* @internal prvTgfIngressMirrorSflowV5EpclConfigSet function
* @endinternal
*
* @brief   EPCL configurations
*           Enable analyzer port for EPCL trigger
*           Enable port mode for config table
*           10B UDB keys with eport, egr mtag cmd = TO_ANALYZER
*           Action = trigger PHA thread, flow based OAM
*/
static GT_VOID prvTgfIngressMirrorSflowV5EpclConfigSet
(
    CPSS_PACKET_CMD_ENT packetCommand
)
{
    GT_STATUS                           rc = GT_OK;
    PRV_TGF_PCL_RULE_FORMAT_UNT         mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT         pattern;
    PRV_TGF_PCL_ACTION_STC              action;
    GT_U32                              ruleIndex;
    PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT    ruleFormat;
    PRV_TGF_PCL_UDB_SELECT_STC          udbSelect;
    GT_U32                              udbSelectidx;

    PRV_UTF_LOG0_MAC("======= Setting EPCL Configuration =======\n");

    /* AUTODOC: init PCL Engine for Egress PCL for analyzer port */
    rc = prvTgfPclDefPortInit(
            prvTgfPortsArray[PRV_TGF_LB_PORT_IDX_CNS],
            CPSS_PCL_DIRECTION_EGRESS_E,
            CPSS_PCL_LOOKUP_0_E,
            PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E,
            PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E,
            PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfPclDefPortInit");

    /* AUTODOC: enable EPCL on Analyzer port */
    rc = prvTgfPclEgressPclPacketTypesSet(prvTgfDevNum,
                                          prvTgfPortsArray[PRV_TGF_LB_PORT_IDX_CNS],
                                          PRV_TGF_PCL_EGRESS_PKT_TO_ANALYZER_E, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfPclEgressPclPacketTypesSet");
    rc = prvTgfPclEgressPclPacketTypesSet(prvTgfDevNum,
                                          prvTgfPortsArray[PRV_TGF_LB_PORT_IDX_CNS],
                                          PRV_TGF_PCL_EGRESS_PKT_TS_E, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfPclEgressPclPacketTypesSet");
    rc = prvTgfPclEgressPclPacketTypesSet(prvTgfDevNum,
                                          prvTgfPortsArray[PRV_TGF_LB_PORT_IDX_CNS],
                                          PRV_TGF_PCL_EGRESS_PKT_NON_TS_E, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfPclEgressPclPacketTypesSet");

    /* AUTODOC: Configure ePort access mode for EPCL lookup */
    rc = prvTgfPclPortLookupCfgTabAccessModeGet(prvTgfPortsArray[PRV_TGF_LB_PORT_IDX_CNS],
                           CPSS_PCL_DIRECTION_EGRESS_E, CPSS_PCL_LOOKUP_0_E, 0,
                           &prvTgfRestoreCfg.pclPortAccessModeCfgGet);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclPortLookupCfgTabAccessModeGet");

    rc = prvTgfPclPortLookupCfgTabAccessModeSet(prvTgfPortsArray[PRV_TGF_LB_PORT_IDX_CNS],
                           CPSS_PCL_DIRECTION_EGRESS_E, CPSS_PCL_LOOKUP_0_E, 0,
                           PRV_TGF_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclPortLookupCfgTabAccessModeSet");

    /* AUTODOC: Configure "Access to the EPCL configuration" table with Port Mode */
    rc = cpssDxChPclEgressTunnelStartPacketsCfgTabAccessModeGet(prvTgfDevNum,
                                &prvTgfRestoreCfg.pclTsAccessModeCfgGet);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPclEgressTunnelStartPacketsCfgTabAccessModeSet");

    rc = cpssDxChPclEgressTunnelStartPacketsCfgTabAccessModeSet(prvTgfDevNum,
                                CPSS_DXCH_PCL_PORT_LOOKUP_CFG_TAB_ACC_MODE_BY_PORT_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPclEgressTunnelStartPacketsCfgTabAccessModeSet");

    cpssOsMemSet(&udbSelect, 0, sizeof(CPSS_DXCH_PCL_UDB_SELECT_STC));
    udbSelectidx = 0;

    while(1)
    {
        if(prvTgfsflowV5EpclMetadataUdbInfo[udbSelectidx].offsetType == PRV_TGF_PCL_OFFSET_INVALID_E)
            break;

        /* AUTODOC:   configure User Defined Byte(UDB) */
        /* AUTODOC:   format CPSS_DXCH_PCL_RULE_FORMAT_EGRESS_UDB_10_E */
        /* AUTODOC:   packet type PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E */
        /* AUTODOC:   offsetType PRV_TGF_PCL_OFFSET_METADATA_E */
        /* AUTODOC:   offset 53 Analyzer ePort[0:6] */
        /* AUTODOC:   offset 54 Analyzer ePort[12:7] */
        /* AUTODOC:   offset 57 Analyzer ePort[13] */
        /* AUTODOC:   offset 64 egress marvell tagged[1:0] */
        rc = prvTgfPclUserDefinedByteSet(PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E,
                                        PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E,
                                        CPSS_PCL_DIRECTION_EGRESS_E,
                                        prvTgfsflowV5EpclMetadataUdbInfo[udbSelectidx].udbIndex,
                                        prvTgfsflowV5EpclMetadataUdbInfo[udbSelectidx].offsetType,
                                        prvTgfsflowV5EpclMetadataUdbInfo[udbSelectidx].offset);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclUserDefinedByteSet: %d for udbSelectidx = %d\n", prvTgfDevNum, udbSelectidx);

        udbSelect.udbSelectArr[udbSelectidx]=prvTgfsflowV5EpclMetadataUdbInfo[udbSelectidx].udbIndex;
        udbSelectidx++;
    }

    rc = prvTgfPclUserDefinedBytesSelectSet(PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E, PRV_TGF_PCL_PACKET_TYPE_ETHERNET_OTHER_E,
                                           CPSS_PCL_LOOKUP_0_E, &udbSelect);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfPclUserDefinedBytesSelectSet: %d\n", prvTgfDevNum);
    /* AUTODOC: set PCL rule 0 */
    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));
    cpssOsMemSet(&action, 0, sizeof(action));

    /* bits 1:7 is ePort[0:6] */
    pattern.ruleEgrUdbOnly.udb[0] = (PRV_TGF_DEFAULT_EPORT_CNS&0x7f) <<1;
    mask.ruleEgrUdbOnly.udb[0] = prvTgfsflowV5EpclMetadataUdbInfo[0].byteMask;

    /* bits 0:5 is ePort[12:7] */
    pattern.ruleEgrUdbOnly.udb[1] = ((PRV_TGF_DEFAULT_EPORT_CNS>>7)&0x3f);
    mask.ruleEgrUdbOnly.udb[1] = prvTgfsflowV5EpclMetadataUdbInfo[1].byteMask;

    /* bits 5 is ePort[13] */
    pattern.ruleEgrUdbOnly.udb[2] = (PRV_TGF_DEFAULT_EPORT_CNS>>8)&0x20;
    mask.ruleEgrUdbOnly.udb[2] = prvTgfsflowV5EpclMetadataUdbInfo[2].byteMask;

    /*egress pkt cmd*/
    pattern.ruleEgrUdbOnly.udb[3] = 0x02;
    mask.ruleEgrUdbOnly.udb[3] = prvTgfsflowV5EpclMetadataUdbInfo[3].byteMask;

    ruleIndex                                    = 0;
    ruleFormat                                   = PRV_TGF_PCL_RULE_FORMAT_EGRESS_UDB_10_E;
    action.pktCmd                                = (packetCommand == CPSS_PACKET_CMD_FORWARD_E) ? CPSS_PACKET_CMD_FORWARD_E :
                                                                                                  CPSS_PACKET_CMD_DROP_SOFT_E;
    action.egressPolicy                          = GT_TRUE;
    action.epclPhaInfo.phaThreadIdAssignmentMode = PRV_TGF_PCL_ACTION_EGRESS_PHA_THREAD_ID_ASSIGNMENT_MODE_ENABLED_E;
    action.epclPhaInfo.phaThreadId               = PRV_TGF_PHA_THREAD_ID_SFLOW_V5_IPV4_CNS;
    action.epclPhaInfo.phaThreadType             = PRV_TGF_PCL_ACTION_EGRESS_PHA_THREAD_SFLOW_V5_MIRROR_E;
    action.epclPhaInfo.phaThreadUnion.epclSflowV5Mirror.sflowSamplingRate = PRV_TGF_SFLOW_MIRROR_RATE_CNS;
    rc = prvTgfPclRuleSet(ruleFormat, ruleIndex, &mask, &pattern, &action);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPclRuleSet, prvTgfDevNum:%d,"
                                            "ruleFormat:%d, ruleIndex:%d", prvTgfDevNum, ruleFormat, ruleIndex);
}

/**
* @internal prvTgfIngresssMirrorSflowV5LoopbackPortConfig function
* @endinternal
*
* @brief   Loopback config
*
*/
static GT_VOID  prvTgfIngresssMirrorSflowV5LoopbackPortConfig
(
    GT_VOID
)
{
    CPSS_INTERFACE_INFO_STC  portInterface;
    GT_STATUS rc;

    cpssOsMemSet(&portInterface, 0, sizeof(portInterface));

    portInterface.type              = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevNum;
    portInterface.devPort.portNum   = prvTgfPortsArray[PRV_TGF_LB_PORT_IDX_CNS];

    /* set loopback mode on port */
    rc = tgfTrafficGeneratorPortLoopbackModeEnableSet(&portInterface, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Failure in tgfTrafficGeneratorPortLoopbackModeEnableSet port %d",prvTgfPortsArray[PRV_TGF_LB_PORT_IDX_CNS]);

    /*give port mgr time to config the LB*/
    cpssOsTimerWkAfter(10);
}

/**
* @internal prvTgfIngressMirrorSflowV5TtiConfig function
* @endinternal
*
* @brief   Enhanced Sflow pass #2.
*          TTI config to redirect partially built sflow packet to PHA
*               Enable TTI lookup for LB
*               Create UDB size 10 for ETH_Other,
*                   key : sflow ver, src mod id, src port
*                   action: redirect to eport, bridge bypass
*/
static GT_VOID prvTgfIngressMirrorSflowV5TtiConfig(GT_VOID)
{
    GT_STATUS                rc = GT_OK;
    PRV_TGF_TTI_ACTION_STC   ttiAction;
    PRV_TGF_TTI_RULE_UNT     ttiPattern;
    PRV_TGF_TTI_RULE_UNT     ttiMask;

    /* AUTODOC: clear pattern and mask */
    cpssOsMemSet((GT_VOID*) &ttiPattern, 0, sizeof(ttiPattern));
    cpssOsMemSet((GT_VOID*) &ttiMask,    0, sizeof(ttiMask));
    cpssOsMemSet((GT_VOID*) &ttiAction, 0, sizeof(ttiAction));

    PRV_UTF_LOG0_MAC("======= Setting TTI Configuration =======\n");

    /* AUTODOC: enable TTI lookup for Ethernet key on LB port */
    rc = prvTgfTtiPortLookupEnableGet(
        prvTgfPortsArray[PRV_TGF_LB_PORT_IDX_CNS], PRV_TGF_TTI_KEY_ETH_E, &prvTgfRestoreCfg.ttiLookupEnable);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfTtiPortLookupEnableGet: %d", prvTgfDevNum);
    rc = prvTgfTtiPortLookupEnableSet(
        prvTgfPortsArray[PRV_TGF_LB_PORT_IDX_CNS], PRV_TGF_TTI_KEY_ETH_E, GT_TRUE);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfTtiPortLookupEnableSet: %d", prvTgfDevNum);

    /* set TTI Pattern and Mask */
    ttiMask.eth.common.srcPortTrunk    = 0xFF;
    ttiPattern.eth.common.srcPortTrunk = prvTgfPortsArray[PRV_TGF_LB_PORT_IDX_CNS];

    ttiMask.eth.common.vid    = 0xFF;
    ttiPattern.eth.common.vid = PRV_TGF_VLANID_CNS;

    /* AUTODOC: set the TTI Rule Action for matched packet */
    ttiAction.interfaceInfo.type             = CPSS_INTERFACE_PORT_E;
    ttiAction.interfaceInfo.devPort.hwDevNum = prvTgfDevNum;
    ttiAction.interfaceInfo.devPort.portNum  = prvTgfPortsArray[PRV_TGF_RX_PORT_IDX_CNS];
    ttiAction.ingressPipeBypass              = GT_TRUE;
    ttiAction.redirectCommand                = PRV_TGF_TTI_REDIRECT_TO_EGRESS_E;
    ttiAction.tunnelStart                    = GT_TRUE;
    ttiAction.tunnelStartPtr                 = PRV_TGF_TUNNEL_START_IPV4_INDEX_CNS;
    ttiAction.tsPassengerPacketType          = PRV_TGF_TUNNEL_PASSENGER_OTHER_E;

    /* set TTI Rule */
    rc = prvTgfTtiRuleSet(PRV_TGF_TTI0_INDEX_CNS, PRV_TGF_TTI_RULE_ETH_E,
            &ttiPattern, &ttiMask, &ttiAction);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTtiRuleSet");
}

/**
* @internal prvTgfIngressMirrorSflowV5TsConfig function
* @endinternal
*
* @brief   TS configurations for IPv4 tunnel
*           Set Tunnel Start entry 8 with:
*           tunnelType = Generic IPv4, tagEnable = TRUE, vlanId=5, ipHeaderProtocol = UDP, udp port = 6343
*           MACDA = 00:00:00:00:44:04 , DIP = 20.1.1.2, SIP=20.1.1.3
*           Tunnel start profile with 16B zeros
*
*/
static GT_VOID prvTgfIngressMirrorSflowV5TsConfig(GT_VOID)
{
    GT_STATUS                                       rc;
    PRV_TGF_TUNNEL_START_ENTRY_UNT                  tunnelEntry;
    PRV_TGF_TUNNEL_START_GEN_IP_PROFILE_STC         profileData;

    cpssOsMemSet(&tunnelEntry, 0, sizeof(tunnelEntry));
    tunnelEntry.ipv4Cfg.tagEnable              = GT_TRUE;
    tunnelEntry.ipv4Cfg.vlanId                 = PRV_TGF_VLANID_CNS;
    tunnelEntry.ipv4Cfg.ttl                    = 20;
    tunnelEntry.ipv4Cfg.ipHeaderProtocol       = PRV_TGF_TUNNEL_START_IP_HEADER_PROTOCOL_UDP_E;
    tunnelEntry.ipv4Cfg.udpDstPort             = PRV_TGF_SFLOW_V5_UDP_PORT_CNS;
    tunnelEntry.ipv4Cfg.udpSrcPort             = PRV_TGF_SFLOW_V5_UDP_PORT_CNS;

    cpssOsMemCpy(tunnelEntry.ipv4Cfg.macDa.arEther, prvTgfMirroredPacketTunnelL2Part.daMac,    sizeof(TGF_MAC_ADDR));
    cpssOsMemCpy(tunnelEntry.ipv4Cfg.destIp.arIP, prvTgfMirroredPacketTunnelIpv4Part.dstAddr, sizeof(TGF_IPV4_ADDR));
    cpssOsMemCpy(tunnelEntry.ipv4Cfg.srcIp.arIP, prvTgfMirroredPacketTunnelIpv4Part.srcAddr, sizeof(TGF_IPV4_ADDR));

    rc = prvTgfTunnelStartEntrySet(PRV_TGF_TUNNEL_START_IPV4_INDEX_CNS, CPSS_TUNNEL_GENERIC_IPV4_E, &tunnelEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfIngressMirrorSflowV5TsConfig");

    /* AUTODOC: configure tunnel start profile for Sflow v5 (16 Bytes as "Zero") */
    cpssOsMemSet(&profileData,0,sizeof(profileData));
    profileData.templateDataSize = PRV_TGF_TUNNEL_START_TEMPLATE_DATA_SIZE_16B_E;
    rc = prvTgfTunnelStartGenProfileTableEntrySet(prvTgfDevNum,
                                                  PRV_TGF_TEMPLATE_PROFILE_INDEX_CNS, &profileData);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfTunnelStartGenProfileTableEntrySet");

}

static void prvTgfIngressMirrorSflowV5PacketValidate(GT_U8* packetBuf, GT_U8 packetCount, CPSS_PACKET_CMD_ENT packetCommand)
{
    GT_STATUS               rc = GT_OK;
    GT_U32                  samplingRate;
    GT_U32                  dataFormat;
    GT_U32                  offset;
    GT_U8*                  ipAddr;
    GT_U32                  sflowVersion;
    GT_U32                  ipVersion;
    GT_U32                  subAgentId;
    GT_U32                  numberOfSamples;
    GT_U32                  srcIdTypeIndex;
    GT_U32                  drops;
    GT_U32                  inputInterface;
    GT_U32                  outputInterface;
    GT_U32                  numberOfRecords;
    GT_U32                  headerProtocol;
    GT_U32                  stripped;
    GT_U32                  sampleLength;
    GT_U32                  descEgressByteCount;
    GT_U32                  flowDataLength;
    GT_U32                  frameLength;
    GT_U32                  headerSize;
    GT_U32                  vlanId;
    GT_U32                  datagramSequenceNumber;
    GT_U32                  sampleSequenceNumber;
    GT_U32                  samplingPool;
    static GT_U32           dropsExpected = 0;
    /* Components to be verified
    ==sFlow DataGram Header (28B)===
    1. sFlow Version
    2. IP version
    3. Agent IPv4 Address
    4. Sub Agent ID
    5. Number of samples
    ==Sample Data Header (8B)========
    1. Data Format
    ==Flow Sample Header (32B)=======
    1. Source Id Type + Index
    2. Sampling Rate
    3. Sampling Pool
    4. Drops
    5. Input Interface
    6. Output Interface
    7. Number of records
    ==Flow Record Header (8B)========
    1. Data Format
    2. Flow Data Length
    ==Raw Packet Header (16B)========
    1. Header Protocol
    2. Frame Length
    3. Stripped
    4. Header size
    */
    rc = cpssOsMemCmp(&packetBuf[0], prvTgfMirroredPacketTunnelL2Part.daMac, sizeof(TGF_MAC_ADDR));
    UTF_VERIFY_EQUAL0_STRING_MAC(0, rc, "MAC DA mismatch");

    offset = TGF_L2_HEADER_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS;
    vlanId = ((packetBuf[offset] << 8) | packetBuf[offset+1]) & 0xFFF;
    UTF_VERIFY_EQUAL0_STRING_MAC(PRV_TGF_VLANID_CNS, vlanId, "Vlan mismatch");

    offset = TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS;
    dataFormat = (packetBuf[offset] << 8) | packetBuf[offset+1];
    UTF_VERIFY_EQUAL0_STRING_MAC(TGF_ETHERTYPE_0800_IPV4_TAG_CNS, dataFormat, "ether type mismatch");

    offset = PRV_TGF_SFLOW_V5_IPV4_OFFSET_CNS;
    ipAddr = sflowV5AgentIpv4RandomAddrCollector1;

    sflowVersion = (packetBuf[offset] << 24) | (packetBuf[offset+1] << 16)| (packetBuf[offset+2] << 8) | (packetBuf[offset+3]);
    UTF_VERIFY_EQUAL0_STRING_MAC(PRV_TGF_SFLOW_V5_VERSION_CNS, sflowVersion, "sflow version mismatch");

    offset += 4;
    ipVersion = (packetBuf[offset] << 24) | (packetBuf[offset+1] << 16)| (packetBuf[offset+2] << 8) | (packetBuf[offset+3]);
    UTF_VERIFY_EQUAL0_STRING_MAC(PRV_TGF_SFLOW_V5_IP_VERSION_CNS, ipVersion, "ip version mismatch");

    offset += 4;
    rc = cpssOsMemCmp(ipAddr, &packetBuf[offset], TGF_VLAN_TAG_SIZE_CNS);
    UTF_VERIFY_EQUAL0_STRING_MAC(0, rc, "Agent IPv4 mismatch");

    offset += 4;
    subAgentId = (packetBuf[offset] << 24) | (packetBuf[offset+1] << 16)| (packetBuf[offset+2] << 8) | (packetBuf[offset+3]);
    UTF_VERIFY_EQUAL0_STRING_MAC(0x0, subAgentId, "sub agent id mismatch");

    offset += 4;
    datagramSequenceNumber = (packetBuf[offset] << 24) | (packetBuf[offset+1] << 16)| (packetBuf[offset+2] << 8) | (packetBuf[offset+3]);
    UTF_VERIFY_EQUAL0_STRING_MAC(packetCount, datagramSequenceNumber, "datagram sequence number mismatch");

    offset += 8;
    numberOfSamples = (packetBuf[offset] << 24) | (packetBuf[offset+1] << 16)| (packetBuf[offset+2] << 8) | (packetBuf[offset+3]);
    UTF_VERIFY_EQUAL0_STRING_MAC(PRV_TGF_SFLOW_V5_NUMBER_OF_SAMPLES_CNS, numberOfSamples, "number of samples mismatch");

    offset += 4;
    dataFormat = (packetBuf[offset] << 24) | (packetBuf[offset+1] << 16)| (packetBuf[offset+2] << 8) | (packetBuf[offset+3]);
    UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfRestoreCfg.dataFormat, dataFormat, "data format mismatch");

    offset += 4;
    sampleLength = (packetBuf[offset] << 24) | (packetBuf[offset+1] << 16)| (packetBuf[offset+2] << 8) | (packetBuf[offset+3]);
    descEgressByteCount = sampleLength - 20;

    offset += 4;
    sampleSequenceNumber = (packetBuf[offset] << 24) | (packetBuf[offset+1] << 16)| (packetBuf[offset+2] << 8) | (packetBuf[offset+3]);
    UTF_VERIFY_EQUAL0_STRING_MAC(packetCount, sampleSequenceNumber, "sample sequence number mismatch");

    offset += 4;
    srcIdTypeIndex = (packetBuf[offset] << 24) | (packetBuf[offset+1] << 16)| (packetBuf[offset+2] << 8) | (packetBuf[offset+3]);
    UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS], srcIdTypeIndex, "source id type/index mismatch");

    offset += 4;
    samplingRate = (packetBuf[offset] << 24) | (packetBuf[offset+1] << 16)| (packetBuf[offset+2] << 8) | (packetBuf[offset+3]);
    UTF_VERIFY_EQUAL0_STRING_MAC(PRV_TGF_SFLOW_MIRROR_RATE_CNS, samplingRate, "Sampling rate mismatch");

    offset += 4;
    samplingPool = (packetBuf[offset] << 24) | (packetBuf[offset+1] << 16)| (packetBuf[offset+2] << 8) | (packetBuf[offset+3]);
    UTF_VERIFY_EQUAL0_STRING_MAC(packetCount, samplingPool, "Sampling pool mismatch");

    offset += 4;
    drops = (packetBuf[offset] << 24) | (packetBuf[offset+1] << 16)| (packetBuf[offset+2] << 8) | (packetBuf[offset+3]);
    if(packetCommand == CPSS_PACKET_CMD_DROP_SOFT_E)
    {
        dropsExpected++;
    }
    UTF_VERIFY_EQUAL0_STRING_MAC(dropsExpected, drops, "drops mismatch");

    offset += 4;
    inputInterface = (packetBuf[offset] << 24) | (packetBuf[offset+1] << 16)| (packetBuf[offset+2] << 8) | (packetBuf[offset+3]);
    UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS], inputInterface, "input interface mismatch");

    offset += 4;
    outputInterface = (packetBuf[offset] << 24) | (packetBuf[offset+1] << 16)| (packetBuf[offset+2] << 8) | (packetBuf[offset+3]);
    UTF_VERIFY_EQUAL0_STRING_MAC(PRV_TGF_SFLOW_V5_OUTPUT_INTERFACE_CNS, outputInterface, "output interface mismatch");

    offset += 4;
    numberOfRecords = (packetBuf[offset] << 24) | (packetBuf[offset+1] << 16)| (packetBuf[offset+2] << 8) | (packetBuf[offset+3]);
    UTF_VERIFY_EQUAL0_STRING_MAC(PRV_TGF_SFLOW_V5_NUMBER_OF_SAMPLES_CNS, numberOfRecords, "number of records mismatch");

    offset += 4;
    dataFormat = (packetBuf[offset] << 24) | (packetBuf[offset+1] << 16)| (packetBuf[offset+2] << 8) | (packetBuf[offset+3]);
    UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfRestoreCfg.dataFormat, dataFormat, "data format mismatch");

    offset += 4;
    flowDataLength = (packetBuf[offset] << 24) | (packetBuf[offset+1] << 16)| (packetBuf[offset+2] << 8) | (packetBuf[offset+3]);
    UTF_VERIFY_EQUAL0_STRING_MAC(descEgressByteCount - 20, flowDataLength, "flow data length mismatch");

    offset += 4;
    headerProtocol = (packetBuf[offset] << 24) | (packetBuf[offset+1] << 16)| (packetBuf[offset+2] << 8) | (packetBuf[offset+3]);
    UTF_VERIFY_EQUAL0_STRING_MAC(PRV_TGF_SFLOW_V5_HEADER_PROTOCOL_CNS, headerProtocol, "header protocol mismatch");

    offset += 4;
    frameLength = (packetBuf[offset] << 24) | (packetBuf[offset+1] << 16)| (packetBuf[offset+2] << 8) | (packetBuf[offset+3]);
    UTF_VERIFY_EQUAL0_STRING_MAC(descEgressByteCount - 36, frameLength, "frame length mismatch");

    offset += 4;
    stripped = (packetBuf[offset] << 24) | (packetBuf[offset+1] << 16)| (packetBuf[offset+2] << 8) | (packetBuf[offset+3]);
    UTF_VERIFY_EQUAL0_STRING_MAC(PRV_TGF_SFLOW_V5_STRIPPED_CNS, stripped, "stripped mismatch");

    offset += 4;
    headerSize = (packetBuf[offset] << 24) | (packetBuf[offset+1] << 16)| (packetBuf[offset+2] << 8) | (packetBuf[offset+3]);
    UTF_VERIFY_EQUAL0_STRING_MAC(descEgressByteCount - 32, headerSize, "header size mismatch");
}

/**
* @internal prvTgfFalconMirrorIngressSflowV5TrafficSend function
* @endinternal
*
* @brief  Traffic test for ingress mirroring functionality
*           with sflow v5 IPv4 encapsulation
*           RX port #1
*           TX analyzer port #2
*           Check in the SFLOW header for
*           1. agent IPv4 address
*           2. enterprise format
*           3. sampling ratio
*/
void prvTgfFalconMirrorIngressSflowV5TrafficSend(CPSS_PACKET_CMD_ENT packetCommand)
{

    CPSS_INTERFACE_INFO_STC  analyzerPortInterface;
    GT_U8                    packetBuf[256];
    GT_U32                   packetLen          = sizeof(packetBuf);
    GT_U32                   packetActualLength = 0;
    GT_U8                    queue              = 0;
    GT_U8                    dev                = 0;
    GT_BOOL                  getFirst           = GT_TRUE;
    TGF_NET_DSA_STC          rxParam;
    GT_STATUS                rc                 = GT_OK;
    GT_U32                   iter               = 0;
    static GT_U32            packetCount        = 0;

    PRV_UTF_LOG0_MAC("======= Send 4 packets =======\n");
    for(iter = 0; iter < prvTgfBurstCount; iter++)
    {
        /* Enable capture on analyzer port */
        analyzerPortInterface.type              = CPSS_INTERFACE_PORT_E;
        analyzerPortInterface.devPort.hwDevNum  = prvTgfDevNum;
        analyzerPortInterface.devPort.portNum   = prvTgfPortsArray[PRV_TGF_RX_PORT_IDX_CNS];
        rc = tgfTrafficGeneratorPortTxEthCaptureSet(&analyzerPortInterface,
                                                    TGF_CAPTURE_MODE_MIRRORING_E,
                                                    GT_TRUE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet");
        rc = prvTgfSetTxSetupEth(prvTgfDevNum, &prvTgfEnhancedSflowPacketInfo, 1, 0, NULL);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfSetTxSetupEth");

        rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS]);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfStartTransmitingEth");

        /* provide time for TO_CPU packet RX process */
        cpssOsTimerWkAfter(10);

        /* Disable capture on analyzer port */
        rc = tgfTrafficGeneratorPortTxEthCaptureSet(
                &analyzerPortInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet");

        /* AUTODOC: Get the first rx pkt on egress port */
        rc = tgfTrafficGeneratorRxInCpuFromExpectedInterfaceGet(&analyzerPortInterface,
                TGF_PACKET_TYPE_CAPTURE_E,
                getFirst, GT_TRUE, packetBuf,
                &packetLen, &packetActualLength,
                &dev, &queue, &rxParam);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc,
                "Error: failed capture on inPortId %d outPortId %d \n" );

        tgfTrafficTableRxPcktTblClear();

        /*verify the packet*/
        prvTgfIngressMirrorSflowV5PacketValidate(packetBuf, packetCount, packetCommand);
        packetCount++;
    }
}

/**
* @internal prvTgfFalconMirrorIngressSflowV5ConfigurationSet function
* @endinternal
*
* @brief  Test ingress mirroring functionality with sflow v5 encapsulation
*    configure VLAN;
*    configure an analyzer index, sampling rate and eport;
*    enable Rx mirroring for ingress port;
*    E2phy and IPv4 TS for eport
*    Configure OAM, EPLR & ERMRK for other fields of SFLOW header
*    configure EPCL rule to trigger IPv4 PHA thread, and pha metadata
*    send BC traffic and verify expected Sflow mirrored traffic on analyzer port;
*/
void prvTgfFalconMirrorIngressSflowV5ConfigurationSet(CPSS_PACKET_CMD_ENT packetCommand)
{

    /** PASS 1 Configuration **/

    /* bridge config */
    prvTgfIngressMirrorSflowV5BridgeConfigSet();

    /* mirror port config */
    prvTgfIngresssMirrorSflowV5MirrorPortConfig();

    /* enable Rx mirroring on port=1, index=0 */
    prvTgfIngresssMirrorSflowV5RxPortEnableConfig();

    /* loopback port config */
    prvTgfIngresssMirrorSflowV5LoopbackPortConfig();

    /* EPCL trigger for PHA SFLOW */
    prvTgfIngressMirrorSflowV5EpclConfigSet(packetCommand);

    /* PHA config */
    prvTgfIngressMirrorSflowV5PhaConfig();

    /** PASS 2 Configuration **/

    /* TTI Configuration */
    prvTgfIngressMirrorSflowV5TtiConfig();

    /* TS Configuration */
    prvTgfIngressMirrorSflowV5TsConfig();
}

/**
* @internal prvTgfIngressMirrorSflowV5BridgeConfigReset function
* @endinternal
*
* @brief  Bridge config restore
*/
static void prvTgfIngressMirrorSflowV5BridgeConfigReset()
{
    GT_STATUS rc;

    prvTgfBrgVlanEntryRestore(1);
    rc = prvTgfBrgDefVlanEntryInvalidate(PRV_TGF_VLANID_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfBrgDefVlanEntryInvalidate: %d", PRV_TGF_VLANID_CNS);

    /* Flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: %d, %d", prvTgfDevNum, GT_TRUE);

    rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS], prvTgfRestoreCfg.pvid);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfBrgFdbFlush: port %d, vlan %d", prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS], prvTgfRestoreCfg.pvid);
}

/**
* @internal prvTgfIngresssMirrorSflowV5MirrorPortConfigReset function
* @endinternal
*
* @brief   Ingress mirror config restore
*
*/
static GT_VOID prvTgfIngresssMirrorSflowV5MirrorPortConfigReset()
{

    GT_STATUS   rc;
    GT_STATUS   analyzerIndex;

    /* AUTODOC: Restore analyzer interface */
    analyzerIndex = prvTgfRestoreCfg.analyzerIndex;
    rc = prvTgfMirrorAnalyzerInterfaceSet(analyzerIndex, &prvTgfRestoreCfg.interface);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "prvTgfMirrorAnalyzerInterfaceSet: %d", prvTgfDevNum);

    /* AUTODOC: Restore ePort to phyport configuration */
    rc = prvTgfBrgEportToPhysicalPortTargetMappingTableSet(prvTgfDevNum,
                                                      PRV_TGF_DEFAULT_EPORT_CNS,
                                                       &(prvTgfRestoreCfg.phyPortInfo));
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfBrgEportToPhysicalPortTargetMappingTableSet");

    /* AUTODOC: Restore ingress statistical mirroring configuration */
    rc = cpssDxChMirrorRxStatMirrorToAnalyzerIndexAndRatioSet(prvTgfDevNum, analyzerIndex, prvTgfRestoreCfg.samplingEn, prvTgfRestoreCfg.samplingRatio);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChMirrorRxStatMirrorToAnalyzerIndexAndRatioSet");

    /* AUTODOC: Restore ePort attributes configuration */
    rc = prvTgfBrgEportToPhysicalPortEgressPortInfoSet(prvTgfDevNum,
                                                       PRV_TGF_DEFAULT_EPORT_CNS,
                                                       &(prvTgfRestoreCfg.egressInfo));
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfBrgEportToPhysicalPortEgressPortInfoSet");
}

/**
* @internal prvTgfIngresssMirrorSflowV5RxPortEnableConfigReset function
* @endinternal
*
* @brief Ingress mirror port config restore
*
*/
static GT_VOID prvTgfIngresssMirrorSflowV5RxPortEnableConfigReset()
{
    GT_STATUS rc;

    rc = prvTgfMirrorRxPortSet(prvTgfPortsArray[PRV_TGF_TX_PORT_IDX_CNS], prvTgfRestoreCfg.rxMirrorEnable, prvTgfRestoreCfg.rxMirrorIdx);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfMirrorRxPortSet");
}

/**
* @internal prvTgfIngressMirrorSflowV5EpclConfigRstore function
* @endinternal
*
* @brief   EPCL configurations restore
*           Restore port config
*           invalidate EPCL rule
*/
static GT_VOID prvTgfIngressMirrorSflowV5EpclConfigRstore()
{
    GT_STATUS rc;

    /* AUTODOC: Invalidate PCL rule 0 */
    rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_10_BYTES_E, 0, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Failure in prvTgfPclRuleValidStatusSet, index = %d", 0);

    /* AUTODOC: disable all PCL configurations on the ports */
    prvTgfPclPortsRestoreAll();

    /* AUTODOC: disable EPCL on Loopback port per packet type */
    rc = prvTgfPclEgressPclPacketTypesSet(prvTgfDevNum,
                                          prvTgfPortsArray[PRV_TGF_LB_PORT_IDX_CNS],
                                          PRV_TGF_PCL_EGRESS_PKT_TO_ANALYZER_E, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfPclEgressPclPacketTypesSet");
    rc = prvTgfPclEgressPclPacketTypesSet(prvTgfDevNum,
                                          prvTgfPortsArray[PRV_TGF_LB_PORT_IDX_CNS],
                                          PRV_TGF_PCL_EGRESS_PKT_TS_E, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfPclEgressPclPacketTypesSet");
    rc = prvTgfPclEgressPclPacketTypesSet(prvTgfDevNum,
                                          prvTgfPortsArray[PRV_TGF_LB_PORT_IDX_CNS],
                                          PRV_TGF_PCL_EGRESS_PKT_NON_TS_E, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfPclEgressPclPacketTypesSet");

    /* AUTODOC: disable ePort access mode for EPCL lookup */
    rc = prvTgfPclPortLookupCfgTabAccessModeSet(prvTgfPortsArray[PRV_TGF_LB_PORT_IDX_CNS],
                           CPSS_PCL_DIRECTION_EGRESS_E, CPSS_PCL_LOOKUP_0_E, 0,
                           prvTgfRestoreCfg.pclPortAccessModeCfgGet);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPclPortLookupCfgTabAccessModeSet");

    /* AUTODOC: Restore "Access to the EPCL configuration" table with Port Mode */
    rc = cpssDxChPclEgressTunnelStartPacketsCfgTabAccessModeSet(prvTgfDevNum,
                                prvTgfRestoreCfg.pclTsAccessModeCfgGet);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in cpssDxChPclEgressTunnelStartPacketsCfgTabAccessModeSet");
}

/**
* @internal prvTgfIngressMirrorSflowV5PhaConfigRestore function
* @endinternal
*
* @brief PHA test configurations restore
*/
static GT_VOID prvTgfIngressMirrorSflowV5PhaConfigRestore()
{
    GT_STATUS rc = GT_OK;

    rc = cpssDxChPhaPortThreadIdSet(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_LB_PORT_IDX_CNS],
                                    GT_FALSE, PRV_TGF_PHA_THREAD_ID_SFLOW_V5_IPV4_CNS);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "[TGF]: cpssDxChPhaPortThreadIdSet FAILED for SRC_NODE_1_SEGMENT, rc = [%d]", rc);
}

/**
* @internal prvTgfIngressMirrorSflowV5TtiConfigRestore function
* @endinternal
*
* @brief TTI test configurations restore
*/
static GT_VOID prvTgfIngressMirrorSflowV5TtiConfigRestore()
{
    GT_STATUS rc = GT_OK;

    rc = prvTgfTtiPortLookupEnableSet(
        prvTgfPortsArray[PRV_TGF_LB_PORT_IDX_CNS], PRV_TGF_TTI_KEY_ETH_E, prvTgfRestoreCfg.ttiLookupEnable);
    UTF_VERIFY_EQUAL1_STRING_MAC(
        GT_OK, rc, "prvTgfTtiPortLookupEnableSet: %d", prvTgfDevNum);

    /* AUTODOC: Invalidate TTI rule */
    rc = prvTgfTtiRuleValidStatusSet(PRV_TGF_TTI0_INDEX_CNS, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Failure in prvTgfPclRuleValidStatusSet, index = %d", 0);
}

/**
* @internal prvTgfIngressMirrorSflowV5TsConfigRestore function
* @endinternal
*
* @brief TS test configurations restore
*/
static GT_VOID prvTgfIngressMirrorSflowV5TsConfigRestore()
{
    GT_STATUS                           rc = GT_OK;
    PRV_TGF_TUNNEL_START_ENTRY_UNT      tunnelEntry;

    /* AUTODOC: Restore tunnel start entry configuration */
    cpssOsMemSet(&tunnelEntry, 0, sizeof(tunnelEntry));

    rc = prvTgfTunnelStartEntrySet(PRV_TGF_TUNNEL_START_IPV4_INDEX_CNS+8, CPSS_TUNNEL_GENERIC_E, &tunnelEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfTunnelStartEntrySet");
    rc = prvTgfTunnelStartEntrySet(PRV_TGF_TUNNEL_START_IPV4_INDEX_CNS, CPSS_TUNNEL_GENERIC_IPV4_E, &tunnelEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfTunnelStartEntrySet");

}

/**
* @internal prvTgfIngresssMirrorSflowV5LoopbackPortConfigReset function
* @endinternal
*
* @brief   Loopback config restore
*
*/
static GT_VOID  prvTgfIngresssMirrorSflowV5LoopbackPortConfigReset
(
    GT_VOID
)
{
    CPSS_INTERFACE_INFO_STC  portInterface;
    GT_STATUS rc;

    cpssOsMemSet(&portInterface, 0, sizeof(portInterface));

    portInterface.type              = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevNum;
    portInterface.devPort.portNum   = prvTgfPortsArray[PRV_TGF_LB_PORT_IDX_CNS];

    /* set loopback mode on port */
    rc = tgfTrafficGeneratorPortLoopbackModeEnableSet(&portInterface, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Failure in tgfTrafficGeneratorPortLoopbackModeEnableSet port %d",prvTgfPortsArray[PRV_TGF_LB_PORT_IDX_CNS]);

    /*give port mgr time to config the LB*/
    cpssOsTimerWkAfter(10);
}

/**
* @internal prvTgfFalconMirrorIngressSflowV5ConfigurationRestore function
* @endinternal
*
* @brief  Restore ingress mirroring functionality with sflow v5 encapsulation
*/
void prvTgfFalconMirrorIngressSflowV5ConfigurationRestore()
{
    /* restore bridge config */
    prvTgfIngressMirrorSflowV5BridgeConfigReset();

    /* restore mirror port config */
    prvTgfIngresssMirrorSflowV5MirrorPortConfigReset();

    /* restore Rx mirroring */
    prvTgfIngresssMirrorSflowV5RxPortEnableConfigReset();

    /* restore loopback port config */
    prvTgfIngresssMirrorSflowV5LoopbackPortConfigReset();

    /* restore EPCL configuration */
    prvTgfIngressMirrorSflowV5EpclConfigRstore();

    /* restore PHA config */
    prvTgfIngressMirrorSflowV5PhaConfigRestore();

    /* restore TTI Configuration */
    prvTgfIngressMirrorSflowV5TtiConfigRestore();

    /* restore TS Configuration */
    prvTgfIngressMirrorSflowV5TsConfigRestore();
}
