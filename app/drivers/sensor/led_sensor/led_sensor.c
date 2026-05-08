/*
 * Copyright (c) 2024 My Project
 * SPDX-License-Identifier: Apache-2.0
 *
 * LED Sensor driver.
 * - sensor_sample_fetch : turns the LED ON
 * - sensor_channel_get  : turns the LED OFF and returns the last state
 */

#include <zephyr/device.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/logging/log.h>

#define DT_DRV_COMPAT zephyr_led_sensor

LOG_MODULE_REGISTER(led_sensor, CONFIG_SENSOR_LOG_LEVEL);

struct led_sensor_config {
	struct gpio_dt_spec led;
};

struct led_sensor_data {
	int state; /* 1 = ON, 0 = OFF */
};

static int led_sensor_sample_fetch(const struct device *dev,
				   enum sensor_channel chan)
{
	const struct led_sensor_config *cfg = dev->config;
	struct led_sensor_data *data = dev->data;

	if (chan != SENSOR_CHAN_ALL && chan != SENSOR_CHAN_LIGHT) {
		return -ENOTSUP;
	}

	gpio_pin_set_dt(&cfg->led, 1);
	data->state = 1;

	LOG_INF("LED ON (sample_fetch)");
	return 0;
}

static int led_sensor_channel_get(const struct device *dev,
				  enum sensor_channel chan,
				  struct sensor_value *val)
{
	const struct led_sensor_config *cfg = dev->config;
	struct led_sensor_data *data = dev->data;

	if (chan != SENSOR_CHAN_LIGHT) {
		return -ENOTSUP;
	}

	gpio_pin_set_dt(&cfg->led, 0);
	data->state = 0;

	val->val1 = data->state;
	val->val2 = 0;

	LOG_INF("LED OFF (channel_get)");
	return 0;
}

static int led_sensor_init(const struct device *dev)
{
	const struct led_sensor_config *cfg = dev->config;

	if (!gpio_is_ready_dt(&cfg->led)) {
		LOG_ERR("LED GPIO not ready");
		return -ENODEV;
	}

	int ret = gpio_pin_configure_dt(&cfg->led, GPIO_OUTPUT_INACTIVE);
	if (ret < 0) {
		LOG_ERR("Failed to configure LED GPIO: %d", ret);
		return ret;
	}

	LOG_INF("LED sensor initialized");
	return 0;
}

static DEVICE_API(sensor, led_sensor_api) = {
	.sample_fetch = led_sensor_sample_fetch,
	.channel_get  = led_sensor_channel_get,
};

#define LED_SENSOR_INIT(inst)							\
	static struct led_sensor_data led_sensor_data_##inst;			\
	static const struct led_sensor_config led_sensor_cfg_##inst = {		\
		.led = GPIO_DT_SPEC_INST_GET(inst, gpios),			\
	};									\
	SENSOR_DEVICE_DT_INST_DEFINE(inst,					\
				     led_sensor_init,				\
				     NULL,					\
				     &led_sensor_data_##inst,			\
				     &led_sensor_cfg_##inst,			\
				     POST_KERNEL,				\
				     CONFIG_SENSOR_INIT_PRIORITY,		\
				     &led_sensor_api);

DT_INST_FOREACH_STATUS_OKAY(LED_SENSOR_INIT)
