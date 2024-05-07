/*******************************************************************************
*              (c), Copyright 2022, Marvell International Ltd.                 *
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
* @file osWin32SystemDown.c
*
* @brief This file provides handling of app/CPU reset.
*
* @version   1
********************************************************************************
*/

#include <stdlib.h>
#include <signal.h>
#include <gtOs/gtOsSystemDown.h>
#include <cpss/extServices/os/gtOs/gtEnvDep.h>

/************ Private Functions ************************************************/

/*******************************************************************************
* prvOsCpuResetHandler
*
* DESCRIPTION:
*       Calls 'exit', on which other handlers use to register (using
*       osRegisterCpuResetFunc/osAtExit)
*
* INPUTS:
*
*        sig (ignored)
*
* OUTPUTS:
*       None
*
* RETURNS:
*      NONE
*
* COMMENTS:
*   Note! This handler is identical to the handler in shrMemSharedLibraryInit.c,
*   so even if both are called, exit() will be called on the receive of the
*   below signals.
*
*******************************************************************************/
static void prvOsCpuResetHandler(int sig)
{
    GT_UNUSED_PARAM(sig);
    exit(0);   /* All reset handlers will be registered on exit event */
}


/************ Public Functions ************************************************/

/*******************************************************************************
* osRegisterCpuResetFunc
*
* DESCRIPTION:
*       Registers the given function to be called at various events in which
*       the App terminates, including when CPU gets reset and App receives
*       sigterm.
*       The functions are called in the reverse order of their registration
*
* INPUTS:
*
*        None
*
* OUTPUTS:
*       Process ID
*
* RETURNS:
*      NONE
*
* COMMENTS:
*       The same function may be registered multiple times: it is called
*       once for each registration
*
*******************************************************************************/
GT_STATUS osRegisterCpuResetFunc(OS_AT_EXIT_FUNC rstFunc)
{
    static int firstTime = 1;

    if (firstTime)
    {
        firstTime = 0;

        signal(SIGINT,   prvOsCpuResetHandler);
        signal(SIGILL,   prvOsCpuResetHandler);
        signal(SIGFPE,   prvOsCpuResetHandler);
        signal(SIGSEGV,  prvOsCpuResetHandler);
        signal(SIGTERM,  prvOsCpuResetHandler);
        signal(SIGBREAK, prvOsCpuResetHandler);
        signal(SIGABRT,  prvOsCpuResetHandler);
    }

    return atexit(rstFunc);
}


