#ifndef HEXAPOD_HPP
#define HEXAPOD_HPP


#include <iostream>
#include "leg.hpp"

namespace leg_move
{
    class Hexapod {
        public:
        Hexapod(double l1, double l2, double l3);
        void update_legs(double time);

        JointAngles getLF() const { return LF.Cur_Angles(); }
        JointAngles getLM() const { return LM.Cur_Angles(); }
        JointAngles getLB() const { return LB.Cur_Angles(); }
        JointAngles getRF() const { return RF.Cur_Angles(); }
        JointAngles getRM() const { return RM.Cur_Angles(); }
        JointAngles getRB() const { return RB.Cur_Angles(); }   

        private:
            leg LF, LM, LB, RF, RM, RB;
            void computeLegTarget(double phaseOffset, double time, double& x, double& y, double& z);
    };
}

#endif