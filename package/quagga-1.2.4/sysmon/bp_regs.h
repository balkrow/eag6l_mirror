#ifndef _BP_REGS_H_
#define _BP_REGS_H_


/****************************
 * unit related registers
 ****************************/

#define SW_VERSION_ADDR				0x0000	// r/o for mcu, r/w for bp
#define HW_KEEP_ALIVE_1_ADDR		0x0014	// r/w for mcu, r/o for bp
#define HW_KEEP_ALIVE_2_ADDR		0x0016	// r/o for mcu, r/w for bp :
                                            //   bp must increase this value.
#define INIT_COMPLETE_ADDR			0x0018	// bp set 0xAA to [15:8] if init done.

// bp must set 10G/25G speed for port 1~6.
#define COMMON_CTRL2_P1_ADDR		0x0020	// r/w for mcu, r/o for bp 
#define COMMON_CTRL2_P2_ADDR		0x0022	// r/w for mcu, r/o for bp 
#define COMMON_CTRL2_P3_ADDR		0x0024	// r/w for mcu, r/o for bp 
#define COMMON_CTRL2_P4_ADDR		0x0026	// r/w for mcu, r/o for bp 
#define COMMON_CTRL2_P5_ADDR		0x0028	// r/w for mcu, r/o for bp 
#define COMMON_CTRL2_P6_ADDR		0x002A	// r/w for mcu, r/o for bp 
#define COMMON_CTRL2_P7_ADDR		0x002C	// r/w for mcu, r/o for bp 


/****************************
 * Reset/CR related registers
 ****************************/

#define CHIP_RESET_ADDR				0x0020	// r/w for both mcu/bp.
                                            // reset fpga if FPGA_rst[15:8] is 0x5A.
                                            // reset bp if BP_rst[7:0] is 0x5A.
                                            // clear field if action is done.


/****************************
 * DCO related registers
 ****************************/

#define DCO_INTERFACE_ADDR			0x0060	// r/w for mcu, r/o for bp 
#define DCO_WRITE_CMD_ADDR			0x0062	// r/w for mcu, r/o for bp 
#define DCO_READ_CMD_ADDR			0x0064	// r/w for mcu, r/o for bp 
#define DCO_ACCESS_METHOD_ADDR		0x0066	// r/w for mcu, r/o for bp 


/****************************
 * DCO related registers
 ****************************/

// H/W manufacture
#define INV_HW_MANU_1_ADDR			0x0370	// r/o for mcu, r/w for bp 
#define INV_HW_MANU_2_ADDR			0x0372	// r/o for mcu, r/w for bp 
#define INV_HW_MANU_3_ADDR			0x0374	// r/o for mcu, r/w for bp 
#define INV_HW_MANU_4_ADDR			0x0376	// r/o for mcu, r/w for bp 
#define INV_HW_MANU_5_ADDR			0x0378	// r/o for mcu, r/w for bp 

// H/W model
#define INV_HW_MODEL_1_ADDR			0x0380	// r/o for mcu, r/w for bp 
#define INV_HW_MODEL_2_ADDR			0x0382	// r/o for mcu, r/w for bp 
#define INV_HW_MODEL_3_ADDR			0x0384	// r/o for mcu, r/w for bp 
#define INV_HW_MODEL_4_ADDR			0x0386	// r/o for mcu, r/w for bp 
#define INV_HW_MODEL_5_ADDR			0x0388	// r/o for mcu, r/w for bp 

// H/W part number
#define INV_HW_PN_1_ADDR			0x0390	// r/o for mcu, r/w for bp 
#define INV_HW_PN_2_ADDR			0x0392	// r/o for mcu, r/w for bp 
#define INV_HW_PN_3_ADDR			0x0394	// r/o for mcu, r/w for bp 
#define INV_HW_PN_4_ADDR			0x0396	// r/o for mcu, r/w for bp 
#define INV_HW_PN_5_ADDR			0x0398	// r/o for mcu, r/w for bp 
#define INV_HW_PN_6_ADDR			0x039A	// r/o for mcu, r/w for bp 
#define INV_HW_PN_7_ADDR			0x039C	// r/o for mcu, r/w for bp 
#define INV_HW_PN_8_ADDR			0x039E	// r/o for mcu, r/w for bp 

// H/W serial number
#define INV_HW_SN_1_ADDR			0x03A0	// r/o for mcu, r/w for bp 
#define INV_HW_SN_2_ADDR			0x03A2	// r/o for mcu, r/w for bp 

// H/W revision number
#define INV_HW_REV_1_ADDR			0x03B0	// r/o for mcu, r/w for bp 
#define INV_HW_REV_2_ADDR			0x03B2	// r/o for mcu, r/w for bp 

// H/W manufacture date
#define INV_HW_MDATE_1_ADDR			0x03B4	// r/o for mcu, r/w for bp 
#define INV_HW_MDATE_2_ADDR			0x03B6	// r/o for mcu, r/w for bp 
#define INV_HW_MDATE_3_ADDR			0x03B8	// r/o for mcu, r/w for bp 
#define INV_HW_MDATE_4_ADDR			0x03BA	// r/o for mcu, r/w for bp 
#define INV_HW_MDATE_5_ADDR			0x03BC	// r/o for mcu, r/w for bp 
#define INV_HW_MDATE_6_ADDR			0x03BE	// r/o for mcu, r/w for bp 

// H/W repair date
#define INV_HW_RDATE_1_ADDR			0x03C0	// r/o for mcu, r/w for bp 
#define INV_HW_RDATE_2_ADDR			0x03C2	// r/o for mcu, r/w for bp 
#define INV_HW_RDATE_3_ADDR			0x03C4	// r/o for mcu, r/w for bp 
#define INV_HW_RDATE_4_ADDR			0x03C6	// r/o for mcu, r/w for bp 
#define INV_HW_RDATE_5_ADDR			0x03C8	// r/o for mcu, r/w for bp 

// H/W repair code
#define INV_HW_RCODE_1_ADDR			0x03CA	// r/o for mcu, r/w for bp 
#define INV_HW_RCODE_2_ADDR			0x03CC	// r/o for mcu, r/w for bp 


/****************************
 * Port Config related registers
 ****************************/

#define PORT_1_CONF_ADDR			0x800000	// r/w for mcu, r/o for bp 
#define PORT_2_CONF_ADDR			0x800002	// r/w for mcu, r/o for bp 
#define PORT_3_CONF_ADDR			0x800004	// r/w for mcu, r/o for bp 
#define PORT_4_CONF_ADDR			0x800006	// r/w for mcu, r/o for bp 
#define PORT_5_CONF_ADDR			0x800008	// r/w for mcu, r/o for bp 
#define PORT_6_CONF_ADDR			0x80000A	// r/w for mcu, r/o for bp 
#define PORT_7_CONF_ADDR			0x80000C	// r/w for mcu, r/o for bp 

#define PORT_7_CONF2_ADDR			0x80000E	// r/w for mcu, r/o for bp 
#define PORT_7_CONF3_ADDR			0x80001E	// r/w for mcu, r/o for bp 

/****************************
 * SyncE related registers
 ****************************/

#define SYNCE_GCONFIG_ADDR			0x800160	// r/w for mcu, r/o for bp 
#define SYNCE_IF_SELECT_ADDR		0x800162	// r/w for mcu, r/o for bp 
#define SYNCE_ESMC_LQL_ADDR			0x800166	// r/o for mcu, r/w for bp 
#define SYNCE_ESMC_SQL_ADDR			0x800168	// r/o for mcu, r/w for bp 
#define SYNCE_ESMC_RQL_ADDR			0x80016A	// r/o for mcu, r/w for bp 
#define SYNCE_SRC_STAT_ADDR			0x80016C	// r/o for mcu, r/w for bp 

#define SYNCE_ESMC_RQL2_ADDR		0x80016E	// r/o for mcu, r/w for bp 
#define SYNCE_ESMC_RQL3_ADDR		0x800170	// r/o for mcu, r/w for bp 
#define SYNCE_ESMC_RQL4_ADDR		0x800172	// r/o for mcu, r/w for bp 
#define SYNCE_ESMC_RQL5_ADDR		0x800174	// r/o for mcu, r/w for bp 


/****************************
 * SFP Port Status/Control related registers
 ****************************/

#define BD_SFP_CR_ADDR				0x800000	// r/o for both mcu/bp. FPGA must provide this.

#define PORT_1_TX_PWR_ADDR			0x800020	// r/o for mcu, r/w for bp 
#define PORT_2_TX_PWR_ADDR			0x800040	// r/o for mcu, r/w for bp 
#define PORT_3_TX_PWR_ADDR			0x800060	// r/o for mcu, r/w for bp 
#define PORT_4_TX_PWR_ADDR			0x800080	// r/o for mcu, r/w for bp 
#define PORT_5_TX_PWR_ADDR			0x8000A0	// r/o for mcu, r/w for bp 
#define PORT_6_TX_PWR_ADDR			0x8000C0	// r/o for mcu, r/w for bp 
#define PORT_7_TX_PWR_ADDR			0x8000E0	// r/o for mcu, r/w for bp 

#define PORT_1_RX_PWR_ADDR			0x800022	// r/o for mcu, r/w for bp 
#define PORT_2_RX_PWR_ADDR			0x800042	// r/o for mcu, r/w for bp 
#define PORT_3_RX_PWR_ADDR			0x800062	// r/o for mcu, r/w for bp 
#define PORT_4_RX_PWR_ADDR			0x800082	// r/o for mcu, r/w for bp 
#define PORT_5_RX_PWR_ADDR			0x8000A2	// r/o for mcu, r/w for bp 
#define PORT_6_RX_PWR_ADDR			0x8000C2	// r/o for mcu, r/w for bp 
#define PORT_7_RX_PWR_ADDR			0x8000E2	// r/o for mcu, r/w for bp 

#define PORT_1_WL1_ADDR				0x800024	// r/o for mcu, r/w for bp 
#define PORT_2_WL1_ADDR				0x800044	// r/o for mcu, r/w for bp 
#define PORT_3_WL1_ADDR				0x800064	// r/o for mcu, r/w for bp 
#define PORT_4_WL1_ADDR				0x800084	// r/o for mcu, r/w for bp 
#define PORT_5_WL1_ADDR				0x8000A4	// r/o for mcu, r/w for bp 
#define PORT_6_WL1_ADDR				0x8000C4	// r/o for mcu, r/w for bp 
#define PORT_7_WL1_ADDR				0x8000E4	// r/o for mcu, r/w for bp 

#define PORT_1_WL2_ADDR				0x800026	// r/o for mcu, r/w for bp 
#define PORT_2_WL2_ADDR				0x800046	// r/o for mcu, r/w for bp 
#define PORT_3_WL2_ADDR				0x800066	// r/o for mcu, r/w for bp 
#define PORT_4_WL2_ADDR				0x800086	// r/o for mcu, r/w for bp 
#define PORT_5_WL2_ADDR				0x8000A6	// r/o for mcu, r/w for bp 
#define PORT_6_WL2_ADDR				0x8000C6	// r/o for mcu, r/w for bp 
#define PORT_7_WL2_ADDR				0x8000E6	// r/o for mcu, r/w for bp 

#define PORT_1_DIST_ADDR			0x80002C	// r/o for mcu, r/w for bp 
#define PORT_2_DIST_ADDR			0x80004C	// r/o for mcu, r/w for bp 
#define PORT_3_DIST_ADDR			0x80006C	// r/o for mcu, r/w for bp 
#define PORT_4_DIST_ADDR			0x80008C	// r/o for mcu, r/w for bp 
#define PORT_5_DIST_ADDR			0x8000AC	// r/o for mcu, r/w for bp 
#define PORT_6_DIST_ADDR			0x8000CC	// r/o for mcu, r/w for bp 
#define PORT_7_DIST_ADDR			0x8000EC	// r/o for mcu, r/w for bp 

#define PORT_1_STSFP_STAT_ADDR		0x80002E	// r/o for mcu, r/w for bp 
#define PORT_2_STSFP_STAT_ADDR		0x80004E	// r/o for mcu, r/w for bp 
#define PORT_3_STSFP_STAT_ADDR		0x80006E	// r/o for mcu, r/w for bp 
#define PORT_4_STSFP_STAT_ADDR		0x80008E	// r/o for mcu, r/w for bp 
#define PORT_5_STSFP_STAT_ADDR		0x8000AE	// r/o for mcu, r/w for bp 
#define PORT_6_STSFP_STAT_ADDR		0x8000CE	// r/o for mcu, r/w for bp 
#define PORT_7_STSFP_STAT_ADDR		0x8000EE	// r/o for mcu, r/w for bp 

#define PORT_1_ALM_ADDR				0x800030	// r/o for mcu, r/w for bp 
#define PORT_2_ALM_ADDR				0x800050	// r/o for mcu, r/w for bp 
#define PORT_3_ALM_ADDR				0x800070	// r/o for mcu, r/w for bp 
#define PORT_4_ALM_ADDR				0x800090	// r/o for mcu, r/w for bp 
#define PORT_5_ALM_ADDR				0x8000B0	// r/o for mcu, r/w for bp 
#define PORT_6_ALM_ADDR				0x8000D0	// r/o for mcu, r/w for bp 
#define PORT_7_ALM_ADDR				0x8000F0	// r/o for mcu, r/w for bp 

#define PORT_1_ALM_FLAG_ADDR		0x800032	// r/o for mcu, r/w for bp 
#define PORT_2_ALM_FLAG_ADDR		0x800052	// r/o for mcu, r/w for bp 
#define PORT_3_ALM_FLAG_ADDR		0x800072	// r/o for mcu, r/w for bp 
#define PORT_4_ALM_FLAG_ADDR		0x800092	// r/o for mcu, r/w for bp 
#define PORT_5_ALM_FLAG_ADDR		0x8000B2	// r/o for mcu, r/w for bp 
#define PORT_6_ALM_FLAG_ADDR		0x8000D2	// r/o for mcu, r/w for bp 
#define PORT_7_ALM_FLAG_ADDR		0x8000F2	// r/o for mcu, r/w for bp 

#define PORT_1_ALM_MASK_ADDR		0x800034	// r/o for mcu, r/w for bp 
#define PORT_2_ALM_MASK_ADDR		0x800054	// r/o for mcu, r/w for bp 
#define PORT_3_ALM_MASK_ADDR		0x800074	// r/o for mcu, r/w for bp 
#define PORT_4_ALM_MASK_ADDR		0x800094	// r/o for mcu, r/w for bp 
#define PORT_5_ALM_MASK_ADDR		0x8000B4	// r/o for mcu, r/w for bp 
#define PORT_6_ALM_MASK_ADDR		0x8000D4	// r/o for mcu, r/w for bp 
#define PORT_7_ALM_MASK_ADDR		0x8000F4	// r/o for mcu, r/w for bp 

#define PORT_1_GET_CH_NUM_ADDR		0x800036	// r/o for mcu, r/w for bp 
#define PORT_2_GET_CH_NUM_ADDR		0x800056	// r/o for mcu, r/w for bp 
#define PORT_3_GET_CH_NUM_ADDR		0x800076	// r/o for mcu, r/w for bp 
#define PORT_4_GET_CH_NUM_ADDR		0x800096	// r/o for mcu, r/w for bp 
#define PORT_5_GET_CH_NUM_ADDR		0x8000B6	// r/o for mcu, r/w for bp 
#define PORT_6_GET_CH_NUM_ADDR		0x8000D6	// r/o for mcu, r/w for bp 
#define PORT_7_GET_CH_NUM_ADDR		0x8000F6	// r/o for mcu, r/w for bp 

#define PORT_1_SET_CH_NUM_ADDR		0x800038	// r/w for mcu, r/o for bp 
#define PORT_2_SET_CH_NUM_ADDR		0x800058	// r/w for mcu, r/o for bp 
#define PORT_3_SET_CH_NUM_ADDR		0x800078	// r/w for mcu, r/o for bp 
#define PORT_4_SET_CH_NUM_ADDR		0x800098	// r/w for mcu, r/o for bp 
#define PORT_5_SET_CH_NUM_ADDR		0x8000B8	// r/w for mcu, r/o for bp 
#define PORT_6_SET_CH_NUM_ADDR		0x8000D8	// r/w for mcu, r/o for bp 
#define PORT_7_SET_CH_NUM_ADDR		0x8000F8	// r/w for mcu, r/o for bp 


/****************************
 * Q/SFP Information related registers
 ****************************/

#define SFP_P1_VENDOR1_ADDR			0x800400	// r/o for mcu, r/w for bp 
#define SFP_P2_VENDOR1_ADDR			0x800440	// r/o for mcu, r/w for bp 
#define SFP_P3_VENDOR1_ADDR			0x800480	// r/o for mcu, r/w for bp 
#define SFP_P4_VENDOR1_ADDR			0x8004C0	// r/o for mcu, r/w for bp 
#define SFP_P5_VENDOR1_ADDR			0x800500	// r/o for mcu, r/w for bp 
#define SFP_P6_VENDOR1_ADDR			0x800540	// r/o for mcu, r/w for bp 
#define SFP_P7_VENDOR1_ADDR			0x800580	// r/o for mcu, r/w for bp 

#define SFP_P1_VENDOR2_ADDR			0x800402	// r/o for mcu, r/w for bp 
#define SFP_P2_VENDOR2_ADDR			0x800442	// r/o for mcu, r/w for bp 
#define SFP_P3_VENDOR2_ADDR			0x800482	// r/o for mcu, r/w for bp 
#define SFP_P4_VENDOR2_ADDR			0x8004C2	// r/o for mcu, r/w for bp 
#define SFP_P5_VENDOR2_ADDR			0x800502	// r/o for mcu, r/w for bp 
#define SFP_P6_VENDOR2_ADDR			0x800542	// r/o for mcu, r/w for bp 
#define SFP_P7_VENDOR2_ADDR			0x800582	// r/o for mcu, r/w for bp 

#define SFP_P1_VENDOR3_ADDR			0x800404	// r/o for mcu, r/w for bp 
#define SFP_P2_VENDOR3_ADDR			0x800444	// r/o for mcu, r/w for bp 
#define SFP_P3_VENDOR3_ADDR			0x800484	// r/o for mcu, r/w for bp 
#define SFP_P4_VENDOR3_ADDR			0x8004C4	// r/o for mcu, r/w for bp 
#define SFP_P5_VENDOR3_ADDR			0x800504	// r/o for mcu, r/w for bp 
#define SFP_P6_VENDOR3_ADDR			0x800544	// r/o for mcu, r/w for bp 
#define SFP_P7_VENDOR3_ADDR			0x800584	// r/o for mcu, r/w for bp 

#define SFP_P1_VENDOR4_ADDR			0x800406	// r/o for mcu, r/w for bp 
#define SFP_P2_VENDOR4_ADDR			0x800446	// r/o for mcu, r/w for bp 
#define SFP_P3_VENDOR4_ADDR			0x800486	// r/o for mcu, r/w for bp 
#define SFP_P4_VENDOR4_ADDR			0x8004C6	// r/o for mcu, r/w for bp 
#define SFP_P5_VENDOR4_ADDR			0x800506	// r/o for mcu, r/w for bp 
#define SFP_P6_VENDOR4_ADDR			0x800546	// r/o for mcu, r/w for bp 
#define SFP_P7_VENDOR4_ADDR			0x800586	// r/o for mcu, r/w for bp 

#define SFP_P1_VENDOR5_ADDR			0x800408	// r/o for mcu, r/w for bp 
#define SFP_P2_VENDOR5_ADDR			0x800448	// r/o for mcu, r/w for bp 
#define SFP_P3_VENDOR5_ADDR			0x800488	// r/o for mcu, r/w for bp 
#define SFP_P4_VENDOR5_ADDR			0x8004C8	// r/o for mcu, r/w for bp 
#define SFP_P5_VENDOR5_ADDR			0x800508	// r/o for mcu, r/w for bp 
#define SFP_P6_VENDOR5_ADDR			0x800548	// r/o for mcu, r/w for bp 
#define SFP_P7_VENDOR5_ADDR			0x800588	// r/o for mcu, r/w for bp 

#define SFP_P1_VENDOR6_ADDR			0x80040A	// r/o for mcu, r/w for bp 
#define SFP_P2_VENDOR6_ADDR			0x80044A	// r/o for mcu, r/w for bp 
#define SFP_P3_VENDOR6_ADDR			0x80048A	// r/o for mcu, r/w for bp 
#define SFP_P4_VENDOR6_ADDR			0x8004CA	// r/o for mcu, r/w for bp 
#define SFP_P5_VENDOR6_ADDR			0x80050A	// r/o for mcu, r/w for bp 
#define SFP_P6_VENDOR6_ADDR			0x80054A	// r/o for mcu, r/w for bp 
#define SFP_P7_VENDOR6_ADDR			0x80058A	// r/o for mcu, r/w for bp 

#define SFP_P1_VENDOR7_ADDR			0x80040C	// r/o for mcu, r/w for bp 
#define SFP_P2_VENDOR7_ADDR			0x80044C	// r/o for mcu, r/w for bp 
#define SFP_P3_VENDOR7_ADDR			0x80048C	// r/o for mcu, r/w for bp 
#define SFP_P4_VENDOR7_ADDR			0x8004CC	// r/o for mcu, r/w for bp 
#define SFP_P5_VENDOR7_ADDR			0x80050C	// r/o for mcu, r/w for bp 
#define SFP_P6_VENDOR7_ADDR			0x80054C	// r/o for mcu, r/w for bp 
#define SFP_P7_VENDOR7_ADDR			0x80058C	// r/o for mcu, r/w for bp 

#define SFP_P1_VENDOR8_ADDR			0x80040E	// r/o for mcu, r/w for bp 
#define SFP_P2_VENDOR8_ADDR			0x80044E	// r/o for mcu, r/w for bp 
#define SFP_P3_VENDOR8_ADDR			0x80048E	// r/o for mcu, r/w for bp 
#define SFP_P4_VENDOR8_ADDR			0x8004CE	// r/o for mcu, r/w for bp 
#define SFP_P5_VENDOR8_ADDR			0x80050E	// r/o for mcu, r/w for bp 
#define SFP_P6_VENDOR8_ADDR			0x80054E	// r/o for mcu, r/w for bp 
#define SFP_P7_VENDOR8_ADDR			0x80058E	// r/o for mcu, r/w for bp 

#define SFP_P1_PN1_ADDR				0x800410	// r/o for mcu, r/w for bp 
#define SFP_P2_PN1_ADDR				0x800450	// r/o for mcu, r/w for bp 
#define SFP_P3_PN1_ADDR				0x800490	// r/o for mcu, r/w for bp 
#define SFP_P4_PN1_ADDR				0x8004D0	// r/o for mcu, r/w for bp 
#define SFP_P5_PN1_ADDR				0x800510	// r/o for mcu, r/w for bp 
#define SFP_P6_PN1_ADDR				0x800550	// r/o for mcu, r/w for bp 
#define SFP_P7_PN1_ADDR				0x800590	// r/o for mcu, r/w for bp 

#define SFP_P1_PN2_ADDR				0x800412	// r/o for mcu, r/w for bp 
#define SFP_P2_PN2_ADDR				0x800452	// r/o for mcu, r/w for bp 
#define SFP_P3_PN2_ADDR				0x800492	// r/o for mcu, r/w for bp 
#define SFP_P4_PN2_ADDR				0x8004D2	// r/o for mcu, r/w for bp 
#define SFP_P5_PN2_ADDR				0x800512	// r/o for mcu, r/w for bp 
#define SFP_P6_PN2_ADDR				0x800552	// r/o for mcu, r/w for bp 
#define SFP_P7_PN2_ADDR				0x800592	// r/o for mcu, r/w for bp 

#define SFP_P1_PN3_ADDR				0x800414	// r/o for mcu, r/w for bp 
#define SFP_P2_PN3_ADDR				0x800454	// r/o for mcu, r/w for bp 
#define SFP_P3_PN3_ADDR				0x800494	// r/o for mcu, r/w for bp 
#define SFP_P4_PN3_ADDR				0x8004D4	// r/o for mcu, r/w for bp 
#define SFP_P5_PN3_ADDR				0x800514	// r/o for mcu, r/w for bp 
#define SFP_P6_PN3_ADDR				0x800554	// r/o for mcu, r/w for bp 
#define SFP_P7_PN3_ADDR				0x800594	// r/o for mcu, r/w for bp 

#define SFP_P1_PN4_ADDR				0x800416	// r/o for mcu, r/w for bp 
#define SFP_P2_PN4_ADDR				0x800456	// r/o for mcu, r/w for bp 
#define SFP_P3_PN4_ADDR				0x800496	// r/o for mcu, r/w for bp 
#define SFP_P4_PN4_ADDR				0x8004D6	// r/o for mcu, r/w for bp 
#define SFP_P5_PN4_ADDR				0x800516	// r/o for mcu, r/w for bp 
#define SFP_P6_PN4_ADDR				0x800556	// r/o for mcu, r/w for bp 
#define SFP_P7_PN4_ADDR				0x800596	// r/o for mcu, r/w for bp 

#define SFP_P1_PN5_ADDR				0x800418	// r/o for mcu, r/w for bp 
#define SFP_P2_PN5_ADDR				0x800458	// r/o for mcu, r/w for bp 
#define SFP_P3_PN5_ADDR				0x800498	// r/o for mcu, r/w for bp 
#define SFP_P4_PN5_ADDR				0x8004D8	// r/o for mcu, r/w for bp 
#define SFP_P5_PN5_ADDR				0x800518	// r/o for mcu, r/w for bp 
#define SFP_P6_PN5_ADDR				0x800558	// r/o for mcu, r/w for bp 
#define SFP_P7_PN5_ADDR				0x800598	// r/o for mcu, r/w for bp 

#define SFP_P1_PN6_ADDR				0x80041A	// r/o for mcu, r/w for bp 
#define SFP_P2_PN6_ADDR				0x80045A	// r/o for mcu, r/w for bp 
#define SFP_P3_PN6_ADDR				0x80049A	// r/o for mcu, r/w for bp 
#define SFP_P4_PN6_ADDR				0x8004DA	// r/o for mcu, r/w for bp 
#define SFP_P5_PN6_ADDR				0x80051A	// r/o for mcu, r/w for bp 
#define SFP_P6_PN6_ADDR				0x80055A	// r/o for mcu, r/w for bp 
#define SFP_P7_PN6_ADDR				0x80059A	// r/o for mcu, r/w for bp 

#define SFP_P1_PN7_ADDR				0x80041C	// r/o for mcu, r/w for bp 
#define SFP_P2_PN7_ADDR				0x80045C	// r/o for mcu, r/w for bp 
#define SFP_P3_PN7_ADDR				0x80049C	// r/o for mcu, r/w for bp 
#define SFP_P4_PN7_ADDR				0x8004DC	// r/o for mcu, r/w for bp 
#define SFP_P5_PN7_ADDR				0x80051C	// r/o for mcu, r/w for bp 
#define SFP_P6_PN7_ADDR				0x80055C	// r/o for mcu, r/w for bp 
#define SFP_P7_PN7_ADDR				0x80059C	// r/o for mcu, r/w for bp 

#define SFP_P1_PN8_ADDR				0x80041E	// r/o for mcu, r/w for bp 
#define SFP_P2_PN8_ADDR				0x80045E	// r/o for mcu, r/w for bp 
#define SFP_P3_PN8_ADDR				0x80049E	// r/o for mcu, r/w for bp 
#define SFP_P4_PN8_ADDR				0x8004DE	// r/o for mcu, r/w for bp 
#define SFP_P5_PN8_ADDR				0x80051E	// r/o for mcu, r/w for bp 
#define SFP_P6_PN8_ADDR				0x80055E	// r/o for mcu, r/w for bp 
#define SFP_P7_PN8_ADDR				0x80059E	// r/o for mcu, r/w for bp 

#define SFP_P1_SN1_ADDR				0x800420	// r/o for mcu, r/w for bp 
#define SFP_P2_SN1_ADDR				0x800460	// r/o for mcu, r/w for bp 
#define SFP_P3_SN1_ADDR				0x8004A0	// r/o for mcu, r/w for bp 
#define SFP_P4_SN1_ADDR				0x8004E0	// r/o for mcu, r/w for bp 
#define SFP_P5_SN1_ADDR				0x800520	// r/o for mcu, r/w for bp 
#define SFP_P6_SN1_ADDR				0x800560	// r/o for mcu, r/w for bp 
#define SFP_P7_SN1_ADDR				0x8005A0	// r/o for mcu, r/w for bp 

#define SFP_P1_SN2_ADDR				0x800422	// r/o for mcu, r/w for bp 
#define SFP_P2_SN2_ADDR				0x800462	// r/o for mcu, r/w for bp 
#define SFP_P3_SN2_ADDR				0x8004A2	// r/o for mcu, r/w for bp 
#define SFP_P4_SN2_ADDR				0x8004E2	// r/o for mcu, r/w for bp 
#define SFP_P5_SN2_ADDR				0x800522	// r/o for mcu, r/w for bp 
#define SFP_P6_SN2_ADDR				0x800562	// r/o for mcu, r/w for bp 
#define SFP_P7_SN2_ADDR				0x8005A2	// r/o for mcu, r/w for bp 

#define SFP_P1_SN3_ADDR				0x800424	// r/o for mcu, r/w for bp 
#define SFP_P2_SN3_ADDR				0x800464	// r/o for mcu, r/w for bp 
#define SFP_P3_SN3_ADDR				0x8004A4	// r/o for mcu, r/w for bp 
#define SFP_P4_SN3_ADDR				0x8004E4	// r/o for mcu, r/w for bp 
#define SFP_P5_SN3_ADDR				0x800524	// r/o for mcu, r/w for bp 
#define SFP_P6_SN3_ADDR				0x800564	// r/o for mcu, r/w for bp 
#define SFP_P7_SN3_ADDR				0x8005A4	// r/o for mcu, r/w for bp 

#define SFP_P1_SN4_ADDR				0x800426	// r/o for mcu, r/w for bp 
#define SFP_P2_SN4_ADDR				0x800466	// r/o for mcu, r/w for bp 
#define SFP_P3_SN4_ADDR				0x8004A6	// r/o for mcu, r/w for bp 
#define SFP_P4_SN4_ADDR				0x8004E6	// r/o for mcu, r/w for bp 
#define SFP_P5_SN4_ADDR				0x800526	// r/o for mcu, r/w for bp 
#define SFP_P6_SN4_ADDR				0x800566	// r/o for mcu, r/w for bp 
#define SFP_P7_SN4_ADDR				0x8005A6	// r/o for mcu, r/w for bp 

#define SFP_P1_SN5_ADDR				0x800428	// r/o for mcu, r/w for bp 
#define SFP_P2_SN5_ADDR				0x800468	// r/o for mcu, r/w for bp 
#define SFP_P3_SN5_ADDR				0x8004A8	// r/o for mcu, r/w for bp 
#define SFP_P4_SN5_ADDR				0x8004E8	// r/o for mcu, r/w for bp 
#define SFP_P5_SN5_ADDR				0x800528	// r/o for mcu, r/w for bp 
#define SFP_P6_SN5_ADDR				0x800568	// r/o for mcu, r/w for bp 
#define SFP_P7_SN5_ADDR				0x8005A8	// r/o for mcu, r/w for bp 

#define SFP_P1_SN6_ADDR				0x80042A	// r/o for mcu, r/w for bp 
#define SFP_P2_SN6_ADDR				0x80046A	// r/o for mcu, r/w for bp 
#define SFP_P3_SN6_ADDR				0x8004AA	// r/o for mcu, r/w for bp 
#define SFP_P4_SN6_ADDR				0x8004EA	// r/o for mcu, r/w for bp 
#define SFP_P5_SN6_ADDR				0x80052A	// r/o for mcu, r/w for bp 
#define SFP_P6_SN6_ADDR				0x80056A	// r/o for mcu, r/w for bp 
#define SFP_P7_SN6_ADDR				0x8005AA	// r/o for mcu, r/w for bp 

#define SFP_P1_SN7_ADDR				0x80042C	// r/o for mcu, r/w for bp 
#define SFP_P2_SN7_ADDR				0x80046C	// r/o for mcu, r/w for bp 
#define SFP_P3_SN7_ADDR				0x8004AC	// r/o for mcu, r/w for bp 
#define SFP_P4_SN7_ADDR				0x8004EC	// r/o for mcu, r/w for bp 
#define SFP_P5_SN7_ADDR				0x80052C	// r/o for mcu, r/w for bp 
#define SFP_P6_SN7_ADDR				0x80056C	// r/o for mcu, r/w for bp 
#define SFP_P7_SN7_ADDR				0x8005AC	// r/o for mcu, r/w for bp 

#define SFP_P1_SN8_ADDR				0x80042E	// r/o for mcu, r/w for bp 
#define SFP_P2_SN8_ADDR				0x80046E	// r/o for mcu, r/w for bp 
#define SFP_P3_SN8_ADDR				0x8004AE	// r/o for mcu, r/w for bp 
#define SFP_P4_SN8_ADDR				0x8004EE	// r/o for mcu, r/w for bp 
#define SFP_P5_SN8_ADDR				0x80052E	// r/o for mcu, r/w for bp 
#define SFP_P6_SN8_ADDR				0x80056E	// r/o for mcu, r/w for bp 
#define SFP_P7_SN8_ADDR				0x8005AE	// r/o for mcu, r/w for bp 

#define SFP_P1_TEMP_ADDR			0x800430	// r/o for mcu, r/w for bp 
#define SFP_P2_TEMP_ADDR			0x800470	// r/o for mcu, r/w for bp 
#define SFP_P3_TEMP_ADDR			0x8004B0	// r/o for mcu, r/w for bp 
#define SFP_P4_TEMP_ADDR			0x8004F0	// r/o for mcu, r/w for bp 
#define SFP_P5_TEMP_ADDR			0x800530	// r/o for mcu, r/w for bp 
#define SFP_P6_TEMP_ADDR			0x800570	// r/o for mcu, r/w for bp 
#define SFP_P7_TEMP_ADDR			0x8005B0	// r/o for mcu, r/w for bp 

#define SFP_P1_RATE_ADDR			0x800432	// r/o for mcu, r/w for bp 
#define SFP_P2_RATE_ADDR			0x800472	// r/o for mcu, r/w for bp 
#define SFP_P3_RATE_ADDR			0x8004B2	// r/o for mcu, r/w for bp 
#define SFP_P4_RATE_ADDR			0x8004F2	// r/o for mcu, r/w for bp 
#define SFP_P5_RATE_ADDR			0x800532	// r/o for mcu, r/w for bp 
#define SFP_P6_RATE_ADDR			0x800572	// r/o for mcu, r/w for bp 
#define SFP_P7_RATE_ADDR			0x8005B2	// r/o for mcu, r/w for bp 

#define SFP_P1_VOLT_ADDR			0x800434	// r/o for mcu, r/w for bp 
#define SFP_P2_VOLT_ADDR			0x800474	// r/o for mcu, r/w for bp 
#define SFP_P3_VOLT_ADDR			0x8004B4	// r/o for mcu, r/w for bp 
#define SFP_P4_VOLT_ADDR			0x8004F4	// r/o for mcu, r/w for bp 
#define SFP_P5_VOLT_ADDR			0x800534	// r/o for mcu, r/w for bp 
#define SFP_P6_VOLT_ADDR			0x800574	// r/o for mcu, r/w for bp 
#define SFP_P7_VOLT_ADDR			0x8005B4	// r/o for mcu, r/w for bp 

#define SFP_P1_TX_BIAS_ADDR			0x800436	// r/o for mcu, r/w for bp 
#define SFP_P2_TX_BIAS_ADDR			0x800476	// r/o for mcu, r/w for bp 
#define SFP_P3_TX_BIAS_ADDR			0x8004B6	// r/o for mcu, r/w for bp 
#define SFP_P4_TX_BIAS_ADDR			0x8004F6	// r/o for mcu, r/w for bp 
#define SFP_P5_TX_BIAS_ADDR			0x800536	// r/o for mcu, r/w for bp 
#define SFP_P6_TX_BIAS_ADDR			0x800576	// r/o for mcu, r/w for bp 
#define SFP_P7_TX_BIAS_ADDR			0x8005B6	// r/o for mcu, r/w for bp 

#define SFP_P1_LTEMP_ADDR			0x800438	// r/o for mcu, r/w for bp 
#define SFP_P2_LTEMP_ADDR			0x800478	// r/o for mcu, r/w for bp 
#define SFP_P3_LTEMP_ADDR			0x8004B8	// r/o for mcu, r/w for bp 
#define SFP_P4_LTEMP_ADDR			0x8004F8	// r/o for mcu, r/w for bp 
#define SFP_P5_LTEMP_ADDR			0x800538	// r/o for mcu, r/w for bp 
#define SFP_P6_LTEMP_ADDR			0x800578	// r/o for mcu, r/w for bp 
#define SFP_P7_LTEMP_ADDR			0x8005B8	// r/o for mcu, r/w for bp 

#define SFP_P1_TCURR_ADDR			0x80043A	// r/o for mcu, r/w for bp 
#define SFP_P2_TCURR_ADDR			0x80047A	// r/o for mcu, r/w for bp 
#define SFP_P3_TCURR_ADDR			0x8004BA	// r/o for mcu, r/w for bp 
#define SFP_P4_TCURR_ADDR			0x8004FA	// r/o for mcu, r/w for bp 
#define SFP_P5_TCURR_ADDR			0x80053A	// r/o for mcu, r/w for bp 
#define SFP_P6_TCURR_ADDR			0x80057A	// r/o for mcu, r/w for bp 
#define SFP_P7_TCURR_ADDR			0x8005BA	// r/o for mcu, r/w for bp 


/****************************
 * Performance related registers
 ****************************/

#define PM_COUNT_CLEAR_ADDR			0x800100	// r/w for mcu, r/o for bp, read clear

#define PM_P1_TX_BYTE1_ADDR			0x800700	// r/o for mcu, r/w for bp 
#define PM_P2_TX_BYTE1_ADDR			0x800750	// r/o for mcu, r/w for bp 
#define PM_P3_TX_BYTE1_ADDR			0x8007A0	// r/o for mcu, r/w for bp 
#define PM_P4_TX_BYTE1_ADDR			0x8007F0	// r/o for mcu, r/w for bp 
#define PM_P5_TX_BYTE1_ADDR			0x800840	// r/o for mcu, r/w for bp 
#define PM_P6_TX_BYTE1_ADDR			0x800890	// r/o for mcu, r/w for bp 
#define PM_P7_TX_BYTE1_ADDR			0x8008E0	// r/o for mcu, r/w for bp 

#define PM_P1_TX_BYTE2_ADDR			0x800702	// r/o for mcu, r/w for bp 
#define PM_P2_TX_BYTE2_ADDR			0x800752	// r/o for mcu, r/w for bp 
#define PM_P3_TX_BYTE2_ADDR			0x8007A2	// r/o for mcu, r/w for bp 
#define PM_P4_TX_BYTE2_ADDR			0x8007F2	// r/o for mcu, r/w for bp 
#define PM_P5_TX_BYTE2_ADDR			0x800842	// r/o for mcu, r/w for bp 
#define PM_P6_TX_BYTE2_ADDR			0x800892	// r/o for mcu, r/w for bp 
#define PM_P7_TX_BYTE2_ADDR			0x8008E2	// r/o for mcu, r/w for bp 

#define PM_P1_TX_BYTE3_ADDR			0x800704	// r/o for mcu, r/w for bp 
#define PM_P2_TX_BYTE3_ADDR			0x800754	// r/o for mcu, r/w for bp 
#define PM_P3_TX_BYTE3_ADDR			0x8007A4	// r/o for mcu, r/w for bp 
#define PM_P4_TX_BYTE3_ADDR			0x8007F4	// r/o for mcu, r/w for bp 
#define PM_P5_TX_BYTE3_ADDR			0x800844	// r/o for mcu, r/w for bp 
#define PM_P6_TX_BYTE3_ADDR			0x800894	// r/o for mcu, r/w for bp 
#define PM_P7_TX_BYTE3_ADDR			0x8008E4	// r/o for mcu, r/w for bp 

#define PM_P1_TX_BYTE4_ADDR			0x800706	// r/o for mcu, r/w for bp 
#define PM_P2_TX_BYTE4_ADDR			0x800756	// r/o for mcu, r/w for bp 
#define PM_P3_TX_BYTE4_ADDR			0x8007A6	// r/o for mcu, r/w for bp 
#define PM_P4_TX_BYTE4_ADDR			0x8007F6	// r/o for mcu, r/w for bp 
#define PM_P5_TX_BYTE4_ADDR			0x800846	// r/o for mcu, r/w for bp 
#define PM_P6_TX_BYTE4_ADDR			0x800896	// r/o for mcu, r/w for bp 
#define PM_P7_TX_BYTE4_ADDR			0x8008E6	// r/o for mcu, r/w for bp 

#define PM_P1_RX_BYTE1_ADDR			0x800708	// r/o for mcu, r/w for bp 
#define PM_P2_RX_BYTE1_ADDR			0x800758	// r/o for mcu, r/w for bp 
#define PM_P3_RX_BYTE1_ADDR			0x8007A8	// r/o for mcu, r/w for bp 
#define PM_P4_RX_BYTE1_ADDR			0x8007F8	// r/o for mcu, r/w for bp 
#define PM_P5_RX_BYTE1_ADDR			0x800848	// r/o for mcu, r/w for bp 
#define PM_P6_RX_BYTE1_ADDR			0x800898	// r/o for mcu, r/w for bp 
#define PM_P7_RX_BYTE1_ADDR			0x8008E8	// r/o for mcu, r/w for bp 

#define PM_P1_RX_BYTE2_ADDR			0x80070A	// r/o for mcu, r/w for bp 
#define PM_P2_RX_BYTE2_ADDR			0x80075A	// r/o for mcu, r/w for bp 
#define PM_P3_RX_BYTE2_ADDR			0x8007AA	// r/o for mcu, r/w for bp 
#define PM_P4_RX_BYTE2_ADDR			0x8007FA	// r/o for mcu, r/w for bp 
#define PM_P5_RX_BYTE2_ADDR			0x80084A	// r/o for mcu, r/w for bp 
#define PM_P6_RX_BYTE2_ADDR			0x80089A	// r/o for mcu, r/w for bp 
#define PM_P7_RX_BYTE2_ADDR			0x8008EA	// r/o for mcu, r/w for bp 

#define PM_P1_RX_BYTE3_ADDR			0x80070C	// r/o for mcu, r/w for bp 
#define PM_P2_RX_BYTE3_ADDR			0x80075C	// r/o for mcu, r/w for bp 
#define PM_P3_RX_BYTE3_ADDR			0x8007AC	// r/o for mcu, r/w for bp 
#define PM_P4_RX_BYTE3_ADDR			0x8007FC	// r/o for mcu, r/w for bp 
#define PM_P5_RX_BYTE3_ADDR			0x80084C	// r/o for mcu, r/w for bp 
#define PM_P6_RX_BYTE3_ADDR			0x80089C	// r/o for mcu, r/w for bp 
#define PM_P7_RX_BYTE3_ADDR			0x8008EC	// r/o for mcu, r/w for bp 

#define PM_P1_RX_BYTE4_ADDR			0x80070E	// r/o for mcu, r/w for bp 
#define PM_P2_RX_BYTE4_ADDR			0x80075E	// r/o for mcu, r/w for bp 
#define PM_P3_RX_BYTE4_ADDR			0x8007AE	// r/o for mcu, r/w for bp 
#define PM_P4_RX_BYTE4_ADDR			0x8007FE	// r/o for mcu, r/w for bp 
#define PM_P5_RX_BYTE4_ADDR			0x80084E	// r/o for mcu, r/w for bp 
#define PM_P6_RX_BYTE4_ADDR			0x80089E	// r/o for mcu, r/w for bp 
#define PM_P7_RX_BYTE4_ADDR			0x8008EE	// r/o for mcu, r/w for bp 

#define PM_P1_TX_FRAME1_ADDR		0x800710	// r/o for mcu, r/w for bp 
#define PM_P2_TX_FRAME1_ADDR		0x800760	// r/o for mcu, r/w for bp 
#define PM_P3_TX_FRAME1_ADDR		0x8007B0	// r/o for mcu, r/w for bp 
#define PM_P4_TX_FRAME1_ADDR		0x800800	// r/o for mcu, r/w for bp 
#define PM_P5_TX_FRAME1_ADDR		0x800850	// r/o for mcu, r/w for bp 
#define PM_P6_TX_FRAME1_ADDR		0x8008A0	// r/o for mcu, r/w for bp 
#define PM_P7_TX_FRAME1_ADDR		0x8008F0	// r/o for mcu, r/w for bp 

#define PM_P1_TX_FRAME2_ADDR		0x800712	// r/o for mcu, r/w for bp 
#define PM_P2_TX_FRAME2_ADDR		0x800762	// r/o for mcu, r/w for bp 
#define PM_P3_TX_FRAME2_ADDR		0x8007B2	// r/o for mcu, r/w for bp 
#define PM_P4_TX_FRAME2_ADDR		0x800802	// r/o for mcu, r/w for bp 
#define PM_P5_TX_FRAME2_ADDR		0x800852	// r/o for mcu, r/w for bp 
#define PM_P6_TX_FRAME2_ADDR		0x8008A2	// r/o for mcu, r/w for bp 
#define PM_P7_TX_FRAME2_ADDR		0x8008F2	// r/o for mcu, r/w for bp 

#define PM_P1_TX_FRAME3_ADDR		0x800714	// r/o for mcu, r/w for bp 
#define PM_P2_TX_FRAME3_ADDR		0x800764	// r/o for mcu, r/w for bp 
#define PM_P3_TX_FRAME3_ADDR		0x8007B4	// r/o for mcu, r/w for bp 
#define PM_P4_TX_FRAME3_ADDR		0x800804	// r/o for mcu, r/w for bp 
#define PM_P5_TX_FRAME3_ADDR		0x800854	// r/o for mcu, r/w for bp 
#define PM_P6_TX_FRAME3_ADDR		0x8008A4	// r/o for mcu, r/w for bp 
#define PM_P7_TX_FRAME3_ADDR		0x8008F4	// r/o for mcu, r/w for bp 

#define PM_P1_TX_FRAME4_ADDR		0x800716	// r/o for mcu, r/w for bp 
#define PM_P2_TX_FRAME4_ADDR		0x800766	// r/o for mcu, r/w for bp 
#define PM_P3_TX_FRAME4_ADDR		0x8007B6	// r/o for mcu, r/w for bp 
#define PM_P4_TX_FRAME4_ADDR		0x800806	// r/o for mcu, r/w for bp 
#define PM_P5_TX_FRAME4_ADDR		0x800856	// r/o for mcu, r/w for bp 
#define PM_P6_TX_FRAME4_ADDR		0x8008A6	// r/o for mcu, r/w for bp 
#define PM_P7_TX_FRAME4_ADDR		0x8008F6	// r/o for mcu, r/w for bp 

#define PM_P1_RX_FRAME1_ADDR		0x800718	// r/o for mcu, r/w for bp 
#define PM_P2_RX_FRAME1_ADDR		0x800768	// r/o for mcu, r/w for bp 
#define PM_P3_RX_FRAME1_ADDR		0x8007B8	// r/o for mcu, r/w for bp 
#define PM_P4_RX_FRAME1_ADDR		0x800808	// r/o for mcu, r/w for bp 
#define PM_P5_RX_FRAME1_ADDR		0x800858	// r/o for mcu, r/w for bp 
#define PM_P6_RX_FRAME1_ADDR		0x8008A8	// r/o for mcu, r/w for bp 
#define PM_P7_RX_FRAME1_ADDR		0x8008F8	// r/o for mcu, r/w for bp 

#define PM_P1_RX_FRAME2_ADDR		0x80071A	// r/o for mcu, r/w for bp 
#define PM_P2_RX_FRAME2_ADDR		0x80076A	// r/o for mcu, r/w for bp 
#define PM_P3_RX_FRAME2_ADDR		0x8007BA	// r/o for mcu, r/w for bp 
#define PM_P4_RX_FRAME2_ADDR		0x80080A	// r/o for mcu, r/w for bp 
#define PM_P5_RX_FRAME2_ADDR		0x80085A	// r/o for mcu, r/w for bp 
#define PM_P6_RX_FRAME2_ADDR		0x8008AA	// r/o for mcu, r/w for bp 
#define PM_P7_RX_FRAME2_ADDR		0x8008FA	// r/o for mcu, r/w for bp 

#define PM_P1_RX_FRAME3_ADDR		0x80071C	// r/o for mcu, r/w for bp 
#define PM_P2_RX_FRAME3_ADDR		0x80076C	// r/o for mcu, r/w for bp 
#define PM_P3_RX_FRAME3_ADDR		0x8007BC	// r/o for mcu, r/w for bp 
#define PM_P4_RX_FRAME3_ADDR		0x80080C	// r/o for mcu, r/w for bp 
#define PM_P5_RX_FRAME3_ADDR		0x80085C	// r/o for mcu, r/w for bp 
#define PM_P6_RX_FRAME3_ADDR		0x8008AC	// r/o for mcu, r/w for bp 
#define PM_P7_RX_FRAME3_ADDR		0x8008FC	// r/o for mcu, r/w for bp 

#define PM_P1_RX_FRAME4_ADDR		0x80071E	// r/o for mcu, r/w for bp 
#define PM_P2_RX_FRAME4_ADDR		0x80076E	// r/o for mcu, r/w for bp 
#define PM_P3_RX_FRAME4_ADDR		0x8007BE	// r/o for mcu, r/w for bp 
#define PM_P4_RX_FRAME4_ADDR		0x80080E	// r/o for mcu, r/w for bp 
#define PM_P5_RX_FRAME4_ADDR		0x80085E	// r/o for mcu, r/w for bp 
#define PM_P6_RX_FRAME4_ADDR		0x8008AE	// r/o for mcu, r/w for bp 
#define PM_P7_RX_FRAME4_ADDR		0x8008FE	// r/o for mcu, r/w for bp 

#define PM_P1_FCS1_ADDR				0x800720	// r/o for mcu, r/w for bp 
#define PM_P2_FCS1_ADDR				0x800770	// r/o for mcu, r/w for bp 
#define PM_P3_FCS1_ADDR				0x8007C0	// r/o for mcu, r/w for bp 
#define PM_P4_FCS1_ADDR				0x800810	// r/o for mcu, r/w for bp 
#define PM_P5_FCS1_ADDR				0x800860	// r/o for mcu, r/w for bp 
#define PM_P6_FCS1_ADDR				0x8008B0	// r/o for mcu, r/w for bp 
#define PM_P7_FCS1_ADDR				0x800900	// r/o for mcu, r/w for bp 

#define PM_P1_FCS2_ADDR				0x800722	// r/o for mcu, r/w for bp 
#define PM_P2_FCS2_ADDR				0x800772	// r/o for mcu, r/w for bp 
#define PM_P3_FCS2_ADDR				0x8007C2	// r/o for mcu, r/w for bp 
#define PM_P4_FCS2_ADDR				0x800812	// r/o for mcu, r/w for bp 
#define PM_P5_FCS2_ADDR				0x800862	// r/o for mcu, r/w for bp 
#define PM_P6_FCS2_ADDR				0x8008B2	// r/o for mcu, r/w for bp 
#define PM_P7_FCS2_ADDR				0x800902	// r/o for mcu, r/w for bp 

#define PM_P1_FCS3_ADDR				0x800724	// r/o for mcu, r/w for bp 
#define PM_P2_FCS3_ADDR				0x800774	// r/o for mcu, r/w for bp 
#define PM_P3_FCS3_ADDR				0x8007C4	// r/o for mcu, r/w for bp 
#define PM_P4_FCS3_ADDR				0x800814	// r/o for mcu, r/w for bp 
#define PM_P5_FCS3_ADDR				0x800864	// r/o for mcu, r/w for bp 
#define PM_P6_FCS3_ADDR				0x8008B4	// r/o for mcu, r/w for bp 
#define PM_P7_FCS3_ADDR				0x800904	// r/o for mcu, r/w for bp 

#define PM_P1_FCS4_ADDR				0x800726	// r/o for mcu, r/w for bp 
#define PM_P2_FCS4_ADDR				0x800776	// r/o for mcu, r/w for bp 
#define PM_P3_FCS4_ADDR				0x8007C6	// r/o for mcu, r/w for bp 
#define PM_P4_FCS4_ADDR				0x800816	// r/o for mcu, r/w for bp 
#define PM_P5_FCS4_ADDR				0x800866	// r/o for mcu, r/w for bp 
#define PM_P6_FCS4_ADDR				0x8008B6	// r/o for mcu, r/w for bp 
#define PM_P7_FCS4_ADDR				0x800906	// r/o for mcu, r/w for bp 

#define PM_P1_FCS_OK1_ADDR			0x800730	// r/o for mcu, r/w for bp 
#define PM_P2_FCS_OK1_ADDR			0x800780	// r/o for mcu, r/w for bp 
#define PM_P3_FCS_OK1_ADDR			0x8007D0	// r/o for mcu, r/w for bp 
#define PM_P4_FCS_OK1_ADDR			0x800820	// r/o for mcu, r/w for bp 
#define PM_P5_FCS_OK1_ADDR			0x800870	// r/o for mcu, r/w for bp 
#define PM_P6_FCS_OK1_ADDR			0x8008C0	// r/o for mcu, r/w for bp 
#define PM_P7_FCS_OK1_ADDR			0x800910	// r/o for mcu, r/w for bp 

#define PM_P1_FCS_OK2_ADDR			0x800732	// r/o for mcu, r/w for bp 
#define PM_P2_FCS_OK2_ADDR			0x800782	// r/o for mcu, r/w for bp 
#define PM_P3_FCS_OK2_ADDR			0x8007D2	// r/o for mcu, r/w for bp 
#define PM_P4_FCS_OK2_ADDR			0x800822	// r/o for mcu, r/w for bp 
#define PM_P5_FCS_OK2_ADDR			0x800872	// r/o for mcu, r/w for bp 
#define PM_P6_FCS_OK2_ADDR			0x8008C2	// r/o for mcu, r/w for bp 
#define PM_P7_FCS_OK2_ADDR			0x800912	// r/o for mcu, r/w for bp 

#define PM_P1_FCS_OK3_ADDR			0x800734	// r/o for mcu, r/w for bp 
#define PM_P2_FCS_OK3_ADDR			0x800784	// r/o for mcu, r/w for bp 
#define PM_P3_FCS_OK3_ADDR			0x8007D4	// r/o for mcu, r/w for bp 
#define PM_P4_FCS_OK3_ADDR			0x800824	// r/o for mcu, r/w for bp 
#define PM_P5_FCS_OK3_ADDR			0x800874	// r/o for mcu, r/w for bp 
#define PM_P6_FCS_OK3_ADDR			0x8008C4	// r/o for mcu, r/w for bp 
#define PM_P7_FCS_OK3_ADDR			0x800914	// r/o for mcu, r/w for bp 

#define PM_P1_FCS_OK4_ADDR			0x800736	// r/o for mcu, r/w for bp 
#define PM_P2_FCS_OK4_ADDR			0x800786	// r/o for mcu, r/w for bp 
#define PM_P3_FCS_OK4_ADDR			0x8007D6	// r/o for mcu, r/w for bp 
#define PM_P4_FCS_OK4_ADDR			0x800826	// r/o for mcu, r/w for bp 
#define PM_P5_FCS_OK4_ADDR			0x800876	// r/o for mcu, r/w for bp 
#define PM_P6_FCS_OK4_ADDR			0x8008C6	// r/o for mcu, r/w for bp 
#define PM_P7_FCS_OK4_ADDR			0x800916	// r/o for mcu, r/w for bp 

#define PM_P1_FCS_NOK1_ADDR			0x800738	// r/o for mcu, r/w for bp 
#define PM_P2_FCS_NOK1_ADDR			0x800788	// r/o for mcu, r/w for bp 
#define PM_P3_FCS_NOK1_ADDR			0x8007D8	// r/o for mcu, r/w for bp 
#define PM_P4_FCS_NOK1_ADDR			0x800828	// r/o for mcu, r/w for bp 
#define PM_P5_FCS_NOK1_ADDR			0x800878	// r/o for mcu, r/w for bp 
#define PM_P6_FCS_NOK1_ADDR			0x8008C8	// r/o for mcu, r/w for bp 
#define PM_P7_FCS_NOK1_ADDR			0x800918	// r/o for mcu, r/w for bp 

#define PM_P1_FCS_NOK2_ADDR			0x80073A	// r/o for mcu, r/w for bp 
#define PM_P2_FCS_NOK2_ADDR			0x80078A	// r/o for mcu, r/w for bp 
#define PM_P3_FCS_NOK2_ADDR			0x8007DA	// r/o for mcu, r/w for bp 
#define PM_P4_FCS_NOK2_ADDR			0x80082A	// r/o for mcu, r/w for bp 
#define PM_P5_FCS_NOK2_ADDR			0x80087A	// r/o for mcu, r/w for bp 
#define PM_P6_FCS_NOK2_ADDR			0x8008CA	// r/o for mcu, r/w for bp 
#define PM_P7_FCS_NOK2_ADDR			0x80091A	// r/o for mcu, r/w for bp 

#define PM_P1_FCS_NOK3_ADDR			0x80073C	// r/o for mcu, r/w for bp 
#define PM_P2_FCS_NOK3_ADDR			0x80078C	// r/o for mcu, r/w for bp 
#define PM_P3_FCS_NOK3_ADDR			0x8007DC	// r/o for mcu, r/w for bp 
#define PM_P4_FCS_NOK3_ADDR			0x80082C	// r/o for mcu, r/w for bp 
#define PM_P5_FCS_NOK3_ADDR			0x80087C	// r/o for mcu, r/w for bp 
#define PM_P6_FCS_NOK3_ADDR			0x8008CC	// r/o for mcu, r/w for bp 
#define PM_P7_FCS_NOK3_ADDR			0x80091C	// r/o for mcu, r/w for bp 

#define PM_P1_FCS_NOK4_ADDR			0x80073E	// r/o for mcu, r/w for bp 
#define PM_P2_FCS_NOK4_ADDR			0x80078E	// r/o for mcu, r/w for bp 
#define PM_P3_FCS_NOK4_ADDR			0x8007DE	// r/o for mcu, r/w for bp 
#define PM_P4_FCS_NOK4_ADDR			0x80082E	// r/o for mcu, r/w for bp 
#define PM_P5_FCS_NOK4_ADDR			0x80087E	// r/o for mcu, r/w for bp 
#define PM_P6_FCS_NOK4_ADDR			0x8008CE	// r/o for mcu, r/w for bp 
#define PM_P7_FCS_NOK4_ADDR			0x80091E	// r/o for mcu, r/w for bp 


/****************************
 * DCO related registers
 ****************************/

//TBD


/****************************
 * CLEI/USI Information related registers
 ****************************/

#define PORT1_CLEI1_ADDR 			0x800B00	// r/o for mcu, r/w for bp 
#define PORT2_CLEI1_ADDR 			0x800B10	// r/o for mcu, r/w for bp 
#define PORT3_CLEI1_ADDR 			0x800B20	// r/o for mcu, r/w for bp 
#define PORT4_CLEI1_ADDR 			0x800B30	// r/o for mcu, r/w for bp 
#define PORT5_CLEI1_ADDR 			0x800B40	// r/o for mcu, r/w for bp 
#define PORT6_CLEI1_ADDR 			0x800B50	// r/o for mcu, r/w for bp 
#define PORT7_CLEI1_ADDR 			0x800B60	// r/o for mcu, r/w for bp 

#define PORT1_CLEI2_ADDR 			0x800B02	// r/o for mcu, r/w for bp 
#define PORT2_CLEI2_ADDR 			0x800B12	// r/o for mcu, r/w for bp 
#define PORT3_CLEI2_ADDR 			0x800B22	// r/o for mcu, r/w for bp 
#define PORT4_CLEI2_ADDR 			0x800B32	// r/o for mcu, r/w for bp 
#define PORT5_CLEI2_ADDR 			0x800B42	// r/o for mcu, r/w for bp 
#define PORT6_CLEI2_ADDR 			0x800B52	// r/o for mcu, r/w for bp 
#define PORT7_CLEI2_ADDR 			0x800B62	// r/o for mcu, r/w for bp 

#define PORT1_CLEI3_ADDR 			0x800B04	// r/o for mcu, r/w for bp 
#define PORT2_CLEI3_ADDR 			0x800B14	// r/o for mcu, r/w for bp 
#define PORT3_CLEI3_ADDR 			0x800B24	// r/o for mcu, r/w for bp 
#define PORT4_CLEI3_ADDR 			0x800B34	// r/o for mcu, r/w for bp 
#define PORT5_CLEI3_ADDR 			0x800B44	// r/o for mcu, r/w for bp 
#define PORT6_CLEI3_ADDR 			0x800B54	// r/o for mcu, r/w for bp 
#define PORT7_CLEI3_ADDR 			0x800B64	// r/o for mcu, r/w for bp 

#define PORT1_CLEI4_ADDR 			0x800B06	// r/o for mcu, r/w for bp 
#define PORT2_CLEI4_ADDR 			0x800B16	// r/o for mcu, r/w for bp 
#define PORT3_CLEI4_ADDR 			0x800B26	// r/o for mcu, r/w for bp 
#define PORT4_CLEI4_ADDR 			0x800B36	// r/o for mcu, r/w for bp 
#define PORT5_CLEI4_ADDR 			0x800B46	// r/o for mcu, r/w for bp 
#define PORT6_CLEI4_ADDR 			0x800B56	// r/o for mcu, r/w for bp 
#define PORT7_CLEI4_ADDR 			0x800B66	// r/o for mcu, r/w for bp 

#define PORT1_CLEI5_ADDR 			0x800B08	// r/o for mcu, r/w for bp 
#define PORT2_CLEI5_ADDR 			0x800B18	// r/o for mcu, r/w for bp 
#define PORT3_CLEI5_ADDR 			0x800B28	// r/o for mcu, r/w for bp 
#define PORT4_CLEI5_ADDR 			0x800B38	// r/o for mcu, r/w for bp 
#define PORT5_CLEI5_ADDR 			0x800B48	// r/o for mcu, r/w for bp 
#define PORT6_CLEI5_ADDR 			0x800B58	// r/o for mcu, r/w for bp 
#define PORT7_CLEI5_ADDR 			0x800B68	// r/o for mcu, r/w for bp 

#define PORT1_USI1_ADDR 			0x800C00	// r/o for mcu, r/w for bp 
#define PORT2_USI1_ADDR 			0x800C20	// r/o for mcu, r/w for bp 
#define PORT3_USI1_ADDR 			0x800C40	// r/o for mcu, r/w for bp 
#define PORT4_USI1_ADDR 			0x800C60	// r/o for mcu, r/w for bp 
#define PORT5_USI1_ADDR 			0x800C80	// r/o for mcu, r/w for bp 
#define PORT6_USI1_ADDR 			0x800CA0	// r/o for mcu, r/w for bp 
#define PORT7_USI1_ADDR 			0x800CC0	// r/o for mcu, r/w for bp 

#define PORT1_USI2_ADDR 			0x800C02	// r/o for mcu, r/w for bp 
#define PORT2_USI2_ADDR 			0x800C22	// r/o for mcu, r/w for bp 
#define PORT3_USI2_ADDR 			0x800C42	// r/o for mcu, r/w for bp 
#define PORT4_USI2_ADDR 			0x800C62	// r/o for mcu, r/w for bp 
#define PORT5_USI2_ADDR 			0x800C82	// r/o for mcu, r/w for bp 
#define PORT6_USI2_ADDR 			0x800CA2	// r/o for mcu, r/w for bp 
#define PORT7_USI2_ADDR 			0x800CC2	// r/o for mcu, r/w for bp 

#define PORT1_USI3_ADDR 			0x800C04	// r/o for mcu, r/w for bp 
#define PORT2_USI3_ADDR 			0x800C24	// r/o for mcu, r/w for bp 
#define PORT3_USI3_ADDR 			0x800C44	// r/o for mcu, r/w for bp 
#define PORT4_USI3_ADDR 			0x800C64	// r/o for mcu, r/w for bp 
#define PORT5_USI3_ADDR 			0x800C84	// r/o for mcu, r/w for bp 
#define PORT6_USI3_ADDR 			0x800CA4	// r/o for mcu, r/w for bp 
#define PORT7_USI3_ADDR 			0x800CC4	// r/o for mcu, r/w for bp 

#define PORT1_USI4_ADDR 			0x800C06	// r/o for mcu, r/w for bp 
#define PORT2_USI4_ADDR 			0x800C26	// r/o for mcu, r/w for bp 
#define PORT3_USI4_ADDR 			0x800C46	// r/o for mcu, r/w for bp 
#define PORT4_USI4_ADDR 			0x800C66	// r/o for mcu, r/w for bp 
#define PORT5_USI4_ADDR 			0x800C86	// r/o for mcu, r/w for bp 
#define PORT6_USI4_ADDR 			0x800CA6	// r/o for mcu, r/w for bp 
#define PORT7_USI4_ADDR 			0x800CC6	// r/o for mcu, r/w for bp 

#define PORT1_USI5_ADDR 			0x800C08	// r/o for mcu, r/w for bp 
#define PORT2_USI5_ADDR 			0x800C28	// r/o for mcu, r/w for bp 
#define PORT3_USI5_ADDR 			0x800C48	// r/o for mcu, r/w for bp 
#define PORT4_USI5_ADDR 			0x800C68	// r/o for mcu, r/w for bp 
#define PORT5_USI5_ADDR 			0x800C88	// r/o for mcu, r/w for bp 
#define PORT6_USI5_ADDR 			0x800CA8	// r/o for mcu, r/w for bp 
#define PORT7_USI5_ADDR 			0x800CC8	// r/o for mcu, r/w for bp 

#define PORT1_USI6_ADDR 			0x800C0A	// r/o for mcu, r/w for bp 
#define PORT2_USI6_ADDR 			0x800C2A	// r/o for mcu, r/w for bp 
#define PORT3_USI6_ADDR 			0x800C4A	// r/o for mcu, r/w for bp 
#define PORT4_USI6_ADDR 			0x800C6A	// r/o for mcu, r/w for bp 
#define PORT5_USI6_ADDR 			0x800C8A	// r/o for mcu, r/w for bp 
#define PORT6_USI6_ADDR 			0x800CAA	// r/o for mcu, r/w for bp 
#define PORT7_USI6_ADDR 			0x800CCA	// r/o for mcu, r/w for bp 

#define PORT1_USI7_ADDR 			0x800C0C	// r/o for mcu, r/w for bp 
#define PORT2_USI7_ADDR 			0x800C2C	// r/o for mcu, r/w for bp 
#define PORT3_USI7_ADDR 			0x800C4C	// r/o for mcu, r/w for bp 
#define PORT4_USI7_ADDR 			0x800C6C	// r/o for mcu, r/w for bp 
#define PORT5_USI7_ADDR 			0x800C8C	// r/o for mcu, r/w for bp 
#define PORT6_USI7_ADDR 			0x800CAC	// r/o for mcu, r/w for bp 
#define PORT7_USI7_ADDR 			0x800CCC	// r/o for mcu, r/w for bp 

#define PORT1_USI8_ADDR 			0x800C0E	// r/o for mcu, r/w for bp 
#define PORT2_USI8_ADDR 			0x800C2E	// r/o for mcu, r/w for bp 
#define PORT3_USI8_ADDR 			0x800C4E	// r/o for mcu, r/w for bp 
#define PORT4_USI8_ADDR 			0x800C6E	// r/o for mcu, r/w for bp 
#define PORT5_USI8_ADDR 			0x800C8E	// r/o for mcu, r/w for bp 
#define PORT6_USI8_ADDR 			0x800CAE	// r/o for mcu, r/w for bp 
#define PORT7_USI8_ADDR 			0x800CCE	// r/o for mcu, r/w for bp 

#define PORT1_USI9_ADDR 			0x800C10	// r/o for mcu, r/w for bp 
#define PORT2_USI9_ADDR 			0x800C30	// r/o for mcu, r/w for bp 
#define PORT3_USI9_ADDR 			0x800C50	// r/o for mcu, r/w for bp 
#define PORT4_USI9_ADDR 			0x800C70	// r/o for mcu, r/w for bp 
#define PORT5_USI9_ADDR 			0x800C90	// r/o for mcu, r/w for bp 
#define PORT6_USI9_ADDR 			0x800CB0	// r/o for mcu, r/w for bp 
#define PORT7_USI9_ADDR 			0x800CD0	// r/o for mcu, r/w for bp 

#define PORT1_USI10_ADDR 			0x800C12	// r/o for mcu, r/w for bp 
#define PORT2_USI10_ADDR 			0x800C32	// r/o for mcu, r/w for bp 
#define PORT3_USI10_ADDR 			0x800C52	// r/o for mcu, r/w for bp 
#define PORT4_USI10_ADDR 			0x800C72	// r/o for mcu, r/w for bp 
#define PORT5_USI10_ADDR 			0x800C92	// r/o for mcu, r/w for bp 
#define PORT6_USI10_ADDR 			0x800CB2	// r/o for mcu, r/w for bp 
#define PORT7_USI10_ADDR 			0x800CD2	// r/o for mcu, r/w for bp 

#define PORT1_USI11_ADDR 			0x800C14	// r/o for mcu, r/w for bp 
#define PORT2_USI11_ADDR 			0x800C34	// r/o for mcu, r/w for bp 
#define PORT3_USI11_ADDR 			0x800C54	// r/o for mcu, r/w for bp 
#define PORT4_USI11_ADDR 			0x800C74	// r/o for mcu, r/w for bp 
#define PORT5_USI11_ADDR 			0x800C94	// r/o for mcu, r/w for bp 
#define PORT6_USI11_ADDR 			0x800CB4	// r/o for mcu, r/w for bp 
#define PORT7_USI11_ADDR 			0x800CD4	// r/o for mcu, r/w for bp 

#define PORT1_USI12_ADDR 			0x800C16	// r/o for mcu, r/w for bp 
#define PORT2_USI12_ADDR 			0x800C36	// r/o for mcu, r/w for bp 
#define PORT3_USI12_ADDR 			0x800C56	// r/o for mcu, r/w for bp 
#define PORT4_USI12_ADDR 			0x800C76	// r/o for mcu, r/w for bp 
#define PORT5_USI12_ADDR 			0x800C96	// r/o for mcu, r/w for bp 
#define PORT6_USI12_ADDR 			0x800CB6	// r/o for mcu, r/w for bp 
#define PORT7_USI12_ADDR 			0x800CD6	// r/o for mcu, r/w for bp 

#define PORT1_USI13_ADDR 			0x800C18	// r/o for mcu, r/w for bp 
#define PORT2_USI13_ADDR 			0x800C38	// r/o for mcu, r/w for bp 
#define PORT3_USI13_ADDR 			0x800C58	// r/o for mcu, r/w for bp 
#define PORT4_USI13_ADDR 			0x800C78	// r/o for mcu, r/w for bp 
#define PORT5_USI13_ADDR 			0x800C98	// r/o for mcu, r/w for bp 
#define PORT6_USI13_ADDR 			0x800CB8	// r/o for mcu, r/w for bp 
#define PORT7_USI13_ADDR 			0x800CD8	// r/o for mcu, r/w for bp 


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


    struct inventory
    {
        i8 manufact[32];
        i8 model_num[32];
        i8 part_num[32];
        i8 serial_num[32];
        i8 revision[32];
        i8 manufact_date[32];
        i8 repair_date[32];
        i8 repair_code[32];
        i8 clei_number[32];
        i8 usi_number[32];
    };

    struct module_inventory
    {
        u32 wave;       // Module Wave Length
        u32 dist;       // Module Distance(km unit)
        u32 max_rate;   // Max tansmit speed (Gbps)
        i8 serial_num[32];
        i8 vendor[32];
        i8 part_num[32];
        i8 acl_partlist[32];
        i8 date_code[8];
    };

    struct port_status
    {
        u8  equip;
        u8  tx_laser_sts;   // tx_laser_status 0:On, 1:Off
        u8  lpbk_sts;       // line-test loopback status
        u8  sf_led;         // Signal fail led status (ON/OFF)

        // port alarm status
        u16 alm_status; // alarm status
        u16 defect_status; // defect status
        u16 connect_status; // connectiong status, CONNECT_OK, CONNECT_NOK, CONNECT_ING
        u16 reserved;
        
        //DDM information
        f32 rx_pwr;     // Module Rx Power (dbm)
        f32 rx_min, rx_max, rx_avg;     // 1 minute (dbm)
        f32 tx_pwr;     // Module Tx Power (dbm)
        f32 tx_min, tx_max, tx_avg;     // 1 minute (dbm)
        f32 vcc;        // Module Voltage (voltage)
        f32 tx_bias;    // Tx bias (mA)
        f32 temp;       // Module Temperature(degree)
        
        u32 cv;         //CV error counter
        u32 fcs;        //FCS error counter
        u32 pm_time;
        u32 pm_lap_time; // Path measurement delay time (use  1, 3 port)
   };

 
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
