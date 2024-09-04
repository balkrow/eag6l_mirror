#ifndef _BP_REGS_H_
#define _BP_REGS_H_

#if 1/*[#39] Adding Smart T-SFP related functions, dustin, 2024-06-12 */
typedef enum {
    SFP_ID_UNKNOWN = 0,
    SFP_ID_SMART_DUPLEX_TSFP = 4,
    SFP_ID_CU_SFP = 5,
    SFP_ID_SMART_BIDI_TSFP_COT = 6,
    SFP_ID_SMART_BIDI_TSFP_RT = 7,
    SFP_ID_VCSEL_BIDI = 8,
    SFP_ID_6WL = 9,
    SFP_ID_HSFP_HIGH = 10/*0xA*/,
    SFP_ID_HSFP_LOW = 11/*0xB*/,
    SFP_ID_CWDM = 12/*0xC*/,
    SFP_ID_DWDM = 13/*0xD*/,
    SFP_ID_VCSEL = 14/*0xE*/,
    SFP_ID_DWDM_TUNABLE = 15/*0xF*/,
} ePrivateSfpId;
#endif

#if 1/*[#48] register monitoring and update 관련 기능 추가, balkrow, 2024-06-10*/ 
#if 1/*[#65] Adding regMon simulation feature under ACCESS_SIM, dustin, 2024-06-24 */
#if 1 /* [#93] Adding for FPGA FW Bank Select and Error handling, dustin, 2024-08-12 */
typedef uint16_t (*READ_FUNC) (uint32_t, uint8_t);
#else
typedef uint16_t (*READ_FUNC) (uint16_t, uint8_t);
#endif
#else
typedef uint16_t (*READ_FUNC) (uint16_t);
#endif
#if 1/*[#51] Adding register callback templates for config/command registers, dustin, 2024-06-12 */
typedef uint16_t (*REG_CALLBACK) (uint16_t/*portno*/, uint16_t/*reg-val*/);
#else
typedef uint16_t (*REG_CALLBACK) (uint16_t);
#endif

typedef struct reg_mon_str
{
#if 1 /* [#91] Fixing for register updating feature, dustin, 2024-08-05 */
	uint32_t reg;
	uint16_t mask;
	uint16_t shift;
	uint16_t val;
#else
	uint16_t reg;
	uint16_t mask;
	uint16_t shift;
	uint16_t val;
#endif
#if 1/*[#51] Adding register callback templates for config/command registers, dustin, 2024-06-12 */
	uint8_t  portno;/*if zero, then single register. if non-zero then per-port registers. */
#endif
#if 1 /* [#62] SFP eeprom 및 register update 기능 단위 검증 및 디버깅, balkrow, 2024-06-21 */ 
	uint16_t rollback_val;
	struct thread *rb_thread;
#endif
	READ_FUNC func;
	REG_CALLBACK cb;
} RegMON;
#endif

#if 1/*[#53] Clock source status 업데이트 기능 추가, balkrow, 2024-06-13*/
#define FPGA_BASE 0x60000000
#define CPLD_BASE 0x70000000
#endif
#if 1/*[#56] register update timer 수정, balkrow, 2023-06-13 */
#define PORT_BASE 0x800000
#define DPRAM_BASE 0x500000
#endif

#if 1/* [#72] Adding omitted rtWDM related registers, dustin, 2024-06-27 */
#define NULL_REG_ADDR	0xFFFF
#endif

#if 1/*[#56] register update timer 수정, balkrow, 2023-06-13 */
/* Start DPRAM registers*/
#if 1/* [#70] Adding RDL feature, dustin, 2024-07-02 */
#define DPRAM_RDL_STATE RDL_STATE_RESP_ADDR
#else
#define DPRAM_IMG_HDR 0x0
#define DPRAM_RDL_STATE 0x2
#endif
/* End DPRAM registers*/
#endif

#if 1/*[#80] eag6l board SW bring-up, balkrow, 2023-07-25 */
typedef enum {
#if 1 /* [#102] Fixing some register updates, dustin, 2024-08-26 */
	M_PORT1 = 0x01,
	M_PORT2 = 0x02,
	M_PORT3 = 0x03,
	M_PORT4 = 0x04,
	M_PORT5 = 0x05,
	M_PORT6 = 0x06,
	M_PORT7 = 0x07
#else
	M_PORT1 = 0x11,
	M_PORT2 = 0x12,
	M_PORT3 = 0x13,
	M_PORT4 = 0x14,
	M_PORT5 = 0x21,
	M_PORT6 = 0x22,
	M_PORT7 = 0x23
#endif
} MCU_SIDE_PORT_NUM;

typedef enum {
	C_PORT1 = 0,
	C_PORT2 = 8,
	C_PORT3 = 16,
	C_PORT4 = 24,
	C_PORT5 = 32,
	C_PORT6 = 40,
	C_PORT7 = 50
} CARD_SIDE_PORT_NUM;
#endif
/****************************
 * unit related registers
 ****************************/

#define SW_VERSION_ADDR			0x00
#if 1 /* [#62] SFP eeprom 및 register update 기능 단위 검증 및 디버깅, balkrow, 2024-06-21 */ 
#define CPU_FAIL_ADDR			0x12	
#define CPU_FAIL_MASK			0x100
#endif
#define HW_KEEP_ALIVE_1_ADDR		0x14
#define HW_KEEP_ALIVE_2_ADDR		0x16
                                           
#define INIT_COMPLETE_ADDR		0x18	/* bp set 0xAA to [15:8] if init done.*/
#if 1/*[#53] Clock source status 업데이트 기능 추가, balkrow, 2024-06-13*/
#if 1 /*[#82] eag6l board SW Debugging, balkrow, 2024-08-07*/
#define INIT_COMPLETE_ADDR_MASK		0xFF /*LSB write for because FPGA someting*/
#endif
#endif

#if 1/*[#106] init 시 FPGA update 기능 추가, balkrow, 2024-08-28 */
#define FPGA_VER_ADDR			0xe
#define FPGA_SWITCH_ADDR		0x1c
#define FPGA_SWITCH_CONFIRM		0x10
#endif

#if 1 /*[#62] SFP eeprom 및 register update 기능 단위 검증 및 디버깅, balkrow, 2024-06-21 */ 
#define COMMON_CTRL2_SIG_MASK		0x30
#define COMMON_CTRL2_SIG_SHIFT		0x4
#if 1/*[#110] RDL function Debugging 및 수정, balkrow, 2024-09-02*/
#define COMMON_CTRL2_RATE_MASK		0xf
#endif
#define COMMON_CTRL2_RATE_SHIFT		0x0
#endif
#define COMMON_CTRL2_P1_ADDR		0x20
#define COMMON_CTRL2_P2_ADDR		0x22
#define COMMON_CTRL2_P3_ADDR		0x24
#define COMMON_CTRL2_P4_ADDR		0x26
#define COMMON_CTRL2_P5_ADDR		0x28
#define COMMON_CTRL2_P6_ADDR		0x2A


/****************************
 * Reset/CR related registers
 ****************************/

#if 1/*[#65] Adding regMon simulation feature under ACCESS_SIM, dustin, 2024-06-24 */
/* Fixing wrong address */
#define CHIP_RESET_ADDR			0x40
#else
#define CHIP_RESET_ADDR			0x20	
#endif


/****************************
 * DCO related registers
 ****************************/

#define DCO_INTERFACE_ADDR		0x60
#define DCO_WRITE_CMD_ADDR		0x62
#define DCO_READ_CMD_ADDR		0x64
#define DCO_ACCESS_METHOD_ADDR		0x66


/****************************
 * DCO related registers
 ****************************/

/* H/W manufacture */
#define INV_HW_MANU_1_ADDR			0x370	
#define INV_HW_MANU_2_ADDR			0x372	
#define INV_HW_MANU_3_ADDR			0x374	
#define INV_HW_MANU_4_ADDR			0x376	
#define INV_HW_MANU_5_ADDR			0x378	

/* H/W model */
#define INV_HW_MODEL_1_ADDR			0x380	
#define INV_HW_MODEL_2_ADDR			0x382	
#define INV_HW_MODEL_3_ADDR			0x384	
#define INV_HW_MODEL_4_ADDR			0x386	
#define INV_HW_MODEL_5_ADDR			0x388	

/* H/W part number */
#define INV_HW_PN_1_ADDR			0x390	
#define INV_HW_PN_2_ADDR			0x392	
#define INV_HW_PN_3_ADDR			0x394	
#define INV_HW_PN_4_ADDR			0x396	
#define INV_HW_PN_5_ADDR			0x398	
#define INV_HW_PN_6_ADDR			0x39A	
#define INV_HW_PN_7_ADDR			0x39C	
#define INV_HW_PN_8_ADDR			0x39E	

/* H/W serial number */
#define INV_HW_SN_1_ADDR			0x3A0	
#define INV_HW_SN_2_ADDR			0x3A2	
#if 1/* [#78] Adding system inventory management, dustin, 2024-07-24 */
#define INV_HW_SN_3_ADDR			0x3A4	
#define INV_HW_SN_4_ADDR			0x3A6	
#define INV_HW_SN_5_ADDR			0x3A8	
#define INV_HW_SN_6_ADDR			0x3AA	
#define INV_HW_SN_7_ADDR			0x3AC	
#define INV_HW_SN_8_ADDR			0x3AE	
#endif

/* H/W revision number */
#define INV_HW_REV_1_ADDR			0x3B0	
#define INV_HW_REV_2_ADDR			0x3B2	

/* H/W manufacture date */
#define INV_HW_MDATE_1_ADDR			0x3B4	
#define INV_HW_MDATE_2_ADDR			0x3B6	
#define INV_HW_MDATE_3_ADDR			0x3B8	
#define INV_HW_MDATE_4_ADDR			0x3BA	
#define INV_HW_MDATE_5_ADDR			0x3BC	
#if 0/* [#78] Adding system inventory management, dustin, 2024-07-24 */
#define INV_HW_MDATE_6_ADDR			0x3BE
#endif

/* H/W repair date */
#define INV_HW_RDATE_1_ADDR			0x3C0	 
#define INV_HW_RDATE_2_ADDR			0x3C2	 
#define INV_HW_RDATE_3_ADDR			0x3C4	 
#define INV_HW_RDATE_4_ADDR			0x3C6	 
#define INV_HW_RDATE_5_ADDR			0x3C8	 

/* H/W repair code */
#define INV_HW_RCODE_1_ADDR			0x3CA	 
#define INV_HW_RCODE_2_ADDR			0x3CC	 

#if 1/*[#56] register update timer 수정, balkrow, 2023-06-13 */

#define BD_SFP_CR_ADDR				0x4C

#endif

/****************************
 * Port Config related registers
 ****************************/
/*PORT BASE 6080_xxxx*/
#define PORT_1_CONF_ADDR			0x00	
#define PORT_2_CONF_ADDR			0x02	
#define PORT_3_CONF_ADDR			0x04	
#define PORT_4_CONF_ADDR			0x06	
#define PORT_5_CONF_ADDR			0x08	
#define PORT_6_CONF_ADDR			0x0A	
#define PORT_7_CONF_ADDR			0x0C	

#define PORT_7_CONF2_ADDR			0x0E	
#define PORT_7_CONF3_ADDR			0x1E	

/****************************
 * SyncE related registers
 ****************************/

#define SYNCE_GCONFIG_ADDR			0x160
#if 1 /* [#62] SFP eeprom 및 register update 기능 단위 검증 및 디버깅, balkrow, 2024-06-21 */ 
#define SYNCE_GCONFIG_MASK			0xff	 
#endif
#define SYNCE_IF_SELECT_ADDR			0x162	 
#define SYNCE_ESMC_LQL_ADDR			0x166	 
#define SYNCE_ESMC_SQL_ADDR			0x168	 
#define SYNCE_ESMC_RQL_ADDR			0x16A	 
#define SYNCE_SRC_STAT_ADDR			0x16C	 
#if 1/*[#53] Clock source status 업데이트 기능 추가, balkrow, 2024-06-13*/
#define SYNCE_SRC_STAT_ADDR_MASK		0xFF	
#endif

#define SYNCE_ESMC_RQL2_ADDR			0x16E	
#define SYNCE_ESMC_RQL3_ADDR			0x170	
#define SYNCE_ESMC_RQL4_ADDR			0x172	
#define SYNCE_ESMC_RQL5_ADDR			0x174	


/****************************
 * SFP Port Status/Control related registers
 ****************************/
#define PORT_1_TX_PWR_ADDR		0x20	
#define PORT_2_TX_PWR_ADDR		0x40	
#define PORT_3_TX_PWR_ADDR		0x60	
#define PORT_4_TX_PWR_ADDR		0x80	
#define PORT_5_TX_PWR_ADDR		0xA0	
#define PORT_6_TX_PWR_ADDR		0xC0	
#define PORT_7_TX_PWR_ADDR		0xE0	

#define PORT_1_RX_PWR_ADDR		0x22	
#define PORT_2_RX_PWR_ADDR		0x42	
#define PORT_3_RX_PWR_ADDR		0x62	
#define PORT_4_RX_PWR_ADDR		0x82	
#define PORT_5_RX_PWR_ADDR		0xA2	
#define PORT_6_RX_PWR_ADDR		0xC2	
#define PORT_7_RX_PWR_ADDR		0xE2	

#define PORT_1_WL1_ADDR			0x24	
#define PORT_2_WL1_ADDR			0x44	
#define PORT_3_WL1_ADDR			0x64	
#define PORT_4_WL1_ADDR			0x84	
#define PORT_5_WL1_ADDR			0xA4	
#define PORT_6_WL1_ADDR			0xC4	
#define PORT_7_WL1_ADDR			0xE4	

#define PORT_1_WL2_ADDR			0x26	
#define PORT_2_WL2_ADDR			0x46	
#define PORT_3_WL2_ADDR			0x66	
#define PORT_4_WL2_ADDR			0x86	
#define PORT_5_WL2_ADDR			0xA6	
#define PORT_6_WL2_ADDR			0xC6	
#define PORT_7_WL2_ADDR			0xE6	

#define PORT_1_DIST_ADDR		0x2C	
#define PORT_2_DIST_ADDR		0x4C	
#define PORT_3_DIST_ADDR		0x6C	
#define PORT_4_DIST_ADDR		0x8C	
#define PORT_5_DIST_ADDR		0xAC	
#define PORT_6_DIST_ADDR		0xCC	
#define PORT_7_DIST_ADDR		0xEC	

#if 1/* [#72] Adding omitted rtWDM related registers, dustin, 2024-06-27 */
#define PORT_1_TX_PWR_RTWDM_ADDR		0x120	
#define PORT_2_TX_PWR_RTWDM_ADDR		0x140	
#define PORT_3_TX_PWR_RTWDM_ADDR		0x160	
#define PORT_4_TX_PWR_RTWDM_ADDR		0x180	
#define PORT_5_TX_PWR_RTWDM_ADDR		0x1A0	
#define PORT_6_TX_PWR_RTWDM_ADDR		0x1C0	

#define PORT_1_RX_PWR_RTWDM_ADDR		0x122	
#define PORT_2_RX_PWR_RTWDM_ADDR		0x142	
#define PORT_3_RX_PWR_RTWDM_ADDR		0x162	
#define PORT_4_RX_PWR_RTWDM_ADDR		0x182	
#define PORT_5_RX_PWR_RTWDM_ADDR		0x1A2	
#define PORT_6_RX_PWR_RTWDM_ADDR		0x1C2	

#define PORT_1_WL1_RTWDM_ADDR			0x124	
#define PORT_2_WL1_RTWDM_ADDR			0x144	
#define PORT_3_WL1_RTWDM_ADDR			0x164	
#define PORT_4_WL1_RTWDM_ADDR			0x184	
#define PORT_5_WL1_RTWDM_ADDR			0x1A4	
#define PORT_6_WL1_RTWDM_ADDR			0x1C4	

#define PORT_1_WL2_RTWDM_ADDR			0x126	
#define PORT_2_WL2_RTWDM_ADDR			0x146	
#define PORT_3_WL2_RTWDM_ADDR			0x166	
#define PORT_4_WL2_RTWDM_ADDR			0x186	
#define PORT_5_WL2_RTWDM_ADDR			0x1A6	
#define PORT_6_WL2_RTWDM_ADDR			0x1C6	

#define PORT_1_DIST_RTWDM_ADDR			0x12C	
#define PORT_2_DIST_RTWDM_ADDR			0x14C	
#define PORT_3_DIST_RTWDM_ADDR			0x16C	
#define PORT_4_DIST_RTWDM_ADDR			0x18C	
#define PORT_5_DIST_RTWDM_ADDR			0x1AC	
#define PORT_6_DIST_RTWDM_ADDR			0x1CC	
#endif/*[#72]*/

#define PORT_1_STSFP_STAT_ADDR		0x2E	
#define PORT_2_STSFP_STAT_ADDR		0x4E	
#define PORT_3_STSFP_STAT_ADDR		0x6E	
#define PORT_4_STSFP_STAT_ADDR		0x8E	
#define PORT_5_STSFP_STAT_ADDR		0xAE	
#define PORT_6_STSFP_STAT_ADDR		0xCE	
#if 0 /* [#91] Fixing for register updating feature, dustin, 2024-08-05 */
#define PORT_7_STSFP_STAT_ADDR		0xEE	
#endif

#define PORT_1_ALM_ADDR			0x30	
#define PORT_2_ALM_ADDR			0x50	
#define PORT_3_ALM_ADDR			0x70	
#define PORT_4_ALM_ADDR			0x90	
#define PORT_5_ALM_ADDR			0xB0	
#define PORT_6_ALM_ADDR			0xD0	
#define PORT_7_ALM_ADDR			0xF0	

#define PORT_1_ALM_FLAG_ADDR		0x32	
#define PORT_2_ALM_FLAG_ADDR		0x52	
#define PORT_3_ALM_FLAG_ADDR		0x72	
#define PORT_4_ALM_FLAG_ADDR		0x92	
#define PORT_5_ALM_FLAG_ADDR		0xB2	
#define PORT_6_ALM_FLAG_ADDR		0xD2	
#define PORT_7_ALM_FLAG_ADDR		0xF2	

#define PORT_1_ALM_MASK_ADDR		0x34	
#define PORT_2_ALM_MASK_ADDR		0x54	
#define PORT_3_ALM_MASK_ADDR		0x74	
#define PORT_4_ALM_MASK_ADDR		0x94	
#define PORT_5_ALM_MASK_ADDR		0xB4	
#define PORT_6_ALM_MASK_ADDR		0xD4	
#define PORT_7_ALM_MASK_ADDR		0xF4	

#define PORT_1_GET_CH_NUM_ADDR		0x36	
#define PORT_2_GET_CH_NUM_ADDR		0x56	
#define PORT_3_GET_CH_NUM_ADDR		0x76	
#define PORT_4_GET_CH_NUM_ADDR		0x96	
#define PORT_5_GET_CH_NUM_ADDR		0xB6	
#define PORT_6_GET_CH_NUM_ADDR		0xD6	
#define PORT_7_GET_CH_NUM_ADDR		0xF6	

#define PORT_1_SET_CH_NUM_ADDR		0x38	
#define PORT_2_SET_CH_NUM_ADDR		0x58	
#define PORT_3_SET_CH_NUM_ADDR		0x78	
#define PORT_4_SET_CH_NUM_ADDR		0x98	
#define PORT_5_SET_CH_NUM_ADDR		0xB8	
#define PORT_6_SET_CH_NUM_ADDR		0xD8	
#define PORT_7_SET_CH_NUM_ADDR		0xF8	


/****************************
 * Q/SFP Information related registers
 ****************************/

#define SFP_P1_VENDOR1_ADDR			0x400	
#define SFP_P2_VENDOR1_ADDR			0x440	
#define SFP_P3_VENDOR1_ADDR			0x480	
#define SFP_P4_VENDOR1_ADDR			0x4C0	
#define SFP_P5_VENDOR1_ADDR			0x500	
#define SFP_P6_VENDOR1_ADDR			0x540	
#define SFP_P7_VENDOR1_ADDR			0x580	

#define SFP_P1_VENDOR2_ADDR			0x402	
#define SFP_P2_VENDOR2_ADDR			0x442	
#define SFP_P3_VENDOR2_ADDR			0x482	
#define SFP_P4_VENDOR2_ADDR			0x4C2	
#define SFP_P5_VENDOR2_ADDR			0x502	
#define SFP_P6_VENDOR2_ADDR			0x542	
#define SFP_P7_VENDOR2_ADDR			0x582	

#define SFP_P1_VENDOR3_ADDR			0x404	
#define SFP_P2_VENDOR3_ADDR			0x444	
#define SFP_P3_VENDOR3_ADDR			0x484	
#define SFP_P4_VENDOR3_ADDR			0x4C4	
#define SFP_P5_VENDOR3_ADDR			0x504	
#define SFP_P6_VENDOR3_ADDR			0x544	
#define SFP_P7_VENDOR3_ADDR			0x584	

#define SFP_P1_VENDOR4_ADDR			0x406	
#define SFP_P2_VENDOR4_ADDR			0x446	
#define SFP_P3_VENDOR4_ADDR			0x486	
#define SFP_P4_VENDOR4_ADDR			0x4C6	
#define SFP_P5_VENDOR4_ADDR			0x506	
#define SFP_P6_VENDOR4_ADDR			0x546	
#define SFP_P7_VENDOR4_ADDR			0x586	

#define SFP_P1_VENDOR5_ADDR			0x408	
#define SFP_P2_VENDOR5_ADDR			0x448	
#define SFP_P3_VENDOR5_ADDR			0x488	
#define SFP_P4_VENDOR5_ADDR			0x4C8	
#define SFP_P5_VENDOR5_ADDR			0x508	
#define SFP_P6_VENDOR5_ADDR			0x548	
#define SFP_P7_VENDOR5_ADDR			0x588	

#define SFP_P1_VENDOR6_ADDR			0x40A	
#define SFP_P2_VENDOR6_ADDR			0x44A	
#define SFP_P3_VENDOR6_ADDR			0x48A	
#define SFP_P4_VENDOR6_ADDR			0x4CA	
#define SFP_P5_VENDOR6_ADDR			0x50A	
#define SFP_P6_VENDOR6_ADDR			0x54A	
#define SFP_P7_VENDOR6_ADDR			0x58A	

#define SFP_P1_VENDOR7_ADDR			0x40C	
#define SFP_P2_VENDOR7_ADDR			0x44C	
#define SFP_P3_VENDOR7_ADDR			0x48C	
#define SFP_P4_VENDOR7_ADDR			0x4CC	
#define SFP_P5_VENDOR7_ADDR			0x50C	
#define SFP_P6_VENDOR7_ADDR			0x54C	
#define SFP_P7_VENDOR7_ADDR			0x58C	

#define SFP_P1_VENDOR8_ADDR			0x40E	
#define SFP_P2_VENDOR8_ADDR			0x44E	
#define SFP_P3_VENDOR8_ADDR			0x48E	
#define SFP_P4_VENDOR8_ADDR			0x4CE	
#define SFP_P5_VENDOR8_ADDR			0x50E	
#define SFP_P6_VENDOR8_ADDR			0x54E	
#define SFP_P7_VENDOR8_ADDR			0x58E	

#define SFP_P1_PN1_ADDR				0x410	
#define SFP_P2_PN1_ADDR				0x450	
#define SFP_P3_PN1_ADDR				0x490	
#define SFP_P4_PN1_ADDR				0x4D0	
#define SFP_P5_PN1_ADDR				0x510	
#define SFP_P6_PN1_ADDR				0x550	
#define SFP_P7_PN1_ADDR				0x590	

#define SFP_P1_PN2_ADDR				0x412	
#define SFP_P2_PN2_ADDR				0x452	
#define SFP_P3_PN2_ADDR				0x492	
#define SFP_P4_PN2_ADDR				0x4D2	
#define SFP_P5_PN2_ADDR				0x512	
#define SFP_P6_PN2_ADDR				0x552	
#define SFP_P7_PN2_ADDR				0x592	

#define SFP_P1_PN3_ADDR				0x414	
#define SFP_P2_PN3_ADDR				0x454	
#define SFP_P3_PN3_ADDR				0x494	
#define SFP_P4_PN3_ADDR				0x4D4	
#define SFP_P5_PN3_ADDR				0x514	
#define SFP_P6_PN3_ADDR				0x554	
#define SFP_P7_PN3_ADDR				0x594	

#define SFP_P1_PN4_ADDR				0x416	
#define SFP_P2_PN4_ADDR				0x456	
#define SFP_P3_PN4_ADDR				0x496	
#define SFP_P4_PN4_ADDR				0x4D6	
#define SFP_P5_PN4_ADDR				0x516	
#define SFP_P6_PN4_ADDR				0x556	
#define SFP_P7_PN4_ADDR				0x596	

#define SFP_P1_PN5_ADDR				0x418	
#define SFP_P2_PN5_ADDR				0x458	
#define SFP_P3_PN5_ADDR				0x498	
#define SFP_P4_PN5_ADDR				0x4D8	
#define SFP_P5_PN5_ADDR				0x518	
#define SFP_P6_PN5_ADDR				0x558	
#define SFP_P7_PN5_ADDR				0x598	

#define SFP_P1_PN6_ADDR				0x41A	
#define SFP_P2_PN6_ADDR				0x45A	
#define SFP_P3_PN6_ADDR				0x49A	
#define SFP_P4_PN6_ADDR				0x4DA	
#define SFP_P5_PN6_ADDR				0x51A	
#define SFP_P6_PN6_ADDR				0x55A	
#define SFP_P7_PN6_ADDR				0x59A	

#define SFP_P1_PN7_ADDR				0x41C	
#define SFP_P2_PN7_ADDR				0x45C	
#define SFP_P3_PN7_ADDR				0x49C	
#define SFP_P4_PN7_ADDR				0x4DC	
#define SFP_P5_PN7_ADDR				0x51C	
#define SFP_P6_PN7_ADDR				0x55C	
#define SFP_P7_PN7_ADDR				0x59C	

#define SFP_P1_PN8_ADDR				0x41E	
#define SFP_P2_PN8_ADDR				0x45E	
#define SFP_P3_PN8_ADDR				0x49E	
#define SFP_P4_PN8_ADDR				0x4DE	
#define SFP_P5_PN8_ADDR				0x51E	
#define SFP_P6_PN8_ADDR				0x55E	
#define SFP_P7_PN8_ADDR				0x59E	

#define SFP_P1_SN1_ADDR				0x420	
#define SFP_P2_SN1_ADDR				0x460	
#define SFP_P3_SN1_ADDR				0x4A0	
#define SFP_P4_SN1_ADDR				0x4E0	
#define SFP_P5_SN1_ADDR				0x520	
#define SFP_P6_SN1_ADDR				0x560	
#define SFP_P7_SN1_ADDR				0x5A0	

#define SFP_P1_SN2_ADDR				0x422	
#define SFP_P2_SN2_ADDR				0x462	
#define SFP_P3_SN2_ADDR				0x4A2	
#define SFP_P4_SN2_ADDR				0x4E2	
#define SFP_P5_SN2_ADDR				0x522	
#define SFP_P6_SN2_ADDR				0x562	
#define SFP_P7_SN2_ADDR				0x5A2	

#define SFP_P1_SN3_ADDR				0x424	
#define SFP_P2_SN3_ADDR				0x464	
#define SFP_P3_SN3_ADDR				0x4A4	
#define SFP_P4_SN3_ADDR				0x4E4	
#define SFP_P5_SN3_ADDR				0x524	
#define SFP_P6_SN3_ADDR				0x564	
#define SFP_P7_SN3_ADDR				0x5A4	

#define SFP_P1_SN4_ADDR				0x426	
#define SFP_P2_SN4_ADDR				0x466	
#define SFP_P3_SN4_ADDR				0x4A6	
#define SFP_P4_SN4_ADDR				0x4E6	
#define SFP_P5_SN4_ADDR				0x526	
#define SFP_P6_SN4_ADDR				0x566	
#define SFP_P7_SN4_ADDR				0x5A6	

#define SFP_P1_SN5_ADDR				0x428	
#define SFP_P2_SN5_ADDR				0x468	
#define SFP_P3_SN5_ADDR				0x4A8	
#define SFP_P4_SN5_ADDR				0x4E8	
#define SFP_P5_SN5_ADDR				0x528	
#define SFP_P6_SN5_ADDR				0x568	
#define SFP_P7_SN5_ADDR				0x5A8	

#define SFP_P1_SN6_ADDR				0x42A	
#define SFP_P2_SN6_ADDR				0x46A	
#define SFP_P3_SN6_ADDR				0x4AA	
#define SFP_P4_SN6_ADDR				0x4EA	
#define SFP_P5_SN6_ADDR				0x52A	
#define SFP_P6_SN6_ADDR				0x56A	
#define SFP_P7_SN6_ADDR				0x5AA	

#define SFP_P1_SN7_ADDR				0x42C	
#define SFP_P2_SN7_ADDR				0x46C	
#define SFP_P3_SN7_ADDR				0x4AC	
#define SFP_P4_SN7_ADDR				0x4EC	
#define SFP_P5_SN7_ADDR				0x52C	
#define SFP_P6_SN7_ADDR				0x56C	
#define SFP_P7_SN7_ADDR				0x5AC	

#define SFP_P1_SN8_ADDR				0x42E	
#define SFP_P2_SN8_ADDR				0x46E	
#define SFP_P3_SN8_ADDR				0x4AE	
#define SFP_P4_SN8_ADDR				0x4EE	
#define SFP_P5_SN8_ADDR				0x52E	
#define SFP_P6_SN8_ADDR				0x56E	
#define SFP_P7_SN8_ADDR				0x5AE	

#define SFP_P1_TEMP_ADDR			0x430	
#define SFP_P2_TEMP_ADDR			0x470	
#define SFP_P3_TEMP_ADDR			0x4B0	
#define SFP_P4_TEMP_ADDR			0x4F0	
#define SFP_P5_TEMP_ADDR			0x530	
#define SFP_P6_TEMP_ADDR			0x570	
#define SFP_P7_TEMP_ADDR			0x5B0	

#define SFP_P1_RATE_ADDR			0x432	
#define SFP_P2_RATE_ADDR			0x472	
#define SFP_P3_RATE_ADDR			0x4B2	
#define SFP_P4_RATE_ADDR			0x4F2	
#define SFP_P5_RATE_ADDR			0x532	
#define SFP_P6_RATE_ADDR			0x572	
#define SFP_P7_RATE_ADDR			0x5B2	

#define SFP_P1_VOLT_ADDR			0x434	
#define SFP_P2_VOLT_ADDR			0x474	
#define SFP_P3_VOLT_ADDR			0x4B4	
#define SFP_P4_VOLT_ADDR			0x4F4	
#define SFP_P5_VOLT_ADDR			0x534	
#define SFP_P6_VOLT_ADDR			0x574	
#define SFP_P7_VOLT_ADDR			0x5B4	

#define SFP_P1_TX_BIAS_ADDR			0x436	
#define SFP_P2_TX_BIAS_ADDR			0x476	
#define SFP_P3_TX_BIAS_ADDR			0x4B6	
#define SFP_P4_TX_BIAS_ADDR			0x4F6	
#define SFP_P5_TX_BIAS_ADDR			0x536	
#define SFP_P6_TX_BIAS_ADDR			0x576	
#define SFP_P7_TX_BIAS_ADDR			0x5B6	

#define SFP_P1_LTEMP_ADDR			0x438	
#define SFP_P2_LTEMP_ADDR			0x478	
#define SFP_P3_LTEMP_ADDR			0x4B8	
#define SFP_P4_LTEMP_ADDR			0x4F8	
#define SFP_P5_LTEMP_ADDR			0x538	
#define SFP_P6_LTEMP_ADDR			0x578	
#define SFP_P7_LTEMP_ADDR			0x5B8	

#define SFP_P1_TCURR_ADDR			0x43A	
#define SFP_P2_TCURR_ADDR			0x47A	
#define SFP_P3_TCURR_ADDR			0x4BA	
#define SFP_P4_TCURR_ADDR			0x4FA	
#define SFP_P5_TCURR_ADDR			0x53A	
#define SFP_P6_TCURR_ADDR			0x57A	
#define SFP_P7_TCURR_ADDR			0x5BA	

#if 1/* [#72] Adding omitted rtWDM related registers, dustin, 2024-06-27 */
#define SFP_P1_VENDOR1_RTWDM_ADDR	0x600	
#define SFP_P2_VENDOR1_RTWDM_ADDR	0x640	
#define SFP_P3_VENDOR1_RTWDM_ADDR	0x680	
#define SFP_P4_VENDOR1_RTWDM_ADDR	0x6C0	
#define SFP_P5_VENDOR1_RTWDM_ADDR	0x700	
#define SFP_P6_VENDOR1_RTWDM_ADDR	0x740	

#define SFP_P1_VENDOR2_RTWDM_ADDR	0x602	
#define SFP_P2_VENDOR2_RTWDM_ADDR	0x642	
#define SFP_P3_VENDOR2_RTWDM_ADDR	0x682	
#define SFP_P4_VENDOR2_RTWDM_ADDR	0x6C2	
#define SFP_P5_VENDOR2_RTWDM_ADDR	0x702	
#define SFP_P6_VENDOR2_RTWDM_ADDR	0x742	

#define SFP_P1_VENDOR3_RTWDM_ADDR	0x604	
#define SFP_P2_VENDOR3_RTWDM_ADDR	0x644	
#define SFP_P3_VENDOR3_RTWDM_ADDR	0x684	
#define SFP_P4_VENDOR3_RTWDM_ADDR	0x6C4	
#define SFP_P5_VENDOR3_RTWDM_ADDR	0x704	
#define SFP_P6_VENDOR3_RTWDM_ADDR	0x744	

#define SFP_P1_VENDOR4_RTWDM_ADDR	0x606	
#define SFP_P2_VENDOR4_RTWDM_ADDR	0x646	
#define SFP_P3_VENDOR4_RTWDM_ADDR	0x686	
#define SFP_P4_VENDOR4_RTWDM_ADDR	0x6C6	
#define SFP_P5_VENDOR4_RTWDM_ADDR	0x706	
#define SFP_P6_VENDOR4_RTWDM_ADDR	0x746	

#define SFP_P1_VENDOR5_RTWDM_ADDR	0x608	
#define SFP_P2_VENDOR5_RTWDM_ADDR	0x648	
#define SFP_P3_VENDOR5_RTWDM_ADDR	0x688	
#define SFP_P4_VENDOR5_RTWDM_ADDR	0x6C8	
#define SFP_P5_VENDOR5_RTWDM_ADDR	0x708	
#define SFP_P6_VENDOR5_RTWDM_ADDR	0x748	

#define SFP_P1_VENDOR6_RTWDM_ADDR	0x60A	
#define SFP_P2_VENDOR6_RTWDM_ADDR	0x64A	
#define SFP_P3_VENDOR6_RTWDM_ADDR	0x68A	
#define SFP_P4_VENDOR6_RTWDM_ADDR	0x6CA	
#define SFP_P5_VENDOR6_RTWDM_ADDR	0x70A	
#define SFP_P6_VENDOR6_RTWDM_ADDR	0x74A	

#define SFP_P1_VENDOR7_RTWDM_ADDR	0x60C	
#define SFP_P2_VENDOR7_RTWDM_ADDR	0x64C	
#define SFP_P3_VENDOR7_RTWDM_ADDR	0x68C	
#define SFP_P4_VENDOR7_RTWDM_ADDR	0x6CC	
#define SFP_P5_VENDOR7_RTWDM_ADDR	0x70C	
#define SFP_P6_VENDOR7_RTWDM_ADDR	0x74C	

#define SFP_P1_VENDOR8_RTWDM_ADDR	0x60E	
#define SFP_P2_VENDOR8_RTWDM_ADDR	0x64E	
#define SFP_P3_VENDOR8_RTWDM_ADDR	0x68E	
#define SFP_P4_VENDOR8_RTWDM_ADDR	0x6CE	
#define SFP_P5_VENDOR8_RTWDM_ADDR	0x70E	
#define SFP_P6_VENDOR8_RTWDM_ADDR	0x74E	

#define SFP_P1_PN1_RTWDM_ADDR		0x610	
#define SFP_P2_PN1_RTWDM_ADDR		0x650	
#define SFP_P3_PN1_RTWDM_ADDR		0x690	
#define SFP_P4_PN1_RTWDM_ADDR		0x6D0	
#define SFP_P5_PN1_RTWDM_ADDR		0x710	
#define SFP_P6_PN1_RTWDM_ADDR		0x750	

#define SFP_P1_PN2_RTWDM_ADDR		0x612	
#define SFP_P2_PN2_RTWDM_ADDR		0x652	
#define SFP_P3_PN2_RTWDM_ADDR		0x692	
#define SFP_P4_PN2_RTWDM_ADDR		0x6D2	
#define SFP_P5_PN2_RTWDM_ADDR		0x712	
#define SFP_P6_PN2_RTWDM_ADDR		0x752	

#define SFP_P1_PN3_RTWDM_ADDR		0x614	
#define SFP_P2_PN3_RTWDM_ADDR		0x654	
#define SFP_P3_PN3_RTWDM_ADDR		0x694	
#define SFP_P4_PN3_RTWDM_ADDR		0x6D4	
#define SFP_P5_PN3_RTWDM_ADDR		0x714	
#define SFP_P6_PN3_RTWDM_ADDR		0x754	

#define SFP_P1_PN4_RTWDM_ADDR		0x616	
#define SFP_P2_PN4_RTWDM_ADDR		0x656	
#define SFP_P3_PN4_RTWDM_ADDR		0x696	
#define SFP_P4_PN4_RTWDM_ADDR		0x6D6	
#define SFP_P5_PN4_RTWDM_ADDR		0x716	
#define SFP_P6_PN4_RTWDM_ADDR		0x756	

#define SFP_P1_PN5_RTWDM_ADDR		0x618	
#define SFP_P2_PN5_RTWDM_ADDR		0x658	
#define SFP_P3_PN5_RTWDM_ADDR		0x698	
#define SFP_P4_PN5_RTWDM_ADDR		0x6D8	
#define SFP_P5_PN5_RTWDM_ADDR		0x718	
#define SFP_P6_PN5_RTWDM_ADDR		0x758	

#define SFP_P1_PN6_RTWDM_ADDR		0x61A	
#define SFP_P2_PN6_RTWDM_ADDR		0x65A	
#define SFP_P3_PN6_RTWDM_ADDR		0x69A	
#define SFP_P4_PN6_RTWDM_ADDR		0x6DA	
#define SFP_P5_PN6_RTWDM_ADDR		0x71A	
#define SFP_P6_PN6_RTWDM_ADDR		0x75A	

#define SFP_P1_PN7_RTWDM_ADDR		0x61C	
#define SFP_P2_PN7_RTWDM_ADDR		0x65C	
#define SFP_P3_PN7_RTWDM_ADDR		0x69C	
#define SFP_P4_PN7_RTWDM_ADDR		0x6DC	
#define SFP_P5_PN7_RTWDM_ADDR		0x71C	
#define SFP_P6_PN7_RTWDM_ADDR		0x75C	

#define SFP_P1_PN8_RTWDM_ADDR		0x61E	
#define SFP_P2_PN8_RTWDM_ADDR		0x65E	
#define SFP_P3_PN8_RTWDM_ADDR		0x69E	
#define SFP_P4_PN8_RTWDM_ADDR		0x6DE	
#define SFP_P5_PN8_RTWDM_ADDR		0x71E	
#define SFP_P6_PN8_RTWDM_ADDR		0x75E	

#define SFP_P1_SN1_RTWDM_ADDR		0x620	
#define SFP_P2_SN1_RTWDM_ADDR		0x660	
#define SFP_P3_SN1_RTWDM_ADDR		0x6A0	
#define SFP_P4_SN1_RTWDM_ADDR		0x6E0	
#define SFP_P5_SN1_RTWDM_ADDR		0x720	
#define SFP_P6_SN1_RTWDM_ADDR		0x760	

#define SFP_P1_SN2_RTWDM_ADDR		0x622	
#define SFP_P2_SN2_RTWDM_ADDR		0x662	
#define SFP_P3_SN2_RTWDM_ADDR		0x6A2	
#define SFP_P4_SN2_RTWDM_ADDR		0x6E2	
#define SFP_P5_SN2_RTWDM_ADDR		0x722	
#define SFP_P6_SN2_RTWDM_ADDR		0x762	

#define SFP_P1_SN3_RTWDM_ADDR		0x624	
#define SFP_P2_SN3_RTWDM_ADDR		0x664	
#define SFP_P3_SN3_RTWDM_ADDR		0x6A4	
#define SFP_P4_SN3_RTWDM_ADDR		0x6E4	
#define SFP_P5_SN3_RTWDM_ADDR		0x724	
#define SFP_P6_SN3_RTWDM_ADDR		0x764	

#define SFP_P1_SN4_RTWDM_ADDR		0x626	
#define SFP_P2_SN4_RTWDM_ADDR		0x666	
#define SFP_P3_SN4_RTWDM_ADDR		0x6A6	
#define SFP_P4_SN4_RTWDM_ADDR		0x6E6	
#define SFP_P5_SN4_RTWDM_ADDR		0x726	
#define SFP_P6_SN4_RTWDM_ADDR		0x766	

#define SFP_P1_SN5_RTWDM_ADDR		0x628	
#define SFP_P2_SN5_RTWDM_ADDR		0x668	
#define SFP_P3_SN5_RTWDM_ADDR		0x6A8	
#define SFP_P4_SN5_RTWDM_ADDR		0x6E8	
#define SFP_P5_SN5_RTWDM_ADDR		0x728	
#define SFP_P6_SN5_RTWDM_ADDR		0x768	

#define SFP_P1_SN6_RTWDM_ADDR		0x62A	
#define SFP_P2_SN6_RTWDM_ADDR		0x66A	
#define SFP_P3_SN6_RTWDM_ADDR		0x6AA	
#define SFP_P4_SN6_RTWDM_ADDR		0x6EA	
#define SFP_P5_SN6_RTWDM_ADDR		0x72A	
#define SFP_P6_SN6_RTWDM_ADDR		0x76A	

#define SFP_P1_SN7_RTWDM_ADDR		0x62C	
#define SFP_P2_SN7_RTWDM_ADDR		0x66C	
#define SFP_P3_SN7_RTWDM_ADDR		0x6AC	
#define SFP_P4_SN7_RTWDM_ADDR		0x6EC	
#define SFP_P5_SN7_RTWDM_ADDR		0x72C	
#define SFP_P6_SN7_RTWDM_ADDR		0x76C	

#define SFP_P1_SN8_RTWDM_ADDR		0x62E	
#define SFP_P2_SN8_RTWDM_ADDR		0x66E	
#define SFP_P3_SN8_RTWDM_ADDR		0x6AE	
#define SFP_P4_SN8_RTWDM_ADDR		0x6EE	
#define SFP_P5_SN8_RTWDM_ADDR		0x72E	
#define SFP_P6_SN8_RTWDM_ADDR		0x76E	

#define SFP_P1_TEMP_RTWDM_ADDR		0x630	
#define SFP_P2_TEMP_RTWDM_ADDR		0x670	
#define SFP_P3_TEMP_RTWDM_ADDR		0x6B0	
#define SFP_P4_TEMP_RTWDM_ADDR		0x6F0	
#define SFP_P5_TEMP_RTWDM_ADDR		0x730	
#define SFP_P6_TEMP_RTWDM_ADDR		0x770	

#define SFP_P1_RATE_RTWDM_ADDR		0x632	
#define SFP_P2_RATE_RTWDM_ADDR		0x672	
#define SFP_P3_RATE_RTWDM_ADDR		0x6B2	
#define SFP_P4_RATE_RTWDM_ADDR		0x6F2	
#define SFP_P5_RATE_RTWDM_ADDR		0x732	
#define SFP_P6_RATE_RTWDM_ADDR		0x772	

#define SFP_P1_VOLT_RTWDM_ADDR		0x634	
#define SFP_P2_VOLT_RTWDM_ADDR		0x674	
#define SFP_P3_VOLT_RTWDM_ADDR		0x6B4	
#define SFP_P4_VOLT_RTWDM_ADDR		0x6F4	
#define SFP_P5_VOLT_RTWDM_ADDR		0x734	
#define SFP_P6_VOLT_RTWDM_ADDR		0x774	

#define SFP_P1_TX_BIAS_RTWDM_ADDR	0x636	
#define SFP_P2_TX_BIAS_RTWDM_ADDR	0x676	
#define SFP_P3_TX_BIAS_RTWDM_ADDR	0x6B6	
#define SFP_P4_TX_BIAS_RTWDM_ADDR	0x6F6	
#define SFP_P5_TX_BIAS_RTWDM_ADDR	0x736	
#define SFP_P6_TX_BIAS_RTWDM_ADDR	0x776	

#define SFP_P1_LTEMP_RTWDM_ADDR		0x638	
#define SFP_P2_LTEMP_RTWDM_ADDR		0x678	
#define SFP_P3_LTEMP_RTWDM_ADDR		0x6B8	
#define SFP_P4_LTEMP_RTWDM_ADDR		0x6F8	
#define SFP_P5_LTEMP_RTWDM_ADDR		0x738	
#define SFP_P6_LTEMP_RTWDM_ADDR		0x778	

#define SFP_P1_TCURR_RTWDM_ADDR		0x63A	
#define SFP_P2_TCURR_RTWDM_ADDR		0x67A	
#define SFP_P3_TCURR_RTWDM_ADDR		0x6BA	
#define SFP_P4_TCURR_RTWDM_ADDR		0x6FA	
#define SFP_P5_TCURR_RTWDM_ADDR		0x73A	
#define SFP_P6_TCURR_RTWDM_ADDR		0x77A	
#endif

/****************************
 * Performance related registers
 ****************************/

#define PM_COUNT_CLEAR_ADDR			0x100	

#define PM_P1_TX_BYTE1_ADDR			0x700	
#define PM_P2_TX_BYTE1_ADDR			0x750	
#define PM_P3_TX_BYTE1_ADDR			0x7A0	
#define PM_P4_TX_BYTE1_ADDR			0x7F0	
#define PM_P5_TX_BYTE1_ADDR			0x840	
#define PM_P6_TX_BYTE1_ADDR			0x890	
#define PM_P7_TX_BYTE1_ADDR			0x8E0	

#define PM_P1_TX_BYTE2_ADDR			0x702	
#define PM_P2_TX_BYTE2_ADDR			0x752	
#define PM_P3_TX_BYTE2_ADDR			0x7A2	
#define PM_P4_TX_BYTE2_ADDR			0x7F2	
#define PM_P5_TX_BYTE2_ADDR			0x842	
#define PM_P6_TX_BYTE2_ADDR			0x892	
#define PM_P7_TX_BYTE2_ADDR			0x8E2	

#define PM_P1_TX_BYTE3_ADDR			0x704	
#define PM_P2_TX_BYTE3_ADDR			0x754	
#define PM_P3_TX_BYTE3_ADDR			0x7A4	
#define PM_P4_TX_BYTE3_ADDR			0x7F4	
#define PM_P5_TX_BYTE3_ADDR			0x844	
#define PM_P6_TX_BYTE3_ADDR			0x894	
#define PM_P7_TX_BYTE3_ADDR			0x8E4	

#define PM_P1_TX_BYTE4_ADDR			0x706	
#define PM_P2_TX_BYTE4_ADDR			0x756	
#define PM_P3_TX_BYTE4_ADDR			0x7A6	
#define PM_P4_TX_BYTE4_ADDR			0x7F6	
#define PM_P5_TX_BYTE4_ADDR			0x846	
#define PM_P6_TX_BYTE4_ADDR			0x896	
#define PM_P7_TX_BYTE4_ADDR			0x8E6	

#define PM_P1_RX_BYTE1_ADDR			0x708	
#define PM_P2_RX_BYTE1_ADDR			0x758	
#define PM_P3_RX_BYTE1_ADDR			0x7A8	
#define PM_P4_RX_BYTE1_ADDR			0x7F8	
#define PM_P5_RX_BYTE1_ADDR			0x848	
#define PM_P6_RX_BYTE1_ADDR			0x898	
#define PM_P7_RX_BYTE1_ADDR			0x8E8	

#define PM_P1_RX_BYTE2_ADDR			0x70A	
#define PM_P2_RX_BYTE2_ADDR			0x75A	
#define PM_P3_RX_BYTE2_ADDR			0x7AA	
#define PM_P4_RX_BYTE2_ADDR			0x7FA	
#define PM_P5_RX_BYTE2_ADDR			0x84A	
#define PM_P6_RX_BYTE2_ADDR			0x89A	
#define PM_P7_RX_BYTE2_ADDR			0x8EA	

#define PM_P1_RX_BYTE3_ADDR			0x70C	
#define PM_P2_RX_BYTE3_ADDR			0x75C	
#define PM_P3_RX_BYTE3_ADDR			0x7AC	
#define PM_P4_RX_BYTE3_ADDR			0x7FC	
#define PM_P5_RX_BYTE3_ADDR			0x84C	
#define PM_P6_RX_BYTE3_ADDR			0x89C	
#define PM_P7_RX_BYTE3_ADDR			0x8EC	

#define PM_P1_RX_BYTE4_ADDR			0x70E	
#define PM_P2_RX_BYTE4_ADDR			0x75E	
#define PM_P3_RX_BYTE4_ADDR			0x7AE	
#define PM_P4_RX_BYTE4_ADDR			0x7FE	
#define PM_P5_RX_BYTE4_ADDR			0x84E	
#define PM_P6_RX_BYTE4_ADDR			0x89E	
#define PM_P7_RX_BYTE4_ADDR			0x8EE	

#define PM_P1_TX_FRAME1_ADDR		0x710	
#define PM_P2_TX_FRAME1_ADDR		0x760	
#define PM_P3_TX_FRAME1_ADDR		0x7B0	
#define PM_P4_TX_FRAME1_ADDR		0x800	
#define PM_P5_TX_FRAME1_ADDR		0x850	
#define PM_P6_TX_FRAME1_ADDR		0x8A0	
#define PM_P7_TX_FRAME1_ADDR		0x8F0	

#define PM_P1_TX_FRAME2_ADDR		0x712	
#define PM_P2_TX_FRAME2_ADDR		0x762	
#define PM_P3_TX_FRAME2_ADDR		0x7B2	
#define PM_P4_TX_FRAME2_ADDR		0x802	
#define PM_P5_TX_FRAME2_ADDR		0x852	
#define PM_P6_TX_FRAME2_ADDR		0x8A2	
#define PM_P7_TX_FRAME2_ADDR		0x8F2	

#define PM_P1_TX_FRAME3_ADDR		0x714	
#define PM_P2_TX_FRAME3_ADDR		0x764	
#define PM_P3_TX_FRAME3_ADDR		0x7B4	
#define PM_P4_TX_FRAME3_ADDR		0x804	
#define PM_P5_TX_FRAME3_ADDR		0x854	
#define PM_P6_TX_FRAME3_ADDR		0x8A4	
#define PM_P7_TX_FRAME3_ADDR		0x8F4	

#define PM_P1_TX_FRAME4_ADDR		0x716	
#define PM_P2_TX_FRAME4_ADDR		0x766	
#define PM_P3_TX_FRAME4_ADDR		0x7B6	
#define PM_P4_TX_FRAME4_ADDR		0x806	
#define PM_P5_TX_FRAME4_ADDR		0x856	
#define PM_P6_TX_FRAME4_ADDR		0x8A6	
#define PM_P7_TX_FRAME4_ADDR		0x8F6	

#define PM_P1_RX_FRAME1_ADDR		0x718	
#define PM_P2_RX_FRAME1_ADDR		0x768	
#define PM_P3_RX_FRAME1_ADDR		0x7B8	
#define PM_P4_RX_FRAME1_ADDR		0x808	
#define PM_P5_RX_FRAME1_ADDR		0x858	
#define PM_P6_RX_FRAME1_ADDR		0x8A8	
#define PM_P7_RX_FRAME1_ADDR		0x8F8	

#define PM_P1_RX_FRAME2_ADDR		0x71A	
#define PM_P2_RX_FRAME2_ADDR		0x76A	
#define PM_P3_RX_FRAME2_ADDR		0x7BA	
#define PM_P4_RX_FRAME2_ADDR		0x80A	
#define PM_P5_RX_FRAME2_ADDR		0x85A	
#define PM_P6_RX_FRAME2_ADDR		0x8AA	
#define PM_P7_RX_FRAME2_ADDR		0x8FA	

#define PM_P1_RX_FRAME3_ADDR		0x71C	
#define PM_P2_RX_FRAME3_ADDR		0x76C	
#define PM_P3_RX_FRAME3_ADDR		0x7BC	
#define PM_P4_RX_FRAME3_ADDR		0x80C	
#define PM_P5_RX_FRAME3_ADDR		0x85C	
#define PM_P6_RX_FRAME3_ADDR		0x8AC	
#define PM_P7_RX_FRAME3_ADDR		0x8FC	

#define PM_P1_RX_FRAME4_ADDR		0x71E	
#define PM_P2_RX_FRAME4_ADDR		0x76E	
#define PM_P3_RX_FRAME4_ADDR		0x7BE	
#define PM_P4_RX_FRAME4_ADDR		0x80E	
#define PM_P5_RX_FRAME4_ADDR		0x85E	
#define PM_P6_RX_FRAME4_ADDR		0x8AE	
#define PM_P7_RX_FRAME4_ADDR		0x8FE	

#define PM_P1_FCS1_ADDR			0x720	
#define PM_P2_FCS1_ADDR	     	0x770	
#define PM_P3_FCS1_ADDR	     	0x7C0	
#define PM_P4_FCS1_ADDR	     	0x810	
#define PM_P5_FCS1_ADDR	     	0x860	
#define PM_P6_FCS1_ADDR	     	0x8B0	
#define PM_P7_FCS1_ADDR	     	0x900	

#define PM_P1_FCS2_ADDR	     	0x722	
#define PM_P2_FCS2_ADDR	     	0x772	
#define PM_P3_FCS2_ADDR	     	0x7C2	
#define PM_P4_FCS2_ADDR	     	0x812	
#define PM_P5_FCS2_ADDR	     	0x862	
#define PM_P6_FCS2_ADDR	     	0x8B2	
#define PM_P7_FCS2_ADDR	     	0x902	

#define PM_P1_FCS3_ADDR	     	0x724	
#define PM_P2_FCS3_ADDR	     	0x774	
#define PM_P3_FCS3_ADDR	     	0x7C4	
#define PM_P4_FCS3_ADDR	     	0x814	
#define PM_P5_FCS3_ADDR	     	0x864	
#define PM_P6_FCS3_ADDR	     	0x8B4	
#define PM_P7_FCS3_ADDR	     	0x904	

#define PM_P1_FCS4_ADDR	     	0x726	
#define PM_P2_FCS4_ADDR	     	0x776	
#define PM_P3_FCS4_ADDR	     	0x7C6	
#define PM_P4_FCS4_ADDR	     	0x816	
#define PM_P5_FCS4_ADDR	     	0x866	
#define PM_P6_FCS4_ADDR	     	0x8B6	
#define PM_P7_FCS4_ADDR	     	0x906	

#define PM_P1_FCS_OK1_ADDR   	0x730	
#define PM_P2_FCS_OK1_ADDR   	0x780	
#define PM_P3_FCS_OK1_ADDR   	0x7D0	
#define PM_P4_FCS_OK1_ADDR   	0x820	
#define PM_P5_FCS_OK1_ADDR   	0x870	
#define PM_P6_FCS_OK1_ADDR   	0x8C0	
#define PM_P7_FCS_OK1_ADDR   	0x910	

#define PM_P1_FCS_OK2_ADDR   	0x732	
#define PM_P2_FCS_OK2_ADDR   	0x782	
#define PM_P3_FCS_OK2_ADDR   	0x7D2	
#define PM_P4_FCS_OK2_ADDR   	0x822	
#define PM_P5_FCS_OK2_ADDR   	0x872	
#define PM_P6_FCS_OK2_ADDR   	0x8C2	
#define PM_P7_FCS_OK2_ADDR   	0x912	

#define PM_P1_FCS_OK3_ADDR   	0x734	
#define PM_P2_FCS_OK3_ADDR   	0x784	
#define PM_P3_FCS_OK3_ADDR   	0x7D4	
#define PM_P4_FCS_OK3_ADDR   	0x824	
#define PM_P5_FCS_OK3_ADDR   	0x874	
#define PM_P6_FCS_OK3_ADDR   	0x8C4	
#define PM_P7_FCS_OK3_ADDR   	0x914	

#define PM_P1_FCS_OK4_ADDR   	0x736	
#define PM_P2_FCS_OK4_ADDR   	0x786	
#define PM_P3_FCS_OK4_ADDR   	0x7D6	
#define PM_P4_FCS_OK4_ADDR   	0x826	
#define PM_P5_FCS_OK4_ADDR   	0x876	
#define PM_P6_FCS_OK4_ADDR   	0x8C6	
#define PM_P7_FCS_OK4_ADDR   	0x916	

#define PM_P1_FCS_NOK1_ADDR  	0x738	
#define PM_P2_FCS_NOK1_ADDR  	0x788	
#define PM_P3_FCS_NOK1_ADDR  	0x7D8	
#define PM_P4_FCS_NOK1_ADDR  	0x828	
#define PM_P5_FCS_NOK1_ADDR  	0x878	
#define PM_P6_FCS_NOK1_ADDR  	0x8C8	
#define PM_P7_FCS_NOK1_ADDR  	0x918	

#define PM_P1_FCS_NOK2_ADDR  	0x73A	
#define PM_P2_FCS_NOK2_ADDR  	0x78A	
#define PM_P3_FCS_NOK2_ADDR  	0x7DA	
#define PM_P4_FCS_NOK2_ADDR  	0x82A	
#define PM_P5_FCS_NOK2_ADDR  	0x87A	
#define PM_P6_FCS_NOK2_ADDR  	0x8CA	
#define PM_P7_FCS_NOK2_ADDR  	0x91A	

#define PM_P1_FCS_NOK3_ADDR  	0x73C	
#define PM_P2_FCS_NOK3_ADDR  	0x78C	
#define PM_P3_FCS_NOK3_ADDR  	0x7DC	
#define PM_P4_FCS_NOK3_ADDR  	0x82C	
#define PM_P5_FCS_NOK3_ADDR  	0x87C	
#define PM_P6_FCS_NOK3_ADDR  	0x8CC	
#define PM_P7_FCS_NOK3_ADDR  	0x91C	

#define PM_P1_FCS_NOK4_ADDR  	0x73E	
#define PM_P2_FCS_NOK4_ADDR  	0x78E	
#define PM_P3_FCS_NOK4_ADDR  	0x7DE	
#define PM_P4_FCS_NOK4_ADDR  	0x82E	
#define PM_P5_FCS_NOK4_ADDR  	0x87E	
#define PM_P6_FCS_NOK4_ADDR  	0x8CE	
#define PM_P7_FCS_NOK4_ADDR  	0x91E	


/****************************
 * DCO related registers
 ****************************/



/****************************
 * CLEI/USI Information related registers
 ****************************/

#define PORT1_CLEI1_ADDR 			0xB00	
#define PORT2_CLEI1_ADDR 			0xB10	
#define PORT3_CLEI1_ADDR 			0xB20	
#define PORT4_CLEI1_ADDR 			0xB30	
#define PORT5_CLEI1_ADDR 			0xB40	
#define PORT6_CLEI1_ADDR 			0xB50	
#define PORT7_CLEI1_ADDR 			0xB60	

#define PORT1_CLEI2_ADDR 			0xB02	
#define PORT2_CLEI2_ADDR 			0xB12	
#define PORT3_CLEI2_ADDR 			0xB22	
#define PORT4_CLEI2_ADDR 			0xB32	
#define PORT5_CLEI2_ADDR 			0xB42	
#define PORT6_CLEI2_ADDR 			0xB52	
#define PORT7_CLEI2_ADDR 			0xB62	

#define PORT1_CLEI3_ADDR 			0xB04	
#define PORT2_CLEI3_ADDR 			0xB14	
#define PORT3_CLEI3_ADDR 			0xB24	
#define PORT4_CLEI3_ADDR 			0xB34	
#define PORT5_CLEI3_ADDR 			0xB44	
#define PORT6_CLEI3_ADDR 			0xB54	
#define PORT7_CLEI3_ADDR 			0xB64	

#define PORT1_CLEI4_ADDR 			0xB06	
#define PORT2_CLEI4_ADDR 			0xB16	
#define PORT3_CLEI4_ADDR 			0xB26	
#define PORT4_CLEI4_ADDR 			0xB36	
#define PORT5_CLEI4_ADDR 			0xB46	
#define PORT6_CLEI4_ADDR 			0xB56	
#define PORT7_CLEI4_ADDR 			0xB66	

#define PORT1_CLEI5_ADDR 			0xB08	
#define PORT2_CLEI5_ADDR 			0xB18	
#define PORT3_CLEI5_ADDR 			0xB28	
#define PORT4_CLEI5_ADDR 			0xB38	
#define PORT5_CLEI5_ADDR 			0xB48	
#define PORT6_CLEI5_ADDR 			0xB58	
#define PORT7_CLEI5_ADDR 			0xB68	

#define PORT1_USI1_ADDR 			0xC00	
#define PORT2_USI1_ADDR 			0xC20	
#define PORT3_USI1_ADDR 			0xC40	
#define PORT4_USI1_ADDR 			0xC60	
#define PORT5_USI1_ADDR 			0xC80	
#define PORT6_USI1_ADDR 			0xCA0	
#define PORT7_USI1_ADDR 			0xCC0	

#define PORT1_USI2_ADDR 			0xC02	
#define PORT2_USI2_ADDR 			0xC22	
#define PORT3_USI2_ADDR 			0xC42	
#define PORT4_USI2_ADDR 			0xC62	
#define PORT5_USI2_ADDR 			0xC82	
#define PORT6_USI2_ADDR 			0xCA2	
#define PORT7_USI2_ADDR 			0xCC2	

#define PORT1_USI3_ADDR 			0xC04	
#define PORT2_USI3_ADDR 			0xC24	
#define PORT3_USI3_ADDR 			0xC44	
#define PORT4_USI3_ADDR 			0xC64	
#define PORT5_USI3_ADDR 			0xC84	
#define PORT6_USI3_ADDR 			0xCA4	
#define PORT7_USI3_ADDR 			0xCC4	

#define PORT1_USI4_ADDR 			0xC06	
#define PORT2_USI4_ADDR 			0xC26	
#define PORT3_USI4_ADDR 			0xC46	
#define PORT4_USI4_ADDR 			0xC66	
#define PORT5_USI4_ADDR 			0xC86	
#define PORT6_USI4_ADDR 			0xCA6	
#define PORT7_USI4_ADDR 			0xCC6	

#define PORT1_USI5_ADDR 			0xC08	
#define PORT2_USI5_ADDR 			0xC28	
#define PORT3_USI5_ADDR 			0xC48	
#define PORT4_USI5_ADDR 			0xC68	
#define PORT5_USI5_ADDR 			0xC88	
#define PORT6_USI5_ADDR 			0xCA8	
#define PORT7_USI5_ADDR 			0xCC8	

#define PORT1_USI6_ADDR 			0xC0A	
#define PORT2_USI6_ADDR 			0xC2A	
#define PORT3_USI6_ADDR 			0xC4A	
#define PORT4_USI6_ADDR 			0xC6A	
#define PORT5_USI6_ADDR 			0xC8A	
#define PORT6_USI6_ADDR 			0xCAA	
#define PORT7_USI6_ADDR 			0xCCA	

#define PORT1_USI7_ADDR 			0xC0C	
#define PORT2_USI7_ADDR 			0xC2C	
#define PORT3_USI7_ADDR 			0xC4C	
#define PORT4_USI7_ADDR 			0xC6C	
#define PORT5_USI7_ADDR 			0xC8C	
#define PORT6_USI7_ADDR 			0xCAC	
#define PORT7_USI7_ADDR 			0xCCC	

#define PORT1_USI8_ADDR 			0xC0E	
#define PORT2_USI8_ADDR 			0xC2E	
#define PORT3_USI8_ADDR 			0xC4E	
#define PORT4_USI8_ADDR 			0xC6E	
#define PORT5_USI8_ADDR 			0xC8E	
#define PORT6_USI8_ADDR 			0xCAE	
#define PORT7_USI8_ADDR 			0xCCE	

#define PORT1_USI9_ADDR 			0xC10	
#define PORT2_USI9_ADDR 			0xC30	
#define PORT3_USI9_ADDR 			0xC50	
#define PORT4_USI9_ADDR 			0xC70	
#define PORT5_USI9_ADDR 			0xC90	
#define PORT6_USI9_ADDR 			0xCB0	
#define PORT7_USI9_ADDR 			0xCD0	

#define PORT1_USI10_ADDR 			0xC12	
#define PORT2_USI10_ADDR 			0xC32	
#define PORT3_USI10_ADDR 			0xC52	
#define PORT4_USI10_ADDR 			0xC72	
#define PORT5_USI10_ADDR 			0xC92	
#define PORT6_USI10_ADDR 			0xCB2	
#define PORT7_USI10_ADDR 			0xCD2	

#define PORT1_USI11_ADDR 			0xC14	
#define PORT2_USI11_ADDR 			0xC34	
#define PORT3_USI11_ADDR 			0xC54	
#define PORT4_USI11_ADDR 			0xC74	
#define PORT5_USI11_ADDR 			0xC94	
#define PORT6_USI11_ADDR 			0xCB4	
#define PORT7_USI11_ADDR 			0xCD4	

#define PORT1_USI12_ADDR 			0xC16	
#define PORT2_USI12_ADDR 			0xC36	
#define PORT3_USI12_ADDR 			0xC56	
#define PORT4_USI12_ADDR 			0xC76	
#define PORT5_USI12_ADDR 			0xC96	
#define PORT6_USI12_ADDR 			0xCB6	
#define PORT7_USI12_ADDR 			0xCD6	

#define PORT1_USI13_ADDR 			0xC18	
#define PORT2_USI13_ADDR 			0xC38	
#define PORT3_USI13_ADDR 			0xC58	
#define PORT4_USI13_ADDR 			0xC78	
#define PORT5_USI13_ADDR 			0xC98	
#define PORT6_USI13_ADDR 			0xCB8	
#define PORT7_USI13_ADDR 			0xCD8	


/* 
 * Port Alarm Field Mask 
 */

#define PORT_ALM_TYPE_LOS			0x0001
#define PORT_ALM_TYPE_LF			0x0002
#define PORT_ALM_TYPE_LOC			0x0004
#define PORT_ALM_TYPE_RF			0x0008
#define PORT_ALM_TYPE_TX_BIAS		0x0100
#define PORT_ALM_TYPE_LASER			0x0200
#define PORT_ALM_TYPE_LOOP_WDM		0x0400
#define PORT_ALM_TYPE_LOOP_TSFP		0x0800

#define PORT_ALM_MASK_LOS			0x0001
#define PORT_ALM_MASK_LF			0x0002
#define PORT_ALM_MASK_LOC			0x0004
#define PORT_ALM_MASK_RF			0x0008
#define PORT_ALM_MASK_TX_BIAS		0x0100
#define PORT_ALM_MASK_FT_RESET		0x8000


#if 1/* [#70] Adding RDL feature, dustin, 2024-07-02 */
/*
 * RDL DPRAM DATA (RDL PAGE)
 */

#define RDL_DPRAM_BASE              0x600000
#define RDL_PAGE_ADDR_SIZE          0x40000
#define RDL_PAGE_SIZE               (RDL_PAGE_ADDR_SIZE)
#define RDL_PAGE_SEGMENT_SIZE       (RDL_PAGE_ADDR_SIZE / 2)
#define RDL_PAGE_1_START_ADDR       0x600000
#define RDL_PAGE_1_END_ADDR         0x61FFFF
#define RDL_PAGE_2_START_ADDR       0x620000
#define RDL_PAGE_2_END_ADDR         0x63FFFF


/*
 * RDL DPRAM (CONTROL/STATUS)
 */

#define RDL_STATE_REQ_ADDR          0x500000	/* refer to DPRAM_BASE */
#define RDL_STATE_RESP_ADDR         0x500002
#define RDL_PAGE_CRC_ADDR           0x500010
#define RDL_WRITE_STATE_ADDR        0x500012
#define RDL_TARGET_BANK_ADDR        0x500014	/* 1: bank1, 2: bank2, 0/3:none */
#define RDL_MAGIC_NO_1_ADDR         0x500100
#define RDL_MAGIC_NO_2_ADDR         0x500102
#define RDL_TOTAL_CRC_1_ADDR        0x500104
#define RDL_TOTAL_CRC_2_ADDR        0x500106
#define RDL_BUILD_TIME_1_ADDR       0x500108
#define RDL_BUILD_TIME_2_ADDR       0x50010A
#define RDL_TOTAL_SIZE_1_ADDR       0x50010C
#define RDL_TOTAL_SIZE_2_ADDR       0x50010E
#define RDL_VER_STR_START_ADDR      0x500110
#define RDL_VER_STR_END_ADDR        0x50011F
#define RDL_FILE_NAME_START_ADDR    0x500120
#define RDL_FILE_NAME_END_ADDR      0x50013F


/*
 * Bank 1/2 Header
 */

#define BANK1_MAGIC_NO_1_ADDR       0x501100
#define BANK1_MAGIC_NO_2_ADDR       0x501102
#if 1 /* [#102] Fixing some register updates, dustin, 2024-08-26 */
#define BANK1_HEADER_CRC_1_ADDR     0x501104
#define BANK1_HEADER_CRC_2_ADDR     0x501106
#else
#define BANK1_TOTAL_CRC_1_ADDR      0x501104
#define BANK1_TOTAL_CRC_2_ADDR      0x501106
#endif/* [#102] */
#define BANK1_BUILD_TIME_1_ADDR     0x501108
#define BANK1_BUILD_TIME_2_ADDR     0x50110A
#define BANK1_TOTAL_SIZE_1_ADDR     0x50110C
#define BANK1_TOTAL_SIZE_2_ADDR     0x50110E
#if 1 /* [#102] Fixing some register updates, dustin, 2024-08-26 */
#define BANK1_CARD_TYPE_1_ADDR      0x501110
#define BANK1_CARD_TYPE_2_ADDR      0x501112
#define BANK1_TOTAL_CRC_1_ADDR      0x501114
#define BANK1_TOTAL_CRC_2_ADDR      0x501116
#define BANK1_VER_STR_START_ADDR    0x501118
#define BANK1_VER_STR_END_ADDR      0x501126
#define BANK1_FILE_NAME_START_ADDR  0x501128
#define BANK1_FILE_NAME_END_ADDR    0x501146
#else
#define BANK1_VER_STR_START_ADDR    0x501110
#define BANK1_VER_STR_END_ADDR      0x50111F
#define BANK1_FILE_NAME_START_ADDR  0x501120
#define BANK1_FILE_NAME_END_ADDR    0x50113F
#endif/* [#102] */


#define BANK2_MAGIC_NO_1_ADDR       0x502100
#define BANK2_MAGIC_NO_2_ADDR       0x502102
#if 1 /* [#102] Fixing some register updates, dustin, 2024-08-26 */
#define BANK2_HEADER_CRC_1_ADDR     0x502104
#define BANK2_HEADER_CRC_2_ADDR     0x502106
#else
#define BANK2_TOTAL_CRC_1_ADDR      0x502104
#define BANK2_TOTAL_CRC_2_ADDR      0x502106
#endif/* [#102] */
#define BANK2_BUILD_TIME_1_ADDR     0x502108
#define BANK2_BUILD_TIME_2_ADDR     0x50210A
#define BANK2_TOTAL_SIZE_1_ADDR     0x50210C
#define BANK2_TOTAL_SIZE_2_ADDR     0x50210E
#if 1 /* [#102] Fixing some register updates, dustin, 2024-08-26 */
#define BANK2_CARD_TYPE_1_ADDR      0x502110
#define BANK2_CARD_TYPE_2_ADDR      0x502112
#define BANK2_TOTAL_CRC_1_ADDR      0x502114
#define BANK2_TOTAL_CRC_2_ADDR      0x502116
#define BANK2_VER_STR_START_ADDR    0x502118
#define BANK2_VER_STR_END_ADDR      0x502126
#define BANK2_FILE_NAME_START_ADDR  0x502128
#define BANK2_FILE_NAME_END_ADDR    0x502146
#else
#define BANK2_VER_STR_START_ADDR    0x502110
#define BANK2_VER_STR_END_ADDR      0x50211F
#define BANK2_FILE_NAME_START_ADDR  0x502120
#define BANK2_FILE_NAME_END_ADDR    0x50213F
#endif/* [#102] */
#endif/*[#70]*/


#define PWM_NOT_USED        (-99.9f)
#define DEF_PWM_MIN         (-53.0f)
#define DEF_PWM_MAX         (20.0f)


#if 1/*[#65] Adding regMon simulation feature under ACCESS_SIM, dustin, 2024-06-24 */
/*
 * FPGA Bank Select
 */

#define FW_BANK_SELECT_ADDR		0xF00000
#if 1 /* [#91] Fixing for register updating feature, dustin, 2024-08-05 */
#define CPLD_FW_BANK_SELECT_ADDR	0x001C
#endif
#if 1 /* [#93] Adding for FPGA FW Bank Select and Error handling, dustin, 2024-08-12 */
#define CPLD_FW_BANK_STATUS_ADDR	0x0010
#define CPLD_BANK_BAD               0x08
#define CPLD_BANK_OK                0x09
#endif
#endif


extern unsigned long __COMMON_CTRL2_ADDR[PORT_ID_EAG6L_MAX];
extern unsigned long __PORT_CONFIG_ADDR[PORT_ID_EAG6L_MAX];
extern unsigned long __PORT_TX_PWR_ADDR[PORT_ID_EAG6L_MAX];
extern unsigned long __PORT_RX_PWR_ADDR[PORT_ID_EAG6L_MAX];
extern unsigned long __PORT_WL1_ADDR[PORT_ID_EAG6L_MAX];
extern unsigned long __PORT_WL2_ADDR[PORT_ID_EAG6L_MAX];
extern unsigned long __PORT_DIST_ADDR[PORT_ID_EAG6L_MAX];
#if 1/* [#72] Adding omitted rtWDM related registers, dustin, 2024-06-27 */
extern unsigned long __PORT_TX_PWR_RTWDM_ADDR[PORT_ID_EAG6L_MAX];
extern unsigned long __PORT_RX_PWR_RTWDM_ADDR[PORT_ID_EAG6L_MAX];
extern unsigned long __PORT_WL1_RTWDM_ADDR[PORT_ID_EAG6L_MAX];
extern unsigned long __PORT_WL2_RTWDM_ADDR[PORT_ID_EAG6L_MAX];
extern unsigned long __PORT_DIST_RTWDM_ADDR[PORT_ID_EAG6L_MAX];
#endif
extern unsigned long __PORT_STSFP_STAT_ADDR[PORT_ID_EAG6L_MAX];
extern unsigned long __PORT_ALM_ADDR[PORT_ID_EAG6L_MAX];
extern unsigned long __PORT_ALM_FLAG_ADDR[PORT_ID_EAG6L_MAX];
extern unsigned long __PORT_ALM_MASK_ADDR[PORT_ID_EAG6L_MAX];
extern unsigned long __PORT_GET_CH_NUM_ADDR[PORT_ID_EAG6L_MAX];
extern unsigned long __PORT_SET_CH_NUM_ADDR[PORT_ID_EAG6L_MAX];
extern unsigned long __PORT_VENDOR1_ADDR[PORT_ID_EAG6L_MAX];
extern unsigned long __PORT_VENDOR2_ADDR[PORT_ID_EAG6L_MAX];
extern unsigned long __PORT_VENDOR3_ADDR[PORT_ID_EAG6L_MAX];
extern unsigned long __PORT_VENDOR4_ADDR[PORT_ID_EAG6L_MAX];
extern unsigned long __PORT_VENDOR5_ADDR[PORT_ID_EAG6L_MAX];
extern unsigned long __PORT_VENDOR6_ADDR[PORT_ID_EAG6L_MAX];
extern unsigned long __PORT_VENDOR7_ADDR[PORT_ID_EAG6L_MAX];
extern unsigned long __PORT_VENDOR8_ADDR[PORT_ID_EAG6L_MAX];
extern unsigned long __PORT_PN1_ADDR[PORT_ID_EAG6L_MAX];
extern unsigned long __PORT_PN2_ADDR[PORT_ID_EAG6L_MAX];
extern unsigned long __PORT_PN3_ADDR[PORT_ID_EAG6L_MAX];
extern unsigned long __PORT_PN4_ADDR[PORT_ID_EAG6L_MAX];
extern unsigned long __PORT_PN5_ADDR[PORT_ID_EAG6L_MAX];
extern unsigned long __PORT_PN6_ADDR[PORT_ID_EAG6L_MAX];
extern unsigned long __PORT_PN7_ADDR[PORT_ID_EAG6L_MAX];
extern unsigned long __PORT_PN8_ADDR[PORT_ID_EAG6L_MAX];
extern unsigned long __PORT_SN1_ADDR[PORT_ID_EAG6L_MAX];
extern unsigned long __PORT_SN2_ADDR[PORT_ID_EAG6L_MAX];
extern unsigned long __PORT_SN3_ADDR[PORT_ID_EAG6L_MAX];
extern unsigned long __PORT_SN4_ADDR[PORT_ID_EAG6L_MAX];
extern unsigned long __PORT_SN5_ADDR[PORT_ID_EAG6L_MAX];
extern unsigned long __PORT_SN6_ADDR[PORT_ID_EAG6L_MAX];
extern unsigned long __PORT_SN7_ADDR[PORT_ID_EAG6L_MAX];
extern unsigned long __PORT_SN8_ADDR[PORT_ID_EAG6L_MAX];
extern unsigned long __PORT_TEMP_ADDR[PORT_ID_EAG6L_MAX];
extern unsigned long __PORT_RATE_ADDR[PORT_ID_EAG6L_MAX];
extern unsigned long __PORT_VOLT_ADDR[PORT_ID_EAG6L_MAX];
extern unsigned long __PORT_TX_BIAS_ADDR[PORT_ID_EAG6L_MAX];
extern unsigned long __PORT_LTEMP_ADDR[PORT_ID_EAG6L_MAX];
extern unsigned long __PORT_TCURR_ADDR[PORT_ID_EAG6L_MAX];
#if 1/* [#72] Adding omitted rtWDM related registers, dustin, 2024-06-27 */
extern unsigned long __PORT_VENDOR1_RTWDM_ADDR[PORT_ID_EAG6L_MAX];
extern unsigned long __PORT_VENDOR2_RTWDM_ADDR[PORT_ID_EAG6L_MAX];
extern unsigned long __PORT_VENDOR3_RTWDM_ADDR[PORT_ID_EAG6L_MAX];
extern unsigned long __PORT_VENDOR4_RTWDM_ADDR[PORT_ID_EAG6L_MAX];
extern unsigned long __PORT_VENDOR5_RTWDM_ADDR[PORT_ID_EAG6L_MAX];
extern unsigned long __PORT_VENDOR6_RTWDM_ADDR[PORT_ID_EAG6L_MAX];
extern unsigned long __PORT_VENDOR7_RTWDM_ADDR[PORT_ID_EAG6L_MAX];
extern unsigned long __PORT_VENDOR8_RTWDM_ADDR[PORT_ID_EAG6L_MAX];
extern unsigned long __PORT_PN1_RTWDM_ADDR[PORT_ID_EAG6L_MAX];
extern unsigned long __PORT_PN2_RTWDM_ADDR[PORT_ID_EAG6L_MAX];
extern unsigned long __PORT_PN3_RTWDM_ADDR[PORT_ID_EAG6L_MAX];
extern unsigned long __PORT_PN4_RTWDM_ADDR[PORT_ID_EAG6L_MAX];
extern unsigned long __PORT_PN5_RTWDM_ADDR[PORT_ID_EAG6L_MAX];
extern unsigned long __PORT_PN6_RTWDM_ADDR[PORT_ID_EAG6L_MAX];
extern unsigned long __PORT_PN7_RTWDM_ADDR[PORT_ID_EAG6L_MAX];
extern unsigned long __PORT_PN8_RTWDM_ADDR[PORT_ID_EAG6L_MAX];
extern unsigned long __PORT_SN1_RTWDM_ADDR[PORT_ID_EAG6L_MAX];
extern unsigned long __PORT_SN2_RTWDM_ADDR[PORT_ID_EAG6L_MAX];
extern unsigned long __PORT_SN3_RTWDM_ADDR[PORT_ID_EAG6L_MAX];
extern unsigned long __PORT_SN4_RTWDM_ADDR[PORT_ID_EAG6L_MAX];
extern unsigned long __PORT_SN5_RTWDM_ADDR[PORT_ID_EAG6L_MAX];
extern unsigned long __PORT_SN6_RTWDM_ADDR[PORT_ID_EAG6L_MAX];
extern unsigned long __PORT_SN7_RTWDM_ADDR[PORT_ID_EAG6L_MAX];
extern unsigned long __PORT_SN8_RTWDM_ADDR[PORT_ID_EAG6L_MAX];
extern unsigned long __PORT_TEMP_RTWDM_ADDR[PORT_ID_EAG6L_MAX];
extern unsigned long __PORT_RATE_RTWDM_ADDR[PORT_ID_EAG6L_MAX];
extern unsigned long __PORT_VOLT_RTWDM_ADDR[PORT_ID_EAG6L_MAX];
extern unsigned long __PORT_TX_BIAS_RTWDM_ADDR[PORT_ID_EAG6L_MAX];
extern unsigned long __PORT_LTEMP_RTWDM_ADDR[PORT_ID_EAG6L_MAX];
extern unsigned long __PORT_TCURR_RTWDM_ADDR[PORT_ID_EAG6L_MAX];
#endif
extern unsigned long __PORT_PM_TX_BYTE1_ADDR[PORT_ID_EAG6L_MAX];
extern unsigned long __PORT_PM_TX_BYTE2_ADDR[PORT_ID_EAG6L_MAX];
extern unsigned long __PORT_PM_TX_BYTE3_ADDR[PORT_ID_EAG6L_MAX];
extern unsigned long __PORT_PM_TX_BYTE4_ADDR[PORT_ID_EAG6L_MAX];
extern unsigned long __PORT_PM_RX_BYTE1_ADDR[PORT_ID_EAG6L_MAX];
extern unsigned long __PORT_PM_RX_BYTE2_ADDR[PORT_ID_EAG6L_MAX];
extern unsigned long __PORT_PM_RX_BYTE3_ADDR[PORT_ID_EAG6L_MAX];
extern unsigned long __PORT_PM_RX_BYTE4_ADDR[PORT_ID_EAG6L_MAX];
extern unsigned long __PORT_PM_TX_FRAME1_ADDR[PORT_ID_EAG6L_MAX];
extern unsigned long __PORT_PM_TX_FRAME2_ADDR[PORT_ID_EAG6L_MAX];
extern unsigned long __PORT_PM_TX_FRAME3_ADDR[PORT_ID_EAG6L_MAX];
extern unsigned long __PORT_PM_TX_FRAME4_ADDR[PORT_ID_EAG6L_MAX];
extern unsigned long __PORT_PM_RX_FRAME1_ADDR[PORT_ID_EAG6L_MAX];
extern unsigned long __PORT_PM_RX_FRAME2_ADDR[PORT_ID_EAG6L_MAX];
extern unsigned long __PORT_PM_RX_FRAME3_ADDR[PORT_ID_EAG6L_MAX];
extern unsigned long __PORT_PM_RX_FRAME4_ADDR[PORT_ID_EAG6L_MAX];
extern unsigned long __PORT_PM_FCS1_ADDR[PORT_ID_EAG6L_MAX];
extern unsigned long __PORT_PM_FCS2_ADDR[PORT_ID_EAG6L_MAX];
extern unsigned long __PORT_PM_FCS3_ADDR[PORT_ID_EAG6L_MAX];
extern unsigned long __PORT_PM_FCS4_ADDR[PORT_ID_EAG6L_MAX];
extern unsigned long __PORT_PM_FCS_OK1_ADDR[PORT_ID_EAG6L_MAX];
extern unsigned long __PORT_PM_FCS_OK2_ADDR[PORT_ID_EAG6L_MAX];
extern unsigned long __PORT_PM_FCS_OK3_ADDR[PORT_ID_EAG6L_MAX];
extern unsigned long __PORT_PM_FCS_OK4_ADDR[PORT_ID_EAG6L_MAX];
extern unsigned long __PORT_PM_FCS_NOK1_ADDR[PORT_ID_EAG6L_MAX];
extern unsigned long __PORT_PM_FCS_NOK2_ADDR[PORT_ID_EAG6L_MAX];
extern unsigned long __PORT_PM_FCS_NOK3_ADDR[PORT_ID_EAG6L_MAX];
extern unsigned long __PORT_PM_FCS_NOK4_ADDR[PORT_ID_EAG6L_MAX];
extern unsigned long __PORT_CLEI1_ADDR[PORT_ID_EAG6L_MAX];
extern unsigned long __PORT_CLEI2_ADDR[PORT_ID_EAG6L_MAX];
extern unsigned long __PORT_CLEI3_ADDR[PORT_ID_EAG6L_MAX];
extern unsigned long __PORT_CLEI4_ADDR[PORT_ID_EAG6L_MAX];
extern unsigned long __PORT_CLEI5_ADDR[PORT_ID_EAG6L_MAX];
extern unsigned long __PORT_USI1_ADDR[PORT_ID_EAG6L_MAX];
extern unsigned long __PORT_USI2_ADDR[PORT_ID_EAG6L_MAX];
extern unsigned long __PORT_USI3_ADDR[PORT_ID_EAG6L_MAX];
extern unsigned long __PORT_USI4_ADDR[PORT_ID_EAG6L_MAX];
extern unsigned long __PORT_USI5_ADDR[PORT_ID_EAG6L_MAX];
extern unsigned long __PORT_USI6_ADDR[PORT_ID_EAG6L_MAX];
extern unsigned long __PORT_USI7_ADDR[PORT_ID_EAG6L_MAX];
extern unsigned long __PORT_USI8_ADDR[PORT_ID_EAG6L_MAX];
extern unsigned long __PORT_USI9_ADDR[PORT_ID_EAG6L_MAX];
extern unsigned long __PORT_USI10_ADDR[PORT_ID_EAG6L_MAX];
extern unsigned long __PORT_USI11_ADDR[PORT_ID_EAG6L_MAX];
extern unsigned long __PORT_USI12_ADDR[PORT_ID_EAG6L_MAX];
extern unsigned long __PORT_USI13_ADDR[PORT_ID_EAG6L_MAX];
#if 1/*[#71] EAG6L Board Bring-up, balkrow, 2024-07-04*/
typedef struct pll_reg_str {
	uint16_t reg;
	uint16_t val;
} PLL_REG;
#endif
#endif/*_BP_REGS_H_*/
