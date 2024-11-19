.export _prng,_set_rand
.export _rand_middle

.exportzp RAND_SEED

.ZEROPAGE
RAND_SEED: .res 2

.CODE

;u8 __fastcall__ prng(void);
;Galois random generator, found somewhere
;out: A random number 0..255

_prng:
	lda RAND_SEED+1
	tay ; store copy of high byte
	; compute RAND_SEED+1 ($39>>1 = %11100)
	lsr ; shift to consume zeroes on left...
	lsr
	lsr
	sta RAND_SEED+1 ; now recreate the remaining bits in reverse order... %111
	lsr
	eor RAND_SEED+1
	lsr
	eor RAND_SEED+1
	eor RAND_SEED+0 ; recombine with original low byte
	sta RAND_SEED+1
	; compute RAND_SEED+0 ($39 = %111001)
	tya ; original high byte
	sta RAND_SEED+0
	asl
	eor RAND_SEED+0
	asl
	eor RAND_SEED+0
	asl
	asl
	asl
	eor RAND_SEED+0
	sta RAND_SEED+0
	rts

;void __fastcall__ set_rand(u16 seed);

_set_rand:
	sta <RAND_SEED
	stx <RAND_SEED+1
	rts


_rand_middle:
    jsr _prng
    lsr
    clc
    adc #64
    rts

