/*******************************************************************************
Copyright (C) 2019, Marvell International Ltd. and its affiliates
If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.
*******************************************************************************/

/********************************************************************
This file contains functions prototypes and global defines/data for
higher-level functions to configure Marvell CE SERDES IP:
5FFP_COMPHY_56G_PIPE5_X4_4PLL
********************************************************************/
#ifndef MCESD_N5XC56GP5X4_API_H
#define MCESD_N5XC56GP5X4_API_H

#ifdef N5XC56GP5X4

#if C_LINKAGE
#if defined __cplusplus
extern "C" {
#endif
#endif

#ifdef MCESD_EOM_STATS
#include "../mcesdEOMStats.h"
#endif

/**
@brief  Returns the version number of SERDES MCU firmware

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call

@param[out] major - major version number
@param[out] minor - minor version number
@param[out] patch - patch version number
@param[out] build - build version number

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_GetFirmwareRev
(
    IN MCESD_DEV_PTR devPtr,
    OUT MCESD_U8 *major,
    OUT MCESD_U8 *minor,
    OUT MCESD_U8 *patch,
    OUT MCESD_U8 *build
);

/**
@brief  Returns the state of PLL lock

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, 2, 3, etc.

@param[out] tsLocked - True when locked or False when not locked
@param[out] rsLocked - True when locked or False when not locked

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_GetPLLLock
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT MCESD_BOOL* tsLocked,
    OUT MCESD_BOOL* rsLocked
);

/**
@brief  Returns the state of readiness for TX and RX

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, 2, 3, etc.

@param[out] txReady - True when TX is ready, otherwise False
@param[out] rxReady - True when RX is ready, otherwise False

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_GetTxRxReady
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT MCESD_BOOL *txReady,
    OUT MCESD_BOOL *rxReady
);

/**
@brief  Initializes Receiver.

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, 2, 3, etc.
@param[in]  timeout - poll timoeut in ms

@note Should only be used when a valid signal is present on the receiver port

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_RxInit
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_U32 timeout
);

/**
@brief  Sets the TX equalization parameter to a value on the specified lane.

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, 2, 3, etc.
@param[in]  param - TX equalization parameter
@param[in]  paramValue - the value to set

@note PAM2 Valid Taps: PRE, MAIN, POST
@note PAM4 Valid Taps: PRE2, PRE, MAIN, POST

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_SetTxEqParam
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_N5XC56GP5X4_TXEQ_PARAM param,
    IN MCESD_32 paramValue
);

/**
@brief  Sets all TX equalization parameters for a specified lane.

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, 2, 3, etc.
@param[in]  pre2 - PRE2 value
@param[in]  pre - PRE value
@param[in]  main - MAIN value
@param[in]  post - POST value

@note Assign main to 0xFF for default behavior where main is readjusted, so the sum is 63

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_SetTxEqAll
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_32 pre2,
    IN MCESD_32 pre,
    IN MCESD_32 main,
    IN MCESD_32 post
);

/**
@brief  Gets the value of the TX equalization parameter on the specified lane.

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, 2, 3, etc.
@param[in]  param - TX equalization parameter

@param[out] paramValue - value of the specified parameter

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_GetTxEqParam
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_N5XC56GP5X4_TXEQ_PARAM param,
    OUT MCESD_32 *paramValue
);

/**
@brief  Sets the CTLE parameter to a value on the specified lane.

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, 2, 3, etc.
@param[in]  param - CTLE parameter
@param[in]  paramValue - the value to set

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_SetCTLEParam
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_N5XC56GP5X4_CTLE_PARAM param,
    IN MCESD_U32 paramValue
);

/**
@brief  Gets the value of the CTLE parameter on the specified lane.

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, 2, 3, etc.
@param[in]  param - CTLE parameter

@param[out] paramValue - value of the specified parameter

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_GetCTLEParam
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_N5XC56GP5X4_CTLE_PARAM param,
    OUT MCESD_U32 *paramValue
);

/**
@brief  Enable/Disable DFE on a specific lane

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, 2, 3, etc.
@param[in]  state - MCESD_TRUE to enable DFE on the specified lane, other MCESD_FALSE

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_SetDfeEnable
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_BOOL state
);

/**
@brief  Get DFE Enable state on the specified lane

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, 2, 3, etc.

@param[out] state - MCESD_TRUE indicates DFE is enabled on the specified lane, other MCESD_FALSE

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_GetDfeEnable
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT MCESD_BOOL *state
);

/**
@brief  Enable/Disable freezing DFE updates on a specific lane

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, 2, 3, etc.
@param[in]  state - MCESD_TRUE to freeze DFE updates on the specified lane, other MCESD_FALSE

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_SetFreezeDfeUpdates
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_BOOL state
);

/**
@brief  Get Freeze DFE Updates state on the specified lane

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, 2, 3, etc.

@param[out] state - MCESD_TRUE indicates DFE updates is frozen on the specified lane, other MCESD_FALSE

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_GetFreezeDfeUpdates
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT MCESD_BOOL *state
);

/**
@brief  Gets the value of the DFE tap on the specified lane.

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, 2, 3, etc.
@param[in]  eyeTmb - top, middle or bottom
@param[in]  tap - DFE tap

@param[out] tapValue - signed value of the specified tap

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_GetDfeTap
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_N5XC56GP5X4_EYE_TMB eyeTmb,
    IN E_N5XC56GP5X4_DFE_TAP tap,
    OUT MCESD_32 *tapValue
);

/**
@brief  Enable or Disable MCU broadcast. When broadcast is enabled, register writes applies to all lanes.

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  state - MCESD_TRUE to enable MCU broadcast mode; MCESD_FALSE to disable MCU broadcast mode

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_SetMcuBroadcast
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_BOOL state
);

/**
@brief  Returns the state of MCU broadcast

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call

@param[out] state - MCESD_TRUE when MCU broadcast is enabled; otherwise MCESD_FALSE

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_GetMcuBroadcast
(
    IN MCESD_DEV_PTR devPtr,
    OUT MCESD_BOOL *state
);

/**
@brief  Set power on/off to Transmitter

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, 2, 3, etc.
@param[in]  state - MCESD_TRUE to power on the transmitter, otherwise MCESD_FALSE

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_SetPowerTx
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_BOOL state
);

/**
@brief  Get Transmitter power state

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, 2, 3, etc.

@param[out] state - MCESD_TRUE means transmitter has power, otherwise MCESD_FALSE

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_GetPowerTx
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT MCESD_BOOL *state
);

/**
@brief  Set power on/off to Receiver

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, 2, 3, etc.
@param[in]  state - MCESD_TRUE to power on the receiver, otherwise MCESD_FALSE

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_SetPowerRx
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_BOOL state
);

/**
@brief  Get Receiver power state

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, 2, 3, etc.

@param[out] state - MCESD_TRUE means receiver has power, otherwise MCESD_FALSE

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_GetPowerRx
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT MCESD_BOOL *state
);

/**
@brief  Enable/Disable TX Output

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, 2, 3, etc.
@param[in]  state - MCESD_TRUE to enable TX output, otherwise MCESD_FALSE

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_SetTxOutputEnable
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_BOOL state
);

/**
@brief  Retrieves the state of TX Output

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, 2, 3, etc.

@param[out] state - MCESD_TRUE when TX Output is enabled, otherwise MCESD_FALSE

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_GetTxOutputEnable
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT MCESD_BOOL *state
);

/**
@brief  Power on/off current and voltage reference

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  state - MCESD_TRUE to power up current and voltage reference, otherwise MCESD_FALSE to power down current and voltage reference

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_SetPowerIvRef
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_BOOL state
);

/**
@brief  Power on/off current and voltage reference

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  state - MCESD_TRUE to power up current and voltage reference, otherwise MCESD_FALSE to power down current and voltage reference

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_GetPowerIvRef
(
    IN MCESD_DEV_PTR devPtr,
    OUT MCESD_BOOL *state
);

/**
@brief  Power on/off PLL for the specified lane

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, 2, 3, etc.
@param[in]  state - MCESD_TRUE to power up lane PLL, otherwise MCESD_FALSE to power down lane PLL

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_SetPowerPLL
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_BOOL state
);

/**
@brief  Retrieves the state of PLL power for the specified lane

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, 2, 3, etc.

@param[out] state - MCESD_TRUE when lane PLL is powered up, otherwise MCESD_FALSE

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_GetPowerPLL
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT MCESD_BOOL *state
);

/**
@brief  Sets PHYMODE to specified value

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  mode - E_N5XC56GP5X4_PHYMODE enum to represent SERDES, etc.

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_SetPhyMode
(
    IN MCESD_DEV_PTR devPtr,
    IN E_N5XC56GP5X4_PHYMODE mode
);

/**
@brief  Sets PHYMODE to specified value

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call

@param[out] mode - N5XC56GP5X4_PHYMODE_SERDES.

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_GetPhyMode
(
    IN MCESD_DEV_PTR devPtr,
    OUT E_N5XC56GP5X4_PHYMODE *mode
);

/**
@brief  Sets the reference frequency and reference clock selection group

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, 2, 3, etc.
@param[in]  txFreq - E_N5XC56GP5X4_REFFREQ enum that represents the reference frequency
@param[in]  rxFreq - E_N5XC56GP5X4_REFFREQ enum that represents the reference frequency
@param[in]  txClkSel - E_N5XC56GP5X4_REFCLK_SEL enum that represents the reference clock selection group
@param[in]  rxClkSel - E_N5XC56GP5X4_REFCLK_SEL enum that represents the reference clock selection group

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_SetRefFreq
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_N5XC56GP5X4_REFFREQ txFreq,
    IN E_N5XC56GP5X4_REFFREQ rxFreq,
    IN E_N5XC56GP5X4_REFCLK_SEL txClkSel,
    IN E_N5XC56GP5X4_REFCLK_SEL rxClkSel
);

/**
@brief  Gets the reference frequency

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, 2, 3, etc.

@param[out] txFreq - E_N5XC56GP5X4_REFFREQ enum that represents the reference frequency
@param[out] rxFreq - E_N5XC56GP5X4_REFFREQ enum that represents the reference frequency
@param[out] txClkSel - E_N5XC56GP5X4_REFCLK_SEL enum that represents the reference clock selection group
@param[out] rxClkSel - E_N5XC56GP5X4_REFCLK_SEL enum that represents the reference clock selection group

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_GetRefFreq
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT E_N5XC56GP5X4_REFFREQ *txFreq,
    OUT E_N5XC56GP5X4_REFFREQ *rxFreq,
    OUT E_N5XC56GP5X4_REFCLK_SEL *txClkSel,
    OUT E_N5XC56GP5X4_REFCLK_SEL *rxClkSel
);

/**
@brief  Changes TX and RX bitrate to the specified speed

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, 2, 3, etc.
@param[in]  txSpeed - E_N5XC56GP5X4_SERDES_SPEED enum that represents the TX bitrate
@param[in]  rxSpeed - E_N5XC56GP5X4_SERDES_SPEED enum that represents the RX bitrate

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_SetTxRxBitRate
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_N5XC56GP5X4_SERDES_SPEED txSpeed,
    IN E_N5XC56GP5X4_SERDES_SPEED rxSpeed
);

/**
@brief  Gets the current TX/RX bit rate

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, 2, 3, etc.

@param[out] txSpeed - E_N5XC56GP5X4_SERDES_SPEED that represents the TX bitrate
@param[out] rxSpeed - E_N5XC56GP5X4_SERDES_SPEED that represents the RX bitrate

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_GetTxRxBitRate
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT E_N5XC56GP5X4_SERDES_SPEED *txSpeed,
    OUT E_N5XC56GP5X4_SERDES_SPEED *rxSpeed
);

/**
@brief  Sets the databus width

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, 2, 3, etc.
@param[in]  txWidth - E_N5XC56GP5X4_DATABUS_WIDTH enum that represents the number of bits for TX databus
@param[in]  rxWidth - E_N5XC56GP5X4_DATABUS_WIDTH enum that represents the number of bits for RX databus

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_SetDataBusWidth
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_N5XC56GP5X4_DATABUS_WIDTH txWidth,
    IN E_N5XC56GP5X4_DATABUS_WIDTH rxWidth
);

/**
@brief  Gets the databus width

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, 2, 3, etc.

@param[out] txWidth - E_N5XC56GP5X4_DATABUS_WIDTH enum that represents the number of bits for TX databus
@param[out] rxWidth - E_N5XC56GP5X4_DATABUS_WIDTH enum that represents the number of bits for RX databus

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_GetDataBusWidth
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT E_N5XC56GP5X4_DATABUS_WIDTH *txWidth,
    OUT E_N5XC56GP5X4_DATABUS_WIDTH *rxWidth
);

/**
@brief  Sets the MCU clock frequency

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  clockMHz - The clock frequency in MHz for MCU

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_SetMcuClockFreq
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U16 clockMHz
);

/**
@brief  Sets the MCU clock frequency

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call

@param[out] clockMHz - The clock frequency in MHz for MCU

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_GetMcuClockFreq
(
    IN MCESD_DEV_PTR devPtr,
    OUT MCESD_U32 *clockMHz
);

/**
@brief  Set align90 value

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, 2, 3, etc.
@param[in]  align90 - align90 value

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_SetAlign90
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_U16 align90
);

/**
@brief  Get align90 value

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, 2, 3, etc.

@param[out] align90 - align90 value
@param[out] analogSetting - analog setting value

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_GetAlign90
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT MCESD_U16 *align90,
    OUT MCESD_U16 *analogSetting
);

/**
@brief  Perform TRx or Rx Training

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, 2, 3, etc.
@param[in]  type - N5XC56GP5X4_TRAINING_TRX or N5XC56GP5X4_TRAINING_RX

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_ExecuteTraining
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_N5XC56GP5X4_TRAINING type
);

/**
@brief  Start TRx or Rx Training

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.
@param[in]  type - N5XC56GP5X4_TRAINING_TRX or N5XC56GP5X4_TRAINING_RX

@note If running PHY Test, then reset PHY Test by calling StopPhyTest() followed by StartPhyTest().

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_StartTraining
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_N5XC56GP5X4_TRAINING type
);

/**
@brief  Checks TRx or Rx Training

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.
@param[in]  type - N5XC56GP5X4_TRAINING_TRX or N5XC56GP5X4_TRAINING_RX

@param[out] completed - true if completed
@param[out] failed - true if failed


@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_CheckTraining
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_N5XC56GP5X4_TRAINING type,
    OUT MCESD_BOOL *completed,
    OUT MCESD_BOOL *failed
);

/**
@brief  Stops TRx or Rx Training

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.
@param[in]  type - N5XC56GP5X4_TRAINING_TRX or N5XC56GP5X4_TRAINING_RX

@note If running PHY Test, then reset PHY Test by calling StopPhyTest() followed by StartPhyTest().

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_StopTraining
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_N5XC56GP5X4_TRAINING type
);

/**
@brief  Sets the training timeout in milliseconds

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, 2, 3, etc.
@param[in]  type - N5XC56GP5X4_TRAINING_TRX or N5XC56GP5X4_TRAINING_RX
@param[in]  training - S_N5XC56GP5X4_TRAIN_TIMEOUT represents the context for timeout

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_SetTrainingTimeout
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_N5XC56GP5X4_TRAINING type,
    IN S_N5XC56GP5X4_TRAIN_TIMEOUT *training
);

/**
@brief  Gets the training timeout in milliseconds

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, 2, 3, etc.
@param[in]  type - N5XC56GP5X4_TRAINING_TRX or N5XC56GP5X4_TRAINING_RX

@param[out] training - S_N5XC56GP5X4_TRAIN_TIMEOUT represents the context for timeout

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_GetTrainingTimeout
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_N5XC56GP5X4_TRAINING type,
    OUT S_N5XC56GP5X4_TRAIN_TIMEOUT *training
);

/**
@brief  Gets the eye height values after training

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, 2, 3, etc.

@param[out] trainedEyeHeight - pointer to S_N5XC56GP5X4_TRAINED_EYE_H

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_GetTrainedEyeHeight
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT S_N5XC56GP5X4_TRAINED_EYE_H *trainedEyeHeight
);

/**
@brief  Sets the value of the CDR parameter on the specified lane.

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, 2, 3, etc.
@param[in]  param - CDR parameter (E_N5XC56GP5X4_CDR_PARAM)
@param[in]  paramValue - value to set

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_SetCDRParam
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_N5XC56GP5X4_CDR_PARAM param,
    IN MCESD_U32 paramValue
);

/**
@brief  Gets the value of the CDR parameter on the specified lane.

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, 2, 3, etc.
@param[in]  param - CDR parameter (E_N5XC56GP5X4_CDR_PARAM)

@param[out] paramValue - value of the specified parameter

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_GetCDRParam
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_N5XC56GP5X4_CDR_PARAM param,
    OUT MCESD_U32 *paramValue
);

/**
@brief  Determines if signal is squelched

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, 2, 3, etc.

@param[out] squelched - MCESD_TRUE if no signal is detected, otherwise MCESD_FALSE

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_GetSquelchDetect
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT MCESD_BOOL *squelched
);

/**
@brief  Sets the squelch threshold

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, 2, 3, etc.
@param[in]  threshold - the squelch threshold level (range: N5XC56GP5X4_SQ_THRESH_MIN ~ N5XC56GP5X4_SQ_THRESH_MAX)

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_SetSquelchThreshold
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_16 threshold
);

/**
@brief  Gets the squelch threshold

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, 2, 3, etc.

@param[out] threshold - the squelch threshold level

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_GetSquelchThreshold
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT MCESD_16 *threshold
);

/**
@brief  Sets the Data path

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, 2, 3, etc.
@param[in]  path - N5XC56GP5X4_PATH_EXTERNAL or N5XC56GP5X4_PATH_FAR_END_LB

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_SetDataPath
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_N5XC56GP5X4_DATAPATH path
);

/**
@brief  Gets the Data path

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, 2, 3, etc.

@param[out] path - N5XC56GP5X4_PATH_EXTERNAL or N5XC56GP5X4_PATH_FAR_END_LB

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_GetDataPath
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT E_N5XC56GP5X4_DATAPATH *path
);

/**
@brief  Gets the temperature in degrees 100 micro-Celsius (factor of 10,000)

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call

@param[out] temperature - value in degrees 100 micro-Celsius (factor of 10,000)

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_GetTemperature
(
    IN MCESD_DEV_PTR devPtr,
    OUT MCESD_32 *temperature
);

/**
@brief  Sets the Transmitter and Receiver data polarity

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, 2, 3, etc.
@param[in]  txPolarity - set transmitter to N5XC56GP5X4_POLARITY_NORMAL or N5XC56GP5X4_POLARITY_INV
@param[in]  rxPolarity - set receiver to N5XC56GP5X4_POLARITY_NORMAL or N5XC56GP5X4_POLARITY_INV

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_SetTxRxPolarity
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_N5XC56GP5X4_POLARITY txPolarity,
    IN E_N5XC56GP5X4_POLARITY rxPolarity
);

/**
@brief  Gets the Transmitter and Receiver data polarity

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.

@param[out] txPolarity - N5XC56GP5X4_POLARITY_NORMAL or N5XC56GP5X4_POLARITY_INV
@param[out] rxPolarity - N5XC56GP5X4_POLARITY_NORMAL or N5XC56GP5X4_POLARITY_INV

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_GetTxRxPolarity
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT E_N5XC56GP5X4_POLARITY *txPolarity,
    OUT E_N5XC56GP5X4_POLARITY *rxPolarity
);

/**
@brief  Inject x errors on transmitter data path

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, 2, 3, etc.
@param[in]  errors - inject 1..8 errors

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_TxInjectError
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_U8 errors
);

/**
@brief  Sets the Transmitter and Receiver pattern

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, 2, 3, etc.
@param[in]  txPattern - E_N5XC56GP5X4_PATTERN for transmitter
@param[in]  rxPattern - E_N5XC56GP5X4_PATTERN for receiver
@param[in]  txUserPattern - string of hexadecimal characters (max 20 characters); valid when tx pattern = N5XC56GP5X4_PAT_USER
@param[in]  rxUserPattern - string of hexadecimal characters (max 20 characters); valid when rx pattern = N5XC56GP5X4_PAT_USER

@note Use this function to configure TX and RX pattern before calling API_N5XC56GP5X4_StartPhyTest()
@note For patterns JITTER_8T and JITTER_4T, databus is adjusted to 32 or 64 bits
@note For patterns JITTER_10T and JITTER_5T, databus is adjusted to 40 or 80 bits

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_SetTxRxPattern
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_N5XC56GP5X4_PATTERN txPattern,
    IN E_N5XC56GP5X4_PATTERN rxPattern,
    IN const char *txUserPattern,
    IN const char *rxUserPattern
);

/**
@brief  Gets the Transmitter and Receiver pattern

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, 2, 3, etc.

@param[out] txPattern - pattern structure for transmitter
@param[out] rxPattern - pattern structure for receiver
@param[out] txUserPattern - string of hexadecimal characters representing the user pattern
@param[out] rxUserPattern - string of hexadecimal characters representing the user pattern

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_GetTxRxPattern
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT E_N5XC56GP5X4_PATTERN *txPattern,
    OUT E_N5XC56GP5X4_PATTERN *rxPattern,
    OUT char *txUserPattern,
    OUT char *rxUserPattern
);

/**
@brief  Sets MSBLSB Swap

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, 2, 3, etc.
@param[in]  txSwapMsbLsb - E_N5XC56GP5X4_SWAP_MSB_LSB for transmitter
@param[in]  rxSwapMsbLsb - E_N5XC56GP5X4_SWAP_MSB_LSB for receiver

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_SetMSBLSBSwap
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_N5XC56GP5X4_SWAP_MSB_LSB txSwapMsbLsb,
    IN E_N5XC56GP5X4_SWAP_MSB_LSB rxSwapMsbLsb
);

/**
@brief  Get MSBLSB Swap

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, 2, 3, etc.

@param[out]  txSwapMsbLsb - E_N5XC56GP5X4_SWAP_MSB_LSB for transmitter
@param[out]  rxSwapMsbLsb - E_N5XC56GP5X4_SWAP_MSB_LSB for receiver

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_GetMSBLSBSwap
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT E_N5XC56GP5X4_SWAP_MSB_LSB *txSwapMsbLsb,
    OUT E_N5XC56GP5X4_SWAP_MSB_LSB *rxSwapMsbLsb
);

/**
@brief  Sets GrayCode

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, 2, 3, etc.
@param[in]  txGrayCode - E_N5XC56GP5X4_GRAY_CODE for transmitter
@param[in]  rxGrayCode - E_N5XC56GP5X4_GRAY_CODE for receiver

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_SetGrayCode
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_N5XC56GP5X4_GRAY_CODE txGrayCode,
    IN E_N5XC56GP5X4_GRAY_CODE rxGrayCode
);

/**
@brief  Get GrayCode

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, 2, 3, etc.

@param[out]  txGrayCode - E_N5XC56GP5X4_GRAY_CODE for transmitter
@param[out]  rxGrayCode - E_N5XC56GP5X4_GRAY_CODE for receiver

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_GetGrayCode
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT E_N5XC56GP5X4_GRAY_CODE *txGrayCode,
    OUT E_N5XC56GP5X4_GRAY_CODE *rxGrayCode
);

/**
@brief  Sets PreCode

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.
@param[in]  txState - MCESD_BOOL for transmitter
@param[in]  rxState - MCESD_BOOL for receiver

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_SetPreCode
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_BOOL txState,
    IN MCESD_BOOL rxState
);

/**
@brief  Get PreCode

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.

@param[out]  txState - MCESD_BOOL for transmitter
@param[out]  rxState - MCESD_BOOL for receiver

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_GetPreCode
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT MCESD_BOOL *txState,
    OUT MCESD_BOOL *rxState
);

/**
@brief  Gets the pattern comparator statistics

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, 2, 3, etc.

@param[out] statistics - pointer to S_N5XC56GP5X4_PATTERN_STATS which will be populated by this function

@note API_N5XC56GP5X4_StartPhyTest() must be called first to start the pattern checker.

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_GetComparatorStats
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT S_N5XC56GP5X4_PATTERN_STATS *statistics
);

/**
@brief  Resets pattern comparator statistics. Zeroes out total bits and errors

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, 2, 3, etc.

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_ResetComparatorStats
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane
);

/**
@brief  Starts the pattern generator or comparator

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, 2, 3, etc.
@param[in]  type - N5XC56GP5X4_PHYTEST_TX or N5XC56GP5X4_PHYTEST_RX

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_StartPhyTest
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_N5XC56GP5X4_PHYTEST type
);

/**
@brief  Stops the pattern generator or comparator

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, 2, 3, etc.
@param[in]  type - N5XC56GP5X4_PHYTEST_TX or N5XC56GP5X4_PHYTEST_RX

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_StopPhyTest
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_N5XC56GP5X4_PHYTEST type
);

/**
@brief  Initializes EOM

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, 2, 3, etc.

@note Called by API_N5XC56GP5X4_EOMGetWidthHeight to initialize EOM circuit

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_EOMInit
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane
);

/**
@brief  Disables EOM Circuit

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, 2, 3, etc.

@note Called by API_N5XC56GP5X4_EOMGetWidthHeight to disable EOM circuit

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_EOMFinalize
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane
);

/**
@brief  Get measurement data at phase, voltage

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, 2, 3, etc.
@param[in]  eyeTMB - N5XC56GP5X4_EYE_TOP, N5XC56GP5X4_EYE_MID or N5XC56GP5X4_EYE_BOT
@param[in]  phase - phase to measure
@param[in]  voltage - voltage to measure (offset from center; both upper and lower voltage are measured)
@param[in]  minSamples - minimum number of bits to sample

@param[out] measurement - pointer to S_N5XC56GP5X4_EOM_DATA which will hold the results

@note Called by API_N5XC56GP5X4_EOMGetWidthHeight to measure a specific point
@note At least minSamples will be measured

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_EOMMeasPoint
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_N5XC56GP5X4_EYE_TMB eyeTMB,
    IN MCESD_32 phase,
    IN MCESD_U8 voltage,
    IN MCESD_U32 minSamples,
    OUT S_N5XC56GP5X4_EOM_DATA *measurement
);

/**
@brief  Returns the number of phase steps for 1 UI at the current speed

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.

@param[out] phaseStepCount - step count of phase
@param[out] voltageStepCount - step count of voltage

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_EOM1UIStepCount
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT MCESD_U16 *phaseStepCount,
    OUT MCESD_U16 *voltageStepCount
);

/**
@brief  Returns EYE width and height

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, 2, 3, etc.
@param[in]  eyeTMB - N5XC56GP5X4_EYE_TOP, N5XC56GP5X4_EYE_MID or N5XC56GP5X4_EYE_BOT
@param[in]  minSamples - minimum number of bits to sample
@param[in]  berThreshold - Bit Error Rate Threshold in nano (factor of 1E-9)

@param[out] width - EYE width
@param[out] heightUpper - upper EYE height
@param[out] heightLower - lower EYE height
@param[out] sampleCount - sample count

@note Requires a valid signal at the receiver
@note At least minSamples will be measured
@note Example: (berThreshold = 1E+5) => Threshold = 1E+5 * 1E-9 = 1E-4
@note Points where BER is less than 1E-4 are good

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_EOMGetWidthHeight
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_N5XC56GP5X4_EYE_TMB eyeTMB,
    IN MCESD_U32 minSamples,
    IN MCESD_U32 berThreshold,
    OUT MCESD_U16 *width,
    OUT MCESD_U16 *heightUpper,
    OUT MCESD_U16 *heightLower,
    OUT MCESD_U32 *sampleCount
);

#ifdef N5XC56GP5X4_DFE_MILLIVOLTS
/**
@brief  Converts width and height code to ps and 0.1mv

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, etc.
@param[in]  width - width value in code
@param[in]  heightUpper - upper height value in code
@param[in]  heightLower - lower height value in code

@param[out] widthmUI - width converted to mUI
@param[out] height100uVUpper - upper height converted to 100uV
@param[out] height100uVLower - lower height converted to 100uV

@note Preprocessor N5XC56GP5X4_DFE_MILLIVOLTS is required
@note Converts width to mUI by code * 1000 / maxUI
@note Converts height to 100uV by look using DFE tables and upper and lower height code values
@note Height is in 100uV to preserve precision because values in DFE tables are in mV with two significant figures

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_EOMConvertWidthHeight
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_U16 width,
    IN MCESD_U16 heightUpper,
    IN MCESD_U16 heightLower,
    OUT MCESD_U32 *widthmUI,
    OUT MCESD_U32 *height100uVUpper,
    OUT MCESD_U32 *height100uVLower
);
#endif

/**
@brief  Get Eye Data

@param[in]      devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]      lane - lane number 0, 1, etc.
@param[in]      eyeTMB - N5XC56GP5X4_EYE_TOP, N5XC56GP5X4_EYE_MID or N5XC56GP5X4_EYE_BOT
@param[in]      minSamples - minimum number of bits to sample
@param[in]      berThreshold - Bit Error Rate Threshold in nano (factor of 1E-9)
@param[in]      eomStatsMode - when TRUE, captures only necessary points (all phases at voltage 0 and all voltages at phase 0)
@param[in,out]  eyeRawDataPtr - pointer to S_N5XC56GP5X4_EYE_RAW_PTR which stores eye raw data

@note Call API_N5XC56GP5X4_EOMGetWidthHeight before to check if eye is centered
@note At least minSamples will be measured

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_EOMGetEyeData
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_N5XC56GP5X4_EYE_TMB eyeTMB,
    IN MCESD_U32 minSamples,
    IN MCESD_U32 berThreshold,
    IN MCESD_BOOL eomStatsMode,
    INOUT S_N5XC56GP5X4_EYE_RAW_PTR eyeRawDataPtr
);

/**
@brief  Plot Eye Data

@param[in]  eyeRawDataPtr - pointer to S_N5XC56GP5X4_EYE_RAW_PTR which store eye raw data
@param[in]  phaseStepSize - phase step size
@param[in]  voltageStepSize - voltage step size
@param[in]  berThreshold - bit error rate threshold in nano (factor of 1E-9)
@param[in]  berThresholdMax - max bit error rate threshold in nano (factor of 1E-9)

@note Call API_N5XC56GP5X4_EOMGetEyeData before to populate eyeRawDataPtr
@note Outputs plot through MCESD_DBG_INFO
@note Calculate errorThreshold by taking desired BER threshold multiplied total sample bit count
@note The berThreshold and berThresholdMax is used for plotting different BER rates on the plot
@note See API_N5XC56GP5X4_EOMGetWidthHeight() for example of berThreshold

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_EOMPlotEyeData
(
    IN S_N5XC56GP5X4_EYE_RAW_PTR eyeRawDataPtr,
    IN MCESD_U32 phaseStepSize,
    IN MCESD_U32 voltageStepSize,
    IN MCESD_U32 berThreshold,
    IN MCESD_U32 berThresholdMax
);

#ifdef MCESD_EOM_STATS
/**
@brief  Get EOM Statistics

@param[in]      eyeRawDataPtr - pointer to S_N5XC56GP5X4_EYE_RAW_PTR which store eye raw data
@param[in]      eyeBufferDataPtr - pointer to scratch buffer used for eye statistics calculation
@param[in,out]  estimateEyeDimPtr - array of S_EOM_STATS_EYE_DIM of requested estimated eye width/height
@param[in]      estimateEyeDimCount - count in array

@param[out]     amplitudeStats - pointer to S_EOM_STATS_EYE_AMP which stores amplitude statistics

@note Compilation with the MCESD_EOM_STATS is required
@note Compilation requires linking with math libraries ('-lm') or standard that provides math libraries ('-std=C99')
@note Function only supports calculations for one eye
@note Array of estimate eye dimensions expects BER attribute to be populated
@note Format of BER attribute is actual BER for example (1e-9)
@note Dimensions default to -1 when predictions are out of bounds

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_EOMGetStats
(
    IN S_N5XC56GP5X4_EYE_RAW_PTR eyeRawDataPtr,
    IN S_N5XC56GP5X4_EYE_BUFFER_PTR eyeBufferDataPtr,
    INOUT S_EOM_STATS_EYE_DIM* estimateEyeDimPtr,
    IN MCESD_U16 estimateEyeDimCount,
    OUT S_EOM_STATS_EYE_AMP *amplitudeStats
);
#endif

/**
@brief  Returns eye width and upper/lower height based on eye raw data

@param[in]  eyeRawDataPtr - pointer to S_N5XC56GP5X4_EYE_RAW_PTR which store eye raw data
@param[in]  berThreshold - Bit Error Rate Threshold in nano (factor of 1E-9)

@param[out] width - eye width
@param[out] heightUpper - upper eye height
@param[out] heightLower - lower eye height

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_EyeDataDimensions
(
    IN S_N5XC56GP5X4_EYE_RAW_PTR eyeRawDataPtr,
    IN MCESD_U32 berThreshold,
    OUT MCESD_U32* width,
    OUT MCESD_U32* heightUpper,
    OUT MCESD_U32* heightLower
);

/**
@brief  Perform CDS

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, 2, 3, etc.

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_ExecuteCDS(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane
);

/**
@brief  Asserts TX/RX Reset

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, 2, 3, etc.
@param[in]  txReset - MCESD_TRUE to reset tx, otherwise MCESD_FALSE
@param[in]  rxReset - MCESD_TRUE to reset rx, otherwise MCESD_FALSE

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_AssertTxRxCoreReset
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_BOOL txReset,
    IN MCESD_BOOL rxReset
);

/**
@brief  Get State of TX/RX Reset Acknowledgement

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, 2, 3, etc.

@param[out]  txReset - MCESD_TRUE means reset acknowledgement is HI, else MCESD_FALSE
@param[out]  rxReset - MCESD_TRUE means reset acknowledgement is HI, else MCESD_FALSE

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_GetResetCoreAckTxRx
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT MCESD_BOOL* txReset,
    OUT MCESD_BOOL* rxReset
);

/**
@brief  Set Reserved Input RX bit 0

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, 2, 3, etc.
@param[in]  enable - MCESD_TRUE to enable, otherwise MCESD_FALSE
@param[in]  fieldOverride - MCESD_TRUE to use field override, otherwise MCESD_FALSE will use pin

@note Function is used for Ultra Short Reach mode sequence

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_SetReservedInputRX0
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_BOOL enable,
    IN MCESD_BOOL fieldOverride
);

/**
@brief  Get Reserved Input RX bit 0

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, 2, 3, etc.
@param[out] enable - MCESD_TRUE means enabled, otherwise MCESD_FALSE

@note Function is used for Ultra Short Reach mode sequence

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_GetReservedInputRX0
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT MCESD_BOOL *enable
);

/**
@brief  Sets MCU Remote Command and MCU Remote Status

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, 2, 3, etc.
@param[in]  cmdType - command type
@param[in]  subCategory - sub-category
@param[in]  cmdNum - command number
@param[in]  controlBits - control bits
@param[in]  remoteStatus - remote status

@note See MCU Command Interface in Design Spec

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_SetMcuRemoteCmd
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN E_N5XC56GP5X4_MRC_TYPE cmdType,
    IN E_N5XC56GP5X4_MRC_SUB subCategory,
    IN MCESD_U8 cmdNum,
    IN MCESD_U8 controlBits,
    IN MCESD_U16 remoteStatus
);

/**
@brief  Set MCU Remote Request

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, 2, 3, etc.
@param[in]  level - MCESD_TRUE to set high, MCESD_FALSE to set low

@note See MCU Command Interface in Design Spec

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_SetMcuRemoteReq
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_BOOL level
);

/**
@brief  Get MCU Local Acknowledge

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, 2, 3, etc.
@param[out] level - MCESD_TRUE means acknowledged, MCESD_FALSE means not acknowledged

@note See MCU Command Interface in Design Spec

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_GetMcuLocalAck
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT MCESD_BOOL *level
);

/**
@brief  Get MCU Local Status

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, 2, 3, etc.
@param[out] localStatus - local status value

@note See MCU Command Interface in Design Spec

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_GetMcuLocalStatus
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    OUT MCESD_U32 *localStatus
);

/**
@brief  Display Tx Training Log

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, 2, 3, etc.
@param[in]  logArrayDataPtr - pointer to array of S_N5XC56GP5X4_TLOG_ENTRY
@param[in]  logArraySizeEntries - size of array

@param[out] validEntries - number of valid entries in provided array

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_DisplayTrainingLog
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN S_N5XC56GP5X4_TLOG_ENTRY logArrayDataPtr[],
    IN MCESD_U32 logArraySizeEntries,
    OUT MCESD_U32 *validEntries
);

/**
@brief  Disable Tx Fir Force

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane number 0, 1, 2, 3, etc.

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_N5XC56GP5X4_DisableTxFirForce
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane
);

#if C_LINKAGE
#if defined __cplusplus
}
#endif
#endif

#endif /* N5XC56GP5X4 */

#endif /* defined MCESD_N5XC56GP5X4_API_H */
