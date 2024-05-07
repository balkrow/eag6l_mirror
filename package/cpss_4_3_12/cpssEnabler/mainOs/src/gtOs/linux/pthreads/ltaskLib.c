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
#include <sys/resource.h>

#include <gtOs/gtOsTask.h>
#include <gtOs/osObjIdLib.h>
#include <gtOs/gtOsIo.h>
#include <gtOs/gtOsMem.h>
#include <gtOs/gtOsStr.h>
#include <gtOs/gtOsTimer.h>
#include <gtOs/globalDb/gtOsEnablerModGlobalSharedDb.h>
#include <gtOs/globalDb/gtOsGlobalDb.h>
#if __WORDSIZE == 64
#  ifdef ALL_ALLOCS_32BIT
#    include <sys/mman.h>
#  endif
#endif


/* for cascadig required */



#include <gtOs/gtOsSharedData.h>

#if (defined ASIC_SIMULATION_ENV_FORBIDDEN && defined ASIC_SIMULATION)
    /* 'Forbid' the code to run as ASIC_SIMULATION ... we need 'like' HW compilation ! */
    #undef ASIC_SIMULATION
#endif

#include "mainOsPthread.h"

/************* Defines ********************************************************/
#undef DIAG_PRINTFS

/* hash function for index calculation, 12 LSBs are constant in Linux */
#define V2L_PTHREAD_HASH_MAC(_pthreadId) (((_pthreadId) >> 12) & 0x1FF)
#define FAILED_TERMINATING_STR  "Failed terminating thread_id:"
#define V2L_PTHREAD_JOIN_ARRAY_SIZE_CNS 100
/************* Internal data **************************************************/
typedef GT_STATUS (*OS_TASK_GRACEFUL_CB_FNC)(void *);
#define PRV_SHARED_DB osSharedGlobalVarsPtr->cpssEnablerMod.mainOsDir.gtOsLtaskLibSrc

#define TASK(id) ((_V2L_taskSTC*)(PRV_SHARED_DB.tasks.list[id]))

/************ Forward declararions ********************************************/
#if defined(ASIC_SIMULATION) &&  defined(__GLIBC__) && defined(__GLIBC_MINOR__) && __GLIBC_PREREQ(2,13)
    /* we don't need task_log(), gdb shows info */
#else
static GT_VOID task_log(const GT_CHAR *fmt, ...);
#endif
static GT_VOID V2L_taskUnlock_i(
    IN  pthread_t   owner,
    IN  int         force
);
static void osTaskClearGracefulTerminationData(
    IN GT_U32 tid
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
    int t;

#ifdef SHARED_MEMORY
    if (!PRV_SHARED_DB.initialized)
    {
        CREATE_MTX(&PRV_SHARED_DB.tasks_mtx);
        CREATE_MTX(&PRV_SHARED_DB.taskLock_mtx);
        CREATE_COND(&PRV_SHARED_DB.taskLock_cond);
        PRV_SHARED_DB.initialized = 1;
    }
#endif
    DO_LOCK_MUTEX(&PRV_SHARED_DB.tasks_mtx);

#ifdef SHARED_MEMORY
    t = osObjLibGetNewObject(&PRV_SHARED_DB.tasks, 1, name?name:"tUsrRoot", &h,V2L_ts_malloc,V2L_ts_free);
#else
    t = osObjLibGetNewObject(&PRV_SHARED_DB.tasks, 1, name?name:"tUsrRoot", &h);
#endif

    if (t > 0)
    {
        tsk = (_V2L_taskSTC*)h;
        tsk->tid = t;
        tsk->pthrid = pthread_self();
    }

    pthread_mutex_unlock(&PRV_SHARED_DB.tasks_mtx);
    return 0;
}

/*****************************************************************************
**  cleanup_task_from_list ensures that a killed pthread cleans entry in task list
*****************************************************************************/
static void cleanup_task_from_list(void* arg)
{
    _V2L_taskSTC *tsk = (_V2L_taskSTC*)arg;
    GT_U32 hashIdx = V2L_PTHREAD_HASH_MAC(tsk->pthrid);

    DO_LOCK_MUTEX(&PRV_SHARED_DB.tasks_mtx);
    /*Check if thread has terminated on request or by itself and detach accordingly*/
    /*Do not detach thread if it is expected to support pthread_join*/
    if (!((tsk->termination_req && tsk->termination_ack) || (tsk->support_join)))
    {
        pthread_detach(tsk->pthrid);
    }

    if ((PRV_SHARED_DB.pthreadHashTbl[hashIdx].tid > 0) &&
        (PRV_SHARED_DB.pthreadHashTbl[hashIdx].pthrid == tsk->pthrid))
    {
        /* clean hash table */
        PRV_SHARED_DB.pthreadHashTbl[hashIdx].pthrid = 0;
        PRV_SHARED_DB.pthreadHashTbl[hashIdx].tid = 0;
        PRV_SHARED_DB.pthreadHashTbl[hashIdx].tsk = NULL;
    }
    /* clean graceful DB*/
    osTaskClearGracefulTerminationData(tsk->tid);
    tsk->pthrid = 0;
    tsk->tid = 0;
    tsk->header.type = 0;
    pthread_mutex_unlock(&PRV_SHARED_DB.tasks_mtx);
}

/*****************************************************************************
**  os_task_wrapper
*****************************************************************************/
static void* os_task_wrapper(void *arg)
{
    _V2L_taskSTC *tsk = (_V2L_taskSTC*)arg;

    if( tsk->sched == SCHED_OTHER)
    {
        errno = 0;
        if( -1 == nice(tsk->vxw_priority-120-nice(0)) )
        {
            if( 0 != errno )
            {
                return NULL;
            }
        }
    }

    pthread_cleanup_push(cleanup_task_from_list, (void*)tsk );

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
* @param[in] start_addr               - task Function to execute
* @param[in] arglist                  - pointer to list
*                                       parameters for task
*                                       function
* @param[out] tid                     - (pointer to )task id of the created task, will be used as a cookie
*                                       for referencing other task related APIs, e.g. osTaskDelete.
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
    prio = prio;
    return osTaskCreateAdvanced(name, SCHED_OTHER, 120, stack, start_addr, arglist, tid);
}

/**
* @internal osTaskCreateAdvanced function
* @endinternal
*
* @brief   Create OS Task/Thread and start it.
*
* @param[in] name                  - task name, string no longer then OS_MAX_TASK_NAME_LENGTH
* @param[in] sched                 - task scheduling policy
* @param[in] prio                  - task priority
* @param[in] stack                 - task Stack Size
* @param[in] start_addr            - task Function to execute
* @param[in] arglist               - pointer to list of parameters for task function
* @param[out] tid                  - (pointer to)task id of the created task, will be used as a cookie
*                                    for referencing other task related APIs, e.g. osTaskDelete.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
* @note Priority logic is that the lower value means higher priority.
* @note Based on Linux priority levels:
* @note The following scheduling policy definition used SCHED_RR, SCHED_FIFO &
* @note SCHED_OTHER are linux oriented and defined in Linux include file <sched.h>.
* @note SCHED_RR or SCHED_FIFO - 1-99
* @note SCHED_OTHER - prio 100-139 (equivalent to nice value (-20)-19)
* @note (if nice 0 is required, prio input parameter should be set to 120)
*/
GT_STATUS osTaskCreateAdvanced
(
    IN  const GT_CHAR *name,
    IN  GT_U32      sched,
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
    struct sched_param param;
    GT_U32      stack_min;
    GT_U32      hashIdx;
    int err;
#if !defined(ASIC_SIMULATION) && !defined(RTOS_ON_SIM)
    struct rlimit rlim;
#endif

    pthread_attr_init(&attrs);
    pthread_attr_setinheritsched(&attrs, PTHREAD_EXPLICIT_SCHED);

#if !defined(ASIC_SIMULATION) && !defined(RTOS_ON_SIM)
    stack_min = PTHREAD_STACK_MIN;
    err = getrlimit(RLIMIT_STACK, &rlim);
    if (err)
    {
        perror("osTaskCreateAdvanced: getrlimit");
        return GT_FAIL;
    }

    if (stack_min < rlim.rlim_cur)
    {
        stack_min = rlim.rlim_cur;
    }
#else /* simulation */
#ifdef GM_USED
    stack_min = 0x200000; /* 2M */
#else /* !GM_USED */
    stack_min = 0x20000; /* 128K */
#endif /* !GM_USED */

#endif

    if (stack)
    {
#ifdef INCLUDE_UTF
        /* The UTF allocates almost 1K in thread local storage
         * So stack size must be increased by this value
         */
        stack += 0x4000; /* 16K */
#endif

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

    DO_LOCK_MUTEX(&PRV_SHARED_DB.tasks_mtx);
#ifdef SHARED_MEMORY
    t = osObjLibGetNewObject(&PRV_SHARED_DB.tasks, 1, name, &h,V2L_ts_malloc,V2L_ts_free);
#else
    t = osObjLibGetNewObject(&PRV_SHARED_DB.tasks, 1, name, &h);
#endif
    if (t <= 0)
    {
        pthread_mutex_unlock(&PRV_SHARED_DB.tasks_mtx);
        return GT_FAIL;
    }
    tsk = (_V2L_taskSTC*)h;

    if (!name)
    {
        sprintf(tsk->header.name, "t%d", t);
    }

    if( (( SCHED_RR == sched || SCHED_FIFO == sched ) &&
         (( prio > 99 ) || (prio < 1))) ||
        (( SCHED_OTHER == sched ) &&
         (( prio > 139 ) || (prio < 100))) )
        {
            tsk->header.type = 0;
            pthread_mutex_unlock(&PRV_SHARED_DB.tasks_mtx);
            perror("osTaskCreateAdvanced: bad priority value");
            return GT_FAIL;
        }

    tsk->sched = sched;
    err = pthread_attr_setschedpolicy(&attrs, sched);
    if (err)
    {
        tsk->header.type = 0;
        pthread_mutex_unlock(&PRV_SHARED_DB.tasks_mtx);
        perror("osTaskCreateAdvanced: pthread_attr_setschedpolicy");
        return GT_FAIL;
    }

    tsk->vxw_priority = prio;
    if( SCHED_RR == sched || SCHED_FIFO == sched ) {
        prio = 100 - prio;

        err = pthread_attr_getschedparam(&attrs, &param);
        if (err)
        {
            tsk->header.type = 0;
            pthread_mutex_unlock(&PRV_SHARED_DB.tasks_mtx);
            perror("osTaskCreateAdvanced: pthread_attr_getschedpolicy");
            return GT_FAIL;
        }
        param.sched_priority = prio;
        err = pthread_attr_setschedparam(&attrs, &param);
        if (err)
        {
            tsk->header.type = 0;
            pthread_mutex_unlock(&PRV_SHARED_DB.tasks_mtx);
            perror("osTaskCreateAdvanced: pthread_attr_setschedpolicy");
            return GT_FAIL;
        }
    } else if ( SCHED_OTHER != sched ){
        tsk->header.type = 0;
        pthread_mutex_unlock(&PRV_SHARED_DB.tasks_mtx);
        perror("osTaskCreateAdvanced: wrong scheduling policy");
        return GT_FAIL;
    }

    tsk->entry_point = start_addr;
    tsk->param = arglist;

    osTaskClearGracefulTerminationData(t);
    tsk->tid = t;

    if (tid != NULL)
        *tid = (GT_TASK)t;

    if ( pthread_create( &(tsk->pthrid), &attrs,
                         os_task_wrapper, (void*)tsk) != 0 )
    {
        tsk->header.type = 0;
        pthread_mutex_unlock(&PRV_SHARED_DB.tasks_mtx);
#ifdef DIAG_PRINTFS
        perror( "\r\ntaskSpawn(): pthread_create returned error:" );
#endif
        return GT_FAIL;
    } else {
#if defined(__GLIBC__) && defined(__GLIBC_MINOR__) && __GLIBC_PREREQ(2,13)
        if (name != NULL)
        {
            pthread_setname_np(tsk->pthrid, name);
        }
#endif

#if (defined(ASIC_SIMULATION) &&  defined(__GLIBC__) && defined(__GLIBC_MINOR__) && __GLIBC_PREREQ(2,13))
        /*Threads are detached if needed just before termination in os_task_wrapper()*/
        /* we don't need task_log(), gdb shows info */
#else
        task_log("task(%s)\ttid(%d)\tthread(%d)\n", name, t, tsk->pthrid);
#endif
    }

    hashIdx = V2L_PTHREAD_HASH_MAC(tsk->pthrid);
    if (PRV_SHARED_DB.pthreadHashTbl[hashIdx].tid < 1)
    {
        /* add pthread id in hash table */
        PRV_SHARED_DB.pthreadHashTbl[hashIdx].tid = t;
        PRV_SHARED_DB.pthreadHashTbl[hashIdx].pthrid = tsk->pthrid;
        PRV_SHARED_DB.pthreadHashTbl[hashIdx].tsk = tsk;
    }
    pthread_mutex_unlock(&PRV_SHARED_DB.tasks_mtx);

    return GT_OK;
}

/**
* @internal osTasksInfo function
* @endinternal
*
* @brief   Returns detailed list of the currently running tasks
*
* @param[in] buff                     - Buffer to store the dump
* @param[in] size                     - Size of the buffer
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS osTasksInfo
(
    IN GT_CHAR *buff,
    IN GT_U32   size
)
{
    int t;
    GT_U32 curr_size = 0;
    _V2L_taskSTC *tsk;
    static const int line_size = NAME_MAX + 4 + 4 + 3; /* name + itid + tid + crlf */

    if (size < 1) {
        return GT_FAIL;
    }

    buff[0] = 0;
    curr_size += osSprintf(buff, "%s%-4d%-16s\n", buff, 1, "main");

    /* Print history of tasks */
    /* main thread is 1 */
    for (t = 2; t < PRV_SHARED_DB.tasks.allocated; t++)
    {

        if (curr_size + line_size > size) {
            return GT_FAIL;
        }
        if ((TASK(t) && TASK(t)->header.type))
        {/*if tsk is  active*/
            tsk = TASK(t);
            curr_size += osSprintf(buff, "%s%-4d%-16s\n", buff, t,
                               tsk->header.name);
        }

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
    GT_U32 hashIdx;

    if(NULL == osSharedGlobalVarsPtr)
    {
        return 0;
    }

    pthrid = pthread_self();

    hashIdx = V2L_PTHREAD_HASH_MAC(pthrid);
    if (PRV_SHARED_DB.pthreadHashTbl[hashIdx].pthrid == pthrid)
    {
        /* use hash table */
        return PRV_SHARED_DB.pthreadHashTbl[hashIdx].tid;
    }

    for (t = 1; t < PRV_SHARED_DB.tasks.allocated; t++)
        if (TASK(t) && TASK(t)->header.type && TASK(t)->pthrid == pthrid)
            return t;

    return 0;
}


/**
* @internal osTaskGetId function
* @endinternal
*
* @brief   Returns current task id.
*
* @param[in] pthrid - pointer to pthread id
*
* @retval id - Task ID on success
* @retval -1 - negative value on failure
*
*/
GT_32 osTaskGetId(IN GT_VOID *pthrid)
{
    _V2L_taskSTC *tsk;
    int id;

    if(NULL == osSharedGlobalVarsPtr)
    {
        return 0;
    }

    for (id=1; id<PRV_SHARED_DB.tasks.allocated ; id++)
    {
        if (TASK(id) && TASK(id)->header.type)
        {
            tsk = (_V2L_taskSTC*)PRV_SHARED_DB.tasks.list[id];
            if (pthread_equal(tsk->pthrid, *(pthread_t *)pthrid)) /*equal*/
            {
                return id;
            }
        }
    }
    return -1;
}


/**
* @internal osTaskFdSet function
* @endinternal
*
* @brief   Set current thread stdio fd.
*
* @param[in] pthrid - pointer to pthread id. If NULL, use self.
*
*/
GT_VOID osTaskFdSet(IN GT_VOID *pthrid, GT_FD fd)
{
    _V2L_taskSTC *tsk;
    int id;
    GT_U32 hashIdx;
    _V2L_pthread_hash_STC *hashBucket;
    pthread_t ownT;

    if (NULL == pthrid)
    {
        ownT = pthread_self();
        pthrid = &ownT;
    }

    if(NULL == osSharedGlobalVarsPtr)
    {
        return ;
    }

    hashIdx = V2L_PTHREAD_HASH_MAC((*(pthread_t *)pthrid));
    hashBucket = &PRV_SHARED_DB.pthreadHashTbl[hashIdx];
    if ( (pthread_equal(hashBucket->pthrid, *(pthread_t *)pthrid)) && (hashBucket->tsk) )
    {
        tsk = hashBucket->tsk;
        /* use hash table */
        if (pthread_equal(tsk->pthrid, *(pthread_t *)pthrid)) /*equal*/
        {
            tsk->redirStdoutFd = fd;
            return;
        }
    }

    for (id=1; id<PRV_SHARED_DB.tasks.allocated ; id++)
    {
        if (TASK(id) && TASK(id)->header.type)
        {
            tsk = (_V2L_taskSTC*)PRV_SHARED_DB.tasks.list[id];
            if (pthread_equal(tsk->pthrid, *(pthread_t *)pthrid)) /*equal*/
            {
                tsk->redirStdoutFd = fd;
            }
        }
    }
}


/**
* @internal osTaskFdGet function
* @endinternal
*
* @brief   Get current thread stdio fd.
*
* @param[in] pthrid - pointer to pthread id
*
* @retval id - Task fd on success
* @retval -1 - negative value on failure
*
*/
GT_FD osTaskFdGet(IN GT_VOID *pthrid)
{
    _V2L_taskSTC *tsk;
    int id;
    GT_U32 hashIdx;
    _V2L_pthread_hash_STC *hashBucket;

    if(NULL == osSharedGlobalVarsPtr)
    {
        return -1;
    }

    hashIdx = V2L_PTHREAD_HASH_MAC((*(pthread_t *)pthrid));
    hashBucket = &PRV_SHARED_DB.pthreadHashTbl[hashIdx];
    if ( (pthread_equal(hashBucket->pthrid, *(pthread_t *)pthrid)) && (hashBucket->tsk) )
    {
        tsk = hashBucket->tsk;
        /* use hash table */
        if (pthread_equal(tsk->pthrid, *(pthread_t *)pthrid)) /*equal*/
        {
            return tsk->redirStdoutFd;
        }
    }

    for (id=1; id<PRV_SHARED_DB.tasks.allocated ; id++)
    {
        if (TASK(id) && TASK(id)->header.type)
        {
            tsk = (_V2L_taskSTC*)PRV_SHARED_DB.tasks.list[id];
            if (pthread_equal(tsk->pthrid, *(pthread_t *)pthrid)) /*equal*/
            {
                return tsk->redirStdoutFd;
            }
        }
    }
    return -1;
}

/**
* @internal osTaskIsSocketConnection function
* @endinternal
*
* @brief   Returns GT_TRUE if current thread is a socket
*          connection service, GT_FALSE otherwise .
*
* @param[in] pthrid - pointer to pthread id.
*
*
*/
GT_BOOL osTaskIsSocketConnection (IN GT_VOID *pthrid)
{
    _V2L_taskSTC *tsk;
    int id;
    GT_U32 hashIdx;
    _V2L_pthread_hash_STC *hashBucket;

    if(NULL == osSharedGlobalVarsPtr)
    {
        return 0;
    }

    hashIdx = V2L_PTHREAD_HASH_MAC((*(pthread_t *)pthrid));
    hashBucket = &PRV_SHARED_DB.pthreadHashTbl[hashIdx];
    if ( (pthread_equal(hashBucket->pthrid, *(pthread_t *)pthrid)) && (hashBucket->tsk) )
    {
        tsk = hashBucket->tsk;
        /* use hash table */
        if (pthread_equal(tsk->pthrid, *(pthread_t *)pthrid)) /*equal*/
        {
            if (!strncmp(tsk->header.name,"CPSSGenServer",13))
                return GT_TRUE;
            else
                return GT_FALSE;
        }
    }

    for (id=1; id<PRV_SHARED_DB.tasks.allocated ; id++)
    {
        if (TASK(id) && TASK(id)->header.type)
        {
            tsk = (_V2L_taskSTC*)PRV_SHARED_DB.tasks.list[id];
            if (pthread_equal(tsk->pthrid, *(pthread_t *)pthrid)) /*equal*/
            {
                if (!strncmp(tsk->header.name,"CPSSGenServer",13))
                    return GT_TRUE;
                else
                    return GT_FALSE;
            }
        }
    }
    return GT_FALSE;
}

/**
* @internal osTaskGracefulSelfRegister function
* @endinternal
*
* @brief   declare thread supports graceful termination
*
* @param[in] support_graceful_termination - True - yes, False No
* @param[in] callBackFuncPtr - pointer to CB function
* @param[in] callBackFuncParam - parameter fo CB function
*
*
*/
GT_VOID osTaskGracefulCallerRegister(
    GT_U32 tid,
    GT_BOOL support_graceful_termination,
    void *callBackFuncPtr,
    void *callBackFuncParam
    )
{
    _V2L_taskSTC *tsk;
   if (!(TASK(tid) && TASK(tid)->header.type ))
    {
       osPrintf("osTaskGracefulCallerRegister has failed.\n"
                "\tinput is: tid-%d  SGT:%s CB_Func_PTR:%s  CB_Func_Param:%s\n",
                tid, (support_graceful_termination ? "TRUE" : "FALSE"), (callBackFuncPtr ? "Not-NULL" : "NULL"),
                (callBackFuncParam ? "Not-NULL" : "NULL"));
       return;
    }
    tsk = TASK(tid);
    DO_LOCK_MUTEX(&PRV_SHARED_DB.tasks_mtx);
    tsk->support_graceful_termination = support_graceful_termination;
    tsk->callBackFuncPtr = callBackFuncPtr;
    tsk->callBackParam = callBackFuncParam;
    pthread_mutex_unlock(&PRV_SHARED_DB.tasks_mtx);
}


/**
* @internal osTaskGracefulGetTerminationReq function
* @endinternal
*
* @brief   check self thread termination request
*
* @param[in] - tid, used to avoid redundant search.
*
* @retval GT_TRUE
* @retval GT_FALSE
*/
GT_BOOL osTaskGracefulGetTerminationReq
(
    IN GT_U32 tid
)
{
    GT_BOOL term_req = 0;

    if ((TASK(tid) && TASK(tid)->header.type && TASK(tid)->support_graceful_termination))
    {
        /*A mutex may be used here. Perfomence wise it was decided there is no need in mutex here*/
        term_req = TASK(tid)->termination_req;
    }
    return term_req;
}

/**
* @internal osTaskGracefulSetTerminationReq function
* @endinternal
*
* @brief   set thread termination request. This
*          should be called after securing the resource(mutex)
* @param[in] tid - thread identifier
*
*/
static GT_VOID osTaskGracefulSetTerminationReq(GT_32 tid)
{
    if (!(TASK(tid) && TASK(tid)->header.type && TASK(tid)->support_graceful_termination))
        osPrintf("Setting task, tid:%d  termination_req failed, skipping.\n", tid);
    else
        TASK(tid)->termination_req = GT_TRUE;
}

/**
* @internal osTaskGracefulSetTerminationAck function
* @endinternal
*
* @brief   set thread termination ack. No need for
*          mutex lock since the task is the only writer and
*          reader
*
*  @param[in] tid - self task id
*
* @retval None
*/
GT_VOID osTaskGracefulSetTerminationAck(GT_U32 tid)
{

    if (!(TASK(tid) && TASK(tid)->header.type && TASK(tid)->support_graceful_termination))
    {
        osPrintf("osTaskGracefulSetTerminationAck(tid:%d) failed.\n", tid);
        return;
    }
    TASK(tid)->termination_ack = GT_TRUE;
    osPrintf("Thread:%s has acknowledged termination\n ", TASK(tid)->header.name);
    return;
}


/**
* @internal osTaskGracefulGetTerminationAck function
* @endinternal
*
* @brief   get thread termination_ack. tasks_mtx should be
*          locked before calling this.
*
*
*
* @retval GT_TRUE
* @retval GT_FALSE
*/
GT_BOOL osTaskGracefulGetTerminationAck(GT_U32 tid)
{
    GT_BOOL term_ack;

    if (!(TASK(tid) && TASK(tid)->header.type && TASK(tid)->support_graceful_termination))
    {
        osPrintf("osTaskGracefulSetTerminationAck(tid:%d) failed.\n", tid);
        return 0;
    }
    term_ack = TASK(tid)->termination_ack;

    return term_ack;
}

/**
* @internal osTaskPrintAllActiveTasks function
* @endinternal
*
* @brief   print all active tasks
*
*
*/

GT_VOID osTaskPrintAllActiveTasks(GT_VOID)
{
    GT_32 i;
    _V2L_taskSTC *tsk;

    DO_LOCK_MUTEX(&PRV_SHARED_DB.tasks_mtx);
    osPrintf("\nAllocated threads: %d \n", PRV_SHARED_DB.tasks.allocated);
    for (i = 1; i < PRV_SHARED_DB.tasks.allocated; i++)
    {
        if ((TASK(i) && TASK(i)->header.type))
        {/*if tsk is  active*/
            tsk = TASK(i);
            osPrintf("\nthread_name:%s\n\ttid:%d pthread_id:%p index:%d\n"
                     , tsk->header.name, tsk->tid, tsk->pthrid, i);
            osPrintf("\tSGT:%d  Termination - REQ:%d  ACK:%d  \n",
                     tsk->support_graceful_termination, tsk->termination_req, tsk->termination_ack);
        }
    }

    pthread_mutex_unlock(&PRV_SHARED_DB.tasks_mtx);

}

/**
* @internal osGetTaskIdFromName function
* @endinternal
*
* @brief   find thread assign ID based on the task name
*
*
*/

GT_STATUS osGetTaskIdFromName(GT_CHAR *name, GT_TASK *tid)
{
    GT_32 i;
    _V2L_taskSTC *tsk;

    if( NULL == name || NULL == tid ) {
        return GT_BAD_PTR;
    }

    DO_LOCK_MUTEX(&PRV_SHARED_DB.tasks_mtx);
    for (i = 1; i < PRV_SHARED_DB.tasks.allocated; i++)
    {
        if ((TASK(i) && TASK(i)->header.type))
        {/*if tsk is  active*/
            tsk = TASK(i);
            if( 0 == osStrCmp(tsk->header.name, name) )
            {
                *tid = tsk->tid;
                pthread_mutex_unlock(&PRV_SHARED_DB.tasks_mtx);
                return GT_OK;
            }
        }
    }

    pthread_mutex_unlock(&PRV_SHARED_DB.tasks_mtx);

    return GT_NOT_FOUND;
}

/**
* @internal osTaskTerminateAllActiveTasks function
* @endinternal
*
* @brief   iterate over all active PRV_SHARED_DB.tasks and terminate each.
*
*
* @retval GT_TRUE - all threads terminated
* @retval GT_FALSE - a thread has failed to terminate, or out of memory
*/
GT_STATUS osTaskTerminateAllActiveTasks(GT_VOID)
{

    pthread_t self_pthrid;
    pthread_t  *pthreads_to_join;
    GT_32 self_index = -1;
    GT_32 tasks_entered = 0, tasks_closed = 0;
    GT_32 i, ii, ret;
    _V2L_taskSTC *tsk;


    pthreads_to_join = (pthread_t *)osMalloc(sizeof(pthread_t) * V2L_PTHREAD_JOIN_ARRAY_SIZE_CNS);
    if (!pthreads_to_join)
    {
        return GT_FAIL;
    }
    osMemSet(pthreads_to_join, 0, sizeof(pthread_t) * V2L_PTHREAD_JOIN_ARRAY_SIZE_CNS);
    self_pthrid = pthread_self();

    DO_LOCK_MUTEX(&PRV_SHARED_DB.tasks_mtx);
    for (i = 1; i < PRV_SHARED_DB.tasks.allocated; i++)
    {
        if ((TASK(i) && TASK(i)->header.type))/*if tsk is  active*/
        {
            tsk = TASK(i);
            if (tsk->pthrid == self_pthrid)
            {
                self_index = i;
                continue;
            }
            if (!(tsk->support_graceful_termination))
            {
                continue;
            }
            pthreads_to_join[tasks_entered++] = tsk->pthrid;
            if (tsk->callBackFuncPtr)
                tsk->callBackFuncPtr(tsk->callBackParam);

            osTaskGracefulSetTerminationReq(tsk->tid);
        }
    }/*for*/

    pthread_mutex_unlock(&PRV_SHARED_DB.tasks_mtx);
    if (self_index == (-1))
    {
        osPrintf("Terminating thread with thrid:%u was not found in tasks DB!\n", pthread_self());
        osFree(pthreads_to_join);
        return GT_FAIL;
    }
    usleep(1000 * 50);
    for (ii = 0; ii < 21; ii++)
    {
        if (usleep(MV_DEFAULT_TIMEOUT * 100 ) != 0) /*polling on threads status. every 100 [msec] */
        {
            osFree(pthreads_to_join);
            return GT_FAIL;
        }

        for (i = 0; i < tasks_entered; i++)
        {
            if ((pthreads_to_join[i] == 0))/* already closed*/
                continue;
            ret = pthread_tryjoin_np(pthreads_to_join[i],NULL);
            if (ret == 0)/*If joined*/
            {
                tasks_closed++;
                osPrintf("Successfully joined thrdid: %u with ret val:0x%x\n", pthreads_to_join[i], ret);
                pthreads_to_join[i] = 0;
                continue;
            }

        }/*for i*/
        if (tasks_closed == tasks_entered) /*Done joining all except self*/
                break;
    }/*for ii*/
    if (tasks_closed != tasks_entered )  /*closed all except self*/
    {
        osPrintf("Failed to terminate all threads.\n"
                 "Attempted to close %d tasks. Terminated %d tasks \n"
                 "Remaining threads:\n",  tasks_entered, tasks_closed);

        for (i = 0; i < tasks_entered; i++)
        {
            if ((pthreads_to_join[i] != 0))/* not joined*/
                osPrintf("\tThread_name:%s pthrid:%u\n",
                          TASK(osTaskGetId(&pthreads_to_join[i]))->header.name, pthreads_to_join[i]);
        }
        osFree(pthreads_to_join);
        return GT_FAIL;
    }
    osFree(pthreads_to_join);
    pthread_exit(NULL);
    return GT_FAIL; /*In case pthread_exit failed*/
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
    if ((int)tid > PRV_SHARED_DB.tasks.allocated) \
        return GT_FAIL; \
    tsk = (_V2L_taskSTC*)(PRV_SHARED_DB.tasks.list[(int)tid]); \
    if (!tsk || !tsk->header.type) \
        return GT_FAIL;


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
#ifdef PTP_PLUGIN_SUPPORT
GT_STATUS CPSS_osTaskDelete
#else
GT_STATUS osTaskDelete
#endif
(
    IN GT_TASK tid
)
{
    pthread_t   pthrid;
    GT_BOOL     join = GT_FALSE;
    void        *ret = NULL;
    int         rc;

    if(NULL == osSharedGlobalVarsPtr)
    {
        return GT_FAIL;
    }
    CHECK_TID(tid);
    pthrid = pthread_self();

    if (tsk->support_graceful_termination)
    {
        osTaskGracefulTerminateSingleTask(tid);
        if (tsk->header.type)
        {
            return GT_FAIL;
        }
        return GT_OK;
    }

    DO_LOCK_MUTEX(&PRV_SHARED_DB.tasks_mtx);
    tsk->header.type = 0;

    if (tsk->pthrid == pthrid)
    {
        pthread_mutex_unlock(&PRV_SHARED_DB.tasks_mtx);

 /*TBD - for shared library mode need to add for each task the  process id.
        If the task is last one left for process then pthread_exit should not be allowed,
        since the process  will stuck*/
 #ifndef SHARED_MEMORY
        pthread_exit(NULL);
 #endif
        return GT_FAIL;
    }

    pthrid = tsk->pthrid;
    join = tsk->support_join;

    pthread_cancel(tsk->pthrid);
    pthread_mutex_unlock(&PRV_SHARED_DB.tasks_mtx);

    if (join) {
        rc = pthread_join(pthrid, &ret);
        if(rc != 0) {
            fprintf(stderr, "osTaskDelete: pthread_join failed\n");
        }
    }

    return GT_OK;
}

/**
* @internal osTaskEnableJoin function
* @endinternal
*
* @brief   Enables join of a task at the time of task delete
           to ensure task is deleted before the execution
           continues
*
* @param[in] tid                   - Task ID
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
*/
GT_STATUS osTaskEnableJoin
(
    IN GT_TASK tid
)
{
    if(NULL == osSharedGlobalVarsPtr)
    {
        return GT_FAIL;
    }

    CHECK_TID(tid);

    tsk->support_join = GT_TRUE;
    return GT_OK;
}


/**
 * @internal osTaskClearGracefulTerminationData function
 * @endinternal
 *
 * @brief   Clear graceful task termination fields.
 * This should be called after resource is secured(mutex)
 *
 * @param tid - tid of the designated thread.
 *
 */
static void osTaskClearGracefulTerminationData(GT_U32 tid)
{
    if (!(TASK(tid) && TASK(tid)->header.type))
        return;

    TASK(tid)->termination_req = GT_FALSE;
    TASK(tid)->termination_ack = GT_FALSE;
    TASK(tid)->support_graceful_termination = GT_FALSE;
    TASK(tid)->callBackFuncPtr = NULL;
    TASK(tid)->callBackParam = NULL;
}

/**
* @internal osTaskGracefulTerminateSingleTask function
* @endinternal
*
* @brief   Gracefully terminates and joins existing task.
*
* @param[in] tid                   - Task ID

*
* @note This access tasks_mtx.
*
*/
GT_VOID osTaskGracefulTerminateSingleTask
(
    IN GT_TASK tid
)
{
    pthread_t pthrid;
    GT_U32 ii;
    GT_32 ret;
    _V2L_taskSTC *tsk;

    if (!(TASK(tid)) || !(TASK(tid)->header.type))
    {
        osPrintf("osTaskGracefulTerminateSingleTask received an invalid tid:%u.\n", tid);
        return;
    }
    tsk = (_V2L_taskSTC*)(PRV_SHARED_DB.tasks.list[(int)tid]);
    pthrid = tsk->pthrid;
    if (tsk->support_graceful_termination)
    {
        if (pthread_equal(pthread_self(), pthrid)) /*equal*/
        {
            pthread_exit(NULL);
            osPrintf("%s%u\n", FAILED_TERMINATING_STR, pthrid);
            return;
        }
        else/*not equal*/
        {
            DO_LOCK_MUTEX(&PRV_SHARED_DB.tasks_mtx);
            osTaskGracefulSetTerminationReq(tid);
            pthread_mutex_unlock(&PRV_SHARED_DB.tasks_mtx);
            for (ii = 0; ii < 12; ii++)
            {
                if (usleep(MV_DEFAULT_TIMEOUT *100 ) != 0) /*polling on threads status. every 100 [msec] */
                {
                    osPrintf("%s%u\n", FAILED_TERMINATING_STR, pthrid);
                    return;
                }
                ret = pthread_tryjoin_np(pthrid,NULL);
                if (ret == 0)/*If joined*/
                    {
                        osPrintf("Successfully joined thrdid: %u with ret val:0x%x\n", pthrid, ret);
                        return;
                    }
            }
            osPrintf("%s%u\n", FAILED_TERMINATING_STR, pthrid);
                    return;
        }
    }
    osPrintf("Thread:%s  thread_id:%u does not support graceful termination.\n", tsk->header.name, tsk->pthrid);
    return;
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
* @brief   returns the current task (thread) id, if found at task table
*          returns pthreads thread id if not found
*
* @param[out] tid                 - the current task (thread) id if GT_OK is returned,
*                                   or pthreads thread id if GT_NOT_FOUND is retuened
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - if parameter is invalid
* @retval GT_NOT_FOUND             - current task (thread) id not found in OS task array
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
    if (*tid == 0)
    {
        *tid = (GT_U32) pthread_self();
        return GT_NOT_FOUND;
    }
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
* @internal osSetTaskPriorAdvanced function
* @endinternal
*
* @brief   Changes scheduling policy and priority of task/thread.
*
* @param[in] tid                      - Task ID
* @param[in] newsched                 - new scheduling policy of task
* @param[in] newprio                  - new priority of task
*
* @param[out] oldsched                 - old scheduling policy of task
* @param[out] oldprio                  - old priority of task

*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
* @note If tid = 0, change priotity of calling task (itself)
* @note Operations on scheduling policy SCHED_OTHER can be done
* @note on in case tid = 0 (i.e. itself) since nice value
* @note manipulations can be done only within the thread itself.
* @note For description on the priority value please refer to
* @note the description in the osTaskCreateAdvanced API.
*/
GT_STATUS osSetTaskPriorAdvanced
(
    IN  GT_TASK tid,
    IN  GT_U32  newsched,
    IN  GT_U32  newprio,
    OUT GT_U32  *oldsched,
    OUT GT_U32  *oldprio
)
{
    struct sched_param param;
    int err, policy;
    CHECK_TID(tid);

    if (oldprio == NULL || oldsched == NULL)
        return GT_FAIL;
    *oldprio = (GT_U32)(tsk->vxw_priority);
    *oldsched = (GT_U32)(tsk->sched);

    /* If new setting is related to scheduling policy SCHED_OTHER */
    /* can be done only within the thread itself scope. */
    if( (SCHED_OTHER == newsched) && (tid != (GT_TASK)V2L_taskIdSelf()) )
    {
        perror("osSetTaskPriorAdvanced: nice can be set only for self thread");
        return GT_FAIL;
    }

    err = pthread_getschedparam(tsk->pthrid, &policy, &param);
    if (err)
    {
        perror("osSetTaskPriorAdvanced: pthread_getschedparam");
        return GT_FAIL;
    }

    if( (( SCHED_RR == newsched || SCHED_FIFO == newsched ) &&
         (( newprio > 99 ) || (newprio < 1))) ||
        (( SCHED_OTHER == newsched ) &&
         (( newprio > 139 ) || (newprio < 100))) )
    {
        perror("osSetTaskPriorAdvanced: bad priority value");
        return GT_FAIL;
    }

    if( SCHED_RR == newsched || SCHED_FIFO == newsched ) {
        param.sched_priority = 100-newprio;
    } else if ( SCHED_OTHER == newsched ) {
        errno = 0;
        err = nice(newprio-120-nice(0));
        if( err == -1 && errno != 0 )
        {
            perror("osSetTaskPriorAdvanced: update nice value falied");
            return GT_FAIL;
        }
        param.sched_priority = 0;
    } else {
        perror("osSetTaskPriorAdvanced: wrong scheduling policy");
        return GT_FAIL;
    }

    err = pthread_setschedparam(tsk->pthrid, newsched, &param);
    if (err)
    {
        perror("osSetTaskPriorAdvanced: pthread_setschedparam");
        return GT_FAIL;
    }

    tsk->vxw_priority = newprio;
    tsk->sched = newsched;

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
    GT_U32 sched;
    CHECK_TID(tid);
    if (prio == NULL)
        return GT_FAIL;

    return osGetTaskPriorAdvanced(tid, &sched, prio);
}

/**
* @internal osGetTaskPriorAdvanced function
* @endinternal
*
* @brief   Gets scheduling policy and priority of task/thread.
*
* @param[in] tid                   - Task ID
*
* @param[out] sched                - scheduling policy of task
* @param[out] prio                 - priority of task
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
* @note If tid = 0, gets priotity of calling task (itself)
*
*/
GT_STATUS osGetTaskPriorAdvanced
(
    IN  GT_TASK tid,
    OUT GT_U32  *sched,
    OUT GT_U32  *prio
)
{
    CHECK_TID(tid);
    if (sched == NULL || prio == NULL)
        return GT_FAIL;

    *sched = (GT_U32)(tsk->sched);
    *prio = (GT_U32)(tsk->vxw_priority);

    return GT_OK;
}


/**
* @internal CPSS_osTaskLock function
* @endinternal
*
* @brief   Disable task rescheduling of current task.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
* @note The following task lock implementation is not actually doing task lock
*       (i.e. prohibiting the preemption of this task), it is implementing a code
*       to protect critical section, based on Mutexes.
*       Also note that "pthread_cond_wait" enables atomic operation of signaling
*       one mutes and taking another one.
*
*/
GT_STATUS CPSS_osTaskLock(GT_VOID)
{
    if(NULL == osSharedGlobalVarsPtr)
    {
        return GT_FAIL;
    }
    pthread_t self = pthread_self();
    DO_LOCK_MUTEX(&PRV_SHARED_DB.taskLock_mtx);
    while (PRV_SHARED_DB.taskLock_count)
    {
        if (PRV_SHARED_DB.taskLock_owner == self)
            break;
        DO_COND_WAIT(&PRV_SHARED_DB.taskLock_cond, &PRV_SHARED_DB.taskLock_mtx);
    }
    PRV_SHARED_DB.taskLock_count++;
    PRV_SHARED_DB.taskLock_owner = self;
    pthread_mutex_unlock(&PRV_SHARED_DB.taskLock_mtx);
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
    if(NULL == osSharedGlobalVarsPtr)
    {
        return ;
    }

    DO_LOCK_MUTEX(&PRV_SHARED_DB.taskLock_mtx);
    if (PRV_SHARED_DB.taskLock_owner == owner)
    {
        if (force)
            PRV_SHARED_DB.taskLock_count = 0;
        else
            PRV_SHARED_DB.taskLock_count--;
        if (PRV_SHARED_DB.taskLock_count == 0)
        {
            PRV_SHARED_DB.taskLock_owner = 0;
            pthread_cond_signal(&PRV_SHARED_DB.taskLock_cond);
        }
    }
    pthread_mutex_unlock(&PRV_SHARED_DB.taskLock_mtx);
}


/**
* @internal CPSS_osTaskUnLock function
* @endinternal
*
* @brief   Enable task rescheduling.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS CPSS_osTaskUnLock (GT_VOID)
{
    V2L_taskUnlock_i(pthread_self(), 0);

    return GT_OK;
}

/**
* @internal osSpawn function
* @endinternal
*
* @brief   Spawns the command
*
* @note Calls system() to execute the command
*
*/
GT_STATUS osSpawn(char *cmd)
{
#ifndef ASIC_SIMULATION
    if (system(cmd) != 0)
    {
        return GT_FAIL;
    }
#endif

    return GT_OK;
}

#if defined(ASIC_SIMULATION) &&  defined(__GLIBC__) && defined(__GLIBC_MINOR__) && __GLIBC_PREREQ(2,13)
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
#if !defined(ASIC_SIMULATION) && !defined(__FreeBSD__)
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

/**************************************/
/* TASK CREATE & PRIORITY DEBUG APIs  */
/**************************************/
struct edebugTaskParams {
    int delayInSec;
    int sched;
    int prio;
};

/* Task's arguments that will configure a call to set task priority within the task. */
/* The input arguments are the triplet <time, schedule policy,priority>:             */
/* If <time> is different than 0, after <time> seconds after task was created a call */
/* to set task scheduling policy to <schedule policy> with priority <priority> will  */
/* be issued.                                                                        */
unsigned __TASKCONV edebugTaskDelayed( void* args )
{
    int ii = 1, jj, rc;
    int delayInSec, sched, prio;
    struct edebugTaskParams *taskParams = args;

    delayInSec = taskParams->delayInSec;
    sched = taskParams->sched;
    prio = taskParams->prio;

    if(taskParams == NULL)
    {
        return 1;
    }

    if( 0 != delayInSec ) {
        osPrintf("Request to set scheduling policy to %s, priority to %d, after %d sec.\n",
                  (sched == 0)?"SCHED_OTHER":((sched == 1)?"SCHED_FIFO":((sched == 2)?"SCHED_RR":"SCHED_UNKNOWN")),
                  prio, delayInSec);
    }

    while (ii)
    {
        for(jj=0; jj < 1000000; jj++)
        {
            if( delayInSec != 0 )
            {
                if(delayInSec * 10 == jj)
                {
                    GT_U32 oldsched, oldprio;

                    rc = osSetTaskPriorAdvanced(0, sched, prio, &oldsched, &oldprio);
                    if (rc != 0)
                    {
                        osPrintf("edebugTask:osSetTaskPriorAdvanced Failed, rc 0x%x\n", rc);
                    } else {
                        osPrintf("edebugTask:osSetTaskPriorAdvanced Done!\n");
                    }
                    delayInSec = 0;
                }
                /* osTimerWkAfter(100); */
            }
            osTimerWkAfter(100);
        }
    }

    return 0;
}

/* A wrapper for task creation.                                                                  */
/* The first 3 parameters are:                                                                   */
/* name - task name, a string, to identify it in tasks DB and on various linux utilities output. */
/* sched - the scheduler that the task created will be added to:                                 */
/*         0/1/2 for SCHED_OTHER/ SCHED_FIFO/SCHED_RR respectively.                              */
/* prio - the priority that the task will be created within the scheduler selected.              */
/* (See additional descriptions related to scheduling policy and priority values in              */
/* "osTaskCreateAdvanced" header).                                                               */
/* The other 3 parameters, the triplet <delayInSec, lsched, lprio> are the argument passed to    */
/* the task created - see detailed description at header of "edebugTaskDelayed".                 */
GT_STATUS edebugTaskCreate(GT_CHAR *name, GT_U32 sched, GT_U32 prio, int delayInSec, int lsched, int lprio)
{
    GT_STATUS rc;
    GT_TASK lTid;
    struct edebugTaskParams taskParams;

    taskParams.delayInSec = delayInSec;
    taskParams.sched = lsched;
    taskParams.prio = lprio;

    rc = osTaskCreateAdvanced(name,
                              sched,
                              prio,
                              0x4000,
                              edebugTaskDelayed,
                              &taskParams,
                              &lTid);
    if (rc != GT_OK)
    {
        osPrintf("edebugTaskCreate:osTaskCreateAdvanced Failed, rc 0x%x\n", rc);
        return rc;
    } else {
        osPrintf("edebugTaskCreate: Task \"%s\", tid 0x%x\n", name, lTid);
    }

    rc = osGetTaskIdFromName(name, &lTid);
    if (rc != GT_OK)
    {
        osPrintf("edebugTaskCreate:osGetTaskIdFromName Failed, rc 0x%x\n", rc);
    } else {
        osPrintf("edebugTaskCreate:osGetTaskIdFromName tid 0x%x\n", lTid);
    }

    return rc;
}

/* A wrapper for set task scheduling policy & priority.                 */
/* name - task name given when created                                  */
/* newsched - the new scheduler requested                               */
/*            0/1/2 for SCHED_OTHER/ SCHED_FIFO/SCHED_RR respectively.  */
/* newprio - the new priority requested                                 */
GT_STATUS edebugSetTaskPrior(GT_CHAR *name, GT_U32 newsched,GT_U32 newprio)
{
    GT_STATUS rc;
    GT_TASK lTid;
    GT_U32  oldsched,oldprio;

    rc = osGetTaskIdFromName(name, &lTid);
    if (rc != GT_OK)
    {

        osPrintf("edebugSetTaskPrior:osGetTaskIdFromName Failed, rc 0x%x\n", rc);
        return rc;
    }  else {
        osPrintf("edebugSetTaskPrior:osGetTaskIdFromName tid 0x%x\n", lTid);
    }

    rc = osSetTaskPriorAdvanced(lTid, newsched, newprio, &oldsched, &oldprio);
    if (rc != GT_OK)
    {
        osPrintf("edebugSetTaskPrior:osSetTaskPriorAdvanced Failed, rc 0x%x\n", rc);
    } else {
        osPrintf("edebugSetTaskPrior: previous scheduling policy %s, previous prioriry %d\n",
                 (oldsched == 0)?"SCHED_OTHER":((oldsched == 1)?"SCHED_FIFO":((oldsched == 2)?"SCHED_RR":"SCHED_UNKNOWN")),
                  oldprio);
    }

    return rc;
}

/* A wrapper for get task scheduling policy & priority. */
/* name - task name given when created                  */
GT_STATUS edebugGetTaskPrior(GT_CHAR *name)
{
    GT_STATUS rc;
    GT_TASK lTid;
    GT_U32  sched, prio;

    rc = osGetTaskIdFromName(name, &lTid);
    if (rc != GT_OK)
    {
        osPrintf("edebugGetTaskPrior:osGetTaskIdFromName Failed, rc 0x%x\n", rc);
        return rc;
    }

    rc = osGetTaskPriorAdvanced(lTid, &sched, &prio);
    if (rc != GT_OK)
    {
        osPrintf("edebugGetTaskPrior:osGetTaskPriorAdvanced Failed, rc 0x%x\n", rc);
    } else {
        osPrintf("edebugGetTaskPrior: scheduling policy %s, prioriry %d\n",
                 (sched == 0)?"SCHED_OTHER":((sched == 1)?"SCHED_FIFO":((sched == 2)?"SCHED_RR":"SCHED_UNKNOWN")),
                  prio);
    }

    return rc;
}

/* A wrapper for task deletion. */
/* name - task name given when created                  */
GT_STATUS edebugTaskDelete(GT_CHAR *name)
{
    GT_STATUS rc;
    GT_TASK lTid;

    rc = osGetTaskIdFromName(name, &lTid);
    if (rc != GT_OK)
    {
        osPrintf("edebugTaskDelete:osGetTaskIdFromName Failed, rc 0x%x\n", rc);
        return rc;
    }  else {
        osPrintf("edebugTaskDelete:osGetTaskIdFromName tid 0x%x\n", lTid);
    }

    rc = osTaskDelete(lTid);
    if (rc != GT_OK)
    {
        printf("edebugTaskDelete:osTaskDelete Failed, rc 0x%x\n", rc);
    }

    return rc;
}
