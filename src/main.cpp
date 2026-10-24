#include "main.h" // IWYU pragma: keep
#include "lemlib/api.hpp" // IWYU pragma: keep
#include "autons.hpp"
#include "lemlib/pid.hpp"
#include "menu.hpp"
#include "pros/misc.h"

using namespace pros;

/**
 * A callback function for LLEMU's center button.
 *
 * When this callback is fired, it will toggle line 2 of the LCD text between
 * "I was pressed!" and nothing.
 */

Controller controller1(E_CONTROLLER_MASTER);
 // left motor group
MotorGroup right_mg({2, -4, 8}, MotorGearset::blue);
// right motor group
MotorGroup left_mg({-12, 13, -15}, MotorGearset::blue);

pros::adi::Pneumatics claw('C', false); //initally closed

// cascade lift motors
MotorGroup lift({6, -17}, MotorGearset::green);

// drivetrain settings
lemlib::Drivetrain drivetrain(&left_mg, // left motor group
                              &right_mg, // right motor group
                              10, // 10 inch track width
                              lemlib::Omniwheel::NEW_4, // using new 4" omnis
                              450, // drivetrain rpm is 450
                              2 // horizontal drift is 2 (for now)
);

/**
 * Runs initialization code. This occurs as soon as the program is started.
 *
 * All other competition modes are blocked by initialize; it is recommended
 * to keep execution time for this mode under a few seconds.
 */


 


// imu
Imu imu(10);
// horizontal tracking wheel encoder
Rotation horizontal_encoder(20);
Rotation vertical_encoder(19);
// horizontal tracking wheel
lemlib::TrackingWheel horizontal_tracking_wheel(&horizontal_encoder, lemlib::Omniwheel::NEW_275, -5.75);
// vertical tracking wheel
lemlib::TrackingWheel vertical_tracking_wheel(&vertical_encoder, lemlib::Omniwheel::NEW_275, -2.5);

// odometry settings
lemlib::OdomSensors sensors(nullptr, // vertical tracking wheel 1, set to null
                            nullptr, // vertical tracking wheel 2, set to nullptr as we are using IMEs
                            nullptr, // horizontal tracking wheel 1
                            nullptr, // horizontal tracking wheel 2, set to nullptr as we don't have a second one
                            &imu // inertial sensor
);

// angular PID controller
lemlib::ControllerSettings angular_controller(2, // proportional gain (kP)
                                              0, // integral gain (kI)
                                              10, // derivative gain (kD)
                                              3, // anti windup
                                              1, // small error range, in degrees
                                              100, // small error range timeout, in milliseconds
                                              3, // large error range, in degrees
                                              500, // large error range timeout, in milliseconds
                                              0 // maximum acceleration (slew)
);

lemlib::ControllerSettings lateral_controller(10, // proportional gain (kP)
                                              0, // integral gain (kI)
                                              3, // derivative gain (kD)
                                              3, // anti windup
                                              1, // small error range, in inches
                                              100, // small error range timeout, in milliseconds
                                              3, // large error range, in inches
                                              500, // large error range timeout, in milliseconds
                                              20 // maximum acceleration (slew)
);

// input curve for throttle input during driver control
lemlib::ExpoDriveCurve throttle_curve(3, // joystick deadband out of 127
                                     10, // minimum output where drivetrain will move out of 127
                                     1.019 // expo curve gain
);

// input curve for steer input during driver control
lemlib::ExpoDriveCurve steer_curve(3, // joystick deadband out of 127
                                  10, // minimum output where drivetrain will move out of 127
                                  1.019 // expo curve gain
);


lemlib::Chassis chassis(drivetrain,
                        lateral_controller,
                        angular_controller,
                        sensors,
                        &throttle_curve, 
                        &steer_curve
);

//lemlib::PID 


void initialize() {
	//lcd::initialize(); // initialize brain screen
    init_brain_menu();
    chassis.calibrate(); // calibrate sensors
    // print position to brain screen
}

/**
 * Runs while the robot is in the disabled state of Field Management System or
 * the VEX Competition Switch, following either autonomous or opcontrol. When
 * the robot is enabled, this task will exit.
 */
void disabled() {}

/**
 * Runs after initialize(), and before autonomous when connected to the Field
 * Management System or the VEX Competition Switch. This is intended for
 * competition-specific initialization routines, such as an autonomous selector
 * on the LCD.
 *
 * This task will exit when the robot is enabled and autonomous or opcontrol
 * starts.
 */
void competition_initialize() {}

/**
 * Runs the user autonomous code. This function will be started in its own task
 * with the default priority and stack size whenever the robot is enabled via
 * the Field Management System or the VEX Competition Switch in the autonomous
 * mode. Alternatively, this function may be called in initialize or opcontrol
 * for non-competition testing purposes.
 *
 * If the robot is disabled or communications is lost, the autonomous task
 * will be stopped. Re-enabling the robot will restart the task, not re-start it
 * from where it left off.
 */
void autonomous() {
	run_selected_auton();
}

/**
 * Runs the operator control code. This function will be started in its own task
 * with the default priority and stack size whenever the robot is enabled via
 * the Field Management System or the VEX Competition Switch in the operator
 * control mode.
 *
 * If no competition control is connected, this function will run immediately
 * following initialize().
 *
 * If the robot is disabled or communications is lost, the
 * operator control task will be stopped. Re-enabling the robot will restart the
 * task, not resume it from where it left off.
 */



 
void opcontrol() {
	

	while (true) {

		// get left y and right y positions
        int leftY = controller1.get_analog(E_CONTROLLER_ANALOG_LEFT_Y);
        int rightY = controller1.get_analog(E_CONTROLLER_ANALOG_RIGHT_Y);

        // move the robot
        chassis.tank(leftY, rightY);

        if (controller1.get_digital_new_press(pros::E_CONTROLLER_DIGITAL_Y)) {
            claw.toggle();
        }

        // cascade lift control(hold L1 to go up hold L2 to go down)
        if (controller1.get_digital(E_CONTROLLER_DIGITAL_L1)) {
            lift.move(127);
        } else if (controller1.get_digital(E_CONTROLLER_DIGITAL_L2)) {
            lift.move(-127);
        } else {
            lift.set_brake_mode(pros::E_MOTOR_BRAKE_HOLD);
            lift.brake();
        }

        // delay to save resources
        delay(25);
	}
}