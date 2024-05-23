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

SVC_EVT svc_init(SVC_ST st) {
	return SVC_EVT_NONE;
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
	/* check sdk_init result*/
	return SVC_EVT_SDK_INIT_SUCCESS;
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

	switch (state) {

	case SVC_ST_INIT:
		if(event == SVC_EVT_INIT)
			return SVC_ST_DPRAM_CHK; 
		else
			return SVC_ST_INIT_FAIL; 
		break;
	case SVC_ST_DPRAM_CHK:
		if(event == SVC_EVT_DPRAM_ACCESS_SUCCESS)
			return SVC_ST_FPGA_CHK; 
		else if (event == SVC_EVT_DPRAM_ACCESS_FAIL)
			return SVC_ST_INIT_FAIL; 
		/*		
		 *TODO: 			
		else
			return SVC_ST_DPRAM_CHK;		
		*/
		break;
	case SVC_ST_FPGA_CHK:
		if(event == SVC_EVT_FPGA_ACCESS_SUCCESS)
			return SVC_ST_CPLD_CHK; 
		else if (event == SVC_EVT_FPGA_ACCESS_FAIL)
			return SVC_ST_INIT_FAIL; 
		/*		
		 *TODO: 			
		else
			return SVC_ST_DPRAM_CHK;		
		*/
		break;

	case SVC_ST_CPLD_CHK:
		if(event == SVC_EVT_CPLD_ACCESS_SUCCESS)
			return SVC_ST_SDK_INIT; 
		else if (event == SVC_EVT_CPLD_ACCESS_FAIL)
			return SVC_ST_INIT_FAIL; 
		/*		
		 *TODO: 			
		else
			return SVC_ST_DPRAM_CHK;		
		*/
		break;

	case SVC_ST_SDK_INIT:
		if(event == SVC_EVT_SDK_INIT_SUCCESS)
			return SVC_ST_GET_INVEN; 
		else if (event == SVC_EVT_SDK_INIT_FAIL)
			return SVC_ST_INIT_FAIL; 
		/*		
		 *TODO: 			
		else
			return SVC_ST_DPRAM_CHK;		
		*/
		break;

	case SVC_ST_GET_INVEN:
		if(event == SVC_EVT_GET_INVEN_SUCCESS)
			return SVC_ST_INIT_DONE; 
		else if (event == SVC_EVT_GET_INVEN_FAIL)
			return SVC_ST_INIT_FAIL; 
		/*		
		 *TODO: 			
		else
			return SVC_ST_DPRAM_CHK;		
		*/
		break;
	default:
		/*		
		 * TODO: 			
			return SVC_ST_DPRAM_CHK;		
		*/
		break;
	}
}
