#pragma once

#include "main.h"

// Hardware references from main.cpp
extern pros::Controller controller1;
extern pros::MotorGroup left_mg;
extern pros::MotorGroup right_mg;
extern pros::adi::Pneumatics claw;
extern pros::MotorGroup lift;

// Simple Drivetrain Functions (Motor degrees or time)
void driveForward(double degrees, int velocity = 100);
void driveBackward(double degrees, int velocity = 100);
void turnLeft(double degrees, int velocity = 100);
void turnRight(double degrees, int velocity = 100);

void driveTime(int timeMs, int velocity = 100);
void turnTime(int timeMs, int velocity = 100);
void driveStop();

// Simple Pneumatic Claw Functions
void clawOpen();
void clawClose();
void clawToggle();

// Simple Cascade Lift Functions (Motor degrees or time)
void cascadeUp(double degrees, int velocity = 100);
void cascadeDown(double degrees, int velocity = 100);
void cascadeTime(int timeMs, int velocity = 100);
void cascadeStop();
