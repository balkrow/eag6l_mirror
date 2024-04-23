
#include <common.h>
#include <command.h>
#include <image.h>



int do_brp(cmd_tbl_t * cmdtp, int flag, int argc, char * const argv[])
{

	int rc = 1,i;
	char buffer[100] = {0,};
	int getsize =0;
	char uboot_crc[8];
	ulong crc_num = 0;
	ulong check_sum = 0;


	if (argc  > 2) {
		printf ("Usage:\n%s\n", cmdtp->usage);
		return -1;
	}

	if (argc < 2) {
		

		(unsigned short)fpga_memory(0x02,0x3A,0xA5A5);
		sprintf (buffer,"tftpboot 1000000 %s",getenv("bootromname"));

		rc =      run_command(buffer, flag);

		if ( rc > 0 ){	
			sprintf(buffer,"0x%s",getenv("filesize"));
			getsize = simple_strtoul(buffer, NULL, 10);
						
				run_command("protect off bank 1 ", flag);
				printf("flash erase start");
				(unsigned short)fpga_memory(0x02,0x3A,0xA5A5);
				run_command("erase bank 1", flag);
				(unsigned short)fpga_memory(0x02,0x3A,0xA5A5);
				sprintf (buffer,"cp.b 1000000 0xfe000000 0x%x ",getsize);
				run_command(buffer,flag);

				run_command("protect on bank 1 ", flag);	
			}
	}

	if (argc == 2) {
		(unsigned short)fpga_memory(0x02,0x3A,0xA5A5);
		sprintf (buffer,"tftpboot 1000000 %s ",argv[1]);
		rc =     run_command(buffer, flag);

		if ( rc > 0 ){ 
			sprintf(buffer,"0x%s",getenv("filesize"));
			getsize = simple_strtoul(buffer, NULL, 10);
						
				run_command("protect off bank 1 ", flag);
				(unsigned short)fpga_memory(0x02,0x3A,0xA5A5);
				printf("flash erase start");
				run_command("erase bank 1", flag);
				(unsigned short)fpga_memory(0x02,0x3A,0xA5A5);
				sprintf (buffer,"cp.b 1000000 0xfe000000 0x%x ",getsize);
				run_command(buffer,flag);

				run_command("protect on bank 1 ", flag);	

			}		
	}
	return 0;
}     


U_BOOT_CMD(
		brp,	2,	1,	do_brp,
		"warning !!! boot rom programing",
		"[imagename] - warning !!! boot rom image upgrade\n"
		);



