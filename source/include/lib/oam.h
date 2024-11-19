#ifndef OAM_H
#define OAM_H

#define OAM             0x0200

#define OAM_PAL1		0x00
#define OAM_PAL2		0x01
#define OAM_PAL3		0x02
#define OAM_PAL4		0x03
#define OAM_BEHIND		0x20
#define OAM_FLIP_H		0x40
#define OAM_FLIP_V		0x80

extern unsigned char oam_x;
#pragma zpsym ("oam_x")

extern unsigned char oam_y;
#pragma zpsym ("oam_y")

extern unsigned char oam_tile;
#pragma zpsym ("oam_tile")

extern unsigned char oam_attr;
#pragma zpsym ("oam_attr")

// set sprite display mode to 8x8
void __fastcall__ oam_height_8(void);

// set sprite display mode to 8x16
void __fastcall__ oam_height_16(void);

// must set oam_x, oam_y, and oam_attr before calling
void __fastcall__ oam_draw(u8 tile);

// draw two sprites next to each other
void __fastcall__ oam_draw_pair(void);

// clear oam and offset spr_id to value
void __fastcall__ oam_clear(void);

#endif // OAM_H
