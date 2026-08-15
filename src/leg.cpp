#include "leg.hpp"
#include <cmath>
#include <algorithm>
#include <iostream>

namespace {
    constexpr double PI = 3.14159265358979323846;
}


namespace leg_move
{
    double leg::law_of_cosines(double a, double b, double c) {
        double cosA = (b * b + c * c - a * a) / (2.0 * b * c);

        cosA = std::clamp(cosA, -1.0, 1.0); // Clamp to avoid domain errors

        return acos(cosA);
    }
    JointAngles leg::solverIK(double x, double y, double z)
    {
        double coxa_angle = atan2(y, x);
        double r = sqrt(x * x + y * y) - L1; // Distance
        double s = z; // Height

        double d = sqrt(r * r + s * s); // Distance from femur to tibia

        double max = L2 + L3;
        double min = std::abs(L2 - L3);
        if (d > max || d < min) {
            std::cerr << "Target position is out of reach." << std::endl;
            return Cur_Ang; // Return current angles if out of reach
        }
        double femur_angle = law_of_cosines(L3, L2, d) + atan2(s, r);
        double tibia_angle = PI - law_of_cosines(d, L2, L3);

        Cur_Ang.coxa = coxa_angle;
        Cur_Ang.femur = femur_angle;
        Cur_Ang.tibia = tibia_angle;

        return Cur_Ang;
    }
}