#include "command.h"
#include <ctype.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include "hdriver.h"
#include <readline/readline.h>
#include <readline/history.h>

#if 1/* [#78] Adding system inventory management, dustin, 2024-07-24 */
#include "sysmon.h"
#endif

typedef struct sysd_slot_inventory_info_s
{
	int slot;
	board_inventory_t bdinvent;
} sysd_slot_inventory_info_t;

//char *console_buffer = NULL;

#if 1/* [#78] Adding system inventory management, dustin, 2024-07-24 */
int set_eag6l_inventory(struct vty *vty)
{
	FILE *fp;
	char *console_buffer = NULL;
	char message[1024];
	char tmpbuffer[1024];
	unsigned int temp;
	int len;
	char *endptr = NULL;
	board_inventory_t bdinvent;

	memset(&bdinvent, 0, sizeof bdinvent);

	while(1) {
		fp = fopen(EAG6L_INVENTORY_FILE, "r");
		if(fp)
		{
			fread(&bdinvent, 1, sizeof(board_inventory_t)-16, fp);
			fclose(fp);
		}

		if(bdinvent.manufacturer[9] != '\0')		bdinvent.manufacturer[0] = '\0';
		if(bdinvent.model_name[9] != '\0')		bdinvent.model_name[0] ='\0';
		if(bdinvent.part_number[15] != '\0')    bdinvent.part_number[0] = '\0';
		if(bdinvent.serial_number[15] != '\0')		bdinvent.serial_number[0] = '\0';
		if(bdinvent.manufacture_date[9] != '\0')	bdinvent.manufacture_date[0] = '\0';
		if(bdinvent.repair_date[9] != '\0')		bdinvent.repair_date[0] = '\0';
        if(bdinvent.clei_number[10] != '\0')     bdinvent.clei_number[0] = '\0';
        if(bdinvent.usi_number[25] != '\0')     bdinvent.usi_number[0] = '\0';
        if(bdinvent.cfiu_serial[15] != '\0')    bdinvent.cfiu_serial[0] = '\0';

        vty_out (vty,"0: MANUFACTURE        : %s \n",bdinvent.manufacturer);
        vty_out (vty,"1: MODEL NAME         : %s \n",bdinvent.model_name);
        vty_out (vty,"2: PART NUMBER        : %s \n",bdinvent.part_number);
        vty_out (vty,"3: SERIAL NUMBER      : %s \n",bdinvent.serial_number);
        vty_out (vty,"4: REVISION           : 0x%x \n",bdinvent.revision);
        vty_out (vty,"5: MANUFACTURE DATE   : %s \n",bdinvent.manufacture_date);
        vty_out (vty,"6: REPAIR DATE        : %s \n",bdinvent.repair_date);
        vty_out (vty,"7: REPAIR CODE        : 0x%x \n",bdinvent.repair_code);
        vty_out (vty,"8: CFIU SERIAL NUMBER : %s \n",bdinvent.cfiu_serial);
        vty_out (vty,"q: quit \n");
#if 0
		vty_out (vty,"8: CLEI NUMBER		: %s \n",bdinvent.clei_number);
		vty_out (vty,"9: USI NUMBER			: %s \n",bdinvent.usi_number);
#endif

		sprintf(message,"\n\nchoose number?:");   
		console_buffer  = readline(message);
		len = strlen(console_buffer);
		if(!strcmp(console_buffer,"")){
			break ;
		}
        else if (!strcmp(console_buffer,"q")){
            //slot_inventory_read(slot,slinvent,type);
#if 0
            slinvent->slot = 25;
            memcpy(&slinvent->bdinvent,&bdinvent,sizeof(board_inventory_t));
            send_to_sysd_fifo_slot_inventory_update(slinvent);
#endif
            break;
        }
		else if (!strcmp(console_buffer,"0")){					
			sprintf (message, "MANUFACTURE  (%d bytes):[%s]\t:",9,bdinvent.manufacturer);
			console_buffer  = readline(message);
			len = strlen(console_buffer);
			if(!strcmp(console_buffer,"")){
				continue ;
			}
			strncpy(bdinvent.manufacturer,console_buffer,9);
			bdinvent.manufacturer[9] = '\0';
			fp = fopen(EAG6L_INVENTORY_FILE, "w");
			fwrite(&bdinvent,1,sizeof(board_inventory_t)-16,fp );
			fclose(fp);
			// update to boot env
			sprintf(tmpbuffer, "fw_setenv manufacture %s", bdinvent.manufacturer);
			system(tmpbuffer);
		}
		else if (!strcmp(console_buffer,"1")){
			sprintf (message, "MODEL NAME  (%d bytes):[%s]\t:",9,bdinvent.model_name);
			console_buffer  = readline(message);
			len = strlen(console_buffer);
			if(!strcmp(console_buffer,"")){
				continue ;
			}
			strncpy(bdinvent.model_name,console_buffer,9);
			bdinvent.model_name[9] = '\0';
			fp = fopen(EAG6L_INVENTORY_FILE, "w");
			fwrite(&bdinvent,1,sizeof(board_inventory_t)-16,fp );
			fclose(fp);
			// update to boot env
			sprintf(tmpbuffer, "fw_setenv model_name %s", bdinvent.model_name);
			system(tmpbuffer);
		}
		else if (!strcmp(console_buffer,"2")){
			sprintf (message, "PART NUMBER  (%d bytes):[%s]\t:",15,bdinvent.part_number);
			console_buffer  = readline(message);
			len = strlen(console_buffer);
			if(!strcmp(console_buffer,"")){
				continue ;
			}
			strncpy(bdinvent.part_number,console_buffer,15);
			bdinvent.part_number[15] = '\0';
			fp = fopen(EAG6L_INVENTORY_FILE, "w");
			fwrite(&bdinvent,1,sizeof(board_inventory_t)-16,fp );
			fclose(fp);
			// update to boot env
			sprintf(tmpbuffer, "fw_setenv part_number %s", bdinvent.part_number);
			system(tmpbuffer);
		}
		else if (!strcmp(console_buffer,"3")){
			sprintf (message, "SERIAL NUMBER  (%d bytes):[%s]\t:",15,bdinvent.serial_number);
			console_buffer  = readline(message);
			len = strlen(console_buffer);
			if(!strcmp(console_buffer,"")){
				continue ;
			}
			strncpy(bdinvent.serial_number,console_buffer,15);
			bdinvent.serial_number[15] = '\0';
			fp = fopen(EAG6L_INVENTORY_FILE, "w");
			fwrite(&bdinvent,1,sizeof(board_inventory_t)-16,fp );
			fclose(fp);
			// update to boot env
			sprintf(tmpbuffer, "fw_setenv serial_number %s", bdinvent.serial_number);
			system(tmpbuffer);
		}
		else if (!strcmp(console_buffer,"4")){
			sprintf (message, "REVISION  (%d bytes):[0x%x]\t:",4,bdinvent.revision);
			console_buffer  = readline(message);
			len = strlen(console_buffer);
			if(!strcmp(console_buffer,"")){
				continue ;
			}

			bdinvent.revision = strtoul(console_buffer, &endptr, 16);		
			fp = fopen(EAG6L_INVENTORY_FILE, "w");
			fwrite(&bdinvent,1,sizeof(board_inventory_t)-16,fp );
			fclose(fp);
			// update to boot env
			sprintf(tmpbuffer, "fw_setenv revision %x", bdinvent.revision);
			system(tmpbuffer);
		}
		else if (!strcmp(console_buffer,"5")){
			sprintf (message, "MANUFACTURE DATE  (%d bytes):[%s]\t:",9,bdinvent.manufacture_date);
			console_buffer  = readline(message);
			len = strlen(console_buffer);
			if(!strcmp(console_buffer,"")){
				continue ;
			}

			strncpy(bdinvent.manufacture_date,console_buffer,9);
			bdinvent.manufacture_date[9] = '\0';	
			fp = fopen(EAG6L_INVENTORY_FILE, "w");
			fwrite(&bdinvent,1,sizeof(board_inventory_t)-16,fp );
			fclose(fp);
			// update to boot env
			sprintf(tmpbuffer, "fw_setenv manufacture_date %s", bdinvent.manufacture_date);
			system(tmpbuffer);
		}
		else if (!strcmp(console_buffer,"6")){
			sprintf (message, "REPAIR DATE  (%d bytes):[%s]\t:",9,bdinvent.repair_date);
			console_buffer  = readline(message);
			len = strlen(console_buffer);
			if(!strcmp(console_buffer,"")){
				continue ;
			}

			strncpy(bdinvent.repair_date,console_buffer,9);
			bdinvent.repair_date[9] = '\0';	
			fp = fopen(EAG6L_INVENTORY_FILE, "w");
			fwrite(&bdinvent,1,sizeof(board_inventory_t)-16,fp );
			fclose(fp);
			// update to boot env
			sprintf(tmpbuffer, "fw_setenv repair_date %s", bdinvent.repair_date);
			system(tmpbuffer);
		}
		else if (!strcmp(console_buffer,"7")){
			sprintf (message, "REPAIR CODE  (%d bytes):[0x%x]\t:",4,bdinvent.repair_code);
			console_buffer  = readline(message);
			len = strlen(console_buffer);
			if(!strcmp(console_buffer,"")){
				continue ;
			}

			bdinvent.repair_code = strtoul(console_buffer, &endptr, 16);		
			fp = fopen(EAG6L_INVENTORY_FILE, "w");
			fwrite(&bdinvent,1,sizeof(board_inventory_t)-16,fp );
			fclose(fp);
			// update to boot env
			sprintf(tmpbuffer, "fw_setenv repair_code %x", bdinvent.repair_code);
			system(tmpbuffer);
		}
#if 0
		else if (!strcmp(console_buffer,"8")){
            sprintf (message, "CFIU SERIAL NUMBER (%d bytes):[%s]\t:",15,bdinvent.cfiu_serial);
            console_buffer  = readline(message);
            len = strlen(console_buffer);
            if(!strcmp(console_buffer,"")){
                continue ;
            }

            strncpy(bdinvent.cfiu_serial,console_buffer,15);
            bdinvent.cfiu_serial[15] = '\0';
            fp = fopen(EAG6L_INVENTORY_FILE, "w");
			fwrite(bdinvent.cfiu_serial,1,16,fp);
            fclose(fp);
			// update to boot env
			sprintf(tmpbuffer, "fw_setenv cfiu_serial %x", bdinvent.repair_code);
			system(tmpbuffer);
        }
        else if (!strcmp(console_buffer,"8")){
            sprintf (message, "CLEI NUMBER  (%d bytes):[%s]\t:",10,bdinvent.clei_number);
            console_buffer  = readline(message);
            len = strlen(console_buffer);
            if(!strcmp(console_buffer,"")){
                continue ;
            }

            strncpy(bdinvent.clei_number,console_buffer,10);
            bdinvent.clei_number[10] = '\0';
            fp = fopen(EAG6L_INVENTORY_FILE, "w");
            fwrite(&bdinvent,1,sizeof(board_inventory_t),fp );
            fclose(fp);
        }
        else if (!strcmp(console_buffer,"9")){
            sprintf (message, "USI NUMBER  (%d bytes):[%s]\t:",25,bdinvent.usi_number);
            console_buffer  = readline(message);
            len = strlen(console_buffer);
            if(!strcmp(console_buffer,"")){
                continue ;
            }

            strncpy(bdinvent.usi_number,console_buffer,25);
            bdinvent.usi_number[25] = '\0';
            fp = fopen(EAG6L_INVENTORY_FILE, "w");
            fwrite(&bdinvent,1,sizeof(board_inventory_t),fp );
            fclose(fp);
        }
#endif
		else if (!strcmp(console_buffer,"q"))
			break;	
		else
			continue;
	}   
}
#else/////////////////////////////////////////////////////////////////////
unsigned short sys_util_slot_memory_read(int slot,unsigned int addr)
{
	slotmemory_t slotmemory;
	int gebd_fd;
	gebd_fd = open("/dev/hdrv",O_RDWR);
	if ( gebd_fd < 0 ) {
		return 0;
	}	

	slotmemory.addr = addr;slotmemory.value=0;
	ioctl(gebd_fd, HDRIVER_IOCG_SLOT_SHOW_MEMORY, &slotmemory);
	close(gebd_fd);
	return slotmemory.value;

}


unsigned short sys_util_slot_memory_write(int slot,unsigned int addr,unsigned short writeval)
{
	slotmemory_t slotmemory;
	int gebd_fd;
	gebd_fd = open("/dev/hdrv",O_RDWR);
	if ( gebd_fd < 0 ) {
		return 0;
	}	
	slotmemory.addr = addr;slotmemory.value=writeval;
	ioctl(gebd_fd, HDRIVER_IOCS_SLOT_WRITE_MEMORY, &slotmemory);
	close(gebd_fd);		
	return slotmemory.value;

}

unsigned short sys_util_fpga_memory_read(unsigned int addr)
{
	fpgamemory_t fpgamemory;
	int gebd_fd;
	gebd_fd = open("/dev/hdrv",O_RDWR);
	if ( gebd_fd < 0 ) {
		return 0;
	}		

	fpgamemory.addr = addr;fpgamemory.type=HDRIVER_MEMORY_TYPE_READ;fpgamemory.value=0;
	ioctl(gebd_fd, HDRIVER_IOCG_FPGA_SHOW_MEMORY, &fpgamemory);
	close(gebd_fd); 

	return fpgamemory.value;

}


unsigned short sys_util_fpga_memory_write(unsigned int addr,unsigned short writeval)
{
	fpgamemory_t fpgamemory;
	int gebd_fd;
	gebd_fd = open("/dev/hdrv",O_RDWR);
	if ( gebd_fd < 0 ) {
		return 0;
	}		
	fpgamemory.addr = addr;fpgamemory.type=HDRIVER_MEMORY_TYPE_WRITE;fpgamemory.value=writeval;
	ioctl(gebd_fd, HDRIVER_IOCS_FPGA_WRITE_MEMORY, &fpgamemory);
	close(gebd_fd);
	return fpgamemory.value;

}

int set_mcu_inventory(struct vty *vty)
{
	FILE *fp, *fp2;
	char *console_buffer = NULL;
	char message[1024];
	char tmpbuffer[1024];
	unsigned int temp;
	int len;
	char *endptr = NULL;
	board_inventory_t bdinvent;
    sysd_slot_inventory_info_t slotinvent;
    sysd_slot_inventory_info_t *slinvent = &slotinvent;

	while (1){
		fp = fopen("/sys/bus/i2c/drivers/at24/0-0052/eeprom", "r");
		if(fp)
		{
			fread(&bdinvent,1,sizeof(board_inventory_t)-16,fp );
			fclose(fp);
		}

		fp2 = fopen("/sys/bus/i2c/drivers/at24/31-1054/eeprom", "r");
		if(fp2)
		{
			fread(bdinvent.cfiu_serial,1,16,fp2);
			fclose(fp2);
		}

		if(bdinvent.manufacturer[9] != '\0')		bdinvent.manufacturer[0] = '\0';
		if(bdinvent.model_name[9] != '\0')		bdinvent.model_name[0] ='\0';
		if(bdinvent.part_number[15] != '\0')    bdinvent.part_number[0] = '\0';
		if(bdinvent.serial_number[15] != '\0')		bdinvent.serial_number[0] = '\0';
		if(bdinvent.manufacture_date[9] != '\0')	bdinvent.manufacture_date[0] = '\0';
		if(bdinvent.repair_date[9] != '\0')		bdinvent.repair_date[0] = '\0';
        if(bdinvent.clei_number[10] != '\0')     bdinvent.clei_number[0] = '\0';
        if(bdinvent.usi_number[25] != '\0')     bdinvent.usi_number[0] = '\0';
        if(bdinvent.cfiu_serial[15] != '\0')    bdinvent.cfiu_serial[0] = '\0';

        vty_out (vty,"0: MANUFACTURE        : %s \n",bdinvent.manufacturer);
        vty_out (vty,"1: MODEL NAME         : %s \n",bdinvent.model_name);
        vty_out (vty,"2: PART NUMBER        : %s \n",bdinvent.part_number);
        vty_out (vty,"3: SERIAL NUMBER      : %s \n",bdinvent.serial_number);
        vty_out (vty,"4: REVISION           : 0x%x \n",bdinvent.revision);
        vty_out (vty,"5: MANUFACTURE DATE   : %s \n",bdinvent.manufacture_date);
        vty_out (vty,"6: REPAIR DATE        : %s \n",bdinvent.repair_date);
        vty_out (vty,"7: REPAIR CODE        : 0x%x \n",bdinvent.repair_code);
        vty_out (vty,"8: CFIU SERIAL NUMBER : %s \n",bdinvent.cfiu_serial);
        vty_out (vty,"q: quit \n");
#if 0
		vty_out (vty,"8: CLEI NUMBER		: %s \n",bdinvent.clei_number);
		vty_out (vty,"9: USI NUMBER			: %s \n",bdinvent.usi_number);
#endif

		sprintf(message,"\n\nchoose number?:");   
		console_buffer  = readline(message);
		len = strlen(console_buffer);
		if(!strcmp(console_buffer,"")){
			break ;
		}
        else if (!strcmp(console_buffer,"q")){
            //slot_inventory_read(slot,slinvent,type);
#if 0
            slinvent->slot = 25;
            memcpy(&slinvent->bdinvent,&bdinvent,sizeof(board_inventory_t));
            send_to_sysd_fifo_slot_inventory_update(slinvent);
#endif
            break;
        }
		else if (!strcmp(console_buffer,"0")){					
			sprintf (message, "MANUFACTURE  (%d bytes):[%s]\t:",9,bdinvent.manufacturer);
			console_buffer  = readline(message);
			len = strlen(console_buffer);
			if(!strcmp(console_buffer,"")){
				continue ;
			}
			strncpy(bdinvent.manufacturer,console_buffer,9);
			bdinvent.manufacturer[9] = '\0';
			fp = fopen("/sys/bus/i2c/drivers/at24/0-0052/eeprom", "w");
			fwrite(&bdinvent,1,sizeof(board_inventory_t)-16,fp );
			fclose(fp);
		}
		else if (!strcmp(console_buffer,"1")){
			sprintf (message, "MODEL NAME  (%d bytes):[%s]\t:",9,bdinvent.model_name);
			console_buffer  = readline(message);
			len = strlen(console_buffer);
			if(!strcmp(console_buffer,"")){
				continue ;
			}
			strncpy(bdinvent.model_name,console_buffer,9);
			bdinvent.model_name[9] = '\0';
			fp = fopen("/sys/bus/i2c/drivers/at24/0-0052/eeprom", "w");
			fwrite(&bdinvent,1,sizeof(board_inventory_t)-16,fp );
			fclose(fp);
		}
		else if (!strcmp(console_buffer,"2")){
			sprintf (message, "PART NUMBER  (%d bytes):[%s]\t:",15,bdinvent.part_number);
			console_buffer  = readline(message);
			len = strlen(console_buffer);
			if(!strcmp(console_buffer,"")){
				continue ;
			}
			strncpy(bdinvent.part_number,console_buffer,15);
			bdinvent.part_number[15] = '\0';
			fp = fopen("/sys/bus/i2c/drivers/at24/0-0052/eeprom", "w");
			fwrite(&bdinvent,1,sizeof(board_inventory_t)-16,fp );
			fclose(fp);
		}
		else if (!strcmp(console_buffer,"3")){
			sprintf (message, "SERIAL NUMBER  (%d bytes):[%s]\t:",15,bdinvent.serial_number);
			console_buffer  = readline(message);
			len = strlen(console_buffer);
			if(!strcmp(console_buffer,"")){
				continue ;
			}
			strncpy(bdinvent.serial_number,console_buffer,15);
			bdinvent.serial_number[15] = '\0';
			fp = fopen("/sys/bus/i2c/drivers/at24/0-0052/eeprom", "w");
			fwrite(&bdinvent,1,sizeof(board_inventory_t)-16,fp );
			fclose(fp);
		}
		else if (!strcmp(console_buffer,"4")){
			sprintf (message, "REVISION  (%d bytes):[0x%x]\t:",4,bdinvent.revision);
			console_buffer  = readline(message);
			len = strlen(console_buffer);
			if(!strcmp(console_buffer,"")){
				continue ;
			}

			bdinvent.revision = strtoul(console_buffer, &endptr, 16);		
			fp = fopen("/sys/bus/i2c/drivers/at24/0-0052/eeprom", "w");
			fwrite(&bdinvent,1,sizeof(board_inventory_t)-16,fp );
			fclose(fp);
		}
		else if (!strcmp(console_buffer,"5")){
			sprintf (message, "MANUFACTURE DATE  (%d bytes):[%s]\t:",9,bdinvent.manufacture_date);
			console_buffer  = readline(message);
			len = strlen(console_buffer);
			if(!strcmp(console_buffer,"")){
				continue ;
			}

			strncpy(bdinvent.manufacture_date,console_buffer,9);
			bdinvent.manufacture_date[9] = '\0';	
			fp = fopen("/sys/bus/i2c/drivers/at24/0-0052/eeprom", "w");
			fwrite(&bdinvent,1,sizeof(board_inventory_t)-16,fp );
			fclose(fp);
		}
		else if (!strcmp(console_buffer,"6")){
			sprintf (message, "REPAIR DATE  (%d bytes):[%s]\t:",9,bdinvent.repair_date);
			console_buffer  = readline(message);
			len = strlen(console_buffer);
			if(!strcmp(console_buffer,"")){
				continue ;
			}

			strncpy(bdinvent.repair_date,console_buffer,9);
			bdinvent.repair_date[9] = '\0';	
			fp = fopen("/sys/bus/i2c/drivers/at24/0-0052/eeprom", "w");
			fwrite(&bdinvent,1,sizeof(board_inventory_t)-16,fp );
			fclose(fp);
		}
		else if (!strcmp(console_buffer,"7")){
			sprintf (message, "REPAIR CODE  (%d bytes):[0x%x]\t:",4,bdinvent.repair_code);
			console_buffer  = readline(message);
			len = strlen(console_buffer);
			if(!strcmp(console_buffer,"")){
				continue ;
			}

			bdinvent.repair_code = strtoul(console_buffer, &endptr, 16);		
			fp = fopen("/sys/bus/i2c/drivers/at24/0-0052/eeprom", "w");
			fwrite(&bdinvent,1,sizeof(board_inventory_t)-16,fp );
			fclose(fp);
		}
		else if (!strcmp(console_buffer,"8")){
            sprintf (message, "CFIU SERIAL NUMBER (%d bytes):[%s]\t:",15,bdinvent.cfiu_serial);
            console_buffer  = readline(message);
            len = strlen(console_buffer);
            if(!strcmp(console_buffer,"")){
                continue ;
            }

            strncpy(bdinvent.cfiu_serial,console_buffer,15);
            bdinvent.cfiu_serial[15] = '\0';
            fp2 = fopen("/sys/bus/i2c/drivers/at24/31-1054/eeprom", "w");
			fwrite(bdinvent.cfiu_serial,1,16,fp2);
            fclose(fp2);
        }
#if 0
        else if (!strcmp(console_buffer,"8")){
            sprintf (message, "CLEI NUMBER  (%d bytes):[%s]\t:",10,bdinvent.clei_number);
            console_buffer  = readline(message);
            len = strlen(console_buffer);
            if(!strcmp(console_buffer,"")){
                continue ;
            }

            strncpy(bdinvent.clei_number,console_buffer,10);
            bdinvent.clei_number[10] = '\0';
            fp = fopen("/sys/bus/i2c/drivers/at24/0-0051/eeprom", "w");
            fwrite(&bdinvent,1,sizeof(board_inventory_t),fp );
            fclose(fp);
        }
        else if (!strcmp(console_buffer,"9")){
            sprintf (message, "USI NUMBER  (%d bytes):[%s]\t:",25,bdinvent.usi_number);
            console_buffer  = readline(message);
            len = strlen(console_buffer);
            if(!strcmp(console_buffer,"")){
                continue ;
            }

            strncpy(bdinvent.usi_number,console_buffer,25);
            bdinvent.usi_number[25] = '\0';
            fp = fopen("/sys/bus/i2c/drivers/at24/0-0051/eeprom", "w");
            fwrite(&bdinvent,1,sizeof(board_inventory_t),fp );
            fclose(fp);
        }
#endif
		else if (!strcmp(console_buffer,"q"))
			break;	
		else
			continue;



	}   

}
#endif


DEFUN (dbg_setup,
       dbg_setup_cmd,
       "setup",
       "setup\n")
{
	char *console_buffer = NULL;
	char message[1024];
	int len;
	int slot = 0;
	char *endptr = NULL;
	unsigned char unit_fail = 0;
	int i;

	while(1){

		vty_out (vty,"\033[H\033[J");
		vty_out (vty,"EAG6L debug program!!!!");
		vty_out (vty,"\n\n");
		vty_out (vty,"*********************************\n");
		vty_out (vty,"Choose The Setup Type				\n");
		vty_out (vty,"0:       Factory init				\n");
#if 1/* [#78] Adding system inventory management, dustin, 2024-07-24 */
		vty_out (vty,"1:       EAG6L inventory          \n");
#else//////////////////////////////////////////////////////////////////
		vty_out (vty,"1:       OCIU/OFIU eeprom inventory       \n");
		vty_out (vty,"2:       BUZZER  \n");
#endif
		vty_out (vty,"q:       quit			 \n");
		vty_out (vty,"*********************************\n");
		sprintf(message,"\n\nchoose number?:");   


		console_buffer  = readline(message);
		len = strlen(console_buffer);

		if(!strcmp(console_buffer,""))
			continue;
		else if (!strcmp(console_buffer,"0"))
		{
			vty_out (vty,"not yet\n");
		}
		else if (!strcmp(console_buffer,"1"))
		{
#if 1/* [#78] Adding system inventory management, dustin, 2024-07-24 */
			set_eag6l_inventory(vty);
#else
			set_mcu_inventory(vty);
#endif
		}
#if 0/* [#78] Adding system inventory management, dustin, 2024-07-24 */
		else if (!strcmp(console_buffer,"2"))
		{
			sprintf (message, "buzzer on/off:[1:on,2:off]:");
			console_buffer  = readline(message);
			len = strlen(console_buffer);

			if(!strcmp(console_buffer,"")){
				continue;	 
			}
			else if (!strcmp(console_buffer,"1")){

				unsigned short buzdata;
				buzdata = sys_util_fpga_memory_read(0x1c);
				buzdata |= 0x40;
				sys_util_fpga_memory_write(0x1c,buzdata);
				printf("buzzer on!!\n");
				sleep(1);
			}
			else
			{
				unsigned short buzdata;
				buzdata = sys_util_fpga_memory_read(0x1c);
				buzdata &= ~0x40;
				sys_util_fpga_memory_write(0x1c,buzdata);
				printf("buzzer off!!\n");
				sleep(1);
			}
		}
#endif
		else if (!strcmp(console_buffer,"q"))
			return 0;		
		else
			continue;

	}		 

	return CMD_SUCCESS;
}


DEFUN (copy_uimage_factory_image,
		copy_uimage_factory_image_cmd,
		"copy uimage factory-image",
		"Copy from one file to another\n"
		"Copy from uImage\n"
		"Copy to factory image\n")
{

	char buf[512];
	char linkbuf[256];

	int result ;

	sprintf(buf,"ubiattach -m 3");
	result = system(buf);
	sleep(1);
	sprintf(buf,"mount -t ubifs ubi1:factory /mnt/nand");
	result = system(buf);
	if ( result != 0){
		vty_out(vty,"%%factory partition empty\n");
		sprintf(buf,"ubidetach -m 3");
		result = system(buf);
		sprintf(buf,"flash_eraseall  /dev/mtd3");
		result = system(buf);
		sprintf(buf,"ubiformat /dev/mtd3");
		result = system(buf);	
		sleep(1);
		sprintf(buf,"ubiattach -m 3");
		result = system(buf);
		sleep(1);
		sprintf(buf,"ubimkvol /dev/ubi1 -N factory -m");
		result = system(buf);
		sleep(1);
		sprintf(buf,"mount -t ubifs ubi1:factory /mnt/nand");
		result = system(buf);

	}
	vty_out(vty,"%%copy file\n");
	if ((result = readlink("/mnt/flash/boot/uImage",linkbuf , 256)) < 0)
	{
		vty_out(vty, "%% Can't read boot file: %s\n",strerror(errno));
		return CMD_WARNING;
	}
	linkbuf[result] = '\0';
	sprintf(buf,"rm -f /mnt/nand/*");
	result = system(buf);
	sprintf(buf,"cp -f /mnt/flash/boot/%s /mnt/nand/",linkbuf);
	result = system(buf);

	unlink("/mnt/nand/factory");
	sprintf(buf,"cd /mnt/nand");
	result = system(buf);
	sprintf(buf,"ln -sf %s /mnt/nand/factory",linkbuf);
	result = system(buf);
	sprintf(buf,"umount /mnt/nand");
	result = system(buf);
	sprintf(buf,"ubidetach -m 3");
	result = system(buf);

}


DEFUN (copy_filename_factory_image,
		copy_filename_factory_image_cmd,
		"copy FILENAME factory-image",
		"Copy from one file to another\n"
		"Copy from flash/boot/FILENAME\n"
		"Copy to factory image\n")
{

	char buf[512];
	char linkbuf[256];

	int result ;

	sprintf(buf,"ubiattach -m 3");
	result = system(buf);
	sleep(1);
	sprintf(buf,"mount -t ubifs ubi1:factory /mnt/nand");
	result = system(buf);
	if ( result != 0){
		vty_out(vty,"%%factory partition empty\n");
		sprintf(buf,"ubidetach -m 3");
		result = system(buf);
		sprintf(buf,"flash_eraseall  /dev/mtd3");
		result = system(buf);
		sprintf(buf,"ubiformat /dev/mtd3");
		result = system(buf);	
		sleep(1);
		sprintf(buf,"ubiattach -m 3");
		result = system(buf);
		sleep(1);
		sprintf(buf,"ubimkvol /dev/ubi1 -N factory -m");
		result = system(buf);
		sleep(1);
		sprintf(buf,"mount -t ubifs ubi1:factory /mnt/nand");
		result = system(buf);

	}
	vty_out(vty,"%%copy file\n");
	sprintf(buf,"/mnt/flash/boot/%s",argv[0]);
	if (access(buf, F_OK) )
	{
		vty_out(vty, "%% Can't read boot file: %s\n",strerror(errno));
		return CMD_WARNING;

	}



	sprintf(buf,"rm -f /mnt/nand/*");
	result = system(buf);
	sprintf(buf,"cp -f /mnt/flash/boot/%s /mnt/nand/",argv[0]);
	result = system(buf);	
	unlink("/mnt/nand/factory");
	sprintf(buf,"cd /mnt/nand");
	result = system(buf);
	sprintf(buf,"ln -sf %s /mnt/nand/factory",argv[0]);
	result = system(buf);
	sprintf(buf,"umount /mnt/nand");
	result = system(buf);
	sprintf(buf,"ubidetach -m 3");
	result = system(buf);

}

#if 0/* [#78] Adding system inventory management, dustin, 2024-07-24 */
int get_cpu_num(void)
{
	unsigned short read_val;
	
	read_val = sys_util_fpga_memory_read(0x02) & 0x0f;

	return read_val;
}
#endif



int cmd_setup_init()
{
	/* Each node's basic commands. */
	cmd_install_element ( &dbg_setup_cmd);

	cmd_install_element ( &copy_uimage_factory_image_cmd);
	cmd_install_element ( &copy_filename_factory_image_cmd);

	return 0;
}


