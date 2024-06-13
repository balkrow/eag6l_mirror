#ifndef _BP_REGS_H_
#define _BP_REGS_H_


#if 1/*[#48] register monitoring and update 관련 기능 추가, balkrow, 2024-06-10*/ 
typedef uint16_t (*READ_FUNC) (uint16_t);
typedef uint16_t (*REG_CALLBACK) (uint16_t);

typedef struct reg_mon_str
{
	uint16_t reg;
	uint16_t mask;
	uint16_t shift;
	uint16_t val;
	READ_FUNC func;
	REG_CALLBACK cb;
} RegMON;
#endif

#if 1/*[#53] Clock source status 업데이트 기능 추가, balkrow, 2024-06-13*/
#define FPGA_BASE 0x60000000
#define CPLD_BASE 0x70000000
#endif

/****************************
 * unit related registers
 ****************************/

#define SW_VERSION_ADDR			0x60000000	// r/o for mcu, r/w for bp
#define HW_KEEP_ALIVE_1_ADDR		0x60000014	// r/w for mcu, r/o for bp
#define HW_KEEP_ALIVE_2_ADDR		0x60000016	// r/o for mcu, r/w for bp :
                                            //   bp must increase this value.
#define INIT_COMPLETE_ADDR			0x60000018	// bp set 0xAA to [15:8] if init done.
#if 1/*[#53] Clock source status 업데이트 기능 추가, balkrow, 2024-06-13*/
#define INIT_COMPLETE_ADDR_MASK		0xFFF00
#endif

// bp must set 10G/25G speed for port 1~6.
#define COMMON_CTRL2_P1_ADDR		0x60000020	// r/w for mcu, r/o for bp 
#define COMMON_CTRL2_P2_ADDR		0x60000022	// r/w for mcu, r/o for bp 
#define COMMON_CTRL2_P3_ADDR		0x60000024	// r/w for mcu, r/o for bp 
#define COMMON_CTRL2_P4_ADDR		0x60000026	// r/w for mcu, r/o for bp 
#define COMMON_CTRL2_P5_ADDR		0x60000028	// r/w for mcu, r/o for bp 
#define COMMON_CTRL2_P6_ADDR		0x6000002A	// r/w for mcu, r/o for bp 
#define COMMON_CTRL2_P7_ADDR		0x6000002C	// r/w for mcu, r/o for bp 


/****************************
 * Reset/CR related registers
 ****************************/

#define CHIP_RESET_ADDR				0x60000020	// r/w for both mcu/bp.
                                            // reset fpga if FPGA_rst[15:8] is 0x5A.
                                            // reset bp if BP_rst[7:0] is 0x5A.
                                            // clear field if action is done.


/****************************
 * DCO related registers
 ****************************/

#define DCO_INTERFACE_ADDR			0x60000060	// r/w for mcu, r/o for bp 
#define DCO_WRITE_CMD_ADDR			0x60000062	// r/w for mcu, r/o for bp 
#define DCO_READ_CMD_ADDR			0x60000064	// r/w for mcu, r/o for bp 
#define DCO_ACCESS_METHOD_ADDR		0x60000066	// r/w for mcu, r/o for bp 


/****************************
 * DCO related registers
 ****************************/

// H/W manufacture
#define INV_HW_MANU_1_ADDR			0x60000370	// r/o for mcu, r/w for bp 
#define INV_HW_MANU_2_ADDR			0x60000372	// r/o for mcu, r/w for bp 
#define INV_HW_MANU_3_ADDR			0x60000374	// r/o for mcu, r/w for bp 
#define INV_HW_MANU_4_ADDR			0x60000376	// r/o for mcu, r/w for bp 
#define INV_HW_MANU_5_ADDR			0x60000378	// r/o for mcu, r/w for bp 

// H/W model
#define INV_HW_MODEL_1_ADDR			0x60000380	// r/o for mcu, r/w for bp 
#define INV_HW_MODEL_2_ADDR			0x60000382	// r/o for mcu, r/w for bp 
#define INV_HW_MODEL_3_ADDR			0x60000384	// r/o for mcu, r/w for bp 
#define INV_HW_MODEL_4_ADDR			0x60000386	// r/o for mcu, r/w for bp 
#define INV_HW_MODEL_5_ADDR			0x60000388	// r/o for mcu, r/w for bp 

// H/W part number
#define INV_HW_PN_1_ADDR			0x60000390	// r/o for mcu, r/w for bp 
#define INV_HW_PN_2_ADDR			0x60000392	// r/o for mcu, r/w for bp 
#define INV_HW_PN_3_ADDR			0x60000394	// r/o for mcu, r/w for bp 
#define INV_HW_PN_4_ADDR			0x60000396	// r/o for mcu, r/w for bp 
#define INV_HW_PN_5_ADDR			0x60000398	// r/o for mcu, r/w for bp 
#define INV_HW_PN_6_ADDR			0x6000039A	// r/o for mcu, r/w for bp 
#define INV_HW_PN_7_ADDR			0x6000039C	// r/o for mcu, r/w for bp 
#define INV_HW_PN_8_ADDR			0x6000039E	// r/o for mcu, r/w for bp 

// H/W serial number
#define INV_HW_SN_1_ADDR			0x600003A0	// r/o for mcu, r/w for bp 
#define INV_HW_SN_2_ADDR			0x600003A2	// r/o for mcu, r/w for bp 

// H/W revision number
#define INV_HW_REV_1_ADDR			0x600003B0	// r/o for mcu, r/w for bp 
#define INV_HW_REV_2_ADDR			0x600003B2	// r/o for mcu, r/w for bp 

// H/W manufacture date
#define INV_HW_MDATE_1_ADDR			0x600003B4	// r/o for mcu, r/w for bp 
#define INV_HW_MDATE_2_ADDR			0x600003B6	// r/o for mcu, r/w for bp 
#define INV_HW_MDATE_3_ADDR			0x600003B8	// r/o for mcu, r/w for bp 
#define INV_HW_MDATE_4_ADDR			0x600003BA	// r/o for mcu, r/w for bp 
#define INV_HW_MDATE_5_ADDR			0x600003BC	// r/o for mcu, r/w for bp 
#define INV_HW_MDATE_6_ADDR			0x600003BE	// r/o for mcu, r/w for bp 

// H/W repair date
#define INV_HW_RDATE_1_ADDR			0x600003C0	// r/o for mcu, r/w for bp 
#define INV_HW_RDATE_2_ADDR			0x600003C2	// r/o for mcu, r/w for bp 
#define INV_HW_RDATE_3_ADDR			0x600003C4	// r/o for mcu, r/w for bp 
#define INV_HW_RDATE_4_ADDR			0x600003C6	// r/o for mcu, r/w for bp 
#define INV_HW_RDATE_5_ADDR			0x600003C8	// r/o for mcu, r/w for bp 

// H/W repair code
#define INV_HW_RCODE_1_ADDR			0x600003CA	// r/o for mcu, r/w for bp 
#define INV_HW_RCODE_2_ADDR			0x600003CC	// r/o for mcu, r/w for bp 


/****************************
 * Port Config related registers
 ****************************/

#define PORT_1_CONF_ADDR			0x60800000	// r/w for mcu, r/o for bp 
#define PORT_2_CONF_ADDR			0x60800002	// r/w for mcu, r/o for bp 
#define PORT_3_CONF_ADDR			0x60800004	// r/w for mcu, r/o for bp 
#define PORT_4_CONF_ADDR			0x60800006	// r/w for mcu, r/o for bp 
#define PORT_5_CONF_ADDR			0x60800008	// r/w for mcu, r/o for bp 
#define PORT_6_CONF_ADDR			0x6080000A	// r/w for mcu, r/o for bp 
#define PORT_7_CONF_ADDR			0x6080000C	// r/w for mcu, r/o for bp 

#define PORT_7_CONF2_ADDR			0x6080000E	// r/w for mcu, r/o for bp 
#define PORT_7_CONF3_ADDR			0x6080001E	// r/w for mcu, r/o for bp 

/****************************
 * SyncE related registers
 ****************************/

#define SYNCE_GCONFIG_ADDR			0x60800160	// r/w for mcu, r/o for bp 
#define SYNCE_IF_SELECT_ADDR			0x60800162	// r/w for mcu, r/o for bp 
#define SYNCE_ESMC_LQL_ADDR			0x60800166	// r/o for mcu, r/w for bp 
#define SYNCE_ESMC_SQL_ADDR			0x60800168	// r/o for mcu, r/w for bp 
#define SYNCE_ESMC_RQL_ADDR			0x6080016A	// r/o for mcu, r/w for bp 
#define SYNCE_SRC_STAT_ADDR			0x6080016C	// r/o for mcu, r/w for bp 
#if 1/*[#53] Clock source status 업데이트 기능 추가, balkrow, 2024-06-13*/
#define SYNCE_SRC_STAT_ADDR_MASK		0xFFF00		// r/o for mcu, r/w for bp 
#endif

#define SYNCE_ESMC_RQL2_ADDR			0x6080016E	// r/o for mcu, r/w for bp 
#define SYNCE_ESMC_RQL3_ADDR			0x60800170	// r/o for mcu, r/w for bp 
#define SYNCE_ESMC_RQL4_ADDR			0x60800172	// r/o for mcu, r/w for bp 
#define SYNCE_ESMC_RQL5_ADDR			0x60800174	// r/o for mcu, r/w for bp 


/****************************
 * SFP Port Status/Control related registers
 ****************************/

#define BD_SFP_CR_ADDR				0x60800000	// r/o for both mcu/bp. FPGA must provide this.

#define PORT_1_TX_PWR_ADDR			0x60800020	// r/o for mcu, r/w for bp 
#define PORT_2_TX_PWR_ADDR			0x60800040	// r/o for mcu, r/w for bp 
#define PORT_3_TX_PWR_ADDR			0x60800060	// r/o for mcu, r/w for bp 
#define PORT_4_TX_PWR_ADDR			0x60800080	// r/o for mcu, r/w for bp 
#define PORT_5_TX_PWR_ADDR			0x608000A0	// r/o for mcu, r/w for bp 
#define PORT_6_TX_PWR_ADDR			0x608000C0	// r/o for mcu, r/w for bp 
#define PORT_7_TX_PWR_ADDR			0x608000E0	// r/o for mcu, r/w for bp 

#define PORT_1_RX_PWR_ADDR			0x60800022	// r/o for mcu, r/w for bp 
#define PORT_2_RX_PWR_ADDR			0x60800042	// r/o for mcu, r/w for bp 
#define PORT_3_RX_PWR_ADDR			0x60800062	// r/o for mcu, r/w for bp 
#define PORT_4_RX_PWR_ADDR			0x60800082	// r/o for mcu, r/w for bp 
#define PORT_5_RX_PWR_ADDR			0x608000A2	// r/o for mcu, r/w for bp 
#define PORT_6_RX_PWR_ADDR			0x608000C2	// r/o for mcu, r/w for bp 
#define PORT_7_RX_PWR_ADDR			0x608000E2	// r/o for mcu, r/w for bp 

#define PORT_1_WL1_ADDR				0x60800024	// r/o for mcu, r/w for bp 
#define PORT_2_WL1_ADDR				0x60800044	// r/o for mcu, r/w for bp 
#define PORT_3_WL1_ADDR				0x60800064	// r/o for mcu, r/w for bp 
#define PORT_4_WL1_ADDR				0x60800084	// r/o for mcu, r/w for bp 
#define PORT_5_WL1_ADDR				0x608000A4	// r/o for mcu, r/w for bp 
#define PORT_6_WL1_ADDR				0x608000C4	// r/o for mcu, r/w for bp 
#define PORT_7_WL1_ADDR				0x608000E4	// r/o for mcu, r/w for bp 

#define PORT_1_WL2_ADDR				0x60800026	// r/o for mcu, r/w for bp 
#define PORT_2_WL2_ADDR				0x60800046	// r/o for mcu, r/w for bp 
#define PORT_3_WL2_ADDR				0x60800066	// r/o for mcu, r/w for bp 
#define PORT_4_WL2_ADDR				0x60800086	// r/o for mcu, r/w for bp 
#define PORT_5_WL2_ADDR				0x608000A6	// r/o for mcu, r/w for bp 
#define PORT_6_WL2_ADDR				0x608000C6	// r/o for mcu, r/w for bp 
#define PORT_7_WL2_ADDR				0x608000E6	// r/o for mcu, r/w for bp 

#define PORT_1_DIST_ADDR			0x6080002C	// r/o for mcu, r/w for bp 
#define PORT_2_DIST_ADDR			0x6080004C	// r/o for mcu, r/w for bp 
#define PORT_3_DIST_ADDR			0x6080006C	// r/o for mcu, r/w for bp 
#define PORT_4_DIST_ADDR			0x6080008C	// r/o for mcu, r/w for bp 
#define PORT_5_DIST_ADDR			0x608000AC	// r/o for mcu, r/w for bp 
#define PORT_6_DIST_ADDR			0x608000CC	// r/o for mcu, r/w for bp 
#define PORT_7_DIST_ADDR			0x608000EC	// r/o for mcu, r/w for bp 

#define PORT_1_STSFP_STAT_ADDR		0x6080002E	// r/o for mcu, r/w for bp 
#define PORT_2_STSFP_STAT_ADDR		0x6080004E	// r/o for mcu, r/w for bp 
#define PORT_3_STSFP_STAT_ADDR		0x6080006E	// r/o for mcu, r/w for bp 
#define PORT_4_STSFP_STAT_ADDR		0x6080008E	// r/o for mcu, r/w for bp 
#define PORT_5_STSFP_STAT_ADDR		0x608000AE	// r/o for mcu, r/w for bp 
#define PORT_6_STSFP_STAT_ADDR		0x608000CE	// r/o for mcu, r/w for bp 
#define PORT_7_STSFP_STAT_ADDR		0x608000EE	// r/o for mcu, r/w for bp 

#define PORT_1_ALM_ADDR				0x60800030	// r/o for mcu, r/w for bp 
#define PORT_2_ALM_ADDR				0x60800050	// r/o for mcu, r/w for bp 
#define PORT_3_ALM_ADDR				0x60800070	// r/o for mcu, r/w for bp 
#define PORT_4_ALM_ADDR				0x60800090	// r/o for mcu, r/w for bp 
#define PORT_5_ALM_ADDR				0x608000B0	// r/o for mcu, r/w for bp 
#define PORT_6_ALM_ADDR				0x608000D0	// r/o for mcu, r/w for bp 
#define PORT_7_ALM_ADDR				0x608000F0	// r/o for mcu, r/w for bp 

#define PORT_1_ALM_FLAG_ADDR		0x60800032	// r/o for mcu, r/w for bp 
#define PORT_2_ALM_FLAG_ADDR		0x60800052	// r/o for mcu, r/w for bp 
#define PORT_3_ALM_FLAG_ADDR		0x60800072	// r/o for mcu, r/w for bp 
#define PORT_4_ALM_FLAG_ADDR		0x60800092	// r/o for mcu, r/w for bp 
#define PORT_5_ALM_FLAG_ADDR		0x608000B2	// r/o for mcu, r/w for bp 
#define PORT_6_ALM_FLAG_ADDR		0x608000D2	// r/o for mcu, r/w for bp 
#define PORT_7_ALM_FLAG_ADDR		0x608000F2	// r/o for mcu, r/w for bp 

#define PORT_1_ALM_MASK_ADDR		0x60800034	// r/o for mcu, r/w for bp 
#define PORT_2_ALM_MASK_ADDR		0x60800054	// r/o for mcu, r/w for bp 
#define PORT_3_ALM_MASK_ADDR		0x60800074	// r/o for mcu, r/w for bp 
#define PORT_4_ALM_MASK_ADDR		0x60800094	// r/o for mcu, r/w for bp 
#define PORT_5_ALM_MASK_ADDR		0x608000B4	// r/o for mcu, r/w for bp 
#define PORT_6_ALM_MASK_ADDR		0x608000D4	// r/o for mcu, r/w for bp 
#define PORT_7_ALM_MASK_ADDR		0x608000F4	// r/o for mcu, r/w for bp 

#define PORT_1_GET_CH_NUM_ADDR		0x60800036	// r/o for mcu, r/w for bp 
#define PORT_2_GET_CH_NUM_ADDR		0x60800056	// r/o for mcu, r/w for bp 
#define PORT_3_GET_CH_NUM_ADDR		0x60800076	// r/o for mcu, r/w for bp 
#define PORT_4_GET_CH_NUM_ADDR		0x60800096	// r/o for mcu, r/w for bp 
#define PORT_5_GET_CH_NUM_ADDR		0x608000B6	// r/o for mcu, r/w for bp 
#define PORT_6_GET_CH_NUM_ADDR		0x608000D6	// r/o for mcu, r/w for bp 
#define PORT_7_GET_CH_NUM_ADDR		0x608000F6	// r/o for mcu, r/w for bp 

#define PORT_1_SET_CH_NUM_ADDR		0x60800038	// r/w for mcu, r/o for bp 
#define PORT_2_SET_CH_NUM_ADDR		0x60800058	// r/w for mcu, r/o for bp 
#define PORT_3_SET_CH_NUM_ADDR		0x60800078	// r/w for mcu, r/o for bp 
#define PORT_4_SET_CH_NUM_ADDR		0x60800098	// r/w for mcu, r/o for bp 
#define PORT_5_SET_CH_NUM_ADDR		0x608000B8	// r/w for mcu, r/o for bp 
#define PORT_6_SET_CH_NUM_ADDR		0x608000D8	// r/w for mcu, r/o for bp 
#define PORT_7_SET_CH_NUM_ADDR		0x608000F8	// r/w for mcu, r/o for bp 


/****************************
 * Q/SFP Information related registers
 ****************************/

#define SFP_P1_VENDOR1_ADDR			0x60800400	// r/o for mcu, r/w for bp 
#define SFP_P2_VENDOR1_ADDR			0x60800440	// r/o for mcu, r/w for bp 
#define SFP_P3_VENDOR1_ADDR			0x60800480	// r/o for mcu, r/w for bp 
#define SFP_P4_VENDOR1_ADDR			0x608004C0	// r/o for mcu, r/w for bp 
#define SFP_P5_VENDOR1_ADDR			0x60800500	// r/o for mcu, r/w for bp 
#define SFP_P6_VENDOR1_ADDR			0x60800540	// r/o for mcu, r/w for bp 
#define SFP_P7_VENDOR1_ADDR			0x60800580	// r/o for mcu, r/w for bp 

#define SFP_P1_VENDOR2_ADDR			0x60800402	// r/o for mcu, r/w for bp 
#define SFP_P2_VENDOR2_ADDR			0x60800442	// r/o for mcu, r/w for bp 
#define SFP_P3_VENDOR2_ADDR			0x60800482	// r/o for mcu, r/w for bp 
#define SFP_P4_VENDOR2_ADDR			0x608004C2	// r/o for mcu, r/w for bp 
#define SFP_P5_VENDOR2_ADDR			0x60800502	// r/o for mcu, r/w for bp 
#define SFP_P6_VENDOR2_ADDR			0x60800542	// r/o for mcu, r/w for bp 
#define SFP_P7_VENDOR2_ADDR			0x60800582	// r/o for mcu, r/w for bp 

#define SFP_P1_VENDOR3_ADDR			0x60800404	// r/o for mcu, r/w for bp 
#define SFP_P2_VENDOR3_ADDR			0x60800444	// r/o for mcu, r/w for bp 
#define SFP_P3_VENDOR3_ADDR			0x60800484	// r/o for mcu, r/w for bp 
#define SFP_P4_VENDOR3_ADDR			0x608004C4	// r/o for mcu, r/w for bp 
#define SFP_P5_VENDOR3_ADDR			0x60800504	// r/o for mcu, r/w for bp 
#define SFP_P6_VENDOR3_ADDR			0x60800544	// r/o for mcu, r/w for bp 
#define SFP_P7_VENDOR3_ADDR			0x60800584	// r/o for mcu, r/w for bp 

#define SFP_P1_VENDOR4_ADDR			0x60800406	// r/o for mcu, r/w for bp 
#define SFP_P2_VENDOR4_ADDR			0x60800446	// r/o for mcu, r/w for bp 
#define SFP_P3_VENDOR4_ADDR			0x60800486	// r/o for mcu, r/w for bp 
#define SFP_P4_VENDOR4_ADDR			0x608004C6	// r/o for mcu, r/w for bp 
#define SFP_P5_VENDOR4_ADDR			0x60800506	// r/o for mcu, r/w for bp 
#define SFP_P6_VENDOR4_ADDR			0x60800546	// r/o for mcu, r/w for bp 
#define SFP_P7_VENDOR4_ADDR			0x60800586	// r/o for mcu, r/w for bp 

#define SFP_P1_VENDOR5_ADDR			0x60800408	// r/o for mcu, r/w for bp 
#define SFP_P2_VENDOR5_ADDR			0x60800448	// r/o for mcu, r/w for bp 
#define SFP_P3_VENDOR5_ADDR			0x60800488	// r/o for mcu, r/w for bp 
#define SFP_P4_VENDOR5_ADDR			0x608004C8	// r/o for mcu, r/w for bp 
#define SFP_P5_VENDOR5_ADDR			0x60800508	// r/o for mcu, r/w for bp 
#define SFP_P6_VENDOR5_ADDR			0x60800548	// r/o for mcu, r/w for bp 
#define SFP_P7_VENDOR5_ADDR			0x60800588	// r/o for mcu, r/w for bp 

#define SFP_P1_VENDOR6_ADDR			0x6080040A	// r/o for mcu, r/w for bp 
#define SFP_P2_VENDOR6_ADDR			0x6080044A	// r/o for mcu, r/w for bp 
#define SFP_P3_VENDOR6_ADDR			0x6080048A	// r/o for mcu, r/w for bp 
#define SFP_P4_VENDOR6_ADDR			0x608004CA	// r/o for mcu, r/w for bp 
#define SFP_P5_VENDOR6_ADDR			0x6080050A	// r/o for mcu, r/w for bp 
#define SFP_P6_VENDOR6_ADDR			0x6080054A	// r/o for mcu, r/w for bp 
#define SFP_P7_VENDOR6_ADDR			0x6080058A	// r/o for mcu, r/w for bp 

#define SFP_P1_VENDOR7_ADDR			0x6080040C	// r/o for mcu, r/w for bp 
#define SFP_P2_VENDOR7_ADDR			0x6080044C	// r/o for mcu, r/w for bp 
#define SFP_P3_VENDOR7_ADDR			0x6080048C	// r/o for mcu, r/w for bp 
#define SFP_P4_VENDOR7_ADDR			0x608004CC	// r/o for mcu, r/w for bp 
#define SFP_P5_VENDOR7_ADDR			0x6080050C	// r/o for mcu, r/w for bp 
#define SFP_P6_VENDOR7_ADDR			0x6080054C	// r/o for mcu, r/w for bp 
#define SFP_P7_VENDOR7_ADDR			0x6080058C	// r/o for mcu, r/w for bp 

#define SFP_P1_VENDOR8_ADDR			0x6080040E	// r/o for mcu, r/w for bp 
#define SFP_P2_VENDOR8_ADDR			0x6080044E	// r/o for mcu, r/w for bp 
#define SFP_P3_VENDOR8_ADDR			0x6080048E	// r/o for mcu, r/w for bp 
#define SFP_P4_VENDOR8_ADDR			0x608004CE	// r/o for mcu, r/w for bp 
#define SFP_P5_VENDOR8_ADDR			0x6080050E	// r/o for mcu, r/w for bp 
#define SFP_P6_VENDOR8_ADDR			0x6080054E	// r/o for mcu, r/w for bp 
#define SFP_P7_VENDOR8_ADDR			0x6080058E	// r/o for mcu, r/w for bp 

#define SFP_P1_PN1_ADDR				0x60800410	// r/o for mcu, r/w for bp 
#define SFP_P2_PN1_ADDR				0x60800450	// r/o for mcu, r/w for bp 
#define SFP_P3_PN1_ADDR				0x60800490	// r/o for mcu, r/w for bp 
#define SFP_P4_PN1_ADDR				0x608004D0	// r/o for mcu, r/w for bp 
#define SFP_P5_PN1_ADDR				0x60800510	// r/o for mcu, r/w for bp 
#define SFP_P6_PN1_ADDR				0x60800550	// r/o for mcu, r/w for bp 
#define SFP_P7_PN1_ADDR				0x60800590	// r/o for mcu, r/w for bp 

#define SFP_P1_PN2_ADDR				0x60800412	// r/o for mcu, r/w for bp 
#define SFP_P2_PN2_ADDR				0x60800452	// r/o for mcu, r/w for bp 
#define SFP_P3_PN2_ADDR				0x60800492	// r/o for mcu, r/w for bp 
#define SFP_P4_PN2_ADDR				0x608004D2	// r/o for mcu, r/w for bp 
#define SFP_P5_PN2_ADDR				0x60800512	// r/o for mcu, r/w for bp 
#define SFP_P6_PN2_ADDR				0x60800552	// r/o for mcu, r/w for bp 
#define SFP_P7_PN2_ADDR				0x60800592	// r/o for mcu, r/w for bp 

#define SFP_P1_PN3_ADDR				0x60800414	// r/o for mcu, r/w for bp 
#define SFP_P2_PN3_ADDR				0x60800454	// r/o for mcu, r/w for bp 
#define SFP_P3_PN3_ADDR				0x60800494	// r/o for mcu, r/w for bp 
#define SFP_P4_PN3_ADDR				0x608004D4	// r/o for mcu, r/w for bp 
#define SFP_P5_PN3_ADDR				0x60800514	// r/o for mcu, r/w for bp 
#define SFP_P6_PN3_ADDR				0x60800554	// r/o for mcu, r/w for bp 
#define SFP_P7_PN3_ADDR				0x60800594	// r/o for mcu, r/w for bp 

#define SFP_P1_PN4_ADDR				0x60800416	// r/o for mcu, r/w for bp 
#define SFP_P2_PN4_ADDR				0x60800456	// r/o for mcu, r/w for bp 
#define SFP_P3_PN4_ADDR				0x60800496	// r/o for mcu, r/w for bp 
#define SFP_P4_PN4_ADDR				0x608004D6	// r/o for mcu, r/w for bp 
#define SFP_P5_PN4_ADDR				0x60800516	// r/o for mcu, r/w for bp 
#define SFP_P6_PN4_ADDR				0x60800556	// r/o for mcu, r/w for bp 
#define SFP_P7_PN4_ADDR				0x60800596	// r/o for mcu, r/w for bp 

#define SFP_P1_PN5_ADDR				0x60800418	// r/o for mcu, r/w for bp 
#define SFP_P2_PN5_ADDR				0x60800458	// r/o for mcu, r/w for bp 
#define SFP_P3_PN5_ADDR				0x60800498	// r/o for mcu, r/w for bp 
#define SFP_P4_PN5_ADDR				0x608004D8	// r/o for mcu, r/w for bp 
#define SFP_P5_PN5_ADDR				0x60800518	// r/o for mcu, r/w for bp 
#define SFP_P6_PN5_ADDR				0x60800558	// r/o for mcu, r/w for bp 
#define SFP_P7_PN5_ADDR				0x60800598	// r/o for mcu, r/w for bp 

#define SFP_P1_PN6_ADDR				0x6080041A	// r/o for mcu, r/w for bp 
#define SFP_P2_PN6_ADDR				0x6080045A	// r/o for mcu, r/w for bp 
#define SFP_P3_PN6_ADDR				0x6080049A	// r/o for mcu, r/w for bp 
#define SFP_P4_PN6_ADDR				0x608004DA	// r/o for mcu, r/w for bp 
#define SFP_P5_PN6_ADDR				0x6080051A	// r/o for mcu, r/w for bp 
#define SFP_P6_PN6_ADDR				0x6080055A	// r/o for mcu, r/w for bp 
#define SFP_P7_PN6_ADDR				0x6080059A	// r/o for mcu, r/w for bp 

#define SFP_P1_PN7_ADDR				0x6080041C	// r/o for mcu, r/w for bp 
#define SFP_P2_PN7_ADDR				0x6080045C	// r/o for mcu, r/w for bp 
#define SFP_P3_PN7_ADDR				0x6080049C	// r/o for mcu, r/w for bp 
#define SFP_P4_PN7_ADDR				0x608004DC	// r/o for mcu, r/w for bp 
#define SFP_P5_PN7_ADDR				0x6080051C	// r/o for mcu, r/w for bp 
#define SFP_P6_PN7_ADDR				0x6080055C	// r/o for mcu, r/w for bp 
#define SFP_P7_PN7_ADDR				0x6080059C	// r/o for mcu, r/w for bp 

#define SFP_P1_PN8_ADDR				0x6080041E	// r/o for mcu, r/w for bp 
#define SFP_P2_PN8_ADDR				0x6080045E	// r/o for mcu, r/w for bp 
#define SFP_P3_PN8_ADDR				0x6080049E	// r/o for mcu, r/w for bp 
#define SFP_P4_PN8_ADDR				0x608004DE	// r/o for mcu, r/w for bp 
#define SFP_P5_PN8_ADDR				0x6080051E	// r/o for mcu, r/w for bp 
#define SFP_P6_PN8_ADDR				0x6080055E	// r/o for mcu, r/w for bp 
#define SFP_P7_PN8_ADDR				0x6080059E	// r/o for mcu, r/w for bp 

#define SFP_P1_SN1_ADDR				0x60800420	// r/o for mcu, r/w for bp 
#define SFP_P2_SN1_ADDR				0x60800460	// r/o for mcu, r/w for bp 
#define SFP_P3_SN1_ADDR				0x608004A0	// r/o for mcu, r/w for bp 
#define SFP_P4_SN1_ADDR				0x608004E0	// r/o for mcu, r/w for bp 
#define SFP_P5_SN1_ADDR				0x60800520	// r/o for mcu, r/w for bp 
#define SFP_P6_SN1_ADDR				0x60800560	// r/o for mcu, r/w for bp 
#define SFP_P7_SN1_ADDR				0x608005A0	// r/o for mcu, r/w for bp 

#define SFP_P1_SN2_ADDR				0x60800422	// r/o for mcu, r/w for bp 
#define SFP_P2_SN2_ADDR				0x60800462	// r/o for mcu, r/w for bp 
#define SFP_P3_SN2_ADDR				0x608004A2	// r/o for mcu, r/w for bp 
#define SFP_P4_SN2_ADDR				0x608004E2	// r/o for mcu, r/w for bp 
#define SFP_P5_SN2_ADDR				0x60800522	// r/o for mcu, r/w for bp 
#define SFP_P6_SN2_ADDR				0x60800562	// r/o for mcu, r/w for bp 
#define SFP_P7_SN2_ADDR				0x608005A2	// r/o for mcu, r/w for bp 

#define SFP_P1_SN3_ADDR				0x60800424	// r/o for mcu, r/w for bp 
#define SFP_P2_SN3_ADDR				0x60800464	// r/o for mcu, r/w for bp 
#define SFP_P3_SN3_ADDR				0x608004A4	// r/o for mcu, r/w for bp 
#define SFP_P4_SN3_ADDR				0x608004E4	// r/o for mcu, r/w for bp 
#define SFP_P5_SN3_ADDR				0x60800524	// r/o for mcu, r/w for bp 
#define SFP_P6_SN3_ADDR				0x60800564	// r/o for mcu, r/w for bp 
#define SFP_P7_SN3_ADDR				0x608005A4	// r/o for mcu, r/w for bp 

#define SFP_P1_SN4_ADDR				0x60800426	// r/o for mcu, r/w for bp 
#define SFP_P2_SN4_ADDR				0x60800466	// r/o for mcu, r/w for bp 
#define SFP_P3_SN4_ADDR				0x608004A6	// r/o for mcu, r/w for bp 
#define SFP_P4_SN4_ADDR				0x608004E6	// r/o for mcu, r/w for bp 
#define SFP_P5_SN4_ADDR				0x60800526	// r/o for mcu, r/w for bp 
#define SFP_P6_SN4_ADDR				0x60800566	// r/o for mcu, r/w for bp 
#define SFP_P7_SN4_ADDR				0x608005A6	// r/o for mcu, r/w for bp 

#define SFP_P1_SN5_ADDR				0x60800428	// r/o for mcu, r/w for bp 
#define SFP_P2_SN5_ADDR				0x60800468	// r/o for mcu, r/w for bp 
#define SFP_P3_SN5_ADDR				0x608004A8	// r/o for mcu, r/w for bp 
#define SFP_P4_SN5_ADDR				0x608004E8	// r/o for mcu, r/w for bp 
#define SFP_P5_SN5_ADDR				0x60800528	// r/o for mcu, r/w for bp 
#define SFP_P6_SN5_ADDR				0x60800568	// r/o for mcu, r/w for bp 
#define SFP_P7_SN5_ADDR				0x608005A8	// r/o for mcu, r/w for bp 

#define SFP_P1_SN6_ADDR				0x6080042A	// r/o for mcu, r/w for bp 
#define SFP_P2_SN6_ADDR				0x6080046A	// r/o for mcu, r/w for bp 
#define SFP_P3_SN6_ADDR				0x608004AA	// r/o for mcu, r/w for bp 
#define SFP_P4_SN6_ADDR				0x608004EA	// r/o for mcu, r/w for bp 
#define SFP_P5_SN6_ADDR				0x6080052A	// r/o for mcu, r/w for bp 
#define SFP_P6_SN6_ADDR				0x6080056A	// r/o for mcu, r/w for bp 
#define SFP_P7_SN6_ADDR				0x608005AA	// r/o for mcu, r/w for bp 

#define SFP_P1_SN7_ADDR				0x6080042C	// r/o for mcu, r/w for bp 
#define SFP_P2_SN7_ADDR				0x6080046C	// r/o for mcu, r/w for bp 
#define SFP_P3_SN7_ADDR				0x608004AC	// r/o for mcu, r/w for bp 
#define SFP_P4_SN7_ADDR				0x608004EC	// r/o for mcu, r/w for bp 
#define SFP_P5_SN7_ADDR				0x6080052C	// r/o for mcu, r/w for bp 
#define SFP_P6_SN7_ADDR				0x6080056C	// r/o for mcu, r/w for bp 
#define SFP_P7_SN7_ADDR				0x608005AC	// r/o for mcu, r/w for bp 

#define SFP_P1_SN8_ADDR				0x6080042E	// r/o for mcu, r/w for bp 
#define SFP_P2_SN8_ADDR				0x6080046E	// r/o for mcu, r/w for bp 
#define SFP_P3_SN8_ADDR				0x608004AE	// r/o for mcu, r/w for bp 
#define SFP_P4_SN8_ADDR				0x608004EE	// r/o for mcu, r/w for bp 
#define SFP_P5_SN8_ADDR				0x6080052E	// r/o for mcu, r/w for bp 
#define SFP_P6_SN8_ADDR				0x6080056E	// r/o for mcu, r/w for bp 
#define SFP_P7_SN8_ADDR				0x608005AE	// r/o for mcu, r/w for bp 

#define SFP_P1_TEMP_ADDR			0x60800430	// r/o for mcu, r/w for bp 
#define SFP_P2_TEMP_ADDR			0x60800470	// r/o for mcu, r/w for bp 
#define SFP_P3_TEMP_ADDR			0x608004B0	// r/o for mcu, r/w for bp 
#define SFP_P4_TEMP_ADDR			0x608004F0	// r/o for mcu, r/w for bp 
#define SFP_P5_TEMP_ADDR			0x60800530	// r/o for mcu, r/w for bp 
#define SFP_P6_TEMP_ADDR			0x60800570	// r/o for mcu, r/w for bp 
#define SFP_P7_TEMP_ADDR			0x608005B0	// r/o for mcu, r/w for bp 

#define SFP_P1_RATE_ADDR			0x60800432	// r/o for mcu, r/w for bp 
#define SFP_P2_RATE_ADDR			0x60800472	// r/o for mcu, r/w for bp 
#define SFP_P3_RATE_ADDR			0x608004B2	// r/o for mcu, r/w for bp 
#define SFP_P4_RATE_ADDR			0x608004F2	// r/o for mcu, r/w for bp 
#define SFP_P5_RATE_ADDR			0x60800532	// r/o for mcu, r/w for bp 
#define SFP_P6_RATE_ADDR			0x60800572	// r/o for mcu, r/w for bp 
#define SFP_P7_RATE_ADDR			0x608005B2	// r/o for mcu, r/w for bp 

#define SFP_P1_VOLT_ADDR			0x60800434	// r/o for mcu, r/w for bp 
#define SFP_P2_VOLT_ADDR			0x60800474	// r/o for mcu, r/w for bp 
#define SFP_P3_VOLT_ADDR			0x608004B4	// r/o for mcu, r/w for bp 
#define SFP_P4_VOLT_ADDR			0x608004F4	// r/o for mcu, r/w for bp 
#define SFP_P5_VOLT_ADDR			0x60800534	// r/o for mcu, r/w for bp 
#define SFP_P6_VOLT_ADDR			0x60800574	// r/o for mcu, r/w for bp 
#define SFP_P7_VOLT_ADDR			0x608005B4	// r/o for mcu, r/w for bp 

#define SFP_P1_TX_BIAS_ADDR			0x60800436	// r/o for mcu, r/w for bp 
#define SFP_P2_TX_BIAS_ADDR			0x60800476	// r/o for mcu, r/w for bp 
#define SFP_P3_TX_BIAS_ADDR			0x608004B6	// r/o for mcu, r/w for bp 
#define SFP_P4_TX_BIAS_ADDR			0x608004F6	// r/o for mcu, r/w for bp 
#define SFP_P5_TX_BIAS_ADDR			0x60800536	// r/o for mcu, r/w for bp 
#define SFP_P6_TX_BIAS_ADDR			0x60800576	// r/o for mcu, r/w for bp 
#define SFP_P7_TX_BIAS_ADDR			0x608005B6	// r/o for mcu, r/w for bp 

#define SFP_P1_LTEMP_ADDR			0x60800438	// r/o for mcu, r/w for bp 
#define SFP_P2_LTEMP_ADDR			0x60800478	// r/o for mcu, r/w for bp 
#define SFP_P3_LTEMP_ADDR			0x608004B8	// r/o for mcu, r/w for bp 
#define SFP_P4_LTEMP_ADDR			0x608004F8	// r/o for mcu, r/w for bp 
#define SFP_P5_LTEMP_ADDR			0x60800538	// r/o for mcu, r/w for bp 
#define SFP_P6_LTEMP_ADDR			0x60800578	// r/o for mcu, r/w for bp 
#define SFP_P7_LTEMP_ADDR			0x608005B8	// r/o for mcu, r/w for bp 

#define SFP_P1_TCURR_ADDR			0x6080043A	// r/o for mcu, r/w for bp 
#define SFP_P2_TCURR_ADDR			0x6080047A	// r/o for mcu, r/w for bp 
#define SFP_P3_TCURR_ADDR			0x608004BA	// r/o for mcu, r/w for bp 
#define SFP_P4_TCURR_ADDR			0x608004FA	// r/o for mcu, r/w for bp 
#define SFP_P5_TCURR_ADDR			0x6080053A	// r/o for mcu, r/w for bp 
#define SFP_P6_TCURR_ADDR			0x6080057A	// r/o for mcu, r/w for bp 
#define SFP_P7_TCURR_ADDR			0x608005BA	// r/o for mcu, r/w for bp 


/****************************
 * Performance related registers
 ****************************/

#define PM_COUNT_CLEAR_ADDR			0x60800100	// r/w for mcu, r/o for bp, read clear

#define PM_P1_TX_BYTE1_ADDR			0x60800700	// r/o for mcu, r/w for bp 
#define PM_P2_TX_BYTE1_ADDR			0x60800750	// r/o for mcu, r/w for bp 
#define PM_P3_TX_BYTE1_ADDR			0x608007A0	// r/o for mcu, r/w for bp 
#define PM_P4_TX_BYTE1_ADDR			0x608007F0	// r/o for mcu, r/w for bp 
#define PM_P5_TX_BYTE1_ADDR			0x60800840	// r/o for mcu, r/w for bp 
#define PM_P6_TX_BYTE1_ADDR			0x60800890	// r/o for mcu, r/w for bp 
#define PM_P7_TX_BYTE1_ADDR			0x608008E0	// r/o for mcu, r/w for bp 

#define PM_P1_TX_BYTE2_ADDR			0x60800702	// r/o for mcu, r/w for bp 
#define PM_P2_TX_BYTE2_ADDR			0x60800752	// r/o for mcu, r/w for bp 
#define PM_P3_TX_BYTE2_ADDR			0x608007A2	// r/o for mcu, r/w for bp 
#define PM_P4_TX_BYTE2_ADDR			0x608007F2	// r/o for mcu, r/w for bp 
#define PM_P5_TX_BYTE2_ADDR			0x60800842	// r/o for mcu, r/w for bp 
#define PM_P6_TX_BYTE2_ADDR			0x60800892	// r/o for mcu, r/w for bp 
#define PM_P7_TX_BYTE2_ADDR			0x608008E2	// r/o for mcu, r/w for bp 

#define PM_P1_TX_BYTE3_ADDR			0x60800704	// r/o for mcu, r/w for bp 
#define PM_P2_TX_BYTE3_ADDR			0x60800754	// r/o for mcu, r/w for bp 
#define PM_P3_TX_BYTE3_ADDR			0x608007A4	// r/o for mcu, r/w for bp 
#define PM_P4_TX_BYTE3_ADDR			0x608007F4	// r/o for mcu, r/w for bp 
#define PM_P5_TX_BYTE3_ADDR			0x60800844	// r/o for mcu, r/w for bp 
#define PM_P6_TX_BYTE3_ADDR			0x60800894	// r/o for mcu, r/w for bp 
#define PM_P7_TX_BYTE3_ADDR			0x608008E4	// r/o for mcu, r/w for bp 

#define PM_P1_TX_BYTE4_ADDR			0x60800706	// r/o for mcu, r/w for bp 
#define PM_P2_TX_BYTE4_ADDR			0x60800756	// r/o for mcu, r/w for bp 
#define PM_P3_TX_BYTE4_ADDR			0x608007A6	// r/o for mcu, r/w for bp 
#define PM_P4_TX_BYTE4_ADDR			0x608007F6	// r/o for mcu, r/w for bp 
#define PM_P5_TX_BYTE4_ADDR			0x60800846	// r/o for mcu, r/w for bp 
#define PM_P6_TX_BYTE4_ADDR			0x60800896	// r/o for mcu, r/w for bp 
#define PM_P7_TX_BYTE4_ADDR			0x608008E6	// r/o for mcu, r/w for bp 

#define PM_P1_RX_BYTE1_ADDR			0x60800708	// r/o for mcu, r/w for bp 
#define PM_P2_RX_BYTE1_ADDR			0x60800758	// r/o for mcu, r/w for bp 
#define PM_P3_RX_BYTE1_ADDR			0x608007A8	// r/o for mcu, r/w for bp 
#define PM_P4_RX_BYTE1_ADDR			0x608007F8	// r/o for mcu, r/w for bp 
#define PM_P5_RX_BYTE1_ADDR			0x60800848	// r/o for mcu, r/w for bp 
#define PM_P6_RX_BYTE1_ADDR			0x60800898	// r/o for mcu, r/w for bp 
#define PM_P7_RX_BYTE1_ADDR			0x608008E8	// r/o for mcu, r/w for bp 

#define PM_P1_RX_BYTE2_ADDR			0x6080070A	// r/o for mcu, r/w for bp 
#define PM_P2_RX_BYTE2_ADDR			0x6080075A	// r/o for mcu, r/w for bp 
#define PM_P3_RX_BYTE2_ADDR			0x608007AA	// r/o for mcu, r/w for bp 
#define PM_P4_RX_BYTE2_ADDR			0x608007FA	// r/o for mcu, r/w for bp 
#define PM_P5_RX_BYTE2_ADDR			0x6080084A	// r/o for mcu, r/w for bp 
#define PM_P6_RX_BYTE2_ADDR			0x6080089A	// r/o for mcu, r/w for bp 
#define PM_P7_RX_BYTE2_ADDR			0x608008EA	// r/o for mcu, r/w for bp 

#define PM_P1_RX_BYTE3_ADDR			0x6080070C	// r/o for mcu, r/w for bp 
#define PM_P2_RX_BYTE3_ADDR			0x6080075C	// r/o for mcu, r/w for bp 
#define PM_P3_RX_BYTE3_ADDR			0x608007AC	// r/o for mcu, r/w for bp 
#define PM_P4_RX_BYTE3_ADDR			0x608007FC	// r/o for mcu, r/w for bp 
#define PM_P5_RX_BYTE3_ADDR			0x6080084C	// r/o for mcu, r/w for bp 
#define PM_P6_RX_BYTE3_ADDR			0x6080089C	// r/o for mcu, r/w for bp 
#define PM_P7_RX_BYTE3_ADDR			0x608008EC	// r/o for mcu, r/w for bp 

#define PM_P1_RX_BYTE4_ADDR			0x6080070E	// r/o for mcu, r/w for bp 
#define PM_P2_RX_BYTE4_ADDR			0x6080075E	// r/o for mcu, r/w for bp 
#define PM_P3_RX_BYTE4_ADDR			0x608007AE	// r/o for mcu, r/w for bp 
#define PM_P4_RX_BYTE4_ADDR			0x608007FE	// r/o for mcu, r/w for bp 
#define PM_P5_RX_BYTE4_ADDR			0x6080084E	// r/o for mcu, r/w for bp 
#define PM_P6_RX_BYTE4_ADDR			0x6080089E	// r/o for mcu, r/w for bp 
#define PM_P7_RX_BYTE4_ADDR			0x608008EE	// r/o for mcu, r/w for bp 

#define PM_P1_TX_FRAME1_ADDR		0x60800710	// r/o for mcu, r/w for bp 
#define PM_P2_TX_FRAME1_ADDR		0x60800760	// r/o for mcu, r/w for bp 
#define PM_P3_TX_FRAME1_ADDR		0x608007B0	// r/o for mcu, r/w for bp 
#define PM_P4_TX_FRAME1_ADDR		0x60800800	// r/o for mcu, r/w for bp 
#define PM_P5_TX_FRAME1_ADDR		0x60800850	// r/o for mcu, r/w for bp 
#define PM_P6_TX_FRAME1_ADDR		0x608008A0	// r/o for mcu, r/w for bp 
#define PM_P7_TX_FRAME1_ADDR		0x608008F0	// r/o for mcu, r/w for bp 

#define PM_P1_TX_FRAME2_ADDR		0x60800712	// r/o for mcu, r/w for bp 
#define PM_P2_TX_FRAME2_ADDR		0x60800762	// r/o for mcu, r/w for bp 
#define PM_P3_TX_FRAME2_ADDR		0x608007B2	// r/o for mcu, r/w for bp 
#define PM_P4_TX_FRAME2_ADDR		0x60800802	// r/o for mcu, r/w for bp 
#define PM_P5_TX_FRAME2_ADDR		0x60800852	// r/o for mcu, r/w for bp 
#define PM_P6_TX_FRAME2_ADDR		0x608008A2	// r/o for mcu, r/w for bp 
#define PM_P7_TX_FRAME2_ADDR		0x608008F2	// r/o for mcu, r/w for bp 

#define PM_P1_TX_FRAME3_ADDR		0x60800714	// r/o for mcu, r/w for bp 
#define PM_P2_TX_FRAME3_ADDR		0x60800764	// r/o for mcu, r/w for bp 
#define PM_P3_TX_FRAME3_ADDR		0x608007B4	// r/o for mcu, r/w for bp 
#define PM_P4_TX_FRAME3_ADDR		0x60800804	// r/o for mcu, r/w for bp 
#define PM_P5_TX_FRAME3_ADDR		0x60800854	// r/o for mcu, r/w for bp 
#define PM_P6_TX_FRAME3_ADDR		0x608008A4	// r/o for mcu, r/w for bp 
#define PM_P7_TX_FRAME3_ADDR		0x608008F4	// r/o for mcu, r/w for bp 

#define PM_P1_TX_FRAME4_ADDR		0x60800716	// r/o for mcu, r/w for bp 
#define PM_P2_TX_FRAME4_ADDR		0x60800766	// r/o for mcu, r/w for bp 
#define PM_P3_TX_FRAME4_ADDR		0x608007B6	// r/o for mcu, r/w for bp 
#define PM_P4_TX_FRAME4_ADDR		0x60800806	// r/o for mcu, r/w for bp 
#define PM_P5_TX_FRAME4_ADDR		0x60800856	// r/o for mcu, r/w for bp 
#define PM_P6_TX_FRAME4_ADDR		0x608008A6	// r/o for mcu, r/w for bp 
#define PM_P7_TX_FRAME4_ADDR		0x608008F6	// r/o for mcu, r/w for bp 

#define PM_P1_RX_FRAME1_ADDR		0x60800718	// r/o for mcu, r/w for bp 
#define PM_P2_RX_FRAME1_ADDR		0x60800768	// r/o for mcu, r/w for bp 
#define PM_P3_RX_FRAME1_ADDR		0x608007B8	// r/o for mcu, r/w for bp 
#define PM_P4_RX_FRAME1_ADDR		0x60800808	// r/o for mcu, r/w for bp 
#define PM_P5_RX_FRAME1_ADDR		0x60800858	// r/o for mcu, r/w for bp 
#define PM_P6_RX_FRAME1_ADDR		0x608008A8	// r/o for mcu, r/w for bp 
#define PM_P7_RX_FRAME1_ADDR		0x608008F8	// r/o for mcu, r/w for bp 

#define PM_P1_RX_FRAME2_ADDR		0x6080071A	// r/o for mcu, r/w for bp 
#define PM_P2_RX_FRAME2_ADDR		0x6080076A	// r/o for mcu, r/w for bp 
#define PM_P3_RX_FRAME2_ADDR		0x608007BA	// r/o for mcu, r/w for bp 
#define PM_P4_RX_FRAME2_ADDR		0x6080080A	// r/o for mcu, r/w for bp 
#define PM_P5_RX_FRAME2_ADDR		0x6080085A	// r/o for mcu, r/w for bp 
#define PM_P6_RX_FRAME2_ADDR		0x608008AA	// r/o for mcu, r/w for bp 
#define PM_P7_RX_FRAME2_ADDR		0x608008FA	// r/o for mcu, r/w for bp 

#define PM_P1_RX_FRAME3_ADDR		0x6080071C	// r/o for mcu, r/w for bp 
#define PM_P2_RX_FRAME3_ADDR		0x6080076C	// r/o for mcu, r/w for bp 
#define PM_P3_RX_FRAME3_ADDR		0x608007BC	// r/o for mcu, r/w for bp 
#define PM_P4_RX_FRAME3_ADDR		0x6080080C	// r/o for mcu, r/w for bp 
#define PM_P5_RX_FRAME3_ADDR		0x6080085C	// r/o for mcu, r/w for bp 
#define PM_P6_RX_FRAME3_ADDR		0x608008AC	// r/o for mcu, r/w for bp 
#define PM_P7_RX_FRAME3_ADDR		0x608008FC	// r/o for mcu, r/w for bp 

#define PM_P1_RX_FRAME4_ADDR		0x6080071E	// r/o for mcu, r/w for bp 
#define PM_P2_RX_FRAME4_ADDR		0x6080076E	// r/o for mcu, r/w for bp 
#define PM_P3_RX_FRAME4_ADDR		0x608007BE	// r/o for mcu, r/w for bp 
#define PM_P4_RX_FRAME4_ADDR		0x6080080E	// r/o for mcu, r/w for bp 
#define PM_P5_RX_FRAME4_ADDR		0x6080085E	// r/o for mcu, r/w for bp 
#define PM_P6_RX_FRAME4_ADDR		0x608008AE	// r/o for mcu, r/w for bp 
#define PM_P7_RX_FRAME4_ADDR		0x608008FE	// r/o for mcu, r/w for bp 

#define PM_P1_FCS1_ADDR				0x60800720	// r/o for mcu, r/w for bp 
#define PM_P2_FCS1_ADDR				0x60800770	// r/o for mcu, r/w for bp 
#define PM_P3_FCS1_ADDR				0x608007C0	// r/o for mcu, r/w for bp 
#define PM_P4_FCS1_ADDR				0x60800810	// r/o for mcu, r/w for bp 
#define PM_P5_FCS1_ADDR				0x60800860	// r/o for mcu, r/w for bp 
#define PM_P6_FCS1_ADDR				0x608008B0	// r/o for mcu, r/w for bp 
#define PM_P7_FCS1_ADDR				0x60800900	// r/o for mcu, r/w for bp 

#define PM_P1_FCS2_ADDR				0x60800722	// r/o for mcu, r/w for bp 
#define PM_P2_FCS2_ADDR				0x60800772	// r/o for mcu, r/w for bp 
#define PM_P3_FCS2_ADDR				0x608007C2	// r/o for mcu, r/w for bp 
#define PM_P4_FCS2_ADDR				0x60800812	// r/o for mcu, r/w for bp 
#define PM_P5_FCS2_ADDR				0x60800862	// r/o for mcu, r/w for bp 
#define PM_P6_FCS2_ADDR				0x608008B2	// r/o for mcu, r/w for bp 
#define PM_P7_FCS2_ADDR				0x60800902	// r/o for mcu, r/w for bp 

#define PM_P1_FCS3_ADDR				0x60800724	// r/o for mcu, r/w for bp 
#define PM_P2_FCS3_ADDR				0x60800774	// r/o for mcu, r/w for bp 
#define PM_P3_FCS3_ADDR				0x608007C4	// r/o for mcu, r/w for bp 
#define PM_P4_FCS3_ADDR				0x60800814	// r/o for mcu, r/w for bp 
#define PM_P5_FCS3_ADDR				0x60800864	// r/o for mcu, r/w for bp 
#define PM_P6_FCS3_ADDR				0x608008B4	// r/o for mcu, r/w for bp 
#define PM_P7_FCS3_ADDR				0x60800904	// r/o for mcu, r/w for bp 

#define PM_P1_FCS4_ADDR				0x60800726	// r/o for mcu, r/w for bp 
#define PM_P2_FCS4_ADDR				0x60800776	// r/o for mcu, r/w for bp 
#define PM_P3_FCS4_ADDR				0x608007C6	// r/o for mcu, r/w for bp 
#define PM_P4_FCS4_ADDR				0x60800816	// r/o for mcu, r/w for bp 
#define PM_P5_FCS4_ADDR				0x60800866	// r/o for mcu, r/w for bp 
#define PM_P6_FCS4_ADDR				0x608008B6	// r/o for mcu, r/w for bp 
#define PM_P7_FCS4_ADDR				0x60800906	// r/o for mcu, r/w for bp 

#define PM_P1_FCS_OK1_ADDR			0x60800730	// r/o for mcu, r/w for bp 
#define PM_P2_FCS_OK1_ADDR			0x60800780	// r/o for mcu, r/w for bp 
#define PM_P3_FCS_OK1_ADDR			0x608007D0	// r/o for mcu, r/w for bp 
#define PM_P4_FCS_OK1_ADDR			0x60800820	// r/o for mcu, r/w for bp 
#define PM_P5_FCS_OK1_ADDR			0x60800870	// r/o for mcu, r/w for bp 
#define PM_P6_FCS_OK1_ADDR			0x608008C0	// r/o for mcu, r/w for bp 
#define PM_P7_FCS_OK1_ADDR			0x60800910	// r/o for mcu, r/w for bp 

#define PM_P1_FCS_OK2_ADDR			0x60800732	// r/o for mcu, r/w for bp 
#define PM_P2_FCS_OK2_ADDR			0x60800782	// r/o for mcu, r/w for bp 
#define PM_P3_FCS_OK2_ADDR			0x608007D2	// r/o for mcu, r/w for bp 
#define PM_P4_FCS_OK2_ADDR			0x60800822	// r/o for mcu, r/w for bp 
#define PM_P5_FCS_OK2_ADDR			0x60800872	// r/o for mcu, r/w for bp 
#define PM_P6_FCS_OK2_ADDR			0x608008C2	// r/o for mcu, r/w for bp 
#define PM_P7_FCS_OK2_ADDR			0x60800912	// r/o for mcu, r/w for bp 

#define PM_P1_FCS_OK3_ADDR			0x60800734	// r/o for mcu, r/w for bp 
#define PM_P2_FCS_OK3_ADDR			0x60800784	// r/o for mcu, r/w for bp 
#define PM_P3_FCS_OK3_ADDR			0x608007D4	// r/o for mcu, r/w for bp 
#define PM_P4_FCS_OK3_ADDR			0x60800824	// r/o for mcu, r/w for bp 
#define PM_P5_FCS_OK3_ADDR			0x60800874	// r/o for mcu, r/w for bp 
#define PM_P6_FCS_OK3_ADDR			0x608008C4	// r/o for mcu, r/w for bp 
#define PM_P7_FCS_OK3_ADDR			0x60800914	// r/o for mcu, r/w for bp 

#define PM_P1_FCS_OK4_ADDR			0x60800736	// r/o for mcu, r/w for bp 
#define PM_P2_FCS_OK4_ADDR			0x60800786	// r/o for mcu, r/w for bp 
#define PM_P3_FCS_OK4_ADDR			0x608007D6	// r/o for mcu, r/w for bp 
#define PM_P4_FCS_OK4_ADDR			0x60800826	// r/o for mcu, r/w for bp 
#define PM_P5_FCS_OK4_ADDR			0x60800876	// r/o for mcu, r/w for bp 
#define PM_P6_FCS_OK4_ADDR			0x608008C6	// r/o for mcu, r/w for bp 
#define PM_P7_FCS_OK4_ADDR			0x60800916	// r/o for mcu, r/w for bp 

#define PM_P1_FCS_NOK1_ADDR			0x60800738	// r/o for mcu, r/w for bp 
#define PM_P2_FCS_NOK1_ADDR			0x60800788	// r/o for mcu, r/w for bp 
#define PM_P3_FCS_NOK1_ADDR			0x608007D8	// r/o for mcu, r/w for bp 
#define PM_P4_FCS_NOK1_ADDR			0x60800828	// r/o for mcu, r/w for bp 
#define PM_P5_FCS_NOK1_ADDR			0x60800878	// r/o for mcu, r/w for bp 
#define PM_P6_FCS_NOK1_ADDR			0x608008C8	// r/o for mcu, r/w for bp 
#define PM_P7_FCS_NOK1_ADDR			0x60800918	// r/o for mcu, r/w for bp 

#define PM_P1_FCS_NOK2_ADDR			0x6080073A	// r/o for mcu, r/w for bp 
#define PM_P2_FCS_NOK2_ADDR			0x6080078A	// r/o for mcu, r/w for bp 
#define PM_P3_FCS_NOK2_ADDR			0x608007DA	// r/o for mcu, r/w for bp 
#define PM_P4_FCS_NOK2_ADDR			0x6080082A	// r/o for mcu, r/w for bp 
#define PM_P5_FCS_NOK2_ADDR			0x6080087A	// r/o for mcu, r/w for bp 
#define PM_P6_FCS_NOK2_ADDR			0x608008CA	// r/o for mcu, r/w for bp 
#define PM_P7_FCS_NOK2_ADDR			0x6080091A	// r/o for mcu, r/w for bp 

#define PM_P1_FCS_NOK3_ADDR			0x6080073C	// r/o for mcu, r/w for bp 
#define PM_P2_FCS_NOK3_ADDR			0x6080078C	// r/o for mcu, r/w for bp 
#define PM_P3_FCS_NOK3_ADDR			0x608007DC	// r/o for mcu, r/w for bp 
#define PM_P4_FCS_NOK3_ADDR			0x6080082C	// r/o for mcu, r/w for bp 
#define PM_P5_FCS_NOK3_ADDR			0x6080087C	// r/o for mcu, r/w for bp 
#define PM_P6_FCS_NOK3_ADDR			0x608008CC	// r/o for mcu, r/w for bp 
#define PM_P7_FCS_NOK3_ADDR			0x6080091C	// r/o for mcu, r/w for bp 

#define PM_P1_FCS_NOK4_ADDR			0x6080073E	// r/o for mcu, r/w for bp 
#define PM_P2_FCS_NOK4_ADDR			0x6080078E	// r/o for mcu, r/w for bp 
#define PM_P3_FCS_NOK4_ADDR			0x608007DE	// r/o for mcu, r/w for bp 
#define PM_P4_FCS_NOK4_ADDR			0x6080082E	// r/o for mcu, r/w for bp 
#define PM_P5_FCS_NOK4_ADDR			0x6080087E	// r/o for mcu, r/w for bp 
#define PM_P6_FCS_NOK4_ADDR			0x608008CE	// r/o for mcu, r/w for bp 
#define PM_P7_FCS_NOK4_ADDR			0x6080091E	// r/o for mcu, r/w for bp 


/****************************
 * DCO related registers
 ****************************/

//TBD


/****************************
 * CLEI/USI Information related registers
 ****************************/

#define PORT1_CLEI1_ADDR 			0x60800B00	// r/o for mcu, r/w for bp 
#define PORT2_CLEI1_ADDR 			0x60800B10	// r/o for mcu, r/w for bp 
#define PORT3_CLEI1_ADDR 			0x60800B20	// r/o for mcu, r/w for bp 
#define PORT4_CLEI1_ADDR 			0x60800B30	// r/o for mcu, r/w for bp 
#define PORT5_CLEI1_ADDR 			0x60800B40	// r/o for mcu, r/w for bp 
#define PORT6_CLEI1_ADDR 			0x60800B50	// r/o for mcu, r/w for bp 
#define PORT7_CLEI1_ADDR 			0x60800B60	// r/o for mcu, r/w for bp 

#define PORT1_CLEI2_ADDR 			0x60800B02	// r/o for mcu, r/w for bp 
#define PORT2_CLEI2_ADDR 			0x60800B12	// r/o for mcu, r/w for bp 
#define PORT3_CLEI2_ADDR 			0x60800B22	// r/o for mcu, r/w for bp 
#define PORT4_CLEI2_ADDR 			0x60800B32	// r/o for mcu, r/w for bp 
#define PORT5_CLEI2_ADDR 			0x60800B42	// r/o for mcu, r/w for bp 
#define PORT6_CLEI2_ADDR 			0x60800B52	// r/o for mcu, r/w for bp 
#define PORT7_CLEI2_ADDR 			0x60800B62	// r/o for mcu, r/w for bp 

#define PORT1_CLEI3_ADDR 			0x60800B04	// r/o for mcu, r/w for bp 
#define PORT2_CLEI3_ADDR 			0x60800B14	// r/o for mcu, r/w for bp 
#define PORT3_CLEI3_ADDR 			0x60800B24	// r/o for mcu, r/w for bp 
#define PORT4_CLEI3_ADDR 			0x60800B34	// r/o for mcu, r/w for bp 
#define PORT5_CLEI3_ADDR 			0x60800B44	// r/o for mcu, r/w for bp 
#define PORT6_CLEI3_ADDR 			0x60800B54	// r/o for mcu, r/w for bp 
#define PORT7_CLEI3_ADDR 			0x60800B64	// r/o for mcu, r/w for bp 

#define PORT1_CLEI4_ADDR 			0x60800B06	// r/o for mcu, r/w for bp 
#define PORT2_CLEI4_ADDR 			0x60800B16	// r/o for mcu, r/w for bp 
#define PORT3_CLEI4_ADDR 			0x60800B26	// r/o for mcu, r/w for bp 
#define PORT4_CLEI4_ADDR 			0x60800B36	// r/o for mcu, r/w for bp 
#define PORT5_CLEI4_ADDR 			0x60800B46	// r/o for mcu, r/w for bp 
#define PORT6_CLEI4_ADDR 			0x60800B56	// r/o for mcu, r/w for bp 
#define PORT7_CLEI4_ADDR 			0x60800B66	// r/o for mcu, r/w for bp 

#define PORT1_CLEI5_ADDR 			0x60800B08	// r/o for mcu, r/w for bp 
#define PORT2_CLEI5_ADDR 			0x60800B18	// r/o for mcu, r/w for bp 
#define PORT3_CLEI5_ADDR 			0x60800B28	// r/o for mcu, r/w for bp 
#define PORT4_CLEI5_ADDR 			0x60800B38	// r/o for mcu, r/w for bp 
#define PORT5_CLEI5_ADDR 			0x60800B48	// r/o for mcu, r/w for bp 
#define PORT6_CLEI5_ADDR 			0x60800B58	// r/o for mcu, r/w for bp 
#define PORT7_CLEI5_ADDR 			0x60800B68	// r/o for mcu, r/w for bp 

#define PORT1_USI1_ADDR 			0x60800C00	// r/o for mcu, r/w for bp 
#define PORT2_USI1_ADDR 			0x60800C20	// r/o for mcu, r/w for bp 
#define PORT3_USI1_ADDR 			0x60800C40	// r/o for mcu, r/w for bp 
#define PORT4_USI1_ADDR 			0x60800C60	// r/o for mcu, r/w for bp 
#define PORT5_USI1_ADDR 			0x60800C80	// r/o for mcu, r/w for bp 
#define PORT6_USI1_ADDR 			0x60800CA0	// r/o for mcu, r/w for bp 
#define PORT7_USI1_ADDR 			0x60800CC0	// r/o for mcu, r/w for bp 

#define PORT1_USI2_ADDR 			0x60800C02	// r/o for mcu, r/w for bp 
#define PORT2_USI2_ADDR 			0x60800C22	// r/o for mcu, r/w for bp 
#define PORT3_USI2_ADDR 			0x60800C42	// r/o for mcu, r/w for bp 
#define PORT4_USI2_ADDR 			0x60800C62	// r/o for mcu, r/w for bp 
#define PORT5_USI2_ADDR 			0x60800C82	// r/o for mcu, r/w for bp 
#define PORT6_USI2_ADDR 			0x60800CA2	// r/o for mcu, r/w for bp 
#define PORT7_USI2_ADDR 			0x60800CC2	// r/o for mcu, r/w for bp 

#define PORT1_USI3_ADDR 			0x60800C04	// r/o for mcu, r/w for bp 
#define PORT2_USI3_ADDR 			0x60800C24	// r/o for mcu, r/w for bp 
#define PORT3_USI3_ADDR 			0x60800C44	// r/o for mcu, r/w for bp 
#define PORT4_USI3_ADDR 			0x60800C64	// r/o for mcu, r/w for bp 
#define PORT5_USI3_ADDR 			0x60800C84	// r/o for mcu, r/w for bp 
#define PORT6_USI3_ADDR 			0x60800CA4	// r/o for mcu, r/w for bp 
#define PORT7_USI3_ADDR 			0x60800CC4	// r/o for mcu, r/w for bp 

#define PORT1_USI4_ADDR 			0x60800C06	// r/o for mcu, r/w for bp 
#define PORT2_USI4_ADDR 			0x60800C26	// r/o for mcu, r/w for bp 
#define PORT3_USI4_ADDR 			0x60800C46	// r/o for mcu, r/w for bp 
#define PORT4_USI4_ADDR 			0x60800C66	// r/o for mcu, r/w for bp 
#define PORT5_USI4_ADDR 			0x60800C86	// r/o for mcu, r/w for bp 
#define PORT6_USI4_ADDR 			0x60800CA6	// r/o for mcu, r/w for bp 
#define PORT7_USI4_ADDR 			0x60800CC6	// r/o for mcu, r/w for bp 

#define PORT1_USI5_ADDR 			0x60800C08	// r/o for mcu, r/w for bp 
#define PORT2_USI5_ADDR 			0x60800C28	// r/o for mcu, r/w for bp 
#define PORT3_USI5_ADDR 			0x60800C48	// r/o for mcu, r/w for bp 
#define PORT4_USI5_ADDR 			0x60800C68	// r/o for mcu, r/w for bp 
#define PORT5_USI5_ADDR 			0x60800C88	// r/o for mcu, r/w for bp 
#define PORT6_USI5_ADDR 			0x60800CA8	// r/o for mcu, r/w for bp 
#define PORT7_USI5_ADDR 			0x60800CC8	// r/o for mcu, r/w for bp 

#define PORT1_USI6_ADDR 			0x60800C0A	// r/o for mcu, r/w for bp 
#define PORT2_USI6_ADDR 			0x60800C2A	// r/o for mcu, r/w for bp 
#define PORT3_USI6_ADDR 			0x60800C4A	// r/o for mcu, r/w for bp 
#define PORT4_USI6_ADDR 			0x60800C6A	// r/o for mcu, r/w for bp 
#define PORT5_USI6_ADDR 			0x60800C8A	// r/o for mcu, r/w for bp 
#define PORT6_USI6_ADDR 			0x60800CAA	// r/o for mcu, r/w for bp 
#define PORT7_USI6_ADDR 			0x60800CCA	// r/o for mcu, r/w for bp 

#define PORT1_USI7_ADDR 			0x60800C0C	// r/o for mcu, r/w for bp 
#define PORT2_USI7_ADDR 			0x60800C2C	// r/o for mcu, r/w for bp 
#define PORT3_USI7_ADDR 			0x60800C4C	// r/o for mcu, r/w for bp 
#define PORT4_USI7_ADDR 			0x60800C6C	// r/o for mcu, r/w for bp 
#define PORT5_USI7_ADDR 			0x60800C8C	// r/o for mcu, r/w for bp 
#define PORT6_USI7_ADDR 			0x60800CAC	// r/o for mcu, r/w for bp 
#define PORT7_USI7_ADDR 			0x60800CCC	// r/o for mcu, r/w for bp 

#define PORT1_USI8_ADDR 			0x60800C0E	// r/o for mcu, r/w for bp 
#define PORT2_USI8_ADDR 			0x60800C2E	// r/o for mcu, r/w for bp 
#define PORT3_USI8_ADDR 			0x60800C4E	// r/o for mcu, r/w for bp 
#define PORT4_USI8_ADDR 			0x60800C6E	// r/o for mcu, r/w for bp 
#define PORT5_USI8_ADDR 			0x60800C8E	// r/o for mcu, r/w for bp 
#define PORT6_USI8_ADDR 			0x60800CAE	// r/o for mcu, r/w for bp 
#define PORT7_USI8_ADDR 			0x60800CCE	// r/o for mcu, r/w for bp 

#define PORT1_USI9_ADDR 			0x60800C10	// r/o for mcu, r/w for bp 
#define PORT2_USI9_ADDR 			0x60800C30	// r/o for mcu, r/w for bp 
#define PORT3_USI9_ADDR 			0x60800C50	// r/o for mcu, r/w for bp 
#define PORT4_USI9_ADDR 			0x60800C70	// r/o for mcu, r/w for bp 
#define PORT5_USI9_ADDR 			0x60800C90	// r/o for mcu, r/w for bp 
#define PORT6_USI9_ADDR 			0x60800CB0	// r/o for mcu, r/w for bp 
#define PORT7_USI9_ADDR 			0x60800CD0	// r/o for mcu, r/w for bp 

#define PORT1_USI10_ADDR 			0x60800C12	// r/o for mcu, r/w for bp 
#define PORT2_USI10_ADDR 			0x60800C32	// r/o for mcu, r/w for bp 
#define PORT3_USI10_ADDR 			0x60800C52	// r/o for mcu, r/w for bp 
#define PORT4_USI10_ADDR 			0x60800C72	// r/o for mcu, r/w for bp 
#define PORT5_USI10_ADDR 			0x60800C92	// r/o for mcu, r/w for bp 
#define PORT6_USI10_ADDR 			0x60800CB2	// r/o for mcu, r/w for bp 
#define PORT7_USI10_ADDR 			0x60800CD2	// r/o for mcu, r/w for bp 

#define PORT1_USI11_ADDR 			0x60800C14	// r/o for mcu, r/w for bp 
#define PORT2_USI11_ADDR 			0x60800C34	// r/o for mcu, r/w for bp 
#define PORT3_USI11_ADDR 			0x60800C54	// r/o for mcu, r/w for bp 
#define PORT4_USI11_ADDR 			0x60800C74	// r/o for mcu, r/w for bp 
#define PORT5_USI11_ADDR 			0x60800C94	// r/o for mcu, r/w for bp 
#define PORT6_USI11_ADDR 			0x60800CB4	// r/o for mcu, r/w for bp 
#define PORT7_USI11_ADDR 			0x60800CD4	// r/o for mcu, r/w for bp 

#define PORT1_USI12_ADDR 			0x60800C16	// r/o for mcu, r/w for bp 
#define PORT2_USI12_ADDR 			0x60800C36	// r/o for mcu, r/w for bp 
#define PORT3_USI12_ADDR 			0x60800C56	// r/o for mcu, r/w for bp 
#define PORT4_USI12_ADDR 			0x60800C76	// r/o for mcu, r/w for bp 
#define PORT5_USI12_ADDR 			0x60800C96	// r/o for mcu, r/w for bp 
#define PORT6_USI12_ADDR 			0x60800CB6	// r/o for mcu, r/w for bp 
#define PORT7_USI12_ADDR 			0x60800CD6	// r/o for mcu, r/w for bp 

#define PORT1_USI13_ADDR 			0x60800C18	// r/o for mcu, r/w for bp 
#define PORT2_USI13_ADDR 			0x60800C38	// r/o for mcu, r/w for bp 
#define PORT3_USI13_ADDR 			0x60800C58	// r/o for mcu, r/w for bp 
#define PORT4_USI13_ADDR 			0x60800C78	// r/o for mcu, r/w for bp 
#define PORT5_USI13_ADDR 			0x60800C98	// r/o for mcu, r/w for bp 
#define PORT6_USI13_ADDR 			0x60800CB8	// r/o for mcu, r/w for bp 
#define PORT7_USI13_ADDR 			0x60800CD8	// r/o for mcu, r/w for bp 


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
