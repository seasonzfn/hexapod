#define _USE_MATH_DEFINES
#include "leg.hpp"
#include <iostream>
#include <cmath>
#include "Hexapod.hpp"

int main()
{
    leg_move::Hexapod bot(18.0, 55.0, 75.0);

    for (double t = 0.0; t < 6.3; t += 1.0)
    {
        bot.update_legs(t);
        auto LF = bot.getLF();
        std::cout << "t=" << t
                  << "  FL femur=" << LF.femur * 180.0 / M_PI
                  << "  FL tibia=" << LF.tibia * 180.0 / M_PI
                  << "\n";
    }
    return 0;
}