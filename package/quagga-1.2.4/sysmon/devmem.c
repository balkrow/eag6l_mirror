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

#if 1 /* [#62] SFP eeprom 및 register update 기능 단위 검증 및 디버깅, balkrow, 2024-06-21 */
uint16_t fpga_sim_val = 0x100;
#endif

uint16_t sys_fpga_memory_read(uint16_t addr, uint8_t port_reg) 
{

#ifdef ACCESS_SIM
#if 1 /* [#62] SFP eeprom 및 register update 기능 단위 검증 및 디버깅, balkrow, 2024-06-21 */
	return fpga_sim_val;
#endif
#else
        fpgamemory_t fpgamemory;

#if 1/*[#56] register update timer 수정, balkrow, 2023-06-13 */
	if(port_reg)
		fpgamemory.addr = PORT_BASE + addr;
	else
		fpgamemory.addr = addr;
#else
        fpgamemory.addr = addr;
#endif
	fpgamemory.value = 0;

        ioctl(g_hdrv_fd, HDRIVER_IOCG_FPGA_SHOW_MEMORY, &fpgamemory);

        return fpgamemory.value;
#endif
}

uint16_t sys_fpga_memory_write(uint16_t addr, uint16_t writeval, uint8_t port_reg) {
#ifdef ACCESS_SIM
	zlog_debug("[fpga]  reg=%x, writeval=%x", addr, writeval);
	return writeval;
#else
        fpgamemory_t fpgamemory;

#if 1/*[#56] register update timer 수정, balkrow, 2023-06-13 */
	if(port_reg)
		fpgamemory.addr = PORT_BASE + addr;
	else
		fpgamemory.addr = addr;
#else /*! 56*/
        fpgamemory.addr = addr;
#endif /*End 56 */
	fpgamemory.value = writeval;
        ioctl(g_hdrv_fd, HDRIVER_IOCS_FPGA_WRITE_MEMORY, &fpgamemory);

        return fpgamemory.value;
#endif
}

uint16_t sys_cpld_memory_read(uint16_t addr) {

#ifdef ACCESS_SIM
	return 0x100;
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
	return 0xaa;
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
