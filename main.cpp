#include "pico/stdlib.h"
#include "pico/flash.h"
#include "hardware/i2c.h"
#include "hardware/flash.h"
#include <cstdio>
#include <cstring>

#include "pca9685.hpp"
#include "servo.hpp"

constexpr uint SDA_PIN = 4;
constexpr uint SCL_PIN = 5;
constexpr uint8_t PCA_ADDRESS = 0x40;

constexpr uint32_t CALIB_MAGIC = 0x48455841;  // "HEXA"
constexpr uint32_t CALIB_FLASH_OFFSET = PICO_FLASH_SIZE_BYTES - FLASH_SECTOR_SIZE;
constexpr int NUM_SERVOS = 2;

struct Calib {
    uint32_t magic;
    float zero[NUM_SERVOS];
};

static Calib g_calib;

// ---- flash helpers ----
static void load_calib() {
    const Calib* stored = (const Calib*)(XIP_BASE + CALIB_FLASH_OFFSET);
    if (stored->magic == CALIB_MAGIC) {
        g_calib = *stored;
    } else {
        g_calib.magic = CALIB_MAGIC;
        for (int i = 0; i < NUM_SERVOS; i++) g_calib.zero[i] = 90.0f;  // default
    }
}

static void do_save(void*) {
    uint8_t page[FLASH_PAGE_SIZE] = {0xFF};
    memset(page, 0xFF, sizeof(page));
    memcpy(page, &g_calib, sizeof(g_calib));
    flash_range_erase(CALIB_FLASH_OFFSET, FLASH_SECTOR_SIZE);
    flash_range_program(CALIB_FLASH_OFFSET, page, FLASH_PAGE_SIZE);
}

static void save_calib() {
    flash_safe_execute(do_save, nullptr, 1000);
}

int main() {
    stdio_init_all();
    i2c_init(i2c0, 100000);
    gpio_set_function(SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(SDA_PIN);
    gpio_pull_up(SCL_PIN);
    sleep_ms(1000);

    load_calib();

    PCA9685 pca(i2c0, PCA_ADDRESS);
    pca.begin(50);

    Servo servos[NUM_SERVOS] = { Servo(pca, 0), Servo(pca, 1) };

    // logical angle -> physical angle
    auto move = [&](int i, float logical) {
        servos[i].setAngle(g_calib.zero[i] + logical);
    };

    // ---- calibration mode ----
    // 1/2 select servo, a/d nudge -/+ 1 degree, s saves, q finishes
    printf("Calibration: 1/2 = select servo, a/d = -/+1 deg, s = save, q = start program\n");
    int sel = 0;
    for (int i = 0; i < NUM_SERVOS; i++) servos[i].setAngle(g_calib.zero[i]);

    bool calibrating = true;
    while (calibrating) {
        int c = getchar_timeout_us(100000);  // wait up to 100 ms
        if (c == PICO_ERROR_TIMEOUT) continue;

        switch (c) {
            case '1': sel = 0; printf("Servo 1 selected\n"); break;
            case '2': sel = 1; printf("Servo 2 selected\n"); break;
            case 'a': g_calib.zero[sel] -= 1.0f; servos[sel].setAngle(g_calib.zero[sel]); break;
            case 'd': g_calib.zero[sel] += 1.0f; servos[sel].setAngle(g_calib.zero[sel]); break;
            case 's': save_calib(); printf("Saved: %.1f, %.1f\n", g_calib.zero[0], g_calib.zero[1]); break;
            case 'q': calibrating = false; break;
        }
    }

    // ---- main program uses logical angles (0 = your calibrated zero) ----
    while (true) {
        printf("Both to 0\n");
        move(0, 0);   move(1, 0);
        sleep_ms(1000);

        printf("Servo 1 +45, servo 2 -45\n");
        move(0, 45);  move(1, -45);
        sleep_ms(1000);

        printf("Servo 1 -45, servo 2 +45\n");
        move(0, -45); move(1, 45);
        sleep_ms(1000);
    }
}