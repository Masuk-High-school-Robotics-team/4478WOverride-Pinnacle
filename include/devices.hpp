#pragma once
#include "main.h"
namespace lemlib { class ExpoDriveCurve; }

extern pros::Controller controller1;
extern pros::MotorGroup left_mg;
extern pros::MotorGroup right_mg;
extern pros::MotorGroup lift;
extern pros::adi::Pneumatics claw;
extern pros::Imu imu;
extern pros::MotorGroup toggle;
extern lemlib::ExpoDriveCurve throttle_curve;
extern lemlib::ExpoDriveCurve steer_curve;
