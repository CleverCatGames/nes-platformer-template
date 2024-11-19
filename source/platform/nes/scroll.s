	.import popax

	.export _scroll
	.exportzp SCROLL_X,SCROLL_Y

.include "nesdef.inc"

PPU_CTRL_VAR1 =TEMP
SCROLL_X1     =TEMP+1
SCROLL_Y1     =TEMP+2

.ZEROPAGE
SCROLL_X:           .res 1
SCROLL_Y:           .res 1

.CODE

;void __fastcall__ scroll(word x, word y);

.PROC _scroll
	sta TEMP

	txa
	bne over_240
	lda TEMP
	cmp #240
	bcs over_240
	sta SCROLL_Y
	lda #0      ; nametable 1 or 2
	beq get_x	; uncoditional

over_240:

	sec
	lda TEMP
	sbc #240
	sta SCROLL_Y
	lda #2      ; nametable 3 or 4

get_x:
	sta TEMP    ; nametable

	jsr popax
	sta SCROLL_X
	txa
	and #$01
	ora TEMP
	sta TEMP
	lda PPU_CTRL_VAR
	and #%11111100   ; keep everything but nametable
	ora TEMP
	sta PPU_CTRL_VAR
	rts
.ENDPROC

