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

#define PATH_SYSMON_PID "/var/run/sysmon.pid"
#define SYSMON_BUG_ADDRESS "balkrow@hfrnet.com"

//define Memory Macro
#define FPGA_READ sys_fpga_memory_read
#define FPGA_WRITE sys_fpga_memory_write
#define DPRAM_READ sys_dpram_memory_read
#define DPRAM_WRITE sys_dpram_memory_write
#define CPLD_READ sys_cpld_memory_read
#define CPLD_WRITE sys_cpld_memory_write
