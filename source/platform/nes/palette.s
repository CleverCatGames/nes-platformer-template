	.import popa

	.export _pal_all,_pal_bkg,_pal_spr,_pal_row_copy,_pal_color,_pal_clear
	.export _pal_bright,_pal_spr_bright,_pal_bkg_bright

	.export pal_to_ppu
	.exportzp PAL_UPDATE

.include "nesdef.inc"

.ZEROPAGE
PAL_UPDATE:         .res 1
PAL_BKG_PTR:        .res 2
PAL_SPR_PTR:        .res 2

.CODE

pal_to_ppu:
	ldx #0
	stx <PAL_UPDATE

	lda #$3f
	sta PPU_ADDR
	stx PPU_ADDR

	ldy PAL_BUF				;background color, remember it in X
	lda (PAL_BKG_PTR),y
	sta PPU_DATA
	tax

	.repeat 3,I             ;first row of background palette
	ldy PAL_BUF+1+I
	lda (PAL_BKG_PTR),y
	sta PPU_DATA
	.endrepeat

	.repeat 3,J             ;last 3 rows of background palette
	stx PPU_DATA			;background color
	.repeat 3,I
	ldy PAL_BUF+5+(J*4)+I
	lda (PAL_BKG_PTR),y
	sta PPU_DATA
	.endrepeat
	.endrepeat

	.repeat 4,J             ;all 4 rows of sprite palette
	stx PPU_DATA			;background color
	.repeat 3,I
	ldy PAL_BUF+17+(J*4)+I
	lda (PAL_SPR_PTR),y
	sta PPU_DATA
	.endrepeat
	.endrepeat
	rts

;void __fastcall__ pal_all(const char *data);

_pal_all:

	sta <PTR
	stx <PTR+1
	ldx #$00
	lda #$20

.PROC pal_copy

	sta <LEN

	ldy #$00

loop_pal:
	lda (PTR),y
	sta PAL_BUF,x
	inx
	iny
	dec <LEN
	bne loop_pal

	inc <PAL_UPDATE

	rts
.ENDPROC



;void __fastcall__ pal_bg(const char *data);

_pal_bkg:

	sta <PTR
	stx <PTR+1
	ldx #$00
	lda #$10
	bne pal_copy ;bra



;void __fastcall__ pal_spr(const char *data);

_pal_spr:

	sta <PTR
	stx <PTR+1
	ldx #$10
	txa
	bne pal_copy ;bra



;void __fastcall__ pal_row_copy();
; x must be the background index

_pal_row_copy:
	sta <PTR
	sty <PTR+1

	ldy #0
.REPEAT 3
	lda (PTR),y
	iny
	sta PAL_BUF,x
	inx
.ENDREPEAT
	inc <PAL_UPDATE
	rts



;void __fastcall__ pal_color(byte color, byte index);

_pal_color:

	and #$1f
	tax
	jsr popa
	sta PAL_BUF,x
	inc <PAL_UPDATE
	rts



;void __fastcall__ pal_clear(void);

.PROC _pal_clear
	lda #$0f ; set to black color
	ldx #$20

loop_pal:
	sta PAL_BUF,x
	dex
	bne loop_pal
	sta <PAL_UPDATE
	rts
.ENDPROC



;void __fastcall__ pal_spr_bright(byte bright);

_pal_spr_bright:

	tax
	lda palBrightTableL,x
	sta <PAL_SPR_PTR
	lda palBrightTableH,x	;MSB is never zero
	sta <PAL_SPR_PTR+1
	sta <PAL_UPDATE
	rts



;void __fastcall__ pal_bkg_bright(byte bright);

_pal_bkg_bright:

	tax
	lda palBrightTableL,x
	sta <PAL_BKG_PTR
	lda palBrightTableH,x	;MSB is never zero
	sta <PAL_BKG_PTR+1
	sta <PAL_UPDATE
	rts



;void __fastcall__ pal_bright(byte bright);

_pal_bright:

	jsr _pal_spr_bright
	txa
	jmp _pal_bkg_bright



palBrightTableL:

	.byte <palBrightTable0,<palBrightTable1,<palBrightTable2
	.byte <palBrightTable3,<palBrightTable4,<palBrightTable5
	.byte <palBrightTable6,<palBrightTable7,<palBrightTable8

palBrightTableH:

	.byte >palBrightTable0,>palBrightTable1,>palBrightTable2
	.byte >palBrightTable3,>palBrightTable4,>palBrightTable5
	.byte >palBrightTable6,>palBrightTable7,>palBrightTable8

palBrightTable0:
	.byte $0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f	;black
palBrightTable1:
	.byte $0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f
palBrightTable2:
	.byte $0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f
palBrightTable3:
	.byte $0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f,$0f
palBrightTable4:
	.byte $00,$01,$02,$03,$04,$05,$06,$07,$08,$09,$0a,$0b,$0c,$0f,$0f,$0f	;normal colors
palBrightTable5:
	.byte $10,$11,$12,$13,$14,$15,$16,$17,$18,$19,$1a,$1b,$1c,$00,$00,$00
palBrightTable6:
	.byte $10,$21,$22,$23,$24,$25,$26,$27,$28,$29,$2a,$2b,$2c,$10,$10,$10	;$10 because $20 is the same as $30
palBrightTable7:
	.byte $30,$31,$32,$33,$34,$35,$36,$37,$38,$39,$3a,$3b,$3c,$20,$20,$20
palBrightTable8:
	.byte $30,$30,$30,$30,$30,$30,$30,$30,$30,$30,$30,$30,$30,$30,$30,$30	;white
	.byte $30,$30,$30,$30,$30,$30,$30,$30,$30,$30,$30,$30,$30,$30,$30,$30
	.byte $30,$30,$30,$30,$30,$30,$30,$30,$30,$30,$30,$30,$30,$30,$30,$30
	.byte $30,$30,$30,$30,$30,$30,$30,$30,$30,$30,$30,$30,$30,$30,$30,$30


