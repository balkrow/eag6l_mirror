/*******************************************************************************
Copyright (C) 2019, Marvell International Ltd. and its affiliates
If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.
*******************************************************************************/

/********************************************************************
This file contains functions to read/write registers for higher level API 
to bridge host's hardware-specific IO.
********************************************************************/
#ifndef MCESD_C28GP4X2_REGRW_H
#define MCESD_C28GP4X2_REGRW_H

#ifdef C28GP4X2

#if C_LINKAGE
#if defined __cplusplus 
    extern "C" { 
#endif 
#endif  

/**
@brief  Writes a 32-bit value to the specified address

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane to write 0,1,2 etc.
@param[in]  reg - register address
@param[in]  value - data to write

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C28GP4X2_WriteReg
( 
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane, 
    IN MCESD_U32 reg, 
    IN MCESD_U32 value
);

/**
@brief  Reads a 32-bit data from the specified address

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane to write 0,1,2 etc.
@param[in]  reg - register address

@param[out] data - Returns 32 bit data from register address

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C28GP4X2_ReadReg
( 
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane, 
    IN MCESD_U32 reg, 
    OUT MCESD_U32 *data
);

/**
@brief  Writes a 32-bit value to the specified field

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane to write 0,1,2 etc.
@param[in]  fieldPtr - pointer to MCESD_FIELD to write data
@param[in]  value - data to write

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C28GP4X2_WriteField
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_FIELD_PTR fieldPtr,
    IN MCESD_U32 value
);

/**
@brief  Reads a 32-bit data from the specified field

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane to write 0,1,2 etc.
@param[in]  fieldPtr - pointer to MCESD_FIELD to read from

@param[out] data - Returns 32 bit data from field

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C28GP4X2_ReadField
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_FIELD_PTR fieldPtr,
    OUT MCESD_U32 *data
);

/**
@brief  Waits for the specified value to be present for the field

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  lane - lane to write 0,1,2 etc.
@param[in]  fieldPtr - pointer to MCESD_FIELD to check value
@param[in]  value - value to match
@param[in]  timeout_ms - timeout in milliseconds

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C28GP4X2_PollField
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_FIELD_PTR fieldPtr,
    IN MCESD_U32 value,
    IN MCESD_U32 timeout_ms
);

/**
@brief  Waits for the specified value to be present for the pin

@param[in]  devPtr - pointer to MCESD_DEV initialized by mcesdLoadDriver() call
@param[in]  pin - enum of type E_C28GP4X2_PIN
@param[in]  value - value to match
@param[in]  timeout_ms - timeout in milliseconds

@retval MCESD_OK - on success
@retval MCESD_FAIL - on error
*/
MCESD_STATUS API_C28GP4X2_PollPin
(
    IN MCESD_DEV_PTR devPtr,
    IN E_C28GP4X2_PIN pin,
    IN MCESD_U16 value,
    IN MCESD_U32 timeout_ms
);

#if C_LINKAGE
#if defined __cplusplus 
}
#endif 
#endif

#endif /* C28GP4X2 */

#endif /* defined MCESD_C28GP4X2_REGRW_H */
