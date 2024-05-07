/**

   Inclusion of C library headers for Linux kernel.

   File: macsec_clib.h

   @description
   This file includes appropriate header files to provide declarations
   of the following types, macros and functions: assert(), offsetof,
   NULL, uint32_t, uint64_t, memcmp(), memcpy(), memmove() and
   memset().

   @copyright
   Copyright (c) 2012 - 2013 Rambus Inc, all rights reserved.

*/


#ifndef _MACSEC_CLIB_H
#define _MACSEC_CLIB_H

#include <linux/bug.h>
#define assert(x) BUG_ON(!(x))
#include <linux/stddef.h>
#include <linux/types.h>
#include <linux/string.h>

#endif /* _MACSEC_CLIB_H */
