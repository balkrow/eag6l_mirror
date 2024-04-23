/*
 * drivers/net/ethernet/marvell/prestera/mvpp.c
 *
 * Driver for Marvell Prestera switch
 *
 * Author: Raz Adashi
 *
 * Copyright (c) 2020 Marvell, Inc.
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 *
 */
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/of_device.h>

#include <linux/io.h>

MODULE_DESCRIPTION("Marvell Prestera driver");
MODULE_AUTHOR("Raz Adashi");
MODULE_LICENSE("GPL");

#define ADD_COMP_CNTRL_REG     0x140
#define ADD_COMP_REGION_7_REG  0x13c

enum prestera_type {
	PRESTERA_DEV_TYPE_FALCON,
	PRESTERA_DEV_TYPE_BC2
};

static const struct of_device_id prestera_of_match[] = {
	{ .compatible = "marvell,prestera-98dx42xx",
	  .data = (void *)PRESTERA_DEV_TYPE_BC2},
	{ }
};

int prestera_mvpp_probe(struct platform_device *pdev) {
	void *addr;
	int value;
	enum prestera_type type;

	type = (enum prestera_type)of_device_get_match_data(&pdev->dev);
	pr_debug("%s: device type %d %s - resource %s (0x%08x 0x%08x)\n",  __func__, type,
			pdev->name, pdev->resource->name, pdev->resource->start, pdev->resource->end);

	/* Set device to 8 regsion completion register mode */
	addr = ioremap(pdev->resource->start, 0x200);
	value = readl(addr + ADD_COMP_CNTRL_REG);
	value &= ~(1 << 16);
	writel(value, addr + ADD_COMP_CNTRL_REG);
	/* Set regsion #7 completion register to offset 0x54000000 */
	writel(0xa80, addr + ADD_COMP_REGION_7_REG);

	return 0;
}

static struct platform_driver prestera_driver = {
	.probe = prestera_mvpp_probe,
	.driver = {
		.name = "Marvell prestera_driver",
		.of_match_table = prestera_of_match
	}
};

static int __init prestera_mvpp_init(void)
{
	return platform_driver_register(&prestera_driver);
}

static void __init prestera_mvpp_exit(void)
{
	return;
}

subsys_initcall(prestera_mvpp_init);
module_exit(prestera_mvpp_exit);
/* module_driver(prestera_driver, prestera_mvpp_register, prestera_mvpp_unregister); */

MODULE_DEVICE_TABLE(mdio, marvell_tbl);
