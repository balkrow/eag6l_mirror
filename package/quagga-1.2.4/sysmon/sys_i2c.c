#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>

#include <stdarg.h>
#include <sys/ioctl.h>

#include <errno.h>
#include <limits.h>
#include <unistd.h>

#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <math.h>

#include "log.h"
#include "sysmon.h"
#include "bp_regs.h"
#include "sys_i2c.h"

#if 1   /* FIXME, martin, 2022.05.11 [BEGIN] */
#define IF_MIN_UNI_PORT_NUM     1
#define IF_MAX_UNI_PORT_NUM     24
#define IF_MIN_NNI_PORT_NUM     25
#define IF_MAX_NNI_PORT_NUM     28

#define IF_MIN_SLOT_NUM       1
#define IF_MAX_SLOT_NUM       1
#define IF_MIN_PORT_NUM       1
#define IF_MAX_PORT_NUM       PORT_ID_EAG6L_MAX
#define IF_AGGR_MAX_NUM       IF_TOTAL_NUM  /* 12 */
#define IF_AGGR_CLI_RANGE     "<1-12>"

#define HZ_I2C_SFP_INFO       128
#define HZ_I2C_SFP_DIAG       128

#define SFP_IIC_ADDR      0x50
#define DIAG_SFP_IIC_ADDR 0x51
#define CPLD_IIC_ADDR     0x55

#define I2C_MUX                    0x70
#if 1 /* [#84] Fixing for PM counters and I2C on Target system, dustin, 2024-07-30 */
#define I2C_MUX_100G_MASK          0x80 /* 100G port bit mask */
#else
#define I2C_MUX2                   0x71
#endif

#define SFP_INS1                   0x20
#define SFP_INS2                   0x21

int module_is_hisense;
int module_is_superxon;
int module_is_gpon;
#endif /* FIXME, martin, 2022.05.11 [END] */

#if 1/*[#54] Adding Smart T-SFP I2C functions, dustin, 2024-06-13 */
extern port_status_t PORT_STATUS[PORT_ID_EAG6L_MAX];
#endif
#if 1/* [#72] Adding omitted rtWDM related registers, dustin, 2024-06-27 */
extern u8 get_eag6L_dport(u8 lport);
#endif

typedef struct __raw_gbic_info__
{
    /// BASE ID FIELDS

    unsigned char   identifier;     // address 0
    unsigned char   ext_identifier;
    unsigned char   connector;
    unsigned char   transceiver[8];

    unsigned char   encoding;       // address 11
    unsigned char   br_nominal;
    unsigned char   resv1;
    unsigned char   length_km;
    unsigned char   length_100m;
    unsigned char   length_10m_50;
    unsigned char   length_10m_62_5;
    unsigned char   length_10m_copper;
    unsigned char   resv2;          // address 19

    unsigned char   vendor_name[ 16 ];
    unsigned char   resv3;
    unsigned char   vendor_oui[ 3 ];
    unsigned char   vendor_pn[ 16 ];
    unsigned char   vendor_rev[ 4 ];
    unsigned char   wavelength[ 2 ];
    unsigned char   resv4;
    unsigned char   CC_BASE;        // address 63

    // EXTENDED ID FIELDS

    unsigned char   options[ 2 ];
    unsigned char   br_max;
    unsigned char   br_min;
    unsigned char   vendor_sn[ 16 ];
    unsigned char   date_code[ 8 ];
    unsigned char   diagnostic_monitoring_type;
    unsigned char   enhanced_options;
    unsigned char   sff_8472_compliance;
    unsigned char   CC_EXT;

    // VENDOR SPECIFIC ID FIELDS

    unsigned char   vendor_specific[ 32 ];
}RawGbicInfo;

typedef struct __raw_gbic_diag_info__
{
  // Diagnostics: Data Fields - Address A2h

  // Alarm and Warning Thresholds
  unsigned char aw_thresholds[40];    // address 0
  unsigned char unallocated_16[16];

  // Calibration constants for External Calibration Option
  unsigned char ext_cal_constants[36];
  unsigned char unallocated_3[3];
  unsigned char cc_dmi;

  // A/D Values and Status Bits
  unsigned char diagnostics[10];        // byte 96~105 : temp, vcc, tx bias, tx/rx power
  unsigned char optional_diag[4];       // byte 106~109 : laser temp, tec curr
  unsigned char status_control;
  unsigned char reserved;

  // Alarm and Warning Flag Bits
  unsigned char alarm_flags[2];
  unsigned char tx_input_eq_control;
  unsigned char rx_out_emphasis_control;
  unsigned char warning_flags[2];

  // Extended Control/Status Memory Addresses
  unsigned char ext_status_control[2];

  // Vendor Specific Memory Addresses
  unsigned char vendor_specific[7];
  unsigned char table_select;

  // page 00-01h
  // User EEPROM
  unsigned char user_eeprom[120];	/*128-247*/

  // Vendor Control Function Addresses
    unsigned char vendor_control[8];	/*248-255*/
} RawGbicDiagInfo;

typedef struct __raw_qsfp28_lower_page_0__
{
	unsigned char	identifier;	/*0*/
	unsigned char	status[2];	/*1-2*/
	unsigned char	intr_flag[19];	/*3-21*/
	unsigned char	fs_dev_monitor[12];	/*22-33*/
	unsigned char	chann_monitor[48];	/*34-81*/
	unsigned char	reserved1[4];	/*82-85*/
	unsigned char	control[13];	/*86-98*/
	unsigned char	reserved2;	/*99*/
	unsigned char	fs_dev_chan_mask[5];	/*100-104*/
	unsigned char	vendor_specific[2];	/*105-106*/
	unsigned char	reserved3;	/*107*/
	unsigned char	fs_dev_property[3];	/*108-110*/
	unsigned char	pci_express[2];	/*111-112*/
	unsigned char	fs_dev_property2;	/*113*/
	unsigned char	reserved4[5];	/*114-118*/
	unsigned char	pwd_chg_entry[4];	/*119-122*/
	unsigned char	pwd_entry[4];	/*123-126*/
	unsigned char	page_select;	/*127*/
} RawQsfpLowerPage0;

typedef struct __raw_qsfp28_upper_page_0__
{
	unsigned char	identifier;	/*128*/
	unsigned char	ext_identifier;	/*129*/
	unsigned char	connector_type;	/*130*/
	unsigned char	spec_compliance[8];	/*131-138*/
	unsigned char	encoding;	/*139*/
	unsigned char	br_nominal;	/*140*/
	unsigned char	ext_rate_select;	/*141*/
	unsigned char	len_smf;	/*142*/
	unsigned char	len_om3_50um;	/*143*/
	unsigned char	len_om2_50um;	/*144*/
	unsigned char	len_om1_62dot5um;	/*145*/
	unsigned char	len_etc;	/*146*/
	unsigned char	dev_tech;	/*147*/
	unsigned char	vendor_name[16];	/*148-163*/
	unsigned char	ext_module;	/*164*/
	unsigned char	vendor_oui[3];	/*165-167*/
	unsigned char	vendor_pn[16];	/*168-183*/
	unsigned char	vendor_rev[2];	/*184-185*/
	unsigned char	wavelength[2];	/*186-187*/
	unsigned char	wl_tolerance[2];	/*188-189*/
	unsigned char	max_temp;	/*190*/
	unsigned char	cc_base;	/*191*/
	unsigned char	link_code;	/*192*/
	unsigned char	options[3];	/*193-195*/
	unsigned char	vendor_sn[16];	/*196-211*/
	unsigned char	date_code[8];	/*212-219*/
	unsigned char	diag_type;	/*220*/
	unsigned char	enhanced_option;	/*221*/
	unsigned char	br_nominal2;	/*222*/
	unsigned char	cc_ext;	/*223*/
	unsigned char	vendor_specific[32];	/*224-255*/
} RawQsfp28UpperPage0;

/* function declaration */
void i2c_set_sfp_channel_no(int bus, int portno);


void cprintf (const char *format, ...)
{
	int rc, fd;
	va_list args;

	char line[1024];
	va_start(args, format);
	rc = vsprintf(line, format, args);
	va_end(args);
	fd = open("/dev/console", O_RDWR, 0);
	write(fd, line, rc);
	close(fd);

	return;
}

typedef unsigned char uchar;
typedef unsigned char uint8_t;
typedef __u16 uint16_t;

typedef struct _slot_sfp_info_t           /* Maybe used to get SFP information in a slot */
{
	uchar   slot_num;   /* slot num to get SFP information */
	uint    port_stat;   /* port insertion status */
	uint    read_stat;   /* SFP port read status 1 = ok, 0 = not ok P8-P1 */
	uchar   data         [IF_MAX_PORT_NUM][HZ_I2C_SFP_INFO + HZ_I2C_SFP_DIAG]; /* SFP information + DDM field */
#if 1/* [#72] Adding omitted rtWDM related registers, dustin, 2024-06-27 */
	uchar   rtwdm_data   [IF_MAX_PORT_NUM][HZ_I2C_SFP_INFO + HZ_I2C_SFP_DIAG];
#endif
} slot_sfp_info_t;

static struct _slot_sfp_info_t slot_sfp_info;

#if 1/*[#61] Adding omitted functions, dustin, 2024-06-24 */
#define MAX_CHANNEL_NO		40
double WAVELENGTH_25G_TBL[MAX_CHANNEL_NO] = 
{
	1560.61, 1559.79, 1558.98, 1558.17, 1557.36,
	1556.55, 1555.75, 1554.94, 1554.13, 1553.33,
	1552.52, 1551.72, 1550.92, 1550.12, 1549.32,
	1548.51, 1547.72, 1546.92, 1546.12, 1545.32,
	1544.53, 1543.73, 1542.94, 1542.14, 1541.35,
	1540.56, 1539.77, 1538.98, 1538.19, 1537.40,
	1536.61, 1535.82, 1535.04, 1534.25, 1533.47,
	1532.68, 1531.90, 1531.12, 1530.33, 1529.55,
};

double COT_WAVELENGTH_10G_TBL[MAX_CHANNEL_NO] = 
{
	1560.769, 1559.957, 1559.145, 1558.335, 1557.525,
	1556.716, 1555.909, 1555.101, 0,/*0x9*/ 1553.490,
	1552.685, 1551.881, 1551.079, 1550.276, 1549.475,
	1548.675, 1547.875, 0,/*0x12*/ 0,/*0x13*/ 0,/*0x14*/
	0,/*0x15*/ 0,/*0x16*/ 0,/*0x17*/ 1542.301, 1541.508,
	1540.716, 1539.924, 1539.134, 1538.344, 1537.555,
	1536.767, 0,/*0x20*/ 1535.193, 1534.407, 1533.622,
	1532.838, 1532.055, 1531.272, 1530.490, 1529.709,
};

double RT_WAVELENGTH_10G_TBL[MAX_CHANNEL_NO] = 
{
	1560.444, 1559.632, 1558.821, 1558.011, 1557.202,
	1556.393, 1555.586, 1554.779, 0,/*0x9*/ 1553.168,
	1552.364, 1551.560, 1550.758, 1549.956, 1549.155,
	1548.355, 1547.556, 0,/*0x12*/ 0,/*0x13*/ 0,/*0x14*/
	0,/*0x15*/ 0,/*0x16*/ 0,/*0x17*/ 1541.984, 1541.191,
	1540.399, 1539.608, 1538.818, 1538.028, 1537.240,
	1536.452, 0,/*0x20*/ 1534.878, 1534.093, 1533.308,
	1532.525, 1531.742, 1530.959, 1530.178, 1529.397,
};
#endif

void HX_I2C_DBG_U(const char *format, ...)
{
#if 1
	return;
#endif
	int rc, fd;
	va_list args;

	char line[1024];
	va_start(args, format);
	rc = vsprintf(line, format, args);
	va_end(args);
	fd = open("/dev/console", O_RDWR, 0);
	write(fd, line, rc);
	close(fd);

	return;
}

# define ALWAYS_INLINE __attribute__ ((always_inline)) inline

/*
 * This is needed for ioctl_or_perror_and_die() since it only accepts pointers.
 */
static ALWAYS_INLINE void *itoptr(int i)
{
	return (void*)(long)i;
}

/*************************************************

Function itoac : Integer TO Ascii with Comma separated
Convert unsigned integer to string with ',' divided in 3 digits

If buf is NULL, then print to stdout
otherwise, store to buf
Size of buf should be enough to store full digits

Returns the string if buf is not NULL,
        if buf is NULL, number of printed chacters

Added by Sung-Uk. 2006.06.02

*************************************************************/
char * itoac( unsigned int d , char *buf )
{
    char s[ 20 ] = {0, };
    int  pos, rpos, opos;
    int  len, ret;

    sprintf( s, "%u", d );

    len = strlen(s);

    for( ret = 0 , pos = 0, opos =0, rpos = len; pos < len ; pos++ , rpos-- )
    {
        if( buf == NULL )
            printf("%c", s[pos]);
        else
        {
            buf[ opos++ ] = s[ pos ];
        }
        ret++;

        if( rpos%3 == 1 )
        {
            if( rpos == 1 )
                if( buf == NULL )
                    printf(".");
                else
                {
                    buf[ opos++ ] = '\0';
                }
            else
                if( buf == NULL )
                    printf(",");
                else
                {
                    buf[ opos++ ] = ',';
                }
            ret++;
        }
    }

    if( buf == NULL )
        ;
    else
        buf[ opos ] = '\0';
#if 1
    return 0;
#else
    return buf;
#endif
}

#if 1/* [#72] Adding omitted rtWDM related registers, dustin, 2024-06-27 */
/* trim : remove heading/tailing spaces */
void trim(char* str)
{
	int start = 0, end = strlen(str) - 1;

	// locate firt space.
	while (isspace(str[start])) {
		start++;
	}

	// locate last spece.
	while (end > start && isspace(str[end])) {
		end--;
	}

	// trim space.
	for (int i = start; i <= end; i++) {
		str[i - start] = str[i];
	}

	// null termination.
	str[end - start + 1] = '\0';
}
#endif

int ioctl_or_perror_and_die(int fd, unsigned request, void *argp, const char *fmt,...)
{
	int ret,ll,f_cnt;

	f_cnt = 0;

	for(ll = 0; ll < HZ_I2C_SLAVE_RETRY; ll++ ) {
		ret = ioctl(fd, request, argp);

		if (ret < 0) {
			printf("[DW] ioctl [REQ:%04X] [errno:%d]\n", request,errno);
			perror("[DW] ioctl : ");
			f_cnt++;
			usleep(HZ_I2C_SLAVE_SLEEP_UM);
		}
		else {
			break;
		}
	}
	//	if(f_cnt == 0) usleep(HZ_I2C_SLAVE_SLEEP_UM);
	return ret;
}

static int32_t i2c_smbus_access(int fd, char read_write, unsigned char cmd,
		int size, union i2c_smbus_data *data)
{
	struct i2c_smbus_ioctl_data args;

	args.read_write = read_write;
	args.command = cmd;
	args.size = size;
	args.data = data;

	return ioctl(fd, I2C_SMBUS, &args);
}

static int32_t i2c_smbus_read_byte_data(int fd, unsigned char cmd)
{
	union i2c_smbus_data data;
	int err;

	err = i2c_smbus_access(fd, I2C_SMBUS_READ, cmd,
			I2C_SMBUS_BYTE_DATA, &data);
	if (err < 0)
	{
		return err;
	}

	return data.byte;
}

#if 1/*[#25] I2C related register update, dustin, 2024-05-28 */
static int32_t i2c_smbus_write_byte_data(int fd, unsigned char addr, unsigned char val)
{
	union i2c_smbus_data data;
	int err;

	memset(&data, 0, sizeof data);
	data.byte = val;

	err = i2c_smbus_access(fd, I2C_SMBUS_WRITE, addr,
			I2C_SMBUS_BYTE_DATA, &data);
	if (err < 0)
	{
		return err;
	}

	return data.byte;
}
#endif

void i2c_set_slave_addr(int fd, unsigned char addr, int force)
{
	ioctl_or_perror_and_die(fd, force ? I2C_SLAVE_FORCE : I2C_SLAVE,
			itoptr(addr),
			"can't set address to 0x%02x", addr);
}

/*
 * Opens the device file associated with given i2c bus.
 *
 * Upstream i2c-tools also support opening devices by i2c bus name
 * but we drop it here for size reduction.
 */
int i2c_dev_open(int i2cbus)
{
	char filename[30];
	int fd;

	sprintf(filename, "/dev/i2c-%d", i2cbus);
	fd = open(filename, O_RDWR);
	if (fd < 0) 
	{
		zlog_notice("i2c_dev_open : can't open %s \n", filename);
		perror("[DW] i2c_dev_open : ");
	}

	return fd;
}

int i2cget_main(int bus, unsigned char addr, unsigned char data_addr)
{
	/*	
			bus 0 : 0x68 RTC
			: 0x48 Temperature sensor
			: 0x55 System CPLD
			: 0x52 Serial EEPROM (Not use)
			bus 1 : 0x70 I2C MUX(Pon port 1 ~ 8)
			: 0x71 I2C MUX(SFP port 1 ~ 4)

addr : chip address

data_addr : CPLD data address
*/
	int fd, status;

	fd = i2c_dev_open(bus);
	if(fd < 0) {
		HX_I2C_DBG_U("[%s:%s:%d] fd:%d\n",__FILE__,__FUNCTION__,__LINE__,fd);
		zlog_notice("%s : device open failed. bus[%x] addr[%x] data_addr[%x] reason[%s]", 
			__func__, bus, addr, data_addr, strerror(errno));
		return -1;
	}
	i2c_set_slave_addr(fd, addr, 1);

	status = i2c_smbus_read_byte_data(fd, data_addr);

	close(fd);

	if (status < 0) {
		printf("read failed\n");
		HX_I2C_DBG_U("[%s:%s:%d] [B:%d] read failed status:%d DA:%02X, PID:PPID[%d:%d]\n",
				__FILE__,__FUNCTION__,__LINE__,bus,status,data_addr,getpid(),getppid());
		zlog_notice("%s : reading failed. bus[%x] addr[%x] data_addr[%x] reason[%s]", 
			__func__, bus, addr, data_addr, strerror(errno));
	}

	//printf("0x%02x\n",  status);

	return status;
}

#if 1/*[#51] Adding register callback templates for config/command registers, dustin, 2024-06-12 */
#if 1/*[#39] Adding Smart T-SFP related functions, dustin, 2024-06-12 */
int check_sfp_is_present(int portno)
{
#if 1/* [#72] Adding omitted rtWDM related registers, dustin, 2024-06-27 */
	int fd, mux_addr, ret;
	unsigned int chann_mask;

	fd = i2c_dev_open(1/*bus*/);
	if(fd < 0) {
		zlog_notice("%s : device open failed. port[%d(0/%d)] reason[%s]",
			__func__, portno, get_eag6L_dport(portno), strerror(errno));
		return ERR_NOT_FOUND;
	}

#if 1 /* [#84] Fixing for PM counters and I2C on Target system, dustin, 2024-07-30 */
	if(portno == (PORT_ID_EAG6L_MAX - 1)/*100G*/) {
		mux_addr = I2C_MUX;
		chann_mask = I2C_MUX_100G_MASK;
	} else {
		mux_addr = I2C_MUX;
		chann_mask = 1 << (portno - PORT_ID_EAG6L_PORT1);
	}
#else
	if(portno >= (PORT_ID_EAG6L_MAX - 1)/*100G*/) {
		mux_addr = I2C_MUX2;
		chann_mask = 1 << (portno - (PORT_ID_EAG6L_MAX - 1));
	} else {
		mux_addr = I2C_MUX;
		chann_mask = 1 << (portno - PORT_ID_EAG6L_PORT1);
	}

	i2c_set_slave_addr(fd, (mux_addr == I2C_MUX) ? I2C_MUX2 : I2C_MUX, 1);

	// first disable the other mux.
	ret = i2c_smbus_write_byte_data(fd, 
		0x0/*mux-data*/, 0x0);
	if(ret < 0) {
		zlog_notice("%s : port[%d(0/%d)] ret[%d].",
			__func__, portno, get_eag6L_dport(portno), ret);
		goto __exit__;
	}
#endif

	i2c_set_slave_addr(fd, mux_addr, 1);

	// now set target mux.
#if 1 /* [#91] Fixing for register updating feature, dustin, 2024-08-05 */
	ret = i2c_smbus_write_byte_data(fd, 0/*mux-data*/, chann_mask);
#else
	ret = i2c_smbus_write_byte_data(fd, mux_addr, chann_mask);
#endif
	if(ret < 0) {
		zlog_notice("%s : port[%d(0/%d)] ret[%d].",
			__func__, portno, get_eag6L_dport(portno), ret);
		goto __exit__;
	}

	i2c_set_slave_addr(fd, SFP_IIC_ADDR/*0x50*/, 1);

	if((ret = i2c_smbus_read_byte_data(fd, 0/*just-to-check*/)) < 0) {
		zlog_notice("%s: Reading port[%d(0/%d)] rtwdm wavelength msb failed.", 
			__func__, portno, get_eag6L_dport(portno));
		goto __exit__;
	}

__exit__:
	close(fd);
	return (ret < 0) ? ERR_NOT_FOUND : SUCCESS;
#else
	/* set mux to change sfp channel */
	i2c_set_sfp_channel_no(1/*bus*/, portno);

	/* try to read 0x50 */
	if(i2cget_main(1/*bus*/, SFP_IIC_ADDR/*0x50*/, 0x0) < 0) {
		return ERR_NOT_FOUND;
	} else
		return SUCCESS;
#endif
}

int i2cset_main(int bus, unsigned char addr, unsigned char data_addr, unsigned char data)
{
	int fd, status;

	fd = i2c_dev_open(bus);
	if(fd < 0) {
		HX_I2C_DBG_U("[%s:%s:%d] fd:%d\n",__FILE__,__FUNCTION__,__LINE__,fd);
		return -1;
	}
	i2c_set_slave_addr(fd, addr, 1);

	status = i2c_smbus_write_byte_data(fd, data_addr, data);

	close(fd);

	if (status < 0) {
		zlog_notice("%s: writing bus[%x] addr[%x] data_addr[%x] data[%d] failed. ret[%d].", 
			__func__, bus, addr, data_addr, data, status);
	}
	return status;
}

uint16_t set_smart_tsfp_self_loopback(uint16_t portno, uint16_t enable)
{
#if 1/* [#72] Adding omitted rtWDM related registers, dustin, 2024-06-27 */
	int fd, mux_addr, ret;
	unsigned char val;
	unsigned int chann_mask;

    /* do nothing for non-tunable sfp or 100G port */
    if((! PORT_STATUS[portno].tunable_sfp) || (portno >= (PORT_ID_EAG6L_MAX - 1)))
        return SUCCESS;

	if((fd = i2c_dev_open(1/*bus*/)) < 0) {
		zlog_notice("%s : device open failed. port[%d(0/%d)] reason[%s]",
			__func__, portno, get_eag6L_dport(portno), strerror(errno));
		return ERR_NOT_FOUND;
	}

#if 1 /* [#84] Fixing for PM counters and I2C on Target system, dustin, 2024-07-30 */
	if(portno == (PORT_ID_EAG6L_MAX - 1)/*100G*/) {
		mux_addr = I2C_MUX;
		chann_mask = I2C_MUX_100G_MASK;
	} else {
		mux_addr = I2C_MUX;
		chann_mask = 1 << (portno - PORT_ID_EAG6L_PORT1);
	}
#else
	if(portno >= (PORT_ID_EAG6L_MAX - 1)/*100G*/) {
		mux_addr = I2C_MUX2;
		chann_mask = 1 << (portno - (PORT_ID_EAG6L_MAX - 1));
	} else {
		mux_addr = I2C_MUX;
		chann_mask = 1 << (portno - PORT_ID_EAG6L_PORT1);
	}

	i2c_set_slave_addr(fd, (mux_addr == I2C_MUX) ? I2C_MUX2 : I2C_MUX, 1);

	/* first disable the other mux. */
	if((ret = i2c_smbus_write_byte_data(fd, 
		0x0/*mux-data*/, 0x0)) < 0) {
		zlog_notice("%s : Disabling mux for port[%d(0/%d)] ret[%d].",
			__func__, portno, get_eag6L_dport(portno), ret);
		goto __exit__;
	}
#endif

	i2c_set_slave_addr(fd, mux_addr, 1);

	/* now set target mux. */
#if 1 /* [#91] Fixing for register updating feature, dustin, 2024-08-05 */
	ret = i2c_smbus_write_byte_data(fd, 0/*mux-data*/, chann_mask);
#else
	ret = i2c_smbus_write_byte_data(fd, mux_addr, chann_mask);
#endif
	if(ret < 0) {
		zlog_notice("%s : Enabling mux for port[%d(0/%d)] ret[%d].",
			__func__, portno, get_eag6L_dport(portno), ret);
		goto __exit__;
	}

	i2c_set_slave_addr(fd, DIAG_SFP_IIC_ADDR/*0x51*/, 1);

	/* select page */
	if((ret = i2c_smbus_write_byte_data(fd, 127/*0x7F*/, 0x2/*page-2*/)) < 0) {
		zlog_notice("%s: Writing port[%d(0/%d)] page select failed.", 
			__func__, portno, get_eag6L_dport(portno));
		goto __exit__;
    }

#if 1/*[#61] Adding omitted functions, dustin, 2024-06-24 */
	/* wait for updating selected page */
	usleep(HZ_I2C_SLAVE_SLEEP_UM);
#endif

	if((val = i2c_smbus_read_byte_data(fd, 129/*0x81*/)) < 0) {
		zlog_notice("%s: Reading port[%d(0/%d)] smart t-sfp self loopback failed.", 
			__func__, portno, get_eag6L_dport(portno));
		goto __exit__;
    }

	/* update bit 1 for smart t-sfp loopback */
	if(enable)
		val |= 0x2;
	else
		val &= ~0x2;

	if((ret = i2c_smbus_write_byte_data(fd, 129/*0x81*/, val)) < 0) {
		zlog_notice("%s: Writing port[%d(0/%d)] smart t-sfp self loopback failed.", 
			__func__, portno, get_eag6L_dport(portno));

		/* recover page */
		if((ret = i2c_smbus_write_byte_data(fd, 127/*0x7F*/, 0x0/*page-0*/)) < 0)
			zlog_notice("%s: Recovering port[%d(0/%d)] page select failed.", 
			__func__, portno, get_eag6L_dport(portno));

		goto __exit__;
	}

	PORT_STATUS[portno].cfg_smart_tsfp_selfloopback = enable;

	/* recover page */
	if((ret = i2c_smbus_write_byte_data(fd, 127/*0x7F*/, 0x0/*page-0*/)) < 0) {
		zlog_notice("%s: Recovering port[%d(0/%d)] page select failed.", 
			__func__, portno, get_eag6L_dport(portno));
		goto __exit__;
	}

__exit__:
	close(fd);
	return SUCCESS;
#else
	unsigned char val;

	/* do nothing for 100G port. */
	if(portno >= (PORT_ID_EAG6L_MAX - 1))
		return SUCCESS;

	/* set mux to change sfp channel */
	i2c_set_sfp_channel_no(1/*bus*/, portno);

	/* select page */
	if(i2cset_main(1/*bus*/, DIAG_SFP_IIC_ADDR/*0x51*/, 127/*0x7F*/, 0x2/*page-2*/) < 0) {
		zlog_notice("%s: Writing port[%d] page select failed.", __func__, portno);
		return -1;
	}

#if 1/*[#61] Adding omitted functions, dustin, 2024-06-24 */
	/* wait for updating selected page */
	usleep(HZ_I2C_SLAVE_SLEEP_UM);
#endif

	if((val = i2cget_main(1/*bus*/, DIAG_SFP_IIC_ADDR/*0x51*/, 129/*0x81*/)) < 0) {
		zlog_notice("%s: Reading port[%d] smart t-sfp self loopback failed.", __func__, portno);
		return -1;
	}

	/* update bit 1 for smart t-sfp loopback */
	if(enable)
		val |= 0x2;
	else
		val &= ~0x2;

	if(i2cset_main(1/*bus*/, DIAG_SFP_IIC_ADDR/*0x51*/, 129/*0x81*/, val) < 0) {
		zlog_notice("%s: Writing port[%d] smart t-sfp self loopback failed.", __func__, portno);

		/* recover page */
		if(i2cset_main(1/*bus*/, DIAG_SFP_IIC_ADDR/*0x51*/, 127/*0x7F*/, 0x0/*page-0*/) < 0) {
			zlog_notice("%s: Recovering port[%d] page select failed.", __func__, portno);
			return -1;
		}

		return -1;
	}

	/* recover page */
	if(i2cset_main(1/*bus*/, DIAG_SFP_IIC_ADDR/*0x51*/, 127/*0x7F*/, 0x0/*page-0*/) < 0) {
		zlog_notice("%s: Recovering port[%d] page select failed.", __func__, portno);
		return -1;
	}

	PORT_STATUS[portno].cfg_smart_tsfp_selfloopback = enable;
	return SUCCESS;
#endif
}

int get_smart_tsfp_self_loopback(int portno, int * enable)
{
#if 1/* [#72] Adding omitted rtWDM related registers, dustin, 2024-06-27 */
	int fd, mux_addr, ret;
	unsigned char val;
	unsigned int chann_mask;

#if 1 /* [#91] Fixing for register updating feature, dustin, 2024-08-05 */
	if(! PORT_STATUS[portno].equip)
		return;
#endif

    /* do nothing for non-tunable sfp or 100G port */
    if((! PORT_STATUS[portno].tunable_sfp) || (portno >= (PORT_ID_EAG6L_MAX - 1)))
        return SUCCESS;

	if((fd = i2c_dev_open(1/*bus*/)) < 0) {
		zlog_notice("%s : device open failed. port[%d(0/%d)] reason[%s]",
			__func__, portno, get_eag6L_dport(portno), strerror(errno));
		return ERR_NOT_FOUND;
	}

#if 1 /* [#84] Fixing for PM counters and I2C on Target system, dustin, 2024-07-30 */
	if(portno == (PORT_ID_EAG6L_MAX - 1)/*100G*/) {
		mux_addr = I2C_MUX;
		chann_mask = I2C_MUX_100G_MASK;
	} else {
		mux_addr = I2C_MUX;
		chann_mask = 1 << (portno - PORT_ID_EAG6L_PORT1);
	}
#else
	if(portno >= (PORT_ID_EAG6L_MAX - 1)/*100G*/) {
		mux_addr = I2C_MUX2;
		chann_mask = 1 << (portno - (PORT_ID_EAG6L_MAX - 1));
	} else {
		mux_addr = I2C_MUX;
		chann_mask = 1 << (portno - PORT_ID_EAG6L_PORT1);
	}

	i2c_set_slave_addr(fd, (mux_addr == I2C_MUX) ? I2C_MUX2 : I2C_MUX, 1);

	/* first disable the other mux. */
	if((ret = i2c_smbus_write_byte_data(fd, 
		0x0/*mux-data*/, 0x0)) < 0) {
		zlog_notice("%s : Disabling mux for port[%d(0/%d)] ret[%d].",
			__func__, portno, get_eag6L_dport(portno), ret);
		goto __exit__;
	}
#endif

	i2c_set_slave_addr(fd, mux_addr, 1);

	/* now set target mux. */
#if 1 /* [#91] Fixing for register updating feature, dustin, 2024-08-05 */
	ret = i2c_smbus_write_byte_data(fd, 0/*mux-data*/, chann_mask);
#else
	ret = i2c_smbus_write_byte_data(fd, mux_addr, chann_mask);
#endif
	if(ret < 0) {
		zlog_notice("%s : Enabling mux for port[%d(0/%d)] ret[%d].",
			__func__, portno, get_eag6L_dport(portno), ret);
		goto __exit__;
	}

	i2c_set_slave_addr(fd, DIAG_SFP_IIC_ADDR/*0x51*/, 1);

	/* select page */
	if((ret = i2c_smbus_write_byte_data(fd, 127/*0x7F*/, 0x2/*page-2*/)) < 0) {
		zlog_notice("%s: Writing port[%d(0/%d)] page select failed. ret[%d].", 
			__func__, portno, get_eag6L_dport(portno), ret);
		goto __exit__;
	}

	/* wait for updating selected page */
	usleep(HZ_I2C_SLAVE_SLEEP_UM);

	if((val = i2c_smbus_read_byte_data(fd, 129/*0x81*/)) < 0) {
		zlog_notice("%s: Reading port[%d(0/%d)] smart t-sfp self loopback failed. ret[%d].", 
			__func__, portno, get_eag6L_dport(portno), val);

		/* recover page */
		if((ret = i2c_smbus_write_byte_data(fd, 127/*0x7F*/, 0x0/*page-0*/)) < 0)
			zlog_notice("%s: Recovering port[%d(0/%d)] page select failed. ret[%d].", 
				__func__, portno, get_eag6L_dport(portno), ret);
		goto __exit__;
	}

	/* get bit 1 for smart t-sfp loopback */
	if(enable)
		*enable = (val & 0x2) ? 1 : 0;
	PORT_STATUS[portno].tsfp_self_lp = *enable;

	/* recover page */
	if((ret = i2c_smbus_write_byte_data(fd, 127/*0x7F*/, 0x0/*page-0*/)) < 0)
		zlog_notice("%s: Recovering port[%d(0/%d)] page select failed. ret[%d].", 
			__func__, portno, get_eag6L_dport(portno), ret);

__exit__:
	close(fd);
	return SUCCESS;
#else
	unsigned char val;

	/* do nothing for 100G port. */
	if(portno >= (PORT_ID_EAG6L_MAX - 1))
		return SUCCESS;

	/* set mux to change sfp channel */
	i2c_set_sfp_channel_no(1/*bus*/, portno);

	/* select page */
	if(i2cset_main(1/*bus*/, DIAG_SFP_IIC_ADDR/*0x51*/, 127/*0x7F*/, 0x2/*page-2*/) < 0) {
		zlog_notice("%s: Writing port[%d] page select failed.", __func__, portno);
		return -1;
	}

#if 1/*[#61] Adding omitted functions, dustin, 2024-06-24 */
	/* wait for updating selected page */
	usleep(HZ_I2C_SLAVE_SLEEP_UM);
#endif

	if((val = i2cget_main(1/*bus*/, DIAG_SFP_IIC_ADDR/*0x51*/, 129/*0x81*/)) < 0) {
		zlog_notice("%s: Reading port[%d] smart t-sfp self loopback failed.", __func__, portno);

		/* recover page */
		if(i2cset_main(1/*bus*/, DIAG_SFP_IIC_ADDR/*0x51*/, 127/*0x7F*/, 0x0/*page-0*/) < 0) {
			zlog_notice("%s: Recovering port[%d] page select failed.", __func__, portno);
			return -1;
		}
		return -1;
	}

	/* get bit 1 for smart t-sfp loopback */
	if(enable)
		*enable = (val & 0x2) ? 1 : 0;
	PORT_STATUS[portno].tsfp_self_lp = *enable;

	/* recover page */
	if(i2cset_main(1/*bus*/, DIAG_SFP_IIC_ADDR/*0x51*/, 127/*0x7F*/, 0x0/*page-0*/) < 0) {
		zlog_notice("%s: Recovering port[%d] page select failed.", __func__, portno);
		return -1;
	}

	return SUCCESS;
#endif
}

uint16_t set_rtwdm_loopback(uint16_t portno, uint16_t enable)
{
#if 1/* [#72] Adding omitted rtWDM related registers, dustin, 2024-06-27 */
	int fd, mux_addr, ret = SUCCESS;
	unsigned char val;
	unsigned int chann_mask;

    /* do nothing for non-tunable sfp or 100G port */
    if((! PORT_STATUS[portno].tunable_sfp) || (portno >= (PORT_ID_EAG6L_MAX - 1)))
        return SUCCESS;

	if((fd = i2c_dev_open(1/*bus*/)) < 0) {
		zlog_notice("%s : device open failed. port[%d(0/%d)] reason[%s]",
			__func__, portno, get_eag6L_dport(portno), strerror(errno));
		return ERR_NOT_FOUND;
	}

#if 1 /* [#84] Fixing for PM counters and I2C on Target system, dustin, 2024-07-30 */
	if(portno == (PORT_ID_EAG6L_MAX - 1)/*100G*/) {
		mux_addr = I2C_MUX;
		chann_mask = I2C_MUX_100G_MASK;
	} else {
		mux_addr = I2C_MUX;
		chann_mask = 1 << (portno - PORT_ID_EAG6L_PORT1);
	}
#else
	if(portno >= (PORT_ID_EAG6L_MAX - 1)/*100G*/) {
		mux_addr = I2C_MUX2;
		chann_mask = 1 << (portno - (PORT_ID_EAG6L_MAX - 1));
	} else {
		mux_addr = I2C_MUX;
		chann_mask = 1 << (portno - PORT_ID_EAG6L_PORT1);
	}

	i2c_set_slave_addr(fd, (mux_addr == I2C_MUX) ? I2C_MUX2 : I2C_MUX, 1);

	/* first disable the other mux. */
	if((ret = i2c_smbus_write_byte_data(fd, 
		0x0/*mux-data*/, 0x0)) < 0) {
		zlog_notice("%s : Disabling mux for port[%d(0/%d)] ret[%d].",
			__func__, portno, get_eag6L_dport(portno), ret);
		goto __exit__;
	}
#endif

	i2c_set_slave_addr(fd, mux_addr, 1);

	/* now set target mux. */
#if 1 /* [#91] Fixing for register updating feature, dustin, 2024-08-05 */
	ret = i2c_smbus_write_byte_data(fd, 0/*mux-data*/, chann_mask);
#else
	ret = i2c_smbus_write_byte_data(fd, mux_addr, chann_mask);
#endif
	if(ret < 0) {
		zlog_notice("%s : Enabling mux for port[%d(0/%d)] ret[%d].",
			__func__, portno, get_eag6L_dport(portno), ret);
		goto __exit__;
	}

	i2c_set_slave_addr(fd, DIAG_SFP_IIC_ADDR/*0x51*/, 1);

	/* select page */
	if((ret = i2c_smbus_write_byte_data(fd, 127/*0x7F*/, 0x2/*page-2*/)) < 0) {
		zlog_notice("%s: Writing port[%d(0/%d)] page select failed.", 
			__func__, portno, get_eag6L_dport(portno));
		goto __exit__;
	}

	/* wait for updating selected page */
	usleep(HZ_I2C_SLAVE_SLEEP_UM);

	if((val = i2c_smbus_read_byte_data(fd, 129/*0x81*/)) < 0) {
		zlog_notice("%s: Reading port[%d(0/%d)] smart t-sfp self loopback failed.", 
			__func__, portno, get_eag6L_dport(portno));
		goto __exit__;
	}

	/* update bit 0 for rtwdm loopback */
	if(enable)
		val |= 0x1;
	else
		val &= ~0x1;

	if((ret = i2c_smbus_write_byte_data(fd, 129/*0x81*/, val)) < 0) {
		zlog_notice("%s: Writing port[%d(0/%d)] rtWDM loopback failed. ret[%d].", 
			__func__, portno, get_eag6L_dport(portno), ret);

		/* recover page */
		if((ret = i2c_smbus_write_byte_data(fd, 127/*0x7F*/, 0x0/*page-0*/)) < 0)
			zlog_notice("%s: Recovering port[%d(0/%d)] page select failed. ret[%d].", 
				__func__, portno, get_eag6L_dport(portno), ret);

		goto __exit__;
	}

	PORT_STATUS[portno].cfg_rtwdm_loopback = enable;

	/* recover page */
	if((ret = i2c_smbus_write_byte_data(fd, 127/*0x7F*/, 0x0/*page-0*/)) < 0)
		zlog_notice("%s: Recovering port[%d(0/%d)] page select failed. ret[%d].", 
			__func__, portno, get_eag6L_dport(portno), ret);

__exit__:
	close(fd);
	return ret;
#else
	unsigned char val;

	/* do nothing for 100G port. */
	if(portno >= (PORT_ID_EAG6L_MAX - 1))
		return SUCCESS;

	/* set mux to change sfp channel */
	i2c_set_sfp_channel_no(1/*bus*/, portno);

	/* select page */
	if(i2cset_main(1/*bus*/, DIAG_SFP_IIC_ADDR/*0x51*/, 127/*0x7F*/, 0x2/*page-2*/) < 0) {
		zlog_notice("%s: Writing port[%d] page select failed.", __func__, portno);
		return -1;
	}

#if 1/*[#61] Adding omitted functions, dustin, 2024-06-24 */
	/* wait for updating selected page */
	usleep(HZ_I2C_SLAVE_SLEEP_UM);
#endif

	if((val = i2cget_main(1/*bus*/, DIAG_SFP_IIC_ADDR/*0x51*/, 129/*0x81*/)) < 0) {
		zlog_notice("%s: Reading port[%d] rtWDM loopback failed.", __func__, portno);
		return -1;
	}

	/* update bit 0 for rtwdm loopback */
	if(enable)
		val |= 0x1;
	else
		val &= ~0x1;

	if(i2cset_main(1/*bus*/, DIAG_SFP_IIC_ADDR/*0x51*/, 129/*0x81*/, val) < 0) {
		zlog_notice("%s: Writing port[%d] rtWDM loopback failed.", __func__, portno);

		/* recover page */
		if(i2cset_main(1/*bus*/, DIAG_SFP_IIC_ADDR/*0x51*/, 127/*0x7F*/, 0x0/*page-0*/) < 0) {
			zlog_notice("%s: Recovering port[%d] page select failed.", __func__, portno);
			return -1;
		}

		return -1;
	}

	/* recover page */
	if(i2cset_main(1/*bus*/, DIAG_SFP_IIC_ADDR/*0x51*/, 127/*0x7F*/, 0x0/*page-0*/) < 0) {
		zlog_notice("%s: Recovering port[%d] page select failed.", __func__, portno);
		return -1;
	}

	PORT_STATUS[portno].cfg_rtwdm_loopback = enable;
	return SUCCESS;
#endif
}

int get_rtwdm_loopback(int portno, int * enable)
{
#if 1/* [#72] Adding omitted rtWDM related registers, dustin, 2024-06-27 */
	int fd, mux_addr, ret;
	unsigned char val;
	unsigned int chann_mask;

#if 1 /* [#91] Fixing for register updating feature, dustin, 2024-08-05 */
	if(! PORT_STATUS[portno].equip)
		return;
#endif

	/* do nothing for non-tunable sfp or 100G port */
	if((! PORT_STATUS[portno].tunable_sfp) || (portno >= (PORT_ID_EAG6L_MAX - 1)))
		return SUCCESS;

	if((fd = i2c_dev_open(1/*bus*/)) < 0) {
		zlog_notice("%s : device open failed. port[%d(0/%d)] reason[%s]",
			__func__, portno, get_eag6L_dport(portno), strerror(errno));
		return ERR_NOT_FOUND;
	}

#if 1 /* [#84] Fixing for PM counters and I2C on Target system, dustin, 2024-07-30 */
	if(portno == (PORT_ID_EAG6L_MAX - 1)/*100G*/) {
		mux_addr = I2C_MUX;
		chann_mask = I2C_MUX_100G_MASK;
	} else {
		mux_addr = I2C_MUX;
		chann_mask = 1 << (portno - PORT_ID_EAG6L_PORT1);
	}
#else
	if(portno >= (PORT_ID_EAG6L_MAX - 1)/*100G*/) {
		mux_addr = I2C_MUX2;
		chann_mask = 1 << (portno - (PORT_ID_EAG6L_MAX - 1));
	} else {
		mux_addr = I2C_MUX;
		chann_mask = 1 << (portno - PORT_ID_EAG6L_PORT1);
	}

	i2c_set_slave_addr(fd, (mux_addr == I2C_MUX) ? I2C_MUX2 : I2C_MUX, 1);

	/* first disable the other mux. */
	if((ret = i2c_smbus_write_byte_data(fd, 
		0x0/*mux-data*/, 0x0)) < 0) {
		zlog_notice("%s : Disabling mux for port[%d(0/%d)] ret[%d].",
			__func__, portno, get_eag6L_dport(portno), ret);
		goto __exit_2__;
	}
#endif

	i2c_set_slave_addr(fd, mux_addr, 1);

	/* now set target mux. */
#if 1 /* [#91] Fixing for register updating feature, dustin, 2024-08-05 */
	ret = i2c_smbus_write_byte_data(fd, 0/*mux-data*/, chann_mask);
#else
	ret = i2c_smbus_write_byte_data(fd, mux_addr, chann_mask);
#endif
	if(ret < 0) {
		zlog_notice("%s : Enabling mux for port[%d(0/%d)] ret[%d].",
			__func__, portno, get_eag6L_dport(portno), ret);
		goto __exit_2__;
	}

	i2c_set_slave_addr(fd, DIAG_SFP_IIC_ADDR/*0x51*/, 1);

	/* select page */
	if((ret = i2c_smbus_write_byte_data(fd, 127/*0x7F*/, 0x2/*page-2*/)) < 0) {
		zlog_notice("%s: Writing port[%d(0/%d)] page select failed.", 
			__func__, portno, get_eag6L_dport(portno));
		goto __exit_1__;
	}

	/* wait for updating selected page */
	usleep(HZ_I2C_SLAVE_SLEEP_UM);

	if((val = i2c_smbus_read_byte_data(fd, 129/*0x81*/)) < 0) {
		zlog_notice("%s: Reading port[%d(0/%d)] rtWDM loopback failed. ret[%d].", 
			__func__, portno, get_eag6L_dport(portno), val);
		goto __exit_1__;
	}

	/* update bit 0 for rtwdm loopback */
	if(enable)
		*enable = (val & 0x1) ? 1 : 0;
	PORT_STATUS[portno].rtwdm_lp = *enable;

__exit_1__:
	/* recover page */
	if((ret = i2c_smbus_write_byte_data(fd, 127/*0x7F*/, 0x0/*page-0*/)) < 0)
		zlog_notice("%s: Recovering port[%d(0/%d)] page select failed. ret[%d].", 
			__func__, portno, get_eag6L_dport(portno), ret);

__exit_2__:
	close(fd);
	return ret;
#else
	unsigned char val;

	/* do nothing for 100G port. */
	if(portno >= (PORT_ID_EAG6L_MAX - 1))
		return SUCCESS;

	/* set mux to change sfp channel */
	i2c_set_sfp_channel_no(1/*bus*/, portno);

	/* select page */
	if(i2cset_main(1/*bus*/, DIAG_SFP_IIC_ADDR/*0x51*/, 127/*0x7F*/, 0x2/*page-2*/) < 0) {
		zlog_notice("%s: Writing port[%d] page select failed.", __func__, portno);
		return -1;
	}

#if 1/*[#61] Adding omitted functions, dustin, 2024-06-24 */
	/* wait for updating selected page */
	usleep(HZ_I2C_SLAVE_SLEEP_UM);
#endif

	if((val = i2cget_main(1/*bus*/, DIAG_SFP_IIC_ADDR/*0x51*/, 129/*0x81*/)) < 0) {
		zlog_notice("%s: Reading port[%d] rtWDM loopback failed.", __func__, portno);

		/* recover page */
		if(i2cset_main(1/*bus*/, DIAG_SFP_IIC_ADDR/*0x51*/, 127/*0x7F*/, 0x0/*page-0*/) < 0) {
			zlog_notice("%s: Recovering port[%d] page select failed.", __func__, portno);
			return -1;
		}
		return -1;
	}

	/* update bit 0 for rtwdm loopback */
	if(enable)
		*enable = (val & 0x1) ? 1 : 0;
	PORT_STATUS[portno].rtwdm_lp = *enable;

	/* recover page */
	if(i2cset_main(1/*bus*/, DIAG_SFP_IIC_ADDR/*0x51*/, 127/*0x7F*/, 0x0/*page-0*/) < 0) {
		zlog_notice("%s: Recovering port[%d] page select failed.", __func__, portno);
		return -1;
	}

	return SUCCESS;
#endif
}

uint16_t set_flex_tune_control(uint16_t portno, uint16_t enable)
{
#if 1/* [#72] Adding omitted rtWDM related registers, dustin, 2024-06-27 */
	int fd, mux_addr, ret;
	unsigned char val;
	unsigned int chann_mask;

	/* do nothing for non-tunable sfp or 100G port */
	if((! PORT_STATUS[portno].tunable_sfp) || (portno >= (PORT_ID_EAG6L_MAX - 1)))
		return SUCCESS;

	if((fd = i2c_dev_open(1/*bus*/)) < 0) {
		zlog_notice("%s : device open failed. port[%d(0/%d)] reason[%s]",
			__func__, portno, get_eag6L_dport(portno), strerror(errno));
		return ERR_NOT_FOUND;
	}

#if 1 /* [#84] Fixing for PM counters and I2C on Target system, dustin, 2024-07-30 */
	if(portno == (PORT_ID_EAG6L_MAX - 1)/*100G*/) {
		mux_addr = I2C_MUX;
		chann_mask = I2C_MUX_100G_MASK;
	} else {
		mux_addr = I2C_MUX;
		chann_mask = 1 << (portno - PORT_ID_EAG6L_PORT1);
	}
#else
	if(portno >= (PORT_ID_EAG6L_MAX - 1)/*100G*/) {
		mux_addr = I2C_MUX2;
		chann_mask = 1 << (portno - (PORT_ID_EAG6L_MAX - 1));
	} else {
		mux_addr = I2C_MUX;
		chann_mask = 1 << (portno - PORT_ID_EAG6L_PORT1);
	}

	i2c_set_slave_addr(fd, (mux_addr == I2C_MUX) ? I2C_MUX2 : I2C_MUX, 1);

	/* first disable the other mux. */
	if((ret = i2c_smbus_write_byte_data(fd, 
		0x0/*mux-data*/, 0x0)) < 0) {
		zlog_notice("%s : Disabling mux for port[%d(0/%d)] ret[%d].",
			__func__, portno, get_eag6L_dport(portno), ret);
		goto __exit__;
	}
#endif

	i2c_set_slave_addr(fd, mux_addr, 1);

	/* now set target mux. */
#if 1 /* [#91] Fixing for register updating feature, dustin, 2024-08-05 */
	ret = i2c_smbus_write_byte_data(fd, 0/*mux-data*/, chann_mask);
#else
	ret = i2c_smbus_write_byte_data(fd, mux_addr, chann_mask);
#endif
	if(ret < 0) {
		zlog_notice("%s : Enabling mux for port[%d(0/%d)] ret[%d].",
			__func__, portno, get_eag6L_dport(portno), ret);
		goto __exit__;
	}

	i2c_set_slave_addr(fd, DIAG_SFP_IIC_ADDR/*0x51*/, 1);

	if((val = i2c_smbus_read_byte_data(fd, 254/*0xFE*/)) < 0) {
		zlog_notice("%s: Reading port[%d(0/%d)] flex tune control failed. ret[%d].", 
			__func__, portno, get_eag6L_dport(portno), val);
		goto __exit__;
	}

    /* update bit 1 for flex tune on/off */
    if(enable)
        val |= 0x2;
    else
        val &= ~0x2;

	if((ret = i2c_smbus_write_byte_data(fd, 254/*0xFE*/, val)) < 0) {
        zlog_notice("%s: Writing port[%d(0/%d)] flex tune control failed. ret[%d].", 
			__func__, portno, get_eag6L_dport(portno), ret);
		goto __exit__;
    }

    PORT_STATUS[portno].cfg_flex_tune = enable;

    /* update flex tune status */
	if((val = i2c_smbus_read_byte_data(fd, 253/*0xFD*/)) < 0) {
        zlog_notice("%s: Reading port[%d(0/%d)] flex tune status failed. ret[%d].", 
			__func__, portno, get_eag6L_dport(portno), val);
		goto __exit__;
    }

    PORT_STATUS[portno].flex_tune_status = (val & 0x1/*bit-0*/) ? 1 : 0;

__exit__:
	close(fd);
    return SUCCESS;
#else
	unsigned char val;

	/* do nothing for 100G port. */
	if(portno >= (PORT_ID_EAG6L_MAX - 1))
		return SUCCESS;

	/* set mux to change sfp channel */
	i2c_set_sfp_channel_no(1/*bus*/, portno);

	if((val = i2cget_main(1/*bus*/, DIAG_SFP_IIC_ADDR/*0x51*/, 254/*0xFE*/)) < 0) {
		zlog_notice("%s: Reading port[%d] flex tune control failed.", __func__, portno);
		return -1;
	}

	/* update bit 1 for flex tune on/off */
	if(enable)
		val |= 0x2;
	else
		val &= ~0x2;

	if(i2cset_main(1/*bus*/, DIAG_SFP_IIC_ADDR/*0x51*/, 254/*0xFE*/, val) < 0) {
		zlog_notice("%s: Writing port[%d] flex tune control failed.", __func__, portno);
		return -1;
	}

	PORT_STATUS[portno].cfg_flex_tune = enable;

	/* update flex tune status */
	if((val = i2cget_main(1/*bus*/, DIAG_SFP_IIC_ADDR/*0x51*/, 253/*0xFD*/)) < 0) {
		zlog_notice("%s: Reading port[%d] flex tune status failed.", __func__, portno);
		return -1;
	}

	PORT_STATUS[portno].flex_tune_status = (val & 0x1/*bit-0*/) ? 1 : 0;
	return SUCCESS;
#endif
}

uint16_t set_flex_tune_reset(uint16_t portno, uint16_t enable)
{
#if 1/* [#72] Adding omitted rtWDM related registers, dustin, 2024-06-27 */
	int fd, mux_addr, ret = SUCCESS;
	unsigned char val;
	unsigned int chann_mask;

	/* do nothing for non-tunable sfp or 100G port */
	if((! PORT_STATUS[portno].tunable_sfp) || (portno >= (PORT_ID_EAG6L_MAX - 1)))
		return SUCCESS;

	if((fd = i2c_dev_open(1/*bus*/)) < 0) {
		zlog_notice("%s : device open failed. port[%d(0/%d)] reason[%s]",
			__func__, portno, get_eag6L_dport(portno), strerror(errno));
		return ERR_NOT_FOUND;
	}

#if 1 /* [#84] Fixing for PM counters and I2C on Target system, dustin, 2024-07-30 */
	if(portno == (PORT_ID_EAG6L_MAX - 1)/*100G*/) {
		mux_addr = I2C_MUX;
		chann_mask = I2C_MUX_100G_MASK;
	} else {
		mux_addr = I2C_MUX;
		chann_mask = 1 << (portno - PORT_ID_EAG6L_PORT1);
	}
#else
	if(portno >= (PORT_ID_EAG6L_MAX - 1)/*100G*/) {
		mux_addr = I2C_MUX2;
		chann_mask = 1 << (portno - (PORT_ID_EAG6L_MAX - 1));
	} else {
		mux_addr = I2C_MUX;
		chann_mask = 1 << (portno - PORT_ID_EAG6L_PORT1);
	}

	i2c_set_slave_addr(fd, (mux_addr == I2C_MUX) ? I2C_MUX2 : I2C_MUX, 1);

	/* first disable the other mux. */
	if((ret = i2c_smbus_write_byte_data(fd, 
		0x0/*mux-data*/, 0x0)) < 0) {
		zlog_notice("%s : Disabling mux for port[%d(0/%d)] ret[%d].",
			__func__, portno, get_eag6L_dport(portno), ret);
		goto __exit__;
	}
#endif

	i2c_set_slave_addr(fd, mux_addr, 1);

	/* now set target mux. */
#if 1 /* [#91] Fixing for register updating feature, dustin, 2024-08-05 */
	ret = i2c_smbus_write_byte_data(fd, 0/*mux-data*/, chann_mask);
#else
	ret = i2c_smbus_write_byte_data(fd, mux_addr, chann_mask);
#endif
	if(ret < 0) {
		zlog_notice("%s : Enabling mux for port[%d(0/%d)] ret[%d].",
			__func__, portno, get_eag6L_dport(portno), ret);
		goto __exit__;
	}

	i2c_set_slave_addr(fd, DIAG_SFP_IIC_ADDR/*0x51*/, 1);

#if 1 /* [#91] Fixing for register updating feature, dustin, 2024-08-05 */
	/* read for update. */
	if((val = i2c_smbus_read_byte_data(fd, 253/*0xFD*/)) < 0) {
		zlog_notice("%s: Reading port[%d(0/%d)] flex tune reset failed. ret[%d].",
			__func__, portno, get_eag6L_dport(portno), ret);
		goto __exit__;
	}

	/* update bit 4 for flex tune reset, no disable action. */
	if(enable)
		val |= 0x10;

	/* write updated value. */
	if((ret = i2c_smbus_write_byte_data(fd, 253/*0xFD*/, val)) < 0) {
		zlog_notice("%s: Writing port[%d(0/%d)] flex tune reset failed. ret[%d]", 
			__func__, portno, get_eag6L_dport(portno), ret);
		goto __exit__;
	}

	/* read again for reset action. */
	if((val = i2c_smbus_read_byte_data(fd, 253/*0xFD*/)) < 0) {
		zlog_notice("%s: Reading port[%d(0/%d)] flex tune reset failed. ret[%d].",
			__func__, portno, get_eag6L_dport(portno), ret);
		goto __exit__;
	}
#else
	/* read for update. */
	if((ret = i2c_smbus_read_byte_data(fd, 253/*0xFD*/)) < 0) {
		zlog_notice("%s: Reading port[%d(0/%d)] flex tune reset failed. ret[%d].",
			__func__, portno, get_eag6L_dport(portno), ret);
		goto __exit__;
	}

	/* update bit 4 for flex tune reset, no disable action. */
	if(enable)
		val |= 0x10;

	/* write updated value. */
	if((ret = i2c_smbus_write_byte_data(fd, 253/*0xFD*/, val)) < 0) {
		zlog_notice("%s: Writing port[%d(0/%d)] flex tune reset failed. ret[%d]", 
			__func__, portno, get_eag6L_dport(portno), ret);
		goto __exit__;
	}

	/* read again for reset action. */
	if((ret = i2c_smbus_read_byte_data(fd, 253/*0xFD*/)) < 0) {
		zlog_notice("%s: Reading port[%d(0/%d)] flex tune reset failed. ret[%d].",
			__func__, portno, get_eag6L_dport(portno), ret);
		goto __exit__;
	}
#endif

	usleep(HZ_I2C_SLAVE_SLEEP_UM);/* wait for 10ms */

	/* clear bit if reading is set */
	if(val & 0x10) {
		val &= ~0x10;/*clear*/
		if((ret = i2c_smbus_write_byte_data(fd, 253/*0xFD*/, val)) < 0) {
			zlog_notice("%s: Writing port[%d(0/%d)] flex tune reset failed.", 
				__func__, portno, get_eag6L_dport(portno));
			goto __exit__;
		}
	}

__exit__:
	close(fd);
	return ret;
#else
	unsigned char val;

	/* do nothing for 100G port. */
	if(portno >= (PORT_ID_EAG6L_MAX - 1))
		return SUCCESS;

	/* set mux to change sfp channel */
	i2c_set_sfp_channel_no(1/*bus*/, portno);

	/* read for update. */
	if((val = i2cget_main(1/*bus*/, DIAG_SFP_IIC_ADDR/*0x51*/, 253/*0xFD*/)) < 0) {
		zlog_notice("%s: Reading port[%d] flex tune reset failed.", __func__, portno);
		return -1;
	}

	/* update bit 4 for flex tune reset, no disable action. */
	if(enable)
		val |= 0x10;

	/* write updated value. */
	if(i2cset_main(1/*bus*/, DIAG_SFP_IIC_ADDR/*0x51*/, 253/*0xFD*/, val) < 0) {
		zlog_notice("%s: Writing port[%d] flex tune reset failed.", __func__, portno);
		return -1;
	}

	/* read again for reset action. */
	if((val = i2cget_main(1/*bus*/, DIAG_SFP_IIC_ADDR/*0x51*/, 253/*0xFD*/)) < 0) {
		zlog_notice("%s: Reading port[%d] flex tune reset failed.", __func__, portno);
		return -1;
	}

	usleep(HZ_I2C_SLAVE_SLEEP_UM);/* wait for 5ms */

	/* clear bit if reading is set */
	if(val & 0x10) {
		val &= ~0x10;/*clear*/
		if(i2cset_main(1/*bus*/, DIAG_SFP_IIC_ADDR/*0x51*/, 253/*0xFD*/, val) < 0) {
			zlog_notice("%s: Writing port[%d] flex tune reset failed.", __func__, portno);
			return -1;
		}
	}

	return SUCCESS;
#endif
}

#if 1/*[#61] Adding omitted functions, dustin, 2024-06-24 */
int get_flex_tune_status(int portno)
{
#if 1/* [#72] Adding omitted rtWDM related registers, dustin, 2024-06-27 */
	int fd, mux_addr, ret = SUCCESS;
	unsigned char val;
	unsigned int chann_mask;

#if 1 /* [#91] Fixing for register updating feature, dustin, 2024-08-05 */
	if(! PORT_STATUS[portno].equip)
		return;
#endif

	/* do nothing for non-tunable sfp or 100G port */
	if((! PORT_STATUS[portno].tunable_sfp) || (portno >= (PORT_ID_EAG6L_MAX - 1)))
		return SUCCESS;

	if((fd = i2c_dev_open(1/*bus*/)) < 0) {
		zlog_notice("%s : device open failed. port[%d(0/%d)] reason[%s]",
			__func__, portno, get_eag6L_dport(portno), strerror(errno));
		return ERR_NOT_FOUND;
	}

#if 1 /* [#84] Fixing for PM counters and I2C on Target system, dustin, 2024-07-30 */
	if(portno == (PORT_ID_EAG6L_MAX - 1)/*100G*/) {
		mux_addr = I2C_MUX;
		chann_mask = I2C_MUX_100G_MASK;
	} else {
		mux_addr = I2C_MUX;
		chann_mask = 1 << (portno - PORT_ID_EAG6L_PORT1);
	}
#else
	if(portno >= (PORT_ID_EAG6L_MAX - 1)/*100G*/) {
		mux_addr = I2C_MUX2;
		chann_mask = 1 << (portno - (PORT_ID_EAG6L_MAX - 1));
	} else {
		mux_addr = I2C_MUX;
		chann_mask = 1 << (portno - PORT_ID_EAG6L_PORT1);
	}

	i2c_set_slave_addr(fd, (mux_addr == I2C_MUX) ? I2C_MUX2 : I2C_MUX, 1);

	/* first disable the other mux. */
	if((ret = i2c_smbus_write_byte_data(fd, 
		0x0/*mux-data*/, 0x0)) < 0) {
		zlog_notice("%s : Disabling mux for port[%d(0/%d)] ret[%d].",
			__func__, portno, get_eag6L_dport(portno), ret);
		goto __exit__;
	}
#endif

	i2c_set_slave_addr(fd, mux_addr, 1);

	/* now set target mux. */
#if 1 /* [#91] Fixing for register updating feature, dustin, 2024-08-05 */
	ret = i2c_smbus_write_byte_data(fd, 0/*mux-data*/, chann_mask);
#else
	ret = i2c_smbus_write_byte_data(fd, mux_addr, chann_mask);
#endif
	if(ret < 0) {
		zlog_notice("%s : Enabling mux for port[%d(0/%d)] ret[%d].",
			__func__, portno, get_eag6L_dport(portno), ret);
		goto __exit__;
	}

	i2c_set_slave_addr(fd, DIAG_SFP_IIC_ADDR/*0x51*/, 1);

	/* read for update. */
	if((val = i2c_smbus_read_byte_data(fd, 253/*0xFD*/)) < 0) {
		zlog_notice("%s: Reading port[%d(0/%d)] flex tune control failed. ret[%d].",
			__func__, portno, get_eag6L_dport(portno), val);
		goto __exit__;
	}

	/* update smart t-sfp status */
	PORT_STATUS[portno].flex_tune_status = (val & 0x1) ? 1 : 0;

__exit__:
	close(fd);
	return ret;
#else
    unsigned char val;
	unsigned int data;

	/* do nothing for 100G port. */
	if(portno >= (PORT_ID_EAG6L_MAX - 1))
		return SUCCESS;

	/* clear if not configured. */
	if(! PORT_STATUS[portno].cfg_flex_tune) {
		PORT_STATUS[portno].flex_tune_status = 0/*default*/;
		return SUCCESS;
	}

    /* set mux to change sfp channel */
    i2c_set_sfp_channel_no(1/*bus*/, portno);

    /* read for update. */
    if((val = i2cget_main(1/*bus*/, DIAG_SFP_IIC_ADDR/*0x51*/, 253/*0xFD*/)) < 0) {
        zlog_notice("%s: Reading port[%d] flex tune reset failed.", __func__, portno);
        return -1;
    }

	/* update smart t-sfp status */
	PORT_STATUS[portno].flex_tune_status = (val & 0x1) ? 1 : 0;
	return SUCCESS;
#endif
}

int update_flex_tune_status(int portno)
{
    unsigned char val;

	/* do nothing for 100G port. */
	if(portno >= (PORT_ID_EAG6L_MAX - 1))
		return SUCCESS;

	/* clear if not configured. */
	if((! PORT_STATUS[portno].cfg_flex_tune) || 
	   (! PORT_STATUS[portno].tunable_sfp)) {
		gPortRegUpdate(__PORT_STSFP_STAT_ADDR[portno], 0, 0xFFFF, 0x0/*default*/);
		return SUCCESS;
	}

	/* update smart t-sfp status */
	val  = PORT_STATUS[portno].flex_tune_status;
	if(PORT_STATUS[portno].sfp_type == SFP_ID_UNKNOWN/*rtWDM?*/) {
		gPortRegUpdate(__PORT_STSFP_STAT_ADDR[portno], 8, 0xFF00, val ? 0xA5 : 0x5A);
	}
	else {
		gPortRegUpdate(__PORT_STSFP_STAT_ADDR[portno], 0, 0x00FF, val ? 0xA5 : 0x5A);
	}
	return SUCCESS;
}

int get_tunable_sfp_channel_no(int portno)
{
#if 1/* [#72] Adding omitted rtWDM related registers, dustin, 2024-06-27 */
	double wval, tval;
	int fd, mux_addr, ii, ret;
	unsigned char val;
	unsigned int chann_mask, data;

#if 1 /* [#91] Fixing for register updating feature, dustin, 2024-08-05 */
	if(! PORT_STATUS[portno].equip)
		return;
#endif

	/* do nothing for non-tunable sfp or 100G port */
	if((! PORT_STATUS[portno].tunable_sfp) || (portno >= (PORT_ID_EAG6L_MAX - 1)))
		return SUCCESS;

	if((fd = i2c_dev_open(1/*bus*/)) < 0) {
		zlog_notice("%s : device open failed. port[%d(0/%d)] reason[%s]",
			__func__, portno, get_eag6L_dport(portno), strerror(errno));
		return ERR_NOT_FOUND;
	}

#if 1 /* [#84] Fixing for PM counters and I2C on Target system, dustin, 2024-07-30 */
	if(portno == (PORT_ID_EAG6L_MAX - 1)/*100G*/) {
		mux_addr = I2C_MUX;
		chann_mask = I2C_MUX_100G_MASK;
	} else {
		mux_addr = I2C_MUX;
		chann_mask = 1 << (portno - PORT_ID_EAG6L_PORT1);
	}
#else
	if(portno >= (PORT_ID_EAG6L_MAX - 1)/*100G*/) {
		mux_addr = I2C_MUX2;
		chann_mask = 1 << (portno - (PORT_ID_EAG6L_MAX - 1));
	} else {
		mux_addr = I2C_MUX;
		chann_mask = 1 << (portno - PORT_ID_EAG6L_PORT1);
	}

	i2c_set_slave_addr(fd, (mux_addr == I2C_MUX) ? I2C_MUX2 : I2C_MUX, 1);

	/* first disable the other mux. */
	if((ret = i2c_smbus_write_byte_data(fd, 
		0x0/*mux-data*/, 0x0)) < 0) {
		zlog_notice("%s : Disabling mux for port[%d(0/%d)] ret[%d].",
			__func__, portno, get_eag6L_dport(portno), ret);
		goto __exit_2__;
	}
#endif

	i2c_set_slave_addr(fd, mux_addr, 1);

	/* now set target mux. */
#if 1 /* [#91] Fixing for register updating feature, dustin, 2024-08-05 */
	ret = i2c_smbus_write_byte_data(fd, 0/*mux-data*/, chann_mask);
#else
	ret = i2c_smbus_write_byte_data(fd, mux_addr, chann_mask);
#endif
	if(ret < 0) {
		zlog_notice("%s : Enabling mux for port[%d(0/%d)] ret[%d].",
			__func__, portno, get_eag6L_dport(portno), ret);
		goto __exit_2__;
	}

	i2c_set_slave_addr(fd, DIAG_SFP_IIC_ADDR/*0x51*/, 1);

	/* select page */
	if((ret = i2c_smbus_write_byte_data(fd, 127/*0x7F*/, 0x2/*page-2*/)) < 0) {
		zlog_notice("%s: Writing port[%d(0/%d)] page select failed.", 
			__func__, portno, get_eag6L_dport(portno));
		goto __exit_1__;
	}

	/* wait for updating selected page */
	usleep(HZ_I2C_SLAVE_SLEEP_UM);

	/* read wavelength msb. */
	if((val = i2c_smbus_read_byte_data(fd, 146/*0x92*/)) < 0) {
		zlog_notice("%s: Reading port[%d(0/%d)] wavelength msb failed. ret[%d].",
			__func__, portno, get_eag6L_dport(portno), val);
		goto __exit_1__;
	}

	data = ((unsigned int)val << 8);

	/* read wavelength lsb. */
	if((val = i2c_smbus_read_byte_data(fd, 147/*0x93*/)) < 0) {
		zlog_notice("%s: Reading port[%d(0/%d)] wavelength lsb failed. ret[%d].",
			__func__, portno, get_eag6L_dport(portno), val);
		goto __exit_1__;
	}

	data |= val;

	/* get real wavelength in float */
	wval = (double)(0.05 * data);
	if(wval == 0) {
		zlog_notice("%s: invalid wavelength[%7.2f] data[%d].", 
			__func__, wval, data);
		goto __exit_1__;
	}

	/* scan wavelength for channel no */
	if(PORT_STATUS[portno].sfp_type == SFP_ID_SMART_BIDI_TSFP_COT) {
		for(ii = 0; ii < MAX_CHANNEL_NO; ii++) {
			if(! COT_WAVELENGTH_10G_TBL[ii])
				continue;
			tval = COT_WAVELENGTH_10G_TBL[ii] / 5000;
			if(((COT_WAVELENGTH_10G_TBL[ii] - tval) <= wval) &&
					(wval <= (COT_WAVELENGTH_10G_TBL[ii] + tval)))
				break;
		}
#if 1 /* [#91] Fixing for register updating feature, dustin, 2024-08-05 */
		if(ii >= MAX_CHANNEL_NO) {
			for(ii = 0; ii < MAX_CHANNEL_NO; ii++) {
				if(! WAVELENGTH_25G_TBL[ii])
					continue;
				tval = WAVELENGTH_25G_TBL[ii] / 5000;
				if(((WAVELENGTH_25G_TBL[ii] - tval) <= wval) &&
					(wval <= (WAVELENGTH_25G_TBL[ii] + tval)))
					break;
			}
		}
#endif
	} else if(PORT_STATUS[portno].sfp_type == SFP_ID_SMART_BIDI_TSFP_RT) {
		for(ii = 0; ii < MAX_CHANNEL_NO; ii++) {
			if(! RT_WAVELENGTH_10G_TBL[ii])
				continue;
			tval = RT_WAVELENGTH_10G_TBL[ii] / 5000;
			if(((RT_WAVELENGTH_10G_TBL[ii] - tval) <= wval) &&
					(wval <= (RT_WAVELENGTH_10G_TBL[ii] + tval)))
				break;
		}
#if 1 /* [#91] Fixing for register updating feature, dustin, 2024-08-05 */
		if(ii >= MAX_CHANNEL_NO) {
			for(ii = 0; ii < MAX_CHANNEL_NO; ii++) {
				if(! WAVELENGTH_25G_TBL[ii])
					continue;
				tval = WAVELENGTH_25G_TBL[ii] / 5000;
				if(((WAVELENGTH_25G_TBL[ii] - tval) <= wval) &&
					(wval <= (WAVELENGTH_25G_TBL[ii] + tval)))
					break;
			}
		}
#endif
	} else if((PORT_STATUS[portno].speed == PORT_IF_25G_KR) || 
		      (PORT_STATUS[portno].speed == PORT_IF_10G_KR)) {
		for(ii = 0; ii < MAX_CHANNEL_NO; ii++) {
			if(! WAVELENGTH_25G_TBL[ii])
				continue;
			tval = WAVELENGTH_25G_TBL[ii] / 5000;
			if(((WAVELENGTH_25G_TBL[ii] - tval) <= wval) &&
					(wval <= (WAVELENGTH_25G_TBL[ii] + tval)))
				break;
		}
	} else {
		zlog_notice("%s: invalid sfp type[%d] for port[%d(0/%d)].", 
			__func__, PORT_STATUS[portno].sfp_type, portno, get_eag6L_dport(portno));
		goto __exit_1__;
	}

	if(ii >= MAX_CHANNEL_NO) {
		zlog_notice("%s: not found channel for wavelength[%7.2f] for port[%d(0/%d)].",
				__func__, wval, portno, get_eag6L_dport(portno));
		goto __exit_1__;
	}

	PORT_STATUS[portno].tunable_wavelength = wval;
	PORT_STATUS[portno].tunable_chno = ii + 1;

__exit_1__:
	i2c_set_slave_addr(fd, DIAG_SFP_IIC_ADDR/*0x51*/, 1);

	/* recover page to default */
	if((ret = i2c_smbus_write_byte_data(fd, 127/*0x7F*/, 0x0/*page-0*/)) < 0) {
		zlog_notice("%s: Recovering port[%d(0/%d)] page select failed. ret[%d].",
			__func__, portno, get_eag6L_dport(portno), ret);
#if 1 /* [#91] Fixing for register updating feature, dustin, 2024-08-05 */
		goto __exit_2__;
#else
		goto __exit_1__;
#endif
	}

__exit_2__:
	close(fd);
	return ret;
#else
	unsigned char val;
	unsigned int data, ii;
	double wval, tval;

	/* do nothing for non-tunable sfp */
	if(! PORT_STATUS[portno].tunable_sfp)
		return SUCCESS;

	/* do nothing for 100G port. */
	if(portno >= (PORT_ID_EAG6L_MAX - 1))
		return SUCCESS;

	/* set mux to change sfp channel */
	i2c_set_sfp_channel_no(1/*bus*/, portno);

	/* select page */
	if(i2cset_main(1/*bus*/, DIAG_SFP_IIC_ADDR/*0x51*/, 127/*0x7F*/, 0x2/*page-2*/) < 0) {
		zlog_notice("%s: Writing port[%d] page select failed.", __func__, portno);
		return -1;
	}

	/* wait for updating selected page */
	usleep(HZ_I2C_SLAVE_SLEEP_UM);

	/* read wavelength msb. */
	if((val = i2cget_main(1/*bus*/, DIAG_SFP_IIC_ADDR/*0x51*/, 146/*0x92*/)) < 0) {
		zlog_notice("%s: Reading port[%d] flex tune reset failed.", __func__, portno);
		return -1;
	}

	data = ((unsigned int)val << 8);

	/* read wavelength lsb. */
	if((val = i2cget_main(1/*bus*/, DIAG_SFP_IIC_ADDR/*0x51*/, 147/*0x93*/)) < 0) {
		zlog_notice("%s: Reading port[%d] flex tune reset failed.", __func__, portno);
		return -1;
	}

	/* recover page to default */
	if(i2cset_main(1/*bus*/, DIAG_SFP_IIC_ADDR/*0x51*/, 127/*0x7F*/, 0/*page-0*/) < 0) {
		zlog_notice("%s: Resetting port[%d] page select failed.", __func__, portno);
		return -1;
	}

	data |= val;
	/* get real wavelength in float */
	wval = (double)(0.05 * data);
	if(wval == 0) {
		zlog_notice("%s: invalid wavelength[%7.2f] data[%d].", __func__, wval, data);
		return -1;
	}

	/* scan wavelength for channel no */
	if(PORT_STATUS[portno].sfp_type == SFP_ID_SMART_BIDI_TSFP_COT) {
		for(ii = 0; ii < MAX_CHANNEL_NO; ii++) {
			if(! COT_WAVELENGTH_10G_TBL[ii])
				continue;
			tval = COT_WAVELENGTH_10G_TBL[ii] / 5000;
			if(((COT_WAVELENGTH_10G_TBL[ii] - tval) <= wval) && 
			   (wval <= (COT_WAVELENGTH_10G_TBL[ii] + tval)))
				break;
		}
	} else if(PORT_STATUS[portno].sfp_type == SFP_ID_SMART_BIDI_TSFP_RT) {
		for(ii = 0; ii < MAX_CHANNEL_NO; ii++) {
			if(! RT_WAVELENGTH_10G_TBL[ii])
				continue;
			tval = RT_WAVELENGTH_10G_TBL[ii] / 5000;
			if(((RT_WAVELENGTH_10G_TBL[ii] - tval) <= wval) && 
			   (wval <= (RT_WAVELENGTH_10G_TBL[ii] + tval)))
				break;
		}
	} else if(PORT_STATUS[portno].speed == PORT_IF_25G_KR) {
		for(ii = 0; ii < MAX_CHANNEL_NO; ii++) {
			if(! WAVELENGTH_25G_TBL[ii])
				continue;
			tval = WAVELENGTH_25G_TBL[ii] / 5000;
			if(((WAVELENGTH_25G_TBL[ii] - tval) <= wval) && 
			   (wval <= (WAVELENGTH_25G_TBL[ii] + tval)))
				break;
		}
	} else {
		zlog_notice("%s: invalid sfp type[%d].", __func__, PORT_STATUS[portno].sfp_type);
		return -1;
	}

	if(ii >= MAX_CHANNEL_NO) {
		zlog_notice("%s: not found channel for wavelength[%7.2f] for port[%d].", 
			__func__, wval, portno);
		return -1;
	}

	PORT_STATUS[portno].tunable_wavelength = wval;
	PORT_STATUS[portno].tunable_chno = ii + 1;
	return SUCCESS;
#endif
}

int update_sfp_channel_no(int portno)
{
	/* do nothing for 100G port. */
	if(portno >= (PORT_ID_EAG6L_MAX - 1))
		return SUCCESS;

	/* update tunable sfp channel no. */
	if(PORT_STATUS[portno].tunable_sfp)
		FPGA_PORT_WRITE(__PORT_GET_CH_NUM_ADDR[portno], PORT_STATUS[portno].tunable_chno);
	else
		FPGA_PORT_WRITE(__PORT_GET_CH_NUM_ADDR[portno], 0x0/*default*/);
	return SUCCESS;
}

uint16_t set_tunable_sfp_channel_no(uint16_t portno, uint16_t chno)
{
#if 1/* [#72] Adding omitted rtWDM related registers, dustin, 2024-06-27 */
	double wval;
	int fd, mux_addr, ret = SUCCESS;
	unsigned char val;
	unsigned int chann_mask, data;

	if((0 >= chno) || (chno > MAX_CHANNEL_NO)) {
		zlog_notice("%s: Invalid channel no[%d] for port[%d(0/%d)]", 
			__func__, chno, portno, get_eag6L_dport(portno));
		return ERR_INVALID_PARAM;
	}

	/* do nothing for non-tunable sfp or 100G port */
	if((! PORT_STATUS[portno].tunable_sfp) || (portno >= (PORT_ID_EAG6L_MAX - 1)))
		return SUCCESS;

	if(PORT_STATUS[portno].speed == PORT_IF_25G_KR)
		wval = WAVELENGTH_25G_TBL[chno - 1];
	else if(PORT_STATUS[portno].sfp_type == SFP_ID_SMART_BIDI_TSFP_COT)
		wval = COT_WAVELENGTH_10G_TBL[chno - 1];
	else if(PORT_STATUS[portno].sfp_type == SFP_ID_SMART_BIDI_TSFP_RT)
		wval = RT_WAVELENGTH_10G_TBL[chno - 1];
	else {
		zlog_notice("%s: invalid sfp type[%d].", __func__, PORT_STATUS[portno].sfp_type);
		return ERR_INVALID_PARAM;
	}

	/* get writable data for wavelength */
	data = (unsigned int)(wval / 0.05);

	if((fd = i2c_dev_open(1/*bus*/)) < 0) {
		zlog_notice("%s : device open failed. port[%d(0/%d)] reason[%s]",
			__func__, portno, get_eag6L_dport(portno), strerror(errno));
		return ERR_NOT_FOUND;
	}

#if 1 /* [#84] Fixing for PM counters and I2C on Target system, dustin, 2024-07-30 */
	if(portno == (PORT_ID_EAG6L_MAX - 1)/*100G*/) {
		mux_addr = I2C_MUX;
		chann_mask = I2C_MUX_100G_MASK;
	} else {
		mux_addr = I2C_MUX;
		chann_mask = 1 << (portno - PORT_ID_EAG6L_PORT1);
	}
#else
	if(portno >= (PORT_ID_EAG6L_MAX - 1)/*100G*/) {
		mux_addr = I2C_MUX2;
		chann_mask = 1 << (portno - (PORT_ID_EAG6L_MAX - 1));
	} else {
		mux_addr = I2C_MUX;
		chann_mask = 1 << (portno - PORT_ID_EAG6L_PORT1);
	}

	i2c_set_slave_addr(fd, (mux_addr == I2C_MUX) ? I2C_MUX2 : I2C_MUX, 1);

	/* first disable the other mux. */
	if((ret = i2c_smbus_write_byte_data(fd, 
		0x0/*mux-data*/, 0x0)) < 0) {
		zlog_notice("%s : Disabling mux for port[%d(0/%d)] ret[%d].",
			__func__, portno, get_eag6L_dport(portno), ret);
		goto __exit__;
	}
#endif

	i2c_set_slave_addr(fd, mux_addr, 1);

	/* now set target mux. */
#if 1 /* [#91] Fixing for register updating feature, dustin, 2024-08-05 */
	ret = i2c_smbus_write_byte_data(fd, 0/*mux-data*/, chann_mask);
#else
	ret = i2c_smbus_write_byte_data(fd, mux_addr, chann_mask);
#endif
	if(ret < 0) {
		zlog_notice("%s : Enabling mux for port[%d(0/%d)] ret[%d].",
			__func__, portno, get_eag6L_dport(portno), ret);
		goto __exit__;
	}

	i2c_set_slave_addr(fd, DIAG_SFP_IIC_ADDR/*0x51*/, 1);

	/* write wavelength msb. */
	if((ret = i2c_smbus_write_byte_data(fd, 146/*0x92*/, (data >> 8) & 0xFF)) < 0) {
		zlog_notice("%s : Writing port[%d(0/%d)] wavelength msb failed. ret[%d].",
				__func__, portno, get_eag6L_dport(portno), ret);
		goto __exit__;
	}

	/* write wavelength lsb. */
	if((ret = i2c_smbus_write_byte_data(fd, 147/*0x93*/, data & 0xFF)) < 0) {
		zlog_notice("%s : Writing port[%d(0/%d)] wavelength lsb failed. ret[%d].",
				__func__, portno, get_eag6L_dport(portno), ret);
		goto __exit__;
	}

__exit__:
	close(fd);
	return ret;
#else
	unsigned int data, ii;
	double wval;

	if((0 >= chno) || (chno > MAX_CHANNEL_NO)) {
		zlog_notice("%s: Invalid channel no[%d] for port[%d]", __func__, chno, portno);
		return -1;
	}

	/* do nothing for 100G port. */
	if(portno >= (PORT_ID_EAG6L_MAX - 1))
		return SUCCESS;

	if(PORT_STATUS[portno].speed == PORT_IF_25G_KR) 
		wval = WAVELENGTH_25G_TBL[chno - 1];
	else if(PORT_STATUS[portno].sfp_type == SFP_ID_SMART_BIDI_TSFP_COT)
		wval = COT_WAVELENGTH_10G_TBL[chno - 1];
	else if(PORT_STATUS[portno].sfp_type == SFP_ID_SMART_BIDI_TSFP_RT)
		wval = RT_WAVELENGTH_10G_TBL[chno - 1];
	else {
		zlog_notice("%s: invalid sfp type[%d].", __func__, PORT_STATUS[portno].sfp_type);
		return -1;
	}

	/* get writable data for wavelength */
	data = (unsigned int)(wval / 0.05);

	/* set mux to change sfp channel */
	i2c_set_sfp_channel_no(1/*bus*/, portno);

	/* write wavelength msb. */
	if(i2cset_main(1/*bus*/, DIAG_SFP_IIC_ADDR/*0x51*/, 146/*0x92*/, (data >> 8) & 0xFF) < 0) {
		zlog_notice("%s: Writing port[%d] wavelength msb failed.", __func__, portno);
		return -1;
	}

	/* write wavelength lsb. */
	if(i2cset_main(1/*bus*/, DIAG_SFP_IIC_ADDR/*0x51*/, 147/*0x93*/, data & 0xFF) < 0) {
		zlog_notice("%s: Writing port[%d] wavelength msb failed.", __func__, portno);
		return -1;
	}

	return SUCCESS;
#endif
}
#else
int update_flex_tune_status(int portno)
{
    unsigned char val;
	unsigned int data;

	/* do nothing for 100G port. */
	if(portno >= (PORT_ID_EAG6L_MAX - 1))
		return SUCCESS;

	/* clear if not configured. */
	if(! PORT_STATUS[portno].cfg_flex_tune) {
		gPortRegUpdate(__PORT_STSFP_STAT_ADDR[portno], 0, 0xFFFF, 0x0/*default*/);
		return SUCCESS;
	}

    /* set mux to change sfp channel */
    i2c_set_sfp_channel_no(1/*bus*/, portno);

    /* read for update. */
    if(val = i2cget_main(1/*bus*/, DIAG_SFP_IIC_ADDR/*0x51*/, 253/*0xFD*/) < 0) {
        zlog_notice("%s: Reading port[%d] flex tune reset failed.", __func__, portno);
        return -1;
    }

	/* update smart t-sfp status */
	data = FPGA_READ(__PORT_STSFP_STAT_ADDR[portno]);
	if(PORT_STATUS[portno].sfp_type == SFP_ID_UNKNOWN/*rtWDM?*/) {
		gPortRegUpdate(__PORT_STSFP_STAT_ADDR[portno], 8, 0xFF00, (val & 0x1) ? 0xA5 : 0x5A);
	}
	else {
		gPortRegUpdate(__PORT_STSFP_STAT_ADDR[portno], 0, 0x00FF, (val & 0x1) ? 0xA5 : 0x5A);
	}
	return SUCCESS;
}
#endif

ePrivateSfpId get_private_sfp_identifier(int portno)
{
#if 1/* [#72] Adding omitted rtWDM related registers, dustin, 2024-06-27 */
	int fd, mux_addr, ret, type = SFP_ID_UNKNOWN;
	unsigned char val1, val2, val3, val4, val5;
	unsigned int chann_mask, intval, mod;
#if 1 /* [#91] Fixing for register updating feature, dustin, 2024-08-05 */
	unsigned int retry;
#endif

	if((fd = i2c_dev_open(1/*bus*/)) < 0) {
		zlog_notice("%s : device open failed. port[%d(0/%d)] reason[%s]",
			__func__, portno, get_eag6L_dport(portno), strerror(errno));
		return ERR_NOT_FOUND;
	}

#if 1 /* [#84] Fixing for PM counters and I2C on Target system, dustin, 2024-07-30 */
	if(portno == (PORT_ID_EAG6L_MAX - 1)/*100G*/) {
		mux_addr = I2C_MUX;
		chann_mask = I2C_MUX_100G_MASK;
	} else {
		mux_addr = I2C_MUX;
		chann_mask = 1 << (portno - PORT_ID_EAG6L_PORT1);
	}
#else
	if(portno >= (PORT_ID_EAG6L_MAX - 1)/*100G*/) {
		mux_addr = I2C_MUX2;
		chann_mask = 1 << (portno - (PORT_ID_EAG6L_MAX - 1));
	} else {
		mux_addr = I2C_MUX;
		chann_mask = 1 << (portno - PORT_ID_EAG6L_PORT1);
	}

	i2c_set_slave_addr(fd, (mux_addr == I2C_MUX) ? I2C_MUX2 : I2C_MUX, 1);

	/* first disable the other mux. */
	if((ret = i2c_smbus_write_byte_data(fd, 
		0x0/*mux-data*/, 0x0)) < 0) {
		zlog_notice("%s : Disabling mux for port[%d(0/%d)] ret[%d].",
			__func__, portno, get_eag6L_dport(portno), ret);
		goto __exit_2__;
	}
#endif

	i2c_set_slave_addr(fd, mux_addr, 1);

	/* now set target mux. */
#if 1 /* [#91] Fixing for register updating feature, dustin, 2024-08-05 */
	ret = i2c_smbus_write_byte_data(fd, 0/*mux-data*/, chann_mask);
#else
	ret = i2c_smbus_write_byte_data(fd, mux_addr, chann_mask);
#endif
	if(ret < 0) {
		zlog_notice("%s : Enabling mux for port[%d(0/%d)] ret[%d].",
			__func__, portno, get_eag6L_dport(portno), ret);
		goto __exit_2__;
	}

	i2c_set_slave_addr(fd, SFP_IIC_ADDR/*0x50*/, 1);

#if 1 /* [#91] Fixing for register updating feature, dustin, 2024-08-05 */
	retry = 0;
	while(retry++ < 5) {
		if((ret = i2c_smbus_read_byte_data(fd, 65/*0x41*/)) < 0) {
			zlog_notice("%s: Reading port[%d(0/%d)] tunable flag failed. ret[%d].",
				__func__, portno, get_eag6L_dport(portno), ret);
			if(retry < 5)
				continue;
			goto __exit_2__;
		}
		if(ret >= 0)
			break;
	}
	val1 = (unsigned char)ret;
#else
	if((val1 = i2c_smbus_read_byte_data(fd, 65/*0x41*/)) < 0) {
		zlog_notice("%s: Reading port[%d(0/%d)] tunable flag failed. ret[%d].",
			__func__, portno, get_eag6L_dport(portno), val1);
		goto __exit_2__;
	}
#endif

	if(val1 & 0x40) { /* tunable type */
		PORT_STATUS[portno].tunable_sfp = 1;

		/* get 1st value */
		if((val1 = i2c_smbus_read_byte_data(fd, 124/*0x7C*/)) < 0) {
			zlog_notice("%s: Reading port[%d(0/%d)] 1st value failed. ret[%d].",
				__func__, portno, get_eag6L_dport(portno), val1);
			goto __exit_2__;
		}

		val2 = 0;

		if(portno < (PORT_ID_EAG6L_MAX - 1)) {
			i2c_set_slave_addr(fd, DIAG_SFP_IIC_ADDR/*0x51*/, 1);

			/* select page for 2nd value */
			if((ret = i2c_smbus_write_byte_data(fd, 127/*0x7F*/, 0x20/*page-20h*/)) < 0) {
				zlog_notice("%s: Writing port[%d(0/%d)] page select failed. ret[%d].",
						__func__, portno, get_eag6L_dport(portno), ret);
				goto __exit_2__;
			}

			/* wait for updating selected page */
			usleep(HZ_I2C_SLAVE_SLEEP_UM);

			/* get 2nd value */
			if((val2 = i2c_smbus_read_byte_data(fd, 252/*0xFC*/)) < 0) {
				zlog_notice("%s: Reading port[%d(0/%d)] 2nd value failed. ret[%d].",
						__func__, portno, get_eag6L_dport(portno), val2);
				goto __exit_1__;
			}
		}

		if((val1 == 0xCA) || (val2 == 0xCA)) {
			type = SFP_ID_SMART_BIDI_TSFP_COT;
			if(val2 == 0xCA)
				PORT_STATUS[portno].tunable_rtwdm_sfp_type = type;
			goto __exit_1__;
		}
		else if((val1 == 0xCB) || (val2 == 0xCB)) {
			type = SFP_ID_SMART_BIDI_TSFP_RT;
			if(val2 == 0xCB)
				PORT_STATUS[portno].tunable_rtwdm_sfp_type = type;
			goto __exit_1__;
		}
		else if((val1 == 0xCC) || (val2 == 0xCC)) {
			type = SFP_ID_SMART_DUPLEX_TSFP;
			if(val2 == 0xCC)
				PORT_STATUS[portno].tunable_rtwdm_sfp_type = type;
			goto __exit_1__;
		}
		else {
			type = SFP_ID_DWDM_TUNABLE;
			goto __exit_1__;
		}
	} else { /* non-tunable type */
		PORT_STATUS[portno].tunable_sfp = 0;

		/* get 1st value */
		if((val1 = i2c_smbus_read_byte_data(fd, 2/*0x2*/)) < 0) {
			zlog_notice("%s: Reading port[%d(0/%d)] 1st value failed. ret[%d].",
				__func__, portno, get_eag6L_dport(portno), val1);
			goto __exit_2__;
		}

		if(val1 == 0x22) {
			type = SFP_ID_CU_SFP;
			goto __exit_2__;
		}

		/* get 2nd value */
		if((val2 = i2c_smbus_read_byte_data(fd, 98/*0x62*/)) < 0) {
			zlog_notice("%s: Reading port[%d(0/%d)] 2nd value failed. ret[%d].",
				__func__, portno, get_eag6L_dport(portno), val2);
			goto __exit_2__;
		}

		switch(val2) {
			case 'V' :
				type = SFP_ID_VCSEL_BIDI;
				goto __exit_2__;
			case 'L' :
				type = SFP_ID_HSFP_LOW;
				goto __exit_2__;
			case 'H' :
				type = SFP_ID_HSFP_HIGH;
				goto __exit_2__;
			case '1' :
			case '2' :
			case '3' :
			case '4' :
			case '5' :
			case '6' :
				type = SFP_ID_6WL;
				goto __exit_2__;
			default  :  /* pass-through-to-below */;
		}

		/* get 3rd value */
		if((val3 = i2c_smbus_read_byte_data(fd, 60/*0x3C*/)) < 0) {
			zlog_notice("%s: Reading port[%d(0/%d)] 3rd value failed. ret[%d].",
					__func__, portno, get_eag6L_dport(portno), val3);
			goto __exit_2__;
		}

		/* get 4th value */
		if((val4 = i2c_smbus_read_byte_data(fd, 61/*0x3D*/)) < 0) {
			zlog_notice("%s: Reading port[%d(0/%d)] 4th value failed. ret[%d].",
					__func__, portno, get_eag6L_dport(portno), val4);
			goto __exit_2__;
		}

		intval = (val3 << 8) | val4;
		if(intval == 850) {
			type = SFP_ID_VCSEL;
			goto __exit_2__;
		}

		/* get 5th value */
		if((val5 = i2c_smbus_read_byte_data(fd, 62/*0x3E*/)) < 0) {
			zlog_notice("%s: Reading port[%d(0/%d)] 5th value failed. ret[%d].",
					__func__, portno, get_eag6L_dport(portno), val5);
			goto __exit_2__;
		}

		mod = intval % 10;
		if((mod < 2) && ((val5 == 0) || (val5 > 0x63))) {
			type = SFP_ID_CWDM;
			goto __exit_2__;
		}
		else {
			type = SFP_ID_DWDM;
			goto __exit_2__;
		}
	}

__exit_1__:
	/* recover page */
	if((ret = i2c_smbus_write_byte_data(fd, 127/*0x7F*/, 0x0/*page-0*/)) < 0)
		zlog_notice("%s: Recovering port[%d(0/%d)] page select failed. ret[%d].",
			__func__, portno, get_eag6L_dport(portno), ret);	

__exit_2__:
	close(fd);
	return type;
#else
	unsigned char val1, val2, val3, val4, val5;
	unsigned int intval, mod;
	int tunable_flag;

	/* set mux to change sfp channel */
	i2c_set_sfp_channel_no(1/*bus*/, portno);

	/* get tunable flag */
	if((tunable_flag = i2cget_main(1/*bus*/, SFP_IIC_ADDR/*0x50*/, 65/*0x41*/)) < 0) {
		zlog_notice("%s: Reading port[%d] tunable flag failed.", __func__, portno);
		return SFP_ID_UNKNOWN;
	}

	if(tunable_flag & 0x40) { /* tunable type */
		PORT_STATUS[portno].tunable_sfp = 1;
		/* get 1st value */
		if((val1 = i2cget_main(1/*bus*/, SFP_IIC_ADDR/*0x50*/, 124/*0x7C*/)) < 0) {
			zlog_notice("%s: Reading port[%d] 1st value failed.", __func__, portno);
			return SFP_ID_UNKNOWN;
		}

		val2 = 0;

		if(portno < (PORT_ID_EAG6L_MAX - 1)) {
			/* select page for 2nd value */
			if(i2cset_main(1/*bus*/, DIAG_SFP_IIC_ADDR/*0x51*/, 127/*0x7F*/, 0x20/*page-20h*/) < 0) {
				zlog_notice("%s: Writing port[%d] page select failed.", __func__, portno);
				return SFP_ID_UNKNOWN;
			}

			/* wait for updating selected page */
			usleep(HZ_I2C_SLAVE_SLEEP_UM);

			/* get 2nd value */
			if((val2 = i2cget_main(1/*bus*/, DIAG_SFP_IIC_ADDR/*0x51*/, 252/*0xFC*/)) < 0) {
				zlog_notice("%s: Reading port[%d] 2nd value failed.", __func__, portno);
				return SFP_ID_UNKNOWN;
			}

			/* recover page to default */
			if(i2cset_main(1/*bus*/, DIAG_SFP_IIC_ADDR/*0x51*/, 127/*0x7F*/, 0/*page-0*/) < 0) {
				zlog_notice("%s: Resetting port[%d] page select failed.", __func__, portno);
				return SFP_ID_UNKNOWN;
			}
		}

		if((val1 == 0xCA) || (val2 == 0xCA))
			return SFP_ID_SMART_BIDI_TSFP_COT;
		else if((val1 == 0xCB) || (val2 == 0xCB))
			return SFP_ID_SMART_BIDI_TSFP_RT;
		else if((val1 == 0xCC) || (val2 == 0xCC))
			return SFP_ID_SMART_DUPLEX_TSFP;
		else 
			return SFP_ID_DWDM_TUNABLE;
	} else { /* non-tunable type */
		PORT_STATUS[portno].tunable_sfp = 0;

		/* get 1st value */
		if((val1 = i2cget_main(1/*bus*/, SFP_IIC_ADDR/*0x50*/, 2)) < 0) {
			zlog_notice("%s: Reading port[%d] 1st value failed.", __func__, portno);
			return SFP_ID_UNKNOWN;
		}

		if(val1 == 0x22)
			return SFP_ID_CU_SFP;
		else {
			/* get 2nd value */
			if((val2 = i2cget_main(1/*bus*/, SFP_IIC_ADDR/*0x50*/, 0x62)) < 0) {
				zlog_notice("%s: Reading port[%d] 2nd value failed.", __func__, portno);
				return SFP_ID_UNKNOWN;
			}

			switch(val2) {
				case 'V' :	return SFP_ID_VCSEL_BIDI;
				case 'L' :  return SFP_ID_HSFP_LOW;
				case 'H' :  return SFP_ID_HSFP_HIGH;
				case '1' :
				case '2' :
				case '3' :
				case '4' :
				case '5' :
				case '6' :  return SFP_ID_6WL;
				default  :  /* pass-through-to-below */;
			}

			/* get 3rd value */
			if((val3 = i2cget_main(1/*bus*/, SFP_IIC_ADDR/*0x50*/, 0x3C)) < 0) {
				zlog_notice("%s: Reading port[%d] 3rd value failed.", __func__, portno);
				return SFP_ID_UNKNOWN;
			}

			/* get 4th value */
			if((val4 = i2cget_main(1/*bus*/, SFP_IIC_ADDR/*0x50*/, 0x3D)) < 0) {
				zlog_notice("%s: Reading port[%d] 4th value failed.", __func__, portno);
				return SFP_ID_UNKNOWN;
			}

			intval = (val3 << 8) | val4;
			if(intval == 850)
				return SFP_ID_VCSEL;

			/* get 5th value */
			if((val5 = i2cget_main(1/*bus*/, SFP_IIC_ADDR/*0x50*/, 0x3E)) < 0) {
				zlog_notice("%s: Reading port[%d] 5th value failed.", __func__, portno);
				return SFP_ID_UNKNOWN;
			}

			mod = intval % 10;
			if((mod < 2) && ((val5 == 0) || (val5 > 0x63)))
				return SFP_ID_CWDM;
			else
				return SFP_ID_DWDM;
		}
	}
#endif
}
#endif
#endif

#if 1/*[#25] I2C related register update, dustin, 2024-05-28 */
void i2c_set_sfp_channel_no(int bus, int portno)
{
	int fd, ret;
	int mux_addr;
	unsigned int chann_mask;

	fd = i2c_dev_open(bus);
	if(fd < 0) {
#if 1/*[68] eag6l board 를 위한 port number 수정, balkrow, 2024-06-27*/
		zlog_notice("i2c_set_sfp_channel_no : port[%d(0/%d)] device open failed. reason[%s]", 
			portno, get_eag6L_dport(portno), strerror(errno));
#else
		zlog_notice("i2c_set_sfp_channel_no : port[%d(0/%d)] device open failed. reason[%s]", 
			portno, get_eag6L_dport(portno, 0), strerror(errno));
#endif
		return;
	}

#if 1 /* [#84] Fixing for PM counters and I2C on Target system, dustin, 2024-07-30 */
	if(portno == (PORT_ID_EAG6L_MAX - 1)/*100G*/) {
		mux_addr = I2C_MUX;
		chann_mask = I2C_MUX_100G_MASK;
	} else {
		mux_addr = I2C_MUX;
		chann_mask = 1 << (portno - PORT_ID_EAG6L_PORT1);
	}
#else
	if(portno >= (PORT_ID_EAG6L_MAX - 1)/*100G*/) {
		mux_addr = I2C_MUX2;
		chann_mask = 1 << (portno - (PORT_ID_EAG6L_MAX - 1));
	} else {
		mux_addr = I2C_MUX;
		chann_mask = 1 << (portno - PORT_ID_EAG6L_PORT1);
	}

	i2c_set_slave_addr(fd, (mux_addr == I2C_MUX) ? I2C_MUX2 : I2C_MUX, 1);

	// first disable the other mux.
	ret = i2c_smbus_write_byte_data(fd, 0x0/*mux-data*/, 0x0);
#if 1/*[68] eag6l board 를 위한 port number 수정, balkrow, 2024-06-27*/
	if(ret < 0)
		zlog_notice("i2c_set_sfp_channel_no : port[%d(0/%d)] ret[%d].", 
			portno, get_eag6L_dport(portno), ret);
#else
	if(ret < 0)
		zlog_notice("i2c_set_sfp_channel_no : port[%d(0/%d)] ret[%d].", 
			portno, get_eag6L_dport(portno, 0), ret);
#endif
#endif

	i2c_set_slave_addr(fd, mux_addr, 1);

	// now set target mux.
#if 1 /* [#91] Fixing for register updating feature, dustin, 2024-08-05 */
	ret = i2c_smbus_write_byte_data(fd, 0/*mux-data*/, chann_mask);
#else
	ret = i2c_smbus_write_byte_data(fd, mux_addr, chann_mask);
#endif
#if 1/*[68] eag6l board 를 위한 port number 수정, balkrow, 2024-06-27*/
	if(ret < 0)
		zlog_notice("i2c_set_sfp_channel_no : port[%d(0/%d)] ret[%d].", 
			portno, get_eag6L_dport(portno), ret);
#else
	if(ret < 0)
		zlog_notice("i2c_set_sfp_channel_no : port[%d(0/%d)] ret[%d].", 
			portno, get_eag6L_dport(portno, 0), ret);
#endif

	close(fd);
	return;
}
#endif

int32_t i2c_smbus_read_i2c_block_data(int file, uint8_t command, uint8_t length,
                    uint8_t *values)
{
    union i2c_smbus_data data;
    int i, err;

    if (length > I2C_SMBUS_BLOCK_MAX)
        length = I2C_SMBUS_BLOCK_MAX;
    data.block[0] = length;

    err = i2c_smbus_access(file, I2C_SMBUS_READ, command,
                   length == 32 ? I2C_SMBUS_I2C_BLOCK_BROKEN :
                I2C_SMBUS_I2C_BLOCK_DATA, &data);
    if (err < 0)
        return err;

    for (i = 1; i <= data.block[0]; i++)
        values[i-1] = data.block[i];
    return data.block[0];
}

int i2cget_main_len( int bus, unsigned char addr, unsigned char start_data_addr, unsigned char read_len, unsigned char *pBuf)
{
	int fd, status;
	int succ_len;
	unsigned char ucll;
	unsigned long funcs;

	fd = i2c_dev_open(bus);

	if(fd < 0) {
		HX_I2C_DBG_U("[%s:%s:%d] fd:%d\n",__FILE__,__FUNCTION__,__LINE__,fd);
		zlog_notice("%s : device open failed. bus[%x] addr[%x] data_addr[%x] reason[%s]", 
			__func__, bus, addr, start_data_addr, strerror(errno));
		return -1;
	}

	/* check adapter functionality */
    if (ioctl(fd, I2C_FUNCS, &funcs) < 0) {
        fprintf(stderr, "Error: Could not get the adapter "
            "functionality matrix: %s\n", strerror(errno));
		zlog_notice("%s : ioctl failed. bus[%x] addr[%x] data_addr[%x] reason[%s]", 
			__func__, bus, addr, start_data_addr, strerror(errno));
        return -1;
    }

	i2c_set_slave_addr(fd, addr, 1);
	succ_len = 0;

	if (funcs & I2C_FUNC_SMBUS_READ_I2C_BLOCK) 
	{
		int i = 0;
		for(ucll = 0; ucll < read_len; ucll += i)
		{
#if 1/*[#25] I2C related register update, dustin, 2024-05-28 */
			 i = i2c_smbus_read_i2c_block_data(fd,
                        start_data_addr + ucll, 32, pBuf + ucll);
#else
			 i = i2c_smbus_read_i2c_block_data(fd,
                        ucll, 32, pBuf + ucll);
#endif

			 if (i <= 0) 
			 {
				 ucll = i;
				 break;
			 }
			 else
				succ_len+=i;
		}
	}
	else
	{ 

		for(ucll = 0; ucll < read_len; ucll++) {
			status = i2c_smbus_read_byte_data(fd, (start_data_addr+ucll));

			if(status >= 0) {
				succ_len++;
				pBuf[ucll] = (unsigned char)(status & 0xff);
			}
		}
	}
	close(fd);

	//printf("0x%02x\n",  status);
	if(read_len != succ_len) {
		HX_I2C_DBG_U("[%s:%s:%d] [B:%d] [REQ:RD] = [%d:%d] SDA:%02X[%d], PID:PPID[%d:%d]\n",
				__FILE__,__FUNCTION__,__LINE__,bus,read_len,succ_len,start_data_addr,read_len,getpid(),getppid());
	}

	return succ_len;
}

int get_value_by_register_len(
		int bus, unsigned char addr, unsigned char start_data_addr, 
		unsigned char read_len, unsigned char *pBuf
		)
{	
	if((addr == 0) || (pBuf == NULL) ||(read_len <= 0)) {
		HX_I2C_DBG_U("[ERR] Args [addr:pBuf:read_len] = [%02X:%08X:%d]\n",addr,pBuf,read_len);
		return -1;
	}

	return i2cget_main_len(bus, addr,start_data_addr, read_len, pBuf);
}

int filling_sfp_data_realtime(int bus, unsigned char addr, unsigned char port, int array_start,
		unsigned char array_size, unsigned char *test_char)
{
	int rc_len;

#if 1/*[#25] I2C related register update, dustin, 2024-05-28 */
	/* set mux to change sfp channel */
	i2c_set_sfp_channel_no(bus, port);
#endif

#if 1/*[#25] I2C related register update, dustin, 2024-05-28 */
	rc_len = get_value_by_register_len(bus, addr, array_start, array_size, &test_char[0]);
#else
	rc_len = get_value_by_register_len(bus, addr, 0, array_size, &test_char[0]);
#endif

	if(rc_len != array_size)
	{
		HX_I2C_DBG_U("[%s:%s:%d] [REQ:RD] = [%d:%d] \n", __FILE__, __FUNCTION__, __LINE__, array_size, rc_len);
	}

	return 0;
}

void  get_sfp_info(int portno, struct module_inventory * mod_inv)
{
	RawGbicInfo *raw;
	RawQsfp28UpperPage0 *raw2;
	unsigned char buf[ 256 ];
	int len_sm_1, len_mm_1, len_mm_2;
	unsigned short  wl;
	int sonet;  // 0 = not sonet, 1 = oc3, 2 = oc12, 3 = oc48
	int is_zr = 0;
	int	bus;

#if 0/*[119] don't display sfp ddm in du port, balkrow, 2022-07-27*/
	if(portno == PORT_ID_EAG6L_PORT1)
		bus = 26;
	else if(portno == PORT_ID_EAG6L_PORT2)
		bus = 27;
	else if(portno == PORT_ID_EAG6L_PORT3)
		bus = 28;
	else if(portno == PORT_ID_EAG6L_PORT4)
		bus = 29;
	else if(portno >= PORT_ID_EAG6L_PORT5)
		bus = portno - 1;
	else
	bus = portno + 1;
#endif
	bus = 1;

#if 1/*[#25] I2C related register update, dustin, 2024-05-28 */
	filling_sfp_data_realtime(bus, SFP_IIC_ADDR, portno, 
			((portno >= (PORT_ID_EAG6L_MAX - 1)/*100G*/) ? 
				HZ_I2C_SFP_INFO_START + HZ_I2C_SFP_INFO : 
				HZ_I2C_SFP_INFO_START), HZ_I2C_SFP_INFO,
			&slot_sfp_info.data[portno][HZ_I2C_SFP_INFO_START]);
#else
	filling_sfp_data_realtime(bus, SFP_IIC_ADDR, portno, HZ_I2C_SFP_INFO_START, HZ_I2C_SFP_INFO,
			&slot_sfp_info.data[portno][HZ_I2C_SFP_INFO_START]);
#endif

	raw = (RawGbicInfo *)&slot_sfp_info.data[portno][HZ_I2C_SFP_INFO_START];
#if 1/*[#25] I2C related register update, dustin, 2024-05-28 */
	if(portno >= (PORT_ID_EAG6L_MAX - 1)/*100G*/)
		raw2 = (RawQsfp28UpperPage0 *)&slot_sfp_info.data[portno][HZ_I2C_SFP_INFO_START];
#endif

	module_is_hisense = 0;
	module_is_superxon = 0;
	module_is_gpon = 0;

#if 1/*[#25] I2C related register update, dustin, 2024-05-28 */
	if(portno >= (PORT_ID_EAG6L_MAX - 1)/*100G*/) {
		wl = (short)raw2->wavelength[0];
		wl = wl << 8;
		wl = wl | (short)raw2->wavelength[1];

		mod_inv->wave = wl;
		mod_inv->dist = raw2->len_smf;
		if(mod_inv->dist == 0)
			mod_inv->dist = raw2->len_om3_50um;/*unit-of-2m*/
		if(mod_inv->dist == 0)
			mod_inv->dist = raw2->len_om2_50um;/*unit-of-1m*/
		if(mod_inv->dist == 0)
			mod_inv->dist = raw2->len_om1_62dot5um;/*unit-of-1m*/
		mod_inv->max_rate = raw2->br_nominal;
		if(mod_inv->max_rate == 0xFF)
			mod_inv->max_rate = raw2->br_nominal2;
		memcpy( &mod_inv->date_code, &raw2->date_code[0], 8 );
		memcpy( &mod_inv->serial_num, &raw2->vendor_sn[0], 16 );
		memcpy( &mod_inv->vendor, &raw2->vendor_name[0], 16 );
		memcpy( &mod_inv->part_num, &raw2->vendor_pn[0], 16 );
	} else {
		wl = (short)raw->wavelength[0];
		wl = wl << 8;
		wl = wl | (short)raw->wavelength[1];

		mod_inv->wave = wl;
		mod_inv->dist = raw->length_km;
		mod_inv->max_rate = raw->br_nominal;
		memcpy( &mod_inv->date_code, &raw->date_code[0], 8 );
		memcpy( &mod_inv->serial_num, &raw->vendor_sn[0], 16 );
		memcpy( &mod_inv->vendor, &raw->vendor_name[0], 16 );
		memcpy( &mod_inv->part_num, &raw->vendor_pn[0], 16 );
	}

#if 1/* [#72] Adding omitted rtWDM related registers, dustin, 2024-06-27 */
	/* remove heading/tailing spaces */
	trim(mod_inv->vendor);
	trim(mod_inv->part_num);
	trim(mod_inv->serial_num);
	trim(mod_inv->date_code);
#endif
#else
	wl = (short)raw->wavelength[0];
	wl = wl << 8;
	wl = wl | (short)raw->wavelength[1];

	mod_inv->wave = wl;
	mod_inv->dist = raw->length_km;
	mod_inv->max_rate = raw->br_nominal;
	memcpy( &mod_inv->date_code, &raw->date_code[0], 8 );
	memcpy( &mod_inv->serial_num, &raw->vendor_sn[0], 16 );
	memcpy( &mod_inv->vendor, &raw->vendor_name[0], 16 );
	memcpy( &mod_inv->part_num, &raw->vendor_pn[0], 16 );
#endif
	return;
}

#if 1/* [#72] Adding omitted rtWDM related registers, dustin, 2024-06-27 */
void  get_sfp_rtwdm_info(int portno, struct module_inventory * mod_inv)
{
	RawGbicInfo *raw;
	unsigned short  wl;
	unsigned char ucll, addr, start_data_addr, read_len;
	unsigned char *pBuf = NULL;
	unsigned int chann_mask;
	unsigned long funcs;
	int	fd, mux_addr, ret, succ_len, bus = 1;

	// check if tunable sfp and not a 100G port.
	if((portno >= (PORT_ID_EAG6L_MAX - 1)) || (! PORT_STATUS[portno].tunable_sfp))
		return;

	fd = i2c_dev_open(bus);
	if(fd < 0) {
		zlog_notice("%s : device open failed. port[%d(0/%d)] reason[%s]",
			__func__, portno, get_eag6L_dport(portno), strerror(errno));
		goto __exit__;
	}

#if 1 /* [#84] Fixing for PM counters and I2C on Target system, dustin, 2024-07-30 */
	if(portno == (PORT_ID_EAG6L_MAX - 1)/*100G*/) {
		mux_addr = I2C_MUX;
		chann_mask = I2C_MUX_100G_MASK;
	} else {
		mux_addr = I2C_MUX;
		chann_mask = 1 << (portno - PORT_ID_EAG6L_PORT1);
	}
#else
	if(portno >= (PORT_ID_EAG6L_MAX - 1)/*100G*/) {
		mux_addr = I2C_MUX2;
		chann_mask = 1 << (portno - (PORT_ID_EAG6L_MAX - 1));
	} else {
		mux_addr = I2C_MUX;
		chann_mask = 1 << (portno - PORT_ID_EAG6L_PORT1);
	}

	i2c_set_slave_addr(fd, (mux_addr == I2C_MUX) ? I2C_MUX2 : I2C_MUX, 1);

	// first disable the other mux.
	ret = i2c_smbus_write_byte_data(fd, 0x0/*mux-data*/, 0x0);
	if(ret < 0) {
		zlog_notice("%s : port[%d(0/%d)] ret[%d].",
			__func__, portno, get_eag6L_dport(portno), ret);
		goto __exit__;
	}
#endif

	i2c_set_slave_addr(fd, mux_addr, 1);

	// now set target mux.
#if 1 /* [#91] Fixing for register updating feature, dustin, 2024-08-05 */
	ret = i2c_smbus_write_byte_data(fd, 0/*mux-data*/, chann_mask);
#else
	ret = i2c_smbus_write_byte_data(fd, mux_addr, chann_mask);
#endif
	if(ret < 0) {
		zlog_notice("%s : port[%d(0/%d)] ret[%d].",
			__func__, portno, get_eag6L_dport(portno), ret);
		goto __exit__;
	}

	 /* select page */
    if(i2cset_main(1/*bus*/, DIAG_SFP_IIC_ADDR/*0x51*/, 127/*0x7F*/, 0x20/*page-20h*/) < 0) {
        zlog_notice("%s: Writing port[%d(0/%d)] page 20 select failed.", 
			__func__, portno, get_eag6L_dport(portno));
		goto __exit__;
    }

	usleep(HZ_I2C_SLAVE_SLEEP_UM);

	addr = DIAG_SFP_IIC_ADDR;
	start_data_addr = HZ_I2C_SFP_DIAG_START;
	read_len = HZ_I2C_SFP_INFO;
	pBuf = &slot_sfp_info.rtwdm_data[portno][HZ_I2C_SFP_INFO_START];

	/* check adapter functionality */
	if (ioctl(fd, I2C_FUNCS, &funcs) < 0) {
		zlog_notice("%s : adapter functionality get ioctl failed. "
			"bus[%x] addr[%x] data_addr[%x] reason[%s]",
			__func__, bus, addr, start_data_addr, strerror(errno));
		goto __exit__;
	}

	i2c_set_slave_addr(fd, addr, 1);
	succ_len = 0;

	if (funcs & I2C_FUNC_SMBUS_READ_I2C_BLOCK)
	{
		int i = 0;
		for(ucll = 0; ucll < read_len; ucll += i)
		{
			i = i2c_smbus_read_i2c_block_data(fd,
					start_data_addr + ucll, 32, pBuf + ucll);
			if (i <= 0)
			{
				ucll = i;
				break;
			}
			else
				succ_len+=i;
		}
	}
	else
	{

		for(ucll = 0; ucll < read_len; ucll++) {
			ret = i2c_smbus_read_byte_data(fd, (start_data_addr+ucll));

			if(ret >= 0) {
				succ_len++;
				pBuf[ucll] = (unsigned char)(ret & 0xff);
			}
		}
	}

     /* recover select page */
    if(i2cset_main(1/*bus*/, DIAG_SFP_IIC_ADDR/*0x51*/, 127/*0x7F*/, 0x0/*page-0*/) < 0) {
        zlog_notice("%s: Recovering port[%d(0/%d)] page select failed.",
            __func__, portno, get_eag6L_dport(portno));
        goto __exit__;
    }

	if(succ_len != HZ_I2C_SFP_INFO)
	{
        zlog_notice("%s: Reading port[%d(0/%d)] length failed.", 
			__func__, portno, get_eag6L_dport(portno));
		goto __exit__;
	}

	raw = (RawGbicInfo *)&slot_sfp_info.rtwdm_data[portno][HZ_I2C_SFP_INFO_START];

	wl = (short)raw->wavelength[0];
	wl = wl << 8;
	wl = wl | (short)raw->wavelength[1];

	mod_inv->wave = wl;
	mod_inv->dist = raw->length_km;
	mod_inv->max_rate = raw->br_nominal;
	memcpy( &mod_inv->date_code, &raw->date_code[0], 8 );
	memcpy( &mod_inv->serial_num, &raw->vendor_sn[0], 16 );
	memcpy( &mod_inv->vendor, &raw->vendor_name[0], 16 );
	memcpy( &mod_inv->part_num, &raw->vendor_pn[0], 16 );

	trim(mod_inv->vendor);
	trim(mod_inv->part_num);
	trim(mod_inv->serial_num);
	trim(mod_inv->date_code);

__exit__:
	close(fd);
	return;
}
#endif

int sfp_get_ad_us(uint8_t msb, uint8_t lsb, unsigned short *ad)
{
  uint16_t sfp_ad;

  sfp_ad = msb << 8;
  sfp_ad |= lsb;

  *ad = sfp_ad;


  return 0;
}

int sfp_get_ad(uint8_t msb, uint8_t lsb, double *ad)
{
  uint16_t sfp_ad;
  double sfp_ad_d;

  sfp_ad = msb << 8;
  sfp_ad |= lsb;

  sfp_ad_d = (double)sfp_ad;

  *ad = sfp_ad_d;

  return 0;
}

int sfp_get_slope(uint8_t msb, uint8_t lsb, double *slope)
{
  double sfp_slope;

  sfp_slope = (double)msb+ ((double)lsb/256.0);

  *slope = sfp_slope;

  return 0;
}

int sfp_get_offset(uint8_t msb, uint8_t lsb, double *offset)
{
  int16_t sfp_offset;
  double sfp_offset_d;

  sfp_offset = msb << 8;
  sfp_offset |= lsb;

  sfp_offset_d = (double)sfp_offset;

  *offset = sfp_offset_d;

  return 0;
}

int sfp_get_float(uint8_t mem0, uint8_t mem1, uint8_t mem2, uint8_t mem3, float *sfp_float)
{
  uint32_t sfp_byte;
  float sfp_float_f;

  sfp_byte = mem0;
  sfp_byte <<= 8;
  sfp_byte |= mem1;
  sfp_byte <<= 8;
  sfp_byte |= mem2;
  sfp_byte <<= 8;
  sfp_byte |= mem3;

  memcpy(&sfp_float_f, &sfp_byte,4);

  *sfp_float = sfp_float_f;

  return 0;
}

int sfp_get_tx_power(double tx_pwrad, double tx_pwr_slope, double tx_pwr_offset, double *tx_db) {
  double tx_power, tx_mw;

  tx_power = tx_pwr_slope * tx_pwrad + tx_pwr_offset;
  if(tx_power) {
		  tx_mw = tx_power / 10000.0;
	} else {
		  tx_mw = 0.0001;
	}
#if 1
  if(module_is_gpon == 1)
    *tx_db = 10 * log10(tx_mw);
  else
    if((module_is_hisense || module_is_superxon))
      *tx_db = 10 * log10(tx_mw*2);
    else
      *tx_db = 10 * log10(tx_mw);
#endif

  return 0;
}

int sfp_get_rx_power(
    unsigned short rx_pwrad,
    float rx_pwr0,
    float rx_pwr1,
    float rx_pwr2,
    float rx_pwr3,
    float rx_pwr4,
    double *rx_db)
{
  double rx_power, rx_mw;

  rx_power = ((double)rx_pwr4) * (pow((double)rx_pwrad, 4))
    + ((double)rx_pwr3) * (pow((double)rx_pwrad, 3))
    + ((double)rx_pwr2) * (pow((double)rx_pwrad, 2))
    + ((double)rx_pwr1) * ((double)rx_pwrad)
    + ((double)rx_pwr0);

  if(rx_power) {
    rx_mw = rx_power / 10000.0;
  } else {
    rx_mw = 0.0001;
  }

  *rx_db = 10 * log10(rx_mw);

  return 0;
}

int sfp_get_temp(double temp_ad, double temp_slope, double temp_offset, double *temp)
{
  *temp = (temp_slope * temp_ad + temp_offset) * 1 / 256;

  return 0;
}

int sfp_get_vcc(double vcc_ad, double vcc_slope, double vcc_offset, double *vcc)
{
#if 1   /* sfp, martin, 2022.05.18 [BEGIN] */
  *vcc = (vcc_slope * vcc_ad + vcc_offset) / 1000.0;
#else
  *vcc = (vcc_slope * vcc_ad + vcc_offset) / 10000.0;
#endif /* sfp, martin, 2022.05.18 [END] */

  return 0;
}

int sfp_get_bias(double bias_ad, double bias_slope, double bias_offset, double *bias)
{
  *bias = ((bias_slope * bias_ad + bias_offset) * 2.0) / 1000.0;

  return 0;
}

int get_sfp_info_diag(int portno, port_status_t * port_sts)
{
	int	bus = 1;
	RawGbicDiagInfo *raw_diag = NULL;

	double tx_db, rx_db;
	double tx_pwr_slope, tx_pwr_offset, temp_slope, temp_offset, vcc_slope, vcc_offset, bias_slope, bias_offset;
	float rx_pwr0, rx_pwr1, rx_pwr2, rx_pwr3, rx_pwr4;
	unsigned short rx_pwrad_us;

	double tx_pwrad, temp_ad, vcc_ad, bias_ad;
	double temp, vcc, bias, ltemp, tcurr;

	/* do nothing for 100G port. */
	if(portno >= (PORT_ID_EAG6L_MAX - 1))
		return SUCCESS;

#if 1/*[#61] Adding omitted functions, dustin, 2024-06-24 */
	filling_sfp_data_realtime(bus, DIAG_SFP_IIC_ADDR, portno, 0, HZ_I2C_SFP_DIAG,
			&slot_sfp_info.data[portno][HZ_I2C_SFP_DIAG_START]);
#else
	filling_sfp_data_realtime(bus, DIAG_SFP_IIC_ADDR, portno, HZ_I2C_SFP_DIAG_START, HZ_I2C_SFP_DIAG,
			&slot_sfp_info.data[portno][HZ_I2C_SFP_DIAG_START]);
#endif

	raw_diag = (RawGbicDiagInfo *)&slot_sfp_info.data[portno][HZ_I2C_SFP_DIAG_START];

	// Temperature.
	sfp_get_ad(raw_diag->diagnostics[0], raw_diag->diagnostics[1], &temp_ad);
	sfp_get_slope(raw_diag->ext_cal_constants[28], raw_diag->ext_cal_constants[29], &temp_slope);
	sfp_get_offset(raw_diag->ext_cal_constants[30], raw_diag->ext_cal_constants[31], &temp_offset);
	sfp_get_temp(temp_ad, temp_slope, temp_offset, &temp);

	// VCC
	sfp_get_ad(raw_diag->diagnostics[2], raw_diag->diagnostics[3], &vcc_ad);
	sfp_get_slope(raw_diag->ext_cal_constants[32], raw_diag->ext_cal_constants[33], &vcc_slope);
	sfp_get_offset(raw_diag->ext_cal_constants[34], raw_diag->ext_cal_constants[35], &vcc_offset);
	sfp_get_vcc(vcc_ad, vcc_slope, vcc_offset, &vcc);

	// TX Bias
	sfp_get_ad(raw_diag->diagnostics[4], raw_diag->diagnostics[5], &bias_ad);
	sfp_get_slope(raw_diag->ext_cal_constants[20], raw_diag->ext_cal_constants[21], &bias_slope);
	sfp_get_offset(raw_diag->ext_cal_constants[22], raw_diag->ext_cal_constants[23], &bias_offset);
	sfp_get_bias(bias_ad, bias_slope, bias_offset, &bias);

	// Tx Power
	sfp_get_ad(raw_diag->diagnostics[6], raw_diag->diagnostics[7], &tx_pwrad);
	sfp_get_slope(raw_diag->ext_cal_constants[24], raw_diag->ext_cal_constants[25], &tx_pwr_slope);
	sfp_get_offset(raw_diag->ext_cal_constants[26], raw_diag->ext_cal_constants[27], &tx_pwr_offset);
	sfp_get_tx_power(tx_pwrad, tx_pwr_slope, tx_pwr_offset, &tx_db);

	// RX Power
	sfp_get_ad_us(raw_diag->diagnostics[8], raw_diag->diagnostics[9], &rx_pwrad_us);
	sfp_get_float(raw_diag->ext_cal_constants[0],
			raw_diag->ext_cal_constants[1],
			raw_diag->ext_cal_constants[2],
			raw_diag->ext_cal_constants[3],
			&rx_pwr4);
	sfp_get_float(raw_diag->ext_cal_constants[4],
			raw_diag->ext_cal_constants[5],
			raw_diag->ext_cal_constants[6],
			raw_diag->ext_cal_constants[7],
			&rx_pwr3);
	sfp_get_float(raw_diag->ext_cal_constants[8],
			raw_diag->ext_cal_constants[9],
			raw_diag->ext_cal_constants[10],
			raw_diag->ext_cal_constants[11],
			&rx_pwr2);
	sfp_get_float(raw_diag->ext_cal_constants[12],
			raw_diag->ext_cal_constants[13],
			raw_diag->ext_cal_constants[14],
			raw_diag->ext_cal_constants[15],
			&rx_pwr1);
	sfp_get_float(raw_diag->ext_cal_constants[16],
			raw_diag->ext_cal_constants[17],
			raw_diag->ext_cal_constants[18],
			raw_diag->ext_cal_constants[19],
			&rx_pwr0);
	sfp_get_rx_power(rx_pwrad_us, rx_pwr0, rx_pwr1, rx_pwr2, rx_pwr3, rx_pwr4, &rx_db);

	// Laser temperature
	sfp_get_ad(raw_diag->optional_diag[0], raw_diag->optional_diag[1], &ltemp);
#if 1 /* [#91] Fixing for register updating feature, dustin, 2024-08-05 */
	sfp_get_slope(raw_diag->ext_cal_constants[28], raw_diag->ext_cal_constants[29], &temp_slope);
	sfp_get_offset(raw_diag->ext_cal_constants[30], raw_diag->ext_cal_constants[31], &temp_offset);
	sfp_get_temp(ltemp, temp_slope, temp_offset, &ltemp);
#endif

	// TEC Current
	sfp_get_ad(raw_diag->optional_diag[2], raw_diag->optional_diag[3], &tcurr);
#if 1 /* [#91] Fixing for register updating feature, dustin, 2024-08-05 */
	sfp_get_slope(raw_diag->ext_cal_constants[32], raw_diag->ext_cal_constants[33], &vcc_slope);
	sfp_get_offset(raw_diag->ext_cal_constants[34], raw_diag->ext_cal_constants[35], &vcc_offset);
	sfp_get_vcc(tcurr, vcc_slope, vcc_offset, &tcurr);
#endif

#if 0
	cprintf("bias:%+14.4f ", bias);
	cprintf("vcc:%+10.4f ", vcc);
	cprintf("temp:%+14.4f ", temp);
	cprintf("tx_db:%+16.1f ", tx_db);
	cprintf("rx_db:%+13.1f\n", rx_db);

	if(vcc == 0)
	{
		cprintf("%s Error on this module.... [%s:%d] \n", __func__, __FILE__, __LINE__);
	}
#endif

	port_sts->rx_pwr = (float)rx_db;
	port_sts->tx_pwr = (float)tx_db;
	port_sts->vcc = (float)vcc;
	port_sts->temp = (float)temp;
	port_sts->tx_bias = (float)bias;
	port_sts->laser_temp = (float)ltemp;
	port_sts->tec_curr = (float)tcurr;

	return (0);
}

#if 1/* [#72] Adding omitted rtWDM related registers, dustin, 2024-06-27 */
void get_sfp_rtwdm_info_diag(int portno, port_status_t * port_sts)
{
	RawGbicDiagInfo *raw_diag0 = NULL;
	RawGbicDiagInfo *raw_diag = NULL;
	double tx_db, rx_db;
	double tx_pwr_slope, tx_pwr_offset, temp_slope, temp_offset;
	double vcc_slope, vcc_offset, bias_slope, bias_offset;
	double tx_pwrad, temp_ad, vcc_ad, bias_ad;
	double temp, vcc, bias, ltemp, tcurr, wval;
	float rx_pwr0, rx_pwr1, rx_pwr2, rx_pwr3, rx_pwr4;
	unsigned short rx_pwrad_us;
	unsigned char ucll, addr, start_data_addr, read_len;
	unsigned char *pBuf = NULL;
	unsigned int chann_mask;
	unsigned long funcs;
	int fd, mux_addr, ret, succ_len, tunable_flag, type, val, data, bus = 1;

#if 1 /* [#91] Fixing for register updating feature, dustin, 2024-08-05 */
	if(! PORT_STATUS[portno].equip)
		return;
#endif

	// check if tunable sfp and not a 100G port.
	if((portno >= (PORT_ID_EAG6L_MAX - 1)) || (! PORT_STATUS[portno].tunable_sfp))
		return;

	fd = i2c_dev_open(bus);
	if(fd < 0) {
		zlog_notice("%s : device open failed. port[%d(0/%d)] reason[%s]",
			__func__, portno, get_eag6L_dport(portno), strerror(errno));
		goto __exit__;
	}

#if 1 /* [#84] Fixing for PM counters and I2C on Target system, dustin, 2024-07-30 */
	if(portno == (PORT_ID_EAG6L_MAX - 1)/*100G*/) {
		mux_addr = I2C_MUX;
		chann_mask = I2C_MUX_100G_MASK;
	} else {
		mux_addr = I2C_MUX;
		chann_mask = 1 << (portno - PORT_ID_EAG6L_PORT1);
	}
#else
	if(portno >= (PORT_ID_EAG6L_MAX - 1)/*100G*/) {
		mux_addr = I2C_MUX2;
		chann_mask = 1 << (portno - (PORT_ID_EAG6L_MAX - 1));
	} else {
		mux_addr = I2C_MUX;
		chann_mask = 1 << (portno - PORT_ID_EAG6L_PORT1);
	}

	i2c_set_slave_addr(fd, (mux_addr == I2C_MUX) ? I2C_MUX2 : I2C_MUX, 1);

	// first disable the other mux.
	ret = i2c_smbus_write_byte_data(fd, 0x0/*mux-data*/, 0x0);
	if(ret < 0) {
        zlog_notice("%s : port[%d(0/%d)] ret[%d].",
			__func__, portno, get_eag6L_dport(portno), ret);
		goto __exit__;
	}
#endif

	i2c_set_slave_addr(fd, mux_addr, 1);

	// now set target mux.
#if 1 /* [#91] Fixing for register updating feature, dustin, 2024-08-05 */
	ret = i2c_smbus_write_byte_data(fd, 0/*mux-data*/, chann_mask);
#else
	ret = i2c_smbus_write_byte_data(fd, mux_addr, chann_mask);
#endif
	if(ret < 0) {
		zlog_notice("%s : port[%d(0/%d)] ret[%d].",
			__func__, portno, get_eag6L_dport(portno), ret);
		goto __exit__;
	}

	i2c_set_slave_addr(fd, DIAG_SFP_IIC_ADDR/*0x51*/, 1);

	/* select page */
	if(i2c_smbus_write_byte_data(fd, 127/*0x7F*/, 0x20/*page-20h*/) < 0) {
		zlog_notice("%s: Writing port[%d(0/%d)] page 20 select failed.",
			__func__, portno, get_eag6L_dport(portno));
		goto __exit__;
	}

	/* wait for updating selected page */
	usleep(HZ_I2C_SLAVE_SLEEP_UM);

	/* read tunable flag. */
	if((tunable_flag = i2c_smbus_read_byte_data(fd, (128 + 65)/*0xC1*/)) < 0) {
		zlog_notice("%s: Reading port[%d(0/%d)] rtwdm wavelength msb failed.", 
			__func__, portno, get_eag6L_dport(portno));
		goto __exit__;
	}

	/* read tunable type. */
	if((type = i2c_smbus_read_byte_data(fd, (128 + 0x7C)/*0xFC*/)) < 0) {
		zlog_notice("%s: Reading port[%d(0/%d)] rtwdm wavelength msb failed.", 
			__func__, portno, get_eag6L_dport(portno));
		goto __exit__;
	}

	/* update rtwdm sfp type */
	if(tunable_flag) {
		if(type == 0xCA)
			PORT_STATUS[portno].tunable_rtwdm_sfp_type = SFP_ID_SMART_BIDI_TSFP_COT;
		else if(type == 0xCB)
			PORT_STATUS[portno].tunable_rtwdm_sfp_type = SFP_ID_SMART_BIDI_TSFP_RT;
		else if(type == 0xCC)
			PORT_STATUS[portno].tunable_rtwdm_sfp_type = SFP_ID_SMART_DUPLEX_TSFP;
		else {
			PORT_STATUS[portno].tunable_rtwdm_sfp_type = SFP_ID_UNKNOWN;
			zlog_notice("%s: port[%d(0/%d)] rtwdm sfp type unknown.", 
				__func__, portno, get_eag6L_dport(portno));
		}
	}

	/* select page for wavelength */
	if(i2c_smbus_write_byte_data(fd, 127/*0x7F*/, 0x24/*page-24h*/) < 0) {
		zlog_notice("%s: Writing port[%d(0/%d)] page 24 select failed.",
				__func__, portno, get_eag6L_dport(portno));
		goto __exit__;
	}

	/* wait for updating selected page */
	usleep(HZ_I2C_SLAVE_SLEEP_UM);

	/* read wavelength msb. */
	if((val = i2c_smbus_read_byte_data(fd, 146/*0x92*/)) < 0) {
		zlog_notice("%s: Reading port[%d(0/%d)] rtwdm wavelength msb failed.", 
			__func__, portno, get_eag6L_dport(portno));
		goto __exit__;
	}

	data = ((unsigned int)val << 8);

	/* read wavelength lsb. */
	if((val = i2c_smbus_read_byte_data(fd, 147/*0x93*/)) < 0) {
		zlog_notice("%s: Reading port[%d(0/%d)] rtwdm wavelength lsb failed.", 
			__func__, portno, get_eag6L_dport(portno));
		goto __exit__;
	}

	data |= val;

	/* get real wavelength in float */
	wval = (double)(0.05 * data);
	if(wval == 0) {
#ifdef DEBUG
		zlog_notice("%s: invalid port[%d(0/%d)] rtwdm wavelength[%7.2f] data[%d].",
			__func__, portno, get_eag6L_dport(portno), wval, data);
#endif
	}

	PORT_STATUS[portno].tunable_rtwdm_wavelength = wval;

	/* select page */
	if(i2cset_main(1/*bus*/, DIAG_SFP_IIC_ADDR/*0x51*/, 127/*0x7F*/, 0x22/*page-22h*/) < 0) {
		zlog_notice("%s: Writing port[%d(0/%d)] page 2 select failed.",
				__func__, portno, get_eag6L_dport(portno));
		goto __exit__;
	}

	usleep(HZ_I2C_SLAVE_SLEEP_UM);

    addr = DIAG_SFP_IIC_ADDR;
    start_data_addr = HZ_I2C_SFP_DIAG_START;
    read_len = HZ_I2C_SFP_DIAG;
    pBuf = &slot_sfp_info.rtwdm_data[portno][HZ_I2C_SFP_DIAG_START];

    /* check adapter functionality */
    if (ioctl(fd, I2C_FUNCS, &funcs) < 0) {
        zlog_notice("%s : adapter functionality get ioctl failed. "
            "bus[%x] addr[%x] data_addr[%x] reason[%s]",
            __func__, bus, addr, start_data_addr, strerror(errno));
        goto __exit__;
    }

    i2c_set_slave_addr(fd, addr, 1);
    succ_len = 0;

    if (funcs & I2C_FUNC_SMBUS_READ_I2C_BLOCK)
    {
        int i = 0;
        for(ucll = 0; ucll < read_len; ucll += i)
        {
            i = i2c_smbus_read_i2c_block_data(fd,
                    start_data_addr + ucll, 32, pBuf + ucll);
            if (i <= 0)
            {
                ucll = i;
                break;
            }
            else
                succ_len+=i;
        }
    }
    else
    {

        for(ucll = 0; ucll < read_len; ucll++) {
            ret = i2c_smbus_read_byte_data(fd, (start_data_addr+ucll));

            if(ret >= 0) {
                succ_len++;
                pBuf[ucll] = (unsigned char)(ret & 0xff);
            }
        }
    }

     /* recover select page */
    if(i2cset_main(1/*bus*/, DIAG_SFP_IIC_ADDR/*0x51*/, 127/*0x7F*/, 0x0/*page-0*/) < 0) {
        zlog_notice("%s: Recovering port[%d(0/%d)] page select failed.",
            __func__, portno, get_eag6L_dport(portno));
        goto __exit__;
    }

    if(succ_len != HZ_I2C_SFP_INFO)
    {
        zlog_notice("%s: Reading port[%d(0/%d)] length failed.",
            __func__, portno, get_eag6L_dport(portno));
        goto __exit__;
    }

	/* NOTE : rtWDM's ext_cal_constants are all zero, so need to use local values. */
	raw_diag0 = (RawGbicDiagInfo *)&slot_sfp_info.data[portno][HZ_I2C_SFP_DIAG_START];
	raw_diag  = (RawGbicDiagInfo *)&slot_sfp_info.rtwdm_data[portno][HZ_I2C_SFP_DIAG_START];

	// Temperature.
	sfp_get_ad(raw_diag->diagnostics[0], raw_diag->diagnostics[1], &temp_ad);
	sfp_get_slope(raw_diag0->ext_cal_constants[28], raw_diag0->ext_cal_constants[29], &temp_slope);
	sfp_get_offset(raw_diag0->ext_cal_constants[30], raw_diag0->ext_cal_constants[31], &temp_offset);
	sfp_get_temp(temp_ad, temp_slope, temp_offset, &temp);

	// VCC
	sfp_get_ad(raw_diag->diagnostics[2], raw_diag->diagnostics[3], &vcc_ad);
	sfp_get_slope(raw_diag0->ext_cal_constants[32], raw_diag0->ext_cal_constants[33], &vcc_slope);
	sfp_get_offset(raw_diag0->ext_cal_constants[34], raw_diag0->ext_cal_constants[35], &vcc_offset);
	sfp_get_vcc(vcc_ad, vcc_slope, vcc_offset, &vcc);

	// TX Bias
	sfp_get_ad(raw_diag->diagnostics[4], raw_diag->diagnostics[5], &bias_ad);
	sfp_get_slope(raw_diag0->ext_cal_constants[20], raw_diag0->ext_cal_constants[21], &bias_slope);
	sfp_get_offset(raw_diag0->ext_cal_constants[22], raw_diag0->ext_cal_constants[23], &bias_offset);
	sfp_get_bias(bias_ad, bias_slope, bias_offset, &bias);

	// Tx Power
	sfp_get_ad(raw_diag->diagnostics[6], raw_diag->diagnostics[7], &tx_pwrad);
	sfp_get_slope(raw_diag0->ext_cal_constants[24], raw_diag0->ext_cal_constants[25], &tx_pwr_slope);
	sfp_get_offset(raw_diag0->ext_cal_constants[26], raw_diag0->ext_cal_constants[27], &tx_pwr_offset);
	sfp_get_tx_power(tx_pwrad, tx_pwr_slope, tx_pwr_offset, &tx_db);

	// RX Power
	sfp_get_ad_us(raw_diag->diagnostics[8], raw_diag->diagnostics[9], &rx_pwrad_us);
	sfp_get_float(raw_diag0->ext_cal_constants[0],
			raw_diag0->ext_cal_constants[1],
			raw_diag0->ext_cal_constants[2],
			raw_diag0->ext_cal_constants[3],
			&rx_pwr4);
	sfp_get_float(raw_diag0->ext_cal_constants[4],
			raw_diag0->ext_cal_constants[5],
			raw_diag0->ext_cal_constants[6],
			raw_diag0->ext_cal_constants[7],
			&rx_pwr3);
	sfp_get_float(raw_diag0->ext_cal_constants[8],
			raw_diag0->ext_cal_constants[9],
			raw_diag0->ext_cal_constants[10],
			raw_diag0->ext_cal_constants[11],
			&rx_pwr2);
	sfp_get_float(raw_diag0->ext_cal_constants[12],
			raw_diag0->ext_cal_constants[13],
			raw_diag0->ext_cal_constants[14],
			raw_diag0->ext_cal_constants[15],
			&rx_pwr1);
	sfp_get_float(raw_diag0->ext_cal_constants[16],
			raw_diag0->ext_cal_constants[17],
			raw_diag0->ext_cal_constants[18],
			raw_diag0->ext_cal_constants[19],
			&rx_pwr0);
	sfp_get_rx_power(rx_pwrad_us, rx_pwr0, rx_pwr1, rx_pwr2, rx_pwr3, rx_pwr4, &rx_db);

	// Laser temperature
	sfp_get_ad(raw_diag->optional_diag[0], raw_diag->optional_diag[1], &ltemp);
#if 1 /* [#91] Fixing for register updating feature, dustin, 2024-08-05 */
	sfp_get_slope(raw_diag0->ext_cal_constants[28], raw_diag0->ext_cal_constants[29], &temp_slope);
	sfp_get_offset(raw_diag0->ext_cal_constants[30], raw_diag0->ext_cal_constants[31], &temp_offset);
	sfp_get_temp(ltemp, temp_slope, temp_offset, &ltemp);
#endif

	// TEC Current
	sfp_get_ad(raw_diag->optional_diag[2], raw_diag->optional_diag[3], &tcurr);
#if 1 /* [#91] Fixing for register updating feature, dustin, 2024-08-05 */
	sfp_get_slope(raw_diag0->ext_cal_constants[32], raw_diag0->ext_cal_constants[33], &vcc_slope);
	sfp_get_offset(raw_diag0->ext_cal_constants[34], raw_diag0->ext_cal_constants[35], &vcc_offset);
	sfp_get_vcc(tcurr, vcc_slope, vcc_offset, &tcurr);
#endif

	port_sts->rtwdm_ddm_info.rx_pwr = (float)rx_db;
	port_sts->rtwdm_ddm_info.tx_pwr = (float)tx_db;
	port_sts->rtwdm_ddm_info.vcc = (float)vcc;
	port_sts->rtwdm_ddm_info.temp = (float)temp;
	port_sts->rtwdm_ddm_info.tx_bias = (float)bias;
	port_sts->rtwdm_ddm_info.laser_temp = (float)ltemp;
	port_sts->rtwdm_ddm_info.tec_curr = (float)tcurr;

__exit__:
	close(fd);
	return;
}
#endif

#if 1 /* [#91] Fixing for register updating feature, dustin, 2024-08-05 */
void init_100g_sfp(void)
{
	unsigned int chann_mask;
	int fd, mux_addr, ret, portno = PORT_ID_EAG6L_PORT7;
	unsigned char val;

	if(! PORT_STATUS[portno].equip)
		return;

	fd = i2c_dev_open(1/*bus*/);
	if(fd < 0) {
		zlog_notice("%s : device open failed. port[%d(0/%d)] reason[%s]",
			__func__, portno, get_eag6L_dport(portno), strerror(errno));
		goto __exit__;
	}

	if(portno == (PORT_ID_EAG6L_MAX - 1)/*100G*/) {
		mux_addr = I2C_MUX;
		chann_mask = I2C_MUX_100G_MASK;
	} else {
		mux_addr = I2C_MUX;
		chann_mask = 1 << (portno - PORT_ID_EAG6L_PORT1);
	}

	i2c_set_slave_addr(fd, mux_addr, 1);

	// now set target mux.
#if 1 /* [#91] Fixing for register updating feature, dustin, 2024-08-05 */
	ret = i2c_smbus_write_byte_data(fd, 0/*mux-data*/, chann_mask);
#else
	ret = i2c_smbus_write_byte_data(fd, mux_addr, chann_mask);
#endif
	if(ret < 0) {
		zlog_notice("%s : port[%d(0/%d)] ret[%d].",
				__func__, portno, get_eag6L_dport(portno), ret);
		goto __exit__;
	}

	i2c_set_slave_addr(fd, SFP_IIC_ADDR/*0x50*/, 1);

	/* select page */
	if((ret = i2c_smbus_write_byte_data(fd, 127/*0x7F*/, 0x3/*page-3*/)) < 0) {
		zlog_notice("%s: Writing port[%d(0/%d)] page select failed.",
			__func__, portno, get_eag6L_dport(portno));
		goto __exit__;
	}

	/* wait for updating selected page */
	usleep(HZ_I2C_SLAVE_SLEEP_UM);

	if((ret = i2c_smbus_read_byte_data(fd, 230/*0xE6*/)) < 0) {
		zlog_notice("%s: Reading port[%d(0/%d)] host side fec failed. ret[%d].",
			__func__, portno, get_eag6L_dport(portno), ret);
		goto __exit__;
	}
	val = ret;

	val |= 0x80;/*set-host-side-fec*/

	if((ret = i2c_smbus_write_byte_data(fd, 230/*0xE6*/, val)) < 0) {
		zlog_notice("%s: Writing port[%d(0/%d)] host side fec failed. ret[%d].",
			__func__, portno, get_eag6L_dport(portno), ret);
		goto __exit__;
	}

	/* recover page to default */
	if((ret = i2c_smbus_write_byte_data(fd, 127/*0x7F*/, 0x0/*page-0*/)) < 0) {
		zlog_notice("%s: Recovering port[%d(0/%d)] page select failed. ret[%d].",
			__func__, portno, get_eag6L_dport(portno), ret);
		goto __exit__;
	}

__exit__:
    close(fd);
    return;
}
#endif

#if 1 /* [#100] Adding update of Laser status by Laser_con, dustin, 2024-08-23 */
int set_i2c_100G_laser_control(int portno, int enable)
{
	unsigned int chann_mask;
	int fd, mux_addr, ret;
	unsigned char val;
	unsigned char lp_back;

	if(! PORT_STATUS[portno].equip)
		return;

	fd = i2c_dev_open(1/*bus*/);
	if(fd < 0) {
		zlog_notice("%s : device open failed. port[%d(0/%d)] reason[%s]",
			__func__, portno, get_eag6L_dport(portno), strerror(errno));
		goto __exit__;
	}

	if(portno == (PORT_ID_EAG6L_MAX - 1)/*100G*/) {
		mux_addr = I2C_MUX;
		chann_mask = I2C_MUX_100G_MASK;
	} else {
		mux_addr = I2C_MUX;
		chann_mask = 1 << (portno - PORT_ID_EAG6L_PORT1);
	}

	i2c_set_slave_addr(fd, mux_addr, 1);

	// now set target mux.
	ret = i2c_smbus_write_byte_data(fd, 0/*mux-data*/, chann_mask);
	if(ret < 0) {
		zlog_notice("%s : port[%d(0/%d)] ret[%d].",
			__func__, portno, get_eag6L_dport(portno), ret);
		goto __exit__;
	}

	i2c_set_slave_addr(fd, SFP_IIC_ADDR/*0x50*/, 1);

	i2c_set_slave_addr(fd, SFP_IIC_ADDR/*0x50*/, 1);

	/* read power mode */
	if((ret = i2c_smbus_read_byte_data(fd, 93/*0x5D*/)) < 0) {
		zlog_notice("%s: Reading port[%d(0/%d)] power mode failed.",
			__func__, portno, get_eag6L_dport(portno));
		goto __exit__;
	}
	lp_back = ret;

	/* set low power mode */
	if((ret = i2c_smbus_write_byte_data(fd, 93/*0x5D*/, 0x3)) < 0) {
		zlog_notice("%s: Writing port[%d(0/%d)] low power mode failed.",
			__func__, portno, get_eag6L_dport(portno));
		goto __exit__;
	}

	/* read txDisable control byte, page 00h, byte 86(0x56), bit 0-3. */
	if((ret = i2c_smbus_read_byte_data(fd, 86/*0x56*/)) < 0) {
		zlog_notice("%s: Reading port[%d(0/%d)] TxDisable failed. ret[%d].",
			__func__, portno, get_eag6L_dport(portno), ret);
		goto __exit__;
	}

	val = ret;
	val &= ~0xF;
	val |= enable ? 0x0 : 0xF;/*tx-disable-4-lanes*/

	/* update txDisable control byte, page 00h, byte 86(0x56), bit 0-3. */
	if((ret = i2c_smbus_write_byte_data(fd, 86/*0x56*/, val)) < 0) {
		zlog_notice("%s: Writing port[%d(0/%d)] host side fec failed. ret[%d].",
			__func__, portno, get_eag6L_dport(portno), ret);
		goto __exit__;
	}

	/* recover to high power mode */
	if((ret = i2c_smbus_write_byte_data(fd, 93/*0x5D*/, lp_back)) < 0) {
		zlog_notice("%s: Writing port[%d(0/%d)] low power mode failed.",
			__func__, portno, get_eag6L_dport(portno));
		goto __exit__;
	}

__exit__:
	close(fd);
	return;
}
#endif

