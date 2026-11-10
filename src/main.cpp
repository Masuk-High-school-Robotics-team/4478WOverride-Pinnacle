#include "main.h" // IWYU pragma: keep
#include "lemlib/api.hpp" // IWYU pragma: keep
#include "autons.hpp"
#include "lemlib/chassis/trackingWheel.hpp"
#include "lemlib/pid.hpp"
#include "menu.hpp"
#include "pros/misc.h"
#include "devices.hpp"

using namespace pros;

/**
 * A callback function for LLEMU's center button.
 *
 * When this callback is fired, it will toggle line 2 of the LCD text between
 * "I was pressed!" and nothing.
 */



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
	//run_selected_auton();
    //recf_skills_slot1();

    //this is for pid tuning
    // set position to x:0, y:0, heading:0
    chassis.setPose(0, 0, 0);
    // turn to face heading 90 with a very long timeout
    chassis.moveToPoint(0, 48, 100000);
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

    lift.set_zero_position(0);
	

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
        if (controller1.get_digital(E_CONTROLLER_DIGITAL_L1) && lift.get_position() < 0) {
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