/**
   
   @copyright
   Copyright (c) 2002 - 2013, Rambus Inc. All rights reserved.
   
   A method for accessing the frame pointer and program counter 
   of each stack frame on various platforms. However, the user 
   must check that SSH_FP_OK is defined - if it is not, then the 
   functionality listed here is not provided.
   
   <keywords frame pointer, program counter, utility functions/stack, 
   SSH_FP_OK>
   
   File: sshfp.h
   
*/


/* *********************** i386 and GCC *****************************/

#if defined(__GNUC__) && defined(__i386__) && !defined(__STRICT_ANSI__)

/**  Get the currently active frame-pointer value to 'fp'. */
#define SSH_GET_FP(fp)                          \
do                                              \
{                                               \
  register void *_fp;                           \
  asm volatile ("movl %%ebp, %0" : "=r" (_fp)); \
  (fp) = _fp;                                   \
}                                               \
while (0)

/**  Get frame-pointer from the frame, pointed by 'fp'. */
#define SSH_FP_GET_FP(fp) (*((void **) (fp)))

/**  Get program counter from the frame, pointed by 'fp'.  */
#define SSH_FP_GET_PC(fp) (*((void **) (((unsigned char *) fp) + 4)))

#ifdef HAVE_LIBC_STACK_END
extern void *__libc_stack_end;
/*   Skip 2 last frames. */
#define SSH_VALID_FP(fp) (((fp) != NULL) && SSH_FP_GET_PC(fp) != NULL \
  && (SSH_FP_GET_FP(fp) != NULL && SSH_FP_GET_FP(fp) > (fp) && \
      SSH_FP_GET_FP(fp) < __libc_stack_end) \
  && (SSH_FP_GET_FP(SSH_FP_GET_FP(fp)) != NULL && \
      SSH_FP_GET_FP(SSH_FP_GET_FP(fp)) > (fp) && \
      SSH_FP_GET_FP(SSH_FP_GET_FP(fp)) < __libc_stack_end))
#else /* HAVE_LIBC_STACK_END */
#ifdef __linux__
/**  A predicate to check whether the frame-pointer 'fp' is valid.
     For Linux 2.6 we use a heuristic, we also require that
     the next frame pointer on the stack is not also contained
     in *(fp+3). This is the case in the main() frame. */
#define SSH_VALID_FP(fp) \
(((fp) != NULL) && SSH_FP_GET_PC(fp) != NULL \
&& (*(((void **)fp) + 3)) != SSH_FP_GET_FP(fp))
#else /* __linux__ */
#ifdef __OpenBSD__
/*   A predicate to check whether the frame-pointer `fp' is valid. */
extern void _start, _end;
#define SSH_VALID_FP(fp) (((fp) != NULL) \
  && SSH_FP_GET_PC(fp) >= &_start \
  && SSH_FP_GET_PC(fp) < &_end)
#else /* __OpenBSD__ */
/*   A predicate to check whether the frame-pointer `fp' is valid. */
#define SSH_VALID_FP(fp) (((fp) != NULL) && SSH_FP_GET_PC(fp) != NULL)
#endif /* __OpenBSD__ */
#endif /* __linux__ */
#endif /* HAVE_LIBC_STACK_END */

/**  We have SSH_FP functionality under this compiler/platform combination. */
#define SSH_FP_OK

#endif /* __GNUC__ && __i386__ */

/* ********************** sparc, gcc/Sunpro C ***********************/

#if defined(__sparc)
#include <sys/trap.h>

/**  We use auxillary function ssh_get_fp, since we want the system to
     work with the native compiler, which doesn't have the flexibility
     of the GCC asm() directive. */

#ifdef __GNUC__
static void *ssh_get_fp()
{
  void *fp;
  asm volatile ("ta %0" : /* no outputs */ : "i" (ST_FLUSH_WINDOWS));
  asm volatile ("mov %%fp, %0" : "=r" (fp));
  return fp;
}
#endif /* __sun && __GNUC__ */

#ifdef __SUNPRO_C
static void* ssh_get_fp()
{
  /**  On sparcv9 we can use flushw, otherwise we have to perform trap 3
       (ST_FLUSH_WINDOWS) to force the register windows to be written on
       the stack. */
#ifdef __sparcv9
  __asm("flushw\n"
        "mov %fp, %i0\n"
        "ret\n"
        "restore\n");
#else /* !__sparcv9 */
#define XVALUE(X)       #X
#define VALUE(X)        XVALUE(X)
  __asm("ta " VALUE(ST_FLUSH_WINDOWS) "\n"
        "nop\n"
        "mov %fp, %i0\n");
#undef XVALUE
#undef VALUE
#endif /* __sparcv9 */
}
#endif /* __SUNPRO_C */

#define SSH_GET_FP(fp)                                                  \
do {                                                                    \
  /**  Try to avoid inlining the function (with high optimization flags,\
       since inlining will both out the depth correction below.*/       \
  (fp) = (*ssh_get_fp)();                                               \
  (fp) = SSH_FP_BIAS(fp);                                               \
                                                                        \
  /**  Remember to discount the effect of the ssh_get_fp() routine on   \
       the call stack by returning one level higher */                  \
  if (SSH_VALID_FP(fp) && SSH_VALID_FP(SSH_FP_GET_FP(fp)))              \
    {                                                                   \
      (fp) = SSH_FP_GET_FP(fp);                                         \
      (fp) = SSH_FP_BIAS(fp);                                           \
    }                                                                   \
} while (0)

/**  This will take the bias offset of 2047 bytes on sparcv9 under
     64-bit mode and unbias the fp */
#define SSH_FP_BIAS(fp) ((void *) (((unsigned long) (fp) & 1) ? \
        (((unsigned long) fp) + 2047) : (unsigned long) (fp)))

#define SSH_VALID_FP(fp) (fp != (void*)0)
#define SSH_FP_GET_FP(fp) SSH_FP_BIAS((((void **) fp)[14]))
#define SSH_FP_GET_PC(fp) (((void **) fp)[15])
#define SSH_FP_OK

#endif /* __sparc */


/* ********************* Power PC and GCC ***************************/

#if defined(__GNUC__) && defined(__PPC__)

/*  Get the currently active frame-pointer value to 'fp'. */
#define SSH_GET_FP(fp)                          \
do                                              \
{                                               \
  register void *_fp;                           \
  asm volatile ("" : "=l"(_fp));                \
  asm volatile ("lwz %0,0(1)" : "=r"(_fp));     \
  (fp) = _fp;                                   \
}                                               \
while (0)

/*   A predicate to check whether the frame-pointer 'fp' is valid. */
#define SSH_VALID_FP(fp) ((fp) != NULL)

/*   Get frame-pointer from the frame, pointed by 'fp'. */
#define SSH_FP_GET_FP(fp) (*((void **) (fp)))

/*   Get program counter from the frame, pointed by 'fp'.  */
#define SSH_FP_GET_PC(fp) (*((void **) (((unsigned char *) fp) + 4)))

/*   We have SSH_FP functionality under this compiler/platform combination. */
#define SSH_FP_OK

#endif /* __GNUC__ && __PPC__ */

/* ********************* 32 bit Windows architectures ***************/

#if defined(WINDOWS) && defined(WIN32) && !defined(_WIN64)
#include "sshwindebug.h"

/*  Get the currently active frame-pointer value to 'fp'. */
#define SSH_GET_FP(fp)                          \
do                                              \
{                                               \
  __asm mov dword ptr fp, ebp                   \
}                                               \
while (0)

/*   A predicate to check whether the frame-pointer 'fp' is valid. */
#define SSH_VALID_FP(fp) (IsBadReadPtr((fp), 2 * sizeof(void *)) == FALSE)

/*   Get frame-pointer from the frame, pointed by 'fp'. */
#define SSH_FP_GET_FP(fp) (*((void **) (fp)))

/*   Get program counter from the frame, pointed by 'fp'.  */
#define SSH_FP_GET_PC(fp) (*((void **) (((unsigned char *) fp) + 4)))

/*   We have SSH_FP functionality under this compiler/platform combination. */
#define SSH_FP_OK
#endif /* WINDOWS && WIN32 */

#if defined(__GNUC__) && defined(__amd64__) && !defined(__STRICT_ANSI__)

/*  Get the currently active frame-pointer value to 'fp'. */
#define SSH_GET_FP(fp)                          \
do                                              \
{                                               \
  register void *_fp;                           \
  asm volatile ("movq %%rbp, %0" : "=r" (_fp)); \
  (fp) = _fp;                                   \
}                                               \
while (0)

/*   Get frame-pointer from the frame, pointed by 'fp'. */
#define SSH_FP_GET_FP(fp) (*((void **) (fp)))

/*   Get program counter from the frame, pointed by 'fp'.  */
#define SSH_FP_GET_PC(fp) (*((void **) (((unsigned char *) fp) + 8)))

#ifdef HAVE_LIBC_STACK_END
extern void *__libc_stack_end;
/*   Skip 2 last frames. */
#define SSH_VALID_FP(fp) (((fp) != NULL) && SSH_FP_GET_PC(fp) != NULL \
  && (SSH_FP_GET_FP(fp) != NULL && SSH_FP_GET_FP(fp) > (fp) && \
      SSH_FP_GET_FP(fp) < __libc_stack_end) \
  && (SSH_FP_GET_FP(SSH_FP_GET_FP(fp)) != NULL && \
      SSH_FP_GET_FP(SSH_FP_GET_FP(fp)) > (fp) && \
      SSH_FP_GET_FP(SSH_FP_GET_FP(fp)) < __libc_stack_end))
#else /* HAVE_LIBC_STACK_END */
#ifdef __linux__
/*   A predicate to check whether the frame-pointer 'fp' is valid.
     For Linux 2.6 we use a heuristic, we also require that
     the next frame pointer on the stack is not also contained
     in *(fp+3). This is the case in the main() frame. */
#define SSH_VALID_FP(fp) \
(((fp) != NULL) && SSH_FP_GET_PC(fp) != NULL \
&& (*(((void **)fp) + 2)) != SSH_FP_GET_FP(fp))
#else /* __linux__ */
#define SSH_VALID_FP(fp) (((fp) != NULL) && SSH_FP_GET_PC(fp) != NULL)
#endif /* __linux__ */
#endif /* HAVE_LIBC_STACK_END */

/*   We have SSH_FP functionality under this compiler/platform combination. */
#define SSH_FP_OK

#endif /* __GNUC__ && __amd64__ */
