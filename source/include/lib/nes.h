// NES hardware-dependent functions by Shiru (shiru@mail.ru)
// Feel free to do anything you want with this code, consider it Public Domain

// Versions history:
//  280215 - fixed palette glitch caused with the active DMC DMA glitch
//  030914 - minor fixes in the vram update system
//  310814 - added vram_flush_update
//  120414 - removed adr argument from vram_write and vram_read,
//  060414 - many fixes and improvements, including sequental VRAM updates
//  previous versions were created since mid-2011, there were many updates

#ifndef NESLIB_H
#define NESLIB_H

#include "types.h"

#define PAD_A			0x01
#define PAD_B			0x02
#define PAD_SELECT		0x04
#define PAD_START		0x08
#define PAD_UP			0x10
#define PAD_DOWN		0x20
#define PAD_LEFT		0x40
#define PAD_RIGHT		0x80

#define MAX(x1,x2)		(((x1)<(x2))?(x2):(x1))
#define MIN(x1,x2)		(((x1)<(x2))?(x1):(x2))
#define ABS(x)			((x) > 0 ? (x) : -(x))

// bit test functions
#define BIT_ON(a, b)    (((a) & (b)) != 0)
#define BIT_OFF(a, b)   (((a) & (b)) == 0)
// bit flipping functions
#define BIT_SET(a, b)   ((a) |= (b))
#define BIT_CLEAR(a, b) ((a) &= ~(b))
#define BIT_FLIP(a, b)  ((a) ^= (b))

#define STACK_PUSH(val) { \
    __asm__("lda %v", val); \
    __asm__("pha"); \
}

#define STACK_POP(val) { \
    __asm__("pla"); \
    __asm__("sta %v", val); \
}

#define MASK_SPR		0x10 // enable sprites
#define MASK_BG			0x08 // enable background
#define MASK_SPR_EDGE	0x04 // enable sprites in first 8 pixels
#define MASK_BG_EDGE	0x02 // enable background in first 8 pixels
#define MASK_BG_ALL		MASK_BG|MASK_BG_EDGE
#define MASK_SPR_ALL	MASK_SPR|MASK_SPR_EDGE
#define MASK_ALL		MASK_BG_ALL|MASK_SPR_ALL

#define ZP_BYTE(x)     (*((unsigned char *)(x)))
#define ZP_WORD(x)     (*((unsigned int *)(x)))


extern u8 tmp1;
#pragma zpsym ("tmp1");
extern u8 tmp2;
#pragma zpsym ("tmp2");
extern u8 tmp3;
#pragma zpsym ("tmp3");
extern u8 tmp4;
#pragma zpsym ("tmp4");
extern u8 tmp5;
#pragma zpsym ("tmp5");

// nametable layout
// +---+---+
// | A | B |
// +---+---+
// | C | D |
// +---+---+

#define NAMETABLE_A		0x2000
#define NAMETABLE_B		0x2400
#define NAMETABLE_C		0x2800
#define NAMETABLE_D		0x2c00

#define NULL			0
#define TRUE			1
#define FALSE			0

#define NT_UPD_HORZ		0x40
#define NT_UPD_VERT		0x80
#define NT_UPD_EOF		0xff

#define PAL_BKG_1 0
#define PAL_BKG_2 1
#define PAL_BKG_3 2
#define PAL_BKG_4 3
#define PAL_SPR_1 4
#define PAL_SPR_2 5
#define PAL_SPR_3 6
#define PAL_SPR_4 7

#define MIRROR_ONE 0
#define MIRROR_VERT 2
#define MIRROR_HORIZ 3

// macro to calculate nametable address from X,Y in compile time

#define NTADR_A(x,y) 	(NAMETABLE_A | (((y) * 32) | (x)))
#define NTADR_B(x,y) 	(NAMETABLE_B | (((y) * 32) | (x)))
#define NTADR_C(x,y) 	(NAMETABLE_C | (((y) * 32) | (x)))
#define NTADR_D(x,y) 	(NAMETABLE_D | (((y) * 32) | (x)))

#define ASC2NT(x)       ((x) - 0x20)

// macro to get MSB and LSB

#define MSB(x)			((u8)((x) / 256))
#define LSB(x)			((u8)((x) % 256))

#define LUT_CALL_STATE(__states) \
    __asm__("asl a"); /* multiply by 2 */ \
    __asm__("tay"); \
    __asm__("ldx %v+1,y", __states); \
    __asm__("lda %v,y", __states); \
    __asm__("jsr callax")

// Faster way to call routines. Equates to __states[__index]();
// __index MUST be less than 128
// __states MUST be a table of void(void) functions
#define LUT_CALL(__states, __index) \
    __asm__("lda %v", __index); \
    LUT_CALL_STATE(__states)

#define LUT_CALL_MASK(__states, __index, __mask) \
    __asm__("lda %v", __index); \
    __asm__("and #%b", (u8)__mask); \
    LUT_CALL_STATE(__states)

#ifdef PROFILER

/**
 * Adds emphasis to rendering, good for profiling (0-7)
 */
#define profile(x) { \
    __asm__("lda #%s", MASK_ALL + ((x) << 5)); \
    __asm__("sta $2001"); \
}

#else

#define profile(x) {}

#endif

#define PROFILE_CLEAR 0
#define PROFILE_OBJECTS 1
#define PROFILE_LEVEL 2
#define PROFILE_PLAYER 3
#define PROFILE_MISC 5

// set bg and spr palettes, data is 32 bytes array
void __fastcall__ pal_all(const u8 *data);

// set bg palette only, data is 16 bytes array
void __fastcall__ pal_bkg(const u8 *data);

// set spr palette only, data is 16 bytes array
void __fastcall__ pal_spr(const u8 *data);

// set spr row without background color, data is 3 byte array
#define pal_row(__row, __data) { \
    __asm__("lda #<(%v)", __data); \
    __asm__("ldy #>(%v)", __data); \
    __asm__("ldx #%b", (((__row) * 4) + 1)); \
    __asm__("jsr _pal_row_copy"); \
    }

// set a palette entry, index is 0..31
void __fastcall__ pal_color(const u8 color, const u8 index);

// reset palette to $0f
void __fastcall__ pal_clear(void);

// set virtual bright both for sprites and background, 0 is black, 4 is normal, 8 is white
void __fastcall__ pal_bright(u8 bright);

// set virtual bright for sprites only
void __fastcall__ pal_spr_bright(u8 bright);

// set virtual bright for sprites background only
void __fastcall__ pal_bkg_bright(u8 bright);

// wait a number of seconds
void __fastcall__ ppu_wait_seconds(u8 seconds);

// wait actual TV frame, 50hz for PAL, 60hz for NTSC
void __fastcall__ ppu_wait_nmi(void);

// wait virtual frame, it is always 50hz, frame-to-frame in PAL, frameskip in NTSC
void __fastcall__ ppu_wait_frame(void);

// turn off rendering, nmi still enabled when rendering is disabled
void __fastcall__ ppu_off(void);

// turn on bg, spr
void __fastcall__ ppu_on_all(void);

// turn on bg only
void __fastcall__ ppu_on_bg(void);

// turn on spr only
void __fastcall__ ppu_on_spr(void);

// clear ppu vram
void __fastcall__ ppu_clear_vram(u8 tile);

// set PPU_MASK directly
void __fastcall__ ppu_mask(u8 mask);

// get current video system, 0 for PAL, not 0 for NTSC
u8 __fastcall__ ppu_system(void);


// poll controller and return flags like PAD_LEFT etc, input is pad number (0 or 1)
u8 __fastcall__ pad_poll(u8 pad);

// poll controller in trigger mode, a flag is set only on button down, not hold
// if you need to poll the pad in both normal and trigger mode, poll it in the
// trigger mode for first, then use pad_state
u8 __fastcall__ pad_trigger(u8 pad);

// get previous pad state without polling ports
u8 __fastcall__ pad_state(u8 pad);


// set scroll, including rhe top bits
// it is always applied at beginning of a TV frame, not at the function call
void __fastcall__ scroll(u16 x, u16 y);

// set scroll after screen split invoked by the sprite 0 hit
// warning: all CPU time between the function call and the actual split point will be wasted!
// warning: the program loop has to fit into the frame time, ppu_wait_frame should not be used
//          otherwise empty frames without split will be inserted, resulting in jumpy screen
void __fastcall__ split(u16 x, u16 y);
void __fastcall__ split_second(u16 x, u16 y);
void __fastcall__ split_x(u16 x);


// select current chr bank for sprites, 0..1
void __fastcall__ bank_spr(u8 n);

// select current chr bank for background, 0..1
void __fastcall__ bank_bg(u8 n);

// select mirroring mode
void __fastcall__ set_mirroring(u8 mirror);

// select bank to use at $8000
void __fastcall__ bank_switch(u8 bank);

// save current bank to restore later
void __fastcall__ bank_switch_save(u8 bank);

// call using bank then restore
void __fastcall__ bank_call(callbackDef func, u8 bank);

// restores the current bank
void __fastcall__ bank_restore(void);


// get random number 0..255
u8 __fastcall__ prng(void);

// set random seed
void __fastcall__ set_rand(u16 seed);

// get random number 64..192
u8 __fastcall__ rand_middle(void);

// get the number of set bits in a byte
u8 __fastcall__ bit_count(u8 byte);

// donut compression
void __fastcall__ donut_bulk_load(const u8* data);



// when display is enabled, vram access could only be done with this vram update system
// the function sets a pointer to the update buffer that contains data and addresses
// in a special format. It allows to write non-sequental bytes, as well as horizontal or
// vertical nametable sequences.
// buffer pointer could be changed during rendering, but it only takes effect on a new frame
// number of transferred bytes is limited by vblank time
// to disable updates, call this function with NULL pointer

// the update data format:
//  MSB, LSB, byte for a non-sequential write
//  MSB|NT_UPD_HORZ, LSB, LEN, [bytes] for a horizontal sequence
//  MSB|NT_UPD_VERT, LSB, LEN, [bytes] for a vertical sequence
//  NT_UPD_EOF to mark end of the buffer

// length of this data should be under 256 bytes
void __fastcall__ set_vram_update(u8 *buf);

// all following vram functions only work when display is disabled
// do a series of VRAM writes, the same format as for set_vram_update, but writes done right away
void __fastcall__ flush_vram_update(u8 *buf);

// set vram pointer to write operations if you need to write some data to vram
void __fastcall__ vram_adr(u16 adr);

// put a byte at current vram address, works only when rendering is turned off
void __fastcall__ vram_put(u8 n);

// fill a block with a byte at current vram address, works only when rendering is turned off
void __fastcall__ vram_fill(u8 n, u16 len);


// draw a rect to the screen with given dimentions
// @param tile_def a nine byte array containing the tiles to draw
//                 TopL TopC TopR
//                 MidL MidC MidR
//                 BtmL BtmC BtmR
// @param dimensions upper byte = width, lower byte = height
void __fastcall__ vram_rect(const u8 *tile_def, u16 dimensions);

// size needs to be without the borders (minus two for width/height)
#define RECT(w, h) ((((w & 0xFF)-2) * 256) + ((h & 0xFF)-2))


// specialized vram drawing routine
// each set of drawing commands starts with a control byte and number of tiles based on the tiles to draw
// control byte: xxxxxnnn
// x=num to advance
// n=num tiles to draw
// limitations: only 8 tiles can be draw in one go and only 32 tiles can be skipped
void __fastcall__ vram_draw(const u8 *tiles);

#define DRAW(num, advance) ((advance << 3) + (num-1))

// advance a number of ppu tiles (0-255)
void __fastcall__ vram_advance(u8 advance);


// set vram autoincrement, 0 for +1 and not 0 for +32
void __fastcall__ vram_inc(u8 n);

// read a block from current address of vram, works only when rendering is turned off
void __fastcall__ vram_read(u8 *dst, u16 size);

// write a block to current address of vram, works only when rendering is turned off
void __fastcall__ vram_write(const u8 *src, u16 size);

// write a block to current address of vram, works only when rendering is turned off
void __fastcall__ vram_unrle(const u8 *src, u16 size);

// print text to the screen terminated with 0, can't be longer than 256 characters
void __fastcall__ vram_print(const u8 *src);

// quick set offset with only a single byte instead of u16
void __fastcall__ vadr_offset(u8 offset);

// page for vadr_offset, must be set before vadr_offset is called
extern u8 vadr_page;
#pragma zpsym("vadr_page");

// like a normal memcpy, but does not return anything
memcpy_result __fastcall__ memcpy(void *dst, const void *src, size_t len);

// like memset, but does not return anything
void __fastcall__ memfill(void *dst, u8 value, u16 len);

// basically memfill but setting the value to zero
// slower than calling memfill but it takes up less bytes
#define memclear(addr, len) { \
    __asm__("lda #<(%v)", addr); \
    __asm__("ldx #>(%v)", addr); \
    __asm__("jsr pushax"); \
    __asm__("lda #0"); \
    __asm__("jsr pusha"); \
    __asm__("tax"); \
    __asm__("lda #%b", len); \
    __asm__("jsr %v", memfill); \
}

// delay for N frames
void __fastcall__ delay(u8 frames);

// sleep for N seconds
void __fastcall__ sleep(u8 seconds);

// fast mod 3 (x % 3)
u8 __fastcall__ mod3(u8 value);

#endif // NESLIB_H
