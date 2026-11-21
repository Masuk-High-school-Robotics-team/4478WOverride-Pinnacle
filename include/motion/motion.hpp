#pragma once
#include "motion/config.hpp"
#include "lemlib/pose.hpp"

namespace motion {
enum class Result { settled, timedOut, cancelled, sensorFault, positionNotReached, invalidOptions };
const char* resultName(Result result);
struct Options {
    explicit Options(std::uint32_t timeoutMs) : timeoutMs(timeoutMs) {}
    std::uint32_t timeoutMs;
    PIDProfile distance = config::distance;
    PIDProfile headingHold = config::headingHold;
    PIDProfile turn = config::turn;
    double maxVoltage = config::maxVoltage;
    double acceleration = config::acceleration;
    double pointTolerance = config::pointTolerance;
    bool telemetry = true;
};
bool initialize();
// Lifecycle calls are used by the PROS callbacks, not by individual routes.
void beginAutonomous();
void endAutonomous();
void driverTakeover();
void tank(int left, int right);
void stop();
bool setPose(double x, double y, double heading);
lemlib::Pose getPose();
const char* status();
Result driveDistance(double inches, const Options& options);
Result turnToHeading(double degrees, const Options& options);
Result turnBy(double degrees, const Options& options);
Result moveToPoint(double x, double y, const Options& options);
Result moveToPose(double x, double y, double heading, const Options& options);
} // namespace motion
