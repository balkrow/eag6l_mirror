/*******************************************************************************
*              (c), Copyright 2013, Marvell International Ltd.                 *
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
* @file osGlobalSharedDb.c
*
* @brief This file provides handling of global variables DB.
*
* @version   1
********************************************************************************
*/
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <cpssCommon/cpssPresteraDefs.h>
#include <cpss/extServices/private/prvCpssBindFunc.h>
#include <cpss/common/config/private/prvCpssConfigTypes.h>
#include <gtOs/gtOsIo.h>
#include <gtOs/gtOsMem.h>
#include <gtOs/gtOsStr.h>
#include <gtOs/gtOsSharedData.h>
#include <gtOs/globalDb/gtOsGlobalDb.h>
#include <semaphore.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <gtOs/globalDb/gtOsGlobalDbInterface.h>
#include <stdio.h>
#include <sys/mman.h>
#include <gtOs/gtOsSharedMemoryRemapper.h>
#include <gtOs/gtOsGen.h>
#include <gtExtDrv/drivers/gtDmaLayer.h>

typedef enum
{
    PRV_CPSS_APP_REF_MODE_APP_REF_MODE_OFF_E,
    PRV_CPSS_APP_REF_MODE_APP_REF_MODE_ON_NON_MIXED_E,
    PRV_CPSS_APP_REF_MODE_APP_REF_MODE_ON_MIXED_E
}
PRV_CPSS_APP_REF_MODE_ENT;




#ifndef MAP_32BIT
#define MAP_32BIT 0x40
#endif

#ifdef ASIC_SIMULATION
GT_BOOL  simulationDistributedActAsAsicGet(void);
#endif
extern GT_STATUS osMemGlobalDbShmemInit
(
    IN  GT_U32      size,
    IN  GT_CHAR_PTR  name,
    OUT GT_BOOL * initDataSegmentPtr,
    OUT GT_VOID **sharedGlobalVarsPtrPtr
);


extern GT_STATUS osMemGlobalDbShmemUnlink
(
    IN  GT_CHAR_PTR  name
);

extern GT_STATUS osPrintDmaPhysicalBase(GT_VOID);

GT_STATUS prvOsHelperGlobalDbProcListSync
(
    GT_BOOL printResult
);

void  osGlobalSharedDbCleanUp(void);

/*add here module initializers*/

#include <gtOs/globalDb/init/gtOsGlobalDbEnablerInitVars.h>


#define HELPER_SHM_FILENAME "MAINOS_SHM_DATA"


#define SHARED_MEMORY_SEM_NAME   "OS_SHM_INIT_SEM"


PRV_OS_SHARED_GLOBAL_DB       *osSharedGlobalVarsPtr __SHARED_DATA_MAINOS = NULL;
PRV_OS_SHARED_GLOBAL_DB       osSharedGlobalVars __SHARED_DATA_MAINOS ;

PRV_OS_NON_SHARED_GLOBAL_DB   *osNonSharedGlobalVarsPtr = NULL;
PRV_OS_NON_SHARED_GLOBAL_DB    osNonSharedGlobalVars;

extern  char* shrAddNameSuffix(const char* name, char* buffer, int bufsize);

static GT_BOOL  osGlobalDbPicGet
(
    GT_VOID
)
{
#ifdef PIC_BUILD
    return GT_TRUE;
#else
    return GT_FALSE;
#endif
}
GT_VOID osGlobalDbNonSharedDbPpMappingStageDone
(
   GT_VOID

)
{
    osNonSharedGlobalVars.osNonVolatileDb.ppMappingStageDone = GT_TRUE;
}


GT_VOID osGlobalDbNonSharedDbPciePpMappingSet
(
   const char *fname,
   void *vaddr
)
{
    /*Get device info*/

    GT_U32 pciDomain, pciBus, pciDev, pciFunc;
    GT_CHAR * str ="/sys/bus/pci/devices/";
    GT_U32  i;
    GT_BOOL found = GT_FALSE;

    fname+=strlen(str);

    SHM_PRINT("Search  mapping %s (adress %p )in noKmMappingsList\n",fname,vaddr );

    sscanf(fname,"%x",&pciDomain);
    fname+=5;
    sscanf(fname,"%x",&pciBus);
    fname+=3;
    sscanf(fname,"%x",&pciDev);
    fname+=3;
    sscanf(fname,"%x",&pciFunc);

    for(i=0;i<osNonSharedGlobalVars.osNonVolatileDb.noKmMappingsNum;i++)
    {
        if(osNonSharedGlobalVars.osNonVolatileDb.noKmMappingsList[i].hwAddr.busNo == ((pciDomain << 8) | pciBus))
        {
            if(osNonSharedGlobalVars.osNonVolatileDb.noKmMappingsList[i].hwAddr.devSel == pciDev)
            {
                if(osNonSharedGlobalVars.osNonVolatileDb.noKmMappingsList[i].hwAddr.funcNo == pciFunc)
                {
                    found = GT_TRUE;
                    break;
                }
            }
        }
    }

    if(NULL!=strstr(fname,"resource0"))
    {
       osNonSharedGlobalVars.osNonVolatileDb.noKmMappingsList[i].resource.cnm.start = (GT_UINTPTR)vaddr;
    }
    else   if(NULL!=strstr(fname,"resource2"))
    {
       osNonSharedGlobalVars.osNonVolatileDb.noKmMappingsList[i].resource.switching.start = (GT_UINTPTR)vaddr;
    }
    else   if(NULL!=strstr(fname,"resource4"))
    {
       osNonSharedGlobalVars.osNonVolatileDb.noKmMappingsList[i].resource.resetAndInitController.start = (GT_UINTPTR)vaddr;
        /*legacy devices. See ac3_pci_sysmap ,bc2_pci_sysmap or bobk_pci_sysmap*/
       osNonSharedGlobalVars.osNonVolatileDb.noKmMappingsList[i].resource.sram.start = (GT_UINTPTR)vaddr +_2M;
    }

    if(GT_FALSE==found)
    {
        osNonSharedGlobalVars.osNonVolatileDb.noKmMappingsNum ++;
        osNonSharedGlobalVars.osNonVolatileDb.noKmMappingsList[i].hwAddr.busNo = ((pciDomain << 8) | pciBus);
        osNonSharedGlobalVars.osNonVolatileDb.noKmMappingsList[i].hwAddr.devSel = pciDev;
        osNonSharedGlobalVars.osNonVolatileDb.noKmMappingsList[i].hwAddr.funcNo = pciFunc;
    }
    SHM_PRINT("found %d noKmMappingsNum %d\n",found,osNonSharedGlobalVars.osNonVolatileDb.noKmMappingsNum );
}




GT_VOID osGlobalDbNonSharedDbMbusPpMappingSet
(
   GT_U32 res,
   void *vaddr
)
{
    /*Get device info*/

    PRV_OS_SHM_LOG_ENTER

    GT_U32 pciBus, pciDev, pciFunc;
    GT_U32  i;
    GT_BOOL found = GT_FALSE;

    pciBus=0xFF;
    pciDev=pciFunc=0xFF;


    SHM_PRINT("Search res %d  (adress %p )in noKmMappingsList\n",res,vaddr );
    for(i=0;i<osNonSharedGlobalVars.osNonVolatileDb.noKmMappingsNum;i++)
    {
        if(osNonSharedGlobalVars.osNonVolatileDb.noKmMappingsList[i].hwAddr.busNo == pciBus)
        {
            if(osNonSharedGlobalVars.osNonVolatileDb.noKmMappingsList[i].hwAddr.devSel == pciDev)
            {
                if(osNonSharedGlobalVars.osNonVolatileDb.noKmMappingsList[i].hwAddr.funcNo == pciFunc)
                {
                    found = GT_TRUE;
                    break;
                }
            }
        }
    }

    if(1==res)
    {
       osNonSharedGlobalVars.osNonVolatileDb.noKmMappingsList[i].resource.cnm.start = (GT_UINTPTR)vaddr;
    }
    else   if(2==res)
    {
       osNonSharedGlobalVars.osNonVolatileDb.noKmMappingsList[i].resource.switching.start = (GT_UINTPTR)vaddr;
    }
    else   if(3==res)
    {
       osNonSharedGlobalVars.osNonVolatileDb.noKmMappingsList[i].resource.resetAndInitController.start = (GT_UINTPTR)vaddr;
    }

    if(GT_FALSE==found)
    {
        osNonSharedGlobalVars.osNonVolatileDb.noKmMappingsNum ++;
        osNonSharedGlobalVars.osNonVolatileDb.noKmMappingsList[i].hwAddr.busNo = pciBus;
        osNonSharedGlobalVars.osNonVolatileDb.noKmMappingsList[i].hwAddr.devSel = pciDev;
        osNonSharedGlobalVars.osNonVolatileDb.noKmMappingsList[i].hwAddr.funcNo = pciFunc;
        osNonSharedGlobalVars.osNonVolatileDb.noKmMappingsList[i].busType = CPSS_HW_INFO_BUS_TYPE_MBUS_E;
    }
    SHM_PRINT(" found %d noKmMappingsNum %d\n",found,osNonSharedGlobalVars.osNonVolatileDb.noKmMappingsNum );

    PRV_OS_SHM_LOG_EXIT
}



GT_STATUS prvOsPrintAslr
(
    GT_VOID
)
{
    FILE * fd;
    GT_32  lvl;
    GT_CHAR line[256];

    if( (fd = fopen("/proc/sys/kernel/randomize_va_space", "r")) == NULL)
    {
        perror("  Failed to open /proc/sys/kernel/randomize_va_space for write");
        return GT_FAIL;
    }

    if(fscanf(fd,"%d",&lvl))
    {
        SHM_PRINT("/proc/sys/kernel/randomize_va_space is %d\n",lvl);
    }

    fclose(fd);

    if(osNonSharedGlobalVars.osNonVolatileDb.verboseMode)
    {
        SHM_PRINT("Dump memory map\n");
        if( (fd = fopen("/proc/self/maps", "r")) == NULL)
        {
            perror("  Failed to open /proc/self/maps");
            return GT_FAIL;
        }

        while (fgets( line, 256, fd ) != NULL )
        {
           osPrintf("%s\n", line);
        }

        fclose(fd);
    }


    return GT_OK;
}


/*******************************************************************************
* osGetPid
*
* DESCRIPTION:
*       Returns the process ID (PID) of the calling process.
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
* COMMENTS:This function  is always successful.
*
*******************************************************************************/
GT_U32 osGetPid
(
    GT_VOID
)
{
    return (GT_U32)getpid();
}

/*******************************************************************************
* osAtExit
*
* DESCRIPTION:
*       Registers the given function to be called
*       at normal process termination, either via exit(3) or via return
*       from the program's main().  Functions so registered are called in
*       the reverse order of their registration; no arguments are passed.
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
* COMMENTS:  The same function may be registered multiple times: it is called
*       once for each registration
*
*******************************************************************************/

GT_STATUS osAtExit
(
    OS_AT_EXIT_FUNC func
)
{

#if defined SHARED_MEMORY
    GT_U32 size;

    size = osNonSharedGlobalVars.osNonVolatileDb.exitFuncDepth;

    if(size>=AT_EXIT_DEPTH_MAX)
    {
        return GT_FULL;
    }
    osNonSharedGlobalVars.osNonVolatileDb.exitFuncDepth++;
    osNonSharedGlobalVars.osNonVolatileDb.exitFuncs[size] = func;
    return GT_OK;

#else
    GT_U32 res;
    res = atexit(func);
    return res==0?GT_OK:GT_FAIL;
#endif
}

/*******************************************************************************
* osIsProcAlive
*
* DESCRIPTION:
*       Determine if process is alive
*
* INPUTS:
*
*        None
*
* OUTPUTS:
*       GT_TRUE - process is  alive
*       GT_FALSE -process is not alive
*
* RETURNS:
*      NONE
*
* COMMENTS:  The same function may be registered multiple times: it is called
*       once for each registration
*
*******************************************************************************/

GT_BOOL osIsProcAlive
(
   GT_U32 pid
)
{
    GT_BOOL alive=GT_FALSE;
    GT_CHAR buff[128];
    GT_32 procFd;

    /*  check if /proc/PID/stat file exists */
    osSprintf(buff, "/proc/%d/stat",pid);
    procFd = open(buff, O_RDONLY, 0);
    if (procFd >= 0)
    {
        close(procFd);
        alive=GT_TRUE;
    }

    return alive;
}


/**
* @internal osGlobalSharedDbAddProcess function
* @endinternal
*
* @brief add process to pid list(debug)
*  Return GT_TRUE if process was added,GT_FALSE otherwise
*
*/

GT_BOOL  osGlobalSharedDbAddProcess
(
    GT_VOID
)
{
    GT_U32 index = 0,freeIndex=0;
    GT_U32 myPid = osGetPid();
    GT_BOOL freeIndexFound = GT_FALSE;
    GT_BOOL alreadyExist= GT_FALSE;

    PRV_OS_SHM_LOG_ENTER

    SHM_PRINT("osSharedGlobalVarsPtr %p\n",osSharedGlobalVarsPtr);

    if((osSharedGlobalVarsPtr!=NULL))
    {
         SHM_PRINT("Add proc %d  \n",myPid );

         osMutexLock(osSharedGlobalVarsPtr->processDbMtx);

        (void)prvOsHelperGlobalDbProcListSync(GT_FALSE);

        for(index=0;index<MAX_PIDS_SIZE;index++)
        {
            if(GT_FALSE==freeIndexFound)
            {
                if(osSharedGlobalVarsPtr->pids[index]==0)
                {
                    freeIndexFound = GT_TRUE;
                    freeIndex = index;
                    SHM_PRINT("freeIndex %d   \n",index );
                }
             }

            if(osSharedGlobalVarsPtr->pids[index] == myPid)
            {
                alreadyExist = GT_TRUE;
                SHM_PRINT("Proc %d  already exist \n",myPid );
                break;
            }
        }

        if((GT_FALSE==alreadyExist)&&(GT_TRUE==freeIndexFound))
        {
            SHM_PRINT("Adding proc %d  .Already exist is false\n",myPid );

            osSharedGlobalVarsPtr->pids[freeIndex]= myPid;
            if(osSharedGlobalVarsPtr->clientNum<MAX_PIDS_SIZE-1)
            {
                /*support 64 clients*/
                osSharedGlobalVarsPtr->clientNum++;
            }
        }

        osMutexUnlock(osSharedGlobalVarsPtr->processDbMtx);

        SHM_PRINT("Current client number  %d \n",osSharedGlobalVarsPtr->clientNum);
     }

    PRV_OS_SHM_LOG_EXIT

    return alreadyExist?GT_FALSE:GT_TRUE;


}



/**
* @internal cpssGlobalSharedDbRemoveProcess function
* @endinternal
*
* @brief remove process from pid list(debug)
*
*/

static GT_VOID osGlobalSharedDbRemoveProcess
(
    GT_VOID
)
{
    GT_U32 index = 0;
    GT_U32 myPid =osGetPid();

    PRV_OS_SHM_LOG_ENTER

    SHM_PRINT("osSharedGlobalVarsPtr %p\n",osSharedGlobalVarsPtr);

    (void)prvOsHelperGlobalDbProcListSync(GT_FALSE);

    if(osSharedGlobalVarsPtr!=NULL)
    {
        SHM_PRINT("Remove proc %d\n",myPid);

        osMutexLock(osSharedGlobalVarsPtr->processDbMtx);

        for(index=0;index<MAX_PIDS_SIZE;index++)
        {
            if(osSharedGlobalVarsPtr->pids[index]==myPid)
            {
                osSharedGlobalVarsPtr->pids[index]=0;
                osSharedGlobalVarsPtr->clientNum--;
                break;
            }
        }

        osMutexUnlock(osSharedGlobalVarsPtr->processDbMtx);

        SHM_PRINT("Current client number  %d \n",osSharedGlobalVarsPtr->clientNum);
     }


    PRV_OS_SHM_LOG_EXIT
}



/**
 * @internal osGlobalDbNamedSemOpen function
 * @endinternal
 *
 * @brief   creates a new POSIX semaphore or opens an existing
       semaphore.  The semaphore is identified by name.
       The semaphore purpose is to protect global data base access.
 *
 */
GT_STATUS osGlobalDbNamedSemOpen
(
    const GT_CHAR_PTR name,
    sem_t** semid
)
{
        GT_CHAR_PTR tmp;

#if defined SHARED_MEMORY
         GT_CHAR buffer[256];
         shrAddNameSuffix(name,buffer,sizeof(buffer));
         tmp = buffer;
#else
         tmp = (GT_CHAR_PTR)name;
#endif


    if((*semid = sem_open(tmp, O_CREAT, 0666, 1)) == SEM_FAILED)
    {
        osPrintf( "sem_open() : %s\n", strerror(errno));
        return GT_FAIL;
    }
    return GT_OK;
}

/**
 * @internal osGlobalDbNamedSemWait function
 * @endinternal
 *
 * @brief   Locks the semaphore pointed to by sem.
       The semaphore purpose is to protect global data base access.
 *
 */
GT_STATUS osGlobalDbNamedSemWait
(
    sem_t*  semid
)
{
    if(sem_wait(semid) == -1)
    {
        osPrintf(  "sem_wait() : %s\n", strerror(errno));
        return GT_FAIL;
    }
    return GT_OK;
}
/**
 * @internal osGlobalDbNamedSemWait function
 * @endinternal
 *
 * @brief   Unlocks the semaphore pointed to by sem.
       The semaphore purpose is to protect global data base access.
 *
 */
GT_STATUS osGlobalDbNamedSemPost
(
    sem_t* semid
)
{
    if(sem_post(semid) == -1)
    {
        osPrintf("sem_post() : %s\n", strerror(errno));
        return GT_FAIL;
    }

    return GT_OK;
}
/**
 * @internal osGlobalDbNamedSemClose function
 * @endinternal
 *
 * @brief   Closes the named semaphore referred to by sem,
       allowing any resources that the system has allocated to the
       calling process for this semaphore to be freed.
       The semaphore purpose is to protect global data base access.
 *
 */
GT_STATUS osGlobalDbNamedSemClose
(
    sem_t* semid
)
{

    if(sem_close(semid) == -1)
    {
        osPrintf("sem_close() : %s\n", strerror(errno));
        return GT_FAIL;
    }
    return GT_OK;
}

/**
* @internal osGlobalDbNamedSemUnlink function
* @endinternal
*
* @brief   removes the named semaphore referred to by name.
*     The semaphore name is removed immediately.  The semaphore is
       destroyed once all other processes that have the semaphore open
       close it.
       The semaphore purpose is to protect global data base access.
*
*/
GT_STATUS osGlobalDbNamedSemUnlink
(
    GT_CHAR_PTR name
)
{

    GT_CHAR_PTR tmp;

#if defined SHARED_MEMORY
     GT_CHAR buffer[256];
     shrAddNameSuffix(name,buffer,sizeof(buffer));
     tmp = buffer;
#else
     tmp = (GT_CHAR_PTR)name;
#endif

    if(sem_unlink(tmp) < 0)
    {
        if(errno != ENOENT && 0 != errno)
        {
            osPrintf("sem_unlink() : %s %s\n", tmp, strerror(errno));
            return GT_FAIL;
        }
    }
    return GT_OK;
}

static GT_STATUS osGlobalSharedDbLockPtrGet
(
    sem_t** dbInitSemPtr /* named semaphore  to protect data base initialization in shared lib mode  */
)
{
  GT_STATUS rc;

  if(NULL == dbInitSemPtr)
  {
    return GT_BAD_PTR;
  }

  if(NULL == PRV_HELPER_NON_SHARED_GLOBAL_VAR_DIRECT_GET(osNonVolatileDb.dbInitSem))
  {
      /* Open named semaphore to guard  code from other clients. */
       rc = osGlobalDbNamedSemOpen(SHARED_MEMORY_SEM_NAME,dbInitSemPtr);
       if(rc!=GT_OK)
       {
            return rc;
       }

       PRV_HELPER_NON_SHARED_GLOBAL_VAR_DIRECT_SET(osNonVolatileDb.dbInitSem,(GT_VOID * ) (*dbInitSemPtr));
  }
  else
  {
      *dbInitSemPtr = (sem_t * )PRV_HELPER_NON_SHARED_GLOBAL_VAR_DIRECT_GET(osNonVolatileDb.dbInitSem);
  }

  return GT_OK;

}

/**
* @internal osGlobalSharedDbLock function
* @endinternal
*
* @brief  Lock global data base access.
*  Use named semaphore.
*
*/
GT_STATUS osGlobalSharedDbLock
(
    GT_VOID
)
{
    sem_t* dbInitSem; /* named semaphore  to protect data base initialization in shared lib mode  */
    GT_STATUS rc = GT_OK;
    GT_U32    bmp = 0x0;

    /*no race condition here  in updating this field ,
                    since either all process use shared lib or not*/

    /*if it is not in shared library mode ,then do nothing*/
    if(GT_TRUE == osSharedGlobalVarsPtr->sharedMemoryUsed)
    {

      /* register shared memory content cleaner in case init will go wrong */
        if (atexit(osGlobalSharedDbCleanUp))
        {
            fprintf(stderr, "Cannot register osGlobalSharedDbCleanUp, error=%d\n", errno);
        }

         /* Get named semaphore */
         rc = osGlobalSharedDbLockPtrGet(&dbInitSem);
         if(rc!=GT_OK)
         {
              return rc;
         }

         rc = osGlobalDbNamedSemWait(dbInitSem);
         if(rc!=GT_OK)
         {
              return rc;
         }

         bmp |=(DB_INIT_SEM_OWNER_BIT);
         PRV_HELPER_NON_SHARED_GLOBAL_VAR_DIRECT_SET(osNonVolatileDb.semOwnershipBmp,bmp);
     }

     return rc;
}


/**
* @internal osGlobalSharedDbUnLock function
* @endinternal
*
* @brief  Unlock global data base access.
*  Use named semaphore.
*
*/
GT_STATUS osGlobalSharedDbUnLock
(
    GT_VOID
)
{
    sem_t* dbInitSem; /* named semaphore  to protect data base initialization in shared lib mode  */
    GT_STATUS rc = GT_OK;
    GT_U32 bmp = 0x0;

    /*if it is not in shared library mode ,then do nothing*/
    if(GT_TRUE == osSharedGlobalVarsPtr->sharedMemoryUsed)
    {

        /* Get named semaphore */
        rc = osGlobalSharedDbLockPtrGet(&dbInitSem);
        if(rc!=GT_OK)
        {
             return rc;
        }

        rc = osGlobalDbNamedSemPost(dbInitSem);
        if(rc!=GT_OK)
        {
           dbInitSem=NULL;
           return rc;
        }

        bmp &=(~DB_INIT_SEM_OWNER_BIT);
        PRV_HELPER_NON_SHARED_GLOBAL_VAR_DIRECT_SET(osNonVolatileDb.semOwnershipBmp,bmp);

     }

    return rc;
}


void  osGlobalSharedDbCleanUp(void)
{
    GT_STATUS rc = GT_OK;
    if(DB_INIT_SEM_OWNER_BIT&
        PRV_HELPER_NON_SHARED_GLOBAL_VAR_DIRECT_GET(osNonVolatileDb.semOwnershipBmp))
    {
           /*in case something will go wrong during the init*/
        rc = osGlobalSharedDbUnLock();
        fprintf(stderr, "*** osGlobalSharedDbCleanUp() called rc = %d\n",rc);
    }
}


/**
 * @internal osNonSharedDbDataSectionInit function
 * @endinternal
 *
 * @brief   Initialize global non-shared variables .
 *
 */
static GT_STATUS osNonSharedDbDataSectionInit
(
    GT_VOID
)
{
    PRV_OS_NON_SHARED_NON_VOLATILE_DB nonVolatileDb;
    GT_U32 size = sizeof(PRV_OS_NON_SHARED_GLOBAL_DB);
    /*PRV_OS_NON_SHARED_NON_VOLATILE_DB should not be cleared*/
    osMemCpy(&nonVolatileDb,&osNonSharedGlobalVarsPtr->osNonVolatileDb,sizeof(PRV_OS_NON_SHARED_NON_VOLATILE_DB));
    /*helper non-shared*/
    osMemSet(osNonSharedGlobalVarsPtr,0,size);
    osNonSharedGlobalVarsPtr->magic = NON_SHARED_DB_MAGIC;
    osMemCpy(&osNonSharedGlobalVarsPtr->osNonVolatileDb,&nonVolatileDb,sizeof(PRV_OS_NON_SHARED_NON_VOLATILE_DB));

    /*add here modules initializers*/
    osHelperGlobalNonSharedDbEnablerModDataSectionInit();

    return GT_OK;
}



/**
 * @internal osSharedDbDataSectionInit function
 * @endinternal
 *
 * @brief   Initialize global shared variables .
 *
 */
static GT_STATUS osSharedDbDataSectionInit
(
    const char * callerFuncName,
    GT_U32       callerLine
)
{

    if(callerFuncName!=NULL)
    {
       SHM_PRINT("Data section init called from  %s line: %d\n",callerFuncName,callerLine);
    }

    osMemSet(osSharedGlobalVarsPtr,0,sizeof(PRV_OS_SHARED_GLOBAL_DB));
    /*helper shared*/
    osSharedGlobalVarsPtr->magic = SHARED_DB_MAGIC;
    /*add here modules initializers*/
    osHelperGlobalSharedDbEnablerModDataSectionInit();

    return GT_OK;
}

/**
* @internal gtOsHelperGlobalDbInit function
* @endinternal
*
* @brief                       Initialize OS layer  global variables data base.
*                              Global variables data base is allocated.
*                              In case useSharedMem equal GT_TRUE then also shared memory is allocated for shared variables.
*                              After memory is allocated ,the global variables are being initialized to init values.
* @param[in] aslrSupport    - whether shared memory should be used for shared variables.
*                              in case equal GT_FALSE static  memory allocation is used for shared variables,
*                              otherwise shared memory is used s used for shared variables,
*
* @retval GT_OK              - on success
* @retval GT_FAIL            - on error
*/
GT_STATUS gtOsHelperGlobalDbInit
(
    GT_BOOL aslrSupport,
    const char * callerFuncName,
    GT_U32       callerLine
)
{

   GT_STATUS rc = GT_OK;
   GT_BOOL   initDataSegment = GT_FALSE;
   GT_BOOL   sharedMemory = GT_FALSE;
   GT_U32    pid;
   PRV_CPSS_APP_REF_MODE_ENT   appRefMode = PRV_CPSS_APP_REF_MODE_APP_REF_MODE_OFF_E;
   GT_CHAR * str;
#ifdef SHARED_MEMORY
    sharedMemory = GT_TRUE;
#ifndef ASIC_SIMULATION
    SECTION_STC cpssSection;
    SECTION_STC mainOsSection;
#endif
#endif

#ifdef CPSS_APP_PLATFORM_REFERENCE
    appRefMode = PRV_CPSS_APP_REF_MODE_APP_REF_MODE_ON_NON_MIXED_E;
#ifdef MIXED_MODE
    appRefMode = PRV_CPSS_APP_REF_MODE_APP_REF_MODE_ON_MIXED_E;
#endif

#endif


   if(NULL == osNonSharedGlobalVarsPtr)
   {
       osNonSharedGlobalVarsPtr = &osNonSharedGlobalVars;
       rc = osNonSharedDbDataSectionInit();
       if(rc!=GT_OK)
       {
         return rc;
       }
   }


   if(NULL == osSharedGlobalVarsPtr)
   {
 #ifdef ASIC_SIMULATION
        /*asic side does not care about ASLR so do not print it*/
       if(GT_FALSE==simulationDistributedActAsAsicGet())
 #endif
       {
           osPrintf( "SHLIB                :%s\n",sharedMemory?"Y":"N");
           if(GT_TRUE==sharedMemory)
           {
            osPrintf("ASLR                 :%s\n",aslrSupport?"Y":"N");
            osPrintf("PIC build            :%s\n",osGlobalDbPicGet()?"Y":"N");
            osPrintf("Malloc pool size(Mb) :%3d\n",osNonSharedGlobalVars.osNonVolatileDb.mallocPoolSize>>20);


           }
           switch(appRefMode)
           {
                case PRV_CPSS_APP_REF_MODE_APP_REF_MODE_ON_NON_MIXED_E:
                    str = "APP_REF              :Y\nMixed mode           :N\n";
                    break;
                case PRV_CPSS_APP_REF_MODE_APP_REF_MODE_ON_MIXED_E:
                    str = "APP_REF              :Y\nMixed mode           :Y\n";
                    break;
                default:
                    str = "APP_REF              :N\n";
                    break;
           }
           osPrintf(str);

           /* print regardless to shared-memory mode */
#ifndef ASIC_SIMULATION
           osPrintf("DMA mode (if KO needed):%s (for mvDmaDrv.ko)\n",
#if defined(CONFIG_V2_DMA)
                "v2");
#else
                "v1");
#endif
#endif /*! ASIC_SIMULATION*/
       }
       if(GT_FALSE == aslrSupport)
       {
           osSharedGlobalVarsPtr = &osSharedGlobalVars;
           initDataSegment = GT_TRUE;
       }
       else
       {
          rc = osMemGlobalDbShmemInit(sizeof(PRV_OS_SHARED_GLOBAL_DB),HELPER_SHM_FILENAME,
            &initDataSegment,(GT_VOID **)&osSharedGlobalVarsPtr);
          if(rc != GT_OK)
          {
              return rc;
          }
          pid = osGetPid();

          SHM_PRINT("OS SHMEM heap allocated at %p for  PID %d\n",osSharedGlobalVarsPtr,pid);
#if defined SHARED_MEMORY && !defined ASIC_SIMULATION
          rc = findAdressesOfSections(
              &cpssSection,
              &mainOsSection,
              GT_TRUE
          );
          if(rc != GT_OK)
          {
              return rc;
          }

          SHM_PRINT("=> OS SHMEM text section allocated at %p for  PID %d\n",mainOsSection.startPtr,pid);
          SHM_PRINT("=> CPSS SHMEM text section allocated at %p for  PID %d\n",cpssSection.startPtr,pid);
#endif
          rc = prvOsPrintAslr();
          if(rc != GT_OK)
          {
              return rc;
          }

       }

        /*no race condition here  in updating this field ,
                    since either all process use shared lib or not*/
        osSharedGlobalVarsPtr->sharedMemoryUsed = aslrSupport;

        /*only one  process initialize data segment*/
        rc = osGlobalSharedDbLock();
        if(rc!=GT_OK)
        {
             return rc;
        }

        if((GT_TRUE == initDataSegment)&&(GT_FALSE == osSharedGlobalVarsPtr->dbInitialized))
        {
           rc = osSharedDbDataSectionInit(callerFuncName,callerLine);
           if(rc!=GT_OK)
           {
                return rc;
           }

           osSharedGlobalVarsPtr->initializerPid = osGetPid();
           osSharedGlobalVarsPtr->dbInitialized = GT_TRUE;
           osSharedGlobalVarsPtr->sharedMemoryUsed = aslrSupport;

        }
        else
        {
          /*shared memory already existed,check magic*/
          if(osSharedGlobalVarsPtr->magic != SHARED_DB_MAGIC)
          {
            /*something went wrong*/
            return GT_BAD_STATE;
          }

          /*DMA is initialized.Print it.*/
          osPrintDmaPhysicalBase();
        }

        rc = osGlobalSharedDbUnLock();
        if(rc!=GT_OK)
        {
            return rc;
        }

   }

   if(osSharedGlobalVarsPtr->processDbMtx==0)
   {
        /* Create the sync semaphore */
        if (osMutexCreate("processDbMtx", &(osSharedGlobalVarsPtr->processDbMtx)) != GT_OK)
        {
            return GT_FAIL;
        }

        SHM_PRINT("processDbMtx created with id %d   \n",osSharedGlobalVarsPtr->processDbMtx );
    }
    else
    {
        SHM_PRINT("processDbMtx already exist with id %d   \n",osSharedGlobalVarsPtr->processDbMtx );
    }
    if(GT_TRUE== osGlobalSharedDbAddProcess())
    {

        rc = osAtExit(osGlobalSharedDbRemoveProcess);
        if(rc!=GT_OK)
        {
            return rc;
        }
     }

   return rc;
}

/**
* @internal gtOsHelperGlobalDbDestroy function
* @endinternal
*
* @brief               De-Initialize OS layer  global variables data base.
*                      Global variables data base is deallocated.
*                      In case shared memory is used for shared variables,shared memory is unlinked.
*
* @param[in] unlink   - whether shared memory should be unlinked.
*
*
* @retval GT_OK              - on success
* @retval GT_FAIL            - on error
*/

GT_STATUS gtOsHelperGlobalDbDestroy
(
    GT_BOOL unlink
)
{

    GT_STATUS rc = GT_OK;

    sem_t* dbInitSem; /* named semaphore  to protect data base initialization in shared lib mode  */

    if (osSharedGlobalVarsPtr != NULL)
    {
        if(GT_TRUE == osSharedGlobalVarsPtr->sharedMemoryUsed)
        {
            if(GT_TRUE == unlink)
            {
               /* Get named semaphore */
                rc = osGlobalSharedDbLockPtrGet(&dbInitSem);
                if(rc!=GT_OK)
                {
                     return rc;
                }

                rc = osGlobalDbNamedSemClose(dbInitSem);
                if(rc!=GT_OK)
                {
                   return rc;
                }

                /* Remove named semaphore for clean-up */
                rc = osGlobalDbNamedSemUnlink(SHARED_MEMORY_SEM_NAME);
                if(rc!=GT_OK)
                {
                     return rc;
                }
            }
        }
    }

    osSharedGlobalVarsPtr = NULL;
    osNonSharedGlobalVarsPtr = NULL;

    return rc;
}

/**
* @internal prvOsHelperGlobalDbProcListSync function
* @endinternal
*
* @brief             Remove ded processes from client list
*
* @param[in] printResult   - whether to print result
*
*
* @retval GT_OK              - on success
* @retval GT_FAIL            - on error
*/

GT_STATUS prvOsHelperGlobalDbProcListSync
(
    GT_BOOL printResult
)
{
    GT_U32 i;
    GT_BOOL isAlive;

    for(i=0;(NULL!=osSharedGlobalVarsPtr)&&(i<MAX_PIDS_SIZE);i++)
    {
         if(osSharedGlobalVarsPtr->pids[i]!=0)
         {
           isAlive = osIsProcAlive(osSharedGlobalVarsPtr->pids[i]);

           if(GT_TRUE==printResult)
           {
                osPrintf("[%c] Process %d [0x%x] is %s \n",
                    osSharedGlobalVarsPtr->initializerPid==osSharedGlobalVarsPtr->pids[i]?
                    'M':'S',
                    osSharedGlobalVarsPtr->pids[i],
                    osSharedGlobalVarsPtr->pids[i],isAlive?"alive":"dead");
           }

           if(GT_FALSE==isAlive)
           {
               osSharedGlobalVarsPtr->pids[i]=0;
               osSharedGlobalVarsPtr->clientNum--;
           }
         }
    }

    return GT_OK;
}


/**
* @internal prvOsHelperGlobalDbDump function
* @endinternal
*
* @brief   Dump shared global DB attributes
*
* @retval GT_OK                    - anycase
*/

GT_STATUS prvOsHelperGlobalDbDump
(
    GT_VOID
)
{
    GT_U32 myPid;
    GT_U32 i,win;

    myPid = osGetPid();

    osPrintf("Shared globals DB:\n");
    osPrintf("==================\n");

    if(osSharedGlobalVarsPtr)
    {
        osPrintf("Addr            :%p\n",osSharedGlobalVarsPtr);
        osPrintf("Magic           :0x%x\n",osSharedGlobalVarsPtr->magic);
        osPrintf("Master  Pid    :0x%x\n",osSharedGlobalVarsPtr->initializerPid);
        osPrintf("Allocated by me:%d\n",(myPid==osSharedGlobalVarsPtr->initializerPid)?1:0);
        osPrintf("clientNum   :   %d\n",osSharedGlobalVarsPtr->clientNum);
        osPrintf("SHMEM used     :%d\n",osSharedGlobalVarsPtr->sharedMemoryUsed);
    }
    else
    {
        osPrintf("Not initialized\n");
    }

    osPrintf("Non-shared globals DB:\n");
    osPrintf("======================\n");

    if(osNonSharedGlobalVarsPtr)
    {
        osPrintf("Addr            :%p\n",osNonSharedGlobalVarsPtr);
        osPrintf("Magic           :0x%x\n",osNonSharedGlobalVarsPtr->magic);
        osPrintf("ppMappingStageDone:0x%x\n",osNonSharedGlobalVarsPtr->osNonVolatileDb.ppMappingStageDone);
        osPrintf("myPid           :0x%x\n",myPid);
        osPrintf("dmaConfigNumOfActiveWindows :0x%x\n",osNonSharedGlobalVarsPtr->osNonVolatileDb.dmaConfigNumOfActiveWindows);
        for(win = 0 ; win < osNonSharedGlobalVarsPtr->osNonVolatileDb.dmaConfigNumOfActiveWindows; win++)
        {
            if(win == 0 || osNonSharedGlobalVarsPtr->osNonVolatileDb.dmaConfig[win].prvExtDrvDmaLen)
            {
                osPrintf("window[%d]:Dma virtual addr:0x%p\n",win,osNonSharedGlobalVarsPtr->osNonVolatileDb.dmaConfig[win].prvExtDrvDmaPtr);
                osPrintf("window[%d]:Dma size        :%d  \n",win,osNonSharedGlobalVarsPtr->osNonVolatileDb.dmaConfig[win].prvExtDrvDmaLen);
            }
        }
    }
    else
    {
        osPrintf("Not initialized\n");
    }

    if(osSharedGlobalVarsPtr && osSharedGlobalVarsPtr->sharedMemoryUsed)
    {
        osPrintf("Used by processes\n");
        for(i=0;i<MAX_PIDS_SIZE;i++)
        {
            if(osSharedGlobalVarsPtr->pids[i]!=0)
            {
              osPrintf("%d pid %d [0x%x] \n",i,osSharedGlobalVarsPtr->pids[i],osSharedGlobalVarsPtr->pids[i]);
            }
        }
    }

    return GT_OK;
}


GT_STATUS prvOsHelperGlobalDbInfoGet
(
    GT_U32  *myPidPtr,
    GT_U32  *clientNumPtr,
    GT_BOOL *globalDbToSharedMemPtr,
    GT_BOOL *ppMappingStageDonePtr,
    GT_U32  *initializerPidPtr
)
{
    GT_STATUS rc = GT_BAD_PTR;

    if(osSharedGlobalVarsPtr&&osNonSharedGlobalVarsPtr)
    {
        *myPidPtr = osGetPid();
        *clientNumPtr = osSharedGlobalVarsPtr->clientNum;
        *globalDbToSharedMemPtr = osSharedGlobalVarsPtr->sharedMemoryUsed;
        *ppMappingStageDonePtr = osNonSharedGlobalVarsPtr->osNonVolatileDb.ppMappingStageDone;
        *initializerPidPtr = osSharedGlobalVarsPtr->initializerPid;

        rc = GT_OK;
    }

    return rc;
}


GT_STATUS prvOsHelperGlobalDbAdressInfoGet
(
    GT_U32  *dmaVirtualAddrLowPtr,
    GT_U32  *dmaVirtualAddrHighPtr,
    GT_U32  *dmaPhysAddrLowPtr,
    GT_U32  *dmaPhysAddrHighPtr,
    GT_U32  *globalSharedDbAddrLowPtr,
    GT_U32  *globalSharedDbAddrHiPtr
)
{
    GT_STATUS rc = GT_BAD_PTR;

    if(dmaVirtualAddrLowPtr&&dmaVirtualAddrHighPtr&&dmaPhysAddrLowPtr&&dmaPhysAddrHighPtr
        &&globalSharedDbAddrLowPtr&&globalSharedDbAddrHiPtr)
    {
        *globalSharedDbAddrLowPtr = (((uintptr_t)osSharedGlobalVarsPtr)& 0x00000000ffffffffL);
#if __WORDSIZE == 64
        *globalSharedDbAddrHiPtr = ((((uintptr_t)osSharedGlobalVarsPtr)>>32)& 0x00000000ffffffffL);
#else
        *globalSharedDbAddrHiPtr = 0;
#endif

        *dmaVirtualAddrLowPtr = (((uintptr_t)PRV_NON_SHARED_DMA_GLOBAL_VAR_GET(prvExtDrvDmaPtr))& 0x00000000ffffffffL);

#if __WORDSIZE == 64
        *dmaVirtualAddrHighPtr = ((((uintptr_t)PRV_NON_SHARED_DMA_GLOBAL_VAR_GET(prvExtDrvDmaPtr))>>32)& 0x00000000ffffffffL);
#else
        *dmaVirtualAddrHighPtr = 0;
#endif
        *dmaPhysAddrLowPtr = ((uintptr_t)PRV_NON_SHARED_DMA_GLOBAL_VAR_GET(prvExtDrvDmaPhys64).l[0]);

#if __WORDSIZE == 64
       *dmaPhysAddrHighPtr = (((uintptr_t)PRV_NON_SHARED_DMA_GLOBAL_VAR_GET(prvExtDrvDmaPhys64).l[1]));
#else
       *dmaPhysAddrHighPtr = 0;
#endif

        rc = GT_OK;
    }

    return rc;
}

GT_STATUS prvOsHelperGlobalDbDumpAdress
(
    GT_VOID
)
{
    GT_STATUS rc;
    GT_U32  dmaVirtualAddrLow;
    GT_U32  dmaVirtualAddrHigh;
    GT_U32  dmaPhysAddrLow;
    GT_U32  dmaPhysAddrHigh;
    GT_U32  globalSharedDbAddrLow;
    GT_U32  globalSharedDbAddrHi;
    GT_U32  win , numWin = PRV_HELPER_NON_SHARED_GLOBAL_VAR_DIRECT_GET(osNonVolatileDb.dmaConfigNumOfActiveWindows);

    numWin = numWin == 0 ? 1 : numWin;

    osPrintf("numWin[%d] \n", numWin);

    for(win = 0 ; win < numWin && win < GT_MEMORY_DMA_CONFIG_WINDOWS_CNS; win ++)
    {
        PRV_OS_DMA_CURR_WINDOW_SET(win);

        rc = prvOsHelperGlobalDbAdressInfoGet(&dmaVirtualAddrLow,&dmaVirtualAddrHigh,
            &dmaPhysAddrLow,&dmaPhysAddrHigh,&globalSharedDbAddrLow,&globalSharedDbAddrHi);
        if(rc!=GT_OK)
        {
            return rc;
        }

        if(numWin > 1)
        {
            osPrintf("=== start info about window [%d] ===\n",win);
        }
        osPrintf("dmaVirtualAddrLow             :0x%x\n",dmaVirtualAddrLow);
        osPrintf("dmaVirtualAddrHigh            :0x%x\n",dmaVirtualAddrHigh);
        osPrintf("dmaPhysAddrLow                :0x%x\n",dmaPhysAddrLow);
        osPrintf("dmaPhysAddrHigh               :0x%x\n",dmaPhysAddrHigh);
        osPrintf("globalSharedDbAddrLow         :0x%x\n",globalSharedDbAddrLow);
        osPrintf("globalSharedDbAddrHi          :0x%x\n",globalSharedDbAddrHi);

        if(numWin > 1)
        {
            osPrintf("=== ended info about window [%d] ===\n",win);
        }
    }

    PRV_OS_DMA_CURR_WINDOW_SET(0);/*restore*/

    return GT_OK;

}

GT_STATUS prvOsHelperGlobalDbInitParamsGet
(
    GT_U32                  *clientNumPtr,
    GT_U32                  *initPidPtr
)
{
    GT_STATUS rc = GT_BAD_PTR;

    if(clientNumPtr)
    {
        if(osSharedGlobalVarsPtr)
        {
            (void)prvOsHelperGlobalDbProcListSync(GT_FALSE);
            *clientNumPtr = osSharedGlobalVarsPtr->clientNum;
            if(initPidPtr)
            {
                *initPidPtr = osSharedGlobalVarsPtr->initializerPid;
            }
            rc = GT_OK;
        }
        /*if we got here this mean we failed in initalization so act like there are still clients connected,
                so unlink will not happend*/
        else
        {
            *clientNumPtr = 2;
            rc = GT_OK;
        }
    }

    return rc;
}

/**
* @internal osGlobalDbDmaActiveWindowByPcieParamsSet function
* @endinternal
*
* @brief   function to set the current window Id for DMA chunk allocation to be
*          the one that should be associated with the PCIe device.
*          the function sets a new current window Id and returns this window to application.
*
* INPUTS:
*        pciDomain  - the PCI domain
*        pciBus     - the PCI bus
*        pciDev     - the PCI device
*        pciFunc    - the PCI function
*        windowPtr  - in CONFIG_V2_DMA compilation <*windowPtr> need to hold the 'devNum'
*                     in non CONFIG_V2_DMA compilation <*windowPtr> not uses as INPUT
* OUTPUTS:
*       windowPtr   - (pointer to) the window Id that associated with the PCIe device
*
*
* @retval  GT_OK                    - if the PCIe parameters found to be for device
*                                   that was seen during PCIe scan of the extDrv
*          GT_NOT_FOUND             - if not found
*/
GT_STATUS osGlobalDbDmaActiveWindowByPcieParamsSet(
    IN GT_U32  pciDomain,
    IN GT_U32  pciBus,
    IN GT_U32  pciDev,
    IN GT_U32  pciFunc,
    INOUT GT_U32  *windowPtr
)
{
    GT_U32  win;
    GT_MEMORY_DMA_CONFIG *currPtr;
    off_t       compare_mvDmaDrvOffset = /* same logic as in set_mvDmaDrvOffset and in sysfs_pci_configure_pex */
                     ((pciDomain & 0xffff) << 16) |
                     ((pciBus & 0xff) << 8) |
                     ((pciDev & 0x1f) << 3) |
                      (pciFunc & 0x7);

#if (defined CONFIG_V2_DMA)
    GT_U32  devNum = *windowPtr;/* used as IN parameter */
#endif

    for(win = 0,currPtr=&PRV_HELPER_NON_SHARED_GLOBAL_VAR_DIRECT_GET(osNonVolatileDb.dmaConfig[0]);
        win < PRV_HELPER_NON_SHARED_GLOBAL_VAR_DIRECT_GET(osNonVolatileDb.dmaConfigNumOfActiveWindows) &&
        win < GT_MEMORY_DMA_CONFIG_WINDOWS_CNS;
        win ++,currPtr++)
    {
        if(currPtr->mvDmaDrvOffset == compare_mvDmaDrvOffset)
        {
#if (defined SHARED_MEMORY) && (! defined CONFIG_V2_DMA)
            /* the mvDmaDrv.ko not support multi DMAs , keep it as singleton ,
             until supported */
            win = 0;
#endif /*SHARED_MEMORY && !CONFIG_V2_DMA*/
            if(windowPtr)
            {
                *windowPtr = win;
            }

#if (defined CONFIG_V2_DMA)
            if(win!=devNum)
            {
                /* move the win to the device index */
                PRV_HELPER_NON_SHARED_GLOBAL_VAR_DIRECT_SET(osNonVolatileDb.dmaConfig[devNum],
                    PRV_HELPER_NON_SHARED_GLOBAL_VAR_DIRECT_GET(osNonVolatileDb.dmaConfig[win]));

                /* invalidate the old window */
                PRV_HELPER_NON_SHARED_GLOBAL_VAR_DIRECT_GET(osNonVolatileDb.dmaConfig[win]).mvDmaDrvOffset = 0;
                PRV_HELPER_NON_SHARED_GLOBAL_VAR_DIRECT_GET(osNonVolatileDb.dmaConfig[win]).prvExtDrvDmaLen = 0;
                PRV_HELPER_NON_SHARED_GLOBAL_VAR_DIRECT_GET(osNonVolatileDb.dmaConfig[win]).prvExtDrvDmaPhys64.l[0] =0;
                PRV_HELPER_NON_SHARED_GLOBAL_VAR_DIRECT_GET(osNonVolatileDb.dmaConfig[win]).prvExtDrvDmaPhys64.l[1] =0;
                PRV_HELPER_NON_SHARED_GLOBAL_VAR_DIRECT_GET(osNonVolatileDb.dmaConfig[win]).prvExtDrvDmaPtr = NULL;
                win = devNum;
                if(win >= PRV_HELPER_NON_SHARED_GLOBAL_VAR_DIRECT_GET(osNonVolatileDb.dmaConfigNumOfActiveWindows))
                {
                    PRV_HELPER_NON_SHARED_GLOBAL_VAR_DIRECT_SET(osNonVolatileDb.dmaConfigNumOfActiveWindows , win+1);
                }
            }
#endif /*SHARED_MEMORY && CONFIG_V2_DMA*/

            PRV_OS_DMA_CURR_WINDOW_SET(win);
            return GT_OK;
        }
    }

    if(windowPtr)
    {
        *windowPtr = 0;
    }
#ifdef ASIC_SIMULATION
    /*the DB is not maintained for WM */
    return GT_OK;
#else
    return GT_NOT_FOUND;
#endif
}

/**
* @internal osGlobalDbDmaActiveWindowSet function
* @endinternal
*
* @brief   function to set the current window Id for DMA chunk allocation.
*
* INPUTS:
*       window      - the window Id to used as active.
*
* @retval  GT_OK                    - if window in valid range
*          GT_BAD_PARAM             - if window not in valid range
*/
GT_STATUS osGlobalDbDmaActiveWindowSet(
    IN GT_U32  window
)
{
    if(window < PRV_HELPER_NON_SHARED_GLOBAL_VAR_DIRECT_GET(osNonVolatileDb.dmaConfigNumOfActiveWindows))
    {
        PRV_OS_DMA_CURR_WINDOW_SET(window);
        return GT_OK;
    }

#ifdef ASIC_SIMULATION
    /*the DB is not maintained for WM */
    return GT_OK;
#else
    return GT_BAD_PARAM;
#endif
}



GT_STATUS osGlobalDbDmaBlocksHeadGet
(
    GT_U8   devNum,
    GT_U32  *dmaVirtualAddrLowPtr,
    GT_U32  *dmaVirtualAddrHighPtr,
    GT_U32  *dmaPhysAddrLowPtr,
    GT_U32  *dmaLengthPtr
)
{
    if(NULL == dmaVirtualAddrLowPtr ||NULL == dmaVirtualAddrHighPtr|| NULL == dmaLengthPtr||
        NULL == dmaPhysAddrLowPtr)
    {
        return GT_BAD_PTR;
    }


    *dmaLengthPtr = osNonSharedGlobalVarsPtr->osNonVolatileDb.dmaConfig[devNum].prvExtDrvDmaLen;
    *dmaVirtualAddrLowPtr = (uintptr_t)(((EXTDRV_DMA_HANDLE *)PRV_NON_SHARED_DMA_DEVICE_INFO_GLOBAL_VAR_GET(dmaDrvHandle[devNum]))->blocks_head->virt)&0x00000000ffffffffL;
#if __WORDSIZE == 64
      *dmaVirtualAddrHighPtr = ((uintptr_t)(((EXTDRV_DMA_HANDLE *)PRV_NON_SHARED_DMA_DEVICE_INFO_GLOBAL_VAR_GET(dmaDrvHandle[devNum]))->blocks_head->virt)>>32)& 0x00000000ffffffffL;
#else
      *dmaVirtualAddrHighPtr = 0;
#endif
    *dmaPhysAddrLowPtr = (uintptr_t)(((EXTDRV_DMA_HANDLE *)PRV_NON_SHARED_DMA_DEVICE_INFO_GLOBAL_VAR_GET(dmaDrvHandle[devNum]))->blocks_head->dma)&0x00000000ffffffffL;
    return GT_OK;
}


