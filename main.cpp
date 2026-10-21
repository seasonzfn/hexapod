#include "pico/stdlib.h"
#include <algorithm>
#include <iostream>
#include <limits>
#include <stdio.h>

#include "hardware/i2c.h"

#include "pca9685.hpp"
#include "servo.hpp"

constexpr uint SDA_PIN = 4;
constexpr uint SCL_PIN = 5;

constexpr uint8_t PCA_ADDRESS = 0x40;
const uint TOUCH_PIN = 15;

static uint16_t readOffCount(uint8_t channel) {
    const uint8_t regL = 0x08 + 4 * channel;
    const uint8_t regH = 0x09 + 4 * channel;
    uint8_t lo = 0;
    uint8_t hi = 0;

    if (i2c_write_blocking(i2c0, PCA_ADDRESS, &regL, 1, true) != 1 ||
        i2c_read_blocking(i2c0, PCA_ADDRESS, &lo, 1, false) != 1 ||
        i2c_write_blocking(i2c0, PCA_ADDRESS, &regH, 1, true) != 1 ||
        i2c_read_blocking(i2c0, PCA_ADDRESS, &hi, 1, false) != 1) {
        return 0;
    }

    if (hi & 0x10) return 0;
    return static_cast<uint16_t>(((hi & 0x0F) << 8) | lo);
}

static void moveTo(Servo& servo, float target, float step = 1.0f, uint delayMs = 50) {
    target = std::clamp(target, 0.0f, 180.0f);
    while (true) {
        const float difference = target - servo.angle();
        if (difference > -step && difference < step) break;
        servo.moveRelative(difference > 0 ? step : -step);
        sleep_ms(delayMs);
    }
    servo.setAngle(target);
}

int main() {
    stdio_init_all();
    i2c_init(i2c0, 100000);

    gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);

    gpio_pull_up(SDA_PIN);
    gpio_pull_up(SCL_PIN);

    sleep_ms(1000);

    // Read current PWM outputs before begin() changes the PCA9685 configuration.
    const uint16_t count1 = readOffCount(0);
    const uint16_t count2 = readOffCount(1);

    PCA9685 pca(i2c0, PCA_ADDRESS);
    pca.begin(50);

    Servo motor1(pca, 0);
    Servo motor2(pca, 1);

    // The PCA9685 output is the only available position reference; standard
    // servos do not report their physical angle.
    if (!motor1.adoptPwmCount(count1)) {
        motor1.setAngle(90);
        std::cout << "motor1: no previous pulse; using 90 degrees as the base\n";
    }
    if (!motor2.adoptPwmCount(count2)) {
        motor2.setAngle(90);
        std::cout << "motor2: no previous pulse; using 90 degrees as the base\n";
    }

    const float base1 = motor1.angle();
    const float base2 = motor2.angle();
    int angle = 0;
    std::cout << "Base angles: " << base1 << ", " << base2 << "\n";

    while (true) {
        std::cout << "Enter angle offset from the startup position (degrees):\n";
        if (!(std::cin >> angle)) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Invalid input; enter a number.\n";
            continue;
        }

        moveTo(motor1, base1 + angle);
        moveTo(motor2, base2 + angle);
        sleep_ms(1000);
    }
}
