#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <cmdShell/common/cmdCommon.h>
#include <cmdShell/FS/cmdFS.h>
#include <gtOs/gtOsTask.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <netinet/in.h>

#include "multi_thread.h"
#if 1/*[#34] aldrin3s chip initial 기능 추가, balkrow, 2024-05-23*/
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
	syslog(LOG_INFO, "Send msg %x", msg->type);
#endif
	return write(syswrfifo, msg, sizeof(sysmon_fifo_msg_t));
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

cCPSSToSysmonFuncs gCpssToSysmonFuncs[] =
{
	        gCpssSDKInit,
		gCpssHello,
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
		if((sysrdfifo = open(SYSMON_FIFO_WRITE/*to-master*/, O_RDWR)) < 0)
			gEag6LIPCstate = IPC_INIT_FAIL;
	}

	if(mkfifo(SYSMON_FIFO_READ, 0666) != 0)
	{
		gEag6LIPCstate = IPC_INIT_FAIL;
#ifdef DEBUG
#endif
	}
	else
	{
		if((syswrfifo = open(SYSMON_FIFO_READ/*to-master*/, O_RDWR)) < 0)
			gEag6LIPCstate = IPC_INIT_FAIL;
	}
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
	cmdOsPrintf(" SDK init %s\n", msg->result == FIFO_CMD_SUCCESS ? "Success" : "Fail");
#endif
	break;
#endif
	case sysmon_cmd_fifo_hello_test:
		cmdOsPrintf("sysmon_cmd_fifo_hello_test (REQ) : %s\n", msg->noti_msg);

		strcpy(msg->buffer, ">>> TEST DONE <<<");
		send_to_sysmon_master(msg);
		break;
	case sysmon_cmd_fifo_sftp_get:
		cmdOsPrintf("sysmon_cmd_fifo_sftp_get (REQ) :port[%d].\n", msg->portid);
		break;
	case sysmon_cmd_fifo_sftp_set:
		cmdOsPrintf("sysmon_cmd_fifo_sftp_se (REQ) : port[%d].\n", msg->portid);
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
		syslog(LOG_ERR, "Receive len %x error", len);
		goto next_time;
	}

#if 1/*[#34] aldrin3s chip initial 기능 추가, balkrow, 2024-05-23*/
	if(req.type > funcsListLen || req.type < 0) 
	{
		syslog(LOG_ERR, "Receive msg %x error", req.type);
		goto next_time;
	}
	gCpssToSysmonFuncs[req.type](1, req.type);
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


