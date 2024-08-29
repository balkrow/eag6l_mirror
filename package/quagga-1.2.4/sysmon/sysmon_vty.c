#include <zebra.h>

#include "command.h"
#include "prefix.h"
#include "plist.h"
#include "buffer.h"
#include "linklist.h"
#include "stream.h"
#include "thread.h"
#include "log.h"
#include "memory.h"
#include "hash.h"
#include "filter.h"
#if 1/*[#55] Adding vty shell test CLIs, dustin, 2024-06-20 */
#include "sysmon.h"
#include "bp_regs.h"
#endif
#include "sys_fifo.h"

#if 1/* [#70] Adding RDL feature, dustin, 2024-07-02 */
#include "rdl_fsm.h"

extern RDL_IMG_INFO_t RDL_INFO;
extern fw_image_header_t RDL_OS_HEADER;
extern fw_image_header_t RDL_OS2_HEADER;
extern fw_image_header_t RDL_FW_HEADER;

extern int rdl_decompress_package_file(char *filename);
extern int rdl_install_package(int bno);
extern int rdl_activate_bp(int bno);
#if 1/* [#76] Adding for processing FPGA F/W, dustin, 2024-07-15 */
extern int rdl_activate_fpga(uint8_t bno);
#endif
#endif
#if 1 /* [#85] Fixing for resetting PM counter for unexpected FEC counting, dustin, 2024-07-31 */
extern uint16_t portRateSet (uint16_t port, uint16_t val);
#endif

#if 1/*[#56] register update timer ¿¿, balkrow, 2024-06-13 */
extern GLOBAL_DB gDB;

const char * gSvcFsmStateStr[SVC_ST_MAX] = {
	"INIT",
	"INIT Fail",
#if 1/*[#99] EAG6L 개발 검증 이슈 F/U, balkrow, 2024-08-29 */
	"FPGA Update",
	"FPGA Update Failure",
	"FPGA Switch",
	"FPGA Switch Waiting",
	"FPGA Switch Confirm",
	"FPGA Switch Failure",
#endif
	"APPDemo Shutdown",
	"Check DPRAM access",
	"Check FPGA access",
	"Check DPRAM access",
	"SDK Init",
#if 1/*[#80] eag6l board SW bring-up, balkrow, 2024-07-24 */
	"Waiting SDK Initial",
#endif
	"Get Inventory information",
	"INIT Done",
};

const char * gSvcFsmEvtStr[SVC_EVT_MAX] = {
	"None",
	"Init",
	"IPC Comm Successfully",
	"Wait IPC Comm",
	"IPC Comm Failure",
	"PLL Lock",
	"PLL Unlock",
	"DPRAM Access Successfully",
	"DPRAM Access Failed",
	"FPGA Access Successfully",
	"FPGA Access Failed",
	"CPLD Access Successfully",
	"CPLD Access Failed",
	"SDK Init Successfully",
	"SDK Init Failed",
	"Get Inventory Successfully",
	"Get Inventory Failed",
	"Port Link Up",
	"Port Link Down",
	"INIT Done",
	"AppDemo Shutdown",
#if 1/*[#80] eag6l board SW bring-up, balkrow, 2023-07-24 */
	"Waiting SDK Initial",
	"SYS Init Failure",
#endif
#if 1/*[#99] EAG6L 개발 검증 이슈 F/U, balkrow, 2024-08-29 */
	"FPGA Switch Wait",
	"FPGA Switch Wait Expired",
	"FPGA Switch Successfully",
	"FPGA Switch Failed",
	"FPGA Switch Try Expired",
	"FPGA Update Successfully",
	"FPGA Update Passed",
	"FPGA Update Failed",
#endif
	"----"
};
#endif

#if 1/*[#80] eag6l board SW bring-up, balkrow, 2023-07-25 */
extern void getPortStrByCport(CARD_SIDE_PORT_NUM port, char *port_str);
#endif

extern int8_t sysmon_llcf_set
(
 int8_t enable
);

DEFUN (show_sysmon,
       show_sysmon_cmd,
       "show sysmon",
       SHOW_STR
       "sysmon info\n")
{
  vty_out (vty, "show sysmon test%s", VTY_NEWLINE);
  return CMD_SUCCESS;
}

DEFUN (llcf_conf,
       llcf_conf_cmd,
       "llcf (enable|disable)",
       "LLCF"
       "enable\n"
       "disable\n")
{

  if (strncmp (argv[0], "e", 1) == 0)
	sysmon_llcf_set(IPC_FUNC_ON);
  else
	sysmon_llcf_set(IPC_FUNC_OFF);

  return CMD_SUCCESS;
}

#if 1/*[#56] register update timer 수정, balkrow, 2023-06-13 */
extern cSysmonToCPSSFuncs gSysmonToCpssFuncs[];
extern int synce_if_pri_select(int8_t port, int8_t pri);
DEFUN (synce_if_conf,
       synce_if_conf_cmd,
       "sync-e (primary|secondary) IFNAME",
       "sync ethernet"
       "primary\n"
       "secondary\n"
       "interface number(etc 0,8,16..50)\n")
{
  uint8_t pri = 0;
  uint8_t port = 0;

#if 1/*[#59] Synce configuration 연동 기능 추가, balkrow, 2024-06-19 */
  if(argc != 2)
  {
	vty_out (vty, "parameter error%s", VTY_NEWLINE);
	return CMD_SUCCESS;
  }

  if (strncmp (argv[0], "p", 1) == 0)
	  pri = 1;

  port = atoi(argv[1]); 
  gSysmonToCpssFuncs[gSynceIfSelect](2, pri, port);
#endif

  return CMD_SUCCESS;
}
#endif

#if 1 /*[#82] eag6l board SW Debugging, balkrow, 2024-08-09*/
extern uint16_t portESMCenable (uint16_t port, uint16_t val);
DEFUN (esmc_if_conf,
       esmc_if_conf_cmd,
       "port-esmc (enable|disable) IFNAME",
       "Prt ESMC"
       "Enable\n"
       "Disable\n"
       "interface number(etc 0,8,16..50)\n")
{
  uint8_t enable = 0;
  uint8_t port = 0;

#if 1/*[#59] Synce configuration 연동 기능 추가, balkrow, 2024-06-19 */
  if(argc != 2)
  {
	vty_out (vty, "parameter error%s", VTY_NEWLINE);
	return CMD_SUCCESS;
  }

  if (strncmp (argv[0], "e", 1) == 0)
	  enable = 1;

  port = atoi(argv[1]);
  portESMCenable(port , enable);
#endif

  return CMD_SUCCESS;
}
#endif

#if 1/*[#56] register update timer ¿¿, balkrow, 2023-06-13 */
DEFUN (show_sysmon_system,
       show_sysmon_system_cmd,
       "show system",
       SHOW_STR
       "sysmon info\n")
{
#if 1/*[#80] eag6l board SW bring-up, balkrow, 2023-07-25 */
	char port_str[10] = {0, };
	char pll_state[10] = {0, };
#if 1 /*[#82] eag6l board SW Debugging, balkrow, 2024-08-09*/
	int i;
#endif
	/*fsm status*/
	vty_out(vty, "------|---------|---------------------------%s", VTY_NEWLINE);
	vty_out(vty, "%s %s%s", "FSM    state    : ",gSvcFsmStateStr[gDB.svc_fsm.state], VTY_NEWLINE);
	vty_out(vty, "%s %s%s", "       event(L) : ",gSvcFsmEvtStr[gDB.svc_fsm.evt], VTY_NEWLINE);
	vty_out(vty, "%s %s%s%s","SDK    state    : " ,gDB.sdk_init_state == SDK_INIT_DONE ? "Init Down":"Init Fail" 
		, VTY_NEWLINE, VTY_NEWLINE);
	switch(gDB.pll_state) {
	case FREERUN :
		sprintf(pll_state, "%s", "FREERUN");
		break;
	case LOCK_RECOVERY :
		sprintf(pll_state, "%s", "UNLOCK");
		break;
	case PLL_LOCK :
		sprintf(pll_state, "%s", "LOCK");
		break;
	case HOLD_OVER :
		sprintf(pll_state, "%s", "HOLD OVER");
		break;
	default  :
		sprintf(pll_state, "%s", "UNKNOWN");
		break;
	}
#if 0 /*[#82] eag6l board SW Debugging, balkrow, 2024-07-26*/
	if(gDB.pll_state == FREERUN)
		sprintf(pll_state, "%s", "FREERUN");
	else if(gDB.pll_state == FREERUN)
		sprintf(pll_state, "%s", "FREERUN");
#endif

	vty_out(vty, "PLL    state    : %s%s", pll_state, VTY_NEWLINE);
	vty_out(vty, "Sync-e state    : %s%s", gDB.synce_state == CFG_ENABLE ? "Enable":"Disable"
		, VTY_NEWLINE);
	getPortStrByCport(gDB.synce_pri_port, port_str);
	vty_out(vty, " Pri interface  : %s%s", port_str , VTY_NEWLINE);
	getPortStrByCport(gDB.synce_sec_port, port_str);
	vty_out(vty, " Sec interface  : %s%s", port_str , VTY_NEWLINE);
#if 1/*[#99] EAG6L 개발 검증 이슈 F/U, balkrow, 2024-08-29 */
	vty_out(vty, "FPGA  version    : %x%s", gDB.fpga_version, VTY_NEWLINE);
	vty_out(vty, "FPGA  running bank    : %d%s", gDB.fpga_running_bank, VTY_NEWLINE);
	vty_out(vty, "FPGA  act bank    : %d%s", gDB.fpga_act_bank, VTY_NEWLINE);
#endif

#if 1 /*[#82] eag6l board SW Debugging, balkrow, 2024-08-09*/
	for(i = 0; i < PORT_ID_EAG6L_MAX; i++)
	{
		vty_out(vty, "Port[%d] ESMC RX CFG  : %d%s", i+1 , gDB.esmcRxCfg[i], VTY_NEWLINE);
	}
#endif
#endif
	return CMD_SUCCESS;
}
#endif

#if 1/*[#55] Adding vty shell test CLIs, dustin, 2024-06-20 */
extern int set_smart_tsfp_self_loopback(int portno, int enable);
extern int set_flex_tune_control(int portno, int enable);
extern int set_rtwdm_loopback(int portno, int enable);
extern void port_config_speed(int port, int speed, int mode);

extern cSysmonToCPSSFuncs gSysmonToCpssFuncs[];
extern port_status_t PORT_STATUS[PORT_ID_EAG6L_MAX];
extern struct module_inventory INV_TBL[PORT_ID_EAG6L_MAX];
#if 1/* [#72] Adding omitted rtWDM related registers, dustin, 2024-06-27 */
extern struct module_inventory RTWDM_INV_TBL[PORT_ID_EAG6L_MAX];
#endif
extern port_pm_counter_t PM_TBL[PORT_ID_EAG6L_MAX];
extern int i2c_in_use_flag;


#if 1/* [#72] Adding omitted rtWDM related registers, dustin, 2024-06-27 */
static void print_port_info(struct vty *vty, int portno)
{
	port_status_t * ps = NULL;
	struct module_inventory * mod_inv = NULL;	

	ps = &(PORT_STATUS[portno]);
	mod_inv = &(INV_TBL[portno]);

	vty_out(vty, "[%d] equip[%s] link[%s] speed[%s] tunable[%d] chno[0x%02x] wavelength[%7.2f/%7.2f] flex[%d/%d] tsfp-sloop[%d/%d] rtwdm-loop[%d/%d] sfp[%s]\n", 
		portno, 
		(ps->equip ? "O" : "x"),
		(ps->link ? "Up" : "Dn"), 
		(ps->speed == PORT_IF_10G_KR ? "10G" : "25G"),
		ps->tunable_sfp,
		ps->tunable_chno, 
		ps->tunable_wavelength, ps->tunable_rtwdm_wavelength,
		ps->cfg_flex_tune, ps->flex_tune_status,
		ps->cfg_smart_tsfp_selfloopback, ps->tsfp_self_lp,
		ps->cfg_rtwdm_loopback, ps->rtwdm_lp,
		((ps->sfp_type == SFP_ID_SMART_DUPLEX_TSFP) ? "Smart Duplex T-SFP" : 
			(ps->sfp_type == SFP_ID_CU_SFP) ? "CuSFP" :
			(ps->sfp_type == SFP_ID_SMART_BIDI_TSFP_COT) ? "Smart BiDi TSFP(COT)" :
			(ps->sfp_type == SFP_ID_SMART_BIDI_TSFP_RT) ? "Smart BiDi TSFP(RT)" :
			(ps->sfp_type == SFP_ID_VCSEL_BIDI) ? "VCSEL BIDI" :
			(ps->sfp_type == SFP_ID_6WL) ? "6WL" :
			(ps->sfp_type == SFP_ID_HSFP_HIGH) ? "HSFP (HIGH)" :
			(ps->sfp_type == SFP_ID_HSFP_LOW) ? "HSFP (LOW)" :
			(ps->sfp_type == SFP_ID_CWDM) ? "CWDM" :
			(ps->sfp_type == SFP_ID_DWDM) ? "DWDM" :
			(ps->sfp_type == SFP_ID_VCSEL) ? "VCSEL" :
			(ps->sfp_type == SFP_ID_DWDM_TUNABLE) ? "DWDN Tunable" : "Unknown"));
	return;
}

static void print_sfp_inventory_info(struct vty *vty, int portno)
{
	port_status_t * ps = NULL;
	struct module_inventory * mod_inv = NULL;	
	struct module_inventory * mod_inv2 = NULL;	

	ps = &(PORT_STATUS[portno]);
	mod_inv = &(INV_TBL[portno]);
	mod_inv2 = &(RTWDM_INV_TBL[portno]);

	vty_out(vty, "[%d] sfp[%s]\n"
		"    vendor[%s] part-no[%s] seria-no[%s] wavelength[%d] distance[%d] datecode[%s]\n",
		portno,
		((ps->sfp_type == SFP_ID_SMART_DUPLEX_TSFP) ? "Smart Duplex T-SFP" : 
			(ps->sfp_type == SFP_ID_CU_SFP) ? "CuSFP" :
			(ps->sfp_type == SFP_ID_SMART_BIDI_TSFP_COT) ? "Smart BiDi TSFP(COT)" :
			(ps->sfp_type == SFP_ID_SMART_BIDI_TSFP_RT) ? "Smart BiDi TSFP(RT)" :
			(ps->sfp_type == SFP_ID_VCSEL_BIDI) ? "VCSEL BIDI" :
			(ps->sfp_type == SFP_ID_6WL) ? "6WL" :
			(ps->sfp_type == SFP_ID_HSFP_HIGH) ? "HSFP (HIGH)" :
			(ps->sfp_type == SFP_ID_HSFP_LOW) ? "HSFP (LOW)" :
			(ps->sfp_type == SFP_ID_CWDM) ? "CWDM" :
			(ps->sfp_type == SFP_ID_DWDM) ? "DWDM" :
			(ps->sfp_type == SFP_ID_VCSEL) ? "VCSEL" :
			(ps->sfp_type == SFP_ID_DWDM_TUNABLE) ? "DWDN Tunable" : "Unknown"),
		mod_inv->vendor, 
		mod_inv->part_num, 
		mod_inv->serial_num, 
		mod_inv->wave, 
		mod_inv->dist, 
		mod_inv->date_code);
	if(ps->tunable_sfp) {
		vty_out(vty, 
			"    vendor[%s] part-no[%s] seria-no[%s] wavelength[%d] distance[%d] datecode[%s]\n",
			mod_inv2->vendor, 
			mod_inv2->part_num, 
			mod_inv2->serial_num, 
			mod_inv2->wave, 
			mod_inv2->dist, 
			mod_inv2->date_code);
	}
	return;
}

static void print_sfp_ddm(struct vty *vty, int portno)
{
	port_status_t * ps = NULL;

	ps = &(PORT_STATUS[portno]);
#if 1/*[#61] Adding omitted functions, dustin, 2024-06-24 */
	if(! ps->equip) {
		vty_out(vty, "[%d] no sfp\n", portno);
		return;
	}
#endif

	vty_out(vty, "[%d] vcc[%5.2f] temp[%4.1f] tx_bias[%5.2f] laser[%5.2f] tec_curr[%5.2f] tx_pwr[%5.2f] rx_pwr[%5.2f]\n",
		portno,
		ps->vcc,
		ps->temp,
		ps->tx_bias,
		ps->laser_temp,
		ps->tec_curr,
		ps->tx_pwr,
		ps->rx_pwr);
	if(ps->tunable_sfp) {
		vty_out(vty, "    vcc[%5.2f] temp[%4.1f] tx_bias[%5.2f] laser[%5.2f] tec_curr[%5.2f] tx_pwr[%5.2f] rx_pwr[%5.2f]\n",
			ps->rtwdm_ddm_info.vcc,
			ps->rtwdm_ddm_info.temp,
			ps->rtwdm_ddm_info.tx_bias,
			ps->rtwdm_ddm_info.laser_temp,
			ps->rtwdm_ddm_info.tec_curr,
			ps->rtwdm_ddm_info.tx_pwr,
			ps->rtwdm_ddm_info.rx_pwr);
	}
	return;
}
#else
static void print_port_info(struct vty *vty, int portno)
{
	port_status_t * ps = NULL;
	struct module_inventory * mod_inv = NULL;	

	ps = &(PORT_STATUS[portno]);
	mod_inv = &(INV_TBL[portno]);

#if 1/*[#61] Adding omitted functions, dustin, 2024-06-24 */
	vty_out(vty, "[%d] equip[%d] link[%s] speed[%s] sfp[%s] vendor[%s] part-no[%s] seria-no[%s] wavelength[%d] distance[%d] datecode[%s] tunable[%d] chno[0x%x] wavelength[%7.2f] flex[%d/%d] tsfp-sloop[%d/%d] rtwdm-loop[%d/%d]\n", 
		portno, 
		(ps->equip ? "O" : "x"),
		(ps->link ? "Up" : "Dn"), 
		(ps->speed == PORT_IF_10G_KR ? "10G" : "25G"),
		((ps->sfp_type == SFP_ID_SMART_DUPLEX_TSFP) ? "Smart Duplex T-SFP" : 
			(ps->sfp_type == SFP_ID_CU_SFP) ? "CuSFP" :
			(ps->sfp_type == SFP_ID_SMART_BIDI_TSFP_COT) ? "Smart BiDi TSFP(COT)" :
			(ps->sfp_type == SFP_ID_SMART_BIDI_TSFP_RT) ? "Smart BiDi TSFP(RT)" :
			(ps->sfp_type == SFP_ID_VCSEL_BIDI) ? "VCSEL BIDI" :
			(ps->sfp_type == SFP_ID_6WL) ? "6WL" :
			(ps->sfp_type == SFP_ID_HSFP_HIGH) ? "HSFP (HIGH)" :
			(ps->sfp_type == SFP_ID_HSFP_LOW) ? "HSFP (LOW)" :
			(ps->sfp_type == SFP_ID_CWDM) ? "CWDM" :
			(ps->sfp_type == SFP_ID_DWDM) ? "DWDM" :
			(ps->sfp_type == SFP_ID_VCSEL) ? "VCSEL" :
			(ps->sfp_type == SFP_ID_DWDM_TUNABLE) ? "DWDN Tunable" : "Unknown"),
		mod_inv->vendor, 
		mod_inv->part_num, 
		mod_inv->serial_num, 
		mod_inv->wave, 
		mod_inv->dist, 
		mod_inv->date_code,
		ps->tunable_sfp,
		ps->tunable_chno,
		ps->tunable_wavelength,
		ps->cfg_flex_tune, ps->flex_tune_status,
		ps->cfg_smart_tsfp_selfloopback, ps->tsfp_self_lp,
		ps->cfg_rtwdm_loopback, ps->rtwdm_lp);
#else
	vty_out(vty, "[%d] link[%s] speed[%s] sfp[%s] vendor[%s] part-no[%s] seria-no[%s] wavelength[%d] distance[%d] datecode[%s] flex[%d]\n", 
		portno, 
		(ps->link ? "Up" : "Dn"), 
		(ps->speed == PORT_IF_10G_KR ? "10G" : "25G"),
		((ps->sfp_type == SFP_ID_SMART_DUPLEX_TSFP) ? "Smart Duplex T-SFP" : 
			(ps->sfp_type == SFP_ID_CU_SFP) ? "CuSFP" :
			(ps->sfp_type == SFP_ID_SMART_BIDI_TSFP_COT) ? "Smart BiDi TSFP(COT)" :
			(ps->sfp_type == SFP_ID_SMART_BIDI_TSFP_RT) ? "Smart BiDi TSFP(RT)" :
			(ps->sfp_type == SFP_ID_VCSEL_BIDI) ? "VCSEL BIDI" :
			(ps->sfp_type == SFP_ID_6WL) ? "6WL" :
			(ps->sfp_type == SFP_ID_HSFP_HIGH) ? "HSFP (HIGH)" :
			(ps->sfp_type == SFP_ID_HSFP_LOW) ? "HSFP (LOW)" :
			(ps->sfp_type == SFP_ID_CWDM) ? "CWDM" :
			(ps->sfp_type == SFP_ID_DWDM) ? "DWDM" :
			(ps->sfp_type == SFP_ID_VCSEL) ? "VCSEL" :
			(ps->sfp_type == SFP_ID_DWDM_TUNABLE) ? "DWDN Tunable" : "Unknown"),
		mod_inv->vendor, 
		mod_inv->part_num, 
		mod_inv->serial_num, 
		mod_inv->wave, 
		mod_inv->dist, 
		mod_inv->date_code,
		ps->cfg_flex_tune);
#endif
	return;
}

static void print_sfp_ddm(struct vty *vty, int portno)
{
	port_status_t * ps = NULL;

	ps = &(PORT_STATUS[portno]);
#if 1/*[#61] Adding omitted functions, dustin, 2024-06-24 */
	if(! ps->equip) {
		vty_out(vty, "[%d] no sfp\n", portno);
		return;
	}
#endif

	vty_out(vty, "[%d] vcc[%+5.2f] temp[%+4.1f] tx_bias[%+5.2f] laser[%+5.2f] tec_curr[%+5.2f] tx_pwr[%+5.2f] rx_pwr[%+5.2f]\n",
		portno,
		ps->vcc,
		ps->temp,
		ps->tx_bias,
		ps->laser_temp,
		ps->tec_curr,
		ps->tx_pwr,
		ps->rx_pwr);
	return;
}
#endif

static void print_port_pm_counters(struct vty *vty, int portno)
{
    port_pm_counter_t * pmc = NULL;

    pmc = &(PM_TBL[portno]);

    vty_out(vty, "port[%d] tx_frame[%13llu] rx_frame[%13llu] tx_byte[%13llu] rx_byte[%13llu] rx_fcs[%13llu] fec_ok[%13llu] fec_nok[%13llu]\n",
        portno,
		pmc->tx_frame,
		pmc->rx_frame,
		pmc->tx_byte,
		pmc->rx_byte,
		pmc->rx_fcs,
		pmc->fcs_ok,
		pmc->fcs_nok);
    return;
}

DEFUN (show_port, 
	   show_port_cmd,
       "show port (all | <1-9>)",
	   SHOW_STR
	   "Port\n"
	   "All ports\n"
       "Specified port\n")
{
	int portno;

	if(! strncmp(argv[0], "all", strlen("all"))) {
		for(portno = PORT_ID_EAG6L_PORT1; portno < PORT_ID_EAG6L_MAX; portno++) {
			print_port_info(vty, portno);
		}
	} else {
		portno = atoi(argv[0]);
		print_port_info(vty, portno);
	}
	return CMD_SUCCESS;
}

#if 1/* [#72] Adding omitted rtWDM related registers, dustin, 2024-06-27 */
DEFUN (show_spf_inv, 
	   show_spf_inv_cmd,
       "show sfp-inventory (all | <1-9>)",
	   SHOW_STR
	   "SFP inventory\n"
	   "All ports\n"
       "Specified port\n")
{
	int portno;

	if(! strncmp(argv[0], "all", strlen("all"))) {
		for(portno = PORT_ID_EAG6L_PORT1; portno < PORT_ID_EAG6L_MAX; portno++) {
			print_sfp_inventory_info(vty, portno);
		}
	} else {
		portno = atoi(argv[0]);
		print_sfp_inventory_info(vty, portno);
	}
	return CMD_SUCCESS;
}
#endif

DEFUN (show_ddm,
       show_ddm_cmd,
       "show ddm (all | <1-9>)",
       SHOW_STR
       "sfp ddm\n"
       "All ports\n"
       "Specified port\n")
{
	int portno;

	if(! strncmp(argv[0], "all", strlen("all"))) {
		for(portno = PORT_ID_EAG6L_PORT1; portno < PORT_ID_EAG6L_MAX; portno++) {
			print_sfp_ddm(vty, portno);
		}
	} else {
		portno = atoi(argv[0]);
		print_sfp_ddm(vty, portno);
	}
	return CMD_SUCCESS;
}

DEFUN (show_port_pm,
       show_port_pm_cmd,
       "show pm (all | <1-9>)",
       SHOW_STR
       "pm(performance)\n"
       "All ports\n"
       "Specified port\n")
{
	int portno;

	if(! strncmp(argv[0], "all", strlen("all"))) {
		for(portno = PORT_ID_EAG6L_PORT1; portno < PORT_ID_EAG6L_MAX; portno++) {
			print_port_pm_counters(vty, portno);
		}
	} else {
		portno = atoi(argv[0]);
		print_port_pm_counters(vty, portno);
	}
	return CMD_SUCCESS;
}

DEFUN (set_pm_clear,
       set_pm_clear_cmd,
       "clear pm-counters",
       "clear data\n"
       "pm(performance)\n")
{
	memset(PM_TBL, 0, sizeof PM_TBL);
	return CMD_SUCCESS;
}

DEFUN (set_port_flex_tune,
	   set_port_flex_tune_cmd,
	   "flex-tune port <1-9>",
	   "Set flex-tune\n"
       "Port\n"
	   "Specified port\n")
{
	int portno = atoi(argv[0]);

	i2c_in_use_flag = 1;
	set_flex_tune_control(portno, 1/*enable*/);
	i2c_in_use_flag = 0;
	PORT_STATUS[portno].cfg_flex_tune = 1;
	return CMD_SUCCESS;
}

DEFUN (no_set_port_flex_tune,
       no_set_port_flex_tune_cmd,
       "no flex-tune port <1-9>",
	   NO_STR
       "Set flex-tune\n"
       "Port\n"
       "Specified port\n")
{
    int portno = atoi(argv[0]);

	i2c_in_use_flag = 1;
    set_flex_tune_control(portno, 0/*disable*/);
	i2c_in_use_flag = 0;
	PORT_STATUS[portno].cfg_flex_tune = 0;
	return CMD_SUCCESS;
}

DEFUN (set_port_tsfp_self_loopback,
	   set_port_tsfp_self_loopback_cmd,
	   "tsfp-self-loopback port <1-9>",
	   "Set tsfp self loopback\n"
	   "Port\n"
	   "Specified port\n")
{
	int portno = atoi(argv[0]);

	i2c_in_use_flag = 1;
	set_smart_tsfp_self_loopback(portno, 1/*enable*/);
	i2c_in_use_flag = 0;
	PORT_STATUS[portno].cfg_smart_tsfp_selfloopback = 1;
	return CMD_SUCCESS;
}

DEFUN (no_set_port_tsfp_self_loopback,
	   no_set_port_tsfp_self_loopback_cmd,
	   "no tsfp-self-loopback port <1-9>",
	   NO_STR
	   "Set tsfp self loopback\n"
	   "Port\n"
	   "Specified port\n")
{
	int portno = atoi(argv[0]);

	i2c_in_use_flag = 1;
	set_smart_tsfp_self_loopback(portno, 0/*disable*/);
	i2c_in_use_flag = 0;
	PORT_STATUS[portno].cfg_smart_tsfp_selfloopback = 0;
	return CMD_SUCCESS;
}

DEFUN (set_port_rtwdm_loopback,
       set_port_rtwdm_loopback_cmd,
       "rtwdm-loopback port <1-9>",
       "Set rtwdm loopback\n"
       "Port\n"
       "Specified port\n")
{
    int portno = atoi(argv[0]);

	i2c_in_use_flag = 1;
    set_rtwdm_loopback(portno, 1/*enable*/);
	i2c_in_use_flag = 0;
	PORT_STATUS[portno].cfg_rtwdm_loopback = 1;
    return CMD_SUCCESS;
}

DEFUN (no_set_port_rtwdm_loopback,
       no_set_port_rtwdm_loopback_cmd,
       "no rtwdm-loopback port <1-9>",
       NO_STR
       "Set rtwdm loopback\n"
       "Port\n"
       "Specified port\n")
{
    int portno = atoi(argv[0]);

	i2c_in_use_flag = 1;
    set_rtwdm_loopback(portno, 0/*disable*/);
	i2c_in_use_flag = 0;
	PORT_STATUS[portno].cfg_rtwdm_loopback = 0;
    return CMD_SUCCESS;
}

DEFUN (set_port_speed,
       set_port_speed_cmd,
       "port-speed port <1-9> (10G | 25G)",
       "Set port-speed\n"
       "Port\n"
       "Specified port\n"
       "10G in KR mode\n"
       "25G in KR mode\n")
{
	int portno = atoi(argv[0]);
	int speed = ((! strncmp(argv[1], "10G", strlen("10G"))) || 
	             (! strncmp(argv[1], "10g", strlen("10g"))) ? 
			PORT_IF_10G_KR : PORT_IF_25G_KR);

#if 1 /* [#85] Fixing for resetting PM counter for unexpected FEC counting, dustin, 2024-07-31 */
	portRateSet(portno, (speed == PORT_IF_10G_KR) ? 0x6 : 0x7);
#else
	gSysmonToCpssFuncs[gPortSetRate](3, (uint16_t)portno, (uint16_t)speed, (uint16_t)speed);
#endif/*ZZPP*/

	PORT_STATUS[portno].speed = speed;
	PORT_STATUS[portno].ifmode = speed;
	return CMD_SUCCESS;
}
#endif

#if 1/*[#59] Synce configuration 연동 기능 추가, balkrow, 2024-06-19 */
extern uint16_t synceEnableSet(uint16_t port ,uint16_t val);
DEFUN (synce_enable,
       synce_enable_cmd,
       "sync-e (enable|disable)",
       "sync ethernet"
       "Enable\n"
       "Disable\n")
{
#if 0 /* [#62] SFP eeprom 및 register update 기능 단위 검증 및 디버깅, balkrow, 2024-06-21 */
  uint16_t enable = 0x5a;
  extern uint16_t fpga_sim_val;

  if (strncmp (argv[0], "e", 1) == 0)
	  enable = 0xa5;

  fpga_sim_val = enable;

  return CMD_SUCCESS;
#else
  uint16_t enable = 0x5a;

  if (strncmp (argv[0], "e", 1) == 0)
	  enable = 0xa5;

  synceEnableSet(0, enable);	
	
  return CMD_SUCCESS;
#endif
}
#endif

#if 1/*[#65] Adding regMon simulation feature under ACCESS_SIM, dustin, 2024-06-24 */
DEFUN (get_register,
       get_register_cmd,
#if 1/* [#70] Adding RDL feature, dustin, 2024-07-02 */
       "get-register ( synce-gconfig | synce-if-select | pm-clear | " \
           "chip-reset | bd-sfp-cr | fw-bank-select | keep-alive2 | " \
           "init-complete | " \
           "rdl-req | rdl-resp | rdl-page-crc | rdl-target-bank | rdl-magic | " \
           "rdl-total-crci | rdl-build-time | rdl-total-size | rdl-version | rdl-filename " \
           ")",
#else
       "get-register ( synce-gconfig | synce-if-select | pm-clear | chip-reset | bd-sfp-cr | fw-bank-select | keep-alive2 | init-complete )",
#endif
       "get register\n"
       "Synce Global enable\n"
       "Synce IF select\n"
       "PM counter clear\n"
       "Chip reset\n"
       "BD SFP CR\n"
       "FPGA Bank Select\n"
       "Keep-alive-2\n"
       "INIT Complete\n"
#if 1/* [#70] Adding RDL feature, dustin, 2024-07-02 */
       "RDL State Request\n"
       "RDL State Response\n"
       "RDL page crc\n"
       "RDL target bank\n"
       "RDL magic\n"
       "RDL total crc\n"
       "RDL build-time\n"
       "RDL total size\n"
       "RDL version\n"
       "RDL file-name\n"
#endif
       )
{
	uint16_t addr, val;
#if 1/* [#70] Adding RDL feature, dustin, 2024-07-02 */
	uint32_t addr2;
	uint8_t dpram_flag = 0;
#endif

	if(! strncmp(argv[0], "synce-if-select", strlen("synce-if-select")))
		addr = SYNCE_IF_SELECT_ADDR;
	else if(! strncmp(argv[0], "synce-gconfig", strlen("synce-gconfig")))
		addr = SYNCE_GCONFIG_ADDR;
	else if(! strncmp(argv[0], "pm-clear", strlen("pm-clear")))
		addr = PM_COUNT_CLEAR_ADDR;
	else if(! strncmp(argv[0], "chip-reset", strlen("chip-reset")))
		addr = CHIP_RESET_ADDR;
	else if(! strncmp(argv[0], "bd-sfp-cr", strlen("bd-sfp-cr")))
		addr = BD_SFP_CR_ADDR;
	else if(! strncmp(argv[0], "fw-bank-select", strlen("fw-bank-select")))
		addr2 = FW_BANK_SELECT_ADDR;
	else if(! strncmp(argv[0], "keep-alive2", strlen("keep-alive2")))
		addr = HW_KEEP_ALIVE_2_ADDR;
	else if(! strncmp(argv[0], "init-complete", strlen("init-complete")))
		addr = INIT_COMPLETE_ADDR;
#if 1/* [#70] Adding RDL feature, dustin, 2024-07-02 */
	else if(! strncmp(argv[0], "rdl-req", strlen("rdl-req"))) {
		addr2 = RDL_STATE_REQ_ADDR;
		dpram_flag = 1;
	}
	else if(! strncmp(argv[0], "rdl-resp", strlen("rdl-resp"))) {
		addr2 = RDL_STATE_RESP_ADDR;
		dpram_flag = 1;
	}
	else if(! strncmp(argv[0], "rdl-page-crc", strlen("rdl-page-crc"))) {
		addr2 = RDL_PAGE_CRC_ADDR;
		dpram_flag = 1;
	}
	else if(! strncmp(argv[0], "rdl-target-bank", strlen("rdl-target-bank"))) {
		addr2 = RDL_TARGET_BANK_ADDR;
		dpram_flag = 1;
	}
	else if(! strncmp(argv[0], "rdl-magic", strlen("rdl-magic"))) {
		uint16_t val2;
		addr2 = RDL_MAGIC_NO_1_ADDR;
		val = DPRAM_READ(addr2); val2 = DPRAM_READ(addr2 + 2);
		vty_out(vty, "Read addr[%08x] = 0x%x%s\nRead addr[%08x] = 0x%x%s\n", 
			addr2, val, addr2 + 2, val2, VTY_NEWLINE);
		return CMD_SUCCESS;
	}
	else if(! strncmp(argv[0], "rdl-total-crc", strlen("rdl-total-crc"))) {
		uint16_t val2;
		addr2 = RDL_TOTAL_CRC_1_ADDR;
		val = DPRAM_READ(addr2); val2 = DPRAM_READ(addr2 + 2);
		vty_out(vty, "Read addr[%08x] = 0x%x%s\nRead addr[%08x] = 0x%x%s\n", 
			addr2, val, addr2 + 2, val2, VTY_NEWLINE);
		return CMD_SUCCESS;
	}
	else if(! strncmp(argv[0], "rdl-build-time", strlen("rdl-build-time"))) {
		uint16_t val2;
		addr2 = RDL_BUILD_TIME_1_ADDR;
		val = DPRAM_READ(addr2); val2 = DPRAM_READ(addr2 + 2);
		vty_out(vty, "Read addr[%08x] = 0x%x%s\nRead addr[%08x] = 0x%x%s\n", 
			addr2, val, addr2 + 2, val2, VTY_NEWLINE);
		return CMD_SUCCESS;
	}
	else if(! strncmp(argv[0], "rdl-total-size", strlen("rdl-total-size"))) {
		uint16_t val2;
		addr2 = RDL_TOTAL_SIZE_1_ADDR;
		val = DPRAM_READ(addr2); val2 = DPRAM_READ(addr2 + 2);
		vty_out(vty, "Read addr[%08x] = 0x%x%s\nRead addr[%08x] = 0x%x%s\n", 
			addr2, val, addr2 + 2, val2, VTY_NEWLINE);
		return CMD_SUCCESS;
	}
	else if(! strncmp(argv[0], "rdl-version", strlen("rdl-version"))) {
		uint16_t ii = 0;
		uint8_t buf[50];
		memset(buf, 0, sizeof buf);
		for(addr2 = RDL_VER_STR_START_ADDR; addr2 < RDL_VER_STR_END_ADDR; addr2 += 2)
			buf[ii++] = DPRAM_READ(addr2);
		vty_out(vty, "Read = [%s]%s\n", buf, VTY_NEWLINE);
		return CMD_SUCCESS;
	}
	else if(! strncmp(argv[0], "rdl-file-name", strlen("rdl-file-name"))) {
		uint16_t ii = 0;
		uint8_t buf[50];
		memset(buf, 0, sizeof buf);
		for(addr2 = RDL_FILE_NAME_START_ADDR; addr2 < RDL_FILE_NAME_END_ADDR; addr2 += 2)
			buf[ii++] = DPRAM_READ(addr2);
		vty_out(vty, "Read = [%s]%s\n", buf, VTY_NEWLINE);
		return CMD_SUCCESS;
	}
#endif
	else {
		vty_out(vty, "%% ADDRESS NOT MATCH%s", VTY_NEWLINE);
		return CMD_ERR_NO_MATCH;
	}

#if 1/* [#70] Adding RDL feature, dustin, 2024-07-02 */
	if(dpram_flag)
		val = DPRAM_READ(addr2);
	else
		val = FPGA_READ(addr);

#ifdef ACCESS_SIM
	vty_out(vty, "[ACCESS_SIM] mode.%s", VTY_NEWLINE);
#endif
	vty_out(vty, "Read addr[%08x] = 0x%x%s", dpram_flag ? addr2 : addr, val, VTY_NEWLINE);
#else
	val = FPGA_READ(addr);

	vty_out(vty, "Read addr[%08x] = 0x%x%s", addr, val, VTY_NEWLINE);
#endif
	return CMD_SUCCESS;
}

DEFUN (get_register2,
       get_register2_cmd,
       "get-register ( common-control2 | port-config | alarm | alarm-mask | set-channel-no | get-channel-no ) <1-9>",
       "get register\n"
       "Port Common Control2\n"
       "Target port\n"
       "Port Config\n"
       "Target port\n"
       "Port Alarm\n"
       "Port Alarm Mask\n"
       "Set channel no for tunable sfp\n"
       "Get channel no for tunable sfp\n"
       "Target port\n")
{
	int portno;
	uint16_t addr, val;

	portno = atoi(argv[1]);

	if(! strncmp(argv[0], "common-control2", strlen("common-control2")))
		addr = __COMMON_CTRL2_ADDR[portno];
	else if(! strncmp(argv[0], "port-config", strlen("port-config")))
		addr = __PORT_CONFIG_ADDR[portno];
#if 1 /* [#88] Adding LF/RF reading and updating to Alarm, dustin, 2024-08-01 */
	else if(! strncmp(argv[0], "alarm", strlen("alarm")))
		addr = __PORT_ALM_ADDR[portno];
#endif
	else if(! strncmp(argv[0], "alarm-mask", strlen("alarm-mask")))
		addr = __PORT_ALM_MASK_ADDR[portno];
	else if(! strncmp(argv[0], "set-channel-no", strlen("set-channel-no")))
		addr = __PORT_SET_CH_NUM_ADDR[portno];
	else if(! strncmp(argv[0], "get-channel-no", strlen("get-channel-no")))
		addr = __PORT_GET_CH_NUM_ADDR[portno];
	else {
		vty_out(vty, "%% ADDRESS NOT MATCH%s", VTY_NEWLINE);
		return CMD_ERR_NO_MATCH;
	}

#if 1 /* [#91] Fixing for register updating feature, dustin, 2024-08-05 */
	if(addr == __COMMON_CTRL2_ADDR[portno])
		val = FPGA_READ(addr);
	else
		val = FPGA_PORT_READ(addr);
#else
	val = FPGA_PORT_READ(addr);
#endif
#ifdef ACCESS_SIM
	vty_out(vty, "[ACCESS_SIM] mode.%s", VTY_NEWLINE);
#endif
	vty_out(vty, "Read addr[%08x] = 0x%x%s", addr, val, VTY_NEWLINE);
	return CMD_SUCCESS;
}

#if 1 /* [#89] Fixing for RDL changes on Target system, dustin, 2024-08-02 */
DEFUN (get_register3,
       get_register3_cmd,
       "get-register WORD",
       "get register\n"
       "address to read (in hexadecimal, 0x??)\n")
{
	uint32_t addr;
	uint16_t val;

	if(sscanf(argv[0], "0x%x", &addr) != 1) {
		if(sscanf(argv[0], "%u", &addr) != 1) {
			vty_out(vty, "%% INVALID ARG%s", VTY_NEWLINE);
			return CMD_ERR_AMBIGUOUS;
		}
	}

	if((RDL_STATE_REQ_ADDR <= addr) && (addr <= RDL_PAGE_2_END_ADDR))
		val = DPRAM_READ(addr);
	else
		val = FPGA_READ((uint16_t)addr);
#ifdef ACCESS_SIM
	vty_out(vty, "[ACCESS_SIM] mode.%s", VTY_NEWLINE);
#endif
	vty_out(vty, "Read addr[%08x] = 0x%x%s", addr, val, VTY_NEWLINE);
	return CMD_SUCCESS;
}
#endif

DEFUN (set_register,
       set_register_cmd,
#if 1/* [#70] Adding RDL feature, dustin, 2024-07-02 */
       "set-register ( synce-gconfig | synce-if-select | pm-clear | " \
           "chip-reset | bd-sfp-cr | fw-bank-select | keep-alive2 | " \
           "init-complete | " \
           "rdl-req | rdl-resp | rdl-page-crc | rdl-target-bank | rdl-magic | " \
           "rdl-total-crci | rdl-build-time | rdl-total-size | rdl-version | rdl-filename " \
           ") WORD",
#else
       "set-register ( synce-gconfig | synce-if-select | pm-clear | chip-reset | bd-sfp-cr | fw-bank-select | keep-alive2 | init-complete ) <0-65535>",
#endif
       "Set register\n"
       "Synce Global enable\n"
       "Synce IF select\n"
       "PM counter clear\n"
       "Chip reset\n"
       "BD SFP CR\n"
       "FPGA Bank Select\n"
       "Keep-alive-2\n"
       "INIT Complete\n"
#if 1/* [#70] Adding RDL feature, dustin, 2024-07-02 */
       "RDL State Request\n"
       "RDL State Response\n"
       "RDL page crc\n"
       "RDL target bank\n"
       "RDL magic\n"
       "RDL total crc\n"
       "RDL build-time\n"
       "RDL total size\n"
       "RDL version\n"
       "RDL file-name\n"
#endif
       "Vlaue to write <0x0000 - 0xFFFF>\n")
{
	uint16_t addr;
	uint32_t val;
#if 1/* [#70] Adding RDL feature, dustin, 2024-07-02 */
	uint32_t addr2;
	uint8_t dpram_flag = 0;
	uint8_t buf[50];
#endif

	if(! strncmp(argv[0], "synce-if-select", strlen("synce-if-select")))
		addr = SYNCE_IF_SELECT_ADDR;
	else if(! strncmp(argv[0], "synce-gconfig", strlen("synce-gconfig")))
		addr = SYNCE_GCONFIG_ADDR;
	else if(! strncmp(argv[0], "pm-clear", strlen("pm-clear")))
		addr = PM_COUNT_CLEAR_ADDR;
	else if(! strncmp(argv[0], "chip-reset", strlen("chip-reset")))
		addr = CHIP_RESET_ADDR;
	else if(! strncmp(argv[0], "bd-sfp-cr", strlen("bd-sfp-cr")))
		addr = BD_SFP_CR_ADDR;
	else if(! strncmp(argv[0], "fw-bank-select", strlen("fw-bank-select")))
		addr2 = FW_BANK_SELECT_ADDR;
	else if(! strncmp(argv[0], "keep-alive2", strlen("keep-alive2")))
		addr = HW_KEEP_ALIVE_2_ADDR;
	else if(! strncmp(argv[0], "init-complete", strlen("init-complete")))
		addr = INIT_COMPLETE_ADDR;
#if 1/* [#70] Adding RDL feature, dustin, 2024-07-02 */
	else if(! strncmp(argv[0], "rdl-req", strlen("rdl-req"))) {
		addr2 = RDL_STATE_REQ_ADDR;
		dpram_flag = 1;
	}
	else if(! strncmp(argv[0], "rdl-resp", strlen("rdl-resp"))) {
		addr2 = RDL_STATE_RESP_ADDR;
		dpram_flag = 1;
	}
	else if(! strncmp(argv[0], "rdl-page-crc", strlen("rdl-page-crc"))) {
		addr2 = RDL_PAGE_CRC_ADDR;
		dpram_flag = 1;
	}
	else if(! strncmp(argv[0], "rdl-target-bank", strlen("rdl-target-bank"))) {
		addr2 = RDL_TARGET_BANK_ADDR;
		dpram_flag = 1;
	}
	else if(! strncmp(argv[0], "rdl-magic", strlen("rdl-magic"))) {
		addr2 = RDL_MAGIC_NO_1_ADDR;
		dpram_flag = 1;
	}
	else if(! strncmp(argv[0], "rdl-total-crc", strlen("rdl-total-crc"))) {
		addr2 = RDL_TOTAL_CRC_1_ADDR;
		dpram_flag = 1;
	}
	else if(! strncmp(argv[0], "rdl-build-time", strlen("rdl-build-time"))) {
		addr2 = RDL_BUILD_TIME_1_ADDR;
		dpram_flag = 1;
	}
	else if(! strncmp(argv[0], "rdl-total-size", strlen("rdl-total-size"))) {
		addr2 = RDL_TOTAL_SIZE_1_ADDR;
		dpram_flag = 1;
	}
	else if(! strncmp(argv[0], "rdl-version", strlen("rdl-version"))) {
		addr2 = RDL_VER_STR_START_ADDR;
		dpram_flag = 1;
	}
	else if(! strncmp(argv[0], "rdl-file-name", strlen("rdl-file-name"))) {
		addr2 = RDL_FILE_NAME_START_ADDR;
		dpram_flag = 1;
	}
#endif
	else {
		vty_out(vty, "%% ADDRESS NOT MATCH%s", VTY_NEWLINE);
		return CMD_ERR_NO_MATCH;
	}

#if 1/* [#70] Adding RDL feature, dustin, 2024-07-02 */
	if(sscanf(argv[1], "0x%x", &val) != 1) {
		if(sscanf(argv[1], "%u", &val) != 1) {
			if(sscanf(argv[1], "%s", buf) != 1) {
				vty_out(vty, "%% INVALID ARG%s", VTY_NEWLINE);
				return CMD_ERR_AMBIGUOUS;
			}
		}
	}

	if(dpram_flag) {
		if(! strncmp(argv[0], "rdl-magic", strlen("rdl-magic"))) {
			DPRAM_WRITE(addr2, (val & 0xFFFF)); 
			DPRAM_WRITE(addr2 + 2, (val >> 16) & 0xFFFF);
			return CMD_SUCCESS;
		}
		else if(! strncmp(argv[0], "rdl-total-crc", strlen("rdl-total-crc"))) {
			DPRAM_WRITE(addr2, (val & 0xFFFF)); 
			DPRAM_WRITE(addr2 + 2, (val >> 16) & 0xFFFF);
			return CMD_SUCCESS;
		}
		else if(! strncmp(argv[0], "rdl-build-time", strlen("rdl-build-time"))) {
			DPRAM_WRITE(addr2, (val & 0xFFFF)); 
			DPRAM_WRITE(addr2 + 2, (val >> 16) & 0xFFFF);
			return CMD_SUCCESS;
		}
		else if(! strncmp(argv[0], "rdl-total-size", strlen("rdl-total-size"))) {
			DPRAM_WRITE(addr2, (val & 0xFFFF)); 
			DPRAM_WRITE(addr2 + 2, (val >> 16) & 0xFFFF);
			return CMD_SUCCESS;
		}
		else if(! strncmp(argv[0], "rdl-version", strlen("rdl-version"))) {
			uint16_t ii = 0;
			for(addr2 = RDL_VER_STR_START_ADDR; addr2 < RDL_VER_STR_END_ADDR; addr2 += 2) {
				val = buf[ii] | (buf[ii + 1] << 8);
				ii += 2;
				DPRAM_WRITE(addr2, val);
			}
			return CMD_SUCCESS;
		}
		else if(! strncmp(argv[0], "rdl-file-name", strlen("rdl-file-name"))) {
			uint16_t ii = 0;
			for(addr2 = RDL_FILE_NAME_START_ADDR; addr2 < RDL_FILE_NAME_END_ADDR; addr2 += 2) {
				val = buf[ii] | (buf[ii + 1] << 8);
				ii += 2;
				DPRAM_WRITE(addr2, val);
			}
			return CMD_SUCCESS;
		} else
			DPRAM_WRITE(addr2, (uint16_t)val);
	}
	else
		FPGA_WRITE(addr, (uint16_t)val);
#else
	if(sscanf(argv[1], "0x%x", &val) != 1) {
		if(sscanf(argv[1], "%u", &val) != 1) {
			vty_out(vty, "%% INVALID ARG%s", VTY_NEWLINE);
			return CMD_ERR_AMBIGUOUS;
		}
	}

	FPGA_WRITE(addr, (uint16_t)val);
#endif
	return CMD_SUCCESS;
}

DEFUN (set_register2,
       set_register2_cmd,
       "set-register ( common-control2 | port-config | alarm-mask | set-channel-no | get-channel-no ) <1-9> WORD",
       "Set register\n"
       "Port Common Control2\n"
       "Port Config\n"
       "Port Alarm Mask\n"
       "Set channel No for tunable sfp\n"
       "Get channel No for tunable sfp\n"
       "Target port\n"
       "Vlaue to write <0x0000 - 0xFFFF>\n")
{
	int portno;
	uint16_t addr;
	uint32_t val;

	portno = atoi(argv[1]);

	if(! strncmp(argv[0], "common-control2", strlen("common-control2")))
		addr = __COMMON_CTRL2_ADDR[portno];
	else if(! strncmp(argv[0], "port-config", strlen("port-config")))
		addr = __PORT_CONFIG_ADDR[portno];
#if 1 /* [#88] Adding LF/RF reading and updating to Alarm, dustin, 2024-08-01 */
	else if(! strncmp(argv[0], "alarm", strlen("alarm")))
		addr = __PORT_ALM_ADDR[portno];
#endif
	else if(! strncmp(argv[0], "alarm-mask", strlen("alarm-mask")))
		addr = __PORT_ALM_MASK_ADDR[portno];
	else if(! strncmp(argv[0], "set-channel-no", strlen("set-channel-no")))
		addr = __PORT_SET_CH_NUM_ADDR[portno];
	else if(! strncmp(argv[0], "get-channel-no", strlen("get-channel-no")))
		addr = __PORT_GET_CH_NUM_ADDR[portno];
	else {
		vty_out(vty, "%% ADDRESS NOT MATCH%s", VTY_NEWLINE);
		return CMD_ERR_NO_MATCH;
	}

	if(sscanf(argv[2], "0x%x", &val) != 1) {
		if(sscanf(argv[2], "%u", &val) != 1) {
			vty_out(vty, "%% INVALID ARG%s", VTY_NEWLINE);
			return CMD_ERR_AMBIGUOUS;
		}
	}

#if 1 /* [#91] Fixing for register updating feature, dustin, 2024-08-05 */
	if(addr == __COMMON_CTRL2_ADDR[portno])
		FPGA_WRITE(addr, (uint16_t)val);
	else
		FPGA_PORT_WRITE(addr, (uint16_t)val);
#else
	FPGA_PORT_WRITE(addr, (uint16_t)val);
#endif
	return CMD_SUCCESS;
}
#endif

#if 1 /* [#89] Fixing for RDL changes on Target system, dustin, 2024-08-02 */
DEFUN (set_register3,
       set_register3_cmd,
       "set-register WORD WORD",
       "set register\n"
       "address to write (in hexadecimal, 0x??)\n"
       "value to write (in hexadecimal, 0x??)\n")
{
	uint32_t data, addr;
	uint16_t val;

	if(sscanf(argv[0], "0x%x", &data) != 1) {
		if(sscanf(argv[0], "%u", &data) != 1) {
			vty_out(vty, "%% INVALID ARG%s", VTY_NEWLINE);
			return CMD_ERR_AMBIGUOUS;
		}
	}
	addr = data;

	if(sscanf(argv[1], "0x%x", &data) != 1) {
		if(sscanf(argv[1], "%u", &data) != 1) {
			vty_out(vty, "%% INVALID ARG%s", VTY_NEWLINE);
			return CMD_ERR_AMBIGUOUS;
		}
	}
	val = (uint16_t)data;

	if((RDL_STATE_REQ_ADDR <= addr) && (addr <= RDL_PAGE_2_END_ADDR))
		DPRAM_WRITE(addr, (uint16_t)val);
	else
		FPGA_WRITE((uint16_t)addr, (uint16_t)val);
	return CMD_SUCCESS;
}
#endif

#if 1/* [#70] Adding RDL feature, dustin, 2024-07-02 */
DEFUN (rdl_test,
       rdl_test_cmd,
       "rdl (start | decompress | install-1 | install-2 | activate-1 | activate-2 | fpga-1 | fpga-2)",
       "RDL\n"
       "RDL start\n"
       "RDL decompress\n"
       "RDL install to bank 1\n"
       "RDL install to bank 2\n"
       "RDL activate-1\n"
       "RDL activate-2\n"
       "RDL FPGA 1\n"
       "RDL FPGA 2\n")
{
extern uint8_t *RDL_PAGE;
	char cmd[200];
	int mflag = 0;

	// create directories.
#if 1 /* [#89] Fixing for RDL changes on Target system, dustin, 2024-08-02 */
	snprintf(cmd, sizeof(cmd) - 1, "mkdir %s; mkdir %s; mkdir %s",
		RDL_IMG_PATH, RDL_B1_PATH, RDL_B2_PATH);
#else
	snprintf(cmd, sizeof(cmd) - 1, "mkdir %s; mkdir %s; mkdir %s; mkdir %s",
		RDL_IMG_PATH, RDL_B1_PATH, RDL_B2_PATH, RDL_BOOT_PATH);
#endif
	system(cmd);

	memset(&RDL_OS_HEADER, 0, sizeof RDL_OS_HEADER);
	memset(&RDL_OS2_HEADER, 0, sizeof RDL_OS_HEADER);
	memset(&RDL_FW_HEADER, 0, sizeof RDL_OS_HEADER);
#if 1 /* [#89] Fixing for RDL changes on Target system, dustin, 2024-08-02 */
	strcpy(RDL_OS_HEADER.fih_name, "eag6l-os-v1.0.0.bin");
	strcpy(RDL_OS2_HEADER.fih_name, "FPGA_LED_FAIL_240812-v1.0.0.bin");
#else
	strcpy(RDL_OS2_HEADER.fih_name, "eag6l-fpga-os-v1.0.0.bin");
	strcpy(RDL_FW_HEADER.fih_name, "eag6l-fpga-fw-v1.0.0.bin");
#endif
	strcpy(RDL_FW_HEADER.fih_name, "");

	if(RDL_PAGE == NULL) {
		RDL_PAGE = malloc(RDL_PAGE_SIZE);
		if(RDL_PAGE == NULL) {
			vty_out(vty, "Cannot alloc RDL_PAGE\n");
			return CMD_ERR_NO_MATCH;
		}
		mflag = 1;
	}

	if(! strncmp(argv[0], "start", strlen("start"))) {
		// init rdl registers.
		// rdl start and emulate states.
	} else if(! strncmp(argv[0], "decompress", strlen("decompress"))) {
		// init RDL_INFO.
		memset(&RDL_INFO, 0, sizeof(RDL_INFO));
		RDL_INFO.bno           = RDL_BANK_1;
		RDL_INFO.hd.magic      = htonl(RDL_IMG_MAGIC);
		RDL_INFO.hd.total_size = 55427984;
		RDL_INFO.hd.total_crc  = 0xAADD;
		strcpy(RDL_INFO.hd.ver_str, "1.0.0");
#if 1 /* [#89] Fixing for RDL changes on Target system, dustin, 2024-08-02 */
		strcpy(RDL_INFO.hd.file_name, "EAG6L-PKG-v1.0.0.bin");
#else
		strcpy(RDL_INFO.hd.file_name, "EAG6L-PKG-0100.PKG");
#endif

		// call decompress. pkg file fixed as EAG6L-PKG-0100.PKG
		if(rdl_decompress_package_file(RDL_INFO.hd.file_name) < 0) {
        	//FIXME : success or failed, pkg file will be removed anyway.
        	vty_out(vty, "[RDL] Decompressing pkg file %s has failed. Go to IDLE.",
        		RDL_INFO.hd.file_name);
        	return CMD_SUCCESS;
        }

       	vty_out(vty, "[RDL] Decompressing pkg file %s has DONE.",
       		RDL_INFO.hd.file_name);
	} else if(! strncmp(argv[0], "install-1", strlen("install-1"))) {
		rdl_install_package(RDL_BANK_1);
	} else if(! strncmp(argv[0], "install-2", strlen("install-2"))) {
		rdl_install_package(RDL_BANK_2);
	} else if(! strncmp(argv[0], "activate-1", strlen("activate-1"))) {
		rdl_activate_bp(RDL_BANK_1);
	} else if(! strncmp(argv[0], "activate-2", strlen("activate-2"))) {
		rdl_activate_bp(RDL_BANK_2);
	}
#if 1/* [#76] Adding for processing FPGA F/W, dustin, 2024-07-15 */
	else if(! strncmp(argv[0], "fpga-1", strlen("fpga-1"))) {
		rdl_activate_fpga(RDL_BANK_1);
	} else if(! strncmp(argv[0], "fpga-2", strlen("fpga-2"))) {
		rdl_activate_fpga(RDL_BANK_2);
	}
#endif

	if(mflag) {
		free(RDL_PAGE);
		RDL_PAGE = NULL;
	}
	return CMD_SUCCESS;
}

#if 1/* [#77] Adding RDL emulation function, dustin, 2024-07-16 */
DEFUN (rdl_emu_test,
       rdl_emu_test_cmd,
       "rdl emulation WORD",
       "RDL\n"
       "RDL emulation for MCU - BP interworking\n"
       "RDL Target File (img header + zipped pkg)\n")
{
extern char EMUL_FPATH[200];
extern int EMUL_TRIGGERED;

	// check if target file is present.
	snprintf(EMUL_FPATH, sizeof(EMUL_FPATH) - 1, "%s%s", 
		RDL_DFT_PATH, argv[0]);
	if(! syscmd_file_exist(EMUL_FPATH)) {
		vty_out(vty, "RDL] No such file : %s\n", EMUL_FPATH);
		return CMD_ERR_INCOMPLETE;
	}

	// trigger flag to run emulation.
	vty_out(vty, "\n[RDL] Start Emulation for %s.\n\n", EMUL_FPATH);
	EMUL_TRIGGERED = 1;

	return CMD_SUCCESS;
}
#endif
#endif

void
sysmon_vty_init (void)
{
  install_element (VIEW_NODE, &show_sysmon_cmd);
  install_element (VIEW_NODE, &llcf_conf_cmd);
#if 1/*[#56] register update timer 수정, balkrow, 2023-06-13 */
  install_element (VIEW_NODE, &synce_if_conf_cmd);
  install_element (VIEW_NODE, &show_sysmon_system_cmd);
#endif
#if 1 /*[#82] eag6l board SW Debugging, balkrow, 2024-08-09*/
  install_element (VIEW_NODE, &esmc_if_conf_cmd);
#endif
#if 1/*[#55] Adding vty shell test CLIs, dustin, 2024-06-20 */
  install_element (VIEW_NODE, &show_port_cmd);
#if 1/* [#72] Adding omitted rtWDM related registers, dustin, 2024-06-27 */
  install_element (VIEW_NODE, &show_spf_inv_cmd);
#endif
  install_element (VIEW_NODE, &show_ddm_cmd);
  install_element (VIEW_NODE, &show_port_pm_cmd);
  install_element (ENABLE_NODE, &set_pm_clear_cmd);
  install_element (ENABLE_NODE, &set_port_flex_tune_cmd);
  install_element (ENABLE_NODE, &no_set_port_flex_tune_cmd);
  install_element (ENABLE_NODE, &set_port_tsfp_self_loopback_cmd);
  install_element (ENABLE_NODE, &no_set_port_tsfp_self_loopback_cmd);
  install_element (ENABLE_NODE, &set_port_rtwdm_loopback_cmd);
  install_element (ENABLE_NODE, &no_set_port_rtwdm_loopback_cmd);
  install_element (ENABLE_NODE, &set_port_speed_cmd);
#endif
#if 1/*[#65] Adding regMon simulation feature under ACCESS_SIM, dustin, 2024-06-24 */
  install_element (VIEW_NODE, &get_register_cmd);
  install_element (VIEW_NODE, &get_register2_cmd);
  install_element (ENABLE_NODE, &set_register_cmd);
  install_element (ENABLE_NODE, &set_register2_cmd);
#endif
#if 1 /* [#89] Fixing for RDL changes on Target system, dustin, 2024-08-02 */
  install_element (VIEW_NODE, &get_register3_cmd);
  install_element (ENABLE_NODE, &set_register3_cmd);
#endif
#if 1/* [#70] Adding RDL feature, dustin, 2024-07-02 */
  install_element (ENABLE_NODE, &rdl_test_cmd);
#if 1/* [#77] Adding RDL emulation function, dustin, 2024-07-16 */
  install_element (ENABLE_NODE, &rdl_emu_test_cmd);
#endif
#endif

#if 1/*[#59] Synce configuration 연동 기능 추가, balkrow, 2024-06-19 */
  install_element (VIEW_NODE, &synce_enable_cmd);
#endif
}
