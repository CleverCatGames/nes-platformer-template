#ifndef LEVEL_H
#define LEVEL_H

#include "var_defines.h"

#include "levels/levels.enum.h"

#define MT_PAL0 0x00
#define MT_PAL1 0x40
#define MT_PAL2 0x80
#define MT_PAL3 0xC0
#define MT_PAL_MASK 0xC0

enum {
	MT_TYPE_NOCOL = 0,  // no collision
	MT_TYPE_SOLID,      // collision
	MT_TYPE_HURT,       // hurt player
	MT_TYPE_DOOR,       // go to another level
	MT_TYPE_ONEWAY,     // only collide when moving down
	MT_TYPE_ON_OFF,     // on/off block
	MT_TYPE_FRONT,      // display in front of player
};
#define MT_SOLID (1 << MT_TYPE_SOLID)
#define MT_HURT (1 << MT_TYPE_HURT)
#define MT_DOOR (1 << MT_TYPE_DOOR)
#define MT_ONEWAY (1 << MT_TYPE_ONEWAY)
#define MT_ON_OFF (1 << MT_TYPE_ON_OFF)
#define MT_FRONT (1 << MT_TYPE_FRONT)

#define MT_TYPE_MASK 0x0F
#define MT_TYPE_LIQUID 0x10     // water and quicksand

#define LEVEL_FLAGS_NONE      0x00
#define LEVEL_FLAGS_EXIT_MASK 0x07
#define LEVEL_FLAGS_EXIT      0x08
#define LEVEL_FLAGS_PAL_DARK  0x40
#define LEVEL_FLAGS_PAGE_LOCK 0x80

#define LEVEL_FLAGS_LIQUID_MASK  0x30

#define LIQUID_NONE      0x00
#define LIQUID_WATER     0x10
#define LIQUID_QUICKSAND 0x20
#define LIQUID_TOXIC     0x30

enum {
	LEVEL_FLAGS_EXIT_UP,
	LEVEL_FLAGS_EXIT_DOWN,
	LEVEL_FLAGS_EXIT_LEFT,
	LEVEL_FLAGS_EXIT_RIGHT,
	LEVEL_FLAGS_EXIT_VERT,
	LEVEL_FLAGS_EXIT_HORIZ,
	LEVEL_FLAGS_EXIT_UP_LEFT,
	LEVEL_FLAGS_EXIT_DOWN_RIGHT,
};

#define LEVEL_EXIT_R 0x01
#define LEVEL_EXIT_L 0x02
#define LEVEL_EXIT_D 0x04
#define LEVEL_EXIT_U 0x08

#define LEVEL_INVALID 0x3F

#define LVL_ALTERNATE 0x80
#define LVL_LOCKED    0x40 // used by level transition objects
#define LVL_MASK      0x3F

#define OBJ_EXTRA_DATA 0x40
#define OBJ_NEW_PAGE 0x80

#define MAPXY(x, y) (((x) << 2) + (y & 3))

#define OBJ_POS(x, y) (((x) << 4) + (y))
#define OBJ_RANGE(min, max) OBJ_POS(min, max)

void level_init(void);
void level_update(void);
void level_load_next_object(void);
void level_gfx(void);

void init_map_pointers(void);

void camera_move(void);

/**
 * Pointer to tile indexes in metatile
 */
extern const u8 *pMetatileData;
#pragma zpsym("pMetatileData");
/**
 * Each metatile contains a single byte attribute field
 * ppxxtttt
 * pp = 2-bit palette value (MT_PAL0-3)
 * xx = 2-bit unused
 * tttt = 4-bit type (defined as MT_TYPE_*)
 */
extern const u8 *pMetatileAttr;
#pragma zpsym("pMetatileAttr");

extern u8 level_last;
extern u8 level_current;
extern u8 level_next;
extern u8 level_zone;
extern u8 level_bank;
extern u8 level_x;
extern u8 level_pages;
extern u8 level_flags;
extern u8 level_exit;
extern u16 tile_get_x;
extern u8 tile_get_y;
extern u8 tile_type;
extern u8 tile_liquid;
extern u8 trigger_id;
// set tile_x and tile_y before calling get_tile
void get_tile(void);
void get_tile_next_y(void);

extern u8 map_screens[MAX_SCREENS];

#define GET_TILE(x, y) { tile_get_x = (x); tile_get_y = (y); get_tile(); }
#define MT_IS(t) BIT_ON(tile_type, (MT_ ## t))
#define MT_IS_NOT(t) BIT_OFF(tile_type, (MT_ ## t))

#define SOLID_TYPES (MT_SOLID | MT_ONEWAY)
// must call get_tile before calling is_collide
#define TILE_IS_COLLIDE BIT_ON(tile_type, SOLID_TYPES)
#define TILE_IS_NOT_COLLIDE BIT_OFF(tile_type, SOLID_TYPES)

// where did you come from? where did you go?
enum {
	LEVEL_LAST_UP_EXIT = 0xFB,
	LEVEL_LAST_DOWN_EXIT = 0xFC,
	LEVEL_LAST_LEFT_EXIT = 0xFD,
	LEVEL_LAST_RIGHT_EXIT = 0xFE,
};

// used by level and dialogue
extern const u16 screen_to_tile_offset[];
extern const u8 column_to_tile_offset[];
// defined in megatile.s
void __fastcall__ load_megatile_buffer(u8 index);
u8 __fastcall__ load_megatile(u8 offset);
void __fastcall__ megatile_unrle();

void world_to_tile(void);
void megatile_to_vram(u8 offset);
void megatile_attr(u8 offset);

#endif // LEVEL_H
