/*******************************************************************************
*                Copyright 2001, Marvell International Ltd.
* This code contains confidential information of Marvell semiconductor, inc.
* no rights are granted herein under any patent, mask work right or copyright
* of Marvell or any third party.
* Marvell reserves the right at its sole discretion to request that this code
* be immediately returned to Marvell. This code is provided "as is".
* Marvell makes no warranties, express, implied or otherwise, regarding its
* accuracy, completeness or performance.
********************************************************************************
*/
/**
********************************************************************************
* @file RxTxProcess.c
*
* @brief This file includes the RxTx process code for multi process appDemo.
*
* @version   25
********************************************************************************
*/
#include <RxTxProcess.h>
#include <gtOs/gtOsSharedUtil.h>
#include <gtOs/gtOsSharedIPC.h>
#include <gtOs/gtOsSharedData.h>
#include <gtOs/gtOsSharedMalloc.h>
#include <gtOs/gtOsInit.h>
#include <string.h>
#include <extUtils/rxEventHandler/rxEventHandler.h>
#include <gtExtDrv/drivers/gtDmaDrv.h>



#if defined CHX_FAMILY
#include <cpss/dxCh/dxChxGen/networkIf/cpssDxChNetIf.h>
#endif

#if 0
#ifdef APP_DEMO_CPU_ETH_PORT_MANAGE
    #include <appDemo/eventEmulate/gtAppDemoEthPrtCtrl.h>
#endif

#include <appDemo/eventEmulate/gtAppDemoEmulationEvReqDispatcher.h>
#endif

#include <gtOs/gtOsSharedFunctionPointers.h>
#include <gtOs/gtOsSharedUtil.h>
#include <gtOs/globalDb/gtOsGlobalDb.h>

#ifdef HANDLE
#undef HANDLE
#endif

#include <gtOs/gtOsSharedPp.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDb.h>
#include <cpss/common/private/globalShared/prvCpssGlobalDbInterface.h>
#include <gtOs/globalDb/gtOsGlobalDbInterface.h>


#ifdef SHARED_MEMORY
#ifdef ASIC_SIMULATION

#include<gtOs/gtOsMem.h>
#include<gtOs/gtOsIo.h>
#include <gtStack/gtOsSocket.h>


#include <os/simTypesBind.h>
#define EXPLICIT_INCLUDE_TO_SIM_OS_H_FILES
#include <os/simOsBindOwn.h>
#undef EXPLICIT_INCLUDE_TO_SIM_OS_H_FILES

#include <asicSimulation/SInit/sinit.h>

#include <errno.h>

GT_STATUS osTimerWkAfter(    IN GT_U32 mils);

GT_STATUS osWrapperOpen(void * osSpec);


GT_STATUS  simulationDistributedClientSet
(
    GT_U32 index,
    GT_U32 tcpPortSync,
    GT_U32 tcpPortAsync
);

/* commandLine should belong to this module to make mainOs module workable
 * without CPSS Enabler into another third-party client. */
char  commandLine[1280] ;

char ** cmdArgv;
char ** cmdEnv;
/* commandLine should belong to simulation (simulation can be used by another client withot this var) */
extern char commandLine[1280];

#endif
#endif


#define BUFF_LEN    5

static GT_UINTPTR rxEventHanlerHnd = 0;
FILE * fp = NULL;
#define RX_TX_OUTPUT_FILE_NAME  "rxTxOut.txt"


GT_STATUS rxTxProcessBindFuncCallbacks
(
    GT_VOID
);


/**************** static func declaration *****************************/

static GT_STATUS prvUniEvMaskAllSet
(
    IN  CPSS_UNI_EV_CAUSE_ENT       cpssUniEventArr[],
    IN  GT_U32                      arrLength,
    IN  CPSS_EVENT_MASK_SET_ENT     operation
);
static GT_STATUS rxTxEnRxPacketGet
(
    IN RX_EV_HANDLER_EV_TYPE_ENT evType,
    IN GT_U8                devNum,
    IN GT_U8                queue
);
static GT_STATUS rxTxDxChNetRxPktHandle
(
    IN RX_EV_HANDLER_EV_TYPE_ENT evType,
    IN GT_U8                                devNum,
    IN GT_U8                                queueIdx
);
static void showDxChRxPktReceive
(
    IN GT_U8                                devNum,
    IN GT_U8                                queueIdx,
    IN GT_U32                               numOfBuff,
    IN GT_U8                               **packetBuffs,
    IN GT_U32                              *buffLenArr,
    IN CPSS_DXCH_NET_RX_PARAMS_STC         *rxParamsPtr
);

static GT_STATUS rxTxEnPpEvTreat
(
    GT_U8                   devNum,
    GT_U32                  uniEv,
    GT_U32                  evExtData
);

/* Tx */
static GT_STATUS startPacketTransmission(void);

static unsigned __TASKCONV rxTxEventsHndlr(GT_VOID * param);
static unsigned __TASKCONV rxTxCmdTask(GT_VOID * param);

static  int showRxPackets = 0;
static  int dumpToFile = 0;

#define PKTDUMP_MAX_BYTES       64
#define PKTDUMP_BYTES_PER_LINE  16


#ifdef APP_DEMO_CPU_ETH_PORT_MANAGE
static GT_STATUS ethPrtInitTx
(
    IN GT_U32       txHdrBufSize,
    IN GT_U32     * txHdrBufBlock_PTR,
    IN GT_U32       txHdrBufBlockSize
);
static GT_STATUS rxTx_appDemoEthPortTxEndGet
(
    IN GT_U32               hndl,
    OUT GT_U8               *devPtr,
    OUT GT_PTR              *cookiePtr,
    OUT GT_U8               *queuePtr,
    OUT GT_STATUS           *statusPtr
);
#endif /* APP_DEMO_CPU_ETH_PORT_MANAGE */




APP_DEMO_SYS_CONFIG_STC appDemoSysConfig = {
    GT_FALSE,   /* GT_BOOL cpuEtherPortUsed; */
    {
        0,    /* GT_U8 devNum; */
        {
            0, /* CPSS_RX_BUFF_ALLOC_METHOD_ENT   allocMethod; */
            {0}, /* GT_U32                  bufferPercentage[CPSS_MAX_RX_QUEUE_CNS]; */
            0, /* GT_U32                  rxBufSize; */
            0, /* GT_U32                  headerOffset; */
            0, /* GT_BOOL                 buffersInCachedMem; */
            {
                {
                    0, /* GT_U32  *rxBufBlockPtr; */
                    0, /* GT_U32  rxBufBlockSize; */
                } /* staticAlloc; */
            } /* buffData;*/
        },    /* CPSS_RX_BUF_INFO_STC rxBufInfo; */
        NULL, /* GT_U32 *txHdrBufBlock_PTR; */
        0,    /* GT_U32 txHdrBufBlockSize; */
        NULL, /* APP_DEMO_ETH_PRT_INIT_NETIF_FUNC initFunc; */
    }, /* APP_DEMO_CPU_ETHER_PORT_INFO_STC cpuEtherInfo; */
    0,        /* GT_U8 firstDevNum; */
    GT_FALSE, /* GT_BOOL supportAaMessage; */
    GT_FALSE, /* GT_BOOL forceAutoLearn; */
    0, /*GT_U32 appDemoActiveDeviceBmp*/
    GT_FALSE, /* mixedSip5AndSip6System */
    {0}/*resilientInitDevErrorArr*/
};



#ifdef APP_DEMO_CPU_ETH_PORT_MANAGE
extern APP_DEMO_ETH_PORT_TX_CTRL    appDemoEthPrtTxCtrl;
extern APP_DEMO_ETH_PORT_RX_Q_CTRL  appDemoEthPrtRxQCtrl;
extern APP_DEMO_ETH_PORT_CTRL_VALID appDemoEthPrtCtrl;



static GT_STATUS rxTx_appDemoEthPrtTransmitPacket
(
    IN APP_DEMO_ETH_PORT_NET_TX_PARAMS_STC * pcktParams,
    IN GT_U8              * packetBuffs[],
    IN GT_U32               buffLen[],
    IN GT_U32               numOfBufs
);

GT_VOID * appDemoEthPortPoolGetBuf
(
    IN  GT_ETH_PORT_POOL_TYPE_ENT poolIdx
);

GT_STATUS appDemoEthPortPoolFreeBuf
(
    IN  GT_ETH_PORT_POOL_TYPE_ENT   poolIdx,
    IN  GT_VOID                   * buf_PTR
);

static GT_STATUS ethPrtTransmitPacket
(
    IN APP_DEMO_ETH_PORT_NET_TX_PARAMS_STC * pcktParams,
    IN GT_U8              * packetBuffs[],
    IN GT_U32               buffLen[],
    IN GT_U32               numOfBufs
);

static GT_STATUS ethPrtInitRx
(
    IN CPSS_RX_BUF_INFO_STC * rxBufInfo_PTR,
    IN GT_U32           rxBufSize,
    IN GT_U32           rxQNum,
    IN GT_U32           rxQbufPercentage[],
    IN GT_U32           headerOffset
);


GT_STATUS appDemoEthPortCreatePool
(
    IN      GT_ETH_PORT_POOL_TYPE_ENT   poolIdx,
    IN      GT_U32                      reqBufferSize,
    INOUT   GT_U32                    * numOfBuffers_PTR,
    IN      GT_U8                     * pool_PTR,
    IN      GT_U32                      poolSize,
    IN      APPDEMO_CPSS_RX_BUFF_MALLOC_FUNC  mallocFunc
);


/* statistical counters for Ethernet port debugging */
static APP_DEMO_ETH_PORT_STAT ethPortDbg[NUM_OF_INT_QUEUES] = {{0}};

static GT_ETH_PORT_POOL_HEAD * poolArrPtr = NULL;

#endif /* APP_DEMO_CPU_ETH_PORT_MANAGE */




APP_DEMO_PP_CONFIG appDemoPpConfigList[APP_DEMO_PP_CONFIG_SIZE_CNS];
#define DXCH_NET_DSA_CMD_FROM_CPU_E CPSS_DXCH_NET_DSA_CMD_FROM_CPU_E
#define DXCH_NET_DSA_CMD_FORWARD_E CPSS_DXCH_NET_DSA_CMD_FORWARD_E


GT_BOOL multiProcessAppDemo = GT_TRUE;


GT_STATUS appDemoEvReqNodeGet_cpssEmulate
(
    IN GT_U8                   devNum,
    IN CPSS_UNI_EV_CAUSE_ENT         uniEvent,
    OUT GT_UINTPTR             *evHndlPtr
);




GT_STATUS appDemoGetTxDescSize
(
    IN  CPSS_PP_DEVICE_TYPE     devType,
    OUT GT_U32                  *descSize
);



/* by default the pointers set directly to the CPSS API */
/* but on some systems we need the dispatcher to handle */


#if 0
APP_DEMO_CPSS_EVENT_BIND_FUNC            rxTxCpssEventBind = cpssEventBind;
APP_DEMO_CPSS_EVENT_SELECT_FUNC          rxTxCpssEventSelect = cpssEventSelect;
APP_DEMO_CPSS_EVENT_RECV_FUNC            rxTxCpssEventRecv = cpssEventRecv;
APP_DEMO_CPSS_EVENT_DEVICE_MASK_SET_FUNC rxTxCpssEventDeviceMaskSet = cpssEventDeviceMaskSet;
#endif
#define rxTxCpssEventBind           cpssEventBind
#define rxTxCpssEventSelect         cpssEventSelect
#define rxTxCpssEventRecv           cpssEventRecv
#define rxTxCpssEventDeviceMaskSet  cpssEventDeviceMaskSet

#if defined ASIC_SIMULATION

GT_STATUS   osShellExecute
(
    IN  char*   command
)
{
    command = command;
    return GT_NOT_IMPLEMENTED;
}

#endif



#ifdef SHARED_MEMORY
#ifdef ASIC_SIMULATION

#include<gtOs/gtOsMem.h>
#include<gtOs/gtOsIo.h>
#include <gtStack/gtOsSocket.h>


#include <os/simTypesBind.h>
#define EXPLICIT_INCLUDE_TO_SIM_OS_H_FILES
#include <os/simOsBindOwn.h>
#undef EXPLICIT_INCLUDE_TO_SIM_OS_H_FILES

#include <asicSimulation/SInit/sinit.h>

#include <errno.h>


GT_STATUS osTimerWkAfter(    IN GT_U32 mils);

GT_STATUS osWrapperOpen(void * osSpec);


/* commandLine should belong to this module to make mainOs module workable
 * without CPSS Enabler into another third-party client. */
char  commandLine[1280] ;

char ** cmdArgv;
char ** cmdEnv;
/* commandLine should belong to simulation (simulation can be used by another client withot this var) */
extern char commandLine[1280];


#endif
#endif


/**
* @internal rxTxEnPpEvTreat function
* @endinternal
*
* @brief   This routine handles PP events.
*
* @param[in] devNum                   - the device number.
* @param[in] uniEv                    - Unified event number
* @param[in] evExtData                - Unified event additional information
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS rxTxEnPpEvTreat
(
    GT_U8                   devNum,
    GT_U32                  uniEv,
    GT_U32                  evExtData
)
{
        GT_STATUS   rc = GT_OK;
        GT_U8       queue;

        GT_UNUSED_PARAM(evExtData);


        /* Event handler */
        switch(uniEv)
        {
        case CPSS_PP_RX_BUFFER_QUEUE0_E:
        case CPSS_PP_RX_BUFFER_QUEUE1_E:
        case CPSS_PP_RX_BUFFER_QUEUE2_E:
        case CPSS_PP_RX_BUFFER_QUEUE3_E:
        case CPSS_PP_RX_BUFFER_QUEUE4_E:
        case CPSS_PP_RX_BUFFER_QUEUE5_E:
        case CPSS_PP_RX_BUFFER_QUEUE6_E:
        case CPSS_PP_RX_BUFFER_QUEUE7_E:


        queue = (GT_U8)(uniEv - CPSS_PP_RX_BUFFER_QUEUE0_E);
        /* implemented if defined DXCH_CODE */
        rc = rxTxEnRxPacketGet(RX_EV_HANDLER_TYPE_RX_E, devNum, queue);
        break;

    case CPSS_PP_RX_ERR_QUEUE0_E:
    case CPSS_PP_RX_ERR_QUEUE1_E:
    case CPSS_PP_RX_ERR_QUEUE2_E:
    case CPSS_PP_RX_ERR_QUEUE3_E:
    case CPSS_PP_RX_ERR_QUEUE4_E:
    case CPSS_PP_RX_ERR_QUEUE5_E:
    case CPSS_PP_RX_ERR_QUEUE6_E:
    case CPSS_PP_RX_ERR_QUEUE7_E:
        queue = (GT_U8)(uniEv - CPSS_PP_RX_ERR_QUEUE0_E);
        /* implemented if defined DXCH_CODE  */
        rc = rxTxEnRxPacketGet(RX_EV_HANDLER_TYPE_RX_ERR_E, devNum, queue);
                break;

        default:
                break;
        }
        return rc;
}

/**
* @internal rxTxEventRequestDrvnModeInit function
* @endinternal
*
* @brief   This routine spawns the App Demo event handlers.
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
GT_STATUS rxTxEventRequestDrvnModeInit
(
    IN GT_VOID
)
{
        GT_STATUS           rc;                                 /* The returned code            */
        GT_TASK             eventHandlerTid; /* The task Tid                 */
        GT_U32                  i = 1;                                 /* Iterator                     */
        GT_TASK         txPacketTransmitTid; /* The task Tid                 */

        GT_U32 evHndlrCauseArrSize = sizeof(evHndlrCauseArr)/sizeof(evHndlrCauseArr[0]);
        static EV_HNDLR_PARAM taskParamArr;


        if(GT_TRUE== cpssNonSharedGlobalVars.nonVolatileDb.generalVars.aslrSupport)
        {
            if(rxTxProcessBindFuncCallbacks() != GT_OK)
            {
                    osPrintf("rxTxProcessBindFuncCallbacks() failed\n");
                    return -1;
            }
        }


        /* bind the events for all the App-Demo event handlers */
        taskParamArr.hndlrIndex = i;

#ifndef APP_DEMO_CPU_ETH_PORT_MANAGE
        /*in case event allready bound - unbind them */
        rc = cpssEventUnBind(evHndlrCauseArr,evHndlrCauseArrSize);
        if(rc != GT_OK)
        {
          return rc;
        }
#endif
        /* call CPSS to bind the events under single handler */
        rc = rxTxCpssEventBind(evHndlrCauseArr,
                                           evHndlrCauseArrSize,
                                           &taskParamArr.evHndl);
        switch(rc)
        {
        case GT_FAIL:
                printf("General failure\n");
                break;
        case GT_BAD_PTR:
                printf("One of the parameters is NULL pointer\n");
                break;
        case GT_OUT_OF_CPU_MEM:
                printf("Failed to allocate CPU memory\n");
                break;
        case GT_FULL:
                printf("When trying to set the 'tx buffer queue unify event' \
(CPSS_PP_TX_BUFFER_QUEUE_E) with other events in the same handler\n");
                break;
        case GT_ALREADY_EXIST:
                printf("One of the unified events already bound to another \
handler (not applicable to CPSS_PP_TX_BUFFER_QUEUE_E)\n");
                break;
        }

        if(GT_OK != rc)
        {
                return GT_FAIL;
        }

        /* call the CPSS to enable those interrupts in the HW of the device */
        rc = prvUniEvMaskAllSet(evHndlrCauseArr,
                                                        evHndlrCauseArrSize,
                                                        CPSS_EVENT_UNMASK_E);
        if(GT_OK != rc)
        {
                return GT_FAIL;
        }

    rxEventHandlerLibInit();
    rxEventHandlerInitHandler(
            RX_EV_HANDLER_DEVNUM_ALL,
            RX_EV_HANDLER_QUEUE_ALL,
            RX_EV_HANDLER_TYPE_RX_E,
            &rxEventHanlerHnd);

/* spawn all the event handler processes */
        osPrintf("Init done.Create rxTxEventsHndlr.\n");

        rc = osTaskCreate("rxTxHndl",
                                          EV_HANDLER_MAX_PRIO,
                                          _32KB,
                                          rxTxEventsHndlr,
                                          &taskParamArr,
                                          &eventHandlerTid);
        if(rc != GT_OK)
        {
                return GT_FAIL;
        }

        osPrintf("Create rxTxCmdTask.\n");

        rc = osTaskCreate("rxTxCmdTask",
                                          EV_HANDLER_MAX_PRIO,
                                          _32KB,
                                          rxTxCmdTask,
                                          &taskParamArr,
                                          &txPacketTransmitTid);
        if(rc != GT_OK)
        {
                return GT_FAIL;
        }


        return GT_OK;
}

/**
* @internal prvUniEvMaskAllSet function
* @endinternal
*
* @brief   This routine unmasks all the events according to the unified event list.
*
* @param[in] cpssUniEventArr[]        - The CPSS unified event list.
* @param[in] arrLength                - The unified event list length.
* @param[in] operation                - type of  mask/unmask to do on the events
*                                       GT_OK on success, or
*                                       GT_FAIL if failed.
*/
static GT_STATUS prvUniEvMaskAllSet
(
    IN  CPSS_UNI_EV_CAUSE_ENT       cpssUniEventArr[],
    IN  GT_U32                      arrLength,
    IN  CPSS_EVENT_MASK_SET_ENT     operation
)
{
        GT_STATUS rc = GT_OK;
        GT_U32    i;                          /* Iterator                     */
        GT_U8     dev;                          /* Device iterator              */
/*        GT_32     intKey;        */          /* The interrupt lock key       */

        /* lock section to disable interruption of ISR while unmasking events */
/*        intKey = 0;
    osTaskLock();
        extDrvSetIntLockUnlock(INTR_MODE_LOCK, &intKey); */

        /* unmask the interrupt */
        for(i = 0; i < arrLength; i++)
        {
                if(cpssUniEventArr[i] <= CPSS_PP_UNI_EV_MAX_E)
                {
                        /* unmask the interrupt for all PPs */
                        for(dev = 0; dev < _PRV_CPSS_MAX_PP_DEVICES_CNS; dev++)
                        {
/*                if(appDemoPpConfigList[dev].valid == GT_FALSE)
                                {
                                        continue;
                                } */
                                rc = rxTxCpssEventDeviceMaskSet(/*appDemoPpConfigList[dev].devNum*/ dev, cpssUniEventArr[i], operation);
                                if(rc != GT_OK)
                                {
                                        goto exit_cleanly_lbl;
                                }
                        }
                }
                else
                {
                        rc = GT_FAIL;
                        goto exit_cleanly_lbl;
                }
        }

        exit_cleanly_lbl:
/*    extDrvSetIntLockUnlock(INTR_MODE_UNLOCK, &intKey);
        osTaskUnLock();*/

        return rc;
} /* prvUniEvMaskAllSet */

/*******************************************************************************
* appDemoEvHndlr
*
* DESCRIPTION:
*       This routine is the event handler for PSS Event-Request-Driven mode
*       (polling mode).
*
* INPUTS:
*       param - The process data structure.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success,
*       GT_FAIL - otherwise.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
static unsigned __TASKCONV rxTxEventsHndlr
(
GT_VOID * param
)
{
        GT_STATUS       rc;                                        /* return code         */
        GT_U32          i;                                                /* iterator            */
        GT_UINTPTR      evHndl;                                /* event handler       */
        GT_U32          evBitmapArr[CPSS_UNI_EV_BITMAP_SIZE_CNS];        /* event bitmap array  */
        GT_U32          evBitmap;                                                                        /* event bitmap 32 bit */
        GT_U32          evExtData;                                                                        /* event extended data */
        GT_U8           devNum;                                                                                /* device number       */
        GT_U32          uniEv;                                                                                /* unified event cause */
        GT_U32          evCauseIdx;                                                                        /* event index         */
        EV_HNDLR_PARAM      *hndlrParamPtr;                                                                /* bind event array    */

        hndlrParamPtr = (EV_HNDLR_PARAM*)param;
        evHndl        = hndlrParamPtr->evHndl;

        while(1)
        {
                rc = rxTxCpssEventSelect(evHndl, NULL, evBitmapArr, (GT_U32)CPSS_UNI_EV_BITMAP_SIZE_CNS);
                if(GT_OK != rc)
                {
                        /* If seems like this result is not unusual... */
                        /* DBG_LOG(("rxTxCpssEventSelect: err\n", 1, 2, 3, 4, 5, 6));*/
                        continue;
                }

                for(evCauseIdx = 0; evCauseIdx < CPSS_UNI_EV_BITMAP_SIZE_CNS; evCauseIdx++)
                {
                        if(evBitmapArr[evCauseIdx] == 0)
                        {
                                continue;
                        }

                        evBitmap = evBitmapArr[evCauseIdx];

                        for(i = 0; evBitmap; evBitmap >>= 1, i++)
                        {
                                if((evBitmap & 1) == 0)
                                {
                                        continue;
                                }
                                uniEv = (evCauseIdx << 5) + i;

                                if((rc=rxTxCpssEventRecv(evHndl, uniEv, &evExtData, &devNum)) == GT_OK)
                                {
                                        DBG_LOG(("cpssEventRecv: %08x <dev=%d, uniEv=%d(%s), extData=0x%0x>\n",
                                                         (GT_U32)hndlrParamPtr->hndlrIndex, devNum, uniEv,
                                                         (GT_U32)uniEvName[uniEv], evExtData, 6));


                                        /* Treat packet transmit */
                                        rc = rxTxEnPpEvTreat(devNum, uniEv, evExtData);

                                } else
                                {
                                        DBG_LOG(("cpssEventRecv: error %d\n", rc, 2, 3, 4, 5, 6));
                                }
                        }
                }
        }
    return 0;
}


/**
* @internal rxTxEnRxPacketGet function
* @endinternal
*
* @brief   This routine handles the packet Rx event.
*
* @param[in] devNum                   - the device number.
* @param[in] queue                    - the  the rx event occurred upon
*
* @retval GT_OK                    - on success,
* @retval GT_FAIL                  - otherwise.
*/
static GT_STATUS rxTxEnRxPacketGet
(
    IN RX_EV_HANDLER_EV_TYPE_ENT evType,
    IN GT_U8                devNum,
    IN GT_U8                queue
)
{
        GT_STATUS       rc = GT_NOT_IMPLEMENTED;

        do
        {

                /*new packet treatment*/
        rc = rxTxDxChNetRxPktHandle(evType,devNum, queue);

        }while(rc == GT_OK);

        return rc;
}


/**
* @internal rxTxDxChNetRxPktHandle function
* @endinternal
*
* @brief   application routine to receive frames . -- DXCH function
*
* @param[in] devNum                   - Device number.
* @param[in] queueIdx                 - The queue from which this packet was received.
*                                       None.
*
* @note The user is responsible to free the gtBuf structures and rx data buffers.
*       The lport that needs to be transferred to the function netFreeRxGtbuf()
*       is the intLport and not srcLport.
*
*/

static GT_STATUS rxTxDxChNetRxPktHandle
(
    IN RX_EV_HANDLER_EV_TYPE_ENT evType,
    IN GT_U8                                devNum,
    IN GT_U8                                queueIdx
)
{
        GT_STATUS                           rc;
        GT_U32                              numOfBuff = BUFF_LEN;
    GT_U8*                              packetBuffs[BUFF_LEN];
    GT_U32                              buffLenArr[BUFF_LEN];
    CPSS_DXCH_NET_RX_PARAMS_STC         rxParams;

        /* get the packet from the device */
        if(appDemoPpConfigList[devNum].channel == CPSS_CHANNEL_PCI_E)
        {
                rc = cpssDxChNetIfSdmaRxPacketGet(devNum, queueIdx,&numOfBuff,
                            packetBuffs,buffLenArr,&rxParams);
        }
        else
        {
#ifdef APP_DEMO_CPU_ETH_PORT_MANAGE
        CPSS_TBD_BOOKMARK
        /* Shared Memory implementation should be updated to use native MII APIs. */
                APP_DEMO_ETH_PORT_NET_RX_PARAMS_STC appDemoRxParams;

                rc = appDemoEthPrtEthPrtRxPacketGet(queueIdx,&numOfBuff,
                                                        packetBuffs,buffLenArr,&appDemoRxParams);

        osMemCpy(&rxParams,&appDemoRxParams.dsa.dxChDsaParam,
                                 sizeof(CPSS_DXCH_NET_RX_PARAMS_STC));
#else  /*APP_DEMO_CPU_ETH_PORT_MANAGE*/
                rc = GT_NOT_IMPLEMENTED;
#endif /*APP_DEMO_CPU_ETH_PORT_MANAGE*/
        }

        if (rc != GT_OK)  /* GT_NO_MORE is good and not an error !!! */
        {
                /* no need to free buffers because not got any */
                return rc;
        }

    if (showRxPackets)
    {
        showDxChRxPktReceive(devNum,queueIdx,numOfBuff,
                                        packetBuffs,buffLenArr,&rxParams);
    }

    rxEventHandlerDoCallbacks(rxEventHanlerHnd, evType,
            devNum,queueIdx,numOfBuff, packetBuffs,buffLenArr,&rxParams);

        if(appDemoPpConfigList[devNum].channel == CPSS_CHANNEL_PCI_E)
        {
                /* now you need to free the buffers */
                rc = cpssDxChNetIfRxBufFree(devNum,queueIdx,packetBuffs,numOfBuff);
        }
        else
        {
#ifdef APP_DEMO_CPU_ETH_PORT_MANAGE
        CPSS_TBD_BOOKMARK
        /* Shared Memory implementation should be updated to use native MII APIs. */
                rc = appDemoEthPrtRxBufFree(packetBuffs,numOfBuff,queueIdx);
#endif /*APP_DEMO_CPU_ETH_PORT_MANAGE*/
        }

        return rc;
}


/**
* @internal showDxChRxPktReceive function
* @endinternal
*
* @brief   Show received packet function. Enabled by command line
*
* @param[in] devNum                   - The device number in which the packet was received.
*                                      queue        - The Rx queue in which the packet was received.
* @param[in] numOfBuff                - Num of used buffs in packetBuffs
* @param[in] packetBuffs              - The received packet buffers list
* @param[in] buffLenArr               - List of buffer lengths for packetBuffs
* @param[in] rxParamsPtr              - Rx info
*                                       None
*/
static void showDxChRxPktReceive
(
    IN GT_U8                                devNum,
    IN GT_U8                                queueIdx,
    IN GT_U32                               numOfBuff,
    IN GT_U8                               **packetBuffs,
    IN GT_U32                              *buffLenArr,
    IN CPSS_DXCH_NET_RX_PARAMS_STC         *rxParamsPtr
)
{
    int bytesShown = 0;
    unsigned int i;


    if(dumpToFile)
    {
        fp = fopen(RX_TX_OUTPUT_FILE_NAME,"a");
        if(fp==NULL)
        {
            printf(" failed to open file %s.Output to stdout.\n",RX_TX_OUTPUT_FILE_NAME);
            fp=stdout;
        }
        else
        {
            printf("showDxChRxPktReceive from queue %d > %s", queueIdx, RX_TX_OUTPUT_FILE_NAME);
        }
    }
    else
    {
        fp=stdout;
    }

    fprintf(fp,"RX dev=%d queue=%d vid=%d",devNum, queueIdx,
            rxParamsPtr->dsaParam.commonParams.vid);
    if (rxParamsPtr->dsaParam.dsaType == CPSS_DXCH_NET_DSA_CMD_TO_CPU_E)
    {
        CPSS_DXCH_NET_DSA_TO_CPU_STC *toCpu = &(rxParamsPtr->dsaParam.dsaInfo.toCpu);

        fprintf(fp," hwDev=%d tagged=%d", toCpu->hwDevNum, toCpu->isTagged);
        if (toCpu->srcIsTrunk == GT_TRUE)
        {
            printf(" srcTrunk=%d",toCpu->interface.srcTrunkId);
        }
        else
        {
           fprintf(fp," portNum=%d ePort=%d",
                    toCpu->interface.portNum,
                    toCpu->interface.ePort);
        }
        printf(" cpuCode=%d",toCpu->cpuCode);
    }
    /* TODO: other DSA type */
    fprintf(fp,"\n");

    for (;numOfBuff && bytesShown < PKTDUMP_MAX_BYTES; numOfBuff--,packetBuffs++,buffLenArr++)
    {
        for (i = 0; i < *buffLenArr && bytesShown < PKTDUMP_MAX_BYTES; i++, bytesShown++)
        {
            if ((bytesShown % PKTDUMP_BYTES_PER_LINE) == 0 && bytesShown != 0)
                fprintf(fp,"\n");
            fprintf(fp," %02x",(*packetBuffs)[i]);
        }
    }
    if (bytesShown != 0)
        fprintf(fp,"\n");
}


/************************                TX                   ***************************/
/*
static CPSS_DXCH_TRANSMIT_MP_MODE_PARAMS_STC txTransmitParams;
*/
static GT_U32 txDxChTblCapacity = 0;
static DXCH_PKT_DESC_STC* cpssDxChTxPacketDescTbl = NULL;

static CPSS_TX_MODE_SETITNGS_STC modeSettings = {1, 1};
static GT_BOOL flagStopTransmit = GT_FALSE;

static GT_UINTPTR evReqHndl = 0;
static GT_U8 txCookie = 0x10;

/* Transmit task ID */
static GT_TASK taskId = 0;
static GT_SEM  txCmdSemId;                      /* Module semaphore id  */
static GT_BOOL taskBusyFlag = GT_FALSE;

static GT_STATUS __TASKCONV cpssDxChPacketTransminitionTask()
{
/*    GT_STATUS rc = GT_OK; */

    /* while forever */
    while(1)
    {
        /* wait on the TX command semaphore */
        taskBusyFlag = GT_FALSE;
        osSemWait(txCmdSemId, OS_WAIT_FOREVER);
        taskBusyFlag = GT_TRUE;

            /* rc = */ startPacketTransmission();
    }
    return 0;
}

/**
* @internal rxTxCpssDxChTxStart function
* @endinternal
*
* @brief   Starts transmition of packets
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
static GT_STATUS rxTxCpssDxChTxStart
(
)
{
    GT_STATUS rc = GT_OK;
    static GT_BOOL   enterOnce = GT_FALSE;

    /* check if there are element in the transmit table */
    if(txDxChTblCapacity == 0)
        return GT_OK;

    /* creat the task only once */
    if(enterOnce == GT_FALSE)
    {

        if(osSemBinCreate("RxTxTransmit",OS_SEMB_EMPTY,&txCmdSemId) != GT_OK)
        {
            return GT_FAIL;
        }
        if(osTaskCreate("RxTxPktTx",                          /* Task Name                      */
                        EV_HANDLER_MAX_PRIO,                  /* Task Priority                  */
                        0xa000,                               /* Stack Size _40KB               */
                        (unsigned (__TASKCONV *)(void*))cpssDxChPacketTransminitionTask, /* Starting Point */
                        (GT_VOID*)NULL,                       /* there is no arguments */
                        &taskId) != GT_OK)                    /* returned task ID */
            return GT_FAIL;
        enterOnce = GT_TRUE;
    }

    /* check if the last transmition is done */
    if(taskBusyFlag == GT_TRUE)
        return GT_FAIL;

    flagStopTransmit = GT_FALSE;

    /* send a signal for the task to start the transmission */
    osSemSignal(txCmdSemId);

    return  rc;
}

/**
* @internal rxTxCpssDxChTxStop function
* @endinternal
*
* @brief   Stop transmition of packets.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*/
static GT_STATUS rxTxCpssDxChTxStop
(
    void
)
{
    /* there is no send task running */
    if(taskId == 0)
    {
        return GT_NO_CHANGE;
    }

    flagStopTransmit = GT_TRUE;
    return GT_OK;
}

/*******************************************************************************
* rxTxCmdTask
*
* DESCRIPTION:
*
*
* INPUTS:
*       param - The process data structure.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       GT_OK   - on success,
*       GT_FAIL - otherwise.
*
* COMMENTS:
*       None.
*
*******************************************************************************/
static unsigned __TASKCONV rxTxCmdTask
(
GT_VOID * param
)
{
        GT_STATUS           rc;                                                                                        /* return code         */
    CPSS_MP_REMOTE_COMMAND cmd;

    GT_UNUSED_PARAM(param);

        while(cpssMultiProcComWait(CPSS_MP_REMOTE_RXTX_E, &cmd) == GT_OK)
        {
        switch (cmd.command)
        {
            case CPSS_MP_CMD_RXTX_RXPKT_CLEAR_TBL_E:
                /* do nothing */
                cpssMultiProcComComplete(&cmd, GT_OK);
                break;
            case CPSS_MP_CMD_RXTX_START_TRANSMIT_E:
                cpssDxChTxPacketDescTbl = cmd.data.rxTxStartTransmit.cpssDxChTxPacketDescTbl;
                txDxChTblCapacity = cmd.data.rxTxStartTransmit.txDxChTblCapacity;
                modeSettings.cyclesNum = cmd.data.rxTxStartTransmit.modeSettings.cyclesNum;
                modeSettings.gap = cmd.data.rxTxStartTransmit.modeSettings.gap;
#ifdef APP_DEMO_CPU_ETH_PORT_MANAGE
                poolArrPtr = (GT_ETH_PORT_POOL_HEAD *)cmd.data.rxTxStartTransmit.txPoolPtr;
#endif
                printf("TX_PROCESS_START : DescTbl - %p  cyc - %d gap - %d cap - %d\n",
                       cpssDxChTxPacketDescTbl,
                       modeSettings.cyclesNum,
                       modeSettings.gap,
                       txDxChTblCapacity);
                rc = rxTxCpssDxChTxStart();
                cpssMultiProcComComplete(&cmd, rc);
                break;
            case CPSS_MP_CMD_RXTX_STOP_TRANSMIT_E:
                rc = rxTxCpssDxChTxStop();
                cpssMultiProcComComplete(&cmd, rc);
                break;
            default:
                cpssMultiProcComComplete(&cmd, GT_FAIL);

        }
        }
        return GT_OK;
}

/**
* @internal startPacketTransmission function
* @endinternal
*
* @brief   Thi is thread function that performs trunsmition of packets defined
*         in the table.
*
* @retval GT_OK                    - on success
* @retval GT_FAIL                  - on error
*
* @note GalTis:
*       None
*
*/
static GT_STATUS startPacketTransmission(void)
{
        GT_U32      i;
        GT_U32      cycles, pcktNum;
        GT_STATUS   status = GT_OK;

        CPSS_DXCH_NET_TX_PARAMS_STC cpssPcktParams;
        GT_U8                       devNum=0;


        static GT_U32  sdmaTxPacketSendCount = 0;
        static GT_U32  sdmaTxPacketGetCount = 0;

        if (evReqHndl == 0)
        {
                CPSS_UNI_EV_CAUSE_ENT     evCause[1] = { CPSS_PP_TX_BUFFER_QUEUE_E };

                if (rxTxCpssEventBind(evCause, 1, &evReqHndl) != GT_OK)
                {
                        return GT_FAIL;
                }

                /* convert pss events to those of cpss */
                for(i = 0 ; i < 128 ; i++)
                {
                        /*extern CMD_PP_CONFIG *ppConfigList;
                        if(ppConfigList[i].valid == GT_FALSE)
                        {
                                continue;
                        }
                        */

                        /* unmask this event with all HW devices */
                        /*status = cpssEventDeviceMaskSet(ppConfigList[i].devNum,CPSS_PP_TX_BUFFER_QUEUE_E,CPSS_EVENT_UNMASK_E);*/
                        status = rxTxCpssEventDeviceMaskSet((GT_U8)i,CPSS_PP_TX_BUFFER_QUEUE_E,CPSS_EVENT_UNMASK_E);
                        if(status != GT_OK)
                        {
                                /* there are FA/XBAR events that not relevant to PP device,
                                vice versa */
                                continue;
                        }
                }

        }


        cycles = 0;
        pcktNum = 0;
        while (((cycles < modeSettings.cyclesNum) ||
                        (modeSettings.cyclesNum == 0xffffffff)) && (flagStopTransmit == GT_FALSE))
        {
                for(i = 0; i < txDxChTblCapacity; i++) /*For each descriptor*/
                {
                        GT_U32 j;

                        /* If the transmition was disabled */
                        if (flagStopTransmit == GT_TRUE)
                        {
                                break;
                        }


                        if (GT_TRUE != cpssDxChTxPacketDescTbl[i].valid)
                        {
                                /* entry is not valid */
                                continue;
                        }

                        for (j = 0; j < cpssDxChTxPacketDescTbl[i].pcktsNum; j++)
                        {/*For packet sent by this descriptor*/
                                /* If the transmition was disabled */
                                if (flagStopTransmit == GT_TRUE)
                                {
                                        break;
                                }

                                cpssPcktParams.packetIsTagged = cpssDxChTxPacketDescTbl[i].packetIsTagged;
                                cpssPcktParams.cookie = &txCookie;
                                cpssPcktParams.sdmaInfo.evReqHndl  = evReqHndl;
                                cpssPcktParams.sdmaInfo.recalcCrc = cpssDxChTxPacketDescTbl[i].sdmaInfo.recalcCrc;
                                cpssPcktParams.sdmaInfo.txQueue = cpssDxChTxPacketDescTbl[i].sdmaInfo.txQueue;
                                cpssPcktParams.sdmaInfo.invokeTxBufferQueueEvent = cpssDxChTxPacketDescTbl[i].sdmaInfo.invokeTxBufferQueueEvent;


                                cpssPcktParams.dsaParam.commonParams.dsaTagType = cpssDxChTxPacketDescTbl[i].dsaParam.commonParams.dsaTagType;
                                cpssPcktParams.dsaParam.commonParams.vpt = cpssDxChTxPacketDescTbl[i].dsaParam.commonParams.vpt;
                                cpssPcktParams.dsaParam.commonParams.cfiBit = cpssDxChTxPacketDescTbl[i].dsaParam.commonParams.cfiBit;
                                cpssPcktParams.dsaParam.commonParams.vid = cpssDxChTxPacketDescTbl[i].dsaParam.commonParams.vid;
                                cpssPcktParams.dsaParam.dsaType = cpssDxChTxPacketDescTbl[i].dsaParam.dsaType;

                                switch(cpssDxChTxPacketDescTbl[i].dsaParam.dsaType)
                                {
                                        case DXCH_NET_DSA_CMD_FROM_CPU_E:

                                        cpssPcktParams.dsaParam.dsaInfo.fromCpu.tc = cpssDxChTxPacketDescTbl[i].dsaParam.dsaInfo.fromCpu.tc;
                                        cpssPcktParams.dsaParam.dsaInfo.fromCpu.dp = cpssDxChTxPacketDescTbl[i].dsaParam.dsaInfo.fromCpu.dp;
                                        cpssPcktParams.dsaParam.dsaInfo.fromCpu.egrFilterEn = cpssDxChTxPacketDescTbl[i].dsaParam.dsaInfo.fromCpu.egrFilterEn;
                                        cpssPcktParams.dsaParam.dsaInfo.fromCpu.cascadeControl = cpssDxChTxPacketDescTbl[i].dsaParam.dsaInfo.fromCpu.cascadeControl;
                                        cpssPcktParams.dsaParam.dsaInfo.fromCpu.egrFilterRegistered = cpssDxChTxPacketDescTbl[i].dsaParam.dsaInfo.fromCpu.egrFilterRegistered;
                                        cpssPcktParams.dsaParam.dsaInfo.fromCpu.srcId = cpssDxChTxPacketDescTbl[i].dsaParam.dsaInfo.fromCpu.srcId;
                                        cpssPcktParams.dsaParam.dsaInfo.fromCpu.srcHwDev = cpssDxChTxPacketDescTbl[i].dsaParam.dsaInfo.fromCpu.srcHwDev;
                                        cpssPcktParams.dsaParam.dsaInfo.fromCpu.dstInterface.devPort.hwDevNum = cpssDxChTxPacketDescTbl[i].dsaParam.dsaInfo.fromCpu.dstInterface.devPort.hwDevNum;

                                        devNum = cpssPcktParams.dsaParam.dsaInfo.fromCpu.dstInterface.devPort.hwDevNum;


                                        if(cpssDxChTxPacketDescTbl[i].cmdType == DXCH_TX_BY_VIDX)
                                        {
                                                cpssPcktParams.dsaParam.dsaInfo.fromCpu.dstInterface.type = cpssDxChTxPacketDescTbl[i].dsaParam.dsaInfo.fromCpu.dstInterface.type;
                                                if(cpssPcktParams.dsaParam.dsaInfo.fromCpu.dstInterface.type == CPSS_INTERFACE_VIDX_E)
                                                {
                                                        cpssPcktParams.dsaParam.dsaInfo.fromCpu.dstInterface.vidx = cpssDxChTxPacketDescTbl[i].dsaParam.dsaInfo.fromCpu.dstInterface.vidx;
                                                }
                                                else /*CPSS_INTERFACE_VID_E*/
                                                {
                                                        cpssPcktParams.dsaParam.dsaInfo.fromCpu.dstInterface.vlanId = cpssDxChTxPacketDescTbl[i].dsaParam.dsaInfo.fromCpu.dstInterface.vlanId;
                                                }

                                                cpssPcktParams.dsaParam.dsaInfo.fromCpu.extDestInfo.multiDest.excludeInterface = cpssDxChTxPacketDescTbl[i].dsaParam.dsaInfo.fromCpu.extDestInfo.multiDest.excludeInterface;
                                                cpssPcktParams.dsaParam.dsaInfo.fromCpu.extDestInfo.multiDest.excludedInterface.type = cpssDxChTxPacketDescTbl[i].dsaParam.dsaInfo.fromCpu.extDestInfo.multiDest.excludedInterface.type;

                                                if(cpssPcktParams.dsaParam.dsaInfo.fromCpu.extDestInfo.multiDest.excludedInterface.type == 0) /*CPSS_INTERFACE_PORT_E*/
                                                {
                                                        cpssPcktParams.dsaParam.dsaInfo.fromCpu.extDestInfo.multiDest.excludedInterface.devPort.hwDevNum = cpssDxChTxPacketDescTbl[i].dsaParam.dsaInfo.fromCpu.extDestInfo.multiDest.excludedInterface.devPort.hwDevNum;
                                                        cpssPcktParams.dsaParam.dsaInfo.fromCpu.extDestInfo.multiDest.excludedInterface.devPort.portNum = cpssDxChTxPacketDescTbl[i].dsaParam.dsaInfo.fromCpu.extDestInfo.multiDest.excludedInterface.devPort.portNum;
                                                }
                                                else/*CPSS_INTERFACE_TRUNK_E*/
                                                {
                                                        cpssPcktParams.dsaParam.dsaInfo.fromCpu.extDestInfo.multiDest.excludedInterface.trunkId = cpssDxChTxPacketDescTbl[i].dsaParam.dsaInfo.fromCpu.extDestInfo.multiDest.excludedInterface.trunkId;
                                                }
                                        }
                                        else/*DXCH_TX_BY_PORT*/
                                        {
                                                cpssPcktParams.dsaParam.dsaInfo.fromCpu.dstInterface.type = cpssDxChTxPacketDescTbl[i].dsaParam.dsaInfo.fromCpu.dstInterface.type;
                                                cpssPcktParams.dsaParam.dsaInfo.fromCpu.dstInterface.devPort.hwDevNum = cpssDxChTxPacketDescTbl[i].dsaParam.dsaInfo.fromCpu.dstInterface.devPort.hwDevNum;
                                                cpssPcktParams.dsaParam.dsaInfo.fromCpu.dstInterface.devPort.portNum = cpssDxChTxPacketDescTbl[i].dsaParam.dsaInfo.fromCpu.dstInterface.devPort.portNum;

                                                cpssPcktParams.dsaParam.dsaInfo.fromCpu.extDestInfo.devPort.dstIsTagged = cpssDxChTxPacketDescTbl[i].dsaParam.dsaInfo.fromCpu.extDestInfo.devPort.dstIsTagged;
                                                cpssPcktParams.dsaParam.dsaInfo.fromCpu.extDestInfo.devPort.mailBoxToNeighborCPU = cpssDxChTxPacketDescTbl[i].dsaParam.dsaInfo.fromCpu.extDestInfo.devPort.mailBoxToNeighborCPU;

                                        }
                                        break;

                                case DXCH_NET_DSA_CMD_FORWARD_E:

                                        cpssPcktParams.dsaParam.dsaInfo.fromCpu.tc = cpssDxChTxPacketDescTbl[i].dsaParam.dsaInfo.fromCpu.tc;
                                        cpssPcktParams.dsaParam.dsaInfo.forward.srcIsTagged = cpssDxChTxPacketDescTbl[i].dsaParam.dsaInfo.forward.srcIsTagged;
                                        cpssPcktParams.dsaParam.dsaInfo.forward.srcHwDev = cpssDxChTxPacketDescTbl[i].dsaParam.dsaInfo.forward.srcHwDev;
                                        cpssPcktParams.dsaParam.dsaInfo.forward.srcIsTrunk = cpssDxChTxPacketDescTbl[i].dsaParam.dsaInfo.forward.srcIsTrunk;
                                        if(cpssPcktParams.dsaParam.dsaInfo.forward.srcIsTrunk == GT_TRUE)
                                                cpssPcktParams.dsaParam.dsaInfo.forward.source.trunkId = cpssDxChTxPacketDescTbl[i].dsaParam.dsaInfo.forward.source.trunkId;
                                        else
                                                cpssPcktParams.dsaParam.dsaInfo.forward.source.portNum = cpssDxChTxPacketDescTbl[i].dsaParam.dsaInfo.forward.source.portNum;

                                        cpssPcktParams.dsaParam.dsaInfo.forward.egrFilterRegistered = cpssDxChTxPacketDescTbl[i].dsaParam.dsaInfo.forward.egrFilterRegistered;
                                        cpssPcktParams.dsaParam.dsaInfo.forward.wasRouted = cpssDxChTxPacketDescTbl[i].dsaParam.dsaInfo.forward.wasRouted;
                                        cpssPcktParams.dsaParam.dsaInfo.forward.qosProfileIndex = cpssDxChTxPacketDescTbl[i].dsaParam.dsaInfo.forward.qosProfileIndex;

                                        cpssPcktParams.dsaParam.dsaInfo.forward.dstInterface.devPort.hwDevNum = cpssDxChTxPacketDescTbl[i].dsaParam.dsaInfo.forward.dstInterface.devPort.hwDevNum;
                                        devNum = cpssPcktParams.dsaParam.dsaInfo.forward.dstInterface.devPort.hwDevNum;

                                        if(cpssDxChTxPacketDescTbl[i].cmdType == DXCH_TX_BY_VIDX)
                                        {
                                                cpssPcktParams.dsaParam.dsaInfo.forward.dstInterface.type = cpssDxChTxPacketDescTbl[i].dsaParam.dsaInfo.forward.dstInterface.type;
                                                if(cpssPcktParams.dsaParam.dsaInfo.forward.dstInterface.type == CPSS_INTERFACE_VIDX_E)
                                                        cpssPcktParams.dsaParam.dsaInfo.forward.dstInterface.vidx = cpssDxChTxPacketDescTbl[i].dsaParam.dsaInfo.forward.dstInterface.vidx;
                                                else /* CPSS_INTERFACE_VID_E */
                                                        cpssPcktParams.dsaParam.dsaInfo.forward.dstInterface.vlanId = cpssDxChTxPacketDescTbl[i].dsaParam.dsaInfo.forward.dstInterface.vlanId;
                                        }
                                        else/*DXCH_TX_BY_PORT*/
                                        {
                                                cpssPcktParams.dsaParam.dsaInfo.forward.dstInterface.type = cpssDxChTxPacketDescTbl[i].dsaParam.dsaInfo.forward.dstInterface.type;
                                                cpssPcktParams.dsaParam.dsaInfo.forward.dstInterface.devPort.hwDevNum = cpssDxChTxPacketDescTbl[i].dsaParam.dsaInfo.forward.dstInterface.devPort.hwDevNum;
                                                cpssPcktParams.dsaParam.dsaInfo.forward.dstInterface.devPort.portNum = cpssDxChTxPacketDescTbl[i].dsaParam.dsaInfo.forward.dstInterface.devPort.portNum;
                                        }
                                        break;

                                default:
                                        return GT_BAD_PARAM;
                                }

                                if(appDemoSysConfig.cpuEtherPortUsed == GT_TRUE)
                                {
#ifdef APP_DEMO_CPU_ETH_PORT_MANAGE

                                         APP_DEMO_ETH_PORT_NET_TX_PARAMS_STC appDemoPcktParams;
                                         osMemCpy(&appDemoPcktParams.dsa.dxChDsaParam,
                                                          &cpssPcktParams.dsaParam,
                                                          sizeof(CPSS_DXCH_NET_DSA_PARAMS_STC));
                                         appDemoPcktParams.packetIsTagged = cpssPcktParams.packetIsTagged;
                                         appDemoPcktParams.cookie         = cpssPcktParams.cookie;
                                         appDemoPcktParams.recalcCrc      = cpssPcktParams.sdmaInfo.recalcCrc;
                                         appDemoPcktParams.txQueue        = cpssPcktParams.sdmaInfo.txQueue;
                                         appDemoPcktParams.evReqHndl      = cpssPcktParams.sdmaInfo.evReqHndl;

                                         status = rxTx_appDemoEthPrtTransmitPacket(&appDemoPcktParams,
                                                                                cpssDxChTxPacketDescTbl[i].pcktData,
                                                                                cpssDxChTxPacketDescTbl[i].pcktDataLen,
                                                                                cpssDxChTxPacketDescTbl[i].numOfBuffers);

#endif /*APP_DEMO_CPU_ETH_PORT_MANAGE*/
                                }
                                else
                                {

                                        if(cpssDxChTxPacketDescTbl[i].txSyncMode==GT_TRUE)
                                         {
                                                 status = cpssDxChNetIfSdmaSyncTxPacketSend(devNum, &cpssPcktParams, cpssDxChTxPacketDescTbl[i].pcktData,
                                                                                                                cpssDxChTxPacketDescTbl[i].pcktDataLen, cpssDxChTxPacketDescTbl[i].numOfBuffers);
                                         }
                                         else
                                         {
                                                 status = cpssDxChNetIfSdmaTxPacketSend(devNum, &cpssPcktParams, cpssDxChTxPacketDescTbl[i].pcktData,
                                                                                                                cpssDxChTxPacketDescTbl[i].pcktDataLen, cpssDxChTxPacketDescTbl[i].numOfBuffers);

                                                 sdmaTxPacketSendCount++;

                                         }
                                }

                                if((status != GT_OK) && (status != GT_NO_RESOURCE))
                                {
                                        return status;
                                }

                                /* if we get GT_NO_RESOURCE and the sdmaTxPacketSendCount
                                   is bigger then MAX_NUM_OF_BUFFERS_AT_POOL
                                   it means no more buffer available in the FIFO */
                                if ((((cpssDxChTxPacketDescTbl[i].txSyncMode==GT_FALSE) &&
                                        (cpssDxChTxPacketDescTbl[i].sdmaInfo.invokeTxBufferQueueEvent==GT_TRUE)) ||
                                        ((status == GT_NO_RESOURCE) && (sdmaTxPacketSendCount >= MAX_NUM_OF_BUFFERS_AT_POOL)))
                                        ||
                                        appDemoSysConfig.cpuEtherPortUsed == GT_TRUE)
                                {
                                        GT_U8           devNum;
                                        GT_U8           queueIdx;
                                        GT_STATUS       retVal;
                                        GT_PTR          cookie;

                                        /* wait for the Tx-End event */
                                        retVal = rxTxCpssEventSelect(evReqHndl,NULL,NULL,0);

                                        if (retVal != GT_OK)
                                        {
                                                return retVal;
                                        }

                                        /* get all Tx end events for the packet */

                                        if(appDemoSysConfig.cpuEtherPortUsed == GT_TRUE)
                                        {
#ifdef APP_DEMO_CPU_ETH_PORT_MANAGE
        CPSS_TBD_BOOKMARK
        /* Shared Memory implementation should be updated to use native MII APIs. */
                                                while(1)
                                                {

                                                        retVal = rxTx_appDemoEthPortTxEndGet(evReqHndl,&devNum,
                                                                                                                        &cookie,&queueIdx,&retVal);
                                                        if (retVal == GT_FAIL)
                                                        {
                                                                return retVal;
                                                        }
                                                        if(retVal == GT_NO_MORE)
                                                        {
                                                                break;
                                                        }
                                                }
#endif /*APP_DEMO_CPU_ETH_PORT_MANAGE*/

                                                }
                                        else
                                        {

                                                /* Nathan -
                                                   I don't think we need this here because the Galtis
                                                   application will not do anything with the returned values,
                                                   no buffers to free. */
                                                while(1)
                                                {
                                                        retVal = cpssDxChNetIfTxBufferQueueGet(evReqHndl,&devNum,
                                                                                                                        &cookie,&queueIdx,&retVal);
                                                        if (retVal == GT_FAIL)
                                                        {
                                                                return retVal;
                                                        }
                                                        if(retVal == GT_NO_MORE)
                                                        {
                                                                break;
                                                        }
                                                        sdmaTxPacketGetCount++;
                                                }
                                        }


                                        if(status != GT_NO_RESOURCE)
                                        {
                                                if(sdmaTxPacketSendCount == sdmaTxPacketGetCount)
                                                {
                                                        sdmaTxPacketSendCount = 0;
                                                        sdmaTxPacketGetCount = 0;
                                                }
                                                else
                                                {
                                                        osPrintf("Number of sent packets NOT equal to number of packets get \n\n");
                                                }
                                        }

                                        /* when we get GT_NO_RESOURCE then it means that not all packet that
                                        was transmitted were inserted into the FIFO, this is why
                                        there is no need to check if the number of transmitted
                                        packets are equal to the number of packets in the FIFO*/
                                        else
                                        {

                                                osPrintf("%d packets were sent without invoking TxBufferQueueEvent\n\n",sdmaTxPacketSendCount);

                                                sdmaTxPacketSendCount = 0;
                                                sdmaTxPacketGetCount = 0;

                                                /* wait and try to send the packet again */
                                                osTimerWkAfter(1);


                                                cpssDxChNetIfSdmaTxPacketSend(devNum, &cpssPcktParams, cpssDxChTxPacketDescTbl[i].pcktData,
                                                                                                          cpssDxChTxPacketDescTbl[i].pcktDataLen, cpssDxChTxPacketDescTbl[i].numOfBuffers);
                                        }

                                }

                                /* in case transmit succeed */
                                pcktNum++;
                                cpssDxChTxPacketDescTbl[i].numSentPackets++;


                                /* wait n milliseconds before sending next packet */
                                if(cpssDxChTxPacketDescTbl[i].gap != 0)
                                {
                                        osTimerWkAfter(cpssDxChTxPacketDescTbl[i].gap);
                                }
                        }

                        /* wait n milliseconds before moving to the next entry */
                        if(cpssDxChTxPacketDescTbl[i].waitTime != 0)
                        {
                                osTimerWkAfter(cpssDxChTxPacketDescTbl[i].waitTime);
                        }
                }
                /* wait n milliseconds before starting the next cycle */
                if(modeSettings.gap != 0)
                {
                        osTimerWkAfter(modeSettings.gap);
                }
                /* move to the next cycle */
                cycles++;
        }
        return GT_OK;
}
#if defined(CONFIG_V2_DMA)&&!defined (ASIC_SIMULATION)

/**
* @internal rxtxClient_configure_dma_per_devNum function
* @endinternal
*
* @brief   Map DMA buffer of a device.
*   Loop over all available device found on PCI/MBUS and map DMA buffer
*
* @param[in]    hwInfoPtr             - array of device hwInfo[busNo,  devSel, funcNo]
* @param[in]    numberOfDevices   - array size
*
* @retval GT_OK                    - on success,
* @retval GT_NOT_FOUND    - device not found in CPSS data base
*/
GT_STATUS   rxtxClient_configure_dma_per_devNum
(
    IN CPSS_HW_INFO_STC     *hwInfoPtr,
    IN GT_U32               numberOfDevices
)
{
    GT_STATUS rc = GT_OK;
    GT_U8     devNum;
    GT_U32    i;

    for(i=0;i<numberOfDevices;i++)
    {
      rc = prvCpssDeviceNumberByHwInfoGet(hwInfoPtr,&devNum);
      if(rc != GT_OK)
      {
          osPrintf("prvCpssDeviceNumberByHwInfoGet  failed with %d for index %d", rc,i);
          return rc;
      }

      osPrintf("Configure DMA for device %d \n", devNum);

          /*
     * Store current devNum for later use by DMA driver
     */
    PRV_OS_DMA_CURR_DEVICE_SET(devNum);

      extDrvMapDmaBasePerDevice(devNum);
    }
    return GT_OK;
}

#endif
/****************************************************
* entryPoint:
*             - run event Handler
*****************************************************/

static void entryPoint(void)
{
#ifdef APP_DEMO_CPU_ETH_PORT_MANAGE
#error "not supported"
        GT_STATUS rc;

        /* set appDemoPpConfigList params */
        appDemoPpConfigList[0].channel = CPSS_CHANNEL_SMI_E;
        /* set eth port parameters for RxTxProcess */
        CPSS_DXCH_MP_SMI_TRANSMIT_PARAMS_STC ethPortParams;

        rc = cpssMultiProcComGetSmiTransmitParams(&ethPortParams);

        if(ethPortParams.valid == GT_TRUE)
        {
                appDemoSysConfig = ethPortParams.appDemoSysConfig;
        }
        else
        {
                printf("Eth port pinit error\n");
        }

    if(appDemoSysConfig.cpuEtherPortUsed == GT_TRUE)
    {
        /* we need the dispatcher */
        rxTxCpssEventBind = appDemoEventBind_Dispatcher;
        rxTxCpssEventSelect = appDemoEventSelect_Dispatcher;
        rxTxCpssEventRecv = appDemoEventRecv_Dispatcher;
        rxTxCpssEventDeviceMaskSet = appDemoEventDeviceMaskSet_Dispatcher;
    }
        rxTxEthPrtInitNetIfDev();

#else

        appDemoPpConfigList[0].channel = CPSS_CHANNEL_PCI_E;

#endif


        if(rxTxEventRequestDrvnModeInit() == GT_FAIL)
        {
                osPrintf("rxTxEventRequestDrvnModeInit() error\n");
        osStopEngine();
                return;
        }

        /* SMI init */
#ifdef APP_DEMO_CPU_ETH_PORT_MANAGE
        appDemoSysConfig.cpuEtherPortUsed = GT_TRUE;
#endif
}


#ifdef SHARED_MEMORY
#ifdef ASIC_SIMULATION


#define BIND_FUNC(funcName)        \
                        simOsBindInfo.funcName = funcName

#define BIND_LEVEL_FUNC(level,funcName) \
            simOsBindInfo.level.funcName = funcName

SIM_OS_FUNC_BIND_STC simOsBindInfo;

static void simulationBindOsFuncs(void)
{
        /* reset all fields of simOsBindInfo */
        osMemSet(&simOsBindInfo,0,sizeof(simOsBindInfo));

        /* let the simulation OS bind it's own functions to the simulation */
        simOsFuncBindOwnSimOs();

        simOsBindInfo.osWrapperOpen = osWrapperOpen;

        BIND_LEVEL_FUNC(sockets,osSocketTcpCreate);
        BIND_LEVEL_FUNC(sockets,osSocketUdpCreate);
        BIND_LEVEL_FUNC(sockets,osSocketTcpDestroy);
        BIND_LEVEL_FUNC(sockets,osSocketUdpDestroy);
        BIND_LEVEL_FUNC(sockets,osSocketCreateAddr);
        BIND_LEVEL_FUNC(sockets,osSocketDestroyAddr);
        BIND_LEVEL_FUNC(sockets,osSocketBind);
        BIND_LEVEL_FUNC(sockets,osSocketListen);
        BIND_LEVEL_FUNC(sockets,osSocketAccept);
        BIND_LEVEL_FUNC(sockets,osSocketConnect);
        BIND_LEVEL_FUNC(sockets,osSocketSetNonBlock);
        BIND_LEVEL_FUNC(sockets,osSocketSetBlock);
        BIND_LEVEL_FUNC(sockets,osSocketSend);
        BIND_LEVEL_FUNC(sockets,osSocketSendTo);
        BIND_LEVEL_FUNC(sockets,osSocketRecv);
        BIND_LEVEL_FUNC(sockets,osSocketRecvFrom);
        BIND_LEVEL_FUNC(sockets,osSocketSetSocketNoLinger);
        BIND_LEVEL_FUNC(sockets,osSocketExtractIpAddrFromSocketAddr);
        BIND_LEVEL_FUNC(sockets,osSocketGetSocketAddrSize);
        BIND_LEVEL_FUNC(sockets,osSocketShutDown);

        BIND_LEVEL_FUNC(sockets,osSelectCreateSet);
        BIND_LEVEL_FUNC(sockets,osSelectEraseSet);
        BIND_LEVEL_FUNC(sockets,osSelectZeroSet);
        BIND_LEVEL_FUNC(sockets,osSelectAddFdToSet);
        BIND_LEVEL_FUNC(sockets,osSelectClearFdFromSet);
        BIND_LEVEL_FUNC(sockets,osSelectIsFdSet);
        BIND_LEVEL_FUNC(sockets,osSelectCopySet);
        BIND_LEVEL_FUNC(sockets,osSelect);
        BIND_LEVEL_FUNC(sockets,osSocketGetSocketFdSetSize);

        /* this needed for binding the OS of simulation with our OS functions */
        simOsFuncBind(&simOsBindInfo);

        return;
}



int simulationInputGet(int argc,  char * argv[])
{
   int i,len=0;

   cmdArgv = calloc(argc+1, sizeof(char*));
    if (cmdArgv == NULL)
    {
        fprintf(stderr, "calloc(%d, %d) failed: %s\n",
                argc+1, (int)sizeof(char*), strerror(errno));
        return 1;
    }
    commandLine[0] = 0;
    for (i=0; i<argc; i++)
    {
        len += strlen(argv[i]);
        cmdArgv[i] = (char *)argv[i];
        if ((unsigned)len < sizeof(commandLine)-2) /*for ' ' and '\0' */
        {
            strcat(commandLine,argv[i]);
            if (i<argc-1)
            {
                strcat(commandLine," ");
                len++;
            }
        }

    }
    cmdArgv[argc] = 0;

   return 0;
}


void simulationStart (void)
{
    /* Must be called before use any of OS functions. */
    osWrapperOpen(NULL);

    printf("Simulation init start....\n");

    simulationBindOsFuncs();

    /* call simulation init */
    simulationLibInit();

    printf("\nSimulation init done\n");

}



#endif
#endif


/****************************************************
*        Main function :
*            - init of CPSS Shared Lib
*            - start mainOs engine
*****************************************************/

int main(int argc, const char * argv[])
{
    GT_32 i;
    GT_BOOL verbose = GT_FALSE;
#ifdef ASIC_SIMULATION
    GT_U32 tcpPortSync,tcpPortAsync,clientIndex =0xFFFF,result;
    GT_BOOL aslrSupport = OS_ASLR_SUPPORT_DEFAULT;
    GT_STATUS rc;
#endif

    for(i=1;i<argc;i++)
    {
       if (strcmp(argv[i], "showRx") == 0)
       {
            showRxPackets = 1;
       }
       if (strcmp(argv[i], "dumpToFile") == 0)
       {
            dumpToFile = 1;
       }
       if (strcmp("-verbose", argv[i]) == 0)
       {
            verbose = GT_TRUE;
       }



#ifdef ASIC_SIMULATION
          if(strcmp("-noaslr", argv[i]) == 0)
          {
            aslrSupport=GT_FALSE;
          }

          if(0==strcmp(argv[i],CLIENT_TOKEN0))
          {
               if(i+1<argc)
               {
                   result = sscanf(argv[i+1],"%d",&clientIndex);
                   if(result==0)
                   {
                       clientIndex=0;
                   }
                }
           }
           else if(0==strcmp(argv[i],CLIENT_TOKEN1))
           {
               if(i+1<argc)
               {
                   result = sscanf(argv[i+1],"%d",&tcpPortSync);
                   if(result==0)
                   {
                       tcpPortSync=0;
                   }
                }
           }
           else if(0==strcmp(argv[i],CLIENT_TOKEN2))
           {
               if(i+1<argc)
               {
                   result = sscanf(argv[i+1],"%d",&tcpPortAsync);
                   if(result==0)
                   {
                       tcpPortAsync=0;
                   }
                }
           }
 #endif
    }


    if(GT_TRUE==verbose)
    {
        shrMemPrintMapsDebugInfo();
    }


#ifdef SHARED_MEMORY
#ifdef ASIC_SIMULATION
            if(GT_TRUE== aslrSupport)
            {   simulationInputGet(argc,(char **)argv);
                rc = simulationDistributedClientSet(clientIndex,tcpPortSync,tcpPortAsync);
                if(rc!=GT_OK)
                {
                   printf ("simulationDistributedClientSet  failed with code %d\n",rc);
                   return 1;
                }
                simulationStart();
            }
#endif
#endif

    osPrintf("\n=====================================\n");
    osPrintf("Create rxTx process and wait for events\n");
    osPrintf("=======================================\n\n");
    if (osStartEngine(argc, argv, "rxTxProcess", entryPoint) != GT_OK)
        return 1;

    return 0;
}


GT_STATUS rxTxProcessBindFuncCallbacks
(
    GT_VOID
)
{
    GT_STATUS rc;
    CPSS_EXT_DRV_FUNC_BIND_STC  extDrvFuncBindInfo;
    CPSS_OS_FUNC_BIND_STC       osFuncBind;
    CPSS_TRACE_FUNC_BIND_STC    traceFuncBindInfo;

    if(GT_FALSE == prvCpssGlobalDbExistGet())
    {
        /*
                    We must be LUA stand alone context if we got here.
                    So need to load shared memory and initialize callbacks.
                 */

        /* Shared memory approach on ARM should use dlsym-based workaround for pointers to functions */
        rc = shrMemGetDefaultOsBindFuncsThruDynamicLoader(&osFuncBind);
        if (rc != GT_OK)
        {
          return GT_FAIL;
        }
        /* Shared memory approach on ARM should use dlsym-based workaround for pointers to functions */
        rc = shrMemGetDefaultExtDrvFuncs(&extDrvFuncBindInfo);
        if (rc != GT_OK)
        {
          return GT_FAIL;
        }

       /* use NULL pointer to avoid trash stored by cpssExtServicesBind.
                The cpssExtServicesBind will use default "not implemented" callbacks in this case.  */
        osMemSet(&traceFuncBindInfo, 0, sizeof(traceFuncBindInfo));

        rc = cpssExtServicesBind(&extDrvFuncBindInfo, &osFuncBind, &traceFuncBindInfo);
        if(rc != GT_OK)
        {
            return rc;
        }

        rc = cpssGlobalDbInit(GT_TRUE,GT_FALSE,osNonSharedGlobalVars.osNonVolatileDb.noKmMappingsList,
            osNonSharedGlobalVars.osNonVolatileDb.noKmMappingsNum);
        if(rc != GT_OK)
        {
            return rc;
        }
              /*DMA map*/
#if defined(CONFIG_V2_DMA) &&!defined (ASIC_SIMULATION)
        rc = rxtxClient_configure_dma_per_devNum(osNonSharedGlobalVars.osNonVolatileDb.noKmMappingsList,
            osNonSharedGlobalVars.osNonVolatileDb.noKmMappingsNum);
        if(rc != GT_OK)
        {
            return rc;
        }
#endif
    }

    return GT_OK;
}


/***********************************************************************/

#ifdef APP_DEMO_CPU_ETH_PORT_MANAGE


GT_STATUS interruptEthPortRxSR
(
    IN GT_U8_PTR        segmentList[],
    IN GT_U32           segmentLen[],
    IN GT_U32           numOfSegments,
    IN GT_U32           rxQueue
)
{
        GT_STATUS               rc;
        GT_U8                   devNum;             /* src device number        */
        APP_DEMO_ETH_PORT_RX_Q_CTRL *qCtrl_PTR;         /* queue control struct     */
        GT_32                   intKey;             /* context of interrupts    */
        APP_DEMO_RX_Q_MSG           qMsg;               /* the message sent to Q    */
        GT_U32                  seg;                /* iterator                 */
        APP_DEMO_SEG_STC            *currSeg_PTR;       /* segment ptr              */
        APP_DEMO_SEG_STC            *newSeg_PTR;        /* segment ptr              */
        GT_BOOL       isAuMsg;
        GT_QUE_ID     queueId;

        qCtrl_PTR = appDemoEthPrtRxQCtrlGet();

        if (NULL == qCtrl_PTR)
        {
                return GT_OK;
        }

        /* validate the number of segments received */
        if (0 == numOfSegments || MAX_GT_BUF_SEG_NUM < numOfSegments)
        {
                qCtrl_PTR->packetFreeFunc_PTR(segmentList, numOfSegments, rxQueue);
                return GT_OK;
        }

        /* validate the Rx queue number */
        if (rxQueue >= NUM_OF_RX_QUEUES)
        {
                qCtrl_PTR->packetFreeFunc_PTR(segmentList, numOfSegments, rxQueue);
                return GT_OK;
        }

        ethPortDbg[rxQueue].pcktIn++;

        /* validate minimal packet size */
        if (segmentLen[0] < APP_DEMO_ETHPRT_BUFF_MIN_SIZE)
        {
                qCtrl_PTR->packetFreeFunc_PTR(segmentList, numOfSegments, rxQueue);
                ethPortDbg[rxQueue].pcktLen++;
                return GT_OK;
        }

        /* validate the devNum */
        if (appDemoSysConfig.cpuEtherPortUsed == GT_FALSE)
        {
                qCtrl_PTR->packetFreeFunc_PTR(segmentList, numOfSegments, rxQueue);
                ethPortDbg[rxQueue].device++;
                return GT_OK;
        }

        /* get the device from which we get the packets ---
           currently support getting packets from one device --
           from CPU Eth port */
        devNum = appDemoSysConfig.cpuEtherInfo.devNum;

        /* get the first segment node from pool */
        newSeg_PTR = appDemoEthPortPoolGetBuf(GT_ETH_PORT_RX_SEG_POOL_E);

        if (NULL == newSeg_PTR)
        {
                qCtrl_PTR->packetFreeFunc_PTR(segmentList, numOfSegments, rxQueue);
                ethPortDbg[rxQueue].noQbuffer++;

                return GT_OK;
        }

        /* save the first segment, it is transmitted     */
        /* through the queue to the Rx handler routine  */
        qMsg.seg = (GT_U32)newSeg_PTR;

        newSeg_PTR->len     = segmentLen[0];
        newSeg_PTR->seg_PTR = segmentList[0];

        currSeg_PTR = newSeg_PTR;

        /* get buffers from pool for all remaining segments */
        for (seg = 1; seg < numOfSegments; seg++)
        {
                /* get a segment node from pool */
                newSeg_PTR = appDemoEthPortPoolGetBuf(GT_ETH_PORT_RX_SEG_POOL_E);
                /* buffer allocation failed on on certain segment
                   already allocated buffers has to be released */
                if (NULL == newSeg_PTR)
                {
                        /* find start of the list */
                        newSeg_PTR = (APP_DEMO_SEG_STC *)qMsg.seg;
                        /* run on the list till failed segment */
                        while (newSeg_PTR != NULL)
                        {
                                currSeg_PTR = newSeg_PTR;
                                newSeg_PTR = newSeg_PTR->nextSeg_PTR;
                                appDemoEthPortPoolFreeBuf(GT_ETH_PORT_RX_SEG_POOL_E,currSeg_PTR);
                        }

                        qCtrl_PTR->packetFreeFunc_PTR(segmentList, numOfSegments, rxQueue);
                        /* update the number of free buffers according to loop iterator */
                        ethPortDbg[rxQueue].noQbuffer++ ;

                        return GT_OK;
                }

                newSeg_PTR->len     = segmentLen[seg];
                newSeg_PTR->seg_PTR = segmentList[seg];

                currSeg_PTR->nextSeg_PTR = newSeg_PTR;

                currSeg_PTR = newSeg_PTR;
        }

        /* terminate the linked list of segments */
        currSeg_PTR->nextSeg_PTR = NULL;

        /* set the Queue Rx queue number */
        qMsg.rxQueue = rxQueue;

        intKey = 0;

        /* Usually this routine is called from interrupt context */
        rc = appDemoEthPrtRxIntCauseGet(devNum,
                                                                 &segmentList[0][GT_MRVL_TAG_PCKT_OFFSET],
                                                                 &isAuMsg);
        if (rc != GT_OK)
        {
                qCtrl_PTR->packetFreeFunc_PTR(segmentList, numOfSegments, rxQueue);
                return GT_OK;
        }

        if (GT_TRUE == isAuMsg)
        {
                queueId = qCtrl_PTR->auQueId;
        }
        else
        {
                queueId = qCtrl_PTR->intQueId[(GT_U8)rxQueue];
        }

        /* send the packet through Queue */
        if (msgQueueSend(queueId,
                                         (GT_8*)&qMsg,
                                         APP_DEMO_RX_Q_MSG_LEN) != GT_OK)
        {
                /* queue send failed, clean up all allocated buffers */
                ethPortDbg[rxQueue].msgSend++;

                /* Free the buffer */
                newSeg_PTR = (APP_DEMO_SEG_STC *)qMsg.seg;

                while (newSeg_PTR != NULL)
                {
                        currSeg_PTR = newSeg_PTR;
                        newSeg_PTR = newSeg_PTR->nextSeg_PTR;
                        appDemoEthPortPoolFreeBuf(GT_ETH_PORT_RX_SEG_POOL_E,currSeg_PTR);
                }

                qCtrl_PTR->packetFreeFunc_PTR(segmentList, numOfSegments, rxQueue);

                return GT_OK;
        }

        osTaskLock();
        extDrvSetIntLockUnlock(INTR_MODE_LOCK, &intKey);

        {

                GT_UINTPTR       evHndlPtr;

                /* the pss will emulate the "even of rx packets" */
                rc = appDemoEvReqNodeGet_cpssEmulate(devNum,
                                CPSS_PP_RX_BUFFER_QUEUE0_E + rxQueue,
                                &evHndlPtr);

                if(rc == GT_OK)
                {
                        rc = evReqQInsert(evHndlPtr, 0, GT_FALSE);
                }
        }

        extDrvSetIntLockUnlock(INTR_MODE_UNLOCK, &intKey);
        osTaskUnLock();

        if (rc != GT_OK)
        {
                qCtrl_PTR->packetFreeFunc_PTR(segmentList, numOfSegments, rxQueue);
                return GT_OK;
        }

        ethPortDbg[rxQueue].pcktTreated++;

        return GT_OK;
}

GT_STATUS interruptEthPortTxEndSR
(
    IN GT_U8_PTR        segmentList[],
    IN GT_U32           numOfSegments
)
{
        GT_U32                      intKey;      /* context of interrupts       */
        APP_DEMO_ETH_PORT_TX_CTRL     * txCtrl_PTR;  /* Tx control struct           */
        GT_U8                       txDevice;    /* the source device number    */
        GT_U8                     * hdr_PTR;     /* header buffer ptr           */
        GT_U8                       txQueue;/* the queue the packet was send on */
        GT_TX_END_FIFO          *fifoPtr;     /* TxEnd FIFO pointer         */
        SW_TX_FREE_DATA_STCT    *txEndFifoPtr;/* TxEnd FIFO element ptr     */
        GT_U32                  userData;     /* user data from packet      */
        GT_U32                  evHndl;       /* event handle from packet   */
        GT_EVENT_HNDL*          evHndlPtr;    /* ptr to event handle        */

        txCtrl_PTR = appDemoEthPrtTxCtrlGet();

        if (NULL == txCtrl_PTR)
                return GT_FAIL;

        /* get the device number from the header segment (segment 0) */
        hdr_PTR = segmentList[0] - APP_DEMO_ETHPRT_HDR_MAC_OFFSET;
        txDevice  = hdr_PTR[APP_DEMO_ETHPRT_HDR_DEVICE_OFFSET]; /* devNum is GT_U8 */
        txQueue = hdr_PTR[APP_DEMO_ETHPRT_HDR_QUEUE_OFFSET]; /* queue is GT_U8 */

        /* validate the device number */
/*
        if (txDevice >= PRV_CPSS_MAX_PP_DEVICES_CNS ||
                appDemoPpConfigList[txDevice].valid == GT_FALSE)
        {
                appDemoEthPortPoolFreeBuf(GT_ETH_PORT_TX_POOL_E, (GT_VOID *)hdr_PTR);
                return GT_FAIL;
        }
*/
        /* extract user data (cookie) from packet header */
        userData  = (GT_U32)hdr_PTR[APP_DEMO_ETHPRT_HDR_COOKIE_OFFSET]   << 24;
        userData |= (GT_U32)hdr_PTR[APP_DEMO_ETHPRT_HDR_COOKIE_OFFSET+1] << 16;
        userData |= (GT_U32)hdr_PTR[APP_DEMO_ETHPRT_HDR_COOKIE_OFFSET+2] <<  8;
        userData |= (GT_U32)hdr_PTR[APP_DEMO_ETHPRT_HDR_COOKIE_OFFSET+3];

        /* extract the user handle from tx packet */
        evHndl  = (GT_U32)hdr_PTR[APP_DEMO_ETHPRT_HDR_HNDL_OFFSET]   << 24;
        evHndl |= (GT_U32)hdr_PTR[APP_DEMO_ETHPRT_HDR_HNDL_OFFSET+1] << 16;
        evHndl |= (GT_U32)hdr_PTR[APP_DEMO_ETHPRT_HDR_HNDL_OFFSET+2] <<  8;
        evHndl |= (GT_U32)hdr_PTR[APP_DEMO_ETHPRT_HDR_HNDL_OFFSET+3];
        evHndlPtr = (GT_EVENT_HNDL*)evHndl;

        /* set the FIFO element data */
        txEndFifoPtr = (SW_TX_FREE_DATA_STCT*)hdr_PTR;
        txEndFifoPtr->devNum   = txDevice;
        txEndFifoPtr->queueNum = txQueue;
        txEndFifoPtr->userData = (GT_PTR)userData;
        txEndFifoPtr->nextPtr  = NULL;

        /* insert the new Tx end node into FIFO */

        /* lock to prevent preemption on FIFO */
        intKey = 0;
        osTaskLock();
        extDrvSetIntLockUnlock(INTR_MODE_LOCK, (GT_32*)&intKey);

        fifoPtr = (GT_TX_END_FIFO*)evHndlPtr->extDataPtr;

        if (NULL == fifoPtr->tailPtr)
        {
                fifoPtr->tailPtr = (SW_TX_FREE_DATA_STCT*)hdr_PTR;
                fifoPtr->headPtr = (SW_TX_FREE_DATA_STCT*)hdr_PTR;
        }
        else
        {
                fifoPtr->tailPtr->nextPtr = (SW_TX_FREE_DATA_STCT*)hdr_PTR;
                fifoPtr->tailPtr          = (SW_TX_FREE_DATA_STCT*)hdr_PTR;
        }

        extDrvSetIntLockUnlock(INTR_MODE_UNLOCK, (GT_32*)&intKey);
        osTaskUnLock();

        /* signal the user process */
        osSemSignal((void*)evHndlPtr->semId);

        return GT_OK;
}

static GT_STATUS rxTx_appDemoEthPortTxEndGet
(
    IN GT_U32               hndl,
    OUT GT_U8               *devPtr,
    OUT GT_PTR              *cookiePtr,
    OUT GT_U8               *queuePtr,
    OUT GT_STATUS           *statusPtr
)
{

    GT_EVENT_HNDL           *evHndlPtr;  /* The event handle pointer        */
    GT_TX_END_FIFO          *fifoPtr;    /* The Handle Tx-End FIFO          */
    GT_U32                  intKey;      /* The interrupt key               */
    SW_TX_FREE_DATA_STCT    *fifoElemPtr;/* FIFO element                    */

    evHndlPtr = (GT_EVENT_HNDL*)hndl;

    /* validate handle */
    if (NULL == evHndlPtr || NULL == evHndlPtr->extDataPtr)
    {
        return GT_FAIL;
    }

    /* set alias FIFO pointer */
    fifoPtr = (GT_TX_END_FIFO*)evHndlPtr->extDataPtr;

    /* validate FIFO is not empty */
    if (NULL == fifoPtr->headPtr)
    {
        return GT_NO_MORE;
    }

    /* lock section from ISR preemption */
    intKey = 0;
    osTaskLock();
    extDrvSetIntLockUnlock(INTR_MODE_LOCK, (GT_32*)&intKey);

    /* set the packet header from FIFO first element */
    fifoElemPtr = fifoPtr->headPtr;

    /* remove the first element from FIFO */
    fifoPtr->headPtr = fifoElemPtr->nextPtr;

    if (NULL == fifoPtr->headPtr)
    {
        /* last element in FIFO, update last pointer */
        fifoPtr->tailPtr = NULL;
    }

    extDrvSetIntLockUnlock(INTR_MODE_UNLOCK, (GT_32*)&intKey);
    osTaskUnLock();

    /* update other output parameters */
    *devPtr     = fifoElemPtr->devNum;
    *queuePtr   = fifoElemPtr->queueNum;
    *cookiePtr  = fifoElemPtr->userData;
    *statusPtr  = GT_OK;

    /* return the packet header buffer to pool */
    if (appDemoEthPortPoolFreeBuf(GT_ETH_PORT_TX_POOL_E,
                                 (GT_U8*)fifoElemPtr) != GT_OK)
    {
        return GT_FAIL;
    }


    return GT_OK;
}



static GT_STATUS rxTx_appDemoEthPrtTransmitPacket
(
    IN APP_DEMO_ETH_PORT_NET_TX_PARAMS_STC * pcktParams,
    IN GT_U8              * packetBuffs[],
    IN GT_U32               buffLen[],
    IN GT_U32               numOfBufs
)
{

    GT_STATUS               rc;

        GT_SEM                  txSem;
    APP_DEMO_ETH_PORT_TX_CTRL   *txCtrlPtr;
    GT_BOOL getTxSem = GT_TRUE;

    txCtrlPtr = appDemoEthPrtTxCtrlGet();

    if (NULL == txCtrlPtr)
    {
        return GT_FAIL;
    }
    txSem = txCtrlPtr->txSem;

    TAKE_TX_SEM(getTxSem, txSem);

        printf("-> enter ethPrtTransmitPacket\n");

    rc = ethPrtTransmitPacket(pcktParams,packetBuffs,buffLen,numOfBufs);

        printf("-> exit ethPrtTransmitPacket rc - %d \n",rc);

    GIVE_TX_SEM(getTxSem, txSem);

        return rc;
}

#if 0
static APP_DEMO_ETH_PORT_TX_CTRL * appDemoEthPrtTxCtrlGet(GT_VOID)
{
    if (appDemoEthPrtCtrl.TxValid)
        {
                return &appDemoEthPrtTxCtrl;
        }
    else
        {
                return NULL;
        }

}
#endif

static GT_STATUS ethPrtTransmitPacket
(
    IN APP_DEMO_ETH_PORT_NET_TX_PARAMS_STC * pcktParams,
    IN GT_U8              * packetBuffs[],
    IN GT_U32               buffLen[],
    IN GT_U32               numOfBufs
)
{
    GT_STATUS       rc;
    GT_U8         * hdr_PTR;                /* TX header buff from pool     */
    GT_U16          buff;                   /* buffer index                 */
    GT_U8           segment;                /* the segment index            */
    GT_U8         * segmentList[MAX_GT_BUF_SEG_NUM]; /* segment ptr list    */
    GT_U32          segmentLen[MAX_GT_BUF_SEG_NUM]; /* segment length list   */
    GT_U8           bufferPcktOffset;       /* second segment offset        */
    GT_U32          cookie;                 /* the user cookie              */
    GT_U32          ii;
    GT_U32          hndl;                   /* handle got from gtEventBind  */
    GT_U8           devNum = appDemoSysConfig.cpuEtherInfo.devNum;
    GT_BOOL         controlPacket = GT_FALSE;
    GT_U32          dsaSize = 8;
    GT_BOOL         sendToDxCh = GT_FALSE;
    GT_BOOL         sendToExMxPm = GT_FALSE;


    if(appDemoSysConfig.cpuEtherPortUsed == GT_FALSE)
    {
        return GT_BAD_STATE;
    }


/* ALEXLIF ????? */
CPSS_PP_FAMILY_TYPE_ENT    localDevFamily = CPSS_PP_FAMILY_CHEETAH_E;
/*
        switch(appDemoPpConfigList[devNum].devFamily)
*/
        switch(localDevFamily)
    {
        case CPSS_PP_FAMILY_CHEETAH2_E:
#ifdef CHX_FAMILY
            if(pcktParams->dsa.dxChDsaParam.dsaType == CPSS_DXCH_NET_DSA_CMD_FROM_CPU_E &&
               pcktParams->dsa.dxChDsaParam.dsaInfo.fromCpu.egrFilterEn == GT_TRUE)
            {
                controlPacket = GT_TRUE;
            }

            switch(pcktParams->dsa.dxChDsaParam.commonParams.dsaTagType)
            {
                case CPSS_DXCH_NET_DSA_TYPE_REGULAR_E:
                    dsaSize = 4;
                    break;
                case CPSS_DXCH_NET_DSA_TYPE_EXTENDED_E:
                    dsaSize = 8;
                    break;
                default:
                    return GT_BAD_PARAM;
            }

            sendToDxCh = GT_TRUE;

            break;
#else /*CHX_FAMILY*/
            return GT_BAD_STATE;
#endif /*CHX_FAMILY*/
        case CPSS_PP_FAMILY_PUMA_E:
            return GT_BAD_STATE;
        default:
            return GT_NOT_SUPPORTED;
    }



    /* validate minimum packet length */
    if (0 == numOfBufs || APP_DEMO_ETHPRT_BUFF_MIN_SIZE > buffLen[0])
    {
        return GT_FAIL;
    }

    hdr_PTR = (GT_U8 *)appDemoEthPortPoolGetBuf(GT_ETH_PORT_TX_POOL_E);


        if (NULL == hdr_PTR)
    {
        return GT_FAIL;
    }


    /* copy the packet MAC addresses */
    osMemCpy(&hdr_PTR[APP_DEMO_ETHPRT_HDR_MAC_OFFSET],
             packetBuffs[0],
             APP_DEMO_ETHPRT_HDR_MAC_SIZE);

    if (pcktParams->packetIsTagged == GT_TRUE)
    {
        bufferPcktOffset    = APP_DEMO_BUFF_PCKT_TAGGED_OFFSET;
    }
    else
    {
        bufferPcktOffset    = APP_DEMO_BUFF_PCKT_UNTAGGED_OFFSET;
    }

    /* set first segment to point to buffer header from pool */
    segmentList[0]  = (GT_U8*)&hdr_PTR[APP_DEMO_ETHPRT_HDR_MAC_OFFSET];
    segmentLen[0]   = APP_DEMO_ETHPRT_HDR_MAC_SIZE + dsaSize/* bytes of regular/extended DSA*/;

    /* set the second segment to point to the rest of the buffer */
    segmentList[1]  = &packetBuffs[0][bufferPcktOffset];
    segmentLen[1]   = buffLen[0] - bufferPcktOffset;

    /* copy the rest of the pointers and length to segment list */
    for (segment = 2, buff = 1; buff < numOfBufs; buff++, segment++)
    {
        segmentList[segment] = &packetBuffs[buff][0];
        segmentLen[segment]  = buffLen[buff];
    }

    /* if recalc CRC and regular encapsulation, reduce the packet    */
    /* length by 4 the PP adds the CRC by default on egress.        */
    if (GT_TRUE == pcktParams->recalcCrc &&
        controlPacket == GT_FALSE)
    {
        segmentLen[numOfBufs] -= 4;
    }


    rc = GT_BAD_STATE;

    if(sendToDxCh == GT_TRUE)
    {
        /* build DSA Tag buffer */
#ifdef CHX_FAMILY
        rc = cpssDxChNetIfDsaTagBuild(devNum,
                &pcktParams->dsa.dxChDsaParam,
                &hdr_PTR[APP_DEMO_ETHPRT_HDR_MRVLTAG_OFFSET]);
#endif /*CHX_FAMILY*/
    }
    else if(sendToExMxPm == GT_TRUE)
    {
        /* build DSA Tag buffer */
    }


    if(rc != GT_OK)
    {
        appDemoEthPortPoolFreeBuf(GT_ETH_PORT_TX_POOL_E, hdr_PTR);
        return rc;
    }


    /* set the device number in the header buffer */
    ii = APP_DEMO_ETHPRT_HDR_DEVICE_OFFSET;
    hdr_PTR[ii] = devNum; /* one byte */

    /* set the queue index in the header buffer */
    ii = APP_DEMO_ETHPRT_HDR_QUEUE_OFFSET; /* one byte */
    hdr_PTR[ii] = pcktParams->txQueue;

    ii = APP_DEMO_ETHPRT_HDR_COOKIE_OFFSET;
    /* set the control cookie in the header buffer */
    cookie = (GT_U32)pcktParams->cookie;
    hdr_PTR[ii++] = (GT_U8)(cookie >> 24);
    hdr_PTR[ii++] = (GT_U8)(cookie >> 16);
    hdr_PTR[ii++] = (GT_U8)(cookie >> 8);
    hdr_PTR[ii++] = (GT_U8)(cookie);

    ii = APP_DEMO_ETHPRT_HDR_HNDL_OFFSET;
    hndl = pcktParams->evReqHndl;
    hdr_PTR[ii++] = (GT_U8)(hndl >> 24);
    hdr_PTR[ii++] = (GT_U8)(hndl >> 16);
    hdr_PTR[ii++] = (GT_U8)(hndl >> 8);
    hdr_PTR[ii++] = (GT_U8)(hndl);


    if (extDrvEthPortTx(segmentList, segmentLen, segment, pcktParams->txQueue) != GT_OK)
    {
            appDemoEthPortPoolFreeBuf(GT_ETH_PORT_TX_POOL_E, hdr_PTR);
        return GT_FAIL;
    }

        printf("-->> extDrvEthPortTx - transmitted \n");

    return GT_OK;
}






GT_STATUS appDemoEthPortPoolFreeBuf
(
    IN  GT_ETH_PORT_POOL_TYPE_ENT   poolIdx,
    IN  GT_VOID                   * buf_PTR
)
{
    GT_ETH_PORT_POOL_FREE_BUF_HEAD  *retBuf_PTR;
    GT_ETH_PORT_POOL_HEAD           *head_PTR;

    if (poolIdx >= GT_ETH_PORT_MAX_POOLS_E)
    {
        return GT_BAD_VALUE;
    }

        /*
    head_PTR = &poolArr[poolIdx];
        */
        head_PTR = &poolArrPtr[poolIdx];

    /* pool boundaries check */
    if ((GT_U8*)buf_PTR < (GT_U8*)head_PTR->alignedBlockStart_PTR ||
        (GT_U8*)buf_PTR > (GT_U8*)head_PTR->alignedBlockEnd_PTR)
    {
                /*
        DBG_INFO(("free: boundry fail, pool= %d\n",poolIdx));
        */
        return GT_BAD_PTR;
    }

    /* buffer alignment check */
    if (((GT_U8*)buf_PTR - (GT_U8*)head_PTR->alignedBlockStart_PTR) %
        head_PTR->neededBufferSize != 0)
    {
                /*
        DBG_INFO(("free: block align fail, pool= %d\n",poolIdx));
*/
        return GT_BAD_PTR;
    }

    /* lock section, wait for ever */
    osSemWait(head_PTR->poolSem, 0);

    /* Insert the returned buffer to the head of the list */
    retBuf_PTR = buf_PTR;
    retBuf_PTR->pNext = head_PTR->listHead_PTR;
    head_PTR->listHead_PTR = retBuf_PTR;

    /* Update number of free buffers */
    head_PTR->numOfFreeBuffers++;

#ifdef ETH_POOL_DEBUG
    if (head_PTR->numOfFreeBuffers > head_PTR->numOfBuffers)
        osPrintf("Too many buffers, pool= %d, free= %d max= %d\n",
                 poolIdx, head_PTR->numOfFreeBuffers, head_PTR->numOfBuffers);
#endif

    /* unlock section */
    osSemSignal(head_PTR->poolSem);

    return GT_OK;
}

GT_VOID * appDemoEthPortPoolGetBuf
(
    IN  GT_ETH_PORT_POOL_TYPE_ENT poolIdx
)
{
    GT_ETH_PORT_POOL_FREE_BUF_HEAD *buf_PTR;
    GT_ETH_PORT_POOL_HEAD          *head_PTR;

    if(poolIdx >= GT_ETH_PORT_MAX_POOLS_E)
    {
        return NULL;
    }

        /*
    head_PTR = &poolArr[poolIdx];
*/
         head_PTR = &poolArrPtr[poolIdx];

    /* lock section, wait for ever */
    osSemWait(head_PTR->poolSem, 0);

    /* Take the buffer from the head of the list */
    buf_PTR = head_PTR->listHead_PTR;

    /* if the list is not empty, update the header */
    if (buf_PTR != NULL)
    {
        head_PTR->listHead_PTR = buf_PTR->pNext;

#ifdef ETH_POOL_DEBUG
        if (head_PTR->numOfFreeBuffers == 0)
            osPrintf("List and counter not in sync, pool= %d\n", poolIdx);
#endif

        /* Update number of free buffers */
        head_PTR->numOfFreeBuffers--;
    }
#ifdef ETH_POOL_DEBUG
    else
    {
        if (head_PTR->numOfFreeBuffers != 0)
            osPrintf("List and counter not in sync, pool= %d cnt= %d\n",
                     poolIdx, head_PTR->numOfFreeBuffers);
    }
#endif

    /* unlock section */
    osSemSignal(head_PTR->poolSem);

    return buf_PTR;
}

GT_STATUS rxTxEthPrtInitNetIfDev
(
    void
)
{
    GT_STATUS           retVal;
    GT_U32              txHdrBufSize;
    static GT_BOOL      initialized = GT_FALSE;
    static GT_STATUS    lastStatus = GT_FAIL;

/*
    if(appDemoSysConfig.cpuEtherPortUsed == GT_FALSE ||
       appDemoPpConfigList[appDemoSysConfig.cpuEtherInfo.devNum].valid == GT_FALSE )
    {
        return GT_BAD_STATE;
    }
*/

    /* the NetworkIf should not be initialized for this device */
    if (CPSS_RX_BUFF_NO_ALLOC_E == appDemoSysConfig.cpuEtherInfo.rxBufInfo.allocMethod)
    {
        return GT_OK;
    }

    if (initialized)
    {
        return lastStatus;
    }

    initialized = GT_TRUE;

    appDemoPpConfigList[0].devFamily = CPSS_PP_FAMILY_CHEETAH_E;

    /* INIT RX MODULE */
    retVal = ethPrtInitRx(&appDemoSysConfig.cpuEtherInfo.rxBufInfo,
                              appDemoSysConfig.cpuEtherInfo.rxBufInfo.rxBufSize,
                              NUM_OF_INT_QUEUES,
                              appDemoSysConfig.cpuEtherInfo.rxBufInfo.bufferPercentage,
                              appDemoSysConfig.cpuEtherInfo.rxBufInfo.headerOffset);
    if (GT_OK != retVal)
    {
        return retVal;
    }


    /* INIT TX MODULE */
    /* The Tx desc block and size are utilized for Tx header buffer.    */
    /* This block is allocated by user. The Tx header buffer is used to */
    /* allow user Cookie and Marvell-Tag insertion in frame.            */
/*
        appDemoGetTxDescSize(appDemoPpConfigList[appDemoSysConfig.cpuEtherInfo.devNum].deviceId, &txHdrBufSize);
*/

        appDemoGetTxDescSize(CPSS_98DX107_CNS, &txHdrBufSize);

    retVal = ethPrtInitTx(txHdrBufSize,
                              appDemoSysConfig.cpuEtherInfo.txHdrBufBlock_PTR,
                              appDemoSysConfig.cpuEtherInfo.txHdrBufBlockSize);

    if (GT_OK != retVal)
    {
        return retVal;
    }

    /* bind the RX callback routine */
    retVal = extDrvEthInputHookAdd(extDrvEthPortRxCb);
    if (GT_OK != retVal)
    {
        return retVal;
    }

    /* bind the TX complete callback routine */
    retVal = extDrvEthTxCompleteHookAdd(extDrvEthPortTxEndCb);
    if (GT_OK != retVal)
    {
        return retVal;
    }

    /* enable the ethernet port */
    retVal = extDrvEthPortEnable();
    if (GT_OK != retVal)
    {
        return retVal;
    }

    lastStatus = GT_OK;

    return GT_OK;
}


GT_STATUS extDrvEthPortRxCb
(
    IN GT_U8_PTR        segmentList[],
    IN GT_U32           segmentLen[],
    IN GT_U32           numOfSegments,
    IN GT_U32           queueNum
)
{
#ifdef APP_DEMO_CPU_ETH_PORT_MANAGE
    return interruptEthPortRxSR(segmentList, segmentLen, numOfSegments, queueNum);
#else /*APP_DEMO_CPU_ETH_PORT_MANAGE*/
    return GT_NOT_SUPPORTED;
#endif /*APP_DEMO_CPU_ETH_PORT_MANAGE*/
}


GT_STATUS extDrvEthPortTxEndCb
(
    IN GT_U8_PTR        segmentList[],
    IN GT_U32           numOfSegments
)
{
#ifdef APP_DEMO_CPU_ETH_PORT_MANAGE
    CPSS_TBD_BOOKMARK
    /* Shared Memory implementation should be updated to use native MII APIs. */
    return interruptEthPortTxEndSR(segmentList, numOfSegments);
#else /*APP_DEMO_CPU_ETH_PORT_MANAGE*/
    return GT_NOT_SUPPORTED;
#endif /*APP_DEMO_CPU_ETH_PORT_MANAGE*/
}

GT_STATUS appDemoGetTxDescSize
(
    IN  CPSS_PP_DEVICE_TYPE     devType,
    OUT GT_U32                  *descSize
)
{
#if defined CHX_FAMILY
    return cpssDxChHwTxDescSizeGet(devType, descSize);
#else
    *descSize = 0;
    return GT_NOT_IMPLEMENTED;
#endif /* CHX_FAMILY */
}

static GT_STATUS ethPrtInitTx
(
    IN GT_U32       txHdrBufSize,
    IN GT_U32     * txHdrBufBlock_PTR,
    IN GT_U32       txHdrBufBlockSize
)
{
    GT_STATUS       status;
    GT_U32          numOfTxHdrBufs;



    /* tx buffers are taken from user allocated Tx decs Block. User MUST */
    /* allocate this area and set the tx desc block size accordingly.    */
    if (0 == txHdrBufBlockSize || NULL == txHdrBufBlock_PTR)
        return GT_FAIL;


    numOfTxHdrBufs = txHdrBufBlockSize / txHdrBufSize;

    osSemBinCreate("txSem", 1, &appDemoEthPrtTxCtrl.txSem);

    /* create pool with caller`s allocated memory */
    status = appDemoEthPortCreatePool(GT_ETH_PORT_TX_POOL_E,
                                     txHdrBufSize,
                                     &numOfTxHdrBufs,
                                     (GT_U8*)txHdrBufBlock_PTR,
                                     txHdrBufBlockSize,
                                     NULL);

    /* temporary solution, similar to previous version */
    if ((GT_OK != status) && (GT_NOT_SUPPORTED != status))
    {

        return GT_FAIL;
    }

    /* init the external driver Tx */
    status = extDrvEthPortTxInit(numOfTxHdrBufs);
    /* temporary solution, similar to previous version */
    if ((GT_OK != status) && (GT_NOT_SUPPORTED != status))
    {
        return GT_FAIL;
    }



    appDemoEthPrtCtrl.TxValid = GT_TRUE;

    return GT_OK;
}

static GT_STATUS ethPrtInitRx
(
    IN CPSS_RX_BUF_INFO_STC * rxBufInfo_PTR,
    IN GT_U32           rxBufSize,
    IN GT_U32           rxQNum,
    IN GT_U32           rxQbufPercentage[],
    IN GT_U32           headerOffset
)
{
    GT_U16              rxQueue;                /* Rx queue number          */
    GT_U8               semName[30];            /* semaphore name           */
    GT_U32              numOfRxBufs;            /* number of Rx buffers     */
    GT_U32              rxBufBlockSize;         /* Rx block size            */
    GT_STATUS           status;                 /* routine status           */
    GT_U8             * rxBufBlock_PTR;         /* Rx block mem pointer     */
    GT_U8             * tmp_PTR;                /*  */
    GT_U32              segPoolSize;            /*  */
    GT_U8             * segPoolBlock_PTR;       /*  */


        /* validate the Rx buffer size */
    if (0 == rxBufSize)
        return GT_FAIL;


    /* save space before user header for tagged packets indication */
    rxBufSize    += APP_DEMO_ETHPRT_TAGGED_PCKT_FLAG_LEN;
    headerOffset += APP_DEMO_ETHPRT_TAGGED_PCKT_FLAG_LEN;

    /* Align the buffer size */
    rxBufSize = (rxBufSize + ALIGN_4B_MASK_CNS) & (~ALIGN_4B_MASK_CNS);

    /* set the buffer block, size and malloc routine ptr */
    if (CPSS_RX_BUFF_STATIC_ALLOC_E == rxBufInfo_PTR->allocMethod)
    {
        rxBufBlock_PTR = (GT_U8*)rxBufInfo_PTR->buffData.staticAlloc.rxBufBlockPtr;
        rxBufBlockSize = rxBufInfo_PTR->buffData.staticAlloc.rxBufBlockSize;

        /* validate the Rx buffer block size */
        if (0 == rxBufBlockSize)
        {
            return GT_FAIL;
        }


        /* validate the buffer pointer */
        if (NULL == rxBufBlock_PTR)
        {
            return GT_FAIL;
        }

    }
    else
    {
        if (NULL == rxBufInfo_PTR->buffData.dynamicAlloc.mallocFunc)
        {
            return GT_FAIL;
        }


        numOfRxBufs = rxBufInfo_PTR->buffData.dynamicAlloc.numOfRxBuffers;

        /* validate the number of requested Rx buffers */
        if (0 == numOfRxBufs)
        {
            return GT_FAIL;
        }


        /* set the requested buffer block size */
        rxBufBlockSize = numOfRxBufs * rxBufSize;

        /* allocate memory for buffer pool */
        rxBufBlock_PTR = rxBufInfo_PTR->buffData.dynamicAlloc.mallocFunc(
                            rxBufBlockSize, ALIGN_4B_CNS);

        if (NULL == rxBufBlock_PTR)
        {
            return GT_NO_RESOURCE;
        }

    }

    /* align the memory pool block */
    if ((ALIGN_4B_MASK_CNS & (GT_U32)rxBufBlock_PTR) != 0)
    {
        tmp_PTR = rxBufBlock_PTR;
        rxBufBlock_PTR = (GT_U8*)((GT_U32)(rxBufBlock_PTR + ALIGN_4B_MASK_CNS) &
                                           (~ALIGN_4B_MASK_CNS));
        rxBufBlockSize -= rxBufBlock_PTR - tmp_PTR;
    }

    /* update the number of buffers in the pool */
    numOfRxBufs = rxBufBlockSize / rxBufSize;

    /* validate that we did not run out of buffers */
    if (0 == numOfRxBufs)
    {
        return GT_NO_RESOURCE;
    }


    /* init the external driver Rx */
    status = extDrvEthPortRxInit(rxBufBlockSize, rxBufBlock_PTR, rxQNum,
                                 rxQbufPercentage, rxBufSize, &numOfRxBufs,
                                 headerOffset);


    /* temporary solution, similar to previous version */
    if ((GT_OK != status) && (GT_NOT_SUPPORTED != status))
    {
        return GT_FAIL;
    }

    /* create segment pool that is used by the Rx ISR and Handler */
    segPoolSize         = numOfRxBufs * sizeof(APP_DEMO_SEG_STC);
    segPoolBlock_PTR    = osStaticMalloc(segPoolSize);

    if (NULL == segPoolBlock_PTR)
    {
        return GT_FAIL;
    }


    status = appDemoEthPortCreatePool(GT_ETH_PORT_RX_SEG_POOL_E, sizeof(APP_DEMO_SEG_STC),
                                     &numOfRxBufs, segPoolBlock_PTR, segPoolSize,
                                     NULL);


    /* temporary solution, similar to previous version */
    if ((GT_OK != status) && (GT_NOT_SUPPORTED != status))
    {
        return GT_FAIL;
    }

        /* get shared pointer */
    poolArrPtr = (GT_ETH_PORT_POOL_HEAD *)
        cpssMultiProcGetSharedData(CPSS_SHARED_DATA_ETHPORT_POOLARRAY_PTR_E);

    /* create the queues for the Rx packets (one per int queue) */
    for (rxQueue = 0; rxQueue < NUM_OF_INT_QUEUES; rxQueue++)
    {
        osSprintf(semName,"rxQ-%d", rxQueue);

        appDemoEthPrtRxQCtrl.intQueId[rxQueue] =
                                 msgQueueCreate(semName,
                                                numOfRxBufs,
                                                APP_DEMO_RX_Q_MSG_LEN);

        if (NULL == appDemoEthPrtRxQCtrl.intQueId[rxQueue])
        {
            return GT_FAIL;
        }

    }

    /* set the header size */
    appDemoEthPrtRxQCtrl.headerOffset = headerOffset;
    appDemoEthPrtRxQCtrl.auQueId      = NULL;
    appDemoEthPrtRxQCtrl.numOfRxBufs  = numOfRxBufs;

    /* bind External Driver routine */
    appDemoEthPrtRxQCtrl.packetFreeFunc_PTR = extDrvEthRxPacketFree;

    appDemoEthPrtCtrl.RxValid = GT_TRUE;


    return GT_OK;
}
GT_STATUS extDrvEthRxPacketFree
(
    IN GT_U8_PTR        segmentList[],
    IN GT_U32           numOfSegments,
    IN GT_U32           queueNum
)
{
    return GT_FAIL;
}


GT_STATUS appDemoEthPortCreatePool
(
    IN      GT_ETH_PORT_POOL_TYPE_ENT   poolIdx,
    IN      GT_U32                      reqBufferSize,
    INOUT   GT_U32                    * numOfBuffers_PTR,
    IN      GT_U8                     * pool_PTR,
    IN      GT_U32                      poolSize,
    IN      APPDEMO_CPSS_RX_BUFF_MALLOC_FUNC  mallocFunc
)
{
        #if 0
    GT_U32                          neededBufferSize;
    GT_U32                          neededPoolSize;
    GT_U32                          indx;
    GT_ETH_PORT_POOL_FREE_BUF_HEAD *buff_PTR;
    GT_ETH_PORT_POOL_HEAD          *head_PTR;
    GT_U8                          *tmp_PTR;

    buff_PTR = NULL;

    /* validate pool index */
    if (poolIdx >= GT_ETH_PORT_MAX_POOLS_E)
    {
        return GT_BAD_VALUE;
    }

    /* The numOfBuffers_PTR MUST not be zero. */
    if (0 == *numOfBuffers_PTR)
    {
        return GT_BAD_VALUE;
    }

    /* set the pool */
    head_PTR = &poolArr[poolIdx];

    /* Align the buffer size */
    neededBufferSize = (reqBufferSize + ALIGN_4B_MASK_CNS) & (~ALIGN_4B_MASK_CNS);

    /* allocate memory chunk for the pool if user did not allocate allready */
    if (NULL == pool_PTR)
    {
        neededPoolSize = (*numOfBuffers_PTR) * neededBufferSize;
        pool_PTR = mallocFunc(neededPoolSize, ALIGN_4B_CNS);
        if (NULL == pool_PTR)
        {
            return GT_NO_RESOURCE;
        }
    }
    else
    {
        neededPoolSize = poolSize;
    }

    /* Align the memory pool block if needed */
    if ((ALIGN_4B_MASK_CNS & (GT_U32)pool_PTR) != 0)
    {
        /* align the memory pool block */
        tmp_PTR = pool_PTR;
        pool_PTR = (GT_U8*)((GT_U32)(pool_PTR + ALIGN_4B_MASK_CNS) &
                            (~ALIGN_4B_MASK_CNS));
        neededPoolSize -= pool_PTR - tmp_PTR;
    }

    /* update the actual number of buffers in the pool */
    *numOfBuffers_PTR = neededPoolSize / neededBufferSize;

    /* validate that we did not run out of buffers */
    if (0 == (*numOfBuffers_PTR))
    {
        return GT_NO_RESOURCE;
    }

    /* now we have a valid pool size, number of buffers and buffer size */

    /* create semaphore for the pool get/free routines */
    if (osSemBinCreate(poolIdx == GT_ETH_PORT_RX_SEG_POOL_E ? "rxPool" : "txPool",
                       1, &head_PTR->poolSem) != GT_OK)
    {
        return GT_NO_RESOURCE;
    }

    /* Store the memory block boundaries, block memory address and its size */
    head_PTR->poolBlock_PTR         = pool_PTR;
    head_PTR->memBlocSize           = poolSize;
    head_PTR->alignedBlockStart_PTR = pool_PTR;
    head_PTR->alignedBlockEnd_PTR   = pool_PTR + neededPoolSize - 1;
    head_PTR->reqBufferSize         = reqBufferSize;
    head_PTR->neededBufferSize      = neededBufferSize;
    head_PTR->numOfBuffers          = *numOfBuffers_PTR;
    head_PTR->numOfFreeBuffers      = *numOfBuffers_PTR;

    /* The first buffer is located in the beginning of the memory block */
    head_PTR->listHead_PTR = (GT_ETH_PORT_POOL_FREE_BUF_HEAD*)pool_PTR;

    /* Devide the memory block into buffers and make it as a link list */
    for (indx=0; indx < (*numOfBuffers_PTR); indx++)
    {
        buff_PTR = (GT_ETH_PORT_POOL_FREE_BUF_HEAD*)pool_PTR;
        pool_PTR = pool_PTR + neededBufferSize;
        buff_PTR->pNext = (GT_ETH_PORT_POOL_FREE_BUF_HEAD*)pool_PTR;
    }

    /* Last buffer is pointing to NULL */
    buff_PTR->pNext = NULL;

        #endif
    return GT_OK;
}

#endif

