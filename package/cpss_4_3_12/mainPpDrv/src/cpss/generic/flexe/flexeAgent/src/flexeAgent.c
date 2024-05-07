/*******************************************************************************
*              (c), Copyright 2023, Marvell International Ltd.                 *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL SEMICONDUCTOR, INC.   *
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
********************************************************************************
*/
/**
********************************************************************************
* @file flexeAgent.c
*
* @brief FLEXE AGENT high level API implementation
*
* @version   1
********************************************************************************
*/

#ifndef FLEXE_AGENT_IN_CPSS
    #include <hw.h>
    #include <task.h>
    #include <FreeRTOS_CLI.h>
    #include <global.h>
    #include <uart.h>
    #include <srvCpuServices.h>
    #include <srvCpuTimer.h>
#else

#endif

#include <flexeAgentTypes.h>
#include <flexeAgent.h>
#include <flexeAgentIpc.h>
#include <flexeAgentRegAddr.h>
#include <flexeAgentTransit.h>

#ifdef FLEXE_AGENT_IN_CPSS
extern GT_VOID flexeTransitTaskCreate
(
    GT_VOID
);
#endif

#if 0

/* clear the counters DB */
static void clearCountersDb(void)
{
    ipcMsgCount                     = 0;
}

void clearDb(void)
{
    clearCountersDb();
    return;
}

static void timerInit(void)
{
    MV_U32 configRatioValue = SYS_TCLK_RATIO; /* divide in order to get value in millisecond */

    /* Stop the timer */
    srvCpuRegWrite(TIMER0_CNRTL_REG, 0x1<<2);

    /* Set timer limit value*/
    srvCpuRegWrite(TIMER0_MAX_VAL_LSB_REG, 0xFF);
    srvCpuRegWrite(TIMER0_MAX_VAL_MSB_REG, 0xFF);

    /* Unset timer stop operation & reset timer */
    srvCpuRegWrite(TIMER0_CNRTL_REG, 0x1<<3);

    /* Start timer */
    srvCpuRegWrite(TIMER0_START_REG, 0x1<<0);

    /* Set timer counting resoultion to 1ms */
    srvCpuRegWrite(TIMER0_RATIO_REG, configRatioValue);
    return;
}

static void timerCompute(unsigned int interval)
{
    MV_U16 count            = 0x0;
    static MV_U8  iter      = 0x0;
    MV_U8  timer            = interval; /* CM3 polling interval in seconds for IPFIX scheduler */

    if(srvCpuRegRead(TIMER0_START_REG) & 0x1)
    {
        count = srvCpuRegRead(TIMER0_CNT_VAL_MSB_REG);
        count = (count << 8) | srvCpuRegRead(TIMER0_CNT_VAL_LSB_REG);
        /* track every 1s of timer ticks */
        /* CM3 external timer resolution is set to 1ms and based on experimentation,
         * it was found that for AC5X&P in order to get seconds order granularity
         * the count value should be 40.
         */
        if(count > 0)
        {
            if((count % 400) == 0)
            {
                iter++;
                count = 0x0;
            }
        }
        if((iter > 0) && ((iter % timer) == 0))
        {
            done = MV_TRUE;
            iter = 0x0;
        }
    }
}

static void timerRestart(void)
{
    /* Timer stop */
    /* The following is the sequence to reload\reset the timer without clearing
       the start bit. */
    /* Stop the timer and set restart*/
    srvCpuRegWrite(TIMER0_CNRTL_REG, 0x5);
    /* Set (in addition to the stop & restart) the reset */
    srvCpuRegWrite(TIMER0_CNRTL_REG, 0xd);
    /* Clear all bits (stop, restart & reset) */
    srvCpuRegWrite(TIMER0_CNRTL_REG, 0x0);
    /* Re-start timer */
    if((srvCpuRegRead(TIMER0_START_REG) & 0x1) == 0)
    {
        /* Start timer */
        srvCpuRegWrite(TIMER0_START_REG, 0x1<<0);
    }
    done = MV_FALSE;
    timerCompute(10/* Table scan cycle interval*/);
}

void prvTimerTask( void *pvParameters )
{
    timerInit();
    while(1)
    {
        timerCompute(10);
    }
}
#endif
