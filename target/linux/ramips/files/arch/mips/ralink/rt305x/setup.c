/*
 * Ralink RT305x SoC specific setup
 *
 * Copyright (C) 2008-2011 Gabor Juhos <juhosg@openwrt.org>
 *
 * Parts of this file are based on Ralink's 2.6.21 BSP
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/err.h>
#include <linux/clk.h>

#include <asm/mips_machine.h>
#include <asm/reboot.h>
#include <asm/time.h>

#include <asm/mach-ralink/common.h>
#include <asm/mach-ralink/rt305x.h>
#include <asm/mach-ralink/rt305x_regs.h>
#include "common.h"

static unsigned int ramips_cpu_freq;

static void rt305x_restart(char *command)
{
	rt305x_sysc_wr(RT305X_RESET_SYSTEM, SYSC_REG_RESET_CTRL);
	while (1)
		if (cpu_wait)
			cpu_wait();
}

static void rt305x_halt(void)
{
	while (1)
		if (cpu_wait)
			cpu_wait();
}

unsigned int __cpuinit get_c0_compare_irq(void)
{
	return CP0_LEGACY_COMPARE_IRQ;
}

void __init ramips_soc_setup(void)
{
	struct clk *clk;

	rt305x_sysc_base = ioremap_nocache(RT305X_SYSC_BASE, PAGE_SIZE);
	rt305x_memc_base = ioremap_nocache(RT305X_MEMC_BASE, PAGE_SIZE);

	rt305x_clocks_init();

	clk = clk_get(NULL, "cpu");
	if (IS_ERR(clk))
		panic("unable to get CPU clock, err=%ld", PTR_ERR(clk));

	ramips_cpu_freq = clk_get_rate(clk);
	printk(KERN_INFO "%s running at %lu.%02lu MHz\n", ramips_sys_type,
		clk_get_rate(clk) / 1000000,
		(clk_get_rate(clk) % 1000000) * 100 / 1000000);

	_machine_restart = rt305x_restart;
	_machine_halt = rt305x_halt;
	pm_power_off = rt305x_halt;

	clk = clk_get(NULL, "uart");
	if (IS_ERR(clk))
		panic("unable to get UART clock, err=%ld", PTR_ERR(clk));

	ramips_early_serial_setup(0, RT305X_UART0_BASE, clk_get_rate(clk),
				  RT305X_INTC_IRQ_UART0);
	ramips_early_serial_setup(1, RT305X_UART1_BASE, clk_get_rate(clk),
				  RT305X_INTC_IRQ_UART1);
}

void __init plat_time_init(void)
{
	struct clk *clk;

	clk = clk_get(NULL, "cpu");
	if (IS_ERR(clk))
		panic("unable to get CPU clock, err=%ld", PTR_ERR(clk));

	mips_hpt_frequency = clk_get_rate(clk) / 2;
}

unsigned int ramips_get_cpu_freq(void)
{
	return ramips_cpu_freq;
}
