#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <cmdShell/common/cmdCommon.h>
#include <cmdShell/FS/cmdFS.h>
#include <gtOs/gtOsTask.h>
#include <cpss/dxCh/dxChxGen/port/cpssDxChPortSyncEther.h>
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
#else
#include "../../../../../quagga-1.2.4/sysmon/sys_fifo.h"
#endif


int sysrdfifo;
int syswrfifo;

struct multi_thread_master *master;

#if 1/*[#34] aldrin3s chip initial 기능 추가, balkrow, 2024-05-23*/
#define DEBUG
uint8_t gEag6LIPCstate = IPC_INIT_SUCCESS; 

extern GT_STATUS cpssInitSystem
(
    IN  GT_U32  boardIdx,
    IN  GT_U32  boardRevId,
    IN  GT_U32  reloadEeprom
);


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

cCPSSToSysmonFuncs gCpssToSysmonFuncs[] =
{
	gCpssSDKInit,
	gCpssHello,
#if 1/*[#24] Verifying syncE register update, dustin, 2024-05-28 */
	gCpssSynceEnable,
	gCpssSynceDisable,
	gCpssSynceIfSelect,
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
#if 0/*[#4] register updating : opening file at both sides don't work, dustin, 2024-05-28 */
	else
	{
		if((syswrfifo = open(SYSMON_FIFO_READ/*to-master*/, O_RDWR)) < 0)
			gEag6LIPCstate = IPC_INIT_FAIL;
	}
#endif/*PWY_FIXME*/
}
#endif

#if 0
static int sysmon_slave_system_command(sysmon_fifo_msg_t * msg)
{
    if(msg->type > sysmon_cmd_fifo_test && msg->type < sysmon_cmd_fifo_max)
    {
        switch(msg->type)
        {
#if 1/*[#34] aldrin3s chip initial 기능 추가, balkrow, 2024-05-23*/
	case sysmon_cmd_fifo_sdk_init:
	cpssInitSystem(38, 1, 0);
	msg->result = FIFO_CMD_SUCCESS; 	
	send_to_sysmon_master(msg);
#ifdef DEBUG
	syslog(LOG_INFO, " SDK init %s\n", msg->result == FIFO_CMD_SUCCESS ? "Success" : "Fail");
#endif
	break;
#endif
	case sysmon_cmd_fifo_hello_test:
		syslog(LOG_INFO, "sysmon_cmd_fifo_hello_test (REQ) : %s\n", msg->noti_msg);

		strcpy(msg->buffer, ">>> TEST DONE <<<");
		send_to_sysmon_master(msg);
		break;
	case sysmon_cmd_fifo_sftp_get:
		syslog(LOG_INFO, "sysmon_cmd_fifo_sftp_get (REQ) :port[%d].\n", msg->portid);
		break;
	case sysmon_cmd_fifo_sftp_set:
		syslog(LOG_INFO, "sysmon_cmd_fifo_sftp_se (REQ) : port[%d].\n", msg->portid);
		break;

		/*TODO*/

	default:
		break;
	}
    }
	return 0;
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
			gCpssToSysmonFuncs[req.type](1, req.type);
            break;

        case gHello:
			gCpssToSysmonFuncs[req.type](1, req.type);
            break;
        case gSynceEnable:
			gCpssToSysmonFuncs[req.type](1, req.type);
            break;
        case gSynceDisable:
			gCpssToSysmonFuncs[req.type](1, req.type);
            break;
        case gSynceIfSelect:
			gCpssToSysmonFuncs[req.type](3, req.type, req.portid, req.portid2);
            break;
        case gPortPM:
			gCpssToSysmonFuncs[req.type](2, req.type, req.portid);
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

void sysmon_slave_init (void) 
{
	master = multi_thread_make_master();
#if 1/*[#34] aldrin3s chip initial 기능 추가, balkrow, 2024-05-23*/
	eag6l_ipc_init();
#endif
	sysmon_slave_fifo_recv_init();
}

#if 0/*test-code*/
int _echo_to_terminal (struct multi_thread *thread)
{
	thread = thread;
    cmdOsPrintf("sysmon_slave_task run\n");
	multi_thread_add_timer (master, _echo_to_terminal, NULL, 10);
	return 0;
}
#endif/*test-code*/

static unsigned __TASKCONV OAM_thread(void)
{
	struct multi_thread thread;

#if 1/*[#34] aldrin3s chip initial 기능 추가, balkrow, 2024-05-23*/
	openlog("EAG6LIPC", LOG_PID | LOG_CONS| LOG_NDELAY | LOG_PID, LOG_USER);
#endif
	sysmon_slave_init();

#if 0/*test-code*/
	/*add timer*/
	multi_thread_add_timer (master, _echo_to_terminal, NULL, 10);
#endif/*test-code*/

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


