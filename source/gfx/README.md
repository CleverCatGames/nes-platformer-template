# Graphics Library

Contains code for updating PPU graphics.

When level scrolling is idle graphics routines will be performed. This is because the vram_buffer is available for use. Otherwise you have to be extremely careful to not clobber the exiting vram_buffer data.

Every area is contained within the `assets/data/zones.csv` file. This also specifies the gfx routine to run when the level is idle.

You can see that I've used this to update the water sprite in `example.c`.

