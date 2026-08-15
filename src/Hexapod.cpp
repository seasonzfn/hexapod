#include "Hexapod.hpp"
#include <cmath>
#include <algorithm>

namespace leg_move
{
	static constexpr double PI = 3.14159265358979323846;

	Hexapod::Hexapod(double l1, double l2, double l3)
    : LF(l1, l2, l3), LM(l1, l2, l3), LB(l1, l2, l3),
      RF(l1, l2, l3), RM(l1, l2, l3), RB(l1, l2, l3)
    {}

	void Hexapod::setMotionMode(int mode)
	{
		if (mode < 0 || mode > static_cast<int>(MotionMode::Stride)) {
			mode_ = MotionMode::Stand;
		} else {
			mode_ = static_cast<MotionMode>(mode);
		}
	}

	void Hexapod::setWalkSpeed(double speed) { walkSpeed_ = std::clamp(speed, 0.1, 3.0); }
	void Hexapod::setStepLength(double length) { stepLength_ = std::clamp(length, 10.0, 80.0); }
	void Hexapod::setTurnRate(double rate) { turnRate_ = std::clamp(rate, 0.1, 2.0); }
	void Hexapod::setDanceEnergy(double energy) { danceEnergy_ = std::clamp(energy, 0.2, 3.0); }

	void Hexapod::walkForward(double time) { applyGait(time, 1.0, stepLength_, stepHeight_, stanceRatio_); }
	void Hexapod::walkBackward(double time) { applyGait(time, -1.0, stepLength_, stepHeight_, stanceRatio_); }
	void Hexapod::turnLeft(double time) { applyTurn(time, -1.0, false); }
	void Hexapod::turnRight(double time) { applyTurn(time, 1.0, false); }
	void Hexapod::turn360(double time) { applyTurn(time, 1.0, true); }
	void Hexapod::dance(double time) { applyDance(time); }
	void Hexapod::stride(double time) { applyGait(time, 1.0, stepLength_ * 1.5, stepHeight_ * 1.0, 0.55); }
	void Hexapod::stand(double time) { applyStand(time); }

	void Hexapod::computeLegTarget(double phaseOffset, double time, double baseX, double baseY,
	                              double direction, double stepLength, double stepHeight,
	                              double stanceRatio, double& x, double& y, double& z)
	{
		double phase = std::fmod(time * walkSpeed_ + phaseOffset, 1.0);
		if (phase < 0.0)
			phase += 1.0;

		y = baseY;
		double frontX = baseX + stepLength * 0.5 * direction;
		double backX = baseX - stepLength * 0.5 * direction;

		if (phase < stanceRatio) {
			double t = phase / stanceRatio;
			x = frontX - (frontX - backX) * t;
			z = -10.0;
		} else {
			double t = (phase - stanceRatio) / (1.0 - stanceRatio);
			x = backX + (frontX - backX) * t;
			z = -10.0 + stepHeight * std::sin(PI * t);
		}
	}

	void Hexapod::applyGait(double time, double direction, double stepLength, double stepHeight,
	                       double stanceRatio)
	{
		double x, y, z;
		const double phaseOffsets[6] = {0.0, 0.5, 0.0, 0.5, 0.0, 0.5};
		const double baseX[6] = {80.0, 60.0, 40.0, 80.0, 60.0, 40.0};
		const double baseY[6] = {35.0, 35.0, 35.0, -35.0, -35.0, -35.0};

		computeLegTarget(phaseOffsets[0], time, baseX[0], baseY[0], direction, stepLength, stepHeight, stanceRatio, x, y, z);
		LF.solverIK(x, y, z);

		computeLegTarget(phaseOffsets[1], time, baseX[1], baseY[1], direction, stepLength, stepHeight, stanceRatio, x, y, z);
		LM.solverIK(x, y, z);

		computeLegTarget(phaseOffsets[2], time, baseX[2], baseY[2], direction, stepLength, stepHeight, stanceRatio, x, y, z);
		LB.solverIK(x, y, z);

		computeLegTarget(phaseOffsets[3], time, baseX[3], baseY[3], direction, stepLength, stepHeight, stanceRatio, x, y, z);
		RF.solverIK(x, y, z);

		computeLegTarget(phaseOffsets[4], time, baseX[4], baseY[4], direction, stepLength, stepHeight, stanceRatio, x, y, z);
		RM.solverIK(x, y, z);

		computeLegTarget(phaseOffsets[5], time, baseX[5], baseY[5], direction, stepLength, stepHeight, stanceRatio, x, y, z);
		RB.solverIK(x, y, z);
	}

	void Hexapod::applyTurn(double time, double direction, bool spin360)
	{
		double x, y, z;
		const double phaseOffsets[6] = {0.0, 0.5, 0.0, 0.5, 0.0, 0.5};
		const double baseX[6] = {60.0, 50.0, 40.0, 60.0, 50.0, 40.0};
		const double baseY[6] = {30.0, 30.0, 30.0, -30.0, -30.0, -30.0};
		const double sideFactor[6] = {1.0, 1.0, 1.0, -1.0, -1.0, -1.0};
		double turnPower = direction * turnRate_;
		const double stepLength = stepLength_ * 0.7;
		const double stepHeight = stepHeight_ * 0.9;
		const double stanceRatio = 0.55;

		for (int i = 0; i < 6; ++i) {
			double phaseOffset = phaseOffsets[i];
			double adjustedY = baseY[i] + 10.0 * sideFactor[i] * turnPower;
			computeLegTarget(phaseOffset, time, baseX[i], adjustedY, sideFactor[i] * direction, stepLength, stepHeight, stanceRatio, x, y, z);
			if (spin360) {
				x += 6.0 * direction * std::sin(time * PI);
			}
			if (i == 0) LF.solverIK(x, y, z);
			if (i == 1) LM.solverIK(x, y, z);
			if (i == 2) LB.solverIK(x, y, z);
			if (i == 3) RF.solverIK(x, y, z);
			if (i == 4) RM.solverIK(x, y, z);
			if (i == 5) RB.solverIK(x, y, z);
		}
	}

	void Hexapod::applyDance(double time)
	{
		double x, y, z;
		const double baseX[6] = {70.0, 55.0, 40.0, 70.0, 55.0, 40.0};
		const double baseY[6] = {35.0, 35.0, 35.0, -35.0, -35.0, -35.0};
		const double phaseOffsets[6] = {0.0, 0.25, 0.5, 0.5, 0.75, 0.0};
		for (int i = 0; i < 6; ++i) {
			double phase = std::fmod(time * walkSpeed_ + phaseOffsets[i], 1.0);
			if (phase < 0.0) phase += 1.0;
			double footLift = danceEnergy_ * 8.0 * std::max(0.0, std::sin(PI * phase));
			x = baseX[i] + 8.0 * std::sin(2.0 * PI * phase);
			y = baseY[i];
			z = -10.0 + footLift;
			if (i == 0) LF.solverIK(x, y, z);
			if (i == 1) LM.solverIK(x, y, z);
			if (i == 2) LB.solverIK(x, y, z);
			if (i == 3) RF.solverIK(x, y, z);
			if (i == 4) RM.solverIK(x, y, z);
			if (i == 5) RB.solverIK(x, y, z);
		}
	}

	void Hexapod::applyStand(double time)
	{
	    (void)time;
		double baseX[6] = {80.0, 60.0, 40.0, 80.0, 60.0, 40.0};
		double baseY[6] = {35.0, 35.0, 35.0, -35.0, -35.0, -35.0};
		double z = -10.0;
		for (int i = 0; i < 6; ++i) {
			double x = baseX[i];
			double y = baseY[i];
			if (i == 0) LF.solverIK(x, y, z);
			if (i == 1) LM.solverIK(x, y, z);
			if (i == 2) LB.solverIK(x, y, z);
			if (i == 3) RF.solverIK(x, y, z);
			if (i == 4) RM.solverIK(x, y, z);
			if (i == 5) RB.solverIK(x, y, z);
		}
	}

	void Hexapod::update_legs(double time)
	{
		switch (mode_) {
			case MotionMode::WalkForward:
				applyGait(time, 1.0, stepLength_, stepHeight_, stanceRatio_);
				break;
			case MotionMode::WalkBackward:
				applyGait(time, -1.0, stepLength_, stepHeight_, stanceRatio_);
				break;
			case MotionMode::TurnLeft:
				applyTurn(time, -1.0, false);
				break;
			case MotionMode::TurnRight:
				applyTurn(time, 1.0, false);
				break;
			case MotionMode::Turn360:
				applyTurn(time, 1.0, true);
				break;
			case MotionMode::Dance:
				applyDance(time);
				break;
			case MotionMode::Stride:
				applyGait(time, 1.0, stepLength_ * 1.5, stepHeight_ * 1.0, 0.55);
				break;
			case MotionMode::Stand:
			default:
				applyStand(time);
				break;
		}
	}
}
