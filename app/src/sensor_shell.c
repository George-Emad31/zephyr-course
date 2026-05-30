/*
 * Copyright (c) 2024 My Project
 * SPDX-License-Identifier: Apache-2.0
 *
 * Shell commands for the LED sensor driver.
 *
 * Usage:
 *   sensor fetch          -- calls sensor_sample_fetch() (turns LED ON)
 *   sensor read           -- calls sensor_channel_get()  (turns LED OFF, prints value)
 *   sensor info           -- prints device name and ready state
 *   sensor set <ms>       -- calls led_sensor_set_blink_interval() (1-60000 ms)
 */

#include <zephyr/kernel.h>
#include <zephyr/shell/shell.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <stdlib.h>

#include "drivers/sensor/led_sensor/led_sensor.h"

static const struct device *led_sensor_dev =
	DEVICE_DT_GET(DT_NODELABEL(led_sensor_node));

#define SET_INTERVAL_MIN 1U
#define SET_INTERVAL_MAX 60000U

/* sensor fetch */
static int cmd_sensor_fetch(const struct shell *sh, size_t argc, char **argv)
{
	ARG_UNUSED(argc);
	ARG_UNUSED(argv);

	int ret = sensor_sample_fetch(led_sensor_dev);

	if (ret < 0) {
		shell_error(sh, "sensor_sample_fetch() failed: %d", ret);
		return ret;
	}

	shell_print(sh, "sensor_sample_fetch() OK (LED ON)");
	return 0;
}

/* sensor read */
static int cmd_sensor_read(const struct shell *sh, size_t argc, char **argv)
{
	ARG_UNUSED(argc);
	ARG_UNUSED(argv);

	struct sensor_value val;
	int ret = sensor_channel_get(led_sensor_dev, SENSOR_CHAN_LIGHT, &val);

	if (ret < 0) {
		shell_error(sh, "sensor_channel_get() failed: %d", ret);
		return ret;
	}

	shell_print(sh, "SENSOR_CHAN_LIGHT: val1=%d, val2=%d (LED OFF)",
		    val.val1, val.val2);
	return 0;
}

/* sensor set <interval_ms> */
static int cmd_sensor_set(const struct shell *sh, size_t argc, char **argv)
{
	/* SHELL_CMD_ARG enforces argc >= 2, but guard for out-of-range */
	if (argc < 2) {
		shell_error(sh, "Usage: sensor set <interval_ms> (%u-%u ms)",
			    SET_INTERVAL_MIN, SET_INTERVAL_MAX);
		return -EINVAL;
	}

	char *end;
	unsigned long val = strtoul(argv[1], &end, 10);

	if (*end != '\0') {
		shell_error(sh, "Invalid number: '%s'", argv[1]);
		return -EINVAL;
	}

	if (val < SET_INTERVAL_MIN || val > SET_INTERVAL_MAX) {
		shell_error(sh, "Value %lu out of range [%u, %u]",
			    val, SET_INTERVAL_MIN, SET_INTERVAL_MAX);
		return -EINVAL;
	}

	int ret = led_sensor_set_blink_interval(led_sensor_dev, (uint32_t)val);

	if (ret < 0) {
		shell_error(sh, "led_sensor_set_blink_interval() failed: %d", ret);
		return ret;
	}

	shell_print(sh, "blink_interval_ms set to %lu ms", val);
	return 0;
}

/* sensor info */
static int cmd_sensor_info(const struct shell *sh, size_t argc, char **argv)
{
	ARG_UNUSED(argc);
	ARG_UNUSED(argv);

	shell_print(sh, "Device name : %s", led_sensor_dev->name);
	shell_print(sh, "Ready       : %s",
		    device_is_ready(led_sensor_dev) ? "yes" : "no");
	return 0;
}

SHELL_STATIC_SUBCMD_SET_CREATE(sub_sensor,
	SHELL_CMD(fetch, NULL,
		  "Call sensor_sample_fetch() -- turns LED ON",
		  cmd_sensor_fetch),
	SHELL_CMD(read,  NULL,
		  "Call sensor_channel_get() and print result -- turns LED OFF",
		  cmd_sensor_read),
	SHELL_CMD(info,  NULL,
		  "Print device name and ready state",
		  cmd_sensor_info),
	SHELL_CMD_ARG(set, NULL,
		      "Set blink interval: sensor set <interval_ms> (1-60000)",
		      cmd_sensor_set, 2, 0),
	SHELL_SUBCMD_SET_END
);

SHELL_CMD_REGISTER(sensor, &sub_sensor, "LED sensor commands", NULL);
