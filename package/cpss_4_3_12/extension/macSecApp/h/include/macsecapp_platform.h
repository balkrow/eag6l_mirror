/**

   Interface to the platform-specific part of QuickSec for MACsec
   example application.

   File: macsecapp_platform.h

   @description

   @copyright
   Copyright (c) 2012 - 2021 Rambus Inc, all rights reserved.

*/

#ifndef MACSECAPP_PLATFORM_H
#define MACSECAPP_PLATFORM_H

#define _DEFAULT_SOURCE
#define _POSIX_SOURCE
#define _POSIX_C_SOURCE 200112L

#include <stdarg.h>

#if !defined(__GNUC__) && !defined(__attribute__)
#define __attribute__(x)
#endif

/*
 * Try opening an interactive terminal. Return a non-negative file
 * descriptor if successful, -1 otherwise.
 */
int ma_open_terminal();

/*
 * Close terminal previously opened using ma_open_terminal().
 */
void ma_close_terminal(int fd);

/*
 * Try reading one character from terminal. Return character value
 * (max 255) if successful, -1 otherwise.
 */
int ma_read_terminal(int fd);

/*
 * Output a one-line error message.
 */
void ma_errorf(const char *fmt, ...)
     __attribute__ ((format (printf, 1, 2)));
void ma_verrorf(const char *fmt, va_list ap);

/*
 * Output a one-line error message in two parts.
 */
void ma_errorf_begin(const char *fmt, ...)
     __attribute__ ((format (printf, 1, 2)));
void ma_verrorf_end(const char *fmt, va_list ap);

/*
 * Entry point in the platform-independent part.
 */
int ma_main(int argc, char *argv[]);

/*
 * Command handlers in the platform-independent part.
 */
void ma_quit(void);
void ma_print(void);
void ma_configure(void);

void ma_resume(void);

#endif /* MACSECAPP_PLATFORM_H */
