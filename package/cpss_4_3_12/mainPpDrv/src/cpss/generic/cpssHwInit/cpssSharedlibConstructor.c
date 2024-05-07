/*******************************************************************************
*              (c), Copyright 2001, Marvell International Ltd.                 *
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
* @file cpssSharedlibConstructor.c
*
* @brief File contains implementation of init routines for  shared library approach.
*
* @version   1
********************************************************************************
*/
#include <string.h>


#include <cpss/generic/log/prvCpssGenCommonTypesLog.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#ifdef PIC_BUILD
#define CPSS_ASLR_SUPPORT_DEFAULT (GT_TRUE)
#else
#define CPSS_ASLR_SUPPORT_DEFAULT (GT_FALSE)
#endif

extern GT_VOID * prvCpssDxChBspCallbacksBind
(
    GT_VOID
);

extern GT_VOID * prvCpssDxChLogCallbacksBind
(
    GT_VOID
);

extern GT_VOID * prvCpssDxChVirtualTcamCallbacksBind
(
    GT_VOID
);

extern GT_VOID * prvCpssDxChLpmTcamCallbacksBind
(
    GT_VOID
);

extern GT_VOID * prvCpssDxChLpmCallbacksBind
(
    GT_VOID
);


extern GT_VOID * prvCpssDxChDiagCallbacksBind
(
    GT_VOID
);

extern GT_VOID * prvCpssDxChCuckooCallbacksBind
(
    GT_VOID
);

__attribute__((constructor))
static GT_VOID cpssLibConstructor
(
    int argc,
    const char * argv[]
)
{
    GT_32 i;

    cpssNonSharedGlobalVars.nonVolatileDb.generalVars.aslrSupport = CPSS_ASLR_SUPPORT_DEFAULT;
    cpssNonSharedGlobalVars.nonVolatileDb.generalVars.verboseMode = GT_FALSE;

    for (i=1; i < argc; i++)
    {
       if (strcmp("-noaslr", argv[i]) == 0)
       {
            cpssNonSharedGlobalVars.nonVolatileDb.generalVars.aslrSupport = GT_FALSE;
       }

       if (strcmp("-verbose", argv[i]) == 0)
       {
            cpssNonSharedGlobalVars.nonVolatileDb.generalVars.verboseMode = GT_TRUE;
       }
    }

    cpssNonSharedGlobalVars.nonVolatileDb.generalVars.exitVars.atExitIndex = 0;
    cpssNonSharedGlobalVars.nonVolatileDb.generalVars.exitVars.removeProcRegistered= GT_FALSE;
    cpssNonSharedGlobalVars.nonVolatileDb.generalVars.exitVars.cleanUpRegistered= GT_FALSE;

     /*register callbacks*/
#ifdef CHX_FAMILY
    cpssNonSharedGlobalVars.nonVolatileDb.callbackArray[GT_CALLBACK_LIB_ID_VTCAM_E]=
        prvCpssDxChVirtualTcamCallbacksBind();

    cpssNonSharedGlobalVars.nonVolatileDb.callbackArray[GT_CALLBACK_LIB_ID_TCAM_E]=
        prvCpssDxChLpmTcamCallbacksBind();

    cpssNonSharedGlobalVars.nonVolatileDb.callbackArray[GT_CALLBACK_LIB_ID_LPM_E]=
        prvCpssDxChLpmCallbacksBind();

    cpssNonSharedGlobalVars.nonVolatileDb.callbackArray[GT_CALLBACK_LIB_ID_BSP_E]=
         prvCpssDxChBspCallbacksBind();
#ifdef CPSS_LOG_ENABLE
    cpssNonSharedGlobalVars.nonVolatileDb.callbackArray[GT_CALLBACK_LIB_ID_LOG_E]=
         prvCpssDxChLogCallbacksBind();
#endif

    cpssNonSharedGlobalVars.nonVolatileDb.callbackArray[GT_CALLBACK_LIB_ID_DIAG_E]=
         prvCpssDxChDiagCallbacksBind();

    cpssNonSharedGlobalVars.nonVolatileDb.callbackArray[GT_CALLBACK_LIB_ID_CUCKOO_E]=
         prvCpssDxChCuckooCallbacksBind();
#endif
}



