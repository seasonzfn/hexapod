#ifndef LEG_HPP
#define LEG_HPP

#include <iostream>

namespace leg_move
{
    struct JointAngles {
        JointAngles(double cox, double fem, double tib) : coxa(cox) , femur(fem) , tibia(tib) {}

        double coxa; //rotation of each
        double femur;
        double tibia;
    };

    class leg {
    public:
        leg(double l1, double l2, double l3) : L1(l1), L2(l2), L3(l3) {}
        JointAngles solverIK(double x, double y, double z);

    private:
        double L1 , L2 , L3; //LENGTH OF EACH BONES
        JointAngles Cur_Ang{0.0, 0.0, 0.0};

        double law_of_cosines(double a, double b, double c);

    };
}

#endif