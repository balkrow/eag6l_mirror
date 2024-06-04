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

#if 1/*[#24] Verifying syncE register update, dustin, 2024-05-28 */
uint8_t gCpssSynceEnable(int args, ...)
{
	va_list argP;
	sysmon_fifo_msg_t msg;
#ifdef DEBUG
	zlog_notice("called %s args=%d", __func__, args);
#endif

	memset(&msg, 0, sizeof msg);

	if(args != 1)
		return IPC_CMD_FAIL;

	va_start(argP, args);
        msg.type = va_arg(argP, uint32_t);	
	va_end(argP);
zlog_notice("gCpssSynceEnable : type[%d]", msg.type);/*ZZPP*/

	if(send_to_sysmon_slave(&msg) == 0) {
		zlog_notice("%s : send_to_sysmon_slave() has failed.", __func__);
		return IPC_CMD_FAIL;
	}

	return IPC_CMD_SUCCESS;
}

uint8_t gCpssSynceDisable(int args, ...)
{
	va_list argP;
	sysmon_fifo_msg_t msg;
#ifdef DEBUG
	zlog_notice("called %s args=%d", __func__, args);
#endif

	memset(&msg, 0, sizeof msg);

	if(args != 1)
		return IPC_CMD_FAIL;

	va_start(argP, args);
		msg.type = va_arg(argP, uint32_t);
	va_end(argP);
zlog_notice("gCpssSynceDisable : type[%d]", msg.type);/*ZZPP*/

	if(send_to_sysmon_slave(&msg) == 0) {
		zlog_notice("%s : send_to_sysmon_slave() has failed.", __func__);
		return IPC_CMD_FAIL;
	}

	return IPC_CMD_SUCCESS;
}

uint8_t gCpssSynceIfSelect(int args, ...)
{
	va_list argP;
	sysmon_fifo_msg_t msg;
#ifdef DEBUG
	zlog_notice("called %s args=%d", __func__, args);
#endif

	memset(&msg, 0, sizeof msg);

	if(args != 3)
		return IPC_CMD_FAIL;

	va_start(argP, args);
        msg.type = va_arg(argP, uint32_t);	
		msg.portid  = va_arg(argP, uint32_t);
		msg.portid2 = va_arg(argP, uint32_t);
	va_end(argP);
zlog_notice("gCpssSynceIfSelect : type[%d] pri[%d] sec[%d]", msg.type, msg.portid, msg.portid2);/*ZZPP*/

	if(send_to_sysmon_slave(&msg) == 0) {
		zlog_notice("%s : send_to_sysmon_slave() has failed.", __func__);
		return IPC_CMD_FAIL;
	}

	return IPC_CMD_SUCCESS;
}

#if 1/*[#44] Add a callback templates for compiling, dustin, 2024-06-04 */
uint8_t gCpssPortSetRate(int args, ...)
{
    va_list argP;
    sysmon_fifo_msg_t *msg = NULL;

    va_start(argP, args);
    msg = va_arg(argP, sysmon_fifo_msg_t *);
    va_end(argP);

    /* FIXME : call sdk api to set port rate/mode. */

    /* reply the result */
    send_to_sysmon_slave(msg);
    return IPC_CMD_SUCCESS;
}

uint8_t gCpssPortESMCenable(int args, ...)
{
    va_list argP;
    sysmon_fifo_msg_t *msg = NULL;

    va_start(argP, args);
    msg = va_arg(argP, sysmon_fifo_msg_t *);
    va_end(argP);

    /* FIXME : call sdk api to en/disable per-port ESMC state. */

    /* reply the result */
    send_to_sysmon_slave(msg);
    return IPC_CMD_SUCCESS;
}

uint8_t gCpssPortAlarm(int args, ...)
{
    va_list argP;
    sysmon_fifo_msg_t *msg = NULL;

    va_start(argP, args);
    msg = va_arg(argP, sysmon_fifo_msg_t *);
    va_end(argP);

    /* FIXME : call sdk api to get alarm status. */

    /* reply the result */
    send_to_sysmon_slave(msg);
    return IPC_CMD_SUCCESS;
}
#endif

uint8_t gCpssPortPM(int args, ...)
{
	va_list argP;
	sysmon_fifo_msg_t msg;
#ifdef DEBUG
	zlog_notice("called %s args=%d", __func__, args);
#endif

	memset(&msg, 0, sizeof msg);

	if(args != 2)
		return IPC_CMD_FAIL;

	va_start(argP, args);
        msg.type = va_arg(argP, uint32_t);	
		msg.portid = va_arg(argP, uint32_t);
	va_end(argP);
zlog_notice("gCpssPortPM : type[%d] port[%d]", msg.type, msg.portid);/*ZZPP*/

	if(send_to_sysmon_slave(&msg) == 0) {
		zlog_notice("%s : send_to_sysmon_slave() has failed.", __func__);
		return IPC_CMD_FAIL;
	}

	return IPC_CMD_SUCCESS;
}
#endif

#if 1/*[#34] aldrin3s chip initial 옜 옜, balkrow, 2024-05-23*/
static int sysmon_master_hello_test(struct thread *thread)
{
    int len = 0;
    sysmon_fifo_msg_t msg;

    thread_add_timer (master, sysmon_master_hello_test, NULL, 10);

#if 0//PWY_FIXME
	memset(&msg, 0, sizeof msg);
	msg.type = sysmon_cmd_fifo_hello_test;
	strcpy(msg.noti_msg, "Hello~ THIS IS TEST");

	send_to_sysmon_slave(&msg);
#else/////////////////////////////////////////////
int synce_config_set_admin(int enable);
int synce_config_set_if_select(int pri_port, int sec_port);

zlog_notice(" sysmon_master_hello_test~!\n");/*ZZPP*/
	synce_config_set_admin(1);
	synce_config_set_if_select(1, 2);
#endif //PWY_FIXME
    return 0;
}
#endif

cSysmonToCPSSFuncs gSysmonToCpssFuncs[] =
{
	gCpssSDKInit,	
	gCpssHello,	
#if 1/*[#24] Verifying syncE register update, dustin, 2024-05-28 */
	gCpssSynceEnable,
	gCpssSynceDisable,
	gCpssSynceIfSelect,
#endif
#if 1/*[#25] cpss � sysmon � interface 옜 옜, balkrow, 2024-05-30 */
	gCpssPortSetRate,
	gCpssPortESMCenable, /*further implements*/
	gCpssPortAlarm, /* Link Down/up */
#endif
	gCpssPortPM,
};

const uint32_t funcsListLen = sizeof(gSysmonToCpssFuncs) / sizeof(cSysmonToCPSSFuncs);
/*define recv callback function */



uint8_t gAppDemoIPCstate = IPC_INIT_SUCCESS; 
int32_t syscmdrdfifo;
int32_t syscmdwrfifo;
#endif

#if 1/*[#24] Verifying syncE register update, dustin, 2024-05-28 */
int synce_config_set_admin(int enable)
{
	/* use marvell sdk to set synce enable/disable. */
	if(enable == 1)
		gSysmonToCpssFuncs[gSynceEnable](1, gSynceEnable);
	else
		gSysmonToCpssFuncs[gSynceDisable](1, gSynceDisable);
	return 0;
}

int synce_config_set_if_select(int pri_port, int sec_port)
{
	/* use marvell sdk to set synce if select. */
	gSysmonToCpssFuncs[gSynceIfSelect](3, gSynceIfSelect, pri_port, sec_port);
	return 0;
}
#endif

int update_port_pm_counters(void)
{
	/* use marvell sdk to get pm counters. */
	return 0;
}

int send_to_sysmon_slave(sysmon_fifo_msg_t * msg)
{
#if 0/*[#4] register updating : opening file at both sides don't work, dustin, 2024-05-28 */
	/*[#34] aldrin3s chip initial 옜 옜, balkrow, 2024-05-23*/

#ifdef DEBUG
	zlog_notice("%s : msg %x send to cpss", __func__, msg->type);
#endif
	return write(syscmdwrfifo, msg, sizeof(sysmon_fifo_msg_t));
#else
	int syscmdwrfifo = 0;
	int retry, len;

	retry = 0;
	/* retry 5 times to open to write. */
	while((syscmdwrfifo = open(SYSMON_FIFO_WRITE/*to-slave*/, O_RDWR)) < 0) {
		if(retry++ < 5)
			continue;
		return 0;
	}

	len = write(syscmdwrfifo, msg, sizeof(sysmon_fifo_msg_t));

	close(syscmdwrfifo);

	return len;
#endif
}

static int sysmon_master_system_command(sysmon_fifo_msg_t * msg)
{
#if 1/*[#34] aldrin3s chip initial 옜 옜, balkrow, 2024-05-23*/
#ifdef DEBUG
	zlog_info("%s recv msg %x from cpss", __func__, msg->type);
#endif
#if 1/*[#24] Verifying syncE register update, dustin, 2024-05-28 */
	switch(msg->type)
	{
		case gSDKInit:
			break;
		case gHello:
			zlog_notice("gHello (REPLY) : %s\n", 
					msg->buffer[0] ? msg->buffer : "N/A");
			break;
		case gSynceEnable:
			zlog_notice("gSynceEnable (REPLY) : port[%d].\n", msg->portid);
			break;
		case gSynceDisable:
			zlog_notice("gSynceDisable (REPLY) : port[%d].\n", msg->portid);
			break;
		case gSynceIfSelect:
			zlog_notice("gSynceIfSelect (REPLY) : port[%d].\n", msg->portid);
			break;
		case gPortPM:
			zlog_notice("gPortPM (REPLY) : port[%d].\n", msg->portid);
			break;
			//TODO

		default:
			break;
	}
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

void sysmon_master_fifo_init (void)
{
#if 0/*[#4] register updating : opening file at both sides don't work, dustin, 2024-05-28 */
	/*[#34] aldrin3s chip initial 옜 옜, balkrow, 2024-05-23*/
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

#if 1//PWY_FIXME
	// for testing.
	thread_add_timer(master, sysmon_master_hello_test, NULL, 10);
#endif //PWY_FIXME

	return;
}
