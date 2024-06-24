#include <sys/mman.h>
#include <unistd.h>
#include "zebra.h"
#include "log.h" 
#include "sysmon.h"
#if 1/*[#65] Adding regMon simulation feature under ACCESS_SIM, dustin, 2024-06-24 */
#include "bp_regs.h"
#endif
#include <hdriver.h>

extern int32_t g_hdrv_fd;

#if 1/*[#53] Clock source status 업데이트 기능 추가, balkrow, 2024-06-13*/
#define ACCESS_SIM
#if 1/*[#65] Adding regMon simulation feature under ACCESS_SIM, dustin, 2024-06-24 */
static uint16_t __CACHE_SYNCE_GCONFIG = 0x0;
static uint16_t __CACHE_SYNCE_IF_SELECT = 0x0;
static uint16_t __CACHE_PM_COUNT_CLEAR = 0x0;
static uint16_t __CACHE_CHIP_RESET = 0x0;
static uint16_t __CACHE_SFP_CR = 0x0;
static uint16_t __CACHE_BD_SFP_CR = 0x0;
static uint16_t __CACHE_FW_BANK_SELECT = 0x0;
static uint16_t __CACHE_SW_VERSION = 0x0;
static uint16_t __CACHE_COMMON_CTRL2[PORT_ID_EAG6L_MAX] = { 0x0, };
static uint16_t __CACHE_PORT_CONF[PORT_ID_EAG6L_MAX] = { 0x0, };
static uint16_t __CACHE_PORT_ALM_MASK[PORT_ID_EAG6L_MAX] = { 0x0, };
#endif
#endif

#if 1 /* [#62] SFP eeprom 및 register update 기능 단위 검증 및 디버깅, balkrow, 2024-06-21 */
uint16_t fpga_sim_val = 0x100;
#endif

uint16_t sys_fpga_memory_read(uint16_t addr, uint8_t port_reg) 
{
#ifdef ACCESS_SIM
#if 1/*[#65] Adding regMon simulation feature under ACCESS_SIM, dustin, 2024-06-24 */
	int portno;

	/* match single registers */
	switch(addr) {
		case SYNCE_GCONFIG_ADDR:
			return __CACHE_SYNCE_GCONFIG;
		case SYNCE_IF_SELECT_ADDR:
			return __CACHE_SYNCE_IF_SELECT;
		case PM_COUNT_CLEAR_ADDR:
			return __CACHE_PM_COUNT_CLEAR;
		case CHIP_RESET_ADDR:
			return __CACHE_CHIP_RESET;
		case BD_SFP_CR_ADDR:
			return __CACHE_BD_SFP_CR;
		case FW_BANK_SELECT_ADDR:
			return __CACHE_FW_BANK_SELECT;
		case SW_VERSION_ADDR:
			return __CACHE_SW_VERSION;
		default:/*pass-through*/
			break;
	}

	/* scan/match per-port registers */
	for(portno = PORT_ID_EAG6L_PORT1; portno < PORT_ID_EAG6L_MAX; portno++) {
		if(addr == __COMMON_CTRL2_ADDR[portno])
			return __CACHE_COMMON_CTRL2[portno];
		else if(addr == __PORT_CONFIG_ADDR[portno])
			return __CACHE_PORT_CONF[portno];
		else if(addr == __PORT_ALM_MASK_ADDR[portno])
			return __CACHE_PORT_ALM_MASK[portno];
	}
	return 0xFFFF;
#else
	return 0x0100;
#endif/*[#65]*/
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
#if 1/*[#65] Adding regMon simulation feature under ACCESS_SIM, dustin, 2024-06-24 */
	int portno;
	zlog_debug("[fpga]  reg=%x, writeval=%x", addr, writeval);

	/* match single registers */
	if(! port_reg) {
		switch(addr) {
			case SYNCE_GCONFIG_ADDR:
				return (__CACHE_SYNCE_GCONFIG = writeval);
			case SYNCE_IF_SELECT_ADDR:
				return (__CACHE_SYNCE_IF_SELECT = writeval);
			case PM_COUNT_CLEAR_ADDR:
				return (__CACHE_PM_COUNT_CLEAR = writeval);
			case CHIP_RESET_ADDR:
				return (__CACHE_CHIP_RESET = writeval);
			case BD_SFP_CR_ADDR:
				return (__CACHE_BD_SFP_CR = writeval);
			case FW_BANK_SELECT_ADDR:
				return (__CACHE_FW_BANK_SELECT = writeval);
			case SW_VERSION_ADDR:
				return (__CACHE_SW_VERSION = writeval);
			default:/*pass-through*/
				break;
		}
	}
	else {
	/* scan/match per-port registers */
		for(portno = PORT_ID_EAG6L_PORT1; portno < PORT_ID_EAG6L_MAX; portno++) {
			if(addr == __COMMON_CTRL2_ADDR[portno])
				return (__CACHE_COMMON_CTRL2[portno] = writeval);
			else if(addr == __PORT_CONFIG_ADDR[portno])
				return (__CACHE_PORT_CONF[portno] = writeval);
			else if(addr == __PORT_ALM_MASK_ADDR[portno])
				return (__CACHE_PORT_ALM_MASK[portno] = writeval);
			else
				return 0xFFFF;
		}
	}
#else
	zlog_debug("[fpga]  reg=%x, writeval=%x", addr, writeval);
	return writeval;
#endif/*[#65]*/
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
