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
* @file prvTgfStreamSgcMaxSduSizeProfileCheck.c
*
* @brief Stream Gate Control maximum SDU size profile check use cases testing.
*
*        Feature description:
*        - The PCL actions of <Max SDU Size Profile> are used as an indexes to 4 entries Max SDU Size Profile Table.
*        - If the Desc<Byte Count> > SDU Size Profile byte count, the configurable Max SDU Size Exception Command is applied.
*        - We use new action field <CNC Index Mode> to count the packets
*          - for Action <CNC Index Mode> == 1 the CNC index that is passed to the CNC unit is with the following format:
*            CNC index = {Action<CNC Index>, Max SDU Size Pass/Fail (1b)}
*
*        2 tests are added to check above feature for Ingress and for Egress:
*        - prvTgfStreamSgcMaxSduSizeProfileCheckIngress
*        - prvTgfStreamSgcMaxSduSizeProfileCheckEgress
*
*        Ports allocation:
*         - Port#0: Ingress port
*         - Port#4: Target port
*
*        Packet flow:
*         1.  Ethernet frame packet enters the device via Ingress Port#0
*         2.  The packet is sent towards target port#4
*
*          Test description:
*          - Configure entries 1 (for ingress usage) and 2 (for egress usage) in max SDU size profile table to 64 bytes.
*            Rest entries are cleared to '0'
*          - Set IPCL/EPCL actions with Max SDU Size Profile indexes 1 for Ingress and 2 for Egress
*          - Configure max SDU size exception parameters (packet command & drop code) in case exception occurs
*            use different drop codes for Ingress and Egress
*          - Send 2 packets:
*             - first packet with size less than SDU size entry (64 bytes)
*             - second packet with size bigger than SDU size entry (94 bytes)
*          - Expect first packet to be valid and get to target port (since packet size is less than max SDU size that was set in test)
*            - check CNC counter (pcl.counterIndex << 1)| 0 (pass)
*          - Expect second packet to be invalid and to be dropped (since packet size exceeds max SDU size that was set in test)
*            - check CNC counter (pcl.counterIndex << 1)| 1 (fail)
* @version   1
********************************************************************************
*/

#include <stream/prvTgfStreamSgcMaxSduSizeProfileCheck.h>
#include <oam/prvTgfOamGeneral.h>
#include <cpss/dxCh/dxChxGen/oam/cpssDxChOam.h>
#include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIf.h>
#include <common/tgfConfigGen.h>
#include <common/tgfCncGen.h>


/*************************** Constants definitions ***********************************/

/* Size of maximum output packets in bytes include CRC */
#define PRV_TGF_MAX_PACKET_SIZE_CNS              98

/* Number of packets */
#define PRV_TGF_BURST_COUNT_CNS                  2

/* Ports index allocation */
#define PRV_TGF_INGRESS_PORT_IDX_CNS             0
#define PRV_TGF_TARGET_PORT_IDX_CNS              1

/* Default VLAN Id */
#define PRV_TGF_DEFAULT_VLANID_CNS               1

/* CNC counter number used for Ingress test */
#define PRV_TGF_INGRESS_CNC_GEN_COUNTER_NUM_CNS  33

/* CNC counter number used for Egress test */
#define PRV_TGF_EGRESS_CNC_GEN_COUNTER_NUM_CNS   55

/* CNC block number used in test */
#define PRV_TGF_CNC_GEN_BLOCK_NUM_CNS            1


/*************************** static DB definitions ***********************************/

/* CNC counter index (uses different numbers for Ingress and Egress) */
static GT_U32  matchCounterIndex = 0;

/* Input packet information */
static TGF_PACKET_STC   packetInfo;

/* Target port interface information */
static CPSS_INTERFACE_INFO_STC  targetPortInterface;

/* Actual port counters values per packet */
static GT_U32    actualPacketsNum[PRV_TGF_BURST_COUNT_CNS];
static GT_U32    actualBytesNum[PRV_TGF_BURST_COUNT_CNS];

/* Expected bytes number per packet */
static GT_U32    expectedBytesNum[PRV_TGF_BURST_COUNT_CNS];

/* Holds maximum SDU size parameters */
static GT_U32    maxSduSizeProfileIndx;
static GT_U32    maxSduSizeArr[4]= {0,64,64,0};

/* PCL rule id */
static GT_U32    ruleId = 1;

/* Drop code to be used in test in case of exception (use different values for Ingress and Egress) */
static CPSS_NET_RX_CPU_CODE_ENT    dropCode;

/* Ingress Packet: L2 part */
static TGF_PACKET_L2_STC prvTgfPacketL2Part = {
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x02},  /* daMac */
    {0x00, 0x00, 0x00, 0x00, 0x34, 0x03}   /* saMac */
};

/* Short DATA of packet */
static GT_U8 prvTgfShortPayloadDataArr[] = {
    0x66, 0x66, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05,
    0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d,
    0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15,
    0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d,
    0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25,
    0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d,
    0x2e, 0x2f, 0x30, 0x31 /* length 52 bytes */
};

/* Short PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfShortPacketPayloadPart = {
    sizeof(prvTgfShortPayloadDataArr),    /* dataLength */
    prvTgfShortPayloadDataArr             /* dataPtr */
};

/* Short PARTS of packet */
static TGF_PACKET_PART_STC prvTgfShortPacketPartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfShortPacketPayloadPart}
};

/* Long DATA of packet */
static GT_U8 prvTgfLongPayloadDataArr[] = {
    0x66, 0x66, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05,
    0x06, 0x07, 0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d,
    0x0e, 0x0f, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15,
    0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d,
    0x1e, 0x1f, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25,
    0x26, 0x27, 0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d,
    0x2e, 0x2f, 0x30, 0x31, 0x32, 0x33, 0x34, 0x35,
    0x36, 0x37, 0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d,
    0x3e, 0x3f, 0x40, 0x41, 0x42, 0x43, 0x44, 0x45,
    0x46, 0x47, 0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d,
    0x4e, 0x4f    /* length 82 bytes */
};

/* Long PAYLOAD part */
static TGF_PACKET_PAYLOAD_STC prvTgfLongPacketPayloadPart = {
    sizeof(prvTgfLongPayloadDataArr),     /* dataLength */
    prvTgfLongPayloadDataArr              /* dataPtr */
};

/* Long PARTS of packet */
static TGF_PACKET_PART_STC prvTgfLongPacketPartArray[] = {
    {TGF_PACKET_PART_L2_E,        &prvTgfPacketL2Part},
    {TGF_PACKET_PART_PAYLOAD_E,   &prvTgfLongPacketPayloadPart}
};

/* Parameters that are needed to be restored */
static struct
{
    GT_U32                                   maxSduSizeArr[4];
    CPSS_PACKET_CMD_ENT                      pktCmd;
    CPSS_NET_RX_CPU_CODE_ENT                 dropCode;
    CPSS_DXCH_PCL_GENERIC_ACTION_MODE_ENT    genericActionMode;
} prvTgfRestoreCfg;


/******************************************************************************\
 *   Configurations functions                                                 *
\******************************************************************************/

/**
* @internal prvTgfStreamSgcBridgeConfigSet function
* @endinternal
*
* @brief   Bridge test configurations
*          Forward to ePort#1 according to MAC destination
*/
static GT_VOID prvTgfStreamSgcBridgeConfigSet
(
    GT_VOID
)
{
    GT_STATUS   rc = GT_OK;

    /* AUTODOC: add FDB entry with MAC 00:00:00:00:34:02, VLAN ID 1, target port */
    rc = prvTgfBrgDefFdbMacEntryOnPortSet(prvTgfPacketL2Part.daMac,
                                          PRV_TGF_DEFAULT_VLANID_CNS,
                                          prvTgfDevNum,
                                          prvTgfPortsArray[PRV_TGF_TARGET_PORT_IDX_CNS],
                                          GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfBrgDefFdbMacEntryOnPortSet");
}

/**
* @internal prvTgfStreamSgcPclConfigSet function
* @endinternal
*
* @brief   PCL test configurations
*          - Set PCL rule: according to MAC DA
*          - Set PCL actions with 802.1Qci parameters
*            - For Ingress
*              - Set redirect command to work with Generic action of 802.1Qci
*              - Set 12 bits generic action data with | cncIndxMode(1b) | maxSduSizeProfileIndx(2b) | gateId(9b) |
*            - For Egress
*              - enableEgressMaxSduSizeCheck
*              - egressMaxSduSizeProfile
*              - egressCncIndexMode
*          - Configure max SDU size profiles table
*/
static GT_VOID prvTgfStreamSgcPclConfigSet
(
    GT_U32 direction
)
{
    GT_STATUS                                rc = GT_OK;
    PRV_TGF_PCL_RULE_FORMAT_UNT              mask;
    PRV_TGF_PCL_RULE_FORMAT_UNT              pattern;
    PRV_TGF_PCL_ACTION_STC                   action;
    GT_U32                                   gateId;
    GT_BIT                                   cncIndxMode;
    GT_U32                                   ii;
    GT_U32                                   port;
    GT_U32                                   pclQciActions;
    CPSS_DXCH_PCL_GENERIC_ACTION_MODE_ENT    genericActionMode;
    PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT         nonIpKey;
    PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT         ipv4Key;
    PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT         ipv6Key;
    PRV_TGF_PCL_RULE_FORMAT_TYPE_ENT         ruleFormat;

    if (direction == CPSS_PCL_DIRECTION_INGRESS_E)
    { /* Ingress */

        /* AUTODOC: Update Ingress PCL parameters */
        port  = prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS];
        nonIpKey = PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;
        ipv4Key  = PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E;
        ipv6Key  = PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_IP_L2_QOS_E;
    }
    else
    { /* Egress */

        /* AUTODOC: Update Egress PCL parameters */
        port  = prvTgfPortsArray[PRV_TGF_TARGET_PORT_IDX_CNS];
        nonIpKey = PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E;
        ipv4Key  = PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E;
        ipv6Key  = PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_IP_L2_QOS_E;
    }

    /* AUTODOC: Init PCL Engine for ingress port */
    rc = prvTgfPclDefPortInit(port,
                              direction,
                              CPSS_PCL_LOOKUP_NUMBER_0_E,
                              nonIpKey,
                              ipv4Key,
                              ipv6Key);
    UTF_VERIFY_EQUAL3_STRING_MAC(GT_OK, rc, "prvTgfPclDefPortInit: %d, %d, %d", prvTgfDevNum, port, direction);

    /* AUTODOC: Set PCL rule: according to MAC DA */

    /* AUTODOC: Mask for MAC address */
    cpssOsMemSet(&mask, 0, sizeof(mask));
    cpssOsMemSet(mask.ruleStdNotIp.macDa.arEther, 0xFFFF, sizeof(mask.ruleStdNotIp.macDa.arEther));

    /* AUTODOC: Pattern for MAC DA address */
    cpssOsMemSet(&pattern, 0, sizeof(pattern));
    cpssOsMemCpy(pattern.ruleStdNotIp.macDa.arEther, prvTgfPacketL2Part.daMac, sizeof(prvTgfPacketL2Part.daMac));

    /* AUTODOC: Clear action structure */
    cpssOsMemSet(&action, 0, sizeof(action));

    /* AUTODOC: Set PCL action packet command */
    action.pktCmd = CPSS_PACKET_CMD_FORWARD_E;

    if (direction == CPSS_PCL_DIRECTION_INGRESS_E)
    { /* Ingress */

        /* AUTODOC: Set specific Ingress PCL actions:
           - set redirectCmd to Generic action
           - set Generic action with 802.1Qci parameters:
             - cncIndxMode,
             - maxSduSizeProfileIndx,
             - gateId
         */
        /* AUTODOC: Set IPCL action redirect command to generic action type */
        action.redirect.redirectCmd = PRV_TGF_PCL_ACTION_REDIRECT_CMD_GENERIC_ACTION_E;

        /* AUTODOC: Set 802.1QCi IPCL action parameters */
        gateId                = 1;
        maxSduSizeProfileIndx = 1;
        cncIndxMode           = 1;
        /* AUTODOC: Set generic action data with 12 bits of | cncIndxMode(1b) | maxSduSizeProfileIndx(2b) | gateId(9b) | */
        pclQciActions = (cncIndxMode << 11) | (maxSduSizeProfileIndx << 9) | gateId ;
        action.redirect.data.genericActionInfo.genericAction = pclQciActions ;

        /* AUTODOC: Save generic action mode for restore */
        rc = cpssDxChPclGenericActionModeGet(
            prvTgfDevNum, CPSS_PCL_DIRECTION_INGRESS_E,
            CPSS_PCL_LOOKUP_NUMBER_0_E, 0, &prvTgfRestoreCfg.genericActionMode);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of cpssDxChPclGenericActionModeGet");

        /* AUTODOC: Set generic action to 802.1Qci mode */
        genericActionMode = CPSS_DXCH_PCL_GENERIC_ACTION_MODE_802_1_QCI_E;
        rc = cpssDxChPclGenericActionModeSet(
            prvTgfDevNum, CPSS_PCL_DIRECTION_INGRESS_E,
            CPSS_PCL_LOOKUP_NUMBER_0_E, 0, genericActionMode);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of cpssDxChPclGenericActionModeSet");

        /* AUTODOC: Set CNC counter index for Ingress  */
        matchCounterIndex = PRV_TGF_INGRESS_CNC_GEN_COUNTER_NUM_CNS;

        /* AUTODOC: Set drop code for Ingress */
        dropCode = CPSS_NET_USER_DEFINED_2_E;

        /* AUTODOC: Set Ingress PCL rule format */
        ruleFormat = PRV_TGF_PCL_RULE_FORMAT_INGRESS_STD_NOT_IP_E;
    }
    else
    { /* Egress */

        /* AUTODOC: Set specific Egress PCL actions:
           - set egressPolicy to true
           - set 802.1Qci parameters:
             - egressCncIndexMode,
             - egressMaxSduSizeProfile,
             - enableEgressMaxSduSizeCheck
         */
        action.egressPolicy                = GT_TRUE;
        action.enableEgressMaxSduSizeCheck = GT_TRUE;
        maxSduSizeProfileIndx              = 2;
        action.egressMaxSduSizeProfile     = maxSduSizeProfileIndx;
        action.egressCncIndexMode          = 1;

        /* AUTODOC: Set CNC counter index for Egress  */
        matchCounterIndex = PRV_TGF_EGRESS_CNC_GEN_COUNTER_NUM_CNS;

        /* AUTODOC: Set drop code for Egress */
        dropCode = CPSS_NET_USER_DEFINED_3_E;

        /* AUTODOC: Set Egress PCL rule format */
        ruleFormat = PRV_TGF_PCL_RULE_FORMAT_EGRESS_STD_NOT_IP_E;
    }

    /* UTODOC: Enable CNC counter PCL action */
    action.matchCounter.enableMatchCount = GT_TRUE;

    /* AUTODOC: Set CNC counter index */
    action.matchCounter.matchCounterIndex = matchCounterIndex;

    /* AUTODOC: Set PCL rule */
    rc = prvTgfPclRuleSet(ruleFormat,
                          ruleId,
                          &mask,
                          &pattern,
                          &action);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfStreamGateControlPclConfigSet");

    /* AUTODOC: Set 4 entry Max SDU Size Profile Table. Set maximum bytes size per profile. */
    for (ii=0;ii<4;ii++)
    {
        /* Save current values for restore */
        rc = cpssDxChPclMaxSduSizeProfileGet(prvTgfDevNum,
                                             direction,
                                             ii,
                                             &prvTgfRestoreCfg.maxSduSizeArr[ii]);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of cpssDxChPclMaxSduSizeProfileGet");

        /* Set new values */
        rc = cpssDxChPclMaxSduSizeProfileSet(prvTgfDevNum,
                                             direction,
                                             ii,
                                             maxSduSizeArr[ii]);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of cpssDxChPclMaxSduSizeProfileSet");
    }

    /* AUTODOC: Save packet command and CPU drop code for restore */
    rc = cpssDxChPclMaxSduSizeExceptionGet(prvTgfDevNum,
                                           direction,
                                           &prvTgfRestoreCfg.pktCmd,
                                           &prvTgfRestoreCfg.dropCode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of cpssDxChPclMaxSduSizeExceptionGet");

    /* AUTODOC: Set the packet command and CPU drop code for exception case in which
                packet byte count is bigger than max SDU Size profile byte count
     */
    rc = cpssDxChPclMaxSduSizeExceptionSet(prvTgfDevNum,
                                           direction,
                                           CPSS_PACKET_CMD_DROP_HARD_E,
                                           dropCode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of cpssDxChPclMaxSduSizeExceptionSet");

}

/**
* @internal prvTgfStreamSgcCncCounterConfigSet function
* @endinternal
*
* @brief   CNC counter configurations for PCL CNC client
*/
static GT_VOID prvTgfStreamSgcCncCounterConfigSet
(
    GT_U32 direction
)
{
    GT_STATUS                        rc = GT_OK;
    PRV_TGF_CNC_CONFIGURATION_STC    cncCfg;

    /* AUTODOC: Clear CNC structure */
    cpssOsMemSet(&cncCfg, 0, sizeof(cncCfg));

    if (direction == CPSS_PCL_DIRECTION_INGRESS_E)
    { /* Ingress */

        /* AUTODOC: Set CNC parameters for Ingress PCL client */
        cncCfg.countingUnit = PRV_TGF_CNC_COUNTING_ENABLE_UNIT_PCL_E;
        cncCfg.clientType   = PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_E;
    }
    else
    { /* Egress */

        /* AUTODOC: Set CNC parameters for Egress PCL client */
        cncCfg.countingUnit = PRV_TGF_CNC_COUNTING_ENABLE_UNIT_EGRESS_PCL_E;
        cncCfg.clientType   = PRV_TGF_CNC_CLIENT_EGRESS_PCL_E;
    }

    /* AUTODOC: Set CNC block number */
    cncCfg.blockNum      = PRV_TGF_CNC_GEN_BLOCK_NUM_CNS;

    /* AUTODOC: Set CNC counter index (same one we set in PCL action) */
    cncCfg.counterNum    = matchCounterIndex;

    /* AUTODOC: Set CNC counter format to mode 0 */
    cncCfg.counterFormat = PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E;

    /* AUTODOC: Enable CNC counter */
    cncCfg.configureCountingUnitEnable = GT_TRUE;
    cncCfg.countingUnitEnable          = GT_TRUE;
    cncCfg.configureClientEnable       = GT_TRUE;
    cncCfg.clientEnable                = GT_TRUE;

    /* AUTODOC: Set CNC counter index range */
    cncCfg.configureIndexRangeBitMap = GT_TRUE;
    cncCfg.indexRangeBitMap[0]       = 1; /* [1,2,3] - zeros */

    /* AUTODOC: Set CNC counter value to '0' */
    cncCfg.configureCounterValue = GT_TRUE;

    /* AUTODOC: Set CNC counter byte count mode */
    cncCfg.configureByteCountMode = GT_TRUE;
    cncCfg.byteCountMode = PRV_TGF_CNC_BYTE_COUNT_MODE_L2_E;

    rc = prvTgfCncGenConfigure(&cncCfg, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCncGenConfigure");
}

/**
* internal prvTgfStreamSgcMaxSduSizeProfileCheckConfigSet function
* @endinternal
*
* @brief   SGC maximum SDU size profile check use case configurations
*/
GT_VOID prvTgfStreamSgcMaxSduSizeProfileCheckConfigSet
(
    GT_U32 direction
)
{
    /* AUTODOC: Bridge Configuration */
    prvTgfStreamSgcBridgeConfigSet();

    /* AUTODOC: PCL Configuration */
    prvTgfStreamSgcPclConfigSet(direction);

    /* AUTODOC: CNC counter Configuration */
    prvTgfStreamSgcCncCounterConfigSet(direction);
}


/******************************************************************************\
 *   Verification functions                                                   *
\******************************************************************************/

/**
* internal prvTgfStreamSgcMaxSduSizeProfileCheckVerification function
* @endinternal
*
* @brief  Check CNC counters for the 2 packets that were sent
*         - expect first packet to reach target port
*         - expect second packet to be dropped
*/
GT_VOID prvTgfStreamSgcMaxSduSizeProfileCheckVerification
(
    GT_VOID
)
{
    GT_STATUS                rc = GT_OK;
    PRV_TGF_CNC_CHECK_STC    cncCheck;

    cpssOsMemSet(&cncCheck, 0, sizeof(cncCheck));

    /* AUTODOC: Set CNC parameters */
    cncCheck.blockNum            = PRV_TGF_CNC_GEN_BLOCK_NUM_CNS;
    cncCheck.counterFormat       = PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E;
    cncCheck.compareCounterValue = GT_TRUE;

    /********************************************************************************************
      Check first packet:
       - Packet size of first packet is less than configured in max SDU profile therefore
         packet is valid and should be passed to target port

       - CNC index mode was set to '1' in PCL action therefore CNC index that passed to CNC is:
         - action.CNC Index << 1 | Max SDU Size Pass/Fail(1b)
         - Max SDU Size Pass/Fail is set to '1' if Desc<Byte Count> > SDU Size Profile byte count
         - therefore in this case CNC counter: action.CNC Index << 1 | 0
     ********************************************************************************************/

    /* AUTODOC: CNC counter index for valid packet */
    cncCheck.counterNum = (matchCounterIndex<<1) | 0;

    /* AUTODOC: Expect 1 packet to pass to target port */
    cncCheck.counterValue.packetCount.l[0] = 1;
    /* AUTODOC: number of first send packet +4 CRC bytes */
    cncCheck.counterValue.byteCount.l[0] = expectedBytesNum[0] + 4;

    rc = prvTgfCncGenCheck(&cncCheck);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCncGenCheck");


    /********************************************************************************************
      Check second packet:
       - Packet size of second packet is bigger than configured in max SDU profile therefore
         packet is invalid and it is configured to be dropped

       - CNC index mode was set to '1' in PCL action therefore CNC index that passed to CNC is:
         - action.CNC Index << 1 | Max SDU Size Pass/Fail(1b)
         - Max SDU Size Pass/Fail is set to '1' if Desc<Byte Count> > SDU Size Profile byte count
         - therefore in this case CNC counter: action.CNC Index << 1 | 1
     ********************************************************************************************/

    /* CNC counter index for valid packet */
    cncCheck.counterNum = (matchCounterIndex<<1) | 1;

    /* AUTODOC: Expect 1 packet to be dropped */
    cncCheck.counterValue.packetCount.l[0] = 1;
    /* AUTODOC: number of second send packet +4 CRC bytes */
    cncCheck.counterValue.byteCount.l[0] = expectedBytesNum[1] + 4;

    rc = prvTgfCncGenCheck(&cncCheck);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "prvTgfCncGenCheck");
}


/******************************************************************************\
 *   Traffic functions                                                           *
\******************************************************************************/

/**
* @internal prvTgfStreamSgcMaxSduSizeProfileCheckTraffic function
* @endinternal
*
* @brief   Stream Gate Control maximum SDU size profile check use case traffic.
*
*          - Send 2 packets:
*            - first packet with size less than SDU size entry (64 bytes)
*            - second packet with size bigger than SDU size entry (94 bytes)
*          - Enable capture on target port
*          - Read counters from target port and save them to be verified
*/
GT_VOID prvTgfStreamSgcMaxSduSizeProfileCheckTraffic
(
    GT_VOID
)
{
    GT_STATUS                       rc = GT_OK;
    CPSS_PORT_MAC_COUNTER_SET_STC   portCntrs;
    GT_U32                          portIter;

    PRV_UTF_LOG0_MAC("======= Ports allocation =======\n");
    PRV_UTF_LOG1_MAC("Port [%d]: Ingress Port \n", prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS]);
    PRV_UTF_LOG1_MAC("Port [%d]: Target Port \n", prvTgfPortsArray[PRV_TGF_TARGET_PORT_IDX_CNS]);

    /* AUTODOC: reset counters for all 4 ports */
    for (portIter = 0; portIter < prvTgfPortsNum; portIter++)
    {
        rc = prvTgfResetCountersEth(prvTgfDevNum, prvTgfPortsArray[portIter]);
        UTF_VERIFY_EQUAL2_STRING_MAC(GT_OK, rc, "Failure in prvTgfResetCountersEth: %d, %d", prvTgfDevNum, prvTgfPortsArray[portIter]);
    }

    /* AUTODOC: Clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficTableRxPcktTblClear");

    /* AUTODOC: Enable capture on target port */
    targetPortInterface.type             = CPSS_INTERFACE_PORT_E;
    targetPortInterface.devPort.hwDevNum = prvTgfDevNum;
    targetPortInterface.devPort.portNum  = prvTgfPortsArray[PRV_TGF_TARGET_PORT_IDX_CNS];
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&targetPortInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet");


    PRV_UTF_LOG0_MAC("======= Send first packet (short) =======\n");

    /**** Packet size less/equal than max SDU size therefore should passed to target port ****/

    /* AUTODOC: Set input packet interface structure  */
    packetInfo.numOfParts =  sizeof(prvTgfShortPacketPartArray) / sizeof(prvTgfShortPacketPartArray[0]);
    packetInfo.partsArray = prvTgfShortPacketPartArray;
    packetInfo.totalLen   = TGF_PACKET_AUTO_CALC_LENGTH_CNS;
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &packetInfo, 1, 0, NULL);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfSetTxSetupEth");

    /* AUTODOC: Update packet size */
    expectedBytesNum[0] = sizeof(prvTgfPacketL2Part) + sizeof(prvTgfShortPayloadDataArr);

    /* AUTODOC: Send first packet  */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfStartTransmitingEth");

    /* AUTODOC: Read counters from target port and save them to be verified */
    rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_TARGET_PORT_IDX_CNS], GT_TRUE, &portCntrs);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfReadPortCountersEth");
    actualPacketsNum[0] =  portCntrs.goodPktsRcv.l[0];
    actualBytesNum[0] = portCntrs.goodOctetsRcv.l[0];


    PRV_UTF_LOG0_MAC("======= Send second packet (long) =======\n");

    /**** Packet size bigger than max SDU size therefore an exception should occur. Packet will not pass to target port. ****/

    /* AUTODOC: Set input packet interface structure  */
    packetInfo.numOfParts =  sizeof(prvTgfLongPacketPartArray) / sizeof(prvTgfLongPacketPartArray[0]);
    packetInfo.partsArray = prvTgfLongPacketPartArray;
    packetInfo.totalLen   = TGF_PACKET_AUTO_CALC_LENGTH_CNS;
    rc = prvTgfSetTxSetupEth(prvTgfDevNum, &packetInfo, 1, 0, NULL);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfSetTxSetupEth");

    /* AUTODOC: Update packet size */
    expectedBytesNum[1] = sizeof(prvTgfPacketL2Part) + sizeof(prvTgfLongPayloadDataArr);

    /* AUTODOC: Send second packet  */
    rc = prvTgfStartTransmitingEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_INGRESS_PORT_IDX_CNS]);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfStartTransmitingEth");

    /* Read counters from target port and save them to be verified */
    rc = prvTgfReadPortCountersEth(prvTgfDevNum, prvTgfPortsArray[PRV_TGF_TARGET_PORT_IDX_CNS], GT_TRUE, &portCntrs);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of prvTgfReadPortCountersEth");
    actualPacketsNum[1] =  portCntrs.goodPktsRcv.l[0];
    actualBytesNum[1] = portCntrs.goodOctetsRcv.l[0];

    /* AUTODOC: Disable capture on target port */
    rc = tgfTrafficGeneratorPortTxEthCaptureSet(&targetPortInterface, TGF_CAPTURE_MODE_MIRRORING_E, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of tgfTrafficGeneratorPortTxEthCaptureSet");
}


/******************************************************************************\
 *   Restore functions                                                        *
\******************************************************************************/

/**
* @internal prvTgfStreamSgcMaxSduSizeProfileCheckConfigRestore function
* @endinternal
*
* @brief   Stream Gate Control configurations restore.
* @note    1. Restore Bridge Configuration
*          2. Restore CNC counters Configuration
*          3. Restore PCL Configuration
*          4. Restore base Configuration
*/
GT_VOID prvTgfStreamSgcMaxSduSizeProfileCheckConfigRestore
(
    GT_U32 direction
)
{
    GT_STATUS                        rc = GT_OK;
    GT_U32                           ii;
    PRV_TGF_CNC_CONFIGURATION_STC    cncCfg;

    /* -------------------------------------------------------------------------
     * 1. Restore Bridge Configuration
     */
    /* AUTODOC: flush FDB include static entries */
    rc = prvTgfBrgFdbFlush(GT_TRUE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfBrgFdbFlush");

    /* -------------------------------------------------------------------------
     * 2. Restore CNC counters Configuration
     */
    /* AUTODOC: Clear CNC configuration structure */
    cpssOsMemSet(&cncCfg, 0, sizeof(cncCfg));

    if (direction == CPSS_PCL_DIRECTION_INGRESS_E)
    { /* Ingress */

        /* AUTODOC: Set Ingress PCL CNC client */
        cncCfg.countingUnit  = PRV_TGF_CNC_COUNTING_ENABLE_UNIT_PCL_E;
        cncCfg.clientType    = PRV_TGF_CNC_CLIENT_INGRESS_PCL_LOOKUP_0_E;
    }
    else
    { /* Egress */

        /* AUTODOC: Set Egress PCL CNC client */
        cncCfg.countingUnit  = PRV_TGF_CNC_COUNTING_ENABLE_UNIT_EGRESS_PCL_E;
        cncCfg.clientType    = PRV_TGF_CNC_CLIENT_EGRESS_PCL_E;
    }

    /* AUTODOC: Set same CNC block number that was set in test */
    cncCfg.blockNum = PRV_TGF_CNC_GEN_BLOCK_NUM_CNS;

    /* AUTODOC: Set CNC counter index (same one we set in PCL action) */
    cncCfg.counterNum = matchCounterIndex;

    /* AUTODOC: Set CNC counter format to mode 0 */
    cncCfg.counterFormat = PRV_TGF_CNC_COUNTER_FORMAT_MODE_0_E;

    /* AUTODOC: Disable CNC client */
    cncCfg.configureCountingUnitEnable = GT_TRUE;
    cncCfg.countingUnitEnable          = GT_FALSE;
    cncCfg.configureClientEnable       = GT_TRUE;
    cncCfg.clientEnable                = GT_FALSE;

    /* AUTODOC: Reset CNC counter index range */
    cncCfg.configureIndexRangeBitMap = GT_TRUE;

    /* AUTODOC: restore typical CNC configuration */
    prvTgfCncGenConfigure(&cncCfg, GT_FALSE);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfCncGenConfigure");

    /* -------------------------------------------------------------------------
     * 3. Restore PCL Configuration
     */
    /* AUTODOC: invalidate PCL rule #1 */
    rc = prvTgfPclRuleValidStatusSet(CPSS_PCL_RULE_SIZE_STD_E, ruleId, GT_FALSE);
    UTF_VERIFY_EQUAL1_STRING_MAC(GT_OK, rc, "Failure in prvTgfPclRuleValidStatusSet, RuleIndex = %d", 1);

    /* AUTODOC: disable all PCL configurations on the ports */
    prvTgfPclPortsRestoreAll();

    /* AUTODOC: set back max SDU values */
    for (ii=0;ii<4;ii++)
    {
        rc = cpssDxChPclMaxSduSizeProfileSet(prvTgfDevNum,
                                             direction,
                                             ii,
                                             prvTgfRestoreCfg.maxSduSizeArr[ii]);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of cpssDxChPclMaxSduSizeProfileSet");
    }

    /* AUTODOC: set back packet command and CPU drop code */
    rc = cpssDxChPclMaxSduSizeExceptionSet(prvTgfDevNum,
                                           direction,
                                           prvTgfRestoreCfg.pktCmd,
                                           prvTgfRestoreCfg.dropCode);
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of cpssDxChPclMaxSduSizeExceptionGet");

    if (direction == CPSS_PCL_DIRECTION_INGRESS_E)
    { /* Ingress */

        /* AUTODOC: Set back PCL Generic Action mode */
        rc = cpssDxChPclGenericActionModeSet(
            prvTgfDevNum, CPSS_PCL_DIRECTION_INGRESS_E,
            CPSS_PCL_LOOKUP_NUMBER_0_E, 0, prvTgfRestoreCfg.genericActionMode);
        UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "ERROR of cpssDxChPclGenericActionModeSet");
    }

    /* -------------------------------------------------------------------------
     * 4. Restore base Configuration
     */
    /* AUTODOC: clear capturing RxPcktTable */
    rc = tgfTrafficTableRxPcktTblClear();
    PRV_UTF_LOG_VERIFY_GT_OK(rc, "Failure in tgfTrafficTableRxPcktTblClear");

    /* AUTODOC: disable RX capture */
    rc = tgfTrafficTableRxStartCapture(GT_FALSE);
    PRV_UTF_LOG_VERIFY_GT_OK(rc, "Failure in tgfTrafficTableRxStartCapture");

    /* AUTODOC: reset hw counters */
    rc = prvTgfCommonAllBridgeCntrsReset();
    UTF_VERIFY_EQUAL0_STRING_MAC(GT_OK, rc, "Failure in prvTgfCommonAllCntrsReset");
}
