
#include <common.h>
#include <command.h>
#include <rtc.h> 
#include <i2c.h>
#if 1	// BSTAR_SD_BOOTING
#include <fat.h>

#define MAC_ADDR_LEN		17
#endif

DECLARE_GLOBAL_DATA_PTR;



int print_bootargs(const char * s,int col,int spn){


 const char *sc;
int cols =0;
int i = 0;
        for (sc = s; *sc != '\0'; ++sc){
		
		if(cols >= col){
			//printf ("                            ");
			printf("\n");
			 for (i =0; i<=spn; ++i)
				serial_putc(' ');
			//printf(" ");
				cols =0;
			}
			serial_putc(*sc);
			cols++;

			}
                
        return sc - s;


}


int show_env( void) {
	char message[CONFIG_SYS_CBSIZE];
	int len;

	if(env_get ("bootdelay") != NULL)
		printf ("bootdelay                : %s seconds \n",env_get ("bootdelay"));
	if(env_get ("baudrate") != NULL)
		printf ("baudrate					 : %s bps\n", env_get ("baudrate"));
	if(env_get ("loads_echo") != NULL)
		printf ("loads_echo               : %s\n",env_get ("loads_echo"));
	if(env_get ("ipaddr") != NULL)
		printf ("ip address					 : %s\n",env_get ("ipaddr"));
	if(env_get ("serverip") != NULL)
		printf ("server ip addr           : %s\n",env_get ("serverip"));
	if(env_get ("gatewayip") != NULL)
		printf ("gateway ip addr          : %s\n",env_get ("gatewayip"));
	if(env_get ("netmask") != NULL)
		printf ("network mask             : %s\n",env_get ("netmask"));
	if(env_get ("bootromname") != NULL)
		printf ("boot rom name		 : %s\n",env_get ("bootromname"));
	if(env_get ("ethaddr") != NULL)
		printf ("mac address			 : %s\n",env_get ("ethaddr"));
	if(env_get ("serial_number") != NULL)
		printf ("serial number			 : %s\n",env_get ("serial_number"));


	if(env_get ("bootargs") != NULL){
		printf ("bootargs                 : "); 
		print_bootargs(env_get ("bootargs"),40,26);
		printf ("\n"); 
	}


	sprintf(message,"\n\nPress Enter key");
	len = cli_readline (message);

	return 0;
}




/*
int parsing_bootargs(const char * s){


 //const char *sc;


             printf("ret=%s \n", strstr(s,"root="));
             if(!strncmp(strstr(s,"root="),"root=/dev/nfs",13))
				printf(" nfs ok!\n");
              if(!strncmp(strstr(s,"root="),"root=/dev/ram",13))
				printf(" ram ok!\n");

                if(strstr(s,"ip="))
				printf(" ip ok!\n");
          //    printf("ret=%s \n",bootargs_str_cut(strstr(s,"root="),"root="));
        return sc - s;


}
*/


int update_bootargs(const char * s,int flag){


            
           
					run_command("run ramargs_noip", flag);

           
        return 0;


}
/*
char *bootargs_str_cut(const char * s,const char *s1){


 char *sc;
// char *st;
int i = 0;
int slen;
		slen = strlen(s1);
	 while ((*s == ' ') || (*s == '\t')) {
			if(i > slen)
				*sc++ = *s++;
			   ++i;	
                        
                }

         
        return (char *)sc ;


}
*/

unsigned long eval_string_to_ip(char *s)
{
	unsigned long addr;
	char *e;
	int i;

	if (s == NULL)
		return(0);

	for (addr=0, i=0; i<4; ++i) {
		ulong val = s ? simple_strtoul(s, &e, 10) : 0;
		addr <<= 8;
		addr |= (val & 0xFF);
		if (s) {
			s = (*e) ? e+1 : e;
		}
	}

	return (htonl(addr));
}


unsigned long eval_env_get_IPaddr (char *var)
{
	return (eval_string_to_ip(env_get(var)));
}


int set_ip_para(const char * s)
{

	return 0;	

}



char * eth_next_enetaddr(char *addr) {
	unsigned char enetaddr[6];
	char *end;
	int i;

	for (i=0; i<6; i++) {
		enetaddr[i] = addr ? simple_strtoul(addr, &end, 16) : 0;
		if (addr)
			addr = (*end) ? end+1 : end;
	}

	if(enetaddr[5] == 0xff)
		{
			if(enetaddr[4] == 0xff)
				{
					if(enetaddr[3] == 0xff)
					{
						if(enetaddr[2] == 0xff)
							{
								if(enetaddr[1] == 0xff)
								{
									if(enetaddr[0] == 0xff)
									{
										sprintf(addr,"%02X:%02X:%02X:%02X:%02X:%02X",0x00,0x00,0x00, 0x00,0x00,0x00);
									}
								else
									{
										sprintf(addr,"%02X:%02X:%02X:%02X:%02X:%02X",enetaddr[0]+1,0x00,0x00,0x00,0x00,0x00);
									}
								}
							else
								{
									sprintf(addr,"%02X:%02X:%02X:%02X:%02X:%02X",enetaddr[0], enetaddr[1]+1,0x00,0x00,0x00,0x00);
								}
							}
						else
							{
								sprintf(addr,"%02X:%02X:%02X:%02X:%02X:%02X",enetaddr[0], enetaddr[1], enetaddr[2]+1,0x00,0x00,0x00);
							}
						}
					else
						{
							sprintf(addr,"%02X:%02X:%02X:%02X:%02X:%02X",enetaddr[0], enetaddr[1], enetaddr[2], enetaddr[3]+1,0x00,0x00);
						}
				}
			else
				{
					sprintf(addr,"%02X:%02X:%02X:%02X:%02X:%02X",enetaddr[0], enetaddr[1], enetaddr[2], enetaddr[3],enetaddr[4]+1,0x00);
				}
		}
	else
		{
	sprintf(addr,"%02X:%02X:%02X:%02X:%02X:%02X",enetaddr[0], enetaddr[1], enetaddr[2], enetaddr[3], enetaddr[4], enetaddr[5]+ 1);
		}

	return addr;
}



int set_env( int flag)
{
	char message[CONFIG_SYS_CBSIZE];
//char message1[CONFIG_SYS_CBSIZE];
//char message2[CONFIG_SYS_CBSIZE];
	int len;
	int size = CONFIG_SYS_CBSIZE - 1;
	extern char        console_buffer[CONFIG_SYS_CBSIZE];

	while (1)
	{
		printf("\033[H\033[J");
		printf("Choose The Enviroment Parameter	\n");


		printf ("0: bootdelay                : %s seconds \n",env_get ("bootdelay"));
		printf ("1: baudrate                 : %s bps\n", env_get ("baudrate"));
		printf ("2: ethact                   : %s\n",env_get ("ethact"));
		printf ("3: ip address               : %s\n",env_get ("ipaddr"));
		printf ("4: network mask             : %s\n",env_get ("netmask"));
		printf ("5: server ip addr           : %s\n",env_get ("serverip"));
		printf ("6: gateway ip addr          : %s\n",env_get ("gatewayip"));
		printf ("7: boot rom name            : %s\n",env_get ("bootromname"));
		printf ("8: bootargs                 : "); 
		print_bootargs(env_get ("bootargs"),40,29);
		printf ("\n"); 
		printf ("9: eth0 address             : %s\n",env_get ("ethaddr"));
		printf ("10: eth1 address            : %s\n",env_get ("eth1addr"));
		printf ("11: eth2 address            : %s\n",env_get ("eth2addr"));
		printf ("12: serial number           : %s\n",env_get ("serial_number"));

		printf ("q: quit          \n");    
		

		sprintf(message,"\n\nchoose number?:");
		len = cli_readline (message);
		if(!strcmp(console_buffer,""))
		{
			break ;
		}
		else if (!strcmp(console_buffer,"0"))
		{
			sprintf (message, "bootdelay                :[%s]\t:",env_get ("bootdelay"));
			len = cli_readline (message);
			if(!strcmp(console_buffer,""))
			{
					;;
			}
			else if(!strcmp(console_buffer,"0"))
			{
				 env_set( "bootdelay", "1");
			}
			else
			{
				if (size < len)
					console_buffer[size] = '\0';
									
				env_set( "bootdelay", console_buffer);
			}
		}
		else if (!strcmp(console_buffer,"1"))
		{
			sprintf (message, "baudrate                 :[%s]\t:",env_get ("baudrate"));
			len = cli_readline (message);
			if(!strcmp(console_buffer,"")){
                        ;;
            }
			else if(!strcmp(console_buffer,"0")){
				env_set( "baudrate", NULL);
                update_bootargs(env_get("bootargs"),flag);
            }
			else{
				if (size < len)
					console_buffer[size] = '\0';
                                        
                env_set( "baudrate", console_buffer);
                update_bootargs(env_get("bootargs"),flag);
            }
			
        }
		else if (!strcmp(console_buffer,"2"))
		{
			sprintf (message, "ethact             :[%s]\t:",env_get ("ethact"));
			len = cli_readline (message);
			if(!strcmp(console_buffer,"")){
					;;
			}
			else if(!strcmp(console_buffer,"0")){
						 ;;
			}
			else{
				if (size < len)
					console_buffer[size] = '\0';

#if 1/* [#17] uboot 의 setup cli 기능 추가, 2023-11-21, balkrow */
				printf ("1:eTSEC0 2:eTSEC1 3:eTSEC2 \n"); 
#else
				printf ("1:UEC0 2:UEC1 3:UEC2 \n"); 
#endif
				sprintf (message, "Choose eth device:");
				len = cli_readline (message);
				if(!strcmp(console_buffer,"")){
							;;
				}
				else if (!strcmp(console_buffer,"1")){
					env_set( "ethact","eTSEC0"); 
								}
				else if (!strcmp(console_buffer,"2")){
					env_set( "ethact","eTSEC1"); 
				}
				else if (!strcmp(console_buffer,"3")){
					env_set( "ethact","eTSEC2"); 
				}
			 }
			  
         }
		 else if (!strcmp(console_buffer,"3")){
				sprintf (message, "ip address               :[%s]\t:",env_get ("ipaddr"));
				len = cli_readline (message);
				if(!strcmp(console_buffer,"")){
                        ;;
                        }
				else if(!strcmp(console_buffer,"0")){
                         env_set( "ipaddr", NULL);
                          
                        }
					else{
					if (size < len)
                        console_buffer[size] = '\0';
                                        
                        env_set( "ipaddr", console_buffer);
                        
                        }
			  
                        }
		 else if (!strcmp(console_buffer,"4")){
				sprintf (message, "network mask             :[%s]\t:",env_get ("netmask"));
				len = cli_readline (message);
				if(!strcmp(console_buffer,"")){
                        ;;
                        }
				else if(!strcmp(console_buffer,"0")){
                         env_set( "netmask", NULL);
                         
                        }
					else{
					if (size < len)
                        console_buffer[size] = '\0';
                                        
                        env_set( "netmask", console_buffer);
                       
                        }
			 
                        }
		  else if (!strcmp(console_buffer,"5")){
				sprintf (message, "server ip addr           :[%s]\t:",env_get ("serverip"));
				len = cli_readline (message);
				if(!strcmp(console_buffer,"")){
                        ;;
                        }
				else if(!strcmp(console_buffer,"0")){
                         env_set( "serverip", NULL);
                          
                        }
					else{
					if (size < len)
                        console_buffer[size] = '\0';
                                        
                        env_set( "serverip", console_buffer);
                        
                        }
			
                        }
		   else if (!strcmp(console_buffer,"6")){
				sprintf (message, "gateway ip addr          :[%s]\t:",env_get ("gatewayip"));
				len = cli_readline (message);
				if(!strcmp(console_buffer,"")){
                        ;;
                        }
				else if(!strcmp(console_buffer,"0")){
                         env_set( "gatewayip", NULL);
                         
                        }
					else{
					if (size < len)
                        console_buffer[size] = '\0';
                                        
                        env_set( "gatewayip", console_buffer);
                        
                        }
			 
                        }
			
				
			 else if (!strcmp(console_buffer,"7")){
				sprintf (message, "boot rom name          :[%s]\t:",env_get ("bootromname"));
				len = cli_readline (message);
				if(!strcmp(console_buffer,"")){
                        ;;
                        }
				else if(!strcmp(console_buffer,"0")){
                         //env_set( "bootromname", NULL);
                         ;;
					}
					else{
					if (size < len)
                        console_buffer[size] = '\0';
                                        
                        env_set( "bootromname", console_buffer);
                        }
			  
                        }
			  else if (!strcmp(console_buffer,"8")){
				
				
			  printf ("bootargs: "); 
			  print_bootargs(env_get ("bootargs"),60,8);
			  printf ("\n"); 
			  
				sprintf (message, "bootargs setup? (y/n):");
				len = cli_readline (message);
				if(!strcmp(console_buffer,"")){
                        ;;
                        }
					else if (!strcmp(console_buffer,"y")){
						printf ("not yet\n"); 
						
					}  
				}
			   else if (!strcmp(console_buffer,"9")){
				sprintf (message, "eth0 address             :[%s]\t:",env_get ("ethaddr"));
				len = cli_readline (message);
				if(!strcmp(console_buffer,"")){
                        ;;
                        }
					else{
					if (size < len)
                        console_buffer[size] = '\0';
                                        
                        env_set( "ethaddr", console_buffer);
						sprintf(message,"%s",env_get ("ethaddr"));
						env_set( "eth1addr",eth_next_enetaddr(message));
						sprintf(message,"%s",env_get ("eth1addr"));
						env_set( "eth2addr",eth_next_enetaddr(message));
						
                        }
			  
                        }
				else if (!strcmp(console_buffer,"10")){
				sprintf (message, "eth1 address             :[%s]\t:",env_get ("eth1addr"));
				len = cli_readline (message);
				if(!strcmp(console_buffer,"")){
                        ;;
                        }
					else{
					if (size < len)
                        console_buffer[size] = '\0';
                                        
                        env_set( "eth1addr", console_buffer);
                        }
			  
                        }
			
				else if (!strcmp(console_buffer,"11")){
				sprintf (message, "eth2 address             :[%s]\t:",env_get ("eth2addr"));
				len = cli_readline (message);
				if(!strcmp(console_buffer,"")){
                        ;;
                        }
					else{
					if (size < len)
                        console_buffer[size] = '\0';
                                        
                        env_set( "eth2addr", console_buffer);
                        }
			  
                        }
			
			
				else if (!strcmp(console_buffer,"12")){
				sprintf (message, "serial number             :[%s]\t:",env_get ("serial_number"));
				len = cli_readline (message);
				if(!strcmp(console_buffer,"")){
                        ;;
                        }
					else{
					if (size < len)
                        console_buffer[size] = '\0';
                                        
                        env_set( "serial_number", console_buffer);
                        }
			  
                        }
				 else if (!strcmp(console_buffer,"12")){
					
			  
                        }
					 else {
				
			   break;
                        }

	 }                     
			sprintf(message,"are you sure[y:flash save n:not save]?(y/n)");
					len = cli_readline (message);
					 if( strcmp(console_buffer,"y") == 0){
						 run_command ("saveenv",  flag);
											  }
							else printf("not saved !!\n");
return 0;
}




int upgrade_tftp( int flag){

extern char        console_buffer[CONFIG_SYS_CBSIZE];
char message[CONFIG_SYS_CBSIZE];
        int len;
   int size = 0;
     

printf("\033[H\033[J");
printf("CPIU  upgrade program!!!!");
printf("\n\n");
printf("*********************************\n");
printf("Choose The Upgrade Type		\n");
printf("1:         Boot rom image		\n");
printf("*********************************\n");
 sprintf(message,"\n\nchoose number?:");
  len = cli_readline (message);
        if(!strcmp(console_buffer,"")){
                        return 0;
                        }
        else if (!strcmp(console_buffer,"1")){
			 run_command ("brp",  flag);
                        return size;
                        }
		else {
                        return 0;
                        }

return 0;
}




int etc_setup( int flag){
char message[CONFIG_SYS_CBSIZE];
 int len;
 extern char        console_buffer[CONFIG_SYS_CBSIZE];
//int month,day,hour,minute,year,seconds;

 while(1){

printf("\033[H\033[J");
printf("CPIU Etc program!!!!");
printf("\n\n");
printf("*********************************\n");
printf("Choose The Setup Type		\n");
printf("0:       Show Date				\n");
printf("1:       Set Date				   \n");
#ifdef	CONFIG_PRIVATE_PASSWORD_RECOVERY
printf("2:       Password recovery                  \n");
#endif
printf("*********************************\n");
 sprintf(message,"\n\nchoose number?:");
  len = cli_readline (message);
        if(!strcmp(console_buffer,"")){
                        return 0;
                        }
			else if (!strcmp(console_buffer,"0")){
					run_command ("date",  flag);
                        }
		else if (!strcmp(console_buffer,"1")){
			 struct rtc_time tm;
				int year,month,day,hour,minute,seconds;
				int old_bus;
				/* switch to correct I2C bus */
				old_bus = I2C_GET_BUS();
				I2C_SET_BUS(CONFIG_SYS_RTC_BUS_NUM);
				rtc_get (&tm);
				I2C_SET_BUS(old_bus);
				 sprintf(message,"year?[%4d]:",tm.tm_year);
				 len = cli_readline (message);
				  if(!strcmp(console_buffer,"")){
								year = tm.tm_year;
							}
				  else {
					year = simple_strtoul(console_buffer, NULL, 10);

					}
				   sprintf(message,"month?[%02d]:",tm.tm_mon);
				 len = cli_readline (message);
				  if(!strcmp(console_buffer,"")){
								month = tm.tm_mon;
							}
				  else {
					month = simple_strtoul(console_buffer, NULL, 10);
					//if ( (1 < month) || (12 > month))
					//	continue;
					}
				   sprintf(message,"day?[%02d]:",tm.tm_mday);
				 len = cli_readline (message);
				  if(!strcmp(console_buffer,"")){
								day = tm.tm_mday;
							}
				  else {
					day = simple_strtoul(console_buffer, NULL, 10);
					//if ( (1 < day) || (31 > day))
					//	continue;
					}
				   sprintf(message,"hour?[%2d]:",tm.tm_hour);
				 len = cli_readline (message);
				  if(!strcmp(console_buffer,"")){
								hour = tm.tm_hour;
							}
				  else {
					hour = simple_strtoul(console_buffer, NULL, 10);
					//if ( (0 < hour) || (24 > hour))
					//	continue;
					}
				   sprintf(message,"minute?[%02d]:",tm.tm_min);
				 len = cli_readline (message);
				  if(!strcmp(console_buffer,"")){
								minute = tm.tm_min;
							}
				  else {
					minute = simple_strtoul(console_buffer, NULL, 10);
					//if ( (0 < minute) || (60 > minute))
					//	continue;
					}
				   sprintf(message,"seconds?[%02d]:",tm.tm_sec);
				 len = cli_readline (message);
				  if(!strcmp(console_buffer,"")){
								seconds = tm.tm_sec;
							}
				  else {
					seconds = simple_strtoul(console_buffer, NULL, 10);
					//if ( (0 < seconds) || (60 > seconds))
					//	continue;
					}
				   sprintf(message,"date %02d%02d%02d%02d%4d.%02d",month,day,hour,minute,year,seconds);
				  // sprintf("%s\n",message);
				   run_command (message,  flag);
                         }
			else if (!strcmp(console_buffer,"2")){
#ifdef	CONFIG_PRIVATE_PASSWORD_RECOVERY				
					env_set( "password_recovery", "on");
					
				sprintf(message,"booting system ?:");
				 len = cli_readline (message);
				  if( strcmp(console_buffer,"y") == 0){
						 run_command ("boot",  flag);
						  }
#endif			  
							
                        }

		else {
                        return 0;
                        }

		sprintf(message,"\n\nPress Enter key");
	len = cli_readline (message);
	}
}







int board_shell( int flag){
//char message[CONFIG_SYS_CBSIZE];

 static char lastcommand[CONFIG_SYS_CBSIZE] = { 0, };
	int len;
	int rc = 1;
	
 extern char        console_buffer[CONFIG_SYS_CBSIZE];
 
for (;;) {

		
		len = cli_readline (CONFIG_SYS_PROMPT);

		flag = 0;	/* assume no special flags for now */
		if (len > 0)
			strcpy (lastcommand, console_buffer);
		else if (len == 0)
			flag |= CMD_FLAG_REPEAT;

			 if( strcmp(console_buffer,"exit") == 0)
			return -1;		/* retry autoboot */

		if (len == -1)
			puts ("<INTERRUPT>\n");
		else
			rc = run_command (lastcommand, flag);

		if (rc <= 0) {
			/* invalid command or not repeatable, forget it */
			lastcommand[0] = 0;
		}
	}
return 0;
}

#define CONFIG_SYS_SLOT_ADDR 0x40000000
#define CONFIG_SYS_FPGA_ADDR 0xf9000000

#define HDRIVER_MEMORY_TYPE_READ				0x1
#define HDRIVER_MEMORY_TYPE_WRITE				0x2

struct hdrv_slot_desc
{
	int slot;
    unsigned short slotdata;
    char slot_desc[10];
};
typedef struct hdrv_slot_desc hdrv_slot_desc_t;

hdrv_slot_desc_t slotdata_tbl[] = {
	{0,0x0000,"none"},  /* slot none */
	{1,0x0040,"slot1"},{2,0x0044,"slot2"},{3,0x0048,"slot3"},{4,0x004c,"slot4"},{5,0x0050,"slot5"},
	{6,0x0054,"slot6"},{7,0x0058,"slot7"},{8,0x005c,"slot8"},{9,0x0060,"slot9"},{10,0x0064,"slot10"},
	{11,0x0080,"slot11"},{12,0x0084,"slot12"},{13,0x0088,"slot13"},{14,0x008c,"slot14"},{15,0x0090,"slot15"},
	{16,0x0094,"slot16"},{17,0x0098,"slot17"},{18,0x009c,"slot18"},{19,0x00a0,"slot19"},{20,0x00a4,"slot20"},
	{21,0x0070,"dcu"}, /* slot dcu */
	{22,0x0000,"fan1"},{23,0x0000,"fan2"},{24,0x0000,"reserv"},{25,0x0000,"reserv"},{26,0x0000,"reserv"},
	{27,0x0000,"reserv"},{28,0x0000,"reserv"},{29,0x0000,"reserv"},{30,0x0000,"reserv"},{31,0x0000,"reserv"},
};

hdrv_slot_desc_t slotdata_tbl_8300[] = {
	{0,0x0000,"none"},  /* slot none */
	{1,0x0040,"slot1"},{2,0x0044,"slot2"},{3,0x0048,"slot3"},{4,0x004c,"slot4"},{5,0x0050,"slot5"},
	{6,0x0054,"slot6"},{7,0x005c,"slot7"},{8,0x0070,"slot8"},{9,0x0060,"slot9"},{10,0x0064,"slot10"},
	{11,0x0080,"slot11"},{12,0x0084,"slot12"},{13,0x0088,"slot13"},{14,0x008c,"slot14"},{15,0x0090,"slot15"},
	{16,0x0094,"slot16"},{17,0x0098,"slot17"},{18,0x009c,"slot18"},{19,0x00a0,"slot19"},{20,0x00a4,"slot20"},
	{21,0x0070,"dcu"}, /* slot dcu */
	{22,0x0000,"fan1"},{23,0x0000,"fan2"},{24,0x0000,"reserv"},{25,0x0000,"reserv"},{26,0x0000,"reserv"},
	{27,0x0000,"reserv"},{28,0x0000,"reserv"},{29,0x0000,"reserv"},{30,0x0000,"reserv"},{31,0x0000,"reserv"},
};

unsigned short slot_memory( int type,int slot,unsigned int addr, unsigned short value)
{

if( (slot < 0 ) || (slot > 22) )
	 return 0;



	*(volatile unsigned short*)(CONFIG_SYS_FPGA_ADDR + 0x50) = slotdata_tbl[slot].slotdata;
	

	if(type == HDRIVER_MEMORY_TYPE_READ)
		{
			value = *(volatile unsigned short*)(CONFIG_SYS_SLOT_ADDR + addr);
		}
	else if(type == HDRIVER_MEMORY_TYPE_WRITE)
		{
			*(volatile unsigned short*)(CONFIG_SYS_SLOT_ADDR + addr) = (value & 0xffff);
		}
	else
		value = 0;

	return value;	
}

unsigned short slot_memory_8300( int type,int slot,unsigned int addr, unsigned short value)
{

if( (slot < 0 ) || (slot > 22) )
	 return 0;


		*(volatile unsigned short*)(CONFIG_SYS_FPGA_ADDR + 0x50) = slotdata_tbl_8300[slot].slotdata;
			
		
			if(type == HDRIVER_MEMORY_TYPE_READ)
				{
					value = *(volatile unsigned short*)(CONFIG_SYS_SLOT_ADDR + addr);
				}
			else if(type == HDRIVER_MEMORY_TYPE_WRITE)
				{
					*(volatile unsigned short*)(CONFIG_SYS_SLOT_ADDR + addr) = (value & 0xffff);
				}
			else
				value = 0;
	
	return value;	
}



unsigned short fpga_memory( int type,unsigned int addr, unsigned short value)
{

	if(type == HDRIVER_MEMORY_TYPE_READ)
		{
			value = *(volatile unsigned short*)(CONFIG_SYS_FPGA_ADDR + addr);
			return value;

		}
	else if(type == HDRIVER_MEMORY_TYPE_WRITE)
		{
			*(volatile unsigned short*)(CONFIG_SYS_FPGA_ADDR + addr) = (value & 0xffff);
			return value;
		}
	return 0;	
}


#define FPGA_REG_PBA_INSERT1		0x12		//PBA card remove register1 (R, 0xF900_0012)
#define FPGA_REG_PBA_INSERT2		0x14		//PBA card remove register2 (R, 0xF900_0014)
#define FPGA_REG_PBA_INSERT3		0x16		//PBA card remove register3 (R, 0xF900_0016)
#define FPGA_REG_CONTROL			0x1C		//Control register (R/W, 0xF900_001C)




int hsn_led_test(int flag){
	unsigned short tmp_val =0;
	int slot;
	unsigned short shift = 0;
	unsigned char slinsert[32];
	int i;
	(unsigned short)fpga_memory(HDRIVER_MEMORY_TYPE_WRITE,0x3A,0xA5A5);
		tmp_val = *(volatile unsigned short*)(CONFIG_SYS_FPGA_ADDR + FPGA_REG_PBA_INSERT2);
		for(slot = 1 ; slot < 11 ; slot++){
			if( tmp_val & (0x0001 << (slot -1)))
				{
					slinsert[slot] = 0x0;
				}
			else
				{
					slinsert[slot] = 0x1;
				}
			}
		
		tmp_val = *(volatile unsigned short*)(CONFIG_SYS_FPGA_ADDR + FPGA_REG_PBA_INSERT3);
		for(slot = 11 ; slot < 21 ; slot++){
			if( tmp_val & (0x0001 << (slot -11)))
				{
					slinsert[slot] = 0x0;
				}
			else
				{
					slinsert[slot] = 0x1;
				}
			}
			
		tmp_val = *(volatile unsigned short*)(CONFIG_SYS_FPGA_ADDR + FPGA_REG_PBA_INSERT1);
		for(slot= 21; slot<24; slot++){ 
			if(slot == 21) shift = 0;
			else if(slot == 22) shift = 4;
			else if ( slot == 23 ) shift =5;
			
			if(tmp_val & (0x0001 << shift))
				{
					slinsert[slot] = 0x0;
				}
			else
				{
					slinsert[slot] = 0x1;
				}
			}
	

			for(i=0; i <5; i++)
			{

				fpga_memory(HDRIVER_MEMORY_TYPE_WRITE,FPGA_REG_CONTROL,0x02);
						for(slot=1; slot <24; slot++)
						{
								if(slinsert[slot]) 
									slot_memory(HDRIVER_MEMORY_TYPE_WRITE,slot,FPGA_REG_CONTROL,0x02);
						}
				udelay (1000000);
				fpga_memory(HDRIVER_MEMORY_TYPE_WRITE,FPGA_REG_CONTROL,0x01);
						for(slot=1; slot<24; slot++)
						{
								if(slinsert[slot]) 
									slot_memory(HDRIVER_MEMORY_TYPE_WRITE,slot,FPGA_REG_CONTROL,0x01);
						}
				udelay (1000000);

					}

				fpga_memory(HDRIVER_MEMORY_TYPE_WRITE,FPGA_REG_CONTROL,0x00);
						for(slot=1; slot<24; slot++)
						{
								if(slinsert[slot]) 
									slot_memory(HDRIVER_MEMORY_TYPE_WRITE,slot,FPGA_REG_CONTROL,0x00);
						}
 
return 0;
}




#define DRAM_START	0x00100000


int hsn_mcu_dram_test(int flag,int *ctlc){
	volatile unsigned char *dram_addr_b, dram_data_b,dram_data_b_org;
		   volatile unsigned short *dram_addr_w, dram_data_w,dram_data_w_org;
		   volatile unsigned int *dram_addr_l, dram_data_l,dram_data_l_org;

		unsigned int error_cnt = 0;
		int prt_flg =0,lr_flag =0;
		int dram_end = gd->ram_size - 0x100000;

		(unsigned short)fpga_memory(HDRIVER_MEMORY_TYPE_WRITE,0x3A,0xA5A5);

		puts("********** Dram Test : Size=");
		print_size (gd->ram_size, "      **********\n");



                puts ("BYTE DATA WRITE TEST : data=0x00  \n");
                dram_addr_b = (unsigned char *) DRAM_START;
                dram_data_b     = 0x00;
				prt_flg = 0;
				lr_flag = 0;
                for(; (int)dram_addr_b < dram_end ; ) {
					if( prt_flg > 0x4000 ) {
						puts ("*");
						prt_flg = 0;
						lr_flag++;
						if(lr_flag > 40 ){
							puts ("\n");
							lr_flag = 0;
							}
						}
					prt_flg++;
						dram_data_b_org = *dram_addr_b;
                        *dram_addr_b  = dram_data_b;
						if (*dram_addr_b != dram_data_b)
                        {
                                printf ("\n DATA ERROR: addr=0x%08x, DATA=0x%02x (Org=0x00)\n", (int)dram_addr_b, *dram_addr_b);
								error_cnt++;
                        }
						*dram_addr_b  = dram_data_b_org;
                         dram_addr_b = dram_addr_b + 0x100;
				if (ctrlc()) {
				putc ('\n');
				*ctlc = 1;
				return error_cnt;
				}
                }
				puts("\n");

               

                puts ("WORD DATA WRITE TEST : data=0xffff  \n");
                dram_addr_w = (unsigned short *) DRAM_START;
                dram_data_w     = 0xffff;
				prt_flg = 0;
				lr_flag = 0;
                for(; (int)dram_addr_w < dram_end ; ) {
					if( prt_flg > 0x2000 ) {
						puts ("*");
						prt_flg = 0;
						lr_flag++;
						if(lr_flag > 40 ){
							puts ("\n");
							lr_flag = 0;
							}
						}
					prt_flg++;
					dram_data_w_org = *dram_addr_w;
                        *dram_addr_w  = dram_data_w;
						 if (*dram_addr_w != dram_data_w)
                        {
                                printf ("\n DATA ERROR: addr=0x%04x, DATA=0x%04x (Org=0xffff)\n", (int)dram_addr_w, *dram_addr_w);
								error_cnt++;
                        }
						 *dram_addr_w  = dram_data_w_org;
                        dram_addr_w = dram_addr_w + 0x100;
				if (ctrlc()) {
				putc ('\n');
				*ctlc = 1;
				return error_cnt;
				}
                }

                puts ("\n");
              


                puts ("LONG DATA WRITE TEST : data=0x00000000  \n");
                dram_addr_l = (unsigned int *) DRAM_START;
                dram_data_l     = 0x00000000;
                prt_flg = 0;
				lr_flag = 0;
                for(; (int)dram_addr_l < dram_end ; ) {
					if( prt_flg > 0x1000 ) {
						puts ("*");
						prt_flg = 0;
						lr_flag++;
						if(lr_flag > 40 ){
							puts ("\n");
							lr_flag = 0;
							}
						}
					prt_flg++;
					dram_data_l_org = *dram_addr_l;
                        *dram_addr_l  = dram_data_l;
						if (*dram_addr_l != dram_data_l)
                        {
                        printf ("\n DATA ERROR: addr=0x%08x, DATA=0x%08x (Org=0x00000000)\n", (int)dram_addr_l, *dram_addr_l);
						error_cnt++;
                        }
						*dram_addr_l  = dram_data_l_org;
                         dram_addr_l = dram_addr_l + 0x100;
				if (ctrlc()) {
				putc ('\n');
				*ctlc = 1;
				return error_cnt;
				}
                }
                
		puts ("\n");
              
                puts ("LONG DATA WRITE TEST : data=0x5aa53cc3  \n");
                dram_addr_l = (unsigned int *) DRAM_START;
                dram_data_l     = 0x5aa53cc3;
                    prt_flg = 0;
					lr_flag = 0;
                for(; (int)dram_addr_l < dram_end ; ) {
					if( prt_flg > 0x1000 ) {
						puts ("*");
						prt_flg = 0;
						lr_flag++;
						if(lr_flag > 40 ){
							puts ("\n");
							lr_flag = 0;
							}
						}
					prt_flg++;
					dram_data_l_org = *dram_addr_l;
                        *dram_addr_l  = dram_data_l;
						if (*dram_addr_l != dram_data_l)
                        {
                        printf ("\n DATA ERROR: addr=0x%08x, DATA=0x%08x (Org=0x5aa53cc3)\n", (int)dram_addr_l, *dram_addr_l);
						error_cnt++;
                        }
						*dram_addr_l  = dram_data_l_org;
                         dram_addr_l = dram_addr_l + 0x100;
			if (ctrlc()) {
				putc ('\n');
				*ctlc = 1;
				return error_cnt;
				}
                }
                
		puts ("\n");
               
                puts ("LONG DATA WRITE TEST : data=0xc33ca55a  \n");
                dram_addr_l = (unsigned int *) DRAM_START;
                dram_data_l     = 0xc33ca55a;
                  prt_flg = 0;
				  lr_flag = 0;
                for(; (int)dram_addr_l < dram_end ; ) {
					if( prt_flg > 0x1000 ) {
						puts ("*");
						prt_flg = 0;
						lr_flag++;
						if(lr_flag > 40 ){
							puts ("\n");
							lr_flag = 0;
							}
						}

					prt_flg++;
					dram_data_l_org = *dram_addr_l;
                        *dram_addr_l  = dram_data_l;
						if (*dram_addr_l != dram_data_l)
                        {
                        printf ("\n DATA ERROR: addr=0x%08x, DATA=0x%08x (Org=0xc33ca55a)\n", (int)dram_addr_l, *dram_addr_l);
						error_cnt++;
                        }
						*dram_addr_l  = dram_data_l_org;
                         dram_addr_l = dram_addr_l + 0x100;
			if (ctrlc()) {
				putc ('\n');
				*ctlc = 1;
				return error_cnt;
				}
                }
                
		puts ("\n");

				puts ("LONG DATA WRITE TEST : data=0x55555555  \n");
                dram_addr_l = (unsigned int *) DRAM_START;
                dram_data_l     = 0x55555555;
                  prt_flg = 0;
				  lr_flag = 0;
                for(; (int)dram_addr_l < dram_end ; ) {
					if( prt_flg > 0x1000 ) {
						puts ("*");
						prt_flg = 0;
						lr_flag++;
						if(lr_flag > 40 ){
							puts ("\n");
							lr_flag = 0;
							}
						}

					prt_flg++;
					dram_data_l_org = *dram_addr_l;
                        *dram_addr_l  = dram_data_l;
						if (*dram_addr_l != dram_data_l)
                        {
                        printf ("\n DATA ERROR: addr=0x%08x, DATA=0x%08x (Org=0x55555555)\n", (int)dram_addr_l, *dram_addr_l);
						error_cnt++;
                        }
						*dram_addr_l  = dram_data_l_org;
                         dram_addr_l = dram_addr_l + 0x100;
			if (ctrlc()) {
				putc ('\n');
				*ctlc = 1;
				return error_cnt;
				}
                }
                
		puts ("\n");

			puts ("LONG DATA WRITE TEST : data=0xaaaaaaaa  \n");
                dram_addr_l = (unsigned int *) DRAM_START;
                dram_data_l     = 0xaaaaaaaa;
                  prt_flg = 0;
				  lr_flag = 0;
                for(; (int)dram_addr_l < dram_end ; ) {
					if( prt_flg > 0x1000 ) {
						puts ("*");
						prt_flg = 0;
						lr_flag++;
						if(lr_flag > 40 ){
							puts ("\n");
							lr_flag = 0;
							}
						}

					prt_flg++;
					dram_data_l_org = *dram_addr_l;
                        *dram_addr_l  = dram_data_l;
						if (*dram_addr_l != dram_data_l)
                        {
                        printf ("\n DATA ERROR: addr=0x%08x, DATA=0x%08x (Org=0xaaaaaaaa)\n", (int)dram_addr_l, *dram_addr_l);
						error_cnt++;
                        }
						*dram_addr_l  = dram_data_l_org;
                         dram_addr_l = dram_addr_l + 0x100;
			if (ctrlc()) {
				putc ('\n');
				*ctlc = 1;
				return error_cnt;
				}
                }
                
		puts ("\n");
                puts ("LONG DATA WRITE TEST : data=0xffffffff		\n");
                dram_addr_l = (unsigned int *) DRAM_START;
                dram_data_l     = 0xffffffff;
                 prt_flg = 0;
				 lr_flag = 0;
                for(; (int)dram_addr_l < dram_end ; ) {
					if( prt_flg > 0x1000 ) {
						puts ("*");
						prt_flg = 0;
						lr_flag++;
						if(lr_flag > 40 ){
							puts ("\n");
							lr_flag = 0;
							}
						}
						prt_flg++;
						dram_data_l_org = *dram_addr_l;
                        *dram_addr_l  = dram_data_l;
						 if (*dram_addr_l != dram_data_l)
                        {
                        printf ("\n DATA ERROR: addr=0x%08x, DATA=0x%08x (Org=0xffffffff)\n", (int)dram_addr_l, *dram_addr_l);
						error_cnt++;
                        }
						 *dram_addr_l  = dram_data_l_org;
                         dram_addr_l = dram_addr_l + 0x100;
			if (ctrlc()) {
				putc ('\n');
				*ctlc = 1;
				return error_cnt;
				}
                }
                
		puts ("\n");
             



             puts ("LONG ADDR WRITE/READ/VERIFY TEST \n");
                puts ("LONG ADDR WRITE  \n");
                dram_addr_l = (unsigned int *) DRAM_START;
                prt_flg = 0;
				lr_flag = 0;
                for(; (int)dram_addr_l < dram_end ; ) {
					if( prt_flg > 0x1000 ) {
						puts ("*");
						prt_flg = 0;
						lr_flag++;
						if(lr_flag > 40 ){
							puts ("\n");
							lr_flag = 0;
							}
						}

					prt_flg++;
						dram_data_l_org = *dram_addr_l;
                        *dram_addr_l    = (unsigned int) dram_addr_l;
						if (*dram_addr_l != (unsigned int)dram_addr_l)
                        {
                                printf ("Verify ERROR: addr=0x%08x, DATA=0x%08x ,Org=0x%08x \n", (int)dram_addr_l, *dram_addr_l,(int)dram_addr_l );
							error_cnt++;
						}
						*dram_addr_l  = dram_data_l_org;
				 dram_addr_l = dram_addr_l + 0x100;
                        
			if (ctrlc()) {
				putc ('\n');
				*ctlc = 1;
				return error_cnt;
				}
                }
                
		puts ("\n");
               

                
		puts ("==========   DRAM TEST DONE. : Size=");
		print_size (gd->ram_size, "========== \n");


		
		return (error_cnt);
	
}


int hsn_rtc_test(int flag){
	struct rtc_time tm,savetm;
	int rcode = 0;
	rcode = rtc_get (&savetm);
	
			if (rcode) {
				puts("## Get date failed\n");
				return 1;
			}
	
			printf ("Date: %4d-%02d-%02d Time: %2d:%02d:%02d\n",
				savetm.tm_year, savetm.tm_mon, savetm.tm_mday,savetm.tm_hour, savetm.tm_min, savetm.tm_sec);
			printf("wait 1 sec\n");
			udelay (1000000);
			rcode = rtc_get (&tm);
	
			if (rcode) {
				puts("## Get date failed\n");
				return 1;
			}
	
			printf ("Date: %4d-%02d-%02d Time: %2d:%02d:%02d\n",
				tm.tm_year, tm.tm_mon, tm.tm_mday,tm.tm_hour, tm.tm_min, tm.tm_sec);
			if(savetm.tm_min == tm.tm_min)
				{
					if(tm.tm_sec > savetm.tm_sec)
						return 0;
					else
						return 1;

				}
			else if (savetm.tm_min > tm.tm_min)
				{
					return 1;

				}
			else{
					return 0;
				}
			
 
return 0;
}


      


int hsn_humidity_test(int flag){
	int old_bus;
	unsigned char capa_h ;
	unsigned char capa_l ; 
	unsigned char temp_h ;
	unsigned char temp_l ; 
	int j= 0;
	unsigned char	linebuf[16];
			unsigned char	*cp;
	unsigned int humi_cap_data =0,temp_data =0;		
		/* switch to correct I2C bus */
		old_bus = I2C_GET_BUS();
		I2C_SET_BUS(0);

		if (i2c_read(0x28, 0, 1, linebuf, 4) != 0)
			{
			puts ("Error reading the chip.\n");
			return 1;
			}
		else {
			I2C_SET_BUS(old_bus);
			cp = linebuf;
			capa_h = *cp++;
			capa_l = *cp++;
			temp_h = *cp++;
			temp_l = *cp++;	
			if( (capa_h == 0xff) && (capa_l == 0xff) && (temp_h == 0xff) && (temp_l == 0xff))
				return 1;
			else
				return 0;
			}
			
 
return 0;
}



#define HDRIVER_SHELF_TYPE_HSN8100NV			0x2
#define HDRIVER_SHELF_TYPE_HSN8300NV			0x1
#define HDRIVER_SHELF_TYPE_HSN8500NV			0x0

#define HDRIVER_UNIT_TYPE_MCU					0x1
#define HDRIVER_UNIT_TYPE_DCU					0x31
#define HDRIVER_UNIT_TYPE_OTU4G					0x41
#define HDRIVER_UNIT_TYPE_TTU4G					0x42
#define HDRIVER_UNIT_TYPE_ATU10F				0x43
#define HDRIVER_UNIT_TYPE_ETU4G					0x44
#define HDRIVER_UNIT_TYPE_OTU4H					0x45	//OTU4H
#define HDRIVER_UNIT_TYPE_MXDH					0x51
#define HDRIVER_UNIT_TYPE_M0DH0					0x51
#define HDRIVER_UNIT_TYPE_M0CS0					0x52
#if 1 //R1.2	
#define HDRIVER_UNIT_TYPE_M0DH2A               0x53
#define HDRIVER_UNIT_TYPE_M0DH2B               0x54
#define HDRIVER_UNIT_TYPE_M0DH1                0x55
#endif

#define HDRIVER_UNIT_TYPE_OPSU						0x60


#define HDRIVER_UNIT_TYPE_MSC1						0xA1
#define HDRIVER_UNIT_TYPE_MSC2						0xA2
#define HDRIVER_UNIT_TYPE_MSC3						0xA3
#define HDRIVER_UNIT_TYPE_MSC4						0xA4
#define HDRIVER_UNIT_TYPE_MXC0						0xA0


int hsn_system_info( int flag)
{
	char message[CONFIG_SYS_CBSIZE];
	int len;

unsigned short shelf_type;
unsigned short tempval,temp_val;
unsigned short	fpga_ver;
	unsigned char	pba;
	unsigned char	pcb;
unsigned char	aco = 0;
	unsigned char	acolock = 0;	
int slot;
unsigned short	slot_type;

shelf_type = fpga_memory(HDRIVER_MEMORY_TYPE_READ,0 ,tempval);
if(shelf_type == HDRIVER_SHELF_TYPE_HSN8100NV)
{
	printf("Shelf type : HSN8100\n");
	printf("Slot: codes D-DCU, A-ATU10F, O-OTU4G, 4H-OTU4H, T-TTU4G, M-M0DH0,\n"); 
	printf("            C-M0CS0, MA-M0DH2A, MB-M0DH2B, M1-M0DH1, \n");
	printf("            M0-MXC0, M1-MSC1, M2-MSC2, M3-MSC3, M4-MSC4,X-notequip\n");
	printf(" 1| 2| 3\n");
	temp_val = *(volatile unsigned short*)(CONFIG_SYS_FPGA_ADDR + 0x14);
	for(slot = 1 ; slot < 3 ; slot++){
		if( temp_val & (0x0001 << (slot -1)))
			{
				printf(" X|");
			}
		else
			{
				slot_type = (unsigned char)slot_memory(HDRIVER_MEMORY_TYPE_READ,slot,0xA,0);	
				if(slot_type == HDRIVER_UNIT_TYPE_DCU)
				{
					printf(" D|");
				}	
				else if(slot_type == HDRIVER_UNIT_TYPE_ATU10F)
				{
					printf(" A|");
				}
				else if(slot_type == HDRIVER_UNIT_TYPE_OTU4G)
				{
					printf(" O|");
				}
				else if(slot_type == HDRIVER_UNIT_TYPE_OTU4H)	//OTU4H
				{
					printf(" 4H|");
				}				
				else if(slot_type == HDRIVER_UNIT_TYPE_TTU4G)
				{
					printf(" T|");
				}
				else if(slot_type == HDRIVER_UNIT_TYPE_MXDH)
				{
					printf(" M|");
				}
				else if(slot_type == HDRIVER_UNIT_TYPE_M0CS0)
				{
					printf(" C|");
				}
				else if(slot_type == HDRIVER_UNIT_TYPE_M0DH2A)
				{
					printf("MA|");
				}
				else if(slot_type == HDRIVER_UNIT_TYPE_M0DH2B)
				{
					printf("MB|");
				}
				else if(slot_type == HDRIVER_UNIT_TYPE_M0DH1)
				{
					printf("M1|");
				}
				else if(slot_type == HDRIVER_UNIT_TYPE_MXC0)
				{
					printf("M0|");
				}	
				else if(slot_type == HDRIVER_UNIT_TYPE_MSC1)
				{
					printf("M1|");
				}
				else if(slot_type == HDRIVER_UNIT_TYPE_MSC2)
				{
					printf("M2|");
				}
				else if(slot_type == HDRIVER_UNIT_TYPE_MSC3)
				{
					printf("M3|");
				}
				else if(slot_type == HDRIVER_UNIT_TYPE_MSC4)
				{
					printf("M4|");
				}
				else
				{
					printf(" ?|");
				}
				
			}
		
			
		}
	temp_val = *(volatile unsigned short*)(CONFIG_SYS_FPGA_ADDR + 0x12);
	if( temp_val & (0x0001 << 0))
			{
				printf(" X|");
			}
		else
			{
				slot_type = (unsigned char)slot_memory(HDRIVER_MEMORY_TYPE_READ,21,0xA,0);	
				if(slot_type == HDRIVER_UNIT_TYPE_DCU)
				{
					printf(" D");
				}	
				else if(slot_type == HDRIVER_UNIT_TYPE_ATU10F)
				{
					printf(" A");
				}
				else if(slot_type == HDRIVER_UNIT_TYPE_OTU4G)
				{
					printf(" O");
				}
				else if(slot_type == HDRIVER_UNIT_TYPE_OTU4H)	//OTU4H
				{
					printf(" H");
				}				
				else if(slot_type == HDRIVER_UNIT_TYPE_TTU4G)
				{
					printf(" T");
				}
				else if(slot_type == HDRIVER_UNIT_TYPE_MXDH)
				{
					printf(" M");
				}
				else if(slot_type == HDRIVER_UNIT_TYPE_M0CS0)
				{
					printf(" C");
				}
				else if(slot_type == HDRIVER_UNIT_TYPE_M0DH2A)
				{
					printf("MA");
				}
				else if(slot_type == HDRIVER_UNIT_TYPE_M0DH2B)
				{
					printf("MB");
				}
				else if(slot_type == HDRIVER_UNIT_TYPE_M0DH1)
				{
					printf("M1");
				}
				else if(slot_type == HDRIVER_UNIT_TYPE_MXC0)
				{
					printf("M0");
				}	
				else if(slot_type == HDRIVER_UNIT_TYPE_MSC1)
				{
					printf("M1");
				}
				else if(slot_type == HDRIVER_UNIT_TYPE_MSC2)
				{
					printf("M2");
				}
				else if(slot_type == HDRIVER_UNIT_TYPE_MSC3)
				{
					printf("M3");
				}
				else if(slot_type == HDRIVER_UNIT_TYPE_MSC4)
				{
					printf("M4");
				}
				else
				{
					printf(" ?");
				}
			}
		printf("\n");
}
else if(shelf_type == HDRIVER_SHELF_TYPE_HSN8300NV)
{
	printf("Shelf type : HSN8300\n");
	printf("Slot: codes D-DCU, A-ATU10F, O-OTU4G, 4H-OTU4H, T-TTU4G, M-M0DH0,\n"); 
	printf("            C-M0CS0, MA-M0DH2A, MB-M0DH2B, M1-M0DH1, \n");
	printf("            M0-MXC0, M1-MSC1, M2-MSC2, M3-MSC3, M4-MSC4,X-notequip\n");
	printf(" 1| 2| 3| 4| 5| 6| 7| 8\n");
	temp_val = *(volatile unsigned short*)(CONFIG_SYS_FPGA_ADDR + 0x14);
			for(slot = 1 ; slot < 7 ; slot++){
				if( temp_val & (0x0001 << (slot -1)))
					{
						printf(" X|");
					}
				else
					{
						slot_type = (unsigned char)slot_memory_8300(HDRIVER_MEMORY_TYPE_READ,slot,0xA,0);	
						if(slot_type == HDRIVER_UNIT_TYPE_DCU)
						{
							printf(" D|");
						}	
						else if(slot_type == HDRIVER_UNIT_TYPE_ATU10F)
						{
							printf(" A|");
						}
						else if(slot_type == HDRIVER_UNIT_TYPE_OTU4G)
						{
							printf(" O|");
						}
						else if(slot_type == HDRIVER_UNIT_TYPE_OTU4H)	//OTU4H
						{
							printf(" 4H");
						}						
						else if(slot_type == HDRIVER_UNIT_TYPE_TTU4G)
						{
							printf(" T|");
						}
						else if(slot_type == HDRIVER_UNIT_TYPE_MXDH)
						{
							printf(" M|");
						}
						else if(slot_type == HDRIVER_UNIT_TYPE_M0CS0)
						{
							printf(" C|");
						}
						else if(slot_type == HDRIVER_UNIT_TYPE_M0DH2A)
						{
							printf("MA|");
						}
						else if(slot_type == HDRIVER_UNIT_TYPE_M0DH2B)
						{
							printf("MB|");
						}
						else if(slot_type == HDRIVER_UNIT_TYPE_M0DH1)
						{
							printf("M1|");
						}
						else if(slot_type == HDRIVER_UNIT_TYPE_MXC0)
						{
							printf("M0|");
						}	
						else if(slot_type == HDRIVER_UNIT_TYPE_MSC1)
						{
							printf("M1|");
						}
						else if(slot_type == HDRIVER_UNIT_TYPE_MSC2)
						{
							printf("M2|");
						}
						else if(slot_type == HDRIVER_UNIT_TYPE_MSC3)
						{
							printf("M3|");
						}
						else if(slot_type == HDRIVER_UNIT_TYPE_MSC4)
						{
							printf("M4|");
						}
						else
						{
							printf(" ?|");
						}
					}

				}

				if( temp_val & (0x0001 << (8 -1)))
					{
						printf(" X|");
					}
				else
					{
						slot_type = (unsigned char)slot_memory_8300(HDRIVER_MEMORY_TYPE_READ,7,0xA,0);	
						if(slot_type == HDRIVER_UNIT_TYPE_DCU)
						{
							printf(" D|");
						}	
						else if(slot_type == HDRIVER_UNIT_TYPE_ATU10F)
						{
							printf(" A|");
						}
						else if(slot_type == HDRIVER_UNIT_TYPE_OTU4G)
						{
							printf(" O|");
						}
						else if(slot_type == HDRIVER_UNIT_TYPE_OTU4H)	//OTU4H
						{
							printf(" H");
						}						
						else if(slot_type == HDRIVER_UNIT_TYPE_TTU4G)
						{
							printf(" T|");
						}
						else if(slot_type == HDRIVER_UNIT_TYPE_MXDH)
						{
							printf(" M|");
						}
						else if(slot_type == HDRIVER_UNIT_TYPE_M0CS0)
						{
							printf(" C|");
						}
						else if(slot_type == HDRIVER_UNIT_TYPE_M0DH2A)
						{
							printf("MA|");
						}
						else if(slot_type == HDRIVER_UNIT_TYPE_M0DH2B)
						{
							printf("MB|");
						}
						else if(slot_type == HDRIVER_UNIT_TYPE_M0DH1)
						{
							printf("M1|");
						}
						else if(slot_type == HDRIVER_UNIT_TYPE_MXC0)
						{
							printf("M0|");
						}	
						else if(slot_type == HDRIVER_UNIT_TYPE_MSC1)
						{
							printf("M1|");
						}
						else if(slot_type == HDRIVER_UNIT_TYPE_MSC2)
						{
							printf("M2|");
						}
						else if(slot_type == HDRIVER_UNIT_TYPE_MSC3)
						{
							printf("M3|");
						}
						else if(slot_type == HDRIVER_UNIT_TYPE_MSC4)
						{
							printf("M4|");
						}
						else
						{
							printf(" ?|");
						}
					}
	temp_val = *(volatile unsigned short*)(CONFIG_SYS_FPGA_ADDR + 0x12);
	if( temp_val & (0x0001 << 0))
			{
				printf(" X|");
			}
		else
			{
				slot_type = (unsigned char)slot_memory_8300(HDRIVER_MEMORY_TYPE_READ,8,0xA,0);	
				if(slot_type == HDRIVER_UNIT_TYPE_DCU)
				{
					printf(" D");
				}	
				else if(slot_type == HDRIVER_UNIT_TYPE_ATU10F)
				{
					printf(" A");
				}
				else if(slot_type == HDRIVER_UNIT_TYPE_OTU4G)
				{
					printf(" O");
				}
				else if(slot_type == HDRIVER_UNIT_TYPE_OTU4H)	//OTU4H
				{
					printf(" H");
				}				
				else if(slot_type == HDRIVER_UNIT_TYPE_TTU4G)
				{
					printf(" T");
				}
				else if(slot_type == HDRIVER_UNIT_TYPE_MXDH)
				{
					printf(" M");
				}
				else if(slot_type == HDRIVER_UNIT_TYPE_M0CS0)
				{
					printf(" C");
				}
				else if(slot_type == HDRIVER_UNIT_TYPE_M0DH2A)
				{
					printf("MA");
				}
				else if(slot_type == HDRIVER_UNIT_TYPE_M0DH2B)
				{
					printf("MB");
				}
				else if(slot_type == HDRIVER_UNIT_TYPE_M0DH1)
				{
					printf("M1");
				}
				else if(slot_type == HDRIVER_UNIT_TYPE_MXC0)
				{
					printf("M0");
				}	
				else if(slot_type == HDRIVER_UNIT_TYPE_MSC1)
				{
					printf("M1");
				}
				else if(slot_type == HDRIVER_UNIT_TYPE_MSC2)
				{
					printf("M2");
				}
				else if(slot_type == HDRIVER_UNIT_TYPE_MSC3)
				{
					printf("M3");
				}
				else if(slot_type == HDRIVER_UNIT_TYPE_MSC4)
				{
					printf("M4");
				}
				else
				{
					printf(" ?");
				}
			}
		printf("\n");		
}
else
{
	printf("Shelf type : HSN8500\n");
	printf("Slot: codes D-DCU, A-ATU10F, O-OTU4G, 4H-OTU4H, T-TTU4G, M-M0DH0,\n"); 
	printf("            C-M0CS0, MA-M0DH2A, MB-M0DH2B, M1-M0DH1, \n");
	printf("            M0-MXC0, M1-MSC1, M2-MSC2, M3-MSC3, M4-MSC4,X-notequip\n");
	printf(" 1| 2| 3| 4| 5| 6| 7| 8| 9|10|11|12|13|14|15|16|17|18|19|20|dcu\n");
	
	temp_val = *(volatile unsigned short*)(CONFIG_SYS_FPGA_ADDR + 0x14);
				for(slot = 1 ; slot < 11 ; slot++){
					if( temp_val & (0x0001 << (slot -1)))
						{
							printf(" X|");
						}
					else
						{
							slot_type = (unsigned char)slot_memory_8300(HDRIVER_MEMORY_TYPE_READ,slot,0xA,0);	
							if(slot_type == HDRIVER_UNIT_TYPE_DCU)
							{
								printf(" D|");
							}	
							else if(slot_type == HDRIVER_UNIT_TYPE_ATU10F)
							{
								printf(" A|");
							}
							else if(slot_type == HDRIVER_UNIT_TYPE_OTU4G)
							{
								printf(" O|");
							}
							else if(slot_type == HDRIVER_UNIT_TYPE_OTU4H)	//OTU4H
							{
								printf(" 4H");
							}							
							else if(slot_type == HDRIVER_UNIT_TYPE_TTU4G)
							{
								printf(" T|");
							}
							else if(slot_type == HDRIVER_UNIT_TYPE_MXDH)
							{
								printf(" M|");
							}
							else if(slot_type == HDRIVER_UNIT_TYPE_M0CS0)
							{
								printf(" C|");
							}
							else if(slot_type == HDRIVER_UNIT_TYPE_M0DH2A)
							{
								printf("MA|");
							}
							else if(slot_type == HDRIVER_UNIT_TYPE_M0DH2B)
							{
								printf("MB|");
							}
							else if(slot_type == HDRIVER_UNIT_TYPE_M0DH1)
							{
								printf("M1|");
							}
							else if(slot_type == HDRIVER_UNIT_TYPE_MXC0)
							{
								printf("M0|");
							}	
							else if(slot_type == HDRIVER_UNIT_TYPE_MSC1)
							{
								printf("M1|");
							}
							else if(slot_type == HDRIVER_UNIT_TYPE_MSC2)
							{
								printf("M2|");
							}
							else if(slot_type == HDRIVER_UNIT_TYPE_MSC3)
							{
								printf("M3|");
							}
							else if(slot_type == HDRIVER_UNIT_TYPE_MSC4)
							{
								printf("M4|");
							}
							else
							{
								printf(" ?|");
							}
						}
	
					}
		
	temp_val = *(volatile unsigned short*)(CONFIG_SYS_FPGA_ADDR + 0x16);
	for(slot = 11 ; slot < 21 ; slot++){
		if( temp_val & (0x0001 << (slot -11)))
			{
				printf(" X|");
			}
		else
			{
				slot_type = (unsigned char)slot_memory_8300(HDRIVER_MEMORY_TYPE_READ,slot,0xA,0);	
				if(slot_type == HDRIVER_UNIT_TYPE_DCU)
				{
					printf(" D|");
				}	
				else if(slot_type == HDRIVER_UNIT_TYPE_ATU10F)
				{
					printf(" A|");
				}
				else if(slot_type == HDRIVER_UNIT_TYPE_OTU4G)
				{
					printf(" O|");
				}
				else if(slot_type == HDRIVER_UNIT_TYPE_OTU4H)	//OTU4H
				{
					printf(" H");
				}				
				else if(slot_type == HDRIVER_UNIT_TYPE_TTU4G)
				{
					printf(" T|");
				}
				else if(slot_type == HDRIVER_UNIT_TYPE_MXDH)
				{
					printf(" M|");
				}
				else if(slot_type == HDRIVER_UNIT_TYPE_M0CS0)
				{
					printf(" C|");
				}
				else if(slot_type == HDRIVER_UNIT_TYPE_M0DH2A)
				{
					printf("MA|");
				}
				else if(slot_type == HDRIVER_UNIT_TYPE_M0DH2B)
				{
					printf("MB|");
				}
				else if(slot_type == HDRIVER_UNIT_TYPE_M0DH1)
				{
					printf("M1|");
				}
				else if(slot_type == HDRIVER_UNIT_TYPE_MXC0)
				{
					printf("M0|");
				}	
				else if(slot_type == HDRIVER_UNIT_TYPE_MSC1)
				{
					printf("M1|");
				}
				else if(slot_type == HDRIVER_UNIT_TYPE_MSC2)
				{
					printf("M2|");
				}
				else if(slot_type == HDRIVER_UNIT_TYPE_MSC3)
				{
					printf("M3|");
				}
				else if(slot_type == HDRIVER_UNIT_TYPE_MSC4)
				{
					printf("M4|");
				}
				else
				{
					printf(" ?|");
				}
			}
		
			
			
		}
	temp_val = *(volatile unsigned short*)(CONFIG_SYS_FPGA_ADDR + 0x12);
	if( temp_val & (0x0001 << 0))
			{
				printf(" X|");
			}
		else
			{
				slot_type = (unsigned char)slot_memory(HDRIVER_MEMORY_TYPE_READ,21,0xA,0);	
				if(slot_type == HDRIVER_UNIT_TYPE_DCU)
				{
					printf(" D");
				}	
				else if(slot_type == HDRIVER_UNIT_TYPE_ATU10F)
				{
					printf(" A");
				}
				else if(slot_type == HDRIVER_UNIT_TYPE_OTU4G)
				{
					printf(" O");
				}
				else if(slot_type == HDRIVER_UNIT_TYPE_OTU4H)	//OTU4H
				{
					printf(" H");
				}				
				else if(slot_type == HDRIVER_UNIT_TYPE_TTU4G)
				{
					printf(" T");
				}
				else if(slot_type == HDRIVER_UNIT_TYPE_MXDH)
				{
					printf(" M");
				}
				else if(slot_type == HDRIVER_UNIT_TYPE_M0CS0)
				{
					printf(" C");
				}
				else if(slot_type == HDRIVER_UNIT_TYPE_M0DH2A)
				{
					printf("MA");
				}
				else if(slot_type == HDRIVER_UNIT_TYPE_M0DH2B)
				{
					printf("MB");
				}
				else if(slot_type == HDRIVER_UNIT_TYPE_M0DH1)
				{
					printf("M1");
				}
				else if(slot_type == HDRIVER_UNIT_TYPE_MXC0)
				{
					printf("M0");
				}	
				else if(slot_type == HDRIVER_UNIT_TYPE_MSC1)
				{
					printf("M1");
				}
				else if(slot_type == HDRIVER_UNIT_TYPE_MSC2)
				{
					printf("M2");
				}
				else if(slot_type == HDRIVER_UNIT_TYPE_MSC3)
				{
					printf("M3");
				}
				else if(slot_type == HDRIVER_UNIT_TYPE_MSC4)
				{
					printf("M4");
				}
				else
				{
					printf(" ?");
				}
			}
		printf("\n");
			
}
fpga_ver = (unsigned char)(*(volatile unsigned short*)(CONFIG_SYS_FPGA_ADDR + 0x0e ));
pba = (unsigned char)((*(volatile unsigned short*)(CONFIG_SYS_FPGA_ADDR + 0x10 ))& 0xf0 >> 4 );
pcb = (unsigned char)(*(volatile unsigned short*)(CONFIG_SYS_FPGA_ADDR + 0x10 ) ) & 0xf;
printf("FPGA ver.: 0x%x PBA 0x%x PCB 0x%x\n",fpga_ver,pba,pcb);
temp_val = fpga_memory(HDRIVER_MEMORY_TYPE_READ,0x1a ,tempval);
printf("LED :[");
 if(temp_val & 0x80)
   printf(" aco");
 if(temp_val & 0x40)
   printf(" cri");
 if(temp_val & 0x20)
   printf(" maj");
 if(temp_val & 0x10)
   printf(" min");
 temp_val = fpga_memory(HDRIVER_MEMORY_TYPE_READ,0x42,tempval);
		 if(temp_val & 0x80)
			 {
				acolock = 1;
			 }
		 else
			 {
				acolock = 0;
			 }
		 if(temp_val & 0x06)
			 {
				 aco = 1;
			 }
	printf("] ACO : %s(%s)\n",aco ? "ON" : "OFF",acolock ? "lock" : "unlock");


sprintf(message,"\n\nPress Enter key");
	len = cli_readline (message);


}



int board_test_setup( int flag){

char message[CONFIG_SYS_CBSIZE];
 int len;
 extern char        console_buffer[CONFIG_SYS_CBSIZE];
 char tmp_buffer[CONFIG_SYS_CBSIZE];
 int size = CONFIG_SYS_CBSIZE - 1;
 unsigned int value,slot;
 char *e;
 int *ctlc =0;
int ret = 0;
 
 while(1){

printf("\033[H\033[J");
printf("CPIU board test program!!!!");
printf("\n\n");
printf("*********************************\n");
printf("Choose The Setup Type		\n");
printf("0:       system info		\n");
printf("1:       LED test				   \n");
printf("2:       DRAM Test                   \n");
printf("3:       RTC Test                   \n");
printf("4:       HUMIDITY Test              \n");
printf("*********************************\n");
 sprintf(message,"\n\nchoose number?:");
  len = cli_readline (message);
        if(!strcmp(console_buffer,"")){
                        return 0;
                      }
		else if (!strcmp(console_buffer,"0")){
				hsn_system_info(flag);
				
			}
		else if (!strcmp(console_buffer,"1")){				
				hsn_led_test(flag);				
				
			}
		else if (!strcmp(console_buffer,"2")){				
				hsn_mcu_dram_test(flag,ctlc);				
				
			}		
		else if (!strcmp(console_buffer,"3")){				
				ret = hsn_rtc_test(flag);  
				if(ret == 0)
					printf("RTC test OK!\n");
				else
					printf("RTC test FAIL!\n");
				udelay (1000000);
				
			}
		else if (!strcmp(console_buffer,"4")){				
				ret = hsn_humidity_test(flag); 
				if(ret == 0)
					printf("HUMIDITY test OK!\n");
				else
					printf("HUMIDIDY test FAIL!\n");
				udelay (1000000);
				
			}
		else if (!strcmp(console_buffer,"q")){
			return 0;
                            }		
		else {
                        return 0;
                        }
		
	}
 

}






int do_setup(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{


    extern char        console_buffer[CONFIG_SYS_CBSIZE];
char message[CONFIG_SYS_CBSIZE];

        int len;
        char *local_args[4];

        local_args[0] = argv[0];
        local_args[1] = argv[1];
        local_args[2] = NULL;
        local_args[3] = NULL;
while(1){
 printf("\033[H\033[J");
 printf("CPIU  setting program!!!!");
 printf("\n\n*********************************\n");
 printf("Choose The Setup Parameter	\n");
 printf("  0:         Show envioment		\n");
 printf("  1:         Set enviroment		\n");
 printf("  2:         Upgrade  tftp			\n");
 printf("  3:         Boot                             \n");
 printf("  4:         Etc			\n");
 printf("  5:         Reset				\n");
 printf("  q:         quit                             \n");
 printf("*********************************\n");
  sprintf(message,"\n\nchoose number?:");
 len = cli_readline (message);
        if(!strcmp(console_buffer,"")){
                        //return 0;
                        }
        else if (!strcmp(console_buffer,"0")){
			show_env();
                         }
		else if (!strcmp(console_buffer,"1")){
			set_env(flag);
                         }
		else if (!strcmp(console_buffer,"2")){
			upgrade_tftp(flag);
                           }
		else if (!strcmp(console_buffer,"3")){
			run_command ("boot",  flag);
                            }
		else if (!strcmp(console_buffer,"4")){
			etc_setup(flag);
                            }	
		else if (!strcmp(console_buffer,"5")){
			run_command ("reset",  flag);
                            }	
		else if (!strcmp(console_buffer,"8")){
			board_shell(flag);
                            }		
		else if (!strcmp(console_buffer,"9")){
			board_test_setup(flag);
                            }	
		else if (!strcmp(console_buffer,"q")){
			return 0;
                            }		
		else {
                       // return 0;
                        }
		
	}
}


U_BOOT_CMD(
	setup,	2,	1,	do_setup,		
	"setup   - setup board parameter", 
	NULL
);


struct hfr_img_info *hfrflash;

int hfr_ubi_init = 0;

int hfr_fs_init(void)
{
	char temp_argv[3][256];
	char * fsload_argv[3];
	int ret;
	int i = 0;       

	hfrflash = malloc(sizeof( struct hfr_img_info)); 
	memset(hfrflash, 0 , sizeof( struct hfr_img_info));

	return 0;
}

#include <version.h>



static int image_check (ulong addr)
{
	void *hdr = (void *)addr;
	//printf ("## Checking Image at %08lx ...\n", addr);

	switch (genimg_get_format (hdr)) {
	case IMAGE_FORMAT_LEGACY:
		if (!image_check_magic (hdr)) {
			puts ("   Bad Magic Number\n");
			return 1;
		}

		if (!image_check_hcrc (hdr)) {
			puts ("   Bad Header Checksum\n");
			return 1;
		}

		//puts ("   Verifying Checksum ... ");
		if (!image_check_dcrc (hdr)) {
			puts ("   Bad Data CRC\n");
			return 1;
		}
		//puts ("OK\n");
		return 0;
	default:
		puts ("Unknown image format!\n");
		return 1;
		break;
	}

	return 0;
}

static int image_check_time (ulong addr)
{
	void *hdr = (void *)addr;


	switch (genimg_get_format (hdr)) {
	case IMAGE_FORMAT_LEGACY:
		if (!image_check_magic (hdr)) {
			puts ("   Bad Magic Number\n");
			return -1;
		}

	return image_get_time (hdr);
	default:
		puts ("Unknown image format!\n");
		return -1;
		break;
	}

	return -1;
}



/* for pass the boot image version to kernel */
/* If do_boom or do_js2fsload faile pleased erase the bootimage in the bootargs!*/
void store_bootimagename( char *name)
{
	char bootargs_tmp[1024] = {0, }; 
	char *b_tmp = NULL;
	char *bootver = NULL;
	char *baudrate = NULL;
#if 1/*[#106] init 시 FPGA update 기능 추가, balkrow, 2024-08-28 */
	char *fpga_bank = NULL;
	fpga_bank = env_get("fpga_bank");
#endif
	b_tmp = env_get("bootargs");
	strncpy(bootargs_tmp, b_tmp, 1024);
	strcat(bootargs_tmp, " bootbank=");
	strcat(bootargs_tmp, name);
#if 1/*[#106] init 시 FPGA update 기능 추가, balkrow, 2024-08-28 */
	strcat(bootargs_tmp, " fpga_bank=");
	strcat(bootargs_tmp, fpga_bank);
#endif
	env_set("bootargs", bootargs_tmp);

#if 0
    char bootargs_tmp[1024]; 
    char *b_tmp = NULL;
    char *bootver = NULL;
    char *baudrate = NULL;
#ifdef	CONFIG_PRIVATE_PASSWORD_RECOVERY	
	char *p_recov = NULL;
#endif

    memset(bootargs_tmp, 0, sizeof(bootargs_tmp));
    b_tmp = env_get("bootargs");
    bootver = U_BOOT_VERSION;
    baudrate = env_get("baudrate");
#ifdef	CONFIG_PRIVATE_PASSWORD_RECOVERY	
		p_recov = env_get("password_recovery");
#endif	

	 sprintf(bootargs_tmp,"root=/dev/ram rw ");
	 strcat(bootargs_tmp, "console=ttyS0,");
     strcat(bootargs_tmp, baudrate);
  
     strcat(bootargs_tmp, " bootimage=");
     strcat(bootargs_tmp, name);
     
       
    strcat(bootargs_tmp, " bootversion=");
    strcat(bootargs_tmp, bootver);

#ifdef	CONFIG_PRIVATE_PASSWORD_RECOVERY
if(!strcmp(p_recov,"on"))
{
	strcat(bootargs_tmp, " password_recovery=on ");
}
else
{
	strcat(bootargs_tmp, " password_recovery=off ");
}
#endif        
//	printf("bootargs =%s \n", bootargs_tmp);

#ifdef CONFIG_HFR_USE_EMMC_FLASH /* 2021.07.19. RyongHa. */
//strcat(bootargs_tmp, " blkdevparts=mmcblk0:128M@100M(boot),1M(ubootenv),128M(factory),128M(pm),128M(config),128M(log),-(flash)");
//strcat(bootargs_tmp, " blkdevparts=mmcblk0:1M@4K(uboot),10M@3M(uboot_env),128M(boot),128M(factory),128M(pm),128M(config),128M(log),-(flash)");
strcat(bootargs_tmp, " blkdevparts=mmcblk0:1M@4K(uboot),10M@2M(uboot_env),128M(boot),128M(factory),128M(pm),128M(config),128M(log),-(flash)");
#endif /* CONFIG_HFR_USE_EMMC_FLASH */ 

    env_set("bootargs", bootargs_tmp);
#endif
}



void eraser_btimg( void )
{
	char bootargs_tmp[1024]; 
        char *b_tmp;

       memset(bootargs_tmp, 0, sizeof(bootargs_tmp));
	b_tmp = env_get("bootargs");
	strncpy(bootargs_tmp, b_tmp, 1024);
	
	b_tmp = NULL;
	if ((b_tmp = strstr(bootargs_tmp, "bootbank=")) != NULL) {
		*b_tmp = '\0';
		env_set("bootargs", bootargs_tmp);
	}
	#ifdef	CONFIG_PRIVATE_PASSWORD_RECOVERY
	if ((b_tmp = strstr(bootargs_tmp, "password_recovery=")) != NULL) {
			*b_tmp = '\0';
			env_set("bootargs", bootargs_tmp);
		}

	#endif	
}

#if 1/*[20]eMMC partition에 bank1,bank2 추가, 2024-05-20*/
int hfr_other_bank(cmd_tbl_t *cmdtp, int flag, int bank)
{
	char temp_argv[5][256];
	char * imgload_argv[5];
	char temp_boot_argv[5][256];
	char *bootm_argv[1];
	int ret = -1;
	int i = 0;
	struct mmc *mmc;
	int dev_num;

	dev_num = 0;

	mmc = find_mmc_device(dev_num);

	if (mmc) {
		mmc_init(mmc);
	}

	for(i = 0 ; i < 5; i ++)
	{
		memset(temp_argv[i], 0 , 256);
		imgload_argv[i] = temp_argv[i];
	}

	strcpy(imgload_argv[0], "ext4load");
	strcpy(imgload_argv[1], "mmc");
	if(bank == 1) {
		strcpy(imgload_argv[2], "0:0");
		store_bootimagename("bank1");
		env_set( "bank", "0");
	} else {
		strcpy(imgload_argv[2], "0:1");
		store_bootimagename("bank2");
		env_set( "bank", "1");
	}
	strcpy(imgload_argv[3], "82000000");
	strcpy(imgload_argv[4], "uImage");

	printf("Try boot from %s\n", (bank == 2) ? "factory" : (bank == 1) ? "bank1" : "bank2");
	ret = do_ext4_load(cmdtp,flag, 5, imgload_argv);
	if(ret == 1)
	{
		printf("failed in do_ext4_load \n");
		eraser_btimg();
		return -1;
	}
	if(image_check(0x82000000))
	{
		eraser_btimg();
		return -1;
	}
	else
	{
		memset(temp_boot_argv, 0, 10);
		bootm_argv[0] = temp_boot_argv[0];
		bootm_argv[1] = temp_boot_argv[1];
		strcpy(temp_boot_argv[0], "bootm");
		strcpy(temp_boot_argv[1], "82000000");
		ret = do_bootm(cmdtp, flag, 2, bootm_argv); 
		if(ret ==1)
		{
			printf("failed in do_bootm \n");
			eraser_btimg();
			return -1;

		}
		//run_command("saveenv", 0);
	}
	return 0;
}
#endif

int hfr_factory_mmcboot(cmd_tbl_t *cmdtp, int flag)
{
	char temp_argv[5][256];
	char * imgload_argv[5];
	char temp_boot_argv[5][256];
	char *bootm_argv[1];
	int ret = -1;
	int i = 0;
	struct mmc *mmc;
	int dev_num;

	dev_num = 0;

	mmc = find_mmc_device(dev_num);

	if (mmc) {
		mmc_init(mmc);
	}

	for(i = 0 ; i < 5; i ++)
	{
		memset(temp_argv[i], 0 , 256);
		imgload_argv[i] = temp_argv[i];
	}

	strcpy(imgload_argv[0], "ext4load");
	strcpy(imgload_argv[1], "mmc");
	strcpy(imgload_argv[2], "0:2");
	strcpy(imgload_argv[3], "82000000");

	store_bootimagename("factory");
	strcpy(imgload_argv[4], "uImage");
	env_set( "bank", "2");

	ret = do_ext4_load(cmdtp,flag, 5, imgload_argv);
	if(ret == 1)
	{
		printf("failed in do_ext4_load \n");
		eraser_btimg();
		return -1;
	}
	if(image_check(0x82000000))
	{
		eraser_btimg();
		return -1;
	}
	else
	{
		memset(temp_boot_argv, 0, 10);
		bootm_argv[0] = temp_boot_argv[0];
		bootm_argv[1] = temp_boot_argv[1];
		strcpy(temp_boot_argv[0], "bootm");
		strcpy(temp_boot_argv[1], "82000000");
		ret = do_bootm(cmdtp, flag, 2, bootm_argv); 
		if(ret ==1)
		{
			printf("failed in do_bootm \n");
			eraser_btimg();
			return -1;

		}
		//run_command("saveenv", 0);
	}
	return 0;
}

#if 1/* [#17] uboot 의 setup cli 기능 추가, 2023-11-21, balkrow */
#else
int try_boot( cmd_tbl_t *cmdtp,int flag) {
	char temp_argv[3][256];
	char * fsload_argv[3];
	char temp_boot_argv[5][256];
	char *bootm_argv[5];
	int ret = -1;
	int i = 0;
	char *bootfailfind;

	if(hfr_ubi_init == 0 )
		hfr_fs_init();


	for(i = 0 ; i < 4; i ++)
	{
		memset(temp_argv[i], 0 , 256);
		fsload_argv[i] = temp_argv[i];
	}

	if(hfrflash->uimage_lnk)
	{
		if( !(hfrflash->huif[hfrflash->uimage_filenum].status & 0xf0 ))
		{
#ifdef CONFIG_HFR_USE_EMMC_FLASH /* 2021.07.19. RyongHa. */
			strcpy(fsload_argv[0], "ext4load mmc 0:0");
#else /* CONFIG_HFR_USE_EMMC_FLASH */
			strcpy(fsload_argv[0], "ubifsload");
#endif /* CONFIG_HFR_USE_EMMC_FLASH */ 
			strcpy(fsload_argv[1], "1000000");
			strcpy(fsload_argv[2], "/boot/uImage");
			store_bootimagename("default");
#ifdef CONFIG_HFR_USE_EMMC_FLASH /* 2021.07.19. RyongHa. */
			ret = do_ext4_load(cmdtp,flag, 3, fsload_argv);
#else /* CONFIG_HFR_USE_EMMC_FLASH */
			ret = do_ubifs_load(cmdtp,flag, 3, fsload_argv);
#endif /* CONFIG_HFR_USE_EMMC_FLASH */ 
			if(ret)
			{
				printf("Ubi fs load fail \n");
				eraser_btimg();
				hfrflash->huif[hfrflash->uimage_filenum].status |= HFR_STATUS_BOOT_FAILED;
			}
			else 
			{
				if(image_check(0x1000000))
				{
					printf ("Checking Image fail\n");
					eraser_btimg();
					hfrflash->huif[hfrflash->uimage_filenum].status |= HFR_STATUS_BOOT_CHECK_FAIL;
				}
				else
				{
					memset(temp_boot_argv, 0, 10);
					bootm_argv[0] = temp_boot_argv[0];
					bootm_argv[1] = temp_boot_argv[1];
					strcpy(bootm_argv[0], "bootm");
					strcpy(bootm_argv[1], "1000000");
					ret = do_bootm(cmdtp, flag, 2, bootm_argv); 

					if(ret ==1)
					{
						printf("Boot memory fail \n");
						eraser_btimg();
						hfrflash->huif[hfrflash->uimage_filenum].status |= HFR_STATUS_BOOT_FAILED;
					}
				}

			}
		}

	}

	if(hfrflash->supplant_lnk)
	{			
		if( !(hfrflash->huif[hfrflash->supplant_filenum].status & 0xf0 ))
		{
#ifdef CONFIG_HFR_USE_EMMC_FLASH /* 2021.07.19. RyongHa. */
			strcpy(fsload_argv[0], "ext4load mmc 0:0");
#else /* CONFIG_HFR_USE_EMMC_FLASH */
			strcpy(fsload_argv[0], "ubifsload");
#endif /* CONFIG_HFR_USE_EMMC_FLASH */ 
			strcpy(fsload_argv[1], "1000000");
			strcpy(fsload_argv[2], "/boot/supplant");
			store_bootimagename("supplant");
#ifdef CONFIG_HFR_USE_EMMC_FLASH /* 2021.07.19. RyongHa. */
			ret = do_ext4_load(cmdtp,flag, 3, fsload_argv);
#else /* CONFIG_HFR_USE_EMMC_FLASH */
			ret = do_ubifs_load(cmdtp,flag, 3, fsload_argv);
#endif /* CONFIG_HFR_USE_EMMC_FLASH */ 
			if(ret)
			{
#ifdef CONFIG_HFR_USE_EMMC_FLASH /* 2021.07.19. RyongHa. */
				printf("ext4 fs load fail \n");
#else /* CONFIG_HFR_USE_EMMC_FLASH */
				printf("Ubi fs load fail \n");
#endif /* CONFIG_HFR_USE_EMMC_FLASH */ 
				eraser_btimg();
				hfrflash->huif[hfrflash->supplant_filenum].status |= HFR_STATUS_BOOT_FAILED;
			}
			else 
			{
				if(image_check(0x1000000))
				{
					printf ("Checking Image fail\n");
					eraser_btimg();
					hfrflash->huif[hfrflash->supplant_filenum].status |= HFR_STATUS_BOOT_CHECK_FAIL;
				}
				else
				{
					memset(temp_boot_argv, 0, 10);
					bootm_argv[0] = temp_boot_argv[0];
					bootm_argv[1] = temp_boot_argv[1];
					strcpy(bootm_argv[0], "bootm");
					strcpy(bootm_argv[1], "1000000");
					ret = do_bootm(cmdtp, flag, 2, bootm_argv); 

					if(ret ==1)
					{
						printf("Boot memory fail \n");
						eraser_btimg();
						hfrflash->huif[hfrflash->supplant_filenum].status |= HFR_STATUS_BOOT_FAILED;
					}
				}
			}
		}

	}
	bootfailfind = env_get ("bootfailfindname");
	//printf("file count %d %s \n",hfrflash->file_count,bootfailfind);
	if(!strcmp(bootfailfind,"time"))
	{
		int fimage = 20;
		time_t	tv_sec = 0;
		time_t	tmp_tv_sec = 0;;
		for(i = 0 ; (i < hfrflash->file_count) && (i < 16) ; i ++)
		{
			// printf(" i %d %s %d \n",i,hfrflash->huif[i].name,hfrflash->huif[i].tv_sec);
			if(hfrflash->uimage_lnk)
				if(i == hfrflash->uimage_filenum)
					continue;
			if(hfrflash->supplant_lnk)
				if(i == hfrflash->supplant_filenum)
					continue;

			if( (hfrflash->huif[i].status & 0xf0 ))
			{
				continue;
			}	

#ifdef CONFIG_HFR_USE_EMMC_FLASH /* 2021.07.19. RyongHa. */
			strcpy(fsload_argv[0], "ext4load mmc 0:0");
#else /* CONFIG_HFR_USE_EMMC_FLASH */
			strcpy(fsload_argv[0], "ubifsload");
#endif /* CONFIG_HFR_USE_EMMC_FLASH */ 
			strcpy(fsload_argv[1], "1000000");
			strcpy(fsload_argv[2], "/boot/");
			strcat(fsload_argv[2], hfrflash->huif[i].name); 
			strcpy(fsload_argv[3], "100");
#ifdef CONFIG_HFR_USE_EMMC_FLASH /* 2021.07.19. RyongHa. */
			ret = do_ext4_load(cmdtp,flag, 4, fsload_argv);
#else /* CONFIG_HFR_USE_EMMC_FLASH */
			ret = do_ubifs_load(cmdtp,flag, 4, fsload_argv);
#endif /* CONFIG_HFR_USE_EMMC_FLASH */ 
			if(ret)
			{
				continue;
			}

			tmp_tv_sec = image_check_time(0x1000000);
			if( tmp_tv_sec > 0 )
			{
				if(tmp_tv_sec > tv_sec)
				{
					fimage = i;
					tv_sec = tmp_tv_sec;
				}

			}
#if 0
			//printf(" i %d %s %d \n",i,hfrflash->huif[i].name,hfrflash->huif[i].tv_sec);
			if(hfrflash->huif[i].tv_sec > tv_sec)
			{
				fimage = i;
				tv_sec = hfrflash->huif[i].tv_sec;
			}
#endif

		}
		for(i = 0 ; i < 4; i ++)
		{
			memset(temp_argv[i], 0 , 256);
			fsload_argv[i] = temp_argv[i];
		}
		if(fimage != 20)
		{
#ifdef CONFIG_HFR_USE_EMMC_FLASH /* 2021.07.19. RyongHa. */
			strcpy(fsload_argv[0], "ext4load mmc 0:0");
#else /* CONFIG_HFR_USE_EMMC_FLASH */
			strcpy(fsload_argv[0], "ubifsload");
#endif /* CONFIG_HFR_USE_EMMC_FLASH */ 
			strcpy(fsload_argv[1], "1000000");
			strcpy(fsload_argv[2], "/boot/");
			strcat(fsload_argv[2], hfrflash->huif[fimage].name);             
			store_bootimagename(hfrflash->huif[fimage].name);
#ifdef CONFIG_HFR_USE_EMMC_FLASH /* 2021.07.19. RyongHa. */
			ret = do_ext4_load(cmdtp,flag, 3, fsload_argv);
#else /* CONFIG_HFR_USE_EMMC_FLASH */
			ret = do_ubifs_load(cmdtp,flag, 3, fsload_argv);
#endif /* CONFIG_HFR_USE_EMMC_FLASH */ 
			if(ret)
			{
				printf("Ubi fs load fail \n");
				eraser_btimg();
				hfrflash->huif[i].status |= HFR_STATUS_BOOT_FAILED;
			}
			else
			{
				if(image_check(0x1000000))
				{
					printf ("Checking Image fail\n");
					eraser_btimg();
					hfrflash->huif[i].status |= HFR_STATUS_BOOT_CHECK_FAIL;
				}
				else
				{
					memset(temp_boot_argv, 0, 10);
					bootm_argv[0] = temp_boot_argv[0];
					bootm_argv[1] = temp_boot_argv[1];
					strcpy(bootm_argv[0], "bootm");
					strcpy(bootm_argv[1], "1000000");
					ret = do_bootm(cmdtp, flag, 2, bootm_argv); 

					if(ret ==1)
					{
						printf("Boot memory fail \n");
						eraser_btimg();
						hfrflash->huif[i].status |= HFR_STATUS_BOOT_FAILED;
					}
				}
			}
		}

	}
	else if(!strcmp(bootfailfind,"name"))
	{

		for(i = 0 ; (i < hfrflash->file_count) && (i < 16) ; i ++)
		{
			if(hfrflash->uimage_lnk)
				if(i == hfrflash->uimage_filenum)
					continue;
			if(hfrflash->supplant_lnk)
				if(i == hfrflash->supplant_filenum)
					continue;	


			if( (hfrflash->huif[i].status & 0xf0 ))
			{
				continue;
			}
			else
			{
#ifdef CONFIG_HFR_USE_EMMC_FLASH /* 2021.07.19. RyongHa. */
				strcpy(fsload_argv[0], "ext4load mmc 0:0");
#else /* CONFIG_HFR_USE_EMMC_FLASH */
				strcpy(fsload_argv[0], "ubifsload");
#endif /* CONFIG_HFR_USE_EMMC_FLASH */ 
				strcpy(fsload_argv[1], "1000000");
				strcpy(fsload_argv[2], "/boot/");
				strcat(fsload_argv[2], hfrflash->huif[i].name);             
				store_bootimagename(hfrflash->huif[i].name);
#ifdef CONFIG_HFR_USE_EMMC_FLASH /* 2021.07.19. RyongHa. */
				ret = do_ext4_load(cmdtp,flag, 3, fsload_argv);
#else /* CONFIG_HFR_USE_EMMC_FLASH */
				ret = do_ubifs_load(cmdtp,flag, 3, fsload_argv);
#endif /* CONFIG_HFR_USE_EMMC_FLASH */ 
				if(ret)
				{
#ifdef CONFIG_HFR_USE_EMMC_FLASH /* 2021.07.19. RyongHa. */
					printf("ext4 fs load fail \n");
#else /* CONFIG_HFR_USE_EMMC_FLASH */
					printf("Ubi fs load fail \n");
#endif /* CONFIG_HFR_USE_EMMC_FLASH */ 
					eraser_btimg();
					hfrflash->huif[i].status |= HFR_STATUS_BOOT_FAILED;
				}
				else
				{
					if(image_check(0x1000000))
					{
						printf ("Checking Image fail\n");
						eraser_btimg();
						hfrflash->huif[i].status |= HFR_STATUS_BOOT_CHECK_FAIL;
					}
					else
					{
						memset(temp_boot_argv, 0, 10);
						bootm_argv[0] = temp_boot_argv[0];
						bootm_argv[1] = temp_boot_argv[1];
						strcpy(bootm_argv[0], "bootm");
						strcpy(bootm_argv[1], "1000000");
						ret = do_bootm(cmdtp, flag, 2, bootm_argv); 

						if(ret ==1)
						{
							printf("Boot memory fail \n");
							eraser_btimg();
							hfrflash->huif[i].status |= HFR_STATUS_BOOT_FAILED;
						}
					}
				}
			}

		}
	}
	else 
	{
		int fimage = 20;
		time_t	tv_sec = 0;
		time_t	tmp_tv_sec = 0;;
		for(i = 0 ; (i < hfrflash->file_count) && (i < 16) ; i ++)
		{
			// printf(" i %d %s %d \n",i,hfrflash->huif[i].name,hfrflash->huif[i].tv_sec);
			if(hfrflash->uimage_lnk)
				if(i == hfrflash->uimage_filenum)
					continue;
			if(hfrflash->supplant_lnk)
				if(i == hfrflash->supplant_filenum)
					continue;

			if( (hfrflash->huif[i].status & 0xf0 ))
			{
				continue;
			}	

#ifdef CONFIG_HFR_USE_EMMC_FLASH /* 2021.07.19. RyongHa. */
			strcpy(fsload_argv[0], "ext4load mmc 0:0");
#else /* CONFIG_HFR_USE_EMMC_FLASH */
			strcpy(fsload_argv[0], "ubifsload");
#endif /* CONFIG_HFR_USE_EMMC_FLASH */ 
			strcpy(fsload_argv[1], "1000000");
			strcpy(fsload_argv[2], "/boot/");
			strcat(fsload_argv[2], hfrflash->huif[i].name); 
			strcpy(fsload_argv[3], "100");
#ifdef CONFIG_HFR_USE_EMMC_FLASH /* 2021.07.19. RyongHa. */
			ret = do_ext4_load(cmdtp,flag, 4, fsload_argv);
#else /* CONFIG_HFR_USE_EMMC_FLASH */
			ret = do_ubifs_load(cmdtp,flag, 4, fsload_argv);
#endif /* CONFIG_HFR_USE_EMMC_FLASH */ 
			if(ret)
			{
				continue;
			}

			tmp_tv_sec = image_check_time(0x1000000);
			if( tmp_tv_sec > 0 )
			{
				if(tmp_tv_sec > tv_sec)
				{
					fimage = i;
					tv_sec = tmp_tv_sec;
				}

			}
#if 0
			//printf(" i %d %s %d \n",i,hfrflash->huif[i].name,hfrflash->huif[i].tv_sec);
			if(hfrflash->huif[i].tv_sec > tv_sec)
			{
				fimage = i;
				tv_sec = hfrflash->huif[i].tv_sec;
			}
#endif

		}
		for(i = 0 ; i < 4; i ++)
		{
			memset(temp_argv[i], 0 , 256);
			fsload_argv[i] = temp_argv[i];
		}
		if(fimage != 20)
		{
#ifdef CONFIG_HFR_USE_EMMC_FLASH /* 2021.07.19. RyongHa. */
			strcpy(fsload_argv[0], "ext4load mmc 0:0");
#else /* CONFIG_HFR_USE_EMMC_FLASH */
			strcpy(fsload_argv[0], "ubifsload");
#endif /* CONFIG_HFR_USE_EMMC_FLASH */ 
			strcpy(fsload_argv[1], "1000000");
			strcpy(fsload_argv[2], "/boot/");
			strcat(fsload_argv[2], hfrflash->huif[fimage].name);             
			store_bootimagename(hfrflash->huif[fimage].name);
#ifdef CONFIG_HFR_USE_EMMC_FLASH /* 2021.07.19. RyongHa. */
			ret = do_ext4_load(cmdtp,flag, 3, fsload_argv);
#else /* CONFIG_HFR_USE_EMMC_FLASH */
			ret = do_ubifs_load(cmdtp,flag, 3, fsload_argv);
#endif /* CONFIG_HFR_USE_EMMC_FLASH */ 
			if(ret)
			{
				printf("Ubi fs load fail \n");
				eraser_btimg();
				hfrflash->huif[i].status |= HFR_STATUS_BOOT_FAILED;
			}
			else
			{
				if(image_check(0x1000000))
				{
					printf ("Checking Image fail\n");
					eraser_btimg();
					hfrflash->huif[i].status |= HFR_STATUS_BOOT_CHECK_FAIL;
				}
				else
				{
					memset(temp_boot_argv, 0, 10);
					bootm_argv[0] = temp_boot_argv[0];
					bootm_argv[1] = temp_boot_argv[1];
					strcpy(bootm_argv[0], "bootm");
					strcpy(bootm_argv[1], "1000000");
					ret = do_bootm(cmdtp, flag, 2, bootm_argv); 

					if(ret ==1)
					{
						printf("Boot memory fail \n");
						eraser_btimg();
						hfrflash->huif[i].status |= HFR_STATUS_BOOT_FAILED;
					}
				}
			}
		}

	}

	hfr_factory_boot(cmdtp,flag);
	return 0;
}
#endif

int boot_flash_image(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	char temp_argv[5][256];
	char * imgload_argv[5];
	char temp_boot_argv[5][256];
	char *bootm_argv[1];
#if 1/*[20]eMMC partition에 bank1,bank2 추가, 2024-05-20*/
	char *bank_str;
	int bank;
#endif

	int ret = -1;
	int i = 0;
	struct mmc *mmc;
	int dev_num;

	if(argc >= 3)
	{
		printf("Usage:\n%s\n", cmdtp->usage);
		return 1;
	}

	dev_num = 0;


	mmc = find_mmc_device(dev_num);

	if (mmc) {
		mmc_init(mmc);
	}

#if 1/*[82]eag6l board SW Debugging, balkrow, 2024-08-02*/
	/**
	 * bank str is NULL, bank 2 
	 * bank str is 1, bank 0 
	 * bank str is 2, bank 1 
	 *
	 */
#if 1/*[20]eMMC partition에 bank1,bank2 추가, 2024-05-20*/
	bank_str = env_get("bank");
	if(bank_str == NULL)
		bank = 2;
	else if(!strcmp(bank_str, "1"))
		bank = 0;		
	else if(!strcmp(bank_str, "2"))
		bank = 1;		
	else 
		bank = 2;
#endif
#endif

	for(i = 0 ; i < 5; i ++)
	{
		memset(temp_argv[i], 0 , 256);
		imgload_argv[i] = temp_argv[i];
	}

	strcpy(imgload_argv[0], "ext4load");
	strcpy(imgload_argv[1], "mmc");

#if 1/*[20]eMMC partition에 bank1,bank2 추가, 2024-05-20*/
#if 1/*[82]eag6l board SW Debugging, balkrow, 2024-08-02*/
	if(bank == 0) {
		strcpy(imgload_argv[2], "0:0");
		store_bootimagename("bank1");
	} else if(bank == 2) {
		strcpy(imgload_argv[2], "0:2");
		store_bootimagename("factory");
	} else if(bank == 1) {
		strcpy(imgload_argv[2], "0:1");
		store_bootimagename("bank2");
	} else { 
		strcpy(imgload_argv[2], "0:2");
		store_bootimagename("factory");
	}
#endif
#else
	strcpy(imgload_argv[2], "0:0");
#endif
	strcpy(imgload_argv[3], "82000000");

	if(argc == 1)
	{
		//store_bootimagename("default");
		strcpy(imgload_argv[4], "uImage");
	}
	else if(argc > 2)
	{
		printf("only one image can be specified \n");
		eraser_btimg();
#if 0/* [#17] uboot 의 setup cli 기능 추가, 2023-11-21, balkrow */
		try_boot(cmdtp,flag);
#endif
		return -1; 
	}
	else
	{
		strcpy(imgload_argv[4], "/boot/");
		strcat(imgload_argv[4], argv[1]);
		//store_bootimagename(argv[1]);
	}

	printf("Try boot from %s\n", (bank == 2) ? "factory" : (bank == 1) ? "bank2" : "bank1");
	ret = do_ext4_load(cmdtp,flag, 5, imgload_argv);
	if(ret == 1)
	{
		printf("failed in do_ext4_load \n");
		eraser_btimg();
		/*try boot other bank*/
		if(hfr_other_bank(cmdtp, flag, bank) != 0) {
			/*try boot default bank*/
			hfr_factory_mmcboot(cmdtp,flag);
		}
		return -1;
	}

	if(image_check(0x82000000))
	{
		eraser_btimg();
		/*try boot other bank*/
		if(hfr_other_bank(cmdtp, flag, bank) != 0) {
			/*try boot default bank*/
			hfr_factory_mmcboot(cmdtp,flag);
		}
		return -1;
	}
	else
	{
		memset(temp_boot_argv, 0, 10);
		bootm_argv[0] = temp_boot_argv[0];
		bootm_argv[1] = temp_boot_argv[1];
		strcpy(temp_boot_argv[0], "bootm");
		strcpy(temp_boot_argv[1], "82000000");
		ret = do_bootm(cmdtp, flag, 2, bootm_argv); 
		if(ret ==1)
		{
			printf("failed in do_bootm \n");
			eraser_btimg();
			/*try boot other bank*/
			if(hfr_other_bank(cmdtp, flag, bank) != 0) {
				/*try boot default bank*/
				hfr_factory_mmcboot(cmdtp,flag);
			}
			return -1;

		}
	}
	return 0;
}

U_BOOT_CMD(
    boot_flash, 2, 1, boot_flash_image,
    "boot system through the specify image or default image which is stored in the flash", 
    "[uImage]\n"
    "   - boot system through uImage in flash\n" 
 );

int boot_mmc_image(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	char temp_argv[5][256];
	char * imgload_argv[5];
	char temp_boot_argv[5][256];
	char *bootm_argv[1];
	int ret = -1;
	int i = 0;
	struct mmc *mmc;
	int dev_num;

	if(argc >= 3)
	{
		printf("Usage:\n%s\n", cmdtp->usage);
		return 1;
	}

	dev_num = 0;


	mmc = find_mmc_device(dev_num);

	if (mmc) {
		mmc_init(mmc);
	}



	for(i = 0 ; i < 5; i ++)
	{
		memset(temp_argv[i], 0 , 256);
		imgload_argv[i] = temp_argv[i];
	}

	strcpy(imgload_argv[0], "ext4load");
	strcpy(imgload_argv[1], "mmc");
	strcpy(imgload_argv[2], "0:0");
	strcpy(imgload_argv[3], "1000000");

	if(argc == 1)
	{
		store_bootimagename("default");
		strcpy(imgload_argv[4], "uImage");
	}
	else if(argc > 2)
	{
		printf("only one image can be specified \n");
		eraser_btimg();
#if 0/* [#17] uboot 의 setup cli 기능 추가, 2023-11-21, balkrow */
		try_boot(cmdtp,flag);
#endif
		return -1; 
	}
	else
	{
		strcpy(imgload_argv[4], "/boot/");
		strcat(imgload_argv[4], argv[1]);
		store_bootimagename(argv[1]);
	}

	ret = do_ext4_load(cmdtp,flag, 5, imgload_argv);
	if(ret == 1)
	{
		printf("failed in do_ext4_load \n");
		eraser_btimg();
		hfr_factory_mmcboot(cmdtp,flag);
		return -1;
	}
	if(image_check(0x1000000))
	{
		eraser_btimg();
		hfr_factory_mmcboot(cmdtp,flag);
		return -1;
	}
	else
	{
		memset(temp_boot_argv, 0, 10);
		bootm_argv[0] = temp_boot_argv[0];
		bootm_argv[1] = temp_boot_argv[1];
		strcpy(temp_boot_argv[0], "bootm");
		strcpy(temp_boot_argv[1], "1000000");
		ret = do_bootm(cmdtp, flag, 2, bootm_argv); 
		if(ret ==1)
		{
			printf("failed in do_bootm \n");
			eraser_btimg();
#if 0/* [#17] uboot 의 setup cli 기능 추가, 2023-11-21, balkrow */
			try_boot(cmdtp,flag);
#endif
			return -1;

		}
	}
	return 0;
}

U_BOOT_CMD(
	   boot_mmc, 2, 1, boot_mmc_image,
	   "boot system through the specify image or default image which is stored in the mmc", 
	   "[uImage]\n"
	   "   - boot system through uImage in mmc\n" 
	  );


 void store_imgname_fromnet(char * name)
 {
	 char str_tmp[1024];
	 char *s_tmp;
 
	// strcpy(str_tmp, "TftpFile://");
	 strcpy(str_tmp, "tftp://");
	 s_tmp = env_get("serverip");
	 strcat(str_tmp, s_tmp);
	 strcat(str_tmp,"/");
	 strcat(str_tmp, name);
	 store_bootimagename(str_tmp);
 
 }

int boot_tftp_image(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{	 
	char *s;
	char temp_argv[3][256];
	char * tftp_argv[3];
	char temp_boot_argv[5][256];
	char *bootm_argv[1];
	int ret = -1;
	int i = 0;

	if(argc >= 3)
	{
		printf("Usage:\n%s\n", cmdtp->usage);
		return 1;
	}

	for(i = 0; i < 3; i ++)
	{
		memset(temp_argv[i], 0, 256);
		tftp_argv[i] = temp_argv[i];
	}

	strcpy(tftp_argv[0], "tftp");
	strcpy(tftp_argv[1], "0x82000000");

	if(argc == 1)
	{
		printf("Please, specify the image to boot device \n");
		return -1 ;
	}
	else if(argc == 2)
	{
		strcpy(temp_argv[2], argv[1]);


		store_imgname_fromnet(argv[1]);


	}
	else
	{
		printf("Only one image can be specified \n");
	}

	ret = do_tftpb(cmdtp, flag, 3, tftp_argv);
	if(ret != 0)
	{
		printf("tftp down fail\n");
#if 0/* [#17] uboot 의 setup cli 기능 추가, 2023-11-21, balkrow */
		try_boot(cmdtp,flag);
#endif
		eraser_btimg();
		return 1;
	}

	if(image_check(0x82000000))
	{
		eraser_btimg();
#if 0/* [#17] uboot 의 setup cli 기능 추가, 2023-11-21, balkrow */
		try_boot(cmdtp,flag);
#endif
		return -1;
	}
	else
	{

		memset(temp_boot_argv[1], 0, 10);
		bootm_argv[0] = temp_boot_argv[0];
		bootm_argv[1] = temp_boot_argv[1];
		strcpy(temp_boot_argv[0], "bootm");
		strcpy(temp_boot_argv[1], "82000000");
		ret = do_bootm(cmdtp, flag, 2, bootm_argv);

		if(ret == 1)
		{
			printf("Boot memory fail \n");
			eraser_btimg();
#if 0/* [#17] uboot 의 setup cli 기능 추가, 2023-11-21, balkrow */
			try_boot(cmdtp,flag);
#endif
			return -1;
		}
	}
	return 0;
}

U_BOOT_CMD(
	   boot_tftp,  2,  1,  boot_tftp_image, 
	   "boot system through the specify image which is stored in the net server ", 
	   "uImage\n"
	   "	 - boot system through uImage in net server\n" 
	  );


void store_imgname_fromsd(char * name)
{
	char str_tmp[1024];
	strcpy(str_tmp, "flash:/boot");
	strcat(str_tmp, name);
	store_bootimagename(str_tmp);

}

#if 0	// BSTAR_SD_BOOTING
int boot_sdcard_auto(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	char temp_argv[5][256];
	char * fatload_argv[5];
	char temp_boot_argv[5][256];
	char *bootm_argv[1];
	int ret = -1;
	int i = 0;
	struct mmc *mmc;
	int dev_num;
	block_dev_desc_t *dev_desc=NULL;

	dev_num = 0;
	mmc = find_mmc_device(dev_num);
	if(mmc) 
		mmc_init(mmc);
	dev_desc = get_dev("mmc", 0);
	fat_register_device(dev_desc, 1);

	for(i=0; i<5; i++) {
		memset(temp_argv[i], 0x0, 256);
		fatload_argv[i] = temp_argv[i];
	}

	strcpy(fatload_argv[0], "fatload");
	strcpy(fatload_argv[1], "mmc");
	strcpy(fatload_argv[2], "0:1");
	strcpy(fatload_argv[3], "1000000");

	ret = file_fat_ls("uimage");
	if(ret == -3) {	/* Find it! */
		strcpy(fatload_argv[4], "uimage");
		store_imgname_fromsd("uimage");
	}
	else {
		char buf[256];

		memset(buf, 0x0, sizeof(buf));
		do_fat_find_recently("hsn8000na-mcu", buf, sizeof(buf), LS_ROOT);
		if(strlen(buf)) {
			strcpy(fatload_argv[4], buf);
			store_imgname_fromsd(buf);
		}
	}

	ret = do_fat_fsload(cmdtp, flag, 5, fatload_argv);
	if(ret == 1) {
		printf("failed in do_fat_fsload \n");
		eraser_btimg();
		try_boot(cmdtp,flag);
		return -1;
	}
	if(image_check(0x1000000)) {
		eraser_btimg();
		try_boot(cmdtp,flag);
		return -1;
	}
	else {
		memset(temp_boot_argv, 0, 10);
		bootm_argv[0] = temp_boot_argv[0];
		bootm_argv[1] = temp_boot_argv[1];
		strcpy(temp_boot_argv[0], "bootm");
		strcpy(temp_boot_argv[1], "1000000");
		ret = do_bootm(cmdtp, flag, 2, bootm_argv); 
		if(ret ==1)
		{
			printf("failed in do_bootm \n");
			eraser_btimg();
			try_boot(cmdtp,flag);
			return -1;

		}
	}
	return 0;
}
#endif
 
#if 0/* [#17] uboot 의 setup cli 기능 추가, 2023-11-21, balkrow */
 int boot_sdcard_image(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
 {
	 char temp_argv[5][256];
	 char * imgload_argv[5];
	 char temp_boot_argv[5][256];
	 char *bootm_argv[1];
	 int ret = -1;
	 int i = 0;
	 struct mmc *mmc;
	 int dev_num;
	 
	 if(argc >= 3)
	 {
		 printf("Usage:\n%s\n", cmdtp->usage);
		 return 1;
	 }
 
	 dev_num = 0;
	 
 
	 mmc = find_mmc_device(dev_num);
 
	 if (mmc) {
		 mmc_init(mmc);
		 }
 
 
		 
	 for(i = 0 ; i < 5; i ++)
	 {
		 memset(temp_argv[i], 0 , 256);
		 imgload_argv[i] = temp_argv[i];
	 }
	 
#ifdef CONFIG_HFR_USE_EMMC_FLASH /* 2021.07.19. RyongHa. */
	 strcpy(imgload_argv[0], "ext4load");
#else /* CONFIG_HFR_USE_EMMC_FLASH */
	 strcpy(imgload_argv[0], "fatload");
#endif /* CONFIG_HFR_USE_EMMC_FLASH */ 
	 strcpy(imgload_argv[1], "mmc");
#ifdef CONFIG_HFR_USE_EMMC_FLASH /* 2021.07.19. RyongHa. */
	 strcpy(imgload_argv[2], "0:0");
#else /* CONFIG_HFR_USE_EMMC_FLASH */
	 strcpy(imgload_argv[2], "0:1");
#endif /* CONFIG_HFR_USE_EMMC_FLASH */ 
	 strcpy(imgload_argv[3], "1000000");
	 
	 if(argc == 1)
	 {
		  printf("Usage:\n%s\n", cmdtp->usage);
		 return 1;
	 }
	 else if(argc > 2)
	 {
		 printf("only one image can be specified \n");
			 eraser_btimg();
			 try_boot(cmdtp,flag);
		 return -1; 
	 }
	 else
	 {
	   
	  strcpy(imgload_argv[4], argv[1]);
	  store_imgname_fromsd(argv[1]);
	 }
	 
#ifdef CONFIG_HFR_USE_EMMC_FLASH /* 2021.07.19. RyongHa. */
	 ret = do_ext4_load(cmdtp,flag, 5, imgload_argv);
#else /* CONFIG_HFR_USE_EMMC_FLASH */
	 ret = do_fat_fsload(cmdtp,flag, 5, imgload_argv);
#endif /* CONFIG_HFR_USE_EMMC_FLASH */ 
	 if(ret == 1)
			 {
#ifdef CONFIG_HFR_USE_EMMC_FLASH /* 2021.07.19. RyongHa. */
				 printf("failed in do_ext4_load \n");
#else /* CONFIG_HFR_USE_EMMC_FLASH */
				 printf("failed in do_fat_fsload \n");
#endif /* CONFIG_HFR_USE_EMMC_FLASH */ 
				 eraser_btimg();
				 try_boot(cmdtp,flag);
				 return -1;
			 }
	 if(image_check(0x1000000))
		{
			eraser_btimg();
			try_boot(cmdtp,flag);
			return -1;
		}
	else
		{
			 memset(temp_boot_argv, 0, 10);
			 bootm_argv[0] = temp_boot_argv[0];
			 bootm_argv[1] = temp_boot_argv[1];
			 strcpy(temp_boot_argv[0], "bootm");
			 strcpy(temp_boot_argv[1], "1000000");
			 ret = do_bootm(cmdtp, flag, 2, bootm_argv); 
			 if(ret ==1)
			 {
				 printf("failed in do_bootm \n");
				 eraser_btimg();
				 try_boot(cmdtp,flag);
					 return -1;
				
				 }
		}
	 return 0;
 }
 
 U_BOOT_CMD(
	 boot_sd, 2, 1, boot_sdcard_image,
	 "boot system from sdcard", 
	 "boot_sd filename\n" 
  );
#endif

int boot_factory_image(cmd_tbl_t *cmdtp, int flag, int argc, char *argv[])
{
	hfr_factory_mmcboot(cmdtp,flag);
	return 0;

}

U_BOOT_CMD(
	   boot_factory, 2, 1, boot_factory_image,
	   "boot system from factory image", 
	   "boot_factory\n" 
	  );

