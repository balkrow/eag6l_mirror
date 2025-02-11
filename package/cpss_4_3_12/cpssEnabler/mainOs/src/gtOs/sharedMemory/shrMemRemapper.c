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
* @file shrMemRemapper.c
*
* @brief This file contains remapping routines which remaps regular libcpss.so
* shared object (TEXT section is common) to CPSS Shared Library (also
* BSS and DATA sections are common).
*
* @version   14
********************************************************************************
*/

/************* Includes *******************************************************/
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <sys/mman.h>
#include <sys/types.h>
#include <semaphore.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <gtOs/gtEnvDep.h>
#include <gtOs/gtOsIo.h>
#include <gtOs/gtOsMem.h>
#include <gtOs/gtOsGen.h>
#include <gtOs/gtOsSharedMemoryRemapper.h>
#include <gtExtDrv/drivers/prvExtDrvLinuxMapping.h>
#include <gtOs/gtOsSharedPp.h>
#include <gtOs/globalDb/gtOsGlobalDb.h>
#include <gtOs/globalDb/gtOsGlobalDbInterface.h>

GT_STATUS firstProcessDiscoveryUnLink
(
    const GT_CHAR_PTR name
);



#ifdef OS_NAMED_SEM
#   include <gtOs/gtOsSem.h>
#   define SEM_TYPE GT_SEM
    static GT_STATUS SEM_UNLINK(const char *name)
    {
        GT_SEM sem;
        if (osSemMOpenNamed(name, &sem) != GT_OK)
            return GT_FAIL;
        osSemDelete(sem);
        return GT_OK;
    }
    static GT_STATUS SEM_OPEN(const char *name, SEM_TYPE *semid)
    {
        if(osSemMOpenNamed(name, semid) != GT_OK)
        {
            fprintf(stderr, "osSemMOpenNamed(\"%s\") : can't open/create semaphore\n",
                name);
            return GT_FAIL;
        }
        return GT_OK;
    }
    static GT_STATUS SEM_CLOSE(SEM_TYPE semid)
    {
        return GT_OK;
    }
    static GT_STATUS SEM_WAIT(SEM_TYPE semid)
    {
        return osSemWait(semid, OS_WAIT_FOREVER);
    }
    static GT_STATUS SEM_POST(SEM_TYPE semid)
    {
        return osSemSignal(semid);
    }
    static GT_STATUS SEM_NOT_LOCKED(SEM_TYPE semid)
    {
        GT_STATUS status;
        status = osSemWait(semid, OS_NO_WAIT);
        if (status == GT_OK)
        {
            osSemSignal(semid);
        }
        return status;
    }
#endif
#ifdef POSIX_SEM
#   include <semaphore.h>
#   define  SEM_TYPE sem_t*
    static GT_STATUS SEM_UNLINK(const char *name)
    {
        if(sem_unlink(name) < 0)
        {
            if(errno != ENOENT && 0 != errno)
            {
                fprintf(stderr, "sem_unlink() : %s %s\n", name, strerror(errno));
                return GT_FAIL;
            }
        }
        return GT_OK;
    }
    static GT_STATUS SEM_OPEN(const char *name, SEM_TYPE *semid)
    {
        if((*semid = sem_open(name, O_CREAT, 0666, 1)) == SEM_FAILED)
        {
            fprintf(stderr, "sem_open() : %s\n", strerror(errno));
            return GT_FAIL;
        }
        return GT_OK;
    }
    static GT_STATUS SEM_CLOSE(SEM_TYPE semid)
    {
        if(sem_close(semid) == -1)
        {
            fprintf(stderr, "sem_close() : %s\n", strerror(errno));
            return GT_FAIL;
        }
        return GT_OK;
    }
    static GT_STATUS SEM_WAIT(SEM_TYPE semid)
    {
        if(sem_wait(semid) == -1)
        {
            fprintf(stderr, "sem_wait() : %s\n", strerror(errno));
            return GT_FAIL;
        }
        return GT_OK;
    }
    static GT_STATUS SEM_POST(SEM_TYPE semid)
    {
        if(sem_post(semid) == -1)
        {
            fprintf(stderr, "sem_post() : %s\n", strerror(errno));
            return GT_FAIL;
        }
        return GT_OK;
    }
    static GT_STATUS SEM_NOT_LOCKED(SEM_TYPE semid)
    {
        int sval;
        if (sem_getvalue(semid, &sval) < 0)
        {
            fprintf(stderr, "sem_getvalue() : %s\n", strerror(errno));
            return GT_FAIL;
        }
        return (sval > 0) ?  GT_OK : GT_FAIL;
    }
#endif
#if defined(NO_NAMED_SEM) && !defined(SEM_TYPE)
#   define SEM_TYPE int
    static GT_STATUS SEM_UNLINK(const char *name)
    {
        return GT_OK;
    }
    static GT_STATUS SEM_OPEN(const char *name, SEM_TYPE *semid)
    {
        return GT_OK;
    }
    static GT_STATUS SEM_CLOSE(SEM_TYPE semid)
    {
        return GT_OK;
    }
    static GT_STATUS SEM_WAIT(SEM_TYPE semid)
    {
        return GT_OK;
    }
    static GT_STATUS SEM_POST(SEM_TYPE semid)
    {
        return GT_OK;
    }
    static GT_STATUS SEM_NOT_LOCKED(SEM_TYPE semid)
    {
        return GT_OK;
    }
#endif


/************* Functions ******************************************************/
/************* Static Functions ***********************************************/

/**
* @internal findAdressesOfSections function
* @endinternal
*
* @brief   Find termination adress of the shared section into /proc/self/maps file.
*
* @param[in,out] cpss_sectPtr             - pointer to structure with leading adress of the section
* @param[in,out] mainos_sectPtr           - pointer to structure with leading adress of the section
* @param[in,out] cpss_sectPtr             -
* @param[in,out] mainos_sectPtr           - pointer to structure for leading and termination adress
*                                      of the section
*
* @retval GT_OK                    - Operation succeeded
* @retval GT_FAIL                  - Operaton failed
*
* @note Uses SHARED_DATA_ADDR_CNS SHARED_MAINOS_DATA_ADDR_CNS
*       (fixed well-known address) as a key for searching required
*       section by start address. Only end address is calculated here.
*
*/
 GT_STATUS findAdressesOfSections(
    INOUT SECTION_STC *cpss_sectPtr,
    INOUT SECTION_STC *mainos_sectPtr,
    GT_BOOL            byName
)
{
    FILE *map;
    char buf[4096];
    char attr1, attr2, attr3;
    SECTION_STC *sectPtr = cpss_sectPtr;
    char * cpssLibname = LIB_CPSS_NAME;
    char * helperLibname = LIB_OS_NAME;

#ifdef PIC_BUILD
    if(GT_FALSE==osNonSharedGlobalVars.osNonVolatileDb.aslrSupport)
    {
       cpssLibname = LIB_CPSS_NAME_NO_ASLR;
       helperLibname = LIB_OS_NAME_NO_ASLR;
    }
#endif


    /* Init structure -fill by zeroes */
    if(memset(cpss_sectPtr, 0, sizeof(SECTION_STC)) == NULL)
    {
        fprintf(stderr, "memset() : %s\n", strerror(errno));
        return GT_FAIL;
    }
    if(memset(mainos_sectPtr, 0, sizeof(SECTION_STC)) == NULL)
    {
        fprintf(stderr, "memset() : %s\n", strerror(errno));
        return GT_FAIL;
    }

    /* Open special file with section's info.
     * This file is handled by Linux kernel. */
    if( (map = fopen("/proc/self/maps", "r")) == NULL)
    {
        fprintf(stderr, "fopen() : %s\n", strerror(errno));
        return GT_FAIL;
    }

    /* Read it line-by-line */
    while(fgets(buf, sizeof(buf), map) != NULL)
    {
         unsigned long long start, done;/* long will be 64bit on 64bit OS */
        /* Parse varables from line */
        sscanf(buf, "%llx-%llx %c%c%c", &start, &done, &attr1, &attr2, &attr3);


        if(GT_FALSE == byName)
        {
            /* check_overlap(buf, start, done); */
            /* Start address matches the key? */
            if(start == SHARED_DATA_ADDR_CNS /* || start == SHARED_MAINOS_DATA_ADDR_CNS */)
            {
                sectPtr = cpss_sectPtr;
                sectPtr->startPtr = (GT_VOID *)((GT_UINTPTR)start);
                sectPtr->donePtr  = (GT_VOID *)((GT_UINTPTR)done);
            } else if(start == SHARED_MAINOS_DATA_ADDR_CNS)
            {
                sectPtr = mainos_sectPtr;
                sectPtr->startPtr = (GT_VOID *)((GT_UINTPTR)start);
                sectPtr->donePtr  = (GT_VOID *)((GT_UINTPTR)done);
            }
            /* !! NOTE - we merge consecutive sections 'cause compiler
                can split them into parts.
                Warning: unrelated sections shouldn't be consecutive!!! */
            else if((GT_VOID *)((GT_UINTPTR)start) == sectPtr->donePtr)
            {
                /* Don't merge with guider section (with no any rights) */
                if(attr1!='-' || attr2!='-' || attr3!='-')
                    sectPtr->donePtr  = (GT_VOID *)((GT_UINTPTR)done);
            }
        }
        else
        {

            /* check_overlap(buf, start, done); */
            /* Start address matches the key? */
            if(NULL!= strstr(buf,cpssLibname))
            {
                sectPtr = cpss_sectPtr;
                sectPtr->startPtr = (GT_VOID *)((GT_UINTPTR)start);
                sectPtr->donePtr  = (GT_VOID *)((GT_UINTPTR)done);
            } else if(NULL!= strstr(buf,helperLibname))
            {
                sectPtr = mainos_sectPtr;
                sectPtr->startPtr = (GT_VOID *)((GT_UINTPTR)start);
                sectPtr->donePtr  = (GT_VOID *)((GT_UINTPTR)done);
            }
            /* !! NOTE - we merge consecutive sections 'cause compiler
                can split them into parts.
                Warning: unrelated sections shouldn't be consecutive!!! */
            else if((GT_VOID *)((GT_UINTPTR)start) == sectPtr->donePtr)
            {
                /* Don't merge with guider section (with no any rights) */
                if(attr1!='-' || attr2!='-' || attr3!='-')
                    sectPtr->donePtr  = (GT_VOID *)((GT_UINTPTR)done);
            }
        }
    }

    /* Close /proc/self/maps */
    if (fclose(map) == EOF)
    {
        fprintf(stderr, "fclose() : %s\n", strerror(errno));
        return GT_FAIL;
    }

    return GT_OK;
}

/**
* @internal mapFile function
* @endinternal
*
* @brief   Maps shared buffer into processes address space instead of BSS/DATA
*         section. Also the first client have to fulfill initial content of
*         buffer by data from BSS/DATA.
* @param[in] namePtr                  - name of file
* @param[in] sectPtr                  - pointer to the BSS/DATA section for which
*                                      mapping is processed for
* @param[in] isFirstClient            - is client is the first CPSS client(Enabler) on not.
*
* @retval GT_OK                    - Operation succeeded
* @retval GT_FAIL                  - Operaton failed
*
* @note This is the core function of Shared Memory approach.
*
*/
static GT_STATUS mapFile
(
    IN char         *namePtr,
    IN SECTION_STC  *sectPtr,
    IN GT_BOOL      isFirstClient
)
{
    GT_VOID_PTR ptr;
    GT_32 fd;
    void* (*mmapPtr)(void *start, size_t length, int prot , int flags, int fd,
       off_t offset);

    if(sectPtr == NULL || sectPtr->startPtr == NULL)
        return GT_OK;

    if(isFirstClient)
    {
        /* Create shared buffer file */
        /* NOTE - here we use open('dev/shm/XXX') instaead of shm_open('XXX')
         * 'cause this function is likely not supported by ucLibC
        */
        fd = open(namePtr, O_RDWR | O_CREAT, 0666);
        if (fd < 0)
        {
            fprintf(stderr, "open() : %s\n", strerror(errno));
            return GT_FAIL;
        }

        /* Truncate shared buffer file to required length
         * (to the actual section's length) */
        if(ftruncate(fd, sectPtr->donePtr - sectPtr->startPtr) == -1)
        {
            fprintf(stderr, "ftruncate() : %s\n", strerror(errno));
            return GT_FAIL;
        }

        /* Write original content of BSS/DATA section into shared buffer */
        if(write(fd, sectPtr->startPtr, sectPtr->donePtr - sectPtr->startPtr) == -1)
        {
            fprintf(stderr, "write() : %s\n", strerror(errno));
            return GT_FAIL;
        }
    }
    else /* Non-first client */
    {
        /* Just open already created buffer */
        fd = open(namePtr, O_RDWR, 0666);
        if(fd < 0)
        {
            if(errno == ENOENT)
            {
                fprintf(stderr, "open() : file %s not exist, run Enabler first\n", namePtr);
                return GT_FAIL;
            }
            fprintf(stderr, "open() : %s\n", strerror(errno));
            return GT_FAIL;
        }
    }

    /* force dynamic linker to resolve mmap address */
    mmap(0,0,0,0,0,0);

    /* save the mmap address for later usage */
    mmapPtr = mmap;

    /* Shutdown old (private) BSS/DATA section */
    if(munmap(sectPtr->startPtr, sectPtr->donePtr - sectPtr->startPtr) == -1)
    {
        fprintf(stderr, "munmap() : %s\n", strerror(errno));
        return GT_FAIL;
    }

    /* !! NOTE - here is the popentially danger place -
        we havn't access to detached section !! */

    /* !! NOTE - address of 'mmap' function should be already resolved to avoid access
     * to untached section. */
    ptr = mmapPtr(sectPtr->startPtr, sectPtr->donePtr - sectPtr->startPtr,
               PROT_READ | PROT_WRITE /* | PROT_EXEC */,
               MAP_FIXED | MAP_SHARED, fd, 0);

    if(!ptr)
    {
        fprintf(stderr, "mmap() : %s\n", strerror(errno));
        return GT_FAIL;
    }

    /* Shared file descriptor is not required for now.
     * We can close it.
     * !!! Note - we still can use the mapped section into that file. */
    if(close(fd) == -1)
    {
        fprintf(stderr, "close() : %s\n", strerror(errno));
        return GT_FAIL;
    }

    return GT_OK;
}

/************* Public Functions ***********************************************/

/*******************************************************************************
* shrAddNameSuffix
*
* DESCRIPTION:
*       Add suffix to name (getenv("CPSS_SHR_INSTANCE_NUM"))
*
* INPUTS:
*       name
*
* OUTPUTS:
*       buffer
*
* RETURNS:
*   pointer to buffer
*
* COMMENTS:
*
*******************************************************************************/
char* shrAddNameSuffix(const char* name, char* buffer, int bufsize)
{
    const char *e;
    char  uid[64];

    if (name == NULL || buffer == NULL)
    {
        return NULL;
    }
    strncpy(buffer,name,bufsize-1);

    /* add UID as string to buffer*/
    sprintf(uid,"%d", getuid());
    strncat(buffer, uid, bufsize-1);

    /* add CPSS_SHR_INSTANCE_NUM string to buffer */
    e = getenv("CPSS_SHR_INSTANCE_NUM");
    if (e != NULL)
    {
        strncat(buffer, e, bufsize-1);
    }
    buffer[bufsize-1] = 0;
    return buffer;
}




/**
* @internal shrMemUnlinkShmObj function
* @endinternal
*
* @brief   Unlink shared files and semaphore.
*
* @retval GT_OK                    - Operation succeeded
* @retval GT_FAIL                  - Operaton failed
*
* @note This action is required for detect-first-client-by-file-existence
*       method.
*
*/
GT_STATUS shrMemUnlinkShmObj(GT_VOID)
{
    char buf[256];
#define ST_SIZE 9
    GT_STATUS st[ST_SIZE];
    GT_STATUS stOk[ST_SIZE];
    GT_U32  clientNum;
    GT_U32  size;

    PRV_OS_SHM_LOG_ENTER

    size = sizeof(st)/sizeof(st[0]);

    osMemSet(st,GT_OK,sizeof(GT_STATUS)*size);
    osMemSet(stOk,GT_OK,sizeof(GT_STATUS)*size);


    st[0] = prvOsHelperGlobalDbInitParamsGet(&clientNum,NULL);
    if(st[0]!=GT_OK)
    {
        return st[0];
    }

    if(clientNum>0)
    {
      SHM_PRINT("There are %d  clients that are using shared object. So no clean up started.\n",clientNum);
      PRV_OS_SHM_LOG_EXIT
    /*someone still use this shared object ,do not unlink*/
      return GT_OK;
    }


    SHM_PRINT("clientNum is %d .Clean up started.\n",clientNum);


    /* Remove named semaphore for clean-up */
    if (SEM_UNLINK(shrAddNameSuffix(SHARED_MEMORY_SEM_CNS,buf,sizeof(buf))) != GT_OK)
        st[0] = GT_FAIL;

    st[1] = firstProcessDiscoveryUnLink(FIRST_CLIENT_DISCOVERY_SEM_NAME);

    st[2] = gtOsHelperGlobalDbDestroy(GT_TRUE);

    /* Unlink shared buffer */
#define DO_RM(name, suff,ind) \
    if(suff == 1) \
        shrAddNameSuffix(name,buf,sizeof(buf)); \
    else \
        strcpy(buf,name); \
    if(unlink(buf) < 0) \
    { \
        if(errno != ENOENT && 0 != errno) \
        { \
            fprintf(stderr, "unlink() : %s %s\n", buf, strerror(errno)); \
            st[ind] = GT_FAIL; \
        } \
    }

    DO_RM(SHARED_MEMORY_DATA_CNS, 1,3);
    DO_RM(SHARED_MEMORY_MAINOS_DATA_CNS, 1,4);
    DO_RM("/dev/shm/CPSS_SHM_MALLOC", 1,5);
    DO_RM("/dev/shm/cpss_pp_mappings", 0,6);
    DO_RM("/dev/shm/tasks", 0,7);
#undef DO_RM

    PRV_OS_SHM_LOG_EXIT

    return (0==osMemCmp(st,stOk,sizeof(GT_STATUS)*size))?GT_OK:GT_FAIL;
}

/**
* @internal shrMemDoShmMap function
* @endinternal
*
* @brief   Do sharing data for all processes.
*
* @retval GT_OK                    - Operation succeeded
* @retval GT_FAIL                  - Operaton failed
*
* @note Performs major shared memory initializations.
*       It searches BSS/DATA section and remaps it to shared space.
*
*/
GT_STATUS shrMemDoShmMap(GT_BOOL isFirstClient)
{
    SEM_TYPE    semid;
    SECTION_STC cpss_section;
    SECTION_STC mainos_section;
    char buf[256];

#ifndef PIC_BUILD
#if defined(ARMARCH7)
#error "Non ASLR mode compilation is not supported for ARMv7 architecture"
#endif
#endif

    /* Open named semaphore to guard remapping code from other clients. */
    if (SEM_OPEN(shrAddNameSuffix(SHARED_MEMORY_SEM_CNS, buf, sizeof(buf)), &semid) != GT_OK)
        return GT_FAIL;

    if (isFirstClient)
    {
        if (SEM_NOT_LOCKED(semid) != GT_OK)
        {
            /* it seems there is a deadlock */
            fprintf(stderr,
                    "Warning: The shared mem semaphore seems to be locked\n"
                    "Only one appDemo*_shared process will run in the same time\n"
                    "For cleanup please remove the following files:\n"
                    "    " SHARED_MEMORY_DATA_CNS "$CPSS_SHR_INSTANCE_NUM\n"
                    "    /dev/shm/CPSS_SHM_MALLOC$CPSS_SHR_INSTANCE_NUM\n"
                    "    " SHARED_MEMORY_MAINOS_DATA_CNS "$CPSS_SHR_INSTANCE_NUM\n"
                    "    /dev/shm/sem.CPSS_SHM_SEM$CPSS_SHR_INSTANCE_NUM\n");
        }
    }
    /* Now searching for section we should remap.
     * Only one section with known begin address should be searched.
     * We have to know end address of it to map whole section. */
    if(findAdressesOfSections(&cpss_section,&mainos_section,GT_FALSE) == GT_FAIL)
    {
        fprintf(stderr, "findAdressesOfSections() : error\n");
        goto closeSemAndFail;
    }
    shmPrintf("CPSS_SECTION  : Start=%X, End=%X\n", (GT_INTPTR) cpss_section.startPtr, (GT_INTPTR) cpss_section.donePtr);
    shmPrintf("MAINOS_SECTION: Start=%X, End=%X\n", (GT_INTPTR) mainos_section.startPtr, (GT_INTPTR) mainos_section.donePtr);

    /* handle error if sections are not found */
    if ((cpss_section.startPtr == NULL) || (mainos_section.startPtr == NULL))
    {
        FILE *map;
        if( (map = fopen("/proc/self/maps", "r")) != NULL)
        {
            while (!feof(map))
            {
                int r;
                r = fread(buf, 1, sizeof(buf), map);
                if (r <= 0)
                    break;
                fwrite(buf, 1, r, stderr);
            }
            fclose(map);
            fflush(stderr);
        }
    }
    if (cpss_section.startPtr == NULL)
    {
        fprintf(stderr, "error: CPSS_SHM_DATA not found, must start at address %p\n",
                (void*)((GT_UINTPTR)SHARED_DATA_ADDR_CNS));
    }
    if (mainos_section.startPtr == NULL)
    {
        fprintf(stderr, "error: MAINOS_SHM_DATA not found, must start at address 0x%lx\n",
                (void*)((GT_UINTPTR)SHARED_MAINOS_DATA_ADDR_CNS));
    }
    if ((cpss_section.startPtr == NULL) || (mainos_section.startPtr == NULL))
    {
        goto closeSemAndFail;
    }

    /* Lock semaphore */
    if(SEM_WAIT(semid) != GT_OK)
        goto closeSemAndFail;

#ifdef MM_DEBUG
    /* Output sections BEFORE remapping. */
    shmPrintf("Initial mapping: dump /proc/self/maps\n");
    shrMemPrintMapsDebugInfo();
#endif

    /* Map shared buffer into processes address space.
     * The first client also have to create it before mapping. */
    if(mapFile(shrAddNameSuffix(SHARED_MEMORY_DATA_CNS,buf,sizeof(buf)), &cpss_section, isFirstClient) == GT_FAIL)
    {
        SEM_POST(semid);
        fprintf(stderr, "mapFile(CPSS_SHM_DATA) : error\n");
        goto closeSemAndFail;
    }

    if(mapFile(shrAddNameSuffix(SHARED_MEMORY_MAINOS_DATA_CNS,buf,sizeof(buf)), &mainos_section, isFirstClient) == GT_FAIL)
    {
        SEM_POST(semid);
        fprintf(stderr, "mapFile(MAINOS_SHM_DATA) : error\n");
        goto closeSemAndFail;
    }

    if(SEM_POST(semid) != GT_OK)
        goto closeSemAndFail;

    if(SEM_CLOSE(semid) != GT_OK)
        return GT_FAIL;


#ifdef MM_DEBUG
    shmPrintf("Post remapping: dump /proc/self/maps\n");
    shrMemPrintMapsDebugInfo();
#endif

    return GT_OK;

closeSemAndFail:
    SEM_CLOSE(semid);
    return GT_FAIL;

}


/**
* @internal shrMemPrintMapsDebugInfo function
* @endinternal
*
* @brief   Ouputs /proc/self/maps to stderr for debug purpose
*
* @note Should be used only for debugging on SHARED_MEMORY=1
*
*/
GT_VOID shrMemPrintMapsDebugInfo(GT_VOID)
{
    FILE* fp;
    int num;
    char buffer[4*1024];

    fp = fopen("/proc/self/maps", "r");
    num = fread(buffer, 1, sizeof(buffer), fp);
    fclose(fp);
    fwrite(buffer, 1, num, stderr);
}



