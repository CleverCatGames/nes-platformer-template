.import popax
.export _vram_unrle

.include "nesdef.inc"

.CODE

;void __fastcall__ vram_unrle(byte *src, word size);

_vram_unrle:

	sta <LEN
	stx <LEN+1

	jsr popax
	sta <PTR
	stx <PTR+1

	ldy #0

@loop:

	ldx #1
	lda (PTR),y
	cmp #$E0
	bcc @12

	sec
	sbc #$DF
	tax
	inc <PTR
	bne @1
	inc <PTR+1

@1:

	lda <LEN
	bne @11
	dec <LEN+1

@11:

	dec <LEN
	; should check if length is zero but assuming byte count is correct is faster
	lda (PTR),y

@12:

	sta PPU_DATA

	dex
	bne @12

	inc <PTR
	bne @2
	inc <PTR+1

@2:

	lda <LEN
	bne @3
	dec <LEN+1

@3:

	dec <LEN
	lda <LEN
	ora <LEN+1
	bne @loop
	rts

