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

//#define PATH_SYSMON_PID "/var/run/sysmon.pid"
#define SYSMON_BUG_ADDRESS "balkrow@hfrnet.com"
#define SYSMON_DEFAULT_CONFIG    "sysmon.conf"
#define SYSMON_VTY_PORT                 9998 

//define Memory Macro
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
        SUCCESS             = 0,  // Success
        ERR_UNKNOWN         = -1, // Unknown type
        ERR_MALLOC          = -2, // Memory allocation fail
        ERR_LIST_FULL       = -3, // List register fail
        ERR_ARGS            = -4, // Function argument fail
        ERR_NOT_FOUND       = -5, // List not found
        ERR_FILE_OPEN       = -6, // File open fail
        ERR_INVALID_IP      = -7, // Invalid ipaddress
        ERR_INVALID_PARAM   = -8, // Invalid parameter
        ERR_IPC_FAILED      = -9, // Ipc failed
        ERR_ALREADY_EXIST   = -10, // Already Exist
        ERR_NOT_EXIST       = -11, // Not Exist
        ERR_PORT_NOT_READY  = -12, // Port is not ready status
        ERR_MSG_VER_MIS     = -13, // Message Version Mismatch
        ERR_MSG_SIZE_MIS    = -14, // Message Size Mismatch
        ERR_MSG_NOT_SUPPORT = -15, // Message Not Support
        ERR_NOT_SUPPORT = -16, // Message Not Support
#if 1   //YDB_JWKIM
        ERR_MSG_HEADER_MIS  = -17,  // Message Header Mismatch
        ERR_MSG_TIME_OUT    = -18,  // TIME-OUT fail
        ERR_HARDWARE_RE = -19,      // TIME-OUT fail
#endif
        ERR_TYPE_MAX,
    };

    enum
    {
        CMD_OK                      = 1,    // COMMAND OK
        CMD_FAIL_UNKNOWN            = 2,    // UNSUPPORTED TYPE
        CMD_FAIL_PORT_NOT_SUPPORT   = 3,    // UNSUPPORTED PORT
        CMD_FAIL_UNIT_NOT_SUPPORT   = 4,    // UNSUPPORTED UNIT
        CMD_FAIL_SHELF_NOT_SUPPORT  = 5,    // UNSUPPORTED SHELF
        CMD_FAIL_PORT_NOT_FOUND     = 6,    // UNLISTED PORT
        CMD_FAIL_UNIT_NOT_FOUND     = 7,    // UNLISTED UNIT
        CMD_FAIL_SHELF_NOT_FOUND    = 8,    // UNLISTED SHELF
        CMD_FAIL_ALREADY_CONFIG     = 9,    // ALREADY CONFIGURED
        CMD_FAIL_ARGUMENT           = 10,   // INVALID ARGUMENT"
        CMD_FAIL_LIST_FULL          = 11,   // LIST FULL STATE
        CMD_FAIL_UNIT_EXIST         = 12,   // ALREADY UNIT ACTIVE
        CMD_FAIL_UNIT_EMPTY         = 13,   // ALREADY UNIT BLANK
        CMD_DOWNLOAD_FAIL           = 14,   // RDL_FAIL
        CMD_FAIL                    = 15,   // COMMAND FAIL
        CMD_NO_DISTANCE_RESULT      = 16,
        CMD_DOWNLOAD_START          = 17,   // RDL START
        CMD_DOWNLOAD_DONE           = 18,   // RDL DONE
        CMD_DOWNLOAD_CANCEL         = 19,   // RDL CANCEL
        CMD_FAIL_AUTO_NEGO          = 20,   // AUTO_NEGO COMMAND FAIL
        CMD_FAIL_UNIT_NOT_READY     = 21,   // NOT READY UNIT
        CMD_FAIL_INVALID_IP_ADDR    = 22,   // invalid ipaddress
        CMD_FAIL_FILE_NOT_FOUND     = 23,   // Unlisted file
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
        PORT_ID_EAG6L_MAX,
	};
