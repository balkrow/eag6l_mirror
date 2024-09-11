
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
#if 1/* [#70] Adding RDL feature, dustin, 2024-07-02 */
#include <sys/mman.h>
#include "rdl_fsm.h"
#endif
#if 1 /* [#89] Fixing for RDL changes on Target system, dustin, 2024-08-02 */
#include <mtd/mtd-user.h>
#endif

#if 0/*[#61] Adding omitted functions, dustin, 2024-06-24 */
#define ACCESS_SIM	/* moved to sysmon.h */
#endif
#define DEBUG

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
#if 1 /* [#125] Fixing for SFP channel no, wavelength, tx/rx dBm, dustin, 2024-09-10 */
int i2c_in_use_flag_backup;
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
#if 1/*[#80] eag6l board SW bring-up, balkrow, 2023-07-24 */
extern SVC_EVT svc_sdk_init_wait(SVC_ST st); 
#endif
#if 1/*[#106] init ¿ FPGA update ¿¿ ¿¿, balkrow, 2024-08-28 */
SVC_EVT svc_fpga_update(SVC_ST st);
SVC_EVT svc_fpga_update_fail(SVC_ST st);
SVC_EVT svc_fpga_switch_confirm(SVC_ST st);
SVC_EVT svc_fpga_switch_wait(SVC_ST st);
SVC_EVT svc_fpga_switch_bank(SVC_ST st);
SVC_EVT svc_fpga_switch_failure(SVC_ST st);
#endif

#if 1/* [#70] Adding RDL feature, dustin, 2024-07-02 */
extern char *rdl_get_state_str(int sno);
extern char *rdl_get_event_str(int eno);

extern RDL_FSM_t rdl_fsm_list[];
extern RDL_INFO_LIST_t rdl_info_list;
extern int RDL_TRANS_MAX;
#if 1 /* [#89] Fixing for RDL changes on Target system, dustin, 2024-08-02 */
#if 1 /* [#105] Fixing for RDL install/activation process, dustin, 2024-08-27 */
extern uint16_t RDL_INSTALL_STATE;
#else
extern uint16_t RDL_ACTIVATION_STATE;
#endif
#endif

uint8_t PAGE_CRC_OK;
#if 1 /* [#105] Fixing for RDL install/activation process, dustin, 2024-08-27 */
uint8_t IMG_INSTALL_OK;
#else
uint8_t IMG_ACTIVATION_OK;
uint8_t IMG_RUNNING_OK;
#endif
uint8_t *RDL_PAGE = NULL;
RDL_IMG_INFO_t    RDL_INFO;
#if 1/* [#77] Adding RDL emulation function, dustin, 2024-07-16 */
RDL_IMG_INFO_t    EMUL_INFO;
#endif

fw_header_t       RDL_PKG_HEADER;
fw_image_header_t RDL_OS_HEADER;
fw_image_header_t RDL_OS2_HEADER;
fw_image_header_t RDL_FW_HEADER;
#endif
#if 1 /* [#91] Fixing for register updating feature, dustin, 2024-08-05 */
extern int check_fifo_hello(struct thread *thread);
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
#if 1/*[#80] eag6l board SW bring-up, balkrow, 2023-07-24 */
	gDB.svc_fsm.cb[SVC_ST_SDK_INIT_CHK] = svc_sdk_init_wait;
#endif
#if 1/*[#106] init ¿ FPGA update ¿¿ ¿¿, balkrow, 2024-08-28 */
	gDB.svc_fsm.cb[SVC_ST_FPGA_UPDATE] = svc_fpga_update;
	gDB.svc_fsm.cb[SVC_ST_FPGA_UPDATE_FAILURE] = svc_fpga_update_fail;
	gDB.svc_fsm.cb[SVC_ST_FPGA_SWITCH_CONFIRM] = svc_fpga_switch_confirm;
	gDB.svc_fsm.cb[SVC_ST_FPGA_SWITCH_WAIT] = svc_fpga_switch_wait;
	gDB.svc_fsm.cb[SVC_ST_FPGA_SWITCH] = svc_fpga_switch_bank;
	gDB.svc_fsm.cb[SVC_ST_FPGA_SWITCH_FAILURE] = svc_fpga_switch_failure;
#endif
}


int svc_fsm_timer(struct thread *thread) {
	SVC_ST state;
	SVC_EVT event;

	state = transition(gDB.svc_fsm.state, gDB.svc_fsm.evt);

	if(gDB.svc_fsm.cb[state] != NULL)
		event = gDB.svc_fsm.cb[state](state);

	gDB.svc_fsm.state = state;
#if 1/*[#80] eag6l board SW bring-up, balkrow, 2023-07-22 */
	if(event != SVC_EVT_INIT_FAIL) 
#endif
	gDB.svc_fsm.evt = event;

#ifdef DEBUG
//	zlog_notice("FSM state=%x, evt=%x", gDB.svc_fsm.state, gDB.svc_fsm.evt);
#endif

	thread_add_timer_msec (master, svc_fsm_timer, NULL, 100);
	return 0;
}
#endif

#if 1/* [#70] Adding RDL feature, dustin, 2024-07-02 */
/* crc 32 table */
const u_int32_t crc_table[256] =
{
  0x00000000L, 0x77073096L, 0xee0e612cL, 0x990951baL, 0x076dc419L,
  0x706af48fL, 0xe963a535L, 0x9e6495a3L, 0x0edb8832L, 0x79dcb8a4L,
  0xe0d5e91eL, 0x97d2d988L, 0x09b64c2bL, 0x7eb17cbdL, 0xe7b82d07L,
  0x90bf1d91L, 0x1db71064L, 0x6ab020f2L, 0xf3b97148L, 0x84be41deL,
  0x1adad47dL, 0x6ddde4ebL, 0xf4d4b551L, 0x83d385c7L, 0x136c9856L,
  0x646ba8c0L, 0xfd62f97aL, 0x8a65c9ecL, 0x14015c4fL, 0x63066cd9L,
  0xfa0f3d63L, 0x8d080df5L, 0x3b6e20c8L, 0x4c69105eL, 0xd56041e4L,
  0xa2677172L, 0x3c03e4d1L, 0x4b04d447L, 0xd20d85fdL, 0xa50ab56bL,
  0x35b5a8faL, 0x42b2986cL, 0xdbbbc9d6L, 0xacbcf940L, 0x32d86ce3L,
  0x45df5c75L, 0xdcd60dcfL, 0xabd13d59L, 0x26d930acL, 0x51de003aL,
  0xc8d75180L, 0xbfd06116L, 0x21b4f4b5L, 0x56b3c423L, 0xcfba9599L,
  0xb8bda50fL, 0x2802b89eL, 0x5f058808L, 0xc60cd9b2L, 0xb10be924L,
  0x2f6f7c87L, 0x58684c11L, 0xc1611dabL, 0xb6662d3dL, 0x76dc4190L,
  0x01db7106L, 0x98d220bcL, 0xefd5102aL, 0x71b18589L, 0x06b6b51fL,
  0x9fbfe4a5L, 0xe8b8d433L, 0x7807c9a2L, 0x0f00f934L, 0x9609a88eL,
  0xe10e9818L, 0x7f6a0dbbL, 0x086d3d2dL, 0x91646c97L, 0xe6635c01L,
  0x6b6b51f4L, 0x1c6c6162L, 0x856530d8L, 0xf262004eL, 0x6c0695edL,
  0x1b01a57bL, 0x8208f4c1L, 0xf50fc457L, 0x65b0d9c6L, 0x12b7e950L,
  0x8bbeb8eaL, 0xfcb9887cL, 0x62dd1ddfL, 0x15da2d49L, 0x8cd37cf3L,
  0xfbd44c65L, 0x4db26158L, 0x3ab551ceL, 0xa3bc0074L, 0xd4bb30e2L,
  0x4adfa541L, 0x3dd895d7L, 0xa4d1c46dL, 0xd3d6f4fbL, 0x4369e96aL,
  0x346ed9fcL, 0xad678846L, 0xda60b8d0L, 0x44042d73L, 0x33031de5L,
  0xaa0a4c5fL, 0xdd0d7cc9L, 0x5005713cL, 0x270241aaL, 0xbe0b1010L,
  0xc90c2086L, 0x5768b525L, 0x206f85b3L, 0xb966d409L, 0xce61e49fL,
  0x5edef90eL, 0x29d9c998L, 0xb0d09822L, 0xc7d7a8b4L, 0x59b33d17L,
  0x2eb40d81L, 0xb7bd5c3bL, 0xc0ba6cadL, 0xedb88320L, 0x9abfb3b6L,
  0x03b6e20cL, 0x74b1d29aL, 0xead54739L, 0x9dd277afL, 0x04db2615L,
  0x73dc1683L, 0xe3630b12L, 0x94643b84L, 0x0d6d6a3eL, 0x7a6a5aa8L,
  0xe40ecf0bL, 0x9309ff9dL, 0x0a00ae27L, 0x7d079eb1L, 0xf00f9344L,
  0x8708a3d2L, 0x1e01f268L, 0x6906c2feL, 0xf762575dL, 0x806567cbL,
  0x196c3671L, 0x6e6b06e7L, 0xfed41b76L, 0x89d32be0L, 0x10da7a5aL,
  0x67dd4accL, 0xf9b9df6fL, 0x8ebeeff9L, 0x17b7be43L, 0x60b08ed5L,
  0xd6d6a3e8L, 0xa1d1937eL, 0x38d8c2c4L, 0x4fdff252L, 0xd1bb67f1L,
  0xa6bc5767L, 0x3fb506ddL, 0x48b2364bL, 0xd80d2bdaL, 0xaf0a1b4cL,
  0x36034af6L, 0x41047a60L, 0xdf60efc3L, 0xa867df55L, 0x316e8eefL,
  0x4669be79L, 0xcb61b38cL, 0xbc66831aL, 0x256fd2a0L, 0x5268e236L,
  0xcc0c7795L, 0xbb0b4703L, 0x220216b9L, 0x5505262fL, 0xc5ba3bbeL,
  0xb2bd0b28L, 0x2bb45a92L, 0x5cb36a04L, 0xc2d7ffa7L, 0xb5d0cf31L,
  0x2cd99e8bL, 0x5bdeae1dL, 0x9b64c2b0L, 0xec63f226L, 0x756aa39cL,
  0x026d930aL, 0x9c0906a9L, 0xeb0e363fL, 0x72076785L, 0x05005713L,
  0x95bf4a82L, 0xe2b87a14L, 0x7bb12baeL, 0x0cb61b38L, 0x92d28e9bL,
  0xe5d5be0dL, 0x7cdcefb7L, 0x0bdbdf21L, 0x86d3d2d4L, 0xf1d4e242L,
  0x68ddb3f8L, 0x1fda836eL, 0x81be16cdL, 0xf6b9265bL, 0x6fb077e1L,
  0x18b74777L, 0x88085ae6L, 0xff0f6a70L, 0x66063bcaL, 0x11010b5cL,
  0x8f659effL, 0xf862ae69L, 0x616bffd3L, 0x166ccf45L, 0xa00ae278L,
  0xd70dd2eeL, 0x4e048354L, 0x3903b3c2L, 0xa7672661L, 0xd06016f7L,
  0x4969474dL, 0x3e6e77dbL, 0xaed16a4aL, 0xd9d65adcL, 0x40df0b66L,
  0x37d83bf0L, 0xa9bcae53L, 0xdebb9ec5L, 0x47b2cf7fL, 0x30b5ffe9L,
  0xbdbdf21cL, 0xcabac28aL, 0x53b39330L, 0x24b4a3a6L, 0xbad03605L,
  0xcdd70693L, 0x54de5729L, 0x23d967bfL, 0xb3667a2eL, 0xc4614ab8L,
  0x5d681b02L, 0x2a6f2b94L, 0xb40bbe37L, 0xc30c8ea1L, 0x5a05df1bL,
  0x2d02ef8dL
};

#define DO1(buf) crc = crc_table[((int)crc ^ (*(buf++))) & 0xff] ^ (crc >> 8);
#define DO2(buf)  DO1(buf); DO1(buf);
#define DO4(buf)  DO2(buf); DO2(buf);
#define DO8(buf)  DO4(buf); DO4(buf);


uint32_t gen_crc32(uint32_t crc, char *buf, size_t len)
{
	assert(buf != NULL);
	crc = crc ^ 0xffffffffL;
	while (len >= 8)
	{
		DO8(buf);
		len -= 8;
	}
	if (len) do {
		DO1(buf);
	} while (--len);
	return crc ^ 0xffffffffL;
}

// copy dpram page data to buffer for crc and saving.
void rdl_copy_page_segment_to_buffer(int sno)
{
	uint32_t offset, addr, idx;
	uint16_t data;

	// copy dpram page subpage to buffer.
	for(offset = 0; offset < RDL_PAGE_SEGMENT_SIZE; offset += 2) {
		addr = (sno == RDL_PAGE_SEGMENT_1) ? 
			(RDL_DPRAM_BASE + offset) :
			(RDL_DPRAM_BASE + RDL_PAGE_SEGMENT_SIZE + offset);
		idx = (sno == RDL_PAGE_SEGMENT_1) ?
			offset :
			offset + RDL_PAGE_SEGMENT_SIZE;

		// read dpram.
		data = DPRAM_READ(addr);

		// write to buffer.
#if 1 /* [#89] Fixing for RDL changes on Target system, dustin, 2024-08-02 */
		RDL_PAGE[idx]     = (data & 0xFF00) >> 8;
		RDL_PAGE[idx + 1] = (data & 0xFF);
#else
		RDL_PAGE[idx]     = (data & 0xFF);
		RDL_PAGE[idx + 1] = (data & 0xFF00) >> 8;
#endif
#ifdef DEBUG
		if(0/*offset == 0*/)
		zlog_notice("%s : pno[%u] sno[%u] addr[%x] idx[%u] data[%x] RDL_PAGE[idx]=%04x/%04x.", 
			__func__, RDL_INFO.pno, sno, addr, idx, data, RDL_PAGE[idx], RDL_PAGE[idx+1]);
#endif
	}
	return;
}

// calculate crc.
uint16_t calculate_crc16(const uint8_t * byte, uint32_t count)
{
	uint16_t crc = 0xFFFF;
	uint32_t ii, jj;

	for(ii = 0; ii < count; ++ii )
	{
		crc ^= byte[ii] << 8;
		for(jj = 0; jj < 8; ++jj )
		{
			if(crc & 0x8000) crc = (crc << 1) ^ 0x1021;
			else crc = (crc << 1);
		}
	}

	return crc;
}

// calculate crc.
uint16_t get_sum(uint16_t *addr, int32_t nleft)
{
	uint32_t sum = 0;
	uint8_t *odd_addr;

	while (nleft > 1) {
#if 1 /* [#89] Fixing for RDL changes on Target system, dustin, 2024-08-02 */
		sum += htons(*(addr++));
#else
		sum += *(addr++);
#endif
		nleft -= 2;
	}

	/* Mop up an odd byte, if necessary */
	if (nleft == 1)
	{
		odd_addr = (uint8_t *)addr;
		sum += (*odd_addr << 8);
	}

	return sum;
}

#if 1 /* [#89] Fixing for RDL changes on Target system, dustin, 2024-08-02 */
static int mtd_fd = -1;

#define KB(x) ((x) / 1024)
#define PERCENTAGE(x,total) (((x) * 100) / (total))

int fpga_erase_all(int bno)
{
	struct mtd_info_user mtd;
    struct erase_info_user erase;
	int blocks, ii;

	/* open mtd device. */
	mtd_fd = open((bno == RDL_BANK_1) ? 
		RDL_DEV1_FPGA_FW : RDL_DEV2_FPGA_FW, O_SYNC | O_RDWR);
	if(mtd_fd < 0) {
		zlog_notice("%s : Cannot open mtd %s.", __func__,
			(bno == RDL_BANK_1) ?  RDL_DEV1_FPGA_FW : RDL_DEV2_FPGA_FW);
		return -1;
	}

	/* get mtd info. */
	if(ioctl(mtd_fd, MEMGETINFO, &mtd) < 0) {
		zlog_notice("%s : Cannot get mtd info %s.", __func__,
			(bno == RDL_BANK_1) ?  RDL_DEV1_FPGA_FW : RDL_DEV2_FPGA_FW);
		return -1;
	}

	erase.start = 0;
	erase.length = mtd.size;

	zlog_notice("%s : erasing %s started.", __func__, 
		(bno == RDL_BANK_1) ?  RDL_DEV1_FPGA_FW : RDL_DEV2_FPGA_FW);

	blocks = erase.length / mtd.erasesize;
	erase.length = mtd.erasesize;
	for(ii = 1; ii <= blocks; ii++)
	{
#ifdef DEBUG
		zlog_notice("%s : erasing blocks: %d/%d (%d%%)", 
			__func__, ii, blocks, PERCENTAGE(ii, blocks));
#endif

		if(ioctl(mtd_fd, MEMERASE, &erase) < 0) {
			zlog_notice("%s : failed while erasing blocks " \
				"0x%.8x-0x%.8x on %s: %m", __func__,
				(unsigned int)erase.start, 
				(unsigned int)(erase.start + erase.length), 
				(bno == RDL_BANK_1) ?  RDL_DEV1_FPGA_FW : RDL_DEV2_FPGA_FW);
			return -1;
		}
		erase.start += mtd.erasesize;
	}

	zlog_notice("%s : erased done (100%%) for %s.",
		__func__, 
		(bno == RDL_BANK_1) ?  RDL_DEV1_FPGA_FW : RDL_DEV2_FPGA_FW);
	return 0;
}
#endif

// check crc from page buffer with specified one.
RDL_CRC_t rdl_check_page_crc(void)
{
	uint16_t pcrc = 0xFFFF;
	uint32_t plen, left;

	// read current page crc.
	RDL_INFO.pcrc = DPRAM_READ(RDL_PAGE_CRC_ADDR);

	// get current page length.
	left = RDL_INFO.hd.total_size % RDL_PAGE_SIZE;
	plen = (RDL_INFO.pno < RDL_INFO.total_pno) ? RDL_PAGE_SIZE :
		   (left ? left : RDL_PAGE_SIZE);

	// get calculated page crc.
#if 1
	pcrc = get_sum((uint16_t *)RDL_PAGE, plen);
#else
	pcrc = calculate_crc16(RDL_PAGE, plen);
#endif

	// compare with specified page crc.
	if(pcrc != RDL_INFO.pcrc) {
		zlog_err("%s : Page Checksum[0x%x/0x%x] failed. page[%u/%u] len[%u].",
			__func__, pcrc, RDL_INFO.pcrc, RDL_INFO.pno, RDL_INFO.total_pno, plen);
	}

	return ((pcrc != RDL_INFO.pcrc) ? RDL_CRC_FAIL : RDL_CRC_OK);
}

// read/keep rdl related info from registes, written by MCU.
int rdl_read_img_info(RDL_IMG_INFO_t *pinfo)
{
	uint32_t addr;
	uint16_t ii, data;

    // read rdl img info.
    data = DPRAM_READ(RDL_TARGET_BANK_ADDR);
    pinfo->bno = (data >> 8) & 0xFF;
    pinfo->total_pno = data & 0xFF;
	if((pinfo->bno != 1) && (pinfo->bno != 2)) {
#ifdef DEBUG
		zlog_notice("%s : Invalid Bank Number [%d].", 
			__func__, pinfo->bno);
#endif
		return -1;
	} else if(pinfo->total_pno == 0) {
#ifdef DEBUG
		zlog_notice("%s : Invalid Total Page Number [%d].", 
			__func__, pinfo->total_pno);
#endif
		return -1;
	}
#ifdef DEBUG
	zlog_notice("%s : Bank Number [%d] Total Page No [%d].", 
		__func__, pinfo->bno, pinfo->total_pno);
#endif

#if 1 /* [#89] Fixing for RDL changes on Target system, dustin, 2024-08-02 */
    pinfo->hd.magic = DPRAM_READ(RDL_MAGIC_NO_2_ADDR);
    pinfo->hd.magic |= DPRAM_READ(RDL_MAGIC_NO_1_ADDR) << 16;
	if((! pinfo->hd.magic) || (pinfo->hd.magic != RDL_IMG_MAGIC)) {
#ifdef DEBUG
		zlog_notice("%s : Invalid Magic Number [0x%x(0x%x)].", __func__, 
			pinfo->hd.magic, ntohl(pinfo->hd.magic));
#endif
		return -1;
	}
#ifdef DEBUG
	zlog_notice("%s : Magic Number [0x%x].", __func__, pinfo->hd.magic);
#endif
#else
    pinfo->hd.magic = DPRAM_READ(RDL_MAGIC_NO_1_ADDR);
    pinfo->hd.magic |= (DPRAM_READ(RDL_MAGIC_NO_2_ADDR) << 16);
	if((! pinfo->hd.magic) || (ntohl(pinfo->hd.magic) != RDL_IMG_MAGIC)) {
#ifdef DEBUG
		zlog_notice("%s : Invalid Magic Number [0x%x].", __func__, ntohl(pinfo->hd.magic));
#endif
		return -1;
	}
#ifdef DEBUG
	zlog_notice("%s : Magic Number [0x%x].", __func__, ntohl(pinfo->hd.magic));
#endif
#endif

#if 1 /* [#89] Fixing for RDL changes on Target system, dustin, 2024-08-02 */
    pinfo->hd.total_crc = DPRAM_READ(RDL_TOTAL_CRC_2_ADDR);
    pinfo->hd.total_crc |= DPRAM_READ(RDL_TOTAL_CRC_1_ADDR) << 16;
#else
    pinfo->hd.total_crc = DPRAM_READ(RDL_TOTAL_CRC_1_ADDR);
    pinfo->hd.total_crc |= (DPRAM_READ(RDL_TOTAL_CRC_2_ADDR) << 16);
#endif
	if(! pinfo->hd.total_crc) {
#ifdef DEBUG
		zlog_notice("%s : Invalid Total CRC [0x%x].", __func__, pinfo->hd.total_crc);
#endif
		return -1;
	}
#ifdef DEBUG
	zlog_notice("%s : Total CRC [0x%x].", __func__, pinfo->hd.total_crc);
#endif

#if 1 /* [#89] Fixing for RDL changes on Target system, dustin, 2024-08-02 */
    pinfo->hd.build_time = DPRAM_READ(RDL_BUILD_TIME_2_ADDR);
    pinfo->hd.build_time |= DPRAM_READ(RDL_BUILD_TIME_1_ADDR) << 16;
#else
    pinfo->hd.build_time = DPRAM_READ(RDL_BUILD_TIME_1_ADDR);
    pinfo->hd.build_time |= (DPRAM_READ(RDL_BUILD_TIME_2_ADDR) << 16);
#endif
#ifdef DEBUG
	zlog_notice("%s : Build Time [0x%x].", __func__, pinfo->hd.build_time);
#endif
#if 1 /* [#89] Fixing for RDL changes on Target system, dustin, 2024-08-02 */
    pinfo->hd.total_size = DPRAM_READ(RDL_TOTAL_SIZE_2_ADDR);
    pinfo->hd.total_size |= DPRAM_READ(RDL_TOTAL_SIZE_1_ADDR) << 16;
#else
    pinfo->hd.total_size = DPRAM_READ(RDL_TOTAL_SIZE_1_ADDR);
    pinfo->hd.total_size |= (DPRAM_READ(RDL_TOTAL_SIZE_2_ADDR) << 16);
#endif
	if(! pinfo->hd.total_crc) {
#ifdef DEBUG
		zlog_notice("%s : Invalid Total Size [%u].", __func__, pinfo->hd.total_size);
#endif
		return -1;
	}
#ifdef DEBUG
	zlog_notice("%s : Total Size [%u].", __func__, pinfo->hd.total_size);
#endif

	memset(pinfo->hd.ver_str, 0, sizeof(pinfo->hd.ver_str));
    for(ii = 0, addr = RDL_VER_STR_START_ADDR;
        addr < RDL_VER_STR_END_ADDR; 
		addr += 2) {
        data = DPRAM_READ(addr);
#if 1 /* [#89] Fixing for RDL changes on Target system, dustin, 2024-08-02 */
        pinfo->hd.ver_str[ii++] = (data >> 8) & 0xFF;
        pinfo->hd.ver_str[ii++] = data & 0xFF;
#else
        pinfo->hd.ver_str[ii++] = data & 0xFF;
        pinfo->hd.ver_str[ii++] = (data >> 8) & 0xFF;
#endif
    }
#ifdef DEBUG
	zlog_notice("%s : Version [%s].", __func__, pinfo->hd.ver_str);
#endif

	memset(pinfo->hd.file_name, 0, sizeof(pinfo->hd.file_name));
    for(ii = 0, addr = RDL_FILE_NAME_START_ADDR;
        addr < RDL_FILE_NAME_END_ADDR; 
		addr += 2) {
        data = DPRAM_READ(addr);
#if 1 /* [#89] Fixing for RDL changes on Target system, dustin, 2024-08-02 */
        pinfo->hd.file_name[ii++] = (data >> 8) & 0xFF;
        pinfo->hd.file_name[ii++] = data & 0xFF;
#else
        pinfo->hd.file_name[ii++] = data & 0xFF;
        pinfo->hd.file_name[ii++] = (data >> 8) & 0xFF;
#endif
    }
#ifdef DEBUG
	zlog_notice("%s : Image File Name [%s].", __func__, pinfo->hd.file_name);
#endif

	return 0;
}

// get file size.
long rdl_get_file_size(char *filename)
{
	FILE *fp = NULL;
	long size;
	char temp[150];

#if 1 /* [#110] RDL function Debugging ¿ ¿¿, balkrow, 2024-09-02 */
	if(RDL_INFO.bno == RDL_BANK_1)
		sprintf(temp, "%s%s", RDL_INSTALL1_PATH , RDL_INFO.hd.file_name);
	else if(RDL_INFO.bno == RDL_BANK_2)
		sprintf(temp, "%s%s", RDL_INSTALL2_PATH , RDL_INFO.hd.file_name);
#else
	sprintf(temp, "%s%s", RDL_IMG_PATH, filename);
#endif
	fp = fopen(temp, "r");
	if(fp == NULL) {
		zlog_notice("%s : Can't open a file %s. reason[%s].",
			__func__, filename, strerror(errno));
		return 0;
	}

	fseek(fp, 0, SEEK_END);
	size = ftell(fp);
	return size;
}

// save(append) page data into temp img file.
int rdl_save_page_to_img_file(uint32_t size)
{
	FILE *fp = NULL;
	uint32_t ret;
	char temp[150];

#if 1 /* [#110] RDL function Debugging ¿ ¿¿, balkrow, 2024-09-02 */
	if(RDL_INFO.bno == RDL_BANK_1)
		sprintf(temp, "%s%s", RDL_INSTALL1_PATH , RDL_INFO.hd.file_name);
	else if(RDL_INFO.bno == RDL_BANK_2)
		sprintf(temp, "%s%s", RDL_INSTALL2_PATH , RDL_INFO.hd.file_name);
#else
	sprintf(temp, "%s%s", RDL_IMG_PATH, RDL_INFO.hd.file_name);
#endif

	fp = fopen(temp, "a");
	if(fp == NULL) {
		zlog_err("%s : Can't open file %s. reason[%s].", 
			__func__, temp, strerror(errno));
		return -1;
	}

	ret = fwrite(RDL_PAGE, 1, size, fp);
	fclose(fp);
	if(ret != size) {
		zlog_err("%s : saved byte count[%u] is different with size[%u].", 
			__func__, ret, size);
		return -1;
	}
	return 0;
}

// read fw_header_t from package file.
int get_pkg_fwheader_info(char *fpath, fw_header_t *hdr)
{
	FILE *fp = NULL;

	if (fpath == NULL)
		return -1;

	if ((fp = fopen(fpath, "r")) == NULL) {
		zlog_notice("%s : Cannot open file %s. reason[%s].",
			__func__, fpath, strerror(errno));
		return -1;
	}

	if(fread(hdr, sizeof(fw_header_t) - 448, 1, fp) != 1)
	{
		fclose(fp);
		return -1;
	}

	fclose(fp);
	return 0;
}

// read fw_image_header_t from os img file.
int get_img_fwheader_info(char *fpath, fw_image_header_t *hdr)
{
	FILE *fp = NULL;

	if (fpath == NULL)
		return -1;

	if ((fp = fopen(fpath, "r")) == NULL) {
		zlog_notice("%s : Cannot open file %s. reason[%s].",
			__func__, fpath, strerror(errno));
		return -1;
	}

	if(fread(hdr, sizeof(fw_image_header_t), 1, fp) != 1)
	{
		fclose(fp);
		return -1;
	}

#if 0 /* [#105] Fixing for RDL install/activation process, dustin, 2024-08-27 */
	// check if magic is present.
	if(ntohl(hdr->fih_magic) != RDL_IMG_MAGIC) {
		zlog_notice("%s : Magic not found in %s. [0x%x/0x%x]. FW header not present?",
			__func__, fpath, ntohl(hdr->fih_magic), RDL_IMG_MAGIC);
		return -1;
	}
#endif

	fclose(fp);
	return 0;
}

// check header/data crc against header info.
int check_img_file_crc(char *fpath, fw_image_header_t *hdr, uint32_t *t_size, uint32_t *t_crc)
{
	struct stat fs;
	uint32_t checksum = 0l, calc;
	int fd = -1;
	int ret = -1;
	int len = 0;
	char *data = NULL;
	unsigned char *ptr = NULL;

	if((fpath == NULL) || (hdr == NULL))
		return -1;

	if((fd = open(fpath, O_RDONLY)) < 0) {
		zlog_notice("%s : Can't open file %s.", __func__, fpath);
		goto error_out;
	}

	if(fstat(fd, &fs) < 0) {
		zlog_notice("%s : Can't stat file %s.", __func__, fpath);
		goto error_out;
	}

	if((unsigned)fs.st_size < sizeof(fw_image_header_t)) {
		zlog_notice("%s : Bad size img file %s.", __func__, fpath);
		goto error_out;
	}

	ptr = (unsigned char *)mmap(0, fs.st_size, PROT_READ, MAP_SHARED, fd, 0);
	if((caddr_t)-1 == (caddr_t)ptr) {
		zlog_notice("%s : Can't map to img file %s. reason[%s].", 
			__func__, fpath, strerror(errno));
		goto error_out;
	}

	/*
	 * create copy of header so that we can blank out the
	 * checksum field for checking - this can't be done
	 * on the PROT_READ mapped data.
	 */
	memcpy(hdr, ptr, sizeof(fw_image_header_t));
	if(ntohl(hdr->fih_magic) != RDL_IMG_MAGIC) {
		zlog_notice("%s : Bad magic number [0x%x/0x%x] for %s.", 
			__func__, ntohl(hdr->fih_magic), RDL_IMG_MAGIC, fpath);
		goto error_out;
	}

	if(ntohl(hdr->fih_size) + sizeof(fw_image_header_t) != fs.st_size) {
		zlog_notice("%s : Bad file size [%u/%u] for %s.", 
			__func__, ntohl(hdr->fih_size) + sizeof(fw_image_header_t), fs.st_size, fpath);
		goto error_out;
	}

	data = (char *)hdr;
	len  = sizeof(fw_image_header_t);
	checksum = ntohl(hdr->fih_hcrc);
	hdr->fih_hcrc = ntohl(0);  /* clear for re-calculation */

#if 0//FIXME : which one is ok?
	calc = get_sum((uint16_t *)data, len);
#else
	calc = gen_crc32(0, data, len);
#endif
	if(calc != checksum)
	{
		zlog_notice("%s : Bad header crc [0x%x/0x%x] for %s.", 
			__func__, calc, checksum, fpath);
		goto error_out;
	}

	data = (char *)(ptr + sizeof(fw_image_header_t));
	len  = fs.st_size - sizeof(fw_image_header_t) ;
	checksum = ntohl(hdr->fih_dcrc);

#if 0//FIXME : which one is ok?
	calc = get_sum((uint16_t *)data, len);
#else
	calc = gen_crc32(0, data, len);
#endif
	if(calc != checksum)
	{
		zlog_notice("%s : Bad data crc [0x%x/0x%x] for %s.", 
			__func__, calc, checksum, fpath);
		goto error_out;
	}

	ret = 0;
	if(t_crc)
		*t_crc = get_sum((uint16_t *)ptr, fs.st_size);
	if(t_size)
		*t_size = (uint32_t)fs.st_size;

error_out:
	if(ptr != NULL)
		munmap((void *)ptr, fs.st_size);

	if(fd >= 0)
		close(fd);

	return ret;
}

RDL_CRC_t rdl_check_total_crc(char *filename)
{
	fw_image_header_t hd;
	char tbuf[200];

	if(filename == NULL) {
		zlog_notice("%s : Invalid file.", __func__);
		return RDL_CRC_FAIL;
	}

	// open total pkg file and read to calculate CRC.
#if 1 /* [#110] RDL function Debugging ¿ ¿¿, balkrow, 2024-09-02 */
	if(RDL_INFO.bno == RDL_BANK_1)
		snprintf(tbuf, sizeof(tbuf) - 1, "%s%s", RDL_INSTALL1_PATH , filename);
	else if(RDL_INFO.bno == RDL_BANK_2)
		snprintf(tbuf, sizeof(tbuf) - 1, "%s%s", RDL_INSTALL2_PATH , filename);
#else
	snprintf(tbuf, sizeof(tbuf) - 1, "%s%s", RDL_IMG_PATH, filename);
#endif
	if(check_img_file_crc(tbuf, &hd, NULL, NULL) < 0) {
		zlog_notice("%s : Checking crc failed for pkg file %s.",
			__func__, tbuf);
		return RDL_CRC_FAIL;
	}

	return RDL_CRC_OK;
}

int syscmd_file_exist(char *fpath)
{
    struct stat stat_buf;

    if(! stat(fpath, &stat_buf))
        return 1;
    return 0;
}

// mark active bank flag.
// create bank flag file for bank 2 only.
void set_sw_active_bank_flag(uint8_t bno)
{
	char cmd[128];
	if(bno == RDL_BANK_2) {
		sprintf(cmd, "touch %s", RDL_ACTIVE_BANK_FLAG);
		system(cmd);
	} else {
		unlink(RDL_ACTIVE_BANK_FLAG);
	}
	system("sync");
	return;
}

// get bank no by active bank flag.
int get_sw_active_bank_flag(void)
{
	if(syscmd_file_exist(RDL_ACTIVE_BANK_FLAG))
		return RDL_BANK_2;
	return RDL_BANK_1;
}

#if 1 /* [#91] Fixing for register updating feature, dustin, 2024-08-05 */
// mark fpag fw active bank flag.
// create fpag fw bank flag file for bank 2 only.
void set_fpga_fw_active_bank_flag(uint8_t bno)
{
	char cmd[128];
	int act_bank, stb_bank;

	if(bno == RDL_BANK_1) {
		act_bank = RDL_BANK_1;
		stb_bank = RDL_BANK_2;
		unlink(RDL_FPGA_BANK_FLAG );
	} else {
		act_bank = RDL_BANK_2;
		stb_bank = RDL_BANK_1;
		sprintf(cmd, "touch %s", RDL_FPGA_BANK_FLAG );
		system(cmd);
	}

	/* update fpga fw bank env variable for next loading. */
	sprintf(cmd, "fw_setenv fw_act_bank %d", act_bank);
	system(cmd);
	sprintf(cmd, "fw_setenv fw_stb_bank %d", stb_bank);
	system(cmd);

	system("sync");
	return;
}

// get bank no by active bank flag.
int get_fpga_fw_active_bank_flag(void)
{
	if(syscmd_file_exist(RDL_FPGA_BANK_FLAG ))
		return RDL_BANK_2;
	return RDL_BANK_1;
}
#endif

#if 1 /* [#110] RDL function Debugging ¿ ¿¿, balkrow, 2024-09-02 */
/**
 * return value 
 *	1 : ZIPFILE, 2 : NOT ZIP_FILE, -1 : ERROR  
 * */
#endif
int restore_pkg_file(char *src, char *dst)
{
	int in, out, nread, total_sum = 0;
#if 1 /* [#110] RDL function Debugging ¿ ¿¿, balkrow, 2024-09-02 */
	char block[RDL_BUFF_SIZE] = {0, };
	char file_magic[4] = {0, };
	const char zip_magic[4] = {0x50, 0x4B, 0x03, 0x04};
#endif
	fw_image_header_t hd;

#if 1 /* [#110] RDL function Debugging ¿ ¿¿, balkrow, 2024-09-02 */
	if((in = open(src, O_RDONLY)) <= 0)
		return -1;
#else
	in = open(src, O_RDONLY);
	out = open(dst, O_WRONLY|O_CREAT, S_IRUSR|S_IWUSR);
#endif

#if 1 /* [#110] RDL function Debugging ¿ ¿¿, balkrow, 2024-09-02 */
	lseek(in, sizeof(fw_image_header_t), SEEK_SET);
	read(in, file_magic, 4);	

	if(memcmp(file_magic, zip_magic, 4))
	{
#if 1 /* [#110] RDL function Debugging ¿ ¿¿, balkrow, 2024-09-05 */
		fw_image_header_t header;
		char os_image_name[256] = {0, };
		zlog_notice("img file is not Zipp file");		
		lseek(in, 0, SEEK_SET);
		read(in, &header, sizeof(fw_image_header_t));	

		if(RDL_INFO.bno == RDL_BANK_1)
		{
			sprintf(os_image_name, "%s%s%s.bin", RDL_INSTALL1_PATH, "eag6l-os-v", header.fih_ver);
		}
		else if(RDL_INFO.bno == RDL_BANK_1)
		{
			sprintf(os_image_name, "%s%s%s.bin", RDL_INSTALL2_PATH, "eag6l-os-v", header.fih_ver);
		}

		if((out = open(os_image_name, O_WRONLY|O_CREAT, S_IRUSR|S_IWUSR)) <= 0) 
		{
			close(in); 
			zlog_notice("%s create failed %s", dst, strerror(errno));		
			return -1;
		}
		else
			zlog_notice("%s created", os_image_name);		

		lseek(in, sizeof(fw_image_header_t), SEEK_SET);
		// read/write to file and calculate crc.
		while ((nread = read(in, block, RDL_BUFF_SIZE)) > 0) {
			total_sum += get_sum((uint16_t *)block, nread);
			write(out, block, nread);
		}
		close(in); 
		close(out);
#endif
		return PKG_NONZIP; 
	}

	zlog_notice("%s is Zipp file", src);		

	if((out = open(dst, O_WRONLY|O_CREAT, S_IRUSR|S_IWUSR)) <= 0) 
	{
		close(in); 
		zlog_notice("%s create failed %s", dst, strerror(errno));		
		return -1;
	}
	else
		zlog_notice("%s created", dst);		

	lseek(in, sizeof(fw_image_header_t), SEEK_SET);
	// read/write to file and calculate crc.
	while ((nread = read(in, block, RDL_BUFF_SIZE)) > 0) {
		total_sum += get_sum((uint16_t *)block, nread);
		write(out, block, nread);
	}

	close(in); 
	close(out);

#else
	// read pkg header to skip it.
	if((nread = read(in, &hd, sizeof(fw_image_header_t))) <= 0)
		return -1;
	else
		total_sum += get_sum((uint16_t *)&hd, nread);

#ifdef DEBUG
	zlog_notice("%s : ----- RDL Information", __func__);
	zlog_notice("%s : size %u, header crc %x, data crc %x", 
		__func__, ntohl(hd.fih_size), ntohl(hd.fih_hcrc), ntohl(hd.fih_dcrc));
	zlog_notice("%s : ver %s file %s", __func__, hd.fih_ver, hd.fih_name);
#endif

	// read/write to file and calculate crc.
	while ((nread = read(in, block, RDL_BUFF_SIZE)) > 0) {
		total_sum += get_sum((uint16_t *)block, nread);
		write(out, block, nread);
	}

#endif

#if 0 /* [#89] Fixing for RDL changes on Target system, dustin, 2024-08-02 */
	/*NOTE : fih_dcrc and total_crc comes from different crc calculation. cannot match them. */
	if(total_sum != ntohl(hd.fih_dcrc)) {
		zlog_notice("%s : Checking chksum by get_sum() failed. [0x%x/0x%x]",
			__func__, total_sum, ntohl(hd.fih_dcrc));
	}
#endif

	return PKG_ZIP;
}

int restore_img_file(char *src, char *dst)
{
	int in, out, nread, total_sum = 0;
	char block[RDL_BUFF_SIZE];
	fw_image_header_t hd;

	in = open(src, O_RDONLY);
	out = open(dst, O_WRONLY|O_CREAT, S_IRUSR|S_IWUSR);

	// read pkg header to skip it.
	if((nread = read(in, &hd, sizeof(fw_image_header_t))) <= 0)
		return -1;
	else
		total_sum += get_sum((uint16_t *)&hd, nread);

#ifdef DEBUG
	zlog_notice("RDL] ----- IMG Information");
	zlog_notice("RDL] magic 0x%x size %u, h-crc 0x%x, d-crc 0x%x",
			hd.fih_magic, hd.fih_size, hd.fih_hcrc, hd.fih_dcrc);
	zlog_notice("RDL] version : %s", hd.fih_ver);
	zlog_notice("RDL] filename : %s", hd.fih_name);
#endif

	// read/write to file and calculate crc.
	while ((nread = read(in, block, RDL_BUFF_SIZE)) > 0)
	{
		total_sum += get_sum((uint16_t *)block, nread);
		write(out, block, nread);
	}

	close(in); close(out);

	total_sum = (uint16_t)total_sum;
	if(total_sum)
	{
		zlog_notice("RDL] Checking chksum : failed. [0x%x/0x%x]",
				total_sum, hd.fih_dcrc);
		return -1;
	}
#ifdef DEBUG
	zlog_notice("RDL] Checking chksum : success.");
#endif
	return 0;
}

#if 1 /* [#89] Fixing for RDL changes on Target system, dustin, 2024-08-02 */
int rdl_collect_img_header_info(int bno, char *fname, fw_image_header_t *hd)
#else
int rdl_collect_img_header_info(char *fname, fw_image_header_t *hd)
#endif
{
	struct stat fs;
	uint32_t checksum = 0l, calc;
	int fd = -1;
	int ret = -1;
	int major, minor, rev;
#if 1 /* [#105] Fixing for RDL install/activation process, dustin, 2024-08-27 */
	int len;
#endif
	char fpath[200];
	char *str = NULL;
	unsigned char *ptr = NULL;

	if((fname == NULL) || (hd == NULL))
		return -1;

#if 1 /* [#89] Fixing for RDL changes on Target system, dustin, 2024-08-02 */
	if(bno == RDL_BANK_1)
		snprintf(fpath, sizeof(fpath) - 1, "%s%s", RDL_INSTALL1_PATH , fname);
	else if(bno == RDL_BANK_2)
		snprintf(fpath, sizeof(fpath) - 1, "%s%s", RDL_INSTALL2_PATH , fname);
	else
		snprintf(fpath, sizeof(fpath) - 1, "%s%s", RDL_IMG_PATH, fname);
#else
	snprintf(fpath, sizeof(fpath) - 1, "%s%s", RDL_IMG_PATH, fname);
#endif
	if((fd = open(fpath, O_RDONLY)) < 0) {
		zlog_notice("%s : Can't open file %s.", __func__, fpath);
		goto error_out;
	}

	if(fstat(fd, &fs) < 0) {
		zlog_notice("%s : Can't stat file %s.", __func__, fpath);
		goto error_out;
	}

	ptr = (unsigned char *)mmap(0, fs.st_size, PROT_READ, MAP_SHARED, fd, 0);
	if((caddr_t)-1 == (caddr_t)ptr) {
		zlog_notice("%s : Can't map to img file %s. reason[%s].", 
			__func__, fpath, strerror(errno));
		goto error_out;
	}

#if 0//FIXME : which one is ok?
	calc = get_sum((uint16_t *)ptr, fs.st_size);
#else
	calc = gen_crc32(0, ptr, fs.st_size);
#endif

	hd->fih_magic = RDL_IMG_MAGIC;
	hd->fih_size  = fs.st_size;
	hd->fih_dcrc  = calc;
	hd->fih_time  = fs.st_mtime;

	strncpy(hd->fih_name, fname, sizeof(hd->fih_name));

	str = strstr(fname, "v");
	if(str) {
#if 1 /* [#105] Fixing for RDL install/activation process, dustin, 2024-08-27 */
		if((len = sscanf(str, "v%d.%d.%d", &major, &minor, &rev)) != 3) {
			if((len = sscanf(str, "v%d.%d", &major, &minor)) != 2) {
				zlog_notice("%s : Invalid version string [%s].", __func__, str);
				goto error_out;
			}
		}

		if(len == 3)
			sprintf(hd->fih_ver, "v%d.%d.%d", major, minor, rev);
		else
			sprintf(hd->fih_ver, "v%d.%d", major, minor);
#else
		if(sscanf(str, "v%d.%d.%d", &major, &minor, &rev) != 3) {
			zlog_notice("%s : Invalid version string [%s].", __func__, str);
			goto error_out;
		}

		sprintf(hd->fih_ver, "v%d.%d.%d", major, minor, rev);
#endif /* [#105] */
	} else {
#if 1 /* [#89] Fixing for RDL changes on Target system, dustin, 2024-08-02 */
		zlog_notice("%s : Cannot find 'v' for version from [%s]. Force v1.0.0.", 
			__func__, fname);
		sprintf(hd->fih_ver, "v1.0.0");
#else
		zlog_notice("%s : Cannot find 'v' for version from [%s].", 
			__func__, fname);
		goto error_out;
#endif
	}

	ret = 0;

error_out:
	if(ptr != NULL)
		munmap((void *)ptr, fs.st_size);

	if(fd >= 0)
		close(fd);

	return ret;
}

#if 0
// update rdl registers.
#if 1 /* [#89] Fixing for RDL changes on Target system, dustin, 2024-08-02 */
#if 1 /* [#105] Fixing for RDL install/activation process, dustin, 2024-08-27 */
void rdl_update_bank_registers(int bno, int erase_flag)
#else
void rdl_update_bank_registers(int bno)
#endif
{
	const uint32_t magic1_addr[] = { BANK1_MAGIC_NO_1_ADDR,      BANK2_MAGIC_NO_1_ADDR };
	const uint32_t magic2_addr[] = { BANK1_MAGIC_NO_2_ADDR,      BANK2_MAGIC_NO_2_ADDR };
#if 1 /* [#102] Fixing some register updates, dustin, 2024-08-26 */
	const uint32_t hcrc1_addr[] = { BANK1_HEADER_CRC_1_ADDR,      BANK2_HEADER_CRC_1_ADDR };
	const uint32_t hcrc2_addr[] = { BANK1_HEADER_CRC_2_ADDR,      BANK2_HEADER_CRC_2_ADDR };
#endif
	const uint32_t tcrc1_addr[] = { BANK1_TOTAL_CRC_1_ADDR,      BANK2_TOTAL_CRC_1_ADDR };
	const uint32_t tcrc2_addr[] = { BANK1_TOTAL_CRC_2_ADDR,      BANK2_TOTAL_CRC_2_ADDR };
	const uint32_t btime1_addr[] = { BANK1_BUILD_TIME_1_ADDR,    BANK2_BUILD_TIME_1_ADDR };
	const uint32_t btime2_addr[] = { BANK1_BUILD_TIME_2_ADDR,    BANK2_BUILD_TIME_2_ADDR };
	const uint32_t tsize1_addr[] = { BANK1_TOTAL_SIZE_1_ADDR,    BANK2_TOTAL_SIZE_1_ADDR };
	const uint32_t tsize2_addr[] = { BANK1_TOTAL_SIZE_2_ADDR,    BANK2_TOTAL_SIZE_2_ADDR };
#if 1 /* [#102] Fixing some register updates, dustin, 2024-08-26 */
	const uint32_t ctype1_addr[] = { BANK1_CARD_TYPE_1_ADDR,    BANK2_CARD_TYPE_1_ADDR };
	const uint32_t ctype2_addr[] = { BANK1_CARD_TYPE_2_ADDR,    BANK2_CARD_TYPE_2_ADDR };
#endif
	const uint32_t vstr1_addr[] = { BANK1_VER_STR_START_ADDR,    BANK2_VER_STR_START_ADDR };
	const uint32_t vstr2_addr[] = { BANK1_VER_STR_END_ADDR,      BANK2_VER_STR_END_ADDR };
	const uint32_t fname1_addr[] = { BANK1_FILE_NAME_START_ADDR, BANK2_FILE_NAME_START_ADDR };
	const uint32_t fname2_addr[] = { BANK1_FILE_NAME_END_ADDR,   BANK2_FILE_NAME_END_ADDR };
	char ver_str[RDL_VER_STR_MAX];
	char fname[RDL_FILE_NAME_MAX];
	uint32_t data2, addr;
	uint16_t data, ii;

#if 1 /* [#105] Fixing for RDL install/activation process, dustin, 2024-08-27 */
	if(! erase_flag) {
#endif
#ifndef RDL_BIN_HEADER/* NOTE : no header for binary image. */
#if 1 /* [#105] Fixing for RDL install/activation process, dustin, 2024-08-27 */
		/* get pkg info */
		get_pkg_fwheader_info((bno == RDL_BANK_1) ?
			RDL_B1_PKG_INFO_FILE : RDL_B2_PKG_INFO_FILE, &RDL_PKG_HEADER);
#endif

	if(rdl_collect_img_header_info(bno, RDL_PKG_HEADER.ih_image1_str, 
		&RDL_OS_HEADER) < 0) {
		zlog_notice("%s : Collecting header failed for os1 image %s.",
			__func__, RDL_PKG_HEADER.ih_image1_str);
		return;
	}
#else//////////////////////////////////////////////////////////////////
	// get pkg header from specified bank.
	if(get_pkg_fwheader_info((bno == RDL_BANK_1) ? 
		RDL_B1_PKG_INFO_FILE : RDL_B2_PKG_INFO_FILE, &RDL_OS_HEADER) < 0) {
		zlog_notice("%s : Cannot read pkg info for bank%d.", __func__, bno);
		return;
	}
#endif
#if 1 /* [#105] Fixing for RDL install/activation process, dustin, 2024-08-27 */
	}
	else
		zlog_notice("%s : Clearing flag is set for bank[%d].", __func__, bno);
#endif

	// write magic.
	data2 = RDL_OS_HEADER.fih_magic;
#if 1 /* [#105] Fixing for RDL install/activation process, dustin, 2024-08-27 */
	data2 = erase_flag ? 0x0 : data2;
#endif

	DPRAM_WRITE(magic1_addr[bno - RDL_BANK_1], (data2 >> 16) & 0xFFFF);
	DPRAM_WRITE(magic2_addr[bno - RDL_BANK_1], data2 & 0xFFFF);

#if 1 /* [#102] Fixing some register updates, dustin, 2024-08-26 */
	// write header crc.
	data2 = RDL_OS_HEADER.fih_hcrc;
#if 1 /* [#105] Fixing for RDL install/activation process, dustin, 2024-08-27 */
	data2 = erase_flag ? 0x0 : data2;
#endif
	DPRAM_WRITE(hcrc1_addr[bno - RDL_BANK_1], (data2 >> 16) & 0xFFFF);
	DPRAM_WRITE(hcrc2_addr[bno - RDL_BANK_1], data2 & 0xFFFF);
#endif/* [#102] */

	// write total crc.
	data2 = RDL_OS_HEADER.fih_dcrc;
#if 1 /* [#105] Fixing for RDL install/activation process, dustin, 2024-08-27 */
	data2 = erase_flag ? 0x0 : data2;
#endif
	DPRAM_WRITE(tcrc1_addr[bno - RDL_BANK_1], (data2 >> 16) & 0xFFFF);
	DPRAM_WRITE(tcrc2_addr[bno - RDL_BANK_1], data2 & 0xFFFF);

	// write build time.
	data2 = RDL_OS_HEADER.fih_time;
#if 1 /* [#105] Fixing for RDL install/activation process, dustin, 2024-08-27 */
	data2 = erase_flag ? 0x0 : data2;
#endif
	DPRAM_WRITE(btime1_addr[bno - RDL_BANK_1], (data2 >> 16) & 0xFFFF);
	DPRAM_WRITE(btime2_addr[bno - RDL_BANK_1], data2 & 0xFFFF);

	// write total size.
	data2 = RDL_OS_HEADER.fih_size;
#if 1 /* [#105] Fixing for RDL install/activation process, dustin, 2024-08-27 */
	data2 = erase_flag ? 0x0 : data2;
#endif
	DPRAM_WRITE(tsize1_addr[bno - RDL_BANK_1], (data2 >> 16) & 0xFFFF);
	DPRAM_WRITE(tsize2_addr[bno - RDL_BANK_1], data2 & 0xFFFF);

#if 1 /* [#102] Fixing some register updates, dustin, 2024-08-26 */
	// write card type.
	data2 = RDL_OS_HEADER.fih_card_type;
#if 1 /* [#105] Fixing for RDL install/activation process, dustin, 2024-08-27 */
	data2 = erase_flag ? 0x0 : data2;
#endif
	DPRAM_WRITE(ctype1_addr[bno - RDL_BANK_1], (data2 >> 16) & 0xFFFF);
	DPRAM_WRITE(ctype2_addr[bno - RDL_BANK_1], data2 & 0xFFFF);
#endif/* [#102] */

	// write ver string.
    for(ii = 0, addr = vstr1_addr[bno - RDL_BANK_1];
        addr < vstr2_addr[bno - RDL_BANK_1]; 
		addr += 2, ii += 2) {
        data  = RDL_OS_HEADER.fih_ver[ii + 1];
        data |= (uint16_t)(RDL_OS_HEADER.fih_ver[ii] << 8);
#if 1 /* [#105] Fixing for RDL install/activation process, dustin, 2024-08-27 */
		data = erase_flag ? 0x0 : data;
#endif
        DPRAM_WRITE(addr, data);
    }

	// write file name.
    for(ii = 0, addr = fname1_addr[bno - RDL_BANK_1];
        addr < fname2_addr[bno - RDL_BANK_1]; 
		addr += 2, ii += 2) {
        data  = RDL_OS_HEADER.fih_name[ii + 1];
        data |= (uint16_t)(RDL_OS_HEADER.fih_name[ii] << 8);
#if 1 /* [#105] Fixing for RDL install/activation process, dustin, 2024-08-27 */
		data = erase_flag ? 0x0 : data;
#endif
        DPRAM_WRITE(addr, data);
    }
	return;
}
#else /*******************************************************/
void rdl_update_bank_registers(void)
{
	const uint32_t magic1_addr[] = { BANK1_MAGIC_NO_1_ADDR,      BANK2_MAGIC_NO_1_ADDR };
	const uint32_t magic2_addr[] = { BANK1_MAGIC_NO_2_ADDR,      BANK2_MAGIC_NO_2_ADDR };
	const uint32_t tcrc1_addr[] = { BANK1_TOTAL_CRC_1_ADDR,      BANK2_TOTAL_CRC_1_ADDR };
	const uint32_t tcrc2_addr[] = { BANK1_TOTAL_CRC_2_ADDR,      BANK2_TOTAL_CRC_2_ADDR };
	const uint32_t btime1_addr[] = { BANK1_BUILD_TIME_1_ADDR,    BANK2_BUILD_TIME_1_ADDR };
	const uint32_t btime2_addr[] = { BANK1_BUILD_TIME_2_ADDR,    BANK2_BUILD_TIME_2_ADDR };
	const uint32_t tsize1_addr[] = { BANK1_TOTAL_SIZE_1_ADDR,    BANK2_TOTAL_SIZE_1_ADDR };
	const uint32_t tsize2_addr[] = { BANK1_TOTAL_SIZE_2_ADDR,    BANK2_TOTAL_SIZE_2_ADDR };
	const uint32_t vstr1_addr[] = { BANK1_VER_STR_START_ADDR,    BANK2_VER_STR_START_ADDR };
	const uint32_t vstr2_addr[] = { BANK1_VER_STR_END_ADDR,      BANK2_VER_STR_END_ADDR };
	const uint32_t fname1_addr[] = { BANK1_FILE_NAME_START_ADDR, BANK2_FILE_NAME_START_ADDR };
	const uint32_t fname2_addr[] = { BANK1_FILE_NAME_END_ADDR,   BANK2_FILE_NAME_END_ADDR };
	char ver_str[RDL_VER_STR_MAX];
	char fname[RDL_FILE_NAME_MAX];
	uint32_t data2, addr;
	uint16_t data, ii;

	// get current bank no. 
	bno = get_sw_active_bank_flag();

	// get pkg header from specified bank.
	if(get_pkg_fwheader_info((bno == RDL_BANK_1) ? 
		RDL_B1_PKG_INFO_FILE : RDL_B2_PKG_INFO_FILE, &RDL_OS_HEADER) < 0) {
		zlog_notice("%s : Cannot read pkg info for bank%d.", __func__, bno);
		return;
	}

	// write magic.
	data2 = RDL_OS_HEADER.fih_magic;
	DPRAM_WRITE(magic1_addr[bno - RDL_BANK_1], (uint16_t)(data2 & 0xFFFF));
	DPRAM_WRITE(magic2_addr[bno - RDL_BANK_1], (uint16_t)(data2 >> 16) & 0xFFFF);

	// write total crc.
	data2 = RDL_OS_HEADER.fih_dcrc;
	DPRAM_WRITE(tcrc1_addr[bno - RDL_BANK_1], (uint16_t)(data2 & 0xFFFF));
	DPRAM_WRITE(tcrc2_addr[bno - RDL_BANK_1], (uint16_t)(data2 >> 16) & 0xFFFF);

	// write build time.
	data2 = RDL_OS_HEADER.fih_time;
	DPRAM_WRITE(btime1_addr[bno - RDL_BANK_1], (uint16_t)(data2 & 0xFFFF));
	DPRAM_WRITE(btime2_addr[bno - RDL_BANK_1], (uint16_t)(data2 >> 16) & 0xFFFF);

	// write total size.
	data2 = RDL_OS_HEADER.fih_size;
	DPRAM_WRITE(tsize1_addr[bno - RDL_BANK_1], (uint16_t)(data2 & 0xFFFF));
	DPRAM_WRITE(tsize1_addr[bno - RDL_BANK_1], (uint16_t)(data2 >> 16) & 0xFFFF);

	// write ver string.
    for(ii = 0, addr = vstr1_addr[bno - RDL_BANK_1];
        addr < vstr2_addr[bno - RDL_BANK_1]; 
		addr += 2) {
        data  = RDL_OS_HEADER.fih_ver[ii++];
        data |= (uint16_t)(RDL_OS_HEADER.fih_ver[ii++] << 8);
        DPRAM_WRITE(addr, data);
    }

	// write file name.
    for(ii = 0, addr = fname1_addr[bno - RDL_BANK_1];
        addr < fname2_addr[bno - RDL_BANK_1]; 
		addr += 2) {
        data  = RDL_OS_HEADER.fih_name[ii++];
        data |= (uint16_t)(RDL_OS_HEADER.fih_name[ii++] << 8);
        DPRAM_WRITE(addr, data);
    }
	return;
}
#endif
#endif

// decompress pkg file into original bp os & fpga f/w img files.
int rdl_decompress_package_file(char *filename)
{
	fw_image_header_t hd;
	int result = -1, retry;
	char cmd[250], fsrc[100], fdst[100];
#if 1 /* [#110] RDL function Debugging ¿ ¿¿, balkrow, 2024-09-02 */
	uint8_t isZipFile;
#endif

#if 1 /* [#110] RDL function Debugging ¿ ¿¿, balkrow, 2024-09-02 */
	if(RDL_INFO.bno == RDL_BANK_1)
		snprintf(fsrc, sizeof(fsrc) - 1, "%s%s", RDL_INSTALL1_PATH , filename);
	else if(RDL_INFO.bno == RDL_BANK_2)
		snprintf(fsrc, sizeof(fsrc) - 1, "%s%s", RDL_INSTALL2_PATH , filename);
#else
	snprintf(fsrc, sizeof(fsrc) - 1, "%s%s", RDL_IMG_PATH, filename);
#endif

	// get img header from pkg file.
	if(get_img_fwheader_info(fsrc, &hd) < 0) {
		zlog_notice("%s : No fw header for pkg file %s.",
				__func__, fsrc);
		unlink(fsrc);
		goto __return__;
	}

	// header info must be same as RDL_INFO.
#if 1 /* [#110] RDL function Debugging ¿ ¿¿, balkrow, 2024-09-02 */
#if 1 /* [#89] Fixing for RDL changes on Target system, dustin, 2024-08-02 */
	if(htonl(hd.fih_magic) != RDL_INFO.hd.magic) {
		zlog_notice("Different magic [0x%x/0x%x] ? Or No header present ?",
			htonl(hd.fih_magic), RDL_INFO.hd.magic);
		unlink(fsrc);
		goto __return__;
	}
	if(htonl(hd.fih_size) != (RDL_INFO.hd.total_size - sizeof(fw_image_header_t))) {
		zlog_notice("Different file size [%u/%u].",
			ntohl(hd.fih_size), 
			(RDL_INFO.hd.total_size - sizeof(fw_image_header_t)));
		unlink(fsrc);
		goto __return__;
	}
#endif
#endif
	/*check zip file*/

	// remove img header and get zipped pkg(os + fw + .pkg_info) file.
#if 1 /* [#110] RDL function Debugging ¿ ¿¿, balkrow, 2024-09-05 */
	if((isZipFile = restore_pkg_file(fsrc, RDL_TEMP_ZIP_FILE)) == PKG_ERROR)
	{
		goto __return__;
	}

	if(isZipFile == PKG_ZIP)
	{
		gDB.pkg_is_zip = PKG_ZIP;
		if(RDL_INFO.bno == RDL_BANK_1)
			snprintf(cmd, sizeof(cmd) - 1, "unzip -o -q %s -d %s", 
				 RDL_TEMP_ZIP_FILE, RDL_INSTALL1_PATH);
		else if(RDL_INFO.bno == RDL_BANK_2)
			snprintf(cmd, sizeof(cmd) - 1, "unzip -o -q %s -d %s", 
				 RDL_TEMP_ZIP_FILE, RDL_INSTALL2_PATH);
		else
		{
			zlog_notice("Bank %d unzip not excuted.", RDL_INFO.bno);
			goto __return__;
		}

		result = system(cmd);
		if(result != 0) {
			zlog_notice("Decompress failed. reason[%s].", 
				    strerror(errno));
			unlink(RDL_TEMP_ZIP_FILE);
			result = -1;
			goto __return__;
		}
	}
	else
		gDB.pkg_is_zip = PKG_NONZIP;
#endif


	/*bank write*/
	switch(RDL_INFO.bno)
	{
		case RDL_BANK_1:
			if(get_pkg_header(RDL_BANK_1, &(gDB.bank1_header)) == RT_OK)
				write_pkg_header(RDL_BANK_1, &(gDB.bank1_header));
			break;
		case RDL_BANK_2:
			if(get_pkg_header(RDL_BANK_2, &(gDB.bank2_header)) == RT_OK)
				write_pkg_header(RDL_BANK_2, &(gDB.bank2_header));
			break;
		default: 
			break;
	}
	unlink(RDL_TEMP_ZIP_FILE);

	// remove pkg file.
#if 0 /* [#110] RDL function Debugging ¿ ¿¿, balkrow, 2024-09-02 */
	unlink(RDL_TEMP_ZIP_FILE);

	// now we get bp os, fpga f/w, and .pkg_info files.
	// check if fw header info file is present.
	if(! syscmd_file_exist(RDL_PKG_INFO_FILE)) {
		zlog_notice("%s : Not found pkg header %s.", __func__, RDL_PKG_INFO_FILE);
		result = -1;
		goto __return__;
	}

	// read pkg header info.
	get_pkg_fwheader_info(RDL_PKG_INFO_FILE, &RDL_PKG_HEADER);

	// check if os1 is present.
	if(strlen(RDL_PKG_HEADER.ih_image1_str)) {
		snprintf(fsrc, sizeof(fsrc) - 1, "%s%s", RDL_IMG_PATH, 
			RDL_PKG_HEADER.ih_image1_str);
		// check if file is present.
		if(! syscmd_file_exist(fsrc)) {
			zlog_notice("%s : Not found OS1 img %s.", 
				__func__, RDL_PKG_HEADER.ih_image1_str);
			result = -1;
			goto __return__;
		}

#ifndef RDL_BIN_HEADER// NOTE : no header for binary image.
#if 1 /* [#89] Fixing for RDL changes on Target system, dustin, 2024-08-02 */
		if(rdl_collect_img_header_info(0/*no-bank*/, 
			RDL_PKG_HEADER.ih_image1_str, &RDL_OS_HEADER) < 0)
#else
		if(rdl_collect_img_header_info(RDL_PKG_HEADER.ih_image1_str, &RDL_OS_HEADER) < 0)
#endif
		{
			zlog_notice("%s : Collecting header failed for os1 image %s.",
				__func__, RDL_PKG_HEADER.ih_image1_str);
			result = -1;
			goto __return__;
		}
#else//////////////////////////////////////////////////////////////////
		// try to get bp os header from bp os img file.
		if(get_img_fwheader_info(fsrc, &RDL_OS_HEADER) >= 0) {
			// check img file crc with data crc of header.
			if(check_img_file_crc(fsrc, &hd, NULL, NULL) < 0) {
				zlog_notice("%s : Checking crc failed for os1 image %s.",
					__func__, RDL_PKG_HEADER.ih_image1_str);
				result = -1;
				goto __return__;
			}

			// restore original bp os img file.
			snprintf(fdst, sizeof(fdst) - 1, "%s_%s", RDL_IMG_PATH, 
					RDL_PKG_HEADER.ih_image1_str);
			restore_img_file(fsrc, fdst);

			// remove os1 with header file, replace with original os1 file.
			unlink(fsrc);
			snprintf(cmd, sizeof(cmd) - 1, "mv %s %s", fdst, fsrc);
			system(cmd);
			system("sync");
		} else {
			zlog_notice("%s : No fw header for os1 image %s. Leave it.",
				__func__, RDL_PKG_HEADER.ih_image1_str);
		}
#endif
	} else {
		zlog_notice("%s : No os1 file ? %s.", __func__, RDL_PKG_HEADER.ih_image1_str);
	}

	// check if fpga is present.
	if(strlen(RDL_PKG_HEADER.ih_image2_str)) {
		snprintf(fsrc, sizeof(fsrc) - 1, "%s%s", RDL_IMG_PATH, 
			RDL_PKG_HEADER.ih_image2_str);
		// check if file is present.
		if(! syscmd_file_exist(fsrc)) {
			zlog_notice("%s : Not found fpga img %s.", 
				__func__, RDL_PKG_HEADER.ih_image2_str);
			result = -1;
			goto __return__;
		}

#ifndef RDL_BIN_HEADER// NOTE : no header for binary image.
#if 1 /* [#89] Fixing for RDL changes on Target system, dustin, 2024-08-02 */
		if(rdl_collect_img_header_info(0/*no-bank*/, 
			RDL_PKG_HEADER.ih_image2_str, &RDL_OS2_HEADER) < 0)
#else
		if(rdl_collect_img_header_info(RDL_PKG_HEADER.ih_image2_str, &RDL_OS2_HEADER) < 0)
#endif
		{
			zlog_notice("%s : Collecting header failed for fpga image %s.",
				__func__, RDL_PKG_HEADER.ih_image2_str);
			result = -1;
			goto __return__;
		}
#else//////////////////////////////////////////////////////////////////
		// try to get bp os header from bp os img file.
		if(get_img_fwheader_info(fsrc, &RDL_OS2_HEADER) >= 0) {
			// check img file crc with data crc of header.
			if(check_img_file_crc(fsrc, &hd, NULL, NULL) < 0) {
				zlog_notice("%s : Checking crc failed for fpga image %s.",
					__func__, RDL_PKG_HEADER.ih_image2_str);
				result = -1;
				goto __return__;
			}

			// restore original bp os img file.
			snprintf(fdst, sizeof(fdst) - 1, "%s_%s", RDL_IMG_PATH, 
				RDL_PKG_HEADER.ih_image2_str);
			restore_img_file(fsrc, fdst);

			// remove fpga with header file, replace with original fpga file.
			unlink(fsrc);
			snprintf(cmd, sizeof(cmd) - 1, "mv %s %s", fdst, fsrc);
			system(cmd);
			system("sync");
		} else {
			zlog_notice("%s : No fw header for fpga image %s. Leave it.",
				__func__, RDL_PKG_HEADER.ih_image2_str);
		}
#endif
	} else {
		zlog_notice("%s : No fpga file ? %s.", __func__, RDL_PKG_HEADER.ih_image2_str);
	}
#endif

#if 0 /* [#105] Fixing for RDL install/activation process, dustin, 2024-08-27 */
	// check if fpga f/w is present.
	if(strlen(RDL_PKG_HEADER.ih_image3_str)) {
		snprintf(fsrc, sizeof(fsrc) - 1, "%s%s", RDL_IMG_PATH, 
			RDL_PKG_HEADER.ih_image3_str);
		// check if file is present.
		if(! syscmd_file_exist(fsrc)) {
			zlog_notice("%s : Not found FPGA F/W %s.", 
				__func__, RDL_PKG_HEADER.ih_image3_str);
			result = -1;
			goto __return__;
		}

#ifndef RDL_BIN_HEADER// NOTE : no header for binary image.
#if 1 /* [#89] Fixing for RDL changes on Target system, dustin, 2024-08-02 */
		if(rdl_collect_img_header_info(0/*no-bank*/, 
			RDL_PKG_HEADER.ih_image3_str, &RDL_FW_HEADER) < 0)
#else
		if(rdl_collect_img_header_info(RDL_PKG_HEADER.ih_image3_str, &RDL_FW_HEADER) < 0)
#endif
		{
			zlog_notice("%s : Collecting header failed for os3 image %s.",
				__func__, RDL_PKG_HEADER.ih_image3_str);
			result = -1;
			goto __return__;
		}
#else//////////////////////////////////////////////////////////////////
		// get fw header from fpga fw img file.
		if(get_img_fwheader_info(fsrc, &RDL_FW_HEADER) >= 0) {
			// check img file crc with data crc of header.
			if(check_img_file_crc(fsrc, &hd, NULL, NULL) < 0) {
				zlog_notice("%s : Checking crc failed for FPGA F/W image %s.",
					__func__, RDL_PKG_HEADER.ih_image3_str);
				result = -1;
				goto __return__;
			}

			// restore original bp os img file.
			snprintf(fdst, sizeof(fdst) - 1, "%s_%s", RDL_IMG_PATH, 
				RDL_PKG_HEADER.ih_image3_str);
			restore_img_file(fsrc, fdst);

			// remove fw with header file, replace with original fw file.
			unlink(fsrc);
			snprintf(cmd, sizeof(cmd) - 1, "mv %s %s", fdst, fsrc);
			system(cmd);
			system("sync");
		} else {
			zlog_notice("%s : No fw header for fpga fw image %s.",
					__func__, RDL_PKG_HEADER.ih_image3_str);
		}
#endif
	} else {
#ifdef DEBUG
		zlog_notice("%s : No os3 file ? %s.", __func__, RDL_PKG_HEADER.ih_image3_str);
#endif
	}
#endif /* [#105] */

	result = 0;

__return__:
	system("sync");

	return result;
}

int rdl_install_package(int bno)
{
#if 1 /* [#105] Fixing for RDL install/activation process, dustin, 2024-08-27 */
	char cmd[200];
	char tbuf[100];
	int ret = 0;

	// clear target bank.
	snprintf(cmd, sizeof(cmd) - 1, "cd %s; rm *; rm .*", 
		(bno == RDL_BANK_1) ? RDL_INSTALL1_PATH : RDL_INSTALL2_PATH);
	ret = system(cmd);
	if(ret < 0) {
		zlog_notice("%s : command failed [%s]. reason[%s].",
			__func__, cmd, strerror(errno));
		return ret;
	}

	// move .pkg_info file to target bank.
	if(syscmd_file_exist(RDL_PKG_INFO_FILE)) {
		snprintf(cmd, sizeof(cmd) - 1, "mv %s %s", 
			RDL_PKG_INFO_FILE, 
			(bno == RDL_BANK_1) ? RDL_INSTALL1_PATH : RDL_INSTALL2_PATH);
		ret = system(cmd);
		if(ret < 0) {
			zlog_notice("%s : command failed [%s]. reason[%s].",
					__func__, cmd, strerror(errno));
			return ret;
		}
	}

	// move bp os1 to target bank.
	snprintf(tbuf, sizeof(tbuf) - 1, "%s%s", RDL_IMG_PATH, RDL_OS_HEADER.fih_name);
	if(strlen(RDL_OS_HEADER.fih_name) && syscmd_file_exist(tbuf)) {
		snprintf(cmd, sizeof(cmd) - 1, "mv %s %s", tbuf,
			(bno == RDL_BANK_1) ? RDL_INSTALL1_PATH : RDL_INSTALL2_PATH);
		ret = system(cmd);
		if(ret < 0) {
			zlog_notice("%s : command failed [%s]. reason[%s].",
					__func__, cmd, strerror(errno));
			return ret;
		}
	}

	// move bp os2 to target bank, if os2 is present
	snprintf(tbuf, sizeof(tbuf) - 1, "%s%s", RDL_IMG_PATH, RDL_OS2_HEADER.fih_name);
	if(strlen(RDL_OS2_HEADER.fih_name) && syscmd_file_exist(tbuf)) {
		// move bp os2 to target bank.
		snprintf(cmd, sizeof(cmd) - 1, "mv %s %s", tbuf,
			(bno == RDL_BANK_1) ? RDL_INSTALL1_PATH : RDL_INSTALL2_PATH);
		ret = system(cmd);
		if(ret < 0) {
			zlog_notice("%s : command failed [%s]. reason[%s].",
					__func__, cmd, strerror(errno));
			return ret;
		}
	}

	return ret;
#else /***********************************************************/
	char cmd[200];
	char tbuf[100];

	// move BP OS img file to target bank.
	// no need to move FPGA F/W, just open/write to a FPGA flash device.

	// clear target bank.
	snprintf(cmd, sizeof(cmd) - 1, "cd %s; rm *; rm .*", 
		(bno == RDL_BANK_1) ? RDL_B1_PATH : RDL_B2_PATH);
	system(cmd);

	// move .pkg_info file to target bank.
	if(syscmd_file_exist(RDL_PKG_INFO_FILE)) {
		snprintf(cmd, sizeof(cmd) - 1, "mv %s %s", 
			RDL_PKG_INFO_FILE, (bno == RDL_BANK_1) ? RDL_B1_PATH : RDL_B2_PATH);
		system(cmd);
	}

	// move bp os1 to target bank.
	snprintf(tbuf, sizeof(tbuf) - 1, "%s%s", RDL_IMG_PATH, RDL_OS_HEADER.fih_name);
	if(strlen(RDL_OS_HEADER.fih_name) && syscmd_file_exist(tbuf)) {
		snprintf(cmd, sizeof(cmd) - 1, "mv %s %s", tbuf,
			(bno == RDL_BANK_1) ? RDL_B1_PATH : RDL_B2_PATH);
		system(cmd);
	}

	// move bp os2 to target bank, if os2 is present
	snprintf(tbuf, sizeof(tbuf) - 1, "%s%s", RDL_IMG_PATH, RDL_OS2_HEADER.fih_name);
	if(strlen(RDL_OS2_HEADER.fih_name) && syscmd_file_exist(tbuf)) {
		// move bp os2 to target bank.
		snprintf(cmd, sizeof(cmd) - 1, "mv %s %s", tbuf,
			(bno == RDL_BANK_1) ? RDL_B1_PATH : RDL_B2_PATH);
		system(cmd);
	}

#if 0 /* [#89] Fixing for RDL changes on Target system, dustin, 2024-08-02 */
	// move fpga fw to target bank. together with os1.
	snprintf(tbuf, sizeof(tbuf) - 1, "%s%s", RDL_IMG_PATH, RDL_FW_HEADER.fih_name);
	if(strlen(RDL_FW_HEADER.fih_name) && syscmd_file_exist(tbuf)) {
		snprintf(cmd, sizeof(cmd) - 1, "mv %s %s", tbuf,
			(bno == RDL_BANK_1) ? RDL_B1_PATH : RDL_B2_PATH);
		system(cmd);
	}
#endif

#if 1 /* [#89] Fixing for RDL changes on Target system, dustin, 2024-08-02 */
	/* create new link. */
	if(symlink(RDL_OS_HEADER.fih_name,
		(bno == RDL_BANK_1) ? RDL_B1_LINK_PATH : RDL_B2_LINK_PATH) != 0) {
		sprintf(cmd, "cd %s; ln -s %s uImage",
			(bno == RDL_BANK_1) ? RDL_B1_PATH : RDL_B2_PATH, RDL_OS_HEADER.fih_name);
		if(system(cmd) != 0) {
			zlog_notice("%s : Linking %s as uImage has failed.",
				__func__, RDL_OS_HEADER.fih_name);
			return -1;
		}
	}

	system("sync");
#endif

	return 0;
#endif /* [#105] */
}

#if 1 /* [#96] Adding option bit after downloading FPGA, dustin, 2024-08-19 */
int write_fpga_option_bits(void)
{
	char *dev_file="/dev/mtd3";
	struct mtd_info_user mtd;
	struct erase_info_user erase;
	off_t offset;
	u_int8_t buf[20];	
	u_int8_t buf_1[1];	
	u_int8_t opt_bits[] = { 
		0x0, 0x0, 0xf0, 0x0, 0x0, 0x2, 
		0xf0, 0x2, 0x0, 0x4, 0xf0, 0x4};	
	int fd, ret;

	fd = open(dev_file, O_SYNC | O_RDWR);
	if(fd < 0) {
		zlog_notice("%s : Cannot open %s. reason[%s].\n", 
			__func__, strerror(errno));
		ret = -1;
		goto __error_return__;
	}

	if(ioctl(fd, MEMGETINFO, &mtd) < 0) {
		zlog_notice("%s : Cannot read %s. reason[%s].\n", 
			__func__, strerror(errno));
		ret = -1;
		goto __error_return__;
	}

	zlog_notice("%s : mtd.size [%d] mtd.erasesize [%x].\n", 
		__func__, mtd.size, mtd.erasesize);

	/** check option bit**/
	lseek(fd, 0xe000, SEEK_SET);
	read(fd, buf, 12);  

	if(! memcmp(buf, opt_bits, 12)) {
		zlog_notice("%s : Option bits are same.", __func__);
	}
	else {
		offset = lseek(fd, 0x0, SEEK_SET);
		zlog_notice("%s : Rewriting different Option bits. offset[%x].", 
			__func__, offset);

		erase.start = 0x10000;	
		erase.length = mtd.erasesize;
		if(ioctl(fd, MEMERASE, &erase) < 0) {
			zlog_notice("%s : Cannot read %s. reason[%s].\n",
				__func__, strerror(errno));
			ret = -1;
			goto __error_return__;
		}
		lseek(fd, 0xe000, SEEK_SET);
		write(fd, opt_bits, 12);  
		lseek(fd, 0xe080, SEEK_SET);
		buf_1[0] = 0x4;
		write(fd, buf_1, 1);  
	}

__error_return__:
	if(fd >= 0)
		close(fd);
	return 0;
}
#endif

#if 1/*[#106] init ¿ FPGA update ¿¿ ¿¿, balkrow, 2024-08-28 */
int install_fpga_image(uint8_t bno, const char *img_path)
{
	int fd = -1;
	int os_fd = -1;
	int ret = -1, item;
	uint32_t len, wcnt;
	unsigned char sbuf[RDL_BUFF_SIZE] = {0, }, dbuf[RDL_BUFF_SIZE] = {0, };
	struct stat fs;
	size_t size, written;

	/* erase before copying. */
	if(fpga_erase_all(bno) < 0) {
		zlog_err("%s : Cannot erase mtd device %s.", 
			 __func__, (bno == RDL_BANK_1) ? RDL_DEV1_FPGA_FW : RDL_DEV2_FPGA_FW);
		goto __failed__;   
	}

	/* open src file. */
	fd = open(img_path, O_RDONLY);
	if(fd < 0) {
		zlog_err("%s : Cannot open fpag fw file %s. reason[%s].", 
			 __func__, img_path, strerror(errno));
		goto __failed__;
	}

	if(fstat(fd, &fs) < 0) {
		zlog_err("%s : Cannot fstat fpag fw file %s. reason[%s].", 
			 __func__, img_path, strerror(errno));
	}

	/* open device for fpga os. */
	os_fd = open((bno == RDL_BANK_1) ? 
		     RDL_DEV1_FPGA_FW : RDL_DEV2_FPGA_FW, O_SYNC | O_RDWR);
	if(os_fd < 0) {
		zlog_err("%s : Cannot open fpag fw device %s. reason[%s].", 
			 __func__, (bno == RDL_BANK_1) ? RDL_DEV1_FPGA_FW : RDL_DEV2_FPGA_FW, 
			 strerror(errno));
		goto __failed__;
	}

	/* read fpga fw file, write to device. */
	size = fs.st_size;
	written = 0;
	item = RDL_BUFF_SIZE;
	while(size) {
		if(size < RDL_BUFF_SIZE) item = size;

		len = read(fd, sbuf, item);
		if(len > 0) {
			wcnt = write(os_fd, sbuf, len);
			if(len != wcnt) {
				zlog_notice("%s : read %u and written %u : fpag fw failed.",
					    __func__, len, wcnt);
				goto __failed__;
			}
		}

		written += item;
		size -= item;
	}

	/* verify file with flash */
	lseek(fd,    0L, SEEK_SET);
	lseek(os_fd, 0L, SEEK_SET);

	size = fs.st_size;
	written = 0;
	item = RDL_BUFF_SIZE;
	while(size) {
		if(size < RDL_BUFF_SIZE) item = size;

		len  = read(fd,    sbuf, item);
		wcnt = read(os_fd, dbuf, item);

		if((len != wcnt) || memcmp(sbuf, dbuf, len)) {
			zlog_err("%s : Different size[%u/%u] or content. [%u].", 
				 __func__, len, wcnt, written);
			goto __failed__;
		}

		written += item;
		size -= item;
	}

	zlog_notice("%s : Flash writting %u bytes verified.", 
		    __func__, written);

	close(fd);
	close(os_fd);

#if 1 /* [#96] Adding option bit after downloading FPGA, dustin, 2024-08-19 */
	write_fpga_option_bits();
#endif

	ret = 0;

__failed__:
	if(fd > 0)
		close(fd);
	if(os_fd > 0)
		close(os_fd);

	return ret;
}
#endif

int rdl_activate_fpga(uint8_t bno)
{
#if 1/* [#76] Adding for processing FPGA F/W, dustin, 2024-07-15 */
#if 1 /* [#89] Fixing for RDL changes on Target system, dustin, 2024-08-02 */
#if 0 /* [#105] Fixing for RDL install/activation process, dustin, 2024-08-27 */
    fw_header_t bh;
#endif
	int fd = -1;
	int os_fd = -1;
	int ret = -1, item;
	uint32_t len, wcnt;
    char tbuf[100];
#if 1 /* [#105] Fixing for RDL install/activation process, dustin, 2024-08-27 */
    char cmd[200];
#endif
	unsigned char sbuf[RDL_BUFF_SIZE], dbuf[RDL_BUFF_SIZE];
	struct stat fs;
	size_t size, written;

#if 1 /* [#105] Fixing for RDL install/activation process, dustin, 2024-08-27 */
#if 1 /* [#105] Fixing for RDL install/activation process, dustin, 2024-08-27 */
	/* NOTE : Do not unnecessary fpga copying process. */
    snprintf(tbuf, sizeof(tbuf) - 1, "%s%s",
		(bno == RDL_BANK_1) ? RDL_INSTALL1_PATH : RDL_INSTALL2_PATH, 
		RDL_PKG_HEADER.ih_image2_str);
#else
    snprintf(tbuf, sizeof(tbuf) - 1, "%s%s",
		(bno == RDL_BANK_1) ? RDL_B1_PATH : RDL_B2_PATH, 
		RDL_PKG_HEADER.ih_image2_str);

	/* copy files from /mnt/flash/bankX to /media/bankX. */
	snprintf(cmd, sizeof(cmd) - 1, "cp %s%s %s", (bno == RDL_BANK_1) ?
		RDL_INSTALL1_PATH : RDL_INSTALL2_PATH, 
		RDL_PKG_HEADER.ih_image2_str, tbuf);
	ret = system(cmd);
	if(ret < 0) {
		zlog_notice("%s : command failed [%s]. reason[%s].",
			__func__, cmd, strerror(errno));
		goto __failed__;
	}

#if 1 /* [#105] Fixing for RDL install/activation process, dustin, 2024-08-27 */
	system("sync");
#endif
#endif /* [#105] */

	if(strlen(RDL_PKG_HEADER.ih_image2_str) && syscmd_file_exist(tbuf))
#else /************************************************************/
    /* read pkg header info. */
    get_pkg_fwheader_info((bno == RDL_BANK_1) ?
    	RDL_B1_PKG_INFO_FILE : RDL_B2_PKG_INFO_FILE, &bh);
    
    snprintf(tbuf, sizeof(tbuf) - 1, "%s%s",
		(bno == RDL_BANK_1) ? RDL_B1_PATH : RDL_B2_PATH, bh.ih_image2_str);

	if(strlen(bh.ih_image2_str) && syscmd_file_exist(tbuf))
#endif
	{
		/* FIXME : check fpga os version and new version ? how ?? */

		/* erase before copying. */
		if(fpga_erase_all(bno) < 0) {
			zlog_err("%s : Cannot erase mtd device %s.", 
				__func__, tbuf);
			goto __failed__;
		}

		/* open src file. */
		fd = open(tbuf, O_RDONLY);
		if(fd < 0) {
			zlog_err("%s : Cannot open fpag fw file %s. reason[%s].", 
				__func__, tbuf, strerror(errno));
			goto __failed__;
		}

		if(fstat(fd, &fs) < 0) {
			zlog_err("%s : Cannot fstat fpag fw file %s. reason[%s].", 
				__func__, tbuf, strerror(errno));
		}

		/* open device for fpga os. */
		os_fd = open((bno == RDL_BANK_1) ? 
			RDL_DEV1_FPGA_FW : RDL_DEV2_FPGA_FW, O_SYNC | O_RDWR);
		if(os_fd < 0) {
			zlog_err("%s : Cannot open fpag fw device %s. reason[%s].", 
				__func__, (bno == RDL_BANK_1) ? RDL_DEV1_FPGA_FW : RDL_DEV2_FPGA_FW, 
				strerror(errno));
			goto __failed__;
		}

		/* read fpga fw file, write to device. */
		size = fs.st_size;
		written = 0;
		item = RDL_BUFF_SIZE;
		while(size) {
			if(size < RDL_BUFF_SIZE) item = size;

			len = read(fd, sbuf, item);
			if(len > 0) {
				wcnt = write(os_fd, sbuf, len);
				if(len != wcnt) {
					zlog_notice("%s : read %u and written %u : fpag fw failed.",
						__func__, len, wcnt);
					goto __failed__;
				}
			}

			written += item;
			size -= item;
		}

		/* verify file with flash */
		lseek(fd,    0L, SEEK_SET);
		lseek(os_fd, 0L, SEEK_SET);

		size = fs.st_size;
		written = 0;
		item = RDL_BUFF_SIZE;
		while(size) {
			if(size < RDL_BUFF_SIZE) item = size;

			len  = read(fd,    sbuf, item);
			wcnt = read(os_fd, dbuf, item);

			if((len != wcnt) || memcmp(sbuf, dbuf, len)) {
				zlog_err("%s : Different size[%u/%u] or content. [%u].", 
					__func__, len, wcnt, written);
				goto __failed__;
			}

			written += item;
			size -= item;
		}

		zlog_notice("%s : Flash writting %u bytes verified.", 
			__func__, written);

		close(fd);
		fd = -1;
		close(os_fd);
		os_fd = -1;
	}

#if 1 /* [#96] Adding option bit after downloading FPGA, dustin, 2024-08-19 */
	write_fpga_option_bits();
#endif

#if 0 /* [#93] Adding for FPGA FW Bank Select and Error handling, dustin, 2024-08-12 */
	/* NOTE : updating here is useless. */
	/* update working bank. */
	gRegUpdate(FW_BANK_SELECT_ADDR, 8, 0x70, bno);
#endif

	/* set bank flag for fpga fw */
	set_fpga_fw_active_bank_flag(bno);

	ret = 0;

__failed__:
	if(fd > 0)
		close(fd);
	if(os_fd > 0)
		close(os_fd);
	return ret;
#else /*********************************************************************/
    fw_header_t bh;
	FILE *fp = NULL;
	FILE *os_fp = NULL;
	FILE *fw_fp = NULL;
	uint32_t len, wcnt;
	int ret = -1;
    char tbuf[100];

    // read pkg header info.
    get_pkg_fwheader_info((bno == RDL_BANK_1) ?
    	RDL_B1_PKG_INFO_FILE : RDL_B2_PKG_INFO_FILE, &bh);
    
    snprintf(tbuf, sizeof(tbuf) - 1, "%s%s",
		(bno == RDL_BANK_1) ? RDL_B1_PATH : RDL_B2_PATH, bh.ih_image2_str);

	if(strlen(bh.ih_image2_str) && syscmd_file_exist(tbuf)) {
		// FIXME : check fpga os version and new version ? how ??

		// open src file.
		fp = fopen(tbuf, "r");
		if(fp == NULL) {
			zlog_err("%s : Cannot open fpag fw file %s. reason[%s].", 
				__func__, tbuf, strerror(errno));
			goto __failed__;
		}

		// open device for fpga os.
		os_fp = fopen(RDL_DEV_FPGA_OS, "w");
		if(os_fp == NULL) {
			zlog_err("%s : Cannot open fpag os device %s. reason[%s].", 
				__func__, RDL_DEV_FPGA_OS, strerror(errno));
			goto __failed__;
		}

		// read fpga fw file, write to device.
		while(! feof(fp)) {
			len = fread(RDL_PAGE, RDL_BUFF_SIZE, 1, fp);
			if(len > 0) {
				wcnt = fwrite(RDL_PAGE, RDL_BUFF_SIZE, len, os_fp);
				if(len != wcnt) {
					zlog_notice("%s : read %u and written %u : fpag fw failed.",
						__func__, len, wcnt);
					goto __failed__;
				}
			}
		}

		fclose(fp);
		fp = NULL;
		fclose(os_fp);
		os_fp = NULL;
	}

    snprintf(tbuf, sizeof(tbuf) - 1, "%s%s",
		(bno == RDL_BANK_1) ? RDL_B1_PATH : RDL_B2_PATH, bh.ih_image3_str);

	if(strlen(bh.ih_image3_str) && syscmd_file_exist(tbuf)) {
		// FIXME : check fpga fw version and new version ? how ??

		// open src file.
		fp = fopen(tbuf, "r");
		if(fp == NULL) {
			zlog_err("%s : Cannot open fpag fw file %s. reason[%s].", 
				__func__, tbuf, strerror(errno));
			return -1;
		}

		// open device for fpga fw.
		fw_fp = fopen(RDL_DEV_FPGA_FW, "w");
		if(fw_fp == NULL) {
			zlog_err("%s : Cannot open fpag fw device %s. reason[%s].", 
				__func__, RDL_DEV_FPGA_FW, strerror(errno));
			return -1;
		}

		// read fpga os file, write to device.
		while(! feof(fp)) {
			// read from file.
			len = fread(RDL_PAGE, RDL_BUFF_SIZE, 1, fp);
			if(len > 0) {
				// write to device.
				wcnt = fwrite(RDL_PAGE, RDL_BUFF_SIZE, len, fw_fp);
				if(len != wcnt) {
					zlog_notice("%s : read %u and written %u : fpag fw failed.",
						__func__, len, wcnt);
					goto __failed__;
				}
			}
		}

		fclose(fp);
		fp = NULL;
		fclose(fw_fp);
		fw_fp = NULL;
	}

	ret = 0;

__failed__:
	if(fp)
		fclose(fp);
	if(os_fp)
		fclose(os_fp);
	if(fw_fp)
		fclose(fw_fp);
	return ret;
#endif
#else
	return 0;
#endif
}

// just overwrite, don't care version, different file.
int rdl_activate_bp(int bno)
{
#if 1 /* [#89] Fixing for RDL changes on Target system, dustin, 2024-08-02 */
#if 1 /* [#105] Fixing for RDL install/activation process, dustin, 2024-08-27 */
	char cmd[200];
	int ret = 0;

	/* get pkg info */
	get_pkg_fwheader_info((bno == RDL_BANK_1) ? 
		RDL_B1_PKG_INFO_FILE : RDL_B2_PKG_INFO_FILE, &RDL_PKG_HEADER);

	/* remove target bank. */
	snprintf(cmd, sizeof(cmd) - 1, "rm %s*; rm %s.*",
		(bno == RDL_BANK_1) ? RDL_B1_PATH : RDL_B2_PATH);
	ret = system(cmd);
	if(ret < 0) {
		zlog_notice("%s : command failed [%s]. reason[%s].",
			__func__, cmd, strerror(errno));
		return ret;
	}

	/* copy .pkg_info from /mnt/flash/bankX to /media/bankX. */
	snprintf(cmd, sizeof(cmd) - 1, "cp %s %s", 
		(bno == RDL_BANK_1) ? 
			RDL_INSTALL1_PKG_INFO_FILE : RDL_INSTALL2_PKG_INFO_FILE,
		(bno == RDL_BANK_1) ? RDL_B1_PKG_INFO_FILE : RDL_B2_PKG_INFO_FILE);
	ret = system(cmd);

	if(ret < 0) {
		zlog_notice("%s : command failed [%s]. reason[%s].",
			__func__, cmd, strerror(errno));
		return ret;
	}

#if 1 /* [#105] Fixing for RDL install/activation process, dustin, 2024-08-27 */
	system("sync");
#endif

	/* copy os from /mnt/flash/bankX to /media/bankX. */
	snprintf(cmd, sizeof(cmd) - 1, "cp %s%s %s", 
		(bno == RDL_BANK_1) ? RDL_INSTALL1_PATH : RDL_INSTALL2_PATH,
		RDL_PKG_HEADER.ih_image1_str, 
		(bno == RDL_BANK_1) ? RDL_B1_PATH : RDL_B2_PATH);
	ret = system(cmd);
	if(ret < 0) {
		zlog_notice("%s : command failed [%s]. reason[%s].",
			__func__, cmd, strerror(errno));
		return ret;
	}

#if 1 /* [#105] Fixing for RDL install/activation process, dustin, 2024-08-27 */
	system("sync");
#endif

	/* create new link. */
	ret = symlink(RDL_PKG_HEADER.ih_image1_str,
		(bno == RDL_BANK_1) ? RDL_B1_LINK_PATH : RDL_B2_LINK_PATH);
	if(ret < 0) {
		zlog_notice("%s : Linking %s as uImage has failed.",
			__func__, RDL_PKG_HEADER.ih_image1_str);
		return ret;
	}

	system("sync");

	/* update bank env variable for next loading. */
	sprintf(cmd, "fw_setenv bank %d", bno);
	system(cmd);

	return 0;
#else /************************************************************/
	char cmd[100];

	/* update bank env variable for next loading. */
	sprintf(cmd, "fw_setenv bank %d", bno);
	system(cmd);

	/* get pkg info */
	get_pkg_fwheader_info((bno == RDL_BANK_1) ? 
		RDL_B1_PKG_INFO_FILE : RDL_B2_PKG_INFO_FILE, &RDL_PKG_HEADER);

	/* BP update OS related registers. */
	rdl_update_bank_registers(bno);

	/*FIXME : auto rebooting ? */

	return 0;
#endif /* [#105] */
#else
	fw_header_t bh;
	char cmd[200], fname[100];
	int ret;

	// read pkg header info.
	get_pkg_fwheader_info((bno == RDL_BANK_1) ? 
		RDL_B1_PKG_INFO_FILE : RDL_B2_PKG_INFO_FILE, &bh);

	// copy target bank os file to boot path.
	snprintf(cmd, sizeof(cmd) - 1, "cp %s%s %s", 
		(bno == RDL_BANK_1) ? RDL_B1_PATH : RDL_B2_PATH,
		bh.ih_image1_str, RDL_BOOT_PATH);
	ret = system(cmd);
	if(ret != 0) {
		zlog_notice("%s : Copying %s%s to %s has failed.",
			__func__, (bno == RDL_BANK_1) ? RDL_B1_PATH : RDL_B2_PATH,
			 bh.ih_image1_str, RDL_BOOT_PATH);
		//FIXME : handle error case.
		return -1;
	}

	// read link to get current os file name.
	if(readlink(RDL_BOOT_LINK_PATH, fname, sizeof(fname) - 1) < 0) {
		zlog_notice("%s : Cannot read linked file name.", __func__);
	} else {
		// remove current os file.
		snprintf(cmd, sizeof(cmd) - 1, "%s%s", RDL_BOOT_PATH, fname);
		unlink(cmd);
	}

	// remove img link.
	unlink(RDL_BOOT_LINK_PATH);

	// create new link.
	if(symlink(bh.ih_image1_str, RDL_BOOT_LINK_PATH) != 0) {
		zlog_notice("%s : Linking %s as %s has failed.",
			__func__, bh.ih_image1_str, RDL_BOOT_LINK_PATH);
		//FIXME : handle error case.
		return -1;
	}

	system("sync");

	// update active bank flag.
	set_sw_active_bank_flag(bno);

	return 0;
#endif
}

RDL_EVT_t rdl_get_evt(RDL_ST_t state)
{
	RDL_EVT_t evt = EVT_RDL_EVT_MAX;
	uint16_t hw_val, sts, pno;

	hw_val = DPRAM_READ(RDL_STATE_REQ_ADDR);
	sts    = (hw_val & 0xFF00) >> 8;
	pno    = (hw_val & 0xFF);
//zlog_notice("---> %s : read val[0x%x] sts[0x%x] pno[%u].", __func__, hw_val, sts, pno);//ZZPP

	switch(state)	
	{
		case	ST_RDL_IDLE:
			if(sts == RDL_START_BIT)
				evt = EVT_RDL_START;
			break;

		case	ST_RDL_START:
#if 1 /* [#105] Fixing for RDL install/activation process, dustin, 2024-08-27 */
			if(sts == RDL_START_BIT)
				evt = EVT_RDL_START;
			else
#endif
			if(sts == RDL_P1_WRITING_BIT)
				evt = EVT_RDL_WRITING_P1;
			break;

		case	ST_RDL_WRITING_P1:
#if 1 /* [#105] Fixing for RDL install/activation process, dustin, 2024-08-27 */
			if(sts == RDL_START_BIT)
				evt = EVT_RDL_START;
			else
#endif
			if(sts == RDL_P1_WRITING_DONE_BIT)
				evt = EVT_RDL_WRITING_DONE_P1;
			else if(sts == RDL_PAGE_WRITING_ERROR_BIT)
				evt = EVT_RDL_WRITING_ERROR;
			break;

		case	ST_RDL_READING_P1:
#if 1 /* [#105] Fixing for RDL install/activation process, dustin, 2024-08-27 */
			if(sts == RDL_START_BIT)
				evt = EVT_RDL_START;
			else
#endif
			if(sts == RDL_P2_WRITING_BIT)
				evt = EVT_RDL_WRITING_P2;
			break;

		case	ST_RDL_WRITING_P2:
#if 1 /* [#105] Fixing for RDL install/activation process, dustin, 2024-08-27 */
			if(sts == RDL_START_BIT)
				evt = EVT_RDL_START;
			else
#endif
			if(sts == RDL_P2_WRITING_DONE_BIT)
				evt = EVT_RDL_WRITING_DONE_P2;
			else if(sts == RDL_PAGE_WRITING_ERROR_BIT)
				evt = EVT_RDL_WRITING_ERROR;
			break;

		case	ST_RDL_READING_P2:
#if 1 /* [#105] Fixing for RDL install/activation process, dustin, 2024-08-27 */
			if(sts == RDL_START_BIT)
				evt = EVT_RDL_START;
			else
#endif
			if(PAGE_CRC_OK)
				evt = EVT_RDL_READING_DONE_P2;
			else
				evt = EVT_RDL_READING_ERROR;
			break;

		case	ST_RDL_WRITING_TOTAL:
#if 1 /* [#105] Fixing for RDL install/activation process, dustin, 2024-08-27 */
			if(sts == RDL_START_BIT)
				evt = EVT_RDL_START;
			else
#endif
			if(sts == RDL_TOTAL_WRITING_DONE_BIT)
				evt = EVT_RDL_WRITING_DONE_TOTAL;
			else if(sts == RDL_P1_WRITING_BIT)
				evt = EVT_RDL_WRITING_NOT_DONE;
			else if(sts == RDL_TOTAL_WRITING_ERROR_BIT)
				evt = EVT_RDL_WRITING_ERROR_TOTAL;
			break;

		case	ST_RDL_READING_TOTAL:
			// check crc for total integrated img file.
#if 1 /* [#105] Fixing for RDL install/activation process, dustin, 2024-08-27 */
			if(sts == RDL_START_BIT)
				evt = EVT_RDL_START;
			else
#endif
			if(rdl_check_total_crc(RDL_INFO.hd.file_name) == RDL_CRC_OK)
				evt = EVT_RDL_READING_DONE_TOTAL;
			else
				evt = EVT_RDL_READING_ERROR_TOTAL;
			break;

#if 1 /* [#105] Fixing for RDL install/activation process, dustin, 2024-08-27 */
		case	ST_RDL_INSTALL_DONE:
#else
		case	ST_RDL_ACTIVATE_DONE:
#endif
#if 1 /* [#105] Fixing for RDL install/activation process, dustin, 2024-08-27 */
#if 1 /* [#105] Fixing for RDL install/activation process, dustin, 2024-08-27 */
			if(sts == RDL_START_BIT)
				evt = EVT_RDL_START;
			else
#endif
			if(RDL_INSTALL_STATE >= 0)
				evt = EVT_RDL_IMG_INSTALL_SUCCESS;
			else 
				evt = EVT_RDL_IMG_INSTALL_FAIL;
			break;
#else /*********************************************************/
#if 1 /* [#89] Fixing for RDL changes on Target system, dustin, 2024-08-02 */
			if(RDL_ACTIVATION_STATE >= 0)
				evt = EVT_RDL_IMG_ACTIVE_SUCCESS;
			else 
				evt = EVT_RDL_IMG_ACTIVE_FAIL;
#else
			if(1/*FIXME : if activation is ok */)
				evt = EVT_RDL_IMG_ACTIVE_SUCCESS;
			else 
				evt = EVT_RDL_IMG_ACTIVE_FAIL;
#endif
			break;

		case	ST_RDL_RUNNING_CHECK:
			if(1/*FIXME : if new img booting was ok */)
				evt = EVT_RDL_IMG_RUNNING_SUCCESS;
			else
				evt = EVT_RDL_IMG_RUNNING_FAIL;
			break;
#endif /* [#105] */
#if 1/*[#110] RDL function Debugging ¿ ¿¿, balkrow, 2024-09-03*/ 
		case	ST_RDL_BANK_MOVE_WAIT:
			break;
#endif

		case	ST_RDL_TERM:
			evt = EVT_RDL_NONE;
			break;
	}
	return evt;
}

RDL_ST_t rdl_update_fsm(void)
{
	RDL_ST_t st = rdl_info_list.st;
	RDL_EVT_t evt;
	uint8_t offset;
	uint8_t n, ret = 0;
	
	if(st == ST_RDL_TERM)
		st = ST_RDL_IDLE;

	while (st != ST_RDL_TERM)
	{
		evt = rdl_get_evt(st);
		if(evt == EVT_RDL_EVT_MAX)
			goto next_turn;
#ifdef DEBUG
//		zlog_notice("%s : new evt[%s].", 
//			__func__, rdl_get_event_str(evt));//ZZPP
#endif
		
		for(n = 0; n < RDL_TRANS_MAX; n++)
		{
			if((rdl_fsm_list[n].state == st) && (rdl_fsm_list[n].evt == evt))
			{
				st = rdl_fsm_list[n].func();
#ifdef DEBUG
				zlog_notice("%s : new state[%s].", 
					__func__, rdl_get_state_str(st));//ZZPP
#endif
				goto next_turn;
			}
		}

		if(st == rdl_info_list.st)
			break;
	}
next_turn :
	return st;

}

int rdl_fsm_func(struct thread *thread)
{
	rdl_update_fsm();

#if 1 /* [#105] Fixing for RDL install/activation process, dustin, 2024-08-27 */
	thread_add_timer_msec (master, rdl_fsm_func, NULL, 50);
#else
	thread_add_timer_msec (master, rdl_fsm_func, NULL, 100);
#endif
	return 0;
}

#if 1/* [#77] Adding RDL emulation function, dustin, 2024-07-16 */
extern RDL_ST_t emul_trigger(void);

extern RDL_INFO_LIST_t emul_info_list;
extern RDL_FSM_t emul_fsm_list[];
extern int EMUL_TRANS_MAX;
extern int EMUL_TRIGGERED;

RDL_EVT_t emul_get_evt(RDL_ST_t state)
{
	RDL_EVT_t evt = EVT_RDL_EVT_MAX;
	uint16_t hw_val, sts, pno;

	hw_val = DPRAM_READ(RDL_STATE_RESP_ADDR);
	sts    = (hw_val & 0xFF00) >> 8;
	pno    = (hw_val & 0xFF);
//zlog_notice("---> %s : read val[0x%x] sts[0x%x] pno[%u].", __func__, hw_val, sts, pno);//ZZPP

	switch(state)	
	{
		case	ST_RDL_IDLE:
			if(EMUL_TRIGGERED)
				evt = EVT_RDL_TRIGGER;
			break;

		case	ST_RDL_TRIGGER:
			if(sts == RDL_START_ACK_BIT)
				evt = EVT_RDL_START;
			break;

		case	ST_RDL_START:
			if(sts == RDL_P1_WRITING_ACK_BIT)
				evt = EVT_RDL_WRITING_P1;
			break;

		case	ST_RDL_WRITING_P1:
			if(sts == RDL_P1_WRITING_DONE_ACK_BIT)
				evt = EVT_RDL_WRITING_DONE_P1;
			else if(sts == RDL_PAGE_WRITING_ERROR_ACK_BIT)
				evt = EVT_RDL_WRITING_ERROR;
			break;

		case	ST_RDL_READING_P1:
			if(sts == RDL_P2_WRITING_ACK_BIT)
				evt = EVT_RDL_WRITING_P2;
			break;

		case	ST_RDL_WRITING_P2:
			if(sts == RDL_P2_WRITING_DONE_ACK_BIT)
				evt = EVT_RDL_WRITING_DONE_P2;
			else if(sts == RDL_PAGE_WRITING_ERROR_ACK_BIT)
				evt = EVT_RDL_WRITING_ERROR;
			break;

		case	ST_RDL_READING_P2:
			if(sts == RDL_PAGE_READING_DONE_BIT)
				evt = EVT_RDL_READING_DONE_P2;
			else if(sts == RDL_PAGE_READING_ERROR_BIT)
				evt = EVT_RDL_READING_ERROR;
			break;

		case	ST_RDL_WRITING_TOTAL:
			if(sts == RDL_TOTAL_READING_DONE_BIT)
				evt = EVT_RDL_WRITING_DONE_TOTAL;
			else if(sts == RDL_TOTAL_READING_ERROR_BIT)
				evt = EVT_RDL_READING_ERROR_TOTAL;
			else if(sts == RDL_TOTAL_WRITING_ERROR_ACK_BIT)
				evt = EVT_RDL_WRITING_ERROR_TOTAL;
			break;

		case	ST_RDL_TERM:
			evt = EVT_RDL_NONE;
			break;
	}
	return evt;
}

RDL_ST_t emul_update_fsm(void)
{
	RDL_ST_t st = emul_info_list.st;
	RDL_EVT_t evt;
	uint8_t offset;
	uint8_t n, ret = 0;

	if(st == ST_RDL_TERM)
		st = ST_RDL_IDLE;

	while (st != ST_RDL_ST_MAX)
	{
		evt = emul_get_evt(st);
		if(evt == EVT_RDL_EVT_MAX)
			goto next_turn;
#ifdef DEBUG
//		zlog_notice("---> %s : new evt[%s].", 
//			__func__, rdl_get_event_str(evt));//ZZPP
#endif
		
		for(n = 0; n < EMUL_TRANS_MAX; n++)
		{
			if((emul_fsm_list[n].state == st) && (emul_fsm_list[n].evt == evt))
			{
				st = emul_fsm_list[n].func();
#ifdef DEBUG
//				zlog_notice("---> %s : new state[%s].", 
//					__func__, rdl_get_state_str(st));//ZZPP
#endif
				goto next_turn;
			}
		}

		if(st == emul_info_list.st)
			break;
	}
next_turn :
	return st;

}

int rdl_mcu_emul_func(struct thread *thread)
{
	char cmd[200];

	//////////////////////////////////////////////////////////
	// start MCU RDL emulation
	//////////////////////////////////////////////////////////
	if(EMUL_TRIGGERED) {
		emul_trigger();
		EMUL_TRIGGERED = 0;
	}

	emul_update_fsm();

	thread_add_timer_msec (master, rdl_mcu_emul_func, NULL, 100);
	return 0;
}
#endif
#endif

#if 1/*[#53] Clock source status ¿¿¿¿ ¿¿ ¿¿, balkrow, 2024-06-13*/
int monitor_hw_timer(struct thread *thread)
{
#if 0 /*[#82] eag6l board SW Debugging, balkrow, 2024-07-26*/
	if(gDB.init_state != SYS_INIT_DONE) {
		goto __SKIP_1__;
	}
#endif

	/*PLL state*/
	rsmu_pll_update();

__SKIP_1__:
#if 1/*[#71] EAG6L Board Bring-up, balkrow, 2024-07-16*/
	thread_add_timer_msec (master, monitor_hw_timer, NULL, 300);
#endif
	return RT_OK;
}	
#endif

#if 1/*[#56] register update timer ¿¿, balkrow, 2023-06-13 */
int reg_fast_intv_update(struct thread *thread)
{
extern void pm_request_counters(void);
extern void update_KeepAlive(void);
extern void update_port_sfp_information(void);
extern void process_port_pm_counters(void);

	/*update KeepAlive reg*/
	update_KeepAlive();

#if 1 /*[#82] eag6l board SW Debugging, balkrow, 2024-07-26*/
	if(gDB.sdk_init_state != SDK_INIT_DONE) {
		goto __SKIP_2__;
	}
#else
	if(gDB.init_state != SYS_INIT_DONE) {
		goto __SKIP_2__;
	}
#endif

	pm_request_counters();

	/* read per-port spf port status/control. */
	update_port_sfp_information();

	/* process per-port performance info from sdk. */
	process_port_pm_counters();

__SKIP_2__:
	thread_add_timer_msec (master, reg_fast_intv_update, NULL, 500);

	return RT_OK;
}

int reg_slow_intv_update(struct thread *thread)
{
#if 1/*[#54] Adding Smart T-SFP I2C functions, dustin, 2024-06-13 */
extern void update_bp_reg(void);
extern void update_sfp(void);
extern uint16_t portAlarm(void);

#if 0 /*[#82] eag6l board SW Debugging, balkrow, 2024-07-26*/
	if(gDB.init_state != SYS_INIT_DONE) {
		goto __SKIP_3__;
	}
#endif

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

int monMCUupdate(struct thread *thread)
{
extern void regMonitor(void);

#if 0 /*[#82] eag6l board SW Debugging, balkrow, 2024-07-26*/
	if(gDB.sdk_init_state != SDK_INIT_DONE) {
		goto __SKIP_4__;
	}
#else
	if(gDB.init_state != SYS_INIT_DONE) {
		goto __SKIP_4__;
	}
#endif

	regMonitor();

__SKIP_4__:
	thread_add_timer_msec (master, monMCUupdate, NULL, 100);
	return RT_OK;
}
#endif

#if 1/*[#56] register update timer ¿¿, balkrow, 2023-06-13 */
uint16_t sysmonUpdateGetSWVer(void)
{
#if 0 /* [#99] EAG6L ¿¿ ¿¿ ¿¿ F/U, balkrow, 2024-08-30 */
	FILE *fn = NULL;
	char buf[10], fname[100], *str = NULL;
	int bank = RDL_BANK_1, major, minor;

	/* get current bank. */
	fn = popen("fw_printenv -n bank", "r");
	if(fn == NULL) {
		zlog_notice("%s : Cannot get bank env variable.", __func__);
		return 0x10;
	}

	fread(buf, sizeof(char), 1, fn);
	pclose(fn);

	if(sscanf(buf, "%d", &bank) != 1) {
		zlog_notice("%s : Invalid bank env variable.", __func__);
		return 0x10;
	}

	/* read link to get current os file name. */
	if(readlink((bank == RDL_BANK_1) ? 
		RDL_B1_LINK_PATH : RDL_B2_LINK_PATH, fname, sizeof(fname) - 1) < 0) {
		zlog_notice("%s : Cannot read linked file name.", __func__);
		return 0x10;
	} 

	/* parse major/minor version info. */
	str = strstr(fname, "v");
	if(str) {
		if(sscanf(str, "v%d.%d", &major, &minor) != 2) {
            zlog_notice("%s : Invalid version string [%s].", __func__, str);
			return 0x10;
        }

		return ((major << 4) | minor);
	} else {
		zlog_notice("%s : Cannot find 'v' for version from [%s].",
			__func__, fname);
			return 0x10;
	}

	return 0x10;
#else
	/*TODO: real data*/
	return 0x11;
#endif
}
#endif

#if 1 /* [#97] Adding register recovery process after fpga reset, dustin, 2024-08-21 */
int _CHECK_FPGA_STATUS_ = 0;
uint8_t _FPGA_BNO_;
uint8_t _FPGA_CHANGED_FLAG_ = 0;

int check_fpga_status(void)
{
extern void do_recovery_update_after_fpga_reset(void);

	uint16_t data;
	uint8_t bno;

	if(_CHECK_FPGA_STATUS_) {
		data = CPLD_READ(CPLD_FW_BANK_STATUS_ADDR);
		if(data == CPLD_BANK_OK) {
			zlog_notice("%s : FPGA bank[%d] OK. Do register recovery.", 
				__func__, _FPGA_BNO_);
			/* update/recover some registers. */
			do_recovery_update_after_fpga_reset();

			/* turn off flag and no more thread. */
			_CHECK_FPGA_STATUS_ = 0;
			return 0;
		} else if(data == CPLD_BANK_BAD) {
			if(! _FPGA_CHANGED_FLAG_) {
				/* try standby bank */
				bno = (_FPGA_BNO_ == RDL_BANK_1) ? RDL_BANK_2 : RDL_BANK_1;
				zlog_notice("%s : Try standby FPGA bank [%d].", __func__, bno);
				_FPGA_CHANGED_FLAG_ = 1;
			} else {
				bno = 0;
				zlog_notice("%s : Bank1/2 failed. Try default FPGA bank.", 
					__func__);
				_FPGA_CHANGED_FLAG_ = 0;
			}

			/* set fpga standby bank */
			CPLD_WRITE(CPLD_FW_BANK_SELECT_ADDR, bno);
			_FPGA_BNO_ = bno;

			/* give a time to let mcu detect board CR event. */
			thread_add_timer_msec(master, check_fpga_status, NULL, 500);
			return 0;
		}
	}

	thread_add_timer_msec(master, check_fpga_status, NULL, 100);
	return 0;
}
#endif

/* Allocate new sys structure and set default value. */
void sysmon_thread_init (void)
{
#if 1/*[#26] system managent FSM ¿¿, balkrow, 2024-05-20*/
	thread_add_timer (master, svc_fsm_timer, NULL, 2);
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
#if 1/* [#70] Adding RDL feature, dustin, 2024-07-02 */
	thread_add_timer (master, rdl_fsm_func, NULL, 5);
#endif
#if 1/* [#77] Adding RDL emulation function, dustin, 2024-07-16 */
	thread_add_timer (master, rdl_mcu_emul_func, NULL, 5);
#endif
#if 1 /* [#91] Fixing for register updating feature, dustin, 2024-08-05 */
	thread_add_timer(master, check_fifo_hello, NULL, 10/*sec*/);
#endif
}

#if 1 /*[#82] eag6l board SW Debugging, balkrow, 2024-08-09*/
CARD_SIDE_PORT_NUM getCPortBySport(int16_t port)
{
	CARD_SIDE_PORT_NUM portNum;
	switch(port)
	{
	case PORT_ID_EAG6L_PORT1:
		portNum = C_PORT1;
		break;
	case PORT_ID_EAG6L_PORT2:
		portNum = C_PORT2;
		break;
	case PORT_ID_EAG6L_PORT3:
		portNum = C_PORT3;
		break;
	case PORT_ID_EAG6L_PORT4:
		portNum = C_PORT4;
		break;
	case PORT_ID_EAG6L_PORT5:
		portNum = C_PORT5;
		break;
	case PORT_ID_EAG6L_PORT6:
		portNum = C_PORT6;
		break;
	case PORT_ID_EAG6L_PORT7:
		portNum = C_PORT7;
		break;
	default:
		portNum = NOT_DEFINED;
		break;
	}
	return portNum;
}
#endif

#if 1/*[#80] eag6l board SW bring-up, balkrow, 2023-07-25 */
CARD_SIDE_PORT_NUM getCPortByMport(MCU_SIDE_PORT_NUM port)
{
	CARD_SIDE_PORT_NUM portNum;
	switch(port)
	{
	case M_PORT1:
		portNum = C_PORT1;
		break;
	case M_PORT2:
		portNum = C_PORT2;
		break;
	case M_PORT3:
		portNum = C_PORT3;
		break;
	case M_PORT4:
		portNum = C_PORT4;
		break;
	case M_PORT5:
		portNum = C_PORT5;
		break;
	case M_PORT6:
		portNum = C_PORT6;
		break;
	case M_PORT7:
		portNum = C_PORT7;
		break;
	default:
		portNum = NOT_DEFINED;
		break;
	}
	return portNum;
}

MCU_SIDE_PORT_NUM getMPortByCport(CARD_SIDE_PORT_NUM port)
{
	MCU_SIDE_PORT_NUM portNum;
	switch(port)
	{
	case C_PORT1:
		portNum = M_PORT1;
		break;
	case C_PORT2:
		portNum = M_PORT2;
		break;
	case C_PORT3:
		portNum = M_PORT3;
		break;
	case C_PORT4:
		portNum = M_PORT4;
		break;
	case C_PORT5:
		portNum = M_PORT5;
		break;
	case C_PORT6:
		portNum = M_PORT6;
		break;
	case C_PORT7:
		portNum = M_PORT7;
		break;
	default:
		portNum = NOT_DEFINED;
		break;
	}
	return portNum;
}

void getPortStrByCport(CARD_SIDE_PORT_NUM port, char *port_str)
{
	switch(port)
	{
	case C_PORT1:
		sprintf(port_str, "%s", "P1");
		break;
	case C_PORT2:
		sprintf(port_str, "%s", "P2");
		break;
	case C_PORT3:
		sprintf(port_str, "%s", "P3");
		break;
	case C_PORT4:
		sprintf(port_str, "%s", "P4");
		break;
	case C_PORT5:
		sprintf(port_str, "%s", "P5");
		break;
	case C_PORT6:
		sprintf(port_str, "%s", "P6");
		break;
	case C_PORT7:
		sprintf(port_str, "%s", "P7");
		break;
	default:
		sprintf(port_str, "%s", "UNKNOWN");
		break;
	}
}
#endif

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
#if 1/*[#80] eag6l board SW bring-up, balkrow, 2023-07-25 */
	gDB.synce_pri_port = NOT_DEFINED;
	gDB.synce_sec_port = NOT_DEFINED;
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
