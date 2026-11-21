#include "autons.hpp"
#include "motion/motion.hpp"
#include <cstdio>

mode current_mode = VEX_OVERRIDE;
type current_type = MATCH;
int current_slot = 0;


//vex match
void vex_match_slot1() {

}

void vex_match_slot2() {

}

void vex_match_slot3() {

}

void vex_match_slot4() {

}


//vex skills
void vex_skills_slot1() {

}

void vex_skills_slot2() {

}

void vex_skills_slot3() {
    //dont add
}

void vex_skills_slot4() {
    //dont add
}


//recf match
void recf_match_slot1() {

    motion::setPose(0, 0, 0);
    motion::Options options(5000);
    options.maxVoltage = 6000;

    motion::driveDistance(3, options);
    motion::turnToHeading(90, options);
    motion::driveDistance(23, options);
}

void recf_match_slot2() {

}

void recf_match_slot3() {

}

void recf_match_slot4() {

}

//recf skills
void recf_skills_slot1() {

}

void recf_skills_slot2() {

}

void recf_skills_slot3() {

}

void recf_skills_slot4() {
    //dont add
}


namespace {
// All profiles are UNTUNED. Run only on a clear practice field.
// These functions ARE the tuning routines. The menu matrix below points directly
// to them, rather than to the empty vex_skills_slot3/4 and recf_skills_slot2 bodies.
// Select VEX Override -> Skills -> Slot 3 to run this turn test in autonomous.
void tuneTurns() {
    // Define the starting field pose; this does not physically move the robot.
    if (!motion::setPose(0, 0, 0)) return;
    // Allow up to 4000 ms for EACH turn; terminal telemetry is enabled by default.
    const motion::Options options(4000);
    // Block until settled or failed. On failure, return without starting the next move.
    if (motion::turnBy(90, options) != motion::Result::settled) return;
    if (motion::turnBy(-90, options) != motion::Result::settled) return;
}
void tuneDistance() {
    // VEX Override -> Skills -> Slot 4: 24 inches forward, then backward.
    const motion::Options options(5000);
    if (motion::driveDistance(24, options) != motion::Result::settled) return;
    if (motion::driveDistance(-24, options) != motion::Result::settled) return;
}
void tuneCoordinates() {
    // RECF Pinnacle -> Skills -> Slot 2: move to a point/final heading, then return.
    if (!motion::setPose(0, 0, 0)) return;
    const motion::Options options(12000);
    if (motion::moveToPose(12, 24, 0, options) != motion::Result::settled) return;
    if (motion::moveToPose(0, 0, 0, options) != motion::Result::settled) return;
}
}

// Original 2 x 2 x 4 menu matrix and all existing names are retained.
// Only previously empty slots receive tuning routines.
autonSlot autons[2][2][4] = {
    {
        {
            {"VEX Match Slot 1", vex_match_slot1},
            {"VEX Match Slot 2", vex_match_slot2},
            {"VEX Match Slot 3", vex_match_slot3},
            {"VEX Match Slot 4", vex_match_slot4},
        },
        {
            {"VEX Skills Slot 1", vex_skills_slot1},
            {"VEX Skills Slot 2", vex_skills_slot2},
            {"Tune Turns", tuneTurns},
            {"Tune Distance", tuneDistance},
        }
    },
    {
        {
            {"RECF Match Slot 1", recf_match_slot1},
            {"RECF Match Slot 2", recf_match_slot2},
            {"", nullptr},
            {"", nullptr},
        },
        {
            {"RECF Skills Slot 1", recf_skills_slot1},
            {"Tune Coordinates", tuneCoordinates},
            {"", nullptr},
            {"", nullptr},
        }
    }
};

void run_selected_auton() {
    if (current_mode < VEX_OVERRIDE || current_mode > RECF_PINNACLE ||
        current_type < MATCH || current_type > SKILLS || current_slot < 0 || current_slot >= 4) {
        motion::stop();
        std::printf("Invalid autonomous selection\n");
        return;
    }
    const autonSlot selected = autons[current_mode][current_type][current_slot];
    if (selected.auton_fn != nullptr) {
        std::printf("Running: %s\n", selected.name.c_str());
        selected.auton_fn();
    } else {
        motion::stop();
        std::printf("Selected autonomous slot is empty\n");
    }
}

