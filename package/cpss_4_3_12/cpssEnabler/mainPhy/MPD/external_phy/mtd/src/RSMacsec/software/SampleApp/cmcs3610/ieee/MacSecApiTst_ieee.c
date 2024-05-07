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
#include "MacSecApiTst_ieee.h"



void test_MacsecReadTcam(RmsDev_t * device_p)
{
	unsigned idx;

	for (idx = 0; idx< MACSEC_NUM_SC ; idx++) {
	    MacsecFlowCfg_t origVal, testVal;
	    MacsecReadTcam( device_p, MACSEC_INGRESS, idx, &origVal );

	    memcpy(&testVal,&origVal,sizeof(testVal));

	    MacsecWriteTcam( device_p, MACSEC_INGRESS, idx, &origVal );
	    MacsecReadTcam( device_p, MACSEC_INGRESS, idx, &origVal );

	    assert( memcmp(&testVal,&origVal,sizeof(testVal)) == 0 );

            MacsecReadTcam( device_p, MACSEC_EGRESS, idx, &origVal );

	    memcpy(&testVal,&origVal,sizeof(testVal));

	    MacsecWriteTcam( device_p, MACSEC_EGRESS, idx, &origVal );
	    MacsecReadTcam( device_p, MACSEC_EGRESS, idx, &origVal );

	    assert( memcmp(&testVal,&origVal,sizeof(testVal)) == 0 );

    }
}

void test_MacsecReadSecyPolicy(RmsDev_t * device_p)
{
	unsigned idx;

	for (idx = 0; idx< MACSEC_NUM_SC ; idx++) {
	    MacsecSecyPolicyTable_t origVal, testVal;
	    MacsecReadSecyPolicy( device_p, MACSEC_INGRESS, idx, &origVal );

	    memcpy(&testVal,&origVal,sizeof(testVal));

	     MacsecWriteSecyPolicy( device_p, MACSEC_INGRESS, idx, &origVal );
	     MacsecReadSecyPolicy( device_p, MACSEC_INGRESS, idx, &origVal );

	     assert( memcmp(&testVal,&origVal,sizeof(testVal)) == 0 );
	    MacsecReadSecyPolicy( device_p, MACSEC_EGRESS, idx, &origVal );

	    memcpy(&testVal,&origVal,sizeof(testVal));

	     MacsecWriteSecyPolicy( device_p, MACSEC_EGRESS, idx, &origVal );
	     MacsecReadSecyPolicy( device_p, MACSEC_EGRESS, idx, &origVal );

	     assert( memcmp(&testVal,&origVal,sizeof(testVal)) == 0 );


    }
}

void test_MacsecGetIngressScCamSci(RmsDev_t * device_p)
{
  unsigned idx ;


	for (idx = 0; idx< MACSEC_NUM_SC ; idx++) {
	    bool origVal,testVal;
            uint64_t sci, testSci;
	    unsigned secY, testSecy;
	    MacsecGetIngressScCamSci( device_p, idx, &secY, &sci, &origVal );

            memcpy(&testSecy,&secY,sizeof(testSecy));
	    memcpy(&testSci,&sci,sizeof(testSci));
            memcpy(&testVal,&origVal,sizeof(testVal));

	    MacsecSetIngressScCamSci( device_p, idx, secY,sci, origVal );
	    MacsecGetIngressScCamSci( device_p, idx, &secY, &sci, &origVal );

            assert( memcmp(&testSecy,&secY,sizeof(testSecy)) == 0 );
            assert( memcmp(&testSci,&sci,sizeof(testSci)) == 0 );
            assert( memcmp(&testVal,&origVal,sizeof(testVal)) == 0);

    }
}

void test_MacsecGetPacketFilter(RmsDev_t * device_p)
{
	MacsecPacketFilter_t origVal, testVal;
	MacsecGetPacketFilter( device_p, MACSEC_EGRESS, &origVal );

	memcpy(&testVal,&origVal,sizeof(testVal));

	MacsecSetPacketFilter( device_p, MACSEC_EGRESS, &origVal );
	MacsecGetPacketFilter( device_p, MACSEC_EGRESS, &origVal );

	assert( memcmp(&testVal,&origVal,sizeof(testVal)) == 0 );
	
	memset(&origVal, 0, sizeof(origVal));
	memset(&testVal, 0, sizeof(testVal));
	MacsecGetPacketFilter( device_p, MACSEC_INGRESS, &origVal );

	memcpy(&testVal,&origVal,sizeof(testVal));

	MacsecSetPacketFilter( device_p, MACSEC_INGRESS, &origVal );
	MacsecGetPacketFilter( device_p, MACSEC_INGRESS, &origVal );

	assert( memcmp(&testVal,&origVal,sizeof(testVal)) == 0 );
}

void testGetIeeeApi(RmsDev_t * device_p)
{
    test_MacsecReadTcam(device_p);
    test_MacsecReadSecyPolicy(device_p);
    test_MacsecGetIngressScCamSci(device_p);
    test_MacsecGetPacketFilter(device_p);
}
