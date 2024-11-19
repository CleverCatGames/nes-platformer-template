#ifndef OBJECT_H
#define OBJECT_H

#include "lib/nes.h"
#include "lib/oam.h"
#include "game.h"
#include "var_defines.h"

#include "data/object_types.h"

void object_init(void);
void object_update(void);

void object_load_vars(void);
void object_save_vars(void);

void obj_slow_timer();

// set object_state and reset object_timer
void __fastcall__ obj_set_state(u8 state);
// does BIT_SET(object_state, flag) and reset object_timer
void __fastcall__ obj_set_state_flag(u8 flag);
void __fastcall__ obj_and_state_flag(u8 flag);
#define obj_clear_state_flag(flag) obj_and_state_flag((u8)~(flag))
// increment object_state and reset object_timer
void __fastcall__ obj_inc_state();

void move_towards_player_x(void);
void move_towards_player(void);
void move_away_from_player(void);

// move on x-axis based on OAM_FLIP_H in object_state, also sets tile_get_x
void move_x(void);

// same as move_x but flips object_state if a solid tile is hit (like a wall)
void move_x_flip(void);

void move_walking(void);

// change object_state OAM_FLIP_H to face player
void obj_face_player(void);

// flip horizontally to face player
u8 object_to_object_collision(void);
void object_to_player_collision(void);

#define OBJ_TYPE_NONE 0xFF

#define OBJ_FLAG_NONE    0
#define OBJ_FLAG_BEHIND  0x20
#define OBJ_FLAG_FACING  OAM_FLIP_H // 0x40
#define OBJ_FLAG_HIT     OAM_FLIP_V // 0x80

#define OBJ_FLAG_JUMP    0x80

#define COLLIDE_IGNORED  0
#define COLLIDE_OCCURRED 1

#define OBJ_JUMP_TIMER 60

extern u16 object_x;

#define obj_id_16bit_in_y() \
    __asm__("lda %v", obj_id); \
    __asm__("asl a"); \
    __asm__("tay"); \

// faster way to get/set the 16 bit x value
#define OBJ_X_INTO(__x) { \
    __asm__("ldy %v", obj_id); \
    __asm__("lda %v, y", obj_x_l); \
    __asm__("sta %v", __x); \
    __asm__("lda %v, y", obj_x_h); \
    __asm__("sta %v+1", __x); \
}

#define OBJ_X_FROM(__x) { \
    __asm__("ldy %v", obj_id); \
    __asm__("lda %v", __x); \
    __asm__("sta %v, y", obj_x_l); \
    __asm__("lda %v+1", __x); \
    __asm__("sta %v, y", obj_x_h); \
}

// Faster way to call routines. Equates to __routine[object_type]();
// Checks if object_type is invalid before calling
#define OBJ_CALL(__routine) \
    if (object_type != OBJ_TYPE_NONE) { \
        LUT_CALL(__routine, object_type); \
    }

void collide_stomp(void);
void object_stomp(void);
void object_grounded(void);
void object_spawn_into_y(void);

#define gravity(amount) { \
    object_y += ((amount) % 16); \
    object_grounded(); \
}

extern u8 obj_x_l[MAX_OBJECTS];
extern u8 obj_x_h[MAX_OBJECTS];
extern u8 obj_y[MAX_OBJECTS];
extern u8 obj_timer[MAX_OBJECTS];
extern u8 obj_type[MAX_OBJECTS];
extern u8 obj_state[MAX_OBJECTS];

extern u8 obj_active[MAX_ACTIVE];

extern u8 num_active_objects;

extern const u8 obj_width[];
extern const u8 obj_height[];

#endif // OBJECT_H
