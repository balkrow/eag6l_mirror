// SPDX-License-Identifier: GPL-2.0
/* Copyright 2016-2019 NXP Semiconductors
 */
#include <common.h>
#include <asm/arch-ls102xa/ls102xa_soc.h>
#include <asm/arch/ls102xa_devdis.h>
#include <asm/arch/immap_ls102xa.h>
#include <asm/arch/ls102xa_soc.h>
#include <asm/arch/fsl_serdes.h>
#include "../common/sleep.h"
#include <fsl_validate.h>
#include <fsl_immap.h>
#include <fsl_csu.h>
#include <netdev.h>
#include <spl.h>
#ifdef CONFIG_U_QE
#include <fsl_qe.h>
#endif
#include <asm/tzasc380.h>
#if 1/*[#92] uboot에서 FPGA bank 보정작업, balkrow, 2024-08-14*/
#include <mapmem.h>
#include <errno.h>
#include <stdlib.h>
#define BANK_CFG_SUCCESS 0x9
#define BANK_CFG_FAIL 0x8
#define BANK_MAX_TRY 0x2
#endif

DECLARE_GLOBAL_DATA_PTR;

static void ddrmc_init(void)
{
#if (!defined(CONFIG_SPL) || defined(CONFIG_SPL_BUILD))
#warning "---------- NOT CONFIG_SPL or CONFIG_SPL_BUILD -----------"
	struct ccsr_ddr *ddr = (struct ccsr_ddr *)CONFIG_SYS_FSL_DDR_ADDR;
	u32 temp_sdram_cfg, tmp;

	out_be32(&ddr->sdram_cfg, DDR_SDRAM_CFG);

	out_be32(&ddr->cs0_bnds, DDR_CS0_BNDS);
	out_be32(&ddr->cs0_config, DDR_CS0_CONFIG);

	out_be32(&ddr->timing_cfg_0, DDR_TIMING_CFG_0);
	out_be32(&ddr->timing_cfg_1, DDR_TIMING_CFG_1);
	out_be32(&ddr->timing_cfg_2, DDR_TIMING_CFG_2);
	out_be32(&ddr->timing_cfg_3, DDR_TIMING_CFG_3);
	out_be32(&ddr->timing_cfg_4, DDR_TIMING_CFG_4);
	out_be32(&ddr->timing_cfg_5, DDR_TIMING_CFG_5);

#ifdef CONFIG_DEEP_SLEEP
	if (is_warm_boot()) {
		out_be32(&ddr->sdram_cfg_2,
			 DDR_SDRAM_CFG_2 & ~SDRAM_CFG2_D_INIT);
		out_be32(&ddr->init_addr, CONFIG_SYS_SDRAM_BASE);
		out_be32(&ddr->init_ext_addr, (1 << 31));

		/* DRAM VRef will not be trained */
		out_be32(&ddr->ddr_cdr2,
			 DDR_DDR_CDR2 & ~DDR_CDR2_VREF_TRAIN_EN);
	} else
#endif
	{
		out_be32(&ddr->sdram_cfg_2, DDR_SDRAM_CFG_2);
		out_be32(&ddr->ddr_cdr2, DDR_DDR_CDR2);
	}

	out_be32(&ddr->sdram_mode, DDR_SDRAM_MODE);
	out_be32(&ddr->sdram_mode_2, DDR_SDRAM_MODE_2);

	out_be32(&ddr->sdram_interval, DDR_SDRAM_INTERVAL);

	out_be32(&ddr->ddr_wrlvl_cntl, DDR_DDR_WRLVL_CNTL);

	out_be32(&ddr->ddr_wrlvl_cntl_2, DDR_DDR_WRLVL_CNTL_2);
	out_be32(&ddr->ddr_wrlvl_cntl_3, DDR_DDR_WRLVL_CNTL_3);

	out_be32(&ddr->ddr_cdr1, DDR_DDR_CDR1);

	out_be32(&ddr->sdram_clk_cntl, DDR_SDRAM_CLK_CNTL);
	out_be32(&ddr->ddr_zq_cntl, DDR_DDR_ZQ_CNTL);

	out_be32(&ddr->cs0_config_2, DDR_CS0_CONFIG_2);

	/* DDR erratum A-009942 */
	tmp = in_be32(&ddr->debug[28]);
	out_be32(&ddr->debug[28], tmp | 0x0070006f);

	udelay(1);

#ifdef CONFIG_DEEP_SLEEP
	if (is_warm_boot()) {
		/* enter self-refresh */
		temp_sdram_cfg = in_be32(&ddr->sdram_cfg_2);
		temp_sdram_cfg |= SDRAM_CFG2_FRC_SR;
		out_be32(&ddr->sdram_cfg_2, temp_sdram_cfg);

		temp_sdram_cfg = (DDR_SDRAM_CFG_MEM_EN | SDRAM_CFG_BI);
	} else
#endif
		temp_sdram_cfg = (DDR_SDRAM_CFG_MEM_EN & ~SDRAM_CFG_BI);

	out_be32(&ddr->sdram_cfg, DDR_SDRAM_CFG | temp_sdram_cfg);

#ifdef CONFIG_DEEP_SLEEP
	if (is_warm_boot()) {
		/* exit self-refresh */
		temp_sdram_cfg = in_be32(&ddr->sdram_cfg_2);
		temp_sdram_cfg &= ~SDRAM_CFG2_FRC_SR;
		out_be32(&ddr->sdram_cfg_2, temp_sdram_cfg);
	}
#endif
#endif /* !defined(CONFIG_SPL) || defined(CONFIG_SPL_BUILD) */
}

int dram_init(void)
{
#if defined(CONFIG_SYS_FSL_DDR4)
	fsl_initdram();
#else
	ddrmc_init();
#endif

	erratum_a008850_post();

	gd->ram_size = PHYS_SDRAM_SIZE;

#if defined(CONFIG_DEEP_SLEEP) && !defined(CONFIG_SPL_BUILD)
	fsl_dp_resume();
#endif

#ifdef	CONFIG_ARMV7_TEE
#define	CSU_SEC_ACCESS_REG_OFFSET	(0x21c/4)
#define	TZASC_BYPASS_MUX_DISABLE	0x4
#define	CCI_TERMINATE_BARRIER_TX	0x8
	/*
	 * Configure CCI control override register to terminate all barrier
	 * transactions
	 */
	out_le32(((u32 *)(CONFIG_SYS_IMMR + CONFIG_SYS_CCI400_OFFSET)),
		CCI_TERMINATE_BARRIER_TX);
	/* Configure CSU secure access register to disable TZASC bypass mux */
	out_be32(((u32 *)((u32 *)CONFIG_SYS_FSL_CSU_ADDR +
		CSU_SEC_ACCESS_REG_OFFSET)), TZASC_BYPASS_MUX_DISABLE);
	/* Set security permissions for region 0 */
	tzasc_set_region(SYS_FSL_TZASC_ADDR, 0, 0, 0, 0, 0,
			TZASC_REGION_SECURITY_NSRW, 0);

	/* Set region 1 */
	tzasc_set_region(SYS_FSL_TZASC_ADDR, 1, TZASC_REGION_ENABLED,
			SYS_OPTEE_ENTRY, 0, TZASC_REGION_SIZE_64MB,
			TZASC_REGION_SECURITY_SRW, 0x80);

	/* Set region 2 */
#define	TEE_RAM_UPPER_SUBREGION_OFFSET	0x03800000
	tzasc_set_region(SYS_FSL_TZASC_ADDR, 2, TZASC_REGION_ENABLED,
			(SYS_OPTEE_ENTRY + TEE_RAM_UPPER_SUBREGION_OFFSET), 0,
			TZASC_REGION_SIZE_8MB, TZASC_REGION_SECURITY_SRW, 0xc0);
#endif

	return 0;
}

int board_eth_init(bd_t *bis)
{
	return pci_eth_init(bis);
}

int board_early_init_f(void)
{
	struct ccsr_scfg *scfg = (struct ccsr_scfg *)CONFIG_SYS_FSL_SCFG_ADDR;

#ifdef CONFIG_TSEC_ENET
	/*
	 * Clear BD & FR bits for big endian BD's and frame data (aka set
	 * correct eTSEC endianness). This is crucial in ensuring that it does
	 * not report Data Parity Errors in its RX/TX FIFOs when attempting to
	 * send traffic.
	 */
	clrbits_be32(&scfg->etsecdmamcr, SCFG_ETSECDMAMCR_LE_BD_FR);
	/* EC3_GTX_CLK125 (of enet2) used for all RGMII interfaces */
#if 1/*[#71] EAG6L Board Bring-up, balkrow, 2024-07-03*/
	out_be32(&scfg->etsecmcr, SCFG_ETSECCMCR_GE0_CLK125);
#endif /*End 71*/
#endif

#ifdef CONFIG_FSL_IFC
        init_early_memctl_regs();
#endif

	arch_soc_init();

#if defined(CONFIG_DEEP_SLEEP)
	if (is_warm_boot()) {
		timer_init();
		dram_init();
	}
#endif

	return 0;
}

#ifdef CONFIG_SPL_BUILD
void board_init_f(ulong dummy)
{
	void (*second_uboot)(void);

	/* Clear the BSS */
	memset(__bss_start, 0, __bss_end - __bss_start);

	get_clocks();

#if defined(CONFIG_DEEP_SLEEP)
	if (is_warm_boot())
		fsl_dp_disable_console();
#endif

	preloader_console_init();

	dram_init();

	/* Allow OCRAM access permission as R/W */
#ifdef CONFIG_LAYERSCAPE_NS_ACCESS
	enable_layerscape_ns_access();
	enable_layerscape_ns_access();
#endif

	/*
	 * if it is woken up from deep sleep, then jump to second
	 * stage U-Boot and continue executing without recopying
	 * it from SD since it has already been reserved in memory
	 * in last boot.
	 */
	if (is_warm_boot()) {
		second_uboot = (void (*)(void))CONFIG_SYS_TEXT_BASE;
		second_uboot();
	}

	board_init_r(NULL, 0);
}
#endif

#if 1/*[#92] uboot에서 FPGA bank 보정작업, balkrow, 2024-08-14*/
uint16_t str_to_16bitInt(char * const str, uint16_t *value)
{
	ulong temp;

	errno = 0; // 초기화
	temp = simple_strtoul(str, NULL, 10);

	// 변환 오류 확인
	if (errno != 0)
	   return 0; // 변환 실패
	
	if (temp != 1 && temp != 2)
	   return 0; // 범위 초과

	*value = (uint16_t)temp; 
	return 1;
}
int try_boot_with_confirm(uint16_t bank)
{
	const void *buf;
	uint16_t loop;

	buf = map_sysmem(0x7000001c, 2);
	*((u16 *)buf) = bank;
	unmap_sysmem(buf);
#if 1/*[#217] u-boot에서 bank절체 안되고 default bank로 부팅안되는 이슈, balkrow, 2024-12-02*/
	udelay(1000000);
#endif

	for(loop = 0; loop < 2; loop++)
	{
		uint16_t bank_cfg_result;
		buf = map_sysmem(0x70000010, 2);
		bank_cfg_result = *(volatile uint16_t *)buf;
		unmap_sysmem(buf);

		if(bank_cfg_result == BANK_CFG_SUCCESS) 
		{
			printf("Successfully booted from the bank %d.\n", bank);
#if 0/*[#106] init 시 FPGA update 기능 추가, balkrow, 2024-08-28 */
			buf = map_sysmem(0x60f00000, 2);
			*((u16 *)buf) = bank << 8;
			unmap_sysmem(buf);
#else
			if(bank == 1)
				env_set("fpga_bank", "1");
			else if(bank == 2)
				env_set("fpga_bank", "2");
			else
				env_set("fpga_bank", "0");

#endif
			return 1;
		}
		else
			printf("retry bank %d result=%x\n", bank, bank_cfg_result);

#if 1/*[#217] u-boot에서 bank절체 안되고 default bank로 부팅안되는 이슈, balkrow, 2024-12-02*/
		udelay(100000);
#endif
	}
	return 0;
}
int fpga_bank_adjust(void)
{
	const void *buf;
	uint16_t running_bank, act_bank = 0, std_bank = 0;
	uint16_t try_cnt;
	char *act_bank_str = env_get("fw_act_bank");
	char *std_bank_str = env_get("fw_stb_bank");
#if 1/*[#217] u-boot에서 bank절체 안되고 default bank로 부팅안되는 이슈, balkrow, 2024-12-02*/
	buf = map_sysmem(0x70000020, 2);
	*((u16 *)buf) = 0xa5a5;
	unmap_sysmem(buf);
	udelay(100000);
	buf = map_sysmem(0x70000020, 2);
	*((u16 *)buf) = 0x0;
	unmap_sysmem(buf);
#endif

	buf = map_sysmem(0x7000001c, 2);
#if 1/*[#126] bank switch 후 CPU 멈춤현상, balkrow, 2024-09-25*/
	running_bank = *(volatile uint16_t *)buf;
	unmap_sysmem(buf);
#if 1/*[#137] u-boot fpga default booting 관련 수정, balkrow, 2024-09-27*/
	printf("FPGA running bank %d\n", running_bank);
#endif
#endif
	/*check active bank*/
	if(act_bank_str != NULL)
		str_to_16bitInt(act_bank_str, &act_bank);
	printf("FPGA active bank %d\n", act_bank);
	
	/*check standby bank*/
	if(std_bank_str != NULL)
		str_to_16bitInt(std_bank_str, &std_bank);
	printf("FPGA standby bank %d\n", std_bank);

#if 1/*[#126] bank switch 후 CPU 멈춤현상, balkrow, 2024-09-25*/
	if(running_bank == act_bank)
	{
#if 1/*[#126] bank switch 후 CPU 멈춤현상, balkrow, 2024-09-25*/
		uint16_t status;
		buf = map_sysmem(0x70000010, 2);
		status = *(volatile uint16_t *)buf;
		unmap_sysmem(buf);

		if(status == 0x9)
		{
			printf("FPGA running_bank %d, act_bank %d is same\n", running_bank, act_bank);
#if 1/*[#162] u-boot에서 fpga bank를 비정상 적으로 올리는 현상, balkrow, 2024-10-23*/
			env_set("fpga_bank", act_bank_str);
#endif
			return 0;
		}
#endif
	}
#endif

	if(act_bank != 0) 
	{
		printf("Attempt to boot from the Active bank..\n");
		for(try_cnt = 0; try_cnt < BANK_MAX_TRY; try_cnt++)
			if(try_boot_with_confirm(act_bank))
				return;

	}

	if(std_bank != 0) 
	{
		printf("Attempt to boot from the Standby bank..\n");
		for(try_cnt = 0; try_cnt < BANK_MAX_TRY; try_cnt++)
			if(try_boot_with_confirm(std_bank))
				return;

	}

#if 1/*[#126] bank switch 후 CPU 멈춤현상, balkrow, 2024-09-25*/
	if(act_bank != 0 || std_bank != 0) 
	{
		printf("Attempt to boot from the default bank..\n");
		try_boot_with_confirm(0);
	}
#if 1/*[#137] u-boot fpga default booting 관련 수정, balkrow, 2024-09-27*/
	else if(act_bank == 0 && running_bank != act_bank)
	{
		printf("Attempt to boot from the default bank..\n");
		try_boot_with_confirm(0);
	}
#endif
#endif
	printf("FPGA Default bank booting..\n"); 
#if 1/*[#110] RDL function Debugging 및 수정, balkrow, 2024-09-03*/
	env_set("fpga_bank", "0");
#endif
#if 0/*[#106] init 시 FPGA update 기능 추가, balkrow, 2024-08-28 */
#if 1/*[#82] eag6l board SW Debugging, balkrow, 2024-08-26*/
	{
		void *buf;
		buf = map_sysmem(0x60f00000, 2);
		*((u16 *)buf) = 0 << 8;
		unmap_sysmem(buf);
	}
#endif
#endif

	return 0;
}
#endif

int board_init(void)
{
#ifndef CONFIG_SYS_FSL_NO_SERDES
	fsl_serdes_init();
#endif
	ls102xa_smmu_stream_id_init();

#ifdef CONFIG_LAYERSCAPE_NS_ACCESS
	enable_layerscape_ns_access();
#endif

#ifdef CONFIG_U_QE
	u_qe_init();
#endif

#if 0/*debug print */
	print_ifc_regs();
#endif
#if 0/*[#126] bank switch 후 CPU 멈춤현상, balkrow, 2024-09-24*/
	fpga_bank_adjust();
#endif
	return 0;
}

#if defined(CONFIG_SPL_BUILD)
void spl_board_init(void)
{
	ls102xa_smmu_stream_id_init();
}
#endif

#ifdef CONFIG_BOARD_LATE_INIT
int board_late_init(void)
{
#ifdef CONFIG_CHAIN_OF_TRUST
	fsl_setenv_chain_of_trust();
#endif

	return 0;
}
#endif

#if defined(CONFIG_MISC_INIT_R)
int misc_init_r(void)
{
#ifdef CONFIG_FSL_DEVICE_DISABLE
	device_disable(devdis_tbl, ARRAY_SIZE(devdis_tbl));
#endif

#if 1/*[#126] bank switch 후 CPU 멈춤현상, balkrow, 2024-09-24*/
	fpga_bank_adjust();
#endif

#ifdef CONFIG_FSL_CAAM
	return sec_init();
#endif
}
#endif

#if defined(CONFIG_DEEP_SLEEP)
void board_sleep_prepare(void)
{
#ifdef CONFIG_LAYERSCAPE_NS_ACCESS
	enable_layerscape_ns_access();
#endif
}
#endif

int ft_board_setup(void *blob, bd_t *bd)
{
	ft_cpu_setup(blob, bd);

#ifdef CONFIG_PCI
	ft_pci_setup(blob, bd);
#endif

	return 0;
}
