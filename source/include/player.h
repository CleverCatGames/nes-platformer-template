#ifndef PLAYER_H
#define PLAYER_H

#include "lib/nes.h"
#include "lib/oam.h"
#include "sprites.h"

#define PLAYER_WIDTH  16 // should be 16
#define PLAYER_HEIGHT 16 // can be 16 or 32

#define PLAYER_STATE_CLEAR 0
#define PLAYER_STATE_WALL 0x01
#define PLAYER_STATE_OFFSCREEN 0x02
#define PLAYER_STATE_ON_GROUND 0x04
#define PLAYER_STATE_IN_LIQUID 0x08
#define PLAYER_STATE_INTERACT 0x10
#define PLAYER_STATE_BEHIND 0x20
#define PLAYER_STATE_LEFT OAM_FLIP_H // 0x40
#define PLAYER_STATE_PLATFORM 0x80

#define VELOCITY_X_DIV 8 // must be a multiple of 2
#define VELOCITY_Y_DIV 8

#define X_SPEED 2
#define MAX_RUN_VELOCITY 24
#define MAX_WALK_VELOCITY 16
#define MAX_SAND_VELOCITY 8
#define MAX_WATER_VELOCITY 12
#define MAX_Y_VELOCITY 64
#define MAX_WATER_DOWN_VELOCITY 8

#define JUMP_MAX_TIME 8
#define JUMP_WALK_VELOCITY 16
#define JUMP_RUN_VELOCITY 24
#define JUMP_WATER_MAX_VELOCITY 30
#define JUMP_SAND_VELOCITY 12

#define INVINCIBLE_HURT_TIME 60
#define DEATH_VELOCITY 40

#define SCREEN_DBL_WIDTH (SCREEN_WIDTH * 2)
#define HALFWAY_X (SCREEN_WIDTH / 2)
#define PLAYER_SCREEN_RIGHT ((HALFWAY_X + 16) - (PLAYER_WIDTH / 2))
#define PLAYER_SCREEN_LEFT ((HALFWAY_X - 16) - (PLAYER_WIDTH / 2))
#define BOUNDS_RIGHT                                                           \
  (SCREEN_WIDTH - PLAYER_WIDTH - 1) // very edge of right side of screen

#define PLAYER_IS_FALLING                                                      \
  (player_velocity_y > 0 && BIT_OFF(player_state, PLAYER_STATE_ON_GROUND))

#define COLLIDE_LEFT_OFFSET 3
#define COLLIDE_RIGHT_OFFSET (PLAYER_WIDTH - COLLIDE_LEFT_OFFSET)

// Calculates the velocity offset by taking the sign bit and shifting it
// This removes the need for branching
#define VELOCITY_OFFSET(value, bit) ((~(value) & 0x80) >> (bit))
#define VEL_X_OFFSET VELOCITY_OFFSET(velocity_x, 3) // 16 pixels

#define WITHIN(v, dist) (((v) < (dist)) || ((256 - (dist)) < (v)))
// #define WITHIN(v, dist) (((v) + (dist)) < (dist))

void player_init(void);
void player_reset(void);
void player_update(void);
void player_hurt(void);
void player_kill(void); // all health to zero

extern const u8 one_shift[8];

extern u8 player_x;
extern u8 player_y;
extern u8 player_state;
extern s8 player_velocity_y;
extern s8 player_velocity_x;
extern u8 player_invincible_timer;
extern u8 player_damage;
extern u8 player_jump_timer;
extern u8 player_health;
extern u8 player_health_max;

extern u8 target_x;
extern u8 target_y;

#define offsetof(type, element) ((u8) & (((type *)0)->element))
#define data_offset(element) player_data, offsetof(PlayerData, element)

#endif // PLAYER_H
