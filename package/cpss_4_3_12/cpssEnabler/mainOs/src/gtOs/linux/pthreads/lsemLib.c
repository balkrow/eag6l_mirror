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
* @file lsemLib.c
*
* @brief Pthread implementation of mainOs semaphores
*
* @version   9
********************************************************************************
*/

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <gtOs/gtOsSem.h>
#include <gtOs/gtOsSemDefs.h>

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>

#include <gtOs/gtOsSharedData.h>
#include <gtOs/globalDb/gtOsGlobalDb.h>


#include "mainOsPthread.h"




/************* Internal data **************************************************/

#define PRV_SHARED_DB osSharedGlobalVarsPtr->cpssEnablerMod.mainOsDir.gtOsLinuxPthrLsemSrc

#if defined  CPSS_USE_MUTEX_PROFILER
extern GT_STATUS osMutexLockGlValidation
(
    IN GT_MUTEX mtxid
);

#endif

/************ Forward declararions ********************************************/
static int V2L_semOpenExisting
(
    IN  const char  *name,
    IN  semType     type
);


/************ Public Functions ************************************************/

/**
* @internal V2L_lsemInit function
* @endinternal
*
* @brief   Initialize semaphores
*
* @retval OK                       - on success
* @retval ERROR                    - on error
*
* @note This function called from osStartEngine()
*
*/
int V2L_lsemInit(void)
{
#ifdef SHARED_MEMORY
    if (!PRV_SHARED_DB.initialized)
    {
        CREATE_MTX(&PRV_SHARED_DB.semaphores_mtx);
        PRV_SHARED_DB.initialized = 1;
    }
#endif
    return 0;
}


/*******************************************************************************
* CREATE_SEM
*
* DESCRIPTION:
*       creates a semaphore.
*       Open an existing one if openExisting != 0
*
* INPUTS:
*       type       - semaphore type
*       name       - semaphore name
*       openExisting - flag
*
* OUTPUTS:
*       semId      - semaphore Id
*       sem        - semaphore struct pointer
*
* RETURNS:
*       ERROR      - on error
*       semaphoreId if already exists
*
* COMMENTS:
*       None
*
*******************************************************************************/
#ifdef SHARED_MEMORY

#define CREATE_SEM(type, openExisting) \
    int __rc; \
    int semId; \
    OS_OBJECT_HEADER_STC *h; \
    _V2L_semSTC *sem; \
    if (NULL==osSharedGlobalVarsPtr) \
    { \
        return GT_FAIL; \
    } \
    __rc = DO_LOCK_MUTEX(&PRV_SHARED_DB.semaphores_mtx); \
    if ((0 != __rc) && \
        (EOWNERDEAD != __rc)) \
    { \
        return GT_FAIL; \
    } \
    if (openExisting) \
    { \
        semId = V2L_semOpenExisting(name, type); \
        pthread_mutex_unlock(&PRV_SHARED_DB.semaphores_mtx); \
        if (semId > 0) \
        { \
            *smid = (GT_SEM)semId; \
            return GT_OK; \
        } \
        return GT_FAIL; \
    } \
    semId = osObjLibGetNewObject(&PRV_SHARED_DB.semaphores, type, name, &h,V2L_ts_malloc,V2L_ts_free); \
    sem = (_V2L_semSTC*)h; \
    pthread_mutex_unlock(&PRV_SHARED_DB.semaphores_mtx); \
    if (semId <= 0) \
    { \
        return GT_FAIL; \
    }

#else

#define CREATE_SEM(type, openExisting) \
    int semId; \
    OS_OBJECT_HEADER_STC *h; \
    _V2L_semSTC *sem; \
    if (NULL==osSharedGlobalVarsPtr) \
    { \
        return GT_FAIL; \
    } \
    if (0 != pthread_mutex_lock(&PRV_SHARED_DB.semaphores_mtx)) \
    { \
        return GT_FAIL; \
    } \
    if (openExisting) \
    { \
        semId = V2L_semOpenExisting(name, type); \
        pthread_mutex_unlock(&PRV_SHARED_DB.semaphores_mtx); \
        if (semId > 0) \
        { \
            *smid = (GT_SEM)semId; \
            return GT_OK; \
        } \
        return GT_FAIL; \
    } \
    semId = osObjLibGetNewObject(&PRV_SHARED_DB.semaphores, type, name, &h); \
    sem = (_V2L_semSTC*)h; \
    pthread_mutex_unlock(&PRV_SHARED_DB.semaphores_mtx); \
    if (semId <= 0) \
    { \
        return GT_FAIL; \
    }

#endif



/**
* @internal osSemBinCreate function
* @endinternal
*
* @brief   Create and initialize a binary semaphore.
*
* @param[in] name                     - semaphore Name
* @param[in] init                     -  value of semaphore (full or empty)
*
* @param[out] smid                     - semaphore Id
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS osSemBinCreate
(
    IN  const char    *name,
    IN  GT_SEMB_STATE init,
    OUT GT_SEM        *smid
)
{
    CREATE_SEM(SEMTYPE_BINARY, 0);

    /* initialize */
    CREATE_MTX(&(sem->mtx));
    CREATE_COND(&(sem->cond));

    sem->count = (init == OS_SEMB_FULL) ? 1 : 0;
    *smid = (GT_SEM)semId;
    return GT_OK;
}

/**
* @internal osSemMCreate function
* @endinternal
*
* @brief   Create and initialize a mutext semaphore.
*
* @param[in] name                     - semaphore Name
*
* @param[out] smid                     - semaphore Id
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
* @note This is not API. Should be used as development tool only.
*       Will be removed.
*
*/
GT_STATUS osSemMCreate
(
    IN  const char    *name,
    OUT GT_SEM        *smid
)
{
    pthread_mutexattr_t mattr;
    CREATE_SEM(SEMTYPE_MUTEX, 0);

    /* initialize recursive mutex */
    pthread_mutexattr_init(&mattr);
#ifdef SHARED_MEMORY
    pthread_mutexattr_setpshared(&mattr, PTHREAD_PROCESS_SHARED);
    MUTEXATTR_SETROBUST(&mattr);
#endif
    pthread_mutexattr_settype(&mattr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&(sem->mtx), &mattr);
    pthread_mutexattr_destroy(&mattr);
    *smid = (GT_SEM)semId;
    return GT_OK;
}

/**
* @internal osSemCCreate function
* @endinternal
*
* @brief   Create counting semaphore.
*
* @param[in] name                     - semaphore Name
* @param[in] init                     -  value of semaphore
*
* @param[out] smid                     - semaphore Id
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS osSemCCreate
(
    IN  const char    *name,
    IN  GT_U32        init,
    OUT GT_SEM        *smid
)
{
    CREATE_SEM(SEMTYPE_COUNTING, 0);

    /* initialize */
    CREATE_MTX(&(sem->mtx));
    CREATE_COND(&(sem->cond));
    sem->count = init;
    *smid = (GT_SEM)semId;
    return GT_OK;
}

/**
* @internal osSemMOpenNamed function
* @endinternal
*
* @brief   Create or open an existing named mutex semaphore.
*
* @param[in] name                     - semaphore Name
*
* @param[out] smid                     - semaphore Id
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
* @note Works only with kernel module semaphores
*
*/
GT_STATUS osSemMOpenNamed
(
    IN  const char    *name,
    OUT GT_SEM        *smid
)
{
    CREATE_SEM(SEMTYPE_MUTEX, 1);

    /* initialize */
    CREATE_MTX(&(sem->mtx));

    *smid = (GT_SEM)semId;
    return GT_OK;
}

/**
* @internal osSemBinOpenNamed function
* @endinternal
*
* @brief   Create or open existing named binary semaphore.
*
* @param[in] name                     - semaphore Name
* @param[in] init                     -  value of semaphore (full or empty)
*
* @param[out] smid                     - semaphore Id
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
* @note Works only with kernel module semaphores
*
*/
GT_STATUS osSemBinOpenNamed
(
    IN  const char    *name,
    IN  GT_SEMB_STATE init,
    OUT GT_SEM        *smid
)
{
    CREATE_SEM(SEMTYPE_BINARY, 1);

    /* initialize */
    CREATE_MTX(&(sem->mtx));
    CREATE_COND(&(sem->cond));

    sem->count = (init == OS_SEMB_FULL) ? 1 : 0;
    *smid = (GT_SEM)semId;
    return GT_OK;
}

/**
* @internal osSemCOpenNamed function
* @endinternal
*
* @brief   Create or open existing named counting semaphore.
*
* @param[in] name                     - semaphore Name
* @param[in] init                     -  value of semaphore
*
* @param[out] smid                     - semaphore Id
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
* @note Works only with kernel module semaphores
*
*/
GT_STATUS osSemCOpenNamed
(
    IN  const char    *name,
    IN  GT_U32        init,
    OUT GT_SEM        *smid
)
{
    CREATE_SEM(SEMTYPE_COUNTING, 0);

    /* initialize */
    CREATE_MTX(&(sem->mtx));
    CREATE_COND(&(sem->cond));

    sem->count = init;
    *smid = (GT_SEM)semId;
    return GT_OK;
}

/*******************************************************************************
* CHECK_SEMID
*
* DESCRIPTION:
*       Check semaphore Id
*
* INPUTS:
*       semId      - semaphore Id
*
* OUTPUTS:
*       sem        - semaphore struct pointer
*
* RETURNS:
*       ERROR      - on error
*
* COMMENTS:
*       None
*
*******************************************************************************/
#define CHECK_SEMID(semId) \
    int __rc; \
    _V2L_semSTC *sem; \
    if ((int)semId < 1 ||(NULL==osSharedGlobalVarsPtr)) \
    { \
        return GT_FAIL; \
    } \
    __rc = DO_LOCK_MUTEX(&PRV_SHARED_DB.semaphores_mtx); \
    if ((0 != __rc) && \
        (EOWNERDEAD != __rc)) \
    { \
        return GT_FAIL; \
    } \
    if ((int)semId >= PRV_SHARED_DB.semaphores.allocated)\
    { \
        pthread_mutex_unlock(&PRV_SHARED_DB.semaphores_mtx); \
        return GT_FAIL; \
    } \
    sem = (_V2L_semSTC*)(PRV_SHARED_DB.semaphores.list[(int)semId]); \
    if (!sem || !sem->header.type) \
    { \
        pthread_mutex_unlock(&PRV_SHARED_DB.semaphores_mtx); \
        return GT_FAIL; \
    } \
    pthread_mutex_unlock(&PRV_SHARED_DB.semaphores_mtx);

/**
* @internal osSemDelete function
* @endinternal
*
* @brief   Delete semaphore.
*
* @param[in] smid                     - semaphore Id
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS osSemDelete
(
    IN GT_SEM smid
)
{
    CHECK_SEMID(smid);
    switch (sem->header.type)
    {
        case SEMTYPE_NONE:
            break;
        case SEMTYPE_MUTEX:
            sem->header.type = SEMTYPE_NONE;
#if defined  CPSS_USE_MUTEX_PROFILER
             sem->header.glType = GRANULAR_LOCKING_TYPE_NONE;
             sem->header.devNum= 0;
#endif
            pthread_mutex_unlock(&(sem->mtx));
            pthread_mutex_destroy(&(sem->mtx));
            break;
        case SEMTYPE_BINARY:
        case SEMTYPE_COUNTING:
            sem->header.type = SEMTYPE_NONE;
            pthread_cond_broadcast(&(sem->cond));
            pthread_mutex_destroy(&(sem->mtx));
            pthread_cond_destroy(&(sem->cond));
            break;
    }
    sem->header.type = SEMTYPE_NONE;
    return GT_OK;
}

/**
* @internal osSemWait function
* @endinternal
*
* @brief   Wait on semaphore.
*
* @param[in] smid                     - semaphore Id
* @param[in] timeOut                  - time out in miliseconds or OS_WAIT_FOREVER to wait forever
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval OS_TIMEOUT               - on time out
*/
GT_STATUS osSemWait
(
    IN GT_SEM smid,
    IN GT_U32 timeOut
)
{
    CHECK_SEMID(smid);


    if (sem->header.type == SEMTYPE_MUTEX)
    {
        DO_LOCK_MUTEX(&(sem->mtx));

#if defined  CPSS_USE_MUTEX_PROFILER
         /*Perform validation only on first lock*/
        if((sem->mtx.__data.__lock != 0)&&(sem->mtx.__data.__count == 1))
        {
            osMutexLockGlValidation((GT_MUTEX)smid);
            sem->mtxOwner = pthread_self();
        }
#endif

#ifdef SEM_STAT
        sem->wait_count++;
#endif
        return GT_OK;
    }


    DO_LOCK_MUTEX(&(sem->mtx));
    if (sem->count)
    {
        /* wait successful */
        sem->count--;
#ifdef SEM_STAT
        sem->wait_count++;
#endif
        pthread_mutex_unlock(&(sem->mtx));
        return GT_OK;
    }
    if (timeOut == OS_NO_WAIT)
    {
        /* no wait */
#ifdef SEM_STAT
        sem->wait_count++;
#endif
        pthread_mutex_unlock(&(sem->mtx));
        return GT_FAIL;
    }

#ifdef SEM_STAT
    sem->waiting++;
#endif
    if (timeOut == OS_WAIT_FOREVER)
    {
        /* wait forever */
        while (sem->count == 0)
        {
            DO_COND_WAIT(&(sem->cond), &(sem->mtx));

            if (!sem->header.type)
            {
                pthread_mutex_unlock(&(sem->mtx));
                return GT_FAIL;
            }
        }
    }
    else
    {
        /* wait no more than timeOut milliseconds */
        struct timespec timeout, now;

        milliseconds2timespec(timeOut, &timeout);
        clock_gettime(CPSS_CLOCK, &now);
        timespec_add(&timeout, &now);

        while (sem->count == 0 && timespec_gt(&timeout, &now))
        {
            DO_COND_TIMEDWAIT(&(sem->cond), &(sem->mtx), &timeout);
            if (!sem->header.type)
            {
                pthread_mutex_unlock(&(sem->mtx));
                return GT_FAIL;
            }
            if (sem->count == 0)
                clock_gettime(CPSS_CLOCK, &now);
        }
        if (sem->count == 0)
        {
            /* wait failed (timeout) */
#ifdef SEM_STAT
            sem->waiting--;
            sem->timeout_count++;
#endif
            pthread_mutex_unlock(&(sem->mtx));
            return GT_TIMEOUT;
        }
    }
    /* wait successful */
#ifdef SEM_STAT
    sem->waiting--;
#endif
    sem->count--;
    pthread_mutex_unlock(&(sem->mtx));
    return GT_OK;
}

/**
* @internal osSemSignal function
* @endinternal
*
* @brief   Signal a semaphore.
*
* @param[in] smid                     - semaphore Id
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS osSemSignal
(
    IN GT_SEM smid
)
{
    CHECK_SEMID(smid);

    if (sem->header.type == SEMTYPE_MUTEX)
    {
#ifdef SEM_STAT
        sem->sig_count++;
#endif

#if defined  CPSS_USE_MUTEX_PROFILER
        /*Clear owner only if the mutex is going to be really unlocked*/
        if((sem->mtx.__data.__lock != 0)&&(sem->mtx.__data.__count == 1))
        {
            sem->mtxOwner = 0;


        }
#endif

        pthread_mutex_unlock(&(sem->mtx));
        return GT_OK;
    }
    DO_LOCK_MUTEX(&(sem->mtx));
#ifdef SEM_STAT
    sem->sig_count++;
#endif
    sem->count++;
    if (sem->header.type == SEMTYPE_BINARY && sem->count > 1)
        sem->count = 1;
    pthread_cond_signal(&(sem->cond));
    pthread_mutex_unlock(&(sem->mtx));

    return GT_OK;
}


/**
* @internal CPSS_osMutexCreate function
* @endinternal
*
* @brief   Create and initialize a Mutex object.
*
* @param[in] name                     - mutex Name
*
* @param[out] mtxid                    - mutex Id
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS CPSS_osMutexCreate
(
    IN  const char      *name,
    OUT GT_MUTEX        *mtxid
)
{
    return osSemMCreate(name, (GT_SEM*)mtxid);
}

/**
* @internal CPSS_osMutexDelete function
* @endinternal
*
* @brief   Delete mutex.
*
* @param[in] mtxid                    - mutex Id
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS CPSS_osMutexDelete
(
    IN GT_MUTEX mtxid
)
{
    return osSemDelete((GT_SEM)mtxid);
}

/**
* @internal osMutexLock function
* @endinternal
*
* @brief   Lock a mutex.
*
* @param[in] mtxid                    - mutex Id
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS osMutexLock
(
    IN GT_MUTEX mtxid
)
{
    return osSemWait((GT_SEM)mtxid, OS_WAIT_FOREVER);
}

/**
* @internal osMutexUnlock function
* @endinternal
*
* @brief   Unlock a mutex.
*
* @param[in] mtxid                    - mutex Id
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS osMutexUnlock
(
    IN GT_MUTEX mtxid
)
{
    return osSemSignal((GT_SEM)mtxid);
}

#ifdef SEM_STAT
/**
* @internal V2L_semStat function
* @endinternal
*
* @brief   Prints semaphore statistics
*/
int V2L_semStat(void)
{
    int semId;
    for (semId = 1; semId < PRV_SHARED_DB.semaphores.allocated; semId++)
    {
        _V2L_semSTC *sem;
        sem = (_V2L_semSTC*)(PRV_SHARED_DB.semaphores.list[semId]);
        if (!sem)
            continue;
        if (!sem->header.type)
            continue;
        printf("id=%d name=\"%s\" type=%c sig=%d waits=%d timeouts=%d waiting=%d\n",
                semId, sem->header.name, "nmcb"[sem->header.type],
                sem->sig_count, sem->wait_count,
                sem->timeout_count, sem->waiting);
    }
    return 0;
}
#endif

/**
* @internal osMutexStat function
* @endinternal
*
* @brief   Prints userspace mutex statistics
*
* @retval GT_OK                    - on success
*/
GT_STATUS osMutexStat(void)
{
#ifdef SEM_STAT
    V2L_semStat();
#endif
    return GT_OK;
}


/************ Internal Functions **********************************************/

/**
* @internal V2L_semOpenExisting function
* @endinternal
*
* @brief   Try to open an existing semaphore of given type
*
* @param[in] name                     - semaphore name
* @param[in] type                     - semaphore type
*                                       semaphore Id
*                                       0 if semaphore doesn't exists
*                                       < 0 if semaphore with give name has different type
*/
static int V2L_semOpenExisting
(
    IN  const char  *name,
    IN  semType     type
)
{
    int semId;
    _V2L_semSTC *sem = NULL;

    /* try to find existing semaphore first */
    for (semId = 1; semId < PRV_SHARED_DB.semaphores.allocated; semId++)
    {
        sem = (_V2L_semSTC*)(PRV_SHARED_DB.semaphores.list[semId]);
        if (!sem || !sem->header.type)
            continue;

        if (!strncmp(sem->header.name, name, sizeof(sem->header.name)))
            break;
    }

    if (sem && (semId < PRV_SHARED_DB.semaphores.allocated))
    {
        /* found */
        return (sem->header.type != (int)type) ? -1 : semId;
    }
    return 0;
}



