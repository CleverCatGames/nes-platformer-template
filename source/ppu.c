// PPU/VRAM Helper functions

#include "var_defines.h"
#include "game.h"

// Schedules VRAM buffer for consumption by the ppu
void vram_buffer_flush(void) {
	if (vram_buffer_end > 0)
	{
		vram_buffer[vram_buffer_end] = NT_UPD_EOF;
		vram_buffer_end = 0;
		set_vram_update(vram_buffer);
	}
}

// Force VRAM buffer when PPU is turned off
// DO NOT USE WHILE PPU IS ON!
void flush_vram(void) {
	vram_buffer[vram_buffer_end] = NT_UPD_EOF;
	flush_vram_update(vram_buffer);
	vram_buffer_end = 0;
}

// Flushes VRAM buffer as soon as possible
// During pause this is immediate
// During gameplay this freezes for a frame
void vram_live_flush() {
    if (game_state == GAME_STATE_PAUSE) {
        flush_vram();
    } else {
        vram_buffer_flush();
        ppu_wait_nmi();
    }
}

void turn_off_ppu(void) {
	// black out screen for a frame
	pal_bright(0);
	ppu_wait_nmi();

	// clear vram buffer before turning off ppu
	flush_vram();
	ppu_off();
}

void turn_on_ppu(void) {
	ppu_wait_nmi();
	scroll(0, 0);
	ppu_on_all();
	pal_bright(4);
	ppu_wait_nmi();
}

