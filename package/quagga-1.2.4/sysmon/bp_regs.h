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
typedef uint16_t (*READ_FUNC) (uint16_t);
#if 1/*[#51] Adding register callback templates for config/command registers, dustin, 2024-06-12 */
typedef uint16_t (*REG_CALLBACK) (uint16_t/*portno*/, uint16_t/*reg-val*/);
#else
typedef uint16_t (*REG_CALLBACK) (uint16_t);
#endif

typedef struct reg_mon_str
{
	uint16_t reg;
	uint16_t mask;
	uint16_t shift;
	uint16_t val;
#if 1/*[#51] Adding register callback templates for config/command registers, dustin, 2024-06-12 */
	uint8_t  portno;/*if zero, then single register. if non-zero then per-port registers. */
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

#if 1/*[#53] Clock source status 업데이트 기능 추가, balkrow, 2024-06-13*/
#define PRI_SRC 1
#define SEC_SRC 0
#endif

#if 1/*[#56] register update timer 수정, balkrow, 2023-06-13 */
/* Start DPRAM registers*/
#define DPRAM_IMG_HDR 0x0
#define DPRAM_RDL_STATE 0x2
/* End DPRAM registers*/
#endif

/****************************
 * unit related registers
 ****************************/

#define SW_VERSION_ADDR			0x00	// r/o for mcu, r/w for bp
#define HW_KEEP_ALIVE_1_ADDR		0x14	// r/w for mcu, r/o for bp
#define HW_KEEP_ALIVE_2_ADDR		0x16	// r/o for mcu, r/w for bp :
                                            //   bp must increase this value.
#define INIT_COMPLETE_ADDR		0x18	// bp set 0xAA to [15:8] if init done.
#if 1/*[#53] Clock source status 업데이트 기능 추가, balkrow, 2024-06-13*/
#define INIT_COMPLETE_ADDR_MASK		0xFFF00
#endif

// bp must set 10G/25G speed for port 1~6.
#define COMMON_CTRL2_P1_ADDR		0x20	// r/w for mcu, r/o for bp 
#define COMMON_CTRL2_P2_ADDR		0x22	// r/w for mcu, r/o for bp 
#define COMMON_CTRL2_P3_ADDR		0x24	// r/w for mcu, r/o for bp 
#define COMMON_CTRL2_P4_ADDR		0x26	// r/w for mcu, r/o for bp 
#define COMMON_CTRL2_P5_ADDR		0x28	// r/w for mcu, r/o for bp 
#define COMMON_CTRL2_P6_ADDR		0x2A	// r/w for mcu, r/o for bp 
#define COMMON_CTRL2_P7_ADDR		0x2C	// r/w for mcu, r/o for bp 


/****************************
 * Reset/CR related registers
 ****************************/

#define CHIP_RESET_ADDR			0x20	// r/w for both mcu/bp.


/****************************
 * DCO related registers
 ****************************/

#define DCO_INTERFACE_ADDR		0x60	// r/w for mcu, r/o for bp 
#define DCO_WRITE_CMD_ADDR		0x62	// r/w for mcu, r/o for bp 
#define DCO_READ_CMD_ADDR		0x64	// r/w for mcu, r/o for bp 
#define DCO_ACCESS_METHOD_ADDR		0x66	// r/w for mcu, r/o for bp 


/****************************
 * DCO related registers
 ****************************/

// H/W manufacture
#define INV_HW_MANU_1_ADDR			0x370	// r/o for mcu, r/w for bp 
#define INV_HW_MANU_2_ADDR			0x372	// r/o for mcu, r/w for bp 
#define INV_HW_MANU_3_ADDR			0x374	// r/o for mcu, r/w for bp 
#define INV_HW_MANU_4_ADDR			0x376	// r/o for mcu, r/w for bp 
#define INV_HW_MANU_5_ADDR			0x378	// r/o for mcu, r/w for bp 

// H/W model
#define INV_HW_MODEL_1_ADDR			0x380	// r/o for mcu, r/w for bp 
#define INV_HW_MODEL_2_ADDR			0x382	// r/o for mcu, r/w for bp 
#define INV_HW_MODEL_3_ADDR			0x384	// r/o for mcu, r/w for bp 
#define INV_HW_MODEL_4_ADDR			0x386	// r/o for mcu, r/w for bp 
#define INV_HW_MODEL_5_ADDR			0x388	// r/o for mcu, r/w for bp 

// H/W part number
#define INV_HW_PN_1_ADDR			0x390	// r/o for mcu, r/w for bp 
#define INV_HW_PN_2_ADDR			0x392	// r/o for mcu, r/w for bp 
#define INV_HW_PN_3_ADDR			0x394	// r/o for mcu, r/w for bp 
#define INV_HW_PN_4_ADDR			0x396	// r/o for mcu, r/w for bp 
#define INV_HW_PN_5_ADDR			0x398	// r/o for mcu, r/w for bp 
#define INV_HW_PN_6_ADDR			0x39A	// r/o for mcu, r/w for bp 
#define INV_HW_PN_7_ADDR			0x39C	// r/o for mcu, r/w for bp 
#define INV_HW_PN_8_ADDR			0x39E	// r/o for mcu, r/w for bp 

// H/W serial number
#define INV_HW_SN_1_ADDR			0x3A0	// r/o for mcu, r/w for bp 
#define INV_HW_SN_2_ADDR			0x3A2	// r/o for mcu, r/w for bp 

// H/W revision number
#define INV_HW_REV_1_ADDR			0x3B0	// r/o for mcu, r/w for bp 
#define INV_HW_REV_2_ADDR			0x3B2	// r/o for mcu, r/w for bp 

// H/W manufacture date
#define INV_HW_MDATE_1_ADDR			0x3B4	// r/o for mcu, r/w for bp 
#define INV_HW_MDATE_2_ADDR			0x3B6	// r/o for mcu, r/w for bp 
#define INV_HW_MDATE_3_ADDR			0x3B8	// r/o for mcu, r/w for bp 
#define INV_HW_MDATE_4_ADDR			0x3BA	// r/o for mcu, r/w for bp 
#define INV_HW_MDATE_5_ADDR			0x3BC	// r/o for mcu, r/w for bp 
#define INV_HW_MDATE_6_ADDR			0x3BE	// r/o for mcu, r/w for bp 

// H/W repair date
#define INV_HW_RDATE_1_ADDR			0x3C0	// r/o for mcu, r/w for bp 
#define INV_HW_RDATE_2_ADDR			0x3C2	// r/o for mcu, r/w for bp 
#define INV_HW_RDATE_3_ADDR			0x3C4	// r/o for mcu, r/w for bp 
#define INV_HW_RDATE_4_ADDR			0x3C6	// r/o for mcu, r/w for bp 
#define INV_HW_RDATE_5_ADDR			0x3C8	// r/o for mcu, r/w for bp 

// H/W repair code
#define INV_HW_RCODE_1_ADDR			0x3CA	// r/o for mcu, r/w for bp 
#define INV_HW_RCODE_2_ADDR			0x3CC	// r/o for mcu, r/w for bp 

#if 1/*[#56] register update timer 수정, balkrow, 2023-06-13 */

#define BD_SFP_CR_ADDR				0x4C	// r/o for both mcu/bp. FPGA must provide this.

#endif

/****************************
 * Port Config related registers
 ****************************/
/*PORT BASE 6080_xxxx*/
#define PORT_1_CONF_ADDR			0x00	// r/w for mcu, r/o for bp 
#define PORT_2_CONF_ADDR			0x02	// r/w for mcu, r/o for bp 
#define PORT_3_CONF_ADDR			0x04	// r/w for mcu, r/o for bp 
#define PORT_4_CONF_ADDR			0x06	// r/w for mcu, r/o for bp 
#define PORT_5_CONF_ADDR			0x08	// r/w for mcu, r/o for bp 
#define PORT_6_CONF_ADDR			0x0A	// r/w for mcu, r/o for bp 
#define PORT_7_CONF_ADDR			0x0C	// r/w for mcu, r/o for bp 

#define PORT_7_CONF2_ADDR			0x0E	// r/w for mcu, r/o for bp 
#define PORT_7_CONF3_ADDR			0x1E	// r/w for mcu, r/o for bp 

/****************************
 * SyncE related registers
 ****************************/

#define SYNCE_GCONFIG_ADDR			0x160	// r/w for mcu, r/o for bp 
#define SYNCE_IF_SELECT_ADDR			0x162	// r/w for mcu, r/o for bp 
#define SYNCE_ESMC_LQL_ADDR			0x166	// r/o for mcu, r/w for bp 
#define SYNCE_ESMC_SQL_ADDR			0x168	// r/o for mcu, r/w for bp 
#define SYNCE_ESMC_RQL_ADDR			0x16A	// r/o for mcu, r/w for bp 
#define SYNCE_SRC_STAT_ADDR			0x16C	// r/o for mcu, r/w for bp 
#if 1/*[#53] Clock source status 업데이트 기능 추가, balkrow, 2024-06-13*/
#define SYNCE_SRC_STAT_ADDR_MASK		0xFFF00		// r/o for mcu, r/w for bp 
#endif

#define SYNCE_ESMC_RQL2_ADDR			0x16E	// r/o for mcu, r/w for bp 
#define SYNCE_ESMC_RQL3_ADDR			0x170	// r/o for mcu, r/w for bp 
#define SYNCE_ESMC_RQL4_ADDR			0x172	// r/o for mcu, r/w for bp 
#define SYNCE_ESMC_RQL5_ADDR			0x174	// r/o for mcu, r/w for bp 


/****************************
 * SFP Port Status/Control related registers
 ****************************/
#define PORT_1_TX_PWR_ADDR		0x20	// r/o for mcu, r/w for bp 
#define PORT_2_TX_PWR_ADDR		0x40	// r/o for mcu, r/w for bp 
#define PORT_3_TX_PWR_ADDR		0x60	// r/o for mcu, r/w for bp 
#define PORT_4_TX_PWR_ADDR		0x80	// r/o for mcu, r/w for bp 
#define PORT_5_TX_PWR_ADDR		0xA0	// r/o for mcu, r/w for bp 
#define PORT_6_TX_PWR_ADDR		0xC0	// r/o for mcu, r/w for bp 
#define PORT_7_TX_PWR_ADDR		0xE0	// r/o for mcu, r/w for bp 

#define PORT_1_RX_PWR_ADDR		0x22	// r/o for mcu, r/w for bp 
#define PORT_2_RX_PWR_ADDR		0x42	// r/o for mcu, r/w for bp 
#define PORT_3_RX_PWR_ADDR		0x62	// r/o for mcu, r/w for bp 
#define PORT_4_RX_PWR_ADDR		0x82	// r/o for mcu, r/w for bp 
#define PORT_5_RX_PWR_ADDR		0xA2	// r/o for mcu, r/w for bp 
#define PORT_6_RX_PWR_ADDR		0xC2	// r/o for mcu, r/w for bp 
#define PORT_7_RX_PWR_ADDR		0xE2	// r/o for mcu, r/w for bp 

#define PORT_1_WL1_ADDR			0x24	// r/o for mcu, r/w for bp 
#define PORT_2_WL1_ADDR			0x44	// r/o for mcu, r/w for bp 
#define PORT_3_WL1_ADDR			0x64	// r/o for mcu, r/w for bp 
#define PORT_4_WL1_ADDR			0x84	// r/o for mcu, r/w for bp 
#define PORT_5_WL1_ADDR			0xA4	// r/o for mcu, r/w for bp 
#define PORT_6_WL1_ADDR			0xC4	// r/o for mcu, r/w for bp 
#define PORT_7_WL1_ADDR			0xE4	// r/o for mcu, r/w for bp 

#define PORT_1_WL2_ADDR			0x26	// r/o for mcu, r/w for bp 
#define PORT_2_WL2_ADDR			0x46	// r/o for mcu, r/w for bp 
#define PORT_3_WL2_ADDR			0x66	// r/o for mcu, r/w for bp 
#define PORT_4_WL2_ADDR			0x86	// r/o for mcu, r/w for bp 
#define PORT_5_WL2_ADDR			0xA6	// r/o for mcu, r/w for bp 
#define PORT_6_WL2_ADDR			0xC6	// r/o for mcu, r/w for bp 
#define PORT_7_WL2_ADDR			0xE6	// r/o for mcu, r/w for bp 

#define PORT_1_DIST_ADDR		0x2C	// r/o for mcu, r/w for bp 
#define PORT_2_DIST_ADDR		0x4C	// r/o for mcu, r/w for bp 
#define PORT_3_DIST_ADDR		0x6C	// r/o for mcu, r/w for bp 
#define PORT_4_DIST_ADDR		0x8C	// r/o for mcu, r/w for bp 
#define PORT_5_DIST_ADDR		0xAC	// r/o for mcu, r/w for bp 
#define PORT_6_DIST_ADDR		0xCC	// r/o for mcu, r/w for bp 
#define PORT_7_DIST_ADDR		0xEC	// r/o for mcu, r/w for bp 

#define PORT_1_STSFP_STAT_ADDR		0x2E	// r/o for mcu, r/w for bp 
#define PORT_2_STSFP_STAT_ADDR		0x4E	// r/o for mcu, r/w for bp 
#define PORT_3_STSFP_STAT_ADDR		0x6E	// r/o for mcu, r/w for bp 
#define PORT_4_STSFP_STAT_ADDR		0x8E	// r/o for mcu, r/w for bp 
#define PORT_5_STSFP_STAT_ADDR		0xAE	// r/o for mcu, r/w for bp 
#define PORT_6_STSFP_STAT_ADDR		0xCE	// r/o for mcu, r/w for bp 
#define PORT_7_STSFP_STAT_ADDR		0xEE	// r/o for mcu, r/w for bp 

#define PORT_1_ALM_ADDR			0x30	// r/o for mcu, r/w for bp 
#define PORT_2_ALM_ADDR			0x50	// r/o for mcu, r/w for bp 
#define PORT_3_ALM_ADDR			0x70	// r/o for mcu, r/w for bp 
#define PORT_4_ALM_ADDR			0x90	// r/o for mcu, r/w for bp 
#define PORT_5_ALM_ADDR			0xB0	// r/o for mcu, r/w for bp 
#define PORT_6_ALM_ADDR			0xD0	// r/o for mcu, r/w for bp 
#define PORT_7_ALM_ADDR			0xF0	// r/o for mcu, r/w for bp 

#define PORT_1_ALM_FLAG_ADDR		0x32	// r/o for mcu, r/w for bp 
#define PORT_2_ALM_FLAG_ADDR		0x52	// r/o for mcu, r/w for bp 
#define PORT_3_ALM_FLAG_ADDR		0x72	// r/o for mcu, r/w for bp 
#define PORT_4_ALM_FLAG_ADDR		0x92	// r/o for mcu, r/w for bp 
#define PORT_5_ALM_FLAG_ADDR		0xB2	// r/o for mcu, r/w for bp 
#define PORT_6_ALM_FLAG_ADDR		0xD2	// r/o for mcu, r/w for bp 
#define PORT_7_ALM_FLAG_ADDR		0xF2	// r/o for mcu, r/w for bp 

#define PORT_1_ALM_MASK_ADDR		0x34	// r/o for mcu, r/w for bp 
#define PORT_2_ALM_MASK_ADDR		0x54	// r/o for mcu, r/w for bp 
#define PORT_3_ALM_MASK_ADDR		0x74	// r/o for mcu, r/w for bp 
#define PORT_4_ALM_MASK_ADDR		0x94	// r/o for mcu, r/w for bp 
#define PORT_5_ALM_MASK_ADDR		0xB4	// r/o for mcu, r/w for bp 
#define PORT_6_ALM_MASK_ADDR		0xD4	// r/o for mcu, r/w for bp 
#define PORT_7_ALM_MASK_ADDR		0xF4	// r/o for mcu, r/w for bp 

#define PORT_1_GET_CH_NUM_ADDR		0x36	// r/o for mcu, r/w for bp 
#define PORT_2_GET_CH_NUM_ADDR		0x56	// r/o for mcu, r/w for bp 
#define PORT_3_GET_CH_NUM_ADDR		0x76	// r/o for mcu, r/w for bp 
#define PORT_4_GET_CH_NUM_ADDR		0x96	// r/o for mcu, r/w for bp 
#define PORT_5_GET_CH_NUM_ADDR		0xB6	// r/o for mcu, r/w for bp 
#define PORT_6_GET_CH_NUM_ADDR		0xD6	// r/o for mcu, r/w for bp 
#define PORT_7_GET_CH_NUM_ADDR		0xF6	// r/o for mcu, r/w for bp 

#define PORT_1_SET_CH_NUM_ADDR		0x38	// r/w for mcu, r/o for bp 
#define PORT_2_SET_CH_NUM_ADDR		0x58	// r/w for mcu, r/o for bp 
#define PORT_3_SET_CH_NUM_ADDR		0x78	// r/w for mcu, r/o for bp 
#define PORT_4_SET_CH_NUM_ADDR		0x98	// r/w for mcu, r/o for bp 
#define PORT_5_SET_CH_NUM_ADDR		0xB8	// r/w for mcu, r/o for bp 
#define PORT_6_SET_CH_NUM_ADDR		0xD8	// r/w for mcu, r/o for bp 
#define PORT_7_SET_CH_NUM_ADDR		0xF8	// r/w for mcu, r/o for bp 


/****************************
 * Q/SFP Information related registers
 ****************************/

#define SFP_P1_VENDOR1_ADDR			0x400	// r/o for mcu, r/w for bp 
#define SFP_P2_VENDOR1_ADDR			0x440	// r/o for mcu, r/w for bp 
#define SFP_P3_VENDOR1_ADDR			0x480	// r/o for mcu, r/w for bp 
#define SFP_P4_VENDOR1_ADDR			0x4C0	// r/o for mcu, r/w for bp 
#define SFP_P5_VENDOR1_ADDR			0x500	// r/o for mcu, r/w for bp 
#define SFP_P6_VENDOR1_ADDR			0x540	// r/o for mcu, r/w for bp 
#define SFP_P7_VENDOR1_ADDR			0x580	// r/o for mcu, r/w for bp 

#define SFP_P1_VENDOR2_ADDR			0x402	// r/o for mcu, r/w for bp 
#define SFP_P2_VENDOR2_ADDR			0x442	// r/o for mcu, r/w for bp 
#define SFP_P3_VENDOR2_ADDR			0x482	// r/o for mcu, r/w for bp 
#define SFP_P4_VENDOR2_ADDR			0x4C2	// r/o for mcu, r/w for bp 
#define SFP_P5_VENDOR2_ADDR			0x502	// r/o for mcu, r/w for bp 
#define SFP_P6_VENDOR2_ADDR			0x542	// r/o for mcu, r/w for bp 
#define SFP_P7_VENDOR2_ADDR			0x582	// r/o for mcu, r/w for bp 

#define SFP_P1_VENDOR3_ADDR			0x404	// r/o for mcu, r/w for bp 
#define SFP_P2_VENDOR3_ADDR			0x444	// r/o for mcu, r/w for bp 
#define SFP_P3_VENDOR3_ADDR			0x484	// r/o for mcu, r/w for bp 
#define SFP_P4_VENDOR3_ADDR			0x4C4	// r/o for mcu, r/w for bp 
#define SFP_P5_VENDOR3_ADDR			0x504	// r/o for mcu, r/w for bp 
#define SFP_P6_VENDOR3_ADDR			0x544	// r/o for mcu, r/w for bp 
#define SFP_P7_VENDOR3_ADDR			0x584	// r/o for mcu, r/w for bp 

#define SFP_P1_VENDOR4_ADDR			0x406	// r/o for mcu, r/w for bp 
#define SFP_P2_VENDOR4_ADDR			0x446	// r/o for mcu, r/w for bp 
#define SFP_P3_VENDOR4_ADDR			0x486	// r/o for mcu, r/w for bp 
#define SFP_P4_VENDOR4_ADDR			0x4C6	// r/o for mcu, r/w for bp 
#define SFP_P5_VENDOR4_ADDR			0x506	// r/o for mcu, r/w for bp 
#define SFP_P6_VENDOR4_ADDR			0x546	// r/o for mcu, r/w for bp 
#define SFP_P7_VENDOR4_ADDR			0x586	// r/o for mcu, r/w for bp 

#define SFP_P1_VENDOR5_ADDR			0x408	// r/o for mcu, r/w for bp 
#define SFP_P2_VENDOR5_ADDR			0x448	// r/o for mcu, r/w for bp 
#define SFP_P3_VENDOR5_ADDR			0x488	// r/o for mcu, r/w for bp 
#define SFP_P4_VENDOR5_ADDR			0x4C8	// r/o for mcu, r/w for bp 
#define SFP_P5_VENDOR5_ADDR			0x508	// r/o for mcu, r/w for bp 
#define SFP_P6_VENDOR5_ADDR			0x548	// r/o for mcu, r/w for bp 
#define SFP_P7_VENDOR5_ADDR			0x588	// r/o for mcu, r/w for bp 

#define SFP_P1_VENDOR6_ADDR			0x40A	// r/o for mcu, r/w for bp 
#define SFP_P2_VENDOR6_ADDR			0x44A	// r/o for mcu, r/w for bp 
#define SFP_P3_VENDOR6_ADDR			0x48A	// r/o for mcu, r/w for bp 
#define SFP_P4_VENDOR6_ADDR			0x4CA	// r/o for mcu, r/w for bp 
#define SFP_P5_VENDOR6_ADDR			0x50A	// r/o for mcu, r/w for bp 
#define SFP_P6_VENDOR6_ADDR			0x54A	// r/o for mcu, r/w for bp 
#define SFP_P7_VENDOR6_ADDR			0x58A	// r/o for mcu, r/w for bp 

#define SFP_P1_VENDOR7_ADDR			0x40C	// r/o for mcu, r/w for bp 
#define SFP_P2_VENDOR7_ADDR			0x44C	// r/o for mcu, r/w for bp 
#define SFP_P3_VENDOR7_ADDR			0x48C	// r/o for mcu, r/w for bp 
#define SFP_P4_VENDOR7_ADDR			0x4CC	// r/o for mcu, r/w for bp 
#define SFP_P5_VENDOR7_ADDR			0x50C	// r/o for mcu, r/w for bp 
#define SFP_P6_VENDOR7_ADDR			0x54C	// r/o for mcu, r/w for bp 
#define SFP_P7_VENDOR7_ADDR			0x58C	// r/o for mcu, r/w for bp 

#define SFP_P1_VENDOR8_ADDR			0x40E	// r/o for mcu, r/w for bp 
#define SFP_P2_VENDOR8_ADDR			0x44E	// r/o for mcu, r/w for bp 
#define SFP_P3_VENDOR8_ADDR			0x48E	// r/o for mcu, r/w for bp 
#define SFP_P4_VENDOR8_ADDR			0x4CE	// r/o for mcu, r/w for bp 
#define SFP_P5_VENDOR8_ADDR			0x50E	// r/o for mcu, r/w for bp 
#define SFP_P6_VENDOR8_ADDR			0x54E	// r/o for mcu, r/w for bp 
#define SFP_P7_VENDOR8_ADDR			0x58E	// r/o for mcu, r/w for bp 

#define SFP_P1_PN1_ADDR				0x410	// r/o for mcu, r/w for bp 
#define SFP_P2_PN1_ADDR				0x450	// r/o for mcu, r/w for bp 
#define SFP_P3_PN1_ADDR				0x490	// r/o for mcu, r/w for bp 
#define SFP_P4_PN1_ADDR				0x4D0	// r/o for mcu, r/w for bp 
#define SFP_P5_PN1_ADDR				0x510	// r/o for mcu, r/w for bp 
#define SFP_P6_PN1_ADDR				0x550	// r/o for mcu, r/w for bp 
#define SFP_P7_PN1_ADDR				0x590	// r/o for mcu, r/w for bp 

#define SFP_P1_PN2_ADDR				0x412	// r/o for mcu, r/w for bp 
#define SFP_P2_PN2_ADDR				0x452	// r/o for mcu, r/w for bp 
#define SFP_P3_PN2_ADDR				0x492	// r/o for mcu, r/w for bp 
#define SFP_P4_PN2_ADDR				0x4D2	// r/o for mcu, r/w for bp 
#define SFP_P5_PN2_ADDR				0x512	// r/o for mcu, r/w for bp 
#define SFP_P6_PN2_ADDR				0x552	// r/o for mcu, r/w for bp 
#define SFP_P7_PN2_ADDR				0x592	// r/o for mcu, r/w for bp 

#define SFP_P1_PN3_ADDR				0x414	// r/o for mcu, r/w for bp 
#define SFP_P2_PN3_ADDR				0x454	// r/o for mcu, r/w for bp 
#define SFP_P3_PN3_ADDR				0x494	// r/o for mcu, r/w for bp 
#define SFP_P4_PN3_ADDR				0x4D4	// r/o for mcu, r/w for bp 
#define SFP_P5_PN3_ADDR				0x514	// r/o for mcu, r/w for bp 
#define SFP_P6_PN3_ADDR				0x554	// r/o for mcu, r/w for bp 
#define SFP_P7_PN3_ADDR				0x594	// r/o for mcu, r/w for bp 

#define SFP_P1_PN4_ADDR				0x416	// r/o for mcu, r/w for bp 
#define SFP_P2_PN4_ADDR				0x456	// r/o for mcu, r/w for bp 
#define SFP_P3_PN4_ADDR				0x496	// r/o for mcu, r/w for bp 
#define SFP_P4_PN4_ADDR				0x4D6	// r/o for mcu, r/w for bp 
#define SFP_P5_PN4_ADDR				0x516	// r/o for mcu, r/w for bp 
#define SFP_P6_PN4_ADDR				0x556	// r/o for mcu, r/w for bp 
#define SFP_P7_PN4_ADDR				0x596	// r/o for mcu, r/w for bp 

#define SFP_P1_PN5_ADDR				0x418	// r/o for mcu, r/w for bp 
#define SFP_P2_PN5_ADDR				0x458	// r/o for mcu, r/w for bp 
#define SFP_P3_PN5_ADDR				0x498	// r/o for mcu, r/w for bp 
#define SFP_P4_PN5_ADDR				0x4D8	// r/o for mcu, r/w for bp 
#define SFP_P5_PN5_ADDR				0x518	// r/o for mcu, r/w for bp 
#define SFP_P6_PN5_ADDR				0x558	// r/o for mcu, r/w for bp 
#define SFP_P7_PN5_ADDR				0x598	// r/o for mcu, r/w for bp 

#define SFP_P1_PN6_ADDR				0x41A	// r/o for mcu, r/w for bp 
#define SFP_P2_PN6_ADDR				0x45A	// r/o for mcu, r/w for bp 
#define SFP_P3_PN6_ADDR				0x49A	// r/o for mcu, r/w for bp 
#define SFP_P4_PN6_ADDR				0x4DA	// r/o for mcu, r/w for bp 
#define SFP_P5_PN6_ADDR				0x51A	// r/o for mcu, r/w for bp 
#define SFP_P6_PN6_ADDR				0x55A	// r/o for mcu, r/w for bp 
#define SFP_P7_PN6_ADDR				0x59A	// r/o for mcu, r/w for bp 

#define SFP_P1_PN7_ADDR				0x41C	// r/o for mcu, r/w for bp 
#define SFP_P2_PN7_ADDR				0x45C	// r/o for mcu, r/w for bp 
#define SFP_P3_PN7_ADDR				0x49C	// r/o for mcu, r/w for bp 
#define SFP_P4_PN7_ADDR				0x4DC	// r/o for mcu, r/w for bp 
#define SFP_P5_PN7_ADDR				0x51C	// r/o for mcu, r/w for bp 
#define SFP_P6_PN7_ADDR				0x55C	// r/o for mcu, r/w for bp 
#define SFP_P7_PN7_ADDR				0x59C	// r/o for mcu, r/w for bp 

#define SFP_P1_PN8_ADDR				0x41E	// r/o for mcu, r/w for bp 
#define SFP_P2_PN8_ADDR				0x45E	// r/o for mcu, r/w for bp 
#define SFP_P3_PN8_ADDR				0x49E	// r/o for mcu, r/w for bp 
#define SFP_P4_PN8_ADDR				0x4DE	// r/o for mcu, r/w for bp 
#define SFP_P5_PN8_ADDR				0x51E	// r/o for mcu, r/w for bp 
#define SFP_P6_PN8_ADDR				0x55E	// r/o for mcu, r/w for bp 
#define SFP_P7_PN8_ADDR				0x59E	// r/o for mcu, r/w for bp 

#define SFP_P1_SN1_ADDR				0x420	// r/o for mcu, r/w for bp 
#define SFP_P2_SN1_ADDR				0x460	// r/o for mcu, r/w for bp 
#define SFP_P3_SN1_ADDR				0x4A0	// r/o for mcu, r/w for bp 
#define SFP_P4_SN1_ADDR				0x4E0	// r/o for mcu, r/w for bp 
#define SFP_P5_SN1_ADDR				0x520	// r/o for mcu, r/w for bp 
#define SFP_P6_SN1_ADDR				0x560	// r/o for mcu, r/w for bp 
#define SFP_P7_SN1_ADDR				0x5A0	// r/o for mcu, r/w for bp 

#define SFP_P1_SN2_ADDR				0x422	// r/o for mcu, r/w for bp 
#define SFP_P2_SN2_ADDR				0x462	// r/o for mcu, r/w for bp 
#define SFP_P3_SN2_ADDR				0x4A2	// r/o for mcu, r/w for bp 
#define SFP_P4_SN2_ADDR				0x4E2	// r/o for mcu, r/w for bp 
#define SFP_P5_SN2_ADDR				0x522	// r/o for mcu, r/w for bp 
#define SFP_P6_SN2_ADDR				0x562	// r/o for mcu, r/w for bp 
#define SFP_P7_SN2_ADDR				0x5A2	// r/o for mcu, r/w for bp 

#define SFP_P1_SN3_ADDR				0x424	// r/o for mcu, r/w for bp 
#define SFP_P2_SN3_ADDR				0x464	// r/o for mcu, r/w for bp 
#define SFP_P3_SN3_ADDR				0x4A4	// r/o for mcu, r/w for bp 
#define SFP_P4_SN3_ADDR				0x4E4	// r/o for mcu, r/w for bp 
#define SFP_P5_SN3_ADDR				0x524	// r/o for mcu, r/w for bp 
#define SFP_P6_SN3_ADDR				0x564	// r/o for mcu, r/w for bp 
#define SFP_P7_SN3_ADDR				0x5A4	// r/o for mcu, r/w for bp 

#define SFP_P1_SN4_ADDR				0x426	// r/o for mcu, r/w for bp 
#define SFP_P2_SN4_ADDR				0x466	// r/o for mcu, r/w for bp 
#define SFP_P3_SN4_ADDR				0x4A6	// r/o for mcu, r/w for bp 
#define SFP_P4_SN4_ADDR				0x4E6	// r/o for mcu, r/w for bp 
#define SFP_P5_SN4_ADDR				0x526	// r/o for mcu, r/w for bp 
#define SFP_P6_SN4_ADDR				0x566	// r/o for mcu, r/w for bp 
#define SFP_P7_SN4_ADDR				0x5A6	// r/o for mcu, r/w for bp 

#define SFP_P1_SN5_ADDR				0x428	// r/o for mcu, r/w for bp 
#define SFP_P2_SN5_ADDR				0x468	// r/o for mcu, r/w for bp 
#define SFP_P3_SN5_ADDR				0x4A8	// r/o for mcu, r/w for bp 
#define SFP_P4_SN5_ADDR				0x4E8	// r/o for mcu, r/w for bp 
#define SFP_P5_SN5_ADDR				0x528	// r/o for mcu, r/w for bp 
#define SFP_P6_SN5_ADDR				0x568	// r/o for mcu, r/w for bp 
#define SFP_P7_SN5_ADDR				0x5A8	// r/o for mcu, r/w for bp 

#define SFP_P1_SN6_ADDR				0x42A	// r/o for mcu, r/w for bp 
#define SFP_P2_SN6_ADDR				0x46A	// r/o for mcu, r/w for bp 
#define SFP_P3_SN6_ADDR				0x4AA	// r/o for mcu, r/w for bp 
#define SFP_P4_SN6_ADDR				0x4EA	// r/o for mcu, r/w for bp 
#define SFP_P5_SN6_ADDR				0x52A	// r/o for mcu, r/w for bp 
#define SFP_P6_SN6_ADDR				0x56A	// r/o for mcu, r/w for bp 
#define SFP_P7_SN6_ADDR				0x5AA	// r/o for mcu, r/w for bp 

#define SFP_P1_SN7_ADDR				0x42C	// r/o for mcu, r/w for bp 
#define SFP_P2_SN7_ADDR				0x46C	// r/o for mcu, r/w for bp 
#define SFP_P3_SN7_ADDR				0x4AC	// r/o for mcu, r/w for bp 
#define SFP_P4_SN7_ADDR				0x4EC	// r/o for mcu, r/w for bp 
#define SFP_P5_SN7_ADDR				0x52C	// r/o for mcu, r/w for bp 
#define SFP_P6_SN7_ADDR				0x56C	// r/o for mcu, r/w for bp 
#define SFP_P7_SN7_ADDR				0x5AC	// r/o for mcu, r/w for bp 

#define SFP_P1_SN8_ADDR				0x42E	// r/o for mcu, r/w for bp 
#define SFP_P2_SN8_ADDR				0x46E	// r/o for mcu, r/w for bp 
#define SFP_P3_SN8_ADDR				0x4AE	// r/o for mcu, r/w for bp 
#define SFP_P4_SN8_ADDR				0x4EE	// r/o for mcu, r/w for bp 
#define SFP_P5_SN8_ADDR				0x52E	// r/o for mcu, r/w for bp 
#define SFP_P6_SN8_ADDR				0x56E	// r/o for mcu, r/w for bp 
#define SFP_P7_SN8_ADDR				0x5AE	// r/o for mcu, r/w for bp 

#define SFP_P1_TEMP_ADDR			0x430	// r/o for mcu, r/w for bp 
#define SFP_P2_TEMP_ADDR			0x470	// r/o for mcu, r/w for bp 
#define SFP_P3_TEMP_ADDR			0x4B0	// r/o for mcu, r/w for bp 
#define SFP_P4_TEMP_ADDR			0x4F0	// r/o for mcu, r/w for bp 
#define SFP_P5_TEMP_ADDR			0x530	// r/o for mcu, r/w for bp 
#define SFP_P6_TEMP_ADDR			0x570	// r/o for mcu, r/w for bp 
#define SFP_P7_TEMP_ADDR			0x5B0	// r/o for mcu, r/w for bp 

#define SFP_P1_RATE_ADDR			0x432	// r/o for mcu, r/w for bp 
#define SFP_P2_RATE_ADDR			0x472	// r/o for mcu, r/w for bp 
#define SFP_P3_RATE_ADDR			0x4B2	// r/o for mcu, r/w for bp 
#define SFP_P4_RATE_ADDR			0x4F2	// r/o for mcu, r/w for bp 
#define SFP_P5_RATE_ADDR			0x532	// r/o for mcu, r/w for bp 
#define SFP_P6_RATE_ADDR			0x572	// r/o for mcu, r/w for bp 
#define SFP_P7_RATE_ADDR			0x5B2	// r/o for mcu, r/w for bp 

#define SFP_P1_VOLT_ADDR			0x434	// r/o for mcu, r/w for bp 
#define SFP_P2_VOLT_ADDR			0x474	// r/o for mcu, r/w for bp 
#define SFP_P3_VOLT_ADDR			0x4B4	// r/o for mcu, r/w for bp 
#define SFP_P4_VOLT_ADDR			0x4F4	// r/o for mcu, r/w for bp 
#define SFP_P5_VOLT_ADDR			0x534	// r/o for mcu, r/w for bp 
#define SFP_P6_VOLT_ADDR			0x574	// r/o for mcu, r/w for bp 
#define SFP_P7_VOLT_ADDR			0x5B4	// r/o for mcu, r/w for bp 

#define SFP_P1_TX_BIAS_ADDR			0x436	// r/o for mcu, r/w for bp 
#define SFP_P2_TX_BIAS_ADDR			0x476	// r/o for mcu, r/w for bp 
#define SFP_P3_TX_BIAS_ADDR			0x4B6	// r/o for mcu, r/w for bp 
#define SFP_P4_TX_BIAS_ADDR			0x4F6	// r/o for mcu, r/w for bp 
#define SFP_P5_TX_BIAS_ADDR			0x536	// r/o for mcu, r/w for bp 
#define SFP_P6_TX_BIAS_ADDR			0x576	// r/o for mcu, r/w for bp 
#define SFP_P7_TX_BIAS_ADDR			0x5B6	// r/o for mcu, r/w for bp 

#define SFP_P1_LTEMP_ADDR			0x438	// r/o for mcu, r/w for bp 
#define SFP_P2_LTEMP_ADDR			0x478	// r/o for mcu, r/w for bp 
#define SFP_P3_LTEMP_ADDR			0x4B8	// r/o for mcu, r/w for bp 
#define SFP_P4_LTEMP_ADDR			0x4F8	// r/o for mcu, r/w for bp 
#define SFP_P5_LTEMP_ADDR			0x538	// r/o for mcu, r/w for bp 
#define SFP_P6_LTEMP_ADDR			0x578	// r/o for mcu, r/w for bp 
#define SFP_P7_LTEMP_ADDR			0x5B8	// r/o for mcu, r/w for bp 

#define SFP_P1_TCURR_ADDR			0x43A	// r/o for mcu, r/w for bp 
#define SFP_P2_TCURR_ADDR			0x47A	// r/o for mcu, r/w for bp 
#define SFP_P3_TCURR_ADDR			0x4BA	// r/o for mcu, r/w for bp 
#define SFP_P4_TCURR_ADDR			0x4FA	// r/o for mcu, r/w for bp 
#define SFP_P5_TCURR_ADDR			0x53A	// r/o for mcu, r/w for bp 
#define SFP_P6_TCURR_ADDR			0x57A	// r/o for mcu, r/w for bp 
#define SFP_P7_TCURR_ADDR			0x5BA	// r/o for mcu, r/w for bp 


/****************************
 * Performance related registers
 ****************************/

#define PM_COUNT_CLEAR_ADDR			0x100	// r/w for mcu, r/o for bp, read clear

#define PM_P1_TX_BYTE1_ADDR			0x700	// r/o for mcu, r/w for bp 
#define PM_P2_TX_BYTE1_ADDR			0x750	// r/o for mcu, r/w for bp 
#define PM_P3_TX_BYTE1_ADDR			0x7A0	// r/o for mcu, r/w for bp 
#define PM_P4_TX_BYTE1_ADDR			0x7F0	// r/o for mcu, r/w for bp 
#define PM_P5_TX_BYTE1_ADDR			0x840	// r/o for mcu, r/w for bp 
#define PM_P6_TX_BYTE1_ADDR			0x890	// r/o for mcu, r/w for bp 
#define PM_P7_TX_BYTE1_ADDR			0x8E0	// r/o for mcu, r/w for bp 

#define PM_P1_TX_BYTE2_ADDR			0x702	// r/o for mcu, r/w for bp 
#define PM_P2_TX_BYTE2_ADDR			0x752	// r/o for mcu, r/w for bp 
#define PM_P3_TX_BYTE2_ADDR			0x7A2	// r/o for mcu, r/w for bp 
#define PM_P4_TX_BYTE2_ADDR			0x7F2	// r/o for mcu, r/w for bp 
#define PM_P5_TX_BYTE2_ADDR			0x842	// r/o for mcu, r/w for bp 
#define PM_P6_TX_BYTE2_ADDR			0x892	// r/o for mcu, r/w for bp 
#define PM_P7_TX_BYTE2_ADDR			0x8E2	// r/o for mcu, r/w for bp 

#define PM_P1_TX_BYTE3_ADDR			0x704	// r/o for mcu, r/w for bp 
#define PM_P2_TX_BYTE3_ADDR			0x754	// r/o for mcu, r/w for bp 
#define PM_P3_TX_BYTE3_ADDR			0x7A4	// r/o for mcu, r/w for bp 
#define PM_P4_TX_BYTE3_ADDR			0x7F4	// r/o for mcu, r/w for bp 
#define PM_P5_TX_BYTE3_ADDR			0x844	// r/o for mcu, r/w for bp 
#define PM_P6_TX_BYTE3_ADDR			0x894	// r/o for mcu, r/w for bp 
#define PM_P7_TX_BYTE3_ADDR			0x8E4	// r/o for mcu, r/w for bp 

#define PM_P1_TX_BYTE4_ADDR			0x706	// r/o for mcu, r/w for bp 
#define PM_P2_TX_BYTE4_ADDR			0x756	// r/o for mcu, r/w for bp 
#define PM_P3_TX_BYTE4_ADDR			0x7A6	// r/o for mcu, r/w for bp 
#define PM_P4_TX_BYTE4_ADDR			0x7F6	// r/o for mcu, r/w for bp 
#define PM_P5_TX_BYTE4_ADDR			0x846	// r/o for mcu, r/w for bp 
#define PM_P6_TX_BYTE4_ADDR			0x896	// r/o for mcu, r/w for bp 
#define PM_P7_TX_BYTE4_ADDR			0x8E6	// r/o for mcu, r/w for bp 

#define PM_P1_RX_BYTE1_ADDR			0x708	// r/o for mcu, r/w for bp 
#define PM_P2_RX_BYTE1_ADDR			0x758	// r/o for mcu, r/w for bp 
#define PM_P3_RX_BYTE1_ADDR			0x7A8	// r/o for mcu, r/w for bp 
#define PM_P4_RX_BYTE1_ADDR			0x7F8	// r/o for mcu, r/w for bp 
#define PM_P5_RX_BYTE1_ADDR			0x848	// r/o for mcu, r/w for bp 
#define PM_P6_RX_BYTE1_ADDR			0x898	// r/o for mcu, r/w for bp 
#define PM_P7_RX_BYTE1_ADDR			0x8E8	// r/o for mcu, r/w for bp 

#define PM_P1_RX_BYTE2_ADDR			0x70A	// r/o for mcu, r/w for bp 
#define PM_P2_RX_BYTE2_ADDR			0x75A	// r/o for mcu, r/w for bp 
#define PM_P3_RX_BYTE2_ADDR			0x7AA	// r/o for mcu, r/w for bp 
#define PM_P4_RX_BYTE2_ADDR			0x7FA	// r/o for mcu, r/w for bp 
#define PM_P5_RX_BYTE2_ADDR			0x84A	// r/o for mcu, r/w for bp 
#define PM_P6_RX_BYTE2_ADDR			0x89A	// r/o for mcu, r/w for bp 
#define PM_P7_RX_BYTE2_ADDR			0x8EA	// r/o for mcu, r/w for bp 

#define PM_P1_RX_BYTE3_ADDR			0x70C	// r/o for mcu, r/w for bp 
#define PM_P2_RX_BYTE3_ADDR			0x75C	// r/o for mcu, r/w for bp 
#define PM_P3_RX_BYTE3_ADDR			0x7AC	// r/o for mcu, r/w for bp 
#define PM_P4_RX_BYTE3_ADDR			0x7FC	// r/o for mcu, r/w for bp 
#define PM_P5_RX_BYTE3_ADDR			0x84C	// r/o for mcu, r/w for bp 
#define PM_P6_RX_BYTE3_ADDR			0x89C	// r/o for mcu, r/w for bp 
#define PM_P7_RX_BYTE3_ADDR			0x8EC	// r/o for mcu, r/w for bp 

#define PM_P1_RX_BYTE4_ADDR			0x70E	// r/o for mcu, r/w for bp 
#define PM_P2_RX_BYTE4_ADDR			0x75E	// r/o for mcu, r/w for bp 
#define PM_P3_RX_BYTE4_ADDR			0x7AE	// r/o for mcu, r/w for bp 
#define PM_P4_RX_BYTE4_ADDR			0x7FE	// r/o for mcu, r/w for bp 
#define PM_P5_RX_BYTE4_ADDR			0x84E	// r/o for mcu, r/w for bp 
#define PM_P6_RX_BYTE4_ADDR			0x89E	// r/o for mcu, r/w for bp 
#define PM_P7_RX_BYTE4_ADDR			0x8EE	// r/o for mcu, r/w for bp 

#define PM_P1_TX_FRAME1_ADDR		0x710	// r/o for mcu, r/w for bp 
#define PM_P2_TX_FRAME1_ADDR		0x760	// r/o for mcu, r/w for bp 
#define PM_P3_TX_FRAME1_ADDR		0x7B0	// r/o for mcu, r/w for bp 
#define PM_P4_TX_FRAME1_ADDR		0x800	// r/o for mcu, r/w for bp 
#define PM_P5_TX_FRAME1_ADDR		0x850	// r/o for mcu, r/w for bp 
#define PM_P6_TX_FRAME1_ADDR		0x8A0	// r/o for mcu, r/w for bp 
#define PM_P7_TX_FRAME1_ADDR		0x8F0	// r/o for mcu, r/w for bp 

#define PM_P1_TX_FRAME2_ADDR		0x712	// r/o for mcu, r/w for bp 
#define PM_P2_TX_FRAME2_ADDR		0x762	// r/o for mcu, r/w for bp 
#define PM_P3_TX_FRAME2_ADDR		0x7B2	// r/o for mcu, r/w for bp 
#define PM_P4_TX_FRAME2_ADDR		0x802	// r/o for mcu, r/w for bp 
#define PM_P5_TX_FRAME2_ADDR		0x852	// r/o for mcu, r/w for bp 
#define PM_P6_TX_FRAME2_ADDR		0x8A2	// r/o for mcu, r/w for bp 
#define PM_P7_TX_FRAME2_ADDR		0x8F2	// r/o for mcu, r/w for bp 

#define PM_P1_TX_FRAME3_ADDR		0x714	// r/o for mcu, r/w for bp 
#define PM_P2_TX_FRAME3_ADDR		0x764	// r/o for mcu, r/w for bp 
#define PM_P3_TX_FRAME3_ADDR		0x7B4	// r/o for mcu, r/w for bp 
#define PM_P4_TX_FRAME3_ADDR		0x804	// r/o for mcu, r/w for bp 
#define PM_P5_TX_FRAME3_ADDR		0x854	// r/o for mcu, r/w for bp 
#define PM_P6_TX_FRAME3_ADDR		0x8A4	// r/o for mcu, r/w for bp 
#define PM_P7_TX_FRAME3_ADDR		0x8F4	// r/o for mcu, r/w for bp 

#define PM_P1_TX_FRAME4_ADDR		0x716	// r/o for mcu, r/w for bp 
#define PM_P2_TX_FRAME4_ADDR		0x766	// r/o for mcu, r/w for bp 
#define PM_P3_TX_FRAME4_ADDR		0x7B6	// r/o for mcu, r/w for bp 
#define PM_P4_TX_FRAME4_ADDR		0x806	// r/o for mcu, r/w for bp 
#define PM_P5_TX_FRAME4_ADDR		0x856	// r/o for mcu, r/w for bp 
#define PM_P6_TX_FRAME4_ADDR		0x8A6	// r/o for mcu, r/w for bp 
#define PM_P7_TX_FRAME4_ADDR		0x8F6	// r/o for mcu, r/w for bp 

#define PM_P1_RX_FRAME1_ADDR		0x718	// r/o for mcu, r/w for bp 
#define PM_P2_RX_FRAME1_ADDR		0x768	// r/o for mcu, r/w for bp 
#define PM_P3_RX_FRAME1_ADDR		0x7B8	// r/o for mcu, r/w for bp 
#define PM_P4_RX_FRAME1_ADDR		0x808	// r/o for mcu, r/w for bp 
#define PM_P5_RX_FRAME1_ADDR		0x858	// r/o for mcu, r/w for bp 
#define PM_P6_RX_FRAME1_ADDR		0x8A8	// r/o for mcu, r/w for bp 
#define PM_P7_RX_FRAME1_ADDR		0x8F8	// r/o for mcu, r/w for bp 

#define PM_P1_RX_FRAME2_ADDR		0x71A	// r/o for mcu, r/w for bp 
#define PM_P2_RX_FRAME2_ADDR		0x76A	// r/o for mcu, r/w for bp 
#define PM_P3_RX_FRAME2_ADDR		0x7BA	// r/o for mcu, r/w for bp 
#define PM_P4_RX_FRAME2_ADDR		0x80A	// r/o for mcu, r/w for bp 
#define PM_P5_RX_FRAME2_ADDR		0x85A	// r/o for mcu, r/w for bp 
#define PM_P6_RX_FRAME2_ADDR		0x8AA	// r/o for mcu, r/w for bp 
#define PM_P7_RX_FRAME2_ADDR		0x8FA	// r/o for mcu, r/w for bp 

#define PM_P1_RX_FRAME3_ADDR		0x71C	// r/o for mcu, r/w for bp 
#define PM_P2_RX_FRAME3_ADDR		0x76C	// r/o for mcu, r/w for bp 
#define PM_P3_RX_FRAME3_ADDR		0x7BC	// r/o for mcu, r/w for bp 
#define PM_P4_RX_FRAME3_ADDR		0x80C	// r/o for mcu, r/w for bp 
#define PM_P5_RX_FRAME3_ADDR		0x85C	// r/o for mcu, r/w for bp 
#define PM_P6_RX_FRAME3_ADDR		0x8AC	// r/o for mcu, r/w for bp 
#define PM_P7_RX_FRAME3_ADDR		0x8FC	// r/o for mcu, r/w for bp 

#define PM_P1_RX_FRAME4_ADDR		0x71E	// r/o for mcu, r/w for bp 
#define PM_P2_RX_FRAME4_ADDR		0x76E	// r/o for mcu, r/w for bp 
#define PM_P3_RX_FRAME4_ADDR		0x7BE	// r/o for mcu, r/w for bp 
#define PM_P4_RX_FRAME4_ADDR		0x80E	// r/o for mcu, r/w for bp 
#define PM_P5_RX_FRAME4_ADDR		0x85E	// r/o for mcu, r/w for bp 
#define PM_P6_RX_FRAME4_ADDR		0x8AE	// r/o for mcu, r/w for bp 
#define PM_P7_RX_FRAME4_ADDR		0x8FE	// r/o for mcu, r/w for bp 

#define PM_P1_FCS1_ADDR			0x720	// r/o for mcu, r/w for bp 
#define PM_P2_FCS1_ADDR	     	0x770	// r/o for mcu, r/w for bp 
#define PM_P3_FCS1_ADDR	     	0x7C0	// r/o for mcu, r/w for bp 
#define PM_P4_FCS1_ADDR	     	0x810	// r/o for mcu, r/w for bp 
#define PM_P5_FCS1_ADDR	     	0x860	// r/o for mcu, r/w for bp 
#define PM_P6_FCS1_ADDR	     	0x8B0	// r/o for mcu, r/w for bp 
#define PM_P7_FCS1_ADDR	     	0x900	// r/o for mcu, r/w for bp 

#define PM_P1_FCS2_ADDR	     	0x722	// r/o for mcu, r/w for bp 
#define PM_P2_FCS2_ADDR	     	0x772	// r/o for mcu, r/w for bp 
#define PM_P3_FCS2_ADDR	     	0x7C2	// r/o for mcu, r/w for bp 
#define PM_P4_FCS2_ADDR	     	0x812	// r/o for mcu, r/w for bp 
#define PM_P5_FCS2_ADDR	     	0x862	// r/o for mcu, r/w for bp 
#define PM_P6_FCS2_ADDR	     	0x8B2	// r/o for mcu, r/w for bp 
#define PM_P7_FCS2_ADDR	     	0x902	// r/o for mcu, r/w for bp 

#define PM_P1_FCS3_ADDR	     	0x724	// r/o for mcu, r/w for bp 
#define PM_P2_FCS3_ADDR	     	0x774	// r/o for mcu, r/w for bp 
#define PM_P3_FCS3_ADDR	     	0x7C4	// r/o for mcu, r/w for bp 
#define PM_P4_FCS3_ADDR	     	0x814	// r/o for mcu, r/w for bp 
#define PM_P5_FCS3_ADDR	     	0x864	// r/o for mcu, r/w for bp 
#define PM_P6_FCS3_ADDR	     	0x8B4	// r/o for mcu, r/w for bp 
#define PM_P7_FCS3_ADDR	     	0x904	// r/o for mcu, r/w for bp 

#define PM_P1_FCS4_ADDR	     	0x726	// r/o for mcu, r/w for bp 
#define PM_P2_FCS4_ADDR	     	0x776	// r/o for mcu, r/w for bp 
#define PM_P3_FCS4_ADDR	     	0x7C6	// r/o for mcu, r/w for bp 
#define PM_P4_FCS4_ADDR	     	0x816	// r/o for mcu, r/w for bp 
#define PM_P5_FCS4_ADDR	     	0x866	// r/o for mcu, r/w for bp 
#define PM_P6_FCS4_ADDR	     	0x8B6	// r/o for mcu, r/w for bp 
#define PM_P7_FCS4_ADDR	     	0x906	// r/o for mcu, r/w for bp 

#define PM_P1_FCS_OK1_ADDR   	0x730	// r/o for mcu, r/w for bp 
#define PM_P2_FCS_OK1_ADDR   	0x780	// r/o for mcu, r/w for bp 
#define PM_P3_FCS_OK1_ADDR   	0x7D0	// r/o for mcu, r/w for bp 
#define PM_P4_FCS_OK1_ADDR   	0x820	// r/o for mcu, r/w for bp 
#define PM_P5_FCS_OK1_ADDR   	0x870	// r/o for mcu, r/w for bp 
#define PM_P6_FCS_OK1_ADDR   	0x8C0	// r/o for mcu, r/w for bp 
#define PM_P7_FCS_OK1_ADDR   	0x910	// r/o for mcu, r/w for bp 

#define PM_P1_FCS_OK2_ADDR   	0x732	// r/o for mcu, r/w for bp 
#define PM_P2_FCS_OK2_ADDR   	0x782	// r/o for mcu, r/w for bp 
#define PM_P3_FCS_OK2_ADDR   	0x7D2	// r/o for mcu, r/w for bp 
#define PM_P4_FCS_OK2_ADDR   	0x822	// r/o for mcu, r/w for bp 
#define PM_P5_FCS_OK2_ADDR   	0x872	// r/o for mcu, r/w for bp 
#define PM_P6_FCS_OK2_ADDR   	0x8C2	// r/o for mcu, r/w for bp 
#define PM_P7_FCS_OK2_ADDR   	0x912	// r/o for mcu, r/w for bp 

#define PM_P1_FCS_OK3_ADDR   	0x734	// r/o for mcu, r/w for bp 
#define PM_P2_FCS_OK3_ADDR   	0x784	// r/o for mcu, r/w for bp 
#define PM_P3_FCS_OK3_ADDR   	0x7D4	// r/o for mcu, r/w for bp 
#define PM_P4_FCS_OK3_ADDR   	0x824	// r/o for mcu, r/w for bp 
#define PM_P5_FCS_OK3_ADDR   	0x874	// r/o for mcu, r/w for bp 
#define PM_P6_FCS_OK3_ADDR   	0x8C4	// r/o for mcu, r/w for bp 
#define PM_P7_FCS_OK3_ADDR   	0x914	// r/o for mcu, r/w for bp 

#define PM_P1_FCS_OK4_ADDR   	0x736	// r/o for mcu, r/w for bp 
#define PM_P2_FCS_OK4_ADDR   	0x786	// r/o for mcu, r/w for bp 
#define PM_P3_FCS_OK4_ADDR   	0x7D6	// r/o for mcu, r/w for bp 
#define PM_P4_FCS_OK4_ADDR   	0x826	// r/o for mcu, r/w for bp 
#define PM_P5_FCS_OK4_ADDR   	0x876	// r/o for mcu, r/w for bp 
#define PM_P6_FCS_OK4_ADDR   	0x8C6	// r/o for mcu, r/w for bp 
#define PM_P7_FCS_OK4_ADDR   	0x916	// r/o for mcu, r/w for bp 

#define PM_P1_FCS_NOK1_ADDR  	0x738	// r/o for mcu, r/w for bp 
#define PM_P2_FCS_NOK1_ADDR  	0x788	// r/o for mcu, r/w for bp 
#define PM_P3_FCS_NOK1_ADDR  	0x7D8	// r/o for mcu, r/w for bp 
#define PM_P4_FCS_NOK1_ADDR  	0x828	// r/o for mcu, r/w for bp 
#define PM_P5_FCS_NOK1_ADDR  	0x878	// r/o for mcu, r/w for bp 
#define PM_P6_FCS_NOK1_ADDR  	0x8C8	// r/o for mcu, r/w for bp 
#define PM_P7_FCS_NOK1_ADDR  	0x918	// r/o for mcu, r/w for bp 

#define PM_P1_FCS_NOK2_ADDR  	0x73A	// r/o for mcu, r/w for bp 
#define PM_P2_FCS_NOK2_ADDR  	0x78A	// r/o for mcu, r/w for bp 
#define PM_P3_FCS_NOK2_ADDR  	0x7DA	// r/o for mcu, r/w for bp 
#define PM_P4_FCS_NOK2_ADDR  	0x82A	// r/o for mcu, r/w for bp 
#define PM_P5_FCS_NOK2_ADDR  	0x87A	// r/o for mcu, r/w for bp 
#define PM_P6_FCS_NOK2_ADDR  	0x8CA	// r/o for mcu, r/w for bp 
#define PM_P7_FCS_NOK2_ADDR  	0x91A	// r/o for mcu, r/w for bp 

#define PM_P1_FCS_NOK3_ADDR  	0x73C	// r/o for mcu, r/w for bp 
#define PM_P2_FCS_NOK3_ADDR  	0x78C	// r/o for mcu, r/w for bp 
#define PM_P3_FCS_NOK3_ADDR  	0x7DC	// r/o for mcu, r/w for bp 
#define PM_P4_FCS_NOK3_ADDR  	0x82C	// r/o for mcu, r/w for bp 
#define PM_P5_FCS_NOK3_ADDR  	0x87C	// r/o for mcu, r/w for bp 
#define PM_P6_FCS_NOK3_ADDR  	0x8CC	// r/o for mcu, r/w for bp 
#define PM_P7_FCS_NOK3_ADDR  	0x91C	// r/o for mcu, r/w for bp 

#define PM_P1_FCS_NOK4_ADDR  	0x73E	// r/o for mcu, r/w for bp 
#define PM_P2_FCS_NOK4_ADDR  	0x78E	// r/o for mcu, r/w for bp 
#define PM_P3_FCS_NOK4_ADDR  	0x7DE	// r/o for mcu, r/w for bp 
#define PM_P4_FCS_NOK4_ADDR  	0x82E	// r/o for mcu, r/w for bp 
#define PM_P5_FCS_NOK4_ADDR  	0x87E	// r/o for mcu, r/w for bp 
#define PM_P6_FCS_NOK4_ADDR  	0x8CE	// r/o for mcu, r/w for bp 
#define PM_P7_FCS_NOK4_ADDR  	0x91E	// r/o for mcu, r/w for bp 


/****************************
 * DCO related registers
 ****************************/

//TBD


/****************************
 * CLEI/USI Information related registers
 ****************************/

#define PORT1_CLEI1_ADDR 			0xB00	// r/o for mcu, r/w for bp 
#define PORT2_CLEI1_ADDR 			0xB10	// r/o for mcu, r/w for bp 
#define PORT3_CLEI1_ADDR 			0xB20	// r/o for mcu, r/w for bp 
#define PORT4_CLEI1_ADDR 			0xB30	// r/o for mcu, r/w for bp 
#define PORT5_CLEI1_ADDR 			0xB40	// r/o for mcu, r/w for bp 
#define PORT6_CLEI1_ADDR 			0xB50	// r/o for mcu, r/w for bp 
#define PORT7_CLEI1_ADDR 			0xB60	// r/o for mcu, r/w for bp 

#define PORT1_CLEI2_ADDR 			0xB02	// r/o for mcu, r/w for bp 
#define PORT2_CLEI2_ADDR 			0xB12	// r/o for mcu, r/w for bp 
#define PORT3_CLEI2_ADDR 			0xB22	// r/o for mcu, r/w for bp 
#define PORT4_CLEI2_ADDR 			0xB32	// r/o for mcu, r/w for bp 
#define PORT5_CLEI2_ADDR 			0xB42	// r/o for mcu, r/w for bp 
#define PORT6_CLEI2_ADDR 			0xB52	// r/o for mcu, r/w for bp 
#define PORT7_CLEI2_ADDR 			0xB62	// r/o for mcu, r/w for bp 

#define PORT1_CLEI3_ADDR 			0xB04	// r/o for mcu, r/w for bp 
#define PORT2_CLEI3_ADDR 			0xB14	// r/o for mcu, r/w for bp 
#define PORT3_CLEI3_ADDR 			0xB24	// r/o for mcu, r/w for bp 
#define PORT4_CLEI3_ADDR 			0xB34	// r/o for mcu, r/w for bp 
#define PORT5_CLEI3_ADDR 			0xB44	// r/o for mcu, r/w for bp 
#define PORT6_CLEI3_ADDR 			0xB54	// r/o for mcu, r/w for bp 
#define PORT7_CLEI3_ADDR 			0xB64	// r/o for mcu, r/w for bp 

#define PORT1_CLEI4_ADDR 			0xB06	// r/o for mcu, r/w for bp 
#define PORT2_CLEI4_ADDR 			0xB16	// r/o for mcu, r/w for bp 
#define PORT3_CLEI4_ADDR 			0xB26	// r/o for mcu, r/w for bp 
#define PORT4_CLEI4_ADDR 			0xB36	// r/o for mcu, r/w for bp 
#define PORT5_CLEI4_ADDR 			0xB46	// r/o for mcu, r/w for bp 
#define PORT6_CLEI4_ADDR 			0xB56	// r/o for mcu, r/w for bp 
#define PORT7_CLEI4_ADDR 			0xB66	// r/o for mcu, r/w for bp 

#define PORT1_CLEI5_ADDR 			0xB08	// r/o for mcu, r/w for bp 
#define PORT2_CLEI5_ADDR 			0xB18	// r/o for mcu, r/w for bp 
#define PORT3_CLEI5_ADDR 			0xB28	// r/o for mcu, r/w for bp 
#define PORT4_CLEI5_ADDR 			0xB38	// r/o for mcu, r/w for bp 
#define PORT5_CLEI5_ADDR 			0xB48	// r/o for mcu, r/w for bp 
#define PORT6_CLEI5_ADDR 			0xB58	// r/o for mcu, r/w for bp 
#define PORT7_CLEI5_ADDR 			0xB68	// r/o for mcu, r/w for bp 

#define PORT1_USI1_ADDR 			0xC00	// r/o for mcu, r/w for bp 
#define PORT2_USI1_ADDR 			0xC20	// r/o for mcu, r/w for bp 
#define PORT3_USI1_ADDR 			0xC40	// r/o for mcu, r/w for bp 
#define PORT4_USI1_ADDR 			0xC60	// r/o for mcu, r/w for bp 
#define PORT5_USI1_ADDR 			0xC80	// r/o for mcu, r/w for bp 
#define PORT6_USI1_ADDR 			0xCA0	// r/o for mcu, r/w for bp 
#define PORT7_USI1_ADDR 			0xCC0	// r/o for mcu, r/w for bp 

#define PORT1_USI2_ADDR 			0xC02	// r/o for mcu, r/w for bp 
#define PORT2_USI2_ADDR 			0xC22	// r/o for mcu, r/w for bp 
#define PORT3_USI2_ADDR 			0xC42	// r/o for mcu, r/w for bp 
#define PORT4_USI2_ADDR 			0xC62	// r/o for mcu, r/w for bp 
#define PORT5_USI2_ADDR 			0xC82	// r/o for mcu, r/w for bp 
#define PORT6_USI2_ADDR 			0xCA2	// r/o for mcu, r/w for bp 
#define PORT7_USI2_ADDR 			0xCC2	// r/o for mcu, r/w for bp 

#define PORT1_USI3_ADDR 			0xC04	// r/o for mcu, r/w for bp 
#define PORT2_USI3_ADDR 			0xC24	// r/o for mcu, r/w for bp 
#define PORT3_USI3_ADDR 			0xC44	// r/o for mcu, r/w for bp 
#define PORT4_USI3_ADDR 			0xC64	// r/o for mcu, r/w for bp 
#define PORT5_USI3_ADDR 			0xC84	// r/o for mcu, r/w for bp 
#define PORT6_USI3_ADDR 			0xCA4	// r/o for mcu, r/w for bp 
#define PORT7_USI3_ADDR 			0xCC4	// r/o for mcu, r/w for bp 

#define PORT1_USI4_ADDR 			0xC06	// r/o for mcu, r/w for bp 
#define PORT2_USI4_ADDR 			0xC26	// r/o for mcu, r/w for bp 
#define PORT3_USI4_ADDR 			0xC46	// r/o for mcu, r/w for bp 
#define PORT4_USI4_ADDR 			0xC66	// r/o for mcu, r/w for bp 
#define PORT5_USI4_ADDR 			0xC86	// r/o for mcu, r/w for bp 
#define PORT6_USI4_ADDR 			0xCA6	// r/o for mcu, r/w for bp 
#define PORT7_USI4_ADDR 			0xCC6	// r/o for mcu, r/w for bp 

#define PORT1_USI5_ADDR 			0xC08	// r/o for mcu, r/w for bp 
#define PORT2_USI5_ADDR 			0xC28	// r/o for mcu, r/w for bp 
#define PORT3_USI5_ADDR 			0xC48	// r/o for mcu, r/w for bp 
#define PORT4_USI5_ADDR 			0xC68	// r/o for mcu, r/w for bp 
#define PORT5_USI5_ADDR 			0xC88	// r/o for mcu, r/w for bp 
#define PORT6_USI5_ADDR 			0xCA8	// r/o for mcu, r/w for bp 
#define PORT7_USI5_ADDR 			0xCC8	// r/o for mcu, r/w for bp 

#define PORT1_USI6_ADDR 			0xC0A	// r/o for mcu, r/w for bp 
#define PORT2_USI6_ADDR 			0xC2A	// r/o for mcu, r/w for bp 
#define PORT3_USI6_ADDR 			0xC4A	// r/o for mcu, r/w for bp 
#define PORT4_USI6_ADDR 			0xC6A	// r/o for mcu, r/w for bp 
#define PORT5_USI6_ADDR 			0xC8A	// r/o for mcu, r/w for bp 
#define PORT6_USI6_ADDR 			0xCAA	// r/o for mcu, r/w for bp 
#define PORT7_USI6_ADDR 			0xCCA	// r/o for mcu, r/w for bp 

#define PORT1_USI7_ADDR 			0xC0C	// r/o for mcu, r/w for bp 
#define PORT2_USI7_ADDR 			0xC2C	// r/o for mcu, r/w for bp 
#define PORT3_USI7_ADDR 			0xC4C	// r/o for mcu, r/w for bp 
#define PORT4_USI7_ADDR 			0xC6C	// r/o for mcu, r/w for bp 
#define PORT5_USI7_ADDR 			0xC8C	// r/o for mcu, r/w for bp 
#define PORT6_USI7_ADDR 			0xCAC	// r/o for mcu, r/w for bp 
#define PORT7_USI7_ADDR 			0xCCC	// r/o for mcu, r/w for bp 

#define PORT1_USI8_ADDR 			0xC0E	// r/o for mcu, r/w for bp 
#define PORT2_USI8_ADDR 			0xC2E	// r/o for mcu, r/w for bp 
#define PORT3_USI8_ADDR 			0xC4E	// r/o for mcu, r/w for bp 
#define PORT4_USI8_ADDR 			0xC6E	// r/o for mcu, r/w for bp 
#define PORT5_USI8_ADDR 			0xC8E	// r/o for mcu, r/w for bp 
#define PORT6_USI8_ADDR 			0xCAE	// r/o for mcu, r/w for bp 
#define PORT7_USI8_ADDR 			0xCCE	// r/o for mcu, r/w for bp 

#define PORT1_USI9_ADDR 			0xC10	// r/o for mcu, r/w for bp 
#define PORT2_USI9_ADDR 			0xC30	// r/o for mcu, r/w for bp 
#define PORT3_USI9_ADDR 			0xC50	// r/o for mcu, r/w for bp 
#define PORT4_USI9_ADDR 			0xC70	// r/o for mcu, r/w for bp 
#define PORT5_USI9_ADDR 			0xC90	// r/o for mcu, r/w for bp 
#define PORT6_USI9_ADDR 			0xCB0	// r/o for mcu, r/w for bp 
#define PORT7_USI9_ADDR 			0xCD0	// r/o for mcu, r/w for bp 

#define PORT1_USI10_ADDR 			0xC12	// r/o for mcu, r/w for bp 
#define PORT2_USI10_ADDR 			0xC32	// r/o for mcu, r/w for bp 
#define PORT3_USI10_ADDR 			0xC52	// r/o for mcu, r/w for bp 
#define PORT4_USI10_ADDR 			0xC72	// r/o for mcu, r/w for bp 
#define PORT5_USI10_ADDR 			0xC92	// r/o for mcu, r/w for bp 
#define PORT6_USI10_ADDR 			0xCB2	// r/o for mcu, r/w for bp 
#define PORT7_USI10_ADDR 			0xCD2	// r/o for mcu, r/w for bp 

#define PORT1_USI11_ADDR 			0xC14	// r/o for mcu, r/w for bp 
#define PORT2_USI11_ADDR 			0xC34	// r/o for mcu, r/w for bp 
#define PORT3_USI11_ADDR 			0xC54	// r/o for mcu, r/w for bp 
#define PORT4_USI11_ADDR 			0xC74	// r/o for mcu, r/w for bp 
#define PORT5_USI11_ADDR 			0xC94	// r/o for mcu, r/w for bp 
#define PORT6_USI11_ADDR 			0xCB4	// r/o for mcu, r/w for bp 
#define PORT7_USI11_ADDR 			0xCD4	// r/o for mcu, r/w for bp 

#define PORT1_USI12_ADDR 			0xC16	// r/o for mcu, r/w for bp 
#define PORT2_USI12_ADDR 			0xC36	// r/o for mcu, r/w for bp 
#define PORT3_USI12_ADDR 			0xC56	// r/o for mcu, r/w for bp 
#define PORT4_USI12_ADDR 			0xC76	// r/o for mcu, r/w for bp 
#define PORT5_USI12_ADDR 			0xC96	// r/o for mcu, r/w for bp 
#define PORT6_USI12_ADDR 			0xCB6	// r/o for mcu, r/w for bp 
#define PORT7_USI12_ADDR 			0xCD6	// r/o for mcu, r/w for bp 

#define PORT1_USI13_ADDR 			0xC18	// r/o for mcu, r/w for bp 
#define PORT2_USI13_ADDR 			0xC38	// r/o for mcu, r/w for bp 
#define PORT3_USI13_ADDR 			0xC58	// r/o for mcu, r/w for bp 
#define PORT4_USI13_ADDR 			0xC78	// r/o for mcu, r/w for bp 
#define PORT5_USI13_ADDR 			0xC98	// r/o for mcu, r/w for bp 
#define PORT6_USI13_ADDR 			0xCB8	// r/o for mcu, r/w for bp 
#define PORT7_USI13_ADDR 			0xCD8	// r/o for mcu, r/w for bp 


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


#define PWM_NOT_USED        (-99.9f)
#define DEF_PWM_MIN         (-53.0f)
#define DEF_PWM_MAX         (20.0f)


extern unsigned long __COMMON_CTRL2_ADDR[PORT_ID_EAG6L_MAX];
extern unsigned long __PORT_CONFIG_ADDR[PORT_ID_EAG6L_MAX];
extern unsigned long __PORT_TX_PWR_ADDR[PORT_ID_EAG6L_MAX];
extern unsigned long __PORT_RX_PWR_ADDR[PORT_ID_EAG6L_MAX];
extern unsigned long __PORT_WL1_ADDR[PORT_ID_EAG6L_MAX];
extern unsigned long __PORT_WL2_ADDR[PORT_ID_EAG6L_MAX];
extern unsigned long __PORT_DIST_ADDR[PORT_ID_EAG6L_MAX];
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


#endif//_BP_REGS_H_
