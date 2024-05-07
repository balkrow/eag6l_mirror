/*! 
 * @file macsec_api.h 
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
// * $Rev: 
// * $Date: 
// ******************************************************************************

#ifndef __MACSEC_API_H__
#define __MACSEC_API_H__

#include <stdbool.h>
#include "macsec_cfg.h"

/*! @brief Align a value
 *
 * Make sure that X is on the same alignment as Y. Handy for arrays
 * that need to contain Y sized elements.
 *
 * e.g. Storing 12 bytes in uint64_t registers (8 byte aligned):
 *    uint8_t array[ALIGN(12, sizeof(uint64_t))]  // === uint8_t array[16]
 */
#define ALIGN(X, Y)         ((( X + Y ) - 1 ) & ~( Y - 1 ))

#include "macsec_hw.h"

/* Some Doxygen housekeeping
 */
/*!
 * @defgroup filter_cfg_fns Packet Filter
 * The packet filter is the first step in the Macsec processing pipeline.
 * Its purpose is to parse incoming packets and extract relevant fields
 * that will then be fed to the classification engine for further processing.
 * The packet filter relies on a set of configurations depending on the the layer to be parsed
 *    - Layer 2: sets of {etype,size} for different tags
 *    - Layer 3: sets of IANA protocol numbers
 *    - Layer 4: sets of port numbers
 * The set of configurations is fully documented in ::MacsecPacketFilter_t
 *
 * In additions to parsing and extraction, the filter also performs rudimentary control classification based on a set of rules
 * The parser will extract the ethernet destination MAC address as well as the final etype from the packet, and match
 * those against the rules. Any match results the packet getting marked as a control packet. The set of rules is documented in ::MacsecCtrlRules_t
 *
 * @defgroup port_if_fns General Configuration 
 *
 * This is a catch-all section for all configuration not convered by the other sections including (but not limited to)
 *    - per-channel bypass
 *    - RX header insertion 
 *    - padding addition
 *    - PN thresholds
 *    - statistics configuration
 *
 * @defgroup flow_if_fns Flow Configuration 
 *
 * Extracted packet data from the filter is forwarded to the classification engine. The first stage
 * of this engine is to determine what flow-ID the packet belongs to (using a HW associated array, or TCAM).
 * The Flow-ID is then used to determine the Security Entity (SecY). 
 * All configurations related to this process are included in this section
 *
 * @defgroup secy_if_fns Security Entity (SecY) Management 
 *
 * This section contains all configuration related to the Security Entity. The largest portion of this is the 
 * policy (or processing rules) associated to the SecY. 
 *
 * @defgroup sc_if_fns Secure Channel (SC) Management 
 *
 * @defgroup sa_if_fns Secure Association (SA) Management
 *
 * @defgroup int_if_fns Interrupt Interface 
 *
 * @defgroup FIPS_interface The interface functions related to FIPS management
 *
 * @defgroup counter_fns Statistics Counters 
 *
 * @defgroup Fine_Tuning_Interface Extra functions to set individual parameters
 * @defgroup Debug_Interface Function for debugging.
 *
 * @defgroup sc_timer_fns Secure Channel Expiry Timers
 *
 * The Macsec IP provides a timer based mechanism for expiring secure channels. Essentially,
 * the core maintains timers per SC and when those timers exceed programmable thresholds, interrupts fire
 * to alert SW. In the transmit path, the timer expiry mechanism can potentially invalidate the active SA 
 * and trigger an automatic switch to the next active SA. 
 *
 */

/*!
 * @brief number of EthTypes for control packet matching
 *
 * @ingroup base_fns
 */
#define MACSEC_FILTER_ETYPE_RULES           8

/*!
 * @brief number of DA matching rules supported in PEX.
 *
 * @ingroup base_fns
 */
#define MACSEC_FILTER_DA_RULES              8

/*!
 * @brief number of DA range matching rules supported in PEX.
 *
 * @ingroup base_fns
 */
#define MACSEC_FILTER_DA_RANGE_RULES        4

/*!
 * @brief number of combo (DA + EthType) matching rules supported in PEX.
 *
 * @ingroup base_fns
 */
#define MACSEC_FILTER_COMBO_RULES           4

/*!
 * @brief number of VLAN tags supported in PEX.
 *
 * @ingroup base_fns
 */
#define MACSEC_VLAN_TAGS                    8

/*!
 * @brief number of PTP tags supported in PEX.
 *
 * @ingroup base_fns
 */
#define MACSEC_PTP_TAGS                     2

/*!
 * @brief number of MPLS tags supported in PEX.
 *
 * @ingroup base_fns
 */
#define MACSEC_MPLS_TAGS                    4

/*!
 * @brief number of PTP rules supported in PEX.
 *
 * @ingroup base_fns
 */
#define MACSEC_NUM_PTP_RULES                2

/*!
 * @brief number of label select rules supported in CPEX
 *
 * @ingroup base_fns
 */
#define MACSEC_NUM_LABEL_SELECTS            7

/*!
 * @brief number of secure channels supported by the CPM
 *
 * @ingroup base_fns
 */
#define MACSEC_NUM_SC                       32

/*!
 * @brief number of association numbers possible
 *
 * @ingroup base_fns
 */
#define MACSEC_NUM_AN                       4

/*!
 * @brief number of secure associations supported by the CPM
 *
 * @ingroup base_fns
 */
#define MACSEC_NUM_SA                       64

/*!
 * @brief number of flops at the PAB output (used for fixed latency calculation)
 *
 * @ingroup base_fns
 */
#define MACSEC_PAB_OUTPUT_FLOP_COUNT        5

/*!
 * @brief number of flops at the PEX output (used for fixed latency calculation)
 *
 * @ingroup base_fns
 */
#define MACSEC_PEX_OUTPUT_FLOP_COUNT        2

/*!
 * @brief number of bits in a FIPS block
 *
 * @ingroup base_fns
 */
#define MACSEC_FIPS_BLOCK_SIZE              128

/*!
 * @brief Software API version number
 *
 * Stores the current software API version:
 *    major   : Change when binary compatibility changes
 *    minor   : Add functionality, backwards compatible.
 *    release : Bug fixes, binary compatible.
 *
 * @sa MacsecGetCapabilities
 *
 * @ingroup base_fns
 */
typedef struct {
    uint8_t             major;      //!< Major version number
    uint8_t             minor;      //!< Minor version number
    uint16_t            release;    //!< Release version
} MacsecVersion_t;

//------------------------------------------------------------------------------
// General configuration
//------------------------------------------------------------------------------
/*!
 * @brief Configuration information
 *
 * This details the MACsec configuration.  This structure is used to return the
 * number of entries in the different internal MACsec tables.
 *
 * @sa MacsecGetCapabilities
 *
 * @ingroup port_if_fns
 */
typedef struct {
    unsigned            ingressMaxSecy;                  //!< Maximum number of ingress SecYs supported
    unsigned            ingressFlowIdTcamTableSize;      //!< TCAM entries in the Ingress Flow-ID TCAM Table and Flow-ID TCAM to SecY Map
    unsigned            ingressSecyPolicyTableSize;      //!< SecY entries in the Ingress SecY Policy Table
    unsigned            ingressScCamLookupKeyTableSize;  //!< Entries in the SC CAM Lookup Key Table
    unsigned            ingressAnPerSc;                  //!< Supported number of Associations per SC table entry
    unsigned            ingressScAnToSaMapTableSize;     //!< SA Indices in the Ingress SC+AN to SA Map Table
    unsigned            ingressSaPolicyTableSize;        //!< Policy entries in the Ingress SA Policy Table, PNs in the PN Table.

    unsigned            egressMaxSecy;                   //!< Maximum number of egress SecYs supported
    unsigned            egressFlowIdTcamTableSize;       //!< TCAM entries in the Egress Flow-ID TCAM Table and Flow-ID TCAM to SecY Map
    unsigned            egressSecyPolicyTableSize;       //!< SecY entries in the Egress SecY Policy Table
    unsigned            egressSaPolicyTableSize;         //!< Entries in the Egress SecY to SA Map Table
    unsigned            egressSecyToSaMapTableSize;      //!< Egress SA Policy Table size and PN Entries.
    MacsecVersion_t     version;                         //!< Software API version
} MacsecCapabilities_t;

//------------------------------------------------------------------------------
// General types
//------------------------------------------------------------------------------

/*!
 * @brief Flow direction.
 *
 * The flow direction is needed to determine which block to address. Most
 * blocks in the system have a set of ingress and egress registers.
 *
 * @ingroup base_fns
 */
typedef enum {
    MACSEC_INGRESS = 1,                 //!< Ingress block
    MACSEC_EGRESS  = 2,                 //!< Egress block
    MACSEC_RX      = MACSEC_INGRESS,    //!< RX is an alias for ingress
    MACSEC_TX      = MACSEC_EGRESS,     //!< TX is an alias for egress
} MacsecDirection_t;

/*!
 * @brief Port Rates
 *
 * Lists the supported port rates for the device 
 *
 * @ingroup base_fns
 */
typedef enum {
  MACSEC_10G,
  MACSEC_5G,
  MACSEC_2_5G,
  MACSEC_1G,
  MACSEC_100M,
  MACSEC_10M
} MacsecPortRate_t;

/*!
 * @brief Transmit specific parameters
 *
 * These parameters are valid on the transmit ports.
 *
 * @sa MacsecSetConfiguration
 * @sa MacsecGetConfiguration
 *
 * @ingroup port_if_fns
 */
typedef struct {
    bool				valid;					//!< true if these parameters are valid, false to ignore this section

    /*!
     * @brief Miscellaneous TX options
    */
    struct MacsecTxOptParam_s {
        bool				            setOptParameters;			          //!< Set to true to set the parameters in this section. false to leave as default
        uint16_t			            sectagEtype[MACSEC_PORT_COUNT];   //!< MACSec SecTAG e-type for insertion on corresponding port
        uint8_t			            fixedOffsetAdjust;			       /**< SecTAG fixed offset mode adjustment to compensate for Tx_MCS header size. 
                                                                           This value will get added to the sectag_offset from the policy when a 4B tx header 
                                                                           is present and sectag insertion mode is set to 1. Default value of 4 should be used. */
        Ra01RsMcsPabIncludeTagCnt_t	includeTagCount;	                /**< Include tags preceding the sectag in the octet counts... \n 
                                                                           0: octet count includes mac-da + mac-sa + MSDU \n 
                                                                           1: octet count includes all bytes before sectag + MSDU. The optional TX header is not included (if present) \n 
                                                                           note: for untagged packets, this configuration has no effect on the octet counts*/
        Ra01RsMcsBbePaddingCtl_t	   padding;				                //!< BBE Padding control
    } opt;
    bool     chBypass[MACSEC_CHANNELS_PER_PORT]; //!< CH is in bypass
    uint32_t pnThreshold;                        //!< The threshold for a 32-bit PN interrupt
    uint64_t xpnThreshold;                       //!< The threshold for a 64-bit PN interrupt
    bool     enableTxTrunc;                      /**< Enable trunaction. When set, the packet will be truncated and
                                                      eop aborted if its final size exceeds the MTU as specified in the policy. 
                                                      If clearted, the packet will get eop aborted if its size exceeds the MTU
                                                      specified in the policy, but the packet will not be truncated */
    bool     statsClearOnRead;                   //!< When true stats registers will be cleared on read
    bool     earlyPreemptFilterEnable;           /**< Enable Early Preempt Filtering. When set to 1, Early Preempt Filtering will be active.
                                                      The purpose of the early preempt filter is to catch packets whose first
                                                      fragment is too short and terminate them. If this filter is disabled
                                                      care must be taken to not generate first fragments that are too short as this could
                                                      result in deadlock */
} MacsecTxParam_t;

/*!
 * @brief Recieve specific parameters
 *
 * These parameters are valid on the recieve ports.
 *
 * @sa MacsecSetConfiguration
 * @sa MacsecGetConfiguration
 *
 * @ingroup port_if_fns
 */
typedef struct {
    bool					valid;					//!< true if these parameters are valid, false to ignore this section

    /*!
     * @brief Miscellaneous RX options
    */
    struct MacsecRxOptParam_s {
        bool					setOptParameters;		       	//!< Set to true to set the parameters in this section. false to leave as default
        Ra01RsMcsPabIncludeTagCnt_t	  includeTagCount;	/**< Include tags preceding the sectag in the octet counts... \n 
                                                              0: octet count includes mac-da + mac-sa + MSDU \n 
                                                              1: octet count includes all bytes before sectag + MSDU \n 
                                                              note: for untagged packets, this configuration has no effect on the octet counts*/
        Ra01RsMcsBbePaddingCtl_t      padding;           //!< BBE Padding control
        Ra01RsMcsCpmRxRxDefaultSci_t  rxDefaultSci;      //!< Default SCI for tagged packets with sectag.tci.sc == 0 and sectag.tci.es == 0
        Ra01RsMcsCpmRxRxEtype_t       rxSectagEtype;     //!< Ingress e-type pattern. This is used in the classifier to validate the sectag received from the parser and should match the value programmed for the parser
        Ra01RsMcsCpmRxRxMcsBasePort_t rxMcsBasePort;     //!< Rx-MCS header base port
        Ra01RsMcsCpmRxCpuPuntEnable_t cpuPuntEnable;     //!< PUNT enable flags
    } opt; 
    bool     chBypass[MACSEC_CHANNELS_PER_PORT];         //!< Channel is in bypass
    uint32_t pnThreshold;                                //!< The threshold for a 32-bit PN interrupt
    uint64_t xpnThreshold;                               //!< The threshold for a 64-bit PN interrupt
    bool     pnTableUpdate;                              //!< When true updates to PN cannot be less than the current PN
    bool     statsClearOnRead;                           //!< When true stats registers will be cleared on read
    bool     earlyPreemptFilterEnable;                   /**< Enable Early Preempt Filtering. When set to 1, Early Preempt Filtering will be active.
                                                              The purpose of the early preempt filter is to catch packets whose first
                                                              fragment is too short and terminate them. If this filter is disabled
                                                              care must be taken to not generate first fragments that are too short as this could
                                                              result in deadlock */

    /*!
     * @brief RX header insertion options
    */
    struct MacsecRxHdrIns_s {
        bool                                 valid;     //!< Set to true to write these values to the device
        Ra01RsMcsCpmRxEnableRxmcsInsertion_t insertion; //!< RxMCS Insertion rules
    } rxmcsInsertion[MACSEC_PORT_COUNT];//!< Per port enabled for inserting 4B Rx-MCS headers

    /*!
     * @brief sectag rule enables
     */
    struct MacsecRxSectagRuleChkEnb_s {
       bool    sectagVEq1Ena;        //!< When set enables the SecTag Rule checker to deem the SecTag errored if (SecTag.V == 1) 
       bool    sectagEsEq1ScEq1Ena;  //!< When set enables the SecTag Rule checker to deem the SecTag errored if (SecTag.ES == 1 && SecTag.SC == 1).
       bool    sectagScEq1ScbEq1Ena; //!< When set enables the SecTag Rule checker to deem the SecTag errored if (SecTag.SC == 1 && SecTag.SCB == 1).
       bool    sectagPnEq0Ena;       //!< When set enables the SecTag Rule checker to deem the SecTag errored if (SecTag.PN == 0).
    } sectagRuleChkEnable;

} MacsecRxParam_t;

/*!
 * @brief Port symmetric parameters
 *
 * Port parameters that are uniform for ingress and egress.
 *
 * @ingroup port_if_fns
 */
typedef struct {
    bool                     valid;      //!< Table entry is valid
    Ra01RsMcsTopPortConfig_t portConfig; //!< 3 bits. Number of packet header bytes to parse, 0 = 128
} PortSymmetricParameters_t;

/*!
 * @brief MACsec per-port configuration settings
 *
 * @ingroup port_if_fns
 */
typedef struct {
    MacsecPortMode_t            portMode;                       //!< The port mode for the part.
    MacsecRxParam_t             ingress;                        //!< All parameters related to ingress
    MacsecTxParam_t             egress;                         //!< All parameters related to egress
    PortSymmetricParameters_t   symmetric[MACSEC_PORT_COUNT];   //!< Symmetric per port parameters
} MacsecConfig_t;


/*!
 * @brief SA policy data
 *
 * This union is for convenience when calling function that set SA policy.
 *
 * @sa MacsecConfigureSaPolicy
 *
 * @sa MCS_REG_CPM_RX_SA_PLCY_MEM
 * @sa MCS_REG_CPM_TX_SA_PLCY_MEM
 *
 * @ingroup sa_if_fns
 */
typedef union {
    Ra01IfRsMcsCpmRxSaPlcy_t     ingress;    //!< Ingress SA policy
    Ra01IfRsMcsCpmTxSaPlcy_t     egress;     //!< Egress SA policy
} MacsecSaPolicy_t;

/*!
 * @struct Ra01IfRsMcsCpmTxSaPlcy_t
 *
 * @sa MacsecTxRekeyHandler
 * @sa MacsecSetSaPolicy
 * @sa MacsecGetSaPolicy
 *
 * @ingroup sa_if_fns
 *
 * @brief Contains all elements of the egress SA policy table 
 *
 * @var Ra01IfRsMcsCpmTxSaPlcy_t::sectagAn
 * 2b SecTag Association Number (AN)
 *
 * @var Ra01IfRsMcsCpmTxSaPlcy_t::ssci
 * 32b SSCI value: Short Secure Channel Identifier, used in XPN ciphers to compute the 96bit IV
 *
 * @var Ra01IfRsMcsCpmTxSaPlcy_t::hashkey
 * 128b Hash Key used for authentication. This is derived by performing a 128/256 bit AES-ECB 
 * block encryption of an all 0s block with the SAK: E(SAK, 128'h0). 
 * The cipher text result of this operation is the value that needs to be configured in this entry.
 *
 * @var Ra01IfRsMcsCpmTxSaPlcy_t::sak 
 * 256b SAK: Defines the encryption key to be used to decrypt this packet. 
 * The lower 128 bits are used for 128-bit ciphers. The cipher is based on the policy
 * for this SA 
 *
 * @var Ra01IfRsMcsCpmTxSaPlcy_t::salt
 * 96b Salt value: Salt value used in XPN ciphers to compute the 96b ICV
 */

/*!
 * @struct Ra01IfRsMcsCpmRxSaPlcy_t
 * @brief Contains all elements of the ingress SA policy table 
 *
 * @sa MacsecSetSaPolicy
 * @sa MacsecGetSaPolicy
 *
 * @ingroup sa_if_fns
 *
 * @var Ra01IfRsMcsCpmRxSaPlcy_t::ssci
 * 32b SSCI value: Short Secure Channel Identifier, used in XPN ciphers to compute the 96bit IV
 *
 * @var Ra01IfRsMcsCpmRxSaPlcy_t::hashkey
 * 128b Hash Key used for authentication. This is derived by performing a 128/256 bit AES-ECB 
 * block encryption of an all 0s block with the SAK: E(SAK, 128'h0). 
 * The cipher text result of this operation is the value that needs to be configured in this entry.
 *
 * @var Ra01IfRsMcsCpmRxSaPlcy_t::sak 
 * 256b SAK: Defines the encryption key to be used to decrypt this packet. 
 * The lower 128 bits are used for 128-bit ciphers. The cipher is based on the policy
 * for this SA 
 *
 * @var Ra01IfRsMcsCpmRxSaPlcy_t::salt
 * 96b Salt value: Salt value used in XPN ciphers to compute the 96b ICV
 */

typedef Ra01IfRsMcsCpmTxSaPlcy_t MacsecTxSaPolicy_t;
typedef Ra01IfRsMcsCpmRxSaPlcy_t MacsecRxSaPolicy_t;

/*!
 * @brief Egress SecY to SA Map Table entry.
 *
 * This struct is an entry in the Egress SecY to SA Map table.
 *
 * @sa MCS_REG_CPM_TX_SA_MAP_MEM
 *
 * @ingroup sa_if_fns
 */
typedef struct {
    bool      autoRekeyEnable; /**< Enable auto-rekey. When auto rekey is enabled, 
                                    HW will toggle the txSaActive bit when the PN 
                                    for the current SA exceeds the PN threshold and disable (saIndex*Vld == 0)
                                    the inactive SA. So if txSaActive is set to 1 and saIndex1 is set to 40,
                                    then HW will detect when PnTable[40] == pn_thresh, set txSaActive == 0 and
                                    saIndex1Vld = 0 */
    bool      saIndex0Vld;     /**< Specifies whether SaIndex0 is valid. If txSaActive == 0 and saIndex0Vld == 0 for this SC, 
                                    then the no active SA counter will increment if a packet is sent on this SC */
    bool      saIndex1Vld;     /**< Specifies whether SaIndex1 is valid. If txSaActive == 1 and saIndex1Vld == 0 for this SC, 
                                    then the no active SA counter will increment if a packet is sent on this SC */
    bool      txSaActive;      //!< Determines which SA to send for this SC (either saIndex0 or saIndex1)
    int       saIndex0;        //!< SA number for index0
    int       saIndex1;        //!< SA number for index0
    long long sectagSci;       //!< 64-bit secure channel identifier for this SC
} MacsecEgressSaMapEntry_t;

/*!
 * @brief Set a custom e-type
 *
 * This structure is used to enable a specific etype in the MACsec PEX filter.
 *
 * @sa MacsecSetPacketFilter
 * @sa MacsecGetPacketFilter
 *
 * @ingroup filter_cfg_sec_fns
 */
typedef struct {
    bool        enable;     //!< True if this etype is to be enabled
    uint16_t    etype;      //!< The value of the etype, ignored if enable == false
} MacsecFilterEtype_t;

/*!
 * @brief Set a custom e-type
 *
 * This structure is used to enable a specific etype in the MACsec PEX filter.
 *
 * @sa MacsecSetPacketFilter
 * @sa MacsecGetPacketFilter
 *
 * @ingroup filter_cfg_sec_fns
 */
typedef struct {
    bool        enInner;     //!< True if this etype is to be enabled
    bool        enOuter;     //!< True if this etype is to be enabled
    uint16_t    etype;       //!< The value of the etype, ignored if enable == false
} MacsecFilterEtype2_t;

/*!
 * @brief Set a custom combo rule
 *
 * This structure is used to enable a combo control packet rule in the PEX filter
 *
 * @sa MacsecSetPacketFilter
 * @sa MacsecGetPacketFilter
 *
 * @ingroup filter_cfg_sec_fns
 */
typedef struct {
    bool        enInner;     //!< True if this etype is to be enabled
    bool        enOuter;     //!< True if this etype is to be enabled
    uint64_t    minDa;   //!< The minimum DA to match
    uint64_t    maxDa;   //!< The maximum DA to match
    uint16_t    et;         //!< The EthType for matching
} MacsecFilterCombo_t;

/*!
 * @brief Set a DA matching rule
 *
 * This structure is used to enable a DA rule for control packet matching
 *
 * @sa MacsecSetPacketFilter
 * @sa MacsecGetPacketFilter
 *
 * @ingroup filter_cfg_sec_fns
 */
typedef struct {
    bool        enInner;     //!< True if this etype is to be enabled
    bool        enOuter;     //!< True if this etype is to be enabled
    uint64_t    da;      //!< The DA to match
} MacsecFilterDa_t;

/*!
 * @brief Set a DA range rule
 *
 * This structure is used to enable a DA range rule for control packet matching
 *
 * @sa MacsecSetPacketFilter
 * @sa MacsecGetPacketFilter
 *
 * @ingroup filter_cfg_sec_fns
 */
typedef struct {
    bool        enInner;     //!< True if this etype is to be enabled
    bool        enOuter;     //!< True if this etype is to be enabled
    uint64_t    minDa;   //!< The minimum DA to match
    uint64_t    maxDa;   //!< The maximum DA to match
} MacsecFilterDaRange_t;

/*!
 * @brief Set a MAC matching rule
 *
 * This structure is used to enable MAC specific protocol matching
 *
 * @sa MacsecSetPacketFilter
 * @sa MacsecGetPacketFilter
 *
 * @ingroup filter_cfg_sec_fns
 */
typedef struct {
    bool        enInner;     //!< True if this etype is to be enabled
    bool        enOuter;     //!< True if this etype is to be enabled
    uint64_t    mac;     //!< The MAC to match
} MacsecFilterMac_t;

/*!
 * @brief Parameters for a VLAN or custom tag
 *
 * A custom tag is a tag of programmable length between 2B and 16B 
 * (in increments of 2B) and can be used as a means to parse arbitrary L2 tags.
 * This feature is only available if the product supports it, otherwise all
 * custom tags are treated as VLANs
 *
 * @sa MacsecSetPacketFilter
 * @sa MacsecGetPacketFilter
 *
 * @ingroup filter_cfg_sec_fns
 */
typedef struct {
    bool    vlanEn;    //!< Determines if this particular instance is enabled
    uint16_t   vlanEtype; //!< Etype of this instance
    uint8_t    vlanIndx;  /**< Index of this instance. No two instances cannot map to the same index.
                               Unless the user has a particular reason, these should map 1:1 with the instance  */
    uint8_t    vlanSize;  //!< Size of the custom tag. Vlan tag must be 4B
    bool    isVlan;    //!< Indicates whether this instance of the custom tags is a vlan or not
    bool    vlanBonus; /**< Indicates if bonus data can be extracted from this custom tag. 
                               Bonus data corresponds to the first 2B after the vlanEtype in the tag. \n 
                               This field must be 0 if isVlan is 1 */
} MacsecFilterCustomTag_t;


/*!
 * @brief UDP configuration
 *
 * @sa MacsecSetPacketFilter
 * @sa MacsecGetPacketFilter
 *
 * @ingroup filter_cfg_sec_fns
 */
typedef struct {
    bool        enable;     //!< True if this UDP is to be enabled
    uint8_t     udpProto;    //!< UDP Protocol Number
} MacsecFilterUdp_t;

/*!
 * @brief PTP Configuration
 *
 * @sa MacsecSetPacketFilter
 * @sa MacsecGetPacketFilter
 *
 * @ingroup filter_cfg_sec_fns
 */
typedef struct {
    bool        enable;     //!< True if this PTP filter is to be enabled
    uint16_t    ptpPort;    //!< UDP destination port number to identify PTP packets
} MacsecFilterPtp_t;

/*!
 * @brief Egress header Configuration
 *
 * @sa MacsecSetPacketFilter
 * @sa MacsecGetPacketFilter
 *
 * @ingroup filter_cfg_sec_fns
 */
typedef struct {
    bool        enable;     //!< True if this Egress header filter is to be enabled
    uint16_t    ethType;    //!< EthType for CT Egress Header
} MacsecFilterEgressHeader_t;

/*!
 * @brief Interrupt enables for all blocks 
 *
 * This corresponds to the enable bits in the top level
 * interrupt sum register
 *
 * @sa MacsecEnableBlockInterrupts
 *
 * @ingroup int_if_fns
 */
typedef struct {
  bool rxBbeSlaveInterruptEnb;
  bool txBbeSlaveInterruptEnb;
  bool rxPabSlaveInterruptEnb;
  bool txPabSlaveInterruptEnb;
  bool rxPexSlaveInterruptEnb;
  bool txPexSlaveInterruptEnb;
  bool rxCpmSlaveInterruptEnb;
  bool txCpmSlaveInterruptEnb;
} BlockInterruptEnables_t;

/*!
 * @brief Interrupts for BBE slave
 *
 * @sa MacsecGetInterrupts
 *
 * @ingroup int_if_fns
 */
typedef struct {
  bool dfifoOverflow; // Data FIFO (port 0 to 0 for bit 0 to 0 respectively) experienced an overflow, a reset to that channel is required.
  bool plfifoOverflow; // Policy FIFO (port 0 to 0 for bit 0 to 0 respectively) experienced an overflow, a reset to that channel is required.
} BbeInterrupts_t;

/*!
 * @brief Interrupts for PAB slave
 *
 * @sa MacsecGetInterrupts
 *
 * @ingroup int_if_fns
 */
typedef struct {
  char overflow; // Corresponding channel experienced an overflow, a reset to that channel is required. This particular design variant of the MACsec has only even channels active, odd channels are unused.
} PabInterrupts_t;

/*!
 * @brief Interrupts for PEX slave
 *
 * @sa MacsecGetInterrupts
 *
 * @ingroup int_if_fns
 */
typedef struct {
  bool earlyPreemptErr; // Violation of the Early Preemption Rule is detected for port 0 to 0. One or more preempt packets may be dropped/punted.
} PexInterrupts_t;

/*!
 * @brief Interrupts for CPM RX slave
 *
 * @sa MacsecGetInterrupts
 *
 * @ingroup int_if_fns
 */
typedef struct {
  bool sectagVEq1;
  bool sectagEEq0CEq1;
  bool sectagSlGte48;
  bool sectagEsEq1ScEq1;
  bool sectagScEq1ScbEq1;
  bool packetXpnEq0;
  bool pnThreshReached;
  uint32_t scExpiryPreTimeout;
  uint32_t scExpiryActualTimeout;
} CpmRxInterrupts_t;

/*!
 * @brief Interrupts for CPM TX slave
 *
 * @sa MacsecGetInterrupts
 *
 * @ingroup int_if_fns
 */
typedef struct {
  bool packetXpnEq0;
  bool pnThreshReached;
  bool saNotValid;
  uint32_t scExpiryPreTimeout;
  uint32_t scExpiryActualTimeout;
} CpmTxInterrupts_t;
  
/*!
 * @brief Root of interrupt tree
 *
 * @sa MacsecGetInterruptEnables
 *
 * @ingroup int_if_fns
 */

typedef struct {
  BbeInterrupts_t bbeRxInterrupts;
  BbeInterrupts_t bbeTxInterrupts;
  PabInterrupts_t pabRxInterrupts;
  PabInterrupts_t pabTxInterrupts;
  PexInterrupts_t pexRxInterrupts;
  PexInterrupts_t pexTxInterrupts;
  CpmRxInterrupts_t cpmRxInterrupts;
  CpmTxInterrupts_t cpmTxInterrupts;
} MacsecInterrupts_t;

/*!
 * @brief SC timer threshold values
 *
 * The Macsec engine provides 4 programmable threshold values
 * to be used as either pre or actual thresholds in the sc timer memories
 * Each sc timer memory must map its pre and actual thresholds to one 
 * of 4 possible indices 
 *
 * @ingroup sc_timer_fns
 */
typedef uint32_t MacsecScTimerThresholds_t[4];

/*!
 * @struct Ra01RsMcsCpmRxScTimerResetAllGo_t
 * @brief SC timer global reset struct
 *
 * @var Ra01RsMcsCpmRxScTimerResetAllGo_t::reset
 * If set, then all the SC timers will be zeroed out.
 * This bit is cleared by HW once all the corresponding SC timers are cleared (::Ra01AccRsMcsCpmRxScTimerResetAllGo)
 *
 * @ingroup sc_timer_fns
 */

/*!
 * @brief SC timer tick value
 *
 * Configures the duration of the time unit tick in terms of the number of core clock cycles.
 * The SC timers will increment by 1 every MacsecScTimerTickCycles number of cycles
 *
 * @ingroup sc_timer_fns
 */
typedef uint32_t MacsecScTimerTickCycles;

/*!
 * @struct Ra01IfRsMcsCpmRxScTimer_t
 * @brief SC timer memory word
 *
 * @var Ra01IfRsMcsCpmRxScTimer_t::enable
 * Enable SC expiry for this index
 *
 * @var Ra01IfRsMcsCpmRxScTimer_t::actualTimeoutThreshSelect
 * 2 bit select to determine which of the 4 indices (::MacsecScTimerThresholds_t) to use as the actual timeout threshold
 * When the SC timer reaches this threshold, the corresponding interrupt (::Ra01RsMcsCpmRxScExpiryActualTimeout_t) will 
 * trigger for this SC.
 *
 * @var Ra01IfRsMcsCpmRxScTimer_t::preTimeoutThreshSelect
 * 2 bit select to determine which of the 4 indices (::MacsecScTimerThresholds_t) to use as the pre timeout threshold
 * When the SC timer reaches this threshold, the corresponding interrupt (::Ra01RsMcsCpmRxScExpiryPreTimeout_t) will 
 * trigger for this SC. This serves as an alarm to SW that the SC timer will reach the actual threshold soon
 *
 * @var Ra01IfRsMcsCpmRxScTimer_t::timer
 * This is the 32-bit SC timer. This timer monotonically increments by 1 for every number of ticks
 * (::MacsecScTimerTickCycles which is set using ::MacsecConfigureScTimer)
 *
 * @ingroup sc_timer_fns
 */

/*!
 * @struct Ra01IfRsMcsCpmTxScTimer_t
 * @brief SC timer memory word
 *
 * @var Ra01IfRsMcsCpmTxScTimer_t::enable
 * Enable SC expiry for this index
 *
 * @var Ra01IfRsMcsCpmTxScTimer_t::startGo
 * When the bit is set, the corresponding SC Expiry Timer will be cleared by HW. 
 * The bit stays set until HW clears the Timer. Once the Expiry Timer is actually cleared by HW, 
 * the corresponding bit will also be cleared by HW. Writing a 0 to any of the bits in this register has no effect. 
 * SW can poll this register to see that the bit for a particular SC goes low indicating that 
 * the Expiry timer has been cleared and is now actively tracking time for the particular SC. 
 * \note All config registers pertaining to the SC Timer process of the SC must be configured 
 * prior to setting the GO bit. This register is used in the SC/SA timer expiry mechanism.
 *
 * @var Ra01IfRsMcsCpmTxScTimer_t::startMode
 * Configures what starts the corresponding SC's timer. 
 *    - 0: "start on first packet" -> The timer will restart (reset to 0 and start counting)
 *         once the first packet is sent on this SC 
 *    - 1: "start of rekey" -> The timer will restart (reset to 0 and start counting)
 *         when an SA change is detected on this SC
 *
 * @var Ra01IfRsMcsCpmTxScTimer_t::actualTimeoutThreshSelect
 * 2 bit select to determine which of the 4 indices (::MacsecScTimerThresholds_t) to use as the actual timeout threshold
 * When the SC timer reaches this threshold, the corresponding interrupt (::Ra01RsMcsCpmTxScExpiryActualTimeout_t) will 
 * trigger for this SC and the corresponding valid bit (::MacsecEgressSaMapEntry_t::saIndex0Vld or ::MacsecEgressSaMapEntry_t::saIndex1Vld 
 * depending on the value of ::MacsecEgressSaMapEntry_t::txSaActive) will get cleared. 
 * If ::MacsecEgressSaMapEntry_t::autoRekeyEnable is set, the MacsecEgressSaMapEntry_t::txSaActive will also toggle.
 *
 * @var Ra01IfRsMcsCpmTxScTimer_t::preTimeoutThreshSelect
 * 2 bit select to determine which of the 4 indices (::MacsecScTimerThresholds_t) to use as the pre timeout threshold
 * When the SC timer reaches this threshold, the corresponding interrupt (::Ra01RsMcsCpmTxScExpiryPreTimeout_t) will 
 * trigger for this SC. This serves as an alarm to SW that the SC timer will reach the actual threshold soon
 *
 * @var Ra01IfRsMcsCpmTxScTimer_t::timer
 * This is the 32-bit SC timer. This timer monotonically increments by 1 for every number of ticks
 * (::MacsecScTimerTickCycles which is set using ::MacsecConfigureScTimer)
 *
 * @ingroup sc_timer_fns
 */

//------------------------------------------------------------------------------
// FIPS
//------------------------------------------------------------------------------
/*!
 * @brief FIPS operation
 *
 * Used to identify the opertion. This determines with GAE block
 * is handling the request as the ingress side handles the decrytion and
 * the egress side the encryption.
 *
 * @sa MacsecPacketToFips
 *
 * @ingroup FIPS_interface
 */
typedef enum {
    MACSEC_FIPS_ENCRYPT,        //!< Encryption operation
    MACSEC_FIPS_DECRYPT         //!< Decryption operation
} MacsecFipsOperation_t;

/*!
 * @brief Auto/manual counter operation.
 *
 * For the setting of the counter mode in fips mode operation
 * Counter mode allows for the initialization of the bottom 32 bits of IV to a custom value (defined as initCtr in the MacsecSakData_t struct
 *
 * @sa MacsecPacketToFips
 * @sa MacsecFipsMode_t
 *
 * @ingroup FIPS_interface
 */
typedef enum {
    MACSEC_FIPS_CTR_AUTO = 0,       //!< GCM counter increments automatically (default)
    MACSEC_FIPS_CTR_MANUAL = 1,     //!< GCM counter programmed manually
} MacsecFipsCtrMode_t;

/*!
 * @brief AES key length
 *
 * To set the AES key length for FIPS mode operation
 *
 * @sa MacsecPacketToFips
 * @sa MacsecFipsMode_t
 *
 * @ingroup FIPS_interface
 */
typedef enum {
    MACSEC_FIPS_KEY_LEN_128 = 0,        //!< AES-128 mode (default)
    MACSEC_FIPS_KEY_LEN_256 = 1,        //!< AES-256 mode
} MacsecFipsKeyLen_t;

/*!
 * @brief AES-ECB mode
 *
 * To set the use of AES-ECB.
 *
 * @sa MacsecPacketToFips
 * @sa MacsecFipsMode_t
 *
 * @ingroup FIPS_interface
 */
typedef enum {
    MACSEC_FIPS_AES_ECB_DISABLED = 0,  //!< ECB mode disabled
    MACSEC_FIPS_AES_ECB_ENABLED  = 1,  //!< ECB mode enabled (only supported for encryption)
} MacsecFipsAesEcbMode_t;

/*!
 * @brief Set the parameters and data for the packet or FIPS vector to be encrypted/decrypted
 *
 * The entirety of the packet (without the ICV) is passed as a byte array using the data field
 *
 * The data from byte pre_st_aad_start to the byte before pre_st_aad_end is concatenated with the data from aad_start_offset to the byte before (aad_start_offset + aad_size) and used as AAD for GCM mode. The data from bytes (aad_start_offset + aad_size) until the end of the byte array are used as the plaintext/ciphertext.
 *
 * To perform a FIPS operation on a non-MACsec packet, set pre_st_aad_start and pre_st_aad_end to 0 and use the aad_start_offset and aad_size parameters to define the AAD (if applicable)
 * 
 * @sa MacsecPacketToFips
 *
 * @ingroup FIPS_interface
 */
typedef struct {
   char * data;                  /**< packet or FIPS vector data as byte array (ICV not included) */
   uint16_t size;                /**< size of data array */
   uint16_t pre_st_aad_start;    /**< first byte of pre-sectag AAD */
   uint16_t pre_st_aad_end;      /**< byte after the last byte of pre-sectag AAD */
   uint16_t aad_start_offset;    /**< first byte of AAD */
   uint16_t aad_size;            /**< size of AAD data starting from aad_start_offset (does not include pre-sectag AAD)*/
} MacsecPacket_t;

/*!
 * @brief Defines the AES mode and key size to be used
 * 
 * Default mode of operation is GCM unless CTR mode or ECB mode are enabled. 
 * 
 * @sa MacsecPacketToFips
 * @sa MacsecFipsCtrMode_t
 * @sa MacsecFipsKeyLen_t
 * @sa MacsecFipsAesEcbMode_t
 *
 * @ingroup FIPS_interface
 */
typedef struct {
   MacsecFipsCtrMode_t ctrMode;     /**< defines whether CTR mode is enabled (only valid when ECB is disabled) */
   MacsecFipsKeyLen_t keyLen;       /**< defines the key width */
   MacsecFipsAesEcbMode_t ecbMode;  /**< defines whether ECB mode is enabled (only valid for encryption) */
} MacsecFipsMode_t;

/*!
 * @brief Key data to be used for AES operation
 *
 * @ingroup FIPS_interface
 */
typedef struct {
   uint64_t sakBits255_192;         /**< Key bits 255-192 (inclusive) */
   uint64_t sakBits191_128;         /**< Key bits 192-128 (inclusive) */
   uint64_t sakBits127_64;          /**< Key bits 127-64 (inclusive) */
   uint64_t sakBits63_0;            /**< Key bits 63-0 (inclusive) */
   uint64_t hashkeyBits127_64;      /**< Hashkey bits 127-64 (inclusive). 
                                         Hashkey is generated by encrypting 128 bits of 0 wih the SAK, i.e. AES(128'b0, SAK)*/
   uint64_t hashkeyBits63_0;        /**< Hashkey bits 64-0 (inclusive) 
                                         Hashkey is generated by encrypting 128 bits of 0 wih the SAK, i.e. AES(128'b0, SAK) */
   uint32_t ivBits95_64;            /**< IV bits 95-64 (inclusive, unused for ECB mode) */
   uint64_t ivBits63_0;             /**< IV bits 63-0 (inclusive, unused for ECB mode) */
   uint32_t initCtr;                /**< Initial counter value for CTR mode, unused for other modes */
} MacsecFipsSakData_t;

int FipsPolicy(RmsDev_t * rmsDev_p, MacsecFipsMode_t fipsMode, MacsecFipsSakData_t sakData, Ra01RsMcsGaeInstance_t instance);

int FipsStart(RmsDev_t * rmsDev_p, Ra01RsMcsGaeInstance_t instance);

int FipsReset(RmsDev_t * rmsDev_p, Ra01RsMcsGaeInstance_t instance);

int FipsCtl(RmsDev_t * rmsDev_p, Ra01RsMcsGaeInstance_t instance,
            bool valid,
            bool isAad,
            bool eop,
            uint8_t blockSize,
            bool lenRound,
            bool nextIcv);
bool FipsNextIcv(MacsecFipsOperation_t fipsOperation, int aadSize, int textSize, bool isLastRound, int isAad);

int FipsWriteBlock(RmsDev_t * rmsDev_p, Ra01RsMcsGaeInstance_t instance, uint64_t blockMsb, uint64_t blockLsb);

int FipsReadBlock(RmsDev_t * rmsDev_p, Ra01RsMcsGaeInstance_t instance, uint64_t * blockMsb, uint64_t * blockLsb);

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 *                                                                             *
 *  General Software API functions                                                   *
 *                                                                             *
 *******************************************************************************/
/*!
 * @brief Initialize the software API.
 *
 * @param[inout]  rmsDev_p   The hardware handle for the target device
 *
 * Call this first. This function initializes the software API's internal memory and
 * structures.
 *
 * @retval  0      Success
 *
 * NOTE: This function always succeeds.
 *
 * @ingroup port_if_fns
 */
DLL_PUBLIC int MacsecInitialize( RmsDev_t * rmsDev_p );

/*!
 * @brief Uninititalize the software API
 *
 * @param[inout]  rmsDev_p   The hardware handle for the target device
 *
 * If the software is unloaded then this function should be called after all the
 * caller's interactions with the software are complete. Frees any allocated memory
 * and system resources.
 *
 * @retval  0      Success
 *
 * NOTE: This function always succeeds.
 *
 * @ingroup port_if_fns
 */
DLL_PUBLIC int MacsecUninitialize(RmsDev_t * rmsDev_p );

/*!
 * @brief Get the capabilities of the MACsec hardware
 *
 * @param[in] capabilities   Pointer to a capabilities struct to fill
 *
 * @retval  0       Success
 * @retval -ENOSYS Could not read the capabilited structure
 *
 * @sa MacsecCapabilities_t
 *
 * @ingroup port_if_fns
 */
DLL_PUBLIC int MacsecGetCapabilities( MacsecCapabilities_t * capabilities );

/*!
 * @brief Set the fixed latency of MACsec based on port rate
 *
 * @param[inout] rmsDev_p                    The hardware handle for the target device
 * @param[in]    portRate                    The chosen port rate
 * @param[in]    enableTxFixedLatency        Enable fixed latency for egress express packets
 * @param[in]    enableRxFixedLatency        Enable fixed latency for ingress express packets
 * @param[in]    enableTxPreemptFixedLatency   Enable fixed latency for egress preempt packets (default is for express only)
 * @param[in]    enableRxPreemptFixedLatency   Enable fixed latency for ingress preempt packets (default is for express only)
 *
 * @retval  0       Success
 * @retval  int     Return value from hardware write
 *
 * @ingroup port_if_fns
 */
  DLL_PUBLIC int MacsecSetFixedLatency(RmsDev_t * rmsDev_p, MacsecPortRate_t portRate, bool enableTxFixedLatency, bool enableRxFixedLatency, bool enableTxPreemptFixedLatency, bool enableRxPreemptFixedLatency);

/*!
 * @brief Enable/disable egress auto-rekeying
 *
 * @param[inout] rmsDev_p   The hardware handle for the target device
 * @param[in]    index        The handle for the SecY being configured
 * @param[in]    enable     true to enable auto-rekeying
 *
 * @retval  0       Success
 * @retval -EINVAL  Handle is ingress
 *
 * Enable or disable auto re-keying. Note, only valid in EGRESS.
 *
 * @ingroup secy_if_fns
 */
DLL_PUBLIC int MacsecEnableAutoRekey( RmsDev_t * rmsDev_p, unsigned index, bool enable );



/*******************************************************************************
 *                                                                             *
 *  SC Functions                                                               *
 *                                                                             *
 *******************************************************************************/

/*!
 * @brief Enable an ingress SC
 *
 * @param[inout]  rmsDev_p   The hardware handle for the target device
 * @param[in]     index      The SC handle
 * @param[in]     enable     Whether this entry is enabled.
 *
 * @retval  0       Success
 * @retval -EINVAL  Value is out of range
 * @retval  int     Return value from hardware write
 *
 * Toggle the enable bit for an ingress SC.
 *
 * @ingroup sc_if_fns
 */
DLL_PUBLIC int MacsecSetEnableSc( RmsDev_t * rmsDev_p, unsigned index, bool enable );

/*!
 * @brief Read an ingress SC enable setting
 *
 * @param[inout]  rmsDev_p   The hardware handle for the target device
 * @param[in]     index      The SC handle
 * @param[out]    enabled    Whether this entry is enabled.
 *
 * @retval  0       Success
 * @retval -EINVAL  Value is out of range
 * @retval  int     Return value from hardware write
 *
 * Read the enable bit for an ingress SC.
 *
 * @ingroup sc_if_fns
 */
DLL_PUBLIC int MacsecGetEnableSc( RmsDev_t * rmsDev_p, unsigned index, bool * enabled );

/*******************************************************************************
 *                                                                             *
 *  SA Functions                                                               *
 *                                                                             *
 *******************************************************************************/
/*!
 * @brief Write an SA policy
 *
 * @param[inout]  rmsDev_p   The hardware handle for the target device
 * @param[in]     direction  Ingress or Egress CPM
 * @param[in]     index      The SA index.
 * @param[in]     policy     Pointer to the policy to write.
 * @param[in]     lock       Set or clear the SA key lockout before setting it.
 *
 * @retval  0       Success
 * @retval -EINVAL  index is out of range
 * @retval  int     Return value from hardware write
 *
 * The SA is always within an SC so does not need a handle, it can be looked up
 * from the SC and the AN / SA index. On Ingress there are 4 entries, one per AN,
 * on egress there are 2 SA index values.
 *
 * @ingroup sa_if_fns
 *
 */
DLL_PUBLIC int MacsecSetSaPolicy( RmsDev_t *               rmsDev_p,
                                  MacsecDirection_t        direction,
                                  unsigned                 index,
                                  MacsecSaPolicy_t *       policy,
                                  bool                     lock );

/*!
 * @brief Read an SA policy
 *
 * @param[inout]  rmsDev_p   The hardware handle for the target device
 * @param[in]     direction  Ingress or Egress CPM
 * @param[in]     index      The SA index.
 * @param[in]     policy     Pointer to the policy to write.
 * @param[in]     lock       Set or clear the SA key lockout before setting it.
 *
 * @retval  0       Success
 * @retval -EINVAL  index is out of range
 * @retval  int     Return value from hardware write
 *
 * The SA is always within an SC so does not need a handle, it can bee looked up
 * from the SC and the AN / SA index. On Ingress there are 4 entries, one per AN,
 * on egress there are 2 SA index values.
 *
 * @ingroup sa_if_fns
 *
 */
DLL_PUBLIC int MacsecGetSaPolicy( RmsDev_t *               rmsDev_p,
                                  MacsecDirection_t        direction,
                                  unsigned                 index,
                                  MacsecSaPolicy_t *       policy,
                                  bool *                   lock );
/*!
 * @brief Read the next PN value for an SA.
 *
 * @param[inout]  rmsDev_p   The hardware handle for the target device
 * @param[in]     direction  Ingress or Egress PN
 * @param[in]     index      PN Table Index
 * @param[out]    nextPn     The PN value to write.
 *
 * @retval  0       Success
 * @retval -EINVAL  index is out of range
 * @retval  int     Return value from hardware write
 *
 * @ingroup sa_if_fns
 */
DLL_PUBLIC int MacsecGetNextPn( RmsDev_t *        rmsDev_p,
                                MacsecDirection_t direction,
                                unsigned          index,
                                uint64_t *        nextPn );

/*!
 * @brief Set the nextPN value for an SA.
 *
 * @param[inout]  rmsDev_p   The hardware handle for the target device
 * @param[in]     direction  Ingress or Egress PN
 * @param[in]     index      PN Table Index
 * @param[in]     nextPn     The PN value to write.
 *
 * @retval  0       Success
 * @retval -EINVAL  index is out of range
 * @retval  int     Return value from hardware write
 *
 * NOTE: the PN is 64-bits, however if XPN is not in use only the lower 32 are
 * relevant
 *
 * @ingroup sa_if_fns
 */
DLL_PUBLIC int MacsecSetNextPn( RmsDev_t *        rmsDev_p,
                                MacsecDirection_t direction,
                                unsigned          index,
                                uint64_t          nextPn );

/*!
 * @brief Get the Ingress SC+AN to SA Map Table
 *
 * @param[inout]  rmsDev_p   The hardware handle for the target device
 * @param[in]     index      Ingress SC+AN to SA Map Table index
 * @param[out]    saIndex    PN Table Index to set
 * @param[out]    enable     true to enable the SA, false to disable the SA
 *
 * @retval  0       Success
 * @retval -EINVAL  index is out of range
 * @retval  int     Return value from hardware write
 *
 * @ingroup sa_if_fns
 */
int MacsecGetEnableIngressSa( RmsDev_t * rmsDev_p, unsigned index, uint32_t *saIndex, bool *enable );

/*!
 * @brief Set the Ingress SC+AN to SA Map Table
 *
 * @param[inout]  rmsDev_p   The hardware handle for the target device
 * @param[in]     index      Ingress SC+AN to SA Map Table index
 * @param[in]     saIndex    PN Table Index to set
 * @param[in]     enable     true to enable the SA, false to disable the SA
 *
 * @retval  0       Success
 * @retval -EINVAL  index is out of range
 * @retval  int     Return value from hardware write
 *
 * @ingroup sa_if_fns
 */
DLL_PUBLIC int MacsecEnableIngressSa( RmsDev_t *        rmsDev_p,
                                      unsigned          index,
                                      uint32_t          saIndex,
                                      bool              enable );

/*!
 * @brief Retrieve an Egress SecY to SA Map Table entry.
 *
 * @param[inout]  rmsDev_p   The hardware handle for the target device
 * @param[in]     index      Egress SecY to SA Map Table Index
 * @param[in]     saMap      SA data to be read from the Egress to SA Map Table (1..2)
 *
 * The Egress SA map consists of 4 registers:
 *    1. SA_MAP_MEM - the sa_index0 and sa_index1 indicies to the Egress SA Policy
 *                    Table entries
 *    2. TX_SA_ACTIVE[N..0] - the selector between sa_index0 and sa_index1
 *    3. SA_INDEX0_VLD[N..0] - the valid status of sa_index0
 *    4. SA_INDEX1_VLD[N..0] - the valid status of sa_index1
 * This API will set ALL FOUR registers. To set only the SA Policy index use the following:
 *  MacsecSetEgressSaIndex - to set an SA Index value
 *
 * NOTE: The entire struct is written to the chip. Including non-valid and
 *       inactive saIndex values.
 *
 * @retval  0       Success
 * @retval -EINVAL  SC is ingress or index is out of range
 * @retval  int     Return value from hardware write
 *
 * @ingroup sa_if_fns
 *
 * @sa MacsecGetEgressSaIndex
 */
DLL_PUBLIC int MacsecGetEgressSaMap( RmsDev_t * rmsDev_p, unsigned index, MacsecEgressSaMapEntry_t * saMap );

/*!
 * @brief Select an Egress SecY to SA Map Table entry.
 *
 * @param[inout]  rmsDev_p   The hardware handle for the target device
 * @param[in]     index      Egress SecY to SA Map Table Index
 * @param[in]     saMap      SA data to be written to the Egress to SA Map Table (1..2)
 *
 * The Egress SA map consists of 4 registers:
 *    1. SA_MAP_MEM - the sa_index0 and sa_index1 indicies to the Egress SA Policy
 *                    Table entries
 *    2. TX_SA_ACTIVE[N..0] - the selector between sa_index0 and sa_index1
 *    3. SA_INDEX0_VLD[N..0] - the valid status of sa_index0
 *    4. SA_INDEX1_VLD[N..0] - the valid status of sa_index1
 * This API will set ALL FOUR registers. To set only the SA Policy index use the following:
 *  MacsecSetGressSaIndex - to set an SA Index value
 *
 * NOTE: The entire struct is written to the chip. Including non-valid and
 *       inactive saIndex values.
 *
 * @retval  0       Success
 * @retval -EINVAL  SC is ingress or index is out of range
 * @retval  int     Return value from hardware write
 *
 * @ingroup sa_if_fns
 *
 * @sa MacsecSetEgressSaIndex
 */
DLL_PUBLIC int MacsecSetEgressSaMap( RmsDev_t * rmsDev_p, unsigned index, MacsecEgressSaMapEntry_t * saMap );

/*******************************************************************************
 *                                                                             *
 *  Counter Functions                                                          *
 *                                                                             *
 *******************************************************************************/

#include "macsec_counters.h"

/*!
 * @brief Number of counters
 *
 * This is the number of counter identifiers. Each identifier can have several physical
 * counters associated with it and selected by an index value.
 *
 * @sa MacsecCounterId_t
 */
#define MACSEC_COUNTERS     __MACSEC_COUNTER_COUNT__

#undef EXPAND_AS_ENUMERATION

/*******************************************************************************
 *                                                                             *
 *  Miscellaneous Functions                                                    *
 *                                                                             *
 *******************************************************************************/

/*!
 * @brief Set the configuration for the ports
 *
 * @param[inout]  rmsDev_p   The hardware handle for the target device
 * @param[in]     cfg        Array of configuration parameters.
 *
 * @retval  0       Success
 * @retval  int     Return value from hardware write
 *
 * This API requires an array of 4 port confiuration structs, one per physical port.
 *
 *
 * @ingroup port_if_fns
 */
DLL_PUBLIC int MacsecSetConfiguration( RmsDev_t * rmsDev_p, MacsecConfig_t * cfg );

/*!
 * @brief Read the configuration setting for the ports
 *
 * @param[inout]  rmsDev_p   The hardware handle for the target device
 * @param[out]    cfg        Array of configuration parameters
 *
 * @retval  0       Success
 * @retval  int     Return value from hardware write
 *
 * NOTE: the boolean parameter flags will be set to true after the corresponding values are set.
 *       These flags are:
 *          cfg->symmetric[3:0].valid
 *          cfg->ingress[3:0].valid
 *          cfg->ingress.opt.setOptParameters
 *          cfg->egress[3:0].valid
 *          cfg->egress.opt.setOptParameters
 *     The portParams[3:0].valid flag will only be set if that port's mode is valid. For example
 *      if Port 0 is set to 100G, Ports 1-3 will NOT be valid.
 *
 * @ingroup port_if_fns
 */
DLL_PUBLIC int MacsecGetConfiguration( RmsDev_t * rmsDev_p, MacsecConfig_t * cfg );

/*!
 * @brief Set pn threshold for given direction
 *
 * @param[inout]  rmsDev_p         The hardware handle for the target device
 * @param[in]     direction        Direction: MACSEC_INGRESS or MACSEC_EGRESS
 * @param[in]     pnThreshold      Desired PN threshold
 *
 * @retval  0       Success
 * @retval  int     Return value from hardware write
 *
 * @ingroup port_if_fns
 */
  DLL_PUBLIC int MacsecSetPnThreshold( RmsDev_t *                rmsDev_p,
				       MacsecDirection_t         direction,
				       int                       pnThreshold );

/*!
 * @brief Set xpn threshold for given direction
 *
 * @param[inout]  rmsDev_p         The hardware handle for the target device
 * @param[in]     direction        Direction: MACSEC_INGRESS or MACSEC_EGRESS
 * @param[in]     xpnThreshold      Desired PN threshold
 *
 * @retval  0       Success
 * @retval  int     Return value from hardware write
 *
 * @ingroup port_if_fns
 */
  DLL_PUBLIC int MacsecSetXpnThreshold( RmsDev_t *                rmsDev_p,
					MacsecDirection_t         direction,
					long long                 xpnThreshold );

/*!
 * @brief Enable/disable bypass for given channel
 *
 * @param[inout]  rmsDev_p         The hardware handle for the target device
 * @param[in]     index            Channel index
 * @param[in]     isBypass         Bypass enable for channel
 *
 * @retval  0       Success
 * @retval -EINVAL  Value is out of range
 * @retval  int     Return value from hardware write
 *
 * @ingroup port_if_fns
 */
  DLL_PUBLIC int MacsecSetChannelBypass( RmsDev_t * rmsDev_p, unsigned index, bool isBypass );

/*!
 * @brief Get bypass status of channel
 *
 * @param[inout]  rmsDev_p         The hardware handle for the target device
 * @param[in]     index            Channel index
 * @param[out]    isBypass         Bypass enable for channel
 *
 * @retval  0       Success
 * @retval -EINVAL  Value is out of range
 * @retval  int     Return value from hardware write
 *
 * @ingroup port_if_fns
 */
  DLL_PUBLIC int MacsecGetChannelBypass( RmsDev_t * rmsDev_p, unsigned index, bool * isBypass );

/*******************************************************************************
 *                                                                             *
 *  Interrupt Functions                                                        *
 *                                                                             *
 *******************************************************************************/

/*!
 * @brief Enable all interrupts
 *
 * @param[inout]  rmsDev_p   The hardware handle for the target device
 *
 * @retval  0       Success
 * @retval -EINVAL  Invalid argument
 * @retval  int     Return value from hardware write
 *
 * @ingroup int_if_fns
 */
DLL_PUBLIC int MacsecEnableInterrupts( RmsDev_t * rmsDev_p );

/*!
 * @brief Enable interrupts for each block
 *
 * @param[inout]  rmsDev_p   The hardware handle for the target device
 * @param[in]     interruptEnables Interrupt enables for each slave
 *
 * @retval  0       Success
 * @retval -EINVAL  Invalid argument
 * @retval  int     Return value from hardware write
 *
 * @ingroup int_if_fns
 */
DLL_PUBLIC int MacsecEnableBlockInterrupts( RmsDev_t * rmsDev_p, BlockInterruptEnables_t const * interruptEnables );


/*!
 * @brief Get the interrupt enables of the interrupt tree
 *
 * @param[inout]  rmsDev_p          The hardware handle for the target device
 * @param[out]    interruptEnables  Interrupt enable tree
 *
 * @retval  0       Success
 * @retval -EINVAL  Invalid argument
 * @retval  int     Return value from hardware write
 *
 * @ingroup int_if_fns
 */
DLL_PUBLIC int MacsecGetInterruptEnables( RmsDev_t * rmsDev_p, MacsecInterrupts_t * interruptEnables );


/*!
 * @brief Get the interrupt status of the interrupt tree
 *
 * @param[inout]  rmsDev_p          The hardware handle for the target device
 * @param[out]    interruptStatus   Interrupt enables for each slave
 *
 * @retval  0       Success
 * @retval -EINVAL  Invalid argument
 * @retval  int     Return value from hardware write
 *
 * @ingroup int_if_fns
 */
DLL_PUBLIC int MacsecGetInterruptStatus( RmsDev_t * rmsDev_p, MacsecInterrupts_t * interruptStatus );


/*!
 * @brief Clear all interrupts specified in the clearInterrupts input tree
 *
 * @param[inout]  rmsDev_p          The hardware handle for the target device
 * @param[in]     clearInterrupts   Interrupt enables for each slave
 *
 * @retval  0       Success
 * @retval -EINVAL  Invalid argument
 * @retval  int     Return value from hardware write
 *
 * @ingroup int_if_fns
 */
DLL_PUBLIC int MacsecClearInterrupts( RmsDev_t * rmsDev_p, MacsecInterrupts_t clearInterrupts );

/******************************************************************************
 *                                                                             *
 *  FIPS Functions                                                             *
 *                                                                             *
 *******************************************************************************/
/*!
 * @brief Performs AES transform operation on an input packet using FIPS registers
 *
 * @param[inout]  rmsDev_p          The hardware handle for the target device
 * @param[in]     fipsOperation     Defines FIPS operation (encryption or decryption)
 * @param[in]     fipsMode          Defines AES mode and key size
 * @param[in]     sakData           Defines key, IV, and initial counter value (for CTR mode)
 * @param[inout]  packet            Packet data, size, and parameters marking AAD. Transform is performed in-place, so result data will be in the same structure.
 * @param[out]    resultPass        For decryption, this bit will be set to 1 if the input tag matches the calculated tag (for GCM mode only) 
 * @param[inout]  tag               Only used in GCM mode. For encryption, this parameter is an output which contains the calculated tag. For decryption, this is an input for the expected tag. 
 *
 * @retval  0       Success
 * @retval -EINVAL  parameter is invalid
 * @retval -EAGAIN  timeout while waiting for FIPS_START or FIPS_RESET register to go low
 * @retval -EFAULT  Register mapping failed
 * @retval  int     Return value from hardware read
 *
 * @ingroup FIPS_interface
 */
DLL_PUBLIC int MacsecPacketToFips(RmsDev_t * rmsDev_p, MacsecFipsOperation_t fipsOperation, MacsecFipsMode_t fipsMode, MacsecFipsSakData_t sakData, volatile MacsecPacket_t packet, bool * resultPass, char * tag);

/******************************************************************************
 *                                                                             *
 *  Rekey Functions                                                            *
 *                                                                             *
 *******************************************************************************/
/*!
 * @brief Handles TX rekey event for given SC 
 *
 * @param[inout]  rmsDev_p      The hardware handle for the target device
 * @param[in]     sc            SC that has undergone rekey event 
 * @param[in]     newPolicy     SA policy to be written to new SA if updatePolicy bit is set
 * @param[in]     updatePolicy  If set, SA policy for new SA will be updated to the policy specified by newPolicy 
 *
 * @retval  0       Success
 * @retval -EINVAL  parameter is invalid
 * @retval -EFAULT  Register mapping failed
 * @retval  int     Return value from hardware read
 *
 * Performs TX rekey operation for given sc. Steps are as follows
 * 1. Update PN table to 1 for inactive SA
 * 2. Update SA policy for inactive SA (if auto-rekey is disabled and updatePolicy flag is set)
 * 3. Set valid bit for inactive SA to 1
 * 4. Switch active SA (if auto-rekey is disabled)
 * 
 * When auto-rekey is enabled, this function will reset the PN table for the recently invalidated SA and revalidate it
 * When auto-rekey is disabled, this function will setup an entirely new SA and switch the active SA to it
 *
 * @ingroup sa_if_fns
 */
DLL_PUBLIC int MacsecTxRekeyHandler( RmsDev_t * rmsDev_p, unsigned sc, MacsecTxSaPolicy_t * newPolicy, bool updatePolicy ); 

/*!
 * @brief Handles RX rekey event for given SC + AN pair
 *
 * @param[inout]  rmsDev_p      The hardware handle for the target device
 * @param[in]     sc            SC that has undergone rekey event 
 * @param[in]     an            SA policy to be written to new SA if updatePolicy bit is set
 *
 * @retval  0       Success
 * @retval -EINVAL  parameter is invalid
 * @retval -EFAULT  Register mapping failed
 * @retval  int     Return value from hardware read
 *
 * Disables given SA, resets PN table to 1, and re-enables SA
 * @ingroup sa_if_fns
 */
DLL_PUBLIC int MacsecRxRekeyHandler( RmsDev_t * rmsDev_p, unsigned sc, unsigned an ); 

/*!
 * @brief Convenience API for reading packet seen registers
 *
 * @param[inout]  rmsDev_p      The hardware handle for the target device
 * @param[in]     sc            SC that has undergone rekey event 
 * @param[in]     an            SA policy to be written to new SA if updatePolicy bit is set
 * @param[out]    value         Returns the value of the packet seen status register
 *
 * @retval  0       Success
 * @retval  -EINVAL Parameter is invalid
 * @retval  int     Return value from hardware read
 *
 * Get the packet seen status registers that are used for re-key on RX path
 * 
 * @ingroup sa_if_fns
 */
DLL_PUBLIC int MacsecGetPacketSeen( RmsDev_t * rmsDev_p, unsigned sc, unsigned an, bool * value);

/*!
 * @brief Setup SC timer expiry thresholds
 *
 * @param[inout]  rmsDev_p      The hardware handle for the target device
 * @param[in]     direction     Specify configuration for RX or TX
 * @param[in]     thresholds    Array of threshold values
 * @param[in]     tick          Number of cycles for the sc timer tick
 *
 * @retval  0       Success
 * @retval  -EINVAL Parameter is invalid
 * @retval  int     Return value from hardware read
 *
 * Setup the 4 SC timer thresholds. Each SC timer memory will map its pre and actual timer thresholds 
 * to one of these 4 indices. Setup the sc timer tick value
 * 
 * @ingroup sc_timer_fns
 */
DLL_PUBLIC int MacsecConfigureScTimer( RmsDev_t * rmsDev_p, MacsecDirection_t direction, MacsecScTimerThresholds_t thresholds, MacsecScTimerTickCycles tick );

/*!
 * @fn int Ra01WriteRsMcsCpmTxScTimer( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCpmTxScTimer_t * data );
 * @brief Setup SC timer memories
 *
 * @param[inout]  rmsDev_p      The hardware handle for the target device
 * @param[in]     index         The secure channel
 * @param[in]     data          Data to be written
 *
 * @retval  0       Success
 * @retval  -EINVAL Parameter is invalid
 * @retval  int     Return value from hardware read
 *
 * The SC timer memory provides both the per-SC configuration for SC timer as well as 
 * the 32-bit timer itself. There should be no reason for SW to write the 32-bit timer value directly
 * 
 * @ingroup sc_timer_fns
 */

/*!
 * @fn int Ra01ReadRsMcsCpmTxScTimer( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCpmTxScTimer_t * data );
 * @brief Read SC timer memories
 *
 * @param[inout]  rmsDev_p      The hardware handle for the target device
 * @param[in]     index         The secure channel
 * @param[out]     data          Data to be read
 *
 * @retval  0       Success
 * @retval  -EINVAL Parameter is invalid
 * @retval  int     Return value from hardware read
 *
 * The SC timer memory provides both the per-SC configuration for SC timer as well as 
 * the 32-bit timer itself. 
 * 
 * @ingroup sc_timer_fns
 */

/*!
 * @fn int Ra01WriteRsMcsCpmRxScTimer( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCpmRxScTimer_t * data );
 * @brief Setup SC timer memories
 *
 * @param[inout]  rmsDev_p      The hardware handle for the target device
 * @param[in]     index         The secure channel
 * @param[in]     data          Data to be written
 *
 * @retval  0       Success
 * @retval  -EINVAL Parameter is invalid
 * @retval  int     Return value from hardware read
 *
 * The SC timer memory provides both the per-SC configuration for SC timer as well as 
 * the 32-bit timer itself. There should be no reason for SW to write the 32-bit timer value directly
 * 
 * @ingroup sc_timer_fns
 */

/*!
 * @fn int Ra01ReadRsMcsCpmRxScTimer( RmsDev_t * rmsDev_p, unsigned index, Ra01IfRsMcsCpmRxScTimer_t * data );
 * @brief Read SC timer memories
 *
 * @param[inout]  rmsDev_p      The hardware handle for the target device
 * @param[in]     index         The secure channel
 * @param[out]    data          Data to be read
 *
 * @retval  0       Success
 * @retval  -EINVAL Parameter is invalid
 * @retval  int     Return value from hardware read
 *
 * The SC timer memory provides both the per-SC configuration for SC timer as well as 
 * the 32-bit timer itself. 
 * 
 * @ingroup sc_timer_fns
 */

/*!
 * @fn int Ra01AccRsMcsCpmRxScTimerResetAllGo( RmsDev_t * rmsDev_p, Ra01RsMcsCpmRxScTimerResetAllGo_t * data, ra01_reg_access_op_t op );
 * @brief Reset all SC timers
 *
 * @param[inout]  rmsDev_p   The hardware handle for the target device
 * @param[inout]    data        Data for the access (output for READ operation, input for WRITE operation)
 * @param[in]    op          Read/Write operation
 *
 * @retval  0       Success
 * @retval  int     Return value from hardware read
 *
 * Tells the HW to go ahead and clear all of the SC Timers.
 * This register is used in the SC timer expiry mechanism.
 * 
 * @ingroup sc_timer_fns
 */

/*!
 * @fn int Ra01AccRsMcsCpmTxScTimerResetAllGo( RmsDev_t * rmsDev_p, Ra01RsMcsCpmTxScTimerResetAllGo_t * data, ra01_reg_access_op_t op );
 * @brief Reset all SC timers
 *
 * @param[inout]  rmsDev_p   The hardware handle for the target device
 * @param[inout]    data        Data for the access (output for READ operation, input for WRITE operation)
 * @param[in]    op          Read/Write operation
 *
 * @retval  0       Success
 * @retval  int     Return value from hardware read
 *
 * Tells the HW to go ahead and clear all of the SC Timers.
 * This register is used in the SC timer expiry mechanism.
 * 
 * @ingroup sc_timer_fns
 */

/*!
 * @brief Convenience API for clearing packet seen registers
 *
 * @param[inout]  rmsDev_p      The hardware handle for the target device
 * @param[in]     sc            SC that has undergone rekey event 
 * @param[in]     an            SA policy to be written to new SA if updatePolicy bit is set
 *
 * @retval  0       Success
 * @retval  -EINVAL Parameter is invalid
 * @retval  int     Return value from hardware read
 *
 * Clear the packet seen status registers that are used for re-key on RX path
 * 
 * @ingroup sa_if_fns
 */
DLL_PUBLIC int MacsecClearPacketSeen( RmsDev_t * rmsDev_p, unsigned sc, unsigned an);

/*
 * @brief Update the secy map table on RX
 *
 * @param[inout]   rmsDev_p       The hardware handle for the target device
 * @param[in]      tableIndex     Index into the table (corresponds to a flow)
 * @param[in]      secy           The secy to map to this flow
 * @param[in]      controlPacket  If this flow maps to an uncontrolled virtual port (i.e. is a control packet)
 *
 * @retval  0       Success
 * @retval -EIO     Handle does not exist
 * @retval -EINVAL  parameter is invalid
 * @retval  int     Return value from hardware read
 *
 * @ingroup secy_if_fns
 */
int setRxSecyMap( RmsDev_t * rmsDev_p, int tableIndex, uint32_t secy, bool controlPacket );

/*
 * @brief REad the secy map table on RX
 *
 * @param[inout] rmsDev_p       The hardware handle for the target device
 * @param[in]    tableIndex     Index into the table (corresponds to a flow)
 * @param[out]   secy           The secy thats map to this flow
 * @param[out]   controlPacket  If this flow maps to an uncontrolled virtual port (i.e. is a control packet)
 *
 * @retval  0       Success
 * @retval -EIO     Handle does not exist
 * @retval -EINVAL  parameter is invalid
 * @retval  int     Return value from hardware read
 *
 * @ingroup secy_if_fns
 */
int getRxSecyMap( RmsDev_t * rmsDev_p, int tableIndex, uint32_t *secy, bool *controlPacket );

/* 
 * @brief Update the secy map table on TX
 *
 * @param[inout]   rmsDev_p         The hardware handle for the target device
 * @param[in]      tableIndex       Index into the table (corresponds to a flow)
 * @param[in]      secy             The secy to map to this flow
 * @param[in]      sc               The sc to map to this flow
 * @param[in]      controlPacket    If this flow maps to an uncontrolled virtual port (i.e. is a control packet)
 * @param[in]      auxiliaryPolicy  Additional bit for this policy that flows through to the output
 *
 * @retval  0       Success
 * @retval -EIO     Handle does not exist
 * @retval -EINVAL  parameter is invalid
 * @retval  int     Return value from hardware read
 *
 * @ingroup secy_if_fns
 */
int setTxSecyMap( RmsDev_t * rmsDev_p,int tableIndex,uint32_t secy, uint32_t sc, bool controlPacket,bool auxiliaryPolicy );

/* 
 * @brief Read the secy map table on TX
 *
 * @param[inout]    rmsDev_p         The hardware handle for the target device
 * @param[in]       tableIndex       Index into the table (corresponds to a flow)
 * @param[out]      secy             The secy that maps to this flow
 * @param[out]      sc               The sc thats maps to this flow
 * @param[out]      controlPacket    If this flow maps to an uncontrolled virtual port (i.e. is a control packet)
 * @param[out]      auxiliaryPolicy  Additional bit for this policy that flows through to the output
 *
 * @retval  0       Success
 * @retval -EIO     Handle does not exist
 * @retval -EINVAL  parameter is invalid
 * @retval  int     Return value from hardware read
 *
 * @ingroup secy_if_fns
 */
int getTxSecyMap( RmsDev_t * rmsDev_p,int tableIndex,uint32_t * secy,uint32_t * sc,bool * controlPacket,bool * auxiliaryPolicy );


#ifdef __cplusplus
}
#endif

#endif // __MACSEC_API_H__

