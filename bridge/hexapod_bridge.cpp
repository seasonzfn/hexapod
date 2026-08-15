#include <pybind11/pybind11.h>
#include "Hexapod.hpp"

namespace py = pybind11;
using namespace leg_move;

PYBIND11_MODULE(hexapod_core, m)
{
    py::class_<JointAngles>(m, "JointAngles")
        .def_readonly("coxa", &JointAngles::coxa)
        .def_readonly("femur", &JointAngles::femur)
        .def_readonly("tibia", &JointAngles::tibia);

    py::class_<Hexapod>(m, "Hexapod")
        .def(py::init<double, double, double>())
        .def("updateAllLegs", &Hexapod::updateAllLegs)
        .def("setMotionMode", &Hexapod::setMotionMode)
        .def("setWalkSpeed", &Hexapod::setWalkSpeed)
        .def("setStepLength", &Hexapod::setStepLength)
        .def("setTurnRate", &Hexapod::setTurnRate)
        .def("setDanceEnergy", &Hexapod::setDanceEnergy)
        .def("walkForward", &Hexapod::walkForward)
        .def("walkBackward", &Hexapod::walkBackward)
        .def("turnLeft", &Hexapod::turnLeft)
        .def("turnRight", &Hexapod::turnRight)
        .def("turn360", &Hexapod::turn360)
        .def("dance", &Hexapod::dance)
        .def("stride", &Hexapod::stride)
        .def("stand", &Hexapod::stand)
        .def("getFL", &Hexapod::getFL)
        .def("getFR", &Hexapod::getFR)
        .def("getML", &Hexapod::getML)
        .def("getMR", &Hexapod::getMR)
        .def("getBL", &Hexapod::getBL)
        .def("getBR", &Hexapod::getBR)
        .def("getLM", &Hexapod::getML)
        .def("getRM", &Hexapod::getMR);

    m.attr("MODE_STAND") = static_cast<int>(MotionMode::Stand);
    m.attr("MODE_WALK_FORWARD") = static_cast<int>(MotionMode::WalkForward);
    m.attr("MODE_WALK_BACKWARD") = static_cast<int>(MotionMode::WalkBackward);
    m.attr("MODE_TURN_LEFT") = static_cast<int>(MotionMode::TurnLeft);
    m.attr("MODE_TURN_RIGHT") = static_cast<int>(MotionMode::TurnRight);
    m.attr("MODE_TURN_360") = static_cast<int>(MotionMode::Turn360);
    m.attr("MODE_DANCE") = static_cast<int>(MotionMode::Dance);
    m.attr("MODE_STRIDE") = static_cast<int>(MotionMode::Stride);
}
