#ifndef _SYSMON_H_
#define _SYSMON_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/utsname.h>
#include <sys/param.h>	/* for HZ */
#include <syslog.h> 
#include <sys/socket.h> 
#include <sys/un.h> 
#include <netinet/in.h> 
#include <linux/if.h> 
#include <linux/sockios.h> 
#include <signal.h> 
#include <time.h>
#include <sys/uio.h>
#include <sys/ioctl.h>
#include <dirent.h>
#include <sys/vfs.h>
#include <sys/resource.h>
#include <stddef.h>
#include <errno.h>
#include <assert.h>
#if 1/*[#56] register update timer ¿¿, balkrow, 2023-06-13 */
#include "svc_fsm.h"
#endif

#if 1/*[#71] EAG6L Board Bring-up, balkrow, 2024-07-04*/
#undef ACCESS_SIM	/* define if simulation for access on demo system */
#endif

#if 1 /* [#91] Fixing for register updating feature, dustin, 2024-08-05 */
#define BP_BYTE_SWAP
#endif

/*
#define PATH_SYSMON_PID "/var/run/sysmon.pid"
*/
#define SYSMON_BUG_ADDRESS "balkrow@hfrnet.com"
#define SYSMON_DEFAULT_CONFIG    "sysmon.conf"
#define SYSMON_VTY_PORT                 9998 

#if 1/*[#56] register update timer 수정, balkrow, 2023-06-13 */
#if 1 /* [#91] Fixing for register updating feature, dustin, 2024-08-05 */
extern uint16_t sys_fpga_memory_read(uint32_t addr, uint8_t port_reg);
extern uint16_t sys_fpga_memory_write(uint32_t addr, uint16_t val, uint8_t port_reg);
#else
extern uint16_t sys_fpga_memory_read(uint16_t addr, uint8_t port_reg);
extern uint16_t sys_fpga_memory_write(uint16_t addr, uint16_t val, uint8_t port_reg);
#endif
#if 1/* [#70] Adding RDL feature, dustin, 2024-07-02 */
extern uint16_t sys_dpram_memory_read(uint32_t addr);
extern uint16_t sys_dpram_memory_write(uint32_t addr, uint16_t val);
#else
extern uint16_t sys_dpram_memory_read(uint16_t addr);
extern uint16_t sys_dpram_memory_write(uint16_t addr, uint16_t val);
#endif
#endif
#if 1/*[#120] LOC Alarm process ¿¿, balkrow, 2024-10-16 */
#define MAX_LOC_CNT 3
#endif
/*
define Memory Macro
*/
#if 1/*[#56] register update timer 수정, balkrow, 2023-06-13 */
#define PORT_REG 1
#define PORT_NOREG 0

#define FPGA_READ(x) sys_fpga_memory_read(x, PORT_NOREG)
#define FPGA_WRITE(x, y) sys_fpga_memory_write(x, y, PORT_NOREG)
#define FPGA_PORT_READ(x) sys_fpga_memory_read(x, PORT_REG)
#define FPGA_PORT_WRITE(x, y) sys_fpga_memory_write(x, y, PORT_REG)
#else
#define FPGA_READ sys_fpga_memory_read
#define FPGA_WRITE sys_fpga_memory_write
#endif

#define DPRAM_READ sys_dpram_memory_read
#define DPRAM_WRITE sys_dpram_memory_write
#define CPLD_READ sys_cpld_memory_read
#define CPLD_WRITE sys_cpld_memory_write

#if 1/*[#53] Clock source status 업데이트 기능 추가, balkrow, 2024-06-13*/
#define UNINITIALIZED_FD   -1
#if 1/*[#80] eag6l board SW bring-up, balkrow, 2023-07-24 */
#define RSMU_DEVICE_NAME   "/dev/rsmu0"
#define HDRV_DEVICE_NAME   "/dev/hdrv"
#endif
#if 1/*[#71] EAG6L Board Bring-up, balkrow, 2024-07-15*/
#define RSMU_PLL_IDX	4  
#endif
#if 1 /*[#82] eag6l board SW Debugging, balkrow, 2024-08-09*/
#define RT_OK 0xff
#endif
#define RT_NOK 1
#if 1 /* [#62] SFP eeprom 및 register update 기능 단위 검증 및 디버깅, balkrow, 2024-06-21 */
#define UNKNOWN 0
#endif
#define FREERUN 1
#define LOCK_RECOVERY 2
#define PLL_LOCK 4
#define HOLD_OVER 5

#define SYS_INIT_FAIL 1
#define SYS_INIT_DONE 0xAA

#if 1/* [#78] Adding system inventory management, dustin, 2024-07-24 */
#define EAG6L_INVENTORY_FILE        "/mnt/flash/.EAG6L_IDATA"
#endif

#if 1/*[#106] init 시 FPGA update 기능 추가, balkrow, 2024-08-28 */
#define FPGA_IMG_DIR        "/root"
#define FPGA_IMG_PREFIX        "eag6l_fpga_v"
#define FPGA_IMG_LEN 32
#define FPGA_MAX_WAIT 2
#define FPGA_SWITCH_MAX 2
#endif

#if 1/*[#53] Clock source status 업데이트 기능 추가, balkrow, 2024-06-13*/
#define PRI_SRC 1
#define SEC_SRC 0
#endif

#if 1/*[#56] register update timer 수정, balkrow, 2024-06-13 */
#define SDK_INIT_FAIL 1
#define SDK_INIT_DONE 0xAA
#if 1/*[#80] eag6l board SW bring-up, balkrow, 2024-07-22 */
#define SDK_INIT_WAIT 2
#define CFG_ENABLE 1
#if 1/*[#82] eag6l board SW Debugging, balkrow, 2024-08-08*/
#define CFG_DISABLE 2
#endif
#define NOT_DEFINED 0xff
#endif

#define COMM_FAIL 1
#define COMM_SUCCESS 0xAA
#define FSM_MAX_TRIES 10

#define gPortRegUpdate(reg, shift, mask, val) \
{ \
	uint16_t rval = 0, wval =0; \
	rval = FPGA_PORT_READ(reg); \
	wval = (uint16_t)(rval & ~mask) | (uint16_t)((val << shift) & mask); \
	FPGA_PORT_WRITE(reg, wval); \
}
#endif

#define gRegUpdate(reg, shift, mask, val) \
{ \
	uint16_t rval = 0, wval =0; \
	rval = FPGA_READ(reg); \
	wval = (uint16_t)(rval & ~mask) | (uint16_t)((val << shift) & mask); \
	FPGA_WRITE(reg, wval); \
}

#if 1/* [#70] Adding RDL feature, dustin, 2024-07-02 */
#define gDPRAMRegUpdate(reg, shift, mask, val) \
{ \
	uint16_t rval = 0, wval =0; \
	rval = DPRAM_READ(reg); \
	wval = (uint16_t)(rval & ~mask) | (uint16_t)((val << shift) & mask); \
	DPRAM_WRITE(reg, wval); \
}
#endif

#if 1/*[#71] EAG6L Board Bring-up, balkrow, 2024-07-04*/
typedef struct rsmu_get_sts
{
	uint8_t dpll;
	uint8_t state;
} RSMU_PLL_STATE;
#endif

#if 1/*[#110] RDL function Debugging 및 수정, balkrow, 2024-09-02*/
#define RDL_VER_STR_MAX             16/* or 2? */
#define RDL_FILE_NAME_MAX           32
#define PKG_ZIP 1
#define PKG_NONZIP 2
#define PKG_ERROR -1
#define OS_IMG_PREFIX "eag6l-os-v"

typedef struct fw_image_header {
    unsigned int    fih_magic;  /* Image Header Magic Number    */
    unsigned int    fih_hcrc;   /* Image Header CRC Checksum    */
    unsigned int    fih_time;   /* Image Creation Timestamp */
    unsigned int    fih_size;   /* Image Data Size      */
    unsigned int    fih_card_type;  /* card type        */
    unsigned int    fih_dcrc;   /* Image Data CRC Checksum  */
    char        fih_ver[RDL_VER_STR_MAX];    /* Image Version        */
    char        fih_name[RDL_FILE_NAME_MAX];    /* Image Name       */
    char        reserv[8];  /* reserv       */
} fw_image_header_t;
#endif

typedef struct globalDB
{
	uint8_t init_state; /*init state*/
	uint8_t pll_state; /*pll state*/
	uint16_t keepAlive;
#if 1/*[#56] register update timer ¿¿, balkrow, 2023-06-13 */
	uint8_t sdk_init_state; /*sdk init state*/
	uint8_t comm_state; /*sdk init state*/
	uint8_t fsm_retries;
#if 1/*[#80] eag6l board SW bring-up, balkrow, 2023-07-24 */
	uint8_t synce_state;
#endif
#if 1/*[#73] SDK 내에서 CPU trap 된 packet 처리 로직 추가, balkrow, 2024-07-18*/
	uint8_t synce_pri_port;
	uint8_t synce_sec_port;
#endif
#if 1 /*[#82] eag6l board SW Debugging, balkrow, 2024-08-08*/
	uint8_t esmcRxCfg[7];
#endif
#if 1/*[#106] init 시 FPGA update 기능 추가, balkrow, 2024-08-28 */
	uint8_t fpga_running_bank;
	uint8_t fpga_act_bank;
	uint8_t switch_wait_cnt;
	uint8_t switch_cnt;
	uint8_t fpga_version;
	uint8_t os_bank;
#endif
#if 1/*[#110] RDL function Debugging 및 수정, balkrow, 2024-09-02*/
	uint8_t pkg_is_zip;
	fw_image_header_t	bank1_header;
	fw_image_header_t	bank2_header;
#endif
#if 1/*[#122] primary/secondary Send QL 설정, balkrow, 2024-09-09*/
	uint8_t localQL;
#endif
#if 1/*[#127] SYNCE current interface 수정, balkrow, 2024-09-11*/
	uint8_t synce_oper_port;
#endif
	SVC_FSM svc_fsm;
#endif
} GLOBAL_DB;

#define RSMU_MAGIC   '?'
#define RSMU_GET_STATE                      _IOR(RSMU_MAGIC, 2, struct rsmu_get_state)
#endif
#if 1/*[#122] primary/secondary Send QL 설정, balkrow, 2024-09-09*/
#define ESMC_LOCAL_QL 0x12
#endif
/* 
 * type definitions
 */
typedef char            i8;
typedef uint8_t         u8;
typedef int16_t         i16;
typedef uint16_t        u16;
typedef int32_t         i32;
typedef uint32_t        u32;
typedef int64_t         i64;
typedef uint64_t        u64;
typedef float           f32;

    enum {
        SUCCESS             = 0,  /* Success */
        ERR_UNKNOWN         = -1, /* Unknown type */
        ERR_MALLOC          = -2, /* Memory allocation fail */
        ERR_LIST_FULL       = -3, /* List register fail */
        ERR_ARGS            = -4, /* Function argument fail */
        ERR_NOT_FOUND       = -5, /* List not found */
        ERR_FILE_OPEN       = -6, /* File open fail */
        ERR_INVALID_IP      = -7, /* Invalid ipaddress */
        ERR_INVALID_PARAM   = -8, /* Invalid parameter */
        ERR_IPC_FAILED      = -9, /* Ipc failed */
        ERR_ALREADY_EXIST   = -10, /* Already Exist */
        ERR_NOT_EXIST       = -11, /* Not Exist */
        ERR_PORT_NOT_READY  = -12, /* Port is not ready status */
        ERR_MSG_VER_MIS     = -13, /* Message Version Mismatch */
        ERR_MSG_SIZE_MIS    = -14, /* Message Size Mismatch */
        ERR_MSG_NOT_SUPPORT = -15, /* Message Not Support */
        ERR_NOT_SUPPORT = -16, /* Message Not Support */
#if 1   /* YDB_JWKIM */
        ERR_MSG_HEADER_MIS  = -17,  /* Message Header Mismatch */
        ERR_MSG_TIME_OUT    = -18,  /* TIME-OUT fail */
        ERR_HARDWARE_RE = -19,      /* TIME-OUT fail */
#endif
        ERR_TYPE_MAX,
    };

    enum
    {
        CMD_OOK                     = 1,    /* COMMAND OK */
        CMD_FAIL_UNKNOWN            = 2,    /* UNSUPPORTED TYPE */
        CMD_FAIL_PORT_NOT_SUPPORT   = 3,    /* UNSUPPORTED PORT */
        CMD_FAIL_UNIT_NOT_SUPPORT   = 4,    /* UNSUPPORTED UNIT */
        CMD_FAIL_SHELF_NOT_SUPPORT  = 5,    /* UNSUPPORTED SHELF */
        CMD_FAIL_PORT_NOT_FOUND     = 6,    /* UNLISTED PORT */
        CMD_FAIL_UNIT_NOT_FOUND     = 7,    /* UNLISTED UNIT */
        CMD_FAIL_SHELF_NOT_FOUND    = 8,    /* UNLISTED SHELF */
        CMD_FAIL_ALREADY_CONFIG     = 9,    /* ALREADY CONFIGURED */
        CMD_FAIL_ARGUMENT           = 10,   /* INVALID ARGUMENT" */
        CMD_FAIL_LIST_FULL          = 11,   /* LIST FULL STATE */
        CMD_FAIL_UNIT_EXIST         = 12,   /* ALREADY UNIT ACTIVE */
        CMD_FAIL_UNIT_EMPTY         = 13,   /* ALREADY UNIT BLANK */
        CMD_DOWNLOAD_FAIL           = 14,   /* RDL_FAIL */
        CMD_FAIL                    = 15,   /* COMMAND FAIL */
        CMD_NO_DISTANCE_RESULT      = 16,
        CMD_DOWNLOAD_START          = 17,   /* RDL START */
        CMD_DOWNLOAD_DONE           = 18,   /* RDL DONE */
        CMD_DOWNLOAD_CANCEL         = 19,   /* RDL CANCEL */
        CMD_FAIL_AUTO_NEGO          = 20,   /* AUTO_NEGO COMMAND FAIL */
        CMD_FAIL_UNIT_NOT_READY     = 21,   /* NOT READY UNIT */
        CMD_FAIL_INVALID_IP_ADDR    = 22,   /* invalid ipaddress */
        CMD_FAIL_FILE_NOT_FOUND     = 23,   /* Unlisted file */
    };


/* 
 * extern funtions
 */
extern void print_console(const char *fmt, ...); 


    enum
    {
        PORT_ID_EAG6L_NOT_USE = 0,
        PORT_ID_EAG6L_PORT1,
        PORT_ID_EAG6L_PORT2,
        PORT_ID_EAG6L_PORT3,
        PORT_ID_EAG6L_PORT4,
        PORT_ID_EAG6L_PORT5,
        PORT_ID_EAG6L_PORT6,
#ifdef MVDEMO /*[68] eag6l board 를 위한 port number 수정, balkrow, 2024-06-27*/
        PORT_ID_EAG6L_PORT7,
        PORT_ID_EAG6L_PORT8,
        PORT_ID_EAG6L_PORT9,/*100G*/
#else
        PORT_ID_EAG6L_PORT7,/*100G*/
#endif
        PORT_ID_EAG6L_MAX,
	};

	enum ePortIfMode
	{
		PORT_IF_10G_KR,
		PORT_IF_25G_KR,
#if 1/*[#80] eag6l board SW bring-up, balkrow, 2023-07-22 */
		PORT_IF_10G_SR_LR,
		PORT_IF_25G_SR_LR,
#endif
	};
#if 0
	enum ePortLinkStaus
	{
		PORT_LINK_DOWN,
		PORT_LINK_UP,
		PORT_LINK_LAST
	};

	enum ePortFecMode
	{
		PORT_FEC_MODE_ENABLED,
		PORT_FEC_MODE_DISABLED,
		PORT_FEC_MODE_RS,
		PORT_FEC_MODE_BOTH,
		PORT_FEC_MODE_RS_544_514,
		PORT_FEC_MODE_LAST
	};


	enum eAlarmList
	{
		ALM_NONE,
		ALM_UNIT_FAIL,              /* 1 */
		ALM_FLASH_DEFAULT_BOOT,     /* 2 */
		ALM_OPTIC_OUT,              /* 3 */
		ALM_LOS,                    /* 4 */
		ALM_LOF,                    /* 5 */
		ALM_LINK_DOWN,              /* 6 */
		ALM_TX_FAULT,               /* 7 */
		ALM_TEMP_FAIL,              /* 8 */
		ALM_CPU_OVERLOAD,           /* 9 */
		ALM_MEM_FULL,               /* 10 */
		ALM_DISK_FULL,              /* 11 */
		ALM_FAN1_OUT,               /* 12 */
		ALM_FAN1_FAIL,              /* 13 */
		ALM_FAN2_OUT,               /* 14 */
		ALM_FAN2_FAIL,              /* 15 */
		ALM_POWER_IN_FAIL,          /* 16 */
		ALM_POWER_OUT_FAIL,         /* 17 */
		ALM_POWER_ALM_FAIL,         /* 18 add   -> deleted POWER_FAIL */
		ALM_TELEMETRY_1,            /* 19 */
		ALM_TELEMETRY_2,            /* 20 */
		ALM_TELEMETRY_3,            /* 21 */
		ALM_HOLD_OVER,              /* 22 */
		ALM_LOCAL_FAULT,            /* 23 */
		ALM_REMOTE_FAULT,           /* 24 */
		ALM_OPTIC_LOW_PWR,          /* 25 add optic low power */
		ALM_DOOR_OPEN,              /* 26 */
		ALM_IPC_FAIL1,              /* 27 */
		ALM_IPC_FAIL2,              /* 28 */
		ALM_IPC_FAIL3,              /* 29 */
		ALM_IPC_FAIL4,              /* 30 */
		ALM_IPC_FAIL5,              /* 31 */
		ALM_IPC_FAIL6,              /* 32 */
		ALM_L1_LOS,                 /* 33 */
		ALM_L1_LOF,                 /* 34 */
		ALM_L1_SDI,                 /* 35 */
		ALM_L1_RAI,                 /* 37 */
		ALM_OUT_HEALTH,             /* 38 */
		ALM_MISMATCH_FW,            /* 39 */
		ALM_PTP1_LOCK_FAIL,         /* 49 */
		ALM_PTP2_LOCK_FAIL,         /* 50 */
		ALM_LAST,                   /* 51 */
	};
#endif
	typedef struct __port_pm_counter__
	{
		u64 tx_frame;
		u64 tx_byte;
		u64 rx_frame;
		u64 rx_byte;
		u64 rx_fcs;
		u64 fcs_ok;
		u64 fcs_nok;
	} port_pm_counter_t;

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
        u32 wave;       /* Module Wave Length */
        u32 dist;       /* Module Distance(km unit) */
        u32 max_rate;   /* Max tansmit speed (Gbps) */
        i8 serial_num[32];
        i8 vendor[32];
        i8 part_num[32];
        i8 acl_partlist[32];
#if 1 /* [#62] SFP eeprom 및 register update 기능 단위 검증 및 디버깅, balkrow, 2024-06-25 */
        i8 date_code[10];
#endif
#if 1 /* [#125] Fixing for SFP channel no, wavelength, tx/rx dBm, dustin, 2024-09-10 */
		u8 wave_decimal;
#endif
    };

#if 1/* [#72] Adding omitted rtWDM related registers, dustin, 2024-06-27 */
typedef struct _ddm_info_
	{
        /* rtWDM DDM information */
        f32 rx_pwr;     /* Module Rx Power (dbm) */
        f32 rx_min, rx_max, rx_avg;     /* 1 minute (dbm) */
        f32 tx_pwr;     /* Module Tx Power (dbm) */
        f32 tx_min, tx_max, tx_avg;     /* 1 minute (dbm) */
        f32 vcc;        /* Module Voltage (voltage) */
        f32 tx_bias;    /* Tx bias (mA) */
        f32 temp;       /* Module Temperature(degree) */
	f32 laser_temp;
	f32 tec_curr;
        
        u32 cv;         /*CV error counter */
        u32 fcs;        /*FCS error counter */
        u32 pm_time;
        u32 pm_lap_time; /**Path measurement delay time (use  1, 3 port) */
	} ddm_info_t;
#endif

typedef struct port_status
    {
        u8  equip;
        u8  link;
        u8  speed;
        u8  ifmode;
#if 1 /* [#169] Fixing for new DCO install process, dustin, 2024-10-25 */
        u8  cfg_rs_fec;
        u8  cfg_dco_fec;
		u16 cfg_ch_data;
#else
        u8  fec_mode;
#endif
        u8  los;
        u8  lof;
        u8  esmc_loss;
	u8  remote_fault;
#if 1 /* [#88] Adding LF/RF reading and updating to Alarm, dustin, 2024-08-01 */
	u8  local_fault;
#endif
	u8  tsfp_self_lp;
	u8  rtwdm_lp;
	u8  tx_bias_sts;
        u8  tx_laser_sts;   /* tx_laser_status 0:On, 1:Off */
        u8  lpbk_sts;       /* line-test loopback status */
        u8  sf_led;         /* Signal fail led status (ON/OFF) */
#if 1/*[#54] Adding Smart T-SFP I2C functions, dustin, 2024-06-13 */
		u8  sfp_type;
		u8  cfg_esmc_enable;
		u8  cfg_flex_tune;
		u8  flex_tune_status;
#if 1 /* [#160] Fixing for rtWDM flex tune status, dustin, 2024-10-21 */
		u8  flex_tune_rtwdm_status;
#endif
		u8  cfg_smart_tsfp_selfloopback;
		u8  cfg_rtwdm_loopback;
#endif
#if 1/*[#61] Adding omitted functions, dustin, 2024-06-24 */
		u8  tunable_sfp;
		u8  tunable_chno;
		f32 tunable_wavelength;
#if 1/* [#72] Adding omitted rtWDM related registers, dustin, 2024-06-27 */
		f32 tunable_rtwdm_wavelength;
		u8  tunable_rtwdm_sfp_type;
#endif
#endif
#if 1 /* [#157] Fixing for Smart T-SFP rtWDM info, dustin, 2024-10-18 */
		u8  inv_up_flag; /* flag for one time update registers. */
		u8  inv_clear_flag; /* flag for one time clearing registers. */
#endif
#if 1 /* [#154] Fixing for auto FEC mode on DCO, dustin, 2024-10-21 */
		u8  sfp_dco;
#endif
#if 1 /* [#151] Implementing P7 config register, dustin, 2024-10-21 */
		u8  cfg_tx_laser;/*for 100G*/
		u8  cfg_llcf;/*for 100G*/
#endif


        /* port alarm status */
        u16 alm_status; /* alarm status */
        u16 defect_status; /* defect status */
        u16 connect_status; /* connectiong status, CONNECT_OK, CONNECT_NOK, CONNECT_ING */
        u16 reserved;
        
        /* DDM information */
        f32 rx_pwr;     /* Module Rx Power (dbm) */
        f32 rx_min, rx_max, rx_avg;     /* 1 minute (dbm) */
        f32 tx_pwr;     /* Module Tx Power (dbm) */
        f32 tx_min, tx_max, tx_avg;     /* 1 minute (dbm) */
        f32 vcc;        /* Module Voltage (voltage) */
        f32 tx_bias;    /* Tx bias (mA) */
        f32 temp;       /* Module Temperature(degree) */
		f32 laser_temp;
		f32 tec_curr;
        
        u32 cv;         /*CV error counter */
        u32 fcs;        /*FCS error counter */
        u32 pm_time;
        u32 pm_lap_time; /**Path measurement delay time (use  1, 3 port) */

#if 1/* [#72] Adding omitted rtWDM related registers, dustin, 2024-06-27 */
		ddm_info_t rtwdm_ddm_info;
#endif
#if 1 /* [#169] Fixing for new DCO install process, dustin, 2024-10-25 */
		u8  i2cReady;
#endif
#if 1/*[#120] LOC Alarm process ¿¿, balkrow, 2024-10-16 */
	u32 esmc_recv_cnt;
	u32 esmc_prev_cnt;
	u32 received_QL;
	u32 loc_cnt;
#endif
   } port_status_t;

#if 1 /* [#94] Adding for 100G DCO handling, dustin, 2024-09-23 */
#if 1 /* [#150] Implementing LR4 Status register, dustin, 2024-10-21 */
typedef struct lr4_status
	{
		uint16_t  alm_sts;
		uint16_t  alm_flag;
		uint16_t  alm_mask;

		f32       tx_pwr[4];
		f32       rx_pwr[4];
		f32       vcc[4];
		f32       tx_bias[4];
		f32       tec_curr[4];
		f32       wavelength[3]; /*lane2/3/4*/

		uint8_t   tx_bias_mask;
		uint8_t   rx_los_mask;
		uint8_t   rx_lol_mask;
	} lr4_status_t;
#endif /* [#150] */

typedef struct dco_status
	{
		/* dco status */
		u8  dco_IntL;
		u8  dco_DataNotReady;
		u8  dco_TCReadyFlag;
		u8  dco_InitComplete;
		u8  dco_TxLosMask;
		u8  dco_RxLos;
		u8  dco_TxLoLMask;
		u8  dco_RxLoL;
		u8  dco_TempHA;
		u8  dco_TempLA;
		u8  dco_TempHWA;
		u8  dco_TempLWA;
		u8  dco_OpticHA;
		u8  dco_OpticLA;
		u8  dco_OpticHWA;
		u8  dco_OpticLWA;

		u8  dco_power_mode;
		u8  dco_tx_disable;
		u8  dco_host_fec;
		u8  dco_media_fec;

		u16 dco_ch_data;
#if 1 /* [#150] Implementing LR4 Status register, dustin, 2024-10-21 */
		lr4_status_t lr4_stat;
#endif
#if 1 /* [#169] Fixing for new DCO install process, dustin, 2024-10-25 */
#define DCO_INIT_START				0x0
#define DCO_INIT_CHECK_COMPLETE1	0x1
#define DCO_INIT_CONFIG				0x2
#define DCO_INIT_CHECK_COMPLETE2	0x4
#define DCO_INIT_DONE				0x8
#if 1 /* [#172] Fixing for DCO FEC default change, dustin, 2024-10-28 */
#define DCO_INIT_START2				0x10
#endif

#define DCO_BIT_TC_READY			0x2
#define DCO_BIT_INIT_COMPLETE		0x1
		u8  dco_initState;
#endif
	} dco_status_t;

typedef struct dco_count
	{
#if 1 /* [#149] Implementing DCO BER/FER counters, dustin, 2024-10-21 */
		uint16_t ber_data; /* i2c value */
		uint16_t fer_data; /* i2c value */

		f32  ber_rate;/* Pre-BER (Bit Error Ratio) */
		f32  fer_rate;/* FER (Frame Error Rate) */
#else
		u32  ber_count;/* Pre-BER (Bit Error Ratio) */
		u32  fer_count;/* FER (Frame Error Rate) */
#endif
	} dco_count_t;
#endif

#endif/*_SYSMON_H_*/
