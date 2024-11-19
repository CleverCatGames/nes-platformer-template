#ifndef PPU_H
#define PPU_H

void flush_vram(void);
void vram_live_flush(void);
void vram_buffer_flush();

void turn_off_ppu(void);
void turn_on_ppu(void);

#endif
