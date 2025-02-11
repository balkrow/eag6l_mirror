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
* @file osLinuxTimer.c
*
* @brief Linux User Mode Operating System wrapper. Queue facility.
*
* @version   11
********************************************************************************
*/
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <unistd.h>
#include <time.h>
#include <sys/times.h>
#include <gtOs/gtOsTimer.h>

#ifndef CPSS_CLOCK
#define CPSS_CLOCK CLOCK_REALTIME
#endif

/************ Public Functions ************************************************/

/**
* @internal CPSS_osTickGet function
* @endinternal
*
* @brief   Gets the value of the kernel's tick counter.
*/
GT_U32 CPSS_osTickGet(void)
{
    struct tms buf;
    return (GT_U32)(times(&buf));
}


/**
* @internal osTime function
* @endinternal
*
* @brief   Gets number of seconds passed since system boot
*/
GT_U32 osTime(void)
{
   time_t t ;
   return (GT_U32) time(&t);
}

/**
* @internal osStrftime function
* @endinternal
*
* @brief   convert time value into a formatted string
*
* @param[in] bufferSize               - size of string buffer
* @param[in] format                   -  string, see strftime()
* @param[in] timeValue                - time value returned by osTime()
*
* @param[out] bufferPtr                - String buffer for resulting string
*                                       The second counter value.
*/
GT_U32 osStrftime(
    OUT char *  bufferPtr,
    IN  GT_U32  bufferSize,
    IN  const char* format,
    IN  GT_U32  timeValue
)
{
    struct tm tm_val;
    time_t tim_val = (time_t)timeValue;

    localtime_r(&tim_val, &tm_val);
    return (GT_U32)strftime(bufferPtr, bufferSize, format, &tm_val);
}

/**
* @internal osTimeRT function
* @endinternal
*
* @brief   Get the current time with nanoseconds
*
* @param[out] seconds                  - elapsed time in seconds
* @param[out] nanoSeconds              - elapsed time within a second
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS osTimeRT
(
    OUT GT_U32  *seconds,
    OUT GT_U32  *nanoSeconds
)
{
    struct timespec tv;

    if (clock_gettime(CPSS_CLOCK, &tv) < 0)
        return GT_FAIL;

    if (seconds != NULL)
        *seconds = (GT_U32)tv.tv_sec;
    if (nanoSeconds != NULL)
        *nanoSeconds = (GT_U32)tv.tv_nsec;

    return GT_OK;
}

/**
* @internal osGetSysClockRate function
* @endinternal
*
* @brief   Get the system clock rate
*
* @param[out] ticks                    - The number of  per second of the system clock.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS osGetSysClockRate
(
    OUT GT_U32  *ticks
)
{
    *ticks = (GT_U32)sysconf(_SC_CLK_TCK);
    return GT_OK;
}

/**
* @internal osDelay function
* @endinternal
*
* @brief   System Delay nanoseconds.
*
* @param[in] delay                    - non operational time in nanoseconds
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
* @note The values for calculation of iterations number, achieved by tests.
*
*/
GT_STATUS osDelay
(
    IN GT_U32 delay
)
{
  struct timespec sleep, remain ;
  if (1000000000 < delay)
  {
     /* sleep time to big */
    return GT_BAD_PARAM;
  }
  
  sleep.tv_nsec = delay;
  sleep.tv_sec  = 0 ;
  if ( nanosleep(&sleep, &remain) )
  {
    return GT_FAIL;
  }

    return GT_OK;
}

#ifdef DEBUG_OS_TIMER
/**
* @internal timerDebug function
* @endinternal
*
* @brief   System Delay nanoseconds.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS timerDebug(GT_U32 loops)
{
    struct timespec start, end;
    volatile GT_U32 i;                /* loop iterator */

    clock_gettime(CPSS_CLOCK, &start);
    for(i = 0; i < loops; i++);
    clock_gettime(CPSS_CLOCK, &end);
    osPrintf("Start Time: sec=%d, nanosec=%d \n",start.tv_sec,start.tv_nsec);
    osPrintf("End Time: sec=%d, nanosec=%d \n",end.tv_sec,end.tv_nsec);
    osPrintf("Time: sec=%d, nanosec=%d \n",end.tv_sec - start.tv_sec,end.tv_nsec - start.tv_nsec);
    return GT_OK;


}

#endif



