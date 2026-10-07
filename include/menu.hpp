#pragma once

#include "main.h"

//initizlate screen
void init_brain_menu();

//screen objects
extern lv_obj_t* main_screen;
extern lv_obj_t* type_screen;
extern lv_obj_t* slot_screen;

// function of screens
void open_type_screen();
void open_slot_screen();