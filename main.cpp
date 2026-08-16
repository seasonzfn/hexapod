#include "ServoDriver.hpp"
#include "leg.hpp"
#include "constants.hpp"
#include <thread>
#include <iostream>
#include <chrono>



int main(){

    try{
        leg_move::ServoDriver driver;
        auto start = std::chrono::steady_clock::now();

        int laststep = -1;
        
        while(true)
        {
            auto now = std::chrono::steady_clock::now();
            double elapsedseconds = std::chrono::duration<double>(now - start).count();

            int currentstep = static_cast<int>(elapsedseconds) % 4; 

            if(currentstep != laststep)
            {
                if(currentstep == 0) {
                    std::cout << "Step 0: Move leg to position A" << std::endl;
                    driver.setAngle(0, leg_move::PI); // Example angle for motor A
                }
                else if(currentstep == 1) {
                    std::cout << "Step 1: Move leg to position B" << std::endl;
                    driver.setAngle(1, leg_move::PI); // Example angle for motor B
                }
                else if(currentstep == 2) {
                    std::cout << "Step 2: Move leg to position C" << std::endl;
                    driver.setAngle(0, 0);
                }
                else if(currentstep == 3) {
                    std::cout << "Step 3: Move leg to position D" << std::endl;
                    driver.setAngle(1, 0);
                }
                laststep = currentstep;
            }
        
            std::cout << "Elapsed time: " << elapsedseconds << " seconds" << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
    catch(const std::exception& e){
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;

    }
    return 0;


}