#include "zebra.h"
#include "sysmon.h"
#include "bp_regs.h"
#include "log.h"
#include "thread.h"
#include "rdl_fsm.h"
#include <sys/statfs.h>

#define DEBUG

RDL_INFO_LIST_t rdl_info_list = {0, };
extern uint8_t PAGE_CRC_OK;
extern uint8_t IMG_ACTIVATION_OK;
extern uint8_t IMG_RUNNING_OK;
extern uint8_t *RDL_PAGE;
extern RDL_IMG_INFO_t RDL_INFO;

extern RDL_CRC_t rdl_check_page_crc(void);
extern void rdl_copy_page_segment_to_buffer(int sno);
extern int rdl_save_page_to_img_file(uint32_t size);
extern long rdl_get_file_size(char *filename);
extern int rdl_read_img_info(RDL_IMG_INFO_t *pinfo);
extern void rdl_update_bank_registers(void);
extern int get_pkg_fwheader_info(char *fpath, fw_header_t *hdr);
extern int check_img_file_crc(char *fpath, fw_image_header_t *hdr, 
	           uint32_t *t_size, uint32_t *t_crc);
extern int rdl_decompress_package_file(char *filename);
extern int rdl_install_package(int bno);
extern int rdl_activate_bp(int bno);
extern int rdl_activate_fpga(uint8_t bno);
extern uint16_t chipReset(uint16_t port, uint16_t val);
extern int syscmd_file_exist(char *fpath);
extern uint16_t get_sum(uint16_t *addr, int32_t nleft);


RDL_ST_t rdl_start(void) //#1
{
    RDL_ST_t state = ST_RDL_START;
	struct statfs fst;
	FILE *fp = NULL;
	char temp[150];
#ifdef DEBUG
	//zlog_notice("------> %s : entered.", __func__);
#endif
	// RDL start ack
	// MCU start RDL, BP read img header info registers, init page buff, 
	//     check free space and ack it. 
	// read rdl img header info.
	memset(&RDL_INFO, 0, sizeof RDL_INFO);
	if(rdl_read_img_info(&RDL_INFO) < 0) {
		zlog_err("%s : Invalid RDL Info. Go to IDLE.", __func__);
		memset(&RDL_INFO, 0, sizeof RDL_INFO);
		return ST_RDL_IDLE;
	}

	// check if target file is already present in system.
	sprintf(temp, "%s%s", RDL_IMG_PATH, RDL_INFO.hd.file_name);
	if(syscmd_file_exist(temp)) {
		zlog_err("%s : Already present file[%s]. Removed for start.",
			__func__, temp);
		unlink(temp);
		system("sync");
	}

	// check free space for rdl. Go idle state if not available.
	if(statfs(RDL_IMG_PATH, &fst) != 0) {
		zlog_err("%s : Cannot stat for %s. reason[%s]. Go to IDLE.",
			__func__, RDL_IMG_PATH, strerror(errno));
		return ST_RDL_IDLE;
	}

#if 0//PWY_FIXME need to tune size.
	if(fst.f_bavail < (100 * 1024)) {
		zlog_err("%s : Not enough sapce in %s. Free space %u. Go to IDLE.",
			__func__, RDL_IMG_PATH, fst.f_bavail);
		return ST_RDL_IDLE;
	}
#ifdef DEBUG
	//zlog_notice("------> %s : Free space[%u].", __func__, fst.f_bavail);
#endif
#endif //PWY_FIXME

	// get temp page buffer.
	RDL_PAGE = malloc(RDL_PAGE_SIZE);
	if(RDL_PAGE == NULL) {
		zlog_err("%s : Cannot get RDL page buffer.", __func__);
		gDPRAMRegUpdate(RDL_STATE_RESP_ADDR, 8, 0xFF00, 0xFF);
		state = ST_RDL_TERM;
	}
#ifdef DEBUG
//	zlog_notice("------> %s : Got page buffer. size[%u].", 
//		__func__, RDL_PAGE_SIZE);
#endif

	// reset current page no and ack for rdl start.
	gDPRAMRegUpdate(RDL_STATE_RESP_ADDR, 0, 0x00FF, 0/*default-page*/);
	gDPRAMRegUpdate(RDL_STATE_RESP_ADDR, 8, 0xFF00, RDL_START_ACK_BIT);
    rdl_info_list.st = state;
    return state;
}


RDL_ST_t rdl_writing_p1(void) //#2
{	
	RDL_ST_t state = ST_RDL_WRITING_P1;
	uint16_t val;
#ifdef DEBUG
	//zlog_notice("------> %s : entered.", __func__);
#endif
	// RDL writing P-1 ack
	// MCU writing P-1, BP get current page no and ack it.
	// read/update current page no.
	val = DPRAM_READ(RDL_STATE_REQ_ADDR);
	RDL_INFO.pno = val & 0xFF;

	// save current page no.
	gDPRAMRegUpdate(RDL_STATE_RESP_ADDR, 0, 0x00FF, RDL_INFO.pno);
	// ack writing p-1.
	gDPRAMRegUpdate(RDL_STATE_RESP_ADDR, 8, 0xFF00, RDL_P1_WRITING_ACK_BIT);

	rdl_info_list.st = state;
	return state;
}


RDL_ST_t rdl_reading_p1(void) //#3
{
    RDL_ST_t state = ST_RDL_READING_P1;
	pid_t pid;

#ifdef DEBUG
	//zlog_notice("------> %s : entered.", __func__);
#endif
#if 1//PWY_FIXME
	// RDL writing P-1 Done ack / RDL reading P-1
	// MCU done writing P-1, BP copy P-1 data and ack it. No checking CRC for P1.

	// copy P-1 to buffer.
	rdl_copy_page_segment_to_buffer(RDL_PAGE_SEGMENT_1);

	gDPRAMRegUpdate(RDL_STATE_RESP_ADDR, 8, 0xFF00, RDL_P1_WRITING_DONE_ACK_BIT);
    rdl_info_list.st = state;
	return state;
#else//////////////////////////////////////////////////////////////////
	// RDL writing P-1 Done ack / RDL reading P-1
	// MCU done writing P-1, BP ack it first, and copy P-1 data. No checking CRC for P1.
	//     This will let MCU start writing P-2 while BP processing P-1.
	//     But BP need to fork a process to process P-1 data, and should change state 
	//     because MCU need a writing P-2 Ack from BP.
	gDPRAMRegUpdate(RDL_STATE_RESP_ADDR, 8, 0xFF00, RDL_P1_WRITING_DONE_ACK_BIT);
    rdl_info_list.st = state;

	// fork a process to call a function to copy dpram page 1 to buffer.
	pid = fork();
	//////////////////////////////////////////////////////////////////
	// forked process ////////////////////////////////////////////////
	if(pid > 0) { // parent process
    	rdl_info_list.st = state;
		return state;
	} else if(pid == 0) { // child process
#ifdef DEBUG
		//zlog_notice("------> %s : forked process copying p-1.", __func__);
#endif
		// copy P-1 to buffer.
		rdl_copy_page_segment_to_buffer(RDL_PAGE_SEGMENT_1);
		exit(1);
	}
	else { // fork fail case
#ifdef DEBUG
		zlog_err("%s : forking p-1 failed. reason[%s].", __func__, strerror(errno));
#endif
		state = ST_RDL_TERM;
    	rdl_info_list.st = state;
		return state;
	}/////////////////////////////////////////////////////////////////
#endif //PWY_FIXME
}


RDL_ST_t rdl_writing_err_p1(void) //#4
{
    RDL_ST_t state = ST_RDL_START;
#ifdef DEBUG
	//zlog_notice("------> %s : entered.", __func__);
#endif
	// page writing error ack
	// MCU go to rdl start state, BP just waiting state change.
	gDPRAMRegUpdate(RDL_STATE_RESP_ADDR, 8, 0xFF00, RDL_PAGE_WRITING_ERROR_ACK_BIT);
    rdl_info_list.st = state;
    return state;
}


RDL_ST_t rdl_writing_p2(void) //#5
{
    RDL_ST_t state = ST_RDL_WRITING_P2;
#ifdef DEBUG
	//zlog_notice("------> %s : entered.", __func__);
#endif
	// RDL writing P-2 ack 
	// MCU write P-2, BP just ack it.
	gDPRAMRegUpdate(RDL_STATE_RESP_ADDR, 8, 0xFF00, RDL_P2_WRITING_ACK_BIT);
    rdl_info_list.st = state;
    return state;
}


RDL_ST_t rdl_reading_p2(void) //#7
{
	pid_t pid;
	uint32_t total_size, page_size;

    RDL_ST_t state = ST_RDL_READING_P2;
#ifdef DEBUG
	//zlog_notice("------> %s : entered.", __func__);
#endif
	// RDL writing P-2 Done ack / RDL reading P-2
	// MCU done writing P-2, BP copy data to buffer, check page CRC and ack it.
	//     and BP fork a process to write page data to img file.

	// ack for p2 writing done.
	gDPRAMRegUpdate(RDL_STATE_RESP_ADDR, 8, 0xFF00, RDL_P2_WRITING_DONE_ACK_BIT);

	// copy P-2 to buffer.
	rdl_copy_page_segment_to_buffer(RDL_PAGE_SEGMENT_2);

	// check page data.
	PAGE_CRC_OK = rdl_check_page_crc();
	if(PAGE_CRC_OK) {
		// get total size.
		total_size = RDL_INFO.hd.total_size;

		// get the size of current page.
		if(RDL_INFO.pno < RDL_INFO.total_pno)
			page_size = RDL_PAGE_SIZE;
		else
			page_size = total_size % RDL_PAGE_SIZE;
		if(! page_size)
			page_size = RDL_PAGE_SIZE;
#ifdef DEBUG
//		zlog_notice("%s : total_size[%u] pno[%u] page_size[%u]",
//			__func__, total_size, RDL_INFO.pno, page_size);
#endif
#if 1//PWY_FIXME
		// save page data to img file.
		rdl_save_page_to_img_file(page_size);

		rdl_info_list.st = state;
		return state;
#else//////////////////////////////////////////////////////////
		// fork a process to save page data to img file.
		pid = fork();
		//////////////////////////////////////////////////////////////////
		// forked process ////////////////////////////////////////////////
		if(pid > 0) { // parent process
			rdl_info_list.st = state;
			return state;
		}
		else if(pid == 0) {// child process
#ifdef DEBUG
			//zlog_notice("------> %s : forked process copying p-2.", __func__);
#endif
			// save page data to img file.
			rdl_save_page_to_img_file(page_size);

			exit(0);
		}
		else if(pid == -1) {
#ifdef DEBUG
			zlog_err("%s : forking p-2 failed. reason[%s].", __func__, strerror(errno));
#endif
			state = ST_RDL_TERM;
			rdl_info_list.st = state;
			return state;
		}/////////////////////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////
#endif //PWY_FIXME
	} else {
		// nack for page reading.
		gDPRAMRegUpdate(RDL_STATE_RESP_ADDR, 8, 0xFF00, RDL_PAGE_READING_ERROR_BIT);

		//FIXME : necessary to clear page buffer?

		rdl_info_list.st = state;
		return state;
	}
}


RDL_ST_t rdl_writing_err_p2(void) //#8
{
    RDL_ST_t state = ST_RDL_START;
#ifdef DEBUG
	//zlog_notice("------> %s : entered.", __func__);
#endif
	// RDL page writing error ack
	// MCU go to rdl start state
	gDPRAMRegUpdate(RDL_STATE_RESP_ADDR, 8, 0xFF00, RDL_PAGE_WRITING_ERROR_ACK_BIT);
    rdl_info_list.st = state;
    return state;
}


RDL_ST_t rdl_page_done(void) //#9
{
    RDL_ST_t state = ST_RDL_WRITING_TOTAL;
#ifdef DEBUG
	//zlog_notice("------> %s : entered.", __func__);
#endif
	// page reading done
	// MCU go next page writing P-1, or total writing done
	gDPRAMRegUpdate(RDL_STATE_RESP_ADDR, 8, 0xFF00, RDL_PAGE_READING_DONE_BIT);
    rdl_info_list.st = state;
    return state;
}


RDL_ST_t rdl_reading_err_p2(void) //#10
{
    RDL_ST_t state = ST_RDL_START;
#ifdef DEBUG
	//zlog_notice("------> %s : entered.", __func__);
#endif
	// page reading error
	// MCU go to P-1 writing state
	gDPRAMRegUpdate(RDL_STATE_RESP_ADDR, 8, 0xFF00, RDL_PAGE_READING_ERROR_BIT);
    rdl_info_list.st = state;
    return state;
}


RDL_ST_t rdl_reading_total(void) //#11
{
    RDL_ST_t state = ST_RDL_READING_TOTAL;
	long file_size;
#ifdef DEBUG
	//zlog_notice("------> %s : entered.", __func__);
#endif
	// rdl reading total
	// BP check file size.
	// check file size with total size.
	file_size = rdl_get_file_size(RDL_INFO.hd.file_name);
	if(file_size != RDL_INFO.hd.total_size) {
		zlog_notice("%s : Different file size [%u vs %u]. Go to IDLE.",
			__func__, file_size, RDL_INFO.hd.total_size);

		gDPRAMRegUpdate(RDL_STATE_RESP_ADDR, 8, 0xFF00, RDL_TOTAL_READING_ERROR_BIT);
		return ST_RDL_IDLE;
	}

	gDPRAMRegUpdate(RDL_STATE_RESP_ADDR, 8, 0xFF00, RDL_TOTAL_READING_DONE_BIT);
    rdl_info_list.st = state;
    return state;
}


RDL_ST_t rdl_next_page(void) //#12
{
    RDL_ST_t state = ST_RDL_START;
#ifdef DEBUG
	//zlog_notice("------> %s : entered.", __func__);
#endif
	// rdl reading total not done
	// MCU go to next page
    rdl_info_list.st = state;
    return state;
}


RDL_ST_t rdl_img_activation(void) //#13
{
    RDL_ST_t state = ST_RDL_ACTIVATE_DONE;
	struct statfs fst;
	int bno = RDL_INFO.bno;
#ifdef DEBUG
	//zlog_notice("------> %s : entered.", __func__);
#endif
	// BP check if disk space is enough for processing.
	// check free space for rdl. Go idle state if not available.
	if(statfs(RDL_IMG_PATH, &fst) != 0) {
		zlog_err("%s : Cannot stat for %s. reason[%s]. Go to IDLE.",
			__func__, RDL_IMG_PATH, strerror(errno));
		return ST_RDL_IDLE;
	}

#if 0//PWY_FIXME
	if(fst.f_bavail < (65 * 1024)) {
		zlog_err("%s : Not enough sapce in %s. Free space %u. Go to IDLE.",
			__func__, RDL_IMG_PATH, fst.f_bavail);
		return ST_RDL_IDLE;
	}
#endif //PWY_FIXME

	// BP extract FPGA F/W img and BP os img files, and remove integrated img file.
	if(rdl_decompress_package_file(RDL_INFO.hd.file_name) < 0) {
		//FIXME : success or failed, pkg file will be removed anyway.
		zlog_err("%s : Decompressing pkg file %s has failed. Go to IDLE.",
			__func__, RDL_INFO.hd.file_name);
		return ST_RDL_IDLE;
	}

	// BP install(replace bank files) BP os img to target bank.
	rdl_install_package(bno);

	// BP copy os img to /flash/boot for activation.
	rdl_activate_bp(bno);
	
	// BP update OS related registers.
	rdl_update_bank_registers();

	// BP check version and download to FPGA F/W ?
	// FIXME : do it~!!
	rdl_activate_fpga(bno);

    rdl_info_list.st = state;
    return state;
}


RDL_ST_t rdl_restart(void) //#14
{
    RDL_ST_t state = ST_RDL_IDLE;
#ifdef DEBUG
	//zlog_notice("------> %s : entered.", __func__);
#endif
	// total error.
	// MCU go to rdl idle, BP clean-up temp img file, page buffer, rld info.
	// FIXME : do it~!!
    rdl_info_list.st = state;
    return state;
}

RDL_ST_t rdl_sw_reset(void) //#15
{
	RDL_ST_t state = ST_RDL_RUNNING_CHECK;
#ifdef DEBUG
	//zlog_notice("------> %s : entered.", __func__);
#endif
	// sw-reset register check and reset
	// FIXME : always auto reset ?? save a flag file to notify to check running os ??
#if 0//PWY_FIXME
	chipReset(0/*meaningless*/, 0xA5);
#endif //PWY_FIXME
	rdl_info_list.st = state;
	return state;	
}

RDL_ST_t rdl_running_success(void) //#16
{
	RDL_ST_t state = ST_RDL_TERM;
#ifdef DEBUG
	//zlog_notice("------> %s : entered.", __func__);
#endif
 	// how ??
 	// FIXME : remove a flag file if running imsg was booted ok ?
	rdl_info_list.st = state;
	return state;
}

RDL_ST_t rdl_running_fail()  //#17
{
	RDL_ST_t state = ST_RDL_TERM;
#ifdef DEBUG
	//zlog_notice("------> %s : entered.", __func__);
#endif
 	// how ??
 	// FIXME : failed if flag file is still present ?
	rdl_info_list.st = state;
	return state;
}

RDL_ST_t rdl_terminate(void) //#18
{
    RDL_ST_t state = ST_RDL_IDLE;
#ifdef DEBUG
	//zlog_notice("------> %s : entered.", __func__);
#endif
	// terminate state.
	// rdl done anyway (success or failure)
	// FIXME : clean-up buffer? remove temp image file? reset rdl registers?
    rdl_info_list.st = state;
    return state;
}

char *RDL_STATE_STR[] = {
	"ST_RDL_IDLE",
	"ST_RDL_START",
	"ST_RDL_WRITING_P1",
	"ST_RDL_READING_P1",
	"ST_RDL_WRITING_P2",
	"ST_RDL_READING_P2",
	"ST_RDL_WRITING_TOTAL",
	"ST_RDL_READING_TOTAL",
	"ST_RDL_ACTIVATE_DONE",
	"ST_RDL_RUNNING_CHECK",
	"ST_RDL_TERM",
	"ST_RDL_ST_MAX",
};

char *RDL_EVENT_STR[] = {
	"EVT_RDL_NONE",
	"EVT_RDL_START",
	"EVT_RDL_WRITING_P1",
	"EVT_RDL_WRITING_DONE_P1",
	"EVT_RDL_WRITING_P2",
	"EVT_RDL_WRITING_DONE_P2",
	"EVT_RDL_WRITING_ERROR",
	"EVT_RDL_READING_DONE_P2",
	"EVT_RDL_READING_ERROR",
	"EVT_RDL_WRITING_DONE_TOTAL",
	"EVT_RDL_WRITING_NOT_DONE",
	"EVT_RDL_WRITING_ERROR_TOTAL",
	"EVT_RDL_READING_DONE_TOTAL",
	"EVT_RDL_READING_ERROR_TOTAL",
	"EVT_RDL_IMG_ACTIVE_SUCCESS",
	"EVT_RDL_IMG_ACTIVE_FAIL",
	"EVT_RDL_IMG_RUNNING_SUCCESS",
	"EVT_RDL_IMG_RUNNING_FAIL",
	"EVT_RDL_EVT_MAX",
};

char *rdl_get_state_str(int sno)
{
	if((sno < ST_RDL_IDLE) || (sno > ST_RDL_ST_MAX))
		return "INVALID STATE";
	return (RDL_STATE_STR[sno]);
}

char *rdl_get_event_str(int eno)
{
	if((eno < EVT_RDL_NONE) || (eno > EVT_RDL_EVT_MAX))
		return "INVALID EVENT";
	return (RDL_EVENT_STR[eno]);
}

RDL_FSM_t rdl_fsm_list[] =
{
	{ST_RDL_IDLE,               EVT_RDL_START,	                rdl_start},

	{ST_RDL_START,              EVT_RDL_WRITING_P1,	            rdl_writing_p1},

	{ST_RDL_WRITING_P1,			EVT_RDL_WRITING_DONE_P1,        rdl_reading_p1},
	{ST_RDL_WRITING_P1,         EVT_RDL_WRITING_ERROR,  	    rdl_writing_err_p1},

	{ST_RDL_READING_P1,         EVT_RDL_WRITING_P2,             rdl_writing_p2},

	{ST_RDL_WRITING_P2,         EVT_RDL_WRITING_DONE_P2,        rdl_reading_p2},
	{ST_RDL_WRITING_P2,         EVT_RDL_WRITING_ERROR,		    rdl_writing_err_p2},

	{ST_RDL_READING_P2,         EVT_RDL_READING_DONE_P2,        rdl_page_done},
	{ST_RDL_READING_P2,         EVT_RDL_READING_ERROR,	        rdl_reading_err_p2},

	{ST_RDL_WRITING_TOTAL,      EVT_RDL_WRITING_DONE_TOTAL,     rdl_reading_total},
	{ST_RDL_WRITING_TOTAL,      EVT_RDL_WRITING_NOT_DONE,       rdl_next_page},
	{ST_RDL_WRITING_TOTAL,      EVT_RDL_WRITING_ERROR_TOTAL,    rdl_restart},

	{ST_RDL_READING_TOTAL,      EVT_RDL_READING_DONE_TOTAL,     rdl_img_activation},
	{ST_RDL_READING_TOTAL,      EVT_RDL_READING_ERROR_TOTAL,    rdl_restart},

	{ST_RDL_ACTIVATE_DONE,      EVT_RDL_IMG_ACTIVE_SUCCESS,     rdl_sw_reset},
	{ST_RDL_ACTIVATE_DONE,      EVT_RDL_IMG_ACTIVE_FAIL, 	    rdl_terminate},

	{ST_RDL_RUNNING_CHECK,      EVT_RDL_IMG_RUNNING_SUCCESS, 	rdl_running_success},
	{ST_RDL_RUNNING_CHECK,      EVT_RDL_IMG_RUNNING_FAIL, 	    rdl_running_fail},

	{ST_RDL_TERM,               EVT_RDL_NONE, 	                rdl_terminate},
};

int RDL_TRANS_MAX = sizeof(rdl_fsm_list) / sizeof(RDL_FSM_t);


