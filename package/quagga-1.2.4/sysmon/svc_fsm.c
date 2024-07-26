#include <zebra.h>
#if 0/*[#56] register update timer 수정, balkrow, 2023-06-13 */
#include "svc_fsm.h"
#endif
#if 1/*[#56] register update timer 수정, balkrow, 2023-06-13 */
#include "thread.h"
#endif

#if 1/*[#53] Clock source status ¿¿¿¿ ¿¿ ¿¿, balkrow, 2024-06-13*/
#include "sysmon.h"
#include "bp_regs.h"
#endif

#if 1/*[#34] aldrin3s chip initial ¿¿ ¿¿, balkrow, 2024-05-23*/
#include "sys_fifo.h"
extern uint8_t gAppDemoIPCstate;
extern cSysmonToCPSSFuncs gSysmonToCpssFuncs[];
uint16_t gSvcFSMretry = 0;
#undef DEBUG
#define CLR_RETRY_CNT gSvcFSMretry = 0
#endif

#if 1/*[#56] register update timer 수정, balkrow, 2023-06-13 */
extern int watch_get_pidof(char *procname);
extern int32_t syscmdrdfifo;
extern int32_t syscmdwrfifo;
extern struct thread_master *master;
extern int sysmon_master_recv_fifo(struct thread *thread);
extern uint16_t sysmonUpdateGetSWVer(void);

struct thread *fifo_th = NULL;
SVC_EVT svc_init(SVC_ST st);
SVC_EVT svc_init_fail(SVC_ST st);
SVC_EVT svc_appDemo_shutdown(SVC_ST st);
SVC_EVT svc_dpram_check(SVC_ST st);
SVC_EVT svc_fpga_check(SVC_ST st);
#endif

#if 1/*[#53] Clock source status ¿¿¿¿ ¿¿ ¿¿, balkrow, 2024-06-13*/
extern GLOBAL_DB gDB;
#endif

SVC_EVT svc_init(SVC_ST st) {
#if 1/*[#56] register update timer 수정, balkrow, 2023-06-13 */
	SVC_EVT rc;
#endif
	sysmon_fifo_msg_t msg;

	memset(&msg, 0, sizeof msg);
	msg.type = gHello;

#if 1/*[#56] register update timer 수정, balkrow, 2023-06-13 */
	if(syscmdrdfifo == UNINITIALIZED_FD)
	{
		if((syscmdrdfifo = open (SYSMON_FIFO_READ, O_RDWR)) < 0)
		{
			gAppDemoIPCstate = IPC_INIT_FAIL;
			rc = SVC_EVT_IPC_COM_FAIL;
			goto next_tries;
		}

		if(!fifo_th)	
			fifo_th = thread_add_read(master, sysmon_master_recv_fifo, NULL, syscmdrdfifo);
	}

	if(syscmdwrfifo == UNINITIALIZED_FD)
	{
		if((syscmdwrfifo = open (SYSMON_FIFO_WRITE, O_RDWR)) < 0)
		{
			gAppDemoIPCstate = IPC_INIT_FAIL;
			rc = SVC_EVT_IPC_COM_FAIL;
			goto next_tries;
		}
	}

	if(gSysmonToCpssFuncs[gHello](1, &msg) != FIFO_CMD_SUCCESS)
	{
		rc = SVC_EVT_IPC_COM_FAIL;
		goto next_tries;
	}
	else if(gDB.comm_state == COMM_FAIL)
	{
		rc = SVC_EVT_IPC_COM_FAIL;
		goto next_tries;
	}
	else if(gDB.comm_state == COMM_SUCCESS)
	{
		rc = SVC_EVT_IPC_COM_SUCCESS;
	}
	else if(!gDB.comm_state)
	{
		rc = SVC_EVT_IPC_COM_WAIT;
		goto next_tries;
	}

	return rc;

next_tries :
	if(gDB.fsm_retries++ > FSM_MAX_TRIES) 
	{

		gDB.fsm_retries = 0;
		if(syscmdrdfifo != UNINITIALIZED_FD)
		{
			close(syscmdrdfifo);
			syscmdrdfifo = UNINITIALIZED_FD;
		}

		if(syscmdwrfifo != UNINITIALIZED_FD)
		{
			close(syscmdwrfifo);
			syscmdwrfifo = UNINITIALIZED_FD;
		}

		if(fifo_th)
		{
			thread_cancel(fifo_th);
			fifo_th = NULL;
		}

		rc = SVC_EVT_IPC_COM_FAIL;
	}

	return rc;
#endif
}

SVC_EVT svc_init_fail(SVC_ST st) {
#if FSM_SIM
	return SVC_EVT_NONE;
#else /*! FSM_SIM*/
#if 1/*[#56] register update timer 수정, balkrow, 2023-06-13 */
	if(gDB.init_state != SYS_INIT_FAIL)
	{
		/* [#62] SFP eeprom 및 register update 기능 단위 검증 및 디버깅, balkrow, 2024-06-21 
		 *
		 * WRITE INIT COMPLETE (0x18)
		 * WRITE CPU FAIL (0x12)
		 * */
		gRegUpdate(INIT_COMPLETE_ADDR, 0, INIT_COMPLETE_ADDR_MASK, 0);
		gRegUpdate(CPU_FAIL_ADDR, 8, CPU_FAIL_MASK, 1);
#if 1/*[#53] Clock source status ¿¿¿¿ ¿¿ ¿¿, balkrow, 2024-06-13*/
		gDB.init_state = SYS_INIT_FAIL;  
#endif
	}


#if 1/*[#80] eag6l board SW bring-up, balkrow, 2023-07-24 */
	return SVC_EVT_INIT_FAIL;
#else
	return SVC_EVT_INIT;
#endif
#else /*! [#56]*/
	return SVC_EVT_NONE;
#endif /*End [#56]*/
#endif /*End FSM_SIM*/
}

#if 1/*[#56] register update timer 수정, balkrow, 2023-06-13 */
SVC_EVT svc_appDemo_shutdown(SVC_ST st) {

	if(syscmdrdfifo != UNINITIALIZED_FD)
	{
		close(syscmdrdfifo);
		syscmdrdfifo = UNINITIALIZED_FD;
	}

	if(syscmdwrfifo != UNINITIALIZED_FD)
	{
		close(syscmdwrfifo);
		syscmdwrfifo = UNINITIALIZED_FD;
	}

	if(fifo_th)
	{
		thread_cancel(fifo_th);
		fifo_th = NULL;
	}

	return SVC_EVT_INIT;
}
#endif

SVC_EVT svc_dpram_check(SVC_ST st) {
#if FSM_SIM
	return SVC_EVT_DPRAM_ACCESS_SUCCESS;
#else /*! FSM_SIM*/
#if 1/*[#56] register update timer 수정, balkrow, 2023-06-13 */
	SVC_EVT rc = SVC_EVT_DPRAM_ACCESS_FAIL;;
	const char *prog = "appDemo";
	/*write 0x2 0xaa*/
	DPRAM_WRITE(DPRAM_RDL_STATE, 0xaa);
	/*READ 0x2*/
	if(DPRAM_READ(DPRAM_RDL_STATE) == 0xaa)
		rc = SVC_EVT_DPRAM_ACCESS_SUCCESS;

	if(!watch_get_pidof(prog))
		rc = SVC_EVT_APPDEMO_SHUTDOWN;  

	return rc; 
#else /*! 56*/
	/* check dpram access*/
	return SVC_EVT_DPRAM_ACCESS_SUCCESS;
#endif /*End [#56]*/
#endif /*End FSM_SIM*/
}

SVC_EVT svc_fpga_check(SVC_ST st) {
#if FSM_SIM
	return SVC_EVT_FPGA_ACCESS_SUCCESS;
#else /*! FSM_SIM*/
#if 1/*[#56] register update timer 수정, balkrow, 2023-06-13 */
	SVC_EVT rc = SVC_EVT_FPGA_ACCESS_FAIL;
#if 1 /*[#62] SFP eeprom 및 register update 기능 단위 검증 및 디버깅, balkrow, 2024-06-21 */ 
	uint8_t i;
#endif
	uint16_t swVer = sysmonUpdateGetSWVer();

	/* [#62] SFP eeprom 및 register update 기능 단위 검증 및 디버깅, balkrow, 2024-06-21 
	 *
	 * WRITE UNIT VERSION(0xE)
	 * */
	FPGA_WRITE(SW_VERSION_ADDR, swVer);

	/*read SW version reg*/
	if(FPGA_READ(SW_VERSION_ADDR) == swVer)
		rc = SVC_EVT_FPGA_ACCESS_SUCCESS;

	/* [#62] SFP eeprom 및 register update 기능 단위 검증 및 디버깅, balkrow, 2024-06-21 
	 *
	 * WRITE UNIT DEFINED 1(0xA)
	 * */
	FPGA_WRITE(SW_VERSION_ADDR, 0x1);

	/* [#62] SFP eeprom 및 register update 기능 단위 검증 및 디버깅, balkrow, 2024-06-21 
	 *
	 * WRITE UNIT DEFINED 2(0xC)
	 * */
	FPGA_WRITE(SW_VERSION_ADDR, 0x164);
	/* [#62] SFP eeprom 및 register update 기능 단위 검증 및 디버깅, balkrow, 2024-06-21 
	 *
	 * WRITE COMMAND CONTROL REG2(SIG SEL, Rate SEL)(0x20)
	 * */
#if 1 /*[#62] SFP eeprom 및 register update 기능 단위 검증 및 디버깅, balkrow, 2024-06-21 */ 
	for(i = 0; i < PORT_ID_EAG6L_PORT6; i++) 
	{
		gRegUpdate(COMMON_CTRL2_P1_ADDR + (i*2), COMMON_CTRL2_SIG_SHIFT, 
			   COMMON_CTRL2_SIG_MASK, 0x1); /*GGE*/
		gRegUpdate(COMMON_CTRL2_P1_ADDR + (i*2), COMMON_CTRL2_RATE_SHIFT, 
			   COMMON_CTRL2_RATE_MASK, 0x7);/*25G*/
	}
#endif

	return rc; 
#else /*! 56*/
	/* check fpga access*/
	return SVC_EVT_FPGA_ACCESS_SUCCESS;
#endif /*End [#56]*/
#endif
}

SVC_EVT svc_cpld_check(SVC_ST st) {
#if FSM_SIM
	return SVC_EVT_CPLD_ACCESS_SUCCESS;
#else
	/* check cpld access*/
	return SVC_EVT_CPLD_ACCESS_SUCCESS;
#endif
}

SVC_EVT svc_sdk_init(SVC_ST st) {
#if FSM_SIM
	return SVC_EVT_SDK_INIT_SUCCESS;
#else
	SVC_EVT evt;
	sysmon_fifo_msg_t msg;

	memset(&msg, 0, sizeof msg);
	msg.type = gSDKInit;

	/* check sdk_init result*/
	if(gSysmonToCpssFuncs[gSDKInit](1, &msg) == FIFO_CMD_SUCCESS)
#if 1/*[#80] eag6l board SW bring-up, balkrow, 2023-07-24 */
		evt = SVC_EVT_SDK_INIT_WAIT;
#else
		evt = SVC_EVT_SDK_INIT_SUCCESS;
#endif
	else	
		evt = SVC_EVT_SDK_INIT_FAIL;

#ifdef DEBUG
	zlog_notice("called %s ret=%d", __func__, evt);
#endif
		return evt;
#endif
}


SVC_EVT svc_get_inven(SVC_ST st) {
#if FSM_SIM
	return SVC_EVT_GET_INVEN_SUCCESS;
#else
#if 1/* [#78] Adding system inventory management, dustin, 2024-07-24 */
	extern void process_hw_inventory_infos(void);

	/* read/update board inventory information. */
	process_hw_inventory_infos();
#endif
	return SVC_EVT_GET_INVEN_SUCCESS;
#endif
}

#if 1/*[#80] eag6l board SW bring-up, balkrow, 2023-07-22 */
SVC_EVT svc_sdk_init_wait(SVC_ST st) {
	SVC_EVT evt = SVC_EVT_SDK_INIT_WAIT;

	if(gDB.sdk_init_state == SDK_INIT_DONE)
		evt = SVC_EVT_SDK_INIT_SUCCESS;
	else if(gDB.sdk_init_state == SDK_INIT_FAIL)
		evt = SVC_EVT_SDK_INIT_FAIL;

#ifdef DEBUG
	zlog_notice("called %s evt %d sdk_init_state %x", __func__, evt, gDB.sdk_init_state);
#endif

	return evt;
}
#endif

SVC_EVT svc_init_done(SVC_ST st) {
#if 1/*[#53] Clock source status ¿¿¿¿ ¿¿ ¿¿, balkrow, 2024-06-13*/
#if 1/*[#56] register update timer 수정, balkrow, 2023-06-13 */
	SVC_EVT evt = SVC_EVT_NONE;
	const char *prog = "appDemo";
	if(gDB.init_state != SYS_INIT_DONE)
	{
#if 1/* [#70] Adding RDL feature, dustin, 2024-07-02 */
		{
			extern void rdl_update_bank_registers(void);

			// update bp os bank info registers.
			rdl_update_bank_registers();
		}
#endif
		gDB.init_state = SYS_INIT_DONE;  
		/* [#62] SFP eeprom 및 register update 기능 단위 검증 및 디버깅, balkrow, 2024-06-21 
		 *
		 * WRITE INIT COMPLETE (0x18)
		 * */
		gRegUpdate(INIT_COMPLETE_ADDR, 0, INIT_COMPLETE_ADDR_MASK, SYS_INIT_DONE);
	}
	else
	{
		/* check appDemo process */
		if(!watch_get_pidof(prog))
			evt = SVC_EVT_APPDEMO_SHUTDOWN;  
	}
#else /*! [#56]*/
	gDB.init_state = SYS_INIT_DONE;  
	gRegUpdate(INIT_COMPLETE_ADDR, 0, INIT_COMPLETE_ADDR_MASK, SYS_INIT_DONE);
#endif /*End [#56]*/
#endif
	return evt;
}

#if 1/*[#80] eag6l board SW bring-up, balkrow, 2023-07-22 */
SVC_ST transition(SVC_ST state, SVC_EVT event) {

	SVC_ST ret = SVC_ST_INIT_FAIL;

	switch (state) {

	case SVC_ST_INIT:
		if(event == SVC_EVT_IPC_COM_SUCCESS)
			ret = SVC_ST_SDK_INIT; 
#if 1/*[#56] register update timer 수정, balkrow, 2023-06-13 */
		else if(event == SVC_EVT_IPC_COM_WAIT)
			ret = SVC_ST_INIT; 
		else if(event == SVC_EVT_INIT)
			ret = SVC_ST_INIT; 
		else 
			goto init_fail;
#else
		else if(gSvcFSMretry++ < SVC_FSM_INIT_MAX_RETRY) 
			ret = SVC_ST_INIT; 
		else {
			CLR_RETRY_CNT;
			goto init_fail;
		}
#endif
		break;
#if 1/*[#56] register update timer 수정, balkrow, 2023-06-13 */
	case SVC_ST_INIT_FAIL:
		if(event == SVC_EVT_INIT)
			ret = SVC_ST_INIT;
		else
			goto init_fail;
		break;
#endif
	case SVC_ST_DPRAM_CHK:
		if(event == SVC_EVT_DPRAM_ACCESS_SUCCESS)
			ret = SVC_ST_FPGA_CHK; 
		else if (event == SVC_EVT_DPRAM_ACCESS_FAIL)
			goto init_fail;
#if 1/*[#56] register update timer 수정, balkrow, 2023-06-13 */
		else if (event == SVC_EVT_APPDEMO_SHUTDOWN)
			ret = SVC_ST_APPDEMO_SHUTDOWN; 
#endif
		/*		
		 *TODO: 			
		else
			return SVC_ST_DPRAM_CHK;		
		*/
		break;
	case SVC_ST_FPGA_CHK:
		if(event == SVC_EVT_FPGA_ACCESS_SUCCESS)
			ret = SVC_ST_CPLD_CHK; 
		else if (event == SVC_EVT_FPGA_ACCESS_FAIL)
			goto init_fail;
#if 1/*[#56] register update timer 수정, balkrow, 2023-06-13 */
		else if (event == SVC_EVT_APPDEMO_SHUTDOWN)
			ret = SVC_ST_APPDEMO_SHUTDOWN; 
#endif
		/*		
		 *TODO: 			
		else
			return SVC_ST_DPRAM_CHK;		
		*/
		break;

	case SVC_ST_CPLD_CHK:
		if(event == SVC_EVT_CPLD_ACCESS_SUCCESS)
			ret = SVC_ST_GET_INVEN; 
		else if (event == SVC_EVT_CPLD_ACCESS_FAIL)
			goto init_fail;
#if 1/*[#56] register update timer 수정, balkrow, 2023-06-13 */
		else if (event == SVC_EVT_APPDEMO_SHUTDOWN)
			ret = SVC_ST_APPDEMO_SHUTDOWN; 
#endif
		/*		
		 *TODO: 			
		else
			return SVC_ST_DPRAM_CHK;		
		*/
		break;

	case SVC_ST_SDK_INIT:
#if 0/*[#80] eag6l board SW bring-up, balkrow, 2023-07-24 */
		if(event == SVC_EVT_SDK_INIT_SUCCESS)
			ret = SVC_ST_DPRAM_CHK;
		else if (event == SVC_EVT_SDK_INIT_FAIL)
			goto init_fail;
#else
		if (event == SVC_EVT_SDK_INIT_FAIL)
			goto init_fail;
#endif
#if 1/*[#56] register update timer 수정, balkrow, 2023-06-13 */
		else if (event == SVC_EVT_APPDEMO_SHUTDOWN)
			ret = SVC_ST_APPDEMO_SHUTDOWN; 
#endif
#if 1/*[#80] eag6l board SW bring-up, balkrow, 2023-07-24 */
		else if (event == SVC_EVT_SDK_INIT_WAIT)
			ret = SVC_ST_SDK_INIT_CHK;		
#endif
		break;

#if 1/*[#80] eag6l board SW bring-up, balkrow, 2023-07-24 */
	case SVC_ST_SDK_INIT_CHK:
		if(event == SVC_EVT_SDK_INIT_SUCCESS)
			ret = SVC_ST_DPRAM_CHK;
		else if (event == SVC_EVT_SDK_INIT_FAIL)
			goto init_fail;
		else if (event == SVC_EVT_APPDEMO_SHUTDOWN)
			ret = SVC_ST_APPDEMO_SHUTDOWN; 
		else if (event == SVC_EVT_SDK_INIT_WAIT)
			ret = SVC_ST_SDK_INIT_CHK;		
		break;
#endif
	case SVC_ST_GET_INVEN:
		if(event == SVC_EVT_GET_INVEN_SUCCESS)
			ret = SVC_ST_INIT_DONE; 
		else if (event == SVC_EVT_GET_INVEN_FAIL)
			goto init_fail;
#if 1/*[#56] register update timer 수정, balkrow, 2023-06-13 */
		else if (event == SVC_EVT_APPDEMO_SHUTDOWN)
			ret = SVC_ST_APPDEMO_SHUTDOWN; 
#endif
		/*		
		 *TODO: 			
		else
			return SVC_ST_DPRAM_CHK;		
		*/
		break;
#if 1/*[#34] aldrin3s chip initial ¿¿ ¿¿, balkrow, 2024-05-23*/
	case SVC_ST_INIT_DONE:
#if 1/*[#56] register update timer 수정, balkrow, 2023-06-13 */
		if(event == SVC_EVT_APPDEMO_SHUTDOWN)
			ret = SVC_ST_APPDEMO_SHUTDOWN; 
		else
#endif
		ret = SVC_ST_INIT_DONE; 
		break;
#endif
#if 1/*[#56] register update timer 수정, balkrow, 2023-06-13 */
	case SVC_ST_APPDEMO_SHUTDOWN:
		if(event == SVC_EVT_APPDEMO_SHUTDOWN)
			ret = SVC_ST_APPDEMO_SHUTDOWN; 
#endif
		break;
	default:
		/*		
		 * TODO: 			
			return SVC_ST_DPRAM_CHK;		
		*/
		break;
	}

normal_return:
	return ret;
init_fail:
	return SVC_ST_INIT_FAIL;
}
#endif
