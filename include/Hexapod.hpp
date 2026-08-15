#ifndef HEXAPOD_HPP
#define HEXAPOD_HPP


#include <iostream>
#include "leg.hpp"

namespace leg_move
{
    enum class MotionMode {
        Stand = 0,
        WalkForward,
        WalkBackward,
        TurnLeft,
        TurnRight,
        Turn360,
        Dance,
        Stride,
    };

    class Hexapod {
        public:
        Hexapod(double l1, double l2, double l3);
        void update_legs(double time);
        void setMotionMode(int mode);
        void setWalkSpeed(double speed);
        void setStepLength(double length);
        void setTurnRate(double rate);
        void setDanceEnergy(double energy);

        // Direct motion helpers
        void walkForward(double time);
        void walkBackward(double time);
        void turnLeft(double time);
        void turnRight(double time);
        void turn360(double time);
        void dance(double time);
        void stride(double time);
        void stand(double time);

        // Bridge-compatible wrappers
        void updateAllLegs(double time) { update_legs(time); }

        JointAngles getLF() const { return LF.Cur_Angles(); }
        JointAngles getLM() const { return LM.Cur_Angles(); }
        JointAngles getLB() const { return LB.Cur_Angles(); }
        JointAngles getRF() const { return RF.Cur_Angles(); }
        JointAngles getRM() const { return RM.Cur_Angles(); }
        JointAngles getRB() const { return RB.Cur_Angles(); }

        // Alternative names expected by the Python bridge
        JointAngles getFL() const { return getLF(); }
        JointAngles getML() const { return getLM(); }
        JointAngles getBL() const { return getLB(); }
        JointAngles getFR() const { return getRF(); }
        JointAngles getMR() const { return getRM(); }
        JointAngles getBR() const { return getRB(); }

        private:
            leg LF, LM, LB, RF, RM, RB;
            MotionMode mode_{MotionMode::WalkForward};
            double walkSpeed_{0.8};
            double stepLength_{40.0};
            double stepHeight_{18.0};
            double stanceRatio_{0.6};
            double turnRate_{0.40};
            double danceEnergy_{1.0};

            void computeLegTarget(double phaseOffset, double time, double baseX, double baseY,
                                  double direction, double stepLength, double stepHeight,
                                  double stanceRatio, double& x, double& y, double& z);
            void applyGait(double time, double direction, double stepLength, double stepHeight,
                           double stanceRatio);
            void applyTurn(double time, double direction, bool spin360 = false);
            void applyDance(double time);
            void applyStand(double time);
    };
}

#endif