#include "gfx.h"
#include "player.h"
#include "gfxdata.h"
#include "draw.h"
#include "sfx.h"
#include "ppu.h"

// (level_zone*2) for referencing addresses
#define zone_offset tmp1

#define zone_load(__zone) { \
	__asm__("ldy %v", zone_offset); \
	__asm__("lda %v,y", __zone); \
	__asm__("ldx %v+1,y", __zone); \
	__asm__("jsr %v", donut_bulk_load); \
}

#define zone_pal(__row, __data) { \
	__asm__("ldy %v", zone_offset); \
	__asm__("lda %v+1,y", __data ); \
    __asm__("sta tmp2"); \
	__asm__("lda %v,y", __data ); \
    __asm__("ldy tmp2"); \
    __asm__("ldx #%b", (((__row) * 4) + 1)); \
    __asm__("jsr _pal_row_copy"); \
}

#define zone_bkg(__data) { \
	__asm__("ldy %v", zone_offset); \
	__asm__("lda %v,y", __data); \
	__asm__("ldx %v+1,y", __data); \
	__asm__("jsr _pal_bkg"); \
}

#define zone_ptr(__ptr, __data) { \
	__asm__("ldy %v", zone_offset); \
	__asm__("lda %v,y", __data); \
	__asm__("sta %v", __ptr); \
	__asm__("lda %v+1,y", __data); \
	__asm__("sta %v+1", __ptr); \
}

void load_level_gfx(void) {
    // word offset for lookups
	zone_offset = level_zone << 1;

    bank_switch(BANK_GFX);

    // load player sprites
	vram_adr(0x1000);
	donut_bulk_load(chr_player);

    // load palette data
    pal_row(PAL_SPR_1, pal_player_default);
    zone_pal(PAL_SPR_3, spr_pal1);
    zone_pal(PAL_SPR_4, spr_pal2);

	// load zone graphics from level bank
	bank_switch(level_bank);
	zone_bkg(zone_palette);

	zone_ptr(pMetatileData, zone_metatiles);
	zone_ptr(pMetatileAttr, zone_attrs);

	zone_load(zone_object);

	vram_adr(0x0000);
	zone_load(zone_tileset);
}

