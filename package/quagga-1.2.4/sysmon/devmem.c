#include <sys/mman.h>
#include <unistd.h>
#include "zebra.h"
#include "log.h" 
#include "sysmon.h"
#include <hdriver.h>

extern int32_t g_hdrv_fd;

#if 1/*[#53] Clock source status 업데이트 기능 추가, balkrow, 2024-06-13*/
#define ACCESS_SIM
#endif

uint16_t sys_fpga_memory_read(uint16_t addr) {

#ifdef ACCESS_SIM
	return 0x1010;
#else
        fpgamemory_t fpgamemory;

        fpgamemory.addr = addr;
	fpgamemory.value = 0;

        ioctl(g_hdrv_fd, HDRIVER_IOCG_FPGA_SHOW_MEMORY, &fpgamemory);

        return fpgamemory.value;
#endif
}

uint16_t sys_fpga_memory_write(uint16_t addr, uint16_t writeval) {
#ifdef ACCESS_SIM
	zlog_debug("[fpga]  reg=%x, writeval=%x", addr, writeval);
	return writeval;
#else
        fpgamemory_t fpgamemory;

        fpgamemory.addr = addr;
	fpgamemory.value = writeval;
        ioctl(g_hdrv_fd, HDRIVER_IOCS_FPGA_WRITE_MEMORY, &fpgamemory);

        return fpgamemory.value;
#endif
}

uint16_t sys_cpld_memory_read(uint16_t addr) {

#ifdef ACCESS_SIM
	return 0x1010;
#else
        cpldmemory_t cpldmemory;

        cpldmemory.addr = addr;
	cpldmemory.value = 0;

        ioctl(g_hdrv_fd, HDRIVER_IOCG_CPLD_SHOW_MEMORY, &cpldmemory);

        return cpldmemory.value;
#endif
}

uint16_t sys_cpld_memory_write(uint16_t addr, uint16_t writeval) {
#ifdef ACCESS_SIM
	zlog_debug("[cpld] reg=%x, writeval=%x", addr, writeval);
	return writeval;
#else
        cpldmemory_t cpldmemory;

        cpldmemory.addr = addr;
	cpldmemory.value = writeval;
        ioctl(g_hdrv_fd, HDRIVER_IOCS_CPLD_WRITE_MEMORY, &cpldmemory);

        return cpldmemory.value;
#endif
}

uint16_t sys_dpram_memory_read(uint16_t addr) {

#ifdef ACCESS_SIM
	return 0x1010;
#else
        dprammemory_t dprammemory;

        dprammemory.addr = addr;
	dprammemory.value = 0;

        ioctl(g_hdrv_fd, HDRIVER_IOCG_DPRAM_SHOW_MEMORY, &dprammemory);

        return dprammemory.value;
#endif
}

uint16_t sys_dpram_memory_write(uint16_t addr, uint16_t writeval)
{
#ifdef ACCESS_SIM
	zlog_debug("[dpram] reg=%x, writeval=%x", addr, writeval);
	return writeval;
#else
        dprammemory_t dprammemory;

        dprammemory.addr = addr;
	dprammemory.value = writeval;
        ioctl(g_hdrv_fd, HDRIVER_IOCS_DPRAM_WRITE_MEMORY, &dprammemory);

        return dprammemory.value;
#endif
}
