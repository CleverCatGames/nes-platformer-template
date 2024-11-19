#include "game.h"
#include "lib/oam.h"
#include "assert.h"

#ifdef DEBUG

#ifdef PRGRAM
#pragma bss-name("PRGRAM")
#endif

#include "gfx/font_debug.donut.h"

u8 a, x, y;

const u8 debug_pal[16] = {
	0x0f, 0x30, 0x0f, 0x0f,
	0x0f, 0x20, 0x0f, 0x0f,
	0x0f, 0x10, 0x0f, 0x0f,
	0x0f, 0x00, 0x0f, 0x0f,
};

void __fastcall__ print_int(u16 val);

static u8 i, c; // don't use zeropage
void print(const u8 *msg) {
	for (i = 0; c = msg[i]; ++i) {
		c -= (c <= 0x60) ? 0x20 : 0x40;
		vram_put(c);
	}
}

#define goto_line(line) vram_adr(NTADR_A(0, (line)));

#define print_line(line, msg) \
    goto_line((line)); \
	print((msg))

#define print_char(c) vram_put(ASC2NT((c)))

#define print_register(reg, val) \
    print_char(' '); \
    print_char((reg)); \
    print_char('='); \
    print_hex((val));

void __fastcall__ print_hex(u8 hex);
void __fastcall__ print_dump();

void print_registers(void) {
    print_register('A', a);
    print_register('X', x);
    print_register('Y', y);
}

void debug(const u8 *file, const u16 line) {
    // save registers
    __asm__("sta %v", a);
    __asm__("stx %v", x);
    __asm__("sty %v", y);

    // turn off the ppu
    __asm__("lda #0 ");
    __asm__("sta $2000");
    __asm__("sta $2001");

    // silence apu
    __asm__("sta $4000");
    __asm__("sta $4004");
    __asm__("sta $4008");
    __asm__("sta $400C");

    ppu_clear_vram(0);

    vram_adr(0x0000);

    // load font
    bank_switch(BANK_GFX);
    donut_bulk_load(chr_font_debug);

    // force palette
	pal_bkg(debug_pal);
	pal_spr(debug_pal);
    __asm__("jsr pal_to_ppu");

	print_line(2, "ASSERT!");

	print_line(4, file);
    print_char(':');
	print_int(line);

    goto_line(6);
    print_registers();

    goto_line(8);
    print_dump();

    // reset ppu latch
    __asm__("bit $2002");
    // reset scroll
    __asm__("lda #0 ");
    __asm__("sta $2005");
    __asm__("sta $2005");

    // turn on background
    __asm__("lda #$0a");
    __asm__("sta $2001");

	while(1);
}

#endif
