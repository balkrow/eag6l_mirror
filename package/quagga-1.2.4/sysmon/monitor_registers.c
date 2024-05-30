#include "sysmon.h"
#include "bp_regs.h"

extern u32 INIT_COMPLETE_FLAG;

u16 PORT_CONFIG_CACHE[PORT_ID_EAG6L_MAX];
u16 SYNCE_GCONFIG_CACHE;
u16 SYNCE_IF_SELECT_CACHE;
u32 MCU_KEEP_ALIVE_CACHE;


struct port_status PORT_STATUS[PORT_ID_EAG6L_MAX];
struct module_inventory INV_TBL[PORT_ID_EAG6L_MAX];


extern int synce_config_set_admin(u32 enable);
extern int synce_config_set_if_select(u32 pri_port, u32 sec_port);
extern int update_port_pm_counters(void);


unsigned long __COMMON_CTRL2_ADDR[PORT_ID_EAG6L_MAX] = { COMMON_CTRL2_P1_ADDR,
		COMMON_CTRL2_P2_ADDR, COMMON_CTRL2_P3_ADDR, 
		COMMON_CTRL2_P4_ADDR, COMMON_CTRL2_P5_ADDR,
		COMMON_CTRL2_P6_ADDR, COMMON_CTRL2_P7_ADDR };
unsigned long __PORT_CONFIG_ADDR[PORT_ID_EAG6L_MAX] = { PORT_1_CONF_ADDR,
		PORT_2_CONF_ADDR, PORT_3_CONF_ADDR, 
		PORT_4_CONF_ADDR, PORT_5_CONF_ADDR,
		PORT_6_CONF_ADDR, PORT_7_CONF_ADDR };
unsigned long __PORT_TX_PWR_ADDR[PORT_ID_EAG6L_MAX] = { PORT_1_TX_PWR_ADDR,
		PORT_2_TX_PWR_ADDR, PORT_3_TX_PWR_ADDR, 
		PORT_4_TX_PWR_ADDR, PORT_5_TX_PWR_ADDR,
		PORT_6_TX_PWR_ADDR, PORT_7_TX_PWR_ADDR };
unsigned long __PORT_RX_PWR_ADDR[PORT_ID_EAG6L_MAX] = { PORT_1_TX_PWR_ADDR,
		PORT_2_TX_PWR_ADDR, PORT_3_TX_PWR_ADDR, 
		PORT_4_TX_PWR_ADDR, PORT_5_TX_PWR_ADDR,
		PORT_6_TX_PWR_ADDR, PORT_7_TX_PWR_ADDR };
unsigned long __PORT_WL1_ADDR[PORT_ID_EAG6L_MAX] = { PORT_1_WL1_ADDR,
		PORT_2_WL1_ADDR, PORT_3_WL1_ADDR, 
		PORT_4_WL1_ADDR, PORT_5_WL1_ADDR,
		PORT_6_WL1_ADDR, PORT_7_TX_PWR_ADDR };
unsigned long __PORT_WL2_ADDR[PORT_ID_EAG6L_MAX] = { PORT_1_WL2_ADDR,
		PORT_2_WL2_ADDR, PORT_3_WL2_ADDR, 
		PORT_4_WL2_ADDR, PORT_5_WL2_ADDR,
		PORT_6_WL2_ADDR, PORT_7_WL2_ADDR };
unsigned long __PORT_DIST_ADDR[PORT_ID_EAG6L_MAX] = { PORT_1_DIST_ADDR,
		PORT_2_DIST_ADDR, PORT_3_DIST_ADDR, 
		PORT_4_DIST_ADDR, PORT_5_DIST_ADDR,
		PORT_6_DIST_ADDR, PORT_7_WL2_ADDR };
unsigned long __PORT_STSFP_STAT_ADDR[PORT_ID_EAG6L_MAX] = { PORT_1_STSFP_STAT_ADDR,
		PORT_2_STSFP_STAT_ADDR, PORT_3_STSFP_STAT_ADDR, 
		PORT_4_STSFP_STAT_ADDR, PORT_5_STSFP_STAT_ADDR,
		PORT_5_STSFP_STAT_ADDR, PORT_7_STSFP_STAT_ADDR };
unsigned long __PORT_ALM_ADDR[PORT_ID_EAG6L_MAX] = { PORT_1_ALM_ADDR,
		PORT_2_ALM_ADDR, PORT_3_ALM_ADDR, 
		PORT_4_ALM_ADDR, PORT_5_ALM_ADDR,
		PORT_6_ALM_ADDR, PORT_7_ALM_ADDR };
unsigned long __PORT_ALM_FLAG_ADDR[PORT_ID_EAG6L_MAX] = { PORT_1_ALM_FLAG_ADDR,
		PORT_2_ALM_FLAG_ADDR, PORT_3_ALM_FLAG_ADDR, 
		PORT_4_ALM_FLAG_ADDR, PORT_5_ALM_FLAG_ADDR,
		PORT_6_ALM_FLAG_ADDR, PORT_7_ALM_FLAG_ADDR };
unsigned long __PORT_ALM_MASK_ADDR[PORT_ID_EAG6L_MAX] = { PORT_1_ALM_MASK_ADDR,
		PORT_2_ALM_MASK_ADDR, PORT_3_ALM_MASK_ADDR, 
		PORT_4_ALM_MASK_ADDR, PORT_5_ALM_MASK_ADDR,
		PORT_6_ALM_MASK_ADDR, PORT_7_ALM_MASK_ADDR };
unsigned long __PORT_GET_CH_NUM_ADDR[PORT_ID_EAG6L_MAX] = { PORT_1_GET_CH_NUM_ADDR,
		PORT_2_GET_CH_NUM_ADDR, PORT_3_GET_CH_NUM_ADDR, 
		PORT_4_GET_CH_NUM_ADDR, PORT_5_GET_CH_NUM_ADDR,
		PORT_6_GET_CH_NUM_ADDR, PORT_7_GET_CH_NUM_ADDR };
unsigned long __PORT_SET_CH_NUM_ADDR[PORT_ID_EAG6L_MAX] = { PORT_1_SET_CH_NUM_ADDR,
		PORT_2_SET_CH_NUM_ADDR, PORT_3_SET_CH_NUM_ADDR, 
		PORT_4_SET_CH_NUM_ADDR, PORT_5_SET_CH_NUM_ADDR,
		PORT_6_SET_CH_NUM_ADDR, PORT_7_SET_CH_NUM_ADDR };
unsigned long __PORT_VENDOR1_ADDR[PORT_ID_EAG6L_MAX] = { SFP_P1_VENDOR1_ADDR,
		SFP_P2_VENDOR1_ADDR, SFP_P3_VENDOR1_ADDR, 
		SFP_P4_VENDOR1_ADDR, SFP_P5_VENDOR1_ADDR,
		SFP_P6_VENDOR1_ADDR, SFP_P7_VENDOR1_ADDR };
unsigned long __PORT_VENDOR2_ADDR[PORT_ID_EAG6L_MAX] = { SFP_P1_VENDOR2_ADDR,
		SFP_P2_VENDOR2_ADDR, SFP_P3_VENDOR2_ADDR, 
		SFP_P4_VENDOR2_ADDR, SFP_P5_VENDOR2_ADDR,
		SFP_P6_VENDOR2_ADDR, SFP_P7_VENDOR2_ADDR };
unsigned long __PORT_VENDOR3_ADDR[PORT_ID_EAG6L_MAX] = { SFP_P1_VENDOR3_ADDR,
		SFP_P2_VENDOR3_ADDR, SFP_P3_VENDOR3_ADDR, 
		SFP_P4_VENDOR3_ADDR, SFP_P5_VENDOR3_ADDR,
		SFP_P6_VENDOR3_ADDR, SFP_P7_VENDOR3_ADDR };
unsigned long __PORT_VENDOR4_ADDR[PORT_ID_EAG6L_MAX] = { SFP_P1_VENDOR4_ADDR,
		SFP_P2_VENDOR4_ADDR, SFP_P3_VENDOR4_ADDR, 
		SFP_P4_VENDOR4_ADDR, SFP_P5_VENDOR4_ADDR,
		SFP_P6_VENDOR4_ADDR, SFP_P7_VENDOR4_ADDR };
unsigned long __PORT_VENDOR5_ADDR[PORT_ID_EAG6L_MAX] = { SFP_P1_VENDOR5_ADDR,
		SFP_P2_VENDOR5_ADDR, SFP_P3_VENDOR5_ADDR, 
		SFP_P4_VENDOR5_ADDR, SFP_P5_VENDOR5_ADDR,
		SFP_P6_VENDOR5_ADDR, SFP_P7_VENDOR5_ADDR };
unsigned long __PORT_VENDOR6_ADDR[PORT_ID_EAG6L_MAX] = { SFP_P1_VENDOR6_ADDR,
		SFP_P2_VENDOR6_ADDR, SFP_P3_VENDOR6_ADDR, 
		SFP_P4_VENDOR6_ADDR, SFP_P5_VENDOR6_ADDR,
		SFP_P6_VENDOR6_ADDR, SFP_P7_VENDOR6_ADDR };
unsigned long __PORT_VENDOR7_ADDR[PORT_ID_EAG6L_MAX] = { SFP_P1_VENDOR7_ADDR,
		SFP_P2_VENDOR7_ADDR, SFP_P3_VENDOR7_ADDR, 
		SFP_P4_VENDOR7_ADDR, SFP_P5_VENDOR7_ADDR,
		SFP_P6_VENDOR7_ADDR, SFP_P7_VENDOR7_ADDR };
unsigned long __PORT_VENDOR8_ADDR[PORT_ID_EAG6L_MAX] = { SFP_P1_VENDOR8_ADDR,
		SFP_P2_VENDOR8_ADDR, SFP_P3_VENDOR8_ADDR, 
		SFP_P4_VENDOR8_ADDR, SFP_P5_VENDOR8_ADDR,
		SFP_P6_VENDOR8_ADDR, SFP_P7_VENDOR8_ADDR };
unsigned long __PORT_PN1_ADDR[PORT_ID_EAG6L_MAX] = { SFP_P1_PN1_ADDR,
		SFP_P2_PN1_ADDR, SFP_P3_PN1_ADDR, 
		SFP_P4_PN1_ADDR, SFP_P5_PN1_ADDR,
		SFP_P6_PN1_ADDR, SFP_P7_PN1_ADDR };
unsigned long __PORT_PN2_ADDR[PORT_ID_EAG6L_MAX] = { SFP_P1_PN2_ADDR,
		SFP_P2_PN2_ADDR, SFP_P3_PN2_ADDR, 
		SFP_P4_PN2_ADDR, SFP_P5_PN2_ADDR,
		SFP_P6_PN2_ADDR, SFP_P7_PN2_ADDR };
unsigned long __PORT_PN3_ADDR[PORT_ID_EAG6L_MAX] = { SFP_P1_PN3_ADDR,
		SFP_P2_PN3_ADDR, SFP_P3_PN3_ADDR, 
		SFP_P4_PN3_ADDR, SFP_P5_PN3_ADDR,
		SFP_P6_PN3_ADDR, SFP_P7_PN3_ADDR };
unsigned long __PORT_PN4_ADDR[PORT_ID_EAG6L_MAX] = { SFP_P1_PN4_ADDR,
		SFP_P2_PN4_ADDR, SFP_P3_PN4_ADDR, 
		SFP_P4_PN4_ADDR, SFP_P5_PN4_ADDR,
		SFP_P6_PN4_ADDR, SFP_P7_PN4_ADDR };
unsigned long __PORT_PN5_ADDR[PORT_ID_EAG6L_MAX] = { SFP_P1_PN5_ADDR,
		SFP_P2_PN5_ADDR, SFP_P3_PN5_ADDR, 
		SFP_P4_PN5_ADDR, SFP_P5_PN5_ADDR,
		SFP_P6_PN5_ADDR, SFP_P7_PN5_ADDR };
unsigned long __PORT_PN6_ADDR[PORT_ID_EAG6L_MAX] = { SFP_P1_PN6_ADDR,
		SFP_P2_PN6_ADDR, SFP_P3_PN6_ADDR, 
		SFP_P4_PN6_ADDR, SFP_P5_PN6_ADDR,
		SFP_P6_PN6_ADDR, SFP_P7_PN6_ADDR };
unsigned long __PORT_PN7_ADDR[PORT_ID_EAG6L_MAX] = { SFP_P1_PN7_ADDR,
		SFP_P2_PN7_ADDR, SFP_P3_PN7_ADDR, 
		SFP_P4_PN7_ADDR, SFP_P5_PN7_ADDR,
		SFP_P6_PN7_ADDR, SFP_P7_PN7_ADDR };
unsigned long __PORT_PN8_ADDR[PORT_ID_EAG6L_MAX] = { SFP_P1_PN8_ADDR,
		SFP_P2_PN8_ADDR, SFP_P3_PN8_ADDR, 
		SFP_P4_PN8_ADDR, SFP_P5_PN8_ADDR,
		SFP_P6_PN8_ADDR, SFP_P7_PN8_ADDR };
unsigned long __PORT_SN1_ADDR[PORT_ID_EAG6L_MAX] = { SFP_P1_SN1_ADDR,
		SFP_P2_SN1_ADDR, SFP_P3_SN1_ADDR, 
		SFP_P4_SN1_ADDR, SFP_P5_SN1_ADDR,
		SFP_P6_SN1_ADDR, SFP_P7_SN1_ADDR };
unsigned long __PORT_SN2_ADDR[PORT_ID_EAG6L_MAX] = { SFP_P1_SN2_ADDR,
		SFP_P2_SN2_ADDR, SFP_P3_SN2_ADDR, 
		SFP_P4_SN2_ADDR, SFP_P5_SN2_ADDR,
		SFP_P6_SN2_ADDR, SFP_P7_SN2_ADDR };
unsigned long __PORT_SN3_ADDR[PORT_ID_EAG6L_MAX] = { SFP_P1_SN3_ADDR,
		SFP_P2_SN3_ADDR, SFP_P3_SN3_ADDR, 
		SFP_P4_SN3_ADDR, SFP_P5_SN3_ADDR,
		SFP_P6_SN3_ADDR, SFP_P7_SN3_ADDR };
unsigned long __PORT_SN4_ADDR[PORT_ID_EAG6L_MAX] = { SFP_P1_SN4_ADDR,
		SFP_P2_SN4_ADDR, SFP_P3_SN4_ADDR, 
		SFP_P4_SN4_ADDR, SFP_P5_SN4_ADDR,
		SFP_P6_SN4_ADDR, SFP_P7_SN4_ADDR };
unsigned long __PORT_SN5_ADDR[PORT_ID_EAG6L_MAX] = { SFP_P1_SN5_ADDR,
		SFP_P2_SN5_ADDR, SFP_P3_SN5_ADDR, 
		SFP_P4_SN5_ADDR, SFP_P5_SN5_ADDR,
		SFP_P6_SN5_ADDR, SFP_P7_SN5_ADDR };
unsigned long __PORT_SN6_ADDR[PORT_ID_EAG6L_MAX] = { SFP_P1_SN6_ADDR,
		SFP_P2_SN6_ADDR, SFP_P3_SN6_ADDR, 
		SFP_P4_SN6_ADDR, SFP_P5_SN6_ADDR,
		SFP_P6_SN6_ADDR, SFP_P7_SN6_ADDR };
unsigned long __PORT_SN7_ADDR[PORT_ID_EAG6L_MAX] = { SFP_P1_SN7_ADDR,
		SFP_P2_SN7_ADDR, SFP_P3_SN7_ADDR, 
		SFP_P4_SN7_ADDR, SFP_P5_SN7_ADDR,
		SFP_P6_SN7_ADDR, SFP_P7_SN7_ADDR };
unsigned long __PORT_SN8_ADDR[PORT_ID_EAG6L_MAX] = { SFP_P1_SN8_ADDR,
		SFP_P2_SN8_ADDR, SFP_P3_SN8_ADDR, 
		SFP_P4_SN8_ADDR, SFP_P5_SN8_ADDR,
		SFP_P6_SN8_ADDR, SFP_P7_SN8_ADDR };
unsigned long __PORT_TEMP_ADDR[PORT_ID_EAG6L_MAX] = { SFP_P1_TEMP_ADDR,
		SFP_P2_TEMP_ADDR, SFP_P3_TEMP_ADDR, 
		SFP_P4_TEMP_ADDR, SFP_P5_TEMP_ADDR,
		SFP_P6_TEMP_ADDR, SFP_P7_TEMP_ADDR };
unsigned long __PORT_RATE_ADDR[PORT_ID_EAG6L_MAX] = { SFP_P1_RATE_ADDR,
		SFP_P2_RATE_ADDR, SFP_P3_RATE_ADDR, 
		SFP_P4_RATE_ADDR, SFP_P5_RATE_ADDR,
		SFP_P6_RATE_ADDR, SFP_P7_RATE_ADDR };
unsigned long __PORT_VOLT_ADDR[PORT_ID_EAG6L_MAX] = { SFP_P1_VOLT_ADDR,
		SFP_P2_VOLT_ADDR, SFP_P3_VOLT_ADDR, 
		SFP_P4_VOLT_ADDR, SFP_P5_VOLT_ADDR,
		SFP_P6_VOLT_ADDR, SFP_P7_VOLT_ADDR };
unsigned long __PORT_TX_BIAS_ADDR[PORT_ID_EAG6L_MAX] = { SFP_P1_TX_BIAS_ADDR,
		SFP_P2_TX_BIAS_ADDR, SFP_P3_TX_BIAS_ADDR, 
		SFP_P4_TX_BIAS_ADDR, SFP_P5_TX_BIAS_ADDR,
		SFP_P6_TX_BIAS_ADDR, SFP_P7_TX_BIAS_ADDR };
unsigned long __PORT_LTEMP_ADDR[PORT_ID_EAG6L_MAX] = { SFP_P1_LTEMP_ADDR,
		SFP_P2_LTEMP_ADDR, SFP_P3_LTEMP_ADDR, 
		SFP_P4_LTEMP_ADDR, SFP_P5_LTEMP_ADDR,
		SFP_P6_LTEMP_ADDR, SFP_P7_LTEMP_ADDR };
unsigned long __PORT_TCURR_ADDR[PORT_ID_EAG6L_MAX] = { SFP_P1_TCURR_ADDR,
		SFP_P2_TCURR_ADDR, SFP_P3_TCURR_ADDR, 
		SFP_P4_TCURR_ADDR, SFP_P5_TCURR_ADDR,
		SFP_P6_TCURR_ADDR, SFP_P7_TCURR_ADDR };
unsigned long __PORT_PM_TX_BYTE1_ADDR[PORT_ID_EAG6L_MAX] = { PM_P1_TX_BYTE1_ADDR,
		PM_P2_TX_BYTE1_ADDR, PM_P3_TX_BYTE1_ADDR, 
		PM_P4_TX_BYTE1_ADDR, PM_P5_TX_BYTE1_ADDR,
		PM_P6_TX_BYTE1_ADDR, PM_P7_TX_BYTE1_ADDR };
unsigned long __PORT_PM_TX_BYTE2_ADDR[PORT_ID_EAG6L_MAX] = { PM_P1_TX_BYTE2_ADDR,
		PM_P2_TX_BYTE2_ADDR, PM_P3_TX_BYTE2_ADDR, 
		PM_P4_TX_BYTE2_ADDR, PM_P5_TX_BYTE2_ADDR,
		PM_P6_TX_BYTE2_ADDR, PM_P7_TX_BYTE2_ADDR };
unsigned long __PORT_PM_TX_BYTE3_ADDR[PORT_ID_EAG6L_MAX] = { PM_P1_TX_BYTE3_ADDR,
		PM_P2_TX_BYTE3_ADDR, PM_P3_TX_BYTE3_ADDR, 
		PM_P4_TX_BYTE3_ADDR, PM_P5_TX_BYTE3_ADDR,
		PM_P6_TX_BYTE3_ADDR, PM_P7_TX_BYTE3_ADDR };
unsigned long __PORT_PM_TX_BYTE4_ADDR[PORT_ID_EAG6L_MAX] = { PM_P1_TX_BYTE4_ADDR,
		PM_P2_TX_BYTE4_ADDR, PM_P3_TX_BYTE4_ADDR, 
		PM_P4_TX_BYTE4_ADDR, PM_P5_TX_BYTE4_ADDR,
		PM_P6_TX_BYTE4_ADDR, PM_P7_TX_BYTE4_ADDR };
unsigned long __PORT_PM_RX_BYTE1_ADDR[PORT_ID_EAG6L_MAX] = { PM_P1_RX_BYTE1_ADDR,
		PM_P2_RX_BYTE1_ADDR, PM_P3_RX_BYTE1_ADDR, 
		PM_P4_RX_BYTE1_ADDR, PM_P5_RX_BYTE1_ADDR,
		PM_P6_RX_BYTE1_ADDR, PM_P7_RX_BYTE1_ADDR };
unsigned long __PORT_PM_RX_BYTE2_ADDR[PORT_ID_EAG6L_MAX] = { PM_P1_RX_BYTE2_ADDR,
		PM_P2_RX_BYTE2_ADDR, PM_P3_RX_BYTE2_ADDR, 
		PM_P4_RX_BYTE2_ADDR, PM_P5_RX_BYTE2_ADDR,
		PM_P6_RX_BYTE2_ADDR, PM_P7_RX_BYTE2_ADDR };
unsigned long __PORT_PM_RX_BYTE3_ADDR[PORT_ID_EAG6L_MAX] = { PM_P1_RX_BYTE3_ADDR,
		PM_P2_RX_BYTE3_ADDR, PM_P3_RX_BYTE3_ADDR, 
		PM_P4_RX_BYTE3_ADDR, PM_P5_RX_BYTE3_ADDR,
		PM_P6_RX_BYTE3_ADDR, PM_P7_RX_BYTE3_ADDR };
unsigned long __PORT_PM_RX_BYTE4_ADDR[PORT_ID_EAG6L_MAX] = { PM_P1_RX_BYTE4_ADDR,
		PM_P2_RX_BYTE4_ADDR, PM_P3_RX_BYTE4_ADDR, 
		PM_P4_RX_BYTE4_ADDR, PM_P5_RX_BYTE4_ADDR,
		PM_P6_RX_BYTE4_ADDR, PM_P7_RX_BYTE4_ADDR };
unsigned long __PORT_PM_TX_FRAME1_ADDR[PORT_ID_EAG6L_MAX] = { PM_P1_TX_FRAME1_ADDR,
		PM_P2_TX_FRAME1_ADDR, PM_P3_TX_FRAME1_ADDR, 
		PM_P4_TX_FRAME1_ADDR, PM_P5_TX_FRAME1_ADDR,
		PM_P6_TX_FRAME1_ADDR, PM_P7_TX_FRAME1_ADDR };
unsigned long __PORT_PM_TX_FRAME2_ADDR[PORT_ID_EAG6L_MAX] = { PM_P1_TX_FRAME2_ADDR,
		PM_P2_TX_FRAME2_ADDR, PM_P3_TX_FRAME2_ADDR, 
		PM_P4_TX_FRAME2_ADDR, PM_P5_TX_FRAME2_ADDR,
		PM_P6_TX_FRAME2_ADDR, PM_P7_TX_FRAME2_ADDR };
unsigned long __PORT_PM_TX_FRAME3_ADDR[PORT_ID_EAG6L_MAX] = { PM_P1_TX_FRAME3_ADDR,
		PM_P2_TX_FRAME3_ADDR, PM_P3_TX_FRAME3_ADDR, 
		PM_P4_TX_FRAME3_ADDR, PM_P5_TX_FRAME3_ADDR,
		PM_P6_TX_FRAME3_ADDR, PM_P7_TX_FRAME3_ADDR };
unsigned long __PORT_PM_TX_FRAME4_ADDR[PORT_ID_EAG6L_MAX] = { PM_P1_TX_FRAME4_ADDR,
		PM_P2_TX_FRAME4_ADDR, PM_P3_TX_FRAME4_ADDR, 
		PM_P4_TX_FRAME4_ADDR, PM_P5_TX_FRAME4_ADDR,
		PM_P6_TX_FRAME4_ADDR, PM_P7_TX_FRAME4_ADDR };
unsigned long __PORT_PM_RX_FRAME1_ADDR[PORT_ID_EAG6L_MAX] = { PM_P1_RX_FRAME1_ADDR,
		PM_P2_RX_FRAME1_ADDR, PM_P3_RX_FRAME1_ADDR, 
		PM_P4_RX_FRAME1_ADDR, PM_P5_RX_FRAME1_ADDR,
		PM_P6_RX_FRAME1_ADDR, PM_P7_RX_FRAME1_ADDR };
unsigned long __PORT_PM_RX_FRAME2_ADDR[PORT_ID_EAG6L_MAX] = { PM_P1_RX_FRAME2_ADDR,
		PM_P2_RX_FRAME2_ADDR, PM_P3_RX_FRAME2_ADDR, 
		PM_P4_RX_FRAME2_ADDR, PM_P5_RX_FRAME2_ADDR,
		PM_P6_RX_FRAME2_ADDR, PM_P7_RX_FRAME2_ADDR };
unsigned long __PORT_PM_RX_FRAME3_ADDR[PORT_ID_EAG6L_MAX] = { PM_P1_RX_FRAME3_ADDR,
		PM_P2_RX_FRAME3_ADDR, PM_P3_RX_FRAME3_ADDR, 
		PM_P4_RX_FRAME3_ADDR, PM_P5_RX_FRAME3_ADDR,
		PM_P6_RX_FRAME3_ADDR, PM_P7_RX_FRAME3_ADDR };
unsigned long __PORT_PM_RX_FRAME4_ADDR[PORT_ID_EAG6L_MAX] = { PM_P1_RX_FRAME4_ADDR,
		PM_P2_RX_FRAME4_ADDR, PM_P3_RX_FRAME4_ADDR, 
		PM_P4_RX_FRAME4_ADDR, PM_P5_RX_FRAME4_ADDR,
		PM_P6_RX_FRAME4_ADDR, PM_P7_RX_FRAME4_ADDR };
unsigned long __PORT_PM_FCS1_ADDR[PORT_ID_EAG6L_MAX] = { PM_P1_FCS1_ADDR,
		PM_P2_FCS1_ADDR, PM_P3_FCS1_ADDR, 
		PM_P4_FCS1_ADDR, PM_P5_FCS1_ADDR,
		PM_P6_FCS1_ADDR, PM_P7_FCS1_ADDR };
unsigned long __PORT_PM_FCS2_ADDR[PORT_ID_EAG6L_MAX] = { PM_P1_FCS2_ADDR,
		PM_P2_FCS2_ADDR, PM_P3_FCS2_ADDR, 
		PM_P4_FCS2_ADDR, PM_P5_FCS2_ADDR,
		PM_P6_FCS2_ADDR, PM_P7_FCS2_ADDR };
unsigned long __PORT_PM_FCS3_ADDR[PORT_ID_EAG6L_MAX] = { PM_P1_FCS3_ADDR,
		PM_P2_FCS3_ADDR, PM_P3_FCS3_ADDR, 
		PM_P4_FCS3_ADDR, PM_P5_FCS3_ADDR,
		PM_P6_FCS3_ADDR, PM_P7_FCS3_ADDR };
unsigned long __PORT_PM_FCS4_ADDR[PORT_ID_EAG6L_MAX] = { PM_P1_FCS4_ADDR,
		PM_P2_FCS4_ADDR, PM_P3_FCS4_ADDR, 
		PM_P4_FCS4_ADDR, PM_P5_FCS4_ADDR,
		PM_P6_FCS4_ADDR, PM_P7_FCS4_ADDR };
unsigned long __PORT_PM_FCS_OK1_ADDR[PORT_ID_EAG6L_MAX] = { PM_P1_FCS_OK1_ADDR,
		PM_P2_FCS_OK1_ADDR, PM_P3_FCS_OK1_ADDR, 
		PM_P4_FCS_OK1_ADDR, PM_P5_FCS_OK1_ADDR,
		PM_P6_FCS_OK1_ADDR, PM_P7_FCS_OK1_ADDR };
unsigned long __PORT_PM_FCS_OK2_ADDR[PORT_ID_EAG6L_MAX] = { PM_P1_FCS_OK2_ADDR,
		PM_P2_FCS_OK2_ADDR, PM_P3_FCS_OK2_ADDR, 
		PM_P4_FCS_OK2_ADDR, PM_P5_FCS_OK2_ADDR,
		PM_P6_FCS_OK2_ADDR, PM_P7_FCS_OK2_ADDR };
unsigned long __PORT_PM_FCS_OK3_ADDR[PORT_ID_EAG6L_MAX] = { PM_P1_FCS_OK3_ADDR,
		PM_P2_FCS_OK3_ADDR, PM_P3_FCS_OK3_ADDR, 
		PM_P4_FCS_OK3_ADDR, PM_P5_FCS_OK3_ADDR,
		PM_P6_FCS_OK3_ADDR, PM_P7_FCS_OK3_ADDR };
unsigned long __PORT_PM_FCS_OK4_ADDR[PORT_ID_EAG6L_MAX] = { PM_P1_FCS_OK4_ADDR,
		PM_P2_FCS_OK4_ADDR, PM_P3_FCS_OK4_ADDR, 
		PM_P4_FCS_OK4_ADDR, PM_P5_FCS_OK4_ADDR,
		PM_P6_FCS_OK4_ADDR, PM_P7_FCS4_ADDR };
unsigned long __PORT_PM_FCS_NOK1_ADDR[PORT_ID_EAG6L_MAX] = { PM_P1_FCS_NOK1_ADDR,
		PM_P2_FCS_NOK1_ADDR, PM_P3_FCS_NOK1_ADDR, 
		PM_P4_FCS_NOK1_ADDR, PM_P5_FCS_NOK1_ADDR,
		PM_P6_FCS_NOK1_ADDR, PM_P7_FCS_NOK1_ADDR };
unsigned long __PORT_PM_FCS_NOK2_ADDR[PORT_ID_EAG6L_MAX] = { PM_P1_FCS_NOK2_ADDR,
		PM_P2_FCS_NOK2_ADDR, PM_P3_FCS_NOK2_ADDR, 
		PM_P4_FCS_NOK2_ADDR, PM_P5_FCS_NOK2_ADDR,
		PM_P6_FCS_NOK2_ADDR, PM_P7_FCS_NOK2_ADDR };
unsigned long __PORT_PM_FCS_NOK3_ADDR[PORT_ID_EAG6L_MAX] = { PM_P1_FCS_NOK3_ADDR,
		PM_P2_FCS_NOK3_ADDR, PM_P3_FCS_NOK3_ADDR, 
		PM_P4_FCS_NOK3_ADDR, PM_P5_FCS_NOK3_ADDR,
		PM_P6_FCS_NOK3_ADDR, PM_P7_FCS_NOK3_ADDR };
unsigned long __PORT_PM_FCS_NOK4_ADDR[PORT_ID_EAG6L_MAX] = { PM_P1_FCS_NOK4_ADDR,
		PM_P2_FCS_NOK4_ADDR, PM_P3_FCS_NOK4_ADDR, 
		PM_P4_FCS_NOK4_ADDR, PM_P5_FCS_NOK4_ADDR,
		PM_P6_FCS_NOK4_ADDR, PM_P7_FCS4_ADDR };
unsigned long __PORT_CLEI1_ADDR[PORT_ID_EAG6L_MAX] = { PORT1_CLEI1_ADDR,
		PORT2_CLEI1_ADDR, PORT3_CLEI1_ADDR, 
		PORT4_CLEI1_ADDR, PORT5_CLEI1_ADDR,
		PORT6_CLEI1_ADDR, PORT7_CLEI1_ADDR };
unsigned long __PORT_CLEI2_ADDR[PORT_ID_EAG6L_MAX] = { PORT1_CLEI2_ADDR,
		PORT2_CLEI2_ADDR, PORT3_CLEI2_ADDR, 
		PORT4_CLEI2_ADDR, PORT5_CLEI2_ADDR,
		PORT6_CLEI2_ADDR, PORT7_CLEI2_ADDR };
unsigned long __PORT_CLEI3_ADDR[PORT_ID_EAG6L_MAX] = { PORT1_CLEI3_ADDR,
		PORT2_CLEI3_ADDR, PORT3_CLEI3_ADDR, 
		PORT4_CLEI3_ADDR, PORT5_CLEI3_ADDR,
		PORT6_CLEI3_ADDR, PORT7_CLEI3_ADDR };
unsigned long __PORT_CLEI4_ADDR[PORT_ID_EAG6L_MAX] = { PORT1_CLEI4_ADDR,
		PORT2_CLEI4_ADDR, PORT3_CLEI4_ADDR, 
		PORT4_CLEI4_ADDR, PORT5_CLEI4_ADDR,
		PORT6_CLEI4_ADDR, PORT7_CLEI4_ADDR };
unsigned long __PORT_CLEI5_ADDR[PORT_ID_EAG6L_MAX] = { PORT1_CLEI5_ADDR,
		PORT2_CLEI5_ADDR, PORT3_CLEI5_ADDR, 
		PORT4_CLEI5_ADDR, PORT5_CLEI5_ADDR,
		PORT6_CLEI5_ADDR, PORT7_CLEI5_ADDR };
unsigned long __PORT_USI1_ADDR[PORT_ID_EAG6L_MAX] = { PORT1_USI1_ADDR,
		PORT2_USI1_ADDR, PORT3_USI1_ADDR, 
		PORT4_USI1_ADDR, PORT5_USI1_ADDR,
		PORT6_USI1_ADDR, PORT7_USI1_ADDR };
unsigned long __PORT_USI2_ADDR[PORT_ID_EAG6L_MAX] = { PORT1_USI2_ADDR,
		PORT2_USI2_ADDR, PORT3_USI2_ADDR, 
		PORT4_USI2_ADDR, PORT5_USI2_ADDR,
		PORT6_USI2_ADDR, PORT7_USI2_ADDR };
unsigned long __PORT_USI3_ADDR[PORT_ID_EAG6L_MAX] = { PORT1_USI3_ADDR,
		PORT2_USI3_ADDR, PORT3_USI3_ADDR, 
		PORT4_USI3_ADDR, PORT5_USI3_ADDR,
		PORT6_USI3_ADDR, PORT7_USI3_ADDR };
unsigned long __PORT_USI4_ADDR[PORT_ID_EAG6L_MAX] = { PORT1_USI4_ADDR,
		PORT2_USI4_ADDR, PORT3_USI4_ADDR, 
		PORT4_USI4_ADDR, PORT5_USI4_ADDR,
		PORT6_USI4_ADDR, PORT7_USI4_ADDR };
unsigned long __PORT_USI5_ADDR[PORT_ID_EAG6L_MAX] = { PORT1_USI5_ADDR,
		PORT2_USI5_ADDR, PORT3_USI5_ADDR, 
		PORT4_USI5_ADDR, PORT5_USI5_ADDR,
		PORT6_USI5_ADDR, PORT7_USI5_ADDR };
unsigned long __PORT_USI6_ADDR[PORT_ID_EAG6L_MAX] = { PORT1_USI6_ADDR,
		PORT2_USI6_ADDR, PORT3_USI6_ADDR, 
		PORT4_USI6_ADDR, PORT5_USI6_ADDR,
		PORT6_USI6_ADDR, PORT7_USI6_ADDR };
unsigned long __PORT_USI7_ADDR[PORT_ID_EAG6L_MAX] = { PORT1_USI7_ADDR,
		PORT2_USI7_ADDR, PORT3_USI7_ADDR, 
		PORT4_USI7_ADDR, PORT5_USI7_ADDR,
		PORT6_USI7_ADDR, PORT7_USI7_ADDR };
unsigned long __PORT_USI8_ADDR[PORT_ID_EAG6L_MAX] = { PORT1_USI8_ADDR,
		PORT2_USI8_ADDR, PORT3_USI8_ADDR, 
		PORT4_USI8_ADDR, PORT5_USI8_ADDR,
		PORT6_USI8_ADDR, PORT7_USI8_ADDR };
unsigned long __PORT_USI9_ADDR[PORT_ID_EAG6L_MAX] = { PORT1_USI9_ADDR,
		PORT2_USI9_ADDR, PORT3_USI9_ADDR, 
		PORT4_USI9_ADDR, PORT5_USI9_ADDR,
		PORT6_USI9_ADDR, PORT7_USI9_ADDR };
unsigned long __PORT_USI10_ADDR[PORT_ID_EAG6L_MAX] = { PORT1_USI10_ADDR,
		PORT2_USI10_ADDR, PORT3_USI10_ADDR, 
		PORT4_USI10_ADDR, PORT5_USI10_ADDR,
		PORT6_USI10_ADDR, PORT7_USI10_ADDR };
unsigned long __PORT_USI11_ADDR[PORT_ID_EAG6L_MAX] = { PORT1_USI11_ADDR,
		PORT2_USI11_ADDR, PORT3_USI11_ADDR, 
		PORT4_USI11_ADDR, PORT5_USI11_ADDR,
		PORT6_USI11_ADDR, PORT7_USI11_ADDR };
unsigned long __PORT_USI12_ADDR[PORT_ID_EAG6L_MAX] = { PORT1_USI12_ADDR,
		PORT2_USI12_ADDR, PORT3_USI12_ADDR, 
		PORT4_USI12_ADDR, PORT5_USI12_ADDR,
		PORT6_USI12_ADDR, PORT7_USI12_ADDR };
unsigned long __PORT_USI13_ADDR[PORT_ID_EAG6L_MAX] = { PORT1_USI13_ADDR,
		PORT2_USI13_ADDR, PORT3_USI13_ADDR, 
		PORT4_USI13_ADDR, PORT5_USI13_ADDR,
		PORT6_USI13_ADDR, PORT7_USI13_ADDR };


long hw_update_port_inventory(int portno, struct module_inventory * mod_inv)
{
#if 0//PWY_FIXME
    if('\0' == mod_inv->serial_num[0] || mod_inv->dist == 0xFFFF)
#endif //PWY_FIXME
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
		
    zlog_notice("get_port_sfp_reg : port=%d addr=%x type=%x, sts=%x\n", portId, addr, type, sts);
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

#if 0//PWY_FIXME
	zlog_notice(" PORT[%d] %d cwhan_check %4.2f [%4.2f-%4.2f-%4.2f] [%4.2f-%4.2f-%4.2f] \n",
		portno, nidx, PORT_STATUS[portno].tx_pwr,
		PORT_STATUS[portno].tx_avg, PORT_STATUS[portno].tx_min, PORT_STATUS[portno].tx_max,
		port_pwr_1min[p_idx][1][nidx], port_pwr_1min[p_idx][1][nidx +1],
		port_pwr_1min[p_idx][1][nidx +2]);
#endif //PWY_FIXME
	return;
}

void update_sfp(void)
{
	int portno;

	for(portno = PORT_ID_EAG6L_PORT1; portno < PORT_ID_EAG6L_MAX; portno++) {
		/* skip if port has not installed sfp. */
		/* FIXME */
zlog_notice(" update_sfp PORT[%d]\n", portno);/*ZZPP*/
		get_sfp_info_diag(portno, &(PORT_STATUS[portno]));
		update_port_rx_power(portno);
		hw_update_port_inventory(portno, &(INV_TBL[portno]));
	}
	return;
}


u32 MCU_INIT_COMPLETE_FLAG;
u32 MCU_KEEP_ALIVE_RETRY_COUNT;

#define MCU_KEEP_ALIVE_RETRY_MAX	15

int port_config_set_laser(u32 portno, u32 enable)
{
	/* use i2c to set sfp tx power on/off. */
	return 0;
}

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
	/* use i2c to set esmc enable/disable. */
	/* use marvell sdk to set esmc enable/disable. */
	return 0;
}

int update_port_sfp_information(void)
{
	/* use i2c to access sfp information. */
	return 0;
}

void update_reg(void)
{
	struct inventory inv = { "hfrnet1234", "EAG6L12345", "PN123456789ABCD", 
		                     "SN12", "RV12", "240520010203", "2405200102", "RP01",
		                     "SNUIEAG6LL", "" };
	u16 ii, temp, pri_port, sec_port, bitmask;
	u16 val[20];

zlog_notice(" update_reg\n");/*ZZPP*/
	/***********************************************/
	/* check if mcu keep alive counter is ticking. */
	temp = FPGA_READ(HW_KEEP_ALIVE_1_ADDR);
	if(MCU_KEEP_ALIVE_CACHE == temp) {
		if(MCU_KEEP_ALIVE_RETRY_COUNT++ >= MCU_KEEP_ALIVE_RETRY_MAX) {
			zlog_notice("MCU dead ??\n");
			/* FIXME : call alarm function. */
		}
	} else if(temp && (temp > MCU_KEEP_ALIVE_CACHE))
		MCU_KEEP_ALIVE_CACHE = temp;

	/***********************************************/
	/* increase BP's h/w keep alive counter. */
	temp = FPGA_READ(HW_KEEP_ALIVE_1_ADDR);
	FPGA_WRITE(HW_KEEP_ALIVE_1_ADDR, temp + 1);
#if 1//PWY_FIXME
	if(temp == FPGA_READ(HW_KEEP_ALIVE_1_ADDR))
		zlog_notice("Stucked Keep-alive : before[%d] after[%d]\n", 
			temp, FPGA_READ(HW_KEEP_ALIVE_1_ADDR));
#endif //PWY_FIXME

	if(! INIT_COMPLETE_FLAG)
		return;

	/***********************************************/
	/* check if MCU init complete. */
	temp = FPGA_READ(INIT_COMPLETE_ADDR);
	MCU_INIT_COMPLETE_FLAG = ((temp & 0xFF) == 0xAA) ? 1 : 0;
	/* set init complete register. */
	temp |= 0xAA00;
	FPGA_WRITE(INIT_COMPLETE_ADDR, temp);

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

#if 1//PWY_FIXME
	/***********************************************/
	/* update inventory. */
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

	val[0] = FPGA_READ(INV_HW_MANU_1_ADDR);
	val[1] = FPGA_READ(INV_HW_MANU_2_ADDR);
	val[2] = FPGA_READ(INV_HW_MANU_3_ADDR);
	val[3] = FPGA_READ(INV_HW_MANU_4_ADDR);
	val[4] = FPGA_READ(INV_HW_MANU_5_ADDR);

	zlog_notice("INVENTORY: MANUFACTURE [%04x %04x %04x %04x %04x].\n", 
		val[0], val[1], val[2], val[3], val[4]);
#endif //PWY_FIXME

	/***********************************************/
	/* check/configure ports. */
	for(ii = PORT_ID_EAG6L_PORT1; ii < PORT_ID_EAG6L_MAX; ii++) {
		temp = FPGA_READ(__PORT_CONFIG_ADDR[ii]);

		/* skip if value is not changed. */
		if(temp == PORT_CONFIG_CACHE[ii])
			continue;

		/* get changed bit mask */
		bitmask = temp ^ PORT_CONFIG_CACHE[ii];

		/* config port laser on/off. */
		if(bitmask & 0xFF00) {
			if((temp & 0xFF00) == 0xA500)
				port_config_set_laser(ii, 1);
			if((temp & 0xFF00) == 0x5A00)
				port_config_set_laser(ii, 0);
		}

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

	/***********************************************/
	/* read per-port spf port status/control. */
	/* FIXME : call fuction. */
	update_port_sfp_information();


	/***********************************************/
	/* read per-port performance info from sdk. */
	update_port_pm_counters();
}
