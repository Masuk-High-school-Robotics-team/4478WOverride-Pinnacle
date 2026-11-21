#include "motion/odometry.hpp"
#include "devices.hpp"
#include "lemlib/chassis/odom.hpp"
#include "motion/config.hpp"
#include <cmath>

namespace motion::odometry {
    namespace {

        // Protect the cached sample and LemLib globals from simultaneous reads and updates.
        pros::Mutex mutex;
        Sample latest;
        bool attempted = false;
        bool initialized = false;
        std::uint32_t sampledAt = 0;

        // Construct these after the hardware globals, inside initialize().
        lemlib::TrackingWheel *leftWheel = nullptr;
        lemlib::TrackingWheel *rightWheel = nullptr;

        // All three drive motors must return finite encoder positions and the expected gearing.
        bool validMotors(pros::MotorGroup &group) {
            const auto values = group.get_position_all();
            const auto gears = group.get_gearing_all();
            if (values.size() != 3 || gears.size() != 3)
                return false;
            for (unsigned i = 0; i < values.size(); ++i) {
                if (!std::isfinite(values[i]) || gears[i] != pros::MotorGearset::blue)
                    return false;
            }
            return true;
        }

        // Position tracking needs a calibrated IMU and valid encoders on both drive sides.
        bool sensorsHealthy() {
            return imu.is_installed() && !imu.is_calibrating() &&
                   std::isfinite(imu.get_rotation()) && validMotors(left_mg) &&
                   validMotors(right_mg);
        }

        // Called only with mutex held: advance odometry and cache one consistent snapshot.
        void updateLocked() {
            // Latch faults: feeding invalid readings into LemLib would poison its baselines.
            // A sensor fault requires restarting the robot program after reconnecting hardware.
            if (!latest.healthy || !sensorsHealthy()) {
                latest.healthy = false;
                return;
            }
            lemlib::update();
            latest.pose = lemlib::getPose();
            latest.leftDistance = leftWheel->getDistanceTraveled();
            latest.rightDistance = rightWheel->getDistanceTraveled();
            latest.distance = (latest.leftDistance + latest.rightDistance) / 2.0;
            latest.healthy = std::isfinite(latest.pose.x) && std::isfinite(latest.pose.y) &&
                             std::isfinite(latest.pose.theta) && std::isfinite(latest.distance);
            sampledAt = pros::millis();
        }
    } // namespace

    // Calibrate once at startup; the robot must stay stationary during calibration.
    bool initialize() {
        if (attempted)
            return initialized;
        attempted = true;
        if (!imu.is_installed() || imu.reset() == PROS_ERR)
            return false;
        const auto start = pros::millis();
        do {
            pros::delay(10);
            if (pros::millis() - start >= 5000)
                return false;
        } while (imu.is_calibrating());
        if (!sensorsHealthy() || imu.set_rotation(0) == PROS_ERR)
            return false;

        // These are motor-encoder adapters, not physical tracking wheels. Offsets are from center.
        leftWheel = new lemlib::TrackingWheel(&left_mg, config::wheelDiameter,
                                              -config::trackWidth / 2, config::wheelRpm);
        rightWheel = new lemlib::TrackingWheel(&right_mg, config::wheelDiameter,
                                               config::trackWidth / 2, config::wheelRpm);

        // Establish startup baselines once; movement commands never tare these encoders.
        leftWheel->reset();
        rightWheel->reset();
        lemlib::setSensors(lemlib::OdomSensors(leftWheel, rightWheel, nullptr, nullptr, &imu),
                           lemlib::Drivetrain(&left_mg, &right_mg, config::trackWidth,
                                              config::wheelDiameter, config::wheelRpm, 2));
        lemlib::setPose({0, 0, 0});
        latest.healthy = initialized = true;
        sampledAt = pros::millis();
        // Own the task instead of calling lemlib::init(): serialize update/get/setPose
        // because the library's global odometry functions do not provide a mutex.
        static pros::Task tracking([] {
            auto wake = pros::millis();
            while (true) {
                mutex.take();
                updateLocked();
                mutex.give();
                pros::Task::delay_until(&wake, config::loopMs);
            }
        });
        return true;
    }

    // Return a locked copy; samples older than 100 ms cannot safely guide a movement.
    Sample sample() {
        mutex.take();
        Sample value = latest;
        value.healthy = value.healthy && pros::millis() - sampledAt <= 100;
        mutex.give();
        return value;
    }

    // Change field coordinates without resetting the physical encoder or IMU readings.
    bool setPose(double x, double y, double heading) {
        if (!std::isfinite(x) || !std::isfinite(y) || !std::isfinite(heading))
            return false;
        if (!std::isfinite(static_cast<float>(x)) || !std::isfinite(static_cast<float>(y)) ||
            !std::isfinite(static_cast<float>(heading)))
            return false;
        mutex.take();
        updateLocked(); // consume pending sensor deltas before changing the field origin
        const bool healthy = latest.healthy;
        if (healthy) {
            lemlib::setPose(
                {static_cast<float>(x), static_cast<float>(y), static_cast<float>(heading)});
            latest.pose = lemlib::getPose();
        }
        mutex.give();
        return healthy;
    }
} // namespace motion::odometry
