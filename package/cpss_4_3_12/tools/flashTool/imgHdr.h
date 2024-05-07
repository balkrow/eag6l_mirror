#ifndef __imgHdr
#define __imgHdr

#include <stdint.h>

#define BOOTROM_MAGIC 0x49334d43 /*"CM3I"*/
#define BOOTROM_HDR_VER 0

struct bootromHeader {
/*  type        name                byte order */
	uint32_t      magic;              /*  0-3  */
	uint8_t       version;            /*   4   */
	uint8_t       hdrSize;            /*   5   */
	uint8_t       flags;              /*   6   */
	uint8_t       coreId;             /*   7   */
	uint32_t      imageOffset;        /*  8-11 */
	uint32_t      imageSize;          /* 12-15 */
	uint32_t      imageChksum;        /* 16-19 */
	uint32_t      rsvd1;              /* 20-23 */
	uint32_t      rsvd2;              /* 24-27 */
	uint16_t      rsvd3;              /* 28-29 */
	uint8_t       ext;                /*   30  */
	uint8_t       hdrChksum;          /*   31  */
};


extern int validateHeader(uint8_t *address, uint8_t coreId);
extern uint32_t check_crc(uint8_t *buf, uint32_t size);
#endif /* __imgHdr */
