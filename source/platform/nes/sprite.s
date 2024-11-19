	.export _oam_height_8,_oam_height_16,_oam_clear,_oam_draw,_oam_draw_pair
	.exportzp _oam_x, _oam_y, _oam_tile, _oam_tile_alt, _oam_attr
	.import _frame_counter

.include "nesdef.inc"
.include "zeropage.inc"

SCRX        =TEMP+2
SCRY        =TEMP+3


.ZEROPAGE

spr_id:   .RES 1

_oam_x:    .RES 1
_oam_y:    .RES 1
_oam_tile: .RES 1
_oam_tile_alt: .RES 1
_oam_attr: .RES 1

.CODE

;void __fastcall__ oam_clear();

.PROC _oam_clear
	lda #0
	sta spr_id

@oam_end_frame:
	ldx spr_id
	lda #$F8
@next_sprite:
	sta OAM_BUF+0,x
	inx
	inx
	inx
	inx
	bne @next_sprite

	rts
.ENDPROC

;void __fastcall__ oam_height_8();

.PROC _oam_height_8
	lda PPU_CTRL_VAR
	and #$df
	sta PPU_CTRL_VAR
	rts
.ENDPROC

;void __fastcall__ oam_height_16();

.PROC _oam_height_16
	lda PPU_CTRL_VAR
	ora #$20
	sta PPU_CTRL_VAR
	rts
.ENDPROC

;void __fastcall__ oam_draw(u8 tile);

_oam_draw:
	ldx spr_id
	; tile
	sta OAM_BUF+1,x
	; attribute
	lda _oam_attr
	sta OAM_BUF+2,x
	; y value
	lda _oam_y
	sta OAM_BUF+0,x
	; x value
	lda _oam_x
	sta OAM_BUF+3,x

	; increment or decrement spr_id
	txa
	clc
	adc #4
	sta spr_id
	rts

;void __fastcall__ oam_draw_pair(void);

.PROC _oam_draw_pair
	; check reverse
	lda _oam_attr
	and #$40 ; flip horizontal
	bne reverse
	; load tile
    lda _oam_tile
	ldy _oam_tile_alt
	jmp draw
reverse:
	; load tile
	lda _oam_tile_alt
	ldy _oam_tile
draw:
	jsr _oam_draw

	lda _oam_x
	pha        ; save oam_x value
	clc
	adc #8
	sta _oam_x

	tya ; next tile
	jsr _oam_draw
	pla
	sta _oam_x ; restore oam_x value
	rts

.ENDPROC

