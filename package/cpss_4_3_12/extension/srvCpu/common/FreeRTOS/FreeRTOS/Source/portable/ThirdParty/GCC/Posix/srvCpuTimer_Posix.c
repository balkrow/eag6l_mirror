/*******************************************************************************
Copyright (C) Marvell International Ltd. and its affiliates

This software file (the "File") is owned and distributed by Marvell
International Ltd. and/or its affiliates ("Marvell") under the following
alternative licensing terms.  Once you have made an election to distribute the
File under one of the following license alternatives, please (i) delete this
introductory statement regarding license alternatives, (ii) delete the two
license alternatives that you have not elected to use and (iii) preserve the
Marvell copyright notice above.

********************************************************************************
Marvell Commercial License Option

If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.

********************************************************************************
Marvell GPL License Option

If you received this File from Marvell, you may opt to use, redistribute and/or
modify this File in accordance with the terms and conditions of the General
Public License Version 2, June 1991 (the "GPL License"), a copy of which is
available along with the File in the license.txt file or by writing to the Free
Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 or
on the worldwide web at http://www.gnu.org/licenses/gpl.txt.

THE FILE IS DISTRIBUTED AS-IS, WITHOUT WARRANTY OF ANY KIND, AND THE IMPLIED
WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE ARE EXPRESSLY
DISCLAIMED.  The GPL License provides additional details about this warranty
disclaimer.
********************************************************************************
Marvell BSD License Option

If you received this File from Marvell, you may opt to use, redistribute and/or
modify this File under the following licensing terms.
Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

    *   Redistributions of source code must retain the above copyright notice,
        this list of conditions and the following disclaimer.

    *   Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.

    *   Neither the name of Marvell nor the names of its contributors may be
        used to endorse or promote products derived from this software without
        specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*******************************************************************************/
/**
********************************************************************************
* @file mvOsTimer_cm3.c
*
* @brief Timer capabilities implementation.
*
* @version   1
********************************************************************************
*/

#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/time.h>
#include <sys/times.h>
#include <time.h>
#include "srvCpuTimer.h"
#include <srvCpuServices.h>


MV_U32 mvOsTclk;
MV_U32 ticks = 0;

/**
* @internal srvCpuBoardTclkGet function
* @endinternal
*
* @brief   returns number of ticks in 1 sec (sys clk)
* @param none
*
* @retval MV_U32   - number of ticks in 1 sec (sys clk)
*/
MV_U32 srvCpuBoardTclkGet(void)
{
   return 250000000;
}

/**
* @internal srvCpuCpuPclkGet function
* @endinternal
*
* @brief   This routine extract the CPU core clock.
* @param none
*
* @retval MV_U32   - 32bit clock cycles in MHz.
*/
MV_U32 srvCpuCpuPclkGet(MV_VOID)
{
   return 250000000;
}

/**
* @internal srvCpuOsTickInit function
* @endinternal
*
* @brief   Initialize system Timer0 and tclk static variable
* @param none
*
*/
void srvCpuOsTickInit(void)
{
    /* Implemented in posix port of FreeRTOS */
}

/**
* @internal srvCpuOsUsDelay function
* @endinternal
*
* @brief   Delay in micro-sec
*          srvCpuOsUsDelay(), srvCpuOsMsDelay() and srvCpuOsTimerCountGet() use free running timer
*          They are meant for short delays in high resolution, limited to 671ms
*          Do not use them in conjunction with srvCpuOsTickXXX() APIs as they modify the
*          counter to get lower resolution and longer delay periods
* @param MV_U32 us_delay - wanted delay in micro seconds
*
*/
MV_VOID srvCpuOsUsDelay(MV_U32 us_delay)
{
    MV_U32  counter = us_delay;

    /* loop waiting for the timer to expire */
    do
    {
        usleep(1); /* use small value to prevent Linux from rescheduling the process */
    }
    while (counter-- > 0);
}

/**
* @internal srvCpuOsMsDelay function
* @endinternal
*
* @brief   Delay in milli seconds
*          srvCpuOsUsDelay(), srvCpuOsMsDelay() and srvCpuOsTimerCountGet() use free running timer
*          They are meant for short delays in high resolution, limited to 671ms
*          Do not use them in conjunction with srvCpuOsTickXXX() APIs as they modify the
*          counter to get lower resolution and longer delay periods
*
* @param MV_U32 ms_delay - wanted delay in milli seconds
*
*/
MV_VOID srvCpuOsMsDelay(MV_U32 ms_delay)
{
    MV_U32 i;

    for (i = 0; i < ms_delay * 10; i++) {
        srvCpuOsUsDelay(100);
    }
}

/**
* @internal srvCpuOsTimerCountGet function
* @endinternal
*
* @brief   Get the current timer count, Resolution is 25MHz
*        srvCpuOsUsDelay(), srvCpuOsMsDelay() and srvCpuOsTimerCountGet() use free running timer
*        They are meant for short delays in high resolution, limited to 671ms
*        Do not use them in conjunction with srvCpuOsTickXXX() APIs as they modify the
*        counter to get lower resolution and longer delay periods
* @param none
* @retval MV_U32   - Get the current timer count
*/
MV_U32 srvCpuOsTimerCountGet(void)
{
    struct timespec t;

    clock_gettime(CLOCK_MONOTONIC, &t);

    return ( (t.tv_sec * 1000000000ull + t.tv_nsec) / 40); /* 1ns or 1000MHz ==> 25MHz */
}

/**
* @internal srvCpuOsTickCount function
* @endinternal
*
* @brief   Get the current system millisecond tick
*          Returns the current system tick count.
*          Poll this API at fractions of ms, to get accurate measurement.
* @param   none
*
* @retval  MV_U32   - Returns the current system tick value
*/
MV_U32 srvCpuOsTickCount(void)
{
    /* not used */
    return 0;
}

/**
* @internal srvCpuOsTickSet function
* @endinternal
*
* @brief   Set delay counter
*          CM3 timer is a 24bit counter, counting at 25MHz, limited to 671ms
*          (1us = 25 ticks, 1ms = 25000 ticks)
*          To handle delays larger than that, we set the counter to 1ms,
*          and count ms_delay expirations
* @param MV_U32 us_delay - wanted delay in micro seconds
*
*/
MV_VOID srvCpuOsTickSet(MV_U32 ms_delay)
{
    /* not used */
}
