/*******************************************************************************
Copyright (C) 2014 - 2022, Marvell International Ltd. and its affiliates
If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.
*******************************************************************************/

/********************************************************************
This file contains MACSec sample functions description, data definitions
used in MACSec samples for the Marvell CUX3610/CUE2610 MACSec enable 
Ethernet PHYs.
********************************************************************/
#ifndef RSMACSEC_SAMPLES_H
#define RSMACSEC_SAMPLES_H
#if C_LINKAGE
#if defined __cplusplus 
    extern "C" { 
#endif 
#endif

typedef enum
{
    MTD_RSMACS_SAMPLE_DEFAULT_MATCH    = 1<<0,
    MTD_RSMACS_SAMPLE_XPN_PN_THRESHOLD = 1<<1,
    MTD_RSMACS_SAMPLE_INTERRUPTS       = 1<<2,
    MTD_RSMACS_SAMPLE_CONTROL_PKTS     = 1<<3,
    MTD_RSMACS_SAMPLE_256_XPN          = 1<<4,
} MTD_RSMACS_SAMPLE_OPTION;

/* Randomly chosen values use in MACSec samples */ 
static uint64_t    sci = 0x0defdeadbeef2ea4;
static unsigned    secY = 0;
static unsigned    sc = 0;
static unsigned    ingressFlowId = 0;
static unsigned    egressFlowId = 0;

static const uint8_t hashkey[2][16] = { 
    { 0x1a, 0x1c, 0xfb, 0xbb, 0x71, 0x6c, 0x96, 0x3c,
    0x13, 0xf6, 0x66, 0x5d, 0x72, 0xdd, 0xa3, 0xf2 } /* 0 */,
    { 0x17, 0x4c, 0x4d, 0x6e, 0x53, 0x1a, 0x01, 0x7b,
    0x0b, 0xc5, 0x24, 0xf4, 0x0f, 0x8f, 0xc0, 0x3e } /* 1 */ };

static const uint8_t sak[2][32] = { 
    { 0x88, 0x21, 0xd5, 0x55, 0x83, 0x8f, 0xdf, 0x2e,
    0xec, 0x9a, 0xf8, 0x08, 0x12, 0x24, 0x66, 0xb1,
    0xb7, 0xe3, 0x55, 0x75, 0xe0, 0x4d, 0xc4, 0x98,
    0x47, 0x16, 0x65, 0xfb, 0x70, 0x22, 0xf4, 0x73 } /* 0 */,
    { 0x95, 0x60, 0xd8, 0xdc, 0xa1, 0x52, 0xeb, 0xc4,
    0x7f, 0xd1, 0x54, 0xc0, 0xdf, 0x3c, 0x65, 0x7b,
    0xcc, 0x4b, 0x9c, 0xd2, 0xe1, 0x29, 0xc6, 0xa2,
    0x0d, 0x9a, 0x62, 0xe8, 0xdc, 0xd0, 0xdd, 0x86 } /* 1 */ };

static const uint8_t salt[2][12] = { 
    { 0x3d, 0xfe, 0x16, 0xfa, 0x6a, 0x87, 0x6a, 0xaa,
    0xbc, 0x48, 0xb5, 0x4d } /* 0 */,
    { 0x02, 0x22, 0x43, 0x43, 0xa4, 0x8e, 0x6f, 0x0e,
    0x57, 0x77, 0x1c, 0x20 } /* 1 */ };

static const uint8_t eghashkey[2][16] = { 
    { 0x1a, 0x1c, 0xfb, 0xbb, 0x71, 0x6c, 0x96, 0x3c,
    0x13, 0xf6, 0x66, 0x5d, 0x72, 0xdd, 0xa3, 0xf2 } /* 0 */,
    { 0x17, 0x4c, 0x4d, 0x6e, 0x53, 0x1a, 0x01, 0x7b,
    0x0b, 0xc5, 0x24, 0xf4, 0x0f, 0x8f, 0xc0, 0x3e } /* 1 */ };

static const uint8_t egsak[2][32] = { 
    { 0x88, 0x21, 0xd5, 0x55, 0x83, 0x8f, 0xdf, 0x2e,
    0xec, 0x9a, 0xf8, 0x08, 0x12, 0x24, 0x66, 0xb1,
    0xb7, 0xe3, 0x55, 0x75, 0xe0, 0x4d, 0xc4, 0x98,
    0x47, 0x16, 0x65, 0xfb, 0x70, 0x22, 0xf4, 0x73 } /* 0 */,
    { 0x95, 0x60, 0xd8, 0xdc, 0xa1, 0x52, 0xeb, 0xc4,
    0x7f, 0xd1, 0x54, 0xc0, 0xdf, 0x3c, 0x65, 0x7b,
    0xcc, 0x4b, 0x9c, 0xd2, 0xe1, 0x29, 0xc6, 0xa2,
    0x0d, 0x9a, 0x62, 0xe8, 0xdc, 0xd0, 0xdd, 0x86 } /* 1 */ };

static const uint8_t egsalt[2][12] = { 
    { 0x3d, 0xfe, 0x16, 0xfa, 0x6a, 0x87, 0x6a, 0xaa,
    0xbc, 0x48, 0xb5, 0x4d } /* 0 */,
    { 0x02, 0x22, 0x43, 0x43, 0xa4, 0x8e, 0x6f, 0x0e,
    0x57, 0x77, 0x1c, 0x20 } /* 1 */ };

/* clear text packet without AAD */
static const uint8_t packet54[54] = {
    0xE2, 0x01, 0x06, 0xD7, 0xCD, 0x0D, \
    0xF0, 0x76, 0x1E, 0x8D, 0xCD, 0x3D, \
    0x08, 0x00, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, \
    0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, \
    0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24, \
    0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, \
    0x2D, 0x2E, 0x2F, 0x30, 0x31, 0x32, 0x33, 0x34, \
    0x00, 0x04 };

/* clear text packet with AAD */
static const uint8_t packet54_A[62] = {
    0xE2, 0x01, 0x06, 0xD7, 0xCD, 0x0D, \
    0xF0, 0x76, 0x1E, 0x8D, 0xCD, 0x3D, \
    0x88, 0xE5, 0x4C, 0x2A, 0x76, 0xD4, 0x57, 0xED, \
    0x08, 0x00, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, \
    0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, \
    0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24, \
    0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, \
    0x2D, 0x2E, 0x2F, 0x30, 0x31, 0x32, 0x33, 0x34, \
    0x00, 0x04 };

/* AES-GCM 128 encrypted packet  with AAD */
static const uint8_t packet54_A_C[62] = {
    0xE2, 0x01, 0x06, 0xD7, 0xCD, 0x0D, \
    0xF0, 0x76, 0x1E, 0x8D, 0xCD, 0x3D, \
    0x88, 0xE5, 0x4C, 0x2A, 0x76, 0xD4, 0x57, 0xED, \
    0x13, 0xB4, 0xC7, 0x2B, 0x38, 0x9D, 0xC5, 0x01, \
    0x8E, 0x72, 0xA1, 0x71, 0xDD, 0x85, 0xA5, 0xD3, \
    0x75, 0x22, 0x74, 0xD3, 0xA0, 0x19, 0xFB, 0xCA, \
    0xED, 0x09, 0xA4, 0x25, 0xCD, 0x9B, 0x2E, 0x1C, \
    0x9B, 0x72, 0xEE, 0xE7, 0xC9, 0xDE, 0x7D, 0x52, \
    0xB3, 0xF3 };
    

/* clear text packet with AAD */
static const uint8_t packet75_A[83] = {
    0x68, 0xF2, 0xE7, 0x76, 0x96, 0xCE, \
    0x7A, 0xE8, 0xE2, 0xCA, 0x4E, 0xC5, \
    0x88, 0xE5, 0x4D, 0x00, 0x2E, 0x58, 0x49, 0x5C, \
    0x08, 0x00, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, \
    0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C, \
    0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24, \
    0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, \
    0x2D, 0x2E, 0x2F, 0x30, 0x31, 0x32, 0x33, 0x34, \
    0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, \
    0x3D, 0x3E, 0x3F, 0x40, 0x41, 0x42, 0x43, 0x44, \
    0x45, 0x46, 0x47, 0x48, 0x49, 0x00, 0x08 };

/* AES-GCM 128  encrypted packet  with AAD */
static const uint8_t packet75_A_C[83] = {
    0x68, 0xF2, 0xE7, 0x76, 0x96, 0xCE, \
    0x7A, 0xE8, 0xE2, 0xCA, 0x4E, 0xC5, \
    0x88, 0xE5, 0x4D, 0x00, 0x2E, 0x58, 0x49, 0x5C, \
    0xC3, 0x1F, 0x53, 0xD9, 0x9E, 0x56, 0x87, 0xF7, \
    0x36, 0x51, 0x19, 0xB8, 0x32, 0xD2, 0xAA, 0xE7, \
    0x07, 0x41, 0xD5, 0x93, 0xF1, 0xF9, 0xE2, 0xAB, \
    0x34, 0x55, 0x77, 0x9B, 0x07, 0x8E, 0xB8, 0xFE, \
    0xAC, 0xDF, 0xEC, 0x1F, 0x8E, 0x3E, 0x52, 0x77, \
    0xF8, 0x18, 0x0B, 0x43, 0x36, 0x1F, 0x65, 0x12, \
    0xAD, 0xB1, 0x6D, 0x2E, 0x38, 0x54, 0x8A, 0x2C, \
    0x71, 0x9D, 0xBA, 0x72, 0x28, 0xD8, 0x40 };
   
/*******************************************************************************
MTD_FUNC MTD_STATUS mtdRSMACSecSampleConfigMACSec
(
    IN MTD_DEV_PTR pDev,
    IN MTD_U16 mdioPort,
    IN MTD_RSMACS_SAMPLE_OPTION sampleOptions
);

Inputs:
   devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
   mdioPort - MDIO port address, 0-31
   sampleOptions - refer to MTD_RSMACS_SAMPLE_OPTION for MACSec feature samples

Outputs:

Returns:
   MTD_OK if query was successful, MTD_FAIL if not.

Description:
   Main MACSec sample to show all the steps needed to configure a port for MACSec operations.
   This mtdRSMACSecSampleConfigMACSec() sample breaks down the call to each individual MACSec 
   component setting the rules and policies as follows:

   - configure and enable device MACSec - mtdRSMACSecConfigEnable()
   - configure the port setting it to be MACSec enabled - mtdRSMACSecSampleConfigPort()
   - configure the TCAM and control packet filter - mtdRSMACSecSampleSetParserControlPkt()
   - configure rules, SecY and SC - mtdRSMACSecSampleEnableRulesSetSC()
       config rules (TCAM flow) and mapped SecY (MacsecWriteTcam/MacsecEnableTcam)
       config SecY and assigned policy to SecY (MacsecWriteSecyPolicy) 
       config and enable SC with assigned SCI and SecY (MacsecSetIngressScCamSci) 
   - configure SA and mapped to SC - mtdRSMACSecSampleSetSA()

Side effects:
   None.

Notes/Warnings:
********************************************************************************/
MTD_FUNC MTD_STATUS mtdRSMACSecSampleConfigMACSec
(
    IN MTD_DEV_PTR pDev,
    IN MTD_U16 mdioPort,
    IN MTD_RSMACS_SAMPLE_OPTION sampleOptions
);

/*******************************************************************************
MTD_FUNC MTD_STATUS mtdRSMACSecSampleConfigPort
(
    IN MTD_DEV_PTR pDev,
    IN MTD_U16 mdioPort,
    IN MTD_RSMACS_SAMPLE_OPTION sampleOptions
);

Inputs:
   devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
   mdioPort - MDIO port address, 0-31
   sampleOptions - refer to MTD_RSMACS_SAMPLE_OPTION for MACSec feature samples

Outputs:

Returns:
   MTD_OK if query was successful, MTD_FAIL if not.

Description:
   MACSec config sample to show how to configure the MACSec port settings call in 
   MacsecSetConfiguration(). The port ingress and egress behaviors including PN thresholds, 
   channel bypass, ingress/egress parameters, etc... in MacsecConfig_t structures. Refer to 
   the MacsecConfig_t and the structure within it to see all the configurable options

Side effects:
   None.

Notes/Warnings:
********************************************************************************/
MTD_FUNC MTD_STATUS mtdRSMACSecSampleConfigPort
(
    IN MTD_DEV_PTR pDev,
    IN MTD_U16 mdioPort,
    IN MTD_RSMACS_SAMPLE_OPTION sampleOptions
);

/*******************************************************************************
MTD_FUNC MTD_STATUS mtdRSMACSecSampleSetParserControlPkt
(
    IN MTD_DEV_PTR pDev,
    IN MTD_U16 mdioPort,
    IN MTD_RSMACS_SAMPLE_OPTION sampleOptions
);

Inputs:
   devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
   mdioPort - MDIO port address, 0-31
   sampleOptions - refer to MTD_RSMACS_SAMPLE_OPTION for MACSec feature samples

Outputs:

Returns:
   MTD_OK if query was successful, MTD_FAIL if not.

Description:
   MACSec parser and control packet sample to show how to configure the packet parsing and 
   control packet in MacsecSetPacketFilter(). Refer to the MTD_RSMACS_SAMPLE_CONTROL_PKTS
   option setting the control packet options where it allow PTP(etype=0x88F7) packet to bypass
   the MACSec.

Side effects:
   None.

Notes/Warnings:
********************************************************************************/
MTD_FUNC MTD_STATUS mtdRSMACSecSampleSetParserControlPkt
(
    IN MTD_DEV_PTR pDev,
    IN MTD_U16 mdioPort,
    IN MTD_RSMACS_SAMPLE_OPTION sampleOptions
);

/*******************************************************************************
MTD_FUNC MTD_STATUS mtdRSMACSecSampleEnableRulesSetSC
(
    IN MTD_DEV_PTR pDev,
    IN MTD_U16 mdioPort,
    IN MTD_RSMACS_SAMPLE_OPTION sampleOptions
);

Inputs:
   devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
   mdioPort - MDIO port address, 0-31
   sampleOptions - refer to MTD_RSMACS_SAMPLE_OPTION for MACSec feature samples

Outputs:

Returns:
   MTD_OK if query was successful, MTD_FAIL if not.

Description:
   MACSec samples to show the following:
    - Configure rules (TCAM flow) and mapped SecY (MacsecWriteTcam/MacsecEnableTcam) 
      TCAM is the 1st parser to direct the appropriate packets to the MACSec path. Only packet
      that meet TCAM rule will allow to into the MACSec path. The data and mask used in both ingress 
      egress determines the packet flow. Refer to the settings for the ingressTcam/egressTcam when 
      calling the MacsecWriteTcam() and MacsecEnableTcam()

    - config SecY and assigned policy to SecY in MacsecWriteSecyPolicy() 
        sets Ingress security entity policy including the following:
          controlledPortEnabled;  
          preSectagAuthEnable;    
          replayProtect;          
          cipher;                 
          confidentialityOffset;  
          stripSectagIcv;         
          validateFrames;         
          replayWindow;  
        sets Egress security entity policy including the following:
          controlledPortEnabled;  
          preSectagAuthEnable;    
          protectFrames;          
          sectagInsertMode;       
          cipher;                 
          confidentialityOffset;  
          sectagOffset;           
          sectagTci;              
          mtu;                    

    Lastly, configure and enable SC with assigned SCI and SecY by calling MacsecSetIngressScCamSci() 

Side effects:
   None.

Notes/Warnings:
********************************************************************************/
MTD_FUNC MTD_STATUS mtdRSMACSecSampleEnableRulesSetSC
(
    IN MTD_DEV_PTR pDev,
    IN MTD_U16 mdioPort,
    IN MTD_RSMACS_SAMPLE_OPTION sampleOptions
);

/*******************************************************************************
MTD_FUNC MTD_STATUS mtdRSMACSecSampleSetSA
(
    IN MTD_DEV_PTR pDev,
    IN MTD_U16 mdioPort,
    IN MTD_RSMACS_SAMPLE_OPTION sampleOptions
);

Inputs:
   devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
   mdioPort - MDIO port address, 0-31
   sampleOptions - refer to MTD_RSMACS_SAMPLE_OPTION for MACSec feature samples

Outputs:

Returns:
   MTD_OK if query was successful, MTD_FAIL if not.

Description:
    This MACSec sample show the steps to configure and set the SAs. After the SA is configured,
    the SA is assigned to the SCI.

    Assigning the SA by setting the SA policy in saPolicy(MacsecSaPolicy_t)
    Calling MacsecSetSaPolicy() to set the SA policy. 
    Enable the SAs for ingress MacsecEnableIngressSa() and egress MacsecEnableEgressSa(). 
    Lastly mapped the SCI to egress SA MacsecSetEgressSaMap() 

Side effects:
   None.

Notes/Warnings:
********************************************************************************/
MTD_FUNC MTD_STATUS mtdRSMACSecSampleSetSA
(
    IN MTD_DEV_PTR pDev,
    IN MTD_U16 mdioPort,
    IN MTD_RSMACS_SAMPLE_OPTION sampleOptions
);

/*******************************************************************************
MTD_FUNC MTD_STATUS mtdRSMACSecSampleRekeyHandler
(
    IN MTD_DEV_PTR pDev,
    IN MTD_U16 mdioPort
);

Inputs:
   devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
   mdioPort - MDIO port address, 0-31

Outputs:

Returns:
   MTD_OK if query was successful, MTD_FAIL if not.

Description:
    MACSec sample to show the calls to add new key and trigger a re-key by setting
    the next packet number(PN)

Side effects:
   None.

Notes/Warnings:
********************************************************************************/
MTD_FUNC MTD_STATUS mtdRSMACSecSampleRekeyHandler
(
    IN MTD_DEV_PTR pDev,
    IN MTD_U16 mdioPort
);

/*******************************************************************************
MTD_FUNC MTD_STATUS mtdRSMACSecSampleGetInterrupts
(
    IN MTD_DEV_PTR pDev,
    IN MTD_U16 mdioPort
);

Inputs:
   devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
   mdioPort - MDIO port address, 0-31

Outputs:

Returns:
   MTD_OK if query was successful, MTD_FAIL if not.

Description:
   MACSec sample to show how the device and MACSec interrupts are set. When
   the lower lever MACSec interrupt gets triggered, it send the interrupt to the
   device level. The sample shows how to detect the interrupt and list the 
   triggered MACSec interrupts.

Side effects:
   None.

Notes/Warnings:
*******************************************************************************/
MTD_FUNC MTD_STATUS mtdRSMACSecSampleGetInterrupts
(
    IN MTD_DEV_PTR pDev,
    IN MTD_U16 mdioPort
);


/*******************************************************************************
MTD_FUNC MTD_STATUS mtdRSMACSecFIPsSample_1_1_GCM_AES128_75B
(
    IN MTD_DEV_PTR pDev,
    IN MTD_U16 mdioPort
);

Inputs:
   devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
   mdioPort - MDIO port address, 0-31

Outputs:

Returns:
   MTD_OK if query was successful, MTD_FAIL if not.

Description:
   FIPs compliant test for:
   www.ieee802.org/1/files/public/docs2011/bn-randall-test-vectors-0511-v1.pdf
   2.8.1 75-byte Packet Encryption Using GCM-AES-128
********************************************************************************/
MTD_FUNC MTD_STATUS mtdRSMACSecFIPsSample_1_1_GCM_AES128_75B
(
    IN MTD_DEV_PTR pDev,
    IN MTD_U16 mdioPort
);

/*******************************************************************************
MTD_FUNC MTD_STATUS mtdRSMACSecFIPsSample_1_2_GCM_AES256_75B
(
    IN MTD_DEV_PTR pDev,
    IN MTD_U16 mdioPort
);

Inputs:
   devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
   mdioPort - MDIO port address, 0-31

Outputs:

Returns:
   MTD_OK if query was successful, MTD_FAIL if not.

Description:
   FIPs compliant test for:
   www.ieee802.org/1/files/public/docs2011/bn-randall-test-vectors-0511-v1.pdf
   2.8.2 75-byte Packet Encryption Using GCM-AES-256 
********************************************************************************/
MTD_FUNC MTD_STATUS mtdRSMACSecFIPsSample_1_2_GCM_AES256_75B
(
    IN MTD_DEV_PTR pDev,
    IN MTD_U16 mdioPort
);

/*******************************************************************************
MTD_FUNC MTD_STATUS mtdRSMACSecFIPsSample_2_1_GCM_AES128_54B
(
    IN MTD_DEV_PTR pDev,
    IN MTD_U16 mdioPort
);

Inputs:
   devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
   mdioPort - MDIO port address, 0-31

Outputs:

Returns:
   MTD_OK if query was successful, MTD_FAIL if not.

Description:
   FIPs compliant test for:
   54-byte Packet Encryption Using GCM-AES-128
********************************************************************************/
MTD_FUNC MTD_STATUS mtdRSMACSecFIPsSample_2_1_GCM_AES128_54B
(
    IN MTD_DEV_PTR pDev,
    IN MTD_U16 mdioPort
);

/*******************************************************************************
MTD_FUNC MTD_STATUS mtdRSMACSecFIPsSample_2_2_GCM_AES128_54B
(
    IN MTD_DEV_PTR pDev,
    IN MTD_U16 mdioPort
);

Inputs:
   devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
   mdioPort - MDIO port address, 0-31

Outputs:

Returns:
   MTD_OK if query was successful, MTD_FAIL if not.

Description:
   FIPs compliant test for:
   54-byte Packet Decryption Using GCM-AES-128 input packet is encrypted,
   output packet is clear text input packet use output from 
   mtdRSMACSecFIPsSample_2_1_GCM_AES128_54B
********************************************************************************/
MTD_FUNC MTD_STATUS mtdRSMACSecFIPsSample_2_2_GCM_AES128_54B
(
    IN MTD_DEV_PTR pDev,
    IN MTD_U16 mdioPort
);

/*******************************************************************************
MTD_FUNC MTD_STATUS mtdRSMACSecFIPsSample_3_1_ECB_AES128_54B
(
    IN MTD_DEV_PTR pDev,
    IN MTD_U16 mdioPort
);

Inputs:
   devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
   mdioPort - MDIO port address, 0-31

Outputs:

Returns:
   MTD_OK if query was successful, MTD_FAIL if not.

Description:
   FIPs compliant test for:
   54-byte Packet Encryption Using AES_ECB 128 no MACsec tag or ICV is involved,
   AAD set to 0Side effects: clear text packet is simply encrypted at output.
********************************************************************************/
MTD_FUNC MTD_STATUS mtdRSMACSecFIPsSample_3_1_ECB_AES128_54B
(
    IN MTD_DEV_PTR pDev,
    IN MTD_U16 mdioPort
);

/*******************************************************************************
MTD_FUNC MTD_STATUS mtdRSMACSecSampleDumpMCSRegs
(
    IN MTD_DEV_PTR pDev,
    IN MTD_U16 mdioPort
);

Inputs:
   devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
   mdioPort - MDIO port address, 0-31

Outputs:

Returns:
   MTD_OK if query was successful, MTD_FAIL if not.

Description:
    MACSec diagnostic sample function to dump all the MACSec related registers for 
    internal diagnostic. This registers dump is for internal engineering debugging only.

Side effects:
   None.

Notes/Warnings:
********************************************************************************/
MTD_FUNC MTD_STATUS mtdRSMACSecSampleDumpMCSRegs
(
    IN MTD_DEV_PTR pDev,
    IN MTD_U16 mdioPort
);

#if C_LINKAGE
#if defined __cplusplus
}
#endif
#endif


#endif

