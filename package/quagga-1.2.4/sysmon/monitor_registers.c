#if 1 /* [#62] SFP eeprom 및 register update 기능 단위 검증 및 디버깅, balkrow, 2024-06-21 */ 
#include <math.h>
#include "thread.h"
#endif
#include "log.h"
#include "sysmon.h"
#include "bp_regs.h"
#if 1/*[#48] register monitoring and update 관련 기능 추가, balkrow, 2024-06-10*/ 
#include "sys_fifo.h"
#endif
#if 1/* [#78] Adding system inventory management, dustin, 2024-07-24 */
#include "hdriver.h"
#endif
#if 1 /* [#91] Fixing for register updating feature, dustin, 2024-08-05 */
#include "rdl_fsm.h"
#endif


#undef DEBUG

#if 0
extern u32 INIT_COMPLETE_FLAG;
#endif

u16 PORT_CONFIG_CACHE[PORT_ID_EAG6L_MAX];
u16 PORT_COMMON_CONTROL2_CACHE[PORT_ID_EAG6L_MAX];
u16 SYNCE_GCONFIG_CACHE;
u16 SYNCE_IF_SELECT_CACHE;
u32 MCU_KEEP_ALIVE_CACHE;


port_status_t PORT_STATUS[PORT_ID_EAG6L_MAX];
struct module_inventory INV_TBL[PORT_ID_EAG6L_MAX];
#if 1/* [#72] Adding omitted rtWDM related registers, dustin, 2024-06-27 */
struct module_inventory RTWDM_INV_TBL[PORT_ID_EAG6L_MAX];
#endif
port_pm_counter_t PM_TBL[PORT_ID_EAG6L_MAX];
#if 1 /* [#91] Fixing for register updating feature, dustin, 2024-08-05 */
uint16_t boardStatusFlag = 0;
#endif
#if 1 /* [#94] Adding for 100G DCO handling, dustin, 2024-09-23 */
dco_status_t DCO_STAT;
dco_count_t  DCO_COUNT;
#endif
#if 1 /* [#150] Implementing LR4 Status register, dustin, 2024-10-21 */
lr4_status_t LR4_STAT;
#endif


extern int synce_config_set_admin(int enable);
extern int synce_config_set_if_select(int pri_port, int sec_port);
extern void pm_request_counters(void);
extern void pm_request_clear(void);

#if 1/*[#53] Clock source status 업데이트 기능 추가, balkrow, 2024-06-13*/
extern GLOBAL_DB gDB;
extern int8_t rsmuGetPLLState(void); 
#endif
#if 1 /* [#169] Fixing for new DCO install process, dustin, 2024-10-25 */
extern void init_100g_sfp(struct thread *thread);
int thread_kill_flag;
int dco_retry_cnt;
#endif

#if 1/*[#48] register monitoring and update 관련 기능 추가, balkrow, 2024-06-10*/ 
extern cSysmonToCPSSFuncs gSysmonToCpssFuncs[];
#endif
#if 1 /* [#125] Fixing for SFP channel no, wavelength, tx/rx dBm, dustin, 2024-09-10 */
extern int i2c_in_use_flag;
extern int i2c_in_use_flag_backup;
#endif

#if 1/*[#51] Adding register callback templates for config/command registers, dustin, 2024-06-12 */
uint16_t portESMCenable (uint16_t port, uint16_t val);
uint16_t portRateSet (uint16_t port, uint16_t val);
#if 1/*[#59] Synce configuration 연동 기능 추가, balkrow, 2024-06-19 */
uint16_t synceEnableSet(uint16_t port, uint16_t val);
#endif

#if 1/*[#48] register monitoring and update 관련 기능 추가, balkrow, 2024-06-10*/ 
uint16_t synceIFPriSelect(uint16_t port, uint16_t val);
uint16_t synceIFSecSelect(uint16_t port, uint16_t val);
#else
uint16_t synceIFSelect(uint16_t port, uint16_t val);
#endif
uint16_t pmClear(uint16_t port, uint16_t val);
#if 1 /* [#85] Fixing for resetting PM counter for unexpected FEC counting, dustin, 2024-07-31 */
uint16_t pmFECClear(uint16_t port);
#endif
uint16_t chipReset(uint16_t port, uint16_t val);
uint16_t boardStatus(uint16_t port, uint16_t val);
#if 1/*[#61] Adding omitted functions, dustin, 2024-06-24 */
uint16_t bankSelect1(uint16_t port, uint16_t val);
uint16_t bankSelect2(uint16_t port, uint16_t val);
#endif
#if 1 /* [#151] Implementing P7 config register, dustin, 2024-10-21 */
extern int8_t sysmon_llcf_set(int32_t enable);
uint16_t setLLCFenable(uint16_t val);
#endif
#if 1/*[#118] Sync-e option2 지원, balkrow, 2024-09-06*/
uint16_t syncePortSendQL(uint16_t port, uint16_t val);
uint16_t synceLocalQL(uint16_t port, uint16_t val);
#endif
#if 1 /* [#157] Fixing for Smart T-SFP rtWDM info, dustin, 2024-10-18 */
void update_port_sfp_inventory(uint8_t rtwdm_flag);
#else
void update_port_sfp_inventory(void);
#endif
#if 1/*[#56] register update timer 수정, balkrow, 2023-06-13 */
#if 1 /* [#91] Fixing for register updating feature, dustin, 2024-08-05 */
extern uint16_t sys_fpga_memory_read(uint32_t addr, uint8_t port_reg);
#else
extern uint16_t sys_fpga_memory_read(uint16_t addr, uint8_t port_reg);
#endif
#endif
#if 1 /* [#91] Fixing for register updating feature, dustin, 2024-08-05 */
extern uint16_t sys_cpld_memory_read(uint16_t addr);
#endif
extern void port_config_ESMC_enable(int port, int enable);
extern uint16_t set_flex_tune_control(uint16_t port, uint16_t enable);
extern uint16_t set_rtwdm_loopback(uint16_t port, uint16_t enable);
extern uint16_t set_smart_tsfp_self_loopback(uint16_t port, uint16_t enable);
extern uint16_t set_flex_tune_reset(uint16_t port, uint16_t enable);
#if 1/*[#61] Adding omitted functions, dustin, 2024-06-24 */
uint16_t set_tunable_sfp_channel_no(uint16_t portno, uint16_t chno);
#endif
#if 1 /* [#100] Adding update of Laser status by Laser_con, dustin, 2024-08-23 */
void update_laser_status(uint16_t portno, uint16_t val);
#endif
#if 1/* [#72] Adding omitted rtWDM related registers, dustin, 2024-06-27 */
void read_port_inventory(int portno, struct module_inventory * mod_inv);
void read_port_rtwdm_inventory(int portno, struct module_inventory * mod_inv);

extern void get_sfp_info(int portno, struct module_inventory * mod_inv);
extern int get_sfp_info_diag(int portno, port_status_t * port_sts);
extern void get_sfp_rtwdm_info(int portno, struct module_inventory * mod_inv);
extern void get_sfp_rtwdm_info_diag(int portno, port_status_t * port_sts);
extern int update_sfp_channel_no(int portno);
extern int get_tunable_sfp_channel_no(int portno);
extern int get_flex_tune_status(int portno);
#endif
#if 1 /* [#105] Fixing for RDL install/activation process, dustin, 2024-08-27 */
extern void rdl_update_bank_registers(int bno, int erase_flag);
extern uint16_t RDL_B1_ERASE_FLAG;
extern uint16_t RDL_B2_ERASE_FLAG;
#endif
#if 1 /* [#94] Adding for 100G DCO handling, dustin, 2024-09-23 */
#if 1 /* [#152] Adding for port RS-FEC control, dustin, 2024-10-15 */
uint16_t portFECEnable(uint16_t portno, uint16_t enable);
#endif
#if 1 /* [#154] Fixing for auto FEC mode on DCO, dustin, 2024-10-21 */
/* NOTE : regMon callback function must have portno and value arguments. */
uint16_t DcoReset(uint16_t portno, uint16_t val);
uint16_t DcoCountReset(uint16_t portno, uint16_t val);
uint16_t DcoFECEnable(uint16_t portno, uint16_t val);
#else
uint16_t DcoReset(uint16_t val);
uint16_t DcoCountReset(uint16_t val);
uint16_t DcoFECEnable(uint16_t val);
#endif /* [#154] */

extern int set_i2c_dco_reset(void);
extern int set_i2c_dco_fec_enable(int hs_flag, int ms_flag);
extern int set_i2c_dco_count_reset(void);
extern int get_i2c_dco_status(void);

extern uint16_t set_dco_sfp_channel_no(uint16_t portno, uint16_t chno);
#endif
#if 1 /* [#142] Adding for Transparent mode switching, dustin, 2024-10-11 */
#if 1 /* [#164] Fixing for correct switch mode, dustin, 2024-1023 */
/* NOTE : regMon callback function must have portno and value arguments. */
uint16_t swModeSet(uint16_t portno, uint16_t sw_mode);
#else
uint16_t swModeSet(uint16_t sw_mode);
#endif
#endif


#if 1 /* [#62] SFP eeprom 및 register update 기능 단위 검증 및 디버깅, balkrow, 2024-06-21 */ 
extern struct thread_master *master;

RegMON regMonList [] = {
	/* common control 2 - port speed */
#if 1 /* [#102] Fixing some register updates, dustin, 2024-08-26 */
  { COMMON_CTRL2_P1_ADDR, 0xF, 0, 0x9, PORT_ID_EAG6L_PORT1, 0, NULL, sys_fpga_memory_read, portRateSet }, 
  { COMMON_CTRL2_P2_ADDR, 0xF, 0, 0x9, PORT_ID_EAG6L_PORT2, 0, NULL, sys_fpga_memory_read, portRateSet }, 
  { COMMON_CTRL2_P3_ADDR, 0xF, 0, 0x9, PORT_ID_EAG6L_PORT3, 0, NULL, sys_fpga_memory_read, portRateSet }, 
  { COMMON_CTRL2_P4_ADDR, 0xF, 0, 0x9, PORT_ID_EAG6L_PORT4, 0, NULL, sys_fpga_memory_read, portRateSet }, 
  { COMMON_CTRL2_P5_ADDR, 0xF, 0, 0x9, PORT_ID_EAG6L_PORT5, 0, NULL, sys_fpga_memory_read, portRateSet }, 
  { COMMON_CTRL2_P6_ADDR, 0xF, 0, 0x9, PORT_ID_EAG6L_PORT6, 0, NULL, sys_fpga_memory_read, portRateSet }, 
#else
  { COMMON_CTRL2_P1_ADDR, 0x7, 0, 0x7, PORT_ID_EAG6L_PORT1, 0, NULL, sys_fpga_memory_read, portRateSet }, 
  { COMMON_CTRL2_P2_ADDR, 0x7, 0, 0x7, PORT_ID_EAG6L_PORT2, 0, NULL, sys_fpga_memory_read, portRateSet }, 
  { COMMON_CTRL2_P3_ADDR, 0x7, 0, 0x7, PORT_ID_EAG6L_PORT3, 0, NULL, sys_fpga_memory_read, portRateSet }, 
  { COMMON_CTRL2_P4_ADDR, 0x7, 0, 0x7, PORT_ID_EAG6L_PORT4, 0, NULL, sys_fpga_memory_read, portRateSet }, 
  { COMMON_CTRL2_P5_ADDR, 0x7, 0, 0x7, PORT_ID_EAG6L_PORT5, 0, NULL, sys_fpga_memory_read, portRateSet }, 
  { COMMON_CTRL2_P6_ADDR, 0x7, 0, 0x7, PORT_ID_EAG6L_PORT6, 0, NULL, sys_fpga_memory_read, portRateSet }, 
#endif
	/* port configuration - esmc */                        
#if 1 /* [#107] Fixing for 2nd register updates, dustin, 2024-08-29 */
  { PORT_1_CONF_ADDR,     0x8, 3, 0x0, PORT_ID_EAG6L_PORT1, 0, NULL, sys_fpga_memory_read, portESMCenable }, 
  { PORT_2_CONF_ADDR,     0x8, 3, 0x0, PORT_ID_EAG6L_PORT2, 0, NULL, sys_fpga_memory_read, portESMCenable }, 
  { PORT_3_CONF_ADDR,     0x8, 3, 0x0, PORT_ID_EAG6L_PORT3, 0, NULL, sys_fpga_memory_read, portESMCenable }, 
  { PORT_4_CONF_ADDR,     0x8, 3, 0x0, PORT_ID_EAG6L_PORT4, 0, NULL, sys_fpga_memory_read, portESMCenable }, 
  { PORT_5_CONF_ADDR,     0x8, 3, 0x0, PORT_ID_EAG6L_PORT5, 0, NULL, sys_fpga_memory_read, portESMCenable }, /* 10 */  
  { PORT_6_CONF_ADDR,     0x8, 3, 0x0, PORT_ID_EAG6L_PORT6, 0, NULL, sys_fpga_memory_read, portESMCenable },
#if 1 /* [#151] Implementing P7 config register, dustin, 2024-10-21 */
  { PORT_7_CONF_ADDR,     0x3, 0, 0x0, PORT_ID_EAG6L_PORT7, 0, NULL, sys_fpga_memory_read, setLLCFenable }, 
#else
  { PORT_7_CONF_ADDR,     0x8, 3, 0x0, PORT_ID_EAG6L_PORT7, 0, NULL, sys_fpga_memory_read, portESMCenable }, 
#endif
	/* port configuration - flex control */
  { PORT_1_CONF_ADDR,     0x4, 2, 0x0, PORT_ID_EAG6L_PORT1, 0, NULL, sys_fpga_memory_read, set_flex_tune_control }, 
  { PORT_2_CONF_ADDR,     0x4, 2, 0x0, PORT_ID_EAG6L_PORT2, 0, NULL, sys_fpga_memory_read, set_flex_tune_control }, 
  { PORT_3_CONF_ADDR,     0x4, 2, 0x0, PORT_ID_EAG6L_PORT3, 0, NULL, sys_fpga_memory_read, set_flex_tune_control }, 
  { PORT_4_CONF_ADDR,     0x4, 2, 0x0, PORT_ID_EAG6L_PORT4, 0, NULL, sys_fpga_memory_read, set_flex_tune_control }, 
  { PORT_5_CONF_ADDR,     0x4, 2, 0x0, PORT_ID_EAG6L_PORT5, 0, NULL, sys_fpga_memory_read, set_flex_tune_control }, 
  { PORT_6_CONF_ADDR,     0x4, 2, 0x0, PORT_ID_EAG6L_PORT6, 0, NULL, sys_fpga_memory_read, set_flex_tune_control }, 
#if 0 /* [#151] Implementing P7 config register, dustin, 2024-10-21 */
  { PORT_7_CONF_ADDR,     0x4, 2, 0x0, PORT_ID_EAG6L_PORT7, 0, NULL, sys_fpga_memory_read, set_flex_tune_control }, 
#endif
#else /**************************************************************/
  { PORT_1_CONF_ADDR,     0x4, 2, 0x0, PORT_ID_EAG6L_PORT1, 0, NULL, sys_fpga_memory_read, portESMCenable }, 
  { PORT_2_CONF_ADDR,     0x4, 2, 0x0, PORT_ID_EAG6L_PORT2, 0, NULL, sys_fpga_memory_read, portESMCenable }, 
  { PORT_3_CONF_ADDR,     0x4, 2, 0x0, PORT_ID_EAG6L_PORT3, 0, NULL, sys_fpga_memory_read, portESMCenable }, 
  { PORT_4_CONF_ADDR,     0x4, 2, 0x0, PORT_ID_EAG6L_PORT4, 0, NULL, sys_fpga_memory_read, portESMCenable }, 
  { PORT_5_CONF_ADDR,     0x4, 2, 0x0, PORT_ID_EAG6L_PORT5, 0, NULL, sys_fpga_memory_read, portESMCenable }, /* 10 */  
  { PORT_6_CONF_ADDR,     0x4, 2, 0x0, PORT_ID_EAG6L_PORT6, 0, NULL, sys_fpga_memory_read, portESMCenable },
  { PORT_7_CONF_ADDR,     0x4, 2, 0x0, PORT_ID_EAG6L_PORT7, 0, NULL, sys_fpga_memory_read, portESMCenable }, 
	/* port configuration - flex control */
#if 1 /* [#91] Fixing for register updating feature, dustin, 2024-08-05 */
  { PORT_1_CONF_ADDR,     0x8, 3, 0x0, PORT_ID_EAG6L_PORT1, 0, NULL, sys_fpga_memory_read, set_flex_tune_control }, 
  { PORT_2_CONF_ADDR,     0x8, 3, 0x0, PORT_ID_EAG6L_PORT2, 0, NULL, sys_fpga_memory_read, set_flex_tune_control }, 
  { PORT_3_CONF_ADDR,     0x8, 3, 0x0, PORT_ID_EAG6L_PORT3, 0, NULL, sys_fpga_memory_read, set_flex_tune_control }, 
  { PORT_4_CONF_ADDR,     0x8, 3, 0x0, PORT_ID_EAG6L_PORT4, 0, NULL, sys_fpga_memory_read, set_flex_tune_control }, 
  { PORT_5_CONF_ADDR,     0x8, 3, 0x0, PORT_ID_EAG6L_PORT5, 0, NULL, sys_fpga_memory_read, set_flex_tune_control }, 
  { PORT_6_CONF_ADDR,     0x8, 3, 0x0, PORT_ID_EAG6L_PORT6, 0, NULL, sys_fpga_memory_read, set_flex_tune_control }, 
  { PORT_7_CONF_ADDR,     0x8, 3, 0x0, PORT_ID_EAG6L_PORT7, 0, NULL, sys_fpga_memory_read, set_flex_tune_control }, 
#else
  { PORT_1_CONF_ADDR,     0x4, 3, 0x0, PORT_ID_EAG6L_PORT1, 0, NULL, sys_fpga_memory_read, set_flex_tune_control }, 
  { PORT_2_CONF_ADDR,     0x4, 3, 0x0, PORT_ID_EAG6L_PORT2, 0, NULL, sys_fpga_memory_read, set_flex_tune_control }, 
  { PORT_3_CONF_ADDR,     0x4, 3, 0x0, PORT_ID_EAG6L_PORT3, 0, NULL, sys_fpga_memory_read, set_flex_tune_control }, 
  { PORT_4_CONF_ADDR,     0x4, 3, 0x0, PORT_ID_EAG6L_PORT4, 0, NULL, sys_fpga_memory_read, set_flex_tune_control }, 
  { PORT_5_CONF_ADDR,     0x4, 3, 0x0, PORT_ID_EAG6L_PORT5, 0, NULL, sys_fpga_memory_read, set_flex_tune_control }, 
  { PORT_6_CONF_ADDR,     0x4, 3, 0x0, PORT_ID_EAG6L_PORT6, 0, NULL, sys_fpga_memory_read, set_flex_tune_control }, 
  { PORT_7_CONF_ADDR,     0x4, 3, 0x0, PORT_ID_EAG6L_PORT7, 0, NULL, sys_fpga_memory_read, set_flex_tune_control }, 
#endif
#endif /* [#107] */
	/* port configuration - rtwdm loopback */
  { PORT_1_CONF_ADDR,     0x10, 4, 0x0, PORT_ID_EAG6L_PORT1, 0, NULL, sys_fpga_memory_read, set_rtwdm_loopback }, /* 20 */ 
  { PORT_2_CONF_ADDR,     0x10, 4, 0x0, PORT_ID_EAG6L_PORT2, 0, NULL, sys_fpga_memory_read, set_rtwdm_loopback }, 
  { PORT_3_CONF_ADDR,     0x10, 4, 0x0, PORT_ID_EAG6L_PORT3, 0, NULL, sys_fpga_memory_read, set_rtwdm_loopback }, 
  { PORT_4_CONF_ADDR,     0x10, 4, 0x0, PORT_ID_EAG6L_PORT4, 0, NULL, sys_fpga_memory_read, set_rtwdm_loopback }, 
  { PORT_5_CONF_ADDR,     0x10, 4, 0x0, PORT_ID_EAG6L_PORT5, 0, NULL, sys_fpga_memory_read, set_rtwdm_loopback }, 
  { PORT_6_CONF_ADDR,     0x10, 4, 0x0, PORT_ID_EAG6L_PORT6, 0, NULL, sys_fpga_memory_read, set_rtwdm_loopback }, 
#if 0 /* [#151] Implementing P7 config register, dustin, 2024-10-21 */
  { PORT_7_CONF_ADDR,     0x10, 4, 0x0, PORT_ID_EAG6L_PORT7, 0, NULL, sys_fpga_memory_read, set_rtwdm_loopback }, 
#endif
	/* port configuration - smart t-sfp self loopback */
  { PORT_1_CONF_ADDR,     0x20, 5, 0x0, PORT_ID_EAG6L_PORT1, 0, NULL, sys_fpga_memory_read, set_smart_tsfp_self_loopback }, 
  { PORT_2_CONF_ADDR,     0x20, 5, 0x0, PORT_ID_EAG6L_PORT2, 0, NULL, sys_fpga_memory_read, set_smart_tsfp_self_loopback }, 
  { PORT_3_CONF_ADDR,     0x20, 5, 0x0, PORT_ID_EAG6L_PORT3, 0, NULL, sys_fpga_memory_read, set_smart_tsfp_self_loopback }, 
  { PORT_4_CONF_ADDR,     0x20, 5, 0x0, PORT_ID_EAG6L_PORT4, 0, NULL, sys_fpga_memory_read, set_smart_tsfp_self_loopback }, /*30*/ 
  { PORT_5_CONF_ADDR,     0x20, 5, 0x0, PORT_ID_EAG6L_PORT5, 0, NULL, sys_fpga_memory_read, set_smart_tsfp_self_loopback }, 
  { PORT_6_CONF_ADDR,     0x20, 5, 0x0, PORT_ID_EAG6L_PORT6, 0, NULL, sys_fpga_memory_read, set_smart_tsfp_self_loopback }, 
#if 0 /* [#151] Implementing P7 config register, dustin, 2024-10-21 */
  { PORT_7_CONF_ADDR,     0x20, 5, 0x0, PORT_ID_EAG6L_PORT7, 0, NULL, sys_fpga_memory_read, set_smart_tsfp_self_loopback }, 
#endif
	/* alarm mask - flex tune reset */
  { PORT_1_ALM_MASK_ADDR, 0x20, 5, 0x0, PORT_ID_EAG6L_PORT1, 0, NULL, sys_fpga_memory_read, set_flex_tune_reset }, 
  { PORT_2_ALM_MASK_ADDR, 0x20, 5, 0x0, PORT_ID_EAG6L_PORT2, 0, NULL, sys_fpga_memory_read, set_flex_tune_reset }, 
  { PORT_3_ALM_MASK_ADDR, 0x20, 5, 0x0, PORT_ID_EAG6L_PORT3, 0, NULL, sys_fpga_memory_read, set_flex_tune_reset }, 
  { PORT_4_ALM_MASK_ADDR, 0x20, 5, 0x0, PORT_ID_EAG6L_PORT4, 0, NULL, sys_fpga_memory_read, set_flex_tune_reset }, 
  { PORT_5_ALM_MASK_ADDR, 0x20, 5, 0x0, PORT_ID_EAG6L_PORT5, 0, NULL, sys_fpga_memory_read, set_flex_tune_reset }, 
  { PORT_6_ALM_MASK_ADDR, 0x20, 5, 0x0, PORT_ID_EAG6L_PORT6, 0, NULL, sys_fpga_memory_read, set_flex_tune_reset }, 
  { PORT_7_ALM_MASK_ADDR, 0x20, 5, 0x0, PORT_ID_EAG6L_PORT7, 0, NULL, sys_fpga_memory_read, set_flex_tune_reset }, /*40*/ 
#if 1 /* [#100] Adding update of Laser status by Laser_con, dustin, 2024-08-23 */
	/* port configuration - laser control */
  { PORT_1_CONF_ADDR,     0xFF00, 8, 0x5A, PORT_ID_EAG6L_PORT1, 0, NULL, sys_fpga_memory_read, update_laser_status }, 
  { PORT_2_CONF_ADDR,     0xFF00, 8, 0x5A, PORT_ID_EAG6L_PORT2, 0, NULL, sys_fpga_memory_read, update_laser_status }, 
  { PORT_3_CONF_ADDR,     0xFF00, 8, 0x5A, PORT_ID_EAG6L_PORT3, 0, NULL, sys_fpga_memory_read, update_laser_status }, 
  { PORT_4_CONF_ADDR,     0xFF00, 8, 0x5A, PORT_ID_EAG6L_PORT4, 0, NULL, sys_fpga_memory_read, update_laser_status }, /*30*/ 
  { PORT_5_CONF_ADDR,     0xFF00, 8, 0x5A, PORT_ID_EAG6L_PORT5, 0, NULL, sys_fpga_memory_read, update_laser_status }, 
  { PORT_6_CONF_ADDR,     0xFF00, 8, 0x5A, PORT_ID_EAG6L_PORT6, 0, NULL, sys_fpga_memory_read, update_laser_status }, 
  { PORT_7_CONF_ADDR,     0xFF00, 8, 0x5A, PORT_ID_EAG6L_PORT7, 0, NULL, sys_fpga_memory_read, update_laser_status }, 
#endif
#if 1/*[#61] Adding omitted functions, dustin, 2024-06-24 */
#if 1 /* [#125] Fixing for SFP channel no, wavelength, tx/rx dBm, dustin, 2024-09-10 */
	/* channel number set */
  { PORT_1_SET_CH_NUM_ADDR,  0xFFFF, 0, 0x0, PORT_ID_EAG6L_PORT1, 0, NULL, sys_fpga_memory_read, set_tunable_sfp_channel_no },
  { PORT_2_SET_CH_NUM_ADDR,  0xFFFF, 0, 0x0, PORT_ID_EAG6L_PORT2, 0, NULL, sys_fpga_memory_read, set_tunable_sfp_channel_no },
  { PORT_3_SET_CH_NUM_ADDR,  0xFFFF, 0, 0x0, PORT_ID_EAG6L_PORT3, 0, NULL, sys_fpga_memory_read, set_tunable_sfp_channel_no },
  { PORT_4_SET_CH_NUM_ADDR,  0xFFFF, 0, 0x0, PORT_ID_EAG6L_PORT4, 0, NULL, sys_fpga_memory_read, set_tunable_sfp_channel_no },
  { PORT_5_SET_CH_NUM_ADDR,  0xFFFF, 0, 0x0, PORT_ID_EAG6L_PORT5, 0, NULL, sys_fpga_memory_read, set_tunable_sfp_channel_no },
  { PORT_6_SET_CH_NUM_ADDR,  0xFFFF, 0, 0x0, PORT_ID_EAG6L_PORT6, 0, NULL, sys_fpga_memory_read, set_tunable_sfp_channel_no },
#if 1 /* [#94] Adding for 100G DCO handling, dustin, 2024-09-23 */
  { PORT_7_SET_CH_NUM_ADDR,  0xFFFF, 0, 0x0, PORT_ID_EAG6L_PORT7, 0, NULL, sys_fpga_memory_read, set_dco_sfp_channel_no },
#else
  { PORT_7_SET_CH_NUM_ADDR,  0xFFFF, 0, 0x0, PORT_ID_EAG6L_PORT7, 0, NULL, sys_fpga_memory_read, set_tunable_sfp_channel_no },
#endif
#else
	/* channel number set */
	/* FIXME : there are no channel table for 100G */
  { PORT_1_SET_CH_NUM_ADDR,  0x0, 0, 0x0, PORT_ID_EAG6L_PORT1, 0, NULL, sys_fpga_memory_read, set_tunable_sfp_channel_no },
  { PORT_2_SET_CH_NUM_ADDR,  0x0, 0, 0x0, PORT_ID_EAG6L_PORT2, 0, NULL, sys_fpga_memory_read, set_tunable_sfp_channel_no },
  { PORT_3_SET_CH_NUM_ADDR,  0x0, 0, 0x0, PORT_ID_EAG6L_PORT3, 0, NULL, sys_fpga_memory_read, set_tunable_sfp_channel_no },
  { PORT_4_SET_CH_NUM_ADDR,  0x0, 0, 0x0, PORT_ID_EAG6L_PORT4, 0, NULL, sys_fpga_memory_read, set_tunable_sfp_channel_no },
  { PORT_5_SET_CH_NUM_ADDR,  0x0, 0, 0x0, PORT_ID_EAG6L_PORT5, 0, NULL, sys_fpga_memory_read, set_tunable_sfp_channel_no },
  { PORT_6_SET_CH_NUM_ADDR,  0x0, 0, 0x0, PORT_ID_EAG6L_PORT6, 0, NULL, sys_fpga_memory_read, set_tunable_sfp_channel_no },
#endif /* [#125] */
#endif
#if 1 /* [#94] Adding for 100G DCO handling, dustin, 2024-09-23 */
#if 1 /* [#152] Adding for port RS-FEC control, dustin, 2024-10-15 */
  { PORT_1_RS_FEC_ADDR,  0xFF00, 8, 0xA5, PORT_ID_EAG6L_PORT1, 0, NULL, sys_fpga_memory_read, portFECEnable },
  { PORT_2_RS_FEC_ADDR,  0x00FF, 0, 0xA5, PORT_ID_EAG6L_PORT2, 0, NULL, sys_fpga_memory_read, portFECEnable },
  { PORT_3_RS_FEC_ADDR,  0xFF00, 8, 0xA5, PORT_ID_EAG6L_PORT3, 0, NULL, sys_fpga_memory_read, portFECEnable },
  { PORT_4_RS_FEC_ADDR,  0x00FF, 0, 0xA5, PORT_ID_EAG6L_PORT4, 0, NULL, sys_fpga_memory_read, portFECEnable },
  { PORT_5_RS_FEC_ADDR,  0xFF00, 8, 0xA5, PORT_ID_EAG6L_PORT5, 0, NULL, sys_fpga_memory_read, portFECEnable },
  { PORT_6_RS_FEC_ADDR,  0x00FF, 0, 0xA5, PORT_ID_EAG6L_PORT6, 0, NULL, sys_fpga_memory_read, portFECEnable },
#if 1 /* [#154] Fixing for auto FEC mode on DCO, dustin, 2024-10-21 */
  { PORT_7_RS_FEC_ADDR,  0xFF00, 8, 0x0,  PORT_ID_EAG6L_PORT7, 0, NULL, sys_fpga_memory_read, portFECEnable },
#else
  { PORT_7_RS_FEC_ADDR,  0xFF00, 8, 0xA5, PORT_ID_EAG6L_PORT7, 0, NULL, sys_fpga_memory_read, portFECEnable },
#endif
#endif

  { QSFP28_RESET_ADDR,  0xFF, 0, 0x0, PORT_ID_EAG6L_NOT_USE, 0, NULL, sys_fpga_memory_read, DcoReset },
  { QSFP28_FEC_ENABLE_ADDR,  0xFFFF, 0, 0x5AA5, PORT_ID_EAG6L_NOT_USE, 0, NULL, sys_fpga_memory_read, DcoFECEnable },
  { QSFP28_COUNT_RESET_ADDR,  0xFF, 0, 0x0, PORT_ID_EAG6L_NOT_USE, 0, NULL, sys_fpga_memory_read, DcoCountReset },
#endif /* [#94] */
	/* synce global control */
  { SYNCE_GCONFIG_ADDR,   0xFF, 0, 0x5A, PORT_ID_EAG6L_NOT_USE, 0, NULL, sys_fpga_memory_read, synceEnableSet }, 
#if 1/*[#118] Sync-e option2 지원, balkrow, 2024-09-06*/
  { SYNCE_ESMC_LQL_ADDR,  0xFF00, 8, 0x0, PORT_ID_EAG6L_NOT_USE, 0, NULL, sys_fpga_memory_read, syncePortSendQL }, 
  { SYNCE_ESMC_LQL_ADDR,   0xFF, 0, 0x12, PORT_ID_EAG6L_NOT_USE, 0, NULL, sys_fpga_memory_read, synceLocalQL }, 
#endif
	/* synce interface select */
#if 1/*[#56] register update timer 수정, balkrow, 2023-06-13 */
#if 1/*[#65] Adding regMon simulation feature under ACCESS_SIM, dustin, 2024-06-24 */
  { SYNCE_IF_SELECT_ADDR, 0xFF00, 8, 0x0, PORT_ID_EAG6L_NOT_USE, 0, NULL, sys_fpga_memory_read, synceIFPriSelect }, 
#else
  { SYNCE_IF_SELECT_ADDR, 0xFF, 8, 0x0, PORT_ID_EAG6L_NOT_USE, 0, NULL,  sys_fpga_memory_read, synceIFPriSelect }, 
#endif
  { SYNCE_IF_SELECT_ADDR, 0xFF, 0, 0x0, PORT_ID_EAG6L_NOT_USE, 0, NULL,  sys_fpga_memory_read, synceIFSecSelect }, 
#else
  { SYNCE_IF_SELECT_ADDR, 0xFFFF, 0, 0x0, PORT_ID_EAG6L_NOT_USE, 0, NULL, sys_fpga_memory_read, synceIFSelect }, 
#endif
	/* pm counter clear */
  { PM_COUNT_CLEAR_ADDR,  0xFF, 0, 0x0, PORT_ID_EAG6L_NOT_USE, 0, NULL, sys_fpga_memory_read, pmClear }, 
#if 0/*[#99] EAG6L 개발 검증 이슈 F/U, balkrow, 2024-08-29*/
	/* chip reset */
  { CHIP_RESET_ADDR,  0xFFFF, 0, 0x0, PORT_ID_EAG6L_NOT_USE, 0, NULL, sys_fpga_memory_read, chipReset }, 
	/* board status - sfp cr */
	/* FIXME : fix entry */
#endif
#if 1 /* [#91] Fixing for register updating feature, dustin, 2024-08-05 */
  { BD_SFP_CR_ADDR,  0x7F, 0, 0x7F, PORT_ID_EAG6L_NOT_USE, 0, NULL, sys_fpga_memory_read, boardStatus },
#else
  { BD_SFP_CR_ADDR,  0x7F, 0, 0x0, PORT_ID_EAG6L_NOT_USE, 0, NULL, sys_fpga_memory_read, boardStatus },
#endif
	/* fpga bank select */
#if 1/*[#61] Adding omitted functions, dustin, 2024-06-24 */
#if 1 /* [#91] Fixing for register updating feature, dustin, 2024-08-05 */
  { FW_BANK_SELECT_ADDR,  0x7, 0, 0x0, PORT_ID_EAG6L_NOT_USE, 0, NULL, sys_fpga_memory_read, bankSelect2 },
#else
  { FW_BANK_SELECT_ADDR,  0x30, 8, 0x0, PORT_ID_EAG6L_NOT_USE, 0, NULL, sys_fpga_memory_read, bankSelect1 },
  { FW_BANK_SELECT_ADDR,  0x03, 0, 0x0, PORT_ID_EAG6L_NOT_USE, 0, NULL, sys_fpga_memory_read, bankSelect2 },
#endif
#endif
	/* dco register */
	/* FIXME : add entry */
#if 1 /* [#142] Adding for Transparent mode switching, dustin, 2024-10-11 */
  { SWITCH_MODE_ADDR,  0x3, 0, 0x0, PORT_ID_EAG6L_NOT_USE, 0, NULL, sys_fpga_memory_read, swModeSet },
#endif
};
#endif /*End 62*/
#endif

uint16_t regMonArrSize = sizeof(regMonList) / sizeof(RegMON);

#if 1 /* [#85] Fixing for resetting PM counter for unexpected FEC counting, dustin, 2024-07-31 */
void pm_clear_fec_counters(struct thread *thread)
{
	uint8_t portno = thread->arg;

	pmFECClear(portno);
	return 0;
}
#endif

#if 0/*[#51] Adding register callback templates for config/command registers, dustin, 2024-06-12 */
uint16_t portRateSet (uint16_t port)
{
	port = port;
	return 0;
}

uint16_t portRateSet (uint16_t port, uint16_t val)
{
	extern void port_config_speed(int port, long speed, int mode);

	if(val == 0x7/*25G*/)
		port_config_speed(port, PORT_IF_25G_KR, PORT_IF_25G_KR);
	else
		port_config_speed(port, PORT_IF_10G_KR, PORT_IF_10G_KR);
	return SUCCESS;
}
#endif

#if 1 /* [#62] SFP eeprom 및 register update 기능 단위 검증 및 디버깅, balkrow, 2024-06-21 */ 
uint16_t getIdxFromRegMonList (uint16_t reg)
{
	uint16_t i, ret = 0xff;
	for(i = 0; i < regMonArrSize; i++)
	{
		if(regMonList[i].reg == reg)
			return i;
	}
	return ret;
}
#endif

#if 1/*[#120] LOC Alarm process ¿¿, balkrow, 2024-10-16 */
uint16_t portESMCenable (uint16_t port, uint16_t val)
{
	uint16_t rc = RT_OK;
	/*TODO: */
#if 1 /*[#82] eag6l board SW Debugging, balkrow, 2024-08-09*/
	if(val)
	{
#ifdef DEBUG 
		zlog_notice("port %x oper_port %x pri %x sec %x", port, gDB.synce_oper_port, 
			    gDB.synce_pri_port, gDB.synce_sec_port);
#endif

		if(port == getMPortByCport(gDB.synce_pri_port))  
		{
#if 1
			int sec_port;
			sec_port = getMPortByCport(gDB.synce_sec_port);
			zlog_notice("port %x ESMC enable", port);

			gCpssSynceIfConf(3, PRI_SRC, gDB.synce_pri_port, 0);
			PORT_STATUS[port].esmc_recv_cnt = 0;
			PORT_STATUS[port].esmc_prev_cnt = 0;
			PORT_STATUS[port].received_QL = 0;

			if(sec_port != NOT_DEFINED)
			{

				zlog_notice("port %x reconfigure", sec_port);
				gCpssSynceIfConf(3, SEC_SRC, gDB.synce_sec_port, 0);
				PORT_STATUS[sec_port].esmc_recv_cnt = 0;
				PORT_STATUS[sec_port].esmc_prev_cnt = 0;
				PORT_STATUS[sec_port].received_QL = 0;
			}
#else
			int16_t sec_port = getMPortByCport(gDB.synce_sec_port);
			zlog_notice("port %x ESMC enable", port);

			if(gDB.esmcRxCfg[port - 1] != CFG_ENABLE) 
			{
				gCpssSynceIfConf(3, PRI_SRC, gDB.synce_pri_port, 0);
				gCpssSynceIfConf(3, SEC_SRC, gDB.synce_sec_port, 0);
			}

			if(sec_port != NOT_DEFINED && gDB.esmcRxCfg[sec_port - 1] == CFG_ENABLE) 
			{
				zlog_notice("port %x reconfigure", port);
				gCpssSynceIfConf(3, SEC_SRC, gDB.synce_sec_port, 0);
			}
#endif

			gDB.synce_oper_port = NOT_DEFINED; 
		}
		else if(port == getMPortByCport(gDB.synce_sec_port))  
		{
			int16_t pri_port = getMPortByCport(gDB.synce_pri_port);
			zlog_notice("port %x ESMC enable", port);
#if 1
			gCpssSynceIfConf(3, SEC_SRC, gDB.synce_sec_port, 0);
			PORT_STATUS[port].esmc_recv_cnt = 0;
			PORT_STATUS[port].esmc_prev_cnt = 0;
			PORT_STATUS[port].received_QL = 0;

			if(pri_port != NOT_DEFINED)
			{
				zlog_notice("port %x reconfigure", pri_port);
				gCpssSynceIfConf(3, PRI_SRC, gDB.synce_pri_port, 0);
				PORT_STATUS[pri_port].esmc_recv_cnt = 0;
				PORT_STATUS[pri_port].esmc_prev_cnt = 0;
				PORT_STATUS[pri_port].received_QL = 0;
			}
#else
			if(gDB.esmcRxCfg[port - 1] != CFG_ENABLE) 
			{
				gCpssSynceIfConf(3, PRI_SRC, gDB.synce_pri_port, 0);
				gCpssSynceIfConf(3, SEC_SRC, gDB.synce_sec_port, 0);
			}

			if(pri_port != NOT_DEFINED && gDB.esmcRxCfg[pri_port - 1] == CFG_ENABLE) 
			{
				zlog_notice("port %x reconfigure", port);
				gCpssSynceIfConf(3, PRI_SRC, gDB.synce_pri_port, 1);
			}
#endif

			gDB.synce_oper_port = NOT_DEFINED; 
		}
		gDB.esmcRxCfg[port - 1] = CFG_ENABLE;
	
	}
	else
	{
		int pri_port = getMPortByCport(gDB.synce_pri_port);
		int sec_port = getMPortByCport(gDB.synce_sec_port);

		zlog_notice("port %x ESMC disable", port);
		gDB.esmcRxCfg[port - 1] = CFG_DISABLE;

		if(pri_port != NOT_DEFINED && gDB.esmcRxCfg[pri_port - 1] != CFG_ENABLE) 
		{
			if(sec_port != NOT_DEFINED  && gDB.esmcRxCfg[sec_port - 1] != CFG_ENABLE)
			{
				gCpssSynceIfConf(3, PRI_SRC, gDB.synce_pri_port, 1);
				gCpssSynceIfConf(3, SEC_SRC, gDB.synce_sec_port, 1);
				gDB.synce_oper_port = pri_port;
				PORT_STATUS[pri_port].esmc_recv_cnt = 0;
				PORT_STATUS[pri_port].esmc_prev_cnt = 0;
				PORT_STATUS[pri_port].esmc_loss = 0;
				PORT_STATUS[pri_port].received_QL = 0;
			}
			else if(sec_port == NOT_DEFINED)
			{
				gCpssSynceIfConf(3, SEC_SRC, gDB.synce_pri_port, 1);
				gDB.synce_oper_port = pri_port;
				PORT_STATUS[pri_port].esmc_recv_cnt = 0;
				PORT_STATUS[pri_port].esmc_prev_cnt = 0;
				PORT_STATUS[pri_port].esmc_loss = 0;
				PORT_STATUS[pri_port].received_QL = 0;
			}
#if 1/*[#120] LOC Alarm process ¿¿, balkrow, 2024-10-16 */
			gRegUpdate(SYNCE_ESMC_RQL_ADDR, 8, 0xff00, 0);
#endif
		}
		if(sec_port != NOT_DEFINED && gDB.esmcRxCfg[sec_port - 1] != CFG_ENABLE) 
		{
			if(pri_port != NOT_DEFINED  && gDB.esmcRxCfg[pri_port - 1] != CFG_ENABLE)
			{
				gCpssSynceIfConf(3, PRI_SRC, gDB.synce_pri_port, 1);
				gCpssSynceIfConf(3, SEC_SRC, gDB.synce_sec_port, 1);
				gDB.synce_oper_port = pri_port;
				PORT_STATUS[sec_port].esmc_recv_cnt = 0;
				PORT_STATUS[sec_port].esmc_prev_cnt = 0;
				PORT_STATUS[sec_port].esmc_loss = 0;
				PORT_STATUS[sec_port].received_QL = 0;
			}
			else if(pri_port == NOT_DEFINED)
			{
				gCpssSynceIfConf(3, SEC_SRC, gDB.synce_sec_port, 1);
				gDB.synce_oper_port = sec_port;
				PORT_STATUS[sec_port].esmc_recv_cnt = 0;
				PORT_STATUS[sec_port].esmc_prev_cnt = 0;
				PORT_STATUS[sec_port].esmc_loss = 0;
				PORT_STATUS[sec_port].received_QL = 0;
			}
#if 1/*[#120] LOC Alarm process ¿¿, balkrow, 2024-10-16 */
			gRegUpdate(SYNCE_ESMC_RQL_ADDR, 0, 0xff, 0);
#endif
		}

	}
	rc = gSysmonToCpssFuncs[gPortESMCenable](2, port, val);
#endif
	return rc;
}

uint16_t portRateSet (uint16_t port, uint16_t val)
{
	uint16_t rc = RT_OK;

#if 1/* NOTE : to avoid unexpected calling before sdk init. */
#if 1/*[#61] Adding omitted functions, dustin, 2024-06-24 */
	if(! PORT_STATUS[port].equip)/*not-installed*/
		return RT_NOK;
#endif
#endif

#if 1/*[#80] eag6l board SW bring-up, balkrow, 2023-07-22 */
#if 1 /* [#85] Fixing for resetting PM counter for unexpected FEC counting, dustin, 2024-07-31 */
#if 1 /* [#102] Fixing some register updates, dustin, 2024-08-26 */
	if(val == 0x9/*25G*/)
#else
	if(val == 0x7/*25G*/)
#endif
		rc = gSysmonToCpssFuncs[gPortSetRate](3, port, PORT_IF_25G_KR, PORT_IF_25G_KR);
#if 1 /* [#102] Fixing some register updates, dustin, 2024-08-26 */
	else if(val == 0x8/*10G*/)
#else
	else if(val == 0x6/*10G*/)
#endif
		rc = gSysmonToCpssFuncs[gPortSetRate](3, port, PORT_IF_10G_KR, PORT_IF_10G_KR);
#else
	if(val == 0x7/*25G*/)
		rc = gSysmonToCpssFuncs[gPortSetRate](2, port, PORT_IF_25G_SR_LR);
	else if(val == 0x6/*10G*/)
		rc = gSysmonToCpssFuncs[gPortSetRate](2, port, PORT_IF_10G_SR_LR);
#endif
#else
	if(val == 0x7/*25G*/)
		rc = gSysmonToCpssFuncs[gPortSetRate](3, port, PORT_IF_25G_KR, PORT_IF_25G_KR);
	else if(val == 0x6/*10G*/)
		rc = gSysmonToCpssFuncs[gPortSetRate](3, port, PORT_IF_10G_KR, PORT_IF_10G_KR);
#endif
	else
		rc = RT_NOK;

#if 1 /* [#85] Fixing for resetting PM counter for unexpected FEC counting, dustin, 2024-07-31 */
#if 1 /* [#102] Fixing some register updates, dustin, 2024-08-26 */
	if((val == 0x9/*25G*/) && (rc != RT_NOK))
#else
	if((val == 0x7/*25G*/) && (rc != RT_NOK))
#endif
		thread_add_timer(master, pm_clear_fec_counters, port, 2);
#endif

	return rc;
}

#if 1/*[#59] Synce configuration 연동 기능 추가, balkrow, 2024-06-19 */
uint16_t synceEnableSet(uint16_t port, uint16_t val)
{
	uint16_t rc = RT_OK;
	port = port;
#if 1/*[#80] eag6l board SW bring-up, balkrow, 2023-07-24 */
	if(val == 0xa5) {
		rc = gSysmonToCpssFuncs[gSynceEnable](1, 1);
		gDB.synce_state = CFG_ENABLE;
	} else if(val == 0x5a) {
		rc = gSysmonToCpssFuncs[gSynceDisable](1, 0);
		gDB.synce_state = CFG_DISABLE;
	}
#endif
	return rc;	
}
#endif

#endif

#if 1/*[#118] Sync-e option2 지원, balkrow, 2024-09-06*/
uint16_t syncePortSendQL(uint16_t port, uint16_t val)
{
	uint16_t rc = RT_OK;
	rc = gSysmonToCpssFuncs[gPortSendQL](1, val);
	return rc;
}

uint16_t synceLocalQL(uint16_t port, uint16_t val)
{
	uint16_t rc = RT_OK;
	rc = gSysmonToCpssFuncs[gPortLocalQL](1, val);
#if 1/*[#122] primary/secondary Send QL 설정, balkrow, 2024-09-09*/
	gDB.localQL = val;
#endif
	zlog_notice("Local QL %x", val);
	return rc;
}
#endif /* [#118] */

#if 1/*[#56] register update timer 수정, balkrow, 2023-06-13 */
uint16_t synceIFPriSelect(uint16_t port, uint16_t val)
{
	uint16_t rc = RT_OK;

	switch(val)
	{
#if 1 /* [#102] Fixing some register updates, dustin, 2024-08-26 */
		case 0x01 : 
			port = 0; 
			break;
		case 0x02 : 
			port = 8; 
			break;
		case 0x03 : 
			port = 16; 
			break;
		case 0x04 : 
			port = 24; 
			break;
		case 0x05 : 
			port = 32; 
			break;
		case 0x06 : 
			port = 40; 
			break;
		case 0x07 : 
			port = 50; 
			break;
#else
		case 0x11 : 
			port = 0; 
			break;
		case 0x12 : 
			port = 8; 
			break;
		case 0x13 : 
			port = 16; 
			break;
		case 0x14 : 
			port = 24; 
			break;
		case 0x21 : 
			port = 32; 
			break;
		case 0x22 : 
			port = 40; 
			break;
		case 0x23 : 
			port = 50; 
			break;
#endif
		default :
			port = 0xff; 
			break;
	}

#if 1/*[#122] primary/secondary Send QL 설정, balkrow, 2024-09-09*/
	if(port != 0xff)
	{
		rc = gSysmonToCpssFuncs[gSynceIfSelect](2, PRI_SRC, port);
#if 1/*[#120] LOC Alarm process ¿¿, balkrow, 2024-10-16 */
		gRegUpdate(SYNCE_ESMC_SQL_ADDR, 8, 0xff00, gDB.localQL);
#endif
	}
#endif
	else
		rc = RT_NOK;

	return rc;
}

uint16_t synceIFSecSelect(uint16_t port, uint16_t val)
{
	uint16_t rc = RT_OK;

	switch(val)
	{
#if 1 /* [#102] Fixing some register updates, dustin, 2024-08-26 */
		case 0x01 : 
			port = 0; 
			break;
		case 0x02 : 
			port = 8; 
			break;
		case 0x03 : 
			port = 16; 
			break;
		case 0x04 : 
			port = 24; 
			break;
		case 0x05 : 
			port = 32; 
			break;
		case 0x06 : 
			port = 40; 
			break;
		case 0x07 : 
			port = 50; 
			break;
#else
		case 0x11 : 
			port = 0; 
			break;
		case 0x12 : 
			port = 8; 
			break;
		case 0x13 : 
			port = 16; 
			break;
		case 0x14 : 
			port = 24; 
			break;
		case 0x21 : 
			port = 32; 
			break;
		case 0x22 : 
			port = 40; 
			break;
		case 0x23 : 
			port = 50; 
			break;
#endif
		default :
			port = 0xff; 
			break;
	}

#if 1/*[#122] primary/secondary Send QL 설정, balkrow, 2024-09-09*/
	if(port != 0xff)
	{
		zlog_notice("%s:%d", __func__, __LINE__);
		rc = gSysmonToCpssFuncs[gSynceIfSelect](2, SEC_SRC, port);
#if 0/*[#120] LOC Alarm process ¿¿, balkrow, 2024-10-16 */
		gRegUpdate(SYNCE_ESMC_SQL_ADDR, 0, 0xff, gDB.localQL);
#endif
	}
#endif
	else
		rc = RT_NOK;

	return rc;
}
#endif

#if 1/*[#48] register monitoring and update 관련 기능 추가, balkrow, 2024-06-10*/ 
uint16_t pmClear(uint16_t port, uint16_t val)
{
	uint16_t rc = RT_OK;
	port  = port;

	if(val != 0xA5)
		return SUCCESS;

	gSysmonToCpssFuncs[gPortPMClear](1, 1);

#if 1 /* [#85] Fixing for resetting PM counter for unexpected FEC counting, dustin, 2024-07-31 */
	/* clear pm counter for resetting fec counter. */
	memset(&(PM_TBL[port]), 0, sizeof(port_pm_counter_t));
#endif
	return SUCCESS;
}
#else
uint16_t pmClear(uint16_t port, uint16_t val)
{
extern void pm_request_clear(void);

	port;
	if(val != 0xA5)
		return SUCCESS;

	pm_request_clear();
	return SUCCESS;
}
#endif

#if 1 /* [#85] Fixing for resetting PM counter for unexpected FEC counting, dustin, 2024-07-31 */
uint16_t pmFECClear(uint16_t port)
{
	gSysmonToCpssFuncs[gPortPMFECClear](1, port);
	PM_TBL[port].fcs_ok  = 0;
	PM_TBL[port].fcs_nok = 0;
}
#endif

uint16_t portAlarm(void)
{
	gSysmonToCpssFuncs[gPortAlarm](0);
	return SUCCESS;
}

uint16_t chipReset(uint16_t port, uint16_t val)
{
	port;

	if(((val >> 8) & 0xFF) == 0xA5)
		;/*FIXME : need reset function for fpga. */
	else if((val & 0xFF) == 0xA5) {
		zlog_notice("[SYSMON] reboot EAG6L BP system...");
		sleep(1);
		system("reboot -nf");
	}
	return SUCCESS;
}

#if 1 /* [#151] Implementing P7 config register, dustin, 2024-10-21 */
uint16_t setLLCFenable(uint16_t val)
{
	uint8_t enable = (val == 0x2) ? 1/*enable*/ : 0/*disable*/;
zlog_notice("P7 setLLCFenable set to [%d].", enable);//ZZPP
	sysmon_llcf_set(enable);
	PORT_STATUS[PORT_ID_EAG6L_PORT7].cfg_llcf = enable;

	return SUCCESS;
}
#endif

#if 1 /* [#107] Fixing for 2nd register updates, dustin, 2024-08-29 */
/* NOTE : this function is called once sfp is installed. */
void port_scan_sfp(struct thread *thread)
{
extern ePrivateSfpId get_private_sfp_identifier(int portno);

	uint8_t type, port = thread->arg;
	uint16_t data;

#if 1 /* [#169] Fixing for new DCO install process, dustin, 2024-10-25 */
	/* treat 100g diffrently. */
	if(port == PORT_ID_EAG6L_PORT7/*100g*/) {
		PORT_STATUS[port].equip = 1;/*installed*/
		DCO_STAT.dco_initState = DCO_INIT_START;

		if(! thread_kill_flag) {
			zlog_notice("%s : init 100G start~", __func__);
			dco_retry_cnt = 0;
			thread_add_timer(master, init_100g_sfp, NULL, 1);
			thread_kill_flag++;
		}
		return;
	}

	/* NOTE : let pass through to 25G ports. */
	zlog_notice("%s : init 25G start~ for port[%d].", __func__, port);
#endif /* [#169] */

	read_port_inventory(port, &(INV_TBL[port]));

	/* get private sfp identifier */
	type = get_private_sfp_identifier(port);
	/* get wavelength register 2 */
#if 1 /* [#161] Fixing for processing alarm flag, dustin, 2024-10-22 */
	data = FPGA_PORT_READ(__PORT_WL2_ADDR[port]);
#else
	data = FPGA_READ(__PORT_WL2_ADDR[port]);
#endif

	/* update wavelength register 2 */
	data &= ~0x0F00;
	data |= (type << 8);
	FPGA_PORT_WRITE(__PORT_WL2_ADDR[port], data);

	PORT_STATUS[port].sfp_type = type;
	PORT_STATUS[port].equip = 1;/*installed*/

#if 0 /* [#169] Fixing for new DCO install process, dustin, 2024-10-25 */
	/* init for 100G DCO sfp. FIXME: need OE spf. */
	if(! memcmp(INV_TBL[port].part_num, "FTLC3351R3PL1", 
		sizeof("FTLC3351R3PL1")))
	{
		extern void init_100G_sfp(void);

#if 1 /* [#154] Fixing for auto FEC mode on DCO, dustin, 2024-10-21 */
		PORT_STATUS[port].sfp_dco = 1;
#endif
		init_100g_sfp();
	}
#endif /* [#169] */

#if 1 /* [#154] Fixing for auto FEC mode on DCO, dustin, 2024-10-21 */
	/* set port fec as configured. */
	data = FPGA_PORT_READ(__PORT_RS_FEC_ADDR[port]);
	portFECEnable(port, data);
#endif

#if 0 /* [#169] Fixing for new DCO install process, dustin, 2024-10-25 */
	/* move below to init_100g_sfp. */
#if 1 /* [#94] Adding for 100G DCO handling, dustin, 2024-09-23 */
	if(port >= (PORT_ID_EAG6L_MAX - 1)) {
#if 1 /* [#154] Fixing for auto FEC mode on DCO, dustin, 2024-10-21 */
		/* set sfp host fec as configured. */
		data = FPGA_PORT_READ(QSFP28_FEC_ENABLE_ADDR);
#if 1 /* [#154] Fixing for auto FEC mode on DCO, dustin, 2024-10-21 */
		DcoFECEnable(port, data);
#else
		DcoFECEnable(data);
#endif
#endif
		/* read ch no register */
		if(PORT_STATUS[port].tunable_sfp) {
			PORT_STATUS[port].tunable_chno = FPGA_PORT_READ(PORT_7_SET_CH_NUM_ADDR);

			/* set 100G DCO chno. */
			if(PORT_STATUS[port].tunable_chno)
				set_dco_sfp_channel_no(port, PORT_STATUS[port].tunable_chno);
		}
	} else
#endif /* [#94] */
#endif /* [#169] */
	if(PORT_STATUS[port].tunable_sfp) {
		/* get inventory */
		if(PORT_STATUS[port].tunable_sfp) {
			read_port_rtwdm_inventory(port, &(RTWDM_INV_TBL[port]));
		}

		/* set flex tune if configured */
		if(PORT_STATUS[port].cfg_flex_tune)
			set_flex_tune_control(port, 1/*enable*/);
#if 1 /* [#132] Fixing for tunable sfp ch no., dustin, 2024-09-24 */
		else
			set_flex_tune_control(port, 0/*disable*/);
#endif /* [#132] */

		/* set smart t-sfp self loopback if configured. */
		if(PORT_STATUS[port].cfg_smart_tsfp_selfloopback)
			set_smart_tsfp_self_loopback(port, 1/*enable*/);
#if 1 /* [#132] Fixing for tunable sfp ch no., dustin, 2024-09-24 */
		else
			set_smart_tsfp_self_loopback(port, 0/*disable*/);
#endif /* [#132] */

		/* set rtwdm loopback if configured. */
		if(PORT_STATUS[port].cfg_rtwdm_loopback)
			set_rtwdm_loopback(port, 1/*enable*/);
#if 1 /* [#132] Fixing for tunable sfp ch no., dustin, 2024-09-24 */
		else
			set_rtwdm_loopback(port, 0/*disable*/);
#endif /* [#132] */

#if 1 /* [#132] Fixing for tunable sfp ch no., dustin, 2024-09-24 */
		if(PORT_STATUS[port].tunable_chno)
			set_tunable_sfp_channel_no(port, PORT_STATUS[port].tunable_chno);
#endif /* [#132] */
	}

	thread_add_timer(master, pm_clear_fec_counters, port, 2);

#if 1 /* [#157] Fixing for Smart T-SFP rtWDM info, dustin, 2024-10-18 */
	update_port_sfp_inventory(0/*update-no-rtwdm*/);
#else
	update_port_sfp_inventory();
#endif
#if 1 /* [#169] Fixing for new DCO install process, dustin, 2024-10-25 */
	PORT_STATUS[port].i2cReady = 1;
#endif

	return;
}
#endif /* [#107] */

uint16_t boardStatus(uint16_t port, uint16_t val)
{
extern int check_sfp_is_present(int portno);
extern ePrivateSfpId get_private_sfp_identifier(int portno);
	double fval;
	uint16_t type, data;
#if 1 /* [#153] Fixing for updating flex tune, dustin, 2024-10-17 */
	uint8_t cfg_esmc_enable;
	uint8_t cfg_flex_tune;
	uint8_t cfg_smart_tsfp_selfloopback;
	uint8_t cfg_rtwdm_loopback;
#endif
#if 1 /* [#169] Fixing for new DCO install process, dustin, 2024-10-25 */
	uint8_t cfg_rs_fec;
	uint8_t cfg_dco_fec;
	uint8_t cfg_ch_data;
#endif
#if 1 /* [#151] Implementing P7 config register, dustin, 2024-10-21 */
	uint8_t cfg_tx_laser;
	uint8_t cfg_llcf;
#endif
#if 1 /* [#91] Fixing for register updating feature, dustin, 2024-08-05 */
static uint16_t SFP_CR_CACHE = 0x7F;
	uint16_t cmask;

	/* get changed bit mask */
	cmask = val ^ SFP_CR_CACHE;
	SFP_CR_CACHE = val;
#endif

#if 1/*[#61] Adding omitted functions, dustin, 2024-06-24 */
	for(port = PORT_ID_EAG6L_PORT1; port < PORT_ID_EAG6L_MAX; port++) {
#if 1 /* [#91] Fixing for register updating feature, dustin, 2024-08-05 */
		/* skip unchanged port */
		if(! (cmask & (1 << (port - 1))))
			continue;
#endif

		if((val & (1 << (port - 1))) != 0/*1-mean-not-installed*/) {
#if 1 /* [#153] Fixing for updating flex tune, dustin, 2024-10-17 */
			/* backup config values. */
			cfg_esmc_enable = PORT_STATUS[port].cfg_esmc_enable;
			cfg_flex_tune   = PORT_STATUS[port].cfg_flex_tune;
			cfg_smart_tsfp_selfloopback = PORT_STATUS[port].cfg_smart_tsfp_selfloopback;
			cfg_rtwdm_loopback = PORT_STATUS[port].cfg_rtwdm_loopback;
#endif
#if 1 /* [#169] Fixing for new DCO install process, dustin, 2024-10-25 */
			cfg_rs_fec   = PORT_STATUS[port].cfg_rs_fec;
			cfg_dco_fec   = PORT_STATUS[port].cfg_dco_fec;
			cfg_ch_data   = PORT_STATUS[port].cfg_ch_data;
#endif
#if 1 /* [#151] Implementing P7 config register, dustin, 2024-10-21 */
			cfg_tx_laser = PORT_STATUS[port].cfg_tx_laser;
			cfg_llcf   = PORT_STATUS[port].cfg_llcf;
#endif
			/* clear spf inventory */
			memset(&(INV_TBL[port]), 0, sizeof(struct module_inventory));
#if 1/* [#72] Adding omitted rtWDM related registers, dustin, 2024-06-27 */
			memset(&(RTWDM_INV_TBL[port]), 0, sizeof(struct module_inventory));
			memset(&(PORT_STATUS[port]), 0, sizeof(port_status_t));
#endif
#if 1 /* [#153] Fixing for updating flex tune, dustin, 2024-10-17 */
			/* restore config values. */
			PORT_STATUS[port].cfg_esmc_enable = cfg_esmc_enable;
			PORT_STATUS[port].cfg_flex_tune   = cfg_flex_tune;
			PORT_STATUS[port].cfg_smart_tsfp_selfloopback = cfg_smart_tsfp_selfloopback;
			PORT_STATUS[port].cfg_rtwdm_loopback = cfg_rtwdm_loopback;
#endif
#if 1 /* [#169] Fixing for new DCO install process, dustin, 2024-10-25 */
			PORT_STATUS[port].cfg_rs_fec = cfg_rs_fec;
			PORT_STATUS[port].cfg_dco_fec = cfg_dco_fec;
			PORT_STATUS[port].cfg_ch_data = cfg_ch_data;
#endif
#if 1 /* [#151] Implementing P7 config register, dustin, 2024-10-21 */
			PORT_STATUS[port].cfg_tx_laser = cfg_tx_laser;
			PORT_STATUS[port].cfg_llcf = cfg_llcf;
#endif

#if 1 /* [#85] Fixing for resetting PM counter for unexpected FEC counting, dustin, 2024-07-31 */
			pmClear(port, 0xA5);
#else
			/* clear pm counter? */
			memset(&(PM_TBL[port]), 0, sizeof(port_pm_counter_t));
#endif

#if 1 /* [#139] Fixing for updating Rx LoS, dustin, 2024-10-01 */
			/* clear not equip sfp registers. */
			if(port >= (PORT_ID_EAG6L_MAX - 1)) {
#if 1 /* [#150] Implementing LR4 Status register, dustin, 2024-10-21 */
				memset(&DCO_STAT, 0, sizeof DCO_STAT);
				memset(&DCO_COUNT, 0, sizeof DCO_COUNT);

				/* update lr4 vcc. */
				FPGA_PORT_WRITE(QSFP28_LR4_VOLT1_ADDR, 0x0);
				FPGA_PORT_WRITE(QSFP28_LR4_VOLT2_ADDR, 0x0);
				FPGA_PORT_WRITE(QSFP28_LR4_VOLT3_ADDR, 0x0);
				FPGA_PORT_WRITE(QSFP28_LR4_VOLT4_ADDR, 0x0);
				/* update lr4 tx bias. */
				FPGA_PORT_WRITE(QSFP28_LR4_TX_BIAS1_ADDR, 0x0);
				FPGA_PORT_WRITE(QSFP28_LR4_TX_BIAS2_ADDR, 0x0);
				FPGA_PORT_WRITE(QSFP28_LR4_TX_BIAS3_ADDR, 0x0);
				FPGA_PORT_WRITE(QSFP28_LR4_TX_BIAS4_ADDR, 0x0);
				/* update lr4 tec current. */
				FPGA_PORT_WRITE(QSFP28_LR4_TCURR1_ADDR, 0x0);
				FPGA_PORT_WRITE(QSFP28_LR4_TCURR2_ADDR, 0x0);
				FPGA_PORT_WRITE(QSFP28_LR4_TCURR3_ADDR, 0x0);
				FPGA_PORT_WRITE(QSFP28_LR4_TCURR4_ADDR, 0x0);
				/* update qsfp28 status. */
				FPGA_PORT_WRITE(QSFP28_STATUS1_ADDR, 0x0);
				FPGA_PORT_WRITE(QSFP28_STATUS2_ADDR, 0x0);
				FPGA_PORT_WRITE(QSFP28_STATUS3_ADDR, 0x0);
				/* update lr4 tx power. */
				FPGA_PORT_WRITE(QSFP28_LR4_TX_POWER1_ADDR, 0x8999);
				FPGA_PORT_WRITE(QSFP28_LR4_TX_POWER2_ADDR, 0x8999);
				FPGA_PORT_WRITE(QSFP28_LR4_TX_POWER3_ADDR, 0x8999);
				FPGA_PORT_WRITE(QSFP28_LR4_TX_POWER4_ADDR, 0x8999);
				/* update lr4 rx power. */
				FPGA_PORT_WRITE(QSFP28_LR4_RX_POWER1_ADDR, 0x8999);
				FPGA_PORT_WRITE(QSFP28_LR4_RX_POWER2_ADDR, 0x8999);
				FPGA_PORT_WRITE(QSFP28_LR4_RX_POWER3_ADDR, 0x8999);
				FPGA_PORT_WRITE(QSFP28_LR4_RX_POWER4_ADDR, 0x8999);
#else
				FPGA_WRITE(QSFP28_STATUS1_ADDR, 0x0);
				FPGA_WRITE(QSFP28_STATUS2_ADDR, 0x0);
				FPGA_WRITE(QSFP28_STATUS3_ADDR, 0x0);
#endif
			} else {
				FPGA_PORT_WRITE(__PORT_TX_PWR_RTWDM_ADDR[port], 0x0);
				FPGA_PORT_WRITE(__PORT_RX_PWR_RTWDM_ADDR[port], 0x0);
				FPGA_PORT_WRITE(__PORT_TEMP_RTWDM_ADDR[port], 0x0);
				FPGA_PORT_WRITE(__PORT_VOLT_RTWDM_ADDR[port], 0x0);
				FPGA_PORT_WRITE(__PORT_TX_BIAS_RTWDM_ADDR[port], 0x0);
				FPGA_PORT_WRITE(__PORT_LTEMP_RTWDM_ADDR[port], 0x0);
				FPGA_PORT_WRITE(__PORT_TCURR_RTWDM_ADDR[port], 0x0);
				FPGA_PORT_WRITE(__PORT_WL1_RTWDM_ADDR[port], 0x0);
				FPGA_PORT_WRITE(__PORT_WL2_RTWDM_ADDR[port], 0x0);
			}

			FPGA_PORT_WRITE(__PORT_TX_PWR_ADDR[port], 0x8999);
			FPGA_PORT_WRITE(__PORT_RX_PWR_ADDR[port], 0x8999);
			FPGA_PORT_WRITE(__PORT_TEMP_ADDR[port], 0x0);
			FPGA_PORT_WRITE(__PORT_VOLT_ADDR[port], 0x0);
			FPGA_PORT_WRITE(__PORT_TX_BIAS_ADDR[port], 0x0);
			FPGA_PORT_WRITE(__PORT_LTEMP_ADDR[port], 0x0);
			FPGA_PORT_WRITE(__PORT_TCURR_ADDR[port], 0x0);

			FPGA_PORT_WRITE(__PORT_WL1_ADDR[port], 0x0);
			FPGA_PORT_WRITE(__PORT_WL2_ADDR[port],  0x0);
			FPGA_PORT_WRITE(__PORT_GET_CH_NUM_ADDR[port], 0x0);
#endif /* [#139] */
#if 1 /* [#157] Fixing for Smart T-SFP rtWDM info, dustin, 2024-10-18 */
			FPGA_PORT_WRITE(__PORT_VENDOR1_ADDR[port], 0x0);
			FPGA_PORT_WRITE(__PORT_VENDOR2_ADDR[port], 0x0);
			FPGA_PORT_WRITE(__PORT_VENDOR3_ADDR[port], 0x0);
			FPGA_PORT_WRITE(__PORT_VENDOR4_ADDR[port], 0x0);
			FPGA_PORT_WRITE(__PORT_VENDOR5_ADDR[port], 0x0);
			FPGA_PORT_WRITE(__PORT_VENDOR6_ADDR[port], 0x0);
			FPGA_PORT_WRITE(__PORT_VENDOR7_ADDR[port], 0x0);
			FPGA_PORT_WRITE(__PORT_VENDOR8_ADDR[port], 0x0);
			FPGA_PORT_WRITE(__PORT_PN1_ADDR[port], 0x0);
			FPGA_PORT_WRITE(__PORT_PN2_ADDR[port], 0x0);
			FPGA_PORT_WRITE(__PORT_PN3_ADDR[port], 0x0);
			FPGA_PORT_WRITE(__PORT_PN4_ADDR[port], 0x0);
			FPGA_PORT_WRITE(__PORT_PN5_ADDR[port], 0x0);
			FPGA_PORT_WRITE(__PORT_PN6_ADDR[port], 0x0);
			FPGA_PORT_WRITE(__PORT_PN7_ADDR[port], 0x0);
			FPGA_PORT_WRITE(__PORT_PN8_ADDR[port], 0x0);
			FPGA_PORT_WRITE(__PORT_SN1_ADDR[port], 0x0);
			FPGA_PORT_WRITE(__PORT_SN2_ADDR[port], 0x0);
			FPGA_PORT_WRITE(__PORT_SN3_ADDR[port], 0x0);
			FPGA_PORT_WRITE(__PORT_SN4_ADDR[port], 0x0);
			FPGA_PORT_WRITE(__PORT_SN5_ADDR[port], 0x0);
			FPGA_PORT_WRITE(__PORT_SN6_ADDR[port], 0x0);
			FPGA_PORT_WRITE(__PORT_SN7_ADDR[port], 0x0);
			FPGA_PORT_WRITE(__PORT_SN8_ADDR[port], 0x0);
			FPGA_PORT_WRITE(__PORT_RATE_ADDR[port], 0x0);
			FPGA_PORT_WRITE(__PORT_DIST_ADDR[port], 0x0);
#endif

			PORT_STATUS[port].sfp_type = SFP_ID_UNKNOWN;
			PORT_STATUS[port].equip = 0;/*not-installed*/
		} else {/*0-mean-installed*/
#if 1 /* [#125] Fixing for SFP channel no, wavelength, tx/rx dBm, dustin, 2024-09-10 */
			i2c_in_use_flag_backup = i2c_in_use_flag;
			i2c_in_use_flag = 1;
#endif
#if 1 /* [#107] Fixing for 2nd register updates, dustin, 2024-08-29 */
			thread_add_timer(master, port_scan_sfp, port, 
				(port >= (PORT_ID_EAG6L_MAX - 1)) ? 3 : 1);

			if(! boardStatusFlag) {
				boardStatusFlag = 1;
			}
#else /**************************************************************/
#if 0 /* [#91] Fixing for register updating feature, dustin, 2024-08-05 */
			/* blocked becaus checking i2c caused unexpected disabling cr event. */
			/* check if i2c can access 0x50 address */
			if(check_sfp_is_present(port) < 0/*not-found*/) {
				zlog_notice("%s: not found sfp on port[%d].", __func__, port);
				PORT_STATUS[port].equip = 0;/*not-installed*/
				continue;
			}
#endif

#if 1/*[#61] Adding omitted functions, dustin, 2024-06-24 */
			read_port_inventory(port, &(INV_TBL[port]));
#endif

#if 1 /* [#94] Adding for 100G DCO handling, dustin, 2024-08-19 */
			/* need some time to access, if 100G sfp. */
			if(port >= (PORT_ID_EAG6L_MAX - 1))
				sleep(2);
#endif
			/* get private sfp identifier */
			type = get_private_sfp_identifier(port);
			/* get wavelength register 2 */
			data = FPGA_READ(__PORT_WL2_ADDR[port]);

			/* update wavelength register 2 */
			data &= ~0x0F00;
			data |= (type << 8);
#if 1 /* [#107] Fixing for 2nd register updates, dustin, 2024-08-29 */
			FPGA_PORT_WRITE(__PORT_WL2_ADDR[port], data);
#else
			FPGA_PORT_WRITE(__PORT_WL2_ADDR[port], data);
			gPortRegUpdate(__PORT_WL2_ADDR[port], 8, 0xF00, type);
#endif

			PORT_STATUS[port].sfp_type = type;
			PORT_STATUS[port].equip = 1;/*installed*/

#if 1 /* [#91] Fixing for register updating feature, dustin, 2024-08-05 */
			/* init for 100G DCO sfp. FIXME: need OE spf. */
			if(! memcmp(INV_TBL[port].part_num, "FTLC3351R3PL1", 
				sizeof("FTLC3351R3PL1")))
			{
				extern void init_100G_sfp(void);

				init_100g_sfp();
			}
#endif

#if 1/* [#72] Adding omitted rtWDM related registers, dustin, 2024-06-27 */
			if(PORT_STATUS[port].tunable_sfp) {
				/* get inventory */
				if(PORT_STATUS[port].tunable_sfp) {
					read_port_rtwdm_inventory(port, &(RTWDM_INV_TBL[port]));
				}

				/* set flex tune if configured */
				if(PORT_STATUS[port].cfg_flex_tune)
					set_flex_tune_control(port, 1/*enable*/);

				/* set smart t-sfp self loopback if configured. */
				if(PORT_STATUS[port].cfg_smart_tsfp_selfloopback)
					set_smart_tsfp_self_loopback(port, 1/*enable*/);

				/* set rtwdm loopback if configured. */
				if(PORT_STATUS[port].cfg_rtwdm_loopback)
					set_rtwdm_loopback(port, 1/*enable*/);
			}
#else
			/* set flex tune if configured */
			if(PORT_STATUS[port].cfg_flex_tune)
				set_flex_tune_control(port, 1/*enable*/);

			/* set smart t-sfp self loopback if configured. */
			if(PORT_STATUS[port].cfg_smart_tsfp_selfloopback)
				set_smart_tsfp_self_loopback(port, 1/*enable*/);

			/* set rtwdm loopback if configured. */
			if(PORT_STATUS[port].cfg_rtwdm_loopback)
				set_rtwdm_loopback(port, 1/*enable*/);
#endif

#if 1 /* [#85] Fixing for resetting PM counter for unexpected FEC counting, dustin, 2024-07-31 */
			thread_add_timer(master, pm_clear_fec_counters, port, 2);
#endif

			update_port_sfp_inventory();

#if 1 /* [#91] Fixing for register updating feature, dustin, 2024-08-05 */
			if(! boardStatusFlag) {
				boardStatusFlag = 1;
			}
#endif
#endif /* [#107] */
#if 1 /* [#125] Fixing for SFP channel no, wavelength, tx/rx dBm, dustin, 2024-09-10 */
			i2c_in_use_flag = i2c_in_use_flag_backup;
#endif
		}
	}
	return SUCCESS;
#else
	if(val != 0/*1-mean-not-installed*/) {
		/* clear spf inventory */
		memset(&(INV_TBL[port]), 0, sizeof(struct module_inventory));

		/* clear pm counter? */
		memset(&(PM_TBL[port]), 0, sizeof(port_pm_counter_t));

		PORT_STATUS[port].sfp_type = SFP_ID_UNKNOWN;
		PORT_STATUS[port].equip = 0;/*not-installed*/
		return SUCCESS;
	} else {/*0-mean-installed*/
		/* check if i2c can access 0x50 address */
		if(check_sfp_is_present(port) < 0/*not-found*/) {
			zlog_notice("%s: not found sfp on port[%d].", __func__, port);
			PORT_STATUS[port].equip = 0;/*not-installed*/
			return ERR_NOT_FOUND;
		}

		/* get private sfp identifier */
		type = get_private_sfp_identifier(port);
		if(type != SFP_ID_UNKNOWN) {
			/* get wavelength register 2 */
#if 1 /* [#161] Fixing for processing alarm flag, dustin, 2024-10-22 */
			data = FPGA_PORT_READ(__PORT_WL2_ADDR[port]);
#else
			data = FPGA_READ(__PORT_WL2_ADDR[port]);
#endif

			/* update wavelength register 2 */
			data &= ~0x0F00;
			data |= (type << 8);
			FPGA_PORT_WRITE(__PORT_WL2_ADDR[port], data);
			gPortRegUpdate(__PORT_WL2_ADDR[port], 8, 0xF00, type);
		}

		PORT_STATUS[port].sfp_type = type;
		PORT_STATUS[port].equip = 1;/*installed*/

		/* set flex tune if configured */
		if(PORT_STATUS[port].cfg_flex_tune)
			set_flex_tune_control(port, 1/*enable*/);

		/* set smart t-sfp self loopback if configured. */
		if(PORT_STATUS[port].cfg_smart_tsfp_selfloopback)
			set_smart_tsfp_self_loopback(port, 1/*enable*/);

		/* set rtwdm loopback if configured. */
		if(PORT_STATUS[port].cfg_rtwdm_loopback)
			set_rtwdm_loopback(port, 1/*enable*/);

#if 1 /* [#157] Fixing for Smart T-SFP rtWDM info, dustin, 2024-10-18 */
		update_port_sfp_inventory(0/*update-no-rtwdm*/);
#else
		update_port_sfp_inventory();
#endif

		return SUCCESS;
	}
#endif
}

#if 1/*[#61] Adding omitted functions, dustin, 2024-06-24 */
#if 0 /* [#91] Fixing for register updating feature, dustin, 2024-08-05 */
uint16_t bankSelect1(uint16_t port, uint16_t val)
{
	uint16_t wbank;

	port = 0;/*meaningless*/
	wbank = (val >> 8) & 0x3;

	/*FIXME : change working bank */

	return SUCCESS;
}
#endif

#if 1 /* [#97] Adding register recovery process after fpga reset, dustin, 2024-08-21 */
#if 0 /*[#99] EAG6L 개발 검증 이슈 F/U, balkrow, 2024-08-30*/
void update_fpga_bank_status(void)
{
#if 1 /* [#89] Fixing for RDL changes on Target system, dustin, 2024-08-02 */
	/* check cpld 0x7000001C for current fpga bank */
	{
		int bno = CPLD_READ(CPLD_FW_BANK_SELECT_ADDR);
#if 1 /* [#93] Adding for FPGA FW Bank Select and Error handling, dustin, 2024-08-12 */
		int sts, retry;
#endif

		zlog_notice("%s : Current FPGA bank [%d].", __func__, bno);
		if(bno == 0) {
			FILE *fn = NULL;
			char cmd[50];
			char act_flag, stb_flag, dft_flag, none, invalid;

			act_flag = stb_flag = dft_flag = none = invalid = 0;

			/* get bank from env */
			fn = popen("fw_printenv -n fw_act_bank", "r");
			if(fn == NULL) {
_try_stb_:
				fn = popen("fw_printenv -n fw_stb_bank", "r");
				if(fn == NULL) {
_try_dft_:
					fn = popen("fw_printenv -n fw_dft_bank", "r");
					if(fn == NULL) {
						zlog_notice("%s : Cannot get fpga bank env variable.",
							__func__);
						none = 1;
						goto _next_job_;
					} else dft_flag = 1;
				} else stb_flag = 1;
			} else act_flag = 1;

#if 1 /* [#93] Adding for FPGA FW Bank Select and Error handling, dustin, 2024-08-12 */
			if(fn != NULL) {
				fread(cmd, sizeof(char), 1, fn);
				pclose(fn);

				if((sscanf(cmd, "%d", &bno) != 1) || 
					((bno != RDL_BANK_1) && (bno != RDL_BANK_2))) {
					zlog_notice("%s : Invalid fpga bank env variable[%d].", 
						__func__, bno);
					invalid = 1;
				}
			}
#else
			fread(cmd, sizeof(char), 1, fn);
			pclose(fn);

			if((sscanf(cmd, "%d", &bno) != 1) || 
				((bno != RDL_BANK_1) && (bno != RDL_BANK_2))) {
				zlog_notice("%s : Invalid fpga bank env variable[%d].", 
					__func__, bno);
				invalid = 1;
			}
#endif

_next_job_:
			if(none) {
				bno = RDL_BANK_1;
			} else if(invalid) {
				if(act_flag) {
					act_flag = invalid = 0;
					goto _try_stb_;
				}
				else if(stb_flag) {
					stb_flag = invalid = 0;
					goto _try_dft_;
				} else if(dft_flag) {
					dft_flag = 0;
					bno = RDL_BANK_1;
				}
				else
					bno = RDL_BANK_1;
			} else {
				if(stb_flag)
					bno = (bno == RDL_BANK_1) ? RDL_BANK_2 : RDL_BANK_1;
			}

			zlog_notice("%s : Update CPLD FPGA bank to [%d].", __func__, bno);
			/* set fpga active bank */
			CPLD_WRITE(CPLD_FW_BANK_SELECT_ADDR, bno);

			/* set env variable */
			if(act_flag) {
				sprintf(cmd, "fw_setenv fw_stb_bank %d", 
					(bno == RDL_BANK_1) ? RDL_BANK_2 : RDL_BANK_1);
				system(cmd);
			} else if(stb_flag) {
				sprintf(cmd, "fw_setenv fw_act_bank %d", bno);
				system(cmd);
			} else if(dft_flag) {
				sprintf(cmd, "fw_setenv fw_act_bank %d", bno);
				system(cmd);
				sprintf(cmd, "fw_setenv fw_stb_bank %d", 
					(bno == RDL_BANK_1) ? RDL_BANK_2 : RDL_BANK_1);
				system(cmd);
			} else {
				sprintf(cmd, "fw_setenv fw_act_bank %d", bno);
				system(cmd);
				sprintf(cmd, "fw_setenv fw_stb_bank %d", 
					(bno == RDL_BANK_1) ? RDL_BANK_2 : RDL_BANK_1);
				system(cmd);
				sprintf(cmd, "fw_setenv fw_dft_bank %d", bno);
				system(cmd);
			}
		}

#if 1 /* [#93] Adding for FPGA FW Bank Select and Error handling, dustin, 2024-08-12 */
__retry__:
		/* check fpga bank config status. */
		for(retry = 0; retry < 10; retry++) {
			sts = CPLD_READ(CPLD_FW_BANK_STATUS_ADDR);
			if(sts == CPLD_BANK_OK) {
				zlog_notice("%s : FPGA bank status OK.", __func__);
				break;
			}
			usleep(50000);
		}

		/* if target bank failed, change to other bank */
		if(sts == CPLD_BANK_BAD) {
			/* try standby bank */
			bno = (bno == RDL_BANK_1) ? RDL_BANK_2 : RDL_BANK_1;

			zlog_notice("%s : Try standby FPGA bank [%d].", __func__, bno);
			/* set fpga standby bank */
			CPLD_WRITE(CPLD_FW_BANK_SELECT_ADDR, bno);
			goto __retry__;
		} else if(sts == CPLD_BANK_OK) {
			zlog_notice("%s : Update working FPGA bank to [%d].", 
				__func__, bno);
			/* update working bank info. */
			gRegUpdate(FW_BANK_SELECT_ADDR, 8, 0x700, bno);
		}
#endif
	}
#endif
}
#endif

#if 1 /* [#94] Adding for 100G DCO handling, dustin, 2024-09-23 */
#if 1 /* [#152] Adding for port RS-FEC control, dustin, 2024-10-15 */
uint16_t portFECEnable(uint16_t portno, uint16_t enable)
{
#if 1 /* [#154] Fixing for auto FEC mode on DCO, dustin, 2024-10-21 */
	if(enable == 0xA5) {
		gSysmonToCpssFuncs[gPortFECEnable](2, portno, 1);
		PORT_STATUS[portno].cfg_rs_fec = 1;
	}
	else if(enable == 0x5A) {
		gSysmonToCpssFuncs[gPortFECEnable](2, portno, 0);
		PORT_STATUS[portno].cfg_rs_fec = 0;
	}
	else if(enable == 0x0/*auto-mode*/) {
		gSysmonToCpssFuncs[gPortFECEnable](2, portno, 
			PORT_STATUS[portno].sfp_dco ? 0 : 1);
		PORT_STATUS[portno].cfg_rs_fec = PORT_STATUS[portno].sfp_dco ? 0 : 1;
	}
#else /**************************************************************/
	if(enable == 0xA5)
		gSysmonToCpssFuncs[gPortFECEnable](2, portno, 1);
	else if(enable == 0x5A)
		gSysmonToCpssFuncs[gPortFECEnable](2, portno, 0);
#endif
	return SUCCESS;
}
#endif

#if 1 /* [#154] Fixing for auto FEC mode on DCO, dustin, 2024-10-21 */
/* NOTE : regMon callback function must have portno and value arguments. */
uint16_t DcoReset(uint16_t portno, uint16_t val)
#else
uint16_t DcoReset(uint16_t val)
#endif
{
#if 1 /* [#169] Fixing for new DCO install process, dustin, 2024-10-25 */
	if(! PORT_STATUS[PORT_ID_EAG6L_PORT7].i2cReady) {
		zlog_notice("DCO reset was aborted because TCReady/InitComplete.");
		return ERR_PORT_NOT_READY;
	}
#endif

	if(val == 0xA5) {
		set_i2c_dco_reset();
#if 1 /* [#150] Implementing LR4 Status register, dustin, 2024-10-21 */
		memset(&DCO_STAT, 0, sizeof DCO_STAT);
		memset(&DCO_COUNT, 0, sizeof DCO_COUNT);
#endif

		/* FIXME : correct action ? */
		thread_add_timer(master, port_scan_sfp, PORT_ID_EAG6L_PORT7, 3);
	}
	return SUCCESS;
}

/* NOTE : both DCO/LR7 can be configured by this. */
#if 1 /* [#154] Fixing for auto FEC mode on DCO, dustin, 2024-10-21 */
/* NOTE : regMon callback function must have portno and value arguments. */
uint16_t DcoFECEnable(uint16_t portno, uint16_t val)
#else
uint16_t DcoFECEnable(uint16_t val)
#endif
{
	uint8_t hs_flag, ms_flag;

#if 1 /* [#154] Fixing for auto FEC mode on DCO, dustin, 2024-10-21 */
	if(val == 0/*auto-mode*/) {
		hs_flag = PORT_STATUS[PORT_ID_EAG6L_PORT7].sfp_dco ? 0x5A : 0xA5;
		ms_flag = 0xA5;
	} else {
		hs_flag = (val >> 8) & 0xFF;
		ms_flag = (val & 0xFF);
	}
#else
	hs_flag = (val >> 8) & 0xFF;
	ms_flag = (val & 0xFF);
#endif
	if((hs_flag == 0xA5) || (hs_flag == 0x5A) || 
	   (ms_flag == 0xA5) || (ms_flag == 0x5A))
		set_i2c_dco_fec_enable(hs_flag, ms_flag);
#if 1 /* [#169] Fixing for new DCO install process, dustin, 2024-10-25 */
	PORT_STATUS[portno].cfg_dco_fec = (hs_flag == 0xA5) ? 1 : 0;
#endif
	return SUCCESS;
}

#if 1 /* [#154] Fixing for auto FEC mode on DCO, dustin, 2024-10-21 */
/* NOTE : regMon callback function must have portno and value arguments. */
uint16_t DcoCountReset(uint16_t portno, uint16_t val)
#else
uint16_t DcoCountReset(uint16_t val)
#endif
{
#if 1 /* [#169] Fixing for new DCO install process, dustin, 2024-10-25 */
	if(! PORT_STATUS[PORT_ID_EAG6L_PORT7].i2cReady) {
		zlog_notice("DCO BER reset was aborted because TCReady/InitComplete.");
		return ERR_PORT_NOT_READY;
	}
#endif

	if(val == 0xA5)
		set_i2c_dco_count_reset();
	return SUCCESS;
}
#endif /* [#94] */

#if 1 /* [#142] Adding for Transparent mode switching, dustin, 2024-10-11 */
#if 1 /* [#164] Fixing for correct switch mode, dustin, 2024-1023 */
/* NOTE : regMon callback function must have portno and value arguments. */
uint16_t swModeSet(uint16_t portno, uint16_t sw_mode)
#else
uint16_t swModeSet(uint16_t sw_mode)
#endif
{
zlog_notice("swModeSet : sw_mode[0x%x].", sw_mode);//ZZPP
#if 1 /* [#164] Fixing for correct switch mode, dustin, 2024-1023 */
	if(sw_mode == SW_TRANSPARENT_MODE)
		gSysmonToCpssFuncs[gSwitchModeSet](1, 1/*SW_TRANSPARENT_MODE*/);
	else if(sw_mode == SW_AGGREGATION_MODE)
		gSysmonToCpssFuncs[gSwitchModeSet](1, 0/*SW_AGGREGATION_MODE*/);
#else
	if(sw_mode == SW_TRANSPARENT_MODE)
		gSysmonToCpssFuncs[gSwitchModeSet](1, SW_TRANSPARENT_MODE);
	else if(sw_mode == SW_AGGREGATION_MODE)
		gSysmonToCpssFuncs[gSwitchModeSet](1, SW_AGGREGATION_MODE);
#endif
	return SUCCESS;
}
#endif

void do_recovery_update_after_fpga_reset(void)
{
extern uint16_t sysmonUpdateGetSWVer(void);
#if 0 /* [#105] Fixing for RDL install/activation process, dustin, 2024-08-27 */
extern void rdl_update_bank_registers(int bno);
#endif
void process_hw_inventory_infos(void);

uint16_t data;

	data = sysmonUpdateGetSWVer();

	/* recover sw version register. */
	FPGA_WRITE(SW_VERSION_ADDR, data);

	/* update working bank info. */
	data = CPLD_READ(CPLD_FW_BANK_SELECT_ADDR);
	gRegUpdate(FW_BANK_SELECT_ADDR, 8, 0x700, data);

	/* recover hw inventory registers. */
	process_hw_inventory_infos();

	/* recover bp os bank1/2 header registers. */
#if 0/*[#110] RDL function Debugging 및 수정, balkrow, 2024-09-02*/
#if 1 /* [#105] Fixing for RDL install/activation process, dustin, 2024-08-27 */
	rdl_update_bank_registers(RDL_BANK_1, RDL_B1_ERASE_FLAG);
	rdl_update_bank_registers(RDL_BANK_2, RDL_B2_ERASE_FLAG);
#else
	rdl_update_bank_registers(RDL_BANK_1);
	rdl_update_bank_registers(RDL_BANK_2);
#endif
#endif
#if 1/*[#122] primary/secondary Send QL 설정, balkrow, 2024-09-09*/
	gDB.localQL = ESMC_LOCAL_QL;
#endif

	/* recover initial complete register. */
	FPGA_WRITE(INIT_COMPLETE_ADDR, 0xAA);
}
#endif

uint16_t bankSelect2(uint16_t port, uint16_t val)
{
#if 1 /* [#91] Fixing for register updating feature, dustin, 2024-08-05 */
extern void set_fpga_fw_active_bank_flag(uint8_t bno);

#if 1/*[#110] RDL function Debugging 및 수정, balkrow, 2024-09-02*/
	char cmd[100] = {0, };
	int8_t is_pkg;
#endif
	if((val != RDL_BANK_1) && (val != RDL_BANK_2)) {
		zlog_notice("%s : MCU set invalid bank no[%d] for FPGA FW.", __func__, val);
		return -1;
	}

	if(!gDB.pkg_is_zip)
		gDB.pkg_is_zip = is_pkg_zip(val);
#if 1/*[#119] bank switch 시 fpga reset 기능 추가, balkrow, 2024-09-05*/
	zlog_notice("Bank %d PKG TYPE %s", val, gDB.pkg_is_zip == PKG_ZIP ? "ZIP":"Normal");
#endif
	
#if 1/*[#110] RDL function Debugging 및 수정, balkrow, 2024-09-04*/
	if(gDB.pkg_is_zip == PKG_NONZIP)
	{
		/*os only*/
		if(activate_os(val) == RT_NOK) 
		{
			fw_image_header_t *del;
			zlog_notice("Activating os failed.");
			/*
			clear_bank(val);
			*/
			if(val == RDL_BANK_1)
				del = &gDB.bank1_header;
			else if(val == RDL_BANK_2)
				del = &gDB.bank2_header;

			memset(del, 0, sizeof(fw_image_header_t));
			write_pkg_header(val, del);
			return -1;
		}
		else
		{
			/** write OS bank info */
			sprintf(cmd, "fw_setenv bank %d", val);
			system(cmd);

			/** OS reboot */
#if 1/*[#135] fpga register reset 위치 수정, balkrow, 2024-09-26*/
			CPLD_WRITE(FPGA_RESET_BY_BP, 0xa5a5);
			zlog_notice("clear fpga register!!");
			CPLD_WRITE(FPGA_RESET_BY_BP, 0);
#endif
			memset(cmd, 0, 100);
			sprintf(cmd, "reboot -nf");
			system(cmd);
			
		}

	}
	else if(gDB.pkg_is_zip == PKG_ZIP)
	{
		if(activate_os(val) == RT_NOK) 
		{
			fw_image_header_t *del;
			zlog_notice("Activating os failed.");
			/*
			clear_bank(val);
			*/
			if(val == RDL_BANK_1)
				del = &gDB.bank1_header;
			else if(val == RDL_BANK_2)
				del = &gDB.bank2_header;

			memset(del, 0, sizeof(fw_image_header_t));
			write_pkg_header(val, del);
			return -1;
		}
		else
		{
			/** write OS bank info */
			sprintf(cmd, "fw_setenv bank %d", val);
			system(cmd);
			/** write FPGA bank info */
			if(gDB.fpga_running_bank == 0)
			{
				memset(cmd, 0, 100);
				sprintf(cmd, "fw_setenv fw_act_bank %d", val);
				system(cmd);
			} 
			else 
			{
				memset(cmd, 0, 100);
				sprintf(cmd, "fw_setenv fw_act_bank %d", val);
				system(cmd);
				memset(cmd, 0, 100);
				sprintf(cmd, "fw_setenv fw_stb_bank %d", gDB.fpga_running_bank);
				system(cmd);
			}

			/** OS reboot */
#if 0/*[#126] bank switch 후 CPU 멈춤현상, balkrow, 2024-09-11*/
			CPLD_WRITE(FPGA_RESET_BY_BP, 0xa5a5);
			CPLD_WRITE(FPGA_RESET_BY_BP, 0);
#endif
			memset(cmd, 0, 100);
			sprintf(cmd, "reboot -nf");
			system(cmd);

		}

#if 0/*[#110] RDL function Debugging 및 수정, balkrow, 2024-09-04*/
		if(rdl_activate_fpga(val) < 0) {
			fw_image_header_t *del;
			zlog_notice("Activating fpga fw failed.");
			clear_bank(val);
			if(val == RDL_BANK_1)
				del = &gDB.bank1_header;
			else if(val == RDL_BANK_2)
				del = &gDB.bank2_header;

			memset(del, 0, sizeof(fw_image_header_t));
			write_pkg_header(val, del);
			return -1;
		} 
		else
		{
			/* update cpld fpga fw bank select, it will cuase fpga reboot. */
			extern int check_fpga_status(void);
			extern int _CHECK_FPGA_STATUS_;
			extern uint8_t _FPGA_BNO_;
			CPLD_WRITE(CPLD_FW_BANK_SELECT_ADDR, val);

			/* turn on flag to check fpga status by thread. */
			_CHECK_FPGA_STATUS_ = 1;
			_FPGA_BNO_ = val;

			/* give a time to let mcu detect board CR event. */
			thread_add_timer_msec(master, check_fpga_status, NULL, 500);
		}

		{
			fw_image_header_t *new;
			if(val == RDL_BANK_1)
				new = &gDB.bank1_header;
			else if(val == RDL_BANK_2)
				new = &gDB.bank2_header;

			if(get_pkg_header(val, new) == RT_OK)
				write_pkg_header(val, new);

			sprintf(cmd, "fw_setenv bank %d", val);
			system(cmd);
		}
#endif
	}
#else
#if 1 /* [#105] Fixing for RDL install/activation process, dustin, 2024-08-27 */
	{
		extern uint16_t RDL_B1_ERASE_FLAG;
		extern uint16_t RDL_B2_ERASE_FLAG;
		char cmd[100];

		if(rdl_activate_bp(val) < 0) {
			zlog_notice("Activating os failed.");
			if(val == RDL_BANK_1)
				RDL_B1_ERASE_FLAG = 1/*erase*/;
			else
				RDL_B2_ERASE_FLAG = 1/*erase*/;
			rdl_update_bank_registers(val, 1/*erase*/);
			return -1;
		}

		if(rdl_activate_fpga(val) < 0) {
			zlog_notice("Activating fpga fw failed.");
			if(val == RDL_BANK_1)
				RDL_B1_ERASE_FLAG = 1/*erase*/;
			else
				RDL_B2_ERASE_FLAG = 1/*erase*/;
			rdl_update_bank_registers(val, 1/*erase*/);
			return -1;
		}

		sprintf(cmd, "fw_setenv bank %d", val);
		system(cmd);
	}
#endif

		/* update cpld fpga fw bank select, it will cuase fpga reboot. */
		CPLD_WRITE(CPLD_FW_BANK_SELECT_ADDR, val);
		/* set bank flag for fpga fw */
		set_fpga_fw_active_bank_flag(val);

#if 1 /* [#97] Adding register recovery process after fpga reset, dustin, 2024-08-21 */
	{
		extern int check_fpga_status(void);
		extern int _CHECK_FPGA_STATUS_;
		extern uint8_t _FPGA_BNO_;

		/* turn on flag to check fpga status by thread. */
		_CHECK_FPGA_STATUS_ = 1;
		_FPGA_BNO_ = val;

		/* give a time to let mcu detect board CR event. */
		thread_add_timer_msec(master, check_fpga_status, NULL, 500);
	}
#else /***********************************************************/
#if 1 /* [#93] Adding for FPGA FW Bank Select and Error handling, dustin, 2024-08-12 */
		usleep(50000);
		zlog_notice("%s : Reboot BP too after FPGA reset.", __func__);//ZZPP
		system("reboot -nf");
#endif
#endif /* [#97] */
#endif

#else
	uint16_t rbank;

	port = 0;/*meaningless*/
	rbank = val & 0x3;

	/*FIXME : restart bank */
#endif

	return SUCCESS;
}
#endif

#if 1 /* [#100] Adding update of Laser status by Laser_con, dustin, 2024-08-23 */
void update_laser_status(uint16_t portno, uint16_t val)
{
	if(portno >= (PORT_ID_EAG6L_MAX - 1)) {
		/* call i2c function to set laser on/off for 100G sfp. */
		set_i2c_100G_laser_control(portno, 
			(val == 0xA5) ? 1/*on*/ : 0/*off*/);
	}

#if 1 /* [#151] Implementing P7 config register, dustin, 2024-10-21 */
	PORT_STATUS[portno].cfg_tx_laser = (val == 0xA5) ? 1/*ON*/ : 0/*OFF*/;
#endif
	if(val == 0xA5) {
		zlog_notice("%s : laser status set to [ON] for port[%d].", 
			__func__, portno);
		PORT_STATUS[portno].tx_laser_sts = 1/*on*/;
	} else if(val == 0x5A) {
		zlog_notice("%s : laser status set to [OFF] for port[%d].", 
			__func__, portno);
		PORT_STATUS[portno].tx_laser_sts = 0/*off*/;
	}
	return;
}
#endif

#if 1 /* [#62] SFP eeprom 및 register update 기능 단위 검증 및 디버깅, balkrow, 2024-06-21 */ 
int rollback_reg(struct thread *thread)
{
	uint16_t *idx = NULL, reg;	
	idx = (uint16_t *)THREAD_ARG(thread); 

	if(idx)
	{
		regMonList[*idx].val = regMonList[*idx].rollback_val;

		gRegUpdate(regMonList[*idx].reg, 
			   regMonList[*idx].shift, 
			   regMonList[*idx].mask, 
			   regMonList[*idx].val);
		zlog_err("%s rollback reg %x,val %x", __func__, regMonList[*idx].reg, regMonList[*idx].val);
		
	}
	return RT_OK;
}
#endif

void regMonitor(void)
{
	uint16_t i, val;
	uint16_t ret;
	for(i = 0; i < regMonArrSize; i++)
	{
#if 1/*[#65] Adding regMon simulation feature under ACCESS_SIM, dustin, 2024-06-24 */
#if 1 /* [#91] Fixing for register updating feature, dustin, 2024-08-05 */
		/* common control 2 register is not part of port configuration part. */
		if(((COMMON_CTRL2_P1_ADDR <= regMonList[i].reg) && 
		   (regMonList[i].reg <= COMMON_CTRL2_P6_ADDR)) ||
		   (regMonList[i].reg == FW_BANK_SELECT_ADDR))
			val = regMonList[i].func(regMonList[i].reg, PORT_NOREG); 
		/* pm count clear register is part of port configuration part, but without port. */
		else if(regMonList[i].reg == PM_COUNT_CLEAR_ADDR)
			val = regMonList[i].func(regMonList[i].reg, PORT_REG);
#if 1 /* [#154] Fixing for auto FEC mode on DCO, dustin, 2024-10-21 */
        else if((regMonList[i].reg == QSFP28_FEC_ENABLE_ADDR) ||
                (regMonList[i].reg == QSFP28_RESET_ADDR) ||
                (regMonList[i].reg == QSFP28_COUNT_RESET_ADDR))
            val = regMonList[i].func(regMonList[i].reg, PORT_REG);
#endif
		else
			val = regMonList[i].func(regMonList[i].reg, regMonList[i].portno ? PORT_REG : PORT_NOREG); 
#else
		val = regMonList[i].func(regMonList[i].reg, regMonList[i].portno ? PORT_REG : PORT_NOREG); 
#endif
		val = (val & regMonList[i].mask) >> regMonList[i].shift;
#else
		val = regMonList[i].func(regMonList[i].reg);
		val = (val >> regMonList[i].shift) & regMonList[i].mask;
#endif

#if 0/*[#118] Sync-e option2 지원, balkrow, 2024-09-06*/
		/* delay other port-dependent register callbacks after boardStatus is called. */
		if(! boardStatusFlag && 
			((regMonList[i].reg != BD_SFP_CR_ADDR) &&
			 (regMonList[i].reg != PM_COUNT_CLEAR_ADDR) &&
			 (regMonList[i].reg != CHIP_RESET_ADDR) &&
			 (regMonList[i].reg != FW_BANK_SELECT_ADDR)))
			continue;
#endif

		if(regMonList[i].val != val) 
		{
#if 1/*[#51] Adding register callback templates for config/command registers, dustin, 2024-06-12 */
			ret = regMonList[i].cb(regMonList[i].portno, val);
#else
			regMonList[i].cb(val);
#endif
#ifdef DEBUG
			zlog_notice("%s: reg=%x prv %x, cur %x", __func__, regMonList[i].reg, regMonList[i].val, val);
#endif
			regMonList[i].val = val;
		}
	}
}

#if 1/*[#53] Clock source status 업데이트 기능 추가, balkrow, 2024-06-13*/
u8 eag6LPortlist[] =
{
	0,      /*port1-25G*/
	8,      /*port2-25G*/
	16,     /*port3-25G*/
	24,     /*port4-25G*/
	32,     /*port5-25G*/
	40,     /*port6-25G*/
#ifdef MVDEMO /*[68] eag6l board 를 위한 port number 수정, balkrow, 2024-06-27*/
#warning "----- MVDEMO board ------------"
	48,     /*port7-25G*/
	49,     /*port8-25G*/
#endif
	50,     /*port9-100G-offset-0*/
#if 0/*[#53] Clock source status 업데이트 기능 추가, balkrow, 2024-06-13*/
	51,     /*port9-100G-offset-1*/
	52,     /*port9-100G-offset-2*/
	53,     /*port9-100G-offset-3*/
#endif
};
u8 eag6LPortArrSize = sizeof(eag6LPortlist) / sizeof(u8);

#if 1/*[68] eag6l board 를 위한 port number 수정, balkrow, 2024-06-27*/
u8 get_eag6L_dport(u8 lport)
{
#ifdef MVDEMO /*[68] eag6l board 를 위한 port number 수정, balkrow, 2024-06-27*/
	if((lport >= PORT_ID_EAG6L_PORT1) && (lport <= PORT_ID_EAG6L_PORT9))
#else
	if((lport >= PORT_ID_EAG6L_PORT1) && (lport <= PORT_ID_EAG6L_PORT7))
#endif
		return eag6LPortlist[lport - 1];
	else {
		zlog_notice("%s: invalid parameter lport[%d] offset[%d].", __func__, lport);
		return 255;/*invalid dport*/
	}
}
#else
u8 get_eag6L_dport(u8 lport, u8 offset)
{
#ifdef MVDEMO /*[68] eag6l board 를 위한 port number 수정, balkrow, 2024-06-27*/
	if((lport >= PORT_ID_EAG6L_PORT1) && (lport <= PORT_ID_EAG6L_PORT8))
#else
	if((lport >= PORT_ID_EAG6L_PORT1) && (lport <= PORT_ID_EAG6L_PORT6))
#endif
		return eag6LPortlist[lport - 1];
	else if((lport == (PORT_ID_EAG6L_MAX - 1)) && (offset < 4))
		return eag6LPortlist[lport - 1 + offset];
	else {
		zlog_err(LOG_ERR, "%s: invalid parameter lport[%d] offset[%d].",
				__func__, lport, offset);
		return 255;/*invalid dport*/
	}
}
#endif

#if 1 /* [#91] Fixing for register updating feature, dustin, 2024-08-05 */
int check_fifo_hello(struct thread *thread)
{
	sysmon_fifo_msg_t msg;
	uint16_t rc = RT_OK;

	memset(&msg, 0, sizeof msg);
	msg.type = gHello;
	rc = gSysmonToCpssFuncs[gHello](1, &msg);
	if(rc != FIFO_CMD_SUCCESS) {
//zlog_notice("------> %s : BP FAILED~!!!", __func__);//ZZPP
		/* update cpu fail register */
		gRegUpdate(CPU_FAIL_ADDR, 8, 0x100, 1/*bp fail*/);
	} else {
//zlog_notice("------> %s : BP OK~!!!", __func__);//ZZPP
		/* update cpu fail register */
		gRegUpdate(CPU_FAIL_ADDR, 8, 0x100, 0/*bp ok*/);
	}

	thread_add_timer(master, check_fifo_hello, NULL, 5/*sec*/);
	return rc;
}
#endif

#if 0/*[#120] LOC Alarm process ¿¿, balkrow, 2024-10-16 */
int8_t decision_oper_interface()
{
	int8_t ret;

	/*check pri interface*/
	if(gDB.synce_pri_port != NOT_DEFINED)
	{
		if(PORT_STATUS[getMPortByCport(gDB.synce_pri_port)].received_QL)
	}

	return ret;
}
#endif

int8_t rsmu_pll_update(void)
{
	uint16_t wr_val = 0;
#if 1 /* [#62] SFP eeprom 및 register update 기능 단위 검증 및 디버깅, balkrow, 2024-06-21 */
	uint8_t val; 
#endif

	val = rsmuGetPLLState();

	if(val != gDB.pll_state)
	{

		switch(val) 
		{
		case FREERUN :
			wr_val = 0x11;
			break;
		case HOLD_OVER :
			wr_val = 0x12;
#if 1/*[#120] LOC Alarm process ¿¿, balkrow, 2024-10-16 */
			gDB.synce_oper_port = 0xff;
#endif
			break;
		case PLL_LOCK :
			wr_val = 0x13;
			break;
#if 1/*[#127] SYNCE current interface ¿¿, balkrow, 2024-09-11*/
		case LOCK_RECOVERY  :
			break;
#endif
		default :
			wr_val = 0x11;
			val = UNKNOWN; 
			break;
		}

		zlog_notice("change pll state %x -> %x, oper_port %x", gDB.pll_state, val, gDB.synce_oper_port);

		gDB.pll_state = val;
#if 1/*[#127] SYNCE current interface ¿¿, balkrow, 2024-09-11*/
		if(gDB.pll_state == PLL_LOCK)
		{	
#if 1/*[#120] LOC Alarm process ¿¿, balkrow, 2024-10-16 */
			gRegUpdate(SYNCE_SRC_STAT_ADDR, 8, 0xff00, gDB.synce_oper_port); 
			gRegUpdate(SYNCE_SRC_STAT_ADDR, 0, 0xff, val); 
			gRegUpdate(SYNCE_ESMC_SQL_ADDR, 8, 0xff00, PORT_STATUS[gDB.synce_oper_port].received_QL); 
			gRegUpdate(SYNCE_ESMC_SQL_ADDR, 0, 0xff, PORT_STATUS[gDB.synce_oper_port].received_QL); 

#endif
		}
#if 1/*[#120] LOC Alarm process ¿¿, balkrow, 2024-10-16 */
		if(gDB.pll_state == HOLD_OVER)
		{
			gRegUpdate(SYNCE_ESMC_SQL_ADDR, 8, 0xff00, gDB.localQL); 
			gRegUpdate(SYNCE_ESMC_SQL_ADDR, 0, 0xff, gDB.localQL); 
			gRegUpdate(SYNCE_SRC_STAT_ADDR, 8, 0xff00, gDB.synce_oper_port); 
		}
#endif

#endif
		if(wr_val)
			gRegUpdate(SYNCE_SRC_STAT_ADDR, 0, SYNCE_SRC_STAT_ADDR_MASK, wr_val)
	}


	return RT_OK;
}
#endif

void read_port_inventory(int portno, struct module_inventory * mod_inv)
{
#if 0//PWY_FIXME
    if('\0' == mod_inv->serial_num[0] || mod_inv->dist == 0xFFFF)
#endif
        get_sfp_info(portno, mod_inv);

    return;
}

#if 1/* [#72] Adding omitted rtWDM related registers, dustin, 2024-06-27 */
void read_port_rtwdm_inventory(int portno, struct module_inventory * mod_inv)
{
#if 0//PWY_FIXME
    if('\0' == mod_inv->serial_num[0] || mod_inv->dist == 0xFFFF)
#endif
#if 1 /* [#157] Fixing for Smart T-SFP rtWDM info, dustin, 2024-10-18 */
	if(PORT_STATUS[portno].link && (! PORT_STATUS[portno].los))
#endif
		get_sfp_rtwdm_info(portno, mod_inv);

    return;
}
#endif

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
		
#if 1/*[68] eag6l board 를 위한 port number 수정, balkrow, 2024-06-27*/
    zlog_notice("get_port_sfp_reg : port=%d(0/%d) addr=%x type=%x, sts=%x", 
		portId, get_eag6L_dport(portId), addr, type, sts);
#else
    zlog_notice("get_port_sfp_reg : port=%d(0/%d) addr=%x type=%x, sts=%x", 
		portId, get_eag6L_dport(portId, 0), addr, type, sts);
#endif
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
#if 1/*[68] eag6l board 를 위한 port number 수정, balkrow, 2024-06-27*/
	zlog_notice(" PORT[%d(0/%d)] %d cwhan_check %4.2f [%4.2f-%4.2f-%4.2f] [%4.2f-%4.2f-%4.2f]",
		portno, get_eag6L_dport(portno), nidx, PORT_STATUS[portno].tx_pwr,
		PORT_STATUS[portno].tx_avg, PORT_STATUS[portno].tx_min, PORT_STATUS[portno].tx_max,
		port_pwr_1min[p_idx][1][nidx], port_pwr_1min[p_idx][1][nidx +1],
		port_pwr_1min[p_idx][1][nidx +2]);
#else
	zlog_notice(" PORT[%d(0/%d)] %d cwhan_check %4.2f [%4.2f-%4.2f-%4.2f] [%4.2f-%4.2f-%4.2f] \n",
		portno, get_eag6L_dport(portno, 0), nidx, PORT_STATUS[portno].tx_pwr,
		PORT_STATUS[portno].tx_avg, PORT_STATUS[portno].tx_min, PORT_STATUS[portno].tx_max,
		port_pwr_1min[p_idx][1][nidx], port_pwr_1min[p_idx][1][nidx +1],
		port_pwr_1min[p_idx][1][nidx +2]);
#endif
#endif
	return;
}

#if 1/* [#72] Adding omitted rtWDM related registers, dustin, 2024-06-27 */
void update_port_rtwdm_rx_power(int portno)
{
    u32 idx, p_idx, nidx;

#if 1 /* [#91] Fixing for register updating feature, dustin, 2024-08-05 */
	if(! PORT_STATUS[portno].equip)
		return;
#endif

	if((portno >= (PORT_ID_EAG6L_MAX - 1)) || (! PORT_STATUS[portno].tunable_sfp))
		return;

    p_idx = (portno - 1) % (PORT_ID_EAG6L_MAX - 1);
    port_pwr_index[p_idx]++;
    port_pwr_index[p_idx] %= 60;
    nidx = port_pwr_index[p_idx];

    if(!is_init_port_pwr[p_idx])
    {
        for(idx = 0; idx < 60; idx++)
        {
            port_pwr_1min[p_idx][0][idx] = PORT_STATUS[portno].rtwdm_ddm_info.rx_pwr;
            port_pwr_1min[p_idx][1][idx] = PORT_STATUS[portno].rtwdm_ddm_info.tx_pwr;
        }
        PORT_STATUS[portno].rtwdm_ddm_info.rx_min = PORT_STATUS[portno].rtwdm_ddm_info.rx_pwr;
        PORT_STATUS[portno].rtwdm_ddm_info.rx_max = PORT_STATUS[portno].rtwdm_ddm_info.rx_pwr;
        PORT_STATUS[portno].rtwdm_ddm_info.rx_avg = PORT_STATUS[portno].rtwdm_ddm_info.rx_pwr;

        PORT_STATUS[portno].rtwdm_ddm_info.tx_min = PORT_STATUS[portno].rtwdm_ddm_info.tx_pwr;
        PORT_STATUS[portno].rtwdm_ddm_info.tx_max = PORT_STATUS[portno].rtwdm_ddm_info.tx_pwr;
        PORT_STATUS[portno].rtwdm_ddm_info.tx_avg = PORT_STATUS[portno].rtwdm_ddm_info.tx_pwr;

        is_init_port_pwr[p_idx] = 1;
        port_pwr_index[p_idx] = 0;

        port_pwr_1min[p_idx][0][0] = PORT_STATUS[portno].rtwdm_ddm_info.rx_pwr;
        port_pwr_1min[p_idx][1][0] = PORT_STATUS[portno].rtwdm_ddm_info.tx_pwr;

        return ;
    }

    port_pwr_1min[p_idx][0][nidx] = PORT_STATUS[portno].rtwdm_ddm_info.rx_pwr;
    port_pwr_1min[p_idx][1][nidx] = PORT_STATUS[portno].rtwdm_ddm_info.tx_pwr;

    calc_pm_1min(&PORT_STATUS[portno].rtwdm_ddm_info.rx_avg, &PORT_STATUS[portno].rtwdm_ddm_info.rx_min, &PORT_STATUS[portno].rtwdm_ddm_info.rx_max, port_pwr_1min[p_idx][0]);
    calc_pm_1min(&PORT_STATUS[portno].rtwdm_ddm_info.tx_avg, &PORT_STATUS[portno].rtwdm_ddm_info.tx_min, &PORT_STATUS[portno].rtwdm_ddm_info.tx_max, port_pwr_1min[p_idx][1]);

#ifdef DEBUG
	zlog_notice(" PORT[%d(0/%d)] %d rtwdm_cwhan_check %4.2f [%4.2f-%4.2f-%4.2f] [%4.2f-%4.2f-%4.2f]",
		portno, get_eag6L_dport(portno), nidx, 
		PORT_STATUS[portno].rtwdm_ddm_info.tx_pwr,
		PORT_STATUS[portno].rtwdm_ddm_info.tx_avg, 
		PORT_STATUS[portno].rtwdm_ddm_info.tx_min, 
		PORT_STATUS[portno].rtwdm_ddm_info.tx_max,
		port_pwr_1min[p_idx][1][nidx], port_pwr_1min[p_idx][1][nidx +1],
		port_pwr_1min[p_idx][1][nidx +2]);
#endif
	return;
}
#endif

void update_sfp(void)
{
#if 1/*[#54] Adding Smart T-SFP I2C functions, dustin, 2024-06-13 */
#if 0/*[#61] Adding omitted functions, dustin, 2024-06-24 */
extern int update_flex_tune_status(int portno);
#endif
extern int get_smart_tsfp_self_loopback(int portno, int * enable);
extern int get_rtwdm_loopback(int portno, int * enable);

	int enable;
#endif
	int portno;

	for(portno = PORT_ID_EAG6L_PORT1; portno < PORT_ID_EAG6L_MAX; portno++) {
		/* skip if port has not installed sfp. */
#if 1 /* [#91] Fixing for register updating feature, dustin, 2024-08-05 */
#if 1 /* [#169] Fixing for new DCO install process, dustin, 2024-10-25 */
		if(! PORT_STATUS[portno].equip || ! PORT_STATUS[portno].i2cReady)
#else
		if(! PORT_STATUS[portno].equip)
#endif
			continue;
#endif

		get_sfp_info_diag(portno, &(PORT_STATUS[portno]));
		update_port_rx_power(portno);
#if 0/*[#61] Adding omitted functions, dustin, 2024-06-24 */
		/* NOTE : move to boardStatus() for one time action. */
		read_port_inventory(portno, &(INV_TBL[portno]));
#endif
#if 1 /* [#94] Adding for 100G DCO handling, dustin, 2024-09-23 */
		if((portno >= (PORT_ID_EAG6L_MAX - 1))) {
			extern int read_i2c_dco_status(dco_status_t *pdco);
			extern uint16_t get_dco_sfp_channel_no(uint16_t portno);
#if 1 /* [#149] Implementing DCO BER/FER counters, dustin, 2024-10-21 */
			extern uint16_t get_dco_ber_fer_rates(uint16_t portno);
#endif

			if(PORT_STATUS[portno].tunable_sfp) {
				/* get 100G DCO channel no. */
				get_dco_sfp_channel_no(portno);

#if 1 /* [#149] Implementing DCO BER/FER counters, dustin, 2024-10-21 */
				get_dco_ber_fer_rates(portno);
#endif
			}

			read_i2c_dco_status(&DCO_STAT);

			/* NOTE : below is not for 100G. */
			continue;
		}
#endif

#if 1/*[#54] Adding Smart T-SFP I2C functions, dustin, 2024-06-13 */
#if 1/*[#61] Adding omitted functions, dustin, 2024-06-24 */
		/* get only if tunable sfp */
		if(PORT_STATUS[portno].tunable_sfp) {
#if 1/* [#72] Adding omitted rtWDM related registers, dustin, 2024-06-27 */
#if 1 /* [#157] Fixing for Smart T-SFP rtWDM info, dustin, 2024-10-18 */
			if(PORT_STATUS[portno].link && (! PORT_STATUS[portno].los)) {
				read_port_rtwdm_inventory(portno, &(RTWDM_INV_TBL[portno]));
				get_sfp_rtwdm_info_diag(portno, &PORT_STATUS[portno]);
			}
#else /****************************************************************/
			get_sfp_rtwdm_info_diag(portno, &PORT_STATUS[portno]);
#endif /* [#157] */
			update_port_rtwdm_rx_power(portno);
#endif

			/* get flex tune status */
			get_flex_tune_status(portno);

			/* get smart tsfp self loopback */
			get_smart_tsfp_self_loopback(portno, &enable);

			/* get rtwdm loopback */
			get_rtwdm_loopback(portno, &enable);

			/* get wavelength for channel no. */
			get_tunable_sfp_channel_no(portno);
		}
#else
		/* update flex tune status */
		update_flex_tune_status(portno);

		/* update smart tsfp self loopback */
		get_smart_tsfp_self_loopback(portno, &enable);

		/* update rtwdm loopback */
		get_rtwdm_loopback(portno, &enable);
#endif
#endif
	}
	return;
}


u32 MCU_INIT_COMPLETE_FLAG;
u32 MCU_KEEP_ALIVE_RETRY_COUNT;

#define MCU_KEEP_ALIVE_RETRY_MAX	15

// convert to decimal format value from float dbm value
// float -17.5 will be presented as -175 in integer.
#if 1 /* [#125] Fixing for SFP channel no, wavelength, tx/rx dBm, dustin, 2024-09-10 */
unsigned int convert_dbm_float_to_decimal(f32 val, int dbm_flag, int tx_flag)
#else
unsigned int convert_dbm_float_to_decimal(f32 val, int dbm_flag)
#endif /* [#125] */
{
	unsigned int unit_10, unit_1, unit_0, vvv, minus_flag;
	f32 temp;

	if(dbm_flag) {
		if(val > 10.0)
			return 0x100;
#if 1 /* [#125] Fixing for SFP channel no, wavelength, tx/rx dBm, dustin, 2024-09-10 */
		/* NOTE : rx/tx power have different min value. */
		if(tx_flag) {
			if(val < -15.0)
				return 0x8150;
		} else {
			if(val < -39.0)
				return 0x8535;
		}
#else
		if(val > 10.0)
			return 0x100;
		if(val < -15.0)
			return 0x8150;
#endif /* [#125] */
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
#if 1 /* [#91] Fixing for register updating feature, dustin, 2024-08-05 */
	unit_0 = (int)(temp * 10);
#else
	unit_0 = (int)temp;
#endif

	vvv = 0;
	if(minus_flag || (val == 0))
		vvv |= (1 << 15);
	vvv |= ((unit_10 << 8) & 0xf00);
	vvv |= ((unit_1 << 4) & 0xf0);
	vvv |= unit_0;
#if 1 /* [#125] Fixing for SFP channel no, wavelength, tx/rx dBm, dustin, 2024-09-10 */
	if(vvv == 0)
		vvv = 0x8000;
#endif
	return vvv;
}

#if 1 /* [#125] Fixing for SFP channel no, wavelength, tx/rx dBm, dustin, 2024-09-10 */
unsigned int convert_vcc_float_to_decimal(f32 val)
{
	unsigned int unit_10, unit_1, unit_0, unit_02, vvv, minus_flag;
	f32 temp;

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
	unit_0 = (int)(temp * 10);
	temp = temp - (f32)(unit_0 * 0.1);
	unit_02 = (int)(temp * 100);

	vvv = 0;
	if(minus_flag || (val == 0))
		vvv |= (1 << 15);
	vvv |= ((unit_10 << 12) & 0xf000);
	vvv |= ((unit_1 << 8) & 0xf00);
	vvv |= ((unit_0 << 4) & 0xf0);
	vvv |= unit_02 & 0xf;
	return vvv;
}
#endif

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

#if 1 /* [#107] Fixing for 2nd register updates, dustin, 2024-08-29 */
unsigned int convert_hex_to_decimal(unsigned int hval)
{
	unsigned int unit_1k, unit_100, unit_10, unit_1, vvv;

	unit_1k = (int)(hval / 1000);
	hval = hval - (int)(unit_1k * 1000);
	unit_100 = (int)(hval / 100);
	hval = hval - (int)(unit_100 * 100);
	unit_10 = (int)(hval / 10);
	hval = hval - (int)(unit_10 * 10);
	unit_1 = (int)(hval / 1);

	vvv = 0;
	vvv |= ((unit_1k << 12) & 0xF000);
	vvv |= ((unit_100 << 8) & 0x0F00);
	vvv |= ((unit_10 << 4)  & 0x00F0);
	vvv |= (unit_1 & 0xF);
	return vvv;
}

unsigned int convert_decimal_to_hex(unsigned int dval)
{
	unsigned int unit_1k, unit_100, unit_10, unit_1, vvv;

	unit_1k  = (dval >> 12) & 0xF;
	unit_100 = (dval >>  8) & 0xF;
	unit_10  = (dval >>  4) & 0xF;
	unit_1   = dval & 0xF;

	vvv = (unit_1k * 1000) + (unit_100 * 100) + (unit_10 * 10) + unit_1;
	return vvv;
}
#endif

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

#if 1/* [#78] Adding system inventory management, dustin, 2024-07-24 */
int read_bd_inventory(board_inventory_t *binv)
{
	FILE *fp = NULL;

	fp = fopen(EAG6L_INVENTORY_FILE, "r");
	if(fp == NULL) {
		zlog_err("%s : Cannot open inventory file %s. reason[%s].",
			__func__, EAG6L_INVENTORY_FILE, strerror(errno));
		return -1;
	}

	fread(binv, 1, sizeof(board_inventory_t)-16, fp);
	fclose(fp);
	return 0;
}
#endif

void process_hw_inventory_infos(void)
{
#if 1/* [#78] Adding system inventory management, dustin, 2024-07-24 */
	board_inventory_t inv;
	unsigned int val[20];
	char buff[32];

	// read iventory.
	read_bd_inventory(&inv);

	/* update manufacture. */
#ifdef BP_BYTE_SWAP /* [#91] Fixing for register updating feature, dustin, 2024-08-05 */
	val[0] = inv.manufacturer[1] | (inv.manufacturer[0] << 8);
	FPGA_WRITE(INV_HW_MANU_1_ADDR, val[0]);
	val[1] = inv.manufacturer[3] | (inv.manufacturer[2] << 8);
	FPGA_WRITE(INV_HW_MANU_2_ADDR, val[1]);
	val[2] = inv.manufacturer[5] | (inv.manufacturer[4] << 8);
	FPGA_WRITE(INV_HW_MANU_3_ADDR, val[2]);
	val[3] = inv.manufacturer[7] | (inv.manufacturer[6] << 8);
	FPGA_WRITE(INV_HW_MANU_4_ADDR, val[3]);
	val[4] = inv.manufacturer[9] | (inv.manufacturer[8] << 8);
	FPGA_WRITE(INV_HW_MANU_5_ADDR, val[4]);
#else
	val[0] = inv.manufacturer[0] | (inv.manufacturer[1] << 8);
	FPGA_WRITE(INV_HW_MANU_1_ADDR, val[0]);
	val[1] = inv.manufacturer[2] | (inv.manufacturer[3] << 8);
	FPGA_WRITE(INV_HW_MANU_2_ADDR, val[1]);
	val[2] = inv.manufacturer[4] | (inv.manufacturer[5] << 8);
	FPGA_WRITE(INV_HW_MANU_3_ADDR, val[2]);
	val[3] = inv.manufacturer[6] | (inv.manufacturer[7] << 8);
	FPGA_WRITE(INV_HW_MANU_4_ADDR, val[3]);
	val[4] = inv.manufacturer[8] | (inv.manufacturer[9] << 8);
	FPGA_WRITE(INV_HW_MANU_5_ADDR, val[4]);
#endif

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
#ifdef BP_BYTE_SWAP /* [#91] Fixing for register updating feature, dustin, 2024-08-05 */
    val[0] = inv.model_name[1] | (inv.model_name[0] << 8);
    FPGA_WRITE(INV_HW_MODEL_1_ADDR, val[0]);
    val[1] = inv.model_name[3] | (inv.model_name[2] << 8);
    FPGA_WRITE(INV_HW_MODEL_2_ADDR, val[1]);
    val[2] = inv.model_name[5] | (inv.model_name[4] << 8);
    FPGA_WRITE(INV_HW_MODEL_3_ADDR, val[2]);
    val[3] = inv.model_name[7] | (inv.model_name[6] << 8);
    FPGA_WRITE(INV_HW_MODEL_4_ADDR, val[3]);
    val[4] = inv.model_name[9] | (inv.model_name[8] << 8);
    FPGA_WRITE(INV_HW_MODEL_5_ADDR, val[4]);
#else
    val[0] = inv.model_name[0] | (inv.model_name[1] << 8);
    FPGA_WRITE(INV_HW_MODEL_1_ADDR, val[0]);
    val[1] = inv.model_name[2] | (inv.model_name[3] << 8);
    FPGA_WRITE(INV_HW_MODEL_2_ADDR, val[1]);
    val[2] = inv.model_name[4] | (inv.model_name[5] << 8);
    FPGA_WRITE(INV_HW_MODEL_3_ADDR, val[2]);
    val[3] = inv.model_name[6] | (inv.model_name[7] << 8);
    FPGA_WRITE(INV_HW_MODEL_4_ADDR, val[3]);
    val[4] = inv.model_name[8] | (inv.model_name[9] << 8);
    FPGA_WRITE(INV_HW_MODEL_5_ADDR, val[4]);
#endif

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
#ifdef BP_BYTE_SWAP /* [#91] Fixing for register updating feature, dustin, 2024-08-05 */
    val[0] = inv.part_number[1] | (inv.part_number[0] << 8);
    FPGA_WRITE(INV_HW_PN_1_ADDR, val[0]);
    val[1] = inv.part_number[3] | (inv.part_number[2] << 8);
    FPGA_WRITE(INV_HW_PN_2_ADDR, val[1]);
    val[2] = inv.part_number[5] | (inv.part_number[4] << 8);
    FPGA_WRITE(INV_HW_PN_3_ADDR, val[2]);
    val[3] = inv.part_number[7] | (inv.part_number[6] << 8);
    FPGA_WRITE(INV_HW_PN_4_ADDR, val[3]);
    val[4] = inv.part_number[9] | (inv.part_number[8] << 8);
    FPGA_WRITE(INV_HW_PN_5_ADDR, val[4]);
#if 1 /* [#97] Adding register recovery process after fpga reset, dustin, 2024-08-21 */
    val[5] = inv.part_number[11] | (inv.part_number[10] << 8);
    FPGA_WRITE(INV_HW_PN_6_ADDR, val[5]);
    val[6] = inv.part_number[13] | (inv.part_number[12] << 8);
    FPGA_WRITE(INV_HW_PN_7_ADDR, val[6]);
    val[7] = inv.part_number[15] | (inv.part_number[14] << 8);
    FPGA_WRITE(INV_HW_PN_8_ADDR, val[7]);
#else
    val[5] = inv.part_number[11] | (inv.part_number[10] << 8);
    FPGA_WRITE(INV_HW_PN_6_ADDR, val[4]);
    val[6] = inv.part_number[13] | (inv.part_number[12] << 8);
    FPGA_WRITE(INV_HW_PN_7_ADDR, val[4]);
    val[7] = inv.part_number[15] | (inv.part_number[14] << 8);
    FPGA_WRITE(INV_HW_PN_8_ADDR, val[4]);
#endif
#else
    val[0] = inv.part_number[0] | (inv.part_number[1] << 8);
    FPGA_WRITE(INV_HW_PN_1_ADDR, val[0]);
    val[1] = inv.part_number[2] | (inv.part_number[3] << 8);
    FPGA_WRITE(INV_HW_PN_2_ADDR, val[1]);
    val[2] = inv.part_number[4] | (inv.part_number[5] << 8);
    FPGA_WRITE(INV_HW_PN_3_ADDR, val[2]);
    val[3] = inv.part_number[6] | (inv.part_number[7] << 8);
    FPGA_WRITE(INV_HW_PN_4_ADDR, val[3]);
    val[4] = inv.part_number[8] | (inv.part_number[9] << 8);
    FPGA_WRITE(INV_HW_PN_5_ADDR, val[4]);
#if 1 /* [#97] Adding register recovery process after fpga reset, dustin, 2024-08-21 */
    val[5] = inv.part_number[10] | (inv.part_number[11] << 8);
    FPGA_WRITE(INV_HW_PN_6_ADDR, val[5]);
    val[6] = inv.part_number[12] | (inv.part_number[13] << 8);
    FPGA_WRITE(INV_HW_PN_7_ADDR, val[6]);
    val[7] = inv.part_number[14] | (inv.part_number[15] << 8);
    FPGA_WRITE(INV_HW_PN_8_ADDR, val[7]);
#else
    val[5] = inv.part_number[10] | (inv.part_number[11] << 8);
    FPGA_WRITE(INV_HW_PN_6_ADDR, val[4]);
    val[6] = inv.part_number[12] | (inv.part_number[13] << 8);
    FPGA_WRITE(INV_HW_PN_7_ADDR, val[4]);
    val[7] = inv.part_number[14] | (inv.part_number[15] << 8);
    FPGA_WRITE(INV_HW_PN_8_ADDR, val[4]);
#endif
#endif

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
#ifdef BP_BYTE_SWAP /* [#91] Fixing for register updating feature, dustin, 2024-08-05 */
    val[0] = inv.serial_number[1] | (inv.serial_number[0] << 8);
    FPGA_WRITE(INV_HW_SN_1_ADDR, val[0]);
    val[1] = inv.serial_number[3] | (inv.serial_number[2] << 8);
    FPGA_WRITE(INV_HW_SN_2_ADDR, val[1]);
    val[2] = inv.serial_number[5] | (inv.serial_number[4] << 8);
    FPGA_WRITE(INV_HW_SN_3_ADDR, val[2]);
    val[3] = inv.serial_number[7] | (inv.serial_number[6] << 8);
    FPGA_WRITE(INV_HW_SN_4_ADDR, val[3]);
    val[4] = inv.serial_number[9] | (inv.serial_number[8] << 8);
    FPGA_WRITE(INV_HW_SN_5_ADDR, val[4]);
    val[5] = inv.serial_number[11] | (inv.serial_number[10] << 8);
    FPGA_WRITE(INV_HW_SN_6_ADDR, val[5]);
    val[6] = inv.serial_number[13] | (inv.serial_number[12] << 8);
    FPGA_WRITE(INV_HW_SN_7_ADDR, val[6]);
    val[7] = inv.serial_number[15] | (inv.serial_number[14] << 8);
    FPGA_WRITE(INV_HW_SN_8_ADDR, val[7]);
#else
    val[0] = inv.serial_number[0] | (inv.serial_number[1] << 8);
    FPGA_WRITE(INV_HW_SN_1_ADDR, val[0]);
    val[1] = inv.serial_number[2] | (inv.serial_number[3] << 8);
    FPGA_WRITE(INV_HW_SN_2_ADDR, val[1]);
    val[2] = inv.serial_number[4] | (inv.serial_number[5] << 8);
    FPGA_WRITE(INV_HW_SN_3_ADDR, val[2]);
    val[3] = inv.serial_number[6] | (inv.serial_number[7] << 8);
    FPGA_WRITE(INV_HW_SN_4_ADDR, val[3]);
    val[4] = inv.serial_number[8] | (inv.serial_number[9] << 8);
    FPGA_WRITE(INV_HW_SN_5_ADDR, val[4]);
    val[5] = inv.serial_number[10] | (inv.serial_number[11] << 8);
    FPGA_WRITE(INV_HW_SN_6_ADDR, val[5]);
    val[6] = inv.serial_number[12] | (inv.serial_number[13] << 8);
    FPGA_WRITE(INV_HW_SN_7_ADDR, val[6]);
    val[7] = inv.serial_number[14] | (inv.serial_number[15] << 8);
    FPGA_WRITE(INV_HW_SN_8_ADDR, val[7]);
#endif

#ifdef DEBUG
    val[0] = FPGA_READ(INV_HW_SN_1_ADDR);
    val[1] = FPGA_READ(INV_HW_SN_2_ADDR);
    val[2] = FPGA_READ(INV_HW_SN_3_ADDR);
    val[3] = FPGA_READ(INV_HW_SN_4_ADDR);
    val[4] = FPGA_READ(INV_HW_SN_5_ADDR);
    val[5] = FPGA_READ(INV_HW_SN_6_ADDR);
    val[6] = FPGA_READ(INV_HW_SN_7_ADDR);
    val[7] = FPGA_READ(INV_HW_SN_8_ADDR);

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
    zlog_notice("INVENTORY: HW SN [%s]", buff);
#endif

	/* update h/w revision */
#ifdef BP_BYTE_SWAP /* [#97] Adding register recovery process after fpga reset, dustin, 2024-08-21 */
    val[0] = (inv.revision & 0xFF) | (inv.revision & 0xFF00);
    FPGA_WRITE(INV_HW_REV_2_ADDR, val[0]);
    val[1] = ((inv.revision >> 16) & 0xFF) | ((inv.revision >> 16) & 0xFF00);
    FPGA_WRITE(INV_HW_REV_1_ADDR, val[1]);
#else
    val[0] = (inv.revision & 0xFF) | (inv.revision & 0xFF00);
    FPGA_WRITE(INV_HW_REV_1_ADDR, val[0]);
    val[1] = ((inv.revision >> 16) & 0xFF) | ((inv.revision >> 16) & 0xFF00);
    FPGA_WRITE(INV_HW_REV_2_ADDR, val[1]);
#endif

#ifdef DEBUG
#ifdef BP_BYTE_SWAP /* [#97] Adding register recovery process after fpga reset, dustin, 2024-08-21 */
    val[0] = FPGA_READ(INV_HW_REV_2_ADDR);
    val[1] = FPGA_READ(INV_HW_REV_1_ADDR);
#else
    val[0] = FPGA_READ(INV_HW_REV_1_ADDR);
    val[1] = FPGA_READ(INV_HW_REV_2_ADDR);
#endif

    zlog_notice("INVENTORY: HW REV [0x%x]", val[0] | (val[1] << 16));
#endif

	/* update h/w manufacture date */
#ifdef BP_BYTE_SWAP /* [#91] Fixing for register updating feature, dustin, 2024-08-05 */
    val[0] = inv.manufacture_date[1] | (inv.manufacture_date[0] << 8);
    FPGA_WRITE(INV_HW_MDATE_1_ADDR, val[0]);
    val[1] = inv.manufacture_date[3] | (inv.manufacture_date[2] << 8);
    FPGA_WRITE(INV_HW_MDATE_2_ADDR, val[1]);
    val[2] = inv.manufacture_date[5] | (inv.manufacture_date[4] << 8);
    FPGA_WRITE(INV_HW_MDATE_3_ADDR, val[2]);
    val[3] = inv.manufacture_date[7] | (inv.manufacture_date[6] << 8);
    FPGA_WRITE(INV_HW_MDATE_4_ADDR, val[3]);
    val[4] = inv.manufacture_date[9] | (inv.manufacture_date[8] << 8);
    FPGA_WRITE(INV_HW_MDATE_5_ADDR, val[4]);
#else
    val[0] = inv.manufacture_date[0] | (inv.manufacture_date[1] << 8);
    FPGA_WRITE(INV_HW_MDATE_1_ADDR, val[0]);
    val[1] = inv.manufacture_date[2] | (inv.manufacture_date[3] << 8);
    FPGA_WRITE(INV_HW_MDATE_2_ADDR, val[1]);
    val[2] = inv.manufacture_date[4] | (inv.manufacture_date[5] << 8);
    FPGA_WRITE(INV_HW_MDATE_3_ADDR, val[2]);
    val[3] = inv.manufacture_date[6] | (inv.manufacture_date[7] << 8);
    FPGA_WRITE(INV_HW_MDATE_4_ADDR, val[3]);
    val[4] = inv.manufacture_date[8] | (inv.manufacture_date[9] << 8);
    FPGA_WRITE(INV_HW_MDATE_5_ADDR, val[4]);
#endif

#ifdef DEBUG
    val[0] = FPGA_READ(INV_HW_MDATE_1_ADDR);
    val[1] = FPGA_READ(INV_HW_MDATE_2_ADDR);
    val[2] = FPGA_READ(INV_HW_MDATE_3_ADDR);
    val[3] = FPGA_READ(INV_HW_MDATE_4_ADDR);
    val[4] = FPGA_READ(INV_HW_MDATE_5_ADDR);

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
    zlog_notice("INVENTORY: HW MDATE [%s]", buff);
#endif

	/* update h/w repair date */
#ifdef BP_BYTE_SWAP /* [#91] Fixing for register updating feature, dustin, 2024-08-05 */
    val[0] = inv.repair_date[1] | (inv.repair_date[0] << 8);
    FPGA_WRITE(INV_HW_RDATE_1_ADDR, val[0]);
    val[1] = inv.repair_date[3] | (inv.repair_date[2] << 8);
    FPGA_WRITE(INV_HW_RDATE_2_ADDR, val[1]);
    val[2] = inv.repair_date[5] | (inv.repair_date[4] << 8);
    FPGA_WRITE(INV_HW_RDATE_3_ADDR, val[2]);
    val[3] = inv.repair_date[7] | (inv.repair_date[6] << 8);
    FPGA_WRITE(INV_HW_RDATE_4_ADDR, val[3]);
    val[4] = inv.repair_date[9] | (inv.repair_date[8] << 8);
    FPGA_WRITE(INV_HW_RDATE_5_ADDR, val[4]);
#else
    val[0] = inv.repair_date[0] | (inv.repair_date[1] << 8);
    FPGA_WRITE(INV_HW_RDATE_1_ADDR, val[0]);
    val[1] = inv.repair_date[2] | (inv.repair_date[3] << 8);
    FPGA_WRITE(INV_HW_RDATE_2_ADDR, val[1]);
    val[2] = inv.repair_date[4] | (inv.repair_date[5] << 8);
    FPGA_WRITE(INV_HW_RDATE_3_ADDR, val[2]);
    val[3] = inv.repair_date[6] | (inv.repair_date[7] << 8);
    FPGA_WRITE(INV_HW_RDATE_4_ADDR, val[3]);
    val[4] = inv.repair_date[8] | (inv.repair_date[9] << 8);
    FPGA_WRITE(INV_HW_RDATE_5_ADDR, val[4]);
#endif

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
#ifdef BP_BYTE_SWAP /* [#97] Adding register recovery process after fpga reset, dustin, 2024-08-21 */
    val[0] = (inv.repair_code & 0xFF) | (inv.repair_code & 0xFF00);
    FPGA_WRITE(INV_HW_RCODE_2_ADDR, val[0]);
    val[1] = ((inv.repair_code >> 16) & 0xFF) | ((inv.repair_code >> 16) & 0xFF00);
    FPGA_WRITE(INV_HW_RCODE_1_ADDR, val[1]);
#else
    val[0] = (inv.repair_code & 0xFF) | (inv.repair_code & 0xFF00);
    FPGA_WRITE(INV_HW_RCODE_1_ADDR, val[0]);
    val[1] = ((inv.repair_code >> 16) & 0xFF) | ((inv.repair_code >> 16) & 0xFF00);
    FPGA_WRITE(INV_HW_RCODE_2_ADDR, val[1]);
#endif

#ifdef DEBUG
#ifdef BP_BYTE_SWAP /* [#97] Adding register recovery process after fpga reset, dustin, 2024-08-21 */
    val[0] = FPGA_READ(INV_HW_RCODE_2_ADDR);
    val[1] = FPGA_READ(INV_HW_RCODE_1_ADDR);
#else
    val[0] = FPGA_READ(INV_HW_RCODE_1_ADDR);
    val[1] = FPGA_READ(INV_HW_RCODE_2_ADDR);
#endif

    zlog_notice("INVENTORY: HW RCODE [0x%x]", val[0] | (val[1] << 16));
#endif
#else///////////////////////////////////////////////////////////////
	/*FIXME : should replaced with real inv data in flash. */
	struct inventory inv = { "hfrnet1234", "EAG6L12345", "PN123456789ABCD", 
		                     "SN12", "RV12", "240520010203", "2405200102", "RP01",
		                     "SNUIEAG6LL", "" };
	unsigned int val[20];
	char buff[32];

	/* update manufacture. */
	val[0] = inv.manufact[0] | (inv.manufact[1] << 8);
	FPGA_WRITE(INV_HW_MANU_1_ADDR, val[0]);
	val[1] = inv.manufact[2] | (inv.manufact[3] << 8);
	FPGA_WRITE(INV_HW_MANU_2_ADDR, val[1]);
	val[2] = inv.manufact[4] | (inv.manufact[5] << 8);
	FPGA_WRITE(INV_HW_MANU_3_ADDR, val[2]);
	val[3] = inv.manufact[6] | (inv.manufact[7] << 8);
	FPGA_WRITE(INV_HW_MANU_4_ADDR, val[3]);
	val[4] = inv.manufact[8] | (inv.manufact[9] << 8);
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
    val[0] = inv.model_num[0] | (inv.model_num[1] >> 8);
    FPGA_WRITE(INV_HW_MODEL_1_ADDR, val[0]);
    val[1] = inv.model_num[2] | (inv.model_num[3] >> 8);
    FPGA_WRITE(INV_HW_MODEL_2_ADDR, val[1]);
    val[2] = inv.model_num[4] | (inv.model_num[5] >> 8);
    FPGA_WRITE(INV_HW_MODEL_3_ADDR, val[2]);
    val[3] = inv.model_num[6] | (inv.model_num[7] >> 8);
    FPGA_WRITE(INV_HW_MODEL_4_ADDR, val[3]);
    val[4] = inv.model_num[8] | (inv.model_num[9] >> 8);
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
    val[0] = inv.part_num[0] | (inv.part_num[1] >> 8);
    FPGA_WRITE(INV_HW_PN_1_ADDR, val[0]);
    val[1] = inv.part_num[2] | (inv.part_num[3] >> 8);
    FPGA_WRITE(INV_HW_PN_2_ADDR, val[1]);
    val[2] = inv.part_num[4] | (inv.part_num[5] >> 8);
    FPGA_WRITE(INV_HW_PN_3_ADDR, val[2]);
    val[3] = inv.part_num[6] | (inv.part_num[7] >> 8);
    FPGA_WRITE(INV_HW_PN_4_ADDR, val[3]);
    val[4] = inv.part_num[8] | (inv.part_num[9] >> 8);
    FPGA_WRITE(INV_HW_PN_5_ADDR, val[4]);
    val[5] = inv.part_num[10] | (inv.part_num[11] >> 8);
    FPGA_WRITE(INV_HW_PN_6_ADDR, val[4]);
    val[6] = inv.part_num[12] | (inv.part_num[13] >> 8);
    FPGA_WRITE(INV_HW_PN_7_ADDR, val[4]);
    val[7] = inv.part_num[14] | (inv.part_num[15] >> 8);
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
    val[0] = inv.serial_num[0] | (inv.serial_num[1] >> 8);
    FPGA_WRITE(INV_HW_SN_1_ADDR, val[0]);
    val[1] = inv.serial_num[2] | (inv.serial_num[3] >> 8);
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
    val[0] = inv.revision[0] | (inv.revision[1] << 8);
    FPGA_WRITE(INV_HW_REV_1_ADDR, val[0]);
    val[1] = inv.revision[2] | (inv.revision[3] << 8);
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
    val[0] = inv.manufact_date[0] | (inv.manufact_date[1] >> 8);
    FPGA_WRITE(INV_HW_MDATE_1_ADDR, val[0]);
    val[1] = inv.manufact_date[2] | (inv.manufact_date[3] >> 8);
    FPGA_WRITE(INV_HW_MDATE_2_ADDR, val[1]);
    val[2] = inv.manufact_date[4] | (inv.manufact_date[5] >> 8);
    FPGA_WRITE(INV_HW_MDATE_3_ADDR, val[2]);
    val[3] = inv.manufact_date[6] | (inv.manufact_date[7] >> 8);
    FPGA_WRITE(INV_HW_MDATE_4_ADDR, val[3]);
    val[4] = inv.manufact_date[8] | (inv.manufact_date[9] >> 8);
    FPGA_WRITE(INV_HW_MDATE_5_ADDR, val[4]);
    val[5] = inv.manufact_date[10] | (inv.manufact_date[11] >> 8);
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
    val[0] = inv.repair_date[0] | (inv.repair_date[1] >> 8);
    FPGA_WRITE(INV_HW_RDATE_1_ADDR, val[0]);
    val[1] = inv.repair_date[2] | (inv.repair_date[3] >> 8);
    FPGA_WRITE(INV_HW_RDATE_2_ADDR, val[1]);
    val[2] = inv.repair_date[4] | (inv.repair_date[5] >> 8);
    FPGA_WRITE(INV_HW_RDATE_3_ADDR, val[2]);
    val[3] = inv.repair_date[6] | (inv.repair_date[7] >> 8);
    FPGA_WRITE(INV_HW_RDATE_4_ADDR, val[3]);
    val[4] = inv.repair_date[8] | (inv.repair_date[9] >> 8);
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
    val[0] = inv.repair_code[0] | (inv.repair_code[1] >> 8);
    FPGA_WRITE(INV_HW_RCODE_1_ADDR, val[0]);
    val[1] = inv.repair_code[2] | (inv.repair_code[3] >> 8);
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
#endif

	return;
}

#if 1 /* [#157] Fixing for Smart T-SFP rtWDM info, dustin, 2024-10-18 */
void update_port_sfp_inventory(uint8_t rtwdm_flag)
#else
void update_port_sfp_inventory(void)
#endif /* [#157] */
{
	int portno;
	unsigned int val;

#ifdef BP_BYTE_SWAP /* [#91] Fixing for register updating feature, dustin, 2024-08-05 */
#if 1 /* [#157] Fixing for Smart T-SFP rtWDM info, dustin, 2024-10-18 */
	if(rtwdm_flag)
		goto _rtwdm_stage_;
#endif

	/* update vendor name */
	for(portno = PORT_ID_EAG6L_PORT1; portno < PORT_ID_EAG6L_MAX; portno++) {
		val = (INV_TBL[portno].vendor[0] << 8) | INV_TBL[portno].vendor[1];
		FPGA_PORT_WRITE(__PORT_VENDOR1_ADDR[portno], val);
		val = (INV_TBL[portno].vendor[2] << 8) | INV_TBL[portno].vendor[3];
		FPGA_PORT_WRITE(__PORT_VENDOR2_ADDR[portno], val);
		val = (INV_TBL[portno].vendor[4] << 8) | INV_TBL[portno].vendor[5];
		FPGA_PORT_WRITE(__PORT_VENDOR3_ADDR[portno], val);
		val = (INV_TBL[portno].vendor[6] << 8) | INV_TBL[portno].vendor[7];
		FPGA_PORT_WRITE(__PORT_VENDOR4_ADDR[portno], val);
		val = (INV_TBL[portno].vendor[8] << 8) | INV_TBL[portno].vendor[9];
		FPGA_PORT_WRITE(__PORT_VENDOR5_ADDR[portno], val);
		val = (INV_TBL[portno].vendor[10] << 8) | INV_TBL[portno].vendor[11];
		FPGA_PORT_WRITE(__PORT_VENDOR6_ADDR[portno], val);
		val = (INV_TBL[portno].vendor[12] << 8) | INV_TBL[portno].vendor[13];
		FPGA_PORT_WRITE(__PORT_VENDOR7_ADDR[portno], val);
		val = (INV_TBL[portno].vendor[14] << 8) | INV_TBL[portno].vendor[15];
		FPGA_PORT_WRITE(__PORT_VENDOR8_ADDR[portno], val);
	}

	/* update part number */
	for(portno = PORT_ID_EAG6L_PORT1; portno < PORT_ID_EAG6L_MAX; portno++) {
		val = (INV_TBL[portno].part_num[0] << 8) | INV_TBL[portno].part_num[1];
		FPGA_PORT_WRITE(__PORT_PN1_ADDR[portno], val);
		val = (INV_TBL[portno].part_num[2] << 8) | INV_TBL[portno].part_num[3];
		FPGA_PORT_WRITE(__PORT_PN2_ADDR[portno], val);
		val = (INV_TBL[portno].part_num[4] << 8) | INV_TBL[portno].part_num[5];
		FPGA_PORT_WRITE(__PORT_PN3_ADDR[portno], val);
		val = (INV_TBL[portno].part_num[6] << 8) | INV_TBL[portno].part_num[7];
		FPGA_PORT_WRITE(__PORT_PN4_ADDR[portno], val);
		val = (INV_TBL[portno].part_num[8] << 8) | INV_TBL[portno].part_num[9];
		FPGA_PORT_WRITE(__PORT_PN5_ADDR[portno], val);
		val = (INV_TBL[portno].part_num[10] << 8) | INV_TBL[portno].part_num[11];
		FPGA_PORT_WRITE(__PORT_PN6_ADDR[portno], val);
		val = (INV_TBL[portno].part_num[12] << 8) | INV_TBL[portno].part_num[13];
		FPGA_PORT_WRITE(__PORT_PN7_ADDR[portno], val);
		val = (INV_TBL[portno].part_num[14] << 8) | INV_TBL[portno].part_num[15];
		FPGA_PORT_WRITE(__PORT_PN8_ADDR[portno], val);
	}

	/* update serial number */
	for(portno = PORT_ID_EAG6L_PORT1; portno < PORT_ID_EAG6L_MAX; portno++) {
		val = (INV_TBL[portno].serial_num[0] << 8) | INV_TBL[portno].serial_num[1];
		FPGA_PORT_WRITE(__PORT_SN1_ADDR[portno], val);
		val = (INV_TBL[portno].serial_num[2] << 8) | INV_TBL[portno].serial_num[3];
		FPGA_PORT_WRITE(__PORT_SN2_ADDR[portno], val);
		val = (INV_TBL[portno].serial_num[4] << 8) | INV_TBL[portno].serial_num[5];
		FPGA_PORT_WRITE(__PORT_SN3_ADDR[portno], val);
		val = (INV_TBL[portno].serial_num[6] << 8) | INV_TBL[portno].serial_num[7];
		FPGA_PORT_WRITE(__PORT_SN4_ADDR[portno], val);
		val = (INV_TBL[portno].serial_num[8] << 8) | INV_TBL[portno].serial_num[9];
		FPGA_PORT_WRITE(__PORT_SN5_ADDR[portno], val);
		val = (INV_TBL[portno].serial_num[10] << 8) | INV_TBL[portno].serial_num[11];
		FPGA_PORT_WRITE(__PORT_SN6_ADDR[portno], val);
		val = (INV_TBL[portno].serial_num[12] << 8) | INV_TBL[portno].serial_num[13];
		FPGA_PORT_WRITE(__PORT_SN7_ADDR[portno], val);
		val = (INV_TBL[portno].serial_num[14] << 8) | INV_TBL[portno].serial_num[15];
		FPGA_PORT_WRITE(__PORT_SN8_ADDR[portno], val);
	}

	/* update rate */
	for(portno = PORT_ID_EAG6L_PORT1; portno < PORT_ID_EAG6L_MAX; portno++) {
		if(portno < PORT_ID_EAG6L_PORT7)
			val = INV_TBL[portno].max_rate;
		else
			val = INV_TBL[portno].max_rate * 10;/*convert 1G unit to 100M unit. */
		FPGA_PORT_WRITE(__PORT_RATE_ADDR[portno], val);
	}

	/* update distance */
	for(portno = PORT_ID_EAG6L_PORT1; portno < PORT_ID_EAG6L_MAX; portno++) {
		FPGA_PORT_WRITE(__PORT_DIST_ADDR[portno], INV_TBL[portno].dist);
	}

#if 1 /* [#157] Fixing for Smart T-SFP rtWDM info, dustin, 2024-10-18 */
	/* do not update rtwdm if flag is not set. */
	if(! rtwdm_flag)
		return;

_rtwdm_stage_:
#endif

#if 1/* [#72] Adding omitted rtWDM related registers, dustin, 2024-06-27 */
#if 1 /* [#157] Fixing for Smart T-SFP rtWDM info, dustin, 2024-10-18 */
	/* consider exceptional cases. */
	for(portno = PORT_ID_EAG6L_PORT1; portno < PORT_ID_EAG6L_MAX; portno++) {
		/* update only one time for tunable sfp. */
		if(! PORT_STATUS[portno].tunable_sfp)
			continue;

		/* clear registers if port is los. */
		if((! PORT_STATUS[portno].link) || PORT_STATUS[portno].los) {
			if(! PORT_STATUS[portno].inv_clear_flag) {
				FPGA_PORT_WRITE(__PORT_VENDOR1_RTWDM_ADDR[portno], 0x0);
				FPGA_PORT_WRITE(__PORT_VENDOR2_RTWDM_ADDR[portno], 0x0);
				FPGA_PORT_WRITE(__PORT_VENDOR3_RTWDM_ADDR[portno], 0x0);
				FPGA_PORT_WRITE(__PORT_VENDOR4_RTWDM_ADDR[portno], 0x0);
				FPGA_PORT_WRITE(__PORT_VENDOR5_RTWDM_ADDR[portno], 0x0);
				FPGA_PORT_WRITE(__PORT_VENDOR6_RTWDM_ADDR[portno], 0x0);
				FPGA_PORT_WRITE(__PORT_VENDOR7_RTWDM_ADDR[portno], 0x0);
				FPGA_PORT_WRITE(__PORT_VENDOR8_RTWDM_ADDR[portno], 0x0);
				FPGA_PORT_WRITE(__PORT_PN1_RTWDM_ADDR[portno], 0x0);
				FPGA_PORT_WRITE(__PORT_PN2_RTWDM_ADDR[portno], 0x0);
				FPGA_PORT_WRITE(__PORT_PN3_RTWDM_ADDR[portno], 0x0);
				FPGA_PORT_WRITE(__PORT_PN4_RTWDM_ADDR[portno], 0x0);
				FPGA_PORT_WRITE(__PORT_PN5_RTWDM_ADDR[portno], 0x0);
				FPGA_PORT_WRITE(__PORT_PN6_RTWDM_ADDR[portno], 0x0);
				FPGA_PORT_WRITE(__PORT_PN7_RTWDM_ADDR[portno], 0x0);
				FPGA_PORT_WRITE(__PORT_PN8_RTWDM_ADDR[portno], 0x0);
				FPGA_PORT_WRITE(__PORT_SN1_RTWDM_ADDR[portno], 0x0);
				FPGA_PORT_WRITE(__PORT_SN2_RTWDM_ADDR[portno], 0x0);
				FPGA_PORT_WRITE(__PORT_SN3_RTWDM_ADDR[portno], 0x0);
				FPGA_PORT_WRITE(__PORT_SN4_RTWDM_ADDR[portno], 0x0);
				FPGA_PORT_WRITE(__PORT_SN5_RTWDM_ADDR[portno], 0x0);
				FPGA_PORT_WRITE(__PORT_SN6_RTWDM_ADDR[portno], 0x0);
				FPGA_PORT_WRITE(__PORT_SN7_RTWDM_ADDR[portno], 0x0);
				FPGA_PORT_WRITE(__PORT_SN8_RTWDM_ADDR[portno], 0x0);
				FPGA_PORT_WRITE(__PORT_RATE_RTWDM_ADDR[portno], 0x0);
				FPGA_PORT_WRITE(__PORT_DIST_RTWDM_ADDR[portno], 0x0);

				PORT_STATUS[portno].inv_clear_flag = 1;/*once-updated-so-no-more-update*/
				continue;
			}
		}

		/* do not update if all fields are ready */
		if((! RTWDM_INV_TBL[portno].vendor[0]) ||
		   (! RTWDM_INV_TBL[portno].part_num[0]) ||
		   (! RTWDM_INV_TBL[portno].serial_num[0]) ||
		   (! RTWDM_INV_TBL[portno].max_rate) ||
		   (! RTWDM_INV_TBL[portno].dist) ||
		   PORT_STATUS[portno].inv_up_flag)
			continue;

		/* update vendor name */
		val = (RTWDM_INV_TBL[portno].vendor[0] << 8) | RTWDM_INV_TBL[portno].vendor[1];
		FPGA_PORT_WRITE(__PORT_VENDOR1_RTWDM_ADDR[portno], val);
		val = (RTWDM_INV_TBL[portno].vendor[2] << 8) | RTWDM_INV_TBL[portno].vendor[3];
		FPGA_PORT_WRITE(__PORT_VENDOR2_RTWDM_ADDR[portno], val);
		val = (RTWDM_INV_TBL[portno].vendor[4] << 8) | RTWDM_INV_TBL[portno].vendor[5];
		FPGA_PORT_WRITE(__PORT_VENDOR3_RTWDM_ADDR[portno], val);
		val = (RTWDM_INV_TBL[portno].vendor[6] << 8) | RTWDM_INV_TBL[portno].vendor[7];
		FPGA_PORT_WRITE(__PORT_VENDOR4_RTWDM_ADDR[portno], val);
		val = (RTWDM_INV_TBL[portno].vendor[8] << 8) | RTWDM_INV_TBL[portno].vendor[9];
		FPGA_PORT_WRITE(__PORT_VENDOR5_RTWDM_ADDR[portno], val);
		val = (RTWDM_INV_TBL[portno].vendor[10] << 8) | RTWDM_INV_TBL[portno].vendor[11];
		FPGA_PORT_WRITE(__PORT_VENDOR6_RTWDM_ADDR[portno], val);
		val = (RTWDM_INV_TBL[portno].vendor[12] << 8) | RTWDM_INV_TBL[portno].vendor[13];
		FPGA_PORT_WRITE(__PORT_VENDOR7_RTWDM_ADDR[portno], val);
		val = (RTWDM_INV_TBL[portno].vendor[14] << 8) | RTWDM_INV_TBL[portno].vendor[15];
		FPGA_PORT_WRITE(__PORT_VENDOR8_RTWDM_ADDR[portno], val);

		/* update part number */
		val = (RTWDM_INV_TBL[portno].part_num[0] << 8) | RTWDM_INV_TBL[portno].part_num[1];
		FPGA_PORT_WRITE(__PORT_PN1_RTWDM_ADDR[portno], val);
		val = (RTWDM_INV_TBL[portno].part_num[2] << 8) | RTWDM_INV_TBL[portno].part_num[3];
		FPGA_PORT_WRITE(__PORT_PN2_RTWDM_ADDR[portno], val);
		val = (RTWDM_INV_TBL[portno].part_num[4] << 8) | RTWDM_INV_TBL[portno].part_num[5];
		FPGA_PORT_WRITE(__PORT_PN3_RTWDM_ADDR[portno], val);
		val = (RTWDM_INV_TBL[portno].part_num[6] << 8) | RTWDM_INV_TBL[portno].part_num[7];
		FPGA_PORT_WRITE(__PORT_PN4_RTWDM_ADDR[portno], val);
		val = (RTWDM_INV_TBL[portno].part_num[8] << 8) | RTWDM_INV_TBL[portno].part_num[9];
		FPGA_PORT_WRITE(__PORT_PN5_RTWDM_ADDR[portno], val);
		val = (RTWDM_INV_TBL[portno].part_num[10] << 8) | RTWDM_INV_TBL[portno].part_num[11];
		FPGA_PORT_WRITE(__PORT_PN6_RTWDM_ADDR[portno], val);
		val = (RTWDM_INV_TBL[portno].part_num[12] << 8) | RTWDM_INV_TBL[portno].part_num[13];
		FPGA_PORT_WRITE(__PORT_PN7_RTWDM_ADDR[portno], val);
		val = (RTWDM_INV_TBL[portno].part_num[14] << 8) | RTWDM_INV_TBL[portno].part_num[15];
		FPGA_PORT_WRITE(__PORT_PN8_RTWDM_ADDR[portno], val);

		/* update serial number */
		val = (RTWDM_INV_TBL[portno].serial_num[0] << 8) | RTWDM_INV_TBL[portno].serial_num[1];
		FPGA_PORT_WRITE(__PORT_SN1_RTWDM_ADDR[portno], val);
		val = (RTWDM_INV_TBL[portno].serial_num[2] << 8) | RTWDM_INV_TBL[portno].serial_num[3];
		FPGA_PORT_WRITE(__PORT_SN2_RTWDM_ADDR[portno], val);
		val = (RTWDM_INV_TBL[portno].serial_num[4] << 8) | RTWDM_INV_TBL[portno].serial_num[5];
		FPGA_PORT_WRITE(__PORT_SN3_RTWDM_ADDR[portno], val);
		val = (RTWDM_INV_TBL[portno].serial_num[6] << 8) | RTWDM_INV_TBL[portno].serial_num[7];
		FPGA_PORT_WRITE(__PORT_SN4_RTWDM_ADDR[portno], val);
		val = (RTWDM_INV_TBL[portno].serial_num[8] << 8) | RTWDM_INV_TBL[portno].serial_num[9];
		FPGA_PORT_WRITE(__PORT_SN5_RTWDM_ADDR[portno], val);
		val = (RTWDM_INV_TBL[portno].serial_num[10] << 8) | RTWDM_INV_TBL[portno].serial_num[11];
		FPGA_PORT_WRITE(__PORT_SN6_RTWDM_ADDR[portno], val);
		val = (RTWDM_INV_TBL[portno].serial_num[12] << 8) | RTWDM_INV_TBL[portno].serial_num[13];
		FPGA_PORT_WRITE(__PORT_SN7_RTWDM_ADDR[portno], val);
		val = (RTWDM_INV_TBL[portno].serial_num[14] << 8) | RTWDM_INV_TBL[portno].serial_num[15];
		FPGA_PORT_WRITE(__PORT_SN8_RTWDM_ADDR[portno], val);

		/* update rate */
		if(portno < PORT_ID_EAG6L_PORT7)
			val = RTWDM_INV_TBL[portno].max_rate;
		else
			val = RTWDM_INV_TBL[portno].max_rate * 10;/*convert 1G unit to 100M unit. */
		FPGA_PORT_WRITE(__PORT_RATE_RTWDM_ADDR[portno], val);

		/* update distance */
		FPGA_PORT_WRITE(__PORT_DIST_RTWDM_ADDR[portno], RTWDM_INV_TBL[portno].dist);

		PORT_STATUS[portno].inv_up_flag = 1;/*once-updated-so-no-more-update*/
	}
#else /********************************************************************/
	/* update vendor name */
	for(portno = PORT_ID_EAG6L_PORT1; portno < PORT_ID_EAG6L_MAX; portno++) {
		if(! PORT_STATUS[portno].tunable_sfp)
			continue;

		val = (RTWDM_INV_TBL[portno].vendor[0] << 8) | RTWDM_INV_TBL[portno].vendor[1];
		FPGA_PORT_WRITE(__PORT_VENDOR1_RTWDM_ADDR[portno], val);
		val = (RTWDM_INV_TBL[portno].vendor[2] << 8) | RTWDM_INV_TBL[portno].vendor[3];
		FPGA_PORT_WRITE(__PORT_VENDOR2_RTWDM_ADDR[portno], val);
		val = (RTWDM_INV_TBL[portno].vendor[4] << 8) | RTWDM_INV_TBL[portno].vendor[5];
		FPGA_PORT_WRITE(__PORT_VENDOR3_RTWDM_ADDR[portno], val);
		val = (RTWDM_INV_TBL[portno].vendor[6] << 8) | RTWDM_INV_TBL[portno].vendor[7];
		FPGA_PORT_WRITE(__PORT_VENDOR4_RTWDM_ADDR[portno], val);
		val = (RTWDM_INV_TBL[portno].vendor[8] << 8) | RTWDM_INV_TBL[portno].vendor[9];
		FPGA_PORT_WRITE(__PORT_VENDOR5_RTWDM_ADDR[portno], val);
		val = (RTWDM_INV_TBL[portno].vendor[10] << 8) | RTWDM_INV_TBL[portno].vendor[11];
		FPGA_PORT_WRITE(__PORT_VENDOR6_RTWDM_ADDR[portno], val);
		val = (RTWDM_INV_TBL[portno].vendor[12] << 8) | RTWDM_INV_TBL[portno].vendor[13];
		FPGA_PORT_WRITE(__PORT_VENDOR7_RTWDM_ADDR[portno], val);
		val = (RTWDM_INV_TBL[portno].vendor[14] << 8) | RTWDM_INV_TBL[portno].vendor[15];
		FPGA_PORT_WRITE(__PORT_VENDOR8_RTWDM_ADDR[portno], val);
	}

	/* update part number */
	for(portno = PORT_ID_EAG6L_PORT1; portno < PORT_ID_EAG6L_MAX; portno++) {
		if(! PORT_STATUS[portno].tunable_sfp)
			continue;

		val = (RTWDM_INV_TBL[portno].part_num[0] << 8) | RTWDM_INV_TBL[portno].part_num[1];
		FPGA_PORT_WRITE(__PORT_PN1_RTWDM_ADDR[portno], val);
		val = (RTWDM_INV_TBL[portno].part_num[2] << 8) | RTWDM_INV_TBL[portno].part_num[3];
		FPGA_PORT_WRITE(__PORT_PN2_RTWDM_ADDR[portno], val);
		val = (RTWDM_INV_TBL[portno].part_num[4] << 8) | RTWDM_INV_TBL[portno].part_num[5];
		FPGA_PORT_WRITE(__PORT_PN3_RTWDM_ADDR[portno], val);
		val = (RTWDM_INV_TBL[portno].part_num[6] << 8) | RTWDM_INV_TBL[portno].part_num[7];
		FPGA_PORT_WRITE(__PORT_PN4_RTWDM_ADDR[portno], val);
		val = (RTWDM_INV_TBL[portno].part_num[8] << 8) | RTWDM_INV_TBL[portno].part_num[9];
		FPGA_PORT_WRITE(__PORT_PN5_RTWDM_ADDR[portno], val);
		val = (RTWDM_INV_TBL[portno].part_num[10] << 8) | RTWDM_INV_TBL[portno].part_num[11];
		FPGA_PORT_WRITE(__PORT_PN6_RTWDM_ADDR[portno], val);
		val = (RTWDM_INV_TBL[portno].part_num[12] << 8) | RTWDM_INV_TBL[portno].part_num[13];
		FPGA_PORT_WRITE(__PORT_PN7_RTWDM_ADDR[portno], val);
		val = (RTWDM_INV_TBL[portno].part_num[14] << 8) | RTWDM_INV_TBL[portno].part_num[15];
		FPGA_PORT_WRITE(__PORT_PN8_RTWDM_ADDR[portno], val);
	}

	/* update serial number */
	for(portno = PORT_ID_EAG6L_PORT1; portno < PORT_ID_EAG6L_MAX; portno++) {
		if(! PORT_STATUS[portno].tunable_sfp)
			continue;

		val = (RTWDM_INV_TBL[portno].serial_num[0] << 8) | RTWDM_INV_TBL[portno].serial_num[1];
		FPGA_PORT_WRITE(__PORT_SN1_RTWDM_ADDR[portno], val);
		val = (RTWDM_INV_TBL[portno].serial_num[2] << 8) | RTWDM_INV_TBL[portno].serial_num[3];
		FPGA_PORT_WRITE(__PORT_SN2_RTWDM_ADDR[portno], val);
		val = (RTWDM_INV_TBL[portno].serial_num[4] << 8) | RTWDM_INV_TBL[portno].serial_num[5];
		FPGA_PORT_WRITE(__PORT_SN3_RTWDM_ADDR[portno], val);
		val = (RTWDM_INV_TBL[portno].serial_num[6] << 8) | RTWDM_INV_TBL[portno].serial_num[7];
		FPGA_PORT_WRITE(__PORT_SN4_RTWDM_ADDR[portno], val);
		val = (RTWDM_INV_TBL[portno].serial_num[8] << 8) | RTWDM_INV_TBL[portno].serial_num[9];
		FPGA_PORT_WRITE(__PORT_SN5_RTWDM_ADDR[portno], val);
		val = (RTWDM_INV_TBL[portno].serial_num[10] << 8) | RTWDM_INV_TBL[portno].serial_num[11];
		FPGA_PORT_WRITE(__PORT_SN6_RTWDM_ADDR[portno], val);
		val = (RTWDM_INV_TBL[portno].serial_num[12] << 8) | RTWDM_INV_TBL[portno].serial_num[13];
		FPGA_PORT_WRITE(__PORT_SN7_RTWDM_ADDR[portno], val);
		val = (RTWDM_INV_TBL[portno].serial_num[14] << 8) | RTWDM_INV_TBL[portno].serial_num[15];
		FPGA_PORT_WRITE(__PORT_SN8_RTWDM_ADDR[portno], val);
	}

	/* update rate */
	for(portno = PORT_ID_EAG6L_PORT1; portno < PORT_ID_EAG6L_MAX; portno++) {
		if(! PORT_STATUS[portno].tunable_sfp)
			continue;

		if(portno < PORT_ID_EAG6L_PORT7)
			val = RTWDM_INV_TBL[portno].max_rate;
		else
			val = RTWDM_INV_TBL[portno].max_rate * 10;/*convert 1G unit to 100M unit. */
		FPGA_PORT_WRITE(__PORT_RATE_RTWDM_ADDR[portno], val);
	}

	/* update distance */
	for(portno = PORT_ID_EAG6L_PORT1; portno < PORT_ID_EAG6L_MAX; portno++) {
		if(! PORT_STATUS[portno].tunable_sfp)
			continue;

		FPGA_PORT_WRITE(__PORT_DIST_RTWDM_ADDR[portno], RTWDM_INV_TBL[portno].dist);
	}
#endif /* [#157] */
#endif
#else /**********************************************************************/
	/* update vendor name */
	for(portno = PORT_ID_EAG6L_PORT1; portno < PORT_ID_EAG6L_MAX; portno++) {
		val = (INV_TBL[portno].vendor[1] << 8) | INV_TBL[portno].vendor[0];
		FPGA_PORT_WRITE(__PORT_VENDOR1_ADDR[portno], val);
		val = (INV_TBL[portno].vendor[3] << 8) | INV_TBL[portno].vendor[2];
		FPGA_PORT_WRITE(__PORT_VENDOR2_ADDR[portno], val);
		val = (INV_TBL[portno].vendor[5] << 8) | INV_TBL[portno].vendor[4];
		FPGA_PORT_WRITE(__PORT_VENDOR3_ADDR[portno], val);
		val = (INV_TBL[portno].vendor[7] << 8) | INV_TBL[portno].vendor[6];
		FPGA_PORT_WRITE(__PORT_VENDOR4_ADDR[portno], val);
		val = (INV_TBL[portno].vendor[9] << 8) | INV_TBL[portno].vendor[8];
		FPGA_PORT_WRITE(__PORT_VENDOR5_ADDR[portno], val);
		val = (INV_TBL[portno].vendor[11] << 8) | INV_TBL[portno].vendor[10];
		FPGA_PORT_WRITE(__PORT_VENDOR6_ADDR[portno], val);
		val = (INV_TBL[portno].vendor[13] << 8) | INV_TBL[portno].vendor[12];
		FPGA_PORT_WRITE(__PORT_VENDOR7_ADDR[portno], val);
		val = (INV_TBL[portno].vendor[15] << 8) | INV_TBL[portno].vendor[14];
		FPGA_PORT_WRITE(__PORT_VENDOR8_ADDR[portno], val);
	}

	/* update part number */
	for(portno = PORT_ID_EAG6L_PORT1; portno < PORT_ID_EAG6L_MAX; portno++) {
		val = (INV_TBL[portno].part_num[1] << 8) | INV_TBL[portno].part_num[0];
		FPGA_PORT_WRITE(__PORT_PN1_ADDR[portno], val);
		val = (INV_TBL[portno].part_num[3] << 8) | INV_TBL[portno].part_num[2];
		FPGA_PORT_WRITE(__PORT_PN2_ADDR[portno], val);
		val = (INV_TBL[portno].part_num[5] << 8) | INV_TBL[portno].part_num[4];
		FPGA_PORT_WRITE(__PORT_PN3_ADDR[portno], val);
		val = (INV_TBL[portno].part_num[7] << 8) | INV_TBL[portno].part_num[6];
		FPGA_PORT_WRITE(__PORT_PN4_ADDR[portno], val);
		val = (INV_TBL[portno].part_num[9] << 8) | INV_TBL[portno].part_num[8];
		FPGA_PORT_WRITE(__PORT_PN5_ADDR[portno], val);
		val = (INV_TBL[portno].part_num[11] << 8) | INV_TBL[portno].part_num[10];
		FPGA_PORT_WRITE(__PORT_PN6_ADDR[portno], val);
		val = (INV_TBL[portno].part_num[13] << 8) | INV_TBL[portno].part_num[12];
		FPGA_PORT_WRITE(__PORT_PN7_ADDR[portno], val);
		val = (INV_TBL[portno].part_num[15] << 8) | INV_TBL[portno].part_num[14];
		FPGA_PORT_WRITE(__PORT_PN8_ADDR[portno], val);
	}

	/* update serial number */
	for(portno = PORT_ID_EAG6L_PORT1; portno < PORT_ID_EAG6L_MAX; portno++) {
		val = (INV_TBL[portno].serial_num[1] << 8) | INV_TBL[portno].serial_num[0];
		FPGA_PORT_WRITE(__PORT_SN1_ADDR[portno], val);
		val = (INV_TBL[portno].serial_num[3] << 8) | INV_TBL[portno].serial_num[2];
		FPGA_PORT_WRITE(__PORT_SN2_ADDR[portno], val);
		val = (INV_TBL[portno].serial_num[5] << 8) | INV_TBL[portno].serial_num[4];
		FPGA_PORT_WRITE(__PORT_SN3_ADDR[portno], val);
		val = (INV_TBL[portno].serial_num[7] << 8) | INV_TBL[portno].serial_num[6];
		FPGA_PORT_WRITE(__PORT_SN4_ADDR[portno], val);
		val = (INV_TBL[portno].serial_num[9] << 8) | INV_TBL[portno].serial_num[8];
		FPGA_PORT_WRITE(__PORT_SN5_ADDR[portno], val);
		val = (INV_TBL[portno].serial_num[11] << 8) | INV_TBL[portno].serial_num[10];
		FPGA_PORT_WRITE(__PORT_SN6_ADDR[portno], val);
		val = (INV_TBL[portno].serial_num[13] << 8) | INV_TBL[portno].serial_num[12];
		FPGA_PORT_WRITE(__PORT_SN7_ADDR[portno], val);
		val = (INV_TBL[portno].serial_num[15] << 8) | INV_TBL[portno].serial_num[14];
		FPGA_PORT_WRITE(__PORT_SN8_ADDR[portno], val);
	}

	/* update rate */
	for(portno = PORT_ID_EAG6L_PORT1; portno < PORT_ID_EAG6L_MAX; portno++) {
		if(portno < PORT_ID_EAG6L_PORT7)
			val = INV_TBL[portno].max_rate;
		else
			val = INV_TBL[portno].max_rate * 10;/*convert 1G unit to 100M unit. */
		FPGA_PORT_WRITE(__PORT_RATE_ADDR[portno], val);
	}

	/* update distance */
	for(portno = PORT_ID_EAG6L_PORT1; portno < PORT_ID_EAG6L_MAX; portno++) {
		FPGA_PORT_WRITE(__PORT_DIST_ADDR[portno], INV_TBL[portno].dist);
	}

#if 1/* [#72] Adding omitted rtWDM related registers, dustin, 2024-06-27 */
	/* update vendor name */
	for(portno = PORT_ID_EAG6L_PORT1; portno < PORT_ID_EAG6L_MAX; portno++) {
		if(! PORT_STATUS[portno].tunable_sfp)
			continue;

		val = (INV_TBL[portno].vendor[1] << 8) | INV_TBL[portno].vendor[0];
		FPGA_PORT_WRITE(__PORT_VENDOR1_RTWDM_ADDR[portno], val);
		val = (INV_TBL[portno].vendor[3] << 8) | INV_TBL[portno].vendor[2];
		FPGA_PORT_WRITE(__PORT_VENDOR2_RTWDM_ADDR[portno], val);
		val = (INV_TBL[portno].vendor[5] << 8) | INV_TBL[portno].vendor[4];
		FPGA_PORT_WRITE(__PORT_VENDOR3_RTWDM_ADDR[portno], val);
		val = (INV_TBL[portno].vendor[7] << 8) | INV_TBL[portno].vendor[6];
		FPGA_PORT_WRITE(__PORT_VENDOR4_RTWDM_ADDR[portno], val);
		val = (INV_TBL[portno].vendor[9] << 8) | INV_TBL[portno].vendor[8];
		FPGA_PORT_WRITE(__PORT_VENDOR5_RTWDM_ADDR[portno], val);
		val = (INV_TBL[portno].vendor[11] << 8) | INV_TBL[portno].vendor[10];
		FPGA_PORT_WRITE(__PORT_VENDOR6_RTWDM_ADDR[portno], val);
		val = (INV_TBL[portno].vendor[13] << 8) | INV_TBL[portno].vendor[12];
		FPGA_PORT_WRITE(__PORT_VENDOR7_RTWDM_ADDR[portno], val);
		val = (INV_TBL[portno].vendor[15] << 8) | INV_TBL[portno].vendor[14];
		FPGA_PORT_WRITE(__PORT_VENDOR8_RTWDM_ADDR[portno], val);
	}

	/* update part number */
	for(portno = PORT_ID_EAG6L_PORT1; portno < PORT_ID_EAG6L_MAX; portno++) {
		if(! PORT_STATUS[portno].tunable_sfp)
			continue;

		val = (INV_TBL[portno].part_num[1] << 8) | INV_TBL[portno].part_num[0];
		FPGA_PORT_WRITE(__PORT_PN1_RTWDM_ADDR[portno], val);
		val = (INV_TBL[portno].part_num[3] << 8) | INV_TBL[portno].part_num[2];
		FPGA_PORT_WRITE(__PORT_PN2_RTWDM_ADDR[portno], val);
		val = (INV_TBL[portno].part_num[5] << 8) | INV_TBL[portno].part_num[4];
		FPGA_PORT_WRITE(__PORT_PN3_RTWDM_ADDR[portno], val);
		val = (INV_TBL[portno].part_num[7] << 8) | INV_TBL[portno].part_num[6];
		FPGA_PORT_WRITE(__PORT_PN4_RTWDM_ADDR[portno], val);
		val = (INV_TBL[portno].part_num[9] << 8) | INV_TBL[portno].part_num[8];
		FPGA_PORT_WRITE(__PORT_PN5_RTWDM_ADDR[portno], val);
		val = (INV_TBL[portno].part_num[11] << 8) | INV_TBL[portno].part_num[10];
		FPGA_PORT_WRITE(__PORT_PN6_RTWDM_ADDR[portno], val);
		val = (INV_TBL[portno].part_num[13] << 8) | INV_TBL[portno].part_num[12];
		FPGA_PORT_WRITE(__PORT_PN7_RTWDM_ADDR[portno], val);
		val = (INV_TBL[portno].part_num[15] << 8) | INV_TBL[portno].part_num[14];
		FPGA_PORT_WRITE(__PORT_PN8_RTWDM_ADDR[portno], val);
	}

	/* update serial number */
	for(portno = PORT_ID_EAG6L_PORT1; portno < PORT_ID_EAG6L_MAX; portno++) {
		if(! PORT_STATUS[portno].tunable_sfp)
			continue;

		val = (INV_TBL[portno].serial_num[1] << 8) | INV_TBL[portno].serial_num[0];
		FPGA_PORT_WRITE(__PORT_SN1_RTWDM_ADDR[portno], val);
		val = (INV_TBL[portno].serial_num[3] << 8) | INV_TBL[portno].serial_num[2];
		FPGA_PORT_WRITE(__PORT_SN2_RTWDM_ADDR[portno], val);
		val = (INV_TBL[portno].serial_num[5] << 8) | INV_TBL[portno].serial_num[4];
		FPGA_PORT_WRITE(__PORT_SN3_RTWDM_ADDR[portno], val);
		val = (INV_TBL[portno].serial_num[7] << 8) | INV_TBL[portno].serial_num[6];
		FPGA_PORT_WRITE(__PORT_SN4_RTWDM_ADDR[portno], val);
		val = (INV_TBL[portno].serial_num[9] << 8) | INV_TBL[portno].serial_num[8];
		FPGA_PORT_WRITE(__PORT_SN5_RTWDM_ADDR[portno], val);
		val = (INV_TBL[portno].serial_num[11] << 8) | INV_TBL[portno].serial_num[10];
		FPGA_PORT_WRITE(__PORT_SN6_RTWDM_ADDR[portno], val);
		val = (INV_TBL[portno].serial_num[13] << 8) | INV_TBL[portno].serial_num[12];
		FPGA_PORT_WRITE(__PORT_SN7_RTWDM_ADDR[portno], val);
		val = (INV_TBL[portno].serial_num[15] << 8) | INV_TBL[portno].serial_num[14];
		FPGA_PORT_WRITE(__PORT_SN8_RTWDM_ADDR[portno], val);
	}

	/* update rate */
	for(portno = PORT_ID_EAG6L_PORT1; portno < PORT_ID_EAG6L_MAX; portno++) {
		if(! PORT_STATUS[portno].tunable_sfp)
			continue;

		if(portno < PORT_ID_EAG6L_PORT7)
			val = INV_TBL[portno].max_rate;
		else
			val = INV_TBL[portno].max_rate * 10;/*convert 1G unit to 100M unit. */
		FPGA_PORT_WRITE(__PORT_RATE_RTWDM_ADDR[portno], val);
	}

	/* update distance */
	for(portno = PORT_ID_EAG6L_PORT1; portno < PORT_ID_EAG6L_MAX; portno++) {
		if(! PORT_STATUS[portno].tunable_sfp)
			continue;

		FPGA_PORT_WRITE(__PORT_DIST_RTWDM_ADDR[portno], INV_TBL[portno].dist);
	}
#endif
#endif/*BP_BYTE_SWAP*/

#if 0/*[#61] Adding omitted functions, dustin, 2024-06-24 */
	/* it's not inventory, moved to other position */
	/* update wavelength1 */
	for(portno = PORT_ID_EAG6L_PORT1; portno < PORT_ID_EAG6L_MAX; portno++) {
		FPGA_PORT_WRITE(__PORT_WL1_ADDR[portno], INV_TBL[portno].wave);
	}

	/* update wavelength2 */
	for(portno = PORT_ID_EAG6L_PORT1; portno < PORT_ID_EAG6L_MAX; portno++) {
		/*FIXME : refer to Hangle document for encoding sfp idenification codes. */
		val = INV_TBL[portno].wave;
		gPortRegUpdate(__PORT_WL2_ADDR[portno], 0, 0x0FF, val);
		gPortRegUpdate(__PORT_WL2_ADDR[portno], 8, 0xF00, PORT_STATUS[portno].sfp_type);
	}
#endif
}

void process_alarm_info(void)
{
#if 1 /* [#161 Fixing for processing alarm flag, dustin, 2024-10-22 */
	static uint16_t PRE_ALM_FLAG[PORT_ID_EAG6L_MAX] = { 0, 0, 0, 0, 0, 0, 0, 0, };
	uint16_t bitmask, flagmask, ii;
#endif
#if 1 /* [#150] Implementing LR4 Status register, dustin, 2024-10-21 */
	static uint16_t PRE_QALM_FLAG = 0;
	uint16_t data;
#endif
	uint16_t val;
	uint16_t masking;
	uint8_t portno;

	/* update alarm */
	for(portno = PORT_ID_EAG6L_PORT1; portno < PORT_ID_EAG6L_MAX; portno++) {
		val = 0;

		/*FIXME : update LOS */
		if(PORT_STATUS[portno].los)
			val |= (1 << 0);
		else
			val &= ~(1 << 0);
#if 1 /* [#150] Implementing LR4 Status register, dustin, 2024-10-21 */
		if(portno == PORT_ID_EAG6L_PORT7) {
			if(LR4_STAT.rx_los_mask & 0xF)
				val |= (1 << 8);
		}
#endif

		/* update link down (Local Fault?) */
#if 1 /* [#88] Adding LF/RF reading and updating to Alarm, dustin, 2024-08-01 */
#if 1 /* [#148] Fixing for Link UP condition, dustin, 2024-10-14 */
		if((! (PORT_STATUS[portno].link && (! PORT_STATUS[portno].los))) || 
			   PORT_STATUS[portno].local_fault)
#else
		if(! PORT_STATUS[portno].link || PORT_STATUS[portno].local_fault)
#endif /* [#148] */
#else
		if(! PORT_STATUS[portno].link)
#endif
			val |= (1 << 1);
		else
			val &= ~(1 << 1);

#if 1/*[#120] LOC Alarm process ¿¿, balkrow, 2024-10-16 */
		/*FIXME : update LOC (ESMC Loss) */
		if(PORT_STATUS[portno].esmc_loss)
			val |= (1 << 2);
		else
			val &= ~(1 << 2);
#endif
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
#if 1 /* [#150] Implementing LR4 Status register, dustin, 2024-10-21 */
		if(portno == PORT_ID_EAG6L_PORT7) {
			if(LR4_STAT.tx_bias_mask & 0xF)
				val |= (1 << 8);
		}
#endif

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
#if 1 /* [#150] Implementing LR4 Status register, dustin, 2024-10-21 */
		masking = FPGA_PORT_READ(__PORT_ALM_MASK_ADDR[portno]);
#else
		masking = FPGA_READ(__PORT_ALM_MASK_ADDR[portno]);
#endif

#if 0/*[#61] Adding omitted functions, dustin, 2024-06-24 */
		/* update flex tune status */
		update_flex_tune_status(portno);
#endif

		/* remove unnecessary bits */
		masking &= 0x10F;

#if 1 /* [#156] Fixing for correct alarm masking, dustin, 2024-10-18 */
#if 1 /* [#161] Fixing for processing alarm flag, dustin, 2024-10-22 */
		/* NOTE : masking must be applied to both status and flag. */
		val &= ~masking;

		/* update alarm */
		gPortRegUpdate(__PORT_ALM_ADDR[portno], 0, 0xF0F, val);

		/* update alarm flag */
#if 1 /* [#150] Implementing LR4 Status register, dustin, 2024-10-21 */
		if(PRE_ALM_FLAG[portno] != val) {
			PRE_ALM_FLAG[portno] &= val;
			for(flagmask = 0, ii = 0; ii < 16; ii++) {
				bitmask = (1 < ii);

				/* get only changed from 0 to 1. */
				if((PRE_ALM_FLAG[portno] & bitmask == 0) && (val & bitmask == 1))
					flagmask |= bitmask;
			}
			data = flagmask | FPGA_PORT_READ(__PORT_ALM_FLAG_ADDR[portno]);
			FPGA_PORT_WRITE(__PORT_ALM_FLAG_ADDR[portno], (0x10F & data));
			PRE_ALM_FLAG[portno] |= (0x10F & flagmask);
		}
#else
		if(PRE_ALM_FLAG[portno] != val) {
			for(flagmask = 0, ii = 0; ii < 16; ii++) {
				bitmask = (1 < ii);

				/* get only changed from 0 to 1. */
				if((PRE_ALM_FLAG[portno] & bitmask == 0) && (val & bitmask == 1))
					flagmask |= bitmask;
			}
			gPortRegUpdate(__PORT_ALM_FLAG_ADDR[portno], 0, 0x10F, flagmask);
			PRE_ALM_FLAG[portno] = flagmask;
		}
#endif /* [#150] */
#else
		/* update alarm */
		gPortRegUpdate(__PORT_ALM_ADDR[portno], 0, 0xF0F, (val & ~masking));

		/* update alarm flag */
		gPortRegUpdate(__PORT_ALM_FLAG_ADDR[portno], 0, 0x10F, (val & ~masking));
#endif
#else
		/* update alarm */
		gPortRegUpdate(__PORT_ALM_ADDR[portno], 0, 0xF0F, val);

		/* update alarm flag */
		gPortRegUpdate(__PORT_ALM_FLAG_ADDR[portno], 0, 0x10F, val & masking);
#endif /* [#156] */
	}

#if 1 /* [#150] Implementing LR4 Status register, dustin, 2024-10-21 */
	portno = PORT_ID_EAG6L_PORT7;
	val = 0;
	if(PORT_STATUS[portno].equip) {
		/* update tx bias. */
		val |= (LR4_STAT.tx_bias_mask << 8) & 0xF00;

		/* update rx lol. */
		val |= (LR4_STAT.rx_lol_mask << 4) & 0xF0;

		/* update rx los. */
		val |= LR4_STAT.rx_los_mask & 0xF;

		/* read alarm mask register and mask off the result */
		masking = FPGA_PORT_READ(QSFP28_LR4_ALM_MASK_ADDR);
		/* remove unnecessary bits */
		masking &= 0xFFF;
		/* NOTE : masking must be applied to both status and flag. */
		val &= ~masking;

		/* update alarm */
		gPortRegUpdate(QSFP28_LR4_ALM_ADDR, 0, 0xFFF, val);

		/* update alarm flag */
		if(PRE_QALM_FLAG != val) {
			PRE_QALM_FLAG &= val;
			for(flagmask = 0, ii = 0; ii < 16; ii++) {
				bitmask = (1 < ii);

				/* get only changed from 0 to 1. */
				if(((PRE_QALM_FLAG & bitmask) == 0) && ((val & bitmask) == bitmask))
					flagmask |= bitmask;
			}
			data = flagmask | FPGA_PORT_READ(QSFP28_LR4_ALM_FLAG_ADDR);
			FPGA_PORT_WRITE(QSFP28_LR4_ALM_FLAG_ADDR, (0xFFF & data));
			PRE_QALM_FLAG |= (0xFFF & flagmask);
				
			gPortRegUpdate(QSFP28_LR4_ALM_FLAG_ADDR, 0, 0x10F, flagmask);
			PRE_QALM_FLAG = flagmask;
		}
	} else {
		/* clear alarm if not equipped. */
		FPGA_PORT_WRITE(QSFP28_LR4_ALM_ADDR, val);
		FPGA_PORT_WRITE(QSFP28_LR4_ALM_FLAG_ADDR, val);
	}
#endif /* [#150] */
	return;
}

void update_port_sfp_information(void)
{
extern int update_flex_tune_status(int portno);
	int portno;
	unsigned int val, type;
#if 1/*[#61] Adding omitted functions, dustin, 2024-06-24 */
	double fval;

	/* update alarm */
	process_alarm_info();

	for(portno = PORT_ID_EAG6L_PORT1; portno < PORT_ID_EAG6L_MAX; portno++) {
#if 1 /*[#82] eag6l board SW Debugging, balkrow, 2024-07-29*/
		if(!PORT_STATUS[portno].equip)
			continue;
#endif
#if 1 /* [#94] Adding for 100G DCO handling, dustin, 2024-09-23 */
		if(portno == (PORT_ID_EAG6L_MAX - 1)) {
#if 1 /* [#94] Adding for 100G DCO handling, dustin, 2024-09-23 */
			if(PORT_STATUS[portno].tunable_sfp) {
#endif /* [#94] */
#if 1 /* [#150] Implementing LR4 Status register, dustin, 2024-10-21 */
			val = (DCO_STAT.dco_IntL << 12) | 
			      (DCO_STAT.dco_DataNotReady << 8) |
			      (DCO_STAT.dco_TCReadyFlag << 4) | 
			       DCO_STAT.dco_InitComplete;
			FPGA_PORT_WRITE(QSFP28_STATUS1_ADDR, (uint16_t)val);
#else
			val = (DCO_STAT.dco_IntL << 12) | 
			      (DCO_STAT.dco_DataNotReady << 8) |
			      (DCO_STAT.dco_TCReadyFlag << 4) | 
			       DCO_STAT.dco_InitComplete;
			FPGA_WRITE(QSFP28_STATUS1_ADDR, (uint16_t)val);
#endif

#if 1 /* [#147] Fixing for 4th register update, dustin, 2024-10-21 */
			/* NOTE : fields changed. this is for LR4 only. */
			if(PORT_STATUS[portno].sfp_dco) {
				val = (DCO_STAT.dco_TxLosMask << 8) |
					(DCO_STAT.dco_TxLoLMask);
			} else {
				val = 0;
			}
#else
			val = (DCO_STAT.dco_TxLosMask << 12) |
			      (DCO_STAT.dco_RxLos << 8) |
			      (DCO_STAT.dco_TxLoLMask << 4) |
			       DCO_STAT.dco_RxLoL;
#endif /* [#147] */
#if 1 /* [#150] Implementing LR4 Status register, dustin, 2024-10-21 */
			FPGA_PORT_WRITE(QSFP28_STATUS2_ADDR, (uint16_t)val);
#else
			FPGA_WRITE(QSFP28_STATUS2_ADDR, (uint16_t)val);
#endif

			val = (DCO_STAT.dco_TempHA << 15) |
			      (DCO_STAT.dco_TempLA << 14) | 
			      (DCO_STAT.dco_TempLWA << 13) | 
			      (DCO_STAT.dco_TempLWA << 12) | 
			      (DCO_STAT.dco_OpticHA << 11) | 
			      (DCO_STAT.dco_OpticLA << 10) | 
			      (DCO_STAT.dco_OpticHWA << 9) | 
			      (DCO_STAT.dco_OpticLWA << 8);
#if 1 /* [#150] Implementing LR4 Status register, dustin, 2024-10-21 */
			FPGA_PORT_WRITE(QSFP28_STATUS3_ADDR, (uint16_t)val);
#else
			FPGA_WRITE(QSFP28_STATUS3_ADDR, (uint16_t)val);
#endif

#if 1 /* [#149] Implementing DCO BER/FER counters, dustin, 2024-10-21 */
#if 1 /* [#150] Implementing LR4 Status register, dustin, 2024-10-21 */
			/* update pre-fec ber. */
			FPGA_PORT_WRITE(QSFP28_PRE_FEC_BER1_ADDR, ((uint32_t)DCO_COUNT.ber_rate >> 8) & 0xFFFF);
			FPGA_PORT_WRITE(QSFP28_PRE_FEC_BER2_ADDR, ((uint32_t)DCO_COUNT.ber_rate & 0xFFFF));
			/* update fer. */
			FPGA_PORT_WRITE(QSFP28_FER1_ADDR, ((uint32_t)DCO_COUNT.fer_rate >> 8) & 0xFFFF);
			FPGA_PORT_WRITE(QSFP28_FER2_ADDR, ((uint32_t)DCO_COUNT.fer_rate & 0xFFFF));
#else
			/* update pre-fec ber. */
			FPGA_WRITE(QSFP28_PRE_FEC_BER1_ADDR, ((uint32_t)DCO_COUNT.ber_rate >> 8) & 0xFFFF);
			FPGA_WRITE(QSFP28_PRE_FEC_BER2_ADDR, ((uint32_t)DCO_COUNT.ber_rate & 0xFFFF));
			/* update fer. */
			FPGA_WRITE(QSFP28_FER1_ADDR, ((uint32_t)DCO_COUNT.fer_rate >> 8) & 0xFFFF);
			FPGA_WRITE(QSFP28_FER2_ADDR, ((uint32_t)DCO_COUNT.fer_rate & 0xFFFF));
#endif
#endif
#if 1 /* [#94] Adding for 100G DCO handling, dustin, 2024-09-23 */
			}
#endif /* [#94] */
#if 1 /* [#150] Implementing LR4 Status register, dustin, 2024-10-21 */
			else if(! PORT_STATUS[portno].sfp_dco && 
			          PORT_STATUS[portno].equip) { /* 100G LR4 case */
				/* update lr4 vcc. */
				val = convert_vcc_float_to_decimal(DCO_STAT.lr4_stat.vcc[0]);
				FPGA_PORT_WRITE(QSFP28_LR4_VOLT1_ADDR, val);
				val = convert_vcc_float_to_decimal(DCO_STAT.lr4_stat.vcc[1]);
				FPGA_PORT_WRITE(QSFP28_LR4_VOLT2_ADDR, val);
				val = convert_vcc_float_to_decimal(DCO_STAT.lr4_stat.vcc[2]);
				FPGA_PORT_WRITE(QSFP28_LR4_VOLT3_ADDR, val);
				val = convert_vcc_float_to_decimal(DCO_STAT.lr4_stat.vcc[3]);
				FPGA_PORT_WRITE(QSFP28_LR4_VOLT4_ADDR, val);
				/* update lr4 tx bias. */
				val = convert_temperature_float_to_decimal(DCO_STAT.lr4_stat.tx_bias[0]);
				FPGA_PORT_WRITE(QSFP28_LR4_TX_BIAS1_ADDR, val);
				val = convert_temperature_float_to_decimal(DCO_STAT.lr4_stat.tx_bias[1]);
				FPGA_PORT_WRITE(QSFP28_LR4_TX_BIAS2_ADDR, val);
				val = convert_temperature_float_to_decimal(DCO_STAT.lr4_stat.tx_bias[2]);
				FPGA_PORT_WRITE(QSFP28_LR4_TX_BIAS3_ADDR, val);
				val = convert_temperature_float_to_decimal(DCO_STAT.lr4_stat.tx_bias[3]);
				FPGA_PORT_WRITE(QSFP28_LR4_TX_BIAS4_ADDR, val);
				/* update lr4 tec current. */
				val = convert_dbm_float_to_decimal(DCO_STAT.lr4_stat.tec_curr[0], 
						0/*not-dbm*/, 0/*no-use*/);
				FPGA_PORT_WRITE(QSFP28_LR4_TCURR1_ADDR, val);
				val = convert_dbm_float_to_decimal(DCO_STAT.lr4_stat.tec_curr[1], 
						0/*not-dbm*/, 0/*no-use*/);
				FPGA_PORT_WRITE(QSFP28_LR4_TCURR2_ADDR, val);
				val = convert_dbm_float_to_decimal(DCO_STAT.lr4_stat.tec_curr[2], 
						0/*not-dbm*/, 0/*no-use*/);
				FPGA_PORT_WRITE(QSFP28_LR4_TCURR3_ADDR, val);
				val = convert_dbm_float_to_decimal(DCO_STAT.lr4_stat.tec_curr[3], 
						0/*not-dbm*/, 0/*no-use*/);
				FPGA_PORT_WRITE(QSFP28_LR4_TCURR4_ADDR, val);
				/* update lr4 tx power. */
				FPGA_PORT_WRITE(QSFP28_LR4_TX_POWER1_ADDR,
					PORT_STATUS[portno].equip ? 
					convert_dbm_float_to_decimal(DCO_STAT.lr4_stat.tx_pwr[0], 1/*dbm*/, 1/*tx*/)
						: 0x8999); 
				FPGA_PORT_WRITE(QSFP28_LR4_TX_POWER2_ADDR,
					PORT_STATUS[portno].equip ? 
					convert_dbm_float_to_decimal(DCO_STAT.lr4_stat.tx_pwr[0], 1/*dbm*/, 1/*tx*/)
						: 0x8999); 
				FPGA_PORT_WRITE(QSFP28_LR4_TX_POWER3_ADDR,
					PORT_STATUS[portno].equip ? 
					convert_dbm_float_to_decimal(DCO_STAT.lr4_stat.tx_pwr[2], 1/*dbm*/, 1/*tx*/)
						: 0x8999); 
				FPGA_PORT_WRITE(QSFP28_LR4_TX_POWER4_ADDR,
					PORT_STATUS[portno].equip ? 
					convert_dbm_float_to_decimal(DCO_STAT.lr4_stat.tx_pwr[3], 1/*dbm*/, 1/*tx*/)
						: 0x8999); 
				/* update lr4 rx power. */
				if(PORT_STATUS[portno].los) {
					FPGA_PORT_WRITE(QSFP28_LR4_TX_POWER1_ADDR, 0x8600);
					FPGA_PORT_WRITE(QSFP28_LR4_TX_POWER2_ADDR, 0x8600);
					FPGA_PORT_WRITE(QSFP28_LR4_TX_POWER3_ADDR, 0x8600);
					FPGA_PORT_WRITE(QSFP28_LR4_TX_POWER4_ADDR, 0x8600);
				} else  {
					FPGA_PORT_WRITE(QSFP28_LR4_TX_POWER1_ADDR,
						convert_dbm_float_to_decimal(
							DCO_STAT.lr4_stat.rx_pwr[0], 1/*dbm*/, 0/*rx*/));
					FPGA_PORT_WRITE(QSFP28_LR4_TX_POWER2_ADDR,
						convert_dbm_float_to_decimal(
							DCO_STAT.lr4_stat.rx_pwr[1], 1/*dbm*/, 0/*rx*/));
					FPGA_PORT_WRITE(QSFP28_LR4_TX_POWER3_ADDR,
						convert_dbm_float_to_decimal(
							DCO_STAT.lr4_stat.rx_pwr[2], 1/*dbm*/, 0/*rx*/));
					FPGA_PORT_WRITE(QSFP28_LR4_TX_POWER4_ADDR,
						convert_dbm_float_to_decimal(
							DCO_STAT.lr4_stat.rx_pwr[3], 1/*dbm*/, 0/*rx*/));
				}
				/* update wavelength1 for lane 2. */
				fval = DCO_STAT.lr4_stat.wavelength[0];
				fval = ceil(fval * 100) / 100;/* ceiling example : 1558.347 --> 1558.35 */
				FPGA_PORT_WRITE(QSFP28_LR4_WL1_2_ADDR,
					convert_hex_to_decimal((int)fval));
				/* update wavelength2 for lane 2. */
				fval = (fval - (int)fval) * 100;/* extract value below decimal point */
				FPGA_PORT_WRITE(QSFP28_LR4_WL2_2_ADDR, convert_hex_to_decimal((int)fval));

				/* update wavelength1 for lane 3. */
				fval = DCO_STAT.lr4_stat.wavelength[1];
				fval = ceil(fval * 100) / 100;/* ceiling example : 1558.347 --> 1558.35 */
				FPGA_PORT_WRITE(QSFP28_LR4_WL1_3_ADDR,
					convert_hex_to_decimal((int)fval));
				/* update wavelength2 for lane 3. */
				fval = (fval - (int)fval) * 100;/* extract value below decimal point */
				FPGA_PORT_WRITE(QSFP28_LR4_WL2_3_ADDR, convert_hex_to_decimal((int)fval));

				/* update wavelength1 for lane 4. */
				fval = DCO_STAT.lr4_stat.wavelength[2];
				fval = ceil(fval * 100) / 100;/* ceiling example : 1558.347 --> 1558.35 */
				FPGA_PORT_WRITE(QSFP28_LR4_WL1_4_ADDR,
					convert_hex_to_decimal((int)fval));
				/* update wavelength2 for lane 4. */
				fval = (fval - (int)fval) * 100;/* extract value below decimal point */
				FPGA_PORT_WRITE(QSFP28_LR4_WL2_4_ADDR, convert_hex_to_decimal((int)fval));
			}
#endif /* [#150] */
		}
#endif

#if 1 /* [#100] Adding update of Laser status by Laser_con, dustin, 2024-08-23 */
#if 1 /* [#125] Fixing for SFP channel no, wavelength, tx/rx dBm, dustin, 2024-09-10 */
		/* update tx power */
#if 1 /* [#150] Implementing LR4 Status register, dustin, 2024-10-21 */
		fval = PORT_STATUS[portno].tx_pwr;
		if(portno == PORT_ID_EAG6L_PORT7) {
			if(PORT_STATUS[portno].equip  && (! PORT_STATUS[portno].sfp_dco)) {
				fval =  (DCO_STAT.lr4_stat.tx_pwr[0] + DCO_STAT.lr4_stat.tx_pwr[1] +
				         DCO_STAT.lr4_stat.tx_pwr[2] + DCO_STAT.lr4_stat.tx_pwr[3]) / 4;
				fval += 10 * log10(4);
			}
		}
		FPGA_PORT_WRITE(__PORT_TX_PWR_ADDR[portno], 
			PORT_STATUS[portno].equip ? 
				convert_dbm_float_to_decimal(fval, 1/*dbm*/, 1/*tx*/)
				: 0x8999); 
#else /***************************************************************/
		FPGA_PORT_WRITE(__PORT_TX_PWR_ADDR[portno], 
			PORT_STATUS[portno].equip ? 
			convert_dbm_float_to_decimal(PORT_STATUS[portno].tx_pwr, 1/*dbm*/, 1/*tx*/)
			: 0x8999); 
#endif /* [#150] */
		/* update rx power */
#if 1 /* [#139] Fixing for updating Rx LoS, dustin, 2024-10-01 */
#if 1 /* [#150] Implementing LR4 Status register, dustin, 2024-10-21 */
		if(PORT_STATUS[portno].los) {
			FPGA_PORT_WRITE(__PORT_RX_PWR_ADDR[portno], 0x8600);
		} else  {
			fval = PORT_STATUS[portno].rx_pwr;
			if(portno == PORT_ID_EAG6L_PORT7) {
				if(PORT_STATUS[portno].equip  && (! PORT_STATUS[portno].sfp_dco)) {
					fval =  (DCO_STAT.lr4_stat.rx_pwr[0] + DCO_STAT.lr4_stat.rx_pwr[1] +
							DCO_STAT.lr4_stat.rx_pwr[2] + DCO_STAT.lr4_stat.rx_pwr[3]) / 4;
					fval += 10 * log10(4);
				}
			}
			FPGA_PORT_WRITE(__PORT_RX_PWR_ADDR[portno], 
				convert_dbm_float_to_decimal(fval, 1/*dbm*/, 0/*rx*/));
		}
#else /***************************************************************/
		if(PORT_STATUS[portno].los) {
			FPGA_PORT_WRITE(__PORT_RX_PWR_ADDR[portno], 0x8600);
		} else  {
			FPGA_PORT_WRITE(__PORT_RX_PWR_ADDR[portno], 
				convert_dbm_float_to_decimal(PORT_STATUS[portno].rx_pwr, 1/*dbm*/, 0/*rx*/));
		}
#endif /* [#150] */
#else /*******************************************************/
		FPGA_PORT_WRITE(__PORT_RX_PWR_ADDR[portno], 
			PORT_STATUS[portno].equip ? 
#if 1 /* [#139] Fixing for updating Rx LoS, dustin, 2024-10-01 */
			(PORT_STATUS[portno].los ? 
				0x8600/*LOS*/ : 
				convert_dbm_float_to_decimal(PORT_STATUS[portno].rx_pwr, 1/*dbm*/, 0/*rx*/))
#else /*******************************************************/
			convert_dbm_float_to_decimal(PORT_STATUS[portno].rx_pwr, 1/*dbm*/, 0/*rx*/)
#endif /* [#139] */
			: 0x8999);
#endif /* [#139] */
#else /*********************************************************************/
		/* update tx power */
		FPGA_PORT_WRITE(__PORT_TX_PWR_ADDR[portno], 
			PORT_STATUS[portno].equip ? 
			convert_dbm_float_to_decimal(PORT_STATUS[portno].tx_pwr, 1/*dbm*/)
			: 0x8999); 
		/* update rx power */
		FPGA_PORT_WRITE(__PORT_RX_PWR_ADDR[portno], 
			PORT_STATUS[portno].equip ? 
			convert_dbm_float_to_decimal(PORT_STATUS[portno].rx_pwr, 1/*dbm*/)
			: 0x89999);
#endif /* [#125] */
#else
		/* update tx power */
		FPGA_PORT_WRITE(__PORT_TX_PWR_ADDR[portno], 
			convert_dbm_float_to_decimal(PORT_STATUS[portno].tx_pwr, 1/*dbm*/));
		/* update rx power */
		FPGA_PORT_WRITE(__PORT_RX_PWR_ADDR[portno], 
			convert_dbm_float_to_decimal(PORT_STATUS[portno].rx_pwr, 1/*dbm*/));
#endif
		/* update temperature */
		val = convert_temperature_float_to_decimal(PORT_STATUS[portno].temp);
		FPGA_PORT_WRITE(__PORT_TEMP_ADDR[portno], val);
		/* update voltage */
#if 1 /* [#150] Implementing LR4 Status register, dustin, 2024-10-21 */
		fval = PORT_STATUS[portno].vcc;
		if(portno == PORT_ID_EAG6L_PORT7) {
			if(PORT_STATUS[portno].equip  && (! PORT_STATUS[portno].sfp_dco))
				fval =  (DCO_STAT.lr4_stat.vcc[0] + DCO_STAT.lr4_stat.vcc[1] +
				         DCO_STAT.lr4_stat.vcc[2] + DCO_STAT.lr4_stat.vcc[3]) / 4;
		}
		val = convert_vcc_float_to_decimal(fval);
#else
#if 1 /* [#125] Fixing for SFP channel no, wavelength, tx/rx dBm, dustin, 2024-09-10 */
		val = convert_vcc_float_to_decimal(PORT_STATUS[portno].vcc);
#else
		val = convert_dbm_float_to_decimal(PORT_STATUS[portno].vcc, 0/*not-dbm*/);
#endif /* [#125] */
#endif /* [#150] */
		FPGA_PORT_WRITE(__PORT_VOLT_ADDR[portno], val);
		/* update tx bias */
#if 1 /* [#150] Implementing LR4 Status register, dustin, 2024-10-21 */
		fval = PORT_STATUS[portno].tx_bias;
		if(portno == PORT_ID_EAG6L_PORT7) {
			if(PORT_STATUS[portno].equip && (! PORT_STATUS[portno].sfp_dco))
				fval = (DCO_STAT.lr4_stat.tx_bias[0] + DCO_STAT.lr4_stat.tx_bias[1] + 
					    DCO_STAT.lr4_stat.tx_bias[2] + DCO_STAT.lr4_stat.tx_bias[3]) / 4;
		}
		val = convert_temperature_float_to_decimal(fval);
#else
		val = convert_temperature_float_to_decimal(PORT_STATUS[portno].tx_bias);
#endif
		FPGA_PORT_WRITE(__PORT_TX_BIAS_ADDR[portno], val);
		/* update laser temperature */
#if 1 /* [#91] Fixing for register updating feature, dustin, 2024-08-05 */
		val = convert_temperature_float_to_decimal(PORT_STATUS[portno].laser_temp);
		FPGA_PORT_WRITE(__PORT_LTEMP_ADDR[portno], val);
		/* update TEC current */
#if 1 /* [#125] Fixing for SFP channel no, wavelength, tx/rx dBm, dustin, 2024-09-10 */
#if 1 /* [#150] Implementing LR4 Status register, dustin, 2024-10-21 */
		fval = PORT_STATUS[portno].tec_curr;
		if(portno == PORT_ID_EAG6L_PORT7) {
			if(PORT_STATUS[portno].equip && (! PORT_STATUS[portno].sfp_dco))
				fval = DCO_STAT.lr4_stat.tec_curr[0] + DCO_STAT.lr4_stat.tec_curr[1] + 
					   DCO_STAT.lr4_stat.tec_curr[2] + DCO_STAT.lr4_stat.tec_curr[3];
		}
		val = convert_dbm_float_to_decimal(fval, 0/*not-dbm*/, 0/*no-use*/);
#else /***************************************************************/
		val = convert_dbm_float_to_decimal(PORT_STATUS[portno].tec_curr, 0/*not-dbm*/, 0/*no-use*/);
#endif /* [#150] */
#else
		val = convert_dbm_float_to_decimal(PORT_STATUS[portno].tec_curr, 0/*not-dbm*/);
#endif /* [#125] */
		FPGA_PORT_WRITE(__PORT_TCURR_ADDR[portno], val);
#else
		val = convert_temperature_float_to_decimal(PORT_STATUS[portno].temp);
		FPGA_PORT_WRITE(__PORT_TEMP_ADDR[portno], val);
		/* update TEC current */
		val = convert_temperature_float_to_decimal(PORT_STATUS[portno].tec_curr);
		FPGA_PORT_WRITE(__PORT_TCURR_ADDR[portno], val);
#endif

		if(PORT_STATUS[portno].tunable_sfp) {
#if 1 /* [#94] Adding for 100G DCO handling, dustin, 2024-09-23 */
			if(portno < (PORT_ID_EAG6L_MAX - 1)) {
#endif /* [#94] */
#if 1/* [#72] Adding omitted rtWDM related registers, dustin, 2024-06-27 */
#if 1 /* [#100] Adding update of Laser status by Laser_con, dustin, 2024-08-23 */
#if 1 /* [#125] Fixing for SFP channel no, wavelength, tx/rx dBm, dustin, 2024-09-10 */
			/* update tx power */
			FPGA_PORT_WRITE(__PORT_TX_PWR_RTWDM_ADDR[portno], 
			PORT_STATUS[portno].equip ?
				convert_dbm_float_to_decimal(
				PORT_STATUS[portno].rtwdm_ddm_info.tx_pwr, 1/*dbm*/, 1/*tx*/)
				: 0x8999);
			/* update rx power */
			FPGA_PORT_WRITE(__PORT_RX_PWR_RTWDM_ADDR[portno], 
			PORT_STATUS[portno].equip ?
				convert_dbm_float_to_decimal(
				PORT_STATUS[portno].rtwdm_ddm_info.rx_pwr, 1/*dbm*/, 0/*rx*/)
				: 0x8999);
#else /*******************************************************************/
			/* update tx power */
			FPGA_PORT_WRITE(__PORT_TX_PWR_RTWDM_ADDR[portno], 
			PORT_STATUS[portno].equip ?
				convert_dbm_float_to_decimal(
				PORT_STATUS[portno].rtwdm_ddm_info.tx_pwr, 1/*dbm*/)
				: 0x8999);
			/* update rx power */
			FPGA_PORT_WRITE(__PORT_RX_PWR_RTWDM_ADDR[portno], 
			PORT_STATUS[portno].equip ?
				convert_dbm_float_to_decimal(
				PORT_STATUS[portno].rtwdm_ddm_info.rx_pwr, 1/*dbm*/)
				: 0x8999);
#endif /* [#125] */
#else
			/* update tx power */
			FPGA_PORT_WRITE(__PORT_TX_PWR_RTWDM_ADDR[portno], 
					convert_dbm_float_to_decimal(PORT_STATUS[portno].rtwdm_ddm_info.tx_pwr, 1/*dbm*/));
			/* update rx power */
			FPGA_PORT_WRITE(__PORT_RX_PWR_RTWDM_ADDR[portno], 
					convert_dbm_float_to_decimal(PORT_STATUS[portno].rtwdm_ddm_info.rx_pwr, 1/*dbm*/));
#endif
#if 1 /* [#157] Fixing for Smart T-SFP rtWDM info, dustin, 2024-10-18 */
			/* clear registers if port is los or rtwdm serial number is ready. */
			if((! PORT_STATUS[portno].link) || PORT_STATUS[portno].los ||
			  (RTWDM_INV_TBL[portno].serial_num[0] == 0x0/*NULL*/)) {
				/* update temperature */
				FPGA_PORT_WRITE(__PORT_TEMP_RTWDM_ADDR[portno], 0x0);
				/* update voltage */
				FPGA_PORT_WRITE(__PORT_VOLT_RTWDM_ADDR[portno], 0x0);
				/* update tx bias */
				FPGA_PORT_WRITE(__PORT_TX_BIAS_RTWDM_ADDR[portno], 0x0);
				/* update laser temperature */
				FPGA_PORT_WRITE(__PORT_LTEMP_RTWDM_ADDR[portno], 0x0);
				/* update TEC current */
				FPGA_PORT_WRITE(__PORT_TCURR_RTWDM_ADDR[portno], 0x0);
			} else {
				/* update temperature */
				val = convert_temperature_float_to_decimal(PORT_STATUS[portno].rtwdm_ddm_info.temp);
				FPGA_PORT_WRITE(__PORT_TEMP_RTWDM_ADDR[portno], val);
				/* update voltage */
				val = convert_vcc_float_to_decimal(PORT_STATUS[portno].rtwdm_ddm_info.vcc);
				FPGA_PORT_WRITE(__PORT_VOLT_RTWDM_ADDR[portno], val);
				/* update tx bias */
				val = convert_temperature_float_to_decimal(PORT_STATUS[portno].rtwdm_ddm_info.tx_bias);
				FPGA_PORT_WRITE(__PORT_TX_BIAS_RTWDM_ADDR[portno], val);
				/* update laser temperature */
				val = convert_temperature_float_to_decimal(PORT_STATUS[portno].rtwdm_ddm_info.laser_temp);
				FPGA_PORT_WRITE(__PORT_LTEMP_RTWDM_ADDR[portno], val);
				/* update TEC current */
				val = convert_dbm_float_to_decimal(PORT_STATUS[portno].rtwdm_ddm_info.tec_curr, 0/*not-dbm*/, 0/*no-use*/);
				FPGA_PORT_WRITE(__PORT_TCURR_RTWDM_ADDR[portno], val);
			}
#else /****************************************************************/
			/* update temperature */
			val = convert_temperature_float_to_decimal(PORT_STATUS[portno].rtwdm_ddm_info.temp);
			FPGA_PORT_WRITE(__PORT_TEMP_RTWDM_ADDR[portno], val);
			/* update voltage */
#if 1 /* [#125] Fixing for SFP channel no, wavelength, tx/rx dBm, dustin, 2024-09-10 */
			val = convert_vcc_float_to_decimal(PORT_STATUS[portno].rtwdm_ddm_info.vcc);
#else
			val = convert_dbm_float_to_decimal(PORT_STATUS[portno].rtwdm_ddm_info.vcc, 0/*not-dbm*/);
#endif /* [#125] */
			FPGA_PORT_WRITE(__PORT_VOLT_RTWDM_ADDR[portno], val);
			/* update tx bias */
			val = convert_temperature_float_to_decimal(PORT_STATUS[portno].rtwdm_ddm_info.tx_bias);
			FPGA_PORT_WRITE(__PORT_TX_BIAS_RTWDM_ADDR[portno], val);
			/* update laser temperature */
#if 1 /* [#91] Fixing for register updating feature, dustin, 2024-08-05 */
			val = convert_temperature_float_to_decimal(PORT_STATUS[portno].rtwdm_ddm_info.laser_temp);
			FPGA_PORT_WRITE(__PORT_LTEMP_RTWDM_ADDR[portno], val);
			/* update TEC current */
#if 1 /* [#125] Fixing for SFP channel no, wavelength, tx/rx dBm, dustin, 2024-09-10 */
			val = convert_dbm_float_to_decimal(PORT_STATUS[portno].rtwdm_ddm_info.tec_curr, 0/*not-dbm*/, 0/*no-use*/);
#else
			val = convert_dbm_float_to_decimal(PORT_STATUS[portno].rtwdm_ddm_info.tec_curr, 0/*not-dbm*/);
#endif /* [#125] */
			FPGA_PORT_WRITE(__PORT_TCURR_RTWDM_ADDR[portno], val);
#else
			val = convert_temperature_float_to_decimal(PORT_STATUS[portno].rtwdm_ddm_info.laser_temp);
			FPGA_PORT_WRITE(__PORT_TEMP_RTWDM_ADDR[portno], val);
			/* update TEC current */
			val = convert_temperature_float_to_decimal(PORT_STATUS[portno].rtwdm_ddm_info.tec_curr);
			FPGA_PORT_WRITE(__PORT_TCURR_RTWDM_ADDR[portno], val);
#endif
#endif /* [#157] */
#endif
#if 1 /* [#94] Adding for 100G DCO handling, dustin, 2024-09-23 */
			}
#endif /* [#94] */

			/* update wavelength1/2 */
			fval = PORT_STATUS[portno].tunable_wavelength;
			fval = ceil(fval * 100) / 100;/* ceiling example : 1558.347 --> 1558.35 */
#if 1 /* [#107] Fixing for 2nd register updates, dustin, 2024-08-29 */
			FPGA_PORT_WRITE(__PORT_WL1_ADDR[portno], 
				convert_hex_to_decimal((int)fval));
#else
			FPGA_PORT_WRITE(__PORT_WL1_ADDR[portno], (int)fval);
#endif

			fval = (fval - (int)fval) * 100;/* extract value below decimal point */
			type = PORT_STATUS[portno].sfp_type;
#if 1 /* [#107] Fixing for 2nd register updates, dustin, 2024-08-29 */
			type = (type << 8) | convert_hex_to_decimal((int)fval);
			FPGA_PORT_WRITE(__PORT_WL2_ADDR[portno], type);
#else
			gPortRegUpdate(__PORT_WL2_ADDR[portno], 0, 0x0FF, (int)fval);
			gPortRegUpdate(__PORT_WL2_ADDR[portno], 8, 0xF00, type);
#endif

#if 0 /* [#153] Fixing for updating flex tune, dustin, 2024-10-17 */
#if 1 /* [#94] Adding for 100G DCO handling, dustin, 2024-09-23 */
			if(portno < (PORT_ID_EAG6L_MAX - 1)) {
				/* update sfp channel no. */
				update_sfp_channel_no(portno);
				continue;
			}
#endif /* [#94] */
#endif /* [#153] */

#if 1/* [#72] Adding omitted rtWDM related registers, dustin, 2024-06-27 */
#if 1 /* [#153] Fixing for updating flex tune, dustin, 2024-10-17 */
			if(PORT_STATUS[portno].tunable_sfp &&
			  (portno < (PORT_ID_EAG6L_MAX - 1)))
#else
			if(PORT_STATUS[portno].tunable_sfp)
#endif /* [#153] */
			{
				/* update wavelength1/2 */
				fval = PORT_STATUS[portno].tunable_rtwdm_wavelength;
				/* ceiling example : 1558.347 --> 1558.35 */
				fval = ceil(fval * 100) / 100;
#if 1 /* [#107] Fixing for 2nd register updates, dustin, 2024-08-29 */
				FPGA_PORT_WRITE(__PORT_WL1_RTWDM_ADDR[portno], 
						convert_hex_to_decimal((int)fval));
#else
				FPGA_PORT_WRITE(__PORT_WL1_RTWDM_ADDR[portno], (int)fval);
#endif

				/* extract value below decimal point */
				fval = (fval - (int)fval) * 100;
				type = PORT_STATUS[portno].tunable_rtwdm_sfp_type;
#if 1/* [#72] Adding omitted rtWDM related registers, dustin, 2024-06-27 */
#if 1 /* [#107] Fixing for 2nd register updates, dustin, 2024-08-29 */
				type = (type << 8) | convert_hex_to_decimal((int)fval);
				FPGA_PORT_WRITE(__PORT_WL2_RTWDM_ADDR[portno], type);
#else
				gPortRegUpdate(__PORT_WL2_RTWDM_ADDR[portno], 0, 0x0FF, (int)fval);
				gPortRegUpdate(__PORT_WL2_RTWDM_ADDR[portno], 8, 0xF00, type);
#endif
#else
				gPortRegUpdate(__PORT_WL2_RTWDM_ADDR[portno], 0, 0x0FF, (int)fval);
				gPortRegUpdate(__PORT_WL2_RTWDM_ADDR[portno], 8, 0xF00, type);
#endif
			}
#endif

			/* update sfp channel no. */
			update_sfp_channel_no(portno);

			/* update flex tune status */
			update_flex_tune_status(portno);
		} else {
			/* update wavelength1/2 */
#if 1 /* [#107] Fixing for 2nd register updates, dustin, 2024-08-29 */
			FPGA_PORT_WRITE(__PORT_WL1_ADDR[portno], 
				convert_hex_to_decimal(INV_TBL[portno].wave));
			type = (PORT_STATUS[portno].sfp_type << 8);
#if 1 /* [#125] Fixing for SFP channel no, wavelength, tx/rx dBm, dustin, 2024-09-10 */
			/* NOTE : hfr sfp providers have decimal value in reserved field. */
			type |= convert_hex_to_decimal(INV_TBL[portno].wave_decimal);
#endif
			FPGA_PORT_WRITE(__PORT_WL2_ADDR[portno], type);
#else
			FPGA_PORT_WRITE(__PORT_WL1_ADDR[portno], INV_TBL[portno].wave);

			gPortRegUpdate(__PORT_WL2_ADDR[portno], 0, 0x0FF, 0x0/*default*/);
			gPortRegUpdate(__PORT_WL2_ADDR[portno], 8, 0xF00, PORT_STATUS[portno].sfp_type);
#endif
		}
	}

#if 1 /* [#157] Fixing for Smart T-SFP rtWDM info, dustin, 2024-10-18 */
	update_port_sfp_inventory(1/*update-rtwdm*/);
#endif /* [#157] */
#else /****************************************************************/
	/* update tx power */
	for(portno = PORT_ID_EAG6L_PORT1; portno < PORT_ID_EAG6L_MAX; portno++) {
		FPGA_PORT_WRITE(__PORT_TX_PWR_ADDR[portno], 
			convert_dbm_float_to_decimal(PORT_STATUS[portno].tx_pwr, 1/*dbm*/));
	}

	/* update rx power */
	for(portno = PORT_ID_EAG6L_PORT1; portno < PORT_ID_EAG6L_MAX; portno++) {
		FPGA_PORT_WRITE(__PORT_RX_PWR_ADDR[portno], 
			convert_dbm_float_to_decimal(PORT_STATUS[portno].rx_pwr, 1/*dbm*/));
	}

	/* update alarm */
	process_alarm_info();

	/* update temperature */
	for(portno = PORT_ID_EAG6L_PORT1; portno < PORT_ID_EAG6L_MAX; portno++) {
		val = convert_temperature_float_to_decimal(PORT_STATUS[portno].temp);
		FPGA_PORT_WRITE(__PORT_TEMP_ADDR[portno], val);
	}

	/* update voltage */
	for(portno = PORT_ID_EAG6L_PORT1; portno < PORT_ID_EAG6L_MAX; portno++) {
		val = convert_dbm_float_to_decimal(PORT_STATUS[portno].vcc, 0/*not-dbm*/);
		FPGA_PORT_WRITE(__PORT_VOLT_ADDR[portno], val);
	}

	/* update tx bias */
	for(portno = PORT_ID_EAG6L_PORT1; portno < PORT_ID_EAG6L_MAX; portno++) {
		val = convert_temperature_float_to_decimal(PORT_STATUS[portno].tx_bias);
		FPGA_PORT_WRITE(__PORT_TX_BIAS_ADDR[portno], val);
	}

	/* update laser temperature */
	for(portno = PORT_ID_EAG6L_PORT1; portno < PORT_ID_EAG6L_MAX; portno++) {
		val = convert_temperature_float_to_decimal(PORT_STATUS[portno].temp);
		FPGA_PORT_WRITE(__PORT_TEMP_ADDR[portno], val);
	}

	/* update TEC current */
	for(portno = PORT_ID_EAG6L_PORT1; portno < PORT_ID_EAG6L_MAX; portno++) {
		val = convert_temperature_float_to_decimal(PORT_STATUS[portno].tec_curr);
		FPGA_PORT_WRITE(__PORT_TCURR_ADDR[portno], val);
	}
#endif

	return;
}

void process_port_pm_counters(void)
{
	u16 portno;
	u16 val;

#if 0/*[#51] Adding register callback templates for config/command registers, dustin, 2024-06-12 */
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
#endif

    /* update pm tx byte */
    for(portno = PORT_ID_EAG6L_PORT1; portno < PORT_ID_EAG6L_MAX; portno++) {
#if 1 /* [#133] Fixing for PM address changes, dustin, 2024-09-25 */
#if 1 /* [#140] Fixing for PM counters, dustin, 2024-10-02 */
        val = (PM_TBL[portno].tx_byte >> 48) & 0xFFFF;
        FPGA_PORT_WRITE(__PORT_PM_TX_BYTE4_ADDR[portno], val);
        val = (PM_TBL[portno].tx_byte >> 32) & 0xFFFF;
        FPGA_PORT_WRITE(__PORT_PM_TX_BYTE3_ADDR[portno], val);
        val = (PM_TBL[portno].tx_byte >> 16) & 0xFFFF;
        FPGA_PORT_WRITE(__PORT_PM_TX_BYTE2_ADDR[portno], val);
        val = (PM_TBL[portno].tx_byte >>  0) & 0xFFFF;
        FPGA_PORT_WRITE(__PORT_PM_TX_BYTE1_ADDR[portno], val);
#else
        val = (PM_TBL[portno].tx_byte >> 48) & 0xFFFF;
        FPGA_PORT_WRITE(__PORT_PM_TX_BYTE2_ADDR[portno], val);
        val = (PM_TBL[portno].tx_byte >> 32) & 0xFFFF;
        FPGA_PORT_WRITE(__PORT_PM_TX_BYTE1_ADDR[portno], val);
        val = (PM_TBL[portno].tx_byte >> 16) & 0xFFFF;
        FPGA_PORT_WRITE(__PORT_PM_TX_BYTE4_ADDR[portno], val);
        val = (PM_TBL[portno].tx_byte >>  0) & 0xFFFF;
        FPGA_PORT_WRITE(__PORT_PM_TX_BYTE3_ADDR[portno], val);
#endif
#else
        val = (PM_TBL[portno].tx_byte >> 48) & 0xFFFF;
        FPGA_PORT_WRITE(__PORT_PM_TX_BYTE1_ADDR[portno], val);
        val = (PM_TBL[portno].tx_byte >> 32) & 0xFFFF;
        FPGA_PORT_WRITE(__PORT_PM_TX_BYTE2_ADDR[portno], val);
        val = (PM_TBL[portno].tx_byte >> 16) & 0xFFFF;
        FPGA_PORT_WRITE(__PORT_PM_TX_BYTE3_ADDR[portno], val);
        val = (PM_TBL[portno].tx_byte >>  0) & 0xFFFF;
        FPGA_PORT_WRITE(__PORT_PM_TX_BYTE4_ADDR[portno], val);
#endif
    }

    /* update pm rx byte */
    for(portno = PORT_ID_EAG6L_PORT1; portno < PORT_ID_EAG6L_MAX; portno++) {
#if 1 /* [#133] Fixing for PM address changes, dustin, 2024-09-25 */
#if 1 /* [#140] Fixing for PM counters, dustin, 2024-10-02 */
        val = (PM_TBL[portno].rx_byte >> 48) & 0xFFFF;
        FPGA_PORT_WRITE(__PORT_PM_RX_BYTE4_ADDR[portno], val);
        val = (PM_TBL[portno].rx_byte >> 32) & 0xFFFF;
        FPGA_PORT_WRITE(__PORT_PM_RX_BYTE3_ADDR[portno], val);
        val = (PM_TBL[portno].rx_byte >> 16) & 0xFFFF;
        FPGA_PORT_WRITE(__PORT_PM_RX_BYTE2_ADDR[portno], val);
        val = (PM_TBL[portno].rx_byte >>  0) & 0xFFFF;
        FPGA_PORT_WRITE(__PORT_PM_RX_BYTE1_ADDR[portno], val);
#else
        val = (PM_TBL[portno].rx_byte >> 48) & 0xFFFF;
        FPGA_PORT_WRITE(__PORT_PM_RX_BYTE2_ADDR[portno], val);
        val = (PM_TBL[portno].rx_byte >> 32) & 0xFFFF;
        FPGA_PORT_WRITE(__PORT_PM_RX_BYTE1_ADDR[portno], val);
        val = (PM_TBL[portno].rx_byte >> 16) & 0xFFFF;
        FPGA_PORT_WRITE(__PORT_PM_RX_BYTE4_ADDR[portno], val);
        val = (PM_TBL[portno].rx_byte >>  0) & 0xFFFF;
        FPGA_PORT_WRITE(__PORT_PM_RX_BYTE3_ADDR[portno], val);
#endif /* [#140] */
#else
        val = (PM_TBL[portno].rx_byte >> 48) & 0xFFFF;
        FPGA_PORT_WRITE(__PORT_PM_RX_BYTE1_ADDR[portno], val);
        val = (PM_TBL[portno].rx_byte >> 32) & 0xFFFF;
        FPGA_PORT_WRITE(__PORT_PM_RX_BYTE2_ADDR[portno], val);
        val = (PM_TBL[portno].rx_byte >> 16) & 0xFFFF;
        FPGA_PORT_WRITE(__PORT_PM_RX_BYTE3_ADDR[portno], val);
        val = (PM_TBL[portno].rx_byte >>  0) & 0xFFFF;
        FPGA_PORT_WRITE(__PORT_PM_RX_BYTE4_ADDR[portno], val);
#endif
    }

    /* update pm tx frame */
    for(portno = PORT_ID_EAG6L_PORT1; portno < PORT_ID_EAG6L_MAX; portno++) {
#if 1 /* [#133] Fixing for PM address changes, dustin, 2024-09-25 */
#if 1 /* [#140] Fixing for PM counters, dustin, 2024-10-02 */
        val = (PM_TBL[portno].tx_frame >> 48) & 0xFFFF;
        FPGA_PORT_WRITE(__PORT_PM_TX_FRAME4_ADDR[portno], val);
        val = (PM_TBL[portno].tx_frame >> 32) & 0xFFFF;
        FPGA_PORT_WRITE(__PORT_PM_TX_FRAME3_ADDR[portno], val);
        val = (PM_TBL[portno].tx_frame >> 16) & 0xFFFF;
        FPGA_PORT_WRITE(__PORT_PM_TX_FRAME2_ADDR[portno], val);
        val = (PM_TBL[portno].tx_frame >>  0) & 0xFFFF;
        FPGA_PORT_WRITE(__PORT_PM_TX_FRAME1_ADDR[portno], val);
#else
        val = (PM_TBL[portno].tx_frame >> 48) & 0xFFFF;
        FPGA_PORT_WRITE(__PORT_PM_TX_FRAME2_ADDR[portno], val);
        val = (PM_TBL[portno].tx_frame >> 32) & 0xFFFF;
        FPGA_PORT_WRITE(__PORT_PM_TX_FRAME1_ADDR[portno], val);
        val = (PM_TBL[portno].tx_frame >> 16) & 0xFFFF;
        FPGA_PORT_WRITE(__PORT_PM_TX_FRAME4_ADDR[portno], val);
        val = (PM_TBL[portno].tx_frame >>  0) & 0xFFFF;
        FPGA_PORT_WRITE(__PORT_PM_TX_FRAME3_ADDR[portno], val);
#endif /* [#140] */
#else
        val = (PM_TBL[portno].tx_frame >> 48) & 0xFFFF;
        FPGA_PORT_WRITE(__PORT_PM_TX_FRAME1_ADDR[portno], val);
        val = (PM_TBL[portno].tx_frame >> 32) & 0xFFFF;
        FPGA_PORT_WRITE(__PORT_PM_TX_FRAME2_ADDR[portno], val);
        val = (PM_TBL[portno].tx_frame >> 16) & 0xFFFF;
        FPGA_PORT_WRITE(__PORT_PM_TX_FRAME3_ADDR[portno], val);
        val = (PM_TBL[portno].tx_frame >>  0) & 0xFFFF;
        FPGA_PORT_WRITE(__PORT_PM_TX_FRAME4_ADDR[portno], val);
#endif
    }

    /* update pm rx frame */
    for(portno = PORT_ID_EAG6L_PORT1; portno < PORT_ID_EAG6L_MAX; portno++) {
#if 1 /* [#133] Fixing for PM address changes, dustin, 2024-09-25 */
#if 1 /* [#140] Fixing for PM counters, dustin, 2024-10-02 */
        val = (PM_TBL[portno].rx_frame >> 48) & 0xFFFF;
        FPGA_PORT_WRITE(__PORT_PM_RX_FRAME4_ADDR[portno], val);
        val = (PM_TBL[portno].rx_frame >> 32) & 0xFFFF;
        FPGA_PORT_WRITE(__PORT_PM_RX_FRAME3_ADDR[portno], val);
        val = (PM_TBL[portno].rx_frame >> 16) & 0xFFFF;
        FPGA_PORT_WRITE(__PORT_PM_RX_FRAME2_ADDR[portno], val);
        val = (PM_TBL[portno].rx_frame >>  0) & 0xFFFF;
        FPGA_PORT_WRITE(__PORT_PM_RX_FRAME1_ADDR[portno], val);
#else
        val = (PM_TBL[portno].rx_frame >> 48) & 0xFFFF;
        FPGA_PORT_WRITE(__PORT_PM_RX_FRAME2_ADDR[portno], val);
        val = (PM_TBL[portno].rx_frame >> 32) & 0xFFFF;
        FPGA_PORT_WRITE(__PORT_PM_RX_FRAME1_ADDR[portno], val);
        val = (PM_TBL[portno].rx_frame >> 16) & 0xFFFF;
        FPGA_PORT_WRITE(__PORT_PM_RX_FRAME4_ADDR[portno], val);
        val = (PM_TBL[portno].rx_frame >>  0) & 0xFFFF;
        FPGA_PORT_WRITE(__PORT_PM_RX_FRAME3_ADDR[portno], val);
#endif /* [#140] */
#else
        val = (PM_TBL[portno].rx_frame >> 48) & 0xFFFF;
        FPGA_PORT_WRITE(__PORT_PM_RX_FRAME1_ADDR[portno], val);
        val = (PM_TBL[portno].rx_frame >> 32) & 0xFFFF;
        FPGA_PORT_WRITE(__PORT_PM_RX_FRAME2_ADDR[portno], val);
        val = (PM_TBL[portno].rx_frame >> 16) & 0xFFFF;
        FPGA_PORT_WRITE(__PORT_PM_RX_FRAME3_ADDR[portno], val);
        val = (PM_TBL[portno].rx_frame >>  0) & 0xFFFF;
        FPGA_PORT_WRITE(__PORT_PM_RX_FRAME4_ADDR[portno], val);
#endif
    }

    /* update pm fcs */
    for(portno = PORT_ID_EAG6L_PORT1; portno < PORT_ID_EAG6L_MAX; portno++) {
#if 1 /* [#133] Fixing for PM address changes, dustin, 2024-09-25 */
#if 1 /* [#140] Fixing for PM counters, dustin, 2024-10-02 */
        val = (PM_TBL[portno].rx_fcs >> 48) & 0xFFFF;
        FPGA_PORT_WRITE(__PORT_PM_FCS4_ADDR[portno], val);
        val = (PM_TBL[portno].rx_fcs >> 32) & 0xFFFF;
        FPGA_PORT_WRITE(__PORT_PM_FCS3_ADDR[portno], val);
        val = (PM_TBL[portno].rx_fcs >> 16) & 0xFFFF;
        FPGA_PORT_WRITE(__PORT_PM_FCS2_ADDR[portno], val);
        val = (PM_TBL[portno].rx_fcs >>  0) & 0xFFFF;
        FPGA_PORT_WRITE(__PORT_PM_FCS1_ADDR[portno], val);
#else
        val = (PM_TBL[portno].rx_fcs >> 48) & 0xFFFF;
        FPGA_PORT_WRITE(__PORT_PM_FCS2_ADDR[portno], val);
        val = (PM_TBL[portno].rx_fcs >> 32) & 0xFFFF;
        FPGA_PORT_WRITE(__PORT_PM_FCS1_ADDR[portno], val);
        val = (PM_TBL[portno].rx_fcs >> 16) & 0xFFFF;
        FPGA_PORT_WRITE(__PORT_PM_FCS4_ADDR[portno], val);
        val = (PM_TBL[portno].rx_fcs >>  0) & 0xFFFF;
        FPGA_PORT_WRITE(__PORT_PM_FCS3_ADDR[portno], val);
#endif /* [#140] */
#else
        val = (PM_TBL[portno].rx_fcs >> 48) & 0xFFFF;
        FPGA_PORT_WRITE(__PORT_PM_FCS1_ADDR[portno], val);
        val = (PM_TBL[portno].rx_fcs >> 32) & 0xFFFF;
        FPGA_PORT_WRITE(__PORT_PM_FCS2_ADDR[portno], val);
        val = (PM_TBL[portno].rx_fcs >> 16) & 0xFFFF;
        FPGA_PORT_WRITE(__PORT_PM_FCS3_ADDR[portno], val);
        val = (PM_TBL[portno].rx_fcs >>  0) & 0xFFFF;
        FPGA_PORT_WRITE(__PORT_PM_FCS4_ADDR[portno], val);
#endif
    }

    /* update pm fcs corrected error */
    for(portno = PORT_ID_EAG6L_PORT1; portno < PORT_ID_EAG6L_MAX; portno++) {
#if 1 /* [#133] Fixing for PM address changes, dustin, 2024-09-25 */
#if 1 /* [#140] Fixing for PM counters, dustin, 2024-10-02 */
#if 0 /* [#147] Fixing for 4th register update, dustin, 2024-10-21 */
/* NOTE : removed by requst of cwhan. */
        val = (PM_TBL[portno].fcs_ok >> 48) & 0xFFFF;
        FPGA_PORT_WRITE(__PORT_PM_FCS_OK4_ADDR[portno], val);
        val = (PM_TBL[portno].fcs_ok >> 32) & 0xFFFF;
        FPGA_PORT_WRITE(__PORT_PM_FCS_OK3_ADDR[portno], val);
#endif /* [#147] */
        val = (PM_TBL[portno].fcs_ok >> 16) & 0xFFFF;
        FPGA_PORT_WRITE(__PORT_PM_FCS_OK2_ADDR[portno], val);
        val = (PM_TBL[portno].fcs_ok >>  0) & 0xFFFF;
        FPGA_PORT_WRITE(__PORT_PM_FCS_OK1_ADDR[portno], val);
#else
        val = (PM_TBL[portno].fcs_ok >> 48) & 0xFFFF;
        FPGA_PORT_WRITE(__PORT_PM_FCS_OK2_ADDR[portno], val);
        val = (PM_TBL[portno].fcs_ok >> 32) & 0xFFFF;
        FPGA_PORT_WRITE(__PORT_PM_FCS_OK1_ADDR[portno], val);
        val = (PM_TBL[portno].fcs_ok >> 16) & 0xFFFF;
        FPGA_PORT_WRITE(__PORT_PM_FCS_OK4_ADDR[portno], val);
        val = (PM_TBL[portno].fcs_ok >>  0) & 0xFFFF;
        FPGA_PORT_WRITE(__PORT_PM_FCS_OK3_ADDR[portno], val);
#endif /* [#140] */
#else
        val = (PM_TBL[portno].fcs_ok >> 48) & 0xFFFF;
        FPGA_PORT_WRITE(__PORT_PM_FCS_OK1_ADDR[portno], val);
        val = (PM_TBL[portno].fcs_ok >> 32) & 0xFFFF;
        FPGA_PORT_WRITE(__PORT_PM_FCS_OK2_ADDR[portno], val);
        val = (PM_TBL[portno].fcs_ok >> 16) & 0xFFFF;
        FPGA_PORT_WRITE(__PORT_PM_FCS_OK3_ADDR[portno], val);
        val = (PM_TBL[portno].fcs_ok >>  0) & 0xFFFF;
        FPGA_PORT_WRITE(__PORT_PM_FCS_OK4_ADDR[portno], val);
#endif
    }

    /* update pm fcs uncorrected error */
    for(portno = PORT_ID_EAG6L_PORT1; portno < PORT_ID_EAG6L_MAX; portno++) {
#if 1 /* [#133] Fixing for PM address changes, dustin, 2024-09-25 */
#if 1 /* [#140] Fixing for PM counters, dustin, 2024-10-02 */
#if 0 /* [#147] Fixing for 4th register update, dustin, 2024-10-21 */
/* NOTE : removed by requst of cwhan. */
        val = (PM_TBL[portno].fcs_nok >> 48) & 0xFFFF;
        FPGA_PORT_WRITE(__PORT_PM_FCS_NOK4_ADDR[portno], val);
        val = (PM_TBL[portno].fcs_nok >> 32) & 0xFFFF;
        FPGA_PORT_WRITE(__PORT_PM_FCS_NOK3_ADDR[portno], val);
#endif /* [#140] */
        val = (PM_TBL[portno].fcs_nok >> 16) & 0xFFFF;
        FPGA_PORT_WRITE(__PORT_PM_FCS_NOK2_ADDR[portno], val);
        val = (PM_TBL[portno].fcs_nok >>  0) & 0xFFFF;
        FPGA_PORT_WRITE(__PORT_PM_FCS_NOK1_ADDR[portno], val);
#else
        val = (PM_TBL[portno].fcs_nok >> 48) & 0xFFFF;
        FPGA_PORT_WRITE(__PORT_PM_FCS_NOK2_ADDR[portno], val);
        val = (PM_TBL[portno].fcs_nok >> 32) & 0xFFFF;
        FPGA_PORT_WRITE(__PORT_PM_FCS_NOK1_ADDR[portno], val);
        val = (PM_TBL[portno].fcs_nok >> 16) & 0xFFFF;
        FPGA_PORT_WRITE(__PORT_PM_FCS_NOK4_ADDR[portno], val);
        val = (PM_TBL[portno].fcs_nok >>  0) & 0xFFFF;
        FPGA_PORT_WRITE(__PORT_PM_FCS_NOK3_ADDR[portno], val);
#endif /* [#140] */
#else
        val = (PM_TBL[portno].fcs_nok >> 48) & 0xFFFF;
        FPGA_PORT_WRITE(__PORT_PM_FCS_NOK1_ADDR[portno], val);
        val = (PM_TBL[portno].fcs_nok >> 48) & 0xFFFF;
        FPGA_PORT_WRITE(__PORT_PM_FCS_NOK2_ADDR[portno], val);
        val = (PM_TBL[portno].fcs_nok >> 48) & 0xFFFF;
        FPGA_PORT_WRITE(__PORT_PM_FCS_NOK3_ADDR[portno], val);
        val = (PM_TBL[portno].fcs_nok >> 48) & 0xFFFF;
        FPGA_PORT_WRITE(__PORT_PM_FCS_NOK4_ADDR[portno], val);
#endif
    }

	return;
}

#if 1/*[#53] Clock source status ¿¿¿¿ ¿¿ ¿¿, balkrow, 2024-06-13*/
void update_KeepAlive(void)
{
#if 0 /* [#97] Adding register recovery process after fpga reset, dustin, 2024-08-21 */
	/* NOTE : this caused counting in 1 byte(0~255) range. */
	if(gDB.keepAlive == 0xff)
		gDB.keepAlive = 1;
#endif
	/* [#62] SFP eeprom 및 register update 기능 단위 검증 및 디버깅, balkrow, 2024-06-21 
	 *
	 * WRITE KEEP ALIVE(0x16)
	 * */
#if 1/*[#61] Adding omitted functions, dustin, 2024-06-24 */
	FPGA_WRITE(HW_KEEP_ALIVE_2_ADDR,  gDB.keepAlive++);
#else
	FPGA_WRITE(HW_KEEP_ALIVE_1_ADDR,  gDB.keepAlive++);
#endif
}
#endif

#if 0
void update_bp_reg(void)
{
	u16 ii, temp, pri_port, sec_port, bitmask;
	u16 val[20];

#ifdef DEBUG
	zlog_notice(" update_bp_reg");/*ZZPP*/
#endif
#if 0//by balkrow  
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

#if 0
	if(! INIT_COMPLETE_FLAG)
		return;
#endif

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
#endif

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
#endif

	/***********************************************/
	/* read per-port spf port status/control. */
	/* FIXME : call fuction. */
	update_port_sfp_information();


	/***********************************************/
	/* process per-port performance info from sdk. */
	process_port_pm_counters();
}
#endif

unsigned long __COMMON_CTRL2_ADDR[PORT_ID_EAG6L_MAX] = { 
		NULL_REG_ADDR, 
		COMMON_CTRL2_P1_ADDR,
		COMMON_CTRL2_P2_ADDR, 
		COMMON_CTRL2_P3_ADDR, 
		COMMON_CTRL2_P4_ADDR, 
		COMMON_CTRL2_P5_ADDR,
		COMMON_CTRL2_P6_ADDR, 
		NULL_REG_ADDR 
};
unsigned long __PORT_CONFIG_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, PORT_1_CONF_ADDR,
		PORT_2_CONF_ADDR, PORT_3_CONF_ADDR, 
		PORT_4_CONF_ADDR, PORT_5_CONF_ADDR,
		PORT_6_CONF_ADDR, PORT_7_CONF_ADDR };
unsigned long __PORT_TX_PWR_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, PORT_1_TX_PWR_ADDR,
		PORT_2_TX_PWR_ADDR, PORT_3_TX_PWR_ADDR, 
		PORT_4_TX_PWR_ADDR, PORT_5_TX_PWR_ADDR,
		PORT_6_TX_PWR_ADDR, PORT_7_TX_PWR_ADDR };
#if 1 /* [#91] Fixing for register updating feature, dustin, 2024-08-05 */
unsigned long __PORT_RX_PWR_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, PORT_1_RX_PWR_ADDR,
		PORT_2_RX_PWR_ADDR, PORT_3_RX_PWR_ADDR, 
		PORT_4_RX_PWR_ADDR, PORT_5_RX_PWR_ADDR,
		PORT_6_RX_PWR_ADDR, PORT_7_RX_PWR_ADDR };
#else
unsigned long __PORT_RX_PWR_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, PORT_1_TX_PWR_ADDR,
		PORT_2_TX_PWR_ADDR, PORT_3_TX_PWR_ADDR, 
		PORT_4_TX_PWR_ADDR, PORT_5_TX_PWR_ADDR,
		PORT_6_TX_PWR_ADDR, PORT_7_TX_PWR_ADDR };
#endif
#if 1 /* [#107] Fixing for 2nd register updates, dustin, 2024-08-29 */
unsigned long __PORT_WL1_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, PORT_1_WL1_ADDR,
		PORT_2_WL1_ADDR, PORT_3_WL1_ADDR, 
		PORT_4_WL1_ADDR, PORT_5_WL1_ADDR,
		PORT_6_WL1_ADDR, PORT_7_WL1_ADDR };
#else
unsigned long __PORT_WL1_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, PORT_1_WL1_ADDR,
		PORT_2_WL1_ADDR, PORT_3_WL1_ADDR, 
		PORT_4_WL1_ADDR, PORT_5_WL1_ADDR,
		PORT_6_WL1_ADDR, PORT_7_TX_PWR_ADDR };
#endif
unsigned long __PORT_WL2_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, PORT_1_WL2_ADDR,
		PORT_2_WL2_ADDR, PORT_3_WL2_ADDR, 
		PORT_4_WL2_ADDR, PORT_5_WL2_ADDR,
		PORT_6_WL2_ADDR, PORT_7_WL2_ADDR };
#if 1 /* [#107] Fixing for 2nd register updates, dustin, 2024-08-29 */
unsigned long __PORT_DIST_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, PORT_1_DIST_ADDR,
		PORT_2_DIST_ADDR, PORT_3_DIST_ADDR, 
		PORT_4_DIST_ADDR, PORT_5_DIST_ADDR,
		PORT_6_DIST_ADDR, PORT_7_DIST_ADDR };
#else
unsigned long __PORT_DIST_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, PORT_1_DIST_ADDR,
		PORT_2_DIST_ADDR, PORT_3_DIST_ADDR, 
		PORT_4_DIST_ADDR, PORT_5_DIST_ADDR,
		PORT_6_DIST_ADDR, PORT_7_WL2_ADDR };
#endif
#if 1 /* [#91] Fixing for register updating feature, dustin, 2024-08-05 */
unsigned long __PORT_STSFP_STAT_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, PORT_1_STSFP_STAT_ADDR,
		PORT_2_STSFP_STAT_ADDR, PORT_3_STSFP_STAT_ADDR, 
		PORT_4_STSFP_STAT_ADDR, PORT_5_STSFP_STAT_ADDR,
		PORT_5_STSFP_STAT_ADDR, NULL_REG_ADDR };
#else
unsigned long __PORT_STSFP_STAT_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, PORT_1_STSFP_STAT_ADDR,
		PORT_2_STSFP_STAT_ADDR, PORT_3_STSFP_STAT_ADDR, 
		PORT_4_STSFP_STAT_ADDR, PORT_5_STSFP_STAT_ADDR,
		PORT_5_STSFP_STAT_ADDR, PORT_7_STSFP_STAT_ADDR };
#endif
unsigned long __PORT_ALM_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, PORT_1_ALM_ADDR,
		PORT_2_ALM_ADDR, PORT_3_ALM_ADDR, 
		PORT_4_ALM_ADDR, PORT_5_ALM_ADDR,
		PORT_6_ALM_ADDR, PORT_7_ALM_ADDR };
unsigned long __PORT_ALM_FLAG_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, PORT_1_ALM_FLAG_ADDR,
		PORT_2_ALM_FLAG_ADDR, PORT_3_ALM_FLAG_ADDR, 
		PORT_4_ALM_FLAG_ADDR, PORT_5_ALM_FLAG_ADDR,
		PORT_6_ALM_FLAG_ADDR, PORT_7_ALM_FLAG_ADDR };
unsigned long __PORT_ALM_MASK_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, PORT_1_ALM_MASK_ADDR,
		PORT_2_ALM_MASK_ADDR, PORT_3_ALM_MASK_ADDR, 
		PORT_4_ALM_MASK_ADDR, PORT_5_ALM_MASK_ADDR,
		PORT_6_ALM_MASK_ADDR, PORT_7_ALM_MASK_ADDR };
unsigned long __PORT_GET_CH_NUM_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, PORT_1_GET_CH_NUM_ADDR,
		PORT_2_GET_CH_NUM_ADDR, PORT_3_GET_CH_NUM_ADDR, 
		PORT_4_GET_CH_NUM_ADDR, PORT_5_GET_CH_NUM_ADDR,
		PORT_6_GET_CH_NUM_ADDR, PORT_7_GET_CH_NUM_ADDR };
unsigned long __PORT_SET_CH_NUM_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, PORT_1_SET_CH_NUM_ADDR,
		PORT_2_SET_CH_NUM_ADDR, PORT_3_SET_CH_NUM_ADDR, 
		PORT_4_SET_CH_NUM_ADDR, PORT_5_SET_CH_NUM_ADDR,
		PORT_6_SET_CH_NUM_ADDR, PORT_7_SET_CH_NUM_ADDR };
#if 1 /* [#154] Fixing for auto FEC mode on DCO, dustin, 2024-10-21 */
unsigned long __PORT_RS_FEC_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, PORT_1_RS_FEC_ADDR,
		PORT_2_RS_FEC_ADDR, PORT_3_RS_FEC_ADDR, 
		PORT_4_RS_FEC_ADDR, PORT_5_RS_FEC_ADDR,
		PORT_6_RS_FEC_ADDR, PORT_7_RS_FEC_ADDR };
#endif
unsigned long __PORT_VENDOR1_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, SFP_P1_VENDOR1_ADDR,
		SFP_P2_VENDOR1_ADDR, SFP_P3_VENDOR1_ADDR, 
		SFP_P4_VENDOR1_ADDR, SFP_P5_VENDOR1_ADDR,
		SFP_P6_VENDOR1_ADDR, SFP_P7_VENDOR1_ADDR };
unsigned long __PORT_VENDOR2_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, SFP_P1_VENDOR2_ADDR,
		SFP_P2_VENDOR2_ADDR, SFP_P3_VENDOR2_ADDR, 
		SFP_P4_VENDOR2_ADDR, SFP_P5_VENDOR2_ADDR,
		SFP_P6_VENDOR2_ADDR, SFP_P7_VENDOR2_ADDR };
unsigned long __PORT_VENDOR3_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, SFP_P1_VENDOR3_ADDR,
		SFP_P2_VENDOR3_ADDR, SFP_P3_VENDOR3_ADDR, 
		SFP_P4_VENDOR3_ADDR, SFP_P5_VENDOR3_ADDR,
		SFP_P6_VENDOR3_ADDR, SFP_P7_VENDOR3_ADDR };
unsigned long __PORT_VENDOR4_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, SFP_P1_VENDOR4_ADDR,
		SFP_P2_VENDOR4_ADDR, SFP_P3_VENDOR4_ADDR, 
		SFP_P4_VENDOR4_ADDR, SFP_P5_VENDOR4_ADDR,
		SFP_P6_VENDOR4_ADDR, SFP_P7_VENDOR4_ADDR };
unsigned long __PORT_VENDOR5_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, SFP_P1_VENDOR5_ADDR,
		SFP_P2_VENDOR5_ADDR, SFP_P3_VENDOR5_ADDR, 
		SFP_P4_VENDOR5_ADDR, SFP_P5_VENDOR5_ADDR,
		SFP_P6_VENDOR5_ADDR, SFP_P7_VENDOR5_ADDR };
unsigned long __PORT_VENDOR6_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, SFP_P1_VENDOR6_ADDR,
		SFP_P2_VENDOR6_ADDR, SFP_P3_VENDOR6_ADDR, 
		SFP_P4_VENDOR6_ADDR, SFP_P5_VENDOR6_ADDR,
		SFP_P6_VENDOR6_ADDR, SFP_P7_VENDOR6_ADDR };
unsigned long __PORT_VENDOR7_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, SFP_P1_VENDOR7_ADDR,
		SFP_P2_VENDOR7_ADDR, SFP_P3_VENDOR7_ADDR, 
		SFP_P4_VENDOR7_ADDR, SFP_P5_VENDOR7_ADDR,
		SFP_P6_VENDOR7_ADDR, SFP_P7_VENDOR7_ADDR };
unsigned long __PORT_VENDOR8_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, SFP_P1_VENDOR8_ADDR,
		SFP_P2_VENDOR8_ADDR, SFP_P3_VENDOR8_ADDR, 
		SFP_P4_VENDOR8_ADDR, SFP_P5_VENDOR8_ADDR,
		SFP_P6_VENDOR8_ADDR, SFP_P7_VENDOR8_ADDR };
unsigned long __PORT_PN1_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, SFP_P1_PN1_ADDR,
		SFP_P2_PN1_ADDR, SFP_P3_PN1_ADDR, 
		SFP_P4_PN1_ADDR, SFP_P5_PN1_ADDR,
		SFP_P6_PN1_ADDR, SFP_P7_PN1_ADDR };
unsigned long __PORT_PN2_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, SFP_P1_PN2_ADDR,
		SFP_P2_PN2_ADDR, SFP_P3_PN2_ADDR, 
		SFP_P4_PN2_ADDR, SFP_P5_PN2_ADDR,
		SFP_P6_PN2_ADDR, SFP_P7_PN2_ADDR };
unsigned long __PORT_PN3_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, SFP_P1_PN3_ADDR,
		SFP_P2_PN3_ADDR, SFP_P3_PN3_ADDR, 
		SFP_P4_PN3_ADDR, SFP_P5_PN3_ADDR,
		SFP_P6_PN3_ADDR, SFP_P7_PN3_ADDR };
unsigned long __PORT_PN4_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, SFP_P1_PN4_ADDR,
		SFP_P2_PN4_ADDR, SFP_P3_PN4_ADDR, 
		SFP_P4_PN4_ADDR, SFP_P5_PN4_ADDR,
		SFP_P6_PN4_ADDR, SFP_P7_PN4_ADDR };
unsigned long __PORT_PN5_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, SFP_P1_PN5_ADDR,
		SFP_P2_PN5_ADDR, SFP_P3_PN5_ADDR, 
		SFP_P4_PN5_ADDR, SFP_P5_PN5_ADDR,
		SFP_P6_PN5_ADDR, SFP_P7_PN5_ADDR };
unsigned long __PORT_PN6_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, SFP_P1_PN6_ADDR,
		SFP_P2_PN6_ADDR, SFP_P3_PN6_ADDR, 
		SFP_P4_PN6_ADDR, SFP_P5_PN6_ADDR,
		SFP_P6_PN6_ADDR, SFP_P7_PN6_ADDR };
unsigned long __PORT_PN7_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, SFP_P1_PN7_ADDR,
		SFP_P2_PN7_ADDR, SFP_P3_PN7_ADDR, 
		SFP_P4_PN7_ADDR, SFP_P5_PN7_ADDR,
		SFP_P6_PN7_ADDR, SFP_P7_PN7_ADDR };
unsigned long __PORT_PN8_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, SFP_P1_PN8_ADDR,
		SFP_P2_PN8_ADDR, SFP_P3_PN8_ADDR, 
		SFP_P4_PN8_ADDR, SFP_P5_PN8_ADDR,
		SFP_P6_PN8_ADDR, SFP_P7_PN8_ADDR };
unsigned long __PORT_SN1_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, SFP_P1_SN1_ADDR,
		SFP_P2_SN1_ADDR, SFP_P3_SN1_ADDR, 
		SFP_P4_SN1_ADDR, SFP_P5_SN1_ADDR,
		SFP_P6_SN1_ADDR, SFP_P7_SN1_ADDR };
unsigned long __PORT_SN2_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, SFP_P1_SN2_ADDR,
		SFP_P2_SN2_ADDR, SFP_P3_SN2_ADDR, 
		SFP_P4_SN2_ADDR, SFP_P5_SN2_ADDR,
		SFP_P6_SN2_ADDR, SFP_P7_SN2_ADDR };
unsigned long __PORT_SN3_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, SFP_P1_SN3_ADDR,
		SFP_P2_SN3_ADDR, SFP_P3_SN3_ADDR, 
		SFP_P4_SN3_ADDR, SFP_P5_SN3_ADDR,
		SFP_P6_SN3_ADDR, SFP_P7_SN3_ADDR };
unsigned long __PORT_SN4_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, SFP_P1_SN4_ADDR,
		SFP_P2_SN4_ADDR, SFP_P3_SN4_ADDR, 
		SFP_P4_SN4_ADDR, SFP_P5_SN4_ADDR,
		SFP_P6_SN4_ADDR, SFP_P7_SN4_ADDR };
unsigned long __PORT_SN5_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, SFP_P1_SN5_ADDR,
		SFP_P2_SN5_ADDR, SFP_P3_SN5_ADDR, 
		SFP_P4_SN5_ADDR, SFP_P5_SN5_ADDR,
		SFP_P6_SN5_ADDR, SFP_P7_SN5_ADDR };
unsigned long __PORT_SN6_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, SFP_P1_SN6_ADDR,
		SFP_P2_SN6_ADDR, SFP_P3_SN6_ADDR, 
		SFP_P4_SN6_ADDR, SFP_P5_SN6_ADDR,
		SFP_P6_SN6_ADDR, SFP_P7_SN6_ADDR };
unsigned long __PORT_SN7_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, SFP_P1_SN7_ADDR,
		SFP_P2_SN7_ADDR, SFP_P3_SN7_ADDR, 
		SFP_P4_SN7_ADDR, SFP_P5_SN7_ADDR,
		SFP_P6_SN7_ADDR, SFP_P7_SN7_ADDR };
unsigned long __PORT_SN8_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, SFP_P1_SN8_ADDR,
		SFP_P2_SN8_ADDR, SFP_P3_SN8_ADDR, 
		SFP_P4_SN8_ADDR, SFP_P5_SN8_ADDR,
		SFP_P6_SN8_ADDR, SFP_P7_SN8_ADDR };
unsigned long __PORT_TEMP_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, SFP_P1_TEMP_ADDR,
		SFP_P2_TEMP_ADDR, SFP_P3_TEMP_ADDR, 
		SFP_P4_TEMP_ADDR, SFP_P5_TEMP_ADDR,
		SFP_P6_TEMP_ADDR, SFP_P7_TEMP_ADDR };
unsigned long __PORT_RATE_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, SFP_P1_RATE_ADDR,
		SFP_P2_RATE_ADDR, SFP_P3_RATE_ADDR, 
		SFP_P4_RATE_ADDR, SFP_P5_RATE_ADDR,
		SFP_P6_RATE_ADDR, SFP_P7_RATE_ADDR };
unsigned long __PORT_VOLT_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, SFP_P1_VOLT_ADDR,
		SFP_P2_VOLT_ADDR, SFP_P3_VOLT_ADDR, 
		SFP_P4_VOLT_ADDR, SFP_P5_VOLT_ADDR,
		SFP_P6_VOLT_ADDR, SFP_P7_VOLT_ADDR };
unsigned long __PORT_TX_BIAS_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, SFP_P1_TX_BIAS_ADDR,
		SFP_P2_TX_BIAS_ADDR, SFP_P3_TX_BIAS_ADDR, 
		SFP_P4_TX_BIAS_ADDR, SFP_P5_TX_BIAS_ADDR,
		SFP_P6_TX_BIAS_ADDR, SFP_P7_TX_BIAS_ADDR };
unsigned long __PORT_LTEMP_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, SFP_P1_LTEMP_ADDR,
		SFP_P2_LTEMP_ADDR, SFP_P3_LTEMP_ADDR, 
		SFP_P4_LTEMP_ADDR, SFP_P5_LTEMP_ADDR,
		SFP_P6_LTEMP_ADDR, SFP_P7_LTEMP_ADDR };
unsigned long __PORT_TCURR_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, SFP_P1_TCURR_ADDR,
		SFP_P2_TCURR_ADDR, SFP_P3_TCURR_ADDR, 
		SFP_P4_TCURR_ADDR, SFP_P5_TCURR_ADDR,
		SFP_P6_TCURR_ADDR, SFP_P7_TCURR_ADDR };
unsigned long __PORT_PM_TX_BYTE1_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, PM_P1_TX_BYTE1_ADDR,
		PM_P2_TX_BYTE1_ADDR, PM_P3_TX_BYTE1_ADDR, 
		PM_P4_TX_BYTE1_ADDR, PM_P5_TX_BYTE1_ADDR,
		PM_P6_TX_BYTE1_ADDR, PM_P7_TX_BYTE1_ADDR };
unsigned long __PORT_PM_TX_BYTE2_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, PM_P1_TX_BYTE2_ADDR,
		PM_P2_TX_BYTE2_ADDR, PM_P3_TX_BYTE2_ADDR, 
		PM_P4_TX_BYTE2_ADDR, PM_P5_TX_BYTE2_ADDR,
		PM_P6_TX_BYTE2_ADDR, PM_P7_TX_BYTE2_ADDR };
unsigned long __PORT_PM_TX_BYTE3_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, PM_P1_TX_BYTE3_ADDR,
		PM_P2_TX_BYTE3_ADDR, PM_P3_TX_BYTE3_ADDR, 
		PM_P4_TX_BYTE3_ADDR, PM_P5_TX_BYTE3_ADDR,
		PM_P6_TX_BYTE3_ADDR, PM_P7_TX_BYTE3_ADDR };
unsigned long __PORT_PM_TX_BYTE4_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, PM_P1_TX_BYTE4_ADDR,
		PM_P2_TX_BYTE4_ADDR, PM_P3_TX_BYTE4_ADDR, 
		PM_P4_TX_BYTE4_ADDR, PM_P5_TX_BYTE4_ADDR,
		PM_P6_TX_BYTE4_ADDR, PM_P7_TX_BYTE4_ADDR };
unsigned long __PORT_PM_RX_BYTE1_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, PM_P1_RX_BYTE1_ADDR,
		PM_P2_RX_BYTE1_ADDR, PM_P3_RX_BYTE1_ADDR, 
		PM_P4_RX_BYTE1_ADDR, PM_P5_RX_BYTE1_ADDR,
		PM_P6_RX_BYTE1_ADDR, PM_P7_RX_BYTE1_ADDR };
unsigned long __PORT_PM_RX_BYTE2_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, PM_P1_RX_BYTE2_ADDR,
		PM_P2_RX_BYTE2_ADDR, PM_P3_RX_BYTE2_ADDR, 
		PM_P4_RX_BYTE2_ADDR, PM_P5_RX_BYTE2_ADDR,
		PM_P6_RX_BYTE2_ADDR, PM_P7_RX_BYTE2_ADDR };
unsigned long __PORT_PM_RX_BYTE3_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, PM_P1_RX_BYTE3_ADDR,
		PM_P2_RX_BYTE3_ADDR, PM_P3_RX_BYTE3_ADDR, 
		PM_P4_RX_BYTE3_ADDR, PM_P5_RX_BYTE3_ADDR,
		PM_P6_RX_BYTE3_ADDR, PM_P7_RX_BYTE3_ADDR };
unsigned long __PORT_PM_RX_BYTE4_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, PM_P1_RX_BYTE4_ADDR,
		PM_P2_RX_BYTE4_ADDR, PM_P3_RX_BYTE4_ADDR, 
		PM_P4_RX_BYTE4_ADDR, PM_P5_RX_BYTE4_ADDR,
		PM_P6_RX_BYTE4_ADDR, PM_P7_RX_BYTE4_ADDR };
unsigned long __PORT_PM_TX_FRAME1_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, PM_P1_TX_FRAME1_ADDR,
		PM_P2_TX_FRAME1_ADDR, PM_P3_TX_FRAME1_ADDR, 
		PM_P4_TX_FRAME1_ADDR, PM_P5_TX_FRAME1_ADDR,
		PM_P6_TX_FRAME1_ADDR, PM_P7_TX_FRAME1_ADDR };
unsigned long __PORT_PM_TX_FRAME2_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, PM_P1_TX_FRAME2_ADDR,
		PM_P2_TX_FRAME2_ADDR, PM_P3_TX_FRAME2_ADDR, 
		PM_P4_TX_FRAME2_ADDR, PM_P5_TX_FRAME2_ADDR,
		PM_P6_TX_FRAME2_ADDR, PM_P7_TX_FRAME2_ADDR };
unsigned long __PORT_PM_TX_FRAME3_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, PM_P1_TX_FRAME3_ADDR,
		PM_P2_TX_FRAME3_ADDR, PM_P3_TX_FRAME3_ADDR, 
		PM_P4_TX_FRAME3_ADDR, PM_P5_TX_FRAME3_ADDR,
		PM_P6_TX_FRAME3_ADDR, PM_P7_TX_FRAME3_ADDR };
unsigned long __PORT_PM_TX_FRAME4_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, PM_P1_TX_FRAME4_ADDR,
		PM_P2_TX_FRAME4_ADDR, PM_P3_TX_FRAME4_ADDR, 
		PM_P4_TX_FRAME4_ADDR, PM_P5_TX_FRAME4_ADDR,
		PM_P6_TX_FRAME4_ADDR, PM_P7_TX_FRAME4_ADDR };
unsigned long __PORT_PM_RX_FRAME1_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, PM_P1_RX_FRAME1_ADDR,
		PM_P2_RX_FRAME1_ADDR, PM_P3_RX_FRAME1_ADDR, 
		PM_P4_RX_FRAME1_ADDR, PM_P5_RX_FRAME1_ADDR,
		PM_P6_RX_FRAME1_ADDR, PM_P7_RX_FRAME1_ADDR };
unsigned long __PORT_PM_RX_FRAME2_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, PM_P1_RX_FRAME2_ADDR,
		PM_P2_RX_FRAME2_ADDR, PM_P3_RX_FRAME2_ADDR, 
		PM_P4_RX_FRAME2_ADDR, PM_P5_RX_FRAME2_ADDR,
		PM_P6_RX_FRAME2_ADDR, PM_P7_RX_FRAME2_ADDR };
unsigned long __PORT_PM_RX_FRAME3_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, PM_P1_RX_FRAME3_ADDR,
		PM_P2_RX_FRAME3_ADDR, PM_P3_RX_FRAME3_ADDR, 
		PM_P4_RX_FRAME3_ADDR, PM_P5_RX_FRAME3_ADDR,
		PM_P6_RX_FRAME3_ADDR, PM_P7_RX_FRAME3_ADDR };
unsigned long __PORT_PM_RX_FRAME4_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, PM_P1_RX_FRAME4_ADDR,
		PM_P2_RX_FRAME4_ADDR, PM_P3_RX_FRAME4_ADDR, 
		PM_P4_RX_FRAME4_ADDR, PM_P5_RX_FRAME4_ADDR,
		PM_P6_RX_FRAME4_ADDR, PM_P7_RX_FRAME4_ADDR };
unsigned long __PORT_PM_FCS1_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, PM_P1_FCS1_ADDR,
		PM_P2_FCS1_ADDR, PM_P3_FCS1_ADDR, 
		PM_P4_FCS1_ADDR, PM_P5_FCS1_ADDR,
		PM_P6_FCS1_ADDR, PM_P7_FCS1_ADDR };
unsigned long __PORT_PM_FCS2_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, PM_P1_FCS2_ADDR,
		PM_P2_FCS2_ADDR, PM_P3_FCS2_ADDR, 
		PM_P4_FCS2_ADDR, PM_P5_FCS2_ADDR,
		PM_P6_FCS2_ADDR, PM_P7_FCS2_ADDR };
unsigned long __PORT_PM_FCS3_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, PM_P1_FCS3_ADDR,
		PM_P2_FCS3_ADDR, PM_P3_FCS3_ADDR, 
		PM_P4_FCS3_ADDR, PM_P5_FCS3_ADDR,
		PM_P6_FCS3_ADDR, PM_P7_FCS3_ADDR };
unsigned long __PORT_PM_FCS4_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, PM_P1_FCS4_ADDR,
		PM_P2_FCS4_ADDR, PM_P3_FCS4_ADDR, 
		PM_P4_FCS4_ADDR, PM_P5_FCS4_ADDR,
		PM_P6_FCS4_ADDR, PM_P7_FCS4_ADDR };
unsigned long __PORT_PM_FCS_OK1_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, PM_P1_FCS_OK1_ADDR,
		PM_P2_FCS_OK1_ADDR, PM_P3_FCS_OK1_ADDR, 
		PM_P4_FCS_OK1_ADDR, PM_P5_FCS_OK1_ADDR,
		PM_P6_FCS_OK1_ADDR, PM_P7_FCS_OK1_ADDR };
unsigned long __PORT_PM_FCS_OK2_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, PM_P1_FCS_OK2_ADDR,
		PM_P2_FCS_OK2_ADDR, PM_P3_FCS_OK2_ADDR, 
		PM_P4_FCS_OK2_ADDR, PM_P5_FCS_OK2_ADDR,
		PM_P6_FCS_OK2_ADDR, PM_P7_FCS_OK2_ADDR };
#if 0 /* [#147] Fixing for 4th register update, dustin, 2024-10-21 */
/* NOTE : removed by requst of cwhan. */
unsigned long __PORT_PM_FCS_OK3_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, PM_P1_FCS_OK3_ADDR,
		PM_P2_FCS_OK3_ADDR, PM_P3_FCS_OK3_ADDR, 
		PM_P4_FCS_OK3_ADDR, PM_P5_FCS_OK3_ADDR,
		PM_P6_FCS_OK3_ADDR, PM_P7_FCS_OK3_ADDR };
#if 1 /* [#107] Fixing for 2nd register updates, dustin, 2024-08-29 */
unsigned long __PORT_PM_FCS_OK4_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, PM_P1_FCS_OK4_ADDR,
		PM_P2_FCS_OK4_ADDR, PM_P3_FCS_OK4_ADDR, 
		PM_P4_FCS_OK4_ADDR, PM_P5_FCS_OK4_ADDR,
		PM_P6_FCS_OK4_ADDR, PM_P7_FCS_OK4_ADDR };
#else
unsigned long __PORT_PM_FCS_OK4_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, PM_P1_FCS_OK4_ADDR,
		PM_P2_FCS_OK4_ADDR, PM_P3_FCS_OK4_ADDR, 
		PM_P4_FCS_OK4_ADDR, PM_P5_FCS_OK4_ADDR,
		PM_P6_FCS_OK4_ADDR, PM_P7_FCS4_ADDR };
#endif
#endif /* [#147] */
unsigned long __PORT_PM_FCS_NOK1_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, PM_P1_FCS_NOK1_ADDR,
		PM_P2_FCS_NOK1_ADDR, PM_P3_FCS_NOK1_ADDR, 
		PM_P4_FCS_NOK1_ADDR, PM_P5_FCS_NOK1_ADDR,
		PM_P6_FCS_NOK1_ADDR, PM_P7_FCS_NOK1_ADDR };
unsigned long __PORT_PM_FCS_NOK2_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, PM_P1_FCS_NOK2_ADDR,
		PM_P2_FCS_NOK2_ADDR, PM_P3_FCS_NOK2_ADDR, 
		PM_P4_FCS_NOK2_ADDR, PM_P5_FCS_NOK2_ADDR,
		PM_P6_FCS_NOK2_ADDR, PM_P7_FCS_NOK2_ADDR };
#if 0 /* [#147] Fixing for 4th register update, dustin, 2024-10-21 */
/* NOTE : removed by requst of cwhan. */
unsigned long __PORT_PM_FCS_NOK3_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, PM_P1_FCS_NOK3_ADDR,
		PM_P2_FCS_NOK3_ADDR, PM_P3_FCS_NOK3_ADDR, 
		PM_P4_FCS_NOK3_ADDR, PM_P5_FCS_NOK3_ADDR,
		PM_P6_FCS_NOK3_ADDR, PM_P7_FCS_NOK3_ADDR };
#if 1 /* [#107] Fixing for 2nd register updates, dustin, 2024-08-29 */
unsigned long __PORT_PM_FCS_NOK4_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, PM_P1_FCS_NOK4_ADDR,
		PM_P2_FCS_NOK4_ADDR, PM_P3_FCS_NOK4_ADDR, 
		PM_P4_FCS_NOK4_ADDR, PM_P5_FCS_NOK4_ADDR,
		PM_P6_FCS_NOK4_ADDR, PM_P7_FCS_NOK4_ADDR };
#else
unsigned long __PORT_PM_FCS_NOK4_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, PM_P1_FCS_NOK4_ADDR,
		PM_P2_FCS_NOK4_ADDR, PM_P3_FCS_NOK4_ADDR, 
		PM_P4_FCS_NOK4_ADDR, PM_P5_FCS_NOK4_ADDR,
		PM_P6_FCS_NOK4_ADDR, PM_P7_FCS4_ADDR };
#endif
#endif /* [#147] */
unsigned long __PORT_CLEI1_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, PORT1_CLEI1_ADDR,
		PORT2_CLEI1_ADDR, PORT3_CLEI1_ADDR, 
		PORT4_CLEI1_ADDR, PORT5_CLEI1_ADDR,
		PORT6_CLEI1_ADDR, PORT7_CLEI1_ADDR };
unsigned long __PORT_CLEI2_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, PORT1_CLEI2_ADDR,
		PORT2_CLEI2_ADDR, PORT3_CLEI2_ADDR, 
		PORT4_CLEI2_ADDR, PORT5_CLEI2_ADDR,
		PORT6_CLEI2_ADDR, PORT7_CLEI2_ADDR };
unsigned long __PORT_CLEI3_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, PORT1_CLEI3_ADDR,
		PORT2_CLEI3_ADDR, PORT3_CLEI3_ADDR, 
		PORT4_CLEI3_ADDR, PORT5_CLEI3_ADDR,
		PORT6_CLEI3_ADDR, PORT7_CLEI3_ADDR };
unsigned long __PORT_CLEI4_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, PORT1_CLEI4_ADDR,
		PORT2_CLEI4_ADDR, PORT3_CLEI4_ADDR, 
		PORT4_CLEI4_ADDR, PORT5_CLEI4_ADDR,
		PORT6_CLEI4_ADDR, PORT7_CLEI4_ADDR };
unsigned long __PORT_CLEI5_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, PORT1_CLEI5_ADDR,
		PORT2_CLEI5_ADDR, PORT3_CLEI5_ADDR, 
		PORT4_CLEI5_ADDR, PORT5_CLEI5_ADDR,
		PORT6_CLEI5_ADDR, PORT7_CLEI5_ADDR };
unsigned long __PORT_USI1_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, PORT1_USI1_ADDR,
		PORT2_USI1_ADDR, PORT3_USI1_ADDR, 
		PORT4_USI1_ADDR, PORT5_USI1_ADDR,
		PORT6_USI1_ADDR, PORT7_USI1_ADDR };
unsigned long __PORT_USI2_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, PORT1_USI2_ADDR,
		PORT2_USI2_ADDR, PORT3_USI2_ADDR, 
		PORT4_USI2_ADDR, PORT5_USI2_ADDR,
		PORT6_USI2_ADDR, PORT7_USI2_ADDR };
unsigned long __PORT_USI3_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, PORT1_USI3_ADDR,
		PORT2_USI3_ADDR, PORT3_USI3_ADDR, 
		PORT4_USI3_ADDR, PORT5_USI3_ADDR,
		PORT6_USI3_ADDR, PORT7_USI3_ADDR };
unsigned long __PORT_USI4_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, PORT1_USI4_ADDR,
		PORT2_USI4_ADDR, PORT3_USI4_ADDR, 
		PORT4_USI4_ADDR, PORT5_USI4_ADDR,
		PORT6_USI4_ADDR, PORT7_USI4_ADDR };
unsigned long __PORT_USI5_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, PORT1_USI5_ADDR,
		PORT2_USI5_ADDR, PORT3_USI5_ADDR, 
		PORT4_USI5_ADDR, PORT5_USI5_ADDR,
		PORT6_USI5_ADDR, PORT7_USI5_ADDR };
unsigned long __PORT_USI6_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, PORT1_USI6_ADDR,
		PORT2_USI6_ADDR, PORT3_USI6_ADDR, 
		PORT4_USI6_ADDR, PORT5_USI6_ADDR,
		PORT6_USI6_ADDR, PORT7_USI6_ADDR };
unsigned long __PORT_USI7_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, PORT1_USI7_ADDR,
		PORT2_USI7_ADDR, PORT3_USI7_ADDR, 
		PORT4_USI7_ADDR, PORT5_USI7_ADDR,
		PORT6_USI7_ADDR, PORT7_USI7_ADDR };
unsigned long __PORT_USI8_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, PORT1_USI8_ADDR,
		PORT2_USI8_ADDR, PORT3_USI8_ADDR, 
		PORT4_USI8_ADDR, PORT5_USI8_ADDR,
		PORT6_USI8_ADDR, PORT7_USI8_ADDR };
unsigned long __PORT_USI9_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, PORT1_USI9_ADDR,
		PORT2_USI9_ADDR, PORT3_USI9_ADDR, 
		PORT4_USI9_ADDR, PORT5_USI9_ADDR,
		PORT6_USI9_ADDR, PORT7_USI9_ADDR };
unsigned long __PORT_USI10_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, PORT1_USI10_ADDR,
		PORT2_USI10_ADDR, PORT3_USI10_ADDR, 
		PORT4_USI10_ADDR, PORT5_USI10_ADDR,
		PORT6_USI10_ADDR, PORT7_USI10_ADDR };
unsigned long __PORT_USI11_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, PORT1_USI11_ADDR,
		PORT2_USI11_ADDR, PORT3_USI11_ADDR, 
		PORT4_USI11_ADDR, PORT5_USI11_ADDR,
		PORT6_USI11_ADDR, PORT7_USI11_ADDR };
unsigned long __PORT_USI12_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, PORT1_USI12_ADDR,
		PORT2_USI12_ADDR, PORT3_USI12_ADDR, 
		PORT4_USI12_ADDR, PORT5_USI12_ADDR,
		PORT6_USI12_ADDR, PORT7_USI12_ADDR };
unsigned long __PORT_USI13_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, PORT1_USI13_ADDR,
		PORT2_USI13_ADDR, PORT3_USI13_ADDR, 
		PORT4_USI13_ADDR, PORT5_USI13_ADDR,
		PORT6_USI13_ADDR, PORT7_USI13_ADDR };

#if 1/* [#72] Adding omitted rtWDM related registers, dustin, 2024-06-27 */
unsigned long __PORT_TX_PWR_RTWDM_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, PORT_1_TX_PWR_RTWDM_ADDR,
		PORT_2_TX_PWR_RTWDM_ADDR, PORT_3_TX_PWR_RTWDM_ADDR, 
		PORT_4_TX_PWR_RTWDM_ADDR, PORT_5_TX_PWR_RTWDM_ADDR,
		PORT_6_TX_PWR_RTWDM_ADDR, NULL_REG_ADDR };
#if 1 /* [#91] Fixing for register updating feature, dustin, 2024-08-05 */
unsigned long __PORT_RX_PWR_RTWDM_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, PORT_1_RX_PWR_RTWDM_ADDR,
		PORT_2_RX_PWR_RTWDM_ADDR, PORT_3_RX_PWR_RTWDM_ADDR, 
		PORT_4_RX_PWR_RTWDM_ADDR, PORT_5_RX_PWR_RTWDM_ADDR,
		PORT_6_RX_PWR_RTWDM_ADDR, NULL_REG_ADDR };
#else
unsigned long __PORT_RX_PWR_RTWDM_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, PORT_1_TX_PWR_RTWDM_ADDR,
		PORT_2_TX_PWR_RTWDM_ADDR, PORT_3_TX_PWR_RTWDM_ADDR, 
		PORT_4_TX_PWR_RTWDM_ADDR, PORT_5_TX_PWR_RTWDM_ADDR,
		PORT_6_TX_PWR_RTWDM_ADDR, NULL_REG_ADDR };
#endif
unsigned long __PORT_WL1_RTWDM_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, PORT_1_WL1_RTWDM_ADDR,
		PORT_2_WL1_RTWDM_ADDR, PORT_3_WL1_RTWDM_ADDR, 
		PORT_4_WL1_RTWDM_ADDR, PORT_5_WL1_RTWDM_ADDR,
		PORT_6_WL1_RTWDM_ADDR, NULL_REG_ADDR };
unsigned long __PORT_WL2_RTWDM_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, PORT_1_WL2_RTWDM_ADDR,
		PORT_2_WL2_RTWDM_ADDR, PORT_3_WL2_RTWDM_ADDR, 
		PORT_4_WL2_RTWDM_ADDR, PORT_5_WL2_RTWDM_ADDR,
		PORT_6_WL2_RTWDM_ADDR, NULL_REG_ADDR };
unsigned long __PORT_DIST_RTWDM_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, PORT_1_DIST_RTWDM_ADDR,
		PORT_2_DIST_RTWDM_ADDR, PORT_3_DIST_RTWDM_ADDR, 
		PORT_4_DIST_RTWDM_ADDR, PORT_5_DIST_RTWDM_ADDR,
		PORT_6_DIST_RTWDM_ADDR, NULL_REG_ADDR };
unsigned long __PORT_VENDOR1_RTWDM_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, SFP_P1_VENDOR1_RTWDM_ADDR,
		SFP_P2_VENDOR1_RTWDM_ADDR, SFP_P3_VENDOR1_RTWDM_ADDR, 
		SFP_P4_VENDOR1_RTWDM_ADDR, SFP_P5_VENDOR1_RTWDM_ADDR,
		SFP_P6_VENDOR1_RTWDM_ADDR, NULL_REG_ADDR };
unsigned long __PORT_VENDOR2_RTWDM_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, SFP_P1_VENDOR2_RTWDM_ADDR,
		SFP_P2_VENDOR2_RTWDM_ADDR, SFP_P3_VENDOR2_RTWDM_ADDR, 
		SFP_P4_VENDOR2_RTWDM_ADDR, SFP_P5_VENDOR2_RTWDM_ADDR,
		SFP_P6_VENDOR2_RTWDM_ADDR, NULL_REG_ADDR };
unsigned long __PORT_VENDOR3_RTWDM_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, SFP_P1_VENDOR3_RTWDM_ADDR,
		SFP_P2_VENDOR3_RTWDM_ADDR, SFP_P3_VENDOR3_RTWDM_ADDR, 
		SFP_P4_VENDOR3_RTWDM_ADDR, SFP_P5_VENDOR3_RTWDM_ADDR,
		SFP_P6_VENDOR3_RTWDM_ADDR, NULL_REG_ADDR };
unsigned long __PORT_VENDOR4_RTWDM_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, SFP_P1_VENDOR4_RTWDM_ADDR,
		SFP_P2_VENDOR4_RTWDM_ADDR, SFP_P3_VENDOR4_RTWDM_ADDR, 
		SFP_P4_VENDOR4_RTWDM_ADDR, SFP_P5_VENDOR4_RTWDM_ADDR,
		SFP_P6_VENDOR4_RTWDM_ADDR, NULL_REG_ADDR };
unsigned long __PORT_VENDOR5_RTWDM_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, SFP_P1_VENDOR5_RTWDM_ADDR,
		SFP_P2_VENDOR5_RTWDM_ADDR, SFP_P3_VENDOR5_RTWDM_ADDR, 
		SFP_P4_VENDOR5_RTWDM_ADDR, SFP_P5_VENDOR5_RTWDM_ADDR,
		SFP_P6_VENDOR5_RTWDM_ADDR, NULL_REG_ADDR };
unsigned long __PORT_VENDOR6_RTWDM_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, SFP_P1_VENDOR6_RTWDM_ADDR,
		SFP_P2_VENDOR6_RTWDM_ADDR, SFP_P3_VENDOR6_RTWDM_ADDR, 
		SFP_P4_VENDOR6_RTWDM_ADDR, SFP_P5_VENDOR6_RTWDM_ADDR,
		SFP_P6_VENDOR6_RTWDM_ADDR, NULL_REG_ADDR };
unsigned long __PORT_VENDOR7_RTWDM_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, SFP_P1_VENDOR7_RTWDM_ADDR,
		SFP_P2_VENDOR7_RTWDM_ADDR, SFP_P3_VENDOR7_RTWDM_ADDR, 
		SFP_P4_VENDOR7_RTWDM_ADDR, SFP_P5_VENDOR7_RTWDM_ADDR,
		SFP_P6_VENDOR7_RTWDM_ADDR, NULL_REG_ADDR };
unsigned long __PORT_VENDOR8_RTWDM_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, SFP_P1_VENDOR8_RTWDM_ADDR,
		SFP_P2_VENDOR8_RTWDM_ADDR, SFP_P3_VENDOR8_RTWDM_ADDR, 
		SFP_P4_VENDOR8_RTWDM_ADDR, SFP_P5_VENDOR8_RTWDM_ADDR,
		SFP_P6_VENDOR8_RTWDM_ADDR, NULL_REG_ADDR };
unsigned long __PORT_PN1_RTWDM_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, SFP_P1_PN1_RTWDM_ADDR,
		SFP_P2_PN1_RTWDM_ADDR, SFP_P3_PN1_RTWDM_ADDR, 
		SFP_P4_PN1_RTWDM_ADDR, SFP_P5_PN1_RTWDM_ADDR,
		SFP_P6_PN1_RTWDM_ADDR, NULL_REG_ADDR };
unsigned long __PORT_PN2_RTWDM_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, SFP_P1_PN2_RTWDM_ADDR,
		SFP_P2_PN2_RTWDM_ADDR, SFP_P3_PN2_RTWDM_ADDR, 
		SFP_P4_PN2_RTWDM_ADDR, SFP_P5_PN2_RTWDM_ADDR,
		SFP_P6_PN2_RTWDM_ADDR, NULL_REG_ADDR };
unsigned long __PORT_PN3_RTWDM_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, SFP_P1_PN3_RTWDM_ADDR,
		SFP_P2_PN3_RTWDM_ADDR, SFP_P3_PN3_RTWDM_ADDR, 
		SFP_P4_PN3_RTWDM_ADDR, SFP_P5_PN3_RTWDM_ADDR,
		SFP_P6_PN3_RTWDM_ADDR, NULL_REG_ADDR };
unsigned long __PORT_PN4_RTWDM_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, SFP_P1_PN4_RTWDM_ADDR,
		SFP_P2_PN4_RTWDM_ADDR, SFP_P3_PN4_RTWDM_ADDR, 
		SFP_P4_PN4_RTWDM_ADDR, SFP_P5_PN4_RTWDM_ADDR,
		SFP_P6_PN4_RTWDM_ADDR, NULL_REG_ADDR };
unsigned long __PORT_PN5_RTWDM_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, SFP_P1_PN5_RTWDM_ADDR,
		SFP_P2_PN5_RTWDM_ADDR, SFP_P3_PN5_RTWDM_ADDR, 
		SFP_P4_PN5_RTWDM_ADDR, SFP_P5_PN5_RTWDM_ADDR,
		SFP_P6_PN5_RTWDM_ADDR, NULL_REG_ADDR };
unsigned long __PORT_PN6_RTWDM_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, SFP_P1_PN6_RTWDM_ADDR,
		SFP_P2_PN6_RTWDM_ADDR, SFP_P3_PN6_RTWDM_ADDR, 
		SFP_P4_PN6_RTWDM_ADDR, SFP_P5_PN6_RTWDM_ADDR,
		SFP_P6_PN6_RTWDM_ADDR, NULL_REG_ADDR };
unsigned long __PORT_PN7_RTWDM_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, SFP_P1_PN7_RTWDM_ADDR,
		SFP_P2_PN7_RTWDM_ADDR, SFP_P3_PN7_RTWDM_ADDR, 
		SFP_P4_PN7_RTWDM_ADDR, SFP_P5_PN7_RTWDM_ADDR,
		SFP_P6_PN7_RTWDM_ADDR, NULL_REG_ADDR };
unsigned long __PORT_PN8_RTWDM_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, SFP_P1_PN8_RTWDM_ADDR,
		SFP_P2_PN8_RTWDM_ADDR, SFP_P3_PN8_RTWDM_ADDR, 
		SFP_P4_PN8_RTWDM_ADDR, SFP_P5_PN8_RTWDM_ADDR,
		SFP_P6_PN8_RTWDM_ADDR, NULL_REG_ADDR };
unsigned long __PORT_SN1_RTWDM_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, SFP_P1_SN1_RTWDM_ADDR,
		SFP_P2_SN1_RTWDM_ADDR, SFP_P3_SN1_RTWDM_ADDR, 
		SFP_P4_SN1_RTWDM_ADDR, SFP_P5_SN1_RTWDM_ADDR,
		SFP_P6_SN1_RTWDM_ADDR, NULL_REG_ADDR };
unsigned long __PORT_SN2_RTWDM_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, SFP_P1_SN2_RTWDM_ADDR,
		SFP_P2_SN2_RTWDM_ADDR, SFP_P3_SN2_RTWDM_ADDR, 
		SFP_P4_SN2_RTWDM_ADDR, SFP_P5_SN2_RTWDM_ADDR,
		SFP_P6_SN2_RTWDM_ADDR, NULL_REG_ADDR };
unsigned long __PORT_SN3_RTWDM_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, SFP_P1_SN3_RTWDM_ADDR,
		SFP_P2_SN3_RTWDM_ADDR, SFP_P3_SN3_RTWDM_ADDR, 
		SFP_P4_SN3_RTWDM_ADDR, SFP_P5_SN3_RTWDM_ADDR,
		SFP_P6_SN3_RTWDM_ADDR, NULL_REG_ADDR };
unsigned long __PORT_SN4_RTWDM_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, SFP_P1_SN4_RTWDM_ADDR,
		SFP_P2_SN4_RTWDM_ADDR, SFP_P3_SN4_RTWDM_ADDR, 
		SFP_P4_SN4_RTWDM_ADDR, SFP_P5_SN4_RTWDM_ADDR,
		SFP_P6_SN4_RTWDM_ADDR, NULL_REG_ADDR };
unsigned long __PORT_SN5_RTWDM_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, SFP_P1_SN5_RTWDM_ADDR,
		SFP_P2_SN5_RTWDM_ADDR, SFP_P3_SN5_RTWDM_ADDR, 
		SFP_P4_SN5_RTWDM_ADDR, SFP_P5_SN5_RTWDM_ADDR,
		SFP_P6_SN5_RTWDM_ADDR, NULL_REG_ADDR };
unsigned long __PORT_SN6_RTWDM_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, SFP_P1_SN6_RTWDM_ADDR,
		SFP_P2_SN6_RTWDM_ADDR, SFP_P3_SN6_RTWDM_ADDR, 
		SFP_P4_SN6_RTWDM_ADDR, SFP_P5_SN6_RTWDM_ADDR,
		SFP_P6_SN6_RTWDM_ADDR, NULL_REG_ADDR };
unsigned long __PORT_SN7_RTWDM_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, SFP_P1_SN7_RTWDM_ADDR,
		SFP_P2_SN7_RTWDM_ADDR, SFP_P3_SN7_RTWDM_ADDR, 
		SFP_P4_SN7_RTWDM_ADDR, SFP_P5_SN7_RTWDM_ADDR,
		SFP_P6_SN7_RTWDM_ADDR, NULL_REG_ADDR };
unsigned long __PORT_SN8_RTWDM_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, SFP_P1_SN8_RTWDM_ADDR,
		SFP_P2_SN8_RTWDM_ADDR, SFP_P3_SN8_RTWDM_ADDR, 
		SFP_P4_SN8_RTWDM_ADDR, SFP_P5_SN8_RTWDM_ADDR,
		SFP_P6_SN8_RTWDM_ADDR, NULL_REG_ADDR };
unsigned long __PORT_TEMP_RTWDM_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, SFP_P1_TEMP_RTWDM_ADDR,
		SFP_P2_TEMP_RTWDM_ADDR, SFP_P3_TEMP_RTWDM_ADDR, 
		SFP_P4_TEMP_RTWDM_ADDR, SFP_P5_TEMP_RTWDM_ADDR,
		SFP_P6_TEMP_RTWDM_ADDR, NULL_REG_ADDR };
unsigned long __PORT_RATE_RTWDM_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, SFP_P1_RATE_RTWDM_ADDR,
		SFP_P2_RATE_RTWDM_ADDR, SFP_P3_RATE_RTWDM_ADDR, 
		SFP_P4_RATE_RTWDM_ADDR, SFP_P5_RATE_RTWDM_ADDR,
		SFP_P6_RATE_RTWDM_ADDR, NULL_REG_ADDR };
unsigned long __PORT_VOLT_RTWDM_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, SFP_P1_VOLT_RTWDM_ADDR,
		SFP_P2_VOLT_RTWDM_ADDR, SFP_P3_VOLT_RTWDM_ADDR, 
		SFP_P4_VOLT_RTWDM_ADDR, SFP_P5_VOLT_RTWDM_ADDR,
		SFP_P6_VOLT_RTWDM_ADDR, NULL_REG_ADDR };
unsigned long __PORT_TX_BIAS_RTWDM_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, SFP_P1_TX_BIAS_RTWDM_ADDR,
		SFP_P2_TX_BIAS_RTWDM_ADDR, SFP_P3_TX_BIAS_RTWDM_ADDR, 
		SFP_P4_TX_BIAS_RTWDM_ADDR, SFP_P5_TX_BIAS_RTWDM_ADDR,
		SFP_P6_TX_BIAS_RTWDM_ADDR, NULL_REG_ADDR };
unsigned long __PORT_LTEMP_RTWDM_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, SFP_P1_LTEMP_RTWDM_ADDR,
		SFP_P2_LTEMP_RTWDM_ADDR, SFP_P3_LTEMP_RTWDM_ADDR, 
		SFP_P4_LTEMP_RTWDM_ADDR, SFP_P5_LTEMP_RTWDM_ADDR,
		SFP_P6_LTEMP_RTWDM_ADDR, NULL_REG_ADDR };
unsigned long __PORT_TCURR_RTWDM_ADDR[PORT_ID_EAG6L_MAX] = { NULL_REG_ADDR, SFP_P1_TCURR_RTWDM_ADDR,
		SFP_P2_TCURR_RTWDM_ADDR, SFP_P3_TCURR_RTWDM_ADDR, 
		SFP_P4_TCURR_RTWDM_ADDR, SFP_P5_TCURR_RTWDM_ADDR,
		SFP_P6_TCURR_RTWDM_ADDR, NULL_REG_ADDR };
#endif
