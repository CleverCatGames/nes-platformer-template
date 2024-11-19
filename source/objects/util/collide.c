#include "object.h"
#include "level.h"
#include "player.h"
#include "sfx.h"

#pragma code-name("OBJECTS")
#pragma rodata-name("OBJECTS")

void collide_hurt_player(void) {
    if (BIT_OFF(object_state, OBJ_FLAG_HIT)) {
        player_hurt();
    }
}

void object_stomp(void) {
	tmp1 = player_y + (PLAYER_HEIGHT/2);
	if (object_y > tmp1) {
		camera_shake = 10;
		obj_set_state_flag(OBJ_FLAG_HIT);
        sound_play_ch1(SFX_HURT);
		// try to avoid second collision
		player_y -= 8;
		// limit bounce back
		if (player_velocity_y > 32) player_velocity_y = 32;
		player_velocity_y = -player_velocity_y; // bounce player back in air
	}
}

void collide_stomp(void) {
	if (BIT_OFF(object_state, OBJ_FLAG_HIT)) {
		if (PLAYER_IS_FALLING) {
			object_stomp();
		} else {
			player_hurt();
		}
	}
}

