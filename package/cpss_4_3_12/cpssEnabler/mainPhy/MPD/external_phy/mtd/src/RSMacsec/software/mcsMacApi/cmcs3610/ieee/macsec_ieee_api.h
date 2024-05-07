/*! 
 * @file macsec_ieee_api.h 
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

#ifndef __MACSEC_IEEE_API_H__
#define __MACSEC_IEEE_API_H__

#include <stdbool.h>
#include "mcs_internals_ieee.h"
#include "macsec_cfg.h"
#include "macsec_api.h"

#define MACSEC_API_MAJOR_VERSION 1
#define MACSEC_API_MINOR_VERSION 1
#define MACSEC_API_BUILD_ID      0

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

/*!
 * @brief Configuration for the port to custom tag mapping
 *
 * Set the index for the VLAN/Custom tag selection per port for relative SecTAG insertion mode.
 * this index actually correponds to the ~instance~ of the custom tag. So for example
 * if the MacsecPacketFilter_t::vlanTag[1].vlanIndx == 3, and user wants to match on vlanTag[i].vlanEtype
 * then a value of 1 would be programmed as MacsecCustomTagSelect_t::select (as opposed to 3)
 *
 * @sa MacsecSetPacketFilter
 * @sa MacsecGetPacketFilter
 *
 * @ingroup filter_cfg_fns
 */

typedef struct {
    bool                   enable;     //!< True if these values are to be programmed.
    uint8_t                select;     //!< The index of the custom tag.
} MacsecCustomTagSelect_t;

/*!
 * @brief Rudimentary control packet classification rules
 * @ingroup filter_cfg_fns
 * @sa MacsecPacketFilter_t
*/ 
typedef struct {
   MacsecFilterEtype2_t  etype[MACSEC_FILTER_ETYPE_RULES];      //!< Ethertype rules
   MacsecFilterDa_t      da[MACSEC_FILTER_DA_RULES];            //!< MAC-DA rules
   MacsecFilterDaRange_t daRange[MACSEC_FILTER_DA_RANGE_RULES]; //!< MAC-DA range rules
   MacsecFilterCombo_t   comboRule[MACSEC_FILTER_COMBO_RULES];  //!< Ethertype + MAC-DA rules
   MacsecFilterMac_t     mac;                                   //!< Additional mac-DA rule
} MacsecCtrlRules_t;

/*!
 * @brief Configuration for the ingress or egress filters;
 *
 * @sa MacsecSetPacketFilter
 * @sa MacsecGetPacketFilter
 *
 * @ingroup filter_cfg_fns
 */
typedef struct {
    // Parts of PEX_CONFIGURATION
    bool                     customHeaderEn[MACSEC_PORT_COUNT];  // !< enable 8B header before DA (per port)
    bool                     vlanAfterCustomEn;                  // !< extract packets with VLAN tags after custom tags
    bool                     nonDixErrEn;                        // !< enable a parse error for non-DIX frames
    MacsecCtrlRules_t        ctrl_rules;                         // !< Set of rudimentary control classifier rules
    MacsecFilterCustomTag_t  vlanTag[MACSEC_VLAN_TAGS];          // !< VLAN/Custom Tag (CUSTOM_TAG[7..0]
    MacsecFilterEtype_t      mplsTag[MACSEC_MPLS_TAGS];          // !< MPLS etype
    MacsecFilterEtype_t      sectag;                             // !< SecTAG tag
    MacsecFilterEtype_t      ipv4;                               // !< IPv4 tag
    MacsecFilterEtype_t      ipv6;                               // !< IPv6 tag
    MacsecFilterEtype_t      optRxMcsHeader;                     // !< EthType for optional TX-MCS and RX-MCS headers
    MacsecFilterEtype_t      optTxMcsHeader;                     // !< EthType for optional TX-MCS and RX-MCS headers
    MacsecFilterUdp_t        udp;                                // !< UDP protocol filter
    MacsecFilterPtp_t        ptp[MACSEC_NUM_PTP_RULES];          // !< PTP UDP port filter
    MacsecCustomTagSelect_t  customTagSelect[MACSEC_PORT_COUNT]; // !< Egress only. \see MacsecCustomTagSelect_t
} MacsecPacketFilter_t;

//------------------------------------------------------------------------------
// SecY Control
//------------------------------------------------------------------------------

/*! 
 * @struct Ra01IfRsMcsCpmRxSecyPlcy_t
 * @brief ingress SecY policy table
 *
 * Contains all fields of the secy policy table
 * thereby defining the policy for a particular SECY
 *
 * @var Ra01IfRsMcsCpmRxSecyPlcy_t::controlledPortEnabled
 * Enable (or disable) operation of the Controlled port associated with this SecY. 
 * This can be used to disable the Controlled port and drop all data packets until 
 * the secure connectivity has been fully established.
 *
 * @var Ra01IfRsMcsCpmRxSecyPlcy_t::preSectagAuthEnable
 * When set, the outer DA/SA bytes are included in the authentication 
 * GHASH calculation. Both standard MACsec and WAN based MACsec with VLAN tags 
 * in the clear require the DA+SA to be included in the authentication.
 *
 * @var Ra01IfRsMcsCpmRxSecyPlcy_t::replayProtect
 * Enable anti-replay protection
 *
 * @var Ra01IfRsMcsCpmRxSecyPlcy_t::replayWindow
 * Unsigned value indicating the size of the anti-replay window. 
 * The incoming packet PN must be greater than or equal to the 
 * associated next_pn (sa_pn_table_mem) minus this value or the packet must be dropped. 
 *
 * @var Ra01IfRsMcsCpmRxSecyPlcy_t::cipher
 * Defines the cipher suite to use for this SecY \n 
 * This is an enum with the following supported options: \n 
 * 0 = GCM-AES-128 \n 
 * 1 = GCM-AES-256 \n 
 * 2 = GCM-AES-XPN-128 \n 
 * 3 = GCM-AES-XPN-256 
 *
 * @var Ra01IfRsMcsCpmRxSecyPlcy_t::confidentialityOffset
 * Defines the number of bytes that are unencrypted following the SecTag.
 *
 * @var Ra01IfRsMcsCpmRxSecyPlcy_t::stripSectagIcv
 * 2'b00 = Strip both SecTag and ICV from packet \n 
 * 2'b01 = Reserved. \n 
 * 2'b10 = Preserve SecTag, Strip ICV. \n 
 * 2'b11 = Preserve both SecTag and ICV. 
 *
 * @var Ra01IfRsMcsCpmRxSecyPlcy_t::validateFrames
 * Defines the permit policy for tagged frames as defined in 802.1ae. Encoded as follows: \n 
 * 0 = DISABLED: Disable validation. \n 
 * 1 = CHECK: Enable validation, do not discard invalid frames. \n 
 * 2 = STRICT: Enable validation and discard invalid frames. \n 
 * 3 = NULL: No processing or accounting. 
 *
 * @var Ra01IfRsMcsCpmRxSecyPlcy_t::reserved
 * Must be programmed to 0
 *
 * @sa MacsecReadSecyPolicy
 * @sa MacsecWriteSecyPolicy
 *
 * @ingroup secy_if_fns
 */

typedef Ra01IfRsMcsCpmRxSecyPlcy_t IngressSecyPolicyTable_t;

/*! 
 * @struct Ra01IfRsMcsCpmTxSecyPlcy_t
 * @brief egress SecY policy table
 *
 * Contains all fields of the secy policy table
 * thereby defining the policy for a particular SECY
 * 
 * @var Ra01IfRsMcsCpmTxSecyPlcy_t::controlledPortEnabled
 * Enable (or disable) operation of the Controlled port associated 
 * with this SecY. This can be used to disable the Controlled port 
 * and drop all data packets until the secure connectivity has been fully established.
 * 
 * @var Ra01IfRsMcsCpmTxSecyPlcy_t::preSectagAuthEnable
 * When set, include the region [pre_sectag_auth_start:pre_sectag_auth_end] 
 * in the generation of the ICV (i.e. in the set of authenticated bytes)
 * 
 * @var Ra01IfRsMcsCpmTxSecyPlcy_t::protectFrames
 * 0 = do not encrypt or authenticate this packet \n 
 * 1 = always Authenticate frame and if SecTag.TCI.E = 1 encrypt the packet as well. 
 * 
 * @var Ra01IfRsMcsCpmTxSecyPlcy_t::cipher
 * Defines the cipher suite to use for this SecY \n 
 * This is an enum with the following supported options: \n 
 * 0= GCM-AES-128 \n 
 * 1= GCM-AES-256 \n 
 * 2= GCM-AES-XPN-128 \n 
 * 3= GCM-AES-XPN-256 
 * 
 * @var Ra01IfRsMcsCpmTxSecyPlcy_t::confidentialityOffset
 * Defines the number of bytes that are unencrypted following the SecTag.
 * 
 * @var Ra01IfRsMcsCpmTxSecyPlcy_t::sectagOffset
 * SecTag can only be inserted into the first 128B of the frame:
 *    - If sectag_insert_mode is set to "Relative Offset Mode" This field 
 *      defines the offset in bytes from a matching Etype
 *    - If sectag_insert_mode is set to "Fixed Offset Mode" 
 *      This field defines the offset in bytes from from 
 *      the start of the MAC_DA of the packet.
 * \note 
 *    - If a TX MCS header is present, then the CPM will add the value in the 
 *      FIXED_OFFSET_ADJUST register to this field before sending to the BBE.
 *    - If an 8B prepended header is present, then the CPM will add the 8 to this field before sending to the BBE. 
 * 
 * @var Ra01IfRsMcsCpmTxSecyPlcy_t::sectagTci
 * Tag Control Information excluding the AN field which originates 
 * from the SA Policy table. This field is inserted into the SecTag of the outgoing packet.
 * 
 * @var Ra01IfRsMcsCpmTxSecyPlcy_t::mtu
 * Specifies the outgoing maximum transmission unit (MTU) in bytes for this SecY. 
 * The MTU must be checked on egress to ensure compliance with the configured MTU 
 * due to the expansion that occurs because of inserting the SecTag and ICV into the frame. 
 * MTU violation causes CRC corruption in the outgoing frame. 
 * The MTU is checked on EOP by comparing this value against the actual computed packet length. 
 * Violation causes the packet to be truncated and EOP-errored. Note that the maximum legal value is (2^14)-1 
 *
 * @sa MacsecReadSecyPolicy
 * @sa MacsecWriteSecyPolicy
 *
 * @ingroup secy_if_fns
 */

typedef Ra01IfRsMcsCpmTxSecyPlcy_t EgressSecyPolicyTable_t;

/*!
 * @brief Generic SecY policy table
 *
 * Union of ingress and egress for the API.
 *
 * @sa MacsecWriteSecyPolicy
 * @sa MacsecReadSecyPolicy
 *
 * @ingroup secy_if_fns
 */
typedef union {
    IngressSecyPolicyTable_t     ingress;        //!< Ingress policy
    EgressSecyPolicyTable_t      egress;         //!< Egress policy
} MacsecSecyPolicyTable_t;


//------------------------------------------------------------------------------
// T-CAM Tables
//------------------------------------------------------------------------------

/*!
* @brief tcam key
*
* This contains all fields of a tcam key
*
* @ingroup flow_if_fns
*
* @sa CpmRxFlowCfg_t
* @sa CpmTxFlowCfg_t
*/
typedef struct {
  bool     express;             // !< true-express, false-preempt
  bool     port;                // !< port, always 0
  uint8_t  tagMatchBitmap;      /**< Indicates which vlan (or custom) tag \b index was found in the packet (\see MacsecFilterCustomTag_t)
                                     For example, if MacsecPacketFilter_t::vlanTag[2].vlanIndx == 4 and this particular tag was found in the packet
                                     then tagMatchBitmap[4] == 1 (\note tagMatchBitmap[2] does not get set) */
  uint16_t bonusData;           /**< 2 bytes of additional bonus data extracted from the packet. If any tags are programmed to extract bonus data (\see MacsecFilterCustomTag_t)
                                     then these 2B will correspond to those found in the first tag parsed in the packet to have vlanBonus set. If no tags are found corresponding
                                     to those programmed with vlanBonus, then the parser will extract 16'h0. If no tags are programmed for bonus extraction 
                                     (i.e. all vlanTag[*].vlanBonus == 0), then the parser will extract the first 2B after the final etype parsed. */
  uint16_t etherType;           // !< First E-Type found in the packet that doesn't match one of the preconfigured values.
  uint8_t innerVlanType;       /**< Encoded value indicating which VLAN TPID value matched for the second outermost VLAN Tag. 
                                     This value corresponds to the \b index of the second vlan/custom tag encountered (\see MacsecFilterCustomTag_t). 
                                     For example, if MacsecPacketFilter_t::vlanTag[3].vlanIndx == 1 and the second 
                                     tag encountered matches MacsecPacketFilter_t::vlanTag[3].vlanEtype, then the parser will extract a value of 1 for the innerVlanType */
  uint8_t numTags;             /**< Number of VLAN tags, CUSTOM tags, or MPLS labels before the SecTAG, excluding the Rx/Tx-MCS header tags. This is a one-hot encoded vector \n 
                                     Bit 0: no tags/labels before SecTAG \n 
                                     Bit 1: 1 tag/label before SecTAG \n 
                                     Bit 2: 2 tags/labels before SecTAG \n 
                                     ... \n 
                                     Bit 6: 6 or more tags/labels before SecTAG. */
  uint8_t outerPriority;       // !< Outermost VLAN {PCP/Pbits, DE/CFI}
  uint32_t outerTagId;          // !< Outermost VLAN ID {8'd0, VLAN_ID[11:0]}
  uint8_t outerVlanType;       /**< Encoded value indicating which VLAN TPID value matched for the outermost (i.e. first encountered) VLAN/custom Tag.
                                     This value corresponds to the \b index of the first vlan/custom tag encountered (\see MacsecFilterCustomTag_t). 
                                     For example, if MacsecPacketFilter_t::vlanTag[2].vlanIndx == 7 and the first 
                                     tag encountered matches MacsecPacketFilter_t::vlanTag[2].vlanEtype, then the parser will extract a value of 7 for the outerVlanType */
  uint8_t packetType;          // !< Encoded Packet Type from the parser: 0-noVLAN, 1-1VLAN, 2-2+VLAN
  uint8_t secondOuterPriority; // !< 2nd Outermost VLAN {PCP/Pbits, DE/CFI}
  uint32_t secondOuterTagId;    // !< 2nd Outermost VLAN ID {8'd0, VLAN_ID[11:0]}
  uint64_t macDa;               // !< dest address
  uint64_t macSa;               // !< source address
} CpmFlowIdTcamEntry_t;

/*!
* @brief ingress flow-specific configuration
* @ingroup flow_if_fns
* @sa CpmFlowCfg_t
*
* \note this is a data/mask tcam, so a key will match on entry `i` if `key & ~mask[i] == data[i] & ~mask[i]`
*/
typedef struct {
   CpmFlowIdTcamEntry_t data; //!< data portion of the TCAM entry
   CpmFlowIdTcamEntry_t mask; //!< mask portion of the TCAM entry
} CpmRxFlowCfg_t;

/*!
* @brief egress flow-specific configuration
* @ingroup flow_if_fns
* @sa CpmFlowCfg_t
*
* \note this is a data/mask tcam, so a key will match on entry `i` if `key & ~mask[i] == data[i] & ~mask[i]`
*/
typedef struct {
   CpmFlowIdTcamEntry_t data;            //!< data portion of the TCAM entry
   CpmFlowIdTcamEntry_t mask;            //!< mask portion of the TCAM entry
   unsigned             sc;              //!< Flow ID to SecY map SC table index
   uint64_t             sci;             //!< Flow ID to SecY map SCI
   bool                 auxiliaryPolicy; //!< true to set the downstream aux. policy bit
} CpmTxFlowCfg_t;

 /*!
  * @brief Flow-specific configuration fo egress and ingress
  * @ingroup flow_if_fns
  * @sa MacsecFlowCfg_t
  */
typedef union {
   CpmRxFlowCfg_t ingress;
   CpmTxFlowCfg_t egress;
} CpmFlowCfg_t; 

/*!
 * @brief Struct to specify all configurations related to a flow
 *
 * provides values for programming the flowID matching rules
 * in addition to any tables indexed by flowID
 *
 * @sa MacsecReadTcam
 * @sa MacsecWriteTcam
 *
 * @sa MacsecConfigureTcam
 * @sa MacsecReadTcam
 *
 * @ingroup flow_if_fns
 */
typedef struct {
    bool     controlPacket; //!< If this flowID maps to the uncontrolled port (i.e. its a control packet), then set this bit to 1, else set to 0
    unsigned secY;          //!< The SecY this flowID maps to
    CpmFlowCfg_t d;         //!< Remainder of all flow related configuration for both directions
} MacsecFlowCfg_t;


/*******************************************************************************
 *                                                                             *
 *  TCAM Functions                                                             *
 *                                                                             *
 *******************************************************************************/

/*!
 * @brief Read the current values of a TCAM
 *
 * @param[inout]  rmsDev_p   The hardware handle for the target device
 * @param[in]     index      The index of the TCAM being queried
 * @param[in]     direction  The direction of the TCAM
 * @param[out]    tcam       The TCAM configuration data
 *
 * @retval  0       Success
 * @retval  int     Return value from hardware read
 *
 * @ingroup flow_if_fns
 */
DLL_PUBLIC int MacsecReadTcam( RmsDev_t *           rmsDev_p,
                               MacsecDirection_t    direction,
                               unsigned             index,
                               MacsecFlowCfg_t * tcam);

/*!
 * @brief Write the current values of a TCAM
 *
 * @param[inout]  rmsDev_p   The hardware handle for the target device
 * @param[in]     direction  The direction of the TCAM
 * @param[in]     index      The index of the TCAM being configured
 * @param[in]     tcam       The TCAM configuration data
 *
 * @retval  0       Success
 * @retval  int     Return value from hardware write
 *
 * @ingroup flow_if_fns
 */
DLL_PUBLIC int MacsecWriteTcam( RmsDev_t *           rmsDev_p,
                                MacsecDirection_t    direction,
                                unsigned             index,
                                MacsecFlowCfg_t * tcam );

/*!
 * @brief Enable/Disable a TCAM.
 *
 * @param[inout]  rmsDev_p   The hardware handle for the target device
 * @param[in]     direction  The direction of the TCAM
 * @param[in]     index      The index of the TCAM being enabled
 * @param[in]     enable     True to enable, false to disable
 *
 * @retval  0       Success
 * @retval  int     Return value from hardware write
 *
 * @ingroup flow_if_fns
 *
 */
DLL_PUBLIC int MacsecEnableTcam( RmsDev_t *        rmsDev_p,
                                 MacsecDirection_t direction,
                                 unsigned          index,
                                 bool              enable );


#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 *                                                                             *
 *  Filter Functions                                                           *
 *                                                                             *
 *******************************************************************************/
/*!
 * @brief Configure the ingress or egress filter table
 *
 * @param[inout] rmsDev_p   The hardware handle for the target device
 * @param[in]    direction  Whether this is an ingress or egress filter
 * @param[in]    filter     The filter parameters
 *
 * @retval  0       Success
 * @retval  int     Return value from hardware write
 *
 * NOTE: These parameters may be set individually via specific functions.
 *
 * @ingroup filter_cfg_fns
 */
DLL_PUBLIC int MacsecSetPacketFilter( RmsDev_t *             rmsDev_p,
                                      MacsecDirection_t      direction,
                                      MacsecPacketFilter_t * filter );

/*!
 * @brief Read the configuration of the ingress or egress filter table
 *
 * @param[inout] rmsDev_p   The hardware handle for the target device
 * @param[in]    direction  Whether this is an ingress or egress filter
 * @param[in]    filter     The filter parameters
 *
 * @retval  0       Success
 * @retval  int     Return value from hardware read
 *
 * NOTE: These parameters may be read individually via specific functions.
 *
 * @ingroup filter_cfg_fns
 */
DLL_PUBLIC int MacsecGetPacketFilter( RmsDev_t *             rmsDev_p,
                                      MacsecDirection_t      direction,
                                      MacsecPacketFilter_t * filter );

/*******************************************************************************
 *                                                                             *
 *  SecY Functions                                                             *
 *                                                                             *
 *******************************************************************************/
/*!
 * @brief Read the configuration from a SecY.
 *
 * @param[inout] rmsDev_p     The hardware handle for the target device
 * @param[in]    index        The index for the SecY being queried
 * @param[in]    direction    The direction of the SecY
 * @param[out]   policy       The policy of the SecY
 *
 * @retval  0       Success
 * @retval -EINVAL  Value is out of range
 * @retval  int     Return value from hardware read
 *
 * @sa MacsecEnableAutoRekey
 *
 * @ingroup secy_if_fns
 */
DLL_PUBLIC int MacsecReadSecyPolicy( RmsDev_t *                rmsDev_p,
                                     MacsecDirection_t         direction,
                                     unsigned                  index,
                                     MacsecSecyPolicyTable_t * policy );

/*!
 * @brief Configure a SecY.
 *
 * @param[inout] rmsDev_p     The hardware handle for the target device
 * @param[in]    index        The handle for the SecY being configured
 * @param[in]    direction    The direction of the SecY
 * @param[in]    policy       The policy to set for the SecY
 *
 * @retval  0       Success
 * @retval -EINVAL  Value is out of range
 * @retval  int     Return value from hardware write
 *
 * @ingroup secy_if_fns
 */
DLL_PUBLIC int MacsecWriteSecyPolicy( RmsDev_t *                      rmsDev_p,
                                      MacsecDirection_t               direction,
                                      unsigned                        index,
                                      MacsecSecyPolicyTable_t *       policy );


/*******************************************************************************
 *                                                                             *
 *  SC Functions                                                               *
 *                                                                             *
 *******************************************************************************/
/*!
 * @brief Get the SCI for an Ingress (Rx) SC
 *
 * @param[inout]  rmsDev_p   The hardware handle for the target device
 * @param[in]     index      The SC table index
 * @param[out]    secY       The SecY index to set in the SC CAM table.
 * @param[out]    sci        The SCI to set in the SC CAM table.
 * @param[out]    enable     Whether this entry is enabled.
 *
 * @retval  0       Success
 * @retval -EINVAL  Value is out of range
 * @retval  int     Return value from hardware write
 *
 * This function can be used to get both the Ingress SC CAM Lookup Key entry for
 * the SC and the SC CAM Enabled bit to indicate if this entry should be
 * considered in SC CAM matching.
 *
 * @ingroup sc_if_fns
 */
DLL_PUBLIC int MacsecGetIngressScCamSci( RmsDev_t * rmsDev_p,
                                         unsigned index,
                                         unsigned * secY,
                                         uint64_t * sci,
                                         bool * enable );

/*!
 * @brief Set the SCI for an Ingress (Rx) SC
 *
 * @param[inout]  rmsDev_p   The hardware handle for the target device
 * @param[in]     index      The SC table index
 * @param[in]     secY       The SecY index to set in the SC CAM table.
 * @param[in]     sci        The SCI to set in the SC CAM table.
 * @param[in]     enable     Whether this entry is enabled.
 *
 * @retval  0       Success
 * @retval -EINVAL  Value is out of range
 * @retval  int     Return value from hardware write
 *
 * This function can be used to set both the Ingress SC CAM Lookup Key entry for
 * the SC and the SC CAM Enabled bit to indicate if this entry should be
 * considered in SC CAM matching.
 *
 * @ingroup sc_if_fns
 */
DLL_PUBLIC int MacsecSetIngressScCamSci( RmsDev_t * rmsDev_p,
                                         unsigned   index,
                                         unsigned   secY,
                                         uint64_t   sci,
                                         bool       enable );

#ifdef __cplusplus
}
#endif

#endif // __MACSEC_IEEE_API_H__

