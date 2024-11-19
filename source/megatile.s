.importzp tmp1,tmp2,tmp3,tmp4,_tmp5
.importzp ptr1,ptr4
.importzp _vram_offset
.importzp _pLevelBuffer,_pTilePtr,_pMapData
.importzp _pMetatileData,_pMetatileAttr
.import _tile_buffer
.import _vram_buffer
.import _one_shift

.export megatile_block

.export _load_megatile,_load_megatile_buffer
.export _megatile_to_vram,_megatile_attr
.export _megatile_unrle
.export _gfx_ignore,_do_nothing,_collide_ignore,_script_ignore,_draw_none ; empty functions

.segment "PRGRAM"

megatile_block: .RES 1024 ; supports 256 megatiles

.CODE

; void __fastcall__ load_megatile_bufer(u8);
_load_megatile_buffer:
	tax ; save index in x register
	ldy #0
    lda (_pLevelBuffer),y
    jsr load_megatile_internal
@increase_buffer: ; ++pLevelBuffer
    inc _pLevelBuffer
    bne @1
    inc _pLevelBuffer+1
@1:
	lda (ptr4),y
	sta _tile_buffer,x ; top left
	iny
	inx
	lda (ptr4),y
	sta _tile_buffer,x ; bottom left
	iny
	txa
	clc
	adc #13 ; SCREEN_ROWS*2 - 1
	tax
	lda (ptr4),y
	sta _tile_buffer,x ; top right
	iny
	inx
	lda (ptr4),y
	sta _tile_buffer,x ; bottom right
    rts

; u8 __fastcall__ load_megatile(u8);
_load_megatile:
    sta tmp2 ; save tile offset
    ldy #0
    lda (_pTilePtr),y
    jsr load_megatile_internal
    ldy tmp2 ; load tile offset
    lda (ptr4),y ; get tile data
    rts

load_megatile_internal:
	sty tmp1 ; clear out value (y is zero already)
    ; move lower 2 bits of a into upper 2 of tmp1
    ;    a - 000000xx
    ; tmp1 - xx000000
	asl a
	rol tmp1
	asl a
	rol tmp1
	clc
    adc #<megatile_block
	sta ptr4
	lda tmp1
    adc #>megatile_block
	sta ptr4+1
_do_nothing: ; empty functions (better than creating in C)
_draw_none:
_gfx_ignore:
_script_ignore:
_collide_ignore:
    rts

; Used in dialogue to fill vram_buffer with metatile information
; fill metatile data in vram_buffer
; A = metatile index
; X = vram_buffer offset
metatile_to_vram:
    ; clear upper byte of ptr1 (so we can rotate into it)
    ldy #0
    sty ptr1+1

    ; rotate upper two bits of metatile into upper byte of ptr1
    asl
    rol ptr1+1
    asl
    rol ptr1+1
    tay ; Y = A * 4

    ; store offset from pMetatileData in ptr1
    ; metatiles are stored 4 tiles wide so that's why we are rotating bits
    lda _pMetatileData
    sta ptr1+0
    lda _pMetatileData+1
    clc
    adc ptr1+1
    sta ptr1+1

    ; now we can read the tiles for this metatile
    ; remember that the Y register contains our offset so now we can read sequentially
    lda (ptr1),y ; top left
    sta _vram_buffer,x
    iny
    inx
    lda (ptr1),y ; top right
    sta _vram_buffer,x
    iny
    ; add 15 to the x register for the correct vram offset
    txa
    clc
    adc _vram_offset ; 12 + 3 vram bytes - incremented byte
    adc #2
    tax
    ; back to our regularly scheduled program
    lda (ptr1),y ; bottom left
    sta _vram_buffer,x
    iny
    inx
    lda (ptr1),y ; bottom right
    sta _vram_buffer,x
	rts

; void __fastcall__ megatile_to_vram(u8 offset);

_megatile_to_vram:
	tax
	ldy #0
	sty _tmp5 ; clear _tmp5 for later
    lda (_pTilePtr),y
    jsr load_megatile_internal

	lda (ptr4),y
    ; megatile to metatile
    ; | tmp1 | tmp2 |
    ; | tmp3 | tmp4 |
    sta tmp1
    iny
    lda (ptr4),y
    sta tmp2
    iny
    lda (ptr4),y
    sta tmp3
    iny
    lda (ptr4),y
    sta tmp4

	; X is loaded from function call
    lda tmp1
    jsr metatile_to_vram ; top left

	txa     ; subtract 14 from X
	sec     ; should be first X + 2
    sbc _vram_offset
    sbc #2
	tax
    lda tmp3
    jsr metatile_to_vram ; bottom left

	txa     ; add 12 to X
	clc     ; should be first X + 30
    adc _vram_offset
	tax
    lda tmp2
    jsr metatile_to_vram ; top right

	txa     ; subtract 14 from X
	sec     ; should be first X + 32
    sbc _vram_offset
    sbc #2
	tax
    lda tmp4
    jmp metatile_to_vram ; bottom right

_megatile_attr:
	tax

	; attribute for top left tile
	ldy tmp1
	lda (_pMetatileAttr),y
	asl
	rol _tmp5
	asl
	rol _tmp5

	; attribute for top right tile
	ldy tmp2
	lda (_pMetatileAttr),y
	and #$C0
	lsr
	lsr
	ora _tmp5
	sta _tmp5

	; attribute for bottom left tile
	ldy tmp3
	lda (_pMetatileAttr),y
	and #$C0
	lsr
	lsr
	lsr
	lsr
	ora _tmp5
	sta _tmp5

	; attribute for bottom right tile
	ldy tmp4
	lda (_pMetatileAttr),y
	and #$C0
	ora _tmp5

	sta _vram_buffer,x
	rts

; increment pMapData value
.PROC read_map_data
    ; advance to next byte
    inc <_pMapData
    bne @exit
    inc <_pMapData+1     ; increment high byte if rotating to zero
@exit:
    lda (_pMapData),y
    rts
.ENDPROC

.PROC store_tile_byte
    sta (ptr1),y
    inc <ptr1
    bne @exit
    inc <ptr1+1
@exit:
    rts
.ENDPROC

_megatile_unrle:

    lda #<megatile_block
    ldx #>megatile_block
    sta <ptr1
    stx <ptr1+1

    ldy #0 ; y register stays fixed to zero
    lda (_pMapData),y

@loop:

    ldx #1 ; x register hold number of bytes to write

    cmp #$FF
    bne @check_rle

    jsr read_map_data
    rts ; if $FF byte hit, exit routine

@check_rle:
	cmp #$80
    bcc @store_value ; if $80 is unset, simply store the value

    cmp #$C0
    bcc @rle ; if $40 is unset, it's a run length byte

    ; get zero bitmap (11xxxxxx)
    ; if a bit is set, a number must be read, otherwise it's a zero
    and #$3F
    sta tmp1

    ldx #5 ; check 6 bits total

@bitmap_loop:
    lda _one_shift,x
    and tmp1
    beq @store_bitmap ; if bit is zero, store a zero

    ; otherwise, load a value and store it
    jsr read_map_data

@store_bitmap:
    jsr store_tile_byte
    dex
    bpl @bitmap_loop
    jmp @read_next

@rle:

    ; store length value in x register
    and #$1F
	tax

    ; if zero bit is reset, store zeroes
    lda (_pMapData),y
    and #$20
    beq @store_value

    ; otherwise, store value from next memory location
    jsr read_map_data

@store_value:

    jsr store_tile_byte

	dex
	bne @store_value

@read_next:
    jsr read_map_data
    ; done storing values, force loop
    jmp @loop

