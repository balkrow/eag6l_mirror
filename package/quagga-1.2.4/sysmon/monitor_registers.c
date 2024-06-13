#include "sysmon.h"
#include "bp_regs.h"

#undef DEBUG

extern u32 INIT_COMPLETE_FLAG;

u16 PORT_CONFIG_CACHE[PORT_ID_EAG6L_MAX];
u16 PORT_COMMON_CONTROL2_CACHE[PORT_ID_EAG6L_MAX];
u16 SYNCE_GCONFIG_CACHE;
u16 SYNCE_IF_SELECT_CACHE;
u32 MCU_KEEP_ALIVE_CACHE;


struct port_status PORT_STATUS[PORT_ID_EAG6L_MAX];
struct module_inventory INV_TBL[PORT_ID_EAG6L_MAX];
port_pm_counter_t PM_TBL[PORT_ID_EAG6L_MAX];


extern int synce_config_set_admin(int enable);
extern int synce_config_set_if_select(int pri_port, int sec_port);
extern void pm_request_counters(void);
extern void pm_request_clear(void);

#if 1/*[#48] register monitoring and update 관련 기능 추가, balkrow, 2024-06-10*/ 
#if 1/*[#51] Adding register callback templates for config/command registers, dustin, 2024-06-12 */
uint16_t portRateSet (uint16_t port, uint16_t val);
uint16_t synceEnableSet(uint16_t port, uint16_t val);
uint16_t synceIFSelect(uint16_t port, uint16_t val);
uint16_t pmClear(uint16_t port, uint16_t val);
uint16_t chipReset(uint16_t port, uint16_t val);
extern uint16_t sys_fpga_memory_read(uint16_t addr);
extern void port_config_ESMC_enable(int port, int enable);
extern int set_flex_tune_control(int port, int enable);
extern int set_rtwdm_loopback(int port, int enable);
extern int set_smart_tsfp_self_loopback(int port, int enable);
extern int set_flex_tune_reset(int port, int enable);

RegMON regMonList [] = {
	/* common control 2 - port speed */
  { COMMON_CTRL2_P1_ADDR, 0x3, 0, 0x7, PORT_ID_EAG6L_PORT1, sys_fpga_memory_read, portRateSet }, 
  { COMMON_CTRL2_P2_ADDR, 0x3, 0, 0x7, PORT_ID_EAG6L_PORT2, sys_fpga_memory_read, portRateSet }, 
  { COMMON_CTRL2_P3_ADDR, 0x3, 0, 0x7, PORT_ID_EAG6L_PORT3, sys_fpga_memory_read, portRateSet }, 
  { COMMON_CTRL2_P4_ADDR, 0x3, 0, 0x7, PORT_ID_EAG6L_PORT4, sys_fpga_memory_read, portRateSet }, 
  { COMMON_CTRL2_P5_ADDR, 0x3, 0, 0x7, PORT_ID_EAG6L_PORT5, sys_fpga_memory_read, portRateSet }, 
  { COMMON_CTRL2_P6_ADDR, 0x3, 0, 0x7, PORT_ID_EAG6L_PORT6, sys_fpga_memory_read, portRateSet }, 
	/* port configuration - esmc */
  { PORT_1_CONF_ADDR,     0x4, 2, 0x0, PORT_ID_EAG6L_PORT1, sys_fpga_memory_read, port_config_ESMC_enable }, 
  { PORT_2_CONF_ADDR,     0x4, 2, 0x0, PORT_ID_EAG6L_PORT2, sys_fpga_memory_read, port_config_ESMC_enable }, 
  { PORT_3_CONF_ADDR,     0x4, 2, 0x0, PORT_ID_EAG6L_PORT3, sys_fpga_memory_read, port_config_ESMC_enable }, 
  { PORT_4_CONF_ADDR,     0x4, 2, 0x0, PORT_ID_EAG6L_PORT4, sys_fpga_memory_read, port_config_ESMC_enable }, 
  { PORT_5_CONF_ADDR,     0x4, 2, 0x0, PORT_ID_EAG6L_PORT5, sys_fpga_memory_read, port_config_ESMC_enable }, 
  { PORT_6_CONF_ADDR,     0x4, 2, 0x0, PORT_ID_EAG6L_PORT6, sys_fpga_memory_read, port_config_ESMC_enable }, 
  { PORT_7_CONF_ADDR,     0x4, 2, 0x0, PORT_ID_EAG6L_PORT7, sys_fpga_memory_read, port_config_ESMC_enable }, 
	/* port configuration - flex control */
  { PORT_1_CONF_ADDR,     0x4, 3, 0x0, PORT_ID_EAG6L_PORT1, sys_fpga_memory_read, set_flex_tune_control }, 
  { PORT_2_CONF_ADDR,     0x4, 3, 0x0, PORT_ID_EAG6L_PORT2, sys_fpga_memory_read, set_flex_tune_control }, 
  { PORT_3_CONF_ADDR,     0x4, 3, 0x0, PORT_ID_EAG6L_PORT3, sys_fpga_memory_read, set_flex_tune_control }, 
  { PORT_4_CONF_ADDR,     0x4, 3, 0x0, PORT_ID_EAG6L_PORT4, sys_fpga_memory_read, set_flex_tune_control }, 
  { PORT_5_CONF_ADDR,     0x4, 3, 0x0, PORT_ID_EAG6L_PORT5, sys_fpga_memory_read, set_flex_tune_control }, 
  { PORT_6_CONF_ADDR,     0x4, 3, 0x0, PORT_ID_EAG6L_PORT6, sys_fpga_memory_read, set_flex_tune_control }, 
  { PORT_7_CONF_ADDR,     0x4, 3, 0x0, PORT_ID_EAG6L_PORT7, sys_fpga_memory_read, set_flex_tune_control }, 
	/* port configuration - rtwdm loopback */
  { PORT_1_CONF_ADDR,     0x10, 4, 0x0, PORT_ID_EAG6L_PORT1, sys_fpga_memory_read, set_rtwdm_loopback }, 
  { PORT_2_CONF_ADDR,     0x10, 4, 0x0, PORT_ID_EAG6L_PORT2, sys_fpga_memory_read, set_rtwdm_loopback }, 
  { PORT_3_CONF_ADDR,     0x10, 4, 0x0, PORT_ID_EAG6L_PORT3, sys_fpga_memory_read, set_rtwdm_loopback }, 
  { PORT_4_CONF_ADDR,     0x10, 4, 0x0, PORT_ID_EAG6L_PORT4, sys_fpga_memory_read, set_rtwdm_loopback }, 
  { PORT_5_CONF_ADDR,     0x10, 4, 0x0, PORT_ID_EAG6L_PORT5, sys_fpga_memory_read, set_rtwdm_loopback }, 
  { PORT_6_CONF_ADDR,     0x10, 4, 0x0, PORT_ID_EAG6L_PORT6, sys_fpga_memory_read, set_rtwdm_loopback }, 
  { PORT_7_CONF_ADDR,     0x10, 4, 0x0, PORT_ID_EAG6L_PORT7, sys_fpga_memory_read, set_rtwdm_loopback }, 
	/* port configuration - smart t-sfp self loopback */
  { PORT_1_CONF_ADDR,     0x20, 5, 0x0, PORT_ID_EAG6L_PORT1, sys_fpga_memory_read, set_smart_tsfp_self_loopback }, 
  { PORT_2_CONF_ADDR,     0x20, 5, 0x0, PORT_ID_EAG6L_PORT2, sys_fpga_memory_read, set_smart_tsfp_self_loopback }, 
  { PORT_3_CONF_ADDR,     0x20, 5, 0x0, PORT_ID_EAG6L_PORT3, sys_fpga_memory_read, set_smart_tsfp_self_loopback }, 
  { PORT_4_CONF_ADDR,     0x20, 5, 0x0, PORT_ID_EAG6L_PORT4, sys_fpga_memory_read, set_smart_tsfp_self_loopback }, 
  { PORT_5_CONF_ADDR,     0x20, 5, 0x0, PORT_ID_EAG6L_PORT5, sys_fpga_memory_read, set_smart_tsfp_self_loopback }, 
  { PORT_6_CONF_ADDR,     0x20, 5, 0x0, PORT_ID_EAG6L_PORT6, sys_fpga_memory_read, set_smart_tsfp_self_loopback }, 
  { PORT_7_CONF_ADDR,     0x20, 5, 0x0, PORT_ID_EAG6L_PORT7, sys_fpga_memory_read, set_smart_tsfp_self_loopback }, 
	/* alarm mask - flex tune reset */
  { PORT_1_ALM_MASK_ADDR, 0x20, 5, 0x0, PORT_ID_EAG6L_PORT1, sys_fpga_memory_read, set_flex_tune_reset }, 
  { PORT_2_ALM_MASK_ADDR, 0x20, 5, 0x0, PORT_ID_EAG6L_PORT2, sys_fpga_memory_read, set_flex_tune_reset }, 
  { PORT_3_ALM_MASK_ADDR, 0x20, 5, 0x0, PORT_ID_EAG6L_PORT3, sys_fpga_memory_read, set_flex_tune_reset }, 
  { PORT_4_ALM_MASK_ADDR, 0x20, 5, 0x0, PORT_ID_EAG6L_PORT4, sys_fpga_memory_read, set_flex_tune_reset }, 
  { PORT_5_ALM_MASK_ADDR, 0x20, 5, 0x0, PORT_ID_EAG6L_PORT5, sys_fpga_memory_read, set_flex_tune_reset }, 
  { PORT_6_ALM_MASK_ADDR, 0x20, 5, 0x0, PORT_ID_EAG6L_PORT6, sys_fpga_memory_read, set_flex_tune_reset }, 
  { PORT_7_ALM_MASK_ADDR, 0x20, 5, 0x0, PORT_ID_EAG6L_PORT7, sys_fpga_memory_read, set_flex_tune_reset }, 
	/* synce global control */
  { SYNCE_GCONFIG_ADDR,   0xFF, 0, 0x5A, PORT_ID_EAG6L_NOT_USE, sys_fpga_memory_read, synceEnableSet }, 
	/* synce interface select */
  { SYNCE_IF_SELECT_ADDR, 0xFFFF, 0, 0x0, PORT_ID_EAG6L_NOT_USE, sys_fpga_memory_read, synceIFSelect }, 
	/* pm counter clear */
  { PM_COUNT_CLEAR_ADDR,  0xFF, 0, 0x0, PORT_ID_EAG6L_NOT_USE, sys_fpga_memory_read, pmClear }, 
	/* chip reset */
  { CHIP_RESET_ADDR,  0xFFFF, 0, 0x0, PORT_ID_EAG6L_NOT_USE, sys_fpga_memory_read, chipReset }, 
	/* fpga bank select */
	/* FIXME : add entry */
	/* dco register */
	/* FIXME : add entry */
};
#else
uint16_t portRateSet (uint16_t port);
extern uint16_t sys_fpga_memory_read(uint16_t addr);

RegMON regMonList [] = {
  { COMMON_CTRL2_P1_ADDR, 0x3, 0, 0x7, sys_fpga_memory_read, portRateSet/*register callback*/}, 
};
#endif
uint16_t regMonArrSize = sizeof(regMonList) / sizeof(RegMON);

#if 0/*[#51] Adding register callback templates for config/command registers, dustin, 2024-06-12 */
uint16_t portRateSet (uint16_t port)
{
	port = port;
	return 0;
}
#else /****************************************************/
uint16_t portRateSet (uint16_t port, uint16_t val)
{
extern void port_config_speed(int port, long speed, int mode);

	if(val == 0x7/*25G*/)
		port_config_speed(port, PORT_IF_25G_KR, PORT_IF_25G_KR);
	else
		port_config_speed(port, PORT_IF_10G_KR, PORT_IF_10G_KR);
	return SUCCESS;
}

uint16_t synceEnableSet(uint16_t port, uint16_t val)
{
	port;
	return synce_config_set_admin(val);
}

uint16_t synceIFSelect(uint16_t port, uint16_t val)
{
	uint16_t pri_port, sec_port;

	port;
	pri_port = (val >> 8) & 0xFF;
	sec_port = (val & 0xFF);

	return synce_config_set_if_select(pri_port, sec_port);
}

uint16_t pmClear(uint16_t port, uint16_t val)
{
extern void pm_request_clear(void);

	port;
	if(val != 0xA5)
		return SUCCESS;

	pm_request_clear();
	return SUCCESS;
}

uint16_t chipReset(uint16_t port, uint16_t val)
{
	port;

	if(((val >> 8) & 0xFF) == 0xA5)
		;/*FIXME : need reset function for fpga. */
	else if((val & 0xFF) == 0xA5)
		;/*FIXME : need reset function for bp. */
	return SUCCESS;
}
#endif

void regMonitor(void)
{
	uint16_t i, val;
	for(i = 0; i < regMonArrSize; i++)
	{
		val = regMonList[i].func(regMonList[i].reg); 
		val = (val >> regMonList[i].shift) & regMonList[i].mask;
		if(regMonList[i].val != val) 
		{
#if 1/*[#51] Adding register callback templates for config/command registers, dustin, 2024-06-12 */
			regMonList[i].cb(regMonList[i].portno, val);
#else
			regMonList[i].cb(val);
#endif
			regMonList[i].val = val;
		}
	}
}
#endif


unsigned long __COMMON_CTRL2_ADDR[PORT_ID_EAG6L_MAX] = { 0x0, COMMON_CTRL2_P1_ADDR,
		COMMON_CTRL2_P2_ADDR, COMMON_CTRL2_P3_ADDR, 
		COMMON_CTRL2_P4_ADDR, COMMON_CTRL2_P5_ADDR,
		COMMON_CTRL2_P6_ADDR, COMMON_CTRL2_P7_ADDR };
unsigned long __PORT_CONFIG_ADDR[PORT_ID_EAG6L_MAX] = { 0x0, PORT_1_CONF_ADDR,
		PORT_2_CONF_ADDR, PORT_3_CONF_ADDR, 
		PORT_4_CONF_ADDR, PORT_5_CONF_ADDR,
		PORT_6_CONF_ADDR, PORT_7_CONF_ADDR };
unsigned long __PORT_TX_PWR_ADDR[PORT_ID_EAG6L_MAX] = { 0x0, PORT_1_TX_PWR_ADDR,
		PORT_2_TX_PWR_ADDR, PORT_3_TX_PWR_ADDR, 
		PORT_4_TX_PWR_ADDR, PORT_5_TX_PWR_ADDR,
		PORT_6_TX_PWR_ADDR, PORT_7_TX_PWR_ADDR };
unsigned long __PORT_RX_PWR_ADDR[PORT_ID_EAG6L_MAX] = { 0x0, PORT_1_TX_PWR_ADDR,
		PORT_2_TX_PWR_ADDR, PORT_3_TX_PWR_ADDR, 
		PORT_4_TX_PWR_ADDR, PORT_5_TX_PWR_ADDR,
		PORT_6_TX_PWR_ADDR, PORT_7_TX_PWR_ADDR };
unsigned long __PORT_WL1_ADDR[PORT_ID_EAG6L_MAX] = { 0x0, PORT_1_WL1_ADDR,
		PORT_2_WL1_ADDR, PORT_3_WL1_ADDR, 
		PORT_4_WL1_ADDR, PORT_5_WL1_ADDR,
		PORT_6_WL1_ADDR, PORT_7_TX_PWR_ADDR };
unsigned long __PORT_WL2_ADDR[PORT_ID_EAG6L_MAX] = { 0x0, PORT_1_WL2_ADDR,
		PORT_2_WL2_ADDR, PORT_3_WL2_ADDR, 
		PORT_4_WL2_ADDR, PORT_5_WL2_ADDR,
		PORT_6_WL2_ADDR, PORT_7_WL2_ADDR };
unsigned long __PORT_DIST_ADDR[PORT_ID_EAG6L_MAX] = { 0x0, PORT_1_DIST_ADDR,
		PORT_2_DIST_ADDR, PORT_3_DIST_ADDR, 
		PORT_4_DIST_ADDR, PORT_5_DIST_ADDR,
		PORT_6_DIST_ADDR, PORT_7_WL2_ADDR };
unsigned long __PORT_STSFP_STAT_ADDR[PORT_ID_EAG6L_MAX] = { 0x0, PORT_1_STSFP_STAT_ADDR,
		PORT_2_STSFP_STAT_ADDR, PORT_3_STSFP_STAT_ADDR, 
		PORT_4_STSFP_STAT_ADDR, PORT_5_STSFP_STAT_ADDR,
		PORT_5_STSFP_STAT_ADDR, PORT_7_STSFP_STAT_ADDR };
unsigned long __PORT_ALM_ADDR[PORT_ID_EAG6L_MAX] = { 0x0, PORT_1_ALM_ADDR,
		PORT_2_ALM_ADDR, PORT_3_ALM_ADDR, 
		PORT_4_ALM_ADDR, PORT_5_ALM_ADDR,
		PORT_6_ALM_ADDR, PORT_7_ALM_ADDR };
unsigned long __PORT_ALM_FLAG_ADDR[PORT_ID_EAG6L_MAX] = { 0x0, PORT_1_ALM_FLAG_ADDR,
		PORT_2_ALM_FLAG_ADDR, PORT_3_ALM_FLAG_ADDR, 
		PORT_4_ALM_FLAG_ADDR, PORT_5_ALM_FLAG_ADDR,
		PORT_6_ALM_FLAG_ADDR, PORT_7_ALM_FLAG_ADDR };
unsigned long __PORT_ALM_MASK_ADDR[PORT_ID_EAG6L_MAX] = { 0x0, PORT_1_ALM_MASK_ADDR,
		PORT_2_ALM_MASK_ADDR, PORT_3_ALM_MASK_ADDR, 
		PORT_4_ALM_MASK_ADDR, PORT_5_ALM_MASK_ADDR,
		PORT_6_ALM_MASK_ADDR, PORT_7_ALM_MASK_ADDR };
unsigned long __PORT_GET_CH_NUM_ADDR[PORT_ID_EAG6L_MAX] = { 0x0, PORT_1_GET_CH_NUM_ADDR,
		PORT_2_GET_CH_NUM_ADDR, PORT_3_GET_CH_NUM_ADDR, 
		PORT_4_GET_CH_NUM_ADDR, PORT_5_GET_CH_NUM_ADDR,
		PORT_6_GET_CH_NUM_ADDR, PORT_7_GET_CH_NUM_ADDR };
unsigned long __PORT_SET_CH_NUM_ADDR[PORT_ID_EAG6L_MAX] = { 0x0, PORT_1_SET_CH_NUM_ADDR,
		PORT_2_SET_CH_NUM_ADDR, PORT_3_SET_CH_NUM_ADDR, 
		PORT_4_SET_CH_NUM_ADDR, PORT_5_SET_CH_NUM_ADDR,
		PORT_6_SET_CH_NUM_ADDR, PORT_7_SET_CH_NUM_ADDR };
unsigned long __PORT_VENDOR1_ADDR[PORT_ID_EAG6L_MAX] = { 0x0, SFP_P1_VENDOR1_ADDR,
		SFP_P2_VENDOR1_ADDR, SFP_P3_VENDOR1_ADDR, 
		SFP_P4_VENDOR1_ADDR, SFP_P5_VENDOR1_ADDR,
		SFP_P6_VENDOR1_ADDR, SFP_P7_VENDOR1_ADDR };
unsigned long __PORT_VENDOR2_ADDR[PORT_ID_EAG6L_MAX] = { 0x0, SFP_P1_VENDOR2_ADDR,
		SFP_P2_VENDOR2_ADDR, SFP_P3_VENDOR2_ADDR, 
		SFP_P4_VENDOR2_ADDR, SFP_P5_VENDOR2_ADDR,
		SFP_P6_VENDOR2_ADDR, SFP_P7_VENDOR2_ADDR };
unsigned long __PORT_VENDOR3_ADDR[PORT_ID_EAG6L_MAX] = { 0x0, SFP_P1_VENDOR3_ADDR,
		SFP_P2_VENDOR3_ADDR, SFP_P3_VENDOR3_ADDR, 
		SFP_P4_VENDOR3_ADDR, SFP_P5_VENDOR3_ADDR,
		SFP_P6_VENDOR3_ADDR, SFP_P7_VENDOR3_ADDR };
unsigned long __PORT_VENDOR4_ADDR[PORT_ID_EAG6L_MAX] = { 0x0, SFP_P1_VENDOR4_ADDR,
		SFP_P2_VENDOR4_ADDR, SFP_P3_VENDOR4_ADDR, 
		SFP_P4_VENDOR4_ADDR, SFP_P5_VENDOR4_ADDR,
		SFP_P6_VENDOR4_ADDR, SFP_P7_VENDOR4_ADDR };
unsigned long __PORT_VENDOR5_ADDR[PORT_ID_EAG6L_MAX] = { 0x0, SFP_P1_VENDOR5_ADDR,
		SFP_P2_VENDOR5_ADDR, SFP_P3_VENDOR5_ADDR, 
		SFP_P4_VENDOR5_ADDR, SFP_P5_VENDOR5_ADDR,
		SFP_P6_VENDOR5_ADDR, SFP_P7_VENDOR5_ADDR };
unsigned long __PORT_VENDOR6_ADDR[PORT_ID_EAG6L_MAX] = { 0x0, SFP_P1_VENDOR6_ADDR,
		SFP_P2_VENDOR6_ADDR, SFP_P3_VENDOR6_ADDR, 
		SFP_P4_VENDOR6_ADDR, SFP_P5_VENDOR6_ADDR,
		SFP_P6_VENDOR6_ADDR, SFP_P7_VENDOR6_ADDR };
unsigned long __PORT_VENDOR7_ADDR[PORT_ID_EAG6L_MAX] = { 0x0, SFP_P1_VENDOR7_ADDR,
		SFP_P2_VENDOR7_ADDR, SFP_P3_VENDOR7_ADDR, 
		SFP_P4_VENDOR7_ADDR, SFP_P5_VENDOR7_ADDR,
		SFP_P6_VENDOR7_ADDR, SFP_P7_VENDOR7_ADDR };
unsigned long __PORT_VENDOR8_ADDR[PORT_ID_EAG6L_MAX] = { 0x0, SFP_P1_VENDOR8_ADDR,
		SFP_P2_VENDOR8_ADDR, SFP_P3_VENDOR8_ADDR, 
		SFP_P4_VENDOR8_ADDR, SFP_P5_VENDOR8_ADDR,
		SFP_P6_VENDOR8_ADDR, SFP_P7_VENDOR8_ADDR };
unsigned long __PORT_PN1_ADDR[PORT_ID_EAG6L_MAX] = { 0x0, SFP_P1_PN1_ADDR,
		SFP_P2_PN1_ADDR, SFP_P3_PN1_ADDR, 
		SFP_P4_PN1_ADDR, SFP_P5_PN1_ADDR,
		SFP_P6_PN1_ADDR, SFP_P7_PN1_ADDR };
unsigned long __PORT_PN2_ADDR[PORT_ID_EAG6L_MAX] = { 0x0, SFP_P1_PN2_ADDR,
		SFP_P2_PN2_ADDR, SFP_P3_PN2_ADDR, 
		SFP_P4_PN2_ADDR, SFP_P5_PN2_ADDR,
		SFP_P6_PN2_ADDR, SFP_P7_PN2_ADDR };
unsigned long __PORT_PN3_ADDR[PORT_ID_EAG6L_MAX] = { 0x0, SFP_P1_PN3_ADDR,
		SFP_P2_PN3_ADDR, SFP_P3_PN3_ADDR, 
		SFP_P4_PN3_ADDR, SFP_P5_PN3_ADDR,
		SFP_P6_PN3_ADDR, SFP_P7_PN3_ADDR };
unsigned long __PORT_PN4_ADDR[PORT_ID_EAG6L_MAX] = { 0x0, SFP_P1_PN4_ADDR,
		SFP_P2_PN4_ADDR, SFP_P3_PN4_ADDR, 
		SFP_P4_PN4_ADDR, SFP_P5_PN4_ADDR,
		SFP_P6_PN4_ADDR, SFP_P7_PN4_ADDR };
unsigned long __PORT_PN5_ADDR[PORT_ID_EAG6L_MAX] = { 0x0, SFP_P1_PN5_ADDR,
		SFP_P2_PN5_ADDR, SFP_P3_PN5_ADDR, 
		SFP_P4_PN5_ADDR, SFP_P5_PN5_ADDR,
		SFP_P6_PN5_ADDR, SFP_P7_PN5_ADDR };
unsigned long __PORT_PN6_ADDR[PORT_ID_EAG6L_MAX] = { 0x0, SFP_P1_PN6_ADDR,
		SFP_P2_PN6_ADDR, SFP_P3_PN6_ADDR, 
		SFP_P4_PN6_ADDR, SFP_P5_PN6_ADDR,
		SFP_P6_PN6_ADDR, SFP_P7_PN6_ADDR };
unsigned long __PORT_PN7_ADDR[PORT_ID_EAG6L_MAX] = { 0x0, SFP_P1_PN7_ADDR,
		SFP_P2_PN7_ADDR, SFP_P3_PN7_ADDR, 
		SFP_P4_PN7_ADDR, SFP_P5_PN7_ADDR,
		SFP_P6_PN7_ADDR, SFP_P7_PN7_ADDR };
unsigned long __PORT_PN8_ADDR[PORT_ID_EAG6L_MAX] = { 0x0, SFP_P1_PN8_ADDR,
		SFP_P2_PN8_ADDR, SFP_P3_PN8_ADDR, 
		SFP_P4_PN8_ADDR, SFP_P5_PN8_ADDR,
		SFP_P6_PN8_ADDR, SFP_P7_PN8_ADDR };
unsigned long __PORT_SN1_ADDR[PORT_ID_EAG6L_MAX] = { 0x0, SFP_P1_SN1_ADDR,
		SFP_P2_SN1_ADDR, SFP_P3_SN1_ADDR, 
		SFP_P4_SN1_ADDR, SFP_P5_SN1_ADDR,
		SFP_P6_SN1_ADDR, SFP_P7_SN1_ADDR };
unsigned long __PORT_SN2_ADDR[PORT_ID_EAG6L_MAX] = { 0x0, SFP_P1_SN2_ADDR,
		SFP_P2_SN2_ADDR, SFP_P3_SN2_ADDR, 
		SFP_P4_SN2_ADDR, SFP_P5_SN2_ADDR,
		SFP_P6_SN2_ADDR, SFP_P7_SN2_ADDR };
unsigned long __PORT_SN3_ADDR[PORT_ID_EAG6L_MAX] = { 0x0, SFP_P1_SN3_ADDR,
		SFP_P2_SN3_ADDR, SFP_P3_SN3_ADDR, 
		SFP_P4_SN3_ADDR, SFP_P5_SN3_ADDR,
		SFP_P6_SN3_ADDR, SFP_P7_SN3_ADDR };
unsigned long __PORT_SN4_ADDR[PORT_ID_EAG6L_MAX] = { 0x0, SFP_P1_SN4_ADDR,
		SFP_P2_SN4_ADDR, SFP_P3_SN4_ADDR, 
		SFP_P4_SN4_ADDR, SFP_P5_SN4_ADDR,
		SFP_P6_SN4_ADDR, SFP_P7_SN4_ADDR };
unsigned long __PORT_SN5_ADDR[PORT_ID_EAG6L_MAX] = { 0x0, SFP_P1_SN5_ADDR,
		SFP_P2_SN5_ADDR, SFP_P3_SN5_ADDR, 
		SFP_P4_SN5_ADDR, SFP_P5_SN5_ADDR,
		SFP_P6_SN5_ADDR, SFP_P7_SN5_ADDR };
unsigned long __PORT_SN6_ADDR[PORT_ID_EAG6L_MAX] = { 0x0, SFP_P1_SN6_ADDR,
		SFP_P2_SN6_ADDR, SFP_P3_SN6_ADDR, 
		SFP_P4_SN6_ADDR, SFP_P5_SN6_ADDR,
		SFP_P6_SN6_ADDR, SFP_P7_SN6_ADDR };
unsigned long __PORT_SN7_ADDR[PORT_ID_EAG6L_MAX] = { 0x0, SFP_P1_SN7_ADDR,
		SFP_P2_SN7_ADDR, SFP_P3_SN7_ADDR, 
		SFP_P4_SN7_ADDR, SFP_P5_SN7_ADDR,
		SFP_P6_SN7_ADDR, SFP_P7_SN7_ADDR };
unsigned long __PORT_SN8_ADDR[PORT_ID_EAG6L_MAX] = { 0x0, SFP_P1_SN8_ADDR,
		SFP_P2_SN8_ADDR, SFP_P3_SN8_ADDR, 
		SFP_P4_SN8_ADDR, SFP_P5_SN8_ADDR,
		SFP_P6_SN8_ADDR, SFP_P7_SN8_ADDR };
unsigned long __PORT_TEMP_ADDR[PORT_ID_EAG6L_MAX] = { 0x0, SFP_P1_TEMP_ADDR,
		SFP_P2_TEMP_ADDR, SFP_P3_TEMP_ADDR, 
		SFP_P4_TEMP_ADDR, SFP_P5_TEMP_ADDR,
		SFP_P6_TEMP_ADDR, SFP_P7_TEMP_ADDR };
unsigned long __PORT_RATE_ADDR[PORT_ID_EAG6L_MAX] = { 0x0, SFP_P1_RATE_ADDR,
		SFP_P2_RATE_ADDR, SFP_P3_RATE_ADDR, 
		SFP_P4_RATE_ADDR, SFP_P5_RATE_ADDR,
		SFP_P6_RATE_ADDR, SFP_P7_RATE_ADDR };
unsigned long __PORT_VOLT_ADDR[PORT_ID_EAG6L_MAX] = { 0x0, SFP_P1_VOLT_ADDR,
		SFP_P2_VOLT_ADDR, SFP_P3_VOLT_ADDR, 
		SFP_P4_VOLT_ADDR, SFP_P5_VOLT_ADDR,
		SFP_P6_VOLT_ADDR, SFP_P7_VOLT_ADDR };
unsigned long __PORT_TX_BIAS_ADDR[PORT_ID_EAG6L_MAX] = { 0x0, SFP_P1_TX_BIAS_ADDR,
		SFP_P2_TX_BIAS_ADDR, SFP_P3_TX_BIAS_ADDR, 
		SFP_P4_TX_BIAS_ADDR, SFP_P5_TX_BIAS_ADDR,
		SFP_P6_TX_BIAS_ADDR, SFP_P7_TX_BIAS_ADDR };
unsigned long __PORT_LTEMP_ADDR[PORT_ID_EAG6L_MAX] = { 0x0, SFP_P1_LTEMP_ADDR,
		SFP_P2_LTEMP_ADDR, SFP_P3_LTEMP_ADDR, 
		SFP_P4_LTEMP_ADDR, SFP_P5_LTEMP_ADDR,
		SFP_P6_LTEMP_ADDR, SFP_P7_LTEMP_ADDR };
unsigned long __PORT_TCURR_ADDR[PORT_ID_EAG6L_MAX] = { 0x0, SFP_P1_TCURR_ADDR,
		SFP_P2_TCURR_ADDR, SFP_P3_TCURR_ADDR, 
		SFP_P4_TCURR_ADDR, SFP_P5_TCURR_ADDR,
		SFP_P6_TCURR_ADDR, SFP_P7_TCURR_ADDR };
unsigned long __PORT_PM_TX_BYTE1_ADDR[PORT_ID_EAG6L_MAX] = { 0x0, PM_P1_TX_BYTE1_ADDR,
		PM_P2_TX_BYTE1_ADDR, PM_P3_TX_BYTE1_ADDR, 
		PM_P4_TX_BYTE1_ADDR, PM_P5_TX_BYTE1_ADDR,
		PM_P6_TX_BYTE1_ADDR, PM_P7_TX_BYTE1_ADDR };
unsigned long __PORT_PM_TX_BYTE2_ADDR[PORT_ID_EAG6L_MAX] = { 0x0, PM_P1_TX_BYTE2_ADDR,
		PM_P2_TX_BYTE2_ADDR, PM_P3_TX_BYTE2_ADDR, 
		PM_P4_TX_BYTE2_ADDR, PM_P5_TX_BYTE2_ADDR,
		PM_P6_TX_BYTE2_ADDR, PM_P7_TX_BYTE2_ADDR };
unsigned long __PORT_PM_TX_BYTE3_ADDR[PORT_ID_EAG6L_MAX] = { 0x0, PM_P1_TX_BYTE3_ADDR,
		PM_P2_TX_BYTE3_ADDR, PM_P3_TX_BYTE3_ADDR, 
		PM_P4_TX_BYTE3_ADDR, PM_P5_TX_BYTE3_ADDR,
		PM_P6_TX_BYTE3_ADDR, PM_P7_TX_BYTE3_ADDR };
unsigned long __PORT_PM_TX_BYTE4_ADDR[PORT_ID_EAG6L_MAX] = { 0x0, PM_P1_TX_BYTE4_ADDR,
		PM_P2_TX_BYTE4_ADDR, PM_P3_TX_BYTE4_ADDR, 
		PM_P4_TX_BYTE4_ADDR, PM_P5_TX_BYTE4_ADDR,
		PM_P6_TX_BYTE4_ADDR, PM_P7_TX_BYTE4_ADDR };
unsigned long __PORT_PM_RX_BYTE1_ADDR[PORT_ID_EAG6L_MAX] = { 0x0, PM_P1_RX_BYTE1_ADDR,
		PM_P2_RX_BYTE1_ADDR, PM_P3_RX_BYTE1_ADDR, 
		PM_P4_RX_BYTE1_ADDR, PM_P5_RX_BYTE1_ADDR,
		PM_P6_RX_BYTE1_ADDR, PM_P7_RX_BYTE1_ADDR };
unsigned long __PORT_PM_RX_BYTE2_ADDR[PORT_ID_EAG6L_MAX] = { 0x0, PM_P1_RX_BYTE2_ADDR,
		PM_P2_RX_BYTE2_ADDR, PM_P3_RX_BYTE2_ADDR, 
		PM_P4_RX_BYTE2_ADDR, PM_P5_RX_BYTE2_ADDR,
		PM_P6_RX_BYTE2_ADDR, PM_P7_RX_BYTE2_ADDR };
unsigned long __PORT_PM_RX_BYTE3_ADDR[PORT_ID_EAG6L_MAX] = { 0x0, PM_P1_RX_BYTE3_ADDR,
		PM_P2_RX_BYTE3_ADDR, PM_P3_RX_BYTE3_ADDR, 
		PM_P4_RX_BYTE3_ADDR, PM_P5_RX_BYTE3_ADDR,
		PM_P6_RX_BYTE3_ADDR, PM_P7_RX_BYTE3_ADDR };
unsigned long __PORT_PM_RX_BYTE4_ADDR[PORT_ID_EAG6L_MAX] = { 0x0, PM_P1_RX_BYTE4_ADDR,
		PM_P2_RX_BYTE4_ADDR, PM_P3_RX_BYTE4_ADDR, 
		PM_P4_RX_BYTE4_ADDR, PM_P5_RX_BYTE4_ADDR,
		PM_P6_RX_BYTE4_ADDR, PM_P7_RX_BYTE4_ADDR };
unsigned long __PORT_PM_TX_FRAME1_ADDR[PORT_ID_EAG6L_MAX] = { 0x0, PM_P1_TX_FRAME1_ADDR,
		PM_P2_TX_FRAME1_ADDR, PM_P3_TX_FRAME1_ADDR, 
		PM_P4_TX_FRAME1_ADDR, PM_P5_TX_FRAME1_ADDR,
		PM_P6_TX_FRAME1_ADDR, PM_P7_TX_FRAME1_ADDR };
unsigned long __PORT_PM_TX_FRAME2_ADDR[PORT_ID_EAG6L_MAX] = { 0x0, PM_P1_TX_FRAME2_ADDR,
		PM_P2_TX_FRAME2_ADDR, PM_P3_TX_FRAME2_ADDR, 
		PM_P4_TX_FRAME2_ADDR, PM_P5_TX_FRAME2_ADDR,
		PM_P6_TX_FRAME2_ADDR, PM_P7_TX_FRAME2_ADDR };
unsigned long __PORT_PM_TX_FRAME3_ADDR[PORT_ID_EAG6L_MAX] = { 0x0, PM_P1_TX_FRAME3_ADDR,
		PM_P2_TX_FRAME3_ADDR, PM_P3_TX_FRAME3_ADDR, 
		PM_P4_TX_FRAME3_ADDR, PM_P5_TX_FRAME3_ADDR,
		PM_P6_TX_FRAME3_ADDR, PM_P7_TX_FRAME3_ADDR };
unsigned long __PORT_PM_TX_FRAME4_ADDR[PORT_ID_EAG6L_MAX] = { 0x0, PM_P1_TX_FRAME4_ADDR,
		PM_P2_TX_FRAME4_ADDR, PM_P3_TX_FRAME4_ADDR, 
		PM_P4_TX_FRAME4_ADDR, PM_P5_TX_FRAME4_ADDR,
		PM_P6_TX_FRAME4_ADDR, PM_P7_TX_FRAME4_ADDR };
unsigned long __PORT_PM_RX_FRAME1_ADDR[PORT_ID_EAG6L_MAX] = { 0x0, PM_P1_RX_FRAME1_ADDR,
		PM_P2_RX_FRAME1_ADDR, PM_P3_RX_FRAME1_ADDR, 
		PM_P4_RX_FRAME1_ADDR, PM_P5_RX_FRAME1_ADDR,
		PM_P6_RX_FRAME1_ADDR, PM_P7_RX_FRAME1_ADDR };
unsigned long __PORT_PM_RX_FRAME2_ADDR[PORT_ID_EAG6L_MAX] = { 0x0, PM_P1_RX_FRAME2_ADDR,
		PM_P2_RX_FRAME2_ADDR, PM_P3_RX_FRAME2_ADDR, 
		PM_P4_RX_FRAME2_ADDR, PM_P5_RX_FRAME2_ADDR,
		PM_P6_RX_FRAME2_ADDR, PM_P7_RX_FRAME2_ADDR };
unsigned long __PORT_PM_RX_FRAME3_ADDR[PORT_ID_EAG6L_MAX] = { 0x0, PM_P1_RX_FRAME3_ADDR,
		PM_P2_RX_FRAME3_ADDR, PM_P3_RX_FRAME3_ADDR, 
		PM_P4_RX_FRAME3_ADDR, PM_P5_RX_FRAME3_ADDR,
		PM_P6_RX_FRAME3_ADDR, PM_P7_RX_FRAME3_ADDR };
unsigned long __PORT_PM_RX_FRAME4_ADDR[PORT_ID_EAG6L_MAX] = { 0x0, PM_P1_RX_FRAME4_ADDR,
		PM_P2_RX_FRAME4_ADDR, PM_P3_RX_FRAME4_ADDR, 
		PM_P4_RX_FRAME4_ADDR, PM_P5_RX_FRAME4_ADDR,
		PM_P6_RX_FRAME4_ADDR, PM_P7_RX_FRAME4_ADDR };
unsigned long __PORT_PM_FCS1_ADDR[PORT_ID_EAG6L_MAX] = { 0x0, PM_P1_FCS1_ADDR,
		PM_P2_FCS1_ADDR, PM_P3_FCS1_ADDR, 
		PM_P4_FCS1_ADDR, PM_P5_FCS1_ADDR,
		PM_P6_FCS1_ADDR, PM_P7_FCS1_ADDR };
unsigned long __PORT_PM_FCS2_ADDR[PORT_ID_EAG6L_MAX] = { 0x0, PM_P1_FCS2_ADDR,
		PM_P2_FCS2_ADDR, PM_P3_FCS2_ADDR, 
		PM_P4_FCS2_ADDR, PM_P5_FCS2_ADDR,
		PM_P6_FCS2_ADDR, PM_P7_FCS2_ADDR };
unsigned long __PORT_PM_FCS3_ADDR[PORT_ID_EAG6L_MAX] = { 0x0, PM_P1_FCS3_ADDR,
		PM_P2_FCS3_ADDR, PM_P3_FCS3_ADDR, 
		PM_P4_FCS3_ADDR, PM_P5_FCS3_ADDR,
		PM_P6_FCS3_ADDR, PM_P7_FCS3_ADDR };
unsigned long __PORT_PM_FCS4_ADDR[PORT_ID_EAG6L_MAX] = { 0x0, PM_P1_FCS4_ADDR,
		PM_P2_FCS4_ADDR, PM_P3_FCS4_ADDR, 
		PM_P4_FCS4_ADDR, PM_P5_FCS4_ADDR,
		PM_P6_FCS4_ADDR, PM_P7_FCS4_ADDR };
unsigned long __PORT_PM_FCS_OK1_ADDR[PORT_ID_EAG6L_MAX] = { 0x0, PM_P1_FCS_OK1_ADDR,
		PM_P2_FCS_OK1_ADDR, PM_P3_FCS_OK1_ADDR, 
		PM_P4_FCS_OK1_ADDR, PM_P5_FCS_OK1_ADDR,
		PM_P6_FCS_OK1_ADDR, PM_P7_FCS_OK1_ADDR };
unsigned long __PORT_PM_FCS_OK2_ADDR[PORT_ID_EAG6L_MAX] = { 0x0, PM_P1_FCS_OK2_ADDR,
		PM_P2_FCS_OK2_ADDR, PM_P3_FCS_OK2_ADDR, 
		PM_P4_FCS_OK2_ADDR, PM_P5_FCS_OK2_ADDR,
		PM_P6_FCS_OK2_ADDR, PM_P7_FCS_OK2_ADDR };
unsigned long __PORT_PM_FCS_OK3_ADDR[PORT_ID_EAG6L_MAX] = { 0x0, PM_P1_FCS_OK3_ADDR,
		PM_P2_FCS_OK3_ADDR, PM_P3_FCS_OK3_ADDR, 
		PM_P4_FCS_OK3_ADDR, PM_P5_FCS_OK3_ADDR,
		PM_P6_FCS_OK3_ADDR, PM_P7_FCS_OK3_ADDR };
unsigned long __PORT_PM_FCS_OK4_ADDR[PORT_ID_EAG6L_MAX] = { 0x0, PM_P1_FCS_OK4_ADDR,
		PM_P2_FCS_OK4_ADDR, PM_P3_FCS_OK4_ADDR, 
		PM_P4_FCS_OK4_ADDR, PM_P5_FCS_OK4_ADDR,
		PM_P6_FCS_OK4_ADDR, PM_P7_FCS4_ADDR };
unsigned long __PORT_PM_FCS_NOK1_ADDR[PORT_ID_EAG6L_MAX] = { 0x0, PM_P1_FCS_NOK1_ADDR,
		PM_P2_FCS_NOK1_ADDR, PM_P3_FCS_NOK1_ADDR, 
		PM_P4_FCS_NOK1_ADDR, PM_P5_FCS_NOK1_ADDR,
		PM_P6_FCS_NOK1_ADDR, PM_P7_FCS_NOK1_ADDR };
unsigned long __PORT_PM_FCS_NOK2_ADDR[PORT_ID_EAG6L_MAX] = { 0x0, PM_P1_FCS_NOK2_ADDR,
		PM_P2_FCS_NOK2_ADDR, PM_P3_FCS_NOK2_ADDR, 
		PM_P4_FCS_NOK2_ADDR, PM_P5_FCS_NOK2_ADDR,
		PM_P6_FCS_NOK2_ADDR, PM_P7_FCS_NOK2_ADDR };
unsigned long __PORT_PM_FCS_NOK3_ADDR[PORT_ID_EAG6L_MAX] = { 0x0, PM_P1_FCS_NOK3_ADDR,
		PM_P2_FCS_NOK3_ADDR, PM_P3_FCS_NOK3_ADDR, 
		PM_P4_FCS_NOK3_ADDR, PM_P5_FCS_NOK3_ADDR,
		PM_P6_FCS_NOK3_ADDR, PM_P7_FCS_NOK3_ADDR };
unsigned long __PORT_PM_FCS_NOK4_ADDR[PORT_ID_EAG6L_MAX] = { 0x0, PM_P1_FCS_NOK4_ADDR,
		PM_P2_FCS_NOK4_ADDR, PM_P3_FCS_NOK4_ADDR, 
		PM_P4_FCS_NOK4_ADDR, PM_P5_FCS_NOK4_ADDR,
		PM_P6_FCS_NOK4_ADDR, PM_P7_FCS4_ADDR };
unsigned long __PORT_CLEI1_ADDR[PORT_ID_EAG6L_MAX] = { 0x0, PORT1_CLEI1_ADDR,
		PORT2_CLEI1_ADDR, PORT3_CLEI1_ADDR, 
		PORT4_CLEI1_ADDR, PORT5_CLEI1_ADDR,
		PORT6_CLEI1_ADDR, PORT7_CLEI1_ADDR };
unsigned long __PORT_CLEI2_ADDR[PORT_ID_EAG6L_MAX] = { 0x0, PORT1_CLEI2_ADDR,
		PORT2_CLEI2_ADDR, PORT3_CLEI2_ADDR, 
		PORT4_CLEI2_ADDR, PORT5_CLEI2_ADDR,
		PORT6_CLEI2_ADDR, PORT7_CLEI2_ADDR };
unsigned long __PORT_CLEI3_ADDR[PORT_ID_EAG6L_MAX] = { 0x0, PORT1_CLEI3_ADDR,
		PORT2_CLEI3_ADDR, PORT3_CLEI3_ADDR, 
		PORT4_CLEI3_ADDR, PORT5_CLEI3_ADDR,
		PORT6_CLEI3_ADDR, PORT7_CLEI3_ADDR };
unsigned long __PORT_CLEI4_ADDR[PORT_ID_EAG6L_MAX] = { 0x0, PORT1_CLEI4_ADDR,
		PORT2_CLEI4_ADDR, PORT3_CLEI4_ADDR, 
		PORT4_CLEI4_ADDR, PORT5_CLEI4_ADDR,
		PORT6_CLEI4_ADDR, PORT7_CLEI4_ADDR };
unsigned long __PORT_CLEI5_ADDR[PORT_ID_EAG6L_MAX] = { 0x0, PORT1_CLEI5_ADDR,
		PORT2_CLEI5_ADDR, PORT3_CLEI5_ADDR, 
		PORT4_CLEI5_ADDR, PORT5_CLEI5_ADDR,
		PORT6_CLEI5_ADDR, PORT7_CLEI5_ADDR };
unsigned long __PORT_USI1_ADDR[PORT_ID_EAG6L_MAX] = { 0x0, PORT1_USI1_ADDR,
		PORT2_USI1_ADDR, PORT3_USI1_ADDR, 
		PORT4_USI1_ADDR, PORT5_USI1_ADDR,
		PORT6_USI1_ADDR, PORT7_USI1_ADDR };
unsigned long __PORT_USI2_ADDR[PORT_ID_EAG6L_MAX] = { 0x0, PORT1_USI2_ADDR,
		PORT2_USI2_ADDR, PORT3_USI2_ADDR, 
		PORT4_USI2_ADDR, PORT5_USI2_ADDR,
		PORT6_USI2_ADDR, PORT7_USI2_ADDR };
unsigned long __PORT_USI3_ADDR[PORT_ID_EAG6L_MAX] = { 0x0, PORT1_USI3_ADDR,
		PORT2_USI3_ADDR, PORT3_USI3_ADDR, 
		PORT4_USI3_ADDR, PORT5_USI3_ADDR,
		PORT6_USI3_ADDR, PORT7_USI3_ADDR };
unsigned long __PORT_USI4_ADDR[PORT_ID_EAG6L_MAX] = { 0x0, PORT1_USI4_ADDR,
		PORT2_USI4_ADDR, PORT3_USI4_ADDR, 
		PORT4_USI4_ADDR, PORT5_USI4_ADDR,
		PORT6_USI4_ADDR, PORT7_USI4_ADDR };
unsigned long __PORT_USI5_ADDR[PORT_ID_EAG6L_MAX] = { 0x0, PORT1_USI5_ADDR,
		PORT2_USI5_ADDR, PORT3_USI5_ADDR, 
		PORT4_USI5_ADDR, PORT5_USI5_ADDR,
		PORT6_USI5_ADDR, PORT7_USI5_ADDR };
unsigned long __PORT_USI6_ADDR[PORT_ID_EAG6L_MAX] = { 0x0, PORT1_USI6_ADDR,
		PORT2_USI6_ADDR, PORT3_USI6_ADDR, 
		PORT4_USI6_ADDR, PORT5_USI6_ADDR,
		PORT6_USI6_ADDR, PORT7_USI6_ADDR };
unsigned long __PORT_USI7_ADDR[PORT_ID_EAG6L_MAX] = { 0x0, PORT1_USI7_ADDR,
		PORT2_USI7_ADDR, PORT3_USI7_ADDR, 
		PORT4_USI7_ADDR, PORT5_USI7_ADDR,
		PORT6_USI7_ADDR, PORT7_USI7_ADDR };
unsigned long __PORT_USI8_ADDR[PORT_ID_EAG6L_MAX] = { 0x0, PORT1_USI8_ADDR,
		PORT2_USI8_ADDR, PORT3_USI8_ADDR, 
		PORT4_USI8_ADDR, PORT5_USI8_ADDR,
		PORT6_USI8_ADDR, PORT7_USI8_ADDR };
unsigned long __PORT_USI9_ADDR[PORT_ID_EAG6L_MAX] = { 0x0, PORT1_USI9_ADDR,
		PORT2_USI9_ADDR, PORT3_USI9_ADDR, 
		PORT4_USI9_ADDR, PORT5_USI9_ADDR,
		PORT6_USI9_ADDR, PORT7_USI9_ADDR };
unsigned long __PORT_USI10_ADDR[PORT_ID_EAG6L_MAX] = { 0x0, PORT1_USI10_ADDR,
		PORT2_USI10_ADDR, PORT3_USI10_ADDR, 
		PORT4_USI10_ADDR, PORT5_USI10_ADDR,
		PORT6_USI10_ADDR, PORT7_USI10_ADDR };
unsigned long __PORT_USI11_ADDR[PORT_ID_EAG6L_MAX] = { 0x0, PORT1_USI11_ADDR,
		PORT2_USI11_ADDR, PORT3_USI11_ADDR, 
		PORT4_USI11_ADDR, PORT5_USI11_ADDR,
		PORT6_USI11_ADDR, PORT7_USI11_ADDR };
unsigned long __PORT_USI12_ADDR[PORT_ID_EAG6L_MAX] = { 0x0, PORT1_USI12_ADDR,
		PORT2_USI12_ADDR, PORT3_USI12_ADDR, 
		PORT4_USI12_ADDR, PORT5_USI12_ADDR,
		PORT6_USI12_ADDR, PORT7_USI12_ADDR };
unsigned long __PORT_USI13_ADDR[PORT_ID_EAG6L_MAX] = { 0x0, PORT1_USI13_ADDR,
		PORT2_USI13_ADDR, PORT3_USI13_ADDR, 
		PORT4_USI13_ADDR, PORT5_USI13_ADDR,
		PORT6_USI13_ADDR, PORT7_USI13_ADDR };


long hw_update_port_inventory(int portno, struct module_inventory * mod_inv)
{
#if 0//PWY_FIXME
    if('\0' == mod_inv->serial_num[0] || mod_inv->dist == 0xFFFF)
#endif
        get_sfp_info(portno, mod_inv);

    return SUCCESS;
}

unsigned long get_port_sfp_cr(unsigned long portId)
{
    u16 hw_val, equip;
    if(portId <= PORT_ID_EAG6L_PORT2) //RU
    {
        hw_val = FPGA_READ(BD_SFP_CR_ADDR);
        if((hw_val >> (portId -1)) & 0x1)
            equip = 1;
        else
            equip = 0;
    }
    else
    {
        hw_val = FPGA_READ(BD_SFP_CR_ADDR);
        if((hw_val >> (portId - 1)) & 0x1)
            equip = 1;
        else
            equip = 0;
    }

    return equip;
}

unsigned long get_port_sfp_reg(unsigned int addr, unsigned long type, unsigned long portId)
{
    u16 hw_val, sts;

    hw_val = FPGA_READ(addr);
	sts = (hw_val & type) ? 1 : 0;
		
    zlog_notice("get_port_sfp_reg : port=%d(0/%d) addr=%x type=%x, sts=%x", 
		portId, get_eag6L_dport(portId, 0), addr, type, sts);
    return sts;
}

void calc_pm_1min(f32 *avg, f32 *min, f32 *max, f32 *base)
{
	f32 total,  tmin, tmax;
	u32 idx, cnt;

	// calc avg rx power
	for(idx = 0, cnt =0, total=0; idx < 60; idx++, base++)
	{
		if(idx == 0) tmin = tmax = *base;

		if((*base != DEF_PWM_MIN) && (*base != (0.0)) && (*base != (-0.0)))
		{
			total += *base;
			cnt++;

			if(tmin > *base) tmin = *base;
			if(tmax < *base) tmax = *base;
		}
	}

	if(cnt) *avg = total / cnt;
	else *avg = DEF_PWM_MIN;

	*min = tmin; *max = tmax;
}
    
static f32 port_pwr_1min[PORT_ID_EAG6L_MAX-1][2][60];
static u32 port_pwr_index[PORT_ID_EAG6L_MAX-1] = {0};
static u32 is_init_port_pwr[PORT_ID_EAG6L_MAX-1] = {0};

void update_port_rx_power(int portno)
{
    u32 idx, p_idx, nidx;

    p_idx = (portno - 1)%(PORT_ID_EAG6L_MAX-1);
    port_pwr_index[p_idx]++;
    port_pwr_index[p_idx] %= 60;
    nidx = port_pwr_index[p_idx];

    if(!is_init_port_pwr[p_idx])
    {
        for(idx = 0; idx < 60; idx++)
        {
            port_pwr_1min[p_idx][0][idx] = PORT_STATUS[portno].rx_pwr;
            port_pwr_1min[p_idx][1][idx] = PORT_STATUS[portno].tx_pwr;
        }
        PORT_STATUS[portno].rx_min = PORT_STATUS[portno].rx_pwr;
        PORT_STATUS[portno].rx_max = PORT_STATUS[portno].rx_pwr;
        PORT_STATUS[portno].rx_avg = PORT_STATUS[portno].rx_pwr;

        PORT_STATUS[portno].tx_min = PORT_STATUS[portno].tx_pwr;
        PORT_STATUS[portno].tx_max = PORT_STATUS[portno].tx_pwr;
        PORT_STATUS[portno].tx_avg = PORT_STATUS[portno].tx_pwr;

        is_init_port_pwr[p_idx] = 1;
        port_pwr_index[p_idx] = 0;

        port_pwr_1min[p_idx][0][0] = PORT_STATUS[portno].rx_pwr;
        port_pwr_1min[p_idx][1][0] = PORT_STATUS[portno].tx_pwr;

        return ;
    }

    port_pwr_1min[p_idx][0][nidx] = PORT_STATUS[portno].rx_pwr;
    port_pwr_1min[p_idx][1][nidx] = PORT_STATUS[portno].tx_pwr;

#if 0
	if(PORT_STATUS[portno].rx_pwr != DEF_PWM_MIN)
	{
		// calc min rx,tx power
		if(PORT_STATUS[portno].rx_min > PORT_STATUS[portno].rx_pwr)
			PORT_STATUS[portno].rx_min = PORT_STATUS[portno].rx_pwr;
		// calc max rx power
		if(PORT_STATUS[portno].rx_max < PORT_STATUS[portno].rx_pwr)
			PORT_STATUS[portno].rx_max = PORT_STATUS[portno].rx_pwr;
	} else
		PORT_STATUS[portno].rx_min = PORT_STATUS[portno].rx_pwr;


	if(PORT_STATUS[portno].tx_pwr != DEF_PWM_MIN)
	{
		if(PORT_STATUS[portno].tx_min > PORT_STATUS[portno].tx_pwr)
			PORT_STATUS[portno].tx_min = PORT_STATUS[portno].tx_pwr;
		if(PORT_STATUS[portno].tx_max < PORT_STATUS[portno].tx_pwr)
			PORT_STATUS[portno].tx_max = PORT_STATUS[portno].tx_pwr;
	} else
		PORT_STATUS[portno].tx_min = PORT_STATUS[portno].tx_pwr;
#endif

    calc_pm_1min(&PORT_STATUS[portno].rx_avg, &PORT_STATUS[portno].rx_min, &PORT_STATUS[portno].rx_max, port_pwr_1min[p_idx][0]);
    calc_pm_1min(&PORT_STATUS[portno].tx_avg, &PORT_STATUS[portno].tx_min, &PORT_STATUS[portno].tx_max, port_pwr_1min[p_idx][1]);

#ifdef DEBUG
	zlog_notice(" PORT[%d(0/%d)] %d cwhan_check %4.2f [%4.2f-%4.2f-%4.2f] [%4.2f-%4.2f-%4.2f] \n",
		portno, get_eag6L_dport(portno, 0), nidx, PORT_STATUS[portno].tx_pwr,
		PORT_STATUS[portno].tx_avg, PORT_STATUS[portno].tx_min, PORT_STATUS[portno].tx_max,
		port_pwr_1min[p_idx][1][nidx], port_pwr_1min[p_idx][1][nidx +1],
		port_pwr_1min[p_idx][1][nidx +2]);
#endif
	return;
}

void update_sfp(void)
{
	int portno;

	for(portno = PORT_ID_EAG6L_PORT1; portno < PORT_ID_EAG6L_MAX; portno++) {
		/* skip if port has not installed sfp. */
		/* FIXME */
zlog_notice(" update_sfp PORT[%d(0/%d)]", portno, get_eag6L_dport(portno, 0));/*ZZPP*/
		get_sfp_info_diag(portno, &(PORT_STATUS[portno]));
		update_port_rx_power(portno);
		hw_update_port_inventory(portno, &(INV_TBL[portno]));
	}
	return;
}


u32 MCU_INIT_COMPLETE_FLAG;
u32 MCU_KEEP_ALIVE_RETRY_COUNT;

#define MCU_KEEP_ALIVE_RETRY_MAX	15

// convert to decimal format value from float dbm value
// float -17.5 will be presented as -175 in integer.
unsigned int convert_dbm_float_to_decimal(f32 val, int dbm_flag)
{
	unsigned int unit_10, unit_1, unit_0, vvv, minus_flag;
	f32 temp;

	if(dbm_flag) {
		if(val > 10.0)
			return 0x100;
		if(val < -15.0)
			return 0x8150;
	}

	// save original value and minus flag and turn into positive.
	minus_flag = (val < 0) ? 1 : 0;
	if(minus_flag)
		temp = 0 - val;
	else
		temp = val;
	unit_10 = (int)(temp / 10);
	temp = temp - (int)(unit_10 * 10);
	unit_1 = (int)(temp / 1);
	temp = temp - (int)(unit_1 * 1);
	unit_0 = (int)temp;

	vvv = 0;
	if(minus_flag || (val == 0))
		vvv |= (1 << 15);
	vvv |= ((unit_10 << 8) & 0xf00);
	vvv |= ((unit_1 << 4) & 0xf0);
	vvv |= unit_0;
	return vvv;
}

unsigned int convert_temperature_float_to_decimal(f32 val)
{
    unsigned int unit_100, unit_10, unit_1, unit_0, vvv, minus_flag;
    f32 temp;

	// save original value and minus flag and turn into positive.
    minus_flag = (val < 0) ? 1 : 0;
    if(minus_flag)
        temp = 0 - val;
    else
        temp = val;
	unit_100 = (int)(temp / 100);
	temp = temp - (int)(unit_100 * 100);
    unit_10 = (int)(temp / 10);
    temp = temp - (int)(unit_10 * 10);
    unit_1 = (int)(temp / 1);
    temp = temp - (int)(unit_1 * 1);
    unit_0 = (int)temp;

    vvv = 0;
    if(minus_flag || (val == 0))
        vvv |= (1 << 15);
    vvv |= ((unit_100 << 12) & 0x700);
    vvv |= ((unit_10 << 8) & 0xf00);
    vvv |= ((unit_1 << 4) & 0xf0);
    vvv |= unit_0;
    return vvv;
}

#if 0/*[#51] Adding register callback templates for config/command registers, dustin, 2024-06-12 */
void process_port_common_control_register(void)
{
extern void port_config_speed(int port, long speed, int mode);

	int portno;
	unsigned int val;

	for(portno = PORT_ID_EAG6L_PORT1; portno < PORT_ID_EAG6L_MAX; portno++) {
		if(portno != PORT_ID_EAG6L_PORT7) {
			val = FPGA_READ(__COMMON_CTRL2_ADDR[portno]);
			if(val != PORT_COMMON_CONTROL2_CACHE[portno]) {
				/* configure port rate from marvell sdk side */
				if((val & 0x7) != (PORT_COMMON_CONTROL2_CACHE[portno] & 0x7)) {
					if((val & 0x7) == 0x7)
						port_config_speed(portno, PORT_IF_25G_KR, PORT_IF_25G_KR);
					else
						port_config_speed(portno, PORT_IF_10G_KR, PORT_IF_10G_KR);
				}
				PORT_COMMON_CONTROL2_CACHE[portno] = val;
			}
		}
	}
	return;
}
#endif

void process_dco_registers(void)
{
/*FIXME*/
}

void process_hw_inventory_infos(void)
{
	/*FIXME : should replaced with real inv data in flash. */
	struct inventory inv = { "hfrnet1234", "EAG6L12345", "PN123456789ABCD", 
		                     "SN12", "RV12", "240520010203", "2405200102", "RP01",
		                     "SNUIEAG6LL", "" };
	unsigned int val[20];
	char buff[32];

	/* update manufacture. */
	val[0] = inv.manufact[0] | inv.manufact[1];
	FPGA_WRITE(INV_HW_MANU_1_ADDR, val[0]);
	val[1] = inv.manufact[2] | inv.manufact[3];
	FPGA_WRITE(INV_HW_MANU_2_ADDR, val[1]);
	val[2] = inv.manufact[4] | inv.manufact[5];
	FPGA_WRITE(INV_HW_MANU_3_ADDR, val[2]);
	val[3] = inv.manufact[6] | inv.manufact[7];
	FPGA_WRITE(INV_HW_MANU_4_ADDR, val[3]);
	val[4] = inv.manufact[8] | inv.manufact[9];
	FPGA_WRITE(INV_HW_MANU_5_ADDR, val[4]);

#ifdef DEBUG
	val[0] = FPGA_READ(INV_HW_MANU_1_ADDR);
	val[1] = FPGA_READ(INV_HW_MANU_2_ADDR);
	val[2] = FPGA_READ(INV_HW_MANU_3_ADDR);
	val[3] = FPGA_READ(INV_HW_MANU_4_ADDR);
	val[4] = FPGA_READ(INV_HW_MANU_5_ADDR);

	memset(buff, 0, sizeof buff);
	buff[0] = val[0] & 0xFF;
	buff[1] = (val[0] >> 8) & 0xFF;
	buff[2] = val[1] & 0xFF;
	buff[3] = (val[1] >> 8) & 0xFF;
	buff[4] = val[2] & 0xFF;
	buff[5] = (val[2] >> 8) & 0xFF;
	buff[6] = val[3] & 0xFF;
	buff[7] = (val[3] >> 8) & 0xFF;
	buff[8] = val[4] & 0xFF;
	buff[9] = (val[4] >> 8) & 0xFF;
	zlog_notice("INVENTORY: HW MANUFACTURE [%s]", buff);
#endif

	/* update h/w model name */
    val[0] = inv.model_num[0] | inv.model_num[1];
    FPGA_WRITE(INV_HW_MODEL_1_ADDR, val[0]);
    val[1] = inv.model_num[2] | inv.model_num[3];
    FPGA_WRITE(INV_HW_MODEL_2_ADDR, val[1]);
    val[2] = inv.model_num[4] | inv.model_num[5];
    FPGA_WRITE(INV_HW_MODEL_3_ADDR, val[2]);
    val[3] = inv.model_num[6] | inv.model_num[7];
    FPGA_WRITE(INV_HW_MODEL_4_ADDR, val[3]);
    val[4] = inv.model_num[8] | inv.model_num[9];
    FPGA_WRITE(INV_HW_MODEL_5_ADDR, val[4]);

#ifdef DEBUG
    val[0] = FPGA_READ(INV_HW_MODEL_1_ADDR);
    val[1] = FPGA_READ(INV_HW_MODEL_2_ADDR);
    val[2] = FPGA_READ(INV_HW_MODEL_3_ADDR);
    val[3] = FPGA_READ(INV_HW_MODEL_4_ADDR);
    val[4] = FPGA_READ(INV_HW_MODEL_5_ADDR);

    memset(buff, 0, sizeof buff);
    buff[0] = val[0] & 0xFF;
    buff[1] = (val[0] >> 8) & 0xFF;
    buff[2] = val[1] & 0xFF;
    buff[3] = (val[1] >> 8) & 0xFF;
    buff[4] = val[2] & 0xFF;
    buff[5] = (val[2] >> 8) & 0xFF;
    buff[6] = val[3] & 0xFF;
    buff[7] = (val[3] >> 8) & 0xFF;
    buff[8] = val[4] & 0xFF;
    buff[9] = (val[4] >> 8) & 0xFF;
    zlog_notice("INVENTORY: HW MODEL [%s]", buff);
#endif

	/* update h/w part number */
    val[0] = inv.part_num[0] | inv.part_num[1];
    FPGA_WRITE(INV_HW_PN_1_ADDR, val[0]);
    val[1] = inv.part_num[2] | inv.part_num[3];
    FPGA_WRITE(INV_HW_PN_2_ADDR, val[1]);
    val[2] = inv.part_num[4] | inv.part_num[5];
    FPGA_WRITE(INV_HW_PN_3_ADDR, val[2]);
    val[3] = inv.part_num[6] | inv.part_num[7];
    FPGA_WRITE(INV_HW_PN_4_ADDR, val[3]);
    val[4] = inv.part_num[8] | inv.part_num[9];
    FPGA_WRITE(INV_HW_PN_5_ADDR, val[4]);
    val[5] = inv.part_num[10] | inv.part_num[11];
    FPGA_WRITE(INV_HW_PN_6_ADDR, val[4]);
    val[6] = inv.part_num[12] | inv.part_num[13];
    FPGA_WRITE(INV_HW_PN_7_ADDR, val[4]);
    val[7] = inv.part_num[14] | inv.part_num[15];
    FPGA_WRITE(INV_HW_PN_8_ADDR, val[4]);

#ifdef DEBUG
    val[0] = FPGA_READ(INV_HW_PN_1_ADDR);
    val[1] = FPGA_READ(INV_HW_PN_2_ADDR);
    val[2] = FPGA_READ(INV_HW_PN_3_ADDR);
    val[3] = FPGA_READ(INV_HW_PN_4_ADDR);
    val[4] = FPGA_READ(INV_HW_PN_5_ADDR);
    val[5] = FPGA_READ(INV_HW_PN_6_ADDR);
    val[6] = FPGA_READ(INV_HW_PN_7_ADDR);
    val[7] = FPGA_READ(INV_HW_PN_8_ADDR);

    memset(buff, 0, sizeof buff);
    buff[0] = val[0] & 0xFF;
    buff[1] = (val[0] >> 8) & 0xFF;
    buff[2] = val[1] & 0xFF;
    buff[3] = (val[1] >> 8) & 0xFF;
    buff[4] = val[2] & 0xFF;
    buff[5] = (val[2] >> 8) & 0xFF;
    buff[6] = val[3] & 0xFF;
    buff[7] = (val[3] >> 8) & 0xFF;
    buff[8] = val[4] & 0xFF;
    buff[9] = (val[4] >> 8) & 0xFF;
    buff[10] = val[5] & 0xFF;
    buff[11] = (val[5] >> 8) & 0xFF;
    buff[12] = val[6] & 0xFF;
    buff[13] = (val[6] >> 8) & 0xFF;
    buff[14] = val[7] & 0xFF;
    buff[15] = (val[7] >> 8) & 0xFF;
    zlog_notice("INVENTORY: HW PN [%s]", buff);
#endif

	/* update h/w serial number */
    val[0] = inv.serial_num[0] | inv.serial_num[1];
    FPGA_WRITE(INV_HW_SN_1_ADDR, val[0]);
    val[1] = inv.serial_num[2] | inv.serial_num[3];
    FPGA_WRITE(INV_HW_SN_2_ADDR, val[1]);

#ifdef DEBUG
    val[0] = FPGA_READ(INV_HW_SN_1_ADDR);
    val[1] = FPGA_READ(INV_HW_SN_2_ADDR);

    memset(buff, 0, sizeof buff);
    buff[0] = val[0] & 0xFF;
    buff[1] = (val[0] >> 8) & 0xFF;
    buff[2] = val[1] & 0xFF;
    buff[3] = (val[1] >> 8) & 0xFF;
    zlog_notice("INVENTORY: HW SN [%s]", buff);
#endif

	/* update h/w revision */
    val[0] = inv.revision[0] | inv.revision[1];
    FPGA_WRITE(INV_HW_REV_1_ADDR, val[0]);
    val[1] = inv.revision[2] | inv.revision[3];
    FPGA_WRITE(INV_HW_REV_2_ADDR, val[1]);

#ifdef DEBUG
    val[0] = FPGA_READ(INV_HW_REV_1_ADDR);
    val[1] = FPGA_READ(INV_HW_REV_2_ADDR);

    memset(buff, 0, sizeof buff);
    buff[0] = val[0] & 0xFF;
    buff[1] = (val[0] >> 8) & 0xFF;
    buff[2] = val[1] & 0xFF;
    buff[3] = (val[1] >> 8) & 0xFF;
    zlog_notice("INVENTORY: HW REV [%s]", buff);
#endif

	/* update h/w manufacture date */
    val[0] = inv.manufact_date[0] | inv.manufact_date[1];
    FPGA_WRITE(INV_HW_MDATE_1_ADDR, val[0]);
    val[1] = inv.manufact_date[2] | inv.manufact_date[3];
    FPGA_WRITE(INV_HW_MDATE_2_ADDR, val[1]);
    val[2] = inv.manufact_date[4] | inv.manufact_date[5];
    FPGA_WRITE(INV_HW_MDATE_3_ADDR, val[2]);
    val[3] = inv.manufact_date[6] | inv.manufact_date[7];
    FPGA_WRITE(INV_HW_MDATE_4_ADDR, val[3]);
    val[4] = inv.manufact_date[8] | inv.manufact_date[9];
    FPGA_WRITE(INV_HW_MDATE_5_ADDR, val[4]);
    val[5] = inv.manufact_date[10] | inv.manufact_date[11];
    FPGA_WRITE(INV_HW_MDATE_6_ADDR, val[4]);

#ifdef DEBUG
    val[0] = FPGA_READ(INV_HW_MDATE_1_ADDR);
    val[1] = FPGA_READ(INV_HW_MDATE_2_ADDR);
    val[2] = FPGA_READ(INV_HW_MDATE_3_ADDR);
    val[3] = FPGA_READ(INV_HW_MDATE_4_ADDR);
    val[4] = FPGA_READ(INV_HW_MDATE_5_ADDR);
    val[5] = FPGA_READ(INV_HW_MDATE_6_ADDR);

    memset(buff, 0, sizeof buff);
    buff[0] = val[0] & 0xFF;
    buff[1] = (val[0] >> 8) & 0xFF;
    buff[2] = val[1] & 0xFF;
    buff[3] = (val[1] >> 8) & 0xFF;
    buff[4] = val[2] & 0xFF;
    buff[5] = (val[2] >> 8) & 0xFF;
    buff[6] = val[3] & 0xFF;
    buff[7] = (val[3] >> 8) & 0xFF;
    buff[8] = val[4] & 0xFF;
    buff[9] = (val[4] >> 8) & 0xFF;
    buff[10] = val[5] & 0xFF;
    buff[11] = (val[5] >> 8) & 0xFF;
    zlog_notice("INVENTORY: HW MDATE [%s]", buff);
#endif

	/* update h/w repair date */
    val[0] = inv.repair_date[0] | inv.repair_date[1];
    FPGA_WRITE(INV_HW_RDATE_1_ADDR, val[0]);
    val[1] = inv.repair_date[2] | inv.repair_date[3];
    FPGA_WRITE(INV_HW_RDATE_2_ADDR, val[1]);
    val[2] = inv.repair_date[4] | inv.repair_date[5];
    FPGA_WRITE(INV_HW_RDATE_3_ADDR, val[2]);
    val[3] = inv.repair_date[6] | inv.repair_date[7];
    FPGA_WRITE(INV_HW_RDATE_4_ADDR, val[3]);
    val[4] = inv.repair_date[8] | inv.repair_date[9];
    FPGA_WRITE(INV_HW_RDATE_5_ADDR, val[4]);

#ifdef DEBUG
    val[0] = FPGA_READ(INV_HW_RDATE_1_ADDR);
    val[1] = FPGA_READ(INV_HW_RDATE_2_ADDR);
    val[2] = FPGA_READ(INV_HW_RDATE_3_ADDR);
    val[3] = FPGA_READ(INV_HW_RDATE_4_ADDR);
    val[4] = FPGA_READ(INV_HW_RDATE_5_ADDR);

    memset(buff, 0, sizeof buff);
    buff[0] = val[0] & 0xFF;
    buff[1] = (val[0] >> 8) & 0xFF;
    buff[2] = val[1] & 0xFF;
    buff[3] = (val[1] >> 8) & 0xFF;
    buff[4] = val[2] & 0xFF;
    buff[5] = (val[2] >> 8) & 0xFF;
    buff[6] = val[3] & 0xFF;
    buff[7] = (val[3] >> 8) & 0xFF;
    buff[8] = val[4] & 0xFF;
    buff[9] = (val[4] >> 8) & 0xFF;
    zlog_notice("INVENTORY: HW RDATE [%s]", buff);
#endif

	/* update h/w repair code */
    val[0] = inv.repair_code[0] | inv.repair_code[1];
    FPGA_WRITE(INV_HW_RCODE_1_ADDR, val[0]);
    val[1] = inv.repair_code[2] | inv.repair_code[3];
    FPGA_WRITE(INV_HW_RCODE_2_ADDR, val[1]);

#ifdef DEBUG
    val[0] = FPGA_READ(INV_HW_RCODE_1_ADDR);
    val[1] = FPGA_READ(INV_HW_RCODE_2_ADDR);

    memset(buff, 0, sizeof buff);
    buff[0] = val[0] & 0xFF;
    buff[1] = (val[0] >> 8) & 0xFF;
    buff[2] = val[1] & 0xFF;
    buff[3] = (val[1] >> 8) & 0xFF;
    zlog_notice("INVENTORY: HW RCODE [%s]", buff);
#endif

	return;
}

void process_alarm_info(void)
{
	u16 val;
	u16 masking;
	u8 portno;

	/* update alarm */
	for(portno = PORT_ID_EAG6L_PORT1; portno < PORT_ID_EAG6L_MAX; portno++) {
		val = 0;

		/*FIXME : update LOS */
		if(PORT_STATUS[portno].los)
			val |= (1 << 0);
		else
			val &= ~(1 << 0);

		/* update link down (Local Fault?) */
		if(! PORT_STATUS[portno].link)
			val |= (1 << 1);
		else
			val &= ~(1 << 1);

		/*FIXME : update LOC (ESMC Loss) */
		if(PORT_STATUS[portno].esmc_loss)
			val |= (1 << 2);
		else
			val &= ~(1 << 2);

		/*FIXME : update RF (Remote Fault) */
		if(PORT_STATUS[portno].remote_fault)
			val |= (1 << 3);
		else
			val &= ~(1 << 3);

		/*FIXME : update TX Bias alarm */
		if(PORT_STATUS[portno].tx_bias_sts)
			val |= (1 << 8);
		else
			val &= ~(1 << 8);

		/*FIXME : update Laser status */
		if(! PORT_STATUS[portno].tx_laser_sts)
			val |= (1 << 9);
		else
			val &= ~(1 << 9);

		/*FIXME : update rtWDM Loopback */
		if(PORT_STATUS[portno].rtwdm_lp)
			val |= (1 << 10);
		else
			val &= ~(1 << 10);

		/*FIXME : update Smart T-SFP Self Loopback */
		if(PORT_STATUS[portno].tsfp_self_lp)
			val |= (1 << 11);
		else
			val &= ~(1 << 11);

		/* read alarm mask register and mask off the result */
		masking = FPGA_READ(__PORT_ALM_MASK_ADDR[portno]);

		/* check if flex tune reset is on */
		if(masking & 0x8000) {
			/* FIXME : reset flex tune */
		}

		/* remove unnecessary bits */
		masking &= 0x10F;

		/* update alarm */
		FPGA_WRITE(__PORT_ALM_ADDR[portno], val);

		/* update alarm flag */
		FPGA_WRITE(__PORT_ALM_FLAG_ADDR[portno], val & masking);
	}

	return;
}

#if 0/*[#51] Adding register callback templates for config/command registers, dustin, 2024-06-12 */
#if 0 /* FIXME : laser control must be done by FPGA. */
int port_config_set_laser(u32 portno, u32 enable)
{
	/* use i2c to set sfp tx power on/off. */
	return 0;
}
#endif

int port_config_set_smart_tsfp_loop(u32 portno, u32 enable)
{
	/* use i2c to set smart tsfp loop control. */
	return 0;
}

int port_config_set_rtwdm_loop(u32 portno, u32 enable)
{
	/* use i2c to set rt wdm  loop control. */
	return 0;
}

int port_config_set_flex_tune(u32 portno, u32 enable)
{
	/* use i2c to set flex tune. */
	return 0;
}

int port_config_set_esmc(u32 portno, u32 enable)
{
extern void port_config_ESMC_enable(int port, int enable);

	/* use marvell sdk to set esmc enable/disable. */
	port_config_ESMC_enable(portno, enable);
	return 0;
}
#endif

void update_port_sfp_information(void)
{
	int portno;
	unsigned int val;

	/* update tx power */
	for(portno = PORT_ID_EAG6L_PORT1; portno < PORT_ID_EAG6L_MAX; portno++) {
		FPGA_WRITE(__PORT_TX_PWR_ADDR[portno], 
			convert_dbm_float_to_decimal(PORT_STATUS[portno].tx_pwr, 1/*dbm*/));
	}

	/* update rx power */
	for(portno = PORT_ID_EAG6L_PORT1; portno < PORT_ID_EAG6L_MAX; portno++) {
		FPGA_WRITE(__PORT_RX_PWR_ADDR[portno], 
			convert_dbm_float_to_decimal(PORT_STATUS[portno].rx_pwr, 1/*dbm*/));
	}

	/* update wavelength1 */
	for(portno = PORT_ID_EAG6L_PORT1; portno < PORT_ID_EAG6L_MAX; portno++) {
		FPGA_WRITE(__PORT_WL1_ADDR[portno], INV_TBL[portno].wave);
	}

	/* update wavelength2 */
	for(portno = PORT_ID_EAG6L_PORT1; portno < PORT_ID_EAG6L_MAX; portno++) {
		/*FIXME : refer to Hangle document for encoding sfp idenification codes. */
		val = INV_TBL[portno].wave;
		FPGA_WRITE(__PORT_WL1_ADDR[portno], val);
	}

	/* update distance */
	for(portno = PORT_ID_EAG6L_PORT1; portno < PORT_ID_EAG6L_MAX; portno++) {
		FPGA_WRITE(__PORT_WL1_ADDR[portno], INV_TBL[portno].dist);
	}

	/* update smart t-sfp status */
	/*FIXME*/

	/* update alarm */
	/*FIXME*/
	process_alarm_info();

	/* update channel number read */
	/*FIXME*/

	/* update channel number set */
	/*FIXME*/

	/* update vendor name */
	for(portno = PORT_ID_EAG6L_PORT1; portno < PORT_ID_EAG6L_MAX; portno++) {
		val = (INV_TBL[portno].vendor[0] << 8) | INV_TBL[portno].vendor[1];
		FPGA_WRITE(__PORT_VENDOR1_ADDR[portno], val);
		val = (INV_TBL[portno].vendor[2] << 8) | INV_TBL[portno].vendor[3];
		FPGA_WRITE(__PORT_VENDOR2_ADDR[portno], val);
		val = (INV_TBL[portno].vendor[4] << 8) | INV_TBL[portno].vendor[5];
		FPGA_WRITE(__PORT_VENDOR3_ADDR[portno], val);
		val = (INV_TBL[portno].vendor[6] << 8) | INV_TBL[portno].vendor[7];
		FPGA_WRITE(__PORT_VENDOR4_ADDR[portno], val);
		val = (INV_TBL[portno].vendor[8] << 8) | INV_TBL[portno].vendor[9];
		FPGA_WRITE(__PORT_VENDOR5_ADDR[portno], val);
		val = (INV_TBL[portno].vendor[10] << 8) | INV_TBL[portno].vendor[11];
		FPGA_WRITE(__PORT_VENDOR6_ADDR[portno], val);
		val = (INV_TBL[portno].vendor[12] << 8) | INV_TBL[portno].vendor[13];
		FPGA_WRITE(__PORT_VENDOR7_ADDR[portno], val);
		val = (INV_TBL[portno].vendor[14] << 8) | INV_TBL[portno].vendor[15];
		FPGA_WRITE(__PORT_VENDOR8_ADDR[portno], val);
	}

	/* update part number */
	for(portno = PORT_ID_EAG6L_PORT1; portno < PORT_ID_EAG6L_MAX; portno++) {
		val = (INV_TBL[portno].part_num[0] << 8) | INV_TBL[portno].part_num[1];
		FPGA_WRITE(__PORT_PN1_ADDR[portno], val);
		val = (INV_TBL[portno].part_num[2] << 8) | INV_TBL[portno].part_num[3];
		FPGA_WRITE(__PORT_PN2_ADDR[portno], val);
		val = (INV_TBL[portno].part_num[4] << 8) | INV_TBL[portno].part_num[5];
		FPGA_WRITE(__PORT_PN3_ADDR[portno], val);
		val = (INV_TBL[portno].part_num[6] << 8) | INV_TBL[portno].part_num[7];
		FPGA_WRITE(__PORT_PN4_ADDR[portno], val);
		val = (INV_TBL[portno].part_num[8] << 8) | INV_TBL[portno].part_num[9];
		FPGA_WRITE(__PORT_PN5_ADDR[portno], val);
		val = (INV_TBL[portno].part_num[10] << 8) | INV_TBL[portno].part_num[11];
		FPGA_WRITE(__PORT_PN6_ADDR[portno], val);
		val = (INV_TBL[portno].part_num[12] << 8) | INV_TBL[portno].part_num[13];
		FPGA_WRITE(__PORT_PN7_ADDR[portno], val);
		val = (INV_TBL[portno].part_num[14] << 8) | INV_TBL[portno].part_num[15];
		FPGA_WRITE(__PORT_PN8_ADDR[portno], val);
	}

	/* update serial number */
	for(portno = PORT_ID_EAG6L_PORT1; portno < PORT_ID_EAG6L_MAX; portno++) {
		val = (INV_TBL[portno].serial_num[0] << 8) | INV_TBL[portno].serial_num[1];
		FPGA_WRITE(__PORT_SN1_ADDR[portno], val);
		val = (INV_TBL[portno].serial_num[2] << 8) | INV_TBL[portno].serial_num[3];
		FPGA_WRITE(__PORT_SN2_ADDR[portno], val);
		val = (INV_TBL[portno].serial_num[4] << 8) | INV_TBL[portno].serial_num[5];
		FPGA_WRITE(__PORT_SN3_ADDR[portno], val);
		val = (INV_TBL[portno].serial_num[6] << 8) | INV_TBL[portno].serial_num[7];
		FPGA_WRITE(__PORT_SN4_ADDR[portno], val);
		val = (INV_TBL[portno].serial_num[8] << 8) | INV_TBL[portno].serial_num[9];
		FPGA_WRITE(__PORT_SN5_ADDR[portno], val);
		val = (INV_TBL[portno].serial_num[10] << 8) | INV_TBL[portno].serial_num[11];
		FPGA_WRITE(__PORT_SN6_ADDR[portno], val);
		val = (INV_TBL[portno].serial_num[12] << 8) | INV_TBL[portno].serial_num[13];
		FPGA_WRITE(__PORT_SN7_ADDR[portno], val);
		val = (INV_TBL[portno].serial_num[14] << 8) | INV_TBL[portno].serial_num[15];
		FPGA_WRITE(__PORT_SN8_ADDR[portno], val);
	}

	/* update temperature */
	for(portno = PORT_ID_EAG6L_PORT1; portno < PORT_ID_EAG6L_MAX; portno++) {
		val = convert_temperature_float_to_decimal(PORT_STATUS[portno].temp);
		FPGA_WRITE(__PORT_TEMP_ADDR[portno], val);
	}

	/* update rate */
	for(portno = PORT_ID_EAG6L_PORT1; portno < PORT_ID_EAG6L_MAX; portno++) {
		if(portno < PORT_ID_EAG6L_PORT7)
			val = INV_TBL[portno].max_rate;
		else
			val = INV_TBL[portno].max_rate * 10;/*convert 1G unit to 100M unit. */
		FPGA_WRITE(__PORT_RATE_ADDR[portno], val);
	}

	/* update voltage */
	for(portno = PORT_ID_EAG6L_PORT1; portno < PORT_ID_EAG6L_MAX; portno++) {
		val = convert_dbm_float_to_decimal(PORT_STATUS[portno].vcc, 0/*not-dbm*/);
		FPGA_WRITE(__PORT_VOLT_ADDR[portno], val);
	}

	/* update tx bias */
	for(portno = PORT_ID_EAG6L_PORT1; portno < PORT_ID_EAG6L_MAX; portno++) {
		val = convert_temperature_float_to_decimal(PORT_STATUS[portno].tx_bias);
		FPGA_WRITE(__PORT_TX_BIAS_ADDR[portno], val);
	}

	/* update laser temperature */
	for(portno = PORT_ID_EAG6L_PORT1; portno < PORT_ID_EAG6L_MAX; portno++) {
		val = convert_temperature_float_to_decimal(PORT_STATUS[portno].temp);
		FPGA_WRITE(__PORT_TEMP_ADDR[portno], val);
	}

	/* update TEC current */
	for(portno = PORT_ID_EAG6L_PORT1; portno < PORT_ID_EAG6L_MAX; portno++) {
		val = convert_temperature_float_to_decimal(PORT_STATUS[portno].tec_curr);
		FPGA_WRITE(__PORT_TCURR_ADDR[portno], val);
	}

	return;
}

void process_port_pm_counters(void)
{
	u16 portno;
	u16 val;

	/* check/execute clear register */
	val = FPGA_READ(PM_COUNT_CLEAR_ADDR);
	if(val == 0xA5/*clear*/) {
		/* marvell counters are cleared after reading, 
		   so don't need to request to clear counters
		   just clear PM_TBL counter caches */
		memset(PM_TBL, 0, sizeof(PM_TBL));
	}
	//FIXME : read-clear ?
	FPGA_WRITE(PM_COUNT_CLEAR_ADDR, 0x0);

    /* update pm tx byte */
    for(portno = PORT_ID_EAG6L_PORT1; portno < PORT_ID_EAG6L_MAX; portno++) {
        val = (PM_TBL[portno].tx_byte >> 48) & 0xFFFF;
        FPGA_WRITE(__PORT_PM_TX_BYTE1_ADDR[portno], val);
        val = (PM_TBL[portno].tx_byte >> 32) & 0xFFFF;
        FPGA_WRITE(__PORT_PM_TX_BYTE2_ADDR[portno], val);
        val = (PM_TBL[portno].tx_byte >> 16) & 0xFFFF;
        FPGA_WRITE(__PORT_PM_TX_BYTE3_ADDR[portno], val);
        val = (PM_TBL[portno].tx_byte >>  0) & 0xFFFF;
        FPGA_WRITE(__PORT_PM_TX_BYTE4_ADDR[portno], val);
    }

    /* update pm rx byte */
    for(portno = PORT_ID_EAG6L_PORT1; portno < PORT_ID_EAG6L_MAX; portno++) {
        val = (PM_TBL[portno].rx_byte >> 48) & 0xFFFF;
        FPGA_WRITE(__PORT_PM_RX_BYTE1_ADDR[portno], val);
        val = (PM_TBL[portno].rx_byte >> 32) & 0xFFFF;
        FPGA_WRITE(__PORT_PM_RX_BYTE2_ADDR[portno], val);
        val = (PM_TBL[portno].rx_byte >> 16) & 0xFFFF;
        FPGA_WRITE(__PORT_PM_RX_BYTE3_ADDR[portno], val);
        val = (PM_TBL[portno].rx_byte >>  0) & 0xFFFF;
        FPGA_WRITE(__PORT_PM_RX_BYTE4_ADDR[portno], val);
    }

    /* update pm tx frame */
    for(portno = PORT_ID_EAG6L_PORT1; portno < PORT_ID_EAG6L_MAX; portno++) {
        val = (PM_TBL[portno].tx_frame >> 48) & 0xFFFF;
        FPGA_WRITE(__PORT_PM_TX_FRAME1_ADDR[portno], val);
        val = (PM_TBL[portno].tx_frame >> 32) & 0xFFFF;
        FPGA_WRITE(__PORT_PM_TX_FRAME2_ADDR[portno], val);
        val = (PM_TBL[portno].tx_frame >> 16) & 0xFFFF;
        FPGA_WRITE(__PORT_PM_TX_FRAME3_ADDR[portno], val);
        val = (PM_TBL[portno].tx_frame >>  0) & 0xFFFF;
        FPGA_WRITE(__PORT_PM_TX_FRAME4_ADDR[portno], val);
    }

    /* update pm rx frame */
    for(portno = PORT_ID_EAG6L_PORT1; portno < PORT_ID_EAG6L_MAX; portno++) {
        val = (PM_TBL[portno].rx_frame >> 48) & 0xFFFF;
        FPGA_WRITE(__PORT_PM_RX_FRAME1_ADDR[portno], val);
        val = (PM_TBL[portno].rx_frame >> 32) & 0xFFFF;
        FPGA_WRITE(__PORT_PM_RX_FRAME2_ADDR[portno], val);
        val = (PM_TBL[portno].rx_frame >> 16) & 0xFFFF;
        FPGA_WRITE(__PORT_PM_RX_FRAME3_ADDR[portno], val);
        val = (PM_TBL[portno].rx_frame >>  0) & 0xFFFF;
        FPGA_WRITE(__PORT_PM_RX_FRAME4_ADDR[portno], val);
    }

    /* update pm fcs */
    for(portno = PORT_ID_EAG6L_PORT1; portno < PORT_ID_EAG6L_MAX; portno++) {
        val = (PM_TBL[portno].rx_fcs >> 48) & 0xFFFF;
        FPGA_WRITE(__PORT_PM_FCS1_ADDR[portno], val);
        val = (PM_TBL[portno].rx_fcs >> 32) & 0xFFFF;
        FPGA_WRITE(__PORT_PM_FCS2_ADDR[portno], val);
        val = (PM_TBL[portno].rx_fcs >> 16) & 0xFFFF;
        FPGA_WRITE(__PORT_PM_FCS3_ADDR[portno], val);
        val = (PM_TBL[portno].rx_fcs >>  0) & 0xFFFF;
        FPGA_WRITE(__PORT_PM_FCS4_ADDR[portno], val);
    }

    /* update pm fcs corrected error */
    for(portno = PORT_ID_EAG6L_PORT1; portno < PORT_ID_EAG6L_MAX; portno++) {
        val = (PM_TBL[portno].fcs_ok >> 48) & 0xFFFF;
        FPGA_WRITE(__PORT_PM_FCS_OK1_ADDR[portno], val);
        val = (PM_TBL[portno].fcs_ok >> 32) & 0xFFFF;
        FPGA_WRITE(__PORT_PM_FCS_OK2_ADDR[portno], val);
        val = (PM_TBL[portno].fcs_ok >> 16) & 0xFFFF;
        FPGA_WRITE(__PORT_PM_FCS_OK3_ADDR[portno], val);
        val = (PM_TBL[portno].fcs_ok >>  0) & 0xFFFF;
        FPGA_WRITE(__PORT_PM_FCS_OK4_ADDR[portno], val);
    }

    /* update pm fcs uncorrected error */
    for(portno = PORT_ID_EAG6L_PORT1; portno < PORT_ID_EAG6L_MAX; portno++) {
        val = (PM_TBL[portno].fcs_nok >> 48) & 0xFFFF;
        FPGA_WRITE(__PORT_PM_FCS_NOK1_ADDR[portno], val);
        val = (PM_TBL[portno].fcs_nok >> 48) & 0xFFFF;
        FPGA_WRITE(__PORT_PM_FCS_NOK2_ADDR[portno], val);
        val = (PM_TBL[portno].fcs_nok >> 48) & 0xFFFF;
        FPGA_WRITE(__PORT_PM_FCS_NOK3_ADDR[portno], val);
        val = (PM_TBL[portno].fcs_nok >> 48) & 0xFFFF;
        FPGA_WRITE(__PORT_PM_FCS_NOK4_ADDR[portno], val);
    }

	return;
}

void update_bp_reg(void)
{
	u16 ii, temp, pri_port, sec_port, bitmask;
	u16 val[20];

#ifdef DEBUG
	zlog_notice(" update_bp_reg");/*ZZPP*/
#endif
	/***********************************************/
	/* check if mcu keep alive counter is ticking. */
	temp = FPGA_READ(HW_KEEP_ALIVE_1_ADDR);
	if(MCU_KEEP_ALIVE_CACHE == temp) {
		if(MCU_KEEP_ALIVE_RETRY_COUNT++ >= MCU_KEEP_ALIVE_RETRY_MAX) {
			zlog_notice("MCU dead ??");
			/* FIXME : call alarm function. */
		}
	} else if(temp && (temp > MCU_KEEP_ALIVE_CACHE))
		MCU_KEEP_ALIVE_CACHE = temp;

	/***********************************************/
	/* increase BP's h/w keep alive counter. */
	temp = FPGA_READ(HW_KEEP_ALIVE_1_ADDR);
	FPGA_WRITE(HW_KEEP_ALIVE_1_ADDR, temp + 1);
#ifdef DEBUG
	if(temp == FPGA_READ(HW_KEEP_ALIVE_1_ADDR))
		zlog_notice("Stucked Keep-alive : before[%d] after[%d]", 
			temp, FPGA_READ(HW_KEEP_ALIVE_1_ADDR));
#endif

	if(! INIT_COMPLETE_FLAG)
		return;

	/***********************************************/
	/* check if MCU init complete. */
	temp = FPGA_READ(INIT_COMPLETE_ADDR);
	MCU_INIT_COMPLETE_FLAG = ((temp & 0xFF) == 0xAA) ? 1 : 0;
	/* set init complete register. */
	temp |= 0xAA00;
	FPGA_WRITE(INIT_COMPLETE_ADDR, temp);

#if 0/*[#51] Adding register callback templates for config/command registers, dustin, 2024-06-12 */
	/***********************************************/
	/* check/configure common control2. */
	process_port_common_control_register();
#endif

	/***********************************************/
	/* check/execute if reset CR is set. */
	temp = FPGA_READ(CHIP_RESET_ADDR);
	/* check/execute if reset CR is set for BP. */
	if((temp & 0xFF) == 0xA5) {
		/* do reset for BP. */
		/* FIXME call reset function for BP */
		/* read-clear */
		temp &= ~0xFF;
	} else if((temp & 0xFF00) == 0xA500) {
		/* do reset for FPGA. */
		/* FIXME call reset function for FPGA */
		/* read-clear */
		temp &= ~0xFF00;
	}
	FPGA_WRITE(CHIP_RESET_ADDR, temp);

	/***********************************************/
	/* check/configure dco registers. */
	/*FIXME*/
	process_dco_registers();

	/***********************************************/
	/* update inventory. */
	process_hw_inventory_infos();

#if 0/*[#51] Adding register callback templates for config/command registers, dustin, 2024-06-12 */
	/***********************************************/
	/* check/configure ports. */
	for(ii = PORT_ID_EAG6L_PORT1; ii < PORT_ID_EAG6L_MAX; ii++) {
		temp = FPGA_READ(__PORT_CONFIG_ADDR[ii]);

		/* skip if value is not changed. */
		if(temp == PORT_CONFIG_CACHE[ii])
			continue;

		/* get changed bit mask */
		bitmask = temp ^ PORT_CONFIG_CACHE[ii];

#if 0 /* FIXME : laser control must be done by FPGA. */
		/* config port laser on/off. */
		if(bitmask & 0xFF00) {
			if((temp & 0xFF00) == 0xA500)
				port_config_set_laser(ii, 1);
			if((temp & 0xFF00) == 0x5A00)
				port_config_set_laser(ii, 0);
		}
#endif

		/* config port smart tsfp self loop. */
		if(bitmask & 0x20) 
			port_config_set_smart_tsfp_loop(ii, (temp & 0x20));

		/* config port rtWDM loop. */
		if(bitmask & 0x10) 
			port_config_set_rtwdm_loop(ii, (temp & 0x10));

		/* config port flex tune. */
		if(bitmask & 0x8) 
			port_config_set_flex_tune(ii, (temp & 0x8));

		/* config port ESMC enable. */
		if(bitmask & 0x4) 
			port_config_set_esmc(ii, (temp & 0x4));
	}
#endif

	/***********************************************/
	/* check/configure synce enable. */
	temp = FPGA_READ(SYNCE_GCONFIG_ADDR);
	if(temp != SYNCE_GCONFIG_CACHE) {
		if((temp & 0xFF) == 0xA5)
			synce_config_set_admin(1);
		else if((temp & 0xFF) == 0x5A)
			synce_config_set_admin(0);
	}

	/***********************************************/
	/* check/configure synce i/f select. */
	temp = FPGA_READ(SYNCE_IF_SELECT_ADDR);
	if(temp != SYNCE_IF_SELECT_CACHE) {
		val[0] = (temp >> 8) & 0xFF;
		val[1] = (temp & 0xFF);
		if(val[0] == 0x11)
			pri_port = PORT_ID_EAG6L_PORT1;
		else if(val[0] == 0x12)
			pri_port = PORT_ID_EAG6L_PORT2;
		else if(val[0] == 0x13)
			pri_port = PORT_ID_EAG6L_PORT3;
		else if(val[0] == 0x14)
			pri_port = PORT_ID_EAG6L_PORT4;
		else if(val[0] == 0x21)
			pri_port = PORT_ID_EAG6L_PORT5;
		else if(val[0] == 0x22)
			pri_port = PORT_ID_EAG6L_PORT6;
		else if(val[0] == 0x23)
			pri_port = PORT_ID_EAG6L_PORT7;
		else
			pri_port = PORT_ID_EAG6L_NOT_USE;

		if(val[1] == 0x11)
			sec_port = PORT_ID_EAG6L_PORT1;
		else if(val[1] == 0x12)
			sec_port = PORT_ID_EAG6L_PORT2;
		else if(val[1] == 0x13)
			sec_port = PORT_ID_EAG6L_PORT3;
		else if(val[1] == 0x14)
			sec_port = PORT_ID_EAG6L_PORT4;
		else if(val[1] == 0x21)
			sec_port = PORT_ID_EAG6L_PORT5;
		else if(val[1] == 0x22)
			sec_port = PORT_ID_EAG6L_PORT6;
		else if(val[1] == 0x23)
			sec_port = PORT_ID_EAG6L_PORT7;
		else 
			sec_port = PORT_ID_EAG6L_NOT_USE;

		if((pri_port != PORT_ID_EAG6L_NOT_USE) && (sec_port != PORT_ID_EAG6L_NOT_USE))
			synce_config_set_if_select(pri_port, sec_port);
	}

	/***********************************************/
	/* read per-port spf port status/control. */
	/* FIXME : call fuction. */
	update_port_sfp_information();


	/***********************************************/
	/* process per-port performance info from sdk. */
	process_port_pm_counters();
}
