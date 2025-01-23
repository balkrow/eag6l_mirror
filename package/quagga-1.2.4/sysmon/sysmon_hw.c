/* 
	Copyright ¡§I 2014, by HFR,Inc. All Right Reserved.

	This Software of copyright is reserved by HFR.Inc. No all or part of this Software may be reproduced by  the 
	hard-copy format, electronically, or otherwise and transmitted to Ineligible Person. If not you should be subject 
	to civil suit and criminal case according to circumstances.

	º» ÆÄÀÏÀÇ ÀúÀÛ±ÇÀº (ÁÖ)¿¡Ä¡¿¡ÇÁ¾Ë¿¡ ÀÖ½À´Ï´Ù. (ÁÖ)¿¡Ä¡¿¡ÇÁ¾ËÀÇ ¸í½ÃÀûÀÎ µ¿ÀÇ¾øÀÌ º» ÆÄÀÏÀÇ ÀüÃ¼ ¶Ç´Â ÀÏºÎ¸¦ 
	ÇÏµåÄ«ÇÇ Çü½Ä, ÀüÀÚÀû ¹æ½Ä ¶Ç´Â ±âÅ¸¹æ½ÄÀ¸·Î Àç»ý»êÇÏ°Å³ª ¹Þ¾Æº¼ ¼ö ÀÖ´Â ÀÚ°ÝÀÌ ¾ø´Â »ç¶÷¿¡°Ô Àç¹èÆ÷ÇÏ´Â ÇàÀ§´Â
	ÀúÀÛ±Ç¹ýÀ» À§¹ÝÇÏ´Â °ÍÀÌ¸ç ¹Î»ç»ó ¼ÕÇØ¹è»ó ±×¸®°í ÇØ´çÇÏ´Â °æ¿ì¿¡´Â Çü»ç»ó Ã³¹úÀÇ ´ë»óÀÌ µË´Ï´Ù
*/
#include "zebra.h"
#include "sysmon.h" 
#include "log.h" 
#include "thread.h" 
#include "sys/ioctl.h" 
#if 1 /* [#62] SFP eeprom 및 register update 기능 단위 검증 및 디버깅, balkrow, 2024-06-21 */ 
#include "bp_regs.h" 
#endif
#if 1/*[#71] EAG6L Board Bring-up, balkrow, 2024-07-04*/
#include "rsmu.h" 
#endif

#undef DEBUG
#if 0/*[#61] Adding omitted functions, dustin, 2024-06-24 */
#define ACCESS_SIM	/* moved to sysmon.h */
#endif

int32_t g_rsmu_fd = UNINITIALIZED_FD;
int32_t g_hdrv_fd = UNINITIALIZED_FD;

int8_t rsmu_init (void)
{
	int8_t;
	if(g_rsmu_fd == UNINITIALIZED_FD)
	{
		if((g_rsmu_fd = open(RSMU_DEVICE_NAME, O_WRONLY)) < 0)
		{
			zlog_err("%s open faild %s(%d)", RSMU_DEVICE_NAME, strerror(errno), errno);
			return RT_NOK;
		} else
			zlog_notice("%s open success", RSMU_DEVICE_NAME);
	} else 
		zlog_notice("%s already opened", RSMU_DEVICE_NAME);
	
	return RT_OK;
}

int8_t rsmuGetPLLState(void) 
{
	RSMU_PLL_STATE get;

	memset(&get, 0, sizeof(get));
	get.dpll = RSMU_PLL_IDX;

	if(ioctl(g_rsmu_fd, RSMU_GET_STATE, &get))
	{
		zlog_err("%s ioctl faild %s(%d)", RSMU_DEVICE_NAME, strerror(errno), errno);
		return RT_NOK;
	}
#ifdef DEBUG
	zlog_notice("pll state %x:%x", get.dpll, get.state);
#endif
	return get.state;
}

#if 1/*[177] link down 시 clock 절체가 안되거나 oper interface 바뀌지 않음, balkrow, 2024-10-30*/
int8_t rsmuGetClockIdx(void) 
{
	RSMU_PLL_CLK_IDX get;

	memset(&get, 0, sizeof(get));
	get.dpll = RSMU_PLL_IDX;

	if(ioctl(g_rsmu_fd, RSMU_GET_CURRENT_CLOCK_INDEX, &get))
	{
		zlog_err("%s ioctl faild %s(%d)", RSMU_DEVICE_NAME, strerror(errno), errno);
		return RT_NOK;
	}
#ifdef DEBUG
	zlog_notice("pll idx %x:%x", get.dpll, get.idx);
#endif
	return get.idx;
}
#endif

#if 1/*[#246] force Freerun 동작 추가, balkrow, 2025-01-17*/
int8_t rsmuSetClockStateMode(uint8_t mode) 
{
	RSMU_REG_RW set;
	RSMU_REG_RW get;

	memset(&set, 0, sizeof(set));
	memset(&get, 0, sizeof(get));

	set.offset = 0x2010c4ef;
	set.byte_count = 1;
	set.bytes[0] = mode;

	if(ioctl(g_rsmu_fd, RSMU_REG_WRITE, &set))
	{
		zlog_err("%s ioctl faild %s(%d)", RSMU_DEVICE_NAME, strerror(errno), errno);
		return RT_NOK;
	}

	get.offset = 0x2010c4ef;
	get.byte_count = 1;
	if(ioctl(g_rsmu_fd, RSMU_REG_READ, &get))
	{
		zlog_err("%s ioctl faild %s(%d)", RSMU_DEVICE_NAME, strerror(errno), errno);
		return RT_NOK;
	}
#ifdef DEBUG
	zlog_notice("%s: pll idx %x state %x, mode %x", __FUNCTION__, RSMU_PLL_IDX, get.bytes[0], mode);
#endif
	return 0;
}
#endif

#if 1/*[#245] primary interface가 none 일시 emsc QL 비교 하지 않도록 수정, balkrow, 2025-01-17*/
int8_t rsmuSetPriClockIdx(uint8_t clk_idx, uint8_t priority) 
{
	RSMU_CLK_PRI set;

	memset(&set, 0, sizeof(set));
	set.dpll = RSMU_PLL_IDX;
	set.num_entries = 1;

	/* Write clock priority table starting with highest priority */
	set.priority_entry[0].clock_index = clk_idx;
	set.priority_entry[0].priority = priority;
#if 0
	prev_rank = priority_entry->rank;
	priority_entry++;
	for(i = 1; i < num_entries; i++) {
		set.priority_entry[i].clock_index = priority_entry->clk_idx;
		if(prev_rank != priority_entry->rank) {
			prev_rank = priority_entry->rank;
			priority++;
		}

		set.priority_entry[i].priority = priority;

		priority_entry++;
	}
#endif

	if(ioctl(g_rsmu_fd, RSMU_SET_CLOCK_PRIORITIES, &set)) {
		zlog_err("%s failed: %s", __func__, strerror(errno));
		return RT_NOK;
	}

	return RT_OK;
}
#endif

int8_t hdriv_init(void)
{
		
	if(g_hdrv_fd == UNINITIALIZED_FD)
	{
		if((g_hdrv_fd = open(HDRV_DEVICE_NAME ,O_RDWR)) < 0)
		{
			zlog_err("%s open faild", HDRV_DEVICE_NAME);
			return RT_NOK;
		} else
			zlog_notice("%s open success", HDRV_DEVICE_NAME);
	} else 
		zlog_notice("%s already opened", HDRV_DEVICE_NAME);
	
	return RT_OK;
}

int8_t monitor_hw_init(void)
{
	int8_t rc = 0;
	rc = rsmu_init();
	rc += hdriv_init();
#if 1 /* [#62] SFP eeprom 및 register update 기능 단위 검증 및 디버깅, balkrow, 2024-06-21 */ 
	if(rc)
	{
		/**
		 * WRITE CPU FAIL (0x12)
		 * */
		gRegUpdate(CPU_FAIL_ADDR, 8, CPU_FAIL_MASK, 1);
	}

	return rc;
#endif
}
