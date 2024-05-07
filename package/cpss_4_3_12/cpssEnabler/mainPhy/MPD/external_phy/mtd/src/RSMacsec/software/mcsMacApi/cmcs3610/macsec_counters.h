/*! 
 * @file macsec_counters.h
 */
// *-----------------------------------------------------------------------------
// * @brief MACsec Counter definitions. Include via macsec_api.h
// *
// * @par Copyright
// * Copyright 2018-2020 Rianta Solutions Inc. All Rights Reserved.
// *-----------------------------------------------------------------------------
// *
// * All information contained herein is, and remains the property of
// * Rianta Solutions Inc. and its suppliers, if any. The intellectual and
// * technical concepts contained herein are proprietary to Rianta
// * Solutions Inc. and its suppliers and may be covered by U.S. and
// * Foreign Patents, patents in process, and are protected by trade secret
// * or copyright law. Dissemination of this information or reproduction of
// * this material is strictly forbidden unless prior written permission is
// * obtained from Rianta Solutions Inc.
// *-----------------------------------------------------------------------------
// * $Date: 2022-11-17-16:28:56
// * $Rev: 1.1.0
// *
// *---------------------------------------------------------------------------
#ifndef __MACSEC_COUNTERS_H__
#define __MACSEC_COUNTERS_H__

#ifndef __MACSEC_API_H__
#error "Do not include the file directly. Use macsec_api.h"
#endif

/*!
 * @struct Ra01IfRsMcsCseRxMemIfinunctloctets_t
 * @sa MacsecGetRxSecyStatisticCounters
 * @ingroup counter_types
 * @var Ra01IfRsMcsCseRxMemIfinunctloctets_t::rxUnctlOctetCnt
   Ingress Total MSDU and MAC-DA/SA Octets that are permitted by the uncontrolled port policies of this SecY.
*/

/*!
 * @struct Ra01IfRsMcsCseRxMemIfinctloctets_t
 * @sa MacsecGetRxSecyStatisticCounters
 * @ingroup counter_types
 * @var Ra01IfRsMcsCseRxMemIfinctloctets_t::rxCtlOctetCnt
   Ingress Total MSDU and MAC-DA/SA Octets that are permitted by the controlled port policies of this SecY.
*/

/*!
 * @struct Ra01IfRsMcsCseRxMemIfinunctlucpkts_t
 * @sa MacsecGetRxSecyStatisticCounters
 * @ingroup counter_types
 * @var Ra01IfRsMcsCseRxMemIfinunctlucpkts_t::rxUnctlPktUcastCnt
   Ingress Unicast packet count value for uncontrolled port of this SecY.
*/

/*!
 * @struct Ra01IfRsMcsCseRxMemIfinunctlmcpkts_t
 * @sa MacsecGetRxSecyStatisticCounters
 * @ingroup counter_types
 * @var Ra01IfRsMcsCseRxMemIfinunctlmcpkts_t::rxUnctlPktMcastCnt
   Ingress Multicast packet count value for uncontrolled port of this SecY.
*/

/*!
 * @struct Ra01IfRsMcsCseRxMemIfinunctlbcpkts_t
 * @sa MacsecGetRxSecyStatisticCounters
 * @ingroup counter_types
 * @var Ra01IfRsMcsCseRxMemIfinunctlbcpkts_t::rxUnctlPktBcastCnt
   Ingress Broadcast packet count value for uncontrolled port of this SecY."
*/

/*!
 * @struct Ra01IfRsMcsCseRxMemIfinctlucpkts_t
 * @sa MacsecGetRxSecyStatisticCounters
 * @ingroup counter_types
 * @var Ra01IfRsMcsCseRxMemIfinctlucpkts_t::rxCtlPktUcastCnt
   Ingress Unicast packet count value for controlled port of this SecY.
*/

/*!
 * @struct Ra01IfRsMcsCseRxMemIfinctlmcpkts_t
 * @sa MacsecGetRxSecyStatisticCounters
 * @ingroup counter_types
 * @var Ra01IfRsMcsCseRxMemIfinctlmcpkts_t::rxCtlPktMcastCnt
   Ingress Multicast packet count value for controlled port of this SecY.
*/

/*!
 * @struct Ra01IfRsMcsCseRxMemIfinctlbcpkts_t
 * @sa MacsecGetRxSecyStatisticCounters
 * @ingroup counter_types
 * @var Ra01IfRsMcsCseRxMemIfinctlbcpkts_t::rxCtlPktBcastCnt
   Ingress Broadcast packet count value for controlled ports of this SecY.
*/

/*!
 * @struct Ra01IfRsMcsCseRxMemInpktssecyuntaggedornotag_t
 * @sa MacsecGetRxSecyStatisticCounters
 * @ingroup counter_types
 * @var Ra01IfRsMcsCseRxMemInpktssecyuntaggedornotag_t::rxSecyPktUntaggedCnt
   The number of packets without a SecTag received while SecY.Validate_Frames was not STRICT.
*/

/*!
 * @struct Ra01IfRsMcsCseRxMemInpktssecybadtag_t
 * @sa MacsecGetRxSecyStatisticCounters
 * @ingroup counter_types
 * @var Ra01IfRsMcsCseRxMemInpktssecybadtag_t::rxSecyPktBadtagCnt
   The number of received packets discarded with an invalid SecTag, zero value PN, or invalid ICV.
*/

/*!
 * @struct Ra01IfRsMcsCseRxMemInpktssecyctl_t
 * @sa MacsecGetRxSecyStatisticCounters
 * @ingroup counter_types
 * @var Ra01IfRsMcsCseRxMemInpktssecyctl_t::rxSecyPktCtlCnt
   Number of control packets received by the SecY.
*/

/*!
 * @struct Ra01IfRsMcsCseRxMemInpktssecytaggedctl_t
 * @sa MacsecGetRxSecyStatisticCounters
 * @ingroup counter_types
 * @var Ra01IfRsMcsCseRxMemInpktssecytaggedctl_t::rxSecyPktTaggedCtlCnt
   Number of tagged control packets received by the SecY.
*/

/*!
 * @struct Ra01IfRsMcsCseRxMemInpktssecyunknownsci_t
 * @sa MacsecGetRxSecyStatisticCounters
 * @ingroup counter_types
 * @var Ra01IfRsMcsCseRxMemInpktssecyunknownsci_t::rxSecyPktNosaCnt
   The number of received packets with an unknown SCI and secyValidateFrames not strict and sectag.tci.c is zero
*/

/*!
 * @struct Ra01IfRsMcsCseRxMemInpktssecynosci_t
 * @sa MacsecGetRxSecyStatisticCounters
 * @ingroup counter_types
 * @var Ra01IfRsMcsCseRxMemInpktssecynosci_t::rxSecyPktNosaerrorCnt
   The number of received packets with an unknown SCI and secyValidateFrames is strict or sectag.tci.c is one
*/

/*!
 * @struct Ra01IfRsMcsCseRxMemInpktsctrlportdisabled_t
 * @sa MacsecGetRxSecyStatisticCounters
 * @ingroup counter_types
 * @var Ra01IfRsMcsCseRxMemInpktsctrlportdisabled_t::rxSecyPktCtrlPortDisabledCnt
   The number of packets received which are dropped on disabled SecY.
*/

/*!
 * @struct Ra01IfRsMcsCseRxMemInoctetsscvalidate_t
 * @sa MacsecGetRxScStatisticCounters
 * @ingroup counter_types
 * @var Ra01IfRsMcsCseRxMemInoctetsscvalidate_t::rxSecyOctetValidatedCnt
   The number of plaintext octets recovered from packets that were integrity protected but not encrypted.
*/

/*!
 * @struct Ra01IfRsMcsCseRxMemInoctetsscdecrypted_t
 * @sa MacsecGetRxScStatisticCounters
 * @ingroup counter_types
 * @var Ra01IfRsMcsCseRxMemInoctetsscdecrypted_t::rxSecyOctetDecryptedCnt
   The number of plaintext octets recovered from packets that were integrity protected and encrypted.
*/

/*!
 * @struct Ra01IfRsMcsCseRxMemInpktsscunchecked_t
 * @sa MacsecGetRxScStatisticCounters
 * @ingroup counter_types
 * @var Ra01IfRsMcsCseRxMemInpktsscunchecked_t::rxScPktUncheckedCnt
   The number of packets received for this SC, while Validate_Frames was disabled.
*/

/*!
 * @struct Ra01IfRsMcsCseRxMemInpktssclateordelayed_t
 * @sa MacsecGetRxScStatisticCounters
 * @ingroup counter_types
 * @var Ra01IfRsMcsCseRxMemInpktssclateordelayed_t::rxScPktLateCnt
   The number of packets discarded, for this SC, because the received PN was lower than the lowest acceptable PN and with replay protect true.
*/

/*!
 * @struct Ra01IfRsMcsCseRxMemInpktssccamhit_t
 * @sa MacsecGetRxScStatisticCounters
 * @ingroup counter_types
 * @var Ra01IfRsMcsCseRxMemInpktssccamhit_t::rxScPktScCamHitCnt
   The number of Packets which hit an entry in the SC TCAM.  Only 1 counter increments per packet.
*/

/*!
 * @struct Ra01IfRsMcsCseRxMemInpktssaok_t
 * @sa MacsecGetRxSaStatisticCounters
 * @ingroup counter_types
 * @var Ra01IfRsMcsCseRxMemInpktssaok_t::rxScPktOkCnt
   The number of packets received for this SA successfully validated and within the replay window.
*/

/*!
 * @struct Ra01IfRsMcsCseRxMemInpktssainvalid_t
 * @sa MacsecGetRxSaStatisticCounters
 * @ingroup counter_types
 * @var Ra01IfRsMcsCseRxMemInpktssainvalid_t::rxScPktInvalidCnt
   The number of packets, for this SA, that failed validation but could be received because SecY.Validate_Frames was 'CHECK' and the data was not encrypted so the original frame could be recovered.
*/

/*!
 * @struct Ra01IfRsMcsCseRxMemInpktssanotvalid_t
 * @sa MacsecGetRxSaStatisticCounters
 * @ingroup counter_types
 * @var Ra01IfRsMcsCseRxMemInpktssanotvalid_t::rxScPktNotvalidCnt
   The number of packets discarded for this SA because validation failed and SecY.Validate_Frames is 'STRICT' or the data was encrypted so the original frame could not be recovered.
*/

/*!
 * @struct Ra01IfRsMcsCseRxMemInpktssaunusedsa_t
 * @sa MacsecGetRxSaStatisticCounters
 * @ingroup counter_types
 * @var Ra01IfRsMcsCseRxMemInpktssaunusedsa_t::rxSecyPktNosaCnt
   The number of received packets with an unused SA when secyValidateFrames is not strict and sectag.tci.c is 0.
*/

/*!
 * @struct Ra01IfRsMcsCseRxMemInpktssanotusingsaerror_t
 * @sa MacsecGetRxSaStatisticCounters
 * @ingroup counter_types
 * @var Ra01IfRsMcsCseRxMemInpktssanotusingsaerror_t::rxSecyPktNosaerrorCnt
   The number of received packets with an unused SA when secyValidateFrames is strict or sectag.tci.c is 1.
*/

/*!
 * @struct Ra01IfRsMcsCseRxMemInpktsflowidtcammiss_t
 * @sa MacsecGetRxPortStatisticCounters
 * @ingroup counter_types
 * @var Ra01IfRsMcsCseRxMemInpktsflowidtcammiss_t::rxPortPktFlowidTcamMissCnt
   The number of Flow ID TCAM misses per port.
*/

/*!
 * @struct Ra01IfRsMcsCseRxMemInpktsparseerr_t
 * @sa MacsecGetRxPortStatisticCounters
 * @ingroup counter_types
 * @var Ra01IfRsMcsCseRxMemInpktsparseerr_t::rxPortPktParseErrCnt
   The number of packets that have a parse error as indicated by PEX per port.
*/

/*!
 * @struct Ra01IfRsMcsCseRxMemInpktsearlypreempterr_t
 * @sa MacsecGetRxPortStatisticCounters
 * @ingroup counter_types
 * @var Ra01IfRsMcsCseRxMemInpktsearlypreempterr_t::rxPortPktEarlyPreemptErrCnt
   The number of packets with Early Preemption violations detected per port.
*/

/*!
 * @struct Ra01IfRsMcsCseRxMemInpktsflowidtcamhit_t
 * @sa MacsecGetRxFlowStatisticCounters
 * @ingroup counter_types
 * @var Ra01IfRsMcsCseRxMemInpktsflowidtcamhit_t::rxFlowidPktFlowidTcamHitCnt
   The number of Packets which hit an entry in the Flow-ID TCAM.  Only 1 counter increments per packet.
*/

/*!
 * @struct Ra01IfRsMcsCseTxMemIfoutcommonoctets_t
 * @sa MacsecGetTxSecyStatisticCounters
 * @ingroup counter_types
 * @var Ra01IfRsMcsCseTxMemIfoutcommonoctets_t::txUnctlOctetCnt
   Total MSDU and MAC-DA/SA Octets that are permitted by the uncontrolled port policies of this SecY.
*/

/*!
 * @struct Ra01IfRsMcsCseTxMemIfoutunctloctets_t
 * @sa MacsecGetTxSecyStatisticCounters
 * @ingroup counter_types
 * @var Ra01IfRsMcsCseTxMemIfoutunctloctets_t::txUnctlOctetCnt
   Total MSDU and MAC-DA/SA Octets that are permitted by the uncontrolled port policies of this SecY.
*/

/*!
 * @struct Ra01IfRsMcsCseTxMemIfoutctloctets_t
 * @sa MacsecGetTxSecyStatisticCounters
 * @ingroup counter_types
 * @var Ra01IfRsMcsCseTxMemIfoutctloctets_t::txCtlOctetCnt
   Total MSDU and MAC-DA/SA Octets that are permitted by the controlled port policies of this SecY.
*/

/*!
 * @struct Ra01IfRsMcsCseTxMemIfoutunctlucpkts_t
 * @sa MacsecGetTxSecyStatisticCounters
 * @ingroup counter_types
 * @var Ra01IfRsMcsCseTxMemIfoutunctlucpkts_t::txUnctlPktUcastCnt
   Unicast packet count value for uncontrolled ports of this SecY.
*/

/*!
 * @struct Ra01IfRsMcsCseTxMemIfoutunctlmcpkts_t
 * @sa MacsecGetTxSecyStatisticCounters
 * @ingroup counter_types
 * @var Ra01IfRsMcsCseTxMemIfoutunctlmcpkts_t::txUnctlPktMcastCnt
  Multicast packets permitted by the uncontrolled port policies of this SecY
*/

/*!
 * @struct Ra01IfRsMcsCseTxMemIfoutunctlbcpkts_t
 * @sa MacsecGetTxSecyStatisticCounters
 * @ingroup counter_types
 * @var Ra01IfRsMcsCseTxMemIfoutunctlbcpkts_t::txUnctlPktBcastCnt
   Broadcast packet count value for uncontrolled ports of this SecY."
*/

/*!
 * @struct Ra01IfRsMcsCseTxMemIfoutctlucpkts_t
 * @sa MacsecGetTxSecyStatisticCounters
 * @ingroup counter_types
 * @var Ra01IfRsMcsCseTxMemIfoutctlucpkts_t::txCtlPktUcastCnt
   Unicast packet count value for controlled ports of this SecY.
*/

/*!
 * @struct Ra01IfRsMcsCseTxMemIfoutctlmcpkts_t
 * @sa MacsecGetTxSecyStatisticCounters
 * @ingroup counter_types
 * @var Ra01IfRsMcsCseTxMemIfoutctlmcpkts_t::txCtlPktMcastCnt
   Multicast packet count value for controlled ports of this SecY.
*/

/*!
 * @struct Ra01IfRsMcsCseTxMemIfoutctlbcpkts_t
 * @sa MacsecGetTxSecyStatisticCounters
 * @ingroup counter_types
 * @var Ra01IfRsMcsCseTxMemIfoutctlbcpkts_t::txCtlPktBcastCnt
   Broadcast packet count value for controlled ports of this SecY.
*/

/*!
 * @struct Ra01IfRsMcsCseTxMemOutpktssecyuntagged_t
 * @sa MacsecGetTxSecyStatisticCounters
 * @ingroup counter_types
 * @var Ra01IfRsMcsCseTxMemOutpktssecyuntagged_t::txSecyPktUntaggedCnt
   The number of data packets (excluding control packets) transmitted without a SecTag because Protect Frames is false.
*/

/*!
 * @struct Ra01IfRsMcsCseTxMemOutpktssecytoolong_t
 * @sa MacsecGetTxSecyStatisticCounters
 * @ingroup counter_types
 * @var Ra01IfRsMcsCseTxMemOutpktssecytoolong_t::txSecyPktToolongCnt
   The number of transmit packets discarded because their length is greater than the configured MTU after SecTag/ICV insertion.
*/

/*!
 * @struct Ra01IfRsMcsCseTxMemOutpktssecynoactivesa_t
 * @sa MacsecGetTxSecyStatisticCounters
 * @ingroup counter_types
 * @var Ra01IfRsMcsCseTxMemOutpktssecynoactivesa_t::txSecyPktNoactivesaCnt
   The number of data packets with SA value not matching any active SA value configured.
*/

/*!
 * @struct Ra01IfRsMcsCseTxMemOutpktsctrlportdisabled_t
 * @sa MacsecGetTxSecyStatisticCounters
 * @ingroup counter_types
 * @var Ra01IfRsMcsCseTxMemOutpktsctrlportdisabled_t::txSecyPktCtrlPortDisabledCnt
   The number of packets received on disabled SecY.
*/

/*!
 * @struct Ra01IfRsMcsCseTxMemOutoctetsscprotected_t
 * @sa MacsecGetTxScStatisticCounters
 * @ingroup counter_types
 * @var Ra01IfRsMcsCseTxMemOutoctetsscprotected_t::txSecyOctetProtectedCnt
   The number of plain text octets integrity protected but not encrypted in transmitted frames.
*/

/*!
 * @struct Ra01IfRsMcsCseTxMemOutoctetsscencrypted_t
 * @sa MacsecGetTxScStatisticCounters
 * @ingroup counter_types
 * @var Ra01IfRsMcsCseTxMemOutoctetsscencrypted_t::txSecyOctetEncryptedCnt
   The number of plain text octets integrity protected and encrypted in transmitted frames.
*/

/*!
 * @struct Ra01IfRsMcsCseTxMemOutpktssaprotected_t
 * @sa MacsecGetTxSaStatisticCounters
 * @ingroup counter_types
 * @var Ra01IfRsMcsCseTxMemOutpktssaprotected_t::txScPktProtectedCnt
   The number of integrity protected but not encrypted packets for this transmit SA.
*/

/*!
 * @struct Ra01IfRsMcsCseTxMemOutpktssaencrypted_t
 * @sa MacsecGetTxSaStatisticCounters
 * @ingroup counter_types
 * @var Ra01IfRsMcsCseTxMemOutpktssaencrypted_t::txScPktEncryptedCnt
The number of integrity protected and encrypted packets for this transmit SA.
*/

/*!
 * @struct Ra01IfRsMcsCseTxMemOutpktsflowidtcammiss_t
 * @sa MacsecGetTxPortStatisticCounters
 * @ingroup counter_types
 * @var Ra01IfRsMcsCseTxMemOutpktsflowidtcammiss_t::txPortPktFlowidTcamMissCnt
   The number of Flow ID TCAM misses per port.
*/

/*!
 * @struct Ra01IfRsMcsCseTxMemOutpktsparseerr_t
 * @sa MacsecGetTxPortStatisticCounters
 * @ingroup counter_types
 * @var Ra01IfRsMcsCseTxMemOutpktsparseerr_t::txPortPktParseErrCnt
   The number of packets that have a parse error as indicated by PEX per port.
*/

/*!
 * @struct Ra01IfRsMcsCseTxMemOutpktssectaginsertionerr_t
 * @sa MacsecGetTxPortStatisticCounters
 * @ingroup counter_types
 * @var Ra01IfRsMcsCseTxMemOutpktssectaginsertionerr_t::txPortPktSectagInsertionErrCnt
   The number of transmit packets discarded because their associated policy attempted to insert a SecTAG at an offset beyond the EOP of the packet.
*/

/*!
 * @struct Ra01IfRsMcsCseTxMemOutpktsearlypreempterr_t
 * @sa MacsecGetTxPortStatisticCounters
 * @ingroup counter_types
 * @var Ra01IfRsMcsCseTxMemOutpktsearlypreempterr_t::txPortPktEarlyPreemptErrCnt
   The number of packets with Early Preemption violations detected per port.
*/

/*!
 * @struct Ra01IfRsMcsCseTxMemOutpktsflowidtcamhit_t
 * @sa MacsecGetTxFlowStatisticCounters
 * @ingroup counter_types
 * @var Ra01IfRsMcsCseTxMemOutpktsflowidtcamhit_t::txFlowidPktFlowidTcamHitCnt
   The number of Flow ID TCAM hits per flow id entry.
*/

/*!
 * @brief per-Secy RX macsec counters
 *
 * Stores all RX macsec counters that are indexed per secy
 *
 * @sa MacsecGetRxSecyStatisticCounters
 *
 * @ingroup counter_types
 */
typedef struct {
   Ra01IfRsMcsCseRxMemIfinunctloctets_t           Ifinunctloctets;           //!< @sa Ra01IfRsMcsCseRxMemIfinunctloctets_t
   Ra01IfRsMcsCseRxMemIfinctloctets_t             Ifinctloctets;             //!< @sa Ra01IfRsMcsCseRxMemIfinctloctets_t
   Ra01IfRsMcsCseRxMemIfinunctlucpkts_t           Ifinunctlucpkts;           //!< @sa Ra01IfRsMcsCseRxMemIfinunctlucpkts_t
   Ra01IfRsMcsCseRxMemIfinunctlmcpkts_t           Ifinunctlmcpkts;           //!< @sa Ra01IfRsMcsCseRxMemIfinunctlmcpkts_t
   Ra01IfRsMcsCseRxMemIfinunctlbcpkts_t           Ifinunctlbcpkts;           //!< @sa Ra01IfRsMcsCseRxMemIfinunctlbcpkts_t
   Ra01IfRsMcsCseRxMemIfinctlucpkts_t             Ifinctlucpkts;             //!< @sa Ra01IfRsMcsCseRxMemIfinctlucpkts_t
   Ra01IfRsMcsCseRxMemIfinctlmcpkts_t             Ifinctlmcpkts;             //!< @sa Ra01IfRsMcsCseRxMemIfinctlmcpkts_t
   Ra01IfRsMcsCseRxMemIfinctlbcpkts_t             Ifinctlbcpkts;             //!< @sa Ra01IfRsMcsCseRxMemIfinctlbcpkts_t
   Ra01IfRsMcsCseRxMemInpktssecyuntaggedornotag_t Inpktssecyuntaggedornotag; //!< @sa Ra01IfRsMcsCseRxMemInpktssecyuntaggedornotag_t
   Ra01IfRsMcsCseRxMemInpktssecybadtag_t          Inpktssecybadtag;          //!< @sa Ra01IfRsMcsCseRxMemInpktssecybadtag_t
   Ra01IfRsMcsCseRxMemInpktssecyctl_t             Inpktssecyctl;             //!< @sa Ra01IfRsMcsCseRxMemInpktssecyctl_t
   Ra01IfRsMcsCseRxMemInpktssecytaggedctl_t       Inpktssecytaggedctl;       //!< @sa Ra01IfRsMcsCseRxMemInpktssecytaggedctl_t
   Ra01IfRsMcsCseRxMemInpktssecyunknownsci_t      Inpktssecyunknownsci;      //!< @sa Ra01IfRsMcsCseRxMemInpktssecyunknownsci_t
   Ra01IfRsMcsCseRxMemInpktssecynosci_t           Inpktssecynosci;           //!< @sa Ra01IfRsMcsCseRxMemInpktssecynosci_t
   Ra01IfRsMcsCseRxMemInpktsctrlportdisabled_t    Inpktsctrlportdisabled;    //!< @sa Ra01IfRsMcsCseRxMemInpktsctrlportdisabled_t
} MacsecRxSecyCounters_t;

/*!
 * @brief per-SC RX macsec counters
 *
 * Stores all RX macsec counters that are indexed per sc
 *
 * @sa MacsecGetRxScStatisticCounters
 *
 * @ingroup counter_types
 */
typedef struct {
   Ra01IfRsMcsCseRxMemInoctetsscvalidate_t    Inoctetsscvalidate;    //!< @sa Ra01IfRsMcsCseRxMemInoctetsscvalidate_t
   Ra01IfRsMcsCseRxMemInoctetsscdecrypted_t   Inoctetsscdecrypted;   //!< @sa Ra01IfRsMcsCseRxMemInoctetsscdecrypted_t
   Ra01IfRsMcsCseRxMemInpktsscunchecked_t     Inpktsscunchecked;     //!< @sa Ra01IfRsMcsCseRxMemInpktsscunchecked_t
   Ra01IfRsMcsCseRxMemInpktssclateordelayed_t Inpktssclateordelayed; //!< @sa Ra01IfRsMcsCseRxMemInpktssclateordelayed_t
   Ra01IfRsMcsCseRxMemInpktssccamhit_t        Inpktssccamhit;        //!< @sa Ra01IfRsMcsCseRxMemInpktssccamhit_t
} MacsecRxScCounters_t;

/*!
 * @brief per-SA RX macsec counters
 *
 * Stores all RX macsec counters that are indexed per sa
 *
 * @sa MacsecGetRxSaStatisticCounters
 *
 * @ingroup counter_types
 */
typedef struct {
   Ra01IfRsMcsCseRxMemInpktssaok_t              Inpktssaok;              //!< @sa Ra01IfRsMcsCseRxMemInpktssaok_t
   Ra01IfRsMcsCseRxMemInpktssainvalid_t         Inpktssainvalid;         //!< @sa Ra01IfRsMcsCseRxMemInpktssainvalid_t
   Ra01IfRsMcsCseRxMemInpktssanotvalid_t        Inpktssanotvalid;        //!< @sa Ra01IfRsMcsCseRxMemInpktssanotvalid_t
   Ra01IfRsMcsCseRxMemInpktssaunusedsa_t        Inpktssaunusedsa;        //!< @sa Ra01IfRsMcsCseRxMemInpktssaunusedsa_t
   Ra01IfRsMcsCseRxMemInpktssanotusingsaerror_t Inpktssanotusingsaerror; //!< @sa Ra01IfRsMcsCseRxMemInpktssanotusingsaerror_t
} MacsecRxSaCounters_t;

/*!
 * @brief per-port RX macsec counters
 *
 * Stores all RX macsec counters that are indexed per port
 *
 * @sa MacsecGetRxPortStatisticCounters
 *
 * @ingroup counter_types
 */
typedef struct {
   Ra01IfRsMcsCseRxMemInpktsflowidtcammiss_t  Inpktsflowidtcammiss;  //!< @sa Ra01IfRsMcsCseRxMemInpktsflowidtcammiss_t
   Ra01IfRsMcsCseRxMemInpktsparseerr_t        Inpktsparseerr;        //!< @sa Ra01IfRsMcsCseRxMemInpktsparseerr_t
   Ra01IfRsMcsCseRxMemInpktsearlypreempterr_t Inpktsearlypreempterr; //!< @sa Ra01IfRsMcsCseRxMemInpktsearlypreempterr_t
} MacsecRxPortCounters_t;

/*!
 * @brief per-flow RX macsec counters
 *
 * Stores all RX macsec counters that are indexed per flow
 *
 * @sa MacsecGetRxFlowStatisticCounters
 *
 * @ingroup counter_types
 */
typedef struct {
   Ra01IfRsMcsCseRxMemInpktsflowidtcamhit_t Inpktsflowidtcamhit; //!< @sa Ra01IfRsMcsCseRxMemInpktsflowidtcamhit_t
} MacsecRxFlowCounters_t;

/*!
 * @brief per-Secy TX macsec counters
 *
 * Stores all TX macsec counters that are indexed per secy
 *
 * @sa MacsecGetTxSecyStatisticCounters
 *
 * @ingroup counter_types
 */
typedef struct {
   Ra01IfRsMcsCseTxMemIfoutcommonoctets_t       Ifoutcommonoctets;       //!< @sa Ra01IfRsMcsCseTxMemIfoutcommonoctets_t
   Ra01IfRsMcsCseTxMemIfoutunctloctets_t        Ifoutunctloctets;        //!< @sa Ra01IfRsMcsCseTxMemIfoutunctloctets_t
   Ra01IfRsMcsCseTxMemIfoutctloctets_t          Ifoutctloctets;          //!< @sa Ra01IfRsMcsCseTxMemIfoutctloctets_t
   Ra01IfRsMcsCseTxMemIfoutunctlucpkts_t        Ifoutunctlucpkts;        //!< @sa Ra01IfRsMcsCseTxMemIfoutunctlucpkts_t
   Ra01IfRsMcsCseTxMemIfoutunctlmcpkts_t        Ifoutunctlmcpkts;        //!< @sa Ra01IfRsMcsCseTxMemIfoutunctlmcpkts_t
   Ra01IfRsMcsCseTxMemIfoutunctlbcpkts_t        Ifoutunctlbcpkts;        //!< @sa Ra01IfRsMcsCseTxMemIfoutunctlbcpkts_t
   Ra01IfRsMcsCseTxMemIfoutctlucpkts_t          Ifoutctlucpkts;          //!< @sa Ra01IfRsMcsCseTxMemIfoutctlucpkts_t
   Ra01IfRsMcsCseTxMemIfoutctlmcpkts_t          Ifoutctlmcpkts;          //!< @sa Ra01IfRsMcsCseTxMemIfoutctlmcpkts_t
   Ra01IfRsMcsCseTxMemIfoutctlbcpkts_t          Ifoutctlbcpkts;          //!< @sa Ra01IfRsMcsCseTxMemIfoutctlbcpkts_t
   Ra01IfRsMcsCseTxMemOutpktssecyuntagged_t     Outpktssecyuntagged;     //!< @sa Ra01IfRsMcsCseTxMemOutpktssecyuntagged_t
   Ra01IfRsMcsCseTxMemOutpktssecytoolong_t      Outpktssecytoolong;      //!< @sa Ra01IfRsMcsCseTxMemOutpktssecytoolong_t
   Ra01IfRsMcsCseTxMemOutpktssecynoactivesa_t   Outpktssecynoactivesa;   //!< @sa Ra01IfRsMcsCseTxMemOutpktssecynoactivesa_t
   Ra01IfRsMcsCseTxMemOutpktsctrlportdisabled_t Outpktsctrlportdisabled; //!< @sa Ra01IfRsMcsCseTxMemOutpktsctrlportdisabled_t
} MacsecTxSecyCounters_t;

/*!
 * @brief per-SC TX macsec counters
 *
 * Stores all TX macsec counters that are indexed per sc
 *
 * @sa MacsecGetTxScStatisticCounters
 *
 * @ingroup counter_types
 */
typedef struct {
   Ra01IfRsMcsCseTxMemOutoctetsscprotected_t Outoctetsscprotected; //!< @sa Ra01IfRsMcsCseTxMemOutoctetsscprotected_t
   Ra01IfRsMcsCseTxMemOutoctetsscencrypted_t Outoctetsscencrypted; //!< @sa Ra01IfRsMcsCseTxMemOutoctetsscencrypted_t
} MacsecTxScCounters_t;

/*!
 * @brief per-SA TX macsec counters
 *
 * Stores all TX macsec counters that are indexed per sa
 *
 * @sa MacsecGetTxSaStatisticCounters
 *
 * @ingroup counter_types
 */
typedef struct {
   Ra01IfRsMcsCseTxMemOutpktssaprotected_t Outpktssaprotected; //!< @sa Ra01IfRsMcsCseTxMemOutpktssaprotected_t
   Ra01IfRsMcsCseTxMemOutpktssaencrypted_t Outpktssaencrypted; //!< @sa Ra01IfRsMcsCseTxMemOutpktssaencrypted_t
} MacsecTxSaCounters_t;

/*!
 * @brief per-port TX macsec counters
 *
 * Stores all TX macsec counters that are indexed per port
 *
 * @sa MacsecGetTxPortStatisticCounters
 *
 * @ingroup counter_types
 */
typedef struct {
   Ra01IfRsMcsCseTxMemOutpktsflowidtcammiss_t     Outpktsflowidtcammiss;     //!< @sa Ra01IfRsMcsCseTxMemOutpktsflowidtcammiss_t
   Ra01IfRsMcsCseTxMemOutpktsparseerr_t           Outpktsparseerr;           //!< @sa Ra01IfRsMcsCseTxMemOutpktsparseerr_t
   Ra01IfRsMcsCseTxMemOutpktssectaginsertionerr_t Outpktssectaginsertionerr; //!< @sa Ra01IfRsMcsCseTxMemOutpktssectaginsertionerr_t
   Ra01IfRsMcsCseTxMemOutpktsearlypreempterr_t    Outpktsearlypreempterr;    //!< @sa Ra01IfRsMcsCseTxMemOutpktsearlypreempterr_t
} MacsecTxPortCounters_t;

/*!
 * @brief per-flow TX macsec counters
 *
 * Stores all TX macsec counters that are indexed per flow
 *
 * @sa MacsecGetTxFlowStatisticCounters
 *
 * @ingroup counter_types
 */
typedef struct {
   Ra01IfRsMcsCseTxMemOutpktsflowidtcamhit_t Outpktsflowidtcamhit; //!< @sa Ra01IfRsMcsCseTxMemOutpktsflowidtcamhit_t
} MacsecTxFlowCounters_t;

/*!
 * @brief Clear all statistics counters in macsec
 *
 * @param[inout]  rmsDev_p    The hardware handle for the target device
 * @param[in]     clearOnRead Whether or not the device is programmed to clear registers on read
 *
 * Use this function to clear all statistics counters
 *
 * @retval  0      Success
 *
 * @ingroup counter_fns
 */
DLL_PUBLIC int MacsecClearStatisticCounters(RmsDev_t* rmsDev_p, bool clearOnRead);

/*!
 * @brief Read per-secy TX counters
 *
 * @param[inout]  rmsDev_p               The hardware handle for the target device
 * @param[in]     index                  The secy for which the statistics are being accessed
 * @param[inout]  txSecyCounters Structure containing all secy counters
 *
 * @retval  0      Success
 *
 * @ingroup counter_fns
 */
DLL_PUBLIC int MacsecGetTxSecyStatisticCounters( RmsDev_t* rmsDev_p, unsigned index, MacsecTxSecyCounters_t* txSecyCounters);

/*!
 * @brief Read per-sc TX counters
 *
 * @param[inout]  rmsDev_p               The hardware handle for the target device
 * @param[in]     index                  The sc for which the statistics are being accessed
 * @param[inout]  txScCounters   Structure containing all sc counters
 *
 * @retval  0      Success
 *
 * @ingroup counter_fns
 */
DLL_PUBLIC int MacsecGetTxSCStatisticCounters( RmsDev_t* rmsDev_p, unsigned index, MacsecTxScCounters_t* txScCounters);

/*!
 * @brief Read per-sa TX counters
 *
 * @param[inout]  rmsDev_p               The hardware handle for the target device
 * @param[in]     index                  The sa for which the statistics are being accessed
 * @param[inout]  txSaCounters   Structure containing all sa counters
 *
 * Use this function to clear all statistics counters
 *
 * @retval  0      Success
 *
 * @ingroup counter_fns
 */
DLL_PUBLIC int MacsecGetTxSAStatisticCounters( RmsDev_t* rmsDev_p, unsigned index, MacsecTxSaCounters_t* txSaCounters);

/*!
 * @brief Read per-port TX counters
 *
 * @param[inout]  rmsDev_p               The hardware handle for the target device
 * @param[in]     index                  The port for which the statistics are being accessed
 * @param[inout]  txPortCounters   Structure containing all port counters
 *
 * @retval  0      Success
 *
 * @ingroup counter_fns
 */
DLL_PUBLIC int MacsecGetTxPortStatisticCounters( RmsDev_t* rmsDev_p, unsigned index, MacsecTxPortCounters_t* txPortCounters);

/*!
 * @brief Read per-flow TX counters
 *
 * @param[inout]  rmsDev_p               The hardware handle for the target device
 * @param[in]     index                  The flow for which the statistics are being accessed
 * @param[inout]  txFlowCounters   Structure containing all flow counters
 *
 * @retval  0      Success
 *
 * @ingroup counter_fns
 */
DLL_PUBLIC int MacsecGetTxFlowStatisticCounters( RmsDev_t* rmsDev_p, unsigned index, MacsecTxFlowCounters_t* txFlowCounters);

/*!
 * @brief Read per-secy RX counters
 *
 * @param[inout]  rmsDev_p               The hardware handle for the target device
 * @param[in]     index                  The secy for which the statistics are being accessed
 * @param[inout]  rxSecyCounters   Structure containing all secy counters
 *
 * @retval  0      Success
 *
 * @ingroup counter_fns
 */
DLL_PUBLIC int MacsecGetRxSecyStatisticCounters( RmsDev_t* rmsDev_p, unsigned index, MacsecRxSecyCounters_t* rxSecyCounters);

/*!
 * @brief Read per-sc RX counters
 *
 * @param[inout]  rmsDev_p               The hardware handle for the target device
 * @param[in]     index                  The sc for which the statistics are being accessed
 * @param[inout]  rxScCounters   Structure containing all sc counters
 *
 * @retval  0      Success
 *
 * @ingroup counter_fns
 */
DLL_PUBLIC int MacsecGetRxSCStatisticCounters( RmsDev_t* rmsDev_p, unsigned index, MacsecRxScCounters_t* rxScCounters);

/*!
 * @brief Read per-sa RX counters
 *
 * @param[inout]  rmsDev_p               The hardware handle for the target device
 * @param[in]     index                  The sa for which the statistics are being accessed
 * @param[inout]  rxSaCounters   Structure containing all sa counters
 *
 * @retval  0      Success
 *
 * @ingroup counter_fns
 */
DLL_PUBLIC int MacsecGetRxSAStatisticCounters( RmsDev_t* rmsDev_p, unsigned index, MacsecRxSaCounters_t* rxSaCounters);

/*!
 * @brief Read per-port RX counters
 *
 * @param[inout]  rmsDev_p               The hardware handle for the target device
 * @param[in]     index                  The port for which the statistics are being accessed
 * @param[inout]  rxPortCounters   Structure containing all port counters
 *
 * @retval  0      Success
 *
 * @ingroup counter_fns
 */
DLL_PUBLIC int MacsecGetRxPortStatisticCounters( RmsDev_t* rmsDev_p, unsigned index, MacsecRxPortCounters_t* rxPortCounters);

/*!
 * @brief Read per-flow RX counters
 *
 * @param[inout]  rmsDev_p               The hardware handle for the target device
 * @param[in]     index                  The flow for which the statistics are being accessed
 * @param[inout]  rxFlowCounters   Structure containing all flow counters
 *
 * @retval  0      Success
 *
 * @ingroup counter_fns
 */
DLL_PUBLIC int MacsecGetRxFlowStatisticCounters( RmsDev_t* rmsDev_p, unsigned index, MacsecRxFlowCounters_t* rxFlowCounters);

#ifdef __cplusplus
}
#endif

#endif // __MACSEC_COUNTERS_H__

