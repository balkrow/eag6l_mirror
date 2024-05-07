/*******************************************************************************
Copyright (C) Marvell International Ltd. and its affiliates

This software file (the "File") is owned and distributed by Marvell
International Ltd. and/or its affiliates ("Marvell") under the following
alternative licensing terms.  Once you have made an election to distribute the
File under one of the following license alternatives, please (i) delete this
introductory statement regarding license alternatives, (ii) delete the two
license alternatives that you have not elected to use and (iii) preserve the
Marvell copyright notice above.

********************************************************************************
Marvell Commercial License Option

If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.

********************************************************************************
Marvell GPL License Option

If you received this File from Marvell, you may opt to use, redistribute and/or
modify this File in accordance with the terms and conditions of the General
Public License Version 2, June 1991 (the "GPL License"), a copy of which is
available along with the File in the license.txt file or by writing to the Free
Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 or
on the worldwide web at http://www.gnu.org/licenses/gpl.txt.

THE FILE IS DISTRIBUTED AS-IS, WITHOUT WARRANTY OF ANY KIND, AND THE IMPLIED
WARRANTIES OF MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE ARE EXPRESSLY
DISCLAIMED.  The GPL License provides additional details about this warranty
disclaimer.
********************************************************************************
Marvell BSD License Option

If you received this File from Marvell, you may opt to use, redistribute and/or
modify this File under the following licensing terms.
Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

    *   Redistributions of source code must retain the above copyright notice,
        this list of conditions and the following disclaimer.

    *   Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.

    *   Neither the name of Marvell nor the names of its contributors may be
        used to endorse or promote products derived from this software without
        specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*******************************************************************************/

/* Includes */
#include <stdio.h>
#include "fileOps.h"
#ifdef CONFIG_QSPI
#include "mv_qspi.h"
#endif
#include "string.h"
#include "crc32.h"

extern void remap_window(uint32_t offset);

#define IMAGE_MAGIC 0x74696e69 /*"init"*/
#define DEBUG_MAGIC 0x67626564 /*"debg"*/
#define CRC_MAGIC   0x78787878
#define IMAGE_HDR_VER 0
#define MAX_FILES 8
#define IMAGE_OFFSET 0x40000

enum IMAGE_TYPE
{
	IMAGE_TYPE_BOOT,       /* boot loader image */
	IMAGE_TYPE_IMAGE,      /* Image binary */
	IMAGE_TYPE_EXTENDED    /* boot and image combined */
};



/*******************************************************************************
 * file_image_header_STC:
 *
 * Description: Header of packed image
 *
 * Fields:
 *         magic
 *         hdr_ver      - version of file header
 *         hdr_checksum - crc32 of header
 *         toatl_hdr_size - size image header + all file headers
 *         type         - enumerator for type of file
 *         img_size     - size of full image in bytes
 *         img_crc      - crc32 of image
 *         version      - 32 chars of version string
 *         date_time    - 32 chars of date / time string
 *         num_files    - amount of files in image
 *
 *******************************************************************************/

struct file_image_header_STC
{
	uint32_t    magic;
	uint16_t    hdr_size;
	uint16_t    hdr_ver;
	uint32_t    hdr_chksum;
	uint32_t    total_hdr_size;
	uint32_t    img_size;
	uint32_t    img_crc;
	uint8_t     version[32];
	uint8_t     date_time[32];
	uint32_t    num_files;
};

/*******************************************************************************
 * file_params_STC:
 *
 * Description: Header for each file in the image
 *
 * Fields:
 *         hdr_size     - size of file header in bytes
 *         hdr_ver      - version of file header
 *         hdr_checksum - 8bit checksum of header
 *         file_size    - size of file
 *         type         - enumerator for type of file
 *         name         - file name
 *         exec_cpy_offset (relevant for MI)
 *         exec_offset     (relevant for MI)
 *         file_offset  - offset of file from image start
 *         useBmp       - file usage bitmap (relevant for MI)
 *
 *******************************************************************************/

struct file_header_STC
{
	uint16_t   hdr_size;
	uint16_t   hdr_ver;
	uint32_t   file_chksum;
	uint32_t   file_size;
	uint8_t    name[16];
	uint32_t   type;
	uint32_t   exec_cpy_offset;
	uint32_t   exec_offset;
	uint32_t   file_offset;
	uint32_t   useBmp;
};

/*******************************************************************************
 * file_params_STC:
 *
 * Description: entry of file in file list shadow
 *
 * Fields:
 *         file_size    - size of file
 *         file_offset  - offset of file from image start
 *         type         - enumerator for type of file
 *         name         - file name
 *
 *******************************************************************************/

/*******************************************************************************
* fileOpsInit -
*
* Description: initialize file system structures by reading headers from flash
*
* Inputs:
*       offset - offset of image in flash
*
* Return:
*       int - 0
*                   -1
*******************************************************************************/
int validate_image(uint8_t *buffer)
{
	uint32_t chksum, calc_crc;
	struct file_image_header_STC *image_hdr;
	uint32_t img_crc = 0;

	image_hdr = (struct file_image_header_STC *)buffer;

	/* Check sanity */
	if (image_hdr->magic != IMAGE_MAGIC)
	{
		printf("Image validation failed - wrong magic\n");
		return -1;
	}

	if ((__builtin_bswap16(image_hdr->hdr_size) > 2 * sizeof(struct file_image_header_STC)) ||
			(__builtin_bswap32(image_hdr->img_size) > 0x400000)) {
		printf("Image validation failed - values in header!\n");
		return -1;
	}
	chksum = __builtin_bswap32(image_hdr->hdr_chksum);
	image_hdr->hdr_chksum = CRC_MAGIC;
	calc_crc = mvFtlCrc32Calc((const unsigned char *)image_hdr, __builtin_bswap32(image_hdr->total_hdr_size), 0);
	if (calc_crc != chksum) {
		printf("Image header cheksum error!\n");
		return -1;
	}
	image_hdr->version[31] = 0;
	image_hdr->date_time[31] = 0;

	printf("  Image header: version %s %s ",
			(char *)image_hdr->version, (char *)image_hdr->date_time);
	printf("%d files, size 0x%x\n\r", __builtin_bswap32(image_hdr->num_files), __builtin_bswap32(image_hdr->img_size));

	/* Validate correct image crc */
	img_crc = __builtin_bswap32(image_hdr->img_crc);
	image_hdr->img_crc = CRC_MAGIC;
	calc_crc = mvFtlCrc32Calc(buffer, __builtin_bswap32(image_hdr->img_size), 0);
	if (calc_crc != img_crc) {
		printf("Image cheksum error!\n");
		return -1;
	}

	image_hdr->img_crc = __builtin_bswap32(img_crc);
	image_hdr->hdr_chksum = __builtin_bswap32(chksum);

	return 0;
}




