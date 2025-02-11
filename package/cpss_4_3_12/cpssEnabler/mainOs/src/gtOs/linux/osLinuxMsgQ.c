/**
********************************************************************************
* @file osLinuxMsgQ.c
*
* @brief Linux User Mode Operating System wrapper. Message queues
*
* @version   7
********************************************************************************
*/
/*******************************************************************************
* osLinuxMsgQ.c
*
* DESCRIPTION:
*       Linux User Mode Operating System wrapper. Message queues
*
* DEPENDENCIES:
*       Linux, CPU independed , and posix threads implementation.
*
* FILE REVISION NUMBER:
*       $Revision: 7 $
*******************************************************************************/

/* look for pthreads implementation in pthreads/lmsgqLib.c */
#if !defined(V2L_PTHREADS) && !defined(V2L_KERNEL)

#include <gtOs/gtOsMsgQ.h>
#include <gtOs/gtOsMem.h>
#include <gtOs/gtOsSem.h>
#include <gtOs/gtOsIo.h>
#include <gtOs/gtOsTimer.h>
#include <gtOs/osObjIdLib.h>

#include <pthread.h>

/************* Defines ********************************************************/

/* enable statistic collection for userspace mutexes */
#undef OS_MESSAGE_QUEUES_STAT

#define PRV_SHARED_DB osSharedGlobalVarsPtr->cpssEnablerMod.mainOsDir.gtOsLinuxMsgQSrc

/************ Internal Typedefs ***********************************************/
typedef struct _osMsgQ
{
    OS_OBJECT_HEADER_STC    header;
    GT_MUTEX                mtx;
    GT_SEM                  rxSem;
    GT_SEM                  txSem;
    int                     maxMsgs;
    int                     maxMsgSize;
    int                     messages;
    char                    *buffer;
    int                     head;
    int                     tail;
    int                     waitRx;
    int                     waitTx;
} OS_MSGQ_STC;

#ifdef OS_MALLOC_MEMORY_LEAKAGE_DBG
static GT_VOID *myOsMalloc
(
    IN GT_U32 size
)
{
    return osMalloc_MemoryLeakageDbg(size,__FILE__,__LINE__);
}
static GT_VOID myOsFree
(
    IN GT_VOID* const memblock
)
{
    osFree_MemoryLeakageDbg(memblock,__FILE__,__LINE__);
}
#endif /*OS_MALLOC_MEMORY_LEAKAGE_DBG*/

/************ Implementation defines ******************************************/
#define MSGQ_CHECK() \
    do { \
        if ((int)msgqId < 1 || (int)msgqId >= PRV_SHARED_DB.mqList.list.allocated) \
        { \
            /* bad msgqId */ \
            return GT_FAIL; \
        } \
        q = (OS_MSGQ_STC*)(PRV_SHARED_DB.mqList.list.list[(int)msgqId]); \
        if (!q || q->header.type == 0) \
        { \
            /* uninitialized or deleted */ \
            return GT_FAIL; \
        } \
    } while (0)

/************ Public Functions ************************************************/

/**
* @internal osMsgQCreate function
* @endinternal
*
* @brief   Create and initialize a message queue.
*
* @param[in] name                     - message queue name
* @param[in] maxMsgs                  - max messages in queue
* @param[in] maxMsgSize               - max length of single message
*
* @param[out] msgqId                   - message queue id
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS osMsgQCreate
(
    IN  const char    *name,
    IN  GT_U32        maxMsgs,
    IN  GT_U32        maxMsgSize,
    OUT GT_MSGQ_ID    *msgqId
)
{
    int qnum;
    OS_OBJECT_HEADER_STC *h;
    OS_MSGQ_STC *q;
    char tmpName[100];

    DO_LOCK_MUTEX(&PRV_SHARED_DB.mqList.mutex);
#ifdef SHARED_MEMORY
    qnum = osObjLibGetNewObject(&PRV_SHARED_DB.mqList.list, 1, name, &h,(OS_OBJ_ALLOC_FUNC_TYPE)osMalloc,osFree);
#else
    qnum = osObjLibGetNewObject(&PRV_SHARED_DB.mqList.list, 1, name, &h);
#endif
    q = (OS_MSGQ_STC*)h;
    if (qnum <= 0)
    {
ret_fail:
        pthread_mutex_unlock(&PRV_SHARED_DB.mqList.mutex);
        return GT_FAIL;
    }


    /* align max message size by 4 bytes */
    maxMsgSize = (maxMsgSize+3) & ~3;
    q->buffer = (char*)osMalloc((maxMsgSize + sizeof(GT_U32))*maxMsgs);
    if (q->buffer == NULL)
    {
        goto ret_fail;
    }
    osSprintf(tmpName,"mQ-M-%s",name);
    if (osMutexCreate(tmpName, &(q->mtx)) != GT_OK)
    {
ret_fail_1:
        osFree(q->buffer);
        goto ret_fail;
    }
    osSprintf(tmpName,"mQ-R-%s",name);
    if (osSemBinCreate(tmpName, OS_SEMB_EMPTY, &(q->rxSem)) != GT_OK)
    {
ret_fail_2:
        osMutexDelete(q->mtx);
        goto ret_fail_1;
    }
    osSprintf(tmpName,"mQ-T-%s",name);
    if (osSemBinCreate(tmpName, OS_SEMB_EMPTY, &(q->txSem)) != GT_OK)
    {
        osMutexDelete(q->rxSem);
        goto ret_fail_2;
    }

    q->maxMsgs = maxMsgs;
    q->maxMsgSize = maxMsgSize;
    pthread_mutex_unlock(&PRV_SHARED_DB.mqList.mutex);

    *msgqId = (GT_MSGQ_ID)qnum;
    return GT_OK;
}

/**
* @internal osMsgQDelete function
* @endinternal
*
* @brief   Delete message queue
*
* @param[in] msgqId                   - message queue Id
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS osMsgQDelete
(
    IN GT_MSGQ_ID msgqId
)
{
    int timeOut;
    OS_MSGQ_STC *q;

    MSGQ_CHECK();

    osMutexLock(q->mtx);
    q->header.type = 2; /* deleting */
    osMutexUnlock(q->mtx);

    for (timeOut = 1000; q->waitRx && timeOut; timeOut--)
    {
        osSemSignal(q->rxSem);
        osTimerWkAfter(1);
    }
    for (timeOut = 1000; q->waitTx && timeOut; timeOut--)
    {
        osSemSignal(q->txSem);
        osTimerWkAfter(1);
    }
    osFree(q->buffer);
    osMutexDelete(q->mtx);
    osSemDelete(q->rxSem);
    osSemDelete(q->txSem);

    q->header.type = 0;

    return GT_OK;
}

/**
* @internal osMsgQSend function
* @endinternal
*
* @brief   Send message to queue
*
* @param[in] msgqId                   - Message queue Idsemaphore Id
* @param[in] message                  -  data pointer
* @param[in] messageSize              - message size
* @param[in] timeOut                  - time out in miliseconds or
*                                      OS_MSGQ_WAIT_FOREVER or OS_MSGQ_NO_WAIT
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_TIMEOUT               - on time out
*/
GT_STATUS osMsgQSend
(
    IN GT_MSGQ_ID   msgqId,
    IN GT_PTR       message,
    IN GT_U32       messageSize,
    IN GT_U32       timeOut
)
{
    GT_STATUS ret;
    char    *msg;
    OS_MSGQ_STC *q;

    MSGQ_CHECK();

    osMutexLock(q->mtx);
    while (q->messages == q->maxMsgs)
    {
        /* queue full */
        if (timeOut == OS_MSGQ_NO_WAIT)
        {
            osMutexUnlock(q->mtx);
            return GT_FAIL; /* ??? GT_TIMEOUT */
        }
        q->waitTx++;
        osMutexUnlock(q->mtx);

        ret = osSemWait(q->txSem, timeOut);
        if (ret != GT_OK)
            return ret;

        osMutexLock(q->mtx);
        q->waitTx--;
        if (q->header.type != 1)
        {
            /* deleting */
            osMutexUnlock(q->mtx);
            return GT_FAIL;
        }
    }

    /* put message */
    msg = q->buffer + q->head * (q->maxMsgSize + sizeof(GT_U32));
    if (messageSize > q->maxMsgSize)
        messageSize = q->maxMsgSize;

    *((GT_U32*)msg) = messageSize;
    osMemCpy(msg+sizeof(GT_U32), message, messageSize);
    q->head++;
    if (q->head >= q->maxMsgs) /* round up */
        q->head = 0;
    q->messages++;

    /* signal to Recv thread if any */
    if (q->waitRx)
    {
        osSemSignal(q->rxSem);
    }

    osMutexUnlock(q->mtx);
    return GT_OK;
}

/**
* @internal osMsgQRecv function
* @endinternal
*
* @brief   Receive message from queuee
*
* @param[in] msgqId                   - Message queue Idsemaphore Id
* @param[in,out] messageSize              - size of buffer pointed by message
* @param[in] timeOut                  - time out in miliseconds or
*                                      OS_MSGQ_WAIT_FOREVER or OS_MSGQ_NO_WAIT
*
* @param[out] message                  -  data pointer
* @param[in,out] messageSize              - actual message size
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
* @retval GT_TIMEOUT               - on time out
*/
GT_STATUS osMsgQRecv
(
    IN    GT_MSGQ_ID   msgqId,
    OUT   GT_PTR       message,
    INOUT GT_U32       *messageSize,
    IN    GT_U32       timeOut
)
{
    GT_STATUS ret;
    char    *msg;
    GT_U32  msgSize;
    OS_MSGQ_STC *q;

    MSGQ_CHECK();

    osMutexLock(q->mtx);
    while (q->messages == 0)
    {
        /* queue empty */
        if (timeOut == OS_MSGQ_NO_WAIT)
        {
            osMutexUnlock(q->mtx);
            return GT_FAIL; /* ??? GT_TIMEOUT */
        }
        q->waitRx++;
        osMutexUnlock(q->mtx);

        ret = osSemWait(q->rxSem, timeOut);
        if (ret != GT_OK)
            return ret;

        osMutexLock(q->mtx);
        q->waitRx--;
        if (q->header.type != 1)
        {
            /* deleting */
            osMutexUnlock(q->mtx);
            return GT_FAIL;
        }
    }
    /* get message */
    msg = q->buffer + q->tail * (q->maxMsgSize + sizeof(GT_U32));
    msgSize = *((GT_U32*)msg);
    if (msgSize > *messageSize)
        msgSize = *messageSize;

    osMemCpy(message, msg+sizeof(GT_U32), msgSize);
    *messageSize = msgSize;
    q->tail++;
    if (q->tail >= q->maxMsgs) /* round up */
        q->tail = 0;
    q->messages--;

    /* signal to Recv thread if any */
    if (q->waitTx)
    {
        osSemSignal(q->txSem);
    }

    osMutexUnlock(q->mtx);
    return GT_OK;
}

/**
* @internal osMsgQNumMsgs function
* @endinternal
*
* @brief   Return number of messages pending in queue
*
* @param[in] msgqId                   - Message queue Idsemaphore Id
*
* @param[out] numMessages              - number of messages pending in queue
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
GT_STATUS osMsgQNumMsgs
(
    IN    GT_MSGQ_ID   msgqId,
    OUT   GT_U32       *numMessages
)
{
    OS_MSGQ_STC *q;

    MSGQ_CHECK();

    osMutexLock(q->mtx);
    *numMessages = q->messages;
    osMutexUnlock(q->mtx);

    return GT_OK;
}

#endif /* !defined(V2L_PTHREADS) && !defined(V2L_KERNEL) */


