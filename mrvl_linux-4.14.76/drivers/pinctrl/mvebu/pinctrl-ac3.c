/*
 * Marvell ac3 pinctrl driver based on mvebu pinctrl core
 *
 * Copyright (C) 2021 Marvell
 *
 * Noam Liron <lnoam@marvell.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <linux/err.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/of_device.h>
#include <linux/pinctrl/pinctrl.h>

#include "pinctrl-mvebu.h"

static struct mvebu_mpp_mode ac3_mpp_modes[] = {
	MPP_MODE(0,
		 MPP_FUNCTION(0, "gpio",    NULL)),
	MPP_MODE(1,
		 MPP_FUNCTION(0, "gpio",    NULL)),
	MPP_MODE(2,
		 MPP_FUNCTION(0, "gpio",    NULL)),
	MPP_MODE(3,
		 MPP_FUNCTION(0, "gpio",    NULL)),
	MPP_MODE(4,
		 MPP_FUNCTION(0, "gpio",    NULL)),
	MPP_MODE(5,
		 MPP_FUNCTION(0, "gpio",    NULL)),
	MPP_MODE(6,
		 MPP_FUNCTION(0, "gpio",    NULL)),
	MPP_MODE(7,
		 MPP_FUNCTION(0, "gpio",    NULL)),
	MPP_MODE(8,
		 MPP_FUNCTION(0, "gpio",    NULL)),
	MPP_MODE(9,
		 MPP_FUNCTION(0, "gpio",    NULL)),
	MPP_MODE(10,
		 MPP_FUNCTION(0, "gpio",    NULL)),
	MPP_MODE(11,
		 MPP_FUNCTION(0, "gpio",    NULL)),
	MPP_MODE(12,
		 MPP_FUNCTION(0, "gpio",    NULL)),
	MPP_MODE(13,
		 MPP_FUNCTION(0, "gpio",    NULL)),
	MPP_MODE(14,
		 MPP_FUNCTION(0, "gpio",    NULL),
		 MPP_FUNCTION(1, "i2c0-opt", "scl")),
	MPP_MODE(15,
		 MPP_FUNCTION(0, "gpio",    NULL),
		 MPP_FUNCTION(1, "i2c0-opt", "sda")),
	MPP_MODE(16,
		 MPP_FUNCTION(0, "gpio",    NULL)),
	MPP_MODE(17,
		 MPP_FUNCTION(0, "gpio",    NULL)),
	MPP_MODE(18,
		 MPP_FUNCTION(0, "gpio",    NULL)),
	MPP_MODE(19,
		 MPP_FUNCTION(0, "gpio",    NULL)),
	MPP_MODE(20,
		 MPP_FUNCTION(0, "gpio",    NULL)),
	MPP_MODE(21,
		 MPP_FUNCTION(0, "gpio",    NULL)),
	MPP_MODE(22,
		 MPP_FUNCTION(0, "gpio",    NULL)),
	MPP_MODE(23,
		 MPP_FUNCTION(0, "gpio",    NULL)),
	MPP_MODE(24,
		 MPP_FUNCTION(0, "gpio",    NULL)),
	MPP_MODE(25,
		 MPP_FUNCTION(0, "gpio",    NULL)),
	MPP_MODE(26,
		 MPP_FUNCTION(0, "gpio",    NULL)),
	MPP_MODE(27,
		 MPP_FUNCTION(0, "gpio",    NULL)),
	MPP_MODE(28,
		 MPP_FUNCTION(0, "gpio",    NULL)),
	MPP_MODE(29,
		 MPP_FUNCTION(0, "gpio",    NULL)),
	MPP_MODE(30,
		 MPP_FUNCTION(0, "gpio",    NULL)),
	MPP_MODE(31,
		 MPP_FUNCTION(0, "gpio",    NULL)),
};

static struct mvebu_pinctrl_soc_info ac3_pinctrl_info;

static const struct of_device_id ac3_pinctrl_of_match[] = {
	{
		.compatible = "marvell,ac3-pinctrl",
	},
	{ },
};

static const struct mvebu_mpp_ctrl ac3_mpp_controls[] = {
	MPP_FUNC_CTRL(0, 31, NULL, mvebu_regmap_mpp_ctrl), };

static struct pinctrl_gpio_range ac3_mpp_gpio_ranges[] = {
	MPP_GPIO_RANGE(0,   0,  0, 31), };

static int ac3_pinctrl_probe(struct platform_device *pdev) {
	struct mvebu_pinctrl_soc_info *soc = &ac3_pinctrl_info;
	const struct of_device_id *match =
		of_match_device(ac3_pinctrl_of_match, &pdev->dev);

	if (!match || !pdev->dev.parent)
		return -ENODEV;

	soc->variant = 0; /* no variants for ac3 */
	soc->controls = ac3_mpp_controls;
	soc->ncontrols = ARRAY_SIZE(ac3_mpp_controls);
	soc->gpioranges = ac3_mpp_gpio_ranges;
	soc->ngpioranges = ARRAY_SIZE(ac3_mpp_gpio_ranges);
	soc->modes = ac3_mpp_modes;
	soc->nmodes = ac3_mpp_controls[0].npins;

	pdev->dev.platform_data = soc;

	return mvebu_pinctrl_simple_regmap_probe(pdev, &pdev->dev, 0);
}

static struct platform_driver ac3_pinctrl_driver = {
	.driver = {
		.name = "ac3-pinctrl",
		.of_match_table = of_match_ptr(ac3_pinctrl_of_match),
	},
	.probe = ac3_pinctrl_probe,
};

builtin_platform_driver(ac3_pinctrl_driver);
