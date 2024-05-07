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
* @file cpssLedCtrl.h
*
* @brief Includes LEDs control data structures and enumerations definitions
*
* @version   2
********************************************************************************
*/
#ifndef __cpssLedCtrlh
#define __cpssLedCtrlh

#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/common/cpssTypes.h>
#include <cpss/generic/cpssCommonDefs.h>
#include <cpss/common/cpssHwInit/cpssCommonLedCtrl.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Maximal number of LED parallel bit selectors */
#define CPSS_LED_BIT_SELECTORS_MAX_NUM_CNS  8

/* Bit selector not in use */
#define CPSS_LED_BIT_SELECTOR_NOT_USED_CNS  0XFFFFFFFF

/**
* @enum CPSS_LED_CLASS_5_SELECT_ENT
 *
 * @brief The indication displayed on class5 (for dual-media port).
*/
typedef enum{

    /** Half Duplex is displayed on class5. */
    CPSS_LED_CLASS_5_SELECT_HALF_DUPLEX_E,

    /** @brief If port is a dual media port,
     *  Fiber Link Up is displayed on class5.
     */
    CPSS_LED_CLASS_5_SELECT_FIBER_LINK_UP_E

} CPSS_LED_CLASS_5_SELECT_ENT;

/**
* @enum CPSS_LED_CLASS_13_SELECT_ENT
 *
 * @brief The indication displayed on class13 (for dual-media port\phy).
*/
typedef enum{

    /** Link Down is displayed on class13. */
    CPSS_LED_CLASS_13_SELECT_LINK_DOWN_E,

    /** @brief If port is a dual media port,
     *  Copper Link Up is displayed on class13.
     */
    CPSS_LED_CLASS_13_SELECT_COPPER_LINK_UP_E

} CPSS_LED_CLASS_13_SELECT_ENT;

/**
* @enum CPSS_LED_DATA_ENT
 *
 * @brief Types of data reflected by the XG-PCS lane LEDs
*/
typedef enum{

    /** LED low */
    CPSS_LED_DATA_LOW_E,

    /** LED high */
    CPSS_LED_DATA_HIGH1_E,

    /** LED high */
    CPSS_LED_DATA_HIGH2_E,

    /** Sync indication for the lane */
    CPSS_LED_DATA_SYNC_E,

    /** PPM FIFO overrun */
    CPSS_LED_DATA_OVERRUN_E,

    /** PPM FIFO underrun */
    CPSS_LED_DATA_UNDERRUN_E,

    /** /E/ code or invalid code */
    CPSS_LED_DATA_BYTE_ERROR_E,

    /** Disparity error, invalid code */
    CPSS_LED_DATA_DISPARITY_ERROR_E,

    /** Link and data on RX XGMII */
    CPSS_LED_DATA_RX_E,

    /** Link and data on RX XGMII */
    CPSS_LED_DATA_TX_E,

    /** RX or TX */
    CPSS_LED_DATA_ACTIVITY_E,

    /** RX or Link */
    CPSS_LED_DATA_RX_OR_LINK_E,

    /** Lane's link is up */
    CPSS_LED_DATA_LINK_E,

    /** Local fault indication on the lane */
    CPSS_LED_DATA_LOCAL_FAULT_E

} CPSS_LED_DATA_ENT;

 /** @struct CPSS_LED_SIP6_CONF_STC
 *
 * @brief LED stream SIP6 specific configuration parameters.
 * @note
 *      The enumeration of LED unit indexes in full chip system is as follows:
 *      For Falcon devices:
 *                              - LED index 15 is the first (also in the stream) and 0 is last.
 *                              - The mapping of MACs in LED indexes
 *                              For Falcon 6.4T:
 *                              ------------------------------------------
 *                              MAC network ports   LED index     LED group
 *                              ------------------------------------------
 *                                  0.. 15              0           7
 *                                 16.. 31              1           6
 *                                 32.. 47              2           5
 *                                 48.. 63              3           4
 *                                 64.. 79              4           3
 *                                 80.. 95              5           2
 *                                 96..111              6           1
 *                                112..127              7           0
 *
 *                             For Falcon 12.8T:
 *                                  0.. 15              0          15
 *                                 16.. 31              1          14
 *                                 32.. 47              2          13
 *                                 48.. 63              3          12
 *                                 64.. 79              4          11
 *                                 80.. 95              5          10
 *                                 96..111              6           9
 *                                112..127              7           8
 *                                128..143              8           7
 *                                144..159              9           6
 *                                160..175             10           5
 *                                176..191             11           4
 *                                192..207             12           3
 *                                208..223             13           2
 *                                224..239             14           1
 *                                240..255             15           0
 *
 *      For AC5X and above: the LED index == LED Unit.
 *      For mapping of the ports/MACs to internal LED units,
 *      refer to the MAC-DMA Numbering  Excel sheet attached to
 *      the device Hardware Specifications.
*/
typedef struct {
    /** LED output clock frequency in kHz.  (APPLICABLE RANGES: 500..80000) */
    GT_U32     ledClockFrequency;

    /** The first bit in the LED stream indication to be driven in current LED unit. (APPLICABLE RANGES: 0..255) */
    GT_U32     ledStart[CPSS_LED_UNITS_MAX_NUM_CNS];

    /** The last bit in the LED stream indication to be driven in the current LED unit. (APPLICABLE RANGES: 0..255) */
    GT_U32     ledEnd[CPSS_LED_UNITS_MAX_NUM_CNS];

    /** The LED unit is excluded from LED chain.
    *   It is not used in calculating of the LED chain size, no indication is inserted into the LED stream.
    *          GT_TRUE   - bypass the LED unit
    *          GT_FALSE  - the LED unit is used in the LED chain */
    GT_BOOL    ledChainBypass[CPSS_LED_UNITS_MAX_NUM_CNS];

    /** The LED output stream will be driven on clock POSEDGE.
     *          GT_TRUE     - data driven on clock POSEDGE
     *          GT_FALSE    - data driven on clock NEGEDGE */
    GT_BOOL    ledClockPosEdgeModeEnable;

}CPSS_LED_SIP6_CONF_STC;

/**
* @struct CPSS_LED_TWO_CLASS_MODE_CONF_STC
 *
 * @brief LED stream two-class mode specific configuration parameters.
*/
typedef struct {
    /** @brief enable LED two-class configuration mode
     *         GT_TRUE  - LED chains will be configured per port using first and second indication classes
     *         GT_FALSE - Legacy mode: LED chain will be configured per all ports with all indication classes
     */
    GT_BOOL    ledTwoClassModeEnable;

    /** @brief If LED two-class mode enable -
     *         LED chains select the first class to reflect on the LED chains stream (the same class for both streams),
     *         else the parameter is ignored.
     *         (APPLICABLE RANGES: 0..13)
     */
    GT_U32     ledClassSelectChain0;

    /** @brief If LED two-class mode enable -
     *         LED chains select the second class to reflect on the LED chains stream (the same class for both streams),
     *         else the parameter is ignored.
     *         (APPLICABLE RANGES: 0..13)
     */
    GT_U32     ledClassSelectChain1;
}CPSS_LED_TWO_CLASS_MODE_CONF_STC;

/**
* @struct CPSS_LED_CONF_STC
 *
 * @brief LED stream configuration parameters.
*/
typedef struct CPSS_LED_CONF_STCT
{
    /** The order of the signals driven in the LED stream. */
    CPSS_LED_ORDER_MODE_ENT         ledOrganize;

    /** @brief How to display indications when the link is down:
     *         GT_FALSE - No effect, Link status has no effect
     *                    on other indications.
     *         GT_TRUE - When link is down, the following indications:
     *                   speed, duplex, activity are forced down.
     *           (APPLICABLE DEVICES: xCat3; AC5; Lion2)
     */
    GT_BOOL                         disableOnLinkDown;

    /** The duty cycle of the Blink0 signal. */
    CPSS_LED_BLINK_DUTY_CYCLE_ENT   blink0DutyCycle;

    /** The period of the Blink0 signal. */
    CPSS_LED_BLINK_DURATION_ENT     blink0Duration;

    /** The duty cycle of the Blink1 signal. */
    CPSS_LED_BLINK_DUTY_CYCLE_ENT   blink1DutyCycle;

    /** The period of the Blink1 signal. */
    CPSS_LED_BLINK_DURATION_ENT     blink1Duration;

    /** The length of stretching for dynamic signals. */
    CPSS_LED_PULSE_STRETCH_ENT      pulseStretch;

    /** The first bit in the LED stream to be driven, range 0..255 */
    GT_U32                          ledStart;

    /** The last bit in the LED stream to be driven, range 0..255 */
    GT_U32                          ledEnd;

    /** @brief Inverts the LEDClk pin, GT_FALSE - disable, GT_TRUE - enable.
     *  (APPLICABLE DEVICES: xCat3; AC5; Lion2)
     */
    GT_BOOL                         clkInvert;

    /** @brief Selects the indication displayed on class5
     *  (relevant only for GE ports LED stream).
     *  (APPLICABLE DEVICES: xCat3; AC5; Lion2)
     */
    CPSS_LED_CLASS_5_SELECT_ENT     class5select;

    /** @brief Selects the indication displayed on class13
     *  (relevant only for GE ports LED stream).
     *  (APPLICABLE DEVICES: xCat3; AC5; Lion2)
     */
    CPSS_LED_CLASS_13_SELECT_ENT    class13select;

    /** @brief LED data polarity
     *  GT_TRUE - active low: LED light is in low indication.
     *  GT_FALSE - active high: LED light is in high indication.
     *  (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3, Aldrin2, Falcon, AC5P, AC5X, Harrier, Ironman)
     */
    GT_BOOL                         invertEnable;

    /** @brief LED output clock frequency.
     *   For SIP6 devices - sip6LedConfig.ledClockFrequency need to be used
     *  (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2)
     */
    CPSS_LED_CLOCK_OUT_FREQUENCY_ENT ledClockFrequency;

    /** @brief LED stream SIP6 specific configuration parameters.
     *  (APPLICABLE DEVICES: Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    CPSS_LED_SIP6_CONF_STC          sip6LedConfig;
} CPSS_LED_CONF_STC;

/**
* @struct CPSS_LED_CLASS_MANIPULATION_STC
 *
 * @brief LED class manipulation configuration.
*/
typedef struct CPSS_LED_CLASS_MANIPULATION_STCT
{
    /** @brief Enables a NOT operation on Class data.
     *   GT_TRUE - data inversion enabled.
     *   GT_FALSE - data is not inverted.
     *  (APPLICABLE DEVICES: xCat3; AC5; Lion2)
     */
    GT_BOOL                     invertEnable;

    /** @brief Enables an AND operation on Class with selected Blink signals.
     *   GT_TRUE - blinking enabled: High data displayed as blinking.
     *   GT_FALSE - blinking feature not enabled.
     */
    GT_BOOL                     blinkEnable;

    /** Selects between Blink 0 or Blink 1 signals. */
    CPSS_LED_BLINK_SELECT_ENT   blinkSelect;

    /** @brief Enables forcing Class data.
     *   GT_TRUE - forceData is the data displayed.
     *   GT_FALSE - no forcing on dispalyed data.
     */
    GT_BOOL                     forceEnable;

    /** @brief The data to be forced on Class data.
     *  For GE devices: range 0x0..0xFFF (12 bits length)
     *  For XG devices: range 0x0..0x3   (2 bits length)
     */
    GT_U32                      forceData;

    /** @brief enable the stretch affect on a specific class
     *  (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3, Aldrin2, Falcon, AC5P, AC5X, Harrier, Ironman)
     */
    GT_BOOL                     pulseStretchEnable;

    /** @brief disable the LED indication classes when the link is down
     *  (APPLICABLE DEVICES: Bobcat2, Caelum, Aldrin, AC3X, Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman)
     */
    GT_BOOL                     disableOnLinkDown;
} CPSS_LED_CLASS_MANIPULATION_STC;

/**
* @struct CPSS_LED_PARALLEL_BIT_CONFIG_STC
 *
 * @brief LED parallel bit selector configurations.
*/
typedef struct
{
    /** @brief defines bit locations taken from the serial data stream
     *  to the parallel data bus
     *  (APPLICABLE RANGES: Ironman  0..63)
     *  @note The value CPSS_LED_BIT_SELECTOR_NOT_USED_CNS means there is no relevant parallel bit in serial LED data stream
     *  and all configurations for this bit selector will be skipped
     */
    GT_U32                              dataBitSelector;

    /** @brief polarity selection of each parallel data bit in relation to the serial LED data
     *  GT_TRUE   - parallel data is inverted from input LED data
     *  GT_FALSE  - parallel data is the same polarity as input LED data
     */
    GT_BOOL                             polarityInvert;
} CPSS_LED_PARALLEL_BIT_CONFIG_STC;

/**
* @struct CPSS_LED_PARALLEL_CONFIG_STC
 *
 * @brief LED parallel mode configuration parameters.
*/
typedef struct
{
    /** @brief enable LED parallel configuration mode
     *  GT_TRUE   - parallel data is selected and sent on the parallel data bus
     *  GT_FALSE  - legacy mode: parallel mode is disabled
     */
    GT_BOOL                             ledParallelModeEnable;

    /** @brief array of the bits from the led data stream.
     *  The value of the array is port indication in the LED data stream to be placed to parallel bit selector interface
     *  (APPLICABLE RANGES: Ironman  0..63)
     */
    CPSS_LED_PARALLEL_BIT_CONFIG_STC    ledParallelBitConfigArr[CPSS_LED_BIT_SELECTORS_MAX_NUM_CNS];
} CPSS_LED_PARALLEL_CONFIG_STC;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __cpssLedCtrlh */


