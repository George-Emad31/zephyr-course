#include <zephyr/kernel.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/logging/log.h>

#include "drivers/sensor/led_sensor/led_sensor.h"

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

static const struct device *led_sensor = DEVICE_DT_GET(DT_NODELABEL(led_sensor_node));

int main(void) {
  if (!device_is_ready(led_sensor)) {
    LOG_ERR("LED sensor device not ready");
    return 0;
  }

  LOG_INF("LED sensor ready. Use shell: 'sensor fetch|read|info'");
  return 0;
}
