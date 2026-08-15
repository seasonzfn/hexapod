#include "ServoDriver.hpp"
#include "leg.hpp"
#include "constants.hpp"
#include <iostream>
#include <thread>
#include <chrono>

int main()
{
    try {
        std::cout << "Opening I2C connection to PCA9685...\n";
        leg_move::ServoDriver driver;

        std::cout << "Starting per-channel cycle test\n";

        // Pulse widths (ms). Adjust for your hardware. 1.5ms is usually stop/neutral.
        double forwardMs = 2.0;
        double stopMs = 1.5;

        // Duration for one "cycle" (seconds). For continuous-rotation servos
        // a "cycle" is defined here as running the motor for this duration.
        int cycleSeconds = 5;

        auto runChannelCycle = [&](int channel, double runPulseMs, int seconds){
            std::cout << "Running channel " << channel << " for " << seconds << "s\n";
            driver.setPulseMs(channel, runPulseMs);
            std::this_thread::sleep_for(std::chrono::seconds(seconds));
            driver.setPulseMs(channel, stopMs);
            std::cout << "Channel " << channel << " stopped\n";
        };

        // Run channel 0, then channel 1 sequentially
        runChannelCycle(0, forwardMs, cycleSeconds);
        runChannelCycle(1, forwardMs, cycleSeconds);

        std::cout << "Per-channel cycle test complete.\n";
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}