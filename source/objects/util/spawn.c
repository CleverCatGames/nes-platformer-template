/**
 * @file spawn.c
 * @brief Contains functions that execute when objects spawn
 * Object spawn functions are set in assets/data/object_types.csv
 * The csv file is processed through tools/csv2h.py and generates two files.
 *   out/assets/data/object_types.h        header defines
 *   out/assets/data/object_types.impl.h   table data
 */

#include "object.h"
#include "player.h"
#include "level.h"

#pragma code-name("OBJECTS")
#pragma rodata-name("OBJECTS")

/**
 * @brief Normal spawn (used by default)
 * This sets the object timer to zero when spawning.
 */
void spawn_normal(void) {
}

/**
 * @brief Stores y position in object_state
 * This allows objects to restore to a specific y position at a later time.
 * Used for beer bottles and buzzards to know where they spawn at.
 */
void spawn_store_y(void) {
	object_state = object_y & 0xF0;
}

void spawn_bat(void) {
    object_y += 2;
}

/**
 * @brief Spawn for level zone trigger
 * If this is the trigger for the last level the player exited, move the player to this trigger area and reset the camera.
 * This also sets the zone y position to zero since the trigger zone is fills the entire screen
 */
void trigger_spawn_level_zone(void) {
    if (level_last == object_state) {
        // set up boundaries
        if (BIT_ON(level_flags, LEVEL_FLAGS_PAGE_LOCK)) {
            camera_page_min = MSB(object_x);
            camera_page_max = camera_page_min;
        }

        // set player_y
        __asm__("lda %v", object_y);
        __asm__("sta %v", player_y);
        move_camera_to_object_x(8);
    }
    obj_y[obj_id] = 0;
}
void trigger_spawn_normal(void) {}

/**
 * @brief Flame spawn (x+4 offset)
 * The flame sprite is only 8 pixels wide so it is centered to the right by 4 pixels
 */
void spawn_x_plus_4(void) {
    object_x += 4;
}

void spawn_x_plus_8(void) {
    object_x += 8;
}

void object_spawn_into_y(void) {
    __asm__("ldy #%b", (u8)(MAX_OBJECTS-1));
spawn_loop:
    __asm__("lda %v,y", obj_type);
    __asm__("cmp #%b", (u8)OBJ_TYPE_NONE);
    __asm__("beq %g", spawn_exit);
    __asm__("dey");
    __asm__("bne %g", spawn_loop);
spawn_exit:
    __asm__("lda %v+1", object_x); // same page as object
    __asm__("sta %v,y", obj_x_h);

    __asm__("lda #0"); // clear timer and state
    __asm__("sta %v,y", obj_state);
    __asm__("sta %v,y", obj_timer);
}

