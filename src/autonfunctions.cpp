#include "autonfunctions.hpp"
#include <cmath>

static void waitLiftMotors() {
    pros::delay(50);
    while (std::abs(lift.get_actual_velocity()) > 1) pros::delay(20);
    lift.set_brake_mode(pros::E_MOTOR_BRAKE_HOLD);
    lift.brake();
}


void clawOpen() { 

    claw.extend(); 

}
void clawClose() { 

    claw.set_value(true); 

}
void clawToggle() { 

    claw.toggle(); 

}


void cascadeUp(double degrees, int velocity) {
    lift.move_relative(std::abs(degrees), std::abs(velocity));
    waitLiftMotors();
}


void cascadeDown(double degrees, int velocity) {
    lift.move_relative(-std::abs(degrees), std::abs(velocity));
    waitLiftMotors();
}


void cascadeTime(int timeMs, int velocity) {
    lift.move(velocity);
    pros::delay(timeMs);
    cascadeStop();
}


void cascadeStop() {
    lift.set_brake_mode(pros::E_MOTOR_BRAKE_HOLD);
    lift.brake();
}
