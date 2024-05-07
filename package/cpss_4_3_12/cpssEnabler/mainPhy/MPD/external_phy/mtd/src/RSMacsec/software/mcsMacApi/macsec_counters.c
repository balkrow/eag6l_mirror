/*! 
 * @file macsec_counters.c
 */

// *******************************************************************************
// *
// * MARVELL CONFIDENTIAL
// * ___________________
// *
// * Copyright (C) 2022 Marvell.
// * All Rights Reserved.
// *
// * NOTICE: All information contained herein is, and remains the property of
// * Marvell Asia Pte Ltd and its suppliers, if any. The intellectual and
// * technical concepts contained herein are proprietary to Marvell Asia Pte Ltd
// * and its suppliers and may be covered by U.S. and Foreign Patents, patents
// * in process, and are protected by trade secret or copyright law.
// * Dissemination of this information or reproduction of this material is strictly
// * forbidden unless prior written permission is obtained from Marvell Asia Pte Ltd.
// *
// ******************************************************************************
// * $Rev: 1.1.0
// * $Date: 2022-11-17-16:28:56
// ******************************************************************************

#include <errno.h>
#include <string.h>

#ifdef MACSEC_API_IEEE
#include "mcs_internals_ieee.h"
#else
#include "mcs_internals_CT.h"
#endif

#include "macsec_api.h"
#include "macsec_counters.h"
#include "i_macsec.h"

#define SIM_PRINT_LEVEL SIM_DEBUG
#include "sim_log.h"

int MacsecClearStatisticCounters
(
    RmsDev_t * rmsDev_p,
    bool clearOnRead
)
{
  int ret = 0;
    Ra01RsMcsCseRxCtrl_t cseRxCtrl;
    Ra01RsMcsCseTxCtrl_t cseTxCtrl;
    Ra01RsMcsCseRxStatsClear_t cseRxStatsClear;
    Ra01RsMcsCseTxStatsClear_t cseTxStatsClear;

    //SIM_PRINT(SIM_DEBUG, "MacsecClearStatisticCounters.\n");

    cseRxCtrl.cpuCseClrOnRd = clearOnRead;
    ret |= Ra01AccRsMcsCseRxCtrl(rmsDev_p, &cseRxCtrl, RA01_READ_OP);

    cseTxCtrl.cpuCseClrOnRd = clearOnRead;
    ret |= Ra01AccRsMcsCseTxCtrl(rmsDev_p, &cseTxCtrl, RA01_READ_OP);

    cseRxStatsClear.rxFlowidGo = 1;
    cseRxStatsClear.rxPortGo = 1;
    cseRxStatsClear.rxSaGo = 1;
    cseRxStatsClear.rxScGo = 1;
    cseRxStatsClear.rxSecyGo = 1;
    ret |= Ra01AccRsMcsCseRxStatsClear(rmsDev_p, &cseRxStatsClear, RA01_WRITE_OP);

    cseTxStatsClear.txFlowidGo = 1;
    cseTxStatsClear.txPortGo = 1;
    cseTxStatsClear.txSaGo = 1;
    cseTxStatsClear.txScGo = 1;
    cseTxStatsClear.txSecyGo = 1;
    ret |= Ra01AccRsMcsCseTxStatsClear(rmsDev_p, &cseTxStatsClear, RA01_WRITE_OP);

    return ret;
}

int MacsecGetTxSecyStatisticCounters( RmsDev_t* rmsDev_p, unsigned index, MacsecTxSecyCounters_t* txSecyCounters) {
   int ret = 0;
   ret |= Ra01ReadRsMcsCseTxMemIfoutcommonoctets(rmsDev_p, index, &(txSecyCounters->Ifoutcommonoctets)); 
   ret |= Ra01ReadRsMcsCseTxMemIfoutunctloctets(rmsDev_p, index, &(txSecyCounters->Ifoutunctloctets)); 
   ret |= Ra01ReadRsMcsCseTxMemIfoutctloctets(rmsDev_p, index, &(txSecyCounters->Ifoutctloctets)); 
   ret |= Ra01ReadRsMcsCseTxMemIfoutunctlucpkts(rmsDev_p, index, &(txSecyCounters->Ifoutunctlucpkts)); 
   ret |= Ra01ReadRsMcsCseTxMemIfoutunctlmcpkts(rmsDev_p, index, &(txSecyCounters->Ifoutunctlmcpkts)); 
   ret |= Ra01ReadRsMcsCseTxMemIfoutunctlbcpkts(rmsDev_p, index, &(txSecyCounters->Ifoutunctlbcpkts)); 
   ret |= Ra01ReadRsMcsCseTxMemIfoutctlucpkts(rmsDev_p, index, &(txSecyCounters->Ifoutctlucpkts)); 
   ret |= Ra01ReadRsMcsCseTxMemIfoutctlmcpkts(rmsDev_p, index, &(txSecyCounters->Ifoutctlmcpkts)); 
   ret |= Ra01ReadRsMcsCseTxMemIfoutctlbcpkts(rmsDev_p, index, &(txSecyCounters->Ifoutctlbcpkts)); 
   ret |= Ra01ReadRsMcsCseTxMemOutpktssecyuntagged(rmsDev_p, index, &(txSecyCounters->Outpktssecyuntagged)); 
   ret |= Ra01ReadRsMcsCseTxMemOutpktssecytoolong(rmsDev_p, index, &(txSecyCounters->Outpktssecytoolong)); 
   ret |= Ra01ReadRsMcsCseTxMemOutpktssecynoactivesa(rmsDev_p, index, &(txSecyCounters->Outpktssecynoactivesa)); 
   ret |= Ra01ReadRsMcsCseTxMemOutpktsctrlportdisabled(rmsDev_p, index, &(txSecyCounters->Outpktsctrlportdisabled)); 
   return ret;
}

int MacsecGetTxSCStatisticCounters( RmsDev_t* rmsDev_p, unsigned index, MacsecTxScCounters_t* txScCounters) {
   int ret = 0;
   ret |= Ra01ReadRsMcsCseTxMemOutoctetsscprotected(rmsDev_p, index, &(txScCounters->Outoctetsscprotected)); 
   ret |= Ra01ReadRsMcsCseTxMemOutoctetsscencrypted(rmsDev_p, index, &(txScCounters->Outoctetsscencrypted)); 
   return ret;
}

int MacsecGetTxSAStatisticCounters( RmsDev_t* rmsDev_p, unsigned index, MacsecTxSaCounters_t* txSaCounters) {
   int ret = 0;
   ret |= Ra01ReadRsMcsCseTxMemOutpktssaprotected(rmsDev_p, index, &(txSaCounters->Outpktssaprotected)); 
   ret |= Ra01ReadRsMcsCseTxMemOutpktssaencrypted(rmsDev_p, index, &(txSaCounters->Outpktssaencrypted)); 
   return ret;
}

int MacsecGetTxPortStatisticCounters( RmsDev_t* rmsDev_p, unsigned index, MacsecTxPortCounters_t* txPortCounters) {
   int ret = 0;
   ret |= Ra01ReadRsMcsCseTxMemOutpktsflowidtcammiss(rmsDev_p, index, &(txPortCounters->Outpktsflowidtcammiss)); 
   ret |= Ra01ReadRsMcsCseTxMemOutpktsparseerr(rmsDev_p, index, &(txPortCounters->Outpktsparseerr)); 
   ret |= Ra01ReadRsMcsCseTxMemOutpktssectaginsertionerr(rmsDev_p, index, &(txPortCounters->Outpktssectaginsertionerr)); 
   ret |= Ra01ReadRsMcsCseTxMemOutpktsearlypreempterr(rmsDev_p, index, &(txPortCounters->Outpktsearlypreempterr)); 
   return ret;
}

int MacsecGetTxFlowStatisticCounters( RmsDev_t* rmsDev_p, unsigned index, MacsecTxFlowCounters_t* txFlowCounters) {
   int ret = 0;
   ret = Ra01ReadRsMcsCseTxMemOutpktsflowidtcamhit(rmsDev_p, index, &(txFlowCounters->Outpktsflowidtcamhit)); 
   return ret;
}

int MacsecGetRxSecyStatisticCounters( RmsDev_t* rmsDev_p, unsigned index, MacsecRxSecyCounters_t* rxSecyCounters) {
   int ret = 0;
   ret |= Ra01ReadRsMcsCseRxMemIfinunctloctets(rmsDev_p, index, &(rxSecyCounters->Ifinunctloctets)); 
   ret |= Ra01ReadRsMcsCseRxMemIfinctloctets(rmsDev_p, index, &(rxSecyCounters->Ifinctloctets)); 
   ret |= Ra01ReadRsMcsCseRxMemIfinunctlucpkts(rmsDev_p, index, &(rxSecyCounters->Ifinunctlucpkts)); 
   ret |= Ra01ReadRsMcsCseRxMemIfinunctlmcpkts(rmsDev_p, index, &(rxSecyCounters->Ifinunctlmcpkts)); 
   ret |= Ra01ReadRsMcsCseRxMemIfinunctlbcpkts(rmsDev_p, index, &(rxSecyCounters->Ifinunctlbcpkts)); 
   ret |= Ra01ReadRsMcsCseRxMemIfinctlucpkts(rmsDev_p, index, &(rxSecyCounters->Ifinctlucpkts)); 
   ret |= Ra01ReadRsMcsCseRxMemIfinctlmcpkts(rmsDev_p, index, &(rxSecyCounters->Ifinctlmcpkts)); 
   ret |= Ra01ReadRsMcsCseRxMemIfinctlbcpkts(rmsDev_p, index, &(rxSecyCounters->Ifinctlbcpkts)); 
   ret |= Ra01ReadRsMcsCseRxMemInpktssecyuntaggedornotag(rmsDev_p, index, &(rxSecyCounters->Inpktssecyuntaggedornotag)); 
   ret |= Ra01ReadRsMcsCseRxMemInpktssecybadtag(rmsDev_p, index, &(rxSecyCounters->Inpktssecybadtag)); 
   ret |= Ra01ReadRsMcsCseRxMemInpktssecyctl(rmsDev_p, index, &(rxSecyCounters->Inpktssecyctl)); 
   ret |= Ra01ReadRsMcsCseRxMemInpktssecytaggedctl(rmsDev_p, index, &(rxSecyCounters->Inpktssecytaggedctl)); 
   ret |= Ra01ReadRsMcsCseRxMemInpktssecyunknownsci(rmsDev_p, index, &(rxSecyCounters->Inpktssecyunknownsci)); 
   ret |= Ra01ReadRsMcsCseRxMemInpktssecynosci(rmsDev_p, index, &(rxSecyCounters->Inpktssecynosci)); 
   ret |= Ra01ReadRsMcsCseRxMemInpktsctrlportdisabled(rmsDev_p, index, &(rxSecyCounters->Inpktsctrlportdisabled)); 
   return ret;
}
int MacsecGetRxSCStatisticCounters( RmsDev_t* rmsDev_p, unsigned index, MacsecRxScCounters_t* rxScCounters) {
   int ret = 0;
   ret |= Ra01ReadRsMcsCseRxMemInoctetsscvalidate(rmsDev_p, index, &(rxScCounters->Inoctetsscvalidate)); 
   ret |= Ra01ReadRsMcsCseRxMemInoctetsscdecrypted(rmsDev_p, index, &(rxScCounters->Inoctetsscdecrypted)); 
   ret |= Ra01ReadRsMcsCseRxMemInpktsscunchecked(rmsDev_p, index, &(rxScCounters->Inpktsscunchecked)); 
   ret |= Ra01ReadRsMcsCseRxMemInpktssclateordelayed(rmsDev_p, index, &(rxScCounters->Inpktssclateordelayed)); 
   ret |= Ra01ReadRsMcsCseRxMemInpktssccamhit(rmsDev_p, index, &(rxScCounters->Inpktssccamhit)); 
   return ret;
}
int MacsecGetRxSAStatisticCounters( RmsDev_t* rmsDev_p, unsigned index, MacsecRxSaCounters_t* rxSaCounters) {
   int ret = 0;
   ret |= Ra01ReadRsMcsCseRxMemInpktssaok(rmsDev_p, index, &(rxSaCounters->Inpktssaok)); 
   ret |= Ra01ReadRsMcsCseRxMemInpktssainvalid(rmsDev_p, index, &(rxSaCounters->Inpktssainvalid)); 
   ret |= Ra01ReadRsMcsCseRxMemInpktssanotvalid(rmsDev_p, index, &(rxSaCounters->Inpktssanotvalid)); 
   ret |= Ra01ReadRsMcsCseRxMemInpktssaunusedsa(rmsDev_p, index, &(rxSaCounters->Inpktssaunusedsa)); 
   ret |= Ra01ReadRsMcsCseRxMemInpktssanotusingsaerror(rmsDev_p, index, &(rxSaCounters->Inpktssanotusingsaerror)); 
   return ret;
}
int MacsecGetRxPortStatisticCounters( RmsDev_t* rmsDev_p, unsigned index, MacsecRxPortCounters_t* rxPortCounters) {
   int ret = 0;
   ret |= Ra01ReadRsMcsCseRxMemInpktsflowidtcammiss(rmsDev_p, index, &(rxPortCounters->Inpktsflowidtcammiss)); 
   ret |= Ra01ReadRsMcsCseRxMemInpktsparseerr(rmsDev_p, index, &(rxPortCounters->Inpktsparseerr)); 
   ret |= Ra01ReadRsMcsCseRxMemInpktsearlypreempterr(rmsDev_p, index, &(rxPortCounters->Inpktsearlypreempterr)); 
   return ret;
}
int MacsecGetRxFlowStatisticCounters( RmsDev_t* rmsDev_p, unsigned index, MacsecRxFlowCounters_t* rxFlowCounters) {
   int ret = 0;
   ret = Ra01ReadRsMcsCseRxMemInpktsflowidtcamhit(rmsDev_p, index, &(rxFlowCounters->Inpktsflowidtcamhit)); 
   return ret;
}
