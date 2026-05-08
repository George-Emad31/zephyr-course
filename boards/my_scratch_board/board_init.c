/*
 * Copyright (c) 2024 My Project
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/init.h>
#include <zephyr/sys/printk.h>

static int my_scratch_board_init(void)
{
	printk("Board Initialized\n");
	return 0;
}

SYS_INIT(my_scratch_board_init, APPLICATION, 0);
