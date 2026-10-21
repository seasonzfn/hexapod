#ifndef SERVO_HPP
#define SERVO_HPP

#include <cstdint>
#include "pca9685.hpp"

class Servo {
public:
    Servo(PCA9685& driver, uint8_t channel);

    void setPulseWidth(uint16_t pulseUs);
    void setAngle(float angle);
    void moveRelative(float degrees);

private:
    PCA9685& driver_;
    uint8_t channel_;

    static constexpr uint32_t PWM_PERIOD_US = 20000;
    static constexpr uint16_t MIN_PULSE_US = 1000;
    static constexpr uint16_t MAX_PULSE_US = 2000;

    float currentAngle_ = 0.0f;
    bool adoptPwmCount(uint16_t offCount);   // true if a valid pulse was found
    float angle() const { return currentAngle_; }
};

#endif