
#include "pca9685.hpp"
#include <stdio.h>
#include "pico/stdlib.h"

namespace {
constexpr uint8_t MODE1 = 0x00;
constexpr uint8_t PRESCALE = 0xFE;
constexpr uint8_t LED0_ON_L = 0x06;
}

PCA9685::PCA9685(i2c_inst_t* i2c, uint8_t address)
    : i2c_(i2c), address_(address) {
}

void PCA9685::writeRegister(uint8_t reg, uint8_t value) {
    uint8_t data[] = {reg, value};

    int result = i2c_write_blocking(
        i2c_,
        address_,
        data,
        2,
        false
    );

    if (result != 2) {
        printf("PCA9685: I2C register write failed\n");
    }
}

uint8_t PCA9685::readRegister(uint8_t reg) {
    uint8_t value = 0;

    int written = i2c_write_blocking(
        i2c_,
        address_,
        &reg,
        1,
        true
    );

    if (written != 1) {
        printf("PCA9685: I2C register selection failed\n");
        return 0;
    }

    int received = i2c_read_blocking(
        i2c_,
        address_,
        &value,
        1,
        false
    );

    if (received != 1) {
        printf("PCA9685: I2C register read failed\n");
        return 0;
    }

    return value;
}

void PCA9685::begin(uint32_t frequency) {
    if (frequency == 0 || frequency > 1526) {
        printf("PCA9685: Invalid PWM frequency\n");
        return;
    }

    // Calculate the prescale from the nominal 25 MHz
    // internal oscillator frequency.
    uint32_t prescaleValue =
        (25000000u + (2048u * frequency)) /
        (4096u * frequency);

    if (prescaleValue == 0) {
        printf("PCA9685: Invalid prescale\n");
        return;
    }

    --prescaleValue;

    if (prescaleValue < 3 || prescaleValue > 255) {
        printf("PCA9685: Prescale out of range\n");
        return;
    }

    uint8_t oldMode = readRegister(MODE1);
    uint8_t sleepMode = (oldMode & 0x7F) | 0x10;

    // Enter sleep mode before changing the prescaler.
    writeRegister(MODE1, sleepMode);
    writeRegister(PRESCALE, static_cast<uint8_t>(prescaleValue));

    // Restore the previous mode and allow the oscillator to start.
    writeRegister(MODE1, oldMode & ~0x10);
    sleep_ms(5);

    // Enable auto-increment and restart PWM operation.
    writeRegister(MODE1, (oldMode & ~0x10) | 0xA0);
}

void PCA9685::setPWM(
    uint8_t channel,
    uint16_t onCount,
    uint16_t offCount
) {
    if (channel > 15 || onCount > 4095 || offCount > 4095) {
        printf("PCA9685: Invalid PWM values\n");
        return;
    }

    uint8_t reg = LED0_ON_L + 4 * channel;

    uint8_t data[] = {
        reg,
        static_cast<uint8_t>(onCount & 0xFF),
        static_cast<uint8_t>((onCount >> 8) & 0x0F),
        static_cast<uint8_t>(offCount & 0xFF),
        static_cast<uint8_t>((offCount >> 8) & 0x0F)
    };

    int result = i2c_write_blocking(
        i2c_,
        address_,
        data,
        sizeof(data),
        false
    );

    if (result != sizeof(data)) {
        printf("PCA9685: PWM write failed on channel %u\n", channel);
    }
}
