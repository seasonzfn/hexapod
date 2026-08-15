#include "ServoDriver.hpp"
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include <stdexcept>
#include <cmath>
#include <algorithm>

namespace leg_move
{
    ServoDriver::ServoDriver(int i2cAddress)
        : address(i2cAddress)
    {
        fileDescriptor = open("/dev/i2c-1", O_RDWR);
        if (fileDescriptor < 0) {
            throw std::runtime_error("Failed to open I2C bus");
        }

        if (ioctl(fileDescriptor, I2C_SLAVE, address) < 0) {
            throw std::runtime_error("Failed to set I2C slave address");
        }

        writeRegister(0x00, 0x00); // MODE1 register, wake the chip up
        setPWMFreq(50.0);          // servos want ~50Hz
    }

    ServoDriver::~ServoDriver()
    {
        if (fileDescriptor >= 0) {
            close(fileDescriptor);
        }
    }

    void ServoDriver::writeRegister(uint8_t reg, uint8_t value)
    {
        uint8_t buffer[2] = { reg, value };
        if (write(fileDescriptor, buffer, 2) != 2) {
            throw std::runtime_error("I2C write failed");
        }
    }

    uint8_t ServoDriver::readRegister(uint8_t reg)
    {
        if (write(fileDescriptor, &reg, 1) != 1) {
            throw std::runtime_error("I2C write (register select) failed");
        }
        uint8_t value;
        if (read(fileDescriptor, &value, 1) != 1) {
            throw std::runtime_error("I2C read failed");
        }
        return value;
    }

    void ServoDriver::setPWMFreq(double freqHz)
    {
        double prescaleValue = 25000000.0 / (4096.0 * freqHz) - 1.0;
        uint8_t prescale = static_cast<uint8_t>(std::round(prescaleValue));

        uint8_t oldMode = readRegister(0x00);
        writeRegister(0x00, (oldMode & 0x7F) | 0x10); // sleep mode, to change prescale
        writeRegister(0xFE, prescale);                 // PRESCALE register
        writeRegister(0x00, oldMode);                  // restore original mode
        writeRegister(0x00, oldMode | 0x80);            // restart
    }

    void ServoDriver::setAngle(int channel, double angleRadians)
    {
        double degrees = angleRadians * 180.0 / PI;
        degrees = std::clamp(degrees, 0.0, 180.0);

        double pulseMs = 0.5 + (degrees / 180.0) * 2.0; // 0.5ms to 2.5ms range
        int ticks = static_cast<int>((pulseMs / 20.0) * 4096.0);

        uint8_t baseReg = 0x06 + 4 * channel; // LED0_ON_L is 0x06, each channel is 4 registers apart

        writeRegister(baseReg, 0);              // ON_L
        writeRegister(baseReg + 1, 0);          // ON_H
        writeRegister(baseReg + 2, ticks & 0xFF);        // OFF_L
        writeRegister(baseReg + 3, (ticks >> 8) & 0x0F); // OFF_H
    }
}