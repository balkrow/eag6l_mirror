#include <zebra.h>
#include "svc_fsm.h"
/*
SVC_FSM svc_fsm[] = 
{
	{SVC_ST_INIT,		SVC_EVT_INIT,	svc_dpram_check},
	{SVC_ST_INIT,		SVC_EVT_INIT,	svc_dpram_check},
	{SVC_ST_DPRAM_CHK,	SVC_EVT_DPRAM_ACCESS_FAIL,	svc_init_fail},
	{SVC_ST_DPRAM_CHK,	SVC_EVT_DPRAM_ACCESS_SUCCESS,	svc_fpga_check},
	{SVC_ST_FPGA_CHK,	SVC_EVT_FPGA_ACCESS_FAIL,	svc_init_fail},
	{SVC_ST_FPGA_CHK,	SVC_EVT_FPGA_ACCESS_SUCCESS,	svc_cpld_check},
};
*/

#if 1/*[#34] aldrin3s chip initial ¿¿ ¿¿, balkrow, 2024-05-23*/
#include "sys_fifo.h"
extern uint8_t gAppDemoIPCstate;
extern cSysmonToCPSSFuncs gSysmonToCpssFuncs[];
uint16_t gSvcFSMretry = 0;
#define DEBUG
#define CLR_RETRY_CNT gSvcFSMretry = 0
#endif

SVC_EVT svc_init(SVC_ST st) {
	sysmon_fifo_msg_t msg;

	memset(&msg, 0, sizeof msg);
	msg.type = gHello;

#if 1/*[#34] aldrin3s chip initial ¿¿ ¿¿, balkrow, 2024-05-23*/
	if(gAppDemoIPCstate == IPC_INIT_FAIL) 
		return SVC_EVT_IPC_COM_FAIL;
	else if(gSysmonToCpssFuncs[gHello](1, &msg) == FIFO_CMD_SUCCESS)
		return SVC_EVT_IPC_COM_SUCCESS;
	else
		return SVC_EVT_IPC_COM_FAIL;
#endif
}

SVC_EVT svc_init_fail(SVC_ST st) {
#if FSM_SIM
	return SVC_EVT_NONE;
#else
	/*write BP fail*/
	return SVC_EVT_NONE;
#endif
}

SVC_EVT svc_dpram_check(SVC_ST st) {
#if FSM_SIM
	return SVC_EVT_DPRAM_ACCESS_SUCCESS;
#else
	/* check dpram access*/
	return SVC_EVT_DPRAM_ACCESS_SUCCESS;
#endif
}

SVC_EVT svc_fpga_check(SVC_ST st) {
#if FSM_SIM
	return SVC_EVT_FPGA_ACCESS_SUCCESS;
#else
	/* check fpga access*/
	return SVC_EVT_FPGA_ACCESS_SUCCESS;
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

SVC_EVT svc_init_done(SVC_ST st) {
	return SVC_EVT_NONE;
}


SVC_ST transition(SVC_ST state, SVC_EVT event) {

	SVC_ST ret = SVC_ST_INIT_FAIL;

	switch (state) {

	case SVC_ST_INIT:
		if(event == SVC_EVT_IPC_COM_SUCCESS)
			ret = SVC_ST_DPRAM_CHK; 
		else if(gSvcFSMretry++ < SVC_FSM_INIT_MAX_RETRY) 
			ret = SVC_ST_INIT; 
		else {
			CLR_RETRY_CNT;
			goto init_fail;
		}
		break;
	case SVC_ST_DPRAM_CHK:
		if(event == SVC_EVT_DPRAM_ACCESS_SUCCESS)
			ret = SVC_ST_FPGA_CHK; 
		else if (event == SVC_EVT_DPRAM_ACCESS_FAIL)
			goto init_fail;
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
		/*		
		 *TODO: 			
		else
			return SVC_ST_DPRAM_CHK;		
		*/
		break;
#if 1/*[#34] aldrin3s chip initial ¿¿ ¿¿, balkrow, 2024-05-23*/
	case SVC_ST_INIT_DONE:
		ret = SVC_ST_INIT_DONE; 
		break;
#endif
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
