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
* @file osLinuxGen.c
*
* @brief Operating System wrapper general APIs implementation
*
* @version   5
********************************************************************************
*/


/************* Includes *******************************************************/
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <gtOs/gtOsGen.h>
#include <gtOs/gtOsSharedPp.h>
#include <gtOs/gtOsSharedData.h>
#include <gtOs/globalDb/gtOsGlobalDbInterface.h>
#include <gtOs/globalDb/gtOsGlobalDb.h>

#include <gtUtil/gtBmPool.h>

#include "pthreads/mainOsPthread.h"

#ifndef SHARED_MEMORY
/* Required for shrMemSharedPpInit */
#   include <gtOs/gtOsSharedPp.h>
#endif

#if (defined ASIC_SIMULATION_ENV_FORBIDDEN && defined ASIC_SIMULATION)
    /* 'Forbid' the code to run as ASIC_SIMULATION ... we need 'like' HW compilation ! */
    #undef ASIC_SIMULATION
#endif


/*************** Globals ******************************************************/
#define PRV_SHARED_DB osSharedGlobalVarsPtr->cpssEnablerMod.mainOsDir.gtOsLinuxGenSrc

/**
* @internal osWrapperOpen function
* @endinternal
*
* @brief   Initialize OS wrapper facility.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_ALREADY_EXIST         - wrapper was already open
*/
GT_STATUS osWrapperOpen(void * osSpec)
{
    GT_STATUS rc;
    GT_BOOL aslrSupport ;


#if defined SHARED_MEMORY
    aslrSupport = osNonSharedGlobalVars.osNonVolatileDb.aslrSupport;
#else
    aslrSupport = GT_FALSE;
#endif


    rc = gtOsHelperGlobalDbInit(aslrSupport,__func__,__LINE__);
    if(rc != GT_OK)
    {
        return rc;
    }

    if (PRV_SHARED_DB.gtOsWrapperOpen == GT_TRUE)
    {
        return GT_OK;
    }
    else
    {
        if (osSpec != NULL)
        {
            const char *name   = (const char *) osSpec;
            V2L_ltaskInit(name);
            V2L_lsemInit();
            V2L_lmsgQInit();
        }

#if (!defined SHARED_MEMORY && (!defined LINUX || !defined ASIC_SIMULATION))
        /*  Open the packet processor device driver.
            NOTE - share DMA buffer here only wihtout Shared Memory approach!
            Otherwise it should be initialized
            into shrMemInitSharedLibrary[_FirstClient] */
        if (shrMemSharedPpInit(GT_TRUE) != GT_OK)
        {
            return GT_FAIL;
        }

#endif
        /* Must be set to GT_TRUE before use any of OS functions. */
        PRV_SHARED_DB.gtOsWrapperOpen = GT_TRUE;

        gtPoolInit( 500 );
    }

    return GT_OK;
}

/**
* @internal osWrapperClose function
* @endinternal
*
* @brief   Close OS wrapper facility and free all used resources.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS osWrapperClose(void)
{
    GT_STATUS rc;

    IS_WRAPPER_OPEN_STATUS;

    rc = gtOsHelperGlobalDbDestroy(GT_TRUE);
    if(rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}

/*******************************************************************************
* osWrapperIsOpen
*
* DESCRIPTION:
*       Returns GT_TRUE if OS wrapper facility was initialized .
*
* INPUTS:
*       None
*
* OUTPUTS:
*       None
*
* RETURNS:
*       GT_TRUE          - on success
*       GT_FALSE         - on error
*
* COMMENTS:
*       None
*
*******************************************************************************/
GT_INLINE GT_BOOL osWrapperIsOpen(void)
{
    return PRV_SHARED_DB.gtOsWrapperOpen;
}

