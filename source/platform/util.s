	.globalzp TEMP
	.import popa,popax

	.export _memcpy,_memfill
	.export _mod3,_bit_count

PTR         =TEMP   ;word
LEN         =TEMP+2 ;word
SRC         =TEMP+4 ;word
DST         =TEMP+6 ;word

.CODE

;u8 __fastcall__ bit_count(u8 byte);
;Count the number of set bits in a byte
;out: the numer of set bits

_bit_count:

	ldx #0

@1:

	asl
	bcc @2
	inx
	ora #0

@2:

	bne @1
	txa
	ldx #0 ; return values are 16-bit so x must be zero
	rts


;void __fastcall__ memcpy(void *dst, void *src, u16 len);

_memcpy:

	sta <LEN
	stx <LEN+1
	jsr popax
	sta <SRC
	stx <SRC+1
	jsr popax
	sta <DST
	stx <DST+1

	ldx #0

@1:

	lda <LEN+1
	beq @2
	jsr @3
	dec <LEN+1
	inc <SRC+1
	inc <DST+1
	jmp @1

@2:

	ldx <LEN
	beq @5

@3:

	ldy #0

@4:

	lda (SRC),y
	sta (DST),y
	iny
	dex
	bne @4

@5:

	rts



;void __fastcall__ memfill(void *dst, u8 value, u16 len);

_memfill:

	sta <LEN
	stx <LEN+1
	jsr popa
	sta <TEMP
	jsr popax
	sta <DST
	stx <DST+1

	ldx #0

@1:

	lda <LEN+1
	beq @2
	jsr @3
	dec <LEN+1
	inc <DST+1
	jmp @1

@2:

	ldx <LEN
	beq @5

@3:

	ldy #0
	lda <TEMP

@4:

	sta (DST),y
	iny
	dex
	bne @4

@5:

	rts


;u8 __fastcall__ mod3(const u8 value);

.PROC _mod3

	ldx #$00 ; x needs to be 0 (return values are "16 bit")
	and #$03 ; a % 4
	cmp #$03
	bne exit
	txa      ; if (value == 3) a = 0

exit:
	rts

.ENDPROC
