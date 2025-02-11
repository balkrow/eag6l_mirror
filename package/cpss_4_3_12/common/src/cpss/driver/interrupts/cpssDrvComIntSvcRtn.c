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
* @file cpssDrvComIntSvcRtn.c
*
* @brief Includes generic interrupt service routine functions declarations.
*
* @version   15
********************************************************************************
*/

/* get the OS , extDrv functions*/
#include <cpss/extServices/private/prvCpssBindFunc.h>
/* get common defs */
#include <cpssCommon/cpssPresteraDefs.h>

/* get the common deriver info */
#include <cpss/driver/interrupts/cpssDrvComIntEvReqQueues.h>
#include <cpss/driver/interrupts/cpssDrvComIntSvcRtn.h>

/* get interrupts names */
#include <cpss/generic/events/private/prvCpssGenEvReq.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#if (defined ASIC_SIMULATION_ENV_FORBIDDEN && defined ASIC_SIMULATION)
    /* 'Forbid' the code to run as ASIC_SIMULATION ... we need 'like' HW compilation ! */
    #undef ASIC_SIMULATION
#endif

#ifdef ASIC_SIMULATION
    extern GT_U32 cpssSimLogIsOpen(void);
    #include <os/simTypesBind.h>
    #define EXPLICIT_INCLUDE_TO_SIM_OS_H_FILES
    #include <os/simOsBindOwn.h>
    #undef EXPLICIT_INCLUDE_TO_SIM_OS_H_FILES
#endif /*ASIC_SIMULATION*/

/*******************************************************************************
* internal definitions
*******************************************************************************/

#ifdef GEN_ISR_DEBUG
#define DBG_INFO(x)   cpssOsPrintf x
#else
#define DBG_INFO(x)
#endif

GT_VOID interruptMainSr
(
    IN GT_U8 intVecMapIdx
);

/*  about INT_SCAN_DEBUG :
    to open this flag use environment parameter:
    set INT_SCAN_DEBUG=DEBUG_ON

    use function intScanPrintControl(...) to allow/deny printings
*/

/* #define INT_SCAN_DEBUG */

/*
    about INT_SCAN_DEBUG_NOT_ENABLE_INTERRUPT_IN_INITIALIZATION :
    allow to skip the enabling of the interrupts : cpssExtDrvIntEnable(intMask);
    --> to allow initialization to go over with no 'Interrupt issues'
        --> to allow to debug interrupts after initialization.
            --> call extDrvIntEnable(intMask) <-- according to the printings of "intMask"
*/
/*#define INT_SCAN_DEBUG_NOT_ENABLE_INTERRUPT_IN_INITIALIZATION*/

#ifdef INT_SCAN_DEBUG
    #undef DBG_INFO

    #define DBG_INFO(x)     myPrint x

    static char * uniEvName[CPSS_UNI_EVENT_COUNT_E] = {UNI_EV_NAME};
    static GT_U32   allowPrint=1;/* option to disable the print in runtime*/
    static GT_U32   numPrints = 0;
    #define myPrint cpssOsPrintf

    #define DBG_LOG(x)                          \
            if(allowPrint)                      \
            {                                   \
                myPrint x   ;                   \
                if(((++numPrints)%20) == 0)     \
                {                               \
                    /* the sleep make sure uart is not stuck */ \
                    cpssOsTimerWkAfter(1) ;     \
                }                               \
            }

/*control the interrupt printings , set new value
 function return the old value

 this function need to be called from the terminal...
*/
extern GT_U32 intScanPrintControl(IN GT_U32  allowPrintNew)
{
    GT_U32  oldState = allowPrint;
    allowPrint = allowPrintNew;

    return oldState;
}

/* function need to be called before the initSystem ,
   the function will generate task that will be responsible to periodically
   (every 200ms) to cal the ISR
*/
typedef GT_U32  GT_TASK;
extern GT_STATUS osTaskCreate
(
    IN  char    *name,
    IN  GT_U32  prio,
    IN  GT_U32  stack,
    IN  unsigned (__TASKCONV *start_addr)(void*),
    IN  void    *arglist,
    OUT GT_TASK *tid
);
extern GT_STATUS osTimerWkAfter(    IN GT_U32 mils);

#define INT_SCAN_PRIO_CNS            0
#define INT_SCAN_STACK_SIZE_CNS      0x2000

static GT_U32   dbgIntScanUseTaskRunning =0;
static GT_U32   dbgInterruptIsEnabled=0;
static GT_U8    dbgDevNum=0;
static unsigned __TASKCONV intScanPullingTask
(
    IN GT_VOID * notUsed
)
{
    notUsed = notUsed;

    /* wait for the 'Interrupt enable' is called */
    while(1)
    {
        osTimerWkAfter(10);

        if(dbgInterruptIsEnabled)
        {
            break;
        }
    }
    /* pulling on the ISR to check if there are interrupts waiting */
    while(1)
    {
        osTimerWkAfter(200);

        /* invoke ISR until all interrupts have been treated */
        interruptMainSr(dbgDevNum);
    }

    return 0;
}


extern GT_STATUS intScanUseTask(void)
{
    GT_TASK tid;

    if(osTaskCreate("intLineEmul",
        INT_SCAN_PRIO_CNS , INT_SCAN_STACK_SIZE_CNS,
        /* create the ISR pulling task */
        intScanPullingTask,(GT_VOID*)NULL, &tid) != GT_OK)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
    }

    dbgIntScanUseTaskRunning = 1;

    /*we can't allow printings because the task doing 'pulling' every 200ms */
    intScanPrintControl(0);

    return GT_OK;
}


#else /*INT_SCAN_DEBUG*/
    #undef DBG_INFO
    #define DBG_INFO(x)
    #define DBG_LOG(x)

extern GT_STATUS intScanUseTask(void)
{
    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_NOT_IMPLEMENTED, LOG_ERROR_NO_MSG);
}
#endif /*INT_SCAN_DEBUG*/

/*******************************************************************************
* Internal usage variables
*******************************************************************************/

/* Macro definitions for interrupts scanning functions use.  */

/* Return an interrupt cause specific bit.          */
#define INT_CAUSE_BIT(causeReg,intIdx)  \
            (((causeReg) >> ((intIdx) & 0x1f)) & 0x1)


/* Updated the interrupt mask bits to be set into   */
/* the int_mask register.                           */
#define UPDATE_INT_MASK_BITS(intMaskBits,i) \
            ((intMaskBits) |= (1 << ((i) & 0x1f)))


#define NOT_USED_CNS    0xffffffff

/**
* @internal isIntLineAlreadyUsed function
* @endinternal
*
* @brief   check if ISR was already connected to the extDrv (external driver), for given interrupt line
*         this function needed when we are initialize after we had 'DB release'
* @param[in] intVecNum                - The interrupt vector number this device is connected to.
*
* @retval GT_FALSE                 - the interrupt line was not connected yet to extDrv (external driver)
* @retval GT_TRUE                  - the interrupt line was connected already to extDrv (external driver)
*/
static GT_BOOL isIntLineAlreadyUsed
(
    IN  GT_U32          intVecNum
)
{
    GT_U32  ii;

    if(PRV_SHARED_COMMON_INT_DIR_GENERIC_GLOBAL_VAR_GET(afterDbReleaseIntVecAttached_initialized) == GT_FALSE)
    {
        /* check in a 'static' DB for the interrupt lines that already bound */
        for(ii = 0 ;ii < PRV_CPSS_MAX_PP_DEVICES_CNS ; ii++)
        {
            PRV_SHARED_COMMON_INT_DIR_GENERIC_GLOBAL_VAR_SET(afterDbReleaseIntVecAttached[ii], NOT_USED_CNS);
        }

        PRV_SHARED_COMMON_INT_DIR_GENERIC_GLOBAL_VAR_SET(afterDbReleaseIntVecAttached_initialized, GT_TRUE);
    }

    /* check in a 'static' DB for the interrupt lines that already bound */
    for(ii = 0 ;ii < PRV_CPSS_MAX_PP_DEVICES_CNS ; ii++)
    {
        if(PRV_SHARED_COMMON_INT_DIR_GENERIC_GLOBAL_VAR_GET(afterDbReleaseIntVecAttached[ii]) == intVecNum)
        {
            /* existing interrupt line */
            return GT_TRUE;
        }
    }

    /* new interrupt line */
    return GT_FALSE;
}

/**
* @internal intLineIsUsed function
* @endinternal
*
* @brief   set the ISR was connected to the extDrv (external driver), for given interrupt line
*         this function needed when we are initialize after we had 'DB release'
* @param[in] intVecNum                - The interrupt vector number this device is connected to.
*                                       none
*/
static void intLineIsUsed
(
    IN  GT_U32          intVecNum
)
{
    GT_U32  ii;

    /* check in a 'static' DB for the interrupt lines that a */
    for(ii = 0 ;ii < PRV_CPSS_MAX_PP_DEVICES_CNS ; ii++)
    {
        /* find first free place */
        if(PRV_SHARED_COMMON_INT_DIR_GENERIC_GLOBAL_VAR_GET(afterDbReleaseIntVecAttached[ii]) == NOT_USED_CNS)
        {
            /* set the interrupt line as used */
            PRV_SHARED_COMMON_INT_DIR_GENERIC_GLOBAL_VAR_SET(afterDbReleaseIntVecAttached[ii], intVecNum);
            return;
        }
    }

    return;
}

/**
* @internal intLineUnusedSet function
* @endinternal
*
* @brief  Mark the interrupt line as unused after Disconnect ISR from the
* interrupt line.
* @param[in] intVecNum - The interrupt vector number this device is connected to.
*
*/
static void intLineUnusedSet
(
    IN  GT_U32          intVecNum
)
{
    GT_U32  ii;

    /* check in a 'static' DB for the interrupt lines that already bound */
    for(ii = 0 ;ii < PRV_CPSS_MAX_PP_DEVICES_CNS ; ii++)
    {
        if(PRV_SHARED_COMMON_INT_DIR_GENERIC_GLOBAL_VAR_GET(afterDbReleaseIntVecAttached[ii]) == intVecNum)
        {
            /* set the interrupt line as not used */
            PRV_SHARED_COMMON_INT_DIR_GENERIC_GLOBAL_VAR_SET(afterDbReleaseIntVecAttached[ii], NOT_USED_CNS);
            return;
        }
    }

    return;
}

#define FIRST_FREE_NOT_FOUND_CNS 0xFFFFFFFF

/**
* @internal prvCpssDrvInterruptEnable function
* @endinternal
*
* @brief   Enable interrupts in accordance with given interrupt mask.
*
* @param[in] intMask                  - The interrupt mask to enable/disable interrupts on
*                                      this device.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on failure
*/
GT_STATUS prvCpssDrvInterruptEnable
(
    IN  GT_U32          intMask
)
{
    GT_STATUS rc = GT_OK;
    /* enable interrupt.    */
#ifndef INT_SCAN_DEBUG_NOT_ENABLE_INTERRUPT_IN_INITIALIZATION
    /* enable interrupt.    */
    rc =  cpssExtDrvIntEnable(intMask);
#else /*INT_SCAN_DEBUG_NOT_ENABLE_INTERRUPT_IN_INITIALIZATION*/
    cpssOsPrintf("intMask [%d] \n",intMask);
#endif /*INT_SCAN_DEBUG_NOT_ENABLE_INTERRUPT_IN_INITIALIZATION*/
    return rc;
}


/**
* @internal prvCpssDrvInterruptDisable function
* @endinternal
*
* @brief   Disable interrupts in accordance with given interrupt mask.
*
* @param[in] intMask    - The interrupt mask to enable/disable interrupts on
*                         this device.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on failure
*/
GT_STATUS prvCpssDrvInterruptDisable
(
    IN  GT_U32          intMask
)
{
    GT_STATUS rc = GT_OK;

    /* disable interrupt */
    rc = cpssExtDrvIntDisable(intMask);

    return rc;
}


/**
* @internal prvCpssDrvInterruptConnect function
* @endinternal
*
* @brief   Connect a given function to the given interrupt vector (mask).
*
* @param[in] intVecNum                - The interrupt vector number this device is connected to.
* @param[in] intMask                  - The interrupt mask to enable/disable interrupts on
*                                      this device.
* @param[in] intRoutine               - A pointer to the interrupt routine to be connected to the
*                                      given interrupt line.
* @param[in] cookie                   - A user defined parameter to be passed to the isr on interrupt
*                                      reception.
* @param[in] connectionId             Id to be used for future access to the connected isr.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on failure
* @retval GT_OUT_OF_CPU_MEM        - on memory allocation failure
* @retval GT_FULL                  - the DB is full and can't connect the new interrupt vector num
*/
GT_STATUS prvCpssDrvInterruptConnect
(
    IN  GT_U32          intVecNum,
    IN  GT_U32          intMask,
    IN  CPSS_EVENT_ISR_FUNC intRoutine,
    IN  void            *cookie,
    OUT GT_UINTPTR      *connectionId
)
{
    GT_STATUS       rc;
    GT_U32          firstFree;  /* The index of the first free cell in      */
                                /* prvCpssDrvComIntSvcRtnDb.intVecMap to be */
                                /* used if we need to create                */
                                /* a new list for a new interrupt vector.   */
    GT_BOOL         newList;    /* Whether a new list of devices should be  */
                                /* created.                                 */
    PRV_CPSS_DRV_ISR_PARAMS_STC*  isrParams, *prev;
    GT_U32          i,j;
    PRV_CPSS_DRV_INT_VEC_MEM_LIST_STC   *vecMemListPtr;/* pointer to the current
                                interrupt vector */

    /* avoid warning */
    (GT_VOID)intMask;

    firstFree   = FIRST_FREE_NOT_FOUND_CNS;
    newList     = GT_TRUE;

    for(i = 0; i < PRV_CPSS_MAX_PP_DEVICES_CNS; i++)
    {
        if(PRV_SHARED_COMMON_INT_DIR_GENERIC_GLOBAL_VAR_GET(prvCpssDrvComIntSvcRtnDb).intVecMap[i] == NULL)
        {
            if(firstFree == FIRST_FREE_NOT_FOUND_CNS)
            {
                firstFree = i;
            }
        }
        else if(PRV_SHARED_COMMON_INT_DIR_GENERIC_GLOBAL_VAR_GET(prvCpssDrvComIntSvcRtnDb).intVecMap[i]->intVecNum == intVecNum)
        {
            newList  = GT_FALSE;
            break;
        }
    }

    if (newList == GT_TRUE)
    {
        if(firstFree == FIRST_FREE_NOT_FOUND_CNS)
        {
            /* all the array filled already with other vectors */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FULL, LOG_ERROR_NO_MSG);
        }

        i = firstFree;
        PRV_SHARED_COMMON_INT_DIR_GENERIC_GLOBAL_VAR_GET(prvCpssDrvComIntSvcRtnDb).intVecMap[i] =
            cpssOsMalloc(sizeof(PRV_CPSS_DRV_INT_VEC_MEM_LIST_STC));
        if(PRV_SHARED_COMMON_INT_DIR_GENERIC_GLOBAL_VAR_GET(prvCpssDrvComIntSvcRtnDb).intVecMap[i] == NULL)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
        }

        vecMemListPtr = PRV_SHARED_COMMON_INT_DIR_GENERIC_GLOBAL_VAR_GET(prvCpssDrvComIntSvcRtnDb).intVecMap[i];

        cpssOsMemSet(vecMemListPtr,0,sizeof(PRV_CPSS_DRV_INT_VEC_MEM_LIST_STC));

        vecMemListPtr->intVecNum     = intVecNum;

        j = 0;/* set index 0 as the first free index ... */
    }
    else
    {

        vecMemListPtr = PRV_SHARED_COMMON_INT_DIR_GENERIC_GLOBAL_VAR_GET(prvCpssDrvComIntSvcRtnDb).intVecMap[i];

        /* look for an empty space to add the device's isr params */
        for (j = 0; j < PRV_CPSS_MAX_PP_DEVICES_CNS; j++)
        {
            if (vecMemListPtr->isrParamsArray[j]== NULL ||
                vecMemListPtr->isrParamsArray[j]->isrFuncPtr == NULL)
                break;
        }
    }

    /* no room left */
    if (j == PRV_CPSS_MAX_PP_DEVICES_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FULL, LOG_ERROR_NO_MSG);
    }

    if (vecMemListPtr->isrParamsArray[j]== NULL)
    {
        /* Allocate a new isr params struct.    */
        isrParams = cpssOsMalloc(sizeof(PRV_CPSS_DRV_ISR_PARAMS_STC));
        if(isrParams == NULL)
        {
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
        }
        vecMemListPtr->isrParamsArray[j] = isrParams;
    }
    else
    {
        isrParams = vecMemListPtr->isrParamsArray[j];
    }
    isrParams->cookie       = cookie;
    isrParams->isrFuncPtr   = intRoutine;
    *connectionId           = (GT_UINTPTR)isrParams;


    /* arrange the list */
    prev = NULL;
    for (j = 0; j < PRV_CPSS_MAX_PP_DEVICES_CNS; j++)
    {
        if (vecMemListPtr->isrParamsArray[j] != NULL &&
            vecMemListPtr->isrParamsArray[j]->isrFuncPtr != NULL)
        {
            if (prev != NULL)
            {
                prev->next = vecMemListPtr->isrParamsArray[j];
            }
            prev = vecMemListPtr->isrParamsArray[j];
        }
    }

    if (prev != NULL)
    {
        prev->next = NULL;
    }

    /* If reached here, the interrupt vector represented in cell i  */
    /* of prvCpssDrvComIntSvcRtnDb.intVecMap contains the device number
       devNum in it's list. */

    if(newList == GT_TRUE)
    {
        vecMemListPtr->first = isrParams;

        if(isIntLineAlreadyUsed(intVecNum) == GT_FALSE)
        {
#ifdef INT_SCAN_DEBUG
            if(dbgIntScanUseTaskRunning)
            {
                dbgInterruptIsEnabled = 1;
                dbgDevNum = (GT_U8)i;
            }
            else
#endif /*INT_SCAN_DEBUG*/
            {
                /* Connect the ISR to the interrupt vector. */
                rc = cpssExtDrvIntConnect(intVecNum, (GT_VOIDFUNCPTR)interruptMainSr, i);
                if(rc != GT_OK)
                {
                    CPSS_LOG_ERROR_AND_RETURN_MAC(rc, "cpssExtDrvIntConnect : failed on intVecNum[0x%8.8x] , param [%d] \n",intVecNum,i);
                }

            }
            /* set the DB that the interrupt line connected to extDrv (external driver) */
            intLineIsUsed(intVecNum);
        }
    }

    return GT_OK;
}


/**
* @internal prvCpssDrvInterruptDisconnect function
* @endinternal
*
* @brief   Disconnect a device from a given interrupt vector.
*
* @param[in] intVecNum                - The interrupt vector number the device is connected to.
*                                      connectioId - The connection Id returned on the interrupt connect
*                                      operation.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
* @retval GT_NO_SUCH               - there is not such interrupt vector num
*/
GT_STATUS prvCpssDrvInterruptDisconnect
(
    IN  GT_U32      intVecNum,
    IN  GT_UINTPTR  connectionId
)
{
    PRV_CPSS_DRV_ISR_PARAMS_STC         *isrParams; /* pointer to ISR params */
    PRV_CPSS_DRV_ISR_PARAMS_STC         *prev;      /* previous pointer to ISR params */
    GT_U32           i,j,k;              /* Loops index.                     */
    PRV_CPSS_DRV_INT_VEC_MEM_LIST_STC   *vecMemListPtr = NULL;/* pointer to the current
                                interrupt vector */


    /* Search for the appropriate cell in prvCpssDrvComIntSvcRtnDb.intVecMap.*/
    for(i = 0; i < PRV_CPSS_MAX_PP_DEVICES_CNS; i++ )
    {
        vecMemListPtr = PRV_SHARED_COMMON_INT_DIR_GENERIC_GLOBAL_VAR_GET(prvCpssDrvComIntSvcRtnDb).intVecMap[i];
        if(vecMemListPtr == NULL)
        {
            continue;
        }

        if(vecMemListPtr->intVecNum == intVecNum)
        {
            break;
        }
    }

    if(i == PRV_CPSS_MAX_PP_DEVICES_CNS)
    {
        /* not found , return OK !! */
        /* nothing more to clean ! */
        return GT_OK;
    }

    isrParams = (PRV_CPSS_DRV_ISR_PARAMS_STC*)connectionId;
    prev = NULL;
    /* find appropriate device ISR parameters  */
    for (j = 0; j < PRV_CPSS_MAX_PP_DEVICES_CNS; j++)
    {
        if (vecMemListPtr->isrParamsArray[j] != NULL &&
            vecMemListPtr->isrParamsArray[j]->isrFuncPtr != NULL)
        {
            if (vecMemListPtr->isrParamsArray[j] == isrParams)
            {
                break;
            }
            else
            {
                prev = vecMemListPtr->isrParamsArray[j];
            }
        }
    }

    if(j == PRV_CPSS_MAX_PP_DEVICES_CNS)
    {
        /* not found , return OK !! */
        /* nothing more to clean ! */
        return GT_OK;
    }


    if(isrParams)
    {
        /* Delete the node from the linked-list.    */
        isrParams->cookie = NULL;
        isrParams->isrFuncPtr = NULL;
        isrParams->next = NULL;
    }

    /* look for next element in the linked list  */
    for (; j < PRV_CPSS_MAX_PP_DEVICES_CNS; j++)
    {
        if (vecMemListPtr->isrParamsArray[j] != NULL &&
            vecMemListPtr->isrParamsArray[j]->isrFuncPtr != NULL)
            break;
    }

    if (j == PRV_CPSS_MAX_PP_DEVICES_CNS)
    {
        /* deleted element is in the tail of the linked list */
        if (prev != NULL)
        {
            prev->next = NULL;
        }
        else
        {
            vecMemListPtr->first = NULL;
        }
    }
    else
    {
        if (prev != NULL)
        {
            prev->next = vecMemListPtr->isrParamsArray[j];
        }
        else
        {
            vecMemListPtr->first = vecMemListPtr->isrParamsArray[j];
        }
    }

    /* cpssDrv support for "shutdown" of devices */
    if(vecMemListPtr->first == NULL)
    {
        /* we need to free the memory , in order to support "shutdown" of
           devices -- see the use of "osMalloc" instead of "osStaticMalloc"
        */
        for (k = 0; k < PRV_CPSS_MAX_PP_DEVICES_CNS; k++)
        {
            if(vecMemListPtr->isrParamsArray[k])
                cpssOsFree(vecMemListPtr->isrParamsArray[k]);
        }

        cpssOsFree(vecMemListPtr);

        PRV_SHARED_COMMON_INT_DIR_GENERIC_GLOBAL_VAR_GET(prvCpssDrvComIntSvcRtnDb).intVecMap[i] = NULL;
    }

    if(isIntLineAlreadyUsed(intVecNum) == GT_TRUE)
    {
        /* DisConnect the ISR to the interrupt vector. */
        cpssExtDrvIntDisconnect(intVecNum);

        intLineUnusedSet(intVecNum);
    }

    return GT_OK;
}


/**
* @internal prvCpssDrvInterruptScanInit function
* @endinternal
*
* @brief   This function initializes the Devices interrupt data structures for
*         isr interrupt scan operations.
* @param[in] intrScanArray            intrScanArray Length.
* @param[in] intrScanArray            - Array of hierarchy tree of interrupt scan struct to
*                                      initialize.
* @param[in,out] scanArrayIdx             - start index in scan array to use.
* @param[in,out] subIntrScan              - Pointer to be assigned with scan tree.
* @param[in,out] scanArrayIdx             - New start index in scan array to use.
* @param[in,out] subIntrScan              - Pointer to scan tree created.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS prvCpssDrvInterruptScanInit
(
    IN      GT_U32          intrScanArrayLen,
    IN      const PRV_CPSS_DRV_INTERRUPT_SCAN_STC  *intrScanArray,
    INOUT   GT_U32          *scanArrayIdx,
    INOUT   PRV_CPSS_DRV_INTERRUPT_SCAN_STC  **subIntrScan
)
{
    PRV_CPSS_DRV_INTERRUPT_SCAN_STC *intrScan;
    GT_STATUS       status;
    GT_U8           i;

    if(*scanArrayIdx == intrScanArrayLen)
    {
        DBG_INFO(("Mismatch between array and hierarchy struct"));
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "Mismatch between array and hierarchy struct");
    }

    while(intrScanArray[(*scanArrayIdx)].maskRegAddr == CPSS_EVENT_SKIP_MASK_REG_ADDR_CNS)
    {
        /* skip the entries that need to be skipped , we not need them in the
           'Dynamic tree' */
        (*scanArrayIdx)++;

        if(*scanArrayIdx == intrScanArrayLen)
        {
            DBG_INFO(("CPSS_EVENT_SKIP_MASK_REG_ADDR_CNS: Mismatch between array and hierarchy struct"));

            /* NOTE: the table can not end with 'skip' since the 'subIntrListLen'
                should be aware that this sub tree not exists */
            CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, "CPSS_EVENT_SKIP_MASK_REG_ADDR_CNS: Mismatch between array and hierarchy struct");
        }
    }


    intrScan = cpssOsMalloc(sizeof(PRV_CPSS_DRV_INTERRUPT_SCAN_STC));
    if (intrScan == NULL)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_OUT_OF_CPU_MEM, LOG_ERROR_NO_MSG);
    }

    *subIntrScan = intrScan;

    cpssOsMemCpy(intrScan,
                 &(intrScanArray[(*scanArrayIdx)]),
                 sizeof(PRV_CPSS_DRV_INTERRUPT_SCAN_STC));

    if(intrScan->subIntrListLen > 0)
    {
        intrScan->subIntrScan =
            cpssOsMalloc(sizeof(PRV_CPSS_DRV_INTERRUPT_SCAN_STC*) *
                                         (intrScan->subIntrListLen));

        for(i = 0; i < intrScan->subIntrListLen; i++)
        {
            (*scanArrayIdx)++;
            status = prvCpssDrvInterruptScanInit(intrScanArrayLen,intrScanArray,
                                       scanArrayIdx,
                                       &(intrScan->subIntrScan[i]));
            if(status != GT_OK)
            {
                DBG_INFO(("Scan Init error.\n"));
                return status;
            }
        }
    }

    return GT_OK;
}

/**
* @internal prvCpssDrvInterruptScanFree function
* @endinternal
*
* @brief   This function Frees the Device's interrupt data structures
*
* @param[in] subIntrScan              - Pointer to be assigned with scan tree.
*/
void prvCpssDrvInterruptScanFree
(
    IN   PRV_CPSS_DRV_INTERRUPT_SCAN_STC  *subIntrScan
)
{
    GT_U8           i;
    PRV_CPSS_DRV_INTERRUPT_SCAN_STC  **iter;

    if(subIntrScan == NULL)
    {
        return;
    }

    if(subIntrScan->subIntrScan != NULL)
    {
        iter = subIntrScan->subIntrScan;
        for(i = 0; i < subIntrScan->subIntrListLen; i++)
        {
            if (*iter != NULL)
            {
                prvCpssDrvInterruptScanFree(*iter);
            }
            iter++;
        }
    }

    FREE_PTR_MAC(subIntrScan->subIntrScan);
    FREE_PTR_MAC(subIntrScan);

    return;
}


/**
* @internal prvCpssDrvInterruptCauseRegsClear function
* @endinternal
*
* @brief   This function clears the interrupt cause registers for the given device.
*
* @param[in] devNum                   - The device number to scan.
* @param[in] portGroupId              - The portGroupId. for multi-port-groups device.
* @param[in] pIntScanNode             - Struct for accessing interrupt register in hierarchy
*                                      tree.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS prvCpssDrvInterruptCauseRegsClear
(
    IN GT_U8               devNum,
    IN GT_U32              portGroupId,
    IN PRV_CPSS_DRV_INTERRUPT_SCAN_STC      *pIntScanNode
)
{
    GT_U32      intCause;       /* A temp var. to hold the interrupt    */
                                /* cause reg. value.                    */
    GT_STATUS   retVal;         /* Function's return value.             */
    GT_U32      i;

    if(pIntScanNode->isGpp == GT_TRUE)
    {
        return GT_OK;
    }

    /* Clear all sub registers before clearing this level.  */
    for(i = 0; i < pIntScanNode->subIntrListLen; i++)
    {
        retVal = prvCpssDrvInterruptCauseRegsClear(devNum,portGroupId,pIntScanNode->subIntrScan[i]);
        if(retVal != GT_OK)
        {
            return retVal;
        }
    }

    /* Clear the interrupt cause register.                  */
    pIntScanNode->pRegReadFunc(devNum,portGroupId,pIntScanNode->causeRegAddr,&intCause);

    /* Clear the interrupts (if needed).                    */
    if(pIntScanNode->rwBitMask != 0)
    {
        pIntScanNode->pRegWriteFunc(devNum,portGroupId,pIntScanNode->causeRegAddr,0);
    }

    return GT_OK;
}


/**
* @internal interruptMainSr function
* @endinternal
*
* @brief   This is the main interrupt service routine, from which the other
*         interrupt SRs are called according to the devices types.
* @param[in] intVecMapIdx             - An index to the prvCpssDrvComIntSvcRtnDb.intVecMap list,
*                                      identifying the list of potential devices that could have
*                                      caused the interrupt.
*/
GT_VOID interruptMainSr
(
    IN GT_U8 intVecMapIdx
)
{
    PRV_CPSS_DRV_ISR_PARAMS_STC   *pIsrParams;
    void            *cookie;

#ifdef ASIC_SIMULATION
    {
        if(cpssSimLogIsOpen())
        {
            /* state that this task is the 'ISR' for the simulation logger */
            simOsTaskOwnTaskPurposeSet(SIM_OS_TASK_PURPOSE_TYPE_CPU_ISR_E,NULL);
        }
    }
#endif /*ASIC_SIMULATION*/

    PRV_CPSS_INT_SCAN_LOCK();
    if(PRV_SHARED_COMMON_INT_DIR_GENERIC_GLOBAL_VAR_GET(prvCpssDrvComIntSvcRtnDb).intVecMap[intVecMapIdx] == NULL)
    {
        /* the vector was removed in the litle time between the logic of the caller
           and this point that we do PRV_CPSS_INT_SCAN_LOCK() */
        pIsrParams = NULL;
    }
    else
    {
        pIsrParams = PRV_SHARED_COMMON_INT_DIR_GENERIC_GLOBAL_VAR_GET(prvCpssDrvComIntSvcRtnDb).intVecMap[intVecMapIdx]->first;
    }

    while(pIsrParams != NULL)
    {
        cookie = pIsrParams->cookie;

        /* invoke ISR untill all interrupts have been treated */
        while((pIsrParams->isrFuncPtr)(cookie));

        /* proceed to the next device */
        pIsrParams = pIsrParams->next;
    }

    PRV_CPSS_INT_SCAN_UNLOCK();

    return;
}


/**
* @internal prvCpssDrvIntEvReqDrvnScan function
* @endinternal
*
* @brief   This function scans a given interrupt cause register, and performs all
*         needed interrupt nodes queuing.
* @param[in] devNum                   - The device number to scan.
* @param[in] portGroupId              - The portGroupId. for multi-port-groups device.
* @param[in] intNodesPool             - Pool of interrupt nodes belonging to the scanned
*                                      device.
* @param[in] intMaskShadow            - Interrupt mask registers shadow array.
* @param[in] pIntScanNode             - Structure for accessing interrupt register in hierarchy
*                                      tree.
* @param[in] hwAccessCheckCb          - pointer to callback function to check managment access to
*                                       device. May be NULL.
*
* @retval 1                        - if interrupts where received from the given device
* @retval 0                        - otherwise.
*
* @note This routine is used on systems configured to poll events.
*
*/
GT_U8 prvCpssDrvIntEvReqDrvnScan
(
    IN GT_U8               devNum,
    IN GT_U32              portGroupId,
    IN PRV_CPSS_DRV_EV_REQ_NODE_STC            *intNodesPool,
    IN GT_U32              *intMaskShadow,
    IN PRV_CPSS_DRV_INTERRUPT_SCAN_STC      *pIntScanNode,
    IN CPSS_INTERRUPTS_HW_ACCESS_CHECK_FUNC  hwAccessCheckCb
)
{
    PRV_CPSS_DRV_EV_REQ_NODE_STC    *evNodesPool;   /* Event node pool for the device     */
    PRV_CPSS_DRV_INTERRUPT_SCAN_STC *intScanStackPtr[PRV_CPSS_DRV_SCAN_STACK_SIZE_CNS]; /* scan stack      */
    PRV_CPSS_DRV_INTERRUPT_SCAN_STC *currIntScanPtr;/* The current pointer in scan stack  */
    PRV_CPSS_DRV_INTERRUPT_SCAN_STC **intStackTopPtr;/* The top of scan stack pointer     */
    GT_U32              i;             /* Iterator                            */
    GT_U32              tmpCauseBits;  /* Temp to hold the needed cause bits  */
    GT_U32              intMaskBits;   /* The int mask bits to be set to      */
                                       /* 1, at the end of scanning.          */
    GT_U32              intMaskBit;    /* Interrupt mask bit                  */
    GT_U32              *maskShdwValPtr; /* mask shadow value pointer         */
    GT_BOOL             intMasked;      /* Int received while masked          */
    GT_U32              intCause;       /* Interrupt cause register value     */
    GT_U32              newInt;         /* New interrupt was received flag    */

    evNodesPool = intNodesPool;
    newInt = 0;

    /* push */
    intScanStackPtr[0] = pIntScanNode;
    intStackTopPtr = &intScanStackPtr[0];

    DBG_LOG((">>> INTERRUPT dev %d, portGroup %d scan registers...\n",devNum,portGroupId,3,4,5,6));

    while (intStackTopPtr >= &intScanStackPtr[0])
    {
        /* pop the interrupt scan node */
        currIntScanPtr = *intStackTopPtr;
        intStackTopPtr--;

        if(currIntScanPtr->isGpp == GT_TRUE)
        {
            DBG_LOG(("GPP interrupt id %d causeReg 0x%8.8X bitNum %d\n",
                 currIntScanPtr->gppId, currIntScanPtr->causeRegAddr,
                 currIntScanPtr->bitNum, 4,5,6));

            /* This is a Gpp interrupt, call the attached function. */
            currIntScanPtr->gppFuncPtr(devNum,currIntScanPtr->gppId);
            newInt = 1;
            continue;
        }

        /* Read the interrupt cause register.           */
        currIntScanPtr->pRegReadFunc(devNum, portGroupId,currIntScanPtr->causeRegAddr,
                                     &intCause);

        /* ISR of CPSS may be called in case CPU detects PCI_e link down.
           OS calls CPSS ISR if CPSS shares same IRQ with PCI_e controller of CPU
           in such case.
           Register read returns 0xFFFFFFFF when PCI_e link down. Need to check
           managment interface status when get such value.
           Break scan if management failure condition detected to avoid wrong
           scan decisions.*/
        if((intCause == 0xFFFFFFFF) && (hwAccessCheckCb))
        {
            if(hwAccessCheckCb(devNum, portGroupId) != GT_OK)
            {
                /* management interface is dead */
                return (GT_U8)newInt;
            }
        }

        maskShdwValPtr = &intMaskShadow[currIntScanPtr->startIdx >> 5];

        /* Scan local interrupts (non-summary bits).    */
        tmpCauseBits = intCause & currIntScanPtr->nonSumBitMask;
        intMaskBits = 0;

        DBG_LOG(("intCause 0x%8.8X=0x%8.8X (non-masked 0x%8.8X  , nonSumBitMask=0x%8.8X , shadowMask=0x%8.8X)\n",
                 currIntScanPtr->causeRegAddr,
                 intCause, tmpCauseBits,
                 currIntScanPtr->nonSumBitMask,
                 *maskShdwValPtr,6));

        if (tmpCauseBits != 0)
        {
            tmpCauseBits >>= currIntScanPtr->startIdx & 0x1f;

            for (i = currIntScanPtr->startIdx;
                 (i <= currIntScanPtr->endIdx && tmpCauseBits);
                 i++, tmpCauseBits >>= 1)
            {
                if (tmpCauseBits & 1)
                {
                    DBG_LOG(("new event: dev %d uniEv %-50s indx %d\n",devNum,
                             (evNodesPool[i].uniEvCause >= CPSS_UNI_EVENT_COUNT_E) ?
                             "RESERVED" :
                             uniEvName[evNodesPool[i].uniEvCause], i, 4,5,6));

                    /* If a callBack routine is registered for the event    */
                    /* invoke it.                                           */
                    if (evNodesPool[i].intCbFuncPtr != NULL)
                    {
                        evNodesPool[i].intCbFuncPtr(devNum, i);
                    }

                    /* check with the "unified event causes" --
                       if it is Tx completed event */
                    if (CPSS_PP_TX_BUFFER_QUEUE_E == evNodesPool[i].uniEvCause)
                    {
                        /* invoke the Tx end call back from the array of
                           call backs */
                        if(PRV_SHARED_COMMON_INT_DIR_GENERIC_GLOBAL_VAR_GET(prvCpssDrvComIntSvcRtnDb).txEndCb[devNum])
                        {
                            (PRV_SHARED_COMMON_INT_DIR_GENERIC_GLOBAL_VAR_GET(prvCpssDrvComIntSvcRtnDb).txEndCb[devNum])(devNum,
                                                    (GT_U8)evNodesPool[i].uniEvExt);
                        }

                        newInt = 1;
                        continue;
                    }

                    /* This is a regular event */
                    if (NULL != prvCpssDrvEvReqQUserHndlGet(evNodesPool[i].uniEvCause))
                    {
                        /* insert the event into queue */
                        intMaskBit = 1 << (i & 0x1f);
                        intMaskBits |= intMaskBit;

                        if (*maskShdwValPtr & intMaskBit)
                        {
                            intMasked = GT_FALSE;
                            newInt = 1;
                        }
                        else
                        {
                            intMasked = GT_TRUE;
                        }

                        prvCpssDrvEvReqQInsert(evNodesPool, i, intMasked);
                    }
                    else
                    {
                        newInt = 1;
                        DBG_LOG(("...unbinded event, evNodesPool[%d].uniEvCause = 0x%4.4x, "
                                 "intCause 0x%8.8X=0x%8.8X, intMask 0x%8.8X=0x%8.8X !!\n",
                                 i, evNodesPool[i].uniEvCause,
                                 currIntScanPtr->causeRegAddr, intCause,
                                 currIntScanPtr->maskRegAddr, *maskShdwValPtr));
                    }
                }
            }

            /* mask the received interrupt bits if needed */
            *maskShdwValPtr &= ~(intMaskBits & currIntScanPtr->maskRcvIntrEn);

            DBG_LOG(("mask interrupts addr 0x%8.8X value 0x%8.8X\n",
                     currIntScanPtr->maskRegAddr, *maskShdwValPtr,3,4,5,6));

            currIntScanPtr->pRegWriteFunc(devNum,portGroupId,
                                          currIntScanPtr->maskRegAddr,
                                          *maskShdwValPtr);
        }

        /* Scan summary interrupt bits.                 */
        tmpCauseBits = (intCause & ~(currIntScanPtr->nonSumBitMask) &
                        *maskShdwValPtr);

        if(tmpCauseBits != 0)
        {
            for(i = 0; i < currIntScanPtr->subIntrListLen; i++)
            {
                if (((tmpCauseBits >> ((currIntScanPtr->subIntrScan[i]->bitNum)
                                       & 0x1f)) & 1) == 1)
                {
                    intStackTopPtr++;
                    if (intStackTopPtr > &intScanStackPtr[PRV_CPSS_DRV_SCAN_STACK_LAST_ENTRY_CNS])
                    {
                        DBG_LOG(("scan stack overflow !!\n",1,2,3,4,5,6));
                        return 0;
                    }
                    *intStackTopPtr = currIntScanPtr->subIntrScan[i];
                }
            }
        }

        /* Clear the interrupts (if needed).            */
        if((intCause & currIntScanPtr->rwBitMask) != 0)
        {
            currIntScanPtr->pRegWriteFunc(devNum,portGroupId,currIntScanPtr->causeRegAddr,
                                        intCause & (~(currIntScanPtr->rwBitMask)));
        }
    }

    return (GT_U8)newInt;
}


/**
* @internal prvCpssDrvInterruptPpTxEndedCbBind function
* @endinternal
*
* @brief   bind a callback function for the device(Pp) for the "tx ended" event
*
* @param[in] devNum                   - the device number of the device that received the "tx ended"
*                                      event
* @param[in] txEndedCb                - the call back function to be called when the "tx ended"
*                                      event occur on the device
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PARAM             - on bad device number.
*/
GT_STATUS prvCpssDrvInterruptPpTxEndedCbBind
(
    IN  GT_U8   devNum,
    IN  PRV_CPSS_DRV_COM_INT_TX_ENDED_CB txEndedCb
)
{
    if(devNum >= PRV_CPSS_MAX_PP_DEVICES_CNS)
    {
        CPSS_LOG_ERROR_AND_RETURN_MAC(GT_BAD_PARAM, LOG_ERROR_NO_MSG);
    }

    PRV_SHARED_COMMON_INT_DIR_GENERIC_GLOBAL_VAR_GET(prvCpssDrvComIntSvcRtnDb).txEndCb[devNum] = txEndedCb;

    return GT_OK;
}

/**
* @internal prvCpssDrvInterruptsMaskDefSummaryInit function
* @endinternal
*
* @brief   Initialize the interrupt mask default value for polling mode. Only the
*         summary bits need to be unmasked, cause bits are unmasked by user app
*         on event bind
* @param[in] devMaskRegSize           - The size of interrupt mask register array.
* @param[in] maskRegMapArr            - The interrupt mask register map.
* @param[in] intScanRootPtr           - The interrupt information hierarchy tree root.
*
* @param[out] maskRegDefArr            - The interrupt mask default value.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS prvCpssDrvInterruptsMaskDefSummaryInit
(
    IN GT_U32                   devMaskRegSize,
    IN GT_U32                   *maskRegMapArr,
    IN GT_U32                   *maskRegDefArr,
    OUT PRV_CPSS_DRV_INTERRUPT_SCAN_STC          *intScanRootPtr
)
{
    PRV_CPSS_DRV_INTERRUPT_SCAN_STC  *intScanStackPtr[PRV_CPSS_DRV_SCAN_STACK_SIZE_CNS]; /* scan stack         */
    PRV_CPSS_DRV_INTERRUPT_SCAN_STC  *currIntScanPtr;    /* The current pointer in scan stack */
    PRV_CPSS_DRV_INTERRUPT_SCAN_STC  **intStackTopPtr;   /* The top of scan stack pointer     */
    GT_U32          i;                  /* Iterator                          */
    GT_U32          maskRegIdx;         /* The mask register index           */
    GT_BOOL         regFound;           /* A flag for register addr search   */

    /* clear the default mask value */
    cpssOsBzero((char *)maskRegDefArr, devMaskRegSize * sizeof(maskRegDefArr[0]));

    /* push the first scan element into the stack */
    intScanStackPtr[0] = intScanRootPtr;
    intStackTopPtr     = &intScanStackPtr[0];

    while (intStackTopPtr >= &intScanStackPtr[0])
    {
        /* pop the interrupt scan node */
        currIntScanPtr = *intStackTopPtr;
        intStackTopPtr--;

        while(currIntScanPtr->maskRegAddr == CPSS_EVENT_SKIP_MASK_REG_ADDR_CNS)
        {
            /* the entry should be fully ignored !!! */
            currIntScanPtr++;
        }

        if (0 == currIntScanPtr->subIntrListLen)
        {
            /* scan element does not have sub interrupts */
            continue;
        }
        else
        {
            regFound = GT_FALSE;

            /* search for the register address */
            for (maskRegIdx = 0; maskRegIdx < devMaskRegSize; maskRegIdx++)
            {
                if (maskRegMapArr[maskRegIdx] == currIntScanPtr->maskRegAddr)
                {
                    regFound = GT_TRUE;
                    break;
                }
            }

            if (GT_FALSE == regFound)
            {
                /* register address was not found, this is an error */
                CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
            }

            /* for all sub interrupts, update the mask default value */
            for (i = 0; i < currIntScanPtr->subIntrListLen; i++)
            {
                maskRegDefArr[maskRegIdx] |=
                                    1 << currIntScanPtr->subIntrScan[i]->bitNum;

                intStackTopPtr++;

                if (intStackTopPtr > &intScanStackPtr[PRV_CPSS_DRV_SCAN_STACK_LAST_ENTRY_CNS])
                {
                    /* stack is too small */
                    CPSS_LOG_ERROR_AND_RETURN_MAC(GT_FAIL, LOG_ERROR_NO_MSG);
                }
                *intStackTopPtr = currIntScanPtr->subIntrScan[i];
            }
        }
    }
    return GT_OK;
}

/**
* @internal prvCpssDrvComIntSvcRtnDbRelease function
* @endinternal
*
* @brief   private (internal) function to release the DB of the interrupts.
*         NOTE: function 'free' the allocated memory.
*         and restore DB to 'pre-init' state
*
* @note   APPLICABLE DEVICES:      xCat3; AC5; Lion2; Bobcat2; Caelum; Aldrin; AC3X; Bobcat3; Aldrin2; Falcon; AC5P; AC5X; Harrier; Ironman; Pipe.
* @note   NOT APPLICABLE DEVICES:  None.
*
*
* @retval GT_OK                    - on success.
*/
GT_STATUS prvCpssDrvComIntSvcRtnDbRelease
(
    void
)
{
    GT_STATUS rc;

    cpssOsMemSet(&PRV_SHARED_COMMON_INT_DIR_GENERIC_GLOBAL_VAR_GET(prvCpssDrvComIntSvcRtnDb),0,sizeof(PRV_SHARED_COMMON_INT_DIR_GENERIC_GLOBAL_VAR_GET(prvCpssDrvComIntSvcRtnDb)));

    rc = prvCpssDrvComIntEvReqQueuesDbRelease();
    if (rc != GT_OK)
    {
        return rc;
    }

    return GT_OK;
}
