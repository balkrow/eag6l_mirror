#include <sys/mman.h>
#include <unistd.h>
#include "sysmon.h"
#include "hdriver.h"

extern int32_t hdrv_fd;

uint16_t sys_fpga_memory_read(uint16_t addr) {

        fpgamemory_t fpgamemory;

        fpgamemory.addr = addr;
	fpgamemory.value = 0;

        ioctl(hdrv_fd, HDRIVER_IOCG_FPGA_SHOW_MEMORY, &fpgamemory);

        return fpgamemory.value;
}

uint16_t sys_fpga_memory_write(uint16_t addr, uint16_t writeval)
{
        fpgamemory_t fpgamemory;

        fpgamemory.addr = addr;
	fpgamemory.value = writeval;
        ioctl(hdrv_fd, HDRIVER_IOCS_FPGA_WRITE_MEMORY, &fpgamemory);

        return fpgamemory.value;
}

uint16_t sys_cpld_memory_read(uint16_t addr) {

        cpldmemory_t cpldmemory;

        cpldmemory.addr = addr;
	cpldmemory.value = 0;

        ioctl(hdrv_fd, HDRIVER_IOCG_CPLD_SHOW_MEMORY, &cpldmemory);

        return cpldmemory.value;
}

uint16_t sys_cpld_memory_write(uint16_t addr, uint16_t writeval)
{
        cpldmemory_t cpldmemory;

        cpldmemory.addr = addr;
	cpldmemory.value = writeval;
        ioctl(hdrv_fd, HDRIVER_IOCS_CPLD_WRITE_MEMORY, &cpldmemory);

        return cpldmemory.value;
}

uint16_t sys_dpram_memory_read(uint16_t addr) {

        dprammemory_t dprammemory;

        dprammemory.addr = addr;
	dprammemory.value = 0;

        ioctl(hdrv_fd, HDRIVER_IOCG_DPRAM_SHOW_MEMORY, &dprammemory);

        return dprammemory.value;
}

uint16_t sys_dpram_memory_write(uint16_t addr, uint16_t writeval)
{
        dprammemory_t dprammemory;

        dprammemory.addr = addr;
	dprammemory.value = writeval;
        ioctl(hdrv_fd, HDRIVER_IOCS_DPRAM_WRITE_MEMORY, &dprammemory);

        return dprammemory.value;
}
