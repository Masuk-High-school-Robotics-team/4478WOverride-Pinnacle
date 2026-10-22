#include "menu.hpp"
#include "autons.hpp"
#include "liblvgl/lvgl.h"
#include <string>
#include <vector>

// define expected ports
struct ExpectedDevice {
    int port;
    std::string name;
};

//change here
static std::vector<ExpectedDevice> expected_hardware = {
    {1, "Left Drive 1"},
    {2, "Left Drive 2"},
    {3, "Left Drive 3"},
    {4, "Right Drive 1"},
    {5, "Right Drive 2"},
    {6, "Right Drive 3"},
    {10, "Inertial Sensor"}
};

// UI Element Handles
lv_obj_t* main_screen = nullptr;
lv_obj_t* type_screen = nullptr;
lv_obj_t* slot_screen = nullptr;
lv_obj_t* standby_screen = nullptr;
lv_obj_t* debug_screen = nullptr;

// ui
static lv_obj_t* slot_btns[4];
static lv_obj_t* slot_labels[4];
static lv_obj_t* slot_title_label = nullptr;
static lv_obj_t* list_container = nullptr;
static lv_obj_t* battery_label = nullptr;
static lv_obj_t* armed_routine_label = nullptr;
static lv_obj_t* warning_drawer_label = nullptr;

// event callbacks
// debug button, nothing yet will add later
static void debug_btn_event_cb(lv_event_t* e) {
    open_debug_screen();
}

// main to tpe
static void vex_override_btn_event_cb(lv_event_t* e) {
    current_mode = VEX_OVERRIDE;
    open_type_screen();
}

static void recf_btn_event_cb(lv_event_t* e) {
    current_mode = RECF_PINNACLE;
    open_type_screen();
}

// type to slot
static void match_btn_event_cb(lv_event_t* e) {
    current_type = MATCH;
    open_slot_screen();
}

static void skills_btn_event_cb(lv_event_t* e) {
    current_type = SKILLS;
    open_slot_screen();
}

// back buttons
static void back_to_main_cb(lv_event_t* e) {
    if (main_screen != nullptr) {
        lv_screen_load(main_screen);
    }
}

static void back_to_type_cb(lv_event_t* e) {
    open_type_screen();
}

// slot selection
static void slot_btn_event_cb(lv_event_t* e) {
    uintptr_t slot_idx = (uintptr_t)lv_event_get_user_data(e);
    current_slot = static_cast<int>(slot_idx);
    
    // standby
    open_standby_screen();
}

// screen creation functions
void open_type_screen() {
    if (type_screen == nullptr) {
        type_screen = lv_obj_create(NULL);

        // header label
        lv_obj_t* title = lv_label_create(type_screen);
        lv_label_set_text(title, "Select Match Type");
        lv_obj_align(title, LV_ALIGN_TOP_LEFT, 15, 12);

        // back button
        lv_obj_t* back_btn = lv_button_create(type_screen);
        lv_obj_set_size(back_btn, 90, 36);
        lv_obj_align(back_btn, LV_ALIGN_TOP_RIGHT, -10, 5);
        lv_obj_add_event_cb(back_btn, back_to_main_cb, LV_EVENT_CLICKED, NULL);

        lv_obj_t* back_lbl = lv_label_create(back_btn);
        lv_label_set_text(back_lbl, "< Back");
        lv_obj_center(back_lbl);

        // separator line
        lv_obj_t* line = lv_obj_create(type_screen);
        lv_obj_set_size(line, 480, 2);
        lv_obj_set_pos(line, 0, 42);

        // match button
        lv_obj_t* match_btn = lv_button_create(type_screen);
        lv_obj_set_size(match_btn, 210, 160);
        lv_obj_align(match_btn, LV_ALIGN_LEFT_MID, 15, 20);
        lv_obj_add_event_cb(match_btn, match_btn_event_cb, LV_EVENT_CLICKED, NULL);

        lv_obj_t* match_lbl = lv_label_create(match_btn);
        lv_label_set_text(match_lbl, "MATCH AUTON");
        lv_obj_center(match_lbl);

        // skills button
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

// standby screen
void open_standby_screen() {
    if (standby_screen == nullptr) {
        standby_screen = lv_obj_create(NULL);

        // header
        lv_obj_t* title = lv_label_create(standby_screen);
        lv_label_set_text(title, "AUTON ARMED & READY");
        lv_obj_align(title, LV_ALIGN_TOP_LEFT, 15, 12);

        // change auton
        lv_obj_t* reselect_btn = lv_button_create(standby_screen);
        lv_obj_set_size(reselect_btn, 85, 36);
        lv_obj_align(reselect_btn, LV_ALIGN_TOP_RIGHT, -110, 5);
        lv_obj_add_event_cb(reselect_btn, back_to_type_cb, LV_EVENT_CLICKED, NULL);

        lv_obj_t* reselect_lbl = lv_label_create(reselect_btn);
        lv_label_set_text(reselect_lbl, "< Change");
        lv_obj_center(reselect_lbl);

        // debug shortcut
        lv_obj_t* debug_btn = lv_button_create(standby_screen);
        lv_obj_set_size(debug_btn, 90, 36);
        lv_obj_align(debug_btn, LV_ALIGN_TOP_RIGHT, -10, 5);
        lv_obj_add_event_cb(debug_btn, debug_btn_event_cb, LV_EVENT_CLICKED, NULL);

        lv_obj_t* debug_lbl = lv_label_create(debug_btn);
        lv_label_set_text(debug_lbl, "DEBUG");
        lv_obj_center(debug_lbl);

        // line
        lv_obj_t* line = lv_obj_create(standby_screen);
        lv_obj_set_size(line, 480, 2);
        lv_obj_set_pos(line, 0, 42);

        // armed card container
        lv_obj_t* armed_card = lv_obj_create(standby_screen);
        lv_obj_set_size(armed_card, 450, 75);
        lv_obj_align(armed_card, LV_ALIGN_TOP_MID, 0, 50);

        armed_routine_label = lv_label_create(armed_card);
        lv_obj_center(armed_routine_label);

        // warning
        lv_obj_t* warning_box = lv_obj_create(standby_screen);
        lv_obj_set_size(warning_box, 450, 85);
        lv_obj_align(warning_box, LV_ALIGN_BOTTOM_MID, 0, -10);

        warning_drawer_label = lv_label_create(warning_box);
        lv_obj_align(warning_drawer_label, LV_ALIGN_TOP_LEFT, 5, 5);
    }

    // update armed label
    autonSlot selected = autons[current_mode][current_type][current_slot];
    std::string mode_str = (current_mode == VEX_OVERRIDE) ? "VEX" : "RECF";
    std::string type_str = (current_type == MATCH) ? "Match" : "Skills";
    std::string armed_text = "ARMED [" + mode_str + " " + type_str + " S" + std::to_string(current_slot + 1) + "]:\n" + selected.name;
    lv_label_set_text(armed_routine_label, armed_text.c_str());
    // update status summary
    int bat_pct = static_cast<int>(pros::battery::get_capacity());
    std::string status_txt = "Battery: " + std::to_string(bat_pct) + "%\n";

    // check if any expected device is missing
    int missing_count = 0;
    for (const auto& dev : expected_hardware) {
        if (pros::c::get_plugged_type(dev.port) == pros::c::E_DEVICE_NONE) {
            missing_count++;
        }
    }

    if (missing_count == 0) {
        status_txt += "Hardware Status: ALL EXPECTED DEVICES CONNECTED";
    } else {
        status_txt += "Hardware Alert: " + std::to_string(missing_count) + " EXPECTED DEVICE(S) DISCONNECTED!";
    }

    lv_label_set_text(warning_drawer_label, status_txt.c_str());
    lv_screen_load(standby_screen);
}

// debug screen
void open_debug_screen() {
    if (debug_screen == nullptr) {
        debug_screen = lv_obj_create(NULL);

        // header label
        lv_obj_t* title = lv_label_create(debug_screen);
        lv_label_set_text(title, "SYSTEM DIAGNOSTICS");
        lv_obj_align(title, LV_ALIGN_TOP_LEFT, 15, 12);

        // battery telemntery
        battery_label = lv_label_create(debug_screen);
        lv_label_set_text(battery_label, "Bat: ---%");
        lv_obj_align(battery_label, LV_ALIGN_TOP_RIGHT, -110, 12);

        // back button
        lv_obj_t* back_btn = lv_button_create(debug_screen);
        lv_obj_set_size(back_btn, 90, 36);
        lv_obj_align(back_btn, LV_ALIGN_TOP_RIGHT, -10, 5);
        lv_obj_add_event_cb(back_btn, back_to_main_cb, LV_EVENT_CLICKED, NULL);

        lv_obj_t* back_lbl = lv_label_create(back_btn);
        lv_label_set_text(back_lbl, "< Back");
        lv_obj_center(back_lbl);

        // divider line
        lv_obj_t* line = lv_obj_create(debug_screen);
        lv_obj_set_size(line, 480, 2);
        lv_obj_set_pos(line, 0, 42);

        // scollable container
        list_container = lv_obj_create(debug_screen);
        lv_obj_set_size(list_container, 450, 180);
        lv_obj_align(list_container, LV_ALIGN_BOTTOM_MID, 0, -5);

        // enable scrolling and auto scrollbar
        lv_obj_set_scroll_dir(list_container, LV_DIR_VER);
        lv_obj_set_scrollbar_mode(list_container, LV_SCROLLBAR_MODE_ON);
    }

    // populatr rows
    update_debug_hardware();
    lv_screen_load(debug_screen);
}

void update_debug_hardware() {
    if (debug_screen == nullptr || list_container == nullptr) return;

    // update battery percentage
    int bat_pct = static_cast<int>(pros::battery::get_capacity());
    std::string bat_str = "Bat: " + std::to_string(bat_pct) + "%";
    if (battery_label != nullptr) {
        lv_label_set_text(battery_label, bat_str.c_str());
    }

    // clear old list items
    lv_obj_clean(list_container);

    int y_pos = 5;

    // 3. Scan all 21 ports
    for (int port = 1; port <= 21; port++) {
        pros::c::v5_device_e_t type = pros::c::get_plugged_type(port);

        // check if this port is expected and get its name
        std::string expected_name = "";
        for (const auto& dev : expected_hardware) {
            if (dev.port == port) {
                expected_name = dev.name;
                break;
            }
        }

        // filter logic: if the port is not expected and has no device, skip it
        if (type == pros::c::E_DEVICE_NONE && expected_name.empty()) {
            continue;
        }

        // display the port and its status
        lv_obj_t* row_label = lv_label_create(list_container);
        lv_obj_set_pos(row_label, 10, y_pos);

        std::string text = "Port " + std::to_string(port);
        if (!expected_name.empty()) {
            text += " (" + expected_name + "): ";
        } else {
            text += ": ";
        }

        if (type == pros::c::E_DEVICE_NONE) {
            text += "[!] DISCONNECTED";
        } else if (type == pros::c::E_DEVICE_MOTOR) {
            pros::Motor temp_motor(port);
            int temp = static_cast<int>(temp_motor.get_temperature());
            text += "MOTOR | Temp: " + std::to_string(temp) + "C";
        } else if (type == pros::c::E_DEVICE_RADIO) {
            text += "VEX RADIO";
        } else if (type == pros::c::E_DEVICE_IMU) {
            text += "INERTIAL SENSOR";
        } else if (type == pros::c::E_DEVICE_ROTATION) {
            text += "ROTATION SENSOR";
        } else {
            text += "CONNECTED DEVICE";
        }

        lv_label_set_text(row_label, text.c_str());
        y_pos += 28; // vertical spacing
    }
}

void open_slot_screen() {
    if (slot_screen == nullptr) {
        slot_screen = lv_obj_create(NULL);

        // title header
        slot_title_label = lv_label_create(slot_screen);
        lv_obj_align(slot_title_label, LV_ALIGN_TOP_LEFT, 15, 12);

        // back button
        lv_obj_t* back_btn = lv_button_create(slot_screen);
        lv_obj_set_size(back_btn, 90, 36);
        lv_obj_align(back_btn, LV_ALIGN_TOP_RIGHT, -10, 5);
        lv_obj_add_event_cb(back_btn, back_to_type_cb, LV_EVENT_CLICKED, NULL);

        lv_obj_t* back_lbl = lv_label_create(back_btn);
        lv_label_set_text(back_lbl, "< Back");
        lv_obj_center(back_lbl);

        // Separator line
        lv_obj_t* line = lv_obj_create(slot_screen);
        lv_obj_set_size(line, 480, 2);
        lv_obj_set_pos(line, 0, 42);

        // 2x2 slot
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

    // refresh
    std::string mode_str = (current_mode == VEX_OVERRIDE) ? "VEX" : "RECF";
    std::string type_str = (current_type == MATCH) ? "MATCH" : "SKILLS";
    std::string title_text = mode_str + " - " + type_str + " - Select Slot";
    lv_label_set_text(slot_title_label, title_text.c_str());

    // names
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

    // team label
    lv_obj_t* team_label = lv_label_create(main_screen);
    lv_label_set_text(team_label, "4478W");
    lv_obj_align(team_label, LV_ALIGN_TOP_LEFT, 15, 12);

    // status
    lv_obj_t* status_badge = lv_label_create(main_screen);
    lv_label_set_text(status_badge, "[ ALL SYS OK ]");
    lv_obj_align(status_badge, LV_ALIGN_TOP_MID, 0, 12);

    // debug button
    lv_obj_t* debug_btn = lv_button_create(main_screen);
    lv_obj_set_size(debug_btn, 90, 36);
    lv_obj_align(debug_btn, LV_ALIGN_TOP_RIGHT, -10, 5);
    lv_obj_add_event_cb(debug_btn, debug_btn_event_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t* debug_label = lv_label_create(debug_btn);
    lv_label_set_text(debug_label, "DEBUG");
    lv_obj_center(debug_label);

    // header separator line
    lv_obj_t* line = lv_obj_create(main_screen);
    lv_obj_set_size(line, 480, 2);
    lv_obj_set_pos(line, 0, 42);

    // main mode selection buttons
    // vex override button
    lv_obj_t* vex_btn = lv_button_create(main_screen);
    lv_obj_set_size(vex_btn, 210, 160);
    lv_obj_align(vex_btn, LV_ALIGN_LEFT_MID, 15, 20);
    lv_obj_add_event_cb(vex_btn, vex_override_btn_event_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t* vex_label = lv_label_create(vex_btn);
    lv_label_set_text(vex_label, "VEX OVERRIDE");
    lv_obj_center(vex_label);

    // RECF pinnacle button
    lv_obj_t* recf_btn = lv_button_create(main_screen);
    lv_obj_set_size(recf_btn, 210, 160);
    lv_obj_align(recf_btn, LV_ALIGN_RIGHT_MID, -15, 20);
    lv_obj_add_event_cb(recf_btn, recf_btn_event_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t* recf_label = lv_label_create(recf_btn);
    lv_label_set_text(recf_label, "RECF PINNACLE");
    lv_obj_center(recf_label);
}