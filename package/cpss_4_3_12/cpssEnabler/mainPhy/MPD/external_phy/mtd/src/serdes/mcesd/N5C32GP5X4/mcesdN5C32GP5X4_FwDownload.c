/*******************************************************************************
Copyright (C) 2023, Marvell International Ltd. and its affiliates
If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.
*******************************************************************************/

/********************************************************************
This file contains functions prototypes and global defines/data for
higher-level functions to download MCU firmware to Marvell
5FFP_COMPHY_32G_PIPE5_X4_4PLL
********************************************************************/
#include <stdio.h>
#include <stdlib.h>

#include "mcesdTop.h"
#include "mcesdApiTypes.h"
#include "mcesdUtils.h"
#include "mcesdN5C32GP5X4_Defs.h"
#include "mcesdN5C32GP5X4_RegRW.h"
#include "mcesdN5C32GP5X4_FwDownload.h"

#ifdef N5C32GP5X4

MCESD_STATUS API_N5C32GP5X4_DownloadFirmware
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U32 *fwCodePtr,
    IN MCESD_U32 fwCodeSizeDW,
    OUT MCESD_U16 *errCode
)
{
    *errCode = MCESD_IO_ERROR;

    /* Write to MCU program memory */
    MCESD_ATTEMPT(API_N5C32GP5X4_UpdateRamCode(devPtr, 255 /* ignored */, fwCodePtr, fwCodeSizeDW, N5C32GP5X4_FW_MAX_SIZE, N5C32GP5X4_FW_BASE_ADDR, errCode));

    *errCode = 0;
    return MCESD_OK;
}

MCESD_STATUS API_N5C32GP5X4_UpdateRamCode
(
    IN MCESD_DEV_PTR devPtr,
    IN MCESD_U8 lane,
    IN MCESD_U32 code[],
    IN MCESD_U32 codeSize,
    IN MCESD_U32 memSize,
    IN MCESD_U32 address,
    OUT MCESD_U16 *errCode
)
{
    MCESD_U32 index;

    /*******************************************************************************
       Check if the code about to be downloaded can fit into the device's memory
    *******************************************************************************/
    if (codeSize > memSize)
    {
        *errCode = MCESD_IMAGE_TOO_LARGE_TO_DOWNLOAD;
        return MCESD_FAIL;
    }

    /* Write to program memory incrementally */
    for (index = 0; index < codeSize; index++)
    {
        MCESD_ATTEMPT(API_N5C32GP5X4_WriteReg(devPtr, lane, address, code[index]));
        address += 4;
    }

    *errCode = 0;
    return MCESD_OK;
}

#endif /* N5C32GP5X4 */