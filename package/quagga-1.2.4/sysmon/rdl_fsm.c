#include "zebra.h"
#include "sysmon.h"
#include "bp_regs.h"
#include "log.h"
#include "thread.h"
#include "rdl_fsm.h"
#include <sys/statfs.h>
#if 1/*[#110] RDL function Debugging 및 수정, balkrow, 2024-09-02*/ 
#include <pthread.h>
#endif

#define DEBUG

RDL_INFO_LIST_t rdl_info_list = {0, };
#if 1/* [#77] Adding RDL emulation function, dustin, 2024-07-16 */
RDL_INFO_LIST_t emul_info_list = {0, };
#endif
extern uint8_t PAGE_CRC_OK;
#if 0 /* [#105] Fixing for RDL install/activation process, dustin, 2024-08-27 */
extern uint8_t IMG_ACTIVATION_OK;
extern uint8_t IMG_RUNNING_OK;
#endif
extern uint8_t *RDL_PAGE;
extern RDL_IMG_INFO_t RDL_INFO;

extern RDL_CRC_t rdl_check_page_crc(void);
extern void rdl_copy_page_segment_to_buffer(int sno);
extern int rdl_save_page_to_img_file(uint32_t size);
extern long rdl_get_file_size(char *filename);
extern int rdl_read_img_info(RDL_IMG_INFO_t *pinfo);
#if 1 /* [#89] Fixing for RDL changes on Target system, dustin, 2024-08-02 */
#if 1 /* [#105] Fixing for RDL install/activation process, dustin, 2024-08-27 */
extern void rdl_update_bank_registers(int bno, int erase_flag);
#else
extern void rdl_update_bank_registers(int bno);
#endif
#else
extern void rdl_update_bank_registers(void);
#endif
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

#if 1 /* [#124] Fixing for 3rd registers update, dustin, 2024-09-09 */
extern struct timeval ktt1;
extern struct timeval ktt2;
extern struct timeval ktt3;
extern struct timeval ktt4;
extern uint8_t zone1, zone2, zone3, zone4; /* flags for timeout zones. */
#endif

#if 1 /* [#89] Fixing for RDL changes on Target system, dustin, 2024-08-02 */
#if 1 /* [#105] Fixing for RDL install/activation process, dustin, 2024-08-27 */
uint16_t RDL_INSTALL_STATE;
uint16_t RDL_B1_ERASE_FLAG;
uint16_t RDL_B2_ERASE_FLAG;
#else
uint16_t RDL_ACTIVATION_STATE;
#endif
#endif

#if 1 /* [#110] RDL function Debugging ¿ ¿¿, balkrow, 2024-09-02 */
extern GLOBAL_DB gDB;
pthread_t thread_t;
#endif

RDL_ST_t rdl_start(void) //#1
{
    RDL_ST_t state = ST_RDL_START;
#if 1 /* [#89] Fixing for RDL changes on Target system, dustin, 2024-08-02 */
	struct timeval sss;
#endif
	struct statfs fst;
	FILE *fp = NULL;
	char temp[150];
#ifdef DEBUG
	//zlog_notice("------> %s : entered.", __func__);
#endif
#if 1 /* [#89] Fixing for RDL changes on Target system, dustin, 2024-08-02 */
	gettimeofday(&sss, NULL);
    zlog_notice("%s : ----> RDL Start time[%ld sec].", __func__, sss.tv_sec);
#if 1 /* [#124] Fixing for 3rd registers update, dustin, 2024-09-09 */
	ktt1 = sss;
	zone1 = 1;
	zone2 = zone3 = zone4 = 0;
#endif /* [#124] */
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

#if 1 /* [#124] Fixing for 3rd registers update, dustin, 2024-09-09 */
	/* NOTE : hd is not available in this moment, so do it later. */
#else /**************************************************************/
	// check if target file is already present in system.
	sprintf(temp, "%s%s", RDL_IMG_PATH, RDL_INFO.hd.file_name);
	if(syscmd_file_exist(temp)) {
		zlog_err("%s : Already present file[%s]. Removed for start.",
			__func__, temp);
		unlink(temp);
		system("sync");
	}
#endif

	// check free space for rdl. Go idle state if not available.
	if(statfs(RDL_IMG_PATH, &fst) != 0) {
		zlog_err("%s : Cannot stat for %s. reason[%s]. Go to IDLE.",
			__func__, RDL_IMG_PATH, strerror(errno));
		return ST_RDL_IDLE;
	}

	if(fst.f_bavail < (40 * 1024)) {
		zlog_err("%s : Not enough sapce in %s. Free space %u. Go to IDLE.",
			__func__, RDL_IMG_PATH, fst.f_bavail);
		return ST_RDL_IDLE;
	}
#ifdef DEBUG
	zlog_notice("%s : Free space[%u].", __func__, fst.f_bavail);
#endif

	// get temp page buffer.
#if 1 /* [#89] Fixing for RDL changes on Target system, dustin, 2024-08-02 */
	if(RDL_PAGE == NULL)
#endif
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
#if 1 /* [#110] RDL function Debugging ¿ ¿¿, balkrow, 2024-09-02 */
	clear_bank(RDL_INFO.bno);
	if(RDL_INFO.bno == RDL_BANK_1)
	{
		memset(&gDB.bank1_header, 0, sizeof(fw_image_header_t));
		write_pkg_header(RDL_BANK_1, &(gDB.bank1_header));
	} 
	else if(RDL_INFO.bno == RDL_BANK_2)
	{
		memset(&gDB.bank2_header, 0, sizeof(fw_image_header_t));
		write_pkg_header(RDL_BANK_2, &(gDB.bank2_header));
	}
#endif
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

#if 1 /* [#124] Fixing for 3rd registers update, dustin, 2024-09-09 */
	if(! zone1) {
		gettimeofday(&ktt1, NULL);
		zone1 = 1;
	}
#endif /* [#124] */

	rdl_info_list.st = state;
	return state;
}


RDL_ST_t rdl_reading_p1(void) //#3
{
    RDL_ST_t state = ST_RDL_READING_P1;
	pid_t pid;
#if 1 /* [#124] Fixing for 3rd registers update, dustin, 2024-09-09 */
	struct timeval now;
#endif

#ifdef DEBUG
	//zlog_notice("------> %s : entered.", __func__);
#endif
#if 1//PWY_FIXME
	// RDL writing P-1 Done ack / RDL reading P-1
	// MCU done writing P-1, BP copy P-1 data and ack it. No checking CRC for P1.

	// copy P-1 to buffer.
	rdl_copy_page_segment_to_buffer(RDL_PAGE_SEGMENT_1);

#if 1 /* [#124] Fixing for 3rd registers update, dustin, 2024-09-09 */
	/* read fw_image_header_t from 1st page head part. */
	if(RDL_INFO.pno == 1) {
		char temp[100];

		/* copy fw_image_header_t area. */
		memcpy(&RDL_INFO.hd, RDL_PAGE, sizeof(fw_image_header_t));
		RDL_INFO.hd.fih_magic = ntohl(RDL_INFO.hd.fih_magic);
		RDL_INFO.hd.fih_hcrc = ntohl(RDL_INFO.hd.fih_hcrc);
		RDL_INFO.hd.fih_dcrc = ntohl(RDL_INFO.hd.fih_dcrc);
		RDL_INFO.hd.fih_time = ntohl(RDL_INFO.hd.fih_time);
		RDL_INFO.hd.fih_size = ntohl(RDL_INFO.hd.fih_size);
#ifdef DEBUG
		zlog_notice("%s : Magic Number [0x%x].", __func__, RDL_INFO.hd.fih_magic);
		zlog_notice("%s : Header CRC [0x%x].", __func__, RDL_INFO.hd.fih_hcrc);
		zlog_notice("%s : Data CRC [0x%x].", __func__, RDL_INFO.hd.fih_dcrc);
		zlog_notice("%s : Build Time [0x%x].", __func__, RDL_INFO.hd.fih_time);
		zlog_notice("%s : Total Size [%u].", __func__, RDL_INFO.hd.fih_size);
		zlog_notice("%s : Version [%s].", __func__, RDL_INFO.hd.fih_ver);
		zlog_notice("%s : File Name [%s].", __func__, RDL_INFO.hd.fih_name);
#endif
		// check if target file is already present in system.
		sprintf(temp, "%s%s", RDL_IMG_PATH, RDL_INFO.hd.fih_name);
		if(syscmd_file_exist(temp)) {
			zlog_err("%s : Already present file[%s]. Removed for start.",
				__func__, temp);
			unlink(temp);
			system("sync");
		}
	}

	gettimeofday(&now, NULL);
	now.tv_sec -= ktt1.tv_sec;
	if(now.tv_sec > RDL_TIMEOUT_1) {
		zlog_notice("Timeout [%d] sec at zone1. Go to IDLE.", now.tv_sec);
    	rdl_info_list.st = ST_RDL_IDLE;
		return ST_RDL_IDLE;
	}
	zone1 = 0;
#endif /* [#124] */

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
#if 1 /* [#124] Fixing for 3rd registers update, dustin, 2024-09-09 */
	gDPRAMRegUpdate(RDL_STATE_RESP_ADDR, 8, 0xFF00, RDL_PAGE_READING_ERROR_BIT);
#else
	gDPRAMRegUpdate(RDL_STATE_RESP_ADDR, 8, 0xFF00, RDL_PAGE_WRITING_ERROR_ACK_BIT);
#endif /* [#124] */
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
#if 1 /* [#89] Fixing for RDL changes on Target system, dustin, 2024-08-02 */
	uint32_t left;
#endif

#if 1 /* [#124] Fixing for 3rd registers update, dustin, 2024-09-09 */
	/* keep current state for waiting page write done (0x6) from mcu. */
    RDL_ST_t state = ST_RDL_WRITING_P2;
#else
    RDL_ST_t state = ST_RDL_READING_P2;
#endif
#ifdef DEBUG
	//zlog_notice("------> %s : entered.", __func__);
#endif
	// RDL writing P-2 Done ack / RDL reading P-2
	// MCU done writing P-2, BP copy data to buffer, check page CRC and ack it.
	//     and BP fork a process to write page data to img file.

	// ack for p2 writing done.
	gDPRAMRegUpdate(RDL_STATE_RESP_ADDR, 8, 0xFF00, RDL_P2_WRITING_DONE_ACK_BIT);
#if 1 /* [#124] Fixing for 3rd registers update, dustin, 2024-09-09 */
	gettimeofday(&ktt3, NULL);
	zone3 = 1;

	rdl_info_list.st = state;
	return state;
#else /**************************************************************/
#if 1 /* [#89] Fixing for RDL changes on Target system, dustin, 2024-08-02 */
	// get current page length.
	left = RDL_INFO.hd.total_size % RDL_PAGE_SIZE;
	if((RDL_INFO.pno < RDL_INFO.total_pno) || 
	   (left >= RDL_PAGE_SEGMENT_SIZE)) {
		// copy P-2 to buffer.
		rdl_copy_page_segment_to_buffer(RDL_PAGE_SEGMENT_2);
	}
#else
	// copy P-2 to buffer.
	rdl_copy_page_segment_to_buffer(RDL_PAGE_SEGMENT_2);
#endif

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
#endif /* [#124] */
}

#if 1 /* [#124] Fixing for 3rd registers update, dustin, 2024-09-09 */
RDL_ST_t rdl_check_page_done(void)
{
	uint32_t total_size, page_size;
	uint32_t left;
	RDL_ST_t state = ST_RDL_READING_P2;
#ifdef DEBUG
	//zlog_notice("------> %s : entered.", __func__);
#endif
	// get current page length.
	left = (RDL_INFO.hd.fih_size + sizeof(fw_image_header_t)) % RDL_PAGE_SIZE;
	if((RDL_INFO.pno < RDL_INFO.total_pno) || 
	   (left >= RDL_PAGE_SEGMENT_SIZE)) {
		// copy P-2 to buffer.
		rdl_copy_page_segment_to_buffer(RDL_PAGE_SEGMENT_2);
	}

	// check page data.
	PAGE_CRC_OK = rdl_check_page_crc();
	if(1/*PAGE_CRC_OK*/) {
		// get total size.
		total_size = RDL_INFO.hd.fih_size + sizeof(fw_image_header_t);

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
		// save page data to img file.
		rdl_save_page_to_img_file(page_size);

		rdl_info_list.st = state;
		return state;
	} else {
		// nack for page reading.
		gDPRAMRegUpdate(RDL_STATE_RESP_ADDR, 8, 0xFF00, RDL_TOTAL_FW_CRC_ERROR_BIT);

		//FIXME : necessary to clear page buffer?

		rdl_info_list.st = state;
		return state;
	}

	rdl_info_list.st = state;
	return state;
}
#endif /* [#124] */

RDL_ST_t rdl_writing_err_p2(void) //#8
{
    RDL_ST_t state = ST_RDL_START;
#ifdef DEBUG
	//zlog_notice("------> %s : entered.", __func__);
#endif
	// RDL page writing error ack
	// MCU go to rdl start state
#if 1 /* [#124] Fixing for 3rd registers update, dustin, 2024-09-09 */
	gDPRAMRegUpdate(RDL_STATE_RESP_ADDR, 8, 0xFF00, RDL_PAGE_READING_ERROR_BIT);
#else
	gDPRAMRegUpdate(RDL_STATE_RESP_ADDR, 8, 0xFF00, RDL_PAGE_WRITING_ERROR_ACK_BIT);
#endif /* [#124] */
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
#if 1 /* [#124] Fixing for 3rd registers update, dustin, 2024-09-09 */
	{
		struct timeval now;
		gettimeofday(&now, NULL);
		now.tv_sec -= ktt3.tv_sec;
		if(now.tv_sec > RDL_TIMEOUT_3) {
			zlog_notice("Timeout [%d] sec at zone3. Go to IDLE.", now.tv_sec);
			rdl_info_list.st = ST_RDL_IDLE;
			return ST_RDL_IDLE;
		}
		zone3 = 0;
	}
#endif /* [#124] */
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
#if 1 /* [#124] Fixing for 3rd registers update, dustin, 2024-09-09 */
	gettimeofday(&ktt4, NULL);
	zone4 = 1;

	file_size = rdl_get_file_size(RDL_INFO.hd.fih_name);
	if(file_size != (RDL_INFO.hd.fih_size + sizeof(fw_image_header_t))) {
		zlog_notice("%s : Different file size [%u vs %u]. Go to IDLE.",
			__func__, file_size, RDL_INFO.hd.fih_size + 
			sizeof(fw_image_header_t));

		gDPRAMRegUpdate(RDL_STATE_RESP_ADDR, 8, 0xFF00, RDL_TOTAL_FW_CRC_ERROR_BIT);
		return ST_RDL_IDLE;
	}
#else
	file_size = rdl_get_file_size(RDL_INFO.hd.file_name);
	if(file_size != RDL_INFO.hd.total_size) {
		zlog_notice("%s : Different file size [%u vs %u]. Go to IDLE.",
			__func__, file_size, RDL_INFO.hd.total_size);

		gDPRAMRegUpdate(RDL_STATE_RESP_ADDR, 8, 0xFF00, RDL_TOTAL_READING_ERROR_BIT);
		return ST_RDL_IDLE;
	}

	gDPRAMRegUpdate(RDL_STATE_RESP_ADDR, 8, 0xFF00, RDL_TOTAL_READING_DONE_BIT);
#endif
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

#if 1/*[#110] RDL function Debugging 및 수정, balkrow, 2024-09-03*/ 
void move_image_to_bank(void) 
{
	struct timeval sss;
	int bno = RDL_INFO.bno;
	gettimeofday(&sss, NULL);
	zlog_notice("%s : ----> Activation start time[%ld sec].", __func__, sss.tv_sec);

#if 1 /* [#124] Fixing for 3rd registers update, dustin, 2024-09-09 */
	if(rdl_decompress_package_file(RDL_INFO.hd.fih_name) < 0) {
		//FIXME : success or failed, pkg file will be removed anyway.
		zlog_err("Decompressing pkg file %s has failed. Go to IDLE.",
			RDL_INFO.hd.fih_name);
		rdl_info_list.st = ST_RDL_IDLE;
		return;
	}
#else
	if(rdl_decompress_package_file(RDL_INFO.hd.file_name) < 0) {
		//FIXME : success or failed, pkg file will be removed anyway.
		zlog_err("Decompressing pkg file %s has failed. Go to IDLE.",
			RDL_INFO.hd.file_name);
		rdl_info_list.st = ST_RDL_IDLE;
		return;
	}
#endif

	if(gDB.pkg_is_zip == PKG_ZIP)
	{
		char fpga_img_file_path[128] = {0, };
		char *bank_path;

		if(RDL_INFO.bno == RDL_BANK_1)
			bank_path = RDL_INSTALL1_PATH;
		else if(RDL_INFO.bno == RDL_BANK_2)
			bank_path = RDL_INSTALL2_PATH;

		if(get_fpga_filename(FPGA_IMG_PREFIX, bank_path, fpga_img_file_path) == RT_NOK)
		{
			zlog_err("fpga image %s not found. Go to IDLE.",
				 fpga_img_file_path);
			rdl_info_list.st = ST_RDL_IDLE;
			return;
		}

		if(install_fpga_image(RDL_INFO.bno , fpga_img_file_path) == -1)
		{
			zlog_err("fpga image %s update failed. Go to IDLE.",
				 fpga_img_file_path);
			rdl_info_list.st = ST_RDL_IDLE;
			return;
		}
	}
#if 1 /* [#124] Fixing for 3rd registers update, dustin, 2024-09-09 */
	{
		struct timeval now;
		/* notify rdl done to mcu. */
		gDPRAMRegUpdate(RDL_STATE_RESP_ADDR, 8, 0xFF00, RDL_IMG_INSTALL_DONE_BIT);

		gettimeofday(&now, NULL);
		now.tv_sec -= ktt4.tv_sec;
		if(now.tv_sec > RDL_TIMEOUT_4) {
			zlog_notice("Timeout [%d] sec at zone4. Go to IDLE.", now.tv_sec);
			rdl_info_list.st = ST_RDL_IDLE;
			return;
		}
		zone4 = 0;
	}
#endif /* [#124] */

	RDL_INSTALL_STATE = 1;

	gettimeofday(&sss, NULL);
	zlog_notice("%s : ----> RDL end time[%ld sec].", __func__, sss.tv_sec);
	rdl_info_list.st = ST_RDL_INSTALL_DONE;
}

RDL_ST_t rdl_img_move(void) //#13
{
    RDL_ST_t state = ST_RDL_BANK_MOVE_WAIT;
    pthread_create(&thread_t, NULL, move_image_to_bank, NULL);
    rdl_info_list.st = state;

}

RDL_ST_t wait_rdl_move(void) //#13
{
    RDL_ST_t state = ST_RDL_BANK_MOVE_WAIT;
    rdl_info_list.st = state;

}
#endif

#if 1 /* [#105] Fixing for RDL install/activation process, dustin, 2024-08-27 */
RDL_ST_t rdl_img_install(void) //#13
#else
RDL_ST_t rdl_img_activation(void) //#13
#endif
{
#if 1 /* [#105] Fixing for RDL install/activation process, dustin, 2024-08-27 */
    RDL_ST_t state = ST_RDL_INSTALL_DONE;
#else
    RDL_ST_t state = ST_RDL_ACTIVATE_DONE;
#endif
#if 1 /* [#89] Fixing for RDL changes on Target system, dustin, 2024-08-02 */
	struct timeval sss;
#endif
	struct statfs fst;
	int bno = RDL_INFO.bno;
#ifdef DEBUG
	//zlog_notice("------> %s : entered.", __func__);
#endif
#if 1 /* [#89] Fixing for RDL changes on Target system, dustin, 2024-08-02 */
	gettimeofday(&sss, NULL);
    zlog_notice("%s : ----> Installing start time[%ld sec].", __func__, sss.tv_sec);
#endif
	// BP check if disk space is enough for processing.
	// check free space for rdl. Go idle state if not available.
#if 0/*[#110] RDL function Debugging 및 수정, balkrow, 2024-09-02*/ 
	if(statfs(RDL_IMG_PATH, &fst) != 0) {
		zlog_err("%s : Cannot stat for %s. reason[%s]. Go to IDLE.",
			__func__, RDL_IMG_PATH, strerror(errno));
		return ST_RDL_IDLE;
	}
#endif

	// BP extract FPGA F/W img and BP os img files, and remove integrated img file.
#if 1 /* [#124] Fixing for 3rd registers update, dustin, 2024-09-09 */
	if(rdl_decompress_package_file(RDL_INFO.hd.fih_name) < 0) {
		zlog_err("Decompressing pkg file %s has failed. Go to IDLE.",
			RDL_INFO.hd.fih_name);
		rdl_info_list.st = state;
		return ST_RDL_IDLE;
	}
#else /**************************************************************/
	if(rdl_decompress_package_file(RDL_INFO.hd.file_name) < 0) {
		//FIXME : success or failed, pkg file will be removed anyway.
#if 1/*[#110] RDL function Debugging 및 수정, balkrow, 2024-09-02*/ 
		zlog_err("Decompressing pkg file %s has failed. Go to IDLE.",
			RDL_INFO.hd.file_name);
		rdl_info_list.st = state;
#else	
		zlog_err("%s : Decompressing pkg file %s has failed. Go to IDLE.",
			__func__, RDL_INFO.hd.file_name);
#endif
		return ST_RDL_IDLE;
	}
#endif /* [#124] */

#if 1 /* [#105] Fixing for RDL install/activation process, dustin, 2024-08-27 */
	// BP install(replace bank files) BP os img to target bank.
#if 1/*[#110] RDL function Debugging 및 수정, balkrow, 2024-09-02*/ 
	RDL_INSTALL_STATE = 1;
#else
	RDL_INSTALL_STATE = rdl_install_package(bno);
#endif

	gettimeofday(&sss, NULL);
	zlog_notice("%s : ----> RDL end time[%ld sec].", __func__, sss.tv_sec);

#if 0 /* [#105] Fixing for RDL install/activation process, dustin, 2024-08-27 */
	// BP update OS related registers.
	rdl_update_bank_registers(bno, 
		(RDL_INSTALL_STATE >= 0) ? 0/*update*/ : 1/*erase*/);
#endif
#else /***********************************************************/
	// BP install(replace bank files) BP os img to target bank.
	rdl_install_package(bno);

	// BP copy os img to /flash/boot for activation.
#if 1 /* [#89] Fixing for RDL changes on Target system, dustin, 2024-08-02 */
#if 1 /* [#105] Fixing for RDL install/activation process, dustin, 2024-08-27 */
	RDL_INSTALL_STATE = rdl_install_bp(bno);
#else
	RDL_ACTIVATION_STATE = rdl_activate_bp(bno);
#endif
#else
	rdl_activate_bp(bno);
#endif
	
	// BP update OS related registers.
#if 1 /* [#89] Fixing for RDL changes on Target system, dustin, 2024-08-02 */
	if(RDL_ACTIVATION_STATE >= 0)
		rdl_update_bank_registers(bno);
#else
	rdl_update_bank_registers();
#endif
#if 1 /* [#89] Fixing for RDL changes on Target system, dustin, 2024-08-02 */
	gettimeofday(&sss, NULL);
    zlog_notice("%s : ----> BP OS end time[%ld sec].", __func__, sss.tv_sec);
#endif

	// BP check version and download to FPGA F/W ?
	// FIXME : do it~!!
#if 1 /* [#89] Fixing for RDL changes on Target system, dustin, 2024-08-02 */
	RDL_ACTIVATION_STATE = rdl_activate_fpga(bno);

	gettimeofday(&sss, NULL);
    zlog_notice("%s : ----> FPGA FW end time[%ld sec].", __func__, sss.tv_sec);
#else
	rdl_activate_fpga(bno);
#endif
#if 1 /* [#93] Adding for FPGA FW Bank Select and Error handling, dustin, 2024-08-12 */
	if(RDL_ACTIVATION_STATE >= 0) {
		/* request reboot to specified bank */
		CPLD_WRITE(CPLD_FW_BANK_SELECT_ADDR, bno);
#if 1 /* [#96] Adding option bit after downloading FPGA, dustin, 2024-08-19 */
		/* need some time to display syslog. */
		usleep(500000);
#endif
		/* reset bp too */
		system("reboot -nf");
	}
#endif
#endif /* [#105] */

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

#if 1 /* [#89] Fixing for RDL changes on Target system, dustin, 2024-08-02 */
	/* reset rdl state response register. */
	gDPRAMRegUpdate(RDL_STATE_RESP_ADDR, 0, 0xFFFF, 0);
#endif

    rdl_info_list.st = state;
    return state;
}

RDL_ST_t rdl_sw_reset(void) //#15
{
	RDL_ST_t state = ST_RDL_RUNNING_CHECK;
#ifdef DEBUG
	//zlog_notice("------> %s : entered.", __func__);
#endif
#if 1 /* [#89] Fixing for RDL changes on Target system, dustin, 2024-08-02 */
	/* reset rdl state response register. */
	gDPRAMRegUpdate(RDL_STATE_RESP_ADDR, 0, 0xFFFF, 0);
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

#if 1 /* [#89] Fixing for RDL changes on Target system, dustin, 2024-08-02 */
	/* reset rdl state response register. */
	gDPRAMRegUpdate(RDL_STATE_RESP_ADDR, 0, 0xFFFF, 0);
#endif

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
#if 1 /* [#124] Fixing for 3rd registers update, dustin, 2024-09-09 */
	{
		static int keep_time;

		/* keep last state for 5 interval times to let mcu be notified. */
		if(keep_time++ < RDL_MAX_FAIL_TRY) {
			return (rdl_info_list.st);
		}
	}
#endif

#if 1 /* [#89] Fixing for RDL changes on Target system, dustin, 2024-08-02 */
	/* reset rdl state response register. */
	gDPRAMRegUpdate(RDL_STATE_RESP_ADDR, 0, 0xFFFF, 0);
#endif

    rdl_info_list.st = state;
    return state;
}

char *RDL_STATE_STR[] = {
	"ST_RDL_IDLE",
#if 1/* [#77] Adding RDL emulation function, dustin, 2024-07-16 */
    "ST_RDL_TRIGGER",
#endif
	"ST_RDL_START",
	"ST_RDL_WRITING_P1",
	"ST_RDL_READING_P1",
	"ST_RDL_WRITING_P2",
	"ST_RDL_READING_P2",
	"ST_RDL_WRITING_TOTAL",
	"ST_RDL_READING_TOTAL",
#if 1 /* [#105] Fixing for RDL install/activation process, dustin, 2024-08-27 */
	"ST_RDL_INSTALL_DONE",
#else
	"ST_RDL_ACTIVATE_DONE",
#endif
	"ST_RDL_RUNNING_CHECK",
	"ST_RDL_TERM",
	"ST_RDL_ST_MAX",
};

char *RDL_EVENT_STR[] = {
	"EVT_RDL_NONE",
#if 1/* [#77] Adding RDL emulation function, dustin, 2024-07-16 */
    "EVT_RDL_TRIGGER",
#endif
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
#if 1 /* [#105] Fixing for RDL install/activation process, dustin, 2024-08-27 */
	"EVT_RDL_IMG_INSTALL_SUCCESS",
	"EVT_RDL_IMG_INSTALL_FAIL",
#else
	"EVT_RDL_IMG_ACTIVE_SUCCESS",
	"EVT_RDL_IMG_ACTIVE_FAIL",
#endif
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
#if 1 /* [#105] Fixing for RDL install/activation process, dustin, 2024-08-27 */
	{ST_RDL_WRITING_P1,         EVT_RDL_START,	                rdl_start},
#endif

	{ST_RDL_READING_P1,         EVT_RDL_WRITING_P2,             rdl_writing_p2},
#if 1 /* [#105] Fixing for RDL install/activation process, dustin, 2024-08-27 */
	{ST_RDL_READING_P1,         EVT_RDL_START,	                rdl_start},
#endif

	{ST_RDL_WRITING_P2,         EVT_RDL_WRITING_DONE_P2,        rdl_reading_p2},
#if 1 /* [#124] Fixing for 3rd registers update, dustin, 2024-09-09 */
	{ST_RDL_WRITING_P2,         EVT_RDL_PAGE_WRITING_DONE,      rdl_check_page_done},
#endif
	{ST_RDL_WRITING_P2,         EVT_RDL_WRITING_ERROR,		    rdl_writing_err_p2},
#if 1 /* [#105] Fixing for RDL install/activation process, dustin, 2024-08-27 */
	{ST_RDL_WRITING_P2,         EVT_RDL_START,	                rdl_start},
#endif

	{ST_RDL_READING_P2,         EVT_RDL_READING_DONE_P2,        rdl_page_done},
	{ST_RDL_READING_P2,         EVT_RDL_READING_ERROR,	        rdl_reading_err_p2},
#if 1 /* [#105] Fixing for RDL install/activation process, dustin, 2024-08-27 */
	{ST_RDL_READING_P2,         EVT_RDL_START,	                rdl_start},
#endif

	{ST_RDL_WRITING_TOTAL,      EVT_RDL_WRITING_DONE_TOTAL,     rdl_reading_total},
	{ST_RDL_WRITING_TOTAL,      EVT_RDL_WRITING_NOT_DONE,       rdl_next_page},
	{ST_RDL_WRITING_TOTAL,      EVT_RDL_WRITING_ERROR_TOTAL,    rdl_restart},
#if 1 /* [#105] Fixing for RDL install/activation process, dustin, 2024-08-27 */
	{ST_RDL_WRITING_TOTAL,      EVT_RDL_START,	                rdl_start},
#endif

#if 1 /* [#110] RDL function Debugging ¿ ¿¿, balkrow, 2024-09-02 */
	{ST_RDL_READING_TOTAL,      EVT_RDL_READING_DONE_TOTAL,     rdl_img_move},
#else
	{ST_RDL_READING_TOTAL,      EVT_RDL_READING_DONE_TOTAL,     rdl_img_install},
#endif
	{ST_RDL_READING_TOTAL,      EVT_RDL_READING_ERROR_TOTAL,    rdl_restart},
#if 1 /* [#105] Fixing for RDL install/activation process, dustin, 2024-08-27 */
	{ST_RDL_READING_TOTAL,      EVT_RDL_START,	                rdl_start},
#endif

#if 1 /* [#105] Fixing for RDL install/activation process, dustin, 2024-08-27 */
	{ST_RDL_INSTALL_DONE,       EVT_RDL_IMG_INSTALL_SUCCESS,    rdl_terminate},
	{ST_RDL_INSTALL_DONE,       EVT_RDL_IMG_INSTALL_FAIL, 	    rdl_terminate},
#else
	{ST_RDL_ACTIVATE_DONE,      EVT_RDL_IMG_ACTIVE_SUCCESS,     rdl_sw_reset},
	{ST_RDL_ACTIVATE_DONE,      EVT_RDL_IMG_ACTIVE_FAIL, 	    rdl_terminate},

	{ST_RDL_RUNNING_CHECK,      EVT_RDL_IMG_RUNNING_SUCCESS, 	rdl_running_success},
	{ST_RDL_RUNNING_CHECK,      EVT_RDL_IMG_RUNNING_FAIL, 	    rdl_running_fail},
#endif
#if 1 /* [#105] Fixing for RDL install/activation process, dustin, 2024-08-27 */
	{ST_RDL_BANK_MOVE_WAIT,     EVT_RDL_WAIT_MOVE_IMG_TO_BANK, wait_rdl_move},
#endif

	{ST_RDL_TERM,               EVT_RDL_NONE, 	                rdl_terminate},
};

int RDL_TRANS_MAX = sizeof(rdl_fsm_list) / sizeof(RDL_FSM_t);


#if 1 /* [#124] Fixing for 3rd registers update, dustin, 2024-09-09 */
	/* NOTE : no more use below emul functions. */
#else /**************************************************************/
#if 1/* [#77] Adding RDL emulation function, dustin, 2024-07-16 */
extern RDL_IMG_INFO_t EMUL_INFO;
extern uint8_t __CACHE_RDL_PAGE[RDL_PAGE_ADDR_SIZE];

FILE *EMUL_FP = NULL;
FILE *EMUL_OS_FP = NULL;
long EMUL_FP_POS;
uint32_t EMUL_LEFT_LEN;
int EMUL_TRIGGERED;
int EMUL_FILE_END, EMUL_RDL_DONE;
char EMUL_FPATH[200];

RDL_ST_t emul_trigger(void)
{
    RDL_ST_t state = ST_RDL_TRIGGER;
	fw_image_header_t hd;
	struct statfs fst;
	uint32_t addr, t_crc, t_size, ii;
	char temp[150];
#ifdef DEBUG
	//zlog_notice("------> %s : entered.", __func__);
#endif
	// MCU triggered emulation.
	//    init something(pkg file present, size/page info, and update rdl registers.
	//    finally set rdl start bit.

	memset(&EMUL_INFO, 0, sizeof EMUL_INFO);
	EMUL_INFO.bno = RDL_BANK_2;

	// check if pkg file present.
	if(! syscmd_file_exist(EMUL_FPATH)) {
		zlog_notice("%s : No such file : %s\n", __func__, EMUL_FPATH);
		emul_info_list.st = ST_RDL_IDLE;
		return ST_RDL_IDLE;
	}

	// get pkg file size.
	if(check_img_file_crc(EMUL_FPATH, &hd, &t_size, &t_crc) < 0) {
		zlog_notice("%s : Checking crc failed for pkg file %s.",
			__func__, EMUL_FPATH);
		emul_info_list.st = ST_RDL_IDLE;
		return ST_RDL_IDLE;
	}

	t_crc = ntohl(hd.fih_dcrc);

	EMUL_INFO.hd.magic      = RDL_IMG_MAGIC;
	EMUL_INFO.hd.total_crc  = t_crc;
	EMUL_INFO.hd.total_size = t_size;
	memcpy(EMUL_INFO.hd.ver_str, hd.fih_ver, RDL_VER_STR_MAX);
	memcpy(EMUL_INFO.hd.file_name, hd.fih_name, RDL_FILE_NAME_MAX);

	// get total page no.
	EMUL_INFO.total_pno = t_size / RDL_PAGE_SIZE;
	EMUL_LEFT_LEN  = t_size % RDL_PAGE_SIZE;
	if(EMUL_LEFT_LEN)
		EMUL_INFO.total_pno += 1;

	// update target bank no and total page no.
	gDPRAMRegUpdate(RDL_TARGET_BANK_ADDR, 0, 0xFFFF, 
		(EMUL_INFO.bno << 8) | EMUL_INFO.total_pno);

	// set magic number.
	gDPRAMRegUpdate(RDL_MAGIC_NO_1_ADDR, 0, 0xFFFF, (hd.fih_magic & 0xFFFF));
	gDPRAMRegUpdate(RDL_MAGIC_NO_2_ADDR, 0, 0xFFFF, ((hd.fih_magic >> 16) & 0xFFFF));

	// set total crc.
	gDPRAMRegUpdate(RDL_TOTAL_CRC_1_ADDR, 0, 0xFFFF, (t_crc & 0xFFFF));
	gDPRAMRegUpdate(RDL_TOTAL_CRC_2_ADDR, 0, 0xFFFF, ((t_crc >> 16) & 0xFFFF));

	// set total size.
	gDPRAMRegUpdate(RDL_TOTAL_SIZE_1_ADDR, 0, 0xFFFF, (t_size & 0xFFFF));
	gDPRAMRegUpdate(RDL_TOTAL_SIZE_2_ADDR, 0, 0xFFFF, ((t_size >> 16) & 0xFFFF));

	// set build time.
	gDPRAMRegUpdate(RDL_BUILD_TIME_1_ADDR, 0, 0xFFFF, (hd.fih_time & 0xFFFF));
	gDPRAMRegUpdate(RDL_BUILD_TIME_2_ADDR, 0, 0xFFFF, ((hd.fih_time >> 16) & 0xFFFF));

	// set version string.
	for(ii = 0, addr = RDL_VER_STR_START_ADDR;
	   (ii < strlen(hd.fih_ver)) && (addr < RDL_VER_STR_END_ADDR);
	    ii += 2, addr += 2) {
		gDPRAMRegUpdate(addr, 0, 0xFFFF, hd.fih_ver[ii] | (hd.fih_ver[ii + 1] << 8));
	}

	// set file name string.
	for(ii = 0, addr = RDL_FILE_NAME_START_ADDR;
	   (ii < strlen(hd.fih_name)) && (addr < RDL_FILE_NAME_END_ADDR);
	    ii += 2, addr += 2) {
		gDPRAMRegUpdate(addr, 0, 0xFFFF, hd.fih_name[ii] | (hd.fih_name[ii + 1] << 8));
	}

	EMUL_INFO.pno = 1;
	EMUL_FP_POS = 0;

	// MCU set rdl start.
	gDPRAMRegUpdate(RDL_STATE_REQ_ADDR, 0, 0xFFFF, 
		(RDL_START_BIT << 8) | EMUL_INFO.pno);
    emul_info_list.st = state;
    return state;
}

RDL_ST_t emul_start(void)
{
    RDL_ST_t state = ST_RDL_START;
#ifdef DEBUG
	//zlog_notice("------> %s : entered.", __func__);
#endif
	// BP sent rdl start ack.

	// MCU close src file to open again, if it is open already.
	if(EMUL_FP != NULL)
		fclose(EMUL_FP);

	// MCU open src file.
	EMUL_FP = fopen(EMUL_FPATH, "r");
	if(EMUL_FP == NULL) {
		zlog_notice("%s : Cannot open file %s.", __func__, EMUL_FPATH);
		emul_info_list.st = ST_RDL_IDLE;
		return ST_RDL_IDLE;
	}

	// MCU just set writing p-1.
	gDPRAMRegUpdate(RDL_STATE_REQ_ADDR, 8, 0xFF00, RDL_P1_WRITING_BIT);
    emul_info_list.st = state;
    return state;
}

RDL_ST_t emul_writing_p1(void)
{
	RDL_ST_t state = ST_RDL_WRITING_P1;
#ifdef DEBUG
	//zlog_notice("------> %s : entered.", __func__);
#endif
	// BP sent writing p-1 ack.
	// MCU read/write to p-1 and set writing p-1 done.

	// backup current position for p-1, for restarting case.
	EMUL_FP_POS = ftell(EMUL_FP);

	// MCU read/write to p-1.
	if(fread(&(__CACHE_RDL_PAGE[0]), 1, RDL_PAGE_SEGMENT_SIZE, EMUL_FP) != 
		RDL_PAGE_SEGMENT_SIZE) {
		if(EMUL_INFO.pno != EMUL_INFO.total_pno) {
			zlog_notice("%s : Reading to P-1 failed. pno[%u/%u].",
				__func__, EMUL_INFO.pno, EMUL_INFO.total_pno);
			// MCU set page writing error.
#if 1 /* [#124] Fixing for 3rd registers update, dustin, 2024-09-09 */
			gDPRAMRegUpdate(RDL_STATE_REQ_ADDR, 8, 0xFF00, 
				RDL_TOTAL_WRITING_ERROR_BIT);
#else
			gDPRAMRegUpdate(RDL_STATE_REQ_ADDR, 8, 0xFF00, 
				RDL_PAGE_WRITING_ERROR_BIT);
#endif /* [#124] */
			emul_info_list.st = ST_RDL_TRIGGER;
		}
		EMUL_FILE_END = 1;
	}

	// MCU set writing p-1 done.
	gDPRAMRegUpdate(RDL_STATE_REQ_ADDR, 8, 0xFF00, RDL_P1_WRITING_DONE_BIT);
	emul_info_list.st = state;
	return state;
}

RDL_ST_t emul_writing_err_p1(void)
{
    RDL_ST_t state = ST_RDL_START;
#ifdef DEBUG
    //zlog_notice("------> %s : entered.", __func__);
#endif
	// BP sent page writing error ack.
    // MCU goto writing p-1 with current page.
	//    rewind file position to backup position.
	zlog_notice("%s : Rewinding file position to offset[%u] for page %u p-1.",
		__func__, EMUL_FP_POS, EMUL_INFO.pno);
	fseek(EMUL_FP, EMUL_FP_POS, SEEK_SET);

	// MCU set writing p-1.
	gDPRAMRegUpdate(RDL_STATE_REQ_ADDR, 8, 0xFF00, RDL_P1_WRITING_BIT);
    emul_info_list.st = state;
    return state;
}

RDL_ST_t emul_reading_p1(void)
{
	RDL_ST_t state = ST_RDL_READING_P1;
#ifdef DEBUG
	//zlog_notice("------> %s : entered.", __func__);
#endif

	// MCU set writing p-2.
	gDPRAMRegUpdate(RDL_STATE_REQ_ADDR, 8, 0xFF00, RDL_P2_WRITING_BIT);
	emul_info_list.st = state;
	return state;
}

RDL_ST_t emul_writing_p2(void)
{
	RDL_ST_t state = ST_RDL_WRITING_P2;
	uint16_t calc;
#ifdef DEBUG
	//zlog_notice("------> %s : entered.", __func__);
#endif

	// BP sent writing p-1 ack.
	// MCU read/write to p-1 and set writing p-1 done.
	// NOTE p-2 stage don't need to backup current position.

	// skip if file end in writing p-1.
	if(! EMUL_FILE_END) {
		// MCU read/write to p-2.
		if(fread(&(__CACHE_RDL_PAGE[RDL_PAGE_SEGMENT_SIZE]), 
			1, RDL_PAGE_SEGMENT_SIZE, EMUL_FP) != RDL_PAGE_SEGMENT_SIZE) {
			if(EMUL_INFO.pno != EMUL_INFO.total_pno) {
				zlog_notice("%s : Reading to p-2 failed. pno[%u/%u].",
					__func__, EMUL_INFO.pno, EMUL_INFO.total_pno);
				// MCU set page writing error.
#if 1 /* [#124] Fixing for 3rd registers update, dustin, 2024-09-09 */
				gDPRAMRegUpdate(RDL_STATE_REQ_ADDR, 8, 0xFF00,
					RDL_TOTAL_WRITING_ERROR_BIT);
#else
				gDPRAMRegUpdate(RDL_STATE_REQ_ADDR, 8, 0xFF00,
					RDL_PAGE_WRITING_ERROR_BIT);
#endif /* [#124] */
				emul_info_list.st = ST_RDL_TRIGGER;
			}
			EMUL_FILE_END = 1;
		}
	}

	// calculate crc for the page and update page crc register.
	calc = get_sum((uint16_t *)__CACHE_RDL_PAGE,
		(EMUL_INFO.pno < EMUL_INFO.total_pno) ? 
			RDL_PAGE_SIZE: 
				(EMUL_LEFT_LEN ? EMUL_LEFT_LEN : RDL_PAGE_SIZE));
	gDPRAMRegUpdate(RDL_PAGE_CRC_ADDR, 0, 0xFFFF, calc);	

	// MCU set writing p-2 done.
	gDPRAMRegUpdate(RDL_STATE_REQ_ADDR, 8, 0xFF00, RDL_P2_WRITING_DONE_BIT);
	emul_info_list.st = state;
	return state;
}

RDL_ST_t emul_reading_p2(void)
{
	RDL_ST_t state = ST_RDL_READING_P2;
#ifdef DEBUG
	//zlog_notice("------> %s : entered.", __func__);
#endif
	// BP will send page reading done.
	// MCU do nothing.
	emul_info_list.st = state;
	return state;
}

RDL_ST_t emul_writing_err_p2(void)
{
	RDL_ST_t state = ST_RDL_START;
#ifdef DEBUG
	//zlog_notice("------> %s : entered.", __func__);
#endif
	// BP sent page writing error ack.
	// MCU goto writing p-1 with current page.
	//    rewind file position to backup position.
	zlog_notice("%s : Rewinding file position to offset[%u] for page %u p-1.",
		__func__, EMUL_FP_POS, EMUL_INFO.pno);
	fseek(EMUL_FP, EMUL_FP_POS, SEEK_SET);

	// MCU set writing p-1.
	gDPRAMRegUpdate(RDL_STATE_REQ_ADDR, 8, 0xFF00, RDL_P1_WRITING_BIT);
	emul_info_list.st = state;
	return state;
}

RDL_ST_t emul_page_done(void)
{
	RDL_ST_t state;
#ifdef DEBUG
	//zlog_notice("------> %s : entered.", __func__);
#endif
	// BP sent page reading done.
	// MCU check all pages done.
	//    If so, goto writing total done
	if(EMUL_INFO.pno == EMUL_INFO.total_pno) {
		// total page done.  MCU send total writing done.
		gDPRAMRegUpdate(RDL_STATE_REQ_ADDR, 8, 0xFF00, RDL_TOTAL_WRITING_DONE_BIT);
		state = ST_RDL_WRITING_TOTAL;
	} else {
		// MCU goto next page.
		EMUL_INFO.pno += 1;
		gDPRAMRegUpdate(RDL_STATE_REQ_ADDR, 0, 0xFFFF, 
			(RDL_P1_WRITING_BIT << 8) | EMUL_INFO.pno);
		state = ST_RDL_START;
#ifdef DEBUG
		if(!(EMUL_INFO.pno % 20))
			zlog_notice("%s : rdl page [%u/%u].", 
				__func__, EMUL_INFO.pno, EMUL_INFO.total_pno);
#endif
	}

	emul_info_list.st = state;
	return state;
}

RDL_ST_t emul_reading_err_p2(void)
{
	RDL_ST_t state = ST_RDL_START;
#ifdef DEBUG
	//zlog_notice("------> %s : entered.", __func__);
#endif
	// BP sent page reading error.
	// MCU goto writing p-1 with current page.
	//    rewind file position to backup position.
	zlog_notice("%s : Rewinding file position to offset[%u] for page %u p-1.",
			__func__, EMUL_FP_POS, EMUL_INFO.pno);
	fseek(EMUL_FP, EMUL_FP_POS, SEEK_SET);

	// MCU set writing p-1.
	gDPRAMRegUpdate(RDL_STATE_REQ_ADDR, 8, 0xFF00, RDL_P1_WRITING_BIT);
	emul_info_list.st = state;
	return state;
}

RDL_ST_t emul_writing_total(void)
{
	RDL_ST_t state = ST_RDL_TERM;
#ifdef DEBUG
	//zlog_notice("------> %s : entered.", __func__);
#endif
	// BP sent total reading done.
	// MCU finish rdl process.
	zlog_notice("%s : Finishing rdl process.", __func__);
	if(EMUL_FP != NULL) {
		fclose(EMUL_FP);
		EMUL_FP = NULL;
	}

#if 1//PWY_FIXME
	// remove rdl src file.
	if(syscmd_file_exist(EMUL_FPATH)) {
		unlink(EMUL_FPATH);
		system("sync");
	}
#endif //PWY_FIXME

	EMUL_RDL_DONE = 1;
	emul_info_list.st = state;
	return state;
}

RDL_ST_t emul_restart(void)
{
	RDL_ST_t state = ST_RDL_TRIGGER;
#ifdef DEBUG
	//zlog_notice("------> %s : entered.", __func__);
#endif
	// reset pno.
	EMUL_INFO.pno = 1;

	// close file.
	if(EMUL_FP != NULL) {
		fclose(EMUL_FP);
		EMUL_FP = NULL;
		EMUL_FP_POS = 0;
	}

	// MCU set rdl start.
	gDPRAMRegUpdate(RDL_STATE_REQ_ADDR, 0, 0xFFFF, 
		(RDL_START_BIT << 8) | EMUL_INFO.pno);

	emul_info_list.st = state;
	return state;
}

RDL_ST_t emul_terminate(void)
{
	RDL_ST_t state = ST_RDL_TERM;
#ifdef DEBUG
	//zlog_notice("------> %s : entered.", __func__);
#endif
	if(EMUL_FP != NULL) {
		fclose(EMUL_FP);
		EMUL_FP = NULL;
	}
	emul_info_list.st = state;
	return state;
}

RDL_FSM_t emul_fsm_list[] =
{
    {ST_RDL_IDLE,               EVT_RDL_TRIGGER,                emul_trigger},

    {ST_RDL_TRIGGER,            EVT_RDL_START,                  emul_start},

    {ST_RDL_START,              EVT_RDL_WRITING_P1,             emul_writing_p1},

    {ST_RDL_WRITING_P1,         EVT_RDL_WRITING_DONE_P1,        emul_reading_p1},
    {ST_RDL_WRITING_P1,         EVT_RDL_WRITING_ERROR,          emul_writing_err_p1},

    {ST_RDL_READING_P1,         EVT_RDL_WRITING_P2,             emul_writing_p2},

    {ST_RDL_WRITING_P2,         EVT_RDL_WRITING_DONE_P2,        emul_reading_p2},
    {ST_RDL_WRITING_P2,         EVT_RDL_WRITING_ERROR,          emul_writing_err_p2},

    {ST_RDL_READING_P2,         EVT_RDL_READING_DONE_P2,        emul_page_done},
    {ST_RDL_READING_P2,         EVT_RDL_READING_ERROR,          emul_reading_err_p2},

    {ST_RDL_WRITING_TOTAL,      EVT_RDL_WRITING_DONE_TOTAL,     emul_writing_total},
    {ST_RDL_WRITING_TOTAL,      EVT_RDL_READING_ERROR_TOTAL,    emul_restart},
    {ST_RDL_WRITING_TOTAL,      EVT_RDL_WRITING_ERROR_TOTAL,    emul_restart},

    {ST_RDL_TERM,               EVT_RDL_NONE,                   emul_terminate},
};

int EMUL_TRANS_MAX = sizeof(emul_fsm_list) / sizeof(RDL_FSM_t);
#endif
#endif /* [#124] */
