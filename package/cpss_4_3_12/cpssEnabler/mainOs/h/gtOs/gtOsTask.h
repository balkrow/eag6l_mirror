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
* @file gtOsTask.h
*
* @brief Operating System wrapper. Task facility.
*
* @version   13
********************************************************************************
*/

#ifndef __gtOsTaskh
#define __gtOsTaskh

#ifdef __cplusplus
extern "C" {
#endif

/************* Includes *******************************************************/

#include <gtOs/gtGenTypes.h>
#include <gtStack/gtStackTypes.h>

/************ Defines  ********************************************************/
#ifdef WIN32
#define __TASKCONV
#else
#define __TASKCONV
#endif
#define MV_DEFAULT_TIMEOUT 1000
/************* Typedefs *******************************************************/
#if (!defined __cmdExtServices_h_) || (defined PSS_PRODUCT)
typedef GT_U32  GT_TASK;
#endif
/************* Functions ******************************************************/

/**
* @internal osTaskCreate function
* @endinternal
*
* @brief   Create OS Task/Thread and start it.
*
* @param[in] name                     - task name, string no longer then OS_MAX_TASK_NAME_LENGTH
* @param[in] prio                     - task priority 255 - 0 => HIGH
* @param[in] stack                    - task Stack Size
*                                      start_addr - task Function to execute
*                                      arglist    - pointer to list of parameters for task function
* @param[out] tid                     - (pointer to )task id of the created task, will be used as a cookie
*                                       for referencing other task related APIs, e.g. osTaskDelete.
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
);

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
);

/**
* @internal osTaskDelete function
* @endinternal
*
* @brief   Deletes existing task/thread.
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
#define osTaskDelete CPSS_osTaskDelete
#endif
GT_STATUS osTaskDelete
(
    IN GT_TASK tid
);

/**
* @internal osTaskEnableJoin function
* @endinternal
*
* @brief   Enables join of a task at the time of task delete
           to ensure task is deleted before the execution
           continues
*
* @param[in] tid                      - Task ID
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
*/
GT_STATUS osTaskEnableJoin
(
    IN GT_TASK tid
);

/**
* @internal osTaskGracefulSelfRegister function
* @endinternal
*
* @brief   declare that thread supports graceful termination
*
* @param[in] support_graceful_termination - True - yes, False No
* @param[in] callBackFuncPtr - pointer to CB function
* @param[in] callBackFuncParam - parameter fo CB function
*
*
*/
GT_VOID osTaskGracefulCallerRegister(
    GT_TASK tid,
    GT_BOOL support_graceful_termination,
    void *callBackFuncPtr,
    void *callBackFuncParam
);


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
GT_BOOL osTaskGracefulGetTerminationReq(GT_TASK tid);

/**
* @internal osTaskGracefulTerminateSingleTask function
* @endinternal
*
* @brief   Gracefully terminates existing task.
*
* @param[in] tid                   - Task ID
*
* @note This fuction access tasks_mtx.
*
*/
GT_VOID osTaskGracefulTerminateSingleTask
(
    IN GT_TASK tid
);

/**
* @internal osTaskGracefulSetTerminationAck function
* @endinternal
*
* @brief   set thread termination ack
*
* @param[in] tid - self task id
*
* @retval None
*/
GT_VOID osTaskGracefulSetTerminationAck(GT_TASK tid);

/**
* @internal osTaskGracefulGetTerminationAck function
* @endinternal
*
* @brief   get thread termination_ack
*
*
* @retval GT_TRUE
* @retval GT_FALSE
*/
GT_BOOL osTaskGracefulGetTerminationAck(GT_TASK tid);

/**
* @internal osTaskPrintAllActiveTasks function
* @endinternal
*
* @brief   print all active tasks
*
*
*/
GT_VOID osTaskPrintAllActiveTasks(GT_VOID);

/**
* @internal osTaskTerminateAllActiveTasks function
* @endinternal
*
* @brief   iterate over all active tasks and terminate each.
*
*
* @retval GT_TRUE - all threads terminated
* @retval GT_FALSE - a thread has failed to terminate
*/
GT_STATUS osTaskTerminateAllActiveTasks(GT_VOID);

/**
* @internal osTaskSuspend function
* @endinternal
*
* @brief   Suspends existing task/thread.
*
* @param[in] tid                   - Task ID
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
);

/**
* @internal osTaskResume function
* @endinternal
*
* @brief   Resumes existing task/thread.
*
* @param[in] tid                   - Task ID
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS osTaskResume
(
    IN GT_TASK tid
);

/**
* @internal osTaskGetSelf function
* @endinternal
*
* @brief   returns the current task (thread) id
*
* @param[out] tid                  -  the current task (thread) id
*
* @retval GT_OK                    - on success.
* @retval GT_FAIL                  - if parameter is invalid
*/
GT_STATUS osTaskGetSelf
(
    OUT GT_TASK *tid
);

/**
* @internal osTaskGetId function
* @endinternal
*
* @brief   Returns current task id.
*
* @param[in] pthrid - pointer to pthread id.
*
* @retval id - Task ID on success
* @retval -1 - negative value on failure
*
*/
GT_32 osTaskGetId(IN GT_VOID *pthrid);

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
GT_BOOL osTaskIsSocketConnection(IN GT_VOID *pthrid);

/**
* @internal osTaskFdSet function
* @endinternal
*
* @brief   Set current thread stdio fd.
*
* @param[in] pthrid - pointer to pthread id. If NULL, use self.
*
*/
GT_VOID osTaskFdSet(IN GT_VOID *pthrid, GT_FD fd);

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
GT_FD osTaskFdGet(IN GT_VOID *pthrid);

/**
* @internal osSetTaskPrior function
* @endinternal
*
* @brief   Changes priority of task/thread.
*
* @param[in] tid                   - Task ID
* @param[in] newprio               - new priority of task
*
* @param[out] oldprio              - old priority of task
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
);

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
);

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
);

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
);

/**
* @internal CPSS_osTaskLock function
* @endinternal
*
* @brief   Disable task rescheduling of current task.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
#define osTaskLock CPSS_osTaskLock
GT_STATUS CPSS_osTaskLock (GT_VOID);

/**
* @internal CPSS_osTaskUnLock function
* @endinternal
*
* @brief   Enable task rescheduling.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/

#define osTaskUnLock CPSS_osTaskUnLock
GT_STATUS CPSS_osTaskUnLock (GT_VOID);

/**
* @internal osTaskRmonStart function
* @endinternal
*
* @brief   Start resource monitoring thread
*
* @param[in] date                      - CPSS build date
* @param[in] flags                     - CPSS build flags
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
*/
GT_STATUS osTaskRmonStart(const char *date, const char *flags);

#if !defined(WIN32) && defined(CONFIG_RMON)
    #define CPSS_ENABLER_OS_RMON_CREATE_ENTRY_CHECKPOINT(function)  \
        osRmonCreateEntryCheckpoint(__func__, function, __FILE__, __LINE__)

    #define CPSS_ENABLER_OS_RMON_CREATE_EXIT_CHECKPOINT(function)   \
        osRmonCreateExitCheckpoint(__func__, function, __FILE__, __LINE__)
#else
    #define CPSS_ENABLER_OS_RMON_CREATE_ENTRY_CHECKPOINT(function)

    #define CPSS_ENABLER_OS_RMON_CREATE_EXIT_CHECKPOINT(function)
#endif

/**
* @internal osRmonCreateEntryCheckpoint function
* @endinternal
*
* @brief   This function stores entry checkpoint data in rmon stat file
*
* @param[in] caller                - Caller to checkpoint
* @param[in] callee                - Callee to checkpoint
* @param[in] file                  - File to checkpoint
* @param[in] line                  - Line to checkpoint
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
*/
GT_STATUS osRmonCreateEntryCheckpoint(
    const GT_CHAR   *caller,
    const GT_CHAR   *callee,
    const GT_CHAR   *file,
    GT_U32          line
);

/**
* @internal osRmonCreateExitCheckpoint function
* @endinternal
*
* @brief   This function stores exit checkpoint data in rmon stat file
*
* @param[in] caller                - Caller to checkpoint
* @param[in] callee                - Callee to checkpoint
* @param[in] file                  - File to checkpoint
* @param[in] line                  - Line to checkpoint
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
*/
GT_STATUS osRmonCreateExitCheckpoint(
    const GT_CHAR   *caller,
    const GT_CHAR   *callee,
    const GT_CHAR   *file,
    GT_U32          line
);

/**
* @internal osSpawn function
* @endinternal
*
* @brief   Spawns the command
*
* @note Calls system() to execute the command
*
*/
GT_STATUS  osSpawn(char *cmd);


#ifdef __cplusplus
}
#endif

#endif  /* __gtOsTaskh */
/* Do Not Add Anything Below This Line */



