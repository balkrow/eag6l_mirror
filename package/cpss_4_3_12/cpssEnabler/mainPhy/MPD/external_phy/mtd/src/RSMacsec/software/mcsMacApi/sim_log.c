/*-----------------------------------------------------------------------------
 * Copyright 2018-2020 Rianta Solutions Inc. All Rights Reserved.
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
 *---------------------------------------------------------------------------*/

#include <stdarg.h>
#include <stdio.h>
#include <stdbool.h>
#include <errno.h>

#include "sim_log.h"

static sim_log_printer_t sim_log_printer = NULL;
static FILE *            sim_log_stream  = NULL;
static bool              once = true;

DLL_PUBLIC int sim_log_printf( char const * format, ... )
{
    int ret = -ENODEV;

    if( once ) {
        sim_log_register( stderr, &vfprintf );
        once = false;
    }
    if( sim_log_printer != NULL ) {
        va_list va;

        va_start(va, format);
        ret = sim_log_printer( sim_log_stream, format, va );
        va_end(va);
    }

    return ret;
}

int sim_log_register( FILE * new_stream, sim_log_printer_t new_printer )
{
    sim_log_printer = new_printer;
    if( new_stream == NULL ) {
        sim_log_stream = DEFAULT_SIM_PRINT_STREAM;
    } else {
        sim_log_stream = new_stream;
    }
    once = false;

    return 0;
}

char const * sim_log_stringify_result( int err )
{
    switch(err) {
        case -EINVAL:
            return "EINVAL";
        case -EPERM:
            return "EPERM";
        case -EFAULT:
            return "EFAULT";
        case -ENOMEM:
            return "ENOMEM";
        case 0:
            return "SUCCESS";
        default:
            return "UNKNOWN";
    }
}

