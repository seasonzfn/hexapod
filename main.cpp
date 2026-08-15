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

        std::cout << "Starting continuous test: all channels forward\n";
        // Example: continuous-rotation servos typically treat ~1.5ms as stop,
        // >1.5ms forward, <1.5ms reverse. Adjust the values for your hardware.
        double forwardMs = 2.0; // forward pulse width (ms)
        double stopMs = 1.5;    // stop pulse width (ms)

        // Set all channels to forward pulse width and hold for 5 seconds
        driver.setAllPulseMs(forwardMs);
        std::this_thread::sleep_for(std::chrono::seconds(5));

        // Stop all channels
        driver.setAllPulseMs(stopMs);

        std::cout << "Continuous test complete.\n";
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}