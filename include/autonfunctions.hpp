#pragma once
#include "devices.hpp"

// Mechanism helpers; drivetrain movement is in motion/motion.hpp.
void clawOpen();
void clawClose();
void clawToggle();
void cascadeUp(double degrees, int velocity = 100);
void cascadeDown(double degrees, int velocity = 100);
void cascadeTime(int timeMs, int velocity = 100);
void cascadeStop();
