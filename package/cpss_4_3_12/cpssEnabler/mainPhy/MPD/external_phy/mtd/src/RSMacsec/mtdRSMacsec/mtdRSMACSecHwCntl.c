/*******************************************************************************
Copyright (C) 2014 - 2022, Marvell International Ltd. and its affiliates
If you received this File from Marvell and you have entered into a commercial
license agreement (a "Commercial License") with Marvell, the File is licensed
to you under the terms of the applicable Commercial License.
*******************************************************************************/

/********************************************************************
This file contains functions for initializing the driver and setting 
up the user-provide MDIO access functions for the Marvell CUX3610/CUE2610 
MACSec enable Ethernet PHYs.
********************************************************************/
#include "mtdFeatures.h"
#include "mtdApiTypes.h"
#include "mtdHwCntl.h"
#include "mtdRSMACSecTypes.h"

/*******************************************************************************
*
********************************************************************************/
MTD_STATUS mtdRSMACSecRegRead64
(
    IN MTD_DEV_PTR pDev,    
    IN MTD_U16  regAddr,
    OUT MTD_U64 volatile *regVal
)
{
    MTD_U16 data = 0;
    MTD_U16 mdioPort;

    if (regAddr % 2 != 0)
    {
        MTD_DBG_ERROR("mtdRSMACSecRegRead64: reg address:0x%X MACSec address must be an even number.\n", regAddr);
        return MTD_FAIL;
    }

    mdioPort = pDev->rsMACSecCntl.rsMACSecPort;

    mtdHwXmdioRead(pDev, mdioPort, MTD_MACSEC_UNIT, (regAddr/2), &data);

    mtdHwXmdioRead(pDev, mdioPort, MTD_MACSEC_UNIT, 0x97FD, &data);
    if (data != (regAddr/2))
    {
        MTD_DBG_ERROR("mtdRSMACSecRegRead32: Indirect read FAIL. 0x97FD address:0x%X data:0x%X.\n", (regAddr/2), data);
        return MTD_FAIL;
    }

    mtdHwXmdioRead(pDev, mdioPort, MTD_MACSEC_UNIT, MTD_MACSEC_READ_LOW, &data);
    *regVal = data;
    mtdHwXmdioRead(pDev, mdioPort, MTD_MACSEC_UNIT, MTD_MACSEC_READ_HIGH, &data);
    *regVal |= (MTD_U64) data << 16;

    /* 2nd set of 32-bit read */
    mtdHwXmdioRead(pDev, mdioPort, MTD_MACSEC_UNIT, (regAddr/2)+2, &data);

    mtdHwXmdioRead(pDev, mdioPort, MTD_MACSEC_UNIT, 0x97FD, &data);
    if (data != (regAddr/2)+2)
    {
        MTD_DBG_ERROR("mtdRSMACSecRegRead64: Indirect read FAIL. 0x97FD address:0x%X data:0x%X.\n", (regAddr/2)+2, data);
        return MTD_FAIL;
    }

    mtdHwXmdioRead(pDev, mdioPort, MTD_MACSEC_UNIT, MTD_MACSEC_READ_LOW, &data);
    *regVal |= (MTD_U64) data << 32;
    mtdHwXmdioRead(pDev, mdioPort, MTD_MACSEC_UNIT, MTD_MACSEC_READ_HIGH, &data);
    *regVal |= (MTD_U64) data << 48;

    return MTD_OK;
}

/*******************************************************************************
*
********************************************************************************/
MTD_STATUS mtdRSMACSecRegWrite64
(
    IN MTD_DEV_PTR pDev,    
    IN MTD_U16  regAddr,
    IN MTD_U64  data
)
{
    MTD_U16  mdioPort;

    if (regAddr % 2 != 0)
    {
        MTD_DBG_ERROR("mtdRSMACSecRegWrite64: reg address:0x%X MACSec address must be an even number.\n", regAddr);
        return MTD_FAIL;
    }

    mdioPort = pDev->rsMACSecCntl.rsMACSecPort;

    mtdHwXmdioWrite(pDev, mdioPort, MTD_MACSEC_UNIT, (regAddr/2), (data & 0xFFFF));
    mtdHwXmdioWrite(pDev, mdioPort, MTD_MACSEC_UNIT, (regAddr/2)+1, ((data >> 16) & 0xFFFF));
    mtdHwXmdioWrite(pDev, mdioPort, MTD_MACSEC_UNIT, (regAddr/2)+2, ((data >> 32) & 0xFFFF));
    mtdHwXmdioWrite(pDev, mdioPort, MTD_MACSEC_UNIT, (regAddr/2)+3, ((data >> 48) & 0xFFFF));

    return MTD_OK;
}

/*******************************************************************************
*
********************************************************************************/
MTD_STATUS mtdRSMACSecRegRead32
(
    IN MTD_DEV_PTR pDev,    
    IN MTD_U16  regAddr,
    OUT MTD_U32 *regVal
)
{
    MTD_U16 data = 0;
    MTD_U16 mdioPort;

    if (regAddr % 2 != 0)
    {
        MTD_DBG_ERROR("mtdRSMACSecRegRead32: reg address:0x%X MACSec address must be an even number.\n", regAddr);
        return MTD_FAIL;
    }

    mdioPort = pDev->rsMACSecCntl.rsMACSecPort;

    mtdHwXmdioRead(pDev, mdioPort, MTD_MACSEC_UNIT, (regAddr/2), &data);

    mtdHwXmdioRead(pDev, mdioPort, MTD_MACSEC_UNIT, 0x97FD, &data);
    if (data != (regAddr / 2))
    {
        MTD_DBG_ERROR("mtdRSMACSecRegRead32: Indirect read FAIL. 0x97FD address:0x%X data:0x%X.\n", (regAddr/2), data);
        return MTD_FAIL;
    }

    mtdHwXmdioRead(pDev, mdioPort, MTD_MACSEC_UNIT, MTD_MACSEC_READ_LOW, &data);
    *regVal = data;
    mtdHwXmdioRead(pDev, mdioPort, MTD_MACSEC_UNIT, MTD_MACSEC_READ_HIGH, &data);
    *regVal |= (MTD_U32) data << 16;

    return MTD_OK;
}

/*******************************************************************************
*
********************************************************************************/
MTD_STATUS mtdRSMACSecRegWrite32
(
    IN MTD_DEV_PTR pDev,    
    IN MTD_U16  regAddr,
    IN MTD_U32  data
)
{
    MTD_U16  mdioPort;

    if (regAddr % 2 != 0)
    {
        MTD_DBG_ERROR("mtdRSMACSecRegWrite32: reg address:0x%X MACSec address must be an even number.\n", regAddr);
        return MTD_FAIL;
    }

    mdioPort = pDev->rsMACSecCntl.rsMACSecPort;

    mtdHwXmdioWrite(pDev, mdioPort, MTD_MACSEC_UNIT, (regAddr/2), (data & 0xFFFF));
    mtdHwXmdioWrite(pDev, mdioPort, MTD_MACSEC_UNIT, (regAddr/2)+1, ((data >> 16) & 0xFFFF));

#if 0
    MTD_DBG_INFO("mtdRSMACSecRegWrite32 reg:0x%X val:0x%X\n", (regAddr/2), data);
#endif

    return MTD_OK;
}


/*******************************************************************************
*
********************************************************************************/
MTD_STATUS mtdIndirectRegRead32
(
    IN MTD_DEV_PTR pDev,
    IN MTD_U16 mdioPort,
    IN MTD_U16  regAddr,
    OUT MTD_U32 volatile *regVal
)
{
    MTD_U16 data = 0;

    mtdHwXmdioRead(pDev, mdioPort, MTD_MACSEC_UNIT, regAddr, &data);

    mtdHwXmdioRead(pDev, mdioPort, MTD_MACSEC_UNIT, 0x97FD, &data);
    if (data != regAddr)
    {
        MTD_DBG_ERROR("mtdIndirectRegRead32: Indirect read FAIL. 0x97FD address:0x%X data:0x%X.\n", regAddr, data);
        return MTD_FAIL;
    }

    mtdHwXmdioRead(pDev, mdioPort, MTD_MACSEC_UNIT, MTD_MACSEC_READ_LOW, &data);
    *regVal = data;
    mtdHwXmdioRead(pDev, mdioPort, MTD_MACSEC_UNIT, MTD_MACSEC_READ_HIGH, &data);
    *regVal |= (MTD_U64)data << 16;

    return MTD_OK;
}

/*******************************************************************************
*
********************************************************************************/
MTD_STATUS mtdIndirectRegRead64
(
    IN MTD_DEV_PTR pDev,
    IN MTD_U16 mdioPort,
    IN MTD_U16  regAddr,
    OUT MTD_U64 volatile *regVal
)
{
    MTD_U16 data = 0;

    mtdHwXmdioRead(pDev, mdioPort, MTD_MACSEC_UNIT, regAddr, &data);

    mtdHwXmdioRead(pDev, mdioPort, MTD_MACSEC_UNIT, 0x97FD, &data);
    if (data != regAddr)
    {
        MTD_DBG_ERROR("mtdIndirectRegRead64: Indirect read FAIL. 0x97FD address:0x%X data:0x%X.\n", regAddr, data);
        return MTD_FAIL;
    }

    mtdHwXmdioRead(pDev, mdioPort, MTD_MACSEC_UNIT, MTD_MACSEC_READ_LOW, &data);
    *regVal = data;
    mtdHwXmdioRead(pDev, mdioPort, MTD_MACSEC_UNIT, MTD_MACSEC_READ_HIGH, &data);
    *regVal |= (MTD_U64)data << 16;

    /* 2nd set of 32-bit read */
    mtdHwXmdioRead(pDev, mdioPort, MTD_MACSEC_UNIT, regAddr + 2, &data);

    mtdHwXmdioRead(pDev, mdioPort, MTD_MACSEC_UNIT, 0x97FD, &data);
    if (data != regAddr + 2)
    {
        MTD_DBG_ERROR("mtdRSMACSecRegRead64: Indirect read FAIL. 0x97FD address:0x%X data:0x%X.\n", regAddr + 2, data);
        return MTD_FAIL;
    }

    mtdHwXmdioRead(pDev, mdioPort, MTD_MACSEC_UNIT, MTD_MACSEC_READ_LOW, &data);
    *regVal |= (MTD_U64)data << 32;
    mtdHwXmdioRead(pDev, mdioPort, MTD_MACSEC_UNIT, MTD_MACSEC_READ_HIGH, &data);
    *regVal |= (MTD_U64)data << 48;

    return MTD_OK;
}

/*******************************************************************************
*
********************************************************************************/
MTD_STATUS mtdRegRead32
(
    IN MTD_DEV_PTR pDev,    
    IN MTD_U16 mdioPort,
    IN MTD_U16 dev, 
    IN MTD_U16 regAddr,
    OUT MTD_U32 *regVal
)
{
    MTD_U16 data = 0;

    mtdHwXmdioRead(pDev, mdioPort, dev, regAddr, &data);
    *regVal = data;
    mtdHwXmdioRead(pDev, mdioPort, dev, regAddr+1, &data);
    *regVal |= (MTD_U32) data << 16;

    return MTD_OK;
}

/*******************************************************************************
*
********************************************************************************/
MTD_STATUS mtdRegWrite32
(
    IN MTD_DEV_PTR pDev,    
    IN MTD_U16 mdioPort,
    IN MTD_U16 dev, 
    IN MTD_U16 regAddr,
    IN MTD_U32 regVal
)
{
    mtdHwXmdioWrite(pDev, mdioPort, dev, regAddr, regVal & 0xFFFF);
    mtdHwXmdioWrite(pDev, mdioPort, dev, regAddr + 1, (regVal >> 16) & 0xFFFF);

#if 0
    MTD_DBG_INFO("mtdRegWrite32 reg:0x%X val:0x%X\n", regAddr, regVal);
#endif

    return MTD_OK;
}

/*******************************************************************************
*
********************************************************************************/
MTD_STATUS mtdRSMACSecInit
(
    IN MTD_DEV_PTR pDev
)
{
    if (!MTD_IS_X3610_DEVICE(pDev))
    {
        MTD_DBG_ERROR("mtdRSMACSecInit: Failed. Only for X3610\n");
        return MTD_FAIL;
    }

    pDev->rsMACSecCntl.rmsDev.rmsHwReadFn = (RmsHwReadFn_t)&mtdRSMACSecRegRead32;
    pDev->rsMACSecCntl.rmsDev.rmsHwWriteFn = (RmsHwWriteFn_t)&mtdRSMACSecRegWrite32;
    pDev->rsMACSecCntl.rmsDev.userData_p = pDev;

    return MTD_OK;
}

/*******************************************************************************
*
*******************************************************************************/
MTD_STATUS mtdRSMACSecUnload
(
    IN MTD_DEV_PTR pDev
)
{
    if (!MTD_IS_X3610_DEVICE(pDev))
    {
        MTD_DBG_ERROR("mtdRSMACSecUnload: Failed. Only for X3610\n");
        return MTD_FAIL;
    }

    pDev->rsMACSecCntl.rmsDev.rmsHwReadFn = NULL;
    pDev->rsMACSecCntl.rmsDev.rmsHwWriteFn = NULL;
    pDev->rsMACSecCntl.rmsDev.userData_p = NULL;

    return MTD_OK;
}

