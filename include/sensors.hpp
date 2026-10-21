#pragma once

#include "pico/stdlib.h"
#include "hardware/i2c.h"

// Change the temperature address here after running the I2C scan.
constexpr uint8_t SENSOR_TEMP_ADDR  = 0x48;   // LM75B
constexpr uint8_t SENSOR_LIGHT_ADDR = 0x52;   // APDS-9306-065

class Sensors {
public:
    explicit Sensors(i2c_inst_t* i2c,
                     uint8_t tempAddr = SENSOR_TEMP_ADDR,
                     uint8_t lightAddr = SENSOR_LIGHT_ADDR);

    void scan() const;                       // prints every device on the bus
    bool beginLight();                       // checks part id, sets 20-bit mode
    bool readTemperature(float& tempC) const;   // 11-bit, 0.125 C steps
    bool readLight(float& lux) const;           // 20-bit
    uint8_t lightPartId() const { return partId_; }

private:
    bool writeReg(uint8_t addr, uint8_t reg, uint8_t value) const;
    bool readRegs(uint8_t addr, uint8_t reg, uint8_t* dst, size_t n) const;

    i2c_inst_t* i2c_;
    uint8_t tempAddr_;
    uint8_t lightAddr_;
    uint8_t partId_ = 0;
};

class TouchButton {
public:
    TouchButton(uint pin, bool activeHigh = true, uint32_t debounceMs = 50);

    void begin();
    bool isTouched() const;     // current raw state
    bool wasPressed();          // true once per press (call often)

private:
    uint pin_;
    bool activeHigh_;
    uint32_t debounceMs_;
    bool last_ = false;
    uint32_t lastChange_ = 0;
};