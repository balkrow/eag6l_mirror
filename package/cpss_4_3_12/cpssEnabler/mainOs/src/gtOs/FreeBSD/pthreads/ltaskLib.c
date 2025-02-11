/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/
/**
********************************************************************************
* @file ltaskLib.c
*
* @brief Pthread implementation of mainOs tasks
*
* @version   15
********************************************************************************
*/
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <errno.h>
#include <unistd.h>
#include <sched.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <pthread.h>
#include <signal.h>
#include <limits.h>

#include <gtOs/gtOsTask.h>
#include <gtOs/osObjIdLib.h>
#if __WORDSIZE == 64
#  ifdef ALL_ALLOCS_32BIT
#    include <sys/mman.h>
#  endif
#endif

#include <gtOs/gtOsSharedData.h>

#include "mainOsPthread.h"

/************* Defines ********************************************************/
#undef DIAG_PRINTFS
#define TASKS_ALLOC_CHUNK 32

/************* Internal types *************************************************/
typedef struct _V2L_taskSTC
{
    OS_OBJECT_HEADER_STC    header;
    pthread_t               pthrid;
    int                     vxw_priority;
    unsigned                (__TASKCONV *entry_point)(GT_VOID*);
    GT_VOID                 *param;
} _V2L_taskSTC;


/************* Internal data **************************************************/

static OS_OBJECT_LIST_STC tasks __SHARED_DATA_MAINOS = {
    NULL,                   /* list */
    0,                      /* allocated */
    TASKS_ALLOC_CHUNK,      /* allocChunk */
    sizeof(_V2L_taskSTC),   /* objSize */
    V2L_ts_malloc,          /* allocFuncPtr */
    V2L_ts_free             /* freeFuncPtr */
};

#define TASK(id) ((_V2L_taskSTC*)(tasks.list[id]))

#ifndef SHARED_MEMORY
static pthread_mutex_t tasks_mtx = PTHREAD_MUTEX_INITIALIZER;
#else
static int initialized = 0;
static pthread_mutex_t tasks_mtx __SHARED_DATA_MAINOS;
#endif

/* taskLock data */
static pthread_mutex_t taskLock_mtx = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t taskLock_cond = PTHREAD_COND_INITIALIZER;
static int taskLock_count = 0;
static pthread_t taskLock_owner = 0;

/************ Forward declararions ********************************************/
#if defined(LINUX_SIM) && __GLIBC_PREREQ(2,13)
    /* we don't need task_log(), gdb shows info */
#else
static GT_VOID task_log(const GT_CHAR *fmt, ...);
#endif
static GT_VOID V2L_taskUnlock_i(
    IN  pthread_t   owner,
    IN  int         force
);


/************ Public Functions ************************************************/

/**
* @internal V2L_ltaskInit function
* @endinternal
*
* @brief   Initialize tasks
*
* @param[in] name                     - root task name
*
* @retval OK                       - on success
* @retval ERROR                    - on error
*
* @note This function called from osStartEngine()
*
*/
int V2L_ltaskInit
(
    IN  const GT_CHAR *name
)
{
    OS_OBJECT_HEADER_STC *h;
    _V2L_taskSTC *tsk;

#ifdef SHARED_MEMORY
    if (!initialized)
    {
        pthread_mutexattr_t attr;
        pthread_mutexattr_init(&attr);
        pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);
        pthread_mutex_init(&tasks_mtx, &attr);
        initialized = 1;
    }
#endif
    pthread_mutex_lock(&tasks_mtx);

    osObjLibGetNewObject(&tasks, 1, name?name:"tUsrRoot", &h);
    tsk = (_V2L_taskSTC*)h;

    tsk->pthrid = pthread_self();

    pthread_mutex_unlock(&tasks_mtx);
    return 0;
}

/*****************************************************************************
**  cleanup_task_list ensures that a killed pthread cleans entry in task list
*****************************************************************************/
static void cleanup_task_list(void* arg)
{
    _V2L_taskSTC *tsk = (_V2L_taskSTC*)arg;

    tsk->header.type = 0;
}

/*****************************************************************************
**  os_task_wrapper
*****************************************************************************/
static void* os_task_wrapper(void *arg)
{
    _V2L_taskSTC *tsk = (_V2L_taskSTC*)arg;

    pthread_cleanup_push(cleanup_task_list, (void*)tsk );

    tsk->entry_point(tsk->param);

    pthread_cleanup_pop( 1 );

    return NULL;
}


/**
* @internal osTaskCreate function
* @endinternal
*
* @brief   Create OS Task and start it.
*
* @param[in] name                     - task name, string no longer then OS_MAX_TASK_NAME_LENGTH
* @param[in] prio                     - task priority 255 - 0 => HIGH
* @param[in] stack                    - task Stack Size
*                                      start_addr - task Function to execute
*                                      arglist    - pointer to list of parameters for task function
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS osTaskCreate
(
    IN  const GT_CHAR  *name,
    IN  GT_U32      prio,
    IN  GT_U32      stack,
    IN  unsigned    (__TASKCONV *start_addr)(GT_VOID*),
    IN  GT_VOID     *arglist,
    OUT GT_TASK     *tid
)
{
    int t;
    OS_OBJECT_HEADER_STC *h;
    _V2L_taskSTC *tsk;
    pthread_attr_t  attrs;
    GT_U32      stack_min;

    pthread_attr_init(&attrs);

#if !defined(LINUX_SIM) && !defined(RTOS_ON_SIM)
    stack_min = PTHREAD_STACK_MIN;
#else /* simulation */
#ifdef GM_USED
    stack_min = 0x200000; /* 2M */
#else /* !GM_USED */
    stack_min = 0x20000; /* 128K */
#endif /* !GM_USED */

#endif
	
    if (stack)
    {
        if (stack < stack_min)
            stack = stack_min;
        pthread_attr_setstacksize(&attrs, stack);
    }

#if (__WORDSIZE == 64) && !defined(MIPS64_CPU) && !defined(INTEL64_CPU)
#  ifdef ALL_ALLOCS_32BIT
    /* workaround: allocate stack in first 2Gig address space */
    {
        size_t  stksize;
        GT_VOID *stkaddr;
        pthread_attr_getstacksize( &attrs, &stksize);
        stkaddr = mmap(NULL, stksize,
            PROT_READ | PROT_WRITE,
            MAP_32BIT | MAP_GROWSDOWN | MAP_STACK | MAP_ANONYMOUS | MAP_PRIVATE,
            0, 0);
        pthread_attr_setstack( &attrs, stkaddr, stksize);
    }
#endif
#endif

    /*!! set priority */

    pthread_mutex_lock(&tasks_mtx);

    t = osObjLibGetNewObject(&tasks, 1, name, &h);
    tsk = (_V2L_taskSTC*)h;

    if (!name)
    {
        sprintf(tsk->header.name, "t%d", t);
    }
    tsk->vxw_priority = prio;
    tsk->entry_point = start_addr;
    tsk->param = arglist;

    pthread_mutex_unlock(&tasks_mtx);

    if (tid != NULL)
        *tid = (GT_TASK)t;
    if ( pthread_create( &(tsk->pthrid), &attrs,
                         os_task_wrapper, (void*)tsk) != 0 )
    {
        tsk->header.type = 0;
#ifdef DIAG_PRINTFS
        perror( "\r\ntaskSpawn(): pthread_create returned error:" );
#endif
        return GT_FAIL;
    } else {
#if __GLIBC_PREREQ(2,13)
        if (name != NULL)
        {
            pthread_setname_np(tsk->pthrid, name);
        }
#endif
        pthread_detach(tsk->pthrid);
#if defined(LINUX_SIM) && __GLIBC_PREREQ(2,13)
        /* we don't need task_log(), gdb shows info */
#else
        task_log("task(%s)\ttid(%d)\tthread(%d)\n", name, t, tsk->pthrid);
#endif
    }

    return GT_OK;
}

/**
* @internal V2L_taskIdSelf function
* @endinternal
*
* @brief   returns the identifier of the calling task
*/
static int V2L_taskIdSelf(GT_VOID)
{
    int t;
    pthread_t pthrid;

    pthrid = pthread_self();
    for (t = 1; t < tasks.allocated; t++)
        if (TASK(t) && TASK(t)->header.type && TASK(t)->pthrid == pthrid)
            return t;
    return 0;
}

/*******************************************************************************
* CHECK_TID
*
* DESCRIPTION:
*       Check task Id and return pointer to task struct
*
* INPUTS:
*       tid         - task Id. Zero means calling task
*
* OUTPUTS:
*       tsk         - pointer to task struct
*
* RETURNS:
*       ERROR if error
*
*******************************************************************************/
#define CHECK_TID(tid) \
    _V2L_taskSTC *tsk; \
    if ((int)tid == 0) \
        tid = (GT_TASK)V2L_taskIdSelf(); \
    if (tid == 0) \
        return GT_FAIL; \
    if ((int)tid > tasks.allocated) \
        return GT_FAIL; \
    tsk = (_V2L_taskSTC*)(tasks.list[(int)tid]); \
    if (!tsk || !tsk->header.type) \
        return GT_FAIL; \
    

/**
* @internal osTaskDelete function
* @endinternal
*
* @brief   Deletes existing task.
*
* @param[in] tid                      - Task ID
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
* @note If tid = 0, delete calling task (itself)
*
*/
GT_STATUS osTaskDelete
(
    IN GT_TASK tid
)
{
    CHECK_TID(tid);
    pthread_mutex_lock(&tasks_mtx);

    tsk->header.type = 0;
    if (tsk->pthrid == pthread_self())
    {
        pthread_mutex_unlock(&tasks_mtx);
        pthread_exit(NULL);
        return GT_FAIL;
    }
    pthread_cancel(tsk->pthrid);
    pthread_mutex_unlock(&tasks_mtx);

    return GT_OK;
}

/**
* @internal osTaskSuspend function
* @endinternal
*
* @brief   Suspends existing task/thread.
*
* @param[in] tid                      - Task ID
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
* @note If tid = 0, suspend calling task (itself)
*
*/
GT_STATUS osTaskSuspend
(
    IN GT_TASK tid
)
{
    CHECK_TID(tid);
    /* force to unlock taskLock */
    V2L_taskUnlock_i(tsk->pthrid, 1);
    /* send SIGSTOP */
    pthread_kill(tsk->pthrid, SIGSTOP);
    return GT_OK;
}


/**
* @internal osTaskResume function
* @endinternal
*
* @brief   Resumes existing task/thread.
*
* @param[in] tid                      - Task ID
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS osTaskResume
(
    IN GT_TASK tid
)
{
    CHECK_TID(tid);
    pthread_kill(tsk->pthrid, SIGCONT);
    return GT_OK;
}


/**
* @internal osTaskGetSelf function
* @endinternal
*
* @brief   returns the current task (thread) id
*
* @param[out] tid                      -  the current task (thread) id
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - if parameter is invalid
*/
GT_STATUS osTaskGetSelf
(
    OUT GT_U32 *tid
)
{
    /* check validity of function arguments */
    if (tid == NULL)
        return GT_FAIL;
    *tid = (GT_U32)V2L_taskIdSelf();
    return GT_OK;
}


/**
* @internal osSetTaskPrior function
* @endinternal
*
* @brief   Changes priority of task/thread.
*
* @param[in] tid                      - Task ID
* @param[in] newprio                  - new priority of task
*
* @param[out] oldprio                  - old priority of task
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
* @note If tid = 0, change priotity of calling task (itself)
*
*/
GT_STATUS osSetTaskPrior
(
    IN  GT_TASK tid,
    IN  GT_U32  newprio,
    OUT GT_U32  *oldprio
)
{
    CHECK_TID(tid);
    if ( oldprio != NULL )
        *oldprio = (GT_U32)(tsk->vxw_priority);

    tsk->vxw_priority = newprio;
    /*!! set priority */
    return GT_OK;
}

/**
* @internal osGetTaskPrior function
* @endinternal
*
* @brief   Gets priority of task/thread.
*
* @param[in] tid                      - Task ID
*
* @param[out] prio                     - priority of task
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
* @note If tid = 0, gets priotity of calling task (itself)
*
*/
GT_STATUS osGetTaskPrior
(
    IN  GT_TASK tid,
    OUT GT_U32  *prio
)
{
    CHECK_TID(tid);
    if (prio == NULL)
        return GT_FAIL;

    *prio = (GT_U32)(tsk->vxw_priority);
    return GT_OK;
}


/**
* @internal osTaskLock function
* @endinternal
*
* @brief   Disable task rescheduling of current task.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS osTaskLock(GT_VOID)
{
    pthread_t self = pthread_self();
    pthread_mutex_lock(&taskLock_mtx);
    while (taskLock_count)
    {
        if (taskLock_owner == self)
            break;
        pthread_cond_wait(&taskLock_cond, &taskLock_mtx);
    }
    taskLock_count++;
    taskLock_owner = self;
    pthread_mutex_unlock(&taskLock_mtx);
    return GT_OK;
}

/**
* @internal V2L_taskUnlock_i function
* @endinternal
*
* @brief   Global mutex unlock
*         (was unlock the scheduller)
* @param[in] owner                    - pthread id of task which should unlock
* @param[in] force                    -  to unlock recursive locks
*
* @retval OK                       - on success
* @retval ERROR                    - on error
*/
static GT_VOID V2L_taskUnlock_i(
    IN  pthread_t   owner,
    IN  int         force
)
{
    pthread_mutex_lock(&taskLock_mtx);
    if (taskLock_owner == owner)
    {
        if (force)
            taskLock_count = 0;
        else
            taskLock_count--;
        if (taskLock_count == 0)
        {
            taskLock_owner = 0;
            pthread_cond_signal(&taskLock_cond);
        }
    }
    pthread_mutex_unlock(&taskLock_mtx);
}


/**
* @internal osTaskUnLock function
* @endinternal
*
* @brief   Enable task rescheduling.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS osTaskUnLock (GT_VOID)
{
    V2L_taskUnlock_i(pthread_self(), 0);

    return GT_OK;
}

#if defined(LINUX_SIM) && __GLIBC_PREREQ(2,13)
    /* we don't need task_log(), gdb shows info */
#else
/**
* @internal task_log function
* @endinternal
*
* @brief   logs the message to /tmp/tasks or to stderr
*/
static GT_VOID
    task_log(const GT_CHAR *fmt, ...)
{
    static FILE *fpLog = 0;
    /*
    **  v2pthread_file_lock is a mutex used to make open log file a critical section
    */
    static pthread_mutex_t
        v2pthread_file_lock = PTHREAD_MUTEX_INITIALIZER;
    va_list ap;

    pthread_mutex_lock( &v2pthread_file_lock );

    if (fpLog == 0)
    {
        /* Open log file to record thread creation */
        /* We need it for debugging */
#if !defined(LINUX_SIM) && !defined(__FreeBSD__)
#  define TASKS_FILE_NAME "/dev/shm/tasks"
#else
#  define TASKS_FILE_NAME "/tmp/tasks"
#endif
        fpLog = fopen(TASKS_FILE_NAME, "a");
        if (fpLog == 0)
        {
            fprintf(stderr,
                        "fopen(\"%s\", \"a\") failed, errno(%s)\n",
                        TASKS_FILE_NAME,
                        strerror(errno));
            /* fallback */
            fpLog = stderr;
        }
        else
        {
            /* log file should be line buffered */
            setbuf(fpLog, NULL);
        }
    }

    va_start( ap, fmt );
    vfprintf( fpLog, fmt, ap );
    va_end( ap );

    pthread_mutex_unlock( &v2pthread_file_lock );
}
#endif


