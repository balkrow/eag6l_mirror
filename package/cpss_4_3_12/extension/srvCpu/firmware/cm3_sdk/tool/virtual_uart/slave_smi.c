#include <stdio.h>
#include <string.h>
#include "smi_drv.h"
#include "slave_smi.h"


#define SMI_WRITE_ADDRESS_MSB_REGISTER    (0x00)
#define SMI_WRITE_ADDRESS_LSB_REGISTER    (0x01)
#define SMI_WRITE_DATA_MSB_REGISTER       (0x08)
#define SMI_WRITE_DATA_LSB_REGISTER       (0x09)

#define SMI_READ_ADDRESS_MSB_REGISTER     (0x04)
#define SMI_READ_ADDRESS_LSB_REGISTER     (0x05)
#define SMI_READ_DATA_MSB_REGISTER        (0x06)
#define SMI_READ_DATA_LSB_REGISTER        (0x07)

#define SMI_STATUS_REGISTER               (0x1f)
#define SMI_STATUS_WRITE_DONE             (0x02)
#define SMI_STATUS_READ_READY             (0x01)

#define ARMADA_SMI_REG                    0xf212a200

#undef SMI_WAIT_FOR_STATUS_DONE
#define SMI_TIMEOUT_COUNTER  10000

#define CHK_RC(cmd) \
        rc = cmd; \
        if (rc != MV_OK) \
            return rc

#define SSMI_XBAR_PORT_REMAP(_port) (SSMI_XBAR_PORT_REMAP_REG + (_port)*4)
MV_U32  SSMI_XBAR_PORT_REMAP_REG;

MV_STATUS slaveSmiWrite32(
    IN  MV_U32  busId,
    IN  MV_U32  phyAddr,
    IN  MV_U32  regAddr,
    IN  MV_U32  data
);

/*******************************************************************************
* slaveSmiInit
*
* DESCRIPTION:
*       Slave SMI init
*
* INPUTS:
*       ssmiXbarPortRemapAddr - 32-bit SSMI XBAR Port Remap Address
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       MV_STATUS
*
* COMMENTS:
*
*******************************************************************************/
MV_STATUS slaveSmiInit(MV_U32 ssmiXbarPortRemapAddr)
{
    SSMI_XBAR_PORT_REMAP_REG = ssmiXbarPortRemapAddr;
    /* TODO: Read HOST CPU ID, in order to get HOST CPU SMI REG */
    return initSmi(ARMADA_SMI_REG);
}

/*******************************************************************************
* slaveSmiDestroy
*
* DESCRIPTION:
*       order deallocation of slave SMI
*
* INPUTS:
*       None.
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       MV_STATUS
*
* COMMENTS:
*
*******************************************************************************/
MV_STATUS slaveSmiDestroy(void)
{
    return destroySmi();
}

/*******************************************************************************
* slaveSmiWaitForStatus
*
* DESCRIPTION:
*       Slave SMI wait
*
* INPUTS:
*       waitFor     - bit to wait SMI_STATUS_READ_READY/SMI_STATUS_WRITE_DONE
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       MV_STATUS
*
* COMMENTS:
*
*******************************************************************************/
static MV_STATUS slaveSmiWaitForStatus(MV_U32 busId, MV_U32 phyAddr, MV_U32 waitFor)
{
#ifdef SMI_WAIT_FOR_STATUS_DONE
    MV_U32 stat;
    unsigned int timeOut;
    int rc;

    /* wait for write done */
    timeOut = SMI_TIMEOUT_COUNTER;
    do
    {
        rc = smiRead(busId, phyAddr, SMI_STATUS_REGISTER, &stat);
        if (rc != MV_OK)
            return rc;
        if (--timeOut < 1)
        {
            /* printk("bspSmiWaitForStatus timeout !\n"); */
            return MV_TIMEOUT;
        }
    } while ((stat & waitFor) == 0);
#endif
    return MV_OK;
}

/*******************************************************************************
* slaveSmiRead32
*
* DESCRIPTION:
*       Slave SMI read 32bit word (low level)
*
* INPUTS:
*       regAddr     - 32-bit register address
*
* OUTPUTS:
*       dataPtr     - pointer to store data read
*
* RETURNS:
*       MV_STATUS
*
* COMMENTS:
*
*******************************************************************************/
static MV_STATUS slaveSmiRead32(
    IN  MV_U32  busId,
    IN  MV_U32  phyAddr,
    IN  MV_U32  regAddr,
    OUT MV_U32 *dataPtr
)
{
    MV_STATUS rc;
    MV_U32  msb, lsb;

    msb = regAddr >> 16;
    lsb = regAddr & 0x0000ffff;
    CHK_RC(smiWrite(busId, phyAddr, SMI_READ_ADDRESS_MSB_REGISTER, &msb));
    CHK_RC(smiWrite(busId, phyAddr, SMI_READ_ADDRESS_LSB_REGISTER, &lsb));

    CHK_RC(slaveSmiWaitForStatus(busId, phyAddr, SMI_STATUS_READ_READY));

    CHK_RC(smiRead(busId, phyAddr, SMI_READ_DATA_MSB_REGISTER, &msb));
    CHK_RC(smiRead(busId, phyAddr, SMI_READ_DATA_LSB_REGISTER, &lsb));

    *dataPtr = (msb << 16) | lsb;

    return MV_OK;
}


/*******************************************************************************
* slaveSmiWrite32
*
* DESCRIPTION:
*       Slave SMI write 32bit word (low level)
*
* INPUTS:
*       regAddr     - 32-bit register address
*       dataPtr     - 32-bit data
*
* OUTPUTS:
*       None.
*
* RETURNS:
*       MV_STATUS
*
* COMMENTS:
*
*******************************************************************************/
MV_STATUS slaveSmiWrite32(
    IN  MV_U32  busId,
    IN  MV_U32  phyAddr,
    IN  MV_U32  regAddr,
    IN  MV_U32  data
)
{
    MV_STATUS rc;
    MV_U32  msb, lsb;

    msb = regAddr >> 16;
    lsb = regAddr & 0xffff;

    CHK_RC(smiWrite(busId, phyAddr, SMI_WRITE_ADDRESS_MSB_REGISTER, &msb));
    CHK_RC(smiWrite(busId, phyAddr, SMI_WRITE_ADDRESS_LSB_REGISTER, &lsb));

    msb = data >> 16;
    lsb = data & 0xffff;

    CHK_RC(smiWrite(busId, phyAddr, SMI_WRITE_DATA_MSB_REGISTER, &msb));
    CHK_RC(smiWrite(busId, phyAddr, SMI_WRITE_DATA_LSB_REGISTER, &lsb));

    rc = slaveSmiWaitForStatus(busId, phyAddr, SMI_STATUS_WRITE_DONE);

    return rc;
}

static MV_STATUS slaveSmiCompl(
    IN  MV_U32  busId,
    IN  MV_U32  phyAddr,
    IN  MV_U32  port,
    IN  MV_U32  regAddr,
    OUT MV_U32  *reg
)
{
    MV_U32 r;
    MV_STATUS rc;
    static uint32_t shadowPort;

    r = (regAddr >> 28) & 0x0f;
    if ( shadowPort != port) /* skip init if last transaction used the same port */
    {
        shadowPort = port;
        rc = slaveSmiWrite32(busId, phyAddr, SSMI_XBAR_PORT_REMAP(port), r|0x00010000);
        if (rc != MV_OK)
        return rc;
    }
    *reg = (regAddr & 0x0ffffffc) | ((port << 28) | 2);

    return MV_OK;
}


MV_STATUS slaveSmiRead(
    IN  MV_U32  busId,
    IN  MV_U32  phyAddr,
    IN  MV_U32  port,
    IN  MV_U32  regAddr,
    OUT MV_U32 *dataPtr)
{
    MV_STATUS rc;
    MV_U32    reg;

    rc = slaveSmiCompl(busId, phyAddr, port, regAddr, &reg);
    if (rc == MV_OK)
        rc = slaveSmiRead32(busId, phyAddr, reg, dataPtr);

    return rc;
}


MV_STATUS slaveSmiWrite(
    IN  MV_U32  busId,
    IN  MV_U32  phyAddr,
    IN  MV_U32  port,
    IN  MV_U32  regAddr,
    IN  MV_U32 *dataPtr
)
{
    MV_STATUS rc = MV_BAD_PARAM;
    MV_U32 reg, dataWr = *dataPtr;

    rc = slaveSmiCompl(busId, phyAddr, port, regAddr, &reg);
    if (rc == MV_OK)
        rc = slaveSmiWrite32(busId, phyAddr, reg, dataWr);

    return rc;
}
