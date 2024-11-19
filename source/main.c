#include "level.h"
#include "object.h"
#include "player.h"
#include "sprites.h"
#include "sfx.h"
#include "ppu.h"

void level_load_initial();

void time_advance_frame(void) {
	++time_frames;
	if (time_frames == 60) {
		time_frames = 0;
		++time_seconds;
		if (time_seconds == 60) {
			time_seconds = 0;
			++time_minutes;
			if (time_minutes == 60) {
				time_minutes = 0;
				// if we get to 256 hours the player has played too long :D
				++time_hours;
			}
		}
	}
}

void game_fade_to_black(void) {
	object_init();
	frame_counter = 0;
	game_state = GAME_STATE_FADE_BLACK;
}

const u8 fade_black[] = { 4, 3, 2, 1, 0, 0, 1, 2, 3, 4 };

#define FADE_FRAME_MID 5
#define FADE_FRAME_FINAL 9

void do_fade(void) {
	tmp1 = frame_counter >> 2;
	tmp2 = fade_black[tmp1];
	// white fades in the inverse direction (4 -> 8 -> 4)
	if (game_state != GAME_STATE_FADE_BLACK) {
		tmp2 = 8 - tmp2;
	}
	pal_bright(tmp2);
}

// defines how long fade in/out will take
// shifts the bits of the timer so it's multiplying or dividing by factors of 2
void main_fade(void) {
    do_fade();
    switch (tmp1) {
        case FADE_FRAME_MID: // pure black or white
            level_init();
            frame_counter = (6 << 2);
            break;
        case FADE_FRAME_FINAL:
            game_state = GAME_STATE_PLAY;
            frame_counter = 0;
	}
}

void reset_level(void) {
    level_last = 0xFF;

    // choose level
    #ifdef DEBUG
    level_current = LVL_DEBUG;
    camera_x = LVL_DEBUG_PAGE*256;
    #else
    level_current = LVL_START;
    camera_x = 0;
    #endif
}

void main_reset(void) {
    player_reset();

    player_health = player_health_max;
	frame_counter = 0;
	object_init();
    reset_level();

	game_state = GAME_STATE_FADE_WHITE;
}

void main_play(void) {
	// only advance game time when playing
	time_advance_frame();
	pal_bright(4);

	if (button_pressed(PAD_START)) {
		game_state = GAME_STATE_PAUSE;
		music_pause();
        sound_play_ch0(SFX_PAUSE);
	} else {
		profile(PROFILE_PLAYER);
		player_update();

		profile(PROFILE_OBJECTS);
		object_update();

		profile(PROFILE_LEVEL);
		level_update();

		profile(PROFILE_CLEAR);
	}
    // force sprites to clear early on fade
    if (game_state == GAME_STATE_FADE_BLACK) {
        oam_clear();
	}
}

void main_resume(void) {
	turn_off_ppu();
	load_level_gfx();
	level_load_initial();

	ppu_on_all();
	pal_bright(4);
	ppu_wait_nmi();
	game_state = GAME_STATE_PLAY;

	music_resume();
}

void main_pause() {
    if (button_pressed(PAD_START)) {
        game_state = GAME_STATE_PLAY;
        music_resume();
    }
}

const callbackDef game_states[GAME_STATE_NUM] = {
	main_play,
    main_reset,
	main_pause,
	main_fade,
	main_fade,
};

void run_frame(void) {
	++frame_counter;
	ppu_wait_nmi();

	profile(PROFILE_MISC);
    oam_clear();

	last_pad = pad;
	pad = pad_poll(0);
	LUT_CALL(game_states, game_state);

	profile(PROFILE_CLEAR); // make sure profile is cleared
	vram_buffer_flush();
}

void game_init() {
	set_mirroring(MIRROR_VERT);

	turn_off_ppu();
    player_init();
    reset_level();

    level_init();
    game_state = GAME_STATE_PLAY;
}

int main(void)
{
	oam_height_16(); // 8x16 sprites
	set_rand(0x1234);
    game_init();

	while(1) {
		run_frame();
	}
	return 0;
}
