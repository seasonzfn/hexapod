#ifndef SERVODRIVER_HPP
#define SERVODRIVER_HPP
#include <cstdint>
#include "constants.hpp"

namespace leg_move
{
    class ServoDriver {
    public:
        ServoDriver(int i2cAddress = 0x40);
        ~ServoDriver();

        void setAngle(int channel, double angleRadians);
        // Set pulse width in milliseconds for a specific channel (useful for continuous-rotation servos)
        void setPulseMs(int channel, double pulseMs);
        // Set same pulse width on all channels at once
        void setAllPulseMs(double pulseMs);

    private:
        int fileDescriptor;
        int address;

        void writeRegister(uint8_t reg, uint8_t value);
        uint8_t readRegister(uint8_t reg);
        void setPWMFreq(double freqHz);
    };
}

#endif