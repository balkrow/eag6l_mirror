/* 
	Copyright ¨I 2014, by HFR,Inc. All Right Reserved.

	This Software of copyright is reserved by HFR.Inc. No all or part of this Software may be reproduced by  the 
	hard-copy format, electronically, or otherwise and transmitted to Ineligible Person. If not you should be subject 
	to civil suit and criminal case according to circumstances.


       본 파일의 저작권은 (주)에치에프알에 있습니다. (주)에치에프알의 명시적인 동의없이 본 파일의 전체 또는 일부를 
       하드카피 형식, 전자적 방식 또는 기타방식으로 재생산하거나 받아볼 수 있는 자격이 없는 사람에게 재배포하는 행위는
       저작권법을 위반하는 것이며 민사상 손해배상 그리고 해당하는 경우에는 형사상 처벌의 대상이 됩니다.
*/	


#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include "thread.h"

#include "sys_fifo.h"

#include <dirent.h>

#if 1/*[#34] aldrin3s chip initial 옜 옜, balkrow, 2024-05-23*/
#define DEBUG
#endif

extern struct thread_master *master;

/*define send callback function */
#if 1/*[#34] aldrin3s chip initial 옜 옜, balkrow, 2024-05-23*/
uint8_t gCpssSDKInit(int args, ...)
{
	uint8_t ret = IPC_CMD_SUCCESS;
	va_list argP;
	sysmon_fifo_msg_t msg;

#ifdef DEBUG
	zlog_notice("called %s args=%d", __func__, args);
#endif
	if(args !=  1)
		return IPC_CMD_FAIL;

	va_start(argP, args);
        msg.type = va_arg(argP, uint32_t);	
	va_end(argP);
	
	if(send_to_sysmon_slave(&msg) == 0)
		ret = IPC_CMD_FAIL;

	return ret;
}

uint8_t gCpssHello(int args, ...)
{
	uint8_t ret = IPC_CMD_SUCCESS;
	va_list argP;
	sysmon_fifo_msg_t msg;
#ifdef DEBUG
	zlog_notice("called %s args=%d", __func__, args);
#endif

	if(args !=  1)
		return IPC_CMD_FAIL;

	va_start(argP, args);
        msg.type = va_arg(argP, uint32_t);	
	va_end(argP);
	
	if(send_to_sysmon_slave(&msg) == 0)
		ret = IPC_CMD_FAIL;

	return ret;
}

cSysmonToCPSSFuncs gSysmonToCpssFuncs[] =
{
	gCpssSDKInit,	
	gCpssHello,	
};

const uint32_t funcsListLen = sizeof(gSysmonToCpssFuncs) / sizeof(cSysmonToCPSSFuncs);
/*define recv callback function */



uint8_t gAppDemoIPCstate = IPC_INIT_SUCCESS; 
int32_t syscmdrdfifo;
int32_t syscmdwrfifo;
#endif

int send_to_sysmon_slave(sysmon_fifo_msg_t * msg)
{
#if 1/*[#34] aldrin3s chip initial 옜 옜, balkrow, 2024-05-23*/

#ifdef DEBUG
	zlog_notice("msg %x send to cpss", msg->type);
#endif
	return write(syscmdwrfifo, msg, sizeof(sysmon_fifo_msg_t));
#else
	int syscmdwrfifo = 0;

	if((syscmdwrfifo = open(SYSMON_FIFO_WRITE/*to-slave*/, O_RDWR)) < 0)
		return 1;

	write(syscmdwrfifo, msg, sizeof(sysmon_fifo_msg_t));

	close(syscmdwrfifo);

	return 0;
#endif
}

static int sysmon_master_system_command(sysmon_fifo_msg_t * msg)
{
#if 1/*[#34] aldrin3s chip initial 옜 옜, balkrow, 2024-05-23*/
#ifdef DEBUG
	zlog_info("recv msg %x from cpss", msg->type);
#endif
#else
	if(msg->type > sysmon_cmd_fifo_test && msg->type < sysmon_cmd_fifo_max)   
	{
		switch(msg->type)
		{
			case sysmon_cmd_fifo_hello_test:
				print_console("sysmon_cmd_fifo_hello_test (REPLY) : %s\n", 
					msg->buffer[0] ? msg->buffer : "N/A");
				break;
			case sysmon_cmd_fifo_sftp_get:
				print_console("sysmon_cmd_fifo_sftp_get (REPLY) : port[%d].\n", msg->portid);
				break;
			case sysmon_cmd_fifo_sftp_set:
				print_console("sysmon_cmd_fifo_sftp_set (REPLY) : port[%d].\n", msg->portid);
				break;

			//TODO

			default:
				break;
		}
	}
#endif
	return 0;
}

static int sysmon_master_recv_fifo(struct thread *thread)
{
	int len = 0;
	sysmon_fifo_msg_t msg;

	len = read(THREAD_FD(thread), &msg, sizeof(sysmon_fifo_msg_t));
	if (len < 0) 
	{
		thread_add_read (master, sysmon_master_recv_fifo, NULL, THREAD_FD(thread));
		return 0;
	}

	sysmon_master_system_command(&msg);

	thread_add_read (master, sysmon_master_recv_fifo, NULL, THREAD_FD(thread));
	return 0;
}

#if 0/*[#34] aldrin3s chip initial 옜 옜, balkrow, 2024-05-23*/
static int sysmon_master_hello_test(struct thread *thread)
{
    int len = 0;
    sysmon_fifo_msg_t msg;

    thread_add_timer (master, sysmon_master_hello_test, NULL, 3);

	memset(&msg, 0, sizeof msg);
	msg.type = sysmon_cmd_fifo_hello_test;
	strcpy(msg.noti_msg, "Hello~ THIS IS TEST");

	send_to_sysmon_slave(&msg);
    return 0;
}
#endif

void sysmon_master_fifo_init (void)
{
#if 1/*[#34] aldrin3s chip initial 옜 옜, balkrow, 2024-05-23*/
	if((syscmdrdfifo = open (SYSMON_FIFO_READ, O_RDWR)) < 0)
		gAppDemoIPCstate = IPC_INIT_FAIL;

	if((syscmdwrfifo = open (SYSMON_FIFO_WRITE, O_RDWR)) < 0)
		gAppDemoIPCstate = IPC_INIT_FAIL;
#else
	int syscmdrdfifo;

	umask(0000);
	(void) unlink(SYSMON_FIFO_READ);

	if(mkfifo(SYSMON_FIFO_READ, 0666) != 0)
		return;
	if((syscmdrdfifo = open (SYSMON_FIFO_READ, O_RDWR)) < 0)
		return;
#endif

	thread_add_read(master, sysmon_master_recv_fifo, NULL, syscmdrdfifo);

#if 0//PWY_FIXME
	// for testing.
	thread_add_timer(master, sysmon_master_hello_test, NULL, 3);
#endif //PWY_FIXME

	return;
}
