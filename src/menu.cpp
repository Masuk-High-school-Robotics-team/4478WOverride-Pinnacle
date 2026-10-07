
#include "menu.hpp"
#include "autons.hpp"
#include "liblvgl/lvgl.h"
#include <string>

lv_obj_t* main_screen = nullptr;
lv_obj_t* type_screen = nullptr;
lv_obj_t* slot_screen = nullptr;

//ui
static lv_obj_t* slot_btns[4];
static lv_obj_t* slot_labels[4];
static lv_obj_t* slot_title_label = nullptr;

//event callbacks

//debug button, nothing yet will add later
static void debug_btn_event_cb(lv_event_t* e) {

}

//main to tpe
static void vex_override_btn_event_cb(lv_event_t* e) {
    current_mode = VEX_OVERRIDE;
    open_type_screen();
}

static void recf_btn_event_cb(lv_event_t* e) {
    current_mode = RECF_PINNACLE;
    open_type_screen();
}

//type to slot
static void match_btn_event_cb(lv_event_t* e) {
    current_type = MATCH;
    open_slot_screen();
}

static void skills_btn_event_cb(lv_event_t* e) {
    current_type = SKILLS;
    open_slot_screen();
}

//back buttons
static void back_to_main_cb(lv_event_t* e) {
    lv_screen_load(main_screen);
}

static void back_to_type_cb(lv_event_t* e) {
    lv_screen_load(type_screen);
}

//slot selection
static void slot_btn_event_cb(lv_event_t* e) {
    uintptr_t slot_idx = (uintptr_t)lv_event_get_user_data(e);
    current_slot = static_cast<int>(slot_idx);

    //standby
}

//screen creation functions

void open_type_screen() {
    if (type_screen == nullptr) {
        type_screen = lv_obj_create(NULL);

        //header label
        lv_obj_t* title = lv_label_create(type_screen);
        lv_label_set_text(title, "Select Match Type");
        lv_obj_align(title, LV_ALIGN_TOP_LEFT, 15, 12);

        //back button
        lv_obj_t* back_btn = lv_button_create(type_screen);
        lv_obj_set_size(back_btn, 90, 36);
        lv_obj_align(back_btn, LV_ALIGN_TOP_RIGHT, -10, 5);
        lv_obj_add_event_cb(back_btn, back_to_main_cb, LV_EVENT_CLICKED, NULL);
        
        lv_obj_t* back_lbl = lv_label_create(back_btn);
        lv_label_set_text(back_lbl, "< Back");
        lv_obj_center(back_lbl);

        //seperator line
        static lv_point_t line_points[] = { {0, 42}, {480, 42} };
        lv_obj_t* line = lv_line_create(type_screen);
        lv_obj_set_size(line, 480, 2);
        lv_obj_set_pos(line, 0, 42);

        //match button
        lv_obj_t* match_btn = lv_button_create(type_screen);
        lv_obj_set_size(match_btn, 210, 160);
        lv_obj_align(match_btn, LV_ALIGN_LEFT_MID, 15, 20);
        lv_obj_add_event_cb(match_btn, match_btn_event_cb, LV_EVENT_CLICKED, NULL);

        lv_obj_t* match_lbl = lv_label_create(match_btn);
        lv_label_set_text(match_lbl, "MATCH AUTON");
        lv_obj_center(match_lbl);

        //skills button
        lv_obj_t* skills_btn = lv_button_create(type_screen);
        lv_obj_set_size(skills_btn, 210, 160);
        lv_obj_align(skills_btn, LV_ALIGN_RIGHT_MID, -15, 20);
        lv_obj_add_event_cb(skills_btn, skills_btn_event_cb, LV_EVENT_CLICKED, NULL);

        lv_obj_t* skills_lbl = lv_label_create(skills_btn);
        lv_label_set_text(skills_lbl, "SKILLS AUTON");
        lv_obj_center(skills_lbl);
    }

    lv_screen_load(type_screen);
}

void open_slot_screen() {
    if (slot_screen == nullptr) {
        slot_screen = lv_obj_create(NULL);

        //title header
        slot_title_label = lv_label_create(slot_screen);
        lv_obj_align(slot_title_label, LV_ALIGN_TOP_LEFT, 15, 12);

        //back button
        lv_obj_t* back_btn = lv_button_create(slot_screen);
        lv_obj_set_size(back_btn, 90, 36);
        lv_obj_align(back_btn, LV_ALIGN_TOP_RIGHT, -10, 5);
        lv_obj_add_event_cb(back_btn, back_to_type_cb, LV_EVENT_CLICKED, NULL);
        
        lv_obj_t* back_lbl = lv_label_create(back_btn);
        lv_label_set_text(back_lbl, "< Back");
        lv_obj_center(back_lbl);

        //seperator line
        static lv_point_t line_points[] = { {0, 42}, {480, 42} };
        lv_obj_t* line = lv_line_create(slot_screen);
        lv_obj_set_size(line, 480, 2);
        lv_obj_set_pos(line, 0, 42);

        //2x2 slot
        int btn_w = 210;
        int btn_h = 75;
        int x_off[4] = {15, 255, 15, 255};
        int y_off[4] = {50, 50, 140, 140};

        for (int i = 0; i < 4; i++) {
            slot_btns[i] = lv_button_create(slot_screen);
            lv_obj_set_size(slot_btns[i], btn_w, btn_h);
            lv_obj_set_pos(slot_btns[i], x_off[i], y_off[i]);
            lv_obj_add_event_cb(slot_btns[i], slot_btn_event_cb, LV_EVENT_CLICKED, (void*)(uintptr_t)i);

            slot_labels[i] = lv_label_create(slot_btns[i]);
            lv_obj_center(slot_labels[i]);
        }
    }

    //refresh 
    std::string mode_str = (current_mode == VEX_OVERRIDE) ? "VEX" : "RECF";
    std::string type_str = (current_type == MATCH) ? "MATCH" : "SKILLS";
    std::string title_text = mode_str + " - " + type_str + " - Select Slot";
    lv_label_set_text(slot_title_label, title_text.c_str());

    //names
    for (int i = 0; i < 4; i++) {
        autonSlot slot = autons[current_mode][current_type][i];

        if (slot.auton_fn != nullptr) {
            std::string btn_txt = "Slot " + std::to_string(i + 1) + ":\n" + slot.name;
            lv_label_set_text(slot_labels[i], btn_txt.c_str());
            lv_obj_remove_state(slot_btns[i], LV_STATE_DISABLED);
        } else {
            std::string btn_txt = "Slot " + std::to_string(i + 1) + ":\nNothing Saved";
            lv_label_set_text(slot_labels[i], btn_txt.c_str());
            lv_obj_add_state(slot_btns[i], LV_STATE_DISABLED);
        }
    }

    lv_screen_load(slot_screen);
}

void init_brain_menu() {
    main_screen = lv_obj_create(NULL);
    lv_screen_load(main_screen);

    //team label
    lv_obj_t* team_label = lv_label_create(main_screen);
    lv_label_set_text(team_label, "4478W");
    lv_obj_align(team_label, LV_ALIGN_TOP_LEFT, 15, 12);

    //status
    lv_obj_t* status_badge = lv_label_create(main_screen);
    lv_label_set_text(status_badge, "[ ALL SYS OK ]");
    lv_obj_align(status_badge, LV_ALIGN_TOP_MID, 0, 12);

    //debug button
    lv_obj_t* debug_btn = lv_button_create(main_screen);
    lv_obj_set_size(debug_btn, 90, 32);
    lv_obj_align(debug_btn, LV_ALIGN_TOP_RIGHT, -10, 5);
    lv_obj_add_event_cb(debug_btn, debug_btn_event_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t* debug_label = lv_label_create(debug_btn);
    lv_label_set_text(debug_label, "DEBUG");
    lv_obj_center(debug_label);

    //header separator line
    //static lv_point_line_t line_points[] = { {0, 42}, {480, 42} };
    lv_obj_t* line = lv_obj_create(main_screen);
    lv_obj_set_size(line, 480, 2);
    lv_obj_set_pos(line, 0, 42);


    // main mode selection buttons


    //vex override button
    lv_obj_t* vex_btn = lv_button_create(main_screen);
    lv_obj_set_size(vex_btn, 210, 160);
    lv_obj_align(vex_btn, LV_ALIGN_LEFT_MID, 15, 20);
    lv_obj_add_event_cb(vex_btn, vex_override_btn_event_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t* vex_label = lv_label_create(vex_btn);
    lv_label_set_text(vex_label, "VEX OVERRIDE");
    lv_obj_center(vex_label);

    //RECF pinnacle button
    lv_obj_t* recf_btn = lv_button_create(main_screen);
    lv_obj_set_size(recf_btn, 210, 160);
    lv_obj_align(recf_btn, LV_ALIGN_RIGHT_MID, -15, 20);
    lv_obj_add_event_cb(recf_btn, recf_btn_event_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t* recf_label = lv_label_create(recf_btn);
    lv_label_set_text(recf_label, "RECF PINNACLE");
    lv_obj_center(recf_label);
}