/**
   
   @copyright
   Copyright (c) 2002 - 2013, Rambus Inc. All rights reserved.
   
   sshmallocdebug.c
   
   Memory leak check and various failure memory allocation routines.
   
   Instructions how to use the by-stack failure routines:
   
     Define at least SSH_ALLOC_FAIL_PREFIX before running
     the program:
   
       $ export SSH_ALLOC_FAIL_PREFIX=/l/stack
   
     If you have run the stack-fault-instrumented binary before,
     remember to clear temporary files:
   
       $ rm -f $SSH_ALLOC_FAIL_PREFIX*
   
     Now, run the program:
   
       $ ./foobin
   
     And run it again. And again, and again. During each iteration the
     memory allocation routines will fail at different locations. See
     * ${SSH_ALLOC_FAIL_PREFIX}.log file for actual traces
     of * the failing locations. You can see the path using a
     debugger, for * instance (gdb: disassemble <PC>).
   
     BTW: If you run "stack-alloc fault enabled" program under
     insure/purify/etc. it *will* report memory and file descriptor *
     leaks. These are inevitable, since there is no point where we
     ever * deallocate and/or close the ndbm/gdbm resources or log files.
   
   Instructions how to use the by-count failure routines:
   
     Define SSH_ALLOC_FAIL_COUNT environment variable before running
     the program:
   
       $ export SSH_ALLOC_FAIL_COUNT=100
   
     Now, run the program:
   
       $ ./foobin
   
     The first SSH_ALLOC_FAIL_COUNT allocations will be successful
     after which the alloc routines start to fail.  Check that your
     program handles the error cases successfully, increase the limit
     and rerun.
   
   Instructions how to use the by-random failure routines:
   
     Define SSH_ALLOC_FAIL_RANDOM environment variable to some
     32-bit unsigned integer value:
   
       $ export SSH_ALLOC_FAIL_RANDOM=0xfeffffff
   
     Now, run the program
   
       $ ./foobin
   
     The memory allocation routines will fail randomly.  If the value,
     returned by ssh_rand() is bigger that your limit, the allocation
     fails.
   
     You can set the seed for the random function by setting the
     environment variable SSH_ALLOC_FAIL_RANDOM_SEED to a seed value:
   
       $ export SSH_ALLOC_FAIL_RANDOM_SEED=42
   
    You can also set the SSH_ALLOC_FAIL_RANDOM_SUCCESS environment
    variable which describes how many initial allocations are
    successful.  After the system has performed that many allocations,
    it starts failing them randomly.
   
*/


#include "sshincludes.h"
#include "sshdebug.h"
#include "sshmatch.h"

#undef SSH_ASSERT
#define SSH_ASSERT(cond) if (!(cond)) *(char*)0xffffffff=1;

#ifdef SSH_MEMORY_DEBUG_FREE_QUEUE
#include "sshsimplehashtable.h"

#ifdef VXWORKS
#include "taskLib.h"
#undef SSH_GLOBALS_EMULATION
#define ML_GET_CTX_ID() taskIdSelf()
typedef long SshCtxIdType;
#else
#define ML_GET_CTX_ID() 0
typedef long SshCtxIdType;
#endif /* VXWORKS */

#define SSH_ML_FREE_QUEUE
#define SSH_ML_FREE_QUEUE_LEN 100
#endif /* SSH_MEMORY_DEBUG_FREE_QUEUE */

#ifdef MEMORY_LEAK_CHECKS

/* Stack trace is enabled for debugs if FP functionality is
   present. But have ifdef here anyway in case someone goes and
   manually undefs it in sshconf.h.*/
#ifdef MEMORY_LEAK_STACK_TRACE
#include "sshfp.h"
#ifndef SSH_FP_OK
#undef MEMORY_LEAK_STACK_TRACE
#endif /* SSH_FP_OK */
#endif /* MEMORY_LEAK_STACK_TRACE */

#ifdef MEMORY_LEAK_STACK_TRACE
#ifdef WINDOWS
#include "sshwindebug.h"
#endif /* WINDOWS */
#endif /* MEMORY_LEAK_STACK_TRACE */

/* Functionality provided by this file is allowed in nettask. */
#define SSH_GLOBAL_FLAGS SSH_GLOBAL_F_VXWORKS_ALLOW_NETTASK
#include "sshglobals.h"

/************************** Types and definitions ***************************/

#ifdef VXWORKS
#include "memLib.h"
#else /* VXWORKS */
#include <stdlib.h>
#endif /* VXWORKS */
#include "string.h"
/* Undefine the macro versions. */
#undef ssh_xmalloc
#undef ssh_xrealloc
#undef ssh_xcalloc
#undef ssh_xstrdup
#undef ssh_xmemdup
#undef ssh_xfree

#undef ssh_malloc
#undef ssh_realloc
#undef ssh_calloc
#undef ssh_strdup
#undef ssh_memdup
#undef ssh_free

#ifndef VXWORKS




void *ssh_malloc(size_t size);
void *ssh_realloc(void *ptr, size_t old_size, size_t new_size);
void *ssh_calloc(size_t nitems, size_t size);
void *ssh_strdup(const void *p);
void *ssh_memdup(const void *p, size_t len);
void ssh_free(void *ptr);
#else
#undef ssh_malloc
#undef ssh_calloc
#undef ssh_free
#undef ssh_strdup
#undef ssh_memdup
#undef ssh_realloc
#undef malloc
#undef free
#undef calloc
#undef strdup
#undef memdup
#undef realloc
#define ssh_malloc malloc
#define ssh_calloc calloc
#define ssh_free free
#define ssh_strdup strdup
#define ssh_memdup memdup
#define ssh_realloc realloc

#endif /* VXWORKS */

/* Simple mutex implementation, which is needed to protect some global
   variables */
#ifdef WIN32
CRITICAL_SECTION ssh_md_mutex;
Boolean ssh_md_mutex_initialized = FALSE;
#define SSH_MD_MUTEX_INITIALIZE(m)              \
do                                              \
  {                                             \
    if (!ssh_md_mutex_initialized)              \
      {                                         \
        InitializeCriticalSection(&m);          \
        ssh_md_mutex_initialized = TRUE;        \
      }                                         \
  }                                             \
while (0)

#define SSH_MEMORY_LEAK_CHECK_LOCK()            \
do                                              \
  {                                             \
    SSH_MD_MUTEX_INITIALIZE(ssh_md_mutex);      \
    EnterCriticalSection(&ssh_md_mutex);        \
  }                                             \
while (0)

#define SSH_MEMORY_LEAK_CHECK_UNLOCK()          \
do                                              \
  {                                             \
    LeaveCriticalSection(&ssh_md_mutex);        \
  }                                             \
while (0)

#else /* not WIN32 */
#ifdef HAVE_PTHREADS
#include <pthread.h>
pthread_mutex_t ssh_md_mutex = PTHREAD_MUTEX_INITIALIZER;
#define SSH_MEMORY_LEAK_CHECK_LOCK()            \
  pthread_mutex_lock(&ssh_md_mutex)

#define SSH_MEMORY_LEAK_CHECK_UNLOCK()          \
  pthread_mutex_unlock(&ssh_md_mutex)
#else /* HAVE_THREADS */



/* No special locking needed. */
#define SSH_MEMORY_LEAK_CHECK_LOCK()
#define SSH_MEMORY_LEAK_CHECK_UNLOCK()
#endif /* HAVE_THREADS */
#endif /* WIN32 */

#ifdef VXWORKS
#undef SSH_MEMORY_LEAK_CHECK_LOCK
#undef SSH_MEMORY_LEAK_CHECK_UNLOCK
typedef SEMAPHORE *SEMAPHOREPtr;
SSH_GLOBAL_DECLARE(SEMAPHOREPtr, ssh_md_mutex);
SSH_GLOBAL_DEFINE_INIT(SEMAPHOREPtr, ssh_md_mutex) = NULL;
#define ssh_md_mutex SSH_GLOBAL_USE_INIT(ssh_md_mutex)
SSH_GLOBAL_DECLARE(Boolean, ssh_md_mutex_initialized);
SSH_GLOBAL_DEFINE_INIT(Boolean, ssh_md_mutex_initialized) = FALSE;
#define ssh_md_mutex_initialized SSH_GLOBAL_USE_INIT(ssh_md_mutex_initialized)

#define SSH_MD_MUTEX_INITIALIZE(m)                \
do                                                \
  {                                               \
    if (!ssh_md_mutex_initialized)                \
      {                                           \
        m = semBCreate(SEM_Q_PRIORITY, SEM_FULL); \
        ssh_md_mutex_initialized = TRUE;          \
      }                                           \
  }                                               \
while (0)

int semline; int semtask;

#define SSH_MEMORY_LEAK_CHECK_LOCK()              \
do                                                \
  {                                               \
    SSH_MD_MUTEX_INITIALIZE(ssh_md_mutex);        \
    semTake(ssh_md_mutex, WAIT_FOREVER);          \
    semline = __LINE__; semtask = taskIdSelf();   \
  }                                               \
while (0)

#define SSH_MEMORY_LEAK_CHECK_UNLOCK()            \
do                                                \
  {                                               \
    semline = 0; semtask = 0;                     \
    semGive(ssh_md_mutex);                        \
  }                                               \
while (0)
#endif /* VXWORKS */

/***************** Determine if we have a DB lib ************************/
#ifdef SSH_ALLOC_FAIL
/* Determine DBM type, give preference to GDBM */
#ifdef HAVE_GDBM_OPEN
# ifdef HAVE_GDBM_H
#  include <gdbm.h>
#  define USE_GDBM
# endif /* HAVE_GDBM_H */
#else /* !HAVE_GDBM_OPEN */
# ifdef HAVE_DBM_OPEN
#  ifdef HAVE_NDBM_H
#   include <ndbm.h>
#   define USE_NDBM
#  else
#   ifdef HAVE_DB1_NDBM_H
#    include <db1/ndbm.h>
#   endif /* HAVE_DB1_NDBM_H */
#  endif /* HAVE_NDBM_H */
# define USE_NDBM
# endif /* HAVE_DBM_OPEN */
#endif /* HAVE_GDBM_OPEN */
#endif /* SSH_ALLOC_FAIL */

#if defined(USE_GDBM) || defined(USE_NDBM)
#define HAVE_DBM
#endif /* defined(USE_GDBM) || defined(USE_NDBM) */

#include "sshrand.h"

#if defined(MEMORY_LEAK_STACK_TRACE) && defined(HAVE_DBM)

#define MAX_STACK_DEPTH 512


typedef struct SshStackAllocSkipStruct
{
  void *start, *end;
} *SshStackAllocSkip, SshStackAllocSkipStruct;

SSH_GLOBAL_DECLARE(Boolean, ssh_fp_db_init);
SSH_GLOBAL_DEFINE_INIT(Boolean, ssh_fp_db_init) = FALSE;
#define ssh_fp_db_init SSH_GLOBAL_USE_INIT(ssh_fp_db_init)
typedef FILE *FILEPtr;
SSH_GLOBAL_DECLARE(FILEPtr, ssh_fp_db_log);
SSH_GLOBAL_DEFINE_INIT(FILEPtr, ssh_fp_db_log) = NULL;
#define ssh_fp_db_log SSH_GLOBAL_USE_INIT(ssh_fp_db_log)
SSH_GLOBAL_DECLARE(FILEPtr, ssh_fp_db_blog);
SSH_GLOBAL_DEFINE_INIT(FILEPtr, ssh_fp_db_blog) = NULL;
#define ssh_fp_db_blog SSH_GLOBAL_USE_INIT(ssh_fp_db_blog)
SSH_GLOBAL_DECLARE(FILEPtr, ssh_fp_db_count);
SSH_GLOBAL_DEFINE_INIT(FILEPtr, ssh_fp_db_count) = NULL;
#define ssh_fp_db_count SSH_GLOBAL_USE_INIT(ssh_fp_db_count)
#ifdef USE_GDBM
SSH_GLOBAL_DECLARE(GDBM_FILE, ssh_fp_db);
SSH_GLOBAL_DEFINE_INIT(GDBM_FILE, ssh_fp_db) = NULL;
#define ssh_fp_db SSH_GLOBAL_USE_INIT(ssh_fp_db)
#endif
#ifdef USE_NDBM
typedef DBM *DBMPtr;
SSH_GLOBAL_DECLARE(DBMPtr, ssh_fp_db);
SSH_GLOBAL_DEFINE_INIT(DBMPtr, ssh_fp_db) = NULL;
#define ssh_fp_db SSH_GLOBAL_USE_INIT(ssh_fp_db)
#endif /* USE_NDBM */
typedef void *SshFpPcsArray[MAX_STACK_DEPTH];
SSH_GLOBAL_DECLARE(SshFpPcsArray, ssh_fp_pcs);
SSH_GLOBAL_DEFINE_INIT(SshFpPcsArray, ssh_fp_pcs);
#define ssh_fp_pcs SSH_GLOBAL_USE_INIT(ssh_fp_pcs)
SSH_GLOBAL_DECLARE(SshUInt32, ssh_fp_skip);
SSH_GLOBAL_DEFINE_INIT(SshUInt32, ssh_fp_skip) = 0;
#define ssh_fp_skip SSH_GLOBAL_USE_INIT(ssh_fp_skip)
SSH_GLOBAL_DECLARE(SshStackAllocSkip, ssh_fp_pc_skips);
SSH_GLOBAL_DEFINE_INIT(SshStackAllocSkip, ssh_fp_pc_skips) = NULL;
#define ssh_fp_pc_skips SSH_GLOBAL_USE_INIT(ssh_fp_pc_skips)
SSH_GLOBAL_DECLARE(int, ssh_fp_pc_skips_count);
SSH_GLOBAL_DEFINE_INIT(int, ssh_fp_pc_skips_count) = 0;
#define ssh_fp_pc_skips_count SSH_GLOBAL_USE_INIT(ssh_fp_pc_skips_count)

static Boolean ssh_alloc_stack_fail(void)
{
  void *fp, *tmp;
  int depth;
  datum key, value;

  if (!ssh_fp_db_init)
    {
      char *env;
      Boolean created = FALSE;

      env = getenv("SSH_ALLOC_FAIL_PREFIX");

      /* Notice: we cannot use sshbuffer, ssh_dvsprintf or anything
         like that which would call allocation routines.. we use
         ssh_malloc directly, and bomb out with a fatal error if
         *that* fails */

      if (env)
        {
          int len;
          char *log;
          Boolean exists = FALSE;
          struct stat st;

          len = strlen(env) + 7;
          log = ssh_malloc(len);

          if (!log)
              ssh_fatal("Deep memory allocation failure in memory allocation "
                        "debug code initialization");

          memcpy(log, env, strlen(env));
          memcpy(log + strlen(env), ".log", 5);

          if (stat(log, &st) != -1)
            exists = TRUE;

          ssh_fp_db_log = fopen(log, "a+");

          if (!exists && ssh_fp_db_log != NULL)
            created = TRUE;

          memcpy(log, env, strlen(env));
          memcpy(log + strlen(env), ".blog", 6);

          ssh_fp_db_blog = fopen(log, "a+");

          memcpy(log, env, strlen(env));
          memcpy(log + strlen(env), ".count", 7);
          ssh_fp_db_count = fopen(log, "w");

          ssh_free(log);

#ifdef USE_GDBM
          ssh_fp_db = gdbm_open(env, 0, GDBM_WRCREAT, 0664, 0);
#endif
#ifdef USE_NDBM
          ssh_fp_db = dbm_open(env, O_RDWR|O_CREAT, 0664);
#endif

          if (!ssh_fp_db_blog || !ssh_fp_db || !ssh_fp_db_count)
            {
              if (ssh_fp_db_blog)
                {
                  fclose(ssh_fp_db_blog);
                  ssh_fp_db_blog = NULL;
                }

              if (ssh_fp_db)
                {
#ifdef USE_GDBM
                  gdbm_close(ssh_fp_db);
#endif /* USE_GDBM */
#ifdef USE_NDBM
                  dbm_close(ssh_fp_db);
#endif /* USE_NDBM */
                  ssh_fp_db = NULL;
                }

              if (ssh_fp_db_count)
                {
                  fclose(ssh_fp_db_count);
                  ssh_fp_db_count = NULL;
                }
            }
          else
            {
              int depth;
              void *addr;

              /* Replay blog into the main database .. since not on
                 all platform dbm_store is synchronous */

              depth = 0;

              fseek(ssh_fp_db_blog, 0, SEEK_SET);

              while (1)
                {
                  if (fread(&addr, sizeof(addr), 1, ssh_fp_db_blog) != 1)
                    {
                      if (!feof(ssh_fp_db_blog))
                        perror("fread");
                      break;
                    }

                  /*fprintf(stderr, "Replay #%d = %p\n", depth, addr);*/

                  if (depth < MAX_STACK_DEPTH)
                    ssh_fp_pcs[depth++] = addr;

                  if (addr != NULL)
                    continue;

                  if (depth >= MAX_STACK_DEPTH)
                    {
                      depth = 0;
                      continue;
                    }

                  key.dptr = (void*) ssh_fp_pcs;
                  key.dsize = sizeof(*ssh_fp_pcs) * depth;

#ifdef USE_GDBM
                  value = gdbm_fetch(ssh_fp_db, key);
#endif /* USE_GDBM */
#ifdef USE_NDBM
                  value = dbm_fetch(ssh_fp_db, key);
#endif /* USE_NDBM */

                  if (!value.dptr)
                    {
                      /* fprintf(stderr, "Replay event not found,
                         adding.\n"); */

                      value.dptr = "\x01";
                      value.dsize = 1;

#ifdef USE_GDBM
                      gdbm_store(ssh_fp_db, key, value, GDBM_INSERT);
#endif /* USE_GDBM */
#ifdef USE_NDBM
                      dbm_store(ssh_fp_db, key, value, DBM_INSERT);
#endif /* USE_NDBM */
                    }

                  depth = 0;
                }

#ifdef USE_GDBM
              gdbm_close(ssh_fp_db);
#endif /* USE_GDBM */
#ifdef USE_NDBM
              dbm_close(ssh_fp_db);
#endif /* USE_NDBM */

#ifdef USE_GDBM
              ssh_fp_db = gdbm_open(env, 0, GDBM_WRCREAT, 0664, 0);
#endif /* USE_GDBM */
#ifdef USE_NDBM
              ssh_fp_db = dbm_open(env, O_RDWR|O_CREAT, 0664);
#endif /* USE_NDBM */

              /* This should never happen */
              if (ssh_fp_db_blog == NULL)
                fprintf(stderr, "*** ssh_fp_db_blog == NULL ***\n");

              ftruncate(fileno(ssh_fp_db_blog), 0);
              fseek(ssh_fp_db_blog, 0, SEEK_END);
            }
        }

      env = getenv("SSH_ALLOC_FAIL_STACK_SKIP_FIRST");

      if (env && created)
        ssh_fp_skip = (SshUInt32) strtol(env, NULL, 0);

      env = getenv("SSH_ALLOC_FAIL_STACK_SKIP_PC");

      if (env)
        {
          char *elem, *tail;
          unsigned long start, end;
          int count, i;

          for (count = 0, elem = env; elem; elem = strchr(elem + 1, ','))
            count++;

          ssh_fp_pc_skips_count = count;
          ssh_fp_pc_skips = ssh_malloc(sizeof(*ssh_fp_pc_skips) * count);

          for (elem = strtok(env, ","), i = 0;
               elem;
               elem = strtok(NULL, ","), i++)
            {
              /* Start parsing first integer. */
              start = strtoul(elem, &tail, 0);

              /* Range? */
              if (*tail == '-')
                end = strtoul(tail, NULL, 0);
              /* Plus base? */
              else if (*tail == '+')
                end = start + strtoul(tail, NULL, 0);
              /* End of string? */
              else if (*tail == '\0')
                end = start;
              /* Invalid? */
              else
                /* don't use 0, since most stacks have pc=0 at
                   the bottom frame */
                start = end = ~0UL;

              ssh_fp_pc_skips[i].start = (void*) start;
              ssh_fp_pc_skips[i].end = (void*) end;

#if 0
              fprintf(stderr, "%d %p-%p\n",
                      i, ssh_fp_pc_skips[i].start, ssh_fp_pc_skips[i].end);
#endif
            }
        }

      ssh_fp_db_init = TRUE;
    }

  if (ssh_fp_db)
    {
      Boolean skip;
      int i, k;

      /* get fp, and skip the ssh_kmalloc part, since we're always
         called from there */
      SSH_GET_FP(fp);

      if (fp)
        fp = SSH_FP_GET_FP(fp);

      /* Go once through the stack to get the size required for the
         key material. */

      for (depth = 0, tmp = fp; SSH_VALID_FP(tmp); tmp = SSH_FP_GET_FP(tmp))
        ;

      if (depth >= MAX_STACK_DEPTH)
        return FALSE;

      for (depth = 0, tmp = fp; SSH_VALID_FP(tmp); tmp = SSH_FP_GET_FP(tmp))
        ssh_fp_pcs[depth++] = SSH_FP_GET_PC(tmp);

      /* Ensure that the last pc is NULL */
      if ((depth == 0 || ssh_fp_pcs[depth - 1] != NULL) &&
          depth < MAX_STACK_DEPTH)
        ssh_fp_pcs[depth++] = NULL;

      key.dptr = (void*) ssh_fp_pcs;
      key.dsize = sizeof(*ssh_fp_pcs) * depth;

#ifdef USE_GDBM
      value = gdbm_fetch(ssh_fp_db, key);
#endif /* USE_GDBM */
#ifdef USE_NDBM
      value = dbm_fetch(ssh_fp_db, key);
#endif /* USE_NDBM */

      if (value.dptr)
        return FALSE;

      for (k = 0, skip = FALSE; k < depth; k++)
        for (i = 0; i < ssh_fp_pc_skips_count; i++)
          {
            if (ssh_fp_pc_skips[i].start >= ssh_fp_pcs[k] &&
                ssh_fp_pc_skips[i].end <= ssh_fp_pcs[k])
              skip = TRUE;
          }

      value.dptr = "\x01";
      value.dsize = 1;

#ifdef USE_GDBM
      gdbm_store(ssh_fp_db, key, value, GDBM_INSERT);
#endif /* USE_GDBM */
#ifdef USE_NDBM
      dbm_store(ssh_fp_db, key, value, DBM_INSERT);
#endif /* USE_NDBM */

      if (ssh_fp_db_blog)
        {
          fwrite(key.dptr, key.dsize, 1, ssh_fp_db_blog);
          fflush(ssh_fp_db_blog);
        }

      if (ssh_fp_db_log)
        {
          int i;

          for (i = 0; i < depth; i++)
            fprintf(ssh_fp_db_log, "%p\n", ssh_fp_pcs[i]);

          fflush(ssh_fp_db_log);
        }

      if (skip)
        return FALSE;

      if (ssh_fp_skip > 0)
        {
          ssh_fp_skip--;
          return FALSE;
        }

      return TRUE;
    }

  return FALSE;
}

#else /* not (MEMORY_LEAK_STACK_TRACE && HAVE_DBM) */

static Boolean ssh_fp_db_init = FALSE;

static Boolean ssh_alloc_stack_fail(void)
{
  char *env;

  if (!ssh_fp_db_init)
    {
      ssh_fp_db_init = TRUE;

      env = getenv("SSH_ALLOC_FAIL_PREFIX");

      if (env)
        {
          fprintf(stderr, "WARNING: Alloc stack failer not available.\n");
        }
    }

  return FALSE;
}

#endif /* not (MEMORY_LEAK_STACK_TRACE && HAVE_DBM) */

SSH_GLOBAL_DECLARE(Boolean, ssh_alloc_count_init);
SSH_GLOBAL_DEFINE_INIT(Boolean, ssh_alloc_count_init) = FALSE;
#define ssh_alloc_count_init SSH_GLOBAL_USE_INIT(ssh_alloc_count_init)
SSH_GLOBAL_DECLARE(SshUInt32, ssh_alloc_count_limit);
SSH_GLOBAL_DEFINE_INIT(SshUInt32, ssh_alloc_count_limit) = 0;
#define ssh_alloc_count_limit SSH_GLOBAL_USE_INIT(ssh_alloc_count_limit)
SSH_GLOBAL_DECLARE(SshUInt32, ssh_alloc_count);
SSH_GLOBAL_DEFINE_INIT(SshUInt32, ssh_alloc_count) = 0;
#define ssh_alloc_count SSH_GLOBAL_USE_INIT(ssh_alloc_count)
	
/* Data structure for file-specific alloc fail settings.. */
typedef struct SshAllocFailFilesRec {
  /* Pointer to the next files-specific level setting. */
  struct SshAllocFailFilesRec *next;

  /* Pattern specifying the files for which this level is used.  The
     pattern may contain '*' and '?' as special characters.  It is allocated
     using ssh_xfree. */
  char *file;
  Boolean enabled;
} *SshAllocFailFiles;

static Boolean alloc_fail_initialized = FALSE;
static SshAllocFailFiles ssh_alloc_fail_files = NULL;

void 
ssh_alloc_fail_set_file(const char *file, Boolean enabled)
{
  SshAllocFailFiles am;

  /* Memory allocation failure on debug is ignored. */
  if (!(am = ssh_malloc_i(sizeof(*am), FALSE, "", 0)))
    return;

  if (!(am->file = ssh_strdup_i(file, FALSE, "", 0)))
    {
      ssh_free_i(am, "", 0);
      return;
    }

  am->next = ssh_alloc_fail_files;
  am->enabled = enabled;
  ssh_alloc_fail_files = am;
}

void 
ssh_alloc_fail_files_free(void)
{
  SshAllocFailFiles am;
  
  /* Clear (free) any per-file settings. */
  while (ssh_alloc_fail_files)
    {
      am = ssh_alloc_fail_files;
      ssh_alloc_fail_files = am->next;
      ssh_free_i(am->file, "", 0);
      ssh_free_i(am, "", 0);
    }
}

void ssh_alloc_fail_set_filter_string(const char *string)
{
  const char *name_start, *name_end;
  char *name;
  size_t name_len;
  Boolean error;
  long enabled;

#ifndef DEBUG_LIGHT
  {
    static int warned = 0;
    if (!warned)
      {



        warned = 1;
      }
  }
#endif /* !DEBUG_LIGHT */

  while (*string)
    {
      error = FALSE;

      /* Skip whitespace */
      while (*string && isspace(*string))
        string++;

      /* Parse name */
      name_start = string;
      while (*string && !isspace(*string) && *string != '=' && *string != ',')
        string++;
      name_end = string;
      name_len = name_end - name_start;

      /* Skip whitespace */
      while (*string && isspace(*string))
        string++;

      enabled = 1;
      if (*string == '=')
        {
          string++;
          /* Skip whitespace */
          while (*string && isspace(*string))
            string++;

          if (isdigit(*string))
            {
              enabled = atoi(string);
              for (; *string && isdigit(*string); string++)
                ;
            }
          else
            {
              ssh_warning("ssh_file_set_enabled: Invalid numeric "
                          "argument for %s", name_start);
              error = TRUE;
            }

          /* Skip whitespace */
          while (*string && isspace(*string))
            string++;
        }

      if (*string)
        {
          if (*string != ',')
            {
              if (!error)
                ssh_warning("ssh_alloc_fail_set_filter_string: Ignored"
			    " junk after "
                            "command : %s", string);
              while (*string && *string != ',')
                string++;
            }
          else
            {
              string++;
            }
        }

      name = ssh_malloc_i(name_len + 1, FALSE, "", 0);
      if (name)
	{
	  memcpy(name, name_start, name_len);
	  name[name_len] = '\0';
	  ssh_alloc_fail_set_file(name, enabled > 0 ? TRUE : FALSE);
	  ssh_free_i(name, "", 0);
	}
    }
}

Boolean 
ssh_alloc_fail_enabled(const char *file)
{
  SshAllocFailFiles am;

  for (am = ssh_alloc_fail_files; am; am = am->next)
    {
      if (ssh_match_pattern(file, am->file))
	return am->enabled;
    }

  return FALSE;
}

static Boolean ssh_alloc_file_can_fail(const char *file)
{
  char *alloc_fail_str;
  char default_str[] = "*=1";

  if (alloc_fail_initialized == FALSE)
    {
      alloc_fail_str = (char *)getenv("SSH_ALLOC_FAIL_STR");
      if (alloc_fail_str == NULL)
	ssh_alloc_fail_set_filter_string(default_str);
      else
	ssh_alloc_fail_set_filter_string(alloc_fail_str);

      alloc_fail_initialized = TRUE;
    }

  return ssh_alloc_fail_enabled(file);
}

static Boolean ssh_alloc_count_fail(void)
{
#if defined(MEMORY_LEAK_STACK_TRACE) && defined(HAVE_DBM)
  char count_value[32];
#endif /* not (MEMORY_LEAK_STACK_TRACE && HAVE_DBM) */

  if (!ssh_alloc_count_init)
    {
      char *env = getenv("SSH_ALLOC_FAIL_COUNT");

      if (env)
        ssh_alloc_count_limit = strtoul(env, NULL, 0);

      ssh_alloc_count_init = TRUE;
    }

#if defined(MEMORY_LEAK_STACK_TRACE) && defined(HAVE_DBM)
  if (ssh_fp_db_count)
    {
      ssh_snprintf(count_value, sizeof(count_value), "%d",
		   (int) ssh_alloc_count);
      rewind(ssh_fp_db_count);
      fwrite(count_value, strlen(count_value), 1, ssh_fp_db_count);
      fflush(ssh_fp_db_count);
    }
#endif /* not (MEMORY_LEAK_STACK_TRACE && HAVE_DBM) */

  if (++ssh_alloc_count > ssh_alloc_count_limit)
    return TRUE;
  
  return FALSE;
}

SSH_GLOBAL_DECLARE(Boolean, ssh_alloc_random_init);
SSH_GLOBAL_DEFINE_INIT(Boolean, ssh_alloc_random_init) = FALSE;
#define ssh_alloc_random_init SSH_GLOBAL_USE_INIT(ssh_alloc_random_init)
SSH_GLOBAL_DECLARE(SshUInt32, ssh_alloc_random_limit);
SSH_GLOBAL_DEFINE_INIT(SshUInt32, ssh_alloc_random_limit) = 0;
#define ssh_alloc_random_limit SSH_GLOBAL_USE_INIT(ssh_alloc_random_limit)
SSH_GLOBAL_DECLARE(SshUInt32, ssh_alloc_random_success_limit);
SSH_GLOBAL_DEFINE_INIT(SshUInt32, ssh_alloc_random_success_limit) = 0;
#define ssh_alloc_random_success_limit \
  SSH_GLOBAL_USE_INIT(ssh_alloc_random_success_limit)
SSH_GLOBAL_DECLARE(SshUInt32, ssh_alloc_random_count);
SSH_GLOBAL_DEFINE_INIT(SshUInt32, ssh_alloc_random_count) = 0;
#define ssh_alloc_random_count SSH_GLOBAL_USE_INIT(ssh_alloc_random_count)

static Boolean ssh_alloc_random_fail(void)
{
  if (!ssh_alloc_random_init)
    {
      char *env = getenv("SSH_ALLOC_FAIL_RANDOM");

      /* Set the limit. */
      if (env)
        ssh_alloc_random_limit = strtoul(env, NULL, 0);

      /* Seed. */
      env = getenv("SSH_ALLOC_FAIL_RANDOM_SEED");
      if (env)
        ssh_rand_seed(atoi(env));

      /* Number of successful allocations. */
      env = getenv("SSH_ALLOC_FAIL_RANDOM_SUCCESS");
      if (env)
        ssh_alloc_random_success_limit = strtoul(env, NULL, 0);
      
      ssh_alloc_random_init = TRUE;
    }
  
  if (ssh_alloc_random_limit
      && ++ssh_alloc_random_count > ssh_alloc_random_success_limit
      && ssh_rand() > ssh_alloc_random_limit)
    return TRUE;

  return FALSE;
}

/* A predicate to check whether the current allocation should fail.
   If it should fail, the function returns NULL, otherwise it does
   nothing. */
#define MAYBE_FAIL_ALLOC(_file)					   \
do                                                                 \
  {                                                                \
    if ((ssh_alloc_count_fail() || ssh_alloc_random_fail()) &&     \
	ssh_alloc_stack_fail()					   \
	&& ssh_alloc_file_can_fail((_file)))			   \
      return NULL;                                                 \
  }                                                                \
while (0)


/* An active memory block. */
struct SshMemDebugHdrRec
{
#ifdef SSH_MEMORY_DEBUG_FREE_QUEUE
  SshDlNodeStruct node;
#else /* SSH_MEMORY_DEBUG_FREE_QUEUE */
  struct SshMemDebugHdrRec *next;
  struct SshMemDebugHdrRec *prev;
#endif /* SSH_MEMORY_DEBUG_FREE_QUEUE */

  /* Flags. */
  unsigned int seen : 1;
  unsigned int marked : 1;
  unsigned int freed : 1;

  /* The allocation location. */
  const char *file;
  int line;

#ifdef MEMORY_LEAK_STACK_TRACE
  /* Stack trace of the allocation location. */
  SshUInt32 stack_trace_depth;
  void **stack_trace;
#endif /* MEMORY_LEAK_STACK_TRACE */

#ifdef SSH_MEMORY_DEBUG_FREE_QUEUE
  /* Context identifier, eg. allocating thread identifier. */
  SshCtxIdType ctxid;
#endif /* SSH_MEMORY_DEBUG_FREE_QUEUE */

  /* The size of the block. */
  unsigned long size;
};

typedef struct SshMemDebugHdrRec *SshMemDebugHdr;
typedef struct SshMemDebugHdrRec SshMemDebugHdrStruct;

#define SSH_MEM_DEBUG_SIZE(size) ((size) + sizeof(SshMemDebugHdrStruct))

#define SSH_MEM_DEBUG_HDR(ptr)  \
  ((SshMemDebugHdr) (((unsigned char *) ptr) - sizeof(SshMemDebugHdrStruct)))

/***************************** Static variables *****************************/

#ifdef SSH_MEMORY_DEBUG_FREE_QUEUE

#define SSH_MEM_DEBUG_BLOCKS_HASH_SIZE 16384
typedef union
{
  void *size[SSH_SIMPLE_HASH_SIZE_POINTERS(SSH_MEM_DEBUG_BLOCKS_HASH_SIZE)];
  SshSimpleHashStruct hash;
} SshSimpleHashDebugBlockUnion;

SSH_GLOBAL_DECLARE(SshSimpleHashDebugBlockUnion, ssh_mem_debug_blocks);
SSH_GLOBAL_DEFINE_INIT(SshSimpleHashDebugBlockUnion, ssh_mem_debug_blocks) = 
{ { NULL } };
#define ssh_mem_debug_blocks SSH_GLOBAL_USE_INIT(ssh_mem_debug_blocks)

SSH_GLOBAL_DECLARE(Boolean, ssh_mem_debug_initialized);
SSH_GLOBAL_DEFINE_INIT(Boolean, ssh_mem_debug_initialized) = 0;
#define ssh_mem_debug_initialized \
  SSH_GLOBAL_USE_INIT(ssh_mem_debug_initialized)

typedef SshMemDebugHdrStruct *SshMemDebugHdrFreeList[SSH_ML_FREE_QUEUE_LEN];
SSH_GLOBAL_DECLARE(SshMemDebugHdrFreeList, ssh_mem_free_blocks);
SSH_GLOBAL_DEFINE_INIT(SshMemDebugHdrFreeList, ssh_mem_free_blocks) = { NULL };
#define ssh_mem_free_blocks SSH_GLOBAL_USE_INIT(ssh_mem_free_blocks)

SSH_GLOBAL_DECLARE(int, ssh_mem_free_block);
SSH_GLOBAL_DEFINE_INIT(int, ssh_mem_free_block) = 0;
#define ssh_mem_free_block SSH_GLOBAL_USE_INIT(ssh_mem_free_block)

#else /* SSH_MEMORY_DEBUG_FREE_QUEUE */

SSH_GLOBAL_DECLARE(SshMemDebugHdr, ssh_mem_debug_blocks);
SSH_GLOBAL_DEFINE_INIT(SshMemDebugHdr, ssh_mem_debug_blocks) = NULL;
#define ssh_mem_debug_blocks \
  SSH_GLOBAL_USE_INIT(ssh_mem_debug_blocks)

#endif /* SSH_MEMORY_DEBUG_FREE_QUEUE */

SSH_GLOBAL_DECLARE(SshUInt32, ssh_mem_debug_balance);
SSH_GLOBAL_DEFINE_INIT(SshUInt32, ssh_mem_debug_balance) = 0;
#define ssh_mem_debug_balance SSH_GLOBAL_USE_INIT(ssh_mem_debug_balance)
SSH_GLOBAL_DECLARE(SshUInt32, ssh_mem_debug_num_blocks);
SSH_GLOBAL_DEFINE_INIT(SshUInt32, ssh_mem_debug_num_blocks) = 0;
#define ssh_mem_debug_num_blocks SSH_GLOBAL_USE_INIT(ssh_mem_debug_num_blocks)
SSH_GLOBAL_DECLARE(SshUInt32, ssh_mem_debug_max_balance);
SSH_GLOBAL_DEFINE_INIT(SshUInt32, ssh_mem_debug_max_balance) = 0;
#define ssh_mem_debug_max_balance \
  SSH_GLOBAL_USE_INIT(ssh_mem_debug_max_balance)
SSH_GLOBAL_DECLARE(SshUInt32, ssh_mem_debug_max_num_blocks);
SSH_GLOBAL_DEFINE_INIT(SshUInt32, ssh_mem_debug_max_num_blocks) = 0;
#define ssh_mem_debug_max_num_blocks \
  SSH_GLOBAL_USE_INIT(ssh_mem_debug_max_num_blocks)

SSH_GLOBAL_DECLARE(SshUInt32, ssh_mem_debug_flags);
SSH_GLOBAL_DEFINE_INIT(SshUInt32, ssh_mem_debug_flags) = 0;
#define ssh_mem_debug_flags SSH_GLOBAL_USE_INIT(ssh_mem_debug_flags)

/************************** Stack trace extraction **************************/

#ifdef MEMORY_LEAK_STACK_TRACE

typedef void **voidPtrPtr;
SSH_GLOBAL_DECLARE(SshUInt32, ssh_mem_debug_stack_trace_depth);
SSH_GLOBAL_DEFINE_INIT(SshUInt32, ssh_mem_debug_stack_trace_depth) = 100;
#define ssh_mem_debug_stack_trace_depth \
  SSH_GLOBAL_USE_INIT(ssh_mem_debug_stack_trace_depth)
SSH_GLOBAL_DECLARE(voidPtrPtr, ssh_mem_debug_stack_trace);
SSH_GLOBAL_DEFINE_INIT(voidPtrPtr, ssh_mem_debug_stack_trace) = NULL;
#define ssh_mem_debug_stack_trace \
  SSH_GLOBAL_USE_INIT(ssh_mem_debug_stack_trace)

static void
ssh_malloc_set_stack_trace(SshMemDebugHdr hdr)
{
  void *fp;
  SshUInt32 stack_trace_depth;
  if (ssh_mem_debug_stack_trace == NULL)
    {
      char *env = getenv("SSH_ALLOC_STACK_TRACE_DEPTH");
      SshUInt32 num_frames;

      if (env)
        ssh_mem_debug_stack_trace_depth = strtoul(env, NULL, 0);

      num_frames = ssh_mem_debug_stack_trace_depth;
      if (num_frames == 0)
        num_frames = 1;

      ssh_mem_debug_stack_trace = ssh_malloc(num_frames * sizeof(void *));
      if (ssh_mem_debug_stack_trace == NULL)
        return;
    }

  if (ssh_mem_debug_stack_trace_depth == 0)
    /* Stack trace disabled. */
    return;

  /* Get the stack-trace into our temporary buffer. */

  SSH_GET_FP(fp);

  for (stack_trace_depth = 0;
       (SSH_VALID_FP(fp)
#ifdef WIN32
        /* Bounds checker makes the initial frames to be
           different. This is an extra check for Bounds Checker
           runs */
        && ((stack_trace_depth == 0)
            || (ssh_mem_debug_stack_trace[stack_trace_depth - 1] != NULL))
#endif /* WIN32 */
        && stack_trace_depth < ssh_mem_debug_stack_trace_depth);
       stack_trace_depth++)
    {
      ssh_mem_debug_stack_trace[stack_trace_depth] = SSH_FP_GET_PC(fp);
      fp = SSH_FP_GET_FP(fp);
    }

  if (stack_trace_depth == 0)
    /* An empty stack-trace. */
    return;

  hdr->stack_trace = ssh_malloc(stack_trace_depth * sizeof(void *));
  if (hdr->stack_trace == NULL)
    return;

  memcpy(hdr->stack_trace, ssh_mem_debug_stack_trace,
         stack_trace_depth * sizeof(void *));
  hdr->stack_trace_depth = stack_trace_depth;
}

#endif /* MEMORY_LEAK_STACK_TRACE */

/*********************** Handling of allocated blocks ***********************/

void
ssh_malloc_set_flags(SshUInt32 flags)
{
  SSH_MEMORY_LEAK_CHECK_LOCK();
  ssh_mem_debug_flags = flags;
  SSH_MEMORY_LEAK_CHECK_UNLOCK();
}

/* Dump additional symbol files that are in use in this
   application. */
static void
ssh_malloc_dump_symbol_files(FILE *outfp)
{
#ifdef __linux__
  pid_t pid = getpid();
  char buf[2048];
  char exe[1024];
  int i;
  FILE *fp;
  char *cp;

  /* Resole the executable name. */

  ssh_snprintf(buf, sizeof(buf), "/proc/%u/exe", pid);
  i = readlink(buf, exe, sizeof(exe) - 1);
  if (i < 0)
    {
      /* Could not read symlink. */
      fprintf(stderr,
              "ssh_malloc_dump_symbol_files(): could not readlink(%s): %s\n",
              buf, strerror(errno));
      return;
    }
  exe[i] = '\0';

  /* Read maps. */

  ssh_snprintf(buf, sizeof(buf), "/proc/%u/maps", pid);
  fp = fopen(buf, "r");
  if (fp == NULL)
    {
      /* Could not read maps. */
      fprintf(stderr,
              "ssh_malloc_dump_symbol_files(): could not fopen(%s): %s\n",
              buf, strerror(errno));
      return;
    }

  while (fgets(buf, sizeof(buf), fp))
    {
      unsigned long int addr;
      unsigned long int size;

      /* Delete trailing whitespace. */
      i = strlen(buf);
      for (i--;
           i >= 0 && (buf[i] == ' ' || buf[i] == '\n' || buf[i] == '\t');
           i--)
        ;
      i++;
      buf[i] = '\0';

      /* Get address. */
      addr = strtoul(buf, &cp, 16);
      if (*cp != '-')
        /* Malformed line. */
        continue;

      /* Compute section's size from the end offset. */
      cp++;
      size = strtoul(cp, &cp, 16);
      if (*cp != ' ')
        /* Malformed line. */
        continue;

      size -= addr;
      cp++;

      /* Check mode column. */
      if (cp[0] == ' ' || cp[1] == ' ' || cp[2] == ' ' || cp[3] == ' '
          || cp[4] != ' ')
        /* No mode column found. */
        continue;

      if (cp[1] == 'w')
        /* The map is writable.  Skip this map. */
        continue;

      cp += 5;

      /* Skip file offset. */
      cp = strchr(cp, ' ');
      if (cp == NULL)
        /* Malformed line. */
        continue;

      /* Skip device major & minor. */
      cp++;
      cp = strchr(cp, ' ');
      if (cp == NULL)
        /* Malformed line. */
        continue;

      /* Skip inode. */
      for (cp++; *cp && '0' <= *cp && *cp <= '9'; cp++)
        ;

      /* Skip whitespace. */
      for (; *cp && (*cp == ' ' || *cp == '\t' || *cp == '\n'); cp++)
        ;

      if (*cp == '\0')
        /* No shared object of file path name specified. */
        continue;

      /* If this is the executable itself, skip it. */
      if (strcmp(cp, exe) == 0)
        continue;

      /* Found a new symbol file. */
      fprintf(outfp,
              "<symbol-file offset=\"%lx\" size=\"%lx\" file=\"%s\"/>\n",
              addr, size, cp);
    }
  fclose(fp);

#endif /* __linux__ */
}

#ifdef VXWORKS
#include "sysSymTbl.h"
#endif /* VXWORKS */

#ifdef SSH_MEMORY_DEBUG_FREE_QUEUE
SshUInt32
ssh_malloc_dump_blocks(void)
{
  SshMemDebugHdr b, copy, copied_blocks = NULL;
  SshUInt32 num_leaks = 0, i, j;
  SshUInt32 copied_flags, copied_num_blocks, copied_balance, copied_count;
  FILE *outfp = NULL;
  const char *filename = NULL;
  SshMallocDumpBlocksFlags flags =
    SSH_MALLOC_DUMP_BLOCKS_SHOW_CTX_ID |
    SSH_MALLOC_DUMP_BLOCKS_SEPARATE_BY_CTX_ID;
  void *ctx_id = NULL;
  int use_ctx_id = flags & SSH_MALLOC_DUMP_BLOCKS_USE_CTX_ID;
  int show_ctx_id = flags & SSH_MALLOC_DUMP_BLOCKS_SHOW_CTX_ID;
  int separate_ctx_id = flags & SSH_MALLOC_DUMP_BLOCKS_SEPARATE_BY_CTX_ID;
  SshDlNode bh;
  SshSimpleHashEnumerator henum;
  
  if (alloc_fail_initialized == TRUE)
    ssh_alloc_fail_files_free();
  alloc_fail_initialized = FALSE;

  /* Return immediately if no memory handling operations. */
  if (ssh_mem_debug_blocks.hash.max_elem == 0) return 0;

  /* Lock and copy global data. */
  SSH_MEMORY_LEAK_CHECK_LOCK();

  copied_flags =
    SSH_GLOBAL_CHECK("ssh_mem_debug_flags") ? ssh_mem_debug_flags : 0;
  copied_num_blocks = ssh_mem_debug_num_blocks;
  copied_balance = ssh_mem_debug_balance;

  copied_count = 0;
  for (bh = SSH_SIMPLE_HASH_ENUMERATOR_START(&ssh_mem_debug_blocks.hash,
					     henum);
       bh;
       bh = SSH_SIMPLE_HASH_ENUMERATOR_NEXT(&ssh_mem_debug_blocks.hash, henum))
    {
      b = (SshMemDebugHdr)bh;
      
      if (use_ctx_id && ((SshCtxIdType)ctx_id) != b->ctxid)
        continue;
      if (b->marked)
        continue;
      copied_count++;
    }
  
  if (!(copied_blocks = ssh_malloc(copied_count * sizeof *b)))
    {
      fprintf(stderr, "Too many memory leaks to dump details.\n");
      num_leaks = copied_count;
      SSH_MEMORY_LEAK_CHECK_UNLOCK();
      goto final;
    }

  copied_count = 0;
  for (bh = SSH_SIMPLE_HASH_ENUMERATOR_START(&ssh_mem_debug_blocks.hash,
					     henum);
       bh;
       bh = SSH_SIMPLE_HASH_ENUMERATOR_NEXT(&ssh_mem_debug_blocks.hash, henum))
    {
      b = (SshMemDebugHdr)bh;
      copy = &copied_blocks[copied_count];

      if (use_ctx_id && ((SshCtxIdType)ctx_id) != b->ctxid)
        continue;
      if (b->marked)
        continue;

      memcpy(copy, b, sizeof *copy);
      copied_count++;
    }

  SSH_MEMORY_LEAK_CHECK_UNLOCK();

  for (i = 0; i < copied_count; i++)
    {
      SshUInt32 num_blocks = 0;
      SshUInt32 num_bytes = 0;
      Boolean ctxmix = FALSE;
      SshMemDebugHdr t;
      b = &copied_blocks[i];

      if (use_ctx_id && ((SshCtxIdType)ctx_id) != b->ctxid) continue;
      
      if (b->seen || b->marked)
        /* This is already handled. */
        continue;
      
      /* This is a new leak. */
      num_leaks++;

      /* Open the output file if it is not opened yet. */
      
      if (outfp == NULL)
        {
          filename = getenv("SSH_ALLOC_LEAKS_FILE");
          if (filename == NULL)
	    {
#ifndef SSH_STACKTRACE_FILE
              filename = "stacktrace.log";
#else /* SSH_STACKTRACE_FILE */
	      filename = SSH_STACKTRACE_FILE;
#endif /* SSH_STACKTRACE_FILE */
	    }
	  
          outfp = fopen(filename, "wb");
          if (outfp == NULL)
            outfp = stderr;
	  
          /* Dump additional symbol files that are in use in this
             program. */
          ssh_malloc_dump_symbol_files(outfp);
        }
      
      /* Collect all allocations from the same location. */
      for (j = 0; j < copied_count; j++)
        {
	  t = &copied_blocks[j];

	  if (!use_ctx_id || ((SshCtxIdType)ctx_id) == t->ctxid)
	    {
              if (t->file == b->file && t->line == b->line
#ifdef MEMORY_LEAK_STACK_TRACE
                  && t->stack_trace_depth == b->stack_trace_depth
                  && memcmp(t->stack_trace, b->stack_trace,
                            t->stack_trace_depth * sizeof(void *)) == 0
#endif /* MEMORY_LEAK_STACK_TRACE */
		  )
		{
		  if (t->ctxid != b->ctxid) 
		    {
		      if (separate_ctx_id) continue;
		      ctxmix = TRUE;
		    }
                  num_blocks++;
                  num_bytes += t->size;

                  /* Mark this as processed. */
                  t->seen = 1;
                }
	    }
	}

      if (num_blocks == 1
          && (copied_flags & SSH_MALLOC_F_IGNORE_SINGLE_BLOCKS))
        {
          /* We do not report single allocs. */
        }
      else
        {
	  if (show_ctx_id)
	    {
	      fprintf(outfp, "<stacktrace>%s:%d: #blocks=%lu, bytes=%lu, "
		      "ctxid=%p%s\n",
		      b->file, b->line, (unsigned long) num_blocks,
		      (unsigned long) num_bytes, (void*)b->ctxid, 
		      ctxmix?"+":"");
	    }
	  else
	    {	
	      fprintf(outfp, "<stacktrace>%s:%d: #blocks=%lu, bytes=%lu\n",
		      b->file, b->line, (unsigned long) num_blocks,
		      (unsigned long) num_bytes);
	    }
#ifdef MEMORY_LEAK_STACK_TRACE
          {
            SshUInt32 i;
#ifdef VXWORKS
            char buf[256];
            int value;
            SYM_TYPE type;
#endif /* VXWORKS */
            for (i = 0; i < b->stack_trace_depth; i++)
              {
#ifdef WINDOWS
                {
                  char file_name[MAX_PATH];
                  int line_number;
                  if (ssh_debug_win_find_line_by_pc(NULL, b->stack_trace[i],
                                                    file_name, &line_number))
                    {
                      fprintf(outfp, "<file location>%s:%u\n",
                              file_name, line_number);
                    }
                  else
                    {
                      fprintf(outfp, "<pc>%p\n", b->stack_trace[i]);
                    }
                }
#else /* !WINDOWS */
#ifndef VXWORKS
                fprintf(outfp, "<pc>%p\n", b->stack_trace[i]);
#endif /* VXWORKS */
#ifdef VXWORKS
                symFindByValue(sysSymTbl,
                               (int)b->stack_trace[i],
                               buf,
                               &value,
                               &type);
		if ((int)b->stack_trace[i] == (int)value)
		  fprintf(outfp, "\t%s\n", buf);
		else
		  fprintf(outfp, "\t%s+0x%x\n", 
			  buf, ((int)b->stack_trace[i]) - (int)value);
#endif /* VXWORKS */
#endif /* WINDOWS */
              }
          }
#endif /* MEMORY_LEAK_STACK_TRACE */
        }
    }

 final:
  /* Print final statistics. */
  if (num_leaks == 0)
    {
      fprintf(stderr, "No memory leaks.\n");
      if (outfp != NULL && outfp != stderr) fclose(outfp);
    }
  else
    {
      if (outfp == stderr)
        {
          fprintf(outfp,
                  "Summary: %lu leaks, %lu blocks, %lu bytes\n",
                  (unsigned long) num_leaks,
                  (unsigned long) copied_num_blocks,
                  (unsigned long) copied_balance);
        }
      else
        {
          fprintf(stderr,
                  "\
**********************************************************************\n"
                  "*\tMemory leaks dumped to `%s'.\n", filename);
          fprintf(stderr,
                  "*\tSummary: %lu leaks, %lu blocks, %lu bytes\n\
**********************************************************************\n",
                  (unsigned long) num_leaks,
                  (unsigned long) copied_num_blocks,
                  (unsigned long) copied_balance);

          /* We opened the output file so let's close it now. */
          fclose(outfp);
        }
    }
  if (copied_blocks)
    ssh_free(copied_blocks);

  return num_leaks;
}
#else /* SSH_MEMORY_DEBUG_FREE_QUEUE */
SshUInt32
ssh_malloc_dump_blocks(void)
{
  SshMemDebugHdr b;
  SshUInt32 num_leaks = 0;
  FILE *outfp = NULL;
  const char *filename = NULL;

  SSH_MEMORY_LEAK_CHECK_LOCK();

  for (b = ssh_mem_debug_blocks; b; b = b->next)
    b->seen = 0;

  for (b = ssh_mem_debug_blocks; b; b = b->next)
    {
      SshUInt32 num_blocks = 0;
      SshUInt32 num_bytes = 0;
      SshMemDebugHdr t;

      if (b->seen || b->marked)
        /* This is already handled. */
        continue;

      /* This is a new leak. */
      num_leaks++;

      /* Open the output file if it is not opened yet. */

      if (outfp == NULL)
        {
          filename = getenv("SSH_ALLOC_LEAKS_FILE");
          if (filename == NULL)
	    {
#ifndef SSH_STACKTRACE_FILE
	      filename = "stacktrace.log";
#else /* SSH_STACKTRACE_FILE */
	      filename = SSH_STACKTRACE_FILE;
#endif /* SSH_STACKTRACE_FILE */
	    }
	      
          outfp = fopen(filename, "wb");
          if (outfp == NULL)
            outfp = stderr;

          /* Dump additional symbol files that are in use in this
             program. */
          ssh_malloc_dump_symbol_files(outfp);
        }

      /* Collect all allocations from the same location. */
      for (t = b; t; t = t->next)
        if (t->file == b->file && t->line == b->line
#ifdef MEMORY_LEAK_STACK_TRACE
            && t->stack_trace_depth == b->stack_trace_depth
            && memcmp(t->stack_trace, b->stack_trace,
                      t->stack_trace_depth * sizeof(void *)) == 0
#endif /* MEMORY_LEAK_STACK_TRACE */
            )
          {
            num_blocks++;
            num_bytes += t->size;

            /* Mark this as processed. */
            t->seen = 1;
          }

      if (num_blocks == 1
          && (ssh_mem_debug_flags & SSH_MALLOC_F_IGNORE_SINGLE_BLOCKS))
        {
          /* We do not report single allocs. */
        }
      else
        {
          fprintf(outfp, "<stacktrace>%s:%d: #blocks=%lu, bytes=%lu\n",
                  b->file, b->line, (unsigned long) num_blocks,
                  (unsigned long) num_bytes);
#ifdef MEMORY_LEAK_STACK_TRACE
          {
            SshUInt32 i;
#ifdef VXWORKS
#include "sysSymTbl.h"
            char buf[256];
            int value;
            SYM_TYPE type;
#endif /* VXWORKS */
            for (i = 0; i < b->stack_trace_depth; i++)
              {
#ifdef WINDOWS
                {
                  char file_name[MAX_PATH];
                  int line_number;
                  if (ssh_debug_win_find_line_by_pc(NULL, b->stack_trace[i],
                                                    file_name, &line_number))
                    {
                      fprintf(outfp, "<file location>%s:%u\n",
                              file_name, line_number);
                    }
                  else
                    {
                      fprintf(outfp, "<pc>%p\n", b->stack_trace[i]);
                    }
                }
#else /* !WINDOWS */
#ifndef VXWORKS
                fprintf(outfp, "<pc>%p\n", b->stack_trace[i]);
#endif /* VXWORKS */
#ifdef VXWORKS
                symFindByValue(sysSymTbl,
                               (int) b->stack_trace[i],
                               buf,
                               &value,
                               &type);

		if ((int)b->stack_trace[i] == (int)value)
		  fprintf(outfp, "\t%s\n", buf);
		else
		  fprintf(outfp, "\t%s+0x%x\n",
			  buf, ((int)b->stack_trace[i]) - (int)value);
#endif /* VXWORKS */
#endif /* WINDOWS */
              }
          }
#endif /* MEMORY_LEAK_STACK_TRACE */
        }
    }

  /* Print final statistics. */
  if (num_leaks == 0)
    {
      fprintf(stderr, "No memory leaks.\n");
      if (outfp != NULL && outfp != stderr)
	fclose(outfp);
    }
  else
    {
      if (outfp == stderr)
        {
          fprintf(outfp,
                  "Summary: %lu leaks, %lu blocks, %lu bytes\n",
                  (unsigned long) num_leaks,
                  (unsigned long) ssh_mem_debug_num_blocks,
                  (unsigned long) ssh_mem_debug_balance);
        }
      else
        {
          fprintf(stderr,
                  "\
**********************************************************************\n"
                  "*\tMemory leaks dumped to `%s'.\n", filename);
          fprintf(stderr,
                  "*\tSummary: %lu leaks, %lu blocks, %lu bytes\n\
**********************************************************************\n",
                  (unsigned long) num_leaks,
                  (unsigned long) ssh_mem_debug_num_blocks,
                  (unsigned long) ssh_mem_debug_balance);

          /* We opened the output file so let's close it now. */
          fclose(outfp);
        }
    }

  SSH_MEMORY_LEAK_CHECK_UNLOCK();

  return num_leaks;
}
#endif /* SSH_MEMORY_DEBUG_FREE_QUEUE */

void
ssh_malloc_dump_stats(void)
{
  SshMallocStatisticsStruct s;

  ssh_malloc_get_stats(&s);

  fprintf(stderr, "now: #blocks=%lu, bytes=%lu\n",
          (unsigned long) s.current_blocks,
          (unsigned long) s.current_bytes);
  fprintf(stderr, "max: #blocks=%lu, bytes=%lu\n",
          (unsigned long) s.max_blocks,
          (unsigned long) s.max_bytes);
}


void
ssh_malloc_get_stats(SshMallocStatistics statistics)
{
  SSH_MEMORY_LEAK_CHECK_LOCK();

  statistics->current_blocks = ssh_mem_debug_num_blocks;
  statistics->current_bytes = ssh_mem_debug_balance;
  statistics->max_blocks = ssh_mem_debug_max_num_blocks;
  statistics->max_bytes = ssh_mem_debug_max_balance;

  SSH_MEMORY_LEAK_CHECK_UNLOCK();
}


void
ssh_malloc_mark_blocks(void)
{
  SshMemDebugHdr b;
#ifdef SSH_MEMORY_DEBUG_FREE_QUEUE
  SshDlNode bh;
  SshSimpleHashEnumerator henum;
#endif /* SSH_MEMORY_DEBUG_FREE_QUEUE */

  SSH_MEMORY_LEAK_CHECK_LOCK();

#ifdef SSH_MEMORY_DEBUG_FREE_QUEUE
  for (bh = SSH_SIMPLE_HASH_ENUMERATOR_START(&ssh_mem_debug_blocks.hash,
					     henum);
       bh;
       bh = SSH_SIMPLE_HASH_ENUMERATOR_NEXT(&ssh_mem_debug_blocks.hash, henum))
    {
      b = (SshMemDebugHdr)bh;
      b->marked = 1;
    }
#else /* SSH_MEMORY_DEBUG_FREE_QUEUE */
  for (b = ssh_mem_debug_blocks; b; b = b->next)
    b->marked = 1;
#endif /* SSH_MEMORY_DEBUG_FREE_QUEUE */

  SSH_MEMORY_LEAK_CHECK_UNLOCK();
}

void
ssh_malloc_unmark_blocks(void)
{
  SshMemDebugHdr b;
#ifdef SSH_MEMORY_DEBUG_FREE_QUEUE
  SshDlNode bh;
  SshSimpleHashEnumerator henum;
#endif /* SSH_MEMORY_DEBUG_FREE_QUEUE */

  SSH_MEMORY_LEAK_CHECK_LOCK();

#ifdef SSH_MEMORY_DEBUG_FREE_QUEUE
  for (bh = SSH_SIMPLE_HASH_ENUMERATOR_START(&ssh_mem_debug_blocks.hash,
					     henum);
       bh;
       bh = SSH_SIMPLE_HASH_ENUMERATOR_NEXT(&ssh_mem_debug_blocks.hash, henum))
    {
      b = (SshMemDebugHdr)bh;
      b->marked = 0;
    }
#else /* SSH_MEMORY_DEBUG_FREE_QUEUE */
  for (b = ssh_mem_debug_blocks; b; b = b->next)
    b->marked = 0;
#endif /* SSH_MEMORY_DEBUG_FREE_QUEUE */

  SSH_MEMORY_LEAK_CHECK_UNLOCK();
}

#ifdef SSH_MEMORY_DEBUG_FREE_QUEUE
/*
#define CHECK_MD_LIST() do { SshMemDebugHdr b; SshMemDebugHdr bp = NULL; \
                             for (b = ssh_mem_debug_blocks; \
                                  b; b = b->next) \
                               { \
                                 bp = b; \
                                 if (b->freed) *(char*)0xffffffff = -1; \
                               } \
                           } while(0)
*/
/*
#define CHECK_MD_LIST_E(e) do { CHECK_MD_LIST(); \
                              { SshMemDebugHdr b; \
                                for (b = ssh_mem_debug_blocks;; b = b->next)\
                              if(b==e) break;} \
                           } while(0)
*/

#define CHECK_MD_LIST() do { } while(0)
#define CHECK_MD_LIST_E(e) \
  do { if (!mem_debug_exists(e)) *(char*)0xffffffff = -1; } while(0)

static SshUInt32
mem_debug_hash_noassert(void *ptr, void *stub_ctx)
{
  SshUInt32 hash;

  hash = (SshUInt32)(unsigned long)ptr;
  return hash ^ (hash >> 16);
}

static SshUInt32
mem_debug_hash(void *ptr, void *stub_ctx)
{
  SshMemDebugHdr ptr_ = ptr;
  SshUInt32 hash;
  SSH_ASSERT(ptr_->freed == FALSE);

  hash = (SshUInt32)(unsigned long)ptr;
  return hash ^ (hash >> 16);
}

#if 0
static int
mem_debug_compare(void *ptr1, void *ptr2, void *stub_ctx)
{
  SshMemDebugHdr a_ = ptr1;
  SshMemDebugHdr b_ = ptr2;
  SshUInt32 a = (SshUInt32)ptr1;
  SshUInt32 b = (SshUInt32)ptr2;

  SSH_ASSERT(a_->freed == FALSE);
  SSH_ASSERT(b_->freed == FALSE);

  if (a<b) return -1;
  else if (a==b) return 0;
  else return 1;
}
#endif /* 0 */

void mem_debug_queue(SshMemDebugHdr b_, void *stub_ctx)
{
  /* Makes queue of freed entries. This allows recently freed memblock
     entries to work relatively well and us to detect that recently
     freed entry is messed with. */
  SshMemDebugHdr b = b_;
  SshMemDebugHdrStruct **blocks = ssh_mem_free_blocks;
  b->freed = TRUE;
  ssh_mem_free_block = (ssh_mem_free_block + 1) % SSH_ML_FREE_QUEUE_LEN;
  if (blocks[ssh_mem_free_block] != NULL)
    {
      ssh_free(blocks[ssh_mem_free_block]);
    }
  blocks[ssh_mem_free_block] = b;
}

void mem_debug_init(void)
{ 
  if (ssh_mem_debug_initialized) return;

  SSH_SIMPLE_HASH_INIT(&ssh_mem_debug_blocks.hash,
		       SSH_MEM_DEBUG_BLOCKS_HASH_SIZE,
		       sizeof(ssh_mem_debug_blocks));
  
  ssh_mem_debug_initialized = TRUE;
}

Boolean mem_debug_exists(SshMemDebugHdr b)
{
  if (!ssh_mem_debug_initialized)
    mem_debug_init();

  return SSH_SIMPLE_HASH_NODE_EXISTS(&ssh_mem_debug_blocks.hash, 
				     &b->node, 
				     mem_debug_hash(b, NULL));
}

void mem_debug_add(SshMemDebugHdr b)
{
  if (!ssh_mem_debug_initialized)
    mem_debug_init();

  SSH_SIMPLE_HASH_NODE_INSERT(&ssh_mem_debug_blocks.hash, 
			      &b->node, 
			      mem_debug_hash(b, NULL));
}

void mem_debug_rem(SshMemDebugHdr b)
{
  SSH_ASSERT(ssh_mem_debug_initialized);
  
  SSH_SIMPLE_HASH_NODE_DETACH(&ssh_mem_debug_blocks.hash, 
			      &b->node, 
			      mem_debug_hash(b, NULL));
}
#else /* SSH_MEMORY_DEBUG_FREE_QUEUE */

#define CHECK_MD_LIST()
#define CHECK_MD_LIST_E(hdr)

#endif /* SSH_MEMORY_DEBUG_FREE_QUEUE */

/*************************** Allocation functions ***************************/

void *
ssh_malloc_i(size_t size, Boolean can_fail, const char *file, int line)
{
  SshMemDebugHdr hdr;

  if (can_fail)
    MAYBE_FAIL_ALLOC(file);

  if (size == 0)
    size = 1;

  hdr = ssh_malloc(SSH_MEM_DEBUG_SIZE(size));
  if (hdr == NULL)
    return NULL;

  hdr->seen = 0;
  hdr->marked = 0;
  hdr->file = file;
  hdr->line = line;
  hdr->size = size;
#ifdef SSH_MEMORY_DEBUG_FREE_QUEUE
  hdr->ctxid = ML_GET_CTX_ID();
#endif /* SSH_MEMORY_DEBUG_FREE_QUEUE */
  hdr->freed = FALSE;

  SSH_MEMORY_LEAK_CHECK_LOCK();

  CHECK_MD_LIST();

#ifdef MEMORY_LEAK_STACK_TRACE
  hdr->stack_trace_depth = 0;
  hdr->stack_trace = NULL;
  ssh_malloc_set_stack_trace(hdr);
#endif /* MEMORY_LEAK_STACK_TRACE */

#ifdef SSH_MEMORY_DEBUG_FREE_QUEUE
  mem_debug_add(hdr);
#else /* SSH_MEMORY_DEBUG_FREE_QUEUE */
  hdr->next = ssh_mem_debug_blocks;
  hdr->prev = NULL;

  if (ssh_mem_debug_blocks)
    ssh_mem_debug_blocks->prev = hdr;

  ssh_mem_debug_blocks = hdr;
#endif /* SSH_MEMORY_DEBUG_FREE_QUEUE */

  ssh_mem_debug_balance += size;
  ssh_mem_debug_num_blocks++;

  if (ssh_mem_debug_balance > ssh_mem_debug_max_balance)
    ssh_mem_debug_max_balance = ssh_mem_debug_balance;
  if (ssh_mem_debug_num_blocks > ssh_mem_debug_max_num_blocks)
    ssh_mem_debug_max_num_blocks = ssh_mem_debug_num_blocks;

  CHECK_MD_LIST();

  SSH_MEMORY_LEAK_CHECK_UNLOCK();

  return ((unsigned char *) hdr) + sizeof(SshMemDebugHdrStruct);
}


void *
ssh_realloc_i(void *ptr, size_t old_size, size_t new_size,
              Boolean check_old_size, Boolean can_fail,
	      const char *file, int line)
{
  SshMemDebugHdr hdr;
  void *new_ptr;

  if (ptr == NULL)
    return ssh_malloc_i(new_size, can_fail, file, line);

  hdr = SSH_MEM_DEBUG_HDR(ptr);
#ifdef SSH_MEMORY_DEBUG_FREE_QUEUE
  CHECK_MD_LIST_E(hdr);

  if (hdr->ctxid != ML_GET_CTX_ID())
    {
      fprintf(stderr,
	      "%s:%d: memory block %p:0x%x reallocation "
	      "changes threadid from %p to %p\n",
	      file, line, ptr, (int)old_size, 
              (void*)hdr->ctxid, (void*)ML_GET_CTX_ID());

      hdr->ctxid = ML_GET_CTX_ID();
    }
#endif /* SSH_MEMORY_DEBUG_FREE_QUEUE */
    
  if (check_old_size && hdr->size != old_size)
    fprintf(stderr,
            "%s:%d: reallocating block with wrong idea about the old size: "
            "block initially allocated at `%s:%d'\n",
            file, line, hdr->file, hdr->line);

  if (hdr->size >= new_size)
    {
      /* it is valid to fail even when shrinking */
      if (can_fail)
        MAYBE_FAIL_ALLOC(file);

      SSH_MEMORY_LEAK_CHECK_LOCK();
      CHECK_MD_LIST();
      ssh_mem_debug_balance -= (hdr->size - new_size);
      SSH_MEMORY_LEAK_CHECK_UNLOCK();

      hdr->size = new_size;
      return ptr;
    }

  /* Allocating a bigger block. */
  new_ptr = ssh_malloc_i(new_size, can_fail, file, line);
  if (new_ptr == NULL)
    return NULL;

  memcpy(new_ptr, ptr, hdr->size);

  ssh_free_i(ptr, file, line);

  return new_ptr;
}


void *
ssh_calloc_i(size_t nitems, size_t size, Boolean can_fail,
             const char *file, int line)
{
  void *ptr = ssh_malloc_i(nitems * size, can_fail, file, line);

  if (ptr == NULL)
    return NULL;

  memset(ptr, 0, nitems * size);

  return ptr;
}


void *
ssh_strdup_i(const void *p, Boolean can_fail, 
	     const char *file, int line)
{
  size_t len;
  char *str;

  if (p == NULL)
    return NULL;

  len = strlen(p);
  str = ssh_malloc_i(len + 1, can_fail, file, line);
  if (str == NULL)
    return NULL;

  memcpy(str, p, len);
  str[len] = '\0';

  return str;
}


void *
ssh_memdup_i(const void *p, size_t len, Boolean can_fail,
             const char *file, int line)
{
  unsigned char *mem = ssh_malloc_i(len + 1, can_fail, file, line);

  if (mem == NULL)
    return NULL;

  memcpy(mem, p, len);
  mem[len] = '\0';

  return mem;
}


void
ssh_free_i(void *ptr, const char *file, int line)
{
  SshMemDebugHdr hdr;

  if (ptr == NULL)
    return;

  SSH_MEMORY_LEAK_CHECK_LOCK();

  hdr = SSH_MEM_DEBUG_HDR(ptr);

  CHECK_MD_LIST_E(hdr);

  ssh_mem_debug_balance -= hdr->size;
  ssh_mem_debug_num_blocks--;

#ifdef MEMORY_LEAK_STACK_TRACE
  if (hdr->stack_trace)
    ssh_free(hdr->stack_trace);
#endif /* MEMORY_LEAK_STACK_TRACE */

#ifdef SSH_MEMORY_DEBUG_FREE_QUEUE
  mem_debug_rem(hdr);

  CHECK_MD_LIST();
  memset(ptr, 'F', hdr->size);
  CHECK_MD_LIST();
  mem_debug_queue(hdr, 0);
  SSH_MEMORY_LEAK_CHECK_UNLOCK();
#else /* SSH_MEMORY_DEBUG_FREE_QUEUE */
  if (hdr->next)
    hdr->next->prev = hdr->prev;

  if (hdr->prev)
    hdr->prev->next = hdr->next;
  else
    ssh_mem_debug_blocks = hdr->next;
  
  SSH_MEMORY_LEAK_CHECK_UNLOCK();

  memset(ptr, 'F', hdr->size);
  ssh_free(hdr);
#endif /* SSH_MEMORY_DEBUG_FREE_QUEUE */

}

void *
ssh_xmalloc_i(size_t size, const char *file, int line)
{
  void *ptr = ssh_malloc_i(size, FALSE, file, line);

  if (ptr == NULL)
    {
      ssh_malloc_dump_blocks();
      ssh_fatal("Could not allocate %zu bytes", size);
    }

  return ptr;
}


void *
ssh_xrealloc_i(void *ptr, size_t new_size,  
	       const char *file, int line)
{
  void *nptr = ssh_realloc_i(ptr, 0, new_size, FALSE, FALSE, 
			     file, line);
  if (nptr == NULL)
    {
      ssh_malloc_dump_blocks();
      ssh_fatal("Could not allocate %zu bytes", new_size);
    }

  return nptr;
}


void *
ssh_xcalloc_i(size_t nitems, size_t size,  
	      const char *file, int line)
{
  void *ptr = ssh_calloc_i(nitems, size, FALSE, file, line);

  if (ptr == NULL)
    {
      ssh_malloc_dump_blocks();
      ssh_fatal("Could not allocate %zu bytes", nitems * size);
    }

  return ptr;
}


void *
ssh_xstrdup_i(const void *p, const char *file, int line)
{
  void *ptr;

  if (p == NULL)
    return NULL;

  ptr = ssh_strdup_i(p, FALSE, file, line);

  if (ptr == NULL)
    {
      ssh_malloc_dump_blocks();
      ssh_fatal("Could not allocate %zu bytes", strlen(p));
    }

  return ptr;
}


void *
ssh_xmemdup_i(const void *p, size_t len,  
	      const char *file, int line)
{
  void *ptr = ssh_memdup_i(p, len, FALSE, file, line);

  if (ptr == NULL)
    {
      ssh_malloc_dump_blocks();
      ssh_fatal("Could not allocate %zu bytes", len);
    }

  return ptr;
}


void
ssh_xfree_i(void *ptr, const char *file, int line)
{
  ssh_free_i(ptr, file, line);
}

#ifdef SSH_MEMORY_DEBUG_FREE_QUEUE
/* Is pointer free or allocated, and where it is? */
#define PTR_RECENTLY_FREED 0x2
#define PTR_FREE 0x0
#define PTR_ALLOCATED 0x1
#define PTR_MIDDLE_OF_BLOCK 0x10
size_t ssh_ptr_status(void *ptr, 
		      Boolean print_data,
		      Boolean do_not_try_middle_block)
{
  size_t bits = 0;
  size_t blocksize = 0;
  SshMemDebugHdr b = NULL;
  int i;
  SshMemDebugHdr bb;
  unsigned char *ptr_s;
  unsigned char *ptr_e;
  SshSimpleHashEnumerator henum;
  
  bb = SSH_MEM_DEBUG_HDR(ptr);
  SSH_MEMORY_LEAK_CHECK_LOCK();
  if (SSH_SIMPLE_HASH_NODE_EXISTS(&ssh_mem_debug_blocks.hash, 
				  &bb->node, 
				  mem_debug_hash_noassert(bb, NULL)))
    {
      b = SSH_MEM_DEBUG_HDR(ptr);
      bits |= PTR_ALLOCATED;
    }

  for(i=0; i<SSH_ML_FREE_QUEUE_LEN; i++)
    {
      bb = ssh_mem_free_blocks[i];
      if (!bb) continue;
      ptr_s = (unsigned char*)&bb[1];
      ptr_e = ptr_s + bb->size;
      if ((unsigned long)ptr >= (unsigned long)ptr_s &&
	  (unsigned long)ptr < (unsigned long)ptr_e)
	{
	  /* block found in free */
	  bits |= PTR_RECENTLY_FREED;
	  if (ptr != ptr_s)
	    {
	      bits |= PTR_MIDDLE_OF_BLOCK;
	    }
	  b = bb;
	  break;
	}
    }

  if (b == NULL && !do_not_try_middle_block)
    {
      SshDlNode bh;

      for (bh = SSH_SIMPLE_HASH_ENUMERATOR_START(&ssh_mem_debug_blocks.hash,
						 henum);
	   bh;
	   bh = SSH_SIMPLE_HASH_ENUMERATOR_NEXT(&ssh_mem_debug_blocks.hash, 
						henum))
	{
	  bb = (SshMemDebugHdr)bh;
	  ptr_s = (unsigned char*)&bb[1];
	  ptr_e = ptr_s + bb->size;
	  
	  if ((unsigned long)ptr >= (unsigned long)ptr_s &&
	      (unsigned long)ptr < (unsigned long)ptr_e)
	    {
	      /* block found in free */
	      bits |= PTR_ALLOCATED;
	      SSH_ASSERT(ptr != ptr_s);
	      bits |= PTR_MIDDLE_OF_BLOCK;
	      b = bb;
	      break;
	    }
	}
    }

  if (!b) 
    {
      SSH_MEMORY_LEAK_CHECK_UNLOCK();
      return bits;
    }

  if (print_data)
    {
#ifdef MEMORY_LEAK_STACK_TRACE
          {
            SshUInt32 i;
#ifdef VXWORKS
            char buf[256];
            int value;
            SYM_TYPE type;
#endif /* VXWORKS */
            for (i = 0; i < b->stack_trace_depth; i++)
              {
#ifdef WINDOWS
                {
                  char file_name[MAX_PATH];
                  int line_number;
                  if (ssh_debug_win_find_line_by_pc(NULL, b->stack_trace[i],
                                                    file_name, &line_number))
                    {
                      fprintf(stderr, "<file location>%s:%u\n",
                              file_name, line_number);
                    }
                  else
                    {
                      fprintf(stderr, "<pc>%p\n", b->stack_trace[i]);
                    }
                }
#else /* !WINDOWS */
#ifndef VXWORKS
                fprintf(stderr, "<pc>%p\n", b->stack_trace[i]);
#endif /* VXWORKS */
#ifdef VXWORKS
                symFindByValue(sysSymTbl,
                               (int)b->stack_trace[i],
                               buf,
                               &value,
                               &type);
		if ((int)b->stack_trace[i] == (int)value)
		  printf("\t%s\n", buf);
		else
		  printf("\t%s+0x%x\n", buf, 
			 ((int)b->stack_trace[i]) - (int)value);
#endif /* VXWORKS */
#endif /* WINDOWS */
              }
          }
#endif /* MEMORY_LEAK_STACK_TRACE */
    }
  blocksize = b->size;
  SSH_MEMORY_LEAK_CHECK_UNLOCK();
  return bits | (blocksize << 8);
}
#endif /* SSH_MEMORY_DEBUG_FREE_QUEUE */

#else /* not MEMORY_LEAK_CHECKS */

/**************** Stubs functions without memory leak checks ****************/

void
ssh_malloc_set_flags(SshUInt32 flags)
{
}


SshUInt32
ssh_malloc_dump_blocks(void)
{
  ssh_warning("Memory leak checks not compiled in.");
  return 0;
}


void
ssh_malloc_dump_stats(void)
{
  ssh_warning("Memory leak checks not compiled in.");
}


void
ssh_malloc_mark_blocks(void)
{
}


void
ssh_malloc_unmark_blocks(void)
{
}

#endif /* not MEMORY_LEAK_CHECKS */
