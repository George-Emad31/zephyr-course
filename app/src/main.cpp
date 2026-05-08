#include <zephyr/drivers/gpio.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(DT_ALIAS(app_led), gpios);

LOG_MODULE_REGISTER(main, LOG_LEVEL_INF);

int main(void) {
  bool led_state = true;

  if (!gpio_is_ready_dt(&led)) {
    LOG_ERR("app-led gpio not ready");
    return 0;
  }

  if (gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE) < 0) {
    LOG_ERR("Failed to configure app-led pin");
    return 0;
  }

  LOG_INF("Heartbeat LED configured. Period: %d ms", CONFIG_APP_HEARTBEAT_PERIOD_MS);

  while (1) {
    gpio_pin_set_dt(&led, led_state);
    led_state = !led_state;
    LOG_INF("LED state: %s", led_state ? "ON" : "OFF");
    k_msleep(CONFIG_APP_HEARTBEAT_PERIOD_MS);
  }
  return 0;
}
