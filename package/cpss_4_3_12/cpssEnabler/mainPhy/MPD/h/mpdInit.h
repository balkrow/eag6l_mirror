/* *****************************************************************************
Copyright (C) 2023, Marvell International Ltd. and its affiliates
If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.
*******************************************************************************/

#ifndef MPD_H_MPDINIT_H_
#define MPD_H_MPDINIT_H_
#include <mpdPrefix.h>

typedef struct {
	UINT_16		page;
	UINT_16		address;
	UINT_16		mask;
	UINT_16		value;
}PRV_MPD_REG_WRITE_INFO_STC;

#endif /* MPD_H_MPDINIT_H_ */
