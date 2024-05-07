/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*/
/**
********************************************************************************
* @file prvCpssDxChPort.h
*
* @brief Includes structures definition for the use of CPSS DxCh Port lib .
*
*
* @version   58
********************************************************************************
*/
#ifndef __prvCpssDxChPortStath
#define __prvCpssDxChPortStath

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <cpss/common/cpssTypes.h>

/**
* @enum PRV_CPSS_PORT_MAC_MTI_RX_COUNTER_ENT
 *
 * @brief Enumeration of the ethernet MTI MAC RX Counters.
*/
typedef enum{

    /** @brief
     */
    PRV_CPSS_MTI_RX_ETHER_STATS_OCTETS_E = 0,

    /** @brief
     */
    PRV_CPSS_MTI_RX_OCTETS_RECEIVED_OK_E = 1,

    /** @brief
     */
    PRV_CPSS_MTI_RX_A_ALIGNMENT_ERRORS_E = 2,

    /** @brief
     */
    PRV_CPSS_MTI_RX_A_PAUSE_MAC_CTRL_FRAMES_RECIEVED_E = 3,

    /** @brief
     */
    PRV_CPSS_MTI_RX_A_FRAME_TOO_LONG_ERRORS_E = 4,

    /** @brief
     */
    PRV_CPSS_MTI_RX_A_IN_RANGE_LENGTH_ERRORS_E = 5,

    /** @brief
     */
    PRV_CPSS_MTI_RX_A_FRAMES_RECIEVED_OK_E = 6,

    /** @brief
     */
    PRV_CPSS_MTI_RX_A_FRAME_CHECK_SEQUENCE_ERRORS_E = 7,

    /** @brief
     */
    PRV_CPSS_MTI_RX_VLAN_RECIEVED_OK_E = 8,

    /** @brief
     */
    PRV_CPSS_MTI_RX_IF_IN_ERRORS_E = 9,

    /** @brief
     */
    PRV_CPSS_MTI_RX_IF_IN_UC_PKTS_E = 10,

    /** @brief
     */
    PRV_CPSS_MTI_RX_IF_IN_MC_PKTS_E = 11,

    /** @brief
     */
    PRV_CPSS_MTI_RX_IF_IN_BC_PKTS_E = 12,

    /** @brief
     */
    PRV_CPSS_MTI_RX_ETHER_STATS_DROP_EVENTS_E = 13,

    /** @brief
     */
    PRV_CPSS_MTI_RX_ETHER_STATS_PKTS_E = 14,

    /** @brief
     */
    PRV_CPSS_MTI_RX_ETHER_STATS_UNDERSIZE_PKTS_E = 15,

    /** @brief
     */
    PRV_CPSS_MTI_RX_ETHER_STATS_PTKS_64OCTETS_E = 16,

    /** @brief
     */
    PRV_CPSS_MTI_RX_ETHER_STATS_PTKS_65TO127_OCTETS_E = 17,

    /** @brief
     */
    PRV_CPSS_MTI_RX_ETHER_STATS_PTKS_128TO255_OCTETS_E = 18,

    /** @brief
     */
    PRV_CPSS_MTI_RX_ETHER_STATS_PTKS_256TO511_OCTETS_E = 19,

    /** @brief
     */
    PRV_CPSS_MTI_RX_ETHER_STATS_PTKS_512TO1023_OCTETS_E = 20,

    /** @brief
     */
    PRV_CPSS_MTI_RX_ETHER_STATS_PTKS_1024TO1518_OCTETS_E = 21,

    /** @brief
     */
    PRV_CPSS_MTI_RX_ETHER_STATS_PTKS_1519TOMAX_OCTETS_E = 22,

    /** @brief
     */
    PRV_CPSS_MTI_RX_ETHER_STATS_OVERSIZE_PKTS_E = 23,

    /** @brief
     */
    PRV_CPSS_MTI_RX_ETHER_STATS_JABBERS_E = 24,

    /** @brief
     */
    PRV_CPSS_MTI_RX_ETHER_STATS_FRAGMENTS_E = 25,

    /** @brief
     */
    PRV_CPSS_MTI_RX_A_CBFC_PAUSE_FRAMES_RECIEVED_0_E = 26,

    /** @brief
     */
    PRV_CPSS_MTI_RX_A_CBFC_PAUSE_FRAMES_RECIEVED_1_E = 27,

    /** @brief
     */
    PRV_CPSS_MTI_RX_A_CBFC_PAUSE_FRAMES_RECIEVED_2_E = 28,

    /** @brief
     */
    PRV_CPSS_MTI_RX_A_CBFC_PAUSE_FRAMES_RECIEVED_3_E = 29,

    /** @brief
     */
    PRV_CPSS_MTI_RX_A_CBFC_PAUSE_FRAMES_RECIEVED_4_E = 30,

    /** @brief
     */
    PRV_CPSS_MTI_RX_A_CBFC_PAUSE_FRAMES_RECIEVED_5_E = 31,

    /** @brief
     */
    PRV_CPSS_MTI_RX_A_CBFC_PAUSE_FRAMES_RECIEVED_6_E = 32,

    /** @brief
     */
    PRV_CPSS_MTI_RX_A_CBFC_PAUSE_FRAMES_RECIEVED_7_E = 33,

    /** @brief
     */
    PRV_CPSS_MTI_RX_A_CBFC_PAUSE_FRAMES_RECIEVED_8_E = 34,

    /** @brief
     */
    PRV_CPSS_MTI_RX_A_CBFC_PAUSE_FRAMES_RECIEVED_9_E = 35,

    /** @brief
     */
    PRV_CPSS_MTI_RX_A_CBFC_PAUSE_FRAMES_RECIEVED_10_E = 36,

    /** @brief
     */
    PRV_CPSS_MTI_RX_A_CBFC_PAUSE_FRAMES_RECIEVED_11_E = 37,

    /** @brief
     */
    PRV_CPSS_MTI_RX_A_CBFC_PAUSE_FRAMES_RECIEVED_12_E = 38,

    /** @brief
     */
    PRV_CPSS_MTI_RX_A_CBFC_PAUSE_FRAMES_RECIEVED_13_E = 39,

    /** @brief
     */
    PRV_CPSS_MTI_RX_A_CBFC_PAUSE_FRAMES_RECIEVED_14_E = 40,

    /** @brief
     */
    PRV_CPSS_MTI_RX_A_CBFC_PAUSE_FRAMES_RECIEVED_15_E = 41,

    /** @brief
     */
    PRV_CPSS_MTI_RX_A_MAC_CONTROL_FRAMES_RECIEVED_E = 42,

    PRV_CPSS_MTI_RX_LAST_E

} PRV_CPSS_PORT_MAC_MTI_RX_COUNTER_ENT;


/**
* @enum PRV_CPSS_SIP7_PORT_MAC_MTI_RX_COUNTER_ENT
 *
 * @brief Enumeration of the ethernet MTI MAC RX Counters.
*/
typedef enum{

    /** @brief
     */
    PRV_CPSS_SIP7_MTI_RX_ETHER_STATS_OCTETS_E = 0,

    /** @brief
     */
    PRV_CPSS_SIP7_MTI_RX_OCTETS_RECEIVED_OK_E = 1,

    /** @brief
     */
    PRV_CPSS_SIP7_MTI_RX_A_ALIGNMENT_ERRORS_E = 2,

    /** @brief
     */
    PRV_CPSS_SIP7_MTI_RX_A_PAUSE_MAC_CTRL_FRAMES_RECIEVED_E = 3,

    /** @brief
     */
    PRV_CPSS_SIP7_MTI_RX_A_FRAME_TOO_LONG_ERRORS_E = 4,

    /** @brief
     */
    PRV_CPSS_SIP7_MTI_RX_A_IN_RANGE_LENGTH_ERRORS_E = 5,

    /** @brief
     */
    PRV_CPSS_SIP7_MTI_RX_A_FRAMES_RECIEVED_OK_E = 6,

    /** @brief
     */
    PRV_CPSS_SIP7_MTI_RX_A_FRAME_CHECK_SEQUENCE_ERRORS_E = 7,

    /** @brief
     */
    PRV_CPSS_SIP7_MTI_RX_VLAN_RECIEVED_OK_E = 8,

    /** @brief
     */
    PRV_CPSS_SIP7_MTI_RX_IF_IN_ERRORS_E = 9,

    /** @brief
     */
    PRV_CPSS_SIP7_MTI_RX_IF_IN_UC_PKTS_E = 10,

    /** @brief
     */
    PRV_CPSS_SIP7_MTI_RX_IF_IN_MC_PKTS_E = 11,

    /** @brief
     */
    PRV_CPSS_SIP7_MTI_RX_IF_IN_BC_PKTS_E = 12,

    /** @brief
     */
    PRV_CPSS_SIP7_MTI_RX_ETHER_STATS_DROP_EVENTS_E = 13,

    /** @brief
     */
    PRV_CPSS_SIP7_MTI_RX_ETHER_STATS_PKTS_E = 14,

    /** @brief
     */
    PRV_CPSS_SIP7_MTI_RX_ETHER_STATS_UNDERSIZE_PKTS_E = 15,

    /** @brief
     */
    PRV_CPSS_SIP7_MTI_RX_ETHER_STATS_PTKS_64OCTETS_E = 16,

    /** @brief
     */
    PRV_CPSS_SIP7_MTI_RX_ETHER_STATS_PTKS_65TO127_OCTETS_E = 17,

    /** @brief
     */
    PRV_CPSS_SIP7_MTI_RX_ETHER_STATS_PTKS_128TO255_OCTETS_E = 18,

    /** @brief
     */
    PRV_CPSS_SIP7_MTI_RX_ETHER_STATS_PTKS_256TO511_OCTETS_E = 19,

    /** @brief
     */
    PRV_CPSS_SIP7_MTI_RX_ETHER_STATS_PTKS_512TO1023_OCTETS_E = 20,

    /** @brief
     */
    PRV_CPSS_SIP7_MTI_RX_ETHER_STATS_PTKS_1024TO1518_OCTETS_E = 21,

    /** @brief
     */
    PRV_CPSS_SIP7_MTI_RX_ETHER_STATS_PTKS_1519TOMAX_OCTETS_E = 22,

    /** @brief
     */
    PRV_CPSS_SIP7_MTI_RX_ETHER_STATS_PKTS_1519TO2047_OCTETS_E = 23,

    /** @brief
     */
    PRV_CPSS_SIP7_MTI_RX_ETHER_STATS_PKTS_2048TO4095_OCTETS_E = 24,

    /** @brief
     */
    PRV_CPSS_SIP7_MTI_RX_ETHER_STATS_PKTS_4096TO8191_OCTETS_E = 25,

    /** @brief
     */
    PRV_CPSS_SIP7_MTI_RX_ETHER_STATS_PKTS_8192TO9216_OCTETS_E = 26,

    /** @brief
     */
    PRV_CPSS_SIP7_MTI_RX_ETHER_STATS_PKTS_9217TO16383_OCTETS_E = 27,

    /** @brief
     */
    PRV_CPSS_SIP7_MTI_RX_ETHER_STATS_PKTS_BIGGERTHAN16383_E = 28,

    /** @brief
     */
    PRV_CPSS_SIP7_MTI_RX_ETHER_STATS_OVERSIZE_PKTS_E = 29,

    /** @brief
     */
    PRV_CPSS_SIP7_MTI_RX_ETHER_STATS_JABBERS_E = 30,

    /** @brief
     */
    PRV_CPSS_SIP7_MTI_RX_ETHER_STATS_FRAGMENTS_E = 31,

    /** @brief
     */
    PRV_CPSS_SIP7_MTI_RX_A_CBFC_PAUSE_FRAMES_RECIEVED_0_E = 32,

    /** @brief
     */
    PRV_CPSS_SIP7_MTI_RX_A_CBFC_PAUSE_FRAMES_RECIEVED_1_E = 33,

    /** @brief
     */
    PRV_CPSS_SIP7_MTI_RX_A_CBFC_PAUSE_FRAMES_RECIEVED_2_E = 34,

    /** @brief
     */
    PRV_CPSS_SIP7_MTI_RX_A_CBFC_PAUSE_FRAMES_RECIEVED_3_E = 35,

    /** @brief
     */
    PRV_CPSS_SIP7_MTI_RX_A_CBFC_PAUSE_FRAMES_RECIEVED_4_E = 36,

    /** @brief
     */
    PRV_CPSS_SIP7_MTI_RX_A_CBFC_PAUSE_FRAMES_RECIEVED_5_E = 37,

    /** @brief
     */
    PRV_CPSS_SIP7_MTI_RX_A_CBFC_PAUSE_FRAMES_RECIEVED_6_E = 38,

    /** @brief
     */
    PRV_CPSS_SIP7_MTI_RX_A_CBFC_PAUSE_FRAMES_RECIEVED_7_E = 39,

    /** @brief
     */
    PRV_CPSS_SIP7_MTI_RX_A_MAC_CONTROL_FRAMES_RECIEVED_E = 40,

    PRV_CPSS_SIP7_MTI_RX_LAST_E

} PRV_CPSS_SIP7_PORT_MAC_MTI_RX_COUNTER_ENT;

/**
* @enum PRV_CPSS_PORT_MAC_MTI_TX_COUNTER_ENT
 *
 * @brief Enumeration of the ethernet MTI MAC TX Counters.
*/
typedef enum{

    /** @brief
     */
    PRV_CPSS_MTI_TX_ETHER_STATS_OCTETS_E = 0,

    /** @brief
     */
    PRV_CPSS_MTI_TX_OCTETS_TRANSMITTED_OK_E = 1,

    /** @brief
     */
    PRV_CPSS_MTI_TX_A_PAUSE_MAC_CTRL_FRAMES_TRANSMITTED_E = 2,

    /** @brief
     */
    PRV_CPSS_MTI_TX_A_FRAMES_TRANSMITTED_OK_E = 3,

    /** @brief
     */
    PRV_CPSS_MTI_TX_VLAN_RECIEVED_OK_E = 4,

    /** @brief
     */
    PRV_CPSS_MTI_TX_IF_OUT_ERRORS_E = 5,

    /** @brief
     */
    PRV_CPSS_MTI_TX_IF_OUT_UC_PKTS_E = 6,

    /** @brief
     */
    PRV_CPSS_MTI_TX_IF_OUT_MC_PKTS_E = 7,

    /** @brief
     */
    PRV_CPSS_MTI_TX_IF_OUT_BC_PKTS_E = 8,

    /** @brief
     */
    PRV_CPSS_MTI_TX_ETHER_STATS_PTKS_64OCTETS_E = 9,

    /** @brief
     */
    PRV_CPSS_MTI_TX_ETHER_STATS_PTKS_65TO127_OCTETS_E = 10,

    /** @brief
     */
    PRV_CPSS_MTI_TX_ETHER_STATS_PTKS_128TO255_OCTETS_E = 11,

    /** @brief
     */
    PRV_CPSS_MTI_TX_ETHER_STATS_PTKS_256TO511_OCTETS_E = 12,

    /** @brief
     */
    PRV_CPSS_MTI_TX_ETHER_STATS_PTKS_512TO1023_OCTETS_E = 13,

    /** @brief
     */
    PRV_CPSS_MTI_TX_ETHER_STATS_PTKS_1024TO1518_OCTETS_E = 14,

    /** @brief
     */
    PRV_CPSS_MTI_TX_ETHER_STATS_PTKS_1519TOMAX_OCTETS_E = 15,

    /** @brief
     */
    PRV_CPSS_MTI_TX_A_CBFC_PAUSE_FRAMES_TRANSMITTED_0_E = 16,

    /** @brief
     */
    PRV_CPSS_MTI_TX_A_CBFC_PAUSE_FRAMES_TRANSMITTED_1_E = 17,

    /** @brief
     */
    PRV_CPSS_MTI_TX_A_CBFC_PAUSE_FRAMES_TRANSMITTED_2_E = 18,

    /** @brief
     */
    PRV_CPSS_MTI_TX_A_CBFC_PAUSE_FRAMES_TRANSMITTED_3_E = 19,

    /** @brief
     */
    PRV_CPSS_MTI_TX_A_CBFC_PAUSE_FRAMES_TRANSMITTED_4_E = 20,

    /** @brief
     */
    PRV_CPSS_MTI_TX_A_CBFC_PAUSE_FRAMES_TRANSMITTED_5_E = 21,

    /** @brief
     */
    PRV_CPSS_MTI_TX_A_CBFC_PAUSE_FRAMES_TRANSMITTED_6_E = 22,

    /** @brief
     */
    PRV_CPSS_MTI_TX_A_CBFC_PAUSE_FRAMES_TRANSMITTED_7_E = 23,

    /** @brief
     */
    PRV_CPSS_MTI_TX_A_CBFC_PAUSE_FRAMES_TRANSMITTED_8_E = 24,

    /** @brief
     */
    PRV_CPSS_MTI_TX_A_CBFC_PAUSE_FRAMES_TRANSMITTED_9_E = 25,

    /** @brief
     */
    PRV_CPSS_MTI_TX_A_CBFC_PAUSE_FRAMES_TRANSMITTED_10_E = 26,

    /** @brief
     */
    PRV_CPSS_MTI_TX_A_CBFC_PAUSE_FRAMES_TRANSMITTED_11_E = 27,

    /** @brief
     */
    PRV_CPSS_MTI_TX_A_CBFC_PAUSE_FRAMES_TRANSMITTED_12_E = 28,

    /** @brief
     */
    PRV_CPSS_MTI_TX_A_CBFC_PAUSE_FRAMES_TRANSMITTED_13_E = 29,

    /** @brief
     */
    PRV_CPSS_MTI_TX_A_CBFC_PAUSE_FRAMES_TRANSMITTED_14_E = 30,

    /** @brief
     */
    PRV_CPSS_MTI_TX_A_CBFC_PAUSE_FRAMES_TRANSMITTED_15_E = 31,

    /** @brief
     */
    PRV_CPSS_MTI_TX_A_MAC_CONTROL_FRAMES_TRANSMITTED_E = 32,

    /** @brief
     */
    PRV_CPSS_MTI_TX_ETHER_STATS_PKTS_E = 33,

    PRV_CPSS_MTI_TX_LAST_E

} PRV_CPSS_PORT_MAC_MTI_TX_COUNTER_ENT;


/**
* @enum PRV_CPSS_SIP7_PORT_MAC_MTI_TX_COUNTER_ENT
 *
 * @brief Enumeration of the ethernet MTI MAC TX Counters.
*/
typedef enum{

    /** @brief
     */
    PRV_CPSS_SIP7_MTI_TX_ETHER_STATS_OCTETS_E = 0,

    /** @brief
     */
    PRV_CPSS_SIP7_MTI_TX_OCTETS_TRANSMITTED_OK_E = 1,

    /** @brief
     */
    PRV_CPSS_SIP7_MTI_TX_A_PAUSE_MAC_CTRL_FRAMES_TRANSMITTED_E = 2,

    /** @brief
     */
    PRV_CPSS_SIP7_MTI_TX_A_FRAMES_TRANSMITTED_OK_E = 3,

    /** @brief
     */
    PRV_CPSS_SIP7_MTI_TX_VLAN_RECIEVED_OK_E = 4,

    /** @brief
     */
    PRV_CPSS_SIP7_MTI_TX_IF_OUT_ERRORS_E = 5,

    /** @brief
     */
    PRV_CPSS_SIP7_MTI_TX_IF_OUT_UC_PKTS_E = 6,

    /** @brief
     */
    PRV_CPSS_SIP7_MTI_TX_IF_OUT_MC_PKTS_E = 7,

    /** @brief
     */
    PRV_CPSS_SIP7_MTI_TX_IF_OUT_BC_PKTS_E = 8,

    /** @brief
     */
    PRV_CPSS_SIP7_MTI_TX_ETHER_STATS_PTKS_64OCTETS_E = 9,

    /** @brief
     */
    PRV_CPSS_SIP7_MTI_TX_ETHER_STATS_PTKS_65TO127_OCTETS_E = 10,

    /** @brief
     */
    PRV_CPSS_SIP7_MTI_TX_ETHER_STATS_PTKS_128TO255_OCTETS_E = 11,

    /** @brief
     */
    PRV_CPSS_SIP7_MTI_TX_ETHER_STATS_PTKS_256TO511_OCTETS_E = 12,

    /** @brief
     */
    PRV_CPSS_SIP7_MTI_TX_ETHER_STATS_PTKS_512TO1023_OCTETS_E = 13,

    /** @brief
     */
    PRV_CPSS_SIP7_MTI_TX_ETHER_STATS_PTKS_1024TO1518_OCTETS_E = 14,

    /** @brief
     */
    PRV_CPSS_SIP7_MTI_TX_ETHER_STATS_PTKS_1519TOMAX_OCTETS_E = 15,

    /** @brief
     */
    PRV_CPSS_SIP7_MTI_TX_ETHER_STATS_PKTS_1519O2047_OCTETS_E = 16,

    /** @brief
     */
    PRV_CPSS_SIP7_MTI_TX_ETHER_STATS_PKTS_2048TO4095_OCTETS_E = 17,

    /** @brief
     */
    PRV_CPSS_SIP7_MTI_TX_ETHER_STATS_PKTS_4096TO8191_OCTETS_E = 18,

    /** @brief
     */
    PRV_CPSS_SIP7_MTI_TX_ETHER_STATS_PKTS_8192TO9216_OCTETS_E = 19,

    /** @brief
     */
    PRV_CPSS_SIP7_MTI_TX_ETHER_STATS_PKTS_9217TO16383_OCTETS_E = 20,

    /** @brief
     */
    PRV_CPSS_SIP7_MTI_TX_ETHER_STATS_PKTS_BIGGERTHAN16383_E = 21,

    /** @brief
     */
    PRV_CPSS_SIP7_MTI_TX_ETHER_STATS_PKTS_MORE_THAN_MTU_E = 22,

    /** @brief
     */
    PRV_CPSS_SIP7_MTI_TX_A_CBFC_PAUSE_FRAMES_TRANSMITTED_0_E = 23,

    /** @brief
     */
    PRV_CPSS_SIP7_MTI_TX_A_CBFC_PAUSE_FRAMES_TRANSMITTED_1_E = 24,

    /** @brief
     */
    PRV_CPSS_SIP7_MTI_TX_A_CBFC_PAUSE_FRAMES_TRANSMITTED_2_E = 25,

    /** @brief
     */
    PRV_CPSS_SIP7_MTI_TX_A_CBFC_PAUSE_FRAMES_TRANSMITTED_3_E = 26,

    /** @brief
     */
    PRV_CPSS_SIP7_MTI_TX_A_CBFC_PAUSE_FRAMES_TRANSMITTED_4_E = 27,

    /** @brief
     */
    PRV_CPSS_SIP7_MTI_TX_A_CBFC_PAUSE_FRAMES_TRANSMITTED_5_E = 28,

    /** @brief
     */
    PRV_CPSS_SIP7_MTI_TX_A_CBFC_PAUSE_FRAMES_TRANSMITTED_6_E = 29,

    /** @brief
     */
    PRV_CPSS_SIP7_MTI_TX_A_CBFC_PAUSE_FRAMES_TRANSMITTED_7_E = 30,

    /** @brief
     */
    PRV_CPSS_SIP7_MTI_TX_A_MAC_CONTROL_FRAMES_TRANSMITTED_E = 31,

    /** @brief
     */
    PRV_CPSS_SIP7_MTI_TX_ETHER_STATS_PKTS_E = 32,

    PRV_CPSS_SIP7_MTI_TX_LAST_E

} PRV_CPSS_SIP7_PORT_MAC_MTI_TX_COUNTER_ENT;


/**
* @internal prvCpssDxChPortMacCountersIsSupportedCheck function
* @endinternal
*
* @brief   Checks if the counter is supported by current device and port.
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman.
* @note   NOT APPLICABLE DEVICES:  None.
*
* @param[in] devNum                   - physical device number
* @param[in] cntrName                 - counter name
* @param[in] portMacNum               - port MAC number
* @param[in] counterMode              - CPSS_DXCH_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_NORMAL_E -
*                                      MIB counter index 4 is sent deferred. MIB counter index 12 is Frames1024toMaxOctets
*                                      CPSS_DXCH_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_HISTOGRAM_1518_E -
*                                      MIB counter index 4 is Frames1024to1518Octets. MIB counter index 12 is Frames1519toMaxOctets
*
* @retval GT_TRUE                  - if counter is supported by current device and port
* @retval GT_FALSE                 - if counter is not supported by current device and port
*/
GT_BOOL prvCpssDxChPortMacCountersIsSupportedCheck
(
    IN  GT_U8                                                  devNum,
    IN  GT_U32                                                 portMacNum,
    IN  CPSS_PORT_MAC_COUNTERS_ENT                             cntrName,
    IN  CPSS_DXCH_PORT_MAC_OVERSIZED_PACKETS_COUNTER_MODE_ENT  counterMode
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __prvCpssDxChPortStath */
