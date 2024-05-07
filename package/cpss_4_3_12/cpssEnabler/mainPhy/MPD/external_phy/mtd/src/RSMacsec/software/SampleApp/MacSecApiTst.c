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

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <errno.h>
#include <inttypes.h>

#include "version.h"

#include "MacSecApiTst.h"

void test_MacSecGetEnableSc(RmsDev_t * device_p)
{
    unsigned idx;


    for (idx = 0; idx< MACSEC_NUM_SC; idx++) {
        bool origVal, testVal;

        MacsecGetEnableSc( device_p, idx, &origVal );
        memcpy(&testVal,&origVal,sizeof(testVal));

        MacsecSetEnableSc( device_p, idx, origVal );
        MacsecGetEnableSc( device_p, idx, &origVal );

        assert( memcmp(&testVal,&origVal,sizeof(testVal)) == 0 );
    }
}

void test_MacsecGetSaPolicy(RmsDev_t * device_p, bool testLockout)
{
    unsigned idx;

    for (idx = 0; idx< MACSEC_NUM_SA; idx++) {
        MacsecSaPolicy_t origVal, testVal;
        bool origLock, testLock;

        MacsecGetSaPolicy( device_p, MACSEC_EGRESS , idx, &origVal, &origLock );
        memcpy(&testVal,&origVal,sizeof(testVal));
        testLock = origLock;

        MacsecSetSaPolicy( device_p, MACSEC_EGRESS , idx, &origVal, origLock );
        MacsecGetSaPolicy( device_p, MACSEC_EGRESS , idx, &origVal, &origLock );

        assert( memcmp(&testVal,&origVal,sizeof(testVal)) == 0 );
        assert( testLock == origLock );

        if (testLockout) {
           // test lockout
           MacsecSetSaPolicy( device_p, MACSEC_EGRESS , idx, &origVal, 1 );
           MacsecGetSaPolicy( device_p, MACSEC_EGRESS , idx, &origVal, &origLock );
           for (int ii = 0; ii < 32; ii++) {
              if (ii < 16) {
                 assert(origVal.egress.hashkey[ii] == 0);
              }
              if (ii < 12) {
                 assert(origVal.egress.salt[ii] == testVal.egress.salt[ii]);
              }
              assert(origVal.egress.sak[ii] == 0);
           }
           assert(origVal.egress.ssci == testVal.egress.ssci);
           assert(origLock == 1);
           MacsecSetSaPolicy( device_p, MACSEC_EGRESS , idx, &testVal, 0 );
        }

        origLock = testLock = 0;
    
        MacsecGetSaPolicy( device_p, MACSEC_INGRESS , idx, &origVal, &origLock );
        memcpy(&testVal,&origVal,sizeof(testVal));
        testLock = origLock;

        MacsecSetSaPolicy( device_p, MACSEC_INGRESS , idx, &origVal, origLock );
        MacsecGetSaPolicy( device_p, MACSEC_INGRESS , idx, &origVal, &origLock );

        assert( memcmp(&(testVal.ingress),&(origVal.ingress),sizeof(testVal.ingress)) == 0 );
        assert( testLock == origLock );

        if (testLockout) {
           // test lockout
           MacsecSetSaPolicy( device_p, MACSEC_INGRESS , idx, &origVal, 1 );
           MacsecGetSaPolicy( device_p, MACSEC_INGRESS , idx, &origVal, &origLock );
           for (int ii = 0; ii < 32; ii++) {
              if (ii < 16) {
                 assert(origVal.ingress.hashkey[ii] == 0);
              }
              if (ii < 12) {
                 assert(origVal.ingress.salt[ii] == testVal.ingress.salt[ii]);
              }
              assert(origVal.ingress.sak[ii] == 0);
           }
           assert(origVal.ingress.ssci == testVal.ingress.ssci);
           assert(origLock == 1);
           MacsecSetSaPolicy( device_p, MACSEC_INGRESS , idx, &testVal, 0 );
        }
    }
}

void test_MacsecGetNextPn(RmsDev_t * device_p)
{
    unsigned idx;

    for (idx = 0; idx< MACSEC_NUM_SA ; idx++) {
        uint64_t origVal, testVal;

        MacsecGetNextPn( device_p, MACSEC_INGRESS , idx,  &origVal );
        memcpy(&testVal,&origVal,sizeof(testVal));

        MacsecSetNextPn( device_p, MACSEC_INGRESS , idx, origVal );
        MacsecGetNextPn( device_p, MACSEC_INGRESS , idx, &origVal );

        assert( memcmp(&testVal,&origVal,sizeof(testVal)) == 0 );

        origVal = testVal = 0;
        MacsecGetNextPn( device_p, MACSEC_EGRESS , idx,  &origVal );
        memcpy(&testVal,&origVal,sizeof(testVal));

        MacsecSetNextPn( device_p, MACSEC_EGRESS , idx, origVal );
        MacsecGetNextPn( device_p, MACSEC_EGRESS , idx, &origVal );

        assert( memcmp(&testVal,&origVal,sizeof(testVal)) == 0 );
    }
}


void test_MacsecGetEgressSaMap(RmsDev_t * device_p)
{
	unsigned idx;

	for (idx = 0; idx< MACSEC_NUM_SC; idx++) {
	    MacsecEgressSaMapEntry_t origVal, testVal;

	    MacsecGetEgressSaMap( device_p, idx, &origVal );
	    memcpy(&testVal,&origVal,sizeof(testVal));

	    MacsecSetEgressSaMap( device_p, idx, & origVal );
	    MacsecGetEgressSaMap( device_p, idx, & origVal );
	    assert( memcmp(&testVal,&origVal,sizeof(testVal)) == 0 );
    }
}

void test_MacsecGetEnableIngressSa(RmsDev_t * device_p)
{

  for (int idx = 0; idx < MACSEC_NUM_SC*MACSEC_NUM_AN; idx++) {
    uint32_t origSaIndex, testSaIndex;
    bool origEnable, testEnable;
    MacsecGetEnableIngressSa(device_p, idx, &origSaIndex, &origEnable);
    testSaIndex = origSaIndex;
    testEnable = origEnable;
    MacsecEnableIngressSa(device_p, idx, origSaIndex, origEnable);
    MacsecGetEnableIngressSa(device_p, idx, &origSaIndex, &origEnable);
    if ((testSaIndex != origSaIndex) || (testEnable != origEnable)) {
      assert (0);
    }
  }
}

void test_MacsecGetConfiguration(RmsDev_t * device_p)
{

	    MacsecConfig_t origVal, testVal;

	    MacsecGetConfiguration( device_p, &origVal );
	    memcpy(&testVal,&origVal,sizeof(testVal));

	    MacsecSetConfiguration( device_p, &origVal );
	    MacsecGetConfiguration( device_p, & origVal );

	    assert( memcmp(&testVal,&origVal,sizeof(testVal)) == 0 );
}

void test_InterruptApis(RmsDev_t * device_p) {
   MacsecEnableInterrupts(device_p);
   #ifdef MCS_API_DEBUG
   MacsecInterrupts_t interruptStatus, expStatus, interruptClear;
   memset(&expStatus, 0, sizeof(MacsecInterrupts_t));
   memset(&interruptClear, 0, sizeof(MacsecInterrupts_t));
   MacsecGetInterruptStatus(device_p, &interruptStatus);
   assert(memcmp(&interruptStatus, &expStatus, sizeof(MacsecInterrupts_t)) == 0);

   // test leaf interrupts
   // activate interrupts for all SCs
   Ra01RsMcsCpmRxScExpiryPreTimeoutIntrRw_t rxScExpiryPreTimeoutInt;
   rxScExpiryPreTimeoutInt.scExpiryPreTimeoutInt= 0xffffffff;
   Ra01AccRsMcsCpmRxScExpiryPreTimeoutIntrRw(device_p, 0, &rxScExpiryPreTimeoutInt, RA01_WRITE_OP);
   rxScExpiryPreTimeoutInt.scExpiryPreTimeoutInt= 0x0;
   Ra01AccRsMcsCpmRxScExpiryPreTimeoutIntrRw(device_p, 0, &rxScExpiryPreTimeoutInt, RA01_WRITE_OP);
   // activate root interrupt
   Ra01RsMcsCpmRxCpmRxIntIntrRw_t rxCpmInt;
   memset(&rxCpmInt, 0, sizeof(Ra01RsMcsCpmRxCpmRxIntIntrRw_t));
   rxCpmInt.scExpiryPreTimeout = 1;
   Ra01AccRsMcsCpmRxCpmRxIntIntrRw(device_p, &rxCpmInt, RA01_WRITE_OP);
   rxCpmInt.scExpiryPreTimeout = 0;
   Ra01AccRsMcsCpmRxCpmRxIntIntrRw(device_p, &rxCpmInt, RA01_WRITE_OP);
   // check interrupt status is as expected
   expStatus.cpmRxInterrupts.scExpiryPreTimeout = 0xffffffff;
   MacsecGetInterruptStatus(device_p, &interruptStatus);
   assert(memcmp(&interruptStatus, &expStatus, sizeof(MacsecInterrupts_t)) == 0);
   // clear one, ensure root node still active
   interruptClear.cpmRxInterrupts.scExpiryPreTimeout = 0x1;
   MacsecClearInterrupts(device_p, interruptClear);
   expStatus.cpmRxInterrupts.scExpiryPreTimeout = 0xfffffffe;
   MacsecGetInterruptStatus(device_p, &interruptStatus);
   assert(memcmp(&interruptStatus, &expStatus, sizeof(MacsecInterrupts_t)) == 0);
   Ra01RsMcsCpmRxCpmRxInt_t rxCpmIntStatus;
   Ra01AccRsMcsCpmRxCpmRxInt(device_p, &rxCpmIntStatus, RA01_READ_OP);
   assert(rxCpmIntStatus.scExpiryPreTimeout == 1);
   // clear remaining, ensure root node is cleared
   interruptClear.cpmRxInterrupts.scExpiryPreTimeout = 0xfffffffe;
   MacsecClearInterrupts(device_p, interruptClear);
   expStatus.cpmRxInterrupts.scExpiryPreTimeout = 0x0;
   MacsecGetInterruptStatus(device_p, &interruptStatus);
   assert(memcmp(&interruptStatus, &expStatus, sizeof(MacsecInterrupts_t)) == 0);
   Ra01AccRsMcsCpmRxCpmRxInt(device_p, &rxCpmIntStatus, RA01_READ_OP);
   assert(rxCpmIntStatus.scExpiryPreTimeout == 0);
   // activate interrupts for all SCs
   Ra01RsMcsCpmRxScExpiryActualTimeoutIntrRw_t rxScExpiryActualTimeoutInt;
   rxScExpiryActualTimeoutInt.scExpiryActualTimeoutInt= 0xffffffff;
   Ra01AccRsMcsCpmRxScExpiryActualTimeoutIntrRw(device_p, 0, &rxScExpiryActualTimeoutInt, RA01_WRITE_OP);
   rxScExpiryActualTimeoutInt.scExpiryActualTimeoutInt= 0x0;
   Ra01AccRsMcsCpmRxScExpiryActualTimeoutIntrRw(device_p, 0, &rxScExpiryActualTimeoutInt, RA01_WRITE_OP);
   // activate root interrupt
   memset(&rxCpmInt, 0, sizeof(Ra01RsMcsCpmRxCpmRxIntIntrRw_t));
   rxCpmInt.scExpiryActualTimeout = 1;
   Ra01AccRsMcsCpmRxCpmRxIntIntrRw(device_p, &rxCpmInt, RA01_WRITE_OP);
   rxCpmInt.scExpiryActualTimeout = 0;
   Ra01AccRsMcsCpmRxCpmRxIntIntrRw(device_p, &rxCpmInt, RA01_WRITE_OP);
   // check interrupt status is as expected
   expStatus.cpmRxInterrupts.scExpiryActualTimeout = 0xffffffff;
   MacsecGetInterruptStatus(device_p, &interruptStatus);
   assert(memcmp(&interruptStatus, &expStatus, sizeof(MacsecInterrupts_t)) == 0);
   // clear one, ensure root node still active
   interruptClear.cpmRxInterrupts.scExpiryActualTimeout = 0x1;
   MacsecClearInterrupts(device_p, interruptClear);
   expStatus.cpmRxInterrupts.scExpiryActualTimeout = 0xfffffffe;
   MacsecGetInterruptStatus(device_p, &interruptStatus);
   assert(memcmp(&interruptStatus, &expStatus, sizeof(MacsecInterrupts_t)) == 0);
   Ra01AccRsMcsCpmRxCpmRxInt(device_p, &rxCpmIntStatus, RA01_READ_OP);
   assert(rxCpmIntStatus.scExpiryActualTimeout == 1);
   // clear remaining, ensure root node is cleared
   interruptClear.cpmRxInterrupts.scExpiryActualTimeout = 0xfffffffe;
   MacsecClearInterrupts(device_p, interruptClear);
   expStatus.cpmRxInterrupts.scExpiryActualTimeout = 0x0;
   MacsecGetInterruptStatus(device_p, &interruptStatus);
   assert(memcmp(&interruptStatus, &expStatus, sizeof(MacsecInterrupts_t)) == 0);
   Ra01AccRsMcsCpmRxCpmRxInt(device_p, &rxCpmIntStatus, RA01_READ_OP);
   assert(rxCpmIntStatus.scExpiryActualTimeout == 0);

   // activate interrupts for all SCs
   Ra01RsMcsCpmTxScExpiryPreTimeoutIntrRw_t txScExpiryPreTimeoutInt;
   txScExpiryPreTimeoutInt.scExpiryPreTimeoutInt= 0xffffffff;
   Ra01AccRsMcsCpmTxScExpiryPreTimeoutIntrRw(device_p, 0, &txScExpiryPreTimeoutInt, RA01_WRITE_OP);
   txScExpiryPreTimeoutInt.scExpiryPreTimeoutInt= 0x0;
   Ra01AccRsMcsCpmTxScExpiryPreTimeoutIntrRw(device_p, 0, &txScExpiryPreTimeoutInt, RA01_WRITE_OP);
   // activate root interrupt
   Ra01RsMcsCpmTxCpmTxIntIntrRw_t txCpmInt;
   memset(&txCpmInt, 0, sizeof(Ra01RsMcsCpmTxCpmTxIntIntrRw_t));
   txCpmInt.scExpiryPreTimeout = 1;
   Ra01AccRsMcsCpmTxCpmTxIntIntrRw(device_p, &txCpmInt, RA01_WRITE_OP);
   txCpmInt.scExpiryPreTimeout = 0;
   Ra01AccRsMcsCpmTxCpmTxIntIntrRw(device_p, &txCpmInt, RA01_WRITE_OP);
   // check interrupt status is as expected
   expStatus.cpmTxInterrupts.scExpiryPreTimeout = 0xffffffff;
   MacsecGetInterruptStatus(device_p, &interruptStatus);
   assert(memcmp(&interruptStatus, &expStatus, sizeof(MacsecInterrupts_t)) == 0);
   // clear one, ensure root node still active
   interruptClear.cpmTxInterrupts.scExpiryPreTimeout = 0x1;
   MacsecClearInterrupts(device_p, interruptClear);
   expStatus.cpmTxInterrupts.scExpiryPreTimeout = 0xfffffffe;
   MacsecGetInterruptStatus(device_p, &interruptStatus);
   assert(memcmp(&interruptStatus, &expStatus, sizeof(MacsecInterrupts_t)) == 0);
   Ra01RsMcsCpmTxCpmTxInt_t txCpmIntStatus;
   Ra01AccRsMcsCpmTxCpmTxInt(device_p, &txCpmIntStatus, RA01_READ_OP);
   assert(txCpmIntStatus.scExpiryPreTimeout == 1);
   // clear remaining, ensure root node is cleared
   interruptClear.cpmTxInterrupts.scExpiryPreTimeout = 0xfffffffe;
   MacsecClearInterrupts(device_p, interruptClear);
   expStatus.cpmTxInterrupts.scExpiryPreTimeout = 0x0;
   MacsecGetInterruptStatus(device_p, &interruptStatus);
   assert(memcmp(&interruptStatus, &expStatus, sizeof(MacsecInterrupts_t)) == 0);
   Ra01AccRsMcsCpmTxCpmTxInt(device_p, &txCpmIntStatus, RA01_READ_OP);
   assert(txCpmIntStatus.scExpiryPreTimeout == 0);

   // activate interrupts for all SCs
   Ra01RsMcsCpmTxScExpiryActualTimeoutIntrRw_t txScExpiryActualTimeoutInt;
   txScExpiryActualTimeoutInt.scExpiryActualTimeoutInt= 0xffffffff;
   Ra01AccRsMcsCpmTxScExpiryActualTimeoutIntrRw(device_p, 0, &txScExpiryActualTimeoutInt, RA01_WRITE_OP);
   txScExpiryActualTimeoutInt.scExpiryActualTimeoutInt= 0x0;
   Ra01AccRsMcsCpmTxScExpiryActualTimeoutIntrRw(device_p, 0, &txScExpiryActualTimeoutInt, RA01_WRITE_OP);
   // activate root interrupt
   memset(&txCpmInt, 0, sizeof(Ra01RsMcsCpmTxCpmTxIntIntrRw_t));
   txCpmInt.scExpiryActualTimeout = 1;
   Ra01AccRsMcsCpmTxCpmTxIntIntrRw(device_p, &txCpmInt, RA01_WRITE_OP);
   txCpmInt.scExpiryActualTimeout = 0;
   Ra01AccRsMcsCpmTxCpmTxIntIntrRw(device_p, &txCpmInt, RA01_WRITE_OP);
   // check interrupt status is as expected
   expStatus.cpmTxInterrupts.scExpiryActualTimeout = 0xffffffff;
   MacsecGetInterruptStatus(device_p, &interruptStatus);
   assert(memcmp(&interruptStatus, &expStatus, sizeof(MacsecInterrupts_t)) == 0);
   // clear one, ensure root node still active
   interruptClear.cpmTxInterrupts.scExpiryActualTimeout = 0x1;
   MacsecClearInterrupts(device_p, interruptClear);
   expStatus.cpmTxInterrupts.scExpiryActualTimeout = 0xfffffffe;
   MacsecGetInterruptStatus(device_p, &interruptStatus);
   assert(memcmp(&interruptStatus, &expStatus, sizeof(MacsecInterrupts_t)) == 0);
   Ra01AccRsMcsCpmTxCpmTxInt(device_p, &txCpmIntStatus, RA01_READ_OP);
   assert(txCpmIntStatus.scExpiryActualTimeout == 1);
   // clear remaining, ensure root node is cleared
   interruptClear.cpmTxInterrupts.scExpiryActualTimeout = 0xfffffffe;
   MacsecClearInterrupts(device_p, interruptClear);
   expStatus.cpmTxInterrupts.scExpiryActualTimeout = 0x0;
   MacsecGetInterruptStatus(device_p, &interruptStatus);
   assert(memcmp(&interruptStatus, &expStatus, sizeof(MacsecInterrupts_t)) == 0);
   Ra01AccRsMcsCpmTxCpmTxInt(device_p, &txCpmIntStatus, RA01_READ_OP);
   assert(txCpmIntStatus.scExpiryActualTimeout == 0);

   // fire all interrupts, check status, clear, check again
   Ra01RsMcsBbeBbeIntIntrRw_t bbeInt;
   Ra01RsMcsPabPabIntIntrRw_t pabInt;
   Ra01RsMcsPexPexIntIntrRw_t pexInt;
   rxCpmInt.sectagVEq1 = 1;
   rxCpmInt.sectagEEq0CEq1 = 1;
   rxCpmInt.sectagSlGte48 = 1;
   rxCpmInt.sectagEsEq1ScEq1 = 1;
   rxCpmInt.sectagScEq1ScbEq1 = 1;
   rxCpmInt.packetXpnEq0 = 1;
   rxCpmInt.pnThreshReached = 1;
   expStatus.cpmRxInterrupts.sectagVEq1 = 1;
   expStatus.cpmRxInterrupts.sectagEEq0CEq1 = 1;
   expStatus.cpmRxInterrupts.sectagSlGte48 = 1;
   expStatus.cpmRxInterrupts.sectagEsEq1ScEq1 = 1;
   expStatus.cpmRxInterrupts.sectagScEq1ScbEq1 = 1;
   expStatus.cpmRxInterrupts.packetXpnEq0 = 1;
   expStatus.cpmRxInterrupts.pnThreshReached = 1;
   txCpmInt.packetXpnEq0 = 1;
   txCpmInt.pnThreshReached = 1;
   txCpmInt.saNotValid = 1;
   expStatus.cpmTxInterrupts.packetXpnEq0 = 1;
   expStatus.cpmTxInterrupts.pnThreshReached = 1;
   expStatus.cpmTxInterrupts.saNotValid = 1;   
   bbeInt.dfifoOverflow = 1;
   bbeInt.plfifoOverflow = 1;
   expStatus.bbeRxInterrupts.dfifoOverflow = 1;
   expStatus.bbeRxInterrupts.plfifoOverflow = 1;
   expStatus.bbeTxInterrupts.dfifoOverflow = 1;
   expStatus.bbeTxInterrupts.plfifoOverflow = 1;
   pabInt.overflow = 1;
   expStatus.pabRxInterrupts.overflow = 1;
   expStatus.pabTxInterrupts.overflow = 1;
   pexInt.earlyPreemptErr = 1;
   expStatus.pexRxInterrupts.earlyPreemptErr = 1;
   expStatus.pexTxInterrupts.earlyPreemptErr = 1;
   Ra01AccRsMcsCpmTxCpmTxIntIntrRw(device_p, &txCpmInt, RA01_WRITE_OP);
   Ra01AccRsMcsCpmRxCpmRxIntIntrRw(device_p, &rxCpmInt, RA01_WRITE_OP);
   Ra01AccRsMcsPabPabIntIntrRw(device_p, RA01_RS_MCS_PAB_RX_SLAVE, &pabInt, RA01_WRITE_OP);
   Ra01AccRsMcsPabPabIntIntrRw(device_p, RA01_RS_MCS_PAB_TX_SLAVE, &pabInt, RA01_WRITE_OP);
   Ra01AccRsMcsBbeBbeIntIntrRw(device_p, RA01_RS_MCS_BBE_RX_SLAVE, &bbeInt, RA01_WRITE_OP);
   Ra01AccRsMcsBbeBbeIntIntrRw(device_p, RA01_RS_MCS_BBE_TX_SLAVE, &bbeInt, RA01_WRITE_OP);
   Ra01AccRsMcsPexPexIntIntrRw(device_p, RA01_RS_MCS_PEX_RX_SLAVE, &pexInt, RA01_WRITE_OP);
   Ra01AccRsMcsPexPexIntIntrRw(device_p, RA01_RS_MCS_PEX_TX_SLAVE, &pexInt, RA01_WRITE_OP);
   rxCpmInt.sectagVEq1 = 0;
   rxCpmInt.sectagEEq0CEq1 = 0;
   rxCpmInt.sectagSlGte48 = 0;
   rxCpmInt.sectagEsEq1ScEq1 = 0;
   rxCpmInt.sectagScEq1ScbEq1 = 0;
   rxCpmInt.packetXpnEq0 = 0;
   rxCpmInt.pnThreshReached = 0;
   txCpmInt.packetXpnEq0 = 0;
   txCpmInt.pnThreshReached = 0;
   txCpmInt.saNotValid = 0;
   bbeInt.dfifoOverflow = 0;
   bbeInt.plfifoOverflow = 0;
   pabInt.overflow = 0;
   pexInt.earlyPreemptErr = 0;
   Ra01AccRsMcsCpmTxCpmTxIntIntrRw(device_p, &txCpmInt, RA01_WRITE_OP);
   Ra01AccRsMcsCpmRxCpmRxIntIntrRw(device_p, &rxCpmInt, RA01_WRITE_OP);
   Ra01AccRsMcsPabPabIntIntrRw(device_p, RA01_RS_MCS_PAB_RX_SLAVE, &pabInt, RA01_WRITE_OP);
   Ra01AccRsMcsPabPabIntIntrRw(device_p, RA01_RS_MCS_PAB_TX_SLAVE, &pabInt, RA01_WRITE_OP);
   Ra01AccRsMcsBbeBbeIntIntrRw(device_p, RA01_RS_MCS_BBE_RX_SLAVE, &bbeInt, RA01_WRITE_OP);
   Ra01AccRsMcsBbeBbeIntIntrRw(device_p, RA01_RS_MCS_BBE_TX_SLAVE, &bbeInt, RA01_WRITE_OP);
   Ra01AccRsMcsPexPexIntIntrRw(device_p, RA01_RS_MCS_PEX_RX_SLAVE, &pexInt, RA01_WRITE_OP);
   Ra01AccRsMcsPexPexIntIntrRw(device_p, RA01_RS_MCS_PEX_TX_SLAVE, &pexInt, RA01_WRITE_OP);
   MacsecGetInterruptStatus(device_p, &interruptStatus);
   assert(memcmp(&interruptStatus, &expStatus, sizeof(MacsecInterrupts_t)) == 0);
   MacsecClearInterrupts(device_p, interruptStatus);
   MacsecGetInterruptStatus(device_p, &interruptStatus);
   memset(&expStatus, 0, sizeof(MacsecInterrupts_t));
   assert((memcmp(&interruptStatus, &expStatus, sizeof(MacsecInterrupts_t)) == 0));
   #endif
}

void testGetApi(RmsDev_t * device_p, bool testLockout)
{
	test_MacSecGetEnableSc(device_p);
	test_MacsecGetSaPolicy(device_p, testLockout);
	test_MacsecGetNextPn( device_p);
	test_MacsecGetEgressSaMap(device_p);
	test_MacsecGetEnableIngressSa(device_p);
	test_MacsecGetConfiguration(device_p);
}
