/*******************************************************************************
Copyright (C) 2014 - 2022, Marvell International Ltd. and its affiliates
If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.
*******************************************************************************/

/********************************************************************
This file contains prototype functions for enabling and configuring the 
MACSec block, reading the statistics, enabling interrupts and various 
operations for the Marvell CUX3610/CUE2610 MACSec enable Ethernet PHYs.
********************************************************************/
#ifndef RSMACSEC_API_H
#define RSMACSEC_API_H
#if C_LINKAGE
#if defined __cplusplus 
    extern "C" { 
#endif 
#endif

/*******************************************************************************
MTD_FUNC MTD_VOID mtdRSMACSecGetAPIVersion
(
    OUT MTD_U8* major,
    OUT MTD_U8* minor,
    OUT MTD_U8* buildID
);

Inputs:
   None

Outputs:
   major - MACSec SDK major version number
   minor - MACSec SDK minor version number
   buildID - MACSec SDK build ID

Returns:
   None

Description:
   Returns the version number of this MACSec SDK. 

Side effects:
   None.

Notes/Warnings:
   None
********************************************************************************/
MTD_FUNC MTD_VOID mtdRSMACSecGetAPIVersion
(
    OUT MTD_U8* major,
    OUT MTD_U8* minor,
    OUT MTD_U8* buildID
);

/*******************************************************************************
MTD_FUNC MTD_STATUS mtdRSMACSecConfigEnable
(
    IN MTD_DEV_PTR pDev,
    IN MTD_U16 mdioPort,
    IN MTD_U32 configOptions
);

Inputs:
   devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
   mdioPort - MDIO port address, 0-31
   configOptions - reserved for future configuration options

Outputs:

Returns:
   MTD_OK if query was successful, MTD_FAIL if not.

Description:
   This API configures the MACSec block and enables the port for MACSec operation. 
   This API must be called at initialization before calling MACSec related APIs.  

Side effects:
   None.

Notes/Warnings:
   Refer to the MACSec samples in mtdRSMACSecSamples.c for more details using this
   API
********************************************************************************/
MTD_FUNC MTD_STATUS mtdRSMACSecConfigEnable
(
    IN MTD_DEV_PTR pDev,
    IN MTD_U16 mdioPort,
    IN MTD_U32 configOptions
);

/*******************************************************************************
MTD_FUNC MTD_STATUS mtdRSMACSecUnloadDriver
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
   Reset the pass-in functions to NULL when the API is unloaded. 

Side effects:
   None.

Notes/Warnings:
*******************************************************************************/
MTD_FUNC MTD_STATUS mtdRSMACSecUnloadDriver
(
    IN MTD_DEV_PTR pDev,
    IN MTD_U16 mdioPort
);

/*******************************************************************************
MTD_FUNC MTD_STATUS mtdGetRxSMCStats
(
    IN MTD_DEV_PTR pDev,
    IN MTD_U16 mdioPort,
    IN MTD_RX_SMC_STATS *stats
);

Inputs:
   devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
   mdioPort - MDIO port address, 0-31
   stats - MIB host side RX statistics structure 

Outputs:

Returns:
   MTD_OK if query was successful, MTD_FAIL if not.

Description:
   Reads the host side RX MIB statistics. Refer to MTD_RX_SMC_STATS for details   

   typedef struct _MTD_RX_SMC_STATS {
    MTD_U64 goodOctets;        // Good frame octets
    MTD_U64 errorOctets;       // Bad frame octets
    MTD_U64 jabber;            // Frame of size > MTU and bad CRC
    MTD_U64 fragment;          // Frame < 48B with bad CRC
    MTD_U64 undersize;         // Frame < 48B with good CRC
    MTD_U64 oversize;          // Frame of size > MTU and good CRC
    MTD_U64 rxError;           // sequence, code, symbol errors
    MTD_U64 crcErrTxUnderrun;  // CRC error
    MTD_U64 rxOverrunBadFC;    // Overrun or bad flow control packet
    MTD_U64 goodPkts;          // number of good packets
    MTD_U64 fcPkts;            // flow control packet
    MTD_U64 broadcast;         // number of broadcast packets
    MTD_U64 multicast;         // number of multicast packets
    MTD_U64 inPAssemblyErr;
    MTD_U64 inPFrames;
    MTD_U64 inPFrags;
    MTD_U64 inPBadFrags;
}MTD_RX_SMC_STATS;

Side effects:
   None.

Notes/Warnings:
*******************************************************************************/
MTD_FUNC MTD_STATUS mtdGetRxSMCStats
(
    IN MTD_DEV_PTR pDev,
    IN MTD_U16 mdioPort,
    IN MTD_RX_SMC_STATS *stats
);

/*******************************************************************************
MTD_FUNC MTD_STATUS mtdGetTxSMCStats
(
    IN MTD_DEV_PTR pDev,
    IN MTD_U16 mdioPort,
    IN MTD_TX_SMC_STATS *stats
);

Inputs:
   devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
   mdioPort - MDIO port address, 0-31
   stats - MIB host side TX statistics structure

Outputs:

Returns:
   MTD_OK if query was successful, MTD_FAIL if not.

Description:
   Reads the host side TX MIB statistics. Refer to MTD_TX_SMC_STATS for details

typedef struct _MTD_SMC_TX_STATS {
    MTD_U64 goodOctets;        // Good frame octets
    MTD_U64 errorOctets;       // Bad frame octets
    MTD_U64 jabber;            // Frame of size > MTU and bad CRC
    MTD_U64 fragment;          // Frame < 48B with bad CRC
    MTD_U64 undersize;         // Frame < 48B with good CRC
    MTD_U64 oversize;          // Frame of size > MTU and good CRC
    MTD_U64 rxError;           // sequence, code, symbol errors
    MTD_U64 crcErrTxUnderrun;  // CRC error
    MTD_U64 rxOverrunBadFC;    // Overrun or bad flow control packet
    MTD_U64 goodPkts;          // number of good packets
    MTD_U64 fcPkts;            // flow control packet
    MTD_U64 broadcast;         // number of broadcast packets
    MTD_U64 multicast;         // number of multicast packets
    MTD_U64 outPFrags;
    MTD_U64 outPFrames;
}MTD_TX_SMC_STATS;

Side effects:
   None.

Notes/Warnings:
*******************************************************************************/
MTD_FUNC MTD_STATUS mtdGetTxSMCStats
(
    IN MTD_DEV_PTR pDev,
    IN MTD_U16 mdioPort,
    IN MTD_TX_SMC_STATS *stats
);

/*******************************************************************************
MTD_FUNC MTD_STATUS mtdGetRxWMCStats
(
    IN MTD_DEV_PTR pDev,
    IN MTD_U16 mdioPort,
    IN MTD_RX_WMC_STATS *stats
);

Inputs:
   devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
   mdioPort - MDIO port address, 0-31
   stats - MIB line side RX statistics structure

Outputs:

Returns:
   MTD_OK if query was successful, MTD_FAIL if not.

Description:
   Reads the line side RX MIB statistics. Refer to MTD_RX_WMC_STATS for details

typedef struct _MTD_RX_WMC_STATS {
    MTD_U64 pkts;               // all packets
    MTD_U64 droppedPkts;        // dropped packets
    MTD_U64 octets;             // all packets octets
    MTD_U64 jabber;             // packet > MTU and bad CRC
    MTD_U64 fragment;           // packet < 64B and bad CRC
    MTD_U64 undersize;          // packet < 64B and good CRC
    MTD_U64 rxError;            // sequence, code, symbol errors
    MTD_U64 crcError;           // CRC error packet
    MTD_U64 fcPkts;             // flow control packet
    MTD_U64 broadcast;          // number of broadcast packets
    MTD_U64 multicast;          // number of multicast packets
    MTD_U64 inPAssemblyErr;
    MTD_U64 inPFrames;
    MTD_U64 inPFrags;
    MTD_U64 inPBadFrags;
}MTD_RX_WMC_STATS;

Side effects:
   None.

Notes/Warnings:
*******************************************************************************/
MTD_FUNC MTD_STATUS mtdGetRxWMCStats
(
    IN MTD_DEV_PTR pDev,
    IN MTD_U16 mdioPort,
    IN MTD_RX_WMC_STATS *stats
);

/*******************************************************************************
MTD_FUNC MTD_STATUS mtdGetTxWMCStats
(
    IN MTD_DEV_PTR pDev,
    IN MTD_U16 mdioPort,
    IN MTD_TX_WMC_STATS *stats
);

Inputs:
   devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
   mdioPort - MDIO port address, 0-31
   stats - MIB line side TX statistics structure

Outputs:

Returns:
   MTD_OK if query was successful, MTD_FAIL if not.

Description:
   Reads the line side TX MIB statistics. Refer to MTD_TX_WMC_STATS for details

typedef struct _MTD_TX_WMC_STATS {
    MTD_U64 pkts;                // all packets
    MTD_U64 droppedPkts;         // dropped packets
    MTD_U64 octets;              // all packets octets
    MTD_U64 jabber;              // packet > MTU and bad CRC
    MTD_U64 fragment;            // packet < 64B and bad CRC
    MTD_U64 undersize;           // packet < 64B and good CRC
    MTD_U64 rxError;             // sequence, code, symbol errors
    MTD_U64 crcError;            // CRC error packet
    MTD_U64 fcPkts;              // flow control packet
    MTD_U64 broadcast;           // number of broadcast packets
    MTD_U64 multicast;           // number of multicast packets
    MTD_U64 outPFrags;
    MTD_U64 outPFrames;
}MTD_TX_WMC_STATS;

Side effects:
   None.

Notes/Warnings:
*******************************************************************************/
MTD_FUNC MTD_STATUS mtdGetTxWMCStats
(
    IN MTD_DEV_PTR pDev,
    IN MTD_U16 mdioPort,
    IN MTD_TX_WMC_STATS *stats
);

/*******************************************************************************
MTD_FUNC MTD_STATUS mtdClearMIBStats
(
    IN MTD_DEV_PTR pDev,
    IN MTD_U16  mdioPort
);

Inputs:
   devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
   mdioPort - MDIO port address, 0-31

Outputs:

Returns:
   MTD_OK if query was successful, MTD_FAIL if not.

Description:
   This API clears and reset all MIB statistic counters on the provided MDIO port

Side effects:
   None.

Notes/Warnings:
*******************************************************************************/
MTD_FUNC MTD_STATUS mtdClearMIBStats
(
    IN MTD_DEV_PTR pDev,
    IN MTD_U16  mdioPort
);

/*******************************************************************************
MTD_FUNC MTD_STATUS mtdClearOnReadMIBStats
(
    IN MTD_DEV_PTR pDev,
    IN MTD_U16  mdioPort,
    IN MTD_BOOL clearOnReadFlag
);

Inputs:
   devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
   mdioPort - MDIO port address, 0-31
   clearOnReadFlag - MTD_TRUE to set clear-on-read; MTD_FALSE if not

Outputs:

Returns:
   MTD_OK if query was successful, MTD_FAIL if not.

Description:
   This API enable or disable the MIB counters clear on read on the provided MDIO port.
   When this is enabled, the counter will be reset to 0s after reading the counter.

Side effects:
   None.

Notes/Warnings:
*******************************************************************************/
MTD_FUNC MTD_STATUS mtdClearOnReadMIBStats
(
    IN MTD_DEV_PTR pDev,
    IN MTD_U16  mdioPort,
    IN MTD_BOOL clearOnReadFlag
);

/*******************************************************************************
MTD_FUNC MTD_STATUS mtdRSMACSecClearStatisticCounters
(
    IN MTD_DEV_PTR pDev,
    IN MTD_U16 mdioPort,
    IN MTD_BOOL clearOnReadFlag
);

Inputs:
   devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
   mdioPort - MDIO port address, 0-31
   clearOnReadFlag - MTD_TRUE to set clear-on-read; MTD_FALSE if not

Outputs:

Returns:
   MTD_OK if query was successful, MTD_FAIL if not.

Description:
   This API enable or disable the MACSec statistic counters clear on read on the 
   provided MDIO port. When this is enabled, the counter will be reset to 0s after
   reading the counter.

Side effects:
   None.

Notes/Warnings:
*******************************************************************************/
MTD_FUNC MTD_STATUS mtdRSMACSecClearStatisticCounters
(
    IN MTD_DEV_PTR pDev,
    IN MTD_U16 mdioPort,
    IN MTD_BOOL clearOnReadFlag
);

/*******************************************************************************
MTD_FUNC MTD_STATUS mtdRSMACRxSecYStatisticCounters
(
    IN MTD_DEV_PTR pDev,
    IN MTD_U16 mdioPort,
    IN MTD_U16 index,
    IN MacsecRxSecyCounters_t* rxSecYCounters
);

Inputs:
   devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
   mdioPort - MDIO port address, 0-31
   index - SecY index, 0-31
   rxSecYCounters -  Security Entity SecY RX ingress statistic counters structure

Outputs:

Returns:
   MTD_OK if query was successful, MTD_FAIL if not.

Description:
   Reads the Security Entity RX ingress statistic counters. This API calls the MACSec
   statistic API and provide a logging output for easy reference. If logging 
   is not needed, refer to this API to call the MACSec statistic API to retrieve
   the counter structure.  Refer to MacsecRxSecyCounters_t for details

typedef struct {
   Ra01IfRsMcsCseRxMemIfinunctloctets_t           Ifinunctloctets;          
   Ra01IfRsMcsCseRxMemIfinctloctets_t             Ifinctloctets;            
   Ra01IfRsMcsCseRxMemIfinunctlucpkts_t           Ifinunctlucpkts;          
   Ra01IfRsMcsCseRxMemIfinunctlmcpkts_t           Ifinunctlmcpkts;          
   Ra01IfRsMcsCseRxMemIfinunctlbcpkts_t           Ifinunctlbcpkts;          
   Ra01IfRsMcsCseRxMemIfinctlucpkts_t             Ifinctlucpkts;            
   Ra01IfRsMcsCseRxMemIfinctlmcpkts_t             Ifinctlmcpkts;            
   Ra01IfRsMcsCseRxMemIfinctlbcpkts_t             Ifinctlbcpkts;            
   Ra01IfRsMcsCseRxMemInpktssecyuntaggedornotag_t Inpktssecyuntaggedornotag;
   Ra01IfRsMcsCseRxMemInpktssecybadtag_t          Inpktssecybadtag;         
   Ra01IfRsMcsCseRxMemInpktssecyctl_t             Inpktssecyctl;            
   Ra01IfRsMcsCseRxMemInpktssecytaggedctl_t       Inpktssecytaggedctl;      
   Ra01IfRsMcsCseRxMemInpktssecyunknownsci_t      Inpktssecyunknownsci;     
   Ra01IfRsMcsCseRxMemInpktssecynosci_t           Inpktssecynosci;          
   Ra01IfRsMcsCseRxMemInpktsctrlportdisabled_t    Inpktsctrlportdisabled;   
} MacsecRxSecyCounters_t;

Side effects:
   None.

Notes/Warnings:
*******************************************************************************/
MTD_FUNC MTD_STATUS mtdRSMACRxSecYStatisticCounters
(
    IN MTD_DEV_PTR pDev,
    IN MTD_U16 mdioPort,
    IN MTD_U16 index,
    IN MacsecRxSecyCounters_t* rxSecYCounters
);

/*******************************************************************************
MTD_FUNC MTD_STATUS mtdRSMACRxSCStatisticCounters
(
    IN MTD_DEV_PTR pDev,
    IN MTD_U16 mdioPort,
    IN MTD_U16 index,
    IN MacsecRxScCounters_t* rxSCCounters
);

Inputs:
   devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
   mdioPort - MDIO port address, 0-31
   index - SC index, 0-31
   rxSCCounters - ingress Secure Channel statistic counters structure

Outputs:

Returns:
   MTD_OK if query was successful, MTD_FAIL if not.

Description:
   Reads the Secure Channel RX ingress statistic counters. This API calls the MACSec
   statistic API and provide a logging output for easy reference. If logging
   is not needed, refer to this API to call the MACSec statistic API to retrieve
   the counter structure. Refer to MacsecRxScCounters_t for details

typedef struct {
   Ra01IfRsMcsCseRxMemInoctetsscvalidate_t    Inoctetsscvalidate;   
   Ra01IfRsMcsCseRxMemInoctetsscdecrypted_t   Inoctetsscdecrypted;  
   Ra01IfRsMcsCseRxMemInpktsscunchecked_t     Inpktsscunchecked;    
   Ra01IfRsMcsCseRxMemInpktssclateordelayed_t Inpktssclateordelayed;
   Ra01IfRsMcsCseRxMemInpktssccamhit_t        Inpktssccamhit;       
} MacsecRxScCounters_t;

Side effects:
   None.

Notes/Warnings:
*******************************************************************************/
MTD_FUNC MTD_STATUS mtdRSMACRxSCStatisticCounters
(
    IN MTD_DEV_PTR pDev,
    IN MTD_U16 mdioPort,
    IN MTD_U16 index,
    IN MacsecRxScCounters_t* rxSCCounters
);

/*******************************************************************************
MTD_FUNC MTD_STATUS mtdRSMACRxSAStatisticCounters
(
    IN MTD_DEV_PTR pDev,
    IN MTD_U16 mdioPort,
    IN MTD_U16 index,
    IN MacsecRxSaCounters_t* rxSACounters
);

Inputs:
   devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
   mdioPort - MDIO port address, 0-31
   index - SA index, 0-63
   rxSACounters - ingress Secure Associations statistic counters structure

Outputs:

Returns:
   MTD_OK if query was successful, MTD_FAIL if not.

Description:
   Reads the Secure Associations RX ingress statistic counters. This API calls the MACSec
   statistic API and provide a logging output for easy reference. If logging
   is not needed, refer to this API to call the MACSec statistic API to retrieve
   the counter structure. Refer to MacsecRxSaCounters_t for details

typedef struct {
   Ra01IfRsMcsCseRxMemInpktssaok_t              Inpktssaok;             
   Ra01IfRsMcsCseRxMemInpktssainvalid_t         Inpktssainvalid;        
   Ra01IfRsMcsCseRxMemInpktssanotvalid_t        Inpktssanotvalid;       
   Ra01IfRsMcsCseRxMemInpktssaunusedsa_t        Inpktssaunusedsa;       
   Ra01IfRsMcsCseRxMemInpktssanotusingsaerror_t Inpktssanotusingsaerror;
} MacsecRxSaCounters_t;

Side effects:
   None.

Notes/Warnings:
*******************************************************************************/
MTD_FUNC MTD_STATUS mtdRSMACRxSAStatisticCounters
(
    IN MTD_DEV_PTR pDev,
    IN MTD_U16 mdioPort,
    IN MTD_U16 index,
    IN MacsecRxSaCounters_t* rxSACounters
);

/*******************************************************************************
MTD_FUNC MTD_STATUS mtdRSMACRxPortStatisticCounters
(
    IN MTD_DEV_PTR pDev,
    IN MTD_U16 mdioPort,
    IN MTD_U16 index,
    IN MacsecRxPortCounters_t* rxPortCounters
);

Inputs:
   devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
   mdioPort - MDIO port address, 0-31
   index - 0; single port only
   rxPortCounters - ingress Port statistic counters structure

Outputs:

Returns:
   MTD_OK if query was successful, MTD_FAIL if not.

Description:
   Reads the MACSec Port ingress statistic counters. This API calls the MACSec
   statistic API and provide a logging output for easy reference. If logging
   is not needed, refer to this API to call the MACSec statistic API to retrieve
   the counter structure. Refer to MacsecRxPortCounters_t for details

typedef struct {
   Ra01IfRsMcsCseRxMemInpktsflowidtcammiss_t  Inpktsflowidtcammiss; 
   Ra01IfRsMcsCseRxMemInpktsparseerr_t        Inpktsparseerr;       
   Ra01IfRsMcsCseRxMemInpktsearlypreempterr_t Inpktsearlypreempterr;
} MacsecRxPortCounters_t;

Side effects:
   None.

Notes/Warnings:
*******************************************************************************/
MTD_FUNC MTD_STATUS mtdRSMACRxPortStatisticCounters
(
    IN MTD_DEV_PTR pDev,
    IN MTD_U16 mdioPort,
    IN MTD_U16 index,
    IN MacsecRxPortCounters_t* rxPortCounters
);

/*******************************************************************************
MTD_FUNC MTD_STATUS mtdRSMACRxFlowStatisticCounters
(
    IN MTD_DEV_PTR pDev,
    IN MTD_U16 mdioPort,
    IN MTD_U16 index,
    IN MacsecRxFlowCounters_t* rxFlowCounters
);

Inputs:
   devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
   mdioPort - MDIO port address, 0-31
   index - flow index, 0-31;
   rxFlowCounters - ingress flow statistic counters structure

Outputs:

Returns:
   MTD_OK if query was successful, MTD_FAIL if not.

Description:
   Reads the MACSec RX ingress flow statistic counters. This API calls the MACSec
   statistic API and provide a logging output for easy reference. If logging
   is not needed, refer to this API to call the MACSec statistic API to retrieve
   the counter structure. Refer to MacsecRxFlowCounters_t for details

typedef struct {
   Ra01IfRsMcsCseRxMemInpktsflowidtcamhit_t Inpktsflowidtcamhit;
} MacsecRxFlowCounters_t;

Side effects:
   None.

Notes/Warnings:
*******************************************************************************/
MTD_FUNC MTD_STATUS mtdRSMACRxFlowStatisticCounters
(
    IN MTD_DEV_PTR pDev,
    IN MTD_U16 mdioPort,
    IN MTD_U16 index,
    IN MacsecRxFlowCounters_t* rxFlowCounters
);

/*******************************************************************************
MTD_FUNC MTD_STATUS mtdRSMACTxSecYStatisticCounters
(
    IN MTD_DEV_PTR pDev,
    IN MTD_U16 mdioPort,
    IN MTD_U16 index,
    IN MacsecTxSecyCounters_t* txSecYCounters
);

Inputs:
   devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
   mdioPort - MDIO port address, 0-31
   index - SecY index, 0-31
   txSecYCounters -  Security Entity SecY TX egress statistic counters structure

Outputs:

Returns:
   MTD_OK if query was successful, MTD_FAIL if not.

Description:
   Reads the Security Entity egress statistic counters. This API calls the MACSec
   statistic API and provide a logging output for easy reference. If logging
   is not needed, refer to this API to call the MACSec statistic API to retrieve
   the counter structure.  Refer to MacsecTxSecyCounters_t for details

typedef struct {
   Ra01IfRsMcsCseTxMemIfoutcommonoctets_t       Ifoutcommonoctets;      
   Ra01IfRsMcsCseTxMemIfoutunctloctets_t        Ifoutunctloctets;       
   Ra01IfRsMcsCseTxMemIfoutctloctets_t          Ifoutctloctets;         
   Ra01IfRsMcsCseTxMemIfoutunctlucpkts_t        Ifoutunctlucpkts;       
   Ra01IfRsMcsCseTxMemIfoutunctlmcpkts_t        Ifoutunctlmcpkts;       
   Ra01IfRsMcsCseTxMemIfoutunctlbcpkts_t        Ifoutunctlbcpkts;       
   Ra01IfRsMcsCseTxMemIfoutctlucpkts_t          Ifoutctlucpkts;         
   Ra01IfRsMcsCseTxMemIfoutctlmcpkts_t          Ifoutctlmcpkts;         
   Ra01IfRsMcsCseTxMemIfoutctlbcpkts_t          Ifoutctlbcpkts;         
   Ra01IfRsMcsCseTxMemOutpktssecyuntagged_t     Outpktssecyuntagged;    
   Ra01IfRsMcsCseTxMemOutpktssecytoolong_t      Outpktssecytoolong;     
   Ra01IfRsMcsCseTxMemOutpktssecynoactivesa_t   Outpktssecynoactivesa;  
   Ra01IfRsMcsCseTxMemOutpktsctrlportdisabled_t Outpktsctrlportdisabled;
} MacsecTxSecyCounters_t;

Side effects:
   None.

Notes/Warnings:
*******************************************************************************/
MTD_FUNC MTD_STATUS mtdRSMACTxSecYStatisticCounters
(
    IN MTD_DEV_PTR pDev,
    IN MTD_U16 mdioPort,
    IN MTD_U16 index,
    IN MacsecTxSecyCounters_t* txSecYCounters
);

/*******************************************************************************
MTD_FUNC MTD_STATUS mtdRSMACTxSCStatisticCounters
(
    IN MTD_DEV_PTR pDev,
    IN MTD_U16 mdioPort,
    IN MTD_U16 index,
    IN MacsecTxScCounters_t* txSCCounters
);

Inputs:
   devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
   mdioPort - MDIO port address, 0-31
   index - SC index, 0-31
   txSCCounters - Egress Secure Channel statistic counters structure

Outputs:

Returns:
   MTD_OK if query was successful, MTD_FAIL if not.

Description:
   Reads the Secure Channel egress statistic counters. This API calls the MACSec
   statistic API and provide a logging output for easy reference. If logging
   is not needed, refer to this API to call the MACSec statistic API to retrieve
   the counter structure. Refer to MacsecTxScCounters_t for details

typedef struct {
   Ra01IfRsMcsCseTxMemOutoctetsscprotected_t Outoctetsscprotected;
   Ra01IfRsMcsCseTxMemOutoctetsscencrypted_t Outoctetsscencrypted;
} MacsecTxScCounters_t;

Side effects:
   None.

Notes/Warnings:
*******************************************************************************/
MTD_FUNC MTD_STATUS mtdRSMACTxSCStatisticCounters
(
    IN MTD_DEV_PTR pDev,
    IN MTD_U16 mdioPort,
    IN MTD_U16 index,
    IN MacsecTxScCounters_t* txSCCounters
);

/*******************************************************************************
MTD_FUNC MTD_STATUS mtdRSMACTxSAStatisticCounters
(
    IN MTD_DEV_PTR pDev,
    IN MTD_U16 mdioPort,
    IN MTD_U16 index,
    IN MacsecTxSaCounters_t* txSaCounters
);

Inputs:
   devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
   mdioPort - MDIO port address, 0-31
   index - SA index, 0-63
   txSaCounters - Egress Secure Associations statistic counters structure

Outputs:

Returns:
   MTD_OK if query was successful, MTD_FAIL if not.

Description:
   Reads the Secure Associations egress statistic counters. This API calls the MACSec
   statistic API and provide a logging output for easy reference. If logging
   is not needed, refer to this API to call the MACSec statistic API to retrieve
   the counter structure. Refer to MacsecTxSaCounters_t for details

typedef struct {
   Ra01IfRsMcsCseTxMemOutpktssaprotected_t Outpktssaprotected; //!< @sa Ra01IfRsMcsCseTxMemOutpktssaprotected_t
   Ra01IfRsMcsCseTxMemOutpktssaencrypted_t Outpktssaencrypted; //!< @sa Ra01IfRsMcsCseTxMemOutpktssaencrypted_t
} MacsecTxSaCounters_t;

Side effects:
   None.

Notes/Warnings:
*******************************************************************************/
MTD_FUNC MTD_STATUS mtdRSMACTxSAStatisticCounters
(
    IN MTD_DEV_PTR pDev,
    IN MTD_U16 mdioPort,
    IN MTD_U16 index,
    IN MacsecTxSaCounters_t* txSaCounters
);

/*******************************************************************************
MTD_FUNC MTD_STATUS mtdRSMACTxPortStatisticCounters
(
    IN MTD_DEV_PTR pDev,
    IN MTD_U16 mdioPort,
    IN MTD_U16 index,
    IN MacsecTxPortCounters_t* txPortCounters
);

Inputs:
   devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
   mdioPort - MDIO port address, 0-31
   index - 0; single port only
   txPortCounters - Egress Port statistic counters structure

Outputs:

Returns:
   MTD_OK if query was successful, MTD_FAIL if not.

Description:
   Reads the MACSec Port egress statistic counters. This API calls the MACSec
   statistic API and provide a logging output for easy reference. If logging
   is not needed, refer to this API to call the MACSec statistic API to retrieve
   the counter structure. Refer to MacsecTxPortCounters_t for details

typedef struct {
   Ra01IfRsMcsCseTxMemOutpktsflowidtcammiss_t     Outpktsflowidtcammiss;    
   Ra01IfRsMcsCseTxMemOutpktsparseerr_t           Outpktsparseerr;          
   Ra01IfRsMcsCseTxMemOutpktssectaginsertionerr_t Outpktssectaginsertionerr;
   Ra01IfRsMcsCseTxMemOutpktsearlypreempterr_t    Outpktsearlypreempterr;   
} MacsecTxPortCounters_t;

Side effects:
   None.

Notes/Warnings:
*******************************************************************************/
MTD_FUNC MTD_STATUS mtdRSMACTxPortStatisticCounters
(
    IN MTD_DEV_PTR pDev,
    IN MTD_U16 mdioPort,
    IN MTD_U16 index,
    IN MacsecTxPortCounters_t* txPortCounters
);

/*******************************************************************************
MTD_FUNC MTD_STATUS mtdRSMACTxFlowStatisticCounters
(
    IN MTD_DEV_PTR pDev,
    IN MTD_U16 mdioPort,
    IN MTD_U16 index,
    IN MacsecTxFlowCounters_t* txFlowCounters
);

Inputs:
   devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
   mdioPort - MDIO port address, 0-31
   index - flow index, 0-31;
   txFlowCounters - Egress flow statistic counters structure

Outputs:

Returns:
   MTD_OK if query was successful, MTD_FAIL if not.

Description:
   Reads the MACSec egress flow statistic counters. This API calls the MACSec
   statistic API and provide a logging output for easy reference. If logging
   is not needed, refer to this API to call the MACSec statistic API to retrieve
   the counter structure. Refer to MacsecTxFlowCounters_t for details

typedef struct {
   Ra01IfRsMcsCseTxMemOutpktsflowidtcamhit_t Outpktsflowidtcamhit;
} MacsecTxFlowCounters_t;

Side effects:
   None.

Notes/Warnings:
*******************************************************************************/
MTD_FUNC MTD_STATUS mtdRSMACTxFlowStatisticCounters
(
    IN MTD_DEV_PTR pDev,
    IN MTD_U16 mdioPort,
    IN MTD_U16 index,
    IN MacsecTxFlowCounters_t* txFlowCounters
);

/*******************************************************************************
MTD_FUNC MTD_STATUS mtdRSMACInterruptEnable
(
    IN MTD_DEV_PTR pDev,
    IN MTD_U16 mdioPort,
    IN MTD_BOOL interruptEnable
);

Inputs:
   devPtr - pointer to MTD_DEV initialized by mtdLoadDriver() call
   mdioPort - MDIO port address, 0-31
   interruptEnable - MTD_TRUE to enable device top level and MACSec interrupts;
                     otherwise MTD_FALSE

Outputs:

Returns:
   MTD_OK if query was successful, MTD_FAIL if not.

Description:
   Sets the appropriate device and MACSec block interrupt. Refer to MACSec sample
   MTD_RSMACS_SAMPLE_INTERRUPTS option in mtdRSMACSecSampleConfigMACSec and
   mtdRSMACSecSampleGetInterrupts for more MACSec interrupt details 
   
Side effects:
   None.

Notes/Warnings:
*******************************************************************************/
MTD_FUNC MTD_STATUS mtdRSMACInterruptEnable
(
    IN MTD_DEV_PTR pDev,
    IN MTD_U16 mdioPort,
    IN MTD_BOOL interruptEnable
);


#if C_LINKAGE
#if defined __cplusplus
}
#endif
#endif


#endif

