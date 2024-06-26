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

#if 1/*[#56] register update timer ¿¿, balkrow, 2023-06-13 */
extern GLOBAL_DB gDB;

uint8_t * gSvcFsmStateStr[SVC_ST_MAX] = {
	"INIT",
	"INIT Fail",
	"APPDemo Shutdown",
	"Check DPRAM access",
	"Check FPGA access",
	"Check DPRAM access",
	"SDK Init",
	"Get Inventory information",
	"INIT Done",
	"----"
};

uint8_t * gSvcFsmEvtStr[SVC_EVT_MAX] = {
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
	"----"
};
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
extern synce_if_pri_select(int8_t port, int8_t pri);
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

#if 1/*[#56] register update timer ¿¿, balkrow, 2023-06-13 */
DEFUN (show_sysmon_system,
       show_sysmon_system_cmd,
       "show system",
       SHOW_STR
       "sysmon info\n")
{
	/*fsm status*/
	vty_out(vty, "-------------------------------------------%s", VTY_NEWLINE);
	vty_out(vty, "Init FSM state %s (%s)%s", gSvcFsmStateStr[gDB.svc_fsm.state],
		gSvcFsmEvtStr[gDB.svc_fsm.evt], VTY_NEWLINE);
	vty_out(vty, "Init FSM state %x (%x)%s", gDB.svc_fsm.state,
		gDB.svc_fsm.evt, VTY_NEWLINE);
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
extern port_pm_counter_t PM_TBL[PORT_ID_EAG6L_MAX];
extern int i2c_in_use_flag;


static void print_port_info(struct vty *vty, int portno)
{
	port_status_t * ps = NULL;
	struct module_inventory * mod_inv = NULL;	

	ps = &(PORT_STATUS[portno]);
	mod_inv = &(INV_TBL[portno]);

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
	return;
}

static void print_sfp_ddm(struct vty *vty, int portno)
{
	port_status_t * ps = NULL;

	ps = &(PORT_STATUS[portno]);

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

static void print_port_pm_counters(struct vty *vty, int portno)
{
    port_pm_counter_t * pmc = NULL;

    pmc = &(PM_TBL[portno]);

    vty_out(vty, "[%d] tx_frame[%13lu] rx_frame[%13lu] tx_byte[%13lu] rx_byte[%13lu] rx_fcs[%13lu] rec_ok[%13lu] rec_nok[%13lu]\n",
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

	gSysmonToCpssFuncs[gPortSetRate](3, (uint16_t)portno, (uint16_t)speed, (uint16_t)speed);

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
       "primary\n"
       "secondary\n")
{
#if 1 /* [#62] SFP eeprom 및 register update 기능 단위 검증 및 디버깅, balkrow, 2024-06-21 */
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
       "get-register ( synce-gconfig | synce-if-select | pm-clear | chip-reset | bd-sfp-cr | fw-bank-select )",
       "get register\n"
       "Synce Global enable\n"
       "Synce IF select\n"
       "PM counter clear\n"
       "Chip reset\n"
       "BD SFP CR\n"
       "FPGA Bank Select\n")
{
	uint16_t addr, val;

	if(! strncmp(argv[0], "synce-if-select", strlen("synce-if-select")))
		addr = SYNCE_IF_SELECT_ADDR;
	else if(! strncmp(argv[0], "pm-clear", strlen("pm-clear")))
		addr = PM_COUNT_CLEAR_ADDR;
	else if(! strncmp(argv[0], "chip-reset", strlen("chip-reset")))
		addr = CHIP_RESET_ADDR;
	else if(! strncmp(argv[0], "bd-sfp-cr", strlen("bd-sfp-cr")))
		addr = BD_SFP_CR_ADDR;
	else if(! strncmp(argv[0], "fw-bank-select", strlen("fw-bank-select")))
		addr = FW_BANK_SELECT_ADDR;
	else {
		vty_out(vty, "%% ADDRESS NOT MATCH%s", VTY_NEWLINE);
		return CMD_ERR_NO_MATCH;
	}

	val = FPGA_READ(addr);

	vty_out(vty, "Read addr[%08x] = 0x%x%s", addr, val, VTY_NEWLINE);
	return CMD_SUCCESS;
}

DEFUN (get_register2,
       get_register2_cmd,
       "get-register ( common-control2 | port-config | alarm-mask ) <1-9>",
       "get register\n"
       "Port Common Control2\n"
       "Target port\n"
       "Port Config\n"
       "Target port\n"
       "Port Alarm Mask\n"
       "Target port\n")
{
	int portno;
	uint16_t addr, val;

	portno = atoi(argv[1]);

	if(! strncmp(argv[0], "common-control2", strlen("common-control2")))
		addr = __COMMON_CTRL2_ADDR[portno];
	else if(! strncmp(argv[0], "port-config", strlen("port-config")))
		addr = __PORT_CONFIG_ADDR[portno];
	else if(! strncmp(argv[0], "alarm-mask", strlen("alarm-mask")))
		addr = __PORT_ALM_MASK_ADDR[portno];
	else {
		vty_out(vty, "%% ADDRESS NOT MATCH%s", VTY_NEWLINE);
		return CMD_ERR_NO_MATCH;
	}

	val = FPGA_PORT_READ(addr);
	vty_out(vty, "Read addr[%08x] = 0x%x%s", addr, val, VTY_NEWLINE);
	return CMD_SUCCESS;
}

DEFUN (set_register,
       set_register_cmd,
       "set-register ( synce-gconfig | synce-if-select | pm-clear | chip-reset | bd-sfp-cr | fw-bank-select ) <0-65535>",
       "Set register\n"
       "Synce Global enable\n"
       "Synce IF select\n"
       "PM counter clear\n"
       "Chip reset\n"
       "BD SFP CR\n"
       "FPGA Bank Select\n"
       "Vlaue to write <0x0000 - 0xFFFF>\n")
{
	uint16_t addr;
	uint32_t val;

	if(! strncmp(argv[0], "synce-if-select", strlen("synce-if-select")))
		addr = SYNCE_IF_SELECT_ADDR;
	else if(! strncmp(argv[0], "pm-clear", strlen("pm-clear")))
		addr = PM_COUNT_CLEAR_ADDR;
	else if(! strncmp(argv[0], "chip-reset", strlen("chip-reset")))
		addr = CHIP_RESET_ADDR;
	else if(! strncmp(argv[0], "bd-sfp-cr", strlen("bd-sfp-cr")))
		addr = BD_SFP_CR_ADDR;
	else if(! strncmp(argv[0], "fw-bank-select", strlen("fw-bank-select")))
		addr = FW_BANK_SELECT_ADDR;
#if 0//PWY_FIXME
	else {
		vty_out(vty, "%% ADDRESS NOT MATCH%s", VTY_NEWLINE);
		return CMD_ERR_NO_MATCH;
	}
#endif //PWY_FIXME

	if(sscanf(argv[1], "%u", &val) != 1) {
		vty_out(vty, "%% INVALID ARG%s", VTY_NEWLINE);
		return CMD_ERR_AMBIGUOUS;
	}
	FPGA_WRITE(addr, (uint16_t)val);
	return CMD_SUCCESS;
}

DEFUN (set_register2,
       set_register2_cmd,
       "set-register ( common-control2 | port-config | alarm-mask ) <1-9> <0-65535>",
       "Set register\n"
       "Port Common Control2\n"
       "Port Config\n"
       "Port Alarm Mask\n"
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
	else if(! strncmp(argv[0], "alarm-mask", strlen("alarm-mask")))
		addr = __PORT_ALM_MASK_ADDR[portno];
	else {
		vty_out(vty, "%% ADDRESS NOT MATCH%s", VTY_NEWLINE);
		return CMD_ERR_NO_MATCH;
	}

	if(sscanf(argv[2], "%u", &val) != 1) {
		vty_out(vty, "%% INVALID ARG%s", VTY_NEWLINE);
		return CMD_ERR_AMBIGUOUS;
	}

	FPGA_PORT_WRITE(addr, (uint16_t)val);
	return CMD_SUCCESS;
}
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
#if 1/*[#55] Adding vty shell test CLIs, dustin, 2024-06-20 */
  install_element (VIEW_NODE, &show_port_cmd);
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

#if 1/*[#59] Synce configuration 연동 기능 추가, balkrow, 2024-06-19 */
  install_element (VIEW_NODE, &synce_enable_cmd);
#endif
}
