#define _USE_MATH_DEFINES
#include "leg.hpp"
#include <iostream>
#include <cmath>


int main()
{

    leg_move::leg testLeg(18.0, 55.0, 75.0);
    leg_move::JointAngles result = testLeg.solverIK(90.0, 0.0, 0.0);

    std::cout << "=== Test 1: reachable target ===\n";
    std::cout << "coxa:  " << result.coxa  * 180.0 / M_PI << " deg\n";
    std::cout << "femur: " << result.femur * 180.0 / M_PI << " deg\n";
    std::cout << "tibia: " << result.tibia * 180.0 / M_PI << " deg\n";

    std::cout << "\n=== Test 2: unreachable target ===\n";
    leg_move::JointAngles result2 = testLeg.solverIK(500.0, 0.0, 0.0);
    std::cout << "coxa:  " << result2.coxa  * 180.0 / M_PI << " deg\n";
    std::cout << "femur: " << result2.femur * 180.0 / M_PI << " deg\n";
    std::cout << "tibia: " << result2.tibia * 180.0 / M_PI << " deg\n";

    return 0;
}