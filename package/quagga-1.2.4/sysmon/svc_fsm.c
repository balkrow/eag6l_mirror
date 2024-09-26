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
#if 1 /* [#89] Fixing for RDL changes on Target system, dustin, 2024-08-02 */
#include "rdl_fsm.h"

extern uint16_t sys_cpld_memory_read(uint16_t addr);
extern uint16_t sys_cpld_memory_write(uint16_t addr, uint16_t writeval);
#endif

#if 1/*[#110] RDL function Debugging 및 수정, balkrow, 2024-09-02*/
extern int8_t write_pkg_header(uint8_t bank, fw_image_header_t *header);
extern uint8_t get_pkg_header(uint8_t bank, fw_image_header_t *header);
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

#if 1/*[#106] init 시 FPGA update 기능 추가, balkrow, 2024-08-28 */
extern int16_t chk_file_from_dir(const char * prefix, uint16_t *fpga_ver, char *img_name);
extern uint8_t get_fpga_bank(void);
extern int install_fpga_image(uint8_t bno, const char *img_path);
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
#if 1/*[#111] SDK INIT FAIL 시에도 INIT_DONE 설정 그리고 BP FAIL SET, balkrow, 2024-09-06*/
		gRegUpdate(INIT_COMPLETE_ADDR, 0, INIT_COMPLETE_ADDR_MASK, SYS_INIT_DONE);
		if(gDB.svc_fsm.evt == SVC_EVT_SDK_INIT_FAIL) 
			gRegUpdate(CPU_FAIL_ADDR, 8, CPU_FAIL_MASK, 1);
		gDB.init_state = SYS_INIT_DONE;  

#else
		gRegUpdate(INIT_COMPLETE_ADDR, 0, INIT_COMPLETE_ADDR_MASK, 0);
		gRegUpdate(CPU_FAIL_ADDR, 8, CPU_FAIL_MASK, 1);
#if 1/*[#53] Clock source status ¿¿¿¿ ¿¿ ¿¿, balkrow, 2024-06-13*/
		gDB.init_state = SYS_INIT_FAIL;  
#endif
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
#if 1/*[#110] RDL function Debugging 및 수정, balkrow, 2024-09-02*/
#endif
	const char *prog = "appDemo";

	/* NOTE : balkrow moved code for fixing fpga reset issue. */
#if 0/*[#135] fpga register reset 위치 수정, balkrow, 2024-09-26*/
	CPLD_WRITE(0x20, 0xa5a5);
	zlog_notice("clear fpga register!!");
	CPLD_WRITE(0x20, 0);
#endif
	/*write 0x2 0xaa*/
	DPRAM_WRITE(DPRAM_RDL_STATE, 0xaa);
	/*READ 0x2*/
	if(DPRAM_READ(DPRAM_RDL_STATE) == 0xaa)
		rc = SVC_EVT_DPRAM_ACCESS_SUCCESS;
#if 1 /*[#123] BP Debug register, balkrow, 2024-09-09*/
	else
		gRegUpdate(CPU_FAIL_ADDR, 0x15, 0x8000, 0x1); 
#endif

	if(!watch_get_pidof(prog))
		rc = SVC_EVT_APPDEMO_SHUTDOWN;  
#if 1/*[#110] RDL function Debugging 및 수정, balkrow, 2024-09-02*/
	if(get_pkg_header(RDL_BANK_1, &(gDB.bank1_header)) == RT_OK)
		write_pkg_header(RDL_BANK_1, &(gDB.bank1_header));
	else
		zlog_notice("BANK1 pkg header get failed");

	if(get_pkg_header(RDL_BANK_2, &(gDB.bank2_header)) == RT_OK)
		write_pkg_header(RDL_BANK_2, &(gDB.bank2_header));
	else
		zlog_notice("BANK2 pkg header get failed");
#endif

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
#if 1/*[#110] RDL function Debugging 및 수정, balkrow, 2024-09-02*/
	uint16_t running_bank;
#endif
	uint16_t swVer = sysmonUpdateGetSWVer();

	/* [#62] SFP eeprom 및 register update 기능 단위 검증 및 디버깅, balkrow, 2024-06-21 
	 *
	 * WRITE UNIT VERSION(0x0)
	 * */
	FPGA_WRITE(SW_VERSION_ADDR, swVer);

	/*read SW version reg*/
	if(FPGA_READ(SW_VERSION_ADDR) == swVer)
		rc = SVC_EVT_FPGA_ACCESS_SUCCESS;
#if 1 /*[#123] BP Debug register, balkrow, 2024-09-09*/
	else
		gRegUpdate(CPU_FAIL_ADDR, 0x14, 0x4000, 0x1); 
#endif

#if 1/*[#122] primary/secondary Send QL 설정, balkrow, 2024-09-09*/
	gDB.localQL = ESMC_LOCAL_QL;
#endif
#if 1/*[#110] RDL function Debugging 및 수정, balkrow, 2024-09-02*/
	gDB.os_bank = get_os_bank();
	running_bank = (gDB.os_bank << 8) & 0xff00;
	zlog_notice("running os bank %d %x", gDB.os_bank, (gDB.os_bank << 8) & 0xff00);
	FPGA_WRITE(FW_BANK_SELECT_ADDR, running_bank);
#endif

#if 1 /*[#82] eag6l board SW Debugging, balkrow, 2024-08-09*/
	FPGA_WRITE(SYNCE_GCONFIG_ADDR, 0x5a);
#endif
	/* [#62] SFP eeprom 및 register update 기능 단위 검증 및 디버깅, balkrow, 2024-06-21 
	 *
	 * WRITE COMMAND CONTROL REG2(SIG SEL, Rate SEL)(0x20)
	 * */
#if 0/*[#110] RDL function Debugging 및 수정, balkrow, 2024-09-02*/
#if 1 /*[#62] SFP eeprom 및 register update 기능 단위 검증 및 디버깅, balkrow, 2024-06-21 */ 
	for(i = 0; i < PORT_ID_EAG6L_PORT6; i++) 
	{
		gRegUpdate(COMMON_CTRL2_P1_ADDR + (i*2), COMMON_CTRL2_SIG_SHIFT, 
			   COMMON_CTRL2_SIG_MASK, 0x1); /*GGE*/
		gRegUpdate(COMMON_CTRL2_P1_ADDR + (i*2), COMMON_CTRL2_RATE_SHIFT, 
			   COMMON_CTRL2_RATE_MASK, 9);/*25G*/
	}
#endif
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

#if 1/*[#106] init 시 FPGA update 기능 추가, balkrow, 2024-08-28 */
SVC_EVT svc_fpga_switch_failure(SVC_ST st) {
	SVC_EVT evt = SVC_EVT_FPGA_SWITCH_FAIL;
	/*TODO BP FAIL*/
	zlog_notice("FPGA Switch.. Failed");
	return evt;
}

SVC_EVT svc_fpga_switch_confirm(SVC_ST st) {
	SVC_EVT evt = SVC_EVT_FPGA_SWITCH_FAIL;
	uint16_t val;
	char cmd[25] = {0, };

	val = CPLD_READ(FPGA_SWITCH_CONFIRM);
	if(val == 0x9)
	{
		evt = SVC_EVT_FPGA_SWITCH_SUCCESS;
		zlog_notice("Sucessfully FPGA Switch.. bank %d -> %d", gDB.fpga_running_bank, gDB.fpga_act_bank);
		sprintf(cmd, "fw_setenv fw_act_bank %d", gDB.fpga_act_bank);	
		system(cmd);
		if(gDB.fpga_running_bank != 0)
		{
			sprintf(cmd, "fw_setenv fw_stb_bank %d", gDB.fpga_running_bank);	
			system(cmd);
		}
		gDB.fpga_running_bank = gDB.fpga_act_bank;
	}

	return evt;
}

SVC_EVT svc_fpga_switch_wait(SVC_ST st) {
	SVC_EVT evt = SVC_EVT_FPGA_SWITCH_WAIT;
	if(gDB.switch_wait_cnt++ > FPGA_MAX_WAIT)
	{
		evt = SVC_EVT_FPGA_SWITCH_WAIT_EXP; 
		gDB.switch_wait_cnt = 0;
	}

	return evt;
}

SVC_EVT svc_fpga_switch_bank(SVC_ST st) {
	SVC_EVT evt = SVC_EVT_FPGA_SWITCH_WAIT;
	/*FPGA write 0x1c*/	
	if(gDB.switch_cnt++ < FPGA_SWITCH_MAX) 
		CPLD_WRITE(FPGA_SWITCH_ADDR, gDB.fpga_act_bank);
	else
	{
		evt = SVC_EVT_FPGA_SWITCH_TRY_EXP;
	}
	return evt;
}

SVC_EVT svc_fpga_update_fail(SVC_ST st) {
	SVC_EVT evt = SVC_EVT_FPGA_UPDATE_FAIL;
#if 1 /*[#123] BP Debug register, balkrow, 2024-09-09*/
	if(gDB.fpga_running_bank == 0) 
		gRegUpdate(CPU_FAIL_ADDR, 0x13, 0x2000, 0x1); 
#endif
	zlog_notice("FPGA Update.. Failed");
	return evt;
}

SVC_EVT svc_fpga_update(SVC_ST st) {
	SVC_EVT evt = SVC_EVT_FPGA_UPDATE_SUCCESS;
	uint16_t fpga_ver = 0;
	char fpga_img_file[FPGA_IMG_LEN] = {0, };
	char fpga_img_file_path[64] = {0, };
	uint8_t bank, switch_bank, ret;

	gDB.fpga_running_bank = get_fpga_bank();
	gDB.fpga_version = FPGA_READ(FPGA_VER_ADDR); 

	/*check fpga_ver from fpga_image*/
	if(chk_file_from_dir(FPGA_IMG_PREFIX, &fpga_ver, fpga_img_file) == RT_NOK)
	{
		evt = SVC_EVT_FPGA_UPDATE_PASS;
#if 1 /*[#123] BP Debug register, balkrow, 2024-09-09*/
		if(gDB.fpga_running_bank == 0) 
			gRegUpdate(CPU_FAIL_ADDR, 0x13, 0x2000, 0x1); 
#endif
		zlog_notice("No Update FPGA PASS. ");
	}
	else 
	{
		uint16_t fw_ver;
		/* READ FPGA_VER_REG(0xE) */
		if(gDB.fpga_version == fpga_ver)
		{
			evt = SVC_EVT_FPGA_UPDATE_PASS;
			zlog_notice("Check FPGA version (%x,%x) PASS. ",
				    gDB.fpga_version, fpga_ver);
		}
		else /*mismatch update fpga */
		{
			zlog_notice("Check FPGA version (%x,%x) Mismatch. ",
				    gDB.fpga_version, fpga_ver);

			sprintf(fpga_img_file_path, "%s/%s", FPGA_IMG_DIR, fpga_img_file);		

			if(gDB.fpga_running_bank == 0 || 
				gDB.fpga_running_bank  == 2)
				gDB.fpga_act_bank = 1;
			else if(gDB.fpga_running_bank  == 1)
				gDB.fpga_act_bank = 2;
			if(install_fpga_image(gDB.fpga_act_bank , fpga_img_file_path) == -1)
				evt = SVC_EVT_FPGA_UPDATE_FAIL;
		}

	}

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
#if 0/* [#70] Adding RDL feature, dustin, 2024-07-02 */
		{
#if 1 /* [#89] Fixing for RDL changes on Target system, dustin, 2024-08-02 */
			extern fw_header_t RDL_PKG_HEADER;
#if 1 /* [#105] Fixing for RDL install/activation process, dustin, 2024-08-27 */
			extern void rdl_update_bank_registers(int bno, int erase_flag);
			extern uint16_t RDL_B1_ERASE_FLAG, RDL_B2_ERASE_FLAG;
#else
			extern void rdl_update_bank_registers(int bno);
#endif
			int bno = get_sw_active_bank_flag();

			/* get pkg info */
			get_pkg_fwheader_info((bno == RDL_BANK_1) ?
				RDL_B1_PKG_INFO_FILE : RDL_B2_PKG_INFO_FILE, &RDL_PKG_HEADER);

			// update bp os bank info registers.
#if 1 /* [#105] Fixing for RDL install/activation process, dustin, 2024-08-27 */
			if(syscmd_file_exist(RDL_B1_PKG_INFO_FILE))
				rdl_update_bank_registers(RDL_BANK_1, RDL_B1_ERASE_FLAG);
			if(syscmd_file_exist(RDL_B2_PKG_INFO_FILE))
				rdl_update_bank_registers(RDL_BANK_2, RDL_B2_ERASE_FLAG);
#else
			rdl_update_bank_registers(bno);
#endif
#else
			extern void rdl_update_bank_registers(void);

			// update bp os bank info registers.
			rdl_update_bank_registers();
#endif
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
#if 1/*[#106] init 시 FPGA update 기능 추가, balkrow, 2024-08-28 */
			ret = SVC_ST_FPGA_UPDATE; 
#else
			ret = SVC_ST_SDK_INIT; 
#endif
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
#if 1/*[#106] init 시 FPGA update 기능 추가, balkrow, 2024-08-28 */
	case SVC_ST_FPGA_UPDATE:

		if(event == SVC_EVT_FPGA_UPDATE_PASS)
			ret = SVC_ST_SDK_INIT;
		else if(event == SVC_EVT_FPGA_UPDATE_SUCCESS)
			ret = SVC_ST_FPGA_SWITCH;
		else if(event == SVC_EVT_FPGA_UPDATE_FAIL)
			ret = SVC_ST_FPGA_UPDATE_FAILURE;
		else 
			ret = SVC_ST_SDK_INIT;
		break;
	case SVC_ST_FPGA_UPDATE_FAILURE:
			ret = SVC_ST_SDK_INIT;
		break;
	case SVC_ST_FPGA_SWITCH:
		if(event == SVC_EVT_FPGA_SWITCH_WAIT)
			ret = SVC_ST_FPGA_SWITCH_WAIT;
		else if(event == SVC_EVT_FPGA_SWITCH_TRY_EXP)
			ret = SVC_ST_FPGA_SWITCH_FAILURE;
		else
			ret = SVC_ST_SDK_INIT;
		break;
	case SVC_ST_FPGA_SWITCH_FAILURE:
			ret = SVC_ST_SDK_INIT;
		break;

	case SVC_ST_FPGA_SWITCH_CONFIRM:
		if(event == SVC_EVT_FPGA_SWITCH_FAIL)
			ret = SVC_ST_FPGA_SWITCH;
		else
			ret = SVC_ST_SDK_INIT;
		break;

	case SVC_ST_FPGA_SWITCH_WAIT:
		if(event == SVC_EVT_FPGA_SWITCH_WAIT_EXP)
			ret = SVC_ST_FPGA_SWITCH_CONFIRM;
		else
			ret = SVC_ST_FPGA_SWITCH_WAIT;
		break;
#if 0
	case SVC_ST_FPGA_WAIT:
		if(event == SVC_EVT_FPGA_SWITCH_WAIT)
			ret = SVC_ST_FPGA_SWITCH_WAIT;
		else if(event == SVC_EVT_FPGA_SWITCH_SUCCESS)
			ret = SVC_ST_SDK_INIT;
		else if(event == SVC_EVT_FPGA_SWITCH_FAIL)
			ret = SVC_ST_FPGA_SWITCH_FAILURE;
		else
			ret = SVC_ST_SDK_INIT;
		break;
#endif
#endif

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
