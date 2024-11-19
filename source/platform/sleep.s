.import _ppu_wait_nmi
.importzp _tmp1
.export _delay,_sleep

;void __fastcall__ delay(byte frames);

.PROC _delay
	tax

next_frame:
	jsr _ppu_wait_nmi
	dex
	bne next_frame

	rts
.ENDPROC

;void __fastcall__ sleep(const u8 value);

.PROC _sleep
    sta _tmp1
next_sec:
    lda #60
	jsr _delay
    dec _tmp1
    bne next_sec
exit:
	rts
.ENDPROC