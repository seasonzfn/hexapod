#include "pico/stdlib.h"
#include <iostream>
#include <stdio

#include "hardware/i2c.h"

#include "pca9685.hpp"
#include "servo.hpp"

constexpr uint SDA_PIN = 4;
constexpr uint SCL_PIN = 5;

constexpr uint8_t PCA_ADDRESS = 0x40;
const uint TOUCH_PIN = 15;

int main() {
    stdio_init_all();
    i2c_init(i2c0, 100000);

    gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);

    gpio_pull_up(SDA_PIN);
    gpio_pull_up(SCL_PIN);

    sleep_ms(1000);

    PCA9685 pca(i2c0, PCA_ADDRESS);

    pca.begin(50);

    Servo motor1(pca, 0);
    Servo motor2(pca, 1);

    while (true) {
        std::cout << "Moving both servos to 90 degrees\n";
        motor1.setAngle(20);
        motor2.setAngle(20);
        sleep_ms(1000);

        std::cout << "Moving servo 1 to 45, servo 2 to 135\n";
        motor1.setAngle(35);
        motor2.setAngle(55);
        sleep_ms(1000);

        std::cout << "Moving servo 1 to 135, servo 2 to 45\n";
        motor1.setAngle(50);
        motor2.setAngle(35);
        sleep_ms(1000);
    }
}
