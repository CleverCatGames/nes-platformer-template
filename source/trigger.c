#include "level.h"
#include "object.h"
#include "player.h"
#include "trigger.h"
#include "draw.h"

#pragma code-name("OBJECTS")
#pragma rodata-name("OBJECTS")

#include "data/trigger.impl.h"

extern u8 trigger_x_l[MAX_TRIGGERS];
extern u8 trigger_x_h[MAX_TRIGGERS];
extern u8 trigger_y[MAX_TRIGGERS];
extern u8 trigger_type[MAX_TRIGGERS];
extern u8 trigger_state[MAX_TRIGGERS];

void script_collide(void);

#define TRIGGER_WIDTH 32

#define TRIGGER_CAMERA_DIFF() { \
    __asm__("lda %v,y", trigger_x_l); \
    __asm__("sec"); \
    __asm__("sbc %v", camera_x); \
    __asm__("sta %v", camera_diff); \
    __asm__("lda %v,y", trigger_x_h); \
    __asm__("sbc %v+1", camera_x); \
    __asm__("sta %v+1", camera_diff); \
}

void trigger_save_vars(void) {
    __asm__("ldy %v", trigger_id);

    __asm__("lda %v", object_state);
    __asm__("sta %v,y", trigger_state);

    __asm__("lda %v", object_type);
    __asm__("sta %v,y", trigger_type);

    __asm__("lda %v", object_y);
    __asm__("sta %v,y", trigger_y);

    __asm__("lda %v", object_x);
    __asm__("sta %v,y", trigger_x_l);
    __asm__("lda %v+1", object_x);
    __asm__("sta %v,y", trigger_x_h);
}

void trigger_update() {
    for (obj_id = 0; obj_id < MAX_TRIGGERS; ++obj_id) {

        // if the object isn't active, skip it
        object_type = trigger_type[obj_id];
        if (object_type == OBJ_TYPE_NONE) continue;

        // y register should already contain obj_id
        __asm__("lda %v,y", trigger_state);
        __asm__("sta %v", object_state);
        TRIGGER_CAMERA_DIFF();

        // check collision with player
        camera_diff.word += ((TRIGGER_WIDTH + PLAYER_WIDTH) / 2);

        __asm__("lda %v", camera_diff);
        __asm__("sec");
        //__asm__("sbc #%b", (TRIGGER_WIDTH/2 - PLAYER_WIDTH/2));
        __asm__("sbc %v", player_x);
        __asm__("sta %v", tmp1);
        __asm__("lda %v+1", camera_diff); // page offset
        __asm__("sbc #0");
        __asm__("beq %g", check_x); // if zero it means we're on the same page as the trigger
        continue;
    check_x:
        __asm__("lda %v", tmp1); // this should be 0-TRIGGER_WIDTH for a collision
        __asm__("cmp #%b", TRIGGER_WIDTH+1);
        __asm__("bcc %g", collide);
        continue;
    collide:

        switch (object_type) {
            case TRIGGER_LEVEL_ZONE:
                level_next = object_state;
                break;
            case TRIGGER_CAMERA_LOCK:
                camera_page_min = (object_state >> 4);
                camera_page_max = object_state & 0x0F;
                break;
        }
    }
}
