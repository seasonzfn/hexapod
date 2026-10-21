
#include "servo.hpp"

#include <algorithm>

Servo::Servo(PCA9685& driver, uint8_t channel)
    : driver_(driver), channel_(channel) {
}

void Servo::setPulseWidth(uint16_t pulseUs) {
    pulseUs = std::clamp(
        pulseUs,
        MIN_PULSE_US,
        MAX_PULSE_US
    );

    uint16_t pulseCount = static_cast<uint16_t>(
        (static_cast<uint32_t>(pulseUs) * 4096u
            + PWM_PERIOD_US / 2)
        / PWM_PERIOD_US
    );

    driver_.setPWM(channel_, 0, pulseCount);
}

void Servo::setAngle(float angle) {
    angle = std::clamp(angle, 0.0f, 180.0f);

    float pulse = MIN_PULSE_US +
        (angle / 180.0f) * (MAX_PULSE_US - MIN_PULSE_US);

    setPulseWidth(static_cast<uint16_t>(pulse + 0.5f));
}
