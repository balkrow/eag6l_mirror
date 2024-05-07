/*******************************************************************************
Copyright (C) 2014 - 2023, Marvell International Ltd. and its affiliates
If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.
*******************************************************************************/

/********************************************************************
This file contains sample functions code for the Marvell 8X35X0, 
88E25X0, CUX36X0 and CUE26X0 ethernet PHYs.
********************************************************************/
#if MTD_TU_PTP

#include "mtdFeatures.h"
#include "mtdApiTypes.h"
#include "mtdHwCntl.h"
#include "mtdAPI.h"
#include "mtdTuPtpSample.h"
#include "mtdTuPtpTypes.h"
#include "mtdTuPtp.h"
#include "mtdTuPtpInternal.h"
#include <time.h>

#define MTD_TOD_LOAD_DELAY (50000000)
#define MTD_PTP_CLOCK_250M
#define MTD_LOW_LATENCY_MODE

#ifdef MTD_PTP_CLOCK_250M
#define MTD_TAI_CLOCK_PERIOD 4
#define MTD_TOD_COMP_PER_ONE_PPM 0x218E
#elif defined MTD_PTP_CLOCK_125M
#define MTD_TAI_CLOCK_PERIOD 8
#define MTD_TOD_COMP_PER_ONE_PPM 0x431E
#endif


static MTD_STATUS mtdSamplePtpSetCurrentTime
(
    IN MTD_DEV_PTR pDev, 
    IN MTD_U16 mdioPort,
    IN MTD_U8 timeArrayIndex, 
    IN MTD_U8 domainNumber, 
    OUT MTD_TIME_INFO *timeInfo
)
{
    MTD_STATUS status = MTD_OK;
    MTD_TU_PTP_TIME_ARRAY timeArray;
    MTD_TIME_INFO tempTime;

    tempTime.nanoseconds = (timeInfo->nanoseconds + MTD_TOD_LOAD_DELAY) % 1000000000;
    tempTime.seconds = timeInfo->seconds + (timeInfo->nanoseconds + MTD_TOD_LOAD_DELAY) / 1000000000;

    /* Initialize the time array */
    timeArray.clkActive = MTD_TRUE;
    timeArray.domainNumber = domainNumber;
    timeArray.todSecondsHigh = (tempTime.seconds >> 32) & 0xffff;
    timeArray.todSecondsLow = tempTime.seconds & 0xffffffff;
    timeArray.todNanoseconds = tempTime.nanoseconds;
    timeArray.Nanoseconds1722High = 0;
    timeArray.Nanoseconds1722Low = 0;
    timeArray.todCompensation = 0;
    /* Get the current Global time and add a delay for loadPoint, the value is incremented once by TAI_CLOCK_PERIOD ns, so the loadPoint value = TOD_LOAD_DELAY / TAI_CLOCK_PERIOD */
    mtdTuPTPGetPTPGlobalTime(pDev, mdioPort, &timeArray.todLoadPoint);
    timeArray.todLoadPoint = timeArray.todLoadPoint + MTD_TOD_LOAD_DELAY / MTD_TAI_CLOCK_PERIOD;

    /* Only one TOD operation can be executing at one time, so the busy bit must be zero before any TOD operation */
    MTD_ATTEMPT(mtdTuPTPWaitGlobalBitSC(pDev, mdioPort, MTD_TUPTP_WAITTYPE_TOD)); /* Wait for ToDBusy cleared */

    MTD_ATTEMPT(mtdTuPTPTODStoreOperation(pDev, mdioPort, MTD_TU_PTP_TOD_STORE_ALL, timeArrayIndex, &timeArray));

    return status;
}


/*
 * Sample code to set device to work at PTP IEEE 1588v2 mode 
 * This example uses two-step PTP and Boundary Clock settings
 *
 */
MTD_STATUS mtdSamplePtpSet1588v2
(
    IN MTD_DEV_PTR pDev, 
    IN MTD_U16 mdioPort
)
{
    MTD_STATUS status = MTD_OK;
    MTD_TU_PTP_MODE ptpMode;
    MTD_TU_PTP_STEP ptpStep;
    MTD_TU_PTP_TOD_STORE_OPERATION storeOp = MTD_TU_PTP_TOD_STORE_ALL;
    MTD_U8 selTimeArrayIndex = 0;           /* use timeArray 0 in sample */
    MTD_U16 setHwAccel = 1;                 /* Both Ingress and Egress PTP hardware acceleration are enabled */
    MTD_U16 msgTSEnable;
    MTD_U16 transSpec;
    MTD_U8 domainNumber;
    time_t rawtime;
    MTD_TIME_INFO timeInfo;

	/* Init PTP */
    MTD_U16 enablePTPBlock = MTD_ENABLE;
    MTD_BOOL doSwReset = MTD_TRUE;

    MTD_ATTEMPT(mtdTuPTPConfigPTPBlock(pDev, mdioPort, enablePTPBlock, doSwReset));

    /* Set time array 0 PTP to  Boundary Clock mode(BC), two-step */
    ptpMode = MTD_PTP_BOUNDARY_CLK;
    ptpStep = MTD_PTP_TWO_STEP;
    transSpec = MTD_TU_PTP_IEEE_1588;
    domainNumber = 0;

 	/* Load TOD(time of day) to current time */
    time(&rawtime);
    timeInfo.seconds = rawtime;
    timeInfo.nanoseconds = 0;
    MTD_ATTEMPT(mtdSamplePtpSetCurrentTime(pDev, mdioPort, selTimeArrayIndex, domainNumber, &timeInfo));

    MTD_ATTEMPT(mtdTuPTPSetPTPMode(pDev, mdioPort, ptpMode));
    MTD_ATTEMPT(mtdTuPTPSetOneStep(pDev, mdioPort, ptpStep));

    msgTSEnable = 0xFFFF;   /* enable matching all message type */
    MTD_ATTEMPT(mtdTuPTPSetMsgTSEnable(pDev, mdioPort, msgTSEnable));

    MTD_ATTEMPT(mtdTuPTPSetCfgTransSpec(pDev, mdioPort, transSpec));
    MTD_ATTEMPT(mtdTuPTPSetCfgHWAccel(pDev, mdioPort, setHwAccel));

    return MTD_OK;
}

/*
 *  Sample code to enable PTP block for One-step mode with hardware acceleration on.
 *  MultiPTPSync mode need to be enabled to sync PTP global timer in multiple devices
 *  environment.
 *
 */
MTD_STATUS mtdSampleConfTuPTP
(
    IN MTD_DEV_PTR pDev,
    IN MTD_U16 mdioPort
)
{
    MTD_U16 enablePTPBlock = MTD_ENABLE;
    MTD_BOOL doSwReset = MTD_TRUE;
    time_t rawtime;
    MTD_TIME_INFO timeInfo;
    MTD_U16 igrMeanDelay, egrPDAsymm;

    MTD_U8 selTimeArrayIndex = 0;           /* use timeArray 0 in sample; support only 1 time array */
    MTD_TU_PTP_TOD_STORE_OPERATION storeOp = MTD_TU_PTP_TOD_STORE_ALL;

    MTD_U16 setPtpMode = MTD_PTP_PEER_TO_PEER_CLK;  /* Peer to Peer Transparent Clock */
    MTD_U16 setOneStep = MTD_PTP_ONE_STEP;  /* Hardware accelerate using One Step frame formats */
    MTD_U16 setGrandMaster = 0x0;           /* Hardware accelerate with this device being the Grand Master */
    MTD_U16 setAltScheme = 0x0;             /* pDelay response behavior (refer to function prototype description) */

    MTD_U16 setPtpEType = 0x88F7;           /* PTP packet on Layer 2 expect the packet Ethernet type to be 0x88F7 */
    MTD_U16 setHwAccel = 1;                 /* Both Ingress and Egress PTP hardware acceleration are enabled */
    MTD_U16 setTransSpec = MTD_TU_PTP_IEEE_1588;  /* MTD_TU_PTP_IEEE_1588 or MTD_TU_PTP_IEEE_802_1AS */
    MTD_U16 msgTSEnable = 0xFFFF;           /* enable matching all message type */
    MTD_U8 domainNumber = 0;                /* domain number matches the PTP frame domain number */

    if (MTD_IS_X35X0_E2540_DEVICE(pDev) || MTD_IS_X36X0_BASE(pDev->deviceId))
    {
        MTD_ATTEMPT(mtdTuPTPConfigPTPBlock(pDev, mdioPort, enablePTPBlock, doSwReset));

	    /* Load TOD(time of day) to current time */
        time(&rawtime);
        timeInfo.seconds = rawtime;
        timeInfo.nanoseconds = 0;
        MTD_ATTEMPT(mtdSamplePtpSetCurrentTime(pDev, mdioPort, selTimeArrayIndex, domainNumber, &timeInfo));

        /* give it a bit of time for the global timer to reach load point before continue */
        MTD_ATTEMPT(mtdWait(pDev, 500));

#if 0   /* sample calls to trigger GPIO for TAI signals */
        /* GPIO to select TAI */
        MTD_ATTEMPT(mtdHwSetPhyRegField(pDev, mdioPort, 0x1F, 0xF014, 7, 1, 1));
        MTD_ATTEMPT(mtdHwSetPhyRegField(pDev, mdioPort, 0x1F, 0xF014, 7, 1, 1));

        /* enable TOD TAI signals to chip GPIO connection */
        MTD_ATTEMPT(mtdTuPTPSetTaiGPIO(pDev, mdioPort, 1));

        /* TAI global config */
        MTD_ATTEMPT(mtdTuPTPSetTAIGlobalCfg(pDev, mdioPort, MTD_TU_PTP_TAI_EVENT_CAP_OW, 1));
        MTD_ATTEMPT(mtdTuPTPSetTAIGlobalCfg(pDev, mdioPort, MTD_TU_PTP_TAI_EVENT_CNT_START, 1));
        MTD_ATTEMPT(mtdTuPTPSetTAIGlobalCfg(pDev, mdioPort, MTD_TU_PTP_TAI_EVENT_CAP_INTR, 1));
        MTD_ATTEMPT(mtdTuPTPSetTAIGlobalCfg(pDev, mdioPort, MTD_TU_PTP_TAI_MULT_SYNC, 1));
        
        /* sets the PTP Time Application Interface trigger generation time amount */
        MTD_ATTEMPT(mtdTuPTPSetTrigGenAmt(pDev, mdioPort, 0));
#endif

        MTD_ATTEMPT(mtdTuPTPSetPTPMode(pDev, mdioPort, setPtpMode));
        MTD_ATTEMPT(mtdTuPTPSetOneStep(pDev, mdioPort, setOneStep));
        if (MTD_IS_X36X0_BASE(pDev->deviceId))
        {
            MTD_ATTEMPT(mtdTuPTPSetOneStepDomain(pDev, mdioPort, 1));
            MTD_ATTEMPT(mtdTuPTPSetInternalTime(pDev, mdioPort, 0));
        }
        MTD_ATTEMPT(mtdTuPTPSetGrandMaster(pDev, mdioPort, setGrandMaster));
        MTD_ATTEMPT(mtdTuPTPSetAltScheme(pDev, mdioPort, setAltScheme));
        MTD_ATTEMPT(mtdTuPTPSetEtherType(pDev, mdioPort, setPtpEType));
        MTD_ATTEMPT(mtdTuPTPSetMsgTSEnable(pDev, mdioPort, msgTSEnable));
        MTD_ATTEMPT(mtdTuPTPSetCfgTransSpec(pDev, mdioPort, setTransSpec));
        MTD_ATTEMPT(mtdTuPTPSetCfgDisableTSpecCheck(pDev, mdioPort, 1));
        
        MTD_ATTEMPT(mtdTuPTPSetCfgHWAccel(pDev, mdioPort, setHwAccel));

        egrPDAsymm = 0x8608; /* 100M delay sample in ns */
        MTD_ATTEMPT(mtdTuPTPEgrPathDelayAsymSet(pDev, mdioPort, egrPDAsymm));

        igrMeanDelay = 0x653; /* 100M delay sample in ns */
        MTD_ATTEMPT(mtdTuPTPSetIgrMeanPathDelay(pDev, mdioPort, igrMeanDelay));
        MTD_ATTEMPT(mtdTuPTPSetIgrPathDelayAsym(pDev, mdioPort, igrMeanDelay));

        /* mtdTuPTPGetTimeStamp() can be called to get MTD_TU_PTP_TS_STATUS, which including the timestamp */
        /* ARR0_TIME, ARR1_TIME, and DEP_TIME */
        /* saved in the register and the valid status. */
        /* In this case, all timestamp register should be invalid */
    }
    else
    {
        MTD_DBG_ERROR("mtdSampleConfTuPtp: Device doesn't support TuPtp.\n");
        return MTD_FAIL;
    }

    /* After these procedures, if you are sending a Sync-up message(0x0), */
    /* the reserved field(17th - 20th octets) of the PTP packet will be updated by the timestamp */
    /* Both of the arrival timestamp registers are invalid */

    /* Other examples: */

    /* One-Step, Egress Path, Hw Accel On, Sync-up message 0x0 */
    /* The timestamp fields of the PTP packet(starting at 35th octet) will be updated by the calculated timestamp */
    /* Departure timestamp register DEP_TIME is invalid */

    /* One-Step, Hw Accel Off, Sync-up message 0x0 */
    /* The reserved field and timestamp fields of the PTP packet will not be updated. */
    /* Timestamp will be placed into either arrival timestamp register ARR0_TIME or ARR1_TIME, */
    /* Call mtdTuPTPSetTimeStampArrivalPtr() to select which register to go based on the type of message */

    /* Two-Step, Ingress path, Hw Accel On, Delay_Resp message 0x9, PTP Ver.2 */
    /* Neither PTP packet nor the timestamp registers will be updated/modified. */
    /* Arrival timestamps registers are invalid */

    /* Two-Step, Ingress path, Hw Accel On, Delay_Resp message 0x9, PTP Ver.1 */
    /* Current PTP does not support Hw Accel for version 1 PTP packet */
    /* Thus, the arrival timestamp register ARR0_TIME or ARR1_TIME will be updated */

    /* Two-Step, Egress path, Hw Accel On, sent Sync-up message 0x0 then Follow-up message 0x8 */
    /* For the Sync-up message, neither PTP packet nor the timestamp registers will be updated. */
    /* Instead, the timestamp is saved in an internal register. */
    /* For the Follow-up message, the timestamp fields of the PTP packet will be updated by the saved timestamps */
    /* when Sync-up message is sent. The departure timestamp register is invalid */

    return MTD_OK;
}

/*
 *  Sample code to get current ToD from a time array
 *
 */
MTD_STATUS mtdSamplePtpGetCurrentTime
(
    IN MTD_DEV_PTR pDev, 
    IN MTD_U16 mdioPort,
    OUT MTD_TIME_INFO *timeInfo
)
{
	MTD_TU_PTP_TIME_ARRAY timeArray;
    MTD_U8 timeArrayIndex = 0;

    MTD_ATTEMPT(mtdTuPTPTODCaptureAll(pDev, mdioPort, timeArrayIndex, &timeArray));

    timeInfo->nanoseconds = timeArray.todNanoseconds;
    timeInfo->seconds = ((timeArray.todSecondsHigh * 1LL) << 32) | timeArray.todSecondsLow;

    return MTD_OK;
}

/*
 * Departure timestamp is a 32 bits global time counter 
 * With HW acceleration enabled and ToD loaded:
 * For Delay_Req and Pdelay_Req frame, the departure timestamp is placed at PTP Departure Time Register
 * The arrival timestamp is put at 4-byte reserved field in PTP header, not at PTP arrival Time Register
 * This example is assumed that ToD has been loaded and PTP mode has been configured 
 *
 */
MTD_STATUS mtdSamplePtpGetDepartureTime
(
    IN MTD_DEV_PTR pDev, 
    IN MTD_U16 mdioPort,
    OUT MTD_TU_PTP_TS_STATUS *depTimeStamp
)
{
    /* If Delay_Req or Pdelay_Req frame egress PHY, we can get departure timestamp 
       from PTP Departure Time Register as below steps shown */
    MTD_ATTEMPT(mtdTuPTPGetTimeStamp(pDev, mdioPort, MTD_TU_PTP_DEP_TIME, depTimeStamp));

    MTD_DBG_INFO("mtdSamplePtpGetDepartureTime: Valid: %d\n", depTimeStamp->isValid);
    MTD_DBG_INFO("mtdSamplePtpGetDepartureTime: Interrupt Status: %d\n", depTimeStamp->status);
    MTD_DBG_INFO("mtdSamplePtpGetDepartureTime: SeqId: %d\n", depTimeStamp->ptpSeqId);
    MTD_DBG_INFO("mtdSamplePtpGetDepartureTime: timestamp value: 0x%x\n", depTimeStamp->timeStamped);

	if (depTimeStamp->isValid != MTD_TRUE)
	{
		MTD_DBG_INFO("mtdSamplePtpGetDepartureTime Valid incorrect\n");
	}

    return MTD_OK;
}


#endif  /* MTD_TU_PTP */
