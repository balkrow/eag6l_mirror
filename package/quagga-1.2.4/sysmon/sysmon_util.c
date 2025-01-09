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

#if 1/*[#110] RDL function Debugging 및 수정, balkrow, 2024-09-02*/
int8_t clear_bank(uint8_t bank)
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
		{
			char file_name[256] = {0, };
			sprintf(file_name, "%s%s", dir_path, entry->d_name);
			if(unlink(file_name) == -1)
				zlog_notice("unlink error %s %s", file_name, strerror(errno));
		}
	}
fail: 
	if(dir != NULL)	
		closedir(dir);
	return rc;
}

uint8_t is_pkg_zip(uint8_t bank)
{
	uint8_t rc = PKG_ERROR;
	char pkg_file_path[256] = {0, };
	const char zip_magic[4] = {0x50, 0x4B, 0x03, 0x04};
	char file_magic[4] = {0, };
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
#ifdef SELF_TEST
			{
				extern int restore_pkg_file(char *src, char *dst);
				restore_pkg_file("/mnt/flash/bank1/eag6l-os-v99.0.1.bin", "");
			}
#endif

			if((fd = open(file_name, O_RDONLY)) < 0) 
			{
				zlog_notice("%s open err %s", file_name, strerror(errno));
				goto fail;
			}

			lseek(fd, sizeof(fw_image_header_t), SEEK_SET);
			read(fd, file_magic, 4);	
			if(memcmp(file_magic, zip_magic, 4))
				rc = PKG_NONZIP;
			else
				rc = PKG_ZIP;
			close(fd);
			break;
		}
	}

fail: 
#if 1/*[#110] RDL function Debugging 및 수정, balkrow, 2024-09-02*/
	if(dir != NULL)	
		closedir(dir);
#endif
	return rc;

}

#endif

uint8_t get_pkg_header(uint8_t bank, fw_image_header_t *header)
{
	uint8_t rc = RT_NOK;
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
#ifdef SELF_TEST
			{
				extern int restore_pkg_file(char *src, char *dst);
				restore_pkg_file("/mnt/flash/bank1/eag6l-os-v99.0.1.bin", "");
			}
#endif

			if((fd = open(file_name, O_RDONLY)) < 0) 
			{
				zlog_notice("%s open err %s", file_name, strerror(errno));
				goto fail;
			}

			if(read(fd, header, sizeof(fw_image_header_t)) > 0) 
			{
#if 1 /* [#240] Fixing for checking OS pkg size for bank info, dustin, 2025-01-08 */
				unsigned int lsize;
				/* get real file size. */
				lsize = lseek(fd, 0, SEEK_END);
				lsize -= sizeof(fw_image_header_t);
				/* check file size. */
				if(ntohl(header->fih_size) != lsize) {
					zlog_notice("Invalid OS file size : %d vs %d.",
						ntohl(header->fih_size), lsize);
					close(fd);
					goto fail;
				}
#endif/* [#240] */
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
#if 1/*[#110] RDL function Debugging 및 수정, balkrow, 2024-09-02*/
	if(dir != NULL)	
		closedir(dir);
#endif
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

#if 1 /* [#110] RDL function Debugging ¿ ¿¿, balkrow, 2024-09-02 */
	/*Version String*/
	for(n = 0; n < RDL_VER_STR_MAX; n += 2)
	{
		uint16_t w_val;
		w_val = (header->fih_ver[n] << 8) | header->fih_ver[n + 1];
		DPRAM_WRITE((BANK1_VER_STR_START_ADDR + ((bank - 1) * 0x1000)) + n, w_val); 
	}

	/*name String*/
	for(n = 0; n < RDL_FILE_NAME_MAX; n += 2)
	{
		uint16_t w_val;
		w_val = (header->fih_name[n] << 8) | header->fih_name[n + 1];
		DPRAM_WRITE((BANK1_FILE_NAME_START_ADDR + ((bank - 1) * 0x1000)) + n, w_val); 
	}
#endif
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

#if 1/*[#110] RDL function Debugging 및 수정, balkrow, 2024-09-04*/ 
int16_t get_fpga_filename(const char * prefix, char * img_dir, char *img_name) 
{
	DIR *dir;
	struct dirent *entry;
	int8_t rc = RT_NOK;

	dir = opendir(img_dir);
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
			sprintf(img_name, "%s%s", img_dir, entry->d_name);
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

int16_t get_os_filename(const char * prefix, char * img_dir, char *img_name) 
{
	DIR *dir;
	struct dirent *entry;
	int8_t rc = RT_NOK;

	dir = opendir(img_dir);
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
			sprintf(img_name, "%s", entry->d_name);
			rc = RT_OK;
			break;
		}
	}

fail:
	closedir(dir);

	return rc;
}


int16_t activate_os(uint8_t bank) 
{
	/*move os image from package bank to os bank*/
	char cmd[512] = {0, }; 
	char os_img[256] = {0, }; 
#if 1/*[#210] bank switch 시 기존 image 삭제 로직 구현, balkrow, 2024-11-21*/
	char uImage_orig[256] = {0, }; 
#endif
	char *bank_path = NULL;
	char *os_bank_path = NULL;
	char *uImage_path = NULL;
	int32_t result;
	int16_t rc;

	if(bank == RDL_BANK_1)
	{
		bank_path = RDL_INSTALL1_PATH; 
		os_bank_path = RDL_B1_PATH;
		uImage_path = RDL_B1_LINK_PATH;
	}
	else if(bank == RDL_BANK_2)
	{
		bank_path = RDL_INSTALL2_PATH; 
		os_bank_path = RDL_B2_PATH;
		uImage_path = RDL_B2_LINK_PATH;
	}

	if(get_os_filename(OS_IMG_PREFIX, bank_path, os_img) == RT_NOK)
		return RT_NOK; 

#if 1/*[#210] bank switch 시 기존 image 삭제 로직 구현, balkrow, 2024-11-21*/
	if(readlink(uImage_path, uImage_orig, 256) > 0)
	{
		int ret;
		memset(cmd, 0, 512); 
		sprintf(cmd, "%s%s", os_bank_path, uImage_orig);
		ret = unlink(cmd);
		zlog_notice("delete previous img %s[%d]", cmd, ret);
	}
#endif

	sprintf(cmd, "cp %s%s %s", bank_path, os_img, os_bank_path);
	result = system(cmd);
	zlog_notice("cmd %s result=%d", cmd, result);
	/*remove uImage symbolic link*/


	memset(cmd, 0, 512); 
	sprintf(cmd, "rm %s", uImage_path);
	result = system(cmd);
	zlog_notice("cmd %s result=%d", cmd, result);
	/*create uImage symbolic link*/
	if(chdir(os_bank_path))
	{
		zlog_notice("directory %s change failed", os_bank_path);
		return RT_NOK; 
	}

	memset(cmd, 0, 512); 
	sprintf(cmd, "ln -sf %s uImage", os_img);
	if(system(cmd))
	{
		zlog_notice("cmd %s failed", cmd);
		return RT_NOK; 
	}
	zlog_notice("cmd %s result=%d", cmd, result);
	memset(cmd, 0, 512); 
	sprintf(cmd, "sync");
	system(cmd);

	return RT_OK; 
}
#endif

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
	chr_p = strstr(szBuf, "bootbank=bank");

	if(chr_p != NULL )
	{
		chr_p += strlen("bootbank=bank");
		bank = *chr_p; 
		zlog_notice("bank %c", bank);
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
