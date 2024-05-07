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

********************************************************************************
* fileOps.c
*
* DESCRIPTION:
*
* RESTRICTIONS:
*
* DEPENDENCIES:
*
*******************************************************************************/

/* Includes */
#include "imgHdr.h"
#include "crc32.h"
#include <stdio.h>
#include <stdbool.h>



/*******************************************************************************
*
* 8 bit checksum
*
*******************************************************************************/
static uint8_t checksum8(uint8_t* start, uint32_t len, uint8_t csum)
{
	uint8_t  sum = csum;
	uint8_t* startp = start;

	do {
		sum += *startp;
		startp++;

	} while(--len);

	return (sum);
}

/*******************************************************************************
* validateHeader
*
* DESCRIPTION:
*       This routine validate image header at given address in memory.
*
* INPUT:
*       address - memory mapped address of buffer with image (or start of it)
*
* OUTPUT:
*       None
*
* RETURN:
*       true  - header is valid
*
*******************************************************************************/
int validateHeader(uint8_t *address, uint8_t coreId)
{
	struct bootromHeader *mainHeader = NULL;

	uint8_t           u;
	uint8_t           c8;

	/* Cast header struct over buffer pointer */
	mainHeader = (struct bootromHeader *)address;

	/* Sanity check */
	if ((mainHeader->magic != BOOTROM_MAGIC) ||
			(mainHeader->version > BOOTROM_HDR_VER) ||
			(mainHeader->hdrSize != sizeof(struct bootromHeader)) ||
			(mainHeader->coreId != coreId) ||
			(mainHeader->imageOffset > 0x1000000) ||
			(mainHeader->imageSize > 0x20000)) /* Image size limited to available SRAM */
		return false;

	/* Verify header checksum */
	u = mainHeader->hdrChksum;

	c8 = checksum8(address, sizeof(struct bootromHeader) - 1, 0);
	return (c8 == u ? true : false);

}

/*******************************************************************************
* check_crc - Check crc of buffer
*
* DESCRIPTION:
*       This routine Check crc for given buffer and size.
*
* INPUT:
*       uint8_t *buf - pointer to buffer.
*       uint32_t size
*
* OUTPUT:
*
* RETURN:
*       CRC32 result.
*
*******************************************************************************/
uint32_t check_crc(uint8_t *buf, uint32_t size)
{
	uint32_t calc_crc = 0;
	calc_crc = mvFtlCrc32Calc(buf, size, calc_crc);
	return calc_crc;
}


