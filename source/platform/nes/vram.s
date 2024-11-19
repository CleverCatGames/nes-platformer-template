	.import popa,popax

    .export _vram_rect,_vram_print,_vram_draw,_vram_advance
	.export _vram_adr,_vram_put,_vram_fill,_vram_inc
    .export _vadr_page,_vadr_offset
	.export _set_vram_update,_flush_vram_update
	.export flush_vram_update_nmi

	.exportzp NAME_UPD_ENABLE

.include "nesdef.inc"

.ZEROPAGE
NAME_UPD_ADR:       .res 2
NAME_UPD_ENABLE:    .res 1
_vadr_page:         .res 1
_vadr_addr:         .res 1

.CODE

;void __fastcall__ vram_print(byte *src);

_vram_print:
	sta <PTR
	stx <PTR+1

	ldy #0

@loop:

	lda (PTR),y
	beq @exit       ; check for null terminator
	sta PPU_DATA
	iny

	bne @loop       ; loop until y resets (basically infinite)

@exit:

	rts


;void __fastcall__ vram_draw(const byte* tiles);

_vram_draw:

	sta <PTR
	stx <PTR+1

    ldy #0
@control_loop:
    lda (PTR),y  ; control byte
    beq @exit    ; quit if control byte is zero

    pha          ; save

    and #7       ; length must be less than 8
    tax
    inx          ; avoid looping with zero
    stx LEN

@draw_loop:

    iny
    lda (PTR),y
    sta PPU_DATA

    dex
    bne @draw_loop

    pla          ; restore
    lsr
    lsr
    lsr
    clc
    adc LEN      ; add tiles to draw
    jsr _vram_advance

    iny
    jmp @control_loop

@exit:
    rts


; draws a row for the rectangle

draw_rect_row:

    lda (PTR),y   ; draw first tile
    sta PPU_DATA

    iny           ; next tile

    ldx <LEN+1    ; draw middle part of row
@row_loop:
    lda (PTR),y
    sta PPU_DATA
    dex
    bne @row_loop

    iny           ; next tile

    lda (PTR),y   ; draw last tile
    sta PPU_DATA

    lda #32       ; advance to next row

    ; fallthrough

;void __fastcall__ vram_advance(u8 num_bytes);

_vram_advance:
    ldx _vadr_page ; load page for calling _vram_adr
    clc
    adc _vadr_addr ; add addr to the value passed
    bcc @set_adr
    inx            ; increase page, if carry was set
@set_adr:
    jmp _vram_adr


;void __fastcall__ vram_rect(const byte* tile_def, u16 dimensions);

_vram_rect:

    sta <LEN    ; height
    stx <LEN+1  ; width

	jsr popax
	sta <PTR
	stx <PTR+1

@draw_first:
    ldy #0
    jsr draw_rect_row

    iny
@draw_middle:
    jsr draw_rect_row
    dec <LEN
    beq @draw_last
    dey ; reset y
    dey
    jmp @draw_middle

@draw_last:
    iny
    jsr draw_rect_row

    rts


;void __fastcall__ set_vram_update(byte *buf);

_set_vram_update:

	sta <NAME_UPD_ADR+0
	stx <NAME_UPD_ADR+1
	ora <NAME_UPD_ADR+1
	sta <NAME_UPD_ENABLE

	rts



;void __fastcall__ flush_vram_update(byte *buf);

_flush_vram_update:

	sta <NAME_UPD_ADR+0
	stx <NAME_UPD_ADR+1

flush_vram_update_nmi:

	ldy #0

@updName:

	lda (NAME_UPD_ADR),y
	iny
	cmp #$40				;is it a non-sequential write?
	bcs @updNotSeq
	sta PPU_ADDR
	lda (NAME_UPD_ADR),y
	iny
	sta PPU_ADDR
	lda (NAME_UPD_ADR),y
	iny
	sta PPU_DATA
	jmp @updName

@updNotSeq:

	tax
	lda <PPU_CTRL_VAR
	cpx #$80				;is it a horizontal or vertical sequence?
	bcc @updHorzSeq
	cpx #$ff				;is it end of the update?
	beq @updDone

@updVertSeq:

	ora #$04
	bne @updNameSeq			;bra

@updHorzSeq:

	and #$fb

@updNameSeq:

	sta PPU_CTRL

	txa
	and #$3f
	sta PPU_ADDR
	lda (NAME_UPD_ADR),y
	iny
	sta PPU_ADDR
	lda (NAME_UPD_ADR),y  ; update length
	iny
	tax

@updNameLoop:

	lda (NAME_UPD_ADR),y
	iny
	sta PPU_DATA
	dex
	bne @updNameLoop

	lda <PPU_CTRL_VAR
	sta PPU_CTRL

	jmp @updName

@updDone:

	rts


_vadr_offset:
    ldx _vadr_page

;void __fastcall__ vram_adr(word adr);

_vram_adr:
    stx _vadr_page
    sta _vadr_addr
	stx PPU_ADDR
	sta PPU_ADDR
	rts



;void __fastcall__ vram_put(byte n);

_vram_put:
	sta PPU_DATA
	rts



;void __fastcall__ vram_fill(byte n, word len);

_vram_fill:

	sta <LEN
	stx <LEN+1
	jsr popa
	ldx <LEN+1
	beq @2
	ldx #0

@1:

	sta PPU_DATA
	dex
	bne @1
	dec <LEN+1
	bne @1

@2:

	ldx <LEN
	beq @4

@3:

	sta PPU_DATA
	dex
	bne @3

@4:

	rts



;void __fastcall__ vram_inc(byte n);

_vram_inc:

	ora #0
	beq @1
	lda #$04

@1:

	sta <TEMP
	lda <PPU_CTRL_VAR
	and #$fb
	ora <TEMP
	sta <PPU_CTRL_VAR
	sta PPU_CTRL

	rts
