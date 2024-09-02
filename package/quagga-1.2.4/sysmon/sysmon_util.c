#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <zebra.h>
#include "sysmon.h"
#if 1/*[#110] RDL function Debugging 및 수정, balkrow, 2024-08-30*/
#include "bp_regs.h"
#include "rdl_fsm.h"
#define SWAP_BYTE(x) x >> 8 & 0xff | (x  << 8) & 0xff00

int8_t get_pkg_header(uint8_t bank, fw_image_header_t *header)
{
	int8_t rc = RT_NOK;
	char pkg_file_path[256] = {0, };
	char *dir_path;
	DIR *dir;
	struct dirent *entry;

	if(bank == RDL_BANK_1)
	{
		if((dir = opendir(RDL_INSTALL1_PATH)) == NULL)
			goto fail;
		dir_path = RDL_INSTALL1_PATH;
	}
	else if(bank == RDL_BANK_2)
	{
		if((dir = opendir(RDL_INSTALL2_PATH)) == NULL)
			goto fail;
		dir_path = RDL_INSTALL2_PATH;
	}
	else
		goto fail;

	while ((entry = readdir(dir)) != NULL) 
	{
		if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
			continue;
		if (entry->d_name[0] == 0) continue;

		if (strstr(entry->d_name, PKG_FILE_PREFIX) !=  NULL)
		{
			int fd;
			char file_name[256] = {0, };
			sprintf(file_name, "%s%s", dir_path, entry->d_name);

			if((fd = open(file_name, O_RDONLY)) < 0) 
			{
				zlog_notice("%s open err %s", file_name, strerror(errno));
				goto fail;
			}

			if(read(fd, header, sizeof(fw_image_header_t)) > 0) 
			{
				zlog_notice("file name=%s", entry->d_name);
				close(fd);
				rc = RT_OK;
				break;
			}
			else
			{
				close(fd);
				goto fail;
			}
		}
	}



fail: 
	return rc;

}

int8_t write_pkg_header(uint8_t bank, fw_image_header_t *header)
{
	int n;
	uint16_t msb, lsb;
	/*Magic Num*/
	/*LSB*/
	lsb = (header->fih_magic >> 16) & 0xffff;
	msb = header->fih_magic & 0xffff;
	DPRAM_WRITE(BANK1_MAGIC_NO_1_ADDR + ((bank - 1) * 0x1000), SWAP_BYTE(msb));
	DPRAM_WRITE(BANK1_MAGIC_NO_1_ADDR + 2 + ((bank - 1) * 0x1000), SWAP_BYTE(lsb));
	/*Header CRC*/
	lsb = (header->fih_hcrc >> 16) & 0xffff;
	msb = header->fih_hcrc & 0xffff;
	DPRAM_WRITE((BANK1_MAGIC_NO_1_ADDR + 4) + ((bank - 1) * 0x1000), SWAP_BYTE(msb));
	DPRAM_WRITE((BANK1_MAGIC_NO_1_ADDR + 6) + ((bank - 1) * 0x1000), SWAP_BYTE(lsb));
	/*Build time*/
	lsb = (header->fih_time >> 16) & 0xffff;
	msb = header->fih_time & 0xffff;
	DPRAM_WRITE((BANK1_MAGIC_NO_1_ADDR + 8) + ((bank - 1) * 0x1000), SWAP_BYTE(msb));
	DPRAM_WRITE((BANK1_MAGIC_NO_1_ADDR + 10) + ((bank - 1) * 0x1000), SWAP_BYTE(lsb));
	/*total_size*/
	lsb = (header->fih_size >> 16) & 0xffff;
	msb = header->fih_size & 0xffff;
	DPRAM_WRITE((BANK1_MAGIC_NO_1_ADDR + 0xC) + ((bank - 1) * 0x1000), SWAP_BYTE(msb));  
	DPRAM_WRITE((BANK1_MAGIC_NO_1_ADDR + 0xE) + ((bank - 1) * 0x1000), SWAP_BYTE(lsb));
	/*Card Type*/
	lsb = (header->fih_card_type >> 16) & 0xffff;
	msb = header->fih_card_type & 0xffff;
	DPRAM_WRITE((BANK1_MAGIC_NO_1_ADDR + 0x10) + ((bank - 1) * 0x1000), SWAP_BYTE(msb)); 
	DPRAM_WRITE((BANK1_MAGIC_NO_1_ADDR + 0x12) + ((bank - 1) * 0x1000), SWAP_BYTE(lsb));
	/*Total CRC*/
	lsb = (header->fih_dcrc >> 16) & 0xffff;
	msb = header->fih_dcrc & 0xffff;
	DPRAM_WRITE((BANK1_MAGIC_NO_1_ADDR + 0x14) + ((bank - 1) * 0x1000), SWAP_BYTE(msb)); 
	DPRAM_WRITE((BANK1_MAGIC_NO_1_ADDR + 0x14) + ((bank - 1) * 0x1000), SWAP_BYTE(lsb));

	/*Version String*/
	for(n = 0; n < RDL_VER_STR_MAX; n += 2)
	{
		uint16_t w_val;
		w_val = (header->fih_ver[n] << 8) | header->fih_ver[n + 1];
		DPRAM_WRITE(BANK1_VER_STR_START_ADDR + n, w_val); 
	}

	/*name String*/
	for(n = 0; n < RDL_FILE_NAME_MAX; n += 2)
	{
		uint16_t w_val;
		w_val = (header->fih_name[n] << 8) | header->fih_name[n + 1];
		DPRAM_WRITE(BANK1_FILE_NAME_START_ADDR + n, w_val); 
	}
	return RT_OK;
}

#endif

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
