
#include "command.h"
#include <ctype.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>

DEFUN (i2c_read_write,
       i2c_read_write_cmd,
       "i2c (read|write) BUS DEV REG LENGTH [VAL]",
       "i2c\n"
       "read\n"
       "write\n"
       "i2c slave address\n"
       "reg address\n"
       "length (byte)\n"
       "val\n")
{
	int i2c_fd;
	unsigned int slave_addr, reg_addr, val, len, bus;
	char *endptr = NULL;

	bus = strtoul(argv[1], &endptr, 16);
	if(bus == 0) 
	{
		i2c_fd = open("/dev/i2c-0", O_RDWR);
	} else if(bus == 1) 
	{
		i2c_fd = open("/dev/i2c-1", O_RDWR);
	} else 
	{
		vty_out(vty,"i2c wrong bus number:%d\n", bus);
		return CMD_WARNING;
	}

	if(i2c_fd < 0)
	{
		vty_out(vty,"i2c open error:%s\n", strerror(errno));
		return CMD_WARNING;
	}

	slave_addr = strtoul(argv[2], &endptr, 16);
	reg_addr = strtoul(argv[3], &endptr, 16);
	len = strtoul(argv[4], &endptr, 16);
	val = strtoul(argv[5], &endptr, 16);

	if (ioctl(i2c_fd, I2C_SLAVE_FORCE, slave_addr) < 0 ) {
		vty_out(vty,"i2c slave error ioctl %d :%s\n", errno, strerror(errno));
		close (i2c_fd);
		return CMD_WARNING;
	}

	if (!strcmp(argv[0], "read"))			
	{
		vty_out(vty,"%x chip address i2c-read 0x%x address len:%d..reading \n", slave_addr, reg_addr, len);
#if 0
		int fd,ret;
		struct i2c_msg msg[2];
		struct i2c_rdwr_ioctl_data data;
		unsigned char buf[2];
		buf[0] = reg_addr & 0xf; 

		fd = open("/dev/i2c-2", O_RDWR);

		if(fd < 0)
			return -1;

		if (ioctl(fd, I2C_SLAVE, slave_addr) < 0)
		{
			close (fd);
			return -1;
		}

		msg[0].addr = slave_addr;
		msg[0].flags = 0;
		msg[0].len   = 1;
		msg[0].buf = buf;

		msg[1].addr = slave_addr;
		msg[1].flags = 1;
		msg[1].len   = 1;
		msg[1].buf = buf;

		data.msgs = msg;
		data.nmsgs = 2;

		ret = ioctl(fd, I2C_RDWR , &data);
		vty_out(vty, "%02x\n", buf[0]);
		close (fd);
#else
		int i;
		struct i2c_msg msg;
		struct i2c_rdwr_ioctl_data msgset;
		unsigned char buf[2];
		unsigned char temp[512];

		buf[0] = reg_addr & 0xff; 
		msg.addr = slave_addr;
		msg.flags = 0;
		msg.len   = 1;
		msg.buf = buf;

		msgset.msgs  = &msg;
		msgset.nmsgs = 1;

		ioctl(i2c_fd, I2C_RDWR , &msgset );

		msg.flags = I2C_M_RD;
		msg.len = 1;

		for (i = 0; i < len; i++) 
		{
			if(!(i%16)) vty_out(vty, "\n");
			msg.buf = temp + i;
			ioctl(i2c_fd , I2C_RDWR , &msgset );
			vty_out(vty, "%02x ", temp[i]);
		}
		vty_out(vty, "\n");
#endif
	}
	else if (!strcmp(argv[0], "write")) 
	{
		vty_out(vty,"%x chip address i2c-write 0x%x address val:%x..write \n", slave_addr, reg_addr, val);
		struct i2c_msg msg;
		struct i2c_rdwr_ioctl_data msgset;
		unsigned char buf[2];
		int ret;

		buf[0] = reg_addr & 0xff; 
		buf[1] = val & 0xff; 

		msg.addr = slave_addr;
		msg.flags = 0;
		msg.len   = 2;
		msg.buf = buf;

		msgset.msgs  = &msg;
		msgset.nmsgs = 1;

		ret = ioctl(i2c_fd, I2C_RDWR , &msgset );

		if(ret < 0)
		{
			vty_out(vty, "err : %s\n", strerror(errno));
		}
	}
	else
	{
		close (i2c_fd);
		return CMD_WARNING;
	}

	close (i2c_fd);
	return CMD_SUCCESS;
}

#if 0
int i2c_read(int fd ,unsigned short dev_addr,unsigned short reg_addr)
{
	int i;
	int rc, adr, found=0;
	struct i2c_msg msg;
	struct i2c_rdwr_ioctl_data msgset;

	unsigned char temp[12];
	msg.addr = dev_addr;
	msg.flags = 0;
	msg.len   = 1;
	msg.buf = &reg_addr;

	msgset.msgs  = &msg;
	msgset.nmsgs = 1;

	ioctl(fd , I2C_RDWR , &msgset );

	msg.flags = 1;
	msg.len = 1;

	for (i = 0; i < 10; i++) {
		msg.buf = temp + i;
		ioctl(fd , I2C_RDWR , &msgset );
		vty_out(vty, "%02x ", temp[i]);
	}
	vty_out(vty, "\n");
	return 0;
} 
#endif
#if 0/* [#78] Adding system inventory management, dustin, 2024-07-24 */
extern int get_cpu_num(void);
#endif

int cmd_i2c_init()
{
	cmd_install_element (&i2c_read_write_cmd);
	return 0;
}

