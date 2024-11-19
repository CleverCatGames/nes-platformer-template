// -----------------------------------------------------------------
// This file allows variables to be organized by size and location
// -----------------------------------------------------------------

#include "level.h"

// -----------------------------------------------------
// LEVEL

u8 tile_buffer[SCREEN_ROWS*4]; // 4 column wide buffer (32 pixels)
u8 level_col; // active column position on screen

u8 attr_buffer[SCREEN_ROWS];

u8 vram_buffer[VRAM_BUFFER_SIZE]; // buffer for vram updates
u8 level_current; // id of current level
u8 level_last;  // the previous level
u8 level_next;  // id of upcoming level (used for transitions)
u8 level_x;
u16 tile_get_x; // set by routines (temp)
u8 tile_get_y;  // set by routines (temp)
u8 tile_type;
u8 tile_liquid;
u8 level_bank;  // the bank for the active level data
u8 level_state;
u8 level_pages; // number of screens wide for the active level
u8 level_flags; // custom flags for level
u8 level_exit;  // level exits (xxxxudlr)
u8 level_zone;  // where to load graphics for the level

// used to load object variables
u8 next_obj_type;
u8 next_obj_page;
u8 next_obj_state;
u8 next_obj_pos;

// how screens correspond to map locations
u8 map_screens[MAX_SCREENS];

// -----------------------------------------------------
// GLOBAL

u8 game_state;
u8 frame_counter;
u8 pad; // controller 1 buttons

u16 camera_x;
word camera_diff;
u8 camera_shake; // timer for camera shake
u8 camera_page_min;
u8 camera_page_max;

// -----------------------------------------------------
// OBJECTS

u8 obj_x_l[MAX_OBJECTS];
u8 obj_x_h[MAX_OBJECTS];
u8 obj_y[MAX_OBJECTS];
u8 obj_type[MAX_OBJECTS];
u8 obj_timer[MAX_OBJECTS];
u8 obj_state[MAX_OBJECTS];

u8 obj_active[MAX_ACTIVE];

u16 object_x;

u8 num_active_objects;

// -----------------------------------------------------
// TRIGGER AABB (full height and 64 pixels wide bounding boxes)

u8 trigger_id;
u8 trigger_x_l[MAX_TRIGGERS];
u8 trigger_x_h[MAX_TRIGGERS];
u8 trigger_type[MAX_TRIGGERS];
u8 trigger_state[MAX_TRIGGERS];
u8 trigger_y[MAX_TRIGGERS];

// -----------------------------------------------------
// PLAYER

u8 player_x; // left point of player sprite
u8 player_y; // top point of player sprite
s8 player_velocity_x;
s8 player_velocity_y; // should never be more than MAX_Y_VELOCITY

u8 player_state;

u8 player_frame;
u8 player_frame_speed;
u8 player_animation;
u8 player_animation_len;
u8 player_health;
u8 player_health_max;
u8 player_damage; // how much damage the player will take
u8 player_invincible_timer;
u8 player_jump_timer;
u8 player_tile; // current tile the player is on
u8 last_pad;

