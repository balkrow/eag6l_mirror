#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <cmdShell/common/cmdCommon.h>
#include <cmdShell/FS/cmdFS.h>
#include <gtOs/gtOsTask.h>
#if 1/*[#43] LF발생시 RF 전달 기능 추가, balkrow, 2024-06-05*/
#include <gtOs/gtEnvDep.h>
#endif
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortSyncEther.h>
#if 1/*[#43] LF발생시 RF 전달 기능 추가, balkrow, 2024-06-05*/
#include <cpss/common/port/cpssPortManager.h>
#endif
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <netinet/in.h>

#include "multi_thread.h"
#if 1/*[#34] aldrin3s chip initial 기능 추가, balkrow, 2024-05-23*/
#include "sysmon.h"
#include "sys_fifo.h"
#include "eag6l.h"
#include "syslog.h"
#endif

#if 1/*[#43] LF발생시 RF 전달 기능 추가, balkrow, 2024-06-05*/
#include "eag6l_fsm.h"
#endif

#if 1/*[#35] traffic test 용 vlan 설정 기능 추가, balkrow, 2024-05-27*/
extern uint8_t EAG6LVlanInit (void);
extern uint8_t EAG6LFecInit (void);
extern GT_VOID appDemoTraceOn_GT_OK_Set(GT_U32);
extern void initFaultFsmList (void);
#endif

#if 1/*[#43] LF발생시 RF 전달 기능 추가, balkrow, 2024-06-05*/
extern uint8_t eag6LPortlist [];
extern uint8_t eag6LPortArrSize;
extern SVC_FAULT_FSM svcPortFaultFsm[9];
int32_t svc_fault_fsm_timer(struct multi_thread *thread);

int portLfRfDetect (struct multi_thread *thread);
uint8_t gEag6LSDKInitStatus = GT_FALSE;
#if 1/*[#43] LF발생시 RF 전달 기능 추가, balkrow, 2024-06-05*/
struct multi_thread *llcf_thread = NULL;
#endif

EVENT_NOTIFY_FUNC *notifyEventToIPC = NULL;
extern GT_STATUS cpssDxChPortManagerStatusGet
(
    IN  GT_U8                           devNum,
    IN  GT_PHYSICAL_PORT_NUM            portNum,
    OUT CPSS_PORT_MANAGER_STATUS_STC    *portStagePtr
);

extern GT_STATUS cpssDxChPortXgmiiLocalFaultGet
(
    IN  GT_U8       devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL     *isLocalFaultPtr
);

extern GT_STATUS cpssDxChPortXgmiiRemoteFaultGet
(
    IN  GT_U8       devNum,
    IN  GT_PHYSICAL_PORT_NUM portNum,
    OUT GT_BOOL     *isRemoteFaultPtr
);

extern GT_STATUS cpssDxChPortManagerEventSet
(
    IN  GT_U8                   devNum,
    IN  GT_PHYSICAL_PORT_NUM    portNum,
    IN  CPSS_PORT_MANAGER_STC   *portEventStcPtr
);
#endif

int sysrdfifo;
int syswrfifo;

struct multi_thread_master *master;

#if 1/*[#34] aldrin3s chip initial 기능 추가, balkrow, 2024-05-23*/
#undef DEBUG
uint8_t gEag6LIPCstate = IPC_INIT_SUCCESS; 

extern GT_STATUS cpssInitSystem
(
    IN  GT_U32  boardIdx,
    IN  GT_U32  boardRevId,
    IN  GT_U32  reloadEeprom
);


#if 1/*[#43] LF발생시 RF 전달 기능 추가, balkrow, 2024-06-05*/
GT_VOID processPortEvt
(
 GT_U8 devNum,
 GT_U32 uniEv,
 GT_U32 evExtData
 )
{
	CPSS_PORT_MANAGER_STATUS_STC portConfigOutParams;

	if(uniEv == CPSS_PP_PORT_PM_LINK_STATUS_CHANGED_E) 
	{
		cpssDxChPortManagerStatusGet(devNum, evExtData, &portConfigOutParams);
#if 1/*[#43] LF발생시 RF 전달 기능 추가, balkrow, 2024-06-05*/

		syslog(LOG_INFO,"port event : devNum=%x, uniEv=%x, evExtData=%x, portState=%s",
			devNum, uniEv, evExtData, 
			portConfigOutParams.portState == CPSS_PORT_MANAGER_STATE_LINK_UP_E ? "Link Up":"Link Down");
#endif
	}
}
#endif

static int send_to_sysmon_master(sysmon_fifo_msg_t * msg) {
#ifdef DEBUG
	syslog(LOG_INFO, "Send msg %x to sysmon", msg->type);
#endif
#if 0/*[#4] register updating : opening file at both sides don't work, dustin, 2024-05-28 */
	return write(syswrfifo, msg, sizeof(sysmon_fifo_msg_t));
#else
	if((syswrfifo = open(SYSMON_FIFO_READ/*to-master*/, O_RDWR)) < 0) {
		syslog(LOG_INFO, "Can't open file to send to master");
		return 1;
	}

	write(syswrfifo, msg, sizeof(sysmon_fifo_msg_t));

	close(syswrfifo);

	return 0;
#endif /*PWY_FIXME*/
}

uint8_t gCpssSDKInit(int args, ...)
{
	uint8_t result;
	va_list argP;
	sysmon_fifo_msg_t msg;

	result = cpssInitSystem(38, 1, 0);
#if 1/*[#35]traffic test 용 vlan 설정 기능 추가, balkrow, 2024-05-27*/
	/*initial tag/untag forwarding */
	result = EAG6LVlanInit();
#endif
#if 1/*[#43] LF발생시 RF 전달 기능 추가, balkrow, 2024-06-05*/
	if(!result)
		gEag6LSDKInitStatus = GT_TRUE; 
#endif

#ifdef DEBUG
	syslog(LOG_INFO, "cpssInitSystem result %x", result);
#endif
	va_start(argP, args);
	msg.type = va_arg(argP, uint32_t);
	va_end(argP);

	msg.result = FIFO_CMD_SUCCESS; 	
	send_to_sysmon_master(&msg);
	return IPC_CMD_SUCCESS;
}

uint8_t gCpssHello(int args, ...)
{
	sysmon_fifo_msg_t msg;
	msg.type = gHello;		
	send_to_sysmon_master(&msg);
	args = args;
	return IPC_CMD_SUCCESS;
}

#if 1/*[#24] Verifying syncE register update, dustin, 2024-05-28 */
uint8_t gCpssSynceEnable(int args, ...)
{
	uint8_t ret;
	uint8_t portno;
	sysmon_fifo_msg_t msg;

	memset(&msg, 0, sizeof msg);
	msg.type = gSynceEnable;		

	syslog(LOG_INFO, "gCpssSynceEnable (REQ).");
	for(portno = PORT_ID_EAG6L_PORT1; portno < PORT_ID_EAG6L_MAX; portno++) {
		ret = cpssDxChPortRefClockSourceOverrideEnableSet(0/*devNum*/, 
			portno, 1/*overrideEnable*/, 
			CPSS_PORT_REF_CLOCK_SOURCE_SECONDARY_E);
		if(ret != GT_OK) {
			msg.result |= (0x1 < (portno - 1));
			syslog(LOG_INFO, "cpssDxChPortRefClockSourceOverrideEnableSet ret[%d]", ret);
		}
	}

	for(portno = PORT_ID_EAG6L_PORT1; portno < PORT_ID_EAG6L_MAX; portno++) {
		ret = cpssDxChPortSyncEtherRecoveryClkConfigSet(0/*devNum*/, 
			CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK0_E/*recoveryClkType*/, 
			1/*enable*/, 0/*portNum*/, 0/*laneNum*/);
		if(ret != GT_OK) {
			msg.result |= (0x100 < (portno - 1));
			syslog(LOG_INFO, "cpssDxChPortSyncEtherRecoveryClkConfigSet ret[%d]", ret);
		}
	}

	for(portno = PORT_ID_EAG6L_PORT1; portno < PORT_ID_EAG6L_MAX; portno++) {
		ret = cpssDxChPortSyncEtherRecoveryClkDividerValueSet(0/*devNum*/,
			portno, 0/*laneNum*/, 
			CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLOCK_SELECT_0_E,
			CPSS_DXCH_PORT_SYNC_ETHER_RECOVERY_CLK_DIVIDER_8_E);
		if(ret != GT_OK) {
			msg.result |= (0x10000 < (portno - 1));
			syslog(LOG_INFO, "cpssDxChPortSyncEtherRecoveryClkDividerValueSet ret[%d]", ret);
		}
	}

	sprintf(msg.buffer, ">>> gCpssSynceEnable DONE <<<");
	syslog(LOG_INFO, ">>> gCpssSynceEnable DONE total ret[0x%x] <<<", (unsigned int)msg.result);

	/* reply the result */
	send_to_sysmon_master(&msg);
	args = args;
	return IPC_CMD_SUCCESS;
}

uint8_t gCpssSynceDisable(int args, ...)
{
	uint8_t ret;
	uint8_t portno;
	sysmon_fifo_msg_t msg;

	memset(&msg, 0, sizeof msg);
	msg.type = gSynceDisable;		

	syslog(LOG_INFO, "gCpssSynceDisable (REQ) : port[%d].\n", msg.portid);
	for(portno = PORT_ID_EAG6L_PORT1; portno < PORT_ID_EAG6L_MAX; portno++) {
		ret = cpssDxChPortRefClockSourceOverrideEnableSet(0/*devNum*/, 
			portno, 0/*overrideDisable*/, 
			CPSS_PORT_REF_CLOCK_SOURCE_SECONDARY_E);
		/* FIXME : just disabling is OK?? */
		if(ret != GT_OK)
			msg.result |= (0x1 < (portno - 1));
	}

	sprintf(msg.buffer, ">>> gCpssSynceDisable DONE <<<");
	syslog(LOG_INFO, ">>> gCpssSynceDisable DONE <<<");

	/* reply the result */
	send_to_sysmon_master(&msg);
	args = args;
	return IPC_CMD_SUCCESS;
}

uint8_t gCpssSynceIfSelect(int args, ...)
{
	uint8_t ret;
	uint8_t portno;
	va_list argP;
	sysmon_fifo_msg_t msg;

	memset(&msg, 0, sizeof msg);

	va_start(argP, args);
	msg.type = va_arg(argP, uint32_t);
	msg.portid = va_arg(argP, uint32_t);
	msg.portid2 = va_arg(argP, uint32_t);
	va_end(argP);

	syslog(LOG_INFO, "gCpssSynceIfSelect (REQ) : pri[%d] sec[%d].\n", msg.portid, msg.portid2);
	for(portno = PORT_ID_EAG6L_PORT1; portno < PORT_ID_EAG6L_MAX; portno++) {
		ret = cpssDxChPortRefClockSourceOverrideEnableSet(0/*devNum*/, 
			portno, 0/*overrideDisable*/, 
			CPSS_PORT_REF_CLOCK_SOURCE_SECONDARY_E);
		/* FIXME : just disabling is OK?? */
		if(ret != GT_OK)
			msg.result |= (0x1 < (portno - 1));
	}

	sprintf(msg.buffer, ">>> gCpssSynceIfSelect DONE <<<");
	syslog(LOG_INFO, ">>> gCpssSynceIfSelect DONE <<<");

	/* reply the result */
	send_to_sysmon_master(&msg);
	args = args;
	return IPC_CMD_SUCCESS;
}
#endif

uint8_t gCpssPortPM(int args, ...)
{
	va_list argP;
	sysmon_fifo_msg_t msg;

	memset(&msg, 0, sizeof msg);

	va_start(argP, args);
	msg.type = va_arg(argP, uint32_t);
	msg.portid = va_arg(argP, uint32_t);
	va_end(argP);

	/*FIXME : call cpss api*/

	/* copy data to msg.pm */

	sprintf(msg.buffer, ">>> gCpssPortPM DONE <<<");
	syslog(LOG_INFO, ">>> gCpssPortPM DONE port[%d] <<<", msg.portid);

	/* reply the result */
	send_to_sysmon_master(&msg);
	args = args;
	return IPC_CMD_SUCCESS;
}

#if 1/*[#43] LF발생시 RF 전달 기능 추가, balkrow, 2024-06-05*/
uint8_t gCpssLLCFSet(int args, ...)
{
	va_list argP;
	sysmon_fifo_msg_t msg;

	va_start(argP, args);
	msg.type = va_arg(argP, uint32_t);
	msg.portid = va_arg(argP, uint32_t);
	va_end(argP);
	msg.result = FIFO_CMD_SUCCESS; 	

	syslog(LOG_INFO, "LLCFSet enable[%d] <<<", msg.portid);

	if(msg.portid == IPC_FUNC_ON)
	{
		llcf_thread = multi_thread_add_timer (master, svc_fault_fsm_timer, NULL, 1);
		if(!llcf_thread)
			msg.result = FIFO_CMD_FAIL; 	
	}
	else
	{
		multi_thread_cancel(llcf_thread);
	}

	send_to_sysmon_master(&msg);
	return IPC_CMD_SUCCESS;
}
#endif

cCPSSToSysmonFuncs gCpssToSysmonFuncs[] =
{
	gCpssSDKInit,
	gCpssHello,
#if 1/*[#24] Verifying syncE register update, dustin, 2024-05-28 */
	gCpssSynceEnable,
	gCpssSynceDisable,
	gCpssSynceIfSelect,
#endif
#if 1/*[#43] LF발생시 RF 전달 기능 추가, balkrow, 2024-06-05*/
	gCpssLLCFSet,
#endif
	gCpssPortPM,
};

const uint32_t funcsListLen = sizeof(gCpssToSysmonFuncs) / sizeof(cCPSSToSysmonFuncs);

void eag6l_ipc_init(void) {

	umask(0000);
	unlink(SYSMON_FIFO_WRITE);
	unlink(SYSMON_FIFO_READ);

	if(mkfifo(SYSMON_FIFO_WRITE, 0666) != 0)
	{
		gEag6LIPCstate = IPC_INIT_FAIL;
#ifdef DEBUG
#endif
	}
	else
	{
		if((sysrdfifo = open(SYSMON_FIFO_WRITE/*from-master*/, O_RDWR)) < 0)
			gEag6LIPCstate = IPC_INIT_FAIL;
	}

	if(mkfifo(SYSMON_FIFO_READ, 0666) != 0)
	{
		gEag6LIPCstate = IPC_INIT_FAIL;
#ifdef DEBUG
#endif
	}
}
#endif

int sysmon_slave_recv_fifo(struct multi_thread *thread)
{
	int len = 0;
	sysmon_fifo_msg_t req;

	memset(&req, 0, sizeof req);
	len = read(MULTI_THREAD_FD (thread),&req, sizeof(sysmon_fifo_msg_t));
	if (len < 0)
	{
		syslog(LOG_ERR, "%s : Receive len %x error", __func__, len);
		goto next_time;
	}

#if 1/*[#34] aldrin3s chip initial 기능 추가, balkrow, 2024-05-23*/
	if(req.type > funcsListLen || req.type < 0) 
	{
		syslog(LOG_ERR, "%s : Receive msg %x error", __func__, req.type);
		goto next_time;
	}
  
#if 1/*[#24] Verifying syncE register update, dustin, 2024-05-28 */
	switch(req.type) 
    {
        case gSDKInit:
        case gHello:
        case gSynceEnable:
        case gSynceDisable:
		gCpssToSysmonFuncs[req.type](1, req.type);
		break;
        case gSynceIfSelect:
			gCpssToSysmonFuncs[req.type](3, req.type, req.portid, req.portid2);
            break;
        case gPortPM:
#if 1/*[#43] LF발생시 RF 전달 기능 추가, balkrow, 2024-06-05*/
	case gLLCFSet:
			gCpssToSysmonFuncs[req.type](2, req.type, req.portid);
#endif
            break;
	default:
            syslog(LOG_INFO,"%s : default? (REQ) : type[%d]", __func__, req.type);
			gCpssToSysmonFuncs[req.type](1, req.type);
	}
#endif
#else
	/**TODO: define request type, process**/
	sysmon_slave_system_command(&req);
#endif
next_time:
	multi_thread_add_read (master, sysmon_slave_recv_fifo, NULL, MULTI_THREAD_FD (thread));
	return 0;
}

#if 1/*[#34] aldrin3s chip initial 기능 추가, balkrow, 2024-05-23*/
int sysmon_slave_fifo_recv_init (void )
{
	if(gEag6LIPCstate == IPC_INIT_SUCCESS)
		multi_thread_add_read (master, sysmon_slave_recv_fifo, NULL, sysrdfifo);
	return 0;
}
#else
int sysmon_slave_fifo_recv_init (void )
{
	umask(0000);
	(void) unlink(SYSMON_FIFO_WRITE);

	if(mkfifo(SYSMON_FIFO_WRITE, 0666) != 0)
	{
		return 1;
	}

	if( (syswrfifo = open (SYSMON_FIFO_WRITE, O_RDWR)) < 0)
		return 1;

	multi_thread_add_read (master, sysmon_slave_recv_fifo, NULL, syswrfifo);

	return 0;
}
#endif

#if 1/*[#43] LF발생시 RF 전달 기능 추가, balkrow, 2024-06-05*/
SVC_FAULT_ST fault_st_transition
(
 SVC_FAULT_ST state,
 SVC_FAULT_EVT evt
) 
{
	SVC_FAULT_ST rc = SVC_FAULT_ST_INIT;
	switch (state) {
	case SVC_FAULT_ST_INIT:
		if(evt == SVC_FAULT_EVT_LINK_DOWN)
			rc = SVC_FAULT_ST_LFRF_CHECK;
		else if(evt == SVC_FAULT_EVT_LINK_UP)
			rc = SVC_FAULT_ST_INIT;

		break;
	case SVC_FAULT_ST_LFRF_CHECK:
		if(evt == SVC_FAULT_EVT_LINK_DOWN)
			rc = SVC_FAULT_ST_LFRF_CHECK;
		else if(evt == SVC_FAULT_EVT_LINK_UP)
			rc = SVC_FAULT_ST_INIT;
		break;
	case SVC_FAULT_ST_LF:
		if(evt == SVC_FAULT_EVT_LINK_DOWN)
			rc = SVC_FAULT_ST_LF;
		else if(evt == SVC_FAULT_EVT_LINK_UP)
			rc = SVC_FAULT_ST_INIT;
		break;
	case SVC_FAULT_ST_FWD_LF:
		if(evt == SVC_FAULT_EVT_LINK_DOWN)
			rc = SVC_FAULT_ST_FWD_LF;
		else if(evt == SVC_FAULT_EVT_LINK_UP)
			rc = SVC_FAULT_ST_CLR_LF;
		break;
	case SVC_FAULT_ST_RF:
		if(evt == SVC_FAULT_EVT_LINK_DOWN)
			rc = SVC_FAULT_ST_RF;
		else if(evt == SVC_FAULT_EVT_LINK_UP)
			rc = SVC_FAULT_ST_INIT;
		break;
	case SVC_FAULT_ST_FWD_RF:
		if(evt == SVC_FAULT_EVT_LINK_DOWN)
			rc = SVC_FAULT_ST_FWD_RF;
		else if(evt == SVC_FAULT_EVT_LINK_UP)
			rc = SVC_FAULT_ST_CLR_RF;
		break;
	case SVC_FAULT_ST_CLR_LF:
		if(evt == SVC_FAULT_EVT_LINK_DOWN)
			rc = SVC_FAULT_ST_LF;
		else if(evt == SVC_FAULT_EVT_LINK_UP)
			rc = SVC_FAULT_ST_CLR_LF;
		break;
	case SVC_FAULT_ST_CLR_RF:
		if(evt == SVC_FAULT_EVT_LINK_DOWN)
			rc = SVC_FAULT_ST_RF;
		else if(evt == SVC_FAULT_EVT_LINK_UP)
			rc = SVC_FAULT_ST_CLR_RF;
		break;
	case SVC_FAULT_ST_NO_LFRF:
		if(evt == SVC_FAULT_EVT_LINK_DOWN)
			rc = SVC_FAULT_ST_NO_LFRF;
		else if(evt == SVC_FAULT_EVT_LINK_UP)
			rc = SVC_FAULT_ST_INIT;
		break;
	default:
		break;
	}
	return rc;
}

int32_t svc_fault_fsm_timer(struct multi_thread *thread)
{
	uint8_t i;
	SVC_FAULT_ST state, next_state;
	SVC_FAULT_ST evt;
	CPSS_PORT_MANAGER_STATUS_STC portConfigOutParams;
	GT_U8 devNum = 0;

	thread = thread;

	if(gEag6LSDKInitStatus == GT_FALSE)
		goto next_timer;

	for(i = 0; i < eag6LPortArrSize; i++)
	{

		cpssDxChPortManagerStatusGet(devNum, eag6LPortlist[i], &portConfigOutParams);

		evt = (portConfigOutParams.portState == CPSS_PORT_MANAGER_STATE_LINK_UP_E) ? 
			SVC_FAULT_EVT_LINK_UP : SVC_FAULT_EVT_LINK_DOWN;

		state = fault_st_transition(svcPortFaultFsm[i].state, evt);

		next_state = svcPortFaultFsm[i].cb[state](i);
#ifdef DEBUG
		syslog(LOG_INFO, "port=%d, state=%d, evt=%d", eag6LPortlist[i], state, evt);
#endif
		svcPortFaultFsm[i].state = next_state;
		svcPortFaultFsm[i].evt = evt;
	}
next_timer:	
	multi_thread_add_timer_msec(master, svc_fault_fsm_timer, NULL, 200);
	return 0;
}

#endif

void sysmon_slave_init (void) 
{

	master = multi_thread_make_master();
#if 1/*[#34] aldrin3s chip initial 기능 추가, balkrow, 2024-05-23*/
	eag6l_ipc_init();
#endif
	sysmon_slave_fifo_recv_init();
#if 1/*[#43] LF발생시 RF 전달 기능 추가, balkrow, 2024-06-05*/
	initFaultFsmList ();
#endif
}

static unsigned __TASKCONV OAM_thread(void)
{
	struct multi_thread thread;

#if 1/*[#34] aldrin3s chip initial 기능 추가, balkrow, 2024-05-23*/
	openlog("EAG6LIPC", LOG_PID | LOG_CONS| LOG_NDELAY | LOG_PID, LOG_USER);
#endif
#if 1/*[#43] LF발생시 RF 전달 기능 추가, balkrow, 2024-06-05*/
	notifyEventToIPC = processPortEvt;
#endif
	sysmon_slave_init();

	syslog(LOG_NOTICE, "Start EAG6L IPC Thread");
	while (multi_thread_fetch (master, &thread))
		multi_thread_call (&thread);

#ifdef __GNUC__
    /* to avoid warnings in gcc */
    return 0;
#endif
}



GT_STATUS eag6lOAMstart(void)
{
    GT_TASK   tid;

    if (cmdOsTaskCreate(
                "OAMSupport",
                6,                      /* thread priority          */
                8192,                   /* use default stack size   */
                (unsigned (__TASKCONV *)(void*))OAM_thread,
                NULL,
                &tid) != GT_OK)
    {
        cmdOsPrintf("commander: failed to create OAM task\n");
        return GT_FAIL;
    }
    osTaskGracefulCallerRegister(tid, GT_TRUE, NULL, NULL);

    return GT_OK;
}


