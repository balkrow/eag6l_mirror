#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <zebra.h>
#include "sysmon.h"

int16_t chk_file_from_dir(const char * prefix, uint16_t *fpga_ver, char *img_name) 
{
	DIR *dir;
	struct dirent *entry;
	uint8_t major, minor; 
	int8_t rc = RT_NOK;
	char *p;

	dir = opendir(FPGA_IMG_DIR);
	if (dir == NULL) 
	{
		/*TODO: log*/
		goto fail;	
	}

	while ((entry = readdir(dir)) != NULL) 
	{
		if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
			continue;
		if (entry->d_name[0] == 0) continue;

		if (strstr(entry->d_name, prefix) !=  NULL)
		{
			p = entry->d_name + strlen(prefix);
			major = ((int8_t)*p) - 48;
			p = p+2;
			minor = ((int8_t)*p) - 48;

			*fpga_ver = ((major << 4) & 0xf0) | minor;
			strncpy(img_name, entry->d_name, FPGA_IMG_LEN);
#ifdef DEBUG 
			zlog_notice("img %s %x %d %d", img_name, *fpga_ver, major, minor);			
#endif
			rc = RT_OK;
			break;
		}
	}

fail:
	closedir(dir);

	return rc;
}

#if 0
uint16_t str_to_16bitInt(char * const str, uint16_t *value)
{
	ulong temp;

	errno = 0; // 초기화
	temp = simple_strtoul(str, NULL, 10);

	// 변환 오류 확인
	if (errno != 0)
	   return 0; // 변환 실패
	
	if (temp != 1 && temp != 2)
	   return 0; // 범위 초과

	*value = (uint16_t)temp; 
	return 1;
}
#endif

uint8_t get_os_bank(void)
{

	char szBuf[512] = {0, };
	char bank;
	FILE * fd;
	char *chr_p;
	uint8_t ret = 0;


	fd = fopen("/proc/cmdline", "r");
	if(!fd)
	{
		goto fail;
	}

	fgets(szBuf, 512, fd);

	chr_p = NULL;
	chr_p = strstr(szBuf, "bank=");

	if(chr_p != NULL )
	{
		chr_p += strlen("bank=");
		bank = *chr_p++; 

		if(bank == '1')
			ret = 1;
		else if(bank == '2')
			ret = 2;
	}
fail:
	if(fd)
		fclose(fd);
	return ret;

}

uint8_t get_fpga_bank(void)
{

	char szBuf[512] = {0, };
	char bank;
	FILE * fd;
	char *chr_p;
	uint8_t ret = 0;


	fd = fopen("/proc/cmdline", "r");
	if(!fd)
	{
		goto fail;
	}

	fgets(szBuf, 512, fd);

	chr_p = NULL;
	chr_p = strstr(szBuf, "fpga_bank=");

	if(chr_p != NULL )
	{
		chr_p += strlen("fpga_bank=");
		bank = *chr_p++; 

		if(bank == '1')
			ret = 1;
		else if(bank == '2')
			ret = 2;
	}
fail:
	if(fd)
		fclose(fd);
	return ret;

}
