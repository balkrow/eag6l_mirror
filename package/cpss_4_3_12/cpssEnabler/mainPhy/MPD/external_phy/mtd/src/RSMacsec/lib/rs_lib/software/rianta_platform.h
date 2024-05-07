/*! 
 * @file rianta_platform.h
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
// * $Rev: 1.1.0
// * $Date: 2022-11-17-16:28:56
// ******************************************************************************

#ifndef __RIANTA_PLATFORM_H__
#define __RIANTA_PLATFORM_H__

#include "rianta_arch.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * These functions need to be supplied by the hardware interface driver.
 */
/*! @brief Hardware access function to read a register
 *
 * @param[inout] userData_p  Pointer to HW accedd information
 * @param[in]    offset      The offset from teh base address of the device
 * @param[out]   value       Pointer to a register sized uint to hold the result
 *
 * @retval 0    Success
 * @retval -int Failure code.
 *
 * The system integrator is expected to provide a function to read a register from this device.
 * Handling of the physical addressing is assumed to occur within the hardware access code. The return
 * value from this is propegated back through the driver to the calling function, a negative value is
 * interpreted as a failure and prevents further processing, a 0 or positive value is considered a
 * success. NOTE: Zero is guaranteed to be considered a successful return value.
 *
 * The userData_p is for the host system use.
 */
typedef int (* RmsHwReadFn_t)( void * userData_p, uintAddrBus_t const offset, uintRegAccess_t volatile * value );

/*! @brief Hardware access function to write a register
 *
 * @param[inout] userData_p  Pointer to HW access information
 * @param[in]    offset      The offset from the base address of the device
 * @param[out]   value       Pointer to a register sized uint to write to offset
 *
 * @retval 0    Success
 * @retval -int Failure code.
 *
 * The system integrator is expected to provide a function to write a register on this device.
 * Handling of the physical addressing is assumed to occur within the hardware access code. The return
 * value from this is propegated back through the driver to the calling function, a negative value is
 * interpreted as a failure and prevents further processing, a 0 or positive value is considered a
 * success. NOTE: Zero is guaranteed to be considered a successful return value.
 *
 * The userData_p is for the host system use.
 */
typedef int (* RmsHwWriteFn_t)( void * userData_p, uintAddrBus_t const offset, uintRegAccess_t const value );

#ifdef __cplusplus
}
#endif

/*! @brief A handle to the hardware
 *
 * This should be a suitable handle to be passed through the driver into the
 * HW instance to facilitate mutiple instnace handling.
 */
typedef struct {
    uint32_t                    version;             //!< The version of this struct. This must alway be first!
    uint32_t                    capabilities;        //!< Device capabilities bitmap
    RmsHwReadFn_t               rmsHwReadFn;         //!< User supplied read function.
    RmsHwWriteFn_t              rmsHwWriteFn;        //!< User supplied write function.
    void *                      userData_p;          //!< Pointer to data required by the HW access functions
} RmsDev_t;


#define DEBUG (1)
#if DEBUG
#   define DBGRETURN(ret)					\
  do { if( (ret) < 0 ) {					\
      printf("%s:%d ret/%d\n", __FILE__, __LINE__, (ret));	\
      return (ret);						\
    } } while(0)
#else
#   define DBGRETURN(ret)			\
  do { if( (ret) < 0 ) { return (ret); } } while(0)
#endif

#endif // __RIANTA_PLATFORM_H__
