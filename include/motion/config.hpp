#pragma once
#include "motion/control.hpp"

namespace motion::config {
constexpr int imuPort = 16;
constexpr double wheelDiameter = 3.25;
constexpr double wheelRpm = 450;
constexpr double trackWidth = 13.0625;
constexpr std::uint32_t loopMs = 10;
constexpr std::uint32_t telemetryMs = 100;

// Same order and gain scale as LemLib. Angular gains still need robot tuning.

// Lateral controller
inline constexpr PIDProfile distance{
    5.871, // proportional gain (kP)
    0, // integral gain (kI)
    2.825200962832, // derivative gain (kD)
    3, // anti windup
    1, // small error range, in inches
    100, // small error range timeout, in milliseconds
    3, // large error range, in inches
    500, // large error range timeout, in milliseconds
    20 // maximum acceleration (slew)
};

// Angular correction while driving
inline constexpr PIDProfile headingHold{
    0.846666666667, // proportional gain (kP)
    0, // integral gain (kI)
    8.466666666667, // derivative gain (kD)
    5, // anti windup
    2, // small error range, in degrees
    200, // small error range timeout, in milliseconds
    0, // large error range, in degrees
    0, // large error range timeout, in milliseconds
    0 // maximum acceleration (slew)
};

// Angular controller for turns
inline constexpr PIDProfile turn{
    0.846666666667, // proportional gain (kP)
    0, // integral gain (kI)
    8.466666666667, // derivative gain (kD)
    5, // anti windup
    2, // small error range, in degrees
    200, // small error range timeout, in milliseconds
    0, // large error range, in degrees
    0, // large error range timeout, in milliseconds
    0 // maximum acceleration (slew)
};
constexpr double maxVoltage = 4500;          // conservative initial tuning ceiling, mV
constexpr double acceleration = 12000;      // mV per second
constexpr double pointTolerance = 2;        // inches
} // namespace motion::config
