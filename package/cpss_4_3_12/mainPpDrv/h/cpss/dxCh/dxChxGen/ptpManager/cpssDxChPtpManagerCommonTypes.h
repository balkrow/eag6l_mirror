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
* @file cpssDxChPtpManagerCommonTypes.h
*
* @brief Precision Time Protocol Manager Support - API level data type definitions
*
* @version   1
********************************************************************************
*/
#ifndef __cpssDxChPtpManagerCommonTypes_h
#define __cpssDxChPtpManagerCommonTypes_h

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
* @struct CPSS_DXCH_PTP_MANAGER_PTP_PHY_DELAY_VALUES_STC
*
* @brief Structure for PTP PHY delay values.
*/
typedef struct{

    /** @brief PPS delay time, the link delay from switch to PHY,
     *         represented as a unsigned 32-bit nanosecond value.
     *         (APPLICABLE RANGES: 0..10^9-1.)
     */
    GT_U32  phyPpsLinkDelay;

    /** @brief Pulse delay time, the link delay from switch to PHY,
     *         represented as a unsigned 32-bit nanosecond value.
     *         (APPLICABLE RANGES: 0..10^9-1.)
     */
    GT_U32  phyPulseLinkDelay;

} CPSS_DXCH_PTP_MANAGER_PTP_PHY_DELAY_VALUES_STC;

/**
* @struct CPSS_DXCH_PTP_MANAGER_PTP_DELAY_VALUES_STC
*
* @brief Structure for PTP delay values.
*/
typedef struct{

    /** @brief Egress pipe delay of channel n encrypted time-stamping frames,
     *        represented as an unsigned 30 bit nanosecond value.
     *        (APPLICABLE RANGES: 0..10^9-1.)                                               .
     *        (APPLICABLE DEVICES: AC5P; Harrier; Ironman).
     */
    GT_32  egressPipeDelayEncrypted;

    /** @brief The latency from the FSU till the pin according to
     *        port mode, represented as an unsigned 10 bit nanosecond value.
     *        (APPLICABLE RANGES: 0.)                                                                .
     *        (APPLICABLE DEVICES: Harrier).
     */
    GT_32  egressPipeDelayFsuOffset;

    /** @brief Egress pipe delay, represented as unsigned 30-bit
     *         nanosecond value.
     *        (APPLICABLE RANGES: 0..10^9-1.)
     */
    GT_32  egressPipeDelay;

    /** @brief Ingress port delay, represented as a signed 32-bit nanosecond value.
     *         (APPLICABLE RANGES: -10^9..10^9-1.)
     */
    GT_32 ingressPortDelay;

    /** @brief Ingress link delay, represented as a signed 32-bit nanosecond value
     *         for Domain Profile [0..8].
     *         (APPLICABLE RANGES: -10^9..10^9-1.)
     */

} CPSS_DXCH_PTP_MANAGER_PTP_DELAY_VALUES_STC;

/**
* @struct CPSS_DXCH_PTP_MANAGER_DB_INFO_STC
 *
*  @brief Structure represent the DB of PTP Manager
*/
typedef struct{

    /** @brief init should be done only once
     */
    GT_BOOL initDone;

} CPSS_DXCH_PTP_MANAGER_DB_INFO_STC;

/**
* @enum CPSS_DXCH_PTP_MANAGER_PORT_SPEED_ENT
*
* @brief Enumeration of port speeds
*/
typedef enum
{
    CPSS_DXCH_PTP_MANAGER_PORT_SPEED_100M_E, /* (APPLICABLE DEVICES: IRONMAN) */
    CPSS_DXCH_PTP_MANAGER_PORT_SPEED_1G_E,
    CPSS_DXCH_PTP_MANAGER_PORT_SPEED_10G_E,
    CPSS_DXCH_PTP_MANAGER_PORT_SPEED_25G_E,
    CPSS_DXCH_PTP_MANAGER_PORT_SPEED_40G_E,
    CPSS_DXCH_PTP_MANAGER_PORT_SPEED_50G_E,
    CPSS_DXCH_PTP_MANAGER_PORT_SPEED_100G_E,
    CPSS_DXCH_PTP_MANAGER_PORT_SPEED_200G_E,
    CPSS_DXCH_PTP_MANAGER_PORT_SPEED_400G_E,

    CPSS_DXCH_PTP_MANAGER_PORT_SPEED_LAST_E

}CPSS_DXCH_PTP_MANAGER_PORT_SPEED_ENT;

/**
* @enum CPSS_DXCH_PTP_MANAGER_PORT_INTERFACE_MODE_ENT
*
* @brief Enumeration of port interface modes
*/
typedef enum
{
    CPSS_DXCH_PTP_MANAGER_PORT_INTERFACE_MODE_DEFAULT_E,
    CPSS_DXCH_PTP_MANAGER_PORT_INTERFACE_MODE_2LANE_E,
    CPSS_DXCH_PTP_MANAGER_PORT_INTERFACE_MODE_4LANE_E,

    CPSS_DXCH_PTP_MANAGER_PORT_INTERFACE_MODE_LAST_E

}CPSS_DXCH_PTP_MANAGER_PORT_INTERFACE_MODE_ENT;

/**
* @enum CPSS_DXCH_PTP_MANAGER_PORT_FEC_MODE_ENT
 *
 * @brief Enumeration of port's Forward Error Correction modes.
*/
typedef enum{

     /** FEC disabled on port */
    CPSS_DXCH_PTP_MANAGER_PORT_FEC_MODE_DISABLED_E,

    /** FC-FEC enabled on port */
    CPSS_DXCH_PTP_MANAGER_PORT_FC_FEC_MODE_ENABLED_E,

    /** @brief Reed-Solomon (528,514) FEC mode */
    CPSS_DXCH_PTP_MANAGER_PORT_RS_FEC_528_514_MODE_ENABLED_E,

    /** @brief Reed-Solomon (544,514) FEC mode */
    CPSS_DXCH_PTP_MANAGER_PORT_RS_FEC_544_514_MODE_ENABLED_E,

    CPSS_DXCH_PTP_MANAGER_PORT_FEC_MODE_LAST_E

} CPSS_DXCH_PTP_MANAGER_PORT_FEC_MODE_ENT;

/**
* @enum CPSS_DXCH_PTP_ENHANCED_TIMESTAMPING_MODE_ENT
*
* @brief PTP enhanced timestamping mode.
*/
typedef enum{

    /** @brief When timestamping mode is disabled.
     */
    CPSS_DXCH_PTP_ENHANCED_TIMESTAMPING_MODE_NONE_E,

    /** @brief When no enhanced timestamping mode is used, backward compatible.
     */
    CPSS_DXCH_PTP_ENHANCED_TIMESTAMPING_MODE_STANDARD_E,

    /** @brief When using Copper PHY where correction is implemented
     *         by them. The information is passed as proprietary 8B
     *         header instead of the frame preamble.
     */
    CPSS_DXCH_PTP_ENHANCED_TIMESTAMPING_MODE_USGMII_E,

    /** @brief When using encryption for PTP packets.
     *         Utilizing the "timestamp to the future" capability.
     */
    CPSS_DXCH_PTP_ENHANCED_TIMESTAMPING_MODE_MACSEC_E,

} CPSS_DXCH_PTP_ENHANCED_TIMESTAMPING_MODE_ENT;

/**
* @enum CPSS_DXCH_PTP_MANAGER_OPERATION_MODE_ENT
*
* @brief This enum defines PTP operation modes.
* (APPLICABLE DEVICES: AC5P; AC5X; Harrier; Ironman)
*/
typedef enum{

    /** One or two step timestamps in the CTSU and possible CF update at the MAC */
    CPSS_DXCH_PTP_MANAGER_OPERATION_MODE_ONE_OR_TWO_STEP_IN_SWITCH_E,

     /** USXGMII/USGMII PHY timestamp (1-step or 2-steps) via Marvell MCH.
      *  In 1-step usecase, switch also timestamps in the CTSU. */
    CPSS_DXCH_PTP_MANAGER_OPERATION_MODE_ONE_OR_TWO_STEP_IN_USGMII_PHY_MARVELL_FORMAT_E,

    /** USXGMII/USGMII PHY timestamp (2-steps) in using the standard format */
    CPSS_DXCH_PTP_MANAGER_OPERATION_MODE_TWO_STEP_PHY_STANDARD_PCH_FORMAT_E,

    /** 2-steps at the MAC (Capture actions) or 1-step with CF update at the CTSU + MAC (Sync msg).
     *  (APPLICABLE DEVICES: Ironman) */
    CPSS_DXCH_PTP_MANAGER_OPERATION_MODE_ONE_OR_TWO_STEP_IN_MAC_E

} CPSS_DXCH_PTP_MANAGER_OPERATION_MODE_ENT;

/**
* @struct CPSS_DXCH_PTP_MANAGER_DB_DELAYS_STC
*
* @brief Structure for default PTP delay values
*        (egressPipeDelay,ingressPortDelay) per port
*        {speed,interface,fec} in PTP manager DB for each
*        device.
*/
typedef struct
{
    CPSS_DXCH_PTP_MANAGER_PORT_SPEED_ENT            speed;
    CPSS_DXCH_PTP_MANAGER_PORT_INTERFACE_MODE_ENT   intMode;
    CPSS_DXCH_PTP_MANAGER_PORT_FEC_MODE_ENT         fecMode;
    GT_32                                           egressPipeDelay;
    GT_32                                           ingressPortDelay;
} CPSS_DXCH_PTP_MANAGER_DB_DELAYS_STC;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssDxChPtpManagerCommonTypes_h */


