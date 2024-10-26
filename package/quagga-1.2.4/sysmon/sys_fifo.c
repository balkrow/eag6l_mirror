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

#include "log.h"
#include "sysmon.h"
#include "sys_fifo.h"

#include <dirent.h>
#if 1 /* [#62] SFP eeprom ¿ register update ¿¿ ¿¿ ¿¿ ¿ ¿¿¿, balkrow, 2024-06-21 */ 
#include "bp_regs.h"
extern RegMON regMonList [];
extern uint16_t getIdxFromRegMonList (uint16_t reg);
#endif

#if 1/*[#73] SDK ¿¿¿ CPU trap ¿ packet ¿¿ ¿¿ ¿¿, balkrow, 2024-07-16 */
#undef DEBUG
#endif


extern struct thread_master *master;
extern port_status_t PORT_STATUS[PORT_ID_EAG6L_MAX];
extern port_pm_counter_t PM_TBL[PORT_ID_EAG6L_MAX];
#if 1/*[#56] register update timer ¿¿, balkrow, 2023-06-13 */
extern GLOBAL_DB gDB;
#endif

int send_to_sysmon_slave(sysmon_fifo_msg_t * msg);

#if 0/*[#120] LOC Alarm process ¿¿, balkrow, 2024-10-16 */
int loc_port[PORT_ID_EAG6L_MAX];
#endif

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
	zlog_notice("called %s args=%d", __func__, args);
	if(args !=  1)
		return IPC_CMD_FAIL;

	va_start(argP, args);
	msg = va_arg(argP, sysmon_fifo_msg_t *);
	va_end(argP);

	msg->type = gSDKInit;
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

	msg->type = gHello;
	if(send_to_sysmon_slave(msg) == 0)
		ret = IPC_CMD_FAIL;

	return ret;
}

#if 1/*[#56] register update timer ¿¿, balkrow, 2023-06-13 */
uint8_t gCpssSynceEnable(int args, ...)
{
	va_list argP;
	sysmon_fifo_msg_t msg;
#ifdef DEBUG
	zlog_notice("called %s args=%d", __func__, args);
#endif

	if(args != 1)
		return IPC_CMD_FAIL;

	memset(&msg, 0, sizeof msg);
	va_start(argP, args);
	va_arg(argP, uint32_t);	
	va_end(argP);
	
	msg.type = gSynceEnable; 

	if(send_to_sysmon_slave(&msg) == 0) {
		zlog_notice("%s : send_to_sysmon_slave() has failed.", __func__);
		return IPC_CMD_FAIL;
	}

	return IPC_CMD_SUCCESS;
}
#endif

#if 1/*[#59] Synce configuration ¿¿ ¿¿ ¿¿, balkrow, 2024-06-19 */
uint8_t gCpssSynceDisable(int args, ...)
{
	va_list argP;
	sysmon_fifo_msg_t msg;
#ifdef DEBUG
	zlog_notice("called %s args=%d", __func__, args);
#endif

	if(args != 1)
		return IPC_CMD_FAIL;

	memset(&msg, 0, sizeof msg);
	va_start(argP, args);
	va_arg(argP, uint32_t);	
	va_end(argP);
	
	msg.type = gSynceDisable; 

	if(send_to_sysmon_slave(&msg) == 0) {
		zlog_notice("%s : send_to_sysmon_slave() has failed.", __func__);
		return IPC_CMD_FAIL;
	}

	return IPC_CMD_SUCCESS;
}
#endif

#if 1/*[#127] SYNCE current interface ¿¿, balkrow, 2024-09-12*/
uint8_t gCpssSynceIfConf(int args, ...)
{
	va_list argP;
	uint32_t src, inf, conf;
	sysmon_fifo_msg_t msg;

#ifdef DEBUG
	zlog_notice("called %s args=%d", __func__, args);
#endif

	if(args != 3)
		return IPC_CMD_FAIL;

	memset(&msg, 0, sizeof msg);
	va_start(argP, args);
	src = va_arg(argP, uint32_t);	
	inf = va_arg(argP, uint32_t);	
	conf = va_arg(argP, uint32_t);	
	va_end(argP);

	msg.type = gPortSyncePortConf;
	msg.mode = src;
	msg.portid = inf;
	msg.portid2 = conf;

	if(send_to_sysmon_slave(&msg) == 0) {
		zlog_notice("%s : send_to_sysmon_slave() has failed.", __func__);
		return IPC_CMD_FAIL;
	}

	return IPC_CMD_SUCCESS;
}
#endif

#if 1/*[#24] Verifying syncE register update, dustin, 2024-05-28 */

#if 1/*[#56] register update timer ¿¿, balkrow, 2023-06-13 */
uint8_t gCpssSynceIfSelect(int args, ...)
{
	va_list argP;
	uint32_t pri_src, port;
	sysmon_fifo_msg_t msg;
#ifdef DEBUG
	zlog_notice("called %s args=%d", __func__, args);
#endif

	if(args != 2)
		return IPC_CMD_FAIL;

	memset(&msg, 0, sizeof msg);
	va_start(argP, args);
	pri_src = va_arg(argP, uint32_t);	
	port = va_arg(argP, uint32_t);	
	va_end(argP);

#if 1/*[#59] Synce configuration ¿¿ ¿¿ ¿¿, balkrow, 2024-06-19 */
	msg.type = gSynceIfSelect;
#endif
	msg.mode = pri_src;
	msg.portid = port;

#if 0/*[#120] LOC Alarm process ¿¿, balkrow, 2024-10-16 */
	if(msg.mode == PRI_SRC)
	{
		if(gDB.synce_pri_port != port)
		{
			uint16_t val, wr_val;
			gDB.synce_pri_port = port;

			PORT_STATUS[getMPortByCport(port)].esmc_loss = 1;
			gRegUpdate(SYNCE_SRC_STAT_ADDR, 0x8, 0xff00, (getMPortByCport(port))); 
			gRegUpdate(SYNCE_SRC_STAT_ADDR, 0, 0xff, 0x11); 
		}
	}
	else if(msg.mode == SEC_SRC)
	{
		if(gDB.synce_sec_port != port)
		{
			PORT_STATUS[getMPortByCport(port)].esmc_loss = 1;
			gRegUpdate(SYNCE_SRC_STAT_ADDR, 0x8, 0xff00, (getMPortByCport(port))); 
			gRegUpdate(SYNCE_SRC_STAT_ADDR, 0, 0xff, 0x11); 
		}
		gDB.synce_sec_port = port;
			
	}
#else
#if 1/*[#80] eag6l board SW bring-up, balkrow, 2024-07-24 */
	if(msg.mode == PRI_SRC)
	{
		if(gDB.synce_pri_port != port)
		{
			uint16_t val, wr_val;
			gDB.synce_pri_port = port;
#if 1/*[#127] SYNCE current interface ¿¿, balkrow, 2024-09-11*/
			if(gDB.synce_oper_port != NOT_DEFINED && 
			   (gDB.synce_oper_port == getMPortByCport(gDB.synce_sec_port)) && 
			    gDB.synce_oper_port != getMPortByCport(port))
				gCpssSynceIfConf(3, SEC_SRC, getCPortByMport(gDB.synce_oper_port), 0);

			gDB.synce_oper_port = getMPortByCport(port);
			gRegUpdate(SYNCE_SRC_STAT_ADDR, 0x8, 0xff00, (getMPortByCport(port))); 
			gRegUpdate(SYNCE_SRC_STAT_ADDR, 0, 0xff, 0x11); 
#endif
		}
	}
	else if(msg.mode == SEC_SRC)
	{
#if 1/*[#127] SYNCE current interface ¿¿, balkrow, 2024-09-11*/
		if(gDB.synce_sec_port != port)
		{
			gRegUpdate(SYNCE_SRC_STAT_ADDR, 0x8, 0xff00, (getMPortByCport(port))); 
			gRegUpdate(SYNCE_SRC_STAT_ADDR, 0, 0xff, 0x11); 
			gDB.synce_sec_port = port;
		}

		if(gDB.synce_pri_port == NOT_DEFINED)
			gDB.synce_oper_port = getMPortByCport(port);
			
#endif
	}
#endif
	if(send_to_sysmon_slave(&msg) == 0) {
		zlog_notice("%s : send_to_sysmon_slave() has failed.", __func__);
		return IPC_CMD_FAIL;
	}
#endif


	return IPC_CMD_SUCCESS;
}
#endif

uint8_t gCpssPortSetRate(int args, ...)
{
	va_list argP;
	sysmon_fifo_msg_t msg;
#ifdef DEBUG
	zlog_notice("called %s args=%d", __func__, args);
#endif

	if(args != 3) {
		zlog_notice("%s: invalid args[%d].", __func__, args);
		return IPC_CMD_FAIL;
	}

	memset(&msg, 0, sizeof msg);
	va_start(argP, args);
#if 1/*[#80] eag6l board SW bring-up, balkrow, 2023-07-22 */
	msg.portid = va_arg(argP, uint32_t);
	msg.speed = va_arg(argP, uint32_t);
#endif
	va_end(argP);
	msg.type = gPortSetRate;

	if(send_to_sysmon_slave(&msg) == 0) {
		zlog_notice("%s : send_to_sysmon_slave() has failed.", __func__);
		return IPC_CMD_FAIL;
	}

	return IPC_CMD_SUCCESS;
}

uint8_t gCpssPortESMCenable(int args, ...)
{
	va_list argP;
	sysmon_fifo_msg_t msg;
#ifdef DEBUG
	zlog_notice("called %s args=%d", __func__, args);
#endif

#if 1 /*[#82] eag6l board SW Debugging, balkrow, 2024-08-09*/
	if(args != 2) {
		zlog_notice("%s: invalid args[%d].", __func__, args);
		return IPC_CMD_FAIL;
	}

	memset(&msg, 0, sizeof msg);
	va_start(argP, args);
	msg.portid = va_arg(argP, uint32_t);
	msg.mode = va_arg(argP, uint32_t);
#endif
	va_end(argP);

	msg.type = gPortESMCenable;
	if(send_to_sysmon_slave(&msg) == 0) {
		zlog_notice("%s : send_to_sysmon_slave() has failed.", __func__);
		return IPC_CMD_FAIL;
	}

	return IPC_CMD_SUCCESS;
}

uint8_t gCpssPortAlarm(int args, ...)
{
    va_list argP;
    sysmon_fifo_msg_t msg;
#ifdef DEBUG
    zlog_notice("called %s args=%d", __func__, args);
#endif

    if(args != 0) {
		zlog_notice("%s: invalid args[%d].", __func__, args);
        return IPC_CMD_FAIL;
	}

	memset(&msg, 0, sizeof msg);
    va_start(argP, args);
    va_end(argP);

	msg.type = gPortAlarm;
    if(send_to_sysmon_slave(&msg) == 0) {
        zlog_notice("%s : send_to_sysmon_slave() has failed.", __func__);
        return IPC_CMD_FAIL;
    }

	return IPC_CMD_SUCCESS;
}

#if 1/*[#43] LF¢¯¢¯¢¯ RF ¢¯¢¯ ¢¯¢¯ ¢¯¢¯, balkrow, 2024-06-05*/
uint8_t gCpssLLCFSet(int args, ...)
{
	va_list argP;
	sysmon_fifo_msg_t msg;
	memset(&msg, 0, sizeof(sysmon_fifo_msg_t));
#ifdef DEBUG
	zlog_notice("called %s args=%d", __func__, args);
#endif

	if(args != 1) {
		zlog_notice("%s: invalid args[%d].", __func__, args);
		return IPC_CMD_FAIL;
	}

	memset(&msg, 0, sizeof msg);
	va_start(argP, args);
	msg.portid = va_arg(argP, int32_t);
	va_end(argP);
	msg.type = gLLCFSet;

	if(send_to_sysmon_slave(&msg) == 0) {
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
	sysmon_fifo_msg_t msg;
#ifdef DEBUG
	zlog_notice("called %s args=%d", __func__, args);
#endif

	if(args != 0) {
		zlog_notice("%s: invalid args[%d].", __func__, args);
		return IPC_CMD_FAIL;
	}

	memset(&msg, 0, sizeof msg);
	va_start(argP, args);
	va_end(argP);

	msg.type = gPortPMGet;
	if(send_to_sysmon_slave(&msg) == 0) {
		zlog_notice("%s : send_to_sysmon_slave() has failed.", __func__);
		return IPC_CMD_FAIL;
	}

	return IPC_CMD_SUCCESS;
}

uint8_t gCpssPortPMClear(int args, ...)
{
	va_list argP;
	sysmon_fifo_msg_t msg;
#ifdef DEBUG
	zlog_notice("called %s args=%d", __func__, args);
#endif

	if(args != 1) {
		zlog_notice("%s: invalid args[%d].", __func__, args);
		return IPC_CMD_FAIL;
	}

	memset(&msg, 0, sizeof msg);
	va_start(argP, args);
#if 1 /* [#62] SFP eeprom ¿ register update ¿¿ ¿¿ ¿¿ ¿ ¿¿¿, balkrow, 2024-06-21 */ 
	va_arg(argP, uint32_t);
#endif
	va_end(argP);
	msg.type = gPortPMClear;

	if(send_to_sysmon_slave(&msg) == 0) {
		zlog_notice("%s : send_to_sysmon_slave() has failed.", __func__);
		return IPC_CMD_FAIL;
	}

	return IPC_CMD_SUCCESS;
}
#endif
#endif
#endif

#if 1 /* [#85] Fixing for resetting PM counter for unexpected FEC counting, dustin, 2024-07-31 */
uint8_t gCpssPortPMFECClear(int args, ...)
{
	va_list argP;
	sysmon_fifo_msg_t msg;
#ifdef DEBUG
	zlog_notice("called %s args=%d", __func__, args);
#endif

	if(args != 1) {
		zlog_notice("%s: invalid args[%d].", __func__, args);
		return IPC_CMD_FAIL;
	}

	memset(&msg, 0, sizeof msg);
	va_start(argP, args);
	msg.portid = va_arg(argP, uint32_t);
	va_end(argP);
	msg.type = gPortPMFECClear;

	if(send_to_sysmon_slave(&msg) == 0) {
		zlog_notice("%s : send_to_sysmon_slave() has failed.", __func__);
		return IPC_CMD_FAIL;
	}

	return IPC_CMD_SUCCESS;
}
#endif
#if 1/*[#118] Sync-e option2 ¿¿, balkrow, 2024-09-06*/
uint8_t gCpssNone(int args, ...)
{
	return IPC_CMD_SUCCESS;
}

uint8_t gCpssPortSendQL(int args, ...)
{
	va_list argP;
	sysmon_fifo_msg_t msg;
#ifdef DEBUG
	zlog_notice("called %s args=%d", __func__, args);
#endif

	if(args != 1) {
		zlog_notice("%s: invalid args[%d].", __func__, args);
		return IPC_CMD_FAIL;
	}

	memset(&msg, 0, sizeof msg);
	va_start(argP, args);
	msg.portid = va_arg(argP, uint32_t);
	va_end(argP);
	msg.type = gPortSendQL;

	if(send_to_sysmon_slave(&msg) == 0) {
		zlog_notice("%s : send_to_sysmon_slave() has failed.", __func__);
		return IPC_CMD_FAIL;
	}
	return IPC_CMD_SUCCESS;
}

uint8_t gCpssLocalQL(int args, ...)
{
	va_list argP;
	sysmon_fifo_msg_t msg;
#ifdef DEBUG
	zlog_notice("called %s args=%d", __func__, args);
#endif

	if(args != 1) {
		zlog_notice("%s: invalid args[%d].", __func__, args);
		return IPC_CMD_FAIL;
	}

	memset(&msg, 0, sizeof msg);
	va_start(argP, args);
	msg.portid = va_arg(argP, uint32_t);
	va_end(argP);
	msg.type = gPortLocalQL;

	if(send_to_sysmon_slave(&msg) == 0) {
		zlog_notice("%s : send_to_sysmon_slave() has failed.", __func__);
		return IPC_CMD_FAIL;
	}
	return IPC_CMD_SUCCESS;
}
#endif

#if 1 /* [#152] Adding for port RS-FEC control, dustin, 2024-10-15 */
uint8_t gCpssPortFECEnable(int args, ...)
{
	va_list argP;
	sysmon_fifo_msg_t msg;
#ifdef DEBUG
	zlog_notice("called %s args=%d", __func__, args);
#endif

	if(args != 2) {
		zlog_notice("%s: invalid args[%d].", __func__, args);
		return IPC_CMD_FAIL;
	}

	memset(&msg, 0, sizeof msg);
	va_start(argP, args);
	msg.portid = va_arg(argP, uint32_t);
	msg.state = va_arg(argP, uint32_t);
	va_end(argP);
	msg.type = gPortFECEnable;

	if(send_to_sysmon_slave(&msg) == 0) {
		zlog_notice("%s : send_to_sysmon_slave() has failed.", __func__);
		return IPC_CMD_FAIL;
	}

	return IPC_CMD_SUCCESS;
}
#endif /* [#152] */

#if 1 /* [#165] DCO SFP ¿¿ LLCF ¿¿, balkrow, 2024-10-24 */
uint8_t gCpssNotiDcoState(int args, ...)
{
    va_list argP;
    sysmon_fifo_msg_t msg;
#ifdef DEBUG
    zlog_notice("called %s args=%d", __func__, args);
#endif

    if(args != 1) {
        zlog_notice("%s: invalid args[%d].", __func__, args);
        return IPC_CMD_FAIL;
    }

    memset(&msg, 0, sizeof msg);
    va_start(argP, args);
    msg.state = va_arg(argP, uint32_t);
    va_end(argP);
    msg.type = gNotifyDcoState;

    if(send_to_sysmon_slave(&msg) == 0) {
        zlog_notice("%s : send_to_sysmon_slave() has failed.", __func__);
        return IPC_CMD_FAIL;
    }

    return IPC_CMD_SUCCESS;
}
#endif

#if 1 /* [#142] Adding for Transparent mode switching, dustin, 2024-10-11 */
uint8_t gCpssSwitchModeSet(int args, ...)
{
    va_list argP;
    sysmon_fifo_msg_t msg;
#ifdef DEBUG
    zlog_notice("called %s args=%d", __func__, args);
#endif

    if(args != 1) {
        zlog_notice("%s: invalid args[%d].", __func__, args);
        return IPC_CMD_FAIL;
    }

    memset(&msg, 0, sizeof msg);
    va_start(argP, args);
    msg.state = va_arg(argP, uint32_t);
    va_end(argP);
    msg.type = gSwitchModeSet;

    if(send_to_sysmon_slave(&msg) == 0) {
        zlog_notice("%s : send_to_sysmon_slave() has failed.", __func__);
        return IPC_CMD_FAIL;
    }

    return IPC_CMD_SUCCESS;
}
#endif /* [#142] */

cSysmonToCPSSFuncs gSysmonToCpssFuncs[] =
{
	gCpssSDKInit,	
	gCpssHello,	
#if 1/*[#24] Verifying syncE register update, dustin, 2024-05-28 */
	gCpssSynceEnable,
#if 1/*[#56] register update timer ¿¿, balkrow, 2023-06-13 */
	gCpssSynceDisable,
#endif /*End 56*/
	gCpssSynceIfSelect,
#endif /*End 24*/
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
#if 1 /* [#85] Fixing for resetting PM counter for unexpected FEC counting, dustin, 2024-07-31 */
	gCpssPortPMFECClear,
#endif
#if 1/*[#118] Sync-e option2 ¿¿, balkrow, 2024-09-06*/
	gCpssNone,
	gCpssPortSendQL,
	gCpssLocalQL,
#endif
#if 1/*[#127] SYNCE current interface ¿¿, balkrow, 2024-09-12*/
	gCpssSynceIfConf,
#endif
#if 1 /* [#152] Adding for port RS-FEC control, dustin, 2024-10-15 */
	gCpssPortFECEnable,
#endif /* [#152] */
#if 1 /* [#142] Adding for Transparent mode switching, dustin, 2024-10-11 */
	gCpssSwitchModeSet,
#endif /* [#142] */
#if 1 /* [#165] DCO SFP ¿¿ LLCF ¿¿, balkrow, 2024-10-24 */
	gCpssNotiDcoState,
#endif
};

const uint32_t funcsListLen = sizeof(gSysmonToCpssFuncs) / sizeof(cSysmonToCPSSFuncs);
/*define recv callback function */



uint8_t gAppDemoIPCstate = IPC_INIT_SUCCESS; 
#if 1/*[#56] register update timer ¿¿, balkrow, 2023-06-13 */
int32_t syscmdrdfifo = UNINITIALIZED_FD;
int32_t syscmdwrfifo = UNINITIALIZED_FD;
#endif

#if 1/*[#56] register update timer ¿¿, balkrow, 2023-06-13 */
int synce_if_pri_select(int8_t port, int8_t pri)
{
#if 1/* use variable args scheme */
	/* use marvell sdk to set synce if select. */
	gSysmonToCpssFuncs[gSynceIfSelect](2, PRI_SRC, port);
#else
	sysmon_fifo_msg_t msg;

	memset(&msg, 0, sizeof msg);
	msg.type    = gSynceIfSelect;

	msg.mode = pri;
	msg.portid  = port;

	/* use marvell sdk to set synce if select. */
	gSysmonToCpssFuncs[gSynceIfSelect](1, &msg);
#endif
	return 0;
}
#endif

#if 1/*[#43] LF¢¯¢¯¢¯ RF ¢¯¢¯ ¢¯¢¯ ¢¯¢¯, balkrow, 2024-06-05*/
int8_t sysmon_llcf_set(int32_t enable)
{
#if 1/*[#56] register update timer ¿¿, balkrow, 2023-06-13 */
	return gSysmonToCpssFuncs[gLLCFSet](1, (uint16_t)enable);
#else
	sysmon_fifo_msg_t msg;

	memset(&msg, 0, sizeof msg);
	msg.type = gLLCFSet;
	msg.portid = enable;

	return gSysmonToCpssFuncs[gLLCFSet](&msg);
#endif
}
#endif

#if 1/*[#32] PM related register update, dustin, 2024-05-28 */
void pm_request_counters(void)
{
	/* use marvell sdk to get pm counters. */
	gSysmonToCpssFuncs[gPortPMGet](0);
	return;
}

void pm_request_clear(void)
{
	/* use marvell sdk to clear pm counters. */
	gSysmonToCpssFuncs[gPortPMClear](1, 1);
	return;
}
#endif

int send_to_sysmon_slave(sysmon_fifo_msg_t * msg)
{
#if 1/*[#4] register updating : opening file at both sides don't work, dustin, 2024-05-28 */
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

	if(args !=  1) {
		zlog_notice("%s: invalid args[%d].", __func__, args);
		return IPC_CMD_FAIL;
	}

	va_start(argP, args);
	msg = va_arg(argP, sysmon_fifo_msg_t *);
	va_end(argP);

	/* process for result. */
	if(msg->result != FIFO_CMD_SUCCESS) {
#if 1/*[#56] register update timer ¿¿, balkrow, 2023-06-13 */
		gDB.sdk_init_state = SDK_INIT_FAIL;
#endif
	}
#if 1/*[#56] register update timer ¿¿, balkrow, 2023-06-13 */
	else
		gDB.sdk_init_state = SDK_INIT_DONE;
#endif

	zlog_notice("SDK INIT %s", gDB.sdk_init_state == SDK_INIT_DONE ? "SUCCESS" : "FAIL" );

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
		zlog_notice("%s: invalid args[%d].", __func__, args);
		return IPC_CMD_FAIL;
	}

	va_start(argP, args);
	msg = va_arg(argP, sysmon_fifo_msg_t *);
	va_end(argP);

	/* process for result. */
	if(msg->result != FIFO_CMD_SUCCESS) {
#if 1/*[#56] register update timer ¿¿, balkrow, 2023-06-13 */
		gDB.comm_state = COMM_FAIL;
#endif
		/*FIXME*/
	}
#if 1/*[#56] register update timer ¿¿, balkrow, 2023-06-13 */
	else
		gDB.comm_state = COMM_SUCCESS;
#endif

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
		zlog_notice("%s: invalid args[%d].", __func__, args);
		return IPC_CMD_FAIL;
	}

	va_start(argP, args);
	msg = va_arg(argP, sysmon_fifo_msg_t *);
	va_end(argP);

	/* process for result. */
#if 1 /* [#62] SFP eeprom ¿ register update ¿¿ ¿¿ ¿¿ ¿ ¿¿¿, balkrow, 2024-06-21 */ 
#if 0/*[#127] SYNCE current interface ¿¿, balkrow, 2024-09-11*/
	if(msg->result == FIFO_CMD_SUCCESS)
	{
		uint16_t idx;
		idx = getIdxFromRegMonList(SYNCE_GCONFIG_ADDR);
		if(idx != 0xff && regMonList[idx].rb_thread)
		{
			thread_cancel(regMonList[idx].rb_thread);
			regMonList[idx].rb_thread = NULL;
		}
	}
#endif

	zlog_notice("%s  result=%x", __func__, msg->result);
#endif

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
		zlog_notice("%s: invalid args[%d].", __func__, args);
		return IPC_CMD_FAIL;
	}

	va_start(argP, args);
	msg = va_arg(argP, sysmon_fifo_msg_t *);
	va_end(argP);

	/* process for result. */
#if 1 /* [#62] SFP eeprom ¿ register update ¿¿ ¿¿ ¿¿ ¿ ¿¿¿, balkrow, 2024-06-21 */ 
#if 0/*[#127] SYNCE current interface ¿¿, balkrow, 2024-09-11*/
	if(msg->result == FIFO_CMD_SUCCESS)
	{
		uint16_t idx;
		idx = getIdxFromRegMonList(SYNCE_GCONFIG_ADDR);
		if(idx != 0xff && regMonList[idx].rb_thread)
		{
			thread_cancel(regMonList[idx].rb_thread);
			regMonList[idx].rb_thread = NULL;
		}
	}
#endif
	zlog_notice("%s  result=%x", __func__, msg->result);
#endif

	return ret;
}

#if 1/*[#127] SYNCE current interface ¿¿, balkrow, 2024-09-11*/
int8_t synce_sec_port_config (void)
{
	gCpssSynceIfConf(3, SEC_SRC, getCPortByMport(gDB.synce_oper_port), 1);
	return 0;
}
#endif
uint8_t gReplySynceIfSelect(int args, ...)
{
	uint8_t ret = IPC_CMD_SUCCESS;
	va_list argP;
	sysmon_fifo_msg_t *msg = NULL;

#ifdef DEBUG
	zlog_notice("%s (REPLY): args=%d", __func__, args);
#endif
	if(args !=  1) {
		zlog_notice("%s: invalid args[%d].", __func__, args);
		return IPC_CMD_FAIL;
	}

	va_start(argP, args);
	msg = va_arg(argP, sysmon_fifo_msg_t *);
	va_end(argP);

	/* process for result. */
#if 0
	/*FIXME*/
#if 1/*[#73] SDK ¿¿¿ CPU trap ¿ packet ¿¿ ¿¿ ¿¿, balkrow, 2024-07-18*/
	if(msg->portid != 0xff)
	{
		gDB.synce_pri_port = msg->portid;
#if 1/*[#127] SYNCE current interface ¿¿, balkrow, 2024-09-11*/
		thread_add_timer(master, synce_sec_port_config, NULL, 2);
#endif
	}
	else if(msg->portid2 != 0xff)
		gDB.synce_sec_port = msg->portid2;
#endif
#endif

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
		zlog_notice("%s: invalid args[%d].", __func__, args);
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
		zlog_notice("%s: invalid args[%d].", __func__, args);
		return IPC_CMD_FAIL;
	}

	va_start(argP, args);
	msg = va_arg(argP, sysmon_fifo_msg_t *);
	va_end(argP);

	/* process for result. */
	if(msg->result != FIFO_CMD_SUCCESS) {
#if 1/*[#59] Synce configuration ¿¿ ¿¿ ¿¿, balkrow, 2024-06-19 */
		zlog_notice("Setting port speed/mode failed for port[%d]. ret[%d].", 
			msg->portid, msg->result);
		return ret;
#endif
	}

	PORT_STATUS[msg->portid].speed  = msg->speed;
	PORT_STATUS[msg->portid].ifmode = msg->mode;

#if 1 /* [#62] SFP eeprom ¿ register update ¿¿ ¿¿ ¿¿ ¿ ¿¿¿, balkrow, 2024-06-21 */ 
#if 0/*[#127] SYNCE current interface ¿¿, balkrow, 2024-09-11*/
	if(msg->result == FIFO_CMD_SUCCESS)
	{
		uint16_t idx;
#if 1 /* [#91] Fixing for register updating feature, dustin, 2024-08-05 */
		idx = getIdxFromRegMonList(__COMMON_CTRL2_ADDR[msg->portid]);
#else
		idx = getIdxFromRegMonList(SYNCE_GCONFIG_ADDR);
#endif
		if(idx != 0xff && regMonList[idx].rb_thread)
		{
			thread_cancel(regMonList[idx].rb_thread);
			regMonList[idx].rb_thread = NULL;
		}
	}
#endif

	zlog_notice("%s  result=%x for port[%d].", __func__, msg->result, msg->portid);
#endif

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
		zlog_notice("%s: invalid args[%d].", __func__, args);
		return IPC_CMD_FAIL;
	}

	va_start(argP, args);
	msg = va_arg(argP, sysmon_fifo_msg_t *);
	va_end(argP);

	/* process for result. */
#if 0 /*[#82] eag6l board SW Debugging, balkrow, 2024-08-08*/
	PORT_STATUS[msg->portid].cfg_esmc_enable = msg->mode;
#endif

	return ret;
}

#if 0/*[#127] SYNCE current interface ¿¿, balkrow, 2024-09-11*/
void link_change_notifier(int8_t port, int8_t status)
{
	/*Check Sync-e operation interface*/
	if(gDB.synce_oper_port == port)
	{

	}
}
#endif

uint8_t gReplyPortAlarm(int args, ...)
{
	uint8_t portno, ret = IPC_CMD_SUCCESS;
	va_list argP;
	sysmon_fifo_msg_t *msg = NULL;

#ifdef DEBUG
	zlog_notice("%s (REPLY): args=%d", __func__, args);
#endif
	if(args !=  1) {
		zlog_notice("%s: invalid args[%d].", __func__, args);
		return IPC_CMD_FAIL;
	}

	va_start(argP, args);
	msg = va_arg(argP, sysmon_fifo_msg_t *);
	va_end(argP);

	/* process for result. */
	for(portno = PORT_ID_EAG6L_PORT1; portno < PORT_ID_EAG6L_MAX; portno++) {
#if 0/*[#127] SYNCE current interface ¿¿, balkrow, 2024-09-11 TODO:*/
		if(PORT_STATUS[portno].link != msg->port_sts[portno].link)
			link_change_notifier(port_no, msg->port_sts[portno].link);
#endif
#if 1 /* [#157] Fixing for Smart T-SFP rtWDM info, dustin, 2024-10-18 */
		if(PORT_STATUS[portno].tunable_sfp) {
			extern struct module_inventory RTWDM_INV_TBL[PORT_ID_EAG6L_MAX];

			/* clear rtWDM info if link goes down. */
			if(PORT_STATUS[portno].link && (! msg->port_sts[portno].link)) {
				memset(&(RTWDM_INV_TBL[portno]), 0, sizeof(struct module_inventory));
				memset(&(PORT_STATUS[portno].rtwdm_ddm_info), 0, sizeof(ddm_info_t));
			}
		}

		/* clear flag for next update, if link state changed. */
        if(PORT_STATUS[portno].link != msg->port_sts[portno].link)
            PORT_STATUS[portno].inv_up_flag = PORT_STATUS[portno].inv_clear_flag = 0;

#endif /* [#157] */
		PORT_STATUS[portno].link = msg->port_sts[portno].link;
#if 1/*[#66] Adding for updating port speed info, dustin, 2024-06-24 */
		PORT_STATUS[portno].speed = msg->port_sts[portno].speed;
#endif
#if 1 /* [#88] Adding LF/RF reading and updating to Alarm, dustin, 2024-08-01 */
		PORT_STATUS[portno].local_fault  = msg->port_sts[portno].local_fault;
		PORT_STATUS[portno].remote_fault = msg->port_sts[portno].remote_fault;
#endif
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
		zlog_notice("%s: invalid args[%d].", __func__, args);
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
		}
	} else
#if 1/*[#59] Synce configuration ¿¿ ¿¿ ¿¿, balkrow, 2024-06-19 */
		zlog_notice("Getting PM counters failed. ret[%d].", msg->result);
#endif

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
		zlog_notice("%s: invalid args[%d].", __func__, args);
		return IPC_CMD_FAIL;
	}

	va_start(argP, args);
	msg = va_arg(argP, sysmon_fifo_msg_t *);
	va_end(argP);

	/* process for result. */
	if(msg->result == FIFO_CMD_SUCCESS) {
		memset(PM_TBL, 0, sizeof PM_TBL);
	} else
#if 1/*[#59] Synce configuration ¿¿ ¿¿ ¿¿, balkrow, 2024-06-19 */
		zlog_notice("Clearing PM counters failed. ret[%d].", msg->result);
#endif

	return ret;
}
#endif

#if 1 /* [#91] Fixing for register updating feature, dustin, 2024-08-05 */
uint8_t gReplyPortPMFECClear(int args, ...)
{
	uint8_t ret = IPC_CMD_SUCCESS;
    va_list argP;
	sysmon_fifo_msg_t *msg = NULL;

#ifdef DEBUG
	zlog_notice("%s (REPLY): args=%d", __func__, args);
#endif
    if(args != 1) {
        zlog_notice("%s: invalid args[%d].", __func__, args);
        return IPC_CMD_FAIL;
    }

    va_start(argP, args);
    msg = va_arg(argP, sysmon_fifo_msg_t *);
    va_end(argP);

	/* process for result. */
	if(msg->result == FIFO_CMD_SUCCESS) {
		PM_TBL[msg->portid].fcs_ok = 0;
		PM_TBL[msg->portid].fcs_nok = 0;
	} else
		zlog_notice("Clearing PM FEC counters failed. ret[%d].", msg->result);
    return ret;
}
#endif

#if 1/*[#120] LOC Alarm process ¿¿, balkrow, 2024-10-16 */
int8_t enable_synce_interface(int8_t src) 
{
	int16_t port;
	if(src == PRI_SRC)
	{
		port = getMPortByCport(gDB.synce_pri_port);
		if(port != NOT_DEFINED)
		{
			gDB.synce_oper_port = port; 
			gCpssSynceIfConf(3, PRI_SRC, gDB.synce_pri_port, 1);
		}
	}
	else if(src == SEC_SRC)
	{
		
		port = getMPortByCport(gDB.synce_sec_port);
		if(port != NOT_DEFINED)
		{
			if(getMPortByCport(gDB.synce_pri_port) == NOT_DEFINED)
				gDB.synce_oper_port = port; 
			gCpssSynceIfConf(3, SEC_SRC, gDB.synce_sec_port, 1);
		}
	}
	return 0;
}

int16_t get_synce_oper_port(int16_t candidate_port) 
{
	int16_t port = NOT_DEFINED;
	if(candidate_port == NOT_DEFINED)
		port = NOT_DEFINED;
	else if(gDB.esmcRxCfg[candidate_port -1] == CFG_ENABLE && !PORT_STATUS[candidate_port].esmc_loss)
	{
		if(PORT_STATUS[candidate_port].received_QL != 0x11 && 
		PORT_STATUS[candidate_port].received_QL != 0x21) 
			port = candidate_port;
	}
	else if(gDB.esmcRxCfg[candidate_port -1] == CFG_ENABLE && PORT_STATUS[candidate_port].esmc_loss)
		port = NOT_DEFINED;

	zlog_notice("candidate_port %d QL %x ret %x", candidate_port, PORT_STATUS[candidate_port].received_QL, port);

	return port;
}

uint8_t processLOC(struct thread *thread)
{
	uint8_t portno;
	for(portno = PORT_ID_EAG6L_PORT1; portno < PORT_ID_EAG6L_MAX; portno++) 
	{
#if 0
		if(gDB.esmcRxCfg[portno -1] == CFG_DISABLE)
			continue;
#endif

		if(PORT_STATUS[portno].esmc_prev_cnt == 0)
		{
			PORT_STATUS[portno].esmc_prev_cnt = PORT_STATUS[portno].esmc_recv_cnt; 
			continue;
		}

		if(PORT_STATUS[portno].esmc_prev_cnt == PORT_STATUS[portno].esmc_recv_cnt)
			PORT_STATUS[portno].loc_cnt++;
		else
		{
			PORT_STATUS[portno].esmc_prev_cnt = PORT_STATUS[portno].esmc_recv_cnt; 
			PORT_STATUS[portno].loc_cnt = 0;
		}
#ifdef DEBUG
	zlog_notice("port %d esmc_recv_cnt  %x loc_cnt %x", portno, PORT_STATUS[portno].esmc_recv_cnt, PORT_STATUS[portno].loc_cnt );
#endif
		if(PORT_STATUS[portno].loc_cnt > MAX_LOC_CNT)
		{
			/*declare alarm*/
			zlog_notice("port %d %x ESMC LOSS while 3 seconds", portno, gDB.synce_pri_port );
			PORT_STATUS[portno].esmc_recv_cnt = 0;
			PORT_STATUS[portno].esmc_prev_cnt = 0;

			if(gDB.esmcRxCfg[portno - 1] == CFG_ENABLE)
				PORT_STATUS[portno].esmc_loss = 1;

			PORT_STATUS[portno].loc_cnt = 0;
			PORT_STATUS[portno].received_QL = 0;

			if(getMPortByCport(gDB.synce_pri_port) == portno)
			{
				uint16_t val;
				zlog_notice("port %d clear pri interface", portno);
				gCpssSynceIfConf(3, PRI_SRC, getCPortByMport(portno), 0);

				val = sys_fpga_memory_read(SYNCE_ESMC_RQL_ADDR, PORT_NOREG);
				val = (val & ~(0xff00));
				sys_fpga_memory_write(SYNCE_ESMC_RQL_ADDR, val, PORT_NOREG);
				val = sys_fpga_memory_read(SYNCE_ESMC_SQL_ADDR, PORT_NOREG);
				val = (val & ~(0xff00));
				sys_fpga_memory_write(SYNCE_ESMC_SQL_ADDR, val, PORT_NOREG);
#if 1
				/*switch secondary interface*/
				if(gDB.synce_oper_port == portno)
					gDB.synce_oper_port = get_synce_oper_port(getMPortByCport(gDB.synce_sec_port)); 
#else
				if(gDB.synce_oper_port == portno)
				{
					int16_t sec_port = getMPortByCport(gDB.synce_sec_port);

					if(sec_port == 0xff)
						gDB.synce_oper_port = NOT_DEFINED;
					else if(!PORT_STATUS[sec_port].esmc_loss)
						gDB.synce_oper_port = sec_port;
				}
#endif
			}
			else if(getMPortByCport(gDB.synce_sec_port) == portno)
			{
				uint16_t val;
				zlog_notice("port %d clear sec interface", portno);
				gCpssSynceIfConf(3, SEC_SRC, getCPortByMport(portno), 0);

				val = sys_fpga_memory_read(SYNCE_ESMC_RQL_ADDR, PORT_NOREG);
				val = (val & ~(0xff));
				sys_fpga_memory_write(SYNCE_ESMC_RQL_ADDR, val, PORT_NOREG);

				val = sys_fpga_memory_read(SYNCE_ESMC_SQL_ADDR, PORT_NOREG);
				val = (val & ~(0xff));
				sys_fpga_memory_write(SYNCE_ESMC_SQL_ADDR, val, PORT_NOREG);
#if 1
				/*switch primary interface*/
				if(gDB.synce_oper_port == portno)
					gDB.synce_oper_port = get_synce_oper_port(getMPortByCport(gDB.synce_pri_port)); 
#else
				if(gDB.synce_oper_port == portno)
				{
					int16_t pri_port = getMPortByCport(gDB.synce_pri_port);

					if(pri_port == 0xff)
						gDB.synce_oper_port = NOT_DEFINED;
					else if(!PORT_STATUS[pri_port].esmc_loss)
						gDB.synce_oper_port = pri_port;
				}
#endif
			}

			{
				int16_t pri_port, sec_port;
				pri_port = getMPortByCport(gDB.synce_pri_port);
				sec_port = getMPortByCport(gDB.synce_sec_port);

				zlog_notice("oper port %x  pri %x sec %x", gDB.synce_oper_port, pri_port, sec_port );
				if(gDB.synce_oper_port == NOT_DEFINED) 
				{
					if(gDB.esmcRxCfg[pri_port -1] != CFG_ENABLE &&
					   gDB.esmcRxCfg[sec_port -1] != CFG_ENABLE)
					{
						enable_synce_interface(PRI_SRC);
						enable_synce_interface(SEC_SRC);
					}
					else if(pri_port == NOT_DEFINED && gDB.esmcRxCfg[pri_port -1] != CFG_ENABLE) 
					{
						enable_synce_interface(SEC_SRC);
					}
					else if(sec_port == NOT_DEFINED && gDB.esmcRxCfg[sec_port -1] != CFG_ENABLE) 
					{
						enable_synce_interface(PRI_SRC);
					}
				}
				else
				{
					if(gDB.synce_oper_port == pri_port)
						enable_synce_interface(PRI_SRC);
					if(gDB.synce_oper_port == sec_port)
						enable_synce_interface(SEC_SRC);

				}

			}

			{
				uint16_t val;
				if(portno == M_PORT1)
				{
					val = sys_fpga_memory_read(SYNCE_ESMC_RQL2_ADDR, PORT_NOREG);
					val = (val & ~(0xff00));
					sys_fpga_memory_write(SYNCE_ESMC_RQL2_ADDR, val, PORT_NOREG);
				}
				else if(portno == M_PORT2)
				{
					val = sys_fpga_memory_read(SYNCE_ESMC_RQL2_ADDR, PORT_NOREG);
					val = (val & ~(0xff));
					sys_fpga_memory_write(SYNCE_ESMC_RQL2_ADDR, val, PORT_NOREG);
				}
				else if(portno == M_PORT3)
				{
					val = sys_fpga_memory_read(SYNCE_ESMC_RQL3_ADDR, PORT_NOREG);
					val = (val & ~(0xff00));
					sys_fpga_memory_write(SYNCE_ESMC_RQL3_ADDR, val, PORT_NOREG);
				}
				else if(portno == M_PORT4)
				{
					val = sys_fpga_memory_read(SYNCE_ESMC_RQL3_ADDR, PORT_NOREG);
					val = (val & ~(0xff));
					sys_fpga_memory_write(SYNCE_ESMC_RQL3_ADDR, val, PORT_NOREG);
				}
				else if(portno == M_PORT5)
				{
					val = sys_fpga_memory_read(SYNCE_ESMC_RQL4_ADDR, PORT_NOREG);
					val = (val & ~(0xff00));
					sys_fpga_memory_write(SYNCE_ESMC_RQL4_ADDR, val, PORT_NOREG);
				}
				else if(portno == M_PORT6)
				{
					val = sys_fpga_memory_read(SYNCE_ESMC_RQL4_ADDR, PORT_NOREG);
					val = (val & ~(0xff));
					sys_fpga_memory_write(SYNCE_ESMC_RQL4_ADDR, val, PORT_NOREG);
				}
				else if(portno == M_PORT7)
				{
					val = sys_fpga_memory_read(SYNCE_ESMC_RQL5_ADDR, PORT_NOREG);
					val = (val & ~(0xff00));
					sys_fpga_memory_write(SYNCE_ESMC_RQL5_ADDR, val, PORT_NOREG);
				}
			}
		}
	}

	thread_add_timer(master, processLOC, NULL, 1);
	return 0;
}
#endif

#if 1/*[#73] SDK ¿¿¿ CPU trap ¿ packet ¿¿ ¿¿ ¿¿, balkrow, 2024-07-17*/

#if 1/*[#120] LOC Alarm process ¿¿, balkrow, 2024-10-16 */
#define OPT1_QL_PRC 0x13
#define OPT1_QL_SSUA 0x14
#define OPT1_QL_SSUB 0x15
#define OPT1_QL_SEC 0x12
#define OPT1_QL_DNU 0x11
#define ESMC_SWITCH 1
#define ESMC_DONOT_SWITCH 0

int8_t compare_QL(int QL, int port)
{
	int8_t ret = 0;

	if(PORT_STATUS[port].received_QL == OPT1_QL_PRC)
	{
		ret = ESMC_DONOT_SWITCH;
	}
	else if(PORT_STATUS[port].received_QL == OPT1_QL_SSUA)
	{
		if(QL == OPT1_QL_PRC) 
			ret =  ESMC_SWITCH;
	}
	else if(PORT_STATUS[port].received_QL == OPT1_QL_SSUB)
	{
		if((QL == OPT1_QL_PRC) || (QL == OPT1_QL_SSUA)) 
			ret =  ESMC_SWITCH;
	}
	else if(PORT_STATUS[port].received_QL == OPT1_QL_SEC)
	{
		if((QL == OPT1_QL_PRC) || (QL == OPT1_QL_SSUA) || (QL == OPT1_QL_SSUB)) 
			ret =  ESMC_SWITCH;
	}

#ifdef DEBUG
	zlog_notice("%s:%d port %x Recevie QL %x QL %x ret %x", __func__, __LINE__, port, PORT_STATUS[port].received_QL, QL, ret); 
#endif
	return ret;

}
uint8_t switchEsmcInterface(int port, int QL)
{
	int pri_port, sec_port, oper_port; 
	/*check dnu*/
	if(!PORT_STATUS[port].received_QL)
	{
#ifdef DEBUG
		zlog_notice("%s:%d port %x Recevie QL %x QL %x oper_port %x, %x, %x", __func__, __LINE__, port, 
			    PORT_STATUS[port].received_QL, QL, gDB.synce_oper_port, 
			    gDB.synce_pri_port,gDB.synce_sec_port);
#endif

		pri_port = getMPortByCport(gDB.synce_pri_port);
		sec_port = getMPortByCport(gDB.synce_sec_port);

		if(QL == 0x11 || QL == 0x21) 
		{
			if(pri_port == port)
			{
				zlog_notice("Recevie DNU.. port %d clear pri interface", port);
				gDB.synce_oper_port = get_synce_oper_port(sec_port);
				gCpssSynceIfConf(3, PRI_SRC, getCPortByMport(port), 0);

				if(gDB.synce_oper_port != NOT_DEFINED)
				{
					gCpssSynceIfConf(3, SEC_SRC, getCPortByMport(sec_port), 0);
					zlog_notice("Synce Current interface %d", gDB.synce_oper_port);
					gCpssSynceIfConf(3, SEC_SRC, getCPortByMport(sec_port), 1);
				}
			}
			else if(sec_port == port)
			{
				zlog_notice("Recevie DNU.. port %d clear sec interface", port);
				gDB.synce_oper_port = get_synce_oper_port(pri_port);
				gCpssSynceIfConf(3, SEC_SRC, getCPortByMport(port), 0);
				if(gDB.synce_oper_port != NOT_DEFINED)
				{
					gCpssSynceIfConf(3, PRI_SRC, getCPortByMport(pri_port), 0);
					zlog_notice("Synce Current interface %d", gDB.synce_oper_port);
					gCpssSynceIfConf(3, PRI_SRC, getCPortByMport(pri_port), 1);
				}
			}
		}

		/*Check oper interface QL*/
		oper_port = gDB.synce_oper_port;
		if(oper_port != NOT_DEFINED)
		{
			/*received QL is better than oper interrface */
			if(compare_QL(QL, oper_port))
			{
				if(oper_port == pri_port) 
				{
					gCpssSynceIfConf(3, PRI_SRC, getCPortByMport(oper_port), 0);
					zlog_notice("port %x Recevie QL %x better than port %x", port, QL, oper_port);
					gCpssSynceIfConf(3, PRI_SRC, getCPortByMport(oper_port), 1);
					gDB.synce_oper_port = port; 
					zlog_notice("Synce Current interface %d", gDB.synce_oper_port);
				}
				else
				{
					gCpssSynceIfConf(3, SEC_SRC, getCPortByMport(oper_port), 0);
					zlog_notice("port %x Recevie QL %x better than port %x", port, QL, oper_port);
					gCpssSynceIfConf(3, SEC_SRC, getCPortByMport(oper_port), 1);
					gDB.synce_oper_port = port; 
					zlog_notice("Synce Current interface %d", gDB.synce_oper_port);
				}
			}

		}
		else
		{
			PORT_STATUS[port].received_QL = QL;
			if(gDB.synce_pri_port != NOT_DEFINED && 
			   (port == getMPortByCport(gDB.synce_pri_port)))  
			{
					gDB.synce_oper_port = get_synce_oper_port(port);
					zlog_notice("Synce Current interface %d", gDB.synce_oper_port);
			}

			if(gDB.synce_pri_port != NOT_DEFINED && 
			   (port == getMPortByCport(gDB.synce_sec_port)))  
			{
					gDB.synce_oper_port = get_synce_oper_port(port);
					zlog_notice("Synce Current interface %d", gDB.synce_oper_port);
			}

		}
		/*update QL*/
		PORT_STATUS[port].received_QL = QL;
	}
	else
	{
#ifdef DEBUG
		zlog_notice("%s:%d port %x Recevie QL %x QL %x oper_port %x, %x, %x", __func__, __LINE__, port, 
			    PORT_STATUS[port].received_QL, QL, gDB.synce_oper_port, 
			    gDB.synce_pri_port,gDB.synce_sec_port);
#endif

		if(PORT_STATUS[port].received_QL == QL)
			return 0;
		else/*compare pri/sec QL */
		{

			pri_port = getMPortByCport(gDB.synce_pri_port);
			sec_port = getMPortByCport(gDB.synce_sec_port);

			if(QL == 0x11 || QL == 0x21) 
			{
				if(pri_port == port)
				{
					zlog_notice("Recevie DNU.. port %d clear pri interface", port);
					gDB.synce_oper_port = get_synce_oper_port(sec_port);
					gCpssSynceIfConf(3, PRI_SRC, getCPortByMport(port), 0);

					if(gDB.synce_oper_port != NOT_DEFINED)
					{
						gCpssSynceIfConf(3, SEC_SRC, getCPortByMport(sec_port), 0);
						zlog_notice("Synce Current interface %d", gDB.synce_oper_port);
						gCpssSynceIfConf(3, SEC_SRC, getCPortByMport(sec_port), 1);
					}
				}
				else if(sec_port == port)
				{
					zlog_notice("Recevie DNU.. port %d clear sec interface", port);
					gDB.synce_oper_port = get_synce_oper_port(pri_port);
					gCpssSynceIfConf(3, SEC_SRC, getCPortByMport(port), 0);
					if(gDB.synce_oper_port != NOT_DEFINED)
					{
						gCpssSynceIfConf(3, PRI_SRC, getCPortByMport(pri_port), 0);
						zlog_notice("Synce Current interface %d", gDB.synce_oper_port);
						gCpssSynceIfConf(3, PRI_SRC, getCPortByMport(pri_port), 1);
					}
				}
				PORT_STATUS[port].received_QL = QL;
				return 0;
			}

			/*Check oper interface QL*/
			oper_port = gDB.synce_oper_port;
			if(oper_port != NOT_DEFINED)
			{
				/*received QL is better than oper interrface */
				if(compare_QL(QL, oper_port))
				{
					if(oper_port == pri_port) 
					{
						gCpssSynceIfConf(3, PRI_SRC, getCPortByMport(oper_port), 0);
						zlog_notice("port %x Recevie QL better than port %x", port, oper_port);
						gCpssSynceIfConf(3, PRI_SRC, getCPortByMport(oper_port), 1);
						gDB.synce_oper_port = port; 
						zlog_notice("Synce Current interface %d", gDB.synce_oper_port);
					}
					else
					{
						gCpssSynceIfConf(3, SEC_SRC, getCPortByMport(oper_port), 0);
						zlog_notice("port %x Recevie QL better than port %x", port, oper_port);
						gCpssSynceIfConf(3, SEC_SRC, getCPortByMport(oper_port), 1);
						gDB.synce_oper_port = port; 
						zlog_notice("Synce Current interface %d", gDB.synce_oper_port);
					}
				}

			}
			else
			{
				PORT_STATUS[port].received_QL = QL;
				if(gDB.synce_pri_port != NOT_DEFINED && 
				   (port == getMPortByCport(gDB.synce_pri_port)))  
				{
					gDB.synce_oper_port = get_synce_oper_port(port);
					zlog_notice("Synce Current interface %d", gDB.synce_oper_port);
				}

				if(gDB.synce_pri_port != NOT_DEFINED && 
				   (port == getMPortByCport(gDB.synce_sec_port)))  
				{
					gDB.synce_oper_port = get_synce_oper_port(port);
					zlog_notice("Synce Current interface %d", gDB.synce_oper_port);
				}

			}
			/*update QL*/
			PORT_STATUS[port].received_QL = QL;
		}
	}
	return 0;
}
#endif

uint8_t gReplyPortESMCQLupdate(int args, ...)
{
	uint8_t ret = IPC_CMD_SUCCESS;
	va_list argP;
	sysmon_fifo_msg_t *msg = NULL;
#if 1/*[#120] LOC Alarm process ¿¿, balkrow, 2024-10-16 */
	int mport;
#endif
	if(args !=  1) {
		zlog_notice("%s: invalid args[%d].", __func__, args);
		return IPC_CMD_FAIL;
	}

	va_start(argP, args);
	msg = va_arg(argP, sysmon_fifo_msg_t *);
	va_end(argP);
#if 0
	zlog_notice("port %d RX ESMC QL %x", msg->portid, msg->mode);
#endif
#if 1/*[#120] LOC Alarm process ¿¿, balkrow, 2024-10-16 */
	mport = getMPortByCport(msg->portid);

	if(gDB.esmcRxCfg[mport -1] == CFG_DISABLE)
		return ret;

#if 0
	zlog_notice("port %d RX ESMC QL %x esmc_recv_cnt %x port %x", msg->portid, msg->mode, PORT_STATUS[mport].esmc_recv_cnt, mport);
#endif
	if(mport != NOT_DEFINED) 
	{

		if(PORT_STATUS[mport].received_QL == 0x11 || PORT_STATUS[mport].received_QL == 0x21)  
		{
			if(msg->mode != 0x11 && msg->mode != 0x21) 
			{
				PORT_STATUS[mport].esmc_recv_cnt = 0; 	
				PORT_STATUS[mport].esmc_prev_cnt = 0; 	
			}

		}
		/*check first packet*/
		if(PORT_STATUS[mport].esmc_recv_cnt == 0) 
		{
			if(PORT_STATUS[mport].esmc_loss)
			{
				PORT_STATUS[mport].esmc_loss = 0; 
				zlog_notice("port %d CLEAR ESMC LOSS", mport);
			}

			if(msg->mode != 0x11 && msg->mode != 0x21) 
			{
				if(getMPortByCport(gDB.synce_pri_port) == mport)
				{
					gCpssSynceIfConf(3, PRI_SRC, getCPortByMport(mport), 0);
					zlog_notice("port %d Set ClockConfig", mport);
					gCpssSynceIfConf(3, PRI_SRC, getCPortByMport(mport), 1);
				}
				else if(getMPortByCport(gDB.synce_sec_port) == mport)
				{
					gCpssSynceIfConf(3, SEC_SRC, getCPortByMport(mport), 0);
					zlog_notice("port %d Set ClockConfig", mport);
					gCpssSynceIfConf(3, SEC_SRC, getCPortByMport(mport), 1);
				}

				PORT_STATUS[mport].esmc_recv_cnt = 0xf; 
			}
		}
		/*recv_cnt swap*/
		else if(PORT_STATUS[mport].esmc_recv_cnt == 0xf) 
			PORT_STATUS[mport].esmc_recv_cnt = 0xc; 
		else if(PORT_STATUS[mport].esmc_recv_cnt == 0xc) 
			PORT_STATUS[mport].esmc_recv_cnt = 0xf; 

	}

	switchEsmcInterface(mport, msg->mode);
#endif

	/* process for result. */
	if(msg->result == FIFO_CMD_SUCCESS) {
		/*regiter update*/
		if(msg->portid == gDB.synce_pri_port)
		{
			uint16_t val, wr_val;
			val = sys_fpga_memory_read(SYNCE_ESMC_RQL_ADDR, PORT_NOREG);
			val = (val & ~(0xff00));
			wr_val = (msg->mode << 8) | val; 
			sys_fpga_memory_write(SYNCE_ESMC_RQL_ADDR, wr_val, PORT_NOREG);
#if 0/*[#120] LOC Alarm process ¿¿, balkrow, 2024-10-16 */
			val = sys_fpga_memory_read(SYNCE_ESMC_SQL_ADDR, PORT_NOREG);
			val = (val & ~(0xff00));
			wr_val = (msg->mode << 8) | val; 
			sys_fpga_memory_write(SYNCE_ESMC_SQL_ADDR, wr_val, PORT_NOREG);
#endif
		}
		else if(msg->portid == gDB.synce_sec_port)
		{
			uint16_t val, wr_val;
			val = sys_fpga_memory_read(SYNCE_ESMC_RQL_ADDR, PORT_NOREG);
			val = (val & ~(0xff));
			wr_val =  val | msg->mode; 
			sys_fpga_memory_write(SYNCE_ESMC_RQL_ADDR, wr_val, PORT_NOREG);
#if 0/*[#120] LOC Alarm process ¿¿, balkrow, 2024-10-16 */
			val = sys_fpga_memory_read(SYNCE_ESMC_SQL_ADDR, PORT_NOREG);
			val = (val & ~(0xff));
			wr_val =  val | msg->mode; 
			sys_fpga_memory_write(SYNCE_ESMC_SQL_ADDR, wr_val, PORT_NOREG);
#endif
		}
#if 1/*[#120] LOC Alarm process ¿¿, balkrow, 2024-10-16 */
		{
			uint16_t val, wr_val;
			if(mport == M_PORT1)
			{
				val = sys_fpga_memory_read(SYNCE_ESMC_RQL2_ADDR, PORT_NOREG);
				val = (val & ~(0xff00));
				wr_val = (msg->mode << 8) | val; 
				sys_fpga_memory_write(SYNCE_ESMC_RQL2_ADDR, wr_val, PORT_NOREG);
			}
			else if(mport == M_PORT2)
			{
				val = sys_fpga_memory_read(SYNCE_ESMC_RQL2_ADDR, PORT_NOREG);
				val = (val & ~(0xff));
				wr_val =  val | msg->mode; 
				sys_fpga_memory_write(SYNCE_ESMC_RQL2_ADDR, wr_val, PORT_NOREG);
			}
			else if(mport == M_PORT3)
			{
				val = sys_fpga_memory_read(SYNCE_ESMC_RQL3_ADDR, PORT_NOREG);
				val = (val & ~(0xff00));
				wr_val = (msg->mode << 8) | val; 
				sys_fpga_memory_write(SYNCE_ESMC_RQL3_ADDR, wr_val, PORT_NOREG);
			}
			else if(mport == M_PORT4)
			{
				val = sys_fpga_memory_read(SYNCE_ESMC_RQL3_ADDR, PORT_NOREG);
				val = (val & ~(0xff));
				wr_val =  val | msg->mode; 
				sys_fpga_memory_write(SYNCE_ESMC_RQL3_ADDR, wr_val, PORT_NOREG);
			}
			else if(mport == M_PORT5)
			{
				val = sys_fpga_memory_read(SYNCE_ESMC_RQL4_ADDR, PORT_NOREG);
				val = (val & ~(0xff00));
				wr_val = (msg->mode << 8) | val; 
				sys_fpga_memory_write(SYNCE_ESMC_RQL4_ADDR, wr_val, PORT_NOREG);
			}
			else if(mport == M_PORT6)
			{
				val = sys_fpga_memory_read(SYNCE_ESMC_RQL4_ADDR, PORT_NOREG);
				val = (val & ~(0xff));
				wr_val =  val | msg->mode; 
				sys_fpga_memory_write(SYNCE_ESMC_RQL4_ADDR, wr_val, PORT_NOREG);
			}
			else if(mport == M_PORT7)
			{
				val = sys_fpga_memory_read(SYNCE_ESMC_RQL5_ADDR, PORT_NOREG);
				val = (val & ~(0xff00));
				wr_val = (msg->mode << 8) | val; 
				sys_fpga_memory_write(SYNCE_ESMC_RQL5_ADDR, wr_val, PORT_NOREG);
			}
		}
#endif
	} else
		zlog_notice("port %d ESMC QL update failed. ret[%d].", msg->portid, msg->result);
}
#endif

#if 1/*[#127] SYNCE current interface ¿¿, balkrow, 2024-09-12*/
uint8_t gReplyNone(int args, ...)
{
	args = args;
	return IPC_CMD_SUCCESS;
}

uint8_t gReplyPortSendQL(int args, ...)
{
	uint8_t ret = IPC_CMD_SUCCESS;
	va_list argP;
	sysmon_fifo_msg_t *msg = NULL;

#ifdef DEBUG
	zlog_notice("%s (REPLY): args=%d", __func__, args);
#endif
	if(args !=  1) {
		zlog_notice("%s: invalid args[%d].", __func__, args);
		return IPC_CMD_FAIL;
	}

	va_start(argP, args);
	msg = va_arg(argP, sysmon_fifo_msg_t *);
	va_end(argP);

	zlog_notice("%s  result=%x", __func__, msg->result);
	return ret;
}
uint8_t gReplyLocalQL(int args, ...)
{
	uint8_t ret = IPC_CMD_SUCCESS;
	va_list argP;
	sysmon_fifo_msg_t *msg = NULL;

#ifdef DEBUG
	zlog_notice("%s (REPLY): args=%d", __func__, args);
#endif
	if(args !=  1) {
		zlog_notice("%s: invalid args[%d].", __func__, args);
		return IPC_CMD_FAIL;
	}

	va_start(argP, args);
	msg = va_arg(argP, sysmon_fifo_msg_t *);
	va_end(argP);

	zlog_notice("%s  result=%x", __func__, msg->result);
	return ret;
}

uint8_t gReplySynceIfConf(int args, ...)
{
	uint8_t ret = IPC_CMD_SUCCESS;
	va_list argP;
	sysmon_fifo_msg_t *msg = NULL;

#ifdef DEBUG
	zlog_notice("%s (REPLY): args=%d", __func__, args);
#endif
	if(args !=  1) {
		zlog_notice("%s: invalid args[%d].", __func__, args);
		return IPC_CMD_FAIL;
	}

	va_start(argP, args);
	msg = va_arg(argP, sysmon_fifo_msg_t *);
	va_end(argP);

	zlog_notice("%s  result=%x", __func__, msg->result);
	return ret;
}
#endif

#if 1 /* [#152] Adding for port RS-FEC control, dustin, 2024-10-15 */
uint8_t gReplyPortFECEnable(int args, ...)
{
    uint8_t ret = IPC_CMD_SUCCESS;
    va_list argP;
    sysmon_fifo_msg_t *msg = NULL;

#ifdef DEBUG
    zlog_notice("%s (REPLY): args=%d", __func__, args);
#endif
    if(args != 1) {
        zlog_notice("%s: invalid args[%d].", __func__, args);
        return IPC_CMD_FAIL;
    }

    va_start(argP, args);
    msg = va_arg(argP, sysmon_fifo_msg_t *);
    va_end(argP);

    /* process for result. */
    if(msg->result != FIFO_CMD_SUCCESS)
        zlog_notice("Setting RS-FEC failed. ret[%d].", msg->result);
    return ret;
}
#endif /* [#152] */

#if 1 /* [#165] DCO SFP ¿¿ LLCF ¿¿, balkrow, 2024-10-24 */
uint8_t gReplyDcoSFPState(int args, ...)
{
	uint8_t ret = IPC_CMD_SUCCESS;
	va_list argP;
	sysmon_fifo_msg_t *msg = NULL;

#ifdef DEBUG
	zlog_notice("%s (REPLY): args=%d", __func__, args);
#endif
	if(args != 1) {
		zlog_notice("%s: invalid args[%d].", __func__, args);
		return IPC_CMD_FAIL;
	}

	va_start(argP, args);
	msg = va_arg(argP, sysmon_fifo_msg_t *);
	va_end(argP);

	/* process for result. */
	if(msg->result != FIFO_CMD_SUCCESS)
		zlog_notice("Setting Switch Mode failed. ret[%d].", msg->result);
	return ret;
}
#endif

#if 1 /* [#142] Adding for Transparent mode switching, dustin, 2024-10-11 */
uint8_t gReplySwitchModeSet(int args, ...)
{
	uint8_t ret = IPC_CMD_SUCCESS;
	va_list argP;
	sysmon_fifo_msg_t *msg = NULL;

#ifdef DEBUG
	zlog_notice("%s (REPLY): args=%d", __func__, args);
#endif
	if(args != 1) {
		zlog_notice("%s: invalid args[%d].", __func__, args);
		return IPC_CMD_FAIL;
	}

	va_start(argP, args);
	msg = va_arg(argP, sysmon_fifo_msg_t *);
	va_end(argP);

	/* process for result. */
	if(msg->result != FIFO_CMD_SUCCESS)
		zlog_notice("Setting Switch Mode failed. ret[%d].", msg->result);
	return ret;
}
#endif /* [#142] */

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
#if 1 /* [#91] Fixing for register updating feature, dustin, 2024-08-05 */
	gReplyPortPMFECClear,
#endif
#if 1/*[#73] SDK ¿¿¿ CPU trap ¿ packet ¿¿ ¿¿ ¿¿, balkrow, 2024-07-17*/
	gReplyPortESMCQLupdate,
#endif
#if 1/*[#127] SYNCE current interface ¿¿, balkrow, 2024-09-12*/
#if 0 /* [#142] Adding for Transparent mode switching, dustin, 2024-10-11 */
	/* below is added by mistake. above gReplyPortESMCQLupdate is real one. */
	gReplyNone,
#endif
	gReplyPortSendQL,
	gReplyLocalQL,
	gReplySynceIfConf,
#endif
#if 1 /* [#152] Adding for port RS-FEC control, dustin, 2024-10-15 */
    gReplyPortFECEnable,
#endif /* [#152] */
#if 1 /* [#142] Adding for Transparent mode switching, dustin, 2024-10-11 */
	gReplySwitchModeSet,
#endif
#if 1 /* [#165] DCO SFP ¿¿ LLCF ¿¿, balkrow, 2024-10-24 */
	gReplyDcoSFPState,
#endif
};

const uint32_t funcsListLen2 = sizeof(gSysmonReplyFuncs) / sizeof(cSysmonReplyFuncs);
static int sysmon_master_system_command(sysmon_fifo_msg_t * msg)
{
#if 1/*[#34] aldrin3s chip initial ¿¿ ¿¿, balkrow, 2024-05-23*/
#ifdef DEBUG
	zlog_notice("%s recv msg %x from cpss", __func__, msg->type);
#endif
#if 1/*[#59] Synce configuration ¿¿ ¿¿ ¿¿, balkrow, 2024-06-19 */
	if(msg->type > funcsListLen2 || msg->type < 0) 
		return RT_NOK;
#endif
#if 1/*[#24] Verifying syncE register update, dustin, 2024-05-28 */
	gSysmonReplyFuncs[msg->type](1, msg);
#endif

#endif
#if 1/*[#59] Synce configuration ¿¿ ¿¿ ¿¿, balkrow, 2024-06-19 */
	return RT_OK;
#endif
}

int sysmon_master_recv_fifo(struct thread *thread)
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

#if 0/*[#56] register update timer ¿¿, balkrow, 2023-06-13 */
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

	return;
}
#endif
