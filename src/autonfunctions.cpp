#include "autonfunctions.hpp"
#include <cmath>
#include <vector>
#include <algorithm>

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

////////////////////////////////////////////////////////////////////////////////
// PID functions (ported from 4478W_PushBack, VEXcode -> PROS)
//
// Positive = forward / clockwise (right), matching chassis.tank() and the IMU.
// Gains are the PushBack robot's values -- treat them as a starting point and
// retune for this robot.

static const int    PID_TIMEOUT_MS = 1250;   // safety: never run a single PID longer than this
static const double DRIVE_DIR = -1;          // drivePID direction: 1 or -1 (flips reading AND command together)
static const bool   DRIVE_USE_VOLTAGE = true; // drivePID: true = voltage (smooth, coasts), false = velocity (brakes hard)
static const double DRIVE_MIN_POWER = 10;     // drivePID voltage mode: least power (out of 127) that still moves the robot
static const bool   TURN_USE_VOLTAGE = true;  // turnPID/turntoPID: true = voltage, false = velocity
static const double TURN_MIN_POWER = 10;      // turn voltage mode: least power (out of 127) that still turns the robot

// Sends a turn command to the drive. Positive = turn right (clockwise).
// Voltage mode: bumps tiny outputs up to TURN_MIN_POWER while not settled, caps at 127.
static void sendTurn(double speed, double error) {
    if (TURN_USE_VOLTAGE) {
        // Not settled yet but power too low to turn the robot -> bump it to the minimum
        if (std::fabs(error) > 0.3 && std::fabs(speed) < TURN_MIN_POWER) {
            speed = (speed >= 0 ? 1 : -1) * TURN_MIN_POWER;
        }
        if (speed > 127) speed = 127;
        if (speed < -127) speed = -127;

        left_mg.move(speed);
        right_mg.move(-speed);
    } else {
        left_mg.move_velocity(speed);
        right_mg.move_velocity(-speed);
    }
}

// Converts a motor group's position readings to degrees, whatever encoder
// units are set. LemLib's chassis.calibrate() switches the drive motors to
// ROTATIONS when no tracking wheels are given, so we can't assume degrees.
static double toDegreesFactor(pros::MotorGroup& mg) {
    switch (mg.get_encoder_units()) {
        case pros::MotorUnits::rotations: return 360.0;
        case pros::MotorUnits::counts:    return 360.0 / 300.0; // blue cartridge: 300 counts/rev
        default:                          return 1.0;           // already degrees
    }
}

// Median of all 6 drive motor positions (degrees)
double medianMotorPos() {
    std::vector<double> positions = left_mg.get_position_all();
    std::vector<double> rights = right_mg.get_position_all();

    double leftFactor = toDegreesFactor(left_mg);
    double rightFactor = toDegreesFactor(right_mg);
    for (double& p : positions) p *= leftFactor;
    for (double& p : rights) p *= rightFactor;

    positions.insert(positions.end(), rights.begin(), rights.end());

    std::sort(positions.begin(), positions.end());

    size_t n = positions.size();
    if (n == 0) return 0;
    if (n % 2 == 0) {
        return (positions[n / 2 - 1] + positions[n / 2]) / 2.0; // true median for even count
    }
    return positions[n / 2];
}

// Drive PID: returns true when we've settled (slow + close to target)
// 10% of 600 rpm = 60 rpm
bool checkMotors(double error) {
    return std::fabs(left_mg.get_actual_velocity()) < 30 &&
           std::fabs(right_mg.get_actual_velocity()) < 30 &&
           std::fabs(error) < 4;
}

// Turn PID: returns true when we've settled
// 5% of 600 rpm = 30 rpm
bool checkMotorsInertial(double error) {
    return std::fabs(left_mg.get_actual_velocity()) < 20 &&
           std::fabs(right_mg.get_actual_velocity()) < 20 &&
           std::fabs(error) < 0.3;
}

// Drive straight a target distance in MOTOR degrees (positive = forward)
void drivePID(double target) {
    left_mg.set_brake_mode_all(pros::E_MOTOR_BRAKE_COAST);
    right_mg.set_brake_mode_all(pros::E_MOTOR_BRAKE_COAST);

    left_mg.tare_position_all();
    right_mg.tare_position_all();

    int estimatedTimeOut = (target/(200*60) + 0.75)* 1000;

    bool PIDon = true;

    // Gains depend on the mode (DRIVE_USE_VOLTAGE at the top of this file)
    // Velocity mode: output is rpm (-600..600)
    // Voltage mode:  output is power (-127..127). 127/600 = 0.21, so these
    //                start as the velocity gains scaled to the same strength.
    double pK, iK, dK;
    if (DRIVE_USE_VOLTAGE) {
        pK = 0.3967;  // proportional constant
        iK = 0;     // integral constant
        dK = 4.6769420;  // derivative constant
    } else {
        pK = 1.1;   // proportional constant
        iK = 0;     // integral constant
        dK = 2.5;   // derivative constant
    }

    double error = target;
    double prevError = target; // start equal to error so the first derivative is 0
    double integral = 0;

    double runTime = (std::fabs(target) / (250 * 6)) * 1000; // estimated time to run in ms
    uint32_t start = pros::millis();

    while (PIDon || pros::millis() - start < runTime) {
        if (pros::millis() - start > estimatedTimeOut) break;

        double medPos = DRIVE_DIR * medianMotorPos(); // flip reading

        error = target - medPos;

        integral += error;
        if (std::fabs(error) > 100 || std::fabs(error) < 8) { // prevent integral windup
            integral = 0;
        }

        double derivative = error - prevError;
        prevError = error;

        double speed = error * pK + integral * iK + derivative * dK;

        if (DRIVE_USE_VOLTAGE) {
            // Not settled yet but power too low to move the robot -> bump it to the minimum
            if (std::fabs(error) > 6 && std::fabs(speed) < DRIVE_MIN_POWER) {
                speed = (speed >= 0 ? 1 : -1) * DRIVE_MIN_POWER;
            }
            if (speed > 127) speed = 127;
            if (speed < -127) speed = -127;

            left_mg.move(DRIVE_DIR * speed); // flip command
            right_mg.move(DRIVE_DIR * speed);
        } else {
            left_mg.move_velocity(DRIVE_DIR * speed); // flip command
            right_mg.move_velocity(DRIVE_DIR * speed);
        }

        PIDon = !checkMotors(error);

        pros::delay(10);
    }

    left_mg.brake();
    right_mg.brake();
}

// Turn by a RELATIVE angle in degrees (positive = right / clockwise)
void turnPID(double target) {
    left_mg.set_brake_mode_all(pros::E_MOTOR_BRAKE_COAST);
    right_mg.set_brake_mode_all(pros::E_MOTOR_BRAKE_COAST);

    bool PIDon = true;

    // Gains depend on the mode (TURN_USE_VOLTAGE at the top of this file)
    // Velocity mode: output is rpm (-600..600)
    // Voltage mode:  output is power (-127..127), velocity gains x 127/600 to start
    double pK, iK, dK;
    if (TURN_USE_VOLTAGE) {
        pK = 0.61;   // proportional constant
        iK = 0;      // integral constant
        dK = 0.14;   // derivative constant
    } else {
        pK = 2.875;  // proportional constant
        iK = 0;      // integral constant
        dK = 0.6575; // derivative constant
    }

    target = imu.get_rotation() + target; // convert relative turn to absolute target

    double error = target - imu.get_rotation();
    double prevError = error;
    double integral = 0;

    uint32_t start = pros::millis();

    while (PIDon) {
        if (pros::millis() - start > PID_TIMEOUT_MS) break;

        double heading = imu.get_rotation();

        error = target - heading;

        integral += error;
        if (std::fabs(error) > 50 || std::fabs(error) < 5) { // prevent integral windup
            integral = 0;
        }

        double derivative = error - prevError;
        prevError = error;

        double speed = error * pK + integral * iK + derivative * dK;

        sendTurn(speed, error);

        PIDon = !checkMotorsInertial(error);

        pros::delay(15);
    }

    left_mg.brake();
    right_mg.brake();
}

// Turn to an ABSOLUTE heading in degrees (0-360, clockwise), taking the shortest way
void turntoPID(double targetHeading) {
    left_mg.set_brake_mode_all(pros::E_MOTOR_BRAKE_COAST);
    right_mg.set_brake_mode_all(pros::E_MOTOR_BRAKE_COAST);

    bool PIDon = true;

    // Gains depend on the mode (TURN_USE_VOLTAGE at the top of this file)
    // Velocity mode: output is rpm (-600..600)
    // Voltage mode:  output is power (-127..127), velocity gains x 127/600 to start
    double pK, iK, dK;
    if (TURN_USE_VOLTAGE) {
        pK = 1.161;   // proportional constant
        iK = 0;      // integral constant
        dK = 0.81;   // derivative constant
    } else {
        pK = 4.125;  // proportional constant
        iK = 0;      // integral constant
        dK = 10.5;   // derivative constant
    }

    // Find the shortest signed turn (-180..180) from where we are now,
    // then aim at that point on the continuous rotation scale
    double current = imu.get_rotation();
    double delta = std::fmod(targetHeading - current, 360.0);
    if (delta > 180) delta -= 360;
    if (delta < -180) delta += 360;
    double target = current + delta;

    double error = delta;
    double prevError = error;
    double integral = 0;

    uint32_t start = pros::millis();

    while (PIDon) {
        if (pros::millis() - start > PID_TIMEOUT_MS) break;

        double heading = imu.get_rotation();

        error = target - heading;

        integral += error;
        if (std::fabs(error) > 50 || std::fabs(error) < 5) { // prevent integral windup
            integral = 0;
        }

        double derivative = error - prevError;
        prevError = error;

        double speed = error * pK + integral * iK + derivative * dK;

        sendTurn(speed, error);

        PIDon = !checkMotorsInertial(error);

        pros::delay(15);
    }

    left_mg.brake();
    right_mg.brake();
}

// Drive a distance in inches using drivePID
void driveInch(double inches) {
    double wheelDiameter = 3.25;                     // inches
    double circumference = wheelDiameter * M_PI;     // wheel circumference
    double gearRatio = 600.0 / 450.0;                // motor:wheel (600 rpm motors, 450 rpm wheels)
    double motorDegrees = (inches / circumference) * (360.0 * gearRatio);
    drivePID(motorDegrees);
}

////////////////////////////////////////////////////////////////////////////////

void driveBackward(double degrees, int velocity) {
    left_mg.move_relative(std::abs(degrees), std::abs(velocity));
    right_mg.move_relative(std::abs(degrees), std::abs(velocity));
    waitDriveMotors();
}


void driveForward(double degrees, int velocity) {
    left_mg.move_relative(-std::abs(degrees), std::abs(velocity));
    right_mg.move_relative(-std::abs(degrees), std::abs(velocity));
    //waitDriveMotors();
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
    //claw.set_value(false);
    claw.extend();
}

void clawClose() {
    claw.set_value(true);
}

void clawToggle() {
    claw.toggle();
}


void cascadeUp(double degrees, int velocity) {
    lift.move_relative(-std::abs(degrees), std::abs(velocity));
    waitLiftMotors();
}


void cascadeDown(double degrees, int velocity) {
    lift.move_relative(std::abs(degrees), std::abs(velocity));
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
