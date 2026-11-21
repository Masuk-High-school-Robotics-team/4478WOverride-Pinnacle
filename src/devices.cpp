#include "devices.hpp"
#include "lemlib/driveCurve.hpp"
#include "motion/config.hpp"

pros::Controller controller1(pros::E_CONTROLLER_MASTER);

pros::MotorGroup right_mg({-2, 4, -8}, pros::MotorGearset::blue);
pros::MotorGroup left_mg({12, -13, 15}, pros::MotorGearset::blue);


pros::adi::Pneumatics claw('C', false);

pros::MotorGroup lift({6, -17}, pros::MotorGearset::green);

pros::MotorGroup toggle({7, -9});

pros::Imu imu(motion::config::imuPort);

// Same curves previously passed to chassis.tank().
lemlib::ExpoDriveCurve throttle_curve(3, 10, 1.019);
lemlib::ExpoDriveCurve steer_curve(3, 10, 1.019);
