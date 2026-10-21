#include "pico/stdlib.h"
#include <iostream>
#include <stdio.h>

#include "hardware/i2c.h"

#include "pca9685.hpp"
#include "servo.hpp"

constexpr uint SDA_PIN = 4;
constexpr uint SCL_PIN = 5;

constexpr uint8_t PCA_ADDRESS = 0x40;
const uint TOUCH_PIN = 15;

// Read the pulse-end count the PCA9685 is still outputting
static uint16_t readOffCount(uint8_t ch) {
    uint8_t regL = 0x08 + 4 * ch;   // LEDn_OFF_L
    uint8_t regH = 0x09 + 4 * ch;   // LEDn_OFF_H
    uint8_t lo = 0, hi = 0;

    if (i2c_write_blocking(i2c0, PCA_ADDRESS, &regL, 1, true) < 0) return 0;
    if (i2c_read_blocking(i2c0, PCA_ADDRESS, &lo, 1, false) < 0) return 0;
    if (i2c_write_blocking(i2c0, PCA_ADDRESS, &regH, 1, true) < 0) return 0;
    if (i2c_read_blocking(i2c0, PCA_ADDRESS, &hi, 1, false) < 0) return 0;

    if (hi & 0x10) return 0;        // full-off bit set -> no pulse
    return ((hi & 0x0F) << 8) | lo;
}

// Move to (start + offset) in small steps instead of one jump
static void moveTo(Servo& s, float target, float step = 1.0f, uint delayMs = 15) {
    while (true) {
        float diff = target - s.angle();
        if (diff > -step && diff < step) break;
        s.moveRelative(diff > 0 ? step : -step);
        sleep_ms(delayMs);
    }
    s.setAngle(target);
}

int main() {
    stdio_init_all();
    i2c_init(i2c0, 100000);

    gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);

    gpio_pull_up(SDA_PIN);
    gpio_pull_up(SCL_PIN);

    sleep_ms(1000);

    // Read old pulses BEFORE begin()
    uint16_t count1 = readOffCount(0);
    uint16_t count2 = readOffCount(1);

    PCA9685 pca(i2c0, PCA_ADDRESS);
    pca.begin(50);

    Servo motor1(pca, 0);
    Servo motor2(pca, 1);

    // Adopt the old position (fallback 90 if the PCA had nothing)
    if (!motor1.adoptPwmCount(count1)) { motor1.setAngle(90); std::cout << "motor1: no old pulse, assuming 90\n"; }
    if (!motor2.adoptPwmCount(count2)) { motor2.setAngle(90); std::cout << "motor2: no old pulse, assuming 90\n"; }

    // These two numbers are now your "0" positions
    const float ZERO1 = motor1.angle();
    const float ZERO2 = motor2.angle();
    std::cout << "Zero positions: " << ZERO1 << ", " << ZERO2 << "\n";

    while (true) {
        std::cout << "Both to +20\n";
        moveTo(motor1, ZERO1 + 20);
        moveTo(motor2, ZERO2 + 20);
        sleep_ms(1000);

        std::cout << "Servo 1 to +35, servo 2 to +55\n";
        moveTo(motor1, ZERO1 + 35);
        moveTo(motor2, ZERO2 + 55);
        sleep_ms(1000);

        std::cout << "Servo 1 to +50, servo 2 to +35\n";
        moveTo(motor1, ZERO1 + 50);
        moveTo(motor2, ZERO2 + 35);
        sleep_ms(1000);
    }
}