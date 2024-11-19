#ifndef DRAW_H
#define DRAW_H

#include "object.h"
#include "lib/oam.h"
#include "sprites.h"

// set oam position (y offset minus one)
// oam_x = camera_diff.byte.x
// oam_y = obj_y[obj_id] - 1
#define SET_SPRITE_OAM_POS() { \
    oam_x = camera_diff_x; \
    __asm__("ldx %v", object_y); \
    __asm__("dex"); \
    __asm__("stx %v", oam_y); \
}

void wave_oam_y(void);

/**
 * Draws two sprites in sequence horizontally
 * If one sprite is off screen it will skip drawing that sprite
 * oam_tile MUST be set before calling!
 *
 * | A> | B> |
 *
 */
void obj_draw_clipped(void);
void obj_draw_clipped2(void);

/**
 * Draws the same sprite twice horiztonally but the second is flipped. Useful for horizontally mirrored sprites.
 * If one sprite is off screen it will skip drawing that sprite.
 * oam_tile MUST be set before calling!
 *
 * | A> | <A |
 *
 */
void obj_draw_flipped(void);

/**
 * Draws a tile pair and flips every so many frames
 * oam_tile MUST be set before calling!
 */
void obj_draw_square_flip(void);

void obj_draw_square_animate(void);

void obj_draw_mirror_24(void);

#endif // DRAW_H
