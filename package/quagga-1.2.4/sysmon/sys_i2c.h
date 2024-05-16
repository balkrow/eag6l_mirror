#ifndef __SYS_I2C_H_8374387438473858937843 
#define __SYS_I2C_H_8374387438473858937843 

#define	HZ_I2C_SFP_INFO				128
#define	HZ_I2C_SFP_DIAG				128

#define	HZ_I2C_SFP_INFO_START		0		
#define	HZ_I2C_SFP_DIAG_START		(HZ_I2C_SFP_INFO_START + HZ_I2C_SFP_INFO)


#define	HZ_I2C_SLAVE_RETRY			9		/* total 10 times */
#define	HZ_I2C_SLAVE_SLEEP_UM		10000	/* 10 ms */

/*
 * 1. Maximum len is 255 because read_len is unsigned char
 * 2. Return
 *   (1) -1 : argument error
 *   (2) read byte length successfuly
 *     1) if (read_len > return value ) then partiall error
 *     2) if (read_len == return value ) then succedd all
 */
int get_value_by_register_len(
	int bus, unsigned char addr, unsigned char start_data_addr, 
	unsigned char read_len, unsigned char *pBuf);


int get_value_by_register(int bus, unsigned char addr, unsigned char data_addr);
int set_value_by_register(int bus, unsigned char addr, unsigned char data_addr, unsigned char value);
int set_system_reset(void);
int tx_disable(unsigned char uplink, unsigned char port, unsigned char flag);
int debug_sfp_eeprom_read(unsigned char port);
int debug_sfp_eeprom_diag_read(unsigned char port);
int get_mac_by_ioctl(unsigned char * mac);


#define FAIL_TO_GET_TEMPERATURE 0x7F

#endif /* __SYS_I2C_H_8374387438473858937843 */
