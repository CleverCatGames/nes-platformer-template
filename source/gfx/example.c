#include "gfx.h"

#pragma code-name("EXAMPLE")

#include "tilesets/example.meta.h"

void gfx_example(void) {
    // water surface
	VRAM_BUFFER_TILE(EXAMPLE_WATER_TL, 0);
	draw_water();
	vram_buffer_end = 19;
}

