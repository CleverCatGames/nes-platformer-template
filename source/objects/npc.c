#include "draw.h"

#pragma code-name("OBJECTS")

void draw_npc(void) {
    oam_attr = OAM_PAL3;
    oam_tile = SPRITE_PLAYER;
    obj_draw_square_animate();
}

