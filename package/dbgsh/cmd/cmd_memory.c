#include "command.h"
#include <ctype.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include "hdriver.h"



static	unsigned int	base_address = 0x00000000;
#define DISP_LINE_LEN	16

static	unsigned int	dp_last_addr, dp_last_size;
static	unsigned int	dp_last_length = 0x40;



DEFUN (memory_display,
       memory_display_cmd,
       "md ADDRESS [<0-256>]",
       "Memory Display command\n"
       "Memory address hexa\n"
       "Length\n")
{
	
	int gebd_fd,i;
	unsigned int	addr, size, length;
	int	nbytes, linebytes;
	unsigned char	*cp;
	char *endptr = NULL;

	size = dp_last_size;
	length = dp_last_length;


	sscanf(argv[0], "%x", &addr);
	addr += base_address;
	
	size = 4;
	
	if (argc > 1){
		length = strtoul(argv[1], &endptr, 10);

		}

		
gebd_fd = open("/dev/hdrv",O_RDWR);
	if ( gebd_fd < 0 ) {
		vty_out (vty,"%%hdrv open err\n");
		return CMD_WARNING;
	}
	nbytes = length * size;
	do 
	{
		unsigned int bb[2];
		char	linebuf[DISP_LINE_LEN];
		unsigned int	*uip = (unsigned int   *)linebuf;
		unsigned short	*usp = (unsigned short *)linebuf;
		unsigned char	*ucp = (unsigned char *)linebuf;
		showmemory_t streadmemory;
		unsigned char a;
		unsigned short b;
		unsigned int c;
		long long d;

		vty_out (vty,"%08x:", (unsigned int)addr);
		linebytes = (nbytes>DISP_LINE_LEN)?DISP_LINE_LEN:nbytes;
	
		for (i=0; i<linebytes; i+= size) {
			if (size == 8) {
				streadmemory.addr = addr;streadmemory.type=8;streadmemory.dvalue=0;
				ioctl(gebd_fd, HDRIVER_IOCG_SHOW_MEMORY, &streadmemory);
				d = streadmemory.dvalue;
				*(long long*)&bb = d;
				vty_out (vty," %08x%08x", (*uip++ = bb[0]), (*uip++ = bb[1]));
			} else if (size == 4) {
				streadmemory.addr = addr;streadmemory.type=4;streadmemory.value=0;
				ioctl(gebd_fd, HDRIVER_IOCG_SHOW_MEMORY, &streadmemory);
				c = streadmemory.value;
				vty_out (vty," %08x", (*uip++ = c));
			} else if (size == 2) {
				streadmemory.addr = addr;streadmemory.type=2;streadmemory.value=0;
				ioctl(gebd_fd, HDRIVER_IOCG_SHOW_MEMORY, &streadmemory);
				b = streadmemory.value;
				vty_out (vty," %04x", (*usp++ = b));
			} else {
				streadmemory.addr = addr;streadmemory.type=1;streadmemory.value=0;
				ioctl(gebd_fd, HDRIVER_IOCG_SHOW_MEMORY, &streadmemory);
				a = streadmemory.value;
				vty_out (vty," %02x", (*ucp++ = a));
			}
			addr += size;
		}
		
		vty_out (vty,"    ");
		cp = linebuf;
		for (i=0; i<linebytes; i++) {
			if ((*cp < 0x20) || (*cp > 0x7e))
				vty_out (vty,".");
			else
				vty_out (vty,"%c", *cp);
			cp++;
		}
		vty_out (vty,"\n");
		nbytes -= linebytes;
	} while (nbytes > 0);

	dp_last_addr = addr;
	dp_last_length = length;
	dp_last_size = size;

	
	
	
	close(gebd_fd); 

	return CMD_SUCCESS;
}

DEFUN (memory_display_size,
       memory_display_size_cmd,
       "md ADDRESS (byte|word|dword|longlong) [<0-1024>]",
       "Memory Display command\n"
       "Memory address hexa\n"
       "Memory object byte 1byte\n"
       "Memory object word 2byte\n"
       "Memory object dword 4byte\n"
       "Memory object longlong 8byte\n"
       "Length\n")
{
	
	int gebd_fd,i;
	unsigned int	addr, size, length;
	int	nbytes, linebytes;
	unsigned char	*cp;
	char *endptr = NULL;

	size = dp_last_size;
	length = dp_last_length;


	sscanf(argv[0], "%x", &addr);
	addr += base_address;
	
	size = 4;
	if (!strcmp(argv[1], "byte"))			size = 1;
		else if (!strcmp(argv[1], "word"))		size = 2;
		else if (!strcmp(argv[1], "dword"))			size = 4;
		else if (!strcmp(argv[1], "longlong"))	size = 8;

	

	if (argc > 2){
		length = strtoul(argv[2], &endptr, 10);

		}

		
gebd_fd = open("/dev/hdrv",O_RDWR);
	if ( gebd_fd < 0 ) {
		vty_out (vty,"%%hdrv open err\n");
		return CMD_WARNING;
	}
	nbytes = length * size;
	do 
	{
		unsigned int bb[2];
		char	linebuf[DISP_LINE_LEN];
		unsigned int	*uip = (unsigned int   *)linebuf;
		unsigned short	*usp = (unsigned short *)linebuf;
		unsigned char	*ucp = (unsigned char *)linebuf;
		showmemory_t streadmemory;
		unsigned char a;
		unsigned short b;
		unsigned int c;
		long long d;

		vty_out (vty,"%08x:", (unsigned int)addr);
		linebytes = (nbytes>DISP_LINE_LEN)?DISP_LINE_LEN:nbytes;
	
		for (i=0; i<linebytes; i+= size) {
			if (size == 8) {
				streadmemory.addr = addr;streadmemory.type=8;streadmemory.dvalue=0;
				ioctl(gebd_fd, HDRIVER_IOCG_SHOW_MEMORY, &streadmemory);
				d = streadmemory.dvalue;
				*(long long*)&bb = d;
				vty_out (vty," %08x%08x", (*uip++ = bb[0]), (*uip++ = bb[1]));
			} else if (size == 4) {
				streadmemory.addr = addr;streadmemory.type=4;streadmemory.value=0;
				ioctl(gebd_fd, HDRIVER_IOCG_SHOW_MEMORY, &streadmemory);
				c = streadmemory.value;
				vty_out (vty," %08x", (*uip++ = c));
			} else if (size == 2) {
				streadmemory.addr = addr;streadmemory.type=2;streadmemory.value=0;
				ioctl(gebd_fd, HDRIVER_IOCG_SHOW_MEMORY, &streadmemory);
				b = streadmemory.value;
				vty_out (vty," %04x", (*usp++ = b));
			} else {
				streadmemory.addr = addr;streadmemory.type=1;streadmemory.value=0;
				ioctl(gebd_fd, HDRIVER_IOCG_SHOW_MEMORY, &streadmemory);
				a = streadmemory.value;
				vty_out (vty," %02x", (*ucp++ = a));
			}
			addr += size;
		}
		
		vty_out (vty,"    ");
		cp = linebuf;
		for (i=0; i<linebytes; i++) {
			if ((*cp < 0x20) || (*cp > 0x7e))
				vty_out (vty,".");
			else
				vty_out (vty,"%c", *cp);
			cp++;
		}
		vty_out (vty,"\n");
		nbytes -= linebytes;
	} while (nbytes > 0);

	dp_last_addr = addr;
	dp_last_length = length;
	dp_last_size = size;

	
	
	
	close(gebd_fd); 

	return CMD_SUCCESS;
}

unsigned int simple_strtoul(const char *cp,char **endp,unsigned int base)
{
	unsigned int result = 0,value;

	if (*cp == '0') {
		cp++;
		if ((*cp == 'x') && isxdigit(cp[1])) {
			base = 16;
			cp++;
		}
		if (!base) {
			base = 8;
		}
	}
	if (!base) {
		base = 10;
	}
	while (isxdigit(*cp) && (value = isdigit(*cp) ? *cp-'0' : (islower(*cp)
		? toupper(*cp) : *cp)-'A'+10) < base) {
		result = result*base + value;
		cp++;
	}
	if (endp)
		*endp = (char *)cp;
	return result;
}

long long simple_strtoull(const char *cp,char **endp,unsigned int base)
{
	long long result = 0;
	unsigned int value;

	if (*cp == '0') {
		cp++;
		if ((*cp == 'x') && isxdigit(cp[1])) {
			base = 16;
			cp++;
		}
		if (!base) {
			base = 8;
		}
	}
	if (!base) {
		base = 10;
	}
	while (isxdigit(*cp) && (value = isdigit(*cp) ? *cp-'0' : (islower(*cp)
		? toupper(*cp) : *cp)-'A'+10) < base) {
		result = result*base + value;
		cp++;
	}
	if (endp)
		*endp = (char *)cp;
	return result;
}



DEFUN (memory_write,
       memory_write_cmd,
       "mw ADDRESS VAL (byte|word|dword|longlong) [<0-1024>]",
       "Memory Write command\n"
       "Memory address hexa\n"
       "WRITE VALUE\n"
       "VALUE COUNT\n"
       "Memory object byte 1byte\n"
       "Memory object word 2byte\n"
       "Memory object dword 4byte\n"
       "Memory object longlong 8byte\n")
{
	 int gebd_fd;
   unsigned int	addr, size, writeval, count;
	writememory_t stwritememory;
	long long writevald;
	sscanf(argv[0], "%x", &addr);
		addr += base_address;

	
	size = 4;
		if (!strcmp(argv[2], "byte"))			size = 1;
			else if (!strcmp(argv[2], "word"))		size = 2;
			else if (!strcmp(argv[2], "dword"))			size = 4;
			else if (!strcmp(argv[2], "longlong"))	size = 8;

	if(size == 8)	writevald = simple_strtoull(argv[1], NULL, 16);
	else			writeval = simple_strtoul(argv[1], NULL, 16);

if(argc > 3)
	count = simple_strtoul(argv[3], NULL, 10);
else
	count = 1;

gebd_fd = open("/dev/hdrv",O_RDWR);
	if ( gebd_fd < 0 ) {
		printf("hdrv open err\n");
		return -1;
	}


if (size == 8) {
		stwritememory.addr = addr;stwritememory.type=8;stwritememory.dvalue=writevald;stwritememory.count=count;
		ioctl(gebd_fd, HDRIVER_IOCS_WRITE_MEMORY, &stwritememory);
	}
	else if (size == 4) {
		stwritememory.addr = addr;stwritememory.type=4;stwritememory.value=writeval;stwritememory.count=count;
		ioctl(gebd_fd, HDRIVER_IOCS_WRITE_MEMORY, &stwritememory);
	}	
	else if (size == 2) {
		stwritememory.addr = addr;stwritememory.type=2;stwritememory.value=(writeval<<16) & 0xffff0000;stwritememory.count=count;
		ioctl(gebd_fd, HDRIVER_IOCS_WRITE_MEMORY, &stwritememory);
	}	
	else {
		stwritememory.addr = addr;stwritememory.type=1;stwritememory.value=(writeval<<24) & 0xff000000;stwritememory.count=count;
		ioctl(gebd_fd, HDRIVER_IOCS_WRITE_MEMORY, &stwritememory);
	}	
	
	
	
	close(gebd_fd);		

return CMD_SUCCESS;


}

#if 0/*[82] eag6l board SW Debugging, balkrow, 2024-08-02*/
DEFUN (slot_display,
       slot_display_cmd,
       "slotmd ADDRESS [<0-1024>]",
       "Slot memory Display command\n"
       "Slot number\n"
       "Memory address hexa\n"
       "Length\n")
{
	
	int gebd_fd,i;
	unsigned int	addr, size, length;
	int	nbytes, linebytes;
	unsigned char	*cp;
	char *endptr = NULL;


	length = dp_last_length;

	sscanf(argv[0], "%x", &addr);
	addr += base_address;
	
	size = 2;
		
	
	if (argc > 1){
		length = strtoul(argv[1], &endptr, 10);

		}

		
gebd_fd = open("/dev/hdrv",O_RDWR);
	if ( gebd_fd < 0 ) {
		vty_out (vty,"%%hdrv open err\n");
		return CMD_WARNING;
	}
	nbytes = length * size;
	do 
	{
		unsigned int bb[2];
		char	linebuf[DISP_LINE_LEN];
		unsigned short	*usp = (unsigned short *)linebuf;
		slotmemory_t slotmemory;
		unsigned short b;

		vty_out (vty,"%08x:", (unsigned int)addr);
		linebytes = (nbytes>DISP_LINE_LEN)?DISP_LINE_LEN:nbytes;
	
		for (i=0; i<linebytes; i+= size) {
			
				slotmemory.addr = addr;slotmemory.value=0;
				ioctl(gebd_fd, HDRIVER_IOCG_SLOT_SHOW_MEMORY, &slotmemory);
				b = slotmemory.value;
				vty_out (vty," %04x", (*usp++ = b));
			
			addr += size;
		}
		
		vty_out (vty,"    ");
		cp = linebuf;
		for (i=0; i<linebytes; i++) {
			if ((*cp < 0x20) || (*cp > 0x7e))
				vty_out (vty,".");
			else
				vty_out (vty,"%c", *cp);
			cp++;
		}
		vty_out (vty,"\n");
		nbytes -= linebytes;
	} while (nbytes > 0);

	dp_last_length = length;

	close(gebd_fd); 

	return CMD_SUCCESS;
}

DEFUN (slot_write,
       slot_write_cmd,
       "slotmw ADDRESS VAL",
       "Slot memory Write command\n"
       "Slot number\n"
       "Memory address hexa\n"
       "WRITE VALUE\n")
{
	 int gebd_fd;
   unsigned int	addr, size, writeval;
	slotmemory_t slotmemory;
	char *endptr = NULL;
	sscanf(argv[0], "%x", &addr);
		addr += base_address;

	
	size = 2;
		

	writeval = simple_strtoul(argv[1], NULL, 16);


	gebd_fd = open("/dev/hdrv",O_RDWR);
	if ( gebd_fd < 0 ) {
		printf("hdrv open err\n");
		return -1;
	}

	slotmemory.addr = addr;slotmemory.value=writeval;
	ioctl(gebd_fd, HDRIVER_IOCS_SLOT_WRITE_MEMORY, &slotmemory);
	
	close(gebd_fd);		

	return CMD_SUCCESS;

}
#endif


DEFUN (fpga_display,
		fpga_display_cmd,
		"fpgamd ADDRESS [<0-1024>]",
		"Fpga memory Display command\n"
		"Memory address hexa\n"
		"Length\n")
{

	int gebd_fd,i;
	unsigned int	addr, size, length;
	int	nbytes, linebytes;
	unsigned char	*cp;
	char *endptr = NULL;


	length = dp_last_length;

	sscanf(argv[0], "%x", &addr);
	addr += base_address;

	size = 2;


	if (argc > 1){
		length = strtoul(argv[1], &endptr, 10);

	}


	gebd_fd = open("/dev/hdrv",O_RDWR);
	if ( gebd_fd < 0 ) {
		vty_out (vty,"%%hdrv open err\n");
		return CMD_WARNING;
	}
	nbytes = length * size;
	do 
	{
		unsigned int bb[2];
		char	linebuf[DISP_LINE_LEN];
		unsigned short	*usp = (unsigned short *)linebuf;
		fpgamemory_t fpgamemory;
		unsigned short b;

		vty_out (vty,"%08x:", (unsigned int)addr);
		linebytes = (nbytes>DISP_LINE_LEN)?DISP_LINE_LEN:nbytes;

		for (i=0; i<linebytes; i+= size) {
			fpgamemory.addr = addr;
			fpgamemory.value = 0;
			ioctl(gebd_fd, HDRIVER_IOCG_FPGA_SHOW_MEMORY, &fpgamemory);
			b = fpgamemory.value;
			vty_out (vty," %04x", (*usp++ = b));

			addr += size;
		}

		vty_out (vty,"    ");
		cp = linebuf;
		for (i=0; i<linebytes; i++) {
			if ((*cp < 0x20) || (*cp > 0x7e))
				vty_out (vty,".");
			else
				vty_out (vty,"%c", *cp);
			cp++;
		}
		vty_out (vty,"\n");
		nbytes -= linebytes;
	} while (nbytes > 0);

	dp_last_length = length;

	close(gebd_fd); 

	return CMD_SUCCESS;
}


DEFUN (fpga_write,
       fpga_write_cmd,
       "fpgamw ADDRESS VAL",
       "Fpga memory Write command\n"
       "Memory address hexa\n"
       "WRITE VALUE\n")
{
	int gebd_fd,i;
	unsigned int	addr, size, writeval, count;
	fpgamemory_t fpgamemory;
	char *endptr = NULL;

	sscanf(argv[0], "%x", &addr);
	addr += base_address;


	size = 2;


	writeval = simple_strtoul(argv[1], NULL, 16);



	gebd_fd = open("/dev/hdrv",O_RDWR);
	if ( gebd_fd < 0 ) {
		printf("hdrv open err\n");
		return -1;
	}

	fpgamemory.addr = addr;
#if 0/*[82] eag6l board SW Debugging, balkrow, 2024-08-02*/
	fpgamemory.type=HDRIVER_MEMORY_TYPE_WRITE;
#endif
	fpgamemory.value=writeval;
	ioctl(gebd_fd, HDRIVER_IOCS_FPGA_WRITE_MEMORY, &fpgamemory);


	close(gebd_fd);		

	return CMD_SUCCESS;


}

#if 1/*[#82] eag6l board SW Debugging, balkrow, 2024-08-14*/
DEFUN (cpld_display,
		cpld_display_cmd,
		"cpldmd ADDRESS [<0-1024>]",
		"cpldmd memory Display command\n"
		"Memory address hexa\n"
		"Length\n")
{

	int gebd_fd,i;
	unsigned int	addr, size, length;
	int	nbytes, linebytes;
	unsigned char	*cp;
	char *endptr = NULL;


	length = dp_last_length;

	sscanf(argv[0], "%x", &addr);
	addr += base_address;

	size = 2;


	if (argc > 1){
		length = strtoul(argv[1], &endptr, 10);

	}


	gebd_fd = open("/dev/hdrv",O_RDWR);
	if ( gebd_fd < 0 ) {
		vty_out (vty,"%%hdrv open err\n");
		return CMD_WARNING;
	}
	nbytes = length * size;
	do 
	{
		unsigned int bb[2];
		char	linebuf[DISP_LINE_LEN];
		unsigned short	*usp = (unsigned short *)linebuf;
		cpldmemory_t cpldmemory;
		unsigned short b;

		vty_out (vty,"%08x:", (unsigned int)addr);
		linebytes = (nbytes>DISP_LINE_LEN)?DISP_LINE_LEN:nbytes;

		for (i=0; i<linebytes; i+= size) {
			cpldmemory.addr = addr;
			cpldmemory.value = 0;
			ioctl(gebd_fd, HDRIVER_IOCG_CPLD_SHOW_MEMORY, &cpldmemory);
			b = cpldmemory.value;
			vty_out (vty," %04x", (*usp++ = b));

			addr += size;
		}

		vty_out (vty,"    ");
		cp = linebuf;
		for (i=0; i<linebytes; i++) {
			if ((*cp < 0x20) || (*cp > 0x7e))
				vty_out (vty,".");
			else
				vty_out (vty,"%c", *cp);
			cp++;
		}
		vty_out (vty,"\n");
		nbytes -= linebytes;
	} while (nbytes > 0);

	dp_last_length = length;

	close(gebd_fd); 

	return CMD_SUCCESS;
}

DEFUN (cpld_write,
       cpld_write_cmd,
       "cpldmw ADDRESS VAL",
       "Cpld memory Write command\n"
       "Memory address hexa\n"
       "WRITE VALUE\n")
{
	int gebd_fd,i;
	unsigned int	addr, size, writeval, count;
	cpldmemory_t cpldmemory;
	char *endptr = NULL;

	sscanf(argv[0], "%x", &addr);
	addr += base_address;


	size = 2;


	writeval = simple_strtoul(argv[1], NULL, 16);



	gebd_fd = open("/dev/hdrv",O_RDWR);
	if ( gebd_fd < 0 ) {
		printf("hdrv open err\n");
		return -1;
	}

	cpldmemory.addr = addr;
#if 0/*[82] eag6l board SW Debugging, balkrow, 2024-08-02*/
	fpgamemory.type=HDRIVER_MEMORY_TYPE_WRITE;
#endif
	cpldmemory.value=writeval;
	ioctl(gebd_fd, HDRIVER_IOCS_CPLD_WRITE_MEMORY, &cpldmemory);


	close(gebd_fd);		

	return CMD_SUCCESS;


}
#endif

#if 0//modified  by balkrow
DEFUN (ofiu_display,
		ofiu_display_cmd,
		"ofiumd NUM ADDRESS [<0-1024>]",
		"OFIU fpga memory Display command\n"
		"OFIU number\n"
		"Memory address hexa\n"
		"Length\n")
{

	int gebd_fd,i;
	unsigned int	addr, size, length, num, oper_type;
	int	nbytes, linebytes;
	unsigned char	*cp;
	char *endptr = NULL;

	length = dp_last_length;

	sscanf(argv[0], "%d", &num);

	if(num == 1)
		oper_type = HDRIVER_IOCG_OFIU1_SHOW_MEMORY;	
	else if(num == 2)
		oper_type = HDRIVER_IOCG_OFIU2_SHOW_MEMORY;	
	else
	{
		vty_out (vty,"num is incorrect, num must be 1 or 2\n");
		return CMD_WARNING;
	}

	sscanf(argv[1], "%x", &addr);
	addr += base_address;

	size = 2;


	if (argc > 2){
		length = strtoul(argv[2], &endptr, 10);

	}


	gebd_fd = open("/dev/hdrv",O_RDWR);
	if ( gebd_fd < 0 ) {
		vty_out (vty,"%%hdrv open err\n");
		return CMD_WARNING;
	}
	nbytes = length * size;
	do 
	{
		unsigned int bb[2];
		char	linebuf[DISP_LINE_LEN];
		unsigned short	*usp = (unsigned short *)linebuf;
		fpgamemory_t fpgamemory;
		unsigned short b;

		vty_out (vty,"%08x:", (unsigned int)addr);
		linebytes = (nbytes>DISP_LINE_LEN)?DISP_LINE_LEN:nbytes;

		for (i=0; i<linebytes; i+= size) {

			fpgamemory.addr = addr;fpgamemory.type=HDRIVER_MEMORY_TYPE_READ;fpgamemory.value=0;
			ioctl(gebd_fd, oper_type, &fpgamemory);
			b = fpgamemory.value;
			vty_out (vty," %04x", (*usp++ = b));

			addr += size;
		}

		vty_out (vty,"    ");
		cp = linebuf;
		for (i=0; i<linebytes; i++) {
			if ((*cp < 0x20) || (*cp > 0x7e))
				vty_out (vty,".");
			else
				vty_out (vty,"%c", *cp);
			cp++;
		}
		vty_out (vty,"\n");
		nbytes -= linebytes;
	} while (nbytes > 0);

	dp_last_length = length;

	close(gebd_fd); 

	return CMD_SUCCESS;
}

DEFUN (ofiu_write,
       ofiu_write_cmd,
       "ofiumw NUM ADDRESS VAL",
       "OFIU fpga memory Write command\n"
	   "OFIU number\n"
       "Memory address hexa\n"
       "WRITE VALUE\n")
{
	int gebd_fd,i;
	unsigned int	addr, size, writeval, count, oper_type, num;
	fpgamemory_t fpgamemory;
	char *endptr = NULL;

	sscanf(argv[0], "%d", &num);
#if 1/*[#67] OFIU SFP register change, balkrow, 2022-07-07*/
	if(num == 1)
		oper_type = HDRIVER_IOCG_OFIU1_WRITE_MEMORY;	
	else if(num == 2)
		oper_type = HDRIVER_IOCG_OFIU2_WRITE_MEMORY;	
	else
	{
		vty_out (vty,"num is incorrect, num must be 1 or 2\n");
		return CMD_WARNING;
	}
#endif
	sscanf(argv[1], "%x", &addr);
	addr += base_address;


	size = 2;


	writeval = simple_strtoul(argv[2], NULL, 16);

	gebd_fd = open("/dev/hdrv",O_RDWR);
	if ( gebd_fd < 0 ) {
		printf("hdrv open err\n");
		return -1;
	}

	fpgamemory.addr = addr;fpgamemory.type=HDRIVER_MEMORY_TYPE_WRITE;fpgamemory.value=writeval;
	ioctl(gebd_fd, oper_type, &fpgamemory);


	close(gebd_fd);		

	return CMD_SUCCESS;


}
#endif


DEFUN (rtl8368_read_write,
       rtl8368_read_write_cmd,
       "rtl8368 (read|write) REG [VAL]",
       "rtl8368\n"
       "read\n"
       "write\n"
       "device address\n"
       "reg address\n")
{

	int gebd_fd;
	unsigned int   reg,val;
	hdriver_gpio_spi_rtl8368_data_t hg8368;
	
	vty_out(vty," %s %s %s \n",argv[0], argv[1], argv[2]);	

	gebd_fd = open("/dev/hdrv",O_RDWR);
	if ( gebd_fd < 0 ) {
		printf("hdrv open err\n");
		return -1;
	}
//	reg = strtoul(argv[1], &endptr, 16);
	reg = simple_strtoul(argv[1], NULL, 16);
//	sscanf(argv[1], "%x", &reg);

	if (!strcmp(argv[0], "read"))			
	{
		
		hg8368.s_addr = reg;
		hg8368.alen = 2;
		hg8368.dlen = 2;
		ioctl(gebd_fd, HDRIVER_IOCG_GPIO_SPI_RTL8368_READ, &hg8368);
		vty_out(vty,"REG[0x%x]=0x%x\n",reg,hg8368.value);	

	}
	else if (!strcmp(argv[0], "write")) 
	{
		val = simple_strtoul(argv[2], NULL, 16);
//		val = strtoul(argv[2], &endptr, 16);
		hg8368.s_addr = reg;
		hg8368.alen = 2;
		hg8368.dlen = 2;
		hg8368.value = val;
		ioctl(gebd_fd, HDRIVER_IOCS_GPIO_SPI_RTL8368_WRITE, &hg8368);
	}
	else
	{
		close (gebd_fd);
		return CMD_WARNING;
	}
	
	close (gebd_fd);

	return CMD_SUCCESS;
}




int cmd_memory_init()
{
  /* Each node's basic commands. */
 
  cmd_install_element ( &memory_display_cmd);
  cmd_install_element ( &memory_display_size_cmd);
  cmd_install_element ( &memory_write_cmd);
//  cmd_install_element ( &rtl8368_read_write_cmd);
  //cmd_install_element ( &slot_display_cmd);
  //cmd_install_element ( &slot_write_cmd);
  cmd_install_element ( &fpga_display_cmd);
  cmd_install_element ( &fpga_write_cmd);

#if 1/*[#82] eag6l board SW Debugging, balkrow, 2024-08-14*/
  cmd_install_element ( &cpld_display_cmd);
  cmd_install_element ( &cpld_write_cmd);
#endif

#if 0/*[82] eag6l board SW Debugging, balkrow, 2024-08-02*/
  cmd_install_element ( &ofiu_display_cmd);
  cmd_install_element ( &ofiu_write_cmd);
#endif

  
  return 0;
}


