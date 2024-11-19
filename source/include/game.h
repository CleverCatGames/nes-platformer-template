#ifndef GAME_H
#define GAME_H

#include "lib/nes.h"

extern u8 pad;
extern u8 last_pad;
extern u8 frame_counter;
extern u16 camera_x;
extern word camera_diff;
extern u8 camera_shake; // timer for camera shake
extern u8 camera_page_min;
extern u8 camera_page_max;

#define camera_diff_x camera_diff.byte.lower
#define camera_diff_page camera_diff.byte.upper

extern u8 time_frames;
extern u8 time_seconds;
extern u8 time_minutes;
extern u8 time_hours;

u8 __fastcall__ button_pressed(u8 pad_mask);
u8 __fastcall__ button_released(u8 pad_mask);

u8 __fastcall__ sheet_count(void);

// fade the screen and load the current level
void game_fade_to_black(void);
// reset the current level without a screen transition
void game_reset_level(void);

// main menu declarations
void main_pause(void);
void main_resume(void);
void main_menu(void);
void main_dialogue(void);
void main_powerup(void);
void main_sequence(void);

// run a full frame of the game
void run_frame(void);

void do_nothing(void);

void load_mainmenu_gfx(void);
void load_level_gfx(void);

// banks are zero-indexed
enum {
	BANK1 = 0,
	BANK2,
	BANK3,
	BANK4,
	BANK5,
	BANK6,
	BANK7,
	FIXED,
};

// tileset banks
#define BANK_EXAMPLE   BANK1

// general banks
#define BANK_MUSIC     BANK5
#define BANK_GFX       BANK6
#define BANK_OBJECTS   BANK7

#define SCREEN_WIDTH 256
#define SCREEN_HEIGHT 240

enum {
	GAME_STATE_PLAY = 0,
    GAME_STATE_RESET,
	GAME_STATE_PAUSE,
	GAME_STATE_FADE_BLACK,
	GAME_STATE_FADE_WHITE,
	GAME_STATE_NUM
};

extern u8 game_state;

#define SECONDS(x) (60*(x))

void __fastcall__ move_camera_to_object_x(u8 offset);

#define DEREF_PTR_INTO(ptr, offset, store) \
	__asm__("ldy #%b", offset); \
	__asm__("lda (%v),y", ptr); \
	__asm__("sta %v", store)

#endif
