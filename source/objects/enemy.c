#include "draw.h"

#pragma code-name("OBJECTS")

void draw_enemy(void) {
    oam_attr = OAM_PAL4 | (object_state & OAM_FLIP_V);
    oam_tile = SPRITE_ENEMY;
    // bunny hop
    if (object_timer & 8) {
        --oam_y;
    }
    obj_draw_flipped();
}

void move_enemy(void) {
    if (BIT_ON(object_state, OBJ_FLAG_HIT)) {
        // if enemy is hit, move to bottom of screen and remove
        object_y += 3;
        if (object_y > SCREEN_HEIGHT) {
            object_type = OBJ_TYPE_NONE;
        }
    } else {
        move_walking();
    }
}
