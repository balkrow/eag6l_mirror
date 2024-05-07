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
/**
********************************************************************************
* @file srvCpuMdio.c
*
* @brief MDIO driver for SMI and XSMI interfaces
* 
*
* @version   1
********************************************************************************
*/

#include <global.h>
#include "srvCpuMdio.h"
#include <srvCpuServices.h>

/* n is 0-1 represents SMI interface */
#define SMI_CTRL_BUSY_BIT       (1 << 28)
#define SMI_CTRL_RD_VALID_BIT   (1 << 27)
#define SMI_CTRL_RD_OPCOD_BIT   (1 << 26)
#define XSMI_MNG_REG(n)         srvCpuGetMgBase(n) + 0x30000
#define XSMI_ADDR_REG(n)        srvCpuGetMgBase(n) + 0x30008
#define XSMI_CFG_REG(n)         srvCpuGetMgBase(n) + 0x3000c
#define XSMI_CFG_BUSY_BIT       (1 << 30)
#define XSMI_CFG_RD_VALID_BIT   (1 << 29)
#define XSMI_CFG_OPCOD_OFFSET   26
#define XSMI_CFG_OPCOD_READ     7
#define XSMI_CFG_OPCOD_WRITE    5

static MV_U32 prvSmiCtrlReg(MV_U8 interface)
{
    return (devices_details[devIndex].deviceType == SRV_CPU_DEVICE_TYPE_AC5_E) ?
                                    ((unsigned int)0x07004054 + 0x02000000 * interface) : (0x80580000 + 0x10000 * interface);
}

static MV_U32 prvSmiMiscConfReg(MV_U8 interface)
{
    return (devices_details[devIndex].deviceType == SRV_CPU_DEVICE_TYPE_AC5_E) ?
                                    ((unsigned int)0x07004200 + 0x02000000 * interface) : (0x80580004 + 0x10000 * interface);
}

/**
* @internal srvCpuSmiInit function
* @endinternal
*
* @brief  Initialize SMI interface.
*
* @param MV_U8 ifNum - interface number (0-1)
* @param MV_U8 speed - One of 7 possible speeds (0-6), 6 is fastest
*              Speed	Frequency:
*              6        Accelerate the MDC - Core clock divided by 8
*              5        Core clock divided by 16
*              4        Core clock divided by 32
*              3        Core clock divided by 64
*              2        Core clock divided by 128
*              1        Core clock divided by 256
*              0        Core clock divided by 512
*
* @retval MV_STATUS      MV_OK  - For successful operation.
* @retval MV_OUT_OF_RANGE       - The value is out of range.
*
*/
MV_STATUS srvCpuSmiInit(MV_U8 ifNum, MV_U8 speed)
{
    MV_U32 value = 0; /* SMI accelerate and SMI FastMDC both off */

    if (ifNum > 1 || speed > 6)
        return MV_OUT_OF_RANGE;

    if (speed >= 1 && speed <= 5)
        value = 0x2 | ((5 - speed) << 2); /* SMI accelerate off, FastMDC on, Fast MDC Division Selector set to value */

    if (speed == 6) /* Accelerate the MDC - Core clock divided by 8 */
        value = 1;

    if (devices_details[devIndex].deviceType == SRV_CPU_DEVICE_TYPE_AC5_E)
    {
        srvCpuRegWrite(prvSmiMiscConfReg(ifNum), 0x002B0000);
        srvCpuRegWrite(prvSmiCtrlReg(ifNum), 0x00009140);
    }

    srvCpuRegWrite(prvSmiMiscConfReg(ifNum), (srvCpuRegRead(prvSmiMiscConfReg(ifNum)) & 0xffffffe0) | value);

    return MV_OK;
}

/**
* @internal srvCpuSmiRead function
* @endinternal
*
* @brief  Perform register read through SMI interface.
*
* @param IN  MV_U8   ifNum     - interface number (0-1)
* @param IN  MV_U8   device    - endpoint device num (0-31)
* @param IN  MV_U8   reg       - register offset  (0-31)
* @param OUT MV_U16* readValue - value read from device
*
* @retval MV_STATUS      MV_OK  - For successful operation.
* @retval MV_OUT_OF_RANGE       - The value is out of range.
*
*/
MV_STATUS srvCpuSmiRead(IN MV_U8 ifNum, IN MV_U8 device, IN MV_U8 reg, OUT MV_U16* readValue)
{
    MV_U32 value = 0;

    if (ifNum > 1 || device > 31 || reg > 31)
    {
        return MV_OUT_OF_RANGE;
    }

    /* Wait for the interface to be available */
    do
        value = srvCpuRegRead(prvSmiCtrlReg(ifNum));
    while (value & SMI_CTRL_BUSY_BIT);


    /* Construct the read command */
    value = (device << 16) | (reg << 21) | SMI_CTRL_RD_OPCOD_BIT;
    srvCpuRegWrite(prvSmiCtrlReg(ifNum), value);

    /* Wait until transaction is finished */
    do
        value = srvCpuRegRead(prvSmiCtrlReg(ifNum));
    while (!(value & SMI_CTRL_RD_VALID_BIT));

    *readValue = (MV_U16)(value & 0xffff);
    return  MV_OK;
}

/**
* @internal srvCpuSmiWrite function
* @endinternal
*
* @brief  Perform register read through SMI interface.
*
* @param MV_U8  ifNum - interface number (0-1)
* @param MV_U8  device   - endpoint device num (0-31)
* @param MV_U8  reg   - register offset  (0-31)
* @param MV_U16 val   - value to write  (0-0xffff)
*
* @retval MV_STATUS      MV_OK  - For successful operation.
* @retval MV_OUT_OF_RANGE       - The value is out of range.
*
*/
MV_STATUS srvCpuSmiWrite(MV_U8 ifNum, MV_U8 device, MV_U8 reg, MV_U16 val)
{
    MV_U32 value = 0; /* SMI accelerate and SMI FastMDC both off */

    if (ifNum > 1 || device > 31 || reg > 31)
        return MV_OUT_OF_RANGE;

    /* Wait for the interface to be available */
    do
        value = srvCpuRegRead(prvSmiCtrlReg(ifNum));
    while (value & SMI_CTRL_BUSY_BIT);


    /* Construct the write command */
    value = (device << 16) | (reg << 21) | val;
    srvCpuRegWrite(prvSmiCtrlReg(ifNum), value);

    return MV_OK;
}

/**
* @internal srvCpuXsmiInit function
* @endinternal
*
* @brief  Initialize XSMI interface.
*
* @param MV_U8 ifNum - interface number (0-1)
* @param MV_U8 speed - One of 4 possible speeds (0-3), 3 is fastest
*
* @retval MV_STATUS      MV_OK  - For successful operation.
* @retval MV_OUT_OF_RANGE       - The value is out of range.
*
*/
MV_STATUS srvCpuXsmiInit(MV_U8 ifNum, MV_U8 speed)
{
    if (ifNum > 2 || speed > 3)
        return MV_OUT_OF_RANGE;

    srvCpuRegWrite(XSMI_CFG_REG(ifNum), (srvCpuRegRead(XSMI_CFG_REG(ifNum)) & 0xfffffffc) | speed);

    return MV_OK;
}

/**
* @internal srvCpuXsmiRead function
* @endinternal
*
* @brief  Perform register read through XSMI interface.
*
* @param IN   MV_U8   ifNum     - interface number (0-1)
* @param IN   MV_U8   phy       - endpoint device num (0-31)
* @param IN   MV_U8   device    - endpoint device num (0-31)
* @param IN   MV_U16  reg       - register offset  (0-0xffff)
* @param OUT  MV_U16* readValue - value read from device
*
* @retval MV_STATUS      MV_OK  - For successful operation.
* @retval MV_OUT_OF_RANGE       - The value is out of range.
*
*/
MV_STATUS srvCpuXsmiRead( IN MV_U8 ifNum, IN MV_U8 phy, IN MV_U8 device, IN MV_U16 reg, OUT MV_U16* readValue)
{
    MV_U32 value = 0;

    if (ifNum > 2 || phy > 31 || device > 31)
        return MV_OUT_OF_RANGE;

    /* Wait for the interface to be available */
    do
        value = srvCpuRegRead(XSMI_MNG_REG(ifNum));
    while (value & XSMI_CFG_BUSY_BIT);

    /* Write register offset to address reg */
    srvCpuRegWrite(XSMI_ADDR_REG(ifNum), (MV_U32)reg);

    /* Construct the read command */
    value = (phy << 16) | (device << 21) | (XSMI_CFG_OPCOD_READ << XSMI_CFG_OPCOD_OFFSET);
    srvCpuRegWrite(XSMI_MNG_REG(ifNum), value);

    /* Wait until transaction is finished */
    do
        value = srvCpuRegRead(XSMI_MNG_REG(ifNum));
    while (!(value & XSMI_CFG_RD_VALID_BIT));

     *readValue = (MV_U16)(value & 0xffff);
     return MV_OK;
}

/**
* @internal srvCpuXsmiWrite function
* @endinternal
*
* @brief  Perform register write through XSMI interface.
*
* @param MV_U8  ifNum - interface number (0-1)
* @param MV_U8  phy   - endpoint device num (0-31)
* @param MV_U8  device   - endpoint device num (0-31)
* @param MV_U16 reg   - register offset  (0-0xffff)
* @param MV_U16 val   - value to write  (0-0xffff)
*
* @retval MV_STATUS   MV_OK  - For successful operation.
* @retval MV_OUT_OF_RANGE       - The value is out of range.
*
*/
MV_STATUS srvCpuXsmiWrite(MV_U8 ifNum, MV_U8 phy, MV_U8 device, MV_U16 reg, MV_U16 val)
{
    MV_U32 value = 0;

    if (ifNum > 2 || phy > 31 || device > 31)
    {
        return MV_OUT_OF_RANGE;
    }

    /* Wait for the interface to be available */
    do
        value = srvCpuRegRead(XSMI_MNG_REG(ifNum));
    while (value & XSMI_CFG_BUSY_BIT);

    /* Write register offset to address reg */
    srvCpuRegWrite(XSMI_ADDR_REG(ifNum), (MV_U32)reg);

    /* Construct the read command */
    value = (phy << 16) | (device << 21) | (XSMI_CFG_OPCOD_WRITE << XSMI_CFG_OPCOD_OFFSET | val);
    srvCpuRegWrite(XSMI_MNG_REG(ifNum), value);

    return MV_OK;
}


