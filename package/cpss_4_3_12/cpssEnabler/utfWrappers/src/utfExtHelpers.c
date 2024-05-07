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
* @file utfExtHelpers.c
*
* @brief Contains definition of wrappers for calling all cpssEnabler functions
*        - eg. appDemo/appRef used in mainUT
*
* @version
********************************************************************************
*/

#include <utfExtHelpers.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>

#include <appDemo/utils/mainUT/appDemoUtils.h>

#include <cpssAppUtilsEvents.h>

extern GT_BOOL cpssAppPlatformInitialized;

/**
* @internal prvWrAppAllowProcessingOfAuqMessages function
* @endinternal
*
* @brief   function to allow set the flag of : allowProcessingOfAuqMessages
*
* @param[in] enable                   - enable/disable the processing of the AUQ messages
*
* @retval GT_OK                    - on success
*
* @note flag that state the tasks may process events that relate to AUQ messages.
*       this flag allow us to stop processing those messages , by that the AUQ may
*       be full, or check AU storm prevention, and other.
*
*/
GT_STATUS   prvWrAppAllowProcessingOfAuqMessages
(
    GT_BOOL     enable
)
{
    GT_STATUS rc = GT_OK;
#ifdef CPSS_APP_PLATFORM_REFERENCE
    if (cpssAppPlatformInitialized == GT_TRUE) {
        rc = appRefAllowProcessingOfAuqMessages(enable);
    }
#endif /*CPSS_APP_PLATFORM_REFERENCE*/

#if !defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)
    if (cpssAppPlatformInitialized == GT_FALSE) {
        rc = appDemoAllowProcessingOfAuqMessages(enable);
    }
#endif /*!defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)*/
    return rc;
}

/*
* @internal prvWrAppDbEntryAdd function
* @endinternal
*
* @param[in] namePtr                  - points to parameter name
* @param[in] value                    - parameter value
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - if name is too long
* @retval GT_BAD_PTR               - if NULL pointer
* @retval GT_NO_RESOURCE           - if Database is full
*/
GT_STATUS prvWrAppDbEntryAdd
(
    IN  GT_CHAR *namePtr,
    IN  GT_U32  value
)
{
    GT_STATUS rc = GT_OK;
#ifdef CPSS_APP_PLATFORM_REFERENCE
    if (cpssAppPlatformInitialized == GT_TRUE) {
        rc = appPlatformDbEntryAdd(namePtr,value);/*naming convention for app platform functions - appRef* */
    }
#endif /*CPSS_APP_PLATFORM_REFERENCE*/

#if !defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)
    if (cpssAppPlatformInitialized == GT_FALSE) {
        rc = appDemoDbEntryAdd(namePtr,value);
    }
#endif /*!defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)*/
    return rc;
}

/**
* @internal prvWrAppDbEntryGet function
* @endinternal
*
* @brief   Get parameter value from AppDemo DataBase.
*
* @param[in] namePtr                  - points to parameter name
*
* @param[out] valuePtr                 - points to parameter value
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - if name is too long
* @retval GT_BAD_PTR               - if NULL pointer
* @retval GT_NO_SUCH               - there is no such parameter in Database
*/
GT_STATUS prvWrAppDbEntryGet
(
    IN  GT_CHAR *namePtr,
    OUT GT_U32  *valuePtr
)
{
    GT_STATUS rc = GT_OK;
#ifdef CPSS_APP_PLATFORM_REFERENCE
    if (cpssAppPlatformInitialized == GT_TRUE) {
        rc = appPlatformDbEntryGet(namePtr,valuePtr);
    }
#endif /*CPSS_APP_PLATFORM_REFERENCE*/

#if !defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)
    if (cpssAppPlatformInitialized == GT_FALSE) {
        rc = appDemoDbEntryGet(namePtr,valuePtr);
    }
#endif /*!defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)*/
    return rc;
}


/**
* @internal prvWrAppDebugDeviceIdReset function
* @endinternal
*
*
* @retval GT_OK                    - on success
*/
GT_STATUS prvWrAppDebugDeviceIdReset
(
    IN GT_VOID
)
{
    GT_STATUS rc = GT_OK;
#ifdef CPSS_APP_PLATFORM_REFERENCE
    if (cpssAppPlatformInitialized == GT_TRUE) {
        rc = appRefDebugDeviceIdReset();
    }
#endif /*CPSS_APP_PLATFORM_REFERENCE*/

#if !defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)
    if (cpssAppPlatformInitialized == GT_FALSE) {
        rc = appDemoDebugDeviceIdReset();
    }
#endif /*!defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)*/
    return rc;
}


/**
* @internal prvWrAppDebugDeviceIdSet function
* @endinternal
*
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               - on NULL pointer
* @retval GT_OUT_OF_RANGE          - on devIndex > 127
*/
GT_STATUS prvWrAppDebugDeviceIdSet
(
    IN GT_U8    devNum,
    IN GT_U32   deviceId
)
{
    GT_STATUS rc = GT_OK;
#ifdef CPSS_APP_PLATFORM_REFERENCE
    if (cpssAppPlatformInitialized == GT_TRUE) {
        rc = prvCpssDrvDebugDeviceIdSet(devNum,deviceId);
        if(rc == GT_OK)
        {
          useDebugDeviceId = GT_TRUE;
        }
    }
#endif /*CPSS_APP_PLATFORM_REFERENCE*/

#if !defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)
    if (cpssAppPlatformInitialized == GT_FALSE) {
        rc = appDemoDebugDeviceIdSet(devNum,deviceId);
    }
#endif /*!defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)*/
    return rc;
}


/**
* @internal prvWrAppDevIdxGet function
* @endinternal
*
*
* @param[in] devNum                   - SW device number
*
* @param[out] devIdxPtr                - (pointer to) device index in array "appDemoPpConfigList".
*
* @retval GT_OK                    - on success, match found for devNum.
* @retval GT_FAIL                  - no match found for devNum.
*/
GT_STATUS prvWrAppDevIdxGet
(
    IN  GT_U8   devNum,
    OUT GT_U32  *devIdxPtr
)
{
    GT_STATUS rc = GT_OK;
#ifdef CPSS_APP_PLATFORM_REFERENCE
    if (cpssAppPlatformInitialized == GT_TRUE) {
        *devIdxPtr = devNum;/*filling the value for devIdx as devNum itself - no concept of devIdx in appRef*/
    }
#endif /*CPSS_APP_PLATFORM_REFERENCE*/

#if !defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)
    if (cpssAppPlatformInitialized == GT_FALSE) {
        rc = appDemoDevIdxGet(devNum,devIdxPtr);
    }
#endif /*!defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)*/
    return rc;
}

/**
* @internal prvWrAppEventsToTestsHandlerBind function
* @endinternal
*
* @brief   This routine creates new event handler for the unified event list.
*
* @param[in] cpssUniEventArr[]        - The CPSS unified event list.
* @param[in] arrLength                - The unified event list length.
* @param[in] operation                - type of  mask/unmask to do on the events
*                                       GT_OK on success, or
*                                       GT_FAIL if failed.
*/
GT_STATUS prvWrAppEventsToTestsHandlerBind
(
    IN  CPSS_UNI_EV_CAUSE_ENT       cpssUniEventArr[],
    IN  GT_U32                      arrLength,
    IN  CPSS_EVENT_MASK_SET_ENT     operation
)
{
    GT_STATUS rc = GT_OK;
#ifdef CPSS_APP_PLATFORM_REFERENCE
    if (cpssAppPlatformInitialized == GT_TRUE) {
        rc = appRefEventsToTestsHandlerBind(cpssUniEventArr,arrLength,operation);
    }
#endif /*CPSS_APP_PLATFORM_REFERENCE*/

#if !defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)
    if (cpssAppPlatformInitialized == GT_FALSE) {
        rc = appDemoEventsToTestsHandlerBind(cpssUniEventArr,arrLength,operation);
    }
#endif /*!defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)*/
    return rc;
}

/**
* @internal prvWrAppSkipEventMaskSet function
* @endinternal
*
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_PARAM             - on bad parameter
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS prvWrAppSkipEventMaskSet
(
    IN GT_U8                       devNum,
    IN CPSS_EVENT_MASK_SET_ENT     operation
)
{
    GT_STATUS rc = GT_OK;
#ifdef CPSS_APP_PLATFORM_REFERENCE
    if (cpssAppPlatformInitialized == GT_TRUE) {
        (void)devNum;
        (void)operation;
    }
#endif /*CPSS_APP_PLATFORM_REFERENCE*/

#if !defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)
    if (cpssAppPlatformInitialized == GT_FALSE) {
        rc = appDemoDeviceSkipEventMaskSet(devNum, operation);
    }
#endif /*!defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)*/
    return rc;
}

/**
* internal prvWrAppGenEventCounterGet function
* @endinternal
*
* @brief   get the number of times that specific event happened.
*
* @param[in] devNum                   - device number
* @param[in] uniEvent                 - unified event
* @param[in] clearOnRead              - do we 'clear' the counter after 'read' it
*                                      GT_TRUE - set counter to 0 after get it's value
*                                      GT_FALSE - don't update the counter (only read it)
*
* @param[out] counterPtr               - (pointer to)the counter (the number of times that specific event happened)
*
* @retval GT_OK                    - on success.
* @retval GT_BAD_PARAM             - on bad devNum or uniEvent.
* @retval GT_BAD_PTR               - on NULL pointer.
* @retval GT_NOT_INITIALIZED       - the counters DB not initialized for the device.
*
* @note none
*
*/
GT_STATUS prvWrAppGenEventCounterGet
(
    IN  GT_U8                    devNum,
    IN  CPSS_UNI_EV_CAUSE_ENT    uniEvent,
    IN  GT_BOOL                  clearOnRead,
    OUT GT_U32                  *counterPtr
)
{
    return cpssAppUtilsEventCounterGet(devNum,uniEvent,clearOnRead,counterPtr);
}

/**
* @internal prvWrAppOsLogModeSet function
* @endinternal
*
* @brief   Function for setting the mode of cpss log
*
* @param[in] mode                     -  to be set.
* @param[in] name                     - the  of the file.
*
* @retval GT_OK                    - if succsess
* @retval GT_BAD_PTR               - failed to open the file
*/
GT_STATUS prvWrAppOsLogModeSet
(
    IN    APP_DEMO_CPSS_LOG_MODE_ENT      mode,
    IN    GT_CHAR_PTR                     name
)
{
    GT_STATUS rc = GT_OK;
#ifdef CPSS_APP_PLATFORM_REFERENCE
    if (cpssAppPlatformInitialized == GT_TRUE) {
        (void)mode;
        (void)name;
        /*no operation to be done since there is no log written to file in App Platform*/
        GT_UNUSED_PARAM(mode);
        GT_UNUSED_PARAM(name);
    }
#endif /*CPSS_APP_PLATFORM_REFERENCE*/

#if !defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)
    if (cpssAppPlatformInitialized == GT_FALSE) {
        rc = appDemoOsLogModeSet(mode,name);
    }
#endif /*!defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)*/
    return rc;
}

/**
* @internal prvWrAppOsLogStop function
* @endinternal
*
* @brief   Function for stop writing the logs into the file
*/
GT_VOID prvWrAppOsLogStop()
{
#ifdef CPSS_APP_PLATFORM_REFERENCE
    if (cpssAppPlatformInitialized == GT_TRUE) {
        /*no operation to be done since there is no log written to file in App Platform*/
    }
#endif /*CPSS_APP_PLATFORM_REFERENCE*/

#if !defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)
    if (cpssAppPlatformInitialized == GT_FALSE) {
        appDemoOsLogStop();
    }
#endif /*!defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)*/
}

/**
* @internal prvWrAppStaticDbEntryGet function
* @endinternal
*
* @brief   Get parameter value from AppDemo 'static' database .
*         This DB is not reset by the 'system reset' mechanism.
* @param[in] namePtr                  - points to parameter name
*
* @param[out] valuePtr                 - points to parameter value
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - if name is too long
* @retval GT_BAD_PTR               - if NULL pointer
* @retval GT_NO_SUCH               - there is no such parameter in Database
*/
GT_STATUS prvWrAppStaticDbEntryGet
(
    IN  GT_CHAR *namePtr,
    OUT GT_U32  *valuePtr
)
{
    GT_STATUS rc = GT_OK;
#ifdef CPSS_APP_PLATFORM_REFERENCE
    if (cpssAppPlatformInitialized == GT_TRUE) {
        (void)namePtr;
        (void)valuePtr;
        return GT_FAIL;
    }
#endif /*CPSS_APP_PLATFORM_REFERENCE*/

#if !defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)
    if (cpssAppPlatformInitialized == GT_FALSE) {
        rc = appDemoStaticDbEntryGet(namePtr,valuePtr);
    }
#endif /*!defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)*/
    return rc;
}

/**
* @internal prvWrAppSupportAaMessageSet function
* @endinternal
*
* @brief   Configure the appDemo to support/not support the AA messages to CPU
*
* @param[in] supportAaMessage         -
*                                      GT_FALSE - not support processing of AA messages
*                                      GT_TRUE - support processing of AA messages
*                                       nothing
*/
GT_VOID prvWrAppSupportAaMessageSet
(
    IN GT_BOOL supportAaMessage
)
{
#ifdef CPSS_APP_PLATFORM_REFERENCE
    if (cpssAppPlatformInitialized == GT_TRUE) {
        appRefSupportAaMessageSet(supportAaMessage);
    }
#endif /*CPSS_APP_PLATFORM_REFERENCE*/

#if !defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)
    if (cpssAppPlatformInitialized == GT_FALSE) {
        appDemoSupportAaMessageSet(supportAaMessage);
    }
#endif /*!defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)*/
}

/**
* @internal prvWrAppHwAccessCounterGet function
* @endinternal
*
* @brief   The function gets number of hw accesses
*
* @param[out] hwAccessCounterPtr       - points to hw access counter value.
*
* @retval GT_OK                    - on success
*/
GT_STATUS prvWrAppHwAccessCounterGet
(
    OUT GT_U32   *hwAccessCounterPtr
)
{
    GT_STATUS rc = GT_OK;
#ifdef CPSS_APP_PLATFORM_REFERENCE
    if (cpssAppPlatformInitialized == GT_TRUE) {
        rc = appRefHwAccessCounterGet(hwAccessCounterPtr);
    }
#endif /*CPSS_APP_PLATFORM_REFERENCE*/

#if !defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)
    if (cpssAppPlatformInitialized == GT_FALSE) {
        rc = appDemoHwAccessCounterGet(hwAccessCounterPtr);
    }
#endif /*!defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)*/
    return rc;
}

/**
* @internal prvWrAppHwAccessFailerBind function
* @endinternal
*
* @brief   The function binds/unbinds a WriteFailerCb callback for HW write access
*         and set hw write fail counter.
* @param[in] bind                     -  GT_TRUE -  callback routines.
*                                      GT_FALSE - un bind callback routines.
* @param[in] failCounter              - hw write fail counter: set hw write sequence
*                                      number on which write operation should fail.
*
* @retval GT_OK                    - on success
*/
GT_STATUS prvWrAppHwAccessFailerBind
(
    IN GT_BOOL                  bind,
    GT_U32                      failCounter
)
{
    GT_STATUS rc = GT_OK;
#ifdef CPSS_APP_PLATFORM_REFERENCE
    if (cpssAppPlatformInitialized == GT_TRUE) {
        rc = appRefHwAccessFailerBind(bind,failCounter);
    }
#endif /*CPSS_APP_PLATFORM_REFERENCE*/

#if !defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)
    if (cpssAppPlatformInitialized == GT_FALSE) {
        rc = appDemoHwAccessFailerBind(bind,failCounter);
    }
#endif /*!defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)*/
    return rc;
}

/**
* @internal prvWrAppTraceHwAccessClearDb function
* @endinternal
*
* @brief   Clear HW access db
*
* @retval GT_OK                    - on success else if failed
*/
GT_STATUS prvWrAppTraceHwAccessClearDb
(
    GT_VOID
)
{
    GT_STATUS rc = GT_OK;
#ifdef CPSS_APP_PLATFORM_REFERENCE
    if (cpssAppPlatformInitialized == GT_TRUE) {
        rc = appRefTraceHwAccessClearDb();
    }
#endif /*CPSS_APP_PLATFORM_REFERENCE*/

#if !defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)
    if (cpssAppPlatformInitialized == GT_FALSE) {
        rc = appDemoTraceHwAccessClearDb();
    }
#endif /*!defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)*/
    return rc;
}

/**
* @internal prvWrAppTraceHwAccessDbIsCorrupted function
* @endinternal
*
* @brief   Check if the HW access data base has reached the limit
*
* @retval GT_OK                    - on success else if failed
*/

GT_STATUS prvWrAppTraceHwAccessDbIsCorrupted
(
    GT_BOOL * corrupted
)
{
    GT_STATUS rc = GT_OK;
#ifdef CPSS_APP_PLATFORM_REFERENCE
    if (cpssAppPlatformInitialized == GT_TRUE) {
        rc = appRefTraceHwAccessDbIsCorrupted(corrupted);
    }
#endif /*CPSS_APP_PLATFORM_REFERENCE*/

#if !defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)
    if (cpssAppPlatformInitialized == GT_FALSE) {
        rc = appDemoTraceHwAccessDbIsCorrupted(corrupted);
    }
#endif /*!defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)*/
    return rc;
}

/**
* @internal prvWrAppTraceHwAccessEnable function
* @endinternal
*
* @brief   Trace HW read access information.
*
* @param[in] devNum                   - PP device number
* @param[in] accessType               - access type: read or write
* @param[in] enable                   - GT_TRUE:  tracing for given access type
*                                      GT_FALSE: disable tracing for given access type
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on fail
* @retval GT_BAD_PARAM             - on bad access type
*/
GT_STATUS prvWrAppTraceHwAccessEnable
(
    GT_U8                                  devNum,
    CPSS_ENABLER_TRACE_HW_ACCESS_TYPE_ENT  accessType,
    GT_BOOL                                enable
)
{
    GT_STATUS rc = GT_OK;
#ifdef CPSS_APP_PLATFORM_REFERENCE
    if (cpssAppPlatformInitialized == GT_TRUE) {
        rc = appRefTraceHwAccessEnable(devNum,accessType,enable);
    }
#endif /*CPSS_APP_PLATFORM_REFERENCE*/

#if !defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)
    if (cpssAppPlatformInitialized == GT_FALSE) {
        rc = appDemoTraceHwAccessEnable(devNum,accessType,enable);
    }
#endif /*!defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)*/
    return rc;
}

/**
* @internal prvWrAppTraceHwAccessInfoCompare function
* @endinternal
*
* @brief   Compare given data and parameters with trace hw access DB info on
*         specified index.
* @param[in] dev                      - device number
* @param[in] accessType               - access type: read or write
* @param[in] index                    -  in the hw access db
* @param[in] portGroupId              - port group ID
* @param[in] isrContext               - GT_TRUE: ISR contextt
*                                      GT_FALSE: TASK context
* @param[in] addr                     - memory address space
* @param[in] addr                     - addresses to compare
* @param[in] mask                     -  to  the data from the db
* @param[in] data                     - data (read/written) to compare
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - when there is no match between db info and input
*                                       parameters.
* @retval GT_BAD_STATE             - on unexpected ISR context
*/
GT_STATUS prvWrAppTraceHwAccessInfoCompare
(
    IN GT_U8                               dev,
    CPSS_ENABLER_TRACE_HW_ACCESS_TYPE_ENT      accessType,
    IN GT_U32                              index,
    IN GT_U32                              portGroupId,
    IN GT_BOOL                             isrContext,
    IN CPSS_TRACE_HW_ACCESS_ADDR_SPACE_ENT addrSpace,
    IN GT_U32                              addr,
    IN GT_U32                              mask,
    IN GT_U32                              data
)
{
    GT_STATUS rc = GT_OK;
#ifdef CPSS_APP_PLATFORM_REFERENCE
    if (cpssAppPlatformInitialized == GT_TRUE) {
        rc = appRefTraceHwAccessInfoCompare(dev,accessType,index,portGroupId,isrContext,addrSpace,addr,mask,data);
    }
#endif /*CPSS_APP_PLATFORM_REFERENCE*/

#if !defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)
    if (cpssAppPlatformInitialized == GT_FALSE) {
        rc = appDemoTraceHwAccessInfoCompare(dev,accessType,index,portGroupId,isrContext,addrSpace,addr,mask,data);
    }
#endif /*!defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)*/
    return rc;
}

/**
* @internal prvWrAppTraceHwAccessOutputModeSet function
* @endinternal
*
* @brief   Set output tracing mode.
*
* @param[in] mode                     - output tracing mode: print, printSync or store.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_STATE             - on bad state
* @retval GT_BAD_PARAM             - on bad mode
*
* @note It is not allowed to change mode, while one of the HW Access DB
*       is enabled.
*
*/
GT_STATUS prvWrAppTraceHwAccessOutputModeSet
(
    IN CPSS_ENABLER_TRACE_OUTPUT_MODE_ENT   mode
)
{
    GT_STATUS rc = GT_OK;
#ifdef CPSS_APP_PLATFORM_REFERENCE
    if (cpssAppPlatformInitialized == GT_TRUE) {
        rc = appRefTraceHwAccessOutputModeSet(mode);
    }
#endif /*CPSS_APP_PLATFORM_REFERENCE*/

#if !defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)
    if (cpssAppPlatformInitialized == GT_FALSE) {
        rc = appDemoTraceHwAccessOutputModeSet(mode);
    }
#endif /*!defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)*/
    return rc;
}

/**
* @internal prvWrAppEventFatalErrorEnable function
* @endinternal
*
* @brief   Set fatal error handling type.
*
* @param[in] fatalErrorType           - fatal error handling type
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - on wrong fatalErrorType
*/
GT_STATUS prvWrAppEventFatalErrorEnable
(
    CPSS_ENABLER_FATAL_ERROR_TYPE fatalErrorType
)
{
    GT_STATUS rc = GT_OK;
#ifdef CPSS_APP_PLATFORM_REFERENCE
    if (cpssAppPlatformInitialized == GT_TRUE) {
        rc = appRefEventFatalErrorEnable(fatalErrorType);
    }
#endif /*CPSS_APP_PLATFORM_REFERENCE*/

#if !defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)
    if (cpssAppPlatformInitialized == GT_FALSE) {
        rc = appDemoEventFatalErrorEnable(fatalErrorType);
    }
#endif /*!defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)*/
    return rc;
}

/**
* @internal prvWrAppInitSystemGet function
* @endinternal
*
* @brief   Function sets parameters of cpss init system proccess.
*
* @param[out] boardIdxPtr              - (pointer to) The index of the board to be initialized
*                                      from the board list.
* @param[out] boardRevIdPtr            - (pointer to) Board revision Id.
* @param[out] reloadEepromPtr          - (pointer to) Whether the Eeprom should be reloaded when
*                                      corePpHwStartInit() is called.
*                                       None.
*/
void prvWrAppInitSystemGet
(
    OUT GT_U32  *boardIdxPtr,
    OUT GT_U32  *boardRevIdPtr,
    OUT GT_U32  *reloadEepromPtr
)
{
#ifdef CPSS_APP_PLATFORM_REFERENCE
    if (cpssAppPlatformInitialized == GT_TRUE) {
        (void)reloadEepromPtr;
        *boardIdxPtr = 29; /* aldrin2 */
        *boardRevIdPtr = 1;
        GT_UNUSED_PARAM(reloadEepromPtr);
    }
#endif /*CPSS_APP_PLATFORM_REFERENCE*/

#if !defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)
    if (cpssAppPlatformInitialized == GT_FALSE) {
        cpssInitSystemGet(boardIdxPtr, boardRevIdPtr, reloadEepromPtr);
    }
#endif /*!defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)*/
}

/**
* @internal prvWrAppIsHirApp function
* @endinternal
*
* @brief   check appDemo is running in HIR mode.
*
* @retval GT_TRUE       - HIR mode
* @retval GT_FALSE      - non HIR mode
*/
GT_BOOL prvWrAppIsHirApp
(
    GT_VOID
)
{
#ifdef CPSS_APP_PLATFORM_REFERENCE
    if (cpssAppPlatformInitialized == GT_TRUE) {
        return GT_FALSE;
    }
#endif /*CPSS_APP_PLATFORM_REFERENCE*/

#if !defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)
    if (cpssAppPlatformInitialized == GT_FALSE) {
        return isHirApp;
    }
#endif /*!defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)*/
    return GT_FALSE;
}

/**
* @internal prvWrAppTrunkPhy1690_WA_B_Get function
* @endinternal
*
* @brief  indication for Galtis wrappers call the WA 'B' APIs.
*
* @retval 1        - trunk phy1690_WA 'B'APIs
* @retval 0        - otherwise
*/
GT_U32 prvWrAppTrunkPhy1690_WA_B_Get
(
    GT_VOID
)
{
#ifdef CPSS_APP_PLATFORM_REFERENCE
    if (cpssAppPlatformInitialized == GT_TRUE) {
        return 0;
    }
#endif /*CPSS_APP_PLATFORM_REFERENCE*/

#if !defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)
    if (cpssAppPlatformInitialized == GT_FALSE) {
        #ifdef CHX_FAMILY
            return appDemo_trunk_phy1690_WA_B;
        #else
            return 0;
        #endif
    }
#endif /*!defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)*/
    return 0;
}

/**
* @internal prvWrAppForceAutoLearn function
* @endinternal
*
* @brief  returns FDB Auto learn Status.
*
*/
GT_BOOL prvWrAppForceAutoLearn
(
    GT_VOID
)
{
#ifdef CPSS_APP_PLATFORM_REFERENCE
    if (cpssAppPlatformInitialized == GT_TRUE) {
        return GT_FALSE; /* TBA */
    }
#endif /*CPSS_APP_PLATFORM_REFERENCE*/

#if !defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)
    if (cpssAppPlatformInitialized == GT_FALSE) {
        return appDemoSysConfig.forceAutoLearn;
    }
#endif /*!defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)*/
    return GT_FALSE;
}

/**
* @internal prvWrAppunifiedFdbPortGroupsBmpGet function
* @endinternal
*
* @brief  returns unified Fdb Port group Bitmap.
*
* @param[in] dev       - device number
* @param[in] portGroup - portGroup number
*
*/
GT_PORT_GROUPS_BMP prvWrAppunifiedFdbPortGroupsBmpGet
(
    IN GT_U8 dev,
    IN GT_U32 portGroup
)
{
#ifdef CPSS_APP_PLATFORM_REFERENCE
    if (cpssAppPlatformInitialized == GT_TRUE) {
        (void)dev;
        (void)portGroup;
        return 0; /* TBA */
    }
#endif /*CPSS_APP_PLATFORM_REFERENCE*/

#if !defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)
    if (cpssAppPlatformInitialized == GT_FALSE) {
        return appDemoPpConfigList[dev].portGroupsInfo[portGroup].unifiedFdbPortGroupsBmp;
    }
#endif /*!defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)*/
    return 0;
}

/**
* @internal prvWrAppUplinkPortGroupPortsBmpGet function
* @endinternal
*
* @brief  returns uplink port Group Bitmap.
*
* @param[in] dev       - device number
*
* @param[out] portsBmp - pointer to uplink PortGroup PortsBmp.
*
* @retval GT_OK        - on success
* @retval GT_FAIL      - otherwise
*/
GT_STATUS prvWrAppUplinkPortGroupPortsBmpGet
(
    IN GT_U8 dev,
    OUT CPSS_PORTS_BMP_STC **portsBmp
)
{
#ifdef CPSS_APP_PLATFORM_REFERENCE
    if (cpssAppPlatformInitialized == GT_TRUE) {
        (void)dev;
        (void)portsBmp;
        return GT_FAIL; /* not implemented */
    }
#endif /*CPSS_APP_PLATFORM_REFERENCE*/

#if !defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)
    if (cpssAppPlatformInitialized == GT_FALSE) {
        if(appDemoPpConfigList[dev].valid == GT_FALSE)
            return GT_FAIL;
        *portsBmp = &(appDemoPpConfigList[dev].uplinkPortGroupPortsBmp);
        return GT_OK;
    }
#endif /*!defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)*/
    return GT_FAIL;
}

/**
* @internal prvWrAppMultiPortGroupRingPortsBmpGet function
* @endinternal
*
* @brief  Get MutliPortGroup ring Ports Bitmap.
*
* @param[in] dev       - device number
*
* @param[out] portsBmp - pointer to MutliPortGroup ringPortsBmp.
*
* @retval GT_OK        - on success
* @retval GT_FAIL      - otherwise
*/
GT_STATUS prvWrAppMultiPortGroupRingPortsBmpGet
(
    IN GT_U8 dev,
    OUT CPSS_PORTS_BMP_STC **portsBmp
)
{
#ifdef CPSS_APP_PLATFORM_REFERENCE
    if (cpssAppPlatformInitialized == GT_TRUE) {
        (void)dev;
        (void)portsBmp;
        return GT_FAIL; /* not implemented */
    }
#endif /*CPSS_APP_PLATFORM_REFERENCE*/

#if !defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)
    if (cpssAppPlatformInitialized == GT_FALSE) {
        if(appDemoPpConfigList[dev].valid == GT_FALSE)
            return GT_FAIL;
        *portsBmp = &(appDemoPpConfigList[dev].multiPortGroupRingPortsBmp);
        return GT_OK;
    }
#endif /*!defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)*/
    return GT_FAIL;
}

/**
* @internal prvWrAppRelayPortGroupPortsBmpGet function
* @endinternal
*
* @brief  returns relay ports Bitmap.
*
* @param[in] dev       - device number
*
* @param[out] portsBmp - pointer to relay PortsBmp.
*
* @retval GT_OK        - on success
* @retval GT_FAIL      - otherwise
*/
GT_STATUS prvWrAppRelayPortGroupPortsBmpGet
(
    IN GT_U8 dev,
    OUT CPSS_PORTS_BMP_STC **portsBmp
)
{
#ifdef CPSS_APP_PLATFORM_REFERENCE
    if (cpssAppPlatformInitialized == GT_TRUE) {
        (void)dev;
        (void)portsBmp;
        return GT_FAIL; /* not implemented */
    }
#endif /*CPSS_APP_PLATFORM_REFERENCE*/

#if !defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)
    if (cpssAppPlatformInitialized == GT_FALSE) {
        if(appDemoPpConfigList[dev].valid == GT_FALSE)
            return GT_FAIL;
        *portsBmp = &(appDemoPpConfigList[dev].relayPortGroupPortsBmp);
        return GT_OK;
    }
#endif /*!defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)*/
    return GT_FAIL;
}

#ifdef CHX_FAMILY
GT_STATUS prvWrAppAldrinPortGobalResourcesPrint
(
    IN GT_U8 dev
)
{
    GT_STATUS rc = GT_OK;
#ifdef CPSS_APP_PLATFORM_REFERENCE
    if (cpssAppPlatformInitialized == GT_TRUE) {
        rc = appRefAldrinPortGobalResourcesPrint(dev);
    }
#endif /*CPSS_APP_PLATFORM_REFERENCE*/

#if !defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)
    if (cpssAppPlatformInitialized == GT_FALSE) {
        rc = appDemoAldrinPortGobalResourcesPrint(dev);
    }
#endif /*!defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)*/
    return rc;
}

/**
* @internal prvWrAppBc2IpLpmRamDefaultConfigCalc function
* @endinternal
*
* @brief   This function calculate the default RAM LPM DB configuration for LPM management.
*
* @param[in] devNum                   - The Pp device number to get the parameters for.
* @param[in] maxNumOfPbrEntries       - number of PBR entries to deduct from the LPM memory calculations
*
* @param[out] ramDbCfgPtr              - (pointer to) ramDbCfg structure to hold the defaults calculated
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS prvWrAppBc2IpLpmRamDefaultConfigCalc
(
    IN  GT_U8                                    devNum,
    IN  GT_U32                                   maxNumOfPbrEntries,
    OUT CPSS_DXCH_LPM_RAM_CONFIG_STC             *ramDbCfgPtr
)
{
    GT_STATUS rc = GT_OK;
#ifdef CPSS_APP_PLATFORM_REFERENCE
    if (cpssAppPlatformInitialized == GT_TRUE) {
        rc = appPlatformBc2IpLpmRamDefaultConfigCalc(devNum,maxNumOfPbrEntries,ramDbCfgPtr);
    }
#endif /*CPSS_APP_PLATFORM_REFERENCE*/

#if !defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)
    if (cpssAppPlatformInitialized == GT_FALSE) {
        rc = appDemoBc2IpLpmRamDefaultConfigCalc(devNum,maxNumOfPbrEntries,ramDbCfgPtr);
    }
#endif /*!defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)*/
    return rc;
}

GT_STATUS prvWrAppBobk2PortGobalResourcesPrint
(
    IN GT_U8 dev
)
{
    GT_STATUS rc = GT_OK;
#ifdef CPSS_APP_PLATFORM_REFERENCE
    if (cpssAppPlatformInitialized == GT_TRUE) {
        (void)dev;
        /*to be filled later*/
        GT_UNUSED_PARAM(dev);
    }
#endif /*CPSS_APP_PLATFORM_REFERENCE*/

#if !defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)
    if (cpssAppPlatformInitialized == GT_FALSE) {
        rc = appDemoBobk2PortGobalResourcesPrint(dev);
    }
#endif /*!defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)*/
    return rc;
}

GT_STATUS prvWrAppBobk2PortListResourcesPrint
(
    IN GT_U8 dev
)
{
    GT_STATUS rc = GT_OK;
#ifdef CPSS_APP_PLATFORM_REFERENCE
    if (cpssAppPlatformInitialized == GT_TRUE) {
        (void)dev;
        /*to be filled later*/
        GT_UNUSED_PARAM(dev);
    }
#endif /*CPSS_APP_PLATFORM_REFERENCE*/

#if !defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)
    if (cpssAppPlatformInitialized == GT_FALSE) {
        rc = appDemoBobk2PortListResourcesPrint(dev);
    }
#endif /*!defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)*/
    return rc;
}

/**
* @internal prvWrAppCaelumEgressCntrReset function
* @endinternal
*
* @brief   Function resets TxQ Egress counters on Caelum devices.
*
* @param[in] devNum                   - device number
* @param[in] cntrSetNum               - counter set number to be reset
*
* @retval GT_OK                    - on success,
* @retval GT_BAD_STATE             - traffic is still run in device. Cannot execute the WA.
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS prvWrAppCaelumEgressCntrReset
(
    IN GT_U8        devNum,
    IN GT_U32       cntrSetNum
)
{
    GT_STATUS rc = GT_OK;
#ifdef CPSS_APP_PLATFORM_REFERENCE
    if (cpssAppPlatformInitialized == GT_TRUE) {
        (void)devNum;
        (void)cntrSetNum;
        /*to be filled later*/
        GT_UNUSED_PARAM(devNum);
        GT_UNUSED_PARAM(cntrSetNum);
    }
#endif /*CPSS_APP_PLATFORM_REFERENCE*/

#if !defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)
    if (cpssAppPlatformInitialized == GT_FALSE) {
        rc = appDemoCaelumEgressCntrReset(devNum,cntrSetNum);
    }
#endif /*!defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)*/
    return rc;
}

/**
* @internal prvWrAppDxAldrin2TailDropDbaEnableRestore function
* @endinternal
*
*
* @param[in] dev                      -   device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvWrAppDxAldrin2TailDropDbaEnableRestore
(
    IN  GT_U8     dev
)
{
    GT_STATUS rc = GT_OK;
#ifdef CPSS_APP_PLATFORM_REFERENCE
    if (cpssAppPlatformInitialized == GT_TRUE) {
        rc = appRefDxAldrin2TailDropDbaEnableRestore(dev);
    }
#endif /*CPSS_APP_PLATFORM_REFERENCE*/

#if !defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)
    if (cpssAppPlatformInitialized == GT_FALSE) {
        rc = appDemoDxAldrin2TailDropDbaEnableRestore(dev);
    }
#endif /*!defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)*/
    return rc;
}


/**
* @internal prvWrAppDxAldrin2TailDropDbaEnableSet function
* @endinternal
*
*
* @param[in] dev                      -   device number
* @param[in] enable                   -   enable (GT_TRUE) or disable (GT_FALSE) Tail Drop DBA
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvWrAppDxAldrin2TailDropDbaEnableSet
(
    IN  GT_U8     dev,
    IN  GT_BOOL   enable
)
{
    GT_STATUS rc = GT_OK;
#ifdef CPSS_APP_PLATFORM_REFERENCE
    if (cpssAppPlatformInitialized == GT_TRUE) {
        rc = appRefDxAldrin2TailDropDbaEnableSet(dev,enable);
    }
#endif /*CPSS_APP_PLATFORM_REFERENCE*/

#if !defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)
    if (cpssAppPlatformInitialized == GT_FALSE) {
        rc = appDemoDxAldrin2TailDropDbaEnableSet(dev,enable);
    }
#endif /*!defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)*/
    return rc;
}

/**
* @internal prvWrAppDxChNewTtiTcamSupportSet function
* @endinternal
*
*
* @param[in] useTTIOffset         - whether to take the TTI offset in TCAM into consideration
*/
GT_VOID prvWrAppDxChNewTtiTcamSupportSet
(
    IN  GT_BOOL             useTTIOffset
)
{
#ifdef CPSS_APP_PLATFORM_REFERENCE
    if (cpssAppPlatformInitialized == GT_TRUE) {
        appRefDxChNewTtiTcamSupportSet(useTTIOffset);
    }
#endif /*CPSS_APP_PLATFORM_REFERENCE*/

#if !defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)
    if (cpssAppPlatformInitialized == GT_FALSE) {
        appDemoDxChNewTtiTcamSupportSet(useTTIOffset);
    }
#endif /*!defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)*/
}

/**
* @internal prvWrAppDxChPhase1ParamsGet function
* @endinternal
*
* @brief   Gets the parameters used during phase 1 init.
*
* @param[out] ppPhase1ParamsPtr        - (pointer to) parameters used during phase 1 init.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS prvWrAppDxChPhase1ParamsGet
(
    OUT CPSS_DXCH_PP_PHASE1_INIT_INFO_STC *ppPhase1ParamsPtr
)
{
    GT_STATUS rc = GT_OK;
#ifdef CPSS_APP_PLATFORM_REFERENCE
    if (cpssAppPlatformInitialized == GT_TRUE) {
        rc = appRefDxChPhase1ParamsGet(ppPhase1ParamsPtr);
    }
#endif /*CPSS_APP_PLATFORM_REFERENCE*/

#if !defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)
    if (cpssAppPlatformInitialized == GT_FALSE) {
        rc = appDemoDxChPhase1ParamsGet(ppPhase1ParamsPtr);
    }
#endif /*!defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)*/
    return rc;
}

/**
* @internal prvWrAppDxChTcamClientSectionSet function
* @endinternal
*
* @brief   Sets TCAM Section for Client rules : Base index + Num Of Indexes
*/
GT_STATUS prvWrAppDxChTcamClientSectionSet
(
    IN     CPSS_DXCH_TCAM_CLIENT_ENT        client,
    IN     GT_U32                           hitNum,
    IN     GT_U32                           baseIndex,
    IN     GT_U32                           numOfIndexes
)
{
    GT_STATUS rc = GT_OK;
#ifdef CPSS_APP_PLATFORM_REFERENCE
    if (cpssAppPlatformInitialized == GT_TRUE) {
        rc = appRefDxChTcamClientSectionSet(client,hitNum,baseIndex,numOfIndexes);
    }
#endif /*CPSS_APP_PLATFORM_REFERENCE*/

#if !defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)
    if (cpssAppPlatformInitialized == GT_FALSE) {
        rc = appDemoDxChTcamClientSectionSet(client,hitNum,baseIndex,numOfIndexes);
    }
#endif /*!defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)*/
    return rc;
}

/**
* @internal prvWrAppDxChTcamEpclBaseIndexGet function
* @endinternal
*
* @brief   Gets TCAM base index for EPCL rules
*
* @param[in] devNum                   - device number
*                                       The converted rule index.
*/
GT_U32 prvWrAppDxChTcamEpclBaseIndexGet
(
    IN     GT_U8                            devNum
)
{
    GT_U32 rc = GT_OK;
#ifdef CPSS_APP_PLATFORM_REFERENCE
    if (cpssAppPlatformInitialized == GT_TRUE) {
        rc = appRefDxChTcamEpclBaseIndexGet(devNum);
    }
#endif /*CPSS_APP_PLATFORM_REFERENCE*/

#if !defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)
    if (cpssAppPlatformInitialized == GT_FALSE) {
        rc = appDemoDxChTcamEpclBaseIndexGet(devNum);
    }
#endif /*!defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)*/
    return rc;
}

/**
* @internal prvWrAppDxChTcamEpclNumOfIndexsGet function
* @endinternal
*
* @brief   Gets TCAM number of indexes for EPCL rules
*
* @param[in] devNum                   - device number
*                                       TCAM number of indexes for EPCL rules.
*/
GT_U32 prvWrAppDxChTcamEpclNumOfIndexsGet
(
    IN     GT_U8                            devNum
)
{
    GT_U32 rc = GT_OK;
#ifdef CPSS_APP_PLATFORM_REFERENCE
    if (cpssAppPlatformInitialized == GT_TRUE) {
        rc = appRefDxChTcamEpclNumOfIndexsGet(devNum);
    }
#endif /*CPSS_APP_PLATFORM_REFERENCE*/

#if !defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)
    if (cpssAppPlatformInitialized == GT_FALSE) {
        rc = appDemoDxChTcamEpclNumOfIndexsGet(devNum);
    }
#endif /*!defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)*/
    return rc;
}

/**
* @internal prvWrAppDxChTcamIpclBaseIndexGet function
* @endinternal
*
* @brief   Gets TCAM base index for IPCL0/1/2 rules
*
* @param[in] devNum                   - device number
* @param[in] lookupId                 - lookup 0/1/2
*                                       The converted rule index.
*/
GT_U32 prvWrAppDxChTcamIpclBaseIndexGet
(
    IN     GT_U8                            devNum,
    IN     GT_U32                           lookupId
)
{
    GT_U32 rc = GT_OK;
#ifdef CPSS_APP_PLATFORM_REFERENCE
    if (cpssAppPlatformInitialized == GT_TRUE) {
        rc = appRefDxChTcamIpclBaseIndexGet(devNum,lookupId);
    }
#endif /*CPSS_APP_PLATFORM_REFERENCE*/

#if !defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)
    if (cpssAppPlatformInitialized == GT_FALSE) {
        rc = appDemoDxChTcamIpclBaseIndexGet(devNum,lookupId);
    }
#endif /*!defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)*/
    return rc;
}

/**
* @internal prvWrAppDxChTcamIpclNumOfIndexsGet function
* @endinternal
*
* @brief   Gets TCAM number of indexes for IPCL0/1/2 rules
*
* @param[in] devNum                   - device number
* @param[in] lookupId                 - lookup 0/1/2
*                                       TCAM number of indexes for IPCL0/1/2 rules.
*/
GT_U32 prvWrAppDxChTcamIpclNumOfIndexsGet
(
    IN     GT_U8                            devNum,
    IN     GT_U32                           lookupId
)
{
    GT_U32 rc = GT_OK;
#ifdef CPSS_APP_PLATFORM_REFERENCE
    if (cpssAppPlatformInitialized == GT_TRUE) {
        rc = appRefDxChTcamIpclNumOfIndexsGet(devNum,lookupId);
    }
#endif /*CPSS_APP_PLATFORM_REFERENCE*/

#if !defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)
    if (cpssAppPlatformInitialized == GT_FALSE) {
        rc = appDemoDxChTcamIpclNumOfIndexsGet(devNum,lookupId);
    }
#endif /*!defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)*/
    return rc;
}

GT_U32 prvWrAppDxChTcamPclConvertedIndexGet_fromUT
(
    IN     GT_U8                                devNum,
    IN     GT_U32                               index,
    IN     CPSS_DXCH_TCAM_RULE_SIZE_ENT         ruleSize
)
{
    GT_U32 rc = GT_OK;

    if(PRV_CPSS_SIP_6_10_CHECK_MAC(devNum) && ruleSize == CPSS_DXCH_TCAM_RULE_SIZE_10_B_E)
    {
        /* the actions are in 20B rule resolution */
        index *= 2;
    }

#ifdef CPSS_APP_PLATFORM_REFERENCE
    if (cpssAppPlatformInitialized == GT_TRUE) {
        rc = appRefDxChTcamPclConvertedIndexGet_fromUT(devNum,index,ruleSize);
    }
#endif /*CPSS_APP_PLATFORM_REFERENCE*/

#if !defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)
    if (cpssAppPlatformInitialized == GT_FALSE) {
        rc = appDemoDxChTcamPclConvertedIndexGet_fromUT(devNum,index,ruleSize);
    }
#endif /*!defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)*/
    return rc;
}

/* restore TCAM 'Sections' (base+size) for TTI and IPCL/EPLC */
GT_VOID prvWrAppDxChTcamSectionsRestore(GT_VOID)
{
#ifdef CPSS_APP_PLATFORM_REFERENCE
    if (cpssAppPlatformInitialized == GT_TRUE) {
        appRefDxChTcamSectionsRestore();
    }
#endif /*CPSS_APP_PLATFORM_REFERENCE*/

#if !defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)
    if (cpssAppPlatformInitialized == GT_FALSE) {
        appDemoDxChTcamSectionsRestore();
    }
#endif /*!defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)*/
}

/**
* @internal prvWrAppDxChTcamTtiBaseIndexGet function
* @endinternal
*
* @brief   Gets TCAM base index for TTI rules
*
* @param[in] devNum                   - device number
* @param[in] hitNum                   - hit index, applicable range depends on device
*                                       The converted rule index.
*/
GT_U32 prvWrAppDxChTcamTtiBaseIndexGet
(
    IN     GT_U8                            devNum,
    IN     GT_U32                           hitNum
)
{
    GT_U32 rc = GT_OK;
#ifdef CPSS_APP_PLATFORM_REFERENCE
    if (cpssAppPlatformInitialized == GT_TRUE) {
        rc= appRefDxChTcamTtiBaseIndexGet(devNum,hitNum);
    }
#endif /*CPSS_APP_PLATFORM_REFERENCE*/

#if !defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)
    if (cpssAppPlatformInitialized == GT_FALSE) {
        rc= appDemoDxChTcamTtiBaseIndexGet(devNum,hitNum);
    }
#endif /*!defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)*/
    return rc;
}

/**
* @internal prvWrAppDxChTcamTtiConvertedIndexGet function
* @endinternal
*
* @brief   Gets TCAM converted index for client TTI
*
* @param[in] devNum                   - device number
* @param[in] index                    - global line  in TCAM to write to.
*                                       The converted rule index.
*/
GT_U32 prvWrAppDxChTcamTtiConvertedIndexGet
(
    IN     GT_U8                            devNum,
    IN     GT_U32                           index
)
{
    GT_U32 rc = GT_OK;
#ifdef CPSS_APP_PLATFORM_REFERENCE
    if (cpssAppPlatformInitialized == GT_TRUE) {
        rc = appRefDxChTcamTtiConvertedIndexGet(devNum,index);
    }
#endif /*CPSS_APP_PLATFORM_REFERENCE*/

#if !defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)
    if (cpssAppPlatformInitialized == GT_FALSE) {
        rc = appDemoDxChTcamTtiConvertedIndexGet(devNum,index);
    }
#endif /*!defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)*/
    return rc;
}

/**
* @internal prvWrAppDxChTcamTtiNumOfIndexsGet function
* @endinternal
*
* @brief   Gets TCAM number of indexes for TTI rules
*
* @param[in] devNum                   - device number
* @param[in] hitNum                   - hit index, applicable range depends on device
*                                       TCAM number of indexes for TTI rules.
*/
GT_U32 prvWrAppDxChTcamTtiNumOfIndexsGet
(
    IN     GT_U8                            devNum,
    IN     GT_U32                           hitNum
)
{
    GT_U32 rc = GT_OK;
#ifdef CPSS_APP_PLATFORM_REFERENCE
    if (cpssAppPlatformInitialized == GT_TRUE) {
        rc = appRefDxChTcamTtiNumOfIndexsGet(devNum,hitNum);
    }
#endif /*CPSS_APP_PLATFORM_REFERENCE*/

#if !defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)
    if (cpssAppPlatformInitialized == GT_FALSE) {
        rc = appDemoDxChTcamTtiNumOfIndexsGet(devNum,hitNum);
    }
#endif /*!defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)*/
    return rc;
}

/**
* @internal prvWrAppDxHwDevNumChange function
* @endinternal
*
* @brief   Implements HW devNum changes.
*         1. 'ownDevNum'
*         2. E2PHY update
*
* @param[out] devNum                   -   SW devNum
* @param[out] hwDevNum                 -   HW devNum
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PTR               -  on bad pointer
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS prvWrAppDxHwDevNumChange
(
    IN  GT_U8                       devNum,
    IN  GT_HW_DEV_NUM               hwDevNum
)
{
    GT_STATUS rc = GT_OK;
#ifdef CPSS_APP_PLATFORM_REFERENCE
    if (cpssAppPlatformInitialized == GT_TRUE) {
        rc = prvCpssDxChHwDevNumChange(devNum,hwDevNum);
    }
#endif /*CPSS_APP_PLATFORM_REFERENCE*/

#if !defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)
    if (cpssAppPlatformInitialized == GT_FALSE) {
        rc = appDemoDxHwDevNumChange(devNum,hwDevNum);
    }
#endif /*!defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)*/
    return rc;
}

/**
* @internal prvWrAppFalconIpLpmRamDefaultConfigCalc function
* @endinternal
*
* @brief   This function calculate the default RAM LPM DB configuration for LPM management.
*
* @param[in] devNum                   - The Pp device number to get the parameters for.
* @param[in] sharedTableMode          - shared tables mode
* @param[in] maxNumOfPbrEntries       - number of PBR entries to deduct from the LPM memory calculations
*
* @param[out] ramDbCfgPtr              - (pointer to) ramDbCfg structure to hold the defaults calculated
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS prvWrAppFalconIpLpmRamDefaultConfigCalc
(
    IN  GT_U8                                    devNum,
    IN  CPSS_DXCH_CFG_SHARED_TABLE_MODE_ENT      sharedTableMode,
    IN  GT_U32                                   maxNumOfPbrEntries,
    OUT CPSS_DXCH_LPM_RAM_CONFIG_STC             *ramDbCfgPtr
)
{
    GT_STATUS rc = GT_OK;
#ifdef CPSS_APP_PLATFORM_REFERENCE
    if (cpssAppPlatformInitialized == GT_TRUE) {
        rc = appPlatformFalconIpLpmRamDefaultConfigCalc(devNum,sharedTableMode,maxNumOfPbrEntries,ramDbCfgPtr);
    }
#endif /*CPSS_APP_PLATFORM_REFERENCE*/

#if !defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)
    if (cpssAppPlatformInitialized == GT_FALSE) {
        rc = appDemoFalconIpLpmRamDefaultConfigCalc(devNum,sharedTableMode,maxNumOfPbrEntries,ramDbCfgPtr);
    }
#endif /*!defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)*/
    return rc;
}

/* is TM init required by initSystem */
GT_BOOL prvWrAppIsTmEnabled(void)
{
    GT_BOOL rc = GT_TRUE;
#ifdef CPSS_APP_PLATFORM_REFERENCE
    if (cpssAppPlatformInitialized == GT_TRUE) {
        /*to be filled later*/
        rc = GT_FALSE;
    }
#endif /*CPSS_APP_PLATFORM_REFERENCE*/

#if !defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)
    if (cpssAppPlatformInitialized == GT_FALSE) {
        rc = appDemoIsTmEnabled();
    }
#endif /*!defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)*/
    return rc;
}

/*------------------------------------------------------------------------------------------------------------------------
 *  /Cider/EBU/Bobcat2B/Bobcat2 {Current}/Reset and Init Controller/DFX Server Units - BC2 specific registers/Device SAR2
 *  address : 0x000F8204
 *  15-17  PLL_2_Config (Read-Only) TM clock frequency
 *    0x0 = Disabled;  Disabled; TM clock is disabled
 *    0x1 = 400MHz;    TM runs 400MHz, DDR3 runs 800MHz
 *    0x2 = 466MHz;    TM runs 466MHz, DDR3 runs 933MHz
 *    0x3 = 333MHz;    TM runs 333MHz, DDR3 runs 667MHz
 *    0x5 = Reserved0; TM runs from core clock, DDR3 runs 800MHz.; (TM design does not support DDR and TM_core from different source)
 *    0x6 = Reserved1; TM runs from core clock, DDR3 runs 933MHz.; (TM design does not support DDR and TM_core from different source)
 *    0x7 = Reserved; Reserved; PLL bypass
 *
 *    if SAR2 - PLL2  is 0 (disable)  , do we need to configure TM ????
 *    is TM supported ? probably not !!!
 *------------------------------------------------------------------------------------------------------------------------*/
GT_STATUS prvWrAppIsTmSupported
(
    IN   GT_U8 devNum,
    OUT  GT_BOOL *isTmSupported
)
{
    GT_STATUS rc = GT_OK;
#ifdef CPSS_APP_PLATFORM_REFERENCE
    if (cpssAppPlatformInitialized == GT_TRUE) {
        (void)devNum;
        (void)isTmSupported;
        isTmSupported = GT_FALSE;
        GT_UNUSED_PARAM(devNum);
    }
#endif /*CPSS_APP_PLATFORM_REFERENCE*/

#if !defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)
    if (cpssAppPlatformInitialized == GT_FALSE) {
        rc = appDemoIsTmSupported(devNum,isTmSupported);
    }
#endif /*!defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)*/
    return rc;
}

/**
* @internal prvWrAppLion2FabricCleanUp function
* @endinternal
*
* @brief   The function restores Lion2 device from all Lion2Fabric configurations.
*
* @param[in] lion2FabricDevNum        - Lion2 CPSS device number
*                                       GT_OK on success, others on fail.
*
* @note NONE
*
*/
GT_STATUS prvWrAppLion2FabricCleanUp
(
    IN GT_U8              lion2FabricDevNum
)
{
    GT_STATUS rc = GT_OK;
#ifdef CPSS_APP_PLATFORM_REFERENCE
    if (cpssAppPlatformInitialized == GT_TRUE) {
        (void)lion2FabricDevNum;
        /*to be filled later*/
        GT_UNUSED_PARAM(lion2FabricDevNum);
    }
#endif /*CPSS_APP_PLATFORM_REFERENCE*/

#if !defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)
    if (cpssAppPlatformInitialized == GT_FALSE) {
        rc = appDemoLion2FabricCleanUp(lion2FabricDevNum);
    }
#endif /*!defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)*/
    return rc;
}

/**
* @internal prvWrAppLion2FabricConfigureQos function
* @endinternal
*
* @brief   The function configure Lion2Fabric for TC/DP support inside Lion2 device.
*         The eDSA Tag should remain unchanged.
* @param[in] lion2FabricDevNum        - Lion2 CPSS device number
* @param[in] toCpuTc                  - TC for all TO_CPU packets
* @param[in] toCpuDp                  - DP for all TO_CPU packets
* @param[in] toAnalyzerTc             - TC for all TO_ANALYZER packets
* @param[in] toAnalyzerDp             - DP for all TO_ANALYZER packets
* @param[in] fromCpuDp                - DP for all FROM_CPU packets
*                                       GT_OK on success, others on fail.
*
* @note NONE
*
*/
GT_STATUS prvWrAppLion2FabricConfigureQos
(
    IN GT_U8              lion2FabricDevNum,
    IN GT_U32             toCpuTc,
    IN CPSS_DP_LEVEL_ENT  toCpuDp,
    IN GT_U32             toAnalyzerTc,
    IN CPSS_DP_LEVEL_ENT  toAnalyzerDp,
    IN CPSS_DP_LEVEL_ENT  fromCpuDp
)
{
    GT_STATUS rc = GT_OK;
#ifdef CPSS_APP_PLATFORM_REFERENCE
    if (cpssAppPlatformInitialized == GT_TRUE) {
        (void)lion2FabricDevNum;
        (void)toCpuTc;
        (void)toCpuDp;
        (void)toAnalyzerTc;
        (void)toAnalyzerDp;
        (void)fromCpuDp;
        /*to be filled later*/
        GT_UNUSED_PARAM(lion2FabricDevNum);
        GT_UNUSED_PARAM(toCpuTc);
        GT_UNUSED_PARAM(toCpuDp);
        GT_UNUSED_PARAM(toAnalyzerTc);
        GT_UNUSED_PARAM(toAnalyzerDp);
        GT_UNUSED_PARAM(fromCpuDp);
    }
#endif /*CPSS_APP_PLATFORM_REFERENCE*/

#if !defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)
    if (cpssAppPlatformInitialized == GT_FALSE) {
        rc = appDemoLion2FabricConfigureQos(lion2FabricDevNum,toCpuTc,toCpuDp,toAnalyzerTc,toAnalyzerDp,fromCpuDp);
    }
#endif /*!defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)*/
    return rc;
}

/**
* @internal prvWrAppLion2FabricForwardAddBc2Device function
* @endinternal
*
* @brief   The function writes PCL rules for UC packets with commands FORWARD, FROM_CPU, TO_ANALYSER
*         that must be forwarded to the given target device.
* @param[in] lion2FabricDevNum        - Lion2 CPSS device number
* @param[in] hwDevId                  - HW id of BC2 target device
* @param[in] linkPortNum              - number of Lion2 port linked to BC2 target device
*                                       GT_OK on success, others on fail.
*
* @note NONE
*
*/
GT_STATUS prvWrAppLion2FabricForwardAddBc2Device
(
    IN  GT_U8           lion2FabricDevNum,
    IN  GT_U32          hwDevId,
    IN  GT_PORT_NUM     linkPortNum
)
{
    GT_STATUS rc = GT_OK;
#ifdef CPSS_APP_PLATFORM_REFERENCE
    if (cpssAppPlatformInitialized == GT_TRUE) {
        (void)lion2FabricDevNum;
        (void)hwDevId;
        (void)linkPortNum;
        /*to be filled later*/
        GT_UNUSED_PARAM(lion2FabricDevNum);
        GT_UNUSED_PARAM(hwDevId);
        GT_UNUSED_PARAM(linkPortNum);
    }
#endif /*CPSS_APP_PLATFORM_REFERENCE*/

#if !defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)
    if (cpssAppPlatformInitialized == GT_FALSE) {
        rc = appDemoLion2FabricForwardAddBc2Device(lion2FabricDevNum,hwDevId,linkPortNum);
    }
#endif /*!defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)*/
    return rc;
}

/**
* @internal prvWrAppLion2FabricForwardAddBc2Vidx function
* @endinternal
*
* @brief   The function writes PCL rules for MC packets with the given target VIDX
*         with commands FORWARD, FROM_CPU, TO_ANALYSER
*         that must be egressed from the given set (bitmap) of Lion2 ports.
* @param[in] lion2FabricDevNum        - Lion2 CPSS device number
* @param[in] bc2Vidx                  - target VIDX (coded by BC2 device in eDSA Tag)
* @param[in] targetPortsBmpPtr        - (pointer to) bitmap of target Lion2 ports
*                                       GT_OK on success, others on fail.
*
* @note NONE
*
*/
GT_STATUS prvWrAppLion2FabricForwardAddBc2Vidx
(
    IN  GT_U8                 lion2FabricDevNum,
    IN  GT_U32                bc2Vidx,
    IN  CPSS_PORTS_BMP_STC    *targetPortsBmpPtr
)
{
    GT_STATUS rc = GT_OK;
#ifdef CPSS_APP_PLATFORM_REFERENCE
    if (cpssAppPlatformInitialized == GT_TRUE) {
        (void)lion2FabricDevNum;
        (void)bc2Vidx;
        (void)targetPortsBmpPtr;
        /*to be filled later*/
        GT_UNUSED_PARAM(lion2FabricDevNum);
        GT_UNUSED_PARAM(bc2Vidx);
        GT_UNUSED_PARAM(targetPortsBmpPtr);
    }
#endif /*CPSS_APP_PLATFORM_REFERENCE*/

#if !defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)
    if (cpssAppPlatformInitialized == GT_FALSE) {
        rc = appDemoLion2FabricForwardAddBc2Vidx(lion2FabricDevNum,bc2Vidx,targetPortsBmpPtr);
    }
#endif /*!defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)*/
    return rc;
}

/**
* @internal prvWrAppLion2FabricForwardAddBc2Vlan function
* @endinternal
*
* @brief   The function writes PCL rules for BC packets with the given target VID
*         with commands FORWARD, FROM_CPU
*         that must be egressed from the given set (bitmap) of Lion2 ports.
* @param[in] lion2FabricDevNum        - Lion2 CPSS device number
* @param[in] bc2VlanId                - target eVID (coded by BC2 device in eDSA Tag)
* @param[in] targetPortsBmpPtr        - (pointer to) bitmap of target Lion2 ports
*                                       GT_OK on success, others on fail.
*
* @note NONE
*
*/
GT_STATUS prvWrAppLion2FabricForwardAddBc2Vlan
(
    IN  GT_U8                 lion2FabricDevNum,
    IN  GT_U32                bc2VlanId,
    IN  CPSS_PORTS_BMP_STC    *targetPortsBmpPtr
)
{
    GT_STATUS rc = GT_OK;
#ifdef CPSS_APP_PLATFORM_REFERENCE
    if (cpssAppPlatformInitialized == GT_TRUE) {
        (void)lion2FabricDevNum;
        (void)bc2VlanId;
        (void)targetPortsBmpPtr;
        /*to be filled later*/
        GT_UNUSED_PARAM(lion2FabricDevNum);
        GT_UNUSED_PARAM(bc2VlanId);
        GT_UNUSED_PARAM(targetPortsBmpPtr);
    }
#endif /*CPSS_APP_PLATFORM_REFERENCE*/

#if !defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)
    if (cpssAppPlatformInitialized == GT_FALSE) {
        rc = appDemoLion2FabricForwardAddBc2Vlan(lion2FabricDevNum,bc2VlanId,targetPortsBmpPtr);
    }
#endif /*!defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)*/
    return rc;
}

/**
* @internal prvWrAppLion2FabricInit function
* @endinternal
*
* @brief   The function initializes DB and writes PCL rule for TO_CPU packets
*
* @param[in] lion2FabricDevNum        - Lion2 CPSS device number
* @param[in] cpuLinkPortNum           - number of Lion2 port linked to BC2 devices with CPU port
*                                       GT_OK on success, others on fail.
*
* @note NONE
*
*/
GT_STATUS prvWrAppLion2FabricInit
(
    IN GT_U8              lion2FabricDevNum,
    IN GT_PORT_NUM        cpuLinkPortNum
)
{
    GT_STATUS rc = GT_OK;
#ifdef CPSS_APP_PLATFORM_REFERENCE
    if (cpssAppPlatformInitialized == GT_TRUE) {
        (void)lion2FabricDevNum;
        (void)cpuLinkPortNum;
        /*to be filled later*/
        GT_UNUSED_PARAM(lion2FabricDevNum);
        GT_UNUSED_PARAM(cpuLinkPortNum);
    }
#endif /*CPSS_APP_PLATFORM_REFERENCE*/

#if !defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)
    if (cpssAppPlatformInitialized == GT_FALSE) {
        rc = appDemoLion2FabricInit(lion2FabricDevNum,cpuLinkPortNum);
    }
#endif /*!defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)*/
    return rc;
}

/**
* @internal prvWrAppLionTrunkSrcPortHashMappingSet_ForMultiCoreFdbLookup function
* @endinternal
*
* @brief   set the trunk srcPort hash mode settings.
*
* @param[in] boardRevId               - The board revision Id.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS prvWrAppLionTrunkSrcPortHashMappingSet_ForMultiCoreFdbLookup
(
    IN  GT_U8                       boardRevId
)
{
    GT_STATUS rc = GT_OK;
#ifdef CPSS_APP_PLATFORM_REFERENCE
    if (cpssAppPlatformInitialized == GT_TRUE) {
        (void)boardRevId;
        /*to be filled later*/
        GT_UNUSED_PARAM(boardRevId);
    }
#endif /*CPSS_APP_PLATFORM_REFERENCE*/

#if !defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)
    if (cpssAppPlatformInitialized == GT_FALSE) {
        rc = appDemoLionTrunkSrcPortHashMappingSet_ForMultiCoreFdbLookup(boardRevId);
    }
#endif /*!defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)*/
    return rc;
}

#ifdef INCLUDE_TM
/**
* @internal prvWrAppBc2DramOrTmInit function
* @endinternal
*
* @brief   Board specific configurations of TM and all related HW
*         or of TM related DRAM only.
* @param[in] dev                      - device number
* @param[in] flags                    - initialization flags
*                                      currently supported value 1 that means "DRAM only"
*                                      otherwise TM and all related HW initialized
*
* @param[out] dramIfBmpPtr             - pointer to bitmap of TM DRAM interfaces. may be NULL
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*
* @note Used type of board recognized at last device (as in other code)
*
*/
GT_STATUS prvWrAppBc2DramOrTmInit
(
    IN  GT_U8       dev,
    IN  GT_U32      flags,
    OUT GT_U32     *dramIfBmpPtr
)
{
    GT_STATUS rc = GT_OK;
#ifdef CPSS_APP_PLATFORM_REFERENCE
    if (cpssAppPlatformInitialized == GT_TRUE) {
        (void)dev;
        (void)flags;
        (void)dramIfBmpPtr;
        /*to be filled later*/
    }
#endif /*CPSS_APP_PLATFORM_REFERENCE*/

#if !defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)
    if (cpssAppPlatformInitialized == GT_FALSE) {
        rc = appDemoBc2DramOrTmInit(dev,flags,dramIfBmpPtr);
    }
#endif /*!defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)*/
    return rc;
}

/**
* @internal prvWrAppTmGeneral48PortsInit function
* @endinternal
*
* @brief   Implements general 48 Ports initialization for specific device.
*
* @param[in] dev                      -   device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvWrAppTmGeneral48PortsInit
(
    IN  GT_U8   dev
)
{
    GT_STATUS rc = GT_OK;
#ifdef CPSS_APP_PLATFORM_REFERENCE
    if (cpssAppPlatformInitialized == GT_TRUE) {
        (void)dev;
        /*to be filled later*/
    }
#endif /*CPSS_APP_PLATFORM_REFERENCE*/

#if !defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)
    if (cpssAppPlatformInitialized == GT_FALSE) {
        rc = appDemoTmGeneral48PortsInit(dev);
    }
#endif /*!defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)*/
    return rc;
}

/**
* @internal prvWrAppTmGeneral4PortsInit function
* @endinternal
*
* @brief   Implements general 4 Ports initialization for specific device.
*
* @param[in] dev                      -   device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvWrAppTmGeneral4PortsInit
(
    IN  GT_U8   dev
)
{
    GT_STATUS rc = GT_OK;
#ifdef CPSS_APP_PLATFORM_REFERENCE
    if (cpssAppPlatformInitialized == GT_TRUE) {
        (void)dev;
        /*to be filled later*/
    }
#endif /*CPSS_APP_PLATFORM_REFERENCE*/

#if !defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)
    if (cpssAppPlatformInitialized == GT_FALSE) {
        rc = appDemoTmGeneral4PortsInit(dev);
    }
#endif /*!defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)*/
    return rc;
}

/**
* @internal prvWrAppTmScenario20Init function
* @endinternal
*
* @brief   Implements scenario 2 initialization for specific device.
*         10 1G ports, 4 c node per port, 5 b node per c node, 10 a node per b node, 8 queues per a node
*         shaping on all levels (a,b,c,p)
*         Tail Drop on port and queue level
*         port 64 - 40G  -> C -> B -> A -> 252 Queues
*         no shaping , no drop
* @param[in] dev                      -   device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvWrAppTmScenario20Init
(
    IN  GT_U8   dev
)
{
    GT_STATUS rc = GT_OK;
#ifdef CPSS_APP_PLATFORM_REFERENCE
    if (cpssAppPlatformInitialized == GT_TRUE) {
        (void)dev;
        /*to be filled later*/
    }
#endif /*CPSS_APP_PLATFORM_REFERENCE*/

#if !defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)
    if (cpssAppPlatformInitialized == GT_FALSE) {
        rc = appDemoTmScenario20Init(dev);
    }
#endif /*!defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)*/
    return rc;
}

/**
* @internal prvWrAppTmScenario2Init function
* @endinternal
*
* @brief   Implements scenario 2 initialization for specific device.
*         10 1G ports, 4 c node per port, 5 b node per c node, 10 a node per b node, 8 queues per a node
*         shaping on all levels (a,b,c,p)
*         Tail Drop on port and queue level
* @param[in] dev                      -   device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvWrAppTmScenario2Init
(
    IN  GT_U8   dev
)
{
    GT_STATUS rc = GT_OK;
#ifdef CPSS_APP_PLATFORM_REFERENCE
    if (cpssAppPlatformInitialized == GT_TRUE) {
        (void)dev;
        /*to be filled later*/
    }
#endif /*CPSS_APP_PLATFORM_REFERENCE*/

#if !defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)
    if (cpssAppPlatformInitialized == GT_FALSE) {
        rc = appDemoTmScenario2Init(dev);
    }
#endif /*!defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)*/
    return rc;
}

/**
* @internal prvWrAppTmScenario3Init function
* @endinternal
*
* @brief   Implements scenario 3 initialization for specific device.
*         10 1G ports, 4 c node per port, 5 b node per c node, 10 a node per b node, 8 queues per a node
*         shaping on all levels (a,b,c,p), propagated priority.
*         Tail Drop on port and queue level and color aware on Q1 in each A node.
* @param[in] dev                      -   device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvWrAppTmScenario3Init
(
    IN  GT_U8   dev
)
{
    GT_STATUS rc = GT_OK;
#ifdef CPSS_APP_PLATFORM_REFERENCE
    if (cpssAppPlatformInitialized == GT_TRUE) {
        (void)dev;
        /*to be filled later*/
    }
#endif /*CPSS_APP_PLATFORM_REFERENCE*/

#if !defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)
    if (cpssAppPlatformInitialized == GT_FALSE) {
        rc = appDemoTmScenario3Init(dev);
    }
#endif /*!defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)*/
    return rc;
}

/**
* @internal prvWrAppTmScenario4Init function
* @endinternal
*
* @brief   Implements scenario 4 initialization for specific device.
*         10 1G ports, 4 c node per port, 5 b node per c node, 10 a node per b node, 8 queues per a node
*         shaping on all levels (a,b,c,p), propagated priority.
*         Tail Drop on port and queue level and color aware on Q1 in each A node and wred mode on queue0 in each A node
* @param[in] dev                      -   device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvWrAppTmScenario4Init
(
    IN  GT_U8   dev
)
{
    GT_STATUS rc = GT_OK;
#ifdef CPSS_APP_PLATFORM_REFERENCE
    if (cpssAppPlatformInitialized == GT_TRUE) {
        (void)dev;
        /*to be filled later*/
    }
#endif /*CPSS_APP_PLATFORM_REFERENCE*/

#if !defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)
    if (cpssAppPlatformInitialized == GT_FALSE) {
        rc = appDemoTmScenario4Init(dev);
    }
#endif /*!defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)*/
    return rc;
}

/**
* @internal prvWrAppTmScenario5Init function
* @endinternal
*
* @brief   Implements scenario 5 initialization for specific device.
*         10 1G ports, 4 c node per port, 5 b node per c node, 10 a node per b node, 4 queues per a node
*         shaping on all levels (a,b,c,p), propagated priority.
*         Tail Drop on port wred drop all queues
* @param[in] dev                      -   device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvWrAppTmScenario5Init
(
    IN  GT_U8   dev
)
{
    GT_STATUS rc = GT_OK;
#ifdef CPSS_APP_PLATFORM_REFERENCE
    if (cpssAppPlatformInitialized == GT_TRUE) {
        (void)dev;
        /*to be filled later*/
    }
#endif /*CPSS_APP_PLATFORM_REFERENCE*/

#if !defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)
    if (cpssAppPlatformInitialized == GT_FALSE) {
        rc = appDemoTmScenario5Init(dev);
    }
#endif /*!defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)*/
    return rc;
}

/**
* @internal prvWrAppTmScenario6Init function
* @endinternal
*
* @brief   Implements scenario 6 initialization for specific device.
*
* @param[in] dev                      -   device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvWrAppTmScenario6Init
(
    IN  GT_U8   dev
)
{
    GT_STATUS rc = GT_OK;
#ifdef CPSS_APP_PLATFORM_REFERENCE
    if (cpssAppPlatformInitialized == GT_TRUE) {
        (void)dev;
        /*to be filled later*/
    }
#endif /*CPSS_APP_PLATFORM_REFERENCE*/

#if !defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)
    if (cpssAppPlatformInitialized == GT_FALSE) {
        rc = appDemoTmScenario6Init(dev);
    }
#endif /*!defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)*/
    return rc;
}

/**
* @internal prvWrAppTmScenario7Init function
* @endinternal
*
* @brief   Implements scenario 5 initialization for specific device.
*         10 1G ports, 4 c node per port, 5 b node per c node, 10 a node per b node, 4 queues per a node
*         shaping on all levels (a,b,c,p), propagated priority.
*         Tail Drop on port wred drop all queues
* @param[in] dev                      -   device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvWrAppTmScenario7Init
(
    IN  GT_U8   dev
)
{
    GT_STATUS rc = GT_OK;
#ifdef CPSS_APP_PLATFORM_REFERENCE
    if (cpssAppPlatformInitialized == GT_TRUE) {
        (void)dev;
        /*to be filled later*/
    }
#endif /*CPSS_APP_PLATFORM_REFERENCE*/

#if !defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)
    if (cpssAppPlatformInitialized == GT_FALSE) {
        rc = appDemoTmScenario7Init(dev);
    }
#endif /*!defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)*/
    return rc;
}

/**
* @internal prvWrAppTmScenario8Init function
* @endinternal
*
* @brief   Implements scenario 8 initialization for specific device.
*         In this scenario there are shpaers for every (VLAN,TC) pair
* @param[in] dev                      -   device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvWrAppTmScenario8Init
(
    IN  GT_U8   dev
)
{
    GT_STATUS rc = GT_OK;
#ifdef CPSS_APP_PLATFORM_REFERENCE
    if (cpssAppPlatformInitialized == GT_TRUE) {
        (void)dev;
        /*to be filled later*/
    }
#endif /*CPSS_APP_PLATFORM_REFERENCE*/

#if !defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)
    if (cpssAppPlatformInitialized == GT_FALSE) {
        rc = appDemoTmScenario8Init(dev);
    }
#endif /*!defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)*/
    return rc;
}

/**
* @internal prvWrAppTmScenario9Init function
* @endinternal
*
* @brief   Scenario for demonstration of PFC. There is one C-node for one Q-node, including nodes
*         in between
* @param[in] dev                      -   device number
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvWrAppTmScenario9Init
(
    IN  GT_U8   dev
)
{
    GT_STATUS rc = GT_OK;
#ifdef CPSS_APP_PLATFORM_REFERENCE
    if (cpssAppPlatformInitialized == GT_TRUE) {
        (void)dev;
        /*to be filled later*/
    }
#endif /*CPSS_APP_PLATFORM_REFERENCE*/

#if !defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)
    if (cpssAppPlatformInitialized == GT_FALSE) {
        rc = appDemoTmScenario9Init(dev);
    }
#endif /*!defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)*/
    return rc;
}

/**
* @internal prvWrAppTmStressScenarioInit function
* @endinternal
*
* @brief   Implements strss test
*
* @param[in] dev                      -   device number
* @param[in] mode                     -   select tree configuration  -  implementation nodes in  .c file
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - otherwise
*/
GT_STATUS prvWrAppTmStressScenarioInit
(
    IN  GT_U8   dev,
    IN  GT_U8   mode
)
{
    GT_STATUS rc = GT_OK;
#ifdef CPSS_APP_PLATFORM_REFERENCE
    if (cpssAppPlatformInitialized == GT_TRUE) {
        (void)dev;
        (void)mode;
        /*to be filled later*/
    }
#endif /*CPSS_APP_PLATFORM_REFERENCE*/

#if !defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)
    if (cpssAppPlatformInitialized == GT_FALSE) {
        rc = appDemoTmStressScenarioInit(dev,mode);
    }
#endif /*!defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)*/
    return rc;
}
#endif

/* convert the trunkId to the global EPort that represents it */
GT_STATUS prvWrAppTrunkEPortGet_phy1690_WA_B(
    IN GT_TRUNK_ID              trunkId,
    OUT GT_PORT_NUM              *globalEPortPtr
)
{
    GT_STATUS rc = GT_OK;
#ifdef CPSS_APP_PLATFORM_REFERENCE
    if (cpssAppPlatformInitialized == GT_TRUE) {
        (void)trunkId;
        (void)globalEPortPtr;
        /*to be filled later*/
        GT_UNUSED_PARAM(globalEPortPtr);
        GT_UNUSED_PARAM(trunkId);
    }
#endif /*CPSS_APP_PLATFORM_REFERENCE*/

#if !defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)
    if (cpssAppPlatformInitialized == GT_FALSE) {
        rc = trunkEPortGet_phy1690_WA_B(trunkId,globalEPortPtr);
    }
#endif /*!defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)*/
    return rc;
}

/**
* @internal prvWrAppDemoFdbManagerAuMsgHandlerMethod function
* @endinternal
*
* @brief   set AU message handler methord
*
* @param[in] auMsgHandler          - AU Message handler methord for app Demo
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - if name is too long
* @retval GT_BAD_PTR               - if NULL pointer
* @retval GT_NO_SUCH               - there is no such parameter in Database
*/

GT_STATUS prvWrAppDemoFdbManagerAuMsgHandlerMethod
(
    IN APP_DEMO_AU_MESSAGE_HANDLER_METHORD_ENT auMsgHandler
)
{
    GT_STATUS rc = GT_OK;
#ifdef CPSS_APP_PLATFORM_REFERENCE
    if (cpssAppPlatformInitialized == GT_TRUE) {
        (void)auMsgHandler;
        /*to be filled later*/
        GT_UNUSED_PARAM(auMsgHandler);
    }
#endif /*CPSS_APP_PLATFORM_REFERENCE*/

#if !defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)
    if (cpssAppPlatformInitialized == GT_FALSE) {
        rc = appDemoFdbManagerAuMsgHandlerMethod(auMsgHandler);
    }
#endif /*!defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)*/
    return rc;
}

/**
* @internal prvWrAppFirstDevNum function
* @endinternal
*
* @brief convert local devNum to system devNum to add ability to set different device numbers for stacking
*
* @param[in] enable                   - enable/disable the processing of the AUQ messages
*
* @retval GT_OK                    - on success
*
*
*/
GT_STATUS prvWrAppFirstDevNum
(
    GT_U32 devNum
)
{
    GT_U32 rc = devNum;
#ifdef CPSS_APP_PLATFORM_REFERENCE
    if (cpssAppPlatformInitialized == GT_TRUE) {
        (void)devNum;
    }
#endif /*CPSS_APP_PLATFORM_REFERENCE*/

#if !defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)
    if (cpssAppPlatformInitialized == GT_FALSE) {
        rc = SYSTEM_DEV_NUM_MAC(devNum);
    }
#endif /*!defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)*/
    return rc;
}

GT_STATUS prvWrAppDxChNetRxPacketCbRegister
(
    IN  RX_PACKET_RECEIVE_CB_FUN  rxPktReceiveCbFun
)
{
    GT_STATUS rc = GT_OK;
#ifdef CPSS_APP_PLATFORM_REFERENCE
    if (cpssAppPlatformInitialized == GT_TRUE) {
        rc = appRefDxChNetRxPacketCbRegister(rxPktReceiveCbFun);
    }
#endif /*CPSS_APP_PLATFORM_REFERENCE*/

#if !defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)
    if (cpssAppPlatformInitialized == GT_FALSE) {
        rc = appDemoDxChNetRxPacketCbRegister(rxPktReceiveCbFun);
    }
#endif /*!defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)*/
    return rc;
}

GT_STATUS prvWrAppDxChNewPclTcamSupportSet
(
    IN  GT_BOOL             enableIndexConversion
)
{
#ifdef CPSS_APP_PLATFORM_REFERENCE
    if (cpssAppPlatformInitialized == GT_TRUE) {
        appRefDxChNewPclTcamSupportSet(enableIndexConversion);
    }
#endif /*CPSS_APP_PLATFORM_REFERENCE*/

#if !defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)
    if (cpssAppPlatformInitialized == GT_FALSE) {
        appDemoDxChNewPclTcamSupportSet(enableIndexConversion);
    }
#endif /*!defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)*/

    return GT_OK;
}

/**
* @internal prvWrAppDemoFdbManagerAutoAgingStatisticsClear function
* @endinternal
*
* @brief   Clear the FDB manager auto aging statistics.
*
* @retval GT_OK                    - on success
*/

GT_STATUS prvWrAppDemoFdbManagerAutoAgingStatisticsClear()
{
    GT_STATUS rc = GT_OK;
#ifdef CPSS_APP_PLATFORM_REFERENCE
    if (cpssAppPlatformInitialized == GT_TRUE) {
        rc = appRefFdbManagerAutoAgingStatisticsClear();
    }
#endif /*CPSS_APP_PLATFORM_REFERENCE*/

#if !defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)
    if (cpssAppPlatformInitialized == GT_FALSE) {
        rc = appDemoFdbManagerAutoAgingStatisticsClear();
    }
#endif /*!defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)*/
    return rc;
}

/**
* @internal prvWrAppDemoFdbManagerAutoAgingStatisticsGet function
* @endinternal
*
* @brief   Get the FDB manager auto aging statistics.
*
* @param[out] statistics           - (pointer to) auto aging statistics.
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - if invalid input
*/

GT_STATUS prvWrAppDemoFdbManagerAutoAgingStatisticsGet
(
    OUT APP_DEMO_CPSS_DXCH_BRG_FDB_MANAGER_STATISTICS_STC *statistics
)
{
    GT_STATUS rc = GT_OK;
#ifdef CPSS_APP_PLATFORM_REFERENCE
    if (cpssAppPlatformInitialized == GT_TRUE) {
        rc = appRefFdbManagerAutoAgingStatisticsGet(statistics);
    }
#endif /*CPSS_APP_PLATFORM_REFERENCE*/

#if !defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)
    if (cpssAppPlatformInitialized == GT_FALSE) {
        rc = appDemoFdbManagerAutoAgingStatisticsGet(statistics);
    }
#endif /*!defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)*/
    return rc;
}

/**
* @internal prvWrAppDemoFdbManagerControlSet function
* @endinternal
*
* @brief   set AU message handler methord
*
* @param[in] auMsgHandler          - AU Message handler methord for app Demo
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - if name is too long
* @retval GT_BAD_PTR               - if NULL pointer
* @retval GT_NO_SUCH               - there is no such parameter in Database
*/

GT_STATUS prvWrAppDemoFdbManagerControlSet
(
    IN APP_DEMO_AU_MESSAGE_HANDLER_METHORD_ENT  auMsgHandler,
    IN GT_BOOL                                  autoAgingEn,
    IN GT_U32                                   autoAgingInterval
)
{
    GT_STATUS rc = GT_OK;
#ifdef CPSS_APP_PLATFORM_REFERENCE
    if (cpssAppPlatformInitialized == GT_TRUE) {
        rc = appRefFdbManagerControlSet(auMsgHandler, autoAgingEn, autoAgingInterval);
    }
#endif /*CPSS_APP_PLATFORM_REFERENCE*/

#if !defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)
    if (cpssAppPlatformInitialized == GT_FALSE) {
        rc = prvAppDemoFdbManagerControlSet(auMsgHandler, autoAgingEn, autoAgingInterval);
    }
#endif /*!defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)*/
    return rc;
}

/**
* @internal prvWrAppDemoFdbManagerGlobalIDSet function
* @endinternal
*
* @brief   set AU message handler methord
*
* @param[in] auMsgHandler          - FDB Manager global ID set
*
* @retval GT_OK                    - on success
* @retval GT_BAD_PARAM             - if name is too long
* @retval GT_BAD_PTR               - if NULL pointer
* @retval GT_NO_SUCH               - there is no such parameter in Database
*/
GT_STATUS prvWrAppDemoFdbManagerGlobalIDSet
(
    IN GT_U32 fdbManagerId
)
{
    GT_STATUS rc = GT_OK;
#ifdef CPSS_APP_PLATFORM_REFERENCE
    if (cpssAppPlatformInitialized == GT_TRUE) {
        rc = appRefFdbManagerGlobalIDSet(fdbManagerId);
    }
#endif /*CPSS_APP_PLATFORM_REFERENCE*/

#if !defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)
    if (cpssAppPlatformInitialized == GT_FALSE) {
        rc = prvAppDemoFdbManagerGlobalIDSet(fdbManagerId);
    }
#endif /*!defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)*/
    return rc;
}

GT_STATUS prvWrAppTraceAuqFlagSet
(
    IN GT_U32   enable
)
{
    GT_STATUS rc = GT_OK;
#ifdef CPSS_APP_PLATFORM_REFERENCE
    if (cpssAppPlatformInitialized == GT_TRUE) {
        (void)enable;
        /*not implemented in CAP*/
        GT_UNUSED_PARAM(enable);
    }
#endif /*CPSS_APP_PLATFORM_REFERENCE*/

#if !defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)
    if (cpssAppPlatformInitialized == GT_FALSE) {
        rc = appDemoTraceAuqFlagSet(enable);
    }
#endif /*!defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)*/
    return rc;
}

/**
* @internal prvWrAppPrintPortEeeInterruptInfoSet function
* @endinternal
*
* @brief   function to allow set the flag of : printEeeInterruptInfo
*
* @param[in] enable                   - enable/disable the printings of ports 'EEE interrupts info'
*
* @retval GT_OK                    - on success
*/
GT_STATUS prvWrAppPrintPortEeeInterruptInfoSet
(
    IN GT_U32   enable
)
{
    GT_STATUS rc = GT_OK;
#ifdef CPSS_APP_PLATFORM_REFERENCE
    if (cpssAppPlatformInitialized == GT_TRUE) {
        rc = appRefPrintPortEeeInterruptInfoSet(enable);
    }
#endif /*CPSS_APP_PLATFORM_REFERENCE*/

#if !defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)
    if (cpssAppPlatformInitialized == GT_FALSE) {
        rc = appDemoPrintPortEeeInterruptInfoSet(enable);
    }
#endif /*!defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)*/
    return rc;
}

/**
 * @internal prvWrAppDlbWaEnable function
 * @endinternal
 *
 * @brief   Enable/Disable DLB WA and creates a task to call waExecute periodically.
 *
 * @param[in] devNum                - device number
 * @param[in] waEnable              - status of DLB WA
 *                                    GT_TRUE     - DLB WA is enabled
 *                                    GT_FALSE    - DLB WA is disabled
 * @param[in] newDelay              - Interval between consecutive DLB WA call in milisec.
 * @retval GT_OK                    - on success.
 * @retval GT_FAIL                  - otherwise.
 * @note none
 *
 */
GT_STATUS prvWrAppDlbWaEnable
(
    GT_U8                               devNum,
    GT_BOOL                             waEnable,
    GT_U32                              newDelay
)
{
    GT_STATUS rc = GT_OK;
#ifdef CPSS_APP_PLATFORM_REFERENCE
    if (cpssAppPlatformInitialized == GT_TRUE) {
        (void)devNum;
        (void)waEnable;
        (void)newDelay;
    }
#endif /*CPSS_APP_PLATFORM_REFERENCE*/

#if !defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)
    if (cpssAppPlatformInitialized == GT_FALSE) {
        rc = appDemoDlbWaEnable(devNum, waEnable, newDelay);
    }
#endif /*!defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)*/
    return rc;
}

/**
* @internal prvWrAppPortManagerTaskSleepTimeSet function
* @endinternal
*
* @brief   debug option to state the 'sleep' for the port manager ! between 'all devices' iterations
*
* @param[in] timeInMilli
*
* @retval GT_OK                    - on success
*/
GT_STATUS prvWrAppPortManagerTaskSleepTimeSet
(
    GT_U32 timeInMilli
)
{
    GT_STATUS rc = GT_OK;
#ifdef CPSS_APP_PLATFORM_REFERENCE
    if (cpssAppPlatformInitialized == GT_TRUE) {
        rc = appRefPortManagerTaskSleepTimeSet(timeInMilli);
    }
#endif /*CPSS_APP_PLATFORM_REFERENCE*/

#if !defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)
    if (cpssAppPlatformInitialized == GT_FALSE) {
        rc = appDemoPortManagerTaskSleepTimeSet(timeInMilli);
    }
#endif /*!defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)*/
    return rc;
}

/**
* @internal prvWrAppPortManagerTaskEnableSet function
* @endinternal
*
* @brief   debug option to halt the port manager task
*
* @param[in] enable - enable or halt PM task
*
* @retval GT_OK                    - on success
*/
GT_STATUS prvWrAppPortManagerTaskEnableSet
(
    GT_U32 enable
)
{
    GT_STATUS rc = GT_OK;
#ifdef CPSS_APP_PLATFORM_REFERENCE
    if (cpssAppPlatformInitialized == GT_TRUE) {
        GT_UNUSED_PARAM(enable);
        rc = GT_OK;
    }
#endif /*CPSS_APP_PLATFORM_REFERENCE*/

#if !defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)
    if (cpssAppPlatformInitialized == GT_FALSE) {
        rc = appDemoPortManagerTaskEnableSet(enable);
    }
#endif /*!defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)*/
    return rc;
}

/**
* @internal prvWrAppPrintLinkChangeFlagSet function
* @endinternal
*
* @brief   function to allow set the flag of : printLinkChangeEnabled
*
* @param[in] enable                   - enable/disable the printings of 'link change'
*
* @retval GT_OK                    - on success
*/
GT_STATUS prvWrAppPrintLinkChangeFlagSet
(
    IN GT_U32   enable
)
{
    GT_STATUS rc = GT_OK;
#ifdef CPSS_APP_PLATFORM_REFERENCE
    if (cpssAppPlatformInitialized == GT_TRUE) {
        rc = appRefPrintLinkChangeFlagSet(enable);
    }
#endif /*CPSS_APP_PLATFORM_REFERENCE*/

#if !defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)
    if (cpssAppPlatformInitialized == GT_FALSE) {
        rc = appDemoPrintLinkChangeFlagSet(enable);
    }
#endif /*!defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)*/
    return rc;
}

/**
* @internal prvWrAppPrintLinkChangeFlagGet function
* @endinternal
*
* @brief   function to get flag state of : printLinkChangeEnabled
*
*/
GT_BOOL prvWrAppPrintLinkChangeFlagGet
(
    IN GT_VOID
)
{
    GT_STATUS rc = GT_OK;
#ifdef CPSS_APP_PLATFORM_REFERENCE
    if (cpssAppPlatformInitialized == GT_TRUE) {
        rc = appRefPrintLinkChangeFlagGet();
    }
#endif /*CPSS_APP_PLATFORM_REFERENCE*/

#if !defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)
    if (cpssAppPlatformInitialized == GT_FALSE) {
        rc = appDemoPrintLinkChangeFlagGet();
    }
#endif /*!defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)*/
    return rc;
}

/**
* @internal prvWrAppEventHandlerTaskRestore function
* @endinternal
*
* @brief   This routine unbind the unified event list
*          and remove the event handler made in eventsToTestsHandlerbind .
*
* @param[in] cpssUniEventArr[]        - The CPSS unified event list.
* @param[in] arrLength                - The unified event list length.
*/
GT_STATUS prvWrAppEventHandlerTaskRestore
(
    IN  CPSS_UNI_EV_CAUSE_ENT       cpssUniEventArr[],
    IN  GT_U32                      arrLength
)
{
    GT_STATUS rc = GT_OK;
#ifdef CPSS_APP_PLATFORM_REFERENCE
    if (cpssAppPlatformInitialized == GT_TRUE) {
        rc = appRefEventsToTestsHandlerUnbind(cpssUniEventArr,arrLength);
    }
#endif /*CPSS_APP_PLATFORM_REFERENCE*/

#if !defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)
    if (cpssAppPlatformInitialized == GT_FALSE) {
        rc = appDemoEventsToTestsHandlerUnbind(cpssUniEventArr,arrLength);
    }
#endif /*!defined(CPSS_APP_PLATFORM_REFERENCE) || defined(MIXED_MODE)*/
    return rc;
}

#endif
