#include "sysmon.h"
#include "log.h"
#include "thread.h"
#include "rdl_fsm.h"

RDL_INFO_LIST rdl_info_list = {0, };

SV_ST rdl_start() //#1
{
    RDL_ST state = RDL_ST_START;
	zlog_warn("");
// RDL start ack
/*
	FPGA_WRITE(ADDR, RDL_START_ACK_BIT);
*/
    sv_info_list.st = state;
    return state;
}



SV_ST rdl_writing_p1() //#2
{	
	RDL_ST state = RDL_WRITING_P1;
	zlog_warn("");
//RDL writing P-1 ack
/*
	FPGA_WRITE(ADDR, RDL_P1_WRITING_ACK_BIT);
*/
	sv_info_list.st = state;
	return state;
}


SV_ST rdl_reading_p1() //#3
{
    RDL_ST state = RDL_READING_P1;
	zlog_warn("");
//RDL writing P-1 Done ack
//RDL reading P-1
//write file
/*
    FPGA_WRITE(ADDR, RDL_P1_WRITING_DONE_ACK_BIT);
//Page CRC check
	if(r_ok==1)
		FPGA_WRITE(ADDR, RDL_P1_READING_DONE_BIT);
*/
    sv_info_list.st = state;
    return state;
}


SV_ST mcu_rdl_writing_err_p1() //#4
{
    RDL_ST state = RDL_ST_START;
	zlog_warn("");
//page writing error ack
//Go to rdl start state
/*
    FPGA_WRITE(ADDR, RDL_P1_WRITING_ERROR_ACK_BIT);
*/
    sv_info_list.st = state;
    return state;
}


SV_ST rdl_writing_p2() //#5
{
    RDL_ST state = RDL_WRITING_P2;
	zlog_warn("");
//RDL writing P-2 ack 
/*
    FPGA_WRITE(ADDR, RDL_P2_WRITING_ACK_BIT);
*/
    sv_info_list.st = state;
    return state;
}


SV_ST rdl_reading_err_p1() //#6
{
    RDL_ST state = RDL_ST_START;
	zlog_warn("");
//RDL reading error
//Go to rdl start state
/*
    FPGA_WRITE(ADDR, RDL_P1_READING_ERROR_BIT);
*/
    sv_info_list.st = state;
    return state;
}


SV_ST rdl_reading_p2() //#7
{
    RDL_ST state = RDL_READING_P2;
	zlog_warn("");
//writing P-2 Done ack
//reading P-2 
//one's complement
//write file
/*
    FPGA_WRITE(ADDR, VALUE);
*/
    sv_info_list.st = state;
    return state;
}


SV_ST mcu_rdl_writing_err_p2() //#8
{
    RDL_ST state = RDL_READING_P1;
	zlog_warn("");
//page writing error ack
//Go to p1 reading done state
/*
    FPGA_WRITE(ADDR, VALUE);
*/
    sv_info_list.st = state;
    return state;
}


SV_ST check_total_page_done() //#9
{
    RDL_ST state = RDL_WRITING_TOTAL;
	zlog_warn("");
//total reading done
/*
    FPGA_WRITE(ADDR, VALUE);
*/
    sv_info_list.st = state;
    return state;
}


SV_ST rdl_reading_err_p2() //#10
{
    RDL_ST state = RDL_READING_P1;
	zlog_warn("");
//page reading error
// Go to p2 reading state
/*
    FPGA_WRITE(ADDR, VALUE);
*/
    sv_info_list.st = state;
    return state;
}


SV_ST rdl_reading_total() //#11
{
    RDL_ST state = RDL_READING_TOTAL;
	zlog_warn("");
//rdl reading total
/*
    FPGA_WRITE(ADDR, VALUE);
*/
    sv_info_list.st = state;
    return state;
}


SV_ST next_page() //#12
{
    RDL_ST state = RDL_ST_START;
	zlog_warn("");
//rel reading total not done
//go to next page	
    sv_info_list.st = state;
    return state;
}



SV_ST img_activation() //#13
{
    RDL_ST state = RDL_IMG_ACTIVE_DONE;
	zlog_warn("");
// image activation		
//FW image ??
    sv_info_list.st = state;
    return state;
}


SV_ST rdl_state_init() //#14
{
    RDL_ST state = RDL_ST_TERM;
	zlog_warn("");
// total error. go go the rdl start
    sv_info_list.st = state;
    return state;
}

SV_ST rdl_sw_reset() //#15
{
	RDL_ST state = RDL_IMG_RUNNING_CHECK;
	zlog_warn("");
// sw-reset register check and reset
	sv_info_list.st =state;
	return state;	
}

SV_ST rdl_running_succes() //#16
{
 // how ??
	return state;
}

SV_ST rdl_running_fail()  //#17
{
 // how ??
	return state;
}



RDL_FSM rdl_fsm_list[RDL_TRANS_MAX] =
{
	{RDL_ST_IDLE,                   MCU_RDL_START,	                	rdl_start},


	{RDL_ST_START,                  MCU_RDL_WRITING_P1,	             	rdl_writing_p1},


	{RDL_WRITING_P1,			    MCU_RDL_WRITING_DONE_P1,            rdl_reading_p1},
	{RDL_WRITING_P1,                MCU_RDL_WRIGTING_ERROR, 	        mcu_rdl_writing_err_p1},


	{RDL_READING_P1,     			MCU_RDL_WRITING_P2,             	rdl_writing_p2},
	{RDL_READING_P1,			    RDL_READING_ERROR,		            rdl_reading_err_p1},


	{RDL_WRITING_P2,                MCU_RDL_WRITING_DONE_P2,            rdl_reading_p2},
	{RDL_WRITING_P2,                MCU_RDL_WRIGTING_ERROR,		        mcu_rdl_writing_err_p2},


	{RDL_READING_P2,                RDL_READING_DONE_P2,                check_total_page_done},
	{RDL_READING_P2,              	RDL_READING_ERROR,	                rdl_reading_err_p2},


	{RDL_WRITING_TOTAL,             MCU_WRITING_DONE_TOTAL,             rdl_reading_total},
	{RDL_WRITING_TOTAL,             MCU_WRITING_NOT_DONE,		        next_page},
	{RDL_WRITING_TOTAL,             MCU_WRITING_FAIL_TOTAL,             rdl_state_init},


	{RDL_READING_TOTAL,             RDL_READING_DONE_TOTAL,             img_activation},
	{RDL_READING_TOTAL,             RDL_READING_FAIL_TOTAL,             rdl_state_init},


	{RDL_IMG_ACTIVE_DONE,    	    RDL_IMG_ACTIVE_SUCCESS,             rdl_sw_reset},
	{RDL_IMG_ACTIVE_DONE,           RDL_IMG_ACTIVE_FAIL, 	            rdl_state_init},

	{RDL_IMG_RUNNING_CHECK,         RDL_IMG_RUNNING_SUCCESS, 	        rdl_running_success},
	{RDL_IMG_RUNNING_CHECK,         RDL_IMG_RUNNING_FAIL, 	            rdl_running_fail},

	
};
