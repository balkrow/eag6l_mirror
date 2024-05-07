/*! @file sim_log.h
 *-----------------------------------------------------------------------------
 * @brief A primitive logger.
 *
 * @par Copyright
 * Copyright 2018-2020 Rianta Solutions Inc. All Rights Reserved.
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
/*
 * define DEFAULT_SIM_PRINT_STREAM to send the log somewhere, but the caller also needs to open it themselves.
 *
 * TODO: BUild a better logging system.
 */
#ifndef __SIM_LOG_H__
#define __SIM_LOG_H__

#include <stdio.h>
#include <stdarg.h>

#ifdef MACSEC_API_IEEE
#include "mcs_internals_ieee.h"
#else
#include "mcs_internals_CT.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*! @brief log printer
 *
 * @param[in]  stream   Stream pointer to log to
 * @param[in]  format   printf format string
 * @param[in]  va       Variable Argument list.
 *
 * @return 0 on success, negative error code otherwise
 *
 * This is the prototype for the sim logger logging file. The caller needs to
 * supply this function to direct the logging. If it is not supplied vfprintf
 * (to stderr) will be used.  NOTE: it is not required that the printer function
 * use a stream, however it needs to provide at least a placeholder for one.
 */
typedef int (*sim_log_printer_t)(FILE * stream, char const * format, va_list va);

/*! @brief Register a print function
 *
 * @param[in] new_stream   The stream to print to.
 * @param[in] new_printer  The print function to register.
 *
 * @return 0 on success, negative error code otherwise
 *
 * Register a new output function for the logger. If new_stream is NULL then
 * DEFAULT_SIM_PRINT_STREAM will be used.
 *
 * @sa sim_log_printer_t
 */
int sim_log_register( FILE * new_stream, sim_log_printer_t new_printer );

/*! @brief Print to the logger.
 *
 * @param[in] format   A printf format string
 * @param[in] ...      A va list.
 *
 * @return 0 on success, negative error code otherwise
 *
 * Send some data to the registered logging function. NOTE: it is usually
 * preferable to use the SIM_PRINT macro.
 *
 * @sa SIM_PRINT
 * @sa sim_log_register
 */
int sim_log_printf( char const * format, ... );

/*! @brief a simple errno stringifier
 *
 * @param[in] err   The errno to print.
 *
 * @returns char const *  A pointer to the string.
 *
 * NOTE: this function only implements a subset of errno.h
 */
char const * sim_log_stringify_result( int err );

#ifdef __cplusplus
}
#endif

//#undef NDEBUG

/*! @brief logging levels
 *
 * Lower is more severe. Used for filtering. The filter level can be set on a
 * file level using SIM_PRINT_LEVEL. The value set andlower will be printed.
 *
 * Example: If SIM_PRINT_LEVEL is SIM_WARNING then critical, error and warning will be
 * logged and info and debug will be discarded.
 *
 * Set at compiler time!
 */
typedef enum {
    SIM_CRITICAL,       //!< Critical: Run-time error, no workaround, causes system halt
    SIM_ERROR,          //!< Error: Unexpected event. May cause system to behave incorrectly.
    SIM_WARNING,        //!< Warning: Unexpected event with workaround.
    SIM_INFO,           //!< Info: For developer information.
    SIM_DEBUG,          //!< Debug: Very verbose. Used for system development/debugging.
} sim_log_levels_t;

// default level when no level is set.
#ifndef SIM_PRINT_LEVEL
/*! @brief default logging level
 *
 * If no level is set then this will be used.
 */
#define SIM_PRINT_LEVEL SIM_WARNING
#endif

// if no log level is set then send it stderr
#ifndef DEFAULT_SIM_PRINT_STREAM
/*! @brief default stream for logging.
 */
#define DEFAULT_SIM_PRINT_STREAM stderr
#endif

#ifndef NDEBUG
/*! @brief internal logger macro
 *
 * ** DO NOT USE THIS **
 *
 * Check the level and if low enough then send to the logger.
 *
 * @sa sim_log_levels_t
 */
#define _do_sim_log( level, sev, fmt, ... ) do { \
                                                if ( level <= SIM_PRINT_LEVEL ) sim_log_printf( "%s: " fmt "\n", sev, ##__VA_ARGS__ ); \
                                            } while(0);

/*! @brief Logger macro.
 *
 * @param[in] level  The level of this message
 * @param[in] fmt    The format string
 * @param[in] ...    VA_LIST if required
 *
 * NOTE this macro adds the newline. There is no need to add one in the fmt string.
 */
#define SIM_PRINT( level, fmt, ...) _do_sim_log( level, #level, fmt, ##__VA_ARGS__ )

#else    // NDEBUG
#define SIM_PRINT(...)
#endif   // NDEBUG

#endif // __SIM_LOG_H__
