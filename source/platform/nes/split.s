.import popax

.export _split,_split_second

.include "nesdef.inc"

PPU_CTRL_VAR1 =TEMP
SCROLL_X1     =TEMP+1
SCROLL_Y1     =TEMP+2

.MACRO _wait_sprite_zero
wait_sprite_zero_hit:
	bit PPU_STATUS
	bvs wait_sprite_zero_hit

wait_sprite_zero_clear:
	bit PPU_STATUS
	bvc wait_sprite_zero_clear
.ENDMACRO


.IFDEF SPLIT_X

.export _split_x
;;void __fastcall__ split_x(word x);

.PROC _split_x
	sta SCROLL_X1
	txa
	and #%00000001   ; nametable
	sta TEMP
	lda PPU_CTRL_VAR
	and #%11111100   ; keep control settings
	ora TEMP
	sta TEMP

	_wait_sprite_zero

	lda SCROLL_X1
	sta PPU_SCROLL   ; set x offset
	sta PPU_SCROLL   ; reset scroll
	lda TEMP
	sta PPU_CTRL

	rts
.ENDPROC

.ENDIF

setup_split:

	sta SCROLL_Y1
	and #%11000000
	asl
	rol
	rol
	sta TEMP
	lda SCROLL_Y1
	and #%00000011
	asl
	asl
	asl
	asl
	ora TEMP

	txa
	and #%00000010
	asl
	asl
	ora TEMP
	sta TEMP

	jsr popax

	sta SCROLL_X1
	lsr
	lsr
	lsr
	sta PPU_CTRL_VAR1
	lda SCROLL_Y1
	and #%00111000
	asl
	asl
	ora PPU_CTRL_VAR1
	sta PPU_CTRL_VAR1

	txa
	and #%00000001
	asl
	asl
	ora TEMP
	sta TEMP

    rts


do_split:
	lda TEMP
	sta PPU_ADDR
	lda SCROLL_Y1
	sta PPU_SCROLL
	lda SCROLL_X1
	sta PPU_SCROLL
	lda PPU_CTRL_VAR1
	sta PPU_ADDR

	rts


;;void __fastcall__ split(word x, word y);

.PROC _split
    jsr setup_split

	_wait_sprite_zero

    jmp do_split
.ENDPROC

.PROC _split_second
    jsr setup_split
    jmp do_split
.ENDPROC

