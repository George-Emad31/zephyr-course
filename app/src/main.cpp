#include <zephyr/kernel.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

static const struct device *led_sensor = DEVICE_DT_GET(DT_NODELABEL(led_sensor_node));

int main(void) {
  if (!device_is_ready(led_sensor)) {
    LOG_ERR("LED sensor device not ready");
    return 0;
  }

  LOG_INF("LED sensor ready. Starting loop...");

  while (1) {
    /* Turn LED ON via sample_fetch */
    sensor_sample_fetch(led_sensor);
    k_msleep(CONFIG_APP_HEARTBEAT_PERIOD_MS);

    /* Turn LED OFF via channel_get */
    struct sensor_value val;
    sensor_channel_get(led_sensor, SENSOR_CHAN_LIGHT, &val);
    k_msleep(CONFIG_APP_HEARTBEAT_PERIOD_MS);
  }
  return 0;
}
