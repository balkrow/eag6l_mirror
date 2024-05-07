/*******************************************************************************
*                Copyright 2017, MARVELL SEMICONDUCTOR, LTD.                   *
* THIS CODE CONTAINS CONFIDENTIAL INFORMATION OF MARVELL.                      *printf
* NO RIGHTS ARE GRANTED HEREIN UNDER ANY PATENT, MASK WORK RIGHT OR COPYRIGHT  *
* OF MARVELL OR ANY THIRD PARTY. MARVELL RESERVES THE RIGHT AT ITS SOLE        *
* DISCRETION TO REQUEST THAT THIS CODE BE IMMEDIATELY RETURNED TO MARVELL.     *
* THIS CODE IS PROVIDED "AS IS". MARVELL MAKES NO WARRANTIES, EXPRESSED,       *
* IMPLIED OR OTHERWISE, REGARDING ITS ACCURACY, COMPLETENESS OR PERFORMANCE.   *
*                                                                              *
* MARVELL COMPRISES MARVELL TECHNOLOGY GROUP LTD. (MTGL) AND ITS SUBSIDIARIES, *
* MARVELL INTERNATIONAL LTD. (MIL), MARVELL TECHNOLOGY, INC. (MTI), MARVELL    *
* SEMICONDUCTOR, INC. (MSI), MARVELL ASIA PTE LTD. (MAPL), MARVELL JAPAN K.K.  *
* (MJKK), MARVELL ISRAEL LTD. (MSIL).                                          *
********************************************************************************
* smi_drv.c
*
* DESCRIPTION:
*       SMI driver for Marvell platforms
*
* DEPENDENCIES:
*
* FILE REVISION NUMBER:
*       $Revision: 1 $
*
*******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <stdint.h>
#include "smi_drv.h"


#undef DISABLE_DBG
#ifdef DISABLE_DBG
#define SDK_DEBUG(str, ...)
#else
#define SDK_DEBUG(str, ...) printf(str, ## __VA_ARGS__)
#endif /*SDK_DEBUG_FLAG*/

#define SMI_MNG_BUSY            (1<<28)
#define SMI_MNG_RDVALID         (1<<27)
#define SMI_MNG_OPCODE          (1<<26)
#define SMI_MNG_OPCODE_READ     (1<<26)
#define SMI_MNG_OPCODE_WRITE    0
#define SMI_MNG_REG_ADDR(_a)    ((_a)<<21)
#define SMI_MNG_PHY_ADDR(_p)    ((_p)<<16)

MV_U32          SMI_REGISTER;
static MV_VOID  *smi_space_ptr = NULL;
static MV_U32   fd_smi;

static void smi_write_reg(int32_t offset, uint32_t value)
{
    *(volatile uint32_t *)(smi_space_ptr + offset) = value;
}

static uint32_t smi_read_reg(int32_t offset)
{
    return *(volatile uint32_t *)(smi_space_ptr + offset);
}

MV_STATUS initSmi(
    MV_U32 smiReg
)
{
    SMI_REGISTER = smiReg;
    /* mmap address of ssmi register */
    if ((fd_smi = open("/dev/mem", O_RDWR | O_SYNC) ) < 0)
        return MV_FAIL;
    smi_space_ptr = mmap(NULL, 0x100, PROT_READ | PROT_WRITE, MAP_SHARED, fd_smi, SMI_REGISTER & 0xfffff000);
    if (smi_space_ptr == MAP_FAILED)
    {
        printf("Could not mmap line %d\n",__LINE__);
        close(fd_smi);
        return MV_FAIL;
    }

    return MV_OK;
}

MV_STATUS destroySmi(void)
{
    MV_STATUS rc = MV_OK;
    if ( munmap(smi_space_ptr, 0x100) == -1){
        perror("munmap failed with error:");
        rc = MV_FAIL;
    }
    close(fd_smi);
    return rc;
}

MV_STATUS smiRead(
    MV_U32  busId,
    MV_U32  phyAddr,
    MV_U32  regAddr,
    OUT MV_U32 *dataPtr
)
{
    MV_U32  reg;

    if (busId > 3)
        return MV_BAD_PARAM;
    if ((phyAddr >= 32) || (regAddr > 32))
        return MV_BAD_PARAM;

    do /* read reg until Busy is 0 */
    {
        reg = smi_read_reg(SMI_REGISTER & 0x00000fff);
        /* TODO: timeout,delay??? */
    } while (reg & SMI_MNG_BUSY);
    /* write opcode, regAddr, phyAddr */
    reg = SMI_MNG_OPCODE_READ | SMI_MNG_REG_ADDR(regAddr) | SMI_MNG_PHY_ADDR(phyAddr);
    smi_write_reg(SMI_REGISTER & 0x00000fff, reg);
    do /* read reg until ReadValid is 1 */
    {
       reg = smi_read_reg(SMI_REGISTER & 0x00000fff);
        /* TODO: timeout,delay??? */
    } while ((reg & SMI_MNG_RDVALID) == 0);

    *dataPtr = reg & 0x0000ffff;

    return MV_OK;
}

MV_STATUS smiWrite(
    MV_U32  busId,
    MV_U32  phyAddr,
    MV_U32  regAddr,
    MV_U32 *dataPtr
)
{
    MV_U32  reg;

    if (busId > 3)
        return MV_BAD_PARAM;
    if ((phyAddr >= 32) || (regAddr > 32))
        return MV_BAD_PARAM;

    *dataPtr &= 0x0000ffff;

    /* now write */
    do /* read reg until Busy is 0 */
    {
        reg = smi_read_reg(SMI_REGISTER & 0x00000fff);
        /* TODO: timeout,delay??? */
    } while (reg & SMI_MNG_BUSY);

    /* write opcode, regAddr, phyAddr */
    reg = SMI_MNG_OPCODE_WRITE | SMI_MNG_REG_ADDR(regAddr) | SMI_MNG_PHY_ADDR(phyAddr) | *dataPtr;
    smi_write_reg(SMI_REGISTER & 0x00000fff, reg);

    return MV_OK;
}


