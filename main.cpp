#include "ServoDriver.hpp"
#include "leg.hpp"
#include "constants.hpp"
#include <iostream>

int main()
{
    try {
        std::cout << "Opening I2C connection to PCA9685...\n";
        leg_move::ServoDriver driver;

        std::cout << "Moving channel 0 to 90 degrees...\n";
        driver.setAngle(0, leg_move::PI / 2.0); // 90 degrees, in radians

        std::cout << "Command sent. Check the servo.\n";
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }

    return 0;
}