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
#define DEBUG
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
		/*No write Bp Init done*/
#if 1/*[#53] Clock source status ¿¿¿¿ ¿¿ ¿¿, balkrow, 2024-06-13*/
		gDB.init_state = SYS_INIT_FAIL;  
#endif
	}


	return SVC_EVT_INIT;
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
	uint16_t swVer = sysmonUpdateGetSWVer();

	/*write SW version reg*/
	FPGA_WRITE(SW_VERSION_ADDR, swVer);

	/*read SW version reg*/
	if(FPGA_READ(SW_VERSION_ADDR) == swVer)
		rc = SVC_EVT_FPGA_ACCESS_SUCCESS;

	/*write unit defined 1 reg*/
	FPGA_WRITE(SW_VERSION_ADDR, 0x1);

	/*write unit defined 2 reg*/
	FPGA_WRITE(SW_VERSION_ADDR, 0x164);

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
		evt = SVC_EVT_SDK_INIT_SUCCESS;
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
	/* check dpram access*/
	return SVC_EVT_GET_INVEN_SUCCESS;
#endif
}

#if 1/*[#56] register update timer 수정, balkrow, 2023-06-13 */
#endif

SVC_EVT svc_init_done(SVC_ST st) {
#if 1/*[#53] Clock source status ¿¿¿¿ ¿¿ ¿¿, balkrow, 2024-06-13*/
#if 1/*[#56] register update timer 수정, balkrow, 2023-06-13 */
	SVC_EVT evt = SVC_EVT_NONE;
	const char *prog = "appDemo";
	if(gDB.init_state != SYS_INIT_DONE)
	{
		gDB.init_state = SYS_INIT_DONE;  
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


SVC_ST transition(SVC_ST state, SVC_EVT event) {

	SVC_ST ret = SVC_ST_INIT_FAIL;

	switch (state) {

	case SVC_ST_INIT:
		if(event == SVC_EVT_IPC_COM_SUCCESS)
			ret = SVC_ST_DPRAM_CHK; 
#if 1/*[#56] register update timer 수정, balkrow, 2023-06-13 */
		else if(event == SVC_EVT_IPC_COM_WAIT)
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
			ret = SVC_ST_SDK_INIT; 
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
		if(event == SVC_EVT_SDK_INIT_SUCCESS)
			ret = SVC_ST_GET_INVEN; 
		else if (event == SVC_EVT_SDK_INIT_FAIL)
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
