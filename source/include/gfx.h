#ifndef GFX_H
#define GFX_H

#include "game.h"
#include "level.h"

#define VRAM_BUFFER_UPPER(tile_pos, offset) \
	vram_buffer[(offset)] = (((tile_pos) >> 4) | NT_UPD_HORZ);

#define VRAM_BUFFER_LOWER(tile_pos, offset) \
	vram_buffer[(offset)+1] = (((tile_pos) << 4) & 0xF0);

#define VRAM_BUFFER_START(tile_pos, offset) \
	VRAM_BUFFER_UPPER(tile_pos, offset) \
	VRAM_BUFFER_LOWER(tile_pos, offset)

#define VRAM_BUFFER_TILE(tile_pos, offset) \
	VRAM_BUFFER_START(tile_pos, offset*19) \
	vram_buffer[(offset*19)+2] = 16;

// do all upper writes first to potentially save instructions
#define TILE_ONOFF(tileset) \
	on_off_update(); \
	if (vram_buffer_end) { \
		VRAM_BUFFER_UPPER(tileset ## _ONOFF_TL, 0); \
		VRAM_BUFFER_UPPER(tileset ## _ONOFF_TR, 19); \
		VRAM_BUFFER_LOWER(tileset ## _ONOFF_TL, 0); \
		VRAM_BUFFER_LOWER(tileset ## _ONOFF_TR, 19); \
	}

#define TILE_VERTICAL_ROLL(tile_pos, tile) \
	VRAM_BUFFER_START(tile_pos, 0) \
	__asm__("lda #<(%v) ", tile); \
	__asm__("sta %v", tmp4); \
	__asm__("ldx #>(%v) ", tile); \
	__asm__("stx %v+1", tmp4); \
	draw_vertical_moving();

#define ON_OFF_BLOCK_SIZE 19*2

extern const u8 water_surface[];
void on_off_update(void);
void draw_vertical_moving(void);
void draw_water(void);

#endif // GFX_H
