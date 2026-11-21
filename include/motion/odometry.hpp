#pragma once
#include "lemlib/pose.hpp"

namespace motion::odometry {
struct Sample {
    lemlib::Pose pose{0, 0, 0};
    double distance = 0; // average drive-side distance, inches
    double leftDistance = 0;
    double rightDistance = 0;
    bool healthy = false;
};
bool initialize();
Sample sample();
bool setPose(double x, double y, double heading);
} // namespace motion::odometry
