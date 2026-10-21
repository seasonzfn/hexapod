#include "pico/stdlib.h"
#include "sensor_servo_control.hpp"

int main() {
    stdio_init_all();
    sleep_ms(3000);

    runSensorServoControl();
}
