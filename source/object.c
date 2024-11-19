#include "assert.h"
#include "draw.h"
#include "level.h"
#include "object.h"
#include "player.h"

#include "data/object_types.impl.h"

extern const u8* const level_objs[];
extern u8 trigger_type[MAX_TRIGGERS];

void trigger_update();

void object_init(void) {
    memfill(obj_type, OBJ_TYPE_NONE, MAX_OBJECTS);
    memfill(trigger_type, OBJ_TYPE_NONE, MAX_TRIGGERS);
}

void object_load_vars(void) {
    // doing the following in assembly is faster than having to do obj_id lookups in c
    __asm__("ldy %v", obj_id);
    // object_timer = obj_timer[obj_id];
    __asm__("ldx %v,y", obj_timer);
    __asm__("stx %v", object_timer);
    // object_state = obj_state[obj_id];
    __asm__("lda %v,y", obj_state);
    __asm__("sta %v", object_state);
    // object_type = obj_type[obj_id];
    __asm__("lda %v,y", obj_type);
    __asm__("sta %v", object_type);
    // object_y = obj_y[obj_id];
    __asm__("lda %v,y", obj_y);
    __asm__("sta %v", object_y);
    // OBJ_X_INTO(object_x);
    // camera_diff.word = object_x - camera_x;
    __asm__("lda %v,y", obj_x_l);
    __asm__("sta %v", object_x);
    __asm__("sec");
    __asm__("sbc %v", camera_x);
    __asm__("sta %v", camera_diff);
    __asm__("lda %v,y", obj_x_h);
    __asm__("sta %v+1", object_x);
    __asm__("sbc %v+1", camera_x);
    __asm__("sta %v+1", camera_diff);
}

void object_save_vars(void) {
    // store temp object_* var back into obj_* array
    __asm__("ldy %v", obj_id);
    // obj_state[obj_id] = object_state
    __asm__("lda %v", object_state);
    __asm__("sta %v,y", obj_state);
    // obj_timer[obj_id] = object_timer
    __asm__("lda %v", object_timer);
    __asm__("sta %v,y", obj_timer);
    // obj_type[obj_id] = object_type
    __asm__("lda %v", object_type);
    __asm__("sta %v,y", obj_type);
    // obj_y[obj_id] = object_y
    __asm__("lda %v", object_y);
    __asm__("sta %v,y", obj_y);
    // OBJ_X_FROM(object_x);
    __asm__("lda %v", object_x);
    __asm__("sta %v,y", obj_x_l);
    __asm__("lda %v+1", object_x);
    __asm__("sta %v,y", obj_x_h);
}

#pragma code-name(push, "OBJECTS")

void object_to_player_collision(void) {
    collide_result = COLLIDE_IGNORED;
    // object left < player right
    __asm__("lda %v", player_x);
    __asm__("clc");
    __asm__("adc #%b", (u8)(COLLIDE_RIGHT_OFFSET-1));
    __asm__("sec");
    __asm__("sbc %v", camera_diff);
    __asm__("bpl %g", object_right);
    return;

object_right: // player left
    __asm__("ldy %v", object_type);
    __asm__("lda %v", camera_diff);
    __asm__("clc");
    __asm__("adc %v,y", obj_width);
    __asm__("bcs %g", exit_right); // avoid collision on screen right
    __asm__("sec");
    __asm__("sbc %v", player_x);
    __asm__("sbc #%b", (u8)COLLIDE_LEFT_OFFSET);
    __asm__("bpl %g", object_top);
exit_right:
    return;

object_top: // player bottom
    __asm__("lda %v", player_y);
    __asm__("clc");
    __asm__("adc #%b", (u8)(PLAYER_HEIGHT-1));
    __asm__("sec");
    __asm__("sbc %v", object_y);
    __asm__("bpl %g", object_bottom);
    return;

object_bottom: // player top
    __asm__("lda %v", object_y);
    __asm__("clc");
    __asm__("adc %v,y", obj_height);
    __asm__("sec");
    __asm__("sbc #8"); // avoid collision with top of the head
    __asm__("sbc %v", player_y);
    __asm__("bpl %g", do_collide);
    return;

do_collide:
    ++collide_result;
}

// reverse active list in place
// flips values from the outside in until it hits the midpoint
void reverse_active_objects(void) {
#define mid_point tmp1
    __asm__("ldx #0");
    __asm__("ldy %v", obj_active_id);
    __asm__("tya");
    __asm__("lsr");
    __asm__("sta %v", mid_point); // (obj_active_id - 1) / 2
    __asm__("dey");

loop:
    // check if X has hit the mid point
    __asm__("txa");
    __asm__("sec");
    __asm__("sbc %v", mid_point);
    __asm__("bne %g", swap);
    return;

swap:
    __asm__("lda %v,y", obj_active);
    __asm__("pha");
    __asm__("lda %v,x", obj_active);
    __asm__("sta %v,y", obj_active);
    __asm__("pla");
    __asm__("sta %v,x", obj_active);

    // bring X and Y closer to the center of the list
    __asm__("dey");
    __asm__("inx");
    __asm__("jmp %g", loop);
}

void object_find_active(void) {
    // loop through all possible objects
    for (obj_id = 0; obj_id < MAX_OBJECTS; ++obj_id)
    {
        __asm__("ldy %v", obj_id);
        __asm__("lda %v,y", obj_type);
        __asm__("sta %v", object_type);
        // if the object isn't active, skip it
        if (object_type == OBJ_TYPE_NONE) continue;

        // check if the object is on screen or next to it
        tmp1 = obj_x_h[obj_id] - MSB(camera_x);
        ++tmp1; // change range from (-1 - 2) to (0 - 3)
        if (tmp1 >= 3) {
            continue;
        }

        obj_active[obj_active_id] = obj_id;
        ++obj_active_id;
        assert(obj_active_id <= MAX_ACTIVE);
        if (obj_active_id == MAX_ACTIVE) break;
    }

    num_active_objects = obj_active_id;
    if (obj_active_id) {
        if (frame_counter & 1) {
            reverse_active_objects();
        }
    }
}

void object_update_active() {
    for (obj_active_id = 0; obj_active_id < num_active_objects; ++obj_active_id) {
        obj_id = obj_active[obj_active_id];

        object_load_vars();

        // move the object
        if (game_state == GAME_STATE_PLAY) {
            ++object_timer;
            OBJ_CALL(move_routines);
        }

        // update camera diff after move
        camera_diff.word = object_x - camera_x;

        // check if object is on screen
        if (camera_diff_page == 0)
        {
            // check that player is alive and not invincible
            if (player_health != 0)
            {
                object_to_player_collision();
                if (collide_result == COLLIDE_OCCURRED) {
                    player_damage = 1;
                    OBJ_CALL(collide_routines);
                }
            }

            // draw routines (check if exist?)
            SET_SPRITE_OAM_POS();
            OBJ_CALL(draw_routines);
        }
        else
        {
            tmp1 = obj_width[object_type];
            tmp1 = 7 - tmp1; // right side
            if (camera_diff_page == 0xFF && camera_diff_x > tmp1)
            {
                SET_SPRITE_OAM_POS();
                OBJ_CALL(draw_routines);
            }
        }

        object_save_vars();
    }
}

#pragma code-name(pop) // OBJECTS bank

void object_update() {
    obj_active_id = 0;
    bank_switch(BANK_OBJECTS);
    profile(PROFILE_MISC);
    trigger_update();
    profile(PROFILE_OBJECTS);
    object_find_active();
    object_update_active();
}
