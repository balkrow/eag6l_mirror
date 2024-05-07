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
* @file prvTgfPtpManagerGen.c
*
* @brief CPSS Precision Time Protocol (PTP) Manager Technology facility implementation.
*
* @version   1
********************************************************************************
*/

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/generic/cpssCommonDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpssDriver/pp/config/generic/cpssDrvPpGenDump.h>
#include <cpssCommon/private/prvCpssMath.h>
#include <cpss/dxCh/dxChxGen/ptp/cpssDxChPtp.h>
#include <cpss/dxCh/dxChxGen/ptpManager/cpssDxChPtpManager.h>
#include <cpss/dxCh/dxChxGen/pha/cpssDxChPha.h>
#include <ptpManager/prvTgfPtpManagerGen.h>

#include <oam/prvTgfOamGeneral.h>
#include <cpss/generic/events/cpssGenEventRequests.h>
#include <utf/utfMain.h>
#include <utf/private/prvUtfExtras.h>
#include <utf/private/prvUtfHelpers.h>

#include <trafficEngine/tgfTrafficGenerator.h>
#include <trafficEngine/tgfTrafficTable.h>

#include <common/tgfCommon.h>
#include <common/tgfPortGen.h>
#include <common/tgfBridgeGen.h>
#include <common/tgfPtpGen.h>
#include <common/tgfPtpManagerGen.h>
#include <common/tgfCncGen.h>
#include <ptp/prvTgfPtpGen.h>
#include <stdlib.h>
#include <cnc/prvTgfCncVlanL2L3.h>
#include <common/tgfIpGen.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

/******************************************************************************\
 *                       Test configuration section                           *
\******************************************************************************/
#ifdef ASIC_SIMULATION
extern GT_STATUS wmSetPortConnectedToPhyEgress(IN GT_U32 portNum, IN GT_BOOL connected);
extern GT_STATUS wmSetPortConnectedToPhy(IN GT_U32 portNum, IN GT_BOOL connected);
extern GT_STATUS wmSetPhyIn2StepMode(IN GT_BOOL enable);
extern GT_STATUS wmGetPhyIn2StepMode(OUT GT_BOOL *enablePtr);
#endif

static CPSS_NET_RX_CPU_CODE_ENT cpuCode = CPSS_NET_FIRST_UNKNOWN_HW_CPU_CODE_E + 91;
static GT_BOOL origOamEnabler = GT_TRUE;
static PRV_TGF_OAM_COMMON_CONFIG_STC  origOamConfig;
static PRV_TGF_PTP_TS_CFG_ENTRY_STC   origTimeStampCfgEntry;
static PRV_TGF_PTP_TS_TAG_GLOBAL_CFG_STC   origTsTagGlobalCfg;
static PRV_TGF_PTP_TS_TAG_PORT_CFG_STC     origTsTagPortCfg;
static PRV_TGF_PTP_TS_TAG_PORT_CFG_STC     origTsTagPortCfg1;
static GT_BOOL origCapturePreviousMode;

/******************************* Test packet **********************************/

/******************************* common payload *******************************/
/* Data of packet (default version) */
static GT_U8 prvTgfPacketPayloadDataArr[] =
{
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f
};

/* Payload part (default version) */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketPayloadPart =
{
    sizeof(prvTgfPacketPayloadDataArr), /* dataLength */
    prvTgfPacketPayloadDataArr          /* dataPtr */
};

static TGF_PACKET_L2_STC prvTgfPacketL2Part =
{
    {0x00, 0x11, 0x22, 0x33, 0x44, 0x00},                /* daMac */
    {0x00, 0x66, 0x77, 0x88, 0x99, 0x00}                 /* saMac */
};

static TGF_PACKET_L2_STC prvTgfPacketL2Part1 =
{
    {0x00, 0x66, 0x77, 0x88, 0x99, 0x00},               /* daMac */
    {0x00, 0x11, 0x22, 0x33, 0x44, 0x00}                /* saMac */
};

static TGF_PACKET_VLAN_TAG_STC prvTgfPacketVlanPart =
{
    TGF_ETHERTYPE_8100_VLAN_TAG_CNS, 0 /*pri*/, 0 /*cfi*/,
    PRV_TGF_PTP_GEN_VLAN_CNS
};

/* ethertype part of packet */
static TGF_PACKET_ETHERTYPE_STC prvTgfPacketEtherTypePart =
{0x88F7};

static TGF_PACKET_PTP_V2_STC prvTgfPacketPtpV2Part =
{
    CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_SYNC_E    /*messageType*/,
    0xF  /*transportSpecific*/,
    2    /*ptpVersion*/,
    0    /*reserved4*/,
    (TGF_PTP_V2_HDR_SIZE_CNS + sizeof(prvTgfPacketPayloadDataArr)) /*messageLength*/,
    0    /*domainNumber*/,
    0    /*reserved8*/,
    0    /*flagField*/,
    {0, 0}   /*correctionField[2]*/,
    0    /*reserved32*/,
    {0,0,0,0,0,0,0,0,0,0}   /*sourcePortIdentify[10]*/,
    0x55 /*sequenceId*/,
    9    /*controlField*/,
    0x77 /*logMessageInterval*/
};

static TGF_PACKET_PTP_V2_STC prvTgfPacketPtpPiggyV2Part =
{
    CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_SYNC_E    /*messageType*/,
    0xF  /*transportSpecific*/,
    2    /*ptpVersion*/,
    0    /*reserved4*/,
    (TGF_PTP_V2_HDR_SIZE_CNS + sizeof(prvTgfPacketPayloadDataArr)) /*messageLength*/,
    0    /*domainNumber*/,
    0    /*reserved8*/,
    0    /*flagField*/,
    {0, 0}   /*correctionField[2]*/,
    0xAABBCCDD    /*reserved32*/,
    {0,0,0,0,0,0,0,0,0,0}   /*sourcePortIdentify[10]*/,
    0x55 /*sequenceId*/,
    9    /*controlField*/,
    0x77 /*logMessageInterval*/
};

/* PARTS of packet Ethernet Other */
static TGF_PACKET_PART_STC prvTgfPtpPiggyV2PacketPartArray[] =
{
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePart},
    {TGF_PACKET_PART_PTP_V2_E,    &prvTgfPacketPtpPiggyV2Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* Eth Other packet to send */
static TGF_PACKET_STC prvTgfPtpPiggyV2PacketInfo =
{
    TGF_PACKET_AUTO_CALC_LENGTH_CNS,               /* totalLen */
    (sizeof(prvTgfPtpPiggyV2PacketPartArray)
        / sizeof(prvTgfPtpPiggyV2PacketPartArray[0])), /* numOfParts */
    prvTgfPtpPiggyV2PacketPartArray                    /* partsArray */
};

/* Data of packet ptp version1  */
static GT_U8 prvTgfPacket1PayloadDataArr[] =
{
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f
};

static TGF_PACKET_PTP_V2_STC prvTgfPacket1PtpV2Part =
{
    CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_SYNC_E    /*messageType*/,
    0xF  /*transportSpecific*/,
    1    /*ptpVersion*/,
    0    /*reserved4*/,
    (TGF_PTP_V2_HDR_SIZE_CNS + sizeof(prvTgfPacket1PayloadDataArr)) /*messageLength*/,
    0    /*domainNumber*/,
    0    /*reserved8*/,
    0    /*flagField*/,
    {0, 0}   /*correctionField[2]*/,
    0    /*reserved32*/,
    {0,0,0,0,0,0,0,0,0,0}   /*sourcePortIdentify[10]*/,
    0x55 /*sequenceId*/,
    9    /*controlField*/,
    0x77 /*logMessageInterval*/
};

static TGF_PACKET_PTP_V2_STC prvTgfPacketSyncPtpV2Part =
{
    CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_SYNC_E    /*messageType*/,
    0xF  /*transportSpecific*/,
    2    /*ptpVersion*/,
    0    /*reserved4*/,
    (TGF_PTP_V2_HDR_SIZE_CNS + sizeof(prvTgfPacketPayloadDataArr)) /*messageLength*/,
    1    /*domainNumber*/,
    0    /*reserved8*/,
    0    /*flagField*/,
    {0x22221111, 0x22222222}   /*correctionField[2] 0x1111:fracNanoSec bytes 0x222222222222: nanoSec bytes*/,
    0    /*reserved32*/,
    {0,0,0,0,0,0,0,0,0,0}   /*sourcePortIdentify[10]*/,
    0x55 /*sequenceId*/,
    9    /*controlField*/,
    0x77 /*logMessageInterval*/
};

/* PARTS of packet Ethernet Other */
static TGF_PACKET_PART_STC prvTgfSyncPtpV2PacketPartArray[] =
{
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePart},
    {TGF_PACKET_PART_PTP_V2_E,    &prvTgfPacketSyncPtpV2Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* Eth Other packet to send */
static TGF_PACKET_STC prvTgfSyncPtpV2PacketInfo =
{
    TGF_PACKET_AUTO_CALC_LENGTH_CNS,                  /* totalLen */
    (sizeof(prvTgfSyncPtpV2PacketPartArray)
        / sizeof(prvTgfSyncPtpV2PacketPartArray[0])), /* numOfParts */
    prvTgfSyncPtpV2PacketPartArray                    /* partsArray */
};

/* Data of packet (default version) */
static GT_U8 prvTgfPacketTlvPayloadDataArr[] =
{
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* origin timestamp */
    0x01, 0x01, 0x20, 0x00, 0x02, 0x02, 0x02, 0x03, /* follow-up information TLV */
    0x03, 0x03, 0x00, 0x00, 0x00, 0x00, 0x04, 0x04,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x05, 0x05, 0x05, 0x05,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f
};

/* Payload part (default version) */
static TGF_PACKET_PAYLOAD_STC prvTgfPacketTlvPayloadPart =
{
    sizeof(prvTgfPacketTlvPayloadDataArr), /* dataLength */
    prvTgfPacketTlvPayloadDataArr          /* dataPtr */
};

/* PARTS of packet Ethernet Other */
static TGF_PACKET_PART_STC prvTgfSyncWithTlvPtpV2PacketPartArray[] =
{
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePart},
    {TGF_PACKET_PART_PTP_V2_E,    &prvTgfPacketSyncPtpV2Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketTlvPayloadPart}
};

/* Eth Other packet to send */
static TGF_PACKET_STC prvTgfSyncWithTlvPtpV2PacketInfo =
{
    TGF_PACKET_AUTO_CALC_LENGTH_CNS,                  /* totalLen */
    (sizeof(prvTgfSyncWithTlvPtpV2PacketPartArray)
        / sizeof(prvTgfSyncWithTlvPtpV2PacketPartArray[0])), /* numOfParts */
    prvTgfSyncWithTlvPtpV2PacketPartArray                    /* partsArray */
};

static TGF_PACKET_PTP_V2_STC prvTgfPacketFollowUpPtpV2Part =
{
    CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_FOLLOW_UP_E    /*messageType*/,
    0xF  /*transportSpecific*/,
    2    /*ptpVersion*/,
    0    /*reserved4*/,
    (TGF_PTP_V2_HDR_SIZE_CNS + sizeof(prvTgfPacketPayloadDataArr)) /*messageLength*/,
    1    /*domainNumber*/,
    0    /*reserved8*/,
    0    /*flagField*/,
    {0, 0}   /*correctionField[2]*/,
    0    /*reserved32*/,
    {0,0,0,0,0,0,0,0,0,0}   /*sourcePortIdentify[10]*/,
    0x55 /*sequenceId*/,
    9    /*controlField*/,
    0x77 /*logMessageInterval*/
};

/* PARTS of packet Ethernet Other */
static TGF_PACKET_PART_STC prvTgfFollowUpPtpV2PacketPartArray[] =
{
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePart},
    {TGF_PACKET_PART_PTP_V2_E,    &prvTgfPacketFollowUpPtpV2Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* Eth Other packet to send */
static TGF_PACKET_STC prvTgfFollowUpPtpV2PacketInfo =
{
    TGF_PACKET_AUTO_CALC_LENGTH_CNS,                  /* totalLen */
    (sizeof(prvTgfFollowUpPtpV2PacketPartArray)
        / sizeof(prvTgfFollowUpPtpV2PacketPartArray[0])), /* numOfParts */
    prvTgfFollowUpPtpV2PacketPartArray                    /* partsArray */
};

static TGF_PACKET_PTP_V2_STC prvTgfPacketDelayReqPtpV2Part =
{
    CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_DELAY_REQUEST_E   /*messageType*/,
    0xF  /*transportSpecific*/,
    2    /*ptpVersion*/,
    0    /*reserved4*/,
    (TGF_PTP_V2_HDR_SIZE_CNS + sizeof(prvTgfPacketPayloadDataArr)) /*messageLength*/,
    1    /*domainNumber*/,
    0    /*reserved8*/,
    0    /*flagField*/,
    {0, 0}   /*correctionField[2]*/,
    0    /*reserved32*/,
    {0,0,0,0,0,0,0,0,0,0}   /*sourcePortIdentify[10]*/,
    0x56 /*sequenceId*/,
    9    /*controlField*/,
    0x77 /*logMessageInterval*/
};

/* PARTS of packet Ethernet Other */
static TGF_PACKET_PART_STC prvTgfDelayReqPtpV2PacketPartArray[] =
{
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part1},
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePart},
    {TGF_PACKET_PART_PTP_V2_E,    &prvTgfPacketDelayReqPtpV2Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* Eth Other packet to send */
static TGF_PACKET_STC prvTgfDelayReqPtpV2PacketInfo =
{
    TGF_PACKET_AUTO_CALC_LENGTH_CNS,                  /* totalLen */
    (sizeof(prvTgfDelayReqPtpV2PacketPartArray)
        / sizeof(prvTgfDelayReqPtpV2PacketPartArray[0])), /* numOfParts */
    prvTgfDelayReqPtpV2PacketPartArray                    /* partsArray */
};

static TGF_PACKET_PTP_V2_STC prvTgfPacketPDelayReqPtpV2Part =
{
    CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_PDELAY_REQUEST_E   /*messageType*/,
    0xF  /*transportSpecific*/,
    2    /*ptpVersion*/,
    0    /*reserved4*/,
    (TGF_PTP_V2_HDR_SIZE_CNS + sizeof(prvTgfPacketPayloadDataArr)) /*messageLength*/,
    1    /*domainNumber*/,
    0    /*reserved8*/,
    0    /*flagField*/,
    {0x22221111, 0x22222222}   /*correctionField[2] 0x1111:fracNanoSec bytes 0x222222222222: nanoSec bytes*/,
    0    /*reserved32*/,
    {0,0,0,0,0,0,0,0,0,0}   /*sourcePortIdentify[10]*/,
    0x56 /*sequenceId*/,
    9    /*controlField*/,
    0x77 /*logMessageInterval*/
};

/* PARTS of packet Ethernet Other */
static TGF_PACKET_PART_STC prvTgfPDelayReqPtpV2PacketPartArray[] =
{
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePart},
    {TGF_PACKET_PART_PTP_V2_E,    &prvTgfPacketPDelayReqPtpV2Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* Eth Other packet to send */
static TGF_PACKET_STC prvTgfPDelayReqPtpV2PacketInfo =
{
    TGF_PACKET_AUTO_CALC_LENGTH_CNS,                  /* totalLen */
    (sizeof(prvTgfPDelayReqPtpV2PacketPartArray)
        / sizeof(prvTgfPDelayReqPtpV2PacketPartArray[0])), /* numOfParts */
    prvTgfPDelayReqPtpV2PacketPartArray                    /* partsArray */
};

/* PARTS of packet Ethernet Other */
static TGF_PACKET_PART_STC prvTgfPDelayReqPtpV2PacketPartArray1[] =
{
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part1},
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePart},
    {TGF_PACKET_PART_PTP_V2_E,    &prvTgfPacketPDelayReqPtpV2Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* Eth Other packet to send */
static TGF_PACKET_STC prvTgfPDelayReqPtpV2PacketInfo1 =
{
    TGF_PACKET_AUTO_CALC_LENGTH_CNS,                  /* totalLen */
    (sizeof(prvTgfPDelayReqPtpV2PacketPartArray1)
        / sizeof(prvTgfPDelayReqPtpV2PacketPartArray1[0])), /* numOfParts */
    prvTgfPDelayReqPtpV2PacketPartArray1                    /* partsArray */
};

static TGF_PACKET_PTP_V2_STC prvTgfPacketDelayResPtpV2Part =
{
    CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_DELAY_RESPONSE_E   /*messageType*/,
    0xF  /*transportSpecific*/,
    2    /*ptpVersion*/,
    0    /*reserved4*/,
    (TGF_PTP_V2_HDR_SIZE_CNS + sizeof(prvTgfPacketPayloadDataArr)) /*messageLength*/,
    1    /*domainNumber*/,
    0    /*reserved8*/,
    0    /*flagField*/,
    {0x0, 0x0}   /*correctionField[2]*/,
    0    /*reserved32*/,
    {0,0,0,0,0,0,0,0,0,0}   /*sourcePortIdentify[10]*/,
    0x57 /*sequenceId*/,
    9    /*controlField*/,
    0x77 /*logMessageInterval*/
};

/* PARTS of packet Ethernet Other */
static TGF_PACKET_PART_STC prvTgfDelayResPtpV2PacketPartArray[] =
{
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePart},
    {TGF_PACKET_PART_PTP_V2_E,    &prvTgfPacketDelayResPtpV2Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* Eth Other packet to send */
static TGF_PACKET_STC prvTgfDelayResPtpV2PacketInfo =
{
    TGF_PACKET_AUTO_CALC_LENGTH_CNS,                  /* totalLen */
    (sizeof(prvTgfDelayResPtpV2PacketPartArray)
        / sizeof(prvTgfDelayResPtpV2PacketPartArray[0])), /* numOfParts */
    prvTgfDelayResPtpV2PacketPartArray                    /* partsArray */
};

static TGF_PACKET_PTP_V2_STC prvTgfPacketPDelayResPtpV2Part =
{
    CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_PDELAY_RESPONSE_E   /*messageType*/,
    0xF  /*transportSpecific*/,
    2    /*ptpVersion*/,
    0    /*reserved4*/,
    (TGF_PTP_V2_HDR_SIZE_CNS + sizeof(prvTgfPacketPayloadDataArr)) /*messageLength*/,
    1    /*domainNumber*/,
    0    /*reserved8*/,
    0    /*flagField*/,
    {0x22221111, 0x22222222}   /*correctionField[2] 0x1111:fracNanoSec bytes 0x222222222222: nanoSec bytes*/,
    0    /*reserved32*/,
    {0,0,0,0,0,0,0,0,0,0}   /*sourcePortIdentify[10]*/,
    0x57 /*sequenceId*/,
    9    /*controlField*/,
    0x77 /*logMessageInterval*/
};

/* PARTS of packet Ethernet Other */
static TGF_PACKET_PART_STC prvTgfPDelayResPtpV2PacketPartArray[] =
{
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part1},
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePart},
    {TGF_PACKET_PART_PTP_V2_E,    &prvTgfPacketPDelayResPtpV2Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* Eth Other packet to send */
static TGF_PACKET_STC prvTgfPDelayResPtpV2PacketInfo =
{
    TGF_PACKET_AUTO_CALC_LENGTH_CNS,                  /* totalLen */
    (sizeof(prvTgfPDelayResPtpV2PacketPartArray)
        / sizeof(prvTgfPDelayResPtpV2PacketPartArray[0])), /* numOfParts */
    prvTgfPDelayResPtpV2PacketPartArray                    /* partsArray */
};

/* PARTS of packet Ethernet Other */
static TGF_PACKET_PART_STC prvTgfPDelayResPtpV2PacketPartArray1[] =
{
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},
    {TGF_PACKET_PART_VLAN_TAG_E,  &prvTgfPacketVlanPart},
    {TGF_PACKET_PART_ETHERTYPE_E, &prvTgfPacketEtherTypePart},
    {TGF_PACKET_PART_PTP_V2_E,    &prvTgfPacketPDelayResPtpV2Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfPacketPayloadPart}
};

/* Eth Other packet to send */
static TGF_PACKET_STC prvTgfPDelayResPtpV2PacketInfo1 =
{
    TGF_PACKET_AUTO_CALC_LENGTH_CNS,                  /* totalLen */
    (sizeof(prvTgfPDelayResPtpV2PacketPartArray1)
        / sizeof(prvTgfPDelayResPtpV2PacketPartArray1[0])), /* numOfParts */
    prvTgfPDelayResPtpV2PacketPartArray1                    /* partsArray */
};
/******************************************************************************\
 *                           Private declarations                             *
\******************************************************************************/
#define PRV_TGF_PTP_TS_TAG_ETHERTYPE_CNS 0xFEDC

/******************************************************************************\
 *                            Private test functions                          *
\******************************************************************************/

/********************************************************************************/
/*                                     UTILS                                    */
/********************************************************************************/

/********************************************************************************/
/*                                Test Functions                                */
/********************************************************************************/
/**
* @internal prvTgfPtpManagerPtpInitDefaultValues function
* @endinternal
*
* @brief   This routine set default values to global PTP-related settings.
*
* @param[in] outputInterfaceConfPtr   - (pointer to) clock output interface configuration.
* @param[in] ptpOverEthernetConfPtr   - (pointer to) PTP over Ethernet configuration.
* @param[in] ptpOverIpUdpConfPtr      - (pointer to) PTP over IP/UDP configuration.
* @param[in] ptpTsTagGlobalConfPtr    - (pointer to) PTP timestamp tag global configuration.
* @param[in] ptpRefClockConfPtr       - (pointer to) PTP reference clock configuration.
* @param[in] cpuCode                  - The base of CPU code assigned to PTP packets.
*/
static GT_VOID prvTgfPtpManagerPtpInitDefaultValues
(
    IN PRV_TGF_PTP_MANAGER_TAI_OUTPUT_INTERFACE_STC     *outputInterfaceConfPtr,
    IN PRV_TGF_PTP_MANAGER_PTP_OVER_ETHERNET_STC        *ptpOverEthernetConfPtr,
    IN PRV_TGF_PTP_MANAGER_PTP_OVER_IP_UDP_STC          *ptpOverIpUdpConfPtr,
    IN PRV_TGF_PTP_MANAGER_TS_TAG_GLOBAL_CFG_STC        *ptpTsTagGlobalConfPtr,
    IN PRV_TGF_PTP_MANAGER_REF_CLOCK_CFG_STC            *ptpRefClockConfPtr,
    IN CPSS_NET_RX_CPU_CODE_ENT                         *cpuCodePtr
)
{
    cpssOsBzero((GT_VOID*) outputInterfaceConfPtr, sizeof(PRV_TGF_PTP_MANAGER_TAI_OUTPUT_INTERFACE_STC));
    cpssOsBzero((GT_VOID*) ptpOverEthernetConfPtr, sizeof(PRV_TGF_PTP_MANAGER_PTP_OVER_ETHERNET_STC));
    cpssOsBzero((GT_VOID*) ptpOverIpUdpConfPtr, sizeof(PRV_TGF_PTP_MANAGER_PTP_OVER_IP_UDP_STC));
    cpssOsBzero((GT_VOID*) ptpTsTagGlobalConfPtr, sizeof(PRV_TGF_PTP_MANAGER_TS_TAG_GLOBAL_CFG_STC));
    ptpRefClockConfPtr->refClockSource = PRV_TGF_PTP_REF_CLOCK_SELECT_MAIN_25_MHZ_E;
    ptpRefClockConfPtr->refClockFrequency = PRV_TGF_PTP_25_FREQ_E;
    *cpuCodePtr = CPSS_NET_FIRST_USER_DEFINED_E;
}

/**
* @internal prvTgfPtpGenPtpOverEthEnablePtpInit function
* @endinternal
*
* @brief   None
*/
GT_STATUS prvTgfPtpManagerGenPtpOverEthEnablePtpInit
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;
    PRV_TGF_PTP_MANAGER_TAI_OUTPUT_INTERFACE_STC    outputInterfaceConf;
    PRV_TGF_PTP_MANAGER_PTP_OVER_ETHERNET_STC       ptpOverEthernetConf;
    PRV_TGF_PTP_MANAGER_PTP_OVER_IP_UDP_STC         ptpOverIpUdpConf;
    PRV_TGF_PTP_MANAGER_TS_TAG_GLOBAL_CFG_STC       ptpTsTagGlobalConf;
    PRV_TGF_PTP_MANAGER_REF_CLOCK_CFG_STC           ptpRefClockConf;
    CPSS_NET_RX_CPU_CODE_ENT                        cpuCode;

    /* set default values to global PTP-related settings */
    prvTgfPtpManagerPtpInitDefaultValues(&outputInterfaceConf,&ptpOverEthernetConf,&ptpOverIpUdpConf,&ptpTsTagGlobalConf,&ptpRefClockConf,&cpuCode);

    /* set spesific values for this test */
    ptpOverEthernetConf.ptpOverEthernetEnable = GT_TRUE;
    ptpOverEthernetConf.etherType0value = prvTgfPacketEtherTypePart.etherType;

    rc = prvTgfPtpManagerPtpInit(prvTgfDevNum,&outputInterfaceConf,&ptpOverEthernetConf,&ptpOverIpUdpConf,&ptpTsTagGlobalConf,&ptpRefClockConf,cpuCode);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPtpManagerPtpInit");

    return GT_OK;
}

/**
* @internal prvTgfPtpManagerGenPtpOverUdpEnablePtpInit function
* @endinternal
*
* @brief   None
*/
GT_STATUS prvTgfPtpManagerGenPtpOverUdpEnablePtpInit
(
    PRV_TGF_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_ENT checksumMode
)
{
    GT_STATUS rc = GT_OK;
    PRV_TGF_PTP_MANAGER_TAI_OUTPUT_INTERFACE_STC    outputInterfaceConf;
    PRV_TGF_PTP_MANAGER_PTP_OVER_ETHERNET_STC       ptpOverEthernetConf;
    PRV_TGF_PTP_MANAGER_PTP_OVER_IP_UDP_STC         ptpOverIpUdpConf;
    PRV_TGF_PTP_MANAGER_TS_TAG_GLOBAL_CFG_STC       ptpTsTagGlobalConf;
    PRV_TGF_PTP_MANAGER_REF_CLOCK_CFG_STC           ptpRefClockConf;
    CPSS_NET_RX_CPU_CODE_ENT                        cpuCode;

    /* set default values to global PTP-related settings */
    prvTgfPtpManagerPtpInitDefaultValues(&outputInterfaceConf,&ptpOverEthernetConf,&ptpOverIpUdpConf,&ptpTsTagGlobalConf,&ptpRefClockConf,&cpuCode);

    /* set spesific values for this test */
    ptpOverIpUdpConf.ptpOverIpUdpEnable = GT_TRUE;
    ptpOverIpUdpConf.udpPort0value = 0x400;
    ptpOverIpUdpConf.udpCsUpdMode.ptpIpv4Mode = checksumMode;
    ptpOverIpUdpConf.udpCsUpdMode.ptpIpv6Mode = checksumMode;

    rc = prvTgfPtpManagerPtpInit(prvTgfDevNum,&outputInterfaceConf,&ptpOverEthernetConf,&ptpOverIpUdpConf,&ptpTsTagGlobalConf,&ptpRefClockConf,cpuCode);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPtpManagerPtpInit");

    return GT_OK;
}

/**
* @internal prvTgfPtpManagerGenPtpRestore function
* @endinternal
*
* @brief   None
*/
GT_STATUS prvTgfPtpManagerGenPtpRestore
(
    GT_U32 portNum
)
{
    GT_STATUS rc = GT_OK;
    PRV_TGF_PTP_MANAGER_TAI_OUTPUT_INTERFACE_STC    outputInterfaceConf;
    PRV_TGF_PTP_MANAGER_PTP_OVER_ETHERNET_STC       ptpOverEthernetConf;
    PRV_TGF_PTP_MANAGER_PTP_OVER_IP_UDP_STC         ptpOverIpUdpConf;
    PRV_TGF_PTP_MANAGER_TS_TAG_GLOBAL_CFG_STC       ptpTsTagGlobalConf;
    PRV_TGF_PTP_MANAGER_REF_CLOCK_CFG_STC           ptpRefClockConf;
    CPSS_NET_RX_CPU_CODE_ENT                        cpuCode;
    PRV_TGF_PTP_EGRESS_DOMAIN_ENTRY_STC             domainEntry;
    PRV_TGF_PTP_MANAGER_MESSAGE_TYPE_ENT            messageType;
    CPSS_DXCH_PTP_MANAGER_DOMAIN_STC                domainConf;

    /* set default values to global PTP-related settings */
    prvTgfPtpManagerPtpInitDefaultValues(&outputInterfaceConf,&ptpOverEthernetConf,&ptpOverIpUdpConf,&ptpTsTagGlobalConf,&ptpRefClockConf,&cpuCode);

    rc = prvTgfPtpManagerPtpInit(prvTgfDevNum,&outputInterfaceConf,&ptpOverEthernetConf,&ptpOverIpUdpConf,&ptpTsTagGlobalConf,&ptpRefClockConf,cpuCode);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPtpManagerPtpInit");

    cpssOsMemSet(&domainEntry, 0, sizeof(domainEntry));

    rc = prvTgfPtpManagerPortPacketActionsConfig(prvTgfDevNum,portNum,PRV_TGF_PTP_TAI_NUMBER_0_E,
                                                 4 /*domainNum*/,&domainEntry,prvTgfPacketPtpV2Part.messageType,PRV_TGF_PTP_TS_ACTION_NONE_E);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPtpManagerPortPacketActionsConfig");

    rc = prvTgfPtpManagerPortPacketActionsConfig(prvTgfDevNum,portNum,PRV_TGF_PTP_TAI_NUMBER_0_E,
                                                 4 /*domainNum*/,&domainEntry,prvTgfPacket1PtpV2Part.messageType,PRV_TGF_PTP_TS_ACTION_NONE_E);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPtpManagerPortPacketActionsConfig");

    for (messageType=0; messageType<PRV_TGF_PTP_MANAGER_MESSAGE_TYPE_LAST_E; messageType++) {
        rc = prvTgfPtpManagerTsQueueConfig(prvTgfDevNum,messageType,0);
        PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPtpManagerTsQueueConfig");
    }

    if ((portNum != CPSS_CPU_PORT_NUM_CNS) && (PRV_CPSS_SIP_6_10_CHECK_MAC(prvTgfDevNum)))
    {
        rc = cpssDxChPtpManagerPortTimestampingModeConfig(prvTgfDevNum,portNum,CPSS_DXCH_PTP_ENHANCED_TIMESTAMPING_MODE_NONE_E,CPSS_DXCH_PTP_MANAGER_OPERATION_MODE_ONE_OR_TWO_STEP_IN_SWITCH_E);
        PRV_UTF_VERIFY_GT_OK(rc, "cpssDxChPtpManagerPortTimestampingModeConfig");
    }

    domainConf.domainIndex = 0; /* the local domain index to which the domainID is directed */
    domainConf.domainMode = CPSS_DXCH_PTP_INGRESS_DOMAIN_MODE_PTP_V2_E;
    domainConf.v2DomainId = 255; /* domainID is the domain number which is passed in the PTP header */

    /* Configure the global settings for PTP Domain */
    rc = cpssDxChPtpManagerGlobalPtpDomainSet(prvTgfDevNum,&domainConf);
    PRV_UTF_VERIFY_GT_OK(rc, "cpssDxChPtpManagerGlobalPtpDomainSet");

    domainConf.domainIndex = 2; /* the local domain index to which the domainID is directed */

    /* Configure the global settings for PTP Domain */
    rc = cpssDxChPtpManagerGlobalPtpDomainSet(prvTgfDevNum,&domainConf);
    PRV_UTF_VERIFY_GT_OK(rc, "cpssDxChPtpManagerGlobalPtpDomainSet");

    if (PRV_CPSS_SIP_6_30_CHECK_MAC(prvTgfDevNum))
    {
        rc = cpssDxChPtpManager8021AsMsgSelectionEnableSet(prvTgfDevNum,CPSS_DXCH_PTP_TAI_NUMBER_0_E,GT_FALSE,GT_FALSE);
        PRV_UTF_VERIFY_GT_OK(rc, "cpssDxChPtpManager8021AsMsgSelectionEnableSet");

        rc = cpssDxChPtpManager8021AsCumulativeScaledRateOffsetSet(prvTgfDevNum,CPSS_DXCH_PTP_TAI_NUMBER_0_E,0);
        PRV_UTF_VERIFY_GT_OK(rc, "cpssDxChPtpManager8021AsCumulativeScaledRateOffsetSet");
    }

    return GT_OK;
}

/**
* @internal prvTgfPtpManagerGenPtpDefaultInit function
* @endinternal
*
* @brief   None
*/
GT_STATUS prvTgfPtpManagerGenPtpDefaultInit
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;
    PRV_TGF_PTP_MANAGER_TAI_OUTPUT_INTERFACE_STC    outputInterfaceConf;
    PRV_TGF_PTP_MANAGER_PTP_OVER_ETHERNET_STC       ptpOverEthernetConf;
    PRV_TGF_PTP_MANAGER_PTP_OVER_IP_UDP_STC         ptpOverIpUdpConf;
    PRV_TGF_PTP_MANAGER_TS_TAG_GLOBAL_CFG_STC       ptpTsTagGlobalConf;
    PRV_TGF_PTP_MANAGER_REF_CLOCK_CFG_STC           ptpRefClockConf;
    CPSS_NET_RX_CPU_CODE_ENT                        cpuCode;

    /* set default values to global PTP-related settings */
    prvTgfPtpManagerPtpInitDefaultValues(&outputInterfaceConf,&ptpOverEthernetConf,&ptpOverIpUdpConf,&ptpTsTagGlobalConf,&ptpRefClockConf,&cpuCode);

    rc = prvTgfPtpManagerPtpInit(prvTgfDevNum,&outputInterfaceConf,&ptpOverEthernetConf,&ptpOverIpUdpConf,&ptpTsTagGlobalConf,&ptpRefClockConf,cpuCode);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPtpManagerPtpInit");

    return GT_OK;
}

/**
* @internal prvTgfPtpManagerGenAddTimeAfterPtpHeaderCfgSet
*           function
* @endinternal
*
* @brief   None
*/
GT_STATUS prvTgfPtpManagerGenAddTimeAfterPtpHeaderCfgSet
(
     GT_U32 portIdx
)
{
    GT_STATUS                             rc = GT_OK;
    PRV_TGF_PTP_EGRESS_DOMAIN_ENTRY_STC   domainEntry;

    cpssOsMemSet(&domainEntry, 0, sizeof(domainEntry));
    domainEntry.ptpOverEhernetTsEnable = GT_TRUE;
    domainEntry.ptpOverUdpIpv4TsEnable = GT_FALSE;
    domainEntry.ptpOverUdpIpv6TsEnable = GT_FALSE;
    domainEntry.messageTypeTsEnableBmp =
        (1 << prvTgfPacketPtpV2Part.messageType);
    domainEntry.transportSpecificTsEnableBmp =
        (1 << prvTgfPacketPtpV2Part.transportSpecific);

    rc = prvTgfPtpManagerPortPacketActionsConfig(prvTgfDevNum,prvTgfPortsArray[portIdx],PRV_TGF_PTP_TAI_NUMBER_0_E,
                                                 4 /*domainNum*/,&domainEntry,prvTgfPacketPtpV2Part.messageType,PRV_TGF_PTP_TS_ACTION_ADD_TIME_E);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPtpManagerPortPacketActionsConfig");

    /* Enable SIP6 frame counter, GM - TSU not supported */
    if ((!prvUtfIsGmCompilation()) && (PRV_CPSS_SIP_6_CHECK_MAC(prvTgfDevNum)))
    {
        rc = prvTgfPtpFrameCounterEnable(prvTgfPortsArray[portIdx],
                                         PRV_TGF_PTP_TS_PACKET_TYPE_PTP_V2_E,
                                         PRV_TGF_PTP_TS_ACTION_CAPTURE_ADD_TIME_E);

        PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPtpFrameCounterEnable");
    }

    return GT_OK;
}

/**
* @internal prvTgfPtpManagerGenIngressCaptureCfgSet function
* @endinternal
*
* @brief   None
*
* @param[in] receive_port_index       -Egress port index [0,3]
*                                       None
*/
GT_STATUS prvTgfPtpManagerGenIngressCaptureCfgSet
(
        IN GT_U32 receive_port_index
)
{
    GT_STATUS                             rc = GT_OK;
    PRV_TGF_PTP_EGRESS_DOMAIN_ENTRY_STC   domainEntry;

    cpssOsMemSet(&domainEntry, 0, sizeof(domainEntry));
    domainEntry.ptpOverEhernetTsEnable = GT_TRUE;
    domainEntry.ptpOverUdpIpv4TsEnable = GT_FALSE;
    domainEntry.ptpOverUdpIpv6TsEnable = GT_FALSE;
    domainEntry.messageTypeTsEnableBmp =
        (1 << prvTgfPacketPtpV2Part.messageType);
    domainEntry.transportSpecificTsEnableBmp =
        (1 << prvTgfPacketPtpV2Part.transportSpecific);

    rc = prvTgfPtpManagerPortPacketActionsConfig(prvTgfDevNum,prvTgfPortsArray[receive_port_index],PRV_TGF_PTP_TAI_NUMBER_0_E,
                                                 4 /*domainNum*/,&domainEntry,prvTgfPacketPtpV2Part.messageType,PRV_TGF_PTP_TS_ACTION_CAPTURE_INGRESS_TIME_E);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPtpManagerPortPacketActionsConfig");

    rc = prvTgfPtpManagerTsQueueConfig(prvTgfDevNum,prvTgfPacketPtpV2Part.messageType,1);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPtpManagerTsQueueConfig");

    if (PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5X_E || PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_IRONMAN_E)
    {
         /* disable TSX module; When running in MAC loopback, the link_status from the PCS to the TSX is '0' so the TSX RX can't sync. The packet
            arrives to egress of the other port with the negative CF before the update. This is caused by the invalid TS driven by the TSX RX. */
        prvTgfPtpGenTsxDisable(prvTgfPortsArray[PRV_TGF_PTP_GEN_SEND_PORT_INDEX_CNS],prvTgfPortsArray[receive_port_index]);

        rc = cpssDxChPtpManagerPortTimestampingModeConfig(prvTgfDevNum,prvTgfPortsArray[receive_port_index],CPSS_DXCH_PTP_ENHANCED_TIMESTAMPING_MODE_STANDARD_E,CPSS_DXCH_PTP_MANAGER_OPERATION_MODE_ONE_OR_TWO_STEP_IN_SWITCH_E);
        PRV_UTF_VERIFY_GT_OK(rc, "cpssDxChPtpManagerPortTimestampingModeConfig");

        rc = cpssDxChPtpManagerPortTimestampingModeConfig(prvTgfDevNum,prvTgfPortsArray[PRV_TGF_PTP_GEN_SEND_PORT_INDEX_CNS],CPSS_DXCH_PTP_ENHANCED_TIMESTAMPING_MODE_STANDARD_E,CPSS_DXCH_PTP_MANAGER_OPERATION_MODE_ONE_OR_TWO_STEP_IN_SWITCH_E);
        PRV_UTF_VERIFY_GT_OK(rc, "cpssDxChPtpManagerPortTimestampingModeConfig");

        cpssDxChPtpTsuBypassEnableSet(prvTgfDevNum,GT_FALSE);
    }
    if ((PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5P_E) || (PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_HARRIER_E))
    {
         /* disable TSX module; When running in MAC loopback, the link_status from the PCS to the TSX is '0' so the TSX RX can't sync. The packet
            arrives to egress of the other port with the negative CF before the update. This is caused by the invalid TS driven by the TSX RX. */
        prvTgfPtpGenTsxDisable(prvTgfPortsArray[PRV_TGF_PTP_GEN_SEND_PORT_INDEX_CNS],prvTgfPortsArray[receive_port_index]);
    }

    return GT_OK;
}

/**
* @internal prvTgfPtpGenIngressCaptureQueue1Check function
* @endinternal
*
* @brief   None
*/
GT_VOID prvTgfPtpManagerGenIngressCaptureQueue1Check
(
    IN  GT_U32 expected_valid
)
{
    GT_STATUS                                          rc = GT_OK;
    PRV_TGF_PTP_TS_INGRESS_TIMESTAMP_QUEUE_ENTRY_STC   tgfTsQueueEntry;

    rc = prvTgfPtpManagerIngressTimestampGlobalQueueEntryRead(prvTgfDevNum,1 /*queueNum*/, &tgfTsQueueEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPtpManagerIngressTimestampGlobalQueueEntryRead");

    if (GT_TRUE == expected_valid)
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, tgfTsQueueEntry.entryValid, "entryValid");
    }
    else
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_FALSE, tgfTsQueueEntry.entryValid, "entryValid");
        return;
    }

    UTF_VERIFY_EQUAL0_STRING_MAC(PRV_TGF_PTP_TS_PACKET_TYPE_PTP_V2_E  , tgfTsQueueEntry.packetFormat, "packetFormat");

    UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfPacketPtpV2Part.sequenceId  , tgfTsQueueEntry.sequenceId, "sequenceId");

    PRV_UTF_LOG1_MAC("[TGF]: prvTgfPtpGenIngressCaptureQueue1Check end. the entry is valid: %d\n",tgfTsQueueEntry.entryValid);
    PRV_UTF_LOG2_MAC("Ingress Timestamp Queue Entry: nano 0x%08X port %d\n",tgfTsQueueEntry.timestamp,tgfTsQueueEntry.portNum);
    PRV_UTF_LOG4_MAC("domainNum %d taiSelect %d messageType %d sequenceId %d\n",tgfTsQueueEntry.domainNum,tgfTsQueueEntry.taiSelect,tgfTsQueueEntry.messageType,tgfTsQueueEntry.sequenceId);

}

/**
* @internal prvTgfPtpManagerGenTsTagPortCfgPiggyBackSet function
* @endinternal
*
* @brief   set per port configuration timestamp tag Piggyback
*
* @param[in] port- port number
* @param[in] testNum- test number
*   in test 1: TST on egress and packet is incoming with TST - (modify the value);
*              Expect capture packet to have: timestamp tag in <resreved> field == timestamp tag in ingress queue entry
*   in test 2: non TST on egress port (non CPU) and packet is incoming with TST - remove it (action: capture_ingress_time)
*             Expect capture packet to have: timestamp tag in <resreved> field == '0'
*/
GT_STATUS prvTgfPtpManagerGenTsTagPortCfgPiggyBackSet
(
        IN GT_U32 port,
        IN GT_U32 testNum
)
{
    PRV_TGF_PTP_TS_TAG_PORT_CFG_STC     tsTagPortCfg;
    GT_STATUS                           rc;

    if (testNum == 1)
    {
        PRV_UTF_LOG0_MAC("\ntest #1: TST on egress and packet is incoming with TST - (modify the value) (action: capture_ingress_time)");
        PRV_UTF_LOG0_MAC("\n         Expect capture packet to have: timestamp tag in <resreved> field == timestamp tag in ingress queue entry\n\n");

        /* AUTODOC: Set per port configuration timestamp tag */
        tsTagPortCfg.tsReceptionEnable = GT_TRUE; /* the packet has TST inside reserved field. */
        tsTagPortCfg.tsPiggyBackEnable = GT_TRUE;
        tsTagPortCfg.tsTagMode = PRV_TGF_PTP_TS_TAG_MODE_PIGGYBACK_E;

        rc = prvTgfPtpTsTagPortCfgGet(prvTgfPortsArray[port],&origTsTagPortCfg);
        PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPtpTsTagPortCfgGet");

        rc = prvTgfPtpTsTagPortCfgGet(CPSS_CPU_PORT_NUM_CNS,&origTsTagPortCfg1);
        PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPtpTsTagPortCfgGet");

        rc = prvTgfPtpManagerPortTsTagConfig(prvTgfDevNum,prvTgfPortsArray[port],&tsTagPortCfg); /* egress port */
        PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPtpManagerPortTsTagConfig");

        rc = prvTgfPtpManagerPortTsTagConfig(prvTgfDevNum,CPSS_CPU_PORT_NUM_CNS/*63*/,&tsTagPortCfg);
        PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPtpManagerPortTsTagConfig");

    }
    if (testNum == 2)
    {
        PRV_UTF_LOG0_MAC("\ntest #2: non TST on egress port (non CPU) and packet is incoming with TST - remove it (action: capture_ingress_time)");
        PRV_UTF_LOG0_MAC("\n         Expect capture packet to have: timestamp tag in <resreved> field == '0'\n\n");

        /* restore configuiration */
        rc = prvTgfPtpManagerPortTsTagConfig(prvTgfDevNum,prvTgfPortsArray[port],&origTsTagPortCfg);
        PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPtpManagerPortTsTagConfig");

        rc = prvTgfPtpManagerPortTsTagConfig(prvTgfDevNum,CPSS_CPU_PORT_NUM_CNS,&origTsTagPortCfg1);
        PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPtpManagerPortTsTagConfig");

        /* AUTODOC: Set per port configuration timestamp tag */
        tsTagPortCfg.tsReceptionEnable = GT_TRUE; /* the packet has TST inside reserved field. */
        tsTagPortCfg.tsPiggyBackEnable = GT_TRUE;
        tsTagPortCfg.tsTagMode = PRV_TGF_PTP_TS_TAG_MODE_PIGGYBACK_E;

        rc = prvTgfPtpTsTagPortCfgGet(prvTgfPortsArray[1],&origTsTagPortCfg1); /* ingress port */
        PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPtpTsTagPortCfgGet");

        rc = prvTgfPtpManagerPortTsTagConfig(prvTgfDevNum,prvTgfPortsArray[1],&tsTagPortCfg);
        PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPtpManagerPortTsTagConfig");
    }
    return GT_OK;
}

/**
* @internal prvTgfPtpManagerGenTsTagPortCfgHybridTagSet function
* @endinternal
*
* @brief   set per port configuration timestamp tag Hybrid
*
* @param[in] port- port number
* @param[in] testNum- test number:
*   in test 1: non TST on egress port (CPU) and packet is incoming with TST - remove it
*   in test 2: non TST on egress port (non CPU) and packet is incoming with TST - remove it (action: capture_ingress_time)
*   in test 3: non TST on egress port (non CPU) and packet is incoming with TST - remove it (action: add_corrected_time) according to ingress TST
*   in test 4: TST on egress and packet is incoming with TST - preserve incoming (don't modify the value)
*
*/
GT_STATUS prvTgfPtpManagerGenTsTagPortCfgHybridTagSet
(
        IN GT_U32 port,
        IN GT_U32 testNum
)
{
    PRV_TGF_PTP_TS_TAG_PORT_CFG_STC     tsTagPortCfg,tsTagPortCfg1;
    GT_STATUS                           rc;
    PRV_TGF_PTP_TS_TAG_GLOBAL_CFG_STC   tsTagGlobalCfg;
    PRV_TGF_PTP_EGRESS_DOMAIN_ENTRY_STC   domainEntry;

    tsTagPortCfg.tsReceptionEnable = GT_TRUE;
    tsTagPortCfg.tsPiggyBackEnable = GT_FALSE;
    tsTagPortCfg.tsTagMode = PRV_TGF_PTP_TS_TAG_MODE_HYBRID_E;

    tsTagPortCfg1.tsReceptionEnable = GT_FALSE;
    tsTagPortCfg1.tsPiggyBackEnable = GT_FALSE;
    tsTagPortCfg1.tsTagMode = PRV_TGF_PTP_TS_TAG_MODE_NONE_E;

    if (testNum == 1)
    {
        PRV_UTF_LOG0_MAC("\ntest #1: non TST on egress port (CPU) and packet is incoming with TST - remove it\n\n");

        rc = prvTgfPtpTsTagPortCfgGet(prvTgfPortsArray[port],&origTsTagPortCfg);
        PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPtpTsTagPortCfgGet");

        rc = prvTgfPtpTsTagPortCfgGet(CPSS_CPU_PORT_NUM_CNS/*63*/,&origTsTagPortCfg1);
        PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPtpTsTagPortCfgGet");

        rc = prvTgfPtpTsTagGlobalCfgGet(&origTsTagGlobalCfg);
        PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPtpTsTagGlobalCfgGet");
    }

    if (testNum == 2)
    {
        PRV_UTF_LOG0_MAC("\ntest #2: non TST on egress port (non CPU) and packet is incoming with TST - remove it (action: capture_ingress_time)\n\n");

        rc = prvTgfPtpManagerPortTsTagConfig(prvTgfDevNum,prvTgfPortsArray[1] /* ingress hybrid TST */,&tsTagPortCfg);
        PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPtpManagerPortTsTagConfig");

        rc = prvTgfPtpManagerPortTsTagConfig(prvTgfDevNum,prvTgfPortsArray[port] /* egress non TST*/,&tsTagPortCfg1);
        PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPtpManagerPortTsTagConfig");
    }
    else if (testNum == 3)
    {
        PRV_UTF_LOG0_MAC("\ntest #3: non TST on egress port (non CPU) and packet is incoming with TST - remove it (action: add_corrected_time) according to ingress TST\n\n");

        rc = prvTgfPtpManagerPortTsTagConfig(prvTgfDevNum,prvTgfPortsArray[1] /* ingress hybrid TST */,&tsTagPortCfg);
        PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPtpManagerPortTsTagConfig");

        rc = prvTgfPtpManagerPortTsTagConfig(prvTgfDevNum,prvTgfPortsArray[port] /* egress non TST*/,&tsTagPortCfg1);
        PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPtpManagerPortTsTagConfig");
    }
    else
    {
        rc = prvTgfPtpManagerPortTsTagConfig(prvTgfDevNum,prvTgfPortsArray[port],&tsTagPortCfg);
        PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPtpManagerPortTsTagConfig");
    }

    if (testNum == 3)
    {
        cpssOsMemSet(&domainEntry, 0, sizeof(domainEntry));
        domainEntry.ptpOverEhernetTsEnable = GT_TRUE;
        domainEntry.ptpOverUdpIpv4TsEnable = GT_FALSE;
        domainEntry.ptpOverUdpIpv6TsEnable = GT_FALSE;
        domainEntry.messageTypeTsEnableBmp =
            (1 << prvTgfPacketPtpV2Part.messageType);
        domainEntry.transportSpecificTsEnableBmp =
            (1 << prvTgfPacketPtpV2Part.transportSpecific);

        rc = prvTgfPtpManagerPortPacketActionsConfig(prvTgfDevNum,prvTgfPortsArray[port],PRV_TGF_PTP_TAI_NUMBER_0_E,
                                                     4 /*domainNum*/,&domainEntry,prvTgfPacketPtpV2Part.messageType,PRV_TGF_PTP_TS_ACTION_ADD_CORRECTED_TIME_E);
        PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPtpManagerPortPacketActionsConfig");
    }
    if (testNum == 4)
    {
        PRV_UTF_LOG0_MAC("\ntest #4: TST on egress and packet is incoming with TST - preserve incoming (don't modify the value)\n\n");

        rc = prvTgfPtpManagerPortTsTagConfig(prvTgfDevNum,CPSS_CPU_PORT_NUM_CNS/*63*/,&tsTagPortCfg);
        PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPtpManagerPortTsTagConfig");

        cpssOsMemSet(&domainEntry, 0, sizeof(domainEntry));
        domainEntry.ptpOverEhernetTsEnable = GT_TRUE;
        domainEntry.ptpOverUdpIpv4TsEnable = GT_FALSE;
        domainEntry.ptpOverUdpIpv6TsEnable = GT_FALSE;
        domainEntry.messageTypeTsEnableBmp =
            (1 << prvTgfPacketPtpV2Part.messageType);
        domainEntry.transportSpecificTsEnableBmp =
            (1 << prvTgfPacketPtpV2Part.transportSpecific);

        rc = prvTgfPtpManagerPortPacketActionsConfig(prvTgfDevNum,prvTgfPortsArray[port],PRV_TGF_PTP_TAI_NUMBER_0_E,
                                                     4 /*domainNum*/,&domainEntry,prvTgfPacketPtpV2Part.messageType,PRV_TGF_PTP_TS_ACTION_CAPTURE_INGRESS_TIME_E);
        PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPtpManagerPortPacketActionsConfig");
    }

    tsTagGlobalCfg.tsTagParseEnable      = GT_FALSE;
    tsTagGlobalCfg.hybridTsTagParseEnable = GT_TRUE;
    tsTagGlobalCfg.tsTagEtherType       = 0x0;
    tsTagGlobalCfg.hybridTsTagEtherType = PRV_TGF_PTP_TS_TAG_ETHERTYPE_CNS;

    rc = prvTgfPtpTsTagGlobalCfgSet(&tsTagGlobalCfg);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPtpTsTagGlobalCfgSet");

    return GT_OK;
}

/**
* @internal prvTgfPtpManagerGenTsTagPortCfgPiggyBackRestore
*           function
* @endinternal
*
* @brief   restore per port configuration timestamp tag Piggyback
*
* @param[in] port- port number
*
*/
GT_STATUS prvTgfPtpManagerGenTsTagPortCfgPiggyBackRestore
(
        IN GT_U32 port
)
{
    GT_STATUS                           rc;

    /* AUTODOC: Set per port configuration timestamp tag */
    rc = prvTgfPtpManagerPortTsTagConfig(prvTgfDevNum,prvTgfPortsArray[port],&origTsTagPortCfg);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPtpManagerPortTsTagConfig");

    rc = prvTgfPtpManagerPortTsTagConfig(prvTgfDevNum,CPSS_CPU_PORT_NUM_CNS,&origTsTagPortCfg1);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPtpManagerPortTsTagConfig");

    /* Restore TSX module configuration */
    if ((PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5X_E) || (PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5P_E) ||
        (PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_HARRIER_E) || (PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_IRONMAN_E))
    {
        prvTgfPtpGenTsxRestore(prvTgfPortsArray[PRV_TGF_PTP_GEN_SEND_PORT_INDEX_CNS],prvTgfPortsArray[port]);
    }

    return GT_OK;
}

/**
* @internal prvTgfPtpManagerGenEgressCaptureCfgSet function
* @endinternal
*
* @brief   None
*
* @param[in] receive_port_index       -Egress port index [0,3]
*                                       None
*/
GT_STATUS prvTgfPtpManagerGenEgressCaptureCfgSet
(
        IN GT_U32 receive_port_index
)
{
    GT_STATUS                             rc = GT_OK;
    PRV_TGF_PTP_EGRESS_DOMAIN_ENTRY_STC   domainEntry;

    cpssOsMemSet(&domainEntry, 0, sizeof(domainEntry));
    domainEntry.ptpOverEhernetTsEnable = GT_TRUE;
    domainEntry.ptpOverUdpIpv4TsEnable = GT_FALSE;
    domainEntry.ptpOverUdpIpv6TsEnable = GT_FALSE;
    domainEntry.messageTypeTsEnableBmp =
        (1 << prvTgfPacketPtpV2Part.messageType);
    domainEntry.transportSpecificTsEnableBmp =
        (1 << prvTgfPacketPtpV2Part.transportSpecific);

    rc = prvTgfPtpManagerPortPacketActionsConfig(prvTgfDevNum,prvTgfPortsArray[receive_port_index],PRV_TGF_PTP_TAI_NUMBER_0_E,
                                                 4 /*domainNum*/,&domainEntry,prvTgfPacketPtpV2Part.messageType,PRV_TGF_PTP_TS_ACTION_CAPTURE_E);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPtpManagerPortPacketActionsConfig");

    rc = prvTgfPtpManagerTsQueueConfig(prvTgfDevNum,prvTgfPacketPtpV2Part.messageType,1);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPtpManagerTsQueueConfig");

    /* Configure Enhanced Timestamping Mode per Egress port */
    if (PRV_CPSS_SIP_6_30_CHECK_MAC(prvTgfDevNum))
    {
        rc = cpssDxChPtpManagerPortTimestampingModeConfig(prvTgfDevNum,prvTgfPortsArray[receive_port_index],CPSS_DXCH_PTP_ENHANCED_TIMESTAMPING_MODE_STANDARD_E,CPSS_DXCH_PTP_MANAGER_OPERATION_MODE_ONE_OR_TWO_STEP_IN_MAC_E);
        PRV_UTF_VERIFY_GT_OK(rc, "cpssDxChPtpManagerPortTimestampingModeConfig");
    }
    else if (PRV_CPSS_SIP_6_10_CHECK_MAC(prvTgfDevNum))
    {
        rc = cpssDxChPtpManagerPortTimestampingModeConfig(prvTgfDevNum,prvTgfPortsArray[receive_port_index],CPSS_DXCH_PTP_ENHANCED_TIMESTAMPING_MODE_STANDARD_E,CPSS_DXCH_PTP_MANAGER_OPERATION_MODE_ONE_OR_TWO_STEP_IN_SWITCH_E);
        PRV_UTF_VERIFY_GT_OK(rc, "cpssDxChPtpManagerPortTimestampingModeConfig");
    }

    return GT_OK;
}

#define PRV_TGF_PTP_GEN_TOD_SEC_CNS                    200000

/**
* @internal prvTgfPtpManagerGenEgressCaptureQueue1Check function
* @endinternal
*
* @brief   None
*/
GT_VOID prvTgfPtpManagerGenEgressCaptureQueue1Check
(
     IN GT_U32 expected_valid,
     IN GT_U32 receive_port_index
)
{
    GT_STATUS                                          rc = GT_OK;
    PRV_TGF_PTP_TS_EGRESS_TIMESTAMP_QUEUE_ENTRY_STC    tgfTsQueueEntry;
    PRV_TGF_PTP_MANAGER_TSU_TX_TIMESTAMP_QUEUE_ENTRY_STC tgfTsPortEntry;
    CPSS_DXCH_PTP_MANAGER_MAC_TX_TIMESTAMP_QUEUE_ENTRY_STC macTsQueueEntry;

    rc =  prvTgfPtpManagerEgressTimestampGlobalQueueEntryRead(prvTgfDevNum,1 /*queueNum*/, &tgfTsQueueEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPtpManagerEgressTimestampGlobalQueueEntryRead");

    if (GT_TRUE == expected_valid)
    {
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, tgfTsQueueEntry.entryValid, "entryValid");

        PRV_UTF_LOG3_MAC("[TGF]: cpssDxChPtpManagerEgressTimestampGlobalQueueEntryRead: valid entry [%d] queueEntryId [%d] portNum [%d]\n",tgfTsQueueEntry.entryValid, tgfTsQueueEntry.queueEntryId, tgfTsQueueEntry.portNum);
    }
    else
    {
        /* verify global Egress queue is empty */
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_FALSE, tgfTsQueueEntry.entryValid, "entryValid");

        /* verify Egress queue per port is also empty */
        rc = prvTgfPtpManagerEgressTimestampPortQueueEntryRead(prvTgfDevNum,prvTgfPortsArray[receive_port_index],1,&tgfTsPortEntry);
        UTF_VERIFY_EQUAL1_STRING_MAC(
            GT_OK, rc, "cpssDxChPtpManagerEgressTimestampPortQueueEntryRead, rc is %d queue 1",rc);

        UTF_VERIFY_EQUAL0_STRING_MAC(GT_FALSE, tgfTsPortEntry.entryValid, "entryValid");

        /* verify MAC Egress queue per port is also empty*/
        if (PRV_CPSS_SIP_6_30_CHECK_MAC(prvTgfDevNum))
        {
            rc = cpssDxChPtpManagerMacEgressTimestampPortQueueEntryRead(prvTgfDevNum,prvTgfPortsArray[receive_port_index],&macTsQueueEntry);
            UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChPtpManagerMacEgressTimestampPortQueueEntryRead, rc is %d",rc);

            UTF_VERIFY_EQUAL0_STRING_MAC(GT_FALSE, macTsQueueEntry.entryValid, "entryValid");
        }

        if (PRV_CPSS_SIP_6_30_CHECK_MAC(prvTgfDevNum))
        {
            PRV_UTF_LOG3_MAC("[TGF]: prvTgfPtpManagerGenEgressCaptureQueue1Check end. valid entries: %d %d %d\n\n",tgfTsQueueEntry.entryValid,tgfTsPortEntry.entryValid,macTsQueueEntry.entryValid);
        }
        else
        {
            PRV_UTF_LOG2_MAC("[TGF]: prvTgfPtpManagerGenEgressCaptureQueue1Check end. valid entries: %d %d\n\n",tgfTsQueueEntry.entryValid,tgfTsPortEntry.entryValid);
        }
        return;
    }

    UTF_VERIFY_EQUAL0_STRING_MAC(PRV_TGF_PTP_TS_PACKET_TYPE_PTP_V2_E  , tgfTsQueueEntry.packetFormat, "packetFormat");

    UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfPacketPtpV2Part.sequenceId  , tgfTsQueueEntry.sequenceId, "sequenceId");

    if(PRV_CPSS_PP_MAC(prvTgfDevNum) && tgfTsQueueEntry.entryValid)
    {
        rc = prvTgfPtpManagerEgressTimestampPortQueueEntryRead(prvTgfDevNum,tgfTsQueueEntry.portNum,1,&tgfTsPortEntry);
        UTF_VERIFY_EQUAL1_STRING_NO_RETURN_MAC(GT_OK, rc, "prvTgfPtpManagerEgressTimestampPortQueueEntryRead, rc is %d queue 1",rc);

        PRV_UTF_LOG3_MAC("[TGF]: cpssDxChPtpManagerEgressTimestampPortQueueEntryRead: valid entry [%d] queueEntryId [%d] portNum [%d]\n",tgfTsPortEntry.entryValid, tgfTsPortEntry.queueEntryId, tgfTsQueueEntry.portNum);
    }

    if (PRV_CPSS_SIP_6_30_CHECK_MAC(prvTgfDevNum) && tgfTsQueueEntry.entryValid)
    {
        rc = cpssDxChPtpManagerMacEgressTimestampPortQueueEntryRead(prvTgfDevNum,tgfTsQueueEntry.portNum,&macTsQueueEntry);
        UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "cpssDxChPtpManagerMacEgressTimestampPortQueueEntryRead, rc is %d",rc);

        UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, macTsQueueEntry.entryValid, "entryValid");

        PRV_UTF_LOG3_MAC("[TGF]: cpssDxChPtpManagerMacEgressTimestampPortQueueEntryRead: valid entry [%d] queueEntryId [%d] portNum [%d]\n",macTsQueueEntry.entryValid, tgfTsPortEntry.queueEntryId, tgfTsQueueEntry.portNum);

        UTF_VERIFY_EQUAL0_STRING_MAC(macTsQueueEntry.signature.signatureQueueEntryId.queueSelect,1,"queueSelect");

        UTF_VERIFY_EQUAL0_STRING_MAC(macTsQueueEntry.signature.signatureQueueEntryId.queueEntryId,tgfTsPortEntry.queueEntryId,"queueEntryId");
    }

    if (PRV_CPSS_SIP_6_30_CHECK_MAC(prvTgfDevNum))
    {
        PRV_UTF_LOG3_MAC("[TGF]: prvTgfPtpManagerGenEgressCaptureQueue1Check end. valid entries: %d %d %d\n\n",tgfTsQueueEntry.entryValid,tgfTsPortEntry.entryValid,macTsQueueEntry.entryValid);
    }
    else
    {
        PRV_UTF_LOG2_MAC("[TGF]: prvTgfPtpManagerGenEgressCaptureQueue1Check end. valid entries: %d %d\n\n",tgfTsQueueEntry.entryValid,tgfTsPortEntry.entryValid);
    }

}

/**
* @internal
*           prvTgfPtpManagerGenPiggyBackTimestampTrafficGenerateAndCheck
*           function
* @endinternal
*
* @brief  Generate traffic:
*         Send to device's port given packet under capture.
*         Check captured packet.
*
* @param[in] testNum- test number
*
*/
GT_VOID prvTgfPtpManagerGenPiggyBackTimestampTrafficGenerateAndCheck
(
    IN GT_U32 testNum
)
{
    GT_STATUS       rc;
    GT_U32          packetActualLength = 0;
    GT_U8           packetBuff[TGF_RX_BUFFER_MAX_SIZE_CNS] = {0};
    GT_U32          buffLen = TGF_RX_BUFFER_MAX_SIZE_CNS;
    PRV_TGF_PTP_TS_INGRESS_TIMESTAMP_QUEUE_ENTRY_STC   tgfTsQueueEntry;
    CPSS_INTERFACE_INFO_STC portInterface;
    TGF_NET_DSA_STC rxParam;
    GT_U8           queue    = 0;
    GT_U8           receiveDevNum;
    GT_U32          reservedFieldOffset;
    GT_32           reservedFieldValue;

    cpssOsMemSet(&rxParam, 0, sizeof(rxParam));
    cpssOsMemSet(&portInterface, 0, sizeof(portInterface));
    portInterface.type = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevsArray[PRV_TGF_PTP_GEN_RECEIVE_PORT_INDEX_CNS];
    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_PTP_GEN_RECEIVE_PORT_INDEX_CNS];

    /* AUTODOC: Send PTP packet with reserved = 0 and get it's updated
       value by capture egress packet */
    PRV_UTF_LOG0_MAC("Send Packet to Port");
    rc = prvTgfTransmitPacketsWithCapture(
        prvTgfDevsArray[PRV_TGF_PTP_GEN_SEND_PORT_INDEX_CNS],
        prvTgfPortsArray[PRV_TGF_PTP_GEN_SEND_PORT_INDEX_CNS],
        &prvTgfPtpPiggyV2PacketInfo,
        1      /*burstCount*/,
        0      /*numVfd*/,
        NULL   /*vfdArray*/,
        prvTgfDevsArray[PRV_TGF_PTP_GEN_RECEIVE_PORT_INDEX_CNS],
        prvTgfPortsArray[PRV_TGF_PTP_GEN_RECEIVE_PORT_INDEX_CNS],
        TGF_CAPTURE_MODE_MIRRORING_E,
        200                                /*captureOnTime*/);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfTransmitPacketsWithCapture");

    PRV_UTF_LOG1_MAC(
        "Capture Packet from Port [%d]",
        portInterface.devPort.portNum);

    /* AUTODOC: Get first entry from rxNetworkIf table */
    rc = tgfTrafficGeneratorRxInCpuGet(
        TGF_PACKET_TYPE_CAPTURE_E,
        GT_TRUE/*getFirst*/, GT_TRUE /*trace*/, packetBuff,
        &buffLen, &packetActualLength,
        &receiveDevNum, &queue, &rxParam);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "tgfTrafficGeneratorRxInCpuGet");

    reservedFieldOffset = TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS
                       + TGF_ETHERTYPE_SIZE_CNS + 16 /* Offset in PTP header */;

    reservedFieldValue = ((packetBuff[reservedFieldOffset] << 24) +
                          (packetBuff[reservedFieldOffset+1] << 16) +
                          (packetBuff[reservedFieldOffset+2] << 8) +
                           packetBuff[reservedFieldOffset+3]);
    PRV_UTF_LOG1_MAC("Capture Packet reservedField: nano 0x%08X\n",reservedFieldValue);


    /* AUTODOC: Read Ingress Timestamp Queue First Entry */
    rc = prvTgfPtpManagerIngressTimestampGlobalQueueEntryRead(prvTgfDevNum,1 /*queueNum*/, &tgfTsQueueEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPtpManagerIngressTimestampGlobalQueueEntryRead");
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, tgfTsQueueEntry.entryValid, "entryValid");

    PRV_UTF_LOG1_MAC("Ingress Timestamp Queue Entry: nano 0x%08X\n",tgfTsQueueEntry.timestamp);


    if (testNum == 1)
    {
        if(PRV_CPSS_SIP_6_10_CHECK_MAC(prvTgfDevNum))
        {
            if (tgfTsQueueEntry.timestamp != reservedFieldValue)
            {
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, GT_FAIL, "timestamp [0x%08X] != reserved field [0x%08X]\n",tgfTsQueueEntry.timestamp,reservedFieldValue);
            }
            else
            {
                PRV_UTF_LOG0_MAC("\n[TGF]: The entry is valid and timestamp is equall to reserved field.\n");
            }
        }
        else
        {
            /* the 2 LSBits of the ns value are NOT overridden with the T (TAI select - 0/1) and U (update counter LSB).*/
            if ((tgfTsQueueEntry.timestamp >> 2) != (reservedFieldValue >> 2))
            {
                UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, GT_FAIL, "timestamp [0x%08X] != reserved field [0x%08X]\n",tgfTsQueueEntry.timestamp,reservedFieldValue);
            }
            else
            {
                PRV_UTF_LOG0_MAC("\n[TGF]: The entry is valid and timestamp is equall to reserved field.\n");
            }
        }
    }

    if (testNum == 2)
    {
        if (reservedFieldValue != 0)
        {
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, GT_FAIL, "reserved field is not '0'\n");
        }

        /* restore configuiration */
        rc = prvTgfPtpManagerPortTsTagConfig(prvTgfDevNum,prvTgfPortsArray[1],&origTsTagPortCfg1);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPtpManagerPortTsTagConfig");
    }
}

/**
* @internal
*           prvTgfPtpManagerGenHybridTimestampTrafficGenerateAndCheck
*           function
* @endinternal
*
* @brief  Generate traffic:
*         Send to device's port given packet under capture.
*         Check captured packet.
*
* @param[in] testNum- test number:
*   in test 1: non TST on egress port (CPU) and packet is incoming with TST - remove it
*   in test 2: non TST on egress port (non CPU) and packet is incoming with TST - remove it (action: capture_ingress_time)
*   in test 3: non TST on egress port (non CPU) and packet is incoming with TST - remove it (action: add_corrected_time) according to ingress TST
*   in test 4: TST on egress and packet is incoming with TST - preserve incoming (don't modify the value)
*
*/
GT_VOID prvTgfPtpManagerGenHybridTimestampTrafficGenerateAndCheck
(
    IN GT_U32 testNum
)
{
    GT_STATUS       rc;
    GT_U32          packetActualLength = 0;
    GT_U8           packetBuff[TGF_RX_BUFFER_MAX_SIZE_CNS] = {0};
    GT_U32          buffLen = TGF_RX_BUFFER_MAX_SIZE_CNS;
    CPSS_INTERFACE_INFO_STC portInterface;
    TGF_NET_DSA_STC rxParam;
    GT_U8           queue    = 0;
    GT_U8           receiveDevNum;
    GT_U32          reservedFieldOffset,correctionFieldOffset;
    GT_U32          reservedFieldValue;
    GT_32           memCmpRc;

    cpssOsMemSet(&rxParam, 0, sizeof(rxParam));
    cpssOsMemSet(&portInterface, 0, sizeof(portInterface));
    portInterface.type = CPSS_INTERFACE_PORT_E;
    portInterface.devPort.hwDevNum  = prvTgfDevsArray[PRV_TGF_PTP_GEN_RECEIVE_PORT_INDEX_CNS];
    portInterface.devPort.portNum = prvTgfPortsArray[PRV_TGF_PTP_GEN_RECEIVE_PORT_INDEX_CNS];

    /* AUTODOC: Send PTP packet with reserved = 0 and get it's updated
       value by capture egress packet */
    PRV_UTF_LOG0_MAC("Send Packet to Port");
    rc = prvTgfTransmitPacketsWithCapture(
        prvTgfDevsArray[PRV_TGF_PTP_GEN_SEND_PORT_INDEX_CNS],
        prvTgfPortsArray[PRV_TGF_PTP_GEN_SEND_PORT_INDEX_CNS],
        &prvTgfPtpPiggyV2PacketInfo,
        1      /*burstCount*/,
        0      /*numVfd*/,
        NULL   /*vfdArray*/,
        prvTgfDevsArray[PRV_TGF_PTP_GEN_RECEIVE_PORT_INDEX_CNS],
        prvTgfPortsArray[PRV_TGF_PTP_GEN_RECEIVE_PORT_INDEX_CNS],
        TGF_CAPTURE_MODE_MIRRORING_E,
        200                                /*captureOnTime*/);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "prvTgfTransmitPacketsWithCapture");

    PRV_UTF_LOG1_MAC(
        "Capture Packet from Port [%d]",
        portInterface.devPort.portNum);

    /* AUTODOC: Get first entry from rxNetworkIf table */
    rc = tgfTrafficGeneratorRxInCpuGet(
        TGF_PACKET_TYPE_CAPTURE_E,
        GT_TRUE/*getFirst*/, GT_TRUE /*trace*/, packetBuff,
        &buffLen, &packetActualLength,
        &receiveDevNum, &queue, &rxParam);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        GT_OK, rc, "tgfTrafficGeneratorRxInCpuGet");

    /* AUTODOC: restore per port configuration timestamp tag */
    rc = prvTgfPtpManagerPortTsTagConfig(prvTgfDevNum,CPSS_CPU_PORT_NUM_CNS/*63*/,&origTsTagPortCfg1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPtpManagerPortTsTagConfig");

    /* AUTODOC: restore per port configuration timestamp tag */
    rc = prvTgfPtpManagerPortTsTagConfig(prvTgfDevNum,prvTgfPortsArray[1] ,&origTsTagPortCfg1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPtpManagerPortTsTagConfig");

    /* AUTODOC: restore Timestamp Global Configuration */
    rc = prvTgfPtpTsTagGlobalCfgSet(&origTsTagGlobalCfg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPtpTsTagGlobalCfgSet");

    if (testNum == 4)
    {
        if ((packetBuff[12] != ((PRV_TGF_PTP_TS_TAG_ETHERTYPE_CNS >> 8) & 0xFF)) ||
            (packetBuff[13] != (PRV_TGF_PTP_TS_TAG_ETHERTYPE_CNS & 0xFF)) ||
            (packetBuff[20] != 0x81))
        {
            rc = GT_FAIL;
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "packetBuff[12] =  %2.2x (expected 0xfe), packetBuff[13] =  %2.2x (expected 0xdc), "
                                                    "packetBuff[20] =  %2.2x (expected 0x81)",
                                         packetBuff[12], packetBuff[13], packetBuff[20]);
        }

        reservedFieldOffset = TGF_L2_HEADER_SIZE_CNS + TGF_ETHERTYPE_SIZE_CNS + 6 /* 6B seconds */ + TGF_VLAN_TAG_SIZE_CNS
                            + TGF_ETHERTYPE_SIZE_CNS + 16 /* Offset in PTP header */;

        reservedFieldValue = ((packetBuff[reservedFieldOffset] << 24) +
                              (packetBuff[reservedFieldOffset+1] << 16) +
                              (packetBuff[reservedFieldOffset+2] << 8) +
                               packetBuff[reservedFieldOffset+3]);
        PRV_UTF_LOG1_MAC("Capture Packet reservedField: nano 0x%08X\n",reservedFieldValue);

        if (reservedFieldValue == 0)
        {
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, GT_FAIL, "reserved field is '0'\n");
        }
    }
    if (testNum == 3)
    {
        if ((packetBuff[12] == ((PRV_TGF_PTP_TS_TAG_ETHERTYPE_CNS >> 8) & 0xFF)) ||
            (packetBuff[13] == (PRV_TGF_PTP_TS_TAG_ETHERTYPE_CNS & 0xFF)) ||
            (packetBuff[20] == 0x81))
        {
            rc = GT_FAIL;
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "packetBuff[12] =  %2.2x (expected not 0xfe), packetBuff[13] =  %2.2x (expected not 0xdc), "
                                                    "packetBuff[20] =  %2.2x (expected not 0x81)",
                                         packetBuff[12], packetBuff[13], packetBuff[20]);
        }

        correctionFieldOffset = TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS
                            + TGF_ETHERTYPE_SIZE_CNS + 8 /* Offset in PTP header */;

        memCmpRc = cpssOsMemCmp(&(packetBuff[correctionFieldOffset]), prvTgfPacketPtpV2Part.correctionField, 8);
        UTF_VERIFY_NOT_EQUAL0_STRING_MAC(
            0, memCmpRc, "correction field is '0'\n");

        reservedFieldOffset = TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS
                            + TGF_ETHERTYPE_SIZE_CNS + 16 /* Offset in PTP header */;

        reservedFieldValue = ((packetBuff[reservedFieldOffset] << 24) +
                              (packetBuff[reservedFieldOffset+1] << 16) +
                              (packetBuff[reservedFieldOffset+2] << 8) +
                               packetBuff[reservedFieldOffset+3]);
        PRV_UTF_LOG1_MAC("Capture Packet reservedField: nano 0x%08X\n",reservedFieldValue);

        if (reservedFieldValue != 0xAABBCCDD)
        {
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, GT_FAIL, "reserved field is not '0xAABBCCDD'\n");
        }

    }
    if (testNum == 2)
    {
        if ((packetBuff[12] == ((PRV_TGF_PTP_TS_TAG_ETHERTYPE_CNS >> 8) & 0xFF)) ||
            (packetBuff[13] == (PRV_TGF_PTP_TS_TAG_ETHERTYPE_CNS & 0xFF)) ||
            (packetBuff[20] == 0x81))
        {
            rc = GT_FAIL;
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "packetBuff[12] =  %2.2x (expected not 0xfe), packetBuff[13] =  %2.2x (expected not 0xdc), "
                                                    "packetBuff[20] =  %2.2x (expected not 0x81)",
                                         packetBuff[12], packetBuff[13], packetBuff[20]);
        }

        reservedFieldOffset = TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS
                            + TGF_ETHERTYPE_SIZE_CNS + 16 /* Offset in PTP header */;

        reservedFieldValue = ((packetBuff[reservedFieldOffset] << 24) +
                              (packetBuff[reservedFieldOffset+1] << 16) +
                              (packetBuff[reservedFieldOffset+2] << 8) +
                               packetBuff[reservedFieldOffset+3]);
        PRV_UTF_LOG1_MAC("Capture Packet reservedField: nano 0x%08X\n",reservedFieldValue);

        if (reservedFieldValue != 0xAABBCCDD)
        {
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, GT_FAIL, "reserved field is not '0xAABBCCDD'\n");
        }
    }
    if (testNum == 1)
    {
        if ((packetBuff[12] == ((PRV_TGF_PTP_TS_TAG_ETHERTYPE_CNS >> 8) & 0xFF)) ||
            (packetBuff[13] == (PRV_TGF_PTP_TS_TAG_ETHERTYPE_CNS & 0xFF)) ||
            (packetBuff[20] == 0x81))
        {
            rc = GT_FAIL;
            UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "packetBuff[12] =  %2.2x (expected not 0xfe), packetBuff[13] =  %2.2x (expected not 0xdc), "
                                                    "packetBuff[20] =  %2.2x (expected not 0x81)",
                                         packetBuff[12], packetBuff[13], packetBuff[20]);
        }

        reservedFieldOffset = TGF_L2_HEADER_SIZE_CNS + TGF_VLAN_TAG_SIZE_CNS
                            + TGF_ETHERTYPE_SIZE_CNS + 16 /* Offset in PTP header */;

        reservedFieldValue = ((packetBuff[reservedFieldOffset] << 24) +
                              (packetBuff[reservedFieldOffset+1] << 16) +
                              (packetBuff[reservedFieldOffset+2] << 8) +
                               packetBuff[reservedFieldOffset+3]);
        PRV_UTF_LOG1_MAC("Capture Packet reservedField: nano 0x%08X\n",reservedFieldValue);

        if (reservedFieldValue == 0)
        {
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, GT_FAIL, "reserved field is '0'\n");
        }
    }
}

#define PRV_TGF_PTP_PCL_RULE_INDEX_CNS 0

/**
* @internal prvTgfPtpManagerGenAddTimeStampTagAndMirrorSet
*           function
* @endinternal
*
* @brief  1)Timestamping done by OAM triggering
*         2)config oam ptp table
*         3)config pcl rule to mirror and oam action
*         4)config timestamp tag format
*/
GT_VOID prvTgfPtpManagerGenAddTimeStampTagAndMirrorSet
(
    GT_VOID
)
{
    GT_STATUS                             rc = GT_OK;

    /* oam table configuration */
    GT_U32                                oamIndex;           /* OAM entry index: oam table entry --> ptp entry */
    GT_U32                                entryIndex;         /* oam table entry index: pcl rule --> oam table entry*/
    PRV_TGF_OAM_COMMON_CONFIG_STC         oamConfig;          /* OAM configuration */
    PRV_TGF_PTP_TS_CFG_ENTRY_STC          timeStampCfgEntry;  /* TS configuiration table structure */
    /* pcl configuration */
    PRV_TGF_PCL_RULE_FORMAT_UNT           mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT           pattern;
    PRV_TGF_PCL_ACTION_STC                action;
    GT_U32                                ruleIndex;
    PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT      ruleFormat;
    PRV_TGF_PCL_ACTION_OAM_STC            oamAction;

    /* set global and per port configuration timestamp tag */
    PRV_TGF_PTP_TS_TAG_GLOBAL_CFG_STC   tsTagGlobalCfg;
    PRV_TGF_PTP_TS_TAG_PORT_CFG_STC     tsTagPortCfg;

    cpssOsMemSet(&timeStampCfgEntry, 0, sizeof(timeStampCfgEntry));
    cpssOsMemSet(&oamAction, 0, sizeof(oamAction));
    cpssOsMemSet(&oamConfig, 0, sizeof(oamConfig));
    cpssOsMemSet(&origTimeStampCfgEntry, 0, sizeof(origTimeStampCfgEntry));

   /* --------------------- oam table configuration ---------------------*/

    rc = prvTgfOamEnableGet(PRV_TGF_OAM_STAGE_TYPE_INGRESS_E, &origOamEnabler);
    UTF_VERIFY_EQUAL1_PARAM_MAC(
        GT_OK, rc, "prvTgfOamEnableGet");

    /* AUTODOC: enable OAM processing */
    rc = prvTgfOamEnableSet(PRV_TGF_OAM_STAGE_TYPE_INGRESS_E, GT_TRUE);
    UTF_VERIFY_EQUAL1_PARAM_MAC(
        GT_OK, rc, "prvTgfOamEnableSet");

    oamIndex                            = 0;
    entryIndex                          = oamIndex + 1;
    oamConfig.oamEntryIndex             = entryIndex;

    /* Get OAM common configuration */
    prvTgfOamConfigurationGet(&oamConfig);

    origOamConfig =  oamConfig;
    oamConfig.oamEntry.opcodeParsingEnable = GT_FALSE;
    oamConfig.oamEntry.timestampEnable     = GT_TRUE;
    oamConfig.oamEntry.oamPtpOffsetIndex   = oamIndex;

    /* Set common OAM configurations */
    prvTgfOamConfigurationSet(&oamConfig);

    timeStampCfgEntry.packetFormat         = PRV_TGF_PTP_TS_PACKET_TYPE_PTP_V2_E;
    timeStampCfgEntry.tsMode               = PRV_TGF_PTP_TS_TIMESTAMPING_MODE_DO_ACTION_E;
    timeStampCfgEntry.tsAction             = PRV_TGF_PTP_TS_ACTION_ADD_TIME_E;
    timeStampCfgEntry.offset               = 0;

    rc = prvTgfPtpTsCfgTableGet(oamIndex,&origTimeStampCfgEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "prvTgfPtpTsCfgTableGet");

    origTimeStampCfgEntry.packetFormat = PRV_TGF_PTP_TS_PACKET_TYPE_PTP_V2_E;

    rc = prvTgfPtpTsCfgTableSet(oamIndex, &timeStampCfgEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "prvTgfPtpTsCfgTableSet");

    /* --------------------- pcl configuration --------------------- */

    /* AUTODOC: init PCL Engine for sending to port 1 */
    rc = prvTgfPclDefPortInit( prvTgfPortsArray[PRV_TGF_PTP_GEN_SEND_PORT_INDEX_CNS],
                             CPSS_PCL_DIRECTION_INGRESS_E,
                             CPSS_PCL_LOOKUP_0_E,
                             PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E,
                             PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E,
                             PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclDefPortInit: %d, %d", prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_PTP_GEN_SEND_PORT_INDEX_CNS]);

    /* AUTODOC: add PCL rule 0 with following parameters: */
    /* AUTODOC:   format INGRESS_STD_NOT_IP */
    /* AUTODOC:   cmd MIRROR TO CPU */
    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));
    cpssOsMemSet(&action, 0, sizeof(action));
    cpssOsMemSet(mask.ruleStdNotIp.macDa.arEther, 0xFF, sizeof(GT_ETHERADDR));
    cpssOsMemCpy(pattern.ruleStdNotIp.macDa.arEther, prvTgfPacketL2Part.daMac, sizeof(prvTgfPacketL2Part.daMac));

    oamAction.timeStampEnable   = GT_TRUE;
    oamAction.oamProcessEnable  = GT_FALSE;
    oamAction.offsetIndex       = entryIndex;
    oamAction.oamProfile        = 0;

    ruleIndex                   = PRV_TGF_PTP_PCL_RULE_INDEX_CNS; /* call index function client CPSS_PCL_LOOKUP_0_E ipcl*/
    ruleFormat                  = PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;
    action.pktCmd               = CPSS_PACKET_CMD_MIRROR_TO_CPU_E   ;
    action.mirror.cpuCode       = cpuCode;
    action.oam                  = oamAction;
    action.egressPolicy         = GT_FALSE;

    rc = prvTgfPclRuleSet(ruleFormat, ruleIndex, &mask, &pattern, &action);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPclRuleSet: %d, %d, %d",
                                 prvTgfDevNum, ruleFormat, ruleIndex);

    /* --------------------- set global and per port configuration timestamp tag --------------------- */
    tsTagGlobalCfg.tsTagParseEnable      = GT_TRUE;
    tsTagGlobalCfg.hybridTsTagParseEnable = GT_FALSE;
    tsTagGlobalCfg.tsTagEtherType       = PRV_TGF_PTP_TS_TAG_ETHERTYPE_CNS;
    tsTagGlobalCfg.hybridTsTagEtherType = 0x0;

    rc = prvTgfPtpTsTagGlobalCfgGet(&origTsTagGlobalCfg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "prvTgfPtpTsTagGlobalCfgGet");

    rc = prvTgfPtpTsTagGlobalCfgSet(&tsTagGlobalCfg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "prvTgfPtpTsTagGlobalCfgSet");

    /*set per port configuration timestamp tag*/
    tsTagPortCfg.tsReceptionEnable = GT_TRUE;
    tsTagPortCfg.tsPiggyBackEnable = GT_FALSE;
    tsTagPortCfg.tsTagMode = PRV_TGF_PTP_TS_TAG_MODE_ALL_EXTENDED_E;

    rc = prvTgfPtpTsTagPortCfgGet(CPSS_CPU_PORT_NUM_CNS,&origTsTagPortCfg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "prvTgfPtpTsTagPortCfgGet");

    rc = prvTgfPtpManagerPortTsTagConfig(prvTgfDevNum,CPSS_CPU_PORT_NUM_CNS/*63*/,&tsTagPortCfg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "prvTgfPtpManagerPortTsTagConfig");

    if (PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5X_E || PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_IRONMAN_E)
    {
         /* disable TSX module; When running in MAC loopback, the link_status from the PCS to the TSX is '0' so the TSX RX can't sync. The packet
            arrives to egress of the other port with the negative CF before the update. This is caused by the invalid TS driven by the TSX RX. */
        prvTgfPtpGenTsxDisable(prvTgfPortsArray[PRV_TGF_PTP_GEN_SEND_PORT_INDEX_CNS],prvTgfPortsArray[PRV_TGF_PTP_GEN_RECEIVE_PORT_INDEX_CNS]);

        rc = cpssDxChPtpManagerPortTimestampingModeConfig(prvTgfDevNum,prvTgfPortsArray[PRV_TGF_PTP_GEN_RECEIVE_PORT_INDEX_CNS],CPSS_DXCH_PTP_ENHANCED_TIMESTAMPING_MODE_STANDARD_E,CPSS_DXCH_PTP_MANAGER_OPERATION_MODE_ONE_OR_TWO_STEP_IN_SWITCH_E);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "cpssDxChPtpManagerPortTimestampingModeConfig");

        rc = cpssDxChPtpManagerPortTimestampingModeConfig(prvTgfDevNum,prvTgfPortsArray[PRV_TGF_PTP_GEN_SEND_PORT_INDEX_CNS],CPSS_DXCH_PTP_ENHANCED_TIMESTAMPING_MODE_STANDARD_E,CPSS_DXCH_PTP_MANAGER_OPERATION_MODE_ONE_OR_TWO_STEP_IN_SWITCH_E);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "cpssDxChPtpManagerPortTimestampingModeConfig");

        cpssDxChPtpTsuBypassEnableSet(prvTgfDevNum,GT_FALSE);
    }
    if ((PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5P_E) || (PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_HARRIER_E))
    {
         /* disable TSX module; When running in MAC loopback, the link_status from the PCS to the TSX is '0' so the TSX RX can't sync. The packet
            arrives to egress of the other port with the negative CF before the update. This is caused by the invalid TS driven by the TSX RX. */
        prvTgfPtpGenTsxDisable(prvTgfPortsArray[PRV_TGF_PTP_GEN_SEND_PORT_INDEX_CNS],prvTgfPortsArray[PRV_TGF_PTP_GEN_RECEIVE_PORT_INDEX_CNS]);
    }
}

/**
* @internal prvTgfPtpManagerGenAddTimeStampTagAndMirrorrRestore
*           function
* @endinternal
*
* @brief   None
*/
GT_VOID prvTgfPtpManagerGenAddTimeStampTagAndMirrorrRestore
(
    GT_VOID
)
{
    GT_STATUS rc;
    GT_U32                  oamIndex = 0; /* oam index*/
    CPSS_PCL_RULE_SIZE_ENT  ruleSize;

    /* Restore OAM Enabler configurations */
    rc = prvTgfOamEnableSet(PRV_TGF_OAM_STAGE_TYPE_INGRESS_E, origOamEnabler);
    UTF_VERIFY_EQUAL1_PARAM_MAC(
        GT_OK, rc, "prvTgfOamEnableSet");

    /* Restore common OAM configurations */
    prvTgfOamConfigurationSet(&origOamConfig);

    /* Restore ptp configuration table */
    rc = prvTgfPtpTsCfgTableSet(oamIndex,&origTimeStampCfgEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "prvTgfPtpTsCfgTableSet");

    /* restore timestamp tag global configuration */
    rc = prvTgfPtpTsTagGlobalCfgSet(&origTsTagGlobalCfg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "prvTgfPtpTsTagGlobalCfgSet");

    /* restore timestamp tag port configuration*/
    rc = prvTgfPtpManagerPortTsTagConfig(prvTgfDevNum,CPSS_CPU_PORT_NUM_CNS/*63*/,&origTsTagPortCfg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "prvTgfPtpManagerPortTsTagConfig");

    prvTgfPtpGenVidAndFdbDefCfgRestore();

    /* AUTODOC : disable all PCL configurations on the ports */
    prvTgfPclPortsRestoreAll();

    ruleSize = CPSS_PCL_RULE_SIZE_STD_E;
    rc = prvTgfPclRuleValidStatusSet(ruleSize, PRV_TGF_PTP_PCL_RULE_INDEX_CNS, GT_FALSE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPclRuleValidStatusSet: %d, %d, %d",
                                 ruleSize, PRV_TGF_PTP_PCL_RULE_INDEX_CNS, GT_FALSE);

    /* Restore TSX module configuration */
    if ((PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5X_E) || (PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5P_E) ||
        (PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_HARRIER_E) || (PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_IRONMAN_E))
    {
        prvTgfPtpGenTsxRestore(prvTgfPortsArray[PRV_TGF_PTP_GEN_SEND_PORT_INDEX_CNS],prvTgfPortsArray[PRV_TGF_PTP_GEN_RECEIVE_PORT_INDEX_CNS]);
    }
}

/**
* @internal prvTgfPtpManagerGenAddTimestampTaggingSet function
* @endinternal
*
* @brief  1)Timestamping done by OAM triggering
*         2)config oam ptp table
*         3)config pcl rule to mirror and oam action
*         4)config timestamp tag format
*/
GT_VOID prvTgfPtpManagerGenAddTimestampTaggingSet
(
    PRV_TGF_PTP_TS_TAG_MODE_ENT tagMode
)
{
    GT_STATUS                             rc = GT_OK;

    /* oam table configuration */
    GT_U32                                oamIndex;           /* OAM entry index: oam table entry --> ptp entry */
    GT_U32                                entryIndex;         /* oam table entry index: pcl rule --> oam table entry*/
    PRV_TGF_OAM_COMMON_CONFIG_STC         oamConfig;          /* OAM configuration */
    PRV_TGF_PTP_TS_CFG_ENTRY_STC          timeStampCfgEntry;  /* TS configuiration table structure */
    /* pcl configuration */
    PRV_TGF_PCL_RULE_FORMAT_UNT           mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT           pattern;
    PRV_TGF_PCL_ACTION_STC                action;
    GT_U32                                ruleIndex;
    PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT      ruleFormat;
    PRV_TGF_PCL_ACTION_OAM_STC            oamAction;

    /* set global and per port configuration timestamp tag */
    PRV_TGF_PTP_TS_TAG_GLOBAL_CFG_STC   tsTagGlobalCfg;
    PRV_TGF_PTP_TS_TAG_PORT_CFG_STC     tsTagPortCfg;

    cpssOsMemSet(&timeStampCfgEntry, 0, sizeof(timeStampCfgEntry));
    cpssOsMemSet(&oamAction, 0, sizeof(oamAction));
    cpssOsMemSet(&oamConfig, 0, sizeof(oamConfig));
    cpssOsMemSet(&origTimeStampCfgEntry, 0, sizeof(origTimeStampCfgEntry));

   /* --------------------- oam table configuration ---------------------*/

    rc = prvTgfOamEnableGet(PRV_TGF_OAM_STAGE_TYPE_INGRESS_E, &origOamEnabler);
    UTF_VERIFY_EQUAL1_PARAM_MAC(
        GT_OK, rc, "prvTgfOamEnableGet");

    /* AUTODOC: enable OAM processing */
    rc = prvTgfOamEnableSet(PRV_TGF_OAM_STAGE_TYPE_INGRESS_E, GT_TRUE);
    UTF_VERIFY_EQUAL1_PARAM_MAC(
        GT_OK, rc, "prvTgfOamEnableSet");

    oamIndex                            = 0;
    entryIndex                          = oamIndex + 1;
    oamConfig.oamEntryIndex             = entryIndex;

    /* Get OAM common configuration */
    prvTgfOamConfigurationGet(&oamConfig);

    origOamConfig =  oamConfig;
    oamConfig.oamEntry.opcodeParsingEnable = GT_FALSE;
    oamConfig.oamEntry.timestampEnable     = GT_TRUE;
    oamConfig.oamEntry.oamPtpOffsetIndex   = oamIndex;

    /* Set common OAM configurations */
    prvTgfOamConfigurationSet(&oamConfig);

    timeStampCfgEntry.packetFormat         = PRV_TGF_PTP_TS_PACKET_TYPE_PTP_V2_E;
    timeStampCfgEntry.tsMode               = PRV_TGF_PTP_TS_TIMESTAMPING_MODE_DO_ACTION_E;
    timeStampCfgEntry.tsAction             = PRV_TGF_PTP_TS_ACTION_ADD_TIME_E;
    timeStampCfgEntry.offset               = 0;

    rc = prvTgfPtpTsCfgTableGet(oamIndex,&origTimeStampCfgEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "prvTgfPtpTsCfgTableGet");

    origTimeStampCfgEntry.packetFormat = PRV_TGF_PTP_TS_PACKET_TYPE_PTP_V2_E;

    rc = prvTgfPtpTsCfgTableSet(oamIndex, &timeStampCfgEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "prvTgfPtpTsCfgTableSet");

    /* --------------------- pcl configuration --------------------- */

    /* AUTODOC: init PCL Engine for sending to port 1 */
    rc = prvTgfPclDefPortInit( prvTgfPortsArray[PRV_TGF_PTP_GEN_SEND_PORT_INDEX_CNS],
                             CPSS_PCL_DIRECTION_INGRESS_E,
                             CPSS_PCL_LOOKUP_0_E,
                             PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E,
                             PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E,
                             PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E);
    UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "prvTgfPclDefPortInit: %d, %d", prvTgfDevNum,
            prvTgfPortsArray[PRV_TGF_PTP_GEN_SEND_PORT_INDEX_CNS]);

    /* AUTODOC: add PCL rule 0 with following parameters: */
    /* AUTODOC:   format INGRESS_STD_NOT_IP */
    /* AUTODOC:   cmd MIRROR TO CPU */
    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(&pattern, 0, sizeof(pattern));
    cpssOsMemSet(&action, 0, sizeof(action));
    cpssOsMemSet(mask.ruleStdNotIp.macDa.arEther, 0xFF, sizeof(GT_ETHERADDR));
    cpssOsMemCpy(pattern.ruleStdNotIp.macDa.arEther, prvTgfPacketL2Part.daMac, sizeof(prvTgfPacketL2Part.daMac));

    oamAction.timeStampEnable   = GT_TRUE;
    oamAction.oamProcessEnable  = GT_FALSE;
    oamAction.offsetIndex       = entryIndex;
    oamAction.oamProfile        = 0;

    ruleIndex                   = PRV_TGF_PTP_PCL_RULE_INDEX_CNS; /* call index function client CPSS_PCL_LOOKUP_0_E ipcl*/
    ruleFormat                  = PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;
    action.pktCmd               = CPSS_PACKET_CMD_MIRROR_TO_CPU_E   ;
    action.mirror.cpuCode       = cpuCode;
    action.oam                  = oamAction;
    action.egressPolicy         = GT_FALSE;

    rc = prvTgfPclRuleSet(ruleFormat, ruleIndex, &mask, &pattern, &action);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPclRuleSet: %d, %d, %d",
                                 prvTgfDevNum, ruleFormat, ruleIndex);

    /* --------------------- set global and per port configuration timestamp tag --------------------- */
    tsTagGlobalCfg.tsTagParseEnable      = GT_TRUE;
    tsTagGlobalCfg.hybridTsTagParseEnable = GT_FALSE;
    tsTagGlobalCfg.tsTagEtherType       = PRV_TGF_PTP_TS_TAG_ETHERTYPE_CNS;
    tsTagGlobalCfg.hybridTsTagEtherType = 0x0;

    rc = prvTgfPtpTsTagGlobalCfgGet(&origTsTagGlobalCfg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "prvTgfPtpTsTagGlobalCfgGet");

    rc = prvTgfPtpTsTagGlobalCfgSet(&tsTagGlobalCfg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "prvTgfPtpTsTagGlobalCfgSet");

    /*set per port configuration timestamp tag*/
    tsTagPortCfg.tsReceptionEnable = GT_TRUE;
    tsTagPortCfg.tsPiggyBackEnable = GT_FALSE;
    tsTagPortCfg.tsTagMode = tagMode;

    rc = prvTgfPtpTsTagPortCfgGet(CPSS_CPU_PORT_NUM_CNS,&origTsTagPortCfg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "prvTgfPtpTsTagPortCfgGet");

    rc = prvTgfPtpManagerPortTsTagConfig(prvTgfDevNum,CPSS_CPU_PORT_NUM_CNS/*63*/,&tsTagPortCfg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "prvTgfPtpManagerPortTsTagConfig");

    if (PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5X_E || PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_IRONMAN_E)
    {
         /* disable TSX module; When running in MAC loopback, the link_status from the PCS to the TSX is '0' so the TSX RX can't sync. The packet
            arrives to egress of the other port with the negative CF before the update. This is caused by the invalid TS driven by the TSX RX. */
        prvTgfPtpGenTsxDisable(prvTgfPortsArray[PRV_TGF_PTP_GEN_SEND_PORT_INDEX_CNS],prvTgfPortsArray[PRV_TGF_PTP_GEN_RECEIVE_PORT_INDEX_CNS]);

        rc = cpssDxChPtpManagerPortTimestampingModeConfig(prvTgfDevNum,prvTgfPortsArray[PRV_TGF_PTP_GEN_RECEIVE_PORT_INDEX_CNS],CPSS_DXCH_PTP_ENHANCED_TIMESTAMPING_MODE_STANDARD_E,CPSS_DXCH_PTP_MANAGER_OPERATION_MODE_ONE_OR_TWO_STEP_IN_SWITCH_E);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "cpssDxChPtpManagerPortTimestampingModeConfig");

        rc = cpssDxChPtpManagerPortTimestampingModeConfig(prvTgfDevNum,prvTgfPortsArray[PRV_TGF_PTP_GEN_SEND_PORT_INDEX_CNS],CPSS_DXCH_PTP_ENHANCED_TIMESTAMPING_MODE_STANDARD_E,CPSS_DXCH_PTP_MANAGER_OPERATION_MODE_ONE_OR_TWO_STEP_IN_SWITCH_E);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "cpssDxChPtpManagerPortTimestampingModeConfig");

        cpssDxChPtpTsuBypassEnableSet(prvTgfDevNum,GT_FALSE);
    }
    if ((PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5P_E) || (PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_HARRIER_E))
    {
         /* disable TSX module; When running in MAC loopback, the link_status from the PCS to the TSX is '0' so the TSX RX can't sync. The packet
            arrives to egress of the other port with the negative CF before the update. This is caused by the invalid TS driven by the TSX RX. */
        prvTgfPtpGenTsxDisable(prvTgfPortsArray[PRV_TGF_PTP_GEN_SEND_PORT_INDEX_CNS],prvTgfPortsArray[PRV_TGF_PTP_GEN_RECEIVE_PORT_INDEX_CNS]);
    }
}

/**
* @internal prvTgfPtpManagerTimestampTaggingRestorePrevTests function
* @endinternal
*
* @brief   restore configuration from prev tests
*/
GT_VOID prvTgfPtpManagerTimestampTaggingRestorePrevTests
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;
    GT_U32 i;

    rc = cpssDxChBrgVlanTpidEntrySet(prvTgfDevNum,CPSS_DXCH_ETHERTYPE_TABLE_INGRESS_E,1,0x8100);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "cpssDxChBrgVlanTpidEntrySet");

    rc = cpssDxChBrgVlanTpidEntrySet(prvTgfDevNum,CPSS_DXCH_ETHERTYPE_TABLE_EGRESS_E,1,0x8100);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "cpssDxChBrgVlanTpidEntrySet");

    for (i=0; i<4; i++)
    {
        rc = cpssDxChBrgVlanPortEgressTpidSet(prvTgfDevNum,prvTgfPortsArray[i],CPSS_DXCH_BRG_VLAN_ETHERTYPE_TABLE_ENTRY_SELECT_MODE_ETHERTYPE1_E,0);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "cpssDxChBrgVlanPortEgressTpidSet");
    }

}

/**
* @internal prvTgfPtpManagerTimestampTagging function
* @endinternal
*
* @brief        None
*
* @param[in]    tagMode - egress TS tag
*/
GT_VOID prvTgfPtpManagerTimestampTagging
(
    PRV_TGF_PTP_TS_TAG_MODE_ENT tagMode
)
{
    switch (tagMode)
    {
    case PRV_TGF_PTP_TS_TAG_MODE_NON_EXTENDED_E:
        PRV_UTF_LOG0_MAC("\nChecking Timestamp Tag Non-Extended.\n\n");
        break;
    case PRV_TGF_PTP_TS_TAG_MODE_EXTENDED_E:
        PRV_UTF_LOG0_MAC("\nChecking Timestamp Tag Extended.\n\n");
        break;
    case PRV_TGF_PTP_TS_TAG_MODE_HYBRID_E:
        PRV_UTF_LOG0_MAC("\nChecking Timestamp Tag Hybrid.\n\n");
        break;
    case PRV_TGF_PTP_TS_TAG_MODE_PIGGYBACK_E:
        PRV_UTF_LOG0_MAC("\nChecking Timestamp Tag PiggyBack.\n\n");
        break;
    default:
        /* do nothing*/
        break;
    }

    if (tagMode == PRV_TGF_PTP_TS_TAG_MODE_NON_EXTENDED_E || tagMode == PRV_TGF_PTP_TS_TAG_MODE_EXTENDED_E)
    {
        /* global PTP-related settings */
        prvTgfPtpManagerGenPtpDefaultInit();

        /* general bridge configuration */
        prvTgfPtpGenVidAndFdbDefCfgSet();
        prvTgfPtpGenFdbDefCfgSet(3);

        /* Configure OAM engine based timestamping */
        prvTgfPtpManagerGenAddTimestampTaggingSet(tagMode);

        /* Generate traffic: send to device's port given packet */
        prvTgfPtpGenTrafficAndCheckTimestampTagging(tagMode);

        prvTgfPtpManagerGenAddTimeStampTagAndMirrorrRestore();
    }

    if (tagMode == PRV_TGF_PTP_TS_TAG_MODE_HYBRID_E || tagMode == PRV_TGF_PTP_TS_TAG_MODE_PIGGYBACK_E)
    {
        /* clean configuration of prev tests */
        prvTgfPtpManagerTimestampTaggingRestorePrevTests();

        /* global PTP-related settings */
        prvTgfPtpManagerGenPtpOverEthEnablePtpInit();

        /* general bridge configuration */
        prvTgfPtpGenVidAndFdbDefCfgSet();

        /* egress port index for PIPE1 */
        prvTgfPtpManagerGenIngressCaptureCfgSet(3);

        if (tagMode == PRV_TGF_PTP_TS_TAG_MODE_HYBRID_E)
        {
            /* test 1: non TST on egress port (CPU) and packet is incoming with TST - remove it */

            /* PTP Hybrid configuration */
            prvTgfPtpManagerGenTsTagPortCfgHybridTagSet(3,1);

             /* Generate traffic: send to device's port given packet */
            prvTgfPtpManagerGenHybridTimestampTrafficGenerateAndCheck(1);

            /* test 2: non TST on egress port (non CPU) and packet is incoming with TST - remove it (action: capture_ingress_time) */

            /* PTP Hybrid configuration */
            prvTgfPtpManagerGenTsTagPortCfgHybridTagSet(3,2);

             /* Generate traffic: send to device's port given packet */
            prvTgfPtpManagerGenHybridTimestampTrafficGenerateAndCheck(2);

            /* test 3: non TST on egress port (non CPU) and packet is incoming with TST - remove it (action: add_corrected_time) according to ingress TST */

            /* PTP Hybrid configuration */
            prvTgfPtpManagerGenTsTagPortCfgHybridTagSet(3,3);

             /* Generate traffic: send to device's port given packet */
            prvTgfPtpManagerGenHybridTimestampTrafficGenerateAndCheck(3);

            /* test 4: TST on egress and packet is incoming with TST - preserve incoming (don't modify the value) */

            /* PTP Hybrid configuration */
            prvTgfPtpManagerGenTsTagPortCfgHybridTagSet(3,4);

             /* Generate traffic: send to device's port given packet */
            prvTgfPtpManagerGenHybridTimestampTrafficGenerateAndCheck(4);

        }
        else
        {
            /* test 1: TST on egress and packet is incoming with TST - (modify the value) (action: capture_ingress_time);
               Expect capture packet to have: timestamp tag in <resreved> field == timestamp tag in ingress queue entry */

            /* PTP PiggyBack configuration */
            prvTgfPtpManagerGenTsTagPortCfgPiggyBackSet(3,1);

             /* Generate traffic: send to device's port given packet */
            prvTgfPtpManagerGenPiggyBackTimestampTrafficGenerateAndCheck(1);

            /* test 2: non TST on egress port (non CPU) and packet is incoming with TST - remove it (action: capture_ingress_time)
               Expect capture packet to have: timestamp tag in <resreved> field == '0' */

            /* PTP PiggyBack configuration */
            prvTgfPtpManagerGenTsTagPortCfgPiggyBackSet(3,2);

             /* Generate traffic: send to device's port given packet */
            prvTgfPtpManagerGenPiggyBackTimestampTrafficGenerateAndCheck(2);
        }

        /* restore PTP configuration */
        prvTgfPtpManagerGenPtpRestore(3);

        /* restore PTP piggyBack configuration */
        prvTgfPtpManagerGenTsTagPortCfgPiggyBackRestore(3);

        /* restore bridge configuration */
        prvTgfPtpGenVidAndFdbDefCfgRestore();
    }
}

/**
* @internal prvTgfPtpManagerGenIngressEgressCaptureCfgSet function
* @endinternal
*
* @brief        None
*
* @param[in]    None
*/
GT_STATUS prvTgfPtpManagerGenIngressEgressCaptureCfgSet
(
        GT_VOID
)
{
    GT_STATUS                             rc = GT_OK;
    GT_U32                                oamIndex = 0;   /* OAM entry index: oam table entry --> ptp entry */

    /* set global and per port configuration timestamp tag */
    PRV_TGF_PTP_TS_TAG_GLOBAL_CFG_STC   tsTagGlobalCfg;
    PRV_TGF_PTP_TS_TAG_PORT_CFG_STC     tsTagPortCfg;

    /* TS configuiration table structure */
    PRV_TGF_PTP_TS_CFG_ENTRY_STC          timeStampCfgEntry;
    PRV_TGF_PTP_EGRESS_DOMAIN_ENTRY_STC   domainEntry;

    cpssOsMemSet(&domainEntry, 0, sizeof(domainEntry));
    cpssOsMemSet(&timeStampCfgEntry, 0, sizeof(timeStampCfgEntry));
    cpssOsMemSet(&origTimeStampCfgEntry, 0, sizeof(origTimeStampCfgEntry));

    prvTgfPtpOamBasedTimestampTriggerSet(oamIndex, CPSS_PACKET_CMD_FORWARD_E);

    /* --------------------- TS config table --------------------- */

    /* AUTODOC: Configure Add Ingress and Egress time for PTP packet by TS action table */
    timeStampCfgEntry.packetFormat         = PRV_TGF_PTP_TS_PACKET_TYPE_Y1731_E;
    timeStampCfgEntry.tsMode               = PRV_TGF_PTP_TS_TIMESTAMPING_MODE_DO_ACTION_E;
    timeStampCfgEntry.tsAction             = PRV_TGF_PTP_TS_ACTION_ADD_INGRESS_EGRESS_TIME_E;
    timeStampCfgEntry.offset               = 0;
    timeStampCfgEntry.offset2              = 16;

    rc = prvTgfPtpTsCfgTableGet(oamIndex, &origTimeStampCfgEntry);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPtpTsCfgTableGet");

    origTimeStampCfgEntry.packetFormat = PRV_TGF_PTP_TS_PACKET_TYPE_PTP_V2_E;

    rc = prvTgfPtpTsCfgTableSet(oamIndex, &timeStampCfgEntry);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPtpTsCfgTableSet");

    /* --------------------- set global and per port configuration timestamp tag --------------------- */
    /* AUTODOC: Set global configuration timestamp tag */
    tsTagGlobalCfg.tsTagParseEnable       = GT_FALSE;
    tsTagGlobalCfg.hybridTsTagParseEnable = GT_FALSE;
    tsTagGlobalCfg.tsTagEtherType = 0xFFFF;
    tsTagGlobalCfg.hybridTsTagEtherType = 0x0;

    rc = prvTgfPtpTsTagGlobalCfgGet(&origTsTagGlobalCfg);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPtpTsTagGlobalCfgGet");

    rc = prvTgfPtpTsTagGlobalCfgSet(&tsTagGlobalCfg);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPtpTsTagGlobalCfgSet");

    /* AUTODOC: Set per port configuration timestamp tag*/
    tsTagPortCfg.tsReceptionEnable = GT_FALSE;
    tsTagPortCfg.tsPiggyBackEnable = GT_FALSE;
    tsTagPortCfg.tsTagMode = PRV_TGF_PTP_TS_TAG_MODE_ALL_EXTENDED_E;

    rc = prvTgfPtpTsTagPortCfgGet(CPSS_CPU_PORT_NUM_CNS,&origTsTagPortCfg);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPtpTsTagPortCfgGet");

    rc = prvTgfPtpTsTagPortCfgGet(prvTgfPortsArray[PRV_TGF_PTP_GEN_RECEIVE_PORT_INDEX_CNS],
                                  &origTsTagPortCfg1);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPtpTsTagPortCfgGet");

    rc = prvTgfPtpManagerPortTsTagConfig(prvTgfDevNum,prvTgfPortsArray[PRV_TGF_PTP_GEN_RECEIVE_PORT_INDEX_CNS],&tsTagPortCfg);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPtpManagerPortTsTagConfig");

    /* Enable SIP6 frame counter */
    /* Enable SIP6 frame counter, GM - TSU not supported */
    if (!prvUtfIsGmCompilation())
    {
        rc = prvTgfPtpFrameCounterEnable(prvTgfPortsArray[PRV_TGF_PTP_GEN_RECEIVE_PORT_INDEX_CNS],
                                         PRV_TGF_PTP_TS_PACKET_TYPE_Y1731_E,
                                         PRV_TGF_PTP_TS_ACTION_ADD_INGRESS_EGRESS_TIME_E);
        PRV_UTF_VERIFY_GT_OK(rc, "prvTgfPtpFrameCounterEnable");
    }

    if (PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5X_E || PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_IRONMAN_E)
    {
         /* disable TSX module; When running in MAC loopback, the link_status from the PCS to the TSX is '0' so the TSX RX can't sync. The packet
            arrives to egress of the other port with the negative CF before the update. This is caused by the invalid TS driven by the TSX RX. */
        prvTgfPtpGenTsxDisable(prvTgfPortsArray[PRV_TGF_PTP_GEN_SEND_PORT_INDEX_CNS],prvTgfPortsArray[PRV_TGF_PTP_GEN_RECEIVE_PORT_INDEX_CNS]);

        rc = cpssDxChPtpManagerPortTimestampingModeConfig(prvTgfDevNum,prvTgfPortsArray[PRV_TGF_PTP_GEN_RECEIVE_PORT_INDEX_CNS],CPSS_DXCH_PTP_ENHANCED_TIMESTAMPING_MODE_STANDARD_E,CPSS_DXCH_PTP_MANAGER_OPERATION_MODE_ONE_OR_TWO_STEP_IN_SWITCH_E);
        PRV_UTF_VERIFY_GT_OK(rc, "cpssDxChPtpManagerPortTimestampingModeConfig");

        rc = cpssDxChPtpManagerPortTimestampingModeConfig(prvTgfDevNum,prvTgfPortsArray[PRV_TGF_PTP_GEN_SEND_PORT_INDEX_CNS],CPSS_DXCH_PTP_ENHANCED_TIMESTAMPING_MODE_STANDARD_E,CPSS_DXCH_PTP_MANAGER_OPERATION_MODE_ONE_OR_TWO_STEP_IN_SWITCH_E);
        PRV_UTF_VERIFY_GT_OK(rc, "cpssDxChPtpManagerPortTimestampingModeConfig");

        cpssDxChPtpTsuBypassEnableSet(prvTgfDevNum,GT_FALSE);
    }
    if ((PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5P_E) || (PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_HARRIER_E))
    {
         /* disable TSX module; When running in MAC loopback, the link_status from the PCS to the TSX is '0' so the TSX RX can't sync. The packet
            arrives to egress of the other port with the negative CF before the update. This is caused by the invalid TS driven by the TSX RX. */
        prvTgfPtpGenTsxDisable(prvTgfPortsArray[PRV_TGF_PTP_GEN_SEND_PORT_INDEX_CNS],prvTgfPortsArray[PRV_TGF_PTP_GEN_RECEIVE_PORT_INDEX_CNS]);
    }

    return GT_OK;
}

/**
* @internal prvTgfPtpManagerGenIngressEgressCaptureRestore
*           function
* @endinternal
*
* @brief   None
*/
GT_VOID prvTgfPtpManagerGenIngressEgressCaptureRestore
(
    GT_VOID
)
{
    GT_STATUS rc;
    GT_U32                  oamIndex = 0;   /* OAM entry index: oam table entry --> ptp entry */
    CPSS_PCL_RULE_SIZE_ENT  ruleSize;

    /* AUTODOC: RESTORE CONFIGURATION: */

    /* AUTODOC: Restore common OAM configurations */
    prvTgfOamConfigurationSet(&origOamConfig);

    /* AUTODOC: Restore OAM Enabler configurations */
    rc = prvTgfOamEnableSet(PRV_TGF_OAM_STAGE_TYPE_INGRESS_E, origOamEnabler);
    UTF_VERIFY_EQUAL1_PARAM_MAC(
        GT_OK, rc, "prvTgfOamEnableSet");

    /* clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficTableRxPcktTblClear");

    /* AUTODOC: Restore ptp configuration table */
    rc = prvTgfPtpTsCfgTableSet(oamIndex, &origTimeStampCfgEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "prvTgfPtpTsCfgTableSet");

    /* AUTODOC: Restore timestamp tag global configuration */
    rc = prvTgfPtpTsTagGlobalCfgSet(&origTsTagGlobalCfg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "prvTgfPtpTsTagGlobalCfgSet");

    /* AUTODOC: Restore timestamp tag port configuration*/
    rc = prvTgfPtpManagerPortTsTagConfig(prvTgfDevNum,CPSS_CPU_PORT_NUM_CNS/*63*/,&origTsTagPortCfg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "prvTgfPtpManagerPortTsTagConfig");

    rc = prvTgfPtpManagerPortTsTagConfig(prvTgfDevNum,prvTgfPortsArray[PRV_TGF_PTP_GEN_RECEIVE_PORT_INDEX_CNS],&origTsTagPortCfg1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "prvTgfPtpManagerPortTsTagConfig");

    /* restore PTP configuration */
    prvTgfPtpManagerGenPtpRestore(prvTgfPortsArray[PRV_TGF_PTP_GEN_RECEIVE_PORT_INDEX_CNS]);

    /* restore bridge configuration */
    prvTgfPtpGenVidAndFdbDefCfgRestore();

    /* AUTODOC : disable all PCL configurations on the ports */
    prvTgfPclPortsRestoreAll();

    ruleSize = CPSS_PCL_RULE_SIZE_STD_E;
    rc = prvTgfPclRuleValidStatusSet(ruleSize, PRV_TGF_PTP_PCL_RULE_INDEX_CNS, GT_FALSE);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPclRuleValidStatusSet: %d, %d, %d",
                                 ruleSize, PRV_TGF_PTP_PCL_RULE_INDEX_CNS, GT_FALSE);

    /* Restore TSX module configuration */
    if ((PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5X_E) || (PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_AC5P_E) ||
        (PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_HARRIER_E) || (PRV_CPSS_PP_MAC(prvTgfDevNum)->devFamily == CPSS_PP_FAMILY_DXCH_IRONMAN_E))
    {
        prvTgfPtpGenTsxRestore(prvTgfPortsArray[PRV_TGF_PTP_GEN_SEND_PORT_INDEX_CNS],prvTgfPortsArray[PRV_TGF_PTP_GEN_RECEIVE_PORT_INDEX_CNS]);
    }
}

/*
* @internal prvTgfPtpManagerEgresssExceptionCfgSet function
* @endinternal
*
* @brief Configuration to receive PTP Packet
*        1. Enable PTP over ethernet
*        2. Set PTP local action
*        3. Set ethertype of hybrid TStag to match ingress packet format
*        4. Configure per port TS config
*        4. Set PTP egress exception Config:
*            pkt cmd  = TRAP_TO_CPU
*            CPU code = CPSS_NET_PTP_HEADER_ERROR_E
*/
GT_VOID prvTgfPtpManagerEgresssExceptionCfgSet
(
    GT_VOID
)
{
    GT_STATUS                                     rc = GT_OK;
    PRV_TGF_PTP_EGRESS_EXCEPTION_CFG_STC          egrExceptionCfg;
    PRV_TGF_PTP_TS_TAG_PORT_CFG_STC               tsTagPortCfg;
    PRV_TGF_PTP_TS_TAG_GLOBAL_CFG_STC             tsTagGlobalCfg;
    PRV_TGF_PTP_EGRESS_DOMAIN_ENTRY_STC           domainEntry;

    rc = prvTgfPtpEtherTypeSet(0, 0x88F7);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "prvTgfPtpEtherTypeSet");

    cpssOsMemSet(&domainEntry, 0, sizeof(domainEntry));

    domainEntry.ptpOverEhernetTsEnable = GT_TRUE;
    domainEntry.ptpOverUdpIpv4TsEnable = GT_FALSE;
    domainEntry.ptpOverUdpIpv6TsEnable = GT_FALSE;
    domainEntry.messageTypeTsEnableBmp =
        (1 << prvTgfPacket1PtpV2Part.messageType);
    domainEntry.transportSpecificTsEnableBmp =
        (1 << prvTgfPacket1PtpV2Part.transportSpecific);

    rc = prvTgfPtpManagerPortPacketActionsConfig(prvTgfDevNum,prvTgfPortsArray[PRV_TGF_PTP_GEN_RECEIVE_PORT_INDEX_CNS],PRV_TGF_PTP_TAI_NUMBER_0_E,
                                                 4 /*domainNum*/,&domainEntry,prvTgfPacket1PtpV2Part.messageType,PRV_TGF_PTP_TS_ACTION_ADD_TIME_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "prvTgfPtpManagerPortPacketActionsConfig");

    cpssOsMemSet(&tsTagGlobalCfg, 0, sizeof(tsTagGlobalCfg));

    /* AUTODOC: Set Global Timestamp tag Configuration*/
    tsTagGlobalCfg.tsTagParseEnable       = GT_TRUE;
    tsTagGlobalCfg.hybridTsTagParseEnable = GT_TRUE;
    tsTagGlobalCfg.tsTagEtherType         = 0x0000;
    tsTagGlobalCfg.hybridTsTagEtherType   = 0x88F7;

    rc = prvTgfPtpTsTagGlobalCfgGet(&origTsTagGlobalCfg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "prvTgfPtpTsTagGlobalCfgGet");

    rc = prvTgfPtpTsTagGlobalCfgSet(&tsTagGlobalCfg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "prvTgfPtpTsTagGlobalCfgSet");

    /* AUTODOC: Configure per port configuration timestamp tag */
    tsTagPortCfg.tsReceptionEnable = GT_TRUE;
    tsTagPortCfg.tsPiggyBackEnable = GT_FALSE;
    tsTagPortCfg.tsTagMode = PRV_TGF_PTP_TS_TAG_MODE_HYBRID_E;

    rc = prvTgfPtpTsTagPortCfgGet(CPSS_CPU_PORT_NUM_CNS,&origTsTagPortCfg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "prvTgfPtpTsTagPortCfgGet");

    rc = prvTgfPtpManagerPortTsTagConfig(prvTgfDevNum,CPSS_CPU_PORT_NUM_CNS,&tsTagPortCfg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "prvTgfPtpManagerPortTsTagConfig");

    rc = prvTgfPtpTsTagPortCfgGet(prvTgfPortsArray[PRV_TGF_PTP_GEN_SEND_PORT_INDEX_CNS],
                                  &origTsTagPortCfg1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "prvTgfPtpTsTagPortCfgGet");

    rc = prvTgfPtpManagerPortTsTagConfig(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_PTP_GEN_SEND_PORT_INDEX_CNS],
                                  &tsTagPortCfg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "prvTgfPtpManagerPortTsTagConfig");

    /* AUTODOC: Configure PTP Egress Exception */
    egrExceptionCfg.invalidPtpPktCmd          = CPSS_PACKET_CMD_FORWARD_E;
    egrExceptionCfg.invalidOutPiggybackPktCmd = CPSS_PACKET_CMD_FORWARD_E;
    egrExceptionCfg.invalidInPiggybackPktCmd  = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
    egrExceptionCfg.invalidInPiggybackCpuCode = CPSS_NET_PTP_HEADER_ERROR_E;
    egrExceptionCfg.invalidTsPktCmd           = CPSS_PACKET_CMD_FORWARD_E;

    /* call device specific API */
    rc = prvTgfPtpEgressExceptionCfgSet(&egrExceptionCfg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "prvTgfPtpEgressExceptionCfgSet");
}

/*
* @internal prvTgfPtpManagerInvalidPtpCfgSet function
* @endinternal
*
* @brief Configuration to receive PTP Packet
*        1. Enable PTP over UDP IPv4
*        2. Set destination UDP port to match packet UDP header
*        3. Set PTP local action
*        4. Set ethertype of hybrid TStag to match ingress packet format
*        5. Configure per port TS config
*        6. Set PTP egress exception Config:
*            pkt cmd  = TRAP_TO_CPU
*            CPU code = CPSS_NET_PTP_HEADER_ERROR_E
*/
GT_VOID prvTgfPtpManagerInvalidPtpCfgSet
(
    GT_VOID
)
{
    GT_STATUS                                     rc = GT_OK;
    PRV_TGF_PTP_EGRESS_EXCEPTION_CFG_STC          egrExceptionCfg;
    PRV_TGF_PTP_TS_TAG_PORT_CFG_STC               tsTagPortCfg;
    PRV_TGF_PTP_TS_TAG_GLOBAL_CFG_STC             tsTagGlobalCfg;
    PRV_TGF_PTP_EGRESS_DOMAIN_ENTRY_STC           domainEntry;

    cpssOsMemSet(&domainEntry, 0, sizeof(domainEntry));

    domainEntry.ptpOverEhernetTsEnable = GT_TRUE;
    domainEntry.ptpOverUdpIpv4TsEnable = GT_TRUE;
    domainEntry.ptpOverUdpIpv6TsEnable = GT_FALSE;
    domainEntry.messageTypeTsEnableBmp =
        (1 << prvTgfPacketPtpV2Part.messageType);
    domainEntry.transportSpecificTsEnableBmp =
        (1 << prvTgfPacketPtpV2Part.transportSpecific);

    rc = prvTgfPtpManagerPortPacketActionsConfig(prvTgfDevNum,CPSS_CPU_PORT_NUM_CNS/*63*/,PRV_TGF_PTP_TAI_NUMBER_0_E,
                                                 4 /*domainNum*/,&domainEntry,prvTgfPacket1PtpV2Part.messageType,PRV_TGF_PTP_TS_ACTION_ADD_TIME_E);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "prvTgfPtpManagerPortPacketActionsConfig");

    cpssOsMemSet(&tsTagGlobalCfg, 0, sizeof(tsTagGlobalCfg));

    /* AUTODOC: Set Global Timestamp tag Configuration*/
    tsTagGlobalCfg.tsTagParseEnable       = GT_TRUE;
    tsTagGlobalCfg.hybridTsTagParseEnable = GT_TRUE;
    tsTagGlobalCfg.tsTagEtherType         = 0x0000;
    tsTagGlobalCfg.hybridTsTagEtherType   = 0x88F7;

    rc = prvTgfPtpTsTagGlobalCfgGet(&origTsTagGlobalCfg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "prvTgfPtpTsTagGlobalCfgGet");

    rc = prvTgfPtpTsTagGlobalCfgSet(&tsTagGlobalCfg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "prvTgfPtpTsTagGlobalCfgSet");

    /* AUTODOC: Configure per port configuration timestamp tag */
    tsTagPortCfg.tsReceptionEnable = GT_TRUE;
    tsTagPortCfg.tsPiggyBackEnable = GT_FALSE;
    tsTagPortCfg.tsTagMode = PRV_TGF_PTP_TS_TAG_MODE_HYBRID_E;

    rc = prvTgfPtpTsTagPortCfgGet(CPSS_CPU_PORT_NUM_CNS,&origTsTagPortCfg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "prvTgfPtpTsTagPortCfgGet");

    rc = prvTgfPtpManagerPortTsTagConfig(prvTgfDevNum,CPSS_CPU_PORT_NUM_CNS/*63*/,&tsTagPortCfg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "prvTgfPtpManagerPortTsTagConfig");

    rc = prvTgfPtpTsTagPortCfgGet(prvTgfPortsArray[PRV_TGF_PTP_GEN_SEND_PORT_INDEX_CNS],
                                  &origTsTagPortCfg1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "prvTgfPtpTsTagPortCfgGet");

    rc = prvTgfPtpManagerPortTsTagConfig(prvTgfDevNum,prvTgfPortsArray[PRV_TGF_PTP_GEN_SEND_PORT_INDEX_CNS],&tsTagPortCfg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "prvTgfPtpManagerPortTsTagConfig");

    /* AUTODOC: Configure PTP Egress Exception */
    egrExceptionCfg.invalidPtpPktCmd          = CPSS_PACKET_CMD_TRAP_TO_CPU_E;
    egrExceptionCfg.invalidPtpCpuCode         = CPSS_NET_PTP_HEADER_ERROR_E;
    egrExceptionCfg.invalidOutPiggybackPktCmd = CPSS_PACKET_CMD_FORWARD_E;
    egrExceptionCfg.invalidInPiggybackPktCmd  = CPSS_PACKET_CMD_FORWARD_E;
    egrExceptionCfg.invalidTsPktCmd           = CPSS_PACKET_CMD_FORWARD_E;

    /* call device specific API */
    rc = prvTgfPtpEgressExceptionCfgSet(&egrExceptionCfg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "prvTgfPtpEgressExceptionCfgSet");

    /* Mask Ip header length error */
    rc = prvTgfIpHeaderErrorMaskSet(prvTgfDevNum, PRV_TGF_IP_HEADER_ERROR_LENGTH_ENT,
                                    CPSS_IP_PROTOCOL_IPV4_E, CPSS_UNICAST_E, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpHeaderErrorMaskSet");
}

/**
* @internal prvTgfPtpManagerPtpOverIpUDPCfgSet function
* @endinternal
*
* @brief   None
*/
GT_VOID prvTgfPtpManagerPtpOverIpUDPCfgSet
(
    PRV_TGF_PTP_TS_ACTION_ENT ptpAction
)
{
    GT_STATUS                                     rc = GT_OK;
    PRV_TGF_PTP_EGRESS_DOMAIN_ENTRY_STC           domainEntry;

    cpssOsMemSet(&domainEntry, 0, sizeof(domainEntry));

    domainEntry.ptpOverEhernetTsEnable = GT_FALSE;
    domainEntry.ptpOverUdpIpv4TsEnable = GT_TRUE;
    domainEntry.ptpOverUdpIpv6TsEnable = GT_TRUE;
    domainEntry.messageTypeTsEnableBmp =
        (1 << prvTgfPacketPtpV2Part.messageType);
    domainEntry.transportSpecificTsEnableBmp =
        (1 << prvTgfPacketPtpV2Part.transportSpecific);

    rc = prvTgfPtpManagerPortPacketActionsConfig(prvTgfDevNum,prvTgfPortsArray[PRV_TGF_PTP_GEN_RECEIVE_PORT_INDEX_CNS],PRV_TGF_PTP_TAI_NUMBER_0_E,
                                                 4 /*domainNum*/,&domainEntry,prvTgfPacket1PtpV2Part.messageType,ptpAction);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "prvTgfPtpManagerPortPacketActionsConfig");
}

/**
* @internal prvTgfPtpManagerEgressExceptionTestRestore function
* @endinternal
*
* @brief  Restore the Configuration
*/
GT_VOID prvTgfPtpManagerEgressExceptionTestRestore
(
    GT_VOID
)
{
    GT_STATUS rc;

    /* restore timestamp tag global configuration */
    rc = prvTgfPtpTsTagGlobalCfgSet(&origTsTagGlobalCfg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "prvTgfPtpTsTagGlobalCfgSet");

    /* restore timestamp tag port configuration*/
    rc = prvTgfPtpManagerPortTsTagConfig(prvTgfDevNum,CPSS_CPU_PORT_NUM_CNS,&origTsTagPortCfg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "prvTgfPtpManagerPortTsTagConfig");

    rc = prvTgfPtpManagerPortTsTagConfig(prvTgfDevNum,prvTgfPortsArray[PRV_TGF_PTP_GEN_SEND_PORT_INDEX_CNS],&origTsTagPortCfg1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "prvTgfPtpManagerPortTsTagConfig");

    /* restore PTP configuration */
    prvTgfPtpManagerGenPtpRestore(prvTgfPortsArray[PRV_TGF_PTP_GEN_RECEIVE_PORT_INDEX_CNS]);
    prvTgfPtpManagerGenPtpRestore(CPSS_CPU_PORT_NUM_CNS/*63*/);

    /* restore bridge configuration */
    prvTgfPtpGenVidAndFdbDefCfgRestore();
}

/**
* @internal prvTgfPtpManagerInvalidPtpCfgRestore function
* @endinternal
*
* @brief  Restore the Configuration
*/
GT_VOID prvTgfPtpManagerInvalidPtpCfgRestore
(
    GT_VOID
)
{
    GT_STATUS rc;

    /* restore timestamp tag global configuration */
    rc = prvTgfPtpTsTagGlobalCfgSet(&origTsTagGlobalCfg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "prvTgfPtpTsTagGlobalCfgSet");

    /* restore timestamp tag port configuration*/
    rc = prvTgfPtpManagerPortTsTagConfig(prvTgfDevNum,CPSS_CPU_PORT_NUM_CNS,&origTsTagPortCfg);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "prvTgfPtpManagerPortTsTagConfig");

    rc = prvTgfPtpManagerPortTsTagConfig(prvTgfDevNum,prvTgfPortsArray[PRV_TGF_PTP_GEN_SEND_PORT_INDEX_CNS],&origTsTagPortCfg1);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK,rc, "prvTgfPtpManagerPortTsTagConfig");

    /* restore PTP configuration */
    prvTgfPtpManagerGenPtpRestore(CPSS_CPU_PORT_NUM_CNS/*63*/);
    prvTgfPtpManagerGenPtpRestore(prvTgfPortsArray[PRV_TGF_PTP_GEN_RECEIVE_PORT_INDEX_CNS]);

    rc = prvTgfIpHeaderErrorMaskSet(prvTgfDevNum, PRV_TGF_IP_HEADER_ERROR_LENGTH_ENT,
                                    CPSS_IP_PROTOCOL_IPV4_E, CPSS_UNICAST_E, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfIpHeaderErrorMaskSet");


    prvTgfPtpGenCfgRestore();

    /* restore bridge configuration */
    prvTgfPtpGenVidAndFdbDefCfgRestore();
}

/**
* @internal prvTgfPtpManagerCapturePreviousModeCfgRestore
*           function
* @endinternal
*
* @brief  Restore the Configuration
*/
GT_VOID prvTgfPtpManagerCapturePreviousModeCfgRestore
(
    GT_VOID
)
{
    GT_STATUS                           rc;
    PRV_TGF_PTP_TAI_ID_STC              taiId;

    cpssOsMemSet(&taiId, 0, sizeof(taiId));
    taiId.taiInstance = PRV_TGF_PTP_TAI_INSTANCE_PORT_E;
    taiId.taiNumber   = PRV_TGF_PTP_TAI_NUMBER_0_E;
    taiId.portNum     = prvTgfPortsArray[PRV_TGF_PTP_GEN_SEND_PORT_INDEX_CNS];

    /* Restore Capture Previous Mode to Disable */
    rc = prvTgfPtpTaiCapturePreviousModeEnableSet(&taiId, origCapturePreviousMode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPtpTaiCapturePreviousModeEnableSet");
}

/**
* @internal prvTgfPtpManagerCapturePreviousModeCfgAndVerifyCheck
*           function
* @endinternal
*
* @brief  Set the TOD
*         Enable capture previous mode:
*         Set the TOD+1000 sec
*         Verify the capture value.
*/
GT_VOID prvTgfPtpManagerCapturePreviousModeCfgAndVerifyCheck
(
    GT_VOID
)
{
    GT_STATUS                           rc;
    PRV_TGF_PTP_TAI_ID_STC              tgfTaiId;
    PRV_TGF_PTP_TOD_COUNT_STC           tgfTodValue;
    PRV_TGF_PTP_MANAGER_CAPTURE_TOD_VALUE_STC todValueEntry0Get;
    PRV_TGF_PTP_MANAGER_CAPTURE_TOD_VALUE_STC todValueEntry1Get;

    cpssOsMemSet(&tgfTodValue, 0, sizeof(tgfTodValue));
    cpssOsMemSet(&todValueEntry0Get, 0, sizeof(todValueEntry0Get));
    cpssOsMemSet(&todValueEntry1Get, 0, sizeof(todValueEntry1Get));
    cpssOsMemSet(&tgfTaiId, 0, sizeof(tgfTaiId));

    /* Enable Capture Previous Mode */
    cpssOsMemSet(&tgfTaiId, 0, sizeof(tgfTaiId));
    tgfTaiId.taiInstance = PRV_TGF_PTP_TAI_INSTANCE_PORT_E;
    tgfTaiId.taiNumber   = PRV_TGF_PTP_TAI_NUMBER_0_E;
    tgfTaiId.portNum     = prvTgfPortsArray[PRV_TGF_PTP_GEN_SEND_PORT_INDEX_CNS];
    rc = prvTgfPtpTaiCapturePreviousModeEnableGet(&tgfTaiId, &origCapturePreviousMode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPtpTaiCapturePreviousModeEnableGet");

    /* Set the initial TOD Value */
    tgfTodValue.seconds.l[0]  = PRV_TGF_PTP_GEN_TOD_SEC_CNS;
    tgfTodValue.seconds.l[1]  = 0;

    /* AUTODOC: Trigger TOD counter function to set te TOD */
    rc = prvTgfPtpManagerTodValueSet(prvTgfDevNum,PRV_TGF_PTP_TAI_NUMBER_0_E,&tgfTodValue,GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPtpManagerTodValueSet");

    /* Set the incremented TOD Value */
    tgfTodValue.seconds.l[0]  = PRV_TGF_PTP_GEN_TOD_SEC_CNS + 1000;

    /* AUTODOC: Trigger TOD counter function to set te TOD */
    rc = prvTgfPtpManagerTodValueSet(prvTgfDevNum,PRV_TGF_PTP_TAI_NUMBER_0_E,&tgfTodValue,GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPtpManagerTodValueSet");

    /* Get the captured TOD value */
    rc = prvTgfPtpManagerTodValueGet(prvTgfDevNum,PRV_TGF_PTP_TAI_NUMBER_0_E,&todValueEntry0Get,&todValueEntry1Get);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfPtpManagerTodValueGet");
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE,todValueEntry0Get.todValueIsValid, "prvTgfPtpManagerTodValueGet");
    UTF_VERIFY_EQUAL0_STRING_MAC(PRV_TGF_PTP_GEN_TOD_SEC_CNS,todValueEntry0Get.todValue.seconds.l[0], "prvTgfPtpManagerTodValueGet");
}

/**
* @internal prvTgfPtpManagerTC1StepE2ECfgSet function
* @endinternal
*
* @brief   None
*/
GT_STATUS prvTgfPtpManagerTC1StepE2ECfgSet
(
    IN GT_U32 part
)
{
    GT_STATUS                               rc = GT_OK;
    CPSS_DXCH_PTP_MANAGER_DOMAIN_STC        domainConf;
    CPSS_DXCH_PTP_EGRESS_DOMAIN_ENTRY_STC   domainEntry;

    if (part == 1)
    {
        domainConf.domainIndex = 2; /* the local domain index to which the domainID is directed */
        domainConf.domainMode = CPSS_DXCH_PTP_INGRESS_DOMAIN_MODE_PTP_V2_E;
        domainConf.v2DomainId = prvTgfPacketSyncPtpV2Part.domainNumber; /* domainID is the domain number which is passed in the PTP header */

        /* Configure the global settings for PTP Domain */
        rc = cpssDxChPtpManagerGlobalPtpDomainSet(prvTgfDevNum,&domainConf);
        PRV_UTF_VERIFY_GT_OK(rc, "cpssDxChPtpManagerGlobalPtpDomainSet");

        /* Allocate Port Interface to TAI */
        rc = cpssDxChPtpManagerPortAllocateTai(prvTgfDevNum,MASTER_PORT_INDEX,CPSS_DXCH_PTP_TAI_NUMBER_0_E,CPSS_DXCH_PTP_TS_DELAY_TAI_SEL_MODE_EGRESS_E);
        PRV_UTF_VERIFY_GT_OK(rc, "cpssDxChPtpManagerGlobalPtpDomainSet");
        rc = cpssDxChPtpManagerPortAllocateTai(prvTgfDevNum,SLAVE_PORT_INDEX,CPSS_DXCH_PTP_TAI_NUMBER_0_E,CPSS_DXCH_PTP_TS_DELAY_TAI_SEL_MODE_EGRESS_E);
        PRV_UTF_VERIFY_GT_OK(rc, "cpssDxChPtpManagerGlobalPtpDomainSet");

        /* Assign packet command per{ingress port, domain index, message type}. */
        rc = cpssDxChPtpManagerPortCommandAssignment(prvTgfDevNum,MASTER_PORT_INDEX,2,CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_SYNC_E,CPSS_PACKET_CMD_FORWARD_E);
        PRV_UTF_VERIFY_GT_OK(rc, "cpssDxChPtpManagerGlobalPtpDomainSet");
        rc = cpssDxChPtpManagerPortCommandAssignment(prvTgfDevNum,SLAVE_PORT_INDEX,2,CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_DELAY_REQUEST_E,CPSS_PACKET_CMD_FORWARD_E);
        PRV_UTF_VERIFY_GT_OK(rc, "cpssDxChPtpManagerGlobalPtpDomainSet");
        rc = cpssDxChPtpManagerPortCommandAssignment(prvTgfDevNum,MASTER_PORT_INDEX,2,CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_DELAY_RESPONSE_E,CPSS_PACKET_CMD_FORWARD_E);
        PRV_UTF_VERIFY_GT_OK(rc, "cpssDxChPtpManagerGlobalPtpDomainSet");

        cpssOsMemSet(&domainEntry, 0, sizeof(domainEntry));
        domainEntry.ptpOverEhernetTsEnable = GT_TRUE;
        domainEntry.ptpOverUdpIpv4TsEnable = GT_FALSE;
        domainEntry.ptpOverUdpIpv6TsEnable = GT_FALSE;
        domainEntry.messageTypeTsEnableBmp =
            (1 << prvTgfPacketSyncPtpV2Part.messageType) | (1 << prvTgfPacketDelayResPtpV2Part.messageType);
        domainEntry.transportSpecificTsEnableBmp =
            (1 << prvTgfPacketSyncPtpV2Part.transportSpecific) | (1 << prvTgfPacketDelayResPtpV2Part.transportSpecific);

        /* Configure action to be performed on PTP packets per{egress port, domain index, message type}. */
        rc = cpssDxChPtpManagerPortPacketActionsConfig(prvTgfDevNum,SLAVE_PORT_INDEX,CPSS_DXCH_PTP_TAI_NUMBER_0_E,2,&domainEntry,
                                                       CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_SYNC_E,CPSS_DXCH_PTP_TS_ACTION_ADD_CORRECTED_TIME_E);
        PRV_UTF_VERIFY_GT_OK(rc, "cpssDxChPtpManagerPortPacketActionsConfig");

        cpssOsMemSet(&domainEntry, 0, sizeof(domainEntry));
        domainEntry.ptpOverEhernetTsEnable = GT_TRUE;
        domainEntry.ptpOverUdpIpv4TsEnable = GT_FALSE;
        domainEntry.ptpOverUdpIpv6TsEnable = GT_FALSE;
        domainEntry.messageTypeTsEnableBmp =
            (1 << prvTgfPacketDelayReqPtpV2Part.messageType);
        domainEntry.transportSpecificTsEnableBmp =
            (1 << prvTgfPacketDelayReqPtpV2Part.transportSpecific);

        /* Configure action to be performed on PTP packets per{egress port, domain index, message type}. */
        rc = cpssDxChPtpManagerPortPacketActionsConfig(prvTgfDevNum,MASTER_PORT_INDEX,CPSS_DXCH_PTP_TAI_NUMBER_0_E,2,&domainEntry,
                                                       CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_DELAY_REQUEST_E,CPSS_DXCH_PTP_TS_ACTION_ADD_CORRECTED_TIME_E);
        PRV_UTF_VERIFY_GT_OK(rc, "cpssDxChPtpManagerPortPacketActionsConfig");

        if (PRV_CPSS_SIP_6_10_CHECK_MAC(prvTgfDevNum))
        {
            rc = cpssDxChPtpManagerPortTimestampingModeConfig(prvTgfDevNum,SLAVE_PORT_INDEX,CPSS_DXCH_PTP_ENHANCED_TIMESTAMPING_MODE_STANDARD_E,CPSS_DXCH_PTP_MANAGER_OPERATION_MODE_ONE_OR_TWO_STEP_IN_SWITCH_E);
            PRV_UTF_VERIFY_GT_OK(rc, "cpssDxChPtpManagerPortTimestampingModeConfig");
        }

    }
    else
    {
        cpssOsMemSet(&domainEntry, 0, sizeof(domainEntry));
        domainEntry.ptpOverEhernetTsEnable = GT_TRUE;
        domainEntry.ptpOverUdpIpv4TsEnable = GT_FALSE;
        domainEntry.ptpOverUdpIpv6TsEnable = GT_FALSE;
        domainEntry.messageTypeTsEnableBmp =
            (1 << prvTgfPacketSyncPtpV2Part.messageType) | (1 << prvTgfPacketDelayResPtpV2Part.messageType);
        domainEntry.transportSpecificTsEnableBmp =
            (1 << prvTgfPacketSyncPtpV2Part.transportSpecific) | (1 << prvTgfPacketDelayResPtpV2Part.transportSpecific);

        /* Configure action to be performed on PTP packets per{egress port, domain index, message type}. */
        rc = cpssDxChPtpManagerPortPacketActionsConfig(prvTgfDevNum,SLAVE_PORT_INDEX,CPSS_DXCH_PTP_TAI_NUMBER_0_E,2,&domainEntry,
                                                       CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_DELAY_RESPONSE_E,CPSS_DXCH_PTP_TS_ACTION_NONE_E);
        PRV_UTF_VERIFY_GT_OK(rc, "cpssDxChPtpManagerPortPacketActionsConfig");
    }

    return GT_OK;
}

/**
* @internal prvTgfPtpManager1StepFollowUpInformationTlvCfgSet function
* @endinternal
*
* @brief   None
*/
GT_STATUS prvTgfPtpManager1StepFollowUpInformationTlvCfgSet
(
    GT_VOID
)
{
    GT_STATUS                               rc = GT_OK;
    CPSS_DXCH_PTP_MANAGER_DOMAIN_STC        domainConf;
    CPSS_DXCH_PTP_EGRESS_DOMAIN_ENTRY_STC   domainEntry;

    domainConf.domainIndex = 0; /* the local domain index to which the domainID is directed */
    domainConf.domainMode = CPSS_DXCH_PTP_INGRESS_DOMAIN_MODE_PTP_V2_E;
    domainConf.v2DomainId = prvTgfPacketSyncPtpV2Part.domainNumber; /* domainID is the domain number which is passed in the PTP header */

    /* Configure the global settings for PTP Domain */
    rc = cpssDxChPtpManagerGlobalPtpDomainSet(prvTgfDevNum,&domainConf);
    PRV_UTF_VERIFY_GT_OK(rc, "cpssDxChPtpManagerGlobalPtpDomainSet");

    /* Allocate Port Interface to TAI */
    rc = cpssDxChPtpManagerPortAllocateTai(prvTgfDevNum,MASTER_PORT_INDEX,CPSS_DXCH_PTP_TAI_NUMBER_0_E,CPSS_DXCH_PTP_TS_DELAY_TAI_SEL_MODE_EGRESS_E);
    PRV_UTF_VERIFY_GT_OK(rc, "cpssDxChPtpManagerGlobalPtpDomainSet");
    rc = cpssDxChPtpManagerPortAllocateTai(prvTgfDevNum,SLAVE_PORT_INDEX,CPSS_DXCH_PTP_TAI_NUMBER_0_E,CPSS_DXCH_PTP_TS_DELAY_TAI_SEL_MODE_EGRESS_E);
    PRV_UTF_VERIFY_GT_OK(rc, "cpssDxChPtpManagerGlobalPtpDomainSet");

    /* Assign packet command per{ingress port, domain index, message type}. */
    rc = cpssDxChPtpManagerPortCommandAssignment(prvTgfDevNum,MASTER_PORT_INDEX,0,CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_SYNC_E,CPSS_PACKET_CMD_FORWARD_E);
    PRV_UTF_VERIFY_GT_OK(rc, "cpssDxChPtpManagerGlobalPtpDomainSet");

    cpssOsMemSet(&domainEntry, 0, sizeof(domainEntry));
    domainEntry.ptpOverEhernetTsEnable = GT_TRUE;
    domainEntry.ptpOverUdpIpv4TsEnable = GT_FALSE;
    domainEntry.ptpOverUdpIpv6TsEnable = GT_FALSE;
    domainEntry.messageTypeTsEnableBmp =
        (1 << prvTgfPacketSyncPtpV2Part.messageType);
    domainEntry.transportSpecificTsEnableBmp =
        (1 << prvTgfPacketSyncPtpV2Part.transportSpecific);

    /* Configure action to be performed on PTP packets per{egress port, domain index, message type}. */
    rc = cpssDxChPtpManagerPortPacketActionsConfig(prvTgfDevNum,SLAVE_PORT_INDEX,CPSS_DXCH_PTP_TAI_NUMBER_0_E,0,&domainEntry,
                                                   CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_SYNC_E,CPSS_DXCH_PTP_TS_ACTION_ADD_CORRECTED_TIME_E);
    PRV_UTF_VERIFY_GT_OK(rc, "cpssDxChPtpManagerPortPacketActionsConfig");

    cpssOsMemSet(&domainEntry, 0, sizeof(domainEntry));
    domainEntry.ptpOverEhernetTsEnable = GT_TRUE;
    domainEntry.ptpOverUdpIpv4TsEnable = GT_FALSE;
    domainEntry.ptpOverUdpIpv6TsEnable = GT_FALSE;
    domainEntry.messageTypeTsEnableBmp =
        (1 << prvTgfPacketDelayReqPtpV2Part.messageType);
    domainEntry.transportSpecificTsEnableBmp =
        (1 << prvTgfPacketDelayReqPtpV2Part.transportSpecific);

    if (PRV_CPSS_SIP_6_10_CHECK_MAC(prvTgfDevNum))
    {
        rc = cpssDxChPtpManagerPortTimestampingModeConfig(prvTgfDevNum,SLAVE_PORT_INDEX,CPSS_DXCH_PTP_ENHANCED_TIMESTAMPING_MODE_STANDARD_E,CPSS_DXCH_PTP_MANAGER_OPERATION_MODE_ONE_OR_TWO_STEP_IN_SWITCH_E);
        PRV_UTF_VERIFY_GT_OK(rc, "cpssDxChPtpManagerPortTimestampingModeConfig");
    }

    /* Set PTP Cumulative Scaled Rate Offset enable/disable per message Type. */
    rc = cpssDxChPtpManager8021AsMsgSelectionEnableSet(prvTgfDevNum,CPSS_DXCH_PTP_TAI_NUMBER_0_E,GT_FALSE,GT_TRUE);
    PRV_UTF_VERIFY_GT_OK(rc, "cpssDxChPtpManager8021AsMsgSelectionEnableSet");

    /* Set PTP Cumulative Scaled Rate Offset in the "FollowUp information TLV" either in a Sync or a FollowUp message. */
    rc = cpssDxChPtpManager8021AsCumulativeScaledRateOffsetSet(prvTgfDevNum,CPSS_DXCH_PTP_TAI_NUMBER_0_E,0x99887766);
    PRV_UTF_VERIFY_GT_OK(rc, "cpssDxChPtpManager8021AsCumulativeScaledRateOffsetSet");

    return GT_OK;
}

/**
* @internal prvTgfPtpManagerTC1StepP2PCfgSet function
* @endinternal
*
* @brief   None
*/
GT_STATUS prvTgfPtpManagerTC1StepP2PCfgSet
(
    GT_VOID
)
{
    GT_STATUS                               rc = GT_OK;
    CPSS_DXCH_PTP_MANAGER_DOMAIN_STC        domainConf;
    CPSS_DXCH_PTP_EGRESS_DOMAIN_ENTRY_STC   domainEntry;


    domainConf.domainIndex = 2; /* the local domain index to which the domainID is directed */
    domainConf.domainMode = CPSS_DXCH_PTP_INGRESS_DOMAIN_MODE_PTP_V2_E;
    domainConf.v2DomainId = prvTgfPacketSyncPtpV2Part.domainNumber; /* domainID is the domain number which is passed in the PTP header */

    /* Configure the global settings for PTP Domain */
    rc = cpssDxChPtpManagerGlobalPtpDomainSet(prvTgfDevNum,&domainConf);
    PRV_UTF_VERIFY_GT_OK(rc, "cpssDxChPtpManagerGlobalPtpDomainSet");

    /* Allocate Port Interface to TAI */
    rc = cpssDxChPtpManagerPortAllocateTai(prvTgfDevNum,MASTER_PORT_INDEX,CPSS_DXCH_PTP_TAI_NUMBER_0_E,CPSS_DXCH_PTP_TS_DELAY_TAI_SEL_MODE_EGRESS_E);
    PRV_UTF_VERIFY_GT_OK(rc, "cpssDxChPtpManagerGlobalPtpDomainSet");
    rc = cpssDxChPtpManagerPortAllocateTai(prvTgfDevNum,SLAVE_PORT_INDEX,CPSS_DXCH_PTP_TAI_NUMBER_0_E,CPSS_DXCH_PTP_TS_DELAY_TAI_SEL_MODE_EGRESS_E);
    PRV_UTF_VERIFY_GT_OK(rc, "cpssDxChPtpManagerGlobalPtpDomainSet");

    /* Assign packet command per{ingress port, domain index, message type}. */
    rc = cpssDxChPtpManagerPortCommandAssignment(prvTgfDevNum,MASTER_PORT_INDEX,2,CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_SYNC_E,CPSS_PACKET_CMD_FORWARD_E);
    PRV_UTF_VERIFY_GT_OK(rc, "cpssDxChPtpManagerGlobalPtpDomainSet");
    rc = cpssDxChPtpManagerPortCommandAssignment(prvTgfDevNum,MASTER_PORT_INDEX,2,CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_PDELAY_REQUEST_E,CPSS_PACKET_CMD_FORWARD_E);
    PRV_UTF_VERIFY_GT_OK(rc, "cpssDxChPtpManagerGlobalPtpDomainSet");
    rc = cpssDxChPtpManagerPortCommandAssignment(prvTgfDevNum,SLAVE_PORT_INDEX,2,CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_PDELAY_REQUEST_E,CPSS_PACKET_CMD_FORWARD_E);
    PRV_UTF_VERIFY_GT_OK(rc, "cpssDxChPtpManagerGlobalPtpDomainSet");
    rc = cpssDxChPtpManagerPortCommandAssignment(prvTgfDevNum,MASTER_PORT_INDEX,2,CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_PDELAY_RESPONSE_E,CPSS_PACKET_CMD_FORWARD_E);
    PRV_UTF_VERIFY_GT_OK(rc, "cpssDxChPtpManagerGlobalPtpDomainSet");
    rc = cpssDxChPtpManagerPortCommandAssignment(prvTgfDevNum,SLAVE_PORT_INDEX,2,CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_PDELAY_RESPONSE_E,CPSS_PACKET_CMD_FORWARD_E);
    PRV_UTF_VERIFY_GT_OK(rc, "cpssDxChPtpManagerGlobalPtpDomainSet");

    cpssOsMemSet(&domainEntry, 0, sizeof(domainEntry));
    domainEntry.ptpOverEhernetTsEnable = GT_TRUE;
    domainEntry.ptpOverUdpIpv4TsEnable = GT_FALSE;
    domainEntry.ptpOverUdpIpv6TsEnable = GT_FALSE;
    domainEntry.messageTypeTsEnableBmp =
        (1 << prvTgfPacketSyncPtpV2Part.messageType) | (1 << prvTgfPacketPDelayReqPtpV2Part.messageType) | (1 << prvTgfPacketPDelayResPtpV2Part.messageType);
    domainEntry.transportSpecificTsEnableBmp =
        (1 << prvTgfPacketSyncPtpV2Part.transportSpecific) | (1 << prvTgfPacketPDelayReqPtpV2Part.transportSpecific) | (1 << prvTgfPacketPDelayResPtpV2Part.transportSpecific);

    /* Configure action to be performed on PTP packets per{egress port, domain index, message type}. */
    rc = cpssDxChPtpManagerPortPacketActionsConfig(prvTgfDevNum,SLAVE_PORT_INDEX,CPSS_DXCH_PTP_TAI_NUMBER_0_E,2,&domainEntry,
                                                   CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_SYNC_E,CPSS_DXCH_PTP_TS_ACTION_ADD_CORRECTED_TIME_E);
    PRV_UTF_VERIFY_GT_OK(rc, "cpssDxChPtpManagerPortPacketActionsConfig");

    /* Configure action to be performed on PTP packets per{egress port, domain index, message type}. */
    rc = cpssDxChPtpManagerPortPacketActionsConfig(prvTgfDevNum,SLAVE_PORT_INDEX,CPSS_DXCH_PTP_TAI_NUMBER_0_E,2,&domainEntry,
                                                   CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_PDELAY_REQUEST_E,CPSS_DXCH_PTP_TS_ACTION_ADD_TIME_E);
    PRV_UTF_VERIFY_GT_OK(rc, "cpssDxChPtpManagerPortPacketActionsConfig");

    /* Configure action to be performed on PTP packets per{egress port, domain index, message type}. */
    rc = cpssDxChPtpManagerPortPacketActionsConfig(prvTgfDevNum,SLAVE_PORT_INDEX,CPSS_DXCH_PTP_TAI_NUMBER_0_E,2,&domainEntry,
                                                   CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_PDELAY_RESPONSE_E,CPSS_DXCH_PTP_TS_ACTION_ADD_TIME_E);
    PRV_UTF_VERIFY_GT_OK(rc, "cpssDxChPtpManagerPortPacketActionsConfig");

    cpssOsMemSet(&domainEntry, 0, sizeof(domainEntry));
    domainEntry.ptpOverEhernetTsEnable = GT_TRUE;
    domainEntry.ptpOverUdpIpv4TsEnable = GT_FALSE;
    domainEntry.ptpOverUdpIpv6TsEnable = GT_FALSE;
    domainEntry.messageTypeTsEnableBmp =
        (1 << prvTgfPacketPDelayReqPtpV2Part.messageType) | (1 << prvTgfPacketPDelayResPtpV2Part.messageType);
    domainEntry.transportSpecificTsEnableBmp =
        (1 << prvTgfPacketPDelayReqPtpV2Part.transportSpecific) | (1 << prvTgfPacketPDelayResPtpV2Part.transportSpecific);

    /* Configure action to be performed on PTP packets per{egress port, domain index, message type}. */
    rc = cpssDxChPtpManagerPortPacketActionsConfig(prvTgfDevNum,MASTER_PORT_INDEX,CPSS_DXCH_PTP_TAI_NUMBER_0_E,2,&domainEntry,
                                                   CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_PDELAY_REQUEST_E,CPSS_DXCH_PTP_TS_ACTION_ADD_TIME_E);
    PRV_UTF_VERIFY_GT_OK(rc, "cpssDxChPtpManagerPortPacketActionsConfig");

    /* Configure action to be performed on PTP packets per{egress port, domain index, message type}. */
    rc = cpssDxChPtpManagerPortPacketActionsConfig(prvTgfDevNum,MASTER_PORT_INDEX,CPSS_DXCH_PTP_TAI_NUMBER_0_E,2,&domainEntry,
                                                   CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_PDELAY_RESPONSE_E,CPSS_DXCH_PTP_TS_ACTION_ADD_TIME_E);
    PRV_UTF_VERIFY_GT_OK(rc, "cpssDxChPtpManagerPortPacketActionsConfig");

    if (PRV_CPSS_SIP_6_10_CHECK_MAC(prvTgfDevNum))
    {
        rc = cpssDxChPtpManagerPortTimestampingModeConfig(prvTgfDevNum,SLAVE_PORT_INDEX,CPSS_DXCH_PTP_ENHANCED_TIMESTAMPING_MODE_STANDARD_E,CPSS_DXCH_PTP_MANAGER_OPERATION_MODE_ONE_OR_TWO_STEP_IN_SWITCH_E);
        PRV_UTF_VERIFY_GT_OK(rc, "cpssDxChPtpManagerPortTimestampingModeConfig");
    }

    return GT_OK;
}

/**
* @internal prvTgfPtpManagerBC2StepE2ECfgSet function
* @endinternal
*
* @brief   None
*/
GT_STATUS prvTgfPtpManagerBC2StepE2ECfgSet
(
    IN GT_U32 part
)
{
    GT_STATUS                               rc = GT_OK;
    CPSS_DXCH_PTP_MANAGER_DOMAIN_STC        domainConf;
    CPSS_DXCH_PTP_EGRESS_DOMAIN_ENTRY_STC   domainEntry;
    GT_U32                                  taiNum = 2;
    GT_BOOL                                 isAc5y = GT_FALSE;

    PRV_CPSS_DXCH_CHECK_AC5Y_DEVICE_MAC(prvTgfDevNum,isAc5y);

    taiNum = isAc5y ? 4 : 2;
    if (part == 1)
    {
        if (isAc5y == GT_FALSE)
        {
            domainConf.domainIndex = 2; /* the local domain index to which the domainID is directed */
            domainConf.domainMode = CPSS_DXCH_PTP_INGRESS_DOMAIN_MODE_PTP_V2_E;
            domainConf.v2DomainId = prvTgfPacketSyncPtpV2Part.domainNumber; /* domainID is the domain number which is passed in the PTP header */

            /* Configure the global settings for PTP Domain */
            rc = cpssDxChPtpManagerGlobalPtpDomainSet(prvTgfDevNum,&domainConf);
            PRV_UTF_VERIFY_GT_OK(rc, "cpssDxChPtpManagerGlobalPtpDomainSet");
        }

        /* Allocate Port Interface to TAI */
        rc = cpssDxChPtpManagerPortAllocateTai(prvTgfDevNum,MASTER_PORT_INDEX,CPSS_DXCH_PTP_TAI_NUMBER_0_E,CPSS_DXCH_PTP_TS_DELAY_TAI_SEL_MODE_EGRESS_E);
        PRV_UTF_VERIFY_GT_OK(rc, "cpssDxChPtpManagerGlobalPtpDomainSet");
        rc = cpssDxChPtpManagerPortAllocateTai(prvTgfDevNum,SLAVE_PORT_INDEX,CPSS_DXCH_PTP_TAI_NUMBER_0_E,CPSS_DXCH_PTP_TS_DELAY_TAI_SEL_MODE_EGRESS_E);
        PRV_UTF_VERIFY_GT_OK(rc, "cpssDxChPtpManagerGlobalPtpDomainSet");

        /* Assign packet command per{ingress port, domain index, message type}. */
        rc = cpssDxChPtpManagerPortCommandAssignment(prvTgfDevNum,MASTER_PORT_INDEX,taiNum,CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_SYNC_E,CPSS_PACKET_CMD_FORWARD_E);
        PRV_UTF_VERIFY_GT_OK(rc, "cpssDxChPtpManagerGlobalPtpDomainSet");
        rc = cpssDxChPtpManagerPortCommandAssignment(prvTgfDevNum,MASTER_PORT_INDEX,taiNum,CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_FOLLOW_UP_E,CPSS_PACKET_CMD_FORWARD_E);
        PRV_UTF_VERIFY_GT_OK(rc, "cpssDxChPtpManagerGlobalPtpDomainSet");
        rc = cpssDxChPtpManagerPortCommandAssignment(prvTgfDevNum,SLAVE_PORT_INDEX,taiNum,CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_DELAY_REQUEST_E,CPSS_PACKET_CMD_FORWARD_E);
        PRV_UTF_VERIFY_GT_OK(rc, "cpssDxChPtpManagerGlobalPtpDomainSet");
        rc = cpssDxChPtpManagerPortCommandAssignment(prvTgfDevNum,MASTER_PORT_INDEX,taiNum,CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_DELAY_RESPONSE_E,CPSS_PACKET_CMD_FORWARD_E);
        PRV_UTF_VERIFY_GT_OK(rc, "cpssDxChPtpManagerGlobalPtpDomainSet");

        cpssOsMemSet(&domainEntry, 0, sizeof(domainEntry));
        domainEntry.ptpOverEhernetTsEnable = GT_TRUE;
        domainEntry.ptpOverUdpIpv4TsEnable = GT_FALSE;
        domainEntry.ptpOverUdpIpv6TsEnable = GT_FALSE;
        domainEntry.messageTypeTsEnableBmp =
            (1 << prvTgfPacketSyncPtpV2Part.messageType) | (1 << prvTgfPacketDelayResPtpV2Part.messageType) | (1 << prvTgfPacketFollowUpPtpV2Part.messageType);
        domainEntry.transportSpecificTsEnableBmp =
            (1 << prvTgfPacketSyncPtpV2Part.transportSpecific) | (1 << prvTgfPacketDelayResPtpV2Part.transportSpecific) | (1 << prvTgfPacketFollowUpPtpV2Part.transportSpecific);

        /* Configure action to be performed on PTP packets per{egress port, domain index, message type}. */
        rc = cpssDxChPtpManagerPortPacketActionsConfig(prvTgfDevNum,SLAVE_PORT_INDEX,CPSS_DXCH_PTP_TAI_NUMBER_0_E,taiNum,&domainEntry,
                                                       CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_SYNC_E,CPSS_DXCH_PTP_TS_ACTION_CAPTURE_E);
        PRV_UTF_VERIFY_GT_OK(rc, "cpssDxChPtpManagerPortPacketActionsConfig");

        rc = cpssDxChPtpManagerTsQueueConfig(prvTgfDevNum,CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_SYNC_E,0);
        PRV_UTF_VERIFY_GT_OK(rc, "cpssDxChPtpManagerTsQueueConfig");

        /* Configure action to be performed on PTP packets per{egress port, domain index, message type}. */
        rc = cpssDxChPtpManagerPortPacketActionsConfig(prvTgfDevNum,SLAVE_PORT_INDEX,CPSS_DXCH_PTP_TAI_NUMBER_0_E,taiNum,&domainEntry,
                                                       CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_FOLLOW_UP_E,CPSS_DXCH_PTP_TS_ACTION_NONE_E);
        PRV_UTF_VERIFY_GT_OK(rc, "cpssDxChPtpManagerPortPacketActionsConfig");

        cpssOsMemSet(&domainEntry, 0, sizeof(domainEntry));
        domainEntry.ptpOverEhernetTsEnable = GT_TRUE;
        domainEntry.ptpOverUdpIpv4TsEnable = GT_FALSE;
        domainEntry.ptpOverUdpIpv6TsEnable = GT_FALSE;
        domainEntry.messageTypeTsEnableBmp =
            (1 << prvTgfPacketDelayReqPtpV2Part.messageType);
        domainEntry.transportSpecificTsEnableBmp =
            (1 << prvTgfPacketDelayReqPtpV2Part.transportSpecific);

        /* Configure action to be performed on PTP packets per{egress port, domain index, message type}. */
        rc = cpssDxChPtpManagerPortPacketActionsConfig(prvTgfDevNum,MASTER_PORT_INDEX,CPSS_DXCH_PTP_TAI_NUMBER_0_E,taiNum,&domainEntry,
                                                       CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_DELAY_REQUEST_E,CPSS_DXCH_PTP_TS_ACTION_CAPTURE_ADD_TIME_E);
        PRV_UTF_VERIFY_GT_OK(rc, "cpssDxChPtpManagerPortPacketActionsConfig");

        rc = cpssDxChPtpManagerTsQueueConfig(prvTgfDevNum,CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_DELAY_REQUEST_E,0);
        PRV_UTF_VERIFY_GT_OK(rc, "cpssDxChPtpManagerTsQueueConfig");

        if (PRV_CPSS_SIP_6_30_CHECK_MAC(prvTgfDevNum))
        {
            rc = cpssDxChPtpManagerPortTimestampingModeConfig(prvTgfDevNum,MASTER_PORT_INDEX,CPSS_DXCH_PTP_ENHANCED_TIMESTAMPING_MODE_STANDARD_E,CPSS_DXCH_PTP_MANAGER_OPERATION_MODE_ONE_OR_TWO_STEP_IN_MAC_E);
            PRV_UTF_VERIFY_GT_OK(rc, "cpssDxChPtpManagerPortTimestampingModeConfig");
        }
        else if (PRV_CPSS_SIP_6_10_CHECK_MAC(prvTgfDevNum))
        {
            rc = cpssDxChPtpManagerPortTimestampingModeConfig(prvTgfDevNum,MASTER_PORT_INDEX,CPSS_DXCH_PTP_ENHANCED_TIMESTAMPING_MODE_STANDARD_E,CPSS_DXCH_PTP_MANAGER_OPERATION_MODE_ONE_OR_TWO_STEP_IN_SWITCH_E);
            PRV_UTF_VERIFY_GT_OK(rc, "cpssDxChPtpManagerPortTimestampingModeConfig");
        }

    }
    else
    {
        cpssOsMemSet(&domainEntry, 0, sizeof(domainEntry));
        domainEntry.ptpOverEhernetTsEnable = GT_TRUE;
        domainEntry.ptpOverUdpIpv4TsEnable = GT_FALSE;
        domainEntry.ptpOverUdpIpv6TsEnable = GT_FALSE;
        domainEntry.messageTypeTsEnableBmp =
            (1 << prvTgfPacketSyncPtpV2Part.messageType) | (1 << prvTgfPacketDelayResPtpV2Part.messageType);
        domainEntry.transportSpecificTsEnableBmp =
            (1 << prvTgfPacketSyncPtpV2Part.transportSpecific) | (1 << prvTgfPacketDelayResPtpV2Part.transportSpecific);

        /* Configure action to be performed on PTP packets per{egress port, domain index, message type}. */
        rc = cpssDxChPtpManagerPortPacketActionsConfig(prvTgfDevNum,SLAVE_PORT_INDEX,CPSS_DXCH_PTP_TAI_NUMBER_0_E,taiNum,&domainEntry,
                                                       CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_DELAY_RESPONSE_E,CPSS_DXCH_PTP_TS_ACTION_NONE_E);
        PRV_UTF_VERIFY_GT_OK(rc, "cpssDxChPtpManagerPortPacketActionsConfig");
    }

    return GT_OK;
}

/**
* @internal prvTgfPtpManagerBC1StepE2ECfgSet function
* @endinternal
*
* @brief   None
*/
GT_STATUS prvTgfPtpManagerBC1StepE2ECfgSet
(
    IN GT_U32 part
)
{
    GT_STATUS                               rc = GT_OK;
    CPSS_DXCH_PTP_MANAGER_DOMAIN_STC        domainConf;
    CPSS_DXCH_PTP_EGRESS_DOMAIN_ENTRY_STC   domainEntry;
    GT_U32                                  taiNum = 2;
    GT_BOOL                                 isAc5y = GT_FALSE;

    PRV_CPSS_DXCH_CHECK_AC5Y_DEVICE_MAC(prvTgfDevNum,isAc5y);

    taiNum = isAc5y ? 4 : 2;

    if (part == 1)
    {
        if (isAc5y == GT_FALSE)
        {
            domainConf.domainIndex = 2; /* the local domain index to which the domainID is directed */
            domainConf.domainMode = CPSS_DXCH_PTP_INGRESS_DOMAIN_MODE_PTP_V2_E;
            domainConf.v2DomainId = prvTgfPacketSyncPtpV2Part.domainNumber; /* domainID is the domain number which is passed in the PTP header */

            /* Configure the global settings for PTP Domain */
            rc = cpssDxChPtpManagerGlobalPtpDomainSet(prvTgfDevNum,&domainConf);
            PRV_UTF_VERIFY_GT_OK(rc, "cpssDxChPtpManagerGlobalPtpDomainSet");
        }

        /* Allocate Port Interface to TAI */
        rc = cpssDxChPtpManagerPortAllocateTai(prvTgfDevNum,MASTER_PORT_INDEX,CPSS_DXCH_PTP_TAI_NUMBER_0_E,CPSS_DXCH_PTP_TS_DELAY_TAI_SEL_MODE_EGRESS_E);
        PRV_UTF_VERIFY_GT_OK(rc, "cpssDxChPtpManagerGlobalPtpDomainSet");
        rc = cpssDxChPtpManagerPortAllocateTai(prvTgfDevNum,SLAVE_PORT_INDEX,CPSS_DXCH_PTP_TAI_NUMBER_0_E,CPSS_DXCH_PTP_TS_DELAY_TAI_SEL_MODE_EGRESS_E);
        PRV_UTF_VERIFY_GT_OK(rc, "cpssDxChPtpManagerGlobalPtpDomainSet");

        /* Assign packet command per{ingress port, domain index, message type}. */
        rc = cpssDxChPtpManagerPortCommandAssignment(prvTgfDevNum,MASTER_PORT_INDEX,taiNum,CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_SYNC_E,CPSS_PACKET_CMD_FORWARD_E);
        PRV_UTF_VERIFY_GT_OK(rc, "cpssDxChPtpManagerGlobalPtpDomainSet");
        rc = cpssDxChPtpManagerPortCommandAssignment(prvTgfDevNum,SLAVE_PORT_INDEX,taiNum,CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_DELAY_REQUEST_E,CPSS_PACKET_CMD_FORWARD_E);
        PRV_UTF_VERIFY_GT_OK(rc, "cpssDxChPtpManagerGlobalPtpDomainSet");
        rc = cpssDxChPtpManagerPortCommandAssignment(prvTgfDevNum,MASTER_PORT_INDEX,taiNum,CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_DELAY_RESPONSE_E,CPSS_PACKET_CMD_FORWARD_E);
        PRV_UTF_VERIFY_GT_OK(rc, "cpssDxChPtpManagerGlobalPtpDomainSet");

        cpssOsMemSet(&domainEntry, 0, sizeof(domainEntry));
        domainEntry.ptpOverEhernetTsEnable = GT_TRUE;
        domainEntry.ptpOverUdpIpv4TsEnable = GT_FALSE;
        domainEntry.ptpOverUdpIpv6TsEnable = GT_FALSE;
        domainEntry.messageTypeTsEnableBmp =
            (1 << prvTgfPacketSyncPtpV2Part.messageType) | (1 << prvTgfPacketDelayResPtpV2Part.messageType);
        domainEntry.transportSpecificTsEnableBmp =
            (1 << prvTgfPacketSyncPtpV2Part.transportSpecific) | (1 << prvTgfPacketDelayResPtpV2Part.transportSpecific);

        /* Configure action to be performed on PTP packets per{egress port, domain index, message type}. */
        rc = cpssDxChPtpManagerPortPacketActionsConfig(prvTgfDevNum,SLAVE_PORT_INDEX,CPSS_DXCH_PTP_TAI_NUMBER_0_E,taiNum,&domainEntry,
                                                       CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_SYNC_E,CPSS_DXCH_PTP_TS_ACTION_ADD_TIME_E);
        PRV_UTF_VERIFY_GT_OK(rc, "cpssDxChPtpManagerPortPacketActionsConfig");

        cpssOsMemSet(&domainEntry, 0, sizeof(domainEntry));
        domainEntry.ptpOverEhernetTsEnable = GT_TRUE;
        domainEntry.ptpOverUdpIpv4TsEnable = GT_FALSE;
        domainEntry.ptpOverUdpIpv6TsEnable = GT_FALSE;
        domainEntry.messageTypeTsEnableBmp =
            (1 << prvTgfPacketDelayReqPtpV2Part.messageType);
        domainEntry.transportSpecificTsEnableBmp =
            (1 << prvTgfPacketDelayReqPtpV2Part.transportSpecific);

        /* Configure action to be performed on PTP packets per{egress port, domain index, message type}. */
        rc = cpssDxChPtpManagerPortPacketActionsConfig(prvTgfDevNum,MASTER_PORT_INDEX,CPSS_DXCH_PTP_TAI_NUMBER_0_E,taiNum,&domainEntry,
                                                       CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_DELAY_REQUEST_E,CPSS_DXCH_PTP_TS_ACTION_CAPTURE_ADD_TIME_E);
        PRV_UTF_VERIFY_GT_OK(rc, "cpssDxChPtpManagerPortPacketActionsConfig");

        rc = cpssDxChPtpManagerTsQueueConfig(prvTgfDevNum,CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_DELAY_REQUEST_E,0);
        PRV_UTF_VERIFY_GT_OK(rc, "cpssDxChPtpManagerTsQueueConfig");

        if (PRV_CPSS_SIP_6_10_CHECK_MAC(prvTgfDevNum))
        {
            rc = cpssDxChPtpManagerPortTimestampingModeConfig(prvTgfDevNum,SLAVE_PORT_INDEX,CPSS_DXCH_PTP_ENHANCED_TIMESTAMPING_MODE_STANDARD_E,CPSS_DXCH_PTP_MANAGER_OPERATION_MODE_ONE_OR_TWO_STEP_IN_SWITCH_E);
            PRV_UTF_VERIFY_GT_OK(rc, "cpssDxChPtpManagerPortTimestampingModeConfig");
        }

    }
    else
    {
        cpssOsMemSet(&domainEntry, 0, sizeof(domainEntry));
        domainEntry.ptpOverEhernetTsEnable = GT_TRUE;
        domainEntry.ptpOverUdpIpv4TsEnable = GT_FALSE;
        domainEntry.ptpOverUdpIpv6TsEnable = GT_FALSE;
        domainEntry.messageTypeTsEnableBmp =
            (1 << prvTgfPacketSyncPtpV2Part.messageType) | (1 << prvTgfPacketDelayResPtpV2Part.messageType);
        domainEntry.transportSpecificTsEnableBmp =
            (1 << prvTgfPacketSyncPtpV2Part.transportSpecific) | (1 << prvTgfPacketDelayResPtpV2Part.transportSpecific);

        /* Configure action to be performed on PTP packets per{egress port, domain index, message type}. */
        rc = cpssDxChPtpManagerPortPacketActionsConfig(prvTgfDevNum,SLAVE_PORT_INDEX,CPSS_DXCH_PTP_TAI_NUMBER_0_E,taiNum,&domainEntry,
                                                       CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_DELAY_RESPONSE_E,CPSS_DXCH_PTP_TS_ACTION_NONE_E);
        PRV_UTF_VERIFY_GT_OK(rc, "cpssDxChPtpManagerPortPacketActionsConfig");
    }

    return GT_OK;
}

/**
* @internal prvTgfPtpManagerTC1StepE2ECfgSetTrafficGenerateAndCheck function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port given packet uner capture:
*         Check captured packet.
*/
GT_VOID prvTgfPtpManagerTC1StepE2ECfgSetTrafficGenerateAndCheck
(
    IN CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_ENT   messageType
)
{
    GT_STATUS       rc;
    GT_U32          packetActualLength = 0;
    GT_U8           packetBuff[256] = {0};
    GT_U32          buffLen;
    TGF_NET_DSA_STC rxParam;
    GT_U8           queue    = 0;
    GT_U32          tsOffset;
    GT_32           memCmpRc;
    GT_U8           receiveDevNum;

    cpssOsMemSet(&rxParam, 0, sizeof(rxParam));

    switch (messageType)
    {
    case CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_SYNC_E:

        PRV_UTF_LOG0_MAC("\nSYNC MESSAGE MASTER->SLAVE: correctionField will be updated by adding residence time and path delay.\n\n");
        /* Traffic with capture */
        rc = prvTgfTransmitPacketsWithCapture(
            prvTgfDevNum,
            MASTER_PORT_INDEX,
            &prvTgfSyncPtpV2PacketInfo,
            1      /*burstCount*/,
            0      /*numVfd*/,
            NULL   /*vfdArray*/,
            prvTgfDevNum,
            SLAVE_PORT_INDEX,
            TGF_CAPTURE_MODE_MIRRORING_E,
            200     /*captureOnTime*/);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTransmitPacketsWithCapture");

        buffLen  = sizeof(packetBuff);
        /* get entry from rxNetworkIf table */
        rc = tgfTrafficGeneratorRxInCpuGet(
            TGF_PACKET_TYPE_CAPTURE_E,
            GT_TRUE/*getFirst*/, GT_TRUE /*trace*/, packetBuff,
            &buffLen, &packetActualLength,
            &receiveDevNum, &queue, &rxParam);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorRxInCpuGet");

        /* check that timestamp overlapped the correctionField in PTP header */
        tsOffset =
            TGF_L2_HEADER_SIZE_CNS
            + TGF_VLAN_TAG_SIZE_CNS
            + TGF_ETHERTYPE_SIZE_CNS
            + 8 /* correctionField offset in PTP header */;

        memCmpRc = cpssOsMemCmp(&(packetBuff[tsOffset]), prvTgfPacketSyncPtpV2Part.correctionField, 8);
        UTF_VERIFY_NOT_EQUAL0_STRING_MAC(
            0, memCmpRc, "timestamp updated check");

        break;

    case CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_DELAY_REQUEST_E:

        PRV_UTF_LOG0_MAC("\nDELAY_REQUEST MESSAGE SLAVE->MASTER: correctionField will be updated by adding residence time and path delay.\n\n");
        /* Traffic with capture */
        rc = prvTgfTransmitPacketsWithCapture(
            prvTgfDevNum,
            SLAVE_PORT_INDEX,
            &prvTgfDelayReqPtpV2PacketInfo,
            1      /*burstCount*/,
            0      /*numVfd*/,
            NULL   /*vfdArray*/,
            prvTgfDevNum,
            MASTER_PORT_INDEX,
            TGF_CAPTURE_MODE_MIRRORING_E,
            200     /*captureOnTime*/);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTransmitPacketsWithCapture");

        buffLen  = sizeof(packetBuff);
        /* get entry from rxNetworkIf table */
        rc = tgfTrafficGeneratorRxInCpuGet(
            TGF_PACKET_TYPE_CAPTURE_E,
            GT_TRUE/*getFirst*/, GT_TRUE /*trace*/, packetBuff,
            &buffLen, &packetActualLength,
            &receiveDevNum, &queue, &rxParam);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorRxInCpuGet");

        /* check that timestamp overlapped the correctionField in PTP header */
        tsOffset =
            TGF_L2_HEADER_SIZE_CNS
            + TGF_VLAN_TAG_SIZE_CNS
            + TGF_ETHERTYPE_SIZE_CNS
            + 8 /* correctionField offset in PTP header */;

        memCmpRc = cpssOsMemCmp(&(packetBuff[tsOffset]), prvTgfPacketDelayReqPtpV2Part.correctionField, 8);
        UTF_VERIFY_NOT_EQUAL0_STRING_MAC(
            0, memCmpRc, "timestamp updated check");

        break;

    case CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_DELAY_RESPONSE_E:

        PRV_UTF_LOG0_MAC("\nDELAY_RESPONSE MESSAGE MASTER->SLAVE: no change in packet.\n\n");
        /* Traffic with capture */
        rc = prvTgfTransmitPacketsWithCapture(
            prvTgfDevNum,
            MASTER_PORT_INDEX,
            &prvTgfDelayResPtpV2PacketInfo,
            1      /*burstCount*/,
            0      /*numVfd*/,
            NULL   /*vfdArray*/,
            prvTgfDevNum,
            SLAVE_PORT_INDEX,
            TGF_CAPTURE_MODE_MIRRORING_E,
            200     /*captureOnTime*/);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTransmitPacketsWithCapture");

        buffLen  = sizeof(packetBuff);
        /* get entry from rxNetworkIf table */
        rc = tgfTrafficGeneratorRxInCpuGet(
            TGF_PACKET_TYPE_CAPTURE_E,
            GT_TRUE/*getFirst*/, GT_TRUE /*trace*/, packetBuff,
            &buffLen, &packetActualLength,
            &receiveDevNum, &queue, &rxParam);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorRxInCpuGet");

        /* check that the correctionField in PTP header is zero */
        tsOffset =
            TGF_L2_HEADER_SIZE_CNS
            + TGF_VLAN_TAG_SIZE_CNS
            + TGF_ETHERTYPE_SIZE_CNS
            + 8 /* correctionField offset in PTP header */;

        memCmpRc = cpssOsMemCmp(&(packetBuff[tsOffset]), prvTgfPacketDelayResPtpV2Part.correctionField, 8);
        UTF_VERIFY_EQUAL0_STRING_MAC(
            0, memCmpRc, "timestamp updated check");

        break;

    default:
        /* do nothing */
        break;
    }
}


/**
* @internal prvTgfPtpManager1StepFollowUpInformationTlvTrafficGenerateAndCheck function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port given packet uner capture:
*         Check captured packet.
*/
GT_VOID prvTgfPtpManager1StepFollowUpInformationTlvTrafficGenerateAndCheck
(
    GT_VOID
)
{
    GT_STATUS       rc;
    GT_U32          packetActualLength = 0;
    GT_U8           packetBuff[256] = {0};
    GT_U32          buffLen;
    TGF_NET_DSA_STC rxParam;
    GT_U8           queue    = 0;
    GT_U32          tsOffset;
    GT_32           memCmpRc;
    GT_U8           receiveDevNum;
    GT_U32          cumulativeScaledRateOffset = 0x99887766;

    cpssOsMemSet(&rxParam, 0, sizeof(rxParam));

    PRV_UTF_LOG0_MAC("\nSYNC MESSAGE MASTER->SLAVE: cumulativeScaledRateOffset is update in the follow-up information TLV.\n\n");
    /* Traffic with capture */
    rc = prvTgfTransmitPacketsWithCapture(
        prvTgfDevNum,
        MASTER_PORT_INDEX,
        &prvTgfSyncWithTlvPtpV2PacketInfo,
        1      /*burstCount*/,
        0      /*numVfd*/,
        NULL   /*vfdArray*/,
        prvTgfDevNum,
        SLAVE_PORT_INDEX,
        TGF_CAPTURE_MODE_MIRRORING_E,
        200     /*captureOnTime*/);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTransmitPacketsWithCapture");

    buffLen  = sizeof(packetBuff);
    /* get entry from rxNetworkIf table */
    rc = tgfTrafficGeneratorRxInCpuGet(
        TGF_PACKET_TYPE_CAPTURE_E,
        GT_TRUE/*getFirst*/, GT_TRUE /*trace*/, packetBuff,
        &buffLen, &packetActualLength,
        &receiveDevNum, &queue, &rxParam);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorRxInCpuGet");

        /* check that cumulativeScaledRateOffset is update in the follow-up information TLV */
        tsOffset =
            TGF_L2_HEADER_SIZE_CNS
            + TGF_VLAN_TAG_SIZE_CNS
            + TGF_ETHERTYPE_SIZE_CNS
            + 54 /* cumulativeScaledRateOffset offset in TLV header */;

        memCmpRc = cpssOsMemCmp(&(packetBuff[tsOffset]), &cumulativeScaledRateOffset, 4);
        UTF_VERIFY_NOT_EQUAL0_STRING_MAC(
            0, memCmpRc, "cumulativeScaledRateOffset updated check");
}

/**
* @internal prvTgfPtpManagerTC1StepP2PCfgSetTrafficGenerateAndCheck function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port given packet uner capture:
*         Check captured packet.
*/
GT_VOID prvTgfPtpManagerTC1StepP2PCfgSetTrafficGenerateAndCheck
(
    IN CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_ENT   messageType,
    IN GT_U32                                   direction
)
{
    GT_STATUS       rc;
    GT_U32          packetActualLength = 0;
    GT_U8           packetBuff[256] = {0};
    GT_U32          buffLen;
    TGF_NET_DSA_STC rxParam;
    GT_U8           queue    = 0;
    GT_U32          tsOffset;
    GT_32           memCmpRc;
    GT_U8           receiveDevNum;

    cpssOsMemSet(&rxParam, 0, sizeof(rxParam));

    switch (messageType)
    {
    case CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_SYNC_E:

        PRV_UTF_LOG0_MAC("\nSYNC MESSAGE MASTER->SLAVE: correctionField will be updated by adding residence time and path delay.\n\n");
        /* Traffic with capture */
        rc = prvTgfTransmitPacketsWithCapture(
            prvTgfDevNum,
            MASTER_PORT_INDEX,
            &prvTgfSyncPtpV2PacketInfo,
            1      /*burstCount*/,
            0      /*numVfd*/,
            NULL   /*vfdArray*/,
            prvTgfDevNum,
            SLAVE_PORT_INDEX,
            TGF_CAPTURE_MODE_MIRRORING_E,
            200     /*captureOnTime*/);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTransmitPacketsWithCapture");

        buffLen  = sizeof(packetBuff);
        /* get entry from rxNetworkIf table */
        rc = tgfTrafficGeneratorRxInCpuGet(
            TGF_PACKET_TYPE_CAPTURE_E,
            GT_TRUE/*getFirst*/, GT_TRUE /*trace*/, packetBuff,
            &buffLen, &packetActualLength,
            &receiveDevNum, &queue, &rxParam);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorRxInCpuGet");

        /* check that timestamp overlapped the correctionField in PTP header */
        tsOffset =
            TGF_L2_HEADER_SIZE_CNS
            + TGF_VLAN_TAG_SIZE_CNS
            + TGF_ETHERTYPE_SIZE_CNS
            + 8 /* correctionField offset in PTP header */;

        memCmpRc = cpssOsMemCmp(&(packetBuff[tsOffset]), prvTgfPacketSyncPtpV2Part.correctionField, 8);
        UTF_VERIFY_NOT_EQUAL0_STRING_MAC(
            0, memCmpRc, "timestamp updated check");

        break;

    case CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_PDELAY_REQUEST_E:
        if (direction == 1) /* master->slave */
        {
            PRV_UTF_LOG0_MAC("\nPDELAY_REQUEST MESSAGE MASTER->SLAVE: time of transmission is inserted into originTimestamp field.\n\n");
            /* Traffic with capture */
            rc = prvTgfTransmitPacketsWithCapture(
                prvTgfDevNum,
                MASTER_PORT_INDEX,
                &prvTgfPDelayReqPtpV2PacketInfo,
                1      /*burstCount*/,
                0      /*numVfd*/,
                NULL   /*vfdArray*/,
                prvTgfDevNum,
                SLAVE_PORT_INDEX,
                TGF_CAPTURE_MODE_MIRRORING_E,
                200     /*captureOnTime*/);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTransmitPacketsWithCapture");
        }
        else /* slave -> master */
        {
            PRV_UTF_LOG0_MAC("\nPDELAY_REQUEST MESSAGE SLAVE->MASTER: time of transmission is inserted into originTimestamp field.\n\n");
            /* Traffic with capture */
            rc = prvTgfTransmitPacketsWithCapture(
                prvTgfDevNum,
                SLAVE_PORT_INDEX,
                &prvTgfPDelayReqPtpV2PacketInfo1,
                1      /*burstCount*/,
                0      /*numVfd*/,
                NULL   /*vfdArray*/,
                prvTgfDevNum,
                MASTER_PORT_INDEX,
                TGF_CAPTURE_MODE_MIRRORING_E,
                200     /*captureOnTime*/);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTransmitPacketsWithCapture");
        }

        buffLen  = sizeof(packetBuff);
        /* get entry from rxNetworkIf table */
        rc = tgfTrafficGeneratorRxInCpuGet(
            TGF_PACKET_TYPE_CAPTURE_E,
            GT_TRUE/*getFirst*/, GT_TRUE /*trace*/, packetBuff,
            &buffLen, &packetActualLength,
            &receiveDevNum, &queue, &rxParam);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorRxInCpuGet");

        /* check that timestamp overlapped the correctionField in PTP header */
        tsOffset =
            TGF_L2_HEADER_SIZE_CNS
            + TGF_VLAN_TAG_SIZE_CNS
            + TGF_ETHERTYPE_SIZE_CNS
            + 34 /* requestReceiptTimestamp/originTimestamp offset in PTP header */;

        memCmpRc = cpssOsMemCmp(&(packetBuff[tsOffset]), prvTgfPacketPayloadPart.dataPtr, 10);
        UTF_VERIFY_NOT_EQUAL0_STRING_MAC(
            0, memCmpRc, "timestamp updated check");

        break;

    case CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_PDELAY_RESPONSE_E:
        if (direction == 1) /* master->slave */
        {
            PRV_UTF_LOG0_MAC("\nPDELAY_RESPONSE MESSAGE MASTER->SLAVE: time of transmission is inserted into originTimestamp field.\n\n");
            /* Traffic with capture */
            rc = prvTgfTransmitPacketsWithCapture(
                prvTgfDevNum,
                MASTER_PORT_INDEX,
                &prvTgfPDelayResPtpV2PacketInfo1,
                1      /*burstCount*/,
                0      /*numVfd*/,
                NULL   /*vfdArray*/,
                prvTgfDevNum,
                SLAVE_PORT_INDEX,
                TGF_CAPTURE_MODE_MIRRORING_E,
                200     /*captureOnTime*/);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTransmitPacketsWithCapture");
        }
        else /* slave -> master */
        {
            PRV_UTF_LOG0_MAC("\nPDELAY_RESPONSE MESSAGE SLAVE->MASTER: time of transmission is inserted into originTimestamp field.\n\n");
            /* Traffic with capture */
            rc = prvTgfTransmitPacketsWithCapture(
                prvTgfDevNum,
                SLAVE_PORT_INDEX,
                &prvTgfPDelayResPtpV2PacketInfo,
                1      /*burstCount*/,
                0      /*numVfd*/,
                NULL   /*vfdArray*/,
                prvTgfDevNum,
                MASTER_PORT_INDEX,
                TGF_CAPTURE_MODE_MIRRORING_E,
                200     /*captureOnTime*/);
            UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTransmitPacketsWithCapture");
        }

        buffLen  = sizeof(packetBuff);
        /* get entry from rxNetworkIf table */
        rc = tgfTrafficGeneratorRxInCpuGet(
            TGF_PACKET_TYPE_CAPTURE_E,
            GT_TRUE/*getFirst*/, GT_TRUE /*trace*/, packetBuff,
            &buffLen, &packetActualLength,
            &receiveDevNum, &queue, &rxParam);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorRxInCpuGet");

        /* check that timestamp overlapped the correctionField in PTP header */
        tsOffset =
            TGF_L2_HEADER_SIZE_CNS
            + TGF_VLAN_TAG_SIZE_CNS
            + TGF_ETHERTYPE_SIZE_CNS
            + 34 /* requestReceiptTimestamp/originTimestamp offset in PTP header */;

        memCmpRc = cpssOsMemCmp(&(packetBuff[tsOffset]), prvTgfPacketPayloadPart.dataPtr, 10);
        UTF_VERIFY_NOT_EQUAL0_STRING_MAC(
            0, memCmpRc, "timestamp updated check");

        break;

    default:
        /* do nothing */
        break;
    }
}

/**
* @internal prvTgfPtpManagerBC2StepE2ECfgSetTrafficGenerateAndCheck function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port given packet uner capture:
*         Check captured packet.
*/
GT_VOID prvTgfPtpManagerBC2StepE2ECfgSetTrafficGenerateAndCheck
(
    IN CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_ENT   messageType
)
{
    GT_STATUS       rc;
    GT_U32          packetActualLength = 0;
    GT_U8           packetBuff[256] = {0};
    GT_U32          buffLen;
    TGF_NET_DSA_STC rxParam;
    GT_U8           queue    = 0;
    GT_U32          tsOffset;
    GT_32           memCmpRc;
    GT_U8           receiveDevNum;
    CPSS_DXCH_PTP_TS_EGRESS_TIMESTAMP_QUEUE_ENTRY_STC   entry;
    CPSS_DXCH_PTP_MANAGER_TSU_TX_TIMESTAMP_QUEUE_ENTRY_STC portEntry;
    GT_U8           nanoSeconds[4] = {0};

    cpssOsMemSet(&rxParam, 0, sizeof(rxParam));

    switch (messageType)
    {
    case CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_SYNC_E:

        PRV_UTF_LOG0_MAC("\nSYNC MESSAGE MASTER->SLAVE: the time of transmission is captured in timestamp queue.\n\n");
        /* Traffic with capture */
        rc = prvTgfTransmitPacketsWithCapture(
            prvTgfDevNum,
            MASTER_PORT_INDEX,
            &prvTgfSyncPtpV2PacketInfo,
            1      /*burstCount*/,
            0      /*numVfd*/,
            NULL   /*vfdArray*/,
            prvTgfDevNum,
            SLAVE_PORT_INDEX,
            TGF_CAPTURE_MODE_MIRRORING_E,
            200     /*captureOnTime*/);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTransmitPacketsWithCapture");

        buffLen  = sizeof(packetBuff);
        /* get entry from rxNetworkIf table */
        rc = tgfTrafficGeneratorRxInCpuGet(
            TGF_PACKET_TYPE_CAPTURE_E,
            GT_TRUE/*getFirst*/, GT_TRUE /*trace*/, packetBuff,
            &buffLen, &packetActualLength,
            &receiveDevNum, &queue, &rxParam);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorRxInCpuGet");

        /* Read Egress Timestamp Global Queue Entry */
        rc = cpssDxChPtpManagerEgressTimestampGlobalQueueEntryRead(prvTgfDevNum,0,&entry);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChPtpManagerEgressTimestampGlobalQueueEntryRead");

        UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, entry.entryValid, "entryValid");

        UTF_VERIFY_EQUAL0_STRING_MAC(PRV_TGF_PTP_TS_PACKET_TYPE_PTP_V2_E ,entry.packetFormat, "packetFormat");

        UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfPacketSyncPtpV2Part.sequenceId ,entry.sequenceId, "sequenceId");

        /* Read Egress Timestamp Queue Entry per port */
        rc = cpssDxChPtpManagerEgressTimestampPortQueueEntryRead(prvTgfDevNum,entry.portNum,0,&portEntry);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChPtpManagerEgressTimestampPortQueueEntryRead");

        UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, portEntry.entryValid, "entryValid");

        break;
    case CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_FOLLOW_UP_E:

        PRV_UTF_LOG0_MAC("\nFOLLOW_UP MESSAGE MASTER->SLAVE: no change in packet.\n\n");
        /* Traffic with capture */
        rc = prvTgfTransmitPacketsWithCapture(
            prvTgfDevNum,
            MASTER_PORT_INDEX,
            &prvTgfFollowUpPtpV2PacketInfo,
            1      /*burstCount*/,
            0      /*numVfd*/,
            NULL   /*vfdArray*/,
            prvTgfDevNum,
            SLAVE_PORT_INDEX,
            TGF_CAPTURE_MODE_MIRRORING_E,
            200     /*captureOnTime*/);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTransmitPacketsWithCapture");

        buffLen  = sizeof(packetBuff);
        /* get entry from rxNetworkIf table */
        rc = tgfTrafficGeneratorRxInCpuGet(
            TGF_PACKET_TYPE_CAPTURE_E,
            GT_TRUE/*getFirst*/, GT_TRUE /*trace*/, packetBuff,
            &buffLen, &packetActualLength,
            &receiveDevNum, &queue, &rxParam);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorRxInCpuGet");

        break;

    case CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_DELAY_REQUEST_E:

        PRV_UTF_LOG0_MAC("\nDELAY_REQUEST MESSAGE SLAVE->MASTER: the time of transmission is captured in timestamp queue and inserted into the packet <originTimestamp> field.\n\n");
        /* Traffic with capture */
        rc = prvTgfTransmitPacketsWithCapture(
            prvTgfDevNum,
            SLAVE_PORT_INDEX,
            &prvTgfDelayReqPtpV2PacketInfo,
            1      /*burstCount*/,
            0      /*numVfd*/,
            NULL   /*vfdArray*/,
            prvTgfDevNum,
            MASTER_PORT_INDEX,
            TGF_CAPTURE_MODE_MIRRORING_E,
            200     /*captureOnTime*/);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTransmitPacketsWithCapture");

        buffLen  = sizeof(packetBuff);
        /* get entry from rxNetworkIf table */
        rc = tgfTrafficGeneratorRxInCpuGet(
            TGF_PACKET_TYPE_CAPTURE_E,
            GT_TRUE/*getFirst*/, GT_TRUE /*trace*/, packetBuff,
            &buffLen, &packetActualLength,
            &receiveDevNum, &queue, &rxParam);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorRxInCpuGet");

        /* Read Egress Timestamp Global Queue Entry */
        rc = cpssDxChPtpManagerEgressTimestampGlobalQueueEntryRead(prvTgfDevNum,0,&entry);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChPtpManagerEgressTimestampGlobalQueueEntryRead");

        UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, entry.entryValid, "entryValid");

        UTF_VERIFY_EQUAL0_STRING_MAC(PRV_TGF_PTP_TS_PACKET_TYPE_PTP_V2_E ,entry.packetFormat, "packetFormat");

        UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfPacketDelayReqPtpV2Part.sequenceId ,entry.sequenceId, "sequenceId");

        /* Read Egress Timestamp Queue Entry per port */
        rc = cpssDxChPtpManagerEgressTimestampPortQueueEntryRead(prvTgfDevNum,entry.portNum,0,&portEntry);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChPtpManagerEgressTimestampPortQueueEntryRead");

        UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, portEntry.entryValid, "entryValid");

        /* check that timestamp overlapped the correctionField in PTP header */
        tsOffset =
            TGF_L2_HEADER_SIZE_CNS
            + TGF_VLAN_TAG_SIZE_CNS
            + TGF_ETHERTYPE_SIZE_CNS
            + 34 /* requestReceiptTimestamp/originTimestamp offset in PTP header */;

        memCmpRc = cpssOsMemCmp(&(packetBuff[tsOffset]), prvTgfPacketPayloadPart.dataPtr, 10);
        UTF_VERIFY_NOT_EQUAL0_STRING_MAC(
            0, memCmpRc, "timestamp updated check");

        /* check that the nanoSec part in queue is the same as in originTimestamp field */
        nanoSeconds[0] =  portEntry.nanoSeconds & 0xFF;
        nanoSeconds[1] =  (portEntry.nanoSeconds >> 8) & 0xFF;
        nanoSeconds[2] =  (portEntry.nanoSeconds >> 16) & 0xFF;
        nanoSeconds[3] =  (portEntry.nanoSeconds >> 24) & 0xFF;
        memCmpRc = cpssOsMemCmp(&(packetBuff[tsOffset+6]), &(nanoSeconds[0]), 4);
        UTF_VERIFY_NOT_EQUAL0_STRING_MAC(
            0, memCmpRc, "timestamp updated check");
        break;

    case CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_DELAY_RESPONSE_E:

        PRV_UTF_LOG0_MAC("\nDELAY_RESPONSE MESSAGE MASTER->SLAVE: no change in packet.\n\n");
        /* Traffic with capture */
        rc = prvTgfTransmitPacketsWithCapture(
            prvTgfDevNum,
            MASTER_PORT_INDEX,
            &prvTgfDelayResPtpV2PacketInfo,
            1      /*burstCount*/,
            0      /*numVfd*/,
            NULL   /*vfdArray*/,
            prvTgfDevNum,
            SLAVE_PORT_INDEX,
            TGF_CAPTURE_MODE_MIRRORING_E,
            200     /*captureOnTime*/);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTransmitPacketsWithCapture");

        buffLen  = sizeof(packetBuff);
        /* get entry from rxNetworkIf table */
        rc = tgfTrafficGeneratorRxInCpuGet(
            TGF_PACKET_TYPE_CAPTURE_E,
            GT_TRUE/*getFirst*/, GT_TRUE /*trace*/, packetBuff,
            &buffLen, &packetActualLength,
            &receiveDevNum, &queue, &rxParam);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorRxInCpuGet");

        /* check that the correctionField in PTP header is zero */
        tsOffset =
            TGF_L2_HEADER_SIZE_CNS
            + TGF_VLAN_TAG_SIZE_CNS
            + TGF_ETHERTYPE_SIZE_CNS
            + 8 /* correctionField offset in PTP header */;

        memCmpRc = cpssOsMemCmp(&(packetBuff[tsOffset]), prvTgfPacketDelayResPtpV2Part.correctionField, 8);
        UTF_VERIFY_EQUAL0_STRING_MAC(
            0, memCmpRc, "timestamp updated check");

        break;

    default:
        /* do nothing */
        break;
    }
}

/**
* @internal prvTgfPtpManagerBC1StepE2ECfgSetTrafficGenerateAndCheck function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port given packet uner capture:
*         Check captured packet.
*/
GT_VOID prvTgfPtpManagerBC1StepE2ECfgSetTrafficGenerateAndCheck
(
    IN CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_ENT   messageType
)
{
    GT_STATUS       rc;
    GT_U32          packetActualLength = 0;
    GT_U8           packetBuff[256] = {0};
    GT_U32          buffLen;
    TGF_NET_DSA_STC rxParam;
    GT_U8           queue    = 0;
    GT_U32          tsOffset;
    GT_32           memCmpRc;
    GT_U8           receiveDevNum;
    CPSS_DXCH_PTP_TS_EGRESS_TIMESTAMP_QUEUE_ENTRY_STC   entry;
    CPSS_DXCH_PTP_MANAGER_TSU_TX_TIMESTAMP_QUEUE_ENTRY_STC portEntry;
    GT_U8           nanoSeconds[4] = {0};

    cpssOsMemSet(&rxParam, 0, sizeof(rxParam));

    switch (messageType)
    {
    case CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_SYNC_E:

        PRV_UTF_LOG0_MAC("\nSYNC MESSAGE MASTER->SLAVE: time of transmission is inserted into originTimestamp field.\n\n");
        /* Traffic with capture */
        rc = prvTgfTransmitPacketsWithCapture(
            prvTgfDevNum,
            MASTER_PORT_INDEX,
            &prvTgfSyncPtpV2PacketInfo,
            1      /*burstCount*/,
            0      /*numVfd*/,
            NULL   /*vfdArray*/,
            prvTgfDevNum,
            SLAVE_PORT_INDEX,
            TGF_CAPTURE_MODE_MIRRORING_E,
            200     /*captureOnTime*/);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTransmitPacketsWithCapture");

        buffLen  = sizeof(packetBuff);
        /* get entry from rxNetworkIf table */
        rc = tgfTrafficGeneratorRxInCpuGet(
            TGF_PACKET_TYPE_CAPTURE_E,
            GT_TRUE/*getFirst*/, GT_TRUE /*trace*/, packetBuff,
            &buffLen, &packetActualLength,
            &receiveDevNum, &queue, &rxParam);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorRxInCpuGet");


        /* check that timestamp overlapped the correctionField in PTP header */
        tsOffset =
            TGF_L2_HEADER_SIZE_CNS
            + TGF_VLAN_TAG_SIZE_CNS
            + TGF_ETHERTYPE_SIZE_CNS
            + 34 /* requestReceiptTimestamp/originTimestamp offset in PTP header */;

        memCmpRc = cpssOsMemCmp(&(packetBuff[tsOffset]), prvTgfPacketPayloadPart.dataPtr, 10);
        UTF_VERIFY_NOT_EQUAL0_STRING_MAC(
            0, memCmpRc, "timestamp updated check");
        break;

    case CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_DELAY_REQUEST_E:

        PRV_UTF_LOG0_MAC("\nDELAY_REQUEST MESSAGE SLAVE->MASTER: the time of transmission is captured in timestamp queue and inserted into the packet <originTimestamp> field.\n\n");
        /* Traffic with capture */
        rc = prvTgfTransmitPacketsWithCapture(
            prvTgfDevNum,
            SLAVE_PORT_INDEX,
            &prvTgfDelayReqPtpV2PacketInfo,
            1      /*burstCount*/,
            0      /*numVfd*/,
            NULL   /*vfdArray*/,
            prvTgfDevNum,
            MASTER_PORT_INDEX,
            TGF_CAPTURE_MODE_MIRRORING_E,
            200     /*captureOnTime*/);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTransmitPacketsWithCapture");

        buffLen  = sizeof(packetBuff);
        /* get entry from rxNetworkIf table */
        rc = tgfTrafficGeneratorRxInCpuGet(
            TGF_PACKET_TYPE_CAPTURE_E,
            GT_TRUE/*getFirst*/, GT_TRUE /*trace*/, packetBuff,
            &buffLen, &packetActualLength,
            &receiveDevNum, &queue, &rxParam);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorRxInCpuGet");

        /* Read Egress Timestamp Global Queue Entry */
        rc = cpssDxChPtpManagerEgressTimestampGlobalQueueEntryRead(prvTgfDevNum,0,&entry);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChPtpManagerEgressTimestampGlobalQueueEntryRead");

        UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, entry.entryValid, "entryValid");

        UTF_VERIFY_EQUAL0_STRING_MAC(PRV_TGF_PTP_TS_PACKET_TYPE_PTP_V2_E ,entry.packetFormat, "packetFormat");

        UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfPacketDelayReqPtpV2Part.sequenceId ,entry.sequenceId, "sequenceId");

        /* Read Egress Timestamp Queue Entry per port */
        rc = cpssDxChPtpManagerEgressTimestampPortQueueEntryRead(prvTgfDevNum,entry.portNum,0,&portEntry);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChPtpManagerEgressTimestampPortQueueEntryRead");

        UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, portEntry.entryValid, "entryValid");

        /* check that timestamp overlapped the correctionField in PTP header */
        tsOffset =
            TGF_L2_HEADER_SIZE_CNS
            + TGF_VLAN_TAG_SIZE_CNS
            + TGF_ETHERTYPE_SIZE_CNS
            + 34 /* requestReceiptTimestamp/originTimestamp offset in PTP header */;

        memCmpRc = cpssOsMemCmp(&(packetBuff[tsOffset]), prvTgfPacketPayloadPart.dataPtr, 10);
        UTF_VERIFY_NOT_EQUAL0_STRING_MAC(
            0, memCmpRc, "timestamp updated check");

        /* check that the nanoSec part in queue is the same as in originTimestamp field */
        nanoSeconds[0] =  portEntry.nanoSeconds & 0xFF;
        nanoSeconds[1] =  (portEntry.nanoSeconds >> 8) & 0xFF;
        nanoSeconds[2] =  (portEntry.nanoSeconds >> 16) & 0xFF;
        nanoSeconds[3] =  (portEntry.nanoSeconds >> 24) & 0xFF;
        memCmpRc = cpssOsMemCmp(&(packetBuff[tsOffset+6]), &(nanoSeconds[0]), 4);
        UTF_VERIFY_NOT_EQUAL0_STRING_MAC(
            0, memCmpRc, "timestamp updated check");
        break;

    case CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_DELAY_RESPONSE_E:

        PRV_UTF_LOG0_MAC("\nDELAY_RESPONSE MESSAGE MASTER->SLAVE: no change in packet.\n\n");
        /* Traffic with capture */
        rc = prvTgfTransmitPacketsWithCapture(
            prvTgfDevNum,
            MASTER_PORT_INDEX,
            &prvTgfDelayResPtpV2PacketInfo,
            1      /*burstCount*/,
            0      /*numVfd*/,
            NULL   /*vfdArray*/,
            prvTgfDevNum,
            SLAVE_PORT_INDEX,
            TGF_CAPTURE_MODE_MIRRORING_E,
            200     /*captureOnTime*/);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTransmitPacketsWithCapture");

        buffLen  = sizeof(packetBuff);
        /* get entry from rxNetworkIf table */
        rc = tgfTrafficGeneratorRxInCpuGet(
            TGF_PACKET_TYPE_CAPTURE_E,
            GT_TRUE/*getFirst*/, GT_TRUE /*trace*/, packetBuff,
            &buffLen, &packetActualLength,
            &receiveDevNum, &queue, &rxParam);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorRxInCpuGet");

        /* check that the correctionField in PTP header is zero */
        tsOffset =
            TGF_L2_HEADER_SIZE_CNS
            + TGF_VLAN_TAG_SIZE_CNS
            + TGF_ETHERTYPE_SIZE_CNS
            + 8 /* correctionField offset in PTP header */;

        memCmpRc = cpssOsMemCmp(&(packetBuff[tsOffset]), prvTgfPacketDelayResPtpV2Part.correctionField, 8);
        UTF_VERIFY_EQUAL0_STRING_MAC(
            0, memCmpRc, "timestamp updated check");

        break;

    default:
        /* do nothing */
        break;
    }
}

/**
* @internal prvTgfPtpManagerGenVidAndFdbDefCfgSet function
* @endinternal
*
* @brief   None
*/
GT_STATUS prvTgfPtpManagerGenVidAndFdbDefCfgSet
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;

    /* forces Link Up on all ports */
    rc = prvTgfEthCountersReset(prvTgfDevNum);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfEthCountersReset");

    rc = prvTgfBrgDefVlanEntryWriteWithTaggingCmd(
        PRV_TGF_PTP_GEN_VLAN_CNS,
        PRV_TGF_BRG_VLAN_PORT_TAG0_CMD_E);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfBrgDefVlanEntryWriteWithTaggingCmd");

    rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum,
        MASTER_PORT_INDEX, PRV_TGF_PTP_GEN_VLAN_CNS);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfBrgVlanPortVidSet");

    rc = prvTgfBrgVlanPortVidSet(prvTgfDevNum,
        SLAVE_PORT_INDEX, PRV_TGF_PTP_GEN_VLAN_CNS);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfBrgVlanPortVidSet");

    rc = prvTgfBrgDefFdbMacEntryOnPortSet(
            prvTgfPacketL2Part.daMac,
            PRV_TGF_PTP_GEN_VLAN_CNS, prvTgfDevNum,
            SLAVE_PORT_INDEX, GT_TRUE);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfBrgDefFdbMacEntryOnPortSet");

    rc = prvTgfBrgDefFdbMacEntryOnPortSet(
            prvTgfPacketL2Part1.daMac,
            PRV_TGF_PTP_GEN_VLAN_CNS, prvTgfDevNum,
            MASTER_PORT_INDEX, GT_TRUE);
    PRV_UTF_VERIFY_GT_OK(rc, "prvTgfBrgDefFdbMacEntryOnPortSet");

    return rc;
};

/**
* @internal prvTgfPtpManagerUsxgmiiMchEgress1StepInSwitch1StepInPhyCfgSet function
* @endinternal
*
* @brief   None
*/
GT_STATUS prvTgfPtpManagerUsxgmiiMchEgress1StepInSwitch1StepInPhyCfgSet
(
    GT_VOID
)
{
    GT_STATUS                               rc = GT_OK;
    CPSS_DXCH_PTP_MANAGER_DOMAIN_STC        domainConf;
    CPSS_DXCH_PTP_EGRESS_DOMAIN_ENTRY_STC   domainEntry;

    domainConf.domainIndex = 2; /* the local domain index to which the domainID is directed */
    domainConf.domainMode = CPSS_DXCH_PTP_INGRESS_DOMAIN_MODE_PTP_V2_E;
    domainConf.v2DomainId = prvTgfPacketSyncPtpV2Part.domainNumber; /* domainID is the domain number which is passed in the PTP header */

    /* Configure the global settings for PTP Domain */
    rc = cpssDxChPtpManagerGlobalPtpDomainSet(prvTgfDevNum,&domainConf);
    PRV_UTF_VERIFY_GT_OK(rc, "cpssDxChPtpManagerGlobalPtpDomainSet");

    /* Allocate Port Interface to TAI */
    rc = cpssDxChPtpManagerPortAllocateTai(prvTgfDevNum,MASTER_PORT_INDEX,CPSS_DXCH_PTP_TAI_NUMBER_0_E,CPSS_DXCH_PTP_TS_DELAY_TAI_SEL_MODE_EGRESS_E);
    PRV_UTF_VERIFY_GT_OK(rc, "cpssDxChPtpManagerGlobalPtpDomainSet");

    /* Assign packet command per{ingress port, domain index, message type}. */
    rc = cpssDxChPtpManagerPortCommandAssignment(prvTgfDevNum,MASTER_PORT_INDEX,2,CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_SYNC_E,CPSS_PACKET_CMD_FORWARD_E);
    PRV_UTF_VERIFY_GT_OK(rc, "cpssDxChPtpManagerGlobalPtpDomainSet");

    cpssOsMemSet(&domainEntry, 0, sizeof(domainEntry));
    domainEntry.ptpOverEhernetTsEnable = GT_TRUE;
    domainEntry.ptpOverUdpIpv4TsEnable = GT_FALSE;
    domainEntry.ptpOverUdpIpv6TsEnable = GT_FALSE;
    domainEntry.messageTypeTsEnableBmp =
        (1 << prvTgfPacketSyncPtpV2Part.messageType);
    domainEntry.transportSpecificTsEnableBmp =
        (1 << prvTgfPacketSyncPtpV2Part.transportSpecific);

    /* Configure action to be performed on PTP packets per{egress port, domain index, message type}. */
    rc = cpssDxChPtpManagerPortPacketActionsConfig(prvTgfDevNum,SLAVE_PORT_INDEX,CPSS_DXCH_PTP_TAI_NUMBER_0_E,2,&domainEntry,
                                                   CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_SYNC_E,CPSS_DXCH_PTP_TS_ACTION_ADD_TIME_E);
    PRV_UTF_VERIFY_GT_OK(rc, "cpssDxChPtpManagerPortPacketActionsConfig");

    /* Configure action to be performed on PTP packets per{egress port, domain index, message type}. */
    rc = cpssDxChPtpManagerPortPacketActionsConfig(prvTgfDevNum,MASTER_PORT_INDEX,CPSS_DXCH_PTP_TAI_NUMBER_0_E,2,&domainEntry,
                                                   CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_SYNC_E,CPSS_DXCH_PTP_TS_ACTION_ADD_TIME_E);
    PRV_UTF_VERIFY_GT_OK(rc, "cpssDxChPtpManagerPortPacketActionsConfig");

    /* Configure Enhanced Timestamping Mode per Egress port */
    rc = cpssDxChPtpManagerPortTimestampingModeConfig(prvTgfDevNum,MASTER_PORT_INDEX,CPSS_DXCH_PTP_ENHANCED_TIMESTAMPING_MODE_USGMII_E,CPSS_DXCH_PTP_MANAGER_OPERATION_MODE_ONE_OR_TWO_STEP_IN_USGMII_PHY_MARVELL_FORMAT_E);
    PRV_UTF_VERIFY_GT_OK(rc, "cpssDxChPtpManagerPortTimestampingModeConfig");

#ifdef ASIC_SIMULATION
    /* state to simulation that this egress port is connected to PHY */
    wmSetPortConnectedToPhyEgress(MASTER_PORT_INDEX,GT_TRUE);

    /* state to simulation that phy mode is 1-step */
    wmSetPhyIn2StepMode(GT_FALSE);
#endif

    return GT_OK;
}

/**
* @internal prvTgfPtpManagerUsxgmiiMchEgress1StepInSwitch1StepInPhyTrafficGenerateAndCheck function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port given packet uner capture:
*         Check captured packet.
*/
GT_VOID prvTgfPtpManagerUsxgmiiMchEgress1StepInSwitch1StepInPhyTrafficGenerateAndCheck
(
    GT_VOID
)
{
    GT_STATUS       rc;
    GT_U32          packetActualLength = 0;
    GT_U8           packetBuff[256] = {0};
    GT_U32          buffLen;
    TGF_NET_DSA_STC rxParam;
    GT_U8           queue    = 0;
    GT_U32          tsOffset;
    GT_32           memCmpRc;
    GT_U8           receiveDevNum;

    cpssOsMemSet(&rxParam, 0, sizeof(rxParam));

    PRV_UTF_LOG0_MAC("\nSYNC MESSAGE MASTER->SLAVE: CTSU - time of transmission is inserted originTimestamp field; PHY - update correctionField.\n\n");
    /* Traffic with capture */
    rc = prvTgfTransmitPacketsWithCapture(
        prvTgfDevNum,
        MASTER_PORT_INDEX,
        &prvTgfSyncPtpV2PacketInfo,
        1      /*burstCount*/,
        0      /*numVfd*/,
        NULL   /*vfdArray*/,
        prvTgfDevNum,
        SLAVE_PORT_INDEX,
        TGF_CAPTURE_MODE_MIRRORING_E,
        200     /*captureOnTime*/);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTransmitPacketsWithCapture");

    buffLen  = sizeof(packetBuff);
    /* get entry from rxNetworkIf table */
    rc = tgfTrafficGeneratorRxInCpuGet(
        TGF_PACKET_TYPE_CAPTURE_E,
        GT_TRUE/*getFirst*/, GT_TRUE /*trace*/, packetBuff,
        &buffLen, &packetActualLength,
        &receiveDevNum, &queue, &rxParam);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorRxInCpuGet");

    /* check that timestamp overlapped the correctionField in PTP header */
    tsOffset =
        TGF_L2_HEADER_SIZE_CNS
        + TGF_VLAN_TAG_SIZE_CNS
        + TGF_ETHERTYPE_SIZE_CNS
        + 8 /* correctionField offset in PTP header */;

    memCmpRc = cpssOsMemCmp(&(packetBuff[tsOffset]), prvTgfPacketSyncPtpV2Part.correctionField, 8);
    UTF_VERIFY_NOT_EQUAL0_STRING_MAC(
        0, memCmpRc, "correctionField updated check");

    /* check that timestamp overlapped the requestReceiptTimestamp/originTimestamp in PTP header */
    tsOffset =
        TGF_L2_HEADER_SIZE_CNS
        + TGF_VLAN_TAG_SIZE_CNS
        + TGF_ETHERTYPE_SIZE_CNS
        + 34 /* requestReceiptTimestamp/originTimestamp offset in PTP header */;

    memCmpRc = cpssOsMemCmp(&(packetBuff[tsOffset]), prvTgfPacketPayloadPart.dataPtr, 10);
    UTF_VERIFY_NOT_EQUAL0_STRING_MAC(
        0, memCmpRc, "timestamp updated check");

#ifdef ASIC_SIMULATION
    /* state to simulation that this egress port is not connected to PHY */
    wmSetPortConnectedToPhyEgress(MASTER_PORT_INDEX,GT_FALSE);

    /* state to simulation that phy mode is 1-step */
    wmSetPhyIn2StepMode(GT_FALSE);
#endif

}

/**
* @internal prvTgfPtpManagerUsxgmiiMchEgress2StepInSwitch1StepInPhyCfgSet function
* @endinternal
*
* @brief   None
*/
GT_STATUS prvTgfPtpManagerUsxgmiiMchEgress2StepInSwitch1StepInPhyCfgSet
(
    GT_VOID
)
{
    GT_STATUS                               rc = GT_OK;
    CPSS_DXCH_PTP_MANAGER_DOMAIN_STC        domainConf;
    CPSS_DXCH_PTP_EGRESS_DOMAIN_ENTRY_STC   domainEntry;

    domainConf.domainIndex = 2; /* the local domain index to which the domainID is directed */
    domainConf.domainMode = CPSS_DXCH_PTP_INGRESS_DOMAIN_MODE_PTP_V2_E;
    domainConf.v2DomainId = prvTgfPacketSyncPtpV2Part.domainNumber; /* domainID is the domain number which is passed in the PTP header */

    /* Configure the global settings for PTP Domain */
    rc = cpssDxChPtpManagerGlobalPtpDomainSet(prvTgfDevNum,&domainConf);
    PRV_UTF_VERIFY_GT_OK(rc, "cpssDxChPtpManagerGlobalPtpDomainSet");

    /* Allocate Port Interface to TAI */
    rc = cpssDxChPtpManagerPortAllocateTai(prvTgfDevNum,MASTER_PORT_INDEX,CPSS_DXCH_PTP_TAI_NUMBER_0_E,CPSS_DXCH_PTP_TS_DELAY_TAI_SEL_MODE_EGRESS_E);
    PRV_UTF_VERIFY_GT_OK(rc, "cpssDxChPtpManagerGlobalPtpDomainSet");

    /* Assign packet command per{ingress port, domain index, message type}. */
    rc = cpssDxChPtpManagerPortCommandAssignment(prvTgfDevNum,MASTER_PORT_INDEX,2,CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_SYNC_E,CPSS_PACKET_CMD_FORWARD_E);
    PRV_UTF_VERIFY_GT_OK(rc, "cpssDxChPtpManagerGlobalPtpDomainSet");

    cpssOsMemSet(&domainEntry, 0, sizeof(domainEntry));
    domainEntry.ptpOverEhernetTsEnable = GT_TRUE;
    domainEntry.ptpOverUdpIpv4TsEnable = GT_FALSE;
    domainEntry.ptpOverUdpIpv6TsEnable = GT_FALSE;
    domainEntry.messageTypeTsEnableBmp =
        (1 << prvTgfPacketSyncPtpV2Part.messageType);
    domainEntry.transportSpecificTsEnableBmp =
        (1 << prvTgfPacketSyncPtpV2Part.transportSpecific);

    /* Configure action to be performed on PTP packets per{egress port, domain index, message type}. */
    rc = cpssDxChPtpManagerPortPacketActionsConfig(prvTgfDevNum,SLAVE_PORT_INDEX,CPSS_DXCH_PTP_TAI_NUMBER_0_E,2,&domainEntry,
                                                   CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_SYNC_E,CPSS_DXCH_PTP_TS_ACTION_CAPTURE_E);
    PRV_UTF_VERIFY_GT_OK(rc, "cpssDxChPtpManagerPortPacketActionsConfig");

    /* Configure action to be performed on PTP packets per{egress port, domain index, message type}. */
    rc = cpssDxChPtpManagerPortPacketActionsConfig(prvTgfDevNum,MASTER_PORT_INDEX,CPSS_DXCH_PTP_TAI_NUMBER_0_E,2,&domainEntry,
                                                   CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_SYNC_E,CPSS_DXCH_PTP_TS_ACTION_CAPTURE_E);
    PRV_UTF_VERIFY_GT_OK(rc, "cpssDxChPtpManagerPortPacketActionsConfig");

    /* Configure Enhanced Timestamping Mode per Egress port */
    rc = cpssDxChPtpManagerPortTimestampingModeConfig(prvTgfDevNum,MASTER_PORT_INDEX,CPSS_DXCH_PTP_ENHANCED_TIMESTAMPING_MODE_USGMII_E,CPSS_DXCH_PTP_MANAGER_OPERATION_MODE_ONE_OR_TWO_STEP_IN_USGMII_PHY_MARVELL_FORMAT_E);
    PRV_UTF_VERIFY_GT_OK(rc, "cpssDxChPtpManagerPortTimestampingModeConfig");

#ifdef ASIC_SIMULATION
    /* state to simulation that this egress port is connected to PHY */
    wmSetPortConnectedToPhyEgress(MASTER_PORT_INDEX,GT_TRUE);

    /* state to simulation that phy mode is 1-step */
    wmSetPhyIn2StepMode(GT_FALSE);
#endif

    return GT_OK;
}

/**
* @internal prvTgfPtpManagerUsxgmiiMchEgress2StepInSwitch1StepInPhyTrafficGenerateAndCheck function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port given packet uner capture:
*         Check captured packet.
*/
GT_VOID prvTgfPtpManagerUsxgmiiMchEgress2StepInSwitch1StepInPhyTrafficGenerateAndCheck
(
    GT_VOID
)
{
    GT_STATUS       rc;
    GT_U32          packetActualLength = 0;
    GT_U8           packetBuff[256] = {0};
    GT_U32          buffLen;
    TGF_NET_DSA_STC rxParam;
    GT_U8           queue    = 0;
    GT_U32          tsOffset;
    GT_32           memCmpRc;
    GT_U8           receiveDevNum;
    CPSS_DXCH_PTP_TS_EGRESS_TIMESTAMP_QUEUE_ENTRY_STC tsQueueEntry;
    CPSS_DXCH_PTP_MANAGER_TSU_TX_TIMESTAMP_QUEUE_ENTRY_STC tsPortQueueEntry;

    cpssOsMemSet(&rxParam, 0, sizeof(rxParam));

    PRV_UTF_LOG0_MAC("\nSYNC MESSAGE MASTER->SLAVE: CTSU - time of transmission is inserted to TS Queues; PHY - update correctionField.\n\n");
    /* Traffic with capture */
    rc = prvTgfTransmitPacketsWithCapture(
        prvTgfDevNum,
        MASTER_PORT_INDEX,
        &prvTgfSyncPtpV2PacketInfo,
        1      /*burstCount*/,
        0      /*numVfd*/,
        NULL   /*vfdArray*/,
        prvTgfDevNum,
        SLAVE_PORT_INDEX,
        TGF_CAPTURE_MODE_MIRRORING_E,
        200     /*captureOnTime*/);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTransmitPacketsWithCapture");

    buffLen  = sizeof(packetBuff);
    /* get entry from rxNetworkIf table */
    rc = tgfTrafficGeneratorRxInCpuGet(
        TGF_PACKET_TYPE_CAPTURE_E,
        GT_TRUE/*getFirst*/, GT_TRUE /*trace*/, packetBuff,
        &buffLen, &packetActualLength,
        &receiveDevNum, &queue, &rxParam);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorRxInCpuGet");

    /* check that timestamp overlapped the correctionField in PTP header */
    tsOffset =
        TGF_L2_HEADER_SIZE_CNS
        + TGF_VLAN_TAG_SIZE_CNS
        + TGF_ETHERTYPE_SIZE_CNS
        + 8 /* correctionField offset in PTP header */;

    memCmpRc = cpssOsMemCmp(&(packetBuff[tsOffset]), prvTgfPacketSyncPtpV2Part.correctionField, 8);
    UTF_VERIFY_NOT_EQUAL0_STRING_MAC(
        0, memCmpRc, "correctionField updated check");

    /* check that timestamp doesn't overlapped the requestReceiptTimestamp/originTimestamp in PTP header */
    tsOffset =
        TGF_L2_HEADER_SIZE_CNS
        + TGF_VLAN_TAG_SIZE_CNS
        + TGF_ETHERTYPE_SIZE_CNS
        + 34 /* requestReceiptTimestamp/originTimestamp offset in PTP header */;

    memCmpRc = cpssOsMemCmp(&(packetBuff[tsOffset]), prvTgfPacketPayloadPart.dataPtr, 10);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        0, memCmpRc, "timestamp updated check");

    /* read ERMRK Egress queue and check it has a valid entry */
    rc = cpssDxChPtpManagerEgressTimestampGlobalQueueEntryRead(prvTgfDevNum,0 /*queueNum*/, &tsQueueEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChPtpManagerEgressTimestampGlobalQueueEntryRead");

    UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, tsQueueEntry.entryValid, "entryValid");

    PRV_UTF_LOG3_MAC("[TGF]: cpssDxChPtpManagerEgressTimestampGlobalQueueEntryRead: valid entry [%d] queueEntryId [%d] portNum [%d]\n",tsQueueEntry.entryValid, tsQueueEntry.queueEntryId, tsQueueEntry.portNum);

    UTF_VERIFY_EQUAL0_STRING_MAC(PRV_TGF_PTP_TS_PACKET_TYPE_PTP_V2_E  , tsQueueEntry.packetFormat, "packetFormat");

    UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfPacketSyncPtpV2Part.sequenceId  , tsQueueEntry.sequenceId, "sequenceId");

    /* read CTSU queue per port and check it has a valid entry */
    rc = cpssDxChPtpManagerEgressTimestampPortQueueEntryRead(prvTgfDevNum,tsQueueEntry.portNum,0 /*queueNum*/,&tsPortQueueEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChPtpManagerEgressTimestampPortQueueEntryRead");

    UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, tsPortQueueEntry.entryValid, "entryValid");

    PRV_UTF_LOG3_MAC("[TGF]: cpssDxChPtpManagerEgressTimestampPortQueueEntryRead: valid entry [%d] queueEntryId [%d] portNum [%d]\n",tsPortQueueEntry.entryValid, tsPortQueueEntry.queueEntryId, tsQueueEntry.portNum);

#ifdef ASIC_SIMULATION
    /* state to simulation that this egress port is not connected to PHY */
    wmSetPortConnectedToPhyEgress(MASTER_PORT_INDEX,GT_FALSE);

    /* state to simulation that phy mode is 1-step */
    wmSetPhyIn2StepMode(GT_FALSE);
#endif

}

/**
* @internal prvTgfPtpManagerUsxgmiiMchEgress1StepInSwitch1StepInPhyTcModeCfgSet function
* @endinternal
*
* @brief   None
*/
GT_STATUS prvTgfPtpManagerUsxgmiiMchEgress1StepInSwitch1StepInPhyTcModeCfgSet
(
    GT_VOID
)
{
    GT_STATUS                               rc = GT_OK;
    CPSS_DXCH_PTP_MANAGER_DOMAIN_STC        domainConf;
    CPSS_DXCH_PTP_EGRESS_DOMAIN_ENTRY_STC   domainEntry;

    domainConf.domainIndex = 2; /* the local domain index to which the domainID is directed */
    domainConf.domainMode = CPSS_DXCH_PTP_INGRESS_DOMAIN_MODE_PTP_V2_E;
    domainConf.v2DomainId = prvTgfPacketSyncPtpV2Part.domainNumber; /* domainID is the domain number which is passed in the PTP header */

    /* Configure the global settings for PTP Domain */
    rc = cpssDxChPtpManagerGlobalPtpDomainSet(prvTgfDevNum,&domainConf);
    PRV_UTF_VERIFY_GT_OK(rc, "cpssDxChPtpManagerGlobalPtpDomainSet");

    /* Allocate Port Interface to TAI */
    rc = cpssDxChPtpManagerPortAllocateTai(prvTgfDevNum,MASTER_PORT_INDEX,CPSS_DXCH_PTP_TAI_NUMBER_0_E,CPSS_DXCH_PTP_TS_DELAY_TAI_SEL_MODE_EGRESS_E);
    PRV_UTF_VERIFY_GT_OK(rc, "cpssDxChPtpManagerGlobalPtpDomainSet");

    /* Assign packet command per{ingress port, domain index, message type}. */
    rc = cpssDxChPtpManagerPortCommandAssignment(prvTgfDevNum,MASTER_PORT_INDEX,2,CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_SYNC_E,CPSS_PACKET_CMD_FORWARD_E);
    PRV_UTF_VERIFY_GT_OK(rc, "cpssDxChPtpManagerGlobalPtpDomainSet");

    cpssOsMemSet(&domainEntry, 0, sizeof(domainEntry));
    domainEntry.ptpOverEhernetTsEnable = GT_TRUE;
    domainEntry.ptpOverUdpIpv4TsEnable = GT_FALSE;
    domainEntry.ptpOverUdpIpv6TsEnable = GT_FALSE;
    domainEntry.messageTypeTsEnableBmp =
        (1 << prvTgfPacketSyncPtpV2Part.messageType);
    domainEntry.transportSpecificTsEnableBmp =
        (1 << prvTgfPacketSyncPtpV2Part.transportSpecific);

    /* Configure action to be performed on PTP packets per{egress port, domain index, message type}. */
    rc = cpssDxChPtpManagerPortPacketActionsConfig(prvTgfDevNum,SLAVE_PORT_INDEX,CPSS_DXCH_PTP_TAI_NUMBER_0_E,2,&domainEntry,
                                                   CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_SYNC_E,CPSS_DXCH_PTP_TS_ACTION_ADD_CORRECTED_TIME_E);
    PRV_UTF_VERIFY_GT_OK(rc, "cpssDxChPtpManagerPortPacketActionsConfig");

    /* Configure action to be performed on PTP packets per{egress port, domain index, message type}. */
    rc = cpssDxChPtpManagerPortPacketActionsConfig(prvTgfDevNum,MASTER_PORT_INDEX,CPSS_DXCH_PTP_TAI_NUMBER_0_E,2,&domainEntry,
                                                   CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_SYNC_E,CPSS_DXCH_PTP_TS_ACTION_ADD_CORRECTED_TIME_E);
    PRV_UTF_VERIFY_GT_OK(rc, "cpssDxChPtpManagerPortPacketActionsConfig");

    /* Configure Enhanced Timestamping Mode per Egress port */
    rc = cpssDxChPtpManagerPortTimestampingModeConfig(prvTgfDevNum,MASTER_PORT_INDEX,CPSS_DXCH_PTP_ENHANCED_TIMESTAMPING_MODE_USGMII_E,CPSS_DXCH_PTP_MANAGER_OPERATION_MODE_ONE_OR_TWO_STEP_IN_USGMII_PHY_MARVELL_FORMAT_E);
    PRV_UTF_VERIFY_GT_OK(rc, "cpssDxChPtpManagerPortTimestampingModeConfig");

#ifdef ASIC_SIMULATION
    /* state to simulation that this egress port is connected to PHY */
    wmSetPortConnectedToPhyEgress(MASTER_PORT_INDEX,GT_TRUE);

    /* state to simulation that phy mode is 1-step */
    wmSetPhyIn2StepMode(GT_FALSE);
#endif

    return GT_OK;
}

/**
* @internal prvTgfPtpManagerUsxgmiiMchEgress1StepInSwitch1StepInPhyTcModeTrafficGenerateAndCheck function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port given packet uner capture:
*         Check captured packet.
*/
GT_VOID prvTgfPtpManagerUsxgmiiMchEgress1StepInSwitch1StepInPhyTcModeTrafficGenerateAndCheck
(
    GT_VOID
)
{
    GT_STATUS       rc;
    GT_U32          packetActualLength = 0;
    GT_U8           packetBuff[256] = {0};
    GT_U32          buffLen;
    TGF_NET_DSA_STC rxParam;
    GT_U8           queue    = 0;
    GT_U32          tsOffset;
    GT_32           memCmpRc;
    GT_U8           receiveDevNum;

    cpssOsMemSet(&rxParam, 0, sizeof(rxParam));

    PRV_UTF_LOG0_MAC("\nSYNC MESSAGE MASTER->SLAVE: CTSU + PHY - update correctionField.\n\n");
    /* Traffic with capture */
    rc = prvTgfTransmitPacketsWithCapture(
        prvTgfDevNum,
        MASTER_PORT_INDEX,
        &prvTgfSyncPtpV2PacketInfo,
        1      /*burstCount*/,
        0      /*numVfd*/,
        NULL   /*vfdArray*/,
        prvTgfDevNum,
        SLAVE_PORT_INDEX,
        TGF_CAPTURE_MODE_MIRRORING_E,
        200     /*captureOnTime*/);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTransmitPacketsWithCapture");

    buffLen  = sizeof(packetBuff);
    /* get entry from rxNetworkIf table */
    rc = tgfTrafficGeneratorRxInCpuGet(
        TGF_PACKET_TYPE_CAPTURE_E,
        GT_TRUE/*getFirst*/, GT_TRUE /*trace*/, packetBuff,
        &buffLen, &packetActualLength,
        &receiveDevNum, &queue, &rxParam);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorRxInCpuGet");

    /* check that timestamp overlapped the correctionField in PTP header */
    tsOffset =
        TGF_L2_HEADER_SIZE_CNS
        + TGF_VLAN_TAG_SIZE_CNS
        + TGF_ETHERTYPE_SIZE_CNS
        + 8 /* correctionField offset in PTP header */;

    memCmpRc = cpssOsMemCmp(&(packetBuff[tsOffset]), prvTgfPacketSyncPtpV2Part.correctionField, 8);
    UTF_VERIFY_NOT_EQUAL0_STRING_MAC(
        0, memCmpRc, "correctionField updated check");

    /* check that timestamp doesn't overlapped the requestReceiptTimestamp/originTimestamp in PTP header */
    tsOffset =
        TGF_L2_HEADER_SIZE_CNS
        + TGF_VLAN_TAG_SIZE_CNS
        + TGF_ETHERTYPE_SIZE_CNS
        + 34 /* requestReceiptTimestamp/originTimestamp offset in PTP header */;

    memCmpRc = cpssOsMemCmp(&(packetBuff[tsOffset]), prvTgfPacketPayloadPart.dataPtr, 10);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        0, memCmpRc, "timestamp updated check");

#ifdef ASIC_SIMULATION
    /* state to simulation that this egress port is not connected to PHY */
    wmSetPortConnectedToPhyEgress(MASTER_PORT_INDEX,GT_FALSE);

    /* state to simulation that phy mode is 1-step */
    wmSetPhyIn2StepMode(GT_FALSE);
#endif

}

/**
* @internal prvTgfPtpManagerUsxgmiiPchEgress1StepInSwitch2StepInPhyTcModeCfgSet function
* @endinternal
*
* @brief   None
*/
GT_STATUS prvTgfPtpManagerUsxgmiiPchEgress1StepInSwitch2StepInPhyTcModeCfgSet
(
    GT_VOID
)
{
    GT_STATUS                               rc = GT_OK;
    CPSS_DXCH_PTP_MANAGER_DOMAIN_STC        domainConf;
    CPSS_DXCH_PTP_EGRESS_DOMAIN_ENTRY_STC   domainEntry;

    domainConf.domainIndex = 2; /* the local domain index to which the domainID is directed */
    domainConf.domainMode = CPSS_DXCH_PTP_INGRESS_DOMAIN_MODE_PTP_V2_E;
    domainConf.v2DomainId = prvTgfPacketSyncPtpV2Part.domainNumber; /* domainID is the domain number which is passed in the PTP header */

    /* Configure the global settings for PTP Domain */
    rc = cpssDxChPtpManagerGlobalPtpDomainSet(prvTgfDevNum,&domainConf);
    PRV_UTF_VERIFY_GT_OK(rc, "cpssDxChPtpManagerGlobalPtpDomainSet");

    /* Allocate Port Interface to TAI */
    rc = cpssDxChPtpManagerPortAllocateTai(prvTgfDevNum,MASTER_PORT_INDEX,CPSS_DXCH_PTP_TAI_NUMBER_0_E,CPSS_DXCH_PTP_TS_DELAY_TAI_SEL_MODE_EGRESS_E);
    PRV_UTF_VERIFY_GT_OK(rc, "cpssDxChPtpManagerGlobalPtpDomainSet");

    /* Assign packet command per{ingress port, domain index, message type}. */
    rc = cpssDxChPtpManagerPortCommandAssignment(prvTgfDevNum,MASTER_PORT_INDEX,2,CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_SYNC_E,CPSS_PACKET_CMD_FORWARD_E);
    PRV_UTF_VERIFY_GT_OK(rc, "cpssDxChPtpManagerGlobalPtpDomainSet");

    cpssOsMemSet(&domainEntry, 0, sizeof(domainEntry));
    domainEntry.ptpOverEhernetTsEnable = GT_TRUE;
    domainEntry.ptpOverUdpIpv4TsEnable = GT_FALSE;
    domainEntry.ptpOverUdpIpv6TsEnable = GT_FALSE;
    domainEntry.messageTypeTsEnableBmp =
        (1 << prvTgfPacketSyncPtpV2Part.messageType);
    domainEntry.transportSpecificTsEnableBmp =
        (1 << prvTgfPacketSyncPtpV2Part.transportSpecific);

    /* Configure action to be performed on PTP packets per{egress port, domain index, message type}. */
    rc = cpssDxChPtpManagerPortPacketActionsConfig(prvTgfDevNum,SLAVE_PORT_INDEX,CPSS_DXCH_PTP_TAI_NUMBER_0_E,2,&domainEntry,
                                                   CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_SYNC_E,CPSS_DXCH_PTP_TS_ACTION_CAPTURE_E);
    PRV_UTF_VERIFY_GT_OK(rc, "cpssDxChPtpManagerPortPacketActionsConfig");

    /* Configure action to be performed on PTP packets per{egress port, domain index, message type}. */
    rc = cpssDxChPtpManagerPortPacketActionsConfig(prvTgfDevNum,MASTER_PORT_INDEX,CPSS_DXCH_PTP_TAI_NUMBER_0_E,2,&domainEntry,
                                                   CPSS_DXCH_PTP_MANAGER_MESSAGE_TYPE_SYNC_E,CPSS_DXCH_PTP_TS_ACTION_CAPTURE_E);
    PRV_UTF_VERIFY_GT_OK(rc, "cpssDxChPtpManagerPortPacketActionsConfig");

    /* Configure Enhanced Timestamping Mode per Egress port */
    rc = cpssDxChPtpManagerPortTimestampingModeConfig(prvTgfDevNum,MASTER_PORT_INDEX,CPSS_DXCH_PTP_ENHANCED_TIMESTAMPING_MODE_USGMII_E,CPSS_DXCH_PTP_MANAGER_OPERATION_MODE_TWO_STEP_PHY_STANDARD_PCH_FORMAT_E);
    PRV_UTF_VERIFY_GT_OK(rc, "cpssDxChPtpManagerPortTimestampingModeConfig");

    /* Set MCH signature configuration per port */
    rc = cpssDxChPtpManagerPortMacTsQueueSigatureSet(prvTgfDevNum,MASTER_PORT_INDEX,CPSS_DXCH_PTP_MANAGER_SIGNATURE_MODE_QUEUE_ENTRY_ID_E,0);
    PRV_UTF_VERIFY_GT_OK(rc, "cpssDxChPtpManagerPortMacTsQueueSigatureSet");

#ifdef ASIC_SIMULATION
    /* state to simulation that this egress port is connected to PHY */
    wmSetPortConnectedToPhyEgress(MASTER_PORT_INDEX,GT_TRUE);

    /* state to simulation that phy mode is 2-step */
    wmSetPhyIn2StepMode(GT_TRUE);
#endif

    return GT_OK;
}

/**
* @internal prvTgfPtpManagerUsxgmiiPchEgress1StepInSwitch2StepInPhyTcModeTrafficGenerateAndCheck function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port given packet uner capture:
*         Check captured packet.
*/
GT_VOID prvTgfPtpManagerUsxgmiiPchEgress1StepInSwitch2StepInPhyTcModeTrafficGenerateAndCheck
(
    GT_VOID
)
{
    GT_STATUS       rc;
    GT_U32          packetActualLength = 0;
    GT_U8           packetBuff[256] = {0};
    GT_U32          buffLen;
    TGF_NET_DSA_STC rxParam;
    GT_U8           queue    = 0;
    GT_U32          tsOffset;
    GT_32           memCmpRc;
    GT_U8           receiveDevNum;
    CPSS_DXCH_PTP_TS_EGRESS_TIMESTAMP_QUEUE_ENTRY_STC tsQueueEntry;
    CPSS_DXCH_PTP_MANAGER_TSU_TX_TIMESTAMP_QUEUE_ENTRY_STC tsPortQueueEntry;

    cpssOsMemSet(&rxParam, 0, sizeof(rxParam));

    PRV_UTF_LOG0_MAC("\nSYNC MESSAGE MASTER->SLAVE: CTSU + PHY - time of transmission is inserted to TS Queues.\n\n");
    /* Traffic with capture */
    rc = prvTgfTransmitPacketsWithCapture(
        prvTgfDevNum,
        MASTER_PORT_INDEX,
        &prvTgfSyncPtpV2PacketInfo,
        1      /*burstCount*/,
        0      /*numVfd*/,
        NULL   /*vfdArray*/,
        prvTgfDevNum,
        SLAVE_PORT_INDEX,
        TGF_CAPTURE_MODE_MIRRORING_E,
        200     /*captureOnTime*/);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfTransmitPacketsWithCapture");

    buffLen  = sizeof(packetBuff);
    /* get entry from rxNetworkIf table */
    rc = tgfTrafficGeneratorRxInCpuGet(
        TGF_PACKET_TYPE_CAPTURE_E,
        GT_TRUE/*getFirst*/, GT_TRUE /*trace*/, packetBuff,
        &buffLen, &packetActualLength,
        &receiveDevNum, &queue, &rxParam);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "tgfTrafficGeneratorRxInCpuGet");

    /* check that timestamp doesn't overlapped the requestReceiptTimestamp/originTimestamp in PTP header */
    tsOffset =
        TGF_L2_HEADER_SIZE_CNS
        + TGF_VLAN_TAG_SIZE_CNS
        + TGF_ETHERTYPE_SIZE_CNS
        + 34 /* requestReceiptTimestamp/originTimestamp offset in PTP header */;

    memCmpRc = cpssOsMemCmp(&(packetBuff[tsOffset]), prvTgfPacketPayloadPart.dataPtr, 10);
    UTF_VERIFY_EQUAL0_STRING_MAC(
        0, memCmpRc, "timestamp updated check");

    /* read ERMRK Egress queue and check it has a valid entry */
    rc = cpssDxChPtpManagerEgressTimestampGlobalQueueEntryRead(prvTgfDevNum,0 /*queueNum*/, &tsQueueEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChPtpManagerEgressTimestampGlobalQueueEntryRead");

    UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, tsQueueEntry.entryValid, "entryValid");

    PRV_UTF_LOG3_MAC("[TGF]: cpssDxChPtpManagerEgressTimestampGlobalQueueEntryRead: valid entry [%d] queueEntryId [%d] portNum [%d]\n",tsQueueEntry.entryValid, tsQueueEntry.queueEntryId, tsQueueEntry.portNum);

    UTF_VERIFY_EQUAL0_STRING_MAC(PRV_TGF_PTP_TS_PACKET_TYPE_PTP_V2_E  , tsQueueEntry.packetFormat, "packetFormat");

    UTF_VERIFY_EQUAL0_STRING_MAC(prvTgfPacketSyncPtpV2Part.sequenceId  , tsQueueEntry.sequenceId, "sequenceId");

    /* read CTSU queue per port and check it has a valid entry */
    rc = cpssDxChPtpManagerEgressTimestampPortQueueEntryRead(prvTgfDevNum,tsQueueEntry.portNum,0 /*queueNum*/,&tsPortQueueEntry);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "cpssDxChPtpManagerEgressTimestampPortQueueEntryRead");

    UTF_VERIFY_EQUAL0_STRING_MAC(GT_TRUE, tsPortQueueEntry.entryValid, "entryValid");

    PRV_UTF_LOG3_MAC("[TGF]: cpssDxChPtpManagerEgressTimestampPortQueueEntryRead: valid entry [%d] queueEntryId [%d] portNum [%d]\n",tsPortQueueEntry.entryValid, tsPortQueueEntry.queueEntryId, tsQueueEntry.portNum);

#ifdef ASIC_SIMULATION
    /* state to simulation that this egress port is not connected to PHY */
    wmSetPortConnectedToPhyEgress(MASTER_PORT_INDEX,GT_FALSE);

    /* state to simulation that phy mode is 1-step */
    wmSetPhyIn2StepMode(GT_FALSE);
#endif

}

