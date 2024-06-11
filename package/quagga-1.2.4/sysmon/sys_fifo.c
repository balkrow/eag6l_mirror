/* 
	Copyright ¡§I 2014, by HFR,Inc. All Right Reserved.

	This Software of copyright is reserved by HFR.Inc. No all or part of this Software may be reproduced by  the 
	hard-copy format, electronically, or otherwise and transmitted to Ineligible Person. If not you should be subject 
	to civil suit and criminal case according to circumstances.


       º» ÆÄÀÏÀÇ ÀúÀÛ±ÇÀº (ÁÖ)¿¡Ä¡¿¡ÇÁ¾Ë¿¡ ÀÖ½À´Ï´Ù. (ÁÖ)¿¡Ä¡¿¡ÇÁ¾ËÀÇ ¸í½ÃÀûÀÎ µ¿ÀÇ¾øÀÌ º» ÆÄÀÏÀÇ ÀüÃ¼ ¶Ç´Â ÀÏºÎ¸¦ 
       ÇÏµåÄ«ÇÇ Çü½Ä, ÀüÀÚÀû ¹æ½Ä ¶Ç´Â ±âÅ¸¹æ½ÄÀ¸·Î Àç»ý»êÇÏ°Å³ª ¹Þ¾Æº¼ ¼ö ÀÖ´Â ÀÚ°ÝÀÌ ¾ø´Â »ç¶÷¿¡°Ô Àç¹èÆ÷ÇÏ´Â ÇàÀ§´Â
       ÀúÀÛ±Ç¹ýÀ» À§¹ÝÇÏ´Â °ÍÀÌ¸ç ¹Î»ç»ó ¼ÕÇØ¹è»ó ±×¸®°í ÇØ´çÇÏ´Â °æ¿ì¿¡´Â Çü»ç»ó Ã³¹úÀÇ ´ë»óÀÌ µË´Ï´Ù.
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

#if 1/*[#34] aldrin3s chip initial ¿¿ ¿¿, balkrow, 2024-05-23*/
#define DEBUG
#endif

extern struct thread_master *master;
extern struct port_status PORT_STATUS[PORT_ID_EAG6L_MAX];
extern port_pm_counter_t PM_TBL[PORT_ID_EAG6L_MAX];


/*define send callback function */
#if 1/*[#34] aldrin3s chip initial ¿¿ ¿¿, balkrow, 2024-05-23*/
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

#if 1/*[#43] LF¢¯¢¯¢¯ RF ¢¯¢¯ ¢¯¢¯ ¢¯¢¯, balkrow, 2024-06-05*/
uint8_t gCpssLLCFSet(int args, ...)
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
#endif

#if 1/*[#32] PM related register update, dustin, 2024-05-28 */
uint8_t gCpssPortPMGet(int args, ...)
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

uint8_t gCpssPortPMClear(int args, ...)
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
#endif
#endif
#endif

#if 1/*[#34] aldrin3s chip initial ¿¿ ¿¿, balkrow, 2024-05-23*/
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
#if 1/*[#43] LFâ–’~\â–’~C~]â–’~K~\ RF â–’| ~Dâ–’~Kâ–’ ê¸°â–’~Jâ–’ â–’~Tâ–’~@, balkrow, 2024-06-05*/
    gCpssLLCFSet,
#endif
#if 1/*[#40] Port config for rate/ESMC/alarm, dustin, 2024-05-30 */
	gCpssPortSetRate,
	gCpssPortESMCenable, /*further implements*/
	gCpssPortAlarm, /* Link Down/up */
#endif
#if 1/*[#32] PM related register update, dustin, 2024-05-28 */
	gCpssPortPMGet,
	gCpssPortPMClear,
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

#if 1/*[#43] LF¢¯¢¯¢¯ RF ¢¯¢¯ ¢¯¢¯ ¢¯¢¯, balkrow, 2024-06-05*/
int8_t sysmon_llcf_set(int8_t enable)
{
	sysmon_fifo_msg_t msg;

	memset(&msg, 0, sizeof msg);
	msg.type = gLLCFSet;
	msg.portid = enable;

	return gSysmonToCpssFuncs[gLLCFSet](&msg);
}
#endif

#if 1/*[#32] PM related register update, dustin, 2024-05-28 */
void pm_request_counters(void)
{
	sysmon_fifo_msg_t msg;

	memset(&msg, 0, sizeof msg);
	msg.type = gPortPMGet;

	/* use marvell sdk to get pm counters. */
	gSysmonToCpssFuncs[gPortPMGet](1, &msg);
	return;
}

void pm_request_clear(void)
{
	sysmon_fifo_msg_t msg;

	memset(&msg, 0, sizeof msg);
	msg.type = gPortPMClear;

	/* use marvell sdk to clear pm counters. */
	gSysmonToCpssFuncs[gPortPMClear](1, &msg);
	return;
}
#endif

int send_to_sysmon_slave(sysmon_fifo_msg_t * msg)
{
#if 0/*[#4] register updating : opening file at both sides don't work, dustin, 2024-05-28 */
	/*[#34] aldrin3s chip initial ¢¯¢¯ ¢¯¢¯, balkrow, 2024-05-23*/

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

#if 1/*[#43] LFï¿½ï¿½ï¿½ RF ï¿½ï¿½ ï¿½ï¿½ ï¿½ï¿½, balkrow, 2024-06-05*/
uint8_t gReplyLLCF(int args, ...)
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
#endif

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

#if 1/*[#32] PM related register update, dustin, 2024-05-28 */
uint8_t gReplyPortPMGet(int args, ...)
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
	if(msg->result == FIFO_CMD_SUCCESS) {
		/* accumulate the counters. marvell counters are cleared on read. */
		for(portno = PORT_ID_EAG6L_PORT1; portno < PORT_ID_EAG6L_MAX; portno++) {
			PM_TBL[portno].tx_frame += msg->pm[portno].tx_frame;
			PM_TBL[portno].rx_frame += msg->pm[portno].rx_frame;
			PM_TBL[portno].tx_byte  += msg->pm[portno].tx_byte;
			PM_TBL[portno].rx_byte  += msg->pm[portno].rx_byte;
			PM_TBL[portno].rx_fcs   += msg->pm[portno].rx_fcs;
			PM_TBL[portno].fcs_ok   += msg->pm[portno].fcs_ok;
			PM_TBL[portno].fcs_nok  += msg->pm[portno].fcs_nok;
#ifdef DEBUG
			syslog(LOG_INFO, ">>> gReplyPortPMGet : port[0/%d] ret[%d]", 
				get_eag6L_dport(portno, 0), ret);
			syslog(LOG_INFO, ">>> gReplyPortPMGet tx_frame[%lu] rx_frame[ %lu]", 
				PM_TBL[portno].tx_frame, PM_TBL[portno].rx_frame);
			syslog(LOG_INFO, ">>> gReplyPortPMGet tx_bytes[%lu] rx_bytes[%lu]", 
				PM_TBL[portno].tx_byte, PM_TBL[portno].rx_byte);
			syslog(LOG_INFO, ">>> gReplyPortPMGet rx_fcs[%lu]", PM_TBL[portno].rx_fcs);
			syslog(LOG_INFO, ">>> gReplyPortPMGet fcs_ok[%lu] fcs_nok[%lu]", 
				PM_TBL[portno].fcs_ok, PM_TBL[portno].fcs_nok);
#endif
		}
	} else
		syslog(LOG_INFO, "%s: Getting PM counters failed. ret[%d].", msg->result);

	return ret;
}

uint8_t gReplyPortPMClear(int args, ...)
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
	if(msg->result == FIFO_CMD_SUCCESS) {
		memset(PM_TBL, 0, sizeof PM_TBL);
	} else
		syslog(LOG_INFO, "%s: Clearing PM counters failed. ret[%d].", msg->result);

	return ret;
}
#endif

cSysmonReplyFuncs gSysmonReplyFuncs[] =
{
	gReplySDKInit,
	gReplyHello,
	gReplySynceEnable,
	gReplySynceDisable,
	gReplySynceIfSelect,
#if 1/*[#43] LFï¿½ï¿½ï¿½ RF ï¿½ï¿½ ï¿½ï¿½ ï¿½ï¿½, balkrow, 2024-06-05*/
	gReplyLLCF,
#endif
	gReplyPortSetRate,
	gReplyPortESMCEnable,
	gReplyPortAlarm,
#if 1/*[#32] PM related register update, dustin, 2024-05-28 */
	gReplyPortPMGet,
	gReplyPortPMClear,
#endif
};

const uint32_t funcsListLen2 = sizeof(gSysmonReplyFuncs) / sizeof(cSysmonReplyFuncs);
static int sysmon_master_system_command(sysmon_fifo_msg_t * msg)
{
#if 1/*[#34] aldrin3s chip initial ¿¿ ¿¿, balkrow, 2024-05-23*/
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
#if 1/*[#43] LFï¿½ï¿½ï¿½ RF ï¿½ï¿½ ï¿½ï¿½ ï¿½ï¿½, balkrow, 2024-06-05*/
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
#if 1/*[#32] PM related register update, dustin, 2024-05-28 */
		case gPortPMGet:
			zlog_notice("gPortPMGet (REPLY) : result[%d].", msg->result);
			zlog_notice("tx_frame : %lu", msg->pm[1].tx_frame);
			zlog_notice("rx_frame : %lu", msg->pm[1].rx_frame);
			zlog_notice("tx_byte  : %lu", msg->pm[1].tx_byte);
			zlog_notice("rx_byte  : %lu", msg->pm[1].rx_byte);
			zlog_notice("rx_fcs   : %lu", msg->pm[1].rx_fcs);
			break;
		case gPortPMClear:
			zlog_notice("gPortPMClear (REPLY) : result[%d].", msg->result);
			break;
#endif
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
#if 1/*[#4] register updating : opening file at both sides don't work, dustin, 2024-05-28 */
	/*[#34] aldrin3s chip initial ¢¯¢¯ ¢¯¢¯, balkrow, 2024-05-23*/
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
