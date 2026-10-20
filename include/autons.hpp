#pragma once

#include "main.h"
#include <functional>
#include <string>

//this file is the headers for autons.cpp

//enums
enum mode {
    VEX_OVERRIDE = 0,
    RECF_PINNACLE = 1
};

enum type {
    MATCH = 0,
    SKILLS = 1
};

//define slots for code
struct autonSlot {
    std::string name; //name, self expanatory
    std::function<void()> auton_fn;//the function pointer to run
};

//global array for declaration(2 modes x 2 types x 4 slots = 16 slots of code, not all will be needed but jst in case)
extern autonSlot autons[2][2][4];

// Current user selection
extern mode current_mode;
extern type current_type;
extern int current_slot; // 0, 1, 2, 3

// runner fucntion called during autonomus()
void run_selected_auton();

//declerations for auton routines

//vex ovvverride match
void vex_match_slot1();
void vex_match_slot2();
void vex_match_slot3();
void vex_match_slot4();

//vex overrides skills
void vex_skills_slot1();
void vex_skills_slot2();
void vex_skills_slot3();
void vex_skills_slot4();

//RECF pinnacle match
void recf_match_slot1();
void recf_match_slot2();
void recf_match_slot3();
void recf_match_slot4();

//RECF pinnacle skills
void recf_skills_slot1();
void recf_skills_slot2();
void recf_skills_slot3();
void recf_skills_slot4();