#include "draw.h"
#include "player.h"
#include "level.h"

#pragma code-name("OBJECTS")
#pragma rodata-name("OBJECTS")

// ------------------------------------------------------------------------------------------------------------------
// HELPER FUNCTIONS
// ------------------------------------------------------------------------------------------------------------------

void obj_draw_clipped2(void) {
    __asm__("ldx %v", oam_tile); // oam_tile_alt = oam_tile + 2;
    __asm__("inx");
    __asm__("inx");
    __asm__("stx %v", oam_tile_alt);
    obj_draw_clipped();
}

void obj_draw_clipped(void)
{
    if (camera_diff_page == 0xFF) {
        // draw second sprite only
        __asm__("lda %v", oam_x);
        __asm__("pha");
        __asm__("clc");
        __asm__("adc #8");
        __asm__("sta %v", oam_x);
        oam_draw((oam_attr & OAM_FLIP_H) == 0 ? oam_tile_alt : oam_tile);
        __asm__("pla");
        __asm__("sta %v", oam_x); // restore oam_x
    } else {
        if (camera_diff_x < 248)
        {
            oam_draw_pair();
        }
        else
        {
            // account for flipped tile when off screen
            oam_draw((oam_attr & OAM_FLIP_H) ? oam_tile_alt : oam_tile);
        }
    }
}

void obj_draw_flipped(void)
{
    if (camera_diff_page == 0)
    {
        oam_draw(oam_tile);
    }
    if (camera_diff_page == 0xFF || oam_x < 0xF8)
    {
        oam_x += 8;
        oam_attr ^= OAM_FLIP_H; // flip bit
        oam_draw(oam_tile);
    }
}

void obj_draw_square_flip(void)
{
    tmp1 = object_timer & 0x08;
    // draw left half
    if (tmp1 == 0)
    {
        oam_attr |= OAM_FLIP_H;
        oam_tile += 2;
    }
    else
    {
        oam_attr &= ~OAM_FLIP_H; // clear
    }
    if (camera_diff_page == 0)
    {
        oam_draw(oam_tile);
    }
    // draw right half
    if (camera_diff_page == 0xFF || oam_x < 0xF8)
    {
        if (tmp1 == 0)
        {
            oam_tile -= 2;
        }
        else
        {
            oam_tile += 2;
        }
        oam_x += 8;
        oam_draw(oam_tile);
    }
}

void obj_draw_square_animate(void)
{
    // flip horizontal and vertical bits
    oam_attr |= (object_state & (OAM_FLIP_H | OAM_FLIP_V));
    oam_tile += (frame_counter & 0x10) >> 2;
    obj_draw_clipped2();
}

u8 obj_draw_24(void)
{
    if (camera_diff_page == 0) {
        oam_draw(oam_tile); // left
    }
    if (camera_diff_page == 0xFF || oam_x < 248)
    {
        oam_x += 8;
        // check if middle off screen left before drawing
        if (camera_diff_page == 0 || (camera_diff_page == 0xFF && oam_x < 248)) {
            oam_draw(oam_tile_alt); // middle
        }
        if (camera_diff_page == 0xFF || oam_x < 248)
        {
            oam_x += 8;
            return TRUE;
        }
    }
    return FALSE;
}

void obj_draw_mirror_24(void) {
    if (obj_draw_24()) {
        oam_attr ^= OAM_FLIP_H; // flip bit
        oam_draw(oam_tile); // right
    }
}

void obj_face_player(void) {
    // force OAM_FLIP_H bit on
    __asm__("lda %v", object_state);
    __asm__("ora #%b", (u8)OAM_FLIP_H);
    // compare camera location and player
    __asm__("ldy %v", camera_diff);
    __asm__("cpy %v", player_x);
    __asm__("bcc %g", set_state);
    // turn OAM_FLIP_H bit off by flipping it
    __asm__("eor #%b", (u8)OAM_FLIP_H);
set_state:
    __asm__("sta %v", object_state);
}

