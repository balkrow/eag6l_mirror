/*====================================================
*       MCU RDL REQ STATE
*=====================================================*/
#define  DPRAM_RDL_REQ_ADDR				0x00

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
#define  DPRAM_RDL_RESP_ADDR			0x02

	#define RDL_START_ACK_BIT				0x11

	#define RDL_P1_WRITING_ACK_BIT			0x12
	#define RDL_P1_WRITING_DONE_ACK_BIT		0x13

	#define RDL_P2_WRITING_ACK_BIT			0x14
	#define RDL_P2_WRITING_DONE_ACK_BIT		0x15

	#define RDL_PAGE_READING_DONE_BIT		0x16
	
	#define RDL_TOTAL_READING_DONE_BIT		0x0E

	#define RDL_PAGE_READING_ERROR_BIT		0x82
	#define RDL_PAGE_WRITING_ERROR_ACK_BIT	0x91

	#define RDL_TOTAL_READING_ERROR_BIT		0xA2
	#define RDL_TOTAL_WRITING_ERROR_BIT		0xB1
/*====================================================
*       RDL DPRAM 
*=====================================================*/
#define RDL_PAGE_CHECKSUM			0x10      // R
#define RDL_WRITING_BANK			0x14     // R_W


/*====================================================
*       BANK1 HEADER INFO ADDR
*=====================================================*/



/*====================================================
*       BANK2 HEADER INFO ADDR
*=====================================================*/





 enum {
	RDL_ST_IDLE,
	RDL_ST_START,`
	RDL_WRITING_P1,
	RDL_READING_P1,
	RDL_WRITING_P2,
	RDL_READING_P2,
	RDL_WRITING_TOTAL,
	RDL_READING_TOTAL,
	RDL_IMG_ACTIVE_DONE,
	RDL_IMG_RUNNING_CHECK,
	RDL_ST_TERM,
	RDL_ST_MAX,
} RDL_ST;

typedef enum {
	MCU_RDL_START,
	MCU_RDL_WRITING_P1,
	MCU_RDL_WRITING_DONE_P1,
	MCU_RDL_WRITING_P2,
	MCU_RDL_WRITING_DONE_P2,
	MCU_RDL_WRIGTING_ERROR,
	RDL_READING_ERROR,
	MCU_WRITING_DONE_TOTAL,
	MCU_WRITING_NOT_DONE,
	MCU_WRITING_ERROR_TOTAL,
	RDL_READING_ERROR_TOTAL,
	RDL_IMG_ACTIVE_SUCCESS,
	RDL_IMG_ACTIVE_FAIL,
	RDL_IMG_RUNNING_SUCCESS,
	RDL_IMG_RUNNING_FAIL,
	RDL_EVT_MAX,
} RDL_EVT;

typedef struct fdb_state {
	RDL_ST st;
	RDL_EVT evt;
} RDL_INFO_LIST;

typedef struct sv_mng_fsm {
    SV_ST state;
    SV_EVT evt;
    SV_ST (* func)(SV_FPGA);
} RDL_FSM;

#define SV_TRANS_MAX 19
