/*******************************************************************************
Copyright (C) 2014 - 2023, Marvell International Ltd. and its affiliates
If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.
*******************************************************************************/

/********************************************************************
This file contains sample function prototypes, macros and definitions 
for the Marvell 8X35X0, 88E25X0, CUX36X0 and CUE26X0 ethernet PHYs.
********************************************************************/
#ifndef MTD_TUPTP_SAMPLE_H
#define MTD_TUPTP_SAMPLE_H

#if C_LINKAGE
#if defined __cplusplus 
    extern "C" { 
#endif 
#endif

#if MTD_TU_PTP
        
typedef struct {
    MTD_U64 seconds;
    MTD_U32 nanoseconds;
} MTD_TIME_INFO;

/******************************************************************************
MTD_STATUS mtdSamplePtpSet1588v2
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 mdioPort
)

 Inputs:
    devPtr - allocated memory for the device structure
    mdioPort - MDIO port address, 0-31

 Outputs:
    None

 Returns:
    MTD_OK if successfully
    MTD_FAIL if failed

 Description:
    Sample code to set device to work at PTP IEEE 1588v2 mode 
    This example uses two-step PTP and Boundary Clock settings

 Side effects:
    None

 Notes/Warnings:
    None
*******************************************************************************/
MTD_FUNC MTD_STATUS mtdSamplePtpSet1588v2
(
    IN MTD_DEV_PTR pDev,
    IN MTD_U16 mdioPort
);

/******************************************************************************
MTD_STATUS mtdSampleConfTuPTP
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 mdioPort
)

 Inputs:
    devPtr - allocated memory for the device structure
    mdioPort - MDIO port address, 0-31

 Outputs:
    None

 Returns:
    MTD_OK if successfully
    MTD_FAIL if failed

 Description:
    Sample code to enable PTP block for One-step mode with hardware acceleration on.
    MultiPTPSync mode need to be enabled to sync PTP global timer in multiple devices
    environment.

    This is an configuration example for One-Step, Ingress Path, Hw Accel On. See
    the expecting behaviors of other examples at the end of this function.

 Side effects:
    None

 Notes/Warnings:
    None
*******************************************************************************/
MTD_FUNC MTD_STATUS mtdSampleConfTuPTP
(
    IN MTD_DEV_PTR devPtr,
    IN MTD_U16 mdioPort
);

/******************************************************************************
MTD_FUNC MTD_STATUS mtdSamplePtpGetCurrentTime
(
    IN MTD_DEV_PTR pDev, 
    IN MTD_U16 mdioPort,
    OUT MTD_TIME_INFO *timeInfo
);

 Inputs:
    devPtr - allocated memory for the device structure
    mdioPort - MDIO port address, 0-31

 Outputs:
    timeInfo

 Returns:
    MTD_OK if successfully
    MTD_FAIL if failed

 Description:
    Sample code to read the current ToD from a time array

 Side effects:
    None

 Notes/Warnings:
    None
*******************************************************************************/
MTD_FUNC MTD_STATUS mtdSamplePtpGetCurrentTime
(
    IN MTD_DEV_PTR pDev, 
    IN MTD_U16 mdioPort,
    OUT MTD_TIME_INFO *timeInfo
);

/******************************************************************************
MTD_FUNC MTD_STATUS mtdSamplePtpGetDepartureTime
(
    IN MTD_DEV_PTR pDev, 
    IN MTD_U16 mdioPort,
    OUT MTD_TU_PTP_TS_STATUS *depTimeStamp
);

 Inputs:
    devPtr - allocated memory for the device structure
    mdioPort - MDIO port address, 0-31

 Outputs:
    timeInfo

 Returns:
    MTD_OK if successfully
    MTD_FAIL if failed

 Description:
    Sample code to get the departure 32 bits timestamp from global time counter 
    With HW acceleration enabled and ToD loaded:
    For Delay_Req and Pdelay_Req frame, the departure timestamp is placed at PTP 
    Departure Time Register
    The arrival timestamp is put at 4-byte reserved field in PTP header, not at 
    PTP arrival Time Register
    This example is assumed that ToD has been loaded and PTP mode has been 
    configured.

 Side effects:
    None

 Notes/Warnings:
    None
*******************************************************************************/
MTD_FUNC MTD_STATUS mtdSamplePtpGetDepartureTime
(
    IN MTD_DEV_PTR pDev, 
    IN MTD_U16 mdioPort,
    OUT MTD_TU_PTP_TS_STATUS *depTimeStamp
);

#endif /* MTD_TU_PTP */

#endif /* MTD_TUPTP_SAMPLE_H */
