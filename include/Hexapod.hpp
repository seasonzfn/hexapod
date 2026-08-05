#ifndef HEXAPOD_HPP
#define HEXAPOD_HPP


#include <iostream>
#include "leg.hpp"

namespace leg_move
{
    class Hexapod {
        public:
        Hexapod();
        void update_legs(double time);

        private:
            leg LF, LM, LB, RF, RM, RB;
    };
}

#endif