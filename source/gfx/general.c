#include "gfx.h"
#include "ppu.h"

// Sprite for top 2 pixels of water suface
const u8 water_surface[] = {
	0x1f, 0x8f, 0xc7, 0xe3, 0xf1, 0xf8, 0x7c, 0x3e,
	0x3e, 0x7c, 0xf8, 0xf1, 0xe3, 0xc7, 0x8f, 0x1f,
};

void on_off_update(void) {
    memcpy(vram_buffer, pTemp, vram_buffer_end);
}

void draw_water(void) {
	vram_buffer[5] = 0x00;
	vram_buffer[6] = 0x00;
	vram_buffer[7] = 0x00;
	vram_buffer[8] = 0x00;
	vram_buffer[9] = 0x00;
	vram_buffer[10] = 0x00;

	vram_buffer[12] = 0xff;
	vram_buffer[13] = 0xff;
	vram_buffer[14] = 0xff;
	vram_buffer[15] = 0xff;
	vram_buffer[16] = 0xff;
	vram_buffer[17] = 0xff;
	vram_buffer[18] = 0xff;
	vram_buffer[19] = 0xff;
	vram_buffer[20] = 0xff;

	// use top 4 bits of the frame counter to pull water surface data
	tmp2 = frame_counter >> 4;
	tmp3 = water_surface[tmp2];
	vram_buffer[3] = tmp3;
	vram_buffer[11] = tmp3;
	vram_buffer[4] = tmp3 ^ 0xff;  // invert tmp3 to get the reverse pattern
}

void draw_vertical_moving(void) {
	vram_buffer[2] = 16;

	// calculate row offset
	// tmp2 = 7 - ((frame_counter >> 2) % 8)
	tmp2 = frame_counter >> 2;
	tmp2 %= 8;
	tmp2 = 7 - tmp2;

	for (tmp1 = 0; tmp1 < 8; ++tmp1)
	{
		// vram_buffer[tmp1 + 3] = tile_ptr[tmp2]
		__asm__("tax"); // tmp1 from loop
		__asm__("ldy %v", tmp2);
		__asm__("lda (%v),y", tmp4); // tile data
		__asm__("sta %v+3,x", vram_buffer);

		// vram_buffer[tmp1 + 11] = tile_ptr[tmp2 + 8]
		__asm__("tya");
		__asm__("clc");
		__asm__("adc #8");
		__asm__("tay");
		__asm__("lda (%v),y", tmp4);
		__asm__("sta %v+11,x", vram_buffer);

		// shift row offset by 1
		__asm__("iny");
		__asm__("tya");
		__asm__("and #7");
		__asm__("sta %v", tmp2);

	}
	vram_buffer_end = 19;
}

