
/* 
	Copyright ¡§I 2014, by HFR,Inc. All Right Reserved.

	This Software of copyright is reserved by HFR.Inc. No all or part of this Software may be reproduced by  the 
	hard-copy format, electronically, or otherwise and transmitted to Ineligible Person. If not you should be subject 
	to civil suit and criminal case according to circumstances.

	º» ÆÄÀÏÀÇ ÀúÀÛ±ÇÀº (ÁÖ)¿¡Ä¡¿¡ÇÁ¾Ë¿¡ ÀÖ½À´Ï´Ù. (ÁÖ)¿¡Ä¡¿¡ÇÁ¾ËÀÇ ¸í½ÃÀûÀÎ µ¿ÀÇ¾øÀÌ º» ÆÄÀÏÀÇ ÀüÃ¼ ¶Ç´Â ÀÏºÎ¸¦ 
	ÇÏµåÄ«ÇÇ Çü½Ä, ÀüÀÚÀû ¹æ½Ä ¶Ç´Â ±âÅ¸¹æ½ÄÀ¸·Î Àç»ý»êÇÏ°Å³ª ¹Þ¾Æº¼ ¼ö ÀÖ´Â ÀÚ°ÝÀÌ ¾ø´Â »ç¶÷¿¡°Ô Àç¹èÆ÷ÇÏ´Â ÇàÀ§´Â
	ÀúÀÛ±Ç¹ýÀ» À§¹ÝÇÏ´Â °ÍÀÌ¸ç ¹Î»ç»ó ¼ÕÇØ¹è»ó ±×¸®°í ÇØ´çÇÏ´Â °æ¿ì¿¡´Â Çü»ç»ó Ã³¹úÀÇ ´ë»óÀÌ µË´Ï´Ù
*/
#include "zebra.h"
#include "sysmon.h" 
#include "bp_regs.h" 
#include "sys_fifo.h" 
#include <getopt.h>
#include "log.h" 
#include "thread.h" 

#if 0/*[#61] Adding omitted functions, dustin, 2024-06-24 */
#define ACCESS_SIM	/* moved to sysmon.h */
#endif
#undef DEBUG

#if 0/*[#53] Clock source status ¿¿¿¿ ¿¿ ¿¿, balkrow, 2024-06-13*/
u32 INIT_COMPLETE_FLAG;
int32_t hdrv_fd;
#endif
struct thread_master *master;
//const char *pid_file = PATH_SYSMON_PID;
char* progname;

#if 1/*[#54] Adding Smart T-SFP I2C functions, dustin, 2024-06-13 */
int i2c_in_use_flag;
#endif

#if 1/*[#26] system managent FSM ¿¿, balkrow, 2024-05-20*/
extern  SVC_ST transition(SVC_ST state, SVC_EVT event);
extern SVC_EVT svc_init(SVC_ST st);
extern SVC_EVT svc_init_fail(SVC_ST st);
extern SVC_EVT svc_dpram_check(SVC_ST st);
extern SVC_EVT svc_fpga_check(SVC_ST st);
extern SVC_EVT svc_cpld_check(SVC_ST st);
extern SVC_EVT svc_sdk_init(SVC_ST st);
extern SVC_EVT svc_get_inven(SVC_ST st);
extern SVC_EVT svc_init_done(SVC_ST st);
#if 1/*[#56] register update timer ¿¿, balkrow, 2023-06-13 */
extern SVC_EVT svc_appDemo_shutdown(SVC_ST st);
#endif

#if 0
SVC_FSM svc_fsm;
#endif
void init_svc_fsm(void);
#endif


#ifdef DEBUG
void print_console(const char *fmt, ...)
{
	va_list args;
	char *p;
	p = malloc(1024);
	FILE * fp = NULL;
	va_start (args, fmt);
	vsprintf (p,  fmt, args);
	fp = fopen("/dev/console", "w");
	fprintf(fp, p);
	va_end (args);
	fclose(fp);

	free(p);
}
#else
void print_console(const char *fmt, ...)
{

}
#endif

#if 1/*[#53] Clock source status ¿¿¿¿ ¿¿ ¿¿, balkrow, 2024-06-13*/
extern int8_t rsmu_init (void);
extern int8_t rsmu_pll_update(void);
GLOBAL_DB gDB;
#endif

#if 1/*[#56] register update timer ¿¿, balkrow, 2023-06-13 */
extern void update_sfp(void);
#endif

void sigint (int sig) {
	/* TODO. signal */
	exit(0);
}


int test_timer_func(struct thread *thread) {
	thread_add_timer (master, test_timer_func, NULL, 1);
#ifdef DEBUG
	print_console("test...\n");	
#endif
	return 0;
}

#if 0/*[#54] Adding Smart T-SFP I2C functions, dustin, 2024-06-13 */
#if 1/*[#25] I2C related register update, dustin, 2024-05-28 */
int sfp_timer_func(struct thread *thread)
{
	thread_add_timer (master, (int)sfp_timer_func, NULL, 10);

	{
		extern void update_sfp(void);
		update_sfp();
	}
	return 0;
}
#endif

#if 0/*[#4] Register updating, dustin, 2024-05-28 */
int reg_timer_func(struct thread *thread)
{
	 
	thread_add_timer (master, (int)reg_timer_func, NULL, 10);

	{
		extern void update_bp_reg(void);
		update_bp_reg();
	}
	return 0;
}
#endif
#endif

#if 1/*[#26] system managent FSM ¿¿, balkrow, 2024-05-20*/
void init_svc_fsm(void) {
	gDB.svc_fsm.state = SVC_ST_INIT;
	gDB.svc_fsm.evt = SVC_EVT_INIT;

	/*TODO: must be mapping function*/	
	gDB.svc_fsm.cb[SVC_ST_INIT] = svc_init;
	gDB.svc_fsm.cb[SVC_ST_INIT_FAIL] = svc_init_fail;
#if 1/*[#56] register update timer ¿¿, balkrow, 2023-06-13 */
	gDB.svc_fsm.cb[SVC_ST_APPDEMO_SHUTDOWN] = svc_appDemo_shutdown;
#endif
	gDB.svc_fsm.cb[SVC_ST_DPRAM_CHK] = svc_dpram_check;
	gDB.svc_fsm.cb[SVC_ST_FPGA_CHK] = svc_fpga_check;
	gDB.svc_fsm.cb[SVC_ST_CPLD_CHK] = svc_cpld_check;
	gDB.svc_fsm.cb[SVC_ST_SDK_INIT] = svc_sdk_init;
	gDB.svc_fsm.cb[SVC_ST_GET_INVEN] = svc_get_inven;
	gDB.svc_fsm.cb[SVC_ST_INIT_DONE] = svc_init_done;
}


int svc_fsm_timer(struct thread *thread) {
	SVC_ST state;
	SVC_EVT event;

	state = transition(gDB.svc_fsm.state, gDB.svc_fsm.evt);

	if(gDB.svc_fsm.cb[state] != NULL)
		event = gDB.svc_fsm.cb[state](state);

	gDB.svc_fsm.state = state;
	gDB.svc_fsm.evt = event;
#ifdef DEBUG
	zlog_notice("FSM state=%x, evt=%x", gDB.svc_fsm.state, gDB.svc_fsm.evt);
#endif

	thread_add_timer_msec (master, svc_fsm_timer, NULL, 100);
	return 0;
}
#endif

#if 1/*[#53] Clock source status ¿¿¿¿ ¿¿ ¿¿, balkrow, 2024-06-13*/
int8_t monitor_hw_timer(struct thread *thread)
{
#if 0/*[#71] EAG6L Board Bring-up, balkrow, 2024-07-16*/
	if(gDB.init_state != SYS_INIT_DONE) {
		goto __SKIP_1__;
	}
#endif

	/*PLL state*/
	rsmu_pll_update();

#if 1/*[#71] EAG6L Board Bring-up, balkrow, 2024-07-16*/
	thread_add_timer_msec (master, monitor_hw_timer, NULL, 300);
#endif
	return RT_OK;
}	
#endif

#if 1/*[#56] register update timer ¿¿, balkrow, 2023-06-13 */
int8_t reg_fast_intv_update(struct thread *thread)
{
extern void pm_request_counters(void);

	/*update KeepAlive reg*/
	update_KeepAlive();

	if(gDB.init_state != SYS_INIT_DONE) {
		goto __SKIP_2__;
	}

	pm_request_counters();

	/* read per-port spf port status/control. */
	update_port_sfp_information();

	/* process per-port performance info from sdk. */
	process_port_pm_counters();

__SKIP_2__:
	thread_add_timer_msec (master, reg_fast_intv_update, NULL, 500);

	return RT_OK;
}

int8_t reg_slow_intv_update(struct thread *thread)
{
#if 1/*[#54] Adding Smart T-SFP I2C functions, dustin, 2024-06-13 */
extern void update_bp_reg(void);
extern void update_sfp(void);
extern uint16_t portAlarm(void);

	if(gDB.init_state != SYS_INIT_DONE) {
		goto __SKIP_3__;
	}

	if(! i2c_in_use_flag)
#endif
	update_sfp();

#if 0/* [#72] Adding omitted rtWDM related registers, dustin, 2024-06-27 */
	/* removing unnecessary codes */
#if 1/*[#61] Adding omitted functions, dustin, 2024-06-24 */
	/* for initial operation */
#ifdef ACCESS_SIM
	if(! i2c_in_use_flag)
	{
extern ePrivateSfpId get_private_sfp_identifier(int portno);
extern port_status_t PORT_STATUS[PORT_ID_EAG6L_MAX];
extern struct module_inventory INV_TBL[PORT_ID_EAG6L_MAX];
		static one_time_flag = 0;
		int portno, type;

		if(! one_time_flag) {
			for(portno = PORT_ID_EAG6L_PORT1; portno < PORT_ID_EAG6L_MAX; portno++) {
				type = get_private_sfp_identifier(portno);
				PORT_STATUS[portno].sfp_type = type;
				read_port_inventory(portno, &(INV_TBL[portno]));
			}
			one_time_flag = 1;
		}
	}
#endif
#endif
#endif/*[#72]*/

	/* get port alarm (link) */
	portAlarm();

__SKIP_3__:
	thread_add_timer (master, reg_slow_intv_update, NULL, 1);
	return RT_OK;
}

int8_t monMCUupdate(struct thread *thread)
{
	if(gDB.init_state != SYS_INIT_DONE) {
		goto __SKIP_4__;
	}

	regMonitor();

__SKIP_4__:
	thread_add_timer_msec (master, monMCUupdate, NULL, 100);
	return RT_OK;
}
#endif

#if 1/*[#56] register update timer ¿¿, balkrow, 2023-06-13 */
uint16_t sysmonUpdateGetSWVer(void)
{
	/*TODO: real data*/
	return 0x100;
}
#endif

/* Allocate new sys structure and set default value. */
void sysmon_thread_init (void)
{
#if 1/*[#26] system managent FSM ¿¿, balkrow, 2024-05-20*/
	thread_add_timer (master, svc_fsm_timer, NULL, 1);
#endif
#if 0/*[#25] I2C related register update, dustin, 2024-05-28 */
	thread_add_timer (master, sfp_timer_func, NULL, 10);
#endif
#if 0/*[#4] Register updating, dustin, 2024-05-28 */
	thread_add_timer (master, reg_timer_func, NULL, 10);
#endif

#if 1/*[#56] register update timer ¿¿, balkrow, 2023-06-13 */
	thread_add_timer (master, monMCUupdate, NULL, 1);
	thread_add_timer (master, reg_fast_intv_update, NULL, 2);
	thread_add_timer (master, reg_slow_intv_update, NULL, 3);
#endif

#if 1/*[#53] Clock source status ¿¿¿¿ ¿¿ ¿¿, balkrow, 2024-06-13*/
	thread_add_timer (master, monitor_hw_timer, NULL, 1);
#endif	
}

int init_rlimit(void)
{

	struct rlimit limit;

	getrlimit(RLIMIT_CORE, &limit);

	limit.rlim_cur = RLIM_INFINITY;
	limit.rlim_max = RLIM_INFINITY;
	setrlimit(RLIMIT_CORE, &limit);

	getrlimit(RLIMIT_CORE, &limit);

	return 0;
}

void sysmon_init(void) {
#if 1/* [#69] Adding omitted rtWDM related registers, dustin, 2024-06-27 */
extern int8_t monitor_hw_init(void);
#endif

#if 0
	/* clear init complete flag */
	INIT_COMPLETE_FLAG = 0;
#endif
#if 1/*[#53] Clock source status ¿¿¿¿ ¿¿ ¿¿, balkrow, 2024-06-13*/
	memset(&gDB, 0, sizeof(gDB));
#endif

	zlog_notice("init sysmon");
#if 1/*[#53] Clock source status ¿¿¿¿ ¿¿ ¿¿, balkrow, 2024-06-13*/
	monitor_hw_init();
#endif
#if 1/*[#26] system managent FSM ¿¿, balkrow, 2024-05-20*/
#if 0/*[#56] register update timer ¿¿, balkrow, 2023-06-13 */
	sysmon_master_fifo_init ();
#endif
	init_svc_fsm();
#endif
	sysmon_thread_init();

#if 0
	/* set init complete flag */
	INIT_COMPLETE_FLAG = 1;
#endif
}
