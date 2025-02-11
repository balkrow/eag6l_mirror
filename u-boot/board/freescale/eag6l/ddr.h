/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * Copyright 2015 Freescale Semiconductor, Inc.
 */

#ifndef __DDR_H__
#define __DDR_H__

extern void erratum_a008850_post(void);

struct board_specific_parameters {
	u32 n_ranks;
	u32 datarate_mhz_high;
	u32 rank_gb;
	u32 clk_adjust;
	u32 wrlvl_start;
	u32 wrlvl_ctl_2;
	u32 wrlvl_ctl_3;
	u32 cpo_override;
	u32 write_data_delay;
	u32 force_2t;
};

/*
 * These tables contain all valid speeds we want to override with board
 * specific parameters. datarate_mhz_high values need to be in ascending order
 * for each n_ranks group.
 */
static const struct board_specific_parameters udimm0[] = {
	/*
	 * memory controller 0
	 *   num|  hi| rank|  clk| wrlvl |   wrlvl   |  wrlvl | cpo  |wrdata|2T
	 * ranks| mhz| GB  |adjst| start |   ctl2    |  ctl3  |      |delay |
	 */
#ifdef CONFIG_SYS_FSL_DDR4
	{1,  1666, 0, 12,     7, 0x07090800, 0x00000000,},
	{1,  1900, 0, 12,     7, 0x07090800, 0x00000000,},
	{1,  2200, 0, 12,     7, 0x07090800, 0x00000000,},
#endif
	{}
};

static const struct board_specific_parameters *udimms[] = {
	udimm0,
};

#ifndef CONFIG_SYS_DDR_RAW_TIMING
fsl_ddr_cfg_regs_t ddr_cfg_regs_1600 = {
	.cs[0].bnds = 0x008000FF, 
	.cs[1].bnds = 0,
	.cs[2].bnds = 0,
	.cs[3].bnds = 0,
	.cs[0].config = 0x80010412,
	.cs[0].config_2 = 0,
	.cs[1].config = 0,
	.cs[1].config_2 = 0,
	.cs[2].config = 0,
	.cs[3].config = 0,
	.timing_cfg_3 = 0x01111000,
	.timing_cfg_0 = 0x4055000c,
	.timing_cfg_1 = 0xccc60c66,
	.timing_cfg_2 = 0x0048d11c,
	.ddr_sdram_cfg = 0x450C0000,
	.ddr_sdram_cfg_2 = 0x00401010,
	.ddr_sdram_cfg_3 = 0,
	.ddr_sdram_mode = 0x01010214,
	.ddr_sdram_mode_2 = 0,
	.ddr_sdram_mode_3 = 0,
	.ddr_sdram_mode_4 = 0,
	.ddr_sdram_mode_5 = 0,
	.ddr_sdram_mode_6 = 0,
	.ddr_sdram_mode_7 = 0,
	.ddr_sdram_mode_8 = 0,
	.ddr_sdram_mode_9 = 0x0500,
	.ddr_sdram_mode_10 = 0x04000000,
	.ddr_sdram_interval = 0x18600618,
	.ddr_data_init = 0xDEADBEEF,
	.ddr_sdram_clk_cntl = 0x02000000,
	.ddr_init_addr = 0,
	.ddr_init_ext_addr = 0,
	.timing_cfg_4 = 0x01,
	.timing_cfg_5 = 0x05401400,
	.timing_cfg_6 = 0,
	.timing_cfg_7 = 0x13300000,
	.timing_cfg_8 = 0x03115600,
	.timing_cfg_9 = 0,
	.ddr_zq_cntl = 0x8A090705,
	.ddr_wrlvl_cntl = 0x8675F604,
	.ddr_wrlvl_cntl_2 = 0x04050500,
	.ddr_wrlvl_cntl_3 = 0,
	.ddr_sr_cntr = 0,
	.ddr_sdram_rcw_1 = 0,
	.ddr_sdram_rcw_2 = 0,
	.ddr_cdr1 = 0x80080000,
	.ddr_cdr2 = 0x0000A00,
	.dq_map_0 = 0x6106104,
	.dq_map_1 = 0x84184000,
	.dq_map_2 = 0,
	.dq_map_3 = 0,
	.debug[28] = 0x00700046,

};

fixed_ddr_parm_t fixed_ddr_parm_0[] = {
	{1550, 1650, &ddr_cfg_regs_1600},
	{0, 0, NULL}
};

#endif
#endif
