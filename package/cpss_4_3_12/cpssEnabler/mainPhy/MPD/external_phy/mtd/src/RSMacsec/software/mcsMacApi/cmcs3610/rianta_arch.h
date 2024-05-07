/*! @file rianta_arch.h
 *-----------------------------------------------------------------------------
 * @brief Architecture info for mcs_internals.h
 *
 * @par Copyright
 * Copyright 2019-2020 Rianta Solutions Inc. All Rights Reserved.
 *-----------------------------------------------------------------------------
 *
 * All information contained herein is, and remains the property of
 * Rianta Solutions Inc. and its suppliers, if any. The intellectual and
 * technical concepts contained herein are proprietary to Rianta
 * Solutions Inc. and its suppliers and may be covered by U.S. and
 * Foreign Patents, patents in process, and are protected by trade secret
 * or copyright law. Dissemination of this information or reproduction of
 * this material is strictly forbidden unless prior written permission is
 * obtained from Rianta Solutions Inc.
 *-----------------------------------------------------------------------------
 * $Date: 2022-11-17-16:28:56
 * $Rev: 1.1.0
 *
 *---------------------------------------------------------------------------*/
#ifndef __MCS_ARCH_H__
#define __MCS_ARCH_H__

/*! @brief Compiler time parameter check.
 *
 * This macro will cause a compiler error if __cond__ is true. Use to verify assumptions made while
 * programming are still true when building.
 */
#define BUG_BUILD_ON(__cond__) ((void)sizeof(char[1 - 2*!!(__cond__)]))

/*! @def DLL_PUBLIC
 *
 * @brief Mmacro to export a function from a library.
 *
 * MSVC and gcc versions after 4 don't default to emitting external linkage of
 * functions from a dynamically loaded library (dll or .so)
 */
/*! @def DLL_LOCAL
 *
 * @brief A macro to declare a function local when building a library. That is
 * do not emit any linkage from the compilation unit.
 */
#if defined _WIN32 || defined __CYGWIN__ || defined WIN32
  #ifdef BUILDING_DLL
    #ifdef __GNUC__
       #define DLL_PUBLIC __attribute__ ((dllexport))
    #else
       #define DLL_PUBLIC __declspec(dllexport)
    #endif
  #else
    #ifdef __GNUC__
       #define DLL_PUBLIC __attribute__ ((dllimport))
    #else
       #define DLL_PUBLIC __declspec(dllimport)
    #endif
  #endif
  #define DLL_LOCAL
#else
  #if __GNUC__ >= 4
    #define DLL_PUBLIC __attribute__ ((visibility ("default")))
    #define DLL_LOCAL __attribute__ ((visibility ("hidden")))
  #else
    #define DLL_PUBLIC
    #define DLL_LOCAL
  #endif
#endif

/*! @def MACSEC_PACKED
 *
 * @brief Force a struct to be packed.
 */
/*! @def MACSEC_ALIGNED
 *
 * @brief forcea storage unit to be aligned
 */
#if defined __GNUC__
#define MACSEC_PACKED( class_to_pack ) class_to_pack __attribute__((__packed__))
#define MACSEC_ALIGNED(x) __attribute__ ((aligned(x)))
#else
#define MACSEC_PACKED( class_to_pack ) __pragma( pack(push, 1) ) class_to_pack __pragma( pack(pop) )
#define MACSEC_ALIGNED(x) __declspec(align(x))
#endif

/*! @brief the size used to access a register
 *
 * Note that this may be different from the size of the register itself.
 */
typedef uint32_t uintRegAccess_t;

/*! @brief The size of the address bus.
 */
typedef uint32_t uintAddrBus_t;

/*! @brief The size of a register
 */
typedef uint64_t uintRegister_t;

#ifdef __cplusplus
extern "C" {
#endif


#ifdef __cplusplus
}
#endif

#endif // __MCS_ARCH_H__

