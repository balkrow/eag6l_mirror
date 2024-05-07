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

#include "sar_sw_lib.h"

/* help descriptions */
static char *h_DEVICE_ID = "device_id - Used by CPSS to identify the device flavor\n",
        *h_EPROM_LOAD_EN = "eprom_load_en - Enable load from I2C EPROM during device reset sequence (before boot)\n"
        "\t0 - Disable\n"
        "\t1 - Enable\n"
        "\tNOTE: When an EEPROM does not exist, set this value to 0\n",
        *h_PCIE_MODE = "pcie_mode - PCIE/OOB modes\n"
        "\t0 - Reserved for AC5\n"
        "\t1 - PCIe EP (HW init enabled)\n"
        "\t2 - PCIe EP no init (HW init disabled)\n"
        "\t3 - 0x3 - PCIe RC (HW init disabled)\n"
        "\tNOTE: Only MPP40 is SW controlled\n",
        *h_PLL_CONFIG = "pll_config - AC5 CORE PLL config:\n"
        "\t0 - 395mhz\n"
        "\t1 - 290mhz\n"
        "\t2 - 167mhz\n"
        "\t3 - Reserved (Bypass all PLLs for debug)\n",
        *h_BOOT_MODE = "boot_mode - on all options ROM is enabled and accessed first\n"
        "\t0 - boot from NF\n"
        "\t1 - boot from SPI NOR (this will be also for MI)\n"
        "\t2 - boot from SPI NAND (this will be also for MI)\n"
        "\t3 - boot from MMC\n"
        "\t4 - boot from UART0\n"
        "\t5 - Standby slave (CPU IDLE). Set SAR<PCIE_MODE_RC>=EP\n"
        "\t6 - UART debug prompt\n"
        "\t7 - CM3 boot (MI - might be needed for unmanaged only when A55 is efused out)\n",
        *h_DDR_PLL = "ddr_pll- clk speed:\n"
        "\t0 - 2400Mhz (PLL 600mhz)\n"
        "\t1 - 1600mhz (PLL 400mhz)\n",
        *h_CPU_PLL = "cpu_pll - AC5 CPU clk speed AND core power AVS setting:\n"
        "\t0 - 800Mhz, AVS=0.8V\n"
        "\t1 - 1200Mhz, AVS=0.8V (default)\n"
        "\t2 - 1400Mhz AVS=0.9V\n"
        "\t3 - 1000Mhz, 0.8V (to share with the PHYs)\n";

struct satr_info ac5_satr_info[] = {

// satr_reg = (satr_reg >> s_cmd_i->field_offs) & s_cmd_i->bit_mask;
    /* name             twsi_addr twsi_reg field_off bit_mask moreThen256    default     help                  make(pca9555)*/
    {"device_id"        ,   0x4c,   0,      0,      0x1f,        MV_FALSE,       0x0,    &h_DEVICE_ID,       MV_FALSE},
    {"eprom_load_en"    ,   0x4d,   0,      4,      0x1,         MV_FALSE,       0x0,    &h_EPROM_LOAD_EN,   MV_FALSE},
    {"pcie_mode"        ,   0x4d,   0,      3,      0x1,         MV_FALSE,       0x0,    &h_PCIE_MODE,       MV_FALSE}, /*one bit is RO, out of 2. */
    {"boot_mode"        ,   0x4d,   0,      0,      0x7,         MV_FALSE,       0x0,    &h_BOOT_MODE,       MV_FALSE},
    {"pll_config"       ,   0x4e,   0,      3,      0x3,         MV_FALSE,       0x0,    &h_PLL_CONFIG,      MV_FALSE},
    {"ddr_pll"          ,   0x4e,   0,      2,      0x1,         MV_FALSE,       0x0,    &h_DDR_PLL,         MV_FALSE},
    {"cpu_pll"          ,   0x4e,   0,      0,      0x3,         MV_FALSE,       0x1,    &h_CPU_PLL,         MV_FALSE},
    /* the "LAST entry should be always last - it is used for SatR max options calculation */
    {"LAST"             ,   0x0,    0,      0,      0x0,         MV_FALSE,       0x0,    NULL,               MV_FALSE},
};

/* PCA9560PW is used for all SatRs configurations (0x4c, 0x4d, 0x4e) */