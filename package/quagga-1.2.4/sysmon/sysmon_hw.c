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

#undef DEBUG
#define ACCESS_SIM

int32_t g_rsmu_fd = UNINITIALIZED_FD;
int32_t g_hdrv_fd = UNINITIALIZED_FD;

int8_t rsmu_init (void)
{
	int8_t;
	if(g_rsmu_fd == UNINITIALIZED_FD)
	{
		if((g_rsmu_fd = open(RSMU_DEVICE_NAME, O_WRONLY)) < 0)
		{
			zlog_err("%s open faild", RSMU_DEVICE_NAME);
			return RT_NOK;
		} else
			zlog_notice("%s open success", RSMU_DEVICE_NAME);
	} else 
		zlog_notice("%s already opened", RSMU_DEVICE_NAME);
	
	return RT_OK;
}

int8_t rsmuGetPLLState(void) 
{
#ifdef ACCESS_SIM
	return FREERUN;
#endif
	RSMU_PLL_STATE get;

	memset(&get, 0, sizeof(get));
	get.dpll = RSMU_PLL_IDX;

	if(ioctl(g_rsmu_fd, RSMU_GET_STATE, &get))
	{
		zlog_err("%s ioctl faild", RSMU_DEVICE_NAME);
		return RT_NOK;
	}

	return get.state;
}

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
