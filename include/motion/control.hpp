#pragma once
#include <algorithm>
#include <cmath>
#include <cstdint>

// Hardware-independent math, shared by robot code and host tests.
namespace motion {
constexpr double pi = 3.14159265358979323846;
inline double wrapDegrees(double angle) {
    double result = std::fmod(angle + 180.0, 360.0);
    if (result < 0) result += 360.0;
    return result - 180.0; // [-180, 180); exact half turns choose counterclockwise
}
inline double motorRotationsToInches(double rotations, double diameter, double wheelRpm,
                                     double cartridgeRpm = 600.0) {
    return rotations * pi * diameter * wheelRpm / cartridgeRpm;
}
struct PointTarget { double heading; double distance; };
inline PointTarget pointTarget(double x, double y, double targetX, double targetY) {
    const double dx = targetX - x, dy = targetY - y;
    return {std::atan2(dx, dy) * 180.0 / pi, std::hypot(dx, dy)};
}
// First nine fields follow LemLib ControllerSettings order.
struct PIDProfile {
    double kP, kI, kD;
    double integralRange;           // anti windup; 0 disables the range restriction
    double tolerance;               // small error range
    std::uint32_t settleMs;         // small error timeout
    double largeTolerance;          // large error range; 0 disables this exit
    std::uint32_t largeSettleMs;    // large error timeout
    double slew;                    // power change per nominal 10 ms; 0 disables

    // Advanced settings; not part of the nine-value tuning template.
    double integralLimit = 1000;    // maximum integral contribution, mV
    double derivativeFilter = 0;   // filter time constant, seconds; 0 disables
};
struct PIDTerms { double p = 0, i = 0, d = 0; };
class PID {
 public:
    // Public gains use LemLib's nominal 10 ms / 127-power scale.
    // Convert once to our internal millivolt / seconds scale; callers do no conversion.
    explicit PID(PIDProfile settings, bool cyclicError = false) : profile(settings), cyclic(cyclicError) {
        constexpr double powerToMillivolts = 12000.0 / 127.0;
        constexpr double nominalSeconds = 0.010;
        profile.kP *= powerToMillivolts;
        profile.kI *= powerToMillivolts / nominalSeconds;
        profile.kD *= powerToMillivolts * nominalSeconds;
    }
    void reset() { integral = candidate = derivative = previous = 0; first = true; values = {}; }
    double update(double error, double dt) {
        if (!(dt > 0) || !std::isfinite(dt) || !std::isfinite(error)) return 0;
        double difference = error - previous;
        if (cyclic) difference = wrapDegrees(difference);
        if (first) derivative = 0;
        else {
            const double alpha = dt / (profile.derivativeFilter + dt);
            derivative += alpha * (difference / dt - derivative);
        }
        previous = error;
        first = false;
        if (profile.kI == 0 || (profile.integralRange != 0 && std::abs(error) > profile.integralRange)) integral = 0;
        candidate = integral;
        if (profile.kI != 0 && (profile.integralRange == 0 || std::abs(error) <= profile.integralRange)) {
            const double limit = profile.integralLimit / std::abs(profile.kI);
            candidate = std::clamp(integral + error * dt, -limit, limit);
        }
        values = {profile.kP * error, profile.kI * candidate, profile.kD * derivative};
        return values.p + values.i + values.d;
    }
    // Call after mixing, voltage saturation, and acceleration limiting.
    void commit(double appliedOutput) {
        const double requested = values.p + values.i + values.d;
        if ((candidate - integral) * profile.kI * (requested - appliedOutput) <= 0) integral = candidate;
        values.i = profile.kI * integral;
    }
    PIDTerms terms() const { return values; }
 private:
    PIDProfile profile;
    bool cyclic, first = true;
    double integral = 0, candidate = 0, derivative = 0, previous = 0;
    PIDTerms values;
};
struct DriveOutput { double left, right; };
inline DriveOutput limitOutput(double forward, double turn, double limit) {
    DriveOutput output{forward + turn, forward - turn};
    const double scale = std::max(1.0, std::max(std::abs(output.left), std::abs(output.right)) / limit);
    return {output.left / scale, output.right / scale};
}
inline double slew(double requested, double previous, double acceleration, double dt) {
    return std::clamp(requested, previous - acceleration * dt, previous + acceleration * dt);
}
class Settler {
 public:
    bool update(bool inside, std::uint32_t now, std::uint32_t duration) {
        if (!inside) { tracking = false; return false; }
        if (!tracking) { since = now; tracking = true; }
        return now - since >= duration;
    }
 private:
    bool tracking = false;
    std::uint32_t since = 0;
};
// Either enabled error band may finish a movement; leaving a band resets its timer.
class ProfileSettler {
 public:
    bool update(double error, const PIDProfile& profile, std::uint32_t now) {
        const bool smallDone = small.update(profile.tolerance > 0 && std::abs(error) <= profile.tolerance,
                                             now, profile.settleMs);
        const bool largeDone = large.update(profile.largeTolerance > 0 && std::abs(error) <= profile.largeTolerance,
                                             now, profile.largeSettleMs);
        return smallDone || largeDone;
    }
 private:
    Settler small, large;
};
inline double profileSlew(double requested, double previous, double amount, double dt) {
    return amount == 0 ? requested : slew(requested, previous, amount * (12000.0 / 127.0) / 0.010, dt);
}
} // namespace motion
