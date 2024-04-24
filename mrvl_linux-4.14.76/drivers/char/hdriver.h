#ifndef _HDRIVER_H
#define _HDRIVER_H

// kernel ioctl cmd id
#define HDRIVER_VERSION_NUM						0x00000002         
#define HDRIVER_IOCG_ALL							0x01
#define HDRIVER_IOCG_FLAGS							0x02
#define HDRIVER_IOCG_GPIO_SPI_RTL8368_READ			0x30
#define HDRIVER_IOCS_GPIO_SPI_RTL8368_WRITE			0x31
#define HDRIVER_IOCG_GPIO_DIR_READ					0x80
#define HDRIVER_IOCS_GPIO_DIR_WRITE					0x81
#define HDRIVER_IOCG_GPIO_DAT_READ					0x82
#define HDRIVER_IOCS_GPIO_DAT_WRITE					0x83
#define HDRIVER_IOCG_SHOW_MEMORY					0xd0
#define HDRIVER_IOCS_WRITE_MEMORY					0xd1
#define HDRIVER_IOCG_DPRAM_SHOW_MEMORY				0xd2
#define HDRIVER_IOCS_DPRAM_WRITE_MEMORY				0xd3
#define HDRIVER_IOCG_FPGA_SHOW_MEMORY				0xd4
#define HDRIVER_IOCS_FPGA_WRITE_MEMORY				0xd5
#define HDRIVER_IOCG_CPLD_SHOW_MEMORY				0xd6
#define HDRIVER_IOCS_CPLD_WRITE_MEMORY				0xd7
#define HDRIVER_IOCS_TEST_SIGNAL					0xe0
#define HDRIVER_IOCS_TEST_POLLWAIT					0xe1
#define HDRIVER_IOCS_TEST_FPGA_CUT					0xe2
#if 1//modified by balkrow
#define HDRIVER_IOCG_GPIO2_DIR_READ					0xe3
#define HDRIVER_IOCS_GPIO2_DIR_WRITE				0xe4
#define HDRIVER_IOCG_GPIO2_DAT_READ					0xe5
#define HDRIVER_IOCS_GPIO2_DAT_WRITE				0xe6
#define HDRIVER_IOCS_GPIO2_DAT_WRITE				0xe6
#define HDRIVER_IOCS_GPIO_WRITE_BIT					0xe7
#define HDRIVER_IOCS_GPIO_READ_BIT					0xe8
#endif
#define HDRIVER_IOCS_WATCH_DOG						0xf0


#define HDRIVER_FLAG_NONE					0x00000001
#define HDRIVER_FLAG_SDCARD_INSERT				0x00000002
#define HDRIVER_FLAG_SV_LINK				0x00000004
#define HDRIVER_FLAG_LED_SW					0x00000008
#define HDRIVER_FLAG_TEST						0x80000000
#define HDRIVER_FLAG_WATCHDOG_COUNT_LOW			0x01000000

#define HDRIVER_SHELF_TYPE_IUG_INDOOR			0x1
#define HDRIVER_SHELF_TYPE_IUG_OUTDOOR			0x2

#define HDRIVER_MEMORY_TYPE_READ				0x1
#define HDRIVER_MEMORY_TYPE_WRITE				0x2


/* FPGA Offset of MCU  Register Map */ 
#define FPGA_REG_EXT_STATUS			0x42		//Extend status register (R/WC, 0xF900_0042)

typedef struct board_inventory_s {
	unsigned char manufacturer[10];
	unsigned char model_name[10];
	unsigned char part_number[16];
	unsigned char serial_number[16];
	unsigned char manufacture_date[10];
	unsigned int  revision;
	unsigned char repair_date[10];
	unsigned int  repair_code;
	unsigned char clei_number[11];
	unsigned char usi_number[26];
	unsigned char etc[1];
	unsigned char cfiu_serial[16];      // cfiu serial
} board_inventory_t;

#if 0
typedef struct slotmemory {
	unsigned int addr;
	unsigned short value;
} slotmemory_t;

typedef struct fpgamemory {
	unsigned int addr;
	unsigned short value;
} fpgamemory_t;
#else
typedef struct devmem_t {
	unsigned int addr;
	unsigned short value;
} devmemory_t;

#define fpgamemory_t devmemory_t
#define cpldmemory_t devmemory_t
#define dprammemory_t devmemory_t
#endif

typedef struct showmemory {
	unsigned int addr;
	int type;
	unsigned int value;
	long long dvalue;	
} showmemory_t;

typedef struct writememory {
	unsigned int addr;
	int type;
	unsigned int value;
	long long dvalue;
	int count;
} writememory_t;

typedef struct hdriver_gpio_spi_rtl8368_data {
	unsigned int s_addr;
	unsigned int value;
	unsigned int alen;
	unsigned int dlen;
} hdriver_gpio_spi_rtl8368_data_t;


struct hdriver_priv_data {
	unsigned int	flags;
	unsigned short	model_type;    /* shelf_type */
	unsigned short	epld_ver;
	unsigned char	slot_id;
	short			reserved1;
	unsigned int	system_memory;
	unsigned char	sdcard_insert;
	char sdcard_name[32];
	char sdcard_cap[32];
	int version;
	int result;
	int reserved_2[4];
};

unsigned int hdriver_slot_num(void);
#endif // _HDRIVER_H

