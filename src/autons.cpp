#include "autons.hpp"


//initizale default
mode current_mode = VEX_OVERRIDE;
type current_type = MATCH;
int current_slot = 0; // default to slot 0

// auton route def

//vex override match
void vex_match_slot1() { 
    //put auton code here in future
    pros::lcd::set_text(1, "Running: VEX Match Slot 1"); 

}

void vex_match_slot2() { 

    pros::lcd::set_text(1, "Running: VEX Match Slot 2"); 

}

void vex_match_slot3() { 

    pros::lcd::set_text(1, "Running: VEX Match Slot 3"); 

}

void vex_match_slot4() { 

    pros::lcd::set_text(1, "Running: VEX Match Slot 4"); 

}


//vex overrides skills
void vex_skills_slot1() { 

    pros::lcd::set_text(1, "Running: VEX Skills Slot 1"); 

}

void vex_skills_slot2() { 
    
    pros::lcd::set_text(1, "Running: VEX Skills Slot 2"); 
}

void vex_skills_slot3() { 
    
    pros::lcd::set_text(1, "Running: VEX Skills Slot 3"); 
}

void vex_skills_slot4() { 
    
    pros::lcd::set_text(1, "Running: VEX Skills Slot 4"); 
}


//RECF pinnacle match
void recf_match_slot1() { 
    
    pros::lcd::set_text(1, "Running: RECF Match Slot 1"); 

}

void recf_match_slot2() { 
    
    pros::lcd::set_text(1, "Running: RECF Match Slot 2"); 

}

void recf_match_slot3() { 
    
    pros::lcd::set_text(1, "Running: RECF Match Slot 3"); 

}

void recf_match_slot4() { 
    
    pros::lcd::set_text(1, "Running: RECF Match Slot 4"); 

}


//RECF pinnacle skills
void recf_skills_slot1() {
    
    pros::lcd::set_text(1, "Running: RECF Skills Slot 1"); 

}

void recf_skills_slot2() {
    
    pros::lcd::set_text(1, "Running: RECF Skills Slot 2"); 

}

void recf_skills_slot3() {
    
    pros::lcd::set_text(1, "Running: RECF Skills Slot 3"); 

}

void recf_skills_slot4() {

     pros::lcd::set_text(1, "Running: RECF Skills Slot 4"); 

}


//16 slot matrix
//Leave unused slots with "" and nullptr
autonSlot autons[2][2][4] = {
    // mode 0: VEX_OVERRIDE
    {
        // type 0: MATCH
        {
            {"VEX Match Slot 1", vex_match_slot1},
            {"VEX Match Slot 2", vex_match_slot2},
            {"VEX Match Slot 3", vex_match_slot3},
            {"VEX Match Slot 4", vex_match_slot4},
        },
        // type 1: SKILLS
        {
            {"VEX Skills Slot 1", vex_skills_slot1},
            {"VEX Skills Slot 2", vex_skills_slot2},
            {"", nullptr}, //empty slot 3
            {"", nullptr}  //empty slot 4
        }
    },
    // mode 1: RECF_PINNACLE
    {
        // type 0: MATCH
        {
            {"RECF Match Slot 1", recf_match_slot1},
            {"RECF Match Slot 2", recf_match_slot2},
            {"", nullptr},
            {"", nullptr}
        },
        // type 1: SKILLS
        {
            {"RECF Skills Slot 1", recf_skills_slot1},
            {"", nullptr},
            {"", nullptr},
            {"", nullptr}
        }
    }
};


//auton execution

void run_selected_auton() {
    //fetch slot
    autonSlot selected = autons[current_mode][current_type][current_slot];

    // saftey thing so it only excetues if the function pointer is not null
    if (selected.auton_fn != nullptr) {
        selected.auton_fn();
    } else {
        pros::lcd::set_text(1, "WARNING: Selected slot is empty!");
    }
}