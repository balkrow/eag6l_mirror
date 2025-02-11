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
* @file shrMemSharedLibraryInit.c
*
* @brief File contains implementation of init routines for
* Shared Memory approach.
*
* @version   1
********************************************************************************
*/

/************* Includes *******************************************************/
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif


#include <sys/mman.h>
#include <semaphore.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>


#include <gtOs/gtOsIo.h>
#include <gtOs/gtOsMem.h>
#include <gtOs/gtOsGen.h>

#include <gtOs/gtOsSharedMemory.h>
#include <gtOs/gtOsSharedMemoryRemapper.h>
#include <gtOs/gtOsSharedMalloc.h>
#include <gtOs/gtOsSharedPp.h>
#include <gtOs/globalDb/gtOsGlobalDb.h>
#include <gtOs/globalDb/gtOsGlobalDbInterface.h>
#include <signal.h>
#define PRV_OS_SHM_HANDLED_SIGNALS SIGINT,/* ctrl+c */\
                                   SIGABRT,/* abort */\
                                   SIGTERM,\
                                   SIGSEGV,\
                                   SIGBUS,\
                                   SIGXCPU /*CPU limit exceeded*/

char* shrAddNameSuffix(const char* name, char* buffer, int bufsize);


static void sigStopTempHandler(int sig)
{
    GT_32 i;

    SHM_PRINT("Handling signal %d \n",sig);

    for(i=osNonSharedGlobalVars.osNonVolatileDb.exitFuncDepth;i>0;i--)
    {
        SHM_PRINT("Handling exit func index %d \n",i-1);
        osNonSharedGlobalVars.osNonVolatileDb.exitFuncs[i-1]();
    }

    signal(sig, SIG_DFL);
    raise(sig);
}

static void normalExitHandler
(
    GT_VOID
)
{
    sigStopTempHandler(SIGTERM);
}

GT_STATUS firstProcessDiscoveryUnLock
(
   const GT_CHAR_PTR name
);


void  firstProcessDiscoveryCleanUp(void)
{
    GT_STATUS rc = GT_OK;

    if(FIRST_CLIENT_DISCOVERY_SEM_OWNER_BIT&
        PRV_HELPER_NON_SHARED_GLOBAL_VAR_DIRECT_GET(osNonVolatileDb.semOwnershipBmp))
    {
           /*in case something will go wrong during the discovery*/
        rc = firstProcessDiscoveryUnLock(FIRST_CLIENT_DISCOVERY_SEM_NAME);
        fprintf(stderr, "*** firstProcessDiscoveryCleanUp() called rc = %d\n",rc);
    }
}

static GT_STATUS firstProcessDiscoveryLockPtrGet
(
    sem_t** dbDiscoverySemPtr ,
    GT_CHAR * buffer
)
{

  if(NULL == dbDiscoverySemPtr)
  {
    return GT_BAD_PTR;
  }

  if(NULL == PRV_HELPER_NON_SHARED_GLOBAL_VAR_DIRECT_GET(osNonVolatileDb.dbDiscoverySem))
  {
       /* Open named semaphore to guard  code from other clients. */
       if((*dbDiscoverySemPtr = sem_open(buffer, O_CREAT, 0666, 1)) == SEM_FAILED)
       {
           perror( "firstProcessDiscoveryLock:sem_open()");
           return GT_FAIL;
       }
       PRV_HELPER_NON_SHARED_GLOBAL_VAR_DIRECT_SET(osNonVolatileDb.dbDiscoverySem,(GT_VOID * ) (*dbDiscoverySemPtr));
  }
  else
  {
      *dbDiscoverySemPtr = (sem_t * )PRV_HELPER_NON_SHARED_GLOBAL_VAR_DIRECT_GET(osNonVolatileDb.dbDiscoverySem);
  }

  return GT_OK;

}

/**
* @internal firstProcessDiscoveryLock function
* @endinternal
*
* @brief  Lock access to file system when trying to determine if this is a first process
*  Use named semaphore.
*
*/
GT_STATUS firstProcessDiscoveryLock
(
    const GT_CHAR_PTR name
)
{
    sem_t* dbInitSem; /* named semaphore  to protect data base initialization in shared lib mode  */
    GT_STATUS rc = GT_OK;
    GT_CHAR buffer[256];
    GT_U32 bmp = PRV_HELPER_NON_SHARED_GLOBAL_VAR_DIRECT_GET(osNonVolatileDb.semOwnershipBmp);
    shrAddNameSuffix(name,buffer,sizeof(buffer));


    /* register shared memory content cleaner */
    if (osAtExit(firstProcessDiscoveryCleanUp))
    {
        fprintf(stderr, "Cannot register /dev/shr cleaner, error=%d\n", errno);
    }

    rc = firstProcessDiscoveryLockPtrGet(&dbInitSem,buffer);
    if(rc!=GT_OK)
    {
        return rc;
    }

    if(sem_wait(dbInitSem) == -1)
    {
        perror( "firstProcessDiscoveryLock:sem_wait()");
        return GT_FAIL;
    }

    bmp |=(FIRST_CLIENT_DISCOVERY_SEM_OWNER_BIT);

    PRV_HELPER_NON_SHARED_GLOBAL_VAR_DIRECT_SET(osNonVolatileDb.semOwnershipBmp,bmp);

    return rc;
}


/**
* @internal firstProcessDiscoveryUnLock function
* @endinternal
*
* @brief  UnLock access to file system when trying to determine if this is a first process
*  Use named semaphore.
*
*/

GT_STATUS firstProcessDiscoveryUnLock
(
    const GT_CHAR_PTR name
)
{
    sem_t* dbInitSem; /* named semaphore  to protect data base initialization in shared lib mode  */
    GT_STATUS rc = GT_OK;
    GT_CHAR buffer[256];
    shrAddNameSuffix(name,buffer,sizeof(buffer));
    GT_U32 bmp = PRV_HELPER_NON_SHARED_GLOBAL_VAR_DIRECT_GET(osNonVolatileDb.semOwnershipBmp);


    rc = firstProcessDiscoveryLockPtrGet(&dbInitSem,buffer);
    if(rc!=GT_OK)
    {
        return rc;
    }


    if(sem_post(dbInitSem) == -1)
    {
        osPrintf("sem_post() : %s\n", strerror(errno));
        return GT_FAIL;
    }

    bmp &=(~FIRST_CLIENT_DISCOVERY_SEM_OWNER_BIT);

    PRV_HELPER_NON_SHARED_GLOBAL_VAR_DIRECT_SET(osNonVolatileDb.semOwnershipBmp,bmp);


    return rc;
}


GT_STATUS firstProcessDiscoveryUnLink
(
    const GT_CHAR_PTR name
)
{
    sem_t* dbInitSem; /* named semaphore  to protect data base initialization in shared lib mode  */
    GT_STATUS rc = GT_OK;
    GT_CHAR buffer[256];
    shrAddNameSuffix(name,buffer,sizeof(buffer));



    rc = firstProcessDiscoveryLockPtrGet(&dbInitSem,buffer);
    if(rc!=GT_OK)
    {
        return rc;
    }

    if(sem_close(dbInitSem) == -1)
    {
        osPrintf("sem_close() : %s\n", strerror(errno));
        return GT_FAIL;
    }

    if(sem_unlink(buffer) < 0)
    {
        if(errno != ENOENT && 0 != errno)
        {
            osPrintf("sem_unlink() : %s %s\n", buffer, strerror(errno));
            return GT_FAIL;
        }
    }

    return rc;
}


GT_STATUS shrMemDetectLinkerScript
(
    GT_BOOL *linkerScriptDetectedPtr
)
{
#ifdef PIC_BUILD
    SECTION_STC cpssSection;
    SECTION_STC mainOsSection;
    GT_STATUS   rc;

    rc = findAdressesOfSections(
              &cpssSection,
              &mainOsSection,
              GT_FALSE
          );
    if(rc != GT_OK)
    {
        return rc;
    }

    if ((cpssSection.startPtr == NULL)&&(mainOsSection.startPtr == NULL))
    {
        *linkerScriptDetectedPtr = GT_FALSE;
    }
    else
    {
        *linkerScriptDetectedPtr = GT_TRUE;
     }
#else
    *linkerScriptDetectedPtr = GT_TRUE;
#endif

return GT_OK;
}


/************* Functions ******************************************************/
/************* Public Functions ***********************************************/

/**
* @internal shrMemInitSharedLibrary_FirstClient function
* @endinternal
*
* @brief   Initialize shared library for the first process (CPSS Enabler process)
*
* @retval GT_OK                    - Operation succeeded
* @retval GT_FAIL                  - Operaton failed
*
* @note First process should perform additional steps to initialize all
*       shared buffers.
*       See also shrMemInitSharedLibrary function for non-first clients.
*
*/
GT_STATUS shrMemInitSharedLibrary_FirstClient
(
    GT_VOID
)
{
    GT_STATUS rc;


    if (GT_TRUE == osNonSharedGlobalVars.osNonVolatileDb.clientIsInitialised)
        return GT_OK;

    if(GT_FALSE==osNonSharedGlobalVars.osNonVolatileDb.aslrSupport)
    {
        /* 1nd step - create shared buffer and map it instead private BSS/DATA */
        if(shrMemDoShmMap(/*isFirstCLient=*/GT_TRUE) == GT_FAIL)
        {
            fprintf(stderr, "shrMemDoShmMap() : error\n");
            return GT_FAIL;
        }
        shmPrintf("Step 1: Master process shrMemDoShmMap() success\n");
    }
    /* 2rd step - create and initialize and map shared buffer for osMalloc */
    rc = shrMemSharedMallocInit(/*isFirstCLient=*/GT_TRUE);
    if(rc!=GT_OK)
    {
        fprintf(stderr, "shrMemSharedMallocInit() : error\n");
        return GT_FAIL;
    }
    shmPrintf("Step 2: Master process shrMemSharedMallocInit() success\n");

#ifndef ASIC_SIMULATION

    /* Applicable only for BM */

    /* 3th step - initialize shared buffer for DMA with PP */
    if (shrMemSharedPpInit(/*isFirstCLient=*/GT_TRUE) != GT_OK)
    {
        fprintf(stderr, "shrMemSharedPpInit() : error\n");
        return GT_FAIL;
    }
    shmPrintf("Step 3: Master process shrMemSharedPpInit() success\n");

#endif

    return GT_OK;
}

/**
* @internal shrMemInitSharedLibrary function
* @endinternal
*
* @brief   Initialize shared library for non-first processes.
*
* @retval GT_OK                    - Operation succeeded
* @retval GT_FAIL                  - Operaton failed
*
* @note This routine assumes that all shared buffers are already created,
*       so we can just use them.
*       Note - shrMemInitSharedLibrary_FirstClient routine HAVE to be called
*       before by the first client to create shared buffers.
*
*/
GT_STATUS shrMemInitSharedLibrary
(
    GT_VOID
)
{

    if (GT_TRUE == osNonSharedGlobalVars.osNonVolatileDb.clientIsInitialised)
        return GT_OK;


    if(GT_FALSE==osNonSharedGlobalVars.osNonVolatileDb.aslrSupport)
    {
        /* 1ts step - map already created shared buffer instead private BSS/DATA */
        if(shrMemDoShmMap(/*isFirstCLient=*/GT_FALSE) == GT_FAIL)
        {
            fprintf(stderr, "shrMemDoShmMap() : error\n");
            return GT_FAIL;
        }
        shmPrintf("Step 1: Slave process shrMemDoShmMap() success\n");
    }

    /* 2nd step - map already created shared buffer for osMalloc */
    if (shrMemSharedMallocInit(/*isFirstCLient=*/GT_FALSE) != GT_OK)
    {
        fprintf(stderr, "shrMemSharedMallocInit() : error\n");
        return GT_FAIL;
    }
    shmPrintf("Step 2: Slave process shrMemSharedMallocInit() success\n");

#ifndef ASIC_SIMULATION /* Applicable ONLY for BM */

    /* 3rd step - map already created shared buffer for DMA with PP
     * to processes address space. */
    if (shrMemSharedPpInit(/*isFirstCLient=*/GT_FALSE) != GT_OK)
    {
        fprintf(stderr, "shrMemSharedPpInit() : error\n");
        return GT_FAIL;
    }
    shmPrintf("Step 3: Slave process shrMemSharedPpInit() success\n");

#endif

    return GT_OK;
}


#ifndef ASIC_SIMULATION /* Applicable ONLY for BM */
extern GT_STATUS extDrvInitDrv(OUT GT_BOOL  *isFirstClient);
#endif




__attribute__((constructor))
static void libhelper_constructor(int argc, const char * argv[])
{
    GT_BOOL     first = GT_FALSE;
    GT_STATUS   rc;
    GT_BOOL     osAslrSupport = OS_ASLR_SUPPORT_DEFAULT;
    char buf[256];
    GT_32 i;
    GT_U32 j;
    GT_BOOL linkerScriptDetected = GT_FALSE;
    int  handledSignals[] ={PRV_OS_SHM_HANDLED_SIGNALS};

    osNonSharedGlobalVars.osNonVolatileDb.verboseMode = GT_FALSE;
    osNonSharedGlobalVars.osNonVolatileDb.isFirst     =  GT_FALSE;
    osNonSharedGlobalVars.osNonVolatileDb.semOwnershipBmp = 0;
    osNonSharedGlobalVars.osNonVolatileDb.dbInitSem = NULL;
    osNonSharedGlobalVars.osNonVolatileDb.dbDiscoverySem= NULL;
    osNonSharedGlobalVars.osNonVolatileDb.exitFuncDepth = 0;

    for (i=1; i < argc; i++)
    {
       if (strcmp("-noaslr", argv[i]) == 0)
       {
            osAslrSupport = GT_FALSE;
       }

       if (strcmp("-verbose", argv[i]) == 0)
       {
            osNonSharedGlobalVars.osNonVolatileDb.verboseMode= GT_TRUE;
       }
    }

     for(j=0;j<sizeof(handledSignals)/sizeof(handledSignals[0]);j++)
     {
         SHM_PRINT("Add signal handler for signal %d [%s]\n",handledSignals[j],strsignal(handledSignals[j]));
         signal(handledSignals[j],sigStopTempHandler);
     }

     atexit(normalExitHandler);

     if(osAslrSupport==GT_TRUE)
     {
        /*check if indeed build contain ASLR support*/
        (GT_VOID)shrMemDetectLinkerScript(&linkerScriptDetected);
        if (GT_TRUE==linkerScriptDetected)
        {
            fprintf(stderr, "Linker script detected.Please add    \"-noaslr\"  command line parameter and rerun.\n");
            sigStopTempHandler(0);
        }
     }

    osNonSharedGlobalVars.osNonVolatileDb.aslrSupport  = osAslrSupport;

    rc = firstProcessDiscoveryLock(FIRST_CLIENT_DISCOVERY_SEM_NAME);
    if (rc != GT_OK)
    {
        fprintf(stderr, "firstProcessDiscoveryLock() failed, rc=%d\n",rc);
    }
     /* Since BSS remap  is not done ,decide based on shared malloc shared memory
            */
    shrAddNameSuffix(SHARED_MEMORY_MALLOC_DATA_CNS, buf, sizeof(buf));

    first = (access(buf, R_OK|W_OK) == 0) ? GT_FALSE : GT_TRUE;
    if (first == GT_TRUE)
    {
        osNonSharedGlobalVars.osNonVolatileDb.isFirst = GT_TRUE;
        rc = shrMemInitSharedLibrary_FirstClient();
        if (rc != GT_OK)
        {
            fprintf(stderr, "shrMemInitSharedLibrary_FirstClient() failed, rc=%d\n",rc);
        }
    }
    else
    {
        rc = shrMemInitSharedLibrary();
        if (rc != GT_OK)
        {
            fprintf(stderr, "shrMemInitSharedLibrary() failed, rc=%d\n",rc);
        }
    }

    if (rc == GT_OK)
    {
        osNonSharedGlobalVars.osNonVolatileDb.clientIsInitialised = GT_TRUE;
    }

    rc = firstProcessDiscoveryUnLock(FIRST_CLIENT_DISCOVERY_SEM_NAME);
    if (rc != GT_OK)
    {
        fprintf(stderr, "firstProcessDiscoveryUnLock() failed, rc=%d\n",rc);
    }

#ifndef ASIC_SIMULATION /* Applicable ONLY for BM */

     /*in anycase try to map PCIe .May successed if cpssInit is called by someone.*/
     rc = extDrvInitDrv(NULL);
     if (rc != GT_OK)
     {
         fprintf(stderr, "extDrvInitDrv() failed, rc=%d\n", rc);
         return;
     }
#endif


}


