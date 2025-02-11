/*******************************************************************************
* osTestPerf
*
* DESCRIPTION:
*       Test performance facility.
*
* DEPENDENCIES:
*       Non win32 , CPU independed , and posix threads implementation.
*
* FILE REVISION NUMBER:
*       $Revision: 3 $
*******************************************************************************/

#include <string.h>
#include <gtOs/gtOsSem.h>
#include <gtOs/gtOsTask.h>
#include <gtOs/gtOsTimer.h>
#include <gtOs/gtOsIo.h>
#include <gtOs/gtOsMsgQ.h>

#include <gtOs/globalDb/gtOsGlobalDb.h>
#include <gtOs/globalDb/gtOsGlobalDbInterface.h>

/*global variables macros*/

#define PRV_SHARED_MAIN_OS_DIR_OS_TEST_PERF_SRC_GLOBAL_VAR_SET(_var,_value)\
    PRV_HELPER_SHARED_GLOBAL_VAR_SET(cpssEnablerMod.mainOsDir.osTestPrefSrc._var,_value)

#define PRV_SHARED_MAIN_OS_DIR_OS_TEST_PERF_SRC_GLOBAL_VAR_GET(_var)\
    PRV_HELPER_SHARED_GLOBAL_VAR_GET(cpssEnablerMod.mainOsDir.osTestPrefSrc._var)

#define PERF_COUNT 1000000
#define P_MESSAGE_SIZE  64
#define P_MSGQ_BUFFER   128

/************ Private Functions ***********************************************/
static int time_gt(GT_U32 a_s, GT_U32 a_n, GT_U32 b_s, GT_U32 b_n)
{
    if (a_s > b_s)
        return 1;
    if (a_s < b_s)
        return 0;
    return (a_n > b_n) ? 1 : 0;
}
static void time_sub(GT_U32 a_s, GT_U32 a_n, GT_U32 b_s, GT_U32 b_n, GT_U32 *d_s, GT_U32 *d_n)
{
    if (time_gt(a_s, a_n, b_s, b_n))
    {
        *d_s = a_s - b_s;
        if (a_n >= b_n)
            *d_n = a_n - b_n;
        else
        {
            *d_n = 1000000000 + a_n - b_n;
            (*d_s)--;
        }
    } else
    {
        *d_s = 0;
        *d_n = 0;
    }
}

#define TIMING_START() \
    osTimeRT(&PRV_SHARED_MAIN_OS_DIR_OS_TEST_PERF_SRC_GLOBAL_VAR_GET(start_s), \
             &PRV_SHARED_MAIN_OS_DIR_OS_TEST_PERF_SRC_GLOBAL_VAR_GET(start_n));

#define TIMING_PRINT(desc,count) \
    osTimeRT(&now_s, &now_n); \
    time_sub(now_s, now_n, \
             PRV_SHARED_MAIN_OS_DIR_OS_TEST_PERF_SRC_GLOBAL_VAR_GET(start_s), \
             PRV_SHARED_MAIN_OS_DIR_OS_TEST_PERF_SRC_GLOBAL_VAR_GET(start_n), \
             &delta_s, &delta_n); \
    osPrintf("%s: count=%d time=%d.%09d seconds\n", \
            desc, count, delta_s, delta_n);

#define TIMING_STARTL() \
    osTimeRT(&start_s, &start_n);

#define TIMING_PRINTL(desc,count) \
    osTimeRT(&now_s, &now_n); \
    time_sub(now_s, now_n, start_s, start_n, &delta_s, &delta_n); \
    osPrintf("%s: count=%d time=%d.%09d seconds\n", \
            desc, count, delta_s, delta_n);

static unsigned __TASKCONV tsk1(void* unused)
{
    GT_U32      now_s, now_n, delta_s, delta_n;

    (void)unused;
    PRV_SHARED_MAIN_OS_DIR_OS_TEST_PERF_SRC_GLOBAL_VAR_SET(t_count,0);
    /* wait till second task started */
    osSemWait(PRV_SHARED_MAIN_OS_DIR_OS_TEST_PERF_SRC_GLOBAL_VAR_GET(t_sem2), OS_WAIT_FOREVER);

    TIMING_START();
    while (PRV_SHARED_MAIN_OS_DIR_OS_TEST_PERF_SRC_GLOBAL_VAR_GET(t_count) < PERF_COUNT)
    {
        osSemSignal(PRV_SHARED_MAIN_OS_DIR_OS_TEST_PERF_SRC_GLOBAL_VAR_GET(t_sem1));
        osSemWait(PRV_SHARED_MAIN_OS_DIR_OS_TEST_PERF_SRC_GLOBAL_VAR_GET(t_sem2), OS_WAIT_FOREVER);
        PRV_SHARED_MAIN_OS_DIR_OS_TEST_PERF_SRC_GLOBAL_VAR_GET(t_count)++;
    }
    TIMING_PRINT("task switch by sem",PRV_SHARED_MAIN_OS_DIR_OS_TEST_PERF_SRC_GLOBAL_VAR_GET(t_count));

    osSemSignal(PRV_SHARED_MAIN_OS_DIR_OS_TEST_PERF_SRC_GLOBAL_VAR_GET(t_sem1));
    osSemSignal(PRV_SHARED_MAIN_OS_DIR_OS_TEST_PERF_SRC_GLOBAL_VAR_GET(t_semE));
    return 0;
}

static unsigned __TASKCONV tsk2(void* unused)
{
    (void)unused;

    PRV_SHARED_MAIN_OS_DIR_OS_TEST_PERF_SRC_GLOBAL_VAR_SET(t_count,0);
    /* signal tsk1: tsk2 ready */
    osSemSignal(PRV_SHARED_MAIN_OS_DIR_OS_TEST_PERF_SRC_GLOBAL_VAR_GET(t_sem2));

    while (PRV_SHARED_MAIN_OS_DIR_OS_TEST_PERF_SRC_GLOBAL_VAR_GET(t_count) < PERF_COUNT)
    {
        osSemWait(PRV_SHARED_MAIN_OS_DIR_OS_TEST_PERF_SRC_GLOBAL_VAR_GET(t_sem1), OS_WAIT_FOREVER);
        PRV_SHARED_MAIN_OS_DIR_OS_TEST_PERF_SRC_GLOBAL_VAR_GET(t_count)++;
        osSemSignal(PRV_SHARED_MAIN_OS_DIR_OS_TEST_PERF_SRC_GLOBAL_VAR_GET(t_sem2));
    }

    osSemSignal(PRV_SHARED_MAIN_OS_DIR_OS_TEST_PERF_SRC_GLOBAL_VAR_GET(t_sem2));
    osSemSignal(PRV_SHARED_MAIN_OS_DIR_OS_TEST_PERF_SRC_GLOBAL_VAR_GET(t_semE));
    return 0;
}


static unsigned __TASKCONV msgqrcv(void* param)
{
    GT_MSGQ_ID  msgq = (GT_MSGQ_ID)param;
    int         r_count = 0;
    GT_U32      now_s, now_n, delta_s, delta_n;

    /* signal task is ready */
    osSemSignal(PRV_SHARED_MAIN_OS_DIR_OS_TEST_PERF_SRC_GLOBAL_VAR_GET(t_semE));

    while (r_count < PERF_COUNT)
    {
        char buf[P_MESSAGE_SIZE];
        GT_U32  msgSize;
        int t;
        msgSize = P_MESSAGE_SIZE;
        if (osMsgQRecv(msgq, buf, &msgSize, OS_MSGQ_WAIT_FOREVER) != GT_OK)
        {
            osPrintf("osMsgQ returned != GT_OK\n");
            break;
        }
        if (msgSize != P_MESSAGE_SIZE)
        {
            osPrintf("(msgSize==%d) != %d\n",msgSize,P_MESSAGE_SIZE);
            break;
        }
        memcpy(&t, buf, 4);
        if (t != r_count)
        {
            osPrintf("bad message: expected: %d, got: %d\n", r_count, t);
            break;
        }
        r_count++;
    }
    TIMING_PRINT("osMsgQ",r_count);
    osPrintf(" msgSize=%d buffer=%dmsgs\n", P_MESSAGE_SIZE, P_MSGQ_BUFFER);


    /* task finished */
    osSemSignal(PRV_SHARED_MAIN_OS_DIR_OS_TEST_PERF_SRC_GLOBAL_VAR_GET(t_semE));
    return 0;
}
/************ Public Functions ************************************************/
#if 1
GT_STATUS prvCpssDrvHwPpPortGroupReadRegister
(
    IN GT_U8    devNum,
    IN GT_U32   portGroupId,
    IN GT_U32   regAddr,
    OUT GT_U32  *data
);
#endif

/**
* @internal osTestPerf function
* @endinternal
*
* @brief   run performance tests
*/
GT_STATUS osTestPerf(void)
{
    GT_U32      start_s, start_n, now_s, now_n, delta_s, delta_n;
    GT_MUTEX    mtx;
    GT_SEM      sem;
    int         k;
    GT_TASK     tid;
    GT_MSGQ_ID  msgq;

    TIMING_STARTL();
    for (k = 0; k < PERF_COUNT; k++)
    {
        osTaskLock();
        osTaskUnLock();
    }
    TIMING_PRINTL("osTaskLock/osTaskUnLock (single thread)",k);

    osMutexCreate("testPerf", &mtx);
    TIMING_STARTL();
    for (k = 0; k < PERF_COUNT; k++)
    {
        osMutexLock(mtx);
        osMutexUnlock(mtx);
    }
    TIMING_PRINTL("osMutexLock/osMutexUnlock (single thread)",k);
    osMutexDelete(mtx);

    osSemBinCreate("testPerf", OS_SEMB_FULL, &sem);
    TIMING_STARTL();
    for (k = 0; k < PERF_COUNT; k++)
    {
        osSemWait(sem, OS_WAIT_FOREVER);
        osSemSignal(sem);
    }
    TIMING_PRINTL("binary semaphore osSemWait/osSemSignal (single thread)",k);
    osSemDelete(sem);

    /* task switch test */
    osSemBinCreate("testPerf1", OS_SEMB_EMPTY, &PRV_SHARED_MAIN_OS_DIR_OS_TEST_PERF_SRC_GLOBAL_VAR_GET(t_sem1));
    osSemBinCreate("testPerf2", OS_SEMB_EMPTY, &PRV_SHARED_MAIN_OS_DIR_OS_TEST_PERF_SRC_GLOBAL_VAR_GET(t_sem2));
    osSemBinCreate("testPerfE", OS_SEMB_EMPTY, &PRV_SHARED_MAIN_OS_DIR_OS_TEST_PERF_SRC_GLOBAL_VAR_GET(t_semE));
    osTaskCreate("tsk1", 10/*prio*/, 16384/*stack*/,tsk1, NULL, &tid);
    osTaskCreate("tsk2", 10/*prio*/, 16384/*stack*/,tsk2, NULL, &tid);
    osSemWait(PRV_SHARED_MAIN_OS_DIR_OS_TEST_PERF_SRC_GLOBAL_VAR_GET(t_semE), OS_WAIT_FOREVER);
    osTimerWkAfter(1000);
    osSemDelete(PRV_SHARED_MAIN_OS_DIR_OS_TEST_PERF_SRC_GLOBAL_VAR_GET(t_sem1));
    osSemDelete(PRV_SHARED_MAIN_OS_DIR_OS_TEST_PERF_SRC_GLOBAL_VAR_GET(t_sem2));
    osSemDelete(PRV_SHARED_MAIN_OS_DIR_OS_TEST_PERF_SRC_GLOBAL_VAR_GET(t_semE));


    /* osMsgQ test */
    osSemBinCreate("testPerfE", OS_SEMB_EMPTY, &PRV_SHARED_MAIN_OS_DIR_OS_TEST_PERF_SRC_GLOBAL_VAR_GET(t_semE));
    osMsgQCreate("testPerf", P_MSGQ_BUFFER, P_MESSAGE_SIZE, &msgq);
    /* receiver task */
    osTaskCreate("msgqrcv", 10/*prio*/, 16384/*stack*/,msgqrcv, (void*)msgq, &tid);
    /* wait till task ready */
    osSemWait(PRV_SHARED_MAIN_OS_DIR_OS_TEST_PERF_SRC_GLOBAL_VAR_GET(t_semE), OS_WAIT_FOREVER);
    /* start transmit */
    TIMING_STARTL();
    for (k = 0; k < PERF_COUNT; k++)
    {
        char buf[P_MESSAGE_SIZE];
        memcpy(buf, &k, 4);
        osMsgQSend(msgq, buf, P_MESSAGE_SIZE, OS_MSGQ_WAIT_FOREVER);
    }
    /* wait till rx task finish */
    osSemWait(PRV_SHARED_MAIN_OS_DIR_OS_TEST_PERF_SRC_GLOBAL_VAR_GET(t_semE), OS_WAIT_FOREVER);
    osMsgQDelete(msgq);
    osSemDelete(PRV_SHARED_MAIN_OS_DIR_OS_TEST_PERF_SRC_GLOBAL_VAR_GET(t_semE));

#if 1
    TIMING_STARTL();
    for (k = 0; k < PERF_COUNT; k++)
    {
        GT_U32 data;
        GT_STATUS rc;
        rc = prvCpssDrvHwPpPortGroupReadRegister(0,0,0x50,&data);
        if (rc != GT_OK)
        {
            osPrintf("prvCpssDrvHwPpPortGroupReadRegister() failed, rc=%d\n",rc);
            break;
        }
    }
    TIMING_PRINTL("prvCpssDrvHwPpPortGroupReadRegister(0,0,0x50)",k);
    TIMING_STARTL();
    for (k = 0; k < PERF_COUNT; k++)
    {
        GT_U32 data;
        GT_STATUS rc;
        rc = prvCpssDrvHwPpPortGroupReadRegister(0,1,0x50,&data);
        if (rc != GT_OK)
        {
            osPrintf("prvCpssDrvHwPpPortGroupReadRegister() failed, rc=%d\n",rc);
            break;
        }
    }
    TIMING_PRINTL("prvCpssDrvHwPpPortGroupReadRegister(0,1,0x50)",k);
    TIMING_STARTL();
    for (k = 0; k < PERF_COUNT; k++)
    {
        GT_U32 data;
        GT_STATUS rc;
        rc = prvCpssDrvHwPpPortGroupReadRegister(0,2,0x50,&data);
        if (rc != GT_OK)
        {
            osPrintf("prvCpssDrvHwPpPortGroupReadRegister() failed, rc=%d\n",rc);
            break;
        }
    }
    TIMING_PRINTL("prvCpssDrvHwPpPortGroupReadRegister(0,2,0x50)",k);
#endif


    return GT_OK;
}

