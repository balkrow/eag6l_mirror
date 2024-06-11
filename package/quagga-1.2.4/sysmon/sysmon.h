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

/*
#define PATH_SYSMON_PID "/var/run/sysmon.pid"
*/
#define SYSMON_BUG_ADDRESS "balkrow@hfrnet.com"
#define SYSMON_DEFAULT_CONFIG    "sysmon.conf"
#define SYSMON_VTY_PORT                 9998 

#if 1/*[#48] register monitoring and update 관련 기능 추가, balkrow, 2024-06-10*/
extern uint16_t sys_fpga_memory_read(uint16_t addr);
#endif
/*
define Memory Macro
*/
#define FPGA_READ sys_fpga_memory_read
#define FPGA_WRITE sys_fpga_memory_write
#define DPRAM_READ sys_dpram_memory_read
#define DPRAM_WRITE sys_dpram_memory_write
#define CPLD_READ sys_cpld_memory_read
#define CPLD_WRITE sys_cpld_memory_write


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
        PORT_ID_EAG6L_PORT7,
        PORT_ID_EAG6L_PORT8,/*for-demo-board-100G-test*/
        PORT_ID_EAG6L_PORT9,/*for-demo-board-100G-test*/
        PORT_ID_EAG6L_MAX,
	};

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

	enum ePortIfMode
	{
		PORT_IF_10G_XGMII,
		PORT_IF_10G_RXAUI,
		PORT_IF_10G_KR,
		PORT_IF_10G_SR_LR,
		PORT_IF_10G_HX,
		PORT_IF_25G_KR,
		PORT_IF_25G_CR,
		PORT_IF_100G_KR4,
		PORT_IF_LAST
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
        i8 date_code[8];
    };

    struct port_status
    {
        u8  equip;
        u8  link;
        u8  speed;
        u8  ifmode;
        u8  fec_mode;
        u8  los;
        u8  lof;
        u8  esmc_loss;
		u8  remote_fault;
		u8  tsfp_self_lp;
		u8  rtwdm_lp;
		u8  tx_bias_sts;
        u8  tx_laser_sts;   /* tx_laser_status 0:On, 1:Off */
        u8  lpbk_sts;       /* line-test loopback status */
        u8  sf_led;         /* Signal fail led status (ON/OFF) */

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
   };

#endif/*_SYSMON_H_*/
