/*====================================================
*       MCU RDL REQ STATE
*=====================================================*/
#define RDL_NONE_BIT					0x00

#define RDL_START_BIT					0x01		
	
#define RDL_P1_WRITING_BIT				0x02
#define RDL_P1_WRITING_DONE_BIT			0x03

#define RDL_P2_WRITING_BIT				0x04
#define RDL_P2_WRITING_DONE_BIT			0x05

#define RDL_PAGE_WRITING_ERROR_BIT		0x81
	
#define RDL_TOTAL_WRITING_DONE_BIT		0x0C
#define RDL_TOTAL_WRITING_ERROR_BIT		0xA1

/*====================================================
*       BP RDL RESP STATE
*=====================================================*/
#define RDL_START_ACK_BIT				0x11

#define RDL_P1_WRITING_ACK_BIT			0x12
#define RDL_P1_WRITING_DONE_ACK_BIT		0x13

#define RDL_P2_WRITING_ACK_BIT			0x14
#define RDL_P2_WRITING_DONE_ACK_BIT		0x15

#define RDL_PAGE_READING_DONE_BIT		0x16
#define RDL_IMG_ACTIVATE_DONE_BIT       0x17/*FIXME : it's ok to add this ? */

#define RDL_TOTAL_READING_DONE_BIT		0x0E

#define RDL_PAGE_READING_ERROR_BIT		0x82
#define RDL_PAGE_WRITING_ERROR_ACK_BIT	0x91

#define RDL_TOTAL_READING_ERROR_BIT		0xA2
#define RDL_TOTAL_WRITING_ERROR_ACK_BIT	0xB1


#define RDL_IMG_MAGIC               0x48534E38

#define RDL_DFT_PATH                "/mnt/flash/"
#define RDL_IMG_PATH                "/mnt/flash/.RDL/"
#define RDL_PKG_INFO_FILE           "/mnt/flash/.RDL/.pkg_info"
#define RDL_TEMP_ZIP_FILE           "/mnt/flash/.RDL/_pkt_.zip"

#if 1 /* [#89] Fixing for RDL changes on Target system, dustin, 2024-08-02 */
#define RDL_B1_PATH                 "/media/bank1/"
#define RDL_B2_PATH                 "/media/bank2/"

#define RDL_B1_PKG_INFO_FILE        "/media/bank1/.pkg_info"
#define RDL_B2_PKG_INFO_FILE        "/media/bank2/.pkg_info"

#define RDL_B1_LINK_PATH            "/media/bank1/uImage"
#define RDL_B2_LINK_PATH            "/media/bank2/uImage"

#define RDL_DEV1_FPGA_FW            "/dev/mtd0" /* FPGA bank1 */
#define RDL_DEV2_FPGA_FW            "/dev/mtd1" /* FPGA bank2 */
#else /*************************************************************/
#define RDL_B1_PATH                 "/mnt/flash/Bank1/"
#define RDL_B2_PATH                 "/mnt/flash/Bank2/"

#define RDL_B1_PKG_INFO_FILE        "/mnt/flash/Bank1/.pkg_info"
#define RDL_B2_PKG_INFO_FILE        "/mnt/flash/Bank2/.pkg_info"

#define RDL_BOOT_PATH               "/mnt/flash/boot/"
#define RDL_BOOT_LINK_PATH          "/mnt/flash/boot/uImage"

#define RDL_DEV_FPGA_OS             "/mnt/flash/FPGA_OS"
#if 1 /*[#82] eag6l board SW Debugging, balkrow, 2024-07-29*/
#define RDL_DEV_FPGA_FW             "/dev/mtd1" /*bank1 : /dev/mtd1, bank2 : /dev/mtd2*/
#endif
#if 0//PWY_FIXME
#define RDL_DEV_FPGA_OS             "/dev/fpga_os"
#define RDL_DEV_FPGA_FW             "/dev/fpga_fw"
#endif //PWY_FIXME
#endif

#define RDL_ACTIVE_BANK_FLAG        "/mnt/flash/.activate.bank"
#if 1 /* [#91] Fixing for register updating feature, dustin, 2024-08-05 */
#define RDL_FPGA_BANK_FLAG          "/media/.fpga.bank"
#endif

#define RDL_BUFF_SIZE               8196

#define RDL_PAGE_SEGMENT_1          1
#define RDL_PAGE_SEGMENT_2          2

#define RDL_VER_STR_MAX             16/* or 2? */
#define RDL_FILE_NAME_MAX           32

#define RDL_BANK_1                  1
#define RDL_BANK_2                  2

#define two_bcd2dec(v)  ((v&0xF) + ((v&0xF0)>>4)*10 + ((v&0xF00)>>8)*100) + (((v&0xF000)>>12)*1000)
#define one_bcd2dec(v)  ((v & 0xF) + ((v & 0xF0) >> 4)*10)


typedef enum {
	RDL_CRC_FAIL,
	RDL_CRC_OK
} RDL_CRC_t;

typedef enum {
	ST_RDL_IDLE,
#if 1/* [#77] Adding RDL emulation function, dustin, 2024-07-16 */
	ST_RDL_TRIGGER,
#endif
	ST_RDL_START,
	ST_RDL_WRITING_P1,
	ST_RDL_READING_P1,
	ST_RDL_WRITING_P2,
	ST_RDL_READING_P2,
	ST_RDL_WRITING_TOTAL,
	ST_RDL_READING_TOTAL,
	ST_RDL_ACTIVATE_DONE,
	ST_RDL_RUNNING_CHECK,
	ST_RDL_TERM,
	ST_RDL_ST_MAX,
} RDL_ST_t;

typedef enum {
	EVT_RDL_NONE,
#if 1/* [#77] Adding RDL emulation function, dustin, 2024-07-16 */
	EVT_RDL_TRIGGER,
#endif
	EVT_RDL_START,
	EVT_RDL_WRITING_P1,
	EVT_RDL_WRITING_DONE_P1,
	EVT_RDL_WRITING_P2,
	EVT_RDL_WRITING_DONE_P2,
	EVT_RDL_WRITING_ERROR,
	EVT_RDL_READING_DONE_P2,
	EVT_RDL_READING_ERROR,
	EVT_RDL_WRITING_DONE_TOTAL,
	EVT_RDL_WRITING_NOT_DONE,
	EVT_RDL_WRITING_ERROR_TOTAL,
	EVT_RDL_READING_DONE_TOTAL,
	EVT_RDL_READING_ERROR_TOTAL,
	EVT_RDL_IMG_ACTIVE_SUCCESS,
	EVT_RDL_IMG_ACTIVE_FAIL,
	EVT_RDL_IMG_RUNNING_SUCCESS,
	EVT_RDL_IMG_RUNNING_FAIL,
	EVT_RDL_EVT_MAX,
} RDL_EVT_t;

typedef struct fdb_state {
	RDL_ST_t st;
	RDL_EVT_t evt;
} RDL_INFO_LIST_t;

typedef struct rdl_fsm {
    RDL_ST_t  state;
    RDL_EVT_t evt;
    RDL_ST_t (* func)(void);
} RDL_FSM_t;

typedef struct rdl_img_header {
	uint32_t           magic;/* 0x48534E38 (HSN8) */
	uint32_t           total_crc;
	uint32_t           build_time;
	uint32_t           total_size;
	uint8_t            ver_str[RDL_VER_STR_MAX];
	uint8_t            file_name[RDL_FILE_NAME_MAX];
} RDL_IMG_HEADER_t;

typedef struct rdl_info {
	uint8_t           bno;/* target bank no */
	uint8_t           pno;/* current page no */
	uint8_t           total_pno;/* total page no */
	uint16_t          pcrc;/* current page crc */

	RDL_IMG_HEADER_t  hd;
} RDL_IMG_INFO_t;

typedef struct fw_image_header {
    unsigned int    fih_magic;  /* Image Header Magic Number    */
    unsigned int    fih_hcrc;   /* Image Header CRC Checksum    */
    unsigned int    fih_time;   /* Image Creation Timestamp */
    unsigned int    fih_size;   /* Image Data Size      */
    unsigned int    fih_card_type;  /* card type        */
    unsigned int    fih_dcrc;   /* Image Data CRC Checksum  */
    char        fih_ver[RDL_VER_STR_MAX];    /* Image Version        */
    char        fih_name[RDL_FILE_NAME_MAX];    /* Image Name       */
    char        reserv[8];  /* reserv       */
} fw_image_header_t;

typedef struct fwimage_header {
    uint32_t    ih_prg_size;        /* Image Header Magic Number    */
    uint16_t    ih_crc;             /* Image Header CRC Checksum    */
    char        ih_manuf_id[2];     /* Manufacturer identifier  */
    char        ih_name_of_manuf[16];   /* Manufacturer name        */
    char        ih_ftype_name[16];  /* File Type name       */
    int16_t     ih_ftype;           /* File Type        */
    char        ih_major[2];            /* File Type        */
    char        ih_minor[2];            /* File Type        */
    char        ih_rev[2];          /* File Type        */
    int16_t     ih_year;            /* File Type        */
    int8_t      ih_month;           /* File Type        */
    int8_t      ih_day;         /* File Type        */
    char        ih_manuf_id2[2];        /* Manufacturer identifier  */
    int8_t      ih_image1_str[64];      /* 1st file_name     */
    int8_t      ih_image2_str[64];      /* 2nd file_name     */
    int8_t      ih_image3_str[64];      /* 3rd file_name     */
    int8_t      ih_reserved[266];       /* Operating System     */
    int32_t     ih_os_size;     /* os size */
    int32_t     ih_fpga_size;       /* fpga size */
    int8_t      ih_specific[504];   /* Originally 512 byte  */
} fw_header_t;
