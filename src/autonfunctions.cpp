#include "autonfunctions.hpp"
#include <cmath>

//helper to wait until drive motors finish moving
static void waitDriveMotors() {
    pros::delay(50); // Small initial delay so motors register movement
    while (std::abs(left_mg.get_actual_velocity()) > 1 || std::abs(right_mg.get_actual_velocity()) > 1) {
        pros::delay(20);
    }
}

//helper to wait until lift motors finish moving
static void waitLiftMotors() {
    pros::delay(50); // Small initial delay
    while (std::abs(lift.get_actual_velocity()) > 1) {
        pros::delay(20);
    }
    lift.set_brake_mode(pros::E_MOTOR_BRAKE_HOLD);
    lift.brake();
}

//need to add PID

void driveForward(double degrees, int velocity) {
    left_mg.move_relative(std::abs(degrees), std::abs(velocity));
    right_mg.move_relative(std::abs(degrees), std::abs(velocity));
    waitDriveMotors();
}


void driveBackward(double degrees, int velocity) {
    left_mg.move_relative(-std::abs(degrees), std::abs(velocity));
    right_mg.move_relative(-std::abs(degrees), std::abs(velocity));
    waitDriveMotors();
}


void turnLeft(double degrees, int velocity) {
    left_mg.move_relative(-std::abs(degrees), std::abs(velocity));
    right_mg.move_relative(std::abs(degrees), std::abs(velocity));
    waitDriveMotors();
}


void turnRight(double degrees, int velocity) {
    left_mg.move_relative(std::abs(degrees), std::abs(velocity));
    right_mg.move_relative(-std::abs(degrees), std::abs(velocity));
    waitDriveMotors();
}


void driveTime(int timeMs, int velocity) {
    left_mg.move(velocity);
    right_mg.move(velocity);
    pros::delay(timeMs);
    driveStop();
}


void turnTime(int timeMs, int velocity) {
    left_mg.move(velocity);
    right_mg.move(-velocity);
    pros::delay(timeMs);
    driveStop();
}


void driveStop() {
    left_mg.set_brake_mode(pros::E_MOTOR_BRAKE_BRAKE);
    right_mg.set_brake_mode(pros::E_MOTOR_BRAKE_BRAKE);
    left_mg.brake();
    right_mg.brake();
}



void clawOpen() {
    claw.set_value(true);
}

void clawClose() {
    claw.set_value(false);
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
