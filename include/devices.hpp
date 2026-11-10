#pragma once

#include "main.h" // IWYU pragma: keep
#include "lemlib/api.hpp" // IWYU pragma: keep
#include "pros/motor_group.hpp"

extern pros::Controller controller1;
extern pros::MotorGroup lift;
extern pros::adi::Pneumatics claw;
extern lemlib::Chassis chassis; 
extern pros::MotorGroup toggle;