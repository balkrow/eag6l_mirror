/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
* THIS CODE IS A REFERENCE CODE FOR MARVELL SWITCH PRODUCTS.  IT IS PROVIDED   *
* "AS IS" WITH NO WARRANTIES, EXPRESSED, IMPLIED OR OTHERWISE, REGARDING ITS   *
* ACCURACY, COMPLETENESS OR PERFORMANCE.                                       *
* CUSTOMERS ARE FREE TO MODIFY IT AND USE IT ONLY IN THEIR PRODUCTION          *
* SOFTWARE RELEASES WITH MARVELL SWITCH CHIPSETS.                              *
*******************************************************************************/
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include "kernelExt.h"
#include <gtOs/gtOsTask.h>

#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <limits.h>
#include <pthread.h>


#if defined(_syscall0)
    _syscall0(int,gettid)
#else /* !defined _syscall0 */
#  include <sys/syscall.h>
#  include <unistd.h>
#  define   gettid() syscall(SYS_gettid)
#endif /* !defined(_syscall0) */

#if (defined ASIC_SIMULATION_ENV_FORBIDDEN && defined ASIC_SIMULATION)
    /* 'Forbid' the code to run as ASIC_SIMULATION ... we need 'like' HW compilation ! */
    #undef ASIC_SIMULATION
#endif


/************ Defines  ********************************************************/

#define VXW_DEF_STACK_SIZE  0x2000

/************ Internal Typedefs ***********************************************/

struct task_wrapper_stc {
    pthread_mutex_t mtx;
    pthread_cond_t cond;
    const GT_CHAR *taskname;
    int         vxw_priority;
    unsigned    (__TASKCONV *entry_point)(GT_VOID*);
    GT_VOID     *param;
    GT_TASK     *tid;
};


/************ Internal Functions **********************************************/

/*****************************************************************************
**  cleanup_scheduler_lock ensures that a killed pthread releases the
**                         scheduler lock if it owned it.
*****************************************************************************/
static GT_VOID
    cleanup_scheduler_lock( GT_VOID *unused )
{
    mv_ctrl(MVKERNELEXT_IOC_UNREGISTER, 0);
}

/*****************************************************************************
**  V2L_task_wrapper
*****************************************************************************/
static GT_VOID *
    V2L_task_wrapper( GT_VOID *arg )
{
    struct task_wrapper_stc *params;
    unsigned    (__TASKCONV *entry_point)( GT_VOID* );
    GT_VOID*     param;

    /*
    **  Ensure that errno for this thread is cleared.
    */
    errno = 0;

    /*
    **  Make a parameter block pointer from the caller's argument
    **  Then extract the needed info from the parameter block and
    **  free its memory before beginning the v2pthread task
    */
    params = (struct task_wrapper_stc*)arg;
    pthread_mutex_lock(&(params->mtx));

    entry_point = params->entry_point;
    param = params->param;


    if (params->tid)
        *(params->tid) = gettid();

    {
        mv_registertask_stc rparam;

        if ( params->taskname == (GT_CHAR *)NULL )
        {
            /*
            ** Synthesize a default task name if none specified
            */
            sprintf(rparam.name, "t%ld", gettid() );
        }
        else
        {
            strncpy(rparam.name, params->taskname, sizeof(rparam.name));
        }

        /* register task in kernel */
        rparam.vxw_priority = params->vxw_priority;
        rparam.pthread_id = pthread_self();
        mv_ctrl(MVKERNELEXT_IOC_REGISTER, &rparam);
    }

    /* set priority */
    {
        mv_priority_stc priority_params;
        priority_params.taskid = 0;
        priority_params.vxw_priority = params->vxw_priority;
        mv_ctrl(MVKERNELEXT_IOC_SET_PRIO, &priority_params);
    }

    /* all data ready, signal to parent */
    pthread_cond_signal(&(params->cond));
    pthread_mutex_unlock(&(params->mtx));

    /*
    **  Ensure that this pthread will release the scheduler lock if killed.
    */
    pthread_cleanup_push( cleanup_scheduler_lock, NULL );

    /*
    **  Call the v2pthread task.  Normally this is an endless loop and doesn't
    **  return here.
    */

    entry_point(param);


    /*
    **  If for some reason the task above DOES return, clean up the
    **  pthread and task resources and kill the pthread.
    */
    pthread_cleanup_pop( 1 );

    mv_ctrl(MVKERNELEXT_IOC_UNREGISTER, 0);

    return( (GT_VOID *)NULL );
}


/************ Public Functions ************************************************/

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
    IN  const GT_CHAR *name,
    IN  GT_U32      prio,
    IN  GT_U32      stack,
    IN  unsigned    (__TASKCONV *start_addr)(GT_VOID*),
    IN  GT_VOID     *arglist,
    OUT GT_TASK     *tid
)
{
    struct task_wrapper_stc params;
    pthread_t pthrid;
    pthread_attr_t  attrs;

    if (stack == 0)
      stack = VXW_DEF_STACK_SIZE;

    stack &= ~0x01UL;




    pthread_mutex_init(&(params.mtx), NULL);
    pthread_cond_init(&(params.cond), NULL);
    pthread_mutex_lock(&(params.mtx));
    params.taskname = name;
    params.vxw_priority = prio;
    params.entry_point = start_addr;
    params.param = arglist;
    params.tid = tid;


    pthread_attr_init(&attrs);

#if (!defined LINUX || !defined ASIC_SIMULATION) && !defined(RTOS_ON_SIM)
    if (stack)
    {
#ifdef INCLUDE_UTF
        /* The UTF allocates almost 1K in thread local storage
         * So stack size must be increased by this value
         */
        stack += 0x4000; /* 16K */
#endif

        if (stack < PTHREAD_STACK_MIN)
            stack = PTHREAD_STACK_MIN;
        pthread_attr_setstacksize(&attrs, stack);
    }
#else
    {
        size_t  def_stksize;
        pthread_attr_getstacksize( &attrs, &def_stksize);
        if (def_stksize < stack)
        {
            pthread_attr_setstacksize( &attrs, stack);
        }
    }
#endif

    if ( pthread_create( &pthrid, &attrs,
                         V2L_task_wrapper, (GT_VOID *)&params ) != 0 )
    {
#ifdef DIAG_PRINTFS
        perror( "\r\ntaskActivate pthread_create returned error:" );
#endif
        return GT_FAIL;
    }
    else
    {
        pthread_detach(pthrid);
        pthread_cond_wait(&(params.cond), &(params.mtx));
    }

#if __GLIBC_PREREQ(2,13)
    if (name != NULL)
    {
        pthread_setname_np(pthrid, name);
    }
#endif
    pthread_mutex_unlock(&(params.mtx));
    pthread_mutex_destroy(&(params.mtx));
    pthread_cond_destroy(&(params.cond));

    return GT_OK;
}


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
    mv_get_pthrid_stc params;
    pthread_t pthread_id;
    params.taskid = (int)tid;
    if (mv_ctrl(MVKERNELEXT_IOC_GET_PTHRID, &params) != 0)
        return GT_FAIL;
    pthread_id = (pthread_t)params.pthread_id;
    if (pthread_id == pthread_self())
    {
        pthread_exit(NULL);
        return GT_FAIL;
    }

    pthread_cancel(pthread_id);
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
    int ret;
    ret = mv_ctrl(MVKERNELEXT_IOC_SUSPEND, (int)tid);

    if (ret != 0)
        return GT_FAIL;

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
    int ret;
    ret = mv_ctrl(MVKERNELEXT_IOC_RESUME, (int)tid);

    if (ret != 0)
        return GT_FAIL;

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
    *tid = (GT_U32)gettid();
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
    mv_priority_stc params;

    params.taskid = (int)tid;

    if (oldprio != NULL)
    {
        if (mv_ctrl(MVKERNELEXT_IOC_GET_PRIO, &params) == 0)
            *oldprio = (GT_U32)params.vxw_priority;
    }

    params.vxw_priority = newprio;
    if (mv_ctrl(MVKERNELEXT_IOC_SET_PRIO, &params) != 0)
        return GT_FAIL;

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
    mv_priority_stc params;

    if (prio == NULL)
        return GT_FAIL;

    params.taskid = (int)tid;

    if (mv_ctrl(MVKERNELEXT_IOC_GET_PRIO, &params) != 0)
        return GT_FAIL;

    *prio = (GT_U32)params.vxw_priority;

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
    while (mv_tasklock() < 0)
    {
        if (errno != MVKERNELEXT_EINTR)
            break;
    }

    return GT_OK;
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
    mv_taskunlock();

    return GT_OK;
}


