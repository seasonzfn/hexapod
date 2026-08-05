#include "Hexapod.hpp"
#include <cmath>

namespace leg_move
{
	Hexapod::Hexapod(double l1, double l2, double l3)
    : LF(l1, l2, l3), LM(l1, l2, l3), LB(l1, l2, l3),
      RF(l1, l2, l3), RM(l1, l2, l3), RB(l1, l2, l3)
    {}

	void Hexapod::computeLegTarget(double phaseOffset, double time, double& x, double& y, double& z)
	{
		double stepLength = 30.0;
		double stepHeight = 12.0;
		double baseX = 80.0;
		double baseZ = 0.0;

		double phase = time + phaseOffset;
        double liftPhase = std::sin(phase);


		x = baseX + liftPhase * stepLength;
        y = 0.0;
        z = (liftPhase > 0) ? (baseZ + liftPhase * stepHeight) : baseZ;
	}

	void Hexapod::update_legs(double time)
	{
		double x, y, z;
		const double offsets[6] = {0.0, 2.0, 4.0, M_PI, M_PI + 2.0, M_PI + 4.0};

		computeLegTarget(offsets[0], time, x, y, z);
		LF.solverIK(x, y, z);

		computeLegTarget(offsets[1], time, x, y, z);
		LM.solverIK(x, y, z);

		computeLegTarget(offsets[2], time, x, y, z);
		LB.solverIK(x, y, z);

		computeLegTarget(offsets[3], time, x, y, z);
		RF.solverIK(x, y, z);

		computeLegTarget(offsets[4], time, x, y, z);
		RM.solverIK(x, y, z);

		computeLegTarget(offsets[5], time, x, y, z);
		RB.solverIK(x, y, z);
	}
}
