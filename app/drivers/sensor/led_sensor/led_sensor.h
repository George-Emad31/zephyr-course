/*
 * Copyright (c) 2024 My Project
 * SPDX-License-Identifier: Apache-2.0
 *
 * Public header for the LED sensor driver custom extension API.
 */

#ifndef APP_DRIVERS_SENSOR_LED_SENSOR_H_
#define APP_DRIVERS_SENSOR_LED_SENSOR_H_

#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>

/**
 * @brief LED sensor custom driver API.
 *
 * The standard sensor_driver_api MUST be the first member so that
 * sensor_sample_fetch() / sensor_channel_get() keep working via a
 * safe cast to (const struct sensor_driver_api *).
 */
struct led_sensor_driver_api {
	struct sensor_driver_api sensor; /* must be first */
	int (*set_blink_interval)(const struct device *dev,
				  uint32_t interval_ms);
};

/**
 * @brief Set the blink interval stored in the driver's dynamic data.
 *
 * This is the custom extension function that goes beyond the standard
 * sensor API.  It updates blink_interval_ms in the driver data struct.
 *
 * @param dev         LED sensor device.
 * @param interval_ms New blink interval in milliseconds.
 * @return 0 on success, negative errno on error.
 */
static inline int led_sensor_set_blink_interval(const struct device *dev,
						 uint32_t interval_ms)
{
	const struct led_sensor_driver_api *api =
		(const struct led_sensor_driver_api *)dev->api;

	return api->set_blink_interval(dev, interval_ms);
}

#endif /* APP_DRIVERS_SENSOR_LED_SENSOR_H_ */
