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
#define IF_MAX_PORT_NUM       28
#define IF_AGGR_MAX_NUM       IF_TOTAL_NUM  /* 12 */
#define IF_AGGR_CLI_RANGE     "<1-12>"

#define HZ_I2C_SFP_INFO       128
#define HZ_I2C_SFP_DIAG       128

#define SFP_IIC_ADDR      0x50
#define DIAG_SFP_IIC_ADDR 0x51
#define CPLD_IIC_ADDR     0x55

#define SFP_INS1                   0x20
#define SFP_INS2                   0x21

int module_is_hisense;
int module_is_superxon;
int module_is_gpon;
#endif /* FIXME, martin, 2022.05.11 [END] */

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
  unsigned char diagnostics[10];
  unsigned char unallocated_4[4];
  unsigned char status_control;
  unsigned char reserved;

  // Alarm and Warning Flag Bits
  unsigned char alarm_flags[2];
  unsigned char unallocated_2[2];
  unsigned char warning_flags[2];

  // Extended Control/Status Memory Addresses
  unsigned char ext_status_control[2];

  // Vendor Specific Memory Addresses
  unsigned char vendor_specific[8];

  // User EEPROM
  unsigned char user_eeprom[120];

  // Vendor Control Function Addresses
    unsigned char vendor_control[8];
} RawGbicDiagInfo;


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
} slot_sfp_info_t;

static struct _slot_sfp_info_t slot_sfp_info;

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
	char filename[sizeof("/dev/i2c-%d", i2cbus)];
	int fd;

	sprintf(filename, "/dev/i2c-%d", i2cbus);
	fd = open(filename, O_RDWR);
	if (fd < 0) 
	{
		printf("can't open %s \n", filename);
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
		return -1;
	}
	i2c_set_slave_addr(fd, addr, 1);

	status = i2c_smbus_read_byte_data(fd, data_addr);

	close(fd);

	if (status < 0) {
		printf("read failed\n");
		HX_I2C_DBG_U("[%s:%s:%d] [B:%d] read failed status:%d DA:%02X, PID:PPID[%d:%d]\n",
				__FILE__,__FUNCTION__,__LINE__,bus,status,data_addr,getpid(),getppid());
	}

	//printf("0x%02x\n",  status);

	return status;
}

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
		return -1;
	}

	/* check adapter functionality */
    if (ioctl(fd, I2C_FUNCS, &funcs) < 0) {
        fprintf(stderr, "Error: Could not get the adapter "
            "functionality matrix: %s\n", strerror(errno));
        return -1;
    }

	i2c_set_slave_addr(fd, addr, 1);
	succ_len = 0;

	if (funcs & I2C_FUNC_SMBUS_READ_I2C_BLOCK) 
	{
		int i = 0;
		for(ucll = 0; ucll < read_len; ucll += i)
		{
			 i = i2c_smbus_read_i2c_block_data(fd,
                        ucll, 32, pBuf + ucll);

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

	rc_len = get_value_by_register_len(bus, addr, 0, array_size, &test_char[0]);

	if(rc_len != array_size)
	{
		HX_I2C_DBG_U("[%s:%s:%d] [REQ:RD] = [%d:%d] \n", __FILE__, __FUNCTION__, __LINE__, array_size, rc_len);
	}

	return 0;
}

void  get_sfp_info(int portno, struct module_inventory * mod_inv)
{
	RawGbicInfo *raw;
	unsigned char buf[ 256 ];
	int len_sm_1, len_mm_1, len_mm_2;
	unsigned short  wl;
	int sonet;  // 0 = not sonet, 1 = oc3, 2 = oc12, 3 = oc48
	int is_zr = 0;
	int	bus;

#if 0//PWY_FIXME
#if 1/*[119] don't display sfp ddm in du port, balkrow, 2022-07-27*/
	if(portno == PORT_ID_FHM_CELL1_DU1)
		bus = 26;
	else if(portno == PORT_ID_FHM_CELL1_DU2)
		bus = 27;
	else if(portno == PORT_ID_FHM_CELL2_DU1)
		bus = 28;
	else if(portno == PORT_ID_FHM_CELL2_DU2)
		bus = 29;
	else if(portno >= PORT_ID_FHM_CELL2_RU1)
		bus = portno - 1;
	else
#endif
#endif //PWY_FIXME
	bus = portno + 1;

	filling_sfp_data_realtime(bus, SFP_IIC_ADDR, portno, HZ_I2C_SFP_INFO_START, HZ_I2C_SFP_INFO,
			&slot_sfp_info.data[portno-IF_MIN_PORT_NUM][HZ_I2C_SFP_INFO_START]);

	raw = &slot_sfp_info.data[portno-1][HZ_I2C_SFP_INFO_START];

	len_sm_1 = 0;
	if( raw->length_km )
		len_sm_1 = raw->length_km*1000;
	else
		len_sm_1 = raw->length_100m*100;

	len_mm_1 = 0;
	if( raw->length_10m_50 )
		len_mm_1 = raw->length_10m_50*10;
	if( raw->length_10m_62_5 )
		len_mm_2 = raw->length_10m_62_5*10;

	memset(&buf[0], 0, sizeof(buf));

	if( len_sm_1 )
	{
		memset(&buf[0], 0, sizeof(buf));
		itoac(len_sm_1,buf);
		if(len_sm_1 > 40000)
		{
			is_zr = 1;
		}
		else
		{
			is_zr = 0;
		}
	}

	bzero( &buf[0], 250 );
	memcpy( &buf[0], &raw->vendor_name[0], 16 );

	module_is_hisense = 0;
	module_is_superxon = 0;
	module_is_gpon = 0;

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

	return;
}

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

int get_sfp_info_diag(int portno, struct port_status * port_sts)
{
	int	bus;
	RawGbicDiagInfo *raw_diag = NULL;

	double tx_db, rx_db;
	double tx_pwr_slope, tx_pwr_offset, temp_slope, temp_offset, vcc_slope, vcc_offset, bias_slope, bias_offset;
	float rx_pwr0, rx_pwr1, rx_pwr2, rx_pwr3, rx_pwr4;
	unsigned short rx_pwrad_us;

	double tx_pwrad, temp_ad, vcc_ad, bias_ad;
	double temp, vcc, bias;

#if 1/*[119] don't display sfp ddm in du port, balkrow, 2022-07-27*/
#if 0//PWY_FIXME
	if(portno == PORT_ID_FHM_CELL1_DU1)
		bus = 26;
	else if(portno == PORT_ID_FHM_CELL1_DU2)
		bus = 27;
	else if(portno == PORT_ID_FHM_CELL2_DU1)
		bus = 28;
	else if(portno == PORT_ID_FHM_CELL2_DU2)
		bus = 29;
	else if(portno >= PORT_ID_FHM_CELL2_RU1)
		bus = portno - 1;
	else
#endif //PWY_FIXME
		bus = portno + 1;
#else
	bus = portno + 1;
#endif
	filling_sfp_data_realtime(bus, DIAG_SFP_IIC_ADDR, portno, HZ_I2C_SFP_DIAG_START, HZ_I2C_SFP_DIAG,
			&slot_sfp_info.data[portno-IF_MIN_PORT_NUM][HZ_I2C_SFP_DIAG_START]);

	raw_diag = &slot_sfp_info.data[portno-1][HZ_I2C_SFP_DIAG_START];

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
	sfp_get_ad(raw_diag->diagnostics[5], raw_diag->diagnostics[6], &bias_ad);
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
	port_sts->vcc = vcc;
	port_sts->temp = temp;
	port_sts->tx_bias = bias;

	return (0);
}

