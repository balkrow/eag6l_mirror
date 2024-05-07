/**
   
   @copyright
   Copyright (c) 2002 - 2013, Rambus Inc. All rights reserved.
   
   File: sshincludes.h
   
   Common include files for various platforms.
   
*/


#ifndef SSHINCLUDES_H
#define SSHINCLUDES_H

#define  _DEFAULT_SOURCE
#define _POSIX_SOURCE
#define _POSIX_C_SOURCE 200112L
#ifndef __USE_MISC
    #define __USE_MISC
#endif

/* Defines related to segmented memory architectures. */
#ifndef NULL_FNPTR
#define NULL_FNPTR  NULL
#else
#define HAVE_SEGMENTED_MEMORY  1
#endif

/* */
#include "sshdistdefs.h"

#define SSH_NOT_BUILT_DUE_TO_MISSING_DISTDEFS \
"Program is not built with the specified distribution defines." 


#ifndef ASM_PLATFORM_OCTEON
/* Macros for giving branch prediction hints to the compiler. The
   result type of the expression must be an integral type. */
#if __GNUC__ >= 3
#define SSH_PREDICT_TRUE(expr) __builtin_expect(!!(expr), 1)
#define SSH_PREDICT_FALSE(expr) __builtin_expect(!!(expr), 0)
#else /* __GNUC__ >= 3 */
#define SSH_PREDICT_TRUE(expr) (!!(expr))
#define SSH_PREDICT_FALSE(expr) (!!(expr))
#endif /* __GNUC__ >= 3 */

/* Macros for marking functions to be placed in a special section. */
#if __GNUC__ >= 3
#define SSH_FASTTEXT __attribute__((__section__ (".text.fast")))
#else /* __GNUC__ >= 3 */
#define SSH_FASTTEXT
#endif /* __GNUC__ >= 3 */

#else /* !ASM_PLATFORM_OCTEON */
/* Disabling branch prediction and the FASTTEXT macro for the Octeon platform 
   as they only degrade performance.  */
#define SSH_PREDICT_TRUE(expr) (!!(expr))
#define SSH_PREDICT_FALSE(expr) (!!(expr))
#define SSH_FASTTEXT

#endif /* !ASM_PLATFORM_OCTEON */

/* Some generic pointer types. */
typedef char *SshCharPtr;
typedef void *SshVoidPtr;

#if defined(KERNEL) || defined(_KERNEL)

#if defined (WINNT)
#include "sshincludes_ntddk.h"
#elif defined (WIN95)
#include "sshincludes_w95ddk.h"
#elif defined (macintosh)
#include "kernel_includes_macos.h"
#else
#include "kernel_includes.h"
#endif /* WINNT */

/* Some internal headers used in almost every file. */
#include "sshdebug.h"
#include "engine_alloc.h"

#else /* KERNEL || _KERNEL */

#define SSH_TYPES_DEFINED
#include "sshtypes.h"

#if defined(WIN32)
# include "sshincludes_win32.h"
#elif defined (macintosh)
# include "mac/sshincludes_macos.h"
#elif defined (__SYMBIAN32__)
#include "sshincludes_symbian.h"
#elif defined (VXWORKS)
# include "sshincludes_vxworks.h"
#else 
# include "sshincludes_unix.h"
#endif /* WIN32 */

#ifdef SSH_INCLUDE_ENGINE_ALLOC_H
#include "engine_alloc.h"
#endif /* SSH_INCLUDE_ENGINE_ALLOC_H */
#include "sshmalloc.h"
#endif /* KERNEL || _KERNEL */

#ifdef HAVE_LIBSSHCRYPTO
#undef SSHDISTDEFS_H
#include "sshdistdefscrypto.h"
#endif /* HAVE_LIBSSHCRYPTO */

#ifdef HAVE_LIBSSHUTIL
#undef SSHDISTDEFS_H
#include "sshdistdefsutil.h"
#endif /* HAVE_LIBSSHUTIL */

#ifdef HAVE_LIBSSH
#undef SSHDISTDEFS_H
#include "sshdistdefsssh.h"
#endif /* HAVE_LIBSSH */

/* Common (operating system independent) stuff below */

#include "sshsnprintf.h"

/* Do not perform code checks in kernel space. */
#ifndef KERNEL

#ifndef SSH_ALLOW_SYSTEM_SPRINTFS

/* The sprintf and vsprintf functions are FORBIDDEN in all SSH code.
   This is for security reasons - they are the source of way too many
   security bugs.  Instead, we guarantee the existence of snprintf and
   ssh_vsnprintf.  These MUST be used instead. */
#ifdef sprintf
# undef sprintf
#endif
#define sprintf ssh_fatal(SPRINTF_IS_FORBIDDEN_USE_SSH_SNPRINTF_INSTEAD)

#ifdef vsprintf
# undef vsprintf
#endif
#define vsprintf ssh_fatal(VSPRINTF_IS_FORBIDDEN_USE_SSH_VSNPRINTF_INSTEAD)

#ifdef snprintf
# undef snprintf
#endif
#define snprintf ssh_fatal(SNPRINTF_IS_FORBIDDEN_USE_SSH_SNPRINTF_INSTEAD)

#ifdef vsnprintf
# undef vsnprintf
#endif
#define vsnprintf ssh_fatal(VSNPRINTF_IS_FORBIDDEN_USE_SSH_VSNPRINTF_INSTEAD)

#endif /* !SSH_ALLOW_SYSTEM_SPRINTFS */

#ifdef index
# undef index
#endif
#define index(A,B) ssh_fatal(INDEX_IS_BSDISM_USE_STRCHR_INSTEAD)

#ifdef rindex
# undef rindex
#endif
#define rindex(A,B) ssh_fatal(RINDEX_IS_BSDISM_USE_STRRCHR_INSTEAD)

/* Force library to use ssh- memory allocators (they may be
   implemented using zone mallocs, debug-routines or something
   similar) */

#ifndef SSH_ALLOW_SYSTEM_ALLOCATORS
#ifdef malloc
# undef malloc
#endif
#ifdef calloc
# undef calloc
#endif
#ifdef realloc
# undef realloc
#endif
#ifdef free
# undef free
#endif
#ifdef strdup
# undef strdup
#endif
#ifdef memdup
# undef memdup
#endif

# define malloc  MALLOC_IS_FORBIDDEN_USE_SSH_XMALLOC_INSTEAD
# define calloc  CALLOC_IS_FORBIDDEN_USE_SSH_XCALLOC_INSTEAD
# define realloc REALLOC_IS_FORBIDDEN_USE_SSH_XREALLOC_INSTEAD
# define free    FREE_IS_FORBIDDEN_USE_SSH_XFREE_INSTEAD
# define strdup  STRDUP_IS_FORBIDDEN_USE_SSH_XSTRDUP_INSTEAD
# define memdup  MEMDUP_IS_FORBIDDEN_USE_SSH_XMEMDUP_INSTEAD
#endif /* SSH_ALLOW_SYSTEM_ALLOCATORS */

#ifdef time
# undef time
#endif
#define time(x) ssh_fatal(TIME_IS_FORBIDDEN_USE_SSH_TIME_INSTEAD)

#ifdef localtime
# undef localtime
#endif
#define localtime \
        ssh_fatal(LOCALTIME_IS_FORBIDDEN_USE_SSH_CALENDAR_TIME_INSTEAD)

#ifdef gmtime
# undef gmtime
#endif
#define gmtime ssh_fatal(GMTIME_IS_FORBIDDEN_USE_SSH_CALENDAR_TIME_INSTEAD)

#ifdef asctime
# undef asctime
#endif
#define asctime ssh_fatal(ASCTIME_IS_FORBIDDEN)

#ifdef ctime
# undef ctime
#endif
#define ctime ssh_fatal(CTIME_IS_FORBIDDEN)

#ifdef mktime
# undef mktime
#endif
#define mktime ssh_fatal(MKTIME_IS_FORBIDDEN_USE_SSH_MAKE_TIME_INSTEAD)

/* Conditionals for various OS & compilation environments */

















































































































































#endif /* !KERNEL */

/* Some internal headers used in almost every file. */
#include "sshdebug.h"
#include "sshtime.h"

/* Unitialize and free resource allocated by the utility library,
   including debugging subsystem and global variable storage. */
void ssh_util_uninit(void);

#define SSH_CODE_SEGMENT SSH_RODATA_IN_TEXT
#define SSH_RODATA_IN_TEXT
#define SSH_RODATA
#define SSH_BSS_INITONCE
#define SSH_DATA_INITONCE
#define SSH_DATA_ALLOWED
#define SSH_BSS_ALLOWED

/* Define UID_ROOT to be the user id for root (normally zero, but different
   e.g. on Amiga). */
#ifndef UID_ROOT
#define UID_ROOT 0
#endif /* UID_ROOT */







































#ifdef WITH_REPLACEMENT_MEMCMP

# ifdef memcmp
#  undef memcmp
# endif
# define memcmp  ssh_memcmp

# include "sshmemcmp.h"
#else

# define ssh_memcmp memcmp
#endif  /* WITH_REPLACEMENT_MEMCMP */


















/* Prototypes for missing functions in the host system */

#ifndef HAVE_STRCASECMP
int strcasecmp(const char *s1, const char *s2);
#endif /* !HAVE_STRCASECMP */

#ifndef HAVE_STRNCASECMP
int strncasecmp(const char *s1, const char *s2, size_t len);
#endif /* !HAVE_STRNCASECMP */

/* Cast the argument `string' of type `const char *' into `const
   unsigned char *' type.  This function is equivalent to the `(const
   unsigned char *)' C cast but since it is implemented as a function,
   it provides type checking for the argument `string'.  This is a
   convenience function to be used when a C-string is passed for a
   function taking an `unsigned char *' argument. */
const unsigned char *ssh_custr(const char *string);

/* Cast the argument `string' of type `const unsigned char *' into
   `const char *' type.  This function is equivalent to the `(const
   char *)' C cast but since it is implemented as a function,
   it provides type checking for the argument `string'.  This is a
   convenience function to be used when a company standard string or
   buffer (specified by the Architecture Board to be of an unsigned type)
   is passed for a function taking a `const char *' argument like some
   standard str*() functions tend to do. */
const char *ssh_csstr(const unsigned char *string);

/* Cast the argument `string' of type `char *' into `unsigned char *' type.
   This function is equivalent to the `(unsigned char *)' C cast but since
   it is implemented as a function, it provides type checking for the
   argument `string'.  This is a convenience function to be used when
   for example a standard C function returns a signed char pointer
   that is to be converted to an unsigned char pointer to be used in
   the company library functions. */
unsigned char *ssh_ustr(char *string);

/* Cast the argument `string' of type `unsigned char *' into `char *' type.
   This function is equivalent to the `(char *)' C cast but since it is
   implemented as a function, it provides type checking for the argument
   `string'.  This is a convenience function to be used when a company
   standard string or buffer (specified by the Architecture Board to be
   of an unsigned type) is passed for a function taking a `char *'
   argument like some standard str*() functions tend to do. */
char *ssh_sstr(unsigned char *string);


#if !defined (KERNEL) && !defined(_KERNEL)

#ifdef HAVE_SOCKLEN_T
#include <sys/socket.h>
typedef socklen_t ssh_socklen_t;
#else /* HAVE_SOCKLEN_T */
typedef unsigned int ssh_socklen_t;
#endif /* HAVE_SOCKLEN_T */

#endif /* !KERNEL && ! _KERNEL */


/* Variants of the C library functions which take NUL-terminated 
   unsigned character strings as input. */ 

/* strlen */
int ssh_ustrlen(const unsigned char *str);

/* strcmp */
int ssh_ustrcmp(const unsigned char *str1, const unsigned char *str2);

/* strcmp with literal as str2 */
int ssh_usstrcmp(const unsigned char *str1, const char *str2);

/* strncmp */
int ssh_ustrncmp(const unsigned char *str1, const unsigned char *str2,
		 size_t len);

/* strncmp with literal as str2 */
int ssh_usstrncmp(const unsigned char *str1, const char *str2, size_t len);


/* strcpy */
unsigned char *ssh_ustrcpy(unsigned char *str1, const unsigned char *str2);

/* strncpy */
unsigned char *ssh_ustrncpy(unsigned char *str1, const unsigned char *str2,
			    size_t len);

/* strcat */
unsigned char *ssh_ustrcat(unsigned char *str1, const unsigned char *str2);

/* strdup */
unsigned char *ssh_ustrdup(const unsigned char *str);

/* strchr */
unsigned char *ssh_ustrchr(const unsigned char *str, int c); 

/* strcasecmp */
int ssh_ustrcasecmp(const unsigned char *str1, const unsigned char *str2);

/* strcasecmp with literal as str2 */
int ssh_usstrcasecmp(const unsigned char *str1, const char *str2);

/* strncasecmp */
int ssh_ustrncasecmp(const unsigned char *str1, const unsigned char *str2, 
		    size_t n);

/* strncasecmp with literal as str2 */
int ssh_usstrncasecmp(const unsigned char *str1, const char *str2, 
		    size_t n);

/* atoi */
int ssh_uatoi(const unsigned char *str);

/* atol */
long ssh_uatol(const unsigned char *str);

/* strtol */
int ssh_ustrtol(const unsigned char *nptr, char **endptr, int base);

/* strtoul */
int ssh_ustrtoul(const unsigned char *nptr, char **endptr, int base);

/* strlen */
int ssh_ustrlen(const unsigned char *str);


/*
  ssh implementations of string copying 
 */

/* strcpy */
char *ssh_strcpy(char *dest, const char *src);

/* strncpy */
char *ssh_strncpy(char *dest, const char *src, size_t n);


#endif /* SSHINCLUDES_H */
