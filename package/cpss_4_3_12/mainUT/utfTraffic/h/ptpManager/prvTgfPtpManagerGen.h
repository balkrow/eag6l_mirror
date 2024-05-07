/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
* prvTgfPtpManagerGen.h
*
* DESCRIPTION:
*       Precision Time Protocol (PTP) Manager
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/
#ifndef __prvTgfPtpManagerGen_h
#define __prvTgfPtpManagerGen_h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* include the PTP manager types */
#include <cpss/dxCh/dxChxGen/ptpManager/cpssDxChPtpManagerTypes.h>
#include <common/tgfPtpGen.h>

#define MASTER_PORT_INDEX prvTgfPortsArray[PRV_TGF_PTP_GEN_SEND_PORT_INDEX_CNS]
#define SLAVE_PORT_INDEX  prvTgfPortsArray[PRV_TGF_PTP_GEN_RECEIVE_PORT_INDEX_CNS]

/********************************************************************************/
/*                                     UTILS                                    */
/********************************************************************************/

/********************************************************************************/
/*                                Test Functions                                */
/********************************************************************************/

/**
* @internal prvTgfPtpManagerGenPtpOverEthEnablePtpInit function
* @endinternal
*
* @brief   None
*/
GT_STATUS prvTgfPtpManagerGenPtpOverEthEnablePtpInit
(
    GT_VOID
);

/**
* @internal prvTgfPtpManagerGenPtpRestore function
* @endinternal
*
* @brief   None
*/
GT_STATUS prvTgfPtpManagerGenPtpRestore
(
    GT_U32 portNum
);

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
);

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
);

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
);

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
);

/**
* @internal prvTgfPtpGenIngressCaptureQueue1Check function
* @endinternal
*
* @brief   None
*/
GT_VOID prvTgfPtpManagerGenIngressCaptureQueue1Check
(
    IN  GT_U32 expected_valid
);

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
);

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
);

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
);

/**
* @internal prvTgfPtpManagerGenPtpDefaultInit function
* @endinternal
*
* @brief   None
*/
GT_STATUS prvTgfPtpManagerGenPtpDefaultInit
(
    GT_VOID
);

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
);

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
);

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
);

/**
* @internal prvTgfPtpManagerEgressExceptionTestRestore function
* @endinternal
*
* @brief  Restore the Configuration
*/
GT_VOID prvTgfPtpManagerEgressExceptionTestRestore
(
    GT_VOID
);

/**
* @internal prvTgfPtpManagerGenPtpOverUdpEnablePtpInit function
* @endinternal
*
* @brief   None
*/
GT_STATUS prvTgfPtpManagerGenPtpOverUdpEnablePtpInit
(
    PRV_TGF_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_ENT checksumMode
);

/**
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
);

/**
* @internal prvTgfPtpManagerInvalidPtpCfgRestore function
* @endinternal
*
* @brief  Restore the Configuration
*/
GT_VOID prvTgfPtpManagerInvalidPtpCfgRestore
(
    GT_VOID
);

/**
* @internal prvTgfPtpManagerPtpOverIpUDPCfgSet function
* @endinternal
*
* @brief   None
*/
GT_VOID prvTgfPtpManagerPtpOverIpUDPCfgSet
(
    PRV_TGF_PTP_TS_ACTION_ENT ptpAction
);

/**
* @internal
*           prvTgfPtpManagerPtpOverIpUDPTrafficGenerateAndCheck
*           function
* @endinternal
*
* @brief   Generate traffic:
*         Send to device's port given packet uner capture:
*         Check captured packet.
*/
GT_VOID prvTgfPtpManagerPtpOverIpUDPTrafficGenerateAndCheck
(
    PRV_TGF_PTP_TS_UDP_CHECKSUM_UPDATE_MODE_ENT checksumMode,
    PRV_TGF_PTP_TS_ACTION_ENT                   ptpAction,
    GT_BOOL                                     longPacket,
    GT_BOOL                                     isIpv6
);

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
);

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
);

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
);

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
);

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
);

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
);

/**
* @internal prvTgfPtpManagerTC1StepE2ECfgSet function
* @endinternal
*
* @brief   None
*/
GT_STATUS prvTgfPtpManagerTC1StepE2ECfgSet
(
    IN GT_U32 part
);

/**
* @internal prvTgfPtpManagerTC1StepP2PCfgSet function
* @endinternal
*
* @brief   None
*/
GT_STATUS prvTgfPtpManagerTC1StepP2PCfgSet
(
    GT_VOID
);

/**
* @internal prvTgfPtpManager1StepFollowUpInformationTlvCfgSet function
* @endinternal
*
* @brief   None
*/
GT_STATUS prvTgfPtpManager1StepFollowUpInformationTlvCfgSet
(
    GT_VOID
);

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
);

/**
* @internal prvTgfPtpManagerUsxgmiiMchEgress1StepInSwitch1StepInPhyCfgSet function
* @endinternal
*
* @brief   None
*/
GT_STATUS prvTgfPtpManagerUsxgmiiMchEgress1StepInSwitch1StepInPhyCfgSet
(
    GT_VOID
);

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
);

/**
* @internal prvTgfPtpManagerUsxgmiiMchEgress2StepInSwitch1StepInPhyCfgSet function
* @endinternal
*
* @brief   None
*/
GT_STATUS prvTgfPtpManagerUsxgmiiMchEgress2StepInSwitch1StepInPhyCfgSet
(
    GT_VOID
);

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
);

/**
* @internal prvTgfPtpManagerUsxgmiiMchEgress1StepInSwitch1StepInPhyTcModeCfgSet function
* @endinternal
*
* @brief   None
*/
GT_STATUS prvTgfPtpManagerUsxgmiiMchEgress1StepInSwitch1StepInPhyTcModeCfgSet
(
    GT_VOID
);

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
);

/**
* @internal prvTgfPtpManagerUsxgmiiPchEgress1StepInSwitch2StepInPhyTcModeCfgSet function
* @endinternal
*
* @brief   None
*/
GT_STATUS prvTgfPtpManagerUsxgmiiPchEgress1StepInSwitch2StepInPhyTcModeCfgSet
(
    GT_VOID
);

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
);

/**
* @internal prvTgfPtpManagerBC2StepE2ECfgSet function
* @endinternal
*
* @brief   None
*/
GT_STATUS prvTgfPtpManagerBC2StepE2ECfgSet
(
    IN GT_U32 part
);

/**
* @internal prvTgfPtpManagerBC1StepE2ECfgSet function
* @endinternal
*
* @brief   None
*/
GT_STATUS prvTgfPtpManagerBC1StepE2ECfgSet
(
    IN GT_U32 part
);

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
);

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
);

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
);

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
);

/**
* @internal prvTgfPtpManagerGenVidAndFdbDefCfgSet function
* @endinternal
*
* @brief   None
*/
GT_STATUS prvTgfPtpManagerGenVidAndFdbDefCfgSet
(
    GT_VOID
);

/**
* @internal prvTgfPtpManagerTimestampTaggingRestorePrevTests function
* @endinternal
*
* @brief   restore configuration from prev tests
*/
GT_VOID prvTgfPtpManagerTimestampTaggingRestorePrevTests
(
    GT_VOID
);

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
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvTgfPtpManagerGen_h */


