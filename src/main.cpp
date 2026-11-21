#include "main.h"
#include "autons.hpp"
#include "menu.hpp"
#include "devices.hpp"
#include "motion/motion.hpp"

using namespace pros;

void initialize() {
    // LVGL menu owns the brain screen; do not initialize LLEMU over it.
    init_brain_menu();
    motion::initialize();
    lift.tare_position_all();

}


void disabled() { 

    motion::endAutonomous(); 

}




void competition_initialize() {

}


void autonomous() {


    motion::beginAutonomous();
    run_selected_auton();
    motion::endAutonomous();


}



void opcontrol() {


    motion::driverTakeover();


    while (true) {

        int leftY = controller1.get_analog(E_CONTROLLER_ANALOG_LEFT_Y);
        int rightY = controller1.get_analog(E_CONTROLLER_ANALOG_RIGHT_Y);
        motion::tank(-leftY, -rightY);

        if (controller1.get_digital_new_press(E_CONTROLLER_DIGITAL_R1)) claw.toggle();
        if (controller1.get_digital(E_CONTROLLER_DIGITAL_L1)) {
            lift.move(127);
        } else if (controller1.get_digital(E_CONTROLLER_DIGITAL_L2)) {
            lift.move(-127);
        } else {
            lift.set_brake_mode(E_MOTOR_BRAKE_HOLD);
            lift.brake();
        }
        if (controller1.get_digital(E_CONTROLLER_DIGITAL_R2)) {
            toggle.move(25);
        } else {
            toggle.set_brake_mode(E_MOTOR_BRAKE_COAST);
            toggle.brake();
        }
        delay(25);
    }
}
