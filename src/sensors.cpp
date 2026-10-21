#include "sensors.hpp"

#include <cstdio>

// APDS-9306 registers
static constexpr uint8_t ALS_MAIN_CTRL = 0x00;
static constexpr uint8_t ALS_MEAS_RATE = 0x04;
static constexpr uint8_t ALS_GAIN      = 0x05;
static constexpr uint8_t ALS_PART_ID   = 0x06;
static constexpr uint8_t ALS_DATA_0    = 0x0D;

static constexpr float ALS_GAIN_X   = 3.0f;   // matches ALS_GAIN = 0x01
static constexpr float ALS_INT_FACT = 4.0f;   // 400 ms / 100 ms. VERIFY in datasheet.

Sensors::Sensors(i2c_inst_t* i2c, uint8_t tempAddr, uint8_t lightAddr)
    : i2c_(i2c), tempAddr_(tempAddr), lightAddr_(lightAddr) {
}

bool Sensors::writeReg(uint8_t addr, uint8_t reg, uint8_t value) const {
    const uint8_t buf[2] = {reg, value};
    return i2c_write_blocking(i2c_, addr, buf, 2, false) == 2;
}

bool Sensors::readRegs(uint8_t addr, uint8_t reg, uint8_t* dst, size_t n) const {
    if (i2c_write_blocking(i2c_, addr, &reg, 1, true) != 1) return false;
    return i2c_read_blocking(i2c_, addr, dst, n, false) == static_cast<int>(n);
}

void Sensors::scan() const {
    printf("I2C scan:\n");
    for (uint8_t a = 0x08; a < 0x78; a++) {
        uint8_t dummy;
        if (i2c_read_blocking(i2c_, a, &dummy, 1, false) >= 0) {
            printf("  found device at 0x%02X\n", a);
        }
    }
}

bool Sensors::beginLight() {
    if (!readRegs(lightAddr_, ALS_PART_ID, &partId_, 1)) return false;
    bool ok = writeReg(lightAddr_, ALS_MEAS_RATE, 0x04);   // 20-bit, 500 ms
    ok = writeReg(lightAddr_, ALS_GAIN, 0x01) && ok;       // gain 3x
    ok = writeReg(lightAddr_, ALS_MAIN_CTRL, 0x02) && ok;  // ALS enable
    return ok;
}

bool Sensors::readTemperature(float& tempC) const {
    uint8_t d[2];
    if (!readRegs(tempAddr_, 0x00, d, 2)) return false;
    int16_t raw = static_cast<int16_t>((d[0] << 8) | d[1]);
    raw >>= 5;                       // 11-bit two's complement
    tempC = raw * 0.125f;
    return true;
}

bool Sensors::readLight(float& lux) const {
    uint8_t d[3];
    if (!readRegs(lightAddr_, ALS_DATA_0, d, 3)) return false;
    const uint32_t raw = d[0] | (d[1] << 8) | ((d[2] & 0x0F) << 16);
    lux = raw / (ALS_GAIN_X * ALS_INT_FACT) * 0.6f;
    return true;
}



TouchButton::TouchButton(uint pin, bool activeHigh, uint32_t debounceMs)
    : pin_(pin), activeHigh_(activeHigh), debounceMs_(debounceMs) {
}

void TouchButton::begin() {
    gpio_init(pin_);
    gpio_set_dir(pin_, GPIO_IN);
    if (activeHigh_) gpio_pull_down(pin_);
    else             gpio_pull_up(pin_);
}

bool TouchButton::isTouched() const {
    return gpio_get(pin_) == activeHigh_;
}

bool TouchButton::wasPressed() {
    const bool touched = isTouched();
    const uint32_t now = to_ms_since_boot(get_absolute_time());
    if (touched == last_) return false;
    if (now - lastChange_ < debounceMs_) return false;
    last_ = touched;
    lastChange_ = now;
    return touched;
}