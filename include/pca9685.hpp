
#ifndef PCA9685_HPP
#define PCA9685_HPP

#include <cstdint>
#include "hardware/i2c.h"

class PCA9685 {
public:
    PCA9685(i2c_inst_t* i2c, uint8_t address);

    void begin(uint32_t frequency = 50);
    void setPWM(uint8_t channel,
                uint16_t onCount,
                uint16_t offCount);

private:
    i2c_inst_t* i2c_;
    uint8_t address_;

    void writeRegister(uint8_t reg, uint8_t value);
    uint8_t readRegister(uint8_t reg);
};

#endif
