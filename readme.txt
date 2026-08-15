This repo controls a PCA9685 PWM driver to move servos/continuous motors and calculates leg inverse kinematics for a hexapod. Below is a concise guide: what each file/class/function does, how to run it, and practical debugging tips.
Build & Run

Build on your Raspberry Pi
Notes: I2C must be enabled (sudo raspi-config → Interface Options → I2C), PCA9685 must be powered correctly (motor power often needs separate supply), and you may need sudo or membership in the i2c group to access /dev/i2c-1.
Top-level files

constants.hpp: defines leg_move::PI used across modules.
ServoDriver.hpp / ServoDriver.cpp: PCA9685 driver and helper methods to set PWM/pulse widths.
leg.hpp / leg.cpp: leg class with inverse kinematics (solverIK) and angle storage.
Hexapod.hpp / Hexapod.cpp: high-level hexapod gait generation and leg-target computation.
main.cpp: example continuous-test that sets all channels forward then stops.
Key classes & functions

leg_move::ServoDriver (in ServoDriver.hpp / ServoDriver.cpp)

ServoDriver(int i2cAddress = 0x40): opens /dev/i2c-1, sets the I2C slave address, wakes PCA9685, and calls setPWMFreq(50.0).
~ServoDriver(): closes the I2C file descriptor.
void setAngle(int channel, double angleRadians): converts radians → degrees → pulse width (0.5ms–2.5ms mapped to 0–180°), converts ms→ticks, writes LEDn registers to set the on/off ticks for that channel. Useful for standard positional servos.
void setPulseMs(int channel, double pulseMs): sets a channel directly by pulse width in milliseconds (useful for continuous-rotation servos or ESCs).
void setAllPulseMs(double pulseMs): writes the ALL_LED registers so every channel uses the same pulse width.
void writeRegister(uint8_t reg, uint8_t value): low-level I2C write helper.
uint8_t readRegister(uint8_t reg): low-level I2C read helper.
void setPWMFreq(double freqHz): computes PCA9685 prescale and programs PRESCALE (register 0xFE) with the rounded prescale value (formula uses 25MHz reference and 4096 steps).
leg_move::leg (in leg.hpp / leg.cpp)

leg(double L1, double L2, double L3): constructor stores segment lengths.
JointAngles solverIK(double x, double y, double z): inverse kinematics:
Computes coxa via atan2(y,x).
Computes planar distance to femur/tibia joint, checks reachability (min and max), clamps and computes femur/tibia angles using law of cosines.
Updates and returns Cur_Ang (current stored joint angles).
JointAngles Cur_Angles() const: returns the stored joint angles.
double law_of_cosines(double a, double b, double c): helper used for IK.
leg_move::Hexapod (in Hexapod.hpp / Hexapod.cpp)

Hexapod(double l1, double l2, double l3): creates six leg instances.
Motion setters: setMotionMode, setWalkSpeed, setStepLength, setTurnRate, setDanceEnergy.
Gait/motion calls: walkForward, walkBackward, turnLeft, turnRight, turn360, dance, stride, stand — each calls lower-level helpers.
computeLegTarget(...): computes foot target (x,y,z) for a leg given phaseOffset, time, base positions, direction, step length/height, stance ratio. (Note: it uses phaseOffset added to time * walkSpeed_.)
applyGait, applyTurn, applyDance, applyStand: compute targets for all six legs and call solverIK on each leg.
update_legs(double time): dispatches the appropriate motion routine depending on mode_.
What main.cpp does now

It creates a leg_move::ServoDriver and runs a simple continuous test:
driver.setAllPulseMs(forwardMs); — sets all channels to a forward pulse width (example uses 2.0 ms).
Sleeps 5 seconds, then driver.setAllPulseMs(stopMs); — sets the stop pulse width (~1.5 ms).
Change forwardMs, stopMs, and sleep duration to match your motor/controller.
How the PCA9685 mapping works (practical)

PCA9685 period at 50 Hz = 20 ms. 4096 ticks per period → ticks = (pulseMs / 20.0) * 4096.
For typical servos:
~0.5ms → full one direction
~1.5ms → neutral/stop
~2.5ms → full the other direction
For continuous-rotation motors or ESCs, ranges and midpoints vary — calibrate.
Troubleshooting checklist (motor doesn't turn)

Power: ensure motor/ESC has adequate, separate power; PCA9685 V+ must be powered for servos.
I2C presence: run i2cdetect -y 1 and confirm the PCA9685 (usually at 0x40) appears.
Permissions: either run sudo ./hexapod or add your user to i2c group.
Pulse calibration: try a sweep in main.cpp:
Per-channel test:
driver.setPulseMs(channel, 1.0); → driver.setPulseMs(channel, 2.5); etc.
Adjust setPWMFreq(...) if your ESC/motor controller expects a different frequency.
Check wiring: SDA, SCL to Pi pins, common ground between Pi, PCA9685, and motor power supply.
Test PCA9685 registers directly with i2cset/i2cget (careful — can disrupt devices).
Quick per-channel test snippet

To test a single channel from main.cpp, replace the setAllPulseMs calls with:
