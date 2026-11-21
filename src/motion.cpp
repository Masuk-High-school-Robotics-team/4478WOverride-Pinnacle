#include "motion/motion.hpp"
#include "devices.hpp"
#include "lemlib/driveCurve.hpp"
#include "motion/odometry.hpp"
#include <atomic>
#include <cstdio>

namespace motion {
    namespace {

        // Shared motor access: only the current autonomous command or driver may write outputs.
        pros::Mutex outputMutex;
        std::atomic<bool> autonomousEnabled{false};
        std::atomic<bool> driverEnabled{false};
        std::atomic<std::uint32_t> owner{0}, nextOwner{1};
        std::atomic<const char *> currentStatus{"Not initialized"};
        std::atomic<bool> initializationFinished{false};

        // Stop both sides. The caller must hold outputMutex.
        void brakeLocked() {
            left_mg.set_brake_mode_all(pros::E_MOTOR_BRAKE_BRAKE);
            right_mg.set_brake_mode_all(pros::E_MOTOR_BRAKE_BRAKE);
            left_mg.brake();
            right_mg.brake();
        }

        // Reject non-finite gains and invalid settling/filter settings before moving.
        bool validProfile(const PIDProfile &p) {
            return std::isfinite(p.kP) && p.kP >= 0 && std::isfinite(p.kI) && p.kI >= 0 &&
                   std::isfinite(p.kD) && p.kD >= 0 && std::isfinite(p.integralRange) &&
                   p.integralRange >= 0 && std::isfinite(p.integralLimit) && p.integralLimit >= 0 &&
                   std::isfinite(p.derivativeFilter) && p.derivativeFilter >= 0 &&
                   std::isfinite(p.tolerance) && p.tolerance >= 0 && p.settleMs < 0x80000000u &&
                   std::isfinite(p.largeTolerance) && p.largeTolerance >= 0 &&
                   p.largeSettleMs < 0x80000000u && std::isfinite(p.slew) && p.slew >= 0 &&
                   (p.tolerance > 0 || p.largeTolerance > 0);
        }

        // Validate the overall deadline, voltage ceiling, acceleration, and all PID profiles.
        bool validOptions(const Options &o) {
            return o.timeoutMs > 0 && o.timeoutMs < 0x80000000u && std::isfinite(o.maxVoltage) &&
                   o.maxVoltage > 0 && o.maxVoltage <= 12000 && std::isfinite(o.acceleration) &&
                   o.acceleration > 0 && std::isfinite(o.pointTolerance) && o.pointTolerance > 0 &&
                   validProfile(o.distance) && validProfile(o.headingHold) && validProfile(o.turn);
        }

        // Generation ownership survives PROS killing/restarting the autonomous task.
        // Lifecycle callbacks revoke it; no mutex is held across a blocking movement.
        struct Command {
            const Options options;
            std::uint32_t token = 0, start;
            Result admission = Result::settled;

            // Claim a unique command ID; refuse overlapping commands without disturbing them.
            explicit Command(const Options &value) : options(value), start(pros::millis()) {
                outputMutex.take();
                if (!autonomousEnabled || pros::competition::is_disabled() || owner != 0)
                    admission = Result::cancelled;
                else {
                    token = nextOwner.fetch_add(1);
                    if (token == 0)
                        token = nextOwner.fetch_add(1);
                    owner = token;
                    if (!validOptions(options))
                        admission = Result::invalidOptions;
                    currentStatus = "Moving";
                }
                outputMutex.give();
            }

            // Check cancellation, sensor health, and the single deadline shared by every stage.
            Result check() const {
                if (admission != Result::settled)
                    return admission;
                if (!odometry::sample().healthy)
                    return Result::sensorFault;
                if (owner != token || !autonomousEnabled || pros::competition::is_disabled())
                    return Result::cancelled;
                if (pros::millis() - start >= options.timeoutMs)
                    return Result::timedOut;
                return Result::settled; // here means permission to continue
            }

            // Recheck ownership while locked before sending voltage to the motors.
            bool output(DriveOutput value) {
                outputMutex.take();
                const bool permitted =
                    owner == token && autonomousEnabled && !pros::competition::is_disabled();
                if (permitted) {
                    left_mg.move_voltage(static_cast<int>(std::lround(value.left)));
                    right_mg.move_voltage(static_cast<int>(std::lround(value.right)));
                }
                outputMutex.give();
                return permitted;
            }

            void brake() {
                outputMutex.take();
                if (token != 0 && owner == token)
                    brakeLocked();
                outputMutex.give();
            }

            // Release ownership and report the result. An old command cannot brake a new owner.
            Result finish(Result result) {
                outputMutex.take();
                if (token != 0 && owner == token) {
                    brakeLocked();
                    owner = 0;
                    currentStatus = resultName(result);
                }
                outputMutex.give();
                if (options.telemetry)
                    std::printf("motion,end,%s,%lu\n", resultName(result),
                                static_cast<unsigned long>(pros::millis() - start));
                return result;
            }
        };

        // Run one straight-drive or turning stage. Each stage starts with fresh PID state.
        Result runStage(Command &command, bool driving, double target, bool shortest = false) {
            auto result = command.check();
            if (result != Result::settled)
                return result;
            if (!std::isfinite(target))
                return Result::invalidOptions;
            const auto start = odometry::sample();
            const auto &options = command.options;
            const auto &profile = driving ? options.distance : options.turn;
            PID primary(profile, !driving && shortest);
            PID heading(options.headingHold, true);
            ProfileSettler primarySettler, headingSettler;
            double previousControl = 0, previousCorrection = 0;
            DriveOutput previous{0, 0};
            auto wake = pros::millis(), previousTime = wake;
            auto lastLog = wake - config::telemetryMs;
            while (true) {
                result = command.check();
                if (result != Result::settled) {
                    command.brake();
                    return result;
                }
                const auto now = pros::millis();
                const double dt =
                    now == previousTime ? config::loopMs / 1000.0 : (now - previousTime) / 1000.0;
                previousTime = now;
                const auto sample = odometry::sample();
                if (!sample.healthy) {
                    command.brake();
                    return Result::sensorFault;
                }

                // Straight travel uses encoder distance; turning uses the accumulated field
                // heading.
                const double measured =
                    driving ? sample.distance - start.distance : sample.pose.theta;
                const double error =
                    !driving && shortest ? wrapDegrees(target - measured) : target - measured;
                const double headingError = wrapDegrees(start.pose.theta - sample.pose.theta);

                // Error must remain inside tolerance continuously for the settling duration.
                const bool primaryDone = primarySettler.update(error, profile, now);
                const bool headingDone =
                    headingSettler.update(headingError, options.headingHold, now);
                if (primaryDone && (!driving || headingDone)) {
                    command.brake();
                    return Result::settled;
                }

                // Calculate PID outputs, mix forward/turn power, then limit voltage and
                // acceleration.
                const double control =
                    profileSlew(primary.update(error, dt), previousControl, profile.slew, dt);
                previousControl = control;
                const double correction =
                    driving ? profileSlew(heading.update(headingError, dt), previousCorrection,
                                          options.headingHold.slew, dt)
                            : 0;
                previousCorrection = correction;
                auto output = limitOutput(driving ? control : 0, driving ? correction : control,
                                          options.maxVoltage);
                output.left = slew(output.left, previous.left, options.acceleration, dt);
                output.right = slew(output.right, previous.right, options.acceleration, dt);
                // PID outputs can exceed floating point range with invalid extreme settings.
                if (!std::isfinite(output.left) || !std::isfinite(output.right)) {
                    command.brake();
                    return Result::invalidOptions;
                }

                // Feed the actual limited output back to the integral anti-windup calculation.
                primary.commit(driving ? (output.left + output.right) / 2
                                       : (output.left - output.right) / 2);
                if (driving)
                    heading.commit((output.left - output.right) / 2);
                if (!command.output(output)) {
                    const auto reason = command.check();
                    return reason == Result::settled ? Result::cancelled : reason;
                }
                previous = output;
                if (options.telemetry && now - lastLog >= config::telemetryMs) {
                    const auto p = primary.terms(), h = heading.terms();
                    std::printf(
                        "motion,%s,t=%lu,target=%.3f,measured=%.3f,error=%.3f,P=%.1f,I=%.1f,D=%.1f,"
                        "headingError=%.3f,hP=%.1f,hI=%.1f,hD=%.1f,left=%.0f,right=%.0f\n",
                        driving ? "distance" : "turn",
                        static_cast<unsigned long>(now - command.start), target, measured, error,
                        p.p, p.i, p.d, headingError, h.p, h.i, h.d, output.left, output.right);
                    lastLog = now;
                }
                pros::Task::delay_until(&wake, config::loopMs);
            }
        }

        // Coordinate moves use separate turn and drive stages, stopping on the first failure.
        Result pointStages(Command &command, double x, double y) {
            auto result = command.check();
            if (result != Result::settled)
                return result;
            if (!std::isfinite(x) || !std::isfinite(y))
                return Result::invalidOptions;
            auto pose = odometry::sample().pose;
            auto target = pointTarget(pose.x, pose.y, x, y);
            if (target.distance <= command.options.pointTolerance)
                return Result::settled;
            result = runStage(command, false, target.heading, true);
            if (result != Result::settled)
                return result;
            // Recompute travel distance after the turn, since a real turn may translate slightly.
            pose = odometry::sample().pose;
            target = pointTarget(pose.x, pose.y, x, y);
            result = runStage(command, true, target.distance);
            if (result != Result::settled)
                return result;
            result = command.check();
            if (result != Result::settled)
                return result;
            pose = odometry::sample().pose;
            return std::hypot(x - pose.x, y - pose.y) <= command.options.pointTolerance
                       ? Result::settled
                       : Result::positionNotReached;
        }
    } // namespace

    // Human-readable result strings are shared by terminal telemetry and the brain menu.
    const char *resultName(Result result) {
        switch (result) {
        case Result::settled:
            return "Settled";
        case Result::timedOut:
            return "Timed out";
        case Result::cancelled:
            return "Cancelled";
        case Result::sensorFault:
            return "Sensor fault: restart required";
        case Result::positionNotReached:
            return "Position not reached";
        case Result::invalidOptions:
            return "Invalid motion options";
        }
        return "Unknown result";
    }

    // Start position tracking and the independent disable/sensor-fault watchdog once.
    bool initialize() {
        currentStatus = "Calibrating IMU";
        const bool ready = odometry::initialize();
        currentStatus = ready ? "Ready - UNTUNED" : resultName(Result::sensorFault);
        initializationFinished = true;
        // Independent watchdog also stops output if PROS kills a movement task.
        static pros::Task watchdog([] {
            while (true) {
                const bool fault = !odometry::sample().healthy;
                outputMutex.take();
                // Recheck the mode under the output lock so a stale watchdog sample
                // cannot revoke a driver takeover that just finished.
                if (autonomousEnabled && (pros::competition::is_disabled() || fault)) {
                    autonomousEnabled = false;
                    owner = 0;
                    brakeLocked();
                    currentStatus = fault ? resultName(Result::sensorFault) : "Cancelled";
                }
                outputMutex.give();
                pros::delay(config::loopMs);
            }
        });
        return ready;
    }

    // Revoke command ownership immediately, then brake both drive sides.
    void stop() {
        outputMutex.take();
        owner = 0;
        brakeLocked();
        currentStatus = "Stopped";
        outputMutex.give();
    }

    // Called by the autonomous callback to give autonomous commands drivetrain access.
    void beginAutonomous() {
        outputMutex.take();
        owner = 0;
        driverEnabled = false;
        autonomousEnabled = true;
        brakeLocked();
        currentStatus = "Autonomous";
        outputMutex.give();
    }

    // Called when autonomous ends or the robot is disabled; revoke all motor access.
    void endAutonomous() {
        outputMutex.take();
        autonomousEnabled = driverEnabled = false;
        owner = 0;
        brakeLocked();
        outputMutex.give();
    }

    // Cancel autonomous ownership before enabling joystick control.
    void driverTakeover() {
        endAutonomous();
        outputMutex.take();
        driverEnabled = true;
        currentStatus = "Driver control";
        outputMutex.give();
    }

    // Apply the original tank-drive joystick curve while driver control owns the motors.
    void tank(int left, int right) {
        outputMutex.take();
        if (driverEnabled && !pros::competition::is_disabled()) {
            left_mg.move(static_cast<int>(throttle_curve.curve(std::clamp(left, -127, 127))));
            right_mg.move(static_cast<int>(throttle_curve.curve(std::clamp(right, -127, 127))));
        }
        outputMutex.give();
    }

    // Prevent a field-origin reset while an autonomous command is using the pose.
    bool setPose(double x, double y, double heading) {
        outputMutex.take();
        const bool result = owner == 0 && odometry::setPose(x, y, heading);
        outputMutex.give();
        return result;
    }

    lemlib::Pose getPose() { return odometry::sample().pose; }

    const char *status() {
        if (!initializationFinished)
            return currentStatus.load();
        if (!odometry::sample().healthy)
            return resultName(Result::sensorFault);
        return currentStatus.load();
    }

    // Public blocking movement commands: claim ownership, run stages, and always finish.
    Result driveDistance(double inches, const Options &options) {
        Command command(options);
        return command.finish(runStage(command, true, inches));
    }

    Result turnToHeading(double degrees, const Options &options) {
        Command command(options);
        return command.finish(runStage(command, false, degrees, true));
    }

    // Relative turns intentionally keep the full signed angle, even beyond one revolution.
    Result turnBy(double degrees, const Options &options) {
        Command command(options);
        const auto result = command.check();
        if (result != Result::settled)
            return command.finish(result);
        return command.finish(runStage(command, false, odometry::sample().pose.theta + degrees));
    }

    Result moveToPoint(double x, double y, const Options &options) {
        Command command(options);
        return command.finish(pointStages(command, x, y));
    }

    // Only attempt the final heading after reaching the point; verify position again afterward.
    Result moveToPose(double x, double y, double heading, const Options &options) {
        Command command(options);
        if (!std::isfinite(heading))
            return command.finish(Result::invalidOptions);
        auto result = pointStages(command, x, y);
        if (result == Result::settled)
            result = runStage(command, false, heading, true);
        if (result == Result::settled) {
            result = command.check();
            const auto pose = odometry::sample().pose;
            if (result == Result::settled &&
                std::hypot(x - pose.x, y - pose.y) > options.pointTolerance)
                result = Result::positionNotReached;
        }
        return command.finish(result);
    }
} // namespace motion
