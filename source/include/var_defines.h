#ifndef VAR_DEFINES_H
#define VAR_DEFINES_H

#include "types.h"

#define MAX_OBJECTS 34  // max objects in a level
#define MAX_TRIGGERS 14 // max trigger areas in a level
#define MAX_SCREENS 16  // max screens wide a level can be
#define MAX_ACTIVE 10   // max active objects at any point

#define SCREEN_ROWS 7
#define SCREEN_COLS 8

// Store enough to update for 4 tiles
#define VRAM_BUFFER_SIZE 19*4+1

extern u8 vram_buffer[VRAM_BUFFER_SIZE];

// Zero page defines (created in zeropage.s)

extern const u8 *pTemp; // ONLY for temporary pointers (within scope of function)
#pragma zpsym ("pTemp");

extern const u8 *pTilePtr;
#pragma zpsym ("pTilePtr");

extern const u8 *pMapData;
#pragma zpsym ("pMapData");

extern const u8 *pLevelBuffer;
#pragma zpsym ("pLevelBuffer");

extern u8 object_state;
#pragma zpsym ("object_state");

extern u8 object_type;
#pragma zpsym ("object_type");

extern u8 object_timer;
#pragma zpsym ("object_timer");

// alternate tile for oam draw routines
extern u8 oam_tile_alt;
#pragma zpsym ("oam_tile_alt");

extern u8 obj_id;
#pragma zpsym ("obj_id");

extern u8 obj_active_id;
#pragma zpsym ("obj_active_id");

extern u8 collide_result;
#pragma zpsym ("collide_result");

extern u8 vram_buffer_end;
#pragma zpsym ("vram_buffer_end");

extern u8 vram_offset;
#pragma zpsym ("vram_offset");

extern u8 object_y;
#pragma zpsym ("object_y");

// --- TIME ---

extern u8 time_frames;
#pragma zpsym ("time_frames");

extern u8 time_seconds;
#pragma zpsym ("time_seconds");

extern u8 time_minutes;
#pragma zpsym ("time_minutes");

extern u8 time_hours;
#pragma zpsym ("time_hours");

#endif
