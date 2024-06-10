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

#include "sysmon.h"
#include "sys_fifo.h"

#include <dirent.h>

#if 1/*[#34] aldrin3s chip initial 옜 옜, balkrow, 2024-05-23*/
#define DEBUG
#endif

extern struct thread_master *master;
extern struct port_status PORT_STATUS[PORT_ID_EAG6L_MAX];
extern port_pm_counter_t PM_TBL[PORT_ID_EAG6L_MAX];


/*define send callback function */
#if 1/*[#34] aldrin3s chip initial 옜 옜, balkrow, 2024-05-23*/
uint8_t gCpssSDKInit(int args, ...)
{
	uint8_t ret = IPC_CMD_SUCCESS;
	va_list argP;
	sysmon_fifo_msg_t *msg = NULL;

#ifdef DEBUG
	zlog_notice("called %s args=%d", __func__, args);
#endif
	if(args !=  1)
		return IPC_CMD_FAIL;

	va_start(argP, args);
	msg = va_arg(argP, sysmon_fifo_msg_t *);	
	va_end(argP);
	
	if(send_to_sysmon_slave(msg) == 0)
		ret = IPC_CMD_FAIL;

	return ret;
}

uint8_t gCpssHello(int args, ...)
{
	uint8_t ret = IPC_CMD_SUCCESS;
	va_list argP;
	sysmon_fifo_msg_t *msg = NULL;
#ifdef DEBUG
	zlog_notice("called %s args=%d", __func__, args);
#endif

	if(args !=  1)
		return IPC_CMD_FAIL;

	va_start(argP, args);
	msg = va_arg(argP, sysmon_fifo_msg_t *);	
	va_end(argP);
	
	if(send_to_sysmon_slave(msg) == 0)
		ret = IPC_CMD_FAIL;

	return ret;
}

#if 1/*[#24] Verifying syncE register update, dustin, 2024-05-28 */
uint8_t gCpssSynceEnable(int args, ...)
{
	va_list argP;
	sysmon_fifo_msg_t *msg = NULL;
#ifdef DEBUG
	zlog_notice("called %s args=%d", __func__, args);
#endif

	if(args != 1)
		return IPC_CMD_FAIL;

	va_start(argP, args);
	msg = va_arg(argP, sysmon_fifo_msg_t *);	
	va_end(argP);

	if(send_to_sysmon_slave(msg) == 0) {
		zlog_notice("%s : send_to_sysmon_slave() has failed.", __func__);
		return IPC_CMD_FAIL;
	}

	return IPC_CMD_SUCCESS;
}

uint8_t gCpssSynceDisable(int args, ...)
{
	va_list argP;
	sysmon_fifo_msg_t *msg = NULL;
#ifdef DEBUG
	zlog_notice("called %s args=%d", __func__, args);
#endif

	if(args != 1)
		return IPC_CMD_FAIL;

	va_start(argP, args);
	msg = va_arg(argP, sysmon_fifo_msg_t *);
	va_end(argP);

	if(send_to_sysmon_slave(msg) == 0) {
		zlog_notice("%s : send_to_sysmon_slave() has failed.", __func__);
		return IPC_CMD_FAIL;
	}

	return IPC_CMD_SUCCESS;
}

uint8_t gCpssSynceIfSelect(int args, ...)
{
	va_list argP;
	sysmon_fifo_msg_t *msg = NULL;
#ifdef DEBUG
	zlog_notice("called %s args=%d", __func__, args);
#endif

	if(args != 1)
		return IPC_CMD_FAIL;

	va_start(argP, args);
	msg = va_arg(argP, sysmon_fifo_msg_t *);	
	va_end(argP);

	if(send_to_sysmon_slave(msg) == 0) {
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
#ifdef DEBUG
    zlog_notice("called %s args=%d", __func__, args);
#endif

    if(args != 1) {
		syslog(LOG_INFO, "%s: invalid args[%d].", __func__, args);
        return IPC_CMD_FAIL;
	}

    va_start(argP, args);
	msg = va_arg(argP, sysmon_fifo_msg_t *);
    va_end(argP);

    if(send_to_sysmon_slave(msg) == 0) {
        zlog_notice("%s : send_to_sysmon_slave() has failed.", __func__);
        return IPC_CMD_FAIL;
    }

	return IPC_CMD_SUCCESS;
}

uint8_t gCpssPortESMCenable(int args, ...)
{
    va_list argP;
    sysmon_fifo_msg_t *msg = NULL;
#ifdef DEBUG
    zlog_notice("called %s args=%d", __func__, args);
#endif

    if(args != 1) {
		syslog(LOG_INFO, "%s: invalid args[%d].", __func__, args);
        return IPC_CMD_FAIL;
	}

    va_start(argP, args);
	msg = va_arg(argP, sysmon_fifo_msg_t *);
    va_end(argP);

    if(send_to_sysmon_slave(msg) == 0) {
        zlog_notice("%s : send_to_sysmon_slave() has failed.", __func__);
        return IPC_CMD_FAIL;
    }

	return IPC_CMD_SUCCESS;
}

uint8_t gCpssPortAlarm(int args, ...)
{
    va_list argP;
    sysmon_fifo_msg_t *msg = NULL;
#ifdef DEBUG
    zlog_notice("called %s args=%d", __func__, args);
#endif

    if(args != 1) {
		syslog(LOG_INFO, "%s: invalid args[%d].", __func__, args);
        return IPC_CMD_FAIL;
	}

    va_start(argP, args);
	msg = va_arg(argP, sysmon_fifo_msg_t *);
    va_end(argP);

    if(send_to_sysmon_slave(msg) == 0) {
        zlog_notice("%s : send_to_sysmon_slave() has failed.", __func__);
        return IPC_CMD_FAIL;
    }

	return IPC_CMD_SUCCESS;
}

#if 1/*[#43] LF¿¿¿ RF ¿¿ ¿¿ ¿¿, balkrow, 2024-06-05*/
int8_t sysmon_llcf_set
(
 int8_t enable
)
{
	sysmon_fifo_msg_t msg;
	msg.type = gLLCFSet;
	msg.portid = enable;

	return send_to_sysmon_slave(msg);
}
#endif
#endif
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

#if 0//PWY_FIXME
	synce_config_set_admin(1);
	synce_config_set_if_select(1, 2);
	port_config_speed(1, PORT_IF_10G_KR, PORT_IF_10G_KR);
#endif //PWY_FIXME
	pm_request_counters();
	port_status_alarm();
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
#if 1/*[#40] Port config for rate/ESMC/alarm, dustin, 2024-05-30 */
	gCpssPortSetRate,
	gCpssPortESMCenable, /*further implements*/
	gCpssPortAlarm, /* Link Down/up */
#endif
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
	sysmon_fifo_msg_t msg;

	memset(&msg, 0, sizeof msg);
	msg.type = (enable == 1) ? gSynceEnable : gSynceDisable;

	/* use marvell sdk to set synce enable/disable. */
	if(enable == 1)
		gSysmonToCpssFuncs[gSynceEnable](1, &msg);
	else
		gSysmonToCpssFuncs[gSynceDisable](1, &msg);
	return 0;
}

int synce_config_set_if_select(int pri_port, int sec_port)
{
	sysmon_fifo_msg_t msg;

	memset(&msg, 0, sizeof msg);
	msg.type    = gSynceIfSelect;
	msg.portid  = pri_port;
	msg.portid2 = sec_port;

	/* use marvell sdk to set synce if select. */
	gSysmonToCpssFuncs[gSynceIfSelect](1, &msg);
	return 0;
}
#endif

#if 1/*[#40] Port config for rate/ESMC/alarm, dustin, 2024-05-30 */
void port_config_speed(int port, long speed, int mode)
{
	sysmon_fifo_msg_t msg;

	memset(&msg, 0, sizeof msg);
	msg.type   = gPortSetRate;
	msg.portid = port;
	msg.speed  = speed;
	msg.mode   = mode;

	/* use marvell sdk to set port speed/mode. */
	gSysmonToCpssFuncs[gPortSetRate](1, &msg);
	return;
}

void port_config_ESMC_enable(int port, int enable)
{
	sysmon_fifo_msg_t msg;

	memset(&msg, 0, sizeof msg);
	msg.type   = gPortESMCenable;
	msg.portid = port;
	msg.state  = enable;

	/* use marvell sdk to set port esmc enable. */
	gSysmonToCpssFuncs[gPortESMCenable](1, &msg);
	return;
}

void port_status_alarm(void)
{
	sysmon_fifo_msg_t msg;

	memset(&msg, 0, sizeof msg);
	msg.type = gPortAlarm;

	/* use marvell sdk to get port link status. */
	gSysmonToCpssFuncs[gPortAlarm](1, &msg);
	return;
}
#endif

int send_to_sysmon_slave(sysmon_fifo_msg_t * msg)
{
#if 0/*[#4] register updating : opening file at both sides don't work, dustin, 2024-05-28 */
	/*[#34] aldrin3s chip initial ¿¿ ¿¿, balkrow, 2024-05-23*/

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

uint8_t gReplySDKInit(int args, ...)
{
	uint8_t ret = IPC_CMD_SUCCESS;
	va_list argP;
	sysmon_fifo_msg_t *msg = NULL;

#ifdef DEBUG
	zlog_notice("%s (REPLY): args=%d", __func__, args);
#endif
	if(args !=  1) {
		syslog(LOG_INFO, "%s: invalid args[%d].", __func__, args);
		return IPC_CMD_FAIL;
	}

	va_start(argP, args);
	msg = va_arg(argP, sysmon_fifo_msg_t *);
	va_end(argP);

	/* process for result. */
	if(msg->result != FIFO_CMD_SUCCESS) {
		/*FIXME*/
	}

	return ret;
}

uint8_t gReplyHello(int args, ...)
{
	uint8_t ret = IPC_CMD_SUCCESS;
	va_list argP;
	sysmon_fifo_msg_t *msg = NULL;

#ifdef DEBUG
	zlog_notice("%s (REPLY): args=%d", __func__, args);
#endif
	if(args !=  1) {
		syslog(LOG_INFO, "%s: invalid args[%d].", __func__, args);
		return IPC_CMD_FAIL;
	}

	va_start(argP, args);
	msg = va_arg(argP, sysmon_fifo_msg_t *);
	va_end(argP);

	/* process for result. */
	if(msg->result != FIFO_CMD_SUCCESS) {
		/*FIXME*/
	}

	return ret;
}

uint8_t gReplySynceEnable(int args, ...)
{
	uint8_t ret = IPC_CMD_SUCCESS;
	va_list argP;
	sysmon_fifo_msg_t *msg = NULL;

#ifdef DEBUG
	zlog_notice("%s (REPLY): args=%d", __func__, args);
#endif
	if(args !=  1) {
		syslog(LOG_INFO, "%s: invalid args[%d].", __func__, args);
		return IPC_CMD_FAIL;
	}

	va_start(argP, args);
	msg = va_arg(argP, sysmon_fifo_msg_t *);
	va_end(argP);

	/* process for result. */
	/*FIXME*/

	return ret;
}

uint8_t gReplySynceDisable(int args, ...)
{
	uint8_t ret = IPC_CMD_SUCCESS;
	va_list argP;
	sysmon_fifo_msg_t *msg = NULL;

#ifdef DEBUG
	zlog_notice("%s (REPLY): args=%d", __func__, args);
#endif
	if(args !=  1) {
		syslog(LOG_INFO, "%s: invalid args[%d].", __func__, args);
		return IPC_CMD_FAIL;
	}

	va_start(argP, args);
	msg = va_arg(argP, sysmon_fifo_msg_t *);
	va_end(argP);

	/* process for result. */
	/*FIXME*/

	return ret;
}

uint8_t gReplySynceIfSelect(int args, ...)
{
	uint8_t ret = IPC_CMD_SUCCESS;
	va_list argP;
	sysmon_fifo_msg_t *msg = NULL;

#ifdef DEBUG
	zlog_notice("%s (REPLY): args=%d", __func__, args);
#endif
	if(args !=  1) {
		syslog(LOG_INFO, "%s: invalid args[%d].", __func__, args);
		return IPC_CMD_FAIL;
	}

	va_start(argP, args);
	msg = va_arg(argP, sysmon_fifo_msg_t *);
	va_end(argP);

	/* process for result. */
	/*FIXME*/

	return ret;
}

uint8_t gReplyPortSetRate(int args, ...)
{
	uint8_t ret = IPC_CMD_SUCCESS;
	va_list argP;
	sysmon_fifo_msg_t *msg = NULL;

#ifdef DEBUG
	zlog_notice("%s (REPLY): args=%d", __func__, args);
#endif
	if(args !=  1) {
		syslog(LOG_INFO, "%s: invalid args[%d].", __func__, args);
		return IPC_CMD_FAIL;
	}

	va_start(argP, args);
	msg = va_arg(argP, sysmon_fifo_msg_t *);
	va_end(argP);

	/* process for result. */
	if(msg->result != FIFO_CMD_SUCCESS) {
		syslog(LOG_INFO, "%s: Setting port speed/mode failed. ret[%d].", msg->result);
	}

	return ret;
}

uint8_t gReplyPortESMCEnable(int args, ...)
{
	uint8_t ret = IPC_CMD_SUCCESS;
	va_list argP;
	sysmon_fifo_msg_t *msg = NULL;

#ifdef DEBUG
	zlog_notice("%s (REPLY): args=%d", __func__, args);
#endif
	if(args !=  1) {
		syslog(LOG_INFO, "%s: invalid args[%d].", __func__, args);
		return IPC_CMD_FAIL;
	}

	va_start(argP, args);
	msg = va_arg(argP, sysmon_fifo_msg_t *);
	va_end(argP);

	/* process for result. */
	/*FIXME*/

	return ret;
}

uint8_t gReplyPortAlarm(int args, ...)
{
	uint8_t portno, ret = IPC_CMD_SUCCESS;
	va_list argP;
	sysmon_fifo_msg_t *msg = NULL;

#ifdef DEBUG
	zlog_notice("%s (REPLY): args=%d", __func__, args);
#endif
	if(args !=  1) {
		syslog(LOG_INFO, "%s: invalid args[%d].", __func__, args);
		return IPC_CMD_FAIL;
	}

	va_start(argP, args);
	msg = va_arg(argP, sysmon_fifo_msg_t *);
	va_end(argP);

	/* process for result. */
	for(portno = PORT_ID_EAG6L_PORT1; portno < PORT_ID_EAG6L_MAX; portno++) {
		PORT_STATUS[portno].link = msg->port_sts[portno].link;
	}

	return ret;
}

cSysmonReplyFuncs gSysmonReplyFuncs[] =
{
	gReplySDKInit,
	gReplyHello,
	gReplySynceEnable,
	gReplySynceDisable,
	gReplySynceIfSelect,
	gReplyPortSetRate,
	gReplyPortESMCEnable,
	gReplyPortAlarm,
};

const uint32_t funcsListLen2 = sizeof(gSysmonReplyFuncs) / sizeof(cSysmonReplyFuncs);
static int sysmon_master_system_command(sysmon_fifo_msg_t * msg)
{
#if 1/*[#34] aldrin3s chip initial 옜 옜, balkrow, 2024-05-23*/
#ifdef DEBUG
	zlog_notice("%s recv msg %x from cpss", __func__, msg->type);
#endif
#if 1/*[#24] Verifying syncE register update, dustin, 2024-05-28 */
#if 1/*FIXME*/
	gSysmonReplyFuncs[msg->type](1, msg);
#else
	switch(msg->type)
	{
		case gSDKInit:
			break;
		case gHello:
			zlog_notice("gHello (REPLY) : %s", msg->noti_msg[0] ? msg->noti_msg : "N/A");
			break;
		case gSynceEnable:
			zlog_notice("gSynceEnable (REPLY) : port[%d].", msg->portid);
			break;
		case gSynceDisable:
			zlog_notice("gSynceDisable (REPLY) : port[%d].", msg->portid);
			break;
		case gSynceIfSelect:
			zlog_notice("gSynceIfSelect (REPLY) : port[%d].", msg->portid);
			break;
#if 1/*[#43] LF占쏙옙占� RF 占쏙옙 占쏙옙 占쏙옙, balkrow, 2024-06-05*/
		case gLLCFSet:
			zlog_notice("gLLCFSet (REPLY) : enable[%d].\n", msg->portid);
 			break;
#endif
		case gPortSetRate:
			zlog_notice("gPortSetRate (REPLY) : result[%d].", msg->result);
			break;
		case gPortESMCenable:
			zlog_notice("gPortESMCenable (REPLY) : result[%d].", msg->result);
			break;
		case gPortAlarm:
			zlog_notice("gPortAlarm (REPLY) : result[%d].", msg->result);
			break;
			//TODO

		default:
			break;
	}
#endif
#endif

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
	/*[#34] aldrin3s chip initial ¿¿ ¿¿, balkrow, 2024-05-23*/
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
	thread_add_timer(master, sysmon_master_hello_test, NULL, 10);
#endif //PWY_FIXME

	return;
}
