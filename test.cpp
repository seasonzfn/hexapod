#include "pico/stdlib.h"
#include <cstdio>

#include "hardware/i2c.h"
#include "sensors.hpp"

constexpr uint SDA_PIN = 4;
constexpr uint SCL_PIN = 5;
constexpr uint TOUCH_PIN = 15;
constexpr bool TOUCH_ACTIVE_HIGH = true;   // set false if the pad reads inverted

int main() {
    stdio_init_all();
    sleep_ms(3000);                         // time to open minicom

    i2c_init(i2c0, 100000);
    gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(SDA_PIN);
    gpio_pull_up(SCL_PIN);

    Sensors sensors(i2c0);
    TouchButton touch(TOUCH_PIN, TOUCH_ACTIVE_HIGH);
    touch.begin();

    printf("\n=== sensor test ===\n");
    sensors.scan();

    // ---- self checks ----
    float t = 0, lux = 0;
    const bool tempOk = sensors.readTemperature(t);
    printf("[%s] temperature sensor at 0x%02X", tempOk ? "PASS" : "FAIL", SENSOR_TEMP_ADDR);
    if (tempOk) printf("  (%.3f C)", t);
    printf("\n");

    const bool lightOk = sensors.beginLight();
    printf("[%s] light sensor at 0x%02X, part id 0x%02X (expect 0xB3)\n",
           (lightOk && sensors.lightPartId() == 0xB3) ? "PASS" : "FAIL",
           SENSOR_LIGHT_ADDR, sensors.lightPartId());

    printf("Touch the pad to switch mode. Raw pad level is printed on every change.\n\n");

    bool showTemp = true;
    bool lastRaw = touch.isTouched();
    uint32_t lastPrint = 0;
    uint presses = 0;

    while (true) {
        // raw level, to find out the polarity of your touch module
        const bool raw = touch.isTouched();
        if (raw != lastRaw) {
            printf("touch level changed: %s\n", raw ? "TOUCHED" : "released");
            lastRaw = raw;
        }

        if (touch.wasPressed()) {
            showTemp = !showTemp;
            presses++;
            printf(">>> press #%u, mode = %s\n", presses, showTemp ? "temperature" : "light");
        }

        const uint32_t now = to_ms_since_boot(get_absolute_time());
        if (now - lastPrint >= 1000) {
            lastPrint = now;
            if (showTemp) {
                if (sensors.readTemperature(t)) printf("Temp : %.3f C\n", t);
                else                            printf("Temp : read error\n");
            } else {
                if (sensors.readLight(lux)) printf("Light: %.2f lux\n", lux);
                else                        printf("Light: read error\n");
            }
        }
        sleep_ms(10);
    }
}