/* linux/arch/arm/mach-msm/board-vivow-keypad.c
 *
 * Copyright (C) 2010-2011 HTC Corporation.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/platform_device.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/gpio_event.h>
#include <linux/keyreset.h>
#include <asm/mach-types.h>
#include <linux/gpio.h>
#include <mach/gpio.h>

#include "board-vivow.h"
#include "proc_comm.h"
#include <linux/mfd/pmic8058.h>

static char *keycaps = "--qwerty";
#undef MODULE_PARAM_PREFIX
#define MODULE_PARAM_PREFIX "board_vivow."
module_param_named(keycaps, keycaps, charp, 0);

static struct gpio_event_direct_entry vivow_keypad_input_map[] = {
	{
		.gpio = VIVOW_GPIO_KEYPAD_POWER_KEY,
		.code = KEY_POWER,
	},
	{
		.gpio = PM8058_GPIO_PM_TO_SYS(VIVOW_VOL_UP),
		.code = KEY_VOLUMEUP,
	},
	{
		.gpio = PM8058_GPIO_PM_TO_SYS(VIVOW_VOL_DN),
		.code = KEY_VOLUMEDOWN,
	},
};

uint32_t inputs_gpio_table[] = {
	PCOM_GPIO_CFG(VIVOW_GPIO_KEYPAD_POWER_KEY, 0, GPIO_INPUT,
		      GPIO_PULL_UP, GPIO_4MA),
};

static void vivow_setup_input_gpio(void)
{
	gpio_tlmm_config(inputs_gpio_table[0], GPIO_CFG_ENABLE);
}

static struct gpio_event_input_info vivow_keypad_input_info = {
	.info.func = gpio_event_input_func,
	.flags = GPIOEDF_PRINT_KEYS,
	.type = EV_KEY,
#if BITS_PER_LONG != 64 && !defined(CONFIG_KTIME_SCALAR)
	.debounce_time.tv.nsec = 8 * NSEC_PER_MSEC,
# else
	.debounce_time.tv64 = 8 * NSEC_PER_MSEC,
# endif
	.keymap = vivow_keypad_input_map,
	.keymap_size = ARRAY_SIZE(vivow_keypad_input_map),
	.setup_input_gpio = vivow_setup_input_gpio,
};

static struct gpio_event_info *vivow_keypad_info[] = {
	&vivow_keypad_input_info.info,
};

static struct gpio_event_platform_data vivow_keypad_data = {
	.names = {
		"vivow-keypad",
		NULL,
	},
	.info = vivow_keypad_info,
	.info_count = ARRAY_SIZE(vivow_keypad_info),
};

static struct platform_device vivow_keypad_input_device = {
	.name = GPIO_EVENT_DEV_NAME,
	.id = 0,
	.dev		= {
		.platform_data	= &vivow_keypad_data,
	},
};

static struct keyreset_platform_data vivow_reset_keys_pdata = {
	.keys_down = {
		KEY_POWER,
		KEY_VOLUMEDOWN,
		KEY_VOLUMEUP,
		0
	},
};

struct platform_device vivow_reset_keys_device = {
	.name = KEYRESET_NAME,
	.dev.platform_data = &vivow_reset_keys_pdata,
};

int __init vivow_init_keypad(void)
{
	printk(KERN_DEBUG "%s\n", __func__);

	if (platform_device_register(&vivow_reset_keys_device))
		printk(KERN_WARNING "%s: register reset key fail\n", __func__);

	return platform_device_register(&vivow_keypad_input_device);
}
