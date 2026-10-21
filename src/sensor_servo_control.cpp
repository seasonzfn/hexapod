#include "sensor_servo_control.hpp"

#include "pico/stdlib.h"
#include "hardware/i2c.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>

#include "pca9685.hpp"
#include "sensors.hpp"
#include "servo.hpp"

namespace {
constexpr uint SDA_PIN = 4;
constexpr uint SCL_PIN = 5;
constexpr uint TOUCH_PIN = 15;
constexpr bool TOUCH_ACTIVE_HIGH = true;

constexpr uint8_t PCA_ADDRESS = 0x40;
constexpr uint8_t MOTOR_1_CHANNEL = 0;
constexpr uint8_t MOTOR_2_CHANNEL = 1;

constexpr float MAX_ANGLE_OFFSET = 45.0f;
constexpr float DEGREES_PER_CHANGE = 2.0f;
constexpr float TEMPERATURE_DEADBAND_C = 0.5f;
constexpr float LIGHT_DEADBAND_LUX = 5.0f;
constexpr float LIGHT_DEADBAND_RATIO = 0.05f;
constexpr uint32_t SAMPLE_INTERVAL_MS = 500;

uint16_t readServoPosition(uint8_t channel) {
    const uint8_t lowRegister = 0x08 + 4 * channel;
    const uint8_t highRegister = 0x09 + 4 * channel;
    uint8_t lowByte = 0;
    uint8_t highByte = 0;

    if (i2c_write_blocking(i2c0, PCA_ADDRESS, &lowRegister, 1, true) != 1 ||
        i2c_read_blocking(i2c0, PCA_ADDRESS, &lowByte, 1, false) != 1 ||
        i2c_write_blocking(i2c0, PCA_ADDRESS, &highRegister, 1, true) != 1 ||
        i2c_read_blocking(i2c0, PCA_ADDRESS, &highByte, 1, false) != 1) {
        return 0;
    }

    if (highByte & 0x10) return 0;
    return static_cast<uint16_t>(((highByte & 0x0F) << 8) | lowByte);
}

void moveServoTo(Servo& servo, float targetAngle) {
    targetAngle = std::clamp(targetAngle, 0.0f, 180.0f);
    constexpr float STEP_DEGREES = 1.0f;

    while (std::fabs(targetAngle - servo.angle()) > STEP_DEGREES) {
        const float difference = targetAngle - servo.angle();
        servo.moveRelative(difference > 0.0f ? STEP_DEGREES : -STEP_DEGREES);
        sleep_ms(15);
    }
    servo.setAngle(targetAngle);
}

float lightDeadband(float reference) {
    return std::max(LIGHT_DEADBAND_LUX,
                    std::fabs(reference) * LIGHT_DEADBAND_RATIO);
}
}

void runSensorServoControl() {
    i2c_init(i2c0, 100000);
    gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(SDA_PIN);
    gpio_pull_up(SCL_PIN);

    const uint16_t motor1Position = readServoPosition(MOTOR_1_CHANNEL);
    const uint16_t motor2Position = readServoPosition(MOTOR_2_CHANNEL);

    PCA9685 pca(i2c0, PCA_ADDRESS);
    pca.begin(50);
    Servo motor1(pca, MOTOR_1_CHANNEL);
    Servo motor2(pca, MOTOR_2_CHANNEL);

    if (!motor1.adoptPwmCount(motor1Position)) {
        motor1.setAngle(90.0f);
        printf("Motor 1 has no saved position; starting at 90 degrees.\n");
    }
    if (!motor2.adoptPwmCount(motor2Position)) {
        motor2.setAngle(90.0f);
        printf("Motor 2 has no saved position; starting at 90 degrees.\n");
    }

    const float baseAngle1 = motor1.angle();
    const float baseAngle2 = motor2.angle();
    const float positiveOffsetLimit = std::min(
        {MAX_ANGLE_OFFSET, 180.0f - baseAngle1, baseAngle2});
    const float negativeOffsetLimit = std::min(
        {MAX_ANGLE_OFFSET, baseAngle1, 180.0f - baseAngle2});
    float angleOffset = 0.0f;

    Sensors sensors(i2c0);
    TouchButton touch(TOUCH_PIN, TOUCH_ACTIVE_HIGH);
    touch.begin();

    if (!sensors.beginLight() || sensors.lightPartId() != 0xB3) {
        printf("Warning: light sensor initialization failed (part ID 0x%02X).\n",
               sensors.lightPartId());
    }

    bool temperatureMode = true;
    bool referenceValid = false;
    float sensorReference = 0.0f;
    float reading = 0.0f;

    printf("Touch GP15 to switch between temperature and light control.\n");
    printf("Motor 1 increases and motor 2 decreases by up to %.0f degrees.\n",
           MAX_ANGLE_OFFSET);

    uint32_t lastSample = to_ms_since_boot(get_absolute_time());

    while (true) {
        if (touch.wasPressed()) {
            temperatureMode = !temperatureMode;
            referenceValid = false;
            printf("Mode: %s\n", temperatureMode ? "temperature" : "light");
        }

        const uint32_t now = to_ms_since_boot(get_absolute_time());
        if (now - lastSample >= SAMPLE_INTERVAL_MS) {
            lastSample = now;

            const bool readOk = temperatureMode
                ? sensors.readTemperature(reading)
                : sensors.readLight(reading);
            if (!readOk) {
                printf("%s read error; motor positions unchanged.\n",
                       temperatureMode ? "Temperature" : "Light");
                sleep_ms(10);
                continue;
            }

            const float deadband = temperatureMode
                ? TEMPERATURE_DEADBAND_C
                : lightDeadband(sensorReference);

            if (!referenceValid) {
                sensorReference = reading;
                referenceValid = true;
                printf("%s reference: %.2f\n",
                       temperatureMode ? "Temperature (C)" : "Light (lux)",
                       reading);
            } else {
                const float difference = reading - sensorReference;
                const float magnitude = std::fabs(difference);
                if (magnitude >= deadband) {
                    const float possibleSteps = std::floor(magnitude / deadband);
                    const float availableOffset =
                        difference > 0.0f
                            ? positiveOffsetLimit - angleOffset
                            : negativeOffsetLimit + angleOffset;
                    const float maxSteps = std::ceil(
                        std::max(0.0f, availableOffset) / DEGREES_PER_CHANGE);
                    const float steps = std::min(possibleSteps, maxSteps);

                    if (steps > 0.0f) {
                        const float direction = difference > 0.0f ? 1.0f : -1.0f;
                        const float angleChange =
                            direction * steps * DEGREES_PER_CHANGE;
                        angleOffset = std::clamp(
                            angleOffset + angleChange,
                            -negativeOffsetLimit,
                            positiveOffsetLimit);

                        moveServoTo(motor1, baseAngle1 + angleOffset);
                        moveServoTo(motor2, baseAngle2 - angleOffset);
                    }

                    sensorReference +=
                        (difference > 0.0f ? 1.0f : -1.0f) * steps * deadband;
                }

                printf("%s: %.2f, motor angles: %.1f / %.1f\n",
                       temperatureMode ? "Temperature (C)" : "Light (lux)",
                       reading, motor1.angle(), motor2.angle());
            }
        }

        sleep_ms(10);
    }
}
