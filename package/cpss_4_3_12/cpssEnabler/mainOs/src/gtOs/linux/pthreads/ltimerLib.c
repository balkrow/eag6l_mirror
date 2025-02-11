/**
********************************************************************************
* @file ltimerLib.c
*
* @brief Pthread implementation of mainOs tasks and semaphores
* Task delay and time arithmetic
*
* @version   6
********************************************************************************
*/
/*******************************************************************************
* ltimerLib.c
*
* DESCRIPTION:
*       Pthread implementation of mainOs tasks and semaphores
*       Task delay and time arithmetic
*
* DEPENDENCIES:
*       POSIX pthreads
*
* FILE REVISION NUMBER:
*       $Revision: 6 $
*
*******************************************************************************/

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <gtOs/gtOsTimer.h>
#include <sched.h>
#include <sys/types.h>
#include <time.h>
#include <pthread.h>
#include "mainOsPthread.h"


/********************************
** timespec arithmetic
********************************/

/*****************************************************************************
** timespec_is_zero - returns 1 if value is zero
*****************************************************************************/
int timespec_is_zero(struct timespec *value)
{
    return (value->tv_sec == 0 && value->tv_nsec == 0) ? 1 : 0;
}

/*****************************************************************************
** timespec_gt - returns 1 if a greater than b
*****************************************************************************/
int timespec_gt(struct timespec *a, struct timespec *b)
{
    if (a->tv_sec > b->tv_sec)
        return 1;
    if (a->tv_sec < b->tv_sec)
        return 0;
    return (a->tv_nsec > b->tv_nsec) ? 1 : 0;
}

/*****************************************************************************
** timespec_add - add value to accumulator
*****************************************************************************/
void timespec_add(struct timespec *accumulator, struct timespec *value)
{
    accumulator->tv_sec += value->tv_sec;
    accumulator->tv_nsec += value->tv_nsec;
    if (accumulator->tv_nsec >= 1000000000)
    {
        accumulator->tv_sec++;
        accumulator->tv_nsec -= 1000000000;
    }
}

/*****************************************************************************
** timespec_sub - sub b from a, delta = 0 if b >= a, otherwise delta = a - b
*****************************************************************************/
void timespec_sub(struct timespec *a, struct timespec *b, struct timespec *delta)
{
    if (timespec_gt(a,b))
    {
        delta->tv_sec = a->tv_sec - b->tv_sec;
        delta->tv_nsec = a->tv_nsec - b->tv_nsec;
        if (delta->tv_nsec < 0)
        {
            delta->tv_nsec += 1000000000;
            delta->tv_sec--;
        }
    } else
    {
        delta->tv_sec = 0;
        delta->tv_nsec = 0;
    }
}

/*****************************************************************************
** milliseconds2timespec - convert time in system ticks to timespec structure
**            ( one tick is currently implemented as ten milliseconds )
*****************************************************************************/
void milliseconds2timespec(int milliseconds, struct timespec *result)
{
    if (milliseconds < 1000)
    {
        result->tv_sec = 0;
        result->tv_nsec = milliseconds * 1000000;
    }
    else
    {
        result->tv_sec = milliseconds / 1000;
        result->tv_nsec = (milliseconds % 1000) * 1000000;
    }
}
extern GT_U32  cpssDeviceRunCheck_onEmulator(void);
extern GT_U32  cpssDeviceRunCheck_onEmulator_internalCpu(void);
extern GT_U32  cpssDeviceRunGet_embeddedCpuTimeFactor(void);
extern int usleep(unsigned int);
/**
* @internal osTimerWkAfter function
* @endinternal
*
* @brief   Puts current task to sleep for specified number of milisecond.
*
* @param[in] mils                     - time to sleep in miliseconds
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS osTimerWkAfter
(
    IN GT_U32 mils
)
{
    struct timespec now, timeout, delta;

    if(cpssDeviceRunCheck_onEmulator() &&
       cpssDeviceRunCheck_onEmulator_internalCpu())
    {
        GT_U32 uTime = (mils * 1000) / cpssDeviceRunGet_embeddedCpuTimeFactor();

        if(uTime == 0 && mils != 0)
        {
            uTime = mils;
        }

        usleep(uTime);
        return GT_OK;
    }

    if (!mils)
    {
        /*
        **  Delay of zero means yield CPU to other tasks of same priority
        **  Yield to any other task of same priority without blocking.
        */
        sched_yield();
        return GT_OK;
    }

    milliseconds2timespec(mils, &delta);

    /*
    **  Establish absolute time at expiration of delay interval
    */
    clock_gettime(CPSS_CLOCK, &timeout);
    timespec_add(&timeout, &delta);

    /*
    **  Wait for the current time of day to reach the time of day calculated
    **  after the timeout expires.  The loop is necessary since the thread
    **  may be awakened by signals before the timeout has elapsed.
    */
    while ( !timespec_is_zero(&delta) )
    {
        /*
        **  Add a cancellation point to this loop,
        **  since there are no others.
        */
        pthread_testcancel();
        nanosleep(&delta, NULL);

        clock_gettime(CPSS_CLOCK, &now);
        timespec_sub(&timeout, &now, &delta);
    }

    return GT_OK;
}


