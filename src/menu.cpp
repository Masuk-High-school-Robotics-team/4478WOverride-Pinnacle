#include "menu.hpp"
#include "autons.hpp"

// screen object
lv_obj_t* main_screen = nullptr;

// placeholder for debug
static void debug_btn_event_cb(lv_event_t* e) {

}


// callback when VEX override is clicked
static void vex_override_btn_event_cb(lv_event_t* e) {
    current_mode = VEX_OVERRIDE;
}

//callback when RECF pinnacle
static void recf_btn_event_cb(lv_event_t* e) {
    current_mode = RECF_PINNACLE;
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