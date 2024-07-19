#if 1/*[#73] SDK 내에서 CPU trap 된 packet 처리 로직 추가, balkrow, 2024-07-16*/
#include <linux/if.h>
#include <linux/if_ether.h>
#include "esmc_types.h"
#endif


#define SYSMON_FIFO_READ "/tmp/.sysmonrdfifo"
#define SYSMON_FIFO_WRITE "/tmp/.sysmonwrfifo"

typedef struct sysmon_priv_data {
} SYS_IPC_DATA;

#if 1/*[#34] aldrin3s chip initial 기능 추가, balkrow, 2024-05-23*/
typedef uint8_t (*cCPSSToSysmonFuncs) (int args, ...);
#endif

#if 0/*[#43] LF발생시 RF 전달 기능 추가, balkrow, 2024-06-05*/
typedef struct portFaultstr {
	uint16_t port;
	uint16_t flag; 
	uint32_t link; 
} PortFaultList;
#endif

#if 1/*[#71] EAG6L Board Bring-up, balkrow, 2024-07-04*/
typedef struct
{
	GT_UINTPTR          evHndl;
	GT_U32              hndlrIndex;
} EAG6L_HNDLR_PARAM;
#endif

#if 1/*[#73] SDK 내에서 CPU trap 된 packet 처리 로직 추가, balkrow, 2024-07-16*/
#define ESMC_PDU_LEN   60
#define ESMC_PDU_PADDING_LEN   12

#define ESMC_PDU_MISC_FIELD_LEN           10
#define ESMC_PDU_SLOW_PROTO_SUBTYPE_LEN   1
#define ESMC_PDU_ITU_OUI_LEN              3
#define ESMC_PDU_ITU_SUBTYPE_LEN          2
#define ESMC_PDU_VER_EVENT_LEN            1
#define ESMC_PDU_RSVD_LEN                 3 
#define ESMC_PDU_QL_TLV_LEN          4
#define ESMC_PDU_QL_TLV_TYPE_LEN     1
#define ESMC_PDU_QL_TLV_LENGTH_LEN   2
#define ESMC_PDU_QL_SSM_CODE_LEN     1
#define ESMC_PDU_EXT_QL_TLV_LEN                     20
#define ESMC_PDU_EXT_QL_TLV_TYPE_LEN                1
#define ESMC_PDU_EXT_QL_TLV_LENGTH_LEN              2
#define ESMC_PDU_EXT_QL_TLV_ENHANCED_SSM_CODE_LEN   1
#define ESMC_PDU_EXT_QL_TLV_SYNCE_CLOCK_ID_LEN      8
#define ESMC_PDU_EXT_QL_TLV_FLAG_LEN                1
#define ESMC_PDU_EXT_QL_TLV_NUM_CASCADED_EEEC_LEN   1
#define ESMC_PDU_EXT_QL_TLV_NUM_CASCADED_EEC_LEN    1
#define ESMC_PDU_EXT_QL_TLV_RSVD_LEN                5
#define MAX_CLK_ID_LEN   8

/* QL TLV */
struct __attribute__((packed)) ql_tlv {
	unsigned char type;
	unsigned char length[ESMC_PDU_QL_TLV_LENGTH_LEN];
	unsigned char ssm_code;
};
typedef struct ql_tlv T_esmc_ql_tlv;

/* Extended QL TLV */
struct __attribute__((packed)) ext_ql_tlv {
	unsigned char type;
	unsigned char length[ESMC_PDU_EXT_QL_TLV_LENGTH_LEN];
	unsigned char esmc_e_ssm_code;
	unsigned char syncE_clock_id[ESMC_PDU_EXT_QL_TLV_SYNCE_CLOCK_ID_LEN];
	unsigned char flag;
	unsigned char num_cascaded_eEEC;
	unsigned char num_cascaded_EEC;
	unsigned char reserved[ESMC_PDU_EXT_QL_TLV_RSVD_LEN];
};
typedef struct ext_ql_tlv T_esmc_ext_ql_tlv;

typedef struct {
	unsigned char originator_clock_id[MAX_CLK_ID_LEN];
	int mixed_EEC_eEEC;                                /* Equivalent to ITU-T G.8264 (08/2017) Amd. 1 (03/2018) bit 0 of extended QL TLV flag field */
	int partial_chain;                                 /* Equivalent to ITU-T G.8264 (08/2017) Amd. 1 (03/2018) bit 1 of extended QL TLV flag field */
	int num_cascaded_eEEC;
	int num_cascaded_EEC;
} T_port_ext_ql_tlv_data;


#define ESMC_PDU_QL_TLV_TYPE     0x1
#define ESMC_PDU_QL_TLV_LENGTH   {0x00, 0x04}

#define ESMC_PDU_EXT_QL_TLV_TYPE     0x2
#define ESMC_PDU_EXT_QL_TLV_LENGTH   {0x00, 0x14}

static const unsigned char g_ql_tlv_length[ESMC_PDU_QL_TLV_LENGTH_LEN] = ESMC_PDU_QL_TLV_LENGTH;
static const unsigned char g_ext_ql_tlv_length[ESMC_PDU_EXT_QL_TLV_LENGTH_LEN] = ESMC_PDU_EXT_QL_TLV_LENGTH;

/*
 * ESMC PDU
 *
 * struct ethhdr eth_hdr                                       14
 * unsigned char slow_proto_subtype                             1
 * unsigned char itu_oui[ESMC_PDU_ITU_OUI_LEN]                  3
 * unsigned char itu_subtype[ESMC_PDU_ITU_SUBTYPE_LEN]          2
 * unsigned char version_event_flag_reserved                    1
 * unsigned char reserved[ESMC_PDU_RSVD_LEN]                    3
 * T_esmc_ql_tlv ql_tlv                                         4
 * T_esmc_ext_ql_tlv ext_ql_tlv                                20
 * unsigned char padding[ESMC_PDU_PADDING_LEN]         +       12
 *                                                       --------
 *                                                       60 bytes
 * FCS (appended by hardware)                          +        4
 *						         --------
 *							 64 bytes
 * */
union __attribute__((packed)) esmc_pdu {
	struct __attribute__((packed)) {
		struct ethhdr eth_hdr;
		unsigned char slow_proto_subtype;
		unsigned char itu_oui[ESMC_PDU_ITU_OUI_LEN];
		unsigned char itu_subtype[ESMC_PDU_ITU_SUBTYPE_LEN];
		unsigned char version_event_flag_reserved;
		unsigned char reserved[ESMC_PDU_RSVD_LEN];
		T_esmc_ql_tlv ql_tlv;
		T_esmc_ext_ql_tlv ext_ql_tlv;
		/* NIC appends 4 bytes for
		 * FCS after padding;
		 * these bytes are
		 * consumed by hardware,
		 * so there is nothing to
		 * handle by software */
		unsigned char padding[ESMC_PDU_PADDING_LEN];
	};
	unsigned char buff[ESMC_PDU_LEN];
};
typedef union esmc_pdu T_esmc_pdu;
#endif
