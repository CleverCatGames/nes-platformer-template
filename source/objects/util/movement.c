/**
 * @file movement.c
 * @brief Object movement functions
 * Similar to spawn.c functions, these are set in object_types.csv
 */

#include "object.h"
#include "level.h"
#include "player.h"

#pragma code-name("OBJECTS")
#pragma rodata-name("OBJECTS")

u8 object_to_object_collision(void) {
    // x = active object counter
    // y = object id
    __asm__("ldx #0");
    // only check collisions while on screen
collision_loop:
    // don't collide with self
    __asm__("ldy %v,x", obj_active);
    __asm__("cpy %v", obj_id);
    __asm__("beq %g", next);

    // don't collide with inactive objects or level zones
    __asm__("lda %v,y", obj_type);
    __asm__("sta %v", tmp3);
    __asm__("cmp #%b", (u8)OBJ_TYPE_NONE);
    __asm__("beq %g", next);

    // need to get these values before we clobber the y register
    __asm__("lda %v,y", obj_x_l);
    __asm__("sta %v", tmp1);

    __asm__("lda %v,y", obj_y);
    __asm__("sta %v", tmp2);

    // check if the object is on screen
    __asm__("lda %v,y", obj_x_h);
    __asm__("cmp %v+1", object_x);
    __asm__("bne %g", next);

    // first x-axis check
    __asm__("ldy %v", object_type);
    __asm__("lda %v,y", obj_width);
    __asm__("clc");
    __asm__("adc %v", object_x);
    __asm__("cmp %v", tmp1); // obj_x_l from earlier
    __asm__("bcc %g", next); // technically this is off by one and should check beq but let's save space...

    // first y-axis check
    __asm__("lda %v,y", obj_height);
    __asm__("clc");
    __asm__("adc %v", object_y);
    __asm__("cmp %v", tmp2); // obj_y from earlier
    __asm__("bcc %g", next);

    // second x-axis check
    __asm__("ldy %v", tmp3); // object type
    __asm__("lda %v,y", obj_width);
    __asm__("clc");
    __asm__("adc %v", tmp1); // obj_x_l
    __asm__("cmp %v", object_x);
    __asm__("bcc %g", next);

    // second y-axis check
    __asm__("ldy %v", tmp3); // object type
    __asm__("lda %v,y", obj_height);
    __asm__("clc");
    __asm__("adc %v", tmp2); // obj_y
    __asm__("cmp %v", object_y);
    __asm__("bcc %g", next);

    return COLLIDE_OCCURRED;

next:
    __asm__("inx");
    __asm__("cpx #%b", MAX_ACTIVE);
    __asm__("bne %g", collision_loop);
    return COLLIDE_IGNORED;
}

void object_grounded(void) {
    tile_get_y = object_y + obj_height[object_type];
    tile_get_x = object_x + 8; // center x-axis
    get_tile();
    if (TILE_IS_COLLIDE || tile_liquid) {
        object_y &= 0xF0; // clamp to tile (16 pixels)
    }
}

/**
 * @brief No movement (object default)
 */
void move_stationary(void) { }

/**
 * @brief moves away from the player
 * Ignores collision with walls. Useful for flying enemy.
 */
void move_away_from_player(void) {
    if (camera_diff_x < player_x) {
        --object_x;
    } else if (camera_diff_x > player_x) {
        ++object_x;
    }

    tmp1 = object_y - 8;
    if (tmp1 < player_y) {
        --object_y;
    } else if (tmp1 > player_y) {
        ++object_y;
    }
}

void move_towards_player_x(void) {
    if (camera_diff_x < player_x) {
        ++object_x;
    } else if (camera_diff_x > player_x) {
        --object_x;
    }
}

/**
 * @brief follows the player
 * Ignores collision with walls. Useful for flying enemy.
 */
void move_towards_player(void) {
    tmp1 = object_y - 8;
    if (tmp1 < player_y) {
        ++object_y;
    } else if (tmp1 > player_y) {
        --object_y;
    }

    move_towards_player_x();
}

void move_x(void) {
    tile_get_x = object_x;
    if (BIT_ON(object_state, OAM_FLIP_H)) {
        ++object_x;
        tile_get_x += 16;
    } else {
        --object_x;
    }
}

void move_x_flip(void) {
    move_x();
    // move away from wall
    get_tile();
    if (MT_IS(SOLID)) {
        BIT_FLIP(object_state, OAM_FLIP_H);
    }
}

/**
 * @brief walks to right or left and turns around when hitting a wall
 * Object falls off ledges and keep going until either hitting a wall, falling off the level, or dying
 * This assumes the object is 16x16
 */
void move_walking(void) {
    if ((frame_counter ^ obj_id) & 1) return;
    if (BIT_OFF(object_state, OBJ_FLAG_HIT)) {
        OBJ_X_INTO(tile_get_x);

        // move on x-axis
        if (BIT_OFF(object_state, OBJ_FLAG_FACING)) {
            --tile_get_x;
        } else {
            tile_get_x += 17; // TODO: use obj_width?
        }
        tile_get_y = object_y + 3;
        get_tile();
        if (TILE_IS_COLLIDE) {
            BIT_FLIP(object_state, OBJ_FLAG_FACING);
        } else {
            if (MT_IS(FRONT)) {
                BIT_SET(object_state, OBJ_FLAG_BEHIND);
            }
            // check tile below for floor
            get_tile_next_y();
            // clamp y-axis on collision
            if (TILE_IS_COLLIDE) {
                object_y &= 0xF0;
                if (BIT_ON(object_state, OBJ_FLAG_FACING)) {
                    tile_get_x -= 16;
                }
                object_x = tile_get_x;
            } else {
                // flip when about to fall
                BIT_FLIP(object_state, OBJ_FLAG_FACING);
            }
        }
    }
    if (object_y > 240) {
        object_type = OBJ_TYPE_NONE;
    }
}
