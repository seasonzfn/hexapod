
#ifndef SERVO_H
#define SERVO_H

#include <cstdint>
#include "pca9685.h"

class Servo {
public:
    Servo(PCA9685& driver, uint8_t channel);

    void setPulseWidth(uint16_t pulseUs);
    void setAngle(float angle);

private:
    PCA9685& driver_;
    uint8_t channel_;

    static constexpr uint32_t PWM_PERIOD_US = 20000;
    static constexpr uint16_t MIN_PULSE_US = 1000;
    static constexpr uint16_t MAX_PULSE_US = 2000;
};

#endif
